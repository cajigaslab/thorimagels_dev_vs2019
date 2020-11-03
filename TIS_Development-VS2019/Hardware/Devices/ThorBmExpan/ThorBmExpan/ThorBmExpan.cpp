// ThorBmExpan.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "math.h"
#include "Serial.h"
#include "ThorBmExpan.h"
//#include "ThorBmExpanSetupXML.h"

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

wchar_t message[256];

#define CLK_FRQ 16000000.0
#define FULL_STEPS_PER_ROTATION 200.0
#define MICROSTEP 16
#define MICROSTEPINDEX 4
#define STALLGUARD 3
#define HOMESPEED 600
#define HOMEACCELERATION 250
#define POSITIONSPEED 300
#define POSITIONACCELERATION 100
#define MAXCURRENT 800
#define ADDRESSDEFAULT 1


ThorBmExpan::ThorBmExpan()
{
	_deviceDetected = FALSE;
	_expIndex=EXP_DEFAULT;
	_posMode=POS_MODE_DEFAULT;

	_targetReached[0]=true;
	_targetReached[1]=true;
	_address[0]=ADDRESSDEFAULT;
	_address[1]=ADDRESSDEFAULT;
	_expIndex_C = -1;
	_expIndex_B = FALSE;
	_dataBuffer[0] = NULL;
	_readBuffer[0] = NULL;
	_errMsg[0] = NULL;
	_expRatio[0] = EXP_MIN;
	_mot0_Pos = POS_MIN;
	_mot0_PosArray[0] = POS_MIN;
	_mot1_Pos = POS_MIN;
	_mot1_PosArray[0] = POS_MIN;
}

ThorBmExpan::~ThorBmExpan()
{
	_instanceFlag = false;
	_errMsg[0] = 0;
}

bool ThorBmExpan:: _instanceFlag = false;

auto_ptr<ThorBmExpan> ThorBmExpan::_single(new ThorBmExpan());

ThorBmExpan *ThorBmExpan::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorBmExpan());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorBmExpan::FindDevices(long &deviceCount)
{
	long	ret = TRUE;

	//Get motor parameters from hardware setup.xml
	auto_ptr<ThorBmExpanXML> pSetup(new ThorBmExpanXML());

	long portID0=0;
	long baudRate0=0;
	long address0=0;
	long portID1=0;
	long baudRate1=0;
	long address1=0;


	pSetup->OpenConfigFile();
	pSetup->GetConnection(portID0,portID1, baudRate0,baudRate1, address0, address1);
	pSetup->GetMOT0_Pos(_mot0_PosArray[0],
		_mot0_PosArray[1],
		_mot0_PosArray[2],
		_mot0_PosArray[3],
		_mot0_PosArray[4],
		_mot0_PosArray[5],
		_mot0_PosArray[6]);
	pSetup->GetMOT1_Pos(_mot1_PosArray[0],
		_mot1_PosArray[1],
		_mot1_PosArray[2],
		_mot1_PosArray[3],
		_mot1_PosArray[4],
		_mot1_PosArray[5],
		_mot1_PosArray[6]);
	pSetup->GetExpRatios(_expRatio[0],
		_expRatio[1],
		_expRatio[2],
		_expRatio[3],
		_expRatio[4],
		_expRatio[5],
		_expRatio[6]);
	if ((FALSE == _serialPort[0].Open(portID0, baudRate0))||(FALSE == _serialPort[1].Open(portID1, baudRate1)))
	{
		wsprintf(message,L"ThorBmExpan FindDevices could not open serial port");
		LogMessage(message);
		wsprintf(_errMsg,L"ThorBmExpan FindDevices could not open serial port or configuration file is not avaible.");

		// *TODO* perform an automatic scan of the available serial ports for the Z stepper
		deviceCount = 0;
		ret = FALSE;
	}
	else
	{
		deviceCount = 1;
		_deviceDetected = TRUE;

		_address[0]=(unsigned char) address0;
		_address[1]=(unsigned char) address1;

		SetMicroStep(0,MICROSTEP);
		SetMicroStep(1,MICROSTEP);

		SetStandbyCurrent(0, 0);
		SetStandbyCurrent(1, 0);

		SetMaxCurrent(0, MAXCURRENT);
		SetMaxCurrent(1, MAXCURRENT);

		SetPowerDownDelay(0, 10); //10=> 100 ms
		SetPowerDownDelay(1, 10);
		SetFreeWheelingDelay(0, 100); // 100=> 100 ms
		SetFreeWheelingDelay(1, 100);
	}

	return ret;
}

long ThorBmExpan::SelectDevice(const long device)
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

			//home the motor when the device is started
			MotorHome(0);
			MotorHome(1);
		}
		break;
	default:
		{
			wsprintf(_errMsg,L"The device index %d is invalid",device);
		}
	}

	return ret;
}
void ThorBmExpan::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

long ThorBmExpan::TeardownDevice()
{
	_serialPort[0].Close();
	_serialPort[1].Close();
	return TRUE;
}

long ThorBmExpan::GetParamInfo
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

	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = (double)ConnectionStatusType::CONNECTION_READY;
			paramMax = paramDefault = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
		break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long ThorBmExpan::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;
	long foundParam = TRUE;

	switch(paramID)
	{
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

long ThorBmExpan::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{
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
	case PARAM_DEVICE_TYPE:
		{
			param =static_cast<double>(BEAM_EXPANDER);
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = (_deviceDetected) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
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
long ThorBmExpan::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorBmExpan::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorBmExpan::SetParamString(const long paramID, wchar_t* str)
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
long ThorBmExpan::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorBmExpan::PreflightPosition()
{
	return TRUE;
}

long ThorBmExpan::SetupPosition()
{
	if(_expIndex != _expIndex_C)
	{
		_expIndex_B = TRUE;
	}
	return TRUE;
}

long ThorBmExpan::StartPosition()
{
	long	ret = FALSE;

	switch (_posMode)
	{
	case MANUAL_MODE:
		_targetReached[0]=false;
		_targetReached[1]=false;
		if (_mot0_Pos==0)
		{
			MotorHome(0);
		}
		else
		{
			SetStallGuard(0, 0); //disable stall guard
			SetMaxPositionSpeed(0, POSITIONSPEED); 
			SetMaxAcceleration(0, POSITIONACCELERATION);
			GoToStepPos(0, _mot0_Pos, false);

		}
		if (_mot1_Pos==0)
		{
			MotorHome(1);
		}
		else
		{
			SetStallGuard(1, 0); //disable stall guard
			SetMaxPositionSpeed(1, POSITIONSPEED);
			SetMaxAcceleration(1, POSITIONACCELERATION);
			GoToStepPos(1, _mot1_Pos, false);
		}
		break;
	case AUTO_MODE:

		if(_expIndex_B)
		{
			_targetReached[0]=false;
			_targetReached[1]=false;
			SetStallGuard(0, 0); //disable stall guard
			SetStallGuard(1, 0); //disable stall guard
			SetMaxPositionSpeed(0, POSITIONSPEED); 
			SetMaxPositionSpeed(1, POSITIONSPEED);
			SetMaxAcceleration(0, POSITIONACCELERATION);
			SetMaxAcceleration(1, POSITIONACCELERATION);
			GoToStepPos(0, _mot0_PosArray[_expIndex], false);
			GoToStepPos(1, _mot1_PosArray[_expIndex], false);
			_expIndex_C = _expIndex;
			_expIndex_B = FALSE;
		}
		break;
	}
	ret = TRUE;
	return ret;
}

long ThorBmExpan::StatusPosition(long &status)
{
	long	ret = TRUE;

	if(_targetReached[0]&&_targetReached[1]) 
	{
		status = IDevice::STATUS_READY;
	}
	else
	{
		Sleep(10);
		status = IDevice::STATUS_BUSY;
	}
	return ret;
}

long ThorBmExpan::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	if(deviceType & BEAM_EXPANDER)
	{
		pos = static_cast<double>(GetActPos(0));
		ret = TRUE;
	}
	else
	{
		pos = static_cast<double>(GetActPos(1));
		ret = TRUE;
	}

	return ret;
}

long ThorBmExpan::PostflightPosition()
{
	return TRUE;
}


long ThorBmExpan::GoToStepPos(int motID, int stepPos, bool relative)
{
	_targetReached[motID]=false;
	int startStepPos = GetActPos(motID);
	int endStepPos;
	if(relative)
		endStepPos = startStepPos + stepPos;
	else
		endStepPos = stepPos;
	Move(motID, endStepPos, false);

	while(!TargetReached(motID))
	{
		Sleep(10);
	};
	_targetReached[motID]=true;
	return TRUE;
}

long ThorBmExpan::MotorHome(int motID)
{
	_targetReached[motID]=false;
	DisableMixedDecay(motID);
	SetMaxAcceleration(motID, HOMEACCELERATION);
	//	SetStallGuard(motID, STALLGUARD);
	Rotate(motID,HOMESPEED, true);
	//while (GetActSpeed(motID)!=0)
	//{
	//	Sleep(100);
	//};

	Sleep(2000);

	Stop(motID);

	Sleep(1000);

	SetActPos(motID, 0);

	_targetReached[motID]=true;
	return TRUE;
}

long ThorBmExpan::DisableMixedDecay(int motID)
{
	SetAxisParameter(motID, 203, 2048);
	return TRUE;
}

long ThorBmExpan::SetMaxPositionSpeed(int motID, int maxSpeed)
{
	SetAxisParameter(motID, 4, maxSpeed);
	return TRUE;
}

long ThorBmExpan::SetMaxAcceleration(int motID, int maxAccel)
{
	SetAxisParameter(motID, 5, maxAccel);

	// acceleration=0~2047;
	// actual acceleratin= 16MHz/65535*2^PD (usteps/sec) PD=pulse divisor
	return TRUE;
}

long ThorBmExpan::SetMaxCurrent(int motID, int maxCur)
{
	SetAxisParameter(motID, 6, maxCur);
	return TRUE;
}

long ThorBmExpan::SetStandbyCurrent(int motID, int currentFactor)
{
	SetAxisParameter(motID, 7, currentFactor);
	return TRUE;

	// 0 ~255 for TMCM 140 0~ 1500 (1500mA) for TMCM 110
}

long ThorBmExpan::SetRFSSpeed(int motID, int rfsSpeedIndex)
{
	SetAxisParameter(motID, 194, rfsSpeedIndex);
	return TRUE;
}

long ThorBmExpan::SetRFSSwitchSpeed(int motID, int rfsSwitchIndex)
{
	SetAxisParameter(motID, 195, rfsSwitchIndex);
	return TRUE;
}



long ThorBmExpan::Rotate(int motID, int speed, bool right)
{
	_dataBuffer[0] = _address[motID];		// address
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

	_serialPort[motID].SendData(_dataBuffer, 9);
	Echo(motID);
	return TRUE;
}

long ThorBmExpan::Move(int motID, int steps, bool rel)
{
	_dataBuffer[0] = _address[motID];	// address
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

	_serialPort[motID].SendData(_dataBuffer, 9);
	Echo(motID);
	return TRUE;
}

long ThorBmExpan::TargetReached(int motID)
{
	if(GetAxisParameter(motID, 8) == 1)
		return true;
	else
		return false;
}

long ThorBmExpan::IsStop(int motID)
{
	if(GetAxisParameter(motID, 3) == 0)
		return true;
	else
		return false;
}

long ThorBmExpan::Stop(int motID)
{
	_dataBuffer[0] = _address[motID];	// address
	_dataBuffer[1] = 3;	// Instruction: Stop
	_dataBuffer[2] = 0;	// Type: don't care
	_dataBuffer[3] = 0;
	_dataBuffer[4] = 0;
	_dataBuffer[5] = 0;
	_dataBuffer[6] = 0;
	_dataBuffer[7] = 0;
	_dataBuffer[8] = 4;
	_serialPort[motID].SendData(_dataBuffer, 9);
	Echo(motID);
	return TRUE;
}

//
// =======================================================================================================================
//    Get the actual position by driving pulses
// =======================================================================================================================
//
long ThorBmExpan::GetActPos(int motID)
{
	return GetAxisParameter(motID, 1);
}

//
// =======================================================================================================================
//    Get the encorder position, prescaled and counted
// =======================================================================================================================
//
long ThorBmExpan::GetEncPos(int motID)
{
	return GetAxisParameter(motID, 209);
}

long ThorBmExpan::GetActSpeed(int motID)
{
	return GetAxisParameter(motID, 3);
}

//
// =======================================================================================================================
//    Set the encorder position
// =======================================================================================================================
//

long ThorBmExpan::SetActPos(int motID, int posValue)
{
	SetAxisParameter(motID, 1, posValue);
	return TRUE;
}

//Standstill period before the current is changed down to standby current.
long ThorBmExpan::SetPowerDownDelay(int motID, int delay)
{
	SetAxisParameter(motID, 214, delay);   // 1 (10ms) up to 65535 (655350 ms)
	//default 200 (2000 ms)
	return TRUE;
}

//Time after which the power to the motor will be cut when its velocity has reached zero.
long ThorBmExpan::SetFreeWheelingDelay(int motID, int delay)
{
	SetAxisParameter(motID, 204, delay); //0 up to 65535
	//0 => never
	//100 => 100 ms
	return TRUE;
}

long ThorBmExpan::SetStallGuard(int motID, int stallGuard)
{
	SetAxisParameter(motID, 205, stallGuard);
	return TRUE;
}
//
// =======================================================================================================================
//    Get the Absolute encorder position,
// =======================================================================================================================
//
long ThorBmExpan::GetAbsEncPos(int motID)
{
	return GetAxisParameter(motID, 215);
}

long ThorBmExpan::GetPulseDiv(int motID)
{
	return GetAxisParameter(motID, 154);
}

long ThorBmExpan::SetMicroStep(int motID, int microStep)
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

	SetAxisParameter(motID, 140, instrValue);
	return TRUE;
}

long ThorBmExpan::SetMicroStepIndex(int motID, int microStepIndex)
{
	SetAxisParameter(motID, 140, microStepIndex);
	return TRUE;
}

long ThorBmExpan::GetMicroStep(int motID)
{

	// return the microsteps
	int paraValue = GetAxisParameter(motID, 140);
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

long ThorBmExpan::SetPrescaler(int motID, double scaleFactor)
{
	int instrValue;
	instrValue = (int) (scaleFactor * 2048);
	SetAxisParameter(motID, 210, instrValue);
	return TRUE;
}


double ThorBmExpan::GetPrescaler(int motID)
{
	int paraValue = GetAxisParameter(motID, 210);
	return (double) paraValue / 2048;
}

long ThorBmExpan::GetStandbyCurrent(int motID)
{
	return GetAxisParameter(motID, 7);
}

long ThorBmExpan::GetMaxAccel(int motID)
{
	return GetAxisParameter(motID, 5);
}



//
// =======================================================================================================================
//    <summary> Value for type: 0: Start, 1: Stop, 2: Status </summary> Search for the reference point
// =======================================================================================================================
//
long ThorBmExpan::RefSearch(int motID, unsigned char type)
{
	_dataBuffer[0] = _address[motID];		// address
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

	_serialPort[motID].SendData(_dataBuffer, 9);
	Echo(motID);
	return TRUE;
}

long ThorBmExpan::RefSearchStart(int motID)
{
	RefSearch(motID, 0);
	return TRUE;
}

long ThorBmExpan::RefSearchStop(int motID)
{
	RefSearch(motID, 1);
	return TRUE;
}

long ThorBmExpan::RefSearchActive(int motID)
{
	RefSearch(motID, 2);

	int value = GetBufferValue();
	if(value == 0)
		return false;
	else
		return true;
}

long ThorBmExpan::GetAxisParameter(int motID, int parameter)
{
	_dataBuffer[0] = _address[motID];		// address
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

	_serialPort[motID].SendData(_dataBuffer, 9);
	Echo(motID);
	return GetBufferValue();
}

long ThorBmExpan::SetAxisParameter(int motID, int parameter, int value)
{
	_dataBuffer[0] = _address[motID];	// address
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

	_serialPort[motID].SendData(_dataBuffer, 9);
	Echo(motID);
	return TRUE;
}

long ThorBmExpan::GetBufferValue()
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

long ThorBmExpan::Echo(int motID)
{	
	while(_serialPort[motID].ReadDataWaiting() < 9) 
	{
		Sleep(10);
	}

	_serialPort[motID].ReadData(_readBuffer, 9);
	return TRUE;
}


long ThorBmExpan::GetLastErrorMsg(wchar_t * msg, long size)
{	
	wcsncpy_s(msg,size,_errMsg,256);
	return TRUE;
}
