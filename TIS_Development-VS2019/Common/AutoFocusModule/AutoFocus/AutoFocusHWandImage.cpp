#include "stdafx.h"
#include "..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "AutoFocusHWandImage.h"


extern auto_ptr<CommandDll> shwDll;
extern auto_ptr<TiffLibDll> tiffDll;

atomic_long AutoFocusHWandImage::_stopFlag = FALSE;

UINT StatusHWandImageAutoFocusThreadProc( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;

	IDevice * pDevice = (IDevice*)pParam;

	while(status == IDevice::STATUS_BUSY)
	{
		if(FALSE == pDevice->StatusPosition(status))
		{
			break;
		}
	}

	SetEvent( AutoFocusHWandImage::hEventAutoFocus);

	return 0;
}

UINT StatusHWandImageZThreadProc( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;

	IDevice * pDevice = (IDevice*)pParam;

	while(status == IDevice::STATUS_BUSY)
	{
		if(FALSE == pDevice->StatusPosition(status))
		{
			break;
		}
	}

	SetEvent( AutoFocusHWandImage::hEventZ);

	return 0;
}


UINT StatusHWandImageCameraThreadProc( LPVOID pParam )
{
	long status = ICamera::STATUS_BUSY;

	ICamera * pCamera = (ICamera*)pParam;

	while(status == ICamera::STATUS_BUSY)
	{
		if(FALSE == pCamera->StatusAcquisition(status))
		{
			break;
		}
	}

	SetEvent( AutoFocusHWandImage::hEventCamera);

	return 0;
}


AutoFocusHWandImage::AutoFocusHWandImage()
{
	_binning = 1;
	_counter = 0;
	_expTimeMS = 30;
	_focusOffset = 0;
	_height = 512;
	_pCamera = NULL;
	_pZStage = NULL;
	_repeat = 0;
	_startPosMM = 0;
	_stepSizeUM = 0;
	_stopPosMM = 0;
	_width = 512;
	_imageReady = FALSE;
	_imageBuffer = NULL;
	_autoFocusStatus = AutoFocusStatusTypes::NOT_RUNNING;
	_bestContrastScore = 0;
	_bestZPositionFound = 0;
	_nextZPosition = 0;
	_zSteps = 0;
	_frameNumber = 0;
	_currentZIndex = -1;
	_finePercentageDecrease = 0.15;
	_enableGUIUpdate = FALSE;
	_numberOfChannels = 1;
}

HANDLE AutoFocusHWandImage::hEventAutoFocus = NULL;

HANDLE AutoFocusHWandImage::hEventCamera = NULL;

HANDLE AutoFocusHWandImage::hEventZ = NULL;

void AutoFocusHWandImage::SetupParameters(ICamera * pCamera, IDevice * pZStage,long repeat, double focusOffset, double expTime, double stepSizeUM,double startPosMM, double stopPosMM,long binning, double finePercentage, long enableGUIUpdate)
{
	_pCamera = pCamera;
	_pZStage = pZStage;
	_expTimeMS = expTime;
	_stepSizeUM = stepSizeUM;
	_startPosMM = startPosMM;
	_stopPosMM = stopPosMM;
	_repeat = repeat;
	_focusOffset = focusOffset;
	_counter = 0;
	_binning = binning;
	_enableGUIUpdate = enableGUIUpdate;
}

long AutoFocusHWandImage::WillAFExecuteNextIteration()
{
	if ((_counter % _repeat) != 0)
	{	
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

long AutoFocusHWandImage::Execute(long index, IDevice * pAutoFocus, BOOL &afFound)
{	
	_autoFocusStatus = AutoFocusStatusTypes::NOT_RUNNING;
	_frameNumber = 0;
	_stopFlag = FALSE;

	//the first image will always execute autofocus with (counter == 0) 
	if ((_counter % _repeat) != 0)
	{	
		_counter++;
		return TRUE;
	}
	_counter++;

	_autoFocusStatus = AutoFocusStatusTypes::HARDWARE_AUTOFOCUS;
	
	pAutoFocus->SetParam(IDevice::PARAM_AUTOFOCUS_OFFSET,_focusOffset);

	//enable autofocus
	pAutoFocus->SetParam(IDevice::PARAM_AUTOFOCUS_POS, 1.0);

	pAutoFocus->PreflightPosition();

	pAutoFocus->SetupPosition ();

	pAutoFocus->StartPosition();

	hEventAutoFocus = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThread;

	HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusHWandImageAutoFocusThreadProc, pAutoFocus, 0, &dwThread );

	const long MAX_AF_WAIT_TIME = 10000;

	DWORD dwWait = WaitForSingleObject( hEventAutoFocus, MAX_AF_WAIT_TIME);

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"AutoFocusHWandImage Execute HW AutoFocus failed");
		//return FALSE;
	}		

	CloseHandle(hThread);
	CloseHandle(hEventAutoFocus);

	pAutoFocus->PostflightPosition();	

	double param=0;

	pAutoFocus->GetParam(IDevice::PARAM_AUTOFOCUS_FOUND,param);
	
	afFound = static_cast<int>(param);

	if(FALSE == afFound)
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"AutoFocusHWandImage HW focus failed");
		return FALSE;
	}

	//begin image based autofocus
	long zParamType;
	long zParamAvailable;
	long zParamReadOnly;
	double zParamMin;
	double zParamMax;
	double zParamDefault;

	_pZStage->GetParamInfo(IDevice::PARAM_Z_POS_CURRENT, zParamType, zParamAvailable, zParamReadOnly, zParamMin, zParamMax, zParamDefault);

	double focusPosition;
	pAutoFocus->GetParam(IDevice::PARAM_Z_POS_CURRENT,focusPosition);

	StringCbPrintfW(msg,MSG_LENGTH,L"AutoFocusHWandImage HW Focus Location %d.%03d",static_cast<long>(focusPosition),abs(static_cast<long>((focusPosition - static_cast<long>(focusPosition))*1000)));
	logDll->TLTraceEvent(VERBOSE_EVENT,1, msg);

	double start = max(_startPosMM + focusPosition, zParamMin);
	double stop = min(_stopPosMM + focusPosition, zParamMax);
	double step = _stepSizeUM / 1000;

	StringCbPrintfW(msg,MSG_LENGTH,L"AutoFocusHWandImage Start %d.%03d Stop %d.%03d Step %d.%03d",static_cast<long>(start),abs(static_cast<long>((start - static_cast<long>(start))*1000)),static_cast<long>(stop),abs(static_cast<long>((stop - static_cast<long>(stop))*1000)),static_cast<long>(step),abs(static_cast<long>((step - static_cast<long>(step))*1000)));
	logDll->TLTraceEvent(VERBOSE_EVENT,1, msg);

	//verify parameters
	if(start < stop)
	{
		if(step <= 0)
		{
			logDll->TLTraceEvent(ERROR_EVENT,1,L"AutoFocusHWandImage AutoFocusImageBased failed stepsize must be positive");
			return FALSE;
		}
	}
	else
	{
		if(step >= 0)
		{
			logDll->TLTraceEvent(ERROR_EVENT,1,L"AutoFocusHWandImage AutoFocusImageBased failed stepsize must be negative");
			return FALSE;
		}
	}

	
	double left, right, top, bottom, binX, binY, val, pixelX, pixelY, bitDepth, numChannels;
	long width = 0, height = 0, cameraType = ICamera::CameraType::LSM, channel = 1;
	_numberOfChannels = 1;

	_pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE, val);
	cameraType = static_cast<long>(val);
	wchar_t message[MSG_LENGTH];

	if (ICamera::CameraType::LSM == cameraType)
	{
		if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_AVERAGEMODE, ICamera::AVG_MODE_NONE))
		{
			StringCbPrintfW(message, MSG_LENGTH, L"AutoFocusModule AutoFocusHWandImage SetParam PARAM_LSM_AVERAGEMODE failed");
			logDll->TLTraceEvent(ERROR_EVENT, 1, message);
		}

		if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_MULTI_FRAME_COUNT, 1.0))
		{
			StringCbPrintfW(message, MSG_LENGTH, L"AutoFocusModule AutoFocusHWandImage SetParam PARAM_MULTI_FRAME_COUNT failed");
			logDll->TLTraceEvent(ERROR_EVENT, 1, message);
			return FALSE;
		}

		_pCamera->GetParam(ICamera::PARAM_LSM_PIXEL_X, pixelX);
		_pCamera->GetParam(ICamera::PARAM_LSM_PIXEL_Y, pixelY);
		_pCamera->GetParam(ICamera::PARAM_LSM_CHANNEL, numChannels);

		_width = static_cast<long>(pixelX);
		_height = static_cast<long>(pixelY);
		channel = static_cast<long>(numChannels);

		switch (channel)
		{
		case 0x1:_numberOfChannels = 1; break;
		case 0x2:_numberOfChannels = 1; break;
		case 0x4:_numberOfChannels = 1; break;
		case 0x8:_numberOfChannels = 1; break;
		default:
		{
			long paramType;
			long paramAvailable;
			long paramReadOnly;
			double paramMin;
			double paramMax;
			double paramDefault;

			_pCamera->GetParamInfo(ICamera::PARAM_LSM_CHANNEL, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
			switch (static_cast<long>(paramMax))
			{
			case 0x3:_numberOfChannels = 3; break;
			case 0xF:_numberOfChannels = 4; break;
			default:_numberOfChannels = 3;
			}
		}
		break;
		}
	}
	else if (ICamera::CameraType::CCD == cameraType)
	{
		if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_AVERAGENUM, 1.0))
		{
			StringCbPrintfW(message, MSG_LENGTH, L"AutoFocusModule AutoFocusHWandImage SetParam PARAM_MULTI_FRAME_COUNT failed");
			logDll->TLTraceEvent(ERROR_EVENT, 1, message);
			return FALSE;
		}

		if (FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_MULTI_FRAME_COUNT, 1.0))
		{
			StringCbPrintfW(message, ERROR_EVENT, L"AutoFocusModule AutoFocusHWandImage SetParam PARAM_MULTI_FRAME_COUNT failed");
			logDll->TLTraceEvent(WARNING_EVENT, 1, message);
			return FALSE;
		}

		_pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_LEFT, left);
		_pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT, right);
		_pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_TOP, top);
		_pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM, bottom);
		_pCamera->GetParam(ICamera::PARAM_BINNING_X, binX);
		_pCamera->GetParam(ICamera::PARAM_BINNING_Y, binY);

		_width = static_cast<long>((right - left) / binX);
		_height = static_cast<long>((bottom - top) / binY);
	}

	_pCamera->GetParam(ICamera::PARAM_BITS_PER_PIXEL, bitDepth);

	Dimensions d;

	d.c = _numberOfChannels;
	d.dType = INT_16BIT;
	d.m = 1;
	d.mType = CONTIGUOUS_CHANNEL_MEM_MAP;
	d.t = 1;
	d.x = width;
	d.y = height;
	d.z = 1;

	long imageID;

	if(ImageManager::getInstance()->CreateImage(imageID,d)== FALSE)
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"AutoFocusHWandImage could not create memory buffer");
		return FALSE;
	}

	_imageBuffer = ImageManager::getInstance()->GetImagePtr(imageID,0,0,0,0);

	double loc;
	long score =0;
	long highScore=0;
	//start with a focus point at the current position
	double focusLoc = focusPosition;
	const long PIXEL_SKIP_SIZE = 16;

	_currentZIndex = -1;
	_autoFocusStatus = AutoFocusStatusTypes::COARSE_AUTOFOCUS;
	_zSteps = (long)(abs((stop - start) / step) + 1);

	if(step > 0) //currently step is always > 0
	{
		for(loc = start; loc<= stop; loc += step)
		{			
			// Check if stop was called, exit before moving Z
			if (TRUE == _stopFlag)
			{
				_autoFocusStatus = AutoFocusStatusTypes::STOPPED;
				logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"AutoFocusHWandImage AutoFocusImageBased Stopped");
				return TRUE;
			}

			SetZPosition(loc);
			_currentZIndex++;

			//take picture
			if(FALSE == CaptureSingleImage(_imageBuffer,_expTimeMS))
			{
				logDll->TLTraceEvent(ERROR_EVENT,1,L"AutoFocusHWandImage AutoFocusImageBased CaptureSingleImage failed");
				return FALSE;
			}

			ContrastScore(_imageBuffer, width, height, PIXEL_SKIP_SIZE, score);
			
			StringCbPrintfW(msg,MSG_LENGTH,L"AutoFocusHWandImage ContrastScore %d",score);
			logDll->TLTraceEvent(VERBOSE_EVENT,1, msg);

			if (score > highScore)
			{
				_bestZPositionFound = focusLoc = loc;
				_bestContrastScore = highScore = score;
			}
			if (loc + step <= stop)
			{
				_nextZPosition = loc + step;
			}
			
			//Give the GUI time to retrive the image
			while (TRUE == _imageReady && TRUE == _enableGUIUpdate)
			{
				Sleep(1);
				if (TRUE == _stopFlag)
				{
					_autoFocusStatus = AutoFocusStatusTypes::STOPPED;
					logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"AutoFocusImage AutoFocusImageBased Stopped");
					return TRUE;
				}
			}
		}
	}
	else
	{
		for(loc = start; loc>= stop; loc += step)
		{
			// Check if stop was called, exit before moving Z
			if (TRUE == _stopFlag)
			{
				_autoFocusStatus = AutoFocusStatusTypes::STOPPED;
				logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"AutoFocusHWandImage AutoFocusImageBased Stopped");
				return TRUE;
			}

			SetZPosition(loc);
			_currentZIndex++;

			//take picture
			if(FALSE == CaptureSingleImage(_imageBuffer,_expTimeMS))
			{
				logDll->TLTraceEvent(ERROR_EVENT,1,L"AutoFocusHWandImage AutoFocusImageBased CaptureSingleImage failed");
				return FALSE;
			}
			
			ContrastScore(_imageBuffer, width, height, PIXEL_SKIP_SIZE, score);

			StringCbPrintfW(msg,MSG_LENGTH,L"AutoFocusHWandImage ContrastScore %d",score);
			logDll->TLTraceEvent(VERBOSE_EVENT,1, msg);

			if (score > highScore)
			{
				_bestZPositionFound = focusLoc = loc;
				_bestContrastScore = highScore = score;
			}
			if (loc + step >= stop)
			{
				_nextZPosition = loc + step;
			}
			
			//Give the GUI time to retrive the image
			while (TRUE == _imageReady && TRUE == _enableGUIUpdate)
			{
				Sleep(1);
				if (TRUE == _stopFlag)
				{
					_autoFocusStatus = AutoFocusStatusTypes::STOPPED;
					logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"AutoFocusImage AutoFocusImageBased Stopped");
					return TRUE;
				}
			}
		}		
	}

	StringCbPrintfW(msg,MSG_LENGTH,L"AutoFocusHWandImage Image Focus Location %d.%03d",static_cast<long>(focusLoc),abs(static_cast<long>((focusLoc - static_cast<long>(focusLoc))*1000)));
	logDll->TLTraceEvent(VERBOSE_EVENT,1, msg);
	
	// Check if stop was called, exit before moving Z
	if (TRUE == _stopFlag)
	{
		_autoFocusStatus = AutoFocusStatusTypes::STOPPED;
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"AutoFocusHWandImage AutoFocusImageBased Stopped");
		return TRUE;
	}

	SetZPosition(focusLoc);

	if (TRUE == _enableGUIUpdate)
	{
		_currentZIndex++;	//save the frame acquired in the best Z location as the last frame in the Fine folder 
		//take one more image at focus location for GUI update
		if (FALSE == CaptureSingleImage(_imageBuffer, _expTimeMS))
		{
			logDll->TLTraceEvent(ERROR_EVENT, 1, L"AutoFocusHWandImage AutoFocusImageBased CaptureSingleImage failed");
			return FALSE;
		}

		//Give the GUI time to retrive the image
		while (TRUE == _imageReady)
		{
			Sleep(1);
			if (TRUE == _stopFlag)
			{
				_autoFocusStatus = AutoFocusStatusTypes::STOPPED;
				logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"AutoFocusImage AutoFocusImageBased Stopped");
				return TRUE;
			}
		}
	}

	ImageManager::getInstance()->DestroyImage(imageID);

	return TRUE;
}

long AutoFocusHWandImage::SetZPosition(double pos)
{	
	_pZStage->SetParam(IDevice::PARAM_Z_POS,pos);
	_pZStage->PreflightPosition();
	_pZStage->SetupPosition();
	_pZStage->StartPosition();

	hEventZ = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThreadId;

	HANDLE hThread0;

	hThread0 = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusHWandImageZThreadProc, (LPVOID)_pZStage, 0, &dwThreadId );

	DWORD dwWait = WaitForSingleObject(hEventZ, INFINITE);

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"AutoFocusHWandImage SetZPosition WaitForSingleObject failed");
		return FALSE;
	}

	CloseHandle(hThread0);

	CloseHandle(hEventZ);

	_pZStage->PostflightPosition();

	return TRUE;
}

long AutoFocusHWandImage::CaptureSingleImage( char *buffer,double expTime)
{
	FrameInfo frameInfo = {0, 0, 0, 0};
	/*if(FALSE == _pCamera->SetParam(ICamera::PARAM_EXPOSURE_TIME_MS,expTime))
	{
		return FALSE;
	}*/

	if(FALSE == _pCamera->PreflightAcquisition(buffer))
	{
		return FALSE;
	}

	if(FALSE == _pCamera->SetupAcquisition(buffer))
	{
		return FALSE;
	}
	
	if(FALSE == _pCamera->StartAcquisition(buffer))
	{
		return FALSE;
	}
	
	hEventCamera = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThreadId;

	HANDLE hThread0;

	hThread0 = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusHWandImageCameraThreadProc, (LPVOID)_pCamera, 0, &dwThreadId );

	DWORD dwWait = WaitForSingleObject(hEventCamera, INFINITE);

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"AutoFocusHWandImage CaptureSingleImage WaitForSingleObject failed");
		return FALSE;
	}

	CloseHandle(hThread0);	
	CloseHandle(hEventCamera);

	if(FALSE == _pCamera->CopyAcquisition(buffer, &frameInfo))
	{
		return FALSE;
	}

	_imageReady = TRUE;
	_frameNumber++;

	ResetEvent(hEventCamera); //:TODO: see if this is necessary
	
	if(FALSE == _pCamera->PostflightAcquisition(NULL))
	{
		return FALSE;
	}

	return TRUE;

}

void AutoFocusHWandImage::ContrastScore(char * pBuffer, long width, long height, long skipSize, long &score)
{
	unsigned short high1;//highest value in the kernel
	unsigned short high2;
	unsigned short high3;
	unsigned short low1;//lowest value in the kernel
	unsigned short low2;
	unsigned short low3;

	unsigned short * tempPtr;
	unsigned short * imageBuffer = (unsigned short*)pBuffer;

	long      fx,fy;

	int filterWidth = 3;
	int filterHeight = 3;

	long fWidth2,fHeight2;

	fWidth2 = (filterWidth-1)/2;
	fHeight2 = (filterHeight-1)/2;

	unsigned short *dataArr; 

	dataArr = new unsigned short[filterWidth*filterHeight]; 

	long sum = 0;
	unsigned short *startPtr = imageBuffer;

	for(int y=fHeight2;y<(height-fHeight2);y+=skipSize)
	{
		for(int x=fWidth2;x<(width-fWidth2);x+=skipSize) 
		{
			startPtr = imageBuffer + (y*width) + x;
			high1 = *startPtr;
			high2 = *startPtr;
			high3 = *startPtr;
			low1 = *startPtr;
			low2 = *startPtr;
			low3 = *startPtr;

			for(fy=0;fy<filterHeight;fy++)
			{
				tempPtr = startPtr;
				tempPtr +=  (-fWidth2) + ((fy-fHeight2)*(width));

				for(fx=0;fx<filterWidth;fx++)
				{
					if(*tempPtr > high1)
					{
						high3 = high2;
						high2 = high1;
						high1 = *tempPtr;
					}
					if((high1 > *tempPtr) &&(*tempPtr > high2))
					{
						high3 = high2;
						high2 = *tempPtr;
					}

					if(*tempPtr < low1)
					{
						low3 = low2;
						low2 = low1;
						low1 = *tempPtr;
					}
					if((low1 > *tempPtr) && (*tempPtr < low2))
					{
						low3 = low2;
						low2 = *tempPtr;
					}
					tempPtr++;
				}
			}

			//use the third highest/lowest values for the contrast score
			sum += (high3-low3);
		}
	}

	score = sum;
}

// Returns true if an image is ready and there is a buffer to copy, otherwise returns false as a signal that there is no image to be copied
long AutoFocusHWandImage::GetImageBuffer(char* pBuffer, long& frameNumber, long& currentRepeat, long& status, long& zSteps, long& currentZIndex)
{
	if (TRUE == _imageReady)
	{
		memcpy(pBuffer, _imageBuffer, static_cast<unsigned long long>(_width) * static_cast<unsigned long long>(_height) * sizeof(unsigned short) * _numberOfChannels);
		_imageReady = FALSE;
		frameNumber = _frameNumber;
		currentRepeat = _counter;
		status = _autoFocusStatus;
		zSteps = _zSteps;
		currentZIndex = _currentZIndex;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void AutoFocusHWandImage::SetStopFlag(long stopValue)
{
	_stopFlag = stopValue;
}

void AutoFocusHWandImage::GetStatus(long& currentStatus, long& bestContrastScore, double& bestZPosition, double& nextZPosition, long& currentRepeatIndex)
{
	currentStatus = _autoFocusStatus;
	bestContrastScore = _bestContrastScore;
	bestZPosition = _bestZPositionFound;
	nextZPosition = _nextZPosition;
	currentRepeatIndex = _counter;
}
