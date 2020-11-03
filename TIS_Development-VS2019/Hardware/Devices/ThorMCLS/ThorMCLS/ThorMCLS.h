#pragma once

#include "stdafx.h"
#include "ThorSerialCom.h"
#include "InterpolationTable.h"

class ThorMCLS : IDevice
{
private:
	ThorMCLS();
public:


	enum
	{
		MCLS_SYSTEM_ENABLE = 0x01,		
		LASER1_ENABLE = 0x01,
		LASER2_ENABLE = 0x02,
		LASER3_ENABLE = 0x03,
		LASER4_ENABLE = 0x04,
		LASER1_POWER = 0x05,
		LASER2_POWER = 0x06,
		LASER3_POWER = 0x07,
		LASER4_POWER = 0x08
	};

	static ThorMCLS* getInstance();
	~ThorMCLS();

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
	long CalibrateMCLS();
	void GetMinMaxFromReadBuffer(double &minVal, double &maxVal);

private:
	long SetMCLSLaserEnable(int MCLSID, bool enable);
	long SetMCLSPower(int MCLSID, double power);
	long SetSystemEnable(int enable);
	long GetControllerID(char* ctrlID);
	long ReadMCLSPort();
	long ParseReadBuffer(int inquiryID);
	long QueryStatus(void);
	long QueryID();

private:

	long _bEnableSet;
	
	long _laser1Enable;
	long _laser2Enable;
	long _laser3Enable;
	long _laser4Enable;
	double _laser1Power;
	double _laser2Power;
	double _laser3Power;
	double _laser4Power;

	long _laser1Enable_C;
	long _laser2Enable_C;
	long _laser3Enable_C;
	long _laser4Enable_C;
	double _laser1Power_C;
	double _laser2Power_C;
	double _laser3Power_C;
	double _laser4Power_C;

	//bool _r_laser1Enable;
	//bool _r_laser2Enable;
	//bool _r_laser3Enable;
	//bool _r_laser4Enable;
	//int  _r_laser1Power;
	//int  _r_laser2Power;
	//int  _r_laser3Power;
	//int  _r_laser4Power;
	//bool _r_systemEnable;

	double _laser1Min;
	double _laser1Max;
	double _laser2Min;
	double _laser2Max;
	double _laser3Min;
	double _laser3Max;
	double _laser4Min;
	double _laser4Max;

	static bool _instanceFlag;
	static auto_ptr<ThorMCLS> _single;

	CThorSerialCom _serialPort;


	wchar_t _errMsg[MSG_SIZE];

	//CSerial _serialPort;

	bool _deviceDetected;

	TCHAR _dataBuffer[256];
	TCHAR _readBuffer[256];

	std::wstring getTableDirectoryPath();
	static const int MAX_CHANNELS = 4;
	double linearizePower(double minPower, double maxPower, double curPower, int channelIndex);
	InterpolationTable linearizationTables[MAX_CHANNELS];
	
};