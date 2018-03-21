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
    This is a converter function used by PyArg_ParseTupleAndKeywords. It creates
    new epics time object and assigns it to time.
*/
int EpicsTime_FromPyDateTimeConverter(PyDateTime_DateTime * py_datetime, void * epics_time);

/* 
    PyDict_SetItemString increases reference count for the item, so it needs to be decreased,
    if the item is used only in the dict and nowhere else. 
    See https://docs.python.org/3.6/c-api/intro.html for more info. 
    Throws std:runtime_error on failure.
*/
void PyDict_SetItemStringDECREF(PyObject* dict, const char* str_key, PyObject* item);

/* 
    This function decreases refcount for item and for key.
    PyDict_SetItem  increases reference counts for key and item an object. They need to be decreased,
    if they are used only within the dict and no where else separately.
    See https://docs.python.org/3.6/c-api/intro.html for more info.
    Throws std:runtime_error on failure.
*/
void PyDict_SetItemDECREF(PyObject* dict, PyObject* key, PyObject* item);

/* 
    This macro decreases refcount for item only, in case key is stil used somewhere else.
    PyDict_SetItem  increases reference counts for key and item an object. They need to be decreased,
    if they are used only within the dict and no where else separately.
    See https://docs.python.org/3.6/c-api/intro.html for more info.
    Throws std:runtime_error on failure.
*/
void PyDict_SetItemDECREFItem(PyObject* dict, PyObject* key, PyObject* item);

/* 
    PyList_Append  increases reference counts for the item. It needs to be decreased,
    if it is used only within the list and no where else separately.
    See https://docs.python.org/3.6/c-api/intro.html for more info.
    Throws std:runtime_error on failure.
*/
void PyList_AppendDECREF(PyObject * list, PyObject * item);

/*
    convert char * to PyUnicode using PyUnicode_DecodeLocale(*str,"surrogateescape")
    Surrogateescape does not fail on undecodable characters
*/
PyObject *
PyUnicode_Surrogateescape(const char* string);

#endif
