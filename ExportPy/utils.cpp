#define AE_DEBUG

/* C */
#include <time.h> 

/* Python*/
#include <Python.h>
#include <datetime.h>

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
        list = PyList_New(count);
        is_array = true;
    }

    switch (type)
    {
        case DBR_TIME_STRING:{
            #ifdef AE_DEBUG
                printf("case DBR_TIME_STRING\n");
            #endif
            const dbr_string_t * val = (const dbr_string_t *)((dbr_time_string *)p_dbr_value)->value;
            if (is_array){
                for (int i = 0; i < count; ++i)
                {
                   PyList_SetItem(list, i, PyUnicode_FromString(val[i]));
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
                   PyList_SetItem(list, i, PyLong_FromLong(val[i]));
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
                   PyList_SetItem(list, i, PyLong_FromLong(val[i]));
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
                   PyList_SetItem(list, i, PyLong_FromLong(val[i]));
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
                   PyList_SetItem(list, i, PyFloat_FromDouble(val[i]));
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


epicsTime * EpicsTime_FromPyDateTime(PyDateTime_DateTime * pyTime){

    struct tm tm_time = {0};
    tm_time.tm_year = PyDateTime_GET_YEAR(pyTime) - 1900;
    tm_time.tm_mon = PyDateTime_GET_MONTH(pyTime) - 1;
    tm_time.tm_mday = PyDateTime_GET_DAY(pyTime);
    tm_time.tm_hour = PyDateTime_DATE_GET_HOUR(pyTime);
    tm_time.tm_min = PyDateTime_DATE_GET_MINUTE(pyTime);
    tm_time.tm_sec = PyDateTime_DATE_GET_SECOND(pyTime);

    struct local_tm_nano_sec tm_nano_sec = {0};
    tm_nano_sec.ansi_tm = tm_time;
    tm_nano_sec.nSec = PyDateTime_DATE_GET_MICROSECOND(pyTime) * 1000;

    return new epicsTime(tm_nano_sec);
}   

int EpicsTime_FromPyDateTime_converter(PyDateTime_DateTime * O, epicsTime *&  epics_time){

    if(!PyDateTimeAPI) PyDateTime_IMPORT;

    if (!PyDateTime_Check(O)){
        PyErr_SetString(PyExc_TypeError, "parameters specifying time should be of DateTime Type");
        return false;
    }

    epics_time = EpicsTime_FromPyDateTime(O);

    return true;
}
