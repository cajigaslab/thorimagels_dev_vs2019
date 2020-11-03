// ThorBCM.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Serial.h"
#include "ThorBCM.h"
#include "ThorBCMXML.h"
#include "Strsafe.h"
#include "ParamInfo.h"
#include <string>
#include <math.h>
#include <iomanip>

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

/// <summary>
/// The message
/// </summary>
wchar_t message[256];
const string ThorBCM::_deviceSignature[DEVICE_NUM] = {"GG", "GR", "CAM"};

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
/// Prevents a default instance of the <see cref="ThorBCM"/> class from being created.
/// </summary>
ThorBCM::ThorBCM()
{
	_errMsg[0] = NULL;
	_deviceDetected[0] = FALSE;
	for (int i = 0; i < DEVICE_NUM; i++)
	{
		_deviceDetected[i+1] = FALSE;
		_settingsSerialNumber[i] = "NA";
	}
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorBCM"/> class.
/// </summary>
ThorBCM::~ThorBCM()
{
	_instanceFlag = false;
	_errMsg[0] = 0;	
}

bool ThorBCM::_instanceFlag = false;
auto_ptr<ThorBCM> ThorBCM::_single (new ThorBCM());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorBCM *.</returns>
ThorBCM* ThorBCM::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorBCM());
		_instanceFlag = true;
	}
	return _single.get();
}

/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="deviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorBCM::FindDevices(long &deviceCount)
{
	Lock lock(_critSect);
	long ret = FALSE;	
	long portID[DEVICE_NUM];
	long baudRate=0;
	deviceCount=0;
	for (int i = 0; i <= DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
	}
	
	for(int i=0; i<DEVICE_NUM; i++)
	{
		try
		{
			portID[i] = 0;
			//Get portID, etc from hardware ThorBCMSettings.xml
			auto_ptr<ThorBCMXML> pSetup(new ThorBCMXML());
			pSetup->GetConnection(_deviceSignature[i], portID[i],baudRate, _settingsSerialNumber[i]);			
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorBCMSettings.xml file");
		}
		if(FALSE == _serialPort[i].Open(portID[i], baudRate))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorBCM SelectDevice could not open serial port");
			LogMessage(message);
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBCM SelectDevice could not open serial port or configuration file is not avaible.");
		}
		else
		{
			//_deviceDetected[DEVICE_NUM] works as a flag to indicate some device has been found.
			_deviceDetected[DEVICE_NUM] = _deviceDetected[i] = TRUE;
			deviceCount = 1;
			ret = TRUE;
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
	for(int i=0; i<DEVICE_NUM; i++)
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
long ThorBCM::SelectDevice(const long device)
{
	Lock lock(_critSect);
	long ret = FALSE;
	long portID=0;
	long baudRate=0;

	//Check if any device detected before continuing
	if(FALSE == _deviceDetected[DEVICE_NUM]) 
	{
		return FALSE;
	}

	//Reset detected devices
	for (int i = 0; i < DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
	}
		
	for(int i=0; i< DEVICE_NUM; i++)
	{
		try
		{				
			auto_ptr<ThorBCMXML> pSetup(new ThorBCMXML());
			//Get portID, etc from  ThorBCMSettings.xml
			pSetup->GetConnection(_deviceSignature[i], portID,baudRate, _settingsSerialNumber[i]);
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorBCMSettings.xml file");
			return ret;
		}
		if(FALSE == _serialPort[i].Open(portID, baudRate))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorBCM SelectDevice could not open serial port");
			LogMessage(message);
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBCM SelectDevice could not open serial port or configuration file is not avaible.");
		}
		else 
		{
			_deviceDetected[DEVICE_NUM] = _deviceDetected[i] = TRUE;
			ret = TRUE;
		}
	}
	BuildParamTable();
	return ret;
}

/// <summary>
/// Verifies the serial numbers.
/// </summary>
/// <param name="portId">The port identifier.</param>
/// <returns>long.</returns>
long ThorBCM::VerifySerialNumbers(long portId[])
{
	long ret = TRUE;
	if (TRUE == _deviceDetected[DEVICE_NUM])
	{
		//Get the Serial Number from from each device
		string deviceSerialNumber[DEVICE_NUM];		
		for (int i=0; i<DEVICE_NUM; i++)
		{
			string tempSerialNumber = deviceSerialNumber[i] = "No Number";
			if (TRUE == _deviceDetected[i])
			{
				if (TRUE == GetDeviceSerialNumber(i,tempSerialNumber))
				{
					deviceSerialNumber[i] = tempSerialNumber;
				}
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
			string mismatchedPortsText = "The ports for the BCM Flipper Mirrors are mismatched.";
			for (int i=0; i<DEVICE_NUM; i++)
			{
				if (deviceSerialMatch[i] != settingsSerialMatch[i])
				{
					if (0 <= settingsSerialMatch[i])
					{					
						string message = " The serial number for the ";
						message.append(_deviceSignature[i]);				
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
						auto_ptr<ThorBCMXML> pSetup(new ThorBCMXML());
						pSetup->SetPortID(_deviceSignature[i], portId[i]);			
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
long ThorBCM::GetDeviceSerialNumber(long deviceIndx, string &deviceSerialNumber)
{
	if (TRUE == _deviceDetected[deviceIndx])
	{
		char buf[100];
		string cmd = "/SR\r";
		memset(buf, 0, sizeof(buf));
#pragma warning(push)
#pragma warning(disable:4996)
		size_t len = cmd.copy(buf, cmd.length(), 0);
#pragma warning(pop)
		_serialPort[deviceIndx].SendData((const unsigned char*)(buf), static_cast<int>(len));

		Sleep(150); //Determined by testing... Shorter sleep time may cause bad communications

		memset(buf, 0, sizeof(buf));

		_serialPort[deviceIndx].ReadData(buf, 100);
		string str(buf);
		
		//The Serial number coming from the BCM Flipper Mirrors always comes
		//with a the string "Serial:" before the serial number.
		//Remove this from the coming string
		string preSerialN = "Serial:";
		string::size_type i = str.find(preSerialN);
		if (i != std::string::npos)
		{
			str.erase(i, preSerialN.length());
		}
		else
		{
			//If "Serial:" was not found that means there was a communication error
			//in which case FALSE is returned
			return FALSE;
		}

		//The message from the BCM Flipper Mirrors for the Serial number always ends
		//ends with "\r\n"
		string lineEnd = "\r\n";
		i = str.find(lineEnd);
		if (i != std::string::npos)
		{
			str.erase(i, lineEnd.length());
			str = removeSpaces(str);
		}
		else
		{
			//If "\r\n" was not found that means there was a communication error
			//in which case FALSE is returned
			return FALSE;
		}

		std::ostringstream ss;
		ss << std::setw(10) << std::setfill('0') << str;

		deviceSerialNumber = string(ss.str());
	}

	return TRUE;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorBCM::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorBCM::TeardownDevice()
{	
	//Set LightPaths to their default position before clearing the table of parameters
	SetParam(PARAM_LIGHTPATH_GG, 0);
	SetParam(PARAM_LIGHTPATH_GR, 0);
	SetParam(PARAM_LIGHTPATH_CAMERA, 0);
	PreflightPosition();
	SetupPosition();
	StartPosition();
	PostflightPosition();

	for(int i=0; i<DEVICE_NUM; i++)
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
long ThorBCM::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if((*iter)->GetParamID() == paramID)
		{
			paramType = (*iter)->GetParamType();
			paramAvailable = (*iter)->GetParamAvailable();
			paramReadOnly = (*iter)->GetParamReadOnly();
			paramMin = (*iter)->GetParamMin();
			paramMax = (*iter)->GetParamMax();
			paramDefault = (*iter)->GetParamDefault();
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
long ThorBCM::SetParam(const long paramID, const double param)
{
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if((*iter)->GetParamID() == paramID)
		{
			if(((*iter)->GetParamAvailable() == TRUE) && ((*iter)->GetParamReadOnly() == FALSE))
			{
				if(((*iter)->GetParamMin() <= param) && ((*iter)->GetParamMax() >= param))
				{
					(*iter)->UpdateParam(param);

					return TRUE;
				}
			}
		}
	}

	StringCbPrintfW(message,MSG_SIZE,L"ThorBCM SetParam failed. paramID: %d", paramID);
	LogMessage(message);
	return FALSE;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorBCM::GetParam(const long paramID, double &param)
{
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if((*iter)->GetParamID() == paramID)
		{
			if(((*iter)->GetParamAvailable() != FALSE))
			{
				switch (paramID)
				{
				case PARAM_LIGHTPATH_GG_SERIALNUMBER:
				case PARAM_LIGHTPATH_GR_SERIALNUMBER:
				case PARAM_LIGHTPATH_CAMERA_SERIALNUMBER:
					{
						return ExecuteCmd(paramID, (*iter)->GetCmdBytes(), param);
					}
					break;
				case PARAM_CONNECTION_STATUS:
					param = (TRUE ==_deviceDetected[DEVICE_NUM]) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
					return TRUE;
				default:
					param = (*iter)->GetParamVal();
					return TRUE;
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
long ThorBCM::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorBCM::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorBCM::SetParamString(const long paramID, wchar_t* str)
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
long ThorBCM::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorBCM::StartPosition()
{
	//iterate through map and set the parameters in reverse order because we want the position to be set at last
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if(((*iter)->GetParamAvailable() == TRUE) && ((*iter)->GetParamReadOnly() == FALSE) && ((*iter)->GetParamBool() == TRUE))
		{	
			ExecuteCmd(*iter); //no need to parse read back
			(*iter)->UpdateParam_C();
			StringCbPrintfW(message,MSG_SIZE,L"StartPosition succeeded at paramID: %d",(*iter)->GetParamID());
			LogMessage(message);
		}
	}

	return TRUE;
}

/// <summary>
/// Executes the command.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorBCM::ExecuteCmd(ParamInfo* pParamInfo)
{
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();
	long paramID = pParamInfo->GetParamID();	
	double readBackVal = -1;

	unsigned char p = (unsigned char)pParamInfo->GetParamVal();
	cmd[2] += p;

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
long ThorBCM::ExecuteCmd(long paramID, std::vector<unsigned char> cmd, double &readBackValue)
{
	Lock lock(_critSect);
	long ret = TRUE;
	string cmdStr(cmd.begin(),cmd.end());

	long i = 0;

	switch (paramID)
	{
	case PARAM_LIGHTPATH_GG:
	case PARAM_LIGHTPATH_GG_SERIALNUMBER:
		{
			i = 0;
		}
		break;
	case PARAM_LIGHTPATH_GR:
	case PARAM_LIGHTPATH_GR_SERIALNUMBER:
		{
			i = 1;
		}
		break;
	case PARAM_LIGHTPATH_CAMERA:
	case PARAM_LIGHTPATH_CAMERA_SERIALNUMBER:
		{
			i = 2;
		}
		break;
	}
	
	if(_deviceDetected[i]==TRUE)
	{
		_serialPort[i].SendData((const unsigned char*)(cmdStr.c_str()), static_cast<int>(cmdStr.size()));
		Sleep(300);	

		switch (paramID)
		{
		case PARAM_LIGHTPATH_GG_SERIALNUMBER:
		case PARAM_LIGHTPATH_GR_SERIALNUMBER:
		case PARAM_LIGHTPATH_CAMERA_SERIALNUMBER:
			{
				char buf[100];
				memset(buf, 0, sizeof(buf));

				//Read serial number
				if (!_serialPort[i].ReadData(buf, 100))
				{
					return FALSE;
				}
				string str(buf);
		
				//The Serial number coming from the BCM Flipper Mirrors always comes
				//with a the string "Serial:" before the serial number.
				//Remove this from the coming string
				string preSerialN = "Serial:";
				string::size_type i = str.find(preSerialN);
				if (i != std::string::npos)
				{
					str.erase(i, preSerialN.length());
				}
				else
				{
					//If "Serial" was not found that means there was a communication error
					//in which case FALSE is returned
					return FALSE;
				}

				//The message from the BCM Flipper Mirrors for the Serial number always ends
				//ends with "\r\n"
				string lineEnd = "\r\n";
				i = str.find(lineEnd);
				if (i != std::string::npos)
				{
					str.erase(i, lineEnd.length());
					str = removeSpaces(str);
				}
				else
				{
					//If "\r\n" was not found that means there was a communication error
					//in which case FALSE is returned
					return FALSE;
				}
				stringstream ss(str);
				ss>>readBackValue;
			}
			break;
		}
	}
	else
	{
		ret = FALSE;
	}

	return ret;
}

/// <summary>
/// Builds the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorBCM::BuildParamTable()
{
	_tableParams.clear();
	//Execution in order of top to bottom of list:
	std::vector<unsigned char> commandBytes;
	commandBytes.push_back(0x00);
	ParamInfo* tempParamInfo = new ParamInfo(	
		PARAM_DEVICE_TYPE,	//ID
		LIGHT_PATH,			//VAL
		LIGHT_PATH,			//PARAM C
		FALSE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		LIGHT_PATH,			//MIN
		LIGHT_PATH,			//MAX
		LIGHT_PATH,			//DEFAULT
		-1,					//Motor ID
		commandBytes);		//Command
	_tableParams.push_back(tempParamInfo);


	//build table entries for Galvo-Galvo fliper mirror

	unsigned char commandBytesGGTo0[] = { 0x2f, 0x4d, 0x30, 0x52, 0x0d }; //Hex for "/M0R<cr>" -> Move GG Mirror to position 0
	commandBytes.assign(commandBytesGGTo0, commandBytesGGTo0 + sizeof(commandBytesGGTo0)/sizeof(commandBytesGGTo0[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_GG,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		FALSE,				//MIN
		TRUE,				//MAX
		FALSE,				//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entries for Galvo-Resonant fliper mirror
	unsigned char commandBytesGRTo0[] = { 0x2f, 0x4d, 0x30, 0x52, 0x0d }; //Hex for "/M0R<cr>" -> Move GR Mirror to position 0
	commandBytes.assign(commandBytesGRTo0, commandBytesGRTo0 + sizeof(commandBytesGRTo0)/sizeof(commandBytesGRTo0[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_GR,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		FALSE,				//MIN
		TRUE,				//MAX
		FALSE,				//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entries for Camera fliper mirror
	unsigned char commandBytesCamTo0[] = { 0x2f, 0x4d, 0x30, 0x52, 0x0d }; //Hex for "/M0R<cr>" -> Move Camera Mirror to position 0
	commandBytes.assign(commandBytesCamTo0, commandBytesCamTo0 + sizeof(commandBytesCamTo0)/sizeof(commandBytesCamTo0[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_CAMERA,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		FALSE,				//MIN
		TRUE,				//MAX
		FALSE,				//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entry for GG Flipper Mirror Serial Number Request
	unsigned char commandBytesGG_SN[] = { 0x2f, 0x53, 0x52, 0x0d }; // Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesGG_SN, commandBytesGG_SN + sizeof(commandBytesGG_SN)/sizeof(commandBytesGG_SN[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_GG_SERIALNUMBER,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		0,					//MIN
		MAXUINT32,			//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entry for GR Flipper Mirror Serial Number Request
	unsigned char commandBytesGR_SN[] = { 0x2f, 0x53, 0x52, 0x0d }; // Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesGR_SN, commandBytesGR_SN + sizeof(commandBytesGR_SN)/sizeof(commandBytesGR_SN[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_GR_SERIALNUMBER,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		0,					//MIN
		MAXUINT32,			//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entry for Camera Flipper Mirror Serial Number Request
	unsigned char commandBytesCAM_SN[] = { 0x2f, 0x53, 0x52, 0x0d }; // Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesCAM_SN, commandBytesCAM_SN + sizeof(commandBytesCAM_SN)/sizeof(commandBytesCAM_SN[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_CAMERA_SERIALNUMBER,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		0,					//MIN
		MAXUINT32,			//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entry for the device connection status
	unsigned char commandBytesCON_STA[] = { 0x00}; // Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesCON_STA, commandBytesCON_STA + sizeof(commandBytesCON_STA)/sizeof(commandBytesCON_STA[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_CONNECTION_STATUS,	//ID
		FALSE,					//VAL
		FALSE,					//PARAM C
		TRUE,					//PARAM B
		TYPE_LONG,				//TYPE
		TRUE,					//AVAILABLE
		TRUE,					//READ ONLY
		FALSE,					//CONVERSION (YES/NO)
		FALSE,					//CONVERSION FACTOR
		(double)ConnectionStatusType::CONNECTION_READY,		//MIN
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,	//MAX
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,	//DEFAULT
		1,						//Motor ID
		commandBytes);			//COMMAND
	_tableParams.push_back(tempParamInfo);

	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorBCM::SetupPosition()
{
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if(((*iter)->GetParamAvailable() == TRUE) && ((*iter)->GetParamReadOnly() == FALSE))
		{
			if((*iter)->GetParamVal() != (*iter)->GetParamCurrent())
			{
				(*iter)->SetParamBool(TRUE);
			}
			else
			{
				(*iter)->SetParamBool(FALSE);
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
long ThorBCM::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorBCM::StatusPosition(long &status)
{
	status = IDevice::STATUS_READY;
	return TRUE;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorBCM::ReadPosition(DeviceType deviceType,double &pos)
{
	return FALSE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorBCM::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorBCM::PreflightPosition()
{
	return TRUE;
}

