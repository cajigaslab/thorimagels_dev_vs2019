#include "stdafx.h"
#include "DC2200visa.h"
#include <memory>
class ThorDC2200 : IDevice
{
private:
	ThorDC2200();
public:
	static ThorDC2200* getInstance();
	~ThorDC2200();

	long FindDevices(long& DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double& param);
	long SetParamBuffer(const long paramID, char* buffer, long size);
	long GetParamBuffer(const long paramID, char* buffer, long size);
	long SetParamString(const long paramID, wchar_t* str);
	long GetParamString(const long paramID, wchar_t* str, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long& status);
	long ReadPosition(DeviceType deviceType, double& pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t* msg, long size);


private:
	int ModeStrToEnum(char* str);
	char* ModeEnumToStr(int mode);
	long GetLastError();
private:
	static bool _instanceFlag;
	static std::auto_ptr<ThorDC2200> _single;
	static long _ledOutputEnabled;

	ViSession _viRM;
	ViSession _session;
	double _brightness;
	double _brightnessToSet;
	double _maxCurrent1;
	double _maxCurrent2;
	long _status;
	int _mode;
	int _modeToSet;
	BOOL _updateMode;
	BOOL _updateTerminal;
	BOOL _updateBrightness;
	BOOL _updateLEDState;
	long _terminalNumber;
	long _lamp1enable;
	long _lamp2enable;
};