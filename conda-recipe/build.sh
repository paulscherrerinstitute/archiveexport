#!/bin/bash

# Set env. variables
OS="`uname -s`"
export OS

# inconsistencies in epics build files
if [[ "$OS" == "Linux" ]]; then
    export AR="$AR -rc"
elif [[ "$OS" == "Darwin"* ]]; then
    export AR="$AR -rc"
fi

make EPICS_BASE="$PREFIX/epics" PYTHON_INCLUDE=$PREFIX/include/python$PY_VER CC=$CC CCC=$CC AR="$AR" RANLIB=$RANLIB

install -d $SP_DIR

# Copy to Python’s site-packages location

if [[ "$OS" == "Linux" ]]; then
    cp -av ExportPy/O.*/libarchiveexport.so $SP_DIR/archiveexport.so
elif [[ "$OS" == "Darwin"* ]]; then
    cp -av ExportPy/O.*/libarchiveexport.*.dylib $SP_DIR/archiveexport.so
    # The file in the SP_DIR need to be named .so as otherwise somehow Python is not 
    # picking it up.
fi
