#pragma once

#include "Serial.h"
#include "stdafx.h"
#include "ParamInfo.h"
#include<list>

#define MAX_SLEEPTIME				5000

#define PMT2100_OFFSET_LOW_LIMIT	422
#define PMT2100_OFFSET_HIGH_LIMIT	3572
#define PMT2110_OFFSET_LOW_LIMIT	422
#define PMT2110_OFFSET_HIGH_LIMIT	2995
#define HPD_OFFSET_LOW_LIMIT		0
#define HPD_OFFSET_HIGH_LIMIT		1000

#define PMT1000_GAIN_HIGH_LIMIT		814 //900*890/1000+13
#define PMT2100_GAIN_LOW_LIMIT		458 //500*890/1000+13
#define PMT2100_GAIN_HIGH_LIMIT		992    //1100*890/1000+13
#define PMT2110_GAIN_LOW_LIMIT		613
#define PMT2110_GAIN_HIGH_LIMIT		993
#define HPD_GAIN_LOW_LIMIT			0
#define HPD_GAIN_HIGH_LIMIT			4000

#define HPD_VBR_LOW_LIMIT			0
#define HPD_VBR_HIGH_LIMIT			4000

#define PMT_TRIP_THRESH				1500
#define HPD_TRIP_THRESH				4000

#define BW_300MHz					300000000
#define BW_200MHz					200000000
#define BW_80MHz					80000000
#define BW_30MHz					30000000
#define BW_15MHz					15000000
#define BW_2_5MHz					2500000
#define BW_250kHz					250000

const string THORLABS_VID = "1313";
const string THORLABS_PMT1000_PID = "2E01";
const string THORLABS_PMT2100_PID = "2E02";
const string THORLABS_PMT2106_PID = "2E03";  //GaAsP with shutter
const string THORLABS_HPD1000_PID = "2E06";
const string THORLABS_SIPM100_PID = "2E07";
const string THORLABS_PMT2110_PID = "2E08";
const string THORLABS_PMT3100_PID = "2E09";

const USHORT MGMSG_HW_REQ_INFO = 0x0005;
const USHORT MGMSG_HW_GET_INFO = 0x0006;
const USHORT MGMSG_PMT_REQ_INFO = 0x4500;
const USHORT MGMSG_PMT_GET_INFO = 0x4501;
const USHORT MGMSG_REQ_PMT_PARAMS = 0x4502;
const USHORT MGMSG_GET_PMT_PARAMS = 0x4503;
const USHORT MGMSG_SET_PMT_PARAMS = 0x4504;
const USHORT MGMSG_SET_PMT_STATE = 0x4505;
const USHORT MGMSG_REQ_PMT_STATE = 0x4506;
const USHORT MGMSG_GET_PMT_STATE = 0x4507;
const USHORT MGMSG_SET_PMT_FREQ = 0x4508;
const USHORT MGMSG_REQ_PMT_FREQ = 0x4509;
const USHORT MGMSG_GET_PMT_FREQ = 0x450A;
const USHORT MGMSG_SET_PMTTRIPCLR = 0x450B;
const USHORT MGMSG_REQ_PMTTRIP = 0x450C;
const USHORT MGMSG_GET_PMTTRIP = 0x450D;
const USHORT MGMSG_REQ_PMTTRIPCNT = 0x450E;
const USHORT MGMSG_GET_PMTTRIPCNT = 0x450F;
const USHORT MGMSG_SET_PMT_GAIN = 0x4510;
const USHORT MGMSG_REQ_PMT_GAIN = 0x4511;
const USHORT MGMSG_GET_PMT_GAIN = 0x4512;
const USHORT MGMSG_SET_PMT_OFFSET = 0x4513;
const USHORT MGMSG_REQ_PMT_OFFSET = 0x4514;
const USHORT MGMSG_GET_PMT_OFFSET = 0x4515;
const USHORT MGMSG_SET_PMT_SER = 0x4516;
const USHORT MGMSG_SET_PMT_PRE_OFFSET = 0x4517;
const USHORT MGMSG_REQ_PMT_PRE_OFFSET = 0x4518;
const USHORT MGMSG_GET_PMT_PRE_OFFSET = 0x4519;
const USHORT MGMSG_SET_PMT_ATTENUATION = 0x451A;
const USHORT MGMSG_REQ_PMT_ATTENUATION = 0x451B;
const USHORT MGMSG_GET_PMT_ATTENUATION = 0x451C;
const USHORT MGMSG_REQ_HPD_VBR = 0x4520;
const USHORT MGMSG_GET_HPD_VBR = 0x4521;
const USHORT MGMSG_SET_HPD_VBR = 0x4522;
const USHORT MGMSG_REQ_PMT_SATURATION_COUNT = 0x4523;
const USHORT MGMSG_GET_PMT_SATURATION_COUNT = 0x4524;

//singleton device class
class ThorDetector : IDevice
{

public:

	enum
	{
		DEVICE_NUM = 6,
	};

	enum TripStatus
	{
		OFF = 0,
		ON = 1,
		TRIPPED = 2
	};

	static ThorDetector* getInstance();
	~ThorDetector();

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
	static auto_ptr<ThorDetector> _single;        ///pointer to internal Device object

	wchar_t _errMsg[MSG_SIZE];

	CSerial _serialPort[DEVICE_NUM];

	BOOL _deviceDetected[DEVICE_NUM + 1];
	string _settingsSerialNumber[DEVICE_NUM];
	long _connectedPMTs;	
	long _sleepTimeAfterMoveComplete;
	long _defaultGain[DEVICE_NUM];
	string _serialNumber[DEVICE_NUM];
	string _firmwareVersion[DEVICE_NUM];
	CritSect _critSect;
	vector<vector<string>> _snList;
	long _baudRate;
	long _detectorType[DEVICE_NUM];
	long _gainMin[DEVICE_NUM];
	long _gainMax[DEVICE_NUM];
	long _offsetMin[DEVICE_NUM];
	long _offsetMax[DEVICE_NUM];
	long _bandwidthMin[DEVICE_NUM];
	long _bandwidthMax[DEVICE_NUM];
	long _atenuationMin[DEVICE_NUM];
	long _atenuationMax[DEVICE_NUM];
	long _gainSliderMin[DEVICE_NUM];
	long _gainSliderMax[DEVICE_NUM];
	long _offsetSliderMin[DEVICE_NUM];
	long _offsetSliderMax[DEVICE_NUM];

	std::map<long, ParamInfo *> _tableParams;

	static const string _deviceSignature[DEVICE_NUM];
	static const long _pmtSelect[DEVICE_NUM];
	static const vector<wstring> _listOfTypes;

	ThorDetector();
	long DestroyParamTable();
	long BuildParamTable();
	long ExecuteCmdParam(ParamInfo *pParamInfo);
	long ExecuteCmd(std::vector<unsigned char> cmd, int portIndex, double &readBackValue, long requestStatus, USHORT &returnedCommand);
	long RetrieveDevicesInfo();
	void ExecutePositionNow();
	long ConnectToComPort(wstring portName, long pmtIndex);

	long GetDeviceParameters();

	double rndup(double val,int decPlace);

	static void LogMessage(wchar_t* logMsg, long eventLevel);
	std::vector<unsigned char> GetBytes(int value);
	double Round(double number, int decimals);

};