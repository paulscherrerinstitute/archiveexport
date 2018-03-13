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
#include <AutoIndex.h>
#include <ReaderFactory.h>
#include <RawDataReader.h>
#include <RawValue.h>

// Epics Base
#include <epicsVersion.h>


#include "utils.h"

// NC: Could use some test python script that shows how it works with some test index file

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

    // NC: You can avoid future traps by always using braces when doing if-else/while/for.
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|$s", kwlist, &index_name, &pattern ))
        return NULL;

    PyObject *list;

    list = PyList_New(0);
    // NC: list could be null, always check return values
    //     There is a lot of places where return values are not checked. I won't comment each of them.

    /* open index file */ 
    AutoIndex index;
    try{
        index.open(index_name);
    }catch (GenericException &e){
        // NC: Print the filename/index that it failed to open.
        //     The original author of ChannelArchiver _really_ failed to create an exception hierarchy.
        //     It is sad that you have to catch a "Generic" exception and that you have to guess
        //     the reason for it being thrown. Maybe you should print e.what() in case the reason wasn't
        //     that it failed to open the file.
        //     If you want to generate a more complex string to pass to python, look at std::ostringstream.
        PyErr_SetString(PyExc_FileNotFoundError, "Specified index file does not exist.");
        return NULL;
    }

    try
    {
        AutoPtr<RegularExpression> regex;        
        if (pattern && strlen(pattern)  > 0){
            regex.assign(new RegularExpression(pattern));
        }

        Index::NameIterator name_iter;
        if (!index.getFirstChannel(name_iter))
            return list; // No names //< NC: Maybe not so clear comment. Write that you are returning an empty list.
        do
        {
            if (regex && !regex->doesMatch(name_iter.getName()))
                continue; // skip what doesn't match the regex
            // otherwise append it to the list
            // NC: A problem with macros is that it is not clear here that archiveexport_list could return null here.
            PyList_AppendDECREF(list, PyUnicode_FromString(name_iter.getName().c_str()));
        }
        while (index.getNextChannel(name_iter));
        // NC: getFirstChannel and getNextChannel is a pretty insane interface you have to deal with...
    }
    catch (GenericException &e)
    {
        PyErr_SetString(PyExc_ValueError, e.what());
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
    epicsTime * start = NULL;
    epicsTime * end = NULL;
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
                                        // NC: Probably (void*) &start
                                        EpicsTime_FromPyDateTime_converter, &start, 
                                        EpicsTime_FromPyDateTime_converter, &end,
                                        &get_units,
                                        &get_status,
                                        &get_info                                        
                                     ) 
        )
        return NULL;

    n = PyList_Size(channel_names);

    // check channel names for type
    // NC: Declare i inside the for()-statement
    int i;
    for (i = 0; i < n; i++){
        channel_name = PyList_GetItem(channel_names, i);
        // NC: Check if channel_name is null
        if(!PyUnicode_Check(channel_name)){
            PyErr_SetString(PyExc_TypeError, "Channel names must be strings.");
            // `delete` will go away if you store epicsTime in this scope instead of on the heap
            delete start; delete end; 
            return NULL;
        }
    }

    /* open index file */ 
    AutoIndex index;
    try{
        index.open(index_name);
    }catch (GenericException &e){
        PyErr_SetString(PyExc_FileNotFoundError, "Specified index file does not exist.");
        return NULL;
    }

    // NC: Declare variables so that they are in the smallest scope possible.
    //     "value" could have been declared inside the loop
    const RawValue::Data *value;
    AutoPtr<DataReader> reader(ReaderFactory::create(index, ReaderFactory::Raw, 0.0));

    // top container dict
    PyObject *container_dict;
    container_dict = PyDict_New();
    
    // for each channel name
    // NC: Don't reuse i.
    for (i = 0; i < n; i++){
        channel_name = PyList_GetItem(channel_names, i);
        // NC: Check null.
        
        // create first channel list
        PyObject *value_list = PyList_New(0);
        // NC: Check null.

        // find first value
        value = reader->find(PyUnicode_AsUTF8(channel_name), start);

        while (value)
        {
            if (! RawValue::isInfo(value)){ // true here indicates a special record marking interruption in data recording
                // create a placeholder for the value
                PyObject *row_dict = PyDict_New();
                // value 
                PyDict_SetItemStringDECREF(row_dict, "value", PyObject_FromDBRType(value, reader->getType(), reader->getCount()));
                //timestamp
                epicsTime timestamp = RawValue::getTime(value);
                // sec 
                PyDict_SetItemStringDECREF(row_dict, "seconds", PyLong_FromLong(epicsTimeStamp(timestamp).secPastEpoch)); 
                // nsec 
                PyDict_SetItemStringDECREF(row_dict, "nanoseconds", PyLong_FromLong(epicsTimeStamp(timestamp).nsec));
                // units  - surrogateescape does not fail on undecodable characters
                if(get_units && reader->getInfo().getType()==CtrlInfo::Numeric){
                    PyDict_SetItemStringDECREF(row_dict, "unit", PyUnicode_DecodeLocale(reader->getInfo().getUnits(),"surrogateescape"));
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
    
                // append dict to the list 
                PyList_AppendDECREF(value_list, row_dict);

                // break one node after the end timestamp
                if (end && timestamp >= *end)
                    break;
            }
            // next value
            value = reader->next();
        }

        // add values list to the dictionary, dispose item only, since key is still used in channel_names
        PyDict_SetItemDECREFItem(container_dict, channel_name, value_list);

    }
    
    delete start; delete end;
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
    //PyDateTime_IMPORT;
    PyDateTimeAPI = (PyDateTime_CAPI *)PyCapsule_Import(PyDateTime_CAPSULE_NAME, 0);
    return PyModule_Create(&archiveexportmodule);
}
