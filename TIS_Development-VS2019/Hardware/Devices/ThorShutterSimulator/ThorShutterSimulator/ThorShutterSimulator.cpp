//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorShutterSimulator.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "ThorShutterSimulator.h"
#include "ThorShutterSimulatorSetupXML.h"

#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

/// <summary>
/// The message
/// </summary>
wchar_t message[256];

/// <summary>
/// Prevents a default instance of the <see cref="ThorShutterSimulator"/> class from being created.
/// </summary>
ThorShutterSimulator::ThorShutterSimulator()
{
	_deviceDetected = FALSE;
	_shutterDelayMS = 30;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorShutterSimulator"/> class.
/// </summary>
ThorShutterSimulator::~ThorShutterSimulator()
{
	_instanceFlag = false;
}

bool ThorShutterSimulator:: _instanceFlag = false;

auto_ptr<ThorShutterSimulator> ThorShutterSimulator::_single(new ThorShutterSimulator());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorShutterSimulator *.</returns>
ThorShutterSimulator *ThorShutterSimulator::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorShutterSimulator());
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
long ThorShutterSimulator::FindDevices(long &deviceCount)
{
	long	ret = TRUE;
		
	wsprintf(message,L"ThorShutterSimulator FindDevices");
	LogMessage(message);

	deviceCount = 1;

	return ret;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorShutterSimulator::SelectDevice(const long device)
{
	return TRUE;
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorShutterSimulator::TeardownDevice()
{
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
long ThorShutterSimulator::GetParamInfo
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
	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = CONNECTION_READY;
			paramMax = CONNECTION_READY;
			paramDefault = CONNECTION_READY;
		}
		break;
	case PARAM_SHUTTER_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = SHUTTER;
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
long ThorShutterSimulator::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_SHUTTER_POS:
		{
			long tmpLong = static_cast<long>(param);
			if((tmpLong >= 0) && (tmpLong <= 1))
			{
				_shutPos = tmpLong;			
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	default:
		ret = FALSE;
	}

	return ret;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorShutterSimulator::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_SHUTTER_POS:	
		{ 
			param = static_cast<double>(_shutPos_C); 
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			param =static_cast<double>(SHUTTER);
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = CONNECTION_READY;
		}
		break;
	default:
		ret = FALSE;
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
long ThorShutterSimulator::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorShutterSimulator::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorShutterSimulator::SetParamString(const long paramID, wchar_t* str)
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
long ThorShutterSimulator::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorShutterSimulator::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorShutterSimulator::SetupPosition()
{
	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorShutterSimulator::StartPosition()
{
	long	ret = FALSE;

	_shutPos_C=_shutPos;
	ret = TRUE;

	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorShutterSimulator::StatusPosition(long &status)
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
long ThorShutterSimulator::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	if(deviceType & PARAM_SHUTTER_POS)
	{
		pos=_shutPos_C;
		ret = TRUE;
	}
	return ret;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorShutterSimulator::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorShutterSimulator::GetLastErrorMsg(wchar_t * msg, long size)
{	
	wcsncpy_s(msg,size,_errMsg,256);
	return TRUE;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorShutterSimulator::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(ERROR_EVENT, 1, message);
#endif
}


