// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "WinDVI.h"

#define WINDVI_DLLEXPORT extern "C" long __declspec( dllexport )

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

WINDVI_DLLEXPORT ChooseDVI(const wchar_t* monitorId)
{
	return CWinDVI::getInstance()->ChooseDVI(monitorId);
}

WINDVI_DLLEXPORT EditBMP(int id, unsigned char* bmpBuf, BITMAPINFO bmpInfo)
{
	return CWinDVI::getInstance()->EditBMP(id, bmpBuf, bmpInfo);
}

WINDVI_DLLEXPORT ClearBMPs()
{
	CWinDVI::getInstance()->ClearBMPs();
	return TRUE;
}

WINDVI_DLLEXPORT CreateDVIWindow(int w, int h)
{
	return CWinDVI::getInstance()->CreateDVIWindow(w, h);
}

WINDVI_DLLEXPORT DestroyDVIWindow()
{
	CWinDVI::getInstance()->DestroyDVIWindow();
	return TRUE;
}

WINDVI_DLLEXPORT DisplayBMP(int id)
{
	return CWinDVI::getInstance()->DisplayBMP(id);
}

WINDVI_DLLEXPORT GetLastErrorMsg(wchar_t* msg, long size)
{
	return CWinDVI::getInstance()->GetLastErrorMsg(msg, size);
}

WINDVI_DLLEXPORT GetStatus(long& status)
{
	return CWinDVI::getInstance()->GetStatus(status);
}

//WINDVI_DLLEXPORT StopDVI()
//{
//	CWinDVI::getInstance()->stopWinDVI = TRUE;
//	return TRUE;
//}