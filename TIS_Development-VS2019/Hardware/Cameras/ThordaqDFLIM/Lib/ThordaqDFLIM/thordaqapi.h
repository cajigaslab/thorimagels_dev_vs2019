// -------------------------------------------------------------------------
// 
// PRODUCT:			ThorDAQ Driver
// MODULE NAME:		thordaqapi.h
// 
// MODULE DESCRIPTION: 
// 
// Contains defines, structures and exported functions for the DLL-like interface.
// 
// $Revision:  $
//
// ------------------------- CONFIDENTIAL ----------------------------------
// 
//              Copyright (c) 2016 by ThorLabs Imaging Research Group, LLC.   
//                       All rights reserved. 
// 
// -------------------------------------------------------------------------

#ifndef __THORDAQAPI_H__
#define __THORDAQAPI_H__


#include "stdafx.h"

#ifndef ___THORDAQCMD_H___
#include "thordaqcmd.h"
#endif

#ifndef __THORDAQRES__h__
#include "thordaqres.h"
#endif
#ifdef __cplusplus
extern "C"
{
#endif
// TODO: reference additional headers your program requires here
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the THORDAQ_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// THORDAQ_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef THORDAQDFLIM_EXPORTS
#define THORDAQ_API __declspec(dllexport)
#else
#define THORDAQ_API __declspec(dllimport)
#endif
// ----------------------
// Externally Visible API

// ConnectToBoard
//
// Connects to a board.  'board' is used to select between
//   multiple board instances (more than 1 board may be present);
//   If the board class has not been created, it does it now.

THORDAQ_API THORDAQ_STATUS ThordaqConnectToBoard
	(
	UINT				boardNum
	);

// DisconnectFromBoard
//
// Disconnect from a board.  'board' is used to select between
//   multiple board instances (more than 1 board may be present);
// If the board class has not been created, return;  
//   otherwise, call DisconnectFromBoard.  Does not delete the board class.

THORDAQ_API THORDAQ_STATUS ThordaqDisconnectFromBoard
	(
	UINT boardNum                  // Board to target
	);

// GetBoardCfg
//
// Get Board Configuration from the Driver
//
// The Driver auto discovers the board's resources during Driver
//   initialization (via hardware capabilities and configuration
//   register advertisements) and places the information in a
//   BOARD_CONFIG_STRUCT structure. BOARD_CONFIG_STRUCT provides
//   the necessary information about the board so that the
//   application knows what resources are available and how to
//   access them.
//
// GetBoardCfg gets a copy of the current BOARD_CONFIG_STRUCT
//   structure kept by the Driver.  No hardware accesses are
//   initiated by calling GetBoardCfg.

THORDAQ_API THORDAQ_STATUS ThordaqGetBoardCfg
	(
	UINT					boardNum, // Board to target
	BOARD_INFO_STRUCT*	    board_info  // Returned structure
	);

THORDAQ_API THORDAQ_STATUS ThordaqSetImagingConfiguration
	(
	UINT							boardNum, // Board to target
	IMAGING_CONFIGURATION_STRUCT	imaging_config  // Returned structure
	);


// PacketReadEx
//
// Use for Addressable Packet DMA only: 
THORDAQ_API THORDAQ_STATUS ThordaqReadChannel
	(
	UINT    boardNum,          // Board to target
	ULONG   channel,
	ULONG*  buffer_length,            // 
	void*   Buffer,             // 
	double   Timeout_ms
	);

THORDAQ_API THORDAQ_STATUS ThordaqAbortRead
	(
	UINT    boardNum           // Board to target
	);

THORDAQ_API THORDAQ_STATUS ThordaqPacketReadBuffer
	(
	UINT    boardNum,          // Board to target
	ULONG64 Address,           // Start address to read in the card
	ULONG*  Length,            // 
	void*   Buffer,             //
	ULONG   Timeout
	);

THORDAQ_API THORDAQ_STATUS ThordaqPacketWriteBuffer
	(
	UINT     boardNum,          // Board to target
	ULONG64  register_card_offset,           // Start address to read in the card
	ULONG    Length,            // Size of the Packet Write to Data Buffer Count by Byte (FIFO Mode)
	UCHAR*   Buffer,            // Data buffer (Packet Mode)
	ULONG    Timeout            // Generate Timeout error when timeout
	);

THORDAQ_API THORDAQ_STATUS ThordaqStartAcquisition
	(
	UINT    boardNum          // Board to target
	);

THORDAQ_API THORDAQ_STATUS ThordaqSetPacketModeAddressable
	(
	UINT    boardNum,          // Board to target
	bool    enableStreamToDMA  // enable s2mm
	);

THORDAQ_API THORDAQ_STATUS ThordaqStopAcquisition
	(
	UINT    boardNum          // Board to target
	);

THORDAQ_API THORDAQ_STATUS ThordaqWriteReadRegister
    (
    UINT boardNum,             // Board number to target
    UINT read_write_flag,           // 1==Read, 0==Write
    UINT register_bar_num,            // Base Address Register (BAR) to access
    ULONGLONG register_card_offset,   // Offset in BAR to start transfer
    BYTE *buffer,           // Data buffer
    ULONGLONG offset,       // Offset in data buffer to start transfer
    ULONGLONG length,       // Byte length of transfer
    PSTAT_STRUCT status     // Completion Status
    );

THORDAQ_API THORDAQ_STATUS ThordaqSetDACParkValue
	(
	UINT    boardNum,          // Board to target
	ULONG32 outputChannel,
	double outputValue
	);

THORDAQ_API THORDAQ_STATUS ThordaqGetLineTriggerFrequency
	(
	UINT    boardNum,  
	UINT32 sample_rate,
	double& frequency
	);

THORDAQ_API	THORDAQ_STATUS ThordaqGetTotalFrameCount
	(
	UINT    boardNum,  
	UINT32& frame_count
	);

THORDAQ_API THORDAQ_STATUS ThordaqGetExternClockStatus
	(
	UINT    boardNum, 
	ULONG32& isClockedSynced
	);

THORDAQ_API THORDAQ_STATUS ThordaqReSync
	(
	UINT    boardNum
	);

THORDAQ_API THORDAQ_STATUS ThordaqSetCoarseShift
	(
	UINT    boardNum, 
	ULONG32 shift,
	int	channel
	);

THORDAQ_API THORDAQ_STATUS ThordaqSetFineShift
	(
	UINT    boardNum, 
	LONG32  shift,
	int	channel
	);

THORDAQ_API THORDAQ_STATUS ThordaqGetClockFrequency
	(
	UINT    boardNum, 
	int		clockIndex,
	double	&frequency
	);

THORDAQ_API THORDAQ_STATUS ThordaqSetDFLIMSyncingSettings
	(
	UINT    boardNum, 
	ULONG32 syncDelay,
	ULONG32 resyncDelay,
	bool	forceResyncEverLine
	);
THORDAQ_API THORDAQ_STATUS ThordaqSetDLIMFrontEndSettings
	(
	UINT    boardNum
	);


#ifdef __cplusplus
}
#endif

#endif