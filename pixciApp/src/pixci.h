/**
 * @brief This is a driver for PIXCI frame grabber from epix, inc.
 *  Developed for Eagle XV CCD from Raptor photonics
 *  This driver will be using XCLIB Programming Library for PIXCI ® Frame Grabbers
 */

/* AreaDetector headers */
#include "ADDriver.h"

static constexpr const char *driverName = "Pixci";

constexpr const char *SoftTriggerParamString = "PR_SOFT_TRIGGER";
constexpr const char *TriggerPolarityParamString = "PR_TRIGGER_POLARITY";
constexpr const char *UpdateTemperatureString = "PR_UPDATE_TEMPERATURE";
constexpr const char *TemperaturePCBString = "PR_TEMPERATURE_PCB";
constexpr const char *ToggleTecString = "PR_TOGGLE_TEC";
constexpr const char *ToggleGainString = "PR_TOGGLE_Gain";
constexpr const char *ToggleFPGACommsString = "PR_TOGGLE_FPGA_COMMS";
constexpr const char *UpdateStatusString = "PR_UPDATE_STATUS";
constexpr const char *BuildDateString = "PR_BUILD_DATE";
constexpr const char *ADCCalibrationZeroDegreeString = "PR_ADC_CALIBRATION_ZERO_DEGREE";
constexpr const char *ADCCalibrationFortyDegreeString = "PR_ADC_CALIBRATION_FORTY_DEGREE";
constexpr const char *DACCalibrationZeroDegreeString = "PR_DAC_CALIBRATION_ZERO_DEGREE";
constexpr const char *DACCalibrationFortyDegreeString = "PR_DAC_CALIBRATION_FORTY_DEGREE";

#define BINNING1 1
#define BINNING2 2
#define BINNING4 4
#define BINNING8 8
#define BINNING16 16
#define BINNING32 32

constexpr const epicsInt32 DETECTOR_1K = 4710; // 1056 x 1027 active pixels
constexpr const epicsInt32 DETECTOR_2K = 4240; // 2048 x 2048 active pixels

constexpr const epicsInt32 PIXCI_NO_ERROR =  0; // Errors are defined as integers below zero.

constexpr const char* FORMAT = "";      // Video format configuration name.
constexpr const char* DRIVERPARMS = ""; // Default , user '-QU 0' for not using interrupts.
constexpr const epicsInt32 UNIT = 1;        // Unit to be selected for streaming, eb1 model only have 1 unit.
constexpr const epicsInt32 RESERVED  = 0;
constexpr const epicsFloat64 BAUDRATE = 115200;

constexpr const epicsBoolean BIN_AXIS_X = epicsFalse;
constexpr const epicsBoolean BIN_AXIS_Y = epicsTrue;

constexpr const epicsUInt32 PARAM_MESSAGE_QUE_SIZE = 20;
constexpr const epicsUInt32 PARAM_MESSAGE_SIZE = 16;
constexpr const epicsFloat64 COUNT_PER_FRAME = 40e6;
constexpr const epicsFloat64 EXPOSURE_COUNT_TO_TIME = 40e6;
constexpr const epicsFloat64 SEC_TO_mS = 10e2;

constexpr const epicsInt8 SUCCESS_MESSAGE = 0x50;
constexpr const epicsInt8 END_OF_TRANSMISSION_BYTE = 0x50;

constexpr const epicsUInt8 X_BIN_BYTE = 0xA1;
constexpr const epicsUInt8 Y_BIN_BYTE = 0xA2;

constexpr const epicsUInt8 FPGA_STATUS_BYTE = 0x00;
constexpr const epicsUInt8 GET_SYSTEM_STATUS_BYTE = 0x49;
constexpr const epicsUInt8 SET_SYSTEM_STATUS_BYTE = 0x4F;

constexpr const epicsUInt8 TRIGGER_MODE_BYTE = 0xD4;
constexpr const epicsUInt8 CLEAR_TRIGGER_MODE_BYTE = 0x00;
constexpr const epicsUInt8 SOFT_TRIGGER_BYTE = 0x01;
constexpr const epicsUInt8 INTERNAL_ITR_BYTE = 0x04;
constexpr const epicsUInt8 INTERNAL_FFR_BYTE = 0x06;
constexpr const epicsUInt8 EXTERNAL_FALLING_EDGE_BYTE = 0xC0;
constexpr const epicsUInt8 EXTERNAL_RISING_EDGE_BYTE = 0x40;

constexpr const epicsUInt8 SINGLE_OUTPUT_BYTE_PREFIX_BYTES[3] = {0x53, 0xE0, 0x01};
constexpr const epicsUInt8 DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[3] = {0x53, 0xE0, 0x02};
constexpr const epicsUInt8 READ_SERIAL_PREFIX_BYTES[3] = {0x53, 0xE1, 0x01};

constexpr const epicsUInt8 GET_MISC_DATA_BYTES[8] = {0x53, 0xAE, 0x05, 0x01, 0x00, 0x00, 0x02, 0x00};
constexpr const epicsUInt8 MANUFACTURER_DATA_BYTES[3] = {0x53, 0xAF, 0x12};

constexpr const epicsUInt8 EXPOSURE_BYTES[5] = {0xED, 0xEE, 0xEF, 0xF0, 0xF1};
constexpr const epicsUInt8 FRAME_RATE_BYTES[5] = {0xDC, 0xDD, 0xDE, 0xDF, 0xE0};
constexpr const epicsUInt8 PCB_TEMPERATURE_BYTES[4] = {0X70, 0x00, 0X71, 0x00};
constexpr const epicsUInt8 CCD_SILISCON_TEMPERATURE_BYTES[4] = {0X6E, 0x00, 0X6F, 0x00};
constexpr const epicsUInt8 TEC_TEMPERATURE_BYTES[2] = {0X03, 0X04};
constexpr const epicsUInt8 ROI_X_SIZE_BYTES[2] = {0xB4, 0xB5};
constexpr const epicsUInt8 ROI_Y_SIZE_BYTES[2] = {0xB8, 0xB9};
constexpr const epicsUInt8 ROI_X_OFFSET_BYTES[2] = {0xB6, 0xB7};
constexpr const epicsUInt8 ROI_Y_OFFSET_BYTES[2] = {0xBA, 0xBB};


/* Trigger modes of Raptor Eagle-XV" */
/*ITR mode will be used to capture a continuous sequence of images.
 The camera will immediately trigger the start of a new integration period
 when the previous image readouthas completed.

 In FFR mode, the camera will generate an internal trigger signal at a user programmable frame rate.
*/
typedef enum
{
    PR_INTERNAL_ITR,
    PR_INTERNAL_FFR,
    PR_EXTERNAL,
    PR_BUTTON_TRIGGER
} PRAcquisitionMode_t;

/* Trigger Polarity */
typedef enum
{
    PR_EXT_RISING_EDGE,
    PR_EXT_FALLING_EDGE
} PR_TriggerPolarity_t;

/**
 * @brief Inherited from ADDriver class which has all the parameters that all areaDetector drivers should implement.
 * parameters that are specific to the pixci frame grabber are also included in this class.
 */
class Pixci : public ADDriver
{

public:
    /**
     * @brief Pixci object
     *
     * @param portName The name of the asyn port driver to be created.
     * @param maxBuffers maxBuffers The maximum number of NDArray buffer that the NDArrayPool for this
     * driver is allowed to allocate. Set this -1 to allow an unlimited number of buffers.
     * @param maxMemory maxMemory The maximum amount of memory that the NDArrayPool for this driver is
     * allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
     * @param priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynflags.
     * @param stackSize The stack size of the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
     * @param cameraModel Select camera model, supported values are 4240 and 4710. Default value is 4710
     * @param formatfile Video format configuration file location
     */
    Pixci(const char *portName, epicsInt32 maxBuffers, size_t maxMemory, epicsInt32 priority, epicsInt32 stackSize, epicsInt32 cameraModel, const char *formatFile);

    /* These are the methods that we override from ADDriver */
    /**
     * @brief Overriden to implement custom write features
     *
     * @param pasynUser
     * @param value
     * @return asynStatus
     */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);

    /** Reports on the properties of the attribute.
     * @param[in] fp File pointer for the report output.
     * @param[in] details Level of report details desired; currently does nothing
     */
    void report(FILE *fp, epicsInt32 details);

    /**
     * @brief thread that waits for signal from frame grabber during live capture
     *
     */
    void acquireTask(void);

    /**
     * @brief Thread that waits for parameter changes from the queue
     */
    void paramTask(void);

    ~Pixci();

protected:
    epicsInt32 PR_SoftTrigger;
    epicsInt32 PR_UpdateTemperature;
    epicsInt32 PR_TemperaturePcb;
    epicsInt32 PR_ToggleTec;
    epicsInt32 PR_ToggleGain;
    epicsInt32 PR_ToggleFpgaComms;
    epicsInt32 PR_UpdateStatus;
    epicsInt32 PR_BuildDate;
    epicsInt32 PR_ADCCalibrationZeroDegree;
    epicsInt32 PR_ADCCalibrationFortyDegree;
    epicsInt32 PR_DACCalibrationZeroDegree;
    epicsInt32 PR_DACCalibrationFortyDegree;
    epicsInt32 PR_TriggerPolarity;

#define FIRST_PIXCI_PARAM PR_SoftTrigger

private:
    epicsFloat32 ADC_M; // ADC Slope
    epicsFloat32 ADC_C; // ADC Offset
    epicsFloat32 DAC_M; // DAC Slope
    epicsFloat32 DAC_C; // DAC Offset
    epicsInt32 cameraModel;
    
    /* Event handler for acquire task */
    HANDLE g_hEvent;

    /**
     * @brief starts live capture image to frame buffer.
     */
    asynStatus acquireImage(void);
    /**
     * @brief Stops live capturing.
     */
    asynStatus acquireStop(void);
    /**
     * @brief write serial command to the camera connected.
     *
     * @param unit
     * @param serialOut serial command to be send to the camera
     * @return asynStatus
     */
    asynStatus writeSerial(epicsInt32 unit, epicsUInt8 *serialOut, epicsInt32 msgSize);

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
    epicsInt32 writeReadSerial(epicsInt32 unit, char *serialOut, epicsInt32 msgOutSize, char *serialIn, epicsInt32 serialInBufferSize);

    /**
     * @brief load initial settings parameters
     *
     * @return asynStatus asynSuccess or asynError
     */
    asynStatus setupAquisition();

    /**
     * @brief reload of video settings file. Change in some of the video parameters require reload of
     * video settings in order to reflect in image.
     *
     */
    void reloadVideoSettings();

    /**
     * @brief reset video settings file to the default. minX , sizeX parameter changes require reset
     * video settings in order to reflect in image.
     *
     */
    void resetVideoSettings();

    /**
     * @brief read camera registers over serial communication.
     *
     * @param reg register address to be read
     * @param val returned value
     * @return status, asynSuccess if read was successfull , else asynError
     */
    asynStatus readSerialRegister(epicsUInt8 Register, epicsUInt8 *val);

    /**
     * @brief read camera 2 bytes registers over serial communication
     *
     * @param Register1 first register address to be read
     * @param Register2 second register address to be read
     * @param val returned value
     * @return status, asynSuccess if read was successfull , else asynError
     */
    asynStatus readSerialRegister(epicsUInt8 Register1, epicsUInt8 Register2, epicsUInt8 *val);

    /**
     * @brief Set the Binning settings. Uses serial communication.
     *
     * @param val Binning value to set
     * @param coordinate 0 for x axis and 1 for y axis.
     * @return asynStatus
     */
    asynStatus setBin(epicsInt32 val, epicsBoolean coordinate);

    /**
     * @brief queue for changing parameters that use serial communication.
     */
    epicsMessageQueue *paramMsgQue;

    /**
     * @brief add change in parameter value to the queue if it needs serial communication.
     * Serial communication takes more time. So that it is added to the queue and the change in parameters
     * is communicated by FIFO. Parameters that doesn't require serial comminication dont need to be
     * added to the queue.
     *
     * @param function
     * @param value
     */
    void addToParamQue(epicsInt32 function, epicsInt32 value);
    void addToParamQue(epicsInt32 function, epicsFloat64 value);

    /**
     * @brief write value to the registers of the camera using serial command, might take longer
     * time to execute. Advised to run in seperate thread.
     *
     * @param unit
     * @param Register register number , where value has to be written
     * @param val value to be written in the register
     * @return asynStatus
     */
    asynStatus writeSerialRegister(epicsInt32 unit, epicsUInt8 Register, epicsUInt8 val);

    /**
     * @brief Set the Trigger Mode for the image capturing
     *
     * @param mode index of the mode,
     * 0 = internal itr mode
     * 1 = internal ffr mode
     * 2 = External mode
     * 3 = Button (software trigger mode)
     * @return asynStatus
     */
    asynStatus setTriggerMode(epicsInt32 mode);

    /**
     * @brief Send a soft trigger to the camera to capture one image.
     *
     * @return asynStatus
     */
    asynStatus Pixci::sendSoftTrigger();

    /**
     * @brief Set the Frame Rate for Internal FFR mode
     *
     * @param frameRate
     * @return asynStatus
     */
    asynStatus setFrameRate(epicsFloat64 frameRate);

    /**
     * @brief Set the TEC Temperature
     *
     * @param temperature
     * @return asynStatus
     */
    asynStatus setTecTemperature(epicsFloat64 temperature);

    /**
     * @brief Set the system status of the camera
     *
     * @param val
     * Bit 7,5,3 = Reserved
     * Bit 6 = 1 check sum mode enabled
     * Bit 4 = 1 to enable command ACK
     * Bit 2 = 1 if FPGA booted ok
     * Bit 1 = 0 to Hold FPGA in RESET
     * Bit 0 = 1 to enable comms to FPGA EPROM
     * @return asynStatus
     */
    asynStatus setSystemStatus(epicsUInt8 val);

    /**
     * @brief Enable/ Disable TEC controller
     *
     * @param enableTec
     * @return asynStatus
     */
    asynStatus toggleTec(epicsBoolean enableTec);

    /**
     * @brief Enable/Disable Pre-Amp Gain
     *
     * @param enableGain
     * @return asynStatus
     */
    asynStatus toggleGain(epicsBoolean enableGain);

    /**
     * @brief Enable/Disable comms to FPGA EPROM
     *
     * @param enableFpgaComms
     * @return asynStatus
     */
    asynStatus toggleFpgaComms(epicsBoolean enableFpgaComms);

    /**
     * @brief Get the Frame Rate from the camera
     *
     * @return double framerate
     */
    epicsFloat64 getFrameRate();

    /**
     * @brief Get the Actual Temperature from the camera
     *
     * @return double actual temperature
     */
    epicsFloat64 getTemperatureActual();

    /**
     * @brief Get the PCB Temperature from the camera
     *
     * @return double PCB temperature
     */
    epicsFloat64 getTemperaturePcb();

    /**
     * @brief Get the Tec Temperature from the camera
     *
     * @return double TEC temperature in centigrade
     */
    epicsFloat64 getTecTemperature();

    /**
     * @brief Get the FPGA Status from camera
     *
     * @return unsigned char return 1 byte of FPGA flags
     * Bit 7 = 0 to enable high pre amp gain
     * Bit 6,5,4,3,2 = reserved (Default=0)
     * Bit 1 = 1 OverTemp >80°C tripped (Default=0)
     * Bit 0 = 1 to enable TEC (Default=0)
     */
    epicsUInt8 getFpgaStatus();

    /**
     * @brief Get the System Status from camera
     *
     * @return unsigned char 1 byte returned from camera
     * Bit 7,5,3 = Reserved
     * Bit 6 = 1 check sum mode enabled
     * Bit 4 = 1 to enable command ACK
     * Bit 2 = 1 if FPGA booted ok
     * Bit 1 = 0 to Hold FPGA in RESET
     * Bit 0 = 1 to enable comms to FPGA EPROM
     */
    epicsUInt8 getSystemStatus();

    /**
     * @brief Get the TEC enable status
     *
     * @return true - TEC Enabled
     * @return false - TEC Disabled
     */
    epicsBoolean isTecEnabled();

    /**
     * @brief Get the Pre-Amp Gain enable status
     *
     * @return true = Pre Amp Gain Enabled
     * @return false = Pre Amp Gain Disabled
     */
    epicsBoolean isGainEnabled();

    /**
     * @brief Get the comms to FPGA EPROM enable status
     *
     * @return true  = comms to FPGA EPROM Enabled
     * @return false = comms to FPGA EPROM Disabled
     */
    epicsBoolean isFpgaCommsEnabled();

    /**
     * @brief Set the ROI Size X
     *
     * @param RoisizeX
     * @return asynStatus
     */
    asynStatus setRoiSizeX(epicsInt32 RoisizeX);

    /**
     * @brief Set the ROI Size Y
     *
     * @param RoisizeY
     * @return asynStatus
     */
    asynStatus setRoiSizeY(epicsInt32 RoisizeY);

    /**
     * @brief Set the ROI X Offset
     *
     * @param RoiOffsetX
     * @return asynStatus
     */
    asynStatus setRoiOffsetX(epicsInt32 RoiOffsetX);

    /**
     * @brief Set the ROI Y Offset
     *
     * @param RoiOffsetY
     * @return asynStatus
     */
    asynStatus setRoiOffsetY(epicsInt32 RoiOffsetY);

    /**
     * @brief Get the Roi Size X
     *
     * @return int
     */
    epicsInt32 getRoiSizeX();

    /**
     * @brief Get the ROI Size Y
     *
     * @return int
     */
    epicsInt32 getRoiSizeY();

    /**
     * @brief Get the ROI Offset X
     *
     * @return int
     */
    epicsInt32 getRoiOffsetX();

    /**
     * @brief Get the ROI Offset Y
     *
     * @return int
     */
    epicsInt32 getRoiOffsetY();

    /**
     * @brief convert unsigned char to unsigned long long
     *
     * @param cval char array of size 5
     * @return unsigned long long
     */
    epicsUInt64 uCharToEpicsUInt64(epicsUInt8 *cval);

    /**
     * @brief convert unsigned char value to unsigned long long
     *
     * @param lval unsigned long long value
     * @param cval address of unsigned char array of size 5
     */
    void epicsUInt64ToUChar(epicsUInt64 lval, epicsUInt8 *cval);

    /**
     * @brief Set the Acquire Time (exposure)
     *
     * @param exposureTime
     * @return asynStatus
     */
    asynStatus setExposure(epicsFloat64 exposureTime);

    /**
     * @brief Get the Aquire Time from the camera
     *
     * @return double
     */
    epicsFloat64 getExposure();

    /**
     * @brief Convert the ADC Count to the temperature in centigrade
     *
     * @param adcCount ADC count value
     * @return double temperature in centigrade
     */
    epicsFloat64 convertAdcCountToCentigrade(epicsInt16 adcCount);

    /**
     * @brief Convert the temperature in centigrade to DAC Count
     *
     * @param temperature Temperature in centigrade
     * @return unsigned INT16 output DAC count
     */
    epicsInt16 convertCentigradeToDacCount(epicsFloat64 temperature);

    /**
     * @brief Convert the DAC count to temperature in centigrade
     *
     * @param dacCount DAC count
     * @return double temperature in centigrade
     */
    epicsFloat64 convertDacCountToCentigrade(epicsInt16 dacCount);

    // PV Updating Functions

    /**
     * @brief update the PV ADTemperatureActual
     *
     * @param callBackFlag Flag for calling the callParamCallbacks function
     */
    void updateADTemperatureActual(epicsBoolean callBackFlag = epicsFalse);

    /**
     * @brief update the PV TemperaturePCB
     *
     * @param callBackFlag Flag for calling the callParamCallbacks function
     */
    void updateTemperaturePcb(epicsBoolean callBackFlag = epicsFalse);

    /**
     * @brief update the PVs related to manufacturers data
     *
     * @param callBackFlag Flag for calling the callParamCallbacks function
     */
    asynStatus updateManufacturersData(epicsBoolean callBackFlag = epicsFalse);

    /**
     * @brief update the status related to device
     *
     */
    asynStatus updateStatus();

    asynStatus updateIntialPVs();
};