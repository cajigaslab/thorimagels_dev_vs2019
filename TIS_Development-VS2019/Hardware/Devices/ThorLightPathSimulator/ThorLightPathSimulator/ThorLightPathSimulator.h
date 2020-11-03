#pragma once

#include "Serial.h"
#include "stdafx.h"
#include "ParamInfo.h"
#include<list>

//singleton device class
class ThorLightPathSimulator : IDevice
{
public:
	static ThorLightPathSimulator* getInstance();
	~ThorLightPathSimulator();

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
		GALVO_GALVO = 0,
		GALVO_RESONANT = 1,
		CAMERA = 2,
		DEVICE_NUM = 3,
	};

	static bool _instanceFlag;        ///singleton created flag
	static auto_ptr<ThorLightPathSimulator> _single;        ///pointer to internal Device object

	static const string _deviceSignature[DEVICE_NUM];

	wchar_t _errMsg[MSG_SIZE];

	BOOL _deviceDetected[DEVICE_NUM + 1];
	CritSect _critSect;

	std::list<ParamInfo *> _tableParams;

	ThorLightPathSimulator();
	long BuildParamTable();
	long ExecuteCmd(ParamInfo *pParamInfo);
	long ExecuteCmd(long paramID, std::vector<unsigned char> cmd, double &readBackValue);	
	void LogMessage(wchar_t *message);
};

