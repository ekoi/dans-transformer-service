import glob
import logging
import os
from enum import auto
from os.path import exists
from xml.dom.minidom import parseString

import jinja2
import tomli
from fastapi_utils.enums import StrEnum
from jproperties import Properties
from saxonche import PySaxonProcessor, PyXdmValue
import json
import xml.etree.ElementTree as ET

from dynaconf import Dynaconf

settings = Dynaconf(settings_files=["conf/settings.toml", "conf/.secrets.toml"],
                    environments=True)

logging.basicConfig(filename=settings.LOG_FILE, level=settings.LOG_LEVEL,
                    format=settings.LOG_FORMAT)

data = {}

def get_version():
    with open(os.path.join(os.getenv("BASE_DIR"), 'pyproject.toml'), 'rb') as file:
        package_details = tomli.load(file)
    return package_details['tool']['poetry']['version']

def get_name():
    with open(os.path.join(os.getenv("BASE_DIR"), 'pyproject.toml'), 'rb') as file:
        package_details = tomli.load(file)
    return package_details['tool']['poetry']['name']

class RdfOutputFormat(StrEnum):
    xml = auto()
    turtle = auto()
    n3 = auto()
    nt = auto()
    trix = auto()
    trig = auto()
    nquads = auto()
    hext = auto()


class OutputFormat(StrEnum):
    xml = auto()
    json = auto()
    text = auto()


def validate_json(str_json):
    try:
        json.loads(str_json)
    except ValueError as err:
        logging.debug(err)
        return False
    return True


async def create_executable_xslt(s_xsl):
    with PySaxonProcessor(license=False) as proc:
        logging.debug(proc.version)
        xsltproc = proc.new_xslt30_processor()

        if not isinstance(s_xsl, str):
            s_xsl = s_xsl.decode('UTF-8')

        executable = xsltproc.compile_stylesheet(
            stylesheet_text=s_xsl)
        return executable

    # raise HTTPException(status_code=500, detail=f'Invalid submitted xsl.')


def prettify_xml(str_xml):
    reparsed = parseString(str_xml)
    return '\n'.join([line for line in reparsed.toprettyxml(indent=' ' * 2).split('\n') if line.strip()])


def iterate_saved_xsl_dir(xslt_proc):
    path = f"{settings.SAVED_XSLT_DIR}/**/*.xsl"
    xsl_files = glob.glob(path, recursive=True)
    for xslt_file in xsl_files:
        logging.debug(f'loading xsl... {xslt_file}')
        print(xslt_file)
        executable = xslt_proc.compile_stylesheet(stylesheet_file=xslt_file)
        data.update({os.path.basename(xslt_file): executable})


def initialize_xslt_proc():
    logging.debug("---initialize_xslt_proc---")
    with PySaxonProcessor(license=False) as proc:
        logging.debug(proc.version)
        xslt_proc = proc.new_xslt30_processor()
        xslt_proc.set_cwd(os.getcwd())
        iterate_saved_xsl_dir(xslt_proc)
    return data


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
            jsonpath_prop_fname = jinja_template_fname.replace("-jinja_templates.txt",
                                                               "-jsonpathfinder_mapping.properties")
            rel_path_jsonpath_prop_fname = os.path.join(settings.JINJA_AND_PROP_DIR, jsonpath_prop_fname)
            if not exists(rel_path_jsonpath_prop_fname):
                logging.error(f"{rel_path_jsonpath_prop_fname} doesn't exist.")
            else:
                configs = Properties()
                with open(rel_path_jsonpath_prop_fname, 'rb') as read_prop:
                    configs.load(read_prop)
                data.update({jinja_template_fname: jinja_json_template})
                data.update({jsonpath_prop_fname: configs.items()})
        else:
            continue


def initialize_templates():
    logging.debug("startup")
    iterate_saved_jinja_and_props()
    return initialize_xslt_proc

def execute_xsl(f_record_path, rating_model_json, xsl):
    with PySaxonProcessor(license=False) as proc:
        xsltproc = proc.new_xslt30_processor()
        xsltproc.set_cwd(os.getcwd())
        executable = xsltproc.compile_stylesheet(stylesheet_file=xsl)
        value = PyXdmValue()
        value.add_xdm_item(proc.make_string_value(rating_model_json))
        executable.set_parameter("json", value)
        result = executable.apply_templates_returning_string(source_file=f_record_path)
        print(result)
        # with open(f_record_path, "r+") as f:
        #     f.seek(0)
        #     f.write(result)