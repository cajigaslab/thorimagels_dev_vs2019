#pragma once

#include "Serial.h"
#include "stdafx.h"
#include "ParamInfo.h"
#include<list>

//singleton device class
class ThorDDR05 : IDevice
{
public:
	static ThorDDR05* getInstance();
	~ThorDDR05();

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
		POWERREG_1 = 0,
//		POWERREG_2 = 0,//future use
//		POWERREG_3 = 0,//future use
		DEVICE_NUM = 1,


	};

	ThorDDR05();

	static bool _instanceFlag;        ///singleton created flag
	static auto_ptr<ThorDDR05> _single;        ///pointer to internal Device object

	static const string _deviceSignature[DEVICE_NUM];
	static const string _deviceSignatureCalibration[DEVICE_NUM];

	const long WAIT_TIME_BETWEEN_SEND_COMMANDS;

	wchar_t _errMsg[MSG_SIZE];

	CSerial _serialPort[DEVICE_NUM];
	BOOL _deviceDetected[DEVICE_NUM + 1];
	long _timeOutTime;
	string _settingsSerialNumber[DEVICE_NUM];
	long _useShutter[DEVICE_NUM];

	static CritSect critSec;
	
	CritSect testSec;

	std::map<long, ParamInfo *> _tableParams;

	long BuildParamTable();
	long ExecuteCmd(ParamInfo *pParamInfo);
	long ExecuteCmd(long paramID, std::vector<unsigned char> cmd, double &readBackValue);
	void LogMessage(wchar_t *message);
	double Round(double number, int decimals);
	long WaitForRotationComplete(long deviceIndex);
	long GetDeviceSerialNumber(long deviceIndx, string &deviceSerialNumber);
	long SendInitializationMessages(CSerial& serialPort);
	long VerifySerialNumbers(long portID[]);
	long IsCorrectDevice();
	long HomeDevice(CSerial & device);
	bool BlockUntilDataArrives(CSerial & serialPort, int numberOfBytes, long maxTimeInMillis, long pollingFrequencyInMillis);
	long ClearSerialBuffer(CSerial & serialPort);

#pragma pack(push,1)
	struct IncomingMessageHeader {
		unsigned char messageID;
		unsigned char messageIDPart2;
		unsigned char messageLength;
		unsigned char messageLengthPart2;
		unsigned char dest;
		unsigned char source;
	};

	struct OutgoingMessageHeader {
		unsigned char messageID;
		unsigned char idPart2;
		unsigned char chanIdent;
		unsigned char usuallyBlank;
		unsigned char dest;
		unsigned char source;
	};

	struct HomeParamInfo{
		IncomingMessageHeader header;
		short chanID;
		short homeDir;
		short limitSwitch;
		long homeVelocity;
		long offsetDistance;
	};
	
	struct VelocityParamsInfo {
		short chanID;
		long minVelocity;
		long acceleration;
		long maxVelocity;
	};
	
	struct GetVelocityParams{
		IncomingMessageHeader header;
		VelocityParamsInfo settings;
		};
	
	struct SetVelocityParams{
		OutgoingMessageHeader header;
		VelocityParamsInfo settings;
	};
#pragma pack(pop)
	
};

typedef void (__cdecl * funcConvert)(long, double, double&);

