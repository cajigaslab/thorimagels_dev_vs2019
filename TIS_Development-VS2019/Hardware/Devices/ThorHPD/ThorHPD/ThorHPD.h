#pragma once

#include "Serial.h"
#include "stdafx.h"
#include "ParamInfo.h"
#include<list>

#define MAX_SLEEPTIME 5000

//singleton device class
class ThorHPD : IDevice
{

public:

	enum
	{
		DEVICE_NUM = 6,
	};

	enum
	{
		GAIN_MIN = 0,
		GAIN_MAX = 8500
	};

	enum
	{
		VBR_MIN = 0,
		VBR_MAX = 450
	};

	static ThorHPD* getInstance();
	~ThorHPD();

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

	static bool _instanceFlag;        ///singleton created flag
	static auto_ptr<ThorHPD> _single;        ///pointer to internal Device object

	wchar_t _errMsg[MSG_SIZE];

	CSerial _serialPort[DEVICE_NUM];

	BOOL _deviceDetected[DEVICE_NUM + 1];
	string _settingsSerialNumber[DEVICE_NUM];
	int _numOfAxes;
	long _connectedPMTs;	
	long _sleepTimeAfterMoveComplete;
	long _defaultGain[DEVICE_NUM];
	string _serialNumber[DEVICE_NUM];
	string _firmwareVersion[DEVICE_NUM];
	CritSect _critSect;

	std::map<long, ParamInfo *> _tableParams;

	static const string _deviceSignature[DEVICE_NUM];
	static const long _pmtSelect[DEVICE_NUM];

	ThorHPD();
	long DestroyParamTable();
	long BuildParamTable();
	long ExecuteCmd(ParamInfo *pParamInfo);
	long ExecuteCmd(std::vector<unsigned char> cmd, int portIndex, double &readBackValue);
	long SetDetectorType(long index, long type);
	long RetrieveDevicesInfo();
	void ExecutePositionNow();

	long GetDeviceParameters();

	double rndup(double val,int decPlace);

	void LogMessage(wchar_t *message);
	std::vector<unsigned char> GetBytes(int value);
	double Round(double number, int decimals);

};