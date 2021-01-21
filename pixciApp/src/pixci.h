/**
 * @brief This is a driver for PIXCI frame grabber from epix, inc. 
 *  Developed for Eagle XV CCD from Raptor photonics
 *  This driver will be using XCLIB Programming Library for PIXCI ® Frame Grabbers
 */

/* AreaDetector headers */
#include "ADDriver.h"

static const char *driverName = "Pixci";

#define SoftTriggerParamString "PR_SOFT_TRIGGER"
#define TriggerPolarityParamString "PR_TRIGGER_POLARITY"
#define UpdateTemperatureString "PR_UPDATE_TEMPERATURE"
#define TemperaturePCBString "PR_TEMPERATURE_PCB"
#define ToggleTecString "PR_TOGGLE_TEC"
#define ToggleGainString "PR_TOGGLE_Gain" 
#define ToggleFPGACommsString "PR_TOGGLE_FPGA_COMMS"
#define UpdateStatusString "PR_UPDATE_STATUS" 
#define BuildDateString "PR_BUILD_DATE"
#define ADCCalibrationZeroDegreeString "PR_ADC_CALIBRATION_ZERO_DEGREE"
#define ADCCalibrationFortyDegreeString "PR_ADC_CALIBRATION_FORTY_DEGREE"
#define DACCalibrationZeroDegreeString "PR_DAC_CALIBRATION_ZERO_DEGREE"
#define DACCalibrationFortyDegreeString "PR_DAC_CALIBRATION_FORTY_DEGREE"

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
}PR_TriggerPolarity_t;


/** 
 * @brief Inherited from ADDriver class which has all the parameters that all areaDetector drivers should implemented.
 * parameters that are specific to the pixci frame grabber  is also included in this class.
 */
class Pixci: public ADDriver {

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
     * @param formatfile Video format configuration file location
     */
    Pixci(const char *portName, int maxBuffers, size_t maxMemory, int priority, int stackSize, const char *formatfile);

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
    
    /**
     * @brief thread that waits for signal from frame grabber during live capture
     * 
     */
    void acquireTask(void);

    /**
     * @brief Thread that wait for parameter changes from the que
     */
    void paramTask(void);

    ~Pixci();

protected:
  int PR_SoftTrigger; 
  #define FIRST_PIXCI_PARAM PR_SoftTrigger
  int PR_UpdateTemperature;
  int PR_TemperaturePcb;
  int PR_ToggleTec;
  int PR_ToggleGain;
  int PR_ToggleFpgaComms;
  int PR_UpdateStatus;
  int PR_BuildDate;
  int PR_ADCCalibrationZeroDegree;
  int PR_ADCCalibrationFortyDegree;
  int PR_DACCalibrationZeroDegree;
  int PR_DACCalibrationFortyDegree;
  int PR_TriggerPolarity;  

private:

  float ADC_M; //ADC Slope
  float ADC_C; //ADC Offset
  float DAC_M; //DAC Slope
  float DAC_C; //DAC Offset      

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
    asynStatus writeSerial(int unit, char* serialOut, int msgSize);


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
    int writeReadSerial(int unit, char* serialOut, int msgOutSize, char* serialIn, int serialInBufferSize);

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
    asynStatus readSerialRegister(char Register, char* val);

    /**
     * @brief read camera 2 bytes registers over serial communication
     * 
     * @param Register1 first register address to be read
     * @param Register2 second register address to be read
     * @param val returned value
     * @return status, asynSuccess if read was successfull , else asynError 
     */
    asynStatus readSerialRegister(char Register1, char Register2, char *val);

    /**
     * @brief Set the Binning settings. Uses serial communication. 
     * 
     * @param val Binning value to set
     * @param coordinate 0 for x axis and 1 for y axis.
     * @return asynStatus 
     */
    asynStatus setBin(int val, bool coordinate);

    /**
     * @brief que for changing parameters that uses serial communication.
     */
    epicsMessageQueue *paramMsgQue;

    /**
     * @brief add change in parameter value to the que if it needs serial communication.
     * Serial communication takes more times. So that it is added to the que and the change in parameters
     * is communicated by FIFO. Parameters that doesn't require serial comminication dont need to be
     * added to the que.
     * 
     * @param function 
     * @param value 
     */
    void addToParamQue(int function, int value);
    void addToParamQue(int function, epicsFloat64 value);

    /**
     * @brief write value to the registers of the camera using serial command, might take longer
     * time to execute. Advised to run in seperate thread.
     * 
     * @param unit 
     * @param Register register number , where value has to be written
     * @param val value to be written in the register
     * @return asynStatus 
     */
    asynStatus writeSerialRegister(int unit, char Register, char val);

    /**
     * @brief Get the Trigger Status of camera
     * 
     * @return asynStatus 
     */
    asynStatus getTriggerStatus();
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
    asynStatus setTriggerMode(int mode);

    /**
     * @brief Set the Frame Rate for Internal FFR mode
     * 
     * @param frameRate 
     * @return asynStatus 
     */
    asynStatus setFrameRate(double frameRate);

    /**
     * @brief Set the TEC Temperature 
     * 
     * @param temperature 
     * @return asynStatus 
     */
    asynStatus setTecTemperature(double temperature);

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
    asynStatus setSystemStatus(char val);

    /**
     * @brief Enable/ Disable TEC controller
     * 
     * @param enableTec 
     * @return asynStatus 
     */
    asynStatus toggleTec(bool enableTec);

    /**
     * @brief Enable/Disable Pre-Amp Gain
     * 
     * @param enableGain 
     * @return asynStatus 
     */
    asynStatus toggleGain(bool enableGain);

    /**
     * @brief Enable/Disable comms to FPGA EPROM
     * 
     * @param enableFpgaComms 
     * @return asynStatus 
     */
    asynStatus toggleFpgaComms(bool enableFpgaComms);

    /**
     * @brief Get the Frame Rate from the camera
     * 
     * @return double framerate 
     */
    double getFrameRate();

   /**
    * @brief Get the Actual Temperature from the camera
    * 
    * @return double actual temperature
    */
    double getTemperatureActual();

    /**
     * @brief Get the PCB Temperature from the camera
     * 
     * @return double PCB temperature
     */
    double getTemperaturePcb();

    /**
     * @brief Get the Tec Temperature from the camera
     * 
     * @return double TEC temperature in centigrade
     */
    double getTecTemperature();

    /**
     * @brief Get the FPGA Status from camera
     * 
     * @return unsigned char return 1 byte of FPGA flags
     * Bit 7 = 0 to enable high pre amp gain
     * Bit 6,5,4,3,2 = reserved (Default=0)
     * Bit 1 = 1 OverTemp >80°C tripped (Default=0)
     * Bit 0 = 1 to enable TEC (Default=0)
     */
    unsigned char getFpgaStatus();

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
    unsigned char getSystemStatus();

    /**
     * @brief Get the TEC enable status
     * 
     * @return true - TEC Enabled 
     * @return false - TEC Disabled 
     */
    bool isTecEnabled();

    /**
     * @brief Get the Pre-Amp Gain enable status
     * 
     * @return true = Pre Amp Gain Enabled 
     * @return false = Pre Amp Gain Disabled
     */
    bool isGainEnabled();

    /**
     * @brief Get the comms to FPGA EPROM enable status
     * 
     * @return true  = comms to FPGA EPROM Enabled
     * @return false = comms to FPGA EPROM Disabled
     */
    bool isFpgaCommsEnabled();

    /**
     * @brief Set the ROI Size X 
     * 
     * @param RoisizeX 
     * @return asynStatus 
     */
    asynStatus setRoiSizeX(int RoisizeX); 

    /**
     * @brief Set the ROI Size Y
     * 
     * @param RoisizeY 
     * @return asynStatus 
     */
    asynStatus setRoiSizeY(int RoisizeY);

    /**
     * @brief Set the ROI X Offset 
     * 
     * @param RoiOffsetX 
     * @return asynStatus 
     */
    asynStatus setRoiOffsetX(int RoiOffsetX); 
    
    /**
     * @brief Set the ROI Y Offset 
     * 
     * @param RoiOffsetY 
     * @return asynStatus 
     */
    asynStatus setRoiOffsetY(int RoiOffsetY);

    /**
     * @brief Get the Roi Size X 
     * 
     * @return int 
     */
    int getRoiSizeX();

    /**
     * @brief Get the ROI Size Y 
     * 
     * @return int 
     */
    int getRoiSizeY();

    /**
     * @brief Get the ROI Offset X 
     * 
     * @return int 
     */
    int getRoiOffsetX();

    /**
     * @brief Get the ROI Offset Y 
     * 
     * @return int 
     */
    int getRoiOffsetY();
    
    /**
     * @brief convert unsigned char to unsigned long long
     * 
     * @param cval char array of size 5
     * @return unsigned long long 
     */
    unsigned long long UcharToLong( char* cval);

    /**
     * @brief convert unsigned char value to unsigned long long
     * 
     * @param lval unsigned long long value 
     * @param cval address of unsigned char array of size 5
     */
    void longTouchar(long lval, char* cval);
    
    
    /**
     * @brief Set the Acquire Time (exposure)
     * 
     * @param exposureTime 
     * @return asynStatus 
     */
    asynStatus setExposure(double exposureTime);

    /**
     * @brief Get the Aquire Time from the camera
     * 
     * @return double 
     */
    double getExposure();

    /**
     * @brief Convert the ADC Count to the temperature in centigrade
     * 
     * @param adcCount ADC count value
     * @return double temperature in centigrade
     */
    double convertAdcCountToCentigrade(INT16 adcCount);

    /**
     * @brief Convert the temperature in centigrade to DAC Count
     * 
     * @param temperature Temperature in centigrade
     * @return unsigned INT16 output DAC count
     */
    INT16 convertCentigradeToDacCount(double temperature) ;

    /**
     * @brief Convert the DAC count to temperature in centigrade
     * 
     * @param dacCount DAC count
     * @return double temperature in centigrade
     */
    double convertDacCountToCentigrade(INT16 dacCount);

    // PV Updating Functions

    /**
     * @brief update the PV ADTemperatureActual
     * 
     * @param callBackFlag Flag for calling the callParamCallbacks function
     */
    void updateADTemperatureActual(bool callBackFlag = false);
    
    /**
     * @brief update the PV TemperaturePCB
     * 
     * @param callBackFlag Flag for calling the callParamCallbacks function
     */
    void updateTemperaturePcb(bool callBackFlag = false);

    /**
     * @brief update the PVs related to manufacturers data
     * 
     * @param callBackFlag Flag for calling the callParamCallbacks function
     */
    asynStatus updateManufacturersData(bool callBackFlag = false);

    /**
     * @brief update the status related to device
     * 
     */
    void updateStatus(bool updateManufacturersDataFlag = false);

    asynStatus updateIntialPVs();
};