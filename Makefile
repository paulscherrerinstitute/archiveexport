# EPICS R3.14 Makefile for the Channel Archiver

TOP=.
export TOP
include $(TOP)/configure/CONFIG

DIRS += Tools
DIRS += Storage
DIRS += ExportPy

include $(TOP)/configure/RULES_DIRS

