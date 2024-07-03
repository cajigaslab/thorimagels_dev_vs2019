// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#pragma once
#include "thorDAQ.h"
#include "thordaqcmd.h"
#include "thordaqAPI.h"
#include "thordaqguid.h"

#include "windows.h"

#define VALIDATION(x, y)						\
	if (x >= MAXIMUM_NUMBER_OF_BOARDS)			\
	{	return STATUS_INVALID_BOARDNUM; }		\
	if(y[x] == NULL)							\
	{return STATUS_INITIATION_INCOMPLETE; }	

// Local Prototypes
void InitializeDll();
void CleanupDll();

// LED blink task (can have 4 instances)
void DLLBlinkLEDTask(CThordaq& TdBrd)
{
	THORDAQ_STATUS status;
	bool bBlinkThisCycle = true;
	int i;
	OutputDebugString(L"dllmain LEDBlink thread start...");

	// traverse BOBLEDcmdArray array to read values set by API
	// if it "cmd" differs from "state"...
	// IF cmd is 0 or 1 value, send I2C, according, then set "state" array
	// IF cmd is 2, for blink state, toggle the value

	do {
		for (i = 0; i <= BBoxLEDenum::AI13; i++)
		{
			// for each LED
			if (TdBrd.BOBLEDstateArray[i] != TdBrd.BOBLEDcmdArray[i]) // API has changed
			{
				switch (TdBrd.BOBLEDcmdArray[i])
				{
				case 0: // static state change
				case 1:
					status = TdBrd.LEDControlIS31FL(TdBrd, i, TdBrd.BOBLEDcmdArray[i]); // control hardware over I2C
					if (status == STATUS_SUCCESSFUL) // in highly unlikely event of I2C error, next Thread cycle this repeats
					{
						TdBrd.BOBLEDstateArray[i] = TdBrd.BOBLEDcmdArray[i]; // state change accomplished - don't repeat
					}
					break;
				case 2: // blink state change
					TdBrd.BOBLEDstateArray[i] = TdBrd.BOBLEDcmdArray[i];
					break;
				default: // UNKOWN STATE
					break;
				}
			} // "state" change handled... now check if it needs blinking...
			if (TdBrd.BOBLEDstateArray[i] == 2) // do we need to blink?
			{
				TdBrd.LEDControlIS31FL(TdBrd, i, bBlinkThisCycle);
			}
		}
		Sleep(500);
		bBlinkThisCycle = (bBlinkThisCycle == true) ? 0 : 1; // toggle
	} while (!TdBrd.bExitBlinkLEDTask);
}


// Internal variables
CThordaq* DriverList[MAXIMUM_NUMBER_OF_BOARDS];
HANDLE Prod4001_HANDLES[MAXIMUM_NUMBER_OF_BOARDS];
std::thread* pLEDBlinkThread[MAXIMUM_NUMBER_OF_BOARDS] = { NULL, NULL, NULL, NULL };  // controls on/off/blink of LED based on state table 
UINT TotalBoardsFound = 0;

// attempts to return KernelGUID handle (connection to Application code) for passed boardIndex
THORDAQ_STATUS GetPCIkernelDevHandles(LPGUID KernelGUID, UINT boardIndex, HANDLE* hDevHandle)
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

		Prod4001_HANDLES[i] = NULL;
	}

	// We need to determine the TYPE and Counts of ThorDAQ board(s).
	// Get handles for ALL devices recognized by ThorDAQ2586_Gen1 kernel driver
	do {
		status = GetPCIkernelDevHandles((LPGUID)&GUID_DEVINTERFACE_ThorDAQ0x4001, Gen1KernelBoardIndex++, &hDevHandle); // MUST start with Index=0
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
				case 0x4001:
					Prod4001_HANDLES[GlobalTDBoardIndex] = hDevHandle; // add legacy dFLIM to global array
					if (DriverList[GlobalTDBoardIndex] == NULL)
					{
						DriverList[GlobalTDBoardIndex] = new CThordaq(GlobalTDBoardIndex, hDevHandle); // pass index and kernel handle
					}
					if (DriverList[GlobalTDBoardIndex] == NULL) //Cannot initiate the board
					{
						status = THORDAQ_STATUS::STATUS_GET_BOARD_CONFIG_ERROR;;
					}
					else // SUCCESS!
					{
						// Assume ThorDAQ connected to 3U BOB and will run blinkLED task
						if (pLEDBlinkThread[GlobalTDBoardIndex] == NULL)
						{
							pLEDBlinkThread[GlobalTDBoardIndex] = new std::thread{ DLLBlinkLEDTask, std::ref(*DriverList[GlobalTDBoardIndex]) };
						}
						TotalBoardsFound++; // global count of all "ThorDAQ" type Boards
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



BOOL APIENTRY DllMain(HMODULE hModule,
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
void InitializeDll()
{
	for (int i = 0; i < MAXIMUM_NUMBER_OF_BOARDS; i++)
	{
		DriverList[i] = NULL;
	}
}

// CleanupDll
//
// This is called when the DLL is cleaned up.
// This detaches any ongoing connections and cleans up any global variables.
void CleanupDll()
{
	OutputDebugString(L"dllmain: CleanupDll()");
	for (int i = 0; i < MAXIMUM_NUMBER_OF_BOARDS; i++)
	{
		if (DriverList[i] != NULL)
		{
			DriverList[i]->bExitBlinkLEDTask = true; // tell thread to exit
			if (pLEDBlinkThread[i] != NULL)
				pLEDBlinkThread[i]->join();  // waits for thread exit - should take 500 ms MAX

			delete pLEDBlinkThread[i];  // free thread resource
			pLEDBlinkThread[i] = NULL;
			// Make sure we are disconnected
			DriverList[i]->DisconnectFromBoard();
			// Cleanup the class
			delete DriverList[i]; // calls CThordaq deconstructor
			DriverList[i] = NULL;
		}
	}
}

//--------------------------------------------------------------------
//
//  Public DLL Calls - Main interface to the DLL
//
//--------------------------------------------------------------------

// ThorDAQAPIBindBoard
//
// Connects to a board.  'board' is used to select between
//   multiple board instances (more than 1 board may be present);
//   If the board class has not been created, it does it now.



THORDAQ_API THORDAQ_STATUS ThorDAQAPIBindBoard
(
	UINT				boardIndex                  // Board to target
)
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	if ((boardIndex + 1) > TotalBoardsFound)
		status = THORDAQ_STATUS::STATUS_INVALID_BOARDNUM;

	return status;
}

/*	if (boardNum >= MAXIMUM_NUMBER_OF_BOARDS)
	{
		return STATUS_INVALID_BOARDNUM;
	}
	if (DriverList[boardNum] == NULL)
	{
//		DriverList[boardNum] = new CThordaq(boardNum);

		//  start the LED blink thread
//		std::thread LEDblinkThread(DLLBlinkLEDTask, std::ref(*DriverList[boardNum]));
//		pLEDBlinkThread[boardNum] = new std::thread { DLLBlinkLEDTask, std::ref(*DriverList[boardNum]) };
	}
	if (DriverList[boardNum] == NULL) //Cannot initiate the board
	{
		return STATUS_INITIATION_INCOMPLETE;
	}
//	status = DriverList[boardNum]->GetDMAcapability();  BIND BOARD replacement
	if (status == STATUS_SUCCESSFUL)
	{
		// Main board attached - start LED blink thread
		if (pLEDBlinkThread[boardNum] == NULL)
		{
			pLEDBlinkThread[boardNum] = new std::thread{ DLLBlinkLEDTask, std::ref(*DriverList[boardNum]) };
		}
	}

	return status; // add "*DriverList[boardNum]" arg
}
*/

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
	BOARD_INFO_STRUCT* board_info  // Returned structure
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetBoardCfg(board_info);
}

//gets the low frequency board config
THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetLowFreqTriggerBoardCfg
(
	UINT					boardNum, // Board to target
	LOW_FREQ_TRIG_BOARD_INFO_STRUCT* board_info  // Returned structure
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetLowFreqBoardCfg(board_info);
}

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

//         public extern static uint ThorDAQAPIXI2CRead(UInt32 board, UInt32 DevAddr, IntPtr OpCodeBuffer, UInt32 OpCodeLen, IntPtr RxBuffer, UInt32 RxLen); // OpCodeBuffer can be NULL with OpCodeLen=0
THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetAUXGPIOvalue(
	UINT boardNum,                  // Board to target
	UINT32 FPGAindx,
	UINT32* RetValue)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APIGetAUXGPIOvalue(
		*DriverList[boardNum],
		FPGAindx,
		RetValue);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIXI2CReadWrite(
	UINT boardNum,                  // Board to target
	bool bRead,                     // final I/O direction; "READ" may require initial "WRITE"
	UINT32 MasterMUXAddr,			// fixed in hardware
	UINT32 MasterMUXChan,			// selects either slave end device OR slave MUX
	UINT32 SlaveMUXAddr,			// fixed in hardware
	UINT32 SlaveMUXChan,			// selects channel, or 0xFF if DevAddress NOT on Slave MUX
	UINT32 DevAddress,				// final I2C slave device address, behind 1 or 2 MUXes
	UINT32 I2CbusHz,				// e.g. 100 Hz or 400 Hz
	INT32 PageSize,					// e.g. 16 bytes needed for EEPROM write
	PUINT8 OpCodeBuffer,			// i.e., typically WRITE command that sends up READ data, or data for WRITE-only command
	UINT32* OpCodeLen,
	PUINT8 DataBuffer,				// typically the READ data buffer
	UINT32* DataLen
) // IN: expect returned bytes Rx'd from slave  OUT: actual bytes (can be 0), Opcodes+Data
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APIXI2CReadWrite(*DriverList[boardNum],
		bRead,            // when true, READ bytes from slave
		MasterMUXAddr,
		MasterMUXChan,
		SlaveMUXAddr,
		SlaveMUXChan,
		DevAddress,
		I2CbusHz,
		PageSize,     // I2C slave specific, especially for page WRITEs (higher performance)
		OpCodeBuffer,
		OpCodeLen,
		DataBuffer,
		DataLen
	); // IN: expect returned bytes Rx'd from slave  OUT: actual bytes (can be 0), Opcodes+Data
}
// communicate with device on ThorDAQ I2C network
THORDAQ_API THORDAQ_STATUS ThorDAQAPIReadI2CDevice(
	UINT boardNum,                  // Board to target
	UINT32 MasterChan,
	INT32 SlaveChan,
	UINT32 SlaveDevAddress,
	UINT32 SlaveCommandByte,
	PUINT8 ReadBuffer,
	UINT32* ByteLen  // IN: ReadBuffer Len, OUT: byte length read
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APIReadI2CDevice(*DriverList[boardNum],
		MasterChan, SlaveChan, SlaveDevAddress, SlaveCommandByte,
		ReadBuffer, ByteLen);

}
THORDAQ_API THORDAQ_STATUS ThorDAQAPIWriteI2CDevice(
	UINT boardNum,                  // Board to target
	UINT32 MasterChan,
	INT32 SlaveChan,
	UINT32 SlaveDevAddress,
	UINT32 SlaveCommandByte,
	PUINT8 WriteBuffer,
	UINT32* ByteLen  // IN: ReadBuffer Len, OUT: byte length read
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APIWriteI2CDevice(*DriverList[boardNum],
		MasterChan, SlaveChan, SlaveDevAddress, SlaveCommandByte,
		WriteBuffer, ByteLen);

}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetCPLD_DIOConfig(
	UINT boardNum,                // Board to target
	int BNClabel,
	int CopiedSourcelabel,
	int FPGA_HSC,
	int Input     // 1 if Input, 0 if Output (i.e. FGPA AUX GPIO define)
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APISetCPLD_DIOConfig(*DriverList[boardNum], BNClabel, CopiedSourcelabel, FPGA_HSC, Input);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetDDR3status(
	UINT boardNum,            // Board to target
	CHAR* StatusString,
	UINT32 StringChars
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APIGetDDR3status(*DriverList[boardNum], StatusString, StringChars);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetBOBstatus( // returns current Breakout Box status
	UINT boardNum,            // Board to target
	CHAR* StatusString,
	UINT32 StringChars
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APIGetBOBstatus(*DriverList[boardNum], StatusString, StringChars);
}



THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetAI( // Breakout Box "Slow" Analog Inputs
	UINT boardNum,                  // Board to target
	UINT32 BNCindex,					// 0-based BNC connector index
	BOOL bVolts,					// if FALSE return raw ADC counts
	double* Value					// raw 12-bit count received from MAX127 chip
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APIGetAI(*DriverList[boardNum], BNCindex, bVolts, Value);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetAIOConfig( // Breakout Box "Slow" Analog Inputs
	UINT boardNum,                  // Board to target
	CHAR* config,					// e.g. "DnnXmm", where "n" is 0-31 (BNC label D0 through "D31", "X" is Input/Output direction "I" or "O", "mm" is Index of Output source
	UINT32 configSize
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APISetAIOConfig(*DriverList[boardNum], config, configSize);

};
THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetAIOConfig( // Breakout Box "Slow" Analog Inputs
	UINT boardNum,                  // Board to target
	CHAR* config,              // e.g. "DnnXmm", where "n" is 0-31 (BNC label D0 through "D31", "X" is Input/Output direction "I" or "O", "mm" is Index of Output source
	UINT32 configSize
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APIGetDIOConfig(*DriverList[boardNum], config, configSize);

};

THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetDIOConfig( // Breakout Box "Slow" Analog Inputs
	UINT boardNum,                  // Board to target
	CHAR* config,              // e.g. "DnnXmm", where "n" is 0-31 (BNC label D0 through "D31", "X" is Input/Output direction "I" or "O", "mm" is Index of Output source
	UINT32 configSize
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APIGetDIOConfig(*DriverList[boardNum], config, configSize);

};
THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDIOConfig( // Breakout Box "Slow" Analog Inputs
	UINT boardNum,                  // Board to target
	CHAR* config,              // e.g. "DnnXmm", where "n" is 0-31 (BNC label D0 through "D31", "X" is Input/Output direction "I" or "O", "mm" is Index of Output source
	UINT32 configSize
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APISetDIOConfig(*DriverList[boardNum], config, configSize);

};


THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDO( // SET the value of one or more DOs
	UINT boardNum,           // Board to target
	CHAR* config,            // Collection of fields per Config functions - AUX field holds value
	UINT32 configSize,       // field size 
	UINT32 NumDOs            // 1 or more DO records in "config"
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APISetDO(*DriverList[boardNum], config, configSize, NumDOs);

};
THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetDIO( // SET the value of one or more DOs
	UINT boardNum,           // Board to target
	CHAR* config,            // Collection of fields per Config functions - AUX field holds value
	UINT32 configSize,       // field size 
	UINT32 NumDOs            // 1 or more DO records in "config"
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	return DriverList[boardNum]->APIGetDIO(*DriverList[boardNum], config, configSize, NumDOs);
};

// Advanced ProgrammableTrigger
THORDAQ_API THORDAQ_STATUS ThorDAQAPIProgrammableTrigger(
	UINT boardNum,
	signed char chan,
	signed char ArmAssert)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	bool ArmASSERT = (ArmAssert == 0) ? false : true;
	return DriverList[boardNum]->APIProgrammableTrigger(*DriverList[boardNum], chan, ArmASSERT);
}

// Control the ThorDAQ LEDs on the Breakout Boxes
THORDAQ_API THORDAQ_STATUS ThorDAQAPIBreakOutBoxLED(
	UINT boardNum,                  // Board to target
	INT32  LEDenum,                 // -1 for all LEDs, otherwise ENUM according to label (e.g. DIO5)
	UCHAR  State                    // 0 for off, 1 for on, 2 for blink
)
{
	VALIDATION(boardNum, DriverList);     // pass this Thordaq object by ref
	//	if (pLEDBlinkThread[boardNum] == NULL)
	//	{
	//		pLEDBlinkThread[boardNum] = new std::thread{ DLLBlinkLEDTask, std::ref(*DriverList[boardNum]) };

	//	}
	return DriverList[boardNum]->APIBreakOutBoxLED(*DriverList[boardNum], LEDenum, State);
}


/*! SetupPacketMode
 *
 * \brief Does the necessary setup for Packet mode. This includes allocating a
 *  receive packet buffer in the driver and initializing the descriptors and
 *  associated structures
 * \param board
 * \param EngineOffset
 * \param Buffer
 * \param BufferSize
 * \param MaxPacketSize
 * \param PacketMode
 * \param NumberDescriptors
 * \return Status
 */
THORDAQ_API THORDAQ_STATUS ThorDAQAPISetupPacketMode
(
	UINT32  board,                  // Board number to target
	INT32   EngineOffset,           // DMA Engine number offset to use
	PUINT8  Buffer,                 // Data buffer
	PUINT32 BufferSize,             // Total size of Recieve Buffer
	PUINT32 MaxPacketSize,          // Largest single packet size
	INT32   PacketMode,             // Sets mode, FIFO or Addressable Packet mode
	INT32   NumberDescriptors       // Number of DMA Descriptors to allocate (Addressable mode)
)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->SetupPacket(
		EngineOffset,
		Buffer,
		BufferSize,
		MaxPacketSize,
		PacketMode,
		NumberDescriptors);
}

/*! ShutdownPacketMode
 *
 * \brief Does the necessary shutdown of Packet mode. This includes freeing
 *  receive packet buffer in the driver
 *  Returns Processing status of the call.
 * \param board
 * \param EngineOffset
 * \return Status
 */
THORDAQ_API THORDAQ_STATUS ThorDAQAPIShutdownPacketMode(
	UINT32  board,                  // Board number to target
	INT32   EngineOffset            // DMA Engine number offset to use
)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->ReleasePacketBuffers(EngineOffset);
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

/*! PacketReadEx
 *
 * \brief Read a packet containing 'buffer' for 'length' bytes
 * \param board
 * \param EngineOffset
 * \param UserStatus
 * \param CardOffset
 * \param Mode
 * \param Buffer
 * \param Length
 * \return Status
 */
THORDAQ_API THORDAQ_STATUS ThorDAQAPIPacketReadEx(
	UINT32  board,                  // Board to target
	INT32   EngineOffset,           // DMA Engine number offset to use
	PUINT64 UserStatus,             // User Status returned from the EOP DMA Descriptor
	UINT64  CardOffset,             // Card Address to start read from
	UINT32  Mode,                   // Control Mode Flags
	PUINT8  Buffer,                 // Address of data buffer
	PUINT32 Length                  // Length to Read
)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->PacketReadEx(EngineOffset, UserStatus, CardOffset, Mode, Buffer, Length);
}

/*! PacketWriteEx
 *
 * \brief Writes a packet containing 'buffer' for 'length' bytes
 * \param board
 * \param EngineOffset
 * \param UserControl
 * \param CardOffset
 * \param Mode
 * \param Buffer
 * \param Length
 * \return Status
 */
THORDAQ_API THORDAQ_STATUS ThorDAQAPIPacketWriteEx(
	UINT32  board,                  // Board number to target
	INT32   EngineOffset,           // DMA Engine number offset to use
	UINT64  UserControl,
	UINT64  CardOffset,
	UINT32  Mode,                   // Control Mode Flags
	PUINT8  Buffer,
	UINT32  Length
)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->PacketWriteEx(EngineOffset, UserControl, CardOffset, Mode, Buffer, Length);
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



//////////  NWL SDK ///////////////////////////////////////////////

//////////  DZ SDK  ///////////////////////////////////////////////
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

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACwaveInit(
	UINT32 board,                  // Board number to target
	UINT64 DDR3startAddr,          // location in physical DDR3 mem to start (ADC images in low memory)      
	UINT32 CPLDfrequency           // speed of 16-bit sample read (from DDR3 mem)
)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->DACwaveInit(DDR3startAddr, CPLDfrequency);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACwaveLoad(
	UINT32 board,                  // Board number to target
	UINT32 DACchannel,             // DAC hardware channel 
	PUINT8 DACsampleBuffer,        // array of 16-bit waveformBuffer samples to load
	UINT32 BufferSize,             // Total size (bytes) of sample buffer
	PUINT64 DDR3startAddr)     // if successful, the DDR3 start addr where waveformBuffer is loaded
{
	VALIDATION(board, DriverList);
	return DriverList[board]->DACwaveLoad(DACchannel, DACsampleBuffer, BufferSize, DDR3startAddr);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIS2MMconfig(
	UINT32 board,                  // Board number to target
	PS2MM_CONFIG pS2MMconfig)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->S2MMconfig(pS2MMconfig);

}

// ADCsampleImagizerLUT
THORDAQ_API THORDAQ_STATUS ThorDAQAPIADCsampleImagizerLUT(
	UINT32 board,                  // Board number to target
	PS2MM_ADCSAMPLE_LUT pS2MMsampleLUT)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->API_ADCsampleImagizerLUT(pS2MMsampleLUT);
}

// Set DAC Voltage
// ADCsampleImagizerLUT
THORDAQ_API THORDAQ_STATUS ThorDAQAPIsetDACvoltage(
	UINT32 board,                  // Board number to target
	UINT32 channel,                // DAC channel 0-12 (12 is DAC waveformBuffer, not analog chan)
	double Voltage)
{
	VALIDATION(board, DriverList);
	return DriverList[board]->APIsetDACvoltage(channel, Voltage);
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


// Read ADC channels from the DDR3 memory ???
// 
THORDAQ_API THORDAQ_STATUS ThorDAQAPIReadFrames
(
	UINT    boardNum,          // Board to target
	UINT64* buffer_length,
	void* buffer,
	double timeout_ms,
	ULONG transferFrames, //number if frames to transfer
	BOOL isLastTransfer, //flag to let the read function know that this is the last time we will grabing the data.
	BOOL& isPartialData //a flag used by the read function to let the caller know if the image is complete or partial
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->APIReadFrames(buffer_length, buffer, timeout_ms, transferFrames, isLastTransfer, isPartialData);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIAbortRead
(
	UINT    boardNum           // Board to target
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->AbortPacketRead();
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIPacketWriteBuffer
(
	UINT    boardNum,          // Board to target
	ULONG64  register_card_offset,           // Start address to read in the card
	ULONG   Length,            // Size of the Packet Write to Data Buffer Count by Byte (FIFO Mode)
	UCHAR* Buffer,            // Data buffer (Packet Mode)
	ULONG    Timeout            // Generate Timeout error when timeout
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->PacketWriteBuffer(register_card_offset, Length, Buffer, Timeout);
}


THORDAQ_API THORDAQ_STATUS ThorDAQAPISetImagingConfiguration
(
	UINT							boardNum, // Board to target
	IMAGING_CONFIGURATION_STRUCT	imaging_config // config structure
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
	BYTE* buffer,           // Data buffer
	ULONGLONG offset,       // Offset in data buffer to start transfer
	ULONGLONG length,       // Byte length of transfer
	PSTAT_STRUCT status     // Completion Status
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->WriteReadRegister(read_write_flag, register_bar_num, register_card_offset, buffer, offset, length, status);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDACParkValue
(
	UINT    boardNum,          // Board to target
	ULONG32 outputChannel,
	double outputValue
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetDACParkValue(outputChannel, outputValue);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDACOffsetValue
(
	UINT    boardNum,          // Board to target
	ULONG32 outputChannel,
	double outputValue
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetDACOffsetValue(outputChannel, outputValue);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetLineTriggerFrequency
(
	UINT    boardNum,
	UINT32 sample_rate,
	double& frequency,
	ULONG32 expectedFrequency
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetLineTriggerFrequency(sample_rate, frequency, expectedFrequency);
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

THORDAQ_API	THORDAQ_STATUS ThorDAQAPISetClockSource
(
	UINT    boardNum,
	CLOCK_SOURCE clock_source
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetClockSourceAndFrequency((ULONG)clock_source);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIMeasureExternClockRate
(
	UINT    boardNum,
	ULONG32& clock_rate,
	ULONG32& clock_ref,
	ULONG32    mode
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->MeasureExternClockRate(clock_rate, clock_ref, mode);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetThreePhotonEnable
(
	UINT    boardNum,
	bool	threePhotonEnable
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetThreePhotonModeEnable(threePhotonEnable);
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

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetAllADCChannelsGain
(
	UINT    boardNum,
	ULONG clock_source,
	ULONG32 adcGain[],
	bool isThreePhotonMode
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetAllADCChannelsGain(clock_source, adcGain, isThreePhotonMode);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetGRClockRate
(
	UINT    boardNum,
	ULONG32 clock_rate,
	ULONG32 expectedFrequency
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetGRClockRate(clock_rate, expectedFrequency);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDCOffsetPreFIR
(
	UINT	boardNum,
	short	preDcOffset,
	USHORT	channel
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetDCOffsetPreFIR(preDcOffset, channel);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIToggleAuxDigitalOutputs
(
	UINT    boardNum,          // Board to target
	USHORT  auxChannelIndex,
	USHORT  value
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->ToggleAuxDigitalOutputs(auxChannelIndex, value);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetThreePhotonSampleOffset
(
	UINT    boardNum, // Board to target
	UINT	channel,
	UINT8	threePhotonSampleOffset  // Value to toggle the channel to: 1 = high and 0 = low
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetThreePhotonSampleOffset(channel, threePhotonSampleOffset);
}


THORDAQ_API THORDAQ_STATUS ThorDAQAPISetConfigurationWithFullwWaveform(
	UINT    boardNum, // Board to target
	DAC_FREERUN_WAVEFORM_CONFIG& waveformConfig
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACSetWaveformConfigurationForStaticLoading(waveformConfig);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACSetWaveformConfigurationForDynamicLoad(
	UINT    boardNum, // Board to target
	DAC_FREERUN_WAVEFORM_CONFIG& waveformConfig
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACSetWaveformConfigurationForDynamicLoading(waveformConfig);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIStartDACWaveforms(
	UINT    boardNum // Board to target
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACStartWaveforms();
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIStopDACWaveforms(
	UINT    boardNum // Board to target
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACStopWaveforms();
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACRegisterApproachingNSampleCallback(
	UINT    boardNum, // Board to target
	UINT8	dacChannel,
	UINT32	nSamples,
	UINT32	options,
	ThorDAQDACApproachingNSamplesCallbackPtr callbackFunction,
	void* callbackData)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACRegisterApproachingNSamplesEvent(dacChannel, nSamples, options, callbackFunction, callbackData);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACRegisterCycleDoneEvent(
	UINT    boardNum, // Board to target
	UINT8	dacChannel,
	UINT32	options,
	ThorDAQDACCycleDoneCallbackPtr callbackFunction,
	void* callbackData)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACRegisterCycleDoneEvent(dacChannel, options, callbackFunction, callbackData);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACRegisterWaveformPlaybackCompleteEvent(
	UINT    boardNum, // Board to target
	UINT32	options,
	ThorDAQDACWaveformPlaybackCompleteCallbackPtr callbackFunction,
	void* callbackData)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACRegisterWaveformPlaybackCompleteEvent(options, callbackFunction, callbackData);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACRegisterWaveformPlaybackStartedEvent(
	UINT    boardNum, // Board to target
	UINT32	options,
	ThorDAQDACWaveformPlaybackStartedCallbackPtr callbackFunction,
	void* callbackData
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACRegisterWaveformPlaybackStartedEvent(options, callbackFunction, callbackData);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACRegisterApproachingLoadedWaveformEndEvent(
	UINT    boardNum, // Board to target
	UINT8	dacChannel,
	UINT32	options,
	ThorDAQDACApproachingLoadedWaveformEndCallbackPtr callbackFunction,
	void* callbackData)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACRegisterApproachingLoadedWaveformEndEvent(dacChannel, options, callbackFunction, callbackData);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACDynamicLoadWaveform(
	UINT    boardNum, // Board to target
	std::map<UINT, DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> dacCtrlSettings,
	bool isLastPartOfWaveform
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACDynamicLoadWaveform(dacCtrlSettings, isLastPartOfWaveform);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACGetMinSamples(
	UINT    boardNum, // Board to target
	double dacUpdateRate,
	UINT64& minSamples
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACGetMinSamples(dacUpdateRate, minSamples);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACDynamicLoadPresetWaveform(
	UINT    boardNum, // Board to target
	bool isLastPartOfWaveform
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACDynamicLoadPresetWaveform(isLastPartOfWaveform);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACPresetNextWaveformSection(
	UINT    boardNum, // Board to target
	std::map<UINT,
	DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> dacCtrlSettings, bool isLastPartOfWaveform
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACPresetNextWaveformSection(dacCtrlSettings, isLastPartOfWaveform);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACBankSwitchingLoadNextWaveform(
	UINT    boardNum, // Board to target
	std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> dacCtrlSettings
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACBankSwitchingLoadNextWaveform(dacCtrlSettings);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACBankSwitchingRegisterReadyForNextImageWaveformsEvent(
	UINT    boardNum, // Board to target
	UINT32	options,
	ThorDAQDACBankSwitchingReadyForNextWaveformCallbackPtr callbackFunction,
	void* callbackData
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACBankSwitchingRegisterReadyForNextImageWaveformsEvent(options, callbackFunction, callbackData);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACBankSwitchingClearRegisteredReadyForNextImageWaveformsEventHandlers(
	UINT    boardNum // Board to target
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->DACBankSwitchingClearRegisteredReadyForNextImageWaveformsEventHandlers();
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetImagingWaveforms(
	UINT    boardNum, // Board to target
	std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> dacCtrl,
	std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> dacCtrl2 // use this one for 2bank acquisitions to set the second bank
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetImagingWaveforms(dacCtrl, dacCtrl2);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDIOChannelSelection(
	UINT    boardNum, // Board to target
	std::vector<string> DIOSelection
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetDIOChannelSelection(DIOSelection);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetScanActiveLineInvert(
	UINT    boardNum, // Board to target
	bool captureActiveLineInvert
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetScanActiveLineInvert(captureActiveLineInvert);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDefaultBOBMapping(
	UINT    boardNum, // Board to target
	bool	initialParkAllAtZero
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetDefaultBOBMapping(initialParkAllAtZero);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISlowSmoothMoveToAndFromParkEnable(
	UINT    boardNum, // Board to target
	bool	enable
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SlowSmoothMoveToAndFromParkEnable(enable);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDDSClockEnable(
	UINT    boardNum, // Board to target
	bool	enable
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetDDSClockEnable(enable);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDDSClockPhase(
	UINT    boardNum, // Board to target
	int		channel,
	double	phase
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->SetDDSClockPhase(channel, phase);
}

THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetBOBType(
	UINT    boardNum, // Board to target
	THORDAQ_BOB_TYPE& bobType
)
{
	VALIDATION(boardNum, DriverList);
	return DriverList[boardNum]->GetBOBType(bobType);
}