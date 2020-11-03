
#pragma once

#define ThorErrChk(fnCall) if (200 > (error=(fnCall)) > 100) throw "fnCall";
#define ThorFnFailed(error)             ( 200 > (error) > 100 )


class ThorShutterDig3 : IDevice
{
private:
    ThorShutterDig3();
public:

	enum
	{
	};

	static ThorShutterDig3* getInstance();
    ~ThorShutterDig3();

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
	
	long ThorShutterDig3::SetDigital(long val);
	int  ThorShutterDig3::CloseNITasks();

private:

	long _shutPos;
	long _shutPos_C;
	long _shutPos_B;
	
	long _shutterDelayMS;
	string _devName;
	wchar_t _errMsg[256];

	TaskHandle _taskHandleDO0;

	static bool _deviceDetected;
    static bool _instanceFlag;
    static auto_ptr<ThorShutterDig3> _single;
	CritSect _critSect;
};