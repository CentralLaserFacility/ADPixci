/**
 * @brief This is a driver for EPIX inc. PIXCI camera link frame grabbers
 *  This driver uses the PIXCI ® XCLIB Programming Library.
 * 
 * @copyright Copyright (c) 2025, UKRI STFC Central Laser Facility
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 * 
 *  1. Redistributions of source code must retain the above copyright notice, this
 *  list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the names of its
 *  contributors may be used to endorse or promote products derived from
 *  this software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "pixci.h"

/* Pixci headers
 source: http://www.epixinc.com/products/xclib.htm
 XCLW64 .dll and .lib files should be included for windows-64 OS
 XCLIBNT .dll and .lib files should be inlcuded for win32 OS
 xclib_x86_64 .so and .a files should be included for linux_x86_64 OS
 xclib_i386 .so and .a files should be included for linux_x86 OS
*/
extern "C"
{
#include "xcliball.h"
}

/* For windows */
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#include <windows.h>
#endif

/* Epics headers */
#include <iocsh.h>
#include <epicsEvent.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsExit.h>
#include <epicsExport.h>
#include <epicsMessageQueue.h>

#include <algorithm>
#include <string>
#include <cstdio>

// Set video resolution and video offset.
// Set capture resolution to same.
//
// There are more video format fields than are set here.
// We are assuming that the current format hasn't been purposely
// butchered so as to make our job harder.
//
// If setting an AOI with camera commands, this function's
// hoffset & voffset should be 0. In other words: of the
// pixels output by the camera, do not skip any lines or columns.
// In contrast, if the camera is outputting full resolution and
// this function is used to capture less than the full resolution,
// then hoffset & voffset would allow positioning the capture AOI
// within the larger camera space.
//
//
#if !defined(PIXCI_LITE)
_cDcl(_dllpxlib, _cfunfcc, epicsInt32)
    pxd_setVideoResolution(
        epicsInt32 unitmap,     // usual
        epicsInt32 xdim,        // pixels per line
        epicsInt32 ydim,        // pixels per column (per field)
        epicsInt32 hoffset,     // video hoffset
        epicsInt32 voffset      // video voffset
    )
{
    epicsInt32 r = 0, r1 = 0;
    epicsInt32 u = 0, umap = 0, multiple = 0;
    struct xclibs *xc;

#if USEINTERNALAPI
    if (liblog_active)
        liblog_aasrbz("pxd_setVideoResolution", "", "D*",
                      &unitmap, static_cast<size_t>(sizeof(unitmap)),
                      &xdim, static_cast<size_t>(sizeof(xdim)),
                      &ydim, static_cast<size_t>(sizeof(ydim)),
                      &hoffset, static_cast<size_t>(sizeof(hoffset)),
                      &voffset, static_cast<size_t>(sizeof(voffset)),
                      NULL);
#endif
    if (!(xc = pxd_xclibEscape(0, 0, 0)))
        return (PXERNOTOPEN);
#if 1
    {
        pxvidstate_s *vidstatep = NULL;
        // We might have compiled for multiple formats, but it may not be active.
        if (xc->pxlib.getAllocState(&xc->pxlib, 0, PXMODE_DIGI + 1, &vidstatep) >= 0)
        {
            multiple = 1;
            xc->pxlib.freeStateCopy(&xc->pxlib, 0, PXMODE_DIGI + 1, &vidstatep);
        }
        for (u = 0, umap = unitmap; u < PXMAX_UNITS && umap; umap >>= 1, u++)
        {
            if (!(umap & 1))
                continue;
            if ((r1 = xc->pxlib.getAllocState(&xc->pxlib, 0, multiple ? PXMODE_DIGI + u : PXMODE_DIGI, &vidstatep)) < 0)
            {
                r = min(r, r1);
                continue;
            }
            vidstatep->vidformat->xviddim[PXLHCM_MAX] = xdim;
            vidstatep->vidformat->xdatdim[PXLHCM_MAX] = xdim;
            vidstatep->vidformat->yviddim[PXLHCM_MAX] = ydim;
            vidstatep->vidformat->ydatdim[PXLHCM_MAX] = ydim;
            vidstatep->vidformat->xvidoffset[PXLHCM_MAX] = xdim;
            vidstatep->vidformat->yvidoffset[PXLHCM_MAX] = ydim;
            vidstatep->vidformat->xviddim[PXLHCM_MOD] = 0;
            vidstatep->vidformat->xdatdim[PXLHCM_MOD] = 0;
            vidstatep->vidformat->yviddim[PXLHCM_MOD] = 0;
            vidstatep->vidformat->ydatdim[PXLHCM_MOD] = 0;
            vidstatep->vidformat->is.hoffset = hoffset;
            vidstatep->vidformat->is.voffset = voffset;
            //
            vidstatep->vidres->x.setmaxdatsamples = 1;
            vidstatep->vidres->x.setmaxvidsamples = 1;
            vidstatep->vidres->y.setmaxdatsamples = 1;
            vidstatep->vidres->y.setmaxvidsamples = 1;
            vidstatep->vidres->setmaxdatfields = 1;
            vidstatep->vidres->setmaxdatphylds = 1;
            r1 = xc->pxlib.defineState(&xc->pxlib, 0, multiple ? PXMODE_DIGI + u : PXMODE_DIGI, vidstatep);
            r = min(r, r1);
            xc->pxlib.freeStateCopy(&xc->pxlib, 0, multiple ? PXMODE_DIGI + u : PXMODE_DIGI, &vidstatep);
            if (!multiple)
                break;
        }
        r1 = pxd_xclibEscaped(unitmap, 0, 0);
        r = min(r, r1);
        return (r);
    }
#else
    {
#if USEINTERNALAPI  // using internal API
        xclib_DeclareVidStateStructs2(vidstate, pxdstatep->devinfo[0].s.model);
        xclib_InitVidStateStructs2(vidstate, pxdstatep->devinfo[0].s.model);
#else
        xclib_DeclareVidStateStructs2(vidstate, pxd_infoModel(unitmap));
        xclib_InitVidStateStructs2(vidstate, pxd_infoModel(unitmap));
#endif

#if 1 | MULTIPLEFORMATS
        // We might have compiled for multiple formats, but it may not be active.
        if (xc->pxlib.getState(&xc->pxlib, 0, PXMODE_DIGI + 1, &vidstate) >= 0)
            multiple = 1;
        for (u = 0, umap = unitmap; u < PXMAX_UNITS && umap; umap >>= 1, u++)
        {
            if (!(umap & 1))
                continue;
            xc->pxlib.getState(&xc->pxlib, 0, multiple ? PXMODE_DIGI + u : PXMODE_DIGI, &vidstate);
            vidstate.vidformat->xviddim[PXLHCM_MAX] = xdim;
            vidstate.vidformat->xdatdim[PXLHCM_MAX] = xdim;
            vidstate.vidformat->yviddim[PXLHCM_MAX] = ydim;
            vidstate.vidformat->ydatdim[PXLHCM_MAX] = ydim;
            vidstate.vidformat->xvidoffset[PXLHCM_MAX] = xdim;
            vidstate.vidformat->yvidoffset[PXLHCM_MAX] = ydim;
            vidstate.vidformat->xviddim[PXLHCM_MOD] = 0;
            vidstate.vidformat->xdatdim[PXLHCM_MOD] = 0;
            vidstate.vidformat->yviddim[PXLHCM_MOD] = 0;
            vidstate.vidformat->ydatdim[PXLHCM_MOD] = 0;
            vidstate.vidformat->is.hoffset = hoffset;
            vidstate.vidformat->is.voffset = voffset;
            //
            vidstate.vidres->x.setmaxdatsamples = 1;
            vidstate.vidres->x.setmaxvidsamples = 1;
            vidstate.vidres->y.setmaxdatsamples = 1;
            vidstate.vidres->y.setmaxvidsamples = 1;
            vidstate.vidres->setmaxdatfields = 1;
            vidstate.vidres->setmaxdatphylds = 1;
            r1 = xc->pxlib.defineState(&xc->pxlib, 0, multiple ? PXMODE_DIGI + u : PXMODE_DIGI, &vidstate);
            r = min(r, r1);
            if (!multiple)
                break;
        }
        r1 = pxd_xclibEscaped(unitmap, 0, 0);
        r = min(r, r1);
        return (r);
#else
        ?
#endif
    }
#endif
}

#endif  // !defined(PIXCI_LITE)

/*
 * @brief C Function prototypes to tie in with EPICS
 * run acquire task
 * @param drvPvt
 */
static void acquireTaskC(void *drvPvt);

// static void serialTaskC(void *drvPvt);
static void paramTaskC(void *drvPvt);

/*
 * @brief Configuration command for pixci driver; creates a new pixci object.
 * @param See the pixci.h
 */
extern "C" epicsInt32 pixciConfig(const char *portName, epicsInt32 maxBuffers, size_t maxMemory, epicsInt32 priority,
    epicsInt32 stackSize, const char *cameraModel, const char *formatFile)
{
    new Pixci(portName, maxBuffers, maxMemory, priority, stackSize, cameraModel, formatFile);
    return (asynSuccess);
}



/*
 * @brief Default constructor to create a new Pixci::Pixci object
 */
Pixci::Pixci(const char *portName, epicsInt32 maxBuffers, size_t maxMemory, epicsInt32 priority, epicsInt32 stackSize,
    const char *cameraModel, const char *formatFile)
    : ADDriver(portName, 1, 1, maxBuffers, maxMemory, 0, 0, ASYN_CANBLOCK, 1, priority, stackSize)
{
    epicsInt32 connectionStatusCode = 0;
    epicsInt32 serialConnection = 0;

    createParam(SoftTriggerParamString, asynParamInt32, &PR_SoftTrigger);
    createParam(TriggerPolarityParamString, asynParamInt32, &PR_TriggerPolarity);
    createParam(UpdateTemperatureString, asynParamInt32, &PR_UpdateTemperature);
    createParam(UpdateStatusString, asynParamInt32, &PR_UpdateStatus);
    createParam(BuildDateString, asynParamOctet, &PR_BuildDate);

    setStringParam(ADModel, cameraModel);

    /* pxd_PIXCIopen(driverparms, formatname, formatfile) return 0 if connection is successfull
     * returns value <0 if any error occured
     * pxd_mesgErrorCode(int code) will return description of the error occured
     */
    connectionStatusCode = pxd_PIXCIopen(DRIVERPARMS, nullptr, formatFile);

    if (connectionStatusCode < PIXCI_NO_ERROR)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                  "%s: Cannot OPEN camera: %s.",
                  driverName, pxd_mesgErrorCode(connectionStatusCode));
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
                  "%s Camera connected;",
                  driverName);
        serialConnection = pxd_serialConfigure(UNIT, RESERVED, BAUDRATE, 8, 0, 1, RESERVED, RESERVED, RESERVED);
        if (serialConnection < PIXCI_NO_ERROR)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                      "%s: Cannot make serial connection: %s.",
                      driverName, pxd_mesgErrorCode(connectionStatusCode));
        }
    }

    /* Any thread waiting upon the event will be notified whenever a field has been captured by pxd_goSnap,
    pxd_goLive, pxd_goLivePair and pxd_goLiveSeq*/
    g_hEvent = pxd_eventCapturedFieldCreate(UNIT);
    asynStatus status = asynSuccess;
    setStatIfHigher(&status, setStringParam(ADManufacturer, "Raptor Photonics"));

    paramMsgQue = new epicsMessageQueue(PARAM_MESSAGE_QUE_SIZE, PARAM_MESSAGE_SIZE);

    /* Create the thread that does data acquisition */
    if (connectionStatusCode >= PIXCI_NO_ERROR && serialConnection >= PIXCI_NO_ERROR)
    {
        epicsThreadCreate("acquireTask", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium),
                        (EPICSTHREADFUNC)acquireTaskC, this);
        epicsThreadCreate("paramTask", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium),
                        (EPICSTHREADFUNC)paramTaskC, this);
    }

    // Updating all the PVs related to the status of device and the manufacturers data
    setStatIfHigher(&status, updateStatus());
    setStatIfHigher(&status, updateIntialPVs());
    if (status == asynError)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to initialize the detector\n");
        return;
    }
}

Pixci::~Pixci()
{
    /* Closing connection to frame grabber */
    epicsInt32 disconnectStatusCode = PIXCI_NO_ERROR;
    /*pxd_PIXCIclose() disconnect the driver from the device.
     * return 0 if disconnect successfull, return integer <0 if error occured
     * pxd_mesgErrorCode(int code) will return description of the error occured
     */
    disconnectStatusCode = pxd_PIXCIclose();
    if (disconnectStatusCode < PIXCI_NO_ERROR)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                  "%s: disconnect camera error: %s .",
                  driverName, pxd_mesgErrorCode(disconnectStatusCode));
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
                  "%s: camera disconnected;",
                  driverName);
    }
}

asynStatus Pixci::setupAquisition()
{
    epicsInt32 binX = 0;
    epicsInt32 binY = 0;
    epicsInt32 RoiSizeX = 0;
    epicsInt32 RoiSizeY = 0;
    epicsInt32 sizeX = pxd_imageXdim();
    epicsInt32 sizeY = pxd_imageYdim();
    callParamCallbacks();
    getIntegerParam(ADBinX, &binX);
    if (binX <= 0)
    {
        binX = 1;
        setIntegerParam(ADBinX, binX);
    }
    getIntegerParam(ADBinY, &binY);
    if (binY <= 0)
    {
        binY = 1;
        setIntegerParam(ADBinY, binY);
    }

    getIntegerParam(ADSizeX, &RoiSizeX);
    getIntegerParam(ADSizeY, &RoiSizeY);
    // setIntegerParam(ADSizeX, sizeX);
    // setIntegerParam(ADSizeY, sizeY);

    setIntegerParam(NDArraySizeX, RoiSizeX / binX);
    setIntegerParam(NDArraySizeY, RoiSizeY / binY);

    callParamCallbacks();

    return asynSuccess;
}

asynStatus Pixci::acquireImage()
{
    /* TODO: implement all acquisition method like trigger, ringbuffer etc */
    static const char *functionName = "acquireImage";
    pxbuffer_t buffer = 1L;     // Image frame buffer
    /* live capture the image into frame buffer */
    epicsInt32 error = pxd_goLive(UNIT, buffer);
    if (error < PIXCI_NO_ERROR)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                  "acquisition error: %s : %s", functionName, pxd_mesgErrorCode(error));
        return asynError;
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
                  "acquisition initiated ");
        return asynSuccess;
    }
}

asynStatus Pixci::acquireStop()
{
    static const char *functionName = "acquireStop";
    /* stop the live capturing */
    epicsInt32 error = pxd_goUnLive(UNIT);
    if (error < PIXCI_NO_ERROR)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                  "live couldn't stop: %s : %s", functionName, pxd_mesgErrorCode(error));
        return asynError;
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
                  "live stopped \n");
        return asynSuccess;
    }
}

static void acquireTaskC(void *drvPvt)
{
    Pixci *pPvt = reinterpret_cast<Pixci *>(drvPvt);
    pPvt->acquireTask();
}

/**
 * @brief Acquistion task for live image capturing.
 * Event will be notified whenever a field has been captured by pxd_goSnapor, pxd_goLive.
 */
void Pixci::acquireTask()
{
    /* TODO: need to implement in a seperate file */
    NDArray *pImage = this->pArrays[0];
    pxbuffer_t buf = 1L;
    NDDataType_t dataType = NDUInt16;
    epicsInt32 sizeX = 0;
    epicsInt32 sizeY = 0;
    epicsInt32 binX = 0;
    epicsInt32 binY = 0;
    size_t dims[2] = {};
    epicsTimeStamp currentTime = {};
    epicsInt32 numImagesCounter = 0;
    epicsInt32 imageCounter = 0;
    epicsInt32 arrayCallbacks = 0;
    setupAquisition();

    for (;;)
    {
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

        if (arrayCallbacks)
        {
            lock();
            /* Allocate NDArray */
            pImage = this->pNDArrayPool->alloc(2, dims, dataType, 0, NULL);
            /* Pixel values from an image frame buffer and area of interest are copied into buffer
            pxd_readuchar(unit, framebuf, ulxc, ulyc, lrx, lry, membuf, cnt, colorspace)*/
            pxd_readushort(UNIT, buf, 0, 0, sizeX, sizeY, reinterpret_cast<ushort *>(pImage->pData),
                        dims[0] * dims[1] * sizeof(epicsUInt16), "GRAY");
            // pxd_readushort (unitmap, framebuf, ulx, uly, lrx, lry, membuf, cnt, colorspace);

            /* uniqueId and timeStamp must be implemented for standard ADDriver. */
            pImage->uniqueId = imageCounter;
            epicsTimeGetCurrent(&currentTime);
            pImage->timeStamp = currentTime.secPastEpoch + currentTime.nsec / 1.e9;
            updateTimeStamp(&pImage->epicsTS);
            unlock();
            getAttributes(pImage->pAttributeList);

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

        setIntegerParam(NDArraySize, static_cast<epicsInt32>(dims[0] * dims[1] * sizeof(NDUInt16)));
        setIntegerParam(NDArrayCounter, imageCounter);
        setIntegerParam(ADNumImagesCounter, numImagesCounter);
        callParamCallbacks();
    }
}

static void paramTaskC(void *drvPvt)
{
    Pixci *pPvt = reinterpret_cast<Pixci *>(drvPvt);
    pPvt->paramTask();
}

void Pixci::handleParamTask(epicsInt32 function, epicsFloat64 d_val, epicsInt32 i_val, epicsBoolean b_val)
{
    asynStatus status = asynSuccess;
    if (function == ADBinX)
    {
        epicsInt32 sizeX = 0;
        epicsInt32 sizeY = 0;
        epicsInt32 binY = 0;
        getIntegerParam(ADSizeX, &sizeX);
        getIntegerParam(ADSizeY, &sizeY);
        getIntegerParam(ADBinY, &binY);
        status = Pixci::setBin(i_val, BIN_AXIS_X);
        if (status == asynSuccess)
        {
            epicsInt32 acquire = 0;
            epicsInt32 triggerMode;
            epicsInt32 triggerPolarity = PR_EXT_RISING_EDGE;
            setIntegerParam(ADBinX, i_val);     // Updating the binX value.
            getIntegerParam(ADAcquire, &acquire);   // Getting the ADAcquire value.
            getIntegerParam(ADTriggerMode, &triggerMode);
            getIntegerParam(PR_TriggerPolarity, &triggerPolarity);
            if (acquire == 1)
            {
                acquireStop();
            }
            callParamCallbacks();
            changeVideoFormatConfig();  // Video settings have to be loaded respective of binning value.
            pxd_setVideoResolution(UNIT, sizeX / i_val, sizeY / binY, 0, 0);
            setIntegerParam(PR_TriggerPolarity, triggerPolarity);
            setTriggerMode(triggerMode);
            setupAquisition();
            if (acquire == 1)
            {
                acquireImage();     // starting acquisition if acquisition was running before.
            }
        }
    }
    else if (function == ADBinY)
    {
        epicsInt32 sizeX = 0;
        epicsInt32 sizeY = 0;
        epicsInt32 binX = 0;
        getIntegerParam(ADSizeX, &sizeX);
        getIntegerParam(ADSizeY, &sizeY);
        getIntegerParam(ADBinX, &binX);
        status = Pixci::setBin(i_val, BIN_AXIS_Y);
        if (status == asynSuccess)
        {
            epicsInt32 acquire = 0;
            epicsInt32 triggerMode = ADTriggerInternal;
            epicsInt32 triggerPolarity = PR_EXT_RISING_EDGE;
            setIntegerParam(ADBinY, i_val);
            getIntegerParam(ADAcquire, &acquire);
            getIntegerParam(ADTriggerMode, &triggerMode);
            getIntegerParam(PR_TriggerPolarity, &triggerPolarity);
            if (acquire == 1)
            {
                acquireStop();
            }
            callParamCallbacks();
            changeVideoFormatConfig();
            pxd_setVideoResolution(UNIT, sizeX / binX, sizeY / i_val, 0, 0);
            setIntegerParam(PR_TriggerPolarity, triggerPolarity);
            setTriggerMode(triggerMode);
            setupAquisition();
            if (acquire == 1)
            {
                acquireImage();
            }
        }
    }
    else if (function == ADTriggerMode)
    {
        epicsInt32 acquisitionStatus = asynSuccess;
        epicsInt32 previousTriggerMode = PR_INTERNAL_ITR;
        status = setTriggerMode(i_val);
        if (status == asynSuccess)
        {
            if (i_val == PR_BUTTON_TRIGGER)
            {
                /* In button triggermode, for WaitForSingleObject function to be notified pxd_goLive should be
                called. For that acquireImage() function is called.
                */
                status = acquireImage();
            }
            else
            {
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
            setIntegerParam(ADTriggerMode, i_val);
        }
    }
    else if (function == PR_SoftTrigger)
    {
        status = sendSoftTrigger();
    }
    else if (function == PR_UpdateStatus)
    {
        status = updateStatus();
    }
    else if (function == PR_UpdateTemperature)
    {
        updateADTemperatureActual();
        updateTemperaturePcb(epicsTrue);
    }
    else if (function == ADAcquirePeriod)
    {
        if (d_val != 0.0)
        {
            status = setFrameRate(1 / d_val);
            if (status == asynSuccess)
            {
                epicsFloat64 readBackFrameRate = getFrameRate();
                if (readBackFrameRate > 0)
                {
                    setDoubleParam(ADAcquirePeriod, (1 / readBackFrameRate));
                }
            }
        }
    }
    else if (function == ADTemperature)
    {
        status = setTecTemperature(d_val);
        if (status == asynSuccess)
        {
            epicsFloat64 tecTemperature = getTecTemperature();
            setDoubleParam(ADTemperature, tecTemperature);
        }
    }
    else if (function == ADAcquireTime)
    {
        if (d_val != 0.0)
        {
            status = setExposure(d_val);
            if (status == asynSuccess)
            {
                epicsFloat64 readBackAcquireTime = getExposure();
                if (readBackAcquireTime > 0)
                {
                    setDoubleParam(ADAcquireTime, readBackAcquireTime);
                }
            }
        }
    }
    else if (function == ADMinX)
    {
        epicsInt32 maxSizeX = 0;
        epicsInt32 sizeX = 0;
        epicsInt32 sizeY = 0;
        epicsInt32 binX = 0;
        epicsInt32 binY = 0;
        epicsInt32 acquire = 0;
        getIntegerParam(ADMaxSizeX, &maxSizeX);
        getIntegerParam(ADSizeX, &sizeX);
        getIntegerParam(ADSizeY, &sizeY);
        getIntegerParam(ADBinX, &binX);
        getIntegerParam(ADBinY, &binY);
        getIntegerParam(ADAcquire, &acquire);
        epicsInt32 minX = (i_val > maxSizeX) ? maxSizeX : i_val;
        if ((sizeX + minX) > maxSizeX)
        {
            sizeX = maxSizeX - minX;
        }
        acquireStop();

        status = setRoiSizeX(sizeX);
        status = setRoiOffsetX(minX);
        sizeX = getRoiSizeX();
        pxd_setVideoResolution(UNIT, sizeX / binX, sizeY / binY, 0, 0);
        setIntegerParam(ADSizeX, sizeX);
        setIntegerParam(ADMinX, getRoiOffsetX());
        setupAquisition();

        if (acquire == 1)
        {
            acquireImage();
        }
    }
    else if (function == ADMinY)
    {
        epicsInt32 maxSizeY = 0;
        epicsInt32 sizeX = 0;
        epicsInt32 sizeY = 0;
        epicsInt32 binX = 0;
        epicsInt32 binY = 0;
        epicsInt32 acquire = 0;
        getIntegerParam(ADMaxSizeY, &maxSizeY);
        getIntegerParam(ADSizeX, &sizeX);
        getIntegerParam(ADSizeY, &sizeY);
        getIntegerParam(ADBinX, &binX);
        getIntegerParam(ADBinY, &binY);
        getIntegerParam(ADAcquire, &acquire);
        epicsInt32 minY = (i_val > maxSizeY) ? maxSizeY : i_val;
        if ((sizeY + minY) > maxSizeY)
        {
            sizeY = maxSizeY - minY;
        }
        acquireStop();

        // status = setRoiSizeX(sizeX);
        // status = setRoiOffsetX(minX);
        status = setRoiSizeY(sizeY);
        status = setRoiOffsetY(minY);
        sizeY = getRoiSizeY();
        pxd_setVideoResolution(UNIT, sizeX / binX, sizeY / binY, 0, 0);
        setIntegerParam(ADSizeY, sizeY);
        setIntegerParam(ADMinY, getRoiOffsetY());

        setupAquisition();
        if (acquire == 1)
        {
            acquireImage();
        }
    }
    else if (function == ADSizeX)
    {
        epicsInt32 maxSizeX = 0;
        epicsInt32 minX = 0;
        epicsInt32 sizeY = 0;
        epicsInt32 binX = 0;
        epicsInt32 binY = 0;
        epicsInt32 acquire = 0;
        getIntegerParam(ADMaxSizeX, &maxSizeX);
        getIntegerParam(ADMinX, &minX);
        getIntegerParam(ADSizeY, &sizeY);
        getIntegerParam(ADBinX, &binX);
        getIntegerParam(ADBinY, &binY);
        getIntegerParam(ADAcquire, &acquire);
        epicsInt32 sizeX = (i_val > maxSizeX) ? maxSizeX : i_val;

        if ((sizeX + minX) > maxSizeX)
        {
            minX = maxSizeX - sizeX;
        }
        acquireStop();

        status = setRoiSizeX(sizeX);
        status = setRoiOffsetX(minX);
        sizeX = getRoiSizeX();
        // status = setRoiSizeY(sizeY);
        // status = setRoiOffsetY(minY);
        pxd_setVideoResolution(UNIT, sizeX / binX, sizeY / binY, 0, 0);
        setIntegerParam(ADMinX, getRoiOffsetX());
        setIntegerParam(ADSizeX, sizeX);
        setupAquisition();
        if (acquire == 1)
        {
            acquireImage();
        }
    }
    else if (function == ADSizeY)
    {
        epicsInt32 maxSizeY = 0;
        epicsInt32 minY = 0;
        epicsInt32 sizeX = 0;
        epicsInt32 binX = 0;
        epicsInt32 binY = 0;
        epicsInt32 acquire = 0;
        getIntegerParam(ADMaxSizeY, &maxSizeY);
        getIntegerParam(ADMinY, &minY);
        getIntegerParam(ADSizeX, &sizeX);
        getIntegerParam(ADBinX, &binX);
        getIntegerParam(ADBinY, &binY);
        getIntegerParam(ADAcquire, &acquire);
        epicsInt32 sizeY = (i_val > maxSizeY) ? maxSizeY : i_val;

        if ((sizeY + minY) > maxSizeY)
        {
            minY = maxSizeY - sizeY;
        }
        acquireStop();

        // status == setRoiSizeX(sizeX);
        // status = setRoiOffsetX(minX);
        status = setRoiSizeY(sizeY);
        status = setRoiOffsetY(minY);
        sizeY = getRoiSizeY();
        pxd_setVideoResolution(UNIT, sizeX / binX, sizeY / binY, 0, 0);
        setIntegerParam(ADMinY, getRoiOffsetY());
        setIntegerParam(ADSizeY, sizeY);
        setupAquisition();
        if (acquire == 1)
        {
            acquireImage();
        }
    }
    else if (function == PR_TriggerPolarity)
    {
        epicsInt32 triggerMode = PR_INTERNAL_ITR;
        if (i_val == PR_EXT_RISING_EDGE)
        {
            setIntegerParam(PR_TriggerPolarity, PR_EXT_RISING_EDGE);
        }
        else if (i_val == PR_EXT_FALLING_EDGE)
        {
            setIntegerParam(PR_TriggerPolarity, PR_EXT_FALLING_EDGE);
        }
        callParamCallbacks();
        getIntegerParam(ADTriggerMode, &triggerMode);
        if (triggerMode == PR_EXTERNAL)
        {
            setTriggerMode(PR_EXTERNAL);
        }
    }
    else if (function == ADShutterOpenDelay)
    {
        status = setShutterOpenDelay(d_val);
        if (status == asynSuccess)
        {
            epicsFloat64 openDelay = getShutterOpenDelay();
            setDoubleParam(ADShutterOpenDelay, openDelay);
        }
        else {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set shutter open delay\n");
        }
    }
    else if (function == ADShutterCloseDelay)
    {
        status = setShutterCloseDelay(d_val);
        if (status == asynSuccess)
        {
            epicsFloat64 closeDelay = getShutterCloseDelay();
            setDoubleParam(ADShutterCloseDelay, closeDelay);
        }
        else {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set shutter close delay\n");
        }
    }
}

void Pixci::paramTask()
{
    epicsFloat64 functionAndVal[2] = {};
    epicsInt32 function = 0;
    epicsFloat64 d_val = 0.0;
    epicsInt32 i_val = 0;
    epicsBoolean b_val = epicsFalse;
    asynStatus status = asynSuccess;

    for (;;)
    {
        paramMsgQue->receive(functionAndVal, PARAM_MESSAGE_SIZE);
        function = static_cast<epicsInt32>(functionAndVal[0]);
        b_val = static_cast<epicsBoolean>(functionAndVal[1]);
        i_val = static_cast<epicsInt32>(functionAndVal[1]);
        d_val = functionAndVal[1];
        this->handleParamTask(function, d_val, i_val, b_val);
        callParamCallbacks();
    }
}

void Pixci::changeVideoFormatConfig()
{
    epicsInt32 binX = 0;
    epicsInt32 binY = 0;
    std::string cameraModel = "";
    getIntegerParam(ADBinX, &binX);
    getIntegerParam(ADBinY, &binY);
    getStringParam(ADModel, cameraModel);
    if (cameraModel == DETECTOR_1K)
    {
        if (binX == PR_BIN_1 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid binning value");
        }
    }
    else if (cameraModel == DETECTOR_2K)
    {
        if (binX == PR_BIN_1 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid binning value");
        }
    }
    else {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "camera model not supported");
    }
}

epicsUInt64 Pixci::int8ToUInt64(epicsInt8 *cval)
{
    epicsUInt64 lval = 0;
    lval += (epicsUInt64)(epicsUInt8)cval[4];
    lval += ((epicsUInt64)(epicsUInt8)cval[3]) << 8;
    lval += ((epicsUInt64)(epicsUInt8)cval[2]) << 16;
    lval += ((epicsUInt64)(epicsUInt8)cval[1]) << 24;
    lval += ((epicsUInt64)(epicsUInt8)cval[0]) << 32;
    return lval;
}

void Pixci::uInt64ToInt8(epicsUInt64 lval, epicsInt8 *cval)
{
    cval[0] = (epicsInt8)((lval & 0xFF00000000) >> 32);
    cval[1] = (epicsInt8)((lval & 0x00FF000000) >> 24);
    cval[2] = (epicsInt8)((lval & 0x0000FF0000) >> 16);
    cval[3] = (epicsInt8)((lval & 0x000000FF00) >> 8);
    cval[4] = (epicsInt8)((lval & 0x00000000FF));
}

epicsInt32 Pixci::writeReadSerial(epicsInt32 unit, char *serialOut, epicsInt32 msgOutSize, char *serialIn,
    epicsInt32 serialInBufferSize)
{
    epicsInt32 count = 0;
    epicsInt8 bufOut[50] = {};
    epicsInt8 chkSum = 0;
    epicsInt32 outMsgwait = 0;
    epicsInt32 inMsgwait = 0;
    epicsInt32 inMsgwaitFlag = 0;

    /* checking if any message packer left to read, and clear the buffer by reading it */
    if (pxd_serialRead(unit, RESERVED, nullptr, 0) > 0)
    {
        count = pxd_serialRead(unit, 0, serialIn, serialInBufferSize);
    }

    /* wait if any message is in the send que*/
    while (pxd_serialWrite(unit, RESERVED, nullptr, 0) < msgOutSize && outMsgwait < 50)
    {
        outMsgwait++;
        Sleep(10);
    }
    outMsgwait = 0;

    /* creating checksum to send as the last character of the message */
    for (int i = 0; i < msgOutSize; i++)
    {
        bufOut[i] = serialOut[i];
        chkSum ^= serialOut[i];
    }
    serialOut[msgOutSize] = chkSum;

    /* sending the seriaOut message */
    count = pxd_serialWrite(unit, RESERVED, serialOut, msgOutSize + 1);
    Sleep(130);

    if (count < PIXCI_NO_ERROR)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                  "%s: Cannot serial write: %s.",
                  driverName, pxd_mesgErrorCode(count));
        return count;
    }
    else
    {
        /*waiting for the reply */
        inMsgwaitFlag = pxd_serialRead(unit, 0, nullptr, 0);
        while (inMsgwaitFlag < 1 && inMsgwait < 20)
        {
            inMsgwait++;
            inMsgwaitFlag = pxd_serialRead(unit, 0, nullptr, 0);
            Sleep(10);
        }
        inMsgwait = 0;
        /* read the message and message count */
        count = pxd_serialRead(UNIT, RESERVED, serialIn, serialInBufferSize);
    }

    return count;
}

asynStatus Pixci::setBin(epicsInt32 val, epicsBoolean coordinate)
{
    epicsInt8 hexval = 0;
    epicsInt8 reg = (coordinate == BIN_AXIS_X) ? X_BIN_BYTE : Y_BIN_BYTE;
    std::string cameraModel = "";

    /* Assigning corresponding Hex value to send*/
    switch (val)
    {
    case PR_BIN_1:
        hexval = 0x00;
        break;
    case PR_BIN_2:
        hexval = 0x01;
        break;
    case PR_BIN_4:
        hexval = 0x03;
        break;
    case PR_BIN_8:
        hexval = 0x07;
        break;
    case PR_BIN_16:
        hexval = 0x0F;
        break;
    case PR_BIN_32:
        hexval = 0x1F;
        break;
    case PR_BIN_FVB:
        getStringParam(ADModel, cameraModel);
        if (coordinate == BIN_AXIS_Y && cameraModel == DETECTOR_2K) {
            hexval = static_cast<epicsInt8>(0x80);
            break;
        }
    default:
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid binning value %d", val);
        return asynError;
        break;
    }

    return Pixci::writeSerialRegister(UNIT, reg, hexval);
}

asynStatus Pixci::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    epicsInt32 function = pasynUser->reason;
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

    if (function == ADAcquire)
    {
        /* TODO: adstatus == ADStatusIdle has to be checked */
        if (value)
        {
            status = acquireImage();
            if (status == asynSuccess)
            {
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
            epicsInt32 triggerMode = PR_INTERNAL_ITR;
            getIntegerParam(ADTriggerMode, &triggerMode);
            if (triggerMode == PR_BUTTON_TRIGGER)
            {
                status = asynSuccess;
            }
            else
            {
                status = acquireStop();
            }

            if (status == asynSuccess)
            {
                setIntegerParam(ADAcquire, 0);
                callParamCallbacks();
            }
        }
    } /* set  value for default parameters */
    else if (function == ADBinX)
    {
        addToParamQue(function, value);
    }
    else if (function == ADBinY)
    {
        addToParamQue(function, value);
    }
    else if (function == ADReadStatus)
    {
        addToParamQue(function, value);
    }
    else if (function == ADTriggerMode)
    {
        addToParamQue(function, value);
    }
    else if (function == PR_SoftTrigger)
    {
        addToParamQue(function, value);
    }
    else if (function == PR_UpdateStatus)
    {
        addToParamQue(function, value);
    }
    else if (function == PR_UpdateTemperature)
    {
        addToParamQue(function, value);
    }
    else if (function == ADMinX)
    {
        addToParamQue(function, value);
    }
    else if (function == ADMinY)
    {
        addToParamQue(function, value);
    }
    else if (function == ADSizeX)
    {
        addToParamQue(function, value);
    }
    else if (function == ADSizeY)
    {
        addToParamQue(function, value);
    }
    else if (function == PR_TriggerPolarity)
    {
        addToParamQue(function, value);
    }
    else
    {
        status = ADDriver::writeInt32(pasynUser, value);
    }

    return status;
}

asynStatus Pixci::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    epicsInt32 function = pasynUser->reason;
    static const char *functionName = "writeFloat64";

    if (function == ADGain || function == ADAcquirePeriod || function == ADAcquireTime || function == ADTemperature ||
        function == ADShutterOpenDelay || function == ADShutterCloseDelay)
    {
        addToParamQue(function, value);
        return asynSuccess;
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: unknown function(%d) with value: %f\n",
            functionName, function, value);
        return asynError;
    }
}

void Pixci::addToParamQue(epicsInt32 function, epicsInt32 value)
{
    epicsFloat64 functionAndVal[2] = {static_cast<epicsFloat64>(function), static_cast<epicsFloat64>(value)};
    /*sending buffer data to the queue */
    paramMsgQue->send(functionAndVal, PARAM_MESSAGE_SIZE);
}

void Pixci::addToParamQue(epicsInt32 function, epicsFloat64 value)
{
    epicsFloat64 functionAndVal[2] = {static_cast<epicsFloat64>(function), value};
    /*sending buffer data to the queue */
    paramMsgQue->send(functionAndVal, PARAM_MESSAGE_SIZE);
}

asynStatus Pixci::setTriggerMode(epicsInt32 mode)
{
    epicsInt8 hexval = 0;
    switch (mode)
    {
    case PR_INTERNAL_ITR:
        hexval = INTERNAL_ITR_BYTE;     // 00000100
        break;
    case PR_INTERNAL_FFR:
        hexval = INTERNAL_FFR_BYTE;     // 00000110
        break;
    case PR_EXTERNAL:
        {   // brackets so that trigger polarity goes out of scope after this case
            epicsInt32 triggerPolarity = PR_EXT_RISING_EDGE;
            getIntegerParam(PR_TriggerPolarity, &triggerPolarity);
            hexval = (triggerPolarity == PR_EXT_FALLING_EDGE) ? EXTERNAL_FALLING_EDGE_BYTE : EXTERNAL_RISING_EDGE_BYTE;
        }
        break;
    case PR_BUTTON_TRIGGER:
        hexval = CLEAR_TRIGGER_MODE_BYTE;   // 00000000
        break;
    default:
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid trigger mode value %d", mode);
        return asynError;
        break;
    }
    return Pixci::writeSerialRegister(UNIT, TRIGGER_MODE_BYTE, hexval);
}

asynStatus Pixci::sendSoftTrigger() {
    /* if trigger mode is button trigger then, do the soft trigger else print error */
    epicsInt32 triggerMode = PR_INTERNAL_ITR;
    getIntegerParam(ADTriggerMode, &triggerMode);
    if (triggerMode == PR_BUTTON_TRIGGER)
    {
        return Pixci::writeSerialRegister(UNIT, TRIGGER_MODE_BYTE, SOFT_TRIGGER_BYTE);
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Button Trigger mode is not selected");
        return asynError;
    }
}

// PV Updating Functions
asynStatus Pixci::updateADTemperatureActual(epicsBoolean callBackFlag)
{
    asynStatus status = asynSuccess;
    setStatIfHigher(&status, setDoubleParam(ADTemperatureActual, getTemperatureActual()));    // setting the Actual Temperature PV
    if (callBackFlag)
        setStatIfHigher(&status, callParamCallbacks());
    return status;
}

asynStatus Pixci::updateStatus()
{
    asynStatus status = asynSuccess;
    // TODO: Get the manufacturer data and also refactor the AdcCountToCentigrade function
    setStatIfHigher(&status, updateADTemperatureActual());
    return status;
}

asynStatus Pixci::updateIntialPVs()
{
    epicsInt32 sizeX = pxd_imageXdim();
    epicsInt32 sizeY = pxd_imageYdim();
    asynStatus status = asynSuccess;

    setStatIfHigher(&status, setIntegerParam(ADMaxSizeX, sizeX));
    setStatIfHigher(&status, setIntegerParam(ADMaxSizeY, sizeY));
    setStatIfHigher(&status, setIntegerParam(ADSizeX, sizeX));
    setStatIfHigher(&status, setIntegerParam(ADSizeY, sizeY));

    setStatIfHigher(&status, callParamCallbacks());
    return status;
}

void Pixci::report(FILE *fp, epicsInt32 details)
{
    fprintf(fp, "Raptor detector %s\n", this->portName);
    if (details > 0)
    {
        epicsInt32 nx = 0;
        epicsInt32 ny = 0;
        epicsInt32 dataType = 0;
        getIntegerParam(ADSizeX, &nx);
        getIntegerParam(ADSizeY, &ny);
        getIntegerParam(NDDataType, &dataType);
        fprintf(fp, "  NX, NY:            %d  %d\n", nx, ny);
        fprintf(fp, "  Data type:         %d\n", dataType);
    }
    /* Invoke the base class method */
    ADDriver::report(fp, details);
}

/* Code for iocsh registration */

/* pixciConfig parameters from st.cmd */
static const iocshArg pixciConfigArg0 = {"portName", iocshArgString};
static const iocshArg pixciConfigArg1 = {"maxBuffers", iocshArgInt};
static const iocshArg pixciConfigArg2 = {"maxMemory", iocshArgInt};
static const iocshArg pixciConfigArg3 = {"priority", iocshArgInt};
static const iocshArg pixciConfigArg4 = {"stackSize", iocshArgInt};
static const iocshArg pixciConfigArg5 = {"cameraModel", iocshArgString};
static const iocshArg pixciConfigArg6 = {"formatFile", iocshArgString};
static const iocshArg *const pixciConfigArgs[] = {&pixciConfigArg0,
                                                  &pixciConfigArg1,
                                                  &pixciConfigArg2,
                                                  &pixciConfigArg3,
                                                  &pixciConfigArg4,
                                                  &pixciConfigArg5,
                                                  &pixciConfigArg6};
static const iocshFuncDef configpixci = {"pixciConfig", 7, pixciConfigArgs};
static void configpixciCallFunc(const iocshArgBuf *args)
{
    pixciConfig(args[0].sval, args[1].ival, args[2].ival, args[3].ival,
                args[4].ival, args[5].sval, args[6].sval);
}

static void pixciRegister(void)
{
    iocshRegister(&configpixci, configpixciCallFunc);
}

extern "C"
{
    epicsExportRegistrar(pixciRegister);
}
