#include "stdafx.h"
#include "math.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"

DllExportLiveImage GetImageDimensions(long &width, long &height)
{	
	CHECK_PACTIVEIMAGEROUTINE(GetImageDimensions(width, height));
}

DllExportLiveImage GetNumberOfCameras(long &numCameras)
{
	if(NULL == GetCamera(SelectedHardware::SELECTED_CAMERA1))
	{
		numCameras = 0;
	}
	else
	{
		numCameras = 1;
	}
	return TRUE;
}

DllExportLiveImage GetCameraHeight(long &height)
{
	long ret = FALSE;
	long pMin = -1, pMax=-1, dummy=-1;

	if(GetCameraParamRangeLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_TOP,pMin,dummy,dummy))
	{
		if(GetCameraParamRangeLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_BOTTOM,dummy,pMax,dummy))
		{
			height = pMax - pMin;

			ret = TRUE;
		}
	}
	return ret;
}

DllExportLiveImage GetCameraWidth(long &width)
{
	long ret = FALSE;
	long pMin = -1, pMax=-1, dummy=-1;

	if(GetCameraParamRangeLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_LEFT,pMin,dummy, dummy))
	{
		if(GetCameraParamRangeLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_RIGHT,dummy,pMax,dummy))
		{
			width = pMax - pMin;
			ret = TRUE;
		}
	}

	return TRUE;
}

DllExportLiveImage CenterLSMScanners(long selectedCamera)
{
	if(NULL == GetCamera(selectedCamera))
	{
		return FALSE;
	}

	long cameraType = 0;
	if (FALSE == GetCameraParamLong(selectedCamera,ICamera::PARAM_CAMERA_TYPE, cameraType))
	{
		return FALSE;
	}

	switch(cameraType)
	{
	case ICamera::LSM:
		{
			long currentScanMode = 0;
			//first get the current scan mode
			GetCameraParamLong(selectedCamera,ICamera::PARAM_LSM_SCANMODE, currentScanMode);

			//change the scanMode to SCAN_MODE_CENTER
			if(SetCameraParamDouble(selectedCamera,ICamera::PARAM_LSM_SCANMODE, ScanMode::CENTER) != TRUE)
			{
				StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Execute set scanmode alignment (%d) failed", ScanMode::CENTER);
				logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
				return FALSE;
			}

			//centering the scanner only requires a push of the scan mode parameter to the lsm. No image capture is required
			switch (selectedCamera)
			{
			case SelectedHardware::SELECTED_CAMERA1:
				if(FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->PreflightAcquisition(pMemoryBuffer))
				{
					StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage PreflightAcquisition failed");
					logDll->TLTraceEvent(ERROR_EVENT,1,message);
					return FALSE;
				}
				break;
			case SelectedHardware::SELECTED_BLEACHINGSCANNER:
				if(FALSE == GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER)->PreflightAcquisition(pMemoryBuffer))
				{
					StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage PreflightAcquisition failed");
					logDll->TLTraceEvent(ERROR_EVENT,1,message);
					return FALSE;
				}
				break;
			}

			//open shutter
			SetShutterPosition(SHUTTER_OPEN);
			//enable LEDs
			SetBFLampPosition(ENABLE_LEDS);
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

DllExportLiveImage SetRSInitMode(long mode)
{
	long ret = FALSE;

	SetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_SCANNER_INIT_MODE,mode);

	if(1 == mode)
	{
		//turn the scanner on immediately
		SetPMTScannerEnable(1);
		//wait for the scanner to stabilize
		Sleep(200);
		//change after the scanner is started because the control unit will ignore any start or stop scanner commands
		//after PARAM_SCANNER_INIT_MODE is set to 1
		ret = SetDeviceParamDouble(SelectedHardware::SELECTED_CONTROLUNIT,IDevice::PARAM_SCANNER_INIT_MODE,mode,FALSE);
	}
	else
	{
		//change before the scanner is stopepd because the control unit will ignore any stop scanner commands
		//before PARAM_SCANNER_INIT_MODE is set to 0
		ret = SetDeviceParamDouble(SelectedHardware::SELECTED_CONTROLUNIT,IDevice::PARAM_SCANNER_INIT_MODE,mode,FALSE);
		//stop the scanner immediately
		SetPMTScannerEnable(0);
	}	

	return ret;
}

DllExportLiveImage GetFrameRate(double &rate)
{
	rate = CaptureSetup::getInstance()->GetFrameRate();

	return TRUE;
}

DllExportLiveImage SendToCameraConsole(LPTSTR lpStr)
{
	ICamera * pCamera;

	pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);

	long ret = FALSE;

	if(NULL == pCamera)
	{
		return ret;
	}

	long pMin = -1, pMax=-1, cameraType = ICamera::CameraType::LAST_CAMERA_TYPE;
	if((GetCameraParamRangeLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_TYPE,pMin,pMax,cameraType)) && ICamera::CCD == cameraType)
	{
		pCamera->SetParamString(ICamera::PARAM_CONSOLE_WRITE,lpStr);
	}
	return TRUE;
}

DllExportLiveImage GetFromCameraConsole(LPTSTR lpStr, long size)
{
	ICamera * pCamera;
	pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);
	long ret = FALSE;

	if(NULL == pCamera)
	{
		return ret;
	}

	long pMin = -1, pMax=-1, cameraType = ICamera::CameraType::LAST_CAMERA_TYPE;
	if((GetCameraParamRangeLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_TYPE,pMin,pMax,cameraType)) && ICamera::CCD == cameraType)
	{
		pCamera->GetParamString(ICamera::PARAM_CONSOLE_READ,lpStr,size);
	}
	return TRUE;
}

DllExportLiveImage SetPMTScannerEnable(long enable)
{	
	long camType = ICamera::CameraType::LAST_CAMERA_TYPE;
	camType = (GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_TYPE,camType)) ? camType : ICamera::CameraType::LAST_CAMERA_TYPE;

	long lsmType = ICamera::LSMType::LSMTYPE_LAST;
	lsmType = (GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_TYPE,lsmType)) ? lsmType : ICamera::LSMType::LSMTYPE_LAST;

	if((ICamera::CameraType::LSM == camType) && (NULL != GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)))
	{
		double rsInitMode = FALSE;
		switch ((ICamera::LSMType)lsmType)
		{
		case ICamera::LSMType::GALVO_RESONANCE:
			//scanner only needs to be started if the lsm is a resonance/galvo
			//Set the scanner state to the set up control unit, this might defer from the current modality when in the process of switching
			GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)->GetParam(IDevice::PARAM_SCANNER_INIT_MODE, rsInitMode);

			//only turn off the scanner when it is not on the always on mode
			if (FALSE == ((int)rsInitMode))
			{
				GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)->SetParam(IDevice::PARAM_SCANNER_ENABLE, enable);
				GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)->PreflightPosition();
				GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)->SetupPosition();
				GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)->StartPosition();
				GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)->PostflightPosition();
			}
			break;
		case ICamera::LSMType::RESONANCE_GALVO_GALVO:
			//scanner needs to be analog controlled in RGG mode
			GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)->SetParam(IDevice::PARAM_SCANNER_ENABLE_ANALOG, enable);
			GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)->PreflightPosition();
			GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)->SetupPosition();
			GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)->StartPosition();
			GetDevice(SelectedHardware::SELECTED_CONTROLUNIT)->PostflightPosition();
			break;
		default:
			break;
		}
	}
	return TRUE;
}

DllExportLiveImage SetFieldSize(long fieldSize)
{
	long ret = FALSE;
	double paramMin=0.0;
	double paramMax=0.0;

	SetDeviceParamLong(SelectedHardware::SELECTED_CONTROLUNIT,IDevice::PARAM_SCANNER_ZOOM_POS,fieldSize,FALSE);

	//the field size must still be set for the lsm so that the galvo angle is synchronized with
	//the resonance scanner angle
	if(SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_FIELD_SIZE,fieldSize) != TRUE)
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Execute set lsm field size (%d) failed",fieldSize);
		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		return FALSE;
	}

	ret = TRUE;

	return ret;
}

DllExportLiveImage GetFieldSize(long &fieldSize)
{
	long ret = FALSE;
	double paramMin=0.0;
	double paramMax=0.0;

	//initialize the field size to a
	//non zero value in the event a field size
	//cannot be retrieved
	fieldSize = 120;

	ret = GetDeviceParamLong(SelectedHardware::SELECTED_CONTROLUNIT,IDevice::PARAM_SCANNER_ZOOM_POS,fieldSize);

	//if device did not return fieldsize zoom
	//use the legacy parameter in the camera

	//The zoom is not available through the ECU/PMT. Fallback to setting the field size
	//with the legacy parameter in the camera
	if(NULL == GetCamera(SelectedHardware::SELECTED_CAMERA1))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup SetField Size no cameras are loaded");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
		return FALSE;
	}

	ret = GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_FIELD_SIZE,fieldSize);

	return ret;
}

DllExportLiveImage GetFieldSizeRange(long &fieldSizeMin, long &fieldSizeMax)
{
	long ret = FALSE;

	long val;
	long fieldSizeDefault;

	//if the device can change the zoom use it instead of the scanner
	if(GetDeviceParamLong(SelectedHardware::SELECTED_CONTROLUNIT,IDevice::PARAM_SCANNER_ZOOM_POS,val))
	{
		ret = GetDeviceParamRangeLong(SelectedHardware::SELECTED_CONTROLUNIT,IDevice::PARAM_SCANNER_ZOOM_POS,fieldSizeMin,fieldSizeMax,fieldSizeDefault);
	}
	else
	{
		ret = GetCameraParamRangeLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_FIELD_SIZE,fieldSizeMin,fieldSizeMax,fieldSizeDefault);
	}
	return ret;
}

DllExportLiveImage SetTwoWayAlignmentCoarse(long fieldSize, long alignment)
{
	long ret = FALSE;
	long val = -1;

	long minFS,maxFS;

	GetFieldSizeRange(minFS,maxFS);

	long zone = maxFS - fieldSize;

	long zoneECU = IDevice::PARAM_ECU_TWO_WAY_ZONE_1 + zone;

	// Set the scan alignment through ThorECU if supported
	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_CONTROLUNIT, zoneECU, alignment, FALSE);

	//using legacy parameter through ThorConfocal in case of ECU Gen 1
	if(FALSE == ret)
	{
		long zoneLSM = ICamera::PARAM_LSM_TWO_WAY_ZONE_1 + zone;
		ret = SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,zoneLSM, alignment);
	}

	return ret;
}

DllExportLiveImage SetTwoWayAlignmentFine(long fieldSize, long alignment)
{
	long ret = FALSE;
	long val = -1;

	long minFS,maxFS;

	GetFieldSizeRange(minFS,maxFS);

	long zone = maxFS - fieldSize;

	long zoneECU = IDevice::PARAM_ECU_TWO_WAY_ZONE_FINE_1 + zone;

	// Set the scan alignment through ThorECU if supported
	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_CONTROLUNIT, zoneECU, alignment, FALSE);

	//using legacy parameter through ThorConfocal in case of ECU Gen 1
	if(FALSE == ret)
	{
		long zoneLSM = ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_1 + zone;
		ret = SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,zoneLSM, alignment);
	}

	return ret;
}

DllExportLiveImage GetPixelXRange(long &pixelXMin, long &pixelXMax)
{	
	long pixelXDefault;
	long ret = FALSE;

	if(GetCameraParamRangeLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_X,pixelXMin,pixelXMax,pixelXDefault))
	{
		long param;
		GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_SCANMODE,param);
		if(param  == 0)
		{
			pixelXMax = static_cast<long>(pixelXMax/2);
		}

		ret = TRUE;
	}			
	return ret;
}

DllExportLiveImage GetPixelYRange(long &pixelYMin, long &pixelYMax)
{	
	long ret = FALSE;
	long pixelYDefault;

	if(GetCameraParamRangeLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_Y,pixelYMin,pixelYMax,pixelYDefault))
	{
		long param;
		GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_SCANMODE,param);
		if(param  == 0)
		{
			pixelYMax = static_cast<long>(pixelYMax/2);
		}
		ret = TRUE;
	}			
	return ret;
}

/// TODO:
// These two functions should be able to return FALSE, when the parameter is
// not supported because of older firmware
DllExportLiveImage GetTwoWayAlignmentCoarse(long fieldSize, long &alignment)
{
	long ret = FALSE;

	long minFS,maxFS;

	GetFieldSizeRange(minFS,maxFS);

	long zone = maxFS - fieldSize;

	long zoneECU = IDevice::PARAM_ECU_TWO_WAY_ZONE_1 + zone;

	// Get the scan alignment through ThorECU if supported
	ret = GetDeviceParamLong(SelectedHardware::SELECTED_CONTROLUNIT, zoneECU, alignment);

	//using legacy way through ThorConfocal in case of ECU Gen 1
	if(FALSE == ret)
	{
		long zoneLSM = ICamera::PARAM_LSM_TWO_WAY_ZONE_1 + zone;

		ret = GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,zoneLSM, alignment);
	}
	return ret;
}

DllExportLiveImage GetTwoWayAlignmentFine(long fieldSize, long &alignment)
{
	long ret = FALSE;

	long minFS,maxFS;

	GetFieldSizeRange(minFS,maxFS);

	long zone = maxFS - fieldSize;

	long zoneECU = IDevice::PARAM_ECU_TWO_WAY_ZONE_FINE_1 + zone;

	// Get the scan alignment through ThorECU if supported
	ret = GetDeviceParamLong(SelectedHardware::SELECTED_CONTROLUNIT, zoneECU, alignment);

	//using legacy way through ThorConfocal in case of ECU Gen 1
	if(FALSE == ret)
	{
		long zoneLSM = ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_1 + zone;
		ret = GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,zoneLSM, alignment);
	}
	return ret;
}


DllExportLiveImage GetPMTSafetyStatus()
{
	if(FALSE == CaptureSetup::getInstance()->GetSetupFlagState())
		return TRUE;

	long val=0;
	if(GetDeviceParamLong(SelectedHardware::SELECTED_PMT1,IDevice::PARAM_PMT1_SAFETY,val))
	{
		if(FALSE == static_cast<long>(val))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup GetPMTSafetyStatus PMT1 has tripped");
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

			return FALSE;
		}
	}

	val=0;
	if(GetDeviceParamLong(SelectedHardware::SELECTED_PMT2,IDevice::PARAM_PMT2_SAFETY,val))
	{
		if(FALSE == static_cast<long>(val))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup GetPMTSafetyStatus PMT2 has tripped");
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
			return FALSE;
		}
	}

	val=0;
	if(GetDeviceParamLong(SelectedHardware::SELECTED_PMT3,IDevice::PARAM_PMT3_SAFETY,val))
	{
		if(FALSE == static_cast<long>(val))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup GetPMTSafetyStatus PMT3 has tripped");
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
			return FALSE;
		}
	}

	val=0;
	if(GetDeviceParamLong(SelectedHardware::SELECTED_PMT4,IDevice::PARAM_PMT4_SAFETY,val))
	{
		if(FALSE == static_cast<long>(val))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup GetPMTSafetyStatus PMT4 has tripped");
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
			return FALSE;
		}
	}

	//Safety has not tripped. return TRUE
	return TRUE;
}
