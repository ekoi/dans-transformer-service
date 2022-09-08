import json
import shutil
import logging
import os
import requests
import xml.etree.ElementTree as ET
# import codecs
from fastapi import APIRouter, Request, HTTPException

from src.common import ceate_executable_xslt, data, settings

router = APIRouter()


@router.get("/ping")
async def say_hi(name: str):
    return "Hi " + name


@router.get("/settings")
async def get_settings():
    return settings


@router.post('/submit-xsl/{xslt_name}/{save}', status_code=201)
async def submit_xsl(xslt_name: str, submitted_xsl: Request, save: bool | None = False):
    if not xslt_name.endswith(".xsl"):
        logging.error(f"{xslt_name} should be something like 'any-name.xsl' ")
        raise HTTPException(status_code=500, detail=f"Content type {xslt_name} not supported. It should be something like 'any-name.xsl' ")

    content_type = submitted_xsl.headers['Content-Type']
    s_xsl = ""
    if content_type == 'application/xml':
        s_xsl = await submitted_xsl.body()
    else:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')

    msg = await process_xsl(s_xsl, save, xslt_name)

    return {"message": msg}


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


@router.post("/transform-json/{template_name}")
async def transform(template_name: str, submitted_json: Request):
    raise HTTPException(status_code=501, detail=f'This eindpoint is not implemented yet.')


@router.post("/transform/{xslt_name}")
async def transform(xslt_name: str, submitted_json_or_xml: Request):
    content_type = submitted_json_or_xml.headers['Content-Type']
    str_submitted_xml = ""
    if xslt_name not in data.keys():
        raise HTTPException(status_code=500, detail=f'The given xslt_name: "{xslt_name}" is not found')

    if content_type in ['application/json', 'application/xml']:
        if content_type == 'application/json':
            try:
                submitted_json = await submitted_json_or_xml.json()
                # submitted_json = {k: v.replace("&","&#38;").replace("<","&#60;") for k, v in submitted_json.items()}
                submitted_json_str = json.dumps(submitted_json)
                # saxon needs json that encapsulates in xml
                str_submitted_xml = '<data>' + submitted_json_str + '</data>'
                # write the xml to a temporary file
                with open(settings.TEMP_TRANSFORM_FILE, mode="w") as file:
                    file.write(str_submitted_xml)
                etree = ET.parse(settings.TEMP_TRANSFORM_FILE)
            except ET.ParseError as pe:
                logging.debug(pe)
                shutil.copyfile(settings.TEMP_TRANSFORM_FILE, settings.TEMP_TRANSFORM_FILE + "-error-tobe_converted")
                str_submitted_xml = remove_xml_invalid_characters(submitted_json_str)
            except ValueError as err:
                logging.debug(err)
                raise HTTPException(status_code=500, detail=f'Submitted json is not valid. {err}')
        else:
            submitted_xml = await submitted_json_or_xml.body()
            try:
                tree = ET.parse(submitted_xml)
                s_submitted = submitted_xml
            except ValueError as err:
                logging.debug(err)
                raise HTTPException(status_code=500, detail=f'Submitted XML is not valid. {err}')

    else:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')

    with open(settings.TEMP_TRANSFORM_FILE, mode="w") as file:
        file.write(str_submitted_xml)
    # file = codecs.open(settings.TEMP_TRANSFORM_FILE, "w", "utf-8")
    # file.write(str_submitted_xml)
    # file.close()
    result = data[xslt_name].transform_to_string(source_file=settings.TEMP_TRANSFORM_FILE)
    if result is None:
        logging(f'Empty result, submitted_json: {submitted_json}')
        raise HTTPException(status_code=500, detail=f'Empty result, submitted_json: {submitted_json}')

    logging.debug(result)
    return {"result": result}


#
def remove_xml_invalid_characters(str_json):
    str_xml = "<data>" + str_json.replace("&", "&#38;").replace("<", "&#60;") + "</data>"
    # write the xml to a temporary file
    with open(settings.TEMP_TRANSFORM_FILE, mode="w") as file:
        file.write(str_xml)
    try:
        etree = ET.parse(settings.TEMP_TRANSFORM_FILE)
        return str_xml
    except ET.ParseError as pe:
        logging.debug(pe)
        shutil.copyfile(settings.TEMP_TRANSFORM_FILE, settings.TEMP_TRANSFORM_FILE + "-ERROR-converted-fail")
        raise HTTPException(status_code=500, detail=f'Transformed json is not valid. {pe}')


@router.post('/submit-xsl/{xslt_name}/{url:path}/{save}', status_code=201)
async def submit_xslt_from_url(xslt_name: str, url: str, save: bool | None = False):
    response = requests.get(url)
    if response.status_code != 200:
        raise HTTPException(status_code=response.status_code,
                            detail=f'Retrieve response code {response.status_code} from {url}')
    else:
        xsl = response.text
        msg = await process_xsl(xsl, save, xslt_name)
        return {"message": msg}


@router.get("/settings")
async def get_settings():
    return settings


@router.delete("/delete-saved-xsl/{xslt_name}", status_code=204)
def delete_saved_xsl(xslt_name: str):
    raise HTTPException(status_code=501, detail=f'This eindpoint is not implemented yet.')
