#pragma once
#include "..\..\..\..\Common\Device.h"
#include <memory>
#include <ctime>
#include <strsafe.h>
#include "MCM301.h"

#define MCM301_EXPORT

#if defined(MCM301_EXPORT)
#define DllExportMCM301 __declspec(dllexport)
#else
//    definitions used when using DLL
#define DllExport __declspec(dllimport)
#endif

using namespace std;

// This class is exported from the ThorMesoScan.dll
class DllExportMCM301 MCM301Stage :IDevice
{
public:
	~MCM301Stage();

	static MCM301Stage* getInstance();

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
	long IsMCM301Connected();
	long IsCondenserAvailable();

private:
	double _xPosition;
	double _yPosition;
	double _zPosition;
	double _rPosition;
	double _condenserPosition;
	double _xMoveByDistance;
	double _yMoveByDistance;
	double _zMoveByDistance;
	double _rMoveByDistance;
	double _condenserMoveByDistance;

	bool _xPosition_B;
	bool _yPosition_B;
	bool _zPosition_B;
	bool _rPosition_B;
	bool _condenserPosition_B;
	bool _xMoveByDistance_B;
	bool _yMoveByDistance_B;
	bool _zMoveByDistance_B;
	bool _rMoveByDistance_B;
	bool _condenserMoveByDistance_B;

	double _xPosition_C;
	double _yPosition_C;
	double _zPosition_C;
	double _rPosition_C;
	double _zePosition_C;
	double _condenserPosition_C;

	static std::shared_ptr <MCM301> _device;
	static bool _instanceFlag;
	static std::shared_ptr <MCM301Stage> _instance;
	MCM301Stage();
};