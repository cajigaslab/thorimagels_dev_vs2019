// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorMesoScan.h"

//HINSTANCE hDLLInstance;

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	vector<wchar_t> pathBuf; 
	std::wstring path, expFile = L"";
	size_t copied = 0;
	HANDLE hFile;
	WIN32_FIND_DATA FindFileData;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//hDLLInstance = hModule;

		//get module location
		do 
		{
			pathBuf.resize(pathBuf.size()+MAX_PATH);
			copied = GetModuleFileName(hModule, &pathBuf.at(0), static_cast<DWORD>(pathBuf.size()));
		} while( copied >= pathBuf.size() );

		pathBuf.resize(copied);
		path = std::wstring(pathBuf.begin(),pathBuf.end());
		path = path.substr(0,path.rfind('\\')+1).append(L"*.xml");

		//try find experiment file, pick first if many
		hFile = FindFirstFile(path.c_str(), &FindFileData);
		while (hFile != INVALID_HANDLE_VALUE)
		{
			expFile = std::wstring(FindFileData.cFileName);
			if (std::wstring::npos == expFile.find(L"ThorMesoScanSettings.xml"))
				break;
			FindNextFile(hFile,&FindFileData);
			if(0 == expFile.compare(FindFileData.cFileName))
				break;
		}
		FindClose(hFile);
		hFile = NULL;

		if (0 < expFile.size())
		{
			ThorLSMCam::getInstance()->SetParamString(ICamera::PARAM_MESO_EXP_PATH, (wchar_t*)expFile.c_str());
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


DllExport FindCameras(long &cameraCount)
{
	return ThorLSMCam::getInstance()->FindCameras(cameraCount);
}

DllExport SelectCamera(const long camera)
{
	return ThorLSMCam::getInstance()->SelectCamera(camera);
}
DllExport TeardownCamera()
{
	return ThorLSMCam::getInstance()->TeardownCamera();
}

DllExport GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorLSMCam::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport SetParam(const long paramID, const double param)
{
	return ThorLSMCam::getInstance()->SetParam(paramID,param);
}

DllExport GetParam(const long paramID, double &param)
{
	return ThorLSMCam::getInstance()->GetParam(paramID,param);
}

DllExport PreflightAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->PreflightAcquisition(pDataBuffer);
}

DllExport SetupAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->SetupAcquisition(pDataBuffer);
}

DllExport StartAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->StartAcquisition(pDataBuffer);
}

DllExport StatusAcquisition(long &status)
{
	return ThorLSMCam::getInstance()->StatusAcquisition(status);
}

DllExport CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	return ThorLSMCam::getInstance()->CopyAcquisition(pDataBuffer, frameInfo);
}

DllExport PostflightAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->PostflightAcquisition(pDataBuffer);
}

DllExport GetLastErrorMsg(wchar_t * errMsg, long size)
{
	return ThorLSMCam::getInstance()->GetLastErrorMsg(errMsg,size);
}

DllExport SetStatusEvent(HANDLE handle)
{
	ThorLSMCam::getInstance()->SetStatusHandle(handle);
	return TRUE;
}

DllExport StatusAcquisitionEx(long &status, long &indexOfLastCompletedFrame)
{
	return ThorLSMCam::getInstance()->StatusAcquisitionEx(status,indexOfLastCompletedFrame);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorLSMCam::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorLSMCam::getInstance()->GetParamString(paramID,str,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorLSMCam::getInstance()->GetParamBuffer(paramID,buffer,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorLSMCam::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport InitCallBack(ImageCompleteCallback imageCompleteCallback)
{
	return ThorLSMCam::getInstance()->InitCallBack(imageCompleteCallback);
}

DllExport SetMapParam(const long paramID, double inputValue, const double param)
{
	return ThorLSMCam::getInstance()->SetMapParam(paramID, inputValue, param);
}

DllExport GetMapParam(const long paramID, double inputValue, double &param)
{
	return ThorLSMCam::getInstance()->GetMapParam(paramID, inputValue, param);
}

DllExport SaveConfigration()
{
	return ThorLSMCam::getInstance()->SaveConfigration();
}

DllExport GetDeviceConfigration(const long paramID, void** param)
{
	return ThorLSMCam::getInstance()->GetDeviceConfigration(paramID, param);
}

DllExport GetCaptureTime(Scan scans[], const uint8_t scanSize, long& timeMillisecond)
{
	return ThorLSMCam::getInstance()->GetCaptureTime(scans, scanSize, timeMillisecond);
}
