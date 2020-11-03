#pragma once

#include "stdafx.h"
#include "ParamInfo.h"
#include "..\..\..\..\Common\thread.h"
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <visa.h>

//#ifdef _UNICODE
//#define _T(x)      L ## x
//#else /* _UNICODE */
//#define _T(x)      x
//#endif /* _UNICODE */

class ThorPMT2100 : IDevice
{
private:
    ThorPMT2100();
public:


	enum
	{
		DEVICE_NUM = 6,
	};

	enum
	{
		BANDWIDTH_MIN = 250000,
		BANDWIDTH_MAX = 80000000
	};

	enum
	{
		GAIN_MIN = 0,
		GAIN_MAX = 100
	};
	
	static ThorPMT2100* getInstance();
    ~ThorPMT2100();

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
	long DestroyParamTable();
	long CreateParamTable();
	long ExecuteCmd(ParamInfo* pParamInfo);
	long ExecuteCmd(const string cmd, const long pmtIndex, const long bQuery, string &response);
	long QueryPMTStatus(const long pmtIndex, long &status);
	long QueryPMTSafety(const long pmtIndex, long &safetyStatus);
	long QueryPMTSetting(ParamInfo* pParamInfo, double &param);
	long GetPMTTypeAndRanges();
	long QueryDeviceModelNumber(string deviceAddress, string &response);
	long QueryDeviceIDN(ViSession session, vector<string> &parsedID);
	void LogMessage(wchar_t* message);
	double GainPercentToVolts(long pmtIndex, long gainPercent); //returns the gain in Volts
	long GainVoltsToPercent(long pmtIndex, double gainVolts); //returns the gain in Percent

	string _pmtType[DEVICE_NUM];
	BOOL _deviceDetected[DEVICE_NUM + 1];
	wchar_t _errMsg[MSG_SIZE];
	ViSession _viSession[DEVICE_NUM];
	std::map<long, ParamInfo*> _tableParams; //stores the parameter info for each parameter
	double _pmtMaxGainVolts[DEVICE_NUM];
	double _pmtMinGainVolts[DEVICE_NUM];
	string _serialNumber[DEVICE_NUM];
	double _firmwareVersion[DEVICE_NUM];
	long _gainCountsMax[DEVICE_NUM];
	double _gainPower[DEVICE_NUM];
	long _startupGainPercent[DEVICE_NUM];
	ViSession _viRM;
	string _deviceAddress[DEVICE_NUM];
	long _connectedPMTs;	
    static bool _instanceFlag;
    static auto_ptr<ThorPMT2100> _single;
	static const string _deviceSignature[DEVICE_NUM];
	static const long _pmtSelect[DEVICE_NUM];	
	static const wstring _addressStructure;
	static CritSect _critSect;
};