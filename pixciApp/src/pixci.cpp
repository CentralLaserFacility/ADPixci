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

#define BINNING1 0
#define BINNING2 2
#define BINNING4 4
#define BINNING8 8
#define BINNING16 16

#define BINNINGSETTINGS_1X1 "videoSettings\Raptor_Photonics_EagleXV_47-10.fmt"
#define BINNINGSETTINGS_2X2 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning2x2.fmt"
#define BINNINGSETTINGS_4X4 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning4x4.fmt"
#define BINNINGSETTINGS_8X8 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning8x8.fmt"
#define BINNINGSETTINGS_16X16 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning16x16.fmt"





/*
 * @brief C Function prototypes to tie in with EPICS
 * run acquire task
 * @param drvPvt
 */
static void acquireTaskC(void *drvPvt);

// static void serialTaskC(void *drvPvt);
static void paramTaskC(void *drvPvt);

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

        status = (epicsThreadCreate("paramTask",
                              epicsThreadPriorityMedium,
                              epicsThreadGetStackSize(epicsThreadStackMedium),
                              (EPICSTHREADFUNC)paramTaskC,
                              this) == NULL);

        paramMsgQue = new epicsMessageQueue(20,8);

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

        setIntegerParam(ADSizeX, sizeX);
        setIntegerParam(ADSizeY, sizeY);

        setIntegerParam(ADMaxSizeX, sizeX);
        setIntegerParam(ADMaxSizeY, sizeY);

        setIntegerParam(NDArraySizeX, sizeX);
        setIntegerParam(NDArraySizeY, sizeY);

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
            setIntegerParam(ADAcquire, 1);
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
            setIntegerParam(ADAcquire, 0);
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
            dataType = NDUInt8;

            /* Allocate NDArray */
            pImage = this->pNDArrayPool->alloc(2, dims, dataType, 0, NULL);
            /* Pixel values from an image frame buffer and area of interest are copied into buffer
            pxd_readuchar(unit, framebuf, ulxc, ulyc, lrx, lry, membuf, cnt, colorspace)*/
            pxd_readuchar(UNIT, buf, 0, 0, sizeX, sizeY, (epicsUInt8*)pImage->pData, dims[0] * dims[1] * sizeof(epicsUInt8), "GRAY");

             /* uniqueId and timeStamp must be implemented for standard ADDriver. */
            pImage->uniqueId = imageCounter;
            epicsTimeGetCurrent(&currentTime);
            pImage->timeStamp = currentTime.secPastEpoch + currentTime.nsec / 1.e9;
            updateTimeStamp(&pImage->epicsTS);

            getIntegerParam(NDArrayCounter, &imageCounter);
            getIntegerParam(ADNumImagesCounter, &numImagesCounter);
            imageCounter++;
            numImagesCounter++;

            setIntegerParam(NDArraySize, dims[0] * dims[1] * sizeof(epicsUInt8));
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


    static void paramTaskC(void *drvPvt)
    {
        Pixci *pPvt = (Pixci *)drvPvt;
        pPvt->paramTask();
    }

    void Pixci::paramTask(){
        epicsInt32 functionAndVal[2];
        epicsInt32 function;
        epicsInt32 val;
        asynStatus status;
        epicsInt32 acquire;
        for(;;){
            paramMsgQue->receive(functionAndVal,8);
            function = functionAndVal[0];
            val = functionAndVal[1];

            if(function==ADBinX | function==ADBinY){
                status = Pixci::setBin(val,0);
                if (status==asynSuccess)
                {
                    setIntegerParam(ADBinX, val);
                    setIntegerParam(ADBinY, val);
                    callParamCallbacks();
                    getIntegerParam(ADAcquire, &acquire);
                    reloadVideoSettings(val);
                    acquireStop();
                    setupAquisition();
                    if(acquire == 1){
                        acquireImage();
                    }       
                }
                
            }

        }
    }

    void Pixci::reloadVideoSettings(int binn){
        switch(binn){
            case BINNING2:
                {
                    #include BINNINGSETTINGS_2X2
                    pxd_videoFormatAsIncludedInit(0);
                    pxd_videoFormatAsIncluded(0);
                }
                break;
            case BINNING4:
                {
                    #include BINNINGSETTINGS_4X4
                    pxd_videoFormatAsIncludedInit(0);
                    pxd_videoFormatAsIncluded(0);
                }
                break;
            case BINNING8:
                {
                    #include BINNINGSETTINGS_8X8
                    pxd_videoFormatAsIncludedInit(0);
                    pxd_videoFormatAsIncluded(0);
                }
                break;
            case BINNING16:
                {
                    #include BINNINGSETTINGS_16X16
                    pxd_videoFormatAsIncludedInit(0);
                    pxd_videoFormatAsIncluded(0);
                }
                break;
            default:
                {
                    #include BINNINGSETTINGS_1X1
                    pxd_videoFormatAsIncludedInit(0);
                    pxd_videoFormatAsIncluded(0);
                }
                break;
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

    asynStatus Pixci::setBin(int val, bool coordinate){
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

        reg = 0xA1;
        Pixci::writeSerialRegister(UNIT, reg, hexval);
         reg = 0xA2;
        return Pixci::writeSerialRegister(UNIT, reg, hexval);

    }

    asynStatus Pixci::writeInt32(asynUser *pasynUser, epicsInt32 value){
        int function = pasynUser->reason;
        int status = asynSuccess;
        static const char *functionName = "writeInt32";

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
            addToParamQue(function,value);
        }
        else if(function == ADBinY){
            addToParamQue(function,value);
        }
        else{
            status = ADDriver::writeInt32(pasynUser, value);
        }

        return (asynStatus) status;
    }

    void Pixci::addToParamQue(epicsInt32 function, epicsInt32 value){
        epicsInt32 functionAndVal[2];
        functionAndVal[0] = function;
        functionAndVal[1] = value;
        /*sending buffer data to the que */
        paramMsgQue->send(functionAndVal,8);

    }

    asynStatus Pixci::writeSerialRegister(int unit, char Register, char val){
        asynStatus status;
        int inSize;
        char inputMsg[20];
        unsigned char success = 0x50;
        
        /* template of message to write value to registers */
        char bufout[]  = {0x53, 0xE0, 0x02, 0x00, 0x00, 0x50};
        bufout[3] = Register ;
		bufout[4] = val ;

        /*writing to serial connection*/
        inSize = writeReadSerial(UNIT, bufout, 6, inputMsg, 20);

        if(inSize<NOERROR){
            return asynError;
        }

        if(inputMsg[0]==success){
            return asynSuccess;
        }

        return asynError;
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
