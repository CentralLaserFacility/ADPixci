/**
 * @file pixci.h
 * @author Subindev D
 * @brief This is a driver for PIXCI frame grabber from epix, inc. 
 *  Developed for Eagle XV CCD from Raptor photonics
 *  This driver will be using XCLIB Programming Library for PIXCI ® Frame Grabbers
 * @version 0.1
 * @date 2020-06-23
 * 
 */

/* AreaDetector headers */
#include "ADDriver.h"

static const char *driverName = "pixci";

/** 
 * @brief Inherited from ADDriver class which has all the parameters that all areaDetector drivers should implemented.
 * parameters that are specific to the pixci frame grabber  is also included in this class.
 */
class pixci: public ADDriver {

public:
    /**
     * @brief pixci object
     * 
     * @param portName The name of the asyn port driver to be created.
     * @param maxBuffers maxBuffers The maximum number of NDArray buffer that the NDArrayPool for this 
     * driver is allowed to allocate. Set this -1 to allow an unlimited number of buffers.      
     * @param maxMemory maxMemory The maximum amount of memory that the NDArrayPool for this driver is
     * allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
     * @param priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynflags.
     * @param stackSize The stack size of the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
     */
    pixci(const char *portName, int maxBuffers, size_t maxMemory, int priority, int stackSize);

    /* These are the methods that we override from ADDriver */
    virtual asynStatus connect(asynUser* pasynUser);
    virtual asynStatus disconnect(asynUser* pasynUser);

private:
    /**
     * @brief connect to the frame grabber
     * @return asynStatus 
     */
    asynStatus connectCamera();
    /**s
     * @brief disconnect from the frame grabber
     * @return asynStatus 
     */
    asynStatus disconnectCamera();

};