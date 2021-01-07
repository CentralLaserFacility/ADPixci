/**
 * @brief This is a driver for PIXCI frame grabber from epix, inc. Developed for Eagle XV CCD from Raptor photonics
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include<string>


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

using namespace std;

#define FORMAT "" // Video format configuration name.
#define DRIVERPARMS "" // Default , user '-QU 0' for not using interrupts.
#define UNIT 1 // Unit to be selected for streaming, eb1 model only have 1 unit.
#define NOERROR 0 // Errors are defined as integers below zero.
#define RESERVED 0
#define BAUDRATE 115200

#define SUCCESS_MESSAGE 0x50

#define BINNING1 1
#define BINNING2 2
#define BINNING4 4
#define BINNING8 8
#define BINNING16 16
#define BINNING32 32

#define BINNINGSETTINGS_1X1 "videoSettings\Raptor_Photonics_EagleXV_47-10.fmt"
#define BINNINGSETTINGS_1X2 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning1x2.fmt"
#define BINNINGSETTINGS_1X4 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning1x4.fmt"
#define BINNINGSETTINGS_1X8 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning1x8.fmt"
#define BINNINGSETTINGS_1X16 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning1x16.fmt"
#define BINNINGSETTINGS_1X32 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning1x32.fmt"
#define BINNINGSETTINGS_2X1 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning2x1.fmt"
#define BINNINGSETTINGS_2X2 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning2x2.fmt"
#define BINNINGSETTINGS_2X4 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning2x4.fmt"
#define BINNINGSETTINGS_2X8 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning2x8.fmt"
#define BINNINGSETTINGS_2X16 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning2x16.fmt"
#define BINNINGSETTINGS_2X32 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning2x32.fmt"
#define BINNINGSETTINGS_4X1 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning4x1.fmt"
#define BINNINGSETTINGS_4X2 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning4x2.fmt"
#define BINNINGSETTINGS_4X4 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning4x4.fmt"
#define BINNINGSETTINGS_4X8 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning4x8.fmt"
#define BINNINGSETTINGS_4X16 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning4x16.fmt"
#define BINNINGSETTINGS_4X32 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning4x32.fmt"
#define BINNINGSETTINGS_8X1 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning8x1.fmt"
#define BINNINGSETTINGS_8X2 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning8x2.fmt"
#define BINNINGSETTINGS_8X4 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning8x4.fmt"
#define BINNINGSETTINGS_8X8 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning8x8.fmt"
#define BINNINGSETTINGS_8X16 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning8x16.fmt"
#define BINNINGSETTINGS_8X32 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning8x32.fmt"
#define BINNINGSETTINGS_16X1 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning16x1.fmt"
#define BINNINGSETTINGS_16X2 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning16x2.fmt"
#define BINNINGSETTINGS_16X4 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning16x4.fmt"
#define BINNINGSETTINGS_16X8 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning16x8.fmt"
#define BINNINGSETTINGS_16X16 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning16x16.fmt"
#define BINNINGSETTINGS_16X32 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning16x32.fmt"
#define BINNINGSETTINGS_32X1 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning32x1.fmt"
#define BINNINGSETTINGS_32X2 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning32x2.fmt"
#define BINNINGSETTINGS_32X4 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning32x4.fmt"
#define BINNINGSETTINGS_32X8 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning32x8.fmt"
#define BINNINGSETTINGS_32X16 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning32x16.fmt"
#define BINNINGSETTINGS_32X32 "videoSettings\Raptor_Photonics_EagleXV_47-10_binning32x32.fmt"


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
        int connectionStatusCode = 0;
        int serialConnection = 0;

        createParam(SoftTriggerParamString,     asynParamInt32,     &PR_SoftTrigger);
        createParam(TriggerPolarityParamString,     asynParamInt32,     &PR_TriggerPolarity);
        createParam(UpdateTemperatureString,  asynParamInt32, &PR_UpdateTemperature);
        createParam(TemperaturePCBString,  asynParamFloat64, &PR_TemperaturePcb);
        createParam(ToggleTecString,  asynParamInt32, &PR_ToggleTec);
        createParam(ToggleGainString,  asynParamInt32, &PR_ToggleGain);
        createParam(ToggleFPGACommsString,  asynParamInt32, &PR_ToggleFpgaComms);

        createParam(UpdateStatusString,  asynParamInt32, &PR_UpdateStatus);
        createParam(BuildDateString,  asynParamOctet, &PR_BuildDate);
        createParam(ADCCalibrationZeroDegreeString ,  asynParamInt32, &PR_ADCCalibrationZeroDegree);
        createParam(ADCCalibrationFortyDegreeString,  asynParamInt32, &PR_ADCCalibrationFortyDegree);
        createParam(DACCalibrationZeroDegreeString ,  asynParamInt32, &PR_DACCalibrationZeroDegree);
        createParam(DACCalibrationFortyDegreeString,  asynParamInt32, &PR_DACCalibrationFortyDegree);

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
        status |= (epicsThreadCreate("acquireTask",
                              epicsThreadPriorityMedium,
                              epicsThreadGetStackSize(epicsThreadStackMedium),
                              (EPICSTHREADFUNC)acquireTaskC,
                              this) == NULL);

        status |= (epicsThreadCreate("paramTask",
                              epicsThreadPriorityMedium,
                              epicsThreadGetStackSize(epicsThreadStackMedium),
                              (EPICSTHREADFUNC)paramTaskC,
                              this) == NULL);

        paramMsgQue = new epicsMessageQueue(20,16);

        //Updating all the PVs related to the status of device
        updateStatus(true); // Also update the manufacturers data
        status |= updateIntialPVs();
        if(status == asynError){
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to initialize the detector\n");
            return;
        }

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
        int binX, binY, sizeX, sizeY, RoiSizeX, RoiSizeY;
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

        getIntegerParam(ADSizeX, &RoiSizeX);
        getIntegerParam(ADSizeY, &RoiSizeY);
        // setIntegerParam(ADSizeX, sizeX);
        // setIntegerParam(ADSizeY, sizeY);

        setIntegerParam(NDArraySizeX, RoiSizeX/binX);
        setIntegerParam(NDArraySizeY, RoiSizeY/binY);

        callParamCallbacks();

        return asynSuccess;
    }

    asynStatus Pixci::acquireImage(){

        /* TODO: implement all acquisition method like trigger, ringbuffer etc */
        static const char *functionName = "acquireImage";
        int error;
        pxbuffer_t buffer = 1L;         // Image frame buffer
        /* live capture the image into frame buffer */
        error = pxd_goLive(UNIT, buffer);
        if(error < NOERROR){
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                  "acquisition error: %s : %s", functionName, pxd_mesgErrorCode(error));
            return asynError;
        }
        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
                  "acquisition initiated ");
            return asynSuccess;
        }
    }

   
    asynStatus Pixci::acquireStop(){
        static const char *functionName = "acquireStop";
        int error;
        /* stop the live capturing */
        error = pxd_goUnLive(UNIT);
        if(error < NOERROR){
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                  "live couldn't stop: %s : %s",functionName, pxd_mesgErrorCode(error));
            return asynError;
        }
        else{
            asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
                  "live stopped \n");
            return asynSuccess;
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
        epicsInt32 arrayCallbacks;
        setupAquisition();

        for (;;){
            /* waiting for event to be triggered */
            /* TODO: seperate waiting task for linux */
            WaitForSingleObject(g_hEvent, INFINITE);
            

            getIntegerParam(NDArraySizeX, &sizeX);
            getIntegerParam(NDArraySizeY, &sizeY);
            getIntegerParam(ADBinX, &binX);
            getIntegerParam(ADBinY, &binY);
            getIntegerParam(NDArrayCallbacks, &arrayCallbacks);

            dims[0] = sizeX;
            dims[1] = sizeY;
            dataType = NDUInt8;

            if (arrayCallbacks)
            {
                lock();
                /* Allocate NDArray */
                pImage = this->pNDArrayPool->alloc(2, dims, dataType, 0, NULL);
                /* Pixel values from an image frame buffer and area of interest are copied into buffer
                pxd_readuchar(unit, framebuf, ulxc, ulyc, lrx, lry, membuf, cnt, colorspace)*/
                pxd_readuchar(UNIT, buf, 0, 0, sizeX, sizeY, (epicsUInt8 *)pImage->pData, dims[0] * dims[1] * sizeof(epicsUInt8), "GRAY");

                /* uniqueId and timeStamp must be implemented for standard ADDriver. */
                pImage->uniqueId = imageCounter;
                epicsTimeGetCurrent(&currentTime);
                pImage->timeStamp = currentTime.secPastEpoch + currentTime.nsec / 1.e9;
                updateTimeStamp(&pImage->epicsTS);
                unlock();
                /*Call doCallbacksGenericPointer() so that registered clients can get the values of the new arrays.
                Drivers must release their mutex by calling this->unlock() before they call doCallbacksGenericPointer(),
                or a deadlock can occur if the plugin makes a call to one of the driver functions.*/
                doCallbacksGenericPointer(pImage, NDArrayData, 0);
                if (this->pArrays[0])
                    this->pArrays[0]->release();
                this->pArrays[0] = pImage;
            }

            getIntegerParam(NDArrayCounter, &imageCounter);
            getIntegerParam(ADNumImagesCounter, &numImagesCounter);
            imageCounter++;
            numImagesCounter++;

            setIntegerParam(NDArraySize, dims[0] * dims[1] * sizeof(epicsUInt8));
            setIntegerParam(NDArrayCounter, imageCounter);
            setIntegerParam(ADNumImagesCounter, numImagesCounter);
            callParamCallbacks();
        }
    }


    static void paramTaskC(void *drvPvt)
    {
        Pixci *pPvt = (Pixci *)drvPvt;
        pPvt->paramTask();
    }

    void Pixci::paramTask(){
        epicsFloat64 functionAndVal[2];
        epicsInt32 function;
        epicsFloat64 val;
        asynStatus status;
        epicsInt32 acquire;

        for(;;){
            paramMsgQue->receive(functionAndVal,16);
            function = (int)functionAndVal[0];
            val = functionAndVal[1];
            if(function==ADBinX){
                status = Pixci::setBin(val,0);
                if (status==asynSuccess)
                {   
                    setIntegerParam(ADBinX, val); //Updating the binX value.
                    // callParamCallbacks();
                    getIntegerParam(ADAcquire, &acquire); //Getting the ADAcquire value. 
                    reloadVideoSettings(); //Video settings have to be loaded respective of binning value.
                    acquireStop(); //Acquire have to be stopped before calling setupAcquisition.
                    setupAquisition();
                    if(acquire == 1){
                        acquireImage();//starting acquisition if acquisition was running before.
                    }       
                }
                
            }
            else if(function==ADBinY){
                status = Pixci::setBin(val,1);
                if(status==asynSuccess){
                    setIntegerParam(ADBinY, val);
                    callParamCallbacks();
                    getIntegerParam(ADAcquire, &acquire);
                    reloadVideoSettings();
                    acquireStop();
                    setupAquisition();
                    if(acquire == 1){
                        acquireImage();
                    }       
                }
            }
            else if(function==ADReadStatus){
                printf("Reading Exposure from the camera \n");    
            }
            else if(function==ADTriggerMode){
                int acquisitionStatus;
                int previousTriggerMode;
                status = setTriggerMode(val);
                if(status == asynSuccess){
                    if(val == PR_BUTTON_TRIGGER){
                        /* In button triggermode, for WaitForSingleObject function to be notified pxd_goLive should be
                        called. For that acquireImage() function is called.
                        */
                        status = acquireImage();
                    }
                    else{
                        /* When changes the acquiremode from button triggered to any another trigger mode,
                            we have to check the ADAcquire status  stop acquision if ADAcquire is in 'Stop' state.
                            Because in button trigger mode acquireImage() is called irrespective of ADAcquire status.
                        */
                        getIntegerParam(ADTriggerMode, &previousTriggerMode);
                        if (previousTriggerMode == PR_BUTTON_TRIGGER)
                        {
                            getIntegerParam(ADAcquire, &acquisitionStatus);

                            if (acquisitionStatus == 0)
                            {   
                                /* acquisiton is stopped if ADAcquire is on stop state*/
                                acquireStop();
                            }
                        }
                    }
                    setIntegerParam(ADTriggerMode, val);
                }
            }
            else if(function==PR_SoftTrigger){
                /* if trigger mode is button trigger then, do the soft trigger else print error */
                int triggerMode;
                getIntegerParam(ADTriggerMode, &triggerMode);
                if(triggerMode == PR_BUTTON_TRIGGER){
                    char reg = 0xD4;
                    char hexval = 0x01;
                    status = Pixci::writeSerialRegister(UNIT, reg, hexval);
                }
                else{
                    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Button Trigger mode is not selected");
                }         
            }
            else if (function==PR_UpdateStatus)
            {
                updateStatus(true);
            }
            else if (function==PR_UpdateTemperature)
            {
                updateStatus();
            }
            else if(function == ADAcquirePeriod){
                if(val != 0){
                    status = setFrameRate(1/val);
                    if(status == asynSuccess){
                        double readBackFrameRate;
                        readBackFrameRate = getFrameRate();
                        if(readBackFrameRate > 0){
                            setDoubleParam(ADAcquirePeriod, (1/readBackFrameRate));
                        }
                    }
                }
            }
            else if (function == ADTemperature)
            {
                status = setTecTemperature(val);
                if(status == asynSuccess)
                {
                    double tecTemperature = getTecTemperature();
                    setDoubleParam(ADTemperature,tecTemperature);
                }
            }
            else if (function == PR_ToggleTec)
            {
                status = toggleTec(val);
                if(status == asynSuccess)
                {
                    setIntegerParam(PR_ToggleTec, isTecEnabled());
                }
            }
            else if (function == PR_ToggleGain)
            {
                status = toggleGain(val);
                if(status == asynSuccess)
                {
                    setIntegerParam(PR_ToggleGain, isGainEnabled());
                }
            }
            else if (function == PR_ToggleFpgaComms)
            {
                status = toggleFpgaComms(val);
                if(status == asynSuccess)
                {
                    setIntegerParam(PR_ToggleFpgaComms, isFpgaCommsEnabled());
                }
            }
            else if(function == ADAcquireTime){
                if(val!=0){
                    status = setAcquireTime(val);
                    if(status == asynSuccess){
                        double readBackAcquireTime;
                        readBackAcquireTime = getAcquireTime();
                        if(readBackAcquireTime > 0){
                            setDoubleParam(ADAcquireTime, readBackAcquireTime);
                        }
                    }
                }

            }

            else if(function == ADMinX){
                epicsInt32 MaxSizeX, MinX, SizeX;
                getIntegerParam(ADMaxSizeX, &MaxSizeX);
                getIntegerParam(ADSizeX, &SizeX);

                MinX = (val > MaxSizeX) ? MaxSizeX : val;
                if((SizeX + MinX) > MaxSizeX){
                    SizeX = MaxSizeX - MinX;
                    status = setRoiSizeX(SizeX);
                    if(status == asynSuccess){
                        setIntegerParam(ADSizeX,getRoiSizeX());
                        getIntegerParam(ADAcquire, &acquire);
                        reloadVideoSettings();
                        acquireStop();
                        setupAquisition();
                        if(acquire == 1){
                            acquireImage();
                        }       
                    }
                }
                
                status = setRoiOffsetX(MinX);
                if(status == asynSuccess){
                    setIntegerParam(ADMinX,getRoiOffsetX());
                }
            }
            else if(function == ADMinY){
                epicsInt32 MaxSizeY, MinY, SizeY;
                getIntegerParam(ADMaxSizeY, &MaxSizeY);
                getIntegerParam(ADSizeY, &SizeY);

                MinY = (val > MaxSizeY) ? MaxSizeY : val;
                if((SizeY + MinY) > MaxSizeY){
                    SizeY = MaxSizeY - MinY;
                    status = setRoiSizeY(SizeY);
                    if(status == asynSuccess){
                        setIntegerParam(ADSizeY,getRoiSizeY());
                        getIntegerParam(ADAcquire, &acquire);
                        reloadVideoSettings();
                        acquireStop();
                        setupAquisition();
                        if(acquire == 1){
                            acquireImage();
                        }       
                    }
                }

                status == setRoiOffsetY(val);
                if(status = asynSuccess){
                    setIntegerParam(ADMinY,getRoiOffsetY());
                }
            }
            else if(function == ADSizeX){
                epicsInt32 MaxSizeX, MinX, SizeX;
                getIntegerParam(ADMaxSizeX, &MaxSizeX);
                getIntegerParam(ADMinX, &MinX);
                SizeX = (val > MaxSizeX) ? MaxSizeX : val;

                if((SizeX + MinX) > MaxSizeX){
                    MinX = MaxSizeX - SizeX;
                    status = setRoiOffsetX(MinX);
                    if(status == asynSuccess){
                        setIntegerParam(ADMinX,getRoiOffsetX()); 
                    }
                }
                status == setRoiSizeX(SizeX);
                
                if(status == asynSuccess){
                    setIntegerParam(ADSizeX,getRoiSizeX());
                    callParamCallbacks();
                    getIntegerParam(ADAcquire, &acquire);
                    reloadVideoSettings();
                    acquireStop();
                    setupAquisition();
                    if(acquire == 1){
                        acquireImage();
                    }       
                }
            }
            else if(function == ADSizeY){
                epicsInt32 MaxSizeY, MinY, SizeY;
                getIntegerParam(ADMaxSizeY, &MaxSizeY);
                getIntegerParam(ADMinY, &MinY);
                SizeY = (val > MaxSizeY) ? MaxSizeY : val;

                if((SizeY + MinY) > MaxSizeY){
                    MinY = MaxSizeY - SizeY;
                    status = setRoiOffsetY(MinY);
                    if(status == asynSuccess){
                        setIntegerParam(ADMinY,getRoiOffsetY());
                    }
                }
                status == setRoiSizeY(SizeY);
                
                if(status == asynSuccess){
                    setIntegerParam(ADSizeY,getRoiSizeY());
                    callParamCallbacks();
                    getIntegerParam(ADAcquire, &acquire);
                    reloadVideoSettings();
                    acquireStop();
                    setupAquisition();
                    if(acquire == 1){
                        acquireImage();
                    }       
                }
            }
            else if(function == PR_TriggerPolarity){
                int triggerMode;
                if(val == PR_EXT_RISING_EDGE){
                    setIntegerParam(PR_TriggerPolarity, PR_EXT_RISING_EDGE);
                }
                else if(val == PR_EXT_FALLING_EDGE){
                    setIntegerParam(PR_TriggerPolarity, PR_EXT_FALLING_EDGE);
                }
                callParamCallbacks();
                getIntegerParam(ADTriggerMode, &triggerMode);
                if(triggerMode == PR_EXTERNAL){
                    setTriggerMode(PR_EXTERNAL);
                }

            }
            callParamCallbacks();
        }
    }

    void Pixci::reloadVideoSettings(){
        epicsInt32 sizeX;
        epicsInt32 sizeY;
        getIntegerParam(ADBinX, &sizeX);
        getIntegerParam(ADBinY, &sizeY);

        switch (sizeX)
        {
        case BINNING1:
            if (sizeY == BINNING1)
            {
                #include BINNINGSETTINGS_1X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING2)
            {
                #include BINNINGSETTINGS_1X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING4)
            {
                #include BINNINGSETTINGS_1X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING8)
            {
                #include BINNINGSETTINGS_1X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING16)
            {
                #include BINNINGSETTINGS_1X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING32)
            {
                #include BINNINGSETTINGS_1X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            break;
        case BINNING2:
            if (sizeY == BINNING1)
            {
                #include BINNINGSETTINGS_2X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING2)
            {
                #include BINNINGSETTINGS_2X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING4)
            {
                #include BINNINGSETTINGS_2X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING8)
            {
                #include BINNINGSETTINGS_2X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING16)
            {
                #include BINNINGSETTINGS_2X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING32)
            {
                #include BINNINGSETTINGS_2X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            break;
        case BINNING4:
            if (sizeY == BINNING1)
            {
                #include BINNINGSETTINGS_4X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING2)
            {
                #include BINNINGSETTINGS_4X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING4)
            {
                #include BINNINGSETTINGS_4X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING8)
            {
                #include BINNINGSETTINGS_4X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING16)
            {
                #include BINNINGSETTINGS_4X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING32)
            {
                #include BINNINGSETTINGS_4X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            break;
        case BINNING8:
            if (sizeY == BINNING1)
            {
                #include BINNINGSETTINGS_8X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING2)
            {
                #include BINNINGSETTINGS_8X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING4)
            {
                #include BINNINGSETTINGS_8X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING8)
            {
                #include BINNINGSETTINGS_8X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING16)
            {
                #include BINNINGSETTINGS_8X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING32)
            {
                #include BINNINGSETTINGS_8X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            break;
        case BINNING16:
            if (sizeY == BINNING1)
            {
                #include BINNINGSETTINGS_16X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING2)
            {
                #include BINNINGSETTINGS_16X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING4)
            {
                #include BINNINGSETTINGS_16X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING8)
            {
                #include BINNINGSETTINGS_16X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING16)
            {
                #include BINNINGSETTINGS_16X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING32)
            {
                #include BINNINGSETTINGS_16X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            break;
        case BINNING32:
            if (sizeY == BINNING1)
            {
                #include BINNINGSETTINGS_32X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING2)
            {
                #include BINNINGSETTINGS_32X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING4)
            {
                #include BINNINGSETTINGS_32X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING8)
            {
                #include BINNINGSETTINGS_32X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING16)
            {
                #include BINNINGSETTINGS_32X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else if (sizeY == BINNING32)
            {
                #include BINNINGSETTINGS_32X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            break;
        default:
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid binning value");
            break;
        }
    }

    unsigned long long Pixci::UcharToLong( char* cval){
        unsigned long long lval = 0;
        lval += (unsigned long )(unsigned char)cval[4];
        lval += ((unsigned long )(unsigned char)cval[3])<<8;
        lval += ((unsigned long )(unsigned char)cval[2])<<16;
        lval += ((unsigned long )(unsigned char)cval[1])<<24;
        lval += ((unsigned long )(unsigned char)cval[0])<<32;
        return lval;
    }

    void Pixci::longTouchar(long lval, char* cval){
        cval[0] = (char)((lval & 0xFF00000000) >> 32 );
        cval[1]  = (char)((lval & 0x00FF000000) >> 24 );
        cval[2] = (char)((lval & 0x0000FF0000) >> 16 );
        cval[3] = (char)((lval & 0x000000FF00) >> 8 );
        cval[4] = (char)((lval & 0x00000000FF) );
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
                    return asynError;
                    break;
        }

        if(coordinate==false){
            reg = 0xA1;
        }
        else{
            reg = 0xA2;
        }
        return Pixci::writeSerialRegister(UNIT, reg, hexval);

    }

    asynStatus Pixci::setFrameRate(double frameRate){
        unsigned long frameRateCount;
        unsigned long long lval;
        char frameRateHexVal[5] = {0,0,0,0,0};
        frameRateCount = (unsigned long)(40e6/frameRate);
        longTouchar(frameRateCount, frameRateHexVal);

        writeSerialRegister(UNIT, 0xDC, frameRateHexVal[0]);
        writeSerialRegister(UNIT, 0xDD, frameRateHexVal[1]);
        writeSerialRegister(UNIT, 0xDE, frameRateHexVal[2]);
        writeSerialRegister(UNIT, 0xDF, frameRateHexVal[3]);
        return writeSerialRegister(UNIT, 0xE0, frameRateHexVal[4]);
    }

    double Pixci::getFrameRate(){
        char cval[5] ={0,0,0,0,0};
        double frameRate = 0.0;
        asynStatus status;
        unsigned long long frameRateCount = 0;
        readSerialRegister(0XDC, &cval[0]);
        readSerialRegister(0xDD, &cval[1]);
        readSerialRegister(0xDE, &cval[2]);
        readSerialRegister(0XDF, &cval[3]);
        readSerialRegister(0XE0, &cval[4]);

        frameRateCount = UcharToLong(cval);
        if (frameRateCount > 0){
            frameRate = 40e6/double(frameRateCount);
        }
        return frameRate;
    }

    double Pixci::convertAdcCountToCentigrade(INT16 adcCount)
    {       
        return (ADC_M*adcCount)+ADC_C; //temperature in centigrade
    }

    INT16 Pixci::convertCentigradeToDacCount(double temperature)
    {
        return (temperature-DAC_C)/DAC_M;
    }

    double Pixci::convertDacCountToCentigrade(INT16 dacCount)
    {
        return (DAC_M*dacCount)+DAC_C; //temperature in centigrade
    }

    double Pixci::getTemperatureActual()
    {
        char cval[2] ={0,0};

        readSerialRegister(0X6E, 0x00, &cval[0]);
        readSerialRegister(0X6F, 0x00, &cval[1]);

        INT16 adcCount = 0;
        adcCount += (INT16 )(unsigned char)cval[1];
        adcCount += ((INT16 )(unsigned char)cval[0])<<8;

        return convertAdcCountToCentigrade(adcCount);    
    }

    double Pixci::getTemperaturePcb()
    {
        char cval[2] ={0,0};

        readSerialRegister(0X70, 0x00, &cval[1]);
        readSerialRegister(0X71, 0x00, &cval[0]);

        INT16 lval = 0;
        lval += (INT16 )(unsigned char)cval[0];
        lval += (INT16 )(unsigned char)(cval[1] & 0x0F)<<8;
        double temperature  = lval/16.0f;
       
        return temperature;  
    }

    double Pixci::getTecTemperature()
    {
        char cval[2] ={0,0};

        readSerialRegister(0X03, &cval[1]);
        readSerialRegister(0X04, &cval[0]);

        INT16 lval = 0;
        lval += (INT16)(unsigned char)cval[0];
        lval += (INT16)(unsigned char)(cval[1] & 0x0F)<<8;

        return convertDacCountToCentigrade(lval);
    }

    asynStatus Pixci::setTecTemperature(double temperature)
    {
        INT16 dacCount = convertCentigradeToDacCount(temperature);
        
        char cval[2] ={0,0};
        cval[0] = (char)((dacCount & 0x0F00) >> 8 );
        cval[1] = (char)((dacCount & 0x00FF) );

        writeSerialRegister(UNIT, 0x03, cval[0]);
        return writeSerialRegister(UNIT, 0x04, cval[1]);
    }

    unsigned char Pixci::getFpgaStatus()
    {
        char cval = 0;
        readSerialRegister(0x00, &cval);
        //TODO: implement proper error handling
        return (unsigned char)cval;
    }

    asynStatus Pixci::toggleTec(bool enableTec)
    {
        unsigned char fpgaStatus = getFpgaStatus();
        if(enableTec)
            return writeSerialRegister(UNIT, 0x00, fpgaStatus | 0x01); // setting first bit = 1
         else
            return writeSerialRegister(UNIT, 0x00, fpgaStatus & ~(0x01)); //setting first bit = 0
    }

    bool Pixci::isTecEnabled()
    {
        unsigned char fpgaStatus = getFpgaStatus();
        return (fpgaStatus & 0x01) != 0; // check the first bit is not 0
    }

    asynStatus Pixci::toggleGain(bool enableGain)
    {
        unsigned char fpgaStatus = getFpgaStatus();       
        if(enableGain)
            return writeSerialRegister(UNIT, 0x00, fpgaStatus | (1 << 7)); // setting last bit = 1
         else
            return writeSerialRegister(UNIT, 0x00, fpgaStatus & ~(1 << 7)); //setting last bit = 0
            
    }

    bool Pixci::isGainEnabled()
    {
        unsigned char fpgaStatus = getFpgaStatus();
       return (fpgaStatus & (1 << 7)) != 0; // check the last bit is not 0
    }

    unsigned char Pixci::getSystemStatus()
    {
        char cval = 0;
        char inputMsg[2];
        int inSize;
        char first_bufout[] = {0x49, 0x50};

        /*writing to serial connection*/
        inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 2);

        if(inputMsg[1] == SUCCESS_MESSAGE){
            cval=inputMsg[0];       
        }
        
        //TODO: Need proper error handling, same is for reading serial register as else where
        return (unsigned char)cval; //cval will be 0x00 if there is no success
       
    }

    asynStatus Pixci::setSystemStatus(char val){
        int inSize;
        char inputMsg[1];
        
        /* template of message to write value to registers */
        char bufout[]  = {0x4F, 0x00, 0x50};
		bufout[1] = val ;

        /*writing to serial connection*/
        inSize = writeReadSerial(UNIT, bufout, sizeof(bufout), inputMsg, 1);

        if(inSize<NOERROR){
            return asynError;
        }

        if(inputMsg[0]==SUCCESS_MESSAGE){
            return asynSuccess;
        }

        return asynError;
    }

    asynStatus Pixci::toggleFpgaComms(bool enableFpgaComms)
    {
        unsigned char systemStatus = getSystemStatus();
        if(enableFpgaComms)
           return setSystemStatus(systemStatus | 0x01); // setting first bit = 1
         else
            return setSystemStatus(systemStatus & ~(0x01)); //setting first bit = 0

    }

    bool Pixci::isFpgaCommsEnabled()
    {
        unsigned char systemStatus = getSystemStatus();
        return (systemStatus & 0x01) != 0; // check the first bit is not 0
    }
    
    asynStatus Pixci::setAcquireTime(double AcquireTime){
        unsigned long AcquireTimeCount;
        unsigned long long lval;
        char AcquireTimeHexVal[5] = {0,0,0,0,0};
        AcquireTimeCount = (unsigned long)(AcquireTime*40e6/10e2); 
        longTouchar(AcquireTimeCount, AcquireTimeHexVal);

        writeSerialRegister(UNIT, 0xED, AcquireTimeHexVal[0]);
        writeSerialRegister(UNIT, 0xEE, AcquireTimeHexVal[1]);
        writeSerialRegister(UNIT, 0xEF, AcquireTimeHexVal[2]);
        writeSerialRegister(UNIT, 0xF0, AcquireTimeHexVal[3]);
        return writeSerialRegister(UNIT, 0xF1, AcquireTimeHexVal[4]);
    }

    double Pixci::getAcquireTime(){
        char cval[5] ={0,0,0,0,0};
        double AcquireTime = 0.0;
        asynStatus status;
        unsigned long long AcquireTimeCount = 0;
        readSerialRegister(0XED, &cval[0]);
        readSerialRegister(0xEE, &cval[1]);
        readSerialRegister(0xEF, &cval[2]);
        readSerialRegister(0XF0, &cval[3]);
        readSerialRegister(0XF1, &cval[4]);

        AcquireTimeCount = UcharToLong(cval);
        if (AcquireTimeCount > 0){
            AcquireTime = (double(AcquireTimeCount)/40e6)*10e2;
        }
        return AcquireTime;
    }

    asynStatus Pixci::writeInt32(asynUser *pasynUser, epicsInt32 value){
        int function = pasynUser->reason;
        asynStatus status = asynSuccess;
        static const char *functionName = "writeInt32";

        /* There are two types int32 parameters, parameters that uses serial communication and parameters that 
        does not uses serial communication.

        Parameters that dont use serial communication, can be implemented by calling respective XCLIB function
        directly ex: ADAcquire.

        Parameters that uses serial communication cannot be implmented directly here because,
        serial communication may take some time to execute and return the status. To manage that issue
        a paramTask thread is created. functions that uses serial communicaiton is called inside that
        thread. So waiting for the status from serial communication won't affect the whole program.
        addParamQue(funcation, value) is used to add the parameters change in a que. paramTask thread will
        read the que and execute respective function in FIFO mode. ex: ADTriggerMode.
        */
        
        if (function == ADAcquire) {
            /* TODO: adstatus == ADStatusIdle has to be checked */
            if (value )
            {
                status = acquireImage();
                if(status == asynSuccess){
                    setIntegerParam(ADAcquire, 1);
                    callParamCallbacks();
                }
            }

            // Stop acquisition
            /* TODO: adstatus != ADStatusIdle has to be checked */
            if (!value)
            {   
                /* In button trigger mode , acquisition should no be stoped, that will 
                affect the WaitForSingleObject. So only status is updated to STOP. once 
                the trigger mode is changed from button trigger mode, the actual implementation 
                of acquireStop() will be done.
                */
                int triggerMode;
                getIntegerParam(ADTriggerMode, &triggerMode);
                if(triggerMode == PR_BUTTON_TRIGGER){
                    status = asynSuccess;
                }
                else{
                    status = acquireStop();
                }
                
                if(status == asynSuccess){
                    setIntegerParam(ADAcquire, 0);
                    callParamCallbacks();
                }
            }

        }   /* set  value for default parameters */
        else if(function == ADBinX){
            addToParamQue(function,value);
        }
        else if(function == ADBinY){
            addToParamQue(function,value);
        }
        else if(function == ADReadStatus){
            addToParamQue(function,value);
        }
        else if(function == ADTriggerMode){
            addToParamQue(function,value);
        }
        else if(function == PR_SoftTrigger){
            addToParamQue(function,value);
        }
        else if (function == PR_UpdateStatus)
        {
            addToParamQue(function,value);
        }
        else if (function == PR_UpdateTemperature)
        {
            addToParamQue(function,value);
        }
        else if (function == PR_ToggleTec)
        {
            addToParamQue(function,value);
        }
        else if (function == PR_ToggleGain)
        {
            addToParamQue(function,value);
        }
        else if (function == PR_ToggleFpgaComms)
        {
            addToParamQue(function,value);
        }
        else if(function == ADMinX){
            addToParamQue(function,value);
        }
        else if(function == ADMinY){
            addToParamQue(function,value);
        }
        else if(function == ADSizeX){
            addToParamQue(function,value);
        }
        else if(function == ADSizeY){
            addToParamQue(function,value);
        }
        else if(function == PR_TriggerPolarity){
            addToParamQue(function,value);
        }
        else{
            status = ADDriver::writeInt32(pasynUser, value);
        }

        return asynSuccess;
    }

    asynStatus Pixci::writeFloat64(asynUser *pasynUser, epicsFloat64 value){
        int function = pasynUser->reason;
        asynStatus status = asynSuccess;
        static const char *functionName = "writeFloat64";

        if(function == ADAcquirePeriod || function == ADAcquireTime){
            addToParamQue(function,value);
        }
        else if (function == ADTemperature)
        {
            addToParamQue(function,value);
        }
        return asynSuccess;
    }

    void Pixci::addToParamQue(epicsInt32 function, epicsInt32 value){
        epicsFloat64 functionAndVal[2];
        functionAndVal[0] = function;
        functionAndVal[1] = value;
        /*sending buffer data to the que */
        paramMsgQue->send(functionAndVal,16);
    }

    void Pixci::addToParamQue(epicsInt32 function, epicsFloat64 value){
        epicsFloat64 functionAndVal[2];
        functionAndVal[0] = function;
        functionAndVal[1] = value;
        /*sending buffer data to the que */
        paramMsgQue->send(functionAndVal,16);
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

    asynStatus Pixci::readSerialRegister(char Register, char *val){
        char inputMsg[20];
        int inSize;
        char first_bufout[] = {0x53, 0xE0, 0x01, 0xFF, 0x50};
        char last_bufout[] = {0x53, 0xE1, 0x01, 0x50};
        first_bufout[3] = Register;

        /*writing to serial connection*/
        inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 20);
        inSize = writeReadSerial(UNIT, last_bufout, sizeof(last_bufout), inputMsg, 20);

        *val = inputMsg[0];
        if(inputMsg[1] == SUCCESS_MESSAGE){
            return asynSuccess;
        }
        return asynError;
    }

    asynStatus Pixci::readSerialRegister(char Register1, char Register2, char *val)
    {
        char inputMsg[20];
        int inSize;
        char first_bufout[] = {0x53, 0xE0, 0x02, 0xFF, 0xFF, 0x50};
        char last_bufout[] = {0x53, 0xE1, 0x01, 0x50};
        first_bufout[3] = Register1;
        first_bufout[4] = Register2;

        /*writing to serial connection*/
        inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 20);
        inSize = writeReadSerial(UNIT, last_bufout, sizeof(last_bufout), inputMsg, 20);

        *val = inputMsg[0];
        if(inputMsg[1] == SUCCESS_MESSAGE){
            return asynSuccess;
        }
        return asynError;
    }

    asynStatus Pixci::setTriggerMode(int mode){
        char reg = 0xD4;
        char hexval;
        switch(mode){
            case PR_INTERNAL_ITR:
                hexval = 0x04;
                break;
            case PR_INTERNAL_FFR:
                hexval = 0X06;
                break;
            case PR_EXTERNAL:
                int triggerPolarity;
                getIntegerParam(PR_TriggerPolarity, &triggerPolarity);
                if(triggerPolarity == PR_EXT_FALLING_EDGE){
                    hexval = 0xc0;
                }
                else{
                    hexval = 0x40;
                }
                
                break;
            case PR_BUTTON_TRIGGER:
                hexval = 0x00;
                break;
            default:
                asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid trigger mode value %d",mode);
                return asynError;
                break;
        }
        return Pixci::writeSerialRegister(UNIT, reg, hexval);
    }

    //PV Updating Functions
    void Pixci::updateADTemperatureActual(bool callBackFlag)
    {
        setDoubleParam(ADTemperatureActual, getTemperatureActual()); // setting the Actual Temperature PV       
        if(callBackFlag)
            callParamCallbacks();
    }

    void Pixci::updateTemperaturePcb(bool callBackFlag)
    {
        setDoubleParam(PR_TemperaturePcb, getTemperaturePcb()); // setting the PCB Temperature PV       
        if(callBackFlag)
            callParamCallbacks();
    }

    asynStatus Pixci::updateManufacturersData(bool callBackFlag)
    {
        
        char inputMsg[20];
        int inSize;
        char first_bufout[] = {0x53, 0xAE, 0x05, 0x01, 0x00, 0x00, 0x02, 0x00, 0x50};
        char last_bufout[] = {0x53, 0xAF, 0x12, 0x50};

        INT16 serialNumber = 0;
        string buildDate = "";
        char buildCode[5];
        INT16 adcCountZeroDegree=0;
        INT16 adcCountFortyDegree=0;
        INT16 dacCountZeroDegree=0;
        INT16 dacCountFortyDegree=0;


        toggleFpgaComms(true);

        /*writing to serial connection*/
        inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 20);
        inSize = writeReadSerial(UNIT, last_bufout, sizeof(last_bufout), inputMsg, 20);
        
        toggleFpgaComms(false);


        if(inputMsg[18] == SUCCESS_MESSAGE){
            
            
            serialNumber += (INT16)(unsigned char)inputMsg[0];
            serialNumber += (INT16)(unsigned char)(inputMsg[1])<<8;
            setStringParam(ADSerialNumber, to_string(serialNumber));
            
            buildDate = to_string((INT16)(unsigned char)inputMsg[2])+"/"+to_string((INT16)(unsigned char)inputMsg[3])+"/"+to_string((INT16)(unsigned char)inputMsg[4]);
            setStringParam(PR_BuildDate, buildDate);

            buildCode[0]=inputMsg[5];
            buildCode[1]=inputMsg[6];
            buildCode[2]=inputMsg[7];
            buildCode[3]=inputMsg[8];
            buildCode[4]=inputMsg[9];

            adcCountZeroDegree += (INT16)(unsigned char)inputMsg[10];
            adcCountZeroDegree += (INT16)(unsigned char)(inputMsg[11])<<8;
            setIntegerParam(PR_ADCCalibrationZeroDegree, adcCountZeroDegree);

            adcCountFortyDegree += (INT16)(unsigned char)inputMsg[12];
            adcCountFortyDegree += (INT16)(unsigned char)(inputMsg[13])<<8;
            setIntegerParam(PR_ADCCalibrationFortyDegree, adcCountFortyDegree);

            dacCountZeroDegree += (INT16)(unsigned char)inputMsg[14];
            dacCountZeroDegree += (INT16)(unsigned char)(inputMsg[15])<<8;
             setIntegerParam(PR_DACCalibrationZeroDegree, dacCountZeroDegree);

            dacCountFortyDegree += (INT16)(unsigned char)inputMsg[16];
            dacCountFortyDegree += (INT16)(unsigned char)(inputMsg[17])<<8;
            setIntegerParam(PR_DACCalibrationFortyDegree, dacCountFortyDegree);

            ADC_M = 40.0f/(adcCountFortyDegree-adcCountZeroDegree);
            ADC_C = 40.0f-(ADC_M*adcCountFortyDegree);

            DAC_M = 40.0f/(dacCountFortyDegree-dacCountZeroDegree);
            DAC_C = 40.0f-(DAC_M*dacCountZeroDegree);
          
            if(callBackFlag)
                callParamCallbacks();
            return asynSuccess;
        }


        return asynError;


    }

    void Pixci::updateStatus(bool updateManufacturersDataFlag)
    {
        //TODO: Get the manufacturer data and also refactor the AdcCountToCentigrade function
        if(updateManufacturersDataFlag)
            updateManufacturersData(); // TODO: Implement Error Message

        updateADTemperatureActual();
        updateTemperaturePcb(true);
    }

    asynStatus Pixci::updateIntialPVs(){
        epicsInt32 sizeX = pxd_imageXdim();
        epicsInt32 sizeY = pxd_imageYdim();
        int status = asynSuccess;
        status |=  setIntegerParam(ADMaxSizeX, sizeX);
        status |=  setIntegerParam(ADMaxSizeY, sizeY);
        status |=  setIntegerParam(ADSizeX, sizeX);
        status |=  setIntegerParam(ADSizeY, sizeY);

        status |= callParamCallbacks();
        return (asynStatus) status;
    }

    asynStatus Pixci::setRoiSizeX(int RoisizeX){
        char cval[2] ={0,0};
        cval[0] = (char)((RoisizeX & 0x0F00) >> 8 );
        cval[1] = (char)((RoisizeX & 0x00FF) );

        writeSerialRegister(UNIT, 0xB4, cval[0]);
        return writeSerialRegister(UNIT, 0xB5, cval[1]);
    }

    asynStatus Pixci::setRoiSizeY(int RoisizeY){
        char cval[2] ={0,0};
        cval[0] = (char)((RoisizeY & 0x0F00) >> 8 );
        cval[1] = (char)((RoisizeY & 0x00FF) );

        writeSerialRegister(UNIT, 0xB8, cval[0]);
        return writeSerialRegister(UNIT, 0xB9, cval[1]);
    }

    asynStatus Pixci::setRoiOffsetX(int RoiOffsetX){
        char cval[2] ={0,0};
        cval[0] = (char)((RoiOffsetX & 0x0F00) >> 8 );
        cval[1] = (char)((RoiOffsetX & 0x00FF) );

        writeSerialRegister(UNIT, 0xB6, cval[0]);
        return writeSerialRegister(UNIT, 0xB7, cval[1]);
    }

    asynStatus Pixci::setRoiOffsetY(int RoiOffsetY){
        char cval[2] ={0,0};
        cval[0] = (char)((RoiOffsetY & 0x0F00) >> 8 );
        cval[1] = (char)((RoiOffsetY & 0x00FF) );

        writeSerialRegister(UNIT, 0xBA, cval[0]);
        return writeSerialRegister(UNIT, 0xBB, cval[1]);
    }

    int Pixci::getRoiSizeX()
    {
        char cval[2] ={0,0};

        readSerialRegister(0XB4, &cval[1]);
        readSerialRegister(0XB5, &cval[0]);

        INT16 ival = 0;
        ival += (INT16)(unsigned char)cval[0];
        ival += (INT16)(unsigned char)(cval[1] & 0x0F)<<8;

        return ival;
    }

    int Pixci::getRoiSizeY()
    {
        char cval[2] ={0,0};

        readSerialRegister(0XB8, &cval[1]);
        readSerialRegister(0XB9, &cval[0]);

        INT16 ival = 0;
        ival += (INT16)(unsigned char)cval[0];
        ival += (INT16)(unsigned char)(cval[1] & 0x0F)<<8;

        return ival;
    }

    int Pixci::getRoiOffsetX()
    {
        char cval[2] ={0,0};

        readSerialRegister(0XB6, &cval[1]);
        readSerialRegister(0XB7, &cval[0]);

        INT16 ival = 0;
        ival += (INT16)(unsigned char)cval[0];
        ival += (INT16)(unsigned char)(cval[1] & 0x0F)<<8;

        return ival;
    }

    int Pixci::getRoiOffsetY()
    {
        char cval[2] ={0,0};

        readSerialRegister(0XBA, &cval[1]);
        readSerialRegister(0XBB, &cval[0]);

        INT16 ival = 0;
        ival += (INT16)(unsigned char)cval[0];
        ival += (INT16)(unsigned char)(cval[1] & 0x0F)<<8;

        return ival;
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
