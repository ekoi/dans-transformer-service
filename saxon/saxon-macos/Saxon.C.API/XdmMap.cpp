#include "XdmMap.h"
#include "XdmAtomicValue.h"
#include <map>

#ifdef MEM_DEBUG
#define new new(__FILE__, __LINE__)
#endif


XdmMap::XdmMap(): XdmFunctionItem() {
    map_size = 1;
}

XdmMap::XdmMap(const XdmMap &d) {
      arity = d.arity;
      data = d.data; //TODO check if copy works
      map_size = d.map_size;
    }

XdmMap::XdmMap(jobject obj): XdmFunctionItem(obj) {
     map_size = -1;

}

const char * XdmMap::toString(){
    jclass xdmMapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
    jmethodID strbbMID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmMapClass,
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



    int XdmMap::mapSize(){
       if(map_size == -1) {
        jclass xdmMapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
        static jmethodID tbiimapID = nullptr;

        if(tbiimapID == nullptr) {
        tbiimapID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmMapClass, "mapSize",
        					"()I");

        }
        if (!tbiimapID) {
        			std::cerr << "Error: Saxonc.XdmMap." << "mapSize"
        				<< " not found\n" << std::endl;
        			return 0;
        } else {
        		jint result = (jint)(SaxonProcessor::sxn_environ->env->CallIntMethod(value, tbiimapID));
        		map_size =(int)result;
        }
      }

        return map_size;
    }

    XdmValue * XdmMap::get(XdmAtomicValue* key) {
    jclass xdmMapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
    static jmethodID tbmapID = nullptr;

    if(tbmapID == nullptr) {
    tbmapID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmMapClass, "get",
    					"(Lnet/sf/saxon/s9api/XdmAtomicValue;)Lnet/sf/saxon/s9api/XdmValue;");

    }
    if (tbmapID == nullptr) {
    			std::cerr << "Error: Saxonc.XdmMap." << "get(XdmAtomicValue)"
    				<< " not found\n" << std::endl;
    			return nullptr;
    } else {
    			jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, tbmapID, ((XdmItem *)key)->getUnderlyingValue()));
        		if(valueObj) {
                    jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                           "net/sf/saxon/s9api/XdmAtomicValue");
                    jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
                    jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                            "net/sf/saxon/s9api/XdmFunctionItem");
                    jclass mapItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                            "net/sf/saxon/s9api/XdmMap");
                    jclass arrayItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                       "net/sf/saxon/s9api/XdmArray");

                    XdmValue * tempValue = new XdmValue();
                    
                    XdmItem *xdmItem = nullptr;


                      if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, atomicValueClass) == JNI_TRUE) {
                            xdmItem = new XdmAtomicValue(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, nodeClass) == JNI_TRUE) {
                            xdmItem = new XdmNode(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, functionItemClass) == JNI_TRUE) {
                            xdmItem = new XdmFunctionItem(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, mapItemClass) == JNI_TRUE) {
                            xdmItem = new XdmMap(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, arrayItemClass) == JNI_TRUE) {
                            xdmItem = new XdmArray(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else {
                            tempValue = new XdmValue();
                            tempValue->addUnderlyingValue(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            return tempValue;
                        }
       			}
       			return nullptr;
    }

    }

    XdmValue * XdmMap::get(const char * key) {
        jclass xdmMapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
        static jmethodID tbsmapID = nullptr;

        if(tbsmapID == nullptr) {
        tbsmapID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmMapClass, "get",
        					"(Ljava/lang/String;)Lnet/sf/saxon/s9api/XdmValue;");

        }
        if (!tbsmapID) {
        			std::cerr << "Error: Saxonc.XdmMap." << "get(String)"
        				<< " not found\n" << std::endl;
        			return nullptr;
        } else {
                jobject strObj = getJavaStringValue(SaxonProcessor::sxn_environ, key);
        		jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, tbsmapID, strObj));
        		if(valueObj) {
                    jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                           "net/sf/saxon/s9api/XdmAtomicValue");
                    jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
                    jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                            "net/sf/saxon/s9api/XdmFunctionItem");
                    jclass mapItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                            "net/sf/saxon/s9api/XdmMap");
                    jclass arrayItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                       "net/sf/saxon/s9api/XdmArray");

                    XdmValue * tempValue = new XdmValue();
                    XdmItem *xdmItem = nullptr;


                      if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, atomicValueClass) == JNI_TRUE) {
                            xdmItem = new XdmAtomicValue(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, nodeClass) == JNI_TRUE) {
                            xdmItem = new XdmNode(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, functionItemClass) == JNI_TRUE) {
                            xdmItem = new XdmFunctionItem(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, mapItemClass) == JNI_TRUE) {
                            xdmItem = new XdmMap(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, arrayItemClass) == JNI_TRUE) {
                            xdmItem = new XdmArray(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else {
                            tempValue = new XdmValue();
                            tempValue->addUnderlyingValue(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            return tempValue;
                        }
       			}
       			return nullptr;
        }


    }

    XdmValue * XdmMap::get(int key) {
        jclass xdmMapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
        static jmethodID tbimapID = nullptr;

        if(tbimapID == nullptr) {
        tbimapID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmMapClass, "get",
        					"(J)Lnet/sf/saxon/s9api/XdmValue;");

        }
        if (tbimapID == nullptr) {
        			std::cerr << "Error: Saxonc.XdmMap." << "get(int)"
        				<< " not found\n" << std::endl;
        			return nullptr;
        } else {
        		jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, tbimapID, (long)key));
        		if(valueObj) {
                    jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                           "net/sf/saxon/s9api/XdmAtomicValue");
                    jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
                    jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                            "net/sf/saxon/s9api/XdmFunctionItem");
                    jclass mapItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                            "net/sf/saxon/s9api/XdmMap");
                    jclass arrayItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                       "net/sf/saxon/s9api/XdmArray");

                    XdmValue * tempValue = new XdmValue();
                    XdmItem *xdmItem = nullptr;


                      if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, atomicValueClass) == JNI_TRUE) {
                            xdmItem = new XdmAtomicValue(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, nodeClass) == JNI_TRUE) {
                            xdmItem = new XdmNode(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, functionItemClass) == JNI_TRUE) {
                            xdmItem = new XdmFunctionItem(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, mapItemClass) == JNI_TRUE) {
                            xdmItem = new XdmMap(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, arrayItemClass) == JNI_TRUE) {
                            xdmItem = new XdmArray(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else {
                            tempValue = new XdmValue();
                            tempValue->addUnderlyingValue(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            return tempValue;
                        }
       			}
       			return nullptr;
        }


    }

    XdmValue *  XdmMap::get(double key) {
        jclass xdmMapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
        static jmethodID tbdmapID = nullptr;

        if(tbdmapID == nullptr) {
            tbdmapID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmMapClass, "get",
        					"(D)Lnet/sf/saxon/s9api/XdmValue;");

        }
        if (!tbdmapID) {
        			std::cerr << "Error: Saxonc.XdmMap." << "get(double)"
        				<< " not found\n" << std::endl;
        			return nullptr;
        } else {
        		jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, tbdmapID, key));
        		if(valueObj) {
                    jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                           "net/sf/saxon/s9api/XdmAtomicValue");
                    jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
                    jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                            "net/sf/saxon/s9api/XdmFunctionItem");
                    jclass mapItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                            "net/sf/saxon/s9api/XdmMap");
                    jclass arrayItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                       "net/sf/saxon/s9api/XdmArray");

                    XdmValue * tempValue = new XdmValue();
                    XdmItem *xdmItem = nullptr;


                      if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, atomicValueClass) == JNI_TRUE) {
                            xdmItem = new XdmAtomicValue(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, nodeClass) == JNI_TRUE) {
                            xdmItem = new XdmNode(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, functionItemClass) == JNI_TRUE) {
                            xdmItem = new XdmFunctionItem(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, mapItemClass) == JNI_TRUE) {
                            xdmItem = new XdmMap(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, arrayItemClass) == JNI_TRUE) {
                            xdmItem = new XdmArray(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else {
                            tempValue = new XdmValue();
                            tempValue->addUnderlyingValue(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            return tempValue;
                        }
       			}
       			return nullptr;
        }


    }

    XdmValue * XdmMap::get(long key) {
        jclass xdmMapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
        static jmethodID tbdmapID = nullptr;

        if(tbdmapID == nullptr) {
            tbdmapID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmMapClass, "get",
        					"(J)Lnet/sf/saxon/s9api/XdmValue;");

        }
        if (!tbdmapID) {
        			std::cerr << "Error: Saxonc.XdmMap." << "get(long)"
        				<< " not found\n" << std::endl;
        			return nullptr;
        } else {
        		jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, tbdmapID, key));
        		if(valueObj) {
                    jclass atomicValueClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                           "net/sf/saxon/s9api/XdmAtomicValue");
                    jclass nodeClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmNode");
                    jclass functionItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                            "net/sf/saxon/s9api/XdmFunctionItem");
                    jclass mapItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                            "net/sf/saxon/s9api/XdmMap");
                    jclass arrayItemClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                                       "net/sf/saxon/s9api/XdmArray");

                    XdmValue * tempValue = new XdmValue();
                    XdmItem *xdmItem = nullptr;


                      if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, atomicValueClass) == JNI_TRUE) {
                            xdmItem = new XdmAtomicValue(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, nodeClass) == JNI_TRUE) {
                            xdmItem = new XdmNode(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, functionItemClass) == JNI_TRUE) {
                            xdmItem = new XdmFunctionItem(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, mapItemClass) == JNI_TRUE) {
                            xdmItem = new XdmMap(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(valueObj, arrayItemClass) == JNI_TRUE) {
                            xdmItem = new XdmArray(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            tempValue->addXdmItem(xdmItem);
                            return tempValue;
                        } else {
                            tempValue = new XdmValue();
                            tempValue->addUnderlyingValue(valueObj);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(valueObj);
                            return tempValue;
                        }
       			}
       			return nullptr;
        }

    }

//TODO test this method
    XdmMap * XdmMap::put(XdmAtomicValue* key, XdmValue * valuei) {
        if(key == nullptr || valuei == nullptr) {
            std::cerr << "Error: Saxonc.XdmMap." << "key or value is nullptr" << std::endl;
            return nullptr;
        }
        jclass xdmMapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
        static jmethodID putmapID = nullptr;

        if(putmapID == nullptr) {
                    putmapID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmMapClass, "put",
                					"(Lnet/sf/saxon/s9api/XdmAtomicValue;Lnet/sf/saxon/s9api/XdmValue;)Lnet/sf/saxon/s9api/XdmMap;");

        }
        if (!putmapID) {
            std::cerr << "Error: Saxonc.XdmMap." << "put"
                	  << " not found\n" << std::endl;
            return nullptr;
        } else {
                		jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, putmapID, ((XdmItem *)key)->getUnderlyingValue(), valuei->getUnderlyingValue()));
                		if(valueObj) {
                			XdmMap * newValue = new XdmMap(valueObj);
               				return newValue;
               			}
               			return nullptr;
                }

    }

    XdmMap * XdmMap::remove(XdmAtomicValue* key){
                jclass xdmMapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
                static jmethodID removemapID = nullptr;

                if(removemapID == nullptr) {
                    removemapID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmMapClass, "remove",
                					"(Lnet/sf/saxon/s9api/XdmAtomicValue;)Lnet/sf/saxon/s9api/XdmMap;");

                }
                if (!removemapID) {
                			std::cerr << "Error: Saxonc.XdmMap." << "remove"
                				<< " not found\n" << std::endl;
                			return nullptr;
                } else {
                		jobject valueObj = (SaxonProcessor::sxn_environ->env->CallObjectMethod(value, removemapID, ((XdmItem *)key)->getUnderlyingValue()));
                		if(valueObj) {
                			XdmMap * newValue = new XdmMap(valueObj);
               				return newValue;
               			}
               			return nullptr;
                }



    }

    std::set<XdmAtomicValue*> XdmMap::keySet(){
        std::set<XdmAtomicValue *> myset;

        jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
        static jmethodID keySetmID = nullptr;

        if(keySetmID == nullptr) {
            keySetmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getXdmMapKeys",
                        					"(Lnet/sf/saxon/s9api/XdmMap;)[Lnet/sf/saxon/s9api/XdmAtomicValue;");
            }
            if (!keySetmID) {
                std::cerr << "Error: SaxonDll." << "makeXdmMap"<< " not found\n" << std::endl;
                return myset;
            }


            jobjectArray results = (jobjectArray)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, keySetmID,value));
            if(results) {
            	   int sizex = SaxonProcessor::sxn_environ->env->GetArrayLength(results);


                   	XdmAtomicValue * tempValue = nullptr;
                   		for (int p=0; p < sizex; ++p){
                   			jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);
                   			tempValue = new XdmAtomicValue();
                   			tempValue->addUnderlyingValue(resulti);
                   			myset.insert(tempValue);
                   		}
        }



         return myset;
    }



        XdmAtomicValue ** XdmMap::keys(){
            XdmAtomicValue ** keysArr = nullptr;

            jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
            static jmethodID keySetmID = nullptr;

            if(keySetmID == nullptr) {
                keySetmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getXdmMapKeys",
                            					"(Lnet/sf/saxon/s9api/XdmMap;)[Lnet/sf/saxon/s9api/XdmAtomicValue;");
                }
                if (!keySetmID) {
                    std::cerr << "Error: SaxonDll." << "makeXdmMap"<< " not found\n" << std::endl;
                    return nullptr;
                }


                jobjectArray results = (jobjectArray)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, keySetmID,value));
                if(results) {
                	   int sizex = SaxonProcessor::sxn_environ->env->GetArrayLength(results);


                       	XdmAtomicValue * tempValue = nullptr;
                       	keysArr = new XdmAtomicValue*[sizex];
                       	for (int p=0; p < sizex; ++p){
                       			jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);
                       			tempValue = new XdmAtomicValue(resulti);
                       			keysArr[p] =tempValue;
                       	}
            }



             return keysArr;
        }

    //std::map<XdmAtomicValue*, XdmValue*> XdmMap::asMap();

    bool XdmMap::isEmpty() {
                jclass xdmMapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
                static jmethodID isEmptyMapID = nullptr;

                if(isEmptyMapID == nullptr) {
                    isEmptyMapID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmMapClass, "isEmpty",
                					"()Z");

                }
                if (!isEmptyMapID) {
                			std::cerr << "Error: Saxonc.XdmMap." << "isEmpty"
                				<< " not found\n" << std::endl;
                			return false;
                } else {
                		jboolean result = (SaxonProcessor::sxn_environ->env->CallBooleanMethod(value, isEmptyMapID));
                		return result;
                }
    }

    bool XdmMap::containsKey(XdmAtomicValue * key) {
                jclass xdmMapClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/XdmMap");
                static jmethodID containsMapID = nullptr;

                if(containsMapID == nullptr) {
                    containsMapID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(xdmMapClass, "containsKey",
                					"(Lnet/sf/saxon/s9api/XdmAtomicValue;)Z");

                }
                if (!containsMapID) {
                			std::cerr << "Error: Saxonc.XdmMap." << "containsKey"
                				<< " not found\n" << std::endl;
                			return false;
                } else {
                		jboolean result = (SaxonProcessor::sxn_environ->env->CallBooleanMethod(value, containsMapID, ((XdmItem *)key)->getUnderlyingValue()));
                		return result;
                }



    }

    std::list<XdmValue *> XdmMap::valuesAsList() {
         std::list<XdmValue *> mylist;

            jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
            jmethodID xmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getXdmMapValues",
                        					"(Lnet/sf/saxon/s9api/XdmMap;)[Lnet/sf/saxon/s9api/XdmValue;");
            if (!xmID) {
                std::cerr << "Error: SaxonDll." << "makeXdmMap"<< " not found\n" << std::endl;
                return mylist;
            }


            jobjectArray results = (jobjectArray)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xmID,value));
            if(results) {
            	   int sizex = SaxonProcessor::sxn_environ->env->GetArrayLength(results);


                   	XdmValue * tempValue = nullptr;
                   	for (int p=0; p < sizex; ++p){
                   			jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);
                   			tempValue = new XdmValue();
                   			tempValue->addUnderlyingValue(resulti);
                   			mylist.push_back(tempValue);

                   		}
                    SaxonProcessor::sxn_environ->env->DeleteLocalRef(results);
            }


         return mylist;
    }




        XdmValue ** XdmMap::values() {
        //TODO: We need a strategy of the memory management of the values returned
             XdmValue ** valuesArr = nullptr;

                jclass xdmUtilsClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/XdmUtils");
                jmethodID xmID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(xdmUtilsClass,"getXdmMapValues",
                            					"(Lnet/sf/saxon/s9api/XdmMap;)[Lnet/sf/saxon/s9api/XdmValue;");
                if (!xmID) {
                    std::cerr << "Error: SaxonDll." << "values()"<< " not found\n" << std::endl;
                    return valuesArr;
                }


                jobjectArray results = (jobjectArray)(SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(xdmUtilsClass, xmID,value));
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
                        valuesArr = new XdmValue*[sizex];

                       	XdmValue * tempValue = nullptr;
                       	XdmItem *xdmItem = nullptr;
                       	for (int p=0; p < sizex; ++p){
                       			jobject resulti = SaxonProcessor::sxn_environ->env->GetObjectArrayElement(results, p);
                       		tempValue = new XdmValue();

                            if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, atomicValueClass) == JNI_TRUE) {
                            xdmItem = new XdmAtomicValue(resulti);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                            tempValue->addXdmItem(xdmItem);
                            valuesArr[p] = tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, nodeClass) == JNI_TRUE) {
                            xdmItem = new XdmNode(resulti);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                            tempValue->addXdmItem(xdmItem);
                            valuesArr[p] = tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, functionItemClass) == JNI_TRUE) {
                            xdmItem = new XdmFunctionItem(resulti);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                            tempValue->addXdmItem(xdmItem);
                            valuesArr[p] = tempValue;

                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, mapItemClass) == JNI_TRUE) {
                            xdmItem = new XdmMap(resulti);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                            tempValue->addXdmItem(xdmItem);
                            valuesArr[p] = tempValue;
                        } else if (SaxonProcessor::sxn_environ->env->IsInstanceOf(resulti, arrayItemClass) == JNI_TRUE) {
                            xdmItem = new XdmArray(resulti);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                            tempValue->addXdmItem(xdmItem);
                            valuesArr[p] = tempValue;
                        } else {
                            tempValue = new XdmValue(resulti, true);
                            SaxonProcessor::sxn_environ->env->DeleteLocalRef(resulti);
                            valuesArr[p] = tempValue;
                        }



                    }
                }


             return valuesArr;
        }