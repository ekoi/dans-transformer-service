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
  "datasetVersion": {
<!--    "distributionDate": "<xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='administrative']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='date_available']/following-sibling::string[@key='value']"/>",-->
    "productionDate": "<xsl:value-of select="format-dateTime(current-dateTime(), '[Y0001]-[M01]-[D01]')"/>",
      "license": {
      "name": "CC0 1.0",
      "uri": "http://creativecommons.org/publicdomain/zero/1.0"
      },
    "termsOfAccess": "",
    "metadataBlocks": {
      "citation": {
        "displayName": "Citation Metadata",
        "fields": [<!-- CIT01 -->
          {
            "typeName": "title",
            "multiple": false,
            "typeClass": "primitive",
            "value": "<xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='title']/following-sibling::string[@key='value']/."/>"
          },
          {<!-- CIT02-->
            "typeName": "subtitle",
            "multiple": false,
            "typeClass": "primitive",
            "value": "<xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='subtitle']/following-sibling::array[@key='fields']/map/string[@key='value']"/>"
          },
          {
            "typeName": "author",
            "multiple": true,
            "typeClass": "compound",
            "value": [
                <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='author']/following-sibling::array[@key='fields']/array/map/string[@key='name' and text()='name']">
                  {
                    "authorName": {
                          "typeName": "authorName",
                          "multiple": false,
                          "typeClass": "primitive",
                          "value": "<xsl:value-of select="following-sibling::map[@key='value']/string[@key='label']"/>"
                      },
                      "authorAffiliation": {<!-- CITr06 -->
                          "typeName": "authorAffiliation",
                          "multiple": false,
                          "typeClass": "primitive",
                          "value": "<xsl:value-of select="../../map/string[@key='name' and text()='affiliation']/following-sibling::string[@key='value']"/>"
                       }
                  <xsl:if test="following-sibling::map[@key='value']/string[@key='idLabel']='ORCID ID'">
                      ,
                      "authorIdentifierScheme": {
                          "typeName": "authorIdentifierScheme",
                          "multiple": false,
                          "typeClass": "controlledVocabulary",
                          "value": "ORCID"
                      },
                      "authorIdentifier": {
                          "typeName": "authorIdentifier",
                          "multiple": false,
                          "typeClass": "primitive",
                          "value": "<xsl:value-of select="following-sibling::map[@key='value']/string[@key='id']"/>"
                          }
                  </xsl:if>
                  }
                  <xsl:if test="position() != last()">
                    <xsl:text>,</xsl:text>
                  </xsl:if>
                </xsl:for-each>
              ]
          },
          {
          "typeName": "datasetContact",
          "multiple": true,
          "typeClass": "compound",
          "value": [
          {
          
          "datasetContactEmail": {
          "typeName": "datasetContactEmail",
          "multiple": false,
          "typeClass": "primitive",
          "value": "e.indarto@gmail.com"
          }
          }
          ]
          },
        
          {<!-- CIT03 -->
            "typeName": "dsDescription",
            "multiple": true,
            "typeClass": "compound",
            "value": [
              {
                "dsDescriptionValue": {
                  "typeName": "dsDescriptionValue",
                  "multiple": false,
                  "typeClass": "primitive",
                  "value": "<xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='description']/following-sibling::string[@key='value']/."/>"
                }
              }
            ]
          },
          {
            "typeName": "subject",
            "multiple": true,
            "typeClass": "controlledVocabulary",
            "value": [
      <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='relations']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='audience']/following-sibling::map[@key='value']">
        <xsl:variable name="audience">
          <xsl:call-template name="audiencefromkeyword">
            <xsl:with-param name="val" select="./string[@key='id']"/>
          </xsl:call-template>
        </xsl:variable>
        "<xsl:value-of select="$audience"/>"
        <xsl:if test="position() != last()">
          <xsl:text>,</xsl:text>
        </xsl:if>
      </xsl:for-each>
            ]
          },
          {
            "typeName": "keyword",
            "multiple": true,
            "typeClass": "compound",
            "value": [
            <!-- COVx01 -->
                <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='coverage']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='subject_keywords']/following-sibling::array[@key='value']/map">
                  {
                  <xsl:choose>
                    <xsl:when test="./boolean[@key='freetext']">
                      
                          "keywordValue": {
                          "typeName": "keywordValue",
                          "multiple": false,
                          "typeClass": "primitive",
                          "value": "<xsl:value-of select="./string[@key='value']"/>"
                      }
                      
                    </xsl:when>
                    <xsl:otherwise>
                      
                         "keywordValue": {
                         "typeName": "keywordValue",
                         "multiple": false,
                         "typeClass": "primitive",
                         "value": "<xsl:value-of select="./string[@key='label']/."/>"
                      },
                      "keywordVocabulary": {
                          "typeName": "keywordVocabulary",
                          "multiple": false,
                          "typeClass": "primitive",
                          "value": "Art and Architecture Thesaurus"
                      },
                      "keywordVocabularyURI": {
                          "typeName": "keywordVocabularyURI",
                          "multiple": false,
                          "typeClass": "primitive",
                          "value": "<xsl:value-of select="./string[@key='value']/."/>"
                      }
                        
                    </xsl:otherwise>
                  </xsl:choose>
                 }
                  <xsl:if test="position() != last()">
                    <xsl:text>,</xsl:text>
                  </xsl:if>
                </xsl:for-each>
            ]
          },
          {
            "typeName": "language",
            "multiple": true,
            "typeClass": "controlledVocabulary",
            "value": ["English"]
      },
          {
            "typeName": "productionDate",
            "multiple": false,
            "typeClass": "primitive",
            "value": "<xsl:value-of select="format-dateTime(current-dateTime(), '[Y0001]-[M01]-[D01]')"/>"
          },
          {
            "typeName": "dateOfDeposit",
            "multiple": false,
            "typeClass": "primitive",
            "value": "<xsl:value-of  select="format-dateTime(current-dateTime(), '[Y0001]-[M01]-[D01]')"/>"
          }
          
        ]
      }
    }
  }
}

        
       
    </xsl:template>
  <!-- Mapping from the Dataverse keywords to the Narcis Discipline types (https://easy.dans.knaw.nl/schemas/vocab/2015/narcis-type.xsd) -->
  <xsl:template name="audiencefromkeyword">
    <xsl:param name="val"/>
    <xsl:message><xsl:value-of select="$val"/></xsl:message>
    <!-- make our own map, it's small -->
    <xsl:choose>
      <xsl:when test="starts-with($val, 'D11')">
        <xsl:value-of select="'Mathematical Sciences'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D12')">
        <xsl:value-of select="'Physics'"/>
      </xsl:when> 
      <xsl:when test="starts-with($val, 'D13')">
        <xsl:value-of select="'Chemistry'"/>
      </xsl:when>
      <xsl:when test="starts-with($val,'D14')">
        <xsl:value-of select="'Engineering'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D15')">
        <xsl:value-of select="'Earth and Environmental Sciences'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D16')">
        <xsl:value-of select="'Computer and Information Science'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D17')">
        <xsl:value-of select="'Astronomy and Astrophysics'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D18')">
        <xsl:value-of select="'Agricultural Sciences'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D2')">
        <xsl:value-of select="'Medicine, Health and Life Sciences'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D3')">
        <xsl:value-of select="'Arts and Humanities'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D40')">
        <xsl:value-of select="'Law'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D41')">
        <xsl:value-of select="'Law'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D5')">
        <xsl:value-of select="'Social Sciences'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D6')">
        <xsl:value-of select="'Social Sciences'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'D42')">
        <xsl:value-of select="'Social Sciences'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'E13')">
        <xsl:value-of select="'Social Sciences'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'E14')">
        <xsl:value-of select="'Social Sciences'"/>
      </xsl:when>
      <xsl:when test="starts-with($val, 'E15')">
        <xsl:value-of select="'Earth and Environmental Sciences'"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="'Other'"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>