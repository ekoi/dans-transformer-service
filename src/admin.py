import logging
import os
from os.path import exists

from fastapi import HTTPException, Query, APIRouter
from starlette.requests import Request
from starlette.staticfiles import StaticFiles
from starlette.templating import Jinja2Templates

import src
from src import common


router = APIRouter()
templates = Jinja2Templates(directory='../admin-templates')
router.mount('/static', StaticFiles(directory='../static'), name='static')

@router.get('/home')
def eko(request: Request):
    return templates.TemplateResponse('home.html', context={'request': request,  'vers': src.main.__version__})
