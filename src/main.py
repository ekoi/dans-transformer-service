import saxonc
from fastapi import FastAPI
import uvicorn
app = FastAPI()
data = {}

@app.on_event('startup')
def init_data():
    print("startup")
    with saxonc.PySaxonProcessor(license=False) as proc:

        xsltproc = proc.new_xslt30_processor()
        executable = xsltproc.compile_stylesheet(
            stylesheet_file="../resources/examples/test.xsl")
        data.update({"xslt_executable": executable})

    return data

@app.get('/')
def info():
    result = data["xslt_executable"].transform_to_string(source_file="../resources/examples/cat.xml")
    print(result)
    return result

if __name__ == "__main__":

    uvicorn.run("src.main:app", host="0.0.0.0", port=8024, reload=True)
