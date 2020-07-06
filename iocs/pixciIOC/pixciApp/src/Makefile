# TOP=../..

# include $(TOP)/configure/CONFIG
# #----------------------------------------
# #  ADD MACRO DEFINITIONS AFTER THIS LINE
# #=============================

# #=============================
# # Build the IOC application

# PROD_IOC = pixci
# PROD_NAME = pixci
# # pixci.dbd will be created and installed
# DBD += $(PROD_NAME).dbd

# # pixci.dbd will be made up from these files:
# #pixci_DBD += base.dbd
# $(PROD_NAME)_DBD += pixciSupport.dbd
# # Include dbd files from all support applications:
# #pixci_DBD += xxx.dbd

# # Add all the support libraries needed by this IOC
# #pixci_LIBS += xxx
# $(PROD_NAME)_LIBS += pixci
# $(PROD_NAME)_LIBS += XCLIBWNT

# # pixci_registerRecordDeviceDriver.cpp derives from pixci.dbd
# pixci_SRCS += pixci_registerRecordDeviceDriver.cpp

# # Build the main IOC entry point on workstation OSs.
# pixci_SRCS_DEFAULT += pixciMain.cpp
# pixci_SRCS_vxWorks += -nil-

# # Add support from base/src/vxWorks if needed
# #pixci_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# # Finally link to the EPICS Base libraries
# pixci_LIBS += $(EPICS_BASE_IOC_LIBS)

# include $(ADCORE)/ADApp/commonDriverMakefile
# #===========================

# include $(TOP)/configure/RULES
# #----------------------------------------
# #  ADD RULES AFTER THIS LINE

TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

PROD_NAME = pixci
PROD_IOC_WIN32 += $(PROD_NAME)

# <name>.dbd will be created from <name>Include.dbd
DBD += $(PROD_NAME).dbd
$(PROD_NAME)_DBD += pixciSupport.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
$(PROD_NAME)_SRCS += $(PROD_NAME)_registerRecordDeviceDriver.cpp $(PROD_NAME)Main.cpp

# Add locally compiled object code
$(PROD_NAME)_LIBS += pixci
$(PROD_NAME)_LIBS += XCLIBW64


include $(ADCORE)/ADApp/commonDriverMakefile

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

