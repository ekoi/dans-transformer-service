

zend_object_handlers schemaValidator_object_handlers;
zend_class_entry *schemaValidator_ce;

/*     ============== PHP Interface of   SchemaValidator =============== */

void schemaValidator_free_storage(zend_object *object)
{
    zend_object_std_dtor(object);
}

void schemaValidator_destroy_storage(zend_object *object)
{
    zend_objects_destroy_object(object);
}

zend_object *schemaValidator_create_handler(zend_class_entry *type)
{
    zval *tmp;
    zend_object retval;
    schemaValidator_object *obj = (schemaValidator_object *)ecalloc(1, sizeof(schemaValidator_object)+ zend_object_properties_size(type));

    zend_object_std_init(&obj->std, type);
    object_properties_init(&obj->std, type);

    obj->std.handlers = &schemaValidator_object_handlers;

    return &obj->std;
}



PHP_METHOD(SchemaValidator, __destruct)
{
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));


    SchemaValidator * schemaValidator= obj->schemaValidator;
    if(schemaValidator != nullptr) {
    	delete schemaValidator;
    }

}



PHP_METHOD(SchemaValidator, registerSchemaFromFile)
{
    SchemaValidator *schemaValidator;
    char * name;
    size_t len1;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &name, &len1) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (name != nullptr && schemaValidator != nullptr) {
        schemaValidator->registerSchemaFromFile(name);
    }
}

PHP_METHOD(SchemaValidator, registerSchemaFromString)
{
    char * schemaStr;
    size_t len1;
    SchemaValidator *schemaValidator;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &schemaStr, &len1) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaStr!= nullptr && schemaValidator != nullptr) {
        schemaValidator->registerSchemaFromString(schemaStr);
    }
}

PHP_METHOD(SchemaValidator, validate)
{
    char * name = nullptr;
    size_t len1;
    SchemaValidator *schemaValidator;
    if (ZEND_NUM_ARGS()>1) {
        WRONG_PARAM_COUNT;
    }
    if (ZEND_NUM_ARGS()>0 && zend_parse_parameters(ZEND_NUM_ARGS() , "s", &name, &len1) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        schemaValidator->validate(name);
    }
}

PHP_METHOD(SchemaValidator, validateToNode)
{
    char * name = nullptr;
   size_t len1;
    SchemaValidator *schemaValidator;
    if (ZEND_NUM_ARGS()>1) {
        WRONG_PARAM_COUNT;
    }
    if (ZEND_NUM_ARGS()>0 && zend_parse_parameters(ZEND_NUM_ARGS() , "s", &name, &len1) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        XdmNode * node = schemaValidator->validateToNode(name);
	if(node != nullptr) {
	    if (object_init_ex(return_value, xdmNode_ce) != SUCCESS) {
                RETURN_NULL();
                return;
            } else {
                //struct xdmNode_object* vobj = (struct xdmNode_object *)Z_OBJ_P(return_value TSRMLS_CC);
		zend_object* vvobj = Z_OBJ_P(return_value);
 		xdmNode_object * vobj = (xdmNode_object *)((char *)vvobj - XtOffsetOf(xdmNode_object, std));
                assert (vobj != nullptr);
                vobj->xdmNode = node;
                return;
            }
	}
    	schemaValidator->checkException();//TODO
    }
    RETURN_NULL();
}



PHP_METHOD(SchemaValidator, setLax)
{

    SchemaValidator *schemaValidator;

    bool isLax = false;

    if (ZEND_NUM_ARGS()>1) {
        WRONG_PARAM_COUNT;
    }


    if (zend_parse_parameters(ZEND_NUM_ARGS() , "b", &isLax) == FAILURE) {
        RETURN_NULL();
    }


    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        schemaValidator->setLax(isLax);
    }
}


PHP_METHOD(SchemaValidator, getValidationReport)
{

    SchemaValidator *schemaValidator;
    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }

    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        XdmNode * node = schemaValidator->getValidationReport();
	if(node != nullptr) {
	    if (object_init_ex(return_value, xdmNode_ce) != SUCCESS) {

                RETURN_NULL();
                return;
            } else {
                zend_object* vvobj = Z_OBJ_P(return_value);
 		xdmNode_object * vobj = (xdmNode_object *)((char *)vvobj - XtOffsetOf(xdmNode_object, std));
                assert (vobj != nullptr);
                vobj->xdmNode = node;
                return;
            }
	}
    	schemaValidator->checkException();//TODO
    }
    RETURN_NULL();
}


PHP_METHOD(SchemaValidator, setSourceNode)
{
    SchemaValidator *schemaValidator;

    zval* oth;


    if (zend_parse_parameters(ZEND_NUM_ARGS() , "z", &oth) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
	const char * objName = ZSTR_VAL(Z_OBJCE_P(oth)->name);
	if(strcmp(objName, "Saxon\\XdmNode")==0) {
	zend_object * nobj = Z_OBJ_P(oth);
	xdmNode_object* ooth = (xdmNode_object *)((char *)nobj - XtOffsetOf(xdmNode_object, std));
        if(ooth != nullptr) {
            XdmNode * value = ooth->xdmNode;
            if(value != nullptr) {
		schemaValidator->setSourceNode(value);

            }
        }


      }
    }
}

PHP_METHOD(SchemaValidator, setOutputFile)
{
    SchemaValidator *schemaValidator;
    char * name;
    int len1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "s", &name, &len1) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        schemaValidator->setOutputFile(name);
    }
}


PHP_METHOD(SchemaValidator, exportSchema)
{
    SchemaValidator *schemaValidator;
    char * name;
    int len1;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "s", &name, &len1) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        schemaValidator->exportSchema(name);
    }
}


PHP_METHOD(SchemaValidator, setProperty)
{
    SchemaValidator *schemaValidator;
    char * name;
    char * value;
    size_t len1, len2, myint;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "ss", &name, &len1, &value, &len2) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        schemaValidator->setProperty(name, value);
    }
}

PHP_METHOD(SchemaValidator, setParameter)
{

   SchemaValidator *schemaValidator;
   char * name;
   zval* oth;
   size_t len1, len2, myint;
    if (zend_parse_parameters(ZEND_NUM_ARGS() , "sz", &name, &len2, &oth) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
      const char * objName =ZSTR_VAL(Z_OBJCE_P(oth)->name);
      //std::cout<<"test type:"<<(Z_OBJCE_P(oth)->name)<<std::endl;

      if(strcmp(objName, "Saxon\\XdmNode")==0) {
	zend_object * nobj = Z_OBJ_P(oth);

	xdmNode_object* ooth = (xdmNode_object *)((char *)nobj - XtOffsetOf(xdmNode_object, std));
        if(ooth != nullptr) {
            XdmNode * value = ooth->xdmNode;
            if(value != nullptr) {
	        schemaValidator->setParameter(name, (XdmValue *)value);

            }
        }
      } else if(strcmp(objName, "Saxon\\XdmValue")==0){
	zend_object* vvobj = Z_OBJ_P(oth);
	xdmValue_object* ooth = (xdmValue_object *)((char *)vvobj - XtOffsetOf(xdmValue_object, std));
        if(ooth != nullptr) {
            XdmValue * value = ooth->xdmValue;
            if(value != nullptr) {

                schemaValidator->setParameter(name, value);
            }
        }



      } else if(strcmp(objName, "Saxon\\XdmAtomicValue")==0){
	zend_object* vvobj = Z_OBJ_P(oth);
	xdmAtomicValue_object* ooth = (xdmAtomicValue_object *)((char *)vvobj - XtOffsetOf(xdmAtomicValue_object, std));
        if(ooth != nullptr) {
            XdmAtomicValue * value = ooth->xdmAtomicValue;
            if(value != nullptr) {

                schemaValidator->setParameter(name, (XdmValue *)value);
            }
        }



      }

    }
}

PHP_METHOD(SchemaValidator, clearProperties)
{
    SchemaValidator *schemaValidator;
    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }

    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        schemaValidator->clearProperties();
	schemaValidator->exceptionClear();
    }
}

PHP_METHOD(SchemaValidator, clearParameters)
{

   SchemaValidator *schemaValidator;
    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {

                schemaValidator->clearParameters(true);
		schemaValidator->exceptionClear();
        }
    }

PHP_METHOD(SchemaValidator, exceptionOccurred)
{
    SchemaValidator *schemaValidator;
    if (ZEND_NUM_ARGS()>0) {
        WRONG_PARAM_COUNT;
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        bool result = schemaValidator->exceptionOccurred();
        RETURN_BOOL(result);
    }
    RETURN_BOOL(false);
}


PHP_METHOD(SchemaValidator, getErrorCode)
{
    SchemaValidator *schemaValidator;
    long index;
    if (zend_parse_parameters(ZEND_NUM_ARGS() , "l", &index) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        const char * errCode = schemaValidator->getErrorCode((int)index);
        if(errCode != nullptr) {
            //char *str = estrdup(errCode);
            _RETURN_STRING(errCode);
        }
    }
    RETURN_NULL();
}

PHP_METHOD(SchemaValidator, getErrorMessage)
{
    SchemaValidator *schemaValidator;
    long index;
    if (zend_parse_parameters(ZEND_NUM_ARGS() , "l", &index) == FAILURE) {
        RETURN_NULL();
    }
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        const char * errStr = schemaValidator->getErrorMessage(index);
        if(errStr != nullptr) {
            //char *str = estrdup(errStr);
            _RETURN_STRING(errStr);
        }
    }
    RETURN_NULL();
}
PHP_METHOD(SchemaValidator, exceptionClear)
{
    SchemaValidator * schemaValidator;
    zend_object* pobj = Z_OBJ_P(getThis());
   schemaValidator_object * obj = (schemaValidator_object *)((char *)pobj - XtOffsetOf(schemaValidator_object, std));
    schemaValidator = obj->schemaValidator;
    if (schemaValidator != nullptr) {
        schemaValidator->exceptionClear();
    }
}


zend_function_entry SchemaValidator_methods[] = {
    PHP_ME(SchemaValidator,  __destruct,     nullptr, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    PHP_ME(SchemaValidator,  setSourceNode,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  setOutputFile,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  validate,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  validateToNode,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  registerSchemaFromFile,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  registerSchemaFromString,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  getValidationReport,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  setLax,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  setParameter,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  setProperty,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  clearParameters,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  clearProperties,      nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  exceptionOccurred, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  exceptionClear, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  getErrorCode, nullptr, ZEND_ACC_PUBLIC)
    PHP_ME(SchemaValidator,  getErrorMessage, nullptr, ZEND_ACC_PUBLIC)
//    PHP_ME(SchemaValidator,  getExceptionCount, nullptr, ZEND_ACC_PUBLIC)
{nullptr, nullptr, nullptr}
};