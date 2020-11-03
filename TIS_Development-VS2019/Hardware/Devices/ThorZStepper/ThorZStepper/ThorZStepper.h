#pragma once

#include "Serial.h"

class ThorZStepper : IDevice
{
private:
    ThorZStepper();
public:

	enum
	{
		Z_DEFAULT = 0,
		
		Z_STEPS_PER_MM_MIN = 1,
		Z_STEPS_PER_MM_MAX = 1000000,
		Z_STEPS_PER_MM_DEFAULT = 32000,

		Z_VELOCITY_MIN = 8,
		Z_VELOCITY_MAX = 2047,
		Z_VELOCITY_DEFAULT = 256,
	};

	static ThorZStepper* getInstance();
    ~ThorZStepper();

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
	long GetLastErrorMsg(wchar_t * errMsg, long size);

private:
	
long MoveEncPos(int encPos, bool rel);
long GoToStepPos(int stepPos, bool relative);
long SetMaxPositionSpeed(int maxSpeed);
long SetMaxAcceleration(int maxAccel);
long SetMaxCurrent(int maxCur);
long SetStandbyCurrent(int currentFactor);
long SetRFSSpeed(int rfsSpeedIndex);
long SetRFSSwitchSpeed(int rfsSwitchIndex);
double MovemmDistancemm(double mmDistance);
double MovetommPos(double mmPosition);
long MotorVelocCalc(double mmpers);
long Set_Pos_Speed_Accel();
long Rotate(int speed, bool right);
long Move(int steps, bool rel);
long TargetReached();
long IsStop();
long Stop();
long GetActPos();
long GetEncPos();
long SetEncorderPos(int posValue);
long GetAbsEncPos();
long GetPulseDiv();
long SetMicroStep(int microStep);
long GetMicroStep();
long SetPrescaler(double scaleFactor);
double GetPrescaler();
long GetStandbyCurrent();
long GetMaxAccel();
long RefSearch(unsigned char type);
long RefSearchStart();
long RefSearchStop();
long RefSearchActive();
long GetAxisParameter(int parameter);
long SetAxisParameter(int parameter, int value);
long SetStoredAxisParameter(int parameter, int value);
long SetPositionAsZero();
long GetBufferValue();
long Echo();
long VerifyResponse(int commandNumber);
long GetFirmwareVersion(int& firmRevision);

void LogMessage(wchar_t *message);
long GetConnectionStatus();
private:
	wchar_t _errMsg[256];

	double _zPos;
	double _zPos_C;
	BOOL _zPos_B;
	
    static bool _instanceFlag;
    static auto_ptr<ThorZStepper> _single;

	CSerial _serialPort;
	
	CritSect _critSect;

	double _mmperRot;
	double _mm2EncPos;
	long _zStepsPerMM;

	double _zZero;
	BOOL _zZero_B;

	long _zVelocity;
	BOOL _zVelocity_B;
	
	long _zEnableHoldingVoltage;
	BOOL _zEnableHoldingVoltage_B;
	
	long _zStop;
	BOOL _zStop_B;

	double _zMin;
	double _zMax;
	long _zInvert;

	double _threshold;
	double _encoderPrescaler;
	//physical encoder has 1024 position per rotation,
	//scale it up to 3200 positions to match 16 microsteps, because 16x200=3200
	long _microStep;
	double _step2Encoder;

	BOOL _deviceDetected;
	unsigned char _dataBuffer[9];
	unsigned char _readBuffer[9];
	unsigned char _address;
};