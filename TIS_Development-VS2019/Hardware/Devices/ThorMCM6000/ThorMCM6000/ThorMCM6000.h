#pragma once
#include "..\..\..\..\Common\Device.h"
#include <memory>
#include <ctime>
#include <strsafe.h>
#include "MCM6000.h"


using namespace std;

// This class is exported from the ThorMesoScan.dll
class MCM6000Stage :IDevice
{
private:
	double _xPosition;
	double _yPosition;
	double _zPosition;
	double _rPosition;
	long _lpInvertedPos;
	long _etInvertedPos;
	long _ggLightpathPos;
	long _grLightpathPos;
	long _camLightpathPos;
	double _xMoveByDistance;
	double _yMoveByDistance;
	double _zMoveByDistance;
	double _rMoveByDistance;

	bool _xPosition_B;
	bool _yPosition_B;
	bool _zPosition_B;
	bool _rPosition_B;
	bool _lpInvertedPos_B;
	bool _etInvertedPos_B;
	long _ggLightpathPos_B;
	long _grLightpathPos_B;
	long _camLightpathPos_B;
	bool _xMoveByDistance_B;
	bool _yMoveByDistance_B;
	bool _zMoveByDistance_B;
	bool _rMoveByDistance_B;

	double _xPosition_C;
	double _yPosition_C;
	double _zPosition_C;
	double _rPosition_C;
	double _zePosition_C;
	long _lpInvertedPos_C;
	long _etInvertedPos_C;

	MCM6000* _device;
	static MCM6000Stage* _instance;
	MCM6000Stage();
public:
	static MCM6000Stage* getInstance();
	~MCM6000Stage();

	long FindDevices(long& DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double& param);
	long SetParamBuffer(const long paramID, char* buffer, long size);
	long GetParamBuffer(const long paramID, char* buffer, long size);
	long SetParamString(const long paramID, wchar_t* str);
	long GetParamString(const long paramID, wchar_t* str, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long& status);
	long ReadPosition(DeviceType deviceType, double& pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t* msg, long size);
};