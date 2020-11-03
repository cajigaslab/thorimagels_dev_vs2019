// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "AcquireDataFactory.h"

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

///************		AcquireData Export		************///
DllExport_AcquireData long EnterAcquire()
{
	return AcquireDataFactory::getInstance()->getAcquireInstance()->Enter();
}

DllExport_AcquireData long ExitAcquire()
{
	return AcquireDataFactory::getInstance()->getAcquireInstance()->Exit();
}

DllExport_AcquireData long StartAcquireData()
{
	long ret = TRUE;
	if(TRUE == AcquireDataFactory::getInstance()->getAcquireInstance()->LoadXML())
	{
		if(FALSE == AcquireDataFactory::getInstance()->getAcquireInstance()->SetupChannels())
		{
			return FALSE;
		}
		if(FALSE == AcquireDataFactory::getInstance()->getAcquireInstance()->SetupFileIO())
		{
			return FALSE;
		}
		ret = AcquireDataFactory::getInstance()->getAcquireInstance()->Start();
	}
	else
	{
		return FALSE;
	}
	return ret;
}

DllExport_AcquireData long StartAsyncAcquireData()
{
	long ret = TRUE;
	if(FALSE == AcquireDataFactory::getInstance()->getAcquireInstance()->StartAsync())
	{
		return FALSE;
	}
	return ret;
}

DllExport_AcquireData long GetErrorMessage(wchar_t * path, long size)
{
	std::wstring ws = AcquireDataFactory::getInstance()->getAcquireInstance()->GetLastError();
	wcscpy_s(path, size, ws.c_str());
	return TRUE;
}

DllExport_AcquireData long GetStructData(CompDataStruct* dptr)
{
	return	AcquireDataFactory::getInstance()->getAcquireInstance()->CopyStructData(dptr);
}

DllExport_AcquireData long InitCallBack(SpectralUpdateCallback su, DataUpdateCallback du) 
{	
	return	AcquireDataFactory::getInstance()->getAcquireInstance()->InitCallbacks(su, du);
}

DllExport_AcquireData long LoadEpisode() 
{	
	return	ChannelCenter::getInstance()->LoadEpisode();
}

DllExport_AcquireData long SpectralAnalysis() 
{	
	return	ChannelCenter::getInstance()->SpectralAnalysis();
}

DllExport_AcquireData long SetFileSaving(long toSave)
{
	return	AcquireDataFactory::getInstance()->getAcquireInstance()->SetSaving(toSave);
}

DllExport_AcquireData long PauseAcquireData()
{
	return AcquireDataFactory::getInstance()->getAcquireInstance()->Pause();
}

DllExport_AcquireData long RestartAcquireData()
{
	return AcquireDataFactory::getInstance()->getAcquireInstance()->Restart();
}

DllExport_AcquireData long StopAcquireData()
{	
	return AcquireDataFactory::getInstance()->getAcquireInstance()->Stop();
}

DllExport_AcquireData long StopAsyncAcquireData()
{	
	return AcquireDataFactory::getInstance()->getAcquireInstance()->StopAsync();
}

DllExport_AcquireData void StopFileLoading()
{
	ChannelCenter::getInstance()->_stopLoading = TRUE;
}

DllExport_AcquireData long IsInAcquire()
{
	return AcquireDataFactory::getInstance()->getAcquireInstance()->GetAcquiring();
}

DllExport_AcquireData long IsInAsyncAcquire()
{
	return AcquireDataFactory::getInstance()->getAcquireInstance()->GetAsyncAcquiring();
}

DllExport_AcquireData long IsFileLoading()
{
	return ChannelCenter::getInstance()->_isLoading;
}

DllExport_AcquireData long IsFileSaving()
{
	return AcquireDataFactory::getInstance()->getAcquireInstance()->GetSaving();
}

DllExport_AcquireData void UpdateVariable() 
{	
	ChannelCenter::getInstance()->ReloadGlobalVariables();
}

///************		End AcquireData Export		************///

