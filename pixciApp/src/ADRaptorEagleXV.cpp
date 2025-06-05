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

#include "ADRaptorEagleXV.h"

#include <string>

/* ADPixci headers
 source: http://www.epixinc.com/products/xclib.htm
 XCLW64 .dll and .lib files should be included for windows-64 OS
 XCLIBNT .dll and .lib files should be inlcuded for win32 OS
 xclib_x86_64 .so and .a files should be included for linux_x86_64 OS
 xclib_i386 .so and .a files should be included for linux_x86 OS
*/
extern "C"
{
#include "xcliball.h"
}

ADRaptorEagleXV::ADRaptorEagleXV(const char *portName, epicsInt32 maxBuffers, size_t maxMemory, epicsInt32 priority,
    epicsInt32 stackSize, const char *cameraModel, const char *formatFile)
    : ADPixci(portName, maxBuffers, maxMemory, priority, stackSize, cameraModel, formatFile)
{
    asynStatus status = asynSuccess;
    this->driverName = "ADRaptorEagleXV";

    status = setStringParam(ADManufacturer, "Raptor Photonics");
    if (status != asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to set manufacturer name");
    }
    setStatIfHigher(&status, createParam(TemperaturePCBString, asynParamFloat64, &PR_TemperaturePCB));
    setStatIfHigher(&status, createParam(ToggleTecString, asynParamInt32, &PR_ToggleTec));
    setStatIfHigher(&status, createParam(ToggleGainString, asynParamInt32, &PR_ToggleGain));
    setStatIfHigher(&status, createParam(ToggleFPGACommsString, asynParamInt32, &PR_ToggleFpgaComms));
    setStatIfHigher(&status, createParam(ADCCalibrationZeroDegreeString, asynParamInt32, &PR_ADCCalibrationZeroDegree));
    setStatIfHigher(&status, createParam(ADCCalibrationFortyDegreeString, asynParamInt32, &PR_ADCCalibrationFortyDegree));
    setStatIfHigher(&status, createParam(DACCalibrationZeroDegreeString, asynParamInt32, &PR_DACCalibrationZeroDegree));
    setStatIfHigher(&status, createParam(DACCalibrationFortyDegreeString, asynParamInt32, &PR_DACCalibrationFortyDegree));
    if (status > asynSuccess)
    {   // Parameter initialization failed
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Failed to create parameters\n", driverName);
        setIntegerParam(ADStatus, ADStatusError);
        setStringParam(ADStatusMessage, "Cannot create parameters");
        throw std::runtime_error("Failed to create parameters");
    }
    // Updating all the PVs related to the status of device and the manufacturers data
    status = this->updateInitialPVs();
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Failed to update initial PVs\n", driverName);
        setIntegerParam(ADStatus, ADStatusError);
        setStringParam(ADStatusMessage, "Failed to update initial PVs");
        throw std::runtime_error("Failed to update initial PVs");
    }
}

asynStatus ADRaptorEagleXV::writeSerialRegister(epicsInt32 unit, epicsInt8 Register, epicsInt8 val)
{
    char inputMsg[20] = {};

    /* template of message to write value to registers */
    char bufout[] = {
        static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[0]),
        static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[1]),
        static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[2]),
        Register, val,
        END_OF_TRANSMISSION_BYTE
    };

    /*writing to serial connection*/
    epicsInt32 inSize = writeReadSerial(UNIT, bufout, 6, inputMsg, 20);

    if (inSize < PIXCI_NO_ERROR)
    {
        return asynError;
    }

    if (inputMsg[0] == SUCCESS_MESSAGE)
    {
        return asynSuccess;
    }
    return asynError;
}

asynStatus ADRaptorEagleXV::readSerialRegister(epicsInt8 Register, epicsInt8 *val)
{
    char inputMsg[20] = {};
    epicsInt32 inSize = 0;
    char first_bufout[] = {
         static_cast<char>(SINGLE_OUTPUT_BYTE_PREFIX_BYTES[0]),
         static_cast<char>(SINGLE_OUTPUT_BYTE_PREFIX_BYTES[1]),
         static_cast<char>(SINGLE_OUTPUT_BYTE_PREFIX_BYTES[2]),
        Register,
        END_OF_TRANSMISSION_BYTE
    };
    char last_bufout[] = {
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[0]),
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[1]),
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[2]),
        END_OF_TRANSMISSION_BYTE
    };

    /*writing to serial connection*/
    inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 20);
    inSize = writeReadSerial(UNIT, last_bufout, sizeof(last_bufout), inputMsg, 20);

    *val = inputMsg[0];
    if (inputMsg[1] == SUCCESS_MESSAGE)
    {
        return asynSuccess;
    }
    return asynError;
}

asynStatus ADRaptorEagleXV::readSerialRegister(epicsInt8 Register1, epicsInt8 Register2, epicsInt8 *val)
{
    char inputMsg[20] = {};
    epicsInt32 inSize = 0;
    char first_bufout[] = {
         static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[0]),
         static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[1]),
         static_cast<char>(DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[2]),
        Register1, Register2,
        END_OF_TRANSMISSION_BYTE
    };
    char last_bufout[] = {
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[0]),
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[1]),
         static_cast<char>(READ_SERIAL_PREFIX_BYTES[2]),
        END_OF_TRANSMISSION_BYTE
    };

    /*writing to serial connection*/
    inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 20);
    inSize = writeReadSerial(UNIT, last_bufout, sizeof(last_bufout), inputMsg, 20);

    *val = inputMsg[0];
    if (inputMsg[1] == SUCCESS_MESSAGE)
    {
        return asynSuccess;
    }
    return asynError;
}

epicsUInt8 ADRaptorEagleXV::getSystemStatus()
{
    epicsUInt8 cval = 0;
    char inputMsg[2] = {};
    char first_bufout[] = {GET_SYSTEM_STATUS_BYTE, END_OF_TRANSMISSION_BYTE};

    /*writing to serial connection*/
    epicsInt32 inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 2);

    if (inputMsg[1] == SUCCESS_MESSAGE)
    {
        cval = inputMsg[0];
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to get system status\n");
    }

    // TODO(irie-stfc): Need proper error handling, same is for reading serial register as else where
    return cval;    // cval will be 0x00 if there is no success
}

asynStatus ADRaptorEagleXV::setSystemStatus(epicsInt8 val)
{
    char inputMsg[1] = {};

    /* template of message to write value to registers */
    char bufout[] = {SET_SYSTEM_STATUS_BYTE, val, END_OF_TRANSMISSION_BYTE};

    /*writing to serial connection*/
    epicsInt32 inSize = writeReadSerial(UNIT, bufout, sizeof(bufout), inputMsg, 1);

    if (inSize < PIXCI_NO_ERROR)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "Failed to set system status\n");
        return asynError;
    }

    if (inputMsg[0] == SUCCESS_MESSAGE)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "System status set successfully\n");
        return asynSuccess;
    }

    return asynError;
}

asynStatus ADRaptorEagleXV::setFrameRate(epicsFloat64 frameRate)
{
    asynStatus status = asynSuccess;
    epicsInt8 frameRateHexVal[5] = {0, 0, 0, 0, 0};
    epicsUInt64 frameRateCount = (epicsUInt64)(COUNT_PER_FRAME / frameRate);
    uInt64ToInt8(frameRateCount, frameRateHexVal);

    setStatIfHigher(&status, writeSerialRegister(UNIT, FRAME_RATE_BYTES[0], frameRateHexVal[0]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, FRAME_RATE_BYTES[1], frameRateHexVal[1]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, FRAME_RATE_BYTES[2], frameRateHexVal[2]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, FRAME_RATE_BYTES[3], frameRateHexVal[3]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, FRAME_RATE_BYTES[4], frameRateHexVal[4]));
    return status;
}

epicsFloat64 ADRaptorEagleXV::getFrameRate()
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[5] = {0, 0, 0, 0, 0};
    epicsFloat64 frameRate = 0.0;
    setStatIfHigher(&status, readSerialRegister(FRAME_RATE_BYTES[0], &cval[0]));
    setStatIfHigher(&status, readSerialRegister(FRAME_RATE_BYTES[1], &cval[1]));
    setStatIfHigher(&status, readSerialRegister(FRAME_RATE_BYTES[2], &cval[2]));
    setStatIfHigher(&status, readSerialRegister(FRAME_RATE_BYTES[3], &cval[3]));
    setStatIfHigher(&status, readSerialRegister(FRAME_RATE_BYTES[4], &cval[4]));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading frame rate\n");
    }
    else 
    {
        epicsUInt64 frameRateCount = int8ToUInt64(cval);
        if (frameRateCount > 0)
        {
            frameRate = 40e6 / epicsFloat64(frameRateCount);
        }
    }
    return frameRate;
}

epicsFloat64 ADRaptorEagleXV::convertAdcCountToCentigrade(epicsInt16 adcCount)
{
    return (ADC_M * adcCount) + ADC_C;  // temperature in centigrade
}

epicsUInt16 ADRaptorEagleXV::convertCentigradeToDacCount(epicsFloat64 temperature)
{
    return static_cast<epicsUInt16>((temperature - DAC_C) / DAC_M);
}

epicsFloat64 ADRaptorEagleXV::convertDacCountToCentigrade(epicsInt16 dacCount)
{
    return (DAC_M * dacCount) + DAC_C;  // temperature in centigrade
}

epicsFloat64 ADRaptorEagleXV::getTemperatureActual()
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[2] = {0, 0};

    setStatIfHigher(&status, readSerialRegister(CCD_SILISCON_TEMPERATURE_BYTES[0], CCD_SILISCON_TEMPERATURE_BYTES[1], &cval[0]));
    setStatIfHigher(&status, readSerialRegister(CCD_SILISCON_TEMPERATURE_BYTES[2], CCD_SILISCON_TEMPERATURE_BYTES[3], &cval[1]));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading CCD temperature\n");
    }
    epicsInt16 adcCount = 0;
    adcCount += (epicsInt16)(epicsUInt8)cval[1];
    adcCount += ((epicsInt16)(epicsUInt8)cval[0]) << 8;

    return convertAdcCountToCentigrade(adcCount);
}

epicsFloat64 ADRaptorEagleXV::getTemperaturePCB()
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[2] = {0, 0};

    setStatIfHigher(&status, readSerialRegister(PCB_TEMPERATURE_BYTES[0], PCB_TEMPERATURE_BYTES[1], &cval[1]));
    setStatIfHigher(&status, readSerialRegister(PCB_TEMPERATURE_BYTES[2], PCB_TEMPERATURE_BYTES[3], &cval[0]));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading PCB temperature\n");
    }
    epicsInt16 lval = 0;
    lval += (epicsInt16)(epicsUInt8)cval[0];
    lval += (epicsInt16)(epicsUInt8)(cval[1] & 0x0F) << 8;
    return lval / 16.0;
}

epicsFloat64 ADRaptorEagleXV::getCoolingSetPoint()
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[2] = {0, 0};

    setStatIfHigher(&status, readSerialRegister(TEC_TEMPERATURE_BYTES[0], &cval[1]));
    setStatIfHigher(&status, readSerialRegister(TEC_TEMPERATURE_BYTES[1], &cval[0]));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading TEC set point\n");
    }
    epicsInt16 lval = 0;
    lval += (epicsInt16)(epicsUInt8)cval[0];
    lval += (epicsInt16)(epicsUInt8)(cval[1] & 0x0F) << 8;

    return convertDacCountToCentigrade(lval);
}

asynStatus ADRaptorEagleXV::setCoolingSetPoint(epicsFloat64 temperature)
{
    asynStatus status = asynSuccess;
    epicsUInt16 dacCount = convertCentigradeToDacCount(temperature);

    epicsInt8 cval[2] = {0, 0};
    cval[0] = (epicsInt8)((dacCount & 0x0F00) >> 8);
    cval[1] = (epicsInt8)((dacCount & 0x00FF));

    setStatIfHigher(&status, writeSerialRegister(UNIT, TEC_TEMPERATURE_BYTES[0], cval[0]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, TEC_TEMPERATURE_BYTES[1], cval[1]));
    return status;
}

epicsUInt8 ADRaptorEagleXV::getFpgaStatus()
{
    epicsInt8 cval = 0;
    asynStatus status = readSerialRegister(FPGA_STATUS_BYTE, &cval);
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading FPGA status\n");
    }
    return (epicsUInt8)cval;
}

asynStatus ADRaptorEagleXV::toggleFpgaComms(epicsBoolean enableFpgaComms)
{
    epicsUInt8 systemStatus = getSystemStatus();
    if (enableFpgaComms)
        return setSystemStatus(systemStatus | 0x01);    // setting first bit = 1
    else
        return setSystemStatus(systemStatus & ~(0x01));     // setting first bit = 0
}

epicsBoolean ADRaptorEagleXV::isFpgaCommsEnabled()
{
    epicsUInt8 systemStatus = getSystemStatus();
    return static_cast<epicsBoolean>((systemStatus & 0x01) != 0);   // check the first bit is not 0
}


asynStatus ADRaptorEagleXV::toggleTec(epicsBoolean enableTec)
{
    epicsUInt8 fpgaStatus = getFpgaStatus();
    if (enableTec)
        return writeSerialRegister(UNIT, FPGA_STATUS_BYTE, fpgaStatus | 0x01);  // setting first bit = 1
    else
        return writeSerialRegister(UNIT, FPGA_STATUS_BYTE, fpgaStatus & ~(0x01));   // setting first bit = 0
}

epicsBoolean ADRaptorEagleXV::isTecEnabled()
{
    epicsUInt8 fpgaStatus = getFpgaStatus();
    return static_cast<epicsBoolean>((fpgaStatus & 0x01) != 0);     // check the first bit is not 0
}

asynStatus ADRaptorEagleXV::toggleGain(epicsBoolean enableGain)
{
    epicsUInt8 fpgaStatus = getFpgaStatus();
    if (enableGain)
        return writeSerialRegister(UNIT, FPGA_STATUS_BYTE, fpgaStatus | (1 << 7));  // setting last bit = 1
    else
        return writeSerialRegister(UNIT, FPGA_STATUS_BYTE, fpgaStatus & ~(1 << 7));     // setting last bit = 0
}

epicsBoolean ADRaptorEagleXV::isGainEnabled()
{
    epicsUInt8 fpgaStatus = getFpgaStatus();
    return static_cast<epicsBoolean>((fpgaStatus & (1 << 7)) != 0);     // check the last bit is not 0
}

asynStatus ADRaptorEagleXV::setExposure(epicsFloat64 exposureTime)
{
    asynStatus status = asynSuccess;
    epicsInt8 exposureTimeHexVal[5] = {0, 0, 0, 0, 0};
    epicsUInt64 exposureTimeCount = (epicsUInt64)(exposureTime * EXPOSURE_COUNT_TO_TIME / SEC_TO_mS);
    uInt64ToInt8(exposureTimeCount, exposureTimeHexVal);

    setStatIfHigher(&status, writeSerialRegister(UNIT, EXPOSURE_BYTES[0], exposureTimeHexVal[0]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, EXPOSURE_BYTES[1], exposureTimeHexVal[1]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, EXPOSURE_BYTES[2], exposureTimeHexVal[2]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, EXPOSURE_BYTES[3], exposureTimeHexVal[3]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, EXPOSURE_BYTES[4], exposureTimeHexVal[4]));
    return status;
}

epicsFloat64 ADRaptorEagleXV::getExposure()
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[5] = {0, 0, 0, 0, 0};
    epicsFloat64 exposureTime = 0.0;
    setStatIfHigher(&status, readSerialRegister(EXPOSURE_BYTES[0], &cval[0]));
    setStatIfHigher(&status, readSerialRegister(EXPOSURE_BYTES[1], &cval[1]));
    setStatIfHigher(&status, readSerialRegister(EXPOSURE_BYTES[2], &cval[2]));
    setStatIfHigher(&status, readSerialRegister(EXPOSURE_BYTES[3], &cval[3]));
    setStatIfHigher(&status, readSerialRegister(EXPOSURE_BYTES[4], &cval[4]));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading exposure time\n");
    }
    else 
    {
        epicsUInt64 exposureTimeCount = int8ToUInt64(cval);
        if (exposureTimeCount > 0)
        {
            exposureTime = (static_cast<epicsFloat64>(exposureTimeCount) / EXPOSURE_COUNT_TO_TIME) * SEC_TO_mS;
        }
    }
    return exposureTime;
}

epicsUInt8 ADRaptorEagleXV::convertDelayTimeToHex(epicsFloat64 delayTime) {
    epicsFloat64 scaled = delayTime * MILLISECOND_PER_COUNT;
    epicsInt32 rounded = static_cast<epicsInt32>(std::round(scaled));
    epicsUInt8 hexVal = static_cast<epicsUInt8>(rounded);
    return hexVal;
}

epicsFloat64 ADRaptorEagleXV::convertHexToDelayTime(epicsUInt8 hexVal) {
    epicsFloat64 hexAsDouble = static_cast<epicsFloat64>(hexVal);
    epicsFloat64 delayTime = hexAsDouble / MILLISECOND_PER_COUNT;
    return delayTime;
}

asynStatus ADRaptorEagleXV::setShutterOpenDelay(epicsFloat64 delayTime)
{
    epicsUInt8 hexVal = convertDelayTimeToHex(delayTime);
    return writeSerialRegister(UNIT, SHUTTER_OPEN_DELAY_BYTE, reinterpret_cast<epicsInt8&>(hexVal));
}

epicsFloat64 ADRaptorEagleXV::getShutterOpenDelay()
{
    epicsInt8 hexVal = 0;
    asynStatus status = readSerialRegister(SHUTTER_OPEN_DELAY_BYTE, &hexVal);
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to read shutter open delay\n");
    }
    return convertHexToDelayTime(reinterpret_cast<epicsUInt8&>(hexVal));
}

asynStatus ADRaptorEagleXV::setShutterCloseDelay(epicsFloat64 delayTime)
{
    epicsUInt8 hexVal = convertDelayTimeToHex(delayTime);
    return writeSerialRegister(UNIT, SHUTTER_CLOSE_DELAY_BYTE, reinterpret_cast<epicsInt8&>(hexVal));
}

epicsFloat64 ADRaptorEagleXV::getShutterCloseDelay()
{
    epicsInt8 hexVal = 0;
    asynStatus status = readSerialRegister(SHUTTER_CLOSE_DELAY_BYTE, &hexVal);
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to read shutter close delay\n");
    }
    return convertHexToDelayTime(reinterpret_cast<epicsUInt8&>(hexVal));
}

asynStatus ADRaptorEagleXV::updateTemperatureActual()
{
    asynStatus status = asynSuccess;
    setStatIfHigher(&status, setDoubleParam(ADTemperatureActual, this->getTemperatureActual()));
    setStatIfHigher(&status, setDoubleParam(PR_TemperaturePCB, getTemperaturePCB()));
    return status;
}

asynStatus ADRaptorEagleXV::updateInfo()
{
    using std::to_string;

    asynStatus status = asynSuccess;
    char inputMsg[20] = {};
    epicsInt32 inSize = 0;
    char first_bufout[] = {
         static_cast<char>(GET_MISC_DATA_BYTES[0]),
         static_cast<char>(GET_MISC_DATA_BYTES[1]),
         static_cast<char>(GET_MISC_DATA_BYTES[2]),
         static_cast<char>(GET_MISC_DATA_BYTES[3]),
         static_cast<char>(GET_MISC_DATA_BYTES[4]),
         static_cast<char>(GET_MISC_DATA_BYTES[5]),
         static_cast<char>(GET_MISC_DATA_BYTES[6]),
         static_cast<char>(GET_MISC_DATA_BYTES[7]),
        END_OF_TRANSMISSION_BYTE
    };
    char last_bufout[] = {
         static_cast<char>(MANUFACTURER_DATA_BYTES[0]),
         static_cast<char>(MANUFACTURER_DATA_BYTES[1]),
         static_cast<char>(MANUFACTURER_DATA_BYTES[2]),
        END_OF_TRANSMISSION_BYTE
    };

    epicsInt16 serialNumber = 0;
    std::string buildDate = "";
    epicsInt16 adcCountZeroDegree = 0;
    epicsInt16 adcCountFortyDegree = 0;
    epicsInt16 dacCountZeroDegree = 0;
    epicsInt16 dacCountFortyDegree = 0;

    setStatIfHigher(&status, toggleFpgaComms(epicsTrue));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to turn FPGA comms on\n");
    }

    /*writing to serial connection*/
    inSize = writeReadSerial(UNIT, first_bufout, sizeof(first_bufout), inputMsg, 20);
    inSize = writeReadSerial(UNIT, last_bufout, sizeof(last_bufout), inputMsg, 20);

    setStatIfHigher(&status, toggleFpgaComms(epicsFalse));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Failed to turn FPGA comms off\n");
    }

    if (inputMsg[18] == SUCCESS_MESSAGE)
    {
        serialNumber += (epicsInt16)(epicsUInt8)inputMsg[0];
        serialNumber += (epicsInt16)(epicsUInt8)(inputMsg[1]) << 8;
        setStringParam(ADSerialNumber, to_string(serialNumber));

        buildDate = to_string((epicsInt16)(epicsUInt8)inputMsg[2]) + "/"
                + to_string((epicsInt16)(epicsUInt8)inputMsg[3]) + "/"
                + to_string((epicsInt16)(epicsUInt8)inputMsg[4]);
        setStringParam(PR_BuildDate, buildDate);

        adcCountZeroDegree += (epicsInt16)(epicsUInt8)inputMsg[10];
        adcCountZeroDegree += (epicsInt16)(epicsUInt8)(inputMsg[11]) << 8;
        setIntegerParam(PR_ADCCalibrationZeroDegree, adcCountZeroDegree);

        adcCountFortyDegree += (epicsInt16)(epicsUInt8)inputMsg[12];
        adcCountFortyDegree += (epicsInt16)(epicsUInt8)(inputMsg[13]) << 8;
        setIntegerParam(PR_ADCCalibrationFortyDegree, adcCountFortyDegree);

        dacCountZeroDegree += (epicsInt16)(epicsUInt8)inputMsg[14];
        dacCountZeroDegree += (epicsInt16)(epicsUInt8)(inputMsg[15]) << 8;
        setIntegerParam(PR_DACCalibrationZeroDegree, dacCountZeroDegree);

        dacCountFortyDegree += (epicsInt16)(epicsUInt8)inputMsg[16];
        dacCountFortyDegree += (epicsInt16)(epicsUInt8)(inputMsg[17]) << 8;
        setIntegerParam(PR_DACCalibrationFortyDegree, dacCountFortyDegree);

        ADC_M = 40.0f / (adcCountFortyDegree - adcCountZeroDegree);
        ADC_C = 40.0f - (ADC_M * adcCountFortyDegree);

        DAC_M = 40.0f / (dacCountFortyDegree - dacCountZeroDegree);
        DAC_C = 40.0f - (DAC_M * dacCountFortyDegree);

        return asynSuccess;
    }

    return asynError;
}

asynStatus ADRaptorEagleXV::setRoiSizeX(epicsInt32 RoisizeX)
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[2] = {0, 0};
    cval[0] = (epicsInt8)((RoisizeX & 0x0F00) >> 8);
    cval[1] = (epicsInt8)((RoisizeX & 0x00FF));

    setStatIfHigher(&status, writeSerialRegister(UNIT, ROI_X_SIZE_BYTES[0], cval[0]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, ROI_X_SIZE_BYTES[1], cval[1]));
    return status;
}

asynStatus ADRaptorEagleXV::setRoiSizeY(epicsInt32 RoisizeY)
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[2] = {0, 0};
    cval[0] = (epicsInt8)((RoisizeY & 0x0F00) >> 8);
    cval[1] = (epicsInt8)((RoisizeY & 0x00FF));

    setStatIfHigher(&status, writeSerialRegister(UNIT, ROI_Y_SIZE_BYTES[0], cval[0]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, ROI_Y_SIZE_BYTES[1], cval[1]));
    return status;
}

asynStatus ADRaptorEagleXV::setRoiOffsetX(epicsInt32 RoiOffsetX)
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[2] = {0, 0};
    cval[0] = (epicsInt8)((RoiOffsetX & 0x0F00) >> 8);
    cval[1] = (epicsInt8)((RoiOffsetX & 0x00FF));

    setStatIfHigher(&status, writeSerialRegister(UNIT, ROI_X_OFFSET_BYTES[0], cval[0]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, ROI_X_OFFSET_BYTES[1], cval[1]));
    return status;
}

asynStatus ADRaptorEagleXV::setRoiOffsetY(epicsInt32 RoiOffsetY)
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[2] = {0, 0};
    cval[0] = (epicsInt8)((RoiOffsetY & 0x0F00) >> 8);
    cval[1] = (epicsInt8)((RoiOffsetY & 0x00FF));

    setStatIfHigher(&status, writeSerialRegister(UNIT, ROI_Y_OFFSET_BYTES[0], cval[0]));
    setStatIfHigher(&status, writeSerialRegister(UNIT, ROI_Y_OFFSET_BYTES[1], cval[1]));
    return status;
}

epicsInt32 ADRaptorEagleXV::getRoiSizeX()
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[2] = {0, 0};
    epicsInt16 ival = 0;

    setStatIfHigher(&status, readSerialRegister(ROI_X_SIZE_BYTES[0], &cval[1]));
    setStatIfHigher(&status, readSerialRegister(ROI_X_SIZE_BYTES[1], &cval[0]));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading ROI X size\n");
    }
    ival += (epicsInt16)(epicsUInt8)cval[0];
    ival += (epicsInt16)(epicsUInt8)(cval[1] & 0x0F) << 8;

    return ival;
}

epicsInt32 ADRaptorEagleXV::getRoiSizeY()
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[2] = {0, 0};
    epicsInt16 ival = 0;

    setStatIfHigher(&status, readSerialRegister(ROI_Y_SIZE_BYTES[0], &cval[1]));
    setStatIfHigher(&status, readSerialRegister(ROI_Y_SIZE_BYTES[1], &cval[0]));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading ROI Y size\n");
    }
    ival += (epicsInt16)(epicsUInt8)cval[0];
    ival += (epicsInt16)(epicsUInt8)(cval[1] & 0x0F) << 8;

    return ival;
}

epicsInt32 ADRaptorEagleXV::getRoiOffsetX()
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[2] = {0, 0};
    epicsInt16 ival = 0;

    setStatIfHigher(&status, readSerialRegister(ROI_X_OFFSET_BYTES[0], &cval[1]));
    setStatIfHigher(&status, readSerialRegister(ROI_X_OFFSET_BYTES[1], &cval[0]));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading ROI X offset\n");
    }
    ival += (epicsInt16)(epicsUInt8)cval[0];
    ival += (epicsInt16)(epicsUInt8)(cval[1] & 0x0F) << 8;

    return ival;
}

epicsInt32 ADRaptorEagleXV::getRoiOffsetY()
{
    asynStatus status = asynSuccess;
    epicsInt8 cval[2] = {0, 0};
    epicsInt16 ival = 0;

    setStatIfHigher(&status, readSerialRegister(ROI_Y_OFFSET_BYTES[0], &cval[1]));
    setStatIfHigher(&status, readSerialRegister(ROI_Y_OFFSET_BYTES[1], &cval[0]));
    if (status > asynSuccess)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error reading ROI Y offset\n");
    }
    ival += (epicsInt16)(epicsUInt8)cval[0];
    ival += (epicsInt16)(epicsUInt8)(cval[1] & 0x0F) << 8;

    return ival;
}

asynStatus ADRaptorEagleXV::setBin(epicsInt32 val, epicsBoolean coordinate)
{
    epicsInt8 hexval = 0;
    epicsInt8 reg = (coordinate == BIN_AXIS_X) ? X_BIN_BYTE : Y_BIN_BYTE;
    std::string cameraModel = "";

    /* Assigning corresponding Hex value to send*/
    switch (val)
    {
    case PR_BIN_1:
        hexval = 0x00;
        break;
    case PR_BIN_2:
        hexval = 0x01;
        break;
    case PR_BIN_4:
        hexval = 0x03;
        break;
    case PR_BIN_8:
        hexval = 0x07;
        break;
    case PR_BIN_16:
        hexval = 0x0F;
        break;
    case PR_BIN_32:
        hexval = 0x1F;
        break;
    case PR_BIN_FVB:
        getStringParam(ADModel, cameraModel);
        if (coordinate == BIN_AXIS_Y && cameraModel == RAPTOR_EAGLE_XV_4240) {
            hexval = static_cast<epicsInt8>(0x80);
            break;
        }
    default:
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid binning value %d", val);
        return asynError;
        break;
    }

    return writeSerialRegister(UNIT, reg, hexval);
}

asynStatus ADRaptorEagleXV::setTriggerMode(epicsInt32 mode)
{
    epicsInt8 hexval = 0;
    switch (mode)
    {
    case PRInternalITRTrigger:
        hexval = INTERNAL_ITR_BYTE;     // 00000100
        break;
    case PRInternalFFRTrigger:
        hexval = INTERNAL_FFR_BYTE;     // 00000110
        break;
    case PRExternalTrigger:
        {   // brackets so that trigger polarity goes out of scope after this case
            epicsInt32 triggerPolarity = PRExtRisingEdge;
            getIntegerParam(PR_TriggerPolarity, &triggerPolarity);
            hexval = (triggerPolarity == PRExtFallingEdge) ? EXTERNAL_FALLING_EDGE_BYTE : EXTERNAL_RISING_EDGE_BYTE;
        }
        break;
    case PRSoftTrigger:
        hexval = CLEAR_TRIGGER_MODE_BYTE;   // 00000000
        break;
    default:
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid trigger mode value %d", mode);
        return asynError;
        break;
    }
    return writeSerialRegister(UNIT, TRIGGER_MODE_BYTE, hexval);
}

asynStatus ADRaptorEagleXV::sendSoftTrigger() {
    /* if trigger mode is button trigger then, do the soft trigger else print error */
    epicsInt32 triggerMode = PRInternalITRTrigger;
    getIntegerParam(ADTriggerMode, &triggerMode);
    if (triggerMode == PRSoftTrigger)
    {
        return writeSerialRegister(UNIT, TRIGGER_MODE_BYTE, SOFT_TRIGGER_BYTE);
    }
    else
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Button Trigger mode is not selected");
        return asynError;
    }
}

void ADRaptorEagleXV::changeVideoFormatConfig(epicsInt32 binX, epicsInt32 binY, epicsInt32 sizeX, epicsInt32 sizeY)
{
    std::string cameraModel = "";
    getStringParam(ADModel, cameraModel);
    if (cameraModel == RAPTOR_EAGLE_XV_4710)
    {
        if (binX == PR_BIN_1 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_1x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_2x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_4x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_8x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_16x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4710\bin_32x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid binning value");
        }
    }
    else if (cameraModel == RAPTOR_EAGLE_XV_4240)
    {
        if (binX == PR_BIN_1 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_1 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_1xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_2 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_2xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_4 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_4xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_8 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_8xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_16 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_16xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_1)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x1.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_2)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x2.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_4)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x4.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_8)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x8.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_16)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x16.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_32)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32x32.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else if (binX == PR_BIN_32 && binY == PR_BIN_FVB)
        {
            #include "fmt\Raptor_Eagle_XV_4240\bin_32xFVB.fmt"
            pxd_videoFormatAsIncludedInit(0);
            pxd_videoFormatAsIncluded(0);
        }
        else {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "invalid binning value");
        }
    }
    else {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "camera model %s not supported ???", cameraModel.c_str());
    }
    ADPixci::changeVideoFormatConfig(binX, binY, sizeX, sizeY);
}

asynStatus ADRaptorEagleXV::updateInitialPVs(){
    asynStatus status = asynSuccess;
    epicsFloat64 acquireFrameRate = this->getFrameRate();

    epicsFloat64 readBackAcquireTime = this->getExposure();
    if (readBackAcquireTime <= 0.0)
    {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Error in acquire time readback. Readback not set\n");
        return asynError;
    }
    setStatIfHigher(&status, setDoubleParam(ADAcquireTime, readBackAcquireTime));
    if (acquireFrameRate > 0)
    {
        setStatIfHigher(&status, setDoubleParam(ADAcquirePeriod, (1 / acquireFrameRate)));
    }

    setStatIfHigher(&status, ADPixci::updateInitialPVs());
    return status;
}

asynStatus ADRaptorEagleXV::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    epicsInt32 function = pasynUser->reason;

    asynStatus status = asynSuccess;
    if (function == ADAcquire)
    {
        // TODO(irie-stfc): adstatus == ADStatusIdle has to be checked
        if (value)
        {
            status = aquireStart();
            if (status == asynSuccess)
            {
                setIntegerParam(ADAcquire, 1);
                callParamCallbacks();
            }
        }
        // Stop acquisition
        // TODO(irie-stfc): adstatus != ADStatusIdle has to be checked
        if (!value)
        {
            /* In button trigger mode , acquisition should no be stoped, that will
            affect the WaitForSingleObject. So only status is updated to STOP. once
            the trigger mode is changed from button trigger mode, the actual implementation
            of acquireStop() will be done.
            */
            epicsInt32 triggerMode = PRInternalITRTrigger;
            getIntegerParam(ADTriggerMode, &triggerMode);
            if (triggerMode == PRSoftTrigger)
            {
                status = asynSuccess;
            }
            else
            {
                status = acquireStop();
            }

            if (status == asynSuccess)
            {
                setIntegerParam(ADAcquire, 0);
                callParamCallbacks();
            }
        }
    } /* set  value for default parameters */
    else if (function == PR_ToggleTec || function == PR_ToggleGain || function == PR_ToggleFpgaComms)
    {
        addToParamQue(function, value);
    } else {
        setStatIfHigher(&status, ADPixci::writeInt32(pasynUser, value));
    }

    return status;
}

asynStatus ADRaptorEagleXV::handleParamTask(epicsInt32 param, epicsFloat64 d_val, epicsInt32 i_val, epicsBoolean b_val)
{
    asynStatus status = asynSuccess;
    if (param == PR_ToggleTec)
    {
        status = toggleTec(b_val);
        if (status == asynSuccess)
        {
            setIntegerParam(PR_ToggleTec, isTecEnabled());
        }
    }
    else if (param == PR_ToggleGain)
    {
        status = toggleGain(b_val);
        if (status == asynSuccess)
        {
            setIntegerParam(PR_ToggleGain, isGainEnabled());
        }
    }
    else if (param == PR_ToggleFpgaComms)
    {
        status = toggleFpgaComms(b_val);
        if (status == asynSuccess)
        {
            setIntegerParam(PR_ToggleFpgaComms, isFpgaCommsEnabled());
        }
    }
    else if (param == ADTriggerMode)
    {
        epicsInt32 acquisitionStatus = asynSuccess;
        epicsInt32 previousTriggerMode = PRInternalITRTrigger;
        status = this->setTriggerMode(i_val);
        if (status == asynSuccess)
        {
            if (i_val == PRSoftTrigger)
            {
                /* In button triggermode, for WaitForSingleObject function to be notified pxd_goLive should be
                called. For that aquireStart() function is called.
                */
                status = aquireStart();
            }
            else
            {
                /* When changes the acquiremode from button triggered to any another trigger mode,
                    we have to check the ADAcquire status  stop acquision if ADAcquire is in 'Stop' state.
                    Because in button trigger mode aquireStart() is called irrespective of ADAcquire status.
                */
                getIntegerParam(ADTriggerMode, &previousTriggerMode);
                if (previousTriggerMode == PRSoftTrigger)
                {
                    getIntegerParam(ADAcquire, &acquisitionStatus);

                    if (acquisitionStatus == 0)
                    {
                        /* acquisiton is stopped if ADAcquire is on stop state*/
                        acquireStop();
                    }
                }
            }
            setIntegerParam(ADTriggerMode, i_val);
        }
    }
    else if (param == PR_TriggerPolarity)
    {
        epicsInt32 triggerMode = PRInternalITRTrigger;
        if (i_val == PRExtRisingEdge)
        {
            setIntegerParam(PR_TriggerPolarity, PRExtRisingEdge);
        }
        else if (i_val == PRExtFallingEdge)
        {
            setIntegerParam(PR_TriggerPolarity, PRExtFallingEdge);
        }
        callParamCallbacks();
        getIntegerParam(ADTriggerMode, &triggerMode);
        if (triggerMode == PRExternalTrigger)
        {
            setTriggerMode(PRExternalTrigger);
        }
    }
    else {
        ADPixci::handleParamTask(param, d_val, i_val, b_val);
    }
}
