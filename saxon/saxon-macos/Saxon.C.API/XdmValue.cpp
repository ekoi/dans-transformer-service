#include "XdmValue.h"
#include "XdmItem.h"
#include "XdmAtomicValue.h"
#include "XdmNode.h"
#include "XdmFunctionItem.h"
#include "XdmMap.h"
#include "XdmArray.h"


#ifdef MEM_DEBUG
#define new new(__FILE__, __LINE__)
#endif


XdmValue::XdmValue(const XdmValue &other) {
	//SaxonProcessor *proc = other.proc; //TODO
	valueType = other.valueType;
	refCount = 0;
	xdmSize = other.xdmSize;
	jValues = other.jValues;
	toStringValue = other.toStringValue;
	values.resize(0);//TODO memory issue might occur here
	toStringValue = other.toStringValue;
	for (int i = 0; i < xdmSize; i++) {
		addXdmItem(other.values[i]);
	}
	
}

const char * XdmValue::toString() {

    if(size() == 0) {
        return nullptr;
    }
    if (toStringValue.empty()) {
        jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
        jmethodID strMID2 = (jmethodID)SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,
                                                                                     "xdmValueArrayToString",
                                                                                     "([Lnet/sf/saxon/s9api/XdmValue;)Ljava/lang/String;");
        if (!strMID2) {
            std::cerr << "Error: Saxonc.XdmValue." << "xdmValueArrayToString"
                      << " not found\n" << std::endl;
            return nullptr;
        }

        jclass objectClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmValue");
        jobjectArray objectArray = nullptr;

        objectArray = SaxonProcessor::sxn_environ->env->NewObjectArray((jint) size(),
                                                                       objectClass, 0);

        for(int i=0; i<size();i++) {
            SaxonProcessor::sxn_environ->env->SetObjectArrayElement(objectArray, i,itemAt(i)->getUnderlyingValue());

        }
        jstring result = (jstring)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, strMID2, objectArray));
        if (result) {
            toStringValue = SaxonProcessor::sxn_environ->env->GetStringUTFChars(result, nullptr);
            SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);

        }
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(objectArray);

        if(toStringValue.empty()) {
            return nullptr;
        }
        return toStringValue.c_str();
    } else {
        return toStringValue.c_str();

    }

}


int XdmValue::size() {
	return xdmSize;
}

XdmValue::XdmValue(jobject val) {
	XdmItem * value = new XdmItem(val);
	values.resize(0);//TODO memory issue might occur here. Need to delete XdmItems
	values.push_back(value);
	xdmSize++;
	jValues = nullptr;
	valueType = nullptr;
}


XdmValue::XdmValue(jobject val, bool arr){
	xdmSize = 0;
	values.resize(0);
	jValues = nullptr;
	valueType = nullptr;
	jclass xdmValueForcppClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmValueForCpp");
	jmethodID xvfMID = SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmValueForcppClass, "makeArrayFromXdmValue", "(Lnet/sf/saxon/s9api/XdmValue;)[Lnet/sf/saxon/s9api/XdmItem;");

	if(!xvfMID){

		std::cerr << "Error: SaxonDll." << "makeArrayFromXdmValue"
				<< " not found\n" << std::endl;
			return ;
	}
	
	jobjectArray results = (jobjectArray) SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmValueForcppClass, xvfMID, val);
	if(results){
	int sizex = SaxonProcessor::sxn_environ->env->GetArrayLength(results);
	if (sizex>0) {
		jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmAtomicValue");
		jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
		jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmFunctionItem");
        jclass mapItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                           "net/sf/saxon/s9api/XdmMap");
        jclass arrayItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                             "net/sf/saxon/s9api/XdmArray");

		//XdmValue * value = new XdmValue();
		//value->setProcessor(proc);
		XdmItem * xdmItem = nullptr;
		for (int p=0; p < sizex; ++p) 
		{
			jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);
			//value->addUnderlyingValue(resulti);

			if(SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, atomicValueClass)           == JNI_TRUE) {
				xdmItem = new XdmAtomicValue(SaxonProcessor::sxn_environ->env->NewGlobalRef(resulti));
				

			} else if(SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, nodeClass)           == JNI_TRUE) {
				xdmItem = new XdmNode(SaxonProcessor::sxn_environ->env->NewGlobalRef(resulti));


			} else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, functionItemClass)           == JNI_TRUE) {

				xdmItem = new XdmFunctionItem(SaxonProcessor::sxn_environ->env->NewGlobalRef(resulti));

			}  else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, mapItemClass)           == JNI_TRUE) {

                xdmItem = new XdmMap(SaxonProcessor::sxn_environ->env->NewGlobalRef(resulti));

            }  else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, arrayItemClass)           == JNI_TRUE) {

                xdmItem = new XdmArray(SaxonProcessor::sxn_environ->env->NewGlobalRef(resulti));

            }
			//xdmItem->setProcessor(proc);
			addXdmItem(xdmItem);
			SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
		}
	}
		SaxonProcessor::sxn_environ->env->DeleteLocalRef(results);
	}	
}


XdmValue::~XdmValue() {
	//std::cerr<<"XdmValue destructor pointer = "<<(this)<<"value-size="<<values.size()<<std::endl;
	//bool safeToClear = false;
	if(values.size() > 0) {
        for (size_t i = 0; i < values.size(); i++) {
            if (values[i] != nullptr && values[i]->getRefCount()<1) {
                delete values[i];
            }/* else {
                values[i]->decrementRefCount();
            }*/
        }
        values.clear();
    }
	if (valueType != nullptr) { delete valueType; }
	if (jValues) {
		SaxonProcessor::sxn_environ->env->DeleteGlobalRef(jValues);
	}
	xdmSize = 0;

    if(!toStringValue.empty()) {
        toStringValue.clear();
    }

}

void XdmValue::addXdmItem(XdmItem* val) {
	if (val != nullptr) {
		values.push_back(val);
        val->incrementRefCount();
		xdmSize++;
        if (jValues) {
            SaxonProcessor::sxn_environ->env->DeleteGlobalRef(jValues);
            jValues = nullptr;
        }

	}
}


void XdmValue::addUnderlyingValue(jobject val) {
	XdmItem * valuei = new XdmItem(val);
	valuei->incrementRefCount();
	values.push_back(valuei);
	xdmSize++;
	jValues = nullptr; //TODO clear jni array from memory if needed

}

void XdmValue::incrementRefCount() {
		refCount++;
		//std::cerr<<"refCount-inc-xdmVal="<<refCount<<" ob ref="<<(this)<<std::endl;
	}

void XdmValue::decrementRefCount() {
		if (refCount > 0)
			refCount--;
		//std::cerr<<"refCount-dec-xdmVal="<<refCount<<" ob ref="<<(this)<<std::endl;
	}




XdmItem * XdmValue::getHead() {
	if (values.size() > 0) {
		return values[0];
	}
	else {
		return nullptr;
	}
}

jobject XdmValue::getUnderlyingValue() {
	if (jValues == nullptr) {
		int i;
		JNIEnv *env = SaxonProcessor::sxn_environ->env;
		int count = values.size();
		if (count == 0) {
			return nullptr;
		}
		jclass objectClass = lookForClass(env,
			"net/sf/saxon/s9api/XdmItem");
		jobjectArray jValuesi = (jobjectArray)env->NewObjectArray((jint)count, objectClass, 0);

		for (i = 0; i < count; i++) {
			env->SetObjectArrayElement(jValuesi, i, values[i]->getUnderlyingValue());
		}

        jValues = (jobjectArray)SaxonProcessor::sxn_environ->env->NewGlobalRef(jValuesi);
	} 
	return (jobject)jValues;
}

void XdmValue::releaseXdmValue() {
    for (size_t i = 0; i < values.size(); i++) {
        if (values[i] != nullptr) {
            delete values[i];
        }
    }

    if(jValues) {
        SaxonProcessor::sxn_environ->env->DeleteGlobalRef(jValues);
    }


}

XdmItem * XdmValue::itemAt(int n) {
	if (n >= 0 && (unsigned int)n < values.size()) {
		return values[n];
	}
	return nullptr;
}

/**
* Get the type of the object
*/
XDM_TYPE XdmValue::getType() {
	return XDM_VALUE;
}




