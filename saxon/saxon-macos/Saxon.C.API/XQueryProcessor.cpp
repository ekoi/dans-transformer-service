#include "XQueryProcessor.h"
#include "XdmValue.h"
#include "XdmItem.h"
#include "XdmNode.h"
#include "XdmAtomicValue.h"
#include "XdmFunctionItem.h"
#include "XdmMap.h"
#include "XdmArray.h"


#ifdef MEM_DEBUG
#define new new(__FILE__, __LINE__)
#endif

    XQueryProcessor::XQueryProcessor() {
	SaxonProcessor *p = new SaxonProcessor(false);
	XQueryProcessor(p, "");
     }


    XQueryProcessor::~XQueryProcessor(){
        SaxonProcessor::attachCurrentThread();
        clearProperties();
        clearParameters();
        if(!cppXQ) {
            SaxonProcessor::sxn_environ->env->DeleteGlobalRef(cppXQ);
        }
        cwdXQ.erase();
        exceptionClear();
    }

    XQueryProcessor::XQueryProcessor(SaxonProcessor *p, std::string curr) {
        SaxonProcessor::attachCurrentThread();
    proc = p;
        exception = nullptr;
     cppClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XQueryEngine");


    jobject tempcppXQ = createSaxonProcessor2 (SaxonProcessor::sxn_environ->env, cppClass, "(Lnet/sf/saxon/s9api/Processor;)V", proc->proc);
    if(tempcppXQ) {
        cppXQ = SaxonProcessor::sxn_environ->env->NewGlobalRef(tempcppXQ);
    } else {
        createException("Error: Failed to create the Xslt30Processor internal object");
    }
    
#ifdef DEBUG
	jmethodID debugMID = SaxonProcessor::sxn_environ->env->GetStaticMethodID(cppClass, "setDebugMode", "(Z)V");
	SaxonProcessor::sxn_environ->env->CallStaticVoidMethod(cppClass, debugMID, (jboolean)true);
#endif


   // outputfile1 = "";
	if(!(proc->cwd.empty()) && curr.empty()){
		cwdXQ = proc->cwd;
	} else {
		cwdXQ = curr;
	}
}



XQueryProcessor::XQueryProcessor(const XQueryProcessor &other) {
    cwdXQ = other.cwdXQ;
	proc = other.proc; //TODO check thread safety
	cppClass = other.cppClass;
	cppXQ = SaxonProcessor::sxn_environ->env->NewGlobalRef(other.cppXQ);

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


XQueryProcessor * XQueryProcessor::clone() {
      XQueryProcessor * proc = new XQueryProcessor(*this);
      return proc;


}


std::map<std::string,XdmValue*>& XQueryProcessor::getParameters(){
	std::map<std::string,XdmValue*>& ptr = parameters;
	return ptr;
}

std::map<std::string,std::string>& XQueryProcessor::getProperties(){
	std::map<std::string,std::string> &ptr = properties;
	return ptr;
}


    /**
     * Set the source document for the query
    */
    void XQueryProcessor::setContextItem(XdmItem * value){
    	if(value != nullptr){
	 value->incrementRefCount();
     	 parameters["node"] = (XdmValue *)value;
    	}
    }


     void XQueryProcessor::declareNamespace(const char *prefix, const char * uri){
        if (prefix == nullptr || uri == nullptr) {
		    return;
        }  else {
            //setProperty("ns-prefix", uri);
             int s = properties.size();
             std::string skey = std::string("ns-prefix:") + prefix;
             properties.insert(std::pair<std::string, std::string>(skey, std::string(uri)));

             if(s == properties.size()) {
                 std::map<std::string, std::string>::iterator it;
                 it = properties.find(skey);
                 if (it != properties.end()) {
                       properties.erase(skey);
                       properties[skey] = std::string(uri);
                 }
             }

        }
}


    /**
     * Set the source document for the query
    */
    void XQueryProcessor::setContextItemFromFile(const char * ifile){
	setProperty("s", ifile);
    }

    /**
     * Set the output file where the result is sent
    */
    void XQueryProcessor::setOutputFile(const char* ofile){
      // outputfile1 = std::string(ofile); 
       setProperty("o", ofile);
    }

    /**
     * Set a parameter value used in the query
     *
     * @param name  of the parameter, as a string
     * @param value of the query parameter, or nullptr to clear a previously set value
     */
    void XQueryProcessor::setParameter(const char * name, XdmValue*value){
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


    /**
     * Remove a parameter (name, value) pair
     *
     * @param namespacei currently not used
     * @param name  of the parameter
     * @return bool - outcome of the romoval
     */
    bool XQueryProcessor::removeParameter(const char * name){
	return (bool)(parameters.erase("param:"+std::string(name)));
    }
    /**
     * Set a property.
     *
     * @param name of the property
     * @param value of the property
     */
    void XQueryProcessor::setProperty(const char * name, const char * value){
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

    void XQueryProcessor::clearParameters(bool delVal){
	if(delVal){
       		for(std::map<std::string, XdmValue*>::iterator itr = parameters.begin(); itr != parameters.end(); itr++) {
                XdmValue *value = itr->second;
                if (value != nullptr) {
#ifdef DEBUG
                    std::cerr<<"XQueryProc.clearParameter() - XdmValue refCount="<<value->getRefCount()<<std::endl;
#endif
                        delete value;
                        value = nullptr;
                }
            }
	}
	parameters.clear();
    }

   void XQueryProcessor::clearProperties(){
	properties.clear();
        //outputfile1.clear();
   }


   void XQueryProcessor::setcwd(const char* dir){
    cwdXQ = std::string(dir);
   }

    void XQueryProcessor::setQueryBaseURI(const char * baseURI){
	setProperty("base", baseURI);
    }


    void XQueryProcessor::setUpdating(bool updating){
        SaxonProcessor::attachCurrentThread();
    	jmethodID mID =
    		(jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(cppClass, "setUpdating",
    				"(Z)V");
    	if (!mID) {
    	std::cerr << "Error: Saxonc library." << "setUpdating" << " not found\n"
    			<< std::endl;

    	} else {

    			SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXQ, mID,
    					(jboolean)updating);
    	}

    }

    void XQueryProcessor::executeQueryToFile(const char * infilename, const char * ofilename, const char * query){
        SaxonProcessor::attachCurrentThread();

	jmethodID mID = (jmethodID)SaxonProcessor::sxn_environ->env->GetMethodID (cppClass,"executeQueryToFile", "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)V");
 	if (!mID) {
        std::cerr<<"Error: "<<getDllname()<<".executeQueryToFile"<<" not found\n"<<std::endl;
    } else {
        if(cppXQ == nullptr) {
#if defined(DEBUG)
            std::cerr << "The Java XQueryEngine object (i.e. cppXQ) is NULL - Possible exception thrown" << std::endl;
#endif
            createException("The Java XQueryEngine object (i.e. cppXQ) is NULL - Possible exception thrown");
            return;
        }
	setProperty("resources", proc->getResourcesDirectory());
	jobjectArray stringArray = nullptr;
	jobjectArray objectArray = nullptr;

	int size = parameters.size() + properties.size();
	if(query!= nullptr) size++;
	if(infilename!= nullptr) size++;
	if(size >0) {

	   int i=0;
	   jclass objectClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/Object");
	   jclass stringClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/String");
	   objectArray = SaxonProcessor::sxn_environ->env->NewObjectArray( (jint)size, objectClass, 0 );
	   stringArray = SaxonProcessor::sxn_environ->env->NewObjectArray( (jint)size, stringClass, 0 );
	   if(query!= nullptr) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF("qs") );
     	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF(query));
	     i++;	
	   }
	   if(infilename!= nullptr) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF("s") );
     	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF(infilename));
	     i++;	
	   }
	   for(std::map<std::string, XdmValue* >::iterator iter=parameters.begin(); iter!=parameters.end(); ++iter, i++) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF( (iter->first).c_str() ) );
		bool checkCast = SaxonProcessor::sxn_environ->env->IsInstanceOf((iter->second)->getUnderlyingValue(), lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmValueForCpp") );

	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, (jobject)((iter->second)->getUnderlyingValue()) );
	   }
  	   for(std::map<std::string, std::string >::iterator iter=properties.begin(); iter!=properties.end(); ++iter, i++) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF( (iter->first).c_str()  ));
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, (jobject)(SaxonProcessor::sxn_environ->env->NewStringUTF((iter->second).c_str())) );
	   }
	}

	 SaxonProcessor::sxn_environ->env->CallVoidMethod(cppXQ, mID, SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXQ.c_str()), SaxonProcessor::sxn_environ->env->NewStringUTF(ofilename), stringArray, objectArray );
	  SaxonProcessor::sxn_environ->env->DeleteLocalRef(objectArray);
	  SaxonProcessor::sxn_environ->env->DeleteLocalRef(stringArray);

	  if(proc->exceptionOccurred()) {
          createException();
      }
	 
  }


   }


    XdmValue * XQueryProcessor::executeQueryToValue(const char * infilename, const char * query){
        SaxonProcessor::attachCurrentThread();
 jmethodID mID = (jmethodID)SaxonProcessor::sxn_environ->env->GetMethodID (cppClass,"executeQueryToValue", "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)Lnet/sf/saxon/s9api/XdmValue;");
 if (!mID) {
        std::cerr<<"Error: MyClassInDll."<<"executeQueryToValue"<<" not found\n"<<std::endl;
    } else {
     if(cppXQ == nullptr) {
#if defined(DEBUG)
         std::cerr << "The Java XQueryEngine object (i.e. cppXQ) is NULL - Possible exception thrown" << std::endl;
#endif
         createException("The Java XQueryEngine object (i.e. cppXQ) is NULL - Possible exception thrown");
         return nullptr;
     }
	setProperty("resources", proc->getResourcesDirectory());
	jobjectArray stringArray = nullptr;
	jobjectArray objectArray = nullptr;

	int size = parameters.size() + properties.size();
	if(query!= nullptr) size++;
	if(infilename!= nullptr) size++;
	if(size >0) {
	   int i=0;
           jclass objectClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/Object");
	   jclass stringClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/String");
	   objectArray = SaxonProcessor::sxn_environ->env->NewObjectArray( (jint)size, objectClass, 0 );
	   stringArray = SaxonProcessor::sxn_environ->env->NewObjectArray( (jint)size, stringClass, 0 );

	   if(query!= nullptr) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF("qs") );
     	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF(query));
	     i++;	
	   }
	   if(infilename!= nullptr) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF("s") );
     	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF(infilename));
	     i++;	
	   }
	   for(std::map<std::string, XdmValue* >::iterator iter=parameters.begin(); iter!=parameters.end(); ++iter, i++) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF( (iter->first).c_str() ) );
		bool checkCast = SaxonProcessor::sxn_environ->env->IsInstanceOf((iter->second)->getUnderlyingValue(), lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmValueForCpp") );

	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, (jobject)((iter->second)->getUnderlyingValue()) );
	   }
  	   for(std::map<std::string, std::string >::iterator iter=properties.begin(); iter!=properties.end(); ++iter, i++) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF( (iter->first).c_str()  ));
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, (jobject)(SaxonProcessor::sxn_environ->env->NewStringUTF((iter->second).c_str())) );
	   }
	}

	  jobject result = (jobject)(SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXQ, mID, SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXQ.c_str()), stringArray, objectArray ));
	  SaxonProcessor::sxn_environ->env->DeleteLocalRef(objectArray);
	  SaxonProcessor::sxn_environ->env->DeleteLocalRef(stringArray);
    if(result) {
		jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmAtomicValue");
		jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
		jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmFunctionItem");
        jclass mapItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                           "net/sf/saxon/s9api/XdmMap");
        jclass arrayItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                             "net/sf/saxon/s9api/XdmArray");
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

			 } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, mapItemClass) == JNI_TRUE) {
                xdmItem = new XdmMap(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, arrayItemClass) == JNI_TRUE) {
                xdmItem = new XdmArray(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
            } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(result, functionItemClass)           == JNI_TRUE) {
                xdmItem = new XdmFunctionItem(result);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                value->addXdmItem(xdmItem);
                return value;
        } else {
				value = new XdmValue(result, true);
				return value;
			}
     }
	   
	 exception = proc->checkAndCreateException(cppClass);

  }
  return nullptr;

}

    const char * XQueryProcessor::executeQueryToString(const char * infilename, const char * query){
        SaxonProcessor::attachCurrentThread();
        jmethodID mID = (jmethodID)SaxonProcessor::sxn_environ->env->GetMethodID (cppClass,"executeQueryToString", "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/Object;)[B");
        if (!mID) {
            std::cerr<<"Error: MyClassInDll."<<"executeQueryToString"<<" not found\n"<<std::endl;
        } else {
            if(cppXQ == nullptr) {
#if defined(DEBUG)
         std::cerr << "The Java XQueryEngine object (i.e. cppXQ) is NULL - Possible exception thrown" << std::endl;
#endif
         createException("The Java XQueryEngine object (i.e. cppXQ) is NULL - Possible exception thrown");
         return nullptr;
     }
	setProperty("resources", proc->getResourcesDirectory());
	jobjectArray stringArray = nullptr;
	jobjectArray objectArray = nullptr;

	int size = parameters.size() + properties.size();
	if(query!= nullptr) size++;
	if(infilename!= nullptr) size++;
	if(size >0) {
	   int i=0;
           jclass objectClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/Object");
	   jclass stringClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/lang/String");
	   objectArray = SaxonProcessor::sxn_environ->env->NewObjectArray( (jint)size, objectClass, 0 );
	   stringArray = SaxonProcessor::sxn_environ->env->NewObjectArray( (jint)size, stringClass, 0 );

	   if(query!= nullptr) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF("qs") );
     	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF(query));
	     i++;	
	   }
	   if(infilename!= nullptr) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF("s") );
     	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF(infilename));
	     i++;	
	   }
	   for(std::map<std::string, XdmValue* >::iterator iter=parameters.begin(); iter!=parameters.end(); ++iter, i++) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF( (iter->first).c_str() ) );
		bool checkCast = SaxonProcessor::sxn_environ->env->IsInstanceOf((iter->second)->getUnderlyingValue(), lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmValueForCpp") );
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, (jobject)((iter->second)->getUnderlyingValue()) );
	   }
  	   for(std::map<std::string, std::string >::iterator iter=properties.begin(); iter!=properties.end(); ++iter, i++) {
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( stringArray, i, SaxonProcessor::sxn_environ->env->NewStringUTF( (iter->first).c_str()  ));
	     SaxonProcessor::sxn_environ->env->SetObjectArrayElement( objectArray, i, (jobject)(SaxonProcessor::sxn_environ->env->NewStringUTF((iter->second).c_str())) );
	   }
	}

     jbyteArray result = nullptr;
     jobject obj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(cppXQ, mID, SaxonProcessor::sxn_environ->env->NewStringUTF(cwdXQ.c_str()), stringArray, objectArray ));

	  SaxonProcessor::sxn_environ->env->DeleteLocalRef(objectArray);
	  SaxonProcessor::sxn_environ->env->DeleteLocalRef(stringArray);

     if (obj) {
         result = (jbyteArray) obj;
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
     } else {
		    createException();
	   		
     		}
  }
  return nullptr;


    }


    const char * XQueryProcessor::runQueryToString(){
	return executeQueryToString(nullptr, nullptr);

    }


    XdmValue * XQueryProcessor::runQueryToValue(){
	return executeQueryToValue(nullptr, nullptr);
   }

    void XQueryProcessor::runQueryToFile(){
	executeQueryToFile(nullptr, nullptr, nullptr);
   }

    void XQueryProcessor::setQueryFile(const char * ofile){
	   //outputfile1 = std::string(ofile); 
	   setProperty("q", ofile);
    }

   void XQueryProcessor::setQueryContent(const char* content){
	  // outputfile1 = std::string(content); 
	   setProperty("qs", content);
  }



void XQueryProcessor::exceptionClear(){
    SaxonProcessor::attachCurrentThread();
    SaxonProcessor::sxn_environ->env->ExceptionClear();
	if(exception != nullptr) {
		delete exception;
		exception = nullptr;

	}


   
 
}

bool XQueryProcessor::exceptionOccurred(){
    return proc->exceptionOccurred() || exception != nullptr;
}


const char * XQueryProcessor::getErrorCode() {
	if(exception == nullptr) {return nullptr;}
	return exception->getErrorCode();
}

const char * XQueryProcessor::getErrorMessage(){
	if(exception == nullptr) {return nullptr;}
	return exception->getMessage();
}

const char* XQueryProcessor::checkException(){
	/*if(proc->exception == nullptr) {
		proc->exception = proc->checkForException(SaxonProcessor::sxn_environ->env, cppClass, cppXQ);
	}
        return proc->exception;*/
	return proc->checkException();
}



SaxonApiException* XQueryProcessor::getException() {
    return exception;
}

void XQueryProcessor::createException(const char * message) {
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
