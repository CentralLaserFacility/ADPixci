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
#include <epicsMessageQueue.h>

#define FORMAT "" // Video format configuration name.
#define DRIVERPARMS "" // Default , user '-QU 0' for not using interrupts.
#define UNIT 1 // Unit to be selected for streaming, eb1 model only have 1 unit.
#define NOERROR 0 // Errors are defined as integers below zero.
#define RESERVED 0
#define BAUDRATE 115200


/*
 * @brief C Function prototypes to tie in with EPICS
 * run acquire task
 * @param drvPvt
 */
static void acquireTaskC(void *drvPvt);

static void serialTaskC(void *drvPvt);

/* Event handler for acquire task */
HANDLE  g_hEvent;
unsigned char g_ucSerialBuf[256];
/*
 * @brief Configuration command for pixci driver; creates a new pixci object.
 * @param See the pixci.h
 */
extern "C" int pixciConfig(const char *portName,
                                 int maxBuffers, size_t maxMemory, int priority, int stackSize, const char *formatfile)
{
    new Pixci(portName, maxBuffers, maxMemory, priority, stackSize, formatfile);
    return(asynSuccess);
}

/*
 * @brief Default constructor to create a new Pixci::Pixci object
 */
Pixci::Pixci(const char *portName,  int maxBuffers, size_t maxMemory, int priority, int stackSize, const char *formatfile)
    : ADDriver(portName, 1, (int)1, maxBuffers, maxMemory, 0, 0, ASYN_CANBLOCK, 1, priority, stackSize)
    {
        /* TODO:  Driver-specific parameters for the driver will be defined here */


        int connectionStatusCode = 0;
        int serialConnection = 0;
        /* pxd_PIXCIopen(driverparms, formatname, formatfile) return 0 if connection is successfull
         * returns value <0 if any error occured
         * pxd_mesgErrorCode(int code) will return description of the error occured
         */
        connectionStatusCode = pxd_PIXCIopen(DRIVERPARMS, FORMAT, formatfile);

        if(connectionStatusCode < NOERROR){
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                  "%s: Cannot OPEN camera: %s.",
                  driverName,  pxd_mesgErrorCode(connectionStatusCode));
        }

        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
            "%s Camera connected;",
            driverName);
            serialConnection = pxd_serialConfigure(UNIT, RESERVED, BAUDRATE, 8, 0, 1, RESERVED, RESERVED, RESERVED);
            if(serialConnection < NOERROR){
                 asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                    "%s: Cannot make serial connection: %s.",
                    driverName,  pxd_mesgErrorCode(connectionStatusCode));
            }

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

        /* Create the thread that does data acquisition */
        status = (epicsThreadCreate("serialTask",
                              epicsThreadPriorityMedium,
                              epicsThreadGetStackSize(epicsThreadStackMedium),
                              (EPICSTHREADFUNC)serialTaskC,
                              this) == NULL);

        serialMsgQue = new epicsMessageQueue(20,20);

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

    asynStatus Pixci::setupAquisition(){
        int binX, binY, sizeX, sizeY;
        sizeX = pxd_imageXdim();
        sizeY = pxd_imageYdim();

        getIntegerParam(ADBinX, &binX);
        if (binX <= 0) {
            binX = 1;
            setIntegerParam(ADBinX, binX);
        }
        getIntegerParam(ADBinY, &binY);
        if (binY <= 0) {
            binY = 1;
            setIntegerParam(ADBinY, binY);
        }

        setIntegerParam(ADSizeX, sizeX/binX);
        setIntegerParam(ADSizeY, sizeY/binY);

        setIntegerParam(ADMaxSizeX, sizeX/binX);
        setIntegerParam(ADMaxSizeY, sizeY/binY);

        setIntegerParam(NDArraySizeX, sizeX/binX);
        setIntegerParam(NDArraySizeY, sizeY/binY);

        callParamCallbacks();

        return asynSuccess;
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
        epicsInt32 binX, binY;
        size_t        dims[2];
        epicsTimeStamp currentTime;
        epicsInt32 numImagesCounter;
        epicsInt32 imageCounter;
        setupAquisition();
        int test;

        for (;;){
            /* waiting for event to be triggered */
            /* TODO: seperate waiting task for linux */
            WaitForSingleObject(g_hEvent, INFINITE);
            lock();

            getIntegerParam(NDArraySizeX, &sizeX);
            getIntegerParam(NDArraySizeY, &sizeY);
            getIntegerParam(ADBinX, &binX);
            getIntegerParam(ADBinY, &binY);

            dims[0] = sizeX;
            dims[1] = sizeY;
            dataType = NDUInt16;

            /* Allocate NDArray */
            pImage = this->pNDArrayPool->alloc(2, dims, dataType, 0, NULL);
            /* Pixel values from an image frame buffer and area of interest are copied into buffer
            pxd_readushort(unit, framebuf, ulxc, ulyc, lrx, lry, membuf, cnt, colorspace)*/
            test = pxd_readushort(UNIT, buf, 0, 0, sizeX, sizeY, (epicsUInt16*)pImage->pData, dims[0] * dims[1] * sizeof(epicsUInt16), "GRAY");

             /* uniqueId and timeStamp must be implemented for standard ADDriver. */
            pImage->uniqueId = imageCounter;
            epicsTimeGetCurrent(&currentTime);
            pImage->timeStamp = currentTime.secPastEpoch + currentTime.nsec / 1.e9;
            updateTimeStamp(&pImage->epicsTS);

            getIntegerParam(NDArrayCounter, &imageCounter);
            getIntegerParam(ADNumImagesCounter, &numImagesCounter);
            imageCounter++;
            numImagesCounter++;

            setIntegerParam(NDArraySize, dims[0] * dims[1] * sizeof(epicsUInt16));
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

    static void serialTaskC(void *drvPvt)
    {
        Pixci *pPvt = (Pixci *)drvPvt;
        pPvt->serialTask();
    }


    void Pixci::serialTask(){
        char outputMsg[20];
        char inputMsg[20];
        const int regAddress = 2; /* index of register in message template */
        const int readOrWriteAdress = 3; /* index of read or write message in template */

        for(;;){
            int i, acquire;
            int outSize, inSize;
            unsigned char setRegister = 0x02;
            unsigned char success = 0x50;
            unsigned char getorset;
            unsigned char reg;
            unsigned char sendStatus;

            /* receive message to be send from messageQue */
            outSize = serialMsgQue->receive(outputMsg,20);
            inSize = writeReadSerial(UNIT, outputMsg, outSize, inputMsg, 20);

            reg = (unsigned char)outputMsg[readOrWriteAdress];
            getorset = (unsigned char)outputMsg[regAddress];
            sendStatus = (unsigned char)inputMsg[0];

            if(getorset == setRegister && sendStatus == success){
                switch(reg){
                    case 0xA1 : /*set X binning*/
                        getIntegerParam(ADAcquire, &acquire);
                        setupAquisition();
                        reloadVideoSettings();
                        acquireStop();
                        if(acquire == 1){
                            acquireImage();
                        }                  
                        break;
                    case 0xA2 : /*set Y binning*/
                        getIntegerParam(ADAcquire, &acquire);
                        setupAquisition();
                        reloadVideoSettings();
                        acquireStop();
                        if(acquire == 1){
                            acquireImage();
                        }     
                        break;
                    default:
                        break;

            }

            callParamCallbacks();

            }

        }
    }

    void Pixci::reloadVideoSettings(){

        {
            #include "videoSettings\Raptor_Photonics_EagleXV_47-10.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }

    }

    asynStatus Pixci::writeSerialRegister(int unit, char Register, char val){
        int status;

        /* template of message to write value to registers */
        char bufout[]  = {0x53, 0xE0, 0x02, 0x00, 0x00, 0x50};
        bufout[3] = Register ;
		bufout[4] = val ;

        /*sending buffer data to the que */
        status = serialMsgQue->send(bufout,6);
        if(status < NOERROR){
            return asynSuccess;
        }
        else{
            return asynError;
        }
    }

    int Pixci::writeReadSerial(int unit, char* serialOut, int msgOutSize, char* serialIn, int serialInBufferSize){
        int count, i;
        char bufOut[50];
        char chkSum;
        int outMsgwait = 0;
        int inMsgwait = 0;
        int inMsgwaitFlag = 0;


        /* checking if any message packer left to read, and clear the buffer by reading it */
        if(pxd_serialRead(unit, RESERVED, NULL, 0) > 0){
            count = pxd_serialRead(unit, 0, serialIn, serialInBufferSize);
        }

        /* wait if any message is in the send que*/
		while(pxd_serialWrite(unit, RESERVED, NULL, 0)<msgOutSize && outMsgwait <50)
		{
			outMsgwait++;
            Sleep(10);
		}
        outMsgwait = 0;

        /* creating checksum to send as the last character of the message */
        for (i=0; i <msgOutSize ; i++ ){
            bufOut[i]=serialOut[i];
            chkSum ^= serialOut[i];
        }
        serialOut[msgOutSize] = chkSum;

        /* sending the seriaOut message */
        count = pxd_serialWrite(unit, RESERVED, serialOut, msgOutSize+1);
        Sleep(130);

        if(count < ERROR){
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                  "%s: Cannot serial write: %s.",
                  driverName,  pxd_mesgErrorCode(count));
            return count;;
        }
        else{
            /*waiting for the reply */
            inMsgwaitFlag = pxd_serialRead(unit, 0, NULL, 0);
             while(inMsgwaitFlag<1 && inMsgwait<20){
                inMsgwait++;
                inMsgwaitFlag = pxd_serialRead(unit, 0, NULL, 0);
                Sleep(10);
            }
            inMsgwait= 0;
            /* read the message and message count */
            count = pxd_serialRead(UNIT, RESERVED, serialIn, serialInBufferSize);
        }

        return count;
    }

    void Pixci::setBin(int val, bool coordinate){
        char hexval;
        char reg;

        /* Assigning corresponding Hex value to send*/
        switch(val){
            case 1: hexval = 0x00;
                    break;
            case 2: hexval = 0x01;
                    break;
            case 4: hexval = 0x03;
                    break;
            case 8: hexval = 0x07;
                    break;
            case 16: hexval = 0x0F;
                    break;
            case 32: hexval = 0x1F;
                    break;
            case 64: hexval = 0x3F;
                    break;
            default:
                    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid binning value %d",val);
                    break;
        }
        if(coordinate){
            reg = 0xA2; /*register for Y coordinate */
        }
        else{
            reg = 0xA1; /*register for X coordinate */
        }

        Pixci::writeSerialRegister(UNIT, reg, hexval);

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
        else if(function == ADBinX){
            Pixci::setBin(value,0);
        }
        else if(function == ADBinY){
            Pixci::setBin(value,1);
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
static const iocshArg pixciConfigArg5 = {"Format file", iocshArgString};
static const iocshArg * const pixciConfigArgs[] =  {&pixciConfigArg0,
                                                          &pixciConfigArg1,
                                                          &pixciConfigArg2,
                                                          &pixciConfigArg3,
                                                          &pixciConfigArg4,
                                                          &pixciConfigArg5};
static const iocshFuncDef configpixci = {"pixciConfig", 6, pixciConfigArgs};
static void configpixciCallFunc(const iocshArgBuf *args)
{
  pixciConfig(args[0].sval, args[1].ival, args[2].ival, args[3].ival,
                    args[4].ival, args[5].sval);
}

static void pixciRegister(void)
{
  iocshRegister(&configpixci, configpixciCallFunc);
}

extern "C" {
epicsExportRegistrar(pixciRegister);
}
