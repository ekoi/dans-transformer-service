<?xml version="1.0"?>
<xsl:stylesheet
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:dcterms="http://purl.org/dc/terms/"
    xmlns:ddm="http://schemas.dans.knaw.nl/dataset/ddm-v2/"
    xmlns:dcx-dai="http://easy.dans.knaw.nl/schemas/dcx/dai/"
    xmlns:dcx-gml="http://easy.dans.knaw.nl/schemas/dcx/gml/"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:math="http://www.w3.org/2005/xpath-functions/math"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    exclude-result-prefixes="xs math" version="3.0">
    <xsl:output indent="yes" omit-xml-declaration="no" />

    <xsl:template match="data">
        <!-- create a new root tag -->
        <ddm:DDM>
            <!-- apply the xml structure generated from JSON -->
            <xsl:apply-templates select="json-to-xml(.)" />
        </ddm:DDM>
    </xsl:template>

    <!-- template for the first tag -->
    <xsl:template match="map"
        xpath-default-namespace="http://www.w3.org/2005/xpath-functions">
        <ddm:profile>
        <!-- CIT001 -->
            <dc:title><xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='title']/following-sibling::string[@key='value']/."/></dc:title>
            
        <!-- CIT002: see dcmiMetadata -->
        <!-- CIT003: see bag-info.txt -->
        <!-- CIT004: see dcmiMetadata -->

        <!-- CIT009 -->
        <dcterms:description>
            <xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='description']/following-sibling::string[@key='value']/."/>
        </dcterms:description>
        <!-- CIT005 -->
       <!-- <dc:creator>
            Unformatted Creator
        </dc:creator>-->
        <!-- CIT006 -->
            <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='author']/following-sibling::array[@key='fields']/array/map/string[@key='name' and text()='name']">
                <dcx-dai:creatorDetails>
                    <dcx-dai:author>
                        <dcx-dai:titles></dcx-dai:titles>
                        <dcx-dai:initials></dcx-dai:initials>
                        <dcx-dai:surname>
                <xsl:value-of select="following-sibling::map[@key='value']/string[@key='label']"/>
                        </dcx-dai:surname>
                        <xsl:if test="following-sibling::map[@key='value']/string[@key='idLabel']='ORCID ID'">
                            <dcx-dai:ORCID><xsl:value-of select="following-sibling::map[@key='value']/string[@key='value']"/></dcx-dai:ORCID>
                        </xsl:if>
                        <dcx-dai:organization>
                            <dcx-dai:name xml:lang="en">
                <xsl:value-of select="../../map/string[@key='name' and text()='affiliation']/following-sibling::string[@key='value']"/>
                            </dcx-dai:name>
                        </dcx-dai:organization>
                    </dcx-dai:author>
                </dcx-dai:creatorDetails>
            </xsl:for-each>
            
        <!-- CIT007 -->
       <!-- <dcx-dai:creatorDetails>
            <dcx-dai:organization>
                <dcx-dai:name xml:lang="en">Creator Organization</dcx-dai:name>
            </dcx-dai:organization>
        </dcx-dai:creatorDetails>-->
        <!-- CIT008: contact is created from Dataverse account of depositing user -->

        <!-- CIT019 -->
        <xsl:variable name="ori-date">
            <xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='administrative']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='date_available']/following-sibling::string[@key='value']/."/>
        </xsl:variable>
        <xsl:variable name="converted-date">
            <xsl:value-of select="concat(substring($ori-date, 7, 4), '-', substring($ori-date, 4, 2), '-', substring($ori-date, 1, 2))"/>
            
        </xsl:variable>
        <ddm:created><xsl:value-of select="$converted-date"/></ddm:created>
        <!-- CIT025 -->
        <ddm:available>
            <xsl:value-of select="$converted-date"/>
        </ddm:available>
            <!-- CIT013 and REL001 -->
        <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='subject']/following-sibling::array[@key='value']/map">
            <ddm:audience>
                <xsl:value-of select="./string[@key='id']/." />
            </ddm:audience>
        </xsl:for-each>
            <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='relations']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='audience']/following-sibling::array[@key='value']/map">
                <ddm:audience><xsl:value-of select="./string[@key='id']"/></ddm:audience>
            </xsl:for-each>
        <!-- If changed to NO_ACCESS, demonstrates TRM003 -->
        <ddm:accessRights>OPEN_ACCESS</ddm:accessRights>
        <!-- RIG002 -->
         <xsl:variable name="personal-data">
             <xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='rights']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='personal_data']/following-sibling::string[@key='value']/."/>
         </xsl:variable>
<!--            <ddm:personalData present="No"/>-->
        <ddm:personalData>
            <xsl:attribute name="present">
                <xsl:choose>
                    <xsl:when test="$personal-data = 'personal_data_true'">
                        <xsl:text>Yes</xsl:text>
                    </xsl:when>
                    <xsl:when test="$personal-data = 'personal_data_unknown'">
                        <xsl:text>Unknown</xsl:text>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:text>No</xsl:text>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:attribute>
        </ddm:personalData>
    </ddm:profile>
    <ddm:dcmiMetadata>
        <!-- CIT002 -->
        
        <!-- CIT004 -->
        <!--<dcterms:identifier>DCTERMS_ID001</dcterms:identifier>
        <dcterms:identifier>DCTERMS_ID002</dcterms:identifier>
        <dcterms:identifier>DCTERMS_ID003</dcterms:identifier>-->

        <!-- CIT011 -->
       <!-- <dcterms:date>some date</dcterms:date>
        <dcterms:dateAccepted>some acceptance date</dcterms:dateAccepted>
        <dcterms:dateCopyrighted>some copyright date</dcterms:dateCopyrighted>
        <dcterms:dateSubmitted>some submission date</dcterms:dateSubmitted>
        <dcterms:modified>some modified date</dcterms:modified>
        <dcterms:issued>some issuing date</dcterms:issued>
        <dcterms:valid>some validation date</dcterms:valid>
        <dcterms:coverage>some coverage description</dcterms:coverage>-->
            
        <!-- CIT012 -->
<!--        <dcterms:description>Even more descriptions</dcterms:description>
-->
        <!-- CIT014 -->
        <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='humanities']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='domain_specific_keywords']/following-sibling::array[@key='value']/map">
            <dcterms:subject>
                <xsl:value-of select="./string[@key='label']/." />
                (<xsl:value-of select="./string[@key='value']/." />)
            </dcterms:subject>
        </xsl:for-each>
        <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='grant']/following-sibling::array[@key='fields']/array/map">
            <xsl:if test="./string[@key='name' and text()='grant_agency']">
                <dcterms:subject>
                    <xsl:value-of select="./string[@key='name' and text()='grant_agency']/following-sibling::string[@key='value']"/>
                    <xsl:if test="../map/string[@key='name' and text()='grant_number']/following-sibling::string[@key='value'] !=''"><xsl:text>, </xsl:text>
                        <xsl:value-of select="../map/string[@key='name' and text()='grant_number']/following-sibling::string[@key='value']"/>
                    </xsl:if>
                </dcterms:subject>
            </xsl:if>
        </xsl:for-each>
        <!--<dcterms:subject>keyword1</dcterms:subject>-->
        <!--<dcterms:subject>keyword2</dcterms:subject>-->

        <!-- CIT015 -->
        
        <!-- CIT016 -->
       
        <!-- CIT017 -->
<!--        <dcterms:identifier xsi:type="id-type:ISSN">0317-8471</dcterms:identifier>
-->
        <!-- CIT018 -->
        <xsl:choose>
            <xsl:when test="//array[@key='metadata']/map/string[@key='id' and text()='administrative']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='language']/following-sibling::map[@key='value']/string[@key='value']/. != ''">
                <ddm:language encodingScheme="ISO639-1">
                    <xsl:attribute name="code">
                        <xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='administrative']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='language']/following-sibling::map[@key='value']/string[@key='value']/."/>                     
                    </xsl:attribute>  
                    <xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='administrative']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='language']/following-sibling::map[@key='value']/string[@key='label']/."/>        
                </ddm:language>
            </xsl:when>
            <xsl:otherwise>
                <ddm:language encodingScheme="ISO639-1" code="en">English</ddm:language>
            </xsl:otherwise>
        </xsl:choose>
        <!-- CIT019 see profile -->

        <!-- CIT020 -->
        <!--<dcx-dai:contributorDetails>
            <dcx-dai:author>
                <dcx-dai:titles>Prof Dr</dcx-dai:titles>
                <dcx-dai:initials>CON</dcx-dai:initials>
                <dcx-dai:insertions>van</dcx-dai:insertions>
                <dcx-dai:surname>Tributor</dcx-dai:surname>
                <dcx-dai:role>ProjectMember</dcx-dai:role>
                <dcx-dai:organization>
                    <dcx-dai:name xml:lang="en">Contributing Org</dcx-dai:name>
                </dcx-dai:organization>
            </dcx-dai:author>
        </dcx-dai:contributorDetails>
-->
        <!-- CIT021 -->
        <!--<dcx-dai:contributorDetails>
            <dcx-dai:organization>
                <dcx-dai:name xml:lang="en">Contributing Org</dcx-dai:name>
                <dcx-dai:role>Sponsor</dcx-dai:role>
            </dcx-dai:organization>
        </dcx-dai:contributorDetails>-->

        <!-- CIT022 -->
        <!-- Rule under revision -->

        <!-- CIT023 -->
<!--        <dcterms:identifier xsi:type="id-type:NWO-PROJECTNR">54321</dcterms:identifier>-->

        <!-- CIT024 -->
        <dcterms:publisher>
            <xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='citation']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='publisher']/following-sibling::map[@key='value']/string[@key='label']/."/>
        </dcterms:publisher>
        <!-- Special values: skipped -->
        <!--<dcterms:publisher>DANS</dcterms:publisher>
        <dcterms:publisher>DANS-KNAW</dcterms:publisher>
        <dcterms:publisher>DANS/KNAW</dcterms:publisher>-->

        <!-- CIT025 see profile -->

        <!-- CIT026 -->
        

        <!-- CIT027 -->
        
        <!-- CIT028 --> <!-- RIG003 Any element can add a metadata language -->
        
        <!-- RIG001 -->
        <dcterms:rightsHolder>
            <xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='rights']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='rightsholder']/following-sibling::map[@key='value']/string[@key='label']/."/>
        </dcterms:rightsHolder>

        <!-- RIG002 see profile -->

        <!-- RIG003 Any element can add a metadata language -->
<!--        <dcterms:abstract xml:lang="ka">Georgian</dcterms:abstract>-->


        <!-- REL001 see profile -->

        <!-- REL002 -->
       
        <!-- REL003. N.B. hrefs do not resolve -->
        <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='relations']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='relation']/following-sibling::array[@key='fields']/array/map/string[@key='name' and text()='relation_item']">
<!--            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>-->
            <xsl:choose>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Has Format'">
                    <ddm:hasFormat><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:hasFormat>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Requires'">
                    <ddm:requires><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:requires>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Conforms to'">
                    <ddm:conformsTo><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:conformsTo>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='References'">
                    <ddm:references><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:references>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Replaces'">
                    <ddm:replaces><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:replaces>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Has version'">
                    <ddm:hasVersion><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:hasVersion>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Is format of'">
                    <ddm:isFormatOf><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:isFormatOf>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Is part of'">
                    <ddm:isPartOf><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:isPartOf>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Has part'">
                    <ddm:hasPart><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:hasPart>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Is referenced by'">
                    <ddm:isReferencedBy><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:isReferencedBy>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Is required by'">
                    <ddm:isRequiredBy><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:isRequiredBy>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Is version of'">
                    <ddm:isVersionOf><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:isVersionOf>
                </xsl:when>
                <xsl:when test="../../map/map[@key='value']/string[@key='value']/text()='Is replaced by'">
                    <ddm:isReplacedBy><xsl:attribute name="href">            <xsl:value-of select="../../map/string[@key='name' and text()='relation_reference']/following-sibling::string[@key='value']"/>
                    </xsl:attribute><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:isReplacedBy>
                </xsl:when>
            <xsl:otherwise></xsl:otherwise>
            </xsl:choose>
            
<!--            <ddm:conformsTo href="https://example.com/conformsTo"><xsl:value-of select="./following-sibling::string[@key='value']"/></ddm:conformsTo>-->
            
        </xsl:for-each>
       <!--<ddm:relation href="https://example.com/relation">Test relation</ddm:relation>
        <ddm:conformsTo href="https://example.com/conformsTo">Test conforms to</ddm:conformsTo>
        <ddm:hasFormat href="https://example.com/hasFormat">Test has format</ddm:hasFormat>
        <ddm:hasPart href="https://example.com/hasPart">Test has part</ddm:hasPart>
        <ddm:references href="https://example.com/references">Test references</ddm:references>
        <ddm:replaces href="https://example.com/replaces">Test replaces</ddm:replaces>
        <ddm:requires href="https://example.com/requires">Test requires</ddm:requires>
        <ddm:hasVersion href="https://example.com/hasVersion">Test has version</ddm:hasVersion>
        <ddm:isFormatOf href="https://example.com/isFormatOf">Test is format of</ddm:isFormatOf>
        <ddm:isPartOf href="https://example.com/isPartOf">Test is part of</ddm:isPartOf>
        <ddm:isReferencedBy href="https://example.com/isReferencedBy">Test is referenced by</ddm:isReferencedBy>
        <ddm:isRequiredBy href="https://example.com/isRequiredBy">Test is required by</ddm:isRequiredBy>
        <ddm:isVersionOf href="https://example.com/isVersionOf">Test is version of</ddm:isVersionOf>-->

        <!-- AR001 -->
<!--        <dcterms:identifier xsi:type="id-type:ARCHIS-ZAAK-IDENTIFICATIE">12345</dcterms:identifier>-->

        <!-- AR002 -->
       <!-- <dcterms:identifier xsi:type="id-type:ARCHIS-ONDERZOEK">12345</dcterms:identifier>
        <dcterms:identifier xsi:type="id-type:ARCHIS-VONDSTMELDING">67890</dcterms:identifier>
        <dcterms:identifier xsi:type="id-type:ARCHIS-MONUMENT">12345</dcterms:identifier>
        <dcterms:identifier xsi:type="id-type:ARCHIS-WAARNEMING">67890</dcterms:identifier>
-->
        <!-- AR003 and AR004 -->
        
        <!-- AR005 -->
       
        <!-- AR006 -->
       
        <!-- AR007 -->
      

        <!-- AR008 -->
        <!--<ddm:temporal
                subjectScheme="ABR Periodes"
                schemeURI="https://data.cultureelerfgoed.nl/term/id/abr/9b688754-1315-484b-9c89-8817e87c1e84"
                valueURI="https://data.cultureelerfgoed.nl/term/id/abr/5b253754-ddd0-4ae0-a5bb-555176bca858">
            Midden Romeinse Tijd A
        </ddm:temporal>-->

        <!-- TS001 -->
       <!-- <dcterms:temporal>Het Romeinse Rijk</dcterms:temporal>
        <dcterms:temporal>De Oudheid</dcterms:temporal>
-->
        <!-- TS002 -->
        <!--<dcx-gml:spatial srsName="http://www.opengis.net/def/crs/EPSG/0/28992">
            <Point xmlns="http://www.opengis.net/gml">
                <pos>126466 529006</pos>
            </Point>
        </dcx-gml:spatial>
-->
        <!-- TS003, defaults to latitude/longitude-->
        <!-- 
        <xsl:variable name="lat">
            <xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='coverage']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='subject_location']/following-sibling::array[@key='value']/map/array[@key='coordinates']/string[1]/."/>
        </xsl:variable>
        <xsl:variable name="long">
            <xsl:value-of select="//array[@key='metadata']/map/string[@key='id' and text()='coverage']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='subject_location']/following-sibling::array[@key='value']/map/array[@key='coordinates']/string[2]/."/>
        </xsl:variable>
        <dcx-gml:spatial>
            <Point xmlns="http://www.opengis.net/gml">
                <pos>
                    <xsl:value-of select="$lat"/><xsl:text> </xsl:text><xsl:value-of select="$long"/>
                </pos>
            </Point>
        </dcx-gml:spatial>
        -->
        <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='coverage']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='subject_location']/following-sibling::array[@key='value']/map/array[@key='coordinates']">
           <dcx-gml:spatial>
                <Point xmlns="http://www.opengis.net/gml">
                    <pos>
                        <xsl:value-of select="./string[1]"/><xsl:text> </xsl:text><xsl:value-of select="./string[2]"/>
                    </pos>
                </Point>
            </dcx-gml:spatial>
        </xsl:for-each>
       
        <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='coverage']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='subject_keywords']/following-sibling::array[@key='value']/map">
            <ddm:subject xml:lang="en" schemeURI="http://vocab.getty.edu/aat/"
                subjectScheme="Art and Architecture Thesaurus"
                >
                <xsl:attribute name="valueURI">
                    <xsl:value-of select="./string[@key='value']/."/>
                </xsl:attribute>
                <xsl:value-of select="./string[@key='label']/."/>
            </ddm:subject>         
        </xsl:for-each>
        <xsl:for-each select="//array[@key='metadata']/map/string[@key='id' and text()='coverage']/following-sibling::array[@key='fields']/map/string[@key='name' and text()='subject_date_time']/following-sibling::array[@key='fields']/array/map"> 
            <xsl:if test="./string[@key='name']/text()='subject_date_time_start'">
                <dcterms:temporal><xsl:if test="./string[@key='value'] !=''">start:<xsl:value-of select="./string[@key='value']"/></xsl:if><xsl:text>  </xsl:text><xsl:if test="../map/string[@key='name' and text()='subject_date_time_end']/following-sibling::string[@key='value']">end: <xsl:value-of select="../map/string[@key='name' and text()='subject_date_time_end']/following-sibling::string[@key='value']"/></xsl:if></dcterms:temporal>
            </xsl:if>
            
        </xsl:for-each>
        <!-- TS004 -->
        <!--<dcx-gml:spatial>
            <boundedBy xmlns="http://www.opengis.net/gml">
                <Envelope srsName="http://www.opengis.net/def/crs/EPSG/0/28992">
                    <lowerCorner>102000 335000</lowerCorner>
                    <upperCorner>140000 628000</upperCorner>
                </Envelope>
            </boundedBy>
        </dcx-gml:spatial>-->

        <!-- TS005 -->
    <!--    <dcx-gml:spatial>
            <boundedBy xmlns="http://www.opengis.net/gml">
                <Envelope srsName="http://www.opengis.net/def/crs/EPSG/0/4326">
                    <lowerCorner>51.46343658020442 3.5621054065986075</lowerCorner>
                    <upperCorner>53.23074335194507 6.563118076315912</upperCorner>
                </Envelope>
            </boundedBy>
        </dcx-gml:spatial>-->

        <!-- Not RD or lat/lon, so should be ignored -->
        <!--<dcx-gml:spatial>
            <boundedBy xmlns="http://www.opengis.net/gml">
                <Envelope srsName="XXX">
                    <lowerCorner>1 2</lowerCorner>
                    <upperCorner>3 4</upperCorner>
                </Envelope>
            </boundedBy>
        </dcx-gml:spatial>-->

        <!-- TS006, note that the text must exactly match the vocabulary term, so South-Africa will be handled by TS007 -->
<!--        <dcterms:spatial>South Africa</dcterms:spatial>-->
        
        <!-- TS007, not in the controlled list, so mapped to the free text field -->
<!--        <dcterms:spatial>Roman Empire</dcterms:spatial>-->

        <!--TRM001 -->
            <dcterms:license xsi:type="dcterms:URI">https://doi.org/10.17026/fp39-0x58</dcterms:license>
        <!-- TRM002 see example restricted-files-no-access-request -->

        <!-- TRM005 and TRM006 -->
        <!--<dcterms:accessRights>Restricted files accessible under the following conditions: ...</dcterms:accessRights>-->

    </ddm:dcmiMetadata>
        
        <!--<ddm:dcmiMetadata>
            <dcterms:license xsi:type="dcterms:URI">http://opensource.org/licenses/MIT</dcterms:license>
            <dcterms:rightsHolder>I Lastname</dcterms:rightsHolder>
        </ddm:dcmiMetadata>-->
    </xsl:template>


</xsl:stylesheet>