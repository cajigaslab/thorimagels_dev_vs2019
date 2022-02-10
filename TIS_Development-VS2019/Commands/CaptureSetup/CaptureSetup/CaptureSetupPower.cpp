#include "stdafx.h"
#include "math.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"



DllExportLiveImage GetPowerPosition(double &pos)
{	
	if(FALSE == CaptureSetup::getInstance()->GetSetupFlagState())
		return TRUE;

	return GetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR,IDevice::PARAM_POWER_POS_CURRENT,pos);
}

DllExportLiveImage GetPowerPosition2(double &pos)
{	
	if(FALSE == CaptureSetup::getInstance()->GetSetupFlagState())
		return TRUE;

	return GetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR2,IDevice::PARAM_POWER2_POS_CURRENT,pos);
}


int fileSize(wstring fileName){
    ifstream mySource;
    mySource.open(fileName, ios_base::binary);
    mySource.seekg(0,ios_base::end);
    long size = static_cast<long>(mySource.tellg());
    mySource.close();
    return size;
}

DllExportLiveImage SetPockelsMaskFile(char * file)
{
	long ret = FALSE;

	//=== Convert Filename to Wide String ===
	std::string fileNameString(file);
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &fileNameString[0], (int)fileNameString.size(), NULL, 0);
	std::wstring fileNameWideString(size_needed, 0 );
	MultiByteToWideChar(CP_UTF8, 0, &fileNameString[0], (int)fileNameString.size(), &fileNameWideString[0], size_needed);

	try
	{
		long pockelsMaskWidth = 0;
		if(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_POCKELS_MASK_WIDTH,pockelsMaskWidth))	
		{
			logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup SetPockelsMaskFile GetParam failed PARAM_LSM_PIXEL_X");
			return FALSE;
		}
		wstring ext(PathFindExtension(fileNameWideString.c_str()));
		long width = 0,height = 0,colorChannels = 0;
		char * pBuffer;
		char * pMask;
		const long BYTES_PER_PIXEL = 2;
		
		if (ext == L".raw" || ext == L".Raw")
		{			
			if(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_X,width))	
			{
				logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup SetPockelsMaskFile GetParam failed PARAM_LSM_PIXEL_X");
				return FALSE;
			}

			if(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_Y,height))	
			{
				logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup SetPockelsMaskFile GetParam failed PARAM_LSM_PIXEL_Y");
				return FALSE;
			}

			if (fileSize(fileNameWideString) != width*height*BYTES_PER_PIXEL)
			{
				return FALSE;
			}
			pBuffer = new char[width*height*BYTES_PER_PIXEL];
			pMask = new char[pockelsMaskWidth*height*BYTES_PER_PIXEL];
			std::ifstream inFile (fileNameWideString, ios::in|ios::binary);
			// Open the raw image file
			if(inFile.is_open())
			{
				inFile.seekg(0, ios::beg);
				inFile.read(pBuffer, width*height*BYTES_PER_PIXEL);
				inFile.close();
			}
		}
		else if (ext == L".tif" || ext == L".Tif")
		{
			if(FALSE == ReadImageInfo((wchar_t*)fileNameWideString.c_str(), width, height, colorChannels))
			{
				return FALSE;
			}
			pBuffer = new char[width*height*colorChannels*BYTES_PER_PIXEL];
			pMask = new char[pockelsMaskWidth*height*BYTES_PER_PIXEL];
			//Extract the buffer fromt the specified file and assign the buffer
			//to the camera dll
			ReadImage((char*)fileNameWideString.c_str(),(char*&)pBuffer);
		}
		else
		{
			return FALSE;
		}

		ResizeImage(pBuffer, width, height, pMask, pockelsMaskWidth, height);

		ret = SetCameraParamBuffer(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_POCKELS_MASK,pMask,pockelsMaskWidth*height*BYTES_PER_PIXEL);

		delete[] pBuffer;
		delete[] pMask;
	}
	catch(...)
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup unable to create/assign pockels mask");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
	}

	return ret;
}

DllExportLiveImage GetPockelsPlot(char * &data, long index)
{
	long ret = FALSE;

	const int POCKELS_VOLTAGE_STEPS = 100;

	if(NULL == data)
	{
		return ret;
	}

	switch(index)
	{
	case 0:ret = GetCameraParamBuffer(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_0,data,sizeof(double) * POCKELS_VOLTAGE_STEPS);break;
	case 1:ret = GetCameraParamBuffer(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_1,data,sizeof(double) * POCKELS_VOLTAGE_STEPS);break;
	case 2:ret = GetCameraParamBuffer(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_2,data,sizeof(double) * POCKELS_VOLTAGE_STEPS);break;
	case 3:ret = GetCameraParamBuffer(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_3,data,sizeof(double) * POCKELS_VOLTAGE_STEPS);break;
	}

	return ret;
}


DllExportLiveImage IsShutterAvailable()
{	
	return TRUE;
}

UINT SafetyInterlockStatusCheck()
{
	long safetyInterLockState = -1;
	while (TRUE == _shutterOpened)
	{
		if (TRUE == GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::Params::PARAM_SHUTTER_SAFETY_INTERLOCK_STATE, safetyInterLockState))
		{
			if (FALSE == safetyInterLockState)
			{
				wstring messageWstring = L"Safety Interlock is engaged or not installed. Please check if the trinoc is in eyepiece mode. \nIf error persists please contact techsupport@thorlabs.com.";
				MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Primary path shutter closed.", MB_OK);
				return FALSE;
			}
		}
		else
		{
			StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup SafetyInterlockStatusCheck: unable get param PARAM_SHUTTER_SAFETY_INTERLOCK_STATE");
			logDll->TLTraceEvent(ERROR_EVENT, 1, message);
			return FALSE;
		}
		Sleep(1000);
	}
	return TRUE;
}

void InitiateSafetyInterlockStatusCheck(long pos)
{
	long scopeType = ScopeType::UPRIGHT;
	long safetyInterlockCheckEnabled = FALSE;
	long shutterAvailable = FALSE;
	long ret = FALSE;

	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML);
	ret = pHardware->GetInvertedSettings(safetyInterlockCheckEnabled);

	if ((TRUE == GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::Params::PARAM_SCOPE_TYPE, scopeType) && ScopeType::INVERTED == scopeType) &&
		(TRUE == ret && TRUE == safetyInterlockCheckEnabled) &&
		(TRUE == GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::Params::PARAM_SHUTTER_AVAILABLE, shutterAvailable) && TRUE == shutterAvailable))
	{
		if (SHUTTER_OPEN == pos)
		{
			_shutterOpened = TRUE;
			SAFE_DELETE_HANDLE(_hSafetyInterLockCheckThread);
			_hSafetyInterLockCheckThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SafetyInterlockStatusCheck, NULL, 0, &_dwSafetyInterLockCheckThreadId);
			SetThreadPriority(_hSafetyInterLockCheckThread, THREAD_PRIORITY_LOWEST);
		}
		else
		{
			_shutterOpened = FALSE;
		}
	}
}

DllExportLiveImage SetShutterPosition(long pos)
{
	InitiateSafetyInterlockStatusCheck(pos);
	return SetDeviceParamDouble(SelectedHardware::SELECTED_SHUTTER1,IDevice::PARAM_SHUTTER_POS,pos,FALSE);
}

DllExportLiveImage SetBFLampPosition(long pos)
{
	return SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LEDS_ENABLE_DISABLE,pos,FALSE);
}

DllExportLiveImage SetLaser1Emission(long pos) 
{
	return SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_EMISSION, pos, FALSE);
}

DllExportLiveImage SetLaser2Emission(long pos)
{
	return SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_EMISSION, pos, FALSE);
}

DllExportLiveImage SetLaser3Emission(long pos)
{
	return SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_EMISSION, pos, FALSE);
}

DllExportLiveImage SetLaser4Emission(long pos)
{
	return SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_EMISSION, pos, FALSE);
}

DllExportLiveImage GetLaserAnalog(long &pos) 
{
	return GetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_ANALOG_MODE, pos);
}

DllExportLiveImage GetLaser1Enable(long& pos) 
{
	return GetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_ENABLE, pos);
}

DllExportLiveImage GetLaser2Enable(long& pos)
{
	return GetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_ENABLE, pos);
}

DllExportLiveImage GetLaser3Enable(long& pos)
{
	return GetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_ENABLE, pos);
}

DllExportLiveImage GetLaser4Enable(long& pos)
{
	return GetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_ENABLE, pos);
}

DllExportLiveImage PowerCalibrateZero()
{		
	long ret = FALSE;

	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR,IDevice::PARAM_POWER_ZERO,1,FALSE);		
	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR,IDevice::PARAM_POWER_ZERO,0,FALSE);

	return ret;

}

DllExportLiveImage PowerCalibrate2Zero()
{		
	long ret = FALSE;

	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR2,IDevice::PARAM_POWER2_ZERO,1,FALSE);		
	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR2,IDevice::PARAM_POWER2_ZERO,0,FALSE);

	return ret;

}

