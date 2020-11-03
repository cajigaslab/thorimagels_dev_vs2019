// SimDeviceXY.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SimDeviceXY.h"

auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
wchar_t message[256];

SimDeviceXY::SimDeviceXY()
{    
	_xMin = -110;
	_xMax = 110;
	_yMin = -75;
	_yMax = 75;
	_xDefault = 0;
	_yDefault = 0;

	_xPos			= 0;
	_xPos_Current	= 0;
	_xPos_Last		= _xMin-1;
	_xPos_Pending	= FALSE;

	_yPos			= 0;
	_yPos_Last		= _yMin-1;
	_yPos_Current	= 0;
	_yPos_Pending	= FALSE;

	_xMoveComplete	= true;
	_yMoveComplete	= true;

	_xHome_Pending	= FALSE;
	_yHome_Pending	= FALSE;

	_xVel			= X_VELOCITY_DEFAULT;
	_xVel_Current	= X_VELOCITY_DEFAULT;
	_xVel_Last		= (double)X_VELOCITY_DEFAULT-1;

	_xAcc			= ACCELERATION_DEFAULT;
	_xAcc_Current	= ACCELERATION_DEFAULT;
	_xVel_Pending	= TRUE;

	_yVel			= Y_VELOCITY_DEFAULT;
	_yVel_Current	= Y_VELOCITY_DEFAULT;
	_yVel_Last		= (double)Y_VELOCITY_DEFAULT-1;

	_yAcc			= ACCELERATION_DEFAULT;
	_yAcc_Current	= (double)ACCELERATION_DEFAULT-1;
	_yVel_Pending	= TRUE;
	
	_load=LOAD_MIN;
	_load_C=LOAD_MIN;
	_load_B=FALSE;
	
	_afPos=AUTOFOCUS_MIN;
	_afPos_C=AUTOFOCUS_MIN;
	_afPos_B=FALSE;
	
	_shutterPos=SHUTTER_CLOSE;
	_shutterPos_C=SHUTTER_CLOSE;
	_shutterPos_B=FALSE;
	
	_shutterWaitTime =SHUTTER_WAIT_TIME_MIN;
	_shutterWaitTime_C = SHUTTER_WAIT_TIME_MIN;
	_shutterWaitTime_B = FALSE;

	_blankingPeriod = {};
	_direction = {};
	_encoderPerFrame = {};
	_fStart = {};
	_fcnt = {};
	_fractional = {};
	_home = {};
	_increment = {};
	_qdec_settings = {};
	_run = {};
	_tdi_settings = {};
	_triggerPulses = {};
}

SimDeviceXY::~SimDeviceXY()
{
	_instanceFlag = false;
}

bool SimDeviceXY::_instanceFlag = false;

auto_ptr<SimDeviceXY> SimDeviceXY::_single(new SimDeviceXY());

SimDeviceXY* SimDeviceXY::getInstance()
{
	if(! _instanceFlag)
	{
		_single.reset(new SimDeviceXY());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long SimDeviceXY::FindDevices(long &deviceCount)
{
	long ret = TRUE;

	deviceCount = 1;

	return ret;
}

long SimDeviceXY::SelectDevice(const long device)
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

long SimDeviceXY::TeardownDevice()
{
	return TRUE;
}

long SimDeviceXY::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;


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
	case PARAM_X_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _xMin;
			paramMax = _xMax;
			paramDefault = _xDefault;
		}
		break;

	case PARAM_X_ZERO:
		{
		    paramType = TYPE_BOOL;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = _xDefault;
		}
		break;

	case PARAM_Y_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _yMin;
			paramMax = _yMax;
			paramDefault = _yDefault;
		}
		break;
	case PARAM_Y_ZERO:
		{
		    paramType = TYPE_BOOL;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = _xDefault;
		}
		break;
	case PARAM_X_HOME:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = X_HOME_MIN;
			paramMax = X_HOME_MAX;
			paramDefault = X_HOME_DEFAULT;
		}
		break;

	case PARAM_Y_HOME:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = Y_HOME_MIN;
			paramMax = Y_HOME_MAX;
			paramDefault = Y_HOME_DEFAULT;
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
			paramMin = paramMax = paramDefault = STAGE_X | STAGE_Y | AUTOFOCUS;
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
		case PARAM_X_VELOCITY:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = X_VELOCITY_MIN;
			paramMax = X_VELOCITY_MAX;
			paramDefault = X_VELOCITY_DEFAULT;
		}
		break;

	case PARAM_Y_VELOCITY:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = Y_VELOCITY_MIN;
			paramMax = Y_VELOCITY_MAX;
			paramDefault = Y_VELOCITY_DEFAULT;
		}
		break;
	case PARAM_Y_ACCEL:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = ACCELERATION_DEFAULT;
			paramDefault = ACCELERATION_DEFAULT;
		}
		break;

	case PARAM_X_POS_CURRENT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _xMin;
			paramMax = _xMax;
			paramDefault = _xDefault;
		}
		break;		
	case PARAM_Y_POS_CURRENT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _yMin;
			paramMax = _yMax;
			paramDefault = _yDefault;
		}
		break;
	case PARAM_DECODER_INCREMENT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PARAM_DECODER_INCREMENT_MIN;
			paramMax = PARAM_DECODER_INCREMENT_MAX;
			paramDefault = PARAM_DECODER_INCREMENT_MIN;
		}
		break;		

	case PARAM_DECODER_BLANKING_PERIOD:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PARAM_DECODER_BLANKING_PERIOD_MIN;
			paramMax = PARAM_DECODER_BLANKING_PERIOD_MAX;
			paramDefault = PARAM_DECODER_BLANKING_PERIOD_MIN;
		}
		break;	

	case PARAM_DECODER_FRACTIONAL:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PARAM_DECODER_FRACTIONAL_MIN;
			paramMax = PARAM_DECODER_FRACTIONAL_MAX;
			paramDefault = PARAM_DECODER_FRACTIONAL_MIN;
		}
		break;
	case PARAM_DECODER_TRIGGER_PULSES_PER_FRAME:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PARAM_DECODER_TRIGGER_PULSES_PER_FRAME_MIN;
			paramMax = PARAM_DECODER_TRIGGER_PULSES_PER_FRAME_MAX;
			paramDefault = PARAM_DECODER_TRIGGER_PULSES_PER_FRAME_MIN;
		}
		break;	
	case PARAM_DECODER_DIRECTION:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PARAM_DECODER_DIRECTION_MIN;
			paramMax = PARAM_DECODER_DIRECTION_MAX;
			paramDefault = PARAM_DECODER_DIRECTION_MIN;
		}
		break;
	case PARAM_DECODER_HOME_POSITION:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PARAM_DECODER_HOME_POSITION_MIN;
			paramMax = PARAM_DECODER_HOME_POSITION_MAX;
			paramDefault = PARAM_DECODER_HOME_POSITION_MIN;
		}
		break;	

	case PARAM_DECODER_FSTART:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PARAM_DECODER_FSTART_MIN;
			paramMax = PARAM_DECODER_FSTART_MAX;
			paramDefault = PARAM_DECODER_FSTART_MIN;
		}
		break;	

	case PARAM_DECODER_ENCODER_PER_FRAME:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PARAM_DECODER_ENCODER_PER_FRAME_MIN;
			paramMax = PARAM_DECODER_ENCODER_PER_FRAME_MAX;
			paramDefault =PARAM_DECODER_ENCODER_PER_FRAME_MIN;
		}
		break;	

	case PARAM_DECODER_RUN:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PARAM_DECODER_RUN_MIN;
			paramMax = PARAM_DECODER_RUN_MAX;
			paramDefault = PARAM_DECODER_RUN_MIN;
		}
		break;	
		
	case PARAM_DECODER_FRAME_COUNT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PARAM_DECODER_FRAME_COUNT_MIN;
			paramMax = PARAM_DECODER_FRAME_COUNT_MAX;
			paramDefault = PARAM_DECODER_FRAME_COUNT_MIN;
		}
		break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long SimDeviceXY::SetParam(const long paramID, const double param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_X_POS:
		{
			if((param >= _xMin) && (param <= _xMax))
			{
				_xPos = static_cast<double>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
    case PARAM_X_ZERO:
		{
			if(param == 1)
			{
				_xPos = static_cast<double>(0);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	case PARAM_Y_POS:
		{
			if((param >= _yMin) && (param <= _yMax))
			{
				_yPos = static_cast<double>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
    case PARAM_Y_ZERO:
		{
			if(param == 1)
			{
				_yPos = static_cast<double>(0);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	case PARAM_X_HOME:
		{
			if((param >= X_HOME_MIN) && (param <= X_HOME_MAX))
			{
				_xHome_Pending = TRUE;
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	case PARAM_Y_HOME:
		{
			if((param >= Y_HOME_MIN) && (param <= Y_HOME_MAX))
			{
				_yHome_Pending = TRUE;
			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_X_VELOCITY:
		{
			if((param >= X_VELOCITY_MIN) && (param <= X_VELOCITY_MAX))
			{
				_xVel = static_cast<double>(param);
			}		
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_Y_VELOCITY:
		{
			if((param >= Y_VELOCITY_MIN) && (param <= Y_VELOCITY_MAX))
			{
				_yVel = static_cast<double>(param);
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

	case PARAM_LOAD_AND_EJECT:
		{
			if((param >= LOAD_MIN) && (param <= LOAD_MAX))
			{
				_load_B = TRUE;
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

	case PARAM_DECODER_INCREMENT:
		{
			_increment.pos = static_cast<long>(param);
		}
		break;

	case PARAM_DECODER_BLANKING_PERIOD:
		{		
			_blankingPeriod.pos = static_cast<long>(param);
		}
		break;	

	case PARAM_DECODER_FRACTIONAL:
		{
			_fractional.pos = static_cast<long>(param);
		}
		break;
	case PARAM_DECODER_TRIGGER_PULSES_PER_FRAME:
		{
			_triggerPulses.pos  = static_cast<long>(param);
		}
		break;	
	case PARAM_DECODER_DIRECTION:
		{
			_direction.pos = static_cast<long>(param);
		}
		break;
	case PARAM_DECODER_HOME_POSITION:
		{
			_home.pos = static_cast<long>(param);
		}
		break;	

	case PARAM_DECODER_FSTART:
		{
			_fStart.pos = static_cast<long>(param);
		}
		break;	

	case PARAM_DECODER_ENCODER_PER_FRAME:
		{
			_encoderPerFrame.pos = static_cast<long>(param);
		}
		break;	

	case PARAM_DECODER_RUN:
		{
			//Initialize everything if set to RUN_MODE_PRE_RUN (3)
			if(param == 3)
			{
				_qdec_settings.qcr1 = 0;
				_qdec_settings.qcr2 = 0;
				_qdec_settings.qsr = 0;

				memset((void*)&_tdi_settings,0,sizeof(TDI_SETTINGS));
				memset((void*)&_increment,0,sizeof(State));
				memset((void*)&_blankingPeriod,0,sizeof(State));
				memset((void*)&_fractional,0,sizeof(State));
				memset((void*)&_triggerPulses,0,sizeof(State));
				memset((void*)&_home,0,sizeof(State));
				memset((void*)&_direction,0,sizeof(State));
				memset((void*)&_fStart,0,sizeof(State));
				memset((void*)&_encoderPerFrame,0,sizeof(State));
				memset((void*)&_run,0,sizeof(State));
				memset((void*)&_fcnt,0,sizeof(State));

				_home.pos_C = 1;
			}

			_run.pos = static_cast<long>(param);
		}
		break;	
		
	case PARAM_DECODER_FRAME_COUNT:
		{
			_fcnt.pos = static_cast<long>(param);
		}
		break;	
		
	case PARAM_X_WAIT_UNTIL_SETTLED:
		{
			WaitUntilSettled();
		}
		break;
	case PARAM_DECODER_APPLY:
		{
			DecoderSetupPosition ();
			DecoderStartPosition ();
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
long SimDeviceXY::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long SimDeviceXY::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long SimDeviceXY::SetParamString(const long paramID, wchar_t* str)
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
long SimDeviceXY::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long SimDeviceXY::WaitUntilSettled()
{
	return TRUE;
}

void SimDeviceXY::DecoderCheckState(State &state)
{
	//Cacheing had to disabled because the junk error values were being sent to the FPGA
	//the first time without the all of the values being properly initiated.
	if(state.pos != state.pos_C)
	{
		state.pos_B = TRUE;
	}
}

long SimDeviceXY::DecoderSetupPosition()
{
	  DecoderCheckState(_increment);
	  DecoderCheckState(_blankingPeriod);
	  DecoderCheckState(_fractional);
	  DecoderCheckState(_triggerPulses);
	  DecoderCheckState(_home);
	  DecoderCheckState(_direction);
	  DecoderCheckState(_fStart);
	  DecoderCheckState(_encoderPerFrame);
	  DecoderCheckState(_run);
	  DecoderCheckState(_fcnt);

	return TRUE;
}

long SimDeviceXY::DecoderStartPosition()
{
	long	ret = FALSE;

	if(_increment.pos_B)
	{
		_increment.pos_C = _increment.pos;
		_increment.pos_B = FALSE;
	}	

	if(_blankingPeriod.pos_B)
	{
		_blankingPeriod.pos_C = _blankingPeriod.pos;
		_blankingPeriod.pos_B = FALSE;
	}	

	if(_fractional.pos_B)
	{
		_fractional.pos_C = _fractional.pos;
		_fractional.pos_B = FALSE;

	}	

	if(_triggerPulses.pos_B)
	{
		_triggerPulses.pos_C = _triggerPulses.pos;
		_triggerPulses.pos_B = FALSE;

	}	

	if(_direction.pos_B)
	{
		_direction.pos_C = _direction.pos;
		_direction.pos_B = FALSE;
	}	

	if(_fStart.pos_B)
	{
		_fStart.pos_C = _fStart.pos;
		_fStart.pos_B = FALSE;

	}	

	if(_encoderPerFrame.pos_B)
	{
		_encoderPerFrame.pos_C = _encoderPerFrame.pos;
		_encoderPerFrame.pos_B = FALSE;

	}	

	if(_run.pos_B)
	{
		_run.pos_C = _run.pos = -1;//Added this back in.  Guessing this is necessary if caching again
		_run.pos_B = FALSE;

	}	

	if(_home.pos_B)
	{
		//DecoderPositionStatus (returned_home_pos);
		_home.pos_C = _home.pos;
		_home.pos_B = FALSE;

	}	

	if(_fcnt.pos_B)
	{
		_fcnt.pos_C = _fcnt.pos;
		_fcnt.pos_B = FALSE;
	}	

	return TRUE;

}

long SimDeviceXY::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_X_POS:
		{
			param = static_cast<double>(_xPos);
		}
		break;
	case PARAM_Y_POS:
		{
			param = static_cast<double>(_yPos);
		}
		break;
	case PARAM_X_POS_CURRENT:
		{
			ReadPosition(STAGE_X,param);
		}
		break;
	case PARAM_Y_POS_CURRENT:
		{
			ReadPosition(STAGE_Y,param);
		}
		break;
	case PARAM_X_HOME:
		{
			param = X_HOME_MIN;
		}
		break;
	case PARAM_Y_HOME:
		{
			param = Y_HOME_MIN;
		}
		break;
	case PARAM_X_VELOCITY:
		{
			param = static_cast<double>(_xVel);
		}
		break;
	case PARAM_Y_VELOCITY:
		{
			param = static_cast<double>(_yVel);
		}
		break;
	case PARAM_X_ACCEL:
		{
			param = static_cast<double>(_xAcc);
		}
		break;
	case PARAM_Y_ACCEL:
		{
			param = static_cast<double>(_yAcc);
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(STAGE_X | STAGE_Y | AUTOFOCUS);
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
	case PARAM_LOAD_AND_EJECT:
		{
			param = _load;
		}
		break;
		
	case PARAM_AUTOFOCUS_POS:
		{
			param = _afPos;		
		}
		break;
	case PARAM_DECODER_INCREMENT:
		{
			param = _increment.pos;
		}
		break;

	case PARAM_DECODER_BLANKING_PERIOD:
		{		
			param = _blankingPeriod.pos;
		}
		break;	

	case PARAM_DECODER_FRACTIONAL:
		{
			param = _fractional.pos;
		}
		break;
	case PARAM_DECODER_TRIGGER_PULSES_PER_FRAME:
		{
			param = _triggerPulses.pos;
		}
		break;	
	case PARAM_DECODER_DIRECTION:
		{
			param = _direction.pos;
		}
		break;
	case PARAM_DECODER_HOME_POSITION:
		{
			param = _home.pos;
		}
		break;	

	case PARAM_DECODER_FSTART:
		{
			param = _fStart.pos;
		}
		break;	

	case PARAM_DECODER_ENCODER_PER_FRAME:
		{
			param = _encoderPerFrame.pos;
		}
		break;	

	case PARAM_DECODER_RUN:
		{
			param = _run.pos;
		}
		break;	
		
	case PARAM_DECODER_FRAME_COUNT:
		{
			param = _fcnt.pos;
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

long SimDeviceXY::PreflightPosition()
{
	return TRUE;
}

long SimDeviceXY::SetupPosition()
{

	if (_xPos_Last != _xPos)
	{
		_xPos_Last = _xPos;
		_xPos_Pending = TRUE;
	}

	if (_yPos_Last != _yPos)
	{
		_yPos_Last = _yPos;
		_yPos_Pending = TRUE;
	}

	if (_xVel_Last != _xVel)
	{
		_xVel_Last = _xVel;
		_xVel_Pending = TRUE;
	}

	if (_yVel_Last != _yVel)
	{
		_yVel_Last = _yVel;
		_yVel_Pending = TRUE;
	}
	
	if(_afPos != _afPos_C)
	{
		_afPos_C = _afPos;
		_afPos_B = TRUE;
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
	if(_load != _load_C)
	{
		_load_C = _load;
		_load_B = TRUE;
	}

	return TRUE;
}


long SimDeviceXY::StartPosition()
{
	long ret = FALSE;


	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"SimDeviceXY StartPosition");

	if (_xVel_Pending)
	{
		_xVel_Current = _xVel;
		_xAcc_Current = _xAcc;

		ret = TRUE;

		_xVel_Pending = FALSE;
	}

	if (_yVel_Pending)
	{
		_yVel_Current = _yVel;
		_yAcc_Current = _yAcc;

		ret = TRUE;

		_yVel_Pending = FALSE;

	}

	if(_xHome_Pending)
	{
		_xHome_Pending = FALSE;
	}


	if(_yHome_Pending)
	{		
		_yHome_Pending = FALSE;
	}

	if (_xPos_Pending)
	{
		_xPos_Current = _xPos;
		ret				= TRUE;					
		_xPos_Pending	= FALSE;

	}

	if (_yPos_Pending)
	{
		_yPos_Current = _yPos;
		ret				= TRUE;
		_yPos_Pending	= FALSE;

	}

	ret = TRUE;

	return ret;
}

long SimDeviceXY::StatusPosition(long &status)
{
	long ret = TRUE;

	status = IDevice::STATUS_READY;

	return ret;
}

long SimDeviceXY::ReadPosition(DeviceType deviceType, double &pos)
{
	long ret = TRUE;

	if(deviceType & STAGE_X)
	{
		pos = _xPos_Current;
	}
	
	if(deviceType & STAGE_Y)
	{
		pos = _yPos_Current;
	}	
	
	if(deviceType & SHUTTER)
	{
		pos = _shutterPos_C;
	}
	
	if(deviceType & AUTOFOCUS)
	{
		pos = _afPos_C;
	}

	return ret;
}

long SimDeviceXY::PostflightPosition()
{   
	return TRUE;
}

long SimDeviceXY::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}