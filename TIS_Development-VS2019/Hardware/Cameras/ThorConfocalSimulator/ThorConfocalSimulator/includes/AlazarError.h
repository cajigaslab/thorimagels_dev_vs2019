#ifndef __ALAZARERROR_H
#define __ALAZARERROR_H
//=============================================================================
//
// Alazar Technologies Inc
// 
//  File Name:
//
//      AlazarError.h
//
// Copyright (c) 2006 Alazar Technologies Inc. All Rights Reserved.
// Unpublished - rights reserved under the Copyright laws of the
// United States And Canada.
//
// This product contains confidential information and trade secrets 
// of Alazar Technologies Inc. Use, disclosure, or reproduction is 
// prohibited without the prior express written permission of Alazar 
// Technologies Inc
//
// Description:
//
//      Error constants file
//
// Revision History:
//
//=============================================================================

#ifdef __cplusplus
extern "C" {
#endif


/******************************************
*             Definitions
******************************************/

//General Errors
#define API_RETURN_CODE_STARTS		0x200   /* Starting return code */

/* API Return Code Values */
typedef enum _RETURN_CODE
{
    ApiSuccess = API_RETURN_CODE_STARTS,
    ApiFailed,
    ApiAccessDenied,
    ApiDmaChannelUnavailable,
    ApiDmaChannelInvalid,
    ApiDmaChannelTypeError,
    ApiDmaInProgress,
    ApiDmaDone,
    ApiDmaPaused,
    ApiDmaNotPaused,
    ApiDmaCommandInvalid,
    ApiDmaManReady,
    ApiDmaManNotReady,
    ApiDmaInvalidChannelPriority,
    ApiDmaManCorrupted,
    ApiDmaInvalidElementIndex,
    ApiDmaNoMoreElements,
    ApiDmaSglInvalid,
    ApiDmaSglQueueFull,
    ApiNullParam,
    ApiInvalidBusIndex,
    ApiUnsupportedFunction,
    ApiInvalidPciSpace,
    ApiInvalidIopSpace,
    ApiInvalidSize,
    ApiInvalidAddress,
    ApiInvalidAccessType,
    ApiInvalidIndex,
    ApiMuNotReady,
    ApiMuFifoEmpty,
    ApiMuFifoFull,
    ApiInvalidRegister,
    ApiDoorbellClearFailed,
    ApiInvalidUserPin,
    ApiInvalidUserState,
    ApiEepromNotPresent,
    ApiEepromTypeNotSupported,
    ApiEepromBlank,
    ApiConfigAccessFailed,
    ApiInvalidDeviceInfo,
    ApiNoActiveDriver,
    ApiInsufficientResources,
    ApiObjectAlreadyAllocated,
    ApiAlreadyInitialized,
    ApiNotInitialized,
    ApiBadConfigRegEndianMode,
    ApiInvalidPowerState,
    ApiPowerDown,
    ApiFlybyNotSupported,
    ApiNotSupportThisChannel,
    ApiNoAction,
    ApiHSNotSupported,
    ApiVPDNotSupported,
    ApiVpdNotEnabled,
    ApiNoMoreCap,
    ApiInvalidOffset,
    ApiBadPinDirection,
    ApiPciTimeout,
    ApiDmaChannelClosed,
    ApiDmaChannelError,
    ApiInvalidHandle,
    ApiBufferNotReady,
    ApiInvalidData,
    ApiDoNothing,
    ApiDmaSglBuildFailed,
    ApiPMNotSupported,
    ApiInvalidDriverVersion,
    ApiWaitTimeout,
    ApiWaitCanceled,
	ApiBufferTooSmall, 
	ApiBufferOverflow,
	ApiInvalidBuffer,
	ApiInvalidRecordsPerBuffer,
	ApiDmaPending,
	ApiLockAndProbePagesFailed,
	ApiWaitAbandoned,
	ApiWaitFailed,
	ApiTransferComplete,
    ApiLastError               // Do not add API errors below this line
} RETURN_CODE;



#ifdef __cplusplus
}
#endif

#endif //__ALAZARERROR_H
