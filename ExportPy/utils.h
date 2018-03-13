#ifndef _AE_UTILS_H_
#define _AE_UTILS_H_

// Python
#include <Python.h>

//Storage
#include "RawValue.h"
#include "CtrlInfo.h"

// Epics alarmStrings.h has problems with being included multiple times
extern const char* epicsAlarmConditionStrings[4];
extern const char* epicsAlarmSeverityStrings[22];

/*
    This function returns a suitable PyObject depending on the epics DBR Type.
    If the value is scalar, than the conversion is the following:
        DBR_TIME_STRING ... PyUnicodeObject
        DBR_TIME_CHAR   ... PyLongObject
        DBR_TIME_ENUM   ... PyLongObject
        DBR_TIME_SHORT  ... PyLongObject
        DBR_TIME_LONG   ... PyLongObject
        DBR_TIME_FLOAT  ... PyFloatObject
        DBR_TIME_DOUBLE ... PyFloatObject
    If the value is an array an instance of PyListObject is returned, with the elements
    of the type that matches conversion above. An exception is an array of DBR_TIME_CHAR.
        DBR_TIME_CHAR[]  ... PyByteArrayObject
    If type does not match one of the DBR types listed above the function returns NULL.
*/
PyObject *
PyObject_FromDBRType(const void *p_dbr_value, DbrType type, DbrCount count);

/*
    Returns PyUnicodeObject if the status string is found else PyNone
*/
PyObject *
PyObyect_getStatusString(const RawValue::Data *value);

/*
    Returns PyUnicodeObject if the severity string is found else PyNone
*/
PyObject *
PyObyect_getSeverityString(const RawValue::Data *value);

/*
    Returns PyUnicodeObject if the enum string is found else PyNone
*/
PyObject *
PyObyect_getEnumString(const RawValue::Data *value, const CtrlInfo info);

/*
    Takes PyDateTime as an argument and returns new epicsTime object.
*/
epicsTime * EpicsTime_FromPyDateTime(PyDateTime_DateTime * pyTime);

/*
    this is a converter function used by PyArg_ParseTupleAndKeywords. It creates
    new epics time object and assigns it to time.
*/
int EpicsTime_FromPyDateTime_converter(PyDateTime_DateTime * O, epicsTime *& time);

/* 
    PyDict_SetItemString increases reference count for an object, so it needs to be decreased. 
*/
#define PyDict_SetItemStringDECREF(dict, str_key, item) \
    {PyObject * pObj = item; \
    if(! pObj){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemStringDECREF item is NULL"); \
        return NULL; \
    } \
    if(PyDict_SetItemString(dict, str_key, pObj) == -1){ \
        Py_DECREF(dict); \
        return NULL; \
    } \
    Py_DECREF(pObj);} 

/* 
    PyDict_SetItem  increases reference count for an object, so it needs to be decreased.
    This macro decreases refcount for item and for key.
*/
#define PyDict_SetItemDECREF(dict, key, item) \
    {PyObject * pObj = item; \
    if(! pObj){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemDECREF item is NULL"); \
        return NULL; \
    } \
    PyObject * pKey = key; \
    if(! pKey){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemDECREF key is NULL"); \
        return NULL; \
    } \
    if(PyDict_SetItem(dict, pKey, pObj) == -1){ \
        Py_DECREF(dict); \
        return NULL; \
    } \
    Py_DECREF(pObj); \
    Py_DECREF(pKey);}

/* 
    PyDict_SetItem increases reference count for an object, so it needs to be decreased.  
    This macro decreases refcount for item only, in case key is stil used somewhere else.
*/
#define PyDict_SetItemDECREFItem(dict, key, item) \
    {PyObject * pObj = item; \
    if(! pObj){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemDECREF item is NULL"); \
        return NULL; \
    } \
    PyObject * pKey = key; \
    if(! pKey){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemDECREF key is NULL"); \
        return NULL; \
    } \
    if(PyDict_SetItem(dict, pKey, pObj) == -1){ \
        Py_DECREF(dict); \
        return NULL; \
    } \
    Py_DECREF(pObj);}

/* 
    PyDict_SetItem increases reference count for an object, so it needs to be decreased.
*/
#define PyList_AppendDECREF(list, item) \
    {PyObject * pObj = item; \
    if(! pObj){ \
        Py_DECREF(list); \
        PyErr_SetString(PyExc_RuntimeError, "PyList_AppendDECREF item is NULL"); \
        return NULL; \
    }  \
    if(PyList_Append(list, pObj) == -1){ \
        Py_DECREF(list); \
        return NULL; \
    } \
    Py_DECREF(pObj);}

#endif