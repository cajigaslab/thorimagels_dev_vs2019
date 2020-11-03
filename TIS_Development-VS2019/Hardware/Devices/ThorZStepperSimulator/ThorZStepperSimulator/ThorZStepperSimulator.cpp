//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorZStepperSimulator.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "Serial.h"
#include "ThorZStepperSimulator.h"
#include "ZStepperSetupXML.h"

//auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
wchar_t message[256];

#define CLK_FRQ 16000000.0
#define FULL_STEPS_PER_ROTATION 200.0
#define KEY_ACCEL_INTERVAL 800
#define NUMSTEPS_CALIBRATION 3200
#define DURATION_CALIBRATION 1.5
#define PANEL_UPDATE_INTERVAL 250
#define ENCODER_PRESCALER 3.125
//physical encoder has 1024 position per rotation,
//scale it up to 3200 positions to match 16 microsteps, because 16x200=3200
#define MICROSTEP 16
#define MICROSTEPINDEX 4
#define STEP2ENCODER 1

ThorZStepperSimulator::ThorZStepperSimulator()
{
	_zPos = Z_MIN;
	_zPos_C = Z_MIN;
	_zPos_B = FALSE;
	_deviceDetected = FALSE;
	_zZero = 0;
	_zZero_B = FALSE;
	_zStepsPerMM = 32000;
	_zDevType = IDevice::STAGE_Z | IDevice::STAGE_Z2;
}

ThorZStepperSimulator::~ThorZStepperSimulator()
{
	_instanceFlag = false;
}

bool ThorZStepperSimulator:: _instanceFlag = false;

auto_ptr<ThorZStepperSimulator> ThorZStepperSimulator::_single(new ThorZStepperSimulator());

ThorZStepperSimulator *ThorZStepperSimulator::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorZStepperSimulator());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorZStepperSimulator::FindDevices(long &deviceCount)
{
	long ret = TRUE;

	////Get filter parameters from hardware setup.xml
	//auto_ptr<ZStepperXML> pSetup(new ZStepperXML());

	long portID=0;
	long baudRate=0;
	long address=0;
	//pSetup->GetConnection(portID,baudRate,address);

	//if(FALSE == _serialPort.Open(portID, baudRate))
	//{
	//	logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorZStepperSimulator FindDevices could not open serial port");

	//	// *TODO* perform an automatic scane of the available serial ports for the Z stepper
	//	deviceCount = 0;
	//	ret = FALSE;
	//}
	//else
	//{
	//	deviceCount = 1;
	//	_deviceDetected = TRUE;
	//	_address = address;

	//	 SetMicroStep(MICROSTEP);
	//	 SetPrescaler(ENCODER_PRESCALER);

	//	_mmperRot=100;
	//	_mm2EncPos=FULL_STEPS_PER_ROTATION*MICROSTEP*STEP2ENCODER/_mmperRot;
	//}

	deviceCount = 1;
	_deviceDetected = TRUE;
	_address = static_cast<unsigned char>(address);	

	_mmperRot=100;
	_mm2EncPos=FULL_STEPS_PER_ROTATION*MICROSTEP*STEP2ENCODER/_mmperRot;

	return ret;
}

long ThorZStepperSimulator::SelectDevice(const long device)
{
	long ret = FALSE;

	if(FALSE == _deviceDetected)
	{
		return FALSE;
	}

	if(device < 0 || device > _deviceDetected - 1)
		return FALSE;
	/*switch(device)
	{
	case 0:		{ ret = TRUE; }break;
	default:	{ }
	}*/

	ret = TRUE; 

	return ret;
}

long ThorZStepperSimulator::TeardownDevice()
{
	return TRUE;
}

long ThorZStepperSimulator::GetParamInfo
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

	case PARAM_Z_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;	
		paramReadOnly = TRUE;
		paramMin = Z_MIN;
		paramMax = Z_MAX;
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

	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = STAGE_Z;
		}
		break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long ThorZStepperSimulator::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_DEVICE_TYPE:
		{
			DeviceType type = static_cast<DeviceType>(static_cast<int>(param));
			if((type >= IDevice::STAGE_X) && (type <= IDevice::STAGE_Z))
			{
				_zDevType = type;			
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
				_zPos = static_cast<double>(param);
			
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
				_zZero = TRUE;
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
				_mm2EncPos=FULL_STEPS_PER_ROTATION*MICROSTEP*STEP2ENCODER/_mmperRot;
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

long ThorZStepperSimulator::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(static_cast<int>(_zDevType));			
		}
		break;
	case PARAM_Z_POS:	
		{ 
			param = static_cast<double>(_zPos); 
		}
		break;
	case PARAM_Z_POS_CURRENT:
		{
			//int encPos = GetEncPos();
			//param = static_cast<double>((encPos/(double)_zStepsPerMM) - _zZero);
			param =  static_cast<double>(_zPos_C);			 
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
	case PARAM_CONNECTION_STATUS:
		{
			param = static_cast<double>(CONNECTION_READY);
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
long ThorZStepperSimulator::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorZStepperSimulator::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorZStepperSimulator::SetParamString(const long paramID, wchar_t* str)
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
long ThorZStepperSimulator::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorZStepperSimulator::PreflightPosition()
{
	return TRUE;
}

long ThorZStepperSimulator::SetupPosition()
{
	return TRUE;
}

long ThorZStepperSimulator::StartPosition()
{
	long	ret = FALSE;

	if(_zZero_B)
	{
		_zZero = _zPos;
		_zZero_B = FALSE;
		_zPos = 0;
	}

	double zVal = _zPos;

	if(zVal < Z_MIN)
	{
		zVal = Z_MIN;
	}
	if(zVal > Z_MAX)
	{
		zVal = Z_MAX;
	}


	//convert to encoder units
	//actLocation = static_cast<int>((zVal + _zZero)* _zStepsPerMM);

	//MoveEncPos(actLocation, FALSE);

	Sleep(10);
	_zPos_C = zVal;
	
	ret = TRUE;

	return ret;
}

long ThorZStepperSimulator::StatusPosition(long &status)
{
	long	ret = TRUE;

	/*if(TargetReached()) 
	{
		status = IDevice::STATUS_READY;
	}
	else
	{
		Sleep(10);
		status = IDevice::STATUS_BUSY;
	}*/

	status = IDevice::STATUS_READY;
	return ret;
}

long ThorZStepperSimulator::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	/*if(deviceType & STAGE_Z)
	{
		int encPos = GetEncPos();
		pos = static_cast<double>((encPos/(double)_zStepsPerMM) - _zZero);
		ret = TRUE;
	}*/

	pos = _zPos_C;
	ret = TRUE;
	return ret;
}

long ThorZStepperSimulator::PostflightPosition()
{
	return TRUE;
}

long ThorZStepperSimulator::MoveEncPos(int encPos, bool rel)
{
	//int startEncPos = GetEncPos();
	//int endEncPos;
	//if(rel)
	//{
	//	endEncPos = startEncPos + encPos;
	//}
	//else
	//{
	//	endEncPos = encPos;
	//}

	//int ustep;

	//int curEncPos = GetEncPos();

	//ustep = (int) ((double) (endEncPos - curEncPos) / STEP2ENCODER);

	//Move(ustep, true);


	//while((curEncPos != endEncPos) && (abs(ustep) >= 1))
	//{

	//	while(!TargetReached()) 
	//	{
	//		Sleep(10);
	//	};
	//	curEncPos = GetEncPos();
	//	ustep = (int) ((double) (endEncPos - curEncPos) / STEP2ENCODER);
	//}

	return 1;
}

long ThorZStepperSimulator::GoToStepPos(int stepPos, bool relative)
{
	//int startStepPos = GetActPos();
	//int endStepPos;
	//if(relative)
	//	endStepPos = startStepPos + stepPos;
	//else
	//	endStepPos = stepPos;
	//Move(endStepPos, false);
	//while(!TargetReached())
	//{
	//		Sleep(10);
	//};

	return TRUE;
}

long ThorZStepperSimulator::SetMaxPositionSpeed(int maxSpeed)
{
	//SetAxisParameter(4, maxSpeed);
	return TRUE;
}

long ThorZStepperSimulator::SetMaxAcceleration(int maxAccel)
{
	//SetAxisParameter(5, maxAccel);

	// acceleration=0~2047;
	// actual acceleratin= 16MHz/65535*2^PD (usteps/sec) PD=pulse divisor
	return TRUE;
}

long ThorZStepperSimulator::SetMaxCurrent(int maxCur)
{
	//SetAxisParameter(6, maxCur);
	return TRUE;
}

long ThorZStepperSimulator::SetStandbyCurrent(int currentFactor)
{
	//SetAxisParameter(7, currentFactor);
	return TRUE;

	// 0 ~255 for TMCM 140 0~ 1500 (1500mA) for TMCM 110
}

long ThorZStepperSimulator::SetRFSSpeed(int rfsSpeedIndex)
{
	SetAxisParameter(194, rfsSpeedIndex);
	return TRUE;
}

long ThorZStepperSimulator::SetRFSSwitchSpeed(int rfsSwitchIndex)
{
	//SetAxisParameter(195, rfsSwitchIndex);
	return TRUE;
}

double ThorZStepperSimulator::MovemmDistancemm(double mmDistance)
{
	/*int encPos = (int) (mmDistance * _mm2EncPos);
	return((double) MoveEncPos(encPos, true)) / _mm2EncPos;*/

	return 1;
}

//
// =======================================================================================================================
//    < Move to mm position < This function can be called from back ground worker thread
// =======================================================================================================================
//
double ThorZStepperSimulator::MovetommPos(double mmPosition)
{
	/*int encPos = (int) (mmPosition * _mm2EncPos);
	return((double) MoveEncPos(encPos, false)) / _mm2EncPos;*/

	return 1;
}

long ThorZStepperSimulator::MotorVelocCalc(double mmpers)
{
	/*int		pulsediv = GetPulseDiv();
	double	microstep = (double) GetMicroStep();
	double	fstepfrq = mmpers / _mmperRot * FULL_STEPS_PER_ROTATION;
	double	pulsefrq = fstepfrq * microstep;
	double	velocity = pulsefrq * 2048 * 32 * pow(2.0, pulsediv) / CLK_FRQ;
	return (int) velocity;*/

	return 1;
}

long ThorZStepperSimulator::Set_Pos_Speed_Accel()
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

	if(motVelocity == 2048) motVelocity = 2047;

	//SetMaxAcceleration(motAccel);
	//SetMaxPositionSpeed(motVelocity);

	return motVelocity;
}

long ThorZStepperSimulator::Rotate(int speed, bool right)
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

	//Echo();

	return TRUE;
}

long ThorZStepperSimulator::Move(int steps, bool rel)
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
	//Echo();
	return TRUE;
}

long ThorZStepperSimulator::TargetReached()
{
	/*if(GetAxisParameter(8) == 1)
		return true;
	else
		return false;*/

	return true;
}

long ThorZStepperSimulator::IsStop()
{
	/*if(GetAxisParameter(3) == 0)
		return true;
	else
		return false;*/

	return true;
}

long ThorZStepperSimulator::Stop()
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
	//Echo();
	return TRUE;
}

//
// =======================================================================================================================
//    Get the actual position by driving pulses
// =======================================================================================================================
//
long ThorZStepperSimulator::GetActPos()
{
	//return GetAxisParameter(1);

	return 1;
}

//
// =======================================================================================================================
//    Get the encorder position, prescaled and counted
// =======================================================================================================================
//
long ThorZStepperSimulator::GetEncPos()
{
	//return GetAxisParameter(209);
	
	return 1;
}

//
// =======================================================================================================================
//    Set the encorder position
// =======================================================================================================================
//
long ThorZStepperSimulator::SetEncorderPos(int posValue)
{
	//SetAxisParameter(209, posValue);
	return TRUE;
}

//
// =======================================================================================================================
//    Get the Absolute encorder position,
// =======================================================================================================================
//
long ThorZStepperSimulator::GetAbsEncPos()
{
	//return GetAxisParameter(215);
	
	return 1;
}

long ThorZStepperSimulator::GetPulseDiv()
{
	//return GetAxisParameter(154);

	return 1;
}

long ThorZStepperSimulator::SetMicroStep(int microStep)
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

	//SetAxisParameter(140, instrValue);
	return TRUE;
}

long ThorZStepperSimulator::SetMicroStepIndex(int microStepIndex)
{
	//SetAxisParameter(140, microStepIndex);
	return TRUE;
}

long ThorZStepperSimulator::GetMicroStep()
{

	// return the microsteps
	//int paraValue = GetAxisParameter(140);
	/*switch(paraValue)
	{
	case 6: return 64; break;
	case 5: return 32; break;
	case 4: return 16; break;
	case 3: return 8; break;
	case 2: return 4; break;
	case 1: return 2; break;
	case 0: return 1; break;
	}*/

	return 0;
}

long ThorZStepperSimulator::SetPrescaler(double scaleFactor)
{
	int instrValue;
	instrValue = (int) (scaleFactor * 2048);
	//SetAxisParameter(210, instrValue);
	return TRUE;
}

double ThorZStepperSimulator::GetPrescaler()
{
	/*int paraValue = GetAxisParameter(210);
	return (double) paraValue / 2048;*/
	
	return 1;
}

long ThorZStepperSimulator::GetStandbyCurrent()
{
	//return GetAxisParameter(7);

	return 1;
}

long ThorZStepperSimulator::GetMaxAccel()
{
	//return GetAxisParameter(5);

	return 1;
}

//
// =======================================================================================================================
//    <summary> Value for type: 0: Start, 1: Stop, 2: Status </summary> Search for the reference point
// =======================================================================================================================
//
long ThorZStepperSimulator::RefSearch(unsigned char type)
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
	//Echo();
	return TRUE;
}

long ThorZStepperSimulator::RefSearchStart()
{
	//RefSearch(0);
	return TRUE;
}

long ThorZStepperSimulator::RefSearchStop()
{
	//RefSearch(1);
	return TRUE;
}

long ThorZStepperSimulator::RefSearchActive()
{
	/*RefSearch(2);

	int value = GetBufferValue();
	if(value == 0)
		return false;
	else*/
		return true;
}

long ThorZStepperSimulator::GetAxisParameter(int parameter)
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
	//Echo();
	//return GetBufferValue();

	return 1;
}

long ThorZStepperSimulator::SetAxisParameter(int parameter, int value)
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
	//Echo();
	return TRUE;
}

long ThorZStepperSimulator::GetBufferValue()
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

long ThorZStepperSimulator::Echo()
{	
	//while(_serialPort.ReadDataWaiting() < 9) 
	//{
	//	Sleep(10);
	//}

	//_serialPort.ReadData(_readBuffer, 9);
	return TRUE;
}

long ThorZStepperSimulator::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}
