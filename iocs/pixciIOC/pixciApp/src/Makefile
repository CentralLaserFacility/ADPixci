TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#=============================
# Build the IOC application
PROD_NAME = pixci

# <name>.dbd will be created from <name>Include.dbd
DBD += $(PROD_NAME).dbd

$(PROD_NAME)_DBD += base.dbd
# Include dbd files from all support applications:
$(PROD_NAME)_DBD += pixciSupport.dbd
$(PROD_NAME)_DBD += NDFileHDF5.dbd

# Add PIXCI PATH for static build dependency
BIN_INSTALLS += $(ADPIXCI)/bin/$(EPICS_HOST_ARCH)/*

# Add all the support libraries needed by this IOC
$(PROD_NAME)_LIBS += NDPlugin ADBase
$(PROD_NAME)_LIBS += pixci
ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
$(PROD_NAME)_LIBS += XCLIBWNT
else ifeq (windows-x64, $(findstring windows-x64, $(T_A)))
$(PROD_NAME)_LIBS += XCLIBW64
endif

# install ADPixci lib files
LIB_INSTALLS += $(ADPIXCI)/lib/$(EPICS_HOST_ARCH)/*

# install fmt video format files
FMT += $(patsubst $(ADPIXCI)/fmt/%.fmt, %.fmt, $(wildcard $(ADPIXCI)/fmt/*.fmt))

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
$(PROD_NAME)_SRCS += $(PROD_NAME)_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
$(PROD_NAME)_SRCS_DEFAULT += $(PROD_NAME)Main.cpp
$(PROD_NAME)_SRCS_vxWorks += -nil-

# Finally link to the EPICS Base libraries
$(PROD_NAME)_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================

include $(ADCORE)/ADApp/commonDriverMakefile
include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
