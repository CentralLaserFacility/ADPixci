#include "ADPixci.h"
#include <map>

/**
 * @brief Types of camera models supported by the driver.
 */
typedef enum ADCameraModel_t
{
    RaptorEagleXV_4710,  // 1056 x 1027 active pixels
    RaptorEagleXV_4240,  // 2048 x 2048 active pixels
};

std::map<ADCameraModel_t, std::string> cameraModelMap = {
    {ADCameraModel_t::RaptorEagleXV_4710, "RaptorEagleXV_4710"},
    {ADCameraModel_t::RaptorEagleXV_4240, "RaptorEagleXV_4240"},
};

std::string cameraModelToString(ADCameraModel_t cameraModel);

/**
 * @brief C Function prototypes to tie in with EPICS.
 * Runs acquire task
 * @param drvPvt pointer to the ADPixci object
 */
static void acquireTaskC(void *drvPvt);

/**
 * @brief C Function prototypes to tie in with EPICS.
 * Runs param task
 * @param drvPvt pointer to the ADPixci object
 */
static void paramTaskC(void *drvPvt);

/** 
 * @param portName The name of the asyn port driver to be created.
 * @param maxBuffers maxBuffers The maximum number of NDArray buffer that the NDArrayPool for this
 * driver is allowed to allocate. Set this -1 to allow an unlimited number of buffers.
 * @param maxMemory maxMemory The maximum amount of memory that the NDArrayPool for this driver is
 * allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
 * @param priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynflags.
 * @param stackSize The stack size of the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
 * @param cameraModel Select camera model. Options are from ADCameraModel_t enum.
 * @param formatfile Video format configuration file location
 * @return epicsInt32 status result from instantiation of the driver
 */
extern "C" epicsInt32 pixciConfig(const char *portName, epicsInt32 maxBuffers, size_t maxMemory, epicsInt32 priority,
    epicsInt32 stackSize, const char *cameraModel, const char *formatFile);

/**
 * @brief wrapper function for pixciConfig to be called from iocsh
 * @param args arguments from iocsh
 */
static void configpixciCallFunc(const iocshArgBuf *args);

/**
 * @brief Register the configpixciCallFunc function with iocsh
 */
static void pixciRegister(void);