import logging
from enum import auto
from fastapi_utils.enums import StrEnum
import saxonc
import json

from dynaconf import Dynaconf

settings = Dynaconf(settings_files=["conf/settings.toml", "conf/.secrets.toml"],
                    environments=True)

logging.basicConfig(filename=settings.LOG_FILE, level=settings.LOG_LEVEL,
                    format=settings.LOG_FORMAT)

data = {}

class RdfOutputFormat(StrEnum):
    xml = auto()
    turtle = auto()
    n3 = auto()
    nt = auto()
    trix = auto()
    trig = auto()
    nquads = auto()
    hext = auto()


def validate_json(str_json):
    try:
        json.loads(str_json)
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
