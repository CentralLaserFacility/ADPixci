/**
 * @brief This is a driver for EPIX inc. PIXCI camera link frame grabbers
 *  This driver uses the PIXCI ® XCLIB Programming Library.
 *  This class defines specific instructions for Raptor Photonics Eagle XV II 
 *  X-ray CCD cameras.
 * 
 * @copyright Copyright (c) 2025, UKRI STFC Central Laser Facility
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 * 
 *  1. Redistributions of source code must retain the above copyright notice, this
 *  list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the names of its
 *  contributors may be used to endorse or promote products derived from
 *  this software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

    if (strcmp(cameraModel, RAPTOR_EAGLE_XV_4710) == 0 || strcmp(cameraModel, RAPTOR_EAGLE_XV_4240) == 0)
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
static const iocshFuncDef pixciIocshFuncDef = {"pixciConfig", 7, pixciConfigArgs};
static void pixciConfigIocshWrapper(const iocshArgBuf *args)
{
    pixciConfig(args[0].sval, args[1].ival, args[2].ival, args[3].ival,
                args[4].ival, args[5].sval, args[6].sval);
}

/* Code for iocsh registration */
static void pixciRegister(void)
{
    iocshRegister(&pixciIocshFuncDef, pixciConfigIocshWrapper);
}

extern "C"
{
    epicsExportRegistrar(pixciRegister);
}
