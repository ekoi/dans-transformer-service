FROM python:3.12.0-bookworm

ARG VERSION=0.5.9

RUN useradd -ms /bin/bash dans

USER dans
WORKDIR /home/dans

ENV PYTHONPATH=/home/dans/dans-transformer-service:
#/home/dans/dans-transformer-service/saxon/saxon-linux/libsaxon-HEC-11.4/Saxon.C.API/python-saxon
ENV BASE_DIR=/home/dans/dans-transformer-service
#ENV SAXONC_HOME=/home/dans/dans-transformer-service/saxon/saxon-linux/libsaxon-HEC-11.4
RUN mkdir -p ${BASE_DIR}
COPY ./dist/*.* .
#COPY ./dist/dans_transformer_service-${VERSION}.tar.gz .
#COPY ./settings.toml ${BASE_DIR}
RUN mkdir -p ${BASE_DIR} && \
    pip install --no-cache-dir *.whl && rm -rf *.whl && \
    tar xf dans_transformer_service-${VERSION}.tar.gz -C ${BASE_DIR} --strip-components 1

#
#RUN tar -xzvf dans_transformer_service-${VERSION}.tar.gz -C ${BASE_DIR} --strip-components=1 && \
#    cd ${BASE_DIR} && poetry config virtualenvs.create false && poetry install --no-dev
#    cd ${BASE_DIR}/saxon/saxon-linux && unzip libsaxon-HEC-setup64-v11.4.zip && \
#    cd ${BASE_DIR}/saxon/saxon-linux/libsaxon-HEC-11.4/samples/cppTests &&  \
#    ./build64-linux.sh && ./testXPath && \
#    cd ${BASE_DIR}/saxon/saxon-linux/libsaxon-HEC-11.4/Saxon.C.API/python-saxon && python3 saxon-setup.py build_ext -if


#COPY ./conf/.secrets.toml ${BASE_DIR}/conf

#COPY ./saved-xsl/* ${BASE_DIR}/saved-xsl
WORKDIR ${BASE_DIR}
CMD ["python", "src/main.py"]
#CMD ["tail", "-f", "/dev/null"]


#RUN mkdir -p ${BASE_DIR} && tar -xzvf dans-transformer-service-${VERSION}.tar.gz -C ${BASE_DIR} --strip-components=1 && \
#    cd ${BASE_DIR} && poetry config virtualenvs.create false && poetry install --no-dev && \
#    cd ${BASE_DIR}/saxon/saxon-linux && unzip libsaxon-HEC-setup64-v11.4.zip && \
#    cd ${BASE_DIR}/saxon/saxon-linux/libsaxon-HEC-11.4/samples/cppTests &&  \
#    ./build64-linux.sh && ./testXPath && \
#    cd ${BASE_DIR}/saxon/saxon-linux/libsaxon-HEC-11.4/Saxon.C.API/python-saxon && python3 saxon-setup.py build_ext -if
