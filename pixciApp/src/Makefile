TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

ifeq (windows-x64, $(findstring windows-x64, $(T_A)))
USR_CPPFLAGS += -D__X64
endif

LIBRARY_IOC_WIN32 = pixci
LIB_SRCS_WIN32 += pixci.cpp

ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
LIB_LIBS += XCLIBWNT
else ifeq (windows-x64, $(findstring windows-x64, $(T_A)))
LIB_LIBS += XCLIBW64
endif

DBD += pixciSupport.dbd

# Install .fmt files
FMT += $(patsubst ../fmt/%/bin_1x1.fmt, %.fmt, $(wildcard ../fmt/*/bin_1x1.fmt))

include $(ADCORE)/ADApp/commonLibraryMakefile

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
