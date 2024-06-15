import logging
import os
from os.path import exists

from fastapi import HTTPException, Query, APIRouter

import src
from src.commons import settings, data

router = APIRouter()


@router.get('/saved-xsl-list', description="List of saved xsl.")
def get_saved_xslt_list(
        xslt_name: str | None = Query(default=None, description="if not provide, all saved list will be given.",
                                      max_length=100)):
    xslt_list = {}
    if xslt_name:
        fname = os.path.join(settings.SAVED_XSLT_DIR, xslt_name)
        if not exists(fname):
            raise HTTPException(status_code=500, detail=f'{xslt_name} not found')
        else:
            with open(fname) as s:
                text = s.read()
                xslt_list.update({xslt_name: text})

    else:
        for filename in os.listdir(settings.SAVED_XSLT_DIR):
            if filename.endswith(".xsl"):
                logging.debug(filename)  # logging.debuging file name of desired extension
                f = os.path.join(settings.SAVED_XSLT_DIR, filename)
                with open(f) as s:
                    text = s.read()
                    xslt_list.update({filename: text})
            else:
                continue
    return xslt_list


@router.get('/saved-xsl-list-only', description="List (only) of saved xsl.")
def get_saved_xslt_list():
    xslt_list = [k for k in data.keys() if k.endswith('.xsl')]
    # for filename in os.listdir(settings.SAVED_XSLT_DIR):
    #     if filename.endswith(".xsl"):
    #         logging.debug(filename)  # logging.debuging file name of desired extension
    #         f = os.path.join(settings.SAVED_XSLT_DIR, filename)
    #         xslt_list.append(filename)
    sorted_list = sorted(xslt_list)

    return sorted_list
