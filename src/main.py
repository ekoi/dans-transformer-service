import importlib.metadata
import logging
import os
from os.path import exists

import jinja2
import saxonc
import uvicorn
from fastapi import FastAPI, HTTPException, Depends, status, Query
from fastapi.security import OAuth2PasswordBearer

__version__ = importlib.metadata.metadata("dans-transformer-service")["version"]

from jproperties import Properties

from src import common, protected, public
from src.common import settings

api_keys = [
    settings.DANS_TRANSFORMER_SERVICE_API_KEY
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
    public.router,
    tags=["Public"],
    prefix=""
)

app.include_router(
    protected.router,
    tags=["Protected"],
    prefix="",
    dependencies=[Depends(api_key_auth)]
)

@app.on_event('startup')
def common_data():
    logging.debug("startup")
    iterate_saved_jinja_and_props()
    with saxonc.PySaxonProcessor(license=False) as proc:
        logging.debug(proc.version)
        xslt_proc = proc.new_xslt30_processor()
        iterate_saved_xsl_dir(xslt_proc)
    return common.data


def iterate_saved_jinja_and_props():
    templateLoader = jinja2.FileSystemLoader(searchpath=settings.JINJA_AND_PROP_DIR)
    templateEnv = jinja2.Environment(loader=templateLoader)
    for jinja_template_fname in os.listdir(settings.JINJA_AND_PROP_DIR):
        # jinja templates and mapping properties need to be existed in couple
        # Eg. datacite-jinja_templates.txt has to be coupled with datacite-jsonpathfinder_mapping.properties
        if jinja_template_fname.endswith("-jinja_templates.txt"):
            logging.debug(jinja_template_fname)  # logging.debuging file name of desired extension
            # rel_path_jinja_template_filename = os.path.join(settings.JINJA_AND_PROP_DIR, jinja_template_filename)
            jinja_json_template = templateEnv.get_template(jinja_template_fname)
            jsonpath_prop_fname = jinja_template_fname.replace("-jinja_templates.txt", "-jsonpathfinder_mapping.properties")
            rel_path_jsonpath_prop_fname = os.path.join(settings.JINJA_AND_PROP_DIR, jsonpath_prop_fname)
            if not exists(rel_path_jsonpath_prop_fname):
                logging.error(f"{rel_path_jsonpath_prop_fname} doesn't exist.")
            else:
                configs = Properties()
                with open(rel_path_jsonpath_prop_fname, 'rb') as read_prop:
                    configs.load(read_prop)
                common.data.update({jinja_template_fname: jinja_json_template})
                common.data.update({jsonpath_prop_fname: configs.items()})
        else:
            continue


def iterate_saved_xsl_dir(xslt_proc):
    for filename in os.listdir(settings.SAVED_XSLT_DIR):
        if filename.endswith(".xsl"):
            logging.debug(filename)  # logging.debuging file name of desired extension
            f = os.path.join(settings.SAVED_XSLT_DIR, filename)
            executable = xslt_proc.compile_stylesheet(stylesheet_file=f)
            common.data.update({filename: executable})
        else:
            continue


if __name__ == "__main__":
    uvicorn.run("src.main:app", host="0.0.0.0", port=1745, reload=False)
