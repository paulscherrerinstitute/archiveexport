EPICS_BASE=/opt/epics/base 
XERCES_LIB=/usr/local/lib
PYTHON_INCLUDE=/opt/anaconda3/include/python3.6


build:
	EPICS_BASE=$(EPICS_BASE) PYTHON_INCLUDE=$(PYTHON_INCLUDE) XERCES_LIB=$(XERCES_LIB) USR_LDFLAGS="-L$(EPICS_BASE) -L$(XERCES_LIB)" make -C ../
	cp -av ../ExportPy/O.*/libarchiveexport.so ./archiveexport.so

clean: 
	EPICS_BASE=$(EPICS_BASE) PYTHON_INCLUDE=$(PYTHON_INCLUDE) XERCES_LIB=$(XERCES_LIB) make -C ../ clean
	rm -rf ../lib
	rm -rf ../include
	rm archiveexport.so