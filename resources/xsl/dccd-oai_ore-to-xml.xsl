<?xml version="1.0"?>
<xsl:stylesheet
	xmlns:keyword="https://dataverse.org/schema/citation/keyword#"
	xmlns:dcterms="http://purl.org/dc/terms/" 
	xmlns:ore="http://www.openarchives.org/ore/terms/"
	xmlns:dccd="https://dataverse.nl/schema/dccd#"
	xmlns:dccd-location="https://dataverse.nl/schema/dccd/dccd-location#"
	xmlns:citation="https://dataverse.org/schema/citation/"
	xmlns:otherId="https://dataverse.org/schema/citation/otherId#"
	xmlns:datasetContact="https://dataverse.org/schema/citation/datasetContact#"
	xmlns:dsDescription="https://dataverse.org/schema/citation/dsDescription#"
	xmlns:schema="http://schema.org/" 
	xmlns:dvcore="https://dataverse.org/schema/core#"
	xmlns:author="http://purl.org/dc/terms/creator"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:math="http://www.w3.org/2005/xpath-functions/math"
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	xmlns="http://www.w3.org/2005/xpath-functions"
	exclude-result-prefixes="xs math" version="3.0">
	<xsl:output encoding="UTF-8" indent="yes" method="xml"/>
	<xsl:output indent="yes" omit-xml-declaration="no" />
	<xsl:strip-space elements="*"/>
	<xsl:mode on-no-match="shallow-skip"/>
	<xsl:template match="data">
		<!-- create a new root tag -->
		<xsl:element name="root">
			<xsl:namespace name="keyword" select="'https://dataverse.org/schema/citation/keyword#'"/>
			<xsl:namespace name="dcterms" select="'http://purl.org/dc/terms/'"/>
			<xsl:namespace name="ore" select="'http://www.openarchives.org/ore/terms/'"/>
			<xsl:namespace name="dccd" select="'https://dataverse.nl/schema/dccd#'"/>
			<xsl:namespace name="dccd-location" select="'https://dataverse.nl/schema/dccd/dccd-location#'"/>
			<xsl:namespace name="citation" select="'https://dataverse.org/schema/citation/'"/>
			<xsl:namespace name="otherId" select="'https://dataverse.org/schema/citation/otherId#'"/>
			<xsl:namespace name="datasetContact" select="'https://dataverse.org/schema/citation/datasetContact#'"/>
			<xsl:namespace name="dsDescription" select="'https://dataverse.org/schema/citation/dsDescription#'"/>
			<xsl:namespace name="schema" select="'http://schema.org/'"/>
			<xsl:namespace name="dvcore" select="'https://dataverse.org/schema/core#'"/>
			<xsl:namespace name="author" select="'http://purl.org/dc/terms/creator'"/>
			<!-- apply the xml structure generated from JSON -->
			<xsl:apply-templates select="json-to-xml(.)" />
		</xsl:element>	
	</xsl:template>
	
	<!-- template for the first tag -->
	<xsl:template match="/map/string"
		xpath-default-namespace="http://www.w3.org/2005/xpath-functions">		
		<xsl:choose>
			<xsl:when test="@key">
				<xsl:element name="{translate(@key, '@','')}">
					<xsl:value-of select="."/>
				</xsl:element>
			</xsl:when>
			<xsl:otherwise>
				do something else
			</xsl:otherwise>
		</xsl:choose>
		
	</xsl:template>
	<xsl:template match="/map/map[@key='ore:describes']"
		xpath-default-namespace="http://www.w3.org/2005/xpath-functions">
		
			<xsl:call-template name="tai"></xsl:call-template>
		
				
		
	</xsl:template>
	<xsl:template name="tai">
		<xsl:element name="{@key}">
			<xsl:apply-templates></xsl:apply-templates>
		</xsl:element>
	</xsl:template>
	<xsl:template match="/map/map[@key='ore:describes']/node()"
		xpath-default-namespace="http://www.w3.org/2005/xpath-functions">		
		
		<xsl:element name="{translate(translate(./@key, '@',''), ' ', '')}">
			<xsl:choose>
				<xsl:when test="*">
					<xsl:for-each select="node()">
						<xsl:choose>
							<xsl:when test="*">
								<xsl:for-each select="node()">
									<xsl:choose>
										<xsl:when test="@key">
											<xsl:element name="{translate(translate(./@key, '@',''), ' ', '')}">
												<xsl:value-of select="."/>
											</xsl:element>
										</xsl:when>
										<xsl:otherwise>
											do something else
										</xsl:otherwise>
									</xsl:choose>
								</xsl:for-each>
							</xsl:when>
							<xsl:otherwise>
								<xsl:copy-of select="."/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:for-each>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="." />
				</xsl:otherwise>
			</xsl:choose>
			
		</xsl:element>
	</xsl:template>
	<!--
	<xsl:template name="eko">
		<xsl:choose>
			<xsl:when test="./string/@key">
				<xsl:element name="{translate(./@key, '@','')}">
					
				</xsl:element>
			</xsl:when>
			<xsl:otherwise>
				do something else
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
		<xsl:element name="{/map/string[1]/@key}">
				<xsl:value-of select="/map/string[1]"/>
			</xsl:element>
	<xsl:template match="number"
		xpath-default-namespace="http://www.w3.org/2005/xpath-functions">
		<num>
			<xsl:value-of select="." />
		</num>
	</xsl:template>
	 -->
</xsl:stylesheet>