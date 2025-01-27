import logging
import logging
import os
from contextlib import asynccontextmanager
from logging.handlers import RotatingFileHandler

import uvicorn
from akmi_utils import otel, logging as akmi_logging
from fastapi import FastAPI, HTTPException, Depends, status, Request
from fastapi.security import HTTPBearer
from starlette.middleware.cors import CORSMiddleware

from src import protected, public
from src.commons import settings, initialize_xslt_proc, initialize_templates, data, \
    project_details

api_keys = [
    settings.DANS_TRANSFORMER_SERVICE_API_KEY
]  # Todo: This is encrypted in the .secrets.toml

security = HTTPBearer()

APP_NAME = os.environ.get("APP_NAME", project_details['title'])
EXPOSE_PORT = os.environ.get("EXPOSE_PORT", 1745)
OTLP_GRPC_ENDPOINT = os.environ.get("OTLP_GRPC_ENDPOINT", "http://localhost:4317")

def api_key_auth(api_key: str = Depends(security)):
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


app = FastAPI(title= project_details['title'], description = project_details['description'],
              version= project_details['version'], lifespan=lifespan)

LOG_FILE = settings.LOG_FILE
print(f'---------------LOG_FILE: {LOG_FILE}')

app.add_middleware(otel.PrometheusMiddleware, app_name=APP_NAME)
app.add_route("/metrics", otel.metrics)

otel.setting_otlp(app, APP_NAME, OTLP_GRPC_ENDPOINT)


@app.middleware("http")
async def log_requests_middleware(request: Request, call_next):
    return await akmi_logging.log_requests(request, call_next)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)
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

logging.getLogger("uvicorn.access").addFilter(otel.MetricsEndpointFilter())
logging.getLogger("uvicorn.access").addFilter(otel.TraceContextFilter())
@app.get('/info')
def info():
    logging.info('info')
    return project_details


if __name__ == "__main__":

    log_config = uvicorn.config.LOGGING_CONFIG
    log_config["formatters"]["access"]["fmt"] = (
        "%(asctime)s %(levelname)s [%(name)s] [%(filename)s:%(lineno)d] [%(funcName)s] "
        "[trace_id=%(otelTraceID)s span_id=%(otelSpanID)s resource.service.name=%(otelServiceName)s] - %(message)s"
    )

    file_handler = RotatingFileHandler(LOG_FILE, maxBytes=10 * 1024 * 1024, backupCount=10)
    file_handler.setFormatter(logging.Formatter(log_config["formatters"]["access"]["fmt"]))
    print(f'---------------LOG_FILE: {LOG_FILE}')
    logging.getLogger().addHandler(file_handler)
    # Set the logging level for h11 to ERROR
    logging.getLogger("h11").setLevel(logging.ERROR)
    file_handler.setLevel(logging.INFO)

    uvicorn.run(app, host="0.0.0.0", port=EXPOSE_PORT, log_config=log_config)
