//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorZStepper.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "Serial.h"
#include "ThorZStepper.h"
#include "ZStepperSetupXML.h"
#include "StrSafe.h"

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

/// <summary>
/// The message
/// </summary>
wchar_t message[256];

#define CLK_FRQ 16000000.0
#define FULL_STEPS_PER_ROTATION 200.0
//#define KEY_ACCEL_INTERVAL 800
//#define NUMSTEPS_CALIBRATION 3200
//#define DURATION_CALIBRATION 1.5
//#define PANEL_UPDATE_INTERVAL 250

/// <summary>
/// Prevents a default instance of the <see cref="ThorZStepper"/> class from being created.
/// </summary>
ThorZStepper::ThorZStepper()
{
	_zPos = 0;
	_zPos_C = 0;
	_zPos_B = FALSE;
	_deviceDetected = FALSE;
	_zZero = 0;
	_zStepsPerMM = 32000;
	_zZero_B = FALSE;
	_zStop = 0;
	_zStop_B = FALSE;
	_zVelocity = 256;
	_zVelocity_B = TRUE;
	_zMin=0;
	_zMax=0;
	_zInvert = FALSE;
	_threshold = .400;
	_errMsg[0] = 0;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorZStepper"/> class.
/// </summary>
ThorZStepper::~ThorZStepper()
{
	_instanceFlag = false;
	_errMsg[0] = 0;
}

bool ThorZStepper:: _instanceFlag = false;

auto_ptr<ThorZStepper> ThorZStepper::_single(new ThorZStepper());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorZStepper *.</returns>
ThorZStepper *ThorZStepper::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorZStepper());
		_instanceFlag = true;
	}
	return _single.get();
}

/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="deviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorZStepper::FindDevices(long &deviceCount)
{
	auto_ptr<ZStepperXML> pSetup(new ZStepperXML());
	long portID = 0, baudRate = 0, address = 0;

	pSetup->GetConnection(portID, baudRate, address);
	_deviceDetected = deviceCount = _serialPort.Open(portID, baudRate);
	if(!_deviceDetected)
	{
		StringCbPrintfW(message,MSG_SIZE,L"ThorZStepper FindDevices could not open serial port");
		LogMessage(message);
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorZStepper FindDevices could not open serial port or configuration file is not avaible.");

		// *TODO* perform an automatic scan of the available serial ports for the Z stepper
	}
	else
	{
		_serialPort.Close();
	}
	return _deviceDetected;	
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorZStepper::SelectDevice(const long device)
{
	if(!_deviceDetected)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"The device has not been detected");
		return FALSE;
	}

	if(0 != device)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE, L"The device index %d is invalid", device);
		return FALSE;
	}
	else
	{
		auto_ptr<ZStepperXML> pSetup(new ZStepperXML());
		long portID = 0, baudRate = 0, address = 0;

		pSetup->GetConnection(portID, baudRate, address);
		pSetup->GetRangeConfig(_zMin, _zMax, _threshold,_zInvert);

		_encoderPrescaler = _step2Encoder = _microStep = 1;
		_mmperRot=.1;

		pSetup->GetStepConfig(_microStep, _encoderPrescaler, _step2Encoder, _mmperRot);

		StringCbPrintfW(message, MSG_SIZE, L"ThorZStepper GetStepConfig returned %d %d %d %d", _microStep, static_cast<long>(_encoderPrescaler), _step2Encoder, static_cast<long>(_mmperRot));
		LogMessage(message);

		_zStepsPerMM = static_cast<long>(FULL_STEPS_PER_ROTATION / (.0625 * _mmperRot));

		if(!_serialPort.Open(portID, baudRate))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorZStepper FindDevices could not open serial port");
			LogMessage(message);
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorZStepper FindDevices could not open serial port or configuration file is not avaible.");

			return FALSE;
		}

		_address =  static_cast<unsigned char>(address);

		int fmVersion = 0;
		if(!GetFirmwareVersion(fmVersion) || 0 == fmVersion)
		{
			return FALSE; 
		}

		SetMicroStep(_microStep);
		SetPrescaler(_encoderPrescaler);

		_mm2EncPos = FULL_STEPS_PER_ROTATION * _microStep * _step2Encoder / _mmperRot;

		//set the default for the freewheeling to be disabled
		const int FREEWHEELING_PARAMTER = 204;
		const int FREEWHEELING_DISABLE = 100;
		SetStoredAxisParameter(FREEWHEELING_PARAMTER,FREEWHEELING_DISABLE);

		//set the default holding current
		//scale is 0-255
		//factory max holding current is 100

		const int HOLDING_CURRENT_PARAMETER = 7;
		const int HOLDING_CURRENT_VALUE = 50;
		SetStoredAxisParameter(HOLDING_CURRENT_PARAMETER,HOLDING_CURRENT_VALUE);

		//set the current location
		int encPos = GetEncPos();
		_zPos_C = _zPos = static_cast<double>((encPos / (double)_zStepsPerMM) - _zZero);
	}
	return TRUE; 
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorZStepper::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::TeardownDevice()
{
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
long ThorZStepper::GetParamInfo
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
	case PARAM_Z_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _zMin;
			paramMax = _zMax;
			paramDefault = Z_DEFAULT;
		}
		break;

	case PARAM_Z_POS_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _zMin;
			paramMax = _zMax;
			paramDefault = Z_DEFAULT;
		}
		break;	

	case PARAM_Z_ZERO:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 1;
			paramMax = 1;
			paramDefault = 1;
		}
		break;

	case PARAM_Z_STEPS_PER_MM:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = Z_STEPS_PER_MM_MIN;
			paramMax = Z_STEPS_PER_MM_MAX;
			paramDefault = Z_STEPS_PER_MM_DEFAULT;
		}
		break;
	case PARAM_Z_VELOCITY:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = Z_VELOCITY_MIN;
			paramMax = Z_VELOCITY_MAX;
			paramDefault = Z_VELOCITY_DEFAULT;
		}
		break;
	case PARAM_Z_ENABLE_HOLDING_VOLTAGE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;

	case PARAM_Z_STOP:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}

	case PARAM_Z_INVERT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}

	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = STAGE_Z | STAGE_Z2;
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramDefault = CONNECTION_READY; 
			paramMax = CONNECTION_UNAVAILABLE; 
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
long ThorZStepper::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;
	long foundParam = TRUE;

	switch(paramID)
	{
	case PARAM_Z_POS:
		{
			if((param >= _zMin) && (param <= _zMax))
			{
				_zPos = static_cast<double>(param);
				_zPos_B = TRUE;

			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	case PARAM_Z_ZERO:
		{
			if((param >= 1) && (param <= 1))
			{
				_zZero = param;
				_zZero_B = TRUE;
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	case PARAM_Z_STEPS_PER_MM:
		{
			if((param >= Z_STEPS_PER_MM_MIN) && (param <= Z_STEPS_PER_MM_MAX))
			{
				_zStepsPerMM = static_cast<long>(param);
				_mmperRot = FULL_STEPS_PER_ROTATION/_zStepsPerMM;
				_mm2EncPos=FULL_STEPS_PER_ROTATION*_microStep*_step2Encoder/_mmperRot;
			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_Z_VELOCITY:
		{
			if((param >= Z_VELOCITY_MIN) && (param <= Z_VELOCITY_MAX))
			{
				_zVelocity = static_cast<long>(param);
				_zVelocity_B = TRUE;
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	case PARAM_Z_ENABLE_HOLDING_VOLTAGE:
		{
			if((param >= 0) && (param <= 1))
			{
				_zEnableHoldingVoltage = static_cast<long>(param);
				_zEnableHoldingVoltage_B = TRUE;
			}
			else
			{
				ret = FALSE;
			}		

		}
		break;

	case PARAM_Z_STOP:
		{
			if((param >= 0) && (param <= 1))
			{
				_zStop = static_cast<long>(param);
				_zStop_B = TRUE;
			}
			else
			{
				ret = FALSE;
			}		

		}
		break;

	case PARAM_Z_INVERT:
		{
			if((param >= 0) && (param <= 1))
			{
				_zInvert = static_cast<long>(param);
				_zInvert = TRUE;
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
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) out of range",paramID);
		}
		else
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) not implemented",paramID);
		}
	}

	return ret;
}

long ThorZStepper::GetConnectionStatus()
{
	return (_deviceDetected) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorZStepper::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_CONNECTION_STATUS:
		{
			param = GetConnectionStatus();
		}
		break;
	case PARAM_Z_POS:	
		{ 
			param = static_cast<double>(_zPos); 
		}
		break;
	case PARAM_Z_POS_CURRENT:
		{
			Lock lock(_critSect);
			int encPos = GetEncPos();
			param = static_cast<double>((encPos/(double)_zStepsPerMM) - _zZero);

			if(_zInvert)
			{
				//invert the read value
				param *= -1;
			}
		}
		break;
	case PARAM_Z_ZERO:	
		{ 
			param = static_cast<double>(_zZero); 
		}
		break;
	case PARAM_Z_STEPS_PER_MM:
		{
			param = static_cast<double>(_zStepsPerMM);
		}
		break;
	case PARAM_Z_VELOCITY:
		{
			param = static_cast<double>(_zVelocity);
		}
		break;		
	case PARAM_Z_ENABLE_HOLDING_VOLTAGE:
		{
			param = static_cast<double>(_zEnableHoldingVoltage);
		}
		break;
	case PARAM_Z_STOP:	
		{
			param = static_cast<double>(_zStop);
		}
		break;
	case PARAM_Z_INVERT:
		{
			param = static_cast<double>(_zInvert);
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			param =static_cast<double>(STAGE_Z | STAGE_Z2);
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
long ThorZStepper::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorZStepper::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorZStepper::SetParamString(const long paramID, wchar_t* str)
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
long ThorZStepper::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::SetupPosition()
{
	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::StartPosition()
{
	long ret = FALSE;
	Lock lock(_critSect);

	if(_zZero_B)
	{
		SetPositionAsZero();
		_zZero = 0;
		_zPos = 0;
		_zZero_B = FALSE;		
		_zPos_C = _zPos = 0;
	}


	if(_zVelocity_B)
	{
		int motAccel = _zVelocity / 2;

		SetMaxAcceleration(motAccel);
		SetMaxPositionSpeed(_zVelocity);

		_zVelocity_B = FALSE;
	}

	if(_zEnableHoldingVoltage_B)
	{
		const int FREEWHEELING_PARAMTER = 204;
		if(_zEnableHoldingVoltage)
		{
			const int FREEWHEELING_ENABLE = 0;
			SetAxisParameter(FREEWHEELING_PARAMTER,FREEWHEELING_ENABLE);
			StringCbPrintfW(message,MSG_SIZE,L"ThorZStepper holding voltage enabled");
			LogMessage(message);
		}
		else
		{			
			const int FREEWHEELING_DISABLE = 100;
			SetAxisParameter(FREEWHEELING_PARAMTER,FREEWHEELING_DISABLE);
			StringCbPrintfW(message,MSG_SIZE,L"ThorZStepper holding voltage disabled");
			LogMessage(message);
		}
		_zEnableHoldingVoltage_B = FALSE;
	}

	if(_zStop_B)
	{
		Stop();
		_zStop_B = FALSE;
		_zPos_B = FALSE;
	}

	if(_zPos_B)
	{
		int actLocation;
		double zVal = _zPos;

		int encPos = GetEncPos();

		if(_zInvert)
		{
			//invert the output z value to be sent
			zVal *= -1;
			//invert the motor readback position
			encPos *= -1;
		}
		double motorZ = static_cast<double>((encPos/(double)_zStepsPerMM) - _zZero);
		double diff = (_zPos - motorZ)*((_zInvert==FALSE)?1:-1);

		//do not move when outside the limits
		if((motorZ < _zMin)||(motorZ > _zMax))
		{
			ret = TRUE;
			return ret;
		}

		StringCbPrintfW(message,MSG_SIZE,L"ThorZStepper move Z to  %d.%03d",static_cast<long>(zVal*1000),static_cast<long>((zVal-static_cast<long>(zVal)) * 1000000));
		LogMessage(message);

		wchar_t stageMessage[512];
		StringCbPrintfW(stageMessage,512,L"Large Z stage move (%d.%03d um). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the Z stage.\nPower cycling the Z Stage may resolve the issue.",static_cast<long>(diff*1000),static_cast<long>(abs(static_cast<long>((diff-static_cast<long>(diff)) * 1000000))));

		if((diff > _threshold)||(diff < (_threshold*-1.0)))
		{
			if(IDNO == MessageBox(NULL,stageMessage,L"",MB_YESNO))
			{
				return FALSE;
			}
		}
		//convert to encoder units
		actLocation = static_cast<int>((zVal + _zZero)* _zStepsPerMM);

		MoveEncPos(actLocation, FALSE);

		Sleep(10);
		_zPos_C = _zPos;
	}

	ret = TRUE;

	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorZStepper::StatusPosition(long &status)
{
	if(_deviceDetected == FALSE)
		return FALSE;

	Lock lock(_critSect);

	long	ret = TRUE;

	if(TargetReached()) 
	{
		status = IDevice::STATUS_READY;
		Sleep(10);
	}
	else
	{
		Sleep(10);
		status = IDevice::STATUS_BUSY;
	}
	return ret;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorZStepper::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	if((deviceType & (STAGE_Z | STAGE_Z2)))
	{
		int encPos = GetEncPos();

		pos = static_cast<double>((encPos/(double)_zStepsPerMM) - _zZero);
		ret = TRUE;
	}

	return ret;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Moves the enc position.
/// </summary>
/// <param name="encPos">The enc position.</param>
/// <param name="rel">The relative.</param>
/// <returns>long.</returns>
long ThorZStepper::MoveEncPos(int encPos, bool rel)
{
	int startEncPos = GetEncPos();
	int endEncPos;

	if(rel)
	{
		endEncPos = startEncPos + encPos;
	}
	else
	{
		endEncPos = encPos;
	}

	int ustep;

	int curEncPos = GetEncPos();

	ustep = (int) ((double) (endEncPos - curEncPos) / _step2Encoder);

	Move(ustep, true);

	return curEncPos;
}

/// <summary>
/// Goes to step position.
/// </summary>
/// <param name="stepPos">The step position.</param>
/// <param name="relative">The relative.</param>
/// <returns>long.</returns>
long ThorZStepper::GoToStepPos(int stepPos, bool relative)
{
	int startStepPos = GetActPos();
	int endStepPos;
	if(relative)
		endStepPos = startStepPos + stepPos;
	else
		endStepPos = stepPos;
	Move(endStepPos, false);

	while(!TargetReached())
	{
		Sleep(10);
	};

	return TRUE;
}

/// <summary>
/// Sets the position as zero.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::SetPositionAsZero()
{	
	const int SET_ACTUAL_POSITION = 1;
	const int SET_TARGET_POSITION = 0;
	SetEncorderPos(0);
	Stop();
	SetAxisParameter(SET_ACTUAL_POSITION, 0);
	SetAxisParameter(SET_TARGET_POSITION, 0);

	return TRUE;
}

/// <summary>
/// Sets the maximum position speed.
/// </summary>
/// <param name="maxSpeed">The maximum speed.</param>
/// <returns>long.</returns>
long ThorZStepper::SetMaxPositionSpeed(int maxSpeed)
{
	SetAxisParameter(4, maxSpeed);
	return TRUE;
}

/// <summary>
/// Sets the maximum acceleration.
/// </summary>
/// <param name="maxAccel">The maximum accel.</param>
/// <returns>long.</returns>
long ThorZStepper::SetMaxAcceleration(int maxAccel)
{
	SetAxisParameter(5, maxAccel);

	// acceleration=0~2047;
	// actual acceleratin= 16MHz/65535*2^PD (usteps/sec) PD=pulse divisor
	return TRUE;
}

/// <summary>
/// Sets the maximum current.
/// </summary>
/// <param name="maxCur">The maximum current.</param>
/// <returns>long.</returns>
long ThorZStepper::SetMaxCurrent(int maxCur)
{
	SetAxisParameter(6, maxCur);
	return TRUE;
}

/// <summary>
/// Sets the standby current.
/// </summary>
/// <param name="currentFactor">The current factor.</param>
/// <returns>long.</returns>
long ThorZStepper::SetStandbyCurrent(int currentFactor)
{
	SetAxisParameter(7, currentFactor);
	return TRUE;

	// 0 ~255 for TMCM 140 0~ 1500 (1500mA) for TMCM 110
}

/// <summary>
/// Sets the RFS speed.
/// </summary>
/// <param name="rfsSpeedIndex">Index of the RFS speed.</param>
/// <returns>long.</returns>
long ThorZStepper::SetRFSSpeed(int rfsSpeedIndex)
{
	SetAxisParameter(194, rfsSpeedIndex);
	return TRUE;
}

/// <summary>
/// Sets the RFS switch speed.
/// </summary>
/// <param name="rfsSwitchIndex">Index of the RFS switch.</param>
/// <returns>long.</returns>
long ThorZStepper::SetRFSSwitchSpeed(int rfsSwitchIndex)
{
	SetAxisParameter(195, rfsSwitchIndex);
	return TRUE;
}

/// <summary>
/// Movemms the distancemm.
/// </summary>
/// <param name="mmDistance">The mm distance.</param>
/// <returns>double.</returns>
double ThorZStepper::MovemmDistancemm(double mmDistance)
{
	int encPos = (int) (mmDistance * _mm2EncPos);
	return((double) MoveEncPos(encPos, true)) / _mm2EncPos;
}


/// <summary>
/// Move to mm position < This function can be called from back ground worker thread
/// </summary>
/// <param name="mmPosition">The mm position.</param>
/// <returns>double.</returns>
double ThorZStepper::MovetommPos(double mmPosition)
{
	int encPos = (int) (mmPosition * _mm2EncPos);
	return((double) MoveEncPos(encPos, false)) / _mm2EncPos;
}

/// <summary>
/// Motors velocity calculation.
/// </summary>
/// <param name="mmpers">The mmpers.</param>
/// <returns>long.</returns>
long ThorZStepper::MotorVelocCalc(double mmpers)
{
	int		pulsediv = GetPulseDiv();
	double	microstep = (double) GetMicroStep();
	double	fstepfrq = mmpers / _mmperRot * FULL_STEPS_PER_ROTATION;
	double	pulsefrq = fstepfrq * microstep;
	double	velocity = pulsefrq * 2048 * 32 * pow(2.0, pulsediv) / CLK_FRQ;
	return (int) velocity;
}

/// <summary>
/// Set the pos speed accel.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::Set_Pos_Speed_Accel()
{

	// Set speed and acceleration
	//	int motVelocity = (int) (tbar_MotSpeed->Value * tbar_MotSpeed->Value * 32);
	//	int motAccel = motVelocity / 2;

	int motVelocity = (int) (1 * 1 * 32);
	int motAccel = motVelocity / 2;
	if(motVelocity == 0)
	{
		motVelocity = 8;
		motAccel = 4;
	}

	if(motVelocity == 2048)
	{
		motVelocity = 2047;
	}

	SetMaxAcceleration(motAccel);
	SetMaxPositionSpeed(motVelocity);
	return motVelocity;
}

/// <summary>
/// Rotates the specified speed.
/// </summary>
/// <param name="speed">The speed.</param>
/// <param name="right">The right.</param>
/// <returns>long.</returns>
long ThorZStepper::Rotate(int speed, bool right)
{
	_dataBuffer[0] = _address;		// address
	if(right)
	{
		_dataBuffer[1] = 1;	// instruction: rotate right
	}
	else
	{
		_dataBuffer[1] = 2;	// Instruction: roate left
	}

	_dataBuffer[2] = 0;		// Type: don't care
	_dataBuffer[3] = 0;		// Motor number
	_dataBuffer[4] = (unsigned char) ((speed & 0xff000000) >> 24);	// value byte 0xff000000
	_dataBuffer[5] = (unsigned char) ((speed & 0x00ff0000) >> 16);	// value byte 0x00ff0000
	_dataBuffer[6] = (unsigned char) ((speed & 0x0000ff00) >> 8);	// value byte 0x0000ff00
	_dataBuffer[7] = (unsigned char) (speed & 0x000000ff);			// value byte 0x000000ff
	_dataBuffer[8] = 0;	// intitialize the checksum
	///byte;
	///
	for(int i = 0; i < 8; i++) 
	{
		_dataBuffer[8] += _dataBuffer[i];
	}

	_serialPort.SendData(_dataBuffer, 9);
	Echo();
	return TRUE;
}

/// <summary>
/// Moves the specified steps.
/// </summary>
/// <param name="steps">The steps.</param>
/// <param name="rel">The relative.</param>
/// <returns>long.</returns>
long ThorZStepper::Move(int steps, bool rel)
{
	_dataBuffer[0] = _address;	// address
	_dataBuffer[1] = 4;	// instruction
	_dataBuffer[2] = (unsigned char) rel;	// type 0:Absolute 1 Relative 2 Coordinate
	_dataBuffer[3] = 0;	// Motor
	_dataBuffer[4] = (unsigned char) ((steps & 0xff000000) >> 24);	// value byte 0xff000000
	_dataBuffer[5] = (unsigned char) ((steps & 0x00ff0000) >> 16);	// value byte 0x00ff0000
	_dataBuffer[6] = (unsigned char) ((steps & 0x0000ff00) >> 8);	// value byte 0x0000ff00
	_dataBuffer[7] = (unsigned char) (steps & 0x000000ff);			// value byte 0x000000ff
	_dataBuffer[8] = 0;	// intitialize the checksum
	///byte;
	///
	for(int i = 0; i < 8; i++)
	{
		_dataBuffer[8] += _dataBuffer[i];
	}

	_serialPort.SendData(_dataBuffer, 9);
	Echo();
	return TRUE;
}

/// <summary>
/// Returns whether the Target is reached.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::TargetReached()
{
	if(GetAxisParameter(8) == 1)
	{
		Sleep(5);
		return true;
	}
	else
	{
		return false;
	}
}

/// <summary>
/// Determines whether the stage is stopped.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::IsStop()
{
	if(GetAxisParameter(3) == 0)
		return true;
	else
		return false;
}

/// <summary>
/// Stops this stage.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::Stop()
{
	_dataBuffer[0] = _address;	// address
	_dataBuffer[1] = 3;	// Instruction: Stop
	_dataBuffer[2] = 0;	// Type: don't care
	_dataBuffer[3] = 0;
	_dataBuffer[4] = 0;
	_dataBuffer[5] = 0;
	_dataBuffer[6] = 0;
	_dataBuffer[7] = 0;
	_dataBuffer[8] = 4;
	_serialPort.SendData(_dataBuffer, 9);
	Echo();
	return TRUE;
}


/// <summary>
/// Get the actual position by driving pulses
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::GetActPos()
{
	return GetAxisParameter(1);
}

/// <summary>
/// Get the encorder position, prescaled and counted
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::GetEncPos()
{
	return GetAxisParameter(209);
}

/// <summary>
/// Set the encorder position
/// </summary>
/// <param name="posValue">The position value.</param>
/// <returns>long.</returns>
long ThorZStepper::SetEncorderPos(int posValue)
{
	SetAxisParameter(209, posValue);
	return TRUE;
}

/// <summary>
/// Get the Absolute encorder position,
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::GetAbsEncPos()
{
	return GetAxisParameter(215);
}

/// <summary>
/// Gets the pulse div.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::GetPulseDiv()
{
	return GetAxisParameter(154);
}

/// <summary>
/// Sets the micro step.
/// </summary>
/// <param name="microStep">The micro step.</param>
/// <returns>long.</returns>
long ThorZStepper::SetMicroStep(int microStep)
{
	int instrValue;
	switch(microStep)
	{
	case 64:	instrValue = 6; break;
	case 32:	instrValue = 5; break;
	case 16:	instrValue = 4; break;
	case 8:		instrValue = 3; break;
	case 4:		instrValue = 2; break;
	case 2:		instrValue = 1; break;
	case 1:		instrValue = 0; break;
	default:	instrValue = 6; break;
	}

	SetAxisParameter(140, instrValue);
	return TRUE;
}


/// <summary>
/// Gets the micro step.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::GetMicroStep()
{

	// return the microsteps
	int paraValue = GetAxisParameter(140);
	switch(paraValue)
	{
	case 6: return 64; break;
	case 5: return 32; break;
	case 4: return 16; break;
	case 3: return 8; break;
	case 2: return 4; break;
	case 1: return 2; break;
	case 0: return 1; break;
	}

	return 0;
}

/// <summary>
/// Sets the prescaler.
/// </summary>
/// <param name="scaleFactor">The scale factor.</param>
/// <returns>long.</returns>
long ThorZStepper::SetPrescaler(double scaleFactor)
{
	int instrValue;
	instrValue = (int) (scaleFactor * 2048);
	SetAxisParameter(210, instrValue);
	return TRUE;
}

/// <summary>
/// Gets the prescaler.
/// </summary>
/// <returns>double.</returns>
double ThorZStepper::GetPrescaler()
{
	int paraValue = GetAxisParameter(210);
	return (double) paraValue / 2048;
}

/// <summary>
/// Gets the standby current.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::GetStandbyCurrent()
{
	return GetAxisParameter(7);
}

/// <summary>
/// Gets the maximum accel.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::GetMaxAccel()
{
	return GetAxisParameter(5);
}

/// <summary>
/// <summary> Value for type: 0: Start, 1: Stop, 2: Status </summary> Search for the reference point
/// </summary>
/// <param name="type">The type.</param>
/// <returns>long.</returns>
long ThorZStepper::RefSearch(unsigned char type)
{
	_dataBuffer[0] = _address;		// address
	_dataBuffer[1] = 13;		// Instruction: Reference Search
	_dataBuffer[2] = type;	// Type

	// 0: START - start ref. search;
	// 1: STOP - abort ref. search;
	// 2: STATUS - get status;
	_dataBuffer[3] = 0;		// Motor/bank
	_dataBuffer[4] = 0;		// value don't
	///care;
	///
	_dataBuffer[5] = 0;		// value don't
	///care;
	///
	_dataBuffer[6] = 0;		// value don't
	///care;
	///
	_dataBuffer[7] = 0;		// value don't
	///care;
	///
	_dataBuffer[8] = 0;		// intitialize the checksum
	///byte;
	///
	for(int i = 0; i < 8; i++) 
	{
		_dataBuffer[8] += _dataBuffer[i];
	}

	_serialPort.SendData(_dataBuffer, 9);
	Echo();
	return TRUE;
}

/// <summary>
/// References the search start.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::RefSearchStart()
{
	RefSearch(0);
	return TRUE;
}

/// <summary>
/// References the search stop.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::RefSearchStop()
{
	RefSearch(1);
	return TRUE;
}

/// <summary>
/// References the search active.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::RefSearchActive()
{
	RefSearch(2);

	int value = GetBufferValue();
	if(value == 0)
		return false;
	else
		return true;
}

/// <summary>
/// Gets the axis parameter.
/// </summary>
/// <param name="parameter">The parameter.</param>
/// <returns>long.</returns>
long ThorZStepper::GetAxisParameter(int parameter)
{
	_dataBuffer[0] = _address;		// address
	_dataBuffer[1] = 6;		// Instruction: Get Axis Parameter
	_dataBuffer[2] = (unsigned char) parameter;	// Type

	// 0: Target (next) position(RW);
	// 1: Actual Positin(RW);
	// 2: Target (next) speed(RW);
	// 3: Actual speed(R);
	// 4: Max positioning speed(RWE);
	// 5: Max Acceleration(RWE);
	// 6: Abs Max current(RWE);
	// 7: Standby current(RWE);
	// 8: Target pos reached(R);
	// 9: Ref. switch status(R);
	// 10: Log. state of R switch(R);
	// 11: Log state of L switch(R);
	// 12: Right switch disable(RWE);
	// 13: Left switch disable(RWE);
	// 14: Steprate prescaler;
	// 15: 140: Micro Step, 6->64 microstep, 5->32 step, 4->16 step, 3->8 step, 2->4
	// step, 1->2 step 209: Encoder Position;
	// 210: Encorder Prescale;
	// 212: Max Encorder Deviation;
	// 215: Absolute Encoder Value;
	// Note: R - Readable (GAP) W - Writable (SAP) E - Auatomatically restored from
	// EPROM after reset or power-on
	_dataBuffer[3] = 0;	// Motor/bank
	_dataBuffer[4] = 0;	// value, don't care
	_dataBuffer[5] = 0;	// value, don't care
	_dataBuffer[6] = 0;	// value, don't care
	_dataBuffer[7] = 0;	// value, don't care
	_dataBuffer[8] = 0;	// intitialize the checksum
	///byte;
	///
	for(int i = 0; i < 8; i++) 
	{
		_dataBuffer[8] += _dataBuffer[i];
	}

	_serialPort.SendData(_dataBuffer, 9);
	Echo();
	return GetBufferValue();
}

/// <summary>
/// Sets the axis parameter.
/// </summary>
/// <param name="parameter">The parameter.</param>
/// <param name="value">The value.</param>
/// <returns>long.</returns>
long ThorZStepper::SetAxisParameter(int parameter, int value)
{
	_dataBuffer[0] = _address;	// address
	_dataBuffer[1] = 5;	// Instruction: Set Axis Parameter
	_dataBuffer[2] = (unsigned char) parameter;	// Type

	// 0: Target (next) position(RW);
	// 1: Actual Positin(RW);
	// 2: Target (next) speed(RW);
	// 3: Actual speed(R);
	// 4: Max positioning speed(RWE);
	// 5: Max Acceleration(RWE);
	// 6: Abs Max current(RWE);
	// 7: Standby current(RWE);
	// 8: Target pos reached(R);
	// 9: Ref. switch status(R);
	// 10: Log. state of R switch(R);
	// 11: Log state of L switch(R);
	// 12: Right switch disable(RWE);
	// 13: Left switch disable(RWE);
	// 14: Steprate prescaler;
	// 15: 209: Encoder Position;
	// 204: Freewheeling. Applying or releasing holding voltage.
	// 210: Encorder Prescale;
	// 212: Max Encorder Deviation;
	// 215: Absolute Encoder Value;
	// Note: R - Readable (GAP) W - Writable (SAP) E - Auatomatically restored from
	// EPROM after reset or power-on
	_dataBuffer[3] = 0;	// Motor/bank
	_dataBuffer[4] = (unsigned char) ((value & 0xff000000) >> 24);	// value byte 0xff000000
	_dataBuffer[5] = (unsigned char) ((value & 0x00ff0000) >> 16);	// value byte 0x00ff0000
	_dataBuffer[6] = (unsigned char) ((value & 0x0000ff00) >> 8);	// value byte 0x0000ff00
	_dataBuffer[7] = (unsigned char) (value & 0x000000ff);			// value byte 0x000000ff
	_dataBuffer[8] = 0;	// intitialize the checksum
	///byte;
	///
	for(int i = 0; i < 8; i++) 
	{
		_dataBuffer[8] += _dataBuffer[i];
	}

	_serialPort.SendData(_dataBuffer, 9);
	Echo();
	return TRUE;
}


/// <summary>
/// Sets the stored axis parameter.
/// </summary>
/// <param name="parameter">The parameter.</param>
/// <param name="value">The value.</param>
/// <returns>long.</returns>
long ThorZStepper::SetStoredAxisParameter(int parameter, int value)
{
	_dataBuffer[0] = _address;	// address
	_dataBuffer[1] = 7;	// Instruction: Set Axis Parameter
	_dataBuffer[2] = (unsigned char) parameter;	// Type

	// 0: Target (next) position(RW);
	// 1: Actual Positin(RW);
	// 2: Target (next) speed(RW);
	// 3: Actual speed(R);
	// 4: Max positioning speed(RWE);
	// 5: Max Acceleration(RWE);
	// 6: Abs Max current(RWE);
	// 7: Standby current(RWE);
	// 8: Target pos reached(R);
	// 9: Ref. switch status(R);
	// 10: Log. state of R switch(R);
	// 11: Log state of L switch(R);
	// 12: Right switch disable(RWE);
	// 13: Left switch disable(RWE);
	// 14: Steprate prescaler;
	// 15: 209: Encoder Position;
	// 204: Freewheeling. Applying or releasing holding voltage.
	// 210: Encorder Prescale;
	// 212: Max Encorder Deviation;
	// 215: Absolute Encoder Value;
	// Note: R - Readable (GAP) W - Writable (SAP) E - Auatomatically restored from
	// EPROM after reset or power-on
	_dataBuffer[3] = 0;	// Motor/bank
	_dataBuffer[4] = (unsigned char) ((value & 0xff000000) >> 24);	// value byte 0xff000000
	_dataBuffer[5] = (unsigned char) ((value & 0x00ff0000) >> 16);	// value byte 0x00ff0000
	_dataBuffer[6] = (unsigned char) ((value & 0x0000ff00) >> 8);	// value byte 0x0000ff00
	_dataBuffer[7] = (unsigned char) (value & 0x000000ff);			// value byte 0x000000ff
	_dataBuffer[8] = 0;	// intitialize the checksum
	///byte;
	///
	for(int i = 0; i < 8; i++) 
	{
		_dataBuffer[8] += _dataBuffer[i];
	}

	_serialPort.SendData(_dataBuffer, 9);
	Echo();
	return TRUE;
}

/// <summary>
/// Gets the firmware version.
/// </summary>
/// <param name="firmwareRevision">The firmware revision.</param>
/// <returns>long.</returns>
long ThorZStepper::GetFirmwareVersion(int& firmwareRevision)
{
	long ret = FALSE;
	const int GET_FIRMWARE_VERSION_CMD = 136;

	_dataBuffer[0] = _address;	// address
	_dataBuffer[1] = GET_FIRMWARE_VERSION_CMD;	// Instruction: Get firmware version
	_dataBuffer[2] = (unsigned char) 1;	// Type 1: string, 0: binary
	_dataBuffer[3] = 0;	// Motor/bank
	_dataBuffer[4] = 0;	// value do not care
	_dataBuffer[5] = 0;	// value do not care
	_dataBuffer[6] = 0;	// value do not care
	_dataBuffer[7] = 0;	// value do not care
	_dataBuffer[8] = 0;	// intitialize the checksum

	for(int i = 0; i < 8; i++) 
	{
		_dataBuffer[8] += _dataBuffer[i];
	}

	_serialPort.SendData(_dataBuffer, 9);
	Echo();

	if(VerifyResponse(GET_FIRMWARE_VERSION_CMD))
	{
		firmwareRevision = GetBufferValue();
		ret = TRUE;
	}

	return ret;
}

/// <summary>
/// Verifies the response.
/// </summary>
/// <param name="commandNumber">The command number.</param>
/// <returns>long.</returns>
long ThorZStepper::VerifyResponse(int commandNumber)
{
	if((_readBuffer[0] & 2) && (_readBuffer[1] & _address) && 
		(_readBuffer[2] & 100) && (_readBuffer[3] & commandNumber))
	{
		return TRUE;
	}
	return FALSE;
}

/// <summary>
/// Gets the buffer value.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::GetBufferValue()
{
	int value = 0;
	int byteint;
	byteint = (int) _readBuffer[4];
	value += byteint << 24;
	byteint = (int) _readBuffer[5];
	value += byteint << 16;
	byteint = (int) _readBuffer[6];
	value += byteint << 8;
	byteint = (int) _readBuffer[7];
	value += byteint;
	return value;
}

/// <summary>
/// Echoes this instance.
/// </summary>
/// <returns>long.</returns>
long ThorZStepper::Echo()
{	
	int timeout_ms = 500; //half-second time out
	DWORD start = GetTickCount();

	while((_serialPort.ReadDataWaiting() < 9) && (GetTickCount() - start < (DWORD)timeout_ms))
	{
		Sleep(10);
	}

	_serialPort.ReadData(_readBuffer, 9);
	return TRUE;
}


/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorZStepper::GetLastErrorMsg(wchar_t * msg, long size)
{	
	wcsncpy_s(msg,size,_errMsg,256);
	return TRUE;
}
