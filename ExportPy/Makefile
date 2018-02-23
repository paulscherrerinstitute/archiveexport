TOP:=$(TOP)/..
include $(TOP)/configure/CONFIG

LIB_SRCS += archiveexport.cpp
LIBRARY_HOST = archiveexport

USR_CXXFLAGS  = -I$(PYTHON_INCLUDE)m -I$(PYTHON) -I$(INSTALL_LOCATION)/Storage -I$(INSTALL_LOCATION)/Tools -I$(EPICS_BASE)/include -I$(EPICS_BASE)/include/os/$(OS_CLASS)

OP_SYS_LDLIBS = $(INSTALL_LOCATION)/Storage/O.$(EPICS_HOST_ARCH)/libStorage.a $(INSTALL_LOCATION)/Tools/O.$(EPICS_HOST_ARCH)/libTools.a -Wl,-rpath $(EPICS_BASE)/lib/$(EPICS_HOST_ARCH) -L$(EPICS_BASE)/lib/$(EPICS_HOST_ARCH) -lCom -lca -L$(XERCES_LIB) -lxerces-c


include $(TOP)/configure/RULES
