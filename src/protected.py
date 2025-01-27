import json
import logging
import os
import shutil
import uuid
import xml
import xml.etree.ElementTree as ET
from typing import Union

import requests
import xmltodict
from boltons.iterutils import remap
# import codecs
from fastapi import APIRouter, Request, HTTPException, Query
from fastapi import Response
from rdflib import Graph

from src.commons import data, settings, validate_json, RdfOutputFormat, prettify_xml, \
    OutputFormat, initialize_templates, initialize_xslt_proc

router = APIRouter()


@router.post('/upload-xsl/{xslt_file_name}/{save}', status_code=201, tags=['Upload XSLT'])
async def submit_xsl(xslt_file_name: str, submitted_xsl: Request, save: bool | None = False):
    """
    Endpoint to upload an XSLT file.

    Args:
        xslt_file_name (str): The name of the XSLT file to be uploaded.
        submitted_xsl (Request): The request object containing the XSLT file content.
        save (bool | None): Flag indicating whether to save the XSLT file. Defaults to False.

    Returns:
        dict: A dictionary containing a message about the status of the upload.

    Raises:
        HTTPException: If the content type of the submitted file is not 'application/xml'.
    """
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
    """
    Endpoint to upload an XSLT file from a URL.

    Args:
        xslt_name (str): The name to save the XSLT file as.
        xsl_url (str): The URL from which to fetch the XSLT file.
        save (bool | None): Flag indicating whether to save the XSLT file. Defaults to False.

    Returns:
        dict: A dictionary containing a message about the status of the upload.

    Raises:
        HTTPException: If the response status code from the URL is not 200.
    """
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
    """
    Endpoint to transform a submitted JSON or XML document using a specified XSLT.

    Args:
        xslt_name (str): The name of the XSLT to be used for transformation.
        submitted_json_or_xml (Request): The request object containing the JSON or XML document to be transformed.

    Returns:
        dict: A dictionary containing the transformation result.

    Raises:
        HTTPException: If the XSLT name is not found in the data keys.
        HTTPException: If the content type of the submitted document is not supported.
    """
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
    """
    Endpoint to transform a submitted JSON or XML document using a specified XSLT and output format.

    Args:
        xslt_name (str): The name of the XSLT to be used for transformation.
        output_type (OutputFormat): The desired output format (json or xml).
        submitted_json_or_xml (Request): The request object containing the JSON or XML document to be transformed.

    Returns:
        dict or xml.etree.ElementTree.Element: The transformation result in the specified output format.

    Raises:
        HTTPException: If the XSLT name is not found in the data keys.
        HTTPException: If the content type of the submitted document is not supported.
        HTTPException: If the output format is not valid.
    """
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
    """
    Endpoint to transform a document from a given URL using a specified XSLT and output format.

    Args:
        xslt_name (str): The name of the XSLT to be used for transformation.
        source_url (str): The URL from which to fetch the document to be transformed.
        output_format (Union[str | None]): The desired output format (json or xml). Defaults to None.

    Returns:
        dict or Response: The transformation result in the specified output format.

    Raises:
        HTTPException: If the request to the source URL fails.
        HTTPException: If the response status code from the URL is not 200.
        HTTPException: If the XSLT name is not found in the data keys.
        HTTPException: If the content type of the document is not supported.
    """
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


@router.post("/transform-jsonld-to-rdf", tags=['Transform'], name='Transform json-ld to RDF-XML format.'
    , description='The output will be in RDF-XML format.')
async def transform(submitted_json: Request):
    """
    Endpoint to transform a submitted JSON-LD document to RDF-XML format.

    Args:
        submitted_json (Request): The request object containing the JSON-LD document to be transformed.

    Returns:
        dict: A dictionary containing the transformation result in RDF-XML format.

    Raises:
        HTTPException: If the content type of the submitted document is not 'application/json+ld'.
    """
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
    """
    Endpoint to transform a submitted JSON-LD document to a specified RDF output format.

    Args:
        output_format (RdfOutputFormat): The desired RDF output format.
        submitted_json (Request): The request object containing the JSON-LD document to be transformed.

    Returns:
        dict: A dictionary containing the transformation result in the specified RDF format.

    Raises:
        HTTPException: If the content type of the submitted document is not 'application/json+ld'.
    """
    if submitted_json.headers['Content-Type'] not in ['application/json+ld']:
        raise HTTPException(status_code=400,
                            detail=f"Content type {submitted_json.headers['Content-Type']} not supported")
    submitted_json = await submitted_json.json()
    result = await transform_to_rdf(json.dumps(submitted_json), output_format.value)

    return {"result": result}

@router.post("/transform-jsonld-to-rdf/{output_format}/{source_url:path}", tags=['Transform'],
             name='Transform json-ld to a given RDF output format.')
async def transform(output_format: RdfOutputFormat, source_url):
    """
    Endpoint to transform a JSON-LD document from a given URL to a specified RDF output format.

    Args:
        output_format (RdfOutputFormat): The desired RDF output format.
        source_url (str): The URL from which to fetch the JSON-LD document.

    Returns:
        dict: A dictionary containing the transformation result in the specified RDF format.

    Raises:
        HTTPException: If the response status code from the URL is not 200.
    """
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
    """
    Endpoint to transform a submitted XML document to JSON format.

    Args:
        submitted_xml (Request): The request object containing the XML document to be transformed.
        clean_output (bool | None): Flag indicating whether to clean the output JSON. Defaults to False.

    Returns:
        dict: A dictionary containing the transformation result in JSON format.

    Raises:
        HTTPException: If the content type of the submitted document is not 'application/xml'.
    """
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


async def ceate_executable_xslt(s_xsl):
    #TODO: Implement this function
    pass


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
