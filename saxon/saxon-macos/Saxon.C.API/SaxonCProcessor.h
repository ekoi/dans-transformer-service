#ifndef SAXONCPROC_H 
#define SAXONCPROC_H


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Saxonica Limited.
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
// This Source Code Form is "Incompatible With Secondary Licenses", as defined by the Mozilla Public License, v. 2.0.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "SaxonCGlue.h"





/*! <code>sxnc_processor</code>. This struct is used to capture the Java processor objects created in C for XSLT, XQuery and XPath 
 * <p/>
 */
typedef struct {
	jobject xqueryProc;
	jobject xsltProc;
	jobject xpathProc;
} sxnc_processor;

EXTERN_SAXONC
/*
 * Get the Saxon version 
 */
const char * version(sxnc_environment *environi);

const char * getProductVariantAndVersion(sxnc_environment * environi);

void initSaxonc(sxnc_environment ** environi, sxnc_processor ** proc, sxnc_parameter **param, sxnc_property ** prop,int cap, int propCap);

void freeSaxonc(sxnc_environment ** environi, sxnc_processor ** proc, sxnc_parameter **param, sxnc_property ** prop);

void xsltSaveResultToFile(sxnc_environment *environi, sxnc_processor ** proc, char * cwd, char * source, char* stylesheet, char* outputfile, sxnc_parameter *parameters, sxnc_property * properties, int parLen, int propLen);

const char * xsltApplyStylesheet(sxnc_environment *environi, sxnc_processor ** proc, char * cwd, const char * source, const char* stylesheet, sxnc_parameter *parameters, sxnc_property * properties, int parLen, int propLen);

void executeQueryToFile(sxnc_environment *environi, sxnc_processor ** proc, char * cwd, char* outputfile, sxnc_parameter *parameters, sxnc_property * properties, int parLen, int propLen);

const char * executeQueryToString(sxnc_environment *environi, sxnc_processor ** proc, char * cwd, sxnc_parameter *parameters, sxnc_property * properties, int parLen, int propLen);

EXTERN_SAXONC_END

#endif 
