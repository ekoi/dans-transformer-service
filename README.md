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

For PyCharm:
add SAXONC_HOME and PYTHONPATH on your environment

poetry build; docker rm -f dans-transformer-service; docker rmi ekoindarto/dans-transformer-service:0.1.0; docker build -t ekoindarto/dans-transformer-service:0.1.0 -f Dockerfile . ;docker run -d -p 8024:8024 --name dans-transformer-service ekoindarto/dans-transformer-service:0.1.0; docker exec -it dans-transformer-service /bin/bash
