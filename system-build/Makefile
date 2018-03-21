EPICS_BASE=/opt/_epics/base
# XERCES_LIB=/usr/local/lib
PYTHON_INCLUDE=/opt/anaconda3/include/python3.6


build:
	make -C ../ EPICS_BASE=$(EPICS_BASE) PYTHON_INCLUDE=$(PYTHON_INCLUDE)
	cp -av ../ExportPy/O.*/libarchiveexport.so ./archiveexport.so

clean: 
	BASE=$(BASE) PYTHON_INCLUDE=$(PYTHON_INCLUDE) make -C ../ clean
	rm -rf ../lib
	rm -rf ../include
	rm -rf archiveexport.so