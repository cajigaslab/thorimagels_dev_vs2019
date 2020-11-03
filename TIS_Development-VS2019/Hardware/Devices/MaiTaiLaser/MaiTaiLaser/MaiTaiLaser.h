#pragma once

#include "Serial.h"
#include "stdafx.h"
#include "ParamInfo.h"

/**********************************************************************************************//**
 * @class	CMaiTaiLaser
 *
 * @brief	A maitai laser.
 *
 **************************************************************************************************/

class CMaiTaiLaser : IDevice {

public:
	enum
	{
		POWER_DEFAULT = 0,
		HUM_DEFAULT = 0,
		WAVELENGTH_DEFAULT = 800,
	};

private:
	struct LASER_PARAMS
	{
		long WaveLength;
		long ShutterStatus;
		long Shutter2Status;
		long DeviceType;
	} _laserParams;

public:
	static CMaiTaiLaser* getInstance();
	~CMaiTaiLaser();
	long FindDevices ( long& DeviceCount );
	long SelectDevice ( const long Device );
	long TeardownDevice();
	long GetParamInfo ( const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault );
	long SetParam ( const long paramID, const double param );
	long GetParam ( const long paramID, double& param );
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition ( long& status );
	long ReadPosition ( DeviceType deviceType, double& pos );
	long PostflightPosition();
	long GetLastErrorMsg ( wchar_t* msg, long size );	

private:
	CMaiTaiLaser();
	void LogMessage ( wchar_t* message );
	long QueryLaserManufacturer ( void );
	long QueryLaserParams ( void );
	wchar_t* convertCharArrayToLPCWSTR ( const char* charArray );
	long QueryParams ( std::vector<unsigned char> cmd );
	long SetParams ( ParamInfo* pParamInfo );
	long BuildParamTable ( void );
	long GetParametersID ( void );
	long QueryParams ( ParamInfo* pParamInfo );
	double GetWarmUpStatus();
	void TurnOnLaser();
	void TurnOffLaser();
	void UpdateParameter( const long paramID, const double param );

	static void* staticPollingHandler(void* obj);
	void pollingHandler();

	long SetWatchDog(long seconds);

private:
	
	thread* _polling;
	bool _isLaserOff;
	static bool _instanceFlag;
	static auto_ptr<CMaiTaiLaser> _single;
	long _waveLengthMax;
	long _waveLengthMin;
	char* _serialPortBuffer;
	long _blockUpdateParam;
	CSerial _serialPort;
	wchar_t _errMsg[MSG_SIZE];
	bool _deviceDetected;
	CritSect critSect;
	std::map<long, ParamInfo*> _tableParams;
	double _curShutterStatus;
	double _curShutter2Status;

};