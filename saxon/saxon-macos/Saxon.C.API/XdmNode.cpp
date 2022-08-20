

#include "XdmNode.h"

#ifdef MEM_DEBUG
#define new new(__FILE__, __LINE__)
#endif

XdmNode::XdmNode(jobject obj): XdmItem(obj), baseURI(nullptr), nodeName(nullptr),localName(nullptr), childCount(-1), children(nullptr),parent(nullptr), typedValue(nullptr), attrValues(nullptr), attrCount(-1), nodeKind(UNKNOWN){

}

XdmNode::XdmNode(XdmNode * p, jobject obj, XDM_NODE_KIND kind): XdmItem(obj), baseURI(nullptr), nodeName(nullptr), localName(nullptr),  childCount(-1), children(nullptr), parent(p), typedValue(nullptr), attrValues(nullptr),  attrCount(-1), nodeKind(kind){}

bool XdmNode::isAtomic() {
	return false;
} 
    
    XDM_NODE_KIND XdmNode::getNodeKind(){
	if(nodeKind == UNKNOWN) {
            jclass xdmNodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "Lnet/sf/saxon/s9api/XdmNode;");
            static jmethodID nodeKindMID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmNodeClass,
                                                                                                     "getNodeKind",
                                                                                                     "()Lnet/sf/saxon/s9api/XdmNodeKind;");
            if (!nodeKindMID) {
                std::cerr << "Error: MyClassInDll." << "getNodeKind" << " not found\n"
                          << std::endl;
                return UNKNOWN;
            }

            jobject nodeKindObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, nodeKindMID));
            if (!nodeKindObj) {

                return UNKNOWN;
            }
            jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "Lnet/sf/saxon/option/cpp/XdmUtils;");

            jmethodID mID2 = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,
                                                                                             "convertNodeKindType",
                                                                                             "(Lnet/sf/saxon/s9api/XdmNodeKind;)I");

            if (!mID2) {
                std::cerr << "Error: MyClassInDll." << "convertNodeKindType" << " not found\n"
                          << std::endl;
                return UNKNOWN;
            }

            int kvalue = (int) (SaxonProcessor::sxn_environ->env->CallStaticIntMethod(xdmUtilsClass, mID2, nodeKindObj));

		nodeKind = static_cast<XDM_NODE_KIND>(kvalue);
	} 
	return nodeKind;

    }

    XdmNode::~XdmNode() {
       // if(getRefCount() <= 1) {
       if(baseURI != nullptr) {
        delete baseURI;
        baseURI = nullptr;
       }

       if(nodeName != nullptr) {
         delete nodeName;
         nodeName = nullptr;
       }

       if(localName != nullptr) {
         delete localName;
         localName = nullptr;
       }

           /* if (children != nullptr) {

                for (int p = 0; p < childCount; ++p) {
                    delete children[p];

                }
                delete[] children;
            }
            if (attrCount > 0 && attrValues != nullptr) {
                for (int p = 0; p < attrCount; ++p) {
                    delete attrValues[p];

                }
                delete[] attrValues;
            }     */

        //}
    }

                                            
    const char * XdmNode::getLocalName(){
	if(localName != nullptr) {
		return localName;
	}
	XDM_NODE_KIND kind = getNodeKind();
 	jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
	jmethodID xlnmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getLocalName",
					"(Lnet/sf/saxon/s9api/XdmNode;)Ljava/lang/String;");
	switch (kind) {
            case DOCUMENT:
            case TEXT:
            case COMMENT:
                return nullptr;
            case PROCESSING_INSTRUCTION:
            case NAMESPACE:
            case ELEMENT:
            case ATTRIBUTE:

		if (!xlnmID) {
			std::cerr << "Error: MyClassInDll." << "getLocalName"<< " not found\n" << std::endl;
			return nullptr;
		} else {
			jstring result = (jstring)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xlnmID, value));
			if(!result) {
				return nullptr;
			} else {
			jboolean isCopy = JNI_TRUE;
				localName = SaxonProcessor::sxn_environ->env->GetStringUTFChars(result,  &isCopy);
                SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
                return localName;
			}
		}
            default:
                return nullptr;
        }

        

    }

    const char * XdmNode::getNodeName(){
	
	if(nodeName != nullptr) {
		return nodeName;
	}
	XDM_NODE_KIND kind = getNodeKind();
 	jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
	jmethodID xnnmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getNodeName",
					"(Lnet/sf/saxon/s9api/XdmNode;)Ljava/lang/String;");
	switch (kind) {
            case DOCUMENT:
            case TEXT:
            case COMMENT:
                return nullptr;
            case PROCESSING_INSTRUCTION:
            case NAMESPACE:
            case ELEMENT:
            case ATTRIBUTE:
               
		if (!xnnmID) {
			std::cerr << "Error: MyClassInDll." << "getNodeName"<< " not found\n" << std::endl;
			return nullptr;
		} else {
			jstring result = (jstring)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xnnmID, value));
			if(!result) {
				return nullptr;
			} else {
			jboolean isCopy = JNI_TRUE;
				nodeName = SaxonProcessor::sxn_environ->env->GetStringUTFChars(result,  &isCopy);
				SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);
				return nodeName;
			} 
		}
            default:
                return nullptr;
        }
	

    }

    XdmValue * XdmNode::getTypedValue(){
    	if(typedValue == nullptr) {
    		jclass xdmNodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
    		jmethodID tbmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmNodeClass,
    					"getTypedValue",
    					"()Lnet/sf/saxon/s9api/XdmValue;");
    		if (!tbmID) {
    			std::cerr << "Error: Saxonc." << "getTypedValue"
    				<< " not found\n" << std::endl;
    			return nullptr;
    		} else {
    			jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, tbmID));
    			if(valueObj) {
    				typedValue = new XdmValue();
    				typedValue->addUnderlyingValue(valueObj);
    				return typedValue;
    			}
    			return nullptr;
    		}
    	} else {
    		return typedValue;
    	}


    }


XdmItem * XdmNode::getHead(){
return this;}

    const char * XdmNode::getStringValue(){
   		return XdmItem::getStringValue();
    }

    const char * XdmNode::toString(){
        		jclass xdmNodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
        		jmethodID strbMID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmNodeClass,
        					"toString",
        					"()Ljava/lang/String;");
        		if (!strbMID) {
        			std::cerr << "Error: Saxonc." << "toString"
        				<< " not found\n" << std::endl;
        			return nullptr;
        		} else {
        			jstring result = (jstring) (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, strbMID));
        			if(result) {
                       const char * str = SaxonProcessor::sxn_environ->env->GetStringUTFChars(result, nullptr);
                       stringValue = str;
                  		return str;
                }
                   return nullptr;
        		}
        	
    }

    
    const char* XdmNode::getBaseUri(){

	if(baseURI != nullptr) {
		return baseURI;
	}

	jclass xdmNodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
	jmethodID bmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmNodeClass,
					"getBaseURI",
					"()Ljava/net/URI;");
	if (!bmID) {
		std::cerr << "Error: MyClassInDll." << "getBaseURI"
				<< " not found\n" << std::endl;
		return nullptr;
	} else {
		jobject nodeURIObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, bmID));
		if(!nodeURIObj){
			return nullptr;
		} else {
			jclass URIClass = lookForClass(SaxonProcessor::sxn_environ->env, "java/net/URI");
			jmethodID strMID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(URIClass,
					"toString",
					"()Ljava/lang/String;");
			if(strMID){
				jstring result = (jstring)(
				SaxonProcessor::sxn_environ->env->CallObjectMethod(nodeURIObj, strMID));
				baseURI = SaxonProcessor::sxn_environ->env->GetStringUTFChars(result,
					nullptr);
			
				return baseURI;
			}	
		}
	}
	return nullptr;
    }
    
    




    XdmNode* XdmNode::getParent(){
	if(parent == nullptr) {
		jclass xdmNodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
		jmethodID bmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmNodeClass,
					"getParent",
					"()Lnet/sf/saxon/s9api/XdmNode;");
		if (!bmID) {
			std::cerr << "Error: MyClassInDll." << "getParent"
				<< " not found\n" << std::endl;
			return nullptr;
		} else {
			jobject nodeObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, bmID));
			if(nodeObj) {
				parent = new XdmNode(nullptr, nodeObj, UNKNOWN);
				//parent->incrementRefCount();
				return parent;
			}
			return nullptr;
		}
	} else {
		return parent;
	}
	
    }
    
    const char* XdmNode::getAttributeValue(const char *str){

	if(str == nullptr) { return nullptr;}
	jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
	jmethodID xmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getAttributeValue",
					"(Lnet/sf/saxon/s9api/XdmNode;Ljava/lang/String;)Ljava/lang/String;");
	if (!xmID) {
			std::cerr << "Error: SaxonDll." << "getAttributeValue"
				<< " not found\n" << std::endl;
			return nullptr;
		}
	if(str == nullptr) {
		return nullptr;
	}
	jstring eqname = SaxonProcessor::sxn_environ->env->NewStringUTF(str);

	jstring result = (jstring)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xmID,value, eqname));
	SaxonProcessor::sxn_environ->env->DeleteLocalRef(eqname);
	//failure = checkForException(sxn_environ,  (jobject)result);//Remove code
	if(result) {
		const char * stri = SaxonProcessor::sxn_environ->env->GetStringUTFChars(result,
					nullptr);
		
		//SaxonProcessor::sxn_environ->env->DeleteLocalRef(result);

		return stri;
	} else {

		return nullptr;
	}

    }

    XdmNode** XdmNode::getAttributeNodes(bool cached){
    if (cached && attrValues != nullptr) {
        return attrValues;
    }  else {

		jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
		jmethodID xmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getAttributeNodes",
					"(Lnet/sf/saxon/s9api/XdmNode;)[Lnet/sf/saxon/s9api/XdmNode;");
		jobjectArray results = (jobjectArray)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xmID, 
		value));
		if(results == nullptr) {
			return nullptr;
		}
		XdmNode ** attrValuesi = nullptr;
		int sizex = SaxonProcessor::sxn_environ->env->GetArrayLength(results);
		attrCount = sizex;
		if(sizex>0) {	
			attrValuesi =  new XdmNode*[sizex];
			XdmNode * tempNode =nullptr;
			for (int p=0; p < sizex; ++p){
				jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);
				tempNode = new XdmNode(this, resulti, ATTRIBUTE);
				this->incrementRefCount();
				attrValuesi[p] = tempNode;
			}
		}
		if(cached) {
		    attrValues = attrValuesi;
		}
		return attrValuesi;
	} 

    }


    int XdmNode::getAttributeCount(){
	if(attrCount == -1) {
		jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
		jmethodID xmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getAttributeCount",
					"(Lnet/sf/saxon/s9api/XdmNode;)I");
		
		if (!xmID) {
			std::cerr << "Error: SaxonDll." << "getAttributeCount"
				<< " not found\n" << std::endl;
			return 0;
		}
		jint result = (jlong)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xmID,
		value));

		attrCount =(int)result;
	}
	return attrCount;
    }

    int XdmNode::getChildCount(){
	if(childCount == -1) {
		jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
		jmethodID xmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getChildCount",
					"(Lnet/sf/saxon/s9api/XdmNode;)I");
		
		if (!xmID) {
			std::cerr << "Error: SaxonDll." << "getchildCount"
				<< " not found\n" << std::endl;
			return 0;
		}
		jint result = (jlong)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xmID,
		value));

		childCount =(int)result;
	}
	return childCount;
    }
    
    XdmNode** XdmNode::getChildren(bool cached){

    if (cached && children != nullptr) {
        return children;
    }  else {

    XdmNode ** childreni = nullptr;

	//if(children == nullptr) {
		jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
		jmethodID xmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getChildren",
					"(Lnet/sf/saxon/s9api/XdmNode;)[Lnet/sf/saxon/s9api/XdmNode;");
		
		if (!xmID) {
			std::cerr << "Error: SaxonDll." << "getchildren"
				<< " not found\n" << std::endl;
			return nullptr;
		}
		jobjectArray results = (jobjectArray)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xmID, 
		value));
		int sizex = SaxonProcessor::sxn_environ->env->GetArrayLength(results);
		childCount = sizex;	
		childreni =  new XdmNode*[sizex];
		XdmNode * tempNode = nullptr;
		for (int p=0; p < sizex; ++p){
			jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);
			tempNode = new XdmNode(this, resulti, UNKNOWN);
			//tempNode->incrementRefCount();
			childreni[p] = tempNode;
		}
		SaxonProcessor::sxn_environ->env->DeleteLocalRef(results);
	//}
    if (cached) {
        children = childreni;
    }
	return childreni;

    }
    }   

    XdmNode* XdmNode::getChild(int i, bool cached) {

       if (cached ) {

            if(children == nullptr) {
                getChildren(cached);


            }

            if(i < childCount && children[i] != nullptr) {
                return children[i];


                } else {
                    return nullptr;

                }
       }  else {
        XdmNode * child = nullptr;

		jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
		jmethodID xmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getChild",
					"(Lnet/sf/saxon/s9api/XdmNode;I)Lnet/sf/saxon/s9api/XdmNode;");

		if (!xmID) {
			std::cerr << "Error: SaxonDll." << "getchild"
				<< " not found\n" << std::endl;
			return nullptr;
		}
		jobject results = SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xmID,
		value,i);

	    child = new XdmNode(this, results, UNKNOWN);


	    return child;

        }
    }
  
