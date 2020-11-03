#pragma once

class ThorBmExpanSimulator : IDevice
{
private:
    ThorBmExpanSimulator();
public:



	enum
	{
		MANUAL_MODE=0,
		AUTO_MODE=1,
	};

	enum
	{

		EXP_MIN =0,
		EXP_MAX =5,
		EXP_DEFAULT = 1,
		
		POS_MIN =-13100,
		POS_MAX =0,
		POS_DEFAULT =-500,

		POS_MODE_MIN=MANUAL_MODE,
		POS_MODE_MAX=AUTO_MODE,
		POS_MODE_DEFAULT=AUTO_MODE,

		//Z_VELOCITY_MIN = 8,
		//Z_VELOCITY_MAX = 2047,
		//Z_VELOCITY_DEFAULT = 256,
	};



	static ThorBmExpanSimulator* getInstance();
    ~ThorBmExpanSimulator();

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
	
void LogMessage(wchar_t *message);

private:
	wchar_t _errMsg[256];

	long _expIndex;
	long _posMode;
	long _mot0_PosArray[6];
	long _mot1_PosArray[6];
	long _mot0_Pos;
	long _mot1_Pos;

	DeviceType _bmxpDevType;

    static bool _instanceFlag;
    static auto_ptr<ThorBmExpanSimulator> _single;
	BOOL _deviceDetected;
};