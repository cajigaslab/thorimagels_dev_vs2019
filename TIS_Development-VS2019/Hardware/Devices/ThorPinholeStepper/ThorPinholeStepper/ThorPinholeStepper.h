#pragma once

#include "Serial.h"

/// <summary>
/// Class ThorPinholeStepper.
/// </summary>
class ThorPinholeStepper : IDevice
{
private:
    ThorPinholeStepper();

	enum 
	{
		/// <summary>
		/// Position mode lookup table
		/// </summary>
		POS_LOOKUP,
		/// <summary>
		/// Position using the encoder value and update alignment configuration file
		/// </summary>
		POS_AND_ALIGNMENT,
		/// <summary>
		/// Position using the encoder value
		/// </summary>
		POS_MOVE_ONLY
	};
public:

	static ThorPinholeStepper* getInstance();
    ~ThorPinholeStepper();

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

	long _pPinholeSeparation;
	long _pPinholeSeparation_C;
	BOOL _pPinholeSeparation_B;
	
	long _pNonPinholeFreq;
	long _pNonPinholeFreq_C;
	BOOL _pNonPinholeFreq_B;

	const long PINHOLE_LOCATIONS;
	const long PINHOLE_SEPARATION_DEFAULT;
	const long NON_PINHOLE_FREQ_DEFAULT;
	

    static bool _instanceFlag;
    static auto_ptr<ThorPinholeStepper> _single;

	vector<long> _pinholeLocations;
	CSerial _serialPort;


	double _pMin;
	double _pMax;


	BOOL _deviceDetected;
	unsigned char _dataBuffer[9];
	unsigned char _readBuffer[9];
	unsigned char _address;
};