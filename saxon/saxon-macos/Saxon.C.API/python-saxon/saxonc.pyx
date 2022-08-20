"""@package saxonc
This documentation details the Python API for SaxonC, which has been written in cython for Python3.
SaxonC is a cross-compiled variant of Saxon from the Java platform to the C/C++ platform.
SaxonC provides processing in XSLT 3.0, XQuery 3.1 and XPath 3.1, and Schema validation 1.0/1.1.
Main classes in SaxonC Python API: PySaxonProcessor, PyXslt30Processor, PyXsltExecutable, PyXQueryProcessor,
PySchemaValidator, PyXdmValue, PyXdmItem, PyXdmAtomicValue, PyXdmNode, PyXdmFunctionItem, PyXdmArray, and PyXdmMap."""
# distutils: language = c++
# distutils: sources = SaxonProcessor.cpp
# cython: language_level = 3

cimport saxoncClasses
from libcpp cimport bool
from libcpp.string cimport string
from libcpp.map cimport map
from nodekind import *
from cython.operator import dereference, postincrement

from libc.stdlib cimport free, malloc, realloc
from libc.string cimport memcpy

def create_xdm_dict(proc, mmap):
    """
    create_xdm_dict(proc, mmap)
    Function to create dictionary of XdmAtomicValue, XdmValue pairs from primitive types
    Args:
        proc (PySaxonProcessor): PySaxonProcessor object required to create the XdmAtomicValue from primitive types
        mmap(dict): The dict of key value paris to convert
        Example:
              with saxonc.PySaxonProcessor(license=False) as proc:
                         mymap = {"a":saxonproc.make_integer_value(1), "b":saxonproc.make_integer_value(2),
                 "c":saxonproc.make_integer_value(3)}

        xdmdict = create_xdm_dict(saxonproc, mymap)
        map =saxonproc.make_map(xdmdict)
    """
    xdmMap = {}
    xdmValue_ = None
    for (key, value) in mmap.items():
        if isinstance(key, str):
            xdmKey_ = proc.make_string_value(key)

            if isinstance(value, str):
                xdmValue_ = proc.make_sting_value(value)
            elif isinstance(value,int):
                xdmValue_ = proc.make_integer_value(value)
            elif isinstance(value,float):
                xdmValue_ = proc.make_integer_value(double)
            elif value in (True, False):
                xdmValue_ = proc.make_boolean_value(value)

            elif isinstance(value, PyXdmValue):
                xdmValue_ = value

            elif isinstance(value, PyXdmItem):
                xdmValue_ = value

            elif isinstance(value, PyXdmAtomicValue):
                xdmValue_ = value
            elif isinstance(value, PyXdmNode):
                xdmValue_ = value

            elif isinstance(value, PyXdmMap):
                xdmValue_ = value

            elif isinstance(value, PyXdmArray):
                xdmValue_ = value
            else:
                continue

            xdmMap[xdmKey_] = xdmValue_
        else:
                   raise Exception("Error in making Dictionary")

    return xdmMap

cdef char * make_c_str(str str_value, encoding='utf-8'):
    if str_value is None:
        return NULL
    else:
        py_string_string = str_value.encode(encoding) if str_value is not None else None
        c_string = py_string_string if str_value is not None else ""
        return c_string


cdef char * make_c_str2(str_value, encoding='utf-8'):
    cdef char         *line
    cdef Py_ssize_t   i
    cdef Py_ssize_t   length = 0
    cdef Py_ssize_t   incrlength
    cdef char         *out = <char *>malloc(1)  # Reallocate as needed

    py_string_string = str_value.encode(encoding) if str_value is not None else None
    line = py_string_string if str_value is not None else ""

    try:
        out[0] = b'\x00' # keep C-strings null-terminated
        incrlength = len(line)
        out = <char *>realloc(out, length + incrlength + 1)
        memcpy(out + length, line, incrlength)
        length += incrlength
        out[length] = '\x00'  # keep C-strings null-terminated
        return out  # autoconversion back to a Python string

    finally:
       free(out)


cdef str make_py_str(const char * c_value, encoding='utf-8'):
    ustring = c_value.decode(encoding) if c_value is not NULL else None
    return ustring

cdef class PySaxonProcessor:
    """An SaxonProcessor acts as a factory for generating XQuery, XPath, Schema and XSLT compilers.
    This class is itself the context that needs to be managed (i.e. allocation & release)
    Example:
          with saxonc.PySaxonProcessor(license=False) as proc:
             print("Test SaxonC on Python")
             print(proc.version)
             xdmAtomicval = proc.make_boolean_value(False)
             xslt30proc = proc.new_xslt30_processor()
    """
    cdef saxoncClasses.SaxonProcessor *thisptr      # hold a C++ instance which we're wrapping
    cdef bool _release  #  flag to indicate if this PySaxonProcessor should call release
    ##
    # The Constructor
    # @param license Flag that a license is to be used
    # @contextlib.contextmanager
    def __cinit__(self, config_file= None, license=False,  releasei=False):
        """
        __cinit__(self, license=False, config_file=None)
        The constructor.
        Args:
            config_file (str): Construct a Saxon processor based upon an configuration file
            license(bool): Flag that a license is to be used. The Default is false.

        """
        cdef const char * c_str = NULL
        cdef bool l = license
        self._release = releasei
        if config_file is not None:
            '''make_c_str(config_file)'''
            py_config_string = config_file.encode('UTF-8') if config_file is not None else None
            c_str = py_config_string if config_file is not None else ""
            if c_str is not NULL:
                self.thisptr = new saxoncClasses.SaxonProcessor(c_str)
                if self.thisptr == NULL:
                    raise Exception("Configuration file for SaxonProcessor is None")
            else:
                raise Exception("Configuration file for SaxonProcessor is None")
        else:
            self.thisptr = new saxoncClasses.SaxonProcessor(l)

    def __dealloc__(self):
        """The destructor."""
        if self.thisptr is not NULL:
          del self.thisptr
          self.thisptr = NULL
        if self._release is True:
            self.thisptr.release()
    def __enter__(self):
      """enter method for use with the keyword 'with' context"""
      return self

    def __exit__(self, exception_type, exception_value, traceback):
        """The exit method for the context PySaxonProcessor. Here we release the Jet VM resources.
        If we have more than one live PySaxonProcessor object the release() method has no effect.
        """

        ''' if self.thisptr is not NULL:
          del self.thisptr
          self.thisptr = NULL        
        self.thisptr.release()'''


    property version:
        """
        Get the Saxon Version.
        Getter:
            str: The Saxon version
        """
        def __get__(self):
            cdef const char* c_string = self.thisptr.version()
            ustring = make_py_str(c_string)
            return ustring

    @property
    def attach_current_thread(self):
        self.thisptr.attachCurrentThread()

    @property
    def detach_current_thread(self):
        self.thisptr.detachCurrentThread()

    @property
    def cwd(self):
        """
        cwd Property represents the current working directory
        :str: Get or set the current working directory"""
        cdef const char* c_string = self.thisptr.getcwd()
        ustring = make_py_str(c_string)
        return ustring
    def set_cwd(self, cwd):
         cdef char * c_str_ = NULL
         '''make_c_str(cwd)'''
         py_cwd_string = cwd.encode('UTF-8') if cwd is not None else None
         c_str_ = py_cwd_string if cwd is not None else ""
         self.thisptr.setcwd(c_str_)
    def set_resources_directory(self, dir_):
        """
        Property to set or get resources directory
        :str: A string of the resources directory which Saxon will use
        """
        cdef char * c_str_ = NULL
        '''make_c_str(dir_)'''
        py_dir_string = dir_.encode('UTF-8') if dir_ is not None else None
        c_str_ = py_dir_string if dir_ is not None else ""
        self.thisptr.setResourcesDirectory(c_str_)
    @property
    def resources_directory(self):
        cdef const char* c_string = self.thisptr.getResourcesDirectory()
        ustring = make_py_str(c_string)
        return ustring

    def set_configuration_property(self, name, value):
        """
        set_configuration_property(self, name, value)
        Set configuration property specific to the processor in use.
        Properties set here are common across all processors.
        Args:
            name (str): The name of the property
            value (str): The value of the property
        Example:
          'l': 'on' or 'off' - to enable the line number
        """
        cdef char * c_str_ = NULL
        '''make_c_str(name)'''
        py_name_string = name.encode('UTF-8') if name is not None else None
        c_str_ = py_name_string if name is not None else ""
        cdef char * c_value_str_ = NULL
        '''make_c_str(value)'''
        py_value_string = value.encode('UTF-8') if value is not None else None
        c_value_str_ = py_value_string if value is not None else ""
        self.thisptr.setConfigurationProperty(c_str_, c_value_str_)

    def clear_configuration_properties(self):
        """
        clear_configuration_properties(self)
        Clear the configurations properties in use by the processor
        """
        self.thisptr.clearConfigurationProperties()

    @property
    def is_schema_aware(self):
        """
        is_schema_aware(self)
        Property to check if the processor is Schema aware. A licensed Saxon-EE/C product is schema aware
        :bool: Indicate if the processor is schema aware, True or False otherwise
        """
        return self.thisptr.isSchemaAwareProcessor()

    def new_document_builder(self):
        """
        new_document_builder(self)
        Create an PyDocumentBuilder. A PyDocumentBuilder holds properties controlling how a Saxon document tree should be built, and
        provides methods to invoke the tree construction.
        Returns:
            PyDocumentBuilder: a newly created PyDocumentProcessor
        """
        cdef PyDocumentBuilder val = PyDocumentBuilder()
        val.thisdbptr = self.thisptr.newDocumentBuilder()
        return val

    def new_xslt30_processor(self):
        """
        new_xslt30_processor(self)
        Create an PyXslt30Processor. A PyXslt30Processor is used to compile and execute XSLT 3.0 stylesheets.
        Returns:
            PyXslt30Processor: a newly created PyXslt30Processor
        """
        cdef PyXslt30Processor val = PyXslt30Processor()
        val.thisxptr = self.thisptr.newXslt30Processor()
        return val
    def new_xquery_processor(self):
        """
        new_xquery_processor(self)
        Create an PyXqueryProcessor. A PyXQueryProcessor is used to compile and execute XQuery queries.
        Returns:
            PyXQueryProcessor: a newly created PyXQueryProcessor
        """
        cdef PyXQueryProcessor val = PyXQueryProcessor()
        val.thisxqptr = self.thisptr.newXQueryProcessor()
        return val
    def new_xpath_processor(self):
        """
        new_xpath_processor(self)
        Create an PyXPathProcessor. A PyXPathProcessor is used to compile and execute XPath expressions.
        Returns:
            PyXPathProcessor: a newly created XPathProcessor
        """
        cdef PyXPathProcessor val = PyXPathProcessor()
        val.thisxpptr = self.thisptr.newXPathProcessor()
        return val
    def new_schema_validator(self):
        """
        new_schema_validator(self)
        Create a PySchemaValidator which can be used to validate instance documents against the schema held by this
        Returns:
            PySchemaValidator: a newly created PySchemaValidator
        """
        cdef PySchemaValidator val = PySchemaValidator()
        val.thissvptr = self.thisptr.newSchemaValidator()
        if val.thissvptr is NULL:
            raise Exception("Error: Saxon Processor is not licensed for schema processing!")
        return val
    def make_string_value(self, value):
        """
        make_string_value(self, str_)
        Factory method. Unlike the constructor, this avoids creating a new StringValue in the case
        of a zero-length string (and potentially other strings, in future)

        Args:
            str_ (str): the String value. NULL is taken as equivalent to "".
        Returns:
            PyXdmAtomicValue: The corresponding Xdm StringValue
        """
        cdef char * c_str_
        py_value_string = value.encode('utf-8') if value is not None else None
        c_str_ = py_value_string if value is not None else ""
        cdef PyXdmAtomicValue val = PyXdmAtomicValue()
        val.derivedaptr = val.derivedptr = val.thisvptr = self.thisptr.makeStringValue(c_str_)
        return val
    def make_integer_value(self, value):
        """
        make_integer_value(self, value)
        Factory method: makes either an Int64Value or a BigIntegerValue depending on the value supplied

        Args:
            value (int): The supplied primitive integer value
        Returns:
            PyXdmAtomicValue: The corresponding Xdm value which is a BigIntegerValue or Int64Value as appropriate
        """
        cdef PyXdmAtomicValue val = PyXdmAtomicValue()
        val.derivedaptr = val.derivedptr = val.thisvptr = self.thisptr.makeIntegerValue(value)
        return val
    def make_double_value(self, value):
        """
        make_double_value(self, value)
        Factory method: makes a double value
        Args:
            value (double): The supplied primitive double value
        Returns:
            PyXdmAtomicValue: The corresponding Xdm Value
        """
        cdef PyXdmAtomicValue val = PyXdmAtomicValue()
        val.derivedaptr = val.derivedptr = val.thisvptr = self.thisptr.makeDoubleValue(value)
        return val
    def make_float_value(self, value):
        """
        make_float_value(self, value)
        Factory method: makes a float value
        Args:
            value (float): The supplied primitive float value
        Returns:
            PyXdmAtomicValue: The corresponding Xdm Value
        """
        cdef PyXdmAtomicValue val = PyXdmAtomicValue()
        val.derivedaptr = val.derivedptr = val.thisvptr = self.thisptr.makeFloatValue(value)
        return val
    def make_long_value(self, value):
        """
        make_long_value(self, value)
        Factory method: makes either an Int64Value or a BigIntegerValue depending on the value supplied
        Args:
            value (long): The supplied primitive long value
        Returns:
            PyXdmAtomicValue: The corresponding Xdm Value
        """
        cdef PyXdmAtomicValue val = PyXdmAtomicValue()
        val.derivedaptr = val.derivedptr = val.thisvptr = self.thisptr.makeLongValue(value)
        return val
    def make_boolean_value(self, value):
        """
        make_boolean_value(self, value)
        Factory method: makes a XdmAtomicValue representing a boolean Value
        Args:
            value (boolean): True or False, to determine which boolean value is required
        Returns:
            PyAtomicValue: The corresonding XdmValue
        """
        cdef bool c_b = value
        cdef PyXdmAtomicValue val = PyXdmAtomicValue()
        val.derivedaptr = val.derivedptr = val.thisvptr = self.thisptr.makeBooleanValue(c_b)
        return val
    def make_qname_value(self, str_):
        """
        make_qname_value(self, str_)
        Create an QName Xdm value from string representation in clark notation
        Args:
            str_ (str): The value given in a string form in clark notation. {uri}local
        Returns:
            PyAtomicValue: The corresonding value
        """
        py_value_string = str_.encode('UTF-8') if str_ is not None else None
        cdef char * c_str_ = py_value_string if str_ is not None else ""
        cdef PyXdmAtomicValue val = PyXdmAtomicValue()
        val.derivedaptr = val.derivedptr = val.thisvptr = self.thisptr.makeQNameValue(c_str_)
        return val
    def make_atomic_value(self, value_type, value):
        """
        make_atomic_value(self, value_type, value)
        Create an QName Xdm value from string representation in clark notation
        Args:
            str_ (str): The value given in a string form in clark notation. {uri}local
        Returns:
            PyAtomicValue: The corresponding value
        """
        py_valueType_string = value_type.encode('UTF-8') if value_type is not None else None
        cdef char * c_valueType_string = py_valueType_string if value_type is not None else ""
        cdef PyXdmAtomicValue val = PyXdmAtomicValue()
        val.derivedaptr = val.derivedptr = val.thisvptr = self.thisptr.makeAtomicValue(c_valueType_string, value)
        if val.derivedaptr == NULL:
            del val
            return None

        return val

    def make_array(self, list values):
        """
        make_array(self, list_values)
        Make an XdmArray whose members are from a list of XdmValues
        Args:
            list_values (list): List of XdmValues
        Returns:
            PyAXdmArray: The corresponding value
        """
        cdef int len_ = len(values)
        cdef saxoncClasses.XdmValue ** argumentV = self.thisptr.createXdmValueArray(len_)
        cdef PyXdmArray newArray_ = None
        cdef PyXdmAtomicValue avalue_
        cdef PyXdmItem ivalue_
        cdef PyXdmNode nvalue_
        cdef PyXdmArray aavalue_
        cdef PyXdmMap mvalue_
        cdef PyXdmFunctionItem fvalue_
        cdef PyXdmValue value_

        for x in range(len_):
          if isinstance(values[x], PyXdmValue):
            value_ = values[x]
            argumentV[x] = value_.thisvptr
          elif isinstance(values[x], PyXdmItem):
            ivalue_ = values[x]
            argumentV[x] = <saxoncClasses.XdmValue*>ivalue_.derivedptr
          elif isinstance(values[x], PyXdmAtomicValue):
            avalue_ = values[x]
            argumentV[x] = <saxoncClasses.XdmValue*> avalue_.derivedaptr
          elif isinstance(values[x], PyXdmNodeValue):
            nvalue_ = values[x]
            argumentV[x] =  <saxoncClasses.XdmValue *>nvalue_.derivednptr
          elif isinstance(values[x], PyXdmArrayValue):
            aavalue_ = values[x]
            argumentV[x] =  <saxoncClasses.XdmValue *>aavalue_.derivedaaptr
          elif isinstance(values[x], PyXdmMapValue):
            mvalue_ = values[x]
            argumentV[x] =  <saxoncClasses.XdmValue *>mvalue_.derivedmmptr
          else:
            raise Exception("Argument value at position " , x , " is not of type PyXdmValue. The following object found: ", type(values[x]))

        newArray_ = PyXdmArray()
        newArray_.derivedaaptr = newArray_.derivedfptr = newArray_.derivedptr = newArray_.thisvptr =  self.thisptr.makeArray(argumentV, len_)
        if newArray_.derivedaaptr == NULL:
            del newArray_
            return None
        return newArray_



    def make_map(self, dict dataMap):
        """
        make_map(self, dict dataMap)
        Make an XdmMap whose members are from a dict type. Members are key, value pair (XdmAtomicValue, XdmValue).
        The factory method create_xdm_dict(proc, mmap) can be used to create pairs of type (XdmAtomicValue, XdmValue) from primitive types, which
        then can be used as input to this function make_map.
        Args:
            dataMap (dict): Dictionary of  (XdmAtomicValue, XdmValue) pairs
        Returns:
            PyAXdmAMap: The corresponding value
        """
        cdef int len_ = len(dataMap)
        cdef saxoncClasses.XdmValue ** c_values = self.thisptr.createXdmValueArray(len_)
        cdef saxoncClasses.XdmAtomicValue ** c_keys = self.thisptr.createXdmAtomicValueArray(len_)
        cdef PyXdmValue value_
        cdef PyXdmAtomicValue avalue_
        cdef PyXdmAtomicValue key_
        cdef const char * c_key = NULL
        cdef char * c_key_str = NULL
        cdef PyXdmMap newMap_ = None
        cdef int i = 0
        '''global parametersDict
        if kwds is not None:
                parametersDict = kwds '''
        for (key, value) in dataMap.items():

            if isinstance(key, PyXdmAtomicValue):
                avalue_ = key
                c_keys[i]=avalue_.derivedaptr

                if isinstance(value, PyXdmValue) or isinstance(value, PyXdmAtomicValue) or isinstance(value, PyXdmItem) or isinstance(value, PyXdmNode) or isinstance(value, PyXdmMap) or isinstance(value, PyXdmArray) or isinstance(value, PyXdmFunctionItem):
                     value_ = value
                     c_values[i] = value_.thisvptr

                else:
                       raise Exception("Error in making PyXdmMap")

            else:
                 raise Exception("Error in the making of the PyXdmMap - wrong key type")
            i +=1




        if len_ == 0:
            return None

        newMap_ = PyXdmMap()
        newMap_.derivedmmptr = newMap_.derivedfptr = newMap_.derivedptr = newMap_.thisvptr =  self.thisptr.makeMap3(c_keys, c_values, len_)
        if newMap_.derivedmmptr == NULL:
            return None
        return newMap_


    def make_map2(self, dict dataMap):
        """
        make_map2(self, dict_dataMap)
        Make an XdmMap whose members are from a dict type. Members are key, value pair (str, XdmValue).
        The factory method create_xdm_dict(proc, mmap) can be used to create pairs of type (str, XdmValue) from primitive types, which
        then can be used as input to this function make_map.
        Args:
            dataMap (dict): Dictionary of  (str, XdmValue) pairs
        Returns:
            PyAXdmMap: The corresponding value
        """
        cdef map[string, saxoncClasses.XdmValue *] c_dataMap
        cdef PyXdmValue value_
        cdef PyXdmAtomicValue avalue_
        cdef PyXdmAtomicValue key_
        cdef const char * c_key = NULL
        cdef char * c_key_str = NULL
        cdef PyXdmMap newMap_ = None
        '''global parametersDict
        if kwds is not None:
                parametersDict = kwds '''
        for (key, value) in dataMap.items():

            if isinstance(key, str):
                 key_str = key.encode('UTF-8')
                 py_key_str = key.encode('UTF-8') if key is not None else None
                 c_key_str = py_key_str if key_str is not None else ""
                 c_key = c_key_str


                 if isinstance(value, PyXdmValue):
                     value_ = value

                     c_dataMap[c_key] = <saxoncClasses.XdmValue *> value_.thisvptr
                 elif isinstance(value, PyXdmAtomicValue):

                     avalue_ = value

                     c_dataMap[c_key] = <saxoncClasses.XdmValue *> avalue_.derivedaptr


                 else:
                       raise Exception("Error in making PyXdmMap")

            else:
                 raise Exception("Error in the making of the PyXdmMap - wrong key type")




        if len(dataMap) == 0:
            print("make_map is none")
            return None

        newMap_ = PyXdmMap()
        newMap_.derivedmmptr = newMap_.derivedfptr = newMap_.derivedptr = newMap_.thisvptr =  self.thisptr.makeMap2(c_dataMap)
        if newMap_.derivedmmptr == NULL:
            return None
        return newMap_


    def set_catalog(self, str file_name, is_tracing=False):
        """
        set_ctalog(self, name, value)
        Set the XML catalog to be used in Saxon

        Args:
            file_name (str): The file name for the XML catalog
            is_tracing (bool):Customize the resolver to write messages to the Saxon logging destination
        """
        cdef const char * c_filename_string = NULL
        '''make_c_str(file_name)'''
        py_name_string = file_name.encode('UTF-8') if file_name is not None else None
        c_filename_string = py_name_string if file_name is not None else ""
        cdef bool isTracing = is_tracing
        if c_filename_string is not NULL:
            self.thisptr.setCatalog(c_filename_string, isTracing)


    def get_string_value(self, PyXdmItem item):
        """
        get_string_value(self, PyXdmItem item)
        Create an QName Xdm value from string representation in clark notation
        Args:
            str_ (str): The value given in a string form in clark notation. {uri}local
        Returns:
            PyAtomicValue: The corresonding value
        """
        return make_py_str(self.thisptr.getStringValue(item.derivedptr))

    def parse_xml(self, **kwds):
        """
        parse_xml(self, **kwds)
        Parse a lexical representation, source file or uri of the source document and return it as an Xdm Node
        Args:
            **kwds : The possible keyword arguments must be one of the follow (xml_file_name|xml_text|xml_uri)
        Returns:
            PyXdmNode: The Xdm Node representation of the XML document
        Raises:
            Exception: Error if the keyword argument is not one of xml_file_name|xml_text|xml_uri.
        """
        py_error_message = "Error: parseXml should only contain one of the following keyword arguments: (xml_file_name|xml_text|xml_uri)"
        if len(kwds) != 1:
          raise Exception(py_error_message)
        cdef PyXdmNode val = None
        cdef str py_value = None
        cdef char * c_xml_string = NULL

        if "xml_text" in kwds:
          py_value = kwds["xml_text"]
          if py_value is None:
              raise Exception("XML text is None")
          '''c_xml_string = make_c_str(py_value)'''
          py_text_string = py_value.encode('UTF-8') if py_value is not None else None
          c_xml_string = py_text_string if py_value is not None else ""
          if c_xml_string == NULL:
              raise Exception("Error converting XML text")
          val = PyXdmNode()
          val.derivednptr =  val.derivedptr = val.thisvptr =self.thisptr.parseXmlFromString(c_xml_string, NULL)
          if val.derivednptr == NULL:
              return None
          return val
        elif "xml_file_name" in kwds:
          py_value  = kwds["xml_file_name"]

          '''if py_value  is None or isfile(py_value) == False or isfile(make_py_str(self.thisptr.getcwd())+"/"+py_value) == False:
            raise Exception("XML file does not exist")'''
          '''c_xml_string = make_c_str(py_value)'''
          py_value_string = py_value.encode('UTF-8') if py_value is not None else None
          c_xml_string = py_value_string if py_value is not None else ""
          val = PyXdmNode()
          val.derivednptr = val.derivedptr = val.thisvptr = self.thisptr.parseXmlFromFile(c_xml_string, NULL)
          if val.derivednptr is NULL:
              return None
          return val
        elif "xml_uri" in kwds:
          py_value = kwds["xml_uri"]
          py_uri_string = py_value.encode('UTF-8') if py_value is not None else None
          c_xml_string = py_uri_string if py_value is not None else ""
          val = PyXdmNode()
          val.derivednptr = val.derivedptr = val.thisvptr = self.thisptr.parseXmlFromUri(c_xml_string, NULL)
          if val.derivednptr is NULL:
              return None
          return val
        else:
           raise Exception(py_error_message)

    @property
    def exception_occurred(self):
        """
        exception_occurred(self)
        Property to check if an exception has occurred internally within SaxonC
        Returns:
            boolean: True or False if an exception has been reported internally in SaxonC
        """
        return self.thisptr.exceptionOccurred()

    def exception_clear(self):
        """
        exception_clear(self)
        Clear any exception thrown internally in SaxonC.
        """
        self.thisptr.exceptionClear()

    @property
    def error_message(self):
       """
       error_message(self)
       The PySaxonProcessor may have a number of errors reported against it. Get the error message if there are any errors

       Returns:
           str: The message of the exception. Return None if the exception does not exist.
       """
       cdef const char* c_string = self.thisptr.getErrorMessage()
       ustring = make_py_str(c_string)
       return ustring


cdef class PyDocumentBuilder:
    """
    A PyDocumentBuilder holds properties controlling how a Saxon document tree should be built, and
     * provides methods to invoke the tree construction.
     * <p>This class has no public constructor.  To construct a DocumentBuilder,
     * use the factory method SaxonProcessor.newDocumentBuilder().
    """
    cdef saxoncClasses.DocumentBuilder *thisdbptr      # hold a C++ instance which we're wrapping
    def __cinit__(self):
       """Default constructor """
       self.thisdbptr = NULL
    def __dealloc__(self):
       if self.thisdbptr != NULL:
          del self.thisdbptr

    @property
    def line_numbering(self):
        """bool: true if line numbering is enabled"""
        return self.thisdbptr.isLineNumbering()

    @line_numbering.setter
    def line_numbering(self, value):
        """bool: option true if line numbers are to be maintained, false otherwise."""
        cdef bool c_line
        c_line = value
        self.thisdbptr.setLineNumbering(c_line)


    @property
    def dtd_validation(self):
        """bool: Ask whether DTD validation is to be applied to documents loaded using this PyDocumentBuilder"""
        return self.thisdbptr.isDTDValidation()

    @dtd_validation.setter
    def dtd_validation(self, value):
        """bool: Set whether DTD validation should be applied to documents loaded using this PyDocumentBuilder"""
        cdef bool c_dtd
        c_dtd = value
        self.thisdbptr.setDTDValidation(c_dtd)

    def set_schema_validator(self, PySchemaValidator val):
       """Set the source document for the transformation.
       Args:
           **kwds: Keyword argument can only be one of the following: file_name|xdm_node
       Raises:
           Exception: Exception is raised if keyword argument is not one of file_name or node.
        """
       self.thisdbptr.setSchemaValidator(val.thissvptr)


    def get_schema_validator(self):
       """
       get_schema_validator(self)
       Get the SchemaValidator used to validate documents loaded using this DocumentBuilder
       Returns:
           PySchemaValidator: if one has been set; otherwise None.
       """
       cdef PySchemaValidator val = PySchemaValidator()
       val.thissvptr = self.thisdbptr.getSchemaValidator()
       if val.thissvptr is NULL:
           raise Exception("Error: Saxon Processor is not licensed for schema processing!")
       return val

    @property
    def base_uri(self):
       """
       base_uri(self)
       Base uri Property. Get the Base URI for the node, that is, the URI used for resolving a relative URI contained in the node.
       This will be the same as the System ID unless xml:base has been used. Where the node does not have a base URI of its own,
       the base URI of its parent node is returned.
       Returns:
           str: String value of the base uri for this node. This may be NULL if the base URI is unknown, including the case
                where the node has no parent.
       """
       return make_py_str(self.thisdbptr.getBaseUri())

    def set_base_uri(self, base_uri):
       """
       set_base_uri(self, base_uri)
       Get the base URI of documents loaded using this PyDocumentBuilder when no other URI is available.
       Args:
           base_uri (str): the base output URI
       """
       py_uri_string = base_uri.encode('UTF-8') if base_uri is not None else None
       cdef char * c_uri = py_uri_string if base_uri is not None else ""
       self.thisdbptr.setBaseUri(c_uri)

    def parse_xml(self, **kwds):
        """
        parse_xml(self, **kwds)
        Parse a lexical representation, source file or uri of the source document and return it as an Xdm Node
        Args:
            **kwds : The possible keyword arguments must be one of the follow (xml_file_name|xml_text|xml_uri)
        Returns:
            PyXdmNode: The Xdm Node representation of the XML document
        Raises:
            Exception: Error if the keyword argument is not one of xml_file_name|xml_text|xml_uri.
        """
        py_error_message = "Error: parseXml should only contain one of the following keyword arguments: (xml_file_name|xml_text|xml_uri)"
        if len(kwds) != 1:
          raise Exception(py_error_message)
        cdef PyXdmNode val = None
        cdef py_value = None
        cdef char * c_xml_string = NULL
        if "xml_text" in kwds:
          py_value = kwds["xml_text"]
          py_xml_text_string = py_value.encode('UTF-8') if py_value is not None else None
          c_xml_string = py_xml_text_string if py_value is not None else ""
          val = PyXdmNode()
          val.derivednptr = val.derivedptr = val.thisvptr = self.thisdbptr.parseXmlFromString(c_xml_string)
          return val
        elif "xml_file_name" in kwds:
          py_value = kwds["xml_file_name"]
          py_filename_string = py_value.encode('UTF-8') if py_value is not None else None
          '''if py_filename_string  is None or isfile(py_filename_string) == False:
            raise Exception("XML file does not exist")'''
          c_xml_string = py_filename_string if py_value is not None else ""
          val = PyXdmNode()
          val.derivednptr = val.derivedptr = val.thisvptr = self.thisdbptr.parseXmlFromFile(c_xml_string)
          return val
        elif "xml_uri" in kwds:
          py_value = kwds["xml_uri"]
          py_uri_string = py_value.encode('UTF-8') if py_value is not None else None
          c_xml_string = py_uri_string if py_value is not None else ""
          val = PyXdmNode()
          val.derivednptr = val.derivedptr = val.thisvptr = self.thisdbptr.parseXmlFromUri(c_xml_string)
          return val
        else:
           raise Exception(py_error_message)



parametersDict = None


cdef class PyXslt30Processor:
     """An PyXslt30Processor represents factory to compile, load and execute a stylesheet.
     It is possible to cache the context and the stylesheet in the PyXslt30Processor """
     cdef saxoncClasses.Xslt30Processor *thisxptr      # hold a C++ instance which we're wrapping

     def __cinit__(self):
        """Default constructor """
        self.thisxptr = NULL
     def __dealloc__(self):
        if self.thisxptr != NULL:
           del self.thisxptr
     def set_cwd(self, cwd):
        """
        set_cwd(self, cwd)
        Set the current working directory.
        Args:
            cwd (str): current working directory
        """
        cdef char * c_cwd = NULL
        '''make_c_str(cwd)'''
        py_cwd_string = cwd.encode('UTF-8') if cwd is not None else None
        c_cwd = py_cwd_string if cwd is not None else ""
        self.thisxptr.setcwd(c_cwd)

     def set_jit_compilation(self, bool jit):
        """
        set_jit_compilation(self, jit)
        Say whether just-in-time compilation of template rules should be used.
        Args:
            jit (bool): True if just-in-time compilation is to be enabled. With this option enabled,
                static analysis of a template rule is deferred until the first time that the
                template is matched. This can improve performance when many template
                rules are rarely used during the course of a particular transformation; however,
                it means that static errors in the stylesheet will not necessarily cause the
                compile(Source) method to throw an exception (errors in code that is
                actually executed will still be notified but this may happen after the compile(Source)
                method returns). This option is enabled by default in Saxon-EE, and is not available
                in Saxon-HE or Saxon-PE.
                Recommendation: disable this option unless you are confident that the
                stylesheet you are compiling is error-free.
        """
        cdef bool c_jit
        c_jit = jit
        self.thisxptr.setJustInTimeCompilation(c_jit)
        #else:
        #raise Warning("setJustInTimeCompilation: argument must be a boolean type. JIT not set")

     def set_parameter(self, name, PyXdmValue value):
        """
        set_parameter(self, PyXdmValue value)
        Set the value of a stylesheet parameter
        Args:
            name (str): the name of the stylesheet parameter, as a string. For namespaced parameter use the JAXP solution i.e. "{uri}name
            value (PyXdmValue): the value of the stylesheet parameter, or NULL to clear a previously set value
        """
        cdef const char * c_str = NULL
        '''make_c_str(name)'''
        py_name_string = name.encode('UTF-8') if name is not None else None
        c_str = py_name_string if name is not None else ""
        if c_str is not NULL:
            value.thisvptr.incrementRefCount()
            '''print("set_parameter called")'''
            self.thisxptr.setParameter(c_str, value.thisvptr)
     def get_parameter(self, name):
        """
        get_parameter(self, name)
        Get a parameter value by a given name
        Args:
            name (str): The name of the stylesheet parameter
        Returns:
            PyXdmValue: The Xdm value of the parameter
        """
        py_name_string = name.encode('UTF-8') if name is not None else None
        cdef char * c_name = py_name_string if name is not None else ""
        cdef PyXdmValue val = PyXdmValue()
        val.thisvptr = self.thisxptr.getParameter(c_name)
        return val
     def remove_parameter(self, name):
        """
        remove_parameter(self, name)
        Remove the parameter given by name from the PyXslt30Processor. The parameter will not have any affect on the stylesheet if it has not yet been executed
        Args:
            name (str): The name of the stylesheet parameter
        Returns:
            bool: True if the removal of the parameter has been successful, False otherwise.
        """
        py_name_string = name.encode('UTF-8') if name is not None else None
        cdef char * c_name = py_name_string if name is not None else ""
        return self.thisxptr.removeParameter(c_name)

     def clear_parameters(self):
        """
        clear_parameter(self)
        Clear all parameters set on the processor for execution of the stylesheet
        """
        self.thisxptr.clearParameters()


     def compile_stylesheet(self, **kwds):
        """
        compile_stylesheet(self, **kwds)
        Compile a stylesheet  received as text, uri or as a node object. The term "compile" here indicates
        that the stylesheet is converted into an executable
         form. The compilation uses a snapshot of the properties of the <code>Xslt30Processor</code> at the
         time this method is invoked. It is also possible to save the compiled
        stylesheet (SEF file) given the option 'save' and 'output_file'.
        Get the stylesheet associated via the xml-stylesheet processing instruction (see
        http://www.w3.org/TR/xml-stylesheet/) with the document
        document specified in the source parameter, and that match
        the given criteria.  If there are several suitable xml-stylesheet
        processing instructions, then the returned Source will identify
        a synthesized stylesheet module that imports all the referenced
        stylesheet module.
        Args:
            **kwds: Possible keyword arguments stylesheet_text (str), stylesheet_file (str),
            associated_file (str) or stylesheet_node (PyXdmNode). Also possible to add the options
            save (boolean) and output_file, which creates an exported stylesheet to file (SEF).
        Returns:
            PyXsltExecutable - which represents the compiled stylesheet. The XsltExecutable
             is immutable and thread-safe; it may be used to run multiple transformations, in series or concurrently.
        Example:
            1. executable = xsltproc.compile_stylesheet(stylesheet_text="<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform' version='2.0'>
                                             <xsl:param name='values' select='(2,3,4)' /><xsl:output method='xml' indent='yes' />
                                             <xsl:template match='*'><output><xsl:value-of select='//person[1]'/>
                                             <xsl:for-each select='$values' >
                                               <out><xsl:value-of select='. * 3'/></out>
                                             </xsl:for-each></output></xsl:template></xsl:stylesheet>")
            2. executable = xsltproc.compile_stylesheet(stylesheet_file="test1.xsl", save=True, output_file="test1.sef")
            3. executable = xsltproc.compile(associated_file="foo.xml")
        """
        py_error_message = "CompileStylesheet should only be one of the keyword option: (associated_file|stylesheet_text|stylesheet_file|stylesheet_node), also in allowed in addition the optional keyword 'save' boolean with the keyword 'outputfile' keyword"
        if len(kwds) >3:
          raise Exception(py_error_message)
        cdef char * c_outputfile
        cdef char * c_stylesheet
        py_output_string = None
        py_stylesheet_string = None
        cdef PyXsltExecutable executable
        cdef saxoncClasses.XsltExecutable * cexecutable = NULL
        py_save = False
        cdef int option = 0
        cdef PyXdmNode py_xdmNode = None
        if kwds.keys() >= {"stylesheet_text", "stylesheet_file"}:
          raise Exception(py_error_message)
        if kwds.keys() >= {"stylesheet_text", "stylesheet _node"}:
          raise Exception(py_error_message)
        if kwds.keys() >= {"stylesheet_node", "stylesheet_file"}:
          raise Exception(py_error_message)
        if ("save" in kwds) and kwds["save"]==True:
          del kwds["save"]
          if "output_file" not in kwds:
            raise Exception("Output file option not in keyword arugment for compile_stylesheet")
          py_output_string = kwds["output_file"]
          '''c_outputfile = make_c_str(py_output_string)'''
          py_output_sstring = py_output_string.encode('UTF-8') if py_output_string is not None else None
          c_outputfile = py_output_sstring if py_output_string is not None else ""
          if "stylesheet_text" in kwds:
            py_stylesheet_string = kwds["stylesheet_text"]
            '''c_stylesheet = make_c_str(py_stylesheet_string)'''
            py_s_string = py_stylesheet_string.encode('UTF-8') if py_stylesheet_string is not None else None
            c_stylesheet = py_s_string if py_stylesheet_string is not None else ""
            self.thisxptr.compileFromStringAndSave(c_stylesheet, c_outputfile)
          elif "stylesheet_file" in kwds:
            py_stylesheet_string = kwds["stylesheet_file"]
            '''if py_stylesheet_string  is None or isfile(py_stylesheet_string) == False:
              raise Exception("Stylesheet file does not exist")'''
            '''c_stylesheet = make_c_str(py_stylesheet_string)'''
            py__string = py_stylesheet_string.encode('UTF-8') if py_stylesheet_string is not None else None
            c_stylesheet = py__string if py_stylesheet_string is not None else ""
            self.thisxptr.compileFromFileAndSave(c_stylesheet, c_outputfile)
          elif "stylesheet_node" in kwds:
            py_xdmNode = kwds["stylesheet_node"]
            #if not isinstance(py_value, PyXdmNode):
              #raise Exception("StylesheetNode keyword arugment is not of type XdmNode")
            #value = PyXdmNode(py_value)
            self.thisxptr.compileFromXdmNodeAndSave(py_xdmNode.derivednptr, c_outputfile)
          else:
            raise Exception(py_error_message)
        else:
          if "stylesheet_text" in kwds:
            py_stylesheet_string = kwds["stylesheet_text"]
            '''c_stylesheet = make_c_str(py_stylesheet_string)'''
            py__string = py_stylesheet_string.encode('UTF-8') if py_stylesheet_string is not None else None
            c_stylesheet = py__string if py_stylesheet_string is not None else ""

            executable = PyXsltExecutable()
            cexecutable =  self.thisxptr.compileFromString(c_stylesheet)
            if cexecutable is NULL:
                return None
            executable.thisxptr = cexecutable
            return executable
          elif "stylesheet_file" in kwds:
            py_stylesheet_string = kwds["stylesheet_file"]

            py__string = py_stylesheet_string.encode('UTF-8') if py_stylesheet_string is not None else None
            c_stylesheet = py__string if py_stylesheet_string is not None else ""

            '''TODO handle cwd or let java do the complete checking
            if py_stylesheet_string  is None or isfile(py_stylesheet_string) == False:
              raise Exception("Stylesheet file does not exist: "+ py_stylesheet_string)
            c_stylesheet = make_c_str(py_stylesheet_string)'''

            executable = PyXsltExecutable()
            cexecutable =  self.thisxptr.compileFromFile(c_stylesheet)
            if cexecutable is NULL:
                return None
            executable.thisxptr = cexecutable
            return executable
          elif "associated_file" in kwds:
            py_stylesheet_string = kwds["associated_file"]
            '''if py_stylesheet_string  is None or isfile(py_stylesheet_string) == False:
              raise Exception("Stylesheet file does not exist")
            c_stylesheet = make_c_str(py_stylesheet_string)'''

            py__string = py_stylesheet_string.encode('UTF-8') if py_stylesheet_string is not None else None
            c_stylesheet = py__string if py_stylesheet_string is not None else ""

            executable = PyXsltExecutable()
            cexecutable =  self.thisxptr.compileFromAssociatedFile(c_stylesheet)
            if cexecutable is NULL:
                return None
            executable.thisxptr = cexecutable
            return executable
          elif "stylesheet_node" in kwds:
            py_xdmNode = kwds["stylesheet_node"]
            #if not isinstance(py_value, PyXdmNode):
              #raise Exception("StylesheetNode keyword arugment is not of type XdmNode")
            #value = PyXdmNode(py_value)
            executable = PyXsltExecutable()
            cexecutable =  self.thisxptr.compileFromXdmNode(py_xdmNode.derivednptr)
            if cexecutable is NULL:
                return None
            executable.thisxptr = cexecutable
            return executable
          else:
            raise Exception(py_error_message)

     def transform_to_string(self, **kwds):
        """
        transform_to_string(self, **kwds)
        Execute transformation to string.
        Args:
            **kwds: Possible arguments: source_file (str) or xdm_node (PyXdmNode). Other allowed argument: stylesheet_file (str)
                                        and base_output_uri (str) which is used for for resolving relative URIs in the href
                                        attribute of the xsl:result-document instruction.
        Example:
            1) result = xsltproc.transform_to_string(source_file="cat.xml", stylesheet_file="test1.xsl")
            2) xsltproc.set_source("cat.xml")\r
               result = xsltproc.transform_to_string(stylesheet_file="test1.xsl")
            3) node = saxon_proc.parse_xml(xml_text="<in/>")\r
               result = xsltproc.transform_to_string(stylesheet_file="test1.xsl", xdm_node= node)
        """
        cdef char * c_sourcefile = NULL
        cdef char * c_stylesheet = NULL
        cdef PyXdmNode node_ = None
        cdef py_value = None
        cdef py_value2 = None
        if len(kwds) == 0:
            raise Warning("Warning: transform_to_string should only contain the following keyword arguments: (source_file|xdm_node, stylesheet_file)")

        if "source_file" in kwds:
            py_value2 = kwds["source_file"]
            py_source_string = py_value2.encode('utf-8') if py_value2 is not None else None
            c_sourcefile = py_source_string if py_value2 is not None else ""

        if "base_output_uri" in kwds:
            py_value = kwds["base_output_uri"]
            '''c_base_output_uri = make_c_str(py_value)'''
            py_base_string = py_value.encode('utf-8') if py_value is not None else None
            c_base_output_uri = py_base_string if py_value is not None else ""
            self.thisxptr.setBaseOutputURI(c_base_output_uri)

        if "output_file" in kwds:
            py_value3 = kwds["output_file"]
            '''c_sourcefile = make_c_str(py_value2)'''

            py_output_string = py_value3.encode('utf-8') if py_value3 is not None else None
            c_outputfile = py_output_string if py_value3 is not None else ""

        if "stylesheet_file" in kwds:
            py_value1 = kwds["stylesheet_file"]
            py_string_string = py_value1.encode('utf-8') if py_value1 is not None else None
            c_stylesheet = py_string_string if py_value1 is not None else ""


        cdef const char* c_string
        c_string = self.thisxptr.transformFileToString(c_sourcefile, c_stylesheet)
        return make_py_str(c_string)

     def transform_to_file(self, **kwds):
        """
        transform_to_file(self, **kwds)
        Execute transformation to a file. It is possible to specify the as an argument or using the set_output_file method.
        Args:
            **kwds: Possible optional arguments: source_file (str), stylesheet_file (str), output_file (str)
                                                 and base_output_uri (str) which is used for for resolving relative URIs in the href attribute of the
                                                 xsl:result-document instruction.
        Example:
            1) xsltproc.transform_to_file(source_file="cat.xml", stylesheet_file="test1.xsl", output_file="result.xml")
            2) xsltproc.set_source("cat.xml")\r
               xsltproc.setoutput_file("result.xml")\r
               xsltproc.transform_to_file(stylesheet_file="test1.xsl")
            3) node = saxon_proc.parse_xml(xml_text="<in/>")\r
               xsltproc.transform_to_file(output_file="result.xml", stylesheet_file="test1.xsl", xdm_node= node)
        """
        cdef char * c_sourcefile = NULL
        cdef char * c_outputfile = NULL
        cdef char * c_base_output_uri = NULL
        cdef char * c_stylesheet = NULL
        cdef PyXdmNode node_ = None
        for key, value in kwds.items():
                if isinstance(value, str):
                        if key == "source_file":
                                py_source_string = value.encode('utf-8') if value is not None else None
                                c_sourcefile = py_source_string if value is not None else ""
                        elif key == "base_output_uri":
                                '''c_base_output_uri = make_c_str(value)'''
                                py_string_string = value.encode('utf-8') if value is not None else None
                                c_base_output_uri = py_string_string if value is not None else ""
                                self.thisxptr.setBaseOutputURI(c_base_output_uri)
                        elif key == "output_file":
                                '''c_outputfile = make_c_str(value)'''
                                py_string_string = value.encode('utf-8') if value is not None else None
                                c_outputfile = py_string_string if value is not None else ""
                        elif key == "stylesheet_file":
                                '''c_stylesheet = make_c_str(value)'''
                                py_string_string = value.encode('utf-8') if value is not None else None
                                c_stylesheet = py_string_string if value is not None else ""

        self.thisxptr.transformFileToFile(c_sourcefile, c_stylesheet, c_outputfile)

     def transform_to_value(self, **kwds):
        """
        transform_to_value(self, **kwds)
        Execute transformation to an PyXdmNode object
        Args:
            **kwds: Possible optional arguments: source_file (str), stylesheet_file (str)
                                                 and base_output_uri (str) which is used for for resolving relative URIs in the href attribute
                                                 of the xsl:result-document instruction.
        Returns:
            PyXdmNode: Result of the transformation as an PyXdmNode object
        Example:
            1) node = xsltproc.transform_to_value(source_file="cat.xml", stylesheet_file="test1.xsl")
            2) xsltproc.set_source("cat.xml")\r
               node = xsltproc.transform_to_value(stylesheet_file="test1.xsl")
        """
        cdef const char * c_sourcefile = NULL
        cdef char * c_stylesheet = NULL
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              '''c_sourcefile = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_sourcefile = py_string_string if value is not None else ""
            elif key == "base_output_uri":
              '''c_base_output_uri = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_base_output_uri = py_string_string if value is not None else ""
              self.thisxptr.setBaseOutputURI(c_base_output_uri)
            elif key == "stylesheet_file":
               '''c_stylesheet = make_c_str(value)'''
               py_string_string = value.encode('utf-8') if value is not None else None
               c_stylesheet = py_string_string if value is not None else ""
        cdef PyXdmValue val = None
        cdef PyXdmAtomicValue aval = None
        cdef PyXdmNode nval = None
        cdef PyXdmFunctionItem fval = None
        cdef PyXdmMap mval = None
        cdef PyXdmArray aaval = None
        cdef saxoncClasses.XdmValue * xdmValue = NULL
        xdmValue = self.thisxptr.transformFileToValue(c_sourcefile, c_stylesheet)
        if xdmValue is NULL:
            return None
        cdef type_ = xdmValue.getType()
        if type_== 4:
            aval = PyXdmAtomicValue()
            aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>xdmValue
            return aval
        elif type_ == 3:
            nval = PyXdmNode()
            nval.derivednptr = nval.derivedptr = nval.thisvptr = <saxoncClasses.XdmNode*>xdmValue
            return nval
        elif type_ == 5:
            fval = PyXdmFunctionItem()
            fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>xdmValue
            return fval
        elif type_ == 6:
            mval = PyXdmMap()
            mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>xdmValue
            return mval
        elif type_ == 7:
            aaval = PyXdmArray()
            aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>xdmValue
            return aaval
        else:
            val = PyXdmValue()
            val.thisvptr = xdmValue
            return val

     @property
     def exception_occurred(self):
        """
        exception_occurred(self)
        Property to check for pending exceptions without creating a local reference to the exception object
        Returns:
            boolean: True when there is an exception thrown; otherwise return False
        """
        return self.thisxptr.exceptionOccurred()

     def exception_clear(self):
        """
        exception_clear(self)
        Clear any exception thrown
        """
        self.thisxptr.exceptionClear()

     @property
     def error_message(self):
        """
        error_message(self)
        A transformation may have a number of errors reported against it. This property returns an error message if there are any errors

        Returns:
            str: The message of the exception. Return None if the exception does not exist.
        """
        cdef const char* c_string = self.thisxptr.getErrorMessage()
        ustring = make_py_str(c_string)
        return ustring

     @property
     def error_code(self):
        """
        error_code(self)
        A transformation may have a number of errors reported against it. This property returns the error code if there are any errors

        Returns:
            str: The error code associated with the exception. Return None if the exception does not exist.
        """
        cdef const char* c_string = self.thisxptr.getErrorCode()
        ustring = make_py_str(c_string)
        return ustring





cdef class PyXsltExecutable:
     """An PyXsltExecutable represents the compiled form of a stylesheet.
      An XsltExecutable is created by using one of the compile methods on the
       PyXsltProcessor """
     cdef saxoncClasses.XsltExecutable *thisxptr      # hold a C++ instance which we're wrapping
     def __cinit__(self):
        """Default constructor """
        self.thisxptr = NULL
     def __dealloc__(self):
        if self.thisxptr != NULL:
           del self.thisxptr
     def set_cwd(self, cwd):
        """
        set_cwd(self, cwd)
        Set the current working directory.
        Args:
            cwd (str): current working directory
        """
        cdef char * c_cwd = NULL
        '''make_c_str(cwd)'''
        py_string_string = cwd.encode('utf-8') if cwd is not None else None
        c_cwd = py_string_string if cwd is not None else ""
        self.thisxptr.setcwd(c_cwd)

     def clone(self):
         """
         clone(self)
         Create a clone object of this PyXsltExecutable object
         Returns:
               PYXsltExecutable: copy of this object
         """
         cdef PyXsltExecutable executable = PyXsltExecutable()
         executable.thisxptr = self.thisxptr.clone()
         return executable

     def set_initial_mode(self, name):
        """
        set_initial_mode(self, base_uri)
        Set the initial mode for the transformation
        Args:
            name (str): he EQName of the initial mode. Two special values are recognized, in the
                 reserved XSLT namespace:
                 xsl:unnamed to indicate the mode with no name, and xsl:default to indicate the
                 mode defined in the stylesheet header as the default mode.
                 The value null also indicates the default mode (which defaults to the unnamed
                 mode, but can be set differently in an XSLT 3.0 stylesheet).
        """
        cdef char * c_name = NULL
        '''make_c_str(name)'''
        py_string_string = name.encode('utf-8') if name is not None else None
        c_name = py_string_string if name is not None else ""
        self.thisxptr.setInitialMode(c_name)

     def set_base_output_uri(self, base_uri):
        """
        set_base_output_uri(self, base_uri)
        Set the base output URI. The default is the base URI of the principal output
        of the transformation. If a base output URI is supplied using this function then it takes precedence
        over any base URI defined in the principal output, and
        it may cause the base URI of the principal output to be modified in situ.
        The base output URI is used for resolving relative URIs in the 'href' attribute
        of the xsl:result-document instruction; it is accessible to XSLT stylesheet
        code using the XPath current-output-uri() function.
        Args:
            base_uri (str): the base output URI
        """
        cdef char * c_uri = NULL
        '''make_c_str(base_uri)'''
        py_string_string = base_uri.encode('utf-8') if base_uri is not None else None
        c_uri = py_string_string if base_uri is not None else ""
        self.thisxptr.setBaseOutputURI(c_uri)

     def set_global_context_item(self, **kwds):
        """Set the global context item for the transformation.
        Args:
            **kwds: Keyword argument can only be one of the following: file_name|xdm_item
        Raises:
            Exception: Exception is raised if keyword argument is not one of file_name or an Xdm item.
        """
        py_error_message = "Error: set_global_context_item should only contain one of the following keyword arguments: (file_name|xdm_item)"
        if len(kwds) != 1:
          raise Exception(py_error_message)
        cdef py_value = None
        cdef py_value_string = None
        cdef char * c_source =  NULL
        cdef PyXdmItem xdm_item = None
        if "file_name" in kwds:
            py_value = kwds["file_name"]
            '''c_source = make_c_str(py_value)'''
            py_string_string = py_value.encode('utf-8') if py_value is not None else None
            c_source = py_string_string if py_value is not None else ""
            self.thisxptr.setGlobalContextFromFile(c_source)
        elif "xdm_item" in kwds:
            if isinstance(kwds["xdm_item"], PyXdmItem):
                xdm_item = kwds["xdm_item"]
                self.thisxptr.setGlobalContextItem(xdm_item.derivedptr)
            else:
                raise Exception("xdm_item value must be of type PyXdmItem")
        else:
          raise Exception(py_error_message)
     def set_initial_match_selection(self, **kwds):
        """
        set_initial_match_selection(self, **kwds)
        The initial filename to which templates are to be applied (equivalent to the select attribute of xsl:apply-templates).
        Args:
            **kwds: Keyword argument can only be one of the following: file_name|xdm_value
        Raises:
            Exception: Exception is raised if keyword argument is not one of file_name or XdmValue.
        """
        py_error_message = "Error: set_initial_match_selection should only contain one of the following keyword arguments: (file_name|xdm_value)"
        if len(kwds) != 1:
          raise Exception(py_error_message)
        cdef py_value = None
        cdef py_value_string = None
        cdef char * c_source  = NULL
        cdef PyXdmValue xdm_value = None
        cdef PyXdmAtomicValue avalue_
        cdef PyXdmItem ivalue_
        cdef PyXdmNode nvalue_
        cdef PyXdmValue value_
        if "file_name" in kwds:
            py_value = kwds["file_name"]
            '''c_source = make_c_str(py_value)'''
            py_string_string = py_value.encode('utf-8') if py_value is not None else None
            c_source = py_string_string if py_value is not None else ""
            self.thisxptr.setInitialMatchSelectionAsFile(c_source)
        elif "xdm_value" in kwds:
            value = kwds["xdm_value"]
            if value is not None:

                if isinstance(value, PyXdmValue):
                    value_ = value
                    value_.thisvptr.incrementRefCount()
                    self.thisxptr.setInitialMatchSelection(value_.thisvptr)
                elif  isinstance(value, PyXdmItem):
                    ivalue_ = value
                    ivalue_.derivedptr.incrementRefCount()
                    self.thisxptr.setInitialMatchSelection(<saxoncClasses.XdmValue *>  ivalue_.derivedptr)
                elif  isinstance(value, PyXdmNode):
                    nvalue_ = value
                    nvalue_.derivednptr.incrementRefCount()
                    self.thisxptr.setInitialMatchSelection(<saxoncClasses.XdmValue *>  nvalue_.derivednptr)
                elif  isinstance(value, PyXdmAtomicValue):
                    avalue_ = value
                    avalue_.derivedaptr.incrementRefCount()
                    self.thisxptr.setInitialMatchSelection(<saxoncClasses.XdmValue *> avalue_.derivedaptr)
                else:
                    raise Exception("Supplied value is not of the right type")

     def set_output_file(self, output_file):
        """
        set_output_file(self, output_file)
        Set the output file where the output of the transformation will be sent
        Args:
            output_file (str): The output file supplied as a str
        """
        cdef char * c_outputfile =  NULL
        '''make_c_str(output_file)'''
        py_string_string = output_file.encode('utf-8') if output_file is not None else None
        c_outputfile = py_string_string if output_file is not None else ""
        self.thisxptr.setOutputFile(c_outputfile)

     def set_result_as_raw_value(self, bool is_raw):
        """
        set_result_as_raw_value(self, is_raw)
        Set true if the return type of callTemplate, applyTemplates and transform methods is to return XdmValue, otherwise return XdmNode object with root Document node
        Args:
            is_raw (bool): True if returning raw result, i.e. XdmValue, otherwise return XdmNode
        """
        cdef bool c_raw
        c_raw = is_raw
        self.thisxptr.setResultAsRawValue(c_raw)
        #else:
        #raise Warning("setJustInTimeCompilation: argument must be a boolean type. JIT not set")

     def set_capture_result_documents(self, bool value, bool raw_result=False):
        """
        set_capture_result_documents(self, value, raw_result)
        Enable the capture of the result-document output into a dict. This overrides the default mechanism.
        If this option is enabled, then any document created using xsl:result-document is saved (as a PyXdmNode)
        in a dict object where it is accessible using the URI as a key. After the execution of the transformation
        a call on the get_result_documents method is required to get access to the result-documents in the map.
        It is also possible to capture the result-document as a raw result directly as an PyXdmValue, without
        constructing an XML tree, and without serialization. It corresponds to the serialization.
        Args:
            value (bool): true causes secondary result documents from the transformation to be saved in a map;
            false disables this option.
            raw_result (bool): true enables the handling of raw destination for resultsDocuments. If not
            supplied this can also be set on the setResultAsRawValue method. The setResultAsRawValue method
            has higher priority to this flag
        """
        cdef bool c_value
        c_value = value
        cdef bool c_raw_result
        c_raw_result = raw_result
        self.thisxptr.setCaptureResultDocuments(c_value, c_raw_result)

     def get_result_documents(self):
         """
         get_result_documents(self)
         Return the result-documents resulting from the execution of the stylesheet. Null is
         returned if the user has not enabled this feature via the method set_capture_result_documents

         Returns:
             dict [str, PyXdmValue]: Dict of the key, value pair. Indexed by the absolute URI of each result
             document, and the corresponding value is an PyXdmValue object containing the result document (as
             an in-memory tree, without serialization).
         """
         cdef map[string, saxoncClasses.XdmValue *] c_dataMap
         cdef dict p_dataMap = {}
         cdef PyXdmValue nval = None

         c_dataMap = self.thisxptr.getResultDocuments()

         cdef map[string, saxoncClasses.XdmValue *].iterator it = c_dataMap.begin()
         cdef int size = c_dataMap.size()

         cdef str key_str
         while(it != c_dataMap.end()):
             c_xdmNode =  dereference(it).second
             nval = PyXdmValue()
             nval.thisvptr = <saxoncClasses.XdmValue*>c_xdmNode
             key_str = make_py_str(dereference(it).first.c_str())
             p_dataMap[key_str] =  nval
             postincrement(it)

         return p_dataMap

     def set_parameter(self, name, value):
        """
        set_parameter(self, PyXdmValue value)
        Set the value of a stylesheet parameter
        Args:
            name (str): the name of the stylesheet parameter, as a string. For namespaced parameter use the JAXP solution i.e. "{uri}name
            value (PyXdmValue): the value of the stylesheet parameter, or NULL to clear a previously set value
        """
        cdef const char * c_str = NULL
        py_string_string = name.encode('utf-8') if name is not None else None
        c_str = py_string_string if name is not None else ""
        cdef PyXdmAtomicValue avalue_
        cdef PyXdmItem ivalue_
        cdef PyXdmNode nvalue_
        cdef PyXdmValue value_
        if c_str is not NULL:
            if  isinstance(value, PyXdmAtomicValue):
                avalue_ = value
                avalue_.derivedaptr.incrementRefCount()
                self.thisxptr.setParameter(c_str, <saxoncClasses.XdmValue *> avalue_.derivedaptr)
            elif  isinstance(value, PyXdmNode):
                nvalue_ = value
                nvalue_.derivedaptr.incrementRefCount()
                self.thisxptr.setParameter(c_str, <saxoncClasses.XdmValue *>  nvalue_.derivednptr)
            elif  isinstance(value, PyXdmItem):
                ivalue_ = value
                ivalue_.derivedaptr.incrementRefCount()
                self.thisxptr.setParameter(c_str, <saxoncClasses.XdmValue *>  ivalue_.derivedptr)
            elif isinstance(value, PyXdmValue):
                value_ = value
                value_.thisvptr.incrementRefCount()
                self.thisxptr.setParameter(c_str, value_.thisvptr)

     def get_parameter(self, name):
        """
        get_parameter(self, name)
        Get a parameter value by a given name
        Args:
            name (str): The name of the stylesheet parameter
        Returns:
            PyXdmValue: The Xdm value of the parameter
        """
        cdef char * c_name = NULL
        '''make_c_str(name)'''
        py_string_string = name.encode('utf-8') if name is not None else None
        c_name = py_string_string if name is not None else ""
        cdef PyXdmValue val = PyXdmValue()
        val.thisvptr = self.thisxptr.getParameter(c_name)
        return val
     def remove_parameter(self, name):
        """
        remove_parameter(self, name)
        Remove the parameter given by name from the PyXslt30Processor. The parameter will not have any affect on the stylesheet if it has not yet been executed
        Args:
            name (str): The name of the stylesheet parameter
        Returns:
            bool: True if the removal of the parameter has been successful, False otherwise.
        """
        cdef char * c_name = NULL
        '''make_c_str(name)'''
        py_string_string = name.encode('utf-8') if name is not None else None
        c_name = py_string_string if name is not None else ""
        return self.thisxptr.removeParameter(c_name)
     def set_property(self, name, value):
        """
        set_property(self, name, value)
        Set a property specific to the processor in use.
        Args:
            name (str): The name of the property
            value (str): The value of the property
        Example:
            XsltProcessor: set serialization properties (names start with '!' i.e. name "!method" -> "xml")\r
            'o':outfile name,\r
            'it': initial template,\r
            'im': initial mode,\r
            's': source as file name\r
            'm': switch on message listener for xsl:message instructions,\r
            'item'| 'node' : source supplied as an XdmNode object,\r
            'extc':Set the native library to use with Saxon for extension functions written in C/C++/PHP\r
        """
        cdef char * c_name = make_c_str(name)
        py_string_string = name.encode('utf-8') if name is not None else None
        c_name = py_string_string if name is not None else ""

        cdef char * c_value = NULL
        '''make_c_str(value)'''
        py_string_string = value.encode('utf-8') if value is not None else None
        c_value = py_string_string if value is not None else ""
        self.thisxptr.setProperty(c_name, c_value)

     def clear_parameters(self):
        """
        clear_parameter(self)
        Clear all parameters set on the processor for execution of the stylesheet
        """
        self.thisxptr.clearParameters()
     def clear_properties(self):
        """
        clear_properties(self)
        Clear all properties set on the processor
        """
        self.thisxptr.clearProperties()
     def set_initial_template_parameters(self, bool tunnel, dict kwds):
        """
        set_initial_template_parameters(self, bool tunnel, **kwds)
        Set parameters to be passed to the initial template. These are used
        whether the transformation is invoked by applying templates to an initial source item,
        or by invoking a named template. The parameters in question are the xsl:param elements
        appearing as children of the xsl:template element.
        TODO: To fix issue where we pass XdmValue object created directly in the function argument. This causes seg error
        e.g. set_initial_template_parameter(False, {a:saxonproc.make_integer_value(12)})
        Do the following instead:
        paramArr = {a:saxonproc.make_integer_value(12)}
        set_initial_template_parameter(False, paramArr)
        Args:
        	tunnel (bool): True if these values are to be used for setting tunnel parameters;
        	**kwds: the parameters to be used for the initial template supplied as an key-value pair.
        	False if they are to be used for non-tunnel parameters. The default is false.
        Example:
        	1)paramArr = {'a':saxonproc.make_integer_value(12), 'b':saxonproc.make_integer_value(5)}
                  xsltproc.set_initial_template_parameters(False, paramArr)
        """
        cdef map[string, saxoncClasses.XdmValue *] parameters
        cdef bool c_tunnel
        cdef string key_str
        c_tunnel = tunnel
        cdef PyXdmAtomicValue value_
        global parametersDict
        if kwds is not None:
                parametersDict = kwds
        for (key, value) in kwds.items():
                if isinstance(value, PyXdmAtomicValue):
                        value_ = value
                        key_str = key.encode('UTF-8')
                        value_.derivedptr.incrementRefCount()
                        parameters[key_str] = <saxoncClasses.XdmValue *> value_.derivedaptr
                else:
                        raise Exception("Initial template parameters can only be of type PyXdmValue")
        if len(kwds) > 0:
            self.thisxptr.setInitialTemplateParameters(parameters, c_tunnel)

     def set_save_xsl_Message(self, show, str file_name = None):
        """
        setup_xsl_message(self, **kwds)
        gives users the option to switch on or off the <code>xsl:message</code> feature. It is also possible
        to send the <code>xsl:message</code> outputs to file given by file name.
        """
        cdef char * c_file_name = NULL
        '''make_c_str(file_name)'''
        py_string_string = file_name.encode('utf-8') if file_name is not None else None
        c_file_name = py_string_string if file_name is not None else ""
        self.thisxptr.setSaveXslMessage(show, c_file_name)

     def export_stylesheet(self, str file_name):
        """
        export_stylesheet(self, file_name)
        Produce a representation of the compiled stylesheet, in XML form, suitable for
        distribution and reloading.
        """
        cdef char * c_file_name = NULL
        '''make_c_str(file_name)'''
        py_string_string = file_name.encode('utf-8') if file_name is not None else None
        c_file_name = py_string_string if file_name is not None else ""
        self.thisxptr.exportStylesheet(c_file_name)


     def transform_to_string(self, **kwds):
        """
        transform_to_string(self, **kwds)
        Execute transformation to string.
        Args:
            **kwds: Possible arguments: source_file (str) or xdm_node (PyXdmNode). Other allowed argument: base_output_uri (str)
            which is used for for resolving relative URIs in the href attribute of the xsl:result-document instruction.
        Example:
            1) result = executable.transform_to_string(source_file="cat.xml")
            2) executable.set_initial_match_selection(file_name="cat.xml")\r
               result = executable.transform_to_string(stylesheet_file="test1.xsl", xdm_node= node)
            3) node = saxon_proc.parse_xml(xml_text="<in/>")\r
               result = executable.transform_to_string(xdm_node= node)
        """
        cdef char * c_sourcefile = NULL
        cdef char * c_stylesheet = NULL
        cdef const char* c_string = NULL
        cdef const char * c_base_output_uri = NULL
        cdef PyXdmNode xdm_node = None
        cdef saxoncClasses.XdmNode * derivednptr = NULL
        if len(kwds) > 1:
          raise Warning("Warning: transform_to_string should only contain the following keyword arguments: (source_file|xdm_node, stylesheet_file)")
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              c_sourcefile = NULL
              '''make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_sourcefile = py_string_string if value is not None else ""
            elif key == "base_output_uri":
              c_base_output_uri = NULL
              '''make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_base_output_uri = py_string_string if value is not None else ""
              self.thisxptr.setBaseOutputURI(c_base_output_uri)
            else:
              raise Warning("Warning: transform_to_string should only contain the following keyword arguments: (source_file|xdm_node, stylesheet_file)")
          elif key == "xdm_node":

            if isinstance(value, PyXdmNode):
                xdm_node = value
          else:
            raise Warning("Warning: transform_to_string should only contain the following keyword arguments: (source_file|xdm_node, stylesheet_file)")
        if xdm_node is not None:
          c_string = self.thisxptr.transformToString(xdm_node.derivednptr)
          return make_py_str(c_string)
        else:
          c_string = self.thisxptr.transformFileToString(c_sourcefile)
        return make_py_str(c_string)

     def transform_to_file(self, **kwds):
        """
        transform_to_file(self, **kwds)
        Execute transformation to a file. It is possible to specify the as an argument or using the set_output_file method.
        Args:
            **kwds: Possible optional arguments: source_file (str) or xdm_node (PyXdmNode). Other allowed argument: output_file (str)
                                                 and base_output_uri (str) which is used for for resolving relative URIs in the href attribute of the
                                                 xsl:result-document instruction.
        Example:
            1) executable = xslt30_proc.compile_stylesheet(stylesheet_file="test1.xsl")
               executable.transform_to_file(source_file="cat.xml", output_file="result.xml")
            2) executable.set_initial_match_selection("cat.xml")\r
               executable.set_output_file("result.xml")\r
               executable.transform_to_file(stylesheet_file="test1.xsl")
            3) node = saxon_proc.parse_xml(xml_text="<in/>")\r
               executable = xslt30_proc.compile_stylesheet(stylesheet_file="test1.xsl")
               executable.transform_to_file(output_file="result.xml", xdm_node= node)
        """
        cdef char * c_sourcefile = NULL
        cdef char * c_outputfile = NULL
        cdef char * c_stylesheet = NULL
        cdef char * c_base_output_uri = NULL
        cdef PyXdmNode node_ = None
        for key, value in kwds.items():
                if isinstance(value, str):
                        if key == "source_file":
                                c_sourcefile = NULL
                                '''make_c_str(value)'''
                                py_string_string = value.encode('utf-8') if value is not None else None
                                c_sourcefile = py_string_string if value is not None else ""
                        elif key == "base_output_uri":
                                c_base_output_uri = NULL
                                '''make_c_str(value)'''
                                py_string_string = value.encode('utf-8') if value is not None else None
                                c_base_output_uri = py_string_string if value is not None else ""
                                self.thisxptr.setBaseOutputURI(c_base_output_uri)
                        elif key == "output_file":
                                c_outputfile = NULL
                                '''make_c_str(value)'''
                                py_string_string = value.encode('utf-8') if value is not None else None
                                c_outputfile = py_string_string if value is not None else ""
                                self.thisxptr.setOutputFile(c_outputfile)

        if "xdm_node" in kwds:
            py_value = kwds["xdm_node"]
            if(isinstance(py_value, PyXdmNode)):
                node_ = py_value
                self.thisxptr.transformToFile(node_.derivednptr)
            else:
                raise Exception("Keyword argument 'xdm_node' is not of type PyXdmNode")

        else:
            self.thisxptr.transformFileToFile(c_sourcefile, NULL)

     def transform_to_value(self, **kwds):
        """
        transform_to_value(self, **kwds)
        Execute transformation to an Xdm Node
        Args:
            **kwds: Possible optional arguments: source_file (str), xdm_node (PyXdmNode) and base_output_uri (str) which
                                                 is used for resolving relative URIs in the href attribute
                                                 of the xsl:result-document instruction.
        Returns:
            PyXdmNode: Result of the transformation as an PyXdmNode object
        Example:
              xslt30_proc = saxon_proc.new_xsl30_processor()
              executable = xslt30_proc.compile_stylesheet(stylesheet_file="test1.xsl")
            1) node = executable.transform_to_value(source_file="cat.xml")
            2) executable.set_initial_match_selection("cat.xml")\r
               node = executable.transform_to_value()
            3) node = saxon_proc.parse_xml(xml_text="<in/>")\r
               node = executable.transform_to_value(xdm_node= node)
        """
        cdef const char * c_sourcefile = NULL
        cdef char * c_stylesheet = NULL
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              '''c_sourcefile = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_sourcefile = py_string_string if value is not None else ""
            elif key == "base_output_uri":
              '''c_base_output_uri = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_base_output_uri = py_string_string if value is not None else ""
              self.thisxptr.setBaseOutputURI(c_base_output_uri)


        cdef PyXdmValue val = None
        cdef PyXdmAtomicValue aval = None
        cdef PyXdmNode nval = None
        cdef PyXdmMap mval = None
        cdef PyXdmArray aaval = None
        cdef PyXdmFunctionItem fval = None
        cdef saxoncClasses.XdmValue * xdmValue = NULL
        cdef PyXdmNode node_ = None

        if "xdm_node" in kwds:
            py_value = kwds["xdm_node"]
            if(isinstance(py_value, PyXdmNode)):
                node_ = py_value
                xdmValue = self.thisxptr.transformToValue(node_.derivednptr)
            else:
                raise Exception("Keyword argument 'xdm_node' is not of type PyXdmNode")

        else:
            xdmValue = self.thisxptr.transformFileToValue(c_sourcefile)
        if xdmValue is NULL:
            return None
        cdef type_ = xdmValue.getType()
        if type_== 4:
            aval = PyXdmAtomicValue()
            aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>xdmValue
            aval.thisvptr.incrementRefCount()
            return aval
        elif type_ == 3:
            nval = PyXdmNode()
            nval.derivednptr = nval.derivedptr = nval.thisvptr = <saxoncClasses.XdmNode*>xdmValue
            nval.thisvptr.incrementRefCount()
            return nval
        elif type_ == 5:
            fval = PyXdmFunctionItem()
            fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>xdmValue
            fval.thisvptr.incrementRefCount()
            return fval
        elif type_ == 6:
            mval = PyXdmMap()
            mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>xdmValue
            mval.thisvptr.incrementRefCount()
            return mval
        elif type_ == 7:
            aaval = PyXdmArray()
            aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>xdmValue
            aaval.thisvptr.incrementRefCount()
            return aaval
        else:
            val = PyXdmValue()
            val.thisvptr = xdmValue
            return val


     def apply_templates_returning_value(self, **kwds):
        """
        apply_templates_returning_value(self, **kwds)
        Invoke the stylesheet by applying templates to a supplied input sequence, Saving the results as an XdmValue.
        Args:
            **kwds: Possible optional arguments: source_file (str) or xdm_value (PyXdmValue). Other allowed argument:
                                                 base_output_uri (str) which is used for resolving relative URIs in the href attribute of
                                                 the xsl:result-document instruction.
        Returns:
            PyXdmValue: Result of the transformation as an PyXdmValue object
        Example:
              xslt30_proc = saxon_proc.new_xsl30_processor()
              executable = xslt30_proc.compile_stylesheet(stylesheet_file="test1.xsl")
            1) executable.set_initial_match_selection(file_name="cat.xml")\r
               node = executable.apply_templates_returning_value()
            2) node = executable.apply_templates_returning_value(source_file="cat.xml")
        """
        cdef const char * c_sourcefile = NULL
        cdef PyXdmValue value_ = None
        py_source_string = None
        py_stylesheet_string = None
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              '''c_sourcefile = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_sourcefile = py_string_string if value is not None else ""
              self.thisxptr.setInitialMatchSelectionAsFile(c_sourcefile)
            if key == "base_output_uri":
              '''c_base_output_uri = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_base_output_uri = py_string_string if value is not None else ""
              self.thisxptr.setBaseOutputURI(c_base_output_uri)
          elif key == "xdm_value":
            if isinstance(value, PyXdmValue):
              value_ = value
              self.thisxptr.setInitialMatchSelection(value_.thisvptr)
        cdef PyXdmValue val = None
        cdef PyXdmAtomicValue aval = None
        cdef PyXdmNode nval = None
        cdef PyXdmMap mval = None
        cdef PyXdmArray aaval = None
        cdef PyXdmFunctionItem fval = None
        cdef saxoncClasses.XdmValue * xdmValue = NULL
        xdmValue = self.thisxptr.applyTemplatesReturningValue()
        if xdmValue is NULL:
            return None
        cdef type_ = xdmValue.getType()
        if type_== 4:
            aval = PyXdmAtomicValue()
            aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>xdmValue
            return aval
        elif type_ == 3:
            nval = PyXdmNode()
            nval.derivednptr = nval.derivedptr = nval.thisvptr = <saxoncClasses.XdmNode*>xdmValue
            return nval
        elif type_ == 5:
            fval = PyXdmFunctionItem()
            fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>xdmValue
            fval.thisvptr.incrementRefCount()
            return fval
        elif type_ == 6:
            mval = PyXdmMap()
            mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>xdmValue
            mval.thisvptr.incrementRefCount()
            return mval
        elif type_ == 7:
            aaval = PyXdmArray()
            aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>xdmValue
            aaval.thisvptr.incrementRefCount()
            return aaval
        else:
            val = PyXdmValue()
            val.thisvptr = xdmValue
            return val
     def apply_templates_returning_string(self, **kwds):
        """
        apply_templates_returning_string(self, **kwds)
        Invoke the stylesheet by applying templates to a supplied input sequence, Saving the results as a str.
        Args:
            **kwds: Possible optional arguments: source_file (str) or xdm_value (PyXdmValue). Other allowed argument:
                                                 base_output_uri (str) which is used for resolving relative URIs in the href attribute of
                                                 the xsl:result-document instruction
        Returns:
            str: Result of the transformation as str
        Example:
               xslt30_proc = saxon_proc.new_xsl30_processor()
               executable = xslt30_proc.compile_stylesheet(stylesheet_file="test1.xsl")
            1) executable.set_initial_match_selection(file_name="cat.xml")\r
               content = executable.apply_templates_returning_string()
			   print(content)
            2) node = saxon_proc.parse_xml(xml_text="<in/>")\r
               content = executable.apply_templates_returning_string(xdm_value=node)
			   print(content)
        """
        cdef const char * c_sourcefile = NULL
        cdef PyXdmValue value_ = None
        py_source_string = None
        py_stylesheet_string = None
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              '''c_sourcefile = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_sourcefile = py_string_string if value is not None else ""
              self.thisxptr.setInitialMatchSelectionAsFile(c_sourcefile)
            if key == "base_output_uri":
              '''c_base_output_uri = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_base_output_uri = py_string_string if value is not None else ""
              self.thisxptr.setBaseOutputURI(c_base_output_uri)
          elif key == "xdm_value":
            if isinstance(value, PyXdmValue):
              value_ = value
              self.thisxptr.setInitialMatchSelection(value_.thisvptr)
        cdef const char* c_string  = self.thisxptr.applyTemplatesReturningString()
        return make_py_str(c_string)

     def apply_templates_returning_file(self, **kwds):
        """
        apply_templates_returning_file(self, **kwds)
        Invoke the stylesheet by applying templates to a supplied input sequence, Saving the
        results to file.
        Args:
            **kwds: Possible optional arguments: source_file (str) or xdm_value (PyXdmValue).
            Other allowed argument: The required argument output_file (str) and
            base_output_uri (str) which is used for resolving relative URIs in the href attribute of
            the xsl:result-document instruction.

        Example:
            1) executable = trans.compile_stylesheet(stylesheet_file="test1.xsl")
               executable.set_initial_match_selection(file_name="cat.xml")
               content = executable.apply_templates_returning_file(output_file="result.xml")
			   print(content)
        """
        cdef const char * c_sourcefile = NULL
        cdef const char * c_outputfile = NULL
        cdef PyXdmValue value_ = None
        py_source_string = None
        py_stylesheet_string = None
        py_output_string = None
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              '''c_sourcefile = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_sourcefile = py_string_string if value is not None else ""
              self.thisxptr.setInitialMatchSelectionAsFile(c_sourcefile)
            if key == "base_output_uri":
              '''c_base_output_uri = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_base_output_uri = py_string_string if value is not None else ""
              self.thisxptr.setBaseOutputURI(c_base_output_uri)
            if key == "output_file":
              py_output_string = value.encode('UTF-8') if value is not None else None
              c_outputfile = py_output_string if value is not None else ""
          elif key == "xdm_value":
            if isinstance(value, PyXdmNode):
              value_ = value
              self.thisxptr.setInitialMatchSelection(value_.thisvptr)
        self.thisxptr.applyTemplatesReturningFile(c_outputfile)

     def call_template_returning_value(self, str template_name=None, **kwds):
        """
        call_template_returning_value(self, str template_name, **kwds)
        Invoke a transformation by calling a named template and return result as an PyXdmValue.
        Args:
			template_name(str): The name of the template to invoke. If None is supplied then call the initial-template
            **kwds: Possible optional arguments: source_file (str) or xdm_value (PyXdmValue). Other allowed argument:
                    base_output_uri (str) which is used for for resolving relative URIs in the href attribute of the xsl:result-document instruction
        Returns:
            PyXdmValue: Result of the transformation as an PyXdmValue object
        Example:
               xslt30_proc = saxon_proc.new_xsl30_processor()
               executable = xslt30_proc.compile_stylesheet(stylesheet_file="test1.xsl")
            1) node = executable.call_template_returning_value("main")\r
            2) executable.set_initial_match_selection(file_name="cat.xml")\r
               node = executable.call_template_returning_value("main")
        """
        cdef const char * c_templateName = NULL
        cdef const char * c_sourcefile = NULL
        cdef const char * c_stylesheetfile = NULL
        cdef PyXdmValue value_ = None
        py_source_string = None
        py_template_name_str = None
        py_stylesheet_string = None
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              '''c_sourcefile = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_sourcefile = py_string_string if value is not None else ""
              self.thisxptr.setInitialMatchSelectionAsFile(c_sourcefile)
            if key == "base_output_uri":
              '''c_base_output_uri = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_base_output_uri = py_string_string if value is not None else ""
              self.thisxptr.setBaseOutputURI(c_base_output_uri)
          elif key == "xdm_node":
            if isinstance(value, PyXdmValue):
              value_ = value
              self.thisxptr.setInitialMatchSelection(value_.thisvptr)
        c_templateName = make_c_str(template_name)
        cdef PyXdmValue val = None
        cdef PyXdmAtomicValue aval = None
        cdef PyXdmNode nval = None
        cdef PyXdmMap mval = None
        cdef PyXdmArray aaval = None
        cdef PyXdmFunctionItem fval = None
        cdef saxoncClasses.XdmValue * xdmValue = NULL
        xdmValue = self.thisxptr.callTemplateReturningValue(c_templateName)
        if xdmValue is NULL:
            return None
        cdef type_ = xdmValue.getType()
        if type_== 4:
            aval = PyXdmAtomicValue()
            aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>xdmValue
            return aval
        elif type_ == 3:
            nval = PyXdmNode()
            nval.derivednptr = nval.derivedptr = nval.thisvptr = <saxoncClasses.XdmNode*>xdmValue
            return nval
        elif type_ == 5:
            fval = PyXdmFunctionItem()
            fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>xdmValue
            fval.thisvptr.incrementRefCount()
            return fval
        elif type_ == 6:
            mval = PyXdmMap()
            mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>xdmValue
            mval.thisvptr.incrementRefCount()
            return mval
        elif type_ == 7:
            aaval = PyXdmArray()
            aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>xdmValue
            aaval.thisvptr.incrementRefCount()
            return aaval
        else:
            val = PyXdmValue()
            val.thisvptr = xdmValue
            return val
     def call_template_returning_string(self, str template_name=None, **kwds):
        """
        call_template_returning_string(self, str template_name, **kwds)
        Invoke a transformation by calling a named template and return result as a string.
        If no source file or xdm value has been supplied, then the clark name 'xsl:initial-template' is used.
        Args:
			template_name(str): The name of the template to invoke. If None is supplied then call the initial-template
            **kwds: Possible optional arguments: source_file (str) or  xdm_Value (PyXdmValue). Other allowed argument:
            base_output_uri (str) which is used for for resolving relative URIs in the href attribute of the xsl:result-document instruction
        Returns:
            PyXdmValue: Result of the transformation as an PyXdmValue object
        Example:
            1) executable = trans.compile_stylesheet(stylesheet_file="test1.xsl")
               result = executable.call_template_returning_string("main")

            2) executable = trans.compile_stylesheet(stylesheet_file="test1.xsl")
               executable.set_initial_match_selection(file_name="cat.xml")\r
               result = executable.call_template_returning_string("main")

            3) executable = trans.compile_stylesheet(stylesheet_file="test2.xsl")
			   executable.set_initial_match_selection(file_name="cat.xml")\r
               result = executable.call_template_returning_string()
			   print(result)
        """
        cdef const char * c_sourcefile = NULL
        cdef const char * c_templateName = NULL
        cdef PyXdmNode value_ = None
        py_source_string = None
        py_template_name_str = None
        py_stylesheet_string = None
        cdef const char* c_string = NULL
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              '''c_sourcefile = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_sourcefile = py_string_string if value is not None else ""
              self.thisxptr.setInitialMatchSelectionAsFile(c_sourcefile)
            if key == "base_output_uri":
              '''c_base_output_uri = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_base_output_uri = py_string_string if value is not None else ""
              self.thisxptr.setBaseOutputURI(c_base_output_uri)
          elif key == "xdm_value":
            if isinstance(value, PyXdmValue):
              value_ = value
              self.thisxptr.setInitialMatchSelection(value_.thisvptr)
        c_templateName = make_c_str(template_name)
        c_string  = self.thisxptr.callTemplateReturningString(c_templateName)
        return make_py_str(c_string)

     def call_template_returning_file(self, str template_name=None, **kwds):
        """
        call_template_returning_file(self, str template_name, **kwds)
        Invoke a transformation by calling a named template and save result in a specified file.
        Args:
			template_name(str): The name of the template to invoke. If None is supplied then call the initial-template
            **kwds: Possible optional arguments: source_file (str) or xdm_node (PyXdmNode). Other allowed argument: stylesheet_file (str)
                    base_output_uri (str) which is used for for resolving relative URIs in the href attribute of the xsl:result-document instruction

        Example:
              executable = xslt30_proc.compile_stylesheet(stylesheet_file="test1.xsl")
            1) executable.call_template_returning_file("main", output_file="result.xml")
            2) executable.set_initial_match_selection(file_name="cat.xml")\r
               executable.call_template_returning_file("main", output_file="result.xml")
            3) executable.set_initial_match_selection(file_name="cat.xml")\r
               executable.call_template_returning_file(output_file="result.xml")
			   print(result)
        """
        cdef char * c_outputfile = NULL
        cdef const char * c_templateName = NULL
        cdef const char * c_sourcefile = NULL
        cdef PyXdmValue value_ = None
        py_source_string = None
        py_template_name_str = None
        py_stylesheet_string = None
        py_output_string = None
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              '''c_sourcefile = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_sourcefile = py_string_string if value is not None else ""
              self.thisxptr.setInitialMatchSelectionAsFile(c_sourcefile)
            if key == "base_output_uri":
              '''c_base_output_uri = make_c_str(value)'''
              py_string_string = value.encode('utf-8') if value is not None else None
              c_base_output_uri = py_string_string if value is not None else ""
              self.thisxptr.setBaseOutputURI(c_base_output_uri)
            if key == "output_file":
              py_output_string = value.encode('UTF-8') if value is not None else None
              c_outputfile = py_output_string if value is not None else ""
          elif key == "xdm_value":
            if isinstance(value, PyXdmValue):
              value_ = value
              self.thisxptr.setInitialMatchSelection(value_.thisvptr)
        c_templateName = make_c_str(template_name)
        self.thisxptr.callTemplateReturningFile(c_templateName, c_outputfile)

     def call_function_returning_value(self, str function_name, list args, **kwds):
        """
        call_function_returning_value(self, str function_name, list args, **kwds)
        Invoke a transformation by calling a named template and return result as an PyXdmValue.
        Args:
			function_name(str): The name of the template to invoke. If None is supplied then call the initial-template
			list args: Pointer array of XdmValue object - he values of the arguments to be supplied to the function.
            **kwds: Possible optional arguments: source_file (str) or xdm_value (PyXdmValue).
        Returns:
            PyXdmValue: Result of the transformation as an PyXdmValue object
        Example:
              executable = xslt30_proc.compile_stylesheet(stylesheet_file="test1.xsl")
            1) node = executable.call_function_returning_value("{http://localhost/example}func")
            2) xsltproc.set_initial_match_selection(file_name="cat.xml")\r
               value = executable.call_function_returning_value("{http://localhost/test}add")
            3) value = executable.call_function_returning_value(source_file="cat.xml", "{http://exmaple.com}func1")

        """
        cdef const char * c_functionName = NULL
        cdef const char * c_sourcefile = NULL

        cdef PyXdmValue value_ = None
        py_source_string = None
        py_template_name_str = None
        py_stylesheet_string = None
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              '''c_sourcefile = make_c_str(value)'''
              py_source_string = value.encode('UTF-8') if value is not None else None
              c_sourcefile = py_source_string if value is not None else ""
              self.thisxptr.setInitialMatchSelectionAsFile(c_sourcefile)
          elif key == "xdm_value":
            if isinstance(value, PyXdmValue):
              value_ = value
              self.thisxptr.setInitialMatchSelection(value_.thisvptr)
        cdef int len_= 0;
        len_ = len(args)
        """ TODO handle memory when finished with XdmValues """
        cdef saxoncClasses.XdmValue ** argumentV = self.thisxptr.createXdmValueArray(len_)

        for x in range(len(args)):
          if isinstance(args[x], PyXdmValue):
            value_ = args[x];
            argumentV[x] = value_.thisvptr
          else:
            raise Exception("Argument value at position " , x , " is not an PyXdmValue. The following object found: ", type(args[x]))
        '''c_functionName = make_c_str(function_name)'''
        py_function_string = function_name.encode('UTF-8') if function_name is not None else None
        c_functionName = py_function_string if function_name is not None else ""
        cdef PyXdmValue val = None
        cdef PyXdmAtomicValue aval = None
        cdef PyXdmNode nval = None
        cdef PyXdmMap mval = None
        cdef PyXdmArray aaval = None
        cdef PyXdmFunctionItem fval = None
        cdef saxoncClasses.XdmValue * xdmValue = NULL
        xdmValue = self.thisxptr.callFunctionReturningValue(c_functionName, argumentV, len(args))
        if xdmValue is NULL:
          return None
        cdef type_ = xdmValue.getType()
        if type_== 4:
          aval = PyXdmAtomicValue()
          aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>xdmValue
          return aval
        elif type_ == 3:
          nval = PyXdmNode()
          nval.derivednptr = nval.derivedptr = nval.thisvptr = <saxoncClasses.XdmNode*>xdmValue
          return nval
        elif type_ == 5:
            fval = PyXdmFunctionItem()
            fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>xdmValue
            fval.thisvptr.incrementRefCount()
            return fval
        elif type_ == 6:
            mval = PyXdmMap()
            mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>xdmValue
            mval.thisvptr.incrementRefCount()
            return mval
        elif type_ == 7:
            aaval = PyXdmArray()
            aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>xdmValue
            aaval.thisvptr.incrementRefCount()
            return aaval
        else:
          val = PyXdmValue()
          val.thisvptr = xdmValue
          return val
     def call_function_returning_string(self, str function_name, list args, **kwds):
        """
        call_function_returning_string(self, str function_name, list args, **kwds)
        Invoke a transformation by calling a named template and return result as a serialized string.
        Args:
			function_name(str): The name of the template to invoke. If None is supplied then call the initial-template
			list args: Pointer array of XdmValue object - he values of the arguments to be supplied to the function.
            **kwds: Possible optional arguments: source_file (str) or xdm_value (PyXdmValue). Other allowed argument: stylesheet_file (str)
        Returns:
            str: Result of the transformation as a str value
        Example:
               executable = xslt30_proc.compile_stylesheet(stylesheet_file="test1.xsl")
            1) result = executable.call_function_returning_string("{http://localhost/example}func")
            2) executable.set_initial_match_selection(file_name="cat.xml")\r
               result = executable.call_function_returning_string("{http://localhost/test}add")
            3) executable.set_initial_match_selection(file_name="cat.xml")\r
               result = executable.call_function_returning_string("{http://exmaple.com}func1")
        """
        cdef const char * c_functionName = NULL
        cdef const char * c_sourcefile = NULL
        cdef const char * c_stylesheetfile = NULL
        cdef PyXdmValue value_ = None
        py_source_string = None
        py_template_name_str = None
        py_stylesheet_string = None
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              '''c_sourcefile = make_c_str(value)'''
              py_s_string = value.encode('UTF-8') if value is not None else None
              c_sourcefile = py_s_string if value is not None else ""
              self.thisxptr.setInitialMatchSelectionAsFile(c_sourcefile)
            if key == "base_output_uri":
              '''c_base_output_uri = make_c_str(value)'''
              py_s_string = value.encode('UTF-8') if value is not None else None
              c_base_output_uri = py_s_string if value is not None else ""
              self.thisxptr.setBaseOutputURI(c_base_output_uri)
            if key == "stylesheet_file":
              '''c_stylesheetfile = make_c_str(value)'''
              py_s_string = value.encode('UTF-8') if value is not None else None
              c_stylesheetfile = py_s_string if value is not None else ""
              '''if isfile(value) == False:
                raise Exception("Stylesheet file does not exist")'''
          elif key == "xdm_value":
            if isinstance(value, PyXdmValue):
              value_ = value
              self.thisxptr.setInitialMatchSelection(value_.thisvptr)
        cdef int _len = len(args)
        """ TODO handle memory when finished with XdmValues """
        cdef saxoncClasses.XdmValue ** argumentV = self.thisxptr.createXdmValueArray(_len)
        for x in range(len(args)):
          if isinstance(args[x], PyXdmValue):
            value_ = args[x]
            argumentV[x] = value_.thisvptr
          else:
            raise Exception("Argument value at position ",x," is not an PyXdmValue")
        '''c_functionName = make_c_str(function_name)'''
        py_s_string = function_name.encode('UTF-8') if function_name is not None else None
        c_functionName = py_s_string if function_name is not None else ""
        cdef PyXdmValue val = None
        cdef PyXdmAtomicValue aval = None
        cdef PyXdmNode nval = None
        cdef saxoncClasses.XdmValue * xdmValue = NULL
        cdef const char* c_string = self.thisxptr.callFunctionReturningString(c_functionName, argumentV, len(args))
        return make_py_str(c_string)
     def call_function_returning_file(self, str function_name, list args, **kwds):
        """
        call_function_returning_file(self, str function_name, list args, **kwds)
        Invoke a transformation by calling a named template and return result in the supplied file.
        Args:
			function_name(str): The name of the template to invoke. If None is supplied
                        then call the initial-template
			list args: Pointer array of XdmValue object - he values of the arguments to be supplied to the function.
            **kwds: Possible optional arguments: source_file (str) or xdm_value (PyXdmValue). Other allowed argument: stylesheet_file (str)

        Example:
               executable = xslt30_proc.compile_stylesheet(stylesheet_file="test2.xsl")
            1) executable.set_output_file("result.xml")
			   executable.call_function_returning_file("{http://localhost/example}func")
            2) executable.set_initial_match_selection(file_name="cat.xml")\r
               executable.call_function_returning_file("{http://localhost/test}add", output_file="result.xml")
            3) executable.set_initial_match_selection(file_name="cat.xml")\r
               executable.call_function_returning_file("{http://exmaple.com}func1", output_file="result.xml")
        """
        cdef const char * c_functionName = NULL
        cdef const char * c_sourcefile = NULL
        cdef const char * c_outputfile = NULL
        cdef const char * c_stylesheetfile = NULL
        cdef PyXdmValue value_ = None
        cdef PyXdmValue valueArgs_ = None
        py_source_string = None
        py_template_name_str = None
        py_stylesheet_string = None
        for key, value in kwds.items():
          if isinstance(value, str):
            if key == "source_file":
              '''c_sourcefile = make_c_str(value)'''
              py_s_string = value.encode('UTF-8') if value is not None else None
              c_sourcefile = py_s_string if value is not None else ""
              self.thisxptr.setInitialMatchSelectionAsFile(c_sourcefile)
            if key == "base_output_uri":
              '''c_base_output_uri = make_c_str(value)'''
              py_s_string = value.encode('UTF-8') if value is not None else None
              c_base_output_uri = py_s_string if value is not None else ""
              self.thisxptr.setBaseOutputURI(c_base_output_uri)
            if key == "output_file":
              py_output_string = value.encode('UTF-8') if value is not None else None
              c_outputfile = py_output_string if value is not None else ""
            if key == "stylesheet_file":
              '''c_stylesheetfile = make_c_str(value)'''
              py_s_string = value.encode('UTF-8') if value is not None else None
              c_stylesheetfile = py_s_string if value is not None else ""
              '''if isfile(value) == False:
                raise Exception("Stylesheet file does not exist")'''
          elif key == "xdm_value":
            if isinstance(value, PyXdmValue):
              value_ = value
              self.thisxptr.setInitialMatchSelection(value_.thisvptr)
        cdef int _len = len(args)
        """ TODO handle memory when finished with XdmValues """
        cdef saxoncClasses.XdmValue ** argumentV = self.thisxptr.createXdmValueArray(_len)
        for x in range(len(args)):
          if isinstance(args[x], PyXdmValue):
            value_ = args[x]
            argumentV[x] = value_.thisvptr
          else:
            raise Exception("Argument value at position ",x," is not an PyXdmValue")
        '''c_functionName = make_c_str(function_name)'''
        py_s_string = function_name.encode('UTF-8') if function_name is not None else None
        c_functionName = py_s_string if function_name is not None else ""
        cdef PyXdmValue val = None
        cdef PyXdmAtomicValue aval = None
        cdef PyXdmNode nval = None
        cdef saxoncClasses.XdmValue * xdmValue = NULL
        self.thisxptr.callFunctionReturningFile(c_functionName, argumentV, len(args), c_outputfile)


     @property
     def exception_occurred(self):
        """
        exception_occurred(self)
        Property to check for pending exceptions without creating a local reference to the exception object
        Returns:
            boolean: True when there is a pending exception; otherwise return False
        """
        return self.thisxptr.exceptionOccurred()

     def exception_clear(self):
        """
        exception_clear(self)
        Clear any exception thrown
        """
        self.thisxptr.exceptionClear()

     @property
     def error_message(self):
        """
        error_message(self)
        A transformation may have a number of errors reported against it. Get the error message if there are any errors

        Returns:
            str: The message of the exception. Return None if the exception does not exist.
        """
        cdef const char* c_string = self.thisxptr.getErrorMessage()
        ustring = make_py_str(c_string)
        return ustring

        """def error_code(self):  """
        """                        
        error_code(self, index)
        A transformation may have a number of errors reported against it. Get the i'th error code if there are any errors
        Args:
            index (int): The i'th exception
        Returns:
            str: The error code associated with the i'th exception. Return None if the i'th exception does not exist.
        """
        """cdef const char* c_string = self.thisxptr.getErrorCode()
        ustring = c_string.decode('UTF-8') if c_string is not NULL else None
        return ustring"""

cdef class PyXQueryProcessor:
     """An PyXQueryProcessor object represents factory to compile, load and execute queries. """
     cdef saxoncClasses.XQueryProcessor *thisxqptr      # hold a C++ instance which we're wrapping
     def __cinit__(self):
        """
        __cinit__(self)
        Constructor for PyXQueryProcessor
        """
        self.thisxqptr = NULL
     def __dealloc__(self):
        """
        dealloc(self)
        """
        if self.thisxqptr != NULL:
           del self.thisxqptr
     def set_context(self, ** kwds):
        """
        set_context(self, **kwds)
        Set the initial context for the query

        Args:
            **kwds : Possible keyword argument file_name (str) or xdm_item (PyXdmItem)
        """
        py_error_message = "Error: set_context should only contain one of the following keyword arguments: (file_name|xdm_item)"
        if len(kwds) != 1:
          raise Exception(py_error_message)
        cdef py_value = None
        cdef py_value_string = None
        cdef char * c_source
        cdef PyXdmItem xdm_item = None
        if "file_name" in kwds:
            py_value = kwds["file_name"]
            py_value_string = py_value.encode('UTF-8') if py_value is not None else None
            c_source = py_value_string if py_value is not None else ""
            self.thisxqptr.setContextItemFromFile(c_source)
        elif "xdm_item" in kwds:
            xdm_item = kwds["xdm_item"]

            xdm_item = kwds["xdm_item"]

            if  isinstance(xdm_item, PyXdmItem):
                xdm_item.derivedptr.incrementRefCount()
            elif  isinstance(xdm_item, PyXdmNode):
                xdm_item.derivednptr.incrementRefCount()
            elif  isinstance(xdm_item, PyXdmAtomicValue):
                xdm_item.derivedaptr.incrementRefCount()

            self.thisxqptr.setContextItem(xdm_item.derivedptr)
        else:
          raise Exception(py_error_message)

     def set_output_file(self, output_file):
        """
        set_output_file(self, output_file)
        Set the output file where the result is sent
        Args:
            output_file (str): Name of the output file
        """
        cdef const char * c_outfile = NULL
        py_value_string = output_file.encode('UTF-8') if output_file is not None else None
        c_outfile = py_value_string if output_file is not None else ""
        self.thisxqptr.setOutputFile(c_outfile)
     def set_parameter(self, name, PyXdmValue value):
        """
        set_parameter(self, name, PyXdmValue value)
        Set the value of a query parameter
        Args:
            name (str): the name of the stylesheet parameter, as a string. For namespaced parameter use the JAXP solution i.e. "{uri}name
            value (PyXdmValue): the value of the query parameter, or None to clear a previously set value
        """
        cdef const char * c_str = NULL
        '''make_c_str(name)'''
        py_s_string = name.encode('UTF-8') if name is not None else None
        c_str = py_s_string if name is not None else ""
        if c_str is not NULL:
            value.thisvptr.incrementRefCount()
            self.thisxqptr.setParameter(c_str, value.thisvptr)
     def remove_parameter(self, name):
        """
        remove_parameter(self, name)
        Remove the parameter given by name from the PyXQueryProcessor. The parameter will not have any affect on the query if it has not yet been executed
        Args:
            name (str): The name of the query parameter
        Returns:
            bool: True if the removal of the parameter has been successful, False otherwise.
        """
        py_value_string = name.encode('UTF-8') if name is not None else None
        c_name = py_value_string if name is not None else ""
        self.thisxqptr.removeParameter(c_name)
     def set_property(self, name, str value):
        """
        set_property(self, name, value)
        Set a property specific to the processor in use.

        Args:
            name (str): The name of the property
            value (str): The value of the property
        Example:
            PyXQueryProcessor: set serialization properties (names start with '!' i.e. name "!method" -> "xml")\r
            'o':outfile name,\r
            'dtd': Possible values 'on' or 'off' to set DTD validation,\r
            'resources': directory to find Saxon data files,\r
            's': source as file name,\r
        """
        py_name_string = name.encode('UTF-8') if name is not None else None
        c_name = py_name_string if name is not None else ""
        py_value_string = value.encode('UTF-8') if value is not None else None
        c_value = py_value_string if value is not None else ""
        self.thisxqptr.setProperty(c_name, c_value)
     def clear_parameters(self):
        """
        clear_parameter(self)
        Clear all parameters set on the processor
        """
        self.thisxqptr.clearParameters()
     def clear_properties(self):
        """
        clear_parameter(self)
        Clear all properties set on the processor
        """
        self.thisxqptr.clearProperties()
     def set_updating(self, updating):
        """
        set_updating(self, updating)
        Say whether the query is allowed to be updating. XQuery update syntax will be rejected during query compilation unless this
        flag is set. XQuery Update is supported only under Saxon-EE/C.
        Args:
            updating (bool): true if the query is allowed to use the XQuery Update facility (requires Saxon-EE/C). If set to false,
                             the query must not be an updating query. If set to true, it may be either an updating or a non-updating query.
        """
        self.thisxqptr.setUpdating(updating)
     def run_query_to_value(self, ** kwds):
        """
        run_query_to_value(self, **kwds)
        Execute query and output result as an PyXdmValue object
        Args:
            **kwds: Keyword arguments with the possible options input_file_name (str) or input_xdm_item (PyXdmItem). Possible to supply
                    query with the arguments 'query_file' or 'query_text', which are of type str.
        Returns:
            PyXdmValue: Output result as an PyXdmValue
        """
        cdef PyXdmAtomicValue aval = None
        cdef PyXdmAtomicValue avalue_
        cdef PyXdmValue value_
        cdef PyXdmNode nval = None
        cdef PyXdmFunctionItem fval = None
        cdef PyXdmMap mval = None
        cdef PyXdmArray aaval = None
        cdef PyXdmValue val = None
        if not len(kwds) == 0:

            if "input_file_name" in kwds:
                self.set_context(file_name=kwds["input_file_name"])
            elif "input_xdm_item" in kwds:
                self.set_context(xdm_item=(kwds["xdm_item"]))
            if "query_file" in kwds:
                self.set_query_file(kwds["output_file_name"])
            elif "query_text" in kwds:
                self.set_query_content(kwds["query_text"])

        cdef saxoncClasses.XdmValue * xdmValue = self.thisxqptr.runQueryToValue()
        if xdmValue is NULL:
            return None
        cdef type_ = xdmValue.getType()
        if type_== 4:
            aval = PyXdmAtomicValue()
            aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>xdmValue
            return aval
        elif type_ == 3:
            nval = PyXdmNode()
            nval.derivednptr = nval.derivedptr = nval.thisvptr = <saxoncClasses.XdmNode*>xdmValue
            return nval
        elif type_ == 5:
            fval = PyXdmFunctionItem()
            fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>xdmValue
            fval.thisvptr.incrementRefCount()
            return fval
        elif type_ == 6:
            mval = PyXdmMap()
            mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>xdmValue
            mval.thisvptr.incrementRefCount()
            return mval
        elif type_ == 7:
            aaval = PyXdmArray()
            aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>xdmValue
            aaval.thisvptr.incrementRefCount()
            return aaval
        else:
            val = PyXdmValue()
            val.thisvptr = xdmValue
            val.thisvptr.incrementRefCount()
            return val
     def run_query_to_string(self, ** kwds):
        """
        run_query_to_string(self, **kwds)
        Execute query and output result as a string
        Args:
            **kwds: Keyword arguments with the possible options input_file_name (str) or input_xdm_item (PyXdmItem). Possible to supply
                    query with the arguments 'query_file' or 'query_text', which are of type str.
        Returns:
            str: Output result as a string
        """
        cdef const char * c_string
        if len(kwds) == 0:
          ustring = make_py_str(self.thisxqptr.runQueryToString())
          return ustring
        if "input_file_name" in kwds:
          self.set_context(file_name=kwds["input_file_name"])
        elif "input_xdm_item" in kwds:
          self.set_context(xdm_item=(kwds["xdm_item"]))
        if "query_file" in kwds:
          self.set_query_file(kwds["output_file_name"])
        elif "query_text" in kwds:
          self.set_query_content(kwds["query_text"])

        ustring = make_py_str(self.thisxqptr.runQueryToString())
        return ustring

     def run_query_to_file(self, ** kwds):
        """
        run_query_to_file(self, **kwds)
        Execute query with the result saved to file
        Args:
            **kwds: Keyword arguments with the possible options input_file_name (str) or input_xdm_item (PyXdmItem). The Query can be
                    supplied with the arguments 'query_file' or 'query_text', which are of type str. The name of the output file is
                    specified as the argument output_file_name.
        """
        if len(kwds) == 0:
          self.thisxqptr.runQueryToFile()
        if "input_file_name" in kwds:
          self.set_context(file_name=(kwds["input_file_name"]))
        elif "input_xdm_item" in kwds:
          self.set_context(xdm_item=(kwds["xdm_item"]))
        if "output_file_name" in kwds:
          self.set_output_file(kwds["output_file_name"])
        else:
          raise Exception("Error: output_file_name required in method run_query_to_file")
        if "query_file" in kwds:
          self.set_query_file(kwds["output_file_name"])
        elif "query_text" in kwds:
          self.set_query_content(kwds["query_text"])
        self.thisxqptr.runQueryToFile()
     def declare_namespace(self, prefix, uri):
        """
        declare_namespace(self, prefix, uri)
        Declare a namespace binding part of the static context for queries compiled using this.
        This binding may be overridden by a binding that appears in the query prolog.
        The namespace binding will form part of the static context of the query, but it will
        not be copied into result trees unless the prefix is actually used in an element or attribute name.
        Args:
            prefix (str): The namespace prefix. If the value is a zero-length string, this method sets the default namespace for elements and types.
            uri (uri) : The namespace URI. It is possible to specify a zero-length string to "undeclare" a namespace; in this case the prefix will not be available for use,
            except in the case where the prefix is also a zero length string, in which case the absence of a prefix implies that the name is in no namespace.
        """
        cdef const char * c_prefix = NULL
        '''make_c_str(prefix)'''
        py_s_string = prefix.encode('UTF-8') if prefix is not None else None
        c_prefix = py_s_string if prefix is not None else ""

        cdef const char * c_uri = NULL
        '''make_c_str(uri)'''
        py_ss_string = uri.encode('UTF-8') if uri is not None else None
        c_uri = py_ss_string if uri is not None else ""
        self.thisxqptr.declareNamespace(c_prefix, c_uri)

     def set_query_file(self, file_name):
        """
        set_query_file(self, file_name)
        Set the query to be executed as a file
        Args:
            file_name (str): The file name for the query
        """
        cdef const char * c_filename = NULL
        '''make_c_str(file_name)'''
        py_s_string = file_name.encode('UTF-8') if file_name is not None else None
        c_filename = py_s_string if file_name is not None else ""
        self.thisxqptr.setQueryFile(c_filename)

     def set_query_content(self, str content):
        """
        set_query_content(self)
        Set the query to be executed as a string
        Args:
            content (str): The query content suplied as a string
        """
        cdef const char * c_content = NULL
        if content is not None:
            '''make_c_str(content)'''
            py_s_string = content.encode('UTF-8') if content is not None else None
            c_content = py_s_string if content is not None else ""
            self.thisxqptr.setQueryContent(c_content)
     def set_query_base_uri(self, base_uri):
        """
        set_query_base_uri(self, base_uri)
        Set the static base query for the query
        Args:
            base_uri (str): The static base URI; or None to indicate that no base URI is available
        """
        cdef const char * c_baseuri = NULL
        py_base_string = base_uri.encode('UTF-8') if base_uri is not None else None
        c_baseuri = py_base_string if base_uri is not None else ""
        self.thisxqptr.setQueryBaseURI(c_baseuri)
     def set_cwd(self, cwd):
        """
        set_cwd(self, cwd)
        Set the current working directory.
        Args:
            cwd (str): current working directory
        """
        py_cwd_string = cwd.encode('UTF-8') if cwd is not None else None
        c_cwd = py_cwd_string if cwd is not None else ""
        self.thisxqptr.setcwd(c_cwd)
     def check_exception(self):
        """
        check_exception(self)
        Check for exception thrown and get message of the exception.

        Returns:
            str: Returns the exception message if thrown otherwise return None
        """
        return self.thisxqptr.checkException()

     @property
     def exception_occurred(self):
        """
        exception_occurred(self)
        Property to check for pending exceptions without creating a local reference to the exception object
        Returns:
            boolean: True when there is a pending exception; otherwise return False
        """
        return self.thisxqptr.exceptionOccurred()

     def exception_clear(self):
        """
        exception_clear(self)
        Clear any exception thrown
        """
        self.thisxqptr.exceptionClear()

     @property
     def error_message(self):
        """
        error_message(self)
        A transformation may have a number of errors reported against it. This property returns error message if there are any errors

        Returns:
            str: The message of the exception. Return None if the exception does not exist.
        """
        return make_py_str(self.thisxqptr.getErrorMessage())

     @property
     def error_code(self):
        """
        error_code(self)
        A transformation may have a number of errors reported against it. This property returns error code if there are any errors

        Returns:
            str: The error code associated with the exception. Return None if the exception does not exist.
        """
        return make_py_str(self.thisxqptr.getErrorCode())

cdef class PyXPathProcessor:
     """An XPathProcessor represents factory to compile, load and execute the XPath query. """
     cdef saxoncClasses.XPathProcessor *thisxpptr      # hold a C++ instance which we're wrapping
     def __cinit__(self):
        """
        cinit(self)
        Constructor for PyXPathProcessor
        """
        self.thisxpptr = NULL
     def __dealloc__(self):
        """
        dealloc(self)
        """
        if self.thisxpptr != NULL:
           del self.thisxpptr
     def evaluate(self, xpath_str):
        """
        evaluate(self, xpath_str)
        Args:
            xpath_str (str): The XPath query suplied as a string
        Returns:
            PyXdmValue:
        """
        '''c_xpath = make_c_str(xpath_str)'''
        py_value_string = xpath_str.encode('utf-8') if xpath_str is not None else None
        c_xpath = py_value_string if xpath_str is not None else ""
        cdef PyXdmValue val = None
        cdef type_ = 0
        cdef saxoncClasses.XdmValue * xdmValue = self.thisxpptr.evaluate(c_xpath)
        if xdmValue == NULL:
            return None
        else:
            val = PyXdmValue()
            val.thisvptr = xdmValue
            return val

     def evaluate_single(self, xpath_str):
        """
        evaluate_single(self, xpath_str)
        Args:
            xpath_str (str): The XPath query supplied as a string
        Returns:
            PyXdmItem: A single Xdm Item is returned. return None if the expression returns an empty sequence.
            If the expression returns a sequence of more than one item, any items after the first are ignored.
        """
        cdef PyXdmNode val = None
        cdef PyXdmAtomicValue aval = None
        cdef PyXdmMap mval = None
        cdef PyXdmArray aaval = None
        cdef PyXdmFunctionItem fval = None
        cdef const char * c_xpath = NULL
        '''make_c_str(xpath_str)'''
        py_s_string = xpath_str.encode('UTF-8') if xpath_str is not None else None
        c_xpath = py_s_string if xpath_str is not None else ""

        cdef saxoncClasses.XdmItem * xdmItem = self.thisxpptr.evaluateSingle(c_xpath)
        if xdmItem == NULL:
            return None
        cdef type_ = xdmItem.getType()
        if type_ == 4:
            aval = PyXdmAtomicValue()
            aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>xdmItem
            aval.derivedptr.incrementRefCount()
            return aval
        elif type_ == 3:
            val = PyXdmNode()
            val.derivednptr = val.derivedptr = val.thisvptr = <saxoncClasses.XdmNode*>xdmItem
            val.derivedptr.incrementRefCount()
            return val
        elif type_ == 5:
            fval = PyXdmFunctionItem()
            fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>xdmItem
            fval.thisvptr.incrementRefCount()
            return fval
        elif type_ == 6:
            mval = PyXdmMap()
            mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>xdmItem
            mval.thisvptr.incrementRefCount()
            return mval
        elif type_ == 7:
            aaval = PyXdmArray()
            aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>xdmItem
            aaval.thisvptr.incrementRefCount()
            return aaval
        else:
            val = PyXdmItem()
            val.derivedptr = val.thisvptr = xdmItem
            val.derivedptr.incrementRefCount()
            return val

     def set_context(self, **kwds):
        """
        set_context(self, **kwds)
        Set the context for the XPath query

        Args:
            **kwds : Possible keyword argument file_name (str) or xdm_item (PyXdmItem)
        """
        py_error_message = "Error: set_context should only contain one of the following keyword arguments: (file_name|xdm_item)"
        if len(kwds) != 1:
          raise Exception(py_error_message)
        cdef py_value = None
        cdef py_value_string = None
        cdef char * c_source
        cdef PyXdmItem xdm_item = None
        if "file_name" in kwds:
            py_value = kwds["file_name"]
            py_value_string = py_value.encode('UTF-8') if py_value is not None else None
            c_source = py_value_string if py_value is not None else ""
            self.thisxpptr.setContextFile(c_source)
        elif "xdm_item" in kwds:
            xdm_item = kwds["xdm_item"]

            if  isinstance(xdm_item, PyXdmItem):
                xdm_item.derivedptr.incrementRefCount()
            elif  isinstance(xdm_item, PyXdmNode):
                xdm_item.derivednptr.incrementRefCount()
            elif  isinstance(xdm_item, PyXdmAtomicValue):
                xdm_item.derivedaptr.incrementRefCount()

            self.thisxpptr.setContextItem(xdm_item.derivedptr)
        else:
          raise Exception(py_error_message)
     def set_cwd(self, cwd):
        """
        set_cwd(self, cwd)
        Set the current working directory.
        Args:
            cwd (str): current working directory
        """
        py_cwd_string = cwd.encode('UTF-8') if cwd is not None else None
        cdef char * c_cwd = py_cwd_string if cwd is not None else ""
        self.thisxpptr.setcwd(c_cwd)
     def effective_boolean_value(self, xpath_str):
        """
        effective_boolean_value(self, xpath_str)
        Evaluate the XPath expression, returning the effective boolean value of the result.

        Args:
            xpath_str (str): Supplied as a string
        Returns:
            boolean: The result is a boolean value.
        """
        py_value_string = xpath_str.encode('UTF-8') if xpath_str is not None else None
        c_xpath = py_value_string if xpath_str is not None else ""
        return self.thisxpptr.effectiveBooleanValue(c_xpath)
     def set_parameter(self, name, value):
        """
        set_parameter(self, name, PyXdmValue value)
        Set the value of a XPath parameter
        Args:
            name (str): the name of the XPath parameter, as a string. For namespaced parameter use the JAXP solution i.e. "{uri}name
            value (PyXdmValue): the value of the query parameter, or None to clear a previously set value
        """
        cdef char * c_str = NULL
        cdef PyXdmAtomicValue avalue_
        cdef PyXdmItem ivalue_
        cdef PyXdmNode nvalue_
        cdef PyXdmValue value_
        '''make_c_str(name)'''
        py_name_string = name.encode('utf-8') if name is not None else None
        c_str = py_name_string if name is not None else ""
        if c_str is not NULL:
            if isinstance(value, PyXdmValue):
                value_ = value
                value_.thisvptr.incrementRefCount()
                self.thisxpptr.setParameter(c_str, value_.thisvptr)
            elif  isinstance(value, PyXdmNode):
                nvalue_ = value
                nvalue_.derivednptr.incrementRefCount()
                self.thisxpptr.setParameter(c_str, <saxoncClasses.XdmValue *>  nvalue_.derivednptr)
            elif  isinstance(value, PyXdmAtomicValue):
                avalue_ = value
                avalue_.derivedaptr.incrementRefCount()
                self.thisxpptr.setParameter(c_str, <saxoncClasses.XdmValue *> avalue_.derivedaptr)
            elif  isinstance(value, PyXdmItem):
                ivalue_ = value
                ivalue_.derivedptr.incrementRefCount()
                self.thisxpptr.setParameter(c_str, <saxoncClasses.XdmValue *>  ivalue_.derivedptr)

            '''self.thisxpptr.setParameter(c_str, value.thisvptr)'''
     def remove_parameter(self, name):
        """
        remove_parameter(self, name)
        Remove the parameter given by name from the PyXPathProcessor. The parameter will not have any affect on the XPath if it has not yet been executed
        Args:
            name (str): The name of the XPath parameter
        Returns:
            bool: True if the removal of the parameter has been successful, False otherwise.
        """
        self.thisxpptr.removeParameter(name)
     def set_property(self, name, value):
        """
        set_property(self, name, value)
        Set a property specific to the processor in use.

        Args:
            name (str): The name of the property
            value (str): The value of the property
        Example:
            PyXPathProcessor: set serialization properties (names start with '!' i.e. name "!method" -> "xml")\r
            'resources': directory to find Saxon data files,\r
            's': source as file name,\r
            'extc': REgister native library to be used with extension functions
        """
        py_name_string = name.encode('UTF-8') if name is not None else None
        c_name = py_name_string if name is not None else ""
        py_value_string = value.encode('UTF-8') if value is not None else None
        c_value = py_value_string if value is not None else ""
        self.thisxpptr.setProperty(c_name, c_value)
     def declare_namespace(self, prefix, uri):
        """
        declare_namespace(self, prefix, uri)
        Declare a namespace binding as part of the static context for XPath expressions compiled using this compiler
        Args:
            prefix (str): The namespace prefix. If the value is a zero-length string, this method sets the default namespace
                          for elements and types.
            uri (uri) : The namespace URI. It is possible to specify a zero-length string to "undeclare" a namespace;
                        in this case the prefix will not be available for use, except in the case where the prefix is also a
                        zero length string, in which case the absence of a prefix implies that the name is in no namespace.
        """
        py_prefix_string = prefix.encode('UTF-8') if prefix is not None else None
        c_prefix = py_prefix_string if prefix is not None else ""
        py_uri_string = uri.encode('UTF-8') if uri is not None else None
        c_uri = py_uri_string if uri is not None else ""
        self.thisxpptr.declareNamespace(c_prefix, c_uri)



     def declare_variable(self, str name):
        """
        declare_variable(self, name)
        Declare variable name
        Args:
        prefix (str): The variable name to declare

        """
        py_name_string = name.encode('UTF-8') if name is not None else None
        c_name = py_name_string if name is not None else ""
        self.thisxpptr.declareVariable(c_name)

     def set_backwards_compatible(self, option):
        """
        set_backwards_compatible(self, option)
        Say whether XPath 1.0 backwards compatibility mode is to be used
        Args:
            option (bool): true if XPath 1.0 backwards compatibility is to be enabled, false if it is to be disabled.
        """
        cdef bool c_option
        c_option = option
        self.thisxpptr.setBackwardsCompatible(c_option)

     def set_caching(self, is_caching):
         """
         set_caching(self, is_caching)
         Say whether the compiler should maintain a cache of compiled expressions.
         Args:
         is_caching (bool): if set to true, caching of compiled expressions is enabled.
                            If set to false, any existing cache is cleared, and future compiled expressions
                            will not be cached until caching is re-enabled. The cache is also cleared
                            (but without disabling future caching)
                            if any method is called that changes the static context for compiling
                            expressions, for example declareVariable(QName) or
                            declareNamespace(String, String).

         """
         cdef bool c_is_caching
         c_is_caching = is_caching
         self.thisxpptr.setCaching(c_is_caching)
     def import_schema_namespace(self, uri):
         """
         import_schema_namespace(self, uri)
         Import a schema namespace
         Args:
         uri (str): The schema namespace to be imported. To import declarations in a no-namespace schema, supply a zero-length string.

         """
         py_uri_string = uri.encode('UTF-8') if uri is not None else None
         c_name = py_uri_string if uri is not None else ""
         self.thisxpptr.importSchemaNamespace(c_name)
     def clear_parameters(self):
        """
        clear_parameter(self)
        Clear all parameters set on the processor
        """
        self.thisxpptr.clearParameters()
     def clear_properties(self):
        """
        clear_parameter(self)
        Clear all properties set on the processor
        """
        self.thisxpptr.clearProperties()

     @property
     def exception_occurred(self):
        """
        exception_occurred(self)
        Check if an exception has occurred internally within SaxonC
        Returns:
            boolean: True or False if an exception has been reported internally in SaxonC
        """
        return self.thisxpptr.exceptionOccurred()

     def exception_clear(self):
        """
        exception_clear(self)
        Clear any exception thrown
        """
        self.thisxpptr.exceptionClear()

     @property
     def error_message(self):
        """
        error_message(self)
        A transformation may have a number of errors reported against it. This property returns the error message if there are any errors

        Returns:
            str: The message of the exception. Return None if the exception does not exist.
        """
        return make_py_str(self.thisxpptr.getErrorMessage())

     @property
     def error_code(self):
        """
        error_code(self)
        A transformation may have a number of errors reported against it. This property returns the error code if there are any errors

        Returns:
            str: The error code associated with the exception. Return None if the exception does not exist.
        """
        return make_py_str(self.thisxpptr.getErrorCode())

cdef class PySchemaValidator:
     """An PySchemaValidator represents factory for validating instance documents against a schema."""

     cdef saxoncClasses.SchemaValidator *thissvptr      # hold a C++ instance which we're wrapping
     def __cinit__(self):
        self.thissvptr = NULL
     def __dealloc__(self):
        if self.thissvptr != NULL:
           del self.thissvptr
     def set_cwd(self, cwd):
        """
        set_cwd(self, cwd)
        Set the current working directory.
        Args:
            cwd (str): current working directory
        """
        py_cwd_string = cwd.encode('UTF-8') if cwd is not None else None
        cdef char * c_cwd = py_cwd_string if cwd is not None else ""
        self.thissvptr.setcwd(c_cwd)
     def register_schema(self, **kwds):
        """
        Register schema given as file name or schema text. (xsd_text|xsd_file)
        Args:
            **kwds: Keyword argument options only one of 'xsd_text' or 'xsd_file'
        """
        py_error_message = "Error: register_schema should only contain one of the following keyword arguments: (xsd_text|xsd_file)"
        if len(kwds) != 1:
          raise Exception(py_error_message)
        cdef py_value = None
        cdef py_value_string = None
        cdef char * c_source

        if "xsd_text" in kwds:
            py_value = kwds["xsd_text"]
            py_value_string = py_value.encode('UTF-8') if py_value is not None else None
            c_source = py_value_string if py_value is not None else ""
            self.thissvptr.registerSchemaFromString(c_source)
        elif "xsd_file" in kwds:
            py_value = kwds["xsd_file"]
            py_value_string = py_value.encode('UTF-8') if py_value is not None else None
            c_source = py_value_string if py_value is not None else ""
            self.thissvptr.registerSchemaFromFile(c_source)
        else:
          raise Exception(py_error_message)
     def export_schema(self, file_name):
        """
        export_schema(self, file_name)
        Export a precompiled Schema Component Model containing all the components (except built-in components) that have been loaded
        Args:
            file_name (str):The file name that will be used for thje saved SCM
        """
        py_value_string = file_name.encode('UTF-8') if file_name is not None else None
        c_source = py_value_string
        if file_name is not None:
            self.thissvptr.exportSchema(c_source)
        else:
            raise Warning("Unable to export the Schema. file_name has the value None")

     def set_output_file(self, output_file):
        """
        set_output_file(self, output_file)
        Set the name of the output file that will be used by the validator.
        Args:
            output_file (str):The output file name for use by the validator

        """
        py_value_string = output_file.encode('UTF-8') if output_file is not None else None
        c_source = py_value_string
        if output_file is not None:
            self.thissvptr.setOutputFile(c_source)
        else:
            raise Warning("Unable to set output_file. output_file has the value None")
     def validate(self, **kwds):
        """
        validate(self, **kwds)
        Validate an instance document by a registered schema.

        Args:
            **kwds: The possible keyword arguments must be one of the follow (file_name|xml_text|xdm_node).
                    The source file to be validated. Allow None when source document is supplied using the set_source method
        """
        py_error_message = "Error: validate should only contain one of the following keyword arguments: (file_name|xdm_node|xml_text)"
        if len(kwds) > 1:
          raise Exception(py_error_message)
        cdef py_value = None
        cdef py_value_string = None
        cdef char * c_source
        cdef PyXdmNode xdm_node = None
        if "file_name" in kwds:
            py_value = kwds["file_name"]
            py_value_string = py_value.encode('UTF-8') if py_value is not None else None
            c_source = py_value_string if py_value is not None else ""
            self.thissvptr.validate(c_source)
        elif "xdm_node" in kwds:
            xdm_node = kwds["xdm_node"]
            if isinstance(xdm_node, PyXdmNode):
               self.thissvptr.setSourceNode(xdm_node.derivednptr)
               self.thissvptr.validate(NULL)
        else:
            self.thissvptr.validate(NULL)
     def validate_to_node(self, **kwds):
        """
        validate_to_node(self, **kwds)
        Validate an instance document by a registered schema.

        Args:
            **kwds: The possible keyword arguments must be one of the follow (file_name|xml_text|xdm_node).
                    The source file to be validated. Allow None when source document is supplied using the set_source method
        Returns:
            PyXdmNode: The validated document returned to the calling program as an PyXdmNode
        """
        py_error_message = "Error: validate should only contain one of the following keyword arguments: (file_name|xdm_node|xml_text)"
        if len(kwds) > 1:
          raise Exception(py_error_message)
        cdef py_value = None
        cdef py_value_string = None
        cdef char * c_source
        cdef PyXdmNode xdm_node = None
        cdef PyXdmNode val = None
        cdef saxoncClasses.XdmNode * xdmNode = NULL
        if "file_name" in kwds:
            py_value = kwds["file_name"]
            py_value_string = py_value.encode('UTF-8') if py_value is not None else None
            c_source = py_value_string if py_value is not None else ""
            '''if isfile(py_value_string) == False:
                raise Exception("Source file with name "+py_value_string+" does not exist")'''
            xdmNode = self.thissvptr.validateToNode(c_source)
        elif "xdm_node" in kwds:
            xdm_node = kwds["xdm_node"]
            if isinstance(xdm_node, PyXdmNode):
                self.thissvptr.setSourceNode(xdm_node.derivednptr)
                xdmNode = self.thissvptr.validateToNode(NULL)
        else:
            xdmNode = self.thissvptr.validateToNode(NULL)

        if xdmNode == NULL:
            return None
        else:
            val = PyXdmNode()
            val.derivednptr = val.derivedptr = val.thisvptr =  xdmNode
            return val
     def set_source_node(self, PyXdmNode source):
        """
        set_source_node(self, source)
        Set the source as an PyXdmNode object that will be validated
        Args:
            source (PyXdmNode) :
        """
        self.thissvptr.setSourceNode(source.derivednptr)
     @property
     def validation_report(self):
        """
        validation_report
        The validation report Property
        :PyXdmNode: The Validation report result from the Schema validator
        """
        cdef PyXdmNode val = None
        cdef saxoncClasses.XdmNode * xdmNode = NULL
        xdmNode = self.thissvptr.getValidationReport()
        if xdmNode == NULL:
            return None
        else:
            val = PyXdmNode()
            val.derivednptr = val.derivedptr = val.thisvptr = xdmNode
            return val
     def set_parameter(self, name, PyXdmValue value):
        """
        set_parameter(self, name, PyXdmValue value)
        Set the value of the parameter for the Schema validator
        Args:
            name (str): the name of the schema parameter, as a string. For namespaced parameter use the JAXP solution i.e. "{uri}name
            value (PyXdmValue): the value of the parameter, or None to clear a previously set value
        """
        cdef const char * c_str = NULL
        '''make_c_str(name)'''
        py_name_string = name.encode('UTF-8') if name is not None else None
        c_str = py_name_string if name is not None else ""
        if c_str is not NULL:
            value.thisvptr.incrementRefCount()
            self.thissvptr.setParameter(c_str, value.thisvptr)
     def remove_parameter(self, name):
        """
        remove_parameter(self, name)
        Remove the parameter given by name from the PySchemaValidator. The parameter will not have any affect on the SchemaValidator if it has not yet been executed
        Args:
            name (str): The name of the schema parameter
        Returns:
            bool: True if the removal of the parameter has been successful, False otherwise.
        """
        cdef const char * c_str = NULL
        '''make_c_str(name)'''
        py_name_string = name.encode('UTF-8') if name is not None else None
        c_str = py_name_string if name is not None else ""
        if c_str is not NULL:
            self.thissvptr.removeParameter(c_str)
     def set_property(self, name, value):
        """
        set_property(self, name, value)
        Set a property specific to the processor in use.

        Args:
            name (str): The name of the property
            value (str): The value of the property
        Example:
            PySchemaValidator: set serialization properties (names start with '!' i.e. name "!method" -> "xml")\r
            'o':outfile name,\r
            'dtd': Possible values 'on' or 'off' to set DTD validation,\r
            'resources': directory to find Saxon data files,\r
            's': source as file name,\r
            'string': Set the source as xml string for validation. Parsing will take place in the validate method\r
            'report-node': Boolean flag for validation reporting feature. Error validation failures are represented in an XML
                           document and returned as an PyXdmNode object\r
            'report-file': Specifcy value as a file name string. This will switch on the validation reporting feature, which will be
                           saved to the file in an XML format\r
            'verbose': boolean value which sets the verbose mode to the output in the terminal. Default is 'on'
            'element-type': Set the name of the required type of the top-lelvel element of the doucment to be validated.
                            The string should be in the Clark notation {uri}local\r
            'lax': Boolean to set the validation mode to strict (False) or lax ('True')
        """
        cdef const char * c_name = NULL
        '''make_c_str(name)'''
        py_name_string = name.encode('UTF-8') if name is not None else None
        c_name = py_name_string if name is not None else ""

        cdef const char * c_value = NULL
        '''make_c_str(value)'''
        py_value_string = value.encode('UTF-8') if value is not None else None
        c_value = py_value_string if value is not None else ""
        if c_name is not NULL:
            if c_value is not NULL:
                self.thissvptr.setProperty(c_name, c_value)
     def clear_parameters(self):
        """
        clear_parameter(self)
        Clear all parameters set on the processor
        """
        self.thissvptr.clearParameters()
     def clear_properties(self):
        """
        clear_parameter(self)
        Clear all properties set on the processor
        """
        self.thissvptr.clearProperties()

     @property
     def exception_occurred(self):
        """
        exception_occurred(self)
        Property to check if an exception has occurred internally within SaxonC
        Returns:
            boolean: True or False if an exception has been reported internally in SaxonC
        """
        return self.thissvptr.exceptionOccurred()

     def exception_clear(self):
        """
        exception_clear(self)
        Clear any exception thrown
        """
        self.thissvptr.exceptionClear()

     def get_error_message(self, index):
        """
        error_message(self)
        A transformation may have a number of errors reported against it. Get the ith error message if there are any errors

        Args:
            index (int): The i'th exception

        Returns:
            str: The message of the exception. Return None if the exception does not exist.
        """
        return make_py_str(self.thissvptr.getErrorMessage(index))
     def get_error_code(self, index):
        """
        get_error_code(self)
        A transformation may have a number of errors reported against it. Get the error code if there was an error.


        Args:
            index (int): The i'th exception

        Returns:
            str: The error code associated with the exception thrown. Return None if the exception does not exist.
        """
        return make_py_str(self.thissvptr.getErrorCode(index))
     def set_lax(self, lax):
        """
        set_lax(self, lax)
        The validation mode may be either strict or lax. \r
        The default is strict; this method may be called to indicate that lax validation is required. With strict validation,
        validation fails if no element declaration can be located for the outermost element. With lax validation,
        the absence of an element declaration results in the content being considered valid.

        Args:
            lax (boolean): lax True if validation is to be lax, False if it is to be strict
        """
        self.thissvptr.setLax(lax)
cdef class PyXdmValue:
     """Value in the XDM data model. A value is a sequence of zero or more items, each item being either an atomic value or a node. """
     cdef saxoncClasses.XdmValue *thisvptr      # hold a C++ instance which we're wrapping
     def __cinit__(self):
        """
        cinit(self)
        Constructor for PyXdmValue
        """
        if type(self) is PyXdmValue:
            self.thisvptr = new saxoncClasses.XdmValue()
     def __dealloc__(self):
        if type(self) is PyXdmValue and self.thisvptr != NULL:
            if self.thisvptr.getRefCount() < 1:
                del self.thisvptr
            else:
                self.thisvptr.decrementRefCount()
     def add_xdm_item(self, PyXdmItem value):
        """
        add_xdm_tem(self, PyXdmItem value)
        Add PyXdmItem to the Xdm sequence
        Args:
            value (PyXdmItem): The PyXdmItem object
        """
        if value is not None:
            self.thisvptr.addXdmItem(value.derivedptr)

     @property
     def head(self):
        """
        head(self)
        Property to get the first item in the sequence
        Returns:
            PyXdmItem: The PyXdmItem or None if the sequence is empty
        """

        cdef saxoncClasses.XdmItem * xdmItem = NULL
        xdmItem = self.thisvptr.getHead()

        if xdmItem is NULL:
            return None

        cdef PyXdmItem val = PyXdmItem()
        cdef type_ = xdmItem.getType()
        if type_== 4:
            aval = PyXdmAtomicValue()
            xdmItem.incrementRefCount()
            aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>xdmItem
            return aval
        elif type_ == 3:
            nval = PyXdmNode()
            xdmItem.incrementRefCount()
            nval.derivednptr = nval.derivedptr = nval.thisvptr = <saxoncClasses.XdmNode*>xdmItem
            return nval
        elif type_ == 5:
             fval = PyXdmFunctionItem()
             fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>xdmItem
             fval.thisvptr.incrementRefCount()
             return fval
        elif type_ == 6:
             mval = PyXdmMap()
             mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>xdmItem
             mval.thisvptr.incrementRefCount()
             return mval
        elif type_ == 7:
             aaval = PyXdmArray()
             aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>xdmItem
             aaval.thisvptr.incrementRefCount()
             return aaval
        else:
            val = PyXdmItem()
            val.derivedptr = val.thisvptr = xdmItem
            val.derivedptr.incrementRefCount()
            return val

     def item_at(self, index):
        """
        item_at(self, index)
        Get the n'th item in the value, counting from zero.

        Args:
            index (int): the index of the item required. Counting from zero
        Returns:
            PyXdmItem: Get the item indicated at the index. This could be PyXdmNode or PyXdmAtomicValue object. If the item does not exist return None.

        """
        cdef PyXdmValue val = None
        cdef PyXdmAtomicValue aval = None
        cdef PyXdmNode nval = None
        cdef PyXdmItem ival = None
        cdef type_ = None
        cdef saxoncClasses.XdmItem * xdmItem = NULL
        xdmItem = self.thisvptr.itemAt(index)
        if xdmItem == NULL:
            return None
        else :
            type_ = xdmItem.getType()
            xdmItem.incrementRefCount()
            if type_== 4:
                aval = PyXdmAtomicValue()
                aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>xdmItem
                return aval
            elif type_ == 3:
                nval = PyXdmNode()
                nval.derivednptr = nval.derivedptr = nval.thisvptr = <saxoncClasses.XdmNode*>xdmItem
                return nval
            elif type_ == 5:
                fval = PyXdmFunctionItem()
                fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>xdmItem
                fval.thisvptr.incrementRefCount()
                return fval
            elif type_ == 6:
                mval = PyXdmMap()
                mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>xdmItem
                mval.thisvptr.incrementRefCount()
                return mval
            elif type_ == 7:
                aaval = PyXdmArray()
                aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>xdmItem
                aaval.thisvptr.incrementRefCount()
                return aaval
            else:
                ival = PyXdmItem()
                ival.thisvptr = xdmItem
                return ival
     @property
     def size(self):
        """
        size(self)
        Property - Get the number of items in the sequence

        Returns:
            int: The count of items in the sequence
        """
        return self.thisvptr.size()
     def __repr__(self):
        """
        __repr__(self)
        The string representation of PyXdmItem
        """
        cdef const char* c_string = self.thisvptr.toString()
        if c_string == NULL:
            raise Warning('Empty string returned')
        else:
            ustring = make_py_str(c_string)
            return ustring
     def __str__(self):
        """
        __str__(self)
        The string representation of PyXdmItem
        """
        cdef const char* c_string = self.thisvptr.toString()
        ustring = make_py_str(c_string)
        return ustring
     def __iter__(self):
        ''' Returns the Iterator object of PyXdmValue'''
        return PyXdmValueIterator(self)
cdef class PyXdmValueIterator:
     """ Iterator class for the PyXdmValue """
     cdef PyXdmValue _value
     cdef _index
     def __init__(self, value):
     # PyXdmValue object reference
        self._value = value
        # member variable to keep track of current index
        self._index = 0

     def __iter__(self):
         return self

     def __next__(self):
       """Returns the next value from PyXdmValue object's lists """
       if self._index < self._value.size :
           result = self._value.item_at(self._index)
           self._index +=1
           return result
       # End of Iteration
       raise StopIteration



cdef class PyXdmItem(PyXdmValue):
     """The class XdmItem represents an item in a sequence, as defined by the XDM data model.
     An item is either an atomic value or a node."""
     cdef saxoncClasses.XdmItem *derivedptr      # hold a C++ instance which we're wrapping
     def __cinit__(self):
        """
         cinit(self)
         Constructor for PyXdmItem
        """
        if type(self) is PyXdmItem:
            self.derivedptr = self.thisvptr = new saxoncClasses.XdmItem()
     def __dealloc__(self):
        if type(self) is PyXdmItem and self.derivedptr != NULL:
            if self.derivedptr.getRefCount() < 1:
                del self.derivedptr
            else:
                self.derivedptr.decrementRefCount()
        '''if type(self) is PyXdmItem:
            del self.derivedptr'''
     @property
     def string_value(self):
        """
        string_value(self)
        Property to get the the string value of the XdmItem
        """
        cdef const char* c_string = self.derivedptr.getStringValue()
        ustring = make_py_str(c_string)
        return ustring
     
     def __repr__(self):
        return make_py_str(self.derivedptr.toString())
     def __str__(self):
        return make_py_str(self.derivedptr.toString())

     @property
     def is_atomic(self):
        """
        is_atomic(self)
        Property to check if the current PyXdmItem is an atomic value

        Returns:
            bool: Check of is atomic value
        """
        return self.derivedptr.isAtomic()

     @property
     def is_node(self):
        """
        is_atomic(self)
        Property to check if the current PyXdmItem is a node value

        Returns:
            bool: Check for node value
        """
        return self.derivedptr.isNode()

     @property
     def is_function(self):
        """
        is_atomic(self)
        Property to check if the current PyXdmItem is a function value

        Returns:
            bool: Check of is atomic value
        """
        return self.derivedptr.isFunction()

     @property
     def is_map(self):
        """
        is_map(self)
        Property to check if the current PyXdmItem is a map value

        Returns:
            bool: Check for map value
        """
        return self.derivedptr.isMap()

     @property
     def is_array(self):
        """
        is_array(self)
        Property to check if the current PyXdmItem is an XdmArray value

        Returns:
            bool: Check for array value
        """
        return self.derivedptr.isArray()

     def get_node_value(self):
        """
        get_node_value(self)
        Get the subclass PyXdmNode for this PyXdmItem object current object if it is a node value

        Returns:
            PyXdmNode: Subclass this object to PyXdmNode or error
        """
        cdef PyXdmNode val = None
        if self.is_atomic:
          raise Exception("The PyXdmItem is an PyXdmAtomicValue therefore cannot be sub-classed to an PyXdmNode")
        val = PyXdmNode()
        val.derivednptr = val.derivedptr = <saxoncClasses.XdmNode*> self.derivedptr
        '''val.derivednptr.incrementRefCount()'''
        return val

     def get_map_value(self):
        """
        get_node_value(self)
        Get the subclass PyXdmNode for this PyXdmItem object current object if it is a node value

        Returns:
            PyXdmNode: Subclass this object to PyXdmNode or error
        """
        cdef PyXdmMap val = None
        if self.is_atomic:
          raise Exception("The PyXdmItem is an PyXdmAtomicValue therefore cannot be sub-classed to an PyXdmNode")
        val = PyXdmMap()
        val.derivedmmptr = val.derivedptr = <saxoncClasses.XdmMap*> self.derivedptr
        '''val.derivednptr.incrementRefCount()'''
        return val
     @property
     def head(self):
        """
        head(self)
        Property to get the first item in the sequence. This would be the PyXdmItem itself as there is only one item in the sequence
        Returns:
            PyXdmItem: The PyXdmItem or None if the sequence is empty
        """
        return self
     def get_atomic_value(self):
        """
        get_atomic_value(self)
        Get the subclass PyXdmAtomicValue for this PyXdmItem object current object if it is an atomic value

        Returns:
            PyXdmAtomicValue: Subclass this object to PyXdmAtomicValue or error
        """
        if self.is_atomic == False:
          raise Exception("The PyXdmItem is not an PyXdmAtomicValue")
        val = PyXdmAtomicValue()
        val.derivedaptr = val.derivedptr = <saxoncClasses.XdmAtomicValue*>self.derivedptr
        val.derivedaptr.incrementRefCount()
        return val
cdef class PyXdmNode(PyXdmItem):
     """This class represents a node in the XDM data model. A Node is an <code>XdmItem</code>, and is therefore an
     <code>XdmValue</code> in its own right, and may also participate as one item within a sequence value.
     <p>The XdmNode interface exposes basic properties of the node, such as its name, its string value, and     """
     cdef saxoncClasses.XdmNode *derivednptr      # hold a C++ instance which we're wrapping
     def __cinit__(self):
        """
         cinit(self)
         Constructor for PyXdmNode
        """
        self.derivednptr = self.derivedptr = self.thisvptr = NULL

     def __dealloc__(self):
        if type(self) is PyXdmNode and self.derivednptr != NULL:
                 if self.derivednptr.getRefCount() < 1:
                     del self.derivednptr
                 else:
                     self.derivednptr.decrementRefCount()
     @property
     def head(self):
        """
        head(self)
        Property to get the first item in the sequence. This would be the PyXdmNode itself as there is only one item in the sequence
        Returns:
            PyXdmItem: The PyXdmItem or None if the sequence is empty
        """
        return self

     @property
     def node_kind(self):
        """
        node_kind(self)
        Node Kind property. This will be a value such as {@link net.sf.saxon.type.Type#ELEMENT} or {@link net.sf.saxon.type.Type#ATTRIBUTE}.
        There are seven kinds of node: documents, elements, attributes, text, comments, processing-instructions, and namespaces.
        Returns:
            int: an integer identifying the kind of node. These integer values are the same as those used in the DOM
        """
        cdef int kind
        return self.derivednptr.getNodeKind()
     @property
     def node_kind_str(self):
        """
        node_kind(self)
        Node Kind property string. This will be a value such as {@link net.sf.saxon.type.Type#ELEMENT} or {@link net.sf.saxon.type.Type#ATTRIBUTE}.
        There are seven kinds of node: documents, elements, attributes, text, comments, processing-instructions, and namespaces.
        Returns:
            int: an integer identifying the kind of node. These integer values are the same as those used in the DOM
        """
        cdef str kind
        cdef int nk = self.derivednptr.getNodeKind()
        if nk == DOCUMENT:
            return repr('document')
        elif nk == ELEMENT:
            return repr('element')
        elif nk == ATTRIBUTE:
            return repr('attribute')
        elif nk == TEXT:
            return repr('text')
        elif nk == COMMENT:
            return repr('comment')
        elif nk == PROCESSING_INSTRUCTION:
            return repr('processing-instruction')
        elif nk == NAMESPACE:
            return repr('namespace')
        elif nk == UNKNOWN:
            return repr('unknown')
        else:
            raise ValueError('Unknown node kind: %d' % nk)

     @property
     def name(self):
        """
        name(self)
        Get the name of the node, as a string in the form of a EQName
        Returns:
            str: the name of the node. In the case of unnamed nodes (for example, text and comment nodes) return None
        """
        cdef const char* c_string = self.derivednptr.getNodeName()
        if c_string == NULL:
            return None
        else:
            ustring = make_py_str(c_string)
            return ustring
     @property
     def typed_value(self):
        """
        typed_value(self)
        Property - get the typed value of this node, as defined in XDM
        Returns:
            PyXdmValue:the typed value. If the typed value is a single atomic value, this will be returne as an instance of {@link XdmAtomicValue}
        """
        cdef PyXdmValue val = None
        cdef saxoncClasses.XdmValue * xdmValue = self.derivednptr.getTypedValue()
        if xdmValue == NULL:
            return None
        else:
            val = PyXdmValue()
            val.thisvptr = xdmValue
            return val

     @property
     def base_uri(self):
        """
        base_uri(self)
        Base uri Property. Get the Base URI for the node, that is, the URI used for resolving a relative URI contained in the node.
        This will be the same as the System ID unless xml:base has been used. Where the node does not have a base URI of its own,
        the base URI of its parent node is returned.
        Returns:
            str: String value of the base uri for this node. This may be NULL if the base URI is unknown, including the case
                 where the node has no parent.
        """
        return make_py_str(self.derivednptr.getBaseUri())

     @property
     def string_value(self):
        """
        string_value(self)
        Property to get the string value of the node as defined in the XPath data model.
        Returns:
            str: The string value of this node
        """
        cdef const char* c_string = self.derivednptr.getStringValue()
        ustring = make_py_str(c_string)
        return ustring
     def __str__(self):
        """
        __str__(self)
        The string value of the node as defined in the XPath data model
        Returns:
            str: String value of this node
        """
        cdef const char* c_string = self.derivednptr.toString()
        ustring = make_py_str(c_string)
        return ustring
     def __repr__(self):
        """
        ___repr__
        """
        cdef const char* c_string = self.derivednptr.toString()
        ustring = make_py_str(c_string)
        return ustring
     def get_parent(self):
        """
        get_parent(self)
        Get the current node's parent
        Returns:
            PyXdmNode: The parent node as PyXdmNode object
        """
        cdef PyXdmNode val = PyXdmNode()
        val.derivednptr = val.derivedptr = val.thisvptr = self.derivednptr.getParent()
        return val
     def get_attribute_value(self, name):
        """
        getAttribute_value(self, name)
        The name of the required attribute

        Args:
            name(str): the eqname of the required attribute
        """
        py_value_string = name.encode('UTF-8') if name is not None else None
        cdef char * c_name = py_value_string if name is not None else ""

        cdef const char* c_string = self.derivednptr.getAttributeValue(c_name)
        ustring = make_py_str(c_string)

        return ustring
     @property
     def attribute_count(self):
        """
        attribute_count(self)
        Property to get the count of attribute nodes on this XdmNode object. If this current node is not an element node then return 0
        Returns:
            int: Count of attribute nodes
        """
        return self.derivednptr.getAttributeCount()
     @property
     def attributes(self):
        """
        attribute_nodes(self)
        Property to get the attribute nodes as a list of PyXdmNode objects
        Returns:
            list[PyXdmNode]: List of PyXdmNode objects
        """
        cdef list nodes = []
        cdef saxoncClasses.XdmNode **n
        cdef int count, i
        cdef PyXdmNode val = None
        count = self.derivednptr.getAttributeCount()
        if count > 0:
            n = self.derivednptr.getAttributeNodes()
            for i in range(count):
                val = PyXdmNode()
                val.derivednptr = val.derivedptr = val.thisvptr = n[i]
                val.derivednptr.incrementRefCount()
                nodes.append(val)
        return nodes
     @property
     def children(self):
        """
        children(self)
        Property to get children of this current node. List of child nodes
        Returns:
            list[PyXdmNode]: List of PyXdmNode objects
        """
        cdef list nodes = []
        cdef saxoncClasses.XdmNode **n
        cdef int count, i
        cdef PyXdmNode val = None
        count = self.derivednptr.getChildCount()
        if count > 0:
            n = self.derivednptr.getChildren()
            for i in range(count):
                val = PyXdmNode()
                val.derivednptr = val.derivedptr = val.thisvptr = n[i]
                val.derivednptr.incrementRefCount()
                nodes.append(val)
        return nodes
      # def getChildCount(self):

cdef class PyXdmAtomicValue(PyXdmItem):
     """
     The class PyXdmAtomicValue represents an item in an Xath sequence that is an atomic value. The value may belong to any of the
     19 primitive types defined in XML Schema, or to a type derived from these primitive types, or the XPath type xs:untypedAtomic.
     """
     cdef saxoncClasses.XdmAtomicValue *derivedaptr      # hold a C++ instance which we're wrapping
     def __cinit__(self):
        if type(self) is PyXdmAtomicValue:
            self.derivedaptr = self.derivedptr = self.thisvptr = new saxoncClasses.XdmAtomicValue()
     def __dealloc__(self):
        if type(self) is PyXdmAtomicValue and self.derivedaptr != NULL:
            if self.derivedaptr.getRefCount() < 1:
                del self.derivedaptr
            else:
                self.derivedaptr.decrementRefCount()


     @property
     def primitive_type_name(self):
        """
        get_primitive_type_name()
        Property - Get the primitive type name of the PyXdmAtomicValue
        Returns:
            str: String of the primitive type name
        """
        ustring = make_py_str(self.derivedaptr.getPrimitiveTypeName())
        return ustring
     @property
     def boolean_value(self):
        """
        Property which returns the boolean value of the PyXdmAtomicValue
        Returns:
            bool: boolean value.
        """
        return self.derivedaptr.getBooleanValue()
     @property
     def double_value(self):
        """
        Property which is returns the double value of the PyXdmAtomicValue if it can be converted.
        Returns:
            double: Double value of the Xdm object
        """

        return self.derivedaptr.getDoubleValue()
     @property
     def head(self):
        """
        head(self)
        Property to get the first item in the sequence. This would be the PyXdmAtomicValue itself as there is only one item in the sequence
        Returns:
            PyXdmAtomicValue: The PyXdmAtomic or None if the sequence is empty
        """
        return self

     @property
     def integer_value(self):
        """
        Property which is returns the int value of the PyXdmAtomicValue if it can be converted.
        Returns:
            int: Int value of the Xdm object
        """

        return self.derivedaptr.getLongValue()
     @property
     def string_value(self):
        """
        Property which returns the string value of the PyXdmAtomicValue
        Returns:
            str: String value of the Xdm object
        """
        ustring = make_py_str(self.derivedaptr.getStringValue())
        return ustring


     def __int__(self):

         return self.integer_value



     def __str__(self):
        """
        __str__(self)
        The string value of the node as defined in the XPath data model
        Returns:
            str: String value of this node
        """
        ustring = make_py_str(self.derivedaptr.toString())
        return ustring
     def __repr__(self):
        """
        ___repr__
        """
        ustring = make_py_str(self.derivedaptr.toString())
        return ustring

     def __hash__(self):
        return self.derivedaptr.getHashCode()

     def __eq__(self, other):
         if isinstance(other, int):
             return  self.integer_value == other
         elif  isinstance(other, float):
             return self.double_value == other
         elif  isinstance(other, str):
             return (self.string_value) == (other.string_value)
         elif  other  in (True, False):
             return self.boolean_value == other
         return False

     def __ne__(self, other):
        # Not strictly necessary, but to avoid having both x==y and x!=y
        # True at the same time
        return not(self == other)

cdef class PyXdmFunctionItem(PyXdmItem):
     """
     The class PyXdmFunctionItem represents a function item
     """
     cdef saxoncClasses.XdmFunctionItem *derivedfptr      # hold a C++ instance which we're wrapping
     def __cinit__(self):
        if type(self) is PyXdmAtomicValue:
            self.derivedfptr = self.derivedptr = self.thisvptr = new saxoncClasses.XdmFunctionItem()
     def __dealloc__(self):
        if type(self) is PyXdmFunctionItem and self.derivedfptr != NULL:
            if self.derivedfptr.getRefCount() < 1:
                del self.derivedfptr
            else:
                self.derivedfptr.decrementRefCount()

     @property
     def name(self):
         """
         name(self)
         Get the name of the function
         """
         ustring = make_py_str(self.derivedfptr.getName())
         return ustring

     @property
     def arity(self):
         """
         arity(self)
         Property - Get the arity of the function

         :returns:
             int: The arity of the function, that is, the number of arguments in the function's signature
         """
         return self.derivedfptr.getArity()

     def __repr__(self):
        return make_py_str(self.derivedfptr.toString())
     def __str__(self):
        return make_py_str(self.derivedfptr.toString())

     @property
     def string_value(self):
        """
        Property which returns the string value of the PyXdmFunctionItem
        Returns:
            str: String value of the Xdm object
        """
        ustring = make_py_str(self.derivedfptr.getStringValue())
        return ustring

     def get_system_function(self, PySaxonProcessor proc, str name, arity):
         """
         Get a system function. This can be any function defined in XPath 3.1 functions and operators,
         including functions in the math, map, and array namespaces. It can also be a Saxon extension
         function, provided a licensed Processor is used.
         :return:
         """
         cdef char * c_str = NULL
         cdef PyXdmFunctionItem func = None
         cdef saxoncClasses.XdmFunctionItem * c_func = NULL
         if proc is None:
             return None
         c_func = self.derivedfptr.getSystemFunction(proc.thisptr, make_c_str(name), arity)
         if c_func is NULL:
             return None
         func = PyXdmFunctionItem()
         func.derivedfptr = self.derivedptr = self.thisvptr = c_func
         return func


     def call(self, PySaxonProcessor proc, list args):
         """
         call(self, list args)
         Call the function
         :param args: arguments the values to be supplied as arguments to the function. The "function
                      conversion rules" will be applied to con
         :return: PyXdmValue - the result of calling the function
         """
         cdef int _len = len(args)
         cdef saxoncClasses.XdmValue ** argumentV = self.derivedfptr.createXdmValueArray(_len)
         cdef PyXdmValue ivalue_
         for x in range(_len):
           if isinstance(args[x], PyXdmValue):
             ivalue_ = args[x]
             argumentV[x] = ivalue_.thisvptr
           else:
             raise Exception("Argument value at position ",x," is not an PyXdmValue")
         '''c_functionName = make_c_str(function_name)'''
         cdef saxoncClasses.XdmValue * c_xdmValue = NULL
         cdef PyXdmAtomicValue avalue_
         cdef PyXdmValue value_
         cdef PyXdmAtomicValue aval = None
         cdef PyXdmNode nval = None
         cdef PyXdmFunctionItem fval = None
         cdef PyXdmMap mval = None
         cdef PyXdmArray aaval = None
         c_xdmValue = self.derivedfptr.call(proc.thisptr, argumentV, _len)
         type_ = c_xdmValue.getType()
         if type_== 1:
              value_ = PyXdmValue()
              value_.thisvptr = c_xdmValue
              value_.thisvptr.incrementRefCount()
              return value_
         if type_== 4:
              aval = PyXdmAtomicValue()
              aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>c_xdmValue
              aval.thisvptr.incrementRefCount()
              return aval
         elif type_ == 3:
              nval = PyXdmNode()
              nval.derivednptr = nval.derivedptr = nval.thisvptr = <saxoncClasses.XdmNode*>c_xdmValue
              nval.thisvptr.incrementRefCount()
              return nval
         elif type_ == 5:
              fval = PyXdmFunctionItem()
              fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>c_xdmValue
              fval.thisvptr.incrementRefCount()
              return fval
         elif type_ == 6:
              mval = PyXdmMap()
              mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>c_xdmValue
              mval.thisvptr.incrementRefCount()
              return mval
         elif type_ == 7:
              aaval = PyXdmArray()
              aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>c_xdmValue
              aaval.thisvptr.incrementRefCount()
              return aaval
         else:
              return None

cdef class PyXdmMap(PyXdmFunctionItem):
     """
     The class PyXdmMap represents a map in the XDM data model. A map is a list of zero or more entries, each of which
     is a pair comprising a key (which is an atomic value) and a value (which is an arbitrary value).
     The map itself is an XDM item. <p>An XdmMap is immutable.</p>
     """
     cdef saxoncClasses.XdmMap *derivedmmptr      # hold a C++ instance which we're wrapping
     def __cinit__(self):
        """
         cinit(self)
         Constructor for PyXdmMap
        """
        if type(self) is PyXdmMap:
            self.derivedmmptr = self.derivedfptr = self.derivedptr = self.thisvptr = new saxoncClasses.XdmMap()
     def __dealloc__(self):
        if type(self) is PyXdmMap and self.derivedfptr != NULL:
            if self.derivedfptr.getRefCount() < 1:
                del self.derivedfptr
            else:
                self.derivedfptr.decrementRefCount()


     def __repr__(self):
        return make_py_str(self.derivedmmptr.toString())
     def __str__(self):
        return make_py_str(self.derivedmmptr.toString())

     @property
     def map_size(self):
         """
         map_size(self)
         Get the number of entries in the map
         Returns:
             the number of entries in the map. (Note that the <code>size()</code> method returns 1 (one), because an XDM map is an item.)
         """
         return self.derivedmmptr.mapSize()

     def get(self, key):
         """
         get(self, key)
         Returns the value to which the specified key is mapped, or NULL if this map contains no mapping for the key.
         Args:
               key  -   the key whose associated value is to be returned. The key supports the following types XdmAtomicValue, str, int and float
         Returns:
             PyXdmValue  - the value to which the specified key is mapped, or NULL if this map contains no mapping for the key
         """
         cdef saxoncClasses.XdmValue * c_xdmValue = NULL
         cdef PyXdmAtomicValue avalue_
         cdef PyXdmValue value_
         cdef PyXdmAtomicValue aval = None
         cdef PyXdmNode nval = None
         cdef PyXdmFunctionItem fval = None
         cdef PyXdmMap mval = None
         cdef PyXdmArray aaval = None
         cdef int i_key
         cdef float k_key
         cdef str keyStr
         cdef type_ = 0
         cdef char * c_key_str = NULL

         if isinstance(key, PyXdmAtomicValue):
             avalue_ = key
             c_xdmValue = self.derivedmmptr.get(avalue_.derivedaptr)

         elif  isinstance(key, str):
             keyStr = key
             py_value_string = keyStr.encode('UTF-8') if keyStr is not None else None
             c_key_str = py_value_string if keyStr is not None else ""
             c_xdmValue = self.derivedmmptr.get(c_key_str)

         elif  isinstance(key, int):
             i_key = key
             c_xdmValue = self.derivedmmptr.get(i_key)

         elif isinstance(key, float):
             k_key = key
             c_xdmValue = self.derivedmmptr.get(k_key)
         else:
             return None

         if c_xdmValue is NULL:
             return None
         else:


             type_ = c_xdmValue.getType()
             if type_== 1:
                 value_ = PyXdmValue()
                 value_.thisvptr = c_xdmValue
                 value_.thisvptr.incrementRefCount()
                 return value_
             if type_== 4:
                 aval = PyXdmAtomicValue()
                 aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>c_xdmValue
                 aval.thisvptr.incrementRefCount()
                 return aval
             elif type_ == 3:
                 nval = PyXdmNode()
                 nval.derivednptr = nval.derivedptr = nval.thisvptr = <saxoncClasses.XdmNode*>c_xdmValue
                 nval.thisvptr.incrementRefCount()
                 return nval
             elif type_ == 5:
                 fval = PyXdmFunctionItem()
                 fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>c_xdmValue
                 fval.thisvptr.incrementRefCount()
                 return fval
             elif type_ == 6:
                 mval = PyXdmMap()
                 mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>c_xdmValue
                 mval.thisvptr.incrementRefCount()
                 return mval
             elif type_ == 7:
                 aaval = PyXdmArray()
                 aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>c_xdmValue
                 aaval.thisvptr.incrementRefCount()
                 return aaval
             else:
                 return None

     def __iter__(self):
        ''' Returns the Iterator object of PyXdmMap'''
        return iter(self.keys())


     def put(self, PyXdmAtomicValue key, PyXdmValue  value):
         """
         put(self, PYXdmAtomicValue key, PyXdmValue value)
         Create a new map containing an additional (key, value) pair. If there is an existing entry with the same key, it is removed
         Args:
               key  -   The key to be removed given as an XdmAtomicValue
         Returns:
             PyXdmMap  - a new map containing the additional entry. The original map is unchanged.
         """
         cdef PyXdmMap newmap_
         cdef saxoncClasses.XdmMap * c_xdmMap = NULL

         if key is None or value is None:
             return None

         c_xdmMap = self.derivedmmptr.put(key.derivedaptr, value.thisvptr)
         if c_xdmMap is NULL:
             return None
         newmap_ = PyXdmMap()
         newmap_.derivedmmptr = newmap_.derivedfptr = newmap_.derivedptr = newmap_.thisvptr = c_xdmMap
         return newmap_



     def remove(self, PyXdmAtomicValue key):
         """
         remove(self, PYXdmAtomicValue key)
         Create a new map in which the entry for a given key has been removed.
         Args:
               key  -   The key to be removed given as an XdmAtomicValue
         Returns:
             PyXdmMap  - a map without the specified entry. The original map is unchanged.
         """
         cdef PyXdmMap newmap_
         cdef saxoncClasses.XdmMap * c_xdmMap = NULL

         if key is None:
             return None

         c_xdmMap = self.derivedmmptr.remove(key.derivedaptr)
         if c_xdmMap is NULL:
             return None
         newmap_ = PyXdmMap()
         newmap_.derivedmmptr = newmap_.derivedfptr = newmap_.derivedptr = newmap_.thisvptr = c_xdmMap
         return newmap_

     def keys(self):
         """
         keys(self)
         Keys in the XdmMap
         :return: list of PyXdmAtomicValues
         """
         """TODO: memory management for the array returned from C++"""
         cdef list p_values = []
         cdef saxoncClasses.XdmAtomicValue **c_values
         cdef PyXdmAtomicValue val = None
         cdef int count, i
         count = self.derivedmmptr.mapSize()
         if count > 0:
             c_values = self.derivedmmptr.keys()
             for i in range(count):
                 val = PyXdmAtomicValue()
                 val.derivedaptr = val.derivedptr = val.thisvptr = <saxoncClasses.XdmAtomicValue *>c_values[i]
                 val.thisvptr.incrementRefCount()
                 p_values.append(val)
         return p_values

     '''def map[string, XdmValue*]& asMap()'''

     @property
     def isEmpty(self):
         """
         isEmpty(self, PYXdmAtomicValue key)
         Returns true if this map contains no key-value mappings.

         Returns:
             true if this map contains no key-value mappings
         """
         self.derivedmmptr.isEmpty()

     def contains_key(self, PyXdmAtomicValue key):
         """
         contains_key(self, PYXdmAtomicValue key)
         Returns true if this map contains a mapping for the specified key.
         Args:
               key (PyXdmAtomicValue) - key key whose presence in this map is to be tested
         Returns:
             true if this map contains a mapping for the specified key
         """
         if key is None:
             return None

         return self.derivedmmptr.containsKey(key.derivedaptr)

     @property
     def string_value(self):
        """
        Property which returns the string value of the PyXdmMap
        Returns:
            str: String value of the Xdm object
        """
        ustring = make_py_str(self.derivedmmptr.getStringValue())
        return ustring

     def values(self):
         """
         values(self)
         :return:
         """
         """TODO: memory management for the array returned from C++"""
         cdef list p_values = []
         cdef saxoncClasses.XdmValue ** c_values
         cdef int count, i
         cdef PyXdmValue val = None
         cdef PyXdmAtomicValue aval = None
         cdef PyXdmNode nval = None
         cdef PyXdmFunctionItem fval = None
         cdef PyXdmMap mval = None
         cdef PyXdmArray aaval = None
         cdef type_ = 0
         count = self.derivedmmptr.mapSize()
         if count > 0:
             c_values = self.derivedmmptr.values()
             if c_values is NULL:
                 return None
             for i in range(count):

                 type_ = c_values[i].getType()
                 if type_== 1:
                     val = PyXdmValue()
                     val.thisvptr = c_values[i]
                     val.thisvptr.incrementRefCount()
                     p_values.append(val)
                 if type_== 4:
                     aval = PyXdmAtomicValue()
                     aval.derivedaptr = aval.derivedptr = aval.thisvptr = <saxoncClasses.XdmAtomicValue *>c_values[i]
                     aval.thisvptr.incrementRefCount()
                     p_values.append(aval)
                 elif type_ == 3:
                     nval = PyXdmNode()
                     nval.derivednptr = nval.derivedptr = nval.thisvptr = <saxoncClasses.XdmNode*>c_values[i]
                     nval.thisvptr.incrementRefCount()
                     p_values.append(nval)
                 elif type_ == 5:
                     fval = PyXdmFunctionItem()
                     fval.derivedfptr = fval.derivedptr = fval.thisvptr = <saxoncClasses.XdmFunctionItem*>c_values[i]
                     fval.thisvptr.incrementRefCount()
                     p_values.append(fval)
                 elif type_ == 6:
                     mval = PyXdmMap()
                     mval.derivedmmptr = mval.derivedfptr = mval.derivedptr = mval.thisvptr = <saxoncClasses.XdmMap*>c_values[i]
                     mval.thisvptr.incrementRefCount()
                     p_values.append(mval)
                 elif type_ == 7:
                     aaval = PyXdmArray()
                     aaval.derivedaaptr = aaval.derivedfptr = aaval.derivedptr = aaval.thisvptr = <saxoncClasses.XdmArray*>c_values[i]
                     aaval.thisvptr.incrementRefCount()
                     p_values.append(aaval)


         return p_values



cdef class PyXdmArray(PyXdmFunctionItem):
     """
     The class PyXdmArray represents an array in the XDM data model. An array is a list of zero or more members, each of which
       is an arbitrary XDM value. The array itself is an XDM item.
     """
     cdef saxoncClasses.XdmArray *derivedaaptr      # hold a C++ instance which we're wrapping
     def __cinit__(self):
        if type(self) is PyXdmArray:
            self.derivedaaptr = self.derivedfptr = self.derivedptr = self.thisvptr = new saxoncClasses.XdmArray()
     def __dealloc__(self):
        if type(self) is PyXdmArray and self.derivedaaptr != NULL:
            if self.derivedaaptr.getRefCount() < 1:
                del self.derivedaaptr
            else:
                self.derivedaaptr.decrementRefCount()


     def __repr__(self):
        return make_py_str(self.derivedaaptr.toString())
     def __str__(self):
        return make_py_str(self.derivedaaptr.toString())

     def __iter__(self):
        ''' Returns the Iterator object of PyXdmArray'''
        return iter(self.as_list())

     @property
     def array_length(self):
         """
         Get the number of members in the array
         Returns:
             int: the number of members in the array. (Note that the size() method returns 1 (one),
              because an XDM array is an item.)
         """
         return self.derivedaaptr.arrayLength()

     @property
     def string_value(self):
        """
        Property which returns the string value of the PyXdmArray
        Returns:
            str: String value of the Xdm object
        """
        ustring = make_py_str(self.derivedaaptr.getStringValue())
        return ustring

     def get(self, int n):
         """
         get(self, int n)
         Get the n'th member in the array, counting from zero
         Args:
               n (int): the member that is required, counting the first member in the array as member zero
         Returns:
             PyXdmValue: the n'th member in the sequence making up the array, counting from zero
         """
         cdef saxoncClasses.XdmValue * c_xdmValue = NULL
         cdef PyXdmValue value_

         c_xdmValue = self.derivedaaptr.get(n)

         if c_xdmValue is NULL:
             return None
         else:
             value_ = PyXdmValue()
             value_.thisvptr = c_xdmValue
             return value_

     def put(self, int n, PyXdmValue value):
         """
         put(self, int n, PyXdmValue value)
         Create a new array in which one member is replaced with a new value.
         Args:
               n (int): n the position of the member that is to be replaced, counting the first member in the array as member zero
               value (PyXdmValue): the new value for this member
         Returns:
             PyXdmArray: a new array, the same length as the original, with one member replaced by a new value
         """
         cdef PyXdmArray newarr_
         cdef PyXdmAtomicValue avalue_
         cdef PyXdmItem ivalue_
         cdef PyXdmNode nvalue_
         cdef PyXdmValue value_
         cdef saxoncClasses.XdmArray * c_xdmArr = NULL
         cdef saxoncClasses.XdmValue * c_in_xdmValue = NULL

         if value is None or n<0:
             return None

         if isinstance(value, value):
             value_ = value
             c_xdmArr = self.derivedaaptr.put(n, value.thisvptr)
         elif  isinstance(value, PyXdmNode):
             nvalue_ = value
             c_in_xdmValue  = <saxoncClasses.XdmValue*>  nvalue_.derivednptr
             c_xdmArr = self.derivedaaptr.put(n, c_in_xdmValue)
         elif  isinstance(value, PyXdmAtomicValue):
             avalue_ = value
             c_in_xdmValue  = <saxoncClasses.XdmValue*> avalue_.derivedaptr
             c_xdmArr = self.derivedaaptr.put(n, c_in_xdmValue)
         elif  isinstance(value, PyXdmItem):
             ivalue_ = value
             c_in_xdmValue  = <saxoncClasses.XdmValue*>  ivalue_.derivedptr
             c_xdmArr = self.derivedaaptr.put(n, c_in_xdmValue)

         if c_xdmArr is NULL:
             return None
         newarr_ = PyXdmArray()
         newarr_.derivedaaptr = newarr_.derivedfptr = newarr_.derivedptr = newarr_.thisvptr = c_xdmArr
         return newarr_

     def add_member(self, value):
         """
         add_member(self, value)
         Create a new array in which one member is replaced with a new value.
         Args:
               value (PyXdmValue): the new member as PYXdmValue or any of its subclasses
         Returns:
             PyXdmArray: a new array, one item longer than the original
         """

         cdef PyXdmArray newarr_
         cdef PyXdmAtomicValue avalue_
         cdef PyXdmItem ivalue_
         cdef PyXdmNode nvalue_
         cdef PyXdmValue value_
         cdef saxoncClasses.XdmArray * c_xdmArr = NULL

         if isinstance(value, PyXdmValue):
             value_ = value
             c_xdmArr = self.derivedaaptr.addMember(value_.thisvptr)
         elif  isinstance(value, PyXdmNode):
             nvalue_ = value
             c_xdmArr = self.derivedaaptr.addMember(<saxoncClasses.XdmValue*>  nvalue_.derivednptr)
         elif  isinstance(value, PyXdmAtomicValue):
             avalue_ = value
             c_xdmArr = self.derivedaaptr.addMember(<saxoncClasses.XdmValue*> avalue_.derivedaptr)
         elif  isinstance(value, PyXdmItem):
             ivalue_ = value
             c_xdmArr = self.derivedaaptr.addMember(<saxoncClasses.XdmValue*>  ivalue_.derivedptr)

         if c_xdmArr is NULL:
             return None
         newarr_ = PyXdmArray()
         newarr_.derivedaaptr = newarr_.derivedfptr = newarr_.derivedptr = newarr_.thisvptr = c_xdmArr
         return newarr_

     def concat(self, PyXdmArray value):
         """
         concat(self, PyXdmArray value)
         Concatenate another array to this array
         Args:
               value (PyXdmArray): the other array
         Returns:
             PyXdmArray: a new array, containing the members of this array followed by the members of the other array
         """
         cdef PyXdmArray newarr_

         cdef saxoncClasses.XdmArray * c_xdmArr = NULL

         c_xdmArr = self.derivedaaptr.concat(value.derivedaaptr)

         if c_xdmArr is NULL:
             return None
         newarr_ = PyXdmArray()
         newarr_.derivedaaptr = newarr_.derivedfptr = newarr_.derivedptr = newarr_.thisvptr = c_xdmArr
         return newarr_

     def as_list(self):
         """
         as_list(self)
         Get the members of the array in the form of a list.
         Returns:
               list: list of the members of this array.
         """
         """
         TODO: handle memory management"""
         cdef list p_values = []
         cdef int count, i
         count = self.derivedaaptr.arrayLength()
         cdef saxoncClasses.XdmValue **c_values

         cdef PyXdmValue val = None
         if count > 0:
             c_values = self.derivedaaptr.values()
             for i in range(count):
                 val = PyXdmValue()
                 val.thisvptr = c_values[i]
                 val.thisvptr.incrementRefCount()
                 p_values.append(val)
         return p_values

     @property
     def arity(self):
         """
         arity(self)
         Get the arity of the function
         Returns:
             int: the arity of the function, that is, the number of arguments in the function's signature
         """
         return 1

     
_o=PySaxonProcessor(None, False, True)       #  This is important to prevent Jet calling release until all Saxon objects have been garbage collected!
