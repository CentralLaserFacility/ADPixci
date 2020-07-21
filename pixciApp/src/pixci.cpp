/**
 * @brief This is a driver for PIXCI frame grabber from epix, inc. Developed for Eagle XV CCD from Raptor photonics
 * 
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
 XCLW64 .dll and .lib files should be included for windows-64 OS
 XCLIBNT .dll and .lib files should be inlcuded for win32 OS
 xclib_x86_64 .so and .a files should be included for linux_x86_64 OS
 xclib_i386 .so and .a files should be included for linux_x86 OS
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

static void acquireTaskC(void *drvPvt);
HANDLE  hEvent;


/**
 * @brief Configuration command for pixci driver; creates a new pixci object.
 * @param See the pixci.h
 */
extern "C" int pixciConfig(const char *portName,
                                 int maxBuffers, size_t maxMemory, int priority, int stackSize)
{
    new Pixci(portName, maxBuffers, maxMemory, priority, stackSize);
    return(asynSuccess);
}

/**
 * @brief Default constructor to create a new Pixci::Pixci object
 */
Pixci::Pixci(const char *portName,  int maxBuffers, size_t maxMemory, int priority, int stackSize)
    : ADDriver(portName, 1, (int)1, maxBuffers, maxMemory, 0, 0, ASYN_CANBLOCK, 1, priority, stackSize)
    {
        /* TODO:  Driver-specific parameters for the driver will be defined here */
        pxd_PIXCIopen(DRIVERPARMS, FORMAT, SETUPFILE);
        
        DWORD   ThreadId;
        hEvent = pxd_eventCapturedFieldCreate(0x1);
        int status = asynSuccess;
        status = (epicsThreadCreate("acquireTask",
                              epicsThreadPriorityMedium,
                              epicsThreadGetStackSize(epicsThreadStackMedium),
                              (EPICSTHREADFUNC)acquireTaskC,
                              this) == NULL);



    }

    /** @brief From asynPortDriver: attempt to connect driver to device.
     * @return asynStatus asynSuccess if connected successfully else asynError
     *  */ 
    asynStatus Pixci::connect(asynUser* pasynUser){
        int connectionStatusCode = 0;
        static const char *functionName = "connectCamera";

        /* pxd_PIXCIopen(driverparms, formatname, formatfile) return 0 if connection is successfull
         * returns value <0 if any error occured
         * pxd_mesgErrorCode(int code) will return description of the error occured
         */
        connectionStatusCode = pxd_PIXCIopen(DRIVERPARMS, FORMAT, SETUPFILE);
        if(connectionStatusCode < 0){          
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "%s:%s: Cannot OPEN camera: %s.", 
                  driverName, functionName,  pxd_mesgErrorCode(connectionStatusCode));
            return asynError;
        }
        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
            "%s:%s Camera connected;",
            driverName, functionName);
        return asynSuccess;
        }
    }


    /** @brief From asynPortDriver: attempts to disconnect driver from device.
     *  @return asynStatus asynSuccess if disconnected successfully else asynError
     */ 
    asynStatus Pixci::disconnect(asynUser* pasynUser){
        int disconnectStatusCode = 0;
        static const char *functionName = "disconnectCamera";
        
        /*pxd_PIXCIclose() disconnect the driver from the device. 
         * return 0 if disconnect successfull, return integer <0 if error occured
         * pxd_mesgErrorCode(int code) will return description of the error occured
        */
        disconnectStatusCode = pxd_PIXCIclose();
        if(disconnectStatusCode < 0){
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "%s:%s: disconnect camera error: %s.", 
                  driverName, functionName,  pxd_mesgErrorCode(disconnectStatusCode));
        return asynError;

        }
        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
            "%s:%s camera disconnected;",
            driverName, functionName);
        return asynSuccess;
             
        }
    }

    void Pixci::acquireImage(){

        int err = 0;
	    err = pxd_goLive(1, 1L);
	    if (err < 0)
            printf("go live error");

    }

    static void acquireTaskC(void *drvPvt)
    {
        Pixci *pPvt = (Pixci *)drvPvt;
        pPvt->acquireTask();
    }

    

    void Pixci::acquireTask(){
        
        for (;;){
            WaitForSingleObject(hEvent, INFINITE);
            printf("image recieved \n");
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
