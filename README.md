## Description
archiveexport is a self contained (Anaconda) Python package to read "old" archiver files.

## Install

### Conda setup
Create an environment for Python 3.6, on your terminal window or an Anaconda promt run: 

```
conda create --name ArchiveExport python=3.6

```

and activate the new environment:


```
source activate ArchiveExport

```

### Requirements
The library relies on the following packages:

- python
- xerces-c
- epics-base

As conda is being used to install the packages, add the **paulscherrerinstitute** channel to the conda config:
```
conda config --add channels paulscherrerinstitute
```

### Local build
Build the package from the root folder of the project:
```
conda build conda-recipe
conda install --use-local archiveexport
```

## Test
A test example is provided and can be found in ExportPy directory.

SSHFS was used to mount remote archiver directory over a SSH connection:

```
sshfs < username >@gfalc:/net/slsmcarch-navf/export/archiver-data-mc/archive_ST /mnt/archiver/ -o "StrictHostKeyChecking=no" -o UserKnownHostsFile=/dev/null;
```

where _< username >_ should be replaced by PSI's username.

To get the data for ``ARIDI-VME-BPM12:LOAD`` PV one can use provided ``test.py`` script by exetucting the following command from the root folder of the project:
```
python ExportPy/test.py
```

### References
1. [SSHFS - filesystem client based on ssh](https://linux.die.net/man/1/sshfs)