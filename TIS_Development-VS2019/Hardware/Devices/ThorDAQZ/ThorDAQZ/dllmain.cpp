// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorDAQZ.h"
#include "ThorDAQZAPI.h"
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
	return ThorDAQZ::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorDAQZ::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorDAQZ::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault)
{
	return ThorDAQZ::getInstance()->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorDAQZ::getInstance()->SetParam(paramID, param);
}

DllExport long GetParam(const long paramID, double& param)
{
	return ThorDAQZ::getInstance()->GetParam(paramID, param);
}

DllExport long PreflightPosition()
{
	return ThorDAQZ::getInstance()->PreflightPosition();
}

DllExport SetupPosition()
{
	return ThorDAQZ::getInstance()->SetupPosition();
}

DllExport StartPosition()
{
	return ThorDAQZ::getInstance()->StartPosition();
}

DllExport StatusPosition(long& status)
{
	return ThorDAQZ::getInstance()->StatusPosition(status);
}

DllExport ReadPosition(IDevice::DeviceType deviceType, double& pos)
{
	return ThorDAQZ::getInstance()->ReadPosition(deviceType, pos);
}

DllExport PostflightPosition()
{
	return ThorDAQZ::getInstance()->PostflightPosition();
}

DllExport GetLastErrorMsg(wchar_t* msg, long size)
{
	return ThorDAQZ::getInstance()->GetLastErrorMsg(msg, size);
}

DllExport SetParamString(long paramID, wchar_t* str)
{
	return ThorDAQZ::getInstance()->SetParamString(paramID, str);
}

DllExport GetParamString(const long paramID, wchar_t* str, long size)
{
	return ThorDAQZ::getInstance()->GetParamString(paramID, str, size);
}

DllExport SetParamBuffer(const long paramID, char* buffer, long size)
{
	return ThorDAQZ::getInstance()->SetParamBuffer(paramID, buffer, size);
}

DllExport GetParamBuffer(const long paramID, char* buffer, long size)
{
	return ThorDAQZ::getInstance()->GetParamBuffer(paramID, buffer, size);
}

DllExport BuildAndGetFastZWaveform(const double updateRate, const long samplesFrame, const long samplesFlyback,const double frameRate, ThorDAQZWaveformParams* waveformParams)
{
	return ThorDAQZ::getInstance()->BuildAndGetFastZWaveform(updateRate, samplesFrame, samplesFlyback, frameRate, waveformParams);
}