#pragma once

#include "stdafx.h"
#include "Serial.h"
#include "ParamInfo.h"

const string TOPTICA_VID = "0403";
const string TOPTICA_CLE_PID = "6001";

class TopticaiChrome : IDevice
{
private:
	TopticaiChrome();
public:

	static TopticaiChrome* getInstance();
	~TopticaiChrome();

	long FindDevices(long& DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double& param);
	long SetParamString(const long paramID, wchar_t* str);
	long GetParamString(const long paramID, wchar_t* str, long size);
	long SetParamBuffer(const long paramID, char* buffer, long size);
	long GetParamBuffer(const long paramID, char* buffer, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long& status);
	long ReadPosition(DeviceType deviceType, double& pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t* msg, long size);

private:
	long DestroyParamTable();
	long BuildParamTable(void);
	long ExecuteCmdGet(wstring pParamCmdGet);
	long ExecuteCmdSet(long pParamID, wstring pParamCmdSet, double val);
	static void LogMessage(wchar_t* logMsg, long eventLevel);

private:

	CritSect _critSect;

	static bool _instanceFlag; ///singleton created flag
	static auto_ptr<TopticaiChrome> _single; ///pointer to internal Device object

	CSerial _serialPort;

	std::map<long, ParamInfo*> _tableParams;

	wchar_t _errMsg[MSG_SIZE];

	bool _deviceDetected;

	char* _waveBuffer;
	char* _enableBuffer;
	char* _readBuffer;
	char* _powerBuffer;
	char* _readyBuffer;
	char* _analogBuffer;
	char* _allEnableBuffer;

};