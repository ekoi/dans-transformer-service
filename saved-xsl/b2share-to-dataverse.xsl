<?xml version="1.0"?>
<xsl:stylesheet
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:math="http://www.w3.org/2005/xpath-functions/math"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    exclude-result-prefixes="xs math" version="3.0">
    <xsl:output indent="yes" omit-xml-declaration="yes" />

    <xsl:template match="data">
        <!-- create a new root tag -->

            <!-- apply the xml structure generated from JSON -->
            <xsl:apply-templates select="json-to-xml(.)" />

    </xsl:template>

    <!-- template for the first tag -->
    <xsl:template match="/map"
        xpath-default-namespace="http://www.w3.org/2005/xpath-functions">

        {
        "datasetVersion": {
        "metadataBlocks": {
        "citation": {
        "fields": [
        {
        "value": "<xsl:value-of select="//array[@key='titles']/map/string[@key='title']/."/>",
        "typeClass": "primitive",
        "multiple": false,
        "typeName": "title"
        },
        {
        "value": [
        {
        "authorName": {
        "value": "<xsl:value-of select="//array[@key='creators']/map/string[@key='creator_name']"/>",
        "typeClass": "primitive",
        "multiple": false,
        "typeName": "authorName"
        },
        "authorAffiliation": {
        "value": "Ekosoft Inc.",
        "typeClass": "primitive",
        "multiple": false,
        "typeName": "authorAffiliation"
        }
        }
        ],
        "typeClass": "compound",
        "multiple": true,
        "typeName": "author"
        },
        {
        "value": [
        { "datasetContactEmail" : {
        "typeClass": "primitive",
        "multiple": false,
        "typeName": "datasetContactEmail",
        "value" : "<xsl:value-of select="//map[@key='metadata']/string[@key='contact_email']"/>"
        },
        "datasetContactName" : {
        "typeClass": "primitive",
        "multiple": false,
        "typeName": "datasetContactName",
        "value": "Indarto, Eko"
        }
        }],
        "typeClass": "compound",
        "multiple": true,
        "typeName": "datasetContact"
        },
        {
        "value": [ {
        "dsDescriptionValue":{
        "value":   "<xsl:value-of select="/map/map[@key='metadata']/array[@key='descriptions']/map/string[@key='description']"/>,
        "multiple":false,
        "typeClass": "primitive",
        "typeName": "dsDescriptionValue"
        }}],
        "typeClass": "compound",
        "multiple": true,
        "typeName": "dsDescription"
        },
        {
        "value": [
        "Medicine, Health and Life Sciences"
        ],
        "typeClass": "controlledVocabulary",
        "multiple": true,
        "typeName": "subject"
        }
        ],
        "displayName": "Citation Metadata"
        }
        }
        }
        }

    </xsl:template>

    <!-- template to output a number value -->


</xsl:stylesheet>