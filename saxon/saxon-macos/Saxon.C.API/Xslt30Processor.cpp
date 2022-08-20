// Xslt30Processor.cpp : Defines the exported functions for the DLL application.
//

#include "Xslt30Processor.h"
#include "XdmValue.h"
#include "XdmItem.h"
#include "XdmNode.h"
#include "XdmAtomicValue.h"
#include "XdmFunctionItem.h"
#include "XdmMap.h"
#include "XdmArray.h"
//#include "XsltExecutable.h"
#ifdef DEBUG
#include <typeinfo> //used for testing only
#endif

#ifdef MEM_DEBUG
#define new new(__FILE__, __LINE__)
#endif

Xslt30Processor::Xslt30Processor() {

	SaxonProcessor *p = new SaxonProcessor(false);
	Xslt30Processor(p, "");

}

Xslt30Processor::Xslt30Processor(SaxonProcessor * p, std::string curr) {

	proc = p;
    jitCompilation = false;
    exception = nullptr;
	/*
	 * Look for class.
	 */
	cppClass = lookForClass(SaxonProcessor::sxn_environ->env,
			"net/sf/saxon/option/cpp/Xslt30Processor");

	jobject tempcppXT = createSaxonProcessor2(SaxonProcessor::sxn_environ->env, cppClass,
			"(Lnet/sf/saxon/s9api/Processor;)V", proc->proc);
	if(tempcppXT) {
        cppXT = SaxonProcessor::sxn_environ->env->NewGlobalRef(tempcppXT);
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(tempcppXT);

    } else {
	    createException("Error: Failed to create the Xslt30Processor internal object");

	}

#ifdef DEBUG
	jmethodID debugMID = SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass, "setDebugMode", "(Z)V");
	SaxonProcessor::sxn_environ->env->CallStaticVoidMethod(cppClass, debugMID, (jboolean)true);
    
#endif

	if(cppXT == nullptr) {
        createException();
	}

	if(!(proc->cwd.empty()) && curr.empty()){
		cwdXT = proc->cwd;
	} else if(!curr.empty()){
		cwdXT = curr;
	}
}

    Xslt30Processor::~Xslt30Processor(){
	    clearParameters();
	    clearProperties();
	    SaxonProcessor::sxn_environ->env->DeleteGlobalRef(cppXT);
	    cwdXT.erase();
	    exceptionClear();
    }


Xslt30Processor::Xslt30Processor(const Xslt30Processor &other) {
	proc = other.proc;
	cppClass = other.cppClass;
	cppXT = other.cppXT;
    cwdXT = other.cwdXT;
	std::map<std::string, XdmValue*>::const_iterator paramIter = other.parameters.begin();
    while(paramIter != other.parameters.end())
    {

       XdmValue * valuei = paramIter->second;
       if(valuei == nullptr) {
    	 	//std::cerr<<"Error in Xslt30Processor copy constructor"<<std::endl;
       } else {
            parameters[paramIter->first] = new XdmValue(*(valuei));
       }
       paramIter++;
    }

    std::map<std::string, std::string>::const_iterator propIter = other.properties.begin();
    while (propIter != other.properties.end()) {
        properties[propIter->first] = propIter->second;
        propIter++;
    }

	jitCompilation = other.jitCompilation;

}

void Xslt30Processor::setProperty(const char *name, const char *value) {
    if (name != nullptr) {
        properties.erase(name);
        std::string skey = std::string(name);
        properties[skey] = std::string((value == nullptr ? "" : value));
    }
}

void Xslt30Processor::clearProperties() {
    properties.clear();
}




bool Xslt30Processor::exceptionOccurred() {
	return proc->exceptionOccurred() || exception != nullptr;
}

const char * Xslt30Processor::getErrorCode() {
 if(exception == nullptr) {return nullptr;}
 return exception->getErrorCode();
 }

void Xslt30Processor::setBaseOutputURI(const char *baseURI) {
     if (baseURI != nullptr) {
         setProperty("baseoutput", baseURI);
     }
}


void Xslt30Processor::setParameter(const char* name, XdmValue * value) {
    if(value != nullptr && name != nullptr) {
        std::string skey = ("sparam:" + std::string(name));
        std::map<std::string, XdmValue *>::iterator it = parameters.find(skey);
        if (it != parameters.end()) {
            XdmValue *valuei = it->second;
            if (valuei != nullptr) {
                valuei->decrementRefCount();
                parameters.erase(skey);
            }
        }
        parameters[skey] = value;
    }
}

XdmValue* Xslt30Processor::getParameter(const char* name) {
        std::map<std::string, XdmValue*>::iterator it;
        it = parameters.find("sparam:"+std::string(name));
        if (it != parameters.end())
          return it->second;
	    return nullptr;
}

bool Xslt30Processor::removeParameter(const char* name) {
	return (bool)(parameters.erase("param:"+std::string(name)));
}

void Xslt30Processor::setJustInTimeCompilation(bool jit){
    jitCompilation = jit;
}


void Xslt30Processor::clearParameters(bool delValues) {
	if(delValues){
        for(std::map<std::string, XdmValue*>::iterator itr = parameters.begin(); itr != parameters.end(); itr++){
            XdmValue * value = itr->second;
            if(value != nullptr) {
#ifdef DEBUG
                std::cout<<"clearParameter() - XdmValue refCount="<<value->getRefCount()<<std::endl;
#endif
                    delete value;
                    value = nullptr;

            }
        }

	}
	parameters.clear();
	
}



std::map<std::string,XdmValue*>& Xslt30Processor::getParameters(){
	std::map<std::string,XdmValue*>& ptr = parameters;
	return ptr;
}


void Xslt30Processor::exceptionClear(){
 if(exception != nullptr) {
 	delete exception;
 	exception = nullptr;
	SaxonProcessor::sxn_environ->env->ExceptionClear();
 }
  
 }

   void Xslt30Processor::setcwd(const char* dir){
    if (dir!= nullptr) {
        cwdXT = std::string(dir);
    }
   }

SaxonApiException* Xslt30Processor::getException() {
    return exception;
}

void Xslt30Processor::createException(const char * message) {
    if(exception != nullptr) {
        delete exception;
        exception = nullptr;
    }
    if(message == nullptr) {
        exception = proc->checkAndCreateException(cppClass);
    } else {
        exception = new SaxonApiException(message);
    }

}



void Xslt30Processor::compileFromXdmNodeAndSave(XdmNode * node, const char* filename) {
	static jmethodID cAndSNodemID = nullptr;

	if(cAndSNodemID == nullptr) {
			cAndSNodemID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass,
					"compileFromXdmNodeAndSave",
					"(Ljava/lang/String;Ljava/lang/Object;Ljava/lang/String;)V");
	}
	if (!cAndSNodemID) {
		std::cerr<<"Error: "<<getDllname()<<"compileFromXdmNodeAndSave function not found"<<std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
	} else {
        if(cppXT == nullptr) {
#if defined(DEBUG)
            std::cerr << "The Java Xslt30Processor object (i.e. cppXT) is NULL - Possible exception thrown" << std::endl;
#endif
            return;
        }
		
		SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXT, cAndSNodemID,
						SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXT.c_str()),
						node->getUnderlyingValue(), SaxonProcessor::sxn_environ->env->NewStringUTF(filename));
		
		createException();

    }



}

    void Xslt30Processor::compileFromStringAndSave(const char* stylesheetStr, const char* filename){
	static jmethodID cAndSStringmID = nullptr;
	if(stylesheetStr == nullptr || filename == nullptr) {
        createException("Error in compileFromStringAndSave - Found null value - check stylesheet or filename string");
	    return;
	}
	if(cAndSStringmID == nullptr) {
	   cAndSStringmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass,
					"compileFromStringAndSave",
					"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	}
	if (!cAndSStringmID) {
        createException("Error: compileFromStringAndSave function not found\n");

	} else {
        if(cppXT == nullptr) {
#if defined(DEBUG)
            std::cerr << "The Java Xslt30Processor object (i.e. cppXT) is NULL - Possible exception thrown" << std::endl;
#endif
            return;
        }
        std::cerr<<"x30proc cp1"<<std::endl;
		SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXT, cAndSStringmID,
						(cwdXT.empty() ? nullptr : SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXT.c_str())),
						SaxonProcessor::sxn_environ->env->NewStringUTF(stylesheetStr),
						SaxonProcessor::sxn_environ->env->NewStringUTF(filename));
        std::cerr<<"x30proc cp2"<<std::endl;
		createException();
        std::cerr<<"x30proc cp3"<<std::endl;
    }
}



    void Xslt30Processor::compileFromFileAndSave(const char* xslFilename, const char* filename){
	static jmethodID cAndFStringmID =  nullptr;

	if (cAndFStringmID == nullptr) {
	    cAndFStringmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass,
					"compileFromFileAndSave",
					"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	}
	if (!cAndFStringmID) {
        createException("Error: compileFromFileAndSave function not found");

	} else {

        if(cppXT == nullptr) {
#if defined(DEBUG)
            std::cerr << "The Java Xslt30Processor object (i.e. cppXT) is NULL - Possible exception thrown" << std::endl;
#endif
            return;
        }
		SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXT, cAndFStringmID,
						SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXT.c_str()),
						SaxonProcessor::sxn_environ->env->NewStringUTF(xslFilename),SaxonProcessor::sxn_environ->env->NewStringUTF(filename));

        createException();


     }
}

XsltExecutable * Xslt30Processor::compileFromString(const char* stylesheetStr) {
	static jmethodID cStringmID = nullptr;
	if (cStringmID == nullptr) {
			cStringmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass,
					"compileFromString",
					"(Ljava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;[Ljava/lang/Object;)Lnet/sf/saxon/s9api/XsltExecutable;");
	}
					
	if (cStringmID== nullptr) {
        createException(strcat(getDllname(), ".compileFromString function not found"));
		return nullptr;

	} else {
        if(cppXT == nullptr) {
#if defined(DEBUG)
            std::cerr << "The Java Xslt30Processor object (i.e. cppXT) is NULL - Possible exception thrown" << std::endl;
#endif
            return nullptr;
        }
		JParameters comboArrays;
		comboArrays = SaxonProcessor::createParameterJArray(parameters, properties);
		jobject executableObject = (jobject)(
				SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXT, cStringmID,
						SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXT.c_str()),
						SaxonProcessor::sxn_environ->env->NewStringUTF(stylesheetStr), jitCompilation, comboArrays.stringArray, comboArrays.objectArray));
		if (!executableObject) {
            createException();
			return nullptr;
		}
		if (comboArrays.stringArray != nullptr) {
			SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
			SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
		}

		jobject obj = SaxonProcessor::sxn_environ->env->NewGlobalRef(executableObject);
        XsltExecutable * executable = new XsltExecutable(obj, cwdXT, proc->getResourcesDirectory());
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(executableObject);
		return executable;
	}

}

XsltExecutable * Xslt30Processor::compileFromXdmNode(XdmNode * node) {
	static jmethodID cNodemID = nullptr;
    if(cNodemID == nullptr) {
			cNodemID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass,"compileFromXdmNode",
			"(Ljava/lang/String;Ljava/lang/Object;Z[Ljava/lang/String;[Ljava/lang/Object;)Lnet/sf/saxon/s9api/XsltExecutable;");
	}
	if (!cNodemID) {
		std::cerr << "Error: "<< getDllname() << ".compileFromXdmNode"
				<< " not found\n" << std::endl;
		return nullptr;

	} else {
	    if(node == nullptr) {
            createException("Error in compileFromXdmNode - node argument is NULL");
	        return nullptr;
	    }
        if(cppXT == nullptr) {
#if defined(DEBUG)
            std::cerr << "The Java Xslt30Processor object (i.e. cppXT) is NULL - Possible exception thrown" << std::endl;
#endif
            return nullptr;
        }
		JParameters comboArrays;
		comboArrays = SaxonProcessor::createParameterJArray(parameters, properties);
		jobject executableObject = (jobject)(
				SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXT, cNodemID,
						SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXT.c_str()),
						node->getUnderlyingValue(), jitCompilation, comboArrays.stringArray, comboArrays.objectArray));
		if (!executableObject) {
			createException();
			return nullptr;
		}
		if (comboArrays.stringArray != nullptr) {
			SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
			SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
		}

        XsltExecutable * executable = new XsltExecutable(SaxonProcessor::sxn_environ->env->NewGlobalRef(executableObject), cwdXT, proc->getResourcesDirectory());
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(executableObject);
		return executable;
	}

}

XsltExecutable * Xslt30Processor::compileFromAssociatedFile(const char* source) {
	static jmethodID cFilemID = nullptr;
    if(cFilemID == nullptr) {
	    cFilemID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass,
					"compileFromAssociatedFile",
					"(Ljava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;[Ljava/lang/Object;)Lnet/sf/saxon/s9api/XsltExecutable;");
	}
	if (!cFilemID) {
		std::cerr << "Error: "<<getDllname() << ".compileFromAssociatedFile"
				<< " not found\n" << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
		return nullptr;

	} else {
		
		if(source == nullptr) {
            createException("Error in compileFromFile method - The Stylesheet file is NULL");
			return nullptr;
		}
        if(cppXT == nullptr) {
#if defined(DEBUG)
            std::cerr << "The Java Xslt30Processor object (i.e. cppXT) is NULL - Possible exception thrown" << std::endl;
#endif
            return nullptr;
        }
		JParameters comboArrays;
		comboArrays = SaxonProcessor::createParameterJArray(parameters, properties);
		jobject executableObject = (jobject)(
				SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXT, cFilemID,
						SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXT.c_str()),
						SaxonProcessor::sxn_environ->env->NewStringUTF(source), jitCompilation, comboArrays.stringArray, comboArrays.objectArray));
		if (!executableObject) {
			createException();
     		return nullptr;
		}
		if (comboArrays.stringArray != nullptr) {
			SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
			SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
		}

        XsltExecutable * executable = new XsltExecutable(SaxonProcessor::sxn_environ->env->NewGlobalRef(executableObject), cwdXT, proc->getResourcesDirectory());
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(executableObject);
		return executable;
	}

}


XsltExecutable * Xslt30Processor::compileFromFile(const char* stylesheet) {
	static jmethodID cFilemID = nullptr;
	if(cFilemID == nullptr) {
	    cFilemID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass,
					"compileFromFile",
					"(Ljava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;[Ljava/lang/Object;)Lnet/sf/saxon/s9api/XsltExecutable;");
	}

	if (!cFilemID) {
		std::cerr << "Error: "<<getDllname() << ".compileFromFile"
				<< " not found\n" << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
		return nullptr;

	} else {
        if (stylesheet == nullptr) {
            createException("The Stylesheet file is NULL");
            return nullptr;
        }
        if(cppXT == nullptr) {
#if defined(DEBUG)
            std::cerr << "The Java Xslt30Processor object (i.e. cppXT) is NULL - Possible exception thrown" << std::endl;
#endif

            return nullptr;
        }

        JParameters comboArrays;
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties);
        jobject executableObject = (jobject) (
                SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXT, cFilemID,
                                                                   SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                           cwdXT.c_str()),
                                                                   SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                           stylesheet), jitCompilation,
                                                                   comboArrays.stringArray, comboArrays.objectArray));
        if (!executableObject) {
            createException();
            return nullptr;

        }
        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }

        XsltExecutable *executable;
        executable = new XsltExecutable(SaxonProcessor::sxn_environ->env->NewGlobalRef(executableObject), cwdXT, proc->getResourcesDirectory());
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(executableObject);

        return executable;

	}

}


XdmValue * Xslt30Processor::transformFileToValue(const char* sourcefile,
		const char* stylesheetfile) {

	if(exceptionOccurred()) {
		//Possible error detected in the compile phase. Processor not in a clean state.
		//Require clearing exception.
		std::cerr<<"Error: Xslt30Processor not in a clean state. - Exception found"<<std::endl;
		SaxonProcessor::sxn_environ->env->ExceptionDescribe();

		return nullptr;
	}

	if(sourcefile == nullptr && stylesheetfile == nullptr){
	
		return nullptr;
	}


	static jmethodID mtvID = nullptr;

	if(mtvID == nullptr) {
			mtvID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
					"transformToValue",
					"(Ljava/lang/String;Lnet/sf/saxon/option/cpp/Xslt30Processor;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)Lnet/sf/saxon/s9api/XdmValue;");
	}
	if (!mtvID) {
        std::cerr<<"Error: "<<getDllname()<<".transformtoValue function not found"<<std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
	} else {
        if(cppXT == nullptr) {
//#if defined(DEBUG)
            createException("The Java Xslt30Processor object (i.e. cppXT) is NULL - Possible exception thrown");
//#endif
            return nullptr;
        }
	    JParameters comboArrays;
		comboArrays = SaxonProcessor::createParameterJArray(parameters, properties);
        jstring jcwd = SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXT.c_str());
        jstring jsourcefile = (sourcefile != nullptr ? SaxonProcessor::sxn_environ->env->NewStringUTF(sourcefile) : nullptr);
        jstring jstylesheetfile = (stylesheetfile != nullptr ? SaxonProcessor::sxn_environ->env->NewStringUTF(stylesheetfile) : nullptr);
		jobject result = (jobject)(
				SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, mtvID,jcwd, cppXT, NULL, jsourcefile,
						                            jstylesheetfile, comboArrays.stringArray, comboArrays.objectArray));
		if (comboArrays.stringArray != nullptr) {
			SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
			SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
		}
        if (jcwd != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(jcwd);
        }
        if (jsourcefile != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(jsourcefile);
        }
        if (jstylesheetfile != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(jstylesheetfile);
        }
		if (result) {

			jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmAtomicValue");
          		jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
          		jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmFunctionItem");
                jclass mapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
                jclass arrayClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmArray");
			    XdmValue * value = new XdmValue();
          		XdmItem * xdmItem = nullptr;


          			if(SaxonProcessor::sxn_environ->env->IsInstanceOf(result, atomicValueClass)           == JNI_TRUE) {
          				xdmItem = new XdmAtomicValue(result);
                        SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                        value->addXdmItem(xdmItem);
                        return value;

          			} else if(SaxonProcessor::sxn_environ->env->IsInstanceOf(result, nodeClass)           == JNI_TRUE) {
          				xdmItem = new XdmNode(result);
                        SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                        value->addXdmItem(xdmItem);
                        return value;

          			}  else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, mapClass)           == JNI_TRUE) {
                        xdmItem =  new XdmMap(result);
                        SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                        value->addXdmItem(xdmItem);
                        return value;

                    } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, arrayClass)           == JNI_TRUE) {
                        xdmItem =  new XdmArray(result);
                        SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                        value->addXdmItem(xdmItem);
                        return value;

                    } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, functionItemClass)           == JNI_TRUE) {
                        xdmItem =  new XdmFunctionItem(result);
                        SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                        value->addXdmItem(xdmItem);
                        return value;
                    }
				    value = new XdmValue(result, false);
          	        SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
          		    return value;
		}else {

			createException();

     		}
	}
	return nullptr;

}


void Xslt30Processor::transformFileToFile(const char* source,
		const char* stylesheet, const char* outputfile) {

	if(exceptionOccurred()) {
		//Possible error detected in the compile phase. Processor not in a clean state.
		//Require clearing exception.
        std::cerr<<"Error: Xslt30Processor not in a clean state. - Exception found"<<std::endl;
		return;	
	}
	if(stylesheet==nullptr){
		std::cerr<< "Error: stylesheet has not been set."<<std::endl;
		return;
	}
	//
	static jmethodID mtfID = nullptr;

	if(mtfID == nullptr) {
		mtfID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
					"transformToFile",
					"(Ljava/lang/String;Lnet/sf/saxon/option/cpp/Xslt30Processor;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)V");
	}
	if (!mtfID) {
        std::cerr<<"Error: "<<getDllname()<<".transformToFile function not found"<<std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
	} else {
        if(cppXT == nullptr) {
#if defined(DEBUG)
            std::cerr << "The Java Xslt30Processor object (i.e. cppXT) is NULL - Possible exception thrown" << std::endl;
#endif
            return;
        }

	    JParameters comboArrays;
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties);

		SaxonProcessor::sxn_environ->env->CallStaticVoidMethod(cppClass, mtfID,
								SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXT.c_str()), cppXT, NULL,
								(source != nullptr ? SaxonProcessor::sxn_environ->env->NewStringUTF(source) : NULL),
								SaxonProcessor::sxn_environ->env->NewStringUTF(stylesheet),	(outputfile != nullptr ? SaxonProcessor::sxn_environ->env->NewStringUTF(outputfile) : NULL),
								comboArrays.stringArray, comboArrays.objectArray);
		if (comboArrays.stringArray!= nullptr) {
			SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
			SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
		}
		}
		createException();
	}


const char * Xslt30Processor::transformFileToString(const char* source,
		const char* stylesheet) {
	if(exceptionOccurred()) {
		//Possible error detected in the compile phase. Processor not in a clean state.
		//Require clearing exception.
        std::cerr<<"Error: Xslt30Processor not in a clean state. - Exception found"<<std::endl;
		return nullptr;
	}
	if(source == nullptr && stylesheet == nullptr){
		std::cerr<< "Error: nullptr file name found in transformFiletoString."<<std::endl;
		return nullptr;
	}

	static jmethodID mtsID =  nullptr;

	if(mtsID == nullptr) {
			mtsID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
					"transformToString",
					"(Ljava/lang/String;Lnet/sf/saxon/option/cpp/Xslt30Processor;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)[B");
	}
	if (!mtsID) {
        std::cerr<<"Error: "<<getDllname()<<".transformFileToString not found"<<std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
	} else {
        if(cppXT == nullptr) {
#if defined(DEBUG)
            std::cerr << "The Java Xslt30Processor object (i.e. cppXT) is NULL - Possible exception thrown" << std::endl;
#endif
            return nullptr;
        }
    JParameters comboArrays;
    comboArrays = SaxonProcessor::createParameterJArray(parameters, properties);

    jbyteArray result = nullptr;
	jobject obj = SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, mtsID,
								SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXT.c_str()), cppXT, NULL,
						(source != nullptr ? SaxonProcessor::sxn_environ->env->NewStringUTF(
												source) : NULL),
								(stylesheet != nullptr ? SaxonProcessor::sxn_environ->env->NewStringUTF(stylesheet) : NULL),
								comboArrays.stringArray, comboArrays.objectArray);
    if (comboArrays.stringArray!= nullptr) {
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
    }
	if(obj) {
			result = (jbyteArray)obj;
	}

	if (result) {
        jboolean isCopy = false;
        jbyte* b = SaxonProcessor::sxn_environ->env->GetByteArrayElements(result, &isCopy);
        jsize num_bytes = SaxonProcessor::sxn_environ->env->GetArrayLength(result);

            char* str = new char[num_bytes + 1];
            memcpy ( str, b , num_bytes );
            str[num_bytes] = '\0';

             SaxonProcessor::sxn_environ->env->ReleaseByteArrayElements( result, b, 0);
             SaxonProcessor::sxn_environ->env->DeleteLocalRef(obj);
        return str;
	} else  {
	    createException();
	   		
	}
	}
	return nullptr;
}



const char * Xslt30Processor::getErrorMessage(){
 	if(exception == nullptr) {return nullptr;}
 	return exception->getMessage();
 }

