//#include "php_XsltExecutable.h"

zend_object_handlers xsltExecutable_object_handlers;


/*     ============== XSLT30: PHP Interface of  XsltExecutable class =============== */


void XsltExecutable_free_storage(zend_object *object)
{

    zend_object_std_dtor(object);

}

void XsltExecutable_destroy_storage(zend_object *object)
{
    xsltExecutable_object *obj;

    zend_objects_destroy_object(object);
}


zend_object * xsltExecutable_create_handler(zend_class_entry *type)
{
    xsltExecutable_object *obj = (xsltExecutable_object *)ecalloc(1, sizeof(xsltExecutable_object)+ zend_object_properties_size(type));

    zend_object_std_init(&obj->std,type);
    object_properties_init(&obj->std, type);

    obj->std.handlers = &xsltExecutable_object_handlers;

    return &obj->std;
}



PHP_METHOD(XsltExecutable, __destruct)
{
    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));

    XsltExecutable * xsltExecutable= obj->xsltExecutable;
    if(xsltExecutable != nullptr){
        std::map<std::string, XdmValue *> parameters = xsltExecutable->getParameters();
        for (std::map<std::string, XdmValue *>::iterator itr = parameters.begin(); itr != parameters.end(); itr++) {
            XdmValue *value = itr->second;
            if(value != nullptr) {
                value->decrementRefCount();
            }
        }
    	delete xsltExecutable;
     }
}

/*
PHP_METHOD(XsltExecutable, getXslMessages)
{
    XsltExecutable *xsltExecutable;

    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != NULL) {
        xsltExecutable->exceptionClear();
        XdmValue * values = xsltExecutable->getXslMessages();
        if(values != NULL) {
            if (object_init_ex(return_value, xdmValue_ce) != SUCCESS) {
                RETURN_NULL();
            } else {
                //struct xdmValue_object* vobj = (struct xdmValue_object *)Z_OBJ_P(return_value);
                zend_object *vvobj =  Z_OBJ_P(return_value);
                xdmValue_object* vobj  = (xdmValue_object *)((char *)vvobj - XtOffsetOf(xdmValue_object, std));

                assert (vobj != NULL);
                vobj->xdmValue = values;
            }
        } else if(xsltExecutable->exceptionOccurred()){
            SaxonApiException * exception = xsltExecutable->getException();
            if(exception != nullptr) {
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), exception->getMessage(), 0 TSRMLS_CC);
                delete exception;
            }
        }
        RETURN_NULL();
    } else {
        RETURN_NULL();
    }
    }    */




PHP_METHOD(XsltExecutable, getResultDocuments)
{
    XsltExecutable *xsltExecutable;
    zval * php_resultDocArr;

    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != NULL) {
        xsltExecutable->exceptionClear();
        std::map<std::string,XdmValue*> cpp_resultDocMap = xsltExecutable->getResultDocuments();
        if(cpp_resultDocMap.size() == 0) {
            RETURN_NULL();
        }
        //array_init(return_value);
        //XdmMap result = SaxonProcessor::makeMap2(cpp_resultDocMap);
        std::map<std::string,XdmValue*>cpp_resultDocMap2;
        for (auto i = cpp_resultDocMap.begin(); i != cpp_resultDocMap.end(); ++i) {
            cpp_resultDocMap2[i->first] = (XdmValue *)i->second;
                       /* zval * nodei_Val;
                        struct xdmNode_object* nodei_obj = (struct xdmNode_object *)Z_OBJ_P(nodei_Val);
                        zend_object *vvobj =  Z_OBJ_P(nodei_Val);
                        xdmNode_object* vobj  = (xdmNode_object *)((char *)vvobj - XtOffsetOf(xdmNode_object, std));

                        n = (node_object*) zend_object_store_get_object(php_resultDocArr TSRMLS_CC);
                        n->node = i->second;
                        add_index_zval(return_value, i->first, php_resultDocArr);*/
        }
        XdmMap * result = SaxonProcessor::makeMap2(cpp_resultDocMap2);
              if(result != nullptr) {
                            result->incrementRefCount();
                            if (object_init_ex(return_value, xdmMap_ce) != SUCCESS) {
                                RETURN_NULL();
                            } else {
                                zend_object *vvobj =  Z_OBJ_P(return_value);
		                        xdmMap_object* vobj  = (xdmMap_object *)((char *)vvobj - XtOffsetOf(xdmMap_object, std));
		                        assert (vobj != nullptr);
		                        vobj->xdmMap = result;
		                        return;
                            }
                        } else {
        	                RETURN_NULL();

        	            }

        } else if(xsltExecutable->exceptionOccurred()){
            SaxonApiException * exception = xsltExecutable->getException();
            if(exception != nullptr) {
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), exception->getMessage(), 0 TSRMLS_CC);
                delete exception;
            }
            RETURN_NULL();
        }
        RETURN_NULL();
}


PHP_METHOD(XsltExecutable, exportStylesheet)
{
    XsltExecutable *xsltExecutable;
    char * stylesheet;
    size_t len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "s", &stylesheet, &len) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object * obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {

        if(stylesheet != nullptr) {
            xsltExecutable->exceptionClear();
            xsltExecutable->exportStylesheet(stylesheet);
            if(xsltExecutable->exceptionOccurred()){
                SaxonApiException * exception = xsltExecutable->getException();
                if(exception != nullptr) {
                    zend_throw_exception(zend_exception_get_default(TSRMLS_C), exception->getMessage(), 0 TSRMLS_CC);
                    delete exception;
                }
            }
        }
    }
}




PHP_METHOD(XsltExecutable, setcwd)
{
    XsltExecutable *xsltExecutable;
    char * cwdStr;
    size_t len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "s", &cwdStr, &len) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object * obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {

        if(cwdStr != nullptr) {
            xsltExecutable->setcwd(cwdStr);
        }
    }
}


PHP_METHOD(XsltExecutable, setBaseOutputURI)
{
    XsltExecutable *xsltExecutable;
    char * baseOutputURI = nullptr;
    size_t len1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "s", &baseOutputURI, &len1) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr && baseOutputURI != nullptr) {

        xsltExecutable->setBaseOutputURI(baseOutputURI);


    }
}



PHP_METHOD(XsltExecutable, callFunctionReturningValue)
{
//arguments: const char* functionName, XdmValue * arguments, int argument_length

    XsltExecutable *xsltExecutable;
    HashTable *arr_hash;
    char* functionName = nullptr;
    zval * arguments_zval;
    zval * val;
    long num_key;
    XdmValue ** arguments = nullptr;
    int argument_length=0;
    zend_string *key;

    size_t len1;

    if (ZEND_NUM_ARGS()>2) {
        WRONG_PARAM_COUNT;
    }

    if (ZEND_NUM_ARGS()==1 && zend_parse_parameters(ZEND_NUM_ARGS() , "s", &functionName, &len1, &arguments_zval) == FAILURE) {
        RETURN_NULL();
    } else if (ZEND_NUM_ARGS()==2 && zend_parse_parameters(ZEND_NUM_ARGS() , "sa", &functionName, &len1, &arguments_zval) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    arr_hash = Z_ARRVAL_P(arguments_zval);
    argument_length = zend_hash_num_elements(arr_hash);

    arguments = new XdmValue*[argument_length];

    if (xsltExecutable != nullptr) {
      ZEND_HASH_FOREACH_KEY_VAL(arr_hash, num_key, key, val) {
        if(Z_TYPE_P(val) != NULL) {
	        const char * objName =ZSTR_VAL(Z_OBJCE_P(val)->name);

        if(strcmp(objName, "Saxon\\XdmNode")==0) {
		    zend_object* ooth = Z_OBJ_P(val);
		    xdmNode_object * nobj = (xdmNode_object *)((char *)ooth - XtOffsetOf(xdmNode_object, std));
        	if(nobj != nullptr) {
            		XdmValue * value = (XdmValue *) nobj->xdmNode;
            		arguments[num_key] = value;
        	}
	    } else if(strcmp(objName, "Saxon\\XdmAtomicValue")==0) {
		    zend_object* ooth = Z_OBJ_P(val);
		    xdmAtomicValue_object * nobj = (xdmAtomicValue_object *)((char *)ooth - XtOffsetOf(xdmAtomicValue_object, std));
        	if(nobj != nullptr) {
        	    XdmValue * value = (XdmValue *) nobj->xdmAtomicValue;
        	    arguments[num_key] = value;
        	}
	    } else if(strcmp(objName, "Saxon\\XdmValue")==0) {
		zend_object* ooth = Z_OBJ_P(val);
		xdmValue_object * nobj = (xdmValue_object *)((char *)ooth - XtOffsetOf(xdmValue_object, std));
        	if(nobj != nullptr) {
            		XdmValue * value = nobj->xdmValue;
            		arguments[num_key] = value;
        	}
	}
	else {//TODO error warning}
         }}else {
		//TODO error warning
        }
      } ZEND_HASH_FOREACH_END();


	    xsltExecutable->exceptionClear();
        XdmValue * result = xsltExecutable->callFunctionReturningValue(functionName, arguments, argument_length);
	if(result != nullptr) {
	        result->incrementRefCount();
            if (object_init_ex(return_value, xdmValue_ce) != SUCCESS) {
                RETURN_NULL();
            } else {
		zend_object *ooth =  Z_OBJ_P(return_value);
                //struct xdmValue_object* vobj = (struct xdmValue_object *)Z_OBJ_P(return_value TSRMLS_CC);
		xdmValue_object* vobj  = (xdmValue_object *)((char *)ooth - XtOffsetOf(xdmValue_object, std));
                assert (vobj != nullptr);
                vobj->xdmValue = result;
            }
        } else {
            if(obj->xsltExecutable->exceptionOccurred()){
  		//TODO - create PHP exception
	    }
	    RETURN_NULL();
        }
    }


}

PHP_METHOD(XsltExecutable, callFunctionReturningString){
    XsltExecutable *xsltExecutable;
    HashTable *arr_hash;
    char* functionName;
    zval * arguments_zval;
    zval * val;
    long num_key;
    XdmValue ** arguments = nullptr;
    int argument_length=0;
    zend_string *key;

    size_t len1;


     if (ZEND_NUM_ARGS()>2) {
        WRONG_PARAM_COUNT;
    }

     if (ZEND_NUM_ARGS()==1 && zend_parse_parameters(ZEND_NUM_ARGS() , "s", &functionName, &len1, &arguments_zval) == FAILURE) {
        RETURN_NULL();
      }else if (ZEND_NUM_ARGS()==2 && zend_parse_parameters(ZEND_NUM_ARGS() , "sa", &functionName, &len1, &arguments_zval) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    arr_hash = Z_ARRVAL_P(arguments_zval);
    argument_length = zend_hash_num_elements(arr_hash);

    arguments = new XdmValue*[argument_length];

    if (xsltExecutable != nullptr) {
      ZEND_HASH_FOREACH_KEY_VAL(arr_hash, num_key, key, val) {
        if(Z_TYPE_P(val) != NULL) {
	const char * objName =ZSTR_VAL(Z_OBJCE_P(val)->name);
      /*std::cerr<<"test type:"<<(Z_OBJCE_P(val)->name)<<std::endl;
	php_printf("num_key %d =>", num_key);
	php_printf("key %d =>", key);
	*/
	if(strcmp(objName, "Saxon\\XdmNode")==0) {
		zend_object* ooth = Z_OBJ_P(val);
		xdmNode_object * nobj = (xdmNode_object *)((char *)ooth - XtOffsetOf(xdmNode_object, std));
        	if(nobj != nullptr) {
            		XdmValue * value = (XdmValue *) nobj->xdmNode;
            		arguments[num_key] = value;
        	}
	}
	else if(strcmp(objName, "Saxon\\XdmAtomicValue")==0) {
		zend_object* ooth = Z_OBJ_P(val);
		xdmAtomicValue_object * nobj = (xdmAtomicValue_object *)((char *)ooth - XtOffsetOf(xdmAtomicValue_object, std));
        	if(nobj != nullptr) {
            		XdmValue * value = (XdmValue *) nobj->xdmAtomicValue;
            		arguments[num_key] = value;
        	}
	}
	else if(strcmp(objName, "Saxon\\XdmValue")==0) {
		zend_object* ooth = Z_OBJ_P(val);
		xdmValue_object * nobj = (xdmValue_object *)((char *)ooth - XtOffsetOf(xdmValue_object, std));
        	if(nobj != nullptr) {
            		XdmValue * value = nobj->xdmValue;
            		arguments[num_key] = value;
        	}
	}
	else {//TODO error warning}
         }}else {
		//TODO error warning
        }
      } ZEND_HASH_FOREACH_END();

        const char * result = xsltExecutable->callFunctionReturningString(functionName, arguments, argument_length);
	if(result != nullptr) {
            //char *str = estrdup(result);
            _RETURN_STRING(result);
        } else if(xsltExecutable->exceptionOccurred()){
            //TODO: xsltExecutable->getException() then build PHP exception

        }
    }



}


    PHP_METHOD(XsltExecutable, callFunctionReturningFile){
    HashTable *arr_hash;
    XsltExecutable *xsltExecutable;
    char* functionName;
    zval * arguments_zval;
    zval * val;
    long num_key;
    XdmValue ** arguments = nullptr;
    int argument_length=0;
    zend_string *key;

    char * outfilename;
    size_t len1, len2, len3;


    if (ZEND_NUM_ARGS()==3 && zend_parse_parameters(ZEND_NUM_ARGS() , "sas", &functionName, &len2, &arguments_zval, &outfilename, &len3) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    arr_hash = Z_ARRVAL_P(arguments_zval);
    argument_length = zend_hash_num_elements(arr_hash);

    arguments = new XdmValue*[argument_length];

    if (xsltExecutable != nullptr) {
      ZEND_HASH_FOREACH_KEY_VAL(arr_hash, num_key, key, val) {
        if(Z_TYPE_P(val) != NULL) {
	const char * objName =ZSTR_VAL(Z_OBJCE_P(val)->name);
      /*std::cerr<<"test type:"<<(Z_OBJCE_P(val)->name)<<std::endl;
	php_printf("num_key %d =>", num_key);
	php_printf("key %d =>", key);
	*/
	if(strcmp(objName, "Saxon\\XdmNode")==0) {
		zend_object* ooth = Z_OBJ_P(val);
		xdmNode_object * nobj = (xdmNode_object *)((char *)ooth - XtOffsetOf(xdmNode_object, std));
        	if(nobj != nullptr) {
            		XdmValue * value = (XdmValue *) nobj->xdmNode;
            		arguments[num_key] = value;
        	}
	}
	else if(strcmp(objName, "Saxon\\XdmAtomicValue")==0) {
		zend_object* ooth = Z_OBJ_P(val);
		xdmAtomicValue_object * nobj = (xdmAtomicValue_object *)((char *)ooth - XtOffsetOf(xdmAtomicValue_object, std));
        	if(nobj != nullptr) {
            		XdmValue * value = (XdmValue *) nobj->xdmAtomicValue;
            		arguments[num_key] = value;
        	}
	}
	else if(strcmp(objName, "Saxon\\XdmValue")==0) {
		zend_object* ooth = Z_OBJ_P(val);
		xdmValue_object * nobj = (xdmValue_object *)((char *)ooth - XtOffsetOf(xdmValue_object, std));
        	if(nobj != nullptr) {
            		XdmValue * value = nobj->xdmValue;
            		arguments[num_key] = value;
        	}
	}
	else {//TODO error warning}
         }}else {
		//TODO error warning
        }
      } ZEND_HASH_FOREACH_END();


        xsltExecutable->callFunctionReturningFile(functionName, arguments, argument_length, outfilename);
	if(xsltExecutable->exceptionOccurred()){
//TODO: xsltExecutable->getException() then build PHP exception
        }
    }

}

    PHP_METHOD(XsltExecutable, callTemplateReturningValue){

  XsltExecutable *xsltExecutable;
    char* templateName = NULL;

    size_t len2 = 0;

    if (ZEND_NUM_ARGS()>1) {
        WRONG_PARAM_COUNT;
    }
    if (ZEND_NUM_ARGS()==1 && zend_parse_parameters(ZEND_NUM_ARGS() , "s", &templateName, &len2) == FAILURE) {
        RETURN_NULL();
    }


    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {


	//php_printf(" argument_length= %d",argument_length);
        XdmValue * result = xsltExecutable->callTemplateReturningValue((len2 == 0 ? NULL :templateName));

	if(result != nullptr) {
	    result->incrementRefCount();
            if (object_init_ex(return_value, xdmValue_ce) != SUCCESS) {
               RETURN_NULL();
            } else {
		zend_object *ooth =  Z_OBJ_P(return_value);

		xdmValue_object* vobj  = (xdmValue_object *)((char *)ooth - XtOffsetOf(xdmValue_object, std));
                assert (vobj != nullptr);
                vobj->xdmValue = result;

            }
        } else {
            if(obj->xsltExecutable->exceptionOccurred()){
  		//TODO
	    }
	    RETURN_NULL();
        }
    }
   }


    PHP_METHOD(XsltExecutable, callTemplateReturningString){
    XsltExecutable *xsltExecutable;
    char* templateName = nullptr;

    size_t len1, len2;

    if (ZEND_NUM_ARGS()>1) {
        WRONG_PARAM_COUNT;
    }

    if (ZEND_NUM_ARGS()==1 && zend_parse_parameters(ZEND_NUM_ARGS() , "s", &templateName, &len2) == FAILURE) {
        RETURN_NULL();
    }


    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {


	//php_printf(" argument_length= %d",argument_length);
        const char * result = xsltExecutable->callTemplateReturningString((len2 == 0 ? NULL :templateName));
	if(result != nullptr) {
            _RETURN_STRING(result);
        } else {
            if(xsltExecutable->exceptionOccurred()){
//TODO: xsltExecutable->getException() then build PHP exception
           }
        }
    }

     RETURN_NULL();
    }

    PHP_METHOD(XsltExecutable, callTemplateReturningFile){
   XsltExecutable *xsltExecutable;
    char* templateName = nullptr;
    char * filename = nullptr;
    size_t len2;

    if (ZEND_NUM_ARGS()>1) {
        WRONG_PARAM_COUNT;
    }
    if (zend_parse_parameters(ZEND_NUM_ARGS() , "s", &templateName, &len2) == FAILURE) {
        RETURN_NULL();
    }


    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {
   	xsltExecutable->callTemplateReturningFile((len2 == 0 ? NULL :templateName), filename);
    }

  }


PHP_METHOD(XsltExecutable, applyTemplatesReturningValue){
   XsltExecutable *xsltExecutable;
    size_t len1;

    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {
   	XdmValue * result = xsltExecutable->applyTemplatesReturningValue();

	if(result != nullptr) {
	        result->incrementRefCount();
            if (object_init_ex(return_value, xdmValue_ce) != SUCCESS) {
                RETURN_NULL();
            } else {
		zend_object *ooth =  Z_OBJ_P(return_value);
        	xdmValue_object* vobj  = (xdmValue_object *)((char *)ooth - XtOffsetOf(xdmValue_object, std));
                assert (vobj != nullptr);
                vobj->xdmValue = result;
            }
        } else {
            if(obj->xsltExecutable->exceptionOccurred()){
  		//TODO
	    }
	    RETURN_NULL();
        }



    } else {
     RETURN_NULL();
    }
}


PHP_METHOD(XsltExecutable, applyTemplatesReturningString){
    XsltExecutable *xsltExecutable;
    size_t len1;

    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {
        const char * result = xsltExecutable->applyTemplatesReturningString();

	if(result != nullptr) {
            _RETURN_STRING(result);
        } else {
            if(xsltExecutable->exceptionOccurred()){
//TODO: xsltExecutable->getException() then build PHP exception
           }
        }
    }

     RETURN_NULL();



}



PHP_METHOD(XsltExecutable, applyTemplatesReturningFile){
 XsltExecutable *xsltExecutable;
    char * filename = nullptr;
    size_t len1;

    if (ZEND_NUM_ARGS() == 1) {

        if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &filename, &len1) == FAILURE) {
            RETURN_NULL();
        } else {
            WRONG_PARAM_COUNT;
        }
    }


    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {
   	    xsltExecutable->applyTemplatesReturningFile(filename);
    }
    //TODO check for exception



}


/*PHP_METHOD(XsltExecutable, addPackages){
    HashTable *arr_hash;
    XsltExecutable *xsltExecutable;
    char* functionName = nullptr;
    zval * arguments_zval;
    zval * val;
    long num_key;
    const char ** arguments = nullptr;
    int argument_length=0;
    zend_string *key;

    char * infilename = nullptr;

    size_t len1;

        if (zend_parse_parameters(ZEND_NUM_ARGS() , "a", &arguments_zval) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    arr_hash = Z_ARRVAL_P(arguments_zval);
    argument_length = zend_hash_num_elements(arr_hash);
   
    arguments = new const char*[argument_length];

  if (xsltExecutable != nullptr) {
      ZEND_HASH_FOREACH_KEY_VAL(arr_hash, num_key, key, val) {
        if(Z_TYPE_P(val) != nullptr && Z_TYPE_P(val)== IS_STRING) {

		arguments[num_key] = (const char*)Z_STRVAL_P(val);
     		
        } 
    }  ZEND_HASH_FOREACH_END();

        xsltExecutable->addPackages(arguments, argument_length);
	if(xsltExecutable->exceptionOccurred()){
//TODO: xsltExecutable->getException() then build PHP exception
        }
    }
}    */


    PHP_METHOD(XsltExecutable,setInitialTemplateParameters){

  XsltExecutable *xsltExecutable;
    HashTable *arr_hash;
    zval * arguments_zval;
    zval * val;
    long num_key;
    XdmValue ** arguments;
    int argument_length=0;
    zend_string *key;

    bool tunnel = false;
    if (ZEND_NUM_ARGS()==1 && zend_parse_parameters(ZEND_NUM_ARGS() , "a", &arguments_zval) == FAILURE) {
        RETURN_NULL();
    }

    if (ZEND_NUM_ARGS()==2 && zend_parse_parameters(ZEND_NUM_ARGS() , "ab", &arguments_zval, &tunnel) == FAILURE) {
        RETURN_NULL();
    }


    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    arr_hash = Z_ARRVAL_P(arguments_zval);
    argument_length = zend_hash_num_elements(arr_hash);

    arguments = new XdmValue*[argument_length];
    std::map<std::string,XdmValue*> parameters;
    if (xsltExecutable != nullptr) {
      ZEND_HASH_FOREACH_KEY_VAL(arr_hash, num_key, key, val) {
        if(Z_TYPE_P(val) != NULL) {
	const char * objName =ZSTR_VAL(Z_OBJCE_P(val)->name);
      /*std::cerr<<"test type:"<<(Z_OBJCE_P(val)->name)<<std::endl;
	php_printf("num_key %d =>", num_key);
	php_printf("key %d =>", key);
	*/
	if(strcmp(objName, "Saxon\\XdmNode")==0) {
		zend_object* ooth = Z_OBJ_P(val);
		xdmNode_object * nobj = (xdmNode_object *)((char *)ooth - XtOffsetOf(xdmNode_object, std));
        	if(nobj != nullptr) {
            		XdmValue * value = (XdmValue *) nobj->xdmNode;
            		value->incrementRefCount();
            		parameters[ZSTR_VAL(key)] = value;
        	}
	}
	else if(strcmp(objName, "Saxon\\XdmAtomicValue")==0) {
		zend_object* ooth = Z_OBJ_P(val);
		xdmAtomicValue_object * nobj = (xdmAtomicValue_object *)((char *)ooth - XtOffsetOf(xdmAtomicValue_object, std));
        	if(nobj != nullptr) {
            		XdmValue * value = (XdmValue *) nobj->xdmAtomicValue;
            		value->incrementRefCount();
            		parameters[ZSTR_VAL(key)] = value;
        	}
	}
	else if(strcmp(objName, "Saxon\\XdmValue")==0) {
		zend_object* ooth = Z_OBJ_P(val);
		xdmValue_object * nobj = (xdmValue_object *)((char *)ooth - XtOffsetOf(xdmValue_object, std));
        	if(nobj != nullptr) {
            		XdmValue * value = nobj->xdmValue;
            		value->incrementRefCount();
            		parameters[ZSTR_VAL(key)] = value;
            		parameters[ZSTR_VAL(key)] = value;
        	}
	}
	else {//TODO error warning}
         }}else {
		//TODO error warning
        }
      } ZEND_HASH_FOREACH_END();


	//php_printf(" argument_length= %d",argument_length);
        xsltExecutable->setInitialTemplateParameters(parameters, tunnel);

    }

    }


PHP_METHOD(XsltExecutable, setInitialMatchSelection){
   XsltExecutable *xsltExecutable;
    zval* oth = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "z", &oth) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {

    if(!oth) {
	php_error(E_WARNING, "Error setting source value");
	return;
    } else {
	if(Z_TYPE_P(oth) ==IS_NULL){
		php_error(E_WARNING, "Error setting source value - NULL found");
		return;
	}

      const char * objName =ZSTR_VAL(Z_OBJCE_P(oth)->name);
      //std::cout<<"test type:"<<(Z_OBJCE_P(oth)->name)<<std::endl;


      if(strcmp(objName, "Saxon\\XdmValue")==0) {
	zend_object * nobj = Z_OBJ_P(oth);

	xdmValue_object* ooth = (xdmValue_object *)((char *)nobj - XtOffsetOf(xdmValue_object, std));//(xdmNode_object*)Z_OBJ_P(oth);
        if(ooth != nullptr) {
            XdmValue * value = ooth->xdmValue;
            if(value != nullptr) {
		        xsltExecutable->setInitialMatchSelection(value);
                value->incrementRefCount();
            }
        }
      } else if(strcmp(objName, "Saxon\\XdmNode")==0) {
	zend_object * nobj = Z_OBJ_P(oth);

	xdmNode_object* ooth = (xdmNode_object *)((char *)nobj - XtOffsetOf(xdmNode_object, std));//(xdmNode_object*)Z_OBJ_P(oth);
        if(ooth != nullptr) {
            XdmNode * value = ooth->xdmNode;
            if(value != nullptr) {
		XdmValue  *valueX = (XdmValue*)value;
	        xsltExecutable->setInitialMatchSelection(valueX);
               valueX->incrementRefCount();
            } else {

php_error(E_WARNING, "XdmNode - seInitialMatchSelection - ERROR");
}
        }
      } else if(strcmp(objName, "Saxon\\XdmAtomicValue")==0) {
	zend_object* vvobj = Z_OBJ_P(oth);

	xdmAtomicValue_object* ooth = (xdmAtomicValue_object *)((char *)vvobj - XtOffsetOf(xdmAtomicValue_object, std));
        if(ooth != nullptr) {
            XdmAtomicValue * value = ooth->xdmAtomicValue;
            if(value != nullptr) {
	        xsltExecutable->setInitialMatchSelection((XdmValue*)value);
	        value->incrementRefCount();
            }
        }
      } else {
	//TODO raise warning
	}


    }
  }




}


PHP_METHOD(XsltExecutable, setGlobalContextItem)
{
    XsltExecutable *xsltExecutable;
    zval* oth = nullptr;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "z", &oth) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {

    if(!oth) {
	php_error(E_WARNING, "Error setting source value");
	return;
    } else {
	if(Z_TYPE_P(oth) ==IS_NULL){
		php_error(E_WARNING, "Error setting source value");
		return;
	}

      const char * objName =ZSTR_VAL(Z_OBJCE_P(oth)->name);
      //std::cout<<"test type:"<<(Z_OBJCE_P(oth)->name)<<std::endl;

      if(strcmp(objName, "Saxon\\XdmNode")==0) {
	zend_object * nobj = Z_OBJ_P(oth);

	xdmNode_object* ooth = (xdmNode_object *)((char *)nobj - XtOffsetOf(xdmNode_object, std));//(xdmNode_object*)Z_OBJ_P(oth);
        if(ooth != nullptr) {
            XdmNode * value = ooth->xdmNode;
            if(value != nullptr) {
		XdmItem  *valueX = (XdmItem*)value;
	        xsltExecutable->setGlobalContextItem(valueX);
            value->incrementRefCount();
            }
        }
      } else if(strcmp(objName, "Saxon\\XdmAtomicValue")==0) {
	zend_object* vvobj = Z_OBJ_P(oth);
	xdmAtomicValue_object* ooth = (xdmAtomicValue_object *)((char *)vvobj - XtOffsetOf(xdmAtomicValue_object, std));
        if(ooth != nullptr) {
            XdmAtomicValue * value = ooth->xdmAtomicValue;
            if(value != nullptr) {
	        xsltExecutable->setGlobalContextItem((XdmItem*)value);
	        value->incrementRefCount();
            }
        }
      } else {
	//TODO raise warning
	}


    }
  }
}


PHP_METHOD(XsltExecutable, setGlobalContextFromFile)
{
    XsltExecutable *xsltExecutable;
    char * inFilename = nullptr;
    size_t len1;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &inFilename, &len1) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr && inFilename != nullptr) {
	 xsltExecutable->setGlobalContextFromFile(inFilename);


    }
}




PHP_METHOD(XsltExecutable, setInitialMode)
{
    XsltExecutable *xsltExecutable;
    char * modeName = nullptr;
    size_t len1;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &modeName, &len1) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr && modeName != nullptr) {
	 xsltExecutable->setInitialMode(modeName);


    }
}


PHP_METHOD(XsltExecutable, setInitialMatchSelectionAsFile)
{
    XsltExecutable *xsltExecutable;
    char * inFilename = nullptr;
    size_t len1;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &inFilename, &len1) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {
	 xsltExecutable->setInitialMatchSelectionAsFile(inFilename);


    }
}


PHP_METHOD(XsltExecutable, transformFileToFile)
{
    XsltExecutable *xsltExecutable;
    char * outfileName = nullptr;
    char * infilename = nullptr;
    size_t len1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "ss", &infilename, &len1, &outfileName, &len1) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {

        xsltExecutable->transformFileToFile(infilename, outfileName);
        if(xsltExecutable->exceptionOccurred()) {
     	  // TODO: throw exception
        }
    }
}

PHP_METHOD(XsltExecutable, transformFileToValue)
{
    XsltExecutable *xsltExecutable;
    char * infilename = nullptr;
   size_t len1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "s", &infilename, &len1) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {

        XdmValue * node = xsltExecutable->transformFileToValue(infilename);
        if(node != nullptr) {
            node->incrementRefCount();
            if (object_init_ex(return_value, xdmValue_ce) != SUCCESS) {
                RETURN_NULL();
            } else {
                //struct xdmValue_object* vobj = (struct xdmValue_object *)Z_OBJ_P(return_value TSRMLS_CC);
		zend_object *ooth =  Z_OBJ_P(return_value);
		xdmValue_object* vobj  = (xdmValue_object *)((char *)ooth - XtOffsetOf(xdmValue_object, std));
                assert (vobj != nullptr);
                vobj->xdmValue = node;
            }
        } else {
            if(obj->xsltExecutable->exceptionOccurred()){
  		//TODO
	    }
        }
    }else {
        RETURN_NULL();
    }
}


PHP_METHOD(XsltExecutable, transformFileToString)
{
    XsltExecutable *xsltExecutable;
    char * infilename = nullptr;
    size_t len1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "s", &infilename, &len1) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {

        const char * result = xsltExecutable->transformFileToString(infilename);
	if(result != nullptr) {
            //char *str = estrdup(result);
            _RETURN_STRING(result);
        } else if(xsltExecutable->exceptionOccurred()){
//TODO: xsltExecutable->getException() then build PHP exception
        }
    }
}



PHP_METHOD(XsltExecutable, transformToString)
{
    XsltExecutable *xsltExecutable;

    if (ZEND_NUM_ARGS()>1) {
        WRONG_PARAM_COUNT;
    }


    zval* oth = nullptr;


    if (ZEND_NUM_ARGS() == 1 && zend_parse_parameters(ZEND_NUM_ARGS() , "z", &oth) == FAILURE) {
        RETURN_NULL();
    }


    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {

     const char * result = nullptr;
     if(ZEND_NUM_ARGS() == 0 || !oth) {
        result = xsltExecutable->transformToString();
        if(result != nullptr) {
            _RETURN_STRING(result);
            return;

        }


     } else {
         if(Z_TYPE_P(oth) ==IS_NULL){
		    result = xsltExecutable->transformToString();

	     } else {

            const char * objName =ZSTR_VAL(Z_OBJCE_P(oth)->name);
            if(strcmp(objName, "Saxon\\XdmNode")==0) {
	            zend_object * nobj = Z_OBJ_P(oth);

	            xdmNode_object* ooth = (xdmNode_object *)((char *)nobj - XtOffsetOf(xdmNode_object, std));//(xdmNode_object*)Z_OBJ_P(oth);
                if(ooth != nullptr) {
                    XdmNode * value = ooth->xdmNode;
                    if(value != nullptr) {
        	            result = xsltExecutable->transformToString(value);
        	            value->incrementRefCount();
                    }
                }
            }
        }
        if(result != nullptr) {
           _RETURN_STRING(result);
        }
     }

    }

    RETURN_NULL();
}

PHP_METHOD(XsltExecutable, transformToValue)
{
    XsltExecutable *xsltExecutable;

    if (ZEND_NUM_ARGS()>1) {
        WRONG_PARAM_COUNT;
    }


    zval* oth;


    if (ZEND_NUM_ARGS() == 1 && zend_parse_parameters(ZEND_NUM_ARGS() , "z", &oth) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {

        XdmValue * node = nullptr;
        if(ZEND_NUM_ARGS() == 0 || !oth) {
            node = xsltExecutable->transformToValue();

        } else {

            if(Z_TYPE_P(oth) == IS_NULL){
                //php_error(E_WARNING, "Error setting source value");
                node = xsltExecutable->transformToValue();

            } else {

            const char *objName = ZSTR_VAL(Z_OBJCE_P(oth)->name);

            if(strcmp(objName,"Saxon\\XdmNode")==0) {
                zend_object *nobj = Z_OBJ_P(oth);

                xdmNode_object *ooth = (xdmNode_object *) ((char *) nobj -
                                           XtOffsetOf(xdmNode_object, std));//(xdmNode_object*)Z_OBJ_P(oth);
                if(ooth != nullptr) {
                    XdmNode *value = ooth->xdmNode;
                    if(value != nullptr) {
                        node = xsltExecutable->transformToValue(value);
                    }
                }

            }
            }
        }
        if(node != nullptr) {
            node->incrementRefCount();
            if (object_init_ex(return_value, xdmValue_ce) != SUCCESS) {
                RETURN_NULL();
            } else {
                zend_object *vvobj =  Z_OBJ_P(return_value);
		        xdmValue_object* vobj  = (xdmValue_object *)((char *)vvobj - XtOffsetOf(xdmValue_object, std));
		        assert (vobj != nullptr);
		        vobj->xdmValue = node;
		        return;
            }
        }

    }
    RETURN_NULL();

}

PHP_METHOD(XsltExecutable, transformToFile)
{
    XsltExecutable *xsltExecutable;

    if (ZEND_NUM_ARGS()>1) {
        WRONG_PARAM_COUNT;
    }


    zval* oth;


    if (ZEND_NUM_ARGS() == 1 && zend_parse_parameters(ZEND_NUM_ARGS() , "z", &oth) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {
      if(!oth) {
	php_error(E_WARNING, "Error setting source value");
	return;
      } else {
        if(ZEND_NUM_ARGS() == 0) {
            xsltExecutable->transformToFile();
        } else {
            if(Z_TYPE_P(oth) == IS_NULL){
                php_error(E_WARNING,"Error setting source value");
                return;
            }

            const char *objName = ZSTR_VAL(Z_OBJCE_P(oth)->name);

            if(strcmp(objName,"Saxon\\XdmNode")==0) {
                zend_object *nobj = Z_OBJ_P(oth);

                xdmNode_object *ooth = (xdmNode_object *) ((char *) nobj -
                                           XtOffsetOf(xdmNode_object, std));//(xdmNode_object*)Z_OBJ_P(oth);
                if(ooth != nullptr) {
                    XdmNode *value = ooth->xdmNode;
                    if(value != nullptr) {
                        xsltExecutable->transformToFile(value);
                    }
                }
            }

        }
      }
    }
}




PHP_METHOD(XsltExecutable, setResultAsRawValue)
{
    XsltExecutable *xsltExecutable;
    bool raw = false;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "b", &raw) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {
	 xsltExecutable->setResultAsRawValue(raw);
    }

}


PHP_METHOD(XsltExecutable, setOutputFile)
{
    XsltExecutable *xsltExecutable;
    char * outputFilename = nullptr;
    size_t len1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "s", &outputFilename, &len1) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr && outputFilename != nullptr) {

	 xsltExecutable->setOutputFile(outputFilename);


    }
}




PHP_METHOD(XsltExecutable, setProperty)
{
    XsltExecutable *xsltExecutable;
    char * name = nullptr;
    char * value;
    size_t len1, len2, myint;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "ss", &name, &len1, &value, &len2) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {
        xsltExecutable->setProperty(name, value);
    }
}

PHP_METHOD(XsltExecutable, setParameter)
{

   XsltExecutable *xsltExecutable;
   char * name;
   zval* oth;
   size_t len1, len2, myint;
    if (zend_parse_parameters(ZEND_NUM_ARGS() , "sz", &name, &len2, &oth) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {
	if(Z_TYPE_P(oth) ==IS_NULL){
		php_error(E_WARNING, "Error setting source value - value is nullptr");
		return;
	}

      const char * objName =ZSTR_VAL(Z_OBJCE_P(oth)->name);
      //std::cout<<"test type:"<<(Z_OBJCE_P(oth)->name)<<std::endl;

      if(strcmp(objName, "Saxon\\XdmNode")==0) {
	zend_object* ooth = Z_OBJ_P(oth);
	xdmNode_object * nobj = (xdmNode_object *)((char *)ooth - XtOffsetOf(xdmNode_object, std));
        if(nobj != nullptr) {
            XdmNode * value = nobj->xdmNode;
            if(value != nullptr) {
                value->incrementRefCount();
	            xsltExecutable->setParameter(name, (XdmValue *)value);
            }
        }
      } else if(strcmp(objName, "Saxon\\XdmValue")==0){
	zend_object* ooth = Z_OBJ_P(oth);
	xdmValue_object * vobj = (xdmValue_object *)((char *)ooth - XtOffsetOf(xdmValue_object, std));
        if(vobj != nullptr) {
            XdmValue * value = vobj->xdmValue;
            if(value != nullptr) {
                value->incrementRefCount();
                xsltExecutable->setParameter(name, value);
            }
        }



      } else if(strcmp(objName, "Saxon\\XdmAtomicValue")==0){
	zend_object* ooth = Z_OBJ_P(oth);
	xdmAtomicValue_object * aobj = (xdmAtomicValue_object *)((char *)ooth - XtOffsetOf(xdmAtomicValue_object, std));
        if(aobj != nullptr) {
            XdmAtomicValue * value = aobj->xdmAtomicValue;
            if(value != nullptr) {
                value->incrementRefCount();
                xsltExecutable->setParameter(name, (XdmValue *)value);
            }
        }



      }

    }
}

PHP_METHOD(XsltExecutable, clone) {

    XsltExecutable *xsltExecutable;
    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
if (xsltExecutable != nullptr) {
        XsltExecutable * executable = xsltExecutable->clone();
        if(executable != nullptr) {
            if (object_init_ex(return_value, xsltExecutable_ce) != SUCCESS) {
                RETURN_NULL();
            } else {
                zend_object *ooth = Z_OBJ_P(return_value);
                xsltExecutable_object *vobj = (xsltExecutable_object * )((char *) ooth - XtOffsetOf(xsltExecutable_object, std));
                assert (vobj!= nullptr);
                vobj->xsltExecutable = executable;
            }

        } else if(xsltExecutable->exceptionOccurred()){
            SaxonApiException * exception = xsltExecutable->getException();
            if(exception != nullptr) {
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), exception->getMessage(), 0 TSRMLS_CC);
            }

        }
        } else {
        RETURN_NULL();
    }
}

PHP_METHOD(XsltExecutable, clearParameters)
{
    XsltExecutable *xsltExecutable;
    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {
        std::map<std::string, XdmValue *> parameters = xsltExecutable->getParameters();
        for (std::map<std::string, XdmValue *>::iterator itr = parameters.begin(); itr != parameters.end(); itr++) {
            XdmValue *value = itr->second;
            if(value != nullptr) {
                value->decrementRefCount();
                if(value->getRefCount() < 1) {
                    delete value;
                    value = nullptr;
                }
            }
        }
        xsltExecutable->clearParameters();

    }
}

PHP_METHOD(XsltExecutable, clearProperties)
{
    XsltExecutable *xsltExecutable;
    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {
        xsltExecutable->clearProperties();
    }
}


PHP_METHOD(XsltExecutable, setSaveXslMessage)
{
    XsltExecutable *xsltExecutable;
    bool show = false;
    char * filename = nullptr;
    size_t len1;
    if (zend_parse_parameters(ZEND_NUM_ARGS() , "bs", &show, &filename, &len1) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {

	    xsltExecutable->setSaveXslMessage(show, filename);
	}

}




PHP_METHOD(XsltExecutable, setCaptureResultDocuments)
{
    XsltExecutable *xsltExecutable;
    bool create = false;
    size_t len1;
    if (zend_parse_parameters(ZEND_NUM_ARGS() , "b", &create) == FAILURE) {
        RETURN_NULL();
    }

    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;

    if (xsltExecutable != nullptr) {

	    xsltExecutable->setCaptureResultDocuments(create);
	}

}



PHP_METHOD(XsltExecutable, exceptionOccurred)
{
    XsltExecutable *xsltExecutable;
    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }

    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {
        bool result = xsltExecutable->exceptionOccurred();
        RETURN_BOOL(result);
    }
    RETURN_BOOL(false);
}



PHP_METHOD(XsltExecutable, getErrorCode)
{
    XsltExecutable *xsltExecutable;

    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }
    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {
        if(xsltExecutable->exceptionOccurred()){
            SaxonApiException * exception = xsltExecutable->getException();
            if(exception != nullptr) {
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), exception->getMessage(), 0 TSRMLS_CC);
                delete exception;
            }
        }
    }
    RETURN_NULL();
}

PHP_METHOD(XsltExecutable, checkForException)
{
    XsltExecutable *xsltExecutable;
    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }
    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {
        if(xsltExecutable->exceptionOccurred()){
            SaxonApiException * exception = xsltExecutable->getException();
            if(exception != nullptr) {
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), exception->getMessage(), 0 TSRMLS_CC);
                delete exception;
            }
        }
    }
    RETURN_NULL();
}

PHP_METHOD(XsltExecutable, getErrorMessage)
{
    XsltExecutable *xsltExecutable;
    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }
    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {
        if(xsltExecutable->exceptionOccurred()){
            SaxonApiException * exception = xsltExecutable->getException();
            if(exception != nullptr) {
                    const char * errStr = exception->getMessage();
                    if(errStr != NULL) {
                        //char *str = estrdup(errStr);
                        _RETURN_STRING(errStr);
                    }
        }
        }
    }
    RETURN_NULL();
}

PHP_METHOD(XsltExecutable, exceptionClear)
{
    XsltExecutable *xsltExecutable;
    zend_object* pobj = Z_OBJ_P(getThis());
    xsltExecutable_object *obj = (xsltExecutable_object *)((char *)pobj - XtOffsetOf(xsltExecutable_object, std));
    xsltExecutable = obj->xsltExecutable;
    if (xsltExecutable != nullptr) {
        xsltExecutable->exceptionClear();
    }
}


// =============================================================




zend_function_entry XsltExecutable_methods[] = {
    PHP_ME(XsltExecutable,  __destruct,     nullptr, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    PHP_ME(XsltExecutable, callFunctionReturningValue, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, callFunctionReturningString, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, callFunctionReturningFile, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, callTemplateReturningValue, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, callTemplateReturningString, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, callTemplateReturningFile, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, applyTemplatesReturningValue, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, applyTemplatesReturningString, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, applyTemplatesReturningFile, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, setInitialTemplateParameters, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, setInitialMatchSelection, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, setInitialMatchSelectionAsFile, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, setGlobalContextItem, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, setGlobalContextFromFile, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable, setInitialMode, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  transformFileToFile, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  transformFileToString, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  transformFileToValue, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  transformToString, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  transformToValue, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  transformToFile, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  setOutputFile, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  setResultAsRawValue, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  setParameter, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  setProperty, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  setcwd, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  clearParameters, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  clearProperties, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  setSaveXslMessage, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  exceptionOccurred, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  exceptionClear, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  exportStylesheet, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  getErrorCode, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  checkForException, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  getErrorMessage, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  setCaptureResultDocuments,  nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(XsltExecutable,  getResultDocuments, nullptr, ZEND_ACC_PUBLIC)
{nullptr, nullptr, nullptr}
};

