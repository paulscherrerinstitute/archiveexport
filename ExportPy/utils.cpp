/* #define AE_DEBUG */

/* C, C++ */
#include <time.h> 
#include <stdexcept>

/* Python*/
#include <Python.h>
#include <datetime.h>

/* Tools */
#include <ArrayTools.h>

/* Storage */
#include <RawValue.h>


#include "utils.h"

/* 
    T - value type
    U - dbr value type
    cast - c type
    p - pointer to the dbr_value
    PyFun - python convert function
*/
template <typename T, typename U, typename cast>
PyObject* dbr2pyobj(const void * p, int count, PyObject* (PyFun)(cast)) {
    if(!p){
        return NULL;
    }
    T *val = &((U *)p)->value;

    if (! val){
        return NULL;
    }

    if(count > 1) {
        // create a list
        PyObject * list;
        if(!(list = PyList_New(count))){
            return NULL;
        }
        // append all values to the list by converting the to PyObjects using PyFun
        for (int i = 0; i < count; ++i){
            if(PyList_SetItem(list, i, PyFun((cast) val[i])) == -1){
                Py_DECREF(list);
                return NULL;
            }
        }
        return list;
   }
   return PyFun((cast) *val);
}

PyObject *
PyObject_FromDBRType(const void *p_dbr_value, DbrType type, DbrCount count){

    #ifdef AE_DEBUG
        printf("PyObject_FromDBRType\n");
    #endif

    if(!p_dbr_value){
        return NULL;
    }

    switch (type)
    {

        case DBR_TIME_STRING:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_STRING\n");
            #endif
            const dbr_string_t * val = (const dbr_string_t *)((dbr_time_string *)p_dbr_value)->value;
            if (! val){
                return NULL;
            }
            if (count > 1){
                PyObject * list;
                if(!(list = PyList_New(count))){
                    return NULL;
                }
                for (int i = 0; i < count; ++i)
                {
                   if(PyList_SetItem(list, i, PyUnicode_Surrogateescape(*val)) == -1){
                        Py_DECREF(list);
                        return NULL;
                   }
                   ++val;
                }
                return list;
            }
            else 
                return PyUnicode_Surrogateescape(*val); // does not fail on undecodable characters
        }

        case DBR_TIME_CHAR:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_CHAR\n");
            #endif
            dbr_char_t *val = &((dbr_time_char *)p_dbr_value)->value;
            if (count > 1) 
                return PyByteArray_FromStringAndSize((char *) val, count);
            else 
                return PyLong_FromLong(*val);
        }

        case DBR_TIME_ENUM:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_ENUM\n");
            #endif

            return dbr2pyobj<dbr_enum_t, dbr_time_enum, long int>(p_dbr_value, count, PyLong_FromLong);
        }

        case DBR_TIME_SHORT:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_SHORT\n");
            #endif

            return dbr2pyobj<dbr_short_t, dbr_time_short, long int>(p_dbr_value, count, PyLong_FromLong);
        }
        
        case DBR_TIME_LONG:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_LONG\n");
            #endif   

            return dbr2pyobj<dbr_long_t, dbr_time_long, long int>(p_dbr_value, count, PyLong_FromLong);

        }
        case DBR_TIME_FLOAT:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_FLOAT\n");
            #endif

            return dbr2pyobj<dbr_float_t, dbr_time_float, double>(p_dbr_value, count, PyFloat_FromDouble);
        }

        case DBR_TIME_DOUBLE:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_DOUBLE\n");
            #endif     

            return dbr2pyobj<dbr_double_t, dbr_time_double, double>(p_dbr_value, count, PyFloat_FromDouble);
        }

        default:
            #ifdef AE_DEBUG
                printf("case default\n");
            #endif
            PyErr_SetString(PyExc_TypeError, "Unexpected DBR Type");
    }

    
    return NULL;
}


void PyDict_SetItemStringDECREF(PyObject * dict, const char * str_key, PyObject * item){
    if(! item){ 
        throw std::runtime_error("PyDict_SetItemStringDECREF item is NULL"); 
    } 
    if(PyDict_SetItemString(dict, str_key, item) == -1){
        Py_DECREF(item);
        throw std::runtime_error("PyDict_SetItemStringDECREF PyDict_SetItemString is not successfull");
    }
    Py_DECREF(item);
}


void PyDict_SetItemDECREFItem(PyObject * dict, PyObject * key, PyObject * item) {
    if(! item){ 
        throw std::runtime_error("PyDict_SetItemDECREFItem item is NULL"); 
    }
    if(! key){ 
        throw std::runtime_error("PyDict_SetItemDECREFItem key is NULL"); 
    } 
    if(PyDict_SetItem(dict, key, item) == -1){
        Py_DECREF(item);
        throw std::runtime_error("PyDict_SetItemDECREFItem PyDict_SetItemString is not successfull");
    } 
    Py_DECREF(item);
}


void PyDict_SetItemDECREF(PyObject * dict, PyObject * key, PyObject * item){
    try{
        PyDict_SetItemDECREFItem(dict, key, item);
    }catch(std::exception &e) {
        Py_DECREF(key);
        throw e;
    }
    Py_DECREF(key);
}

void PyList_AppendDECREF(PyObject * list, PyObject * item){
    if(! item){ 
        throw std::runtime_error("PyList_AppendDECREF item is NULL"); 
    }  
    if(PyList_Append(list, item) == -1){
        Py_DECREF(item);
        throw std::runtime_error("PyList_AppendDECREF PyList_Append not successfull");
    }  
    Py_DECREF(item);
}


PyObject *
PyObyect_getStatusString(const RawValue::Data *value){

    if(value && (size_t) value->status < SIZEOF_ARRAY(epicsAlarmConditionStrings)) {
        return PyUnicode_Surrogateescape(epicsAlarmConditionStrings[value->status]);
    }else{
        Py_RETURN_NONE;
    }
}

PyObject *
PyObyect_getSeverityString(const RawValue::Data *value){

    if(value && (size_t) value->status < SIZEOF_ARRAY(epicsAlarmSeverityStrings)) {
        return PyUnicode_Surrogateescape(epicsAlarmSeverityStrings[value->status]);
    }else{
        Py_RETURN_NONE;
    }

}
PyObject *
PyObyect_getEnumString(const RawValue::Data *value, const CtrlInfo info){

    if (value){
        int enum_idx = ((dbr_time_enum *)value)->value;

        if ((size_t) enum_idx < info.getNumStates()){
            stdString enum_string;
            info.getState(enum_idx, enum_string);

            return PyUnicode_Surrogateescape(enum_string.c_str());
        }
    } 

    Py_RETURN_NONE;
}


PyObject * PyUnicode_Surrogateescape(const char* string){
    return PyUnicode_DecodeLocale(string,"surrogateescape");
}

epicsTime EpicsTime_FromPyDateTime(PyDateTime_DateTime * py_datetime){

    /* import datetime API */
    if(!PyDateTimeAPI) PyDateTime_IMPORT;

    struct tm tm_time = {0};
    tm_time.tm_year = PyDateTime_GET_YEAR(py_datetime) - 1900;
    tm_time.tm_mon = PyDateTime_GET_MONTH(py_datetime) - 1;
    tm_time.tm_mday = PyDateTime_GET_DAY(py_datetime);
    tm_time.tm_hour = PyDateTime_DATE_GET_HOUR(py_datetime);
    tm_time.tm_min = PyDateTime_DATE_GET_MINUTE(py_datetime);
    tm_time.tm_sec = PyDateTime_DATE_GET_SECOND(py_datetime);

    struct local_tm_nano_sec tm_nano_sec = {0};
    tm_nano_sec.ansi_tm = tm_time;
    tm_nano_sec.nSec = PyDateTime_DATE_GET_MICROSECOND(py_datetime) * 1000;

    return epicsTime(tm_nano_sec);
}   

int EpicsTime_FromPyDateTimeConverter(PyDateTime_DateTime * py_datetime, void * epics_time){

    /* import datetime API */
    if(!PyDateTimeAPI) PyDateTime_IMPORT;

    if (!PyDateTime_Check(py_datetime)){
        PyErr_SetString(PyExc_TypeError, "parameters specifying time should be of DateTime Type");
        return 0;
    }

    * (epicsTime* ) epics_time = EpicsTime_FromPyDateTime(py_datetime);

    return 1;
}
