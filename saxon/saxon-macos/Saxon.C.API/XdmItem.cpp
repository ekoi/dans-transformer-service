

#include "XdmItem.h"

XdmItem::XdmItem(): XdmValue(){
	value = nullptr;
}

    XdmItem::XdmItem(const XdmItem &other): XdmValue(other){
        value = other.value;
	    xdmSize =1;
	    refCount = other.refCount;
	    stringValue = other.stringValue;
    }


XdmItem::XdmItem(jobject obj){
	value = SaxonProcessor::sxn_environ->env->NewGlobalRef(obj);
	xdmSize =1;
	refCount =0;
}

bool XdmItem::isAtomic(){
	return false;
}



bool XdmItem::isNode(){
    return false;

}

bool XdmItem::isFunction(){
   return false;

}

bool XdmItem::isMap(){
   return false;

}

bool XdmItem::isArray(){
   return false;

}


XdmItem * XdmItem::getHead(){
return this;}

  XdmItem * XdmItem::itemAt(int n){
	if (n < 0 || n >= size()) {
		return nullptr;	
	}
	return this;
  }



 int XdmItem::size(){
	return 1;	
   }

jobject XdmItem::getUnderlyingValue(){
#ifdef DEBUG
	std::cerr<<std::endl<<"XdmItem-getUnderlyingValue:"<<std::endl; 
#endif
	if(!value) {
#ifdef DEBUG
	     std::cerr<<std::endl<<"XdmItem-getUnderlyingValue - nullptr:"<<std::endl;
#endif
		return nullptr;	
	}
	return value;
}

    const char * XdmItem:: getStringValue(){
        if(stringValue.empty()) {
    		jclass xdmItemClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmItem");
    		jmethodID sbmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmItemClass,
    					"getStringValue",
    					"()Ljava/lang/String;");
    		if (!sbmID) {
    			std::cerr << "Error: Saxonc." << "getStringValue"
    				<< " not found\n" << std::endl;
    			return nullptr;
    		} else {
    			jstring result = (jstring)(SaxonProcessor::sxn_environ->env->CallObjectMethod(value, sbmID));
    			if(result) {
    					const char * str = SaxonProcessor::sxn_environ->env->GetStringUTFChars(result, nullptr);

    					stringValue = std::string(str);

    					if(stringValue.length() == 0  || stringValue.empty()) {
                            return nullptr;
    					}
                        return stringValue.c_str();
    			}
    			return nullptr;
    		}
    	} else {
    		return stringValue.c_str();
    	}
   }


const char * XdmItem::toString(){
    jclass xdmClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmItem");
    jmethodID strbbMID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmClass,
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

	/**
	* Get the type of the object
	*/
	XDM_TYPE XdmItem::getType(){
		return XDM_ITEM;
	}

XdmItem::~XdmItem() {
   // std::cerr<< " ref="<<getRefCount()<<std::endl;

    if(value !=nullptr /* && getRefCount() <=1*/) {
        SaxonProcessor::sxn_environ->env->DeleteGlobalRef(value);
    } /*else {
        decrementRefCount();
    }*/
    if(stringValue.empty()) {
        stringValue.clear();
    }
}

void XdmItem::incrementRefCount() {
    refCount++;
   // std::cerr<<"after refCount-inc-xdmItem="<<refCount<<" ob ref="<<(this)<<std::endl;
}

void XdmItem::decrementRefCount() {
    if (refCount > 0)
        refCount--;
   // std::cerr<<"after refCount-dec-xdmItem="<<refCount<<" ob ref="<<(this)<<std::endl;
}
