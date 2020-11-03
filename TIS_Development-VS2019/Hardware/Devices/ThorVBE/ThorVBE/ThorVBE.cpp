// ThorVBE.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Serial.h"
#include "ThorVBE.h"
#include "ThorVBEXML.h"
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
const string ThorVBE::_deviceSignature[BEDevs::DEVICE_NUM] = {"BE1", "BE2", "BE3"};

/// <summary>
/// S2WSs the specified s.
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
/// Prevents a default instance of the <see cref="ThorVBE"/> class from being created.
/// </summary>
ThorVBE::ThorVBE()
{
	for (int i=0; i< BEDevs::DEVICE_NUM; i++)
	{
		_settingsSerialNumber[i] = "NA";
	}

	 _magMin = 100;
	 _magMax = 100;
	 _wavelengthMin = 488;
	 _wavelengthMax = 488;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorVBE"/> class.
/// </summary>
ThorVBE::~ThorVBE()
{
	_instanceFlag = false;
	_errMsg[0] = 0;	
}

bool ThorVBE::_instanceFlag = false;

auto_ptr<ThorVBE> ThorVBE::_single (new ThorVBE());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorVBE *.</returns>
ThorVBE* ThorVBE::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorVBE());
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
long ThorVBE::FindDevices(long &deviceCount)
{
	Lock lock(_critSect);
	long ret = FALSE;
	long portID[BEDevs::DEVICE_NUM];
	long baudRate=0;
	deviceCount=0;	
	for (int i = 0; i <= BEDevs::DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
	}

	for(int i=0; i< BEDevs::DEVICE_NUM; i++)
	{
		try
		{
			portID[i] = 0;
			//Get portID, etc from hardware ThorVBESettings.xml
			auto_ptr<ThorVBEXML> pSetup(new ThorVBEXML());
			pSetup->GetConnection(_deviceSignature[i], portID[i],baudRate,_settingsSerialNumber[i]);			
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorVBESettings.xml file");
		}
		if(FALSE == _serialPort[i].Open(portID[i], baudRate))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorVBE SelectDevice could not open serial port");
			LogMessage(message);
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorVBE SelectDevice could not open serial port or configuration file is not avaible.");
		}
		else
		{	
			//_deviceDetected[DEVICE_NUM] works as a flag to indicate some device has been found.
			ret = _deviceDetected[BEDevs::DEVICE_NUM] = _deviceDetected[i] = TRUE;
			deviceCount++;
		}		
	}

	//If any device was detected, determine if the serial number in the 
	//settings file is correct for the dectected device
	if (TRUE == _deviceDetected[BEDevs::DEVICE_NUM])
	{
		if (FALSE == VerifySerialNumbers(portID))
		{
			ret = FALSE;
			deviceCount = 0;
		}
	}
	for(int i=0; i< BEDevs::DEVICE_NUM; i++)
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
long ThorVBE::SelectDevice(const long device)
{
	Lock lock(_critSect);
	long ret = FALSE;
	long portID=0;
	long baudRate=0;

	//Check if any device detected before continuing
	if(_deviceDetected[BEDevs::DEVICE_NUM]==FALSE)
	{
		return FALSE;
	}

	//Reset detected devices
	for (int i = 0; i < BEDevs::DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
	}
		
	for(int i=0; i< BEDevs::DEVICE_NUM; i++)
	{
		try
		{				
			auto_ptr<ThorVBEXML> pSetup(new ThorVBEXML());
			//Get portID, etc from  ThorVBESettings.xml
			pSetup->GetConnection(_deviceSignature[i], portID,baudRate, _settingsSerialNumber[i]);
			pSetup->GetConfiguration(_magMin, _magMax, _wavelengthMin, _wavelengthMax);
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorVBESettings.xml file");
			return ret;
		}
		if(FALSE == _serialPort[i].Open(portID, baudRate))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorVBE SelectDevice could not open serial port");
			LogMessage(message);
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorVBE SelectDevice could not open serial port or configuration file is not avaible.");
		}
		else 
		{
			////////////Home Device///////////////////
			std::vector<unsigned char> commandBytes;

			//build and send command for Motor 1
			//Motor 1 is done first because its range is larger, and might take longer than motor 0
			unsigned char commandHomeMotor1[] = { 0x43, 0x04, 0x01, 0x00, 0x00, 0x00 }; //Request Info
			commandBytes.assign(commandHomeMotor1, commandHomeMotor1 + sizeof(commandHomeMotor1)/sizeof(commandHomeMotor1[0]));
			string cmdStr1 = string(commandBytes.begin(),commandBytes.end());
			_serialPort[i].SendData((const unsigned char*)(cmdStr1.c_str()), static_cast<int>(cmdStr1.size()));
			Sleep(50);

			//build and send command for Motor 0
			unsigned char commandBytesHomeMotor0[] = { 0x43, 0x04, 0x00, 0x00, 0x00, 0x00 }; //Request Info
			commandBytes.assign(commandBytesHomeMotor0, commandBytesHomeMotor0 + sizeof(commandBytesHomeMotor0)/sizeof(commandBytesHomeMotor0[0]));
			string cmdStr0(commandBytes.begin(),commandBytes.end());
			_serialPort[i].SendData((const unsigned char*)(cmdStr0.c_str()), static_cast<int>(cmdStr0.size()));
			Sleep(10);
			////////////End Home Device/////////////////		
			
			_deviceDetected[BEDevs::DEVICE_NUM] = _deviceDetected[i] = TRUE;
			ret = TRUE;
		}
	}
	//Sleep only when devices where selected, and homed
	if (TRUE == ret)
	{
		Sleep(500);
	}
	BuildParamTable();
	return ret;
}

/// <summary>
/// Verifies the serial numbers.
/// </summary>
/// <param name="portId">The port identifier.</param>
/// <returns>long.</returns>
long ThorVBE::VerifySerialNumbers(long portId[])
{
	long ret = TRUE;
	if (TRUE == _deviceDetected[BEDevs::DEVICE_NUM])
	{
		//Get the Serial Number from from each device
		string deviceSerialNumber[BEDevs::DEVICE_NUM];
		for (int i=0; i< BEDevs::DEVICE_NUM; i++)
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
		long deviceSerialMatch[BEDevs::DEVICE_NUM];
		long settingsSerialMatch[BEDevs::DEVICE_NUM];
		for (int i=0; i< BEDevs::DEVICE_NUM; i++)
		{
			deviceSerialMatch[i] = -1;
			settingsSerialMatch[i] = -1;
		}
		
		//Loop through both the settings serial number and the serial number from the device
		//Keep the index of the matching pairs
		long portsMismatched = FALSE;
		
		for (int i=0; i< BEDevs::DEVICE_NUM; i++)
		{
			long serialNumberFound = FALSE;
			for(int j=0; j< BEDevs::DEVICE_NUM; j++)
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
			string mismatchedPortsText = "The ports for the Variable Beam Expanders are mismatched.";
			for (int i=0; i< BEDevs::DEVICE_NUM; i++)
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
				long tempPortId[BEDevs::DEVICE_NUM];
				for (int i=0; i< BEDevs::DEVICE_NUM; i++)
				{
					tempPortId[i] = portId[i];
				}

				//Replace the portId with the correct (serial number matching) portId
				for  (int i=0; i< BEDevs::DEVICE_NUM; i++)
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
				for(int i=0; i< BEDevs::DEVICE_NUM; i++)
				{
					try
					{
						//Get portID, etc from hardware ThorBCMSettings.xml
						auto_ptr<ThorVBEXML> pSetup(new ThorVBEXML());
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
long ThorVBE::GetDeviceSerialNumber(long deviceIndx, string &deviceSerialNumber)
{
	if (TRUE == _deviceDetected[deviceIndx])
	{
		std::vector<unsigned char> commandBytes;
		//build command to request info from the device
		unsigned char commandBytesExpansion1[] = { 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 }; //Request Info
		commandBytes.assign(commandBytesExpansion1, commandBytesExpansion1 + sizeof(commandBytesExpansion1)/sizeof(commandBytesExpansion1[0]));
		string cmdStr(commandBytes.begin(),commandBytes.end());
		_serialPort[deviceIndx].SendData((const unsigned char*)(cmdStr.c_str()), static_cast<int>(cmdStr.size()));		

		Sleep(50); //Determined by testing... Shorter sleep time may cause bad communications

		unsigned char buf[100];
		memset(buf, 0, sizeof(buf));
		_serialPort[deviceIndx].ReadData(buf, 100);
		Sleep(10); 
		//When requested for info the device returns a 90 byte message
		//bytes 6-9 form a 32-bit serial number
		unsigned char serialBytes[4] = {buf[6], buf[7], buf[8], buf[9]};

		//Convert the 4 bytes into a 32bit integer
		unsigned long serialNumber = ((unsigned char)buf[8] | (unsigned char)buf[9] << 8 | (unsigned char)buf[10] << 16 | (unsigned char)buf[11] << 24);

		//Serial Numbers for the VBE must always be formed by 10 digits
		//if the serial number from the device has less than 10 digits
		//fill with leading 0s
		std::ostringstream ss;
		ss << std::setw(10) << std::setfill('0') << serialNumber;

		deviceSerialNumber = string(ss.str());
	}

	return TRUE;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorVBE::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorVBE::TeardownDevice()
{	
	for(int i=0; i< BEDevs::DEVICE_NUM; i++)
	{
		if(_deviceDetected[i] == TRUE)
		{
			_serialPort[i].Close();
			_deviceDetected[i] = FALSE;
		}
	}
	_deviceDetected[BEDevs::DEVICE_NUM] = FALSE;

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
long ThorVBE::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
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
long ThorVBE::SetParam(const long paramID, const double param)
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

	StringCbPrintfW(message,MSG_SIZE,L"ThorVBE SetParam failed. paramID: %d", paramID);
	LogMessage(message);
	return FALSE;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorVBE::GetParam(const long paramID, double &param)
{
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if((*iter)->GetParamID() == paramID)
		{
			if(((*iter)->GetParamAvailable() != FALSE))
			{
				switch (paramID)
				{
				case Params::PARAM_EXP_SERIALNUMBER:
				case Params::PARAM_EXP_SERIALNUMBER2:
					{
						return ExecuteCmd(paramID, (*iter)->GetCmdBytes(), param);
					}
					break;
				case Params::PARAM_CONNECTION_STATUS:
					param = (_deviceDetected[BEDevs::DEVICE_NUM]) ? ConnectionStatusType::CONNECTION_READY : ConnectionStatusType::CONNECTION_UNAVAILABLE;
					return TRUE;
				default:
					param = static_cast<double>((*iter)->GetParamVal());
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
long ThorVBE::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorVBE::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorVBE::SetParamString(const long paramID, wchar_t* str)
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
long ThorVBE::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorVBE::StartPosition()
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
long ThorVBE::ExecuteCmd(ParamInfo* pParamInfo)
{
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();
	long paramID = pParamInfo->GetParamID();	
	double readBackVal = -1;

	long p = (long)pParamInfo->GetParamVal();
	
	char byteVal[2];

	byteVal[0] = (p>>8) & 0xFF;
	byteVal[1] = p & 0xFF;

	cmd[2] = byteVal[0];
	cmd[3] = byteVal[1];

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
long ThorVBE::ExecuteCmd(long paramID, std::vector<unsigned char> cmd, double &readBackValue)
{
	Lock lock(_critSect);

	string cmdStr(cmd.begin(),cmd.end());

	long i = 0;

	switch (paramID)
	{
	case PARAM_EXP_RATIO:
	case PARAM_EXP_WAVELENGTH:
	case PARAM_EXP_SERIALNUMBER:
		{
			i = 0;;
		}
		break;
	case PARAM_EXP_RATIO2:
	case PARAM_EXP_WAVELENGTH2:
	case PARAM_EXP_SERIALNUMBER2:
		{
			i = 1;
		}
		break;
	}

	if(_deviceDetected[i]==TRUE)
	{
		_serialPort[i].SendData((const unsigned char*)(cmdStr.c_str()), static_cast<int>(cmdStr.size()));
		Sleep(50);

		switch (paramID)
		{
		case PARAM_EXP_SERIALNUMBER:
		case PARAM_EXP_SERIALNUMBER2:
			{
				unsigned char buf[100];
				memset(buf, 0, sizeof(buf));
				if (!_serialPort[i].ReadData(buf, 100))
				{
					return FALSE;
				}
				//When requested for info the device returns a 90 byte message
				//bytes 6-9 form a 32-bit serial number
				unsigned char serialBytes[4] = {buf[6], buf[7], buf[8], buf[9]};

				//Convert the 4 bytes into a 32bit integer
				unsigned long serialNumber = ((unsigned char)buf[8] | (unsigned char)buf[9] << 8 | (unsigned char)buf[10] << 16 | (unsigned char)buf[11] << 24);

				readBackValue = serialNumber;
			}
			break;
		}
	}

	return TRUE;
}

/// <summary>
/// Builds the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorVBE::BuildParamTable()
{
	_tableParams.clear();
	//Execution in order of top to bottom of list:
	std::vector<unsigned char> commandBytes;
	commandBytes.push_back(0x00);
	ParamInfo* tempParamInfo = new ParamInfo(	
		PARAM_DEVICE_TYPE,	//ID
		BEAM_EXPANDER,			//VAL
		BEAM_EXPANDER,			//PARAM C
		FALSE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		BEAM_EXPANDER,			//MIN
		BEAM_EXPANDER,			//MAX
		BEAM_EXPANDER,			//DEFAULT
		-1,					//Motor ID
		commandBytes);		//Command
	_tableParams.push_back(tempParamInfo);


	//build table entries for Expansion 1
	unsigned char commandBytesExpansion1[] = { 0x40, 0x08, 0x00, 0x64, 0x21, 0x01 }; //Move to minimum expansion
	commandBytes.assign(commandBytesExpansion1, commandBytesExpansion1 + sizeof(commandBytesExpansion1)/sizeof(commandBytesExpansion1[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_EXP_RATIO,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		_magMin,				//MIN
		_magMax,				//MAX
		_magMin,				//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);	

	//build table entries for Expansion 2
	unsigned char commandBytesExpansion2[] = { 0x40, 0x08, 0x00, 0x64, 0x21, 0x01 }; //Move to minimum expansion
	commandBytes.assign(commandBytesExpansion1, commandBytesExpansion1 + sizeof(commandBytesExpansion1)/sizeof(commandBytesExpansion1[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_EXP_RATIO2,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		_magMin,				//MIN
		_magMax,				//MAX
		_magMin,				//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);
	
	//build table entries for Wavelength 1
	unsigned char commandBytesWavelength1[] = { 0x43, 0x08, 0x01, 0xE8, 0x21, 0x01 }; //Move to minimum expansion
	commandBytes.assign(commandBytesWavelength1, commandBytesWavelength1 + sizeof(commandBytesWavelength1)/sizeof(commandBytesWavelength1[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_EXP_WAVELENGTH,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		_wavelengthMin,				//MIN
		_wavelengthMax,				//MAX
		_wavelengthMin,				//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entries for Wavelength 2
	unsigned char commandBytesWavelength2[] = { 0x43, 0x08, 0x01, 0xE8, 0x21, 0x01 }; //Move to minimum expansion
	commandBytes.assign(commandBytesWavelength2, commandBytesWavelength2 + sizeof(commandBytesWavelength2)/sizeof(commandBytesWavelength2[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_EXP_WAVELENGTH2,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		_wavelengthMin,				//MIN
		_wavelengthMax,				//MAX
		_wavelengthMin,				//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entry for VBE1 Serial Number Request
	unsigned char commandBytesVBE1_SN[] = { 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 };// Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesVBE1_SN, commandBytesVBE1_SN + sizeof(commandBytesVBE1_SN)/sizeof(commandBytesVBE1_SN[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_EXP_SERIALNUMBER,	//ID
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

	//build table entry for VBE2 Serial Number Request
	unsigned char commandBytesVBE2_SN[] = { 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesVBE2_SN, commandBytesVBE2_SN + sizeof(commandBytesVBE2_SN)/sizeof(commandBytesVBE2_SN[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_EXP_SERIALNUMBER2,	//ID
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

	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(	
		PARAM_CONNECTION_STATUS,//ID
		FALSE,					//VAL
		FALSE,					//PARAM C
		FALSE,					//PARAM B
		TYPE_LONG,				//TYPE
		TRUE,					//AVAILABLE
		TRUE,					//READ ONLY
		FALSE,					//CONVERSION (YES/NO)
		FALSE,					//CONVERSION FACTOR
		CONNECTION_READY,		//MIN
		CONNECTION_UNAVAILABLE,	//MAX
		CONNECTION_UNAVAILABLE,	//DEFAULT
		-1,						//Motor ID
		commandBytes);			//Command
	_tableParams.push_back(tempParamInfo);

	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorVBE::SetupPosition()
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
long ThorVBE::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorVBE::StatusPosition(long &status)
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
long ThorVBE::ReadPosition(DeviceType deviceType,double &pos)
{
	return FALSE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorVBE::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorVBE::PreflightPosition()
{
	return TRUE;
}

