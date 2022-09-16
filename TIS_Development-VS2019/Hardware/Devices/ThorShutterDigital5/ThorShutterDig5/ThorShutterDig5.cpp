//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorShutterDig5.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "ThorShutterDig5.h"
#include "ThorShutterDig5SetupXML.h"

/// <summary>
/// The message
/// </summary>
wchar_t message[MAX_PATH];
bool ThorShutterDig5::_deviceDetected = false;

/// <summary>
/// Prevents a default instance of the <see cref="ThorShutterDig5"/> class from being created.
/// </summary>
ThorShutterDig5::ThorShutterDig5()
{	
	_deviceDetected = false;
	_devName = "";
	_taskHandleDO0 = 0;
	_shutterDelayMS = 30;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorShutterDig5"/> class.
/// </summary>
ThorShutterDig5::~ThorShutterDig5()
{
	_instanceFlag = false;
	_deviceDetected = false;
}

bool ThorShutterDig5:: _instanceFlag = false;

auto_ptr<ThorShutterDig5> ThorShutterDig5::_single(new ThorShutterDig5());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorShutterDig5 *.</returns>
ThorShutterDig5 *ThorShutterDig5::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorShutterDig5());
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
long ThorShutterDig5::FindDevices(long &deviceCount)
{
	long	ret = TRUE;

	deviceCount=0;
	//Get filter parameters from hardware setup.xml
	auto_ptr<ThorShutterDig5XML> pSetup(new ThorShutterDig5XML());

	double volts2mm=0;
	double offsetmm=0;
	double zPos_min=0;
	double zPos_max=0;

	wsprintf(message,L"ThorShutterDig5 FindDevices");
	LogMessage(message,VERBOSE_EVENT);

	if (ret=pSetup->OpenConfigFile())
	{
		string str;

		if(ret=pSetup->GetIO(str,_shutterDelayMS))
		{
			_devName = str;
			deviceCount = 1;
			_deviceDetected = true;
			wsprintf(message,L"ThorShutterDig5 Found a device");
			LogMessage(message,VERBOSE_EVENT);
		}
	}
	return ret;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorShutterDig5::SelectDevice(const long device)
{
	return TRUE;
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorShutterDig5::TeardownDevice()
{
	SetDigital(SHUTTER_CLOSED);
	CloseNITasks();
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
long ThorShutterDig5::GetParamInfo
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
	case PARAM_CONNECTION_STATUS:
		{		
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = CONNECTION_READY;
			paramMax = paramDefault = CONNECTION_UNAVAILABLE;
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
long ThorShutterDig5::SetParam(const long paramID, const double param)
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
long ThorShutterDig5::GetParam(const long paramID, double &param)
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
			param = (_deviceDetected) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
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
long ThorShutterDig5::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorShutterDig5::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorShutterDig5::SetParamString(const long paramID, wchar_t* str)
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
long ThorShutterDig5::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorShutterDig5::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorShutterDig5::SetupPosition()
{
	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorShutterDig5::StartPosition()
{
	long	ret = FALSE;

	if (0==SetDigital(_shutPos))
	{
		//If opening the shutter wait sufficent time for the shutter to open
		//Only sleep if the previous state was closed. 
		if(0 == _shutPos && 1 == _shutPos_C)
		{
			wsprintf(message,L"ThorShutterDig5 sleep on open");
			LogMessage(message,INFORMATION_EVENT);

			Sleep(_shutterDelayMS);
		}
		_shutPos_C=_shutPos;
		ret = TRUE;
	}
	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorShutterDig5::StatusPosition(long &status)
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
long ThorShutterDig5::ReadPosition(DeviceType deviceType, double &pos)
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
long ThorShutterDig5::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorShutterDig5::GetLastErrorMsg(wchar_t * msg, long size)
{	
	wcsncpy_s(msg,size,_errMsg,256);
	return TRUE;
}

/// <summary>
/// Sets the digital.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorShutterDig5::SetDigital(long pos)
{
	Lock lock(_critSect);

	wsprintf(message,L"ThorShutterDig5 setting digitial %d", pos);
	LogMessage(message,VERBOSE_EVENT);

	int32 error = 0;

	try 
	{
		int32 retVal=0;

		if(_devName.size() <= 0)
		{
			return retVal;
		}

		if (_taskHandleDO0 != 0)
		{
			retVal = DAQmxStopTask(_taskHandleDO0);
			retVal = DAQmxClearTask(_taskHandleDO0);
		}
		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleDO0));
		DAQmxErrChk(L"DAQmxCreateDOChan",retVal = DAQmxCreateDOChan(_taskHandleDO0, _devName.c_str(), "", DAQmx_Val_ChanForAllLines));
		uInt32 triggerLevel[1];
		if(0 == pos)
		{
			triggerLevel[0] = 0xFFFFFFFF;
		}
		else
		{
			triggerLevel[0] = 0x0;
		}
		DAQmxErrChk(L"DAQmxWriteDigitalU32",retVal = DAQmxWriteDigitalU32(_taskHandleDO0, 1, 1, 10.0, DAQmx_Val_GroupByChannel, &triggerLevel[0], NULL, NULL));
		DAQmxErrChk(L"DAQmxWaitUntilTaskDone",retVal = DAQmxWaitUntilTaskDone(_taskHandleDO0,-1));
		return retVal;

	} 
	catch (...)
	{
		if( DAQmxFailed(error)) 
		{
			wsprintf(message,L"DAQ Digital Output Error, error code %d", error);
			LogMessage(message,VERBOSE_EVENT);
			wsprintf(_errMsg,L"DAQ Digital Output Error, error code %d", error);
			return error;
		}
	}
	return error;
}

/// <summary>
/// Closes the ni tasks.
/// </summary>
/// <returns>int.</returns>
int ThorShutterDig5::CloseNITasks()
{
	int32 retVal;
	if (_taskHandleDO0 != 0)
	{
		DAQmxWaitUntilTaskDone(_taskHandleDO0, 1.0);

		retVal = DAQmxStopTask(_taskHandleDO0);

		wsprintf(message,L"DAQmxStopTask DO0 Return = %d", retVal);
		LogMessage(message,VERBOSE_EVENT);
		wsprintf(_errMsg,L"DAQmxStopTask DO0 Return = %d", retVal);

		retVal = DAQmxClearTask(_taskHandleDO0);

		wsprintf(message,L"DAQmxClearTask DO0 Return = %d", retVal);
		LogMessage(message,VERBOSE_EVENT);
		wsprintf(_errMsg,L"DAQmxClearTask DO0 Return = %d", retVal);
		_taskHandleDO0 = NULL;
	}
	return TRUE;
}

