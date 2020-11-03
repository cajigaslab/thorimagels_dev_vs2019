/****************************************************************************

   Thorlabs CHORLIS VISA instrument driver

   This driver supports CHROLIS six color sources

   FOR DETAILED DESCRIPTION OF THE DRIVER FUNCTIONS SEE THE ONLINE HELP FILE
   AND THE PROGRAMMERS REFERENCE MANUAL.

   Copyright:  Copyright(c) 2008-2017, Thorlabs (www.thorlabs.com)

   Disclaimer:

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
****************************************************************************/


#ifndef _TL6WL_DRIVER_HEADER_
#define _TL6WL_DRIVER_HEADER_
#include "visatype.h"
#if defined(__cplusplus) || defined(__cplusplus__)
extern "C"
{
#endif


// array and buffer sizes used throughout this driver
///\todo streamline this, i.e. use the below macros in driver too
#define TL6WL_SHORT_STRING_SIZE                       (20)
#define TL6WL_LONG_STRING_SIZE                        (120)
#define LED_HEAD_MAX_SPEC_CURVE_DATA_POINTS           (1000)
#define FILTER_MAX_FILTER_SPEC_CURVE_DATA_POINTS      (1000)

/*===========================================================================

 Macros

===========================================================================*/
#undef VI_INSTR_WARNING_OFFSET
#undef VI_INSTR_ERROR_OFFSET

#define VI_INSTR_WARNING_OFFSET        (0x3FFC0900L)

// _VI_ERROR is defined as 0x80000000 in visatype.h
#define VI_INSTR_ERROR_OFFSET          (_VI_ERROR + VI_INSTR_WARNING_OFFSET)           // 0xBFFC0900
#define INSTR_RUNTIME_ERROR            (VI_INSTR_ERROR_OFFSET)                         // 0xBFFC0900
#define INSTR_REM_INTER_ERROR          (VI_INSTR_ERROR_OFFSET - 1)                     // 0xBFFC08FF
#define INSTR_AUTHENTICATION_ERROR     (VI_INSTR_ERROR_OFFSET - 2)                     // 0xBFFC08FE
#define INSTR_PARAM_ERROR              (VI_INSTR_ERROR_OFFSET - 3)                     // 0xBFFC08FD
#define INSTR_HW_ERROR                 (VI_INSTR_ERROR_OFFSET - 4)                     // 0xBFFC08FC
#define INSTR_PARAM_CHNG_ERROR         (VI_INSTR_ERROR_OFFSET - 5)                     // 0xBFFC08FB

#define INSTR_INTERNAL_TX_ERR          (VI_INSTR_ERROR_OFFSET - 6)                     // 0xBFFC08FA
#define INSTR_INTERNAL_RX_ERR          (VI_INSTR_ERROR_OFFSET - 7)                     // 0xBFFC08F9
#define INSTR_INVAL_MODE_ERR           (VI_INSTR_ERROR_OFFSET - 8)                     // 0xBFFC08F8
#define INSTR_SERVICE_ERR              (VI_INSTR_ERROR_OFFSET - 9)                     // 0xBFFC08F7

#define INSTR_NOT_YET_IMPL_ERR         (VI_INSTR_ERROR_OFFSET - 10)                    // 0xBFFC08F6
#define INSTR_LED_ADAPT_IN_PROG        (VI_INSTR_ERROR_OFFSET - 11)                    // 0xBFFC08F5
#define INSTR_LED_ADAPT_STP_REQ        (VI_INSTR_ERROR_OFFSET - 12)                    // 0xBFFC08F4


#define VI_DRIVER_ERROR_OFFSET          (_VI_ERROR + VI_INSTR_WARNING_OFFSET - 1024)   // 0xBFFC0500


// User defined driver error codes
#define VI_DRIVER_USB_ERROR            (VI_DRIVER_ERROR_OFFSET)
#define VI_DRIVER_FRAME_SERV_ERROR     (VI_DRIVER_ERROR_OFFSET - 1)                    // 0xBFFC04FF
#define VI_DRIVER_DEV_INTER_BROKEN     (VI_DRIVER_ERROR_OFFSET - 2)                    // 0xBFFC04FE

#if defined(_WIN64)
   #define CALL_CONV            __fastcall
#elif (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)) && !defined(_NI_mswin16_)
   #define CALL_CONV            __stdcall
#endif

#ifdef IS_DLL_TARGET
   #undef _VI_FUNC
   #define _VI_FUNC __declspec(dllexport) CALL_CONV
#elif defined BUILDING_DEBUG_EXE
   #undef _VI_FUNC
   #define _VI_FUNC
#else
   #undef _VI_FUNC
   #define _VI_FUNC __declspec(dllimport) CALL_CONV
#endif


typedef void (*Box6WL_StatusChangedHandler)(ViUInt32 statRegist);
// OLD following two types will get obsolete, do not use for new code
typedef void (*LED_HeadTemperHandler)(ViUInt8 bitmask, ViInt16 tempers[]);
typedef void (*LED_HeadBrightnessChangedHandler)(ViUInt16 linearMode, ViUInt8 bitmask, ViUInt16 brigtness[]);
typedef void (*LED_PowerStateChangedHandler)(ViUInt8 bitmask, ViUInt8 enableState);

// NEW, using callbacks that use ViReal64 instead of ViInt16/VIUint16
typedef void (*LED_HeadTempHandler)(ViUInt8 bitmask, ViReal64 temps[]);
typedef void (*LED_HeadBrightChangedHandler)(ViReal64 linearMode, ViUInt8 bitmask, ViReal64 brightness[]);


/// -------------- T H O R L A B S   6 W L   I N S T R U M E N T   D R I V E R --------------

ViStatus _VI_FUNC TL6WL_init (ViRsrc resourceName, ViBoolean IDQuery,
                              ViBoolean resetDevice, ViPSession instrumentHandle);
ViStatus _VI_FUNC TL6WL_close (ViSession instrumentHandle);

/// ------------------------  R E S O U R C E S   F U N C T I O N S ------------------------

ViStatus _VI_FUNC TL6WL_findRsrc (ViSession instrumentHandle,
                                  ViPUInt32 deviceCount);
ViStatus _VI_FUNC TL6WL_getRsrcName (ViSession instrumentHandle,
                                     ViUInt32 deviceIndex,
                                     ViChar _VI_FAR resourceName[]);
ViStatus _VI_FUNC TL6WL_getRsrcInfo (ViSession instrumentHandle,
                                     ViUInt32 deviceIndex,
                                     ViChar _VI_FAR deviceName[],
                                     ViChar _VI_FAR serialNumber[],
                                     ViChar _VI_FAR manufacturerName[],
                                     ViPBoolean resourceInUse);

/// --------------------------  U T I L I T Y   F U N C T I O N S --------------------------

ViStatus _VI_FUNC TL6WL_identificationQuery (ViSession instrumentHandle,
                                             ViChar _VI_FAR manufacturerName[],
                                             ViChar _VI_FAR deviceName[],
                                             ViChar _VI_FAR serialNumber[]);
ViStatus _VI_FUNC TL6WL_selfTest (ViSession instrumentHandle,
                                  ViPInt16 selfTestResult,
                                  ViChar _VI_FAR description[]);
ViStatus _VI_FUNC TL6WL_reset (ViSession instrumentHandle);
ViStatus _VI_FUNC TL6WL_revisionQuery (ViSession instrumentHandle,
                                       ViChar _VI_FAR instrumentDriverRevision[],
                                       ViChar _VI_FAR chrolisFirmwareRevision[],
                                       ViChar _VI_FAR timingUnitFirmwareRevision[]);
ViStatus _VI_FUNC TL6WL_getCalibrationMsg (ViSession instrumentHandle,
                                       ViChar _VI_FAR calibrationMsg[]);
ViStatus _VI_FUNC TL6WL_getManufactureDate (ViSession instrumentHandle,
                                       ViChar _VI_FAR manufDate[]);

ViStatus _VI_FUNC TL6WL_errorQuery (ViSession instrumentHandle,
                                    ViPInt32 errorNumber,
                                    ViChar _VI_FAR errorMessage[]);
ViStatus _VI_FUNC TL6WL_errorMessage (ViSession instrumentHandle,
                                      ViStatus statusCode,
                                      ViChar _VI_FAR description[]);

/// ---------------------------  S T A T U S   F U N C T I O N S ---------------------------

ViStatus _VI_FUNC TL6WL_getBoxStatus (ViSession instrumentHandle,
                                      ViPUInt32 boxStatus);
ViStatus _VI_FUNC TL6WL_registerBoxStatusChangedHandler (ViSession instrumentHandle,
                                                         Box6WL_StatusChangedHandler boxStatusChangedHandler);

/// ------------------------------------  M E A S U R E ------------------------------------
/// ------------------------  L E D   H E D   T E M P E R A T U R E ------------------------
ViStatus _VI_FUNC TL6WL_getLED_HeadTemperature (ViSession instrumentHandle,
                                                ViPReal64 temperatureLED1,
                                                ViPReal64 temperatureLED2,
                                                ViPReal64 temperatureLED3,
                                                ViPReal64 temperatureLED4,
                                                ViPReal64 temperatureLED5,
                                                ViPReal64 temperatureLED6);
// OLD following two functions will get obsolete, do not use for new code
ViStatus _VI_FUNC TL6WL_startObserveLED_HeadTemperature (ViSession instrumentHandle,
                                                         LED_HeadTemperHandler temperatureMeasHandler);
ViStatus _VI_FUNC TL6WL_stopObserveLED_HeadTemperature (ViSession instrumentHandle);

// NEW following two functions superseed above two functions, use ViReal64 instead of ViInt16 for temperatures
ViStatus _VI_FUNC TL6WL_startObserveLED_HeadTempEx (ViSession instrumentHandle,
                                                         LED_HeadTempHandler tempMeasHandler);
ViStatus _VI_FUNC TL6WL_stopObserveLED_HeadTempEx (ViSession instrumentHandle);


/// ------------------------------------  M E A S U R E ------------------------------------

ViStatus _VI_FUNC TL6WL_getBoxTemperature (ViSession instrumentHandle,
                                           ViPReal64 boxTemperature);
ViStatus _VI_FUNC TL6WL_getLLGTemperature (ViSession instrumentHandle,
                                           ViPReal64 LLGTemperature);
ViStatus _VI_FUNC TL6WL_getCPUTemperature (ViSession instrumentHandle,
                                           ViPReal64 CPUTemperature);
ViStatus _VI_FUNC TL6WL_getLED_HeadMeasCurrent (ViSession instrumentHandle,
                                                ViPReal64 currentLED1,
                                                ViPReal64 currentLED2,
                                                ViPReal64 currentLED3,
                                                ViPReal64 currentLED4,
                                                ViPReal64 currentLED5,
                                                ViPReal64 currentLED6);
ViStatus _VI_FUNC TL6WL_getLED_HeadMeasCathodeVoltage (ViSession instrumentHandle,
                                                       ViPReal64 cathodeVoltageLED1,
                                                       ViPReal64 cathodeVoltageLED2,
                                                       ViPReal64 cathodeVoltageLED3,
                                                       ViPReal64 cathodeVoltageLED4,
                                                       ViPReal64 cathodeVoltageLED5,
                                                       ViPReal64 cathodeVoltageLED6);

/// -------------------------------------  S O U R C E -------------------------------------
/// ---------------------------  L E D   P O W E R   S T A T E  ----------------------------

ViStatus _VI_FUNC TL6WL_setLED_HeadPowerStates (ViSession instrumentHandle,
                                                ViBoolean enableLED1,
                                                ViBoolean enableLED2,
                                                ViBoolean enableLED3,
                                                ViBoolean enableLED4,
                                                ViBoolean enableLED5,
                                                ViBoolean enableLED6);
ViStatus _VI_FUNC TL6WL_getLED_HeadPowerStates (ViSession instrumentHandle,
                                                ViPBoolean enableLED1,
                                                ViPBoolean enableLED2,
                                                ViPBoolean enableLED3,
                                                ViPBoolean enableLED4,
                                                ViPBoolean enableLED5,
                                                ViPBoolean enableLED6);
ViStatus _VI_FUNC TL6WL_registerPowerStateChangedHandler (ViSession instrumentHandle,
                                                          LED_PowerStateChangedHandler powerStateChangedHandler);

/// -----------------------------  L E D   B R I G H T N E S S -----------------------------

ViStatus _VI_FUNC TL6WL_setLED_HeadBrightness (ViSession instrumentHandle,
                                               ViInt16 brightnessLED1,
                                               ViInt16 brightnessLED2,
                                               ViInt16 brightnessLED3,
                                               ViInt16 brightnessLED4,
                                               ViInt16 brightnessLED5,
                                               ViInt16 brightnessLED6);
ViStatus _VI_FUNC TL6WL_getLED_HeadBrightness (ViSession instrumentHandle,
                                               ViPUInt16 brightnessLED1,
                                               ViPUInt16 brightnessLED2,
                                               ViPUInt16 brightnessLED3,
                                               ViPUInt16 brightnessLED4,
                                               ViPUInt16 brightnessLED5,
                                               ViPUInt16 brightnessLED6);

// OLD following two functions will get obsolete, do not use for new code
ViStatus _VI_FUNC TL6WL_startObserveLED_HeadBrightness (ViSession instrumentHandle,
                                                        LED_HeadBrightnessChangedHandler brightnessChangedHandler);
ViStatus _VI_FUNC TL6WL_stopObserveLED_HeadBrightness (ViSession instrumentHandle);

// NEW following two functions superseed above two functions, use ViReal64 instead of ViInt16 for temperatures
ViStatus _VI_FUNC TL6WL_startObserveLED_HeadBrightnessEx (ViSession instrumentHandle,
                                                            LED_HeadBrightChangedHandler brightChangedHandler);
ViStatus _VI_FUNC TL6WL_stopObserveLED_HeadBrightnessEx (ViSession instrumentHandle);

/// ----------------------------  L E D   L I N E A R   M O D E ----------------------------

ViStatus _VI_FUNC TL6WL_setLED_LinearModeValue (ViSession instrumentHandle,
                                                ViUInt16 brightness);
ViStatus _VI_FUNC TL6WL_getLED_LinearModeValue (ViSession instrumentHandle,
                                                ViPUInt16 brightness);

/// -----------------------------  L E D   H E A D   I N F O -------------------------------

ViStatus _VI_FUNC TL6WL_writeLED_HeadCustomName (ViSession instrumentHandle,
                                                 ViUInt8 LEDNr, ViString LEDName);
ViStatus _VI_FUNC TL6WL_readLED_HeadCustomName (ViSession instrumentHandle,
                                                ViUInt8 LEDNr,
                                                ViChar _VI_FAR LEDName[]);
ViStatus _VI_FUNC TL6WL_readLED_HeadSpectrum (ViSession instrumentHandle,
                                              ViUInt8 LEDNr, ViPUInt16 dataPoints,
                                              ViUInt16 wavelength[],
                                              ViReal64 normPower[]);
ViStatus _VI_FUNC TL6WL_readLED_HeadSpectrumRange (ViSession instrumentHandle,
                                                   ViUInt8 LEDNr,
                                                   ViPUInt16 minWavelength,
                                                   ViPUInt16 maxWavelength);
ViStatus _VI_FUNC TL6WL_readLED_HeadCentroidWL (ViSession instrumentHandle,
                                                ViUInt8 LEDNr,
                                                ViPUInt16 centroidWavelength);
ViStatus _VI_FUNC TL6WL_readLED_HeadPeakWL (ViSession instrumentHandle,
                                            ViUInt8 LEDNr,
                                            ViPUInt16 peakWavelength);
ViStatus _VI_FUNC TL6WL_readLED_HeadFWHMW (ViSession instrumentHandle,
                                           ViUInt8 LEDNr,
                                           ViPUInt16 lowerFWHMWavelength,
                                           ViPUInt16 upperFWHMWavelength);
ViStatus _VI_FUNC TL6WL_readLED_HeadOoe2 (ViSession instrumentHandle, ViUInt8 LEDNr,
                                          ViPUInt16 lower1eWavelength,
                                          ViPUInt16 upper1eWavelength);
ViStatus _VI_FUNC TL6WL_readLED_HeadPmaxCC (ViSession instrumentHandle, ViUInt8 LEDNr,
                                          ViPReal64 maxPower,
                                          ViPReal64 couplingCoefficient);
ViStatus _VI_FUNC TL6WL_readLED_HeadsName (ViSession instrumentHandle,
                                           ViChar _VI_FAR nameLED1[],
                                           ViChar _VI_FAR nameLED2[],
                                           ViChar _VI_FAR nameLED3[],
                                           ViChar _VI_FAR nameLED4[],
                                           ViChar _VI_FAR nameLED5[],
                                           ViChar _VI_FAR nameLED6[]);
ViStatus _VI_FUNC TL6WL_readLED_HeadsSerial (ViSession instrumentHandle,
                                             ViChar _VI_FAR SNLED1[],
                                             ViChar _VI_FAR SNLED2[],
                                             ViChar _VI_FAR SNLED3[],
                                             ViChar _VI_FAR SNLED4[],
                                             ViChar _VI_FAR SNLED5[],
                                             ViChar _VI_FAR SNLED6[]);
ViStatus _VI_FUNC TL6WL_readLED_HeadsManfDate (ViSession instrumentHandle,
                                               ViChar _VI_FAR dateLED1[],
                                               ViChar _VI_FAR dateLED2[],
                                               ViChar _VI_FAR dateLED3[],
                                               ViChar _VI_FAR dateLED4[],
                                               ViChar _VI_FAR dateLED5[],
                                               ViChar _VI_FAR dateLED6[]);
ViStatus _VI_FUNC TL6WL_readLED_HeadsColorName (ViSession instrumentHandle,
                                                ViChar _VI_FAR colorLED1[],
                                                ViChar _VI_FAR colorLED2[],
                                                ViChar _VI_FAR colorLED3[],
                                                ViChar _VI_FAR colorLED4[],
                                                ViChar _VI_FAR colorLED5[],
                                                ViChar _VI_FAR colorLED6[]);
ViStatus _VI_FUNC TL6WL_readLED_HeadsMaxCurrent (ViSession instrumentHandle,
                                                 ViPReal64 currentLED1,
                                                 ViPReal64 currentLED2,
                                                 ViPReal64 currentLED3,
                                                 ViPReal64 currentLED4,
                                                 ViPReal64 currentLED5,
                                                 ViPReal64 currentLED6);
ViStatus _VI_FUNC TL6WL_readLED_HeadsTempCool (ViSession instrumentHandle,
                                               ViPReal64 coolTempLED1,
                                               ViPReal64 coolTempLED2,
                                               ViPReal64 coolTempLED3,
                                               ViPReal64 coolTempLED4,
                                               ViPReal64 coolTempLED5,
                                               ViPReal64 coolTempLED6);
ViStatus _VI_FUNC TL6WL_readLED_HeadsTempHot (ViSession instrumentHandle,
                                              ViPReal64 hotTempLED1,
                                              ViPReal64 hotTempLED2,
                                              ViPReal64 hotTempLED3,
                                              ViPReal64 hotTempLED4,
                                              ViPReal64 hotTempLED5,
                                              ViPReal64 hotTempLED6);
ViStatus _VI_FUNC TL6WL_readLED_HeadsTempMax (ViSession instrumentHandle,
                                              ViPReal64 maxTempLED1,
                                              ViPReal64 maxTempLED2,
                                              ViPReal64 maxTempLED3,
                                              ViPReal64 maxTempLED4,
                                              ViPReal64 maxTempLED5,
                                              ViPReal64 maxTempLED6);
ViStatus _VI_FUNC TL6WL_readLED_HeadsMaxFreq (ViSession instrumentHandle,
                                              ViPReal64 maxFreqLED1,
                                              ViPReal64 maxFreqLED2,
                                              ViPReal64 maxFreqLED3,
                                              ViPReal64 maxFreqLED4,
                                              ViPReal64 maxFreqLED5,
                                              ViPReal64 maxFreqLED6);
ViStatus _VI_FUNC TL6WL_readLED_HeadsMaxUfwd (ViSession instrumentHandle,
                                              ViPReal64 maxUfwdLED1,
                                              ViPReal64 maxUfwdLED2,
                                              ViPReal64 maxUfwdLED3,
                                              ViPReal64 maxUfwdLED4,
                                              ViPReal64 maxUfwdLED5,
                                              ViPReal64 maxUfwdLED6);
ViStatus _VI_FUNC TL6WL_readLED_HeadsTypUfwd (ViSession instrumentHandle,
                                              ViPReal64 typUfwdLED1,
                                              ViPReal64 typUfwdLED2,
                                              ViPReal64 typUfwdLED3,
                                              ViPReal64 typUfwdLED4,
                                              ViPReal64 typUfwdLED5,
                                              ViPReal64 typUfwdLED6);
ViStatus _VI_FUNC TL6WL_readLED_HeadsColorCode (ViSession instrumentHandle,
                                                ViUInt32 *colorCodeLED1,
                                                ViUInt32 *colorCodeLED2,
                                                ViUInt32 *colorCodeLED3,
                                                ViUInt32 *colorCodeLED4,
                                                ViUInt32 *colorCodeLED5,
                                                ViUInt32 *colorCodeLED6);
ViStatus _VI_FUNC TL6WL_readLED_HeadTimeMeter (ViSession instrumentHandle,
                                               ViPUInt32 timeMeterLED1,
                                               ViPUInt32 timeMeterLED2,
                                               ViPUInt32 timeMeterLED3,
                                               ViPUInt32 timeMeterLED4,
                                               ViPUInt32 timeMeterLED5,
                                               ViPUInt32 timeMeterLED6);

ViStatus _VI_FUNC TL6WL_startAdaptionLED (ViSession instrumentHandle, ViUInt8 LEDNr);
ViStatus _VI_FUNC TL6WL_stopAdaptionLED (ViSession instrumentHandle);
ViStatus _VI_FUNC TL6WL_readAdaptionProgressLED (ViSession instrumentHandle, ViReal64 *progres);
ViStatus _VI_FUNC TL6WL_readAdaptionProgressLED_Ex (ViSession instrumentHandle, ViUInt32 *pMaxSteps, ViUInt32 *pCurStep, ViChar *stepName, ViReal64 *stepProgres, ViReal64 *oaProgres);
ViStatus _VI_FUNC TL6WL_writeAdaptionParameterLED (ViSession instrumentHandle, ViUInt32 paramIdx, ViInt32  paramI32, ViReal64  paramF64);
ViStatus _VI_FUNC TL6WL_readAdaptionParameterLED  (ViSession instrumentHandle, ViUInt32 paramIdx, ViInt32 *paramI32, ViReal64 *paramF64);


/// -------------------------------  F I L T E R   H E A D  --------------------------------

ViStatus _VI_FUNC TL6WL_readFilter_HeadsName (ViSession instrumentHandle,
                                              ViChar _VI_FAR nameFilter1[],
                                              ViChar _VI_FAR nameFilter2[],
                                              ViChar _VI_FAR nameFilter3[],
                                              ViChar _VI_FAR nameFilter4[],
                                              ViChar _VI_FAR nameFilter5[]);
ViStatus _VI_FUNC TL6WL_readFilter_HeadsSerial (ViSession instrumentHandle,
                                                ViChar _VI_FAR SNFilter1[],
                                                ViChar _VI_FAR SNFilter2[],
                                                ViChar _VI_FAR SNFilter3[],
                                                ViChar _VI_FAR SNFilter4[],
                                                ViChar _VI_FAR SNFilter5[]);
ViStatus _VI_FUNC TL6WL_readFilter_HeadsManfDate (ViSession instrumentHandle,
                                                  ViChar _VI_FAR dateFilter1[],
                                                  ViChar _VI_FAR dateFilter2[],
                                                  ViChar _VI_FAR dateFilter3[],
                                                  ViChar _VI_FAR dateFilter4[],
                                                  ViChar _VI_FAR dateFilter5[]);
ViStatus _VI_FUNC TL6WL_readFilterSpectrum (ViSession instrumentHandle,
                                            ViUInt8 filterNr, ViPUInt16 dataPoints,
                                            ViUInt16 wavelength[],
                                            ViReal64 normPower[]);
ViStatus _VI_FUNC TL6WL_readFilterSpectrumRange (ViSession instrumentHandle,
                                                 ViUInt8 filterNr,
                                                 ViPUInt16 minWavelength,
                                                 ViPUInt16 maxWavelength);
ViStatus _VI_FUNC TL6WL_readFilterTransmissionInterval (ViSession instrumentHandle,
                                                        ViUInt8 filterNr,
                                                        ViPUInt16 startTransWavelength,
                                                        ViPUInt16 endTransWavelength);
ViStatus _VI_FUNC TL6WL_readFilterReflectionInterval (ViSession instrumentHandle,
                                                      ViUInt8 filterNr,
                                                      ViPUInt16 startReflectionWavelength,
                                                      ViPUInt16 endReflectionWavelength);
ViStatus _VI_FUNC TL6WL_readFilterCutOffWL (ViSession instrumentHandle,
                                            ViUInt8 filterNr,
                                            ViPUInt16 cutOffWavelength);

/// -------------------------------  T I M I N G   U N I T  --------------------------------

ViStatus _VI_FUNC TL6WL_TU_ProgramTestSequence (ViSession instrumentHandle,
                                                ViBoolean activeLow,
                                                ViUInt32 activeTimeus);
ViStatus _VI_FUNC TL6WL_TU_ResetSequence (ViSession instrumentHandle);
ViStatus _VI_FUNC TL6WL_TU_AddGeneratedSelfRunningSignal (ViSession instrumentHandle,
                                                          ViUInt8 signalNr,
                                                          ViBoolean activeLow,
                                                          ViUInt32 startDelayus,
                                                          ViUInt32 activeTimeus,
                                                          ViUInt32 inactiveTimeus,
                                                          ViUInt32 repititionCount);
ViStatus _VI_FUNC TL6WL_TU_AddGeneratedTriggeredSignal (ViSession instrumentHandle,
                                                        ViUInt8 signalNr,
                                                        ViBoolean activeLow,
                                                        ViUInt32 startDelayus,
                                                        ViUInt32 activeTimeus,
                                                        ViUInt32 inactiveTimeus,
                                                        ViUInt32 repititionCount);
ViStatus _VI_FUNC TL6WL_TU_AddDirectlyTriggeredSignal (ViSession instrumentHandle,
                                                       ViUInt8 signalNr);
ViStatus _VI_FUNC TL6WL_TU_AddTriggerPoint (ViSession instrumentHandle,
                                            ViUInt8 signalNr, ViBoolean startsLow,
                                            ViUInt32 edgeCount,
                                            ViInt16 affectedSignalBitmask);

ViStatus _VI_FUNC TL6WL_TU_StartStopGeneratorOutput_TU (ViSession instrumentHandle,
                                                        ViBoolean start);





#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

#endif   /* _TL6WL_DRIVER_HEADER_ */

