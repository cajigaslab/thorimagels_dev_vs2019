// ThorEpiTurret.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "ThorEpiTurretXML.h"
#include "ThorEpiTurret.h"

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

/// <summary>
/// convert string to wide string
/// </summary>
/// <param name="s">The s.</param>
/// <returns>wstring.</returns>
wstring s2ws(const std::string& s)
{
	long len;
	long slength = static_cast<long>(s.length()) + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

/// <summary>
/// Prevents a default instance of the <see cref="ThorEpiTurret"/> class from being created.
/// </summary>
ThorEpiTurret::ThorEpiTurret() :
	WAIT_TIME_BETWEEN_SEND_COMMANDS(5),
	_firmwareVersion(L""),
	_serialNumber(L"")
{
	_errMsg[0] = NULL;
	_deviceDetected[0] = FALSE;
	for (int i=0; i<DEVICE_NUM; i++)
	{
		_deviceDetected[i+1] = FALSE;
		_settingsSerialNumber[i] = "NA";
	}
	_timeOutTime = 10000;



}

/// <summary>
/// Finalizes an instance of the <see cref="ThorEpiTurret"/> class.
/// </summary>
ThorEpiTurret::~ThorEpiTurret()
{
	_instanceFlag = false;
	_errMsg[0] = 0;	
}

bool ThorEpiTurret::_instanceFlag = false;
auto_ptr<ThorEpiTurret> ThorEpiTurret::_single (new ThorEpiTurret());
CritSect ThorEpiTurret::critSec;

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorEpiTurret *.</returns>
ThorEpiTurret* ThorEpiTurret::getInstance()
{
	Lock lock(critSec);

	if(!_instanceFlag)
	{
		_single.reset(new ThorEpiTurret());
		_instanceFlag = true;
	}
	return _single.get();
}

/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="deviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorEpiTurret::FindDevices(long &deviceCount)
{
	Lock lock(critSec);
	long portID[DEVICE_NUM];
	long baudRate = 0;
	double zero = 0;
	deviceCount = 0;
	long ret = FALSE;

	for (int i = 0; i <= DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
	}

	for(int i = 0; i < DEVICE_NUM; i++)
	{
		try
		{
			//Get portID, etc from hardware ThorEpiTurretSettings.xml
			auto_ptr<ThorEpiTurretXML> pSetup(new ThorEpiTurretXML());
			pSetup->GetConnection(portID[i], baudRate, _settingsSerialNumber[i]);	
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Unable to locate ThorEpiTurretSettings.xml file");
			LogMessage(_errMsg);
		}

		if(FALSE == _serialPort[i].Open(portID[i], baudRate))
		{
			StringCbPrintfW(_errMsg, MSG_SIZE,L"ThorEpiTurret FindDevice could not open serial port");
			LogMessage(_errMsg);
		}
		else
		{
			//_deviceDetected[DEVICE_NUM] works as a flag to indicate some device has been found.
			ret = _deviceDetected[DEVICE_NUM] = _deviceDetected[i] = TRUE;
			deviceCount++;
		}
	}

	//If any device was detected, determine if the serial number in the 
	//settings file is correct for the dectected device
	if (TRUE == _deviceDetected[DEVICE_NUM])
	{
		if (FALSE == VerifySerialNumbers(portID))
		{
			ret = FALSE;
			deviceCount = 0;
		}
	}

	for(int i = 0; i < DEVICE_NUM; i++)
	{	
		if (TRUE == _deviceDetected[i])
		{
			_serialPort[i].Close();
		}
	}

	return ret;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorEpiTurret::SelectDevice(const long device)
{
	Lock lock(critSec);
	long ret = FALSE;
	long portID = 0;
	long baudRate = 0;

	//Check if any device detected before continuing
	if(_deviceDetected[DEVICE_NUM] == FALSE) 
	{
		return FALSE;
	}

	//Reset detected devices
	for (int i = 0; i < DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
	}

	for(int i = 0; i < DEVICE_NUM; i++)
	{
		try
		{				
			auto_ptr<ThorEpiTurretXML> pSetup(new ThorEpiTurretXML());
			//Get portID, etc from  ThorEpiTurretSettings.xml
			pSetup->GetConnection(portID, baudRate, _settingsSerialNumber[i]);

			if(FALSE == _serialPort[i].Open(portID, baudRate))
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorEpiTurret SelectDevice could not open serial port");
				LogMessage(_errMsg);
			}
			else 
			{	
				ret = _deviceDetected[DEVICE_NUM] = _deviceDetected[i] = TRUE;
			}
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorEpiTurretSettings.xml file");
			LogMessage(_errMsg);
			return ret;
		}
	}

	GetFirmwareVersionAndSerialNumber(_firmwareVersion, _serialNumber);
	BuildParamTable();
	return ret;
}


/// <summary>
/// Verifies the serial numbers.
/// </summary>
/// <param name="portId">The port identifier.</param>
/// <returns>long.</returns>
long ThorEpiTurret::VerifySerialNumbers(long portId[])
{
	long ret = TRUE;
	if (TRUE == _deviceDetected[DEVICE_NUM])
	{
		//Get the Serial Number from from each device
		string deviceSerialNumber[DEVICE_NUM];		
		for (int i = 0; i < DEVICE_NUM; i++)
		{
			deviceSerialNumber[i] = "No Number";
			if (TRUE == _deviceDetected[i])
			{
				GetDeviceSerialNumber(i, deviceSerialNumber[i]);
			}
		}

		//Initialize comparison parameters to -1
		long deviceSerialMatch[DEVICE_NUM];
		long settingsSerialMatch[DEVICE_NUM];
		for (int i=0; i<DEVICE_NUM; i++)
		{
			deviceSerialMatch[i] = -1;
			settingsSerialMatch[i] = -1;
		}

		//Loop through both the settings serial number and the serial number from the device
		//Keep the index of the matching pairs
		long portsMismatched = FALSE;

		for (int i=0; i<DEVICE_NUM; i++)
		{
			long serialNumberFound = FALSE;
			for(int j=0; j<DEVICE_NUM; j++)
			{
				if (_settingsSerialNumber[i] == deviceSerialNumber[j])
				{
					settingsSerialMatch[i] = j;
					deviceSerialMatch[j] = j;
					if (i != j)
					{
						//Check if there is a mismatch between the port numbers and the serial numbers
						portsMismatched = TRUE;
					}
					serialNumberFound = TRUE;
				}
			}
			if (FALSE == serialNumberFound && TRUE == _deviceDetected[i] && "NA" != _settingsSerialNumber[i])
			{
				return FALSE;
			}
		}

		if (TRUE == portsMismatched)
		{		
			//Build the string to display on the textbox asking the user if they want to switch the
			//port numbers to match the serial numbers of each connected device.
			string mismatchedPortsText = "The ports for the Flipper Mirrors are mismatched.";
			for (int i=0; i<DEVICE_NUM; i++)
			{
				if (deviceSerialMatch[i] != settingsSerialMatch[i])
				{
					if (0 <= settingsSerialMatch[i])
					{					
						string message = " The serial number for the ";				
						message.append(" matches the device in port: ");
						stringstream ss;
						ss << portId[settingsSerialMatch[i]];
						message.append(ss.str());
						message.append(".");						
						mismatchedPortsText.append(message);
					}
				}
			}			

			mismatchedPortsText.append(" Do you wish to update the ports to the right number?");
			wstring stemp = s2ws(mismatchedPortsText);
			LPCWSTR result = stemp.c_str();

			//Show the MessageBox to the user.
			//If the User presses YES, replace the portIds with the correct ones
			//In the settings file
			if (IDYES == MessageBox(NULL,result,L"",MB_YESNO))
			{
				//Make a copy of the portId
				long tempPortId[DEVICE_NUM];
				for (int i=0; i<DEVICE_NUM; i++)
				{
					tempPortId[i] = portId[i];
				}

				//Replace the portId with the correct (serial number matching) portId
				for  (int i=0; i<DEVICE_NUM; i++)
				{
					long temp = settingsSerialMatch[i];
					if (0 <= settingsSerialMatch[i])
					{
						long temp = portId[i];
						portId[i] = tempPortId[settingsSerialMatch[i]];
						if (portId[settingsSerialMatch[i]] == tempPortId[settingsSerialMatch[i]])
						{
							portId[settingsSerialMatch[i]] = temp;
						}
					}
				}

				//For each device save the new portId into the settings file
				for(int i=0; i<DEVICE_NUM; i++)
				{
					try
					{
						//Get portID, etc from hardware ThorBCMSettings.xml
						auto_ptr<ThorEpiTurretXML> pSetup(new ThorEpiTurretXML());
						pSetup->SetPortID("SerialPort", portId[i]);			
					}
					catch(...)
					{
						StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorBCMSettings.xml file");
					}		
				}
			}
			else 
			{
				ret = FALSE;
			}
		}
	}

	return ret;
}

/// <summary>
/// Gets the device serial number.
/// </summary>
/// <param name="deviceIndx">The device indx.</param>
/// <param name="deviceSerialNumber">The device serial number.</param>
/// <returns>long.</returns>
long ThorEpiTurret::GetDeviceSerialNumber(long deviceIndx, string &deviceSerialNumber)
{
	if (TRUE == _deviceDetected[deviceIndx])
	{
		//=========================
		//   Get Serial Number
		//=========================

		//Send request and wait for response
		ClearSerialBuffer(_serialPort[deviceIndx]);
		UCHAR cb_GetSerialNum[] = {0xA1, 0x00, 0x00, 0x00, MOBO_ID, HOST_ID};
		_serialPort[deviceIndx].SendData(cb_GetSerialNum, CMD_LENGTH);				
		bool dataToRead = BlockUntilDataArrives(_serialPort[deviceIndx], 90, WAIT_TIME_BETWEEN_SEND_COMMANDS * 5, 1);

		//Copy buffer onto response buffer
		unsigned char rcvBuffer[90];
		memset(&rcvBuffer, 0, sizeof(90));
		_serialPort[deviceIndx].ReadData(rcvBuffer, 90); 

		deviceSerialNumber = string((const char*)(rcvBuffer + 24), 17);
	}

	return TRUE;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorEpiTurret::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorEpiTurret::TeardownDevice()
{	
	for(int i = 0; i < DEVICE_NUM; i++)
	{
		if(_deviceDetected[i] == TRUE)
		{
			_serialPort[i].Close();
			_deviceDetected[i] = FALSE;
		}
	}
	_deviceDetected[DEVICE_NUM] = FALSE;

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
long ThorEpiTurret::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	if (NULL != _tableParams[paramID])
	{
		if(_tableParams[paramID]->GetParamID() == paramID)
		{
			paramType = _tableParams[paramID]->GetParamType();
			paramAvailable = _tableParams[paramID]->GetParamAvailable();
			paramReadOnly = _tableParams[paramID]->GetParamReadOnly();
			paramMin = _tableParams[paramID]->GetParamMin();
			paramMax = _tableParams[paramID]->GetParamMax();
			paramDefault = _tableParams[paramID]->GetParamDefault();
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
long ThorEpiTurret::SetParam(const long paramID, const double param)
{
	if (NULL != _tableParams[paramID])
	{
		if(_tableParams[paramID]->GetParamID() == paramID)
		{
			if((_tableParams[paramID]->GetParamAvailable() == TRUE) && (_tableParams[paramID]->GetParamReadOnly() == FALSE))
			{
				if((_tableParams[paramID]->GetParamMin() <= param) && (_tableParams[paramID]->GetParamMax() >= param))
				{
					_tableParams[paramID]->UpdateParam(param);

					return TRUE;
				}
			}
		}
	}

	StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorEpiTurret SetParam failed. paramID: %d", paramID);
	LogMessage(_errMsg);
	return FALSE;
}

/// <summary>
/// Rounds the specified number.
/// </summary>
/// <param name="number">The number.</param>
/// <param name="decimals">The decimals.</param>
/// <returns>double.</returns>
double ThorEpiTurret::Round(double number, int decimals)
{
	double decP = std::pow(10, decimals);
	double ret;
	if (number < 0.0)
		ret = -std::floor(-number * decP + 0.5) / decP;
	else
		ret = std::floor(number * decP + 0.5) / decP;
	return ret;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorEpiTurret::GetParam(const long paramID, double &param)
{
	double ret = -1;
	if (NULL != _tableParams[paramID])
	{
		if(_tableParams[paramID]->GetParamID() == paramID)
		{
			if((_tableParams[paramID]->GetParamAvailable() == FALSE))
			{
				return FALSE;
			}
			else
			{
				switch(paramID)
				{
				case PARAM_FW_DIC_POS:
					{		

						//Retrieve the current position from the device
						std::map<long, ParamInfo*>::iterator itmap = _tableParams.find(PARAM_FW_DIC_POS);

						if(itmap != _tableParams.end())
						{		
							std::vector<unsigned char> cmd = _tableParams[paramID]->GetCmdBytes();

							if(TRUE == ExecuteCmd(paramID, cmd, param)) //execute and parse the current position from read back
							{
								_tableParams[PARAM_FW_DIC_POS]->UpdateParam(param);
								_tableParams[PARAM_FW_DIC_POS]->UpdateParam_C();		
							}
							return TRUE;
						}
					}
					break;
				case PARAM_FW_DIC_SERIALNUMBER:
					{
						return ExecuteCmd(paramID, _tableParams[paramID]->GetCmdBytes(), param);
					}
				case PARAM_CONNECTION_STATUS:
					{
						param = (_deviceDetected[DEVICE_NUM]) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
						return TRUE;
					}
				default:
					{
						param = static_cast<double>(_tableParams[paramID]->GetParamVal());
						return TRUE;
					}		
				}
			}
		}
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
long ThorEpiTurret::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	return TRUE;
}

/// <summary>
/// Gets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorEpiTurret::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	return TRUE;
}

/// <summary>
/// Sets the parameter string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <returns>long.</returns>
long ThorEpiTurret::SetParamString(const long paramID, wchar_t* str)
{
	return TRUE;
}

/// <summary>
/// Gets the parameter of type string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorEpiTurret::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;
	switch(paramID)
	{
	case PARAM_FW_DIC_FIRMWAREVERSION:
		{
			wcscpy_s(str,size, _firmwareVersion.c_str());
			ret = TRUE;
		}
		break;
	case PARAM_FW_DIC_SERIALNUMBER:
		{
			wcscpy_s(str,size, _serialNumber.c_str());
			ret = TRUE;
		}
	}
	return ret;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorEpiTurret::StartPosition()
{
	// The only thing that gets set is the bootloader mode
	double param = 0;
	if (NULL != _tableParams[PARAM_FW_DIC_FIRMWAREUPDATE])
	{
		if (_tableParams[PARAM_FW_DIC_FIRMWAREUPDATE]->GetParamAvailable())
		{
			std::vector<unsigned char> cmd = _tableParams[PARAM_FW_DIC_FIRMWAREUPDATE]->GetCmdBytes();
			ExecuteCmd(PARAM_FW_DIC_FIRMWAREUPDATE, cmd, param);
		}
	}
	return TRUE;
}

/// <summary>
/// Executes the command.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorEpiTurret::ExecuteCmd(ParamInfo* pParamInfo)
{
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();
	long paramID = pParamInfo->GetParamID();	
	double readBackVal = -1;


	double pVal = pParamInfo->GetParamVal();	
	long p = static_cast<long>(pVal);
	double pZero = pParamInfo->GetParamZero();

	ExecuteCmd(paramID, cmd, readBackVal);
	return TRUE;
}

/// <summary>
/// Executes the command.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="cmd">The command.</param>
/// <param name="readBackValue">The read back value.</param>
/// <returns>long.</returns>
long ThorEpiTurret::ExecuteCmd(long paramID, std::vector<unsigned char> cmd, double &readBackValue)
{
	Lock lock(critSec);

	string cmdStr(cmd.begin(), cmd.end());

	if(_deviceDetected[0]==FALSE)
	{
		return FALSE;
	}

	ClearSerialBuffer(_serialPort[0]);
	_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());

	if (paramID == PARAM_FW_DIC_POS)
	{
		bool dataToRead = BlockUntilDataArrives(_serialPort[0], 10, WAIT_TIME_BETWEEN_SEND_COMMANDS, 1);
	}
	else
	{
		bool dataToRead = BlockUntilDataArrives(_serialPort[0], 10, WAIT_TIME_BETWEEN_SEND_COMMANDS * 5, 1);
	}

	unsigned char buf[BUF_LENGTH];
	memset(buf, 0, sizeof(buf));
	if (!_serialPort[0].ReadData(buf, BUF_LENGTH))
	{
		return FALSE;
	}

	switch(paramID)		
	{
	case PARAM_FW_DIC_POS:
		{
			readBackValue = (double)buf[6];
		}
		break;
	case PARAM_FW_DIC_SERIALNUMBER:		
		{
			readBackValue = stod(string((const char*)(buf + 24), 17), 0);
		}
		break;
	}

	return TRUE;
}

/// <summary>
/// Builds the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorEpiTurret::BuildParamTable()
{
	_tableParams.clear();

	//Execution in order of top to bottom of list:
	std::vector<unsigned char> commandBytes;
	commandBytes.push_back(0x00);

	double devType = FILTER_WHEEL_DIC;

	ParamInfo* tempParamInfo = new ParamInfo(	
		PARAM_DEVICE_TYPE,	//ID
		devType,//VAL
		devType,//PARAM C
		FALSE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		NULL,
		devType,//MIN
		devType,//MAX
		devType,//DEFAULT
		-1,					//Motor ID
		commandBytes);		//Command
	_tableParams.insert(std::pair<long, ParamInfo*>(Params::PARAM_DEVICE_TYPE,tempParamInfo));


	//_commandPhrases[GET_HARDWARE_INFO] = {0x05, 0x00, 0x00, 0x00, MOBO_ID, HOST_ID};
	UCHAR cb_GetSerialNum[] = {0xA1, 0x00, 0x00, 0x00, MOBO_ID, HOST_ID};
	commandBytes.assign(cb_GetSerialNum, cb_GetSerialNum + CMD_LENGTH);
	tempParamInfo = new ParamInfo(
		PARAM_FW_DIC_SERIALNUMBER,	//ID
		0,					//VAL
		0,					//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		NULL,
		0,					//MIN
		LONG_MAX,			//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_FW_DIC_SERIALNUMBER,tempParamInfo));

	UCHAR cb_GetCurrentStat[] = {0x82, 0x04, 0x00, 0x00, MOBO_ID, HOST_ID};
	commandBytes.assign(cb_GetCurrentStat, cb_GetCurrentStat + CMD_LENGTH);
	tempParamInfo = new ParamInfo(
		PARAM_FW_DIC_POS,	//ID
		1,					//VAL
		0,					//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		NULL,
		0,					//MIN
		10,					//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_FW_DIC_POS,tempParamInfo));

	//build table entry for the device connection status
	unsigned char commandBytesCON_STA[] = { 0x00 };
	commandBytes.assign(commandBytesCON_STA, commandBytesCON_STA + sizeof(commandBytesCON_STA)/sizeof(commandBytesCON_STA[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_CONNECTION_STATUS,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		FALSE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		(double)ConnectionStatusType::CONNECTION_READY,	//MIN
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,	//MAX
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,	//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTION_STATUS,tempParamInfo));

	unsigned char commandFirmwareUpdate[] = { 0xA6, 0x00, 0x04, 0x00, MOBO_ID, HOST_ID, 0x00, 0x00, 0x00, 0x00}; //MGMSG_SET_RES_SCAN_OFFSET
	commandBytes.assign(commandFirmwareUpdate, commandFirmwareUpdate + sizeof(commandFirmwareUpdate)/sizeof(commandFirmwareUpdate[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_FW_DIC_FIRMWAREUPDATE,
		FALSE,
		FALSE,
		TRUE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		0,
		FALSE,
		TRUE,
		0,
		FALSE,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_FW_DIC_FIRMWAREUPDATE, tempParamInfo));

	//if this function is called it means the connection has been stablished
	commandBytes.clear();
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(	
		PARAM_FW_DIC_FIRMWAREVERSION,
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		FALSE,
		0,
		0,
		0,
		0,
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_FW_DIC_FIRMWAREVERSION, tempParamInfo));

	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorEpiTurret::SetupPosition()
{
	for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		if(((iter->second)->GetParamAvailable() == TRUE) && ((iter->second)->GetParamReadOnly() == FALSE))
		{
			if((iter->second)->GetParamVal() != (iter->second)->GetParamCurrent())
			{
				(iter->second)->SetParamBool(TRUE);				
			}
			else
			{
				(iter->second)->SetParamBool(FALSE);				
			}
		}
	}
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorEpiTurret::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorEpiTurret::StatusPosition(long &status)
{
	int falsePositiveBuffer = 8;
	bool moving = true;
	unsigned char buf[BUF_LENGTH];
	const long MAX_WAIT_FOR_COMPLETION_MS =  _timeOutTime;
	DWORD startTime = GetTickCount();

	//Request status update
	ClearSerialBuffer(_serialPort[0]);
	UCHAR cb_GetCurrentStat[] = {0x82, 0x04, 0x00, 0x00, MOBO_ID, HOST_ID};
	_serialPort[0].SendData(cb_GetCurrentStat,  CMD_LENGTH);
	bool dataToRead = BlockUntilDataArrives(_serialPort[0], 12, WAIT_TIME_BETWEEN_SEND_COMMANDS * 5, 1);

	memset(buf, 0, sizeof(buf));
	if(_serialPort[0].ReadData(buf, BUF_LENGTH))
	{
		status = (long)buf[0];
	}		

	return TRUE;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorEpiTurret::ReadPosition(DeviceType deviceType,double &pos)
{
	return FALSE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorEpiTurret::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorEpiTurret::PreflightPosition()
{
	return TRUE;
}


/// <summary>
/// Reads from the serial buffer until it is empty, effectively clearing it.
/// </summary>
/// <param name="serialPort">The serial port to monitor</param>
/// <param name="maxTimeInMillis">the maximum amount of time in milliseconds to wait before returning</param>
/// <param name="pollingFrequencyInMillis">the amount of time to sleep between each check for available data. Lower numbers mean less time is wasted once data arrives, but uses more cpu for the duration of the function</param>
/// <returns>true if data arrived, false if timeout is reached</returns>
bool ThorEpiTurret::BlockUntilDataArrives(CSerial & serialPort, int numberOfBytes, long maxTimeInMillis, long pollingFrequencyInMillis)
{
	int totalTimeWaited = 0;
	int  data = 0;

	while( (data = serialPort.ReadDataWaiting()) < numberOfBytes && totalTimeWaited < maxTimeInMillis)
	{
		Sleep(pollingFrequencyInMillis);
		totalTimeWaited += pollingFrequencyInMillis;
	}

	return data > numberOfBytes;
}

/// <summary>
/// Reads from the serial buffer until it is empty, effectively clearing it.
/// </summary>
/// <param name="CSerial">Reference to the serial port</param>
/// <returns>long.</returns>
long ThorEpiTurret::ClearSerialBuffer(CSerial & serialPort)
{
	char empty[BUF_LENGTH];
	while(serialPort.ReadData(empty, BUF_LENGTH));
	return TRUE;
}

/// <summary>
/// Gets the firmware version from the device
/// </summary>
/// <param name="version">The firmware version.</param>
long ThorEpiTurret::GetFirmwareVersionAndSerialNumber(std::wstring &version, std::wstring &serialNum)
{
	const size_t LEN = 6;//cmdStr.copy(buf, cmdStr.length(), 0);
	unsigned char commandBytesZoomPos[LEN] = { 0x05, 0x00, 0x02, 0x00, MOBO_ID, HOST_ID}; //get board info from processor firmware

	_serialPort[0].SendData(commandBytesZoomPos, static_cast<int>(LEN));
	Sleep(50);
	char readBuf[100];
	memset(readBuf, 0, sizeof(readBuf));
	_serialPort[0].ReadData(readBuf, 100);

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



