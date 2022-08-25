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
    <xsl:template match="map"
        xpath-default-namespace="http://www.w3.org/2005/xpath-functions">
        <xsl:variable name="doi-preffix" select="10.80227"/>
        <xsl:variable name="other-id" select="//map[@key='citation']/array[@key='fields']/map/array[@key='value']/map/map[@key='otherIdValue']/string[@key='typeName' and text()='otherIdValue']//following-sibling::string[@key='value']/."/>
        <xsl:variable name="title" select="//map[@key='citation']/array[@key='fields']/map/string[@key='typeName' and text()='title']//following-sibling::string[@key='value']/."/>
        <xsl:variable name="currentDate" select="current-date()"/>
        {
        "identifiers": [{
        "identifierType": "DOI",
        "identifier": "<xsl:value-of select="concat($doi-preffix,'/', $other-id)"/>"
        }],
        "creators": [
        <xsl:for-each select="//map[@key='citation']/array[@key='fields']/map/array[@key='value']/map/map[@key='authorName']">
            {"name":"<xsl:value-of select="./string[@key='typeName' and text()='authorName']//following-sibling::string[@key='value']/."/>"}
            <xsl:if test="position() != last()">
                <xsl:text>,</xsl:text>
            </xsl:if>
        </xsl:for-each>
         ],
        "titles": [{"title": "<xsl:value-of select="$title"/>"}],
        "publisher": "ODISSEI",
        "publicationYear": "<xsl:value-of select="year-from-date($currentDate)"/>",
        "types": {
        "resourceType": "Dataset",
        "resourceTypeGeneral": "Dataset"
        },
        "schemaVersion": "http://datacite.org/schema/kernel-4"
        }
    </xsl:template>
</xsl:stylesheet>