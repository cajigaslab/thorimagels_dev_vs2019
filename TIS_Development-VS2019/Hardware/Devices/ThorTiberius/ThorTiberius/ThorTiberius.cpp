// ThorTiberius.cpp : Defines the exported functions for the DLL application.
//

//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorTiberius.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "ThorTiberius.h"
#include "ThorTiberiusXML.h"
#include "Strsafe.h"

#define READ_TIMEOUT 1000

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

wchar_t message[MSG_SIZE];


ThorTiberius::ThorTiberius()
{
	_laser1Min=720;
	_laser1Max=1060;

	_laser1ShutterMin = 0;
	_laser1ShutterMax = 1;

	_laser1Position=_laser1Min;

	_laser1Position_C=_laser1Min-1;

	_laser1ShutterPosition=_laser1ShutterMin;

	_laser1ShutterPosition_C=_laser1ShutterMin-1;

	memset(_seqBuffer,0,sizeof(_seqBuffer));

	_lastUpdateTime = 0;

	_lastWLReadTime = 0;
	_lastShutterReadTime = 0;

	_deviceDetected = FALSE;
}

ThorTiberius::~ThorTiberius()
{
	_instanceFlag = false;
}

bool ThorTiberius:: _instanceFlag = false;

auto_ptr<ThorTiberius> ThorTiberius::_single(new ThorTiberius());

ThorTiberius *ThorTiberius::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorTiberius());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorTiberius::FindDevices(long &deviceCount)
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
		auto_ptr<ThorTiberiusXML> pSetup(new ThorTiberiusXML());

		pSetup->GetConnection(portID);
		pSetup->GetWavelength(_laser1Min,_laser1Max);

	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorTiberiusSettings.xml file");
		deviceCount = 0;
		return FALSE;
	}

	const long PORTNAME_LENGTH = 32;
	TCHAR PortName[PORTNAME_LENGTH];
	StringCbPrintfW(PortName,PORTNAME_LENGTH,L"COM%d", portID);

	if(FALSE == _serialPort.Open(PortName))
	{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorTiberius FindDevices could not open serial port");
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
		StringCbPrintfW(inst,INSTRUCTION_LENGTH,L"S?");

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

void ThorTiberius::GetTuningStatus(long & status)
{
	Lock lock(_critSect);

	TCHAR inst[INSTRUCTION_LENGTH];
	BOOL ret;
	StringCbPrintfW(inst,INSTRUCTION_LENGTH,L"STATUS?");
	if (ret=_serialPort.Write(inst))
		ret=ReadPort(READ_PORT_TIMEOUT_MS);


	wstring ws = _readBuffer;

	if(_readBuffer[0] == 'R')
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

void ThorTiberius::GetVWPosition(long & position)
{
	Lock lock(_critSect);

	const DWORD WL_READ_GAP_MS = 2000;


	if((GetTickCount() - _lastWLReadTime)>WL_READ_GAP_MS)
	{
		TCHAR inst[INSTRUCTION_LENGTH];
		BOOL ret;
		StringCbPrintfW(inst, INSTRUCTION_LENGTH, L"W?");
		if (ret=_serialPort.Write(inst))
			ret=ReadPort(READ_PORT_TIMEOUT_MS);

		const std::regex pattern("(.*)\r\n");

		wstring ws = _readBuffer;
		string s = ConvertWStringToString(ws);

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
			_laser1Position_C = position;

#ifdef LOGGING_ENABLED
			StringCbPrintfW(_errMsg,MSG_SIZE,L"GetVWposition position %d",position);
			logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);	
#endif
		}
		_lastWLReadTime = GetTickCount();
	}

	position = _laser1Position_C;
}

void ThorTiberius::GetShutterPosition(long & position)
{		
	Lock lock(_critSect);

	const DWORD SHUTTER_READ_GAP_MS = 2000;


	if((GetTickCount() - _lastShutterReadTime)>SHUTTER_READ_GAP_MS)
	{	
		TCHAR inst[INSTRUCTION_LENGTH];
		BOOL ret;
		StringCbPrintfW(inst, INSTRUCTION_LENGTH, L"S?");

		if (ret=_serialPort.Write(inst))
			ret=ReadPort(READ_PORT_TIMEOUT_MS);

		const std::regex pattern("S:(.*)\r\n");

		wstring ws = _readBuffer;
		string s = ConvertWStringToString(ws);

		std::match_results<std::string::const_iterator> result;
		std::regex_match(s, result, pattern);

		if(result.size() >= 2)
		{
			stringstream ssIop(result[1]);
			ssIop>>position;		

			_laser1ShutterPosition_C = position;
		}

		_lastShutterReadTime=GetTickCount();
	}

	position = _laser1ShutterPosition_C;
}


long ThorTiberius::SelectDevice(const long device)
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
				auto_ptr<ThorTiberiusXML> pSetup(new ThorTiberiusXML());

				pSetup->GetConnection(portID);
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorTiberiusSettings.xml file");
				return FALSE;
			}

			const long PORTNAME_LENGTH = 32;
			TCHAR PortName[PORTNAME_LENGTH];
			StringCbPrintfW(PortName,PORTNAME_LENGTH,L"COM%d", portID);

			if(FALSE == _serialPort.Open(PortName))
			{
#ifdef LOGGING_ENABLED
				logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorTiberius FindDevices could not open serial port");
#endif
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to open com port %d",portID);
				return FALSE;
			}

			ret = SetLaserShutterPosition(0);

			ret = TRUE; 				
		}
		break;
	default:	{ }
	}

	return ret;
}

long ThorTiberius::TeardownDevice()
{
	if (TRUE == SetLaserShutterPosition(0))	// close laser shutter
	{
		_laser1ShutterPosition_C = _laser1ShutterPosition = _laser1ShutterMin; 
	}
	_serialPort.Close();
	return TRUE;
}

long ThorTiberius::GetParamInfo
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


	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = LASER1;
		}
		break;

	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = CONNECTION_READY;
			paramMax = paramDefault = CONNECTION_UNAVAILABLE;
		}
		break;

	case PARAM_LASER1_SEQ:
		{
			paramType = TYPE_BUFFER;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = paramDefault = 0;
		}
		break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long ThorTiberius::SetParam(const long paramID, const double param)
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

	default:
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter not implemented");
			ret = FALSE;
		}
	}

	return ret;
}

long ThorTiberius::GetParam(const long paramID, double &param)
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
			long position;

			GetShutterPosition(position);

			param = static_cast<double>(position); 
		}
		break;

	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(LASER1);
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = (_deviceDetected) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
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
long ThorTiberius::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_LASER1_SEQ:
		{
			memcpy_s(_seqBuffer,sizeof(_seqBuffer),pBuffer,size);
		}
		break;

	default:
		ret = false;
	}
	return ret;
}

/// <summary>
/// Gets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorTiberius::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_LASER1_SEQ:
		{
			memcpy_s(pBuffer,size,_seqBuffer,sizeof(_seqBuffer));
		}
		break;

	default:
		ret = false;
	}
	return ret;
}

/// <summary>
/// Sets the parameter string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <returns>long.</returns>
long ThorTiberius::SetParamString(const long paramID, wchar_t* str)
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
long ThorTiberius::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorTiberius::PreflightPosition()
{
	return TRUE;
}

long ThorTiberius::SetupPosition()
{
	return TRUE;
}

long ThorTiberius::StartPosition()
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
		ret = SetLaserShutterPosition(_laser1ShutterPosition);

		if(ret)
		{
			_laser1ShutterPosition_C = _laser1ShutterPosition;
		}
	}

	if(0 != memcmp(_seqBuffer,_seqBuffer_C,sizeof(_seqBuffer)))
	{
		ret = SetSequence(_seqBuffer);

		if(ret)
		{
			memcpy_s(_seqBuffer_C,sizeof(_seqBuffer_C),_seqBuffer,sizeof(_seqBuffer_C));
		}
	}

	return ret;
}

long ThorTiberius::StatusPosition(long &status)
{
	long	ret = TRUE;

	const DWORD GAP_BETWEEN_STATUS_CALLS_MS = 200;

	status = IDevice::STATUS_BUSY;

	if((GetTickCount() - _lastUpdateTime) > GAP_BETWEEN_STATUS_CALLS_MS)
	{
		long tuningStatus;
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
		_lastUpdateTime = GetTickCount();
	}

	return ret;
}

long ThorTiberius::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	StringCbPrintfW(_errMsg,MSG_SIZE,L"ReadPosition failed! Use the PARAM_LASER#_POSITION_CURRENT to read the power of the laser");

	return ret;
}

long ThorTiberius::PostflightPosition()
{
	return TRUE;
}

long ThorTiberius::SetLaserPosition(long position)
{	
	Lock lock(_critSect);

	TCHAR inst[INSTRUCTION_LENGTH];
	BOOL ret;

	StringCbPrintfW(inst,INSTRUCTION_LENGTH, L"W=%d", position);
	if (ret=_serialPort.Write(inst))
	{
		//acknowledgement not implemented
		//ret=ReadPort(READ_PORT_TIMEOUT_MS);
	}

	return ret;
}

long ThorTiberius::SetLaserShutterPosition(long position)
{	
	Lock lock(_critSect);

	TCHAR inst[INSTRUCTION_LENGTH];
	BOOL ret;

	StringCbPrintfW(inst,INSTRUCTION_LENGTH, L"S=%d", position);
	if (ret=_serialPort.Write(inst))
	{
		//acknowledgement not implemented
		//ret=ReadPort(READ_PORT_TIMEOUT_MS);
	}

	return ret;
}

long ThorTiberius::SetSequence(char *buffer)
{
	Lock lock(_critSect);

	TCHAR inst[INSTRUCTION_LENGTH];
	BOOL ret;

	ULONG32 val1= (((ULONG32)buffer[0])<<8) + ((((ULONG32)buffer[1]) & 0xff));
	ULONG32 val2= (((ULONG32)buffer[2])<<8) + ((((ULONG32)buffer[3]) & 0xff));
				
	if((0 == val1)&&(0==val2))
	{
		StringCbPrintfW(inst,INSTRUCTION_LENGTH, L"SEQ=0");
	}
	else
	{
		StringCbPrintfW(inst,INSTRUCTION_LENGTH, L"SEQ=%d,%d",val1,val2);
	}

	if (ret=_serialPort.Write(inst))
	{
		//acknowledgement not implemented
		//ret=ReadPort(READ_PORT_TIMEOUT_MS);
	}

	return ret;
}


long ThorTiberius::ReadPort(long timeout)
{	
	memset(_readBuffer,0,sizeof(TCHAR)*256);
	return _serialPort.Read(_readBuffer, timeout);
}


long ThorTiberius::GetLastErrorMsg(wchar_t *msg, long size)
{	
	wcsncpy_s(msg,MSG_SIZE,_errMsg,MSG_SIZE);
	return TRUE;
}