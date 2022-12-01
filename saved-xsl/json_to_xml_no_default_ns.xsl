<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:math="http://www.w3.org/2005/xpath-functions/math"
    xmlns:fn="http://www.w3.org/2005/xpath-functions"
    xmlns:xs="http://www.w3.org/2001/XMLSchema" exclude-result-prefixes="xs math fn" version="3.0">
    <xsl:output indent="yes" omit-xml-declaration="no"/>
    
    <xsl:template match="data">
        <!-- apply the xml structure generated from JSON -->
        <xsl:apply-templates select="json-to-xml(.)" />
    </xsl:template>
    
    <xsl:template match="@*|processing-instruction()|comment()">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
    </xsl:template>
    <xsl:template match="*">
        <xsl:element name="{local-name()}">
            <xsl:apply-templates select="@*|node()"/>
        </xsl:element>
    </xsl:template>
    
</xsl:stylesheet> 