// ThorBCMPA.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Serial.h"
#include "ThorBCMPA2.h"
#include "ThorBCMPA2XML.h"
#include "Strsafe.h"
#include "ParamInfo.h"
#include <string>
#include <math.h>
#include <iomanip>

#define TOTAL_COUNTS_PER_MOTOR_REVOLUTION 345855.0

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

/// <summary>
/// The message
/// </summary>
wchar_t message[256];
const string ThorBCMPA2::_deviceSignature[DEVICE_NUM] = {"PA2"};

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
/// Prevents a default instance of the <see cref="ThorBCMPA"/> class from being created.
/// </summary>
ThorBCMPA2::ThorBCMPA2():WAIT_TIME_BETWEEN_SEND_COMMANDS(5)
{
	_errMsg[0] = NULL; 
	_deviceDetected[0] = FALSE;
	_useShutter[0] = FALSE;
	for (int i=0; i<DEVICE_NUM; i++)
	{
		_deviceDetected[i+1] = FALSE;
		_useShutter[i] = FALSE;
		_settingsSerialNumber[i] = "NA";
	}
	_timeOutTime = 10000;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorBCMPA"/> class.
/// </summary>
ThorBCMPA2::~ThorBCMPA2()
{
	_instanceFlag = false;
	_errMsg[0] = 0;	
}

bool ThorBCMPA2::_instanceFlag = false;
auto_ptr<ThorBCMPA2> ThorBCMPA2::_single (new ThorBCMPA2());
CritSect ThorBCMPA2::critSec;

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorBCMPA *.</returns>
ThorBCMPA2* ThorBCMPA2::getInstance()
{
	Lock lock(critSec);

	if(!_instanceFlag)
	{
		_single.reset(new ThorBCMPA2());
		_instanceFlag = true;
	}
	return _single.get();
}

/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="deviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorBCMPA2::FindDevices(long &deviceCount)
{
	Lock lock(critSec);
	long portID[DEVICE_NUM];
	long baudRate=0;
	double zero=0;
	deviceCount=0;
	long ret = FALSE;
	for (int i = 0; i <= DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
	}

	for(int i=0; i<DEVICE_NUM; i++)
	{
		try
		{
			//Get portID, etc from hardware ThorBCMPASettings.xml
			auto_ptr<ThorBCMPA2XML> pSetup(new ThorBCMPA2XML());
			pSetup->GetConnection(_deviceSignature[i], portID[i],baudRate,_settingsSerialNumber[i],_useShutter[i]);			
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorBCMPASettings.xml file");
		}
		if(FALSE == _serialPort[i].Open(portID[i], baudRate))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorBCMPA SelectDevice could not open serial port");
			LogMessage(message);
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBCMPA SelectDevice could not open serial port or configuration file is not avaible.");
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
long ThorBCMPA2::SelectDevice(const long device)
{
	Lock lock(critSec);
	long ret = FALSE;
	long portID=0;
	long baudRate=0;
	double zero=0;

	//Check if any device detected before continuing
	if(_deviceDetected[DEVICE_NUM]==FALSE) 
	{
		return FALSE;
	}

	//Reset detected devices
	for (int i = 0; i < DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
	}

	for(int i=0; i<DEVICE_NUM; i++)
	{
		try
		{				
			auto_ptr<ThorBCMPA2XML> pSetup(new ThorBCMPA2XML());
			//Get portID, etc from  ThorBCMPASettings.xml
			pSetup->GetConnection(_deviceSignature[i], portID,baudRate,_settingsSerialNumber[i],_useShutter[i]);

			if(FALSE == _serialPort[i].Open(portID, baudRate))
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorBCMPA SelectDevice could not open serial port");
				LogMessage(message);
				StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBCMPA SelectDevice could not open serial port or configuration file is not avaible.");
			}
			else 
			{	
				_deviceDetected[DEVICE_NUM] = _deviceDetected[i] = TRUE;			
				ret = TRUE;
			}
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorBCMPASettings.xml file");
			return ret;
		}
	}

	try
	{		
		auto_ptr<ThorBCMPA2XML> pSetup(new ThorBCMPA2XML());
		//Get Settings
		pSetup->GetTimeOut(_timeOutTime);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to load timeOut from ThorBCMPASettings.xml file");
	}
	BuildParamTable();
	return ret;
}

/// <summary>
/// Verifies the serial numbers.
/// </summary>
/// <param name="portId">The port identifier.</param>
/// <returns>long.</returns>
long ThorBCMPA2::VerifySerialNumbers(long portId[])
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
			string mismatchedPortsText = "The ports for the Flipper Mirrors are mismatched.";
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
						auto_ptr<ThorBCMPA2XML> pSetup(new ThorBCMPA2XML());
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
long ThorBCMPA2::GetDeviceSerialNumber(long deviceIndx, string &deviceSerialNumber)
{
	if (TRUE == _deviceDetected[deviceIndx])
	{
		std::vector<unsigned char> commandBytes;
		//build command to request info from the device
		unsigned char commandBytesExpansion1[] = { 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 }; //Request Info
		commandBytes.assign(commandBytesExpansion1, commandBytesExpansion1 + sizeof(commandBytesExpansion1)/sizeof(commandBytesExpansion1[0]));
		string cmdStr(commandBytes.begin(),commandBytes.end());
		_serialPort[deviceIndx].SendData((const unsigned char*)(cmdStr.c_str()), static_cast<int>(cmdStr.size()));		

		Sleep(5); //Determined by testing... Shorter sleep time may cause bad communications

		unsigned char buf[100];
		memset(buf, 0, sizeof(buf));
		_serialPort[deviceIndx].ReadData(buf, 100);

		//When requested for info the device returns a 90 byte message
		//the serial number is found in bytes 6-9 and 24-35 (128-bit number)
		unsigned char serialBytes[16] = {buf[6],buf[7],buf[8],buf[9],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],buf[30],buf[31],buf[32],buf[33],buf[34],buf[35]};		
		unsigned long serialNumber = 0;

		//Because doubles are used to transfer parameters between the devices and the higher level code
		//and this serial number is 128-bit long, the number will be devided in 8 16-bit numbers
		//and added before the number is sent back. The new number will be used as an identifier
		//for the device.
		for(int i=0; i<8; i++)
		{
			serialNumber+= serialBytes[i*2] | serialBytes[i*2+1] << 8;
		}

		//Serial Numbers for the BCMPA must always be formed by 10 digits
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
void ThorBCMPA2::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorBCMPA2::TeardownDevice()
{	
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
long ThorBCMPA2::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
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
long ThorBCMPA2::SetParam(const long paramID, const double param)
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

	StringCbPrintfW(message,MSG_SIZE,L"ThorBCMPA SetParam failed. paramID: %d", paramID);
	LogMessage(message);
	return FALSE;
}

/// <summary>
/// Rounds the specified number.
/// </summary>
/// <param name="number">The number.</param>
/// <param name="decimals">The decimals.</param>
/// <returns>double.</returns>
double ThorBCMPA2::Round(double number, int decimals)
{
	double decP = std::pow(10,decimals);
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
long ThorBCMPA2::GetParam(const long paramID, double &param)
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
				case PARAM_POWER_POS_CURRENT:
					{		

						//Retrieve the current position from the device
						std::map<long, ParamInfo*>::iterator itmap = _tableParams.find(PARAM_POWER_POS);

						if(itmap != _tableParams.end())
						{		
							std::vector<unsigned char> cmd = _tableParams[paramID]->GetCmdBytes();

							if(TRUE == ExecuteCmd(paramID, cmd, ret)) //execute and parse the current position from read back
							{
								//if the parameter requires conversion
								if(_tableParams[paramID]->GetNeedConvert())
								{
									//offset using the zero position
									ret -= _tableParams[PARAM_POWER_ZERO_POS]->GetParamVal();

									void (*myFunctionPointer)(long, double, double&) = NULL;

									double convVal = 0.0;
									myFunctionPointer = (funcConvert)(_tableParams[paramID]->GetConvertFunc());

									//conver to the GUI value
									if(myFunctionPointer != NULL)
									{
										const long DIRECTION_DEVICE_TO_GUI = 1;

										(*myFunctionPointer)(DIRECTION_DEVICE_TO_GUI,ret,convVal);

										param = convVal;
									}

								}

								//Previously when the user would repeate a send position
								//and move the joystick between sends the device would not move.
								//This was due to the storing of the last send position and comparing
								//it to the current position as a test for. whether to not send the position
								//on to the device.
								//
								//To ensure joystick position values are considered in the comparision
								//overwrite the last sent position any time the position is queried
								if (NULL != _tableParams[PARAM_POWER_POS])
								{
									_tableParams[PARAM_POWER_POS]->UpdateParam(param);
									_tableParams[PARAM_POWER_POS]->UpdateParam_C();		
								}
							}
							return TRUE;
						}
					}
					break;
				case PARAM_POWER2_POS_CURRENT:
					{		

						//Retrieve the current position from the device
						std::map<long, ParamInfo*>::iterator itmap = _tableParams.find(PARAM_POWER2_POS);

						if(itmap != _tableParams.end())
						{		
							std::vector<unsigned char> cmd = _tableParams[paramID]->GetCmdBytes();

							if(TRUE == ExecuteCmd(paramID, cmd, ret)) //execute and parse the current position from read back
							{
								//if the parameter requires conversion
								if(_tableParams[paramID]->GetNeedConvert())
								{
									//offset using the zero position
									ret -= _tableParams[PARAM_POWER2_ZERO_POS]->GetParamVal();

									void (*myFunctionPointer)(long, double, double&) = NULL;

									double convVal = 0.0;
									myFunctionPointer = (funcConvert)(_tableParams[paramID]->GetConvertFunc());

									//conver to the GUI value
									if(myFunctionPointer != NULL)
									{
										const long DIRECTION_DEVICE_TO_GUI = 1;

										(*myFunctionPointer)(DIRECTION_DEVICE_TO_GUI,ret,convVal);

										param = convVal;
									}

								}

								//Previously when the user would repeate a send position
								//and move the joystick between sends the device would not move.
								//This was due to the storing of the last send position and comparing
								//it to the current position as a test for. whether to not send the position
								//on to the device.
								//
								//To ensure joystick position values are considered in the comparision
								//overwrite the last sent position any time the position is queried
								if (NULL != _tableParams[PARAM_POWER2_POS])
								{
									_tableParams[PARAM_POWER2_POS]->UpdateParam(param);
									_tableParams[PARAM_POWER2_POS]->UpdateParam_C();		
								}
							}
							return TRUE;
						}
					}
					break;
				case PARAM_POWER_SERIALNUMBER:
				case PARAM_POWER2_SERIALNUMBER:
				case PARAM_POWER3_SERIALNUMBER:
					{
						return ExecuteCmd(paramID, _tableParams[paramID]->GetCmdBytes(), param);
					}
					break;
				case PARAM_CONNECTION_STATUS:
					{
						param = (_deviceDetected[DEVICE_NUM]) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
						return TRUE;
					}
					break;
				case PARAM_POWER2_ENCODER_POS:
					{
						//Retrieve the current position from the device
						return ExecuteCmd(paramID, _tableParams[paramID]->GetCmdBytes(), param);
					}
					break;
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
long ThorBCMPA2::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorBCMPA2::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorBCMPA2::SetParamString(const long paramID, wchar_t* str)
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
long ThorBCMPA2::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorBCMPA2::StartPosition()
{
	//iterate through map and set the parameters in reverse order because we want the position to be set at last
	for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		if(((iter->second)->GetParamAvailable() == TRUE) && ((iter->second)->GetParamReadOnly() == FALSE) && ((iter->second)->GetParamBool() == TRUE))
		{	
			ExecuteCmd(iter->second); //no need to parse read back
			(iter->second)->UpdateParam_C();
			StringCbPrintfW(message,MSG_SIZE,L"StartPosition succeeded at paramID: %d",(iter->second)->GetParamID());
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
long ThorBCMPA2::ExecuteCmd(ParamInfo* pParamInfo)
{
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();
	long paramID = pParamInfo->GetParamID();	
	double readBackVal = -1;


	double pVal = pParamInfo->GetParamVal();	
	long p = static_cast<long>(pVal);
	double pZero = pParamInfo->GetParamZero();

	//if a conversion is needed
	if(pParamInfo->GetNeedConvert())
	{
		void (*myFunctionPointer)(long, double, double&) = NULL;

		double ret = 0.0;
		myFunctionPointer = (funcConvert)(pParamInfo->GetConvertFunc());

		if(myFunctionPointer != NULL)
		{
			const long DIRECTION_GUI_TO_DEVICE = 0;

			(*myFunctionPointer)(DIRECTION_GUI_TO_DEVICE,pVal,ret);

			p = static_cast<long>(ret + pZero);
		}
	}

	switch(pParamInfo->GetParamID())
	{
	case IDevice::PARAM_SHUTTER_POS:
		{
			if(TRUE == _useShutter[0])
			{
				//shutter command is a short character sequence
				const long SHUTTER_OPEN = 0;

				cmd[3] = (SHUTTER_OPEN == p) ? 1 : 2;
				ExecuteCmd(paramID, cmd, readBackVal);

				cmd[0] = 0x59;
				cmd[1] = 0x08;
				cmd[3] = (SHUTTER_OPEN == p) ? 0 : 0xFF;
				ExecuteCmd(paramID, cmd, readBackVal);
			}
		}
		break;
	case IDevice::PARAM_POWER_ZERO_POS:
		{
			//when the zero position is set also set the zero value
			//for the PARAM_POWER_POS Zero member
			std::map<long, ParamInfo*>::iterator itmap = _tableParams.find(PARAM_POWER_POS);

			if(itmap != _tableParams.end() && NULL != _tableParams[PARAM_POWER_POS])
			{
				_tableParams[PARAM_POWER_POS]->UpdateParamZero(pParamInfo->GetParamVal());	
			}
		}
		break;
	case IDevice::PARAM_SHUTTER2_POS:
		{
			if(TRUE == _useShutter[0])
			{
				//shutter command is a short character sequence
				const long SHUTTER_OPEN = 0;

				cmd[3] = (SHUTTER_OPEN == p) ? 1 : 2;
				ExecuteCmd(paramID, cmd, readBackVal);

				cmd[0] = 0x59;
				cmd[1] = 0x08;
				cmd[3] = (SHUTTER_OPEN == p) ? 0 : 0xFF;
				ExecuteCmd(paramID, cmd, readBackVal);
			}
		}
		break;
	case IDevice::PARAM_POWER2_ZERO_POS:
		{
			//when the zero position is set also set the zero value
			//for the PARAM_POWER_POS Zero member
			std::map<long, ParamInfo*>::iterator itmap = _tableParams.find(PARAM_POWER2_POS);

			if(itmap != _tableParams.end() && NULL != _tableParams[PARAM_POWER2_POS])
			{
				_tableParams[PARAM_POWER2_POS]->UpdateParamZero(pParamInfo->GetParamVal());	
			}
		}
		break;
	default:
		{
			//default cmd string sets the the 8-11 bytes
			if(cmd.size() >= 12)
			{
				cmd[8] = p & 0xFF;
				cmd[9] = (p>>8) & 0xFF;
				cmd[10] = (p>>16) & 0xFF;
				cmd[11] = (p>>24) & 0xFF;
			}
			ExecuteCmd(paramID, cmd, readBackVal);
		}
	}
	return TRUE;
}

/// <summary>
/// Waits for rotation complete.
/// </summary>
/// <param name="deviceIndex">Index of the device.</param>
/// <returns>long.</returns>
long ThorBCMPA2::WaitForRotationComplete(long deviceIndex)
{
	char moving = 0;
	char buf[100];
	const unsigned long MAX_WAIT_FOR_COMPLETION_MS =  static_cast<unsigned long>(_timeOutTime);
	DWORD startTime = GetTickCount();
	do
	{
		unsigned char commandBytesStatus[] = { 0x80, 0x04, 0x02, 0x00, 0x11, 0x01}; 

		_serialPort[deviceIndex].SendData(commandBytesStatus,  sizeof(commandBytesStatus)/sizeof(commandBytesStatus[0]));

		Sleep(WAIT_TIME_BETWEEN_SEND_COMMANDS);

		memset(buf, 0, sizeof(buf));

		if(_serialPort[0].ReadData(buf, 100))
		{
			//if any of the moving status bits are enabled
			//set the state to moving
			moving = ((unsigned char)buf[16]  & 0xF0);
		}
	}
	while((moving != 0)&&((GetTickCount() - startTime) < MAX_WAIT_FOR_COMPLETION_MS));

	return TRUE;
}

/// <summary>
/// Executes the command.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="cmd">The command.</param>
/// <param name="readBackValue">The read back value.</param>
/// <returns>long.</returns>
long ThorBCMPA2::ExecuteCmd(long paramID, std::vector<unsigned char> cmd, double &readBackValue)
{
	Lock lock(critSec);

	string cmdStr(cmd.begin(),cmd.end());

	if(_deviceDetected[0]==FALSE)
	{
		return FALSE;
	}

	switch(paramID)		
	{
	case PARAM_POWER_POS:
		{	
			_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());
			Sleep(WAIT_TIME_BETWEEN_SEND_COMMANDS); //Determined by testing... Shorter sleep time may cause bad communications
			WaitForRotationComplete(0);

		}
		break;
	case PARAM_POWER2_POS:
		{
			_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());
			Sleep(WAIT_TIME_BETWEEN_SEND_COMMANDS);
			WaitForRotationComplete(1);

		}
		break;
	case PARAM_POWER3_POS:
		{
			_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());
			Sleep(WAIT_TIME_BETWEEN_SEND_COMMANDS); //Determined by testing... Shorter sleep time may cause bad communications
			WaitForRotationComplete(2);

		}
		break;
	case PARAM_POWER_POS_CURRENT:
		{
			_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());
			Sleep(WAIT_TIME_BETWEEN_SEND_COMMANDS); //Determined by testing... Shorter sleep time may cause bad communications

			char buf[100];
			memset(buf, 0, sizeof(buf));

			if(_serialPort[0].ReadData(buf, 100))
			{
				readBackValue = ((unsigned char)buf[8] | (unsigned char)buf[9] << 8 | (unsigned char)buf[10] << 16 | (unsigned char)buf[11] << 24);
			}
			else
			{
				return FALSE;
			}

		}
		break;
	case PARAM_POWER2_ENCODER_POS:
	case PARAM_POWER2_POS_CURRENT:
		{
			_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());
			Sleep(WAIT_TIME_BETWEEN_SEND_COMMANDS); //Determined by testing... Shorter sleep time may cause bad communications

			char buf[100];
			memset(buf, 0, sizeof(buf));

			if(_serialPort[0].ReadData(buf, 100))
			{
				readBackValue = ((unsigned char)buf[8] | (unsigned char)buf[9] << 8 | (unsigned char)buf[10] << 16 | (unsigned char)buf[11] << 24);
			}
			else
			{
				return FALSE;
			}

		}
		break;
	case PARAM_SHUTTER_POS:
		{
			if(TRUE == _useShutter[0])
			{
				const long SHUTTER_FULL_OPEN_TIME_MS = 500;
				_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());
				Sleep(SHUTTER_FULL_OPEN_TIME_MS); //Determined by testing... Shorter sleep time may cause bad communications
			}
		}
		break;
	case PARAM_SHUTTER2_POS:
		{
			if(TRUE == _useShutter[0])
			{
				const long SHUTTER_FULL_OPEN_TIME_MS = 500;
				_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());
				Sleep(SHUTTER_FULL_OPEN_TIME_MS); //Determined by testing... Shorter sleep time may cause bad communications
			}
		}
		break;
	case PARAM_POWER_ZERO:
		{
			//the user has asked to set the current location as the zero
			//query the position and set the zeroes
			_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());
			Sleep(WAIT_TIME_BETWEEN_SEND_COMMANDS); //Determined by testing... Shorter sleep time may cause bad communications

			char buf[100];
			memset(buf, 0, sizeof(buf));

			_serialPort[0].ReadData(buf, 100);

			readBackValue = ((unsigned char)buf[8] | (unsigned char)buf[9] << 8 | (unsigned char)buf[10] << 16 | (unsigned char)buf[11] << 24);

			std::map<long, ParamInfo*>::iterator itmap = _tableParams.find(PARAM_POWER_POS);

			if(itmap != _tableParams.end())
			{
				_tableParams[PARAM_POWER_POS]->UpdateParamZero(readBackValue);	
			}

			itmap = _tableParams.find(PARAM_POWER_ZERO_POS);

			if(itmap != _tableParams.end())
			{
				_tableParams[PARAM_POWER_ZERO_POS]->UpdateParam(readBackValue);	
			}

		}
		break;
	case PARAM_POWER2_ZERO:
		{
			//the user has asked to set the current location as the zero
			//query the position and set the zeroes
			_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());
			Sleep(WAIT_TIME_BETWEEN_SEND_COMMANDS); //Determined by testing... Shorter sleep time may cause bad communications

			char buf[100];
			memset(buf, 0, sizeof(buf));

			_serialPort[0].ReadData(buf, 100);

			readBackValue = ((unsigned char)buf[8] | (unsigned char)buf[9] << 8 | (unsigned char)buf[10] << 16 | (unsigned char)buf[11] << 24);

			std::map<long, ParamInfo*>::iterator itmap = _tableParams.find(PARAM_POWER2_POS);

			if(itmap != _tableParams.end())
			{
				_tableParams[PARAM_POWER2_POS]->UpdateParamZero(readBackValue);	
			}

			itmap = _tableParams.find(PARAM_POWER2_ZERO_POS);

			if(itmap != _tableParams.end())
			{
				_tableParams[PARAM_POWER2_ZERO_POS]->UpdateParam(readBackValue);	
			}

		}
		break;
	case PARAM_POWER_SERIALNUMBER:		
		{
			_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());
			Sleep(5); //Determined by testing... Shorter sleep time may cause bad communications
			unsigned char buf[100];
			memset(buf, 0, sizeof(buf));
			if (!_serialPort[0].ReadData(buf, 100))
			{
				return FALSE;
			}

			//When requested for info the device returns a 90 byte message
			//the serial number is found in bytes 6-9 and 24-35 (128-bit number)
			unsigned char serialBytes[16] = {buf[6],buf[7],buf[8],buf[9],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],buf[30],buf[31],buf[32],buf[33],buf[34],buf[35]};		
			unsigned long serialNumber = 0;

			//Because doubles are used to transfer parameters between the devices and the higher level code
			//and this serial number is 128-bit long, the number will be devided in 8 16-bit numbers
			//and added before the number is sent back. The new number will be used as an identifier
			//for the device.
			for(int i=0; i<8; i++)
			{
				serialNumber+= serialBytes[i*2] | serialBytes[i*2+1] << 8;
			}

			readBackValue = serialNumber;
		}
		break;
	case PARAM_POWER2_SERIALNUMBER:		
		{
			_serialPort[0].SendData((const unsigned char*)(cmdStr.c_str()), (int)cmdStr.size());
			Sleep(5); //Determined by testing... Shorter sleep time may cause bad communications
			unsigned char buf[100];
			memset(buf, 0, sizeof(buf));
			if (!_serialPort[0].ReadData(buf, 100))
			{
				return FALSE;
			}

			//When requested for info the device returns a 90 byte message
			//the serial number is found in bytes 6-9 and 24-35 (128-bit number)
			unsigned char serialBytes[16] = {buf[6],buf[7],buf[8],buf[9],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],buf[30],buf[31],buf[32],buf[33],buf[34],buf[35]};		
			unsigned long serialNumber = 0;

			//Because doubles are used to transfer parameters between the devices and the higher level code
			//and this serial number is 128-bit long, the number will be devided in 8 16-bit numbers
			//and added before the number is sent back. The new number will be used as an identifier
			//for the device.
			for(int i=0; i<8; i++)
			{
				serialNumber+= serialBytes[i*2] | serialBytes[i*2+1] << 8;
			}

			readBackValue = serialNumber;
		}
		break;
	}

	return TRUE;
}


/// <summary>
/// Linearizes the sine.
/// </summary>
/// <param name="direction">The direction.</param>
/// <param name="val">The value.</param>
/// <param name="ret">The ret.</param>
void LinearizeSine(long direction, double val, double &ret)
{
	//linearize the sine wave response
	const double AREA_UNDER_CURVE = 2.0;

	const double NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION = 8.0;
	const double MAX_POWER_INPUT_POSITION = 100.0;
	const double MOTOR_CONVERSION_VALUE = MAX_POWER_INPUT_POSITION/(TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION);

	switch(direction)
	{
	case 0://GUI to device
		{
			ret = 2.0 * asin(sqrt(val/MAX_POWER_INPUT_POSITION)) * MAX_POWER_INPUT_POSITION /  PI;
			ret = floor((ret / MOTOR_CONVERSION_VALUE) + 0.5);
		}
		break;

	case 1://device to GUI
		{
			double offset = 0.0;

			if(val > TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION)
			{
				offset = floor(val/(TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION));
				val -= offset * (TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION);
			}
			else if(val < 0)
			{
				offset =  floor(val/(TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION));
				val += -1.0 * offset * (TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION);
			}

			ret = val * MOTOR_CONVERSION_VALUE;

			ret =  offset * MAX_POWER_INPUT_POSITION + MAX_POWER_INPUT_POSITION * pow(sin(ret * PI /(2.0*MAX_POWER_INPUT_POSITION)),2);
		}
		break;
	}

}

/// <summary>
/// Builds the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorBCMPA2::BuildParamTable()
{
	const double MAX_POWER_INPUT_POSITION = 100.0;
	_tableParams.clear();
	//Execution in order of top to bottom of list:
	std::vector<unsigned char> commandBytes;
	commandBytes.push_back(0x00);

	double devType = _useShutter[0] ? POWER_REG2 | SHUTTER : POWER_REG2;

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


	//Position the power device
	unsigned char commandBytesBCMPA0To0[] = { 0x53, 0x04, 0x06, 0x00, 0x91, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 }; //Move BMCPA0 to 0
	commandBytes.assign(commandBytesBCMPA0To0, commandBytesBCMPA0To0 + sizeof(commandBytesBCMPA0To0)/sizeof(commandBytesBCMPA0To0[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_POWER_POS,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_DOUBLE,		//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		TRUE,				//CONVERSION (YES/NO)
		&LinearizeSine,
		0,					//MIN
		MAX_POWER_INPUT_POSITION,	//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER_POS,tempParamInfo));

	//Future code for multiple BCMPA devices

	unsigned char commandBytesBCMPA1To0[] = { 0x53, 0x04, 0x06, 0x00, 0x91, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 }; //Move BMCPA1 to 0
	commandBytes.assign(commandBytesBCMPA1To0, commandBytesBCMPA1To0 + sizeof(commandBytesBCMPA1To0)/sizeof(commandBytesBCMPA1To0[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_POWER2_POS,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_DOUBLE,		//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		TRUE,				//CONVERSION (YES/NO)
		&LinearizeSine,
		0,					//MIN
		MAX_POWER_INPUT_POSITION,	//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER2_POS,tempParamInfo));

	//unsigned char commandBytesBCMPA2To0[] = { 0x53, 0x04, 0x06, 0x00, 0x91, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 }; //Move BMCPA2 to 0
	//commandBytes.assign(commandBytesBCMPA2To0, commandBytesBCMPA2To0 + sizeof(commandBytesBCMPA2To0)/sizeof(commandBytesBCMPA2To0[0]));
	//tempParamInfo = new ParamInfo(	
	//	PARAM_POWER3_POS,	//ID
	//	FALSE,				//VAL
	//	FALSE,				//PARAM C
	//	TRUE,				//PARAM B
	//	TYPE_DOUBLE,		//TYPE
	//	TRUE,				//AVAILABLE
	//	FALSE,				//READ ONLY
	//	TRUE,				//CONVERSION (YES/NO)
	//	&LinearizeSine,
	//	0,					//MIN
	//	MAX_POWER_INPUT_POSITION,	//MAX
	//	0,					//DEFAULT
	//	1,					//Motor ID
	//	commandBytes);		//COMMAND
	//_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER3_POS,tempParamInfo));

	//retrieve the current position of the power device
	unsigned char commandBytesBCMPA0Current[] = { 0x0A, 0x04, 0x00, 0x00, 0x00, 0x00 }; 
	commandBytes.assign(commandBytesBCMPA0Current, commandBytesBCMPA0Current + sizeof(commandBytesBCMPA0Current)/sizeof(commandBytesBCMPA0Current[0]));
	tempParamInfo = new ParamInfo(
		PARAM_POWER_POS_CURRENT,	//ID
		0,					//VAL
		0,					//PARAM C
		TRUE,				//PARAM B
		TYPE_DOUBLE,		//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		TRUE,				//CONVERSION (YES/NO)
		&LinearizeSine,
		0,					//MIN
		MAX_POWER_INPUT_POSITION,	//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER_POS_CURRENT,tempParamInfo));

	unsigned char commandBytesBCMPA1Current[] = { 0x0A, 0x04, 0x00, 0x00, 0x00, 0x00 }; 
	commandBytes.assign(commandBytesBCMPA1Current, commandBytesBCMPA1Current + sizeof(commandBytesBCMPA1Current)/sizeof(commandBytesBCMPA1Current[0]));
	tempParamInfo = new ParamInfo(
		PARAM_POWER2_POS_CURRENT,	//ID
		0,					//VAL
		0,					//PARAM C
		TRUE,				//PARAM B
		TYPE_DOUBLE,		//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		TRUE,				//CONVERSION (YES/NO)
		&LinearizeSine,
		0,					//MIN
		MAX_POWER_INPUT_POSITION,	//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER2_POS_CURRENT,tempParamInfo));

	//retrieve the current position of the power device
	unsigned char commandBytesBCMPA1Encoder[] = { 0x0A, 0x04, 0x00, 0x00, 0x00, 0x00 }; 
	commandBytes.assign(commandBytesBCMPA1Encoder, commandBytesBCMPA1Encoder + sizeof(commandBytesBCMPA1Encoder)/sizeof(commandBytesBCMPA1Encoder[0]));
	tempParamInfo = new ParamInfo(
		PARAM_POWER2_ENCODER_POS,	//ID
		0,					//VAL
		0,					//PARAM C
		TRUE,				//PARAM B
		TYPE_DOUBLE,		//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		NULL,				//&LinearizeSine,
		0,					//MIN
		MAX_POWER_INPUT_POSITION,	//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER2_ENCODER_POS,tempParamInfo));

	//Establish a zero at the current location for the power device
	unsigned char commandBytesBCMPA0Zero[] = {  0x0A, 0x04, 0x00, 0x00, 0x00, 0x00 }; 
	commandBytes.assign(commandBytesBCMPA0Zero, commandBytesBCMPA0Zero + sizeof(commandBytesBCMPA0Zero)/sizeof(commandBytesBCMPA0Zero[0]));
	tempParamInfo = new ParamInfo(
		PARAM_POWER_ZERO,	//ID
		0,					//VAL
		0,					//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		NULL,
		0,					//MIN
		1,					//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER_ZERO,tempParamInfo));

	unsigned char commandBytesBCMPA1Zero[] = {  0x0A, 0x04, 0x00, 0x00, 0x00, 0x00 }; 
	commandBytes.assign(commandBytesBCMPA1Zero, commandBytesBCMPA1Zero + sizeof(commandBytesBCMPA1Zero)/sizeof(commandBytesBCMPA1Zero[0]));
	tempParamInfo = new ParamInfo(
		PARAM_POWER2_ZERO,	//ID
		0,					//VAL
		0,					//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		NULL,
		0,					//MIN
		1,					//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER2_ZERO,tempParamInfo));

	//Position the shutter device
	unsigned char commandBytesBCMPA0Shutter[] = { 0xCB, 0x04, 0x00, 0x01, 0x00, 0x00}; 
	commandBytes.assign(commandBytesBCMPA0Shutter, commandBytesBCMPA0Shutter + sizeof(commandBytesBCMPA0Shutter)/sizeof(commandBytesBCMPA0Shutter[0]));
	tempParamInfo = new ParamInfo(
		PARAM_SHUTTER_POS,	//ID
		1,					//VAL
		0,					//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		NULL,
		0,					//MIN
		1,					//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_SHUTTER_POS,tempParamInfo));

	unsigned char commandBytesBCMPA1Shutter[] = { 0xCB, 0x04, 0x00, 0x01, 0x00, 0x00}; 
	commandBytes.assign(commandBytesBCMPA1Shutter, commandBytesBCMPA1Shutter + sizeof(commandBytesBCMPA1Shutter)/sizeof(commandBytesBCMPA1Shutter[0]));
	tempParamInfo = new ParamInfo(
		PARAM_SHUTTER2_POS,	//ID
		1,					//VAL
		0,					//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		NULL,
		0,					//MIN
		1,					//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_SHUTTER2_POS,tempParamInfo));

	//Zero position
	commandBytes.clear();
	tempParamInfo = new ParamInfo(
		PARAM_POWER_ZERO_POS,	//ID
		0,					//VAL
		0,					//PARAM C
		TRUE,				//PARAM B
		TYPE_DOUBLE,		//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		NULL,
		-100000,			//MIN
		100000,				//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER_ZERO_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_POWER2_ZERO_POS,	//ID
		0,					//VAL
		0,					//PARAM C
		TRUE,				//PARAM B
		TYPE_DOUBLE,		//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		NULL,
		-100000,			//MIN
		100000,				//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER2_ZERO_POS,tempParamInfo));

	//build table entry for BCMPA0 Serial Number Request
	unsigned char commandBytesBCMPA0_SN[] = { 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 };
	commandBytes.assign(commandBytesBCMPA0_SN, commandBytesBCMPA0_SN + sizeof(commandBytesBCMPA0_SN)/sizeof(commandBytesBCMPA0_SN[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_POWER_SERIALNUMBER,	//ID
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
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER_SERIALNUMBER,tempParamInfo));

	unsigned char commandBytesBCMPA1_SN[] = { 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 };
	commandBytes.assign(commandBytesBCMPA1_SN, commandBytesBCMPA1_SN + sizeof(commandBytesBCMPA1_SN)/sizeof(commandBytesBCMPA1_SN[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_POWER2_SERIALNUMBER,	//ID
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
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_POWER2_SERIALNUMBER,tempParamInfo));

	//connection status
	unsigned char commandBytesConnStat[] = {0x00};
	commandBytes.assign(commandBytesConnStat, commandBytesConnStat + sizeof(commandBytesConnStat)/sizeof(commandBytesConnStat[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_CONNECTION_STATUS,	//ID
		FALSE,						//VAL
		FALSE,						//PARAM C
		FALSE,						//PARAM B
		TYPE_LONG,					//TYPE
		TRUE,						//AVAILABLE
		TRUE,						//READ ONLY
		FALSE,						//CONVERSION (YES/NO)
		FALSE,						//CONVERSION FACTOR
		(double)ConnectionStatusType::CONNECTION_READY,			//MIN
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,		//MAX
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,		//DEFAULT
		-1,							//Motor ID
		commandBytes);				//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTION_STATUS,tempParamInfo));

	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorBCMPA2::SetupPosition()
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
long ThorBCMPA2::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorBCMPA2::StatusPosition(long &status)
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
long ThorBCMPA2::ReadPosition(DeviceType deviceType,double &pos)
{
	return FALSE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorBCMPA2::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorBCMPA2::PreflightPosition()
{
	return TRUE;
}

