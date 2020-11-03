// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "RunSample.h"
//#include "StackWalker.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
//#ifdef DEBUG
//			InitAllocCheck(ACOutput_XML);
//#endif
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
//#ifdef DEBUG
//			DeInitAllocCheck();
//#endif
		}
		break;

	}
	return TRUE;
}


DllExport_RunSample GetCommandGUID(GUID *guid)
{
	return RunSample::getInstance()->GetCommandGUID(guid);
}

DllExport_RunSample SetupCommand()
{
	return RunSample::getInstance()->SetupCommand();
}

DllExport_RunSample TeardownCommand()
{
	return RunSample::getInstance()->TeardownCommand();
}

DllExport_RunSample GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return RunSample::getInstance()->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

DllExport_RunSample SetParam(const long paramID, const double param)
{
	return RunSample::getInstance()->SetParam(paramID, param);

}

DllExport_RunSample GetParam(const long paramID, double &param)
{
	return RunSample::getInstance()->GetParam(paramID, param);
}

DllExport_RunSample SetCustomParamsBinary(const char *buf)
{
	return RunSample::getInstance()->SetCustomParamsBinary(buf);

}

DllExport_RunSample GetCustomParamsBinary(char *buf)
{
	return RunSample::getInstance()->GetCustomParamsBinary(buf);

}

DllExport_RunSample SaveCustomParamsXML(void *fileHandle)
{
	return RunSample::getInstance()->SaveCustomParamsXML(fileHandle);
}

DllExport_RunSample LoadCustomParamXML(void *fileHandle)
{
	return RunSample::getInstance()->LoadCustomParamXML(fileHandle);
}

DllExport_RunSample Execute()
{
	return RunSample::getInstance()->Execute();
}

DllExport_RunSample Stop()
{
	return RunSample::getInstance()->Stop();
}

DllExport_RunSample Status(long &status)
{
	return RunSample::getInstance()->Status(status);
}
