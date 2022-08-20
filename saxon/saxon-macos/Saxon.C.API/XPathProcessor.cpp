#include "XPathProcessor.h"
#include "XdmValue.h"
#include "XdmItem.h"
#include "XdmNode.h"
#include "XdmAtomicValue.h"
#include "XdmFunctionItem.h"
#include "XdmMap.h"
#include "XdmArray.h"

#include <iostream>
#include <fstream>


#ifdef MEM_DEBUG
#define new new(__FILE__, __LINE__)
#endif

XPathProcessor::XPathProcessor() {
	SaxonProcessor *p = new SaxonProcessor(false);
	XPathProcessor(p, "");
}

XPathProcessor::XPathProcessor(SaxonProcessor* p, std::string curr) {
    proc = p;
    SaxonProcessor::attachCurrentThread();
    /*
     * Look for class.
     */
    cppClass = lookForClass(SaxonProcessor::sxn_environ->env,
                            "net/sf/saxon/option/cpp/XPathProcessor");
    if ((proc->proc) == nullptr) {
        std::cerr << "Processor is nullptr" << std::endl;
    }

    jobject tempcppXP = createSaxonProcessor2(SaxonProcessor::sxn_environ->env, cppClass,
                                              "(Lnet/sf/saxon/s9api/Processor;)V", proc->proc);


    if (tempcppXP) {
        cppXP = SaxonProcessor::sxn_environ->env->NewGlobalRef(tempcppXP);
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(tempcppXP);

    } else {
        std::cerr << "Error: Failed to create the XPathProcessor internal object" << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
        return;
    }





#ifdef DEBUG
	jmethodID debugMID = SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass, "setDebugMode", "(Z)V");
	SaxonProcessor::sxn_environ->env->CallStaticVoidMethod(cppClass, debugMID, (jboolean)true);
#endif    

	exception = nullptr;
	if(!(proc->cwd.empty()) && curr.empty()){
		cwdXP = proc->cwd;
	} else {
		cwdXP = curr;
	}

}



XPathProcessor::XPathProcessor(const XPathProcessor &other) {
    cwdXP = other.cwdXP;
    proc = other.proc;
    cppClass = other.cppClass;
    cppXP = SaxonProcessor::sxn_environ->env->NewGlobalRef(other.cppXP);

    std::map<std::string, XdmValue*>::const_iterator paramIter = other.parameters.begin();
    while(paramIter != other.parameters.end())
    {

        XdmValue * valuei = paramIter->second;
        if(valuei == nullptr) {

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
}


XPathProcessor::~XPathProcessor(){
    SaxonProcessor::attachCurrentThread();
    clearProperties();
    clearParameters();

    SaxonProcessor::sxn_environ->env->DeleteGlobalRef(cppXP);
    exceptionClear();
    //delete contextItem;
}

XdmValue * XPathProcessor::evaluate(const char * xpathStr) {
    SaxonProcessor::attachCurrentThread();
  	if (xpathStr == nullptr) {
		std::cerr << "Error:: XPath string cannot be empty or nullptr" << std::endl;
	return nullptr;
}

jmethodID mID =
		(jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass, "evaluate",
				"(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)[Lnet/sf/saxon/s9api/XdmValue;");
if (!mID) {
	std::cerr << "Error: "<<getDllname() << ".evaluate" << " not found\n"
			<< std::endl;

} else {
	setProperty("resources", proc->getResourcesDirectory());
	jobjectArray stringArray = nullptr;
	jobjectArray objectArray = nullptr;
	jclass objectClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/Object");
	jclass stringClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/String");

	int size = parameters.size() + properties.size();

	if (size > 0) {
		objectArray = SaxonProcessor::sxn_environ->env->NewObjectArray((jint) size,
				objectClass, 0);
		stringArray = SaxonProcessor::sxn_environ->env->NewObjectArray((jint) size,
				stringClass, 0);
		int i = 0;
		for (std::map<std::string, XdmValue*>::iterator iter = parameters.begin();
				iter != parameters.end(); ++iter, i++) {
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(stringArray, i,
					SaxonProcessor::sxn_environ->env->NewStringUTF((iter->first).c_str()));
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(objectArray, i,
					(iter->second)->getUnderlyingValue());
		}
		for (std::map<std::string, std::string>::iterator iter = properties.begin();
				iter != properties.end(); ++iter, i++) {
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(stringArray, i,
					SaxonProcessor::sxn_environ->env->NewStringUTF((iter->first).c_str()));
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(objectArray, i,
					SaxonProcessor::sxn_environ->env->NewStringUTF((iter->second).c_str()));
		}
	} 
	jobjectArray results = (jobjectArray)(
			SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXP, mID,
					SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXP.c_str()),
					SaxonProcessor::sxn_environ->env->NewStringUTF(xpathStr), stringArray, objectArray));
	if(!results) {

		exception = proc->checkAndCreateException(cppClass);
		//SaxonProcessor::sxn_environ->env->ExceptionClear();
		if(exception != nullptr) {
		    std::cout<<exception->getMessage();
		}
		return nullptr;
	}

	int sizex = SaxonProcessor::sxn_environ->env->GetArrayLength(results);
	if (size > 0) {
		SaxonProcessor::sxn_environ->env->DeleteLocalRef(stringArray);
		SaxonProcessor::sxn_environ->env->DeleteLocalRef(objectArray);
	}

	if (sizex>0) {
		jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmAtomicValue");
		jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
		jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmFunctionItem");
        jclass mapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
        jclass arrayClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmArray");

		XdmValue * value = new XdmValue();
		XdmItem * xdmItem = nullptr;
		for (int p=0; p < sizex; ++p) 
		{
			jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);

			if(SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, atomicValueClass)           == JNI_TRUE) {
				xdmItem = new XdmAtomicValue(resulti);

			} else if(SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, nodeClass)           == JNI_TRUE) {
				xdmItem = new XdmNode(resulti);

			}  else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, mapClass)           == JNI_TRUE) {
                xdmItem =  new XdmMap(resulti);
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, arrayClass)           == JNI_TRUE) {
                xdmItem =  new XdmArray(resulti);
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, functionItemClass)           == JNI_TRUE) {
                xdmItem =  new XdmFunctionItem(resulti);
            } else {
			    continue;
			}
  

			value->addXdmItem(xdmItem);
		}
		SaxonProcessor::sxn_environ->env->DeleteLocalRef(results);
		return value;
	}
}
return nullptr;

}

XdmItem * XPathProcessor::evaluateSingle(const char * xpathStr) {
    SaxonProcessor::attachCurrentThread();
	if (xpathStr == nullptr) {
		std::cerr << "Error:: XPath string cannot be empty or nullptr" << std::endl;
	     return nullptr;
        }
        /*std::ofstream myfile;
          myfile.open ("saxon_error_log1.txt");
          myfile<<"evaluateSingle method: start of evaluateSingle\n";*/
jmethodID mID =
		(jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass, "evaluateSingle",
				"(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)Lnet/sf/saxon/s9api/XdmItem;");
if (!mID) {
	std::cerr << "Error: MyClassInDll." << "evaluateSingle" << " not found\n"
			<< std::endl;

} else {
	setProperty("resources", proc->getResourcesDirectory());
	jobjectArray stringArray = nullptr;
	jobjectArray objectArray = nullptr;
	jclass objectClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/Object");
	jclass stringClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/String");

	int size = parameters.size() + properties.size();
#ifdef DEBUG
		std::cerr<<"Properties size: "<<properties.size()<<std::endl;
		std::cerr<<"Parameter size: "<<parameters.size()<<std::endl;
		std::cerr<<"size:"<<size<<std::endl;
#endif
	if (size > 0) {
		objectArray = SaxonProcessor::sxn_environ->env->NewObjectArray((jint) size,
				objectClass, 0);
		stringArray = SaxonProcessor::sxn_environ->env->NewObjectArray((jint) size,
				stringClass, 0);
		int i = 0;
		for (std::map<std::string, XdmValue*>::iterator iter = parameters.begin();
				iter != parameters.end(); ++iter, i++) {
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(stringArray, i,
					SaxonProcessor::sxn_environ->env->NewStringUTF((iter->first).c_str()));
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(objectArray, i,
					(iter->second)->getUnderlyingValue());
#ifdef DEBUG
				std::string s1 = typeid(iter->second).name();
				std::cerr<<"Type of itr:"<<s1<<std::endl;
				jobject xx = (iter->second)->getUnderlyingValue();
				if(xx == nullptr) {
					std::cerr<<"value failed"<<std::endl;
				} else {

					std::cerr<<"Type of value:"<<(typeid(xx).name())<<std::endl;
				}
				if((iter->second)->getUnderlyingValue() == nullptr) {
					std::cerr<<"(iter->second)->getUnderlyingValue() is nullptr"<<std::endl;
				}
#endif
		}
		for (std::map<std::string, std::string>::iterator iter = properties.begin();
				iter != properties.end(); ++iter, i++) {
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(stringArray, i,
					SaxonProcessor::sxn_environ->env->NewStringUTF((iter->first).c_str()));
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(objectArray, i,
					SaxonProcessor::sxn_environ->env->NewStringUTF((iter->second).c_str()));
		}
	}

	jobject result = (jobject)(
			SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXP, mID,
					SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXP.c_str()),
					SaxonProcessor::sxn_environ->env->NewStringUTF(xpathStr), stringArray, objectArray));
	if (size > 0) {
		SaxonProcessor::sxn_environ->env->DeleteLocalRef(stringArray);
		SaxonProcessor::sxn_environ->env->DeleteLocalRef(objectArray);
	}
	if (result) {
		jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmAtomicValue");
		jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
		jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmFunctionItem");
        jclass mapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
        jclass arrayClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmArray");
		XdmItem * xdmItem = nullptr;
		if(SaxonProcessor::sxn_environ->env->IsInstanceOf(result, atomicValueClass)           == JNI_TRUE) {
			xdmItem = new XdmAtomicValue(result);

		} else if(SaxonProcessor::sxn_environ->env->IsInstanceOf(result, nodeClass)           == JNI_TRUE) {

			xdmItem = new XdmNode(result);

		} else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, mapClass)           == JNI_TRUE) {
            xdmItem =  new XdmMap(result);
        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, arrayClass)           == JNI_TRUE) {
            xdmItem =  new XdmArray(result);
        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, functionItemClass)           == JNI_TRUE) {
		    xdmItem = new XdmFunctionItem(result);

		}
		SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
		return xdmItem;
	} else  {
			exception = proc->checkAndCreateException(cppClass);

     		}
}

return nullptr;

}

void XPathProcessor::setContextItem(XdmItem * item) {
    if(item == nullptr) {
       std::map<std::string, XdmValue*>::iterator it;
       it = parameters.find("node");
       if (it != parameters.end()) {
            XdmValue * valuei = it->second;
            if(valuei != nullptr) {
                valuei->decrementRefCount();
                parameters.erase("node");
            }
       }
    } else {
        setParameter("node", item, false);
    }
}

void XPathProcessor::setContextFile(const char * filename) {

	if (filename != nullptr) {
		setProperty("s", filename);
	}  else {
	    properties.erase("s");
	}
}


void XPathProcessor::declareNamespace(const char *prefix, const char * uri){
    SaxonProcessor::attachCurrentThread();
        if (uri == nullptr) {
		return;
        }
	jmethodID dnspID =
		(jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass, "declareNamespace",
				"(Ljava/lang/String;Ljava/lang/String;)V");
	if (!dnspID) {
	    std::cerr << "Error: "<<getDllname() << ".declareNameSpace" << " not found\n"
			<< std::endl;

	} else {
		SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXP, dnspID,
				(prefix== nullptr? SaxonProcessor::sxn_environ->env->NewStringUTF("") :SaxonProcessor::sxn_environ->env->NewStringUTF(prefix)),
				(uri== nullptr? SaxonProcessor::sxn_environ->env->NewStringUTF("") :SaxonProcessor::sxn_environ->env->NewStringUTF(uri)));

		exception = proc->checkAndCreateException(cppClass);

	}

}


void XPathProcessor::declareVariable(const char * name){
    SaxonProcessor::attachCurrentThread();
    if (name == nullptr) {
		return;
    }
	jmethodID dnspID =
		(jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass, "declareVariable",
				"(Ljava/lang/String;)V");
	if (!dnspID) {
	    std::cerr << "Error: "<<getDllname() << ".declareVariable" << " not found\n"
			<< std::endl;

	} else {
		SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXP, dnspID,SaxonProcessor::sxn_environ->env->NewStringUTF(name));

		exception = proc->checkAndCreateException(cppClass);
	}

}


#if CVERSION_API_NO >= 121


void XPathProcessor::setBackwardsCompatible(bool option) {
    if (option) {
        		setProperty("backwardsCom:", "true");
    } else {
       properties.erase("backwardsCom:");
    }
}

void XPathProcessor::setCaching(bool caching){
    if (caching) {
        	setProperty("caching:", "true");
    } else {
       properties.erase("caching:");
    }

}

void XPathProcessor::importSchemaNamespace(const char* uri){
      if (uri != nullptr) {
       		setProperty("importSN", uri);
      }
}


#endif


void XPathProcessor::setBaseURI(const char * uriStr) {
    SaxonProcessor::attachCurrentThread();
	if (uriStr == nullptr) {
		std::cerr << "Error:: XPath string cannot be empty or nullptr" << std::endl;
	     return;
        }

jmethodID mID =
		(jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass, "setBaseURI",
				"(Ljava/lang/String;)V");
if (!mID) {
	std::cerr << "Error: Saxonc." << "setBaseURI" << " not found\n"
			<< std::endl;

} else {

	SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXP, mID,
					SaxonProcessor::sxn_environ->env->NewStringUTF(uriStr));
}

}

bool XPathProcessor::effectiveBooleanValue(const char * xpathStr) {
    SaxonProcessor::attachCurrentThread();
	if (xpathStr == nullptr) {
		std::cerr << "Error:: XPath string cannot be empty or nullptr" << std::endl;
	     return false;
        }

jmethodID mID =
		(jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass, "effectiveBooleanValue",
				"(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)Z");
if (!mID) {
	std::cerr << "Error: MyClassInDll." << "effectiveBooleanValue" << " not found\n"<< std::endl;

} else {
std::cerr<<"xpathStr: = "<<xpathStr<<std::endl;
	setProperty("resources", proc->getResourcesDirectory());
	jobjectArray stringArray = nullptr;
	jobjectArray objectArray = nullptr;
	jclass objectClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/Object");
	jclass stringClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/String");

	int size = parameters.size() + properties.size();
	if (size > 0) {
		objectArray = SaxonProcessor::sxn_environ->env->NewObjectArray((jint) size,
				objectClass, 0);
		stringArray = SaxonProcessor::sxn_environ->env->NewObjectArray((jint) size,
				stringClass, 0);
		int i = 0;
		for (std::map<std::string, XdmValue*>::iterator iter = parameters.begin();
				iter != parameters.end(); ++iter, i++) {
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(stringArray, i,
					SaxonProcessor::sxn_environ->env->NewStringUTF((iter->first).c_str()));
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(objectArray, i,
					(iter->second)->getUnderlyingValue());
		}
		for (std::map<std::string, std::string>::iterator iter = properties.begin();
				iter != properties.end(); ++iter, i++) {
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(stringArray, i,
					SaxonProcessor::sxn_environ->env->NewStringUTF((iter->first).c_str()));
			SaxonProcessor::sxn_environ->env->SetObjectArrayElement(objectArray, i,
					SaxonProcessor::sxn_environ->env->NewStringUTF((iter->second).c_str()));
		}
	}
	jboolean result = (jboolean)(
			SaxonProcessor::sxn_environ->env->CallBooleanMethod(cppXP, mID,
					SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXP.c_str()),
					SaxonProcessor::sxn_environ->env->NewStringUTF(xpathStr), stringArray, objectArray));
	if (size > 0) {
		SaxonProcessor::sxn_environ->env->DeleteLocalRef(stringArray);
		SaxonProcessor::sxn_environ->env->DeleteLocalRef(objectArray);
	}
	exception = proc->checkAndCreateException(cppClass);
	return result;
}
return false;
}                                                 


void XPathProcessor::setParameter(const char * name, XdmValue*value, bool withParam) {
    if(value != nullptr && name != nullptr) {
        std::string skey = (withParam ? "param:" : "")+std::string(name);
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



void XPathProcessor::setParameter(const char * name, XdmValue* value) {
    setParameter(name, value, true);
}

bool XPathProcessor::removeParameter(const char * name) {
	return (bool)(parameters.erase("param:"+std::string(name)));
}

void XPathProcessor::setProperty(const char * name, const char * value) {
	if(name != nullptr) {
	    int s = properties.size();
		std::string skey = std::string(name);
		properties.insert(std::pair<std::string, std::string>(skey, std::string((value == nullptr ? "" : value))));

		if(s == properties.size()) {
            std::map<std::string, std::string>::iterator it;
            it = properties.find(skey);
            if (it != properties.end()) {
                properties.erase(skey);
                properties[skey] = std::string((value == nullptr ? "" : value));
            }
		}
	}
}

void XPathProcessor::clearParameters(bool delVal) {
//std::cerr<<"XPath clearParameter"<<std::endl;
	if(delVal) {
        for (std::map<std::string, XdmValue *>::iterator itr = parameters.begin(); itr != parameters.end(); itr++) {
            XdmValue *value = itr->second;
            if (value != nullptr) {
#ifdef DEBUG
                std::cerr<<"XPathProc.clearParameter() - XdmValue refCount="<<value->getRefCount()<<std::endl;
#endif
                    delete value;
            }
        }
    }
	parameters.clear();

}

void XPathProcessor::clearProperties() {
	properties.clear();
}


   void XPathProcessor::setcwd(const char* dir){
    cwdXP = std::string(dir);
   }

std::map<std::string,XdmValue*>& XPathProcessor::getParameters(){
	std::map<std::string,XdmValue*>& ptr = parameters;
	return ptr;
}

std::map<std::string,std::string>& XPathProcessor::getProperties(){
	std::map<std::string,std::string> &ptr = properties;
	return ptr;
}

void XPathProcessor::exceptionClear(){
    SaxonProcessor::attachCurrentThread();
    SaxonProcessor::sxn_environ->env->ExceptionClear();
    if(exception != nullptr) {
        delete exception;
        exception = nullptr;

    }

}

const char * XPathProcessor::getErrorCode() {
	if(exception == nullptr) {return nullptr;}
	return exception->getErrorCode();
}

const char * XPathProcessor::getErrorMessage(){
	if(exception == nullptr) {return nullptr;}
	return exception->getMessage();
}

    SaxonApiException * XPathProcessor::getException() {
        return exception;

}

bool XPathProcessor::exceptionOccurred() {
    return proc->exceptionOccurred() || exception != nullptr;
}



void XPathProcessor::createException(const char * message) {
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



