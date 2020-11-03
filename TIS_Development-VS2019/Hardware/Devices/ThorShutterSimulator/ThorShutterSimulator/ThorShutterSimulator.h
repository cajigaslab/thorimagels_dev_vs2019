
#pragma once


class ThorShutterSimulator : IDevice
{
private:
    ThorShutterSimulator();
public:
	
	static ThorShutterSimulator* getInstance();
    ~ThorShutterSimulator();

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
	
	void ThorShutterSimulator::LogMessage(wchar_t *message);

private:

	long _shutPos;
	long _shutPos_C;
	long _shutPos_B;
	
	long _shutterDelayMS;
	wchar_t _errMsg[256];
	
	BOOL _deviceDetected;
    static bool _instanceFlag;
    static auto_ptr<ThorShutterSimulator> _single;
	CritSect _critSect;
};