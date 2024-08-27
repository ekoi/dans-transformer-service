<?xml version="1.0"?>
<xsl:stylesheet xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:dcterms="http://purl.org/dc/terms/"
    xmlns:ddm="http://schemas.dans.knaw.nl/dataset/ddm-v2/"
    xmlns:dcx-dai="http://easy.dans.knaw.nl/schemas/dcx/dai/"
    xmlns:dcx-gml="http://easy.dans.knaw.nl/schemas/dcx/gml/"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:math="http://www.w3.org/2005/xpath-functions/math"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:id-type="http://easy.dans.knaw.nl/schemas/vocab/identifier-type/"
    exclude-result-prefixes="xs math" version="3.0">
    <xsl:output indent="yes" omit-xml-declaration="yes"/>
    <xsl:template match="data">
        <xsl:apply-templates select="json-to-xml(.)"/>
    </xsl:template>
    <xsl:template match="map" xpath-default-namespace="http://www.w3.org/2005/xpath-functions">
        <xsl:text>id=</xsl:text>
        <xsl:value-of select="/map/string[@key = 'id']"/>
        <xsl:for-each select="//array[@key = 'metadata']/map">
            <xsl:variable name="Section" select="string[@key = 'id']"/>
            <xsl:for-each select="array[@key = 'fields']/map">
                <xsl:choose>
                    <xsl:when test="string[@key = 'name' and text() = 'interviewee']">
                        <xsl:variable name="hasPrivateFalse" select="array[@key = 'fields']/map[boolean[@key = 'private' and . = 'false']]"/>
                        <xsl:if test="$hasPrivateFalse">
                            <xsl:text>"</xsl:text>
                            <xsl:value-of select="string[@key = 'name']"/>
                            <xsl:text>": { </xsl:text>
                            <xsl:for-each select="array[@key = 'fields']/map[boolean[@key = 'private' and . = 'false']]">
                                <xsl:text>&#10;</xsl:text>
                                <xsl:value-of select="$Section"/>
                                <xsl:text>.</xsl:text>
                                <xsl:value-of select="string[@key = 'name']"/>
                                <xsl:text>=</xsl:text>
                                <xsl:value-of select="string[@key = 'value']"/>
                            </xsl:for-each>
                        </xsl:if>
                    </xsl:when>
                    <xsl:when
                        test="string[@key = 'name' and not(text() = 'interviewee' or text() = 'interviewer' or text() = 'interpreter' or text() = 'others' or text() = 'transcript_human')]">
                        <xsl:choose>
                            <xsl:when test="self::map and map[@key = 'value']">
                                <xsl:text>&#10;</xsl:text>
                                <xsl:value-of select="$Section"/>
                                <xsl:text>.</xsl:text>
                                <xsl:value-of select="string[@key = 'name']"/>
                                <xsl:text>.label=</xsl:text>
                                <xsl:value-of select="map[@key = 'value']/string[@key = 'label']"/>
                                <xsl:text>&#10;</xsl:text>
                                <xsl:value-of select="$Section"/>
                                <xsl:text>.</xsl:text>
                                <xsl:value-of select="string[@key = 'name']"/>
                                <xsl:text>.value=</xsl:text>
                                <xsl:value-of select="map[@key = 'value']/string[@key = 'value']"/>
                            </xsl:when>
                            <xsl:when test="string[@key = 'value']">
                                <xsl:text>&#10;</xsl:text>
                                <xsl:value-of select="$Section"/>
                                <xsl:text>.</xsl:text>
                                <xsl:value-of select="string[@key = 'name']"/>
                                <xsl:text>=</xsl:text>
                                <xsl:value-of select="string[@key = 'value']"/>
                            </xsl:when>
                            <xsl:when test="array[@key = 'value']">
                                <xsl:variable name="ItemName" select="string[@key = 'name']"/>
                                <xsl:for-each select="array[@key = 'value']/map">
                                    <xsl:text>&#10;</xsl:text>
                                    <xsl:value-of select="$Section"/>
                                    <xsl:text>.</xsl:text>
                                    <xsl:value-of select="$ItemName"/>
                                    <xsl:text>.label=</xsl:text>
                                    <xsl:value-of select="string[@key = 'label']"/>
                                    <xsl:text>&#10;</xsl:text>
                                    <xsl:value-of select="$Section"/>
                                    <xsl:text>.</xsl:text>
                                    <xsl:value-of select="$ItemName"/>
                                    <xsl:text>.value=</xsl:text>
                                    <xsl:value-of select="string[@key = 'value']"/>
                                </xsl:for-each>
                            </xsl:when>
                            <xsl:when
                                test="string[@key = 'type' and text() = 'group'] and boolean[@key = 'repeatable' and text() = 'true']">
                                <xsl:variable name="GroupName" select="string[@key = 'name']"/>
                                <xsl:for-each select="array[@key = 'fields']/array">
                                    <xsl:for-each select="map">
                                        <xsl:variable name="GroupItemName"
                                            select="string[@key = 'name']"/>
                                        <xsl:choose>
                                            <xsl:when test="map[@key = 'value']">
                                                <xsl:for-each select="map[@key = 'value']/string">
                                                  <xsl:text>&#10;</xsl:text>
                                                  <xsl:value-of select="$Section"/>
                                                  <xsl:text>.</xsl:text>
                                                  <xsl:value-of select="$GroupName"/>
                                                  <xsl:text>.</xsl:text>
                                                  <xsl:value-of select="$GroupItemName"/>
                                                  <xsl:text>.</xsl:text>
                                                  <xsl:value-of select="@key"/>
                                                  <xsl:text>=</xsl:text>
                                                  <xsl:value-of select="."/>
                                                </xsl:for-each>
                                            </xsl:when>
                                            <xsl:when test="string[@key = 'value']">
                                                <xsl:text>&#10;</xsl:text>
                                                <xsl:value-of select="$Section"/>
                                                <xsl:text>.</xsl:text>
                                                <xsl:value-of select="$GroupName"/>
                                                <xsl:text>.</xsl:text>
                                                <xsl:value-of select="$GroupItemName"/>
                                                <xsl:text>=</xsl:text>
                                                <xsl:value-of select="string[@key = 'value']"/>
                                            </xsl:when>
                                            <xsl:when
                                                test="array[@key = 'fields'] and string[@key = 'type' and text() = 'repeatSingleField']">
                                                <xsl:variable name="SubReapeatFieldName"
                                                  select="string[@key = 'name']"/>
                                                <xsl:for-each select="array[@key = 'fields']/map">
                                                  <xsl:text>&#10;</xsl:text>
                                                  <xsl:value-of select="$Section"/>
                                                  <xsl:text>.</xsl:text>
                                                  <xsl:value-of select="$SubReapeatFieldName"/>
                                                  <xsl:text>=</xsl:text>
                                                  <xsl:value-of select="string[@key = 'value']"/>
                                                </xsl:for-each>
                                            </xsl:when>
                                            <xsl:when test="array[@key = 'fields']">
                                                <xsl:variable name="GroupName2"
                                                  select="string[@key = 'name']"/>
                                                <xsl:for-each select="array[@key = 'fields']/map">
                                                  <xsl:text>&#10;</xsl:text>
                                                  <xsl:value-of select="$Section"/>
                                                  <xsl:text>.</xsl:text>
                                                  <xsl:value-of select="$GroupName2"/>
                                                  <xsl:text>.</xsl:text>
                                                  <xsl:value-of select="string[@key = 'name']"/>
                                                  <xsl:text>=</xsl:text>
                                                  <xsl:value-of select="string[@key = 'value']"/>
                                                </xsl:for-each>
                                            </xsl:when>
                                            <xsl:when test="array[@key = 'value']/string">
                                                <xsl:text>&#10;</xsl:text>
                                                <xsl:value-of select="$Section"/>
                                                <xsl:text>.</xsl:text>
                                                <xsl:value-of select="string[@key = 'name']"/>
                                                <xsl:text>=</xsl:text>
                                                <xsl:value-of select="array[@key = 'value']/string"
                                                />
                                            </xsl:when>
                                        </xsl:choose>
                                    </xsl:for-each>
                                </xsl:for-each>
                            </xsl:when>
                            <xsl:when test="string[@key = 'name' and text() = 'contact']">
                                <xsl:variable name="Contact" select="string[@key = 'name']"/>
                                <xsl:for-each select="array[@key = 'fields']/map">
                                    <xsl:text>&#10;</xsl:text>
                                    <xsl:value-of select="$Section"/>
                                    <xsl:text>.</xsl:text>
                                    <xsl:value-of select="$Contact"/>
                                    <xsl:text>.</xsl:text>
                                    <xsl:value-of select="string[@key = 'name']"/>
                                    <xsl:text>=</xsl:text>
                                    <xsl:value-of select="string[@key = 'value']"/>
                                </xsl:for-each>
                            </xsl:when>
                            <xsl:when
                                test="array[@key = 'fields'] and string[text() = 'repeatSingleField']">
                                <xsl:variable name="RepeatableSingleField"
                                    select="string[@key = 'name']"/>
                                <xsl:for-each select="array[@key = 'fields']/map">
                                    <xsl:text>&#10;</xsl:text>
                                    <xsl:value-of select="$Section"/>
                                    <xsl:text>.</xsl:text>
                                    <xsl:value-of select="$RepeatableSingleField"/>
                                    <xsl:text>=</xsl:text>
                                    <xsl:value-of select="string[@key = 'value']"/>
                                </xsl:for-each>
                            </xsl:when>
                            <xsl:when test="array[@key = 'fields']">
                                <xsl:variable name="GroupName3" select="string[@key = 'name']"/>
                                <xsl:for-each select="array[@key = 'fields']/map">
                                    <xsl:variable name="GroupSubName3"
                                        select="string[@key = 'name']"/>
                                    <xsl:choose>
                                        <xsl:when test="string[@key = 'value']">
                                            <xsl:text>&#10;</xsl:text>
                                            <xsl:value-of select="$Section"/>
                                            <xsl:text>.</xsl:text>
                                            <xsl:value-of select="$GroupName3"/>
                                            <xsl:text>.</xsl:text>
                                            <xsl:value-of select="$GroupSubName3"/>
                                            <xsl:text>=</xsl:text>
                                            <xsl:value-of select="string[@key = 'value']"/>
                                        </xsl:when>
                                        <xsl:when test="array[@key = 'fields']">
                                            <xsl:for-each select="array[@key = 'fields']/map">
                                                <xsl:text>&#10;</xsl:text>
                                                <xsl:value-of select="$Section"/>
                                                <xsl:text>.</xsl:text>
                                                <xsl:value-of select="$GroupName3"/>
                                                <xsl:text>.</xsl:text>
                                                <xsl:value-of select="$GroupSubName3"/>
                                                <xsl:text>=</xsl:text>
                                                <xsl:value-of select="string[@key = 'value']"/>
                                            </xsl:for-each>
                                        </xsl:when>
                                        <xsl:when test="array[@key = 'value']">
                                            <xsl:text>&#10;</xsl:text>
                                            <xsl:value-of select="$Section"/>
                                            <xsl:text>.</xsl:text>
                                            <xsl:value-of select="$GroupName3"/>
                                            <xsl:text>.</xsl:text>
                                            <xsl:value-of select="$GroupSubName3"/>
                                            <xsl:text>=</xsl:text>
                                            <xsl:value-of select="array[@key = 'value']/string"/>
                                        </xsl:when>
                                    </xsl:choose>
                                </xsl:for-each>
                            </xsl:when>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:text/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:for-each>
        </xsl:for-each>
        <xsl:variable name="FileMetadata">file-metadata</xsl:variable>
        <xsl:for-each select="//array[@key = 'file-metadata']/map">
            <xsl:variable name="isExcluded"
                select="boolean(boolean[@key = 'private' and text() = 'true'])"/>
            <xsl:choose>
                <xsl:when test="$isExcluded">
                    <xsl:text/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>&#10;</xsl:text>
                    <xsl:value-of select="$FileMetadata"/>
                    <xsl:text>.name=</xsl:text>
                    <xsl:value-of select="string[@key = 'name']"/>
                    <xsl:text>&#10;</xsl:text>
                    <xsl:value-of select="$FileMetadata"/>
                    <xsl:text>.lastModified=</xsl:text>
                    <xsl:value-of select="number[@key = 'lastModified']"/>
                    <xsl:text>&#10;</xsl:text>
                    <xsl:value-of select="$FileMetadata"/>
                    <xsl:text>.private=</xsl:text>
                    <xsl:value-of select="boolean[@key = 'private']"/>
                    <xsl:choose>
                        <xsl:when test="map[@key = 'role']">
                            <xsl:text>&#10;</xsl:text>
                            <xsl:value-of select="$FileMetadata"/>
                            <xsl:text>.role.label=</xsl:text>
                            <xsl:value-of select="map[@key = 'role']/string[@key = 'label']"/>
                            <xsl:text>&#10;</xsl:text>
                            <xsl:value-of select="$FileMetadata"/>
                            <xsl:text>.role.value=</xsl:text>
                            <xsl:value-of select="map[@key = 'role']/string[@key = 'value']"/>
                        </xsl:when>
                    </xsl:choose>
                    <xsl:choose>
                        <xsl:when test="array[@key = 'process']">
                            <xsl:for-each select="array[@key = 'process']/map">
                                <xsl:text>&#10;</xsl:text>
                                <xsl:value-of select="$FileMetadata"/>
                                <xsl:text>.process.label=</xsl:text>
                                <xsl:value-of select="string[@key = 'label']"/>
                                <xsl:text>&#10;</xsl:text>
                                <xsl:value-of select="$FileMetadata"/>
                                <xsl:text>.process.value=</xsl:text>
                                <xsl:value-of select="string[@key = 'value']"/>
                            </xsl:for-each>
                        </xsl:when>
                    </xsl:choose>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>
</xsl:stylesheet>
