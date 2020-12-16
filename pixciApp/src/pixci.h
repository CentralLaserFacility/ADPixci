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

/* Trigger modes of Raptor Eagle-XV" */
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
  int PR_TriggerPolarity;

private:
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
     * @brief read camera registers over serial communication.
     * 
     * @param reg register address to be read
     * @return status, asynSuccess if read was successfull , else asynError
     */
    asynStatus readSerialRegister(char Register, char* val);

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
     * @brief Get the Frame Rate from the camera
     * 
     * @return double framerate 
     */
    double getFrameRate();

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

};