// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "SelectHardware.h"

BOOL APIENTRY DllMain( HMODULE hModule,
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


DllExport GetCommandGUID(GUID *guid)
{
	return SelectHardware::getInstance()->GetCommandGUID(guid);
}

DllExport SetupCommand()
{
	return SelectHardware::getInstance()->SetupCommand();
}

DllExport TeardownCommand()
{
	return SelectHardware::getInstance()->TeardownCommand();
}

DllExport GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return SelectHardware::getInstance()->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

DllExport SetParam(const long paramID, const double param)
{
	return SelectHardware::getInstance()->SetParam(paramID, param);

}

DllExport GetParam(const long paramID, double &param)
{
	return SelectHardware::getInstance()->GetParam(paramID, param);
}

DllExport SetCustomParamsBinary(const char *buf)
{
	return SelectHardware::getInstance()->SetCustomParamsBinary(buf);

}

DllExport GetCustomParamsBinary(char *buf)
{
	return SelectHardware::getInstance()->GetCustomParamsBinary(buf);

}

DllExport SaveCustomParamsXML(void *fileHandle)
{
	return SelectHardware::getInstance()->SaveCustomParamsXML(fileHandle);
}

DllExport LoadCustomParamXML(void *fileHandle)
{
	return SelectHardware::getInstance()->LoadCustomParamXML(fileHandle);
}

DllExport Execute()
{
	return SelectHardware::getInstance()->Execute();
}

DllExport Status(long &status)
{
	return SelectHardware::getInstance()->Status(status);
}

DllExport DeslectCameras()
{
	return SelectHardware::getInstance()->DeslectCameras();
}

DllExport GetCameraID()
{
	return SelectHardware::getInstance()->GetCameraID();
}

DllExport GetBleachID()
{
	return SelectHardware::getInstance()->GetBleachID();
}

DllExport GetDeviceID(long deviceType)
{
	return SelectHardware::getInstance()->GetDeviceID(deviceType);
}

DllExport GetCameraType(long camID,long &type)
{
	return SelectHardware::getInstance()->GetCameraType(camID, type);
}

DllExport LoadDevice(unsigned long deviceType, long deviceIndex)
{
	return SelectHardware::getInstance()->LoadDevice(deviceType, deviceIndex);
}

DllExport LoadCamera(long cameraIndex)
{
	return SelectHardware::getInstance()->LoadCamera(cameraIndex);
}

DllExport SetDetectorName(long detectorID, wchar_t* name)
{
	return SelectHardware::getInstance()->SetDetectorName(detectorID, name);
}

DllExport SetActiveBleachingScanner(long scannerIndex)
{
	return SelectHardware::getInstance()->SetActiveBleachingScanner(scannerIndex);
}

DllExport GetCameraParameter(long camID, long paramID, double &param)
{
	return SelectHardware::getInstance()->GetCameraParameter(camID, paramID, param);
}

DllExport GetDeviceParamInfo(long deviceID, long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return SelectHardware::getInstance()->GetDeviceParamInfo(deviceID,paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport UpdateAndPersistCurrentDevices()
{
	return SelectHardware::getInstance()->UpdateAndPersistCurrentDevices();
}