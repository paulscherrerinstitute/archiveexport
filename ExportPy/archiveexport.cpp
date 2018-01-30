#include <Python.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
// #include "numpy/arrayobject.h"
// Base
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


// Visitor for BinaryTree of channel names;
// see get_names_for_pattern().
static void add_name2vector(const stdString &name, void *arg)
{
    stdVector<stdString> *names = (stdVector<stdString> *)arg;
    //if (verbose)
    //    printf("%s\n", name.c_str());
    names->push_back(name);
}

void get_names_for_pattern(Index &index,
                           stdVector<stdString> &names,
                           const stdString &pattern)
{
    //if (verbose)
    //    printf("Expanding pattern '%s'\n", pattern.c_str());
    try
    {
        AutoPtr<RegularExpression> regex;
        if (pattern.length() > 0)
            regex.assign(new RegularExpression(pattern.c_str()));
        Index::NameIterator name_iter;
        if (!index.getFirstChannel(name_iter))
            return; // No names
        // Put all names in binary tree
        BinaryTree<stdString> channels;
        do
        {
            if (regex && !regex->doesMatch(name_iter.getName()))
                continue; // skip what doesn't match regex
            channels.add(name_iter.getName());
        }
        while (index.getNextChannel(name_iter));
        // Sorted dump of names
        channels.traverse(add_name2vector, (void *)&names);
    }
    catch (GenericException &e)
    {
        throw GenericException(__FILE__, __LINE__,
                               "Error expanding name pattern '%s':\n%s\n",
                               pattern.c_str(), e.what());
    }
}


static PyObject *
archiveexport_list(PyObject *self, PyObject *args, PyObject *keywds)
{
   
    char *index_name;
    char *pattern;

    char *kwlist[] = {"index_name", "pattern", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|s", kwlist, &index_name, &pattern ))
        return NULL;

    stdVector<stdString> names;

    AutoIndex index;
    index.open(index_name);
    
    printf("Opened index: %s\n", index_name);
    printf("pattern: %s\n", pattern);

    get_names_for_pattern(index, names, pattern);

    PyObject *list;
    list = PyList_New(names.size());

    size_t i;
    for (i=0; i<names.size(); ++i)
    {
        PyList_SET_ITEM(list, i, PyUnicode_FromString(names[i].c_str()));
    }

    return list;
}


static PyObject *
archiveexport_get_data(PyObject *self, PyObject *args, PyObject *keywds)
{
   
    char *index_name;
    char *channel;

    char *kwlist[] = {"index_name", "channel", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|s", kwlist, &index_name, &channel ))
        return NULL;

    AutoIndex index;
    index.open(index_name);

    double delta = 0.0;
    AutoPtr<epicsTime> start, end;

    const RawValue::Data *value;
    AutoPtr<DataReader> reader(ReaderFactory::create(index, ReaderFactory::Raw, delta));

    stdVector<stdString> units;

    bool is_array = false;

    PyObject *list;
    list = PyList_New(0);

    // find first value
    value = reader->find(channel, start);

    if (value){
        // data found
        units.push_back(reader->getInfo().getUnits());
        // Check if it's an array; 
        if (reader->getCount() > 1)
        {
            is_array = true;
        }

    }

    while (value)
    {

        PyObject * row = PyList_New(0);

        epicsTime timestamp = epicsTimeStamp(RawValue::getTime(value));
        if (end && timestamp >= *end)
            break;

        if (RawValue::isInfo(value))
        {   // this indicates interruption in data recording
            1;
        }
        else{

            /* sec */
            PyList_Append(row, PyLong_FromLong(epicsTimeStamp(timestamp).secPastEpoch));
            /* nsec */
            PyList_Append(row, PyLong_FromLong(epicsTimeStamp(timestamp).nsec));

            if (reader->getCount() <= 1)
            {   

                double dbl;
                RawValue::getDouble(reader->getType(), reader->getCount(), value, dbl, 0);
                PyList_Append(row, PyFloat_FromDouble(dbl));
            }
            else
            {   // Array
            
            }
            PyList_Append(list, row);
        }
        value = reader->next();
    }
    
    return list;
}

/* Export to Python */

static PyMethodDef ArchiveExportMethods[] = {
    {"list",   (PyCFunction)archiveexport_list, METH_VARARGS|METH_KEYWORDS, "Find channels."},
    {"get_data",   (PyCFunction)archiveexport_get_data, METH_VARARGS|METH_KEYWORDS, "Get data."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyDoc_STRVAR(archiveexport_doc, "This is a template module just for instruction.");

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
    return PyModule_Create(&archiveexportmodule);
}