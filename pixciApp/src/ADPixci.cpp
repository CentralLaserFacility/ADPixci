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

// For Windows
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#include <windows.h>
#endif

// Epics headers
#include <epicsEvent.h>
#include <epicsTime.h>
#include <epicsString.h>
#include <epicsExit.h>
#include <epicsMessageQueue.h>

// Standard C++ headers
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
// hoffset & voffset should be 0. In other words: if the
// pixels output by the camera, do not skip any lines or columns.
// In contrast, if the camera is outputting full resolution and
// this function is used to capture less than the full resolution,
// then hoffset & voffset would allow positioning the capture AOI
// within the larger camera space.
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

    if (!(xc = pxd_xclibEscape(0, 0, 0)))
        return (PXERNOTOPEN);

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
    epicsInt32 cameraConnectionStatus = PIXCI_NO_ERROR;
    epicsInt32 serialConnectionStatus = PIXCI_NO_ERROR;
    asynStatus status = asynSuccess;
    this->driverName = "ADPixci";

    setStringParam(ADModel, cameraModel);
    setIntegerParam(ADStatus, ADStatusInitializing);
    // Initialize driver parameters
    setStatIfHigher(&status, createParam(SoftTriggerParamString, asynParamInt32, &PR_SoftTrigger));
    setStatIfHigher(&status, createParam(TriggerPolarityParamString, asynParamInt32, &PR_TriggerPolarity));
    setStatIfHigher(&status, createParam(UpdateInfoString, asynParamInt32, &PR_UpdateInfo));
    setStatIfHigher(&status, createParam(UpdateTemperatureString, asynParamInt32, &PR_UpdateTemperature));
    setStatIfHigher(&status, createParam(BuildDateString, asynParamOctet, &PR_BuildDate));
    setStatIfHigher(&status, createParam(ReadoutModeParamString, asynParamInt32, &PR_ReadoutMode));
    setStatIfHigher(&status, createParam(PixelReadoutClockParamString, asynParamInt32, &PR_PixelReadoutClock));

    if (status > asynSuccess)
    {   // Parameter initialization failed
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Failed to create parameters\n", driverName);
        setIntegerParam(ADStatus, ADStatusError);
        setStringParam(ADStatusMessage, "Cannot create parameters");
        throw std::runtime_error("Failed to create parameters");
    }
    // Open the connection to the camera
    cameraConnectionStatus = pxd_PIXCIopen(DRIVERPARMS, nullptr, formatFile);
    if (cameraConnectionStatus < PIXCI_NO_ERROR)
    {   // Failed to connect to the camera
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Cannot OPEN camera: %s\n", driverName,
            pxd_mesgErrorCode(cameraConnectionStatus));
        setIntegerParam(ADStatus, ADStatusDisconnected);
        setStringParam(ADStatusMessage, pxd_mesgErrorCode(cameraConnectionStatus));
        this->deviceIsReachable = epicsFalse;
        throw std::runtime_error(std::string("Failed to open camera: ") + pxd_mesgErrorCode(cameraConnectionStatus));
    }
    else
    {   // Connected to the camera
        asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s Camera connected\n", driverName);
        // Make the serial connection to the camera
        serialConnectionStatus = pxd_serialConfigure(UNIT, RESERVED, BAUDRATE, 8, 0, 1, RESERVED, RESERVED, RESERVED);
        if (serialConnectionStatus < PIXCI_NO_ERROR)
        {   // Failed to make the serial connection
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Cannot make serial connection: %s\n", driverName,
                pxd_mesgErrorCode(serialConnectionStatus));
            setIntegerParam(ADStatus, ADStatusError);
            setStringParam(ADStatusMessage, pxd_mesgErrorCode(serialConnectionStatus));
            this->deviceIsReachable = epicsFalse;
            throw std::runtime_error(std::string("Failed to make serial connection: ") +
                pxd_mesgErrorCode(serialConnectionStatus));
        }
    }
    // Create an event to notify when a field has been captured by pxd_goSnap, pxd_goLive
    this->g_hEvent = pxd_eventCapturedFieldCreate(UNIT);
    // Create a message queue for parameter changes
    this->paramMsgQue = new epicsMessageQueue(PARAM_MESSAGE_QUE_SIZE, PARAM_MESSAGE_SIZE);
}

ADPixci::~ADPixci()
{
    // Destroy the event and message queue
    pxd_eventCapturedFieldClose(UNIT, this->g_hEvent);
    delete this->paramMsgQue;
    // Closing connection to frame grabber
    epicsInt32 disconnectStatusCode = PIXCI_NO_ERROR;
    disconnectStatusCode = pxd_PIXCIclose();
    if (disconnectStatusCode < PIXCI_NO_ERROR)
    {   // Error on disconnect
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Error on camera disconnect: %s\n", driverName,
            pxd_mesgErrorCode(disconnectStatusCode));
        setIntegerParam(ADStatus, ADStatusError);
        setStringParam(ADStatusMessage, pxd_mesgErrorCode(disconnectStatusCode));
    }
    // Camera successfully disconnected
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s: Camera disconnected\n", driverName);
    setIntegerParam(ADStatus, ADStatusDisconnected);
    setStringParam(ADStatusMessage, "Camera disconnected.");
}

asynStatus ADPixci::setupAcquisition()
{
    asynStatus status = asynSuccess;
    epicsInt32 binX = 0;
    epicsInt32 binY = 0;
    epicsInt32 RoiSizeX = 0;
    epicsInt32 RoiSizeY = 0;
    epicsInt32 sizeX = pxd_imageXdim();
    epicsInt32 sizeY = pxd_imageYdim();
    setStatIfHigher(&status, getIntegerParam(ADBinX, &binX));
    setStatIfHigher(&status, getIntegerParam(ADBinY, &binY));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Asyn status %d: Could not get binning parameters\n", status);
        return status;
    }
    if (binX <= 0)
    {
        binX = 1;
        setStatIfHigher(&status, setIntegerParam(ADBinX, binX));
    }
    if (binY <= 0)
    {
        binY = 1;
        setStatIfHigher(&status, setIntegerParam(ADBinY, binY));
    }
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Asyn status %d: Could not set binning parameters\n", status);
        callParamCallbacks();
        return status;
    }

    setStatIfHigher(&status, getIntegerParam(ADSizeX, &RoiSizeX));
    setStatIfHigher(&status, getIntegerParam(ADSizeY, &RoiSizeY));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Asyn status %d: Could not get ROI parameters\n", status);
        return status;
    }
    setStatIfHigher(&status, setIntegerParam(NDArraySizeX, RoiSizeX / binX));
    setStatIfHigher(&status, setIntegerParam(NDArraySizeY, RoiSizeY / binY));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
            "Asyn status %d: Could not set array size parameters\n", status);
    }
    callParamCallbacks();
    return status;
}

asynStatus ADPixci::acquireStart()
{
    pxbuffer_t buffer = 1L;  // Image frame buffer
    // start capture of the image into the frame buffer
    epicsInt32 error = pxd_goLive(UNIT, buffer);
    if (error < PIXCI_NO_ERROR)
    {   // Error starting acquisition
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Acquisition start error: %s\n", pxd_mesgErrorCode(error));
        setIntegerParam(ADStatus, ADStatusError);
        setStringParam(ADStatusMessage, pxd_mesgErrorCode(error));
        return asynError;
    }
    // acquisition successfully started
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "Acquisition started");
    setIntegerParam(ADStatus, ADStatusAcquire);
    setStringParam(ADStatusMessage, "Acquisition started\n");
    return asynSuccess;
}

asynStatus ADPixci::acquireStop()
{
    epicsInt32 error = pxd_goUnLive(UNIT);
    if (error < PIXCI_NO_ERROR)
    {   // Error stopping acquisition
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Acquisition stop error: : %s\n", pxd_mesgErrorCode(error));
        setIntegerParam(ADStatus, ADStatusError);
        setStringParam(ADStatusMessage, pxd_mesgErrorCode(error));
        return asynError;
    }
    // acquisition successfully stopped
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "Acquisition stopped\n");
    setIntegerParam(ADStatus, ADStatusIdle);
    setStringParam(ADStatusMessage, "Acquisition stopped\n");
    return asynSuccess;
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
    epicsInt32 adStatus = ADStatusIdle;
    for (;;)
    {
        // waiting for event to be triggered
        WaitForSingleObject(g_hEvent, INFINITE);
        this->lock();
        getIntegerParam(NDArraySizeX, &sizeX);
        getIntegerParam(NDArraySizeY, &sizeY);
        getIntegerParam(NDArrayCallbacks, &arrayCallbacks);
        getIntegerParam(ADStatus, &adStatus);
        if (adStatus == ADStatusInitializing)
        {
            // If the driver is still initializing, skip this iteration
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Driver is still initializing, skipping acquisition\n");
            this->unlock();
            continue;
        }
        dims[0] = sizeX;
        dims[1] = sizeY;

        if (arrayCallbacks)
        {
            // Allocate NDArray
            pImage = this->pNDArrayPool->alloc(2, dims, dataType, 0, nullptr);
            setIntegerParam(ADStatus, ADStatusReadout);
            // Pixel values from an image frame buffer and area of interest are copied into buffer
            epicsInt32 err = pxd_readushort(UNIT, buf, 0, 0, sizeX, sizeY, reinterpret_cast<ushort *>(pImage->pData),
                        dims[0] * dims[1] * sizeof(epicsUInt16), "GRAY");
            if (err < PIXCI_NO_ERROR)
            {
                asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading image from detector: %s\n",
                    pxd_mesgErrorCode(err));
                setIntegerParam(ADStatus, ADStatusError);
                setStringParam(ADStatusMessage, std::string("Error reading image from detector: ") +
                    pxd_mesgErrorCode(err));
                callParamCallbacks();
                this->unlock();
                continue;
            }
            // uniqueId and timeStamp must be implemented for standard ADDriver
            pImage->uniqueId = imageCounter;
            epicsTimeGetCurrent(&currentTime);
            pImage->timeStamp = currentTime.secPastEpoch + currentTime.nsec / 1.e9;
            updateTimeStamp(&pImage->epicsTS);
            getAttributes(pImage->pAttributeList);
            setIntegerParam(ADStatus, adStatus);
            /*Call doCallbacksGenericPointer() so that registered clients can get the values of the new arrays.
            Drivers must release their mutex by calling this->unlock() before they call doCallbacksGenericPointer(),
            or a deadlock can occur if the plugin makes a call to one of the driver functions.*/
            this->unlock();
            doCallbacksGenericPointer(pImage, NDArrayData, 0);
            this->lock();
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
        this->unlock();
    }
}

asynStatus ADPixci::reloadConfiguration(epicsInt32 param, epicsInt32 binX, epicsInt32 binY,
    epicsInt32 sizeX, epicsInt32 sizeY)
{
    asynStatus status = asynSuccess;
    epicsInt32 acquire = 0;
    epicsInt32 triggerMode = ADTriggerInternal;
    epicsInt32 triggerPolarity = PRExtRisingEdge;
    setStatIfHigher(&status, getIntegerParam(ADAcquire, &acquire));   // Getting the ADAcquire value.
    setStatIfHigher(&status, getIntegerParam(ADTriggerMode, &triggerMode));
    setStatIfHigher(&status, getIntegerParam(PR_TriggerPolarity, &triggerPolarity));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to get parameters for %d update\n", param);
        return status;
    }
    if (acquire == 1)
    {   // If acquisition is enabled, stop the current acquisition before reloading the configuration.
        status = acquireStop();
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "%d updated, but failed to stop acquisition for reloading configuration\n", param);
            return status;
        }
    }
    callParamCallbacks();
    // Video settings have to be loaded respective of new values.
    status = this->changeVideoFormatConfig(binX, binY, sizeX, sizeY);
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to change video format config for %d update\n", param);
        return status;
    }
    status = setIntegerParam(PR_TriggerPolarity, triggerPolarity);
    setStatIfHigher(&status, this->setTriggerMode(triggerMode));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to update trigger mode after update of %d\n", param);
        return status;
    }
    status = setupAcquisition();
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
            "Failed to set up detector for acquisition after update of %d\n", param);
        return status;
    }
    if (acquire == 1)
    {   // starting acquisition if acquisition was running before.
        status = acquireStart();
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "Failed to restart acquisition after update of %d\n", param);
        }
    }
    return status;
}

asynStatus ADPixci::handleParamTask(epicsInt32 param, epicsFloat64 d_val, epicsInt32 i_val, epicsBoolean b_val)
{
    asynStatus status = asynSuccess;
    if (param == PR_SoftTrigger)
    {
        status = this->sendSoftTrigger();
    }
    else if (param == PR_UpdateInfo)
    {
        status = this->updateInfo();
    }
    else if (param == PR_UpdateTemperature)
    {
        status = this->updateTemperatureActual();
    }
    else if (param == PR_ReadoutMode)
    {
        status = this->setReadoutMode(i_val);
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set readout mode\n");
        }
        status = setIntegerParam(PR_ReadoutMode, this->getReadoutMode());
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set readout mode but failed to update readback\n");
        }
    }
    else if (param == PR_PixelReadoutClock)
    {
        status = this->setPixelReadoutClock(i_val);
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set pixel readout clock\n");
        }
        status = setIntegerParam(PR_PixelReadoutClock, this->getPixelReadoutClock());
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set pixel readout clock but failed to update readback\n");
        }
    }
    else if (param == ADAcquirePeriod)
    {
        if (d_val == 0.0) return status;
        status = this->setFrameRate(1 / d_val);
        if (status > asynSuccess) return status;
        epicsFloat64 readBackFrameRate = this->getFrameRate();
        if (readBackFrameRate <= 0.0)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error in frame rate readback. Readback not set\n");
            return asynError;
        }
        status = setDoubleParam(ADAcquirePeriod, (1 / readBackFrameRate));
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set frame rate but failed to update readback\n");
        }
    }
    else if (param == ADTemperature)
    {
        status = this->setCoolingSetPoint(d_val);
        if (status > asynSuccess) return status;
        epicsFloat64 coolingSetPoint = this->getCoolingSetPoint();
        status = setDoubleParam(ADTemperature, coolingSetPoint);
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set cooling set point but failed to update readback\n");
        }
    }
    else if (param == ADAcquireTime)
    {
        if (d_val == 0.0) return status;
        status = this->setExposure(d_val);
        if (status > asynSuccess) return status;
        epicsFloat64 readBackAcquireTime = this->getExposure();
        if (readBackAcquireTime <= 0.0)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error in acquire time readback. Readback not set\n");
            return asynError;
        }
        status = setDoubleParam(ADAcquireTime, readBackAcquireTime);
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set acquire time but failed to update readback\n");
        }
    }
    else if (param == ADShutterOpenDelay)
    {
        status = this->setShutterOpenDelay(d_val);
        if (status > asynSuccess) return status;
        epicsFloat64 openDelay = this->getShutterOpenDelay();
        status = setDoubleParam(ADShutterOpenDelay, openDelay);
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set shutter open delay but failed to update readback\n");
        }
    }
    else if (param == ADShutterCloseDelay)
    {
        status = this->setShutterCloseDelay(d_val);
        if (status > asynSuccess) return status;
        epicsFloat64 closeDelay = this->getShutterCloseDelay();
        status = setDoubleParam(ADShutterCloseDelay, closeDelay);
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set shutter close delay but failed to update readback\n");
        }
    }
    else if (param == ADBinX)
    {
        epicsInt32 binY = 0;
        epicsInt32 sizeX = 0;
        epicsInt32 sizeY = 0;
        status = this->setBin(i_val, BIN_AXIS_X);
        if (status > asynSuccess) return status;
        status = setIntegerParam(ADBinX, i_val);     // Updating the binX value.
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "Status %d: Set binX but failed to update readback\n", status);
            return status;
        }
        status = getIntegerParam(ADBinY, &binY);
        setStatIfHigher(&status, getIntegerParam(ADSizeX, &sizeX));
        setStatIfHigher(&status, getIntegerParam(ADSizeY, &sizeY));
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
        else if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "Status %d: Failed to read current parameter values\n", status);
            return status;
        }
        // Reload the configuration with new binX value
        status = reloadConfiguration(param, i_val, binY, sizeX, sizeY);
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
    }
    else if (param == ADBinY)
    {
        epicsInt32 binX = 0;
        epicsInt32 sizeX = 0;
        epicsInt32 sizeY = 0;
        status = this->setBin(i_val, BIN_AXIS_Y);
        if (status > asynSuccess) return status;
        status = setIntegerParam(ADBinY, i_val);
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set binY but failed to update readback\n");
            return status;
        }
        status = getIntegerParam(ADBinX, &binX);
        setStatIfHigher(&status, getIntegerParam(ADSizeX, &sizeX));
        setStatIfHigher(&status, getIntegerParam(ADSizeY, &sizeY));
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
        else if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to read current parameter value\n");
            return status;
        }
        // Reload the configuration with new binY value
        status = reloadConfiguration(param, binX, i_val, sizeX, sizeY);
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
    }
    else if (param == ADMinX)
    {
        epicsInt32 maxSizeX = 0;
        epicsInt32 sizeX = 0;
        epicsInt32 sizeY = 0;
        epicsInt32 binX = 0;
        epicsInt32 binY = 0;
        status = getIntegerParam(ADMaxSizeX, &maxSizeX);
        setStatIfHigher(&status, getIntegerParam(ADSizeX, &sizeX));
        setStatIfHigher(&status, getIntegerParam(ADSizeY, &sizeY));
        setStatIfHigher(&status, getIntegerParam(ADBinX, &binX));
        setStatIfHigher(&status, getIntegerParam(ADBinY, &binY));
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
        else if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to read current parameter values\n");
            return status;
        }
        epicsInt32 minX = (i_val > maxSizeX) ? maxSizeX : i_val;
        if ((sizeX + minX) > maxSizeX)
        {
            sizeX = maxSizeX - minX;
            status = this->setRoiSizeX(sizeX);
            if (status > asynSuccess)
            {
                asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set new ROI Size X value\n");
                return status;
            }
        }
        status = this->setRoiOffsetX(minX);
        if (status > asynSuccess) return status;
        status = setIntegerParam(ADSizeX, this->getRoiSizeX());
        setStatIfHigher(&status, setIntegerParam(ADMinX, this->getRoiOffsetX()));
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set ROI Offset X but failed to update readback\n");
            return status;
        }
        status = reloadConfiguration(param, binX, binY, sizeX, sizeY);
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
    }
    else if (param == ADMinY)
    {
        epicsInt32 maxSizeY = 0;
        epicsInt32 sizeX = 0;
        epicsInt32 sizeY = 0;
        epicsInt32 binX = 0;
        epicsInt32 binY = 0;
        status = getIntegerParam(ADMaxSizeY, &maxSizeY);
        setStatIfHigher(&status, getIntegerParam(ADSizeX, &sizeX));
        setStatIfHigher(&status, getIntegerParam(ADSizeY, &sizeY));
        setStatIfHigher(&status, getIntegerParam(ADBinX, &binX));
        setStatIfHigher(&status, getIntegerParam(ADBinY, &binY));
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
        else if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to read current parameter values\n");
            return status;
        }
        epicsInt32 minY = (i_val > maxSizeY) ? maxSizeY : i_val;
        if ((sizeY + minY) > maxSizeY)
        {
            sizeY = maxSizeY - minY;
            status = this->setRoiSizeY(sizeY);
            if (status > asynSuccess)
            {
                asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set new ROI Size Y value\n");
                return status;
            }
        }
        status = this->setRoiOffsetY(minY);
        if (status > asynSuccess) return status;
        status = setIntegerParam(ADSizeY, this->getRoiSizeY());
        setStatIfHigher(&status, setIntegerParam(ADMinY, this->getRoiOffsetY()));
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set ROI Offset Y but failed to update readback\n");
            return status;
        }
        status = reloadConfiguration(param, binX, binY, sizeX, sizeY);
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
    }
    else if (param == ADSizeX)
    {
        epicsInt32 maxSizeX = 0;
        epicsInt32 minX = 0;
        epicsInt32 sizeY = 0;
        epicsInt32 binX = 0;
        epicsInt32 binY = 0;
        status = getIntegerParam(ADMaxSizeX, &maxSizeX);
        setStatIfHigher(&status, getIntegerParam(ADMinX, &minX));
        setStatIfHigher(&status, getIntegerParam(ADSizeY, &sizeY));
        setStatIfHigher(&status, getIntegerParam(ADBinX, &binX));
        setStatIfHigher(&status, getIntegerParam(ADBinY, &binY));
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
        else if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to read current parameter values\n");
            return status;
        }
        epicsInt32 sizeX = (i_val > maxSizeX) ? maxSizeX : i_val;
        if ((sizeX + minX) > maxSizeX)
        {
            minX = maxSizeX - sizeX;
            status = this->setRoiOffsetX(minX);
            if (status > asynSuccess)
            {
                asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set new ROI Offset X value\n");
                return status;
            }
        }
        status = this->setRoiSizeX(sizeX);
        if (status > asynSuccess) return status;
        status = setIntegerParam(ADMinX, this->getRoiOffsetX());
        setStatIfHigher(&status, setIntegerParam(ADSizeX, this->getRoiSizeX()));
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set ROI Size X but failed to update readback\n");
            return status;
        }
        status = reloadConfiguration(param, binX, binY, sizeX, sizeY);
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
    }
    else if (param == ADSizeY)
    {
        epicsInt32 maxSizeY = 0;
        epicsInt32 minY = 0;
        epicsInt32 sizeX = 0;
        epicsInt32 binX = 0;
        epicsInt32 binY = 0;
        status = getIntegerParam(ADMaxSizeY, &maxSizeY);
        setStatIfHigher(&status, getIntegerParam(ADMinY, &minY));
        setStatIfHigher(&status, getIntegerParam(ADSizeX, &sizeX));
        setStatIfHigher(&status, getIntegerParam(ADBinX, &binX));
        setStatIfHigher(&status, getIntegerParam(ADBinY, &binY));
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
        else if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to read current parameter values\n");
            return status;
        }
        epicsInt32 sizeY = (i_val > maxSizeY) ? maxSizeY : i_val;
        if ((sizeY + minY) > maxSizeY)
        {
            minY = maxSizeY - sizeY;
            status = this->setRoiOffsetY(minY);
            if (status > asynSuccess)
            {
                asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set new ROI Offset Y value\n");
                return status;
            }
        }
        status = this->setRoiSizeY(sizeY);
        if (status > asynSuccess) return status;
        status = setIntegerParam(ADMinY, this->getRoiOffsetY());
        setStatIfHigher(&status, setIntegerParam(ADSizeY, this->getRoiSizeY()));
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Set ROI Size Y but failed to update readback\n");
            return status;
        }
        status = reloadConfiguration(param, binX, binY, sizeX, sizeY);
        if (status == asynParamUndefined)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "One or more parameters for processing %d are not "
                "initialized yet. Reprocessing parameter %d\n", param, param);
            this->addToParamQue(param, i_val);
            return asynSuccess;
        }
    }
    return status;
}

void ADPixci::paramTask()
{
    epicsFloat64 functionAndVal[2] = {};
    epicsInt32 function = 0;
    epicsFloat64 d_val = 0.0;
    epicsInt32 i_val = 0;
    epicsBoolean b_val = epicsFalse;
    asynStatus status = asynSuccess;
    static const char *functionName = "paramTask";

    // Updating all the PVs related to the status of device and the manufacturers data
    status = this->updateInitialPVs();
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Failed to update initial PVs\n", driverName);
        setIntegerParam(ADStatus, ADStatusError);
        setStringParam(ADStatusMessage, "Failed to update initial PVs");
    }

    for (;;)
    {
        this->paramMsgQue->receive(functionAndVal, PARAM_MESSAGE_SIZE);
        function = static_cast<epicsInt32>(functionAndVal[0]);
        b_val = static_cast<epicsBoolean>(functionAndVal[1]);
        i_val = static_cast<epicsInt32>(functionAndVal[1]);
        d_val = functionAndVal[1];
        asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
            "%s: Parameter: %d, Value: %f\n", functionName, function, d_val);
        this->lock();
        status = this->handleParamTask(function, d_val, i_val, b_val);
        if (status > asynSuccess)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "Failed to handle parameter change. Parameter %d not changed to %f\n", function, d_val);
        }
        callParamCallbacks();
        this->unlock();
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

    // checking if any message packer left to read, and clear the buffer by reading it
    if (pxd_serialRead(unit, RESERVED, nullptr, 0) > 0)
    {
        count = pxd_serialRead(unit, RESERVED, serialIn, serialInBufferSize);
        if (count < PIXCI_NO_ERROR)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "%s: Failed to clear the serial bugger on the camera unit. Error code: %s.",
                driverName, pxd_mesgErrorCode(count));
        }
    }

    // wait if any messages are in the send queue
    while (pxd_serialWrite(unit, RESERVED, nullptr, 0) < msgOutSize && outMsgwait < 50)
    {
        outMsgwait++;
        Sleep(10);
    }
    outMsgwait = 0;

    // creating checksum to send as the last character of the message
    for (int i = 0; i < msgOutSize; i++)
    {
        bufOut[i] = serialOut[i];
        chkSum ^= serialOut[i];
    }
    serialOut[msgOutSize] = chkSum;

    // sending the serialOut message
    count = pxd_serialWrite(unit, RESERVED, serialOut, msgOutSize + 1);
    Sleep(130);

    if (count < PIXCI_NO_ERROR)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Failed to serial write to camera unit. Error code: %s.",
            driverName, pxd_mesgErrorCode(count));
        return count;
    }
    else
    {   // waiting for the reply
        inMsgwaitFlag = pxd_serialRead(unit, 0, nullptr, 0);
        while (inMsgwaitFlag < 1 && inMsgwait < 20)
        {
            inMsgwait++;
            inMsgwaitFlag = pxd_serialRead(unit, 0, nullptr, 0);
            Sleep(10);
        }
        inMsgwait = 0;
        // read the message and message count
        count = pxd_serialRead(UNIT, RESERVED, serialIn, serialInBufferSize);
        if (count < PIXCI_NO_ERROR)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "%s: Failed to serial read from the camera unit. Error code: %s.",
                driverName, pxd_mesgErrorCode(count));
        }
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
        function == PR_ReadoutMode || function == PR_PixelReadoutClock || function == ADBinX || function == ADBinY ||
        function == ADMinX || function == ADMinY || function == ADSizeX || function == ADSizeY)
    {
        return addToParamQue(function, value);
    }
    return ADDriver::writeInt32(pasynUser, value);
}

asynStatus ADPixci::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    epicsInt32 function = pasynUser->reason;
    static const char *functionName = "writeFloat64";

    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
        "%s: Parameter: %d, Value: %d\n", functionName, function, value);

    if (function == ADGain || function == ADAcquirePeriod || function == ADAcquireTime || function == ADTemperature ||
        function == ADShutterOpenDelay || function == ADShutterCloseDelay)
    {
        return addToParamQue(function, value);
    }
    return ADDriver::writeFloat64(pasynUser, value);
}

asynStatus ADPixci::addToParamQue(epicsInt32 function, epicsInt32 value)
{
    epicsInt32 err = 0;
    epicsFloat64 functionAndVal[2] = {static_cast<epicsFloat64>(function), static_cast<epicsFloat64>(value)};
    err = this->paramMsgQue->send(functionAndVal, PARAM_MESSAGE_SIZE);
    if (err < 0)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to send parameter message: message too large\n");
        return asynError;
    }
    return asynSuccess;
}

asynStatus ADPixci::addToParamQue(epicsInt32 function, epicsFloat64 value)
{
    epicsInt32 err = 0;
    epicsFloat64 functionAndVal[2] = {static_cast<epicsFloat64>(function), value};
    err = this->paramMsgQue->send(functionAndVal, PARAM_MESSAGE_SIZE);
    if (err < 0)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to send parameter message: message too large\n");
        return asynError;
    }
    return asynSuccess;
}

asynStatus ADPixci::updateInitialPVs()
{
    epicsInt32 sizeX = pxd_imageXdim();
    epicsInt32 sizeY = pxd_imageYdim();
    asynStatus status = asynSuccess;

    setStatIfHigher(&status, setIntegerParam(ADMaxSizeX, sizeX));
    setStatIfHigher(&status, setIntegerParam(ADMaxSizeY, sizeY));
    setStatIfHigher(&status, setIntegerParam(ADSizeX, sizeX));
    setStatIfHigher(&status, setIntegerParam(ADSizeY, sizeY));

    callParamCallbacks();
    return status;
}

asynStatus ADPixci::changeVideoFormatConfig(epicsInt32 binX, epicsInt32 binY, epicsInt32 sizeX, epicsInt32 sizeY)
{
    epicsInt32 err = PIXCI_NO_ERROR;
    err = pxd_setVideoResolution(UNIT, sizeX / binX, sizeY / binY, 0, 0);
    if (err < PIXCI_NO_ERROR){
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set video resolution\n");
        return asynError;
    }
    return asynSuccess;
}
