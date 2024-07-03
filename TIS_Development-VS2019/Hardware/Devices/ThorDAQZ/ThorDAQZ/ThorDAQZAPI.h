//#include <string.h>
#ifndef __THORDAQZAPI_H__
#define __THORDAQZAPI_H__


#include "stdafx.h"
#include "ThorDAQZ.h"
#ifdef __cplusplus
extern "C"
{
#endif

	//DllExport long FindDevices(long& deviceCount);

	//DllExport long SelectDevice(const long Device);

	//DllExport long TeardownDevice();

	//DllExport long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault);

	//DllExport long SetParam(const long paramID, const double param);

	//DllExport long GetParam(const long paramID, double& param);

	//DllExport long PreflightPosition();

	//DllExport long SetupPosition();

	//DllExport long StartPosition();

	//DllExport long StatusPosition(long& status);

	//DllExport long ReadPosition(IDevice::DeviceType deviceType, double& pos);

	//DllExport long PostflightPosition();

	//DllExport long GetLastErrorMsg(wchar_t* msg, long size);

	//DllExport long SetParamString(long paramID, wchar_t* str);

	//DllExport long GetParamString(const long paramID, wchar_t* str, long size);

	//DllExport long SetParamBuffer(const long paramID, char* buffer, long size);

	//DllExport long GetParamBuffer(const long paramID, char* buffer, long size);

	DllExport BuildAndGetFastZWaveform(const double updateRate, const long samplesFrame, const long samplesFlyback, const double frameRate, ThorDAQZWaveformParams* waveformParams);

	DllExport GetRemoteFocusFastZWaveform(const double updateRate, const long samplesFrame, const long samplesFlyback, const double frameRate, ThorDAQZWaveformParams* waveformParams);

#ifdef __cplusplus
}
#endif

#endif