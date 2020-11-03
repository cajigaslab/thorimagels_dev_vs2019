// DisconnectedDevice.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "DisconnectedDevice.h"

auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
wchar_t message[256];

DisconnectedDevice::DisconnectedDevice()
{    
	xPos = X_MIN;
	xPos_C =X_MIN;
	xPos_B=FALSE;
	
	yPos = Y_MIN;
	yPos_C =Y_MIN;
	yPos_B=FALSE;
	
	zPos = Z_MIN;
	zPos_C =Z_MIN;
	zPos_B=FALSE;
	
	load=LOAD_MIN;
	load_C=LOAD_MIN;
	load_B=FALSE;
	
	afPos=AUTOFOCUS_MIN;
	afPos_C=AUTOFOCUS_MIN;
	afPos_B=FALSE;
	
	shutterPos=SHUTTER_CLOSE;
	shutterPos_C=SHUTTER_CLOSE;
	shutterPos_B=FALSE;
	
	shutterWaitTime =SHUTTER_WAIT_TIME_MIN;
	shutterWaitTime_C = SHUTTER_WAIT_TIME_MIN;
	shutterWaitTime_B = FALSE;
}

DisconnectedDevice::~DisconnectedDevice()
{
	instanceFlag = false;
}

bool DisconnectedDevice::instanceFlag = false;

auto_ptr<DisconnectedDevice> DisconnectedDevice::_single(new DisconnectedDevice());

DisconnectedDevice* DisconnectedDevice::getInstance()
{
	if(! instanceFlag)
	{
		_single.reset(new DisconnectedDevice());
		instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}


long DisconnectedDevice::FindDevices(long &deviceCount)
{
	long ret = TRUE;

	deviceCount = 1;

	return ret;
}



long DisconnectedDevice::SelectDevice(const long device)
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

long DisconnectedDevice::TeardownDevice()
{
	return TRUE;
}


long DisconnectedDevice::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;


	switch(paramID)
	{
	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
			paramMax = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
			paramDefault = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
	case PARAM_X_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = X_MIN;
			paramMax = X_MAX;
			paramDefault = X_DEFAULT;
		}
		break;

	case PARAM_Y_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = Y_MIN;
			paramMax = Y_MAX;
			paramDefault = Y_DEFAULT;
		}
		break;
		
	case PARAM_Z_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = Z_MIN;
			paramMax = Z_MAX;
			paramDefault = Z_DEFAULT;
		}
		break;
		
	case PARAM_AUTOFOCUS_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = AUTOFOCUS_MIN;
			paramMax = AUTOFOCUS_MAX;
			paramDefault = AUTOFOCUS_DEFAULT;
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
		
	case PARAM_LOAD_AND_EJECT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = LOAD_MIN;
			paramMax = LOAD_MAX;
			paramDefault = LOAD_DEFAULT;
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = SHUTTER | STAGE_X | STAGE_Y | STAGE_Z |
				AUTOFOCUS | LAMP | FILTER_WHEEL_EM | FILTER_WHEEL_DIC | TURRET | PMT1 |PMT2 |POWER_REG |BEAM_EXPANDER |
				LASER1 | LASER2 | LASER3 | LASER4 | PINHOLE_WHEEL | PMT3 | PMT4 |SLM |STAGE_R |		
				LIGHT_PATH | PMT5 | PMT6 |STAGE_Z2 |CONTROL_UNIT | SPECTRUM_FILTER | BEAM_STABILIZER | POWER_REG2;
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

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long DisconnectedDevice::SetParam(const long paramID, const double param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_X_POS:
		{
			if((param >= X_MIN) && (param <= X_MAX))
			{
				xPos = static_cast<double>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_Y_POS:
		{
			if((param >= Y_MIN) && (param <= Y_MAX))
			{
				yPos = static_cast<double>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
		
	case PARAM_Z_POS:
		{
			if((param >= Z_MIN) && (param <= Z_MAX))
			{
				zPos = static_cast<double>(param);
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
				shutterPos = static_cast<long>(param);
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
				afPos = static_cast<long>(param);
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
				shutterWaitTime = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	case PARAM_LOAD_AND_EJECT:
		{
			if((param >= LOAD_MIN) && (param <= LOAD_MAX))
			{
				load_B = TRUE;
			}
			else
			{
				ret = FALSE;
			}

			if(FALSE == ret)
			{
				wsprintf(message,L"ThorImager Load outside limits");
				logDll->TLTraceEvent(ERROR_EVENT,1,message);
			}
		}
		break;
	default:
		ret = FALSE;
	}

	return ret;
}

long DisconnectedDevice::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_X_POS:
		{
			param = static_cast<double>(xPos_C);
		}
		break;
	case PARAM_Y_POS:
		{
			param = static_cast<double>(yPos_C);
		}
		break;
	case PARAM_Z_POS:
		{
			param = static_cast<double>(zPos_C);
		}
		break;
	case PARAM_AUTOFOCUS_POS:
		{
			param = static_cast<double>(zPos_C);
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(SHUTTER | STAGE_X | STAGE_Y | STAGE_Z |
				AUTOFOCUS | LAMP | FILTER_WHEEL_EM | FILTER_WHEEL_DIC |TURRET |PMT1 |PMT2 |POWER_REG |BEAM_EXPANDER |
				LASER1 | LASER2 | LASER3 |LASER4 | PINHOLE_WHEEL |PMT3 | PMT4 | SLM |STAGE_R |		
				LIGHT_PATH |PMT5 | PMT6 |STAGE_Z2 |	CONTROL_UNIT | SPECTRUM_FILTER | BEAM_STABILIZER | POWER_REG2);	//EPHYS is not part of user selection
		}
		break;
	case PARAM_SHUTTER_POS:
		{
			param = static_cast<double>(shutterPos_C);
		}
		break;
	case PARAM_SHUTTER_WAIT_TIME:
		{
			param = static_cast<double>(shutterWaitTime_C);
		}
		break;
	case PARAM_LOAD_AND_EJECT:
		{
			param = load;
		}
	case PARAM_CONNECTION_STATUS:
		{
			param = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
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
long DisconnectedDevice::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long DisconnectedDevice::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long DisconnectedDevice::SetParamString(const long paramID, wchar_t* str)
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
long DisconnectedDevice::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long DisconnectedDevice::PreflightPosition()
{
	return TRUE;
}

long DisconnectedDevice::SetupPosition()
{

	if(xPos != xPos_C)
	{
		xPos_C = xPos;
		xPos_B = TRUE;
	}

	if(yPos != yPos_C)
	{
		yPos_C = yPos;
		yPos_B = TRUE;
	}
	
	if(zPos != zPos_C)
	{
		zPos_C = zPos;
		zPos_B = TRUE;
	}
	
	if(afPos != afPos_C)
	{
		afPos_C = afPos;
		afPos_B = TRUE;
	}

	if(shutterPos != shutterPos_C)
	{
		shutterPos_C = shutterPos;
		shutterPos_B = TRUE;
	}

	if(shutterWaitTime != shutterWaitTime_C)
	{
		shutterWaitTime_C = shutterWaitTime;
		shutterWaitTime_B = TRUE;
	}
	if(load != load_C)
	{
		load_C = load;
		load_B = TRUE;
	}

	return TRUE;
}


long DisconnectedDevice::StartPosition()
{
	long ret = FALSE;


	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"DisconnectedDevice StartPosition");

	//Sleep(125);

	//MoveX;
	
	//MoveY;
	
	//MoveShutter;

	ret = TRUE;

	return ret;
}

long DisconnectedDevice::StatusPosition(long &status)
{
	long ret = TRUE;

	status = IDevice::STATUS_READY;

	return ret;
}

long DisconnectedDevice::ReadPosition(DeviceType deviceType, double &pos)
{
	long ret = TRUE;

	if(deviceType & STAGE_X)
	{
		pos = xPos_C;
	}
	
	if(deviceType & STAGE_Y)
	{
		pos = yPos_C;
	}
	
	if(deviceType & STAGE_Z)
	{
		pos = zPos_C;
	}
	
	if(deviceType & SHUTTER)
	{
		pos = shutterPos_C;
	}
	
	if(deviceType & AUTOFOCUS)
	{
		pos = afPos_C;
	}

	return ret;
}

long DisconnectedDevice::PostflightPosition()
{   
	return TRUE;
}


long DisconnectedDevice::GetLastErrorMsg(wchar_t *msg,long size)
{
	return TRUE;
}