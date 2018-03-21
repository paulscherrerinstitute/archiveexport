## Description

archiveexport is a Python module to read archiver files generated with ChanelArchiver. It is packaged to Conda package. 

## Usage

Module exposes two functions `archiveexport.list()` to extrat channel names and `archiveexport.get_data()` to extract the data. 

### Quick example

```python
import archiveexport as ae
import datetime

# find channels by pattern
index_file = "/mnt/archiver/index"
channels = ae.list(index_name=index_file, pattern="ARIDI.*BPM1")

# calculate start and date
now = datetime.datetime.now()
end = now-datetime.timedelta(days=1)
start = end-datetime.timedelta(minutes=1)

# query data
data = ae.get_data(index_name=index_file, channels=channels, start=start, end=end, get_units=True, get_status=True, get_info=True)
print(data)
```

### Reference

**`archiveexport.list`** ( *index_name, pattern=""* )
Searches the index file for channel names.

**Praramters:**                                                                                                
* `index_name` ... filepath of the index file as string.
* `pattern` *(optional)* ... regular expression to find channel names

**Returns:** A list of channel names.

**`archiveexport.get_data`**  ( *index_name, channels=[], start=..., end=... get_units=False, get_status=False, get_info=False* )
Queries archived data.

**Praramters:**                                                                                                
* `index_name` ... filepath of the index file as string.
* `channels`   ... a list of channel names eg. `["CHANNEL1", "CHANNEL2", ...]`
* `start` *(optional)* ... query data from this point in time. *(python datetime object)* 
* `end`   *(optional)* ... query data untill this point in time. *(python datetime object)* 
* `get_units`   *(optional)* ... return also units for numeric data. *(boolean)*
* `get_status`  *(optional)* ... return also status and severity information. *(boolean)* 
* `get_info`    *(optional)* ... return also limit information for numerical data or enum string for enums. *(boolean)* 

**Return value:**
Returns following structure:
```python
{
    "CHANNEL1": 
        [
            {"value":value ,"seconds":seconds, "nanoseconds":nanoseconds, "unit":"unit", ...}
            {"value":value ,"seconds":seconds, "nanoseconds":nanoseconds, ...}
            ...
        ],
    "CHANNEL2":[...],
    ...
}   
```

Returns a dictionary where every key is a channel name and value is a list of dictionaries that describe the stored data point. Keys and values available in the "value dictionary" are the following:

* `"value"` ... For scalar values, depending on the epics data type the following python type is returned:

| Epics DBR type  | Python type     | 
| --------------- | --------------- | 
| DBR_TIME_STRING | PyUnicodeObject |
| DBR_TIME_CHAR   | PyLongObject   |
| DBR_TIME_ENUM   | PyLongObject   |
| DBR_TIME_SHORT  | PyLongObject   |
| DBR_TIME_LONG   | PyLongObject   |
| DBR_TIME_FLOAT  | PyFloatObject   |
| DBR_TIME_DOUBLE | PyFloatObject   |

In case of an array a list of values is retured where the same conversion as above is applied, with one exception **DBR_TIME_CHAR** is converted to **PyByteArrayObject**.
    
* `"seconds"` ... number of seconds past since Epics epoch January 1, 1990 *(PyLongObject)*.
* `"nanoseconds"` ... number of nanoseconds past since the last full second *(PyLongObject)*.

Optional keys:

`get_units=True`:
* `"unit"` ... Engineering unit *(PyUnicodeObject)*. Included only if `get_units=True` and the value is of a numeric type (CHAR, SHORT, LONG, FLOAT or DOUBLE)

`get_status=True`:
* `"status"` ... Numeric representation of status *(PyLongObject)*.
* `"status_string"` ... String representation of status *(PyUnicodeObject)* or `None` if the string representation does not exist.
* `"severity"` ... Numeric representation of severity *(PyLongObject)*.
* `"severity_string"` ... String representation of severity *(PyUnicodeObject)* or `None` if the string representation does not exist.

`get_info=True` - If the value is numeric, limits and percision are added to the dictionary. Limits are stored as C float by Channel Archiver independent of the actual value type, which can lead to discrepancies between the actual limit value and the stored one.
* `"low_alarm"` ... *(PyFloatObject)* 
* `"low_warn"` ... *(PyFloatObject)* 
* `"high_warn"` ... *(PyFloatObject)* 
* `"high_alarm"` ... *(PyFloatObject)* 
* `"disp_low"` ... *(PyFloatObject)* 
* `"disp_high"` ... *(PyFloatObject)*
* `"precision"` ... *(PyLongObject)* 

`get_info=True` - If the value is an (Epics) Enumeration, enum string is added to the dictionary.
* `"enum_string"` ... *(PyUnicodeObject)* or `None` if the string representation does not exist.

## Installation

### Conda setup
Create and activate an environment for Python 3.6, on your terminal window or an Anaconda promt run: 

```
conda create --name ArchiveExport python=3.6
source activate ArchiveExport

```

### Requirements
The library relies on the following packages:

- python
- epics-base

As conda is being used to install the packages, add the **paulscherrerinstitute** channel and **conda-forge** to the conda config:
```
conda config --add channels paulscherrerinstitute
```

### Anaconda build
Build the package from the root folder of the project:
```
conda build conda-recipe
conda install --use-local archiveexport
```


## Test
A test example is provided and can be found in `example` directory.

SSHFS was used to mount remote archiver directory over a SSH connection:

```
sshfs < username >@gfalc:/net/slsmcarch-navf/export/archiver-data-mc/archive_ST /mnt/archiver/ -o "StrictHostKeyChecking=no" -o UserKnownHostsFile=/dev/null;
```

## System build and development
It is possible to build the module library using system libraries on linux. Cd to the `system-build` folder and correct the paths in the Makefile. Then run `make`, which will build the library and copy it to the `system-build` folder. File `test.py` can be used for testing.

To clean the build files run `make clean` from the `system-build` directory.

### Code structure
Sources in folders `Tools` storage `Storage` and `manual` are copied from the original [ChannelArchiver](http://www.sls.psi.ch/cgi-bin/cvsweb.cgi/G/EPICS/extensions/src/ChannelArchiver/) installation used at PSI. For more information about the ChannelArchiver itself please read the [manual](manual/manual.pdf). 

Sources in `PyExport` provide python bindings that are the core of this project.

### References
1. [SSHFS - filesystem client based on ssh](https://linux.die.net/man/1/sshfs)