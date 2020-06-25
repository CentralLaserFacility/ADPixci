TOP=../
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#INC += "C:/Program Files/EPIX/XCLIB/inc/xcliball.h"
 INC += cext.h
 INC += pximage.h
 INC += xcliball.h
 INC += cext_hps.h
 INC += cext_hpe.h
 INC += pxerrno.h
 INC += pxlibvs.h
 INC += xclibvs.h
 INC += pxlibsv.h
 INC += pixcimod.h
 INC += xclibsv.h
 INC += xclibver.h
 INC += xclibsc.h
 INC += pxlibcam.h

ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
LIB_INSTALLS_WIN32 += ../os/win32-x86/XCLIBW64.lib
BIN_INSTALLS_WIN32 += ../os/win32-x86/XCLIBW64.dll

else ifeq (windows-x64, $(findstring windows-x64, $(T_A)))

LIB_INSTALLS_WIN32 += ../os/win32-x86/XCLIBW64.lib
BIN_INSTALLS_WIN32 += ../os/win32-x86/XCLIBW64.dll

endif
#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

