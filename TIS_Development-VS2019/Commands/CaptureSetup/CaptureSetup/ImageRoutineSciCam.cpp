#include "stdafx.h"
#include "CaptureSetup.h"
#include "ImageRoutineSciCam.h"

ImageRoutineSciCam::ImageRoutineSciCam()
{
	_enableCopy = FALSE;
	_channelEnable = 0;
	_captureActive = 0;
}

ImageRoutineSciCam::~ImageRoutineSciCam()
{
}

long ImageRoutineSciCam::InitCallbacks(imageCompleteCallback ic, completeCallback cc)
{	
	myFunctionPointer = ic;

	myFuncPtrZStack = cc;


	if((myFunctionPointer != NULL)&&
		(myFuncPtrZStack != NULL))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup InitCallBack");
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

	if(NULL == GetCamera(SelectedHardware::SELECTED_CAMERA1))
	{
		return FALSE;
	}

	if(GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_CAPTURE_REGION_RIGHT, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
	{
		imageInfo.imageWidth = static_cast<long>(paramMax);		
	}

	if(GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_CAPTURE_REGION_BOTTOM, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
	{
		imageInfo.imageHeight = static_cast<long>(paramMax);
	}

	StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup InitParameters maxCameraWidth %d maxCameraHeight %d",imageInfo.imageWidth,imageInfo.imageHeight);
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	return TRUE;
}
long ImageRoutineSciCam::Snapshot(SnapshotSaveParams * sParam)
{
	if(GetCaptureActive())
	{
		return FALSE;
	}

	if (FALSE == SetupCaptureBuffers())
		return FALSE;

	StringCbPrintfW(message,MSG_SIZE,L"About to SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_TRIGGER_MODE,ICamera::SW_MULTI_FRAME)");
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	if(SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_TRIGGER_MODE,ICamera::SW_MULTI_FRAME) != TRUE)
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Snapshot SetParam PARAM_TRIGGER_MODE failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
	}

	//snapshot shared the parameters with the live mode
	SAFE_DELETE_HANDLE(hCaptureActive);

	hCaptureActive = CreateEvent(0, FALSE, FALSE, 0);

	SAFE_DELETE_HANDLE(hLiveThread);

	stopCapture = FALSE;
	hLiveThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) SnapshotThreadProcSciCam, (LPVOID)sParam, 0, &dwLiveThreadId );
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
	if(myFunctionPointer != NULL)
	{	
		if(GetCaptureActive() == TRUE)
		{
			return TRUE;
		}

		if (FALSE == SetupCaptureBuffers())
			return FALSE;

		hCaptureActive = CreateEvent(0, FALSE, FALSE, 0);

		struct CaptureSetupZCaptureParams * zParams = static_cast<CaptureSetupZCaptureParams *>(malloc(sizeof(struct CaptureSetupZCaptureParams)));
		zParams->start = zStartPos;
		if(zStartPos > zStopPos)
		{
			zParams->stepSize = (-1) * zstageStepSize;
		}
		else 
		{
			zParams->stepSize = zstageStepSize;
		}
		zParams->numOfSteps = zstageSteps;

		stopCapture = FALSE;
		hZStackCaptureThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) ZStackCaptureThreadProcSciCam, (LPVOID)zParams, 0, &dwZStackCaptureThreadId );
		SetCaptureActive(TRUE);
	}

	return TRUE;

}
long ImageRoutineSciCam::StopZStackCapture()
{
	//return if the capture is not active
	if(FALSE == GetCaptureActive())
	{
		return TRUE;
	}

	if(myFunctionPointer != NULL)
	{
		stopCapture = TRUE;
	}

	if(WaitForSingleObject(hCaptureActive,Constants::EVENT_WAIT_TIME)!=WAIT_OBJECT_0)
	{
		return FALSE;
	}

	SAFE_DELETE_HANDLE(hZStackCaptureThread);

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
	if(myFunctionPointer != NULL)
	{	
		if(GetCaptureActive() == TRUE)
		{
			return TRUE;
		}

		if (FALSE == SetupCaptureBuffers())
			return FALSE;

		SAFE_DELETE_HANDLE(hCaptureActive);

		hCaptureActive = CreateEvent(0, FALSE, FALSE, 0);

		SAFE_DELETE_HANDLE(hLiveThread);

		stopCapture = FALSE;
		hLiveThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) LiveThreadProc, (LPVOID)NULL, 0, &dwLiveThreadId );
		SetCaptureActive(TRUE);
	}

	return TRUE;
}
long ImageRoutineSciCam::StopLiveCapture()
{
	//disable LEDs
	SetBFLampPosition(DISABLE_LEDS);

	//return if the capture is not active
	if(FALSE == GetCaptureActive())
	{
		return TRUE;
	}

	if(myFunctionPointer != NULL)
	{
		stopCapture = TRUE;
	}

	if(WaitForSingleObject(hCaptureActive,Constants::EVENT_WAIT_TIME) != WAIT_OBJECT_0)
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

long ImageRoutineSciCam::CopyAcquisition(long isFullFrame)
{
	long ret = 0;
	std::wstring cameraName;

	Lock lock(CaptureSetup::getInstance()->critSect);

	ret = GetCamera(SelectedHardware::SELECTED_CAMERA1)->CopyAcquisition(pChan[0], &imageInfo);

	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition");

	if(_enableCopy)
	{
		imageInfo.channels = 1;
		imageInfo.fullFrame = isFullFrame;
		(*myFunctionPointer)(pMemoryBuffer,imageInfo);

		long left=0;
		long right=0;
		long top=0;
		long bottom=0;
		long binX=1;
		long binY=1;

		if((FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_LEFT,left))||
			(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_RIGHT,right))||
			(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_TOP,top))||
			(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_BOTTOM,bottom))||
			(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_BINNING_X,binX))||
			(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_BINNING_Y,binY))
			)	
		{
			logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition GetParam failed PARAM_CAPTURE_REGION");
			return FALSE;
		}

		if(0 != binX && 0 != binY)
		{
			imageInfo.imageWidth = static_cast<long>((right-left)/binX);
			imageInfo.imageHeight = static_cast<long>((bottom-top)/binY);
			imageInfo.channels = 1;
		}
		else
		{
			logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup CopyAcquisition GetParam PARAM_BINNING_X/Y returned 0");
		}

		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition callback complete");
		_enableCopy = FALSE;
	}
	return TRUE;
}

long ImageRoutineSciCam::GetImageDimensions(long &width, long &height)
{
	if(NULL == GetCamera(SelectedHardware::SELECTED_CAMERA1))
	{
		return FALSE;
	}

	double left=0;
	double right=0;
	double top=0;
	double bottom=0;
	double binX=1.0;
	double binY=1.0;

	double angle = 0;
	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_LEFT,left))
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup GetImageDimensions left failed");
	}

	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_RIGHT,right))
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup GetImageDimensions right failed");
	}

	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_TOP,top))
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup GetImageDimensions top failed");
	}

	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_BOTTOM,bottom))
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup GetImageDimensions bottom failed");
	}

	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_BINNING_X,binX))
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup GetImageDimensions top failed");
	}

	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_BINNING_Y,binY))
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup GetImageDimensions bottom failed");
	}

	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_IMAGE_ANGLE,angle))
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup GetImageDimensions bottom failed");
	}

	//if the angle is not 0 or 180 we need to change the binning and width/height accordingly
	if (static_cast<long>(angle) != 0 && static_cast<long>(angle) != 180)
	{
		double tempRight = right;
		double tempLeft = left;
		right = bottom;
		left = top;
		bottom = tempRight;
		top = tempLeft;

		double tempBinX = binX;
		binX = binY;
		binY = tempBinX;
	}

	width = static_cast<long>((right - left)/binX);
	height = static_cast<long>((bottom - top)/binY);

	return TRUE;
}
//snapshotFlag definitions:
//0: snapshot, preflight and postflight 
//1: capturing first image of stack, preflight only
//2: capturing last image of stack, postflight only
//3: capturing non-start, non-end images of stack, no preflight or postflight
long CaptureSingleImageWithAverageSciCam(char *buffer, double exposureTime, long binningX, long binningY , long avgFrames, long snapshotFlag)
{

	if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_AVERAGENUM,static_cast<double>(1)))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage SetParam PARAM_MULTI_FRAME_COUNT failed");
		logDll->TLTraceEvent(WARNING_EVENT,1,message);
		return FALSE;
	}

	if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_MULTI_FRAME_COUNT,static_cast<double>(avgFrames)))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage SetParam PARAM_MULTI_FRAME_COUNT failed");
		logDll->TLTraceEvent(WARNING_EVENT,1,message);
		return FALSE;
	}

	if(FALSE ==SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE, 1))
	{
		logDll->TLTraceEvent(WARNING_EVENT,1,message);
	}

	if((0 == snapshotFlag) || (1 == snapshotFlag))
	{
		if(FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->PreflightAcquisition(buffer))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage PreflightAcquisition failed");
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
			return FALSE;
		}
	}
	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML);

	wstring streamPath;
	double rate; //this variable is only used in runsample 
	pHardware->GetStreaming(streamPath, rate);

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

	long imageID;

	if(FALSE == ImageManager::getInstance()->CreateImage(imageID,dim))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage CreateImage failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
		return FALSE;
	}

	if(FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->SetupAcquisition(buffer))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage Setup Acquisition failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
		return FALSE;
	}

	if(FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->StartAcquisition(buffer))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage Start Acquisition failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
		return FALSE;
	}

	char * pAvgBuffer = ImageManager::getInstance()->GetImagePtr(imageID,0,0,0,0);

	for(long i=0; i<avgFrames; i++)
	{
		long status = ICamera::STATUS_BUSY;

		while(status == ICamera::STATUS_BUSY)
		{
			if(FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->StatusAcquisition(status))
			{
				break;
			}

			if(TRUE == stopCapture)
			{
				break;
			}
		}

		if(TRUE == stopCapture)
		{
			break;
		}


		if(FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->CopyAcquisition(pAvgBuffer, &imageInfo))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage CopyAcquisition failed");
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
			ImageManager::getInstance()->UnlockImagePtr(imageID,0,0,0,i);		
			ImageManager::getInstance()->DestroyImage(imageID);
			return FALSE;
		}

		//synchrnously start the unlock process for the frame
		ImageManager::getInstance()->UnlockImagePtr(imageID,0,0,0,i);		

		//go ahead and unlock the next frame before the status call
		//this will allow the GetImagePtr delay for memory mapping to 
		//overlap with processing time of the next frame
		if(i<(avgFrames-1))
		{
			pAvgBuffer = ImageManager::getInstance()->GetImagePtr(imageID,0,0,0,i+1);
		}

	}

	imageInfo.channels = 1;
	imageInfo.fullFrame = TRUE;

	long width=0;
	long height=0;

	if((FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_IMAGE_WIDTH,width))||
		(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_IMAGE_HEIGHT,height)))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition GetParam failed PARAM_CAPTURE_REGION");
		return FALSE;
	}

	imageInfo.imageWidth = width;
	imageInfo.imageHeight = height;
	imageInfo.channels = 1;

	unsigned long * pSumBuffer = new unsigned long[imageInfo.imageWidth*imageInfo.imageHeight*imageInfo.channels];

	memset(pSumBuffer,0,sizeof(unsigned long) * imageInfo.imageWidth*imageInfo.imageHeight*imageInfo.channels);

	for(long i=0; i<avgFrames; i++)
	{
		unsigned long * pSum = pSumBuffer;

		unsigned short * pAvgBuffer = (unsigned short*)ImageManager::getInstance()->GetImagePtr(imageID,0,0,0,i);

		for(long k=0; k<imageInfo.imageWidth*imageInfo.imageHeight*imageInfo.channels; k++)
		{
			*pSum += *pAvgBuffer;
			pSum++;
			pAvgBuffer++;
		}

		ImageManager::getInstance()->UnlockImagePtr(imageID,0,0,0,i);

		// return partial of average if stopped:
		if(TRUE == stopCapture)
		{
			break;
		}

	}

	ImageManager::getInstance()->DestroyImage(imageID);

	unsigned short * pBuf = (unsigned short*)buffer;
	unsigned long * pSum = pSumBuffer;

	for(long k=0; k<imageInfo.imageWidth*imageInfo.imageHeight*imageInfo.channels; k++)
	{
		*pBuf = static_cast<unsigned short>((*pSum)/(double)avgFrames);
		pBuf++;
		pSum++;
	}

	delete[] pSumBuffer;

	(*myFunctionPointer)(buffer,imageInfo);
	////// temporarily commented out since auto roi identification has not been released.
	/////*	StatsManager::getInstance()->ComputeContours( (unsigned short*)buffer, 
	////		static_cast<long>(sumWidth),
	////		static_cast<long>(sumHeight),
	////		CaptureSetup::getInstance()->_channelEnable,
	////		_autoDisplayChannel);*/ 
	long r = FALSE;
	CHECK_INEXPRESSION_PACTIVEIMAGEROUTINE(GetDisplayChannels(), r);
	StatsManager::getInstance()->ComputeStats( (unsigned short*)buffer, 
		imageInfo,																	
		r,TRUE,TRUE,FALSE);	

	if((0 == snapshotFlag) || (2 == snapshotFlag))
	{
		if(FALSE == PostflightCamera(SelectedHardware::SELECTED_CAMERA1))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage PostflightAcquisition failed");
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
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

	//snapshot shared the parameters with the live mode
	long avgFrames=1;
	long avgMode = 0;

	//currently camera does not support SW_SINGLE_FRAME, use SW_FREE_RUN_MODE instead for CCD cameras
	if(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_AVERAGENUM, avgFrames))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Snapshot Get PARAM_CAMERA_AVERAGENUM failed.");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
	}

	if(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_AVERAGEMODE, avgMode))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Snapshot Get PARAM_CAMERA_AVERAGEMODE failed.");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
	}

	avgMode = (avgMode && ((avgFrames > 1) ? TRUE : FALSE));

	double exposureTime = 0;
	long binX = 0, binY = 0;
	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_EXPOSURE_TIME_MS, exposureTime) || 
		(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_BINNING_X, binX)) || 
		(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_BINNING_Y, binY)))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Snapshot Get PARAM_EXPOSURE_TIME_MS, PARAM_BINNING_X, or PARAM_BINNING_Y failed.");
		logDll->TLTraceEvent(WARNING_EVENT,1,message);
	}

	//take picture
	long isSnapshotFlag = 0;
	stopCapture = FALSE;
	//long snapshotStatus = TRUE;
	if(FALSE == CaptureSingleImageWithAverageSciCam(pChan[0], exposureTime, binX, binY, (avgMode) ? avgFrames : 1, isSnapshotFlag))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Snapshot CaptureSingleImage failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
	}

	//set back averageMode
	if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_AVERAGEMODE, avgMode))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Snapshot SetParam PARAM_CAMERA_AVERAGEMODE failed");
		logDll->TLTraceEvent(WARNING_EVENT,1,message);
	}

	//set back averageFrames
	if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_AVERAGENUM,static_cast<double>(avgFrames)))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage SetParam PARAM_CAMERA_AVERAGENUM failed");
		logDll->TLTraceEvent(WARNING_EVENT,1,message);
	}

	SetShutterPosition(SHUTTER_CLOSE);

	SetBFLampPosition(DISABLE_LEDS);

	PostflightCamera(SelectedHardware::SELECTED_CAMERA1);

	PostflightPMT();

	//save tiff:
	if(NULL != pParam)
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

UINT ZStackCaptureThreadProcSciCam( LPVOID pParam )
{	
	struct CaptureSetupZCaptureParams zParams;

	if(NULL != pParam)
	{
		memcpy(&zParams,pParam,sizeof(struct CaptureSetupZCaptureParams));
		delete pParam;
	}
	else
	{
		return FALSE;
	}
	//shared parameters with live image mode
	long right = 0, bottom = 0, left = 0, top = 0, binX = 0, binY = 0, lsmPixelX = 0, lsmPixelY = 0;
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_RIGHT, right);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_LEFT, left);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_BOTTOM, bottom);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAPTURE_REGION_TOP, top);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_BINNING_X, binX);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_BINNING_Y, binY);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_X, lsmPixelX);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_Y, lsmPixelY);
	double exposureTime = 0;
	GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_EXPOSURE_TIME_MS, exposureTime);

	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML);

	long turretPos;
	GetTurretPosition(turretPos);
	turretPos++;
	string objName;
	double magnification;
	double numAperture;
	double afStartPos=0;
	double afFocusOffset=0;
	double afAdaptiveOffset=0;
	long beamExpPos=0;
	long beamExpWavelength=0;
	long beamExpPos2=0;
	long beamExpWavelength2=0;
	long turretPosition=0;
	long zAxisToEscape=0;
	double zAxisEscapeDistance=0;
	pHardware->GetMagInfoFromPosition(turretPos,objName,magnification,numAperture,afStartPos,afFocusOffset,afAdaptiveOffset,beamExpPos,beamExpWavelength,beamExpPos2,beamExpWavelength2, turretPosition, zAxisToEscape, zAxisEscapeDistance);

	long width, height;
	double umPerPixel = 1;
	double cameraPixelSize;
	GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_PIXEL_SIZE, cameraPixelSize);
	umPerPixel = cameraPixelSize / magnification;

	width = static_cast<long>((right - left)/binX);
	height = static_cast<long>((bottom - top)/binY);

	PreflightPMT();

	//open shutter
	SetShutterPosition(SHUTTER_OPEN);

	SetBFLampPosition(ENABLE_LEDS);

	if(SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_TRIGGER_MODE,ICamera::SW_MULTI_FRAME) != TRUE)
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup ZStackCaptureProc failed");
		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
	}

	double pos = zParams.start;

	wstring zStackCacheDir = ResourceManager::getInstance()->GetZStackCachePath();

	disableZRead = TRUE;

	long avgFrames=1, avgMode = 0;
	if(TRUE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_AVERAGEMODE, avgMode) && (ICamera::AVG_MODE_CUMULATIVE == avgMode))
	{
		if(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_AVERAGENUM, avgFrames))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup ZStackCaptureProc Get PARAM_LSM_AVERAGENUM failed.");
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
		}
	}

	double paramValue;

	if(TRUE != GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_CHANNEL,paramValue) && 
		TRUE != GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_CHANNEL,paramValue))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup ZStackCaptureProc GetParam failed PARAM_LSM_CHANNEL");
		disableZRead = FALSE;
		return 0;
	}
	long lsmChan = static_cast<long>(paramValue);
	std::vector<wstring> wavelengthNames;
	long chanCount = ParseLSMChannels(lsmChan,NULL,&wavelengthNames,L"Chan");
	long doOME = TRUE;
	long doCompression = TRUE;
	GetTIFFConfiguration(doOME, doCompression);	

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	imgNameFormat << L"%s%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";

	string strOME = (TRUE == doOME) ? uUIDSetup(wavelengthNames, true, chanCount, zParams.numOfSteps, 1, 1) : "";

	//step through z and capture
	for(long z=1; (z<=zParams.numOfSteps)&&(FALSE == stopCapture); z++)
	{	
		if(FALSE == SetZPosition(pos))
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"CaptureSetup ZStackCaptureProc SetZPosition failed ");
			//Capture interrupted:
			InterruptCapture = TRUE;			
			break;
		}

		//0 : snapshot, 
		//1: capturing first image of stack, 
		//2: capturing last image of stack
		//3: capturing non-start, non-end images of stack
		long snapshotFlag = 0;	
		if(1 == z)
		{
			snapshotFlag = 1;
		}
		else if(zParams.numOfSteps == z)
		{
			snapshotFlag = 2;
		}
		else
		{
			snapshotFlag = 3;
		}

		//take picture
		if(FALSE == CaptureSingleImageWithAverageSciCam(pChan[0], exposureTime, binX, binY, avgFrames, snapshotFlag))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup ZStackCaptureProc CaptureSingleImage failed");
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
			disableZRead = FALSE;
			return FALSE;
		}

		//set back averageFrames	
		if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_AVERAGENUM,static_cast<double>(avgFrames)))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage SetParam PARAM_MULTI_FRAME_COUNT failed");
			logDll->TLTraceEvent(WARNING_EVENT,1,message);
		}	

		//get the number of total channels
		long paramLong;
		double paramDouble;
		double paramMax;

		GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_CAMERA_CHANNEL,paramLong,paramLong,paramLong,paramDouble,paramMax,paramDouble);

		long maxChannels = ParseLSMChannels(static_cast<long>(paramMax),NULL,NULL,L"");

		string wavelengthName;
		wchar_t filePathAndName[_MAX_PATH];

		//save each channel to tiff images
		int count = 0;
		for(int c=0; c<maxChannels; c++)
		{
			if(lsmChan & (0x1<<c))
			{
				pHardware->GetWavelengthName(c, wavelengthName);
				StringCbPrintfW(filePathAndName,_MAX_PATH, imgNameFormat.str().c_str(), zStackCacheDir.c_str(), wavelengthName.c_str(), 1, 1, z, 1);
				char *pTemp = pChan[0];
				SaveTIFF(filePathAndName, pTemp+count*width*height*2, width, height, NULL, NULL, NULL, umPerPixel, imageInfo.channels, 1, zParams.numOfSteps, 0, c, 1, z, NULL, 0, &strOME, doCompression);
				count++;
			}
		}

		pos += zParams.stepSize/(double)Constants::UM_TO_MM;		
	}
	//set back averageMode:
	if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_AVERAGEMODE, avgMode))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup ZStackCaptureProc SetParam PARAM_LSM_AVERAGEMODE failed");
		logDll->TLTraceEvent(WARNING_EVENT,1,message);
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
		if(FALSE == stopCapture)	
		{
			(*myFuncPtrZStack)();		
		}

		SetEvent(hCaptureActive);
	}
	SetShutterPosition(SHUTTER_CLOSE);

	SetBFLampPosition(DISABLE_LEDS);

	//return to start z position after z-stack capture finished
	if (FALSE == stopCapture)
	{
		if(FALSE == SetZPosition(zParams.start))
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"CaptureZStack Execute SetZPosition failed ");
		}	
	}
	CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));
	disableZRead = FALSE;

	return 0;
}

