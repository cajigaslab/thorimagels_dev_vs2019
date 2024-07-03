#include "stdafx.h"
#pragma once
#include "dFLIM_4002.h"
#include "HW_EEPROM.h"
#include <fstream>
#include <limits>
//#include <filesystem>


// LOCAL functions
// 





THORDAQ_STATUS CdFLIM_4002::APIReadProdSN(
	CdFLIM_4002& TdBrd,
	UINT32 fileOp,                 // if 0, no file operation, 1 record update to file
	char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
	char ProdID[10],   // TIS Production system fields
	char ProdSN[6])
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	char eepromDATA[64];
	// read the hardware device 
	HW_EEPROM eeprom(true);

	status = eeprom.Read(TdBrd, fileOp, eepromID, eepromDATA);
	if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
		return status;
	// if caller provides buffer, return values
	if(ProdID != NULL)
		eeprom.GetProdID(ProdID);
	if (ProdSN != NULL)
		eeprom.GetProdSN(ProdSN);

	return status;
}

THORDAQ_STATUS CdFLIM_4002::APIWriteProdSN(
	CdFLIM_4002& TdBrd,
	UINT32 fileOp,                 // if 0, no file operation, 1 record update to file
	char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
	char ProdID[10],   // TIS Production system fields
	char ProdSN[6])
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;

	return status;
}


// API to read any I2C supported EEPROM device on mainboard and any component connected via I2C bus (includes FMC126 card which replaces ADC on ThorDAQ)
THORDAQ_STATUS CdFLIM_4002::APIReadEEPROM(
	CdFLIM_4002& TdBrd,
	UINT32 fileOp,                 // if 0, no file operation
	char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
	char eepromDATA[64]) // hardware definition - Thorlabs never uses more than 64 EEPROM bytes in ANY device
{
	THORDAQ_STATUS status;
	HW_EEPROM eeprom(false);

	status = eeprom.Read(TdBrd, fileOp, eepromID, eepromDATA);
	if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
		return status;


	return status;
}

// API to write EEPROM
THORDAQ_STATUS CdFLIM_4002::APIWriteEEPROM(
	CdFLIM_4002& TdBrd,
	UINT32 fileOp,                 // if 0, no file operation
	char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
	char eepromDATA[64]) // hardware definition - Thorlabs never uses more than 64 EEPROM bytes in ANY device
{
	THORDAQ_STATUS status;
	HW_EEPROM eeprom(false);
	
	status = eeprom.Write(TdBrd, fileOp, eepromID, eepromDATA);

	return status;
}


// Get EEPROM label by index - used to enumerate all possible EEPROMs hardware platform can support
// could include mezz. card which is not installed; Read determines what is present/readable

THORDAQ_STATUS CdFLIM_4002::APIGetEEPROMlabel(
	CdFLIM_4002& TdBrd,
	UINT32 eepromIndex, // 0-based; 0 is TD board itself; const struct defines end of indexes
	char eepromID[4])  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
{
	THORDAQ_STATUS status;

	HW_EEPROM eeprom(false);

	status = eeprom.GetLabelByIndex(eepromIndex, eepromID);

	return status;
}

