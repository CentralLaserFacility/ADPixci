#include <iocsh.h>
#include <epicsExport.h>
#include <epicsThread.h>
#include <epicsStdio.h>

#include "pixciMain.h"
#include "ADRaptorEagleXV.h"

extern "C" epicsInt32 pixciConfig(const char *portName, epicsInt32 maxBuffers, size_t maxMemory, epicsInt32 priority,
    epicsInt32 stackSize, const char *cameraModel, const char *formatFile)
{

    ADPixci* drvPvt = nullptr;

    if (cameraModel == RAPTOR_EAGLE_XV_4710 || cameraModel == RAPTOR_EAGLE_XV_4240)
    {
        try
        {
            drvPvt = new ADRaptorEagleXV(portName, maxBuffers, maxMemory, priority, stackSize, cameraModel, formatFile);
        }
        catch(const std::runtime_error& e)
        {  
            epicsStdoutPrintf("%s\n", e.what());
            return asynError;
        }
    }
    else
    {
        epicsStdoutPrintf("Camera model %s not supported\n", cameraModel);
        return asynError;
    }

    epicsThreadCreate("acquireTask", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium),
    (EPICSTHREADFUNC)acquireTaskC, drvPvt);
    epicsThreadCreate("paramTask", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium),
    (EPICSTHREADFUNC)paramTaskC, drvPvt);

    return asynSuccess;
}

static void acquireTaskC(void *drvPvt)
{
    ADPixci *pPvt = reinterpret_cast<ADPixci *>(drvPvt);
    pPvt->acquireTask();
}

static void paramTaskC(void *drvPvt)
{
    ADPixci *pPvt = reinterpret_cast<ADPixci *>(drvPvt);
    pPvt->paramTask();
}

static void pixciConfigIocshWrapper(const iocshArgBuf *args)
{
    pixciConfig(args[0].sval, args[1].ival, args[2].ival, args[3].ival,
                args[4].ival, args[5].sval, args[6].sval);
}

/* Code for iocsh registration */
static void pixciRegister(void)
{
    /* pixciConfig parameters from st.cmd */
    static const iocshArg pixciConfigArg0 = {"portName", iocshArgString};
    static const iocshArg pixciConfigArg1 = {"maxBuffers", iocshArgInt};
    static const iocshArg pixciConfigArg2 = {"maxMemory", iocshArgInt};
    static const iocshArg pixciConfigArg3 = {"priority", iocshArgInt};
    static const iocshArg pixciConfigArg4 = {"stackSize", iocshArgInt};
    static const iocshArg pixciConfigArg5 = {"cameraModel", iocshArgString};
    static const iocshArg pixciConfigArg6 = {"formatFile", iocshArgString};
    static const iocshArg *const pixciConfigArgs[] = {
        &pixciConfigArg0,
        &pixciConfigArg1,
        &pixciConfigArg2,
        &pixciConfigArg3,
        &pixciConfigArg4,
        &pixciConfigArg5,
        &pixciConfigArg6
    };
    static const iocshFuncDef pixciIocshFuncDef = {"pixciConfig", 7, pixciConfigArgs};
    iocshRegister(&pixciIocshFuncDef, pixciConfigIocshWrapper);
}

extern "C"
{
    epicsExportRegistrar(pixciRegister);
}
