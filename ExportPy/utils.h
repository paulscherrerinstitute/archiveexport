#ifndef _AE_UTILS_H_
#define _AE_UTILS_H_

// Python
#include <Python.h>

//Storage
#include "RawValue.h"

PyObject *
PyObject_FromDBRType(const void *p_dbr_value, DbrType type, DbrCount count);

epicsTime * EpicsTime_FromPyDateTime(PyDateTime_DateTime * pyTime);

int EpicsTime_FromPyDateTime_converter(PyDateTime_DateTime * O, epicsTime *& time);

/* add some error handling
 PyDict_SetItemString increases reference count for an object, so it needs to be decreased */
#ifdef AE_DEBUG
#define PyDict_SetItemStringDECREF(dict, str_key, item) \
    printf("Key: %s\n", str_key); \
    pObj = item; \
    if(! pObj){ \
        printf("OBJ was null\n"); \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemStringDECREF item is NULL"); \
        return NULL; \
    } \
    printf("before set item\n"); \
    if(PyDict_SetItemString(dict, str_key, pObj) == -1){ \
        printf("set item error\n"); \
        Py_DECREF(dict); \
        return NULL; \
    } \
    printf("after set item\n"); \
    Py_DECREF(pObj);
#else 
#define PyDict_SetItemStringDECREF(dict, str_key, item) \
    pObj = item; \
    if(! pObj){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemStringDECREF item is NULL"); \
        return NULL; \
    } \
    if(PyDict_SetItemString(dict, str_key, pObj) == -1){ \
        Py_DECREF(dict); \
        return NULL; \
    } \
    Py_DECREF(pObj); 
#endif

/* add some error handling
PyDict_SetItem  increases reference count for an object, so it needs to be decreased */
#define PyDict_SetItemDECREF(dict, key, item) \
    pObj = item; \
    if(! pObj){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemDECREF item is NULL"); \
        return NULL; \
    } \
    pKey = key; \
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
    Py_DECREF(pKey); 

/*PyDict_SetItem  increases reference count for an object, so it needs to be decreased */
#define PyDict_SetItemDECREFItem(dict, key, item) \
    pObj = item; \
    if(! pObj){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemDECREF item is NULL"); \
        return NULL; \
    } \
    pKey = key; \
    if(! pKey){ \
        Py_DECREF(dict); \
        PyErr_SetString(PyExc_RuntimeError, "PyDict_SetItemDECREF key is NULL"); \
        return NULL; \
    } \
    if(PyDict_SetItem(dict, pKey, pObj) == -1){ \
        Py_DECREF(dict); \
        return NULL; \
    } \
    Py_DECREF(pObj);

/* add some error handling
PyDict_SetItem  increases reference count for an object, so it needs to be decreased */
#ifdef AE_DEBUG
  #define PyList_AppendDECREF(list, item) \
    pObj = item; \
    if(! pObj){ \
        printf("OBJ was null\n"); \
        Py_DECREF(list); \
        PyErr_SetString(PyExc_RuntimeError, "PyList_AppendDECREF item is NULL"); \
        return NULL; \
    }  \
    printf("Before list append\n");\
    if(PyList_Append(list, pObj) == -1){ \
        printf("List append error\n");\
        Py_DECREF(list); \
        return NULL; \
    } \
    printf("After PyList_Append\n"); \
    Py_DECREF(pObj);  
#else
#define PyList_AppendDECREF(list, item) \
    pObj = item; \
    if(! pObj){ \
        Py_DECREF(list); \
        PyErr_SetString(PyExc_RuntimeError, "PyList_AppendDECREF item is NULL"); \
        return NULL; \
    }  \
    if(PyList_Append(list, pObj) == -1){ \
        Py_DECREF(list); \
        return NULL; \
    } \
    Py_DECREF(pObj);  
#endif

#endif