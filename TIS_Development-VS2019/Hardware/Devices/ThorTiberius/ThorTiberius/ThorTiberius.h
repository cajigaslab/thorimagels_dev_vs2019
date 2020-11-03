#pragma once

#include "stdafx.h"
#include "ThorSerialCom.h"

class ThorTiberius : IDevice
{
private:
	ThorTiberius();
public:

	static ThorTiberius* getInstance();
	~ThorTiberius();

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
	long GetLastErrorMsg(wchar_t *msg, long size);

private:
	void GetTuningStatus(long & status);
	void GetVWPosition(long & position);
	long SetLaserPosition(long position);
	void GetShutterPosition(long &position);
	long SetLaserShutterPosition(long position);
	long SetSequence(char *buffer);
	long ReadPort(long timeout);

private:

	long _laser1Position;
	long _laser1Position_C;

	long _laser1Min;
	long _laser1Max;

	long _laser1ShutterPosition;
	long _laser1ShutterPosition_C;

	long _laser1ShutterMin;
	long _laser1ShutterMax;

	char _seqBuffer[4];
	char _seqBuffer_C[4];

	DWORD _lastUpdateTime;
	DWORD _lastWLReadTime;
	DWORD _lastShutterReadTime;

	CritSect _critSect;

	static bool _instanceFlag;
	static auto_ptr<ThorTiberius> _single;

	CThorSerialCom _serialPort;

	wchar_t _errMsg[MSG_SIZE];

	//CSerial _serialPort;

	bool _deviceDetected;

	TCHAR _dataBuffer[256];
	TCHAR _readBuffer[256];

};