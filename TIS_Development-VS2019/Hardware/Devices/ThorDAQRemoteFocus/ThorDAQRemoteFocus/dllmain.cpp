// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorDAQRemoteFocus.h"
#include "ThorDAQRemoteFocusAPI.h"
#include "windows.h"
#pragma once

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


DllExport long FindDevices(long& deviceCount)
{
	return ThorDAQRemoteFocus::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorDAQRemoteFocus::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorDAQRemoteFocus::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault)
{
	return ThorDAQRemoteFocus::getInstance()->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorDAQRemoteFocus::getInstance()->SetParam(paramID, param);
}

DllExport long GetParam(const long paramID, double& param)
{
	return ThorDAQRemoteFocus::getInstance()->GetParam(paramID, param);
}

DllExport long PreflightPosition()
{
	return ThorDAQRemoteFocus::getInstance()->PreflightPosition();
}

DllExport SetupPosition()
{
	return ThorDAQRemoteFocus::getInstance()->SetupPosition();
}

DllExport StartPosition()
{
	return ThorDAQRemoteFocus::getInstance()->StartPosition();
}

DllExport StatusPosition(long& status)
{
	return ThorDAQRemoteFocus::getInstance()->StatusPosition(status);
}

DllExport ReadPosition(IDevice::DeviceType deviceType, double& pos)
{
	return ThorDAQRemoteFocus::getInstance()->ReadPosition(deviceType, pos);
}

DllExport PostflightPosition()
{
	return ThorDAQRemoteFocus::getInstance()->PostflightPosition();
}

DllExport GetLastErrorMsg(wchar_t* msg, long size)
{
	return ThorDAQRemoteFocus::getInstance()->GetLastErrorMsg(msg, size);
}

DllExport SetParamString(long paramID, wchar_t* str)
{
	return ThorDAQRemoteFocus::getInstance()->SetParamString(paramID, str);
}

DllExport GetParamString(const long paramID, wchar_t* str, long size)
{
	return ThorDAQRemoteFocus::getInstance()->GetParamString(paramID, str, size);
}

DllExport SetParamBuffer(const long paramID, char* buffer, long size)
{
	return ThorDAQRemoteFocus::getInstance()->SetParamBuffer(paramID, buffer, size);
}

DllExport GetParamBuffer(const long paramID, char* buffer, long size)
{
	return ThorDAQRemoteFocus::getInstance()->GetParamBuffer(paramID, buffer, size);
}

DllExport GetRemoteFocusFastZWaveform(const double updateRate, const long samplesFrame, const long samplesFlyback, const double frameRate, ThorDAQZWaveformParams* waveformParams)
{
	return ThorDAQRemoteFocus::getInstance()->GetRemoteFocusFastZWaveform(updateRate, samplesFrame, samplesFlyback, frameRate, waveformParams);
}