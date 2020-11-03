#pragma once

#include "Serial.h"
#include "stdafx.h"
#include "ParamInfo.h"
#include<list>


//singleton device class
class ThorBScope : IDevice
{

public:

	///parameter limits
	enum
	{
		STAGE_POSITION_MIN = -100,				//X,Y,Z stage range, unit: mm
		STAGE_POSITION_MAX = 100,				//92000: original value before conversion requirement
		STAGER_POSITION_MIN = -90,				//R stage range, unit: degree
		STAGER_POSITION_MAX = 90,
		STAGE_POSITION_DEFAULT = 0,

		STAGE_HOME_MIN = -100,
		STAGE_HOME_MAX = 100,					//100000
		STAGE_HOME_DEFAULT = 0,

		STAGE_VELOCITY_MIN = 0,
		STAGE_VELOCITY_MAX = 1100,				//1<<24
		
		STAGE_X_POSITION_SENDFCR = 256000,		//factor applied before sending to serial
		STAGE_Y_POSITION_SENDFCR = 256000,
		STAGE_Z_POSITION_SENDFCR = 256000,
		STAGE_R_POSITION_SENDFCR = -237281,
		STAGE_X_POSITION_READFCR = 20,			//factor applied after reading from serial
		STAGE_Y_POSITION_READFCR = 20,
		STAGE_Z_POSITION_READFCR = 100,
		STAGE_R_POSITION_READFCR = -8730,
		STAGE_VELOCITY_FCR = 655360000,			//factor applied before sending
		STAGE_ACCEL_FCR = 687194767,
		CONVERT_POSITION_SEND = 1,				//Convert function #1 for position
		CONVERT_VELACCEL_SEND = 2,				//Convert function #2 for velocity/acceleration
		
		STAGE_X_VELOCITY_DEFAULT = 580,			//4000: original value, same for the followings 
		STAGE_Y_VELOCITY_DEFAULT = 580,			//4000
		STAGE_Z_VELOCITY_DEFAULT = 458,			//20000
		STAGE_R_VELOCITY_DEFAULT = 763,			//4000

		STAGE_ACCEL_MIN = 0,
		STAGE_ACCEL_MAX = 10000,					//65000

		STAGE_X_ACCEL_DEFAULT = 4365,				//50
		STAGE_Y_ACCEL_DEFAULT = 4365,				//50
		STAGE_Z_ACCEL_DEFAULT = 4365,				//50
		STAGE_R_ACCEL_DEFAULT = 4365,				//100

	};


	static ThorBScope* getInstance();
	~ThorBScope();

	long FindDevices(long &DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double &param);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);
	long StatusPosition(long &status);
	long ReadPosition(DeviceType deviceType,double &pos);	
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);

private:
	static bool _instanceFlag;        ///singleton created flag
	static auto_ptr<ThorBScope> _single;        ///pointer to internal Device object

	wchar_t _errMsg[MSG_SIZE];

	CSerial _serialPort;
	BOOL _deviceDetected;

	unsigned char _dataBuffer[9];
	unsigned char _readBuffer[9];
	unsigned char _address;

	//Use _blockUpdateParam to to block updating any paremeter in the getParam funtion until after the new position
	//has been sent to the device. This variable allows GetParam calls in between SetParam and StartPosition, without
	//changing the new set position, until the command has been sent to the device. This device must be set to FALSE
	//at the end of StartPosition to allow updating the parameters when reading positions from the device. 
	long _blockUpdateParam;

	long _numOfAxes;	

	double _rMin;
	double _rMax;

	double _xMin;
	double _xMax;
	double _xThreshold;
	bool   _xInvert;

	double _yMin;
	double _yMax;
	double _yThreshold;
	bool   _yInvert;

	double _zMin;
	double _zMax;
	double _zThreshold;

	double *_motorsParamsInvert; //disable 1; enable -1;default 1
	double _stageStartingPos[4];
	CritSect _critSect;

	std::map<long, ParamInfo *> _tableParams;

	ThorBScope();
	long BuildParamTable();
	long ExecuteCmdStr(ParamInfo *pParamInfo);
	long ExecuteCmdStr(string cmd, long &readBackValue);
	void ExecutePositionNow(const long paramID); //use for single command
	void ExecutePositionNow();
	long StartPositionSingleCommand(const long paramID);
	long CheckMotorConnection();	
	long QueryInitialMotorPos();	
	double rndup(double val,int decPlace);
	long ResetStageZeroPosition(long stageID);	//0: X, 1: Y, 2: Z.
	void LogMessage(wchar_t *message);
	void InvertMinMaxRange(double& min, double& max);
};

