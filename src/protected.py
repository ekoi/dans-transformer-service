import json
import logging
import os, requests
import xml.etree.ElementTree as ET

from fastapi import APIRouter, Request, HTTPException, status

from src import common
from src.common import ceate_executable_xslt, saved_xsl_dir, saved_xsl_ext_file, data, settings

router = APIRouter()

@router.get("/ping")
async def say_hi(name: str):
    return "Hi " + name

@router.post('/submit-xsl/{xsl_name}/{save}', status_code=201)
async def submit_xsl(xsl_name: str, submitted_xsl: Request, save: bool | None = False):
    content_type = submitted_xsl.headers['Content-Type']
    s_xsl = ""
    if content_type == 'application/xml':
        s_xsl = await submitted_xsl.body()
    else:
        raise HTTPException(status_code=400, detail=f'Content type {content_type} not supported')

    msg = await process_xsl(s_xsl, save, xsl_name)

    return {"message": msg}


async def process_xsl(s_xsl, save, xsl_name):
    executable_xslt = await ceate_executable_xslt(s_xsl)
    msg = xsl_name + " is active and saved."
    if not save:
        msg = xsl_name + " is active but not saved. It wil destroy when the service is down."
    else:
        if not isinstance(s_xsl, str):
            s_xsl = s_xsl.decode('UTF-8')
        with open(os.path.join(saved_xsl_dir, xsl_name + saved_xsl_ext_file), mode="w") as file:
            file.write(s_xsl)
    data.update({xsl_name: executable_xslt})
    return msg


@router.post("/transform/{xsl_name}")
async def transform(xsl_name: str, submitted_json_or_xml: Request):
    content_type = submitted_json_or_xml.headers['Content-Type']
    s_submitted = ""
    if xsl_name not in data.keys():
        raise HTTPException(status_code=500, detail=f'{xsl_name} not found')

    if content_type in ['application/json', 'application/xml']:
        if content_type == 'application/json':
            try:
                submitted_json = await submitted_json_or_xml.json()
                s_submitted = "<data>" + json.dumps(submitted_json) + "</data>"
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
        file.write(s_submitted)
    result = data[xsl_name].transform_to_string(source_file=settings.TEMP_TRANSFORM_FILE)
    logging.debug(result)
    return {"result": result}

@router.post('/submit-xsl/{xsl_name}/{url:path}/{save}', status_code=201)
async def submit_xsl_from_url(xsl_name: str, url: str, save: bool | None = False):
    response = requests.get(url)
    if response.status_code != 200:
        raise HTTPException(status_code=response.status_code, detail=f'Retrieve response code {response.status_code} from {url}')
    else:
        xsl = response.text
        msg = await process_xsl(xsl, save, xsl_name)
        return {"message": msg}


@router.delete("/delete-saved-xsl/{xsl_name}", status_code=204)
def delete_saved_xsl(xsl_name: str):
    raise HTTPException(status_code=501, detail=f'This eindpoint is not implemented yet.')
