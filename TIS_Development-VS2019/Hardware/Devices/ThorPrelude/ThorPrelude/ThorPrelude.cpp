#include "ThorPrelude.h"
#include "ThorPreludeXML.h"

#ifdef LOGGING_ENABLED
shared_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

ThorPrelude::ThorPrelude() :
	_deviceDetected(false),
	_twoWayZones(new long[NUM_TWOWAY_ZONES]),
	_firmwareVersion(L"")
{
	_errMsg[0] = NULL;
}


ThorPrelude::~ThorPrelude(void)
{
	_instanceFlag = false;
}

bool ThorPrelude::_instanceFlag = false;

std::shared_ptr<ThorPrelude> ThorPrelude::_single(new ThorPrelude());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorPrelude*.</returns>
ThorPrelude* ThorPrelude::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorPrelude());
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
long ThorPrelude::FindDevices(long& deviceCount)
{
	long ret = TRUE;

	if (_deviceDetected)
	{
		_serialPort.Close();
		_deviceDetected = FALSE;
		_serialNums.clear();
	}

	//Search the Registry for any connected devices associated with this PID and VID.
	_serialNums = FindSerialNumbersInRegistry(THORLABS_VID, THORLABS_PRELUDE_PID);
	if (0 < _serialNums.size())
	{
		_deviceDetected = TRUE;
	}
	else
	{
		_deviceDetected = FALSE;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude: Unable to find any device connected in the registry associated with this PID and VID");
		LogMessage(_errMsg, INFORMATION_EVENT);
		ret = FALSE;
	}
	deviceCount = (int)_serialNums.size();
	return ret;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorPrelude::SelectDevice(const long device)
{
	long portID = 0;
	long settingsPortID = 0;
	long baudRate = 0;
	long count = 0;
	wstring port;
	string serialNum;
	vector<string>::iterator it;

	if (FALSE == _deviceDetected)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude: No devices found");
		LogMessage(_errMsg, INFORMATION_EVENT);
		return FALSE;
	}

	try
	{
		std::unique_ptr<ThorPreludeXML> pSetup(new ThorPreludeXML());

		pSetup->GetConnection(settingsPortID, baudRate);
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Unable to locate ThorPreludeSettings.xml file");
		LogMessage(_errMsg, ERROR_EVENT);
		return FALSE;
	}

	_deviceDetected = FALSE;
	// Iterate through the list of serial numbers until the index matches the value passed
	for (it = _serialNums.begin(); it != _serialNums.end(); ++it)
	{
		if (count == device)
		{
			serialNum = it->data();
			//Search the Registry for the COM port associated with the PID&IVID and the selected device serial number.
			port = FindCOMPortInRegistry(THORLABS_VID, THORLABS_PRELUDE_PID, serialNum);
			// port wstring returned should be more than 3 characters
			if (3 >= port.size())
			{
				wstring messageWstring = L"ThorPrelude: Returned port ID doesn't match required format to continue. Format should be: COM##, format returned is: " + port;
				StringCbPrintfW(_errMsg, MSG_SIZE, messageWstring.c_str());
				LogMessage(_errMsg, ERROR_EVENT);
				return FALSE;
			}
			// Check if the port number is between 16 and 56 which are already used port numbers in ThorImageLS
			wstring portNumber = port.substr(3, port.size());
			portID = stoi(portNumber);
			if (16 < portID && 57 > portID)
			{
				wchar_t message[512];
				StringCbPrintfW(message, 512, L"ThorPrelude: WARNING The devide COM port is unavailable. Port: COM%ld.\nPlease change the COM port number of this device to one that is not between 17 and 56 to avoid any conflicts in ThorImageLS. \nRecommended port: COM%ld.", portID, settingsPortID);
				MessageBox(NULL, message, L"ThorPrelude COM port warning", MB_OK | MB_DEFAULT_DESKTOP_ONLY | MB_ICONWARNING);
			}

			// Open the serial port after getting the number
			if (FALSE == _serialPort.Open(portID, baudRate))
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude: SelectDevice could not open serial port: %d", portID);
				LogMessage(_errMsg, ERROR_EVENT);
				return FALSE;
			}
			else
			{
				_deviceDetected = TRUE;
				//Need to wait some time before requesting the board information. Otherwise firmware returns garbage data.
				Sleep(10);
			}
		}
		count++;
	}

	if (FALSE == _deviceDetected)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude: could not find or connect to a device in SelectDevice");
		LogMessage(_errMsg, ERROR_EVENT);
		return FALSE;
	}

	CreateParamTable();	//build table for parameters
	memset(_twoWayZones, 0, NUM_TWOWAY_ZONES * sizeof(long));
	CoarseAlignDataLoadFile(); //load data for two way coarse alignment if it exists

	GetFirmwareVersionAndSerialNumber(_firmwareVersion, _serialNumber);

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
long ThorPrelude::GetParamInfo
(
	const long	paramID,
	long& paramType,
	long& paramAvailable,
	long& paramReadOnly,
	double& paramMin,
	double& paramMax,
	double& paramDefault
)
{
	long parameterID = paramID;

	// update the _twoWayZones array and select the paramID to set the alignment

	if ((paramID >= IDevice::PARAM_ECU_TWO_WAY_ZONE_1) && (paramID <= IDevice::PARAM_ECU_TWO_WAY_ZONE_251))
	{
		parameterID = IDevice::PARAM_SCANNER_ALIGN_POS;
	}
	if (NULL != _tableParams[parameterID])
	{
		if (_tableParams[parameterID]->GetParamID() == parameterID)
		{
			paramType = _tableParams[parameterID]->GetParamType();
			paramAvailable = _tableParams[parameterID]->GetParamAvailable();
			paramReadOnly = _tableParams[parameterID]->GetParamReadOnly();
			paramMin = _tableParams[parameterID]->GetParamMin();
			paramMax = _tableParams[parameterID]->GetParamMax();
			paramDefault = _tableParams[parameterID]->GetParamDefault();
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
long ThorPrelude::SetParam(const long paramID, const double param)
{
	double paramTemp = param;
	long parameterID = paramID;

	// update the _twoWayZones array and select the paramID to set the alignment
	if (((paramID % IDevice::PARAM_LAST_PARAM) >= IDevice::PARAM_ECU_TWO_WAY_ZONE_1) && ((paramID % IDevice::PARAM_LAST_PARAM) <= IDevice::PARAM_ECU_TWO_WAY_ZONE_251))
	{
		parameterID = IDevice::PARAM_SCANNER_ALIGN_POS;
	}

	if (NULL != _tableParams[parameterID])
	{
		if (_tableParams[parameterID]->GetParamID() == parameterID)
		{
			if ((_tableParams[parameterID]->GetParamAvailable() == FALSE) || (_tableParams[parameterID]->GetParamReadOnly() == TRUE))
			{
				return FALSE;
			}

			if ((_tableParams[parameterID]->GetParamMin() <= paramTemp) && (_tableParams[parameterID]->GetParamMax() >= paramTemp))
			{
				if (PARAM_SCANNER_ZOOM_POS == parameterID)
				{
					paramTemp = paramTemp * AMPLITUDE_STEP_SIZE; //conversion for zoom/field size from 256 to 65536
				}

				// update the _twoWayZones array
				if (IDevice::PARAM_SCANNER_ALIGN_POS == parameterID)
				{
					paramTemp = round(paramTemp * MAX_PHASE / PHASE_SCALE); //conversion for phase size from 1800 to 2147483648

					if (paramID <= IDevice::PARAM_LAST_PARAM)
					{
						_twoWayZones[paramID - IDevice::PARAM_ECU_TWO_WAY_ZONE_1] = static_cast<long>(param);
					}
				}

				if (PARAM_LAMP_POS == parameterID)
				{
					paramTemp = paramTemp * CAMERA_LED_MAX / LED_GUI_MAX; //conversion for camera LED brightness from 0-100 to 0-4095
				}

				//If the LED ON OFF state changes, the brigtness position needs to be updated too. Since there is no internal Enable/Disable command
				if (PARAM_LEDS_ENABLE_DISABLE == parameterID)
				{
					_tableParams[PARAM_LAMP_POS]->SetParamBool(TRUE);
				}

				_tableParams[parameterID]->UpdateParam(static_cast<long>(paramTemp));

				// update scanner enable :TODO: Need to find out if we will still need it
				/*if ((IDevice::PARAM_SCANNER_ENABLE == parameterID))
				{
					_tableParams[parameterID]->SetParamBool(TRUE);
				}*/
				return TRUE;
			}

			wstring message = L"ThorPrelude SetParam failed. paramID: %d param: " + _tableParams[paramID]->GetParameterString(), parameterID;
			StringCbPrintfW(_errMsg, MSG_SIZE, message.c_str());
			LogMessage(_errMsg, ERROR_EVENT);

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
long ThorPrelude::GetParam(const long paramID, double& param)
{
	// update the _twoWayZones array and select the paramID to set the alignment
	if ((paramID >= IDevice::PARAM_ECU_TWO_WAY_ZONE_1) && (paramID <= IDevice::PARAM_ECU_TWO_WAY_ZONE_251))
	{
		param = _twoWayZones[paramID - IDevice::PARAM_ECU_TWO_WAY_ZONE_1];
		return TRUE;
	}

	if (NULL != _tableParams[paramID])
	{
		param = static_cast<double>(_tableParams[paramID]->GetParamVal());
		if (PARAM_SCANNER_ZOOM_POS == paramID)
		{
			param = static_cast<double>(_tableParams[paramID]->GetParamVal()) / AMPLITUDE_STEP_SIZE;
		}

		if (PARAM_SCANNER_ALIGN_POS == paramID)
		{
			param = round(static_cast<double>(_tableParams[paramID]->GetParamVal()) * PHASE_SCALE / MAX_PHASE);
		}

		if (PARAM_LAMP_POS == paramID)
		{
			param = static_cast<double>(_tableParams[paramID]->GetParamVal()) * LED_GUI_MAX / CAMERA_LED_MAX;
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
long ThorPrelude::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPrelude::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPrelude::SetParamString(const long paramID, wchar_t* str)
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
long ThorPrelude::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;
	switch (paramID)
	{
	case PARAM_CONTROL_UNIT_FIRMWAREVERSION:
	{
		wcscpy_s(str, size, _firmwareVersion.c_str());
		ret = TRUE;
	}
	break;
	case PARAM_CONTROL_UNIT_SERIALNUMBER:
	{
		wcscpy_s(str, size, _serialNumber.c_str());
		ret = TRUE;
	}
	}
	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorPrelude::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorPrelude::SetupPosition()
{
	for (std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if ((FALSE == (iter->second)->GetParamAvailable()) || (TRUE == (iter->second)->GetParamReadOnly()))
		{
			continue;
		}
		else if (TRUE == (iter->second)->GetParamBool())
		{
			continue;
		}
		else if ((iter->second)->GetParamVal() != (iter->second)->GetParamCurrent())
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
long ThorPrelude::StartPosition()
{
	long ret = FALSE;

	//iterate through map and set the parameters in reverse order because we want the position to be set at last
	for (std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if ((FALSE == (iter->second)->GetParamAvailable()) || (TRUE == (iter->second)->GetParamReadOnly()))
		{
			continue;
		}
		else if (TRUE == (iter->second)->GetParamBool())
		{
			ExecuteCmd(iter->second); //no need to parse read back

			(iter->second)->UpdateParam_C();

			ret = TRUE;
			(iter->second)->SetParamBool(FALSE);

			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude StartPosition succeeded at paramID: %d", (iter->second)->GetParamID());
			LogMessage(_errMsg, INFORMATION_EVENT);
		}
	}

	return ret;
}

/// <summary>
/// Status of the positioning.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorPrelude::StatusPosition(long& status)
{
	status = STATUS_READY;
	return TRUE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorPrelude::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorPrelude::GetLastErrorMsg(wchar_t* msg, long size)
{
	return TRUE;
}

/// <summary>
/// DEPRECATED
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorPrelude::ReadPosition(DeviceType deviceType, double& pos)
{
	return FALSE;
}

/// <summary>
/// Executes the command.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorPrelude::ExecuteCmd(ParamInfo* pParamInfo)
{
	//when the scanner init mode is enabled block commands trying to 
	//change the state of the scanner
	double val = 0;

	//---------------:TODO:----- see if we need this
	/*GetParam(IDevice::PARAM_SCANNER_INIT_MODE, val);

	if ((pParamInfo->GetParamID() == PARAM_SCANNER_ENABLE) && (static_cast<long>(pParamInfo->GetParamVal()) == 0) && (1 == static_cast<long>(val)))
	{
		return TRUE;
	}*/

	long paramID = pParamInfo->GetParamID();
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();
	std::vector<unsigned char> paramValBytes;
	switch (paramID)
	{
		/*case PARAM_SCANNER_ENABLE:
			//if the command is to enable (instead of disable) the scanner
			//Then it must be enabled by using the current angle
			//The scanner is disabled when the angle (Zoom pos) is 0
			//If the command is to disable the scanner, then nothing needs to happen;
			//the default command for this parameter is to disable the scanner
			if (TRUE == pParamInfo->GetParamVal())
			{
				if (NULL != _tableParams[PARAM_SCANNER_ZOOM_POS])
				{
					paramValBytes = GetBytes((int)_tableParams[PARAM_SCANNER_ZOOM_POS]->GetParamVal());
					cmd[6] = paramValBytes[0];
					cmd[7] = paramValBytes[1];
				}
			}
			break;*/

	case PARAM_SCANNER_ZOOM_POS:
		paramValBytes = GetBytes((int)pParamInfo->GetParamVal());
		cmd[7] = paramValBytes[0];
		cmd[8] = paramValBytes[1];
		cmd[9] = paramValBytes[2];
		cmd[10] = paramValBytes[3];
		break;
	case PARAM_SCANNER_ALIGN_POS:
		paramValBytes = GetBytes((int)pParamInfo->GetParamVal());
		cmd[7] = paramValBytes[0];
		cmd[8] = paramValBytes[1];
		cmd[9] = paramValBytes[2];
		cmd[10] = paramValBytes[3];
		break;
	case PARAM_LAMP_POS:
		// if the LED is disabled set the brightness to 0 since there is no enable/disable command
		//The DAC for this LED current control is 12 bits.  The lower 4 bits of the value sent will be truncated.
		if (LED_OUTPUT_ON == _tableParams[PARAM_LEDS_ENABLE_DISABLE]->GetParamVal())
		{
			int shiftedVal = (int)pParamInfo->GetParamVal() << 4;
			paramValBytes = GetBytes(shiftedVal);
		}
		else
		{
			paramValBytes = GetBytes(CAMERA_LED_MIN);
		}
		cmd[6] = paramValBytes[0];
		cmd[7] = paramValBytes[1];

		break;
	default:
		return TRUE;
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
long ThorPrelude::ExecuteCmd(std::vector<unsigned char> cmd, double& readBackValue)
{
	Lock lock(_critSect);

	unsigned char* sendBuf;
	sendBuf = cmd.data();
	std::string cmdStr(cmd.begin(), cmd.end());
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
long ThorPrelude::TeardownDevice()
{
	//-----------TODO------------- see if we need this
	/*if (_tableParams.size() > 0)
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
		SetupPosition();
		StartPosition();
	}*/

	_serialPort.Close();
	DestroyParamTable();
	_deviceDetected = false;

	return true;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorPrelude::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

/// <summary>
/// Gets the bytes.
/// </summary>
/// <param name="value">The value.</param>
/// <returns>std.vector&lt;unsigned char&gt;.</returns>
std::vector<unsigned char> ThorPrelude::GetBytes(int value)
{
	std::vector<unsigned char> bytes(sizeof(int));
	std::memcpy(&bytes[0], &value, sizeof(int));
	return bytes;
}

/// <summary>
/// Loades in the coarse alignment file.
/// </summary>
/// <returns>long.</returns>
long ThorPrelude::CoarseAlignDataLoadFile()
{
	char appPath[256]; ///<the char of application path

	char filePath[256]; ///<char of the whole file path
	if (NULL == _getcwd(appPath, 256))
	{
		return FALSE;
	}
	strcpy_s(filePath, appPath);
	strcat_s(filePath, "\\AlignDataCoarse.txt");

	FILE* AlignDataFile;
	if (fopen_s(&AlignDataFile, filePath, "r") != 0)
	{
	}
	else
	{
		for (int i = 0; i < 256; i++)
		{
			long lVal = 0;
			if (EOF == fscanf_s(AlignDataFile, "%d", &lVal))
			{
			}
			else if (lVal < MIN_PHASE)
			{
				lVal = MIN_PHASE;
			}
			else if (lVal > PHASE_SCALE)
			{
				lVal = PHASE_SCALE;
			}
			if (5 <= i)
			{
				_twoWayZones[255 - i] = lVal;
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
long ThorPrelude::GetFirmwareVersionAndSerialNumber(std::wstring& version, std::wstring& serialNum)
{
	vector<UCHAR>requestCommand = _tableParams[PARAM_CONTROL_UNIT_SERIALNUMBER]->GetCmdBytes();
	unsigned char* sendBuf;
	sendBuf = requestCommand.data();
	_serialPort.SendData(sendBuf, (int)requestCommand.size());
	Sleep(50);

	char readBuf[100];
	memset(readBuf, 0, sizeof(readBuf));
	_serialPort.ReadData(readBuf, 100);

	//Compare the first two bytes to the bytes from the expected response command MGMSG_STCK_SYS_GET_SERIAL_NUMBER
	UCHAR byteCmd1 = (UCHAR)(MGMSG_STCK_SYS_GET_SERIAL_NUMBER & 0xFF);
	UCHAR byteCmd2 = (UCHAR)((MGMSG_STCK_SYS_GET_SERIAL_NUMBER & 0xFF00) >> 8);
	if ((UCHAR)readBuf[0] == byteCmd1 && (UCHAR)readBuf[1] == byteCmd2)
	{
		int size = (UCHAR)readBuf[2] | (UCHAR)readBuf[3] << 8;
		if (1 == size)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude: Serial Number hasn't been set");
			LogMessage(_errMsg, INFORMATION_EVENT);
		}
		else if (100 < size)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude: Error, can't read serial number, set number is bigger than allocated space of 100 bytes");
			LogMessage(_errMsg, INFORMATION_EVENT);
		}
		else
		{
			wchar_t wstr[2];
			size_t returnVal = 0;
			int totalSerialNumbers = (UCHAR)readBuf[6];
			int startIndex = 7;
			serialNum = L"";
			for (int i = 0; i < totalSerialNumbers; i++)
			{
				int snIdentifier = (UCHAR)readBuf[startIndex + 1] | (UCHAR)readBuf[startIndex + 2] << 8;
				int serialSize = (UCHAR)readBuf[startIndex] - 2;
				//:TODO:If there are multiple serial numbers how would we handle them
				for (int k = startIndex + 3; k <= startIndex + serialSize; k++)
				{
					mbstowcs_s(&returnVal, wstr, 2, &readBuf[k], 1);
					serialNum += wstr;
				}
				startIndex = startIndex + (UCHAR)readBuf[startIndex] + 1;
			}
		}
	}

	//:TODO: There are two version numbers to be read, need to figure out how to handle both. Currently we concatenate both and it outputs 
	//something like "77-ITN007777-BIN 1 Jan  3 2023 11:21:1244-ITN004444-BINd 42 Jan  3 2023 11:23:27"
	version = L"";
	byteCmd1 = (UCHAR)(MGMSG_STCK_SYS_REQ_CM7_VERSION & 0xFF);
	byteCmd2 = (UCHAR)((MGMSG_STCK_SYS_REQ_CM7_VERSION & 0xFF00) >> 8);
	const size_t LEN = 6;
	unsigned char commandBytesCM7Version[LEN] = { byteCmd1, byteCmd2, 0x00, 0x00, 0x00, 0 }; //get board info from processor firmware

	_serialPort.SendData(commandBytesCM7Version, static_cast<int>(LEN));
	Sleep(50);
	memset(readBuf, 0, sizeof(readBuf));
	_serialPort.ReadData(readBuf, 100);

	//Compare the first two bytes to the bytes from the expected response command MGMSG_STCK_SYS_GET_SERIAL_NUMBER
	byteCmd1 = (UCHAR)(MGMSG_STCK_SYS_GET_CM7_VERSION & 0xFF);
	byteCmd2 = (UCHAR)((MGMSG_STCK_SYS_GET_CM7_VERSION & 0xFF00) >> 8);
	if ((UCHAR)readBuf[0] == byteCmd1 && (UCHAR)readBuf[1] == byteCmd2)
	{
		int size = (UCHAR)readBuf[2] | (UCHAR)readBuf[3] << 8;
		if (1 == size)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude: CM7 version number hasn't been set");
			LogMessage(_errMsg, INFORMATION_EVENT);
		}
		else if (100 < size)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude: Error, can't CM7 version number, set number is bigger than allocated space of 100 bytes");
			LogMessage(_errMsg, INFORMATION_EVENT);
		}

		wchar_t wstr[2];
		size_t returnVal = 0;
		int startIndex = 6;
		for (int i = startIndex; i < startIndex + size; i++)
		{
			mbstowcs_s(&returnVal, wstr, 2, &readBuf[i], 1);
			version += wstr;
		}
		startIndex = startIndex + (UCHAR)readBuf[startIndex] + 1;
	}

	byteCmd1 = (UCHAR)(MGMSG_STCK_SYS_REQ_CM4_VERSION & 0xFF);
	byteCmd2 = (UCHAR)((MGMSG_STCK_SYS_REQ_CM4_VERSION & 0xFF00) >> 8);
	unsigned char commandBytesCM4Version[LEN] = { byteCmd1, byteCmd2, 0x00, 0x00, 0x00, 0 }; //get board info from processor firmware

	_serialPort.SendData(commandBytesCM4Version, static_cast<int>(LEN));
	Sleep(50);
	memset(readBuf, 0, sizeof(readBuf));
	_serialPort.ReadData(readBuf, 100);

	//Compare the first two bytes to the bytes from the expected response command MGMSG_STCK_SYS_GET_SERIAL_NUMBER
	byteCmd1 = (UCHAR)(MGMSG_STCK_SYS_GET_CM4_VERSION & 0xFF);
	byteCmd2 = (UCHAR)((MGMSG_STCK_SYS_GET_CM4_VERSION & 0xFF00) >> 8);
	if ((UCHAR)readBuf[0] == byteCmd1 && (UCHAR)readBuf[1] == byteCmd2)
	{
		int size = (UCHAR)readBuf[2] | (UCHAR)readBuf[3] << 8;
		if (1 == size)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude: CM4 version number hasn't been set");
			LogMessage(_errMsg, INFORMATION_EVENT);
		}
		else if (100 < size)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude: Error, can't CM4 version number, set number is bigger than allocated space of 100 bytes");
			LogMessage(_errMsg, INFORMATION_EVENT);
		}

		wchar_t wstr[2];
		size_t returnVal = 0;
		int startIndex = 6;
		for (int i = startIndex; i < startIndex + size; i++)
		{
			mbstowcs_s(&returnVal, wstr, 2, &readBuf[i], 1);
			version += wstr;
		}
		startIndex = startIndex + (UCHAR)readBuf[startIndex] + 1;
	}

	return true;
}


/// <summary>
/// Creates the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorPrelude::CreateParamTable()
{
	DestroyParamTable();
	//Execution in order of top to bottom of list:
	std::vector<unsigned char> commandBytes;
	UCHAR byteCmd1;
	UCHAR byteCmd2;
	commandBytes.push_back(0x00);
	ParamInfo* tempParamInfo = new ParamInfo(
		PARAM_DEVICE_TYPE,							//ID
		L"PARAM_DEVICE_TYPE",						//Parameter description
		CONTROL_UNIT | LAMP,								//VAL
		CONTROL_UNIT | LAMP,								//PARAM C
		FALSE,										//PARAM B
		TYPE_LONG,									//TYPE
		TRUE,										//AVAILABLE
		TRUE,										//READ ONLY
		CONTROL_UNIT | LAMP,								//MIN
		CONTROL_UNIT | LAMP,								//MAX
		CONTROL_UNIT | LAMP,								//DEFAULT
		commandBytes,								//Command
		-1,											//Device Index	
		-1											//Related parameter ID	
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(Params::PARAM_DEVICE_TYPE, tempParamInfo));

	//The Prelude doesn't have an enable command. :TODO: Check if we would still need this param
	/*commandBytes.clear();
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(
		PARAM_SCANNER_ENABLE,
		L"PARAM_SCANNER_ENABLE",
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_SCANNER_ENABLE, tempParamInfo));*/

	//Byte 6 is the Operation, set to RAM copy access and to write
	//Byte 7 is the Operation mode, by default should be 0 to store in RAM. Bytes 8 - 11 is the amplitude value to be set.
	byteCmd1 = (UCHAR)(MGMSG_STCK_RES_SET_AMPLITUDE & 0xFF);
	byteCmd2 = (UCHAR)((MGMSG_STCK_RES_SET_AMPLITUDE & 0xFF00) >> 8);
	unsigned char commandBytesZoomPos[] = { byteCmd1, byteCmd2, 0x05, 0x00, MOTHERBOARD_ID, HOST_ID, 0x05, 0x00, 0x00, 0x00, 0x00 };
	commandBytes.assign(commandBytesZoomPos, commandBytesZoomPos + sizeof(commandBytesZoomPos) / sizeof(commandBytesZoomPos[0]));
	tempParamInfo = new ParamInfo(
		PARAM_SCANNER_ZOOM_POS,
		L"PARAM_SCANNER_ZOOM_POS",
		SCANNER_ZOOM_DEFAULT,
		SCANNER_ZOOM_DEFAULT,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		SCANNER_ZOOM_MIN,
		SCANNER_ZOOM_MAX,
		SCANNER_ZOOM_DEFAULT,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_SCANNER_ZOOM_POS, tempParamInfo));

	//Byte 6 is the Operation, set to RAM copy access and to write
	//Byte 7 is the Operation mode, by default should be 0 to store in RAM. Bytes 8 - 11 is the phase value to be set.
	byteCmd1 = (UCHAR)(MGMSG_STCK_RES_SET_PHASE & 0xFF);
	byteCmd2 = (UCHAR)((MGMSG_STCK_RES_SET_PHASE & 0xFF00) >> 8);
	unsigned char commandBytesOffset[] = { byteCmd1, byteCmd2, 0x05, 0x00, MOTHERBOARD_ID, HOST_ID, 0x05, 0x00, 0x00, 0x00, 0x00 };
	commandBytes.assign(commandBytesOffset, commandBytesOffset + sizeof(commandBytesOffset) / sizeof(commandBytesOffset[0]));
	tempParamInfo = new ParamInfo(
		PARAM_SCANNER_ALIGN_POS,
		L"PARAM_SCANNER_ALIGN_POS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		MIN_PHASE,
		PHASE_SCALE,
		MIN_PHASE,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_SCANNER_ALIGN_POS, tempParamInfo));

	//if this function is called it means the connection has been stablished
	commandBytes.clear();
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(
		PARAM_CONNECTION_STATUS,
		L"PARAM_CONNECTION_STATUS",
		(long)ConnectionStatusType::CONNECTION_READY,
		(long)ConnectionStatusType::CONNECTION_READY,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		(long)ConnectionStatusType::CONNECTION_WARMING_UP,
		(long)ConnectionStatusType::CONNECTION_ERROR_STATE,
		(long)ConnectionStatusType::CONNECTION_UNAVAILABLE,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTION_STATUS, tempParamInfo));

	byteCmd1 = (UCHAR)(MGMSG_STCK_SYS_SET_FW_UPDATE & 0xFF);
	byteCmd2 = (UCHAR)((MGMSG_STCK_SYS_SET_FW_UPDATE & 0xFF00) >> 8);
	unsigned char commandFirmwareUpdate[] = { byteCmd1, byteCmd2, 0x01, 0x00, MOTHERBOARD_ID, HOST_ID, 0x01 };
	commandBytes.assign(commandFirmwareUpdate, commandFirmwareUpdate + sizeof(commandFirmwareUpdate) / sizeof(commandFirmwareUpdate[0]));
	tempParamInfo = new ParamInfo(
		PARAM_CONTROL_UNIT_FIRMWAREUPDATE,
		L"PARAM_CONTROL_UNIT_FIRMWAREUPDATE",
		FALSE,
		FALSE,
		FALSE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONTROL_UNIT_FIRMWAREUPDATE, tempParamInfo));

	//if this function is called it means the connection has been stablished. There are two firmware versions for this device. 
	//They are defined in GetFirmwareVersionAndSerialNumber 
	commandBytes.clear();
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(
		PARAM_CONTROL_UNIT_FIRMWAREVERSION,
		L"PARAM_CONTROL_UNIT_FIRMWAREVERSION",
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONTROL_UNIT_FIRMWAREVERSION, tempParamInfo));

	byteCmd1 = (UCHAR)(MGMSG_STCK_SYS_REQ_SERIAL_NUMBER & 0xFF);
	byteCmd2 = (UCHAR)((MGMSG_STCK_SYS_REQ_SERIAL_NUMBER & 0xFF00) >> 8);
	unsigned char commandSerialNumber[] = { byteCmd1, byteCmd2, 0x00, 0x00, 0x00, 0x00 };
	commandBytes.assign(commandSerialNumber, commandSerialNumber + sizeof(commandSerialNumber) / sizeof(commandSerialNumber[0]));
	tempParamInfo = new ParamInfo(
		PARAM_CONTROL_UNIT_SERIALNUMBER,
		L"PARAM_CONTROL_UNIT_SERIALNUMBER",
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONTROL_UNIT_SERIALNUMBER, tempParamInfo));

	commandBytes.clear();
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(
		PARAM_CONTROL_UNIT_TYPE,
		L"PARAM_CONTROL_UNIT_TYPE",
		ScopeType::PRELUDE,
		ScopeType::PRELUDE,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		ScopeType::PRELUDE,
		ScopeType::PRELUDE,
		ScopeType::PRELUDE,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONTROL_UNIT_TYPE, tempParamInfo));

	commandBytes.clear();
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(
		PARAM_SCANNER_INIT_MODE,
		L"PARAM_SCANNER_INIT_MODE",
		TRUE,
		TRUE,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		TRUE,
		TRUE,
		TRUE,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_SCANNER_INIT_MODE, tempParamInfo));

	commandBytes.clear();
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(
		PARAM_LEDS_ENABLE_DISABLE,
		L"PARAM_LEDS_ENABLE_DISABLE",
		LED_OUTPUT_OFF,
		LED_OUTPUT_OFF,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		LED_OUTPUT_OFF,
		LED_OUTPUT_ON,
		LED_OUTPUT_OFF,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LEDS_ENABLE_DISABLE, tempParamInfo));

	//Byte 6 and Byte 7 set the brightness. The DAC for this LED current control is 12 bits.  The lower 4 bits of the value sent will be truncated.
	byteCmd1 = (UCHAR)(MGMSG_STCK_SYS_SET_CAMERA_LED_CONTROL & 0xFF);
	byteCmd2 = (UCHAR)((MGMSG_STCK_SYS_SET_CAMERA_LED_CONTROL & 0xFF00) >> 8);
	unsigned char commandBytesLEDPos[] = { byteCmd1, byteCmd2, 0x02, 0x00, MOTHERBOARD_ID, HOST_ID, 0x00, 0x00 };
	commandBytes.assign(commandBytesLEDPos, commandBytesLEDPos + sizeof(commandBytesLEDPos) / sizeof(commandBytesLEDPos[0]));
	tempParamInfo = new ParamInfo(
		PARAM_LAMP_POS,
		L"PARAM_LAMP_POS",
		CAMERA_LED_MIN,
		CAMERA_LED_MIN,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		CAMERA_LED_MIN,
		LED_GUI_MAX,
		CAMERA_LED_MIN,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LAMP_POS, tempParamInfo));

	commandBytes.clear();
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(
		PARAM_LAMP_TYPE,
		L"PARAM_LAMP_TYPE",
		LampTypes::Prelude_LED,
		LampTypes::Prelude_LED,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		LampTypes::Prelude_LED,
		LampTypes::Prelude_LED,
		LampTypes::Prelude_LED,
		commandBytes,
		-1,
		-1
	);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LAMP_TYPE, tempParamInfo));

	return TRUE;
}

/// <summary>
/// Destroys the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorPrelude::DestroyParamTable()
{
	try
	{
		for (std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
		{
			delete iter->second;
		}
		_tableParams.clear();
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorPrelude DestroyParamTable unable to destroy the table created on heap");
		LogMessage(_errMsg, ERROR_EVENT);
		return FALSE;
	}

	return TRUE;
}