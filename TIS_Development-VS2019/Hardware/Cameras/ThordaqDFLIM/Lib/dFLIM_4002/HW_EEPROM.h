#pragma once
#include "dFLIM_4002.h"
#include <algorithm>

// class which implements all the ThorDAQ board's hardware accessible EEPROM devices, especially via I2C bus
// for dFLIM board; this means the Legacy BreakOut Boxes

// global constants
//static const char* EEPROMfilename = "c:\\temp\\DFLIMeeproms.txt";

static const int NumEEPROMsDecoded = 8;  // the max possible num of supported EEPROMs for this hardware
static const UINT LabelDepth = 4;

// This function aids in clarity of the text file for manual inspection and editing -
	// This call must be made before writing to file (to add hypens) and reading file (to remove hypens)
	// current definition for Thorlabs EEPROM, 64 chars
	// The two EEPROM formats are:  A. Deprecated format of Austin, B. New TIS production format, which is:
	// First 3 chars:  MANDATED: hardware ID of component, e.g. TD3, AB1, DAC, FMC, DCL, etc.
	// Next 25 chars:  optional deprecated Austin format (maintained for legacy data)
	// Next 16 chars:  MANDATED new TIS production number (10 chars) plus serial num (6 chars)
	// Next 20 chars:  optional remarks field
	// AFTER hypens are added, files appear as below
/*
		TD_ProdSN.txt
	TD   TD3-TD30398960-002095
		DFLIMeeproms.txt
	TD   TD3-02095g39896000506/27/2019-TD30398960-002095- AUS 2020 test board
Empty file:
TDn                              -          -      -Comment       NoRead
ADC                              -          -      -Read-only nonThorDAQ
DBB1                             -          -      -Comment       NoRead
DAC                              -          -      -Comment       NoRead
ABB1                             -          -      -Comment       NoRead
ABB2                             -          -      -Comment       NoRead
ABB3                             -          -      -Comment       NoRead
LFT                              -          -      -Comment       NoRead
*/


class TD_EEPROMfile // define the text file which describes the hardware capability
{
private:
	bool bProductionFileFORMAT;
	char EEPROMfilename[132]; 

public:
	static const int EEPROMlineLen = 64; // 64 max bytes for any EEPROM
	static const int EEPROM_FILE_lineLen = EEPROMlineLen + 5 + 4;  // include 5 char hardware prefix field, 4 hyphens:  MUST BE CONSTANT to match legacy PRODUCTION ManufTest fomat
	char FormattedPROMs[NumEEPROMsDecoded][EEPROM_FILE_lineLen];
protected:
	// Legacy field [0-27] ProdID [28-37] ProdSN[38-43]  Comment[44-63] 

	char LegacySN[28 + 1];  // these fields also make ProdSN API easier, as much of the legacy EEPROM data is ignored
	char ProdID[10 + 1];
	char ProdSN[6 + 1];
	char Comment[20 + 1];
public:
	TD_EEPROMfile(bool bProductionFileFormat) // constructor - if true, filename/format is abbreviate TIS production format, else 64-byte Full format
	{
		bProductionFileFORMAT = bProductionFileFormat;
		if (bProductionFileFORMAT)
		{
			strcpy_s(EEPROMfilename, sizeof("c:\\temp\\DF_ProdSN.txt"),"c:\\temp\\DF_ProdSN.txt" );
		}
		else 
		{
			strcpy_s(EEPROMfilename, sizeof("c:\\temp\\DFLIMeeproms.txt"), "c:\\temp\\DFLIMeeproms.txt");
		}

		const char* cDefault = "Comment       NoRead";
		const char* cFMCcard = "Read-only nonThorDAQ";
		// NOTE!  These hardware I2C map descriptors match ThorDAQ hardware addressing to Mezzanine cards, etc, and must be compatible with "ManufTest" production utility.
		// i.e. The FMC126 is a variant of ADC, and the DCL (digital clock) is variable of original LFT "3P" card (Low Frequency Trigger)
		snprintf(FormattedPROMs[0], EEPROM_FILE_lineLen, "%s%s", "TDn                              -          -      -", cDefault);  // max 64 bytes defined in ThorDAQ eeproms
		snprintf(FormattedPROMs[1], EEPROM_FILE_lineLen, "%s%s", "ADC                              -          -      -", cFMCcard);  // FMC, 32 byte read-only non-Thorlabs field
		snprintf(FormattedPROMs[2], EEPROM_FILE_lineLen, "%s%s", "DBB1                             -          -      -", cDefault);  // max 64 bytes defined in ThorDAQ eeproms
		snprintf(FormattedPROMs[3], EEPROM_FILE_lineLen, "%s%s", "DAC                              -          -      -", cDefault);  // max 64 bytes defined in ThorDAQ eeproms
		snprintf(FormattedPROMs[4], EEPROM_FILE_lineLen, "%s%s", "ABB1                             -          -      -", cDefault);  // max 64 bytes defined in ThorDAQ eeproms
		snprintf(FormattedPROMs[5], EEPROM_FILE_lineLen, "%s%s", "ABB2                             -          -      -", cDefault);  // max 64 bytes defined in ThorDAQ eeproms
		snprintf(FormattedPROMs[6], EEPROM_FILE_lineLen, "%s%s", "ABB3                             -          -      -", cDefault);  // max 64 bytes defined in ThorDAQ eeproms
		snprintf(FormattedPROMs[7], EEPROM_FILE_lineLen, "%s%s", "LFT                              -          -      -", cDefault);  // max 64 bytes defined in ThorDAQ eeproms
		// stop warnings
		strncpy_s(LegacySN, " ", 1); 
		strncpy_s(ProdID, " ", 1); 
		strncpy_s(ProdSN, " ", 1);
		strncpy_s(Comment, " ", 1);

	}

	// update an EEPROM record in the fixed-purpose data text file
	THORDAQ_STATUS WriteEEPROMfile(char eepromID[4], UINT32 startByte, UINT32 Len, char eepromDATA[64])
	{
		try
		{                                         // The EEPROM contents begins at Column 5 (columns 0-4 for the static label corresponding to hardware)                     
			ofstream EEpromFile(EEPROMfilename);  // 5 spaces per field: "TD   ", "DAC  ", "FMC  ", "DCL  ", "AB1 ", "AB2 ", "AB3 ","DB1 "
			if (!EEpromFile.is_open())
			{
				return THORDAQ_STATUS::STATUS_WRITE_BUFFER_ERROR;
			}

			int ZeroBasedLine;
			// while line needs updating?
			if (strncmp("TD", eepromID, 2) == 0) ZeroBasedLine = 0;
			else if (strncmp("DAC", eepromID, 3) == 0) ZeroBasedLine = 1;
			else if (strncmp("FNC", eepromID, 3) == 0) ZeroBasedLine = 2;
			else if (strncmp("DCL", eepromID, 3) == 0) ZeroBasedLine = 3;
			else if (strncmp("AB1", eepromID, 3) == 0) ZeroBasedLine = 4;
			else if (strncmp("AB2", eepromID, 3) == 0) ZeroBasedLine = 5;
			else if (strncmp("AB3", eepromID, 3) == 0) ZeroBasedLine = 6;
			else if (strncmp("DB1", eepromID, 3) == 0) ZeroBasedLine = 7;
			else
			{
				//ERROR - bad index
				return THORDAQ_STATUS::STATUS_WRITE_BUFFER_ERROR;
			}

			EEpromFile.close();
		}
		catch (exception e)
		{
			return THORDAQ_STATUS::STATUS_WRITE_BUFFER_ERROR; // cannot update the file - write protected or owned by other process?
		}
		return THORDAQ_STATUS::STATUS_SUCCESSFUL;
	}


	// NOTE!  We WRITE to file only when we have READ from EEPROM
	//        We READ from file only when trying to WRITE to EEPROM
	// A 5-char field precedes each EEPROM data line
	THORDAQ_STATUS RegenerateEPromFile() // creates empty template
	{
		try
		{
			ofstream EEpromFile(EEPROMfilename);  // 5 spaces per field: "TD   ", "FMC  ", ...

			for (int fileLine = 0; fileLine < NumEEPROMsDecoded; fileLine++)
			{
				EEpromFile << FormattedPROMs[fileLine] << endl;  // 0
			}
			EEpromFile.close();
		}
		catch (exception e)
		{
			return THORDAQ_STATUS::STATUS_READ_BUFFER_ERROR; // cannot create the file - write protected or owned by other process?
		}
		return THORDAQ_STATUS::STATUS_SUCCESSFUL;
	}

	// The hardware is capable of all manner of non-printable, trouble causing 8-bit values
	void ReplaceNonPrintableChars(char EPPROMlineContents[64], char subChar)
	{
		int i;
		for (i = 0; i < 63; i++)
		{
			if( (EPPROMlineContents[i] < 32 || EPPROMlineContents[i] > 127))
			{
				EPPROMlineContents[i] = '?';
			}
		}

	}


	// READ the DISK FILE and load values into private class array
	// we always maintain a complete file set of variables, while I2C Read/Write operations of course are singular to an EEPROM device
	THORDAQ_STATUS ReadEpromFile() // loads values into FormattedPROMs[]
	{
		string sLine;
		try
		{
			ifstream EEpromFile(EEPROMfilename);  // 5 PREFIXED spaces per line (device), e.g.: "TD   ", "DAC  ", "FMC  ", "DCL  ", "ABB1 ", "ABB2 ", "ABB3 ","DBB1 "
			int indx = 0;

			while (getline(EEpromFile, sLine) && indx < NumEEPROMsDecoded)
			{
				snprintf(FormattedPROMs[indx++], EEPROM_FILE_lineLen, "%s", sLine.c_str()); // formatted means having the "-" hyphen separators to make text file easier to hand edit
			};
			EEpromFile.close();
		}
		catch (exception e)
		{
			return THORDAQ_STATUS::STATUS_READ_BUFFER_ERROR; // cannot create the file - write protected or owned by other process?
		}
		return THORDAQ_STATUS::STATUS_SUCCESSFUL;
	}

protected:
	// Load raw EEPROM contents into local Fields
			//            1         2         3         4         5         6
			//  0123456789012345678901234567890123456789012345678901234567890123
			// "TDn                         *         *     *                  ",
	void EEPROMrawContentsToAllFields(int EEPROMindex, char rawEEPROMbuf[64]) // strips non-printable chars, parses & stores the 4 fields from a raw EEPROM 64-byte bugger
	{
		// first, replace non-printable binary values which defeat various functions
			// it is very common to encounter non-printable chars - replace a printable char
		ReplaceNonPrintableChars(rawEEPROMbuf, '?');
		// blank the temporary vars
		memset(LegacySN, ' ', 29);
		memset(ProdID, ' ', 11);
		memset(ProdSN, ' ', 7);
		memset(Comment, ' ', 21);

		// update the affected file line, leaving others intact
		// raw bytes parsed above, then added to the formatted line FormattedPROMs[] as below (note hyphens)
		//            1         2         3         4         5         6
		//  0123456789012345678901234567890123456789012345678901234567890123
		// "TDn                              -          -      -            ",
		if (EEPROMindex == 1)  // exception for FMC126 read-only 32-byte format
		{
			memcpy_s(LegacySN, 3, &rawEEPROMbuf[20], 3); // don't modify field definitions
			memcpy_s(ProdID, 6, &rawEEPROMbuf[20], 6); // don't modify field definitions
			memcpy_s(ProdSN, 4, &rawEEPROMbuf[27], 4); // don't modify field definitions
			 // no "comment field" in FMC126
			memcpy_s(&FormattedPROMs[EEPROMindex][5], 3, LegacySN, 3);
			memcpy_s(&FormattedPROMs[EEPROMindex][34], 6, ProdID, 6);
			memcpy_s(&FormattedPROMs[EEPROMindex][45], 4, ProdSN, 4);
		}
		else
		{
			memcpy_s(LegacySN, 28, &rawEEPROMbuf[0], 28); // don't modify field definitions
			memcpy_s(ProdID, 10, &rawEEPROMbuf[28], 10); // don't modify field definitions
			memcpy_s(ProdSN, 6, &rawEEPROMbuf[38], 6); // don't modify field definitions
			memcpy_s(Comment, 20, &rawEEPROMbuf[44], 20); // don't modify field definitions
			memcpy_s(&FormattedPROMs[EEPROMindex][52], 20, Comment, 20);
			memcpy_s(&FormattedPROMs[EEPROMindex][5], 28, LegacySN, 28);
			memcpy_s(&FormattedPROMs[EEPROMindex][34], 10, ProdID, 10);
			memcpy_s(&FormattedPROMs[EEPROMindex][45], 6, ProdSN, 6);
		}
	}

	// This function is useful for reading the disk file, stripping hypens, and loading private vars
	void EEPROMFieldsStrippedOfHyphens(int LineIndx)
	{
		memcpy_s(Comment, 20, &FormattedPROMs[LineIndx][52], 20);
		memcpy_s(LegacySN, 28, &FormattedPROMs[LineIndx][5], 28);
		memcpy_s(ProdID, 10, &FormattedPROMs[LineIndx][34], 10);
		memcpy_s(ProdSN, 6, &FormattedPROMs[LineIndx][45], 6);
	}

	// NOTE!  Must only be called for defined labels
	bool GetLineIndexForLabel(const char Label[4], int *Indx)
	{
		int LineIndx = 0;
		// what line variable is the label?    "TD?", "FMC", "DB1", "DAC", "AB1", "AB2", "AB3", "DCL"
		if (strncmp(Label, "TD", 2) == 0) // Main board
		{
			LineIndx = 0;
		}
		else if (strncmp(Label, "FMC", 3) == 0)
		{
			LineIndx = 1;
		}
		else if (strncmp(Label, "DB1", 3) == 0)
		{
			LineIndx = 2;
		}
		else if (strncmp(Label, "DAC", 3) == 0)
		{
			LineIndx = 3;
		}
		else if (strncmp(Label, "AB1", 3) == 0)
		{
			LineIndx = 4;
		}
		else if (strncmp(Label, "AB2", 3) == 0)
		{
			LineIndx = 5;
		}
		else if (strncmp(Label, "AB3", 3) == 0)
		{
			LineIndx = 6;
		}
		else if (strncmp(Label, "DCL", 3) == 0)
		{
			LineIndx = 7;
		}
		else // FAILED!
		{
			return false;
		}
		*Indx = LineIndx;
		return true;
	}

public:
	// eepromDATABuf are bytes from the I2C hardware read
	THORDAQ_STATUS WriteEpromFile(const char Label[4], UINT32 startByte, UINT32 BufLen, char* eepromDATABuf)
	{
		THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;

		if (!TextFileExists()) 
		{				 // new file will have ONE EEPROM write to it
			status = RegenerateEPromFile(); // builds DEFAULT file
			if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
				return status;
		}
		// file exists - update it
		status = ReadEpromFile(); // loads entire current (formatted -) file into FormattedPROMs[] local class array
		// 

		if (status == THORDAQ_STATUS::STATUS_SUCCESSFUL)
		{
			int LineIndx;
			bool bFound = GetLineIndexForLabel(Label, &LineIndx);
			if( !bFound)
			{
				return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR; // unknown EEPROM label!
			}

			EEPROMrawContentsToAllFields(LineIndx, eepromDATABuf);


			// finally write the updated contents back to file
			status = RegenerateEPromFile(); // updates existing file with refreshed line FormattedPROMs
		}
		return status;
	}

	bool TextFileExists()  // checks if file exists
	{
		struct _stat statBuf;
		if (_stat(EEPROMfilename, &statBuf)) return false;
		return true;
	}
};

class HW_EEPROM : TD_EEPROMfile
{
public: 

	const char* EEPROMdevices[NumEEPROMsDecoded] = { "TD3", "FMC", "DB1", "DAC", "AB1", "AB2", "AB3", "DCL"}; // TD3 required for dFLIM
private:
	// note the I2C map variables for supported devices
	UINT32 MasterMux; 
	UINT32 SlaveMux;
	UINT32 SlaveAddress;


	
public:	
	HW_EEPROM(bool bProductionFileFormat) : TD_EEPROMfile(bProductionFileFormat)    // Constructor
	{
		MasterMux = 0xff; // invalid MUX chan
		SlaveMux = 0xFF;
		SlaveAddress = 0xFF;  
	}
	~HW_EEPROM()    // Destructor
	{

	}
	// copy the member variables from TD_EEPROMfile
	void GetProdID(char prodID[10])
	{
		memcpy(prodID, ProdID, 10);
	}
	void SetProdID(char prodID[10])
	{
		memcpy(ProdID, prodID, 10);
	}

	void GetProdSN(char prodSN[6])
	{
		memcpy(prodSN, ProdSN, 6);
	}
	void SetProdSN(char prodSN[6])
	{
		memcpy(ProdSN, prodSN, 6);
	}

	// returns 3-char EEPROM identifier of 0-based Index of all possible EEPROMs this hardware supports, 3 chars+NULL
	THORDAQ_STATUS GetLabelByIndex(int Index, char* ID)
	{
		if (Index < 0)  return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
		if (Index >= NumEEPROMsDecoded)  return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;

		sprintf_s(ID, 4, "%s", EEPROMdevices[Index]); // sprintf adds NULL
		return THORDAQ_STATUS::STATUS_SUCCESSFUL;
	}

	// if we know how to address this I2C slave EEPROM, we set those map variables here;
	// otherwise tell caller we don't recognize the device (by Label)
	bool Exists(const char *label) // can we access this device?
	{
		MasterMux = 0xff;  // default is Label not found
		bool bRet = false; // default is NOT found
		// The ThorDAQ main board EERPOM must ALWAYS be "present" and working
		if (strncmp(label,"TD", 2) == 0)  // TD1, TD3 ...
		{
			MasterMux = 0x80;
			SlaveMux = 0xFF;
			SlaveAddress = 0x54;
			bRet = true;
		}
		else // boards/systems differ in EEPROMs present
		{
			for (int i = 1; i < NumEEPROMsDecoded; i++) // for all possible EEPROMs
			{
				if (strncmp(label, "DAC", 3) == 0)
				{
					MasterMux = 0x8;
					SlaveMux = 0x4;
					SlaveAddress = 0x54;
					bRet = true;
					break;
				}
				else if (strncmp(label, "FMC", 3) == 0)
				{
					MasterMux = 0x2;
					SlaveMux = 0xff;
					SlaveAddress = 0x51; // note FMC's unique address for EEPROM (not Thorlabs designed)
					bRet = true;
					break;
				}
				else if (strncmp(label, "DCL", 3) == 0) // the "3P" card for digital clock divider 80MHz to 5MHz
				{
					MasterMux = 0x1;
					SlaveMux = 0xff;
					SlaveAddress = 0x50;  // NOTE!  this must change to 0x54 in new PCB to avoid conflict with BOB
					bRet = true;
					break;
				}
				// Only Legacy BreakOut Boxes for dFLIM
				else if (strncmp(label, "AB1", 3) == 0)
				{
					MasterMux = 0x8;
					SlaveMux = 0x1;
					SlaveAddress = 0x50;
					bRet = true;
					break;
				}
				else if (strncmp(label, "AB2", 3) == 0)
				{
					MasterMux = 0x8;
					SlaveMux = 0x2;
					SlaveAddress = 0x50;
					bRet = true;
					break;
				}
				else if (strncmp(label, "AB3", 3) == 0)
				{
					MasterMux = 0x8;
					SlaveMux = 0x8;
					SlaveAddress = 0x50;
					bRet = true;
					break;
				}
				else if (strncmp(label, "DB1", 3) == 0)  // on dFLIM, this comes through 3P mezz, not the FMC ("ADC")
				{
					MasterMux = 0x1;
					SlaveMux = 0x8;
					SlaveAddress = 0x99;  // NOTE!  There is CONFLICT in first Rev of DLC(3Pclock-only) mezz. card; BBox and card both 0x50
					bRet = true;
					break;
				}
			}
		}
		return bRet;
	}

	// READ the EEPROM, copying received bytes into callers buffer

	THORDAQ_STATUS Read(CdFLIM_4002& TdBrd, UINT32 fileOp, const char Label[4], char* eepromDATABuf)
	{
		UINT32 startByte = 0, BufLen = 64;
		THORDAQ_STATUS status;
		bool bI2Cread = true;
		UINT8 OpCodeBuf[8];
		UINT32 OpCodeLen = 1;
		OpCodeBuf[0] = (UINT8)startByte;
		UINT32 BiDirLen = BufLen; // 64 bytes for typical Thorlabs EEPROM, max 32 bytes for FMC126
		
		if( !Exists(Label)) 
			return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
		// if we advance, the correct I2C map must have been assigned by Exists()

		if (MasterMux == 0xff)
			return THORDAQ_STATUS::STATUS_FUNCTION_NOT_SUPPORTED;

		// prevent length error on FMC which has max of 32 bytes to read
		if (MasterMux == 0x2 && SlaveAddress == 0x51)  // only EEPROM with address 51 is FMC
		{
			if (startByte > 31) startByte = 31;
			BiDirLen = (startByte == 0) ? 32 : 32 - startByte;
		}
		status = TdBrd.APIXI2CReadWrite(
			TdBrd,
			bI2Cread,
			0x71,   // hardware Master MUX address
			MasterMux,  // master channel
			0x70,   // slave MUX address
			SlaveMux,   // slave channel
			SlaveAddress,
			100,       // speed kHz (slow)
			16,        // page size
			OpCodeBuf,
			&OpCodeLen,
			(PUINT8)eepromDATABuf,
			&BiDirLen	);

		if (status == THORDAQ_STATUS::STATUS_SUCCESSFUL)
		{
			int EEPROMdevIndex;
			GetLineIndexForLabel(Label, &EEPROMdevIndex);                   // copy the raw data we just read from hardware
			EEPROMrawContentsToAllFields(EEPROMdevIndex, eepromDATABuf);

			// if called for write value we just read to disk file
			if(fileOp > 0)
				status = WriteEpromFile(Label, startByte, BufLen, eepromDATABuf);

		}
		return status;
	}

	private:
		void ReadAllEEPROMSandWriteToFile(CdFLIM_4002& TdBrd)
		{
			char eepromDATA[64]; // hardware definition - Thorlabs never uses more than 64 EEPROM bytes in ANY device

			for (int iDev = 0; iDev < NumEEPROMsDecoded; iDev++)	//EEPROMdevices
			{
				Read(TdBrd, 1, EEPROMdevices[iDev], eepromDATA); // forces update of file
			}
		}

	public:
	// Write EEPROM contents - check for fileOp flag to see if write values should updated in disk file
	// We must ALWAYS READ the current contents of the EEPROMs before any WRITE attempt, in order
	// to avoid possibility of obliterating existing EEPROM contents with empty/initial fields
	// 
	THORDAQ_STATUS Write(CdFLIM_4002& TdBrd, UINT32 fileOp, char Label[4], char* eepromDATABuf)
	{
		THORDAQ_STATUS status;
		UINT32 startByte = 0, BufLen = 64;
		bool bI2Cread = false;  // writing the EEPROM
		UINT8 OpCodeBuf[8];
		UINT32 OpCodeLen;
		OpCodeBuf[0] = startByte; // let other logic handle special case of FMC 
		UINT32 BiDirLen; // 64 bytes for typical Thorlabs EEPROM, max 32 bytes for FMC126

		// check special case fileOp 2, command to get ALL the EEPROM write data from file 
		// so we both Label and eepromDATABuf can be NULL
		if (fileOp != 2)
		{
			if (!Exists(Label)) // sets the I2C map if the EEPROM exists in hardware
				return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
			if (MasterMux == 0xff)
				return THORDAQ_STATUS::STATUS_FUNCTION_NOT_SUPPORTED;
		}
		// if we advance, the correct I2C map must have been assigned by Exists()

		// prevent length error on FMC which has max of 32 bytes to read
		if (MasterMux == 0x2 && SlaveAddress == 0x51)  // only EEPROM with address 51 is FMC
		{
			// CANNOT write to the Read-Only FMC126
			return THORDAQ_STATUS::STATUS_FUNCTION_NOT_SUPPORTED;
		}

		if (fileOp == 2)  // this API calls for our current disk file to be written, in its entirety, to all EEPROMs
		{
			bool bAllEEPROMsWritten = true; // on any error set false
			// FIRST, make sure a file exists!  Don't create blank template file to overwrite existing contents! 
			if( !TextFileExists())
				return THORDAQ_STATUS::STATUS_FUNCTION_NOT_SUPPORTED;
			
			// NOW, write the contents of EXISTING EEPROM .txt file to hardware
			status = ReadEpromFile();
			if( status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
				return THORDAQ_STATUS::STATUS_FUNCTION_NOT_SUPPORTED;

			// we have the data fields loaded - write them to hardware
			for (int iDev = 0; iDev < NumEEPROMsDecoded; iDev++)
			{
				// NOTE the FMC device is READ-ONLY
				if (iDev == 1) continue; // next EEPROM
			
				OpCodeLen = 1; // mandatory EEPROM start index (typically 0)
				BiDirLen = 64; // write entire EEPROM
				OpCodeBuf[0] = startByte;

				if (!Exists(EEPROMdevices[iDev])) // SANITY check the label and setup I2C address map
					return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;

				
				// the "formatted" EEPROM lines (with hypens) are in  FormattedPROMs[iDev] - copy the fields (without hyphens) to private vars 
				EEPROMFieldsStrippedOfHyphens(iDev);

				char eepromRawBuf[65]; // now re-assemble the fields into the EEPROM string, sans '-'
				memset(eepromRawBuf, 0, sizeof(eepromRawBuf));
				strncat_s(eepromRawBuf, LegacySN, 28);
				strncat_s(eepromRawBuf, ProdID, 10);
				strncat_s(eepromRawBuf, ProdSN, 6);
				strncat_s(eepromRawBuf, Comment, 20);

				// write the EEPROM chars for this Device
				status = TdBrd.APIXI2CReadWrite(
					TdBrd,
					bI2Cread, // (write)
					0x71,   // hardware Master MUX address
					MasterMux,  // master channel
					0x70,   // slave MUX address
					SlaveMux,   // slave channel
					SlaveAddress,
					100,       // speed kHz (slow)
					16,        // page size
					OpCodeBuf,
					&OpCodeLen,
					(PUINT8)eepromRawBuf,
					&BiDirLen);
				if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
				{
					bAllEEPROMsWritten = false;
				}
			}
			if(!bAllEEPROMsWritten)
				status = THORDAQ_STATUS::STATUS_WRITE_BUFFER_ERROR; // record any error as API error

		}
		else // write the single EEPROM label in this call
		{
			OpCodeLen = 1; // mandatory EEPROM start index (typically 0)
			BiDirLen = 64; // write entire EEPROM

			status = TdBrd.APIXI2CReadWrite(
				TdBrd,
				bI2Cread, // (write)
				0x71,   // hardware Master MUX address
				MasterMux,  // master channel
				0x70,   // slave MUX address
				SlaveMux,   // slave channel
				SlaveAddress,
				100,       // speed kHz (slow)
				16,        // page size
				OpCodeBuf,
				&OpCodeLen,
				(PUINT8)eepromDATABuf,
				&BiDirLen);

			if (status == THORDAQ_STATUS::STATUS_SUCCESSFUL && fileOp == 1)  // does call want our disk file updated on EEPROM write success? 
			{
				// write value we just WROTE to EEPROM device to the disk file
				status = WriteEpromFile(Label, startByte, BufLen, eepromDATABuf);

			}
		}

		return status;
	}
};


