/**
 * @brief This is a driver for PIXCI frame grabber from epix, inc. Developed for Eagle XV CCD from Raptor photonics
 * 
 */

#include <stdio.h>
#include <stdlib.h>

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

#define FORMAT "default" // Video format configuration name.
#define DRIVERPARMS "" // Default , user '-QU 0' for not using interrupts.
#define SETUPFILE "" // Video format configuration file name.
#define UNIT 1 // Unit to be selected for streaming, eb1 model only have 1 unit.
#define NOERROR 0 // Errors are defined as integers below zero.

/*
 * @brief C Function prototypes to tie in with EPICS
 * run acquire task 
 * @param drvPvt 
 */
static void acquireTaskC(void *drvPvt);
/* Event handler for acquire task */
HANDLE  g_hEvent; 

/*
 * @brief Configuration command for pixci driver; creates a new pixci object.
 * @param See the pixci.h
 */
extern "C" int pixciConfig(const char *portName,
                                 int maxBuffers, size_t maxMemory, int priority, int stackSize)
{
    new Pixci(portName, maxBuffers, maxMemory, priority, stackSize);
    return(asynSuccess);
}

/*
 * @brief Default constructor to create a new Pixci::Pixci object
 */
Pixci::Pixci(const char *portName,  int maxBuffers, size_t maxMemory, int priority, int stackSize)
    : ADDriver(portName, 1, (int)1, maxBuffers, maxMemory, 0, 0, ASYN_CANBLOCK, 1, priority, stackSize)
    {
        /* TODO:  Driver-specific parameters for the driver will be defined here */


        int connectionStatusCode = 0;

        /* pxd_PIXCIopen(driverparms, formatname, formatfile) return 0 if connection is successfull
         * returns value <0 if any error occured
         * pxd_mesgErrorCode(int code) will return description of the error occured
         */
        connectionStatusCode = pxd_PIXCIopen(DRIVERPARMS, FORMAT, SETUPFILE);
        if(connectionStatusCode < NOERROR){          
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "%s: Cannot OPEN camera: %s.", 
                  driverName,  pxd_mesgErrorCode(connectionStatusCode));
        }
        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
            "%s Camera connected;",
            driverName);
        }

        /* Any thread waiting upon the event will be notified whenever a field has been captured by pxd_goSnap, 
        pxd_goLive, pxd_goLivePair and pxd_goLiveSeq*/
        g_hEvent = pxd_eventCapturedFieldCreate(UNIT);
        int status = asynSuccess;
        /* Create the thread that does data acquisition */
        status = (epicsThreadCreate("acquireTask",
                              epicsThreadPriorityMedium,
                              epicsThreadGetStackSize(epicsThreadStackMedium),
                              (EPICSTHREADFUNC)acquireTaskC,
                              this) == NULL);

    }

Pixci::~Pixci(){

    /* Closing connection to frame grabber */
    int disconnectStatusCode = NOERROR;
    /*pxd_PIXCIclose() disconnect the driver from the device. 
     * return 0 if disconnect successfull, return integer <0 if error occured
     * pxd_mesgErrorCode(int code) will return description of the error occured
    */
    disconnectStatusCode = pxd_PIXCIclose();
    if(disconnectStatusCode < NOERROR){
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "%s: disconnect camera error: %s .", 
                  driverName, pxd_mesgErrorCode(disconnectStatusCode));
    }
    else{
        asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
            "%s: camera disconnected;",
            driverName);
    }

}


    void Pixci::acquireImage(){
        /* TODO: implement all acquisition method like trigger, ringbuffer etc */
        static const char *functionName = "acquireImage";
        int error;
        pxbuffer_t buffer = 1L;         // Image frame buffer
        /* live capture the image into frame buffer */
        error = pxd_goLive(UNIT, buffer);
        if(error < NOERROR){
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "live error: %s : %s", functionName, pxd_mesgErrorCode(error));
        }
        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, 
                  "live started");
        }
    }

    void Pixci::acquireStop(){
        static const char *functionName = "acquireStop";
        int error;
        /* stop the live capturing */
        error = pxd_goUnLive(UNIT);
        if(error < NOERROR){
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
                  "live couldn't stop: %s : %s",functionName, pxd_mesgErrorCode(error));
        }
        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, 
                  "live stopped \n");
        }

    }

    static void acquireTaskC(void *drvPvt)
    {
        Pixci *pPvt = (Pixci *)drvPvt;
        pPvt->acquireTask();
    }

    /**
     * @brief Acquistion task for live image capturing.
     * Event will be notified whenever a field has been captured by pxd_goSnapor, pxd_goLive.
     */
    void Pixci::acquireTask(){
        /* TODO: need to implement in a seperate file */
        NDArray *pImage;
        pxbuffer_t  buf = 1L;
        pImage = this->pArrays[0];
        NDDataType_t  dataType;
        epicsInt32 sizeX, sizeY;
        size_t        dims[2];
        epicsTimeStamp currentTime;
        epicsInt32 numImagesCounter;
        epicsInt32 imageCounter;
        
        for (;;){
            /* waiting for event to be triggered */
            /* TODO: seperate waiting task for linux */
            WaitForSingleObject(g_hEvent, INFINITE);
            lock();

            /* Allocate NDArray */
            dims[0] = pxd_imageXdim();
            dims[1] = pxd_imageYdim();
            dataType = NDUInt16;
            pImage = this->pNDArrayPool->alloc(2, dims, dataType, 0, NULL);
            /* Pixel values from an image frame buffer and area of interest are copied into buffer 
            pxd_readushort(unit, framebuf, ulxc, ulyc, lrx, lry, membuf, cnt, colorspace)*/
            pxd_readushort(UNIT, buf, 0, 0, -1, -1, (epicsUInt16*)pImage->pData, dims[0] * dims[1] * sizeof(epicsUInt16), "GRAY");
            
             /* uniqueId and timeStamp must be implemented for standard ADDriver. */
            pImage->uniqueId = imageCounter;
            epicsTimeGetCurrent(&currentTime);
            pImage->timeStamp = currentTime.secPastEpoch + currentTime.nsec / 1.e9;
            updateTimeStamp(&pImage->epicsTS);

            getIntegerParam(NDArrayCounter, &imageCounter);
            getIntegerParam(ADNumImagesCounter, &numImagesCounter);
            imageCounter++;
            numImagesCounter++;
            setIntegerParam(NDArrayCounter, imageCounter);
            setIntegerParam(ADNumImagesCounter, numImagesCounter);
            unlock();

            /*Call doCallbacksGenericPointer() so that registered clients can get the values of the new arrays. 
            Drivers must release their mutex by calling this->unlock() before they call doCallbacksGenericPointer(),
             or a deadlock can occur if the plugin makes a call to one of the driver functions.*/
            doCallbacksGenericPointer(pImage, NDArrayData, 0);
            if (this->pArrays[0]) this->pArrays[0]->release();
            this->pArrays[0] = pImage;
            callParamCallbacks();
        }
    }

    asynStatus Pixci::writeInt32(asynUser *pasynUser, epicsInt32 value){
        int function = pasynUser->reason;
        int status = asynSuccess;
        static const char *functionName = "writeInt32";

        /* Set the parameter and readback in the parameter library.  This may be 
        overwritten when we read back the status at the end, but that's OK */
        status = setIntegerParam(function, value);

        if (function == ADAcquire) {
            /* TODO: adstatus == ADStatusIdle has to be checked */
            if (value ) 
            {
                acquireImage();
            }

            // Stop acquisition
            /* TODO: adstatus != ADStatusIdle has to be checked */
            if (!value)
            {
                acquireStop();
            }
        
        }   /* set  value for default parameters */
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
