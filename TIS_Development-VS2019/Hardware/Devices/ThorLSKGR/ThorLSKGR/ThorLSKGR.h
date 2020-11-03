#pragma once

#include "Serial.h"
#include "ParamInfo.h"
class ThorLSKGR : IDevice
{
public:	
	~ThorLSKGR();

	static ThorLSKGR* getInstance();	
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
	ThorLSKGR();
	long ExecuteCmd(ParamInfo *pParamInfo);
	long ExecuteCmd(std::vector<unsigned char> cmd, double &readBackValue);
	void LogMessage(wchar_t *message);
	std::vector<unsigned char> GetBytes(int value);
	long CoarseAlignDataLoadFile();
	long GetFirmwareVersionAndSerialNumber(std::wstring &version,std::wstring &serialNum);
	long GetSerialNumber();
	long CreateParamTable();
	long DestroyParamTable();	
	static bool _instanceFlag;
	static std::shared_ptr <ThorLSKGR> _single;
	std::wstring _firmwareVersion;
	std::wstring _serialNumber;
	long _rsInitMode;
	bool _deviceDetected;
	CSerial _serialPort;
	long* _twoWayZones;///<digital pot zones for correcting two way alignment
	std::map<long, ParamInfo *> _tableParams;
	wchar_t _errMsg[MSG_SIZE];
	CritSect _critSect;

	enum
	{
		PMT_GAIN_MIN = 0,
		PMT_GAIN_MAX = 100,
		PMT_GAIN_DEFAULT = 0,

		SCANNER_ZOOM_MIN = 5,
		SCANNER_ZOOM_MAX = 255,
		SCANNER_ZOOM_DEFAULT = 120
	};
};

