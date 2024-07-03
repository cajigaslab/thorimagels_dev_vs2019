#include "stdafx.h"
#include "CaptureSetup.h"
#include "ImageRoutineSciCam.h"

ImageRoutineSciCam::ImageRoutineSciCam()
{
	_channelEnable = 0;
	_captureActive = 0;
}

ImageRoutineSciCam::~ImageRoutineSciCam()
{
}

BOOL ImageRoutineSciCam::_enableCopy = FALSE;

long ImageRoutineSciCam::InitCallbacks(imageCompleteCallback ic, completeCallback cc)
{
	myFunctionPointer = ic;

	myFuncPtrZStack = cc;


	if ((myFunctionPointer != NULL) &&
		(myFuncPtrZStack != NULL))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"CaptureSetup InitCallBack");
	}

	return TRUE;
}

long ImageRoutineSciCam::InitParameters()
{
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	if (NULL == GetCamera(SelectedHardware::SELECTED_CAMERA1))
	{
		return FALSE;
	}

	if (GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_CAPTURE_REGION_RIGHT, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
	{
		imageInfo.imageWidth = static_cast<long>(paramMax);
	}

	if (GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_CAPTURE_REGION_BOTTOM, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
	{
		imageInfo.imageHeight = static_cast<long>(paramMax);
	}

	StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup InitParameters maxCameraWidth %d maxCameraHeight %d", imageInfo.imageWidth, imageInfo.imageHeight);
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);

	return TRUE;
}
long ImageRoutineSciCam::Snapshot(SnapshotSaveParams* sParam)
{
	if (GetCaptureActive())
	{
		return FALSE;
	}

	if (FALSE == SetupCaptureBuffers())
		return FALSE;

	StringCbPrintfW(message, MSG_SIZE, L"About to SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_TRIGGER_MODE,ICamera::SW_MULTI_FRAME)");
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);

	if (SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_TRIGGER_MODE, ICamera::SW_MULTI_FRAME) != TRUE)
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup Snapshot SetParam PARAM_TRIGGER_MODE failed");
		logDll->TLTraceEvent(ERROR_EVENT, 1, message);
	}

	//snapshot shared the parameters with the live mode
	SAFE_DELETE_HANDLE(hCaptureActive);

	hCaptureActive = CreateEvent(0, FALSE, FALSE, 0);

	SAFE_DELETE_HANDLE(hLiveThread);

	stopCapture = FALSE;
	hLiveThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SnapshotThreadProcSciCam, (LPVOID)sParam, 0, &dwLiveThreadId);
	SetCaptureActive(TRUE);

	return TRUE;
}
long ImageRoutineSciCam::EnableCopyToExternalBuffer()
{
	_enableCopy = TRUE;
	return TRUE;
}

long ImageRoutineSciCam::CaptureZStack(double zStartPos, double zStopPos, double zstageStepSize, long zstageSteps)
{
	if (myFunctionPointer != NULL)
	{
		if (GetCaptureActive() == TRUE)
		{
			return TRUE;
		}

		if (FALSE == SetupCaptureBuffers())
			return FALSE;

		hCaptureActive = CreateEvent(0, FALSE, FALSE, 0);

		struct CaptureSetupZCaptureParams* zParams = static_cast<CaptureSetupZCaptureParams*>(malloc(sizeof(struct CaptureSetupZCaptureParams)));
		zParams->start = zStartPos;
		if (zStartPos > zStopPos)
		{
			zParams->stepSize = (-1) * zstageStepSize;
		}
		else
		{
			zParams->stepSize = zstageStepSize;
		}
		zParams->numOfSteps = zstageSteps;

		stopCapture = FALSE;
		hZStackCaptureThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ZStackCaptureThreadProcSciCam, (LPVOID)zParams, 0, &dwZStackCaptureThreadId);
		SetCaptureActive(TRUE);
		SetZStackActive(TRUE);
	}

	return TRUE;

}
long ImageRoutineSciCam::StopZStackCapture()
{
	//return if the capture is not active
	if (FALSE == GetCaptureActive())
	{
		return TRUE;
	}

	if (myFunctionPointer != NULL)
	{
		stopCapture = TRUE;
	}

	if (WaitForSingleObject(hCaptureActive, Constants::EVENT_WAIT_TIME) != WAIT_OBJECT_0)
	{
		return FALSE;
	}

	SAFE_DELETE_HANDLE(hZStackCaptureThread);

	SAFE_DELETE_HANDLE(hCaptureActive);

	return TRUE;
}

long ImageRoutineSciCam::CaptureSequentialPreview()
{
	if (myFunctionPointer != NULL)
	{
		if (GetCaptureActive() == TRUE)
		{
			return TRUE;
		}

		if (FALSE == SetupCaptureBuffers())
			return FALSE;

		SAFE_DELETE_HANDLE(_hSequentialPreviewCaptureThread);

		SAFE_DELETE_HANDLE(hCaptureActive);

		hCaptureActive = CreateEvent(0, FALSE, FALSE, 0);

		stopCapture = FALSE;
		SetCaptureActive(TRUE);

		_hSequentialPreviewCaptureThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SequentialPreviewCaptureThreadProcSciCam, (LPVOID)NULL, 0, &_dwSequentialPreviewCaptureThreadId);
	}
	return TRUE;
}

long ImageRoutineSciCam::StopSequentialPreview()
{
	//return if the capture is not active
	if (FALSE == GetCaptureActive())
	{
		return TRUE;
	}

	if (myFunctionPointer != NULL)
	{
		stopCapture = TRUE;
	}
	CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));
	if (WaitForSingleObject(hCaptureActive, Constants::EVENT_WAIT_TIME) != WAIT_OBJECT_0)
	{
		return FALSE;
	}

	SAFE_DELETE_HANDLE(_hSequentialPreviewCaptureThread);

	SAFE_DELETE_HANDLE(hCaptureActive);

	return TRUE;
}

long ImageRoutineSciCam::SetDisplayChannels(long channelEnable)
{
	_channelEnable = channelEnable;
	return TRUE;
}
long ImageRoutineSciCam::GetDisplayChannels()
{
	return _channelEnable;
}
long ImageRoutineSciCam::SetupCaptureBuffers()
{
	InitParameters();

	return SetupBuffers(4);
}
long ImageRoutineSciCam::StartLiveCapture()
{
	if (myFunctionPointer != NULL)
	{
		if (GetCaptureActive() == TRUE)
		{
			return TRUE;
		}

		if (FALSE == SetupCaptureBuffers())
			return FALSE;

		SAFE_DELETE_HANDLE(hCaptureActive);

		hCaptureActive = CreateEvent(0, FALSE, FALSE, 0);

		SAFE_DELETE_HANDLE(hLiveThread);

		if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE, 0))
		{
			StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage SetParam PARAM_LSM_FORCE_SETTINGS_UPDATE failed");
			logDll->TLTraceEvent(WARNING_EVENT, 1, message);
		}

		stopCapture = FALSE;
		hLiveThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LiveThreadProc, (LPVOID)NULL, 0, &dwLiveThreadId);
		SetCaptureActive(TRUE);
	}

	return TRUE;
}
long ImageRoutineSciCam::StopLiveCapture()
{
	//disable LEDs
	SetBFLampPosition(DISABLE_LEDS);

	//disable Laser Emission
	SetLaser1Emission(DISABLE_EMISSION);
	SetLaser2Emission(DISABLE_EMISSION);
	SetLaser3Emission(DISABLE_EMISSION);
	SetLaser4Emission(DISABLE_EMISSION);

	//return if the capture is not active
	if (FALSE == GetCaptureActive())
	{
		return TRUE;
	}

	if (myFunctionPointer != NULL)
	{
		stopCapture = TRUE;
	}

	if (WaitForSingleObject(hCaptureActive, Constants::EVENT_WAIT_TIME) != WAIT_OBJECT_0)
	{
		return FALSE;
	}

	SAFE_DELETE_HANDLE(hLiveThread);

	SAFE_DELETE_HANDLE(hCaptureActive);

	return TRUE;
}

long ImageRoutineSciCam::SetCaptureActive(long active)
{
	_captureActive = active;
	return TRUE;
}

long ImageRoutineSciCam::GetCaptureActive()
{
	return _captureActive;
}

long ImageRoutineSciCam::SetZStackActive(long zStackActive)
{
	_zStackActive = zStackActive;
	return TRUE;
}

long ImageRoutineSciCam::GetZStackActive()
{
	return _zStackActive;
}

long ImageRoutineSciCam::CopyAcquisition(long isFullFrame)
{
	long ret = 0;
	std::wstring cameraName;

	Lock lock(CaptureSetup::getInstance()->critSect);

	imageInfo.numberOfPlanes = 1; // assume 1 and let the camera say otherwise if it supports multiplane
	imageInfo.pixelAspectRatioYScale = 1; //assume 1

	ret = GetCamera(SelectedHardware::SELECTED_CAMERA1)->CopyAcquisition(pChan[0], &imageInfo);

	logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"CaptureSetup CopyAcquisition");

	if (_enableCopy)
	{
		//imageInfo.channels = 1; -> keep the value set via ICamera::CopyAcquisition
		imageInfo.fullFrame = isFullFrame;
		_enableCopy = FALSE;
		(*myFunctionPointer)(pMemoryBuffer, imageInfo);

		long width = 0;
		long height = 0;

		if ((FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_IMAGE_WIDTH, width)) ||
			(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_IMAGE_HEIGHT, height))
			)
		{
			logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"CaptureSetup CopyAcquisition GetParam failed PARAM_CAMERA_IMAGE_WIDTH || PARAM_CAMERA_IMAGE_HEIGHT");
			return FALSE;
		}

		imageInfo.imageHeight = static_cast<long>(height);
		imageInfo.imageWidth = static_cast<long>(width);

		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"CaptureSetup CopyAcquisition callback complete");
	}
	return TRUE;
}

long ImageRoutineSciCam::GetImageDimensions(long& width, long& height)
{
	if (NULL == GetCamera(SelectedHardware::SELECTED_CAMERA1))
	{
		return FALSE;
	}

	double tempWidth = 0.0;
	double tempHeight = 0.0;

	if (FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_IMAGE_WIDTH, tempWidth))
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"CaptureSetup GetImageDimensions width failed");
	}

	if (FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_IMAGE_HEIGHT, tempHeight))
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"CaptureSetup GetImageDimensions height failed");
	}

	width = static_cast<long>(tempWidth);
	height = static_cast<long>(tempHeight);

	return TRUE;
}
//snapshotFlag definitions:
//0: snapshot, preflight and postflight 
//1: capturing first image of stack, preflight only
//2: capturing last image of stack, postflight only
//3: capturing non-start, non-end images of stack, no preflight or postflight
long CaptureSingleImageWithAverageSciCam(char* buffer, long avgFrames, long snapshotFlag)
{

	if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_AVERAGENUM, static_cast<double>(1)))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage SetParam PARAM_MULTI_FRAME_COUNT failed");
		logDll->TLTraceEvent(WARNING_EVENT, 1, message);
		return FALSE;
	}

	if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_MULTI_FRAME_COUNT, static_cast<double>(avgFrames)))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage SetParam PARAM_MULTI_FRAME_COUNT failed");
		logDll->TLTraceEvent(WARNING_EVENT, 1, message);
		return FALSE;
	}

	if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE, 1))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage SetParam PARAM_LSM_FORCE_SETTINGS_UPDATE failed");
		logDll->TLTraceEvent(WARNING_EVENT, 1, message);
	}

	if ((0 == snapshotFlag) || (1 == snapshotFlag))
	{
		if (FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->PreflightAcquisition(buffer))
		{
			StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage PreflightAcquisition failed");
			logDll->TLTraceEvent(ERROR_EVENT, 1, message);
			return FALSE;
		}
	}
	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML);

	wstring streamPath;
	double rate; //this variable is only used in runsample 
	long alwaysSaveImagesOnStop; //this variable is only used in runsample 
	pHardware->GetStreaming(streamPath, rate, alwaysSaveImagesOnStop);

	ImageManager::getInstance()->SetMemMapPath(streamPath.c_str());

	Dimensions dim;

	dim.dType = INT_16BIT;
	dim.c = imageInfo.channels;
	dim.m = 1;
	dim.mType = CONTIGUOUS_CHANNEL_MEM_MAP;
	dim.t = avgFrames;
	dim.x = imageInfo.imageWidth;
	dim.y = imageInfo.imageHeight;
	dim.z = 1;

	imageInfo.numberOfPlanes = 1; // assume 1 and let the camera say otherwise if it supports multiplane
	imageInfo.pixelAspectRatioYScale = 1;

	long imageID;

	if (FALSE == ImageManager::getInstance()->CreateImage(imageID, dim))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage CreateImage failed");
		logDll->TLTraceEvent(ERROR_EVENT, 1, message);
		return FALSE;
	}

	if (FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->SetupAcquisition(buffer))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage Setup Acquisition failed");
		logDll->TLTraceEvent(ERROR_EVENT, 1, message);
		return FALSE;
	}

	if (FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->StartAcquisition(buffer))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage Start Acquisition failed");
		logDll->TLTraceEvent(ERROR_EVENT, 1, message);
		return FALSE;
	}

	char* pAvgBuffer = ImageManager::getInstance()->GetImagePtr(imageID, 0, 0, 0, 0);

	for (long i = 0; i < avgFrames; i++)
	{
		long status = ICamera::STATUS_BUSY;

		while (status == ICamera::STATUS_BUSY)
		{
			if (FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->StatusAcquisition(status))
			{
				break;
			}

			if (TRUE == stopCapture)
			{
				break;
			}
		}

		if (TRUE == stopCapture)
		{
			break;
		}


		if (FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->CopyAcquisition(pAvgBuffer, &imageInfo))
		{
			StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage CopyAcquisition failed");
			logDll->TLTraceEvent(ERROR_EVENT, 1, message);
			ImageManager::getInstance()->UnlockImagePtr(imageID, 0, 0, 0, i);
			ImageManager::getInstance()->DestroyImage(imageID);
			return FALSE;
		}

		//synchrnously start the unlock process for the frame
		ImageManager::getInstance()->UnlockImagePtr(imageID, 0, 0, 0, i);

		//go ahead and unlock the next frame before the status call
		//this will allow the GetImagePtr delay for memory mapping to 
		//overlap with processing time of the next frame
		if (i < (avgFrames - 1))
		{
			pAvgBuffer = ImageManager::getInstance()->GetImagePtr(imageID, 0, 0, 0, i + 1);
		}

	}

	//NOTE: imageInfo channel, width, and height used to be re-read here, but we now rely on CopyAcquisition to correctly retrieve them

	imageInfo.fullFrame = TRUE;

	unsigned long* pSumBuffer = new unsigned long[imageInfo.imageWidth * imageInfo.imageHeight * imageInfo.channels];

	memset(pSumBuffer, 0, sizeof(unsigned long) * imageInfo.imageWidth * imageInfo.imageHeight * imageInfo.channels);

	for (long i = 0; i < avgFrames; i++)
	{
		unsigned long* pSum = pSumBuffer;

		unsigned short* pAvgBuffer = (unsigned short*)ImageManager::getInstance()->GetImagePtr(imageID, 0, 0, 0, i);

		for (long k = 0; k < imageInfo.imageWidth * imageInfo.imageHeight * imageInfo.channels; k++)
		{
			*pSum += *pAvgBuffer;
			pSum++;
			pAvgBuffer++;
		}

		ImageManager::getInstance()->UnlockImagePtr(imageID, 0, 0, 0, i);

		// return partial of average if stopped:
		if (TRUE == stopCapture)
		{
			break;
		}

	}

	ImageManager::getInstance()->DestroyImage(imageID);

	unsigned short* pBuf = (unsigned short*)buffer;
	unsigned long* pSum = pSumBuffer;

	for (long k = 0; k < imageInfo.imageWidth * imageInfo.imageHeight * imageInfo.channels; k++)
	{
		*pBuf = static_cast<unsigned short>((*pSum) / (double)avgFrames);
		pBuf++;
		pSum++;
	}

	delete[] pSumBuffer;

	(*myFunctionPointer)(buffer, imageInfo);
	////// temporarily commented out since auto roi identification has not been released.
	/////*	StatsManager::getInstance()->ComputeContours( (unsigned short*)buffer, 
	////		static_cast<long>(sumWidth),
	////		static_cast<long>(sumHeight),
	////		CaptureSetup::getInstance()->_channelEnable,
	////		_autoDisplayChannel);*/ 
	long r = FALSE;
	CHECK_INEXPRESSION_PACTIVEIMAGEROUTINE(GetDisplayChannels(), r);
	StatsManager::getInstance()->ComputeStats((unsigned short*)buffer,
		imageInfo,
		r, TRUE, TRUE, FALSE);

	if ((0 == snapshotFlag) || (2 == snapshotFlag))
	{
		if (FALSE == PostflightCamera(SelectedHardware::SELECTED_CAMERA1))
		{
			StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage PostflightAcquisition failed");
			logDll->TLTraceEvent(ERROR_EVENT, 1, message);
			return FALSE;
		}
	}

	return TRUE;
}

UINT SnapshotThreadProcSciCam(LPVOID pParam)
{
	PreflightPMT();

	//open shutter
	SetShutterPosition(SHUTTER_OPEN);

	SetBFLampPosition(ENABLE_LEDS);

	//Enable laser emission if TTL mode is off
	long laserTTL = 0;
	GetLaserTTL(laserTTL);
	if (laserTTL == false)
	{
		SetLaser1Emission(ENABLE_EMISSION);
		SetLaser2Emission(ENABLE_EMISSION);
		SetLaser3Emission(ENABLE_EMISSION);
		SetLaser4Emission(ENABLE_EMISSION);
	}

	//snapshot shared the parameters with the live mode
	long avgFrames = 1;
	long avgMode = 0;

	//currently camera does not support SW_SINGLE_FRAME, use SW_FREE_RUN_MODE instead for CCD cameras
	if (FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_AVERAGENUM, avgFrames))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup Snapshot Get PARAM_CAMERA_AVERAGENUM failed.");
		logDll->TLTraceEvent(ERROR_EVENT, 1, message);
	}

	if (FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_AVERAGEMODE, avgMode))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup Snapshot Get PARAM_CAMERA_AVERAGEMODE failed.");
		logDll->TLTraceEvent(ERROR_EVENT, 1, message);
	}

	avgMode = (avgMode && ((avgFrames > 1) ? TRUE : FALSE));

	double exposureTime = 0;
	long binX = 0, binY = 0;
	if (FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_EXPOSURE_TIME_MS, exposureTime) ||
		(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_BINNING_X, binX)) ||
		(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_BINNING_Y, binY)))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup Snapshot Get PARAM_EXPOSURE_TIME_MS, PARAM_BINNING_X, or PARAM_BINNING_Y failed.");
		logDll->TLTraceEvent(WARNING_EVENT, 1, message);
	}

	//take picture
	long isSnapshotFlag = 0;
	stopCapture = FALSE;
	//long snapshotStatus = TRUE;
	if (FALSE == CaptureSingleImageWithAverageSciCam(pChan[0], (avgMode) ? avgFrames : 1, isSnapshotFlag))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup Snapshot CaptureSingleImage failed");
		logDll->TLTraceEvent(ERROR_EVENT, 1, message);
	}

	//set back averageMode
	if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_AVERAGEMODE, avgMode))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup Snapshot SetParam PARAM_CAMERA_AVERAGEMODE failed");
		logDll->TLTraceEvent(WARNING_EVENT, 1, message);
	}

	//set back averageFrames
	if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_AVERAGENUM, static_cast<double>(avgFrames)))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage SetParam PARAM_CAMERA_AVERAGENUM failed");
		logDll->TLTraceEvent(WARNING_EVENT, 1, message);
	}

	SetShutterPosition(SHUTTER_CLOSE);

	SetBFLampPosition(DISABLE_LEDS);

	//disable Laser Emission
	SetLaser1Emission(DISABLE_EMISSION);
	SetLaser2Emission(DISABLE_EMISSION);
	SetLaser3Emission(DISABLE_EMISSION);
	SetLaser4Emission(DISABLE_EMISSION);

	PostflightCamera(SelectedHardware::SELECTED_CAMERA1);

	PostflightPMT();

	//save tiff:
	if (NULL != pParam)
	{
		SnapshotSaveParams* sParam = (SnapshotSaveParams*)pParam;
		std::wstring name(L"Chan");
		std::vector<wchar_t> basicName(name.begin(), name.end());
		basicName.push_back('\0');
		SimplifiedSaveTIFFs(sParam->path, &basicName[0], pMemoryBuffer, sParam->channelBitMask, sParam->saveMultiPage);
		delete sParam;
	}

	CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));
	SetEvent(hCaptureActive);
	return 0;
}

UINT ZStackCaptureThreadProcSciCam(LPVOID pParam)
{
	struct CaptureSetupZCaptureParams zParams;

	if (NULL != pParam)
	{
		memcpy(&zParams, pParam, sizeof(struct CaptureSetupZCaptureParams));
		delete pParam;
	}
	else
	{
		return FALSE;
	}

	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML);

	long turretPos;
	GetTurretPosition(turretPos);
	turretPos++;
	string objName;
	double magnification;
	double numAperture;
	double afStartPos = 0;
	double afFocusOffset = 0;
	double afAdaptiveOffset = 0;
	long beamExpPos = 0;
	long beamExpWavelength = 0;
	long beamExpPos2 = 0;
	long beamExpWavelength2 = 0;
	long turretPosition = 0;
	long zAxisToEscape = 0;
	double zAxisEscapeDistance = 0;
	double fineAutoFocusPercentage = 0.15;
	pHardware->GetMagInfoFromPosition(turretPos, objName, magnification, numAperture, afStartPos, afFocusOffset, afAdaptiveOffset, beamExpPos, beamExpWavelength, beamExpPos2, beamExpWavelength2, turretPosition, zAxisToEscape, zAxisEscapeDistance, fineAutoFocusPercentage);

	double umPerPixel = 1;
	double cameraPixelSize;
	GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_PIXEL_SIZE, cameraPixelSize);
	umPerPixel = cameraPixelSize / magnification;

	PreflightPMT();

	//open shutter
	SetShutterPosition(SHUTTER_OPEN);

	SetBFLampPosition(ENABLE_LEDS);

	//Enable laser emission if TTL mode is off
	long laserTTL = 0;
	GetLaserTTL(laserTTL);
	if (laserTTL == false)
	{
		SetLaser1Emission(ENABLE_EMISSION);
		SetLaser2Emission(ENABLE_EMISSION);
		SetLaser3Emission(ENABLE_EMISSION);
		SetLaser4Emission(ENABLE_EMISSION);
	}

	if (SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_TRIGGER_MODE, ICamera::SW_MULTI_FRAME) != TRUE)
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup ZStackCaptureProc failed");
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
	}

	double pos = zParams.start;

	wstring zStackCacheDir = ResourceManager::getInstance()->GetZStackCachePath();

	disableZRead = TRUE;

	long avgFrames = 1, avgMode = 0;
	if (TRUE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_AVERAGEMODE, avgMode) && (ICamera::AVG_MODE_CUMULATIVE == avgMode))
	{
		if (FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_AVERAGENUM, avgFrames))
		{
			StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup ZStackCaptureProc Get PARAM_LSM_AVERAGENUM failed.");
			logDll->TLTraceEvent(ERROR_EVENT, 1, message);
		}
	}

	double paramValue;

	if (TRUE != GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_CHANNEL, paramValue) &&
		TRUE != GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_CHANNEL, paramValue))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"CaptureSetup ZStackCaptureProc GetParam failed PARAM_LSM_CHANNEL");
		disableZRead = FALSE;
		return 0;
	}
	long lsmChan = static_cast<long>(paramValue);
	std::vector<wstring> wavelengthNames;
	long chanCount = ParseLSMChannels(lsmChan, NULL, &wavelengthNames, L"Chan");
	long doOME = TRUE;
	long doCompression = TRUE;
	GetTIFFConfiguration(doOME, doCompression);

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS, L"ImageNameFormat", L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	imgNameFormat << L"%s%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";

	string strOME = (TRUE == doOME) ? uUIDSetup(wavelengthNames, true, chanCount, zParams.numOfSteps, 1, 1) : "";

	//step through z and capture
	for (long z = 1; (z <= zParams.numOfSteps) && (FALSE == stopCapture); z++)
	{
		if (FALSE == SetZPosition(pos))
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"CaptureSetup ZStackCaptureProc SetZPosition failed ");
			//Capture interrupted:
			InterruptCapture = TRUE;
			break;
		}

		//0 : snapshot, 
		//1: capturing first image of stack, 
		//2: capturing last image of stack
		//3: capturing non-start, non-end images of stack
		long snapshotFlag = 0;
		if (1 == z)
		{
			snapshotFlag = 1;
		}
		else if (zParams.numOfSteps == z)
		{
			snapshotFlag = 2;
		}
		else
		{
			snapshotFlag = 3;
		}

		//take picture
		if (FALSE == CaptureSingleImageWithAverageSciCam(pChan[0], avgFrames, snapshotFlag))
		{
			StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup ZStackCaptureProc CaptureSingleImage failed");
			logDll->TLTraceEvent(ERROR_EVENT, 1, message);
			disableZRead = FALSE;
			return FALSE;
		}

		//shared parameters with live image mode
		// NOTE: These need to be read AFTER acquisition has been setup, because some camera properties do not sync until then.
		long width = 0, height = 0;
		GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_IMAGE_WIDTH, width);
		GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_IMAGE_HEIGHT, height);

		//set back averageFrames	
		if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_AVERAGENUM, static_cast<double>(avgFrames)))
		{
			StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup CaptureSingleImage SetParam PARAM_MULTI_FRAME_COUNT failed");
			logDll->TLTraceEvent(WARNING_EVENT, 1, message);
		}

		//get the number of total channels
		long paramLong;
		double paramDouble;
		double paramMax;

		GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_CAMERA_CHANNEL, paramLong, paramLong, paramLong, paramDouble, paramMax, paramDouble);

		long maxChannels = ParseLSMChannels(static_cast<long>(paramMax), NULL, NULL, L"");

		string wavelengthName;
		wchar_t filePathAndName[_MAX_PATH];

		//save each channel to tiff images
		int count = 0;
		for (int c = 0; c < maxChannels; c++)
		{
			if (lsmChan & (0x1 << c))
			{
				pHardware->GetWavelengthName(c, wavelengthName);
				StringCbPrintfW(filePathAndName, _MAX_PATH, imgNameFormat.str().c_str(), zStackCacheDir.c_str(), wavelengthName.c_str(), 1, 1, z, 1);
				char* pTemp = pChan[0];
				SaveTIFF(filePathAndName, pTemp + count * width * height * 2, width, height, NULL, NULL, NULL, umPerPixel, imageInfo.channels, 1, zParams.numOfSteps, 0, c, 1, z, NULL, 0, &strOME, doCompression);
				count++;
			}
		}

		pos += zParams.stepSize / (double)Constants::UM_TO_MM;
	}
	//set back averageMode:
	if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_AVERAGEMODE, avgMode))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup ZStackCaptureProc SetParam PARAM_LSM_AVERAGEMODE failed");
		logDll->TLTraceEvent(WARNING_EVENT, 1, message);
	}

	//notify listeners that the ZStack capture is finished because of interrupted
	if (InterruptCapture)
	{
		CHECK_INLINE_PACTIVEIMAGEROUTINE(StopZStackCapture());
		(*myFuncPtrZStack)();
		InterruptCapture = FALSE;
	}
	else
	{
		//notify listeners that the ZStack capture is finished if not interrupted
		if (FALSE == stopCapture)
		{
			(*myFuncPtrZStack)();
		}

		SetEvent(hCaptureActive);
	}
	SetShutterPosition(SHUTTER_CLOSE);

	SetBFLampPosition(DISABLE_LEDS);

	//disable Laser Emission
	SetLaser1Emission(DISABLE_EMISSION);
	SetLaser2Emission(DISABLE_EMISSION);
	SetLaser3Emission(DISABLE_EMISSION);
	SetLaser4Emission(DISABLE_EMISSION);

	//return to start z position after z-stack capture finished
	if (FALSE == stopCapture)
	{
		if (FALSE == SetZPosition(zParams.start))
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"CaptureZStack Execute SetZPosition failed ");
		}
	}
	CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));
	CHECK_INLINE_PACTIVEIMAGEROUTINE(SetZStackActive(FALSE));
	disableZRead = FALSE;

	return 0;
}

long ImageRoutineSciCam::StartAutoExposure()
{
	if (NULL == myFunctionPointer)
	{
		return FALSE;
	}

	if (GetCaptureActive() == TRUE)
	{
		return TRUE; // TODO: why return true?
	}

	if (FALSE == SetupCaptureBuffers())
	{
		return FALSE;
	}

	ICamera* camera = GetCamera(SelectedHardware::SELECTED_CAMERA1);
	if (nullptr == camera)
	{
		return FALSE;
	}

	// TODO: use standard library threading
	SAFE_DELETE_HANDLE(_hAutoExposureCaptureThread);

	SAFE_DELETE_HANDLE(_hAutoExposureStatusThread);

	SAFE_DELETE_HANDLE(hCaptureActive);

	hCaptureActive = CreateEvent(0, FALSE, FALSE, 0);

	stopCapture = FALSE;
	SetCaptureActive(TRUE);

	_hAutoExposureCaptureThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoExposureCaptureThreadProcSciCam, NULL, 0, &_dwAutoExposureCaptureThreadId);
	SetThreadPriority(_hAutoExposureCaptureThread, THREAD_PRIORITY_BELOW_NORMAL);
	_hAutoExposureStatusThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoExposureStatusThreadProcSciCam, NULL, 0, &_dwAutoExposureStatusThreadId);
	SetThreadPriority(_hAutoExposureStatusThread, THREAD_PRIORITY_BELOW_NORMAL);

	return 0;
}

long ImageRoutineSciCam::StopAutoExposure()
{
	SetAutoExposureStopFlag();
	return 0;
}

UINT AutoExposureCaptureThreadProcSciCam()
{

	ICamera* camera = GetCamera(SelectedHardware::SELECTED_CAMERA1);
	RunAutoExposure(camera);

	CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));
	SetEvent(hCaptureActive);
	return 0;
}

UINT AutoExposureStatusThreadProcSciCam()
{
	long imageReadyToCopy = FALSE;
	bool isAutoExposureRunning = true;
	long frameNumber = -1;

	// Constantly check for an image buffer from AutoExposureModule, when an image is ready copy it to the local buffer pChan in CaptureSetup
	while (isAutoExposureRunning)
	{
		long newFrameNumber = -1;
		bool success = GetAutoExposureImage(pChan[0], imageInfo, (long)imageBufferSize, newFrameNumber);
		if (success && newFrameNumber != frameNumber)
		{
			//try update display buffer
			ImageRoutineSciCam::_enableCopy = FALSE;
			(*myFunctionPointer)(pChan[0], imageInfo);
			frameNumber = newFrameNumber;
		}
		isAutoExposureRunning = IsAutoExposureRunning();
	}

	return TRUE;
}

long ImageRoutineSciCam::StartAutoFocus(double magnification, long autoFocusType, BOOL& afFound)
{
	if (myFunctionPointer != NULL)
	{
		if (GetCaptureActive() == TRUE)
		{
			return TRUE;
		}

		//Set preview buffer, allocate memory for it
		if (FALSE == SetupCaptureBuffers())
			return FALSE;

		SAFE_DELETE_HANDLE(_hAutoFocusCaptureThread);

		SAFE_DELETE_HANDLE(_hAutoFocusStatusThread);

		SAFE_DELETE_HANDLE(hCaptureActive);

		hCaptureActive = CreateEvent(0, FALSE, FALSE, 0);

		struct AutoFocusCaptureParams* afParams = static_cast<AutoFocusCaptureParams*>(malloc(sizeof(struct AutoFocusCaptureParams)));
		afParams->magnification = magnification;
		afParams->autoFocusType = autoFocusType;
		//afParams->afFound = afFound;

		stopCapture = FALSE;
		SetCaptureActive(TRUE);
		_hAutoFocusCaptureThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoFocusCaptureThreadProcSciCam, (LPVOID)afParams, 0, &_dwAutoFocusCaptureThreadId);
		SetThreadPriority(_hAutoFocusCaptureThread, THREAD_PRIORITY_BELOW_NORMAL);
		_hAutoFocusStatusThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoFocusStatusThreadProcSciCam, NULL, 0, &_dwAutoFocusStatusThreadId);
		SetThreadPriority(_hAutoFocusStatusThread, THREAD_PRIORITY_BELOW_NORMAL);
	}

	return TRUE;
}

//Start the autofocus routine, it needs to be on it's own thread to run the library AutoFocusModule without hanging the GUI update
UINT AutoFocusCaptureThreadProcSciCam(LPVOID pParam)
{
	struct AutoFocusCaptureParams afParams;

	if (NULL != pParam)
	{
		memcpy(&afParams, pParam, sizeof(struct AutoFocusCaptureParams));
		delete pParam;
	}
	else
	{
		CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));
		//disableZRead = FALSE;
		SetEvent(hCaptureActive);
		return FALSE;
	}

	PreflightPMT();

	//open shutter
	SetShutterPosition(SHUTTER_OPEN);

	SetBFLampPosition(ENABLE_LEDS);

	//Enable laser emission if TTL mode is off
	long laserTTL = 0;
	GetLaserTTL(laserTTL);
	if (laserTTL == false)
	{
		SetLaser1Emission(ENABLE_EMISSION);
		SetLaser2Emission(ENABLE_EMISSION);
		SetLaser3Emission(ENABLE_EMISSION);
		SetLaser4Emission(ENABLE_EMISSION);
	}

	if (SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_TRIGGER_MODE, ICamera::SW_MULTI_FRAME) != TRUE)
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup AutoFocusCaptureThreadProcLSM failed");
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
	}

	long magnification = static_cast<long>(afParams.magnification);
	long autoFocusType = afParams.autoFocusType;
	BOOL afFound;

	RunAutofocus(magnification, autoFocusType, afFound);

	SetShutterPosition(SHUTTER_CLOSE);
	SetBFLampPosition(DISABLE_LEDS);

	//disable Laser Emission
	SetLaser1Emission(DISABLE_EMISSION);
	SetLaser2Emission(DISABLE_EMISSION);
	SetLaser3Emission(DISABLE_EMISSION);
	SetLaser4Emission(DISABLE_EMISSION);

	PostflightPMT();

	CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));
	SetEvent(hCaptureActive);
	return 0;
}

//Checks the status of the AutoFocusModule API, checks if an image buffer is ready, retreives it and updates the bitmap in Capture Setup, then saves the frames as TIFF in the Documents AutoFocusCache folder.
UINT AutoFocusStatusThreadProcSciCam()
{
	long imageReadyToCopy = FALSE, autoFocusRunning = FALSE;
	long frameNumber = 0, currentRepeat = 0, status = 0, numOfZSteps = 0, currentZIndex = 0;
	int frameNum = 0;

	//shared parameters with live image mode
	long width = 0, height = 0, binX = 0, binY = 0, lsmPixelX = 0, lsmPixelY = 0;
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_IMAGE_WIDTH, width);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_IMAGE_HEIGHT, height);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_BINNING_X, binX);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_BINNING_Y, binY);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_PIXEL_X, lsmPixelX);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_PIXEL_Y, lsmPixelY);
	double exposureTime = 0;
	GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_EXPOSURE_TIME_MS, exposureTime);

	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML);

	long turretPos;
	GetTurretPosition(turretPos);
	turretPos++;
	string objName;
	double magnification;
	double numAperture;
	double afStartPos = 0;
	double afFocusOffset = 0;
	double afAdaptiveOffset = 0;
	long beamExpPos = 0;
	long beamExpWavelength = 0;
	long beamExpPos2 = 0;
	long beamExpWavelength2 = 0;
	long turretPosition = 0;
	long zAxisToEscape = 0;
	double zAxisEscapeDistance = 0;
	double fineAutoFocusPercentage = 0.15;
	pHardware->GetMagInfoFromPosition(turretPos, objName, magnification, numAperture, afStartPos, afFocusOffset, afAdaptiveOffset, beamExpPos, beamExpWavelength, beamExpPos2, beamExpWavelength2, turretPosition, zAxisToEscape, zAxisEscapeDistance, fineAutoFocusPercentage);

	double umPerPixel = 1;
	double cameraPixelSize;
	GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_PIXEL_SIZE, cameraPixelSize);
	umPerPixel = cameraPixelSize / magnification;

	//double pos = zParams.start;

	wstring autoFocusCacheDir = ResourceManager::getInstance()->GetAutoFocusCachePath();

	double paramValue;
	std::string strOME;
	std::wstringstream imgNameFormat;
	std::vector<wstring> wavelengthNames;

	if (TRUE != GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_CHANNEL, paramValue) &&
		TRUE != GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_CHANNEL, paramValue))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"CaptureSetup AutoFocusStatusThreadProcLSM GetParam failed PARAM_LSM_CHANNEL");
		return FALSE;
	}
	long lsmChan = static_cast<long>(paramValue);
	long chanCount = ParseLSMChannels(lsmChan, NULL, &wavelengthNames, L"Chan");
	long doOME = TRUE;
	long doCompression = TRUE;
	GetTIFFConfiguration(doOME, doCompression);

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS, L"ImageNameFormat", L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	imgNameFormat << L"%s%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";

	imageInfo.fullFrame = TRUE;

	imageInfo.imageWidth = static_cast<long>(width);
	imageInfo.imageHeight = static_cast<long>(height);

	strOME = ""; //(TRUE == doOME) ? uUIDSetup(wavelengthNames, true, chanCount, zParams.numOfSteps, 1, 1) : "";

	//wait until Auto Focus has started or timeout at 200ms
	clock_t nextUpdateLoop = clock();
	while (static_cast<unsigned long>(abs(nextUpdateLoop - clock()) / (CLOCKS_PER_SEC / 1000)) < 1000 && FALSE == autoFocusRunning)
	{
		GetAutoFocusStatusAndImage(pChan[0], imageReadyToCopy, autoFocusRunning, frameNumber, currentRepeat, status, numOfZSteps, currentZIndex);
	}
	if (FALSE == autoFocusRunning) // if auto focus is still not running, throw an error message to the log
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"CaptureSetup AutoFocusStatusThreadProcLSM Failed to start autofocus. AutoFocusCaptureThreadProcLSM didn't call RunAutofocus in time. Timed out after 1000ms.");
	}

	//Check constantly for an image buffer from AutoFocusModule, when an image is ready copy it to the local buffer pChan in CaptureSetup
	while (TRUE == autoFocusRunning)
	{
		if (FALSE == imageReadyToCopy)
		{
			Sleep(1);
		}
		else
		{
			//get the number of total channels
			long paramLong;
			double paramDouble;
			double paramMax;

			GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_CAMERA_CHANNEL, paramLong, paramLong, paramLong, paramDouble, paramMax, paramDouble);

			long maxChannels = ParseLSMChannels(static_cast<long>(paramMax), NULL, NULL, L"");

			string wavelengthName;
			wchar_t filePathAndName[_MAX_PATH];

			wstring savingFolder = (COARSE_AUTOFOCUS == status) ? autoFocusCacheDir + to_wstring(currentRepeat) + L"\\Coarse\\" : autoFocusCacheDir + to_wstring(currentRepeat) + L"\\Fine\\";

			//save each channel to tiff images
			long count = 0;
			for (int c = 0; c < maxChannels; c++)
			{
				if (lsmChan & (0x1 << c))
				{
					pHardware->GetWavelengthName(c, wavelengthName);
					StringCbPrintfW(filePathAndName, _MAX_PATH, imgNameFormat.str().c_str(), savingFolder.c_str(), wavelengthName.c_str(), 1, 1, currentZIndex, 1);
					char* pTemp = pChan[0];
					SaveTIFF(filePathAndName, pTemp + (count * width * height * (long)2), width, height, NULL, NULL, NULL, umPerPixel, imageInfo.channels, 1, numOfZSteps, 0, c, 1, currentZIndex, NULL, 0, &strOME, doCompression);
					count++;
				}
			}

			ImageRoutineSciCam::_enableCopy = FALSE;

			//try update display buffer
			(*myFunctionPointer)(pChan[0], imageInfo);

			frameNum++;
		}
		GetAutoFocusStatusAndImage(pChan[0], imageReadyToCopy, autoFocusRunning, frameNumber, currentRepeat, status, numOfZSteps, currentZIndex);
		//pos += zParams.stepSize / (double)Constants::UM_TO_MM;
	}

	return TRUE;
}

UINT SequentialPreviewCaptureThreadProcSciCam()
{
	//Get the Capture Sequence Settings.
	//reading from the experiment setup XML files
	wstring sequentialCacheDir = ResourceManager::getInstance()->GetSequentialCachePath();
	wstring sequentialExperimentSettings = sequentialCacheDir + wstring(L"Experiment.xml");
	IExperiment* exp;
	vector<IExperiment::SequenceStep> captureSequence;

	exp = ExperimentManager::getInstance()->GetExperiment(sequentialExperimentSettings);
	exp->GetSequenceSteps(captureSequence);

	//Read the current Active settings from all devices that change in sequential mode. Save the values so they can be set back at the end
	long camChannel, lsmChannel, expLaserTTL, laserAnalog, multiphotonPos, lightPathGGEnabled, lightPathGREnabled, lightPathCamEnabled, invertedLightPathPosition, lightPathNDDPos, epiTurretPos;
	long mclsEnabled[MAX_CHANNEL_COUNT], wavelength[MAX_CHANNEL_COUNT], digitalSwitchesPos[MAX_DIGITAL_SWITCHES];
	double mclsPower[MAX_CHANNEL_COUNT], exposure;

	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_CHANNEL, lsmChannel);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_CHANNEL, camChannel);
	GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_EXPOSURE_TIME_MS, exposure);

	GetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_ENABLE, mclsEnabled[0]);
	GetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_POWER, mclsPower[0]);
	GetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_ENABLE, mclsEnabled[1]);
	GetDeviceParamDouble(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_POWER, mclsPower[1]);
	GetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_ENABLE, mclsEnabled[2]);
	GetDeviceParamDouble(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_POWER, mclsPower[2]);
	GetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_ENABLE, mclsEnabled[3]);
	GetDeviceParamDouble(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_POWER, mclsPower[3]);
	GetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_TTL_MODE, expLaserTTL);
	GetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_ANALOG_MODE, laserAnalog);
	GetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_WAVELENGTH, wavelength[0]);
	GetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_WAVELENGTH, wavelength[1]);
	GetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_WAVELENGTH, wavelength[2]);
	GetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_WAVELENGTH, wavelength[3]);

	GetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_POS, multiphotonPos);

	GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_GG, lightPathGGEnabled);
	GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_GR, lightPathGREnabled);
	GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_CAMERA, lightPathCamEnabled);
	GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_INVERTED_POS, invertedLightPathPosition);
	GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_NDD, lightPathNDDPos);

	for (long j = 0; j < MAX_DIGITAL_SWITCHES; j++)
	{
		GetDeviceParamLong(SelectedHardware::SELECTED_EPHYS, IDevice::PARAM_EPHYS_DIG_LINE_OUT_1 + j, digitalSwitchesPos[j]);
	}

	GetDeviceParamLong(SelectedHardware::SELECTED_EPITURRET, IDevice::PARAM_FW_DIC_POS, epiTurretPos);

	//Read the hardware settings for magnification values
	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML);

	long turretPos;
	GetTurretPosition(turretPos);
	turretPos++;

	string objName;
	double magnification, numAperture, afStartPos = 0, afFocusOffset = 0, afAdaptiveOffset = 0, zAxisEscapeDistance = 0, fineAutoFocusPercentage = 0.15;
	long beamExpPos = 0, beamExpWavelength = 0, beamExpPos2 = 0, beamExpWavelength2 = 0, turretPosition = 0, zAxisToEscape = 0;
	pHardware->GetMagInfoFromPosition(turretPos, objName, magnification, numAperture, afStartPos, afFocusOffset, afAdaptiveOffset, beamExpPos, beamExpWavelength, beamExpPos2, beamExpWavelength2, turretPosition, zAxisToEscape, zAxisEscapeDistance, fineAutoFocusPercentage);

	double cameraPixelSize;
	GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_PIXEL_SIZE, cameraPixelSize);
	double umPerPixel = cameraPixelSize / magnification;

	long width, height;
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_IMAGE_WIDTH, width);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_IMAGE_HEIGHT, height);

	PreflightPMT();

	//open shutter
	SetShutterPosition(SHUTTER_OPEN);

	SetBFLampPosition(ENABLE_LEDS);

	//Enable laser emission if TTL mode is off
	long laserTTL = 0;
	GetLaserTTL(laserTTL);
	if (laserTTL == false)
	{
		SetLaser1Emission(ENABLE_EMISSION);
		SetLaser2Emission(ENABLE_EMISSION);
		SetLaser3Emission(ENABLE_EMISSION);
		SetLaser4Emission(ENABLE_EMISSION);
	}

	if (SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_TRIGGER_MODE, ICamera::SW_MULTI_FRAME) != TRUE)
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup SequentialPreviewCaptureThreadProcSciCam failed. Could not set PARAM_TRIGGER_MODE to SW_MULTI_FRAME");
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
	}

	disableZRead = TRUE;

	long avgFrames = 1, avgMode = 0;
	if (TRUE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_AVERAGEMODE, avgMode) && (ICamera::AVG_MODE_CUMULATIVE == avgMode))
	{
		if (FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_AVERAGEMODE, avgFrames))
		{
			StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup SequentialPreviewCaptureThreadProcSciCam Get PARAM_CAMERA_AVERAGEMODE failed.");
			logDll->TLTraceEvent(ERROR_EVENT, 1, message);
		}
	}

	//step through the sequence steps and capture
	for (long i = 0; i < captureSequence.size() && FALSE == stopCapture; i++)
	{
		SetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_CHANNEL, captureSequence[i].LSMChannel);
		SetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_EXPOSURE_TIME_MS, (long)captureSequence[i].Exposure);

		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_TTL_MODE, captureSequence[i].LaserTTL, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_ANALOG_MODE, captureSequence[i].LaserAnalog, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_ENABLE, captureSequence[i].MCLSEnable1, TRUE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_POWER, captureSequence[i].MCLSPower1, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_ENABLE, captureSequence[i].MCLSEnable2, TRUE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_POWER, captureSequence[i].MCLSPower2, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_ENABLE, captureSequence[i].MCLSEnable3, TRUE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_POWER, captureSequence[i].MCLSPower3, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_ENABLE, captureSequence[i].MCLSEnable4, TRUE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_POWER, captureSequence[i].MCLSPower4, TRUE);

		if (FALSE == captureSequence[i].LaserTTL)
		{
			SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_EMISSION, captureSequence[i].MCLSEnable1, TRUE);
			SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_EMISSION, captureSequence[i].MCLSEnable2, TRUE);
			SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_EMISSION, captureSequence[i].MCLSEnable3, TRUE);
			SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_EMISSION, captureSequence[i].MCLSEnable4, TRUE);
		}

		/* :TODO: Need to see if setting the wavelength is necessary
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_WAVELENGTH, captureSequence[i].Wavelength1, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_WAVELENGTH, captureSequence[i].Wavelength2, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_WAVELENGTH, captureSequence[i].Wavelength3, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_WAVELENGTH, captureSequence[i].Wavelength4, TRUE);*/

		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_POS, captureSequence[i].MultiphotonPos, TRUE);

		SetDeviceParamLong(SelectedHardware::SELECTED_PINHOLEWHEEL, IDevice::PARAM_PINHOLE_POS, captureSequence[i].PinholePos, TRUE);

		SetDeviceParamLong(SelectedHardware::SELECTED_PMT1, IDevice::PARAM_PMT1_ENABLE, captureSequence[i].PMT1Enable, TRUE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_PMT1, IDevice::PARAM_PMT1_GAIN_POS, captureSequence[i].PMT1Gain, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_PMT2, IDevice::PARAM_PMT2_ENABLE, captureSequence[i].PMT2Enable, TRUE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_PMT2, IDevice::PARAM_PMT2_GAIN_POS, captureSequence[i].PMT2Gain, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_PMT3, IDevice::PARAM_PMT3_ENABLE, captureSequence[i].PMT3Enable, TRUE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_PMT3, IDevice::PARAM_PMT3_GAIN_POS, captureSequence[i].PMT3Gain, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_PMT4, IDevice::PARAM_PMT4_ENABLE, captureSequence[i].PMT4Enable, TRUE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_PMT4, IDevice::PARAM_PMT4_GAIN_POS, captureSequence[i].PMT4Gain, TRUE);

		SetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_GG, captureSequence[i].LightPathGGEnable, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_GR, captureSequence[i].LightPathGREnable, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_CAMERA, captureSequence[i].LightPathCamEnable, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_INVERTED_POS, captureSequence[i].InvertedLightPathPosition, TRUE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_NDD, captureSequence[i].LightPathNDDPosition, TRUE);

		if (TRUE == captureSequence[i].DigSwitchesEnabled)
		{
			for (long j = 0; j < MAX_DIGITAL_SWITCHES; j++)
			{
				SetDeviceParamLong(SelectedHardware::SELECTED_EPHYS, IDevice::PARAM_EPHYS_DIG_LINE_OUT_1 + j, captureSequence[i].DigSwitchPos[j], TRUE);
			}
		}

		SetDeviceParamLong(SelectedHardware::SELECTED_EPITURRET, IDevice::PARAM_EPI_TURRET_POS, captureSequence[i].EpiTurretPosition - 1, TRUE);

		long lsmChan;
		std::string strOME;
		std::wstringstream imgNameFormat;
		if (TRUE != GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_CHANNEL, lsmChan) &&
			TRUE != GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_CHANNEL, lsmChan))
		{
			logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"CaptureSetup SequentialPreviewCaptureThreadProcSciCam GetParam failed PARAM_LSM_CHANNEL/PARAM_CAMERA_CHANNEL");
			goto RETURN_TASK;
		}

		long chanCount = ParseLSMChannels(lsmChan, NULL, NULL, L"");
		long doOME = TRUE;
		long doCompression = TRUE;
		GetTIFFConfiguration(doOME, doCompression);

		imageInfo.fullFrame = TRUE;
		imageInfo.imageWidth = static_cast<long>(width);
		imageInfo.imageHeight = static_cast<long>(height);
		ImageRoutineSciCam::_enableCopy = FALSE;
		imageInfo.totalSequences = (long)captureSequence.size();
		imageInfo.sequenceIndex = i;
		imageInfo.sequenceSelectedChannels = lsmChan;

		vector<wstring> wavelengthNames;
		for (long j = 0; j < captureSequence[i].Wavelength.size(); j++)
		{
			string name = captureSequence[i].Wavelength[j].name;
			wstring ws(name.begin(), name.end());
			wavelengthNames.push_back(ws);
		}

		long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS, L"ImageNameFormat", L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
		imgNameFormat << L"%s%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";

		strOME = (TRUE == doOME) ? uUIDSetup(wavelengthNames, true, chanCount, 0, 1, 1) : "";

		long snapshotFlag = 0;
		//take picture
		if (FALSE == CaptureSingleImageWithAverageSciCam(pChan[0], avgFrames, snapshotFlag))
		{
			StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup SequentialPreviewCaptureThreadProcSciCam, CaptureSingleImageWithAverageSciCam failed");
			logDll->TLTraceEvent(ERROR_EVENT, 1, message);
			goto RETURN_TASK;
		}
		
		double paramCameraType = 0.0;
		GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParam(ICamera::PARAM_CAMERA_TYPE, paramCameraType);
		ICamera::CameraType cameraType = (ICamera::CameraType)static_cast<int>(paramCameraType);

		//get the number of total channels
		long paramLong;
		double paramDouble;
		double paramMax;
		GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_CAMERA_CHANNEL, paramLong, paramLong, paramLong, paramDouble, paramMax, paramDouble);
		if (ICamera::CameraType::CCD == cameraType)
		{
			// NOTE: TSI cameras can have a variable number of max channels saved to the experiment.
			// When another camera uses the same sequence step template but has a different number of channels, that can cause trouble.
			// TODO: We take the lower of the camera channels and the num of wavelengths in sequence steps in order to prevent crashing
			int numChannels = 0;
			unsigned int channelMask = (unsigned int) paramMax;
			while (channelMask)
			{
				numChannels += 1;
				channelMask >>= 1;
			}
			int numWavelengths = (int) captureSequence[i].Wavelength.size();

			if (numWavelengths < numChannels)
			{
				logDll->TLTraceEvent(ERROR_EVENT, 1, L"ERROR: Camera has more channels than the Sequential Step template. The template channels will be used.");
				paramMax = captureSequence[i].CameraChannel; // This should be equivalent to a wavelength mask
			}
			else if(numWavelengths > numChannels)
			{
				logDll->TLTraceEvent(ERROR_EVENT, 1, L"ERROR: Camera has less channels than the Sequential Step template. The camera channels will be used.");
				//paramMax = paramMax;
			}
		}

		long maxChannels = ParseLSMChannels(static_cast<long>(paramMax), NULL, NULL, L"");

		wchar_t filePathAndName[_MAX_PATH];
		long count = 0;

		//save each channel to tiff images
		for (int c = 0; c < maxChannels; c++)
		{
			if (lsmChan & (0x1 << c))
			{
				StringCbPrintfW(filePathAndName, _MAX_PATH, imgNameFormat.str().c_str(), sequentialCacheDir.c_str(), captureSequence[i].Wavelength[count].name.c_str(), 1, 1, 0, 1);
				char* pTemp = pChan[0];
				SaveTIFF(filePathAndName, pTemp + count * width * height * 2, width, height, NULL, NULL, NULL, umPerPixel, imageInfo.channels, 1, 0, 0, c, 1, 0, NULL, 0, &strOME, doCompression);
				count++;
			}
		}

		//try update display buffer
		(*myFunctionPointer)(pChan[0], imageInfo);
	}
	//set back averageMode:
	if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_AVERAGEMODE, avgMode))
	{
		StringCbPrintfW(message, MSG_SIZE, L"CaptureSetup SequentialPreviewCaptureThreadProcSciCam SetParam PARAM_CAMERA_AVERAGEMODE failed");
		logDll->TLTraceEvent(WARNING_EVENT, 1, message);
	}

	// use the ZStack callback to notify the capture is done and the progress window needs to be closed. No need to create another callback because this one does the job.
	if (FALSE == stopCapture)
	{
		(*myFuncPtrZStack)();
	}

	SetEvent(hCaptureActive);
	SetShutterPosition(SHUTTER_CLOSE);
	SetBFLampPosition(DISABLE_LEDS);

	//disable Laser Emission
	SetLaser1Emission(DISABLE_EMISSION);
	SetLaser2Emission(DISABLE_EMISSION);
	SetLaser3Emission(DISABLE_EMISSION);
	SetLaser4Emission(DISABLE_EMISSION);

	PostflightPMT();

	goto RETURN_TASK;

RETURN_TASK:
	//Set the original active parameters back to each device.
	SetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_CHANNEL, lsmChannel);
	SetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_EXPOSURE_TIME_MS, (long) exposure);

	SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_TTL_MODE, expLaserTTL, TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_ANALOG_MODE, laserAnalog, TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_ENABLE, mclsEnabled[0], TRUE);
	SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_POWER, mclsPower[0], TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_ENABLE, mclsEnabled[1], TRUE);
	SetDeviceParamDouble(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_POWER, mclsPower[1], TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_ENABLE, mclsEnabled[2], TRUE);
	SetDeviceParamDouble(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_POWER, mclsPower[2], TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_ENABLE, mclsEnabled[3], TRUE);
	SetDeviceParamDouble(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_POWER, mclsPower[3], TRUE);

	/* :TODO: Need to see if setting the wavelength is necessary
	SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_WAVELENGTH, wavelength[0], TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_WAVELENGTH, wavelength[1], TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_WAVELENGTH, wavelength[2], TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_WAVELENGTH, wavelength[3], TRUE);*/

	SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_POS, multiphotonPos, TRUE);

	SetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_GG, lightPathGGEnabled, TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_GR, lightPathGREnabled, TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_CAMERA, lightPathCamEnabled, TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_INVERTED_POS, invertedLightPathPosition, TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::PARAM_LIGHTPATH_NDD, lightPathNDDPos, TRUE);

	for (long j = 0; j < MAX_DIGITAL_SWITCHES; j++)
	{
		SetDeviceParamLong(SelectedHardware::SELECTED_EPHYS, IDevice::PARAM_EPHYS_DIG_LINE_OUT_1 + j, digitalSwitchesPos[j], TRUE);
	}

	SetDeviceParamLong(SelectedHardware::SELECTED_EPITURRET, IDevice::PARAM_EPI_TURRET_POS, epiTurretPos - 1, TRUE);

	CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));
	disableZRead = FALSE;
	SetEvent(hCaptureActive);
	return 0;
}


