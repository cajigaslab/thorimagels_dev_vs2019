#pragma once

#include "Serial.h"
#include "stdafx.h"
#include "ParamInfo.h"
#include<list>


const string THORLABS_VID = "1313";
const string THORLABS_BCMPA3_PID = "2019";
//singleton device class
class ThorBCMPA2 : IDevice
{
public:
	static ThorBCMPA2* getInstance();
	~ThorBCMPA2();

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
	long ReadPosition(DeviceType deviceType,double &pos);	
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);

private:
	enum
	{
		//POWERREG_1 = 0,
		//POWERREG_2 = 1,
//		POWERREG_3 = 0,//future use
		DEVICE_NUM = 1,


	};

	ThorBCMPA2();

	static bool _instanceFlag;        ///singleton created flag
	static auto_ptr<ThorBCMPA2> _single;        ///pointer to internal Device object

	static const string _deviceSignature[DEVICE_NUM];
	static const string _deviceSignatureCalibration[DEVICE_NUM];

	const long WAIT_TIME_BETWEEN_SEND_COMMANDS;

	wchar_t _errMsg[MSG_SIZE];

	CSerial _serialPort[DEVICE_NUM];
	BOOL _deviceDetected[DEVICE_NUM + 1];
	long _timeOutTime;
	string _settingsSerialNumber[DEVICE_NUM];
	long _useShutter[DEVICE_NUM];
	BOOL _foundByPID;

	static CritSect critSec;
	
	CritSect testSec;

	std::map<long, ParamInfo *> _tableParams;

	long BuildParamTable();
	long ExecuteCmd(ParamInfo *pParamInfo);
	long ExecuteCmd(long paramID, std::vector<unsigned char> cmd, double &readBackValue);
	void LogMessage(wchar_t *message);
	double Round(double number, int decimals);
	long GetDeviceSerialNumber(long deviceIndx, string &deviceSerialNumber);
	long VerifySerialNumbers(long portID[]);
};

typedef void (__cdecl * funcConvert)(long, double, double&);

