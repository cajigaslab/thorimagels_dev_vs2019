/*++

Copyright (c)2021 Thorlabs, Inc.  All rights reserved.

Module Name: thordaqAPI.cpp
Created by DZimmerman

Abstract:

	Defines the "new" API functions for the thordaq driver, using NWL-model driver

Environment:

	user mode only.

Style:
	Google C++ coding style.
Note:
	Move implementations of API functions to this file, out of thordaq.cpp
--*/
#include "stdafx.h"
#pragma warning(disable:4201) //I just wanna use freedom nonstandard "nameless struct/union"
#include "thordaq.h"
#include "thordaqapi.h"
#include "thordaqguid.h"

THORDAQ_STATUS CThordaq::APIProgrammableTrigger(
	CThordaq& TdBrd, // This CThordaq object by reference	
	signed char chan,
	bool ArmASSERT
)
{
	// sanity check args
	if (chan < -1 || chan > 13)
		return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;

	if (chan == -1) // legacy ImageAcq trigger?
	{
		CThordaq::ImageAcqTrigger->ProgrammableTriggerArmASSERT(TdBrd, chan, ArmASSERT);
	}
	else // a WaveformControl trigger
	{

	}

	return STATUS_SUCCESSFUL;
}


// private function to support MUXing, to find the DIO value of BNC according
// to FPGA index
THORDAQ_STATUS CThordaq::GetDIOvalueForFPGAindex(CThordaq& TdBrd, int iFPGAindex, int *BNCindex, uint8_t *uiValue)
{
	// search the BOB_DIOSettings[] array by FPGAindex to find BNCindex
	int BNCindx = 0;
	for (BNCindx = 0; BNCindx < 32; BNCindx++)
	{
		if (iFPGAindex == BOB_DIOSettings[BNCindx].FPGAioIndex)
		{
			*BNCindex = (int)BOB_DIOSettings[BNCindx].BNClabel;
			if(BOB_DIOSettings[BNCindx].bOutputDir == TRUE)
				*uiValue = (uint8_t)BOB_DIOSettings[BNCindx].OUTvalue;
			else
				*uiValue = (uint8_t)BOB_DIOSettings[BNCindx].INvalue;

			break;
		}
	}
	if( BNCindx >= 32)
		return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR; // should NOT happen
	return THORDAQ_STATUS::STATUS_SUCCESSFUL;
}
// private function to support CPLD I2C read/write (ONLY on 3U Panel BOB)
// in Verilog i2c_gpio.v:
// GPIO_write 0x01 (output from FPGA to CPLD's BNC)
// GPIO_read  0x05 (input to FPGA)
// NOTE!  The API always expects "value" according to the BNCindex (label)
// but with MUXing we must write to hardware according to the FPGAindex
// WE CANNOT READ/WRITE single bits, only bytes from I2C, so we have to
// MASK single bit values appropriately
THORDAQ_STATUS CThordaq::ReadWriteBOB_CPLD_DIO(
	CThordaq& TdBrd,                // Board to target
	int iBNCindex)    // VALUE per BNC - read/write per FPGA side index
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
	// map the 3U BOB CPLD
	UINT32 I2CmasterMux = 2, I2CslaveMux = 0xFF, I2CslaveAddress = 0x46;
	UINT32 I2CopcodeLen, I2CdataLen;
	UINT8 I2CopcodeBuffer[8];
	UINT8 I2CdataBuffer[64];
	// I2C command, for example, CPLD I2C slave 0x46, read "slow" input index
	// Index 0:  FPGA-side_index 8-15
	//       1:  FPGA-side_index 16-23
	//       2:  FPGA-side_index 24-31
	
	// e.g. to read bit 26 (in 2nd byte)
	// 0x46 0x05 0x2 
	// Then bit-mask the correct bit position

	int iFPGAindex = (int)BOB_DIOSettings[iBNCindex].FPGAioIndex;
	// Access the CPLD registers for indexes 8-31; bitwise index (starting at 0x08) of command (2nd byte)
	int8_t CPLD_ioReg = (BYTE)(iFPGAindex / 8) -1; // should be 0, 1 or 2
	if ( (CPLD_ioReg < 0) || CPLD_ioReg > 2) return status; // bogus argument
	
	if (BOB_DIOSettings[iBNCindex].bOutputDir == TRUE)
	{
		I2CopcodeLen = 0; // (no I2C Opcodes)
		I2CdataBuffer[0] = 0x01; // WRITE I2C command byte (1st byte) 
		// NOTE!  The bit value (bool) to write has already been set by caller (per BNC),
		// so reconstruct the entire "shadow" bit's byte that 
		// currently exists in BOB_DIOSettings - this is a MESS because of potential MUXing


		I2CdataBuffer[1] = CPLD_ioReg; // ioReg 0 is FPGA regs 8-15, 1 is 16-23, etc.
		// we need to mask in the bit we are changing
		int iStartIndex = CPLD_ioReg * 8 + 8;  // 8, 16 or 24
		uint8_t ui8I2Cdata = 0;
		
		// look-up FPGAindex shadow-reg values for the output byte
		int iMUXedBNCindex = 0;
		uint8_t ui8_DIOvalue = 0;
		int k;
		for (k = iStartIndex; k < iStartIndex+8; k++)  // this is the FPGA register index; we need 8 contiguous bits
		{
			status = GetDIOvalueForFPGAindex(TdBrd, k, &iMUXedBNCindex, &ui8_DIOvalue);

			ui8I2Cdata |= (ui8_DIOvalue << (k % 8));
		}
		I2CdataBuffer[2] = ui8I2Cdata;
		I2CdataLen = 3;  // CMD, Reg, Data
		//                               WRITE
		status = APIXI2CReadWrite(TdBrd, FALSE, 0x71, I2CmasterMux, 0x70, I2CslaveMux, I2CslaveAddress, 400, 16, I2CopcodeBuffer, &I2CopcodeLen, I2CdataBuffer, &I2CdataLen);
	}
	else // INPUT case (must use Opcodes)
	{
		I2CopcodeBuffer[0] = 0x05;
		I2CopcodeBuffer[1] = CPLD_ioReg;
		I2CopcodeLen = 2;          // send "0x05 0x02" (for 0-based index 2)
		I2CdataLen = 1;            // read single byte
		status = APIXI2CReadWrite(TdBrd, TRUE, 0x71, I2CmasterMux, 0x70, I2CslaveMux, I2CslaveAddress, 400, 16, I2CopcodeBuffer, &I2CopcodeLen, I2CdataBuffer, &I2CdataLen);
		if (status == THORDAQ_STATUS::STATUS_SUCCESSFUL)
		{
			// we need bit masking for the 8 bits grouped with desired bit
			// 
			BYTE I2Cbyte = I2CdataBuffer[0];
			int iMaskVal = (I2Cbyte >> (iFPGAindex % 8)) & 0x1;  // shift from 0 to 7, then select only bit0
			 
			TdBrd.BOB_DIOSettings[iFPGAindex].INvalue = (UINT64)iMaskVal;
		}
	}

	return status;
}

// This function supports rapid successive reading of an AUX GPIO register, originally
// for attempting to get a DIO square wave freqency check
// Function assumes APISetDIOConfig has correctly been set up -
// all we do here is read the MUX register (27-31, assumes AUX indexes 0-4

THORDAQ_STATUS CThordaq::APIGetAUXGPIOvalue(
	CThordaq& TdBrd,
	UINT32 MUXindex,
	UINT32* BitValue
)
{
	THORDAQ_STATUS status = (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;
	UINT64 uiValue;

	// range check 
	if( MUXindex < 27 || MUXindex > 31)
		return status;

	switch (MUXindex - 27)
	{
	case 0: 
		status = FPGAregisterREAD("Global_Aux_GPI_0", &uiValue);
		break;
	case 1:
		status = FPGAregisterREAD("Global_Aux_GPI_1", &uiValue);
		break;
	case 2:
		status = FPGAregisterREAD("Global_Aux_GPI_2", &uiValue);
		break;
	case 3:
		status = FPGAregisterREAD("Global_Aux_GPI_3", &uiValue);
		break;
	case 4:
		status = FPGAregisterREAD("Global_Aux_GPI_4", &uiValue);
		break;

	default:
		return status;
	}
	*BitValue = (UINT32)uiValue;
	
	return THORDAQ_STATUS::STATUS_SUCCESSFUL;
}


THORDAQ_STATUS CThordaq::APIWaitForUserIRQ(UINT64* CountOfChan0_INTs, UINT64* CountOfChan1_INTs, UINT64* CountOfChan2_INTs, UINT64* CountOfChan3_INTs)
{
	THORDAQ_STATUS status = (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;
	USER_IRQ_WAIT_STRUCT UsrIrqWaitStruct;

	*CountOfChan0_INTs = 1;
	*CountOfChan1_INTs = 2;
	*CountOfChan2_INTs = 3;
	*CountOfChan3_INTs = 4;
	UsrIrqWaitStruct.dwTimeoutMilliSec = 0x7FFFFFFF; // we essentially block forever waiting for FPGA interrupt or IOCTL Cancel call

	do {
		// UserIRQWait() waits forever for INTERRUPT (success status) or TIMEOUT error caused by UserIRQCancel()
		// when acquisition stops
		status = UserIRQWait(&UsrIrqWaitStruct, CountOfChan0_INTs, CountOfChan1_INTs, CountOfChan2_INTs, CountOfChan3_INTs);  // synchronous - blocking; SUCCESSFUL status means IRQ received
		if (status != STATUS_SUCCESSFUL)
		{
			break;   // UserIRQCancel() causes a "not" success status & normal thread exit
		}
	} while (status == STATUS_SUCCESSFUL);  // "SUCCESS" defined as getting an IRQ

	return status;
}
THORDAQ_STATUS CThordaq::APICancelWaitForUserIRQ()
{
	THORDAQ_STATUS status = (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;

	status = UserIRQCancel();

	return status;
}


THORDAQ_STATUS CThordaq::APISetDIOConfig(
	CThordaq& TdBrd,                // Board to target
	CHAR* config,
	UINT32 configSize  // byte size of caller's config buffer
)
{
	THORDAQ_STATUS status = (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;

	status = (THORDAQ_STATUS)CThordaq::SetBOB_DIOConfig(TdBrd, config, configSize); // configure a single DIO

	return status;
}

// use I2C commands to configure CPLD DIOs
THORDAQ_STATUS CThordaq::APISetCPLD_DIOConfig(
	CThordaq& TdBrd, // Board to target
	int BNClabel,    // BNC index on 3U BOB
	int FPGAsourceIndex, // indexes 0-7 are to/from FPGA (DBB1 cable), others controlled by I2C commands
	int HSC,         // 1 if signal to/from TD FPGA's DBB1 (which means Sourcelabel indexes DIO1-8), else 0
	int Input)       // 0 if Output, 1 for Input
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
	// map the 3U BOB CPLD
	UINT32 I2CmasterMux = 2, I2CslaveMux = 0xFF, I2CslaveAddress = 0x46;
	UINT32 I2CopcodeLen, I2CdataLen;
	UINT8 I2CopcodeBuffer[8];
	UINT8 I2CdataBuffer[64];
	// CHANGE to GPIO WRITE Cmd 0x1, starting at Reg 0x3 (Regs 0-2 are used for BNC DATA)
	// 0x46 0x01 0x03 0x80  (Reg 0 [BNC D0], Input, FPGAindex 0) 
	I2CdataBuffer[0] = 0x01; // command byte for "WRITE GPIO" in CPLD 
	// The "BNClabel" is the "write index" of the command (2nd byte
	I2CdataBuffer[1] = (UINT8)BNClabel + 3; // the "Destination" port controlled by CPLD

	// CONFIG byte:
	// Bits 4-0 -- Index (0-based) of FPGA source (indexes 0-7 are the DBB1 HSC signals to/from FPGA)
	// Bit 7    -- Direction, '1' for Input, '0' for Output (FPGA defined direction - inverse in CPLD PCB hardware)
	// (e.g. 0x84 Define) is HSC, Input, and FPGA's DIO5 0-based index 4 ("DIO5")
	
	I2CopcodeLen = 0;
	I2CdataBuffer[2] = 0;
	I2CdataBuffer[2] = (UINT8)( (Input << 7) | FPGAsourceIndex);
	// write the config command
	I2CdataLen = 3;
	status = APIXI2CReadWrite(TdBrd, FALSE, 0x71, I2CmasterMux, 0x70, I2CslaveMux, I2CslaveAddress, 400, 16, I2CopcodeBuffer, &I2CopcodeLen, I2CdataBuffer, &I2CdataLen);

	return status;
}
// See JEDEC SPD (Serial Presence Detect) DDR3 format definition (e.g. SPD Annex X)
// capacity of the DDR3 EEPROM is 256
#define MicronDDR3_EEPROM_LEN 176 // only decode fields in this initial segment
THORDAQ_STATUS CThordaq::APIGetDDR3status( // returns current Breakout Box status
	CThordaq& TdBrd,             // Board to target
	CHAR* StatusString,
	UINT32 StringChars)
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	THORDAQ_STATUS I2Cstatus;
	wchar_t logMsg[MSG_SIZE]; // log DDR3 info

	UINT32 CharsRemaining = StringChars;
	UINT32 I2CmasterMux, I2CslaveMux;
	UINT32 I2CbufferLen, I2CopcodeLen;
	UINT8 I2CbyteBuffer[MicronDDR3_EEPROM_LEN];
	I2CbufferLen = MicronDDR3_EEPROM_LEN;      // # capacity of EEPROM bytes we expect to read (176-255 expect 0xFFF...)
	UINT8 I2CopcodeBuffer[8];
	UINT32 I2CslaveAddress = 0x50; // DDR3 slave addr
	I2CmasterMux = 0x4; I2CslaveMux = 0xff; // I2C map to DDR3
	I2CopcodeBuffer[0] = 0; // starting BYTE addr in EEPROM to READ
	I2CopcodeLen = 1;
	I2Cstatus = APIXI2CReadWrite(TdBrd, TRUE, 0x71, I2CmasterMux, 0x70, I2CslaveMux, I2CslaveAddress, 400, 16, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);

	if (I2Cstatus != THORDAQ_STATUS::STATUS_SUCCESSFUL)
	{
		StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::APIGetDDR3status: FATAL I2C ERROR reading ThorDAQ DDR3 image memory card");
		CThordaq::LogMessage(logMsg, ERROR_EVENT);
		return I2Cstatus;
	}
	int CharsLimitToCaller = TD_DDR3_SPD::DDR3_SPD_LEN;
	char ReturnString[TD_DDR3_SPD::DDR3_SPD_LEN];
	if (StringChars < TD_DDR3_SPD::DDR3_SPD_LEN)
		CharsLimitToCaller = StringChars; // DO NOT exceed caller's buffer!

	wchar_t wcSPDBytesUsed[TD_DDR3_SPD::Bytes_Used_LEN], wcSPDRev[TD_DDR3_SPD::SPD_Rev_LEN], wcMfgYear[TD_DDR3_SPD::MFR_Year_LEN], wcMfgWeek[TD_DDR3_SPD::MFR_Week_LEN], wcSerNum[TD_DDR3_SPD::DDR3SerialNum_LEN];
	char SPDBytesUsed[TD_DDR3_SPD::Bytes_Used_LEN], SPDRev[TD_DDR3_SPD::SPD_Rev_LEN], MfgYear[TD_DDR3_SPD::MFR_Year_LEN], MfgWeek[TD_DDR3_SPD::MFR_Week_LEN], SerNum[TD_DDR3_SPD::DDR3SerialNum_LEN];
	UINT32 SerialNum;
	SerialNum = (UINT32)(I2CbyteBuffer[TD_DDR3_SPD::DDR3SerialNum] << 24 & 0xFF000000) + (UINT32)(I2CbyteBuffer[TD_DDR3_SPD::DDR3SerialNum + 1] << 16 & 0x00FF0000) +
		(UINT32)(I2CbyteBuffer[TD_DDR3_SPD::DDR3SerialNum + 2] << 8 & 0x0000FF00) + (UINT32)(I2CbyteBuffer[TD_DDR3_SPD::DDR3SerialNum + 3] & 0xFF);
	wchar_t wcPartNumber[20];
	wchar_t wcDDR3ModulePartNumber[40];
	memset(wcDDR3ModulePartNumber, 0, sizeof(wcDDR3ModulePartNumber));
	char DDR3ModulePartNumber[TD_DDR3_SPD::DDR3PartNum_LEN+1]; // the FieldName + part num field + ';'
	char PartNumber[TD_DDR3_SPD::DDR3PartNum_LEN];  // i.e., the DDR3 part num field (17 chars)
	memset(PartNumber, 0, TD_DDR3_SPD::DDR3PartNum_LEN);
	memcpy(PartNumber, &I2CbyteBuffer[TD_DDR3_SPD::DDR3PartNum], 17); // (see field def.)
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcPartNumber, (sizeof(wcPartNumber)/sizeof(wchar_t)), PartNumber, 17);

	swprintf(wcSPDBytesUsed, TD_DDR3_SPD::Bytes_Used_LEN, L"BytesUsed 0x%X;", (UINT8)I2CbyteBuffer[TD_DDR3_SPD::Bytes_Used]);
	swprintf(wcSPDRev, TD_DDR3_SPD::SPD_Rev_LEN, L"SPD Rev 0x%X;", (UINT8)I2CbyteBuffer[TD_DDR3_SPD::SPD_Rev]);
	swprintf(wcMfgYear, TD_DDR3_SPD::MFR_Year_LEN, L"Mfg Year 20%x;", (UINT8)I2CbyteBuffer[TD_DDR3_SPD::MFR_Year]);
	swprintf(wcMfgWeek, TD_DDR3_SPD::MFR_Week_LEN, L"Mfg Week %x;", (UINT8)I2CbyteBuffer[TD_DDR3_SPD::MFR_Week]);
	swprintf(wcSerNum, TD_DDR3_SPD::DDR3SerialNum_LEN, L"SerialNum %X;", SerialNum);
	swprintf(wcDDR3ModulePartNumber, TD_DDR3_SPD::DDR3PartNum_LEN, L"PartNum %s", wcPartNumber);

	// the DDR3 mem card answered us - parse fields, separate string fields with ";"
	snprintf(SPDBytesUsed, sizeof(SPDBytesUsed), "BytesUsed 0x%X;",  (UINT8)I2CbyteBuffer[TD_DDR3_SPD::Bytes_Used]);
	snprintf(SPDRev, sizeof(SPDRev), "SPD Rev 0x%x;", (UINT8)I2CbyteBuffer[TD_DDR3_SPD::SPD_Rev]);
	snprintf(MfgYear, sizeof(MfgYear), "Mfg Year 20%x;", (UINT8)I2CbyteBuffer[TD_DDR3_SPD::MFR_Year]);
	snprintf(MfgWeek, sizeof(MfgWeek), "Mfg Week %x;", (UINT8)I2CbyteBuffer[TD_DDR3_SPD::MFR_Week]);
	snprintf(SerNum, sizeof(SerNum), "SerialNum %X;", SerialNum);
	snprintf(DDR3ModulePartNumber, sizeof(DDR3ModulePartNumber), "PartNum %s;", PartNumber);
	// record to ThorDAQ event log
	StringCbPrintfW(logMsg, MSG_SIZE, L"TD::APIGetDDR3status: %s %s %s %s %s %s", 
		wcDDR3ModulePartNumber, wcMfgWeek, wcMfgYear, wcSerNum,  wcSPDBytesUsed, wcSPDRev);
	CThordaq::LogMessage(logMsg, INFORMATION_EVENT);

	// return to caller
	snprintf(ReturnString, CharsLimitToCaller, "%s %s %s %s %s %s", DDR3ModulePartNumber, MfgYear, MfgWeek, SerNum, SPDRev, SPDBytesUsed);
	memcpy(StatusString, ReturnString, CharsLimitToCaller);
	return status;
}

// (re)Discover the BOB type, and LOG result in ThorDAQ event logger, only in this function
// to avoid blowing up EventLog in case no BOB is attached
THORDAQ_STATUS CThordaq::APIGetBOBstatus( // returns current Breakout Box status
	CThordaq& TdBrd,             // Board to target
	CHAR* StatusString,
	UINT32 StringChars)
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	THORDAQ_STATUS I2Cstatus;
	wchar_t logMsg[MSG_SIZE]; // log BOB found or config errors
	bool bCableSwapOrError = false; // assume OK
	bool bLightConnectedLED;

	bool bFirstDetectionOfBOB = ((CThordaq::BOB_HardwareType != 'L')  && CThordaq::BOB_HardwareType != 'P') ? true : false;
	// only set the default ONCE!
	I2Cstatus = DiscoverBOBtype(TdBrd);
	if (I2Cstatus != THORDAQ_STATUS::STATUS_SUCCESSFUL)
	{
		StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::DiscoverBOBtype: FATAL I2C ERROR Reading BreakOutBox");
		CThordaq::LogMessage(logMsg, ERROR_EVENT);
		return I2Cstatus;
	}

	// now update TD EVENT LOG & prepare the API return string...
	UINT32 StringCharLen = TD_BOBstatusDef::BOB_ALL;
	char ReturnStatusString[TD_BOBstatusDef::BOB_ALL];
	memset(ReturnStatusString, 0, (size_t)TD_BOBstatusDef::BOB_ALL); // we are copying entire buffer back to caller
	UINT32 StringCharIndex = 0;
	if (StringChars < StringCharLen)
		StringCharLen = StringChars; // DO NOT exceed caller's buffer

	long EvntLevel = VERBOSE_EVENT;
	UINT64 ABBXmuxRegValue = 0;
	switch (CThordaq::BOB_HardwareType)
	{
	case 'L':
		StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::DiscoverBOBtype: Found Legacy BOBs");
		snprintf(ReturnStatusString, TD_BOBstatusDef::BOBtypeCharLen, "Legacy BOBs");
		// set the default ABBX MUX config
		ABBXmuxRegValue = 0x89BA45670123; // for "contiguous channel" ABBx Legacy BOBs (NOT consistent with old TILS versions)
		break;
	case 'P':
		StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::DiscoverBOBtype: Found integrated 3U Panel BOB");
		snprintf(ReturnStatusString, TD_BOBstatusDef::BOBtypeCharLen, "3U Panel BOB");
		// set the default ABBX MUX config
		ABBXmuxRegValue = 0x0000AB3276549810;    // for 3U BOB (Rev3, Rev1 is different!)
		break;
	default:
		StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::DiscoverBOBtype: UNKNOWN BreakOutBox type!");
		snprintf(ReturnStatusString, TD_BOBstatusDef::BOBtypeCharLen, "Unknown BOB");
		EvntLevel = ERROR_EVENT;
		break;
	}
	if (bFirstDetectionOfBOB)
	{ // write the default ABBX MUX config
		status = FPGAregisterWRITE("GlobalABBXmuxReg", ABBXmuxRegValue);
	}
	CThordaq::LogMessage(logMsg, EvntLevel);

	/* TD_BOBstatusDef
		   BOBtypeCharLen = 13, // e.g. "3U Panel BOB"
			BOB_DDB1_SN    = 29, // e.g. DB100105225601014205/10/2019 (defined field as of Oct-2021 28 chars)
			BOB_ADB1_SN = 29, // e.g. AB1...
			BOB_ADB2_SN = 29, // e.g. AB2...
			BOB_ADB3_SN = 29, // e.g. AB3...
			BOB_status =  32, // e.g. "CABLE SWAPPED", "no CPLD program", "Missing DIO/AIO", "OK"
			DAC_CPLD    = 16,  // e.g. "DACCPLD 1.0.0.1" or "DACCPLD error"
			// valid for 3U BOB only...
			BOB_CPLD = 16     // e.g. BOB...
	*/
	 // "3U Panel BOB""
	// read the EEPROM revisions
	UINT8 I2CbyteBuffer[64];
	UINT8 I2CopcodeBuffer[8];
	UINT32 I2CbufferLen = 0;
	UINT32 I2CopcodeLen;
	UINT32 I2CslaveAddress = 0x50; // def. to BOB EEPROMs
	UINT32 I2CmasterMux, I2CslaveMux;

	I2CopcodeBuffer[0] = 0; // starting BYTE addr in EEPROM to READ
	I2CopcodeLen = 1;
	
	// READ the EEPROMs 
	// DBB1
	I2CmasterMux = 0x2; I2CslaveMux = 0xff;
	I2CopcodeLen = 1;
	I2CbufferLen = 28;      // # of defined bytes in the EEPROM string we expect to read
	I2Cstatus = APIXI2CReadWrite(TdBrd, TRUE, 0x71, I2CmasterMux, 0x70, I2CslaveMux, I2CslaveAddress, 400, 16, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
	StringCharIndex += TD_BOBstatusDef::BOBtypeCharLen; // point to next field
	if (I2Cstatus == THORDAQ_STATUS::STATUS_SUCCESSFUL) // did we receive the EEPROM contents?
	{
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::BOB_DBB1_SN, "%s", I2CbyteBuffer);
		if (strncmp((const char*)I2CbyteBuffer, "DB", 2))
		{
			bCableSwapOrError = true; // HDMI cable connected wrong (or EEPROM failed)
		}
	}
	else
	{
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::BOB_DBB1_SN, "%s", "DB Cable/I2C comm fail");
		bCableSwapOrError = true; // HDMI cable failed
	}
	// ABB1
	I2CmasterMux = 0x8; I2CslaveMux = 0x1;
	I2CopcodeLen = 1;
	I2CbufferLen = 28;      // # of defined bytes in the EEPROM string we expect to read
	I2Cstatus = APIXI2CReadWrite(TdBrd, TRUE, 0x71, I2CmasterMux, 0x70, I2CslaveMux, I2CslaveAddress, 400, 16, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
	StringCharIndex += TD_BOBstatusDef::BOB_DBB1_SN; // point to next field
	if (I2Cstatus == THORDAQ_STATUS::STATUS_SUCCESSFUL) // did we receive the EEPROM contents?
	{
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::BOB_ABB1_SN, "%s", I2CbyteBuffer);
		if (strncmp((const char*)I2CbyteBuffer, "AB1", 3))
		{
			bCableSwapOrError = true; // HDMI cable connected wrong (or EEPROM failed)
		}
	}
	else
	{
		bCableSwapOrError = true; // AB1 MUST be connected in Legacy or 3U case
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::BOB_ABB1_SN, "%s", "AB1 Cable/I2C comm fail");
	}
	// ABB2
	// NOTE - 3U Panel BOB must have 3 three HDMI ABBx cables connected!  Not the case for Legacy BOB
	I2CmasterMux = 0x8; I2CslaveMux = 0x2;
	I2CopcodeLen = 1;
	I2CbufferLen = 28;      // # of defined bytes in the EEPROM string we expect to read
	I2Cstatus = APIXI2CReadWrite(TdBrd, TRUE, 0x71, I2CmasterMux, 0x70, I2CslaveMux, I2CslaveAddress, 400, 16, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
	StringCharIndex += TD_BOBstatusDef::BOB_ABB1_SN; // point to next field
	if (I2Cstatus == THORDAQ_STATUS::STATUS_SUCCESSFUL) // did we receive the EEPROM contents?
	{
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::BOB_ABB2_SN, "%s", I2CbyteBuffer);
		if (strncmp((const char*)I2CbyteBuffer, "AB2", 3))
		{
			bCableSwapOrError = true; // HDMI cable connected wrong (or EEPROM failed)
		}
	}
	else
	{
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::BOB_ABB2_SN, "%s", "AB2 Cable/I2C comm fail");
		if (CThordaq::BOB_HardwareType == 'P') // 3U BOB requires ALL HDMI cables working
		{
			bCableSwapOrError = true; // HDMI cable connected wrong (or EEPROM failed)
		}
	}

	// ABB3
	I2CmasterMux = 0x8; I2CslaveMux = 0x8;
	I2CopcodeLen = 1;
	I2CbufferLen = 28;      // # of defined bytes in the EEPROM string we expect to read
	I2Cstatus = APIXI2CReadWrite(TdBrd, TRUE, 0x71, I2CmasterMux, 0x70, I2CslaveMux, I2CslaveAddress, 400, 16, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
	StringCharIndex += TD_BOBstatusDef::BOB_ABB2_SN; // point to next field
	if (I2Cstatus == THORDAQ_STATUS::STATUS_SUCCESSFUL) // did we receive the EEPROM contents?
	{
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::BOB_ABB3_SN, "%s", I2CbyteBuffer);
		if (strncmp((const char*)I2CbyteBuffer, "AB3", 3))
		{
			bCableSwapOrError = true; // HDMI cable connected wrong (or EEPROM failed)
		}
	}
	else
	{
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::BOB_ABB3_SN, "%s", "AB3 Cable/I2C comm fail");
		if (CThordaq::BOB_HardwareType == 'P') // 3U BOB requires ALL HDMI cables working
		{
			bCableSwapOrError = true; // HDMI cable connected wrong (or EEPROM failed)
		}
	}
	StringCharIndex += TD_BOBstatusDef::BOB_ABB3_SN; // point to BOB status field

	// add BOB/CABLE status; if a cable "xBn" does not match (e.g. "AB2" instead of "DB1"), denote
	if(bCableSwapOrError)
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::BOB_status, "%s", "BOB cable swap or I2C COMM fail");
	else
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::BOB_status, "%s", "OK");

	StringCharIndex += TD_BOBstatusDef::BOB_status; // point to next field

	// and the DAC CPLD version
	I2CmasterMux = 0x8; I2CslaveMux = 0xFF;
	I2CopcodeLen = 4;
	memset(I2CopcodeBuffer, 0, 4);
	I2CopcodeBuffer[0] = 0xC0; // Lattice OpCode USERCODE command, 0xC0 0 0 0
	I2CslaveAddress = 0x40;  // DAC CPLD slave
	I2CbufferLen = 4;      // 4 byte USERCODE
	I2Cstatus = APIXI2CReadWrite(TdBrd, TRUE, 0x71, I2CmasterMux, 0x70, I2CslaveMux, I2CslaveAddress, 400, 16, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
	if (I2Cstatus == THORDAQ_STATUS::STATUS_SUCCESSFUL) // did we receive the USERCODE?
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::DAC_CPLD, "DACCPLD %c.%c.%c.%c", I2CbyteBuffer[0], I2CbyteBuffer[1], I2CbyteBuffer[2], I2CbyteBuffer[3]);
	else 
	{
		snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::DAC_CPLD, "%s", "DACCPLD error");
		bCableSwapOrError = true; // CPLD must answer; it won't if NOT programmed correctly
	}
	StringCharIndex += TD_BOBstatusDef::DAC_CPLD; // point to next field

	// and if 3U Panel, read Panel CPLD version
	if (CThordaq::BOB_HardwareType == 'P')
	{
		I2CmasterMux = 0x2; I2CslaveMux = 0xFF; // CPLD is on DBB1 I2C bus
		I2CopcodeLen = 4;
		memset(I2CopcodeBuffer, 0, 4);
		I2CopcodeBuffer[0] = 0xC0; // Lattice OpCode USERCODE command, 0xC0 0 0 0
		I2CslaveAddress = 0x44;  // DAC CPLD slave
		I2CbufferLen = 4;      // 4 byte USERCODE
		I2Cstatus = APIXI2CReadWrite(TdBrd, TRUE, 0x71, I2CmasterMux, 0x70, I2CslaveMux, I2CslaveAddress, 400, 16, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
		if (I2Cstatus == THORDAQ_STATUS::STATUS_SUCCESSFUL) // did we receive the USERCODE?
			snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::BOB_CPLD, "BOBCPLD %c.%c.%c.%c", I2CbyteBuffer[0], I2CbyteBuffer[1], I2CbyteBuffer[2], I2CbyteBuffer[3]);
		else
		{
			snprintf(&ReturnStatusString[StringCharIndex], TD_BOBstatusDef::DAC_CPLD, "%s", "BOBCPLD error");
			bCableSwapOrError = true; // CPLD must answer; it won't if NOT programmed correctly
		}
	}

	// copy complete string back to caller
	memcpy(StatusString, ReturnStatusString, StringCharLen);

	if (CThordaq::BOB_HardwareType == 'P') // no such LED on Legacy BOB
	{
	//	UCHAR State = 0; // def. off
		bLightConnectedLED = (bCableSwapOrError) ? false : true; // if OK, light Connected LED
		int BBoxEnum = BBoxLEDenum::DC1;
		UCHAR State = (bLightConnectedLED) ? 1 : 0; // ON/off, don't blink
//		Control[0] = (bLightConnectedLED) ? 1 : 0; // ON/off
//		Control[0] += 2; // IMAX/2 amps, ON/Off
//		status = LEDControlIS31FL(TdBrd, BBoxEnum, State);
		status = APIBreakOutBoxLED(TdBrd, BBoxEnum, State);
	}
	return status;
}


// DIO get/set configuration (via I2C on 3U BOB) "slow" functions expected at config time
THORDAQ_STATUS CThordaq::APIGetDIOConfig(
	CThordaq& TdBrd,                // Board to target
	CHAR* config,
	UINT32 configSize  // byte size of caller's config buffer
)
{
	THORDAQ_STATUS status = (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;

	status = (THORDAQ_STATUS)CThordaq::GetBOB_DIOConfig(TdBrd, config, configSize);

	return status;
}
