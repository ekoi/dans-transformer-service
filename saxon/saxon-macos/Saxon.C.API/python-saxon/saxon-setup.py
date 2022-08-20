from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
from Cython.Build import cythonize
from glob import glob

sourcefiles = ["saxonc.pyx", "../SaxonProcessor.cpp", "../SaxonApiException.cpp", "../SaxonCGlue.c", "../SaxonCXPath.c", "../XdmValue.cpp", "../XdmItem.cpp", "../XdmNode.cpp", "../XdmAtomicValue.cpp", "../XdmMap.cpp", "../XdmArray.cpp", "../XdmFunctionItem.cpp", "../Xslt30Processor.cpp", "../XsltExecutable.cpp", "../XQueryProcessor.cpp","../XPathProcessor.cpp","../SchemaValidator.cpp", "../DocumentBuilder.cpp" ]

extensions = [Extension("saxonc", sourcefiles, language="c++", include_dirs = ['../jni', "../jni/unix"])]

setup(ext_modules=cythonize(extensions),

cmdclass = {'build_ext': build_ext},
      compiler_directives={'language_level' : 3})
                                                        
