/*
* pixci.cpp
* This is a driver for PIXCI frame grabber from epix, inc. 
* Developed for Eagle XV CCD from Raptor photonics
*
*
* Author: Subindev D
*         CLF, STFC UK
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For windows */
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#include <windows.h>
#endif

/* Pixci headers 
 source: http://www.epixinc.com/products/xclib.htm
 XCLW64 .dll and .lib files should be included for windows-64 os
 XCLIBNT .dll and .lib files should be inlcuded for win32 os
 xclib_x86_64 .so and .a files should be included for linux_x86_64
 xclib_i386 .so and .a files should be included for linux_x86 os
*/
extern "C"{
#include "xcliball.h"
} 
#include "pixci.h"

/* Epics headers */
#include <epicsEvent.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <iocsh.h>
#include <epicsString.h>
#include <epicsExit.h>
#include <epicsExport.h>

#define FORMAT "default" // Video format configuration name
#define DRIVERPARMS "" //default , user '-QU 0' for not using interrupts
#define SETUPFILE "" //Video format configuration file name


/**
 * @brief Configuration command for pixci driver; creates a new pixci object.
 * 
 */
extern "C" int pixciConfig(const char *portName,
                                 int maxBuffers, size_t maxMemory, int priority, int stackSize)
{
    new pixci(portName, maxBuffers, maxMemory, priority, stackSize);
    return(asynSuccess);
}


pixci::pixci(const char *portName,  
                         int maxBuffers, size_t maxMemory, int priority, int stackSize)

    : ADDriver(portName, 1, (int)1, maxBuffers, maxMemory, 0, 0, ASYN_CANBLOCK, 1, priority, stackSize)
    {
        /* TODO:  Driver-specific parameters for the driver will be defined here */

    }

    /** From asynPortDriver: Connects driver to device;
     */ 
    asynStatus pixci::connect(asynUser* pasynUser){
        return connectCamera();
    }

    /**
     * @brief connect to the frame grabber using 'pxd_PIXCIopen(driverparms, formatname, formatfile)' method.
     *  should be called before any other xclib library function is invoked
     * @return asynStatus 
     */
    asynStatus pixci::connectCamera(){
        int connectionStatusCode = 0;
        static const char *functionName = "connectCamera";

        connectionStatusCode = pxd_PIXCIopen(DRIVERPARMS, FORMAT, SETUPFILE);
        if(connectionStatusCode < 0){          
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "%s:%s: Cannot OPEN camera: %s.", 
                  driverName, functionName,  pxd_mesgErrorCode(connectionStatusCode));
            return asynError;
        }
        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACE_WARNING,
            "%s:%s Camera connected;",
            driverName, functionName);
        return asynSuccess;
        }

    }

    /** From asynPortDriver: disconnect driver from device;
     */ 
    asynStatus pixci::disconnect(asynUser* pasynUser){
        return disconnectCamera();
    }

    /**
     * @brief disconnect from frame grabber if already connected to the frame grabber
     * 
     * @return asynStatus 
     */
    asynStatus pixci::disconnectCamera(){
        int disconnectStatusCode = 0;
        static const char *functionName = "disconnectCamera";
        disconnectStatusCode = pxd_PIXCIclose();
        if(disconnectStatusCode < 0){
            asynPrint(this->pasynUserSelf, ASYN_TRACE_WARNING,
            "%s:%s camera disconnected;",
            driverName, functionName);
        return asynSuccess;
        }
        else{
             asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "%s:%s: disconnect camera error: %s.", 
                  driverName, functionName,  pxd_mesgErrorCode(disconnectStatusCode));
        return asynError;
        }

    }


/* Code for iocsh registration */

/* pixciConfig parameters from st.cmd */
static const iocshArg pixciConfigArg0 = {"Port name", iocshArgString};
static const iocshArg pixciConfigArg1 = {"maxBuffers", iocshArgInt};
static const iocshArg pixciConfigArg2 = {"maxMemory", iocshArgInt};
static const iocshArg pixciConfigArg3 = {"priority", iocshArgInt};
static const iocshArg pixciConfigArg4 = {"stackSize", iocshArgInt};
static const iocshArg * const pixciConfigArgs[] =  {&pixciConfigArg0,
                                                          &pixciConfigArg1,
                                                          &pixciConfigArg2,
                                                          &pixciConfigArg3,
                                                          &pixciConfigArg4,};
static const iocshFuncDef configpixci = {"pixciConfig", 5, pixciConfigArgs};
static void configpixciCallFunc(const iocshArgBuf *args)
{
  pixciConfig(args[0].sval, args[1].ival, args[2].ival, args[3].ival, 
                    args[4].ival);
}

static void pixciRegister(void)
{
  iocshRegister(&configpixci, configpixciCallFunc);
}

extern "C" {
epicsExportRegistrar(pixciRegister);
}
