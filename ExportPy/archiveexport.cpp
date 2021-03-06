// #define AE_DEBUG

// Python
#include <Python.h>
#include <datetime.h>

// Tools
#include <AutoPtr.h>
#include <ArgParser.h>
#include <BinaryTree.h>
#include <RegularExpression.h>
#include <epicsTimeHelper.h>


// Storage
#include <IndexFile.h>
#include <ReaderFactory.h>
#include <RawDataReader.h>
#include <RawValue.h>

// Epics Base
#include <epicsVersion.h>


#include "utils.h"

/*
    Callable from python: archiverexport.list()
    Arguments:
        index_name            ... path to the index file
        pattern (optional)    ... regex pattern for channel names
    
    Returns PyList of channel names.
*/
static PyObject *
archiveexport_list(PyObject *self, PyObject *args, PyObject *keywds)
{
   
    char *index_name = NULL;
    char *pattern = NULL;

    char *kwlist[] = {(char *)"index_name", (char *)"pattern", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|$s", kwlist, &index_name, &pattern )){
        return NULL;
    }

    PyObject *list;

    if(!(list = PyList_New(0))) {
        PyErr_SetString(PyExc_RuntimeError, "List could not be created.");
        return NULL;
    }
    
    /* open index file in readonly mode */ 
    IndexFile index;
    try{
        index.open(index_name, true);
    }catch (GenericException &e){
        // guessing that file was not found
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }

    try
    {
        AutoPtr<RegularExpression> regex;        
        if (pattern && strlen(pattern)  > 0) {
            regex.assign(new RegularExpression(pattern));
        }

        Index::NameIterator name_iter;
        if (!index.getFirstChannel(name_iter)) {
            // no names found, return an empty list.
            return list;
        }
        do
        {
            if (regex && !regex->doesMatch(name_iter.getName()))
                continue; // skip what doesn't match the regex
            // otherwise append it to the list
            PyList_AppendDECREF(list, PyUnicode_FromString(name_iter.getName().c_str()));
        }
        while (index.getNextChannel(name_iter));
        // NC: getFirstChannel and getNextChannel is a pretty insane interface you have to deal with...
    }
    catch (GenericException &e)
    {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }

    return list;
}
/*
    Callable from python: archiverexport.get_data()
    Arguments:
        index_name            ... path to the index file
        channels              ... list of channel names
        start (optional)      ... start time (python datetime)
        stop                  ... end time (python datetime)
        get_units             ... get information about engineering units
        get_status            ... get information about status and severity  
        get_info              ... get high low, alarm, warning and display limits or enum string

    Returns Dict of Lists of dicts:
        {
            "channel_name1": 
                [
                    {"value":value ,"seconds":seconds, "nanoseconds":nanoseconds, ...}
                    {"value":value ,"seconds":seconds, "nanoseconds":nanoseconds, ...}
                    ...
                ],
            "channel_name2":[...],
            ...
        }
    If possible it allways returns one data point before start and one after stop and 
    everything in between.
*/
static PyObject *
archiveexport_get_data(PyObject *self, PyObject *args, PyObject *keywds)
{

    char *index_name = NULL;
    PyObject *channel_names = NULL;
    PyObject *channel_name = NULL;
    // NC: Store start and end in this scope instead of on the heap.
    epicsTime start;
    epicsTime end;
    int get_units  = false;
    int get_status = false;
    int get_info   = false;
    
    Py_ssize_t n;

    char *kwlist[] = {  (char *)"index_name", 
                        (char *)"channels", 
                        (char *)"start", 
                        (char *)"end",
                        (char *)"get_units", 
                        (char *)"get_status",
                        (char *)"get_info",
                        NULL
                    };

    if  (!PyArg_ParseTupleAndKeywords(args, keywds, "s|$O!O&O&ppp", kwlist, 
                                        &index_name, 
                                        &PyList_Type, &channel_names,
                                        EpicsTime_FromPyDateTimeConverter, (void*) &start, 
                                        EpicsTime_FromPyDateTimeConverter, (void*) &end,
                                        &get_units,
                                        &get_status,
                                        &get_info                                        
                                     ) 
        )
    {
        return NULL;
    }
        
    n = PyList_Size(channel_names);

    // check channel names for type
    for (int i = 0; i < n; i++){
        if(!(channel_name = PyList_GetItem(channel_names, i))){
            return NULL; // PyExc is set by PyList_GetItem
        }
        if(!PyUnicode_Check(channel_name)){
            PyErr_SetString(PyExc_TypeError, "Channel names must be strings.");
            return NULL;
        }
    }

    /* open index file in readonly mode */ 
    IndexFile index;
    try{
        index.open(index_name, true);
    }catch (GenericException &e){
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
    
    AutoPtr<DataReader> reader(ReaderFactory::create(index, ReaderFactory::Raw, 0.0));

    // top container dict
    PyObject *container_dict;
    if(!(container_dict = PyDict_New())){
        PyErr_SetString(PyExc_RuntimeError, "Dict could not be created.");
        return NULL;
    }
    
    try{
        // for each channel name
        for (int i = 0; i < n; i++){
            if(!(channel_name = PyList_GetItem(channel_names, i))){
                return NULL; // PyExc is set by PyList_GetItem
            }
            
            // create first channel list
            PyObject *value_list; 
            if(!(value_list = PyList_New(0))) {
                PyErr_SetString(PyExc_RuntimeError, "List could not be created.");
                return NULL;
            }

            // find first value
            const RawValue::Data *value;
            try{    
                value = reader->find(PyUnicode_AsUTF8(channel_name), &start);
            }catch (GenericException &e){
                PyErr_SetString(PyExc_RuntimeError, e.what());
                return NULL;
            }
            while (value)
            {
                if (! RawValue::isInfo(value)){ // true here indicates a special record marking interruption in data recording
                    // create a placeholder for the value
                    PyObject *row_dict;

                    if(!(row_dict = PyDict_New())){
                        PyErr_SetString(PyExc_RuntimeError, "row_dict could not be created.");
                        return NULL;
                    }

                    //timestamp
                    epicsTime timestamp = RawValue::getTime(value);

                    try{
                        // value 
                        PyDict_SetItemStringDECREF(row_dict, "value", PyObject_FromDBRType(value, reader->getType(), reader->getCount()));
                        // sec 
                        PyDict_SetItemStringDECREF(row_dict, "seconds", PyLong_FromLong(epicsTimeStamp(timestamp).secPastEpoch)); 
                        // nsec 
                        PyDict_SetItemStringDECREF(row_dict, "nanoseconds", PyLong_FromLong(epicsTimeStamp(timestamp).nsec));
                        // units  - surrogateescape does not fail on undecodable characters
                        if(get_units && reader->getInfo().getType()==CtrlInfo::Numeric){
                            PyDict_SetItemStringDECREF(row_dict, "unit", PyUnicode_Surrogateescape(reader->getInfo().getUnits()));
                        }
                        // status & severity
                        if(get_status){
                            PyDict_SetItemStringDECREF(row_dict, "status", PyLong_FromLong(value->status));
                            PyDict_SetItemStringDECREF(row_dict, "status_string", PyObyect_getStatusString(value));
                            PyDict_SetItemStringDECREF(row_dict, "severity", PyLong_FromLong(value->severity));
                            PyDict_SetItemStringDECREF(row_dict, "severity_string", PyObyect_getSeverityString(value));
                        }
                        // info
                        if(get_info){
                            if(reader->getInfo().getType()==CtrlInfo::Numeric){
                                // all limit values are achived as floats
                                PyDict_SetItemStringDECREF(row_dict, "low_alarm", PyFloat_FromDouble(reader->getInfo().getLowAlarm()));
                                PyDict_SetItemStringDECREF(row_dict, "low_warn", PyFloat_FromDouble(reader->getInfo().getLowWarning()));
                                PyDict_SetItemStringDECREF(row_dict, "high_warn", PyFloat_FromDouble(reader->getInfo().getHighAlarm()));
                                PyDict_SetItemStringDECREF(row_dict, "high_alarm", PyFloat_FromDouble(reader->getInfo().getHighWarning()));
                                PyDict_SetItemStringDECREF(row_dict, "disp_low", PyFloat_FromDouble(reader->getInfo().getDisplayLow()));
                                PyDict_SetItemStringDECREF(row_dict, "disp_high", PyFloat_FromDouble(reader->getInfo().getDisplayHigh()));
                                PyDict_SetItemStringDECREF(row_dict, "precision", PyLong_FromLong(reader->getInfo().getPrecision()));
                            }
                            if(reader->getType()==DBR_TIME_ENUM) {
                                PyDict_SetItemStringDECREF(row_dict, "enum_string", PyObyect_getEnumString(value, reader->getInfo()));
                            }
                        }
                    }
                    catch(std::exception &e){
                        Py_DECREF(row_dict);
                        throw e;
                    }
                    // append dict to the list 
                    PyList_AppendDECREF(value_list, row_dict);


                    // break one node after the end timestamp if end was set (is greater than 0) 
                    if (end > epicsTime() && timestamp >= end)
                        break;
                }
                // next value
                value = reader->next();
            }

            // add values list to the dictionary, dispose item only, since key is still used in channel_names
            PyDict_SetItemDECREFItem(container_dict, channel_name, value_list);

        }
    }catch(std::exception &e){
        PyErr_SetString(PyExc_RuntimeError, e.what());
        Py_DECREF(container_dict);
        return NULL;
    }

    return container_dict;
}

/* Export to Python */

static PyMethodDef ArchiveExportMethods[] = {
    {"list",   (PyCFunction)archiveexport_list, METH_VARARGS|METH_KEYWORDS, "Find channels."},
    {"get_data",   (PyCFunction)archiveexport_get_data, METH_VARARGS|METH_KEYWORDS, "Get data."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyDoc_STRVAR(archiveexport_doc, "This module can extract data from ChannelArchiver files.");

static struct PyModuleDef archiveexportmodule = {
    PyModuleDef_HEAD_INIT,
    "archiveexport",   /* name of module */
    archiveexport_doc, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    ArchiveExportMethods
};


PyMODINIT_FUNC
PyInit_archiveexport(void)
{
    if(!PyDateTimeAPI) PyDateTime_IMPORT;
    return PyModule_Create(&archiveexportmodule);
}
