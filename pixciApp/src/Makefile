TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE


ifeq (windows-x64, $(findstring windows-x64, $(T_A)))
USR_CPPFLAGS += -D__X64
endif


LIBRARY_IOC_WIN32 = pixci
LIB_SRCS_WIN32 += pixci.cpp
LIB_LIBS += XCLIBWNT

DBD += pixciSupport.dbd




# PROD_IOC_Linux += 
# PROD_IOC_Linux += 
# PROD_IOC_Linux += 
# PROD_IOC_WIN32 +=
# PROD_IOC_WIN32 += 


# Add locally compiled object code
# PROD_LIBS += 
# PROD_LIBS_WIN32 += 
# PROD_SYS_LIBS_Linux += 

# ifeq (windows-x64-debug, $(findstring windows-x64-debug, $(T_A)))
# USR_LDFLAGS_WIN32 += /NODEFAULTLIB:LIBCMT.lib
# endif


include $(ADCORE)/ADApp/commonLibraryMakefile

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

