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

    : ADDriver(portName, 1, (int)1, maxBuffers, maxMemory, 0, 0, ASYN_CANBLOCK, 1, priority, stackSize)
    {
        // connectCamera();

    }






    asynStatus pixci::connect(asynUser* pasynUser){
        return connectCamera();
    }

    asynStatus pixci::connectCamera(){
        int status = asynSuccess;
        int connectionStatusCode = 0;
        static const char *functionName = "connectCamera";

        connectionStatusCode = pxd_PIXCIopen(DRIVERPARMS, FORMAT,"");
        if(connectionStatusCode < 0){          
        //pxd_mesgFault(1);   // display more information about the open error

        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "%s:%s: Cannot OPEN camera: %s.", 
                  driverName, functionName,  pxd_mesgErrorCode(connectionStatusCode));
            return asynError;
        }
        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
            "%s:%s: Camera connected;", 
            driverName, functionName);
        return asynSuccess;
        }

    }








/* Code for iocsh registration */

/* pixciConfig */
static const iocshArg pixciConfigArg0 = {"Port name", iocshArgString};
static const iocshArg pixciConfigArg1 = {"ID type", iocshArgInt};
static const iocshArg pixciConfigArg2 = {"ID value", iocshArgString};
static const iocshArg pixciConfigArg3 = {"maxBuffers", iocshArgInt};
static const iocshArg pixciConfigArg4 = {"maxMemory", iocshArgInt};
static const iocshArg pixciConfigArg5 = {"priority", iocshArgInt};
static const iocshArg pixciConfigArg6 = {"stackSize", iocshArgInt};
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
  pixciConfig(args[0].sval, args[1].ival, args[2].sval, args[3].ival, 
                    args[4].ival, args[5].ival, args[6].ival);
}


static void pixciRegister(void)
{
  iocshRegister(&configpixci, configpixciCallFunc);
}

extern "C" {
epicsExportRegistrar(pixciRegister);
}
