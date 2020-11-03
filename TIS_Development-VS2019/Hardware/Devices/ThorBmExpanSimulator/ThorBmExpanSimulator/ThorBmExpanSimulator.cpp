// ThorBmExpanSimulator.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "math.h"
#include "ThorBmExpanSimulator.h"

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

wchar_t message[256];

#define CLK_FRQ 16000000.0
#define FULL_STEPS_PER_ROTATION 200.0
#define MICROSTEP 16
#define MICROSTEPINDEX 4
#define STALLGUARD 5
#define HOMESPEED 500
#define HOMEACCELERATION 500
#define POSITIONSPEED 100
#define POSITIONACCELERATION 50
#define MAXCURRENT 1500


ThorBmExpanSimulator::ThorBmExpanSimulator()
{
	_deviceDetected = FALSE;
	_expIndex=EXP_DEFAULT;
	_posMode=POS_MODE_DEFAULT;
	_bmxpDevType = IDevice::BEAM_EXPANDER;
	_errMsg[0] = NULL;
	_mot0_Pos = POS_DEFAULT;
	_mot0_PosArray[0] = POS_DEFAULT;
	_mot1_Pos = POS_DEFAULT;
	_mot1_PosArray[0] = POS_DEFAULT;
}

ThorBmExpanSimulator::~ThorBmExpanSimulator()
{
	_instanceFlag = false;
	_errMsg[0] = 0;
}

bool ThorBmExpanSimulator:: _instanceFlag = false;

auto_ptr<ThorBmExpanSimulator> ThorBmExpanSimulator::_single(new ThorBmExpanSimulator());

ThorBmExpanSimulator *ThorBmExpanSimulator::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorBmExpanSimulator());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorBmExpanSimulator::FindDevices(long &deviceCount)
{
	long	ret = TRUE;


		deviceCount = 1;
		_deviceDetected = TRUE;

	return ret;
}

long ThorBmExpanSimulator::SelectDevice(const long device)
{
	long	ret = FALSE;

	if(FALSE == _deviceDetected)
	{
		wsprintf(_errMsg,L"The device has not been detected");
		return FALSE;
	}

	switch(device)
	{
	case 0:	
		{
		ret = TRUE; 
		}
		break;
	default:
		{
		wsprintf(_errMsg,L"The device index %d is invalid",device);
		}
	}

	return ret;
}
void ThorBmExpanSimulator::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

long ThorBmExpanSimulator::TeardownDevice()
{
	return TRUE;
}

long ThorBmExpanSimulator::GetParamInfo
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
			paramReadOnly = FALSE;
			paramMin = (double)ConnectionStatusType::CONNECTION_READY;
			paramMax = (double)ConnectionStatusType::CONNECTION_READY;
			paramDefault = (double)ConnectionStatusType::CONNECTION_READY;
		}
		break;
	case PARAM_EXP_RATIO:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = EXP_MIN;
			paramMax = EXP_MAX;
			paramDefault = EXP_DEFAULT;
		}
		break;
		
	case PARAM_MOT0_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = POS_MIN;
			paramMax = POS_MAX;
			paramDefault = POS_DEFAULT;
		}
		break;
		
	case PARAM_MOT1_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = POS_MIN;
			paramMax = POS_MAX;
			paramDefault = POS_DEFAULT;
		}
		break;
	case PARAM_BMEXP_MODE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = POS_MODE_MIN;
			paramMax = POS_MODE_MAX;
			paramDefault = POS_MODE_DEFAULT;
		}
		break;

	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = BEAM_EXPANDER;
		}
		break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long ThorBmExpanSimulator::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;
	long foundParam = TRUE;

	switch(paramID)
	{
	case PARAM_DEVICE_TYPE:
		{
			DeviceType type = static_cast<DeviceType>(static_cast<int>(param));
			if(type >= IDevice::DEVICE_TYPE_FIRST && type <= IDevice::DEVICE_TYPE_LAST)
			{
				_bmxpDevType = type;			
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	case PARAM_EXP_RATIO:
		{
			if((param >= EXP_MIN) && (param <= EXP_MAX))
			{
				_expIndex = static_cast<long>(param);
			
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	case PARAM_MOT0_POS:
		{
			if((param >= POS_MIN) && (param <= POS_MAX))
			{
				_mot0_Pos = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	case PARAM_MOT1_POS:
		{
			if((param >= POS_MIN) && (param <= POS_MAX))
			{
				_mot1_Pos=static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_BMEXP_MODE:
		{
			if((param >= POS_MODE_MIN) && (param <= POS_MODE_MAX))
			{
				_posMode = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	default:
		{
		ret = FALSE;
		foundParam = FALSE;
		}
	}

	if(FALSE == ret)
	{
		if(TRUE == foundParam)
		{
			wsprintf(_errMsg,L"Parameter (%d) out of range",paramID);
		}
		else
		{
			wsprintf(_errMsg,L"Parameter (%d) not implemented",paramID);
		}
	}

	return ret;
}

long ThorBmExpanSimulator::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(static_cast<int>(_bmxpDevType));			
		}
		break;
	case PARAM_EXP_RATIO:	
		{ 
			param = static_cast<double>(_expIndex); 
		}
		break;
	case PARAM_MOT0_POS:	
		{ 
			param = static_cast<double>(_mot0_Pos); 
		}
		break;
	case PARAM_MOT1_POS:
		{
			param = static_cast<double>(_mot1_Pos);
		}
		break;
	case PARAM_BMEXP_MODE:
		{
			param = static_cast<double>(_posMode);
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = static_cast<double>((double)ConnectionStatusType::CONNECTION_READY);
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
long ThorBmExpanSimulator::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorBmExpanSimulator::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorBmExpanSimulator::SetParamString(const long paramID, wchar_t* str)
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
long ThorBmExpanSimulator::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorBmExpanSimulator::PreflightPosition()
{
	return TRUE;
}

long ThorBmExpanSimulator::SetupPosition()
{
	return TRUE;
}

long ThorBmExpanSimulator::StartPosition()
{
	long	ret = FALSE;

	ret = TRUE;

	return ret;
}

long ThorBmExpanSimulator::StatusPosition(long &status)
{
	long	ret = TRUE;

	status = IDevice::STATUS_READY;
	return ret;
}

long ThorBmExpanSimulator::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	pos = static_cast<double>(_expIndex);
	ret = TRUE;

	return ret;
}

long ThorBmExpanSimulator::PostflightPosition()
{
	return TRUE;
}



long ThorBmExpanSimulator::GetLastErrorMsg(wchar_t * msg, long size)
{	
    wcsncpy_s(msg,size,_errMsg,256);
	return TRUE;
}
