////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Saxonica Limited.
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
// This Source Code Form is "Incompatible With Secondary Licenses", as defined by the Mozilla Public License, v. 2.0.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SAXON_XDMVALUE_H
#define SAXON_XDMVALUE_H


#include <string.h>
#include "SaxonProcessor.h"
#include <typeinfo> //used for testing only
#include <vector>
#include <deque>
#include <list>
#include "SaxonCGlue.h"
#include "SaxonCXPath.h"





class SaxonProcessor;
class XdmItem;
class XdmAtomicValue;
class XdmNode;



typedef enum eXdmType { XDM_VALUE = 1, XDM_ITEM = 2, XDM_NODE = 3, XDM_ATOMIC_VALUE = 4, XDM_FUNCTION_ITEM = 5, XDM_MAP=6, XDM_ARRAY=7 } XDM_TYPE;

typedef enum saxonTypeEnum
{
	enumNode,
	enumString,
	enumInteger,
	enumDouble,
	enumFloat,
	enumBool,
	enumArrXdmValue
} PRIMITIVE_TYPE;


/*! <code>XdmValue</code>. Value in the XDM data model. A value is a sequence of zero or more items,
 * each item being either an atomic value or a node. This class is a wrapper of the the XdmValue object created in Java.
 * <p/>
 */
class XdmValue {


public:

    //! A default Constructor.
	/**
		 * Create a empty value
		 */
    XdmValue() {
        initialize();
    }


    //! A copy constructor.
    /**
	 *
	 * @param other - Xdmvalue
	 */
	XdmValue(const XdmValue &other);


	//! Add an XdmItem  to the sequence
	 /**
	  * This method is designed for the primitive types.
	  * @param type - specify target type of the value
	  * @param val - Value to convert
	  */
	XdmValue * addXdmValueWithType(const char * tStr, const char * val);//TODO check and document

	/**
	 * Add an XdmItem to the sequence.
	 * See functions in SaxonCXPath of the C library
	 * @param val - XdmItem object
	 */
	void addXdmItem(XdmItem *val);

	/**
	 * Add an Java XdmValue object to the sequence.
	 * See methods the functions in SaxonCXPath of the C library
	 * @param val - Java object
	 */
	void addUnderlyingValue(jobject val);




	//! A Constructor for handling XdmArray - Internal user only
	/**
	 * Handles a sequence of XdmValues given as a  wrapped an Java XdmValue object.
	 * @param val - Java XdmValue object
	 * @param arrFlag - Currently not used but allows for overloading of constructor methods
	 */
	XdmValue(jobject val, bool arrFlag);

	//! A Constructor
	/**
	 * Wrap a Java XdmValue object.
	 * @param val - Java XdmValue object
	 */
	XdmValue(jobject val);

    //! Destructor.
    /**
     * Destructor for this XdmValue
     */
	virtual ~XdmValue();

	//! Delete the XdmValue object and clean up all items in the sequence. Release the underlying JNI object
    /*
     *
     */
	void releaseXdmValue();

    //! Get the first item in the sequence
	/**
	 *
	 * @return XdmItem or nullptr if sequence is empty
	 */
	virtual XdmItem * getHead();

	//! Get the n'th item in the value, counting from zero.
	/**
	   *
	   *
	   * @param n the item that is required, counting the first item in the sequence as item zero
	   * @return the n'th item in the sequence making up the value, counting from zero
	   * return nullptr  if n is less than zero or greater than or equal to the number
	   *                                    of items in the value
	   */
	virtual XdmItem * itemAt(int n);

	/**
	 * Get the number of items in the sequence
	 *
	 */
	virtual int size();


	//! Create a string representation of the value.
	/**
	 * The is the result of serializing
	 * the value using the adaptive serialization method.
	 * @return a string representation of the value
	 */
	virtual const char * toString();

    //! Get the number of references on this XdmValue
    /**
     * This method is used for internal memory management.
     */
	int getRefCount() {
		return refCount;
	}



    //! Increment reference count of this XdmValue - Memory management - Internal use only
    /**
     * this method is used for internal memory management.
     */
	virtual void incrementRefCount();

    //! Decrement reference count of this XdmValue - Memory management - Internal use only
    /**
     * this method is used for internal memory management.
     */
	virtual void decrementRefCount();

	/**
	  * Get Java XdmValue object.
	  * @return jobject - The Java object of the XdmValue in its JNI representation
	  */
	virtual  jobject getUnderlyingValue();

	/**
	* Get the type of the object
	*/
	virtual XDM_TYPE getType();

protected:

    //! initialize this XdmValue with default values
   inline void initialize() {
    jValues   = nullptr;
    refCount  = 0;
    valueType = nullptr;
    xdmSize   = 0;
    toStringValue ="";
  }

    char* valueType;  /*!< Cached. The type of the XdmValue */


	std::vector<XdmItem*> values; /*!< Cached. XdmItems in the XdmValue */
	int xdmSize; 	/*!< Cached. The count of items in the XdmValue */
	int refCount; /*!< The reference count of this XdmValue. If >1 this object should not be deleted */
private:
	std::string toStringValue;  /*!< Cached. String representation of the XdmValue, if available */
	jobjectArray jValues; /*!< If this XdmValue is a sequence we store values as a JNI array object  */
};

#endif /** SAXON_XDMVALUE_H **/
