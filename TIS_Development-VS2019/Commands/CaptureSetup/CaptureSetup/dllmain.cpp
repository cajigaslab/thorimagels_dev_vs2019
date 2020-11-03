// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "CaptureSetup.h"

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


DllExportLiveImage GetCommandGUID(GUID *guid)
{
	return CaptureSetup::getInstance()->GetCommandGUID(guid);
}

DllExportLiveImage SetupCommand()
{
	return CaptureSetup::getInstance()->SetupCommand();
}

DllExportLiveImage TeardownCommand()
{
	return CaptureSetup::getInstance()->TeardownCommand();
}

DllExportLiveImage GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return CaptureSetup::getInstance()->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

DllExportLiveImage SetParam(const long paramID, const double param)
{
	return CaptureSetup::getInstance()->SetParam(paramID, param);

}

DllExportLiveImage GetParam(const long paramID, double &param)
{
	return CaptureSetup::getInstance()->GetParam(paramID, param);
}

DllExportLiveImage SetCustomParamsBinary(const char *buf)
{
	return CaptureSetup::getInstance()->SetCustomParamsBinary(buf);

}

DllExportLiveImage GetCustomParamsBinary(char *buf)
{
	return CaptureSetup::getInstance()->GetCustomParamsBinary(buf);

}

DllExportLiveImage SaveCustomParamsXML(void *fileHandle)
{
	return CaptureSetup::getInstance()->SaveCustomParamsXML(fileHandle);
}

DllExportLiveImage LoadCustomParamXML(void *fileHandle)
{
	return CaptureSetup::getInstance()->LoadCustomParamXML(fileHandle);
}

DllExportLiveImage Execute()
{
	return CaptureSetup::getInstance()->Execute();
}

DllExportLiveImage Status(long &status)
{
	return CaptureSetup::getInstance()->Status(status);
}
