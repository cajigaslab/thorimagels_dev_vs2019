#pragma once

#include "Serial.h"
#include "stdafx.h"


//singleton device class
class ThorPowerControl : IDevice
{

private:
	ThorPowerControl();
	void LinearizeSine(long direction, double val, double &ret);
	long ValidateResponseFormat(char* response);
	void LogMessage(wchar_t *message);
	long ExecuteCmd(long paramID, const char* cmd, long &readBackValue);
	double _rPos;        ///r location
	double _rPos_C;
	BOOL _rPos_B;

	BOOL _rHome_B;        ///home r stage

	long _paramZeroPos;
	long _blockUpdateParam;
	static CritSect critSec;

	BOOL _paramZero;
	BOOL _paramZero_C;
	BOOL _paramZero_B;	

	static bool _instanceFlag;        ///singleton created flag
	static auto_ptr<ThorPowerControl> _single;        ///pointer to internal Device object

	wchar_t _errMsg[MSG_SIZE];

	CSerial _serialPort;

	BOOL _deviceDetected;
	unsigned char _dataBuffer[9];
	unsigned char _readBuffer[9];
	unsigned char _address;

public:

	///parameter limits
	enum
	{
		POWER_MIN = 0,
		POWER_MAX = 100,
		POWER_DEFAULT = 0,

		POWER_HOME_MIN = 0,
		POWER_HOME_MAX = 0,
		POWER_HOME_DEFAULT = 0,

		POWER_VELOCITY_MIN = 1,
		POWER_VELOCITY_MAX = 10,
		POWER_VELOCITY_DEFAULT = 7,

		POWER_ZERO_POS_MIN = -100000,
		POWER_ZERO_POS_MAX = 100000,
		POWER_ZERO_POS_DEFAULT = 0,
	};

	enum ControlTypes
	{
		POWER_CONTROL,
	};

	static ThorPowerControl* getInstance();
	~ThorPowerControl();

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
	long ReadPosition(DeviceType deviceType,double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);
};
