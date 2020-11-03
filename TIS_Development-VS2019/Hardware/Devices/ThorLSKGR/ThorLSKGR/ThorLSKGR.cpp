#include "stdafx.h"
#include "ThorLSKGR.h"
#include "ThorLSKGRXML.h"
#include "ParamInfo.h"

#ifdef LOGGING_ENABLED
std::shared_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

#define HOST_ID 0x01
#define MOTHERBOARD_ID 0x50
#define NUM_TWOWAY_ZONES 251

ThorLSKGR::ThorLSKGR() :
	_deviceDetected(false),
	_rsInitMode(0),
	_twoWayZones(new long[NUM_TWOWAY_ZONES]),
	_firmwareVersion(L"")
{
	_errMsg[0] = NULL;
}


ThorLSKGR::~ThorLSKGR(void)
{
	_instanceFlag = false;
}

bool ThorLSKGR::_instanceFlag = false;

std::shared_ptr<ThorLSKGR> ThorLSKGR::_single (new ThorLSKGR());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorLSKGR*.</returns>
ThorLSKGR* ThorLSKGR::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorLSKGR());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="deviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorLSKGR::FindDevices(long &deviceCount)
{
	long ret = TRUE;

	if(_deviceDetected)
	{
		_serialPort.Close();
		_deviceDetected = FALSE;
	}

	long portID = 0;
	long baudRate = 0;
	try
	{
		std::unique_ptr<ThorLSKGRXML> pSetup(new ThorLSKGRXML());

		pSetup->GetConnection(portID,baudRate);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorLSKGRSettings.xml file");
		LogMessage(_errMsg);
		deviceCount = 0;
		return FALSE;
	}

	if(FALSE == _serialPort.Open(portID,baudRate))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorLSKGR Unable to open com port %d",portID);
		LogMessage(_errMsg);
		deviceCount = 0;
		ret = FALSE;
	}
	deviceCount = 1;
	_deviceDetected = TRUE;
	return ret;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorLSKGR::SelectDevice(const long device)
{

	if(FALSE == _deviceDetected)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"No devices found");
		return FALSE;
	}	

	long portID=0;
	// resonance scanner initial mode

	long initMode = 0;
	_rsInitMode=0;
	try
	{
		std::unique_ptr<ThorLSKGRXML> pSetup(new ThorLSKGRXML());

		pSetup->GetConfiguration(initMode);	
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorLSKGRSettings.xml file");
		return FALSE;
	}

	GetFirmwareVersionAndSerialNumber(_firmwareVersion,_serialNumber);

	CreateParamTable();	//build table for parameters
	memset(_twoWayZones, 0, NUM_TWOWAY_ZONES * sizeof(long));
	CoarseAlignDataLoadFile(); //load data for two way coarse alignment if it exists

	if(1 == initMode)
	{		
		SetParam(IDevice::PARAM_SCANNER_INIT_MODE, 1);

		//turn the scanner on
		SetParam(IDevice::PARAM_SCANNER_ENABLE, 1);
		SetupPosition();
		StartPosition();
	}
	else if ( 0 == initMode)
	{
		SetParam(IDevice::PARAM_SCANNER_INIT_MODE, 0);

		//turn the scanner on
		SetParam(IDevice::PARAM_SCANNER_ENABLE, 0);
		SetupPosition();
		StartPosition();				
	}

	return TRUE;
}

/// <summary>
/// Gets the parameter information.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="paramType">Type of the parameter.</param>
/// <param name="paramAvailable">The parameter available.</param>
/// <param name="paramReadOnly">The parameter read only.</param>
/// <param name="paramMin">The parameter minimum.</param>
/// <param name="paramMax">The parameter maximum.</param>
/// <param name="paramDefault">The parameter default.</param>
/// <returns>long.</returns>
long ThorLSKGR::GetParamInfo
	(
	const long	paramID,
	long		&paramType,
	long		&paramAvailable,
	long		&paramReadOnly,
	double		&paramMin,
	double		&paramMax,
	double		&paramDefault
	)
{
	long parameterID = paramID;

	// update the _twoWayZones array and select the paramID to set the alignment

	if((paramID >= IDevice::PARAM_ECU_TWO_WAY_ZONE_1) && (paramID <= IDevice::PARAM_ECU_TWO_WAY_ZONE_251))
	{
		parameterID = IDevice::PARAM_SCANNER_ALIGN_POS;
	}
	if (NULL != _tableParams[parameterID])
	{
		if(_tableParams[parameterID]->GetParamID() == parameterID)
		{
			paramType = _tableParams[parameterID]->GetParamType();
			paramAvailable = _tableParams[parameterID]->GetParamAvailable();
			paramReadOnly = _tableParams[parameterID]->GetParamReadOnly();
			//if if the parameter is scanner zoom then convert to actual GUI number
			if (PARAM_SCANNER_ZOOM_POS == parameterID)
			{
				paramMin = _tableParams[parameterID]->GetParamMin() / 4;
				paramMax = _tableParams[parameterID]->GetParamMax() / 4;
				paramDefault = _tableParams[parameterID]->GetParamDefault() / 4;
			}
			else
			{
				paramMin = _tableParams[parameterID]->GetParamMin();
				paramMax = _tableParams[parameterID]->GetParamMax();
				paramDefault = _tableParams[parameterID]->GetParamDefault();
			}
			return TRUE;
		}
	}

	paramAvailable = FALSE;
	return FALSE;
}

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorLSKGR::SetParam(const long paramID, const double param)
{
	double paramTemp = param;
	long parameterID = paramID;

	// update the _twoWayZones array and select the paramID to set the alignment
	if(((paramID % IDevice::PARAM_LAST_PARAM) >= IDevice::PARAM_ECU_TWO_WAY_ZONE_1) && ((paramID % IDevice::PARAM_LAST_PARAM) <= IDevice::PARAM_ECU_TWO_WAY_ZONE_251))
	{
		parameterID = IDevice::PARAM_SCANNER_ALIGN_POS;
	}

	if (NULL != _tableParams[parameterID])
	{
		if(_tableParams[parameterID]->GetParamID() == parameterID)
		{
			if((_tableParams[parameterID]->GetParamAvailable() == FALSE) || (_tableParams[parameterID]->GetParamReadOnly() == TRUE))
			{
				return FALSE;
			}
			else if(PARAM_SCANNER_ZOOM_POS == parameterID)
			{
				paramTemp = paramTemp * 4; //conversion for zoom/field size from 256 to 1024
			}

			if((_tableParams[parameterID]->GetParamMin() <= paramTemp) && (_tableParams[parameterID]->GetParamMax() >= paramTemp))
			{
				_tableParams[parameterID]->SetParamVal(static_cast<long>(paramTemp));

				// update the _twoWayZones array
				if(IDevice::PARAM_SCANNER_ALIGN_POS == parameterID)
				{
					if(paramID <= IDevice::PARAM_LAST_PARAM)
					{
						_twoWayZones[paramID - IDevice::PARAM_ECU_TWO_WAY_ZONE_1] = static_cast<long>(param);
					}
				}

				// update scanner enable
				if((IDevice::PARAM_SCANNER_ENABLE == parameterID) || (IDevice::PARAM_SCANNER_ENABLE_ANALOG == parameterID))
				{
					_tableParams[parameterID]->SetParamBool(TRUE);
				}

				return TRUE;
			}

			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorLSKGR SetParam failed. paramID: %d", paramID);
			LogMessage(_errMsg);

			return FALSE;
		}
	}

	return FALSE;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorLSKGR::GetParam(const long paramID, double &param)
{
	// update the _twoWayZones array and select the paramID to set the alignment
	if((paramID >= IDevice::PARAM_ECU_TWO_WAY_ZONE_1) && (paramID <= IDevice::PARAM_ECU_TWO_WAY_ZONE_251))
	{
		param = _twoWayZones[paramID - IDevice::PARAM_ECU_TWO_WAY_ZONE_1];
		return TRUE;
	}

	if (NULL != _tableParams[paramID])
	{
		if(PARAM_SCANNER_ZOOM_POS == paramID)
		{
			param = static_cast<double>(_tableParams[paramID]->GetParamVal()) / 4;
		}
		else
		{
			param = static_cast<double>(_tableParams[paramID]->GetParamVal());
		}

		return TRUE;
	}

	return FALSE;
}

/// <summary>
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorLSKGR::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	return ret;
}

/// <summary>
/// Gets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorLSKGR::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	return ret;
}

/// <summary>
/// Sets the parameter string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <returns>long.</returns>
long ThorLSKGR::SetParamString(const long paramID, wchar_t* str)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Gets the parameter of type string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorLSKGR::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;
	switch(paramID)
	{
	case PARAM_CONTROL_UNIT_FIRMWAREVERSION:
		{
			wcscpy_s(str,size, _firmwareVersion.c_str());
			ret = TRUE;
		}
		break;
	case PARAM_CONTROL_UNIT_SERIALNUMBER:
		{
			wcscpy_s(str,size, _serialNumber.c_str());
			ret = TRUE;
		}
	}
	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorLSKGR::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorLSKGR::SetupPosition()
{	
	for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if((FALSE == (iter->second)->GetParamAvailable()) || (TRUE == (iter->second)->GetParamReadOnly()))
		{
			continue;
		}
		else if((iter->second)->GetParamVal() != (iter->second)->GetParamCurrent())
		{
			(iter->second)->SetParamBool(TRUE);
		}
		else
		{
			(iter->second)->SetParamBool(FALSE);
		}
	}

	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorLSKGR::StartPosition()
{
	long ret = FALSE;

	//iterate through map and set the parameters in reverse order because we want the position to be set at last
	for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if((FALSE == (iter->second)->GetParamAvailable()) || (TRUE == (iter->second)->GetParamReadOnly()))
		{
			continue;
		}
		else if(TRUE == (iter->second)->GetParamBool()) 
		{	
			ExecuteCmd(iter->second); //no need to parse read back

			(iter->second)->UpdateParam_C();

			ret = TRUE;
			(iter->second)->SetParamBool(FALSE);

			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorLSKGR StartPosition succeeded at paramID: %d",(iter->second)->GetParamID());
			LogMessage(_errMsg);
		}
	}

	return ret;
}

/// <summary>
/// Status of the positioning.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorLSKGR::StatusPosition(long &status)
{
	status = STATUS_READY;
	return TRUE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorLSKGR::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorLSKGR::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}

/// <summary>
/// DEPRECATED
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorLSKGR::ReadPosition(DeviceType deviceType,double &pos)
{
	return FALSE;
}

/// <summary>
/// Executes the command.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorLSKGR::ExecuteCmd(ParamInfo* pParamInfo)
{
	//when the scanner init mode is enabled block commands trying to 
	//change the state of the scanner
	double val = 0;
	
	GetParam(IDevice::PARAM_SCANNER_INIT_MODE, val);

	if((pParamInfo->GetParamID()==PARAM_SCANNER_ENABLE) && (static_cast<long>(pParamInfo->GetParamVal()) == 0) && (1 == static_cast<long>(val)))
	{		
		return TRUE;
	}

	long paramID = pParamInfo->GetParamID();
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();	
	std::vector<unsigned char> paramValBytes;
	switch (paramID)
	{
	case PARAM_SCANNER_ENABLE:
		//if the command is to enable (instead of disable) the scanner
		//Then it must be enabled by using the current angle
		//The scanner is disabled when the angle (Zoom pos) is 0
		//If the command is to disable the scanner, then nothing needs to happen;
		//the default command for this parameter is to disable the scanner
		if (TRUE == pParamInfo->GetParamVal())
		{
			if (NULL != _tableParams[PARAM_SCANNER_ZOOM_POS])
			{			
				paramValBytes = GetBytes(_tableParams[PARAM_SCANNER_ZOOM_POS]->GetParamVal());
				cmd[6] =  paramValBytes[0];
				cmd[7] =  paramValBytes[1];
			}
		}
		break;
	case PARAM_SCANNER_ZOOM_POS:
		//Only update the angle when the scanner is running
		if (NULL != _tableParams[PARAM_SCANNER_ENABLE])
		{
			if (TRUE == _tableParams[PARAM_SCANNER_ENABLE]->GetParamCurrent())
			{
				paramValBytes = GetBytes(pParamInfo->GetParamVal());
				cmd[6] =  paramValBytes[0];
				cmd[7] =  paramValBytes[1];
			}
		}
		break;
	case PARAM_SCANNER_ALIGN_POS:
		paramValBytes = GetBytes(pParamInfo->GetParamVal());
		cmd[3] =  paramValBytes[0];
		break;
	case PARAM_SCANNER_ENABLE_ANALOG:
		cmd[3] = (pParamInfo->GetParamVal()) ? 0x1 : 0x2;
		break;
	default:
		break;
	}
	double readBackVal = -1;
	ExecuteCmd(cmd, readBackVal);
	return TRUE;
}

/// <summary>
/// Executes the command.
/// </summary>
/// <param name="cmd">The command.</param>
/// <param name="readBackValue">The read back value.</param>
/// <returns>long.</returns>
long ThorLSKGR::ExecuteCmd(std::vector<unsigned char> cmd, double &readBackValue)
{
	Lock lock(_critSect);

	unsigned char* sendBuf;
	sendBuf = cmd.data();
	std::string cmdStr(cmd.begin(),cmd.end());
	size_t len = cmd.size();//cmdStr.copy(buf, cmdStr.length(), 0);
	_serialPort.SendData((const unsigned char*)(sendBuf), static_cast<int>(len));

	Sleep(30); //Determined by testing... Shorter sleep time may cause bad communications

	char readBuf[100];
	memset(readBuf, 0, sizeof(readBuf));
	_serialPort.ReadData(readBuf, 100);
	readBackValue = TRUE;
	return TRUE;
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorLSKGR::TeardownDevice()
{
	if (_tableParams.size() > 0)
	{

		double val = 0;

		GetParam(IDevice::PARAM_SCANNER_INIT_MODE, val);

		if(1 == static_cast<long>(val))
		{
			//Stop the resonance scanner if it is running
			SetParam(IDevice::PARAM_SCANNER_INIT_MODE, 0);
			SetParam(IDevice::PARAM_SCANNER_ENABLE, 1);
			SetParam(IDevice::PARAM_SCANNER_ENABLE, 0);		
		}

		//Disable the PMTs
		SetParam(IDevice::PARAM_PMT1_GAIN_POS, 0);
		SetParam(IDevice::PARAM_PMT2_GAIN_POS, 0);
		SetParam(IDevice::PARAM_PMT3_GAIN_POS, 0);
		SetParam(IDevice::PARAM_PMT4_GAIN_POS, 0);

		SetupPosition();
		StartPosition();
	}

	_serialPort.Close();
	DestroyParamTable();
	_deviceDetected = false;

	return true;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorLSKGR::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Gets the bytes.
/// </summary>
/// <param name="value">The value.</param>
/// <returns>std.vector&lt;unsigned char&gt;.</returns>
std::vector<unsigned char> ThorLSKGR::GetBytes(int value)
{
	std::vector<unsigned char> bytes(sizeof(int));
	std::memcpy(&bytes[0], &value, sizeof(int));
	return bytes;
}

/// <summary>
/// Loades in the coarse alignment file.
/// </summary>
/// <returns>long.</returns>
long ThorLSKGR::CoarseAlignDataLoadFile()
{
	char appPath[256]; ///<the char of application path

	char filePath[256]; ///<char of the whole file path
	_getcwd(appPath, 256);
	strcpy_s(filePath, appPath);
	strcat_s(filePath, "\\AlignDataCoarse.txt");

	FILE *AlignDataFile;
	if (fopen_s(&AlignDataFile, filePath, "r") != 0)
	{
	}
	else
	{
		const long MIN_COARSE_ALIGNMENT = 0;
		const long MAX_COARSE_ALIGNMENT = 255;
		for (int i = 0; i < 256; i++)
		{
			long lVal = 0;
			if (EOF == fscanf_s(AlignDataFile, "%d", &lVal))
			{
			}
			else if (lVal < MIN_COARSE_ALIGNMENT)
			{
				lVal = MIN_COARSE_ALIGNMENT;
			}
			else if (lVal > MAX_COARSE_ALIGNMENT)
			{
				lVal = MAX_COARSE_ALIGNMENT;
			}
			if(5 <= i)
			{
				_twoWayZones[255-i] = lVal;
			}
		}
		fclose(AlignDataFile);
	}
	return TRUE;
}

/// <summary>
/// Gets the firmware version from the device
/// </summary>
/// <param name="version">The firmware version.</param>
long ThorLSKGR::GetFirmwareVersionAndSerialNumber(std::wstring &version, std::wstring &serialNum)
{
	const size_t LEN = 6;//cmdStr.copy(buf, cmdStr.length(), 0);
	unsigned char commandBytesZoomPos[LEN] = { 0x05, 0x00, 0x02, 0x00, MOTHERBOARD_ID, HOST_ID}; //get board info from processor firmware

	_serialPort.SendData(commandBytesZoomPos, static_cast<int>(LEN));
	Sleep(50);
	char readBuf[100];
	memset(readBuf, 0, sizeof(readBuf));
	_serialPort.ReadData(readBuf, 100);

	long v1 = readBuf[22];
	long v2 = readBuf[21];
	long v3 = readBuf[20];

	version = std::to_wstring(v1);
	version +='.';
	version += std::to_wstring(v2);
	version +='.';
	version += std::to_wstring(v3);

	wchar_t wstr[2];
	size_t returnVal=0;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[24], 1);
	serialNum = wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[25], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[26], 1);
	serialNum += wstr;	
	mbstowcs_s(&returnVal, wstr,2, &readBuf[27], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[28], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[29], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[30], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[31], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[32], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[33], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[34], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[35], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[36], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[37], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[38], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[39], 1);
	serialNum += wstr;
	mbstowcs_s(&returnVal, wstr,2, &readBuf[40], 1);
	serialNum += wstr;

	return true;
}


/// <summary>
/// Creates the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorLSKGR::CreateParamTable()
{
	DestroyParamTable();
	//Execution in order of top to bottom of list:
	std::vector<unsigned char> commandBytes;
	commandBytes.push_back(0x00);
	ParamInfo* tempParamInfo = new ParamInfo(
		PARAM_DEVICE_TYPE,							//ID
		CONTROL_UNIT,								//VAL
		CONTROL_UNIT,								//PARAM C
		FALSE,										//PARAM B
		TYPE_LONG,									//TYPE
		TRUE,										//AVAILABLE
		TRUE,										//READ ONLY
		CONTROL_UNIT,								//MIN
		CONTROL_UNIT,								//MAX
		CONTROL_UNIT,								//DEFAULT
		commandBytes								//Command
		);
	_tableParams.insert(std::pair<long, ParamInfo*>(Params::PARAM_DEVICE_TYPE, tempParamInfo));

	//the default command for this parameter is to disable the scanner
	unsigned char commandBytesEnable[] = { 0x59, 0x08, 0x02, 0x00, (MOTHERBOARD_ID | 0x80), HOST_ID, 0, 0}; //MGMSG_SET_RES_SCAN_ZOOM
	commandBytes.assign(commandBytesEnable, commandBytesEnable + sizeof(commandBytesEnable)/sizeof(commandBytesEnable[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_SCANNER_ENABLE,
		_rsInitMode,
		_rsInitMode,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_SCANNER_ENABLE, tempParamInfo));

	unsigned char commandBytesZoomPos[] = { 0x59, 0x08, 0x02, 0x00, (MOTHERBOARD_ID | 0x80), HOST_ID, 0, 0}; //MGMSG_SET_RES_SCAN_ZOOM
	commandBytes.assign(commandBytesZoomPos, commandBytesZoomPos + sizeof(commandBytesZoomPos)/sizeof(commandBytesZoomPos[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_SCANNER_ZOOM_POS,
		SCANNER_ZOOM_DEFAULT * 4,
		SCANNER_ZOOM_DEFAULT * 4,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		SCANNER_ZOOM_MIN * 4,
		SCANNER_ZOOM_MAX * 4,
		SCANNER_ZOOM_DEFAULT * 4,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_SCANNER_ZOOM_POS, tempParamInfo));

	commandBytes.clear();
	tempParamInfo = new ParamInfo(	
		PARAM_SCANNER_INIT_MODE,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_SCANNER_INIT_MODE, tempParamInfo));

	unsigned char commandBytesOffset[] = { 0x5A, 0x08, 0x00, 0x00, MOTHERBOARD_ID, HOST_ID}; //MGMSG_SET_RES_SCAN_OFFSET
	commandBytes.assign(commandBytesOffset, commandBytesOffset + sizeof(commandBytesOffset)/sizeof(commandBytesOffset[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_SCANNER_ALIGN_POS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		255,
		128,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_SCANNER_ALIGN_POS, tempParamInfo));

	//if this function is called it means the connection has been stablished
	commandBytes.clear();
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(	
		PARAM_CONNECTION_STATUS,
		(long)ConnectionStatusType::CONNECTION_READY,
		(long)ConnectionStatusType::CONNECTION_READY,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		(long)ConnectionStatusType::CONNECTION_WARMING_UP,
		(long)ConnectionStatusType::CONNECTION_ERROR_STATE,
		(long)ConnectionStatusType::CONNECTION_UNAVAILABLE,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTION_STATUS, tempParamInfo));

	unsigned char commandFirmwareUpdate[] = { 0xA6, 0x00, 0x04, 0x00, MOTHERBOARD_ID, HOST_ID, 0x00, 0x00, 0x00, 0x00}; //MGMSG_SET_RES_SCAN_OFFSET
	commandBytes.assign(commandFirmwareUpdate, commandFirmwareUpdate + sizeof(commandFirmwareUpdate)/sizeof(commandFirmwareUpdate[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_CONTROL_UNIT_FIRMWAREUPDATE,
		FALSE,
		FALSE,
		TRUE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONTROL_UNIT_FIRMWAREUPDATE, tempParamInfo));

	//if this function is called it means the connection has been stablished
	commandBytes.clear();
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(	
		PARAM_CONTROL_UNIT_FIRMWAREVERSION,
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONTROL_UNIT_FIRMWAREVERSION, tempParamInfo));

	commandBytes.clear();
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(	
		PARAM_CONTROL_UNIT_SERIALNUMBER,
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONTROL_UNIT_SERIALNUMBER, tempParamInfo));

	unsigned char commandBytesAnalogEnable[] = { 0x10, 0x02, 0x01, 0x02, MOTHERBOARD_ID, HOST_ID}; //MGMSG_MOD_SET_CHANENABLESTATE
	commandBytes.assign(commandBytesAnalogEnable, commandBytesAnalogEnable + sizeof(commandBytesAnalogEnable)/sizeof(commandBytesAnalogEnable[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_SCANNER_ENABLE_ANALOG,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_SCANNER_ENABLE_ANALOG, tempParamInfo));

	return TRUE;
}

/// <summary>
/// Destroys the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorLSKGR::DestroyParamTable()
{
	try
	{
		for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
		{
			delete iter->second;
		}
		_tableParams.clear();
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorLSKGR DestroyParamTable unable to destroy the table created on heap");
		LogMessage(_errMsg);
		return FALSE;
	}

	return TRUE;
}