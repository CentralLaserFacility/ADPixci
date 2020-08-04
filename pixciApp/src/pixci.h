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
     */
    Pixci(const char *portName, int maxBuffers, size_t maxMemory, int priority, int stackSize);

    /* These are the methods that we override from ADDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    
    // Should be private, but are called from C so must be public
    void acquireTask(void);
    ~Pixci();
private:
    /**
     * @brief Live capture image to frame buffer.
     */
    void acquireImage(void);
    /**
     * @brief Stop live capturing.
     */
    void acquireStop(void);
    

};