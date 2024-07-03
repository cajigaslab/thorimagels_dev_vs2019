// (dFLIM) dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "dFLIMapi.h"
//#include "thordaqcmd.h"
#include "dFLIM_4002.h"
#include "thordaqguid.h"
#include "windows.h"


#define VALIDATION(x, y)						\
	if (x >= MAXIMUM_NUMBER_OF_BOARDS)			\
	{	return STATUS_INVALID_BOARDNUM; }		\
	if(y[x] == NULL)							\
	{return STATUS_INITIATION_INCOMPLETE; }	

// Local Prototypes
void InitializeDll ();
void CleanupDll ();

// Internal variables [NOTE!  These functions to discover multiple boards supported by same kernel driver
//                     abandoned for TILS architecture reasons, in favor of 1 kernel driver per PCI VendorID -
//                     So for example ThorDAQ & dFLIM, when using same kernel driver, have different
//                     kernel driver .sys files which bind to VendorID 0x4001, 0x4002
CdFLIM_4002 *DriverList[MAXIMUM_NUMBER_OF_BOARDS];
HANDLE Prod4002_HANDLES[MAXIMUM_NUMBER_OF_BOARDS];
UINT TotalBoardsFound = 0;

// attempts to return KernelGUID handle (connection to Application code) for passed boardIndex
THORDAQ_STATUS GetPCIkernelDevHandles(LPGUID KernelGUID, UINT boardIndex, HANDLE * hDevHandle)
{
	HDEVINFO hDevInfo;
	SP_DEVICE_INTERFACE_DATA device_interface_data;
	DWORD					 device_interface_data_required_size = 0;
	PSP_DEVICE_INTERFACE_DETAIL_DATA	DeviceInterfaceDetailData;  // The structure to receice information about device specified interface
	DWORD					 boardCount = 0;
	BOOL					 bool_status = TRUE;
	THORDAQ_STATUS			 status = STATUS_DEVICE_NOT_EXISTS;
	INT32					 error = 0;


	// Get the device info
	hDevInfo = SetupDiGetClassDevs(
		KernelGUID,                        // Pointor to the GUID of Thordaq
		NULL,                              // Define no enumerator (global)
		NULL,                              // Define no
		DIGCF_DEVICEINTERFACE |            // Function class devices.
		DIGCF_PRESENT);                    // Only Devices present
	if (hDevInfo != INVALID_HANDLE_VALUE)
	{
		// query kernel's board info
		device_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		// 
		while (SetupDiEnumDeviceInterfaces(hDevInfo, NULL, KernelGUID, boardCount, &device_interface_data))
		{
			++boardCount; // number of PCI boards
		}
		if (boardCount > 0 && (boardIndex < boardCount))
		{
			// get the kernel's device handle for the boardIndex passed to us
			// Get information for device
			bool_status = SetupDiEnumDeviceInterfaces(hDevInfo,
				NULL, // No care about specific PDOs
				KernelGUID,
				boardIndex, //
				&device_interface_data);

			if (bool_status == false)
			{
				return STATUS_INVALID_BOARDNUM;
			}

			// Get the Device Interface Detailed Data
			// This is done in multiple parts:
			// 1.  query for the required size of the data structure (fix part + variable part)
			// 2.  malloc the returned size
			// 3.  Set the cbSize of the data structure to be the fixed size (required by interface)
			// 4.  query for the actual data
			bool_status = SetupDiGetDeviceInterfaceDetail(
				hDevInfo,
				&device_interface_data,
				NULL, // probing so no output buffer yet
				0, // probing so output buffer length of zero
				&device_interface_data_required_size,
				NULL);

			// this should fail (returning false) and setting error to ERROR_INSUFFICIENT_BUFFER
			if ((bool_status == TRUE) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
			{
				return THORDAQ_STATUS::STATUS_INITIATION_INCOMPLETE;
			}

			// allocate the correct size
			DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(device_interface_data_required_size);

			if (DeviceInterfaceDetailData == NULL)
			{
				return THORDAQ_STATUS::STATUS_INITIATION_INCOMPLETE;
			}

			// set the size to the fixed data size (not the full size)
			DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			// get the data. Do not need DeviceInfoData at this time
			bool_status = SetupDiGetDeviceInterfaceDetail(hDevInfo, &device_interface_data, DeviceInterfaceDetailData, device_interface_data_required_size, NULL, NULL);

			if (bool_status == TRUE)
			{
				// Now connect to the card
				*hDevHandle = CreateFile(DeviceInterfaceDetailData->DevicePath,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
					INVALID_HANDLE_VALUE);

				if (*hDevHandle != INVALID_HANDLE_VALUE)
					status = STATUS_SUCCESSFUL;
			}
			free(DeviceInterfaceDetailData);
		}
	}
	return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS GetKernelBoardCfg ( BOARD_INFO_STRUCT* board_info )
 * @brief	Get kernel driver data for this board
 *
 * @author	DZimmerman
 * @date	21-Aug-23

 **************************************************************************************************/
THORDAQ_STATUS GetKernelBoardCfg(HANDLE hDevice, LPVOID Board_CONFIG_data)
{
	THORDAQ_STATUS			status = STATUS_SUCCESSFUL;
	DWORD					bytes_returned = 0;
	DWORD					last_error_status = 0;
	OVERLAPPED overlapped;  // OVERLAPPED structure for the operation

	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Send GET_BOARD_CONFIG_IOCTL
	if (!DeviceIoControl(hDevice, GET_BOARD_CONFIG_IOCTL, NULL, 0, Board_CONFIG_data, sizeof(BOARD_CONFIG_STRUCT), &bytes_returned, &overlapped))
	{
		last_error_status = GetLastError();

		if (last_error_status != ERROR_IO_PENDING)
		{
#if _DEBUG
			printf("GetBoardCfg IOCTL call failed. Error = %d\n", GetLastError());
#endif // _DEBUG            
			status = STATUS_GET_BOARD_CONFIG_ERROR;
		}
		else
		{
			// Wait here (forever) for the Overlapped I/O to complete
			if (!GetOverlappedResult(hDevice, &overlapped, &bytes_returned, TRUE))
			{
				last_error_status = GetLastError();
#if _DEBUG
				printf("GetBoardCfg IOCTL call failed. Error = %d\n", last_error_status);
#endif // _DEBUG
				status = STATUS_GET_BOARD_CONFIG_ERROR;
			}
		}
	}

	// check returned structure size
	if ((bytes_returned != sizeof(BOARD_CONFIG_STRUCT)) && (status == STATUS_SUCCESSFUL))
	{
		// ioctl failed
#if _DEBUG
		printf("GetBoardCfg IOCTL returned invalid size (%d)\n", bytes_returned);
#endif // _DEBUG
		status = STATUS_GET_BOARD_CONFIG_ERROR;
	}

	if (overlapped.hEvent != 0)
		CloseHandle(overlapped.hEvent);
	return status;
}

// Private call to "Discover" the Boards at the time this DLL initially loads, which is
// when the Application calls "ThorDAQAPIBindBoard(0)" for the first board
void DiscoverAllThorDAQs()
{
	THORDAQ_STATUS status;
	HANDLE hDevHandle;
	UINT Prod4001boardsFound = 0, Prod4002boardsFound = 0;
	UINT GlobalTDBoardIndex = 0;
	BOARD_CONFIG_STRUCT BoardCFGdata;
	// we need 0-starting indexes for EACH kernel driver that can enumerate devices
	UINT Gen1KernelBoardIndex = 0;
	UINT Gen2KernelBoardIndex = 0;

	// initialize the global HANDLES - these will be passed to appropriate ThorDAQ object constructors
	for (int i = 0; i < MAXIMUM_NUMBER_OF_BOARDS; i++)
	{

		Prod4002_HANDLES[i] = NULL;
	}

	// We need to determine the TYPE and Counts of ThorDAQ board(s).
	// Get handles for ALL devices recognized by ThorDAQ2586_Gen1 kernel driver
	do {
		status = GetPCIkernelDevHandles((LPGUID)&GUID_DEVINTERFACE_dFLIM0x4002, Gen1KernelBoardIndex++, &hDevHandle); // MUST start with Index=0
		if (status == STATUS_SUCCESSFUL)
		{
			// NOW we need to determine which PCI Product ID the kernel's TDBoardIndex is connected to
			// make kernel IOCTL call to get the BoardInfo
			status = GetKernelBoardCfg(hDevHandle, (LPVOID)&BoardCFGdata);

			if (status == THORDAQ_STATUS::STATUS_SUCCESSFUL)
			{
				// process all possible PCI ProductIDs we support in this kernel driver (according to kernel GUID)
				switch (BoardCFGdata.PciConfig.DeviceId)
				{
				case 0x4002:
					Prod4002_HANDLES[GlobalTDBoardIndex] = hDevHandle; // add legacy dFLIM to global array
					if (DriverList[GlobalTDBoardIndex] == NULL)
					{
						DriverList[GlobalTDBoardIndex] = new CdFLIM_4002(GlobalTDBoardIndex, hDevHandle); // pass index and kernel handle
					}
					if (DriverList[GlobalTDBoardIndex] == NULL) //Cannot initiate the board
					{
						status = THORDAQ_STATUS::STATUS_GET_BOARD_CONFIG_ERROR;;
					}
					else // SUCCESS!
					{
						TotalBoardsFound++; // global count of all Boards
						++GlobalTDBoardIndex;     // next 0-based index (in case of another board)

					}

					break;
				default:
					status = THORDAQ_STATUS::STATUS_GET_BOARD_CONFIG_ERROR;
					break;
				}
			}
		}
		// next Kernel enumerated device
	} while (status == STATUS_SUCCESSFUL && (GlobalTDBoardIndex < MAXIMUM_NUMBER_OF_BOARDS));

}




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
		DiscoverAllThorDAQs();
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
///////  NWL SDK restoration //////////////////////////////////////////
//--------------------------------------------------------------------
// Common Packet Mode Function calls
//--------------------------------------------------------------------

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDoMem(
	UINT boardNum,                  // Board to target
	UINT32          Rd_Wr_n,        // 1==Read, 0==Write
	UINT32          BarNum,         // Base Address Register (BAR) to access
	PUINT8          Buffer,         // Data buffer
	UINT64          Offset,         // Offset in data buffer to start transfer
	UINT64          CardOffset,     // Offset in BAR to start transfer
	UINT64          Length,         // Byte length of transfer
	PSTAT_STRUCT    Status          // Completion Status
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DoMem(
		Rd_Wr_n,
		BarNum,
		Buffer,
		Offset,
		CardOffset,
		Length,
		Status);
}

// App software calls BindBoard with incrementing 0-based index,
// according to # of boards it expects
// The boards are discovered and kernel handles/Thordaq objects created at the time this DLL loads
THORDAQ_API THORDAQ_STATUS ThorDAQAPIBindBoard
(
	UINT				boardIndex                  // 0 based index from Application
)
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	if ((boardIndex + 1) > TotalBoardsFound)
		status = THORDAQ_STATUS::STATUS_INVALID_BOARDNUM;

	return status;
}
// DisconnectFromBoard
//
// Disconnect from a board.  'board' is used to select between
//   multiple board instances (more than 1 board may be present);
// If the board class has not been created, return;  
//   otherwise, call DisconnectFromBoard.  Does not delete the board class.

THORDAQ_API THORDAQ_STATUS ThorDAQAPIReleaseBoard
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
THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetBoardCfg
	(
	UINT					boardNum, // Board to target
	BOARD_INFO_STRUCT*	    board_info  // Returned structure
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetBoardCfg(board_info);
}

/////////  DZ SDK  ///////////////////////////////////////////////
THORDAQ_API THORDAQ_STATUS ThorDAQAPIFPGAregisterRead(
	UINT32 board,					// Board index to target
	const char* pName,				// name of register/field (if found)
	int nameSize,					// char array size
	UINT64* Value					// "shadow" DLL register copy value (REG or FIELD)
)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->FPGAregisterRead(pName, nameSize, Value);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIFPGAregisterWrite(
	UINT32 board,					// Board index to target
	const char* pName,				// name of register/field (if found)
	int nameSize,					// char array size
	UINT64* Value					// "shadow" DLL register copy value (REG or FIELD)
)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->FPGAregisterWrite(pName, nameSize, *Value);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIFPGAregisterQuery(
	UINT32 board,                  // Board index to target
	int RegIndex,           // location in physical DDR3 mem to start (ADC images in low memory)      
	int FldIndex,           // length in bytes of waveformBuffer (all waveforms must be equal), or '0' if unknown
	char* pName,			// name of register/field (if found)
	int pNameSize,
	PREG_FIELD_DESC RegFldDesc // register/field description
)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->FPGAregisterQuery(RegIndex, FldIndex, pName, pNameSize, RegFldDesc);
}


THORDAQ_API THORDAQ_STATUS ThorDAQAPIReadDDR3(
	UINT32  board,                  // Board to target
	UINT64 DDR3SourceAddress,
	PUINT8 HostDestinationAddress,
	PUINT32 ByteLength)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->ReadDDR3(DDR3SourceAddress, HostDestinationAddress, ByteLength);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIWriteDDR3(
	UINT32  board,                  // Board to target
	PUINT8 HostSourceAddress,
	UINT64 DDR3DestinationAddress,
	UINT32 ByteLength)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->WriteDDR3(HostSourceAddress, DDR3DestinationAddress, ByteLength);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPItdUserIntTask(
	USER_IRQ_WAIT_STRUCT* usrIrqWaitStruct)
{
	UINT32 board = usrIrqWaitStruct->boardNum;      // Board number to target
	VALIDATION(board, DriverList);
	return THORDAQ_STATUS::STATUS_SUCCESSFUL;//(THORDAQ_STATUS)DriverList[board]->APItdUserIntTask(usrIrqWaitStruct);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPItdCancelUserIntTask(UINT32 board)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->APItdCancelUserIntTask(); // void type
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIWaitForUserIRQ(
	UINT32  board,                  // Board number to target
	UINT64* CountOfChan0_INTs,
	UINT64* CountOfChan1_INTs,
	UINT64* CountOfChan2_INTs,
	UINT64* CountOfChan3_INTs
)
{

	VALIDATION(board, DriverList);
	return DriverList[board]->APIWaitForUserIRQ(CountOfChan0_INTs, CountOfChan1_INTs, CountOfChan2_INTs, CountOfChan3_INTs);
}
THORDAQ_API THORDAQ_STATUS ThorDAQAPICancelWaitForUserIRQ(
	UINT32  board                  // Board number to target
)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->APICancelWaitForUserIRQ();
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIProgressiveScan(
	UINT32  board,                  // Board number to target
	BOOL    bProgressiveScan)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->APIProgressiveScan(bProgressiveScan);
}


THORDAQ_API THORDAQ_STATUS ThorDAQAPIReadFrames
(
	UINT    boardNum,          // Board to target
	PUINT32 buffer_length,
	int chan,
	void* buffers[4],
	double timeout_ms,
	ULONG transferFrames, //number if frames to transfer
	BOOL isLastTransfer, //flag to let the read function know that this is the last time we will grabing the data.
	BOOL& isPartialData //a flag used by the read function to let the caller know if the image is complete or partial
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->APIReadFrames(buffer_length, chan, buffers, timeout_ms, transferFrames, isLastTransfer, isPartialData);
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

THORDAQ_API THORDAQ_STATUS ThorDAQAPIAbortRead
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

THORDAQ_API THORDAQ_STATUS ThorDAQAPIPacketWriteBuffer
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


THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMSetImagingConfiguration
	(
	UINT							boardNum, // Board to target
	dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config  // Returned structure
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetImagingConfiguration(imaging_config);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIStartAcquisition
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


THORDAQ_API THORDAQ_STATUS ThorDAQAPIStopAcquisition
	(
	UINT    boardNum          // Board to target
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->StopAcquisition();
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIWriteReadRegister
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


THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDACParkValue // this version changes the AUX MUX so "chan" matches 0-based ABBx BNC label (e.g. ABB1's AO1 is outputChannel 0)
	(
	UINT    boardNum,          // Board to target
	ULONG32 outputChannel,
	double outputValue
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->APIsetDACvoltage(outputChannel, outputValue);
	//	return DriverList[boardNum]->SetDACParkValue(outputChannel,outputValue);
}
THORDAQ_API THORDAQ_STATUS ThorDAQSetDACParkValue // Legacy dFLIM DLL interface
(
	UINT    boardNum,          // Board to target
	ULONG32 outputChannel,
	double outputValue
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetDACParkValue(outputChannel, outputValue);
	//	return DriverList[boardNum]->SetDACParkValue(outputChannel,outputValue);
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

THORDAQ_API	THORDAQ_STATUS ThorDAQAPIGetTotalFrameCount
(
UINT    boardNum,  
UINT32& frame_count
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetTotalFrameCount(frame_count);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetExternClockStatus
	(
	UINT    boardNum, 
	ULONG32& isClockedSynced
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetExternClockStatus(isClockedSynced);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMReSync
	(
	UINT    boardNum
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->APIdFLIMReSync();
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMSetFineShift
(
	UINT    boardNum,
	LONG32 shift,
	int	channel
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetFineShift(shift, channel);
}
THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMSetCoarseShift
	(
	UINT    boardNum, 
	LONG32 shift,
	int	channel
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetCoarseShift(shift, channel);
}


THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMGetClockFrequency
	(
	UINT    boardNum, 
	int		clockIndex,
	double	&ferquency
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->APIdFLIMGetClockFrequency(clockIndex, ferquency);
}


THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMSetSyncingSettings
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

THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMSetFrontEndSettings
	(
	UINT    boardNum
	)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetdFLIMFrontEndSettings();
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIMemTest  // to be used in dFLIM (4002) and ThorDAQ (4001) 
(
	UINT32  boardNum,                  // Board to target
	char* RetString
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->APIMemTest(RetString);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetEEPROMlabel(
	UINT    boardNum,          // Board to target
	INT32 eepromIndex, // 0-based; 0 is TD board itself; const struct defines end of indexes
	char eepromID[4])  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->APIGetEEPROMlabel(*DriverList[boardNum], eepromIndex, eepromID);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIReadEEPROM(
	UINT    boardNum,          // Board to target
	UINT32 fileOp,             // if 0, no file operation
	char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
	char eepromDATA[65]) // hardware definition - Thorlabs never uses more than 64 EEPROM bytes in ANY device
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->APIReadEEPROM(*DriverList[boardNum], fileOp, eepromID, /*startByte, Len,*/ eepromDATA);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIWriteEEPROM(
	UINT    boardNum,          // Board to target
	UINT32 fileOp,             // if 0, no file operation
	char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
	char eepromDATA[65]) // hardware definition - Thorlabs never uses more than 64 EEPROM bytes in ANY device
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->APIWriteEEPROM(*DriverList[boardNum], fileOp, eepromID, /*startByte, Len,*/ eepromDATA);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIReadProdSN(
	UINT    boardNum,          // Board to target
	UINT32 fileOp,                 // if 0, no file operation, 1 record update to file
	char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
	char ProdID[10],   // TIS Production system fields
	char ProdSN[6])
{
	VALIDATION(boardNum, DriverList);

	return DriverList[boardNum]->APIReadProdSN(
		*DriverList[boardNum],
		fileOp,
		eepromID,
		ProdID,
		ProdSN
	);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIXI2CReadWrite
(
	UINT boardNum,
	bool bI2C_Read,            // when true, READ bytes from I2C slave
	UINT32 MasterMUXAddr,
	UINT32 MasterMUXChan,
	UINT32 SlaveMUXAddr,
	UINT32 SlaveMUXChan,
	UINT32 DevAddress,
	UINT32 I2CbusHz,    // frequency of bus, i.e. 100 or 400 Hz (or ...)
	INT32 PageSize,     // I2C slave specific, especially for page WRITEs (higher performance)
	PUINT8 OpCodeBuffer,
	UINT32* OpCodeLen,
	PUINT8 BiDirectionalDataBuffer,  // bi-directional data buffer -- DATA bytes to write to slave, or read from slave
	UINT32* BiDirectionalDataLen // IN: expect returned bytes Rx'd from slave  OUT: actual bytes (can be 0), Opcodes+Data
)
{
	VALIDATION(boardNum, DriverList);

	return DriverList[boardNum]->APIXI2CReadWrite(
		*DriverList[boardNum],
		bI2C_Read,            // when true, READ bytes from I2C slave
		MasterMUXAddr,
		MasterMUXChan,
		SlaveMUXAddr,
		SlaveMUXChan,
		DevAddress,
		I2CbusHz,    // frequency of bus, i.e. 100 or 400 Hz (or ...)
		PageSize,     // I2C slave specific, especially for page WRITEs (higher performance)
		OpCodeBuffer,
		OpCodeLen,
		BiDirectionalDataBuffer,  // bi-directional data buffer -- DATA bytes to write to slave, or read from slave
		BiDirectionalDataLen // IN: expect returned bytes Rx'd from slave  OUT: actual bytes (can be 0), Opcodes+Data
	);
}
