#pragma once


class SimDeviceFW : IDevice
{
private:
    SimDeviceFW();
public:

	enum
	{
		X_MIN = -100,
		X_MAX = 100,
		X_DEFAULT = 10,
		
		Y_MIN = -100,
		Y_MAX = 100,
		Y_DEFAULT = 10,

		SHUTTER_CLOSE=0,
		SHUTTER_OPEN=1,
		SHUTTER_DEFAULT=SHUTTER_CLOSE,

		SHUTTER_WAIT_TIME_MIN = 0,
		SHUTTER_WAIT_TIME_MAX = 1000,		
		SHUTTER_WAIT_TIME_DEFAULT = 0,

		FILTER_WHEEL_EX_MIN = 1,
		FILTER_WHEEL_EX_MAX = 8,
		FILTER_WHEEL_EX_DEFAULT = 1,
		
		FILTER_WHEEL_EM_MIN = 1,
		FILTER_WHEEL_EM_MAX = 8,
		FILTER_WHEEL_EM_DEFAULT = 1,
		
		FILTER_WHEEL_DIC_MIN = 1,
		FILTER_WHEEL_DIC_MAX = 5,
		FILTER_WHEEL_DIC_DEFAULT = 1,
		
		TURRET_MIN = 1,
		TURRET_MAX = 4,
		TURRET_DEFAULT = 1,
		

		LAMP_MIN = 0,
		LAMP_MAX = 1,
		LAMP_DEFAULT = 0,
		
		AUTOFOCUS_MIN = 0,
		AUTOFOCUS_MAX = 1,
		AUTOFOCUS_DEFAULT = 0

	};

	static SimDeviceFW* getInstance();
    ~SimDeviceFW();

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

	double _xPos;
	double _xPos_C;
	BOOL _xPos_B;
	
	double _yPos;
	double _yPos_C;
	BOOL _yPos_B;
	
	long _shutterPos;
	long _shutterPos_C;
	BOOL _shutterPos_B;
	
	long _shutterWaitTime;
	long _shutterWaitTime_C;
	BOOL _shutterWaitTime_B;

	long _filterExPos;
	long _filterExPos_C;
	BOOL _filterExPos_B;
	
	long _filterEmPos;
	long _filterEmPos_C;
	BOOL _filterEmPos_B;
	
	long _filterDicPos;
	long _filterDicPos_C;
	BOOL _filterDicPos_B;
	
	long _turretPos;
	long _turretPos_C;
	BOOL _turretPos_B;
	
	long _lampPos;
	long _lampPos_C;
	BOOL _lampPos_B;
	
	long _afPos;
	long _afPos_C;
	BOOL _afPos_B;

    static bool _instanceFlag;
    static auto_ptr<SimDeviceFW> _single;
};