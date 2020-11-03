#pragma once

#include "stdafx.h"
#include "Serial.h"

//singleton device class
class ThorMLSStage: IDevice
{

private:

	double _xPos;///x location
	double _xPos_C;
	BOOL _xPos_B;

	double _yPos;///y location
	double _yPos_C;
	BOOL _yPos_B;

	double _xVel;///x velocity
	double _xVel_C;
	BOOL _xVel_B;
	double _xAcc;///x acceleration
	double _xAcc_C;
	

	double _yVel;///y velocity
	double _yVel_C;
	BOOL _yVel_B;
	double _yAcc; ///y acceleration
	double _yAcc_C;
	

	BOOL _xHome_B;///home x stage
	
	BOOL _yHome_B;///home y stage

	static bool _instanceFlag;///singleton created flag
	static ThorMLSStage *_single;///pointer to internal Device object

	HINSTANCE _hinstance;///dll handle

	wchar_t _errMsg[MSG_SIZE];
	
	double _xMin;
	double _xMax;
	bool _xInvert;
	int _xEncoderToMM;
	double _xDefault;

	double _yMin;
	double _yMax;
	bool _yInvert;
	int _yEncoderToMM;
	double _yDefault;
	CSerial _serialPort;
	bool _deviceDetected;
	double *_motorsParamsInvert; //disable 1; enable -1;default 1	

	static const long MAX_SERIAL_BUFFER_SIZE;
	static DWORD _lastReadTime;
	static const long ENCODER_TO_MM_PER_SEC_CONVERSION;
	static const double ENCODER_TO_MM_PER_SEC_SQR_CONVERSION;

	static const long WAIT_TO_READ_DELAY;
	
	long _comAliveCounter;

	//Use _blockUpdateParam to to block updating any paremeter in the getParam funtion until after the new position
	//has been sent to the device. This variable allows GetParam calls in between SetParam and StartPosition, without
	//changing the new set position, until the command has been sent to the device.
	long _blockUpdateParam;

	CritSect _critSect;
		

public:

	///parameter limits
	enum
	{

		X_HOME_MIN = 0,
		X_HOME_MAX = 0,
		X_HOME_DEFAULT = 0,

		Y_HOME_MIN = 0,
		Y_HOME_MAX = 0,
		Y_HOME_DEFAULT = 0,

		X_VELOCITY_MIN = 0,
		X_VELOCITY_MAX = 250,
		X_VELOCITY_DEFAULT = 70,
		
		Y_VELOCITY_MIN = 0,
		Y_VELOCITY_MAX = 250,
		Y_VELOCITY_DEFAULT = 70,
		ACCELERATION_DEFAULT = 700
	};

	enum ControlTypes
	{
		X_CONTROL,
		Y_CONTROL
	};

	static ThorMLSStage* getInstance();
	~ThorMLSStage();

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
	long WaitUntilSettled ();

	long ReadPosition(DeviceType deviceType,double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);
	long GetXYStageRange();
	void SetHInstance(HINSTANCE hinst);
	HINSTANCE GetHInstance();

private:
	ThorMLSStage();
	static void LogMessage(wchar_t *message,long eventLevel);
	long SetVelocityX(long vx, long ax);
	long SetVelocityY(long vy, long ay);
	long ReqVelocityX(double &vx, double &ax);
	long ReqVelocityY(double &vy, double &ay);
	long MoveStageX(long x);
	long MoveStageY(long y);
	long CommAlive();
	long ReqHardwareInfo();
};

