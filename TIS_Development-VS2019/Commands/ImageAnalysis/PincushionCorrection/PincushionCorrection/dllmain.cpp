// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "PincushionCorrection.h"

extern Point * pLookUp;
extern char * pImageResultBuffer;

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
		break;
	case DLL_PROCESS_DETACH:
		if(pLookUp)
		{
			delete pLookUp;		
		}
		if(pImageResultBuffer )
		{
			delete pImageResultBuffer;
		}
		break;
	}
	return TRUE;
}

