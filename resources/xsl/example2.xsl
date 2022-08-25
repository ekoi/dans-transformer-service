<?xml version="1.0"?>
<xsl:stylesheet
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:math="http://www.w3.org/2005/xpath-functions/math"
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	exclude-result-prefixes="xs math" version="3.0">
	<xsl:output indent="yes" omit-xml-declaration="yes" />

	<xsl:template match="data">
		<!-- create a new root tag -->
		<my-document>
			<!-- apply the xml structure generated from JSON -->
			<xsl:apply-templates select="json-to-xml(.)" />
		</my-document>
	</xsl:template>

	<!-- template for the first tag -->
	<xsl:template match="map"
		xpath-default-namespace="http://www.w3.org/2005/xpath-functions">

		<position>
			<!-- select a sub-node structure  -->
			<xsl:apply-templates select="array[@key='content']/map/array[@key='position']/number" />
		</position>

	</xsl:template>

	<!-- template to output a number value -->
	<xsl:template match="number"
		xpath-default-namespace="http://www.w3.org/2005/xpath-functions">
		<num>
			<xsl:value-of select="." />
		</num>
	</xsl:template>

</xsl:stylesheet>