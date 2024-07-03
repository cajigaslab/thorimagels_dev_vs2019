// -------------------------------------------------------------------------
// 
// PRODUCT:			ThorDAQ Driver
// MODULE NAME:		thorDAQAPI.h
// 
// MODULE DESCRIPTION: 
// 
// Contains defines, structures and exported functions for the DLL-like interface.
// 
// $Revision:  $
//
// ------------------------- CONFIDENTIAL ----------------------------------
// 
//              Copyright (c) 2016-2019 by ThorLabs Imaging Research Group, LLC.   
//                       All rights reserved. 
// 
// -------------------------------------------------------------------------

//#include <string.h>
#ifndef __THORDAQAPI_H__
#define __THORDAQAPI_H__


#include "stdafx.h"

#ifndef ___THORDAQCMD_H___
#include "dFLIMcmd.h"
#endif

#ifndef __THORDAQRES__h__
#include "dFLIMres.h"
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
#ifdef THORDAQ_EXPORTS
#define THORDAQ_API __declspec(dllexport)
#else
#define THORDAQ_API __declspec(dllimport)
#endif
// ----------------------
// Externally Visible API

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDoMem(
		UINT boardNum,                  // Board to target
		UINT32          Rd_Wr_n,        // 1==Read, 0==Write
		UINT32          BarNum,         // Base Address Register (BAR) to access
		PUINT8          Buffer,         // Data buffer
		UINT64          Offset,         // Offset in data buffer to start transfer
		UINT64          CardOffset,     // Offset in BAR to start transfer
		UINT64          Length,         // Byte length of transfer
		PSTAT_STRUCT    Status          // Completion Status
	);


// ThorDAQAPIBindBoard
//
// Connects to a board.  'board' is used to select between
//   multiple board instances (more than 1 board may be present);
//   If the board class has not been created, it does it now.

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIBindBoard
	(
		UINT				boardNum
	);

	// ThorDAQAPIReleaseBoard
	//
	// Disconnect from a board.  'board' is used to select between
	//   multiple board instances (more than 1 board may be present);
	// If the board class has not been created, return;  
	//   otherwise, call DisconnectFromBoard.  Does not delete the board class.

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIReleaseBoard
	(
		UINT boardNum                  // Board to target
	);

	// DoMem (see NWL "DMA Driver Windows & Linux User Guide")
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDoMem(
		UINT boardNum,                  // Board to target
		UINT32          Rd_Wr_n,        // 1==Read, 0==Write
		UINT32          BarNum,         // Base Address Register (BAR) to access
		PUINT8          Buffer,         // Data buffer
		UINT64          Offset,         // Offset in data buffer to start transfer
		UINT64          CardOffset,     // Offset in BAR to start transfer
		UINT64          Length,         // Byte length of transfer
		PSTAT_STRUCT    Status          // Completion Status
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIReadDDR3(
		UINT32  board,                  // Board to target
		UINT64 DDR3SourceAddress,
		PUINT8 HostDestinationAddress,
		PUINT32 ByteLength);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIWriteDDR3(
		UINT32  board,                  // Board to target
		PUINT8 HostSourceAddress,
		UINT64 DDR3DestinationAddress,
		UINT32 ByteLength);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIMemTest  // to be used in dFLIM (4002) and ThorDAQ (4001) 
	(
		UINT32  board,                  // Board to target
		char* RetString	);				// status message on error

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetEEPROMlabel(
		UINT    boardNum,          // Board to target
		INT32 eepromIndex, // 0-based; 0 is TD board itself; const struct defines end of indexes
		char eepromID[4]);  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIReadEEPROM(
		UINT    boardNum,          // Board to target
		UINT32 fileOp,             // if 0, no file operation
		char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
		char eepromDATA[65]);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIWriteEEPROM(
		UINT    boardNum,          // Board to target
		UINT32 fileOp,             // if 0, no file operation
		char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
		char eepromDATA[65]);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIReadProdSN(
		UINT    boardNum,          // Board to target
		UINT32 fileOp,                 // if 0, no file operation, 1 record update to file
		char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
		char ProdID[10],   // TIS Production system fields
		char ProdSN[6]);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIWriteProdSN(
		UINT    boardNum,          // Board to target
		UINT32 fileOp,                 // if 0, no file operation, 1 record update to file
		char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
		char ProdID[10],   // TIS Production system fields
		char ProdSN[6]);


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
	);

	// ThorDAQAPIGetBoardCfg
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
	// ThorDAQAPIGetBoardCfg gets a copy of the current BOARD_CONFIG_STRUCT
	//   structure kept by the Driver.  No hardware accesses are
	//   initiated by calling GetBoardCfg.

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetBoardCfg
	(
		UINT					boardNum, // Board to target
		BOARD_INFO_STRUCT* board_info  // Returned structure
	);


	// for COMMON API, add all functions - objects that don't support the functions
	// have stub function returning error (e.g. dFLIM API call to ThorDAQ returns error)
	// dFLIM FUNCTIONS
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMGetClockFrequency
	(
		UINT boardNum, // Board to target
		int clockIndex,
		double& frequency);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMReSync
	(
		UINT							boardNum
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMSetImagingConfiguration
	(
		UINT							boardNum, // Board to target
		dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config  // config structure
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMSetFrontEndSettings
	(
		UINT    boardNum
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMSetFineShift
	(
		UINT    boardNum,
		LONG32  shift,
		int	channel
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMSetCoarseShift
	(
		UINT    boardNum,
		LONG32  shift,
		int	channel
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIdFLIMSetSyncingSettings
	(
		UINT    boardNum,
		ULONG32 syncDelay,
		ULONG32 resyncDelay,
		bool	forceResyncEverLine
	);

	// end of dFLIM functions
	/*
	//gets the low frequency board config
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetLowFreqTriggerBoardCfg
	(
		UINT					boardNum, // Board to target
		LOW_FREQ_TRIG_BOARD_INFO_STRUCT* board_info  // Returned structure
	);

	// returns image DMA memory module hardware info via I2C
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetDDR3status(
		UINT boardNum,            // Board to target
		CHAR* StatusString,
		UINT32 StringChars
	);
	
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetAUXGPIOvalue(
		UINT boardNum,                  // Board to target
		UINT32 FPGAindx,
		UINT32* RetValue);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetCPLD_DIOConfig(
		UINT boardNum,                // Board to target
		int BNClabel,
		int CopiedSourcelabel,
		int FPGA_AUXindex,
		int Output
	);
	
	// returns current Breakout Box status, Legacy or 3U BOB, CPLD version if 3U
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetBOBstatus( 
		UINT boardNum,            // Board to target
		CHAR* StatusString,
		UINT32 StringChars
	);
	// this function is intended to support "slow" ADC reads from legacy or 3U BOB (BreakOut Box)
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetAI( // Breakout Box "Slow" Analog Inputs
		UINT boardNum,					// Board to target
		UINT32 BNCindex,				// 0-based BNC connector index
		BOOL bVolts,					// if FALSE return raw ADC counts
		double* Value					// Volts or raw 12-bit count received from MAX127 chip
	);         
	// CONFIG the Analog IO
	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetAIOConfig( // Breakout Box Analog I/O config (ABBx and 3U Panel)
		UINT boardNum,            // Board to target
		CHAR* config,             // e.g. "AnnXDVmm", where "n" is 0-31 (BNC label "nn", e.g. "A12", "X" is Input/Output direction "I" or "O", "D" is "U"nipolar or "B"ipolar, "V" is Volts "mm" range e.g. "10"
		UINT32 configSize
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetAIOConfig( // Breakout Box Analog I/O config (ABBx and 3U Panel)
		UINT boardNum,            // Board to target
		CHAR* config,             // e.g. "AnnXDVmm", where "n" is 0-31 (BNC label "nn", e.g. "A12", "X" is Input/Output direction "I" or "O", "D" is "U"nipolar or "B"ipolar, "V" is Volts "mm" range e.g. "10"
		UINT32 configSize
	);
	// CONFIG of the Digital IO...
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetDIOConfig( // returns current Breakout Box Digital IO config
		UINT boardNum,            // Board to target
		CHAR* config,              
		UINT32 configSize
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDIOConfig( // Breakout Box "Slow" Analog Inputs
		UINT boardNum,            // Board to target
		CHAR* config,             // e.g. "DnnXmm", where "n" is 0-31 (BNC label D0 through "D31", "X" is Input/Output direction "I" or "O", "mm" is Index of Output source
		UINT32 configSize
	);
	// VALUES of the DIO...
	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDO( // SET the value of one or more DOs
		UINT boardNum,           // Board to target
		CHAR* config,            // Collection of fields per Config functions - AUX field holds value
		UINT32 configSize,       // field size 
		UINT32 NumDOs            // 1 or more DO records in "config"
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetDIO( // GET value of one or more Inputs or last-value-written for Outputs
		UINT boardNum,           // Board to target
		CHAR* config,            // Collection of fields per Config functions - AUX field holds value
		UINT32 configSize,       // field size 
		UINT32 NumDOs            // 1 or more DO records in "config"
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIProgrammableTrigger(
		UINT boardNum,
		signed char chan,
		signed char ArmASSERT
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIXI2CReadWrite(
		UINT boardNum,                  // Board to target
		bool bRead,
		UINT32 MasterMUXAddr,
		UINT32 MasterMUXChan,
		UINT32 SlaveMUXAddr,
		UINT32 SlaveMUXChan,
		UINT32 DevAddress,
		UINT32 I2CbusHz,
		INT32 PageSize,
		PUINT8 OpCodeBuffer,
		UINT32* OpCodeLen,
		PUINT8 DataBuffer,
		UINT32* DataLen); // IN: expect returned bytes Rx'd from slave  OUT: actual bytes (can be 0), Opcodes+Data
/*
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIReadI2CDevice(    // DEPRECATED
		UINT boardNum,                  // Board to target
		UINT32 MasterChan,
		INT32 SlaveChan,
		UINT32 DevAddress,
		UINT32 SlaveCommandByte,
		PUINT8 ReadBuffer,
		UINT32* ByteLen  // IN: ReadBuffer Len, OUT: byte length read
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIWriteI2CDevice(
		UINT boardNum,                  // Board to target
		UINT32 MasterChan,
		INT32 SlaveChan,
		UINT32 SlaveDevAddress,
		UINT32 SlaveCommandByte,
		PUINT8 WriteBuffer,
		UINT32* ByteLen  // IN: ReadBuffer Len, OUT: byte length read
	);
	
	// ThorDAQAPIFlashI2CSlave() DEPRECATED
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIFlashI2CSlave(   // Dedicated function for flashing through I2C
		UINT boardNum,                  // Board to target
		UINT32 MasterChan,
		INT32 SlaveChan,
		UINT32 SlaveDevAddress,
		UINT32 SlaveCommandByte,
		PUINT8 WriteBuffer,
		UINT32* ByteLen  // IN: ReadBuffer Len, OUT: byte length read
	);
	*/
	/// //////////////////////////////

/*
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIBreakOutBoxLED(
		UINT boardNum,                  // Board to target
		INT32  LEDenum,                 // -1 for all LEDs
		UCHAR  State                    // 0 for off, 1 for on, 2 for blink
	);
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
	);

//	THORDAQ_API THORDAQ_STATUS ThorDAQAPIShutdownPacketMode(
//		UINT32  board,                  // Board number to target
//		INT32   EngineOffset            // DMA Engine number offset to use
//	);



//	THORDAQ_API THORDAQ_STATUS ThorDAQAPIPacketReadEx(
//		UINT32  board,                  // Board to target
//		INT32   EngineOffset,           // DMA Engine number offset to use
//		PUINT64 UserStatus,             // User Status returned from the EOP DMA Descriptor
//		UINT64  CardOffset,             // Card Address to start read from
//		UINT32  Mode,                   // Control Mode Flags
//		PUINT8  Buffer,                 // Address of data buffer
//		PUINT32 Length                  // Length to Read
//	);
//	THORDAQ_API THORDAQ_STATUS ThorDAQAPIPacketWriteEx(
//		UINT32  board,                  // Board number to target
//		INT32   EngineOffset,           // DMA Engine number offset to use
//		UINT64  UserControl,
//		UINT64  CardOffset,
//		UINT32  Mode,                   // Control Mode Flags
//		PUINT8  Buffer,
//		UINT32  Length
//	);

	// DZ SDK ////////////////////////////////////////////////////

	// if TRUE, read DDR3 bank as it's written by S2MM DMA
	// if FALSE (i.e. capture to disk), read DDR3 bank last completed by S2MM DMA
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIProgressiveScan(
		UINT32  board,                  // Board number to target
		BOOL    bProgressiveScan);     // 


	THORDAQ_API THORDAQ_STATUS ThorDAQAPIFPGAregisterRead(
		UINT32 board,					// Board index to target
		const char* pName,				// name of register/field (if found)
		int nameSize,					// char array size
		UINT64* Value					// "shadow" DLL register copy value (register or field)
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIFPGAregisterWrite(
		UINT32 board,					// Board index to target
		const char* pName,				// name of register/field (if found)
		int nameSize,					// char array size
		UINT64* Value					// register/field value to write
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIFPGAregisterQuery(
		UINT32 board,                  // Board index to target
		int RegIndex,           // location in physical DDR3 mem to start (ADC images in low memory)      
		int FldIndex,           // length in bytes of waveformBuffer (all waveforms must be equal), or '0' if unknown
		char* pName,			// returned name of register/field (if found)
		int pNameSize,
		PREG_FIELD_DESC RegFldDesc // register/field description
	);
/*
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACwaveInit(
		UINT32 board,                  // Board number to target
		UINT64 DDR3startAddr,          // location in physical DDR3 mem to start (ADC images in low memory)      
		UINT32 LargestWaveformInBytes  // or "0" if unknown - affects descr. links/partitioning of waveformBuffer table
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACwaveLoad(
		UINT32 board,                  // Board number to target
		UINT32 DACchannel,             // DAC hardware channel 
		PUINT8 DACsampleBuffer,        // array of 16-bit waveformBuffer samples to load
		UINT32 BufferSize,             // Total size (bytes) of sample buffer
		PUINT64 DDR3startAddr);     // if successful, the DDR3 start addr where waveformBuffer is loaded

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIS2MMconfig(   // configures FPGA/BRAM for DMA of (S)tream ADC samples to(2) (M)emory (M)apped DDR3
		UINT32 board,                  // Board number to target
		PS2MM_CONFIG pS2MMconfig);     // S2MM FPGA/DMA configuration array

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIADCsampleImagizerLUT(
		UINT32 board,                  // Board number to target
		PS2MM_ADCSAMPLE_LUT pS2MMsampleLUT);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIsetDACvoltage(
		UINT32 board,                  // Board number to target
		UINT32 channel,                // DAC channel 0-12 (12 is DAC waveformBuffer, not analog chan)
		double Voltage);
*/

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIWaitForUserIRQ(
		UINT32  board,                  // Board number to target
		UINT64*  CountOfChan0_INTs,
		UINT64*  CountOfChan1_INTs,
		UINT64*  CountOfChan2_INTs,
		UINT64*  CountOfChan3_INTs
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPICancelWaitForUserIRQ(
		UINT32  board                  // Board number to target
	);


	THORDAQ_API THORDAQ_STATUS ThorDAQAPIReadFrames  // in upper level DLL
	(
		UINT    boardNum,          // Board to target
		UINT32* buffer_length,
		int Chan,                  // enabled channels (bit mask b1111)
		void* buffers[4],
		double Timeout_ms,
		ULONG transferFrames, //number if frames to transfer
		BOOL isLastTransfer, //flag to let the read function know that this is the last time we will grabing the data.
		BOOL& isPartialData //a flag used by the read function to let the caller know if the image is complete or partial
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIAbortRead
	(
		UINT    boardNum           // Board to target
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIPacketWriteBuffer
	(
		UINT     boardNum,          // Board to target
		ULONG64  register_card_offset,           // Start address to read in the card
		ULONG    Length,            // Size of the Packet Write to Data Buffer Count by Byte (FIFO Mode)
		UCHAR* Buffer,            // Data buffer (Packet Mode)
		ULONG    Timeout            // Generate Timeout error when timeout
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIStartAcquisition
	(
		UINT    boardNum          // Board to target
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIStopAcquisition
	(
		UINT    boardNum          // Board to target
	);

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
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDACParkValue
	(
		UINT    boardNum,          // Board to target
		ULONG32 outputChannel,
		double outputValue
	);


/*	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDACOffsetValue
	(
		UINT    boardNum,          // Board to target
		ULONG32 outputChannel,
		double outputValue
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetLineTriggerFrequency
	(
		UINT    boardNum,
		UINT32 sample_rate,
		double& frequency,
		ULONG32 expectedFrequency
	);
	*/

	THORDAQ_API	THORDAQ_STATUS ThorDAQAPIGetTotalFrameCount
	(
		UINT    boardNum,
		UINT32& frame_count
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIGetExternClockStatus
	(
		UINT    boardNum,
		ULONG32& isClockedSynced
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQSetDACParkValue  // Legacy DLL code call
	(
		UINT    boardNum,          // Board to target
		ULONG32 outputChannel,
		double outputValue
	);

/*
	THORDAQ_API	THORDAQ_STATUS ThorDAQAPISetClockSource
	(
		UINT    boardNum,
		CLOCK_SOURCE clock_source
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIMeasureExternClockRate
	(
		UINT    boardNum,
		ULONG32& clock_ref,
		ULONG32& clock_rate,
		ULONG32 mode
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetAllADCChannelsGain
	(
		UINT    boardNum,
		ULONG clock_source,
		ULONG32 adcGain[],
		bool isThreePhotonMode
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetThreePhotonEnable
	(
		UINT    boardNum,
		bool	threePhotonEnable
	);


	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetGRClockRate
	(
		UINT    boardNum,
		ULONG32 clock_rate,
		ULONG32 expectedFrequency
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDCOffsetPreFIR
	(
		UINT    boardNum, // Board to target
		short	preDcOffset, //pre FIR filter DC Offset value
		USHORT  channel		 //offset's target channel
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIToggleAuxDigitalOutputs
	(
		UINT    boardNum, // Board to target
		USHORT  auxChannelIndex, //Aux Digital channel to toggle, 0 based
		USHORT  value  // Value to toggle the channel to: 1 = high and 0 = low
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetThreePhotonSampleOffset
	(
		UINT    boardNum, // Board to target
		UINT8  threePhotonSampleOffset  // Value to toggle the channel to: 1 = high and 0 = low
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetConfigurationWithFullwWaveform(
		UINT    boardNum, // Board to target
		DAC_FREERUN_WAVEFORM_CONFIG& waveformConfig
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACSetWaveformConfigurationForDynamicLoad(
		UINT    boardNum, // Board to target
		DAC_FREERUN_WAVEFORM_CONFIG& waveformConfig
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIStartDACWaveforms(
		UINT    boardNum // Board to target
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIStopDACWaveforms(
		UINT    boardNum // Board to target
	);
	
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACRegisterApproachingNSampleCallback(
		UINT    boardNum, // Board to target
		UINT8	dacChannel,
		UINT32	nSamples, 
		UINT32	options, 
		ThorDAQDACApproachingNSamplesCallbackPtr callbackFunction, 
		void* callbackData
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACRegisterCycleDoneEvent(
		UINT    boardNum, // Board to target
		UINT8	dacChannel, 
		UINT32	options, 
		ThorDAQDACCycleDoneCallbackPtr callbackFunction, 
		void* callbackData
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACRegisterWaveformPlaybackCompleteEvent(
		UINT    boardNum, // Board to target
		UINT32	options,
		ThorDAQDACWaveformPlaybackCompleteCallbackPtr callbackFunction,
		void* callbackData
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACRegisterWaveformPlaybackStartedEvent(
		UINT    boardNum, // Board to target
		UINT32	options,
		ThorDAQDACWaveformPlaybackStartedCallbackPtr callbackFunction,
		void* callbackData
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACRegisterApproachingLoadedWaveformEndEvent(
		UINT    boardNum, // Board to target
		UINT8	dacChannel,
		UINT32	options,
		ThorDAQDACApproachingLoadedWaveformEndCallbackPtr callbackFunction,
		void* callbackData
	);


	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACGetMinSamples(
		UINT    boardNum, // Board to target
		double dacUpdateRate,
		UINT64& minSamples
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACDynamicLoadPresetWaveform(
		UINT    boardNum, // Board to target
		bool isLastPartOfWaveform
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACDynamicLoadWaveform(
		UINT    boardNum, // Board to target
		std::map<UINT, DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> dacCtrlSettings,
		bool isLastPartOfWaveform
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACPresetNextWaveformSection(
		UINT    boardNum, // Board to target
		std::map<UINT,
		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> dacCtrlSettings, bool isLastPartOfWaveform
	);
	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACBankSwitchingLoadNextWaveform(
		UINT    boardNum, // Board to target
		std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> dacCtrlSettings
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPIDACBankSwitchingRegistergReadyForNextImageWaveformsEvent(
		UINT    boardNum, // Board to target
		UINT32	options,
		ThorDAQDACBankSwitchingReadyForNextWaveformCallbackPtr callbackFunction,
		void* callbackData
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDIOChannelSelection(
		UINT    boardNum, // Board to target
		UINT8 DIOSelection[DIO_CHANNEL_COUNT]
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetScanActiveLineInvert(
		UINT    boardNum, // Board to target
		bool invertCaptureActive
	);
	
	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDefaultBOBMapping(
		UINT    boardNum, // Board to target
		bool	initialParkAllAtZero
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISlowSmoothMoveToAndFromParkEnable(
		UINT    boardNum, // Board to target
		bool	enable
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDDSClockEnable(
		UINT    boardNum, // Board to target
		bool	enable
	);

	THORDAQ_API THORDAQ_STATUS ThorDAQAPISetDDSClockPhase(
		UINT    boardNum, // Board to target
		int		channel,
		double	phase
	);
*/
#ifdef __cplusplus
}
#endif

#endif