// dllmain.cpp : Defines the entry point for the DLL application.
#include "ImageStoreWrapper.h"

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


///ImageStoreWrapper Export
IMAGESTORELIBRARY_API long AddScan(double zStartPosUM, double zStopPosUM, double zStepSizeUM, long frameCount)
{
	return ImageStoreWrapper::getInstance()->AddScan(zStartPosUM, zStopPosUM, zStepSizeUM, frameCount);
}

IMAGESTORELIBRARY_API long ClearImageStore()
{
	return ImageStoreWrapper::getInstance()->ClearImageStore();
}

IMAGESTORELIBRARY_API long GetImageStoreInfo(char* fileWithPath, long regionID, long &regionCount, long &width, long &height, long &channelCount, long &zMaxCount, long &timeCount, long &specCount)
{
	long ret = FALSE;
	if(FALSE == ImageStoreWrapper::getInstance()->LoadImageStore(fileWithPath))
		return ret;

	return ImageStoreWrapper::getInstance()->GetImageStoreInfo(regionID, regionCount, width, height, channelCount, zMaxCount, timeCount, specCount);
}

IMAGESTORELIBRARY_API long ReadImageStoreData(char* buf, long channelCount, long width, long height, long zSliceID, long timeID, long specID, long regionID = 0)
{
	return ImageStoreWrapper::getInstance()->ReadChannelData(buf, channelCount, width, height, zSliceID, timeID, specID, regionID);
}

IMAGESTORELIBRARY_API long SetupImageStore(wchar_t * path, void* exp, long doCompression)
{
	return ImageStoreWrapper::getInstance()->SetupImageStore(path, exp, doCompression);
}

IMAGESTORELIBRARY_API long SetScan(long scanID)
{
	return ImageStoreWrapper::getInstance()->SetScan(scanID);
}

IMAGESTORELIBRARY_API long SetRegion(long regionID)
{
	return ImageStoreWrapper::getInstance()->SetRegion(regionID);
}

IMAGESTORELIBRARY_API long SaveData(void* buf, uint16_t channelID, uint16_t z, uint16_t t, uint16_t s = 1)
{
	return ImageStoreWrapper::getInstance()->SaveData(buf, channelID, z, t, s);
}

IMAGESTORELIBRARY_API long AdjustScanTCount(int tCount)
{
	return ImageStoreWrapper::getInstance()->AdjustScanTCount(tCount);
}