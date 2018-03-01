// #define AE_DEBUG

// Python
#include <Python.h>
#include <datetime.h>

// Epics Base
#include <epicsVersion.h>

// Tools
#include <AutoPtr.h>
#include <BinaryTree.h>
#include <RegularExpression.h>
#include <epicsTimeHelper.h>
#include <ArgParser.h>

// Storage
#include <AutoIndex.h>
#include <ReaderFactory.h>
#include <RawDataReader.h>
#include <RawValue.h>

#include "utils.h"

static PyObject *
archiveexport_list(PyObject *self, PyObject *args, PyObject *keywds)
{
   
    char *index_name = NULL;
    char *pattern = NULL;

    char *kwlist[] = {(char *)"index_name", (char *)"pattern", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|$s", kwlist, &index_name, &pattern ))
        return NULL;

    PyObject *list;

    list = PyList_New(0);

    AutoIndex index;
    index.open(index_name);

    // placeholders to ude with DECREF macros
    PyObject * pObj;

    try
    {
        AutoPtr<RegularExpression> regex;        
        if (pattern && strlen(pattern)  > 0){
            regex.assign(new RegularExpression(pattern));
        }
        

        Index::NameIterator name_iter;
        if (!index.getFirstChannel(name_iter))
            return list; // No names
        do
        {
            if (regex && !regex->doesMatch(name_iter.getName()))
                continue; // skip what doesn't match regex
            PyList_AppendDECREF(list, PyUnicode_FromString(name_iter.getName().c_str()));
        }
        while (index.getNextChannel(name_iter));
    }
    catch (GenericException &e)
    {
        Py_DECREF(list);
        throw GenericException(__FILE__, __LINE__,
                               "Error expanding name pattern '%s':\n%s\n",
                               pattern, e.what());
    }

    return list;
}


static PyObject *
archiveexport_get_data(PyObject *self, PyObject *args, PyObject *keywds)
{


    char *index_name = NULL;;
    PyObject *channel_names = NULL;
    PyObject *channel_name = NULL;
    epicsTime * start = NULL;
    epicsTime * end = NULL;

    Py_ssize_t n;

    char *kwlist[] = {(char *)"index_name", (char *)"channels", (char *)"start", (char *)"end", NULL};

    if  (!PyArg_ParseTupleAndKeywords(args, keywds, "s|$O!O&O&", kwlist, 
                                        &index_name, 
                                        &PyList_Type, &channel_names, 
                                        EpicsTime_FromPyDateTime_converter, &start, 
                                        EpicsTime_FromPyDateTime_converter, &end 
                                     ) 
        )
        return NULL;

    //printf("start:%"PRIu32"\n", epicsTimeStamp(*start).secPastEpoch);
    //printf("end:%"PRIu32"\n", epicsTimeStamp(*end).secPastEpoch);
    

    n = PyList_Size(channel_names);

    // check channel names for type
    int i;
    for (i = 0; i < n; i++){
        channel_name = PyList_GetItem(channel_names, i);
        if(!PyUnicode_Check(channel_name)){
            PyErr_SetString(PyExc_TypeError, "Channel names must be strings.");
            delete start; delete end; 
            return NULL;
        }
    }

    AutoIndex index;
    index.open(index_name);

    const RawValue::Data *value;
    AutoPtr<DataReader> reader(ReaderFactory::create(index, ReaderFactory::Raw, 0.0));

    // top container dict
    PyObject *container_dict;
    container_dict = PyDict_New();
    
    // placeholders to use with DECREF macros from utils.h
    PyObject * pObj;
    PyObject * pKey;

    // for each channel name
    for (i = 0; i < n; i++){
        channel_name = PyList_GetItem(channel_names, i);
        
        // create first channel list
        PyObject *value_list = PyList_New(0);

        // find first value
        value = reader->find(PyUnicode_AsUTF8(channel_name), start);
        epicsTime timestamp = RawValue::getTime(value);

        while (value)
        {
            if (! RawValue::isInfo(value)){ // value returning true here is a special record indicating interruption in data recording

                PyObject *row_dict = PyDict_New();
                // value 
                PyDict_SetItemStringDECREF(row_dict, "value", PyObject_FromDBRType(value, reader->getType(), reader->getCount()));
                // sec 
                PyDict_SetItemStringDECREF(row_dict, "seconds", PyLong_FromLong(epicsTimeStamp(timestamp).secPastEpoch)); 
                // nsec 
                PyDict_SetItemStringDECREF(row_dict, "nanoseconds", PyLong_FromLong(epicsTimeStamp(timestamp).nsec));
                // units  - surrogateescape does not fail on undecodable characters
                PyDict_SetItemStringDECREF(row_dict, "units", PyUnicode_DecodeLocale(reader->getInfo().getUnits(),"surrogateescape"));

                // append dict to list 
                PyList_AppendDECREF(value_list, row_dict);
            }

            // break after one node after the end timestamp
            
            if (end && timestamp >= *end)
                break;

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