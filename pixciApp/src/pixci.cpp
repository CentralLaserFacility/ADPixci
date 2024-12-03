/**
 * @brief This is a driver for PIXCI frame grabber from epix, inc. Developed for Eagle XV CCD from Raptor photonics
 *
 */

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
extern "C"
{
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

#define BINNINGSETTINGS_4240_1X1 "videoSettings\Raptor_Photonics_EagleXV_42-40.fmt"
#define BINNINGSETTINGS_4240_1X2 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning1x2.fmt"
#define BINNINGSETTINGS_4240_1X4 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning1x4.fmt"
#define BINNINGSETTINGS_4240_1X8 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning1x8.fmt"
#define BINNINGSETTINGS_4240_1X16 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning1x16.fmt"
#define BINNINGSETTINGS_4240_1X32 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning1x32.fmt"
#define BINNINGSETTINGS_4240_2X1 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning2x1.fmt"
#define BINNINGSETTINGS_4240_2X2 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning2x2.fmt"
#define BINNINGSETTINGS_4240_2X4 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning2x4.fmt"
#define BINNINGSETTINGS_4240_2X8 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning2x8.fmt"
#define BINNINGSETTINGS_4240_2X16 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning2x16.fmt"
#define BINNINGSETTINGS_4240_2X32 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning2x32.fmt"
#define BINNINGSETTINGS_4240_4X1 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning4x1.fmt"
#define BINNINGSETTINGS_4240_4X2 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning4x2.fmt"
#define BINNINGSETTINGS_4240_4X4 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning4x4.fmt"
#define BINNINGSETTINGS_4240_4X8 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning4x8.fmt"
#define BINNINGSETTINGS_4240_4X16 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning4x16.fmt"
#define BINNINGSETTINGS_4240_4X32 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning4x32.fmt"
#define BINNINGSETTINGS_4240_8X1 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning8x1.fmt"
#define BINNINGSETTINGS_4240_8X2 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning8x2.fmt"
#define BINNINGSETTINGS_4240_8X4 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning8x4.fmt"
#define BINNINGSETTINGS_4240_8X8 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning8x8.fmt"
#define BINNINGSETTINGS_4240_8X16 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning8x16.fmt"
#define BINNINGSETTINGS_4240_8X32 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning8x32.fmt"
#define BINNINGSETTINGS_4240_16X1 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning16x1.fmt"
#define BINNINGSETTINGS_4240_16X2 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning16x2.fmt"
#define BINNINGSETTINGS_4240_16X4 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning16x4.fmt"
#define BINNINGSETTINGS_4240_16X8 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning16x8.fmt"
#define BINNINGSETTINGS_4240_16X16 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning16x16.fmt"
#define BINNINGSETTINGS_4240_16X32 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning16x32.fmt"
#define BINNINGSETTINGS_4240_32X1 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning32x1.fmt"
#define BINNINGSETTINGS_4240_32X2 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning32x2.fmt"
#define BINNINGSETTINGS_4240_32X4 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning32x4.fmt"
#define BINNINGSETTINGS_4240_32X8 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning32x8.fmt"
#define BINNINGSETTINGS_4240_32X16 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning32x16.fmt"
#define BINNINGSETTINGS_4240_32X32 "videoSettings\Raptor_Photonics_EagleXV_42-40_binning32x32.fmt"

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
_cDcl(_dllpxlib, _cfunfcc, int)
    pxd_setVideoResolution(
        int unitmap, // usual
        int xdim,    // pixels per line
        int ydim,    // pixels per column (per field)
        int hoffset, // video hoffset
        int voffset  // video voffset
    )
{
    int r = 0, r1 = 0;
    int u = 0, umap = 0, multiple = 0;
    struct xclibs *xc;

#if USEINTERNALAPI
    if (liblog_active)
        liblog_aasrbz("pxd_setVideoResolution", "", "D*",
                      &unitmap, (size_t)sizeof(unitmap),
                      &xdim, (size_t)sizeof(xdim),
                      &ydim, (size_t)sizeof(ydim),
                      &hoffset, (size_t)sizeof(hoffset),
                      &voffset, (size_t)sizeof(voffset),
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
#if USEINTERNALAPI // using internal API
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

#endif // !defined(PIXCI_LITE)

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
extern "C" int pixciConfig(const char *portName,
                           int maxBuffers, size_t maxMemory, int priority, int stackSize, int cameraModel, const char *formatFile)
{
    new Pixci(portName, maxBuffers, maxMemory, priority, stackSize, cameraModel, formatFile);
    return (asynSuccess);
}

auto setStatIfHigher = [](asynStatus &status, asynStatus returnedStatus)
{
    status = (status > returnedStatus) ? status : returnedStatus;
};

/*
 * @brief Default constructor to create a new Pixci::Pixci object
 */
Pixci::Pixci(const char *portName, int maxBuffers, size_t maxMemory, int priority, int stackSize, int cameraModel, const char *formatFile)
    : ADDriver(portName, 1, (int)1, maxBuffers, maxMemory, 0, 0, ASYN_CANBLOCK, 1, priority, stackSize)
{
    int connectionStatusCode = 0;
    int serialConnection = 0;
    Pixci::cameraModel = cameraModel;

    createParam(SoftTriggerParamString, asynParamInt32, &PR_SoftTrigger);
    createParam(TriggerPolarityParamString, asynParamInt32, &PR_TriggerPolarity);
    createParam(UpdateTemperatureString, asynParamInt32, &PR_UpdateTemperature);
    createParam(TemperaturePCBString, asynParamFloat64, &PR_TemperaturePcb);
    createParam(ToggleTecString, asynParamInt32, &PR_ToggleTec);
    createParam(ToggleGainString, asynParamInt32, &PR_ToggleGain);
    createParam(ToggleFPGACommsString, asynParamInt32, &PR_ToggleFpgaComms);

    createParam(UpdateStatusString, asynParamInt32, &PR_UpdateStatus);
    createParam(BuildDateString, asynParamOctet, &PR_BuildDate);
    createParam(ADCCalibrationZeroDegreeString, asynParamInt32, &PR_ADCCalibrationZeroDegree);
    createParam(ADCCalibrationFortyDegreeString, asynParamInt32, &PR_ADCCalibrationFortyDegree);
    createParam(DACCalibrationZeroDegreeString, asynParamInt32, &PR_DACCalibrationZeroDegree);
    createParam(DACCalibrationFortyDegreeString, asynParamInt32, &PR_DACCalibrationFortyDegree);

    /* pxd_PIXCIopen(driverparms, formatname, formatfile) return 0 if connection is successfull
     * returns value <0 if any error occured
     * pxd_mesgErrorCode(int code) will return description of the error occured
     */
    if (cameraModel == DETECTOR_2048)
    {
        connectionStatusCode = pxd_PIXCIopen(DRIVERPARMS, FORMAT, formatFile);
    }
    else
    {
        connectionStatusCode = pxd_PIXCIopen(DRIVERPARMS, FORMAT, formatFile);
    }
    if (connectionStatusCode < NOERROR)
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
        if (serialConnection < NOERROR)
        {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                      "%s: Cannot make serial connection: %s.",
                      driverName, pxd_mesgErrorCode(connectionStatusCode));
        }
    }

    /* Any thread waiting upon the event will be notified whenever a field has been captured by pxd_goSnap,
    pxd_goLive, pxd_goLivePair and pxd_goLiveSeq*/
    g_hEvent = pxd_eventCapturedFieldCreate(UNIT);
    int status = asynSuccess;
    status = setStringParam(ADManufacturer, "Raptor Photonics");

    paramMsgQue = new epicsMessageQueue(PARAM_MESSAGE_QUE_SIZE, PARAM_MESSAGE_SIZE);

    /* Create the thread that does data acquisition */
    if (connectionStatusCode >= NOERROR && serialConnection >= NOERROR)
    {
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
    }

    // Updating all the PVs related to the status of device
    updateStatus(true); // Also update the manufacturers data
    status |= updateIntialPVs();
    if (status == asynError)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to initialize the detector\n");
        return;
    }
}

Pixci::~Pixci()
{

    /* Closing connection to frame grabber */
    int disconnectStatusCode = NOERROR;
    /*pxd_PIXCIclose() disconnect the driver from the device.
     * return 0 if disconnect successfull, return integer <0 if error occured
     * pxd_mesgErrorCode(int code) will return description of the error occured
     */
    disconnectStatusCode = pxd_PIXCIclose();
    if (disconnectStatusCode < NOERROR)
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
    int binX = 0; 
    int binY = 0; 
    int RoiSizeX = 0; 
    int RoiSizeY = 0;
    int sizeX = pxd_imageXdim();
    int sizeY = pxd_imageYdim();
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
    pxbuffer_t buffer = 1L; // Image frame buffer
    /* live capture the image into frame buffer */
    int error = pxd_goLive(UNIT, buffer);
    if (error < NOERROR)
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
    int error = pxd_goUnLive(UNIT);
    if (error < NOERROR)
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
    Pixci *pPvt = (Pixci *)drvPvt;
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
            pxd_readushort(UNIT, buf, 0, 0, sizeX, sizeY, (ushort *)pImage->pData, dims[0] * dims[1] * sizeof(epicsUInt16), "GRAY");
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

        setIntegerParam(NDArraySize, static_cast<int>(dims[0] * dims[1] * sizeof(NDUInt16)));
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

void Pixci::paramTask()
{
    epicsFloat64 functionAndVal[2] = {};
    epicsInt32 function = 0;
    epicsFloat64 d_val = 0.0;
    epicsInt32 i_val = 0;
    asynStatus status = asynSuccess;
    epicsInt32 acquire = 0;

    for (;;)
    {
        paramMsgQue->receive(functionAndVal, PARAM_MESSAGE_SIZE);
        function = static_cast<epicsInt32>(functionAndVal[0]);
        i_val = static_cast<epicsInt32>(functionAndVal[1]);
        d_val = functionAndVal[1];
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
                setIntegerParam(ADBinX, i_val); // Updating the binX value.
                callParamCallbacks();
                getIntegerParam(ADAcquire, &acquire); // Getting the ADAcquire value.
                reloadVideoSettings();                // Video settings have to be loaded respective of binning value.
                acquireStop();                        // Acquire have to be stopped before calling setupAcquisition.
                pxd_setVideoResolution(UNIT, sizeX / i_val, sizeY / binY, 0, 0);
                setupAquisition();
                if (acquire == 1)
                {
                    acquireImage(); // starting acquisition if acquisition was running before.
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
                setIntegerParam(ADBinY, i_val);
                callParamCallbacks();
                getIntegerParam(ADAcquire, &acquire);
                reloadVideoSettings();
                acquireStop();
                pxd_setVideoResolution(UNIT, sizeX / binX, sizeY / i_val, 0, 0);
                setupAquisition();
                if (acquire == 1)
                {
                    acquireImage();
                }
            }
        }
        else if (function == ADTriggerMode)
        {
            int acquisitionStatus = asynSuccess;
            int previousTriggerMode = 0;
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
            /* if trigger mode is button trigger then, do the soft trigger else print error */
            int triggerMode = 0;
            getIntegerParam(ADTriggerMode, &triggerMode);
            if (triggerMode == PR_BUTTON_TRIGGER)
            {
                status = Pixci::writeSerialRegister(UNIT, TRIGGER_MODE_BYTE, SOFT_TRIGGER_BYTE);
            }
            else
            {
                asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Button Trigger mode is not selected");
            }
        }
        else if (function == PR_UpdateStatus)
        {
            status = updateStatus(true);
        }
        else if (function == PR_UpdateTemperature)
        {
            updateStatus();
        }
        else if (function == ADAcquirePeriod)
        {
            if (d_val != 0.0)
            {
                status = setFrameRate(1 / d_val);
                if (status == asynSuccess)
                {
                    double readBackFrameRate= getFrameRate();
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
                double tecTemperature = getTecTemperature();
                setDoubleParam(ADTemperature, tecTemperature);
            }
        }
        else if (function == PR_ToggleTec)
        {
            status = toggleTec(i_val);
            if (status == asynSuccess)
            {
                setIntegerParam(PR_ToggleTec, isTecEnabled());
            }
        }
        else if (function == PR_ToggleGain)
        {
            status = toggleGain(i_val);
            if (status == asynSuccess)
            {
                setIntegerParam(PR_ToggleGain, isGainEnabled());
            }
        }
        else if (function == PR_ToggleFpgaComms)
        {
            status = toggleFpgaComms(i_val);
            if (status == asynSuccess)
            {
                setIntegerParam(PR_ToggleFpgaComms, isFpgaCommsEnabled());
            }
        }
        else if (function == ADAcquireTime)
        {
            if (d_val != 0.0)
            {
                status = setExposure(d_val);
                if (status == asynSuccess)
                {
                    double readBackAcquireTime = getExposure(); 
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
            int triggerMode = PR_INTERNAL_ITR;
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
        callParamCallbacks();
    }
}

void Pixci::reloadVideoSettings()
{
    epicsInt32 sizeX = 0;
    epicsInt32 sizeY = 0;
    getIntegerParam(ADBinX, &sizeX);
    getIntegerParam(ADBinY, &sizeY);

    switch (sizeX)
    {
    case BINNING1:
        if (sizeY == BINNING1)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_1X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_1X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING2)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_1X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_1X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING4)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_1X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_1X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING8)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_1X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_1X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING16)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_1X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_1X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING32)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_1X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_1X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        break;
    case BINNING2:
        if (sizeY == BINNING1)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_2X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_2X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING2)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_2X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_2X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING4)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_2X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_2X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING8)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_2X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_2X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING16)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_2X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_2X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING32)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_2X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_2X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        break;
    case BINNING4:
        if (sizeY == BINNING1)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_4X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_4X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING2)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_4X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_4X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING4)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_4X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_4X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING8)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_4X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_4X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING16)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_4X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_4X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING32)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_4X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_4X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        break;
    case BINNING8:
        if (sizeY == BINNING1)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_8X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_8X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING2)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_8X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_8X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING4)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_8X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_8X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING8)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_8X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_8X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING16)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_8X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_8X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING32)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_8X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_8X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        break;
    case BINNING16:
        if (sizeY == BINNING1)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_16X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_16X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING2)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_16X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_16X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING4)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_16X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_16X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING8)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_16X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_16X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING16)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_16X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_16X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING32)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_16X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_16X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        break;
    case BINNING32:
        if (sizeY == BINNING1)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_32X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_32X1
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING2)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_32X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_32X2
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING4)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_32X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_32X4
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING8)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_32X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_32X8
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING16)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_32X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_32X16
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        else if (sizeY == BINNING32)
        {
            if (cameraModel == DETECTOR_2048)
            {
#include BINNINGSETTINGS_4240_32X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
            else
            {
#include BINNINGSETTINGS_32X32
                pxd_videoFormatAsIncludedInit(0);
                pxd_videoFormatAsIncluded(0);
            }
        }
        break;
    default:
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid binning value");
        break;
    }
}

void Pixci::resetVideoSettings()
{
    epicsInt32 binX = 0;
    epicsInt32 binY = 0;
    getIntegerParam(ADBinX, &binX);
    getIntegerParam(ADBinY, &binY);
    if (cameraModel == DETECTOR_2048)
    {
#include BINNINGSETTINGS_4240_1X1
        pxd_videoFormatAsIncludedInit(0);
        pxd_videoFormatAsIncluded(0);
    }
    else
    {
#include BINNINGSETTINGS_1X1
        pxd_videoFormatAsIncludedInit(0);
        pxd_videoFormatAsIncluded(0);
    }

    setBin(binX, BIN_AXIS_X);
    setBin(binY, BIN_AXIS_Y);
    reloadVideoSettings();
}

epicsUInt64 Pixci::uCharToEpicsUInt64(char *cval)
{
    epicsUInt64 lval = 0;
    lval += (epicsUInt64)(epicsUInt8)cval[4];
    lval += ((epicsUInt64)(epicsUInt8)cval[3]) << 8;
    lval += ((epicsUInt64)(epicsUInt8)cval[2]) << 16;
    lval += ((epicsUInt64)(epicsUInt8)cval[1]) << 24;
    lval += ((epicsUInt64)(epicsUInt8)cval[0]) << 32;
    return lval;
}

void Pixci::epicsUInt64ToUChar(epicsUInt64 lval, char *cval)
{
    cval[0] = (char)((lval & 0xFF00000000) >> 32);
    cval[1] = (char)((lval & 0x00FF000000) >> 24);
    cval[2] = (char)((lval & 0x0000FF0000) >> 16);
    cval[3] = (char)((lval & 0x000000FF00) >> 8);
    cval[4] = (char)((lval & 0x00000000FF));
}

int Pixci::writeReadSerial(int unit, char *serialOut, int msgOutSize, char *serialIn, int serialInBufferSize)
{
    int count = 0;
    char bufOut[50] = {};
    char chkSum = 0;
    int outMsgwait = 0;
    int inMsgwait = 0;
    int inMsgwaitFlag = 0;

    /* checking if any message packer left to read, and clear the buffer by reading it */
    if (pxd_serialRead(unit, RESERVED, NULL, 0) > 0)
    {
        count = pxd_serialRead(unit, 0, serialIn, serialInBufferSize);
    }

    /* wait if any message is in the send que*/
    while (pxd_serialWrite(unit, RESERVED, NULL, 0) < msgOutSize && outMsgwait < 50)
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

    if (count < NOERROR)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                  "%s: Cannot serial write: %s.",
                  driverName, pxd_mesgErrorCode(count));
        return count;
    }
    else
    {
        /*waiting for the reply */
        inMsgwaitFlag = pxd_serialRead(unit, 0, NULL, 0);
        while (inMsgwaitFlag < 1 && inMsgwait < 20)
        {
            inMsgwait++;
            inMsgwaitFlag = pxd_serialRead(unit, 0, NULL, 0);
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
    char hexval = 0;
    char reg = (coordinate == BIN_AXIS_X) ? X_BIN_BYTE : Y_BIN_BYTE;

    /* Assigning corresponding Hex value to send*/
    switch (val)
    {
    case 1:
        hexval = 0x00;
        break;
    case 2:
        hexval = 0x01;
        break;
    case 4:
        hexval = 0x03;
        break;
    case 8:
        hexval = 0x07;
        break;
    case 16:
        hexval = 0x0F;
        break;
    case 32:
        hexval = 0x1F;
        break;
    case 64:
        hexval = 0x3F;
        break;
    default:
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid binning value %d", val);
        return asynError;
        break;
    }

    return Pixci::writeSerialRegister(UNIT, reg, hexval);
}

asynStatus Pixci::setFrameRate(double frameRate)
{
    char frameRateHexVal[5] = {0, 0, 0, 0, 0};
    epicsUInt64 frameRateCount = (epicsUInt64)(COUNT_PER_FRAME / frameRate);
    epicsUInt64ToUChar(frameRateCount, frameRateHexVal);

    writeSerialRegister(UNIT, FRAME_RATE_BYTES[0], frameRateHexVal[0]);
    writeSerialRegister(UNIT, FRAME_RATE_BYTES[1], frameRateHexVal[1]);
    writeSerialRegister(UNIT, FRAME_RATE_BYTES[2], frameRateHexVal[2]);
    writeSerialRegister(UNIT, FRAME_RATE_BYTES[3], frameRateHexVal[3]);
    return writeSerialRegister(UNIT, FRAME_RATE_BYTES[4], frameRateHexVal[4]);
}

double Pixci::getFrameRate()
{
    char cval[5] = {0, 0, 0, 0, 0};
    double frameRate = 0.0;
    readSerialRegister(FRAME_RATE_BYTES[0], &cval[0]);
    readSerialRegister(FRAME_RATE_BYTES[1], &cval[1]);
    readSerialRegister(FRAME_RATE_BYTES[2], &cval[2]);
    readSerialRegister(FRAME_RATE_BYTES[3], &cval[3]);
    readSerialRegister(FRAME_RATE_BYTES[4], &cval[4]);

    epicsUInt64 frameRateCount = uCharToEpicsUInt64(cval);
    if (frameRateCount > 0)
    {
        frameRate = 40e6 / double(frameRateCount);
    }
    return frameRate;
}

double Pixci::convertAdcCountToCentigrade(INT16 adcCount)
{
    return (ADC_M * adcCount) + ADC_C; // temperature in centigrade
}

INT16 Pixci::convertCentigradeToDacCount(double temperature)
{
    return static_cast<INT16>((temperature - DAC_C) / DAC_M);
}

double Pixci::convertDacCountToCentigrade(INT16 dacCount)
{
    return (DAC_M * dacCount) + DAC_C; // temperature in centigrade
}

double Pixci::getTemperatureActual()
{
    char cval[2] = {0, 0};

    readSerialRegister(CCD_SILISCON_TEMPERATURE_BYTES[0], CCD_SILISCON_TEMPERATURE_BYTES[1], &cval[0]);
    readSerialRegister(CCD_SILISCON_TEMPERATURE_BYTES[2], CCD_SILISCON_TEMPERATURE_BYTES[3], &cval[1]);

    INT16 adcCount = 0;
    adcCount += (INT16)(unsigned char)cval[1];
    adcCount += ((INT16)(unsigned char)cval[0]) << 8;

    return convertAdcCountToCentigrade(adcCount);
}

double Pixci::getTemperaturePcb()
{
    char cval[2] = {0, 0};

    readSerialRegister(PCB_TEMPERATURE_BYTES[0], PCB_TEMPERATURE_BYTES[1], &cval[1]);
    readSerialRegister(PCB_TEMPERATURE_BYTES[2], PCB_TEMPERATURE_BYTES[3], &cval[0]);

    INT16 lval = 0;
    lval += (INT16)(unsigned char)cval[0];
    lval += (INT16)(unsigned char)(cval[1] & 0x0F) << 8;
    return lval / 16.0;
}

double Pixci::getTecTemperature()
{
    char cval[2] = {0, 0};

    readSerialRegister(TEC_TEMPERATURE_BYTES[0], &cval[1]);
    readSerialRegister(TEC_TEMPERATURE_BYTES[1], &cval[0]);

    INT16 lval = 0;
    lval += (INT16)(unsigned char)cval[0];
    lval += (INT16)(unsigned char)(cval[1] & 0x0F) << 8;

    return convertDacCountToCentigrade(lval);
}

asynStatus Pixci::setTecTemperature(double temperature)
{
    INT16 dacCount = convertCentigradeToDacCount(temperature);

    char cval[2] = {0, 0};
    cval[0] = (char)((dacCount & 0x0F00) >> 8);
    cval[1] = (char)((dacCount & 0x00FF));

    writeSerialRegister(UNIT, TEC_TEMPERATURE_BYTES[0], cval[0]);
    return writeSerialRegister(UNIT, TEC_TEMPERATURE_BYTES[1], cval[1]);
}

unsigned char Pixci::getFpgaStatus()
{
    char cval = 0;
    readSerialRegister(FPGA_STATUS_BYTE, &cval);
    // TODO: implement proper error handling
    return (unsigned char)cval;
}

asynStatus Pixci::toggleTec(bool enableTec)
{
    unsigned char fpgaStatus = getFpgaStatus();
    if (enableTec)
        return writeSerialRegister(UNIT, FPGA_STATUS_BYTE, fpgaStatus | 0x01); // setting first bit = 1
    else
        return writeSerialRegister(UNIT, FPGA_STATUS_BYTE, fpgaStatus & ~(0x01)); // setting first bit = 0
}

bool Pixci::isTecEnabled()
{
    unsigned char fpgaStatus = getFpgaStatus();
    return (fpgaStatus & 0x01) != 0; // check the first bit is not 0
}

asynStatus Pixci::toggleGain(bool enableGain)
{
    unsigned char fpgaStatus = getFpgaStatus();
    if (enableGain)
        return writeSerialRegister(UNIT, FPGA_STATUS_BYTE, fpgaStatus | (1 << 7)); // setting last bit = 1
    else
        return writeSerialRegister(UNIT, FPGA_STATUS_BYTE, fpgaStatus & ~(1 << 7)); // setting last bit = 0
}

bool Pixci::isGainEnabled()
{
    unsigned char fpgaStatus = getFpgaStatus();
    return (fpgaStatus & (1 << 7)) != 0; // check the last bit is not 0
}

unsigned char Pixci::getSystemStatus()
{
    char cval = 0;
    char inputMsg[2] = {};
    char first_bufout[] = {GET_SYSTEM_STATUS_BYTE, END_OF_TRANSMISSION_BYTE};

    /*writing to serial connection*/
    int inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 2);

    if (inputMsg[1] == SUCCESS_MESSAGE)
    {
        cval = inputMsg[0];
    }

    // TODO: Need proper error handling, same is for reading serial register as else where
    return (unsigned char)cval; // cval will be 0x00 if there is no success
}

asynStatus Pixci::setSystemStatus(char val)
{
    char inputMsg[1] = {};

    /* template of message to write value to registers */
    char bufout[] = {SET_SYSTEM_STATUS_BYTE, val, END_OF_TRANSMISSION_BYTE};

    /*writing to serial connection*/
    int inSize = writeReadSerial(UNIT, bufout, sizeof(bufout), inputMsg, 1);

    if (inSize < NOERROR)
    {
        return asynError;
    }

    if (inputMsg[0] == SUCCESS_MESSAGE)
    {
        return asynSuccess;
    }

    return asynError;
}

asynStatus Pixci::toggleFpgaComms(bool enableFpgaComms)
{
    unsigned char systemStatus = getSystemStatus();
    if (enableFpgaComms)
        return setSystemStatus(systemStatus | 0x01); // setting first bit = 1
    else
        return setSystemStatus(systemStatus & ~(0x01)); // setting first bit = 0
}

bool Pixci::isFpgaCommsEnabled()
{
    unsigned char systemStatus = getSystemStatus();
    return (systemStatus & 0x01) != 0; // check the first bit is not 0
}

asynStatus Pixci::setExposure(double exposureTime)
{
    char exposureTimeHexVal[5] = {0, 0, 0, 0, 0};
    epicsUInt64 exposureTimeCount = (epicsUInt64)(exposureTime * EXPOSURE_COUNT_TO_TIME / SEC_TO_mS);
    epicsUInt64ToUChar(exposureTimeCount, exposureTimeHexVal);

    writeSerialRegister(UNIT, EXPOSURE_BYTES[0], exposureTimeHexVal[0]);
    writeSerialRegister(UNIT, EXPOSURE_BYTES[1], exposureTimeHexVal[1]);
    writeSerialRegister(UNIT, EXPOSURE_BYTES[2], exposureTimeHexVal[2]);
    writeSerialRegister(UNIT, EXPOSURE_BYTES[3], exposureTimeHexVal[3]);
    return writeSerialRegister(UNIT, EXPOSURE_BYTES[4], exposureTimeHexVal[4]);
}

double Pixci::getExposure()
{
    char cval[5] = {0, 0, 0, 0, 0};
    double exposureTime = 0.0;
    readSerialRegister(EXPOSURE_BYTES[0], &cval[0]);
    readSerialRegister(EXPOSURE_BYTES[1], &cval[1]);
    readSerialRegister(EXPOSURE_BYTES[2], &cval[2]);
    readSerialRegister(EXPOSURE_BYTES[3], &cval[3]);
    readSerialRegister(EXPOSURE_BYTES[4], &cval[4]);

    epicsUInt64 exposureTimeCount = uCharToEpicsUInt64(cval);
    if (exposureTimeCount > 0)
    {
        exposureTime = (double(exposureTimeCount) / EXPOSURE_COUNT_TO_TIME) * SEC_TO_mS;
    }
    return exposureTime;
}

asynStatus Pixci::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
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
            int triggerMode = PR_INTERNAL_ITR;
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
    else if (function == PR_ToggleTec)
    {
        addToParamQue(function, value);
    }
    else if (function == PR_ToggleGain)
    {
        addToParamQue(function, value);
    }
    else if (function == PR_ToggleFpgaComms)
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

    return asynSuccess;
}

asynStatus Pixci::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    static const char *functionName = "writeFloat64";

    if (function == ADAcquirePeriod || function == ADAcquireTime)
    {
        addToParamQue(function, value);
    }
    else if (function == ADTemperature)
    {
        addToParamQue(function, value);
    }
    return asynSuccess;
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

asynStatus Pixci::writeSerialRegister(int unit, char Register, char val)
{
    char inputMsg[20] = {};

    /* template of message to write value to registers */
    char bufout[] = {
        static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[0]), 
        static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[1]), 
        static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[2]), 
        Register, val, 
        END_OF_TRANSMISSION_BYTE
    };

    /*writing to serial connection*/
    int inSize = writeReadSerial(UNIT, bufout, 6, inputMsg, 20);

    if (inSize < NOERROR)
    {
        return asynError;
    }

    if (inputMsg[0] == SUCCESS_MESSAGE)
    {
        return asynSuccess;
    }
    return asynError;
}

asynStatus Pixci::readSerialRegister(char Register, char *val)
{
    char inputMsg[20] = {};
    int inSize = 0;
    char first_bufout[] = {
         static_cast<char>(SINGLE_OUTPUT_BYTE_PREFIX_BYTES[0]), 
         static_cast<char>(SINGLE_OUTPUT_BYTE_PREFIX_BYTES[1]), 
         static_cast<char>(SINGLE_OUTPUT_BYTE_PREFIX_BYTES[2]), 
        Register, 
        END_OF_TRANSMISSION_BYTE
    };
    char last_bufout[] = {
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[0]), 
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[1]), 
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[2]), 
        END_OF_TRANSMISSION_BYTE
    };

    /*writing to serial connection*/
    inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 20);
    inSize = writeReadSerial(UNIT, last_bufout, sizeof(last_bufout), inputMsg, 20);

    *val = inputMsg[0];
    if (inputMsg[1] == SUCCESS_MESSAGE)
    {
        return asynSuccess;
    }
    return asynError;
}

asynStatus Pixci::readSerialRegister(char Register1, char Register2, char *val)
{
    char inputMsg[20] = {};
    int inSize = 0;
    char first_bufout[] = {
         static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[0]), 
         static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[1]), 
         static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[2]), 
        Register1, Register2, 
        END_OF_TRANSMISSION_BYTE
    };
    char last_bufout[] = {
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[0]), 
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[1]), 
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[2]), 
        END_OF_TRANSMISSION_BYTE
    };

    /*writing to serial connection*/
    inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 20);
    inSize = writeReadSerial(UNIT, last_bufout, sizeof(last_bufout), inputMsg, 20);

    *val = inputMsg[0];
    if (inputMsg[1] == SUCCESS_MESSAGE)
    {
        return asynSuccess;
    }
    return asynError;
}

asynStatus Pixci::setTriggerMode(int mode)
{
    char hexval = 0;
    switch (mode)
    {
    case PR_INTERNAL_ITR:
        hexval = INTERNAL_ITR_BYTE; // 00000100
        break;
    case PR_INTERNAL_FFR:
        hexval = INTERNAL_FFR_BYTE; // 00000110
        break;
    case PR_EXTERNAL:
        { // brackets so that trigger polarity goes out of scope after this case
            int triggerPolarity = PR_EXT_RISING_EDGE;
            getIntegerParam(PR_TriggerPolarity, &triggerPolarity);
            hexval = (triggerPolarity == PR_EXT_FALLING_EDGE) ? EXTERNAL_FALLING_EDGE_BYTE : EXTERNAL_RISING_EDGE_BYTE;
        }
        break;
    case PR_BUTTON_TRIGGER:
        hexval = CLEAR_TRIGGER_MODE_BYTE; // 00000000
        break;
    default:
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid trigger mode value %d", mode);
        return asynError;
        break;
    }
    return Pixci::writeSerialRegister(UNIT, TRIGGER_MODE_BYTE, hexval);
}

// PV Updating Functions
void Pixci::updateADTemperatureActual(bool callBackFlag)
{
    setDoubleParam(ADTemperatureActual, getTemperatureActual()); // setting the Actual Temperature PV
    if (callBackFlag)
        callParamCallbacks();
}

void Pixci::updateTemperaturePcb(bool callBackFlag)
{
    setDoubleParam(PR_TemperaturePcb, getTemperaturePcb()); // setting the PCB Temperature PV
    if (callBackFlag)
        callParamCallbacks();
}

asynStatus Pixci::updateManufacturersData(bool callBackFlag)
{
    char inputMsg[20] = {};
    int inSize = 0;
    char first_bufout[] = {
         static_cast<char>(GET_MISC_DATA_BYTES[0]), 
         static_cast<char>(GET_MISC_DATA_BYTES[1]), 
         static_cast<char>(GET_MISC_DATA_BYTES[2]), 
         static_cast<char>(GET_MISC_DATA_BYTES[3]), 
         static_cast<char>(GET_MISC_DATA_BYTES[4]), 
         static_cast<char>(GET_MISC_DATA_BYTES[5]), 
         static_cast<char>(GET_MISC_DATA_BYTES[6]), 
         static_cast<char>(GET_MISC_DATA_BYTES[7]), 
        END_OF_TRANSMISSION_BYTE
    };
    char last_bufout[] = {
         static_cast<char>(MANUFACTURER_DATA_BYTES[0]), 
         static_cast<char>(MANUFACTURER_DATA_BYTES[1]), 
         static_cast<char>(MANUFACTURER_DATA_BYTES[2]), 
        END_OF_TRANSMISSION_BYTE
    };

    INT16 serialNumber = 0;
    string buildDate = "";
    INT16 adcCountZeroDegree = 0;
    INT16 adcCountFortyDegree = 0;
    INT16 dacCountZeroDegree = 0;
    INT16 dacCountFortyDegree = 0;

    toggleFpgaComms(true);

    /*writing to serial connection*/
    inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 20);
    inSize = writeReadSerial(UNIT, last_bufout, sizeof(last_bufout), inputMsg, 20);

    toggleFpgaComms(false);

    if (inputMsg[18] == SUCCESS_MESSAGE)
    {

        serialNumber += (INT16)(unsigned char)inputMsg[0];
        serialNumber += (INT16)(unsigned char)(inputMsg[1]) << 8;
        setStringParam(ADSerialNumber, to_string(serialNumber));

        buildDate = to_string((INT16)(unsigned char)inputMsg[2]) + "/" + to_string((INT16)(unsigned char)inputMsg[3]) + "/" + to_string((INT16)(unsigned char)inputMsg[4]);
        setStringParam(PR_BuildDate, buildDate);

        adcCountZeroDegree += (INT16)(unsigned char)inputMsg[10];
        adcCountZeroDegree += (INT16)(unsigned char)(inputMsg[11]) << 8;
        setIntegerParam(PR_ADCCalibrationZeroDegree, adcCountZeroDegree);

        adcCountFortyDegree += (INT16)(unsigned char)inputMsg[12];
        adcCountFortyDegree += (INT16)(unsigned char)(inputMsg[13]) << 8;
        setIntegerParam(PR_ADCCalibrationFortyDegree, adcCountFortyDegree);

        dacCountZeroDegree += (INT16)(unsigned char)inputMsg[14];
        dacCountZeroDegree += (INT16)(unsigned char)(inputMsg[15]) << 8;
        setIntegerParam(PR_DACCalibrationZeroDegree, dacCountZeroDegree);

        dacCountFortyDegree += (INT16)(unsigned char)inputMsg[16];
        dacCountFortyDegree += (INT16)(unsigned char)(inputMsg[17]) << 8;
        setIntegerParam(PR_DACCalibrationFortyDegree, dacCountFortyDegree);

        ADC_M = 40.0f / (adcCountFortyDegree - adcCountZeroDegree);
        ADC_C = 40.0f - (ADC_M * adcCountFortyDegree);

        DAC_M = 40.0f / (dacCountFortyDegree - dacCountZeroDegree);
        DAC_C = 40.0f - (DAC_M * dacCountFortyDegree);

        if (callBackFlag)
            callParamCallbacks();
        return asynSuccess;
    }

    return asynError;
}

asynStatus Pixci::updateStatus(bool updateManufacturersDataFlag)
{
    asynStatus status = asynSuccess;
    // TODO: Get the manufacturer data and also refactor the AdcCountToCentigrade function
    if (updateManufacturersDataFlag)
        setStatIfHigher(status, updateManufacturersData()); // TODO: Implement Error Message

    updateADTemperatureActual();
    updateTemperaturePcb(true);
    return status;
}

asynStatus Pixci::updateIntialPVs()
{
    epicsInt32 sizeX = pxd_imageXdim();
    epicsInt32 sizeY = pxd_imageYdim();
    epicsFloat64 acquireFrameRate = getFrameRate();
    asynStatus status = asynSuccess;

    setStatIfHigher(status, setIntegerParam(ADMaxSizeX, sizeX));
    setStatIfHigher(status, setIntegerParam(ADMaxSizeY, sizeY));
    setStatIfHigher(status, setIntegerParam(ADSizeX, sizeX));
    setStatIfHigher(status, setIntegerParam(ADSizeY, sizeY));
    setStatIfHigher(status, setDoubleParam(ADAcquireTime, getExposure()));
    if (acquireFrameRate > 0)
    {
        setStatIfHigher(status, setDoubleParam(ADAcquirePeriod, (1 / acquireFrameRate)));
    }

    setStatIfHigher(status, callParamCallbacks());
    return status;
}

asynStatus Pixci::setRoiSizeX(int RoisizeX)
{
    char cval[2] = {0, 0};
    cval[0] = (char)((RoisizeX & 0x0F00) >> 8);
    cval[1] = (char)((RoisizeX & 0x00FF));

    writeSerialRegister(UNIT, ROI_X_SIZE_BYTES[0], cval[0]);
    return writeSerialRegister(UNIT, ROI_X_SIZE_BYTES[1], cval[1]);
}

asynStatus Pixci::setRoiSizeY(int RoisizeY)
{
    char cval[2] = {0, 0};
    cval[0] = (char)((RoisizeY & 0x0F00) >> 8);
    cval[1] = (char)((RoisizeY & 0x00FF));

    writeSerialRegister(UNIT, ROI_Y_SIZE_BYTES[0], cval[0]);
    return writeSerialRegister(UNIT, ROI_Y_SIZE_BYTES[1], cval[1]);
}

asynStatus Pixci::setRoiOffsetX(int RoiOffsetX)
{
    char cval[2] = {0, 0};
    cval[0] = (char)((RoiOffsetX & 0x0F00) >> 8);
    cval[1] = (char)((RoiOffsetX & 0x00FF));

    writeSerialRegister(UNIT, ROI_X_OFFSET_BYTES[0], cval[0]);
    return writeSerialRegister(UNIT, ROI_X_OFFSET_BYTES[1], cval[1]);
}

asynStatus Pixci::setRoiOffsetY(int RoiOffsetY)
{
    char cval[2] = {0, 0};
    cval[0] = (char)((RoiOffsetY & 0x0F00) >> 8);
    cval[1] = (char)((RoiOffsetY & 0x00FF));

    writeSerialRegister(UNIT, ROI_Y_OFFSET_BYTES[0], cval[0]);
    return writeSerialRegister(UNIT, ROI_Y_OFFSET_BYTES[1], cval[1]);
}

int Pixci::getRoiSizeX()
{
    char cval[2] = {0, 0};

    readSerialRegister(ROI_X_SIZE_BYTES[0], &cval[1]);
    readSerialRegister(ROI_X_SIZE_BYTES[1], &cval[0]);

    INT16 ival = 0;
    ival += (INT16)(unsigned char)cval[0];
    ival += (INT16)(unsigned char)(cval[1] & 0x0F) << 8;

    return ival;
}

int Pixci::getRoiSizeY()
{
    char cval[2] = {0, 0};

    readSerialRegister(ROI_Y_SIZE_BYTES[0], &cval[1]);
    readSerialRegister(ROI_Y_SIZE_BYTES[1], &cval[0]);

    INT16 ival = 0;
    ival += (INT16)(unsigned char)cval[0];
    ival += (INT16)(unsigned char)(cval[1] & 0x0F) << 8;

    return ival;
}

int Pixci::getRoiOffsetX()
{
    char cval[2] = {0, 0};

    readSerialRegister(ROI_X_OFFSET_BYTES[0], &cval[1]);
    readSerialRegister(ROI_X_OFFSET_BYTES[1], &cval[0]);

    INT16 ival = 0;
    ival += (INT16)(unsigned char)cval[0];
    ival += (INT16)(unsigned char)(cval[1] & 0x0F) << 8;

    return ival;
}

int Pixci::getRoiOffsetY()
{
    char cval[2] = {0, 0};

    readSerialRegister(ROI_Y_OFFSET_BYTES[0], &cval[1]);
    readSerialRegister(ROI_Y_OFFSET_BYTES[1], &cval[0]);

    INT16 ival = 0;
    ival += (INT16)(unsigned char)cval[0];
    ival += (INT16)(unsigned char)(cval[1] & 0x0F) << 8;

    return ival;
}

void Pixci::report(FILE *fp, int details)
{
    fprintf(fp, "Raptor detector %s\n", this->portName);
    if (details > 0)
    {
        int nx = 0;
        int ny = 0;
        int dataType = 0;
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
static const iocshArg pixciConfigArg0 = {"Port name", iocshArgString};
static const iocshArg pixciConfigArg1 = {"maxBuffers", iocshArgInt};
static const iocshArg pixciConfigArg2 = {"maxMemory", iocshArgInt};
static const iocshArg pixciConfigArg3 = {"priority", iocshArgInt};
static const iocshArg pixciConfigArg4 = {"stackSize", iocshArgInt};
static const iocshArg pixciConfigArg5 = {"camera model", iocshArgInt};
static const iocshArg pixciConfigArg6 = {"camera Format", iocshArgString};
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
                args[4].ival, args[5].ival, args[6].sval);
}

static void pixciRegister(void)
{
    iocshRegister(&configpixci, configpixciCallFunc);
}

extern "C"
{
    epicsExportRegistrar(pixciRegister);
}
