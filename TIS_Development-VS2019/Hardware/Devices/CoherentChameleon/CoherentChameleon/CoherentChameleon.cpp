// CoherentChameleon.cpp : Defines the exported functions for the DLL application.
//

//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// CoherentChameleon.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "CoherentChameleon.h"
#include "CoherentChameleonSetupXML.h"
#include "Strsafe.h"

#define READ_TIMEOUT 1000

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

wchar_t message[MSG_SIZE];


/// <summary>
/// Prevents a default instance of the <see cref="CoherentChameleon"/> class from being created.
/// </summary>
CoherentChameleon::CoherentChameleon()
{
	_laser1Min=690;
	_laser1Max=1040;

	_laser1ShutterMin = 0;
	_laser1ShutterMax = 1;

	_laser1Position=_laser1Min;

	_laser1Position_C=_laser1Min-1;

	_laser2Position = _laser1Min;

	_laser2Position_C = _laser1Min - 1;

	_laser1ShutterPosition=_laser1ShutterMin;

	_laser1Shutter2Position = _laser1ShutterMin;

	_laser1ShutterPosition_C = _laser1Shutter2Position_C =_laser1ShutterMin-1;

	_laser1Shutter2Exists = FALSE;

	_deviceDetected = FALSE;

	_errMsg[0] = NULL;

	_dataBuffer[0] = NULL;
	_readBuffer[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="CoherentChameleon"/> class.
/// </summary>
CoherentChameleon::~CoherentChameleon()
{
	_instanceFlag = false;
}

bool CoherentChameleon:: _instanceFlag = false;

auto_ptr<CoherentChameleon> CoherentChameleon::_single(new CoherentChameleon());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>CoherentChameleon *.</returns>
CoherentChameleon *CoherentChameleon::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new CoherentChameleon());
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
long CoherentChameleon::FindDevices(long &deviceCount)
{
	long	ret = TRUE;

	if(_deviceDetected)
	{
		_serialPort.Close();
		_deviceDetected = FALSE;
	}


	long portID=0;

	try
	{
		auto_ptr<CoherentChameleonXML> pSetup(new CoherentChameleonXML());

		pSetup->GetConnection(portID,_laser1Shutter2Exists,_laser1Min,_laser1Max);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate CoherentChameleonSettings.xml file");
		deviceCount = 0;
		return FALSE;
	}


	const long PORTNAME_LENGTH = 32;
	TCHAR PortName[PORTNAME_LENGTH];
	StringCbPrintfW(PortName,PORTNAME_LENGTH,L"COM%d", portID);

	if(FALSE == _serialPort.Open(PortName))
	{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"CoherentChameleon FindDevices could not open serial port");
#endif
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to open com port %d",portID);

		// *TODO* perform an automatic scane of the available serial ports for the Z stepper
		deviceCount = 0;
		ret = FALSE;
	}
	else
	{
		TCHAR inst[INSTRUCTION_LENGTH];
		long readLength=0;
		StringCbPrintfW(inst,INSTRUCTION_LENGTH,L"?S");

		//*TODO* check the return value for an accurate response string
		//test the connection
		if (ret=_serialPort.Write(inst))
		{
			readLength=ReadPort(READ_PORT_TIMEOUT_MS);
			//there 
			if(readLength <= 0)
			{
				deviceCount = 0;
			}
			else
			{
				deviceCount = 1;
				_deviceDetected = TRUE;
			}
		}
		_serialPort.Close();
	}

	return ret;
}

/// <summary>
/// Gets the tuning status.
/// </summary>
/// <param name="status">The status.</param>
void CoherentChameleon::GetTuningStatus(long & status)
{
	Lock lock(_critSect);

	TCHAR inst[INSTRUCTION_LENGTH];
	BOOL ret;
	StringCbPrintfW(inst,INSTRUCTION_LENGTH,L"?TS");
	if (ret=_serialPort.Write(inst))
		ret=ReadPort(READ_PORT_TIMEOUT_MS);


	wstring ws = _readBuffer;

	if(('0' ==_readBuffer[0]) || ('0' == _readBuffer[1]))
	{
		status = 0;
	}
	else
	{
		status = 1;
	}
#ifdef LOGGING_ENABLED
	StringCbPrintfW(_errMsg,MSG_SIZE,L"Tuning Status status %d",status);
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);	
#endif


}

/// <summary>
/// Gets the vw position.
/// </summary>
/// <param name="position">The position.</param>
void CoherentChameleon::GetVWPosition(long & position)
{
	Lock lock(_critSect);

	TCHAR inst[INSTRUCTION_LENGTH];
	BOOL ret;
	StringCbPrintfW(inst, INSTRUCTION_LENGTH, L"?VW");
	if (ret=_serialPort.Write(inst))
		ret=ReadPort(READ_PORT_TIMEOUT_MS);

	const std::regex pattern("(.*)\r\n");

	string s = ConvertWStringToString(_readBuffer);

	std::match_results<std::string::const_iterator> result;
	std::regex_match(s, result, pattern);

#ifdef LOGGING_ENABLED
	StringCbPrintfW(_errMsg,MSG_SIZE,L"GetVWposition %s resultsize %d",ws.c_str(),result.size());
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);	
#endif

	if(result.size() >= 2)
	{
		stringstream ssIop(result[1]);
		ssIop>>position;

#ifdef LOGGING_ENABLED
		StringCbPrintfW(_errMsg,MSG_SIZE,L"GetVWposition position %d",position);
		logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);	
#endif
	}
}

/// <summary>
/// Gets the laser status.
/// </summary>
/// <param name="status">The status.</param>
void CoherentChameleon::GetLaserStatus(long & status)
{
	Lock lock(_critSect);

	TCHAR inst[INSTRUCTION_LENGTH];
	BOOL ret;
	StringCbPrintfW(inst, INSTRUCTION_LENGTH, L"?L");
	if (ret=_serialPort.Write(inst))
		ret=ReadPort(READ_PORT_TIMEOUT_MS);

	const std::regex pattern("(.*)\r\n");

	string s = ConvertWStringToString(_readBuffer);

	std::match_results<std::string::const_iterator> result;
	std::regex_match(s, result, pattern);

#ifdef LOGGING_ENABLED
	StringCbPrintfW(_errMsg,MSG_SIZE,L"L %s resultsize %d",ws.c_str(),result.size());
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);	
#endif

	if(result.size() >= 2)
	{
		stringstream ssIop(result[1]);
		ssIop>>status;

#ifdef LOGGING_ENABLED
		StringCbPrintfW(_errMsg,MSG_SIZE,L"GetLaserStatus %d",status);
		logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);	
#endif
	}
}

void CoherentChameleon::GetShutterPosition(long & position, long & position2)
{
	Lock lock(_critSect );

	static bool ppSwitch = true;
	static long tempPos1 = 0;
	static long tempPos2 = 0;

	if(_laser1Shutter2Exists) ppSwitch = !ppSwitch;
	else ppSwitch = true;

	TCHAR inst[INSTRUCTION_LENGTH];
	BOOL ret;

	if(ppSwitch)
	{
		StringCbPrintfW(inst, INSTRUCTION_LENGTH, L"?S");
		if (ret=_serialPort.Write(inst))
			ret=ReadPort(READ_PORT_TIMEOUT_MS);

		const std::regex pattern("(.*)\r\n");

		string s = ConvertWStringToString(_readBuffer);

		std::match_results<std::string::const_iterator> result;
		std::regex_match(s, result, pattern);

		if(result.size() >= 2)
		{
			stringstream ssIop(result[1]);
			ssIop>>position;		
		}
		tempPos1 = position;
		position2 =  tempPos2;
	}
	//retrieve the second shutter position
	if(!ppSwitch)
	{
		StringCbPrintfW(inst, INSTRUCTION_LENGTH, L"?SFIXED");
		if (ret=_serialPort.Write(inst))
			ret=ReadPort(READ_PORT_TIMEOUT_MS);

		const std::regex pattern("(.*)\r\n");

		string s = ConvertWStringToString(_readBuffer);

		std::match_results<std::string::const_iterator> result;
		std::regex_match(s, result, pattern);

		if(result.size() >= 2)
		{
			stringstream ssIop(result[1]);
			ssIop>>position2;		
		}		
		position = tempPos1;
		tempPos2 = position2;
	}
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long CoherentChameleon::SelectDevice(const long device)
{
	long	ret = FALSE;

	if(FALSE == _deviceDetected)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"No devices found");
		return FALSE;
	}

	switch(device)
	{
	case 0:	
		{
			long portID=0;

			try
			{
				auto_ptr<CoherentChameleonXML> pSetup(new CoherentChameleonXML());

				pSetup->GetConnection(portID,_laser1Shutter2Exists,_laser1Min,_laser1Max);
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate CoherentChameleonSettings.xml file");
				return FALSE;
			}

			const long PORTNAME_LENGTH = 32;
			TCHAR PortName[PORTNAME_LENGTH];
			StringCbPrintfW(PortName,PORTNAME_LENGTH,L"COM%d", portID);

			if(FALSE == _serialPort.Open(PortName))
			{
#ifdef LOGGING_ENABLED
				logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"CoherentChameleon FindDevices could not open serial port");
#endif
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to open com port %d",portID);
				return FALSE;
			}


			TCHAR inst[INSTRUCTION_LENGTH];

			long tempRet;

			//turn off ECHO
			StringCbPrintfW(inst,INSTRUCTION_LENGTH,L"ECHO=0");
			if (tempRet=_serialPort.Write(inst))
				tempRet=ReadPort(READ_PORT_TIMEOUT_MS);

			//turn off PROMPT
			StringCbPrintfW(inst,INSTRUCTION_LENGTH,L"PROMPT=0");
			if (tempRet=_serialPort.Write(inst))
				tempRet=ReadPort(READ_PORT_TIMEOUT_MS);

			ret = SetLaserShutterPosition(1,0);
			ret = SetLaserShutterPosition(2,0);

			ret = TRUE; 				
		}
		break;
	default:	{ }
	}

	return ret;
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long CoherentChameleon::TeardownDevice()
{
	if (TRUE == SetLaserShutterPosition(1,0))	// close laser shutter
	{
		SetLaserShutterPosition(2,0);
		_laser1ShutterPosition_C = _laser1ShutterPosition = _laser1ShutterMin; 
	}
	_serialPort.Close();
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
long CoherentChameleon::GetParamInfo
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
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_LASER1_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _laser1Min;
			paramMax = _laser1Max;
			paramDefault = _laser1Min;
		}
		break;

	case PARAM_LASER1_POS_CURRENT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _laser1Min;
			paramMax = _laser1Max;
			paramDefault = _laser1Min;
		}
		break;

	case PARAM_LASER1_SHUTTER_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _laser1ShutterMin;
			paramMax = _laser1ShutterMax;
			paramDefault = _laser1ShutterMin;
		}
		break;

	case PARAM_LASER1_SHUTTER_POS_CURRENT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _laser1ShutterMin;
			paramMax = _laser1ShutterMax;
			paramDefault = _laser1ShutterMin;
		}
		break;

	case PARAM_LASER1_SHUTTER2_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _laser1ShutterMin;
			paramMax = _laser1ShutterMax;
			paramDefault = _laser1ShutterMin;
		}
		break;

	case PARAM_LASER1_SHUTTER2_POS_CURRENT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _laser1ShutterMin;
			paramMax = _laser1ShutterMax;
			paramDefault = _laser1ShutterMin;
		}
		break;


	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = (double)DeviceType::LASER1;
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = 0;
		}
		break;
	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long CoherentChameleon::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_LASER1_POS:
		{
			if((param >= _laser1Min) && (param <= _laser1Max))
			{
				_laser1Position = static_cast<long>(param);

			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER1_POWER out of range %d to %d",static_cast<long>(_laser1Min),static_cast<long>(_laser1Max));
				ret = FALSE;
			}
		}
		break;

	case PARAM_LASER1_POS_CURRENT:
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter is read only");
			ret = FALSE;
		}
		break;

	case PARAM_LASER1_SHUTTER_POS:
		{
			if((param >= _laser1ShutterMin) && (param <= _laser1ShutterMax))
			{
				_laser1ShutterPosition = static_cast<long>(param);

			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER1_SHUTTER_POS out of range %d to %d",static_cast<long>(_laser1ShutterMin),static_cast<long>(_laser1ShutterMax));
				ret = FALSE;
			}
		}
		break;

	case PARAM_LASER1_SHUTTER_POS_CURRENT:
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter is read only");
			ret = FALSE;
		}
		break;

	case PARAM_LASER1_SHUTTER2_POS:
		{
			if((param >= _laser1ShutterMin) && (param <= _laser1ShutterMax))
			{
				_laser1Shutter2Position = static_cast<long>(param);
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER1_SHUTTER2_POS out of range %d to %d",static_cast<long>(_laser1ShutterMin),static_cast<long>(_laser1ShutterMax));
				ret = FALSE;
			}
		}
		break;

	case PARAM_LASER1_SHUTTER2_POS_CURRENT:
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter is read only");
			ret = FALSE;
		}
		break;
	default:
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter not implemented");
			ret = FALSE;
		}
	}

	return ret;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long CoherentChameleon::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_LASER1_POS:	
		{ 
			param = static_cast<double>(_laser1Position); 
		}
		break;

	case PARAM_LASER1_POS_CURRENT:	
		{ 
			long position;

			GetVWPosition(position);

			param = static_cast<double>(position); 
		}
		break;

	case PARAM_LASER1_SHUTTER_POS:	
		{ 
			param = static_cast<double>(_laser1ShutterPosition); 
		}
		break;

	case PARAM_LASER1_SHUTTER_POS_CURRENT:	
		{ 
			long position,position2;

			position = _laser1ShutterPosition_C;
			position2 = _laser1Shutter2Position_C;

			GetShutterPosition(position,position2);

			//be sure to set all position parameters when reading from the device to maintain synchronization with the GUI
			_laser1ShutterPosition = _laser1ShutterPosition_C = position;

			param = static_cast<double>(position); 
		}
		break;

	case PARAM_LASER1_SHUTTER2_POS:	
		{ 
			param = static_cast<double>(_laser1Shutter2Position); 
		}
		break;

	case PARAM_LASER1_SHUTTER2_POS_CURRENT:	
		{ 
			long position,position2;

			position = _laser1ShutterPosition_C;
			position2 = _laser1Shutter2Position_C;

			GetShutterPosition(position,position2);

			//be sure to set all position parameters when reading from the device to maintain synchronization with the GUI
			_laser1Shutter2Position =_laser1Shutter2Position_C = position2;

			param = static_cast<double>(position2); 
		}
		break;

	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(DeviceType::LASER1);
		}
		break;

	case PARAM_CONNECTION_STATUS:
		{
			long status = 0;
			GetLaserStatus(status);			
			param = static_cast<double>(status);
		}
		break;
	default:
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter not implemented");

			ret = FALSE;
		}
	}

	return ret;
}

/// <summary>
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long CoherentChameleon::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long CoherentChameleon::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long CoherentChameleon::SetParamString(const long paramID, wchar_t* str)
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
long CoherentChameleon::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long CoherentChameleon::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long CoherentChameleon::SetupPosition()
{
	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long CoherentChameleon::StartPosition()
{
	long ret = TRUE;

#ifdef LOGGING_ENABLED
	StringCbPrintfW(_errMsg,MSG_SIZE,L"SetLaserPosition1 Power %d",_laser1Position);
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);			
#endif

	if(_laser1Position != _laser1Position_C)
	{
		ret=SetLaserPosition(_laser1Position);

		if(ret)
		{
			_laser1Position_C = _laser1Position;

			//const int MAX_TUNING_TIME = 10000;
			//DWORD startTime = GetTickCount();
			//long tuningStatus;
			//do
			//{	
			//	GetTuningStatus(tuningStatus);

			//	//Ready state is zero
			//	if(tuningStatus == 0)
			//	{
			//		break;
			//	}

			//	Sleep(500);

			//}while ((GetTickCount() - startTime)<MAX_TUNING_TIME);

			ret = TRUE;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetLaserPosition 1 failed");
			return FALSE;
		}
	}

	if(_laser1ShutterPosition != _laser1ShutterPosition_C)
	{
		const long SHUTTER1 = 1;
		ret = SetLaserShutterPosition(SHUTTER1, _laser1ShutterPosition);

		if(ret)
		{
			_laser1ShutterPosition_C = _laser1ShutterPosition;
			ret = TRUE;
		}
	}

	if(_laser1Shutter2Position != _laser1Shutter2Position_C)
	{
		const long SHUTTER2 = 2;
		ret = SetLaserShutterPosition(SHUTTER2, _laser1Shutter2Position);

		if(ret)
		{
			_laser1Shutter2Position_C = _laser1Shutter2Position;
			ret = TRUE;
		}
	}
	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long CoherentChameleon::StatusPosition(long &status)
{
	long	ret = TRUE;

	long tuningStatus = 0;
	GetTuningStatus(tuningStatus);

	//Ready state is zero
	if(tuningStatus == 0)
	{
		status = IDevice::STATUS_READY;
	}
	else
	{
		status = IDevice::STATUS_BUSY;
	}
	//Sleep(200); //Fixes Delay between T Series [gw]

	return ret;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long CoherentChameleon::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	StringCbPrintfW(_errMsg,MSG_SIZE,L"ReadPosition failed! Use the PARAM_LASER#_POSITION_CURRENT to read the power of the laser");

	return ret;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long CoherentChameleon::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Sets the laser position.
/// </summary>
/// <param name="position">The position.</param>
/// <returns>long.</returns>
long CoherentChameleon::SetLaserPosition(long position)
{
	Lock lock(_critSect);

	TCHAR inst[INSTRUCTION_LENGTH];
	BOOL ret;

	StringCbPrintfW(inst,INSTRUCTION_LENGTH, L"VW=%d", position);
	if (ret=_serialPort.Write(inst))
		ret=ReadPort(READ_PORT_TIMEOUT_MS);

	return ret;
}

/// <summary>
/// Sets the laser shutter position.
/// </summary>
/// <param name="position">The position.</param>
/// <param name="position2">The position2.</param>
/// <returns>long.</returns>
long CoherentChameleon::SetLaserShutterPosition(long shutter, long position)
{
	Lock lock(_critSect);

	TCHAR inst[INSTRUCTION_LENGTH];
	BOOL ret;

	switch(shutter)
	{

	case 1:
		{
			//MessageBox(NULL,L"Shutter 1",NULL,NULL);
			StringCbPrintfW(inst,INSTRUCTION_LENGTH, L"S=%d", position);
			if (ret=_serialPort.Write(inst))
				ret=ReadPort(READ_PORT_TIMEOUT_MS);
		}
		break;

	case 2:
		{
			//retrieve the second shutter position
			if(_laser1Shutter2Exists)
			{
				Sleep(100);

				StringCbPrintfW(inst,INSTRUCTION_LENGTH, L"SFIXED=%d", position);
				if (ret=_serialPort.Write(inst))
					ret=ReadPort(READ_PORT_TIMEOUT_MS);
			}
		}
		break;

	}
	return TRUE;
}

/// <summary>
/// Reads the port.
/// </summary>
/// <param name="timeout">The timeout.</param>
/// <returns>long.</returns>
long CoherentChameleon::ReadPort(long timeout)
{	
	memset(_readBuffer,0,sizeof(TCHAR)*256);
	return _serialPort.Read(_readBuffer, timeout);
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long CoherentChameleon::GetLastErrorMsg(wchar_t *msg, long size)
{	
	wcsncpy_s(msg,MSG_SIZE,_errMsg,MSG_SIZE);
	return TRUE;
}