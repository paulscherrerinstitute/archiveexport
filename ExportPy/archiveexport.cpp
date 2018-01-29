#include <Python.h>

// Base
#include <epicsVersion.h>
// Tools
#include <AutoPtr.h>
#include <BinaryTree.h>
#include <RegularExpression.h>
#include <epicsTimeHelper.h>
#include <ArgParser.h>
// Storage
#include <SpreadsheetReader.h>
#include <AutoIndex.h>

// List channel names, maybe with start/end info.
void list_channels(Index &index, stdVector<stdString> names, bool info)
{
    epicsTime start, end;
    stdString s, e;
    AutoPtr<RTree> tree;
    size_t i;
    for (i=0; i<names.size(); ++i)
    {
        if (info)
        {
            stdString directory;
            tree = index.getTree(names[i], directory);
            if (!tree)
                throw GenericException(__FILE__, __LINE__,
                                       "Cannot locate channel '%s'",
                                       names[i].c_str());
            tree->getInterval(start, end);
            printf("%s\t%s\t%s\n", names[i].c_str(),
                   epicsTimeTxt(start, s), epicsTimeTxt(end, e));
        }
        else
            printf("%s\n", names[i].c_str());
    }
}

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


static PyMethodDef ArchiveExportMethods[] = {
    {"list",   (PyCFunction)archiveexport_list, METH_VARARGS|METH_KEYWORDS, "Execute a shell command."},
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