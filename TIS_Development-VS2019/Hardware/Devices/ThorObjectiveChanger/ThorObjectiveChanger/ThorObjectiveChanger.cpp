#include "Strsafe.h"
#include "ThorObjectiveChanger.h"
#include "ThorObjectiveChangerXML.h"

const string THORLABS_VID = "1313";
const string THORLABS_OBJECTIVE_CHANGER_PID = "2006";

ThorObjectiveChanger::ThorObjectiveChanger() :
	_deviceDetected(false),
	_homed(0),
	_status(STATUS_READY),
	HOST_ID(0x01),
	MOTHERBOARD_ID(0x11),
	UmDriverCountConv(0.00004765625),
	UmEncoderCountConv(0.00169444444)
{
	UCHAR cmd[] = {  0x05, 0x00, 0x00, 0x00, MOTHERBOARD_ID, HOST_ID,
		0x80, 0x04, 0x00, 0x00, MOTHERBOARD_ID, HOST_ID,
		0xB7, 0x04, 0x00, 0x00, MOTHERBOARD_ID, HOST_ID,		//This command is obsolete in newest firmware (ver 3.0 and up)
		0x16, 0x04, 0x16, 0x00, (MOTHERBOARD_ID | 0x80), HOST_ID,
		0x00, 0x00,					// Chan Ident
		0x00, 0x02,					// Jog Mode - single step jogging
		0x00, 0x00, 0x00, 0x00,		// Jog Step Size in encoder counts
		0x00, 0x00, 0x00, 0x00,		// Jog Min Velocity (not used)
		0x00, 0x00, 0x00, 0x00,		// Jog Acceleration (not used)
		0x00, 0x00, 0x00, 0x00,		// Jog Max Velocity (not used)
		0x00, 0x02,					// Stop Mode - 1 for immediate (abrupt) stop or 2 for profiled stop (with controlled deceleration).
		0xB6, 0x04, 0x10, 0x00, (MOTHERBOARD_ID | 0x80), HOST_ID,
		0x00, 0x00,					// Chan Ident
		0x00, 0x02,                   // Mode - goto 2 different positions
		0x00, 0x00, 0x00, 0x00,		// Position 1 in encoder counts
		0x00, 0x00, 0x00, 0x00,		// Position 2 in encoder counts
		0x00, 0x00,					// Timeout (not used)
		0x00, 0x00,					// (modified) lasy byte used for: 0 = save, 1 = move to position 1, 2 = move to position 2
		0xB6, 0x04, 0x10, 0x00, (MOTHERBOARD_ID | 0x80), HOST_ID,
		0x00, 0x00,					// Chan Ident
		0x00, 0x02,                 // Mode - goto 2 different positions
		0x00, 0x00, 0x00, 0x00,		// Position 1 in encoder counts
		0x00, 0x00, 0x00, 0x00,		// Position 2 in encoder counts
		0x00, 0x00,					// Timeout (not used)
		0x00, 0x01,					// (modified) lasy byte used for: 0 = save, 1 = move to position 1, 2 = move to position 2
		0xB6, 0x04, 0x10, 0x00, (MOTHERBOARD_ID | 0x80), HOST_ID,
		0x00, 0x00,					// Chan Ident
		0x00, 0x02,                 // Mode - goto 2 different positions
		0x00, 0x00, 0x00, 0x00,		// Position 1 in encoder counts
		0x00, 0x00, 0x00, 0x00,		// Position 2 in encoder counts
		0x00, 0x00,					// Timeout (not used)
		0x00, 0x02,					// (modified) lasy byte used for: 0 = save, 1 = move to position 1, 2 = move to position 2
		0x43, 0x04, 0x00, 0x00, MOTHERBOARD_ID, HOST_ID,
		0x6A, 0x04, 0x00, 0x00, MOTHERBOARD_ID, HOST_ID,
		0x6A, 0x04, 0x00, 0x01, MOTHERBOARD_ID, HOST_ID,
		0x65, 0x04, 0x00, 0x00, MOTHERBOARD_ID, HOST_ID,
		0xA6, 0x00, 0x04, 0x00, MOTHERBOARD_ID, HOST_ID,
		0x00, 0x00, 0x00, 0x00		// Update Firmware command, sets it to bootloader mode
	};
	_cmdTBuff = (char*)malloc(sizeof(cmd));
	memcpy(_cmdTBuff, cmd, sizeof(cmd));
	_cmdTemplates[GET_BRD_INFO].offset = 0;
	_cmdTemplates[GET_BRD_INFO].cmdTstring = _cmdTBuff + _cmdTemplates[GET_BRD_INFO].offset;
	_cmdTemplates[GET_BRD_INFO].len = 6;

	_cmdTemplates[GET_MTR_STAT].offset = 6;
	_cmdTemplates[GET_MTR_STAT].cmdTstring = _cmdTBuff + _cmdTemplates[GET_MTR_STAT].offset;
	_cmdTemplates[GET_MTR_STAT].len = 6;

	_cmdTemplates[GET_POS_INFO].offset = 12;
	_cmdTemplates[GET_POS_INFO].cmdTstring = _cmdTBuff + _cmdTemplates[GET_POS_INFO].offset;
	_cmdTemplates[GET_POS_INFO].len = 6;

	_cmdTemplates[SET_JOG_SIZE].offset = 18;
	_cmdTemplates[SET_JOG_SIZE].cmdTstring = _cmdTBuff + _cmdTemplates[SET_JOG_SIZE].offset;
	_cmdTemplates[SET_JOG_SIZE].len = 28;

	_cmdTemplates[SET_POS_1_2].offset = 46;
	_cmdTemplates[SET_POS_1_2].cmdTstring = _cmdTBuff + _cmdTemplates[SET_POS_1_2].offset;
	_cmdTemplates[SET_POS_1_2].len = 22;

	_cmdTemplates[GOTO_POS_1].offset = 68;
	_cmdTemplates[GOTO_POS_1].cmdTstring = _cmdTBuff + _cmdTemplates[GOTO_POS_1].offset;
	_cmdTemplates[GOTO_POS_1].len = 22;

	_cmdTemplates[GOTO_POS_2].offset = 90;
	_cmdTemplates[GOTO_POS_2].cmdTstring = _cmdTBuff + _cmdTemplates[GOTO_POS_2].offset;
	_cmdTemplates[GOTO_POS_2].len = 22;

	_cmdTemplates[GOTO_HOME].offset = 112;
	_cmdTemplates[GOTO_HOME].cmdTstring = _cmdTBuff + _cmdTemplates[GOTO_HOME].offset;
	_cmdTemplates[GOTO_HOME].len = 6;

	_cmdTemplates[JOG_FORWARD].offset = 118;
	_cmdTemplates[JOG_FORWARD].cmdTstring = _cmdTBuff + _cmdTemplates[JOG_FORWARD].offset;
	_cmdTemplates[JOG_FORWARD].len = 6;

	_cmdTemplates[JOG_BACKWARD].offset = 124;
	_cmdTemplates[JOG_BACKWARD].cmdTstring = _cmdTBuff + _cmdTemplates[JOG_BACKWARD].offset;
	_cmdTemplates[JOG_BACKWARD].len = 6;

	_cmdTemplates[JOG_STOP].offset = 130;
	_cmdTemplates[JOG_STOP].cmdTstring = _cmdTBuff + _cmdTemplates[JOG_STOP].offset;
	_cmdTemplates[JOG_STOP].len = 6;

	_cmdTemplates[FIRMWARE_UPDATE].offset = 136;
	_cmdTemplates[FIRMWARE_UPDATE].cmdTstring = _cmdTBuff + _cmdTemplates[FIRMWARE_UPDATE].offset;
	_cmdTemplates[FIRMWARE_UPDATE].len = 10;

	_cmdTemplateBuffSize = 0;
	_currentPos1 = 0;
	_currentPos2 = 0;
	_errMsg[0] = NULL;
	_moverLoc = MOVER_LOCATION::DISCONNECTED;
	_moverLoc_B = FALSE;
	_moverPos = (long)MOVER_LOCATION::DISCONNECTED;
	_mtrStat = {};
	_pos1 = 0;
	_pos2 = 0;
	_rawDriverCount = 0;
	_rawEncoderCount = 0;
	_selectedDevIndex = 0;
	_umDriverCount = 0;
	_umEncoderCount = 0;
}

ThorObjectiveChanger::~ThorObjectiveChanger()
{
	_instanceFlag = false;
	_moverLoc_B = TRUE;
}

bool ThorObjectiveChanger::_instanceFlag = false;

auto_ptr<ThorObjectiveChanger> ThorObjectiveChanger::_single(new ThorObjectiveChanger());

ThorObjectiveChanger *ThorObjectiveChanger::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorObjectiveChanger());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorObjectiveChanger::RefreshChangerCurrentPos()
{
	if(FALSE == _deviceDetected)
	{
		return FALSE;
	}
	if(false == GetValue(GET_MTR_STAT))
	{
		_moverPos = (long)MOVER_LOCATION::DISCONNECTED;
		return FALSE;
	}
	// return -1 if it's not in any position, we don't want the mover return
	// a false position
	_moverPos = DEFAULT_POS; 
	if(_mtrStat.lowerLimit)
	{
		_moverPos = (long)MOVER_LOCATION::POS1;
	}
	if(_mtrStat.upperLimit)
	{
		_moverPos = (long)MOVER_LOCATION::POS2;
	}
	if(_mtrStat.homing)
	{
		//This one doesn't really return 1 when homing, because homing is equal to moving to pos1
		_moverPos = (long)MOVER_LOCATION::HOMING;
	}
	if(_mtrStat.moving_ccw)
	{
		_moverPos = (long)MOVER_LOCATION::MOVING_POS1;
	}
	if(_mtrStat.moving_cw)
	{
		_moverPos = (long)MOVER_LOCATION::MOVING_POS2;
	}

	return TRUE;
}

// Returns a wstring list of serial number from all connected devices associated with the passed VID and PID.
list<string> ThorObjectiveChanger::SerialNumbers(string VID, string PID)
{
	wchar_t data[DATA_SIZE];
	HKEY hk;
	DWORD count = 0;
	DWORD sz = sizeof(DWORD);
	list<string> serialNumbers;
	string dataString;
	string format = "VID_" + VID + ".*&.*PID_" + PID + "\\\\";
	regex reg (format);
	cmatch matchedResult;
	// Select registry key System\CurrentControlSet\servies\usbser as hk
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\usbser", 0, KEY_READ, &hk))
	{
		// No usbSer device is connected, return empty list (size=0).
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorObjectiveChanger: Could not find registry key SYSTEM\\CurrentControlSet\\services\\usbser");
		LogMessage(_errMsg);
		return serialNumbers;
	}
	// Read parameter Count which is the number of connected usbser devices 
	if(ERROR_SUCCESS != RegGetValue(hk, L"Enum", L"Count", RRF_RT_REG_DWORD, NULL, (LPBYTE)&count, &sz))
	{
		// No usbSer device is connected, return empty list (size=0).
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorObjectiveChanger: No USB serial device is connected, registry key SYSTEM\\CurrentControlSet\\services\\usbser\\Enum doesn't exist");
		LogMessage(_errMsg);
		return serialNumbers;
	}
	// Iterate through the parameters, one for each connected device
	for(DWORD i = 0; i < count; i++)
	{
		DWORD cbData = DATA_SIZE;
		wchar_t usbConnectedIndex[DATA_SIZE];
		// Use i as the parameter name, the parameter name for each device is their index number
		swprintf_s(usbConnectedIndex, DATA_SIZE, L"%d", i);
		// Get the data associated with each device connected inside the Enum key
		if(ERROR_SUCCESS != RegGetValue(hk, L"Enum", usbConnectedIndex, RRF_RT_REG_SZ, NULL, (LPBYTE)data, &cbData))
			continue;
		// Convert the returned wstring data to string for regex search
		dataString = ConvertWStringToString(wstring(data));
		// serach for the regular expression VID/PID in 'data' using this format 'VID_####&PID_####\'
		if(regex_search(dataString.c_str(), matchedResult, reg))
		{
			// If regex search matched, store the substring after the regex format. This is the 17-digit serial number of the device.
			serialNumbers.push_back(matchedResult[0].second);
		}
		else
		{
			wstring messageWstring = L"ThorObjectiveChanger: USB serial device " + wstring(data) + L" doesn't match device PID and VID";
			StringCbPrintfW(_errMsg, MSG_SIZE, messageWstring.c_str()); 
			// It would be hellpful to put this message as a Verbose type of log message.
			LogMessage(_errMsg);
		}
	}
	return serialNumbers;
}

// Search the registry for a device that matches the passed VID, PID, and serial number.
// Returns a wstring with the port name of the matching device. Returned value format: "COM##" 
wstring ThorObjectiveChanger::FindCOMPort(string VID, string PID, string serialNum)
{
	HKEY hk;
	HKEY hSubKey;
	HKEY deviceSubKey;
	HKEY parameterSubKey;
	wstring comPort;
	wstring path;
	string deviceNames;
	string devicesPidVidString;
	string format = "VID_" + VID + ".*&.*PID_" + PID;
	regex reg (format);
	cmatch matchedResult;
	// Select registry key System\CurrentControlSet\Enum as hk
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum", 0, KEY_READ, &hk) != ERROR_SUCCESS)
		return comPort;
	// For every subfolder in the key generate a new key (SubKeyName) and open the subfolder
	for (DWORD i = 0; ;i++)
	{
		DWORD cName = DATA_SIZE;
		wchar_t SubKeyName[DATA_SIZE];
		// Get the name of the ith subkey
		if (ERROR_SUCCESS != RegEnumKeyEx(hk, i, SubKeyName, &cName, NULL, NULL, NULL, NULL))
			break;
		// Set the path to the subkey and open it in hsubkey
		path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName);
		if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &hSubKey))
			break;
		// For every subfolder in the key generate a new key (deviceSubKey) and open the subfolder
		for(DWORD j = 0; ; j++)
		{		
			DWORD dName = DATA_SIZE;
			wchar_t devicesPidVid[DATA_SIZE];
			// Get the name of the jth subkey and save it in devicesPidVid
			if (ERROR_SUCCESS != RegEnumKeyEx(hSubKey, j, devicesPidVid, &dName, NULL, NULL, NULL, NULL))
				break;
			// Convert the key name to string for RegEx search
			devicesPidVidString = ConvertWStringToString(wstring(devicesPidVid));
			// Compare the key name to the passed VID/PID using this format VID_####&PID_####
			if(regex_search(devicesPidVidString.c_str(), matchedResult, reg))
			{
				// If the VID and PID match, open this subkey with the matching VID/PID
				path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName) + L"\\" + wstring(devicesPidVid);
				if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &deviceSubKey))
					break;
				// For every subfolder in the matching VID/PID folder look at their key name and compare it to the passed serial number, 
				// if it matches grab the COM port number from the PortName field
				for(DWORD k = 0; ; k++)
				{		
					DWORD pName = DATA_SIZE;
					wchar_t deviceHID[DATA_SIZE];
					// Get the name of the kth subkey and store it in DeviceHID
					if (ERROR_SUCCESS != RegEnumKeyEx(deviceSubKey, k, deviceHID, &pName, NULL, NULL, NULL, NULL))
						break;
					// Convert the name of the subkey to a string and compare it to the passed serialNum
					deviceNames = ConvertWStringToString(wstring(deviceHID));
					if(0 == deviceNames.compare(serialNum))
					{
						DWORD cbData = DATA_SIZE;
						wchar_t value[DATA_SIZE];
						// Set the path the subkey with the matching serial number and open it in parameterSubKey
						path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName) + L"\\" + wstring(devicesPidVid) + L"\\" + wstring(deviceHID);
						if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_QUERY_VALUE, &parameterSubKey))
							continue;
						// Get the value from the PortName parameter
						if(ERROR_SUCCESS != RegGetValue(parameterSubKey, L"Device Parameters", L"PortName", RRF_RT_REG_SZ, NULL, (LPBYTE)value, &cbData))
							continue;
						comPort = wstring(value);
					}
				}
			}
		}
	}
	return comPort;
}

long ThorObjectiveChanger::GetSerialNumberString(const long index, wchar_t * str, long size)
{
	if(256 > size && 0 < size)
	{
		list<string>::iterator it;
		long count = 0;
		string serialNum;
		if(index <= _serialNums.size())
		{
			for(it = _serialNums.begin(); it != _serialNums.end(); ++it)
			{
				if(count == index)
				{
					serialNum = it->data();
					wchar_t* string = (wchar_t*)serialNum[0];
					wstring wide;
					wide.assign(serialNum.begin(), serialNum.end());
					wcscpy_s(str, size, wide.c_str());
				}
				count++;
			}
			return TRUE;
		}
	}
	return FALSE;
}

long ThorObjectiveChanger::FindDevices(long &deviceCount)
{
	long ret = FALSE;

	//Search the Registry for any connected devices associated with this PID and VID.
	_serialNums = SerialNumbers(THORLABS_VID, THORLABS_OBJECTIVE_CHANGER_PID);
	if ( 0 < _serialNums.size())
	{
		_deviceDetected = TRUE;
		ret = TRUE;
	}
	else
	{
		_deviceDetected = FALSE;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorObjectiveChanger: Unable to find any device connected in the registry associated with this PID and VID");
		LogMessage(_errMsg);
	}
	deviceCount = (int)_serialNums.size();
	return ret;
}

long ThorObjectiveChanger::SelectDevice(const long device)
{
	long portID=0;
	long baudRate = 0;
	long count = 0;
	wstring port;
	string serialNum;
	wstring settingsSerialNum;
	list<string>::iterator it;

	if(0 > device)
	{
		return FALSE;
	}

	if (0 >= _serialNums.size())
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorObjectiveChanger: Select Device, no device was detected.");
		LogMessage(_errMsg);
		return FALSE;
	}

	try
	{		
		std::unique_ptr<ThorObjectiveChangerXML> pSetup(new ThorObjectiveChangerXML());
		pSetup->GetConnection(portID, baudRate, settingsSerialNum, _homed);
	}
	catch(exception e)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Unable to locate ThorObjectiveChangerSettings.xml");
		LogMessage(_errMsg);
		MessageBox(NULL, _errMsg, L"Cannot read settings file", MB_OK | MB_DEFAULT_DESKTOP_ONLY | MB_ICONERROR);
		return FALSE;
	}

	// Iterate through the list of serial numbers until the index matches the value passed
	for(it = _serialNums.begin(); it != _serialNums.end(); ++it)
	{
		if(count == device)
		{
			serialNum = it->data();
			//Search the Registry for the COM port associated with the PID&IVID and the selected device serial number.
			port = FindCOMPort(THORLABS_VID, THORLABS_OBJECTIVE_CHANGER_PID, serialNum);
			// port wstring returned should be more than 3 characters
			if(3 >= port.size())
			{
				wstring messageWstring = L"ThorObjectiveChanger: Returned port ID doesn't match required format to continue. Format should be: COM##, format returned is: " + port;
				StringCbPrintfW(_errMsg, MSG_SIZE, messageWstring.c_str());
				LogMessage(_errMsg);
				return FALSE;
			}
		}
		count++;
	}
	// Open the serial port after getting the number
	wstring portNumber = port.substr(3,port.size());
	int portNum = stoi(portNumber);
	_serialPort.Close();
	_deviceDetected = _serialPort.Open(portNum,baudRate);
	if(FALSE == _deviceDetected)
	{
		_serialPort.Close();
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorObjectiveChanger: unable to open COM port %d", portNum);
		LogMessage(_errMsg);
	}
	else
	{
		//Need to wait some time before requesting the board information. Otherwise firmware returns garbage data.
		Sleep(100);
		// Get all the board information and compare the serial number stored in board with the one found earlier
		GetValue(GET_BRD_INFO);
		if(0 != _brdSerialNum.compare(serialNum))
		{
			// Show message box telling the user that serial number from device firmware and serial number in HID are not the same
			// Leave this for later
			/*wstring messageWString = L"ThorObjectiveChanger: Device in COM port " + portNumber + 
			L" doesn't have the same serial number stored in firmware. Please update the firmware so the device HID matches the serial number";
			MessageBox(NULL, messageWString.c_str(), L"Connection Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY | MB_ICONERROR);
			return FALSE;*/
		}
		RefreshChangerCurrentPos();
		//SetParam(PARAM_TURRET_POS, 2);
		//SetParam(PARAM_TURRET_POS, 1);
		//SetParam(PARAM_TURRET_POS, 0);
	}
	return TRUE;
}

long ThorObjectiveChanger::TeardownDevice()
{
	try
	{
		std::unique_ptr<ThorObjectiveChangerXML> pSetup(new ThorObjectiveChangerXML());
		pSetup->UpdateHomedFlagToSettings(_homed);
		pSetup->SaveConfigFile();
	}
	catch(std::exception e)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Cannot write home flag into ThorObjectiveChangerSettings.xml");
		LogMessage(_errMsg);
		MessageBox(NULL, _errMsg, L"Cannot save to settings file", MB_OK | MB_DEFAULT_DESKTOP_ONLY | MB_ICONERROR);
	}
	_serialPort.Close();
	_moverPos = DEFAULT_POS;
	return TRUE;
}

long ThorObjectiveChanger::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;
	switch (paramID)
	{
	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = TURRET;
			paramMax = TURRET;
			paramDefault = TURRET;
		}
		break;
	case PARAM_TURRET_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 0;
		}
		break;
	case PARAM_TURRET_POS_CURRENT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 6;
			paramDefault = 0;
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = (double)ConnectionStatusType::CONNECTION_READY;
			paramMax = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
			paramDefault = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
		break;
	case PARAM_TURRET_HOMED:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
	case PARAM_TURRET_COLLISION:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
	default:
		paramAvailable = FALSE;
		ret = FALSE;
	}
	return ret;
}

long ThorObjectiveChanger::SetParam(const long paramID, const double param)
{
	long ret = TRUE;
	switch (paramID)
	{
	case PARAM_TURRET_POS:
		{
			if ((param >= 0) && (param <= 2))
			{
				long _tmpPos = (static_cast<long>(param))%3;			
				_moverLoc = (MOVER_LOCATION)(_tmpPos);
				_moverLoc_B = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_TURRET_POS out of range 0 to 2");
				ret = FALSE;
			}
		}
		break;
	case PARAM_TURRET_FIRMWAREUPDATE:
		{
			SetValue(FIRMWARE_UPDATE, 0, 0);
		}
		break;
	case PARAM_TURRET_STOP:
		{
			SetValue(JOG_STOP, 0, 0);
		}
		break;
	default:
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter not implemented");
			ret = FALSE;
		}
	}
	return ret;
}

long ThorObjectiveChanger::SetParamString(const long paramID, wchar_t * str)
{
	return FALSE;
}

long ThorObjectiveChanger::SetParamBuffer(const long paramID, char * buffer, long size)
{
	return FALSE;
}

long ThorObjectiveChanger::GetParam(const long paramID, double &param)
{
	long ret = TRUE;
	switch (paramID)
	{
	case PARAM_DEVICE_TYPE:
		{
			param = TURRET;
		}
		break;
	case PARAM_TURRET_POS:
		{
			param = static_cast<double>(_moverLoc);
		}
		break;
	case PARAM_TURRET_POS_CURRENT:
		{
			RefreshChangerCurrentPos();
			param =_moverPos;			
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = (_deviceDetected) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
		break;
	case PARAM_TURRET_HOMED:
		{
			RefreshChangerCurrentPos();
			if (_mtrStat.homed && TRUE == _homed)
			{
				param = _mtrStat.homed;
			}
			else
			{
				param = FALSE;
			}
		}
		break;
	case PARAM_TURRET_COLLISION:
		{
			RefreshChangerCurrentPos();
			param = _mtrStat.collision;
		}
		break;
	default:
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter not implemented");
			ret = FALSE;
		}
	}
	return ret;
}

long ThorObjectiveChanger::GetParamString(const long paramID, wchar_t * str, long size)
{
	long ret = FALSE;
	if(256 > size && 0 < size)
	{

		switch (paramID)
		{
		case PARAM_TURRET_SERIALNUMBER:
			{
				wstring serial;
				serial.assign(_brdSerialNum.begin(), _brdSerialNum.end());
				wcscpy_s(str, size, serial.c_str());
				ret = TRUE;
			}
			break;
		case PARAM_TURRET_FIRMWAREVERSION:
			{
				wstring firmware;
				firmware.assign(_brdFirmwareRev.begin(), _brdFirmwareRev.end());
				wcscpy_s(str, size, firmware.c_str());
				ret = TRUE;
			}
			break;
		default:
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter not implemented");
			}
		}
	}
	return ret;
}

long ThorObjectiveChanger::GetParamBuffer(const long paramID, char * buffer, long size)
{
	return FALSE;
}

long ThorObjectiveChanger::PreflightPosition()
{
	return TRUE;
}

long ThorObjectiveChanger::SetupPosition()
{
	return TRUE;
}

//Get Value should handle an error case and return false, in case ReadData returns with error
long ThorObjectiveChanger::GetValue(MOVER_CMD_TYPE type)
{
	_serialPort.SendData((const UCHAR*)_cmdTemplates[type].cmdTstring, _cmdTemplates[type].len);
	Sleep(100);
	UCHAR* readBack = 0;
	switch(type)
	{
	case GET_BRD_INFO:
		readBack = (UCHAR*)malloc(GET_BRD_INFO_BUF_SIZE + 6);
		ZeroMemory(readBack, GET_BRD_INFO_BUF_SIZE + 6);
		if(0 == _serialPort.ReadData(readBack, GET_BRD_INFO_BUF_SIZE + 6))
		{
			//failed to communicate with the device
			return FALSE;
		}
		break;
	case GET_MTR_STAT:
		readBack = (UCHAR*)malloc(GET_MTR_STAT_BUF_SIZE);
		if(0 == _serialPort.ReadData(readBack, GET_MTR_STAT_BUF_SIZE))
		{
			//failed to communicate with the device
			return FALSE;
		}
		break;
	case GET_POS_INFO:
		readBack = (UCHAR*)malloc(GET_POS_INFO_BUF_SIZE);
		if(0 == _serialPort.ReadData(readBack, GET_POS_INFO_BUF_SIZE))
		{
			//failed to communicate with the device
			return FALSE;
		}
		break;
	}
	ParseReadback(type, readBack);
	free(readBack);
	return TRUE;
}

long ThorObjectiveChanger::ParseReadback(MOVER_CMD_TYPE type, UCHAR* rb)
{
	switch(type)
	{
	case GET_BRD_INFO:
		//char tmpInt[3];
		char tmp;	
		_brdID = "";
		for(int i = 10; i < 18; i++)
		{
			tmp =  (char)rb[i];
			_brdID += tmp;
		}
		/*_brdID += '-';
		swprintf(tmp, sizeof(wchar_t)*3, L"%c", rb[84]);
		_brdID += (wstring)tmp;*/

		/*_brdFirmwareRev = "";
		for(int i = 22; i > 19; i--)
		{
		tmpInt = (int)rb[i]; 
		_brdFirmwareRev += rb[i]);
		_brdFirmwareRev += ".";
		}*/

		_brdFirmwareRev = string("") + to_string(rb[22]) + '.' + to_string(rb[21]) + '.' + to_string(rb[20]);

		_brdSerialNum = "";
		for(int i = 0; i < 17; i ++)
		{
			tmp = (char)rb[24 + i];
			_brdSerialNum += tmp;
			//_brdSerialString += (wstring)tmp;
			//if(i%4 == 3)
			//	_brdSerialNum += ' ';
		}
		break;
	case GET_MTR_STAT:
		_rawDriverCount = *(int*)(rb + 8);
		_umDriverCount = UmDriverCountConv * _rawDriverCount;
		_rawEncoderCount = *(int*)(rb + 12);
		_umEncoderCount = UmEncoderCountConv * _rawEncoderCount;
		_mtrStat.upperLimit = rb[16] & 0x01;
		_mtrStat.lowerLimit = (rb[16] >> 1) & 0x01;
		_mtrStat.homed = (rb[17] >> 2) & 0x01;
		_mtrStat.homing = ((rb[17] & 0x03) != 0) ? 1 : 0;
		_mtrStat.moving_cw = ((rb[16] >> 4) == 1) ? 1 : 0;
		_mtrStat.moving_ccw = ((rb[16] >> 5) ==1) ? 1: 0;
		_mtrStat.collision = ((rb[17] >> 6) == 1) ? 1: 0; 
		break;
	case GET_POS_INFO:;
		_pos1 = *(int*)(rb + 10);
		_pos2 = *(int*)(rb + 14);
		break;
	}

	return TRUE;
}

long ThorObjectiveChanger::SetValue(MOVER_CMD_TYPE type, ULONG val1, ULONG val2)
{
	long ret = FALSE;
	if(_serialPort.IsOpened())
	{
		ULONG tmp;
		UCHAR* cmd =  (UCHAR*)malloc(_cmdTemplates[type].len);
		memcpy(cmd, _cmdTemplates[type].cmdTstring, _cmdTemplates[type].len);
		switch(type)
		{
		case SET_JOG_SIZE:
			memcpy((cmd + SET_JOG_SIZE_OFFSET), &val1, sizeof(ULONG));
			break;
			//case FIRMWARE_UPDATE:
			//	memcpy((cmd + SET_FIRMWARE_UPDATE_OFFSET), &val1, sizeof(ULONG));
			//	break;
		case SET_POS_1_2:
		case GOTO_POS_1:
		case GOTO_POS_2:
			tmp = (val1 != 0) ? val1 : _pos1;
			memcpy((cmd + SET_POS_1_OFFSET), &tmp, sizeof(ULONG));
			tmp = (val2 != 0) ? val2 : _pos2;
			memcpy((cmd + SET_POS_2_OFFSET), &tmp, sizeof(ULONG));
			break;
		}

		ret = _serialPort.SendData(cmd, _cmdTemplates[type].len);
		free(cmd);
	}	
	return ret;
}

long ThorObjectiveChanger::StartPosition()
{
	const long HOME_SLEEP_TIME_MS = 5000;
	const long MOVE_POS_SLEEP_TIME_MS = 3000;
	if(_deviceDetected)
	{
		_status = IDevice::STATUS_BUSY;
		if(_moverLoc_B)
		{
			switch(_moverLoc)
			{
			case MOVER_LOCATION::HOME:
				_homed = 0;
				SetValue(GOTO_HOME);
				Sleep(HOME_SLEEP_TIME_MS);
				SetValue(GOTO_POS_2);
				//Sleep(MOVE_POS_SLEEP_TIME_MS);
				_homed = 1;
				break;
			case MOVER_LOCATION::POS1:
				//read the current location. home if needed
				//it is safer to home here since the Z Stage should
				//be in a retracted location
				RefreshChangerCurrentPos();

				if(!_mtrStat.homed || 1 != _homed )
				{
					_homed = 0;
					SetValue(GOTO_HOME);
					Sleep(HOME_SLEEP_TIME_MS);
					SetValue(GOTO_POS_2);
					//Sleep(MOVE_POS_SLEEP_TIME_MS);
					_homed = 1;
				}
				// Check for first use, if the user stopped during initial homing, do not move to the position
				// Make sure user can only home after the device has been stopped
				SetValue(GOTO_POS_1);
				break;
			case MOVER_LOCATION::POS2:
				//read the current location. home if needed
				//it is safer to home here since the Z Stage should
				//be in a retracted location
				RefreshChangerCurrentPos();

				if(!_mtrStat.homed || 1 != _homed )
				{
					_homed = 0;
					SetValue(GOTO_HOME);
					Sleep(HOME_SLEEP_TIME_MS);
					SetValue(GOTO_POS_2);
					//Sleep(MOVE_POS_SLEEP_TIME_MS);
					_homed = 1;
				}
				// Check for first use, if the user stopped during initial homing, do not move to the position
				// Make sure user can only home after the device has been stopped
				SetValue(GOTO_POS_2);
				break;
			}
			_moverLoc_B = FALSE;
		}
		_status = IDevice::STATUS_READY;
		return TRUE;
	}
	return FALSE;
}

long ThorObjectiveChanger::StatusPosition(long &status)
{
	status = _status;
	return TRUE;
}

long ThorObjectiveChanger::ReadPosition(DeviceType deviceType, double &pos)
{
	return TRUE;
}

long ThorObjectiveChanger::PostflightPosition()
{
	return TRUE;
}

void ThorObjectiveChanger::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

long ThorObjectiveChanger::GetLastErrorMsg(wchar_t *msg, long size)
{
	wcsncpy_s(msg, size, _errMsg, MSG_SIZE);
	return TRUE;
}
