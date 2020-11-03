#pragma once

//path to ActiveX controls
//#import "C:\Program Files\Thorlabs\APT\APT Server\MG17System.ocx" no_namespace named_guids
//#import "C:\Program Files\Thorlabs\APT\APT Server\MG17Motor.ocx" no_namespace named_guids

///* A unique identifier for the source object */
//#define MG17MOTORDISPEVNTID   422
//#define MG17SYSTEMDISPEVNTID   423

//singleton device class
class ThorPowerSimulator : IDevice
{

private:

	double _rPos;        ///r location
	double _rPos_C;
	BOOL _rPos_B;

	BOOL _rHome_B;        ///home r stage

	static bool _instanceFlag;        ///singleton created flag
	static ThorPowerSimulator* _single;        ///pointer to internal Device object

	HINSTANCE _hinstance;        ///dll handle

	DeviceType _zDevType;
	int _zHome;
	double _zVel;
	ThorPowerSimulator();

public:
	long SerialNumR;        ///serial number of x stage

	///parameter limits
	enum
	{
		POWER_MIN = 0,
		POWER_MAX = 100,
		R_DEFAULT = 100,

		POWER_HOME_MIN = 0,
		POWER_HOME_MAX = 1,
		POWER_HOME_DEFAULT = 0,

		POWER_VELOCITY_MIN = 1,
		POWER_VELOCITY_MAX = 10,
		POWER_VELOCITY_DEFAULT = 7,
	};

	enum ControlTypes
	{
		R_CONTROL,
	};

	static ThorPowerSimulator* getInstance();
	~ThorPowerSimulator();

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

	void SetHInstance(HINSTANCE hinst);
	HINSTANCE GetHInstance();

};
