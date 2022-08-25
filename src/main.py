import requests
import saxonc
import os
from os.path import exists
import logging
from fastapi import FastAPI, HTTPException, Depends, status, Query
from fastapi.security import OAuth2PasswordBearer
import uvicorn

from dynaconf import Dynaconf

import importlib.metadata

__version__ = importlib.metadata.metadata("dans-transformer-service")["version"]

from src import common, protected
from src.common import settings
from src.protected import process_xsl

api_keys = [
    settings.API_KEY
]  # Todo: This is encrypted in the .secrets.toml

#Authorization Form: It doesn't matter what you type in the form, it won't work yet. But we'll get there.
#See: https://fastapi.tiangolo.com/tutorial/security/first-steps/
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")  # use token authentication


def api_key_auth(api_key: str = Depends(oauth2_scheme)):
    if api_key not in api_keys:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Forbidden"
        )


app = FastAPI()


app.include_router(
    protected.router,
    prefix="",
    dependencies=[Depends(api_key_auth)]
)

@app.on_event('startup')
def common_data():
    logging.debug("startup")
    with saxonc.PySaxonProcessor(license=False) as proc:
        logging.debug(proc.version)
        xslt_proc = proc.new_xslt30_processor()

        for filename in os.listdir(common.saved_xsl_dir):
            if filename.endswith(common.saved_xsl_ext_file):
                logging.debug(filename)  # logging.debuging file name of desired extension
                f = os.path.join(common.saved_xsl_dir, filename)
                executable = xslt_proc.compile_stylesheet(stylesheet_file=f)
                common.data.update({filename.replace(common.saved_xsl_ext_file, ""): executable})
            else:
                continue
    return common.data


@app.get('/')
def info():
    return {"name": "DANS Transformer Service", "version": __version__}


@app.get('/saved-xsl-list', description="List of saved xsl.")
def get_saved_xsl_list(xsl_name: str | None = Query(default=None, description="if not provide, all saved list will be given.", max_length=25)):
    xsl_list = {}
    if xsl_name:
        fname = os.path.join(common.saved_xsl_dir, xsl_name + common.saved_xsl_ext_file)
        if not exists(fname):
            raise HTTPException(status_code=500, detail=f'{xsl_name} not found')
        else:
            with open(fname) as s:
                text = s.read()
                xsl_list.update({xsl_name: text})

    else:
        for filename in os.listdir(common.saved_xsl_dir):
            if filename.endswith(common.saved_xsl_ext_file):
                logging.debug(filename)  # logging.debuging file name of desired extension
                f = os.path.join(common.saved_xsl_dir, filename)
                with open(f) as s:
                    text = s.read()
                    xsl_list.update({filename.replace(common.saved_xsl_ext_file, ""): text})
            else:
                continue
    return xsl_list


if __name__ == "__main__":
    uvicorn.run("src.main:app", host="0.0.0.0", port=1745, reload=False)
