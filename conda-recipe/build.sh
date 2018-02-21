#!/bin/bash

# Set env. variables
OS="`uname -s`"
if [[ "$OS" == "Linux" ]]; then
   OS='Linux'
elif [[ "$OS" == "darwin"* ]]; then
   OS='$(EPICS_HOST_ARCH)'
fi

export OS

EPICS_BASE=$PREFIX/epics XERCES_LIB=$PREFIX/lib PYTHON=$PYTHON PYTHON_INCLUDE=$PREFIX/include/python$PY_VER make

install -d $SP_DIR

# Copy to Python’s site-packages location
if [[ "$OS" == "Linux" ]]; then
   cp -av ExportPy/O.*/libarchiveexport.so $SP_DIR/archiveexport.so
elif [[ "$OS" == "darwin"* ]]; then
   cp -av ExportPy/O.*/libarchiveexport*.dylib $SP_DIR/archiveexport.so
fi
