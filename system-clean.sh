# Set env. variables
OS="`uname -s`"
export OS

EPICS_BASE=/opt/epics/base XERCES_LIB=$PREFIX/lib PYTHON_INCLUDE=/opt/anaconda3/include/python3.6 make clean

rm ExportPy/archiveexport.so
rm -rf lib