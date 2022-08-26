FROM python:3.10.6-slim-bullseye

ARG VERSION=0.2.5

RUN  apt-get update -y && \
     apt-get upgrade -y && \
     apt-get dist-upgrade -y && \
     apt-get -y autoremove && \
     apt-get install -y unzip && \
     apt-get install -y gcc && \
     apt-get install -y g++ && \
     apt-get clean && \
     useradd -ms /bin/bash dans

USER dans

ENV PYTHONPATH=/home/dans/dans-transformer-service/src:/home/dans/dans-transformer-service/saxon/saxon-linux/libsaxon-HEC-11.4/Saxon.C.API/python-saxon
ENV BASE_DIR=/home/dans/dans-transformer-service
ENV SAXONC_HOME=/home/dans/dans-transformer-service/saxon/saxon-linux/libsaxon-HEC-11.4
RUN mkdir -p ${BASE_DIR}

WORKDIR /home/dans/dans-transformer-service/src

COPY ./dist/*.* .
#COPY ./settings.toml ${BASE_DIR}
#
RUN mkdir -p ${BASE_DIR} && \
    pip install --no-cache-dir *.whl && rm -rf *.whl && \
    tar xf dans-transformer-service-${VERSION}.tar.gz -C ${BASE_DIR} --strip-components 1 &&\
    cd /home/dans/dans-transformer-service/saxon/saxon-linux && unzip libsaxon-HEC-setup64-v11.4.zip&& \
    cd /home/dans/dans-transformer-service/saxon/saxon-linux/libsaxon-HEC-11.4/samples/cppTests &&  \
    ./build64-linux.sh && ./testXPath && \
    cd /home/dans/dans-transformer-service/saxon/saxon-linux/libsaxon-HEC-11.4/Saxon.C.API/python-saxon && python3 saxon-setup.py build_ext -if


COPY ./src/.secrets.toml ${BASE_DIR}/.secrets.toml

CMD ["python", "main.py"] \
#CMD ["tail", "-f", "/dev/null"]
