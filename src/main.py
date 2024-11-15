import importlib.metadata
import logging
from contextlib import asynccontextmanager

import uvicorn
from fastapi import FastAPI, HTTPException, Depends, status
from fastapi.security import OAuth2PasswordBearer


from src import protected, public
from src.commons import settings, initialize_xslt_proc, iterate_saved_jinja_and_props, initialize_templates, data, \
    get_version

api_keys = [
    settings.DANS_TRANSFORMER_SERVICE_API_KEY
]  # Todo: This is encrypted in the .secrets.toml

# Authorization Form: It doesn't matter what you type in the form, it won't work yet. But we'll get there.
# See: https://fastapi.tiangolo.com/tutorial/security/first-steps/
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")  # use token authentication


def api_key_auth(api_key: str = Depends(oauth2_scheme)):
    if api_key not in api_keys:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Forbidden"
        )


@asynccontextmanager
async def lifespan(application: FastAPI):
    """
    Lifespan event handler for the FastAPI application.

    This function is executed during the startup of the FastAPI application.
    It initializes the database, iterates through saved bridge module directories,
    and prints available bridge classes.

    Args:
        application (FastAPI): The FastAPI application.

    Yields:
        None: The context manager does not yield any value.

    """
    print('start up')
    data.clear()
    initialize_templates()
    initialize_xslt_proc()

    yield


app = FastAPI(title=settings.FASTAPI_TITLE, description=settings.FASTAPI_DESCRIPTION,
              version= get_version(), lifespan=lifespan)

app.include_router(
    public.router,
    tags=["Public"],
    prefix=""
)

app.include_router(
    protected.router,
    # tags=["Protected"],
    prefix="",
    dependencies=[Depends(api_key_auth)]
)


@app.get('/')
def info():
    logging.info('info')
    print('info')
    return {"name": "DANS Transformer Service", "version": get_version()}


if __name__ == "__main__":
    uvicorn.run("src.main:app", host="0.0.0.0", port=1745, reload=False)
