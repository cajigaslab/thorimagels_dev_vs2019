#pragma once

#include "Serial.h"

class ThorZStepperSimulator : IDevice
{
private:
    ThorZStepperSimulator();
public:

	enum
	{
		Z_MIN = -2,
		Z_MAX = 2,
		Z_DEFAULT = 0,
		
		Z_STEPS_PER_MM_MIN = 1,
		Z_STEPS_PER_MM_MAX = 1000000,
		Z_STEPS_PER_MM_DEFAULT = 32000,
	};

	static ThorZStepperSimulator* getInstance();
    ~ThorZStepperSimulator();

	long FindDevices(long &DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double &param);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long &status);
	long ReadPosition(DeviceType deviceType, double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);

private:
	
long ThorZStepperSimulator::MoveEncPos(int encPos, bool rel);
long ThorZStepperSimulator::GoToStepPos(int stepPos, bool relative);
long ThorZStepperSimulator::SetMaxPositionSpeed(int maxSpeed);
long ThorZStepperSimulator::SetMaxAcceleration(int maxAccel);
long ThorZStepperSimulator::SetMaxCurrent(int maxCur);
long ThorZStepperSimulator::SetStandbyCurrent(int currentFactor);
long ThorZStepperSimulator::SetRFSSpeed(int rfsSpeedIndex);
long ThorZStepperSimulator::SetRFSSwitchSpeed(int rfsSwitchIndex);
double ThorZStepperSimulator::MovemmDistancemm(double mmDistance);
double ThorZStepperSimulator::MovetommPos(double mmPosition);
long ThorZStepperSimulator::MotorVelocCalc(double mmpers);
long ThorZStepperSimulator::Set_Pos_Speed_Accel();
long ThorZStepperSimulator::Rotate(int speed, bool right);
long ThorZStepperSimulator::Move(int steps, bool rel);
long ThorZStepperSimulator::TargetReached();
long ThorZStepperSimulator::IsStop();
long ThorZStepperSimulator::Stop();
long ThorZStepperSimulator::GetActPos();
long ThorZStepperSimulator::GetEncPos();
long ThorZStepperSimulator::SetEncorderPos(int posValue);
long ThorZStepperSimulator::GetAbsEncPos();
long ThorZStepperSimulator::GetPulseDiv();
long ThorZStepperSimulator::SetMicroStep(int microStep);
long ThorZStepperSimulator::SetMicroStepIndex(int microStepIndex);
long ThorZStepperSimulator::GetMicroStep();
long ThorZStepperSimulator::SetPrescaler(double scaleFactor);
double ThorZStepperSimulator::GetPrescaler();
long ThorZStepperSimulator::GetStandbyCurrent();
long ThorZStepperSimulator::GetMaxAccel();
long ThorZStepperSimulator::RefSearch(unsigned char type);
long ThorZStepperSimulator::RefSearchStart();
long ThorZStepperSimulator::RefSearchStop();
long ThorZStepperSimulator::RefSearchActive();
long ThorZStepperSimulator::GetAxisParameter(int parameter);
long ThorZStepperSimulator::SetAxisParameter(int parameter, int value);
long ThorZStepperSimulator::GetBufferValue();
long ThorZStepperSimulator::Echo();

private:

	double _zPos;
	double _zPos_C;
	BOOL _zPos_B;
	long _zDevType;
	
    static bool _instanceFlag;
    static auto_ptr<ThorZStepperSimulator> _single;

	CSerial _serialPort;

	double _mmperRot;
	double _mm2EncPos;
	double _zZero;
	BOOL _zZero_B;
	long _zStepsPerMM;

	BOOL _deviceDetected;
	unsigned char _dataBuffer[9];
	unsigned char _readBuffer[9];
	unsigned char _address;
};