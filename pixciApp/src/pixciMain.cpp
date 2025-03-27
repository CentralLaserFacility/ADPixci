#include <iocsh.h>
#include <epicsExport.h>

#include "pixciMain.h"
#include <epicsThread.h>

/** 
 * @brief Configuration command for pixci driver; creates a new pixci object.
 * @param See the pixci.h
 */
extern "C" epicsInt32 pixciConfig(const char *portName, epicsInt32 maxBuffers, size_t maxMemory, epicsInt32 priority,
    epicsInt32 stackSize, const char *cameraModel, const char *formatFile)
{
    Pixci* drvPvt = new Pixci(portName, maxBuffers, maxMemory, priority, stackSize, cameraModel, formatFile);

    epicsThreadCreate("acquireTask", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium),
    (EPICSTHREADFUNC)acquireTaskC, drvPvt);
    epicsThreadCreate("paramTask", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium),
    (EPICSTHREADFUNC)paramTaskC, drvPvt);

    return asynSuccess;
}

static void acquireTaskC(void *drvPvt)
{
    Pixci *pPvt = reinterpret_cast<Pixci *>(drvPvt);
    pPvt->acquireTask();
}

static void paramTaskC(void *drvPvt)
{
    Pixci *pPvt = reinterpret_cast<Pixci *>(drvPvt);
    pPvt->paramTask();
}

/* pixciConfig parameters from st.cmd */
static const iocshArg pixciConfigArg0 = {"portName", iocshArgString};
static const iocshArg pixciConfigArg1 = {"maxBuffers", iocshArgInt};
static const iocshArg pixciConfigArg2 = {"maxMemory", iocshArgInt};
static const iocshArg pixciConfigArg3 = {"priority", iocshArgInt};
static const iocshArg pixciConfigArg4 = {"stackSize", iocshArgInt};
static const iocshArg pixciConfigArg5 = {"cameraModel", iocshArgString};
static const iocshArg pixciConfigArg6 = {"formatFile", iocshArgString};
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
                args[4].ival, args[5].sval, args[6].sval);
}

/* Code for iocsh registration */
static void pixciRegister(void)
{
    iocshRegister(&configpixci, configpixciCallFunc);
}

extern "C"
{
    epicsExportRegistrar(pixciRegister);
}
