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
    Takes PyDateTime as an argument and returns epicsTime.
*/
epicsTime EpicsTime_FromPyDateTime(PyDateTime_DateTime * py_datetime);

/*
    this is a converter function used by PyArg_ParseTupleAndKeywords. It creates
    new epics time object and assigns it to time.
*/
int EpicsTime_FromPyDateTimeConverter(PyDateTime_DateTime * py_datetime, void * epics_time);

// NC: Usually it is much better to implement macros as functions.
//     You can communicate failures with exceptions instead. If you want to
//     make it easy you throw std::runtime_error. It takes a string as input
//     that you can print with e.what(). You can then catch both
//     GenericException& and std::runtime_error& after each other and
//     differentiate. You can even catch std::exception& if you want to catch
//     both types in one catch() clause.
//
//     Anyway if you want to keep the macros you need to wrap them in `do { }
//     while(0)` for safety.

/* 
    PyDict_SetItemString increases reference count for the item, so it needs to be decreased,
    if the item is used only in the dict and nowhere else. 
    See https://docs.python.org/3.6/c-api/intro.html for more info. 
    Calls return NULL on error.
*/
#define PyDict_SetItemStringDECREF(dict, str_key, item) \
    do {PyObject * pObj = item; \
    if(! pObj){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemStringDECREF item is NULL"); \
        return NULL; \
    } \
    if(PyDict_SetItemString(dict, str_key, pObj) == -1){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemStringDECREF PyDict_SetItemString is not successfull"); \
        return NULL; \
    } \
    Py_DECREF(pObj);} while (0)

/* 
    This macro decreases refcount for item and for key.
    PyDict_SetItem  increases reference counts for key and item an object. They need to be decreased,
    if they are used only within the dict and no where else separately.
    See https://docs.python.org/3.6/c-api/intro.html for more info.
    Calls return NULL on error.
*/
#define PyDict_SetItemDECREF(dict, key, item) \
    do {PyObject * pObj = item; \
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
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemDECREF PyDict_SetItem not successfull"); \
        return NULL; \
    } \
    Py_DECREF(pObj); \
    Py_DECREF(pKey);} while (0)

/* 
    This macro decreases refcount for item only, in case key is stil used somewhere else.
    PyDict_SetItem  increases reference counts for key and item an object. They need to be decreased,
    if they are used only within the dict and no where else separately.
    See https://docs.python.org/3.6/c-api/intro.html for more info.
    Calls return NULL on error.
*/
#define PyDict_SetItemDECREFItem(dict, key, item) \
    do {PyObject * pObj = item; \
    if(! pObj){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemDECREFItem item is NULL"); \
        return NULL; \
    } \
    PyObject * pKey = key; \
    if(! pKey){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemDECREFItem key is NULL"); \
        return NULL; \
    } \
    if(PyDict_SetItem(dict, pKey, pObj) == -1){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemDECREFItem PyDict_SetItem not successfull"); \
        return NULL; \
    } \
    Py_DECREF(pObj);} while (0)

/* 
    PyList_Append  increases reference counts for the. It needs to be decreased,
    if it is used only within the list and no where else separately.
    See https://docs.python.org/3.6/c-api/intro.html for more info.
    Calls return NULL on error.
*/
#define PyList_AppendDECREF(list, item) \
    do {PyObject * pObj = item; \
    if(! pObj){ \
        Py_DECREF(list); \
        PyErr_SetString(PyExc_RuntimeError, "PyList_AppendDECREF item is NULL"); \
        return NULL; \
    }  \
    if(PyList_Append(list, pObj) == -1){ \
        Py_DECREF(list); \
        PyErr_SetString(PyExc_RuntimeError, "PyList_AppendDECREF PyList_Append not successfull"); \
        return NULL; \
    } \
    Py_DECREF(pObj);} while (0)

#endif
