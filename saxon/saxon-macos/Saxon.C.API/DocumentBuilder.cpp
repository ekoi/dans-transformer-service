// DocumentBuilder.cpp : Defines the exported functions for the DLL application.
//

#include "DocumentBuilder.h"
#include "XdmNode.h"


    DocumentBuilder::DocumentBuilder(){
        SaxonProcessor::attachCurrentThread();
	    SaxonProcessor *p = new SaxonProcessor(false);
	    procClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/s9api/Processor");
        static jmethodID xdocBuilderID = nullptr;
        if (xdocBuilderID == nullptr) {
         			xdocBuilderID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(procClass,
         					"newDocumentBuilder",
         					"()Lnet/sf/saxon/s9api/DocumentBuilder;");

        jobject docBuilderObject = (jobject)(
             				SaxonProcessor::sxn_environ->env->CallObjectMethod(p->proc, xdocBuilderID));
        DocumentBuilder(p, docBuilderObject, "");
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(docBuilderObject);
    }

    }


    DocumentBuilder::DocumentBuilder(SaxonProcessor* p, jobject docObject, std::string cwd) {
        SaxonProcessor::attachCurrentThread();
           /*
             * Look for class.
             */
            docBuilderClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                    "net/sf/saxon/s9api/DocumentBuilder");
            saxonCAPIClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/SaxonCAPI");


           docBuilderObject = SaxonProcessor::sxn_environ->env->NewGlobalRef(docObject);
           lineNumbering = false;
           dtdVal = false;
           schemaValidator = nullptr;
           cwdDB = cwd;
           proc = p;
           exception = nullptr;
           setBaseUri(cwd.c_str());

    }


    DocumentBuilder::DocumentBuilder(const DocumentBuilder &other) {
        SaxonProcessor::attachCurrentThread();
        docBuilderClass = lookForClass(SaxonProcessor::sxn_environ->env,
                                    "net/sf/saxon/s9api/DocumentBuilder");


        saxonCAPIClass = lookForClass(SaxonProcessor::sxn_environ->env, "net/sf/saxon/option/cpp/SaxonCAPI");
        docBuilderObject = SaxonProcessor::sxn_environ->env->NewGlobalRef(other.docBuilderObject);
        lineNumbering = other.lineNumbering;
        schemaValidator = other.schemaValidator;
        dtdVal = other.dtdVal;
        cwdDB = other.cwdDB;
        proc = other.proc;
    }

    DocumentBuilder::~DocumentBuilder(){
        SaxonProcessor::attachCurrentThread();
        if(docBuilderObject) {
            SaxonProcessor::sxn_environ->env->DeleteGlobalRef(docBuilderObject);
        }
        cwdDB.erase();
        exceptionClear();
        //delete contextItem;
    }

    void DocumentBuilder::setLineNumbering(bool option){
        SaxonProcessor::attachCurrentThread();
        static jmethodID db_lineID = nullptr;

			db_lineID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(docBuilderClass,
					"setLineNumbering",
					"(Z)");
        lineNumbering = option;
        SaxonProcessor::sxn_environ->env->CallVoidMethod(docBuilderObject, db_lineID, option);


    }


    bool DocumentBuilder::isLineNumbering(){
        return lineNumbering;


    }



    void DocumentBuilder::setSchemaValidator(SchemaValidator * validator){
        SaxonProcessor::attachCurrentThread();
        static jmethodID db_schemaID = nullptr;
	    if (!db_schemaID || validator == nullptr) {

	        return;
	    }
	    schemaValidator = validator;
		db_schemaID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(docBuilderClass,
					"setSchemaValidator",
					"(Lnet/sf/saxon/s9api/SchemaValidator;)");
        SaxonProcessor::sxn_environ->env->CallVoidMethod(docBuilderObject, db_schemaID, validator->getUnderlyingValidator());


    }


    SchemaValidator * DocumentBuilder::getSchemaValidator() {
        return schemaValidator;
    }



    void DocumentBuilder::setDTDValidation(bool option) {
        SaxonProcessor::attachCurrentThread();
        static jmethodID db_dtdID = nullptr;
	    if (!db_dtdID ) {

	        return;
	    }
		db_dtdID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(docBuilderClass,
					"setDTDValidation",
					"(Z)");
        dtdVal= option;
        SaxonProcessor::sxn_environ->env->CallVoidMethod(docBuilderObject, db_dtdID, dtdVal);


    }



    bool DocumentBuilder::isDTDValidation() {
        return dtdVal;
    }




    void DocumentBuilder::setBaseUri(const char* uri) {
        SaxonProcessor::attachCurrentThread();
        static jmethodID db_dtdID = nullptr;
	    if (!db_dtdID ) {

	        return;
	    }
		db_dtdID = (jmethodID) SaxonProcessor::sxn_environ->env->GetMethodID(docBuilderClass,
					"setBaseURI",
					"(Z)");
        baseURI= std::string(uri);
        SaxonProcessor::sxn_environ->env->CallVoidMethod(docBuilderObject, db_dtdID, dtdVal);

    }



    const char * DocumentBuilder::getBaseUri(){ return baseURI.c_str();}




    XdmNode* DocumentBuilder::parseXmlFromString(const char * source){
        SaxonProcessor::attachCurrentThread();
    jmethodID mdpID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(saxonCAPIClass, "parseXmlString",
                                                                                    "(Ljava/lang/String;Lnet/sf/saxon/s9api/Processor;Lnet/sf/saxon/s9api/DocumentBuilder;Lnet/sf/saxon/s9api/SchemaValidator;Ljava/lang/String;)Lnet/sf/saxon/s9api/XdmNode;");
    if (!mdpID) {
        std::cerr << "\nError: Saxonc." << "parseXmlString()" << " not found" << std::endl;
        return nullptr;
    }
//TODO SchemaValidator
    if(source == nullptr) {
        createException("Source string is NULL");
        return nullptr;
    }
    jobject xdmNodei = SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(saxonCAPIClass, mdpID,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                        cwdDB.c_str()), proc->proc, docBuilderObject, (schemaValidator != nullptr ? schemaValidator->getUnderlyingValidator(): nullptr),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                        source));
    if (xdmNodei) {
        XdmNode *value = nullptr;
        value = new XdmNode(xdmNodei);
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(xdmNodei);
        return value;
    } else if (exceptionOccurred()) {
        createException();
    }

    return nullptr;


    }



   XdmNode* DocumentBuilder::parseXmlFromFile(const char * filename){
       SaxonProcessor::attachCurrentThread();
    jmethodID mID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(saxonCAPIClass, "parseXmlFile",
                                                                                    "(Lnet/sf/saxon/s9api/Processor;Ljava/lang/String;Lnet/sf/saxon/s9api/DocumentBuilder;Lnet/sf/saxon/s9api/SchemaValidator;Ljava/lang/String;)Lnet/sf/saxon/s9api/XdmNode;");
    if (!mID) {
        std::cerr << "\nError: Saxonc.Dll " << "parseXmlFromFile()" << " not found" << std::endl;
        return nullptr;
    }
    jobject xdmNodei = SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(saxonCAPIClass, mID, proc,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                        ""),  docBuilderObject, (schemaValidator != nullptr ? schemaValidator->getUnderlyingValidator(): nullptr),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                        filename));
    if (xdmNodei) {
        XdmNode *value = nullptr;
        value = new XdmNode(xdmNodei);
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(xdmNodei);
        return value;
    } else if (exceptionOccurred()) {
        createException();
    }
    return nullptr;
  }


   XdmNode* DocumentBuilder::parseXmlFromUri(const char * uri){
       SaxonProcessor::attachCurrentThread();
    jmethodID mID = (jmethodID) SaxonProcessor::sxn_environ->env->GetStaticMethodID(saxonCAPIClass, "parseXmlFile",
                                                                                    "(Lnet/sf/saxon/s9api/Processor;Ljava/lang/String;Lnet/sf/saxon/s9api/DocumentBuilder;Lnet/sf/saxon/s9api/SchemaValidator;Ljava/lang/String;)Lnet/sf/saxon/s9api/XdmNode;");
    if (!mID) {
        std::cerr << "\nError: Saxonc.Dll " << "parseXmlFromFile()" << " not found" << std::endl;
        return nullptr;
    }
    jobject xdmNodei = SaxonProcessor::sxn_environ->env->CallStaticObjectMethod(saxonCAPIClass, mID, proc,
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                        ""),  docBuilderObject, (schemaValidator != nullptr ? schemaValidator->getUnderlyingValidator(): nullptr),
                                                                                SaxonProcessor::sxn_environ->env->NewStringUTF(
                                                                                        uri));
    if (xdmNodei) {
        XdmNode *value = nullptr;
        value = new XdmNode(xdmNodei);
        SaxonProcessor::sxn_environ->env->DeleteLocalRef(xdmNodei);
        return value;
    } else if (exceptionOccurred()) {
        createException();
    }


        return nullptr;
   }




void DocumentBuilder::exceptionClear(){
    SaxonProcessor::attachCurrentThread();
    SaxonProcessor::sxn_environ->env->ExceptionClear();
    if(exception != nullptr) {
        delete exception;
        exception = nullptr;

    }

}

const char * DocumentBuilder::getErrorCode() {
	if(exception == nullptr) {return nullptr;}
	return exception->getErrorCode();
}

const char * DocumentBuilder::getErrorMessage(){
	if(exception == nullptr) {return nullptr;}
	return exception->getMessage();
}

    SaxonApiException * DocumentBuilder::getException() {
        return exception;

}

bool DocumentBuilder::exceptionOccurred() {
    return proc->exceptionOccurred() || exception != nullptr;
}



void DocumentBuilder::createException(const char * message) {
    if(exception != nullptr) {
        delete exception;
        exception = nullptr;
    }
    if(message == nullptr) {
        exception = proc->checkAndCreateException(docBuilderClass);
    } else {
        exception = new SaxonApiException(message);
    }

}
