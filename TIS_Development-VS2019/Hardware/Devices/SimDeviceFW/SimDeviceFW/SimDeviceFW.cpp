// SimDeviceFW.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SimDeviceFW.h"

auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
wchar_t message[256];

SimDeviceFW::SimDeviceFW()
{    
	_afPos = AUTOFOCUS_MIN;
	_afPos_B = FALSE;
	_afPos_C = AUTOFOCUS_MIN;

	_filterDicPos = FILTER_WHEEL_DIC_MIN;
	_filterDicPos_B = FALSE;
	_filterDicPos_C = FILTER_WHEEL_DIC_MIN;

	_filterEmPos = FILTER_WHEEL_EM_MIN;
	_filterEmPos_B = FALSE;
	_filterEmPos_C = FILTER_WHEEL_EM_MIN;

	_filterExPos = FILTER_WHEEL_EX_MIN;
	_filterExPos_B = FALSE;
	_filterExPos_C = FILTER_WHEEL_EX_MIN;

	_lampPos = LAMP_MIN;
	_lampPos_B = FALSE;
	_lampPos_C = LAMP_MIN;

	_shutterPos = SHUTTER_CLOSE;
	_shutterPos_B = FALSE;
	_shutterPos_C = SHUTTER_CLOSE;
	_shutterWaitTime = SHUTTER_WAIT_TIME_MIN;

	_shutterWaitTime_B = FALSE;
	_shutterWaitTime_C = SHUTTER_WAIT_TIME_MIN;

	_turretPos = TURRET_MIN;
	_turretPos_B = FALSE;
	_turretPos_C = TURRET_MIN;

	_xPos = 0;
	_xPos_B = FALSE;
	_xPos_C = 0;

	_yPos = 0;
	_yPos_B = FALSE;
	_yPos_C = 0;
}

SimDeviceFW::~SimDeviceFW()
{
	_instanceFlag = false;
}

bool SimDeviceFW::_instanceFlag = false;

auto_ptr<SimDeviceFW> SimDeviceFW::_single(new SimDeviceFW());

SimDeviceFW* SimDeviceFW::getInstance()
{
	if(! _instanceFlag)
	{
		_single.reset(new SimDeviceFW());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}


long SimDeviceFW::FindDevices(long &deviceCount)
{
	long ret = TRUE;

	deviceCount = 1;

	return ret;
}



long SimDeviceFW::SelectDevice(const long device)
{
	long ret = FALSE;

	switch(device)
	{
	case 0:
		{
			ret = TRUE;
		}
		break;
	default:
		{
		}
	}
	return ret;
}

long SimDeviceFW::TeardownDevice()
{
	return TRUE;
}


long SimDeviceFW::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;


	switch(paramID)
	{
	case PARAM_FW_EX_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = FILTER_WHEEL_EX_MIN;
			paramMax = FILTER_WHEEL_EX_MAX;
			paramDefault = FILTER_WHEEL_EX_DEFAULT;
		}
		break;
	case PARAM_FW_EM_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = FILTER_WHEEL_EM_MIN;
			paramMax = FILTER_WHEEL_EM_MAX;
			paramDefault = FILTER_WHEEL_EM_DEFAULT;
		}
		break;
	case PARAM_FW_DIC_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = FILTER_WHEEL_DIC_MIN;
			paramMax = FILTER_WHEEL_DIC_MAX;
			paramDefault = FILTER_WHEEL_DIC_DEFAULT;
		}
		break;
	case PARAM_SHUTTER_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = SHUTTER_CLOSE;
			paramMax = SHUTTER_OPEN;
			paramDefault = SHUTTER_DEFAULT;
		}
		break;
		
	case PARAM_TURRET_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = TURRET_MIN;
			paramMax = TURRET_MAX;
			paramDefault = TURRET_DEFAULT;
		}
		break;
		
	case PARAM_LAMP_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = LAMP_MIN;
			paramMax = LAMP_MAX;
			paramDefault = LAMP_DEFAULT;
		}
		break;
		
	case PARAM_AUTOFOCUS_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = AUTOFOCUS_MIN;
			paramMax = AUTOFOCUS_MAX;
			paramDefault = AUTOFOCUS_DEFAULT;
		}
		break;
		
	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = FILTER_WHEEL_EM | FILTER_WHEEL_DIC | TURRET | LAMP | AUTOFOCUS;
		}
		break;

	case PARAM_SHUTTER_WAIT_TIME:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = SHUTTER_WAIT_TIME_MIN;
			paramMax = SHUTTER_WAIT_TIME_MAX;
			paramDefault = SHUTTER_WAIT_TIME_DEFAULT;
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = (double)ConnectionStatusType::CONNECTION_READY;
		}
		break;
	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long SimDeviceFW::SetParam(const long paramID, const double param)
{
	long ret = TRUE;

	switch(paramID)
	{
		
	case PARAM_FW_EX_POS:
		{
			if((param >= FILTER_WHEEL_EX_MIN) && (param <= FILTER_WHEEL_EX_MAX))
			{
				_filterExPos = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
		
	case PARAM_FW_EM_POS:
		{
			if((param >= FILTER_WHEEL_EM_MIN) && (param <= FILTER_WHEEL_EM_MAX))
			{
				_filterEmPos = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
		
	case PARAM_FW_DIC_POS:
		{
			if((param >= FILTER_WHEEL_DIC_MIN) && (param <= FILTER_WHEEL_DIC_MAX))
			{
				_filterDicPos = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	
		
	case PARAM_SHUTTER_POS:
		{
			if((param >= SHUTTER_CLOSE) && (param <= SHUTTER_OPEN))
			{
				_shutterPos = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
		
	case PARAM_TURRET_POS:
		{
			if((param >= TURRET_MIN) && (param <= TURRET_MAX))
			{
				_turretPos = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
		
	case PARAM_LAMP_POS:
		{
			if((param >= LAMP_MIN) && (param <= LAMP_MAX))
			{
				_lampPos = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
		
	case PARAM_AUTOFOCUS_POS:
		{
			if((param >= AUTOFOCUS_MIN) && (param <= AUTOFOCUS_MAX))
			{
				_afPos = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
		
	case PARAM_SHUTTER_WAIT_TIME:
		{
			if((param >= SHUTTER_WAIT_TIME_MIN) && (param <= SHUTTER_WAIT_TIME_MAX))
			{
				_shutterWaitTime = static_cast<long>(param);
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

long SimDeviceFW::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_FW_EX_POS:
		{
			param = static_cast<double>(_filterExPos);
		}
		break;
	case PARAM_FW_EM_POS:
		{
			param = static_cast<double>(_filterEmPos);
		}
		break;
		
	case PARAM_FW_DIC_POS:
		{
			param = static_cast<double>(_filterDicPos);
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(FILTER_WHEEL_EM | FILTER_WHEEL_DIC |TURRET | LAMP | AUTOFOCUS);
		}
		break;
	case PARAM_SHUTTER_POS:
		{
			param = static_cast<double>(_shutterPos_C);
		}
		break;
	case PARAM_SHUTTER_WAIT_TIME:
		{
			param = static_cast<double>(_shutterWaitTime_C);
		}
		break;
	case PARAM_TURRET_POS:
		{
			param = static_cast<double>(_turretPos);
		}
		break;
	case PARAM_LAMP_POS:
		{
			param = static_cast<double>(_lampPos);
		}
		break;
	case PARAM_AUTOFOCUS_POS:
		{
			param = static_cast<double>(_afPos);
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = (double)ConnectionStatusType::CONNECTION_READY;
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
long SimDeviceFW::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long SimDeviceFW::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long SimDeviceFW::SetParamString(const long paramID, wchar_t* str)
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
long SimDeviceFW::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long SimDeviceFW::PreflightPosition()
{
	return TRUE;
}

long SimDeviceFW::SetupPosition()
{

	if(_filterExPos != _filterExPos_C)
	{
		_filterExPos_C = _filterExPos;
		_filterExPos_B = TRUE;
	}
	
	if(_filterEmPos != _filterEmPos_C)
	{
		_filterEmPos_C = _filterEmPos;
		_filterEmPos_B = TRUE;
	}
	
	if(_filterDicPos != _filterExPos_C)
	{
		_filterDicPos_C = _filterDicPos;
		_filterDicPos_B = TRUE;
	}

	if(_shutterPos != _shutterPos_C)
	{
		_shutterPos_C = _shutterPos;
		_shutterPos_B = TRUE;
	}

	if(_shutterWaitTime != _shutterWaitTime_C)
	{
		_shutterWaitTime_C = _shutterWaitTime;
		_shutterWaitTime_B = TRUE;
	}

	if(_turretPos != _turretPos_C)
	{
		_turretPos_C = _turretPos;
		_turretPos_B = TRUE;
	}
	
	if(_lampPos != _lampPos_C)
	{
		_lampPos_C = _lampPos;
		_lampPos_B = TRUE;
	}
	
	if(_afPos != _afPos_C)
	{
		_afPos_C = _afPos;
		_afPos_B = TRUE;
	}
	return TRUE;
}


long SimDeviceFW::StartPosition()
{
	long ret = FALSE;


	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"SimDeviceFW StartPosition");

	ret = TRUE;

	return ret;
}

long SimDeviceFW::StatusPosition(long &status)
{
	long ret = TRUE;

	status = (long)IDevice::StatusType::STATUS_READY;

	return ret;
}

long SimDeviceFW::ReadPosition(DeviceType deviceType, double &pos)
{
	long ret = FALSE;

	return ret;
}

long SimDeviceFW::PostflightPosition()
{   
	return TRUE;
}

long SimDeviceFW::GetLastErrorMsg(wchar_t *msg,long size)
{
	return TRUE;
}