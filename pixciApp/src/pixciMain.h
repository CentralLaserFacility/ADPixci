#include "pixci.h"

/**
 * @brief C Function prototypes to tie in with EPICS.
 * Runs acquire task
 * @param drvPvt
 */
static void acquireTaskC(void *drvPvt);

/**
 * @brief C Function prototypes to tie in with EPICS.
 * Runs param task
 * @param drvPvt
 */
static void paramTaskC(void *drvPvt);

/** 
 * @brief Configuration command for pixci driver; creates a new pixci object.
 * @param See the pixci.h
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