# dans-transformer-service
For the first time:
git clone https://github.com/ekoi/dans-transformer-service
cd dans-transformer-service
poetry install
poetry shell
poetry update

For macos:
export SAXONC_HOME=$YOUR_PROJECT_BASE_PATH/saxon/saxon-macos
export PYTHONPATH=$PYTHONPATH:$YOUR_PROJECT_BASE_PATH/saxon/saxon-macos/Saxon.C.API/python-saxon


cd $SAXONC_HOME 
unzip libsaxon-HEC-mac-setup-v11.4.zip
cd libsaxon-HEC-11.4
cp libsaxonhec.dylib $SAXONC_HOME
cp -r rt $SAXONC_HOME
cp -r saxon-data $SAXONC_HOME
cp -r Saxon.C.API $SAXONC_HOME
cd $SAXONC_HOME/Saxon.C.API/python-saxon
python3 saxon-setup.py build_ext -if
If it goes well, a build directory will be created in $SAXONC_HOME/Saxon.C.API/python-saxon


For PyCharm:
add SAXONC_HOME and PYTHONPATH on your environment

poetry build; docker rm -f dans-transformer-service; docker rmi ekoindarto/dans-transformer-service:0.3.5; docker build --no-cache -t ekoindarto/dans-transformer-service:0.3.5 -f Dockerfile . ;docker run -d -p 1745:1745 --name dans-transformer-service ekoindarto/dans-transformer-service:0.3.5; docker exec -it dans-transformer-service /bin/bash

curl -X POST -H "Content-Type: application/json" -d @/Users/akmi/git/ODISSEI/mapping-resources/examples/json/ext2.json http://0.0.0.0:1745/transform/cbs-datacite

curl -X 'POST' \
  'http://0.0.0.0:1745/submit-xsl2/abc/https%3A%2F%2Fraw.githubusercontent.com%2Fekoi%2Fmapping-resources%2Fmain%2Fexamples%2Fxsl%2Fdatacite.xsl/true' \
  -H 'accept: application/json' \
  -d ''