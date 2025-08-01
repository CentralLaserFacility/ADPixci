/**
 * @brief This is a driver for EPIX inc. PIXCI camera link frame grabbers
 *  This driver uses the PIXCI ® XCLIB Programming Library.
 *  This class defines specific instructions for Raptor Photonics Eagle XV II 
 *  X-ray CCD cameras.
 * 
 * @authors Irie Railton, Subindev Devadasan, Aoun Muhammad, Sidharth Sarat Raj Batchu, Timothy Speight
 *  
 * @copyright ©Copyright 2025 UK Research and Innovation, Science and Technology Facilities Council, Department
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 * disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PIXCIAPP_SRC_ADRAPTOREAGLEXV_H_
#define PIXCIAPP_SRC_ADRAPTOREAGLEXV_H_

#include "ADPixci.h"

constexpr const char *RAPTOR_EAGLE_XV_4710 = "Raptor_Eagle_XV_4710";  // 1056 x 1027 active pixels
constexpr const char *RAPTOR_EAGLE_XV_4240 = "Raptor_Eagle_XV_4240";  // 2048 x 2048 active pixels

constexpr const char *TemperaturePCBString = "PR_TEMPERATURE_PCB";
constexpr const char *ToggleTecString = "PR_TOGGLE_TEC";
constexpr const char *ToggleGainString = "PR_TOGGLE_Gain";
constexpr const char *ToggleFPGACommsString = "PR_TOGGLE_FPGA_COMMS";
constexpr const char *ADCCalibrationZeroDegreeString = "PR_ADC_CALIBRATION_ZERO_DEGREE";
constexpr const char *ADCCalibrationFortyDegreeString = "PR_ADC_CALIBRATION_FORTY_DEGREE";
constexpr const char *DACCalibrationZeroDegreeString = "PR_DAC_CALIBRATION_ZERO_DEGREE";
constexpr const char *DACCalibrationFortyDegreeString = "PR_DAC_CALIBRATION_FORTY_DEGREE";

constexpr const epicsFloat64 MILLISECOND_PER_COUNT = 1.6384;

constexpr const epicsFloat64 COUNT_PER_FRAME = 40e6;
constexpr const epicsFloat64 EXPOSURE_COUNT_TO_TIME = 40e6;
constexpr const epicsFloat64 SEC_TO_mS = 10e2;

constexpr const epicsInt8 SUCCESS_MESSAGE = 0x50;
constexpr const epicsInt8 END_OF_TRANSMISSION_BYTE = 0x50;

constexpr const epicsUInt8 X_BIN_BYTE = 0xA1;
constexpr const epicsUInt8 Y_BIN_BYTE = 0xA2;

constexpr const epicsUInt8 FPGA_STATUS_BYTE = 0x00;
constexpr const epicsUInt8 GET_SYSTEM_STATUS_BYTE = 0x49;
constexpr const epicsUInt8 SET_SYSTEM_STATUS_BYTE = 0x4F;

constexpr const epicsUInt8 TRIGGER_MODE_BYTE = 0xD4;
constexpr const epicsUInt8 CLEAR_TRIGGER_MODE_BYTE = 0x00;
constexpr const epicsUInt8 SOFT_TRIGGER_BYTE = 0x01;
constexpr const epicsUInt8 INTERNAL_ITR_BYTE = 0x04;
constexpr const epicsUInt8 INTERNAL_FFR_BYTE = 0x06;
constexpr const epicsUInt8 EXTERNAL_FALLING_EDGE_BYTE = 0xC0;
constexpr const epicsUInt8 EXTERNAL_RISING_EDGE_BYTE = 0x40;

constexpr const epicsUInt8 SHUTTER_OPEN_DELAY_BYTE = 0xA6;
constexpr const epicsUInt8 SHUTTER_CLOSE_DELAY_BYTE = 0xA7;

constexpr const epicsUInt8 READOUT_MODE_BYTE = 0xF7;
constexpr const epicsUInt8 READOUT_NORMAL_BYTE = 0x01;
constexpr const epicsUInt8 READOUT_TEST_PATTERN_BYTE = 0x04; 

constexpr const epicsUInt8 PIXEL_READOUT_CLOCK_BYTES[2] = {0xA3, 0xA4};
constexpr const epicsUInt8 PIXEL_READOUT_2MHz_BYTES[2] = {0x02, 0x02};
constexpr const epicsUInt8 PIXEL_READOUT_75kHz_BYTES[2] = {0x43, 0x80};

constexpr const epicsUInt8 SINGLE_OUTPUT_BYTE_PREFIX_BYTES[3] = {0x53, 0xE0, 0x01};
constexpr const epicsUInt8 DOUBLE_OUTPUT_BYTE_PREFIX_BYTES[3] = {0x53, 0xE0, 0x02};
constexpr const epicsUInt8 READ_SERIAL_PREFIX_BYTES[3] = {0x53, 0xE1, 0x01};

constexpr const epicsUInt8 GET_MISC_DATA_BYTES[8] = {0x53, 0xAE, 0x05, 0x01, 0x00, 0x00, 0x02, 0x00};
constexpr const epicsUInt8 MANUFACTURER_DATA_BYTES[3] = {0x53, 0xAF, 0x12};

constexpr const epicsUInt8 EXPOSURE_BYTES[5] = {0xED, 0xEE, 0xEF, 0xF0, 0xF1};
constexpr const epicsUInt8 FRAME_RATE_BYTES[5] = {0xDC, 0xDD, 0xDE, 0xDF, 0xE0};
constexpr const epicsUInt8 PCB_TEMPERATURE_BYTES[4] = {0x70, 0x00, 0x71, 0x00};
constexpr const epicsUInt8 CCD_SILISCON_TEMPERATURE_BYTES[4] = {0x6E, 0x00, 0x6F, 0x00};
constexpr const epicsUInt8 TEC_TEMPERATURE_BYTES[2] = {0x03, 0x04};
constexpr const epicsUInt8 ROI_X_SIZE_BYTES[2] = {0xB4, 0xB5};
constexpr const epicsUInt8 ROI_Y_SIZE_BYTES[2] = {0xB8, 0xB9};
constexpr const epicsUInt8 ROI_X_OFFSET_BYTES[2] = {0xB6, 0xB7};
constexpr const epicsUInt8 ROI_Y_OFFSET_BYTES[2] = {0xBA, 0xBB};

class ADRaptorEagleXV : public ADPixci
{
 public:
    ADRaptorEagleXV(const char *portName, epicsInt32 maxBuffers, size_t maxMemory, epicsInt32 priority,
        epicsInt32 stackSize, const char *cameraModel, const char *formatFile);

    /* Binning Options */
    typedef enum
    {
        PR_BIN_1 = 1,
        PR_BIN_2 = 2,
        PR_BIN_4 = 4,
        PR_BIN_8 = 8,
        PR_BIN_16 = 16,
        PR_BIN_32 = 32,
        PR_BIN_FVB = 2048,
    } PR_BinningOptions_t;

    /* Trigger modes of Raptor Eagle-XV */
    /* ITR mode will be used to capture a continuous sequence of images.
    * The camera will immediately trigger the start of a new integration period
    * when the previous image readouthas completed.
    * In FFR mode, the camera will generate an internal trigger signal at a user programmable frame rate.
    */
    typedef enum
    {
        PRInternalITRTrigger,
        PRInternalFFRTrigger,
        PRExternalTrigger,
        PRSoftTrigger
    } PRTriggerMode_t;

    /* Readout Mode Options*/
    typedef enum
    {
        readoutNormal,
        readoutTestPattern,
    } PRReadoutMode_t;

    /* Pixel Readout Clock Options */
    typedef enum
    {
        pixelReadoutClock25MHz,
        pixelReadoutClock75kHz
    } PRPixelReadoutClock_t;

 protected:
    epicsInt32 PR_TemperaturePCB;
    #define FIRST_RAPTOR_EAGLE_XV_PARAM PR_TemperaturePCB
    epicsInt32 PR_ToggleTec;
    epicsInt32 PR_ToggleGain;
    epicsInt32 PR_ToggleFpgaComms;
    epicsInt32 PR_ADCCalibrationZeroDegree;
    epicsInt32 PR_ADCCalibrationFortyDegree;
    epicsInt32 PR_DACCalibrationZeroDegree;
    epicsInt32 PR_DACCalibrationFortyDegree;

 private:
    epicsFloat32 ADC_M;     // ADC Slope
    epicsFloat32 ADC_C;     // ADC Offset
    epicsFloat32 DAC_M;     // DAC Slope
    epicsFloat32 DAC_C;     // DAC Offset

    /**
     * @brief write value to the registers of the camera using serial command, might take longer
     * time to execute. Advised to run in seperate thread.
     *
     * @param Register register where value has to be written
     * @param val value to be written in the register
     * @return asynStatus
     */
    asynStatus writeSerialRegister(epicsInt8 Register, epicsInt8 val);

    /**
     * @brief read camera registers over serial communication.
     *
     * @param Register register address to be read
     * @param val returned value
     * @return status, asynSuccess if read was successfull , else asynError
     */
    asynStatus readSerialRegister(epicsInt8 Register, epicsInt8 *val);

    /**
     * @brief read camera 2 bytes registers over serial communication
     *
     * @param Register1 first register address to be read
     * @param Register2 second register address to be read
     * @param val returned value
     * @return status, asynSuccess if read was successfull , else asynError
     */
    asynStatus readSerialRegister(epicsInt8 Register1, epicsInt8 Register2, epicsInt8 *val);

    /**
     * @brief Get the System Status from camera
     *
     * @return unsigned char 1 byte returned from camera
     * Bit 7,5,3 = Reserved
     * Bit 6 = 1 check sum mode enabled
     * Bit 4 = 1 to enable command ACK
     * Bit 2 = 1 if FPGA booted ok
     * Bit 1 = 0 to Hold FPGA in RESET
     * Bit 0 = 1 to enable comms to FPGA EPROM
     */
    epicsUInt8 getSystemStatus();

    /**
     * @brief Set the system status of the camera
     *
     * @param val
     * Bit 7,5,3 = Reserved
     * Bit 6 = 1 check sum mode enabled
     * Bit 4 = 1 to enable command ACK
     * Bit 2 = 1 if FPGA booted ok
     * Bit 1 = 0 to Hold FPGA in RESET
     * Bit 0 = 1 to enable comms to FPGA EPROM
     * @return asynStatus
     */
    asynStatus setSystemStatus(epicsInt8 val);

    /**
     * @brief Convert the ADC Count to the temperature in centigrade
     *
     * @param adcCount ADC count value
     * @return double temperature in centigrade
     */
    epicsFloat64 convertAdcCountToCentigrade(epicsInt16 adcCount);

    /**
     * @brief Convert the temperature in centigrade to DAC Count
     *
     * @param temperature Temperature in centigrade
     * @return unsigned INT16 output DAC count
     */
    epicsUInt16 convertCentigradeToDacCount(epicsFloat64 temperature);

    /**
     * @brief Convert the DAC count to temperature in centigrade
     *
     * @param dacCount DAC count
     * @return double temperature in centigrade
     */
    epicsFloat64 convertDacCountToCentigrade(epicsInt16 dacCount);

    /**
     * @brief Get the PCB temperature from the camera
     *
     * @return double PCB temperature
     */
    epicsFloat64 getTemperaturePCB();

    /**
     * @brief Get the FPGA Status from camera
     *
     * @return unsigned char return 1 byte of FPGA flags
     * Bit 7 = 0 to enable high pre amp gain
     * Bit 6,5,4,3,2 = reserved (Default=0)
     * Bit 1 = 1 OverTemp >80°C tripped (Default=0)
     * Bit 0 = 1 to enable TEC (Default=0)
     */
    epicsUInt8 getFpgaStatus();

    /**
     * @brief Enable/Disable comms to FPGA EPROM
     *
     * @param enableFpgaComms
     * @return asynStatus
     */
    asynStatus toggleFpgaComms(epicsBoolean enableFpgaComms);

    /**
     * @brief Get the comms to FPGA EPROM enable status
     *
     * @return true  = comms to FPGA EPROM Enabled
     * @return false = comms to FPGA EPROM Disabled
     */
    epicsBoolean isFpgaCommsEnabled();

    /**
     * @brief Enable/ Disable TEC controller
     *
     * @param enableTec
     * @return asynStatus
     */
    asynStatus toggleTec(epicsBoolean enableTec);

    /**
     * @brief Get the TEC enable status
     *
     * @return true - TEC Enabled
     * @return false - TEC Disabled
     */
    epicsBoolean isTecEnabled();

    /**
     * @brief Enable/Disable Pre-Amp Gain
     *
     * @param enableGain
     * @return asynStatus
     */
    asynStatus toggleGain(epicsBoolean enableGain);

    /**
     * @brief Get the Pre-Amp Gain enable status
     *
     * @return true = Pre Amp Gain Enabled
     * @return false = Pre Amp Gain Disabled
     */
    epicsBoolean isGainEnabled();

    /**
     * @brief Set the Acquire Time (exposure)
     *
     * @param exposureTime
     * @return asynStatus
     */
    asynStatus setExposure(epicsFloat64 exposureTime) final;

    /**
     * @brief Get the Acquire Time from the camera
     *
     * @return double
     */
    epicsFloat64 getExposure() final;

    /**
     * @brief Convert delay time (ms) to hex value
     * 
     * @param delayTime delay time in ms
     * @return epicsInt8 hex value of delay time for camera
     */
    epicsUInt8 convertDelayTimeToHex(epicsFloat64 delayTime);

    /**
     * @brief Convert hex value to delay time (ms)
     * 
     * @param hexVal Hexidecimal value of delay time from camera
     * @return epicsFloat64 delay time in ms
     */
    epicsFloat64 convertHexToDelayTime(epicsUInt8 hexVal);

    /**
     * @brief Update the PVs related to manufacturers data
     */
    asynStatus updateInfo();

    /**
     * @brief Handle trigger mode updates
     * 
     * @param newTriggerMode new trigger mode to set on the camera
     * @return asynStatus
     */
    asynStatus updateTriggerMode(epicsInt32 newTriggerMode);
    
    /**
     * @brief Handle trigger polarity updates
     * 
     * @param newTriggerPolarity new trigger polarity to set on the camera
     * @return asynStatus
     */
    asynStatus updateTriggerPolarity(epicsInt32 newTriggerPolarity);

    /**
     * @brief Handle acquisition updates
     * 
     * @param value 1 to start acquisition, 0 to stop acquisition
     * @return asynStatus
     */
    asynStatus updateAcquisition(epicsInt32 value);

    /*****************************Functions from Pixci class that are only implemented here****************************/

    /**
     * @brief Update the PCB and CCD temperatures of the camera
     * 
     * @return asynStatus
     */
    asynStatus updateTemperatureActual() final;

    /**
     * @brief Get the CCD temperature from the camera
     *
     * @return double CCD temperature
     */
    epicsFloat64 getTemperatureActual() final;

    /**
     * @brief Get the Tec temperature from the camera
     *
     * @return double TEC temperature in centigrade
     */
    epicsFloat64 getCoolingSetPoint() final;

    /**
     * @brief Set the TEC temperature
     *
     * @param temperature
     * @return asynStatus
     */
    asynStatus setCoolingSetPoint(epicsFloat64 temperature) final;

    /**
     * @brief Set the Frame Rate for Internal FFR mode
     *
     * @param frameRate
     * @return asynStatus
     */
    asynStatus setFrameRate(epicsFloat64 frameRate) final;

    /**
     * @brief Get the Frame Rate from the camera
     *
     * @return double framerate
     */
    epicsFloat64 getFrameRate() final;

    /**
     * @brief Set the ROI Size X
     *
     * @param RoisizeX
     * @return asynStatus
     */
    asynStatus setRoiSizeX(epicsInt32 RoisizeX) final;

    /**
     * @brief Set the ROI Size Y
     *
     * @param RoisizeY
     * @return asynStatus
     */
    asynStatus setRoiSizeY(epicsInt32 RoisizeY) final;

    /**
     * @brief Set the ROI X Offset
     *
     * @param RoiOffsetX
     * @return asynStatus
     */
    asynStatus setRoiOffsetX(epicsInt32 RoiOffsetX) final;

    /**
     * @brief Set the ROI Y Offset
     *
     * @param RoiOffsetY
     * @return asynStatus
     */
    asynStatus setRoiOffsetY(epicsInt32 RoiOffsetY) final;

    /**
     * @brief Get the Roi Size X
     *
     * @return int
     */
    epicsInt32 getRoiSizeX() final;

    /**
     * @brief Get the ROI Size Y
     *
     * @return int
     */
    epicsInt32 getRoiSizeY() final;

    /**
     * @brief Get the ROI Offset X
     *
     * @return int
     */
    epicsInt32 getRoiOffsetX() final;

    /**
     * @brief Get the ROI Offset Y
     *
     * @return int
     */
    epicsInt32 getRoiOffsetY() final;

    /**
     * @brief Set the shutter open delay (ms)
     * 
     * @param delayTime delay time in ms
     * @return asynStatus
     */
    asynStatus setShutterOpenDelay(epicsFloat64 delayTime) final;

    /**
     * @brief Get the shutter open delay (ms)
     * 
     * @return epicsFloat64 delay time in ms
     */
    epicsFloat64 getShutterOpenDelay() final;

    /**
     * @brief Set the shutter close delay (ms)
     * 
     * @param delayTime delay time in ms
     * @return asynStatus
     */
    asynStatus setShutterCloseDelay(epicsFloat64 delayTime) final;

    /**
     * @brief Get the shutter close delay (ms)
     * 
     * @return epicsFloat64 delay time in ms
     */
    epicsFloat64 getShutterCloseDelay() final;

    /**
     * @brief Set the Binning settings. Uses serial communication.
     *
     * @param val Binning value to set
     * @param coordinate 0 for x axis and 1 for y axis.
     * @return asynStatus
     */
    asynStatus setBin(epicsInt32 val, epicsBoolean coordinate) final;

    /**
     * @brief Set the Trigger Mode for the image capturing
     *
     * @param mode index of the mode,
     * 0 = internal itr mode
     * 1 = internal ffr mode
     * 2 = External mode
     * 3 = Button (software trigger mode)
     * @return asynStatus
     */
    asynStatus setTriggerMode(epicsInt32 mode) final;

    /**
     * @brief Send a soft trigger to the camera to capture one image.
     *
     * @return asynStatus
     */
    asynStatus sendSoftTrigger() final;

    /**
     * @brief Set the readout mode for the camera.
     *
     * @param mode index of the mode,
     * 0 = Normal readout
     * 1 = Test pattern enabled
     * @return asynStatus
     */
    asynStatus setReadoutMode(epicsInt32 mode) final;

    /**
     * @brief Get the current readout mode of the camera.
     * 
     * @return epicsInt32 current readout mode,
     * 0 = Normal readout
     * 1 = Test pattern enabled
     */
    epicsInt32 getReadoutMode() final;

    /**
     * @brief Set the Pixel readout clock on the camera.
     *
     * @param clockSpeed index of the speed,
     * 0 = 2 MHz
     * 1 = 75 kHz
     * @return asynStatus
     */
    asynStatus setPixelReadoutClock(epicsInt32 clockSpeed) final;

    /**
     * @brief Get the current Pixel readout clock of the camera
     * 
     * @return epicsInt32 current pixel readout clock speed,
     * 0 = 2 MHz
     * 1 = 75 kHz
     */
    epicsInt32 getPixelReadoutClock() final;

    /**************************Overloaded functions from Pixci class that call Pixci class too*************************/

    asynStatus updateInitialPVs() final;

    /**
     * @brief Overriden to implement custom write features
     *
     * @param pasynUser
     * @param value
     * @return asynStatus
     */
    asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value) final;

    /**
     * @brief handle the parameter change from the queue
     * @param param the parameter that has to be changed
     * @param d_val the value of the parameter as a double
     * @param i_val the value of the parameter as an integer
     * @param b_val the value of the parameter as a boolean
     * @return asynStatus
     */
    asynStatus handleParamTask(epicsInt32 param, epicsFloat64 d_val, epicsInt32 i_val, epicsBoolean b_val) final;

    /**
     * @brief reload of video settings file. Change in some of the video parameters require reload of
     * video settings in order to reflect in image.
     * @param binX binning factor in x direction.
     * @param binY binning factor in y direction.
     * @param sizeX size of the image in x direction.
     * @param sizeY size of the image in y direction.
     * @return asynStatus
     */
    asynStatus changeVideoFormatConfig(epicsInt32 binX, epicsInt32 binY, epicsInt32 sizeX, epicsInt32 sizeY) final;
};

#endif  // PIXCIAPP_SRC_ADRAPTOREAGLEXV_H_
