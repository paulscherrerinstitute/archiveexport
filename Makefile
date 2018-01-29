# EPICS R3.14 Makefile for the Channel Archiver

TOP=.
export TOP
include $(TOP)/configure/CONFIG

DIRS += Tools
# DIRS += LibIO
DIRS += Storage
# DIRS += IndexTool
# DIRS += DataTool
DIRS += Export
# DIRS += XMLRPCServer
# DIRS += Engine
# DIRS += Manager
# DIRS += ArchiveDaemon
#DIRS += IndexDump

DIRS += ExportPy

include $(TOP)/configure/RULES_DIRS

