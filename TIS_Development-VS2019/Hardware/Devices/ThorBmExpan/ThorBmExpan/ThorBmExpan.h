#pragma once

#include "Serial.h"
#include "ThorBmExpanSetupXML.h"

class ThorBmExpan : IDevice
{
private:
    ThorBmExpan();
public:



	enum
	{
		MANUAL_MODE=0,
		AUTO_MODE=1,
	};

	enum
	{

		EXP_MIN =0,
		EXP_MAX =NUM_EXP_RATIO - 1,
		EXP_DEFAULT = 1,
		
		POS_MIN =-40000,
		POS_MAX =0,
		POS_DEFAULT =-500,

		POS_MODE_MIN=MANUAL_MODE,
		POS_MODE_MAX=AUTO_MODE,
		POS_MODE_DEFAULT=AUTO_MODE,

		//Z_VELOCITY_MIN = 8,
		//Z_VELOCITY_MAX = 2047,
		//Z_VELOCITY_DEFAULT = 256,
	};



	static ThorBmExpan* getInstance();
    ~ThorBmExpan();

	long FindDevices(long &DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double &param);
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long &status);
	long ReadPosition(DeviceType deviceType, double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t * errMsg, long size);

private:
	
long ThorBmExpan::GoToStepPos(int motID, int stepPos, bool relative);
long ThorBmExpan::SetMaxPositionSpeed(int motID, int maxSpeed);
long ThorBmExpan::SetMaxAcceleration(int motID, int maxAccel);
long ThorBmExpan::SetMaxCurrent(int motID, int maxCur);
long ThorBmExpan::SetStandbyCurrent(int motID, int currentFactor);
long ThorBmExpan::SetPowerDownDelay(int motID, int delay);
long ThorBmExpan::SetFreeWheelingDelay(int motID, int delay);
long ThorBmExpan::SetStallGuard(int motID, int stallGuard);
long ThorBmExpan::DisableMixedDecay(int motID);
long ThorBmExpan::SetRFSSpeed(int motID, int rfsSpeedIndex);
long ThorBmExpan::SetRFSSwitchSpeed(int motID, int rfsSwitchIndex);
long ThorBmExpan::Rotate(int motID, int speed, bool right);
long ThorBmExpan::Move(int motID, int steps, bool rel);
long ThorBmExpan::TargetReached(int motID);
long ThorBmExpan::IsStop(int motID);
long ThorBmExpan::Stop(int motID);
long ThorBmExpan::GetActSpeed(int motID);
long ThorBmExpan::GetActPos(int motID);
long ThorBmExpan::GetEncPos(int motID);
long ThorBmExpan::SetEncorderPos(int motID, int posValue);
long ThorBmExpan::SetActPos(int motID, int posValue);
long ThorBmExpan::GetAbsEncPos(int motID);
long ThorBmExpan::GetPulseDiv(int motID);
long ThorBmExpan::SetMicroStep(int motID, int microStep);
long ThorBmExpan::SetMicroStepIndex(int motID, int microStepIndex);
long ThorBmExpan::GetMicroStep(int motID);
long ThorBmExpan::SetPrescaler(int motID, double scaleFactor);
double ThorBmExpan::GetPrescaler(int motID);
long ThorBmExpan::GetStandbyCurrent(int motID);
long ThorBmExpan::GetMaxAccel(int motID);
long ThorBmExpan::MotorHome(int motID);
long ThorBmExpan::RefSearch(int motID, unsigned char type);
long ThorBmExpan::RefSearchStart(int motID);
long ThorBmExpan::RefSearchStop(int motID);
long ThorBmExpan::RefSearchActive(int motID);
long ThorBmExpan::GetAxisParameter(int motID, int parameter);
long ThorBmExpan::SetAxisParameter(int motID, int parameter, int value);
long ThorBmExpan::GetBufferValue();
long ThorBmExpan::Echo(int motID);
void LogMessage(wchar_t *message);

private:
	wchar_t _errMsg[256];

	long _expIndex;
	long _expIndex_C;
	BOOL _expIndex_B;
	long _posMode;
	long _expRatio[7];
	long _mot0_PosArray[7];
	long _mot1_PosArray[7];
	long _mot0_Pos;
	long _mot1_Pos;
	
    static bool _instanceFlag;
    static auto_ptr<ThorBmExpan> _single;

	CSerial _serialPort[2];


	bool _targetReached[2];


	BOOL _deviceDetected;
	unsigned char _dataBuffer[9];
	unsigned char _readBuffer[9];
	unsigned char _address[2];
};