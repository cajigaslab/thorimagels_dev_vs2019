#include "stdafx.h"
#include "CaptureSetup.h"
#include "ImageRoutineLSM.h"

ImageRoutineLSM::ImageRoutineLSM()
{
	_channelEnable = 0;
	_captureActive = 0;
}

ImageRoutineLSM::~ImageRoutineLSM()
{
}

BOOL ImageRoutineLSM::_enableCopy = FALSE;

long ImageRoutineLSM::InitCallbacks(imageCompleteCallback ic, completeCallback cc)
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

long ImageRoutineLSM::InitParameters()
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

	if(GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_LSM_PIXEL_X, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
	{
		imageInfo.imageWidth = imageInfo.imageHeight = static_cast<long>(paramMax);		
	}

	StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup InitParams maxCameraWidth %d maxCameraHeight %d",imageInfo.imageWidth,imageInfo.imageHeight);
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	return TRUE;
}

long ImageRoutineLSM::Snapshot(SnapshotSaveParams *sParam)
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

	SetCameraParamString(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_WAVEFORM_OUTPATH, sParam->path);

	//snapshot shared the parameters with the live mode
	SAFE_DELETE_HANDLE(hCaptureActive);

	hCaptureActive = CreateEvent(0, FALSE, FALSE, 0);

	SAFE_DELETE_HANDLE(hLiveThread);

	stopCapture = FALSE;
	hLiveThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) SnapshotThreadProcLSM, (LPVOID)sParam, 0, &dwLiveThreadId );
	SetCaptureActive(TRUE);

	return TRUE;
}

long ImageRoutineLSM::EnableCopyToExternalBuffer()
{
	_enableCopy = TRUE;
	return TRUE;
}

long ImageRoutineLSM::CaptureZStack(double zStartPos, double zStopPos, double zstageStepSize, long zstageSteps)
{	
	if(myFunctionPointer != NULL)
	{	
		if(GetCaptureActive() == TRUE)
		{
			return TRUE;
		}

		if (FALSE == SetupCaptureBuffers())
			return FALSE;

		SAFE_DELETE_HANDLE(hZStackCaptureThread);

		SAFE_DELETE_HANDLE(hCaptureActive);

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
		SetCaptureActive(TRUE);
		hZStackCaptureThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) ZStackCaptureThreadProcLSM, (LPVOID)zParams, 0, &dwZStackCaptureThreadId );

	}

	return TRUE;

}

long ImageRoutineLSM::StopZStackCapture()
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
	CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));
	if(WaitForSingleObject(hCaptureActive,Constants::EVENT_WAIT_TIME)!=WAIT_OBJECT_0)
	{
		return FALSE;
	}

	SAFE_DELETE_HANDLE(hZStackCaptureThread);

	SAFE_DELETE_HANDLE(hCaptureActive);

	return TRUE;
}

long ImageRoutineLSM::SetDisplayChannels(long channelEnable)
{
	_channelEnable = channelEnable;
	return TRUE;
}

long ImageRoutineLSM::GetDisplayChannels()
{
	return _channelEnable;
}

long ImageRoutineLSM::SetupCaptureBuffers()
{
	////Reinitialize the width,height,channels properties for the intermediate copy buffer
	////this ensure that any residual data from a previous camera with a different buffer size is not used
	InitParameters();

	long numChanMax, numChanMin, numChanDefault, channelInfo=0;

	if(GetCameraParamRangeLong(SelectedHardware::SELECTED_CAMERA1,static_cast<long>(ICamera::PARAM_LSM_CHANNEL), numChanMin, numChanMax, numChanDefault))
	{
		channelInfo = (ICamera::CameraType)static_cast<long>(numChanMax);		
	}

	return SetupBuffers(max(1,ParseLSMChannels(channelInfo,NULL,NULL,L"")));
}

long ImageRoutineLSM::StartLiveCapture()
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

long ImageRoutineLSM::StopLiveCapture()
{
	//disable LEDs
	SetBFLampPosition(DISABLE_LEDS);

	//disable Laser Emission
	SetLaser1Emission(DISABLE_EMISSION);
	SetLaser2Emission(DISABLE_EMISSION);
	SetLaser3Emission(DISABLE_EMISSION);
	SetLaser4Emission(DISABLE_EMISSION);

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

long ImageRoutineLSM::SetCaptureActive(long active)
{
	_captureActive = active;
	return TRUE;
}

long ImageRoutineLSM::GetCaptureActive()
{
	return _captureActive;
}

long ImageRoutineLSM::CopyAcquisition(long isFullFrame)
{
	long ret = 0;
	std::wstring cameraName;

	Lock lock(CaptureSetup::getInstance()->critSect);

	imageInfo.numberOfPlanes = 1; // assume 1 and let the camera say otherwise if it supports multiplane

	ret = GetCamera(SelectedHardware::SELECTED_CAMERA1)->CopyAcquisition(pChan[0], &imageInfo);

	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition");

	if(_enableCopy)
	{
		if(FALSE == ret)
		{
			logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition callback failed");
			_enableCopy = FALSE;
			return FALSE;
		}
		double paramValue;

		if(GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_CHANNEL,paramValue) != TRUE)
		{
			logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition GetParam failed PARAM_LSM_CHANNEL");
			return FALSE;
		}

		double width,height;
		if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_X,width))	
		{
			logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition GetParam failed PARAM_LSM_PIXEL_X");
			return FALSE;
		}

		if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_Y,height))	
		{
			logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition GetParam failed PARAM_LSM_PIXEL_X");
			return FALSE;;
		}

		long paramType;
		long paramAvailable;
		long paramReadOnly;
		double paramMin;
		double paramMax;
		double paramDefault;

		GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_LSM_CHANNEL,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);

		//if the data is a single channel
		imageInfo.fullFrame = isFullFrame;

		if(1 == ParseLSMChannels(static_cast<long>(paramValue),NULL,NULL,L""))
		{
			imageInfo.channels = 1;
		}
		else
		{
			//check if we're dealing with a multi-channel board
			imageInfo.channels = ParseLSMChannels(static_cast<long>(paramMax),NULL,NULL,L"");
		}


		double k1,k2,k3,k4;
		CaptureSetup::getInstance()->GetPincushionCoefficients(k1,k2,k3,k4);


		ImageCorrections(pMemoryBuffer, static_cast<long>(width), static_cast<long>(height), imageInfo.channels,
			CaptureSetup::getInstance()->_enablePincushionCorrection,
			CaptureSetup::getInstance()->_enableBackgroundSubtraction,
			CaptureSetup::getInstance()->_enableFlatField,
			k1,k2,k3,k4
			);

		imageInfo.imageWidth = static_cast<long>(width);
		imageInfo.imageHeight = static_cast<long>(height);
		(*myFunctionPointer)(pChan[0],imageInfo);		

		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition callback complete");
		_enableCopy = FALSE;
	}

	return TRUE;

}

long ImageRoutineLSM::GetImageDimensions(long &width, long &height)
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


	double areaMode = 0;
	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_AREAMODE,areaMode))
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup GetImageDimensions AREAMODE failed");
	}

	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_X,right))
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup GetImageDimensions PIXEL_X failed");
	}

	switch(static_cast<long>(areaMode))
	{
	case ICamera::SQUARE:
		{
			//square
			bottom = right;
		}
		break;
	case ICamera::RECTANGLE:
	case ICamera::LINE:
	case ICamera::POLYLINE:
		{
			if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_Y,bottom))
			{	
				logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup GetImageDimensions PIXEL_X failed");
			}
		}
		break;					
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
long CaptureSingleImageWithAverageLSM(char *buffer, double exposureTime, long binningX, long binningY , long avgFrames, long snapshotFlag)
{
	if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_AVERAGEMODE,ICamera::AVG_MODE_NONE))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage SetParam PARAM_LSM_AVERAGEMODE failed");
		logDll->TLTraceEvent(WARNING_EVENT,1,message);
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

	imageInfo.numberOfPlanes = 1; // assume 1 and let the camera say otherwise if it supports multiplane

	long imageID;

	if(FALSE == ImageManager::getInstance()->CreateImage(imageID,dim))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage CreateImage failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
		return FALSE;
	}

	//pAvgBuffer: image buffer from camera, reuse the first time point
	char * pAvgBuffer = ImageManager::getInstance()->GetImagePtr(imageID,0,0,0,0);

	double k1,k2,k3,k4;
	CaptureSetup::getInstance()->GetPincushionCoefficients(k1,k2,k3,k4);

	double paramValue;

	if(GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_CHANNEL,paramValue) != TRUE)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition GetParam failed PARAM_LSM_CHANNEL");
		return FALSE;
	}

	double width,height;
	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_X,width))	
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition GetParam failed PARAM_LSM_PIXEL_X");
		return FALSE;
	}

	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_PIXEL_Y,height))	
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup CopyAcquisition GetParam failed PARAM_LSM_PIXEL_Y");
		return FALSE;
	}

	imageInfo.channels = ParseLSMChannels(static_cast<long>(paramValue),NULL,NULL,L"");
	imageInfo.fullFrame = TRUE;

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

	imageInfo.imageWidth = static_cast<long>(width);
	imageInfo.imageHeight = static_cast<long>(height);

	//pSumBuffer: accumulated frame buffer at ready status
	unsigned long * pSumBuffer = new unsigned long[imageInfo.imageWidth*imageInfo.imageHeight*imageInfo.channels];
	memset(pSumBuffer,0,sizeof(unsigned long) * imageInfo.imageWidth*imageInfo.imageHeight*imageInfo.channels);

	for(long i=0; i<avgFrames; i++)
	{
		long status = ICamera::STATUS_BUSY;

		while(status == ICamera::STATUS_BUSY)
		{
			if((TRUE == stopCapture) || (FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->StatusAcquisition(status)))
			{
				break;
			}
			if(ICamera::STATUS_PARTIAL == status)
			{
				if(FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->CopyAcquisition(pAvgBuffer, &imageInfo))
				{
					StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage CopyAcquisition failed");
					logDll->TLTraceEvent(ERROR_EVENT,1,message);
					break;
				}

				//wait until next section started
				while(ICamera::STATUS_PARTIAL == status)
				{
					Sleep(1);
					if((TRUE == stopCapture) || (FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->StatusAcquisition(status)))
					{
						break;
					}
				}

				//try update display buffer
				if((FALSE == stopCapture) && (ImageRoutineLSM::_enableCopy))
				{
					//prepare buffer for display, averaged by current number of frames
					unsigned short * pTgt = (unsigned short *)buffer;
					unsigned long * pSum = pSumBuffer;
					unsigned short * pImg = (unsigned short *)pAvgBuffer;
					for(long k=0; k<imageInfo.imageWidth*imageInfo.imageHeight*imageInfo.channels; k++)
					{
						*pTgt = static_cast<unsigned short>(((*pSum)+(*pImg))/(double)(i+1));
						pTgt++;
						pSum++;
						pImg++;
					}

					ImageCorrections(buffer, imageInfo.imageWidth, imageInfo.imageHeight, imageInfo.channels,
						CaptureSetup::getInstance()->_enablePincushionCorrection,
						CaptureSetup::getInstance()->_enableBackgroundSubtraction,
						CaptureSetup::getInstance()->_enableFlatField,
						k1,k2,k3,k4
						);

					(*myFunctionPointer)(buffer,imageInfo);	//buffer: buffer to display
					ImageRoutineLSM::_enableCopy = FALSE;
				}

				//check status again since it may take a while to prepare display buffer
				if((TRUE == stopCapture) || (FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->StatusAcquisition(status)))
				{
					break;
				}
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
			ImageManager::getInstance()->UnlockImagePtr(imageID,0,0,0,0);		
			ImageManager::getInstance()->DestroyImage(imageID);
			delete[] pSumBuffer;
			return FALSE;
		}

		//image is ready, append to pSumBuffer, 
		//display buffer averaged by current number of frames
		unsigned short * pTgt = (unsigned short *)buffer;
		unsigned long * pSum = pSumBuffer;
		unsigned short * pImg = (unsigned short *)pAvgBuffer;
		for(long k=0; k<imageInfo.imageWidth*imageInfo.imageHeight*imageInfo.channels; k++)
		{
			*pSum += *pImg;
			*pTgt = static_cast<unsigned short>((*pSum)/(double)(i+1));
			pTgt++;
			pSum++;
			pImg++;
		}

		//try update display buffer
		if(ImageRoutineLSM::_enableCopy)
		{
			ImageCorrections(buffer, imageInfo.imageWidth, imageInfo.imageHeight, imageInfo.channels,
				CaptureSetup::getInstance()->_enablePincushionCorrection,
				CaptureSetup::getInstance()->_enableBackgroundSubtraction,
				CaptureSetup::getInstance()->_enableFlatField,
				k1,k2,k3,k4
				);

			(*myFunctionPointer)(buffer,imageInfo);
			ImageRoutineLSM::_enableCopy = FALSE;
		}
	}

	ImageManager::getInstance()->UnlockImagePtr(imageID,0,0,0,0);		
	ImageManager::getInstance()->DestroyImage(imageID);
	delete[] pSumBuffer;

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

UINT SnapshotThreadProcLSM(LPVOID pParam)
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
	long avgFrames=1;
	long avgMode = 0;

	if(TRUE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_AVERAGEMODE, avgMode) && (ICamera::AVG_MODE_CUMULATIVE == avgMode))
	{
		if(FALSE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_AVERAGENUM, avgFrames))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Snapshot Get PARAM_LSM_AVERAGENUM failed.");
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
		}
	}

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
	if(FALSE == CaptureSingleImageWithAverageLSM(pChan[0], exposureTime, binX, binY, avgFrames, isSnapshotFlag))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Snapshot CaptureSingleImage failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
	}

	//set back averageMode
	if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_AVERAGEMODE, avgMode))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Snapshot SetParam PARAM_LSM_AVERAGEMODE failed");
		logDll->TLTraceEvent(WARNING_EVENT,1,message);
	}

	//set back averageFrames
	if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_AVERAGENUM,static_cast<double>(avgFrames)))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage SetParam PARAM_MULTI_FRAME_COUNT failed");
		logDll->TLTraceEvent(WARNING_EVENT,1,message);
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

UINT ZStackCaptureThreadProcLSM( LPVOID pParam )
{	
	struct CaptureSetupZCaptureParams zParams;

	if(NULL != pParam)
	{
		memcpy(&zParams,pParam,sizeof(struct CaptureSetupZCaptureParams));
		delete pParam;
	}
	else
	{
		CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));
		disableZRead = FALSE;
		SetEvent(hCaptureActive);
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
	double fineAutoFocusPercentage = 0.15;
	pHardware->GetMagInfoFromPosition(turretPos,objName,magnification,numAperture,afStartPos,afFocusOffset,afAdaptiveOffset,beamExpPos,beamExpWavelength,beamExpPos2,beamExpWavelength2, turretPosition, zAxisToEscape, zAxisEscapeDistance, fineAutoFocusPercentage);

	long width, height;
	double umPerPixel = 1;

	width = static_cast<long>(lsmPixelX);
	height = static_cast<long>(lsmPixelY);

	double fieldSizeCalibration = 100.0;

	//resolve legacy: if field size calibration value is not available from Confocal, use the one in HardwareSettings.xml
	if(FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION, fieldSizeCalibration))
	{
		pHardware->GetLSM(fieldSizeCalibration);
	}
	long fieldSize;
	GetFieldSize(fieldSize);
	umPerPixel = (fieldSize * fieldSizeCalibration)/(width * magnification);

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
	std::string strOME;
	std::wstringstream imgNameFormat;
	std::vector<wstring> wavelengthNames;

	if(TRUE != GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_CHANNEL,paramValue) && 
		TRUE != GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_CHANNEL,paramValue))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup ZStackCaptureProc GetParam failed PARAM_LSM_CHANNEL");
		goto RETURN_TASK;
	}
	long lsmChan = static_cast<long>(paramValue);
	long chanCount = ParseLSMChannels(lsmChan,NULL,&wavelengthNames,L"Chan");
	long doOME = TRUE;
	long doCompression = TRUE;
	GetTIFFConfiguration(doOME, doCompression);

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	imgNameFormat << L"%s%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";

	strOME = (TRUE == doOME) ? uUIDSetup(wavelengthNames, true, chanCount, zParams.numOfSteps, 1, 1) : "";

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
		if(FALSE == CaptureSingleImageWithAverageLSM(pChan[0], exposureTime, binX, binY, avgFrames, snapshotFlag))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup ZStackCaptureProc CaptureSingleImage failed");
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
			goto RETURN_TASK;
		}

		//get the number of total channels
		long paramLong;
		double paramDouble;
		double paramMax;

		GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_LSM_CHANNEL,paramLong,paramLong,paramLong,paramDouble,paramMax,paramDouble);

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

	//disable Laser Emission
	SetLaser1Emission(DISABLE_EMISSION);
	SetLaser2Emission(DISABLE_EMISSION);
	SetLaser3Emission(DISABLE_EMISSION);
	SetLaser4Emission(DISABLE_EMISSION);

	PostflightPMT();

	//return to start z position after z-stack capture finished
	if (FALSE == stopCapture)
	{
		if(FALSE == SetZPosition(zParams.start))
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"CaptureZStack Execute SetZPosition failed ");
		}	
	}
	goto RETURN_TASK;

RETURN_TASK:
	CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));
	disableZRead = FALSE;
	SetEvent(hCaptureActive);
	return 0;
}

long ImageRoutineLSM::StartAutoFocus(double magnification, long autoFocusType, BOOL& afFound)
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
		_hAutoFocusCaptureThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoFocusCaptureThreadProcLSM, (LPVOID)afParams, 0, &_dwAutoFocusCaptureThreadId);
		SetThreadPriority(_hAutoFocusCaptureThread, THREAD_PRIORITY_BELOW_NORMAL);
		_hAutoFocusStatusThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AutoFocusStatusThreadProcLSM, NULL, 0, &_dwAutoFocusStatusThreadId);
		SetThreadPriority(_hAutoFocusStatusThread, THREAD_PRIORITY_BELOW_NORMAL);
	}

	return TRUE;
}

//Start the autofocus routine, it needs to be on it's own thread to run the library AutoFocusModule without hanging the GUI update
UINT AutoFocusCaptureThreadProcLSM(LPVOID pParam)
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
UINT AutoFocusStatusThreadProcLSM()
{
	long imageReadyToCopy = FALSE, autoFocusRunning = FALSE;
	long frameNumber = 0, currentRepeat = 0, status = 0, numOfZSteps = 0, currentZIndex = 0;
	int frameNum = 0;

	//shared parameters with live image mode
	long right = 0, bottom = 0, left = 0, top = 0, binX = 0, binY = 0, lsmPixelX = 0, lsmPixelY = 0;
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAPTURE_REGION_RIGHT, right);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAPTURE_REGION_LEFT, left);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAPTURE_REGION_BOTTOM, bottom);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAPTURE_REGION_TOP, top);
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

	long width, height;
	double umPerPixel = 1;

	width = static_cast<long>(lsmPixelX);
	height = static_cast<long>(lsmPixelY);

	double fieldSizeCalibration = 100.0;

	//resolve legacy: if field size calibration value is not available from Confocal, use the one in HardwareSettings.xml
	if (FALSE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION, fieldSizeCalibration))
	{
		pHardware->GetLSM(fieldSizeCalibration);
	}
	long fieldSize;
	GetFieldSize(fieldSize);
	umPerPixel = (fieldSize * fieldSizeCalibration) / (width * magnification);

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

	imageInfo.channels = ParseLSMChannels(static_cast<long>(paramValue), NULL, NULL, L"");
	imageInfo.fullFrame = TRUE;

	imageInfo.imageWidth = static_cast<long>(width);
	imageInfo.imageHeight = static_cast<long>(height);

	strOME = ""; //(TRUE == doOME) ? uUIDSetup(wavelengthNames, true, chanCount, zParams.numOfSteps, 1, 1) : "";
	
	//wait until Auto Focus has started or timeout at 200ms
	clock_t nextUpdateLoop = clock();
	while (static_cast<unsigned long>(abs(nextUpdateLoop - clock()) / (CLOCKS_PER_SEC / 1000)) < 200 && FALSE == autoFocusRunning)
	{
		GetAutoFocusStatusAndImage(pChan[0], imageReadyToCopy, autoFocusRunning, frameNumber, currentRepeat, status, numOfZSteps, currentZIndex);
	}
	if (FALSE == autoFocusRunning) // if auto focus is still not running, throw an error message to the log
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"CaptureSetup AutoFocusStatusThreadProcLSM Failed to start autofocus. AutoFocusCaptureThreadProcLSM didn't call RunAutofocus in time. Timed out after 200ms.");
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

			GetCamera(SelectedHardware::SELECTED_CAMERA1)->GetParamInfo(ICamera::PARAM_LSM_CHANNEL, paramLong, paramLong, paramLong, paramDouble, paramMax, paramDouble);

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

			//try update display buffer
			(*myFunctionPointer)(pChan[0], imageInfo);
			ImageRoutineLSM::_enableCopy = FALSE;

			frameNum++;
		}
		GetAutoFocusStatusAndImage(pChan[0], imageReadyToCopy, autoFocusRunning, frameNumber, currentRepeat, status, numOfZSteps, currentZIndex);
		//pos += zParams.stepSize / (double)Constants::UM_TO_MM;
	}

	return TRUE;
}


