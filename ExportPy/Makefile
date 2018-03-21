TOP:=$(TOP)/..
include $(TOP)/configure/CONFIG

LIBRARY = archiveexport
LIB_SRCS += archiveexport.cpp
LIB_SRCS += utils.cpp

# channel archiver
LIB_LIBS += Storage
LIB_LIBS += Tools

# epics base
LIB_SYS_LIBS += Com ca
USR_LDFLAGS += -L$(EPICS_BASE)/lib/$(EPICS_HOST_ARCH)

# python
USR_INCLUDES  += -I$(PYTHON_INCLUDE)m



include $(TOP)/configure/RULES
