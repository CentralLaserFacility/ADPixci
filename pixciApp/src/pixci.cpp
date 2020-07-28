/**
 * @brief This is a driver for PIXCI frame grabber from epix, inc. Developed for Eagle XV CCD from Raptor photonics
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>       // std::cout
#include <exception> 

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
                  "%s:%s: disconnect camera error: %s .", 
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
        int err;
        err = pxd_goLive(1, 1L);
        if(err < 0){
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "live error \n");
        }
        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "live started \n");
        }
    }

    void Pixci::acquireStop(){
        int err;
        err = pxd_goUnLive(1);
        if(err < 0){
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "live couldn't stop \n");
        }
        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "live stopped \n");
        }

    }

    static void acquireTaskC(void *drvPvt)
    {
        Pixci *pPvt = (Pixci *)drvPvt;
        pPvt->acquireTask();
    }

    /**
     * @brief Event will be notified whenever a field has beencaptured by pxd_goSnapor pxd_goLive
     * 
     */
    void Pixci::acquireTask(){
        int xrr = 0;
        NDArray *pImage;
        NDArrayInfo   arrayInfo;
        ushort   *buffer;
        pxbuffer_t  buf = 1;
        pImage = this->pArrays[0];
        NDDataType_t  dataType;
        epicsInt32 sizeX, sizeY;
        size_t        dims[2];
        pxvbtime_t fieldCount;
        epicsTimeStamp currentTime;
        epicsUInt16   *pInput;
        //void *ptest;
        
        for (;;){
            WaitForSingleObject(hEvent, INFINITE);
            lock();
            setIntegerParam(ADStatus, ADStatusIdle);
            setIntegerParam(ADAcquire, 0);
            buffer = (ushort*)malloc(sizeof(ushort)*3*pxd_imageXdim());
            pInput = (epicsUInt16*)malloc(sizeof(epicsUInt16)*3*pxd_imageXdim());
            printf("buffer is %u \n",buffer);
            // xrr = pxd_readushort(1, buf, 0, pxd_imageYdim()/2, -1, 1+pxd_imageYdim()/2, buffer, 3*pxd_imageXdim(), "GRAY");
            pImage = this->pArrays[0];
            sizeX = pxd_imageXdim();
            sizeY = pxd_imageYdim();
            dataType = NDUInt16;

            dims[0] = sizeX;
            dims[1] = sizeY;
            this->pArrays[0] = pNDArrayPool->alloc(2, dims, dataType, 0, NULL);

            if (this->pArrays[0])
                this->pArrays[0]->release();

            if (this->pArrays[0] == NULL) {
                printf("null array \n");
            }
            fieldCount = pxd_capturedFieldCount(1);
            pImage = this->pArrays[0];
            pImage->uniqueId = fieldCount;
            pImage->getInfo(&arrayInfo);
            // pInput = pImage->pData;
            xrr = pxd_readushort(1, buf, 0, pxd_imageYdim()/2, -1, 1+pxd_imageYdim()/2, pInput, 3*pxd_imageXdim(), "GRAY");
            //memcpy(pImage->pData, pInput, arrayInfo.totalBytes);
           
            epicsTimeGetCurrent(&currentTime);
            pImage->timeStamp = currentTime.secPastEpoch + currentTime.nsec / 1.e9;
            updateTimeStamp(&pImage->epicsTS);
            //printf("field count %d\n",fieldCount);            
        }
    }

    asynStatus Pixci::writeInt32(asynUser *pasynUser, epicsInt32 value){
        int function = pasynUser->reason;
        int status = asynSuccess;
        int adstatus;
        static const char *functionName = "writeInt32";

        /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
        * status at the end, but that's OK */
        status = setIntegerParam(function, value);

        if (function == ADAcquire) {
            if (value && (adstatus == ADStatusIdle) )
            {
                acquireImage();
            }

            // Stop acquisition
            if (!value && (adstatus != ADStatusIdle))
            {
                acquireStop();
            }
        
        }
        else{   
            status = ADDriver::writeInt32(pasynUser, value);
        }
        return (asynStatus) status;

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
