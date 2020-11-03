#pragma once


class DisconnectedDevice : IDevice
{
private:
    DisconnectedDevice();
public:

	enum
	{
		X_MIN = -100,
		X_MAX = 100,
		X_DEFAULT = 10,
		
		Y_MIN = -100,
		Y_MAX = 100,
		Y_DEFAULT = 10,
		
		Z_MIN = -100,
		Z_MAX = 100,
		Z_DEFAULT = 10,

		LOAD_MIN = 0,
		LOAD_MAX = 0,
		LOAD_DEFAULT = 0,

		AUTOFOCUS_MIN = 0,
		AUTOFOCUS_MAX = 1,
		AUTOFOCUS_DEFAULT = 0,

		SHUTTER_CLOSE=0,
		SHUTTER_OPEN=1,
		SHUTTER_DEFAULT=SHUTTER_CLOSE,

		SHUTTER_WAIT_TIME_MIN = 0,
		SHUTTER_WAIT_TIME_MAX = 1000,		
		SHUTTER_WAIT_TIME_DEFAULT = 0

	};

	static DisconnectedDevice* getInstance();
    ~DisconnectedDevice();

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
	long GetLastErrorMsg(wchar_t *msg,long size);	

private:

	double xPos;
	double xPos_C;
	BOOL xPos_B;
	
	double yPos;
	double yPos_C;
	BOOL yPos_B;
	
	double zPos;
	double zPos_C;
	BOOL zPos_B;
	
	long load;
	long load_C;
	BOOL load_B;
	
	long afPos;
	long afPos_C;
	BOOL afPos_B;
	
	long shutterPos;
	long shutterPos_C;
	BOOL shutterPos_B;
	
	long shutterWaitTime;
	long shutterWaitTime_C;
	BOOL shutterWaitTime_B;

    static bool instanceFlag;
    static auto_ptr<DisconnectedDevice> _single;
};