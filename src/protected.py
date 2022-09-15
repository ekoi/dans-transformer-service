import json
import shutil
import uuid
import logging
import os
import requests
import xml.etree.ElementTree as ET

from rdflib import Graph

# import codecs
from fastapi import APIRouter, Request, HTTPException

from src.common import ceate_executable_xslt, data, settings, validate_json, RdfOutputFormat

router = APIRouter()


@router.post('/upload-xsl/{xslt_name}/{save}', status_code=201, tags=['Upload XSLT'])
async def submit_xsl(xslt_name: str, submitted_xsl: Request, save: bool | None = False):
    if not xslt_name.endswith(".xsl"):
        logging.error(f"{xslt_name} should be something like 'any-name.xsl' ")
        raise HTTPException(status_code=500,
                            detail=f"Content type {xslt_name} not supported. It should be something like 'any-name.xsl' ")

    content_type = submitted_xsl.headers['Content-Type']
    s_xsl = ""
    if content_type == 'application/xml':
        s_xsl = await submitted_xsl.body()
    else:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')

    msg = await process_xsl(s_xsl, save, xslt_name)

    return {"message": msg}


@router.post('/upload-xsl/{xslt_name}/{xsl_url:path}/{save}', status_code=201, tags=['Upload XSLT'])
async def submit_xslt_from_url(xslt_name: str, xsl_url: str, save: bool | None = False):
    response = requests.get(xsl_url)
    if response.status_code != 200:
        raise HTTPException(status_code=response.status_code,
                            detail=f'Retrieve response code {response.status_code} from {xsl_url}')
    else:
        xsl = response.text
        msg = await process_xsl(xsl, save, xslt_name)
        return {"message": msg}


@router.post("/transform/{xslt_name}/{source_url:path}", tags=['Transform'])
async def transform(xslt_name: str, source_url):
    response = requests.get(source_url)
    if response.status_code != 200:
        raise HTTPException(status_code=response.status_code,
                            detail=f'Retrieve response code {response.status_code} from {source_url}')

    if xslt_name not in data.keys():
        raise HTTPException(status_code=500, detail=f'The given xslt_name: "{xslt_name}" is not found')

    content_type = response.headers['Content-Type']
    if content_type in ['application/json', 'application/xml', 'application/json;charset=UTF-8']:
        temp_file = settings.TEMP_TRANSFORM_FILE + "-" + str(uuid.uuid1()) + ".xml"
        if content_type in ['application/json', 'application/json;charset=UTF-8']:
            submitted_json = json.loads(response.content.decode("UTF-8"))
            str_xml = await validate_xml_encapsulated_json(submitted_json, temp_file)
        else:
            submitted_xml = await response.content.decode("UTF-8")
            str_xml = await validate_submitted_xml(submitted_xml)

    else:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')

    result = await transform_to_string(str_xml, submitted_json, temp_file, xslt_name)
    return {"result": result}


@router.post("/transform/{xslt_name}", tags=['Transform'])
async def transform(xslt_name: str, submitted_json_or_xml: Request):
    content_type = submitted_json_or_xml.headers['Content-Type']
    str_xml = ""
    if xslt_name not in data.keys():
        raise HTTPException(status_code=500, detail=f'The given xslt_name: "{xslt_name}" is not found')

    if content_type in ['application/json', 'application/xml']:
        temp_file = settings.TEMP_TRANSFORM_FILE + "-" + str(uuid.uuid1()) + ".xml"
        if content_type == 'application/json':
            submitted_json = await submitted_json_or_xml.json()
            str_xml = await validate_xml_encapsulated_json(submitted_json, temp_file)
        else:
            submitted_xml = await submitted_json_or_xml.body()
            str_xml = await validate_submitted_xml(submitted_xml)

    else:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')

    result = await transform_to_string(str_xml, submitted_json, temp_file, xslt_name)
    return {"result": result}


@router.post("/transform-jsonld-to-rdf", tags=['Transform'], name='Transform json-ld to RDF-XML format.'
    , description='The output will be in RDF-XML format.')
async def transform(submitted_json: Request):
    content_type = submitted_json.headers['Content-Type']
    if content_type not in ['application/json+ld']:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')
    submitted_json = await submitted_json.json()
    result = await transform_to_rdf(json.dumps(submitted_json))

    return {"result": result}


@router.post("/transform-jsonld-to-rdf/{output_format}", tags=['Transform'], name='Transform json-ld to a given RDF output format.'
    , description='The output will be in RDF-XML format.')
async def transform(output_format: RdfOutputFormat, submitted_json: Request):
    if submitted_json.headers['Content-Type'] not in ['application/json+ld']:
        raise HTTPException(status_code=400, detail=f"Content type {submitted_json.headers['Content-Type']} not supported")
    submitted_json = await submitted_json.json()
    result = await transform_to_rdf(json.dumps(submitted_json), output_format.value)

    return {"result": result}


@router.post("/transform-jsonld-to-rdf/{output_format}/{source_url:path}", tags=['Transform'], name='Transform json-ld to a given RDF output format.')
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

@router.get("/ping", tags=['Other'])
async def say_hi(name: str):
    return "Hi " + name


@router.get("/settings", tags=['Other'])
async def get_settings():
    return settings


@router.delete("/delete-saved-xsl/{xslt_name}", status_code=204, tags=['Danger Zone'])
def delete_saved_xsl(xslt_name: str):
    raise HTTPException(status_code=501, detail=f'This endpoint is not implemented yet.')


async def process_xsl(s_xsl, save, xslt_name):
    executable_xslt = await ceate_executable_xslt(s_xsl)
    msg = xslt_name + " is active and saved."
    if not save:
        msg = xslt_name + " is active but not saved. It wil destroy when the service is down."
    else:
        if not isinstance(s_xsl, str):
            s_xsl = s_xsl.decode('UTF-8')
        with open(os.path.join(settings.SAVED_XSLT_DIR, xslt_name), mode="w") as file:
            file.write(s_xsl)
    data.update({xslt_name: executable_xslt})
    return msg


async def transform_to_string(str_xml, submitted_json, temp_file, xslt_name):
    with open(temp_file, mode="w") as file:
        file.write(str_xml)
    # file = codecs.open(temp_file, "w", "utf-8")
    # file.write(str_submitted_xml)
    # file.close()
    result = data[xslt_name].transform_to_string(source_file=temp_file)
    if result is None:
        logging(f'Empty result, submitted_json: {submitted_json}')
        raise HTTPException(status_code=500, detail=f'Empty result, submitted_json: {submitted_json}')
    try:
        os.remove(temp_file)
    except:
        logging.error(f'Error while deleting file {temp_file}')
    logging.debug(result)
    return result


async def validate_submitted_xml(submitted_xml):
    try:
        tree = ET.parse(submitted_xml)
        return submitted_xml
    except ValueError as err:
        logging.debug(err)
        raise HTTPException(status_code=500, detail=f'Submitted XML is not valid. {err}')


async def validate_xml_encapsulated_json(submitted_json, temp_file):
    try:
        # submitted_json = {k: v.replace("&","&#38;").replace("<","&#60;") for k, v in submitted_json.items()}
        submitted_json_str = json.dumps(submitted_json)
        # saxon needs json that encapsulates in xml
        str_xml_encapsulated_json = '<data>' + submitted_json_str + '</data>'
        # write the xml to a temporary file
        with open(temp_file, mode="w") as file:
            file.write(str_xml_encapsulated_json)
        etree = ET.parse(temp_file)
        return str_xml_encapsulated_json
    except ET.ParseError as pe:
        logging.debug(pe)
        shutil.copyfile(temp_file, temp_file + "-error-tobe_converted")
        return remove_xml_invalid_characters(submitted_json_str, temp_file)
    except ValueError as err:
        logging.debug(err)
        raise HTTPException(status_code=500, detail=f'Submitted json is not valid. {err}')


#
def remove_xml_invalid_characters(str_json, temp_file):
    str_xml = "<data>" + str_json.replace("&", "&#38;").replace("<", "&#60;") + "</data>"
    # write the xml to a temporary file
    with open(temp_file, mode="w") as file:
        file.write(str_xml)
    try:
        etree = ET.parse(temp_file)
        return str_xml
    except ET.ParseError as pe:
        logging.debug(pe)
        shutil.copyfile(temp_file, temp_file + "-ERROR-converted-fail")
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


