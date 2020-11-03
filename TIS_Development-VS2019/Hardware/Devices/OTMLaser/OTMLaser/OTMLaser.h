#pragma once

#include "stdafx.h"
//#include "ThorSerialCom.h"
#include "InterpolationTable.h"


class OTMLaser : IDevice
{
private:
	OTMLaser();
	
public:


	enum
	{
		OTM_SYSTEM_ENABLE = 0x01,		
		LASER1_ENABLE = 0x01,
		LASER2_ENABLE = 0x02,
		LASER3_ENABLE = 0x03,
		LASER1_POWER = 0x05,
		LASER2_POWER = 0x06,
		LASER3_POWER = 0x07
		
	};

	static OTMLaser* getInstance();
	~OTMLaser();

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
	long ReadPosition(DeviceType deviceType, double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);
	long CalibrateOTMLaser();
	void GetMinMaxFromReadBuffer(double &minVal, double &maxVal);
	long LaserPosHome(double laserId);

private:
	long SetOTMLaserEnable(int OTMID, bool enable);
	long SetOTMLaserFocus(double delta, bool isCCW);
	long SetOTMPower(int OTMID, double power);
	long GetControllerID(char* ctrlID);

private:
	long _laserPower;
	long _laserPower_C;
	long _laserPowerMin;
	long _laserPowerMax;
	long _bEnableSet;	
	long _laser1Enable;	
	double _laser1Power;
	long _laser1Enable_C;	
	double _laser1Power_C;
	double _laser1Pos;
	double _laser1Pos_C;
	long _laser2Enable;	
	double _laser2Power;
	long _laser2Enable_C;	
	double _laser2Power_C;
	double _laser2Pos;
	double _laser2Pos_C;
	long _laser3Enable;	
	double _laser3Power;
	long _laser3Enable_C;	
	double _laser3Power_C;
	double _laser3Pos;
	double _laser3Pos_C;
	long _flag;
	long _IsRS232;

	double _laser1Min;
	double _laser1Max;
	double _laser2Min;
	double _laser2Max;
	double _laser3Min;
	double _laser3Max;

	long _shutter1Flag;
	long _shutter2Flag;

	static bool _instanceFlag;
	static auto_ptr<OTMLaser> _single;

	CSerial _serialPort;
	//CSerial _CserialPort[DEVICE_NUM];

	wchar_t _errMsg[MSG_SIZE];

	//CSerial _serialPort;

	bool _deviceDetected;

	TCHAR _dataBuffer[256];
	TCHAR _readBuffer[256];

	std::wstring getTableDirectoryPath();
	static const int MAX_CHANNELS = 3;
	double linearizePower(double minPower, double maxPower, double curPower, int channelIndex);
	InterpolationTable linearizationTables[MAX_CHANNELS];
	
};