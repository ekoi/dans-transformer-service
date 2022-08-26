import logging

import saxonc
import json

from dynaconf import Dynaconf

settings = Dynaconf(settings_files=["conf/settings.toml", "conf/.secrets.toml"],
                    environments=True)

logging.basicConfig(filename=settings.LOG_FILE, level=settings.LOG_LEVEL,
                    format=settings.LOG_FORMAT)

data = {}
saved_xsl_dir = "../saved-xsl"
saved_xsl_ext_file = ".xsl"

def validate_json(jsonData):
    try:
        json.loads(jsonData)
    except ValueError as err:
        logging.debug(err)
        return False
    return True



async def ceate_executable_xslt(s_xsl):
    with saxonc.PySaxonProcessor(license=False) as proc:
        logging.debug(proc.version)
        xsltproc = proc.new_xslt30_processor()

        if not isinstance(s_xsl, str):
            s_xsl = s_xsl.decode('UTF-8')

        executable = xsltproc.compile_stylesheet(
            stylesheet_text=s_xsl)
        return executable

    raise HTTPException(status_code=500, detail=f'Invalid submitted xsl.')
