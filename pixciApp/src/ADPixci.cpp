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

#include "ADPixci.h"

/* ADPixci headers
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
#include <epicsEvent.h>
#include <epicsTime.h>
#include <epicsString.h>
#include <epicsExit.h>
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
_cDcl(_dllpxlib, _cfunfcc, epicsInt32) pxd_setVideoResolution(
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
        pxvidstate_s *vidstatep = nullptr;
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

epicsUInt64 int8ToUInt64(epicsInt8 *cval)
{
    epicsUInt64 lval = 0;
    lval += (epicsUInt64)(epicsUInt8)cval[4];
    lval += ((epicsUInt64)(epicsUInt8)cval[3]) << 8;
    lval += ((epicsUInt64)(epicsUInt8)cval[2]) << 16;
    lval += ((epicsUInt64)(epicsUInt8)cval[1]) << 24;
    lval += ((epicsUInt64)(epicsUInt8)cval[0]) << 32;
    return lval;
}

void uInt64ToInt8(epicsUInt64 lval, epicsInt8 *cval)
{
    cval[0] = (epicsInt8)((lval & 0xFF00000000) >> 32);
    cval[1] = (epicsInt8)((lval & 0x00FF000000) >> 24);
    cval[2] = (epicsInt8)((lval & 0x0000FF0000) >> 16);
    cval[3] = (epicsInt8)((lval & 0x000000FF00) >> 8);
    cval[4] = (epicsInt8)((lval & 0x00000000FF));
}


/**
 * @brief Default constructor to create a new ADPixci::ADPixci object
 */
ADPixci::ADPixci(const char *portName, epicsInt32 maxBuffers, size_t maxMemory, epicsInt32 priority,
    epicsInt32 stackSize, const char *cameraModel, const char *formatFile)
    : ADDriver(portName, 1, 1, maxBuffers, maxMemory, 0, 0, ASYN_CANBLOCK, 1, priority, stackSize)
{
    epicsInt32 connectionStatusCode = 0;
    epicsInt32 serialConnection = 0;

    createParam(SoftTriggerParamString, asynParamInt32, &PR_SoftTrigger);
    createParam(TriggerPolarityParamString, asynParamInt32, &PR_TriggerPolarity);
    createParam(UpdateInfoString, asynParamInt32, &PR_UpdateInfo);
    createParam(UpdateTemperatureString, asynParamInt32, &PR_UpdateTemperature);
    createParam(BuildDateString, asynParamOctet, &PR_BuildDate);

    setStringParam(ADModel, cameraModel);

    /* pxd_PIXCIopen(driverparms, formatname, formatfile) return 0 if connection is successfull
     * returns value <0 if any error occured
     * pxd_mesgErrorCode(int code) will return description of the error occured
     */
    connectionStatusCode = pxd_PIXCIopen(DRIVERPARMS, nullptr, formatFile);

    if (connectionStatusCode < PIXCI_NO_ERROR)
    {
        std::string errMsg = pxd_mesgErrorCode(connectionStatusCode);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Cannot OPEN camera: %s\n", driverName, errMsg);
        setIntegerParam(ADStatus, ADStatusDisconnected);
        setStringParam(ADStatusMessage, errMsg);
        this->deviceIsReachable = epicsFalse;
        throw std::runtime_error("Failed to open camera: " + errMsg);
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s Camera connected\n", driverName);
        setIntegerParam(ADStatus, ADStatusIdle);
        serialConnection = pxd_serialConfigure(UNIT, RESERVED, BAUDRATE, 8, 0, 1, RESERVED, RESERVED, RESERVED);
        if (serialConnection < PIXCI_NO_ERROR)
        {
            std::string errMsg = pxd_mesgErrorCode(serialConnection);
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Cannot make serial connection: %s\n", driverName, 
                errMsg);
            setIntegerParam(ADStatus, ADStatusError);
            setStringParam(ADStatusMessage, errMsg);
            this->deviceIsReachable = epicsFalse;
            throw std::runtime_error("Failed to make serial connection: " + errMsg);
        }
    }

    /* Any thread waiting upon the event will be notified whenever a field has been captured by pxd_goSnap,
    pxd_goLive, pxd_goLivePair and pxd_goLiveSeq*/
    g_hEvent = pxd_eventCapturedFieldCreate(UNIT);

    paramMsgQue = new epicsMessageQueue(PARAM_MESSAGE_QUE_SIZE, PARAM_MESSAGE_SIZE);
}

ADPixci::~ADPixci()
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
        std::string errMsg = pxd_mesgErrorCode(disconnectStatusCode);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Error on camera disconnect: %s\n", driverName, errMsg);
        setIntegerParam(ADStatus, ADStatusError);
        setStringParam(ADStatusMessage, errMsg);
        throw std::runtime_error("Failed to disconnect the camera: " + errMsg);
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s: Camera disconnected\n", driverName);
        setIntegerParam(ADStatus, ADStatusDisconnected);
        setStringParam(ADStatusMessage, "Camera disconnected.");
    }
}

asynStatus ADPixci::setupAquisition()
{
    asynStatus status = asynSuccess;
    epicsInt32 binX = 0;
    epicsInt32 binY = 0;
    epicsInt32 RoiSizeX = 0;
    epicsInt32 RoiSizeY = 0;
    epicsInt32 sizeX = pxd_imageXdim();
    epicsInt32 sizeY = pxd_imageYdim();
    setStatIfHigher(&status, callParamCallbacks());
    setStatIfHigher(&status, getIntegerParam(ADBinX, &binX));
    if (binX <= 0)
    {
        binX = 1;
        setStatIfHigher(&status, setIntegerParam(ADBinX, binX));
    }
    setStatIfHigher(&status, getIntegerParam(ADBinY, &binY));
    if (binY <= 0)
    {
        binY = 1;
        setStatIfHigher(&status, setIntegerParam(ADBinY, binY));
    }

    setStatIfHigher(&status, getIntegerParam(ADSizeX, &RoiSizeX));
    setStatIfHigher(&status, getIntegerParam(ADSizeY, &RoiSizeY));

    setStatIfHigher(&status, setIntegerParam(NDArraySizeX, RoiSizeX / binX));
    setStatIfHigher(&status, setIntegerParam(NDArraySizeY, RoiSizeY / binY));

    setStatIfHigher(&status, callParamCallbacks());

    return status;
}

asynStatus ADPixci::aquireStart()
{
    /* TODO: implement all acquisition method like trigger, ringbuffer etc */
    pxbuffer_t buffer = 1L;     // Image frame buffer
    /* live capture the image into frame buffer */
    epicsInt32 error = pxd_goLive(UNIT, buffer);
    if (error < PIXCI_NO_ERROR)
    {
        std::string errMsg = pxd_mesgErrorCode(error);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Acquisition start error : %s\n", errMsg);
        setIntegerParam(ADStatus, ADStatusError);
        setStringParam(ADStatusMessage, errMsg);
        return asynError;
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "Acquisition started");
        setIntegerParam(ADStatus, ADStatusAcquire);
        setStringParam(ADStatusMessage, "Acquisition started\n");
        return asynSuccess;
    }
}

asynStatus ADPixci::acquireStop()
{
    /* stop the live capturing */
    epicsInt32 error = pxd_goUnLive(UNIT);
    if (error < PIXCI_NO_ERROR)
    {
        std::string errMsg = pxd_mesgErrorCode(error);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Acquisition stop error: : %s\n", errMsg);
        setIntegerParam(ADStatus, ADStatusError);
        setStringParam(ADStatusMessage, errMsg);
        return asynError;
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "Acquisition stopped\n");
        setIntegerParam(ADStatus, ADStatusIdle);
        setStringParam(ADStatusMessage, "Acquisition stopped\n");
        return asynSuccess;
    }
}

/**
 * @brief Acquistion task for live image capturing.
 * Event will be notified whenever a field has been captured by pxd_goSnapor, pxd_goLive.
 */
void ADPixci::acquireTask()
{
    NDArray *pImage = this->pArrays[0];
    pxbuffer_t buf = 1L;
    NDDataType_t dataType = NDUInt16;
    epicsInt32 sizeX = 0;
    epicsInt32 sizeY = 0;
    size_t dims[2] = {};
    epicsTimeStamp currentTime = {};
    epicsInt32 numImagesCounter = 0;
    epicsInt32 imageCounter = 0;
    epicsInt32 arrayCallbacks = 0;
    asynStatus acquisitionSetupStatus = setupAquisition();
    if (acquisitionSetupStatus != asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Acquisition setup failed\n");
        throw std::runtime_error("Acquisition setup failed");
    }
    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "Acquisition setup successful\n");
    for (;;)
    {
        /* waiting for event to be triggered */
        /* TODO: seperate waiting task for linux */
        WaitForSingleObject(g_hEvent, INFINITE);

        getIntegerParam(NDArraySizeX, &sizeX);
        getIntegerParam(NDArraySizeY, &sizeY);
        getIntegerParam(NDArrayCallbacks, &arrayCallbacks);

        dims[0] = sizeX;
        dims[1] = sizeY;

        if (arrayCallbacks)
        {
            lock();
            /* Allocate NDArray */
            pImage = this->pNDArrayPool->alloc(2, dims, dataType, 0, nullptr);
            setIntegerParam(ADStatus, ADStatusReadout);
            /* Pixel values from an image frame buffer and area of interest are copied into buffer */
            epicsInt32 err = pxd_readushort(UNIT, buf, 0, 0, sizeX, sizeY, reinterpret_cast<ushort *>(pImage->pData),
                        dims[0] * dims[1] * sizeof(epicsUInt16), "GRAY");
            if (err < PIXCI_NO_ERROR)
            {
                std::string errMsg = pxd_mesgErrorCode(err);
                asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading image from detector: %s\n", errMsg);
                setIntegerParam(ADStatus, ADStatusError);
                setStringParam(ADStatusMessage, "Error reading image from detector: " + errMsg);
                unlock();
                callParamCallbacks();
                continue;
            }
            /* uniqueId and timeStamp must be implemented for standard ADDriver. */
            pImage->uniqueId = imageCounter;
            epicsTimeGetCurrent(&currentTime);
            pImage->timeStamp = currentTime.secPastEpoch + currentTime.nsec / 1.e9;
            updateTimeStamp(&pImage->epicsTS);
            unlock();
            getAttributes(pImage->pAttributeList);

            setIntegerParam(ADStatus, ADStatusSaving);
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
        setIntegerParam(ADStatus, ADStatusIdle);
        callParamCallbacks();
    }
}

void ADPixci::handleParamTask(epicsInt32 parameter, epicsFloat64 d_val, epicsInt32 i_val, epicsBoolean b_val)
{
    asynStatus status = asynSuccess;

    if (parameter == PR_SoftTrigger)
    {
        status = this->sendSoftTrigger();
    }
    else if (parameter == PR_UpdateInfo)
    {
        status = this->updateInfo();
    }
    else if (parameter == PR_UpdateTemperature)
    {
        status = this->updateTemperatureActual();
    }
    else if (parameter == ADAcquirePeriod)
    {
        if (d_val != 0.0)
        {
            status = this->setFrameRate(1 / d_val);
            if (status == asynSuccess)
            {
                epicsFloat64 readBackFrameRate = this->getFrameRate();
                if (readBackFrameRate > 0)
                {
                    setDoubleParam(ADAcquirePeriod, (1 / readBackFrameRate));
                }
            }
        }
    }
    else if (parameter == ADTemperature)
    {
        status = this->setCoolingSetPoint(d_val);
        if (status == asynSuccess)
        {
            epicsFloat64 coolingSetPoint = this->getCoolingSetPoint();
            setDoubleParam(ADTemperature, coolingSetPoint);
        }
    }
    else if (parameter == ADAcquireTime)
    {
        if (d_val != 0.0)
        {
            status = this->setExposure(d_val);
            if (status == asynSuccess)
            {
                epicsFloat64 readBackAcquireTime = this->getExposure();
                if (readBackAcquireTime > 0)
                {
                    setDoubleParam(ADAcquireTime, readBackAcquireTime);
                }
            }
        }
    }
    else if (parameter == ADBinX)
    {
        epicsInt32 binY = 0;
        epicsInt32 sizeX = 0;
        epicsInt32 sizeY = 0;
        getIntegerParam(ADBinY, &binY);
        getIntegerParam(ADSizeX, &sizeX);
        getIntegerParam(ADSizeY, &sizeY);
        status = this->setBin(i_val, BIN_AXIS_X);
        if (status == asynSuccess)
        {
            epicsInt32 acquire = 0;
            epicsInt32 triggerMode;
            epicsInt32 triggerPolarity = PRExtRisingEdge;
            setIntegerParam(ADBinX, i_val);     // Updating the binX value.
            getIntegerParam(ADAcquire, &acquire);   // Getting the ADAcquire value.
            getIntegerParam(ADTriggerMode, &triggerMode);
            getIntegerParam(PR_TriggerPolarity, &triggerPolarity);
            if (acquire == 1)
            {
                acquireStop();
            }
            callParamCallbacks();
            // Video settings have to be loaded respective of binning value.
            this->changeVideoFormatConfig(i_val, binY, sizeX, sizeY);
            setIntegerParam(PR_TriggerPolarity, triggerPolarity);
            this->setTriggerMode(triggerMode);
            setupAquisition();
            if (acquire == 1)
            {
                aquireStart();     // starting acquisition if acquisition was running before.
            }
        }
    }
    else if (parameter == ADBinY)
    {
        epicsInt32 binX = 0;
        epicsInt32 sizeX = 0;
        epicsInt32 sizeY = 0;
        getIntegerParam(ADBinX, &binX);
        getIntegerParam(ADSizeX, &sizeX);
        getIntegerParam(ADSizeY, &sizeY);
        status = this->setBin(i_val, BIN_AXIS_Y);
        if (status == asynSuccess)
        {
            epicsInt32 acquire = 0;
            epicsInt32 triggerMode = ADTriggerInternal;
            epicsInt32 triggerPolarity = PRExtRisingEdge;
            setIntegerParam(ADBinY, i_val);
            getIntegerParam(ADAcquire, &acquire);
            getIntegerParam(ADTriggerMode, &triggerMode);
            getIntegerParam(PR_TriggerPolarity, &triggerPolarity);
            if (acquire == 1)
            {
                acquireStop();
            }
            callParamCallbacks();
            this->changeVideoFormatConfig(binX, i_val, sizeX, sizeY);
            setIntegerParam(PR_TriggerPolarity, triggerPolarity);
            this->setTriggerMode(triggerMode);
            setupAquisition();
            if (acquire == 1)
            {
                aquireStart();
            }
        }
    }
    else if (parameter == ADMinX)
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

        status = this->setRoiSizeX(sizeX);
        status = this->setRoiOffsetX(minX);
        sizeX = this->getRoiSizeX();
        this->changeVideoFormatConfig(binX, binY, sizeX, sizeY);
        setIntegerParam(ADSizeX, sizeX);
        setIntegerParam(ADMinX, this->getRoiOffsetX());
        setupAquisition();

        if (acquire == 1)
        {
            aquireStart();
        }
    }
    else if (parameter == ADMinY)
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

        status = this->setRoiSizeY(sizeY);
        status = this->setRoiOffsetY(minY);
        sizeY = this->getRoiSizeY();
        this->changeVideoFormatConfig(binX, binY, sizeX, sizeY);
        setIntegerParam(ADSizeY, sizeY);
        setIntegerParam(ADMinY, this->getRoiOffsetY());

        setupAquisition();
        if (acquire == 1)
        {
            aquireStart();
        }
    }
    else if (parameter == ADSizeX)
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

        status = this->setRoiSizeX(sizeX);
        status = this->setRoiOffsetX(minX);
        sizeX = this->getRoiSizeX();
        // status = setRoiSizeY(sizeY);
        // status = setRoiOffsetY(minY);
        this->changeVideoFormatConfig(binX, binY, sizeX, sizeY);
        setIntegerParam(ADMinX, this->getRoiOffsetX());
        setIntegerParam(ADSizeX, sizeX);
        setupAquisition();
        if (acquire == 1)
        {
            aquireStart();
        }
    }
    else if (parameter == ADSizeY)
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
        status = this->setRoiSizeY(sizeY);
        status = this->setRoiOffsetY(minY);
        sizeY = this->getRoiSizeY();
        this->changeVideoFormatConfig(binX, binY, sizeX, sizeY);
        setIntegerParam(ADMinY, this->getRoiOffsetY());
        setIntegerParam(ADSizeY, sizeY);
        setupAquisition();
        if (acquire == 1)
        {
            aquireStart();
        }
    }
    else if (parameter == ADShutterOpenDelay)
    {
        status = this->setShutterOpenDelay(d_val);
        if (status == asynSuccess)
        {
            epicsFloat64 openDelay = this->getShutterOpenDelay();
            setDoubleParam(ADShutterOpenDelay, openDelay);
        }
        else {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set shutter open delay\n");
        }
    }
    else if (parameter == ADShutterCloseDelay)
    {
        status = this->setShutterCloseDelay(d_val);
        if (status == asynSuccess)
        {
            epicsFloat64 closeDelay = this->getShutterCloseDelay();
            setDoubleParam(ADShutterCloseDelay, closeDelay);
        }
        else {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set shutter close delay\n");
        }
    }
}

void ADPixci::paramTask()
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


epicsInt32 ADPixci::writeReadSerial(epicsInt32 unit, char *serialOut, epicsInt32 msgOutSize, char *serialIn,
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

asynStatus ADPixci::writeInt32(asynUser *pasynUser, epicsInt32 value)
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
    if (function == ADReadStatus || function == ADTriggerMode || function == PR_TriggerPolarity ||
        function == PR_SoftTrigger || function == PR_UpdateInfo || function == PR_UpdateTemperature ||
        function == ADBinX || function == ADBinY || function == ADMinX || function == ADMinY ||
        function == ADSizeX || function == ADSizeY )
    {
        addToParamQue(function, value);
    }
    else
    {
        status = ADDriver::writeInt32(pasynUser, value);
    }

    return status;
}

asynStatus ADPixci::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
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

void ADPixci::addToParamQue(epicsInt32 function, epicsInt32 value)
{
    epicsFloat64 functionAndVal[2] = {static_cast<epicsFloat64>(function), static_cast<epicsFloat64>(value)};
    /*sending buffer data to the queue */
    paramMsgQue->send(functionAndVal, PARAM_MESSAGE_SIZE);
}

void ADPixci::addToParamQue(epicsInt32 function, epicsFloat64 value)
{
    epicsFloat64 functionAndVal[2] = {static_cast<epicsFloat64>(function), value};
    /*sending buffer data to the queue */
    paramMsgQue->send(functionAndVal, PARAM_MESSAGE_SIZE);
}

// PV Updating Functions

asynStatus ADPixci::updateInitialPVs()
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

void ADPixci::changeVideoFormatConfig(epicsInt32 binX, epicsInt32 binY, epicsInt32 sizeX, epicsInt32 sizeY){
    pxd_setVideoResolution(UNIT, sizeX / binX, sizeY / binY, 0, 0);
}

void ADPixci::report(FILE *fp, epicsInt32 details)
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
