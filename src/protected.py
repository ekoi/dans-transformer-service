import json
import shutil
import uuid
import logging
import os
from typing import Union
from xml.dom.minidom import parseString

import requests
import xml.etree.ElementTree as ET
from xml.dom import minidom
import xml

import xmltodict
from boltons.iterutils import remap
from fastapi import Response
from rdflib import Graph

# import codecs
from fastapi import APIRouter, Request, HTTPException, Query

from src.commons import data, settings, validate_json, RdfOutputFormat, prettify_xml, \
    OutputFormat, initialize_templates, initialize_xslt_proc

router = APIRouter()


@router.post('/upload-xsl/{xslt_file_name}/{save}', status_code=201, tags=['Upload XSLT'])
async def submit_xsl(xslt_file_name: str, submitted_xsl: Request, save: bool | None = False):
    xslt_file_name = await construct_xslt_file_name(xslt_file_name)

    content_type = submitted_xsl.headers['Content-Type']
    s_xsl = ""
    if content_type == 'application/xml':
        s_xsl = await submitted_xsl.body()
    else:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')

    msg = await process_xsl(s_xsl, save, xslt_file_name)

    return {"message": msg}


async def construct_xslt_file_name(xslt_file_name):
    if not (xslt_file_name.endswith(".xsl") or xslt_file_name.endswith('.xslt')):
        xslt_file_name = f'{xslt_file_name}.xsl'
    return xslt_file_name


@router.post('/upload-xsl/{xslt_name}/{xsl_url:path}/{save}', status_code=201, tags=['Upload XSLT'])
async def submit_xslt_from_url(xslt_name: str, xsl_url: str, save: bool | None = False):
    response = requests.get(xsl_url)
    if response.status_code != 200:
        raise HTTPException(status_code=response.status_code,
                            detail=f'Retrieve response code {response.status_code} from {xsl_url}')
    else:
        xsl = response.text
        xslt_file_name = await construct_xslt_file_name(xslt_name)
        msg = await process_xsl(xsl, save, xslt_file_name)

    return {"message": msg}


@router.post("/transform/{xslt_name}", tags=['Transform'])
async def transform(xslt_name: str, submitted_json_or_xml: Request):
    logging.debug(f'xslt_name: {xslt_name}')
    print(f'xslt_name: {xslt_name}')
    content_type = submitted_json_or_xml.headers['Content-Type']
    str_xml = ""
    if xslt_name not in data.keys():
        raise HTTPException(status_code=500, detail=f'The given xslt_name: "{xslt_name}" is not found')

    if content_type in ['application/json', 'application/xml']:
        xml_tmpfile = await create_xml_tmp_file_name(xslt_name)
        if content_type == 'application/json':
            submitted_json = await submitted_json_or_xml.json()
            str_xml = await validate_xml_encapsulated_json(submitted_json, xml_tmpfile)
        else:
            submitted_xml = await submitted_json_or_xml.body()
            str_xml = await validate_submitted_xml(submitted_xml)

    else:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')

    result = await transform_to_string(str_xml, xml_tmpfile, xslt_name)
    return {"result": result}


@router.post("/transform/{xslt_name}/{output_type}", tags=['Transform'])
async def transform_specified_outpu(xslt_name: str, output_type: OutputFormat, submitted_json_or_xml: Request):
    content_type = submitted_json_or_xml.headers['Content-Type']
    str_xml = ""
    submitted_xml = ""
    if xslt_name not in data.keys():
        raise HTTPException(status_code=500, detail=f'The given xslt_name: "{xslt_name}" is not found')

    if content_type in ['application/json', 'application/xml']:
        xml_tmpfile = await create_xml_tmp_file_name(xslt_name)
        if content_type == 'application/json':
            submitted_json = await submitted_json_or_xml.json()
            str_xml = await validate_xml_encapsulated_json(submitted_json, xml_tmpfile)
        else:
            submitted_xml = await submitted_json_or_xml.body()
            str_xml = await validate_submitted_xml(submitted_xml)

    else:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')

    result = await transform_to_string(str_xml, xml_tmpfile, xslt_name)
    if output_type == output_type.json:
        try:
            return json.loads(result)
        except ValueError as e:
            return HTTPException(status_code=500, detail=f"Output format isn't valid {output_type} format.")
    elif output_type == output_type.xml:
        try:
            tree = ET.fromstring(submitted_xml)
            return tree
        except ET.ParseError as pe:
            return HTTPException(status_code=500, detail=f"Output format isn't valid {output_type} format.")

    return result


# @router.post("/transform/{xslt_name}", tags=['Transform'])
# async def transform(xslt_name: str, submitted_json_or_xml: Request):
#     content_type = submitted_json_or_xml.headers['Content-Type']
#     str_xml = ""
#     if xslt_name not in data.keys():
#         raise HTTPException(status_code=500, detail=f'The given xslt_name: "{xslt_name}" is not found')
#
#     if content_type in ['application/json', 'application/xml']:
#         temp_file = settings.TEMP_TRANSFORM_FILE + "-" + str(uuid.uuid1()) + ".xml"
#         if content_type == 'application/json':
#             submitted_json = await submitted_json_or_xml.json()
#             str_xml = await validate_xml_encapsulated_json(submitted_json, temp_file)
#         else:
#             submitted_xml = await submitted_json_or_xml.body()
#             str_xml = await validate_submitted_xml(submitted_xml)
#
#     else:
#         raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')
#
#     result = await transform_to_string(str_xml, submitted_json, temp_file, xslt_name)
#     return {"result": result}

@router.post("/transform/{xslt_name}/{source_url:path}", tags=['Transform'])
async def transform(xslt_name: str, source_url,
                    output_format: Union[str | None] = Query(default=None, enum=["json", "xml"])):
    try:
        response = requests.get(source_url)
    except requests.exceptions.RequestException as ce:
        raise HTTPException(status_code=404, detail=f'RequestException from {source_url}')

    if response.status_code != 200:
        raise HTTPException(status_code=response.status_code,
                            detail=f'Retrieve response code {response.status_code} from {source_url}')

    if xslt_name not in data.keys():
        raise HTTPException(status_code=500, detail=f'The given xslt_name: "{xslt_name}" is not found')

    content_type = response.headers['Content-Type']
    if content_type in ['application/json', 'application/xml', 'application/json;charset=UTF-8',
                        'text/plain; charset=utf-8']:
        xml_tmp_file_name = create_xml_tmp_file_name(xslt_name)
        if content_type in ['application/json', 'application/json;charset=UTF-8', 'text/plain; charset=utf-8']:
            if validate_json(response.content.decode("UTF-8")):
                submitted_json = json.loads(response.content.decode("UTF-8"))
                str_xml = await validate_xml_encapsulated_json(submitted_json, xml_tmp_file_name)
            else:
                raise HTTPException(status_code=400, detail='Not valid json.')
        else:
            submitted_xml = await response.content.decode("UTF-8")
            str_xml = await validate_submitted_xml(submitted_xml)
    else:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')

    result = await transform_to_string(str_xml, submitted_json, xml_tmp_file_name, xslt_name)
    if output_format == 'json':
        if validate_json(result):
            return {"result": json.loads(result)}
        else:
            return {"ERROR": "Not valid JSON format", "result": result}
    elif output_format == 'xml':
        try:
            result_as_xml = prettify_xml(result)
            return Response(content=result_as_xml, media_type="application/xml")
        except xml.parsers.expat.ExpatError as e:
            return {"ERROR": "Not wel format XML", "result": result}

    else:
        return {"result": result}


# @router.post("/transform2/{xslt_url:path}/{use_cached_xslt}", tags=['Transform'])
# async def transform(xslt_url, use_cached_xslt: bool, submitted_json_or_xml: Request, output_format: Union[str | None] = Query(default=None, enum=["json", "xml"])):
#
#     # Format the name and check whether the xslt exist
#     xslt_url_file_name = str(xslt_url).replace('https://raw.githubusercontent.com/', '').replace('/', '') + '.xsl'
#     if xslt_url_file_name in data.keys() and use_cached_xslt:
#         logging.debug(f'{xslt_url_file_name} is in the cache.')
#     else:
#         # In the case that the 'xslt_url_file_name' not in the cache or use_cached_xslt is false
#         # Or when use_cached_xslt is true, but it is for the first time
#         # try:
#         await submit_xslt_from_url(xslt_url_file_name, xslt_url, use_cached_xslt)
# rsp_xslt_url = requests.get(xslt_url)
# xsl = rsp_xslt_url.text
# xslt_file_name = await construct_xslt_file_name(xslt_url_file_name)
# except requests.exceptions.RequestException as ce:
#     raise HTTPException(status_code=404, detail=f'RequestException from {xslt_url}')
#
# if rsp_xslt_url.status_code != 200:
#     raise HTTPException(status_code=rsp_xslt_url.status_code,
#                         detail=f'Retrieve response code {rsp_xslt_url.status_code} from {xslt_url}')
#


# content_type = rsp_source_url.headers['Content-Type']
# if content_type in ['application/json', 'application/xml', 'application/json;charset=UTF-8', 'text/plain; charset=utf-8']:
#     xml_tmp_file_name = create_xml_tmp_file_name(xslt_url_file_name)
#     if content_type in ['application/json', 'application/json;charset=UTF-8', 'text/plain; charset=utf-8']:
#         if validate_json(rsp_source_url.content.decode("UTF-8")):
#             submitted_json = json.loads(rsp_source_url.content.decode("UTF-8"))
#             str_xml = await validate_xml_encapsulated_json(submitted_json, xml_tmp_file_name)
#         else:
#             raise HTTPException(status_code=400, detail='Not valid json.')
#     else:
#         submitted_xml = await rsp_source_url.content.decode("UTF-8")
#         str_xml = await validate_submitted_xml(submitted_xml)
# else:
#     raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')


#
# result = await transform_to_string(str_xml, submitted_json, xml_tmp_file_name, xslt_url_file_name)
# if output_format == 'json':
#     if validate_json(result):
#         return {"result": json.loads(result)}
#     else:
#         return {"ERROR": "Not valid JSON format", "result": result}
# elif output_format == 'xml':
#     try:
#         result_as_xml = prettify_xml(result)
#         return Response(content=result_as_xml, media_type="application/xml")
#     except xml.parsers.expat.ExpatError as e:
#         return {"ERROR": "Not welformat XML", "result": result}
#
# else:
#     return {"result": result


@router.post("/transform-jsonld-to-rdf", tags=['Transform'], name='Transform json-ld to RDF-XML format.'
    , description='The output will be in RDF-XML format.')
async def transform(submitted_json: Request):
    content_type = submitted_json.headers['Content-Type']
    if content_type not in ['application/json+ld']:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')
    submitted_json = await submitted_json.json()
    result = await transform_to_rdf(json.dumps(submitted_json))

    return {"result": result}


@router.post("/transform-jsonld-to-rdf/{output_format}", tags=['Transform'],
             name='Transform json-ld to a given RDF output format.'
    , description='The output will be in RDF-XML format.')
async def transform(output_format: RdfOutputFormat, submitted_json: Request):
    if submitted_json.headers['Content-Type'] not in ['application/json+ld']:
        raise HTTPException(status_code=400,
                            detail=f"Content type {submitted_json.headers['Content-Type']} not supported")
    submitted_json = await submitted_json.json()
    result = await transform_to_rdf(json.dumps(submitted_json), output_format.value)

    return {"result": result}


@router.post("/transform-jsonld-to-rdf/{output_format}/{source_url:path}", tags=['Transform'],
             name='Transform json-ld to a given RDF output format.')
async def transform(output_format: RdfOutputFormat, source_url):
    response = requests.get(source_url)
    if response.status_code != 200:
        raise HTTPException(status_code=response.status_code,
                            detail=f'Retrieve response code {response.status_code} from {source_url}')

    # content_type = response.headers['Content-Type']
    # if content_type not in ['application/json+ld', 'application/json;charset=UTF-8']:
    #     raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')

    result = await transform_to_rdf(response.content, output_format.value)

    return {"result": result}


@router.post("/transform-xml-to-json/{clean_output}", tags=['Transform'], name='Transform xml to json format.'
    , description='The output will be in json format.')
async def transform(submitted_xml: Request, clean_output: bool | None = False):
    content_type = submitted_xml.headers['Content-Type']
    if content_type not in ['application/xml']:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')
    submitted_xml = await submitted_xml.body()
    result = xmltodict.parse(submitted_xml)
    j_str = json.dumps(result).replace('{"@xsi:nil": "true"}', '""')
    new_result = json.loads(j_str)
    if clean_output:
        clean_result = remap(new_result, visit=lambda path, key, value: bool(value))
        return {"result": clean_result}
    return {"result": new_result}


@router.get("/ping", include_in_schema=False)
async def say_hi(name: str):
    return "Hi " + name


@router.get("/settings", include_in_schema=False)
async def get_settings():
    return settings


@router.get("/refresh", include_in_schema=False)
def refresh():
    data.clear()
    initialize_templates()
    initialize_xslt_proc()
    data_list = list(data.keys())
    return data_list


@router.delete("/delete-saved-xsl/{xslt_name}", status_code=204, tags=['Danger Zone'])
def delete_saved_xsl(xslt_name: str):
    raise HTTPException(status_code=501, detail=f'This endpoint is not implemented yet.')


async def create_xml_tmp_file_name(xslt_name):
    xml_tmp_file_name = f"{settings.TEMP_TRANSFORM_FILE}-{str(uuid.uuid1())}-{xslt_name}.xml"
    return xml_tmp_file_name


async def process_xsl(s_xsl, save, xslt_name):
    executable_xslt = await ceate_executable_xslt(s_xsl)
    msg = f'{xslt_name} is active and saved.'
    if not save:
        msg = f'xslt_name is active but not saved. It wil destroy when the service is restart.'
    else:
        if not isinstance(s_xsl, str):
            s_xsl = s_xsl.decode('UTF-8')
        with open(os.path.join(settings.SAVED_XSLT_DIR, xslt_name), mode="w") as file:
            file.write(s_xsl)
    data.update({xslt_name: executable_xslt})
    return msg


async def transform_to_string(str_xml, xml_tmp_file_name, xslt_name):
    with open(xml_tmp_file_name, mode="w") as file:
        file.write(str_xml)
    # file = codecs.open(temp_file, "w", "utf-8")
    # file.write(str_submitted_xml)
    # file.close()
    result = data[xslt_name].transform_to_string(source_file=xml_tmp_file_name)
    if result is None:
        logging.debug(f'Empty result, submitted_json: {str_xml}. XSLT: {xslt_name}')
        raise HTTPException(status_code=500, detail=f'Empty result, submitted_json: {str_xml}')
    try:
        os.remove(xml_tmp_file_name)
    except:
        logging.error(f'Error while deleting file {xml_tmp_file_name}')
    logging.debug(result)
    return result


async def validate_submitted_xml(submitted_xml):
    if not isinstance(submitted_xml, str):
        submitted_xml = submitted_xml.decode('UTF-8')
    try:
        # lxml should've thrown exception when parsing non well-formed XML
        tree = ET.fromstring(submitted_xml)
        return submitted_xml
    except ValueError as err:
        logging.debug(err)
        raise HTTPException(status_code=500, detail=f'Submitted XML is not valid. {err}')


async def validate_xml_encapsulated_json(submitted_json, xml_tmp_file_name):
    # Create xml with the given json encapsulated.
    try:
        # submitted_json = {k: v.replace("&","&#38;").replace("<","&#60;") for k, v in submitted_json.items()}
        submitted_json_str = json.dumps(submitted_json)
        # saxon needs json that encapsulates in xml
        str_xml_encapsulated_json = '<data>' + submitted_json_str + '</data>'
        # write the xml to a temporary file
        with open(xml_tmp_file_name, mode="w") as file:
            file.write(str_xml_encapsulated_json)
        etree = ET.parse(xml_tmp_file_name)
        return str_xml_encapsulated_json
    except ET.ParseError as pe:
        logging.debug(pe)
        shutil.copyfile(xml_tmp_file_name, xml_tmp_file_name + "-error-tobe_converted")
        return remove_xml_invalid_characters(submitted_json_str, xml_tmp_file_name)
    except ValueError as err:
        logging.debug(err)
        raise HTTPException(status_code=500, detail=f'Submitted json is not valid. {err}')


#
def remove_xml_invalid_characters(str_json, xml_tmp_file):
    str_xml = "<data>" + str_json.replace("&", "&#38;").replace("<", "&#60;") + "</data>"
    # write the xml to a xml temporary file
    with open(xml_tmp_file, mode="w") as file:
        file.write(str_xml)
    try:
        etree = ET.parse(xml_tmp_file)
        return str_xml
    except ET.ParseError as pe:
        logging.debug(pe)
        shutil.copyfile(xml_tmp_file, xml_tmp_file + "-ERROR-converted-fail")
        raise HTTPException(status_code=500, detail=f'Transformed json is not valid. {pe}')


async def transform_to_rdf(str_submitted_json, output_format='xml'):
    if not validate_json(str_submitted_json):
        raise HTTPException(status_code=500, detail=f'Error, msg: The given json input is not valid.')
    g = Graph().parse(data=json.loads(str_submitted_json), format='json-ld')
    try:
        if output_format == 'xml':
            output_format = "pretty-xml"
        result = g.serialize(format=output_format)
    except ValueError as err:
        logging.debug(f'error caused by: {err}')
        raise HTTPException(status_code=500, detail=f'Error, msg: {err} ')
    except:
        raise HTTPException(status_code=500, detail=f'Error!')
    return result
