/****************************************************************************

   Thorlabs CHORLIS SERVICE VISA instrument driver

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


#ifndef _TL6WL_SERVICE_DRIVER_HEADER_
#define _TL6WL_SERVICE_DRIVER_HEADER_
#include "visatype.h"
#include "stdint.h"
#if defined(__cplusplus) || defined(__cplusplus__)
extern "C"
{
#endif


typedef struct
{
   const char* name;
   const char* serialNr;
   const char* colorName;
   const char* manfDate;
   ViReal64 forwardCurr_A;
   ViReal64 forwardVoltTyp_V;
   ViReal64 forwardVoltMax_V;
   ViReal64 Tcool_K;
   ViReal64 Thot_K;
   ViReal64 Tmax_K;
   ViReal64 fMax_Hz;

   ViByte representingColorRGB[3];
} LED_HeadAdjustmentParameters;


typedef struct
{
   const char* name;
   const char* serialNr;
   const char* manfDate;
} FilterAdjustmentParameters;

typedef struct
{
   ViUInt16 specMinWL_nm;
   ViUInt16 specMaxWL_nm;
   ViUInt16 peakWL_nm;
   ViUInt16 WL_FWHMWL_Start_nm;
   ViUInt16 WL_FWHMWL_End_nm;
   ViUInt16 WL_E_Start_nm;
   ViUInt16 WL_E_End_nm;
   ViUInt16 centroid_nm;
} LED_HeadSpectrumParameter;

typedef struct
{
   float    x;
   float    y;
} adjustPoint_t;

/* with the limit of 12 adjustment points sizeof(AdjustmentObjectNvmem_t) (116) remains smaller than WL6_SPEC_BUF_SIZE (120)
 * which is the biggest buffer available for transmission right now
 */
#define  WL6_ADJOBJ_PTS_PERAO    12

typedef struct
{
   int            numObj;  // number of adjustment points in this object, must be in range 2 ... NUM_AO_PTS
   float          limXlo;  // lowest  allowed x-value
   float          limXhi;  // highest allowed x-value
   float          limYlo;  // lowest  allowed y-value
   float          limYhi;  // highest allowed y-value
   adjustPoint_t  aps[WL6_ADJOBJ_PTS_PERAO];
} AdjustmentObjectNvmem_t;

typedef struct
{
   ViUInt16 Idac;
   ViReal64 Iled;
   ViReal64 Ucath;
} LED_HeadCurvePoint;

// struct as used in uC and transmission via USB
_Pragma("pack(push, 1)")
typedef struct
{
   unsigned short Idac_U16;
   float          Iled_F32;
   float          Ucath_F32;
} LED_DeviceHeadCurvePoint;
_Pragma("pack()")
/*#define PACKED_DATA_START  "pack(push, 1)"
#define PACKED
#define PACKED_DATA_END    "pack()"
*/
#define LED_HEAD_MAX_CURVE_DATA_POINTS                   (640)

typedef struct
{
   ViReal64 Ithreshold;       // threshold where to switch ranges 0 <==> 1
   ViUInt16 numPtsLoRange;
   ViUInt16 numPtsHiRange;
} LED_HeadCurvesParameter;

typedef struct
{
    uint16_t specMinWL_nm;
    uint16_t specMaxWL_nm;
    uint16_t startReflexionWL_nm;
    uint16_t endReflexionWL_nm;
    uint16_t startTransmissionWL_nm;
    uint16_t endTransmissionWL_nm;
    uint16_t cutOffWL_nm;
} FilterSpectrumParameter;

#define TL6WL_LED_1 0
#define TL6WL_LED_2 1
#define TL6WL_LED_3 2
#define TL6WL_LED_4 3
#define TL6WL_LED_5 4
#define TL6WL_LED_6 5
typedef enum
{
    LED_6WL_0 = TL6WL_LED_1,
    LED_6WL_1 = TL6WL_LED_2,
    LED_6WL_2 = TL6WL_LED_3,
    LED_6WL_3 = TL6WL_LED_4,
    LED_6WL_4 = TL6WL_LED_5,
    LED_6WL_5 = TL6WL_LED_6,
    AMOUNT_OF_6WL_LEDS,         ///< Needs to be last element to count amount of LEDs
} LED_6WL_Nr;

#define TL6WL_FILTER_1 0
#define TL6WL_FILTER_2 1
#define TL6WL_FILTER_3 2
#define TL6WL_FILTER_4 3
#define TL6WL_FILTER_5 4
typedef enum
{
   FILTER_6WL_0 = TL6WL_FILTER_1,
   FILTER_6WL_1 = TL6WL_FILTER_2,
   FILTER_6WL_2 = TL6WL_FILTER_3,
   FILTER_6WL_3 = TL6WL_FILTER_4,
   FILTER_6WL_4 = TL6WL_FILTER_5,
   AMOUNT_OF_6WL_FILTERS,      ///< Needs to be last element to count amount of dichroit filters
} Filter6WL_Nr;


/*========================================================================*//**
\enum LedSlotState_t
\brief   Mask bits for info about LED/LED slot
         This is a copy of enumeration (same name) of ledModule.h of firmware
*//*=========================================================================*/
typedef enum
{
   LED_NVMEM_RD_MASK_UUID        = (1 << 0),    ///< flag is set on success of reading LED head UUID
   LED_NVMEM_RD_MASK_TIME        = (1 << 1),    ///< flag is set on success of reading LED head time meter
   LED_NVMEM_RD_MASK_ID          = (1 << 2),    ///< flag is set on success of reading LED head basic ID and adjust data
   LED_NVMEM_RD_MASK_SPEC        = (1 << 3),    ///< flag is set on success of reading LED head spectrum data
   LED_NVMEM_RD_MASK_CURVES      = (1 << 4),    ///< flag is set on success of reading LED head I-DAC vs. Iled, Ucath curves
   LED_NVMEM_RD_MASK_LI          = (1 << 5),    ///< flag is set on success of reading LED head L-I curve
   LED_NVMEM_RD_MASK_NAME        = (1 << 6),    ///< flag is set on success of reading LED head custom name
   LED_NVMEM_RD_MASK_ED0         = (1 << 7),    ///< flag is set on success of reading LED head coupling coefficients
   LED_FREE_BIT_8                = (1 << 8),    ///< unused flag 8
   LED_NVMEM_RD_MASK_NVOK        = (1 << 9),    ///< flag is set when at least one access to LED head nvmem succeeded
   LED_NVMEM_RD_MASK_NVCRC       = (1 << 10),   ///< flag is set when at least one access to LED head nvmem reported CRC error
   LED_NVMEM_RD_MASK_NVHWERR     = (1 << 11),   ///< flag is set when at least one access to LED head nvmem reported hardware error
   LED_NVMEM_RD_MASK_NVPARAM     = (1 << 12),   ///< flag is set when at least one access to LED head nvmem reported parameter error (software error)

   LED_POWERSTATUS_UNKNOWN       = (1 << 13),   ///< power status code when LED status is still unknown (missing or not yet initialized)
   LED_POWERSTATUS_DISABLED      = (2 << 13),   ///< power status code when LED nvmem is read out, but LED still switched off
   LED_POWERSTATUS_ENABLED       = (3 << 13),   ///< power status code when LED is switched on
   LED_POWERSTATUS_WRONG_PLACED  = (4 << 13),   ///< power status code when LED is placed in wrong slot
   LED_POWERSTATUS_NEEDS_ADJUST  = (5 << 13),   ///< power status code when LED needs to be adjusted for its slot
   LED_POWERSTATUS_ERROR         = (6 << 13),   ///< power status code when any of the LED head nvmem readouts failed
   LED_POWERSTATUS_MASK          = (7 << 13),   ///< mask for bits[13:15] for above power status codes

   LED_TEMPSTATUS_UNKNOWN        = (0 << 16),   ///< temperature status when LED temperature status is unknown
   LED_TEMPSTATUS_OK             = (1 << 16),   ///< temperature status when LED temperature is OK, i.e. in regular range
   LED_TEMPSTATUS_OTP            = (2 << 16),   ///< temperature status when LED temperature is too high
   LED_TEMPSTATUS_MEAS_ERR       = (3 << 16),   ///< temperature status when LED temperature measurement failed
   LED_TEMPSTATUS_MASK           = (3 << 16),   ///< mask for bits[16:17] for above temperature status codes

   LED_SLOT_LED_ENA              = (1 << 18),   ///< flag is set when mainboard LED is enabled (should shine)
   LED_SLOT_LED_BLINK            = (1 << 19),   ///< flag is set when mainboard LED is enabled and should blink
   LED_SLOT_PRESENT              = (1 << 20),   ///< flag is set when LED presence is detected (hardware detection)

   LED_SLOT_UUID_OK              = (1 << 21),   ///< flag is set when LED slot reading of UUID is OK
   LED_SLOT_UUID_DEF             = (1 << 22),   ///< flag is set when LED slot reading of UUID lead to using default data
   // when neither LED_SLOT_UUID_OK flag nor LED_SLOT_UUID_DEF is set the access to slot UUID nvmem is broken!

   LED_SLOT_AO_OK                = (1 << 23),   ///< flag is set when LED slot reading of adjustment objects is OK
   LED_SLOT_AO_DEF               = (1 << 24),   ///< flag is set when LED slot reading of adjustment objects lead to using default adj. obj.
   // when neither LED_SLOT_AO_OK flag nor LED_SLOT_AO_DEF is set the access to slot adjustment objects nvmem is broken!

   LED_SLOT_HEAD_OK              = (1 << 25),   ///< flag is set when LED readout of nvmem completed OK
   LED_SLOT_HEAD_DEF             = (1 << 26),   ///< flag is set when LED readout of nvmem lead to using a (at least one) default setting
   LED_SLOT_ERROR                = (1 << 27),   ///< flag is set when LED or slot readout of nvmem had an hardware error
   LED_SLOT_USED                 = (1 << 28),   ///< flag is set when LED can be used, sum of bits LED_SLOT_PRESENT, LED_NVMEM_RD_MASK_UUID, LED_SLOT_AO_OK, LED_SLOT_HEAD_OK
   LED_FREE_BIT_29               = (1 << 29),   ///< unused flag 29
   LED_FREE_BIT_30               = (1 << 30),   ///< unused flag 30
//   LED_FREE_BIT_31               = (1 << 31),    will throw warning 'integer op. out of rng'
} LedSlotState_t;

/*========================================================================*//**
\enum InvalidateNvmem_t
\brief   Mask bits for invalidating NVMEM entries
*//*=========================================================================*/
typedef enum
{
   INVAL_ID_ADJ                  = (1 << 0),    ///< mask bit to invalidate head ID
   INVAL_SPECTRUM                = (1 << 1),    ///< mask bit to invalidate LED head spectrum
   INVAL_CNAME                   = (1 << 2),    ///< mask bit to invalidate LED head custom name
   INVAL_CURVES                  = (1 << 3),    ///< mask bit to invalidate LED head curves (I-DAC vs. Iled, Ucath)
   INVAL_LICURVE                 = (1 << 4),    ///< mask bit to invalidate LED head L-I curve
   INVAL_CCOEF                   = (1 << 5),    ///< mask bit to invalidate LED head coupling coefficients
   INVAL_UUID                    = (1 << 7),    ///< mask bit to invalidate LED head UUID
} InvalidateNvmem_t;

/*========================================================================*//**
\enum DefaultNvmem_t
\brief   Mask bits for defaulting NVMEM entries
*//*=========================================================================*/
typedef enum
{
   DFLT_ID_ADJ                   = (1 << 0),    ///< mask bit to default head ID
   DFLT_SPECTRUM                 = (1 << 1),    ///< mask bit to default LED head spectrum
   DFLT_CNAME                    = (1 << 2),    ///< mask bit to default LED head custom name
   DFLT_CURVES                   = (1 << 3),    ///< mask bit to default LED head curves (I-DAC vs. Iled, Ucath)
   DFLT_LICURVE                  = (1 << 4),    ///< mask bit to default LED head L-I curve
   DFLT_CCOEF                    = (1 << 5),    ///< mask bit to default LED head coupling coefficients
} DefaultNvmem_t;

/*===========================================================================

 Macros

===========================================================================*/

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



/// ------ T H O R L A B S   6 W L   S E R V I C E   I N S T R U M E N T   D R I V E R ------

ViStatus _VI_FUNC TL6WL_serviceAuthenticate (ViSession instrumentHandle, ViString passwd);
ViStatus _VI_FUNC TL6WL_isServiceMode (ViSession instrumentHandle, ViPBoolean isService);
ViStatus _VI_FUNC TL6WL_serviceWriteHeadParameters (ViSession vi, LED_6WL_Nr nr, const LED_HeadAdjustmentParameters* params);
ViStatus _VI_FUNC TL6WL_serviceWriteHeadSpectrum (ViSession instrumentHandle, LED_6WL_Nr nr, const LED_HeadSpectrumParameter* header, const uint8_t* normPower_1nm);
ViStatus _VI_FUNC TL6WL_serviceWriteHeadCurves (ViSession vi, LED_6WL_Nr nr, const LED_HeadCurvesParameter* header, const LED_HeadCurvePoint* cp);
ViStatus _VI_FUNC TL6WL_serviceQueryHeadCurves (ViSession vi, LED_6WL_Nr nr,       LED_HeadCurvesParameter* header,       LED_HeadCurvePoint* cp);
ViStatus _VI_FUNC TL6WL_serviceWriteAdjustObject (ViSession vi, LED_6WL_Nr nr, int aoIdx, const AdjustmentObjectNvmem_t*  pao);
ViStatus _VI_FUNC TL6WL_serviceQueryAdjustObject (ViSession vi, LED_6WL_Nr nr, int aoIdx,       AdjustmentObjectNvmem_t*  pao);
ViStatus _VI_FUNC TL6WL_serviceWriteFilterParameters (ViSession vi, Filter6WL_Nr nr, const FilterAdjustmentParameters* params);
ViStatus _VI_FUNC TL6WL_serviceWriteFilterSpectrum (ViSession vi, Filter6WL_Nr nr, const FilterSpectrumParameter* header, const uint8_t* normAtten_1nm);
ViStatus _VI_FUNC TL6WL_serviceSetCouplingCoeffs (ViSession instrumentHandle, LED_6WL_Nr nr, ViReal64 *ccoeff);
ViStatus _VI_FUNC TL6WL_serviceGetCouplingCoeffs (ViSession instrumentHandle, LED_6WL_Nr nr, ViReal64 *ccoeff);
ViStatus _VI_FUNC TL6WL_serviceWriteFanMode (ViSession instrumentHandle, ViUInt8 addr, ViUInt8 mode);
ViStatus _VI_FUNC TL6WL_serviceQueryFanMode (ViSession instrumentHandle, ViUInt8 addr, ViPUInt8 mode);
ViStatus _VI_FUNC TL6WL_serviceWriteFanSpeed (ViSession instrumentHandle, ViUInt8 addr, ViReal64 voltsOrRPM);
ViStatus _VI_FUNC TL6WL_serviceQueryFanSpeed (ViSession instrumentHandle, ViUInt8 addr, ViPReal64 voltsOrRPM);
ViStatus _VI_FUNC TL6WL_serviceQueryBoxUUID (ViSession instrumentHandle, ViPUInt8 uuid);
ViStatus _VI_FUNC TL6WL_serviceQueryLED_UUID (ViSession instrumentHandle, LED_6WL_Nr nr, ViPUInt8 uuid);
ViStatus _VI_FUNC TL6WL_serviceQueryFilterUUID (ViSession instrumentHandle, Filter6WL_Nr nr, ViPUInt8 uuid);
ViStatus _VI_FUNC TL6WL_serviceMTD(ViSession instrumentHandle, ViConstString request, ViString response);
ViStatus _VI_FUNC TL6WL_serviceProgMTD(ViSession instrumentHandle);
ViStatus _VI_FUNC TL6WL_serviceGetADC(ViSession instrumentHandle, LED_6WL_Nr nr, ViUInt8 addr, ViUInt32 *val);
ViStatus _VI_FUNC TL6WL_serviceSetDAC(ViSession instrumentHandle, LED_6WL_Nr nr, ViUInt8 addr, ViUInt16 val, ViUInt8 range);

ViStatus _VI_FUNC TL6WL_serviceSetDeviceName(ViSession instrumentHandle, ViString devName);
ViStatus _VI_FUNC TL6WL_serviceSetSerialNumber(ViSession instrumentHandle, ViString serNum);
ViStatus _VI_FUNC TL6WL_serviceSetManufacturer(ViSession instrumentHandle, ViString manuf);
ViStatus _VI_FUNC TL6WL_serviceSetManufactureDate(ViSession instrumentHandle, ViString manufDate);
ViStatus _VI_FUNC TL6WL_serviceSetCalString(ViSession instrumentHandle, ViString calString);

ViStatus _VI_FUNC TL6WL_serviceGetDeviceName(ViSession instrumentHandle, ViString devName);
ViStatus _VI_FUNC TL6WL_serviceGetSerialNumber(ViSession instrumentHandle, ViString serNum);
ViStatus _VI_FUNC TL6WL_serviceGetManufacturer(ViSession instrumentHandle, ViString manuf);
ViStatus _VI_FUNC TL6WL_serviceGetManufactureDate(ViSession instrumentHandle, ViString manufDate);
ViStatus _VI_FUNC TL6WL_serviceGetCalString(ViSession instrumentHandle, ViString devName);

ViStatus _VI_FUNC TL6WL_serviceSetString(ViSession instrumentHandle, ViUInt16 strIdx, ViString string);
ViStatus _VI_FUNC TL6WL_serviceGetString(ViSession instrumentHandle, ViUInt16 strIdx, ViString string);

ViStatus _VI_FUNC TL6WL_TU_serviceAuthenticate (ViSession instrumentHandle, ViString passwd);
ViStatus _VI_FUNC TL6WL_TU_isServiceMode (ViSession instrumentHandle, ViPBoolean isService);
ViStatus _VI_FUNC TL6WL_TU_serviceSetModulation (ViSession instrumentHandle, ViUInt8 ledBitMask);
ViStatus _VI_FUNC TL6WL_TU_serviceSetSigDir (ViSession instrumentHandle, ViUInt8 ledBitMask);
ViStatus _VI_FUNC TL6WL_TU_serviceSetLED (ViSession instrumentHandle, ViUInt8 ledBitMask);
ViStatus _VI_FUNC TL6WL_TU_serviceGetLED (ViSession instrumentHandle, ViUInt8 *ledBitMask);
ViStatus _VI_FUNC TL6WL_TU_serviceSetAUX (ViSession instrumentHandle, ViUInt8 auxBitMask);
ViStatus _VI_FUNC TL6WL_TU_serviceGetAUX (ViSession instrumentHandle, ViUInt8 *auxBitMask);

ViStatus _VI_FUNC TL6WL_serviceGetLedStatus    (ViSession instrumentHandle, ViUInt32 *ledBits0, ViUInt32 *ledBits1, ViUInt32 *ledBits2, ViUInt32 *ledBits3, ViUInt32 *ledBits4, ViUInt32 *ledBits5);
ViStatus _VI_FUNC TL6WL_serviceGetFilterStatus (ViSession instrumentHandle, ViUInt32 *dicBits0, ViUInt32 *dicBits1, ViUInt32 *dicBits2, ViUInt32 *dicBits3, ViUInt32 *dicBits4);
ViStatus _VI_FUNC TL6WL_serviceResetLedNvmem   (ViSession instrumentHandle, LED_6WL_Nr nr, ViUInt8 flags);
ViStatus _VI_FUNC TL6WL_serviceDefaultLedNvmem (ViSession instrumentHandle, LED_6WL_Nr nr, ViUInt8 flags);

ViStatus _VI_FUNC UC_TL6WL_TU_StartStopGeneratorOutput_TU (ViSession instrumentHandle, ViBoolean start);
ViStatus _VI_FUNC UC_TL6WL_serviceQueryHeadCurves (ViSession vi, LED_6WL_Nr nr,       LED_HeadCurvesParameter* header,       LED_HeadCurvePoint* cp);
ViStatus _VI_FUNC UC_TL6WL_serviceWriteHeadCurves (ViSession vi, LED_6WL_Nr nr, const LED_HeadCurvesParameter* header, const LED_HeadCurvePoint* cp);
ViStatus _VI_FUNC UC_TL6WL_serviceQueryAdjustObject (ViSession vi, LED_6WL_Nr nr, int aoIdx,       AdjustmentObjectNvmem_t*  pao);
ViStatus _VI_FUNC UC_TL6WL_readLED_HeadsMaxCurrent (ViSession instrumentHandle, ViPReal64 cL1, ViPReal64 cL2, ViPReal64 cL3, ViPReal64 cL4, ViPReal64 cL5, ViPReal64 cL6);
ViStatus _VI_FUNC UC_TL6WL_serviceGetADC(ViSession instrumentHandle, LED_6WL_Nr nr, ViUInt8 addr, ViUInt32 *val);
ViStatus _VI_FUNC UC_TL6WL_serviceSetDAC(ViSession instrumentHandle, LED_6WL_Nr nr, ViUInt8 addr, ViUInt16 val, ViUInt8 range);
ViStatus _VI_FUNC UC_TL6WL_serviceAuthenticate (ViSession instrumentHandle, ViString passwd);
ViStatus _VI_FUNC UC_TL6WL_isServiceMode (ViSession instrumentHandle, ViPBoolean isService);

/* following function are not (yet?) needed for LED adaption code, but implemented to be prepared */
ViStatus _VI_FUNC UC_TL6WL_TU_serviceAuthenticate (ViSession instrumentHandle, ViString passwd);
ViStatus _VI_FUNC UC_TL6WL_TU_isServiceMode (ViSession instrumentHandle, ViPBoolean isService);
ViStatus _VI_FUNC UC_TL6WL_TU_serviceSetModulation (ViSession instrumentHandle, ViUInt8 ledBitMask);
ViStatus _VI_FUNC UC_TL6WL_TU_serviceSetSigDir (ViSession instrumentHandle, ViUInt8 ledBitMask);
ViStatus _VI_FUNC UC_TL6WL_TU_serviceSetLED (ViSession instrumentHandle, ViUInt8 ledBitMask);
ViStatus _VI_FUNC UC_TL6WL_TU_serviceGetLED (ViSession instrumentHandle, ViUInt8 *ledBitMask);
ViStatus _VI_FUNC UC_TL6WL_TU_serviceSetAUX (ViSession instrumentHandle, ViUInt8 auxBitMask);
ViStatus _VI_FUNC UC_TL6WL_TU_serviceGetAUX (ViSession instrumentHandle, ViUInt8 *auxBitMask);

ViStatus _VI_FUNC UC_TL6WL_serviceResetLedNvmem   (ViSession instrumentHandle, LED_6WL_Nr nr, ViUInt8 flags);
ViStatus _VI_FUNC UC_TL6WL_serviceDefaultLedNvmem   (ViSession instrumentHandle, LED_6WL_Nr nr, ViUInt8 flags);



#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

#endif   /* _TL6WL_SERVICE_DRIVER_HEADER_ */

