#pragma once

#include "Serial.h"
#include "stdafx.h"
#include "ParamInfo.h"
#include<list>

//singleton device class
class ThorVBE : IDevice
{
public:
	static ThorVBE* getInstance();
	~ThorVBE();

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

private:
	enum BEDevs
	{
		BE1 = 0,
		BE2 = 1,
		BE3 = 2,
		DEVICE_NUM = 3,
	};
	
	static bool _instanceFlag;        ///singleton created flag
	static auto_ptr<ThorVBE> _single;        ///pointer to internal Device object

		static const string _deviceSignature[BEDevs::DEVICE_NUM];
		long _magMin;
		long _magMax;
		long _wavelengthMin;
		long _wavelengthMax;

	wchar_t _errMsg[MSG_SIZE];

	CSerial _serialPort[BEDevs::DEVICE_NUM];
	BOOL _deviceDetected[BEDevs::DEVICE_NUM + 1];
	string _settingsSerialNumber[BEDevs::DEVICE_NUM];

	CritSect _critSect;

	std::list<ParamInfo *> _tableParams;

	ThorVBE();
	long BuildParamTable();
	long ExecuteCmd(ParamInfo *pParamInfo);
	long ExecuteCmd(long paramID, std::vector<unsigned char> cmd, double &readBackValue);
	void LogMessage(wchar_t *message);
	long GetDeviceSerialNumber(long deviceIndx, string &deviceSerialNumber);
	long VerifySerialNumbers(long portID[]);
};

