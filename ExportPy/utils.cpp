/* #define AE_DEBUG */

/* C */
#include <time.h> 

/* Python*/
#include <Python.h>
#include <datetime.h>

/* Tools */
#include <ArrayTools.h>

/* Storage */
#include <RawValue.h>


#include "utils.h"

PyObject *
PyObject_FromDBRType(const void *p_dbr_value, DbrType type, DbrCount count){

    #ifdef AE_DEBUG
        printf("PyObject_FromDBRType\n");
    #endif
    
    PyObject * list;
    bool is_array = false;

    if(count > 1){
        /* it must be an array, generate a list of values */
        if(!(list = PyList_New(count))){
            return NULL;
        }
        is_array = true;
    }

    // NC: I would check if p_dbr_value is null before starting to use it.

    // NC: Each case seem to repeat itself. Maybe you could implement a function like this:
    //     template <typename T, typename U>
    //     PyObject* convert(int count, PyObject* list, PyObject* (fun)(T)) {
    //        const T *val = (const T *)((U *)p)->value;
    //        if(count > 1) {
    //            for (..){
    //                fun(val[i])
    //            }
    //            return list
    //        }
    //        return fun(*val)
    //     }
    //
    //     convert<dbr_enum_t, dbr_time_enum>(count, list, PyLong_FromLong);
    //     convert<dbr_short_t, dbr_time_short>(count, list, PyLong_FromLong);
    //
    //     (It wont work for DecodeLocale unless a wrapper function is made.
    //     Which maybe is a good idea since it always is called with
    //     "surrogateescape")

    switch (type)
    {
        case DBR_TIME_STRING:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_STRING\n");
            #endif
            const dbr_string_t * val = (const dbr_string_t *)((dbr_time_string *)p_dbr_value)->value;
            // NC: Could val be null? I would check it before using it
            if (is_array){
                for (int i = 0; i < count; ++i)
                {
                   // NC: This function returns -1 on failure. Should be checked.
                   if(PyList_SetItem(list, i, PyUnicode_DecodeLocale(*val,"surrogateescape")) == -1){
                        Py_DECREF(list);
                        return NULL;
                   }

                   ++val;
                }
                return list;
            }
            else 
                return PyUnicode_DecodeLocale(*val,"surrogateescape"); // does not fail on undecodable characters
        }
        case DBR_TIME_CHAR:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_CHAR\n");
            #endif
            dbr_char_t *val = &((dbr_time_char *)p_dbr_value)->value;
            if (is_array) 
                return PyByteArray_FromStringAndSize((char *) val, count);
            else 
                return PyLong_FromLong(*val);
        }

        case DBR_TIME_ENUM:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_ENUM\n");
            #endif
            dbr_enum_t *val = &((dbr_time_enum *)p_dbr_value)->value;
            if (is_array){
                for (int i = 0; i < count; ++i)
                {
                   if(PyList_SetItem(list, i, PyLong_FromLong(val[i])) == -1){
                        Py_DECREF(list);
                        return NULL;
                   }
                   ++val;
                }
                return list;
            }
            else 
                return PyLong_FromLong(*val);
        }

        case DBR_TIME_SHORT:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_SHORT\n");
            #endif
            dbr_short_t *val = &((dbr_time_short *)p_dbr_value)->value;
            if (is_array){
                for (int i = 0; i < count; ++i)
                {
                   if(PyList_SetItem(list, i, PyLong_FromLong(val[i])) == -1){
                        Py_DECREF(list);
                        return NULL;
                   }
                   ++val;
                }
                return list;
            }
            else 
                return PyLong_FromLong(*val);
        }
        
        case DBR_TIME_LONG:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_LONG\n");
            #endif   
            dbr_long_t *val = &((dbr_time_long *)p_dbr_value)->value;
            if (is_array){
                for (int i = 0; i < count; ++i)
                {
                   if(PyList_SetItem(list, i, PyLong_FromLong(val[i])) == -1){
                        Py_DECREF(list);
                        return NULL;
                   }
                   ++val;
                }
                return list;
            }
            else 
                return PyLong_FromLong(*val);
        }
        case DBR_TIME_FLOAT:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_FLOAT\n");
            #endif
            dbr_float_t *val = &((dbr_time_float *)p_dbr_value)->value;
            if (is_array){
                for (int i = 0; i < count; ++i)
                {
                   if(PyList_SetItem(list, i, PyFloat_FromDouble(val[i])) == -1){
                        Py_DECREF(list);
                        return NULL;
                   }
                   ++val;
                }
                return list;
            }
            else 
                return PyFloat_FromDouble(*val);
        }

        case DBR_TIME_DOUBLE:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_DOUBLE\n");
            #endif     
            dbr_double_t *val = &((dbr_time_double *)p_dbr_value)->value;
            if (is_array){
                for (int i = 0; i < count; ++i)
                {
                   PyList_SetItem(list, i, PyFloat_FromDouble(val[i]));
                   ++val;
                }
                return list;
            }
            else 
                return PyFloat_FromDouble(*val);
        }

        default:
            #ifdef AE_DEBUG
                printf("case default\n");
            #endif
            PyErr_SetString(PyExc_TypeError, "Unexpected DBR Type");
    }

    
    return NULL;
}

PyObject *
PyObyect_getStatusString(const RawValue::Data *value){

    if(value && (size_t) value->status < SIZEOF_ARRAY(epicsAlarmConditionStrings)) {
        return PyUnicode_DecodeLocale(epicsAlarmConditionStrings[value->status],"surrogateescape");
    }else{
        Py_RETURN_NONE;
    }
}

PyObject *
PyObyect_getSeverityString(const RawValue::Data *value){

    if(value && (size_t) value->status < SIZEOF_ARRAY(epicsAlarmSeverityStrings)) {
        return PyUnicode_DecodeLocale(epicsAlarmSeverityStrings[value->status],"surrogateescape");
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

            return PyUnicode_DecodeLocale(enum_string.c_str(),"surrogateescape");
        }
    } 

    Py_RETURN_NONE;
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
