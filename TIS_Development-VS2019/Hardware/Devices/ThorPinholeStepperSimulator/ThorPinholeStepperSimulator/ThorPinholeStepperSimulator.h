#pragma once

#include "Serial.h"

class ThorPinholeStepperSimulator : IDevice
{
private:
    ThorPinholeStepperSimulator();
public:

	static ThorPinholeStepperSimulator* getInstance();
    ~ThorPinholeStepperSimulator();

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
	long GetLastErrorMsg(wchar_t * errMsg, long size);

private:
	
long MoveStepper(long pos);

void LogMessage(wchar_t *message);

private:
	wchar_t _errMsg[MSG_SIZE];

	long _pPos;
	long _pPos_C;
	BOOL _pPos_B;
	
	long _pPosAlign;
	long _pPosAlign_C;
	BOOL _pPosAlign_B;
	
	long _pMode;
	long _pMode_C;
	BOOL _pMode_B;

	const long PINHOLE_LOCATIONS;

    static bool _instanceFlag;
    static auto_ptr<ThorPinholeStepperSimulator> _single;

	vector<long> _pinholeLocations;
	CSerial _serialPort;


	double _pMin;
	double _pMax;


	BOOL _deviceDetected;
	long _numDevices;
	unsigned char _dataBuffer[9];
	unsigned char _readBuffer[9];
	unsigned char _address;
};