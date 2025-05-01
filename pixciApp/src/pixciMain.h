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

#ifndef PIXCIAPP_SRC_PIXCIMAIN_H_
#define PIXCIAPP_SRC_PIXCIMAIN_H_

#include "ADPixci.h"

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
static void pixciConfigIocshWrapper(const iocshArgBuf *args);

/**
 * @brief Register the pixciConfigIocshWrapper function with iocsh
 */
static void pixciRegister(void);

#endif  // PIXCIAPP_SRC_PIXCIMAIN_H_
