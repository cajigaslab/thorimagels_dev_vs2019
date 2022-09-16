// TopticaiChrome.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include "Serial.h"
#include "math.h"
#include "TopticaiChrome.h"
#include "TopticaiChromeXML.h"
#include "Strsafe.h"
#include <string>
#include <math.h>
#include <iostream>
#include <regex>
#include <cstddef>

#define SLEEP_TIME_40 40
#define SLEEP_TIME_50 50
#define READ_TIMEOUT 1000
#define LASER_WAVELENGTH_MIN 405
#define LASER_WAVELENGTH_MAX 785
#define LASER_POWER_MIN 0
#define LASER_POWER_MAX 100

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

wchar_t message[MSG_SIZE];

/// <summary>
/// Prevents a default instance of the <see cref="TopticaiChrome"/> class from being created.
/// </summary>
TopticaiChrome::TopticaiChrome() 
{
	_errMsg[0] = NULL;
	_deviceDetected = FALSE;
	_readBuffer = nullptr;
	_waveBuffer = nullptr;
	_powerBuffer = nullptr;
	_enableBuffer = nullptr;
	_readyBuffer = nullptr;
	_analogBuffer = nullptr;
	_allEnableBuffer = nullptr;
	if (_readBuffer == nullptr)
	{
		_readBuffer = new char[256];
		memset(_readBuffer, 0, 256); //Initiate the serial communication buffer.
	}
	if (_waveBuffer == nullptr)
	{
		_waveBuffer = new char[256];
		memset(_waveBuffer, 0, 256); //Initiate the laser wavelength buffer.
	}
	if (_powerBuffer == nullptr)
	{
		_powerBuffer = new char[256];
		memset(_powerBuffer, 0, 256); //Initiate the laser power buffer.
	}
	if (_enableBuffer == nullptr)
	{
		_enableBuffer = new char[256];
		memset(_enableBuffer, 0, 256); //Initiate the laser enable buffer.
	}
	if (_readyBuffer == nullptr)
	{
		_readyBuffer = new char[256];
		memset(_readyBuffer, 0, 256); //Initiate the laser connection ready buffer.
	}
	if (_analogBuffer == nullptr) 
	{
		_analogBuffer = new char[256];
		memset(_analogBuffer, 0, 256); //Initialize the laser analog mode buffer.
	}
	if (_allEnableBuffer == nullptr)
	{
		_allEnableBuffer = new char[256];
		memset(_allEnableBuffer, 0, 256); //Initialize the enable all lasers buffer.
	}
}

/// <summary>
/// Finalizes an instance of the <see cref="TopticaiChrome"/> class.
/// </summary>
TopticaiChrome::~TopticaiChrome()
{
	_instanceFlag = false;
}

bool TopticaiChrome::_instanceFlag = false;

auto_ptr<TopticaiChrome> TopticaiChrome::_single(new TopticaiChrome());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>TopticaiChrome *.</returns>
TopticaiChrome* TopticaiChrome::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new TopticaiChrome());
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
long TopticaiChrome::FindDevices(long& deviceCount)
{
	long ret = TRUE;
	deviceCount = 0;

	if (_deviceDetected)
	{
		_serialPort.Close();
		_deviceDetected = FALSE;
	}

	long portID = 0;//62
	long baudRate = 9600;//115200

	try
	{
		auto_ptr<TopticaiChromeXML> pSetup(new TopticaiChromeXML());
		pSetup->GetDeviceConnectionInfo(portID, baudRate);
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Unable to locate TopticaiChromeSettings.xml file");
		return FALSE;
	}

	if (FALSE == _serialPort.Open(portID, baudRate))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"TopticaiChrome FindDevices could not open serial port");
		LogMessage(_errMsg, WARNING_EVENT);
		ret = FALSE;
		_deviceDetected = FALSE;
	}

	if (TRUE == ret) 
	{
		deviceCount = 1;
		_deviceDetected = TRUE;
		_serialPort.Close();
	}
	return ret;
}	

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long TopticaiChrome::SelectDevice(const long device)
{
	Lock lock(_critSect);
	long ret = FALSE;

	//Check if any device detected before continuing
	if (FALSE == _deviceDetected) 
	{
		return FALSE;
	}

	long portID = 0;
	long baudRate = 9600;
	double connectionParam = 0.0;
	//Updates the port ID and baud rate based on the set xml values
	try
	{
		auto_ptr<TopticaiChromeXML> pSetup(new TopticaiChromeXML());
		pSetup->GetDeviceConnectionInfo(portID, baudRate);
	}

	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Unable to locate TopticaiChromeSettings.xml file");
		return FALSE;
	}

	if (FALSE == _serialPort.Open(portID, baudRate))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"TopticaiChrome is unable to open port %d", portID);
		return FALSE;
	}

	BuildParamTable(); //Initiate the Parameter Table

	//Gets the connection status from the laser (when laser is heated and ready)
	GetParam(PARAM_CONNECTION_STATUS, connectionParam);
	if (connectionParam != 1) 
	{
		_deviceDetected = FALSE;
		return FALSE;
	}
	
	return TRUE;
}

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long TopticaiChrome::SetParam(const long paramID, const double param) 
{
	if (NULL != _tableParams[paramID])
	{
		if (_tableParams[paramID]->GetParamID() == paramID)
		{
			if (FALSE == (_tableParams[paramID]->GetParamAvailable()) || (TRUE == _tableParams[paramID]->GetParamReadOnly()))
			{
				return FALSE;
			}
			else if ((_tableParams[paramID]->GetParamMin() <= param) && (_tableParams[paramID]->GetParamMax() >= param))
			{
				_tableParams[paramID]->UpdateParam(param);
				return TRUE;
			}
			else
			{
				wstring message = L"TopticaiChrome SetParam failed. paramID: %d param: " + _tableParams[paramID]->GetParameterString(), paramID;
				StringCbPrintfW(_errMsg, MSG_SIZE, message.c_str());
				LogMessage(_errMsg, INFORMATION_EVENT);
				return FALSE;
			}
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
long TopticaiChrome::GetParam(const long paramID, double& param)
{
	if (NULL != _tableParams[paramID])
	{
		if (_tableParams[paramID]->GetParamID() == paramID)
		{
			if (FALSE == (_tableParams[paramID]->GetParamAvailable()))
			{
				return FALSE;
			}

			if (_tableParams[paramID]->GetCmdGet() != L"")
			{
				ExecuteCmdGet(_tableParams[paramID]->GetCmdGet());
				//Buffer returns a char* so convert to a double
				switch (paramID) 
				{
					//Querying wavelength returns a char array with units at the end - "405 nm" so we only want those first 3 bytes
					case PARAM_LASER1_WAVELENGTH:
					case PARAM_LASER2_WAVELENGTH:
					case PARAM_LASER3_WAVELENGTH:
					case PARAM_LASER4_WAVELENGTH:
					{
						memset(_waveBuffer, 0, 256);
						memcpy(_waveBuffer, _readBuffer+28, 3); //serial port displays format "(param-ref 'laser1:label)\r\n"405 nm"\r\n> " and we need to separate the wavelength
						string waveString = string(_waveBuffer);
						try
						{
							//Separate instances where USB plugged in but laser off (reads empty string)
							if (waveString != "") 
							{
								param = atof(_waveBuffer);
								return TRUE;
							}
							return FALSE;
						}
						catch (...)
						{
							StringCbPrintfW(_errMsg, MSG_SIZE, L"Unable to convert read wavelength to integer");
							return FALSE;
						}
						break;
					}
					//Querying enable state gives a response of #t or #f so turn these into 1 and 0
					case PARAM_LASER1_ENABLE:
					case PARAM_LASER2_ENABLE:
					case PARAM_LASER3_ENABLE:
					case PARAM_LASER4_ENABLE:
					{
						memset(_enableBuffer, 0, 256);
						memcpy(_enableBuffer, _readBuffer+28, 2); //serial port displays format "(param-ref 'laser1:enable)\r\n#t\r\n> " and we need to separate the boolean
						string enableString = string(_enableBuffer);
						if (enableString == "#t")
						{
							param = 1;
							return TRUE;
						}

						else if (enableString == "#f")
						{
							param = 0;
							return TRUE;
						}

						return FALSE;
					}

					//Querying enable state for all lasers give a response of #t or #f so turn these into 1 and 0
					case PARAM_LASER_ALL_ENABLE:
					{
						memset(_allEnableBuffer, 0, 256);
						memcpy(_allEnableBuffer, _allEnableBuffer + 25, 2); //serial port displays format "(param-ref 'all:enable)\r\n#t\r\n> " and we need to separate the boolean
						string allEnableString = string(_allEnableBuffer);
						if (allEnableString == "#t")
						{
							param = 1;
							return TRUE;
						}

						else if (allEnableString == "#f")
						{
							param = 0;
							return TRUE;
						}

						return FALSE;
					}
					//Querying enable state of Analog Mode for all lasers gives a response of #t or #f so turn these into 1 and 0
					case PARAM_LASER_ALL_ANALOG_MODE: //not used right now
					{
						memset(_analogBuffer, 0, 256);
						memcpy(_analogBuffer, _analogBuffer + 30, 2); //serial port displays format "(param-ref 'all:analog-mode)\r\n#t\r\n> " and we need to separate the boolean
						string analogString = string(_analogBuffer);
						if (analogString == "#t")
						{
							param = 1;
							return TRUE;
						}

						else if (analogString == "#f")
						{
							param = 0;
							return TRUE;
						}

						return FALSE;
					}

					//Querying ready status of all lasers (is temperature ready?) gives a response of #t or #f so turn these into 1 and 0
					case PARAM_CONNECTION_STATUS:
					{
						if (_deviceDetected)
						{
							memset(_readyBuffer, 0, 256);
							memcpy(_readyBuffer, _readBuffer + 24, 2); //serial port displays format "(param-ref 'all:ready)\r\n#t\r\n> " and we need to separate the boolean
							string readyString = string(_readyBuffer);
							if (readyString == "#t")
							{
								param = (double)ConnectionStatusType::CONNECTION_READY;
								return TRUE;
							}
							else if (readyString == "#f")
							{
								param = (double)ConnectionStatusType::CONNECTION_ERROR_STATE;
							}
						}
						else 
						{
							param = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
							return TRUE;
						}

						return FALSE;
					}

					default: //Current Power
					{
						string powerString = string(_readBuffer);
						memset(_powerBuffer, 0, 256);
						try
						{
							//For instances where USB plugged in but laser is off
							if (powerString != "") 
							{
								//Serial port displays format "(param-ref 'laser1:power)\r\n100\r\n> " and we need to separate the power
								int pos1 = powerString.find_first_of("\n") + 1; //finds first instance of \n and sets value to position after it
								int pos2 = powerString.find_last_of("\r") - 1; //finds the second instance of \r and sets value to position before it
								memcpy(_powerBuffer, _readBuffer + pos1, (pos2 - pos1));
								param = atof(_powerBuffer);
								return TRUE;
							}
							return FALSE;
						}
						catch (...)
						{
							StringCbPrintfW(_errMsg, MSG_SIZE, L"Unable to convert read power to double");
							return FALSE;
						}
						return FALSE;
					}
				}
			}
			else
			{
				param = _tableParams[paramID]->GetParamVal();
				return TRUE;
			}
		}
	}

	return FALSE;
}

/// <summary>
/// Executes the Get command.
/// </summary>
/// <param name="cmd">The command.</param>
/// <param name="val">The read back value.</param>
/// <returns>long.</returns>
long TopticaiChrome::ExecuteCmdGet(wstring cmdGet) 
{
	Lock lock(_critSect);

	memset(_readBuffer, 0, 256);
#pragma warning(push)
#pragma warning(disable:4996)
	string serialString = ConvertWStringToString(cmdGet);
	const char* getInst = serialString.c_str();
	_serialPort.SendData((const unsigned char*)getInst, static_cast<int> (serialString.length()));
	Sleep(40); //Minimum tested waiting period necessary before reading from serial port
	_serialPort.ReadData(_readBuffer, 100);
	return TRUE;
	
}

/// <summary>
/// Executes the Set command.
/// </summary>
/// <param name="cmdSet">The command.</param>
/// <returns>long.</returns>
long TopticaiChrome::ExecuteCmdSet(long paramID, wstring cmdSet, double val)
{
	Lock lock(_critSect);

	char setInst[INSTRUCTION_LENGTH];
	memset(setInst, 0, INSTRUCTION_LENGTH);
	LPCSTR strStrFormat = "%s%s)\n";
	LPCSTR strIntFormat = "%s%s)\n";
	string cmdString = ConvertWStringToString(cmdSet);
	const char* _cmdBytes = cmdString.c_str();
	switch (paramID)
	{
		case PARAM_LASER1_ENABLE:
		case PARAM_LASER2_ENABLE:
		case PARAM_LASER3_ENABLE:
		case PARAM_LASER4_ENABLE:
		case PARAM_LASER1_EMISSION:
		case PARAM_LASER2_EMISSION:
		case PARAM_LASER3_EMISSION:
		case PARAM_LASER4_EMISSION:
		case PARAM_LASER_ALL_ENABLE:
		case PARAM_LASER_ALL_EMISSION:
		case PARAM_LASER_ALL_TTL_MODE:
		case PARAM_LASER_ALL_ANALOG_MODE:
		{
			//Change the user entered 1 or 0 to #t or #f to pass to the Serial Port
			const char *_stringVal;
			if (val == 0)
			{
				_stringVal = "#f";
			}

			else 
			{
				_stringVal = "#t";
			}

			StringCbPrintfA(setInst, INSTRUCTION_LENGTH, strStrFormat, _cmdBytes, _stringVal);
			_serialPort.SendData((const unsigned char*)setInst, static_cast<int> (strlen(setInst)));
			Sleep(40); //Minimum tested waiting period necessary before reading from serial port
			_serialPort.ReadData(setInst, 100); //Need to read the data to clear the serial port of the sent command
			return TRUE;
		}
		case PARAM_LASER1_POWER:
		case PARAM_LASER2_POWER:
		case PARAM_LASER3_POWER:
		case PARAM_LASER4_POWER:
		{
			//Pass the user entered laser power to the Serial Port
			string powerString = to_string(val);
			const char* _powerBytes = powerString.c_str();
			StringCbPrintfA(setInst, INSTRUCTION_LENGTH, strIntFormat, _cmdBytes, _powerBytes);
			_serialPort.SendData((const unsigned char*)setInst, static_cast<int> (strlen(setInst)));
			Sleep(40); //Minimum tested waiting period necessary before reading from serial port
			_serialPort.ReadData(setInst, 100); //Need to read the data to clear the serial port of the sent command
			return TRUE;
		}
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
long TopticaiChrome::GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault)
{
	if (NULL != _tableParams[paramID])
	{
		if (_tableParams[paramID]->GetParamID() == paramID)
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
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long TopticaiChrome::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long TopticaiChrome::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long TopticaiChrome::SetParamString(const long paramID, wchar_t* str)
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
long TopticaiChrome::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long TopticaiChrome::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long TopticaiChrome::ReadPosition(DeviceType deviceType, double& pos)
{
	long	ret = FALSE;

	StringCbPrintfW(_errMsg, MSG_SIZE, L"ReadPosition failed! Use the PARAM_LASER#_POSITION_CURRENT to read the power of the laser");

	return ret;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long TopticaiChrome::SetupPosition()
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
long TopticaiChrome::StartPosition()
{
	long ret = FALSE;

	//iterate through map and set the parameters
	for (std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if (((iter->second)->GetParamAvailable() == FALSE) || ((iter->second)->GetParamReadOnly() == TRUE))
		{
			continue;
		}
		else if ((iter->second)->GetParamBool() == TRUE)
		{
			ret = ExecuteCmdSet((iter->second)->GetParamID(),(iter->second)->GetCmdSet(), (iter->second)->GetParamVal());
			(iter->second)->UpdateParam_C();

			(iter->second)->SetParamBool(FALSE);

			StringCbPrintfW(_errMsg, MSG_SIZE, L"StartPosition succeeded at paramID: %d", (iter->second)->GetParamID());
			LogMessage(_errMsg, INFORMATION_EVENT);
		}
		else if ((iter->second)->GetParamFirst() == TRUE)
		{
			ret = ExecuteCmdSet((iter->second)->GetParamID(), (iter->second)->GetCmdSet(), (iter->second)->GetParamVal());
			(iter->second)->SetParamFirst();
			StringCbPrintfW(_errMsg, MSG_SIZE, L"StartPosition succeeded at paramID: %d", (iter->second)->GetParamID());
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
long TopticaiChrome::StatusPosition(long& status)
{
	long ret = TRUE;
	status = IDevice::STATUS_READY;
	return ret;
}


/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long TopticaiChrome::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long TopticaiChrome::GetLastErrorMsg(wchar_t* msg, long size)
{
	wcsncpy_s(msg, MSG_SIZE, _errMsg, MSG_SIZE);
	return TRUE;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void TopticaiChrome::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long TopticaiChrome::TeardownDevice()
{
	//Turn off lasers Enable State & Emission
	ExecuteCmdSet(PARAM_LASER1_ENABLE, _tableParams[PARAM_LASER1_ENABLE]->GetCmdSet(), 0);
	ExecuteCmdSet(PARAM_LASER2_ENABLE, _tableParams[PARAM_LASER2_ENABLE]->GetCmdSet(), 0);
	ExecuteCmdSet(PARAM_LASER3_ENABLE, _tableParams[PARAM_LASER3_ENABLE]->GetCmdSet(), 0);
	ExecuteCmdSet(PARAM_LASER4_ENABLE, _tableParams[PARAM_LASER4_ENABLE]->GetCmdSet(), 0);
	ExecuteCmdSet(PARAM_LASER1_EMISSION, _tableParams[PARAM_LASER1_EMISSION]->GetCmdSet(), 0);
	ExecuteCmdSet(PARAM_LASER2_EMISSION, _tableParams[PARAM_LASER2_EMISSION]->GetCmdSet(), 0);
	ExecuteCmdSet(PARAM_LASER3_EMISSION, _tableParams[PARAM_LASER3_EMISSION]->GetCmdSet(), 0);
	ExecuteCmdSet(PARAM_LASER4_EMISSION, _tableParams[PARAM_LASER4_EMISSION]->GetCmdSet(), 0);
	_serialPort.Close();

	return TRUE;
}

long TopticaiChrome::BuildParamTable()
{
	DestroyParamTable();
	//Execution in order of top to bottom of list:
	wstring commandSet = L"";
	wstring commandGet = L"";
	bool laserStartup;
	ParamInfo* tempParamInfo = new ParamInfo(
		PARAM_DEVICE_TYPE,							                //ID
		L"PARAM_DEVICE_TYPE",                                       //Parameter Description
		LASER1 | LASER2 | LASER3 | LASER4,							//VAL
		LASER1 | LASER2 | LASER3 | LASER4,							//PARAM C
		FALSE,										                //PARAM B
		FALSE,														//First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,                                                       //READ ONLY
		0,								                            //MIN
		LASER1 | LASER2 | LASER3 | LASER4,	                        //MAX
		0,								                            //DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(Params::PARAM_DEVICE_TYPE, tempParamInfo));

	//***Table entry to Enable Laser 1 (Does not emit)***// (Toptica laser4 command used since laser wavelengths are from highest to lowest)
	commandSet = L"(param-set! 'laser4:enable ";
	commandGet = L"(param-ref 'laser4:enable)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER1_ENABLE,							            //ID
		L"PARAM_LASER1_ENABLE",										//Parameter Description
		0,					                			            //VAL
		0,                								            //PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER1_ENABLE, tempParamInfo));

	//***Table entry to Enable Laser 2 (Does not emit)***//
	commandSet = L"(param-set! 'laser3:enable ";
	commandGet = L"(param-ref 'laser3:enable)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER2_ENABLE,							            //ID
		L"PARAM_LASER2_ENABLE",										//Parameter Description
		0,					                			            //VAL
		0,                								            //PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER2_ENABLE, tempParamInfo));

	//***Table entry to Enable Laser 3 (Does not emit)***//
	commandSet = L"(param-set! 'laser2:enable ";
	commandGet = L"(param-ref 'laser2:enable)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER3_ENABLE,							            //ID
		L"PARAM_LASER3_ENABLE",										//Parameter Description
		0,					                			            //VAL
		0,                								            //PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER3_ENABLE, tempParamInfo));

	//***Table entry to Enable Laser 4 (Does not emit)***//
	commandSet = L"(param-set! 'laser1:enable ";
	commandGet = L"(param-ref 'laser1:enable)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER4_ENABLE,							            //ID
		L"PARAM_LASER4_ENABLE",										//Parameter Description
		0,					                			            //VAL
		0,                								            //PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER4_ENABLE, tempParamInfo));

	//***Table entry to adjust Laser 1 Power***//
	commandSet = L"(param-set! 'laser4:level ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER1_POWER,							                //ID
		L"PARAM_LASER1_POWER",										//Parameter Description
		LASER_POWER_MIN,					                		//VAL
		LASER_POWER_MIN,                							//PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		LASER_POWER_MIN,								            //MIN
		LASER_POWER_MAX,											//MAX
		LASER_POWER_MIN,											//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER1_POWER, tempParamInfo));

	//***Table entry to adjust Laser 2 Power***//
	commandSet = L"(param-set! 'laser3:level ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER2_POWER,							                //ID
		L"PARAM_LASER2_POWER",										//Parameter Description
		LASER_POWER_MIN,					                		//VAL
		LASER_POWER_MIN,                							//PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		LASER_POWER_MIN,								            //MIN
		LASER_POWER_MAX,											//MAX
		LASER_POWER_MIN,											//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER2_POWER, tempParamInfo));

	//***Table entry to adjust Laser 3 Power***//
	commandSet = L"(param-set! 'laser2:level ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER3_POWER,							                //ID
		L"PARAM_LASER3_POWER",										//Parameter Description
		LASER_POWER_MIN,					                		//VAL
		LASER_POWER_MIN,                							//PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		LASER_POWER_MIN,								            //MIN
		LASER_POWER_MAX,											//MAX
		LASER_POWER_MIN,											//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER3_POWER, tempParamInfo));

	//***Table entry to adjust Laser 4 Power***//
	commandSet = L"(param-set! 'laser1:level ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER4_POWER,							                //ID
		L"PARAM_LASER4_POWER",										//Parameter Description
		LASER_POWER_MIN,					                		//VAL
		LASER_POWER_MIN,                							//PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		LASER_POWER_MIN,								            //MIN
		LASER_POWER_MAX,											//MAX
		LASER_POWER_MIN,											//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER4_POWER, tempParamInfo));

	//***Table entry to read current Laser 1 Power***//
	commandSet = L"";
	commandGet = L"(param-ref 'laser4:level)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER1_POWER_CURRENT,							        //ID
		L"PARAM_LASER1_POWER_CURRENT",								//Parameter Description
		LASER_POWER_MIN,					                		//VAL
		LASER_POWER_MIN,                							//PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,                                                       //READ ONLY
		LASER_POWER_MIN,								            //MIN
		LASER_POWER_MAX,											//MAX
		LASER_POWER_MIN,											//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER1_POWER_CURRENT, tempParamInfo));

	//***Table entry to read current Laser 2 Power***//
	commandSet = L"";
	commandGet = L"(param-ref 'laser3:level)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER2_POWER_CURRENT,							        //ID
		L"PARAM_LASER2_POWER_CURRENT",								//Parameter Description
		LASER_POWER_MIN,					                		//VAL
		LASER_POWER_MIN,                							//PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,                                                       //READ ONLY
		LASER_POWER_MIN,								            //MIN
		LASER_POWER_MAX,											//MAX
		LASER_POWER_MIN,											//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER2_POWER_CURRENT, tempParamInfo));

	//***Table entry to read current Laser 3 Power***//
	commandSet = L"";
	commandGet = L"(param-ref 'laser2:level)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER3_POWER_CURRENT,							        //ID
		L"PARAM_LASER3_POWER_CURRENT",								//Parameter Description
		LASER_POWER_MIN,					                		//VAL
		LASER_POWER_MIN,                							//PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,                                                       //READ ONLY
		LASER_POWER_MIN,								            //MIN
		LASER_POWER_MAX,											//MAX
		LASER_POWER_MIN,											//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER3_POWER_CURRENT, tempParamInfo));

	//***Table entry to read current Laser 4 Power***//
	commandSet = L"";
	commandGet = L"(param-ref 'laser1:level)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER4_POWER_CURRENT,							        //ID
		L"PARAM_LASER4_POWER_CURRENT",								//Parameter Description
		LASER_POWER_MIN,					                		//VAL
		LASER_POWER_MIN,                							//PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,                                                       //READ ONLY
		LASER_POWER_MIN,								            //MIN
		LASER_POWER_MAX,											//MAX
		LASER_POWER_MIN,											//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER4_POWER_CURRENT, tempParamInfo));

	//***Table entry to allow for Emission of Laser 1***//
	commandSet = L"(param-set! 'laser4:cw ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER1_EMISSION,							            //ID
		L"PARAM_LASER1_EMISSION",								    //Parameter Description
		0,					                			            //VAL
		0,                								            //PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER1_EMISSION, tempParamInfo));

	//***Table entry to allow for Emission of Laser 2***//
	commandSet = L"(param-set! 'laser3:cw ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER2_EMISSION,							            //ID
		L"PARAM_LASER2_EMISSION",								    //Parameter Description
		0,					                			            //VAL
		0,                								            //PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER2_EMISSION, tempParamInfo));

	//***Table entry to allow for Emission of Laser 3***//
	commandSet = L"(param-set! 'laser2:cw ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER3_EMISSION,							            //ID
		L"PARAM_LASER3_EMISSION",								    //Parameter Description
		0,					                			            //VAL
		0,                								            //PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER3_EMISSION, tempParamInfo));

	//***Table entry to allow for Emission of Laser 4***//
	commandSet = L"(param-set! 'laser1:cw ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER4_EMISSION,							            //ID
		L"PARAM_LASER4_EMISSION",								    //Parameter Description
		0,					                			            //VAL
		0,                								            //PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER4_EMISSION, tempParamInfo));

	//***Table entry to check the Wavelength of Laser 1***//
	commandSet = L"";
	commandGet = L"(param-ref 'laser4:label)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER1_WAVELENGTH,							        //ID
		L"PARAM_LASER1_WAVELENGTH",								    //Parameter Description
		LASER_WAVELENGTH_MIN,					                    //VAL
		LASER_WAVELENGTH_MIN,                						//PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,                                                       //READ ONLY
		LASER_WAVELENGTH_MIN,										//MIN
		LASER_WAVELENGTH_MAX,										//MAX
		LASER_WAVELENGTH_MIN,										//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER1_WAVELENGTH, tempParamInfo));

	//***Table entry to check the Wavelength of Laser 2***//
	commandSet = L"";
	commandGet = L"(param-ref 'laser3:label)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER2_WAVELENGTH,							        //ID
		L"PARAM_LASER2_WAVELENGTH",								    //Parameter Description
		LASER_WAVELENGTH_MIN,					                	//VAL
		LASER_WAVELENGTH_MIN,                						//PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,                                                       //READ ONLY
		LASER_WAVELENGTH_MIN,										//MIN
		LASER_WAVELENGTH_MAX,										//MAX
		LASER_WAVELENGTH_MIN,										//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER2_WAVELENGTH, tempParamInfo));

	//***Table entry to check the Wavelength of Laser 3***//
	commandSet = L"";
	commandGet = L"(param-ref 'laser2:label)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER3_WAVELENGTH,							        //ID
		L"PARAM_LASER3_WAVELENGTH",								    //Parameter Description
		LASER_WAVELENGTH_MIN,					                	//VAL
		LASER_WAVELENGTH_MIN,                						//PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,                                                       //READ ONLY
		LASER_WAVELENGTH_MIN,										//MIN
		LASER_WAVELENGTH_MAX,										//MAX
		LASER_WAVELENGTH_MIN,										//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER3_WAVELENGTH, tempParamInfo));

	//***Table entry to check the Wavelength of Laser 4***//
	commandSet = L"";
	commandGet = L"(param-ref 'laser1:label)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER4_WAVELENGTH,							        //ID
		L"PARAM_LASER4_WAVELENGTH",								    //Parameter Description
		LASER_WAVELENGTH_MIN,					                	//VAL
		LASER_WAVELENGTH_MIN,                						//PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,                                                       //READ ONLY
		LASER_WAVELENGTH_MIN,										//MIN
		LASER_WAVELENGTH_MAX,										//MAX
		LASER_WAVELENGTH_MIN,										//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER4_WAVELENGTH, tempParamInfo));

	//***Table entry to Enable All Lasers (Does not emit)***//
	commandSet = L"(param-set! 'all:enable ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER_ALL_ENABLE,							            //ID
		L"PARAM_LASER_ALL_ENABLE",									//Parameter Description
		0,					                			            //VAL
		0,                								            //PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER_ALL_ENABLE, tempParamInfo));

	//***Table entry to allow for Emission of All Lasers***//    Not used right now
	commandSet = L"(param-set! 'all:cw ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER_ALL_EMISSION,							        //ID
		L"PARAM_LASER_ALL_EMISSION",								//Parameter Description
		0,					                			            //VAL
		0,                								            //PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER_ALL_EMISSION, tempParamInfo));

	//***Table entry to enable TTL Mode for All Lasers***//
	commandSet = L"(param-set! 'all:use-ttl ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER_ALL_TTL_MODE,									//ID
		L"PARAM_LASER_ALL_TTL_MODE",								//Parameter Description
		0,															//VAL
		0,                											//PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER_ALL_TTL_MODE, tempParamInfo));

	//***Table entry to enable Analog Mode for All Lasers***//
	commandSet = L"(param-set! 'all:analog-mode ";
	commandGet = L"";
	tempParamInfo = new ParamInfo(
		PARAM_LASER_ALL_ANALOG_MODE,								//ID
		L"PARAM_LASER_ALL_ANALOG_MODE",								//Parameter Description
		0,															//VAL
		0,                											//PARAM C
		FALSE,										                //PARAM B
		TRUE,                                                       //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER_ALL_ANALOG_MODE, tempParamInfo));

	//***Table entry to read Connection Status for Toptica***//
	commandSet = L"";
	commandGet = L"(param-ref 'all:ready)\n";
	tempParamInfo = new ParamInfo(
		PARAM_CONNECTION_STATUS,								    //ID
		L"PARAM_CONNECTION_STATUS",								    //Parameter Description
		0,															//VAL
		0,                											//PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,                                                       //READ ONLY
		(double)ConnectionStatusType::CONNECTION_WARMING_UP,		//MIN
		(double)ConnectionStatusType::CONNECTION_ERROR_STATE,		//MAX
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,		//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTION_STATUS, tempParamInfo));

/*
	//***Eventual table entry for finding name of laser connected?***
	commandSet = L"";
	commandGet = L"(param-ref 'system-type)\n";
	tempParamInfo = new ParamInfo(
		PARAM_LASER_TYPE,								            //ID
		L"PARAM_LASER_TYPE",								        //Parameter Description
		0,															//VAL
		0,                											//PARAM C
		FALSE,										                //PARAM B
		FALSE,                                                      //First Set
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		FALSE,                                                      //READ ONLY
		0,															//MIN
		1,															//MAX
		0,															//DEFAULT
		commandSet,													//Command Set
		commandGet);												//Command Get
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LASER_TYPE, tempParamInfo));
*/

	return TRUE;
}

/// <summary>
/// Destroys the parameter table.
/// </summary>
/// <returns>long.</returns>
long TopticaiChrome::DestroyParamTable()
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
#ifdef LOGGING_ENABLED
		StringCbPrintfW(_errMsg, MSG_SIZE, L"TopticaiChrome DestroyParamTable unable to destroy the table created on heap");
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, _errMsg);
#endif
		return FALSE;
	}

	return TRUE;
}