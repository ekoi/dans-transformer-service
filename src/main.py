import importlib.metadata
import logging
import os
from os.path import exists

import saxonc
import uvicorn
from fastapi import FastAPI, HTTPException, Depends, status, Query
from fastapi.security import OAuth2PasswordBearer

__version__ = importlib.metadata.metadata("dans-transformer-service")["version"]

from src import common, protected
from src.common import settings

api_keys = [
    settings.SERVICE_API_KEY
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


app = FastAPI(title=settings.FASTAPI_TITLE, description=settings.FASTAPI_DESCRIPTION,
              version=__version__)


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

        for filename in os.listdir(common.saved_xslt_dir):
            if filename.endswith(common.saved_xslt_ext_file):
                logging.debug(filename)  # logging.debuging file name of desired extension
                f = os.path.join(common.saved_xslt_dir, filename)
                executable = xslt_proc.compile_stylesheet(stylesheet_file=f)
                common.data.update({filename.replace(common.saved_xslt_ext_file, ""): executable})
            else:
                continue
    return common.data


@app.get('/')
def info():
    return {"name": "DANS Transformer Service", "version": __version__}


@app.get('/saved-xsl-list', description="List of saved xsl.")
def get_saved_xslt_list(xslt_name: str | None = Query(default=None, description="if not provide, all saved list will be given.", max_length=25)):
    xslt_list = {}
    if xslt_name:
        fname = os.path.join(common.saved_xslt_dir, xslt_name + common.saved_xslt_ext_file)
        if not exists(fname):
            raise HTTPException(status_code=500, detail=f'{xslt_name} not found')
        else:
            with open(fname) as s:
                text = s.read()
                xslt_list.update({xslt_name: text})

    else:
        for filename in os.listdir(common.saved_xslt_dir):
            if filename.endswith(common.saved_xslt_ext_file):
                logging.debug(filename)  # logging.debuging file name of desired extension
                f = os.path.join(common.saved_xslt_dir, filename)
                with open(f) as s:
                    text = s.read()
                    xslt_list.update({filename.replace(common.saved_xslt_ext_file, ""): text})
            else:
                continue
    return xslt_list


if __name__ == "__main__":
    uvicorn.run("src.main:app", host="0.0.0.0", port=1745, reload=False)
