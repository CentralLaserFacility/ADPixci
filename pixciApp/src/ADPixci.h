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

#ifndef PIXCIAPP_SRC_ADPIXCI_H_
#define PIXCIAPP_SRC_ADPIXCI_H_

/* AreaDetector headers */
#include "ADDriver.h"

/* For windows */
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#include <windows.h>
#endif

#include <cstdio>

constexpr const char *SoftTriggerParamString = "PR_SOFT_TRIGGER";
constexpr const char *TriggerPolarityParamString = "PR_TRIGGER_POLARITY";
constexpr const char *UpdateInfoString = "PR_UPDATE_INFO";
constexpr const char *UpdateTemperatureString = "PR_UPDATE_TEMPERATURE";
constexpr const char *BuildDateString = "PR_BUILD_DATE";

constexpr const epicsInt32 PIXCI_NO_ERROR = 0;  // Errors are defined as integers below zero.

constexpr const char *DRIVERPARMS = "";     // Use '-QU 0' for no interrupts.
constexpr const epicsInt32 UNIT = 1;        // Unit to be selected for streaming
constexpr const epicsInt32 RESERVED  = 0;
constexpr const epicsFloat64 BAUDRATE = 115200;

constexpr const epicsBoolean BIN_AXIS_X = epicsFalse;
constexpr const epicsBoolean BIN_AXIS_Y = epicsTrue;

constexpr const epicsUInt32 PARAM_MESSAGE_QUE_SIZE = 20;
constexpr const epicsUInt32 PARAM_MESSAGE_SIZE = 16;

/**
 * @brief Set status if the returned status is higher
 */
auto setStatIfHigher = [](asynStatus *status, const asynStatus returnedStatus)
{
    *status = (*status > returnedStatus) ? *status : returnedStatus;
};

/**
 * @brief convert char to unsigned long long
 *
 * @param cval char array of size 5
 * @return unsigned long long
 */
epicsUInt64 int8ToUInt64(epicsInt8 *cval);

/**
 * @brief convert unsigned long long to char value 
 *
 * @param lval unsigned long long value
 * @param cval address of char array of size 5
 */
void uInt64ToInt8(epicsUInt64 lval, epicsInt8 *cval);

/**
 * @brief Inherited from ADDriver class which has all the parameters that all areaDetector drivers should implement.
 * parameters that are specific to the pixci frame grabber are also included in this class.
 */
class ADPixci : public ADDriver
{
 public:
    /**
     * @brief ADPixci object
     *
     * @param portName The name of the asyn port driver to be created.
     * @param maxBuffers maxBuffers The maximum number of NDArray buffer that the NDArrayPool for this
     * driver is allowed to allocate. Set this -1 to allow an unlimited number of buffers.
     * @param maxMemory maxMemory The maximum amount of memory that the NDArrayPool for this driver is
     * allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
     * @param priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynflags.
     * @param stackSize The stack size of the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
     * @param cameraModel Select camera model. Options are from ADCameraModel_t enum.
     * @param formatfile Video format configuration file location
     */
    ADPixci(const char *portName, epicsInt32 maxBuffers, size_t maxMemory, epicsInt32 priority, epicsInt32 stackSize,
        const char *cameraModel, const char *formatFile);

    /**
     * @brief Thread that waits for signal from frame grabber during live capture
     */
    void acquireTask();

    /**
     * @brief Thread that waits for parameter changes from the queue
     */
    void paramTask();

    ~ADPixci();

    /* Trigger Polarity */
    typedef enum
    {
        PRExtRisingEdge,
        PRExtFallingEdge
    } PRTriggerPolarity_t;

 protected:
    const char *driverName;

    epicsInt32 PR_SoftTrigger;
    epicsInt32 PR_TriggerPolarity;
    epicsInt32 PR_UpdateInfo;
    epicsInt32 PR_UpdateTemperature;
    epicsInt32 PR_BuildDate;

    /**
     * @brief load initial settings parameters
     *
     * @return asynStatus
     */
    asynStatus setupAquisition();

    /**
     * @brief Starts live capture image to frame buffer.
     */
    asynStatus aquireStart();

    /**
     * @brief Stops live capturing.
     */
    asynStatus acquireStop();

    /**
     * @brief write message to the camera and read the reply after that
     *
     * @param unit unit number of the camera if it supports multiple unit
     * @param serialOut output message buffer to be send to the camera
     * @param msgOutSize size of the output message size
     * @param serialIn input message buffer where message from camera is to be stored
     * @param serialInBufferSize size of input message buffer
     * @return int size of input message, return < 0 if there is an error
     */
    epicsInt32 writeReadSerial(epicsInt32 unit, char *serialOut, epicsInt32 msgOutSize, char *serialIn,
        epicsInt32 serialInBufferSize);

    /**
     * @brief add change in parameter value to the queue if it needs serial communication.
     * Serial communication takes more time. So that it is added to the queue and the change in parameters
     * is communicated by FIFO. Parameters that doesn't require serial comminication dont need to be
     * added to the queue.
     *
     * @param function function to add to the queue
     * @param value value change associated with function to add to the queue
     * @return asynStatus
     */
    asynStatus addToParamQue(epicsInt32 function, epicsInt32 value);
    asynStatus addToParamQue(epicsInt32 function, epicsFloat64 value);

    /**
     * @brief handle the parameter change from the queue
     * @param param the parameter that has to be changed
     * @param d_val the value of the parameter as a double
     * @param i_val the value of the parameter as an integer
     * @param b_val the value of the parameter as a boolean
     * @return asynStatus
     */
    virtual asynStatus handleParamTask(epicsInt32 param, epicsFloat64 d_val, epicsInt32 i_val, epicsBoolean b_val);

    virtual asynStatus updateInitialPVs();

    /**
     * @brief reload of video settings file. Change in some of the video parameters require reload of
     * video settings in order to reflect in image.
     * @param binX binning factor in x direction.
     * @param binY binning factor in y direction.
     * @param sizeX size of the image in x direction.
     * @param sizeY size of the image in y direction.
     * @return asynStatus
     */
    virtual asynStatus changeVideoFormatConfig(epicsInt32 binX, epicsInt32 binY, epicsInt32 sizeX, epicsInt32 sizeY);

    /*****************************************Methods overridden from ADDriver*****************************************/
    /**
     * @brief Overriden to implement custom write features for integers
     *
     * @param pasynUser
     * @param value
     * @return asynStatus
     */
    asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value) override;

    /**
     * @brief Overriden to implement custom write features for floats
     *
     * @param pasynUser
     * @param value
     * @return asynStatus
     */
    asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value) override;
    /******************************************************************************************************************/

#define FIRST_PIXCI_PARAM PR_SoftTrigger

 private:
    /* Event handler for acquire task */
    HANDLE g_hEvent;

    /* Queue for changing parameters that use serial communication. */
    epicsMessageQueue *paramMsgQue;

    /**
     * @brief Reloads configuration based on parameter and binning values.
     *
     * @param parameter The parameter to reload
     * @param binX The horizontal binning value
     * @param binY The vertical binning value
     * @param sizeX The horizontal size of the ROI
     * @param sizeY The vertical size of the ROI
     * @return asynStatus
     */
    asynStatus ADPixci::reloadConfiguration(epicsInt32 param, epicsInt32 binX, epicsInt32 binY, 
        epicsInt32 sizeX, epicsInt32 sizeY);

    /**********************************************Pure virtual functions**********************************************/

    /**
     * @brief update the PVs related to temperature of the camera
     */
    virtual asynStatus updateTemperatureActual() = 0;

    /**
     * @brief Readback the desired temperature for the cooling system on the camera
     *
     * @return double temperature in centigrade
     */
    virtual epicsFloat64 getCoolingSetPoint() = 0;

    /**
     * @brief Set the desired temperature for the cooling system on the camera
     *
     * @param temperature
     * @return asynStatus
     */
    virtual asynStatus setCoolingSetPoint(epicsFloat64 temperature) = 0;

    /**
     * @brief Set the Frame Rate for Internal FFR mode
     *
     * @param frameRate
     * @return asynStatus
     */
    virtual asynStatus setFrameRate(epicsFloat64 frameRate) = 0;

    /**
     * @brief Get the Frame Rate from the camera
     *
     * @return double framerate
     */
    virtual epicsFloat64 getFrameRate() = 0;

    /**
     * @brief Get the Actual Temperature from the camera
     *
     * @return double actual temperature
     */
    virtual epicsFloat64 getTemperatureActual() = 0;

    /**
     * @brief Set the Acquire Time (exposure)
     *
     * @param exposureTime
     * @return asynStatus
     */
    virtual asynStatus setExposure(epicsFloat64 exposureTime) = 0;

    /**
     * @brief Get the Aquire Time from the camera
     *
     * @return double
     */
    virtual epicsFloat64 getExposure() = 0;

    /**
     * @brief Set the ROI Size X
     *
     * @param RoisizeX
     * @return asynStatus
     */
    virtual asynStatus setRoiSizeX(epicsInt32 RoisizeX) = 0;

    /**
     * @brief Set the ROI Size Y
     *
     * @param RoisizeY
     * @return asynStatus
     */
    virtual asynStatus setRoiSizeY(epicsInt32 RoisizeY) = 0;

    /**
     * @brief Set the ROI X Offset
     *
     * @param RoiOffsetX
     * @return asynStatus
     */
    virtual asynStatus setRoiOffsetX(epicsInt32 RoiOffsetX) = 0;

    /**
     * @brief Set the ROI Y Offset
     *
     * @param RoiOffsetY
     * @return asynStatus
     */
    virtual asynStatus setRoiOffsetY(epicsInt32 RoiOffsetY) = 0;

    /**
     * @brief Get the ROI Offset Y
     *
     * @return int
     */
    virtual epicsInt32 getRoiOffsetY() = 0;

    /**
     * @brief Get the Roi Size X
     *
     * @return int
     */
    virtual epicsInt32 getRoiSizeX() = 0;

    /**
     * @brief Get the ROI Size Y
     *
     * @return int
     */
    virtual epicsInt32 getRoiSizeY() = 0;

    /**
     * @brief Get the ROI Offset X
     *
     * @return int
     */
    virtual epicsInt32 getRoiOffsetX() = 0;

    /**
     * @brief Set the shutter open delay (ms)
     * 
     * @param delayTime delay time in ms
     * @return asynStatus
     */
    virtual asynStatus setShutterOpenDelay(epicsFloat64 delayTime) = 0;

    /**
     * @brief Get the shutter open delay (ms)
     * 
     * @return epicsFloat64 delay time in ms
     */
    virtual epicsFloat64 getShutterOpenDelay() = 0;

    /**
     * @brief Set the shutter close delay (ms)
     * 
     * @param delayTime delay time in ms
     * @return asynStatus
     */
    virtual asynStatus setShutterCloseDelay(epicsFloat64 delayTime) = 0;

    /**
     * @brief Get the shutter close delay (ms)
     * 
     * @return epicsFloat64 delay time in ms
     */
    virtual epicsFloat64 getShutterCloseDelay() = 0;

    /**
     * @brief Set the Binning settings. Uses serial communication.
     *
     * @param val Binning value to set
     * @param coordinate 0 for x axis and 1 for y axis.
     * @return asynStatus
     */
    virtual asynStatus setBin(epicsInt32 val, epicsBoolean coordinate) = 0;

    /**
     * @brief Set the Trigger Mode for the image capturing
     *
     * @param mode index of the mode
     * @return asynStatus
     */
    virtual asynStatus setTriggerMode(epicsInt32 mode) = 0;

    /**
     * @brief Send a soft trigger to the camera to capture one image.
     *
     * @return asynStatus
     */
    virtual asynStatus sendSoftTrigger() = 0;

    /**
     * @brief update the status related to device
     *
     */
    virtual asynStatus updateInfo() = 0;

    /******************************************************************************************************************/
};
#endif  // PIXCIAPP_SRC_ADPIXCI_H_
