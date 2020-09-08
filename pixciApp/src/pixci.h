/**
 * @brief This is a driver for PIXCI frame grabber from epix, inc. 
 *  Developed for Eagle XV CCD from Raptor photonics
 *  This driver will be using XCLIB Programming Library for PIXCI ® Frame Grabbers
 */

/* AreaDetector headers */
#include "ADDriver.h"

static const char *driverName = "Pixci";

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
    
    /**
     * @brief thread that waits for signal from frame grabber during live capture
     * 
     */
    void acquireTask(void);
    ~Pixci();
private:
    /**
     * @brief starts live capture image to frame buffer.
     */
    void acquireImage(void);
    /**
     * @brief Stops live capturing.
     */
    void acquireStop(void);
    /**
     * @brief write serial command to the camera connected.
     * 
     * @param unit 
     * @param serialOut serial command to be send to the camera
     * @return asynStatus 
     */
    asynStatus writeSerial(int unit, char* serialOut, int msgSize);

    /**
     * @brief write value to the registers of the camera using serial command
     * 
     * @param unit 
     * @param Register register number , where value has to be written
     * @param val value to be written in the register
     * @return asynStatus 
     */
    asynStatus writeSerialRegister(int unit, char Register, char val);

    int readSerial(int unit, char* serialIn);
    int writeReadSerial(int unit, char* serialOut, int serialOutBufferSize, int msgOutSize, char* serialIn, int serialInBufferSize);
    asynStatus readSerialRegister(int unit, int value);
    void setBin(int val);
    int getBin();
    epicsMessageQueue *pCallbackMsgQ_;
};