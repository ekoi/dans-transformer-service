<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:math="http://www.w3.org/2005/xpath-functions/math"
    xmlns:xs="http://www.w3.org/2001/XMLSchema" exclude-result-prefixes="xs math" version="3.0">
    <xsl:output indent="yes" omit-xml-declaration="yes"/>

   <!-- Mapping from the Dataverse keywords to the Narcis Discipline types (https://easy.dans.knaw.nl/schemas/vocab/2015/narcis-type.xsd) -->
  <xsl:template name="audiencefromkeyword">
    <xsl:param name="val"/>
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
  <xsl:template name="convertdatetime">
    <xsl:param name="val"/>
     <xsl:choose>
        <xsl:when test="string-length($val) = 4">
          <xsl:value-of select="concat($val, '-01-01')"/>
        </xsl:when>
       <xsl:when test="string-length($val) = 7">
         <xsl:value-of select="concat(substring($val, 4, 4), '-', substring($val, 1, 2), '-01')"/>
        </xsl:when>
        <xsl:when test="string-length(.) = 10">
          <xsl:value-of select="concat(substring($val, 7, 4), '-', substring($val, 4, 2), '-', substring($val, 1, 2))"/>
        </xsl:when>
        <xsl:when test="string-length(.) = 16">
          <xsl:value-of select="concat(substring($val, 7, 4), '-', substring($val, 4, 2), '-', substring($val, 1, 2))"/>
        </xsl:when>
        <xsl:otherwise></xsl:otherwise>
      </xsl:choose>
    <xsl:if test="position() != last() and string-length($val) !=0">
        <xsl:text>,</xsl:text>
      </xsl:if>
  </xsl:template>
</xsl:stylesheet>
