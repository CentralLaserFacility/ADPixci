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

#include <epicsEvent.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <iocsh.h>
#include <epicsString.h>
#include <epicsExit.h>


#include "ADDriver.h"

#include <epicsExport.h>

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#include <windows.h>
#endif

extern "C" {
#include "xcliball.h"
}

#include "pixci.h"

#define FORMAT "default"
#define DRIVERPARMS "" //default , user '-QU 0' for not using interrupts



extern "C" int pixciConfig(const char *portName, int IDType, const char *IDValue,
                                 int maxBuffers, size_t maxMemory, int priority, int stackSize)
{
    new pixci(portName, IDType, IDValue, maxBuffers, maxMemory, priority, stackSize);
    return(asynSuccess);
}


pixci::pixci(const char *portName,  int IDType, const char *IDValue,
                         int maxBuffers, size_t maxMemory, int priority, int stackSize)

    : ADDriver(portName, 1, (int)NUM_PERKIN_ELMER_PARAMS, maxBuffers, maxMemory, 0, 0, ASYN_CANBLOCK, 1, priority, stackSize)
    {
        connectCamera();

    }



int pixci::connectCamera(void){
   return pxd_PIXCIopen(DRIVERPARMS, FORMAT,"");
    
}






/* Code for iocsh registration */
static const iocshArg pixciConfigArg0 = {"Port name", iocshArgString};
static const iocshArg pixciConfigArg1 = {"CameraId", iocshArgInt};
static const iocshArg pixciConfigArg2 = {"maxBuffers", iocshArgInt};
static const iocshArg pixciConfigArg3 = {"maxMemory", iocshArgInt};
static const iocshArg pixciConfigArg4 = {"priority", iocshArgInt};
static const iocshArg pixciConfigArg5 = {"stackSize", iocshArgInt};
static const iocshArg pixciConfigArg6 = {"maxFrames", iocshArgInt};
static const iocshArg * const pixciConfigArgs[] =  {&pixciConfigArg0,
                                                     &pixciConfigArg1,
                                                     &pixciConfigArg2,
                                                     &pixciConfigArg3,
                                                     &pixciConfigArg4,
                                                     &pixciConfigArg5,
                                                     &pixciConfigArg6};
static const iocshFuncDef configpixci = {"pixciConfig", 7, pixciConfigArgs};
static void configpixciCallFunc(const iocshArgBuf *args)
{
    pixciConfig(args[0].sval, args[1].ival, args[2].ival,  args[3].ival, 
                 args[4].ival, args[5].ival, args[6].ival);
}

static void pixciRegister(void)
{

    iocshRegister(&configpixci, configpixciCallFunc);
}

extern "C" {
epicsExportRegistrar(pixciRegister);
}

