#pragma once

#include "Serial.h"
#include "stdafx.h"
#include "ParamInfo.h"
#include<list>

#define MAX_SLEEPTIME 5000

//singleton device class
class ThorMCM3000Aux : IDevice
{

public:

	///parameter limits
	enum
	{
		STAGE_POSITION_DEFAULT = 0,
		STAGE_HOME_DEFAULT = 0,
	};

	static ThorMCM3000Aux* getInstance();
	~ThorMCM3000Aux();

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
	static auto_ptr<ThorMCM3000Aux> _single;        ///pointer to internal Device object
	
	//Use _blockUpdateParam to to block updating any paremeter in the getParam funtion until after the new position
	//has been sent to the device. This variable allows GetParam calls in between SetParam and StartPosition, without
	//changing the new set position, until the command has been sent to the device. This device must be set to FALSE
	//at the end of StartPosition to allow updating the parameters when reading positions from the device. 
	long _blockUpdateParam;

	wchar_t _errMsg[MSG_SIZE];

	CSerial _serialPort;

	BOOL _deviceDetected;

	int _numOfAxes;
	
	double _xMin;
	double _xMax;
	double _xThreshold;

	double _yMin;
	double _yMax;
	double _yThreshold;

	double _zMin;
	double _zMax;
	double _zThreshold;
	long _sleepTimeAfterMoveComplete;
	double *_motorsParamsInvert; //disable 1; enable -1;default 1	
	CritSect critSect;

	std::map<long, ParamInfo *> _tableParams;
	
	static const double MOTOR_COUNTER_COEFF[9];// = { 0, 211.66666, 0, 500, 500, 100.0, 5555.555, 211.66666, 11.90625};
	static const double MOTOR_COUNTER_COEFF_OLDFIRMWARE[7];// = { 0, 39.0625, 211.6667, 1, 500, 100.0, 1.0 }; (use this for firmware version < 2)

	struct MOTOR_PARAMS {
		long motorAxis;
		long motorType;
		double MotorEncCoef;
		long ParamStop;
		long ParamHome;
		long ParamPos;
		long ParamPosCurrent;
		long ParamZero;
		double initialPos;
		double StagePosMax;
		double StagePosMin;		
		long ParamInvert;	
	} _motorsParams [3];

	enum MOTOR_TYPE
	{
		NO_MOTOR = 0,
		LNR = 1,
		PLS,
		A_SCOPE_Z,
		B_SCOPE,
		B_SCOPE_Z,
		B_SCOPE_Z_HIGH_TORQUE
	};
	
	ThorMCM3000Aux();
	long BuildParamTable();
	long ExecuteCmd(ParamInfo *pParamInfo);
	long ExecuteCmd(std::vector<unsigned char> cmd, double &readBackValue);
	void ExecutePositionNow();
	long CheckMotorConnection();
	long GetDeviceParameters();
	long GetFirmwareVersion(double &firmwareVersion);
	long GetMotorType();
	long SetMotorParameters();
	long QueryMotorTypeOldFirmware();
	long QueryInitialMotorPos();
	double rndup(double val,int decPlace);
	long ResetStageZeroPosition(long stageID);	//0: X, 1: Y, 2: Z.
	void LogMessage(wchar_t *message);
	std::vector<unsigned char> GetBytes(int value);
	double Round(double number, int decimals);
	void InvertMinMaxRange(double& min, double& max);
};