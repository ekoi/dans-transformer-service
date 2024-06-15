<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:math="http://www.w3.org/2005/xpath-functions/math"
    xmlns:xs="http://www.w3.org/2001/XMLSchema" exclude-result-prefixes="xs math" version="3.0">
    <xsl:output indent="yes" omit-xml-declaration="yes"/>

    <xsl:template match="data">
        <!-- create a new root tag -->

        <!-- apply the xml structure generated from JSON -->
        <xsl:apply-templates select="json-to-xml(.)"/>

    </xsl:template>

    <!-- template for the first tag -->
    <xsl:template match="/map" xpath-default-namespace="http://www.w3.org/2005/xpath-functions"> 
       
       {
       "metadata": {
       "title": "<xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='title']/following-sibling::string[@key='value']/."/>",
       "description": "<xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='description']/following-sibling::string[@key='value']/."/>",
       "creators": [
       {
       "name": "<xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='administrative']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='depositor']/following-sibling::string[@key='value']"/>", 
          "affiliation": "DANS"
        }
       ]
       }
       }
       
    </xsl:template>

    <!-- template to output a number value -->


</xsl:stylesheet>
