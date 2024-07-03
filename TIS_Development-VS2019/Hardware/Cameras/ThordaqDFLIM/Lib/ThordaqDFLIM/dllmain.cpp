// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "thordaqapi.h"
#include "thordaqcmd.h"
#include "thordaqDFLIM.h"
#include "windows.h"

#define VALIDATION(x, y)						\
	if (x >= MAXIMUM_NUMBER_OF_BOARDS)			\
	{	return STATUS_INVALID_BOARDNUM; }		\
	if(y[x] == NULL)							\
	{return STATUS_INITIATION_INCOMPLETE; }	

// Local Prototypes
void InitializeDll ();
void CleanupDll ();

// Internal variables
ThordaqDFLIM *DriverList[MAXIMUM_NUMBER_OF_BOARDS];

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		InitializeDll();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		CleanupDll();
		break;
	}
	return TRUE;
}

// InitializeDll
//
// This is called when the DLL is created.
// It initializes any global variables.
void InitializeDll ()
{
	for (int i=0; i < MAXIMUM_NUMBER_OF_BOARDS; i++)
	{
		DriverList[i] = NULL;
	}
}

// CleanupDll
//
// This is called when the DLL is cleaned up.
// This detaches any ongoing connections and cleans up any global variables.
void CleanupDll ()
{
	for (int i=0; i < MAXIMUM_NUMBER_OF_BOARDS; i++)
	{
		if (DriverList[i] != NULL)
		{
			// Make sure we are disconnected
			DriverList[i]->DisconnectFromBoard();
			// Cleanup the class
			delete DriverList[i];
			DriverList[i] = NULL;
		}
	}
}

//--------------------------------------------------------------------
//
//  Public DLL Calls - Main interface to the DLL
//
//--------------------------------------------------------------------

// ConnectToBoard
//
// Connects to a board.  'board' is used to select between
//   multiple board instances (more than 1 board may be present);
//   If the board class has not been created, it does it now.

THORDAQ_API THORDAQ_STATUS ThordaqConnectToBoard
	(
	UINT				boardNum                  // Board to targetThordaqConnectToBoard
	)
{
	if (boardNum >= MAXIMUM_NUMBER_OF_BOARDS)
	{
		return STATUS_INVALID_BOARDNUM;
	}
	if (DriverList[boardNum] == NULL)
	{
		DriverList[boardNum] = new ThordaqDFLIM(boardNum);
	}
	if (DriverList[boardNum] == NULL) //Cannot initiate the board
	{
		return STATUS_INITIATION_INCOMPLETE;		
	}
	return DriverList[boardNum]->ConnectToBoard();
}

// DisconnectFromBoard
//
// Disconnect from a board.  'board' is used to select between
//   multiple board instances (more than 1 board may be present);
// If the board class has not been created, return;  
//   otherwise, call DisconnectFromBoard.  Does not delete the board class.

THORDAQ_API THORDAQ_STATUS ThordaqDisconnectFromBoard
	(
	UINT boardNum                  // Board to target
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DisconnectFromBoard();
}
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
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetBoardCfg(board_info);
}

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
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->PacketReadChannel(channel,buffer_length,Buffer,Timeout_ms);
}

THORDAQ_API THORDAQ_STATUS ThordaqAbortRead
	(
	UINT    boardNum           // Board to target
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->AbortPacketRead();
}

THORDAQ_API THORDAQ_STATUS ThordaqPacketReadBuffer
	(
	UINT    boardNum,          // Board to target
	ULONG64  Address,           // Start address to read in the card
	ULONG*  Length,            // 
	void*   Buffer,             // 
	ULONG   Timeout
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->PacketReadBuffer(Address,Length,Buffer,Timeout);
}

THORDAQ_API THORDAQ_STATUS ThordaqPacketWriteBuffer
	(
	UINT    boardNum,          // Board to target
	ULONG64  register_card_offset,           // Start address to read in the card
	ULONG   Length,            // Size of the Packet Write to Data Buffer Count by Byte (FIFO Mode)
	UCHAR*    Buffer,            // Data buffer (Packet Mode)
	ULONG    Timeout            // Generate Timeout error when timeout
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->PacketWriteBuffer(register_card_offset,Length,Buffer,Timeout);
}

THORDAQ_API THORDAQ_STATUS ThordaqSetImagingConfiguration
	(
	UINT							boardNum, // Board to target
	IMAGING_CONFIGURATION_STRUCT	imaging_config  // Returned structure
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetImagingConfiguration(imaging_config);
}

THORDAQ_API THORDAQ_STATUS ThordaqStartAcquisition
	(
	UINT    boardNum          // Board to target
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->StartAcquisition();
}

THORDAQ_API THORDAQ_STATUS ThordaqSetPacketModeAddressable
	(
	UINT    boardNum,          // Board to target
	bool    enableStreamToDMA  // enable s2mm
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetPacketModeAddressable(enableStreamToDMA);
}


THORDAQ_API THORDAQ_STATUS ThordaqStopAcquisition
	(
	UINT    boardNum          // Board to target
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->StopAcquisition();
}

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
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->WriteReadRegister(read_write_flag, register_bar_num, register_card_offset, buffer, offset, length, status);
}

THORDAQ_API THORDAQ_STATUS ThordaqSetDACParkValue
	(
	UINT    boardNum,          // Board to target
	ULONG32 outputChannel,
	double outputValue
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetDACParkValue(outputChannel,outputValue);
}

THORDAQ_API THORDAQ_STATUS ThordaqGetLineTriggerFrequency
(
UINT    boardNum,  
UINT32 sample_rate,
double& frequency
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetLineTriggerFrequency(sample_rate,frequency);
}

THORDAQ_API	THORDAQ_STATUS ThordaqGetTotalFrameCount
(
UINT    boardNum,  
UINT32& frame_count
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetTotalFrameCount(frame_count);
}

THORDAQ_API THORDAQ_STATUS ThordaqGetExternClockStatus
	(
	UINT    boardNum, 
	ULONG32& isClockedSynced
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetExternClockStatus(isClockedSynced);
}

THORDAQ_API THORDAQ_STATUS ThordaqReSync
	(
	UINT    boardNum
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->ReSync();
}

THORDAQ_API THORDAQ_STATUS ThordaqSetCoarseShift
	(
	UINT    boardNum, 
	ULONG32 shift,
	int	channel
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetCoarseShift(shift, channel);
}

THORDAQ_API THORDAQ_STATUS ThordaqSetFineShift
	(
	UINT    boardNum, 
	LONG32 shift,
	int	channel
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetFineShift(shift, channel);
}

THORDAQ_API THORDAQ_STATUS ThordaqGetClockFrequency
	(
	UINT    boardNum, 
	int		clockIndex,
	double	&ferquency
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetClockFrequency(clockIndex, ferquency);
}


THORDAQ_API THORDAQ_STATUS ThordaqSetDFLIMSyncingSettings
	(
	UINT    boardNum, 
	ULONG32 syncDelay,
	ULONG32 resyncDelay,
	bool	forceResyncEverLine
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetDFLIMSyncingSettings(syncDelay, resyncDelay, forceResyncEverLine);
}

THORDAQ_API THORDAQ_STATUS ThordaqSetDLIMFrontEndSettings
	(
	UINT    boardNum
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetDLIMFrontEndSettings();
}