#include "XdmArray.h"

#include "XdmMap.h"

XdmArray::XdmArray(): XdmFunctionItem(), arrayLen(0) {


}

XdmArray::XdmArray(const XdmArray &d): XdmFunctionItem(d){
           arrayLen = d.arrayLen;
}



XdmArray::XdmArray(jobject obj) : XdmFunctionItem(obj), arrayLen(0){}


XdmArray::XdmArray(jobject obj, int aLen) : XdmFunctionItem(obj), arrayLen(aLen){}

int XdmArray::arrayLength()  {

    if(arrayLen == 0) {
    jclass xdmArrayClass = lookForClass(SaxonProcessor::sxn_environ->env, "Lnet/sf/saxon/s9api/XdmArray;");
    static jmethodID arLenmID = nullptr;

    if(arLenmID == nullptr) {

        arLenmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmArrayClass, "arrayLength",
                                                                          "()I");
    }

    if (arLenmID == nullptr || !value) {
        std::cerr << "Error: Saxonc.XdmArray." << "arrayLength"
                  << " not found\n" << std::endl;
        arrayLen = 0;
        return 0;
    } else {

        jint valuei = (jint)(SaxonProcessor::sxn_environ->env->CallIntMethod(value, arLenmID));
        arrayLen = (int)valuei;
        return arrayLen;
    }
    } else {
        return arrayLen;
    }
}

XdmValue* XdmArray::get(int n){
    jclass xdmArrayClass = lookForClass(SaxonProcessor::sxn_environ->env, "Lnet/sf/saxon/s9api/XdmArray;");
    static jmethodID tbmID = nullptr;

    if(n<0) {
        return nullptr;

    }

    if(tbmID == nullptr) {

        tbmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmArrayClass, "get",
    					"(I)Lnet/sf/saxon/s9api/XdmValue;");
    }

    if (!tbmID) {
    			std::cerr << "Error: Saxonc.XdmArray." << "get"
    				<< " not found\n" << std::endl;
    			return nullptr;
    } else {
    			jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, tbmID, n));
    			if(valueObj) {
    				auto * newValue = new XdmValue();
    				newValue->addUnderlyingValue(valueObj);
    				return newValue;
    			}
    			return nullptr;
    }
}


const char * XdmArray::toString(){
    jclass xdmArrayClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmArray");
    jmethodID strbbMID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmArrayClass,
                                                                                   "toString",
                                                                                   "()Ljava/lang/String;");
    if (!strbbMID) {
        std::cerr << "Error: Saxonc." << "toString"
                  << " not found\n" << std::endl;
        return nullptr;
    } else {
        jstring result = (jstring) (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, strbbMID));
        if(result) {
            const char * str = SaxonProcessor::sxn_environ->env->GetStringUTFChars(result, nullptr);
            stringValue = str;
            return str;
        }
        return nullptr;
    }

}

XdmArray* XdmArray::put(int n, XdmValue * valuei) {
    jclass xdmArrayClass = lookForClass(SaxonProcessor::sxn_environ->env, "Lnet/sf/saxon/s9api/XdmArray;");
    static jmethodID tbpmID = nullptr;

    if(n<0) {
        return nullptr;

    }

    if(tbpmID == nullptr) {
        tbpmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmArrayClass, "put",
    					"(I;Lnet/sf/saxon/s9api/XdmValue;)Lnet/sf/saxon/s9api/XdmArray;");
    }
    if (!tbpmID) {
    			std::cerr << "Error: Saxonc.XdmArray." << "put"
    				<< " not found\n" << std::endl;
    			return nullptr;
    } else {
    			jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, tbpmID, n, valuei->getUnderlyingValue()));
    			if(valueObj) {
    				auto * newValue = new XdmArray(valueObj);
    				return newValue;
    			}
    			return nullptr;
    }


}

XdmArray* XdmArray::addMember(XdmValue* valuei) {
    jclass xdmArrayClass = lookForClass(SaxonProcessor::sxn_environ->env, "Lnet/sf/saxon/s9api/XdmArray;");
    static jmethodID tbamID = nullptr;

    if(tbamID) {
        tbamID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmArrayClass, "concat",
    					"(Lnet/sf/saxon/s9api/XdmValue;)Lnet/sf/saxon/s9api/XdmArray;");

    }

    if (!tbamID) {
    			std::cerr << "Error: Saxonc.XdmArray." << "concat"
    				<< " not found\n" << std::endl;
    			return nullptr;
    } else {
    			jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, tbamID, valuei->getUnderlyingValue()));
    			if(valueObj) {
    				auto * newValue = new XdmArray(valueObj);
    				return newValue;
    			}
    			return nullptr;
    }
}

XdmArray* XdmArray::concat(XdmArray* valuei) {
    jclass xdmArrayClass = lookForClass(SaxonProcessor::sxn_environ->env, "Lnet/sf/saxon/s9api/XdmArray;");
    static jmethodID tbcaID = nullptr;


    if(tbcaID == nullptr) {
        tbcaID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmArrayClass, "concat",
    					"(Lnet/sf/saxon/s9api/XdmArray;)Lnet/sf/saxon/s9api/XdmArray;");
    }

    if (!tbcaID) {
    			std::cerr << "Error: Saxonc.XdmArray." << "concat"
    				<< " not found\n" << std::endl;
    			return nullptr;
    } else {
    			jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, tbcaID, valuei->getUnderlyingValue()));
    			if(valueObj) {
    				auto * newValue = new XdmArray(valueObj);
    				return newValue;
    			}
    			std::cerr << "Error: Saxonc.XdmArray." << "concat" << " failed\n" << std::endl;
    			return nullptr;
    }

}

std::list<XdmValue *> XdmArray::asList() {
  std::list<XdmValue *> arr;

	jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "Lnet/sf/saxon/option/cpp/XdmUtils;");
	static jmethodID xmID = nullptr;


	if(xmID == nullptr) {
	    xmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"convertXdmArrayToArrayObject",
					"(Lnet/sf/saxon/s9api/XdmArray;)[Lnet/sf/saxon/s9api/XdmValue;");
	}

	if (!xmID) {
			std::cerr << "Error: SaxonDll." << "convertXdmArrayToArrayObject"
				<< " not found\n" << std::endl;
			return arr;
		}


	auto results = (jobjectArray)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xmID,value));
	if(results) {
	   int sizex = SaxonProcessor::sxn_environ->env->GetArrayLength(results);


       		XdmValue * tempValue;
       		for (int p=0; p < sizex; ++p){
       			jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);
       			tempValue = new XdmValue();
       			tempValue->addUnderlyingValue(resulti);
       			arr.push_back(tempValue);

       		}
       	return arr;

	} else {
	    return arr;
	}


}





XdmValue ** XdmArray::values() {
  XdmValue ** arr = nullptr;

	jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "Lnet/sf/saxon/option/cpp/XdmUtils;");
	static jmethodID xmID = nullptr;


	if(xmID == nullptr) {
	    xmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"convertXdmArrayToArrayObject",
					"(Lnet/sf/saxon/s9api/XdmArray;)[Lnet/sf/saxon/s9api/XdmValue;");
	}

	if (!xmID) {
			std::cerr << "Error: SaxonDll." << "convertXdmArrayToArrayObject"
				<< " not found\n" << std::endl;
			return nullptr;
		}


	auto results = (jobjectArray)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xmID,value));
	if(results) {
        jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                               "net/sf/saxon/s9api/XdmAtomicValue");
        jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
        jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                "net/sf/saxon/s9api/XdmFunctionItem");
        jclass mapItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                           "net/sf/saxon/s9api/XdmMap");
        jclass arrayItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                             "net/sf/saxon/s9api/XdmArray");

	   int sizex = SaxonProcessor::sxn_environ->env->GetArrayLength(results);

       if(sizex == 0) {
         return nullptr;
       }
            arr = new XdmValue*[sizex];
       		XdmValue * tempValue = nullptr;
            XdmItem *xdmItem = nullptr;
       		for (int p=0; p < sizex; ++p){
       			jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);
       			tempValue = new XdmValue();

                if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, atomicValueClass) == JNI_TRUE) {
                    xdmItem = new XdmAtomicValue(resulti);
                    SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                    tempValue->addXdmItem(xdmItem);
                    arr[p] = tempValue;

                } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, nodeClass) == JNI_TRUE) {
                    xdmItem = new XdmNode(resulti);
                    SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                    tempValue->addXdmItem(xdmItem);
                    arr[p] = tempValue;
                } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, functionItemClass) == JNI_TRUE) {
                    xdmItem = new XdmFunctionItem(resulti);
                    SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                    tempValue->addXdmItem(xdmItem);
                    arr[p] = tempValue;

                } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, mapItemClass) == JNI_TRUE) {
                    xdmItem = new XdmMap(resulti);
                    SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                    tempValue->addXdmItem(xdmItem);
                    arr[p] = tempValue;
                } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, arrayItemClass) == JNI_TRUE) {
                    xdmItem = new XdmArray(resulti);
                    SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                    tempValue->addXdmItem(xdmItem);
                    arr[p] = tempValue;
                } else {
                    tempValue = new XdmValue(resulti, true);
                    SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                    arr[p] = tempValue;
                }


       		}
       	return arr;

	} else {
	    return arr;
	}


}
