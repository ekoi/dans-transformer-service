// XsltExecutable.cpp : Defines the exported functions for the DLL application.
//

#include "XsltExecutable.h"
#include "XdmValue.h"
#include "XdmItem.h"
#include "XdmNode.h"
#include "XdmFunctionItem.h"
#include "XdmMap.h"
#include "XdmArray.h"
#include "XdmAtomicValue.h"

#ifdef DEBUG
#include <typeinfo> //used for testing only
#endif

#ifdef MEM_DEBUG
#define new new(__FILE__, __LINE__)
#endif

XsltExecutable::XsltExecutable(jobject exObject, std::string curr, std::string resources) {



    /*
     * Look for class.
     */
    cppClass = lookForClass(SaxonProcessor::sxn_environ->env,
                            "net/sf/saxon/option/cpp/Xslt30Processor");

    messageListenerClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                               "net/sf/saxon/option/cpp/SaxonCMessageListener");

    resultDocumentClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                               "net/sf/saxon/option/cpp/SaxonCResultDocumentHandler");

#ifdef DEBUG
    jmethodID debugMID = SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass, "setDebugMode", "(Z)V");
    SaxonProcessor::sxn_environ->env->CallStaticVoidMethod(cppClass, debugMID, (jboolean)true);

#endif
    tunnel = false;
    rawResultsFlag = false;
    selection = nullptr;
    selectionV = nullptr;
    saxonMessageListenerObj = nullptr;
    saxonResultDocObj = nullptr;
    executableObject = SaxonProcessor::sxn_environ->env->NewGlobalRef(exObject);
    cwdXE = curr;
    resources_dir = resources;
}

XsltExecutable::~XsltExecutable() {
    clearProperties();
    clearParameters();

    SaxonProcessor::sxn_environ->env->DeleteGlobalRef(executableObject);
    if(saxonMessageListenerObj != nullptr){
        SaxonProcessor::sxn_environ->env->DeleteGlobalRef(saxonMessageListenerObj);
    }
    if(saxonResultDocObj != nullptr) {
        SaxonProcessor::sxn_environ->env->DeleteGlobalRef(saxonResultDocObj);
    }
    SaxonProcessor::sxn_environ->env->ExceptionClear(); // This is as a precaution to avoid crashing out if JNI gets into an unclean state.
}


XsltExecutable::XsltExecutable(const XsltExecutable &other) {

    /*
     * Look for class.
     */
    cppClass = lookForClass(SaxonProcessor::sxn_environ->env,
                            "net/sf/saxon/option/cpp/Xslt30Processor");

    executableObject = other.executableObject;
    selectionV = other.selectionV; 
    cwdXE = other.cwdXE;
    resources_dir = other.resources_dir;
    if (selectionV != nullptr) {
        setInitialMatchSelection(other.selectionV);
    } else {
        if(selection) {
            selection = SaxonProcessor::sxn_environ->env->NewGlobalRef(other.selection);
        }
    }
    tunnel = other.tunnel;
    rawResultsFlag = other.rawResultsFlag;

    std::map<std::string, XdmValue *>::const_iterator paramIter = other.parameters.begin();
    while (paramIter != other.parameters.end()) {

        XdmValue *valuei = paramIter->second;
        if (valuei == nullptr) {
            std::cerr << "Error in XsltExecutable copy constructor - parameter value is NULL" << std::endl;
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


bool XsltExecutable::exceptionOccurred() {
    return SaxonProcessor::sxn_environ->env->ExceptionCheck();
}


void XsltExecutable::setGlobalContextItem(XdmItem *value) {
    if (value != nullptr) {
        value->incrementRefCount();
        parameters["node"] = value;
    }
}

void XsltExecutable::setGlobalContextFromFile(const char *ifile) {
    if (ifile != nullptr) {
        setProperty("s", ifile);
    }
}

void XsltExecutable::setInitialMatchSelection(XdmValue *_selection) {
    SaxonProcessor::attachCurrentThread();
    if(selection != nullptr) {
        SaxonProcessor::sxn_environ->env->DeleteGlobalRef(selection);
        selection = nullptr;
    }
    if(selectionV != nullptr) {
	selectionV->decrementRefCount();
    }
//    if (_selection != nullptr) {
        //_selection->incrementRefCount();
        selectionV = _selection;
        if(_selection != nullptr) {
            selection = SaxonProcessor::sxn_environ->env->NewGlobalRef(_selection->getUnderlyingValue());
        }
//    }
}


void XsltExecutable::setInitialMatchSelectionAsFile(const char *filename) {
    SaxonProcessor::attachCurrentThread();
    if(selection != nullptr) {
        SaxonProcessor::sxn_environ->env->DeleteGlobalRef(selection);
        selection = nullptr;
    }
    if (filename != nullptr) {
        selection = SaxonProcessor::sxn_environ->env->NewGlobalRef(SaxonProcessor::sxn_environ->env->NewStringUTF(filename));

    }   else {
       selection = nullptr;
    }
}

void XsltExecutable::setOutputFile(const char *ofile) {
    setProperty("o", ofile);
}

void XsltExecutable::setBaseOutputURI(const char *baseURI) {
    if (baseURI != nullptr) {
        setProperty("baseoutput", baseURI);
    }
}

void XsltExecutable::setInitialMode(const char * modeName){
    if(modeName != nullptr) {
        setProperty("im", modeName);
    }

}


void XsltExecutable::setParameter(const char *name, XdmValue *value) {
    if(value != nullptr && name != nullptr) {
        std::string skey = ("param:" + std::string(name));
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

void XsltExecutable::setInitialTemplateParameters(std::map<std::string, XdmValue *> _itparameters, bool _tunnel) {
    for (std::map<std::string, XdmValue *>::iterator itr = _itparameters.begin(); itr != _itparameters.end(); itr++) {
        parameters["itparam:" + std::string(itr->first)] = itr->second;
        itr->second->incrementRefCount();
    }
    tunnel = _tunnel;
    if (tunnel) {
        setProperty("tunnel", "true");
    }
}

void XsltExecutable::setCaptureResultDocuments(bool flag, bool rawResults) {
    SaxonProcessor::attachCurrentThread();
    if(flag) {
        resultDocumentMap.clear();
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteGlobalRef(saxonResultDocObj);
            saxonResultDocObj = nullptr;
        }
        static jmethodID resultDocID =   nullptr;
        if(resultDocID == nullptr) {
            resultDocID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(resultDocumentClass,
                                                                                  "<init>", "(Z)V");
        }
        if (!resultDocID) {
            std::cerr << "Error: SaxonCResultDocumentHandler" << " in " <<getDllname() << " not found\n"
                      << std::endl;
            SaxonProcessor::sxn_environ->env->ExceptionClear();
            return;
        }

        rawResultsFlag = rawResults || rawResultsFlag;


        jobject tempResultDocObj = (jobject)SaxonProcessor::sxn_environ->env->NewObject(resultDocumentClass, resultDocID, rawResultsFlag);

        if(tempResultDocObj) {
            saxonResultDocObj = SaxonProcessor::sxn_environ->env->NewGlobalRef(tempResultDocObj);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(tempResultDocObj);
        } else {
            std::cerr << "Error: Failed to create SaxonCResultDocumentHandler" <<  std::endl;
        }
    } else {
        setProperty("rd", "off");
        
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteGlobalRef(saxonResultDocObj);
            saxonResultDocObj = nullptr;
        }


          while (!resultDocumentMap.empty())
          {
            delete resultDocumentMap.begin()->second;
            resultDocumentMap.erase(resultDocumentMap.begin());
          }
    }


}



std::map<std::string,XdmValue*>& XsltExecutable::getResultDocuments(){
        SaxonProcessor::attachCurrentThread();
        if(saxonResultDocObj) {
            static jmethodID getresultID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(resultDocumentClass,
                                                                                                      "getResultDocuments",
                                                                                                      "()[Lnet/sf/saxon/s9api/XdmValue;");

            static jmethodID getresult_uriID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(resultDocumentClass,
                                                                                                      "getResultDocumentURIs",
                                                                                                      "()[Ljava/lang/String;");
            if (!getresultID) {
                std::cerr << "Error: " << getDllname() << ".getResultDocuments" << " not found\n"
                          << std::endl;
                SaxonProcessor::sxn_environ->env->ExceptionClear();
            } else {
                jobjectArray results = (jobjectArray) (
                        SaxonProcessor::sxn_environ->env->CallObjectMethod(saxonResultDocObj, getresultID));
                jobjectArray resultUris = (jobjectArray) (
                        SaxonProcessor::sxn_environ->env->CallObjectMethod(saxonResultDocObj, getresult_uriID));
                if(!results || !resultUris ) {
                    return resultDocumentMap;
                }

                int sizex = SaxonProcessor::sxn_environ->env->GetArrayLength(results);
                jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmAtomicValue");
                jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
                jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmFunctionItem");
                jclass mapItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                   "net/sf/saxon/s9api/XdmMap");
                jclass arrayItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                     "net/sf/saxon/s9api/XdmArray");

                if (sizex > 0) {
                    for (int p = 0; p < sizex; ++p) {
                        jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);
                        jobject resultURIsi = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(resultUris, p);
                        jstring sresultURI = (jstring)resultURIsi;
                        const char * str = SaxonProcessor::sxn_environ->env->GetStringUTFChars(sresultURI,
                                        NULL);


                        XdmValue * value= nullptr;
                        XdmItem * xdmItem = nullptr;
                        if(rawResultsFlag) {
                            value = new XdmValue();

                            if(SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, atomicValueClass)           == JNI_TRUE) {
                                xdmItem = new XdmAtomicValue(resulti);
                                value->addXdmItem(xdmItem);


                            } else if(SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, nodeClass)           == JNI_TRUE) {
                                xdmItem = new XdmNode(resulti);
                                value->addXdmItem(xdmItem);


                            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, mapItemClass) == JNI_TRUE) {
                                xdmItem = new XdmMap(resulti);
                                value->addXdmItem(xdmItem);

                            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, arrayItemClass) == JNI_TRUE) {
                                xdmItem = new XdmArray(resulti);
                                value->addXdmItem(xdmItem);

                            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, functionItemClass)           == JNI_TRUE) {
                                xdmItem = new XdmFunctionItem(resulti);
                                value->addXdmItem(xdmItem);

                            } else {
                                value = new XdmValue(resulti, true);

                            }

                        } else {
                            value = new XdmValue();
                            value->addXdmItem(new XdmNode(resulti));
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);

                        }
                        resultDocumentMap[str] = value;
                        SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                        SaxonProcessor::sxn_environ->env->DeleteLocalRef(resultURIsi);

                    }
                    SaxonProcessor::sxn_environ->env->DeleteLocalRef(results);

                }
            }
        }
       return resultDocumentMap;
}


XdmValue *XsltExecutable::getParameter(const char *name) {
    std::map<std::string, XdmValue *>::iterator it;
    it = parameters.find("param:" + std::string(name));
    if (it != parameters.end())
        return it->second;
    else {
        it = parameters.find("sparam:" + std::string(name));
        if (it != parameters.end())
            return it->second;
    }
    return nullptr;
}

bool XsltExecutable::removeParameter(const char *name) {
    return (bool) (parameters.erase("param:" + std::string(name)));
}


void XsltExecutable::setResultAsRawValue(bool option) {
    if (option) {
        setProperty("outvalue", "yes");
        rawResultsFlag = true;
    } else {
        removeProperty("outvalue");
        rawResultsFlag = false;
    }
}

XsltExecutable * XsltExecutable::clone() {
    XsltExecutable * executable = new XsltExecutable(*this);
    return executable;

}

void XsltExecutable::setProperty(const char *name, const char *value) {
#ifdef DEBUG
    if(value == nullptr) {
		std::cerr<<"XQueryProc setProperty is nullptr"<<std::endl;
	}
#endif
    if (name != nullptr) {
        properties.erase(name);
        std::string skey = std::string(name);
        properties[skey] = std::string((value == nullptr ? "" : value));
    }

}



bool XsltExecutable::removeProperty(const char *name) {
    return (bool) (properties.erase(std::string(name)));
}

const char *XsltExecutable::getProperty(const char *name) {
    std::map<std::string, std::string>::iterator it;
    it = properties.find(std::string(name));
    if (it != properties.end())
        return it->second.c_str();
    return nullptr;
}

void XsltExecutable::clearParameters(bool delValues) {
    if (delValues) {
        for (std::map<std::string, XdmValue *>::iterator itr = parameters.begin(); itr != parameters.end(); itr++) {

            XdmValue *value = itr->second;
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

void XsltExecutable::clearProperties() {
    SaxonProcessor::attachCurrentThread();
    properties.clear();
    if(selection != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteGlobalRef(selection);
            selection = nullptr;
    }

}


std::map<std::string, XdmValue *> &XsltExecutable::getParameters() {
    std::map<std::string, XdmValue *> &ptr = parameters;
    return ptr;
}

std::map<std::string, std::string> &XsltExecutable::getProperties() {
    std::map<std::string, std::string> &ptr = properties;
    return ptr;
}

void XsltExecutable::exceptionClear() {
    SaxonProcessor::attachCurrentThread();
    SaxonProcessor::sxn_environ->env->ExceptionClear();
}

void XsltExecutable::setcwd(const char *dir) {
    if (dir != nullptr) {
        cwdXE = std::string(dir);
    }
}


void XsltExecutable::exportStylesheet(const char *filename) {
    SaxonProcessor::attachCurrentThread();

    static jmethodID exportmID = nullptr;

    if (!exportmID) {
        exportmID = SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass, "save",
                                                                        "(Ljava/lang/String;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;)V");
    }

    if (filename == nullptr) {
        std::cerr << "Error: Error: export file name is nullptr" << std::endl;
        return;
    }
    SaxonProcessor::sxn_environ->env->CallStaticVoidMethod(cppClass, exportmID,
                                                           SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                   cwdXE.c_str()),
                                                           executableObject,
                                                           SaxonProcessor::sxn_environ->env->NewStringUTF(filename));

}

void XsltExecutable::applyTemplatesReturningFile(const char *output_filename) {
    SaxonProcessor::attachCurrentThread();
    if (selection == nullptr) {
        std::cerr
                << "Error: The initial match selection has not been set. Please set it using setInitialMatchSelection or setInitialMatchSelectionFile."
                << std::endl;
        return;
    }

    static jmethodID atmID = nullptr;

    if (atmID == nullptr) {
        atmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                        "applyTemplatesReturningFile",
                                                                        "(Ljava/lang/String;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/Object;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)V");
    }

    if (!atmID) {
        std::cerr << "Error: " << getDllname() << ".applyTemplatesReturningFile" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
    } else {
    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        }         
        SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, atmID,
                                                                 SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                         cwdXE.c_str()), executableObject, selection,
                                                                 (output_filename != nullptr
                                                                  ? SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                 output_filename) : nullptr),
                                                                 comboArrays.stringArray, comboArrays.objectArray);
        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }

    }
    return;

}

const char *XsltExecutable::applyTemplatesReturningString() {
    SaxonProcessor::attachCurrentThread();

    if (selection == nullptr) {
        std::cerr
                << "Error: The initial match selection has not been set. Please set it using setInitialMatchSelection or setInitialMatchSelectionFile."
                << std::endl;
        return nullptr;
    }


    static jmethodID atsmID = nullptr;

    if (atsmID == nullptr) {
        atsmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                             "applyTemplatesReturningString",
                                                                             "(Ljava/lang/String;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/Object;[Ljava/lang/String;[Ljava/lang/Object;)[B");
    }
    if (!atsmID) {
        std::cerr << "Error: " << getDllname() << "applyTemplatesAsString" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
    } else {
    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        }

        jbyteArray result = nullptr;
        jobject obj = (SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, atsmID,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                        cwdXE.c_str()), executableObject,selection,
                comboArrays.stringArray, comboArrays.objectArray));

        if (obj) {
            result = (jbyteArray) obj;
        }
        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
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
        }
    }
    return nullptr;


}

XdmValue *XsltExecutable::applyTemplatesReturningValue() {
    SaxonProcessor::attachCurrentThread();

    static jmethodID atsvmID = nullptr;
    if (atsvmID == nullptr) {
        atsvmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                                 "applyTemplatesReturningValue",
                                                                                 "(Ljava/lang/String;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/Object;[Ljava/lang/String;[Ljava/lang/Object;)Lnet/sf/saxon/s9api/XdmValue;");
    }
    if (!atsvmID) {
        std::cerr << "Error: " << getDllname() << "applyTemplatesReturningValue" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
    } else {
    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
        //TODO bug here
            comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        }


        // jstring result = nullptr;
        jobject result = (jobject) (SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, atsvmID,
                                                                                             SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                                     cwdXE.c_str()),
                                                                                             executableObject,
                                                                                             selection,
                                                                                             comboArrays.stringArray,
                                                                                             comboArrays.objectArray));
        /*if(obj) {
            result = (jobject)obj;
        }*/
        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }
        if (result) {
            jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                   "net/sf/saxon/s9api/XdmAtomicValue");
            jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
            jclass arrayClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmArray");
            jclass mapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
            jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                    "net/sf/saxon/s9api/XdmFunctionItem");
            XdmValue *value = new XdmValue();
            XdmItem *xdmItem = nullptr;


            if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, atomicValueClass) == JNI_TRUE) {
                xdmItem = new XdmAtomicValue(result);

                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;

            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, nodeClass) == JNI_TRUE) {
                xdmItem = new XdmNode(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            }  else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, arrayClass) == JNI_TRUE) {
                xdmItem = new XdmArray(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, mapClass) == JNI_TRUE) {
                xdmItem = new XdmMap(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, functionItemClass) == JNI_TRUE) {
                xdmItem = new XdmFunctionItem(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            } else {
                value = new XdmValue(result, true);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                return value;
            }
        }
    }
    return nullptr;

}


void XsltExecutable::callFunctionReturningFile(const char *functionName, XdmValue **arguments, int argument_length,
                                               const char *outfile) {

    SaxonProcessor::attachCurrentThread();
    static jmethodID afmID = nullptr;

    if (afmID == nullptr) {
        afmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                                "callFunctionReturningFile",
                                                                                "(Ljava/lang/String;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/Object;[Ljava/lang/String;[Ljava/lang/Object;)V");
    }

    if (!afmID) {
        std::cerr << "Error: " << getDllname() << "callFunctionReturningFile" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
        return;
    } else {
    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
            comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        }

        jobjectArray argumentJArray = SaxonProcessor::createJArray(arguments, argument_length);

        SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, afmID,
                                                                 SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                         cwdXE.c_str()),
                                                                 executableObject,
                                                                 (functionName != nullptr ?
                                                                  SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                          functionName) :
                                                                  nullptr), argumentJArray,
                                                                 (outfile != nullptr ?
                                                                  SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                          outfile) :
                                                                  nullptr),
                                                                 comboArrays.stringArray, comboArrays.objectArray);
        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }
        if (argumentJArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(argumentJArray);
        }

    }
    return;


}

const char *
XsltExecutable::callFunctionReturningString(const char *functionName, XdmValue **arguments, int argument_length) {

    SaxonProcessor::attachCurrentThread();
    static jmethodID afsmID = nullptr;

    if(afsmID == nullptr) {
       afsmID =  (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                        "callFunctionReturningString",
                                                                        "(Ljava/lang/String;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;[Ljava/lang/Object;[Ljava/lang/String;[Ljava/lang/Object;)[B");
    }
    if (!afsmID) {
        std::cerr << "Error: " << getDllname() << ".callFunctionReturningString" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
    } else {
    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        }
        jobjectArray argumentJArray = SaxonProcessor::createJArray(arguments, argument_length);

        jbyteArray result = nullptr;
        jobject obj = (SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, afsmID,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                        cwdXE.c_str()), executableObject,
                                                                                        (functionName != nullptr
                                                                                         ? SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                                        functionName)
                                                                                         : nullptr),
                                                                                argumentJArray, comboArrays.stringArray,
                                                                                comboArrays.objectArray));
        if (obj) {
            result = (jbyteArray) obj;
        }
        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }
        if (result) {
            jboolean isCopy = false;
            jbyte* b = SaxonProcessor::sxn_environ->env->GetByteArrayElements(result, &isCopy);
            jsize num_bytes = SaxonProcessor::sxn_environ->env->GetArrayLength(result);

            char* str = new char[num_bytes + 1];
            //char *str = new char[num_bytes+1];
            memcpy ( str, b , num_bytes );
            str[num_bytes] = '\0';

             SaxonProcessor::sxn_environ->env->ReleaseByteArrayElements( result, b, 0);
             SaxonProcessor::sxn_environ->env->DeleteLocalRef(obj);
            return str;
        }
    }
    return nullptr;
}


XdmValue *
XsltExecutable::callFunctionReturningValue(const char *functionName, XdmValue **arguments, int argument_length) {

    SaxonProcessor::attachCurrentThread();
    static jmethodID cfvmID = nullptr;
    if (cfvmID == nullptr) {
        cfvmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                                 "callFunctionReturningValue",
                                                                                 "(Ljava/lang/String;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;[Ljava/lang/Object;[Ljava/lang/String;[Ljava/lang/Object;)Lnet/sf/saxon/s9api/XdmValue;");
    }
    if (!cfvmID) {
        std::cerr << "Error: " << getDllname() << "callFunctionReturningValue" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
    } else {
    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        } 
        jobjectArray argumentJArray = SaxonProcessor::createJArray(arguments, argument_length);

        jobject result = (jobject) (SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, cfvmID,
                                                                                             SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                                     cwdXE.c_str()),
                                                                                             executableObject,
                                                                                             (functionName != nullptr
                                                                                              ? SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                                             functionName)
                                                                                              : nullptr),
                                                                                             argumentJArray,
                                                                                             comboArrays.stringArray,
                                                                                             comboArrays.objectArray));

        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }
        if (argumentJArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(argumentJArray);
        }
        if (result) {
            jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                   "net/sf/saxon/s9api/XdmAtomicValue");
            jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
            jclass arrayClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmArray");
            jclass mapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
            jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                    "net/sf/saxon/s9api/XdmFunctionItem");
            XdmValue *value = new XdmValue();
            XdmItem *xdmItem = nullptr;

            if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, atomicValueClass) == JNI_TRUE) {
                xdmItem = new XdmAtomicValue(result);

                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;

            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, nodeClass) == JNI_TRUE) {
                xdmItem = new XdmNode(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            }  else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, arrayClass) == JNI_TRUE) {
                xdmItem = new XdmArray(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, mapClass) == JNI_TRUE) {
                xdmItem = new XdmMap(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, functionItemClass) == JNI_TRUE) {
                xdmItem = new XdmFunctionItem(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            } else {
                value = new XdmValue(result, true);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                return value;
            }
        }
    }
    return nullptr;

}


void XsltExecutable::callTemplateReturningFile(const char *templateName, const char *outfile) {

    SaxonProcessor::attachCurrentThread();
    static jmethodID ctmID = nullptr;

    if(ctmID == nullptr) {
        ctmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                        "callTemplateReturningFile",
                                                                        "(Ljava/lang/String;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)V");
    }
    if (!ctmID) {
        std::cerr << "Error: " << getDllname() << ".callTemplateReturningFile" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
    } else {
    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        }
        SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, ctmID,
                                                                 SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                         cwdXE.c_str()),
                                                                 executableObject,
                                                                 (templateName != nullptr
                                                                  ? SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                 templateName) : nullptr),
                                                                 (outfile != nullptr
                                                                  ? SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                 outfile) : nullptr),
                                                                 comboArrays.stringArray, comboArrays.objectArray);
        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }


    }

}


const char * XsltExecutable::callTemplateReturningString(const char *templateName) {
    SaxonProcessor::attachCurrentThread();
    jmethodID ctsmID = nullptr;

	if(exceptionOccurred()) {
		//Possible error detected in the compile phase. Processor not in a clean state.
		//Require clearing exception.
        std::cerr<<"Error: XsltExecutable not in a clean state. - Exception found"<<std::endl;
		return nullptr;
	}

    if(ctsmID == nullptr) {
        ctsmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                        "callTemplateReturningString",
                                                                        "(Ljava/lang/String;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)[B");
    }
    if (!ctsmID) {
        std::cerr << "Error: " << getDllname() << "callTemplateReturningString" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
        return nullptr;

    } else if(!executableObject) {
#if defined(DEBUG)
                std::cerr << "The Java XsltExecutable object (i.e. cppXT) is NULL - Possible exception thrown" << std::endl;
#endif
                return nullptr;
    }

    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        }
        jbyteArray result = nullptr;
        jobject obj = SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, ctsmID,
                                                                                          SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                                  cwdXE.c_str()),
                                                                                          executableObject,
                                                                                          (templateName != nullptr
                                                                                           ? SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                                          templateName)
                                                                                           : NULL),
                                                                                          comboArrays.stringArray,
                                                                                          comboArrays.objectArray);

        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }
        if (obj) {
            result = (jbyteArray) obj;
        }

        if (result) {
            jboolean isCopy = false;
            jbyte* b = SaxonProcessor::sxn_environ->env->GetByteArrayElements(result, &isCopy);
            jsize num_bytes = SaxonProcessor::sxn_environ->env->GetArrayLength(result);

            char* str = new char[num_bytes + 1];
            //char *str = new char[num_bytes+1];
            memcpy ( str, b , num_bytes );
            str[num_bytes] = '\0';

             SaxonProcessor::sxn_environ->env->ReleaseByteArrayElements( result, b, 0);
             SaxonProcessor::sxn_environ->env->DeleteLocalRef(obj);
            return str;
        }
    return nullptr;


}

XdmValue *XsltExecutable::callTemplateReturningValue(const char *templateName) {
    SaxonProcessor::attachCurrentThread();
    static jmethodID ctsvmID = nullptr;
    if (ctsvmID == nullptr) {
        ctsvmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                                 "callTemplateReturningValue",
                                                                                 "(Ljava/lang/String;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)Lnet/sf/saxon/s9api/XdmValue;");
    }
    if (!ctsvmID) {
        std::cerr << "Error: " << getDllname() << "callTemplateReturningValue" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
    } else {
    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        }

        jobject result = (jobject) (SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, ctsvmID,
                                                                                          SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                                  cwdXE.c_str()),
                                                                                          executableObject,
                                                                                          (templateName != nullptr
                                                                                           ? SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                                          templateName)
                                                                                           : nullptr),
                                                                                          comboArrays.stringArray,
                                                                                          comboArrays.objectArray));

         if(!result) {
            std::cerr<<"Error found in callTemplateReturningValue"<<std::endl;
            SaxonProcessor::sxn_environ->env->ExceptionDescribe();
         }

        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }
        if (result) {
            jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                   "net/sf/saxon/s9api/XdmAtomicValue");
            jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
            jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                    "net/sf/saxon/s9api/XdmFunctionItem");
            jclass mapItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                    "net/sf/saxon/s9api/XdmMap");
            jclass arrayItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                               "net/sf/saxon/s9api/XdmArray");
            XdmValue *value = nullptr;

            XdmItem *xdmItem = nullptr;
            value = new XdmValue();
            if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, atomicValueClass) == JNI_TRUE) {
                xdmItem = new XdmAtomicValue(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;

            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, nodeClass) == JNI_TRUE) {
                xdmItem = new XdmNode(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            }  else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, mapItemClass) == JNI_TRUE) {
                xdmItem = new XdmMap(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, arrayItemClass) == JNI_TRUE) {
                xdmItem = new XdmArray(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, functionItemClass) == JNI_TRUE) {
                xdmItem = new XdmFunctionItem(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;

            } else {
                value = new XdmValue(result, true);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                return value;
            }

        }
    }
    return nullptr;
}


XdmValue *XsltExecutable::transformFileToValue(const char *sourcefile) {
    SaxonProcessor::attachCurrentThread();
    if (sourcefile == nullptr) {

        return nullptr;
    }


    static jmethodID tfvMID = nullptr;

    if (tfvMID == nullptr) {
        tfvMID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                                 "transformToValue",
                                                                                 "(Ljava/lang/String;Lnet/sf/saxon/option/cpp/Xslt30Processor;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)Lnet/sf/saxon/s9api/XdmValue;");
    }
    if (!tfvMID) {
        std::cerr << "Error: " << getDllname() << ".transformToValue" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
    } else {
    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        }

        jobject result = (jobject) (
                SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, tfvMID,
                                                                         SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                 cwdXE.c_str()), NULL, executableObject,
                                                                         (sourcefile != nullptr
                                                                          ? SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                         sourcefile) : NULL),
                                                                         comboArrays.stringArray,
                                                                         comboArrays.objectArray));
        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }
        if (result) {
            jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                   "net/sf/saxon/s9api/XdmAtomicValue");
            jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
            jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                    "net/sf/saxon/s9api/XdmFunctionItem");
            jclass mapItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                    "net/sf/saxon/s9api/XdmMap");
            jclass arrayItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                               "net/sf/saxon/s9api/XdmArray");
            XdmValue *value = nullptr;
            XdmItem *xdmItem = nullptr;


            if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, atomicValueClass) == JNI_TRUE) {
                xdmItem = new XdmAtomicValue(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                return xdmItem;

            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, nodeClass) == JNI_TRUE) {
                xdmItem = new XdmNode(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                return xdmItem;

            }  else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, mapItemClass) == JNI_TRUE) {
                xdmItem = new XdmMap(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                return xdmItem;
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, arrayItemClass) == JNI_TRUE) {
                xdmItem = new XdmArray(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                return xdmItem;
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, functionItemClass) == JNI_TRUE) {
                xdmItem = new XdmFunctionItem(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                return xdmItem;
            } else {
                value = new XdmValue(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                return value;
            }
        }
    }
    return nullptr;

}


void XsltExecutable::transformFileToFile(const char *source, const char *outputfile) {

    SaxonProcessor::attachCurrentThread();
    static jmethodID tffMID = nullptr;

    if (tffMID == nullptr) {
        tffMID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                                 "transformToFile",
                                                                                 "(Ljava/lang/String;Lnet/sf/saxon/option/cpp/Xslt30Processor;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)V");
    }
    if (!tffMID) {
        std::cerr << "Error: " << getDllname() << "transformToFile" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
    } else {
    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        }

        SaxonProcessor::sxn_environ->env->CallStaticVoidMethod(cppClass, tffMID,
                                                               SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                       cwdXE.c_str()), NULL, executableObject,
                                                                       (source != nullptr
                                                                        ? SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                       source) : nullptr), NULL,
                                                               (outputfile != nullptr
                                                                ? SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                               outputfile) : nullptr),
                                                               comboArrays.stringArray, comboArrays.objectArray);
        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }
    }

}


void XsltExecutable::setSaveXslMessage(bool create, const char * filename) {
    SaxonProcessor::attachCurrentThread();
    if (create) {

        static jmethodID messageID =   nullptr;
        if(messageID == nullptr) {
            messageID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(messageListenerClass,
                                                                                  "<init>",
                                                                                  "(Ljava/lang/String;Ljava/lang/String;)V");
        }
        if (!messageID) {
            std::cerr << "Error: SaxonCMessageListener" << " in " <<getDllname() << " not found\n"
                      << std::endl;
            SaxonProcessor::sxn_environ->env->ExceptionClear();
        }

        jobject tempListenerObj;
        if (filename == nullptr) {
            tempListenerObj = (jobject)SaxonProcessor::sxn_environ->env->NewObject(messageListenerClass, messageID,
                                                                                           SaxonProcessor::sxn_environ->env->NewStringUTF("-:on"));
            //setProperty("m", "on");
        } else {
            tempListenerObj = (jobject)SaxonProcessor::sxn_environ->env->NewObject(messageListenerClass, messageID,
                                                                                           SaxonProcessor::sxn_environ->env->NewStringUTF(filename));
        }
        if(tempListenerObj) {
            saxonMessageListenerObj = SaxonProcessor::sxn_environ->env->NewGlobalRef(tempListenerObj);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(tempListenerObj);
        } else {
            std::cerr << "Error: Failed to create SaxonCMessageListener" <<  std::endl;
        }
    } else {
        setProperty("m", "off");
        if(saxonMessageListenerObj != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteGlobalRef(saxonMessageListenerObj);
            saxonMessageListenerObj = nullptr;
        }
    }


}

/*
XdmValue * XsltExecutable::getXslMessages(){

    if(saxonMessageListenerObj) {
        static jmethodID getmessageID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(messageListenerClass,
                                                                                                  "getXslMessages",
                                                                                                  "()[Lnet/sf/saxon/s9api/XdmNode;");
        if (!getmessageID) {
            std::cerr << "Error: " << getDllname() << ".getXslMessages" << " not found\n"
                      << std::endl;
            SaxonProcessor::sxn_environ->env->ExceptionClear();
        } else {
            jobjectArray results = (jobjectArray) (
                    SaxonProcessor::sxn_environ->env->CallObjectMethod(saxonMessageListenerObj, getmessageID));
            int sizex = SaxonProcessor::sxn_environ->env->GetArrayLength(results);

            if (sizex > 0) {
                XdmValue *value = new XdmValue();

                for (int p = 0; p < sizex; ++p) {
                    jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);
                    value->addUnderlyingValue(resulti);
                }
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(results);
                return value;
            }
        }
    }
    return nullptr;


}
*/

const char *XsltExecutable::transformFileToString(const char *source) {

    SaxonProcessor::attachCurrentThread();
    static jmethodID tftMID = tftMID;
    if (tftMID == nullptr) {
        tftMID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass,
                                                                                 "transformToString",
                                                                                 "(Ljava/lang/String;Lnet/sf/saxon/option/cpp/Xslt30Processor;Lnet/sf/saxon/s9api/XsltExecutable;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)[B");
    }
    if (!tftMID) {
        std::cerr << "Error: " << getDllname() << "transformToString" << " not found\n"
                  << std::endl;
        SaxonProcessor::sxn_environ->env->ExceptionClear();
    } else {
    	setProperty("resources", resources_dir.c_str());
        JParameters comboArrays;
        int additions = 0;
        int iadd = 0;
        if(saxonMessageListenerObj != nullptr ) {
            additions++;
        }
        if(saxonResultDocObj != nullptr) {
            additions++;
        }
        comboArrays = SaxonProcessor::createParameterJArray(parameters, properties, additions);
        if(saxonMessageListenerObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size(),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("m"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size(),saxonMessageListenerObj);
            iadd++;
        }
        if(saxonResultDocObj != nullptr) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.stringArray, parameters.size() + properties.size()+iadd,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF("rd"));
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(comboArrays.objectArray, parameters.size() + properties.size()+iadd,saxonResultDocObj);
        }

        jbyteArray result = nullptr;
        jobject obj = SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(cppClass, tftMID,
                                                                               SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                       cwdXE.c_str()), NULL,
                                                                               executableObject,
                                                                               (source != nullptr
                                                                                ? SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                               source) : NULL),
                                                                               NULL,
                                                                               comboArrays.stringArray,
                                                                               comboArrays.objectArray);
        if (comboArrays.stringArray != nullptr) {
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.stringArray);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(comboArrays.objectArray);
        }
        if (obj) {
            result = (jbyteArray) obj;
        }
        if (result) {

            jboolean isCopy = false;
            jbyte* b = SaxonProcessor::sxn_environ->env->GetByteArrayElements(result, &isCopy);
            jsize num_bytes = SaxonProcessor::sxn_environ->env->GetArrayLength(result);

            char * str = new char[num_bytes+1];
            memcpy ( str, b , num_bytes );
            str[num_bytes] = '\0';

            SaxonProcessor::sxn_environ->env->ReleaseByteArrayElements( result, b, 0);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(obj);
            return str;
        }
    }
    return nullptr;
}


const char *XsltExecutable::transformToString(XdmNode *source) {

    if (source != nullptr) {
        //source->incrementRefCount();
        parameters["node"] = source;
    }
    return transformFileToString(nullptr);
}


XdmValue *XsltExecutable::transformToValue(XdmNode *source) {

    if (source != nullptr) {
        //source->incrementRefCount();
        parameters["node"] = source;
    }
    return transformFileToValue(nullptr);
}

void XsltExecutable::transformToFile(XdmNode *source) {

    if (source != nullptr) {
        //source->incrementRefCount();
        parameters["node"] = source;
    }
    transformFileToFile(nullptr, nullptr);
}


SaxonApiException *XsltExecutable::getException() {
    SaxonProcessor::attachCurrentThread();
    return SaxonProcessor::checkForExceptionCPP(SaxonProcessor::sxn_environ->env, cppClass, nullptr);
}


const char * XsltExecutable::getErrorMessage() {
    SaxonApiException * exception = getException();
    if(exception != nullptr) {
        std::string str = exception->getMessageStr();
        if(str.length() > 0) {
            char * cstr = new char [str.length()+1];
            strcpy (cstr, str.c_str());
            delete exception;
            return cstr;
        }
    }

    return nullptr;
}
