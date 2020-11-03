/****************************************************************************

	Thorlabs BP1_Drv - BP1 Beam Profiler VISA instrument driver

	Copyright:  Copyright(c) 2005-2011, Thorlabs GmbH (www.thorlabs.com)
	Author:     EgbertKrause (ekrause@thorlabs.com)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


	Header file

	Date:          Jun-11-2013
	Built with:    NI LabWindows/CVI 2012 (12.0.0)
	Software-Nr:   09.164.xxx
	Version:       3.4

	Changelog:     see 'BP1_Drv.c'

****************************************************************************/


#ifndef __BP1_HEADER
#define __BP1_HEADER

#include <vpptype.h>

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C"
{
#endif


/*---------------------------------------------------------------------------
 Buffers
---------------------------------------------------------------------------*/
#define BP1_BUFFER_SIZE                256                 // General buffer size
#define BP1_ERR_DESCR_BUFFER_SIZE      512                 // Buffer size for error messages
#define BP1_MAX_SCANPOINTS             8176


/*---------------------------------------------------------------------------
 Error/Warning Codes
---------------------------------------------------------------------------*/
// Error codes from measurement head
#define BP1_HEAD_NO_ERR                0x00  // no error
#define BP1_HEAD_ERR                   0x01  // unspecified error
#define BP1_HEAD_ERR_CMD               0x02  // unknown command
#define BP1_HEAD_ERR_CMD_DIR           0x03  // not allowed command IN/OUT direction
#define BP1_HEAD_ERR_BYTECOUNT         0x04  // unexpected byte count in download
#define BP1_HEAD_ERR_DATA_RANGE        0x05  // data out of range
#define BP1_HEAD_ERR_NO_MULT_16        0x06  // data is not a multiple of 16 (sample count)
#define BP1_HEAD_ERR_TRANS_RUNNING     0x07  // cannot start new transmission, old one is running
#define BP1_HEAD_ERR_ACCESS            0x08  // access level required, no or wrong access key
#define BP1_HEAD_ERR_NO_DATA           0x09  // no data available for transmission

// Offsets
#define VI_INSTR_WARNING_OFFSET                    (0x3FFC0900L)
#define VI_INSTR_ERROR_OFFSET          (_VI_ERROR + 0x3FFC0900L)  //0xBFFC0900

// Driver Error Codes
// BP1 errors
#define BP1_ERR_BUF_LEN                8

#define BP1_ERROR_CHECKSUM             (VI_INSTR_ERROR_OFFSET + 0x00)
#define BP1_ERROR_BYTECOUNT            (VI_INSTR_ERROR_OFFSET + 0x01)
#define BP1_ERROR_PTH_X                (VI_INSTR_ERROR_OFFSET + 0x02)   // power too high for BP1 (even at lowest gain)
#define BP1_ERROR_PTH_Y                (VI_INSTR_ERROR_OFFSET + 0x03)
#define BP1_ERROR_PTH_P                (VI_INSTR_ERROR_OFFSET + 0x04)
#define BP1_ERROR_UNABLE_GAUSS_FIT     (VI_INSTR_ERROR_OFFSET + 0x05)
#define BP1_ERROR_ACCESS_KEY           (VI_INSTR_ERROR_OFFSET + 0x06)

#define BP1_VI_ERROR_PARAMETER9        (VI_INSTR_ERROR_OFFSET + 0x99)

// BP1 warnings
#define BP1_WARNING_PTL_X              (VI_INSTR_WARNING_OFFSET + 0x00)   // warnings to be canceled by correct gain and offset settings
#define BP1_WARNING_PTH_X              (VI_INSTR_WARNING_OFFSET + 0x01)
#define BP1_WARNING_PTL_Y              (VI_INSTR_WARNING_OFFSET + 0x02)
#define BP1_WARNING_PTH_Y              (VI_INSTR_WARNING_OFFSET + 0x03)
#define BP1_WARNING_PTL_P              (VI_INSTR_WARNING_OFFSET + 0x04)
#define BP1_WARNING_PTH_P              (VI_INSTR_WARNING_OFFSET + 0x05)

#define BP1_WARNING_OTH_X              (VI_INSTR_WARNING_OFFSET + 0x06)   // offset too high (observed in dark windows)
#define BP1_WARNING_OTH_Y              (VI_INSTR_WARNING_OFFSET + 0x07)
#define BP1_WARNING_OTH_P              (VI_INSTR_WARNING_OFFSET + 0x08)

#define BP1_WARNING_GTL_X              (VI_INSTR_WARNING_OFFSET + 0x09)   // gain too low, can be optimized in next scan
#define BP1_WARNING_GTL_Y              (VI_INSTR_WARNING_OFFSET + 0x0a)
#define BP1_WARNING_GTL_P              (VI_INSTR_WARNING_OFFSET + 0x0b)

#define BP1_VAL_NOT_APPLICABLE         (-1.0E-12)


/*---------------------------------------------------------------------------
 VISA strings
---------------------------------------------------------------------------*/
#define BP1_VI_FIND_RSC_PATTERN        "USB?*?{VI_ATTR_MANF_ID==0x1313 && VI_ATTR_MODEL_CODE==0x8011}"


/*---------------------------------------------------------------------------
 Communication timeout
---------------------------------------------------------------------------*/
#define BP1_TIMEOUT_MIN                1000
#define BP1_TIMEOUT_MAX                60000
#define BP1_TIMEOUT_DEFAULT            3000


/*---------------------------------------------------------------------------
 Status reporting
---------------------------------------------------------------------------*/
// BP1 status bit definitions
#define BP1_STATBIT_ERR                0x01 // error occured
#define BP1_STATBIT_MOT                0x02 // motor control by voltage
#define BP1_STATBIT_STAB               0x04 // motor speed not stabilized
#define BP1_STATBIT_STM                0x08 // internal state machine not running
#define BP1_STATBIT_PFAIL              0x10 // power fail
#define BP1_STATBIT_DRDY               0x20 // data ready


/*---------------------------------------------------------------------------
 Exported constants
---------------------------------------------------------------------------*/
// BP1 Switch Mode
#define BP1_MODE_AUTO                  0
#define BP1_MODE_MANUAL                1


// BP1 Power units
#define BP1_UNIT_DIG                   0
#define BP1_UNIT_MA                    1
#define BP1_UNIT_MW                    2
#define BP1_UNIT_DBM                   3           // for arrays still mW
#define BP1_UNIT_MIN                   BP1_UNIT_DIG
#define BP1_UNIT_MAX                   BP1_UNIT_DBM


// Read Scan access modes
#define BP1_ACCESS_NONBLOCK            0
#define BP1_ACCESS_BLOCK               1


/*---------------------------------------------------------------------------
 GLOBAL USER-CALLABLE FUNCTION DECLARATIONS (Exportable Functions)
---------------------------------------------------------------------------*/
// Init - Close
// ViStatus _VI_FUNC BP1_init (ViRsrc rsc, ViInt32 tmo, ViPSession instrp);
//ViStatus _VI_FUNC TLBP1_init (ViRsrc resourceName, ViBoolean IDQuery,
//									 ViBoolean resetDevice, ViSession *instrumentHandle);  // compatible with LabView 8 and later
ViStatus _VI_FUNC TLBP1_init (ViRsrc resourceName, ViBoolean IDQuery,
									ViBoolean resetDevice, ViPSession instrumentHandle);  // compatible with LabView 8 and later
//ViStatus _VI_FUNC BP1_close (ViSession instrument);
ViStatus _VI_FUNC TLBP1_close (ViSession instrumentHandle); // compatible with LabView 8 and later

// Configuration functions
ViStatus _VI_FUNC TLBP1_GetInstrConfig (ViSession instrumentHandle, ViReal64 *apertureWidth, ViReal64 *slitWidth, ViInt32 *photodiodeType, ViInt32 *maxGainIndex, ViInt32 *maxBandwidthIndex);
ViStatus _VI_FUNC TLBP1_SetupScan_SpeedResolution (ViSession instrumentHandle, ViReal64 scanRate, ViReal64 targetResolution, ViReal64 *selectedResolution, ViReal64 *samplingFrequency, ViInt32 *sampleCount);
ViStatus _VI_FUNC TLBP1_SetupScan_MotorVoltage (ViSession instr, ViReal64 motorVoltage, ViInt16 switchModeVoltage);
ViStatus _VI_FUNC TLBP1_SetupScan_GainBandwidthOffset (ViSession instrumentHandle, ViInt16 switchMode, ViInt16 gainIndexX, ViInt16 bandwidthIndexX, ViInt16 gainIndexY, ViInt16 bandwidthIndexY, ViInt16 gainIndexPower, ViInt16 bandwidthIndexPower, ViInt16 offset);
ViStatus _VI_FUNC TLBP1_SetupScan_Gain (ViSession instrumentHandle, ViInt16 switchModeGain, ViInt16 gainIndexX, ViInt16 gainIndexY, ViInt16 gainIndexPower);
ViStatus _VI_FUNC TLBP1_SetupScan_Bandwidth (ViSession instrumentHandle, ViInt16 switchModeBandwidth, ViInt16 bandwidthIndexX, ViInt16 bandwidthIndexY, ViInt16 bandwidthIndexPower);
ViStatus _VI_FUNC TLBP1_SetupScan_Offset (ViSession instrumentHandle, ViInt16 switchModeOffset, ViInt16 offset);
ViStatus _VI_FUNC TLBP1_GetWavelengthRange (ViSession instrumentHandle, ViReal64 *wavelengthMin, ViReal64 *wavelengthMax);
ViStatus _VI_FUNC TLBP1_SetWavelength (ViSession instrumentHandle, ViReal64 wavelength);
ViStatus _VI_FUNC TLBP1_GetWavelength (ViSession instrumentHandle, ViReal64 *actualWavelength);
ViStatus _VI_FUNC TLBP1_SetPowerFactor (ViSession instrumentHandle, ViReal64 powerFactor);
ViStatus _VI_FUNC TLBP1_GetPowerFactor (ViSession instrumentHandle, ViReal64 *powerFactor);
ViStatus _VI_FUNC TLBP1_SetPowerUnit (ViSession instrumentHandle, int verticalUnit);

// Status functions
ViStatus _VI_FUNC TLBP1_getStatus (ViSession instrumentHandle, ViInt32 *devStatus);
ViStatus _VI_FUNC TLBP1_StartScan (ViSession instrumentHandle);
ViStatus _VI_FUNC TLBP1_StopScan (ViSession instrumentHandle);
ViStatus _VI_FUNC TLBP1_ResetScan (ViSession instrumentHandle);

// Data functions
ViStatus _VI_FUNC TLBP1_ReadScan (ViSession instrumentHandle, ViBoolean accessMode, ViReal64 scan_X[], ViReal64 scan_Y[], ViReal64 position_X[], ViReal64 position_Y[], ViUInt32 *points, ViReal64 *power);
ViStatus _VI_FUNC TLBP1_GetScanInfo (ViSession instrumentHandle, ViReal64 scanInfoList[]);
ViStatus _VI_FUNC TLBP1_AverageScan (ViSession instrumentHandle, ViReal64 scanData_X[], ViReal64 scanData_Y[], ViUInt32 points, ViReal64 power, ViUInt32 averageCount, ViReal64 avgScanData_X[], ViReal64 avgScanData_Y[], ViReal64 *avgPower, ViBoolean *avgDataReady);
ViStatus _VI_FUNC TLBP1_RollingAverageScan (ViSession instrumentHandle, ViReal64 scanData_X[], ViReal64 scanData_Y[], ViUInt32 points, ViReal64 power, ViUInt32 averageCount, ViUInt32 reset, ViReal64 avgScanData_X[], ViReal64 avgScanData_Y[], ViReal64 *avgPower);
ViStatus _VI_FUNC TLBP1_MaxHold (ViSession instrumentHandle, ViReal64 scanData_X[], ViReal64 scanData_Y[], ViUInt32 points, ViReal64 power, ViReal64 maxData_X[], ViReal64 maxData_Y[], ViReal64 *maxPower);
ViStatus _VI_FUNC TLBP1_GaussianFit (ViSession instrumentHandle, ViReal64 scanData[], ViReal64 positionData[], ViUInt32 points, ViReal64 *amplitude, ViReal64 *centroid, ViReal64 *sigma);
ViStatus _VI_FUNC TLBP1_CalcBeamWidth (ViSession instrumentHandle, ViReal64 scanData[], ViReal64 positionData[], ViUInt32 points, ViReal64 clipLevelPercent, ViReal64 *beamWidth);
ViStatus _VI_FUNC TLBP1_CalcBeamParams (ViSession instrumentHandle, ViReal64 scanData[], ViReal64 positionData[], ViUInt32 points, ViReal64 *peakLevel, ViReal64 *peakPosition, ViReal64 *centroidPosition, ViReal64 *beamWidth_ISO, ViReal64 *beamWidth_FWHM);
ViStatus _VI_FUNC TLBP1_Calc1st2ndMoments (ViSession instrumentHandle, ViReal64 scanData[], ViReal64 positionData[], ViUInt32 points, ViReal64 *moment1st, ViReal64 *moment2nd);
ViStatus _VI_FUNC TLBP1_CalcBeamEllipticity (ViSession instrumentHandle, ViReal64 beamWidthX, ViReal64 beamWidthY, ViReal64 *ellipticity);
ViStatus _VI_FUNC TLBP1_CorrectBeamWidth_ISO (ViSession instrumentHandle, ViReal64 measuredBeamWidth, ViReal64 *correctedBeamWidth);
ViStatus _VI_FUNC TLBP1_CorrectM2SlitMethod (ViSession instrumentHandle, ViReal64 measuredM2, ViReal64 *correctedM2);

// Calibration functions
ViStatus _VI_FUNC TLBP1_SetAccessLevel (ViSession instrumentHandle, ViChar accessLevel, ViChar accessCode[]);
ViStatus _VI_FUNC TLBP1_CalBeamPosition (ViSession instrumentHandle, ViChar positionOffset, ViChar positionDeviation);
ViStatus _VI_FUNC TLBP1_SetFdParams (ViSession instrumentHandle, ViChar sensorID, ViChar sensorType, ViChar sensorOptions, ViReal64 dimension);
ViStatus _VI_FUNC TLBP1_GetFdParams (ViSession instrumentHandle, ViChar *sensorID, ViChar *sensorType, ViChar *sensorOptions, ViReal64 *dimension);
ViStatus _VI_FUNC TLBP1_SetFdResp (ViSession instrumentHandle, unsigned short lambda_min, unsigned short lambda_max, unsigned short lambda_step, unsigned short lambda_points, ViReal64 response[]);
ViStatus _VI_FUNC TLBP1_GetFdResp (ViSession instrumentHandle, unsigned short *lambda_min, unsigned short *lambda_max, unsigned short *lambda_step, unsigned short *lambda_points, ViReal64 response[]);
ViStatus _VI_FUNC TLBP1_SetMechElParams (ViSession instrumentHandle, ViReal64 drumCircumference, ViReal64 apertureWidth, ViReal64 slitWidth, ViReal64 minRotSpeed, ViReal64 maxRotSpeed, unsigned int maxSamplFreq, unsigned int maxSamplCount, short offsetSetting, ViReal64 offsetCoeff, ViReal64 filterLoss);
ViStatus _VI_FUNC TLBP1_GetMechElParams (ViSession instrumentHandle, ViReal64 *drumCircumference, ViReal64 *apertureWidth, ViReal64 *slitWidth, ViReal64 *minRotSpeed, ViReal64 *maxRotSpeed, unsigned int *maxSamplFreq, unsigned int *maxSamplCount, short *offset_setting, ViReal64 *offsetCoeff, ViReal64 *filterLoss);
ViStatus _VI_FUNC TLBP1_SetTiaParams (ViSession instrumentHandle, ViChar valueCount, ViReal64 TIAValues[]);
ViStatus _VI_FUNC TLBP1_GetTiaParams (ViSession instrumentHandle, ViChar *valueCount, ViReal64 TIAValues[]);
ViStatus _VI_FUNC TLBP1_SetSn (ViSession instrumentHandle, ViChar SNString[]);
ViStatus _VI_FUNC TLBP1_GetSn (ViSession instrumentHandle, ViChar SNString[]);

// Utility functions
ViStatus _VI_FUNC TLBP1_errorMessage (ViSession instrumentHandle, ViStatus status, ViChar _VI_FAR message[]);
ViStatus _VI_FUNC TLBP1_identificationQuery (ViSession instrumentHandle, ViChar _VI_FAR manufacturerName[], ViChar _VI_FAR instrumentName[], ViChar _VI_FAR instrumentSerialNumber[], ViChar _VI_FAR firmwareRevision[]);
ViStatus _VI_FUNC TLBP1_revisionQuery (ViSession instrumentHandle, ViChar _VI_FAR instrumentDriverRevision[], _VI_FAR ViChar firmwareRevision[]);
ViStatus _VI_FUNC TLBP1_setUserText (ViSession instrumentHandle, ViChar userText[]);
ViStatus _VI_FUNC TLBP1_getUserText (ViSession instrumentHandle, ViChar userText[]);
ViStatus _VI_FUNC TLBP1_SetInstrumentName (ViSession instrumentHandle, ViChar instrumentNameString[]);
ViStatus _VI_FUNC TLBP1_GetInstrumentName (ViSession instrumentHandle, ViChar instrumentNameString[]);
ViStatus _VI_FUNC TLBP1_CalcResponse (ViSession instrumentHandle, ViReal64 wavelengthnm, ViReal64 *responseAW);

#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

#endif   /* __BP1_HEADER */

/****************************************************************************
  End of Header file
****************************************************************************/
