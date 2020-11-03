#include "stdafx.h"
#include "DCxCamera.h"
#include <iostream>
#include <fstream>

HANDLE DCxCamera::eventHandle = NULL;
long DCxCamera::camera_status = ICamera::STATUS_BUSY;
unique_ptr<IPPIDll> ippiDll(new IPPIDll(L".\\ippiu8-7.0.dll"));

DCxCamera::DCxCamera(int camera, const char* serialNo)
{
	_cameraID = camera;
	channel = -1;
	if (serialNo != NULL)
	{
		memset(_serialNo, 0, SERIAL_NUMBER_MAX_LENGHT);
		memcpy(_serialNo, serialNo, SERIAL_NUMBER_MAX_LENGHT);
	}
}

DCxCamera::~DCxCamera()
{
	ExitCamera();
	memset(_serialNo, 0, SERIAL_NUMBER_MAX_LENGHT);
	_cameraID = -1;
	channel = -1;
}

long DCxCamera::GetSerialNO(char* serialNo)
{ 
	memcpy(serialNo, _serialNo, SERIAL_NUMBER_MAX_LENGHT);
	return TRUE;
} 

long DCxCamera::GetSensorInfo(SENSORINFO *senserInfo)
{
	*senserInfo = imgPtyDll.sensorInfo;
	return TRUE;
}

long DCxCamera::OpenCamera()
{
	ExitCamera();

	// init camera
	return camera.InitCamera(_cameraID);
}

long DCxCamera::InitCamera()
{
	if( !camera.IsInit() ) {
		return IS_NO_SUCCESS;
	}

	if (IS_SUCCESS != GetDefaultParameters() || IS_SUCCESS != SetDefaultParameters() || IS_SUCCESS != camera.SetDisplayMode(IS_SET_DM_DIB)) 
		return IS_NO_SUCCESS;

	if( imgPtyDll.sensorInfo.nColorMode == IS_COLORMODE_BAYER || imgPtyDll.sensorInfo.nColorMode == IS_COLORMODE_CBYCRY)
	{
		imgPtyDll.colorMode = IS_SET_CM_RGB24;
		imgPtyDll.bitsPerPixel = 24;
		imgPtyDll.channel = 0x111;
	}
	else
	{
		imgPtyDll.colorMode = IS_SET_CM_Y8;
		imgPtyDll.bitsPerPixel = 8;
		imgPtyDll.channel = 0x1;
	}

	if (IS_SUCCESS != ReAllocImageMemory())
		return IS_NO_SUCCESS;

	// set the desired color mode , set auto white balance
	if (IS_SUCCESS != camera.SetColorMode(imgPtyDll.colorMode))
		return IS_NO_SUCCESS;

	int ret = camera.SetAutoParameter(IS_SET_ENABLE_AUTO_WHITEBALANCE);
	if (IS_SUCCESS != ret && IS_NOT_SUPPORTED != ret)
		return IS_NO_SUCCESS;

	return IS_SUCCESS;
}

long DCxCamera::ExitCamera()
{
	if( camera.IsInit())
	{
		camera.DisableEvent(IS_SET_EVENT_FRAME);
		camera.ExitEvent(IS_SET_EVENT_FRAME);
		camera.StopLiveVideo(IS_WAIT);

		if( _imageMemory != NULL )
			camera.FreeImageMem( _imageMemory, _imageMemoryId );

		_imageMemory = NULL;

		camera.ExitCamera();
	}

	return IS_SUCCESS;
}

void LiveThread()
{
	while (true)
	{	
		DWORD dwRet = WaitForSingleObject(DCxCamera::eventHandle, 1000);
		if (dwRet == WAIT_OBJECT_0)
		{
			/* event signalled */			
			DCxCamera::camera_status = ICamera::STATUS_READY;
		}
		else
		{
			DCxCamera::camera_status = ICamera::STATUS_BUSY;
		}
	}

	return;
}

long DCxCamera::StartLive()
{
	eventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (IS_SUCCESS != camera.InitEvent(eventHandle, IS_SET_EVENT_FRAME) || IS_SUCCESS != camera.EnableEvent(IS_SET_EVENT_FRAME))
		return IS_NO_SUCCESS;

	DWORD dwLiveThreadID;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LiveThread, NULL, 0, &dwLiveThreadID);

	return camera.CaptureVideo(IS_DONT_WAIT);
}

long DCxCamera::StopLive()
{
	camera.StopLiveVideo(IS_WAIT);
	camera.DisableEvent(IS_SET_EVENT_FRAME);
	camera.ExitEvent(IS_SET_EVENT_FRAME);

	CloseHandle(eventHandle);

	return IS_SUCCESS;
}

long DCxCamera::ReAllocImageMemory()
{
	if( _imageMemory != NULL )
	{
		if (IS_SUCCESS != camera.FreeImageMem( _imageMemory, _imageMemoryId ))
			return IS_NO_SUCCESS;
	}

	_imageMemory = NULL;

	int imgWidth = imgPtyDll.roi.Right - imgPtyDll.roi.Left;
	int imgHeight = imgPtyDll.roi.Bottom - imgPtyDll.roi.Top;

	// allocate an image memory. 
	// then set image memory. 
	// then set the image size to capture
	if (IS_SUCCESS != camera.AllocImageMem(imgWidth, imgHeight, imgPtyDll.bitsPerPixel, &_imageMemory, &_imageMemoryId )
		|| IS_SUCCESS != camera.SetImageMem( _imageMemory, _imageMemoryId ) || IS_SUCCESS != camera.SetImageSize(imgWidth, imgHeight))
	{
		return IS_NO_SUCCESS;
	}

	return IS_SUCCESS;
}

long DCxCamera::SetDefaultParameters()
{
	// Some default
	_imgPtySetSettings.imageAngle = imgPtyDll.imageAngle = 0;
	_imgPtySetSettings.readOutSpeedIndex = imgPtyDll.readOutSpeedIndex = 0;
	_imgPtySetSettings.averageMode = imgPtyDll.averageMode = ICamera::AVG_MODE_NONE;

	// Set trigger with software
	// maybe need to be set as IS_SET_TRIGGER_HI_LO
	if (IS_SUCCESS != camera.SetExternalTrigger(IS_SET_TRIGGER_SOFTWARE))
	{
		return IS_NO_SUCCESS;
	}

	if (IS_SUCCESS != camera.SetIO(IS_IO_CMD_FLASH_SET_MODE, IO_FLASH_MODE_TRIGGER_HI_ACTIVE))
	{
		return IS_NO_SUCCESS;
	}

	// disable hot pixel correction
	if (IS_SUCCESS != camera.SetHotPixelCorrection(0))
		return IS_NO_SUCCESS;

	_imgPtySetSettings.hotPixelCorrection = imgPtyDll.hotPixelCorrection = FALSE;	

	double cFrame;
	// Set frame rate to max
	if (IS_SUCCESS != camera.SetFrameRate((1 / (imgPtyDll.frameTime.min)), &cFrame))
		return IS_NO_SUCCESS;

	_imgPtySetSettings.frameRate = imgPtyDll.frameRate = cFrame;

	return IS_SUCCESS;
}

long DCxCamera::GetDefaultParameters()
{
	// retrieve original image size
	{
		SENSORINFO sInfo;
		if (IS_SUCCESS != camera.GetSensorInfo(&sInfo))
			return IS_NO_SUCCESS;
		_imgPtySetSettings.sensorInfo = imgPtyDll.sensorInfo = sInfo;
	}

	// Get current position of AOI
	{
		IS_RECT aoirect;
		if (IS_SUCCESS != camera.GetAOIRect(&aoirect))
			return IS_NO_SUCCESS;

		_imgPtySetSettings.roi.Left = imgPtyDll.roi.Left = aoirect.s32X;
		_imgPtySetSettings.roi.Top = imgPtyDll.roi.Top = aoirect.s32Y;
		_imgPtySetSettings.roi.Right = imgPtyDll.roi.Right = aoirect.s32Width + aoirect.s32X;
		_imgPtySetSettings.roi.Bottom = imgPtyDll.roi.Bottom = aoirect.s32Height + aoirect.s32Y;
	}

	// Get current binnings, subsampling
	{
		int supportBinning = camera.GetBinningSupport();
		if (supportBinning != IS_BINNING_DISABLE)
		{
			_imgPtySetSettings.binning.isBinning = imgPtyDll.binning.isBinning = TRUE;
			_imgPtySetSettings.binning.supportBinning = imgPtyDll.binning.supportBinning = supportBinning;
			_imgPtySetSettings.binning.xCurrent = imgPtyDll.binning.xCurrent = camera.GetBinning(0);
			_imgPtySetSettings.binning.yCurrent = imgPtyDll.binning.yCurrent = camera.GetBinning(1);
		}
		else
		{
			supportBinning = camera.GetSubSamplingSupport();
			if (supportBinning != IS_SUBSAMPLING_DISABLE)
			{
				_imgPtySetSettings.binning.isBinning = imgPtyDll.binning.isBinning = FALSE;
				_imgPtySetSettings.binning.supportBinning = imgPtyDll.binning.supportBinning = supportBinning;
				_imgPtySetSettings.binning.xCurrent = imgPtyDll.binning.xCurrent = camera.GetSubSampling(0);
				_imgPtySetSettings.binning.yCurrent = imgPtyDll.binning.yCurrent = camera.GetSubSampling(1);
			}
			else
			{
				return IS_NO_SUCCESS;
			}
		}
	}

	// Get the allowable exposure range...
	{
		double expMin, expMax, expInterval;		
		if(IS_SUCCESS != camera.GetExposureRange(&expMin,&expMax,&expInterval))
			return IS_NO_SUCCESS;

		_imgPtySetSettings.exposure.Min = imgPtyDll.exposure.Min = expMin;
		_imgPtySetSettings.exposure.Max = imgPtyDll.exposure.Max = expMax;
		_imgPtySetSettings.exposure.Interval = imgPtyDll.exposure.Interval = expInterval;
	}

	// Get current exposure
	{
		double expCurrent;
		if (IS_SUCCESS != camera.GetExposure(&expCurrent))
			return IS_NO_SUCCESS;

		_imgPtySetSettings.exposure.Current = imgPtyDll.exposure.Current = expCurrent;
	}

	// Get pixel clock range
	{
		int pixelClockMin, pixelClockMax;
		if (IS_SUCCESS != camera.GetPixelClockRange(&pixelClockMin, &pixelClockMax))
			return IS_NO_SUCCESS;

		_imgPtySetSettings.pixelClock.min = imgPtyDll.pixelClock.min = pixelClockMin;
		_imgPtySetSettings.pixelClock.max = imgPtyDll.pixelClock.max = pixelClockMax;
	}

	// Get current pixel clock
	{
		int pixelClock;
		if (IS_SUCCESS != camera.GetPixelClock(&pixelClock))
			return IS_NO_SUCCESS;

		_imgPtySetSettings.pixelClock.pixelClock = imgPtyDll.pixelClock.pixelClock = pixelClock;
	}

	// Get frame time range and interval
	{
		double frameTimeMin, frameTimeMax, frameTimeInterval;
		if (IS_SUCCESS != camera.GetFrameTimeRange(&frameTimeMin, &frameTimeMax, &frameTimeInterval))
			return IS_NO_SUCCESS;

		_imgPtySetSettings.frameTime.min = imgPtyDll.frameTime.min = frameTimeMin;
		_imgPtySetSettings.frameTime.max = imgPtyDll.frameTime.max = frameTimeMax;
		_imgPtySetSettings.frameTime.interval = imgPtyDll.frameTime.interval = frameTimeInterval;		
	}

	// Get black level range
	{
		IS_RANGE_S32 blvlRange;
		if (IS_SUCCESS != camera.GetBlacklvlOffsetRange(&blvlRange))
			return IS_NO_SUCCESS;
		_imgPtySetSettings.blackRange = imgPtyDll.blackRange = blvlRange;
	}

	// Get current black level
	{
		int blackLevel;
		if (IS_SUCCESS != camera.GetBlacklvlOffset(&blackLevel))
			return IS_NO_SUCCESS;

		_imgPtySetSettings.blackLevel = imgPtyDll.blackLevel = blackLevel;		
	}

	// Get gain
	{
		double gain = camera.GetGain();
		if (IS_NO_SUCCESS == (int)gain)
			return IS_NO_SUCCESS;
		_imgPtySetSettings.gain = imgPtyDll.gain = gain;
	}

	// Get gamma
	{
		int gamma;
		if (IS_SUCCESS != camera.GetGamma(&gamma))
			return IS_NO_SUCCESS;

		_imgPtySetSettings.gamma = imgPtyDll.gamma = gamma / 100.0;
	}

	// Get image flip
	{
		int flipMixed = camera.GetFlipStatus();	
		if (IS_NO_SUCCESS == flipMixed)
			return IS_NO_SUCCESS;

		if (flipMixed > 0)
		{
			// Get current image flip
			if ((flipMixed & IS_SET_ROP_MIRROR_LEFTRIGHT) == IS_SET_ROP_MIRROR_LEFTRIGHT) 
			{
				_imgPtySetSettings.horizontalFlip = imgPtyDll.horizontalFlip = 1;
			}
			if ((flipMixed & IS_SET_ROP_MIRROR_UPDOWN) == IS_SET_ROP_MIRROR_LEFTRIGHT) 
			{
				_imgPtySetSettings.verticalFlip = imgPtyDll.verticalFlip = 1;
			}
		}
		else
		{
			_imgPtySetSettings.horizontalFlip = imgPtyDll.horizontalFlip = 0;
			_imgPtySetSettings.verticalFlip = imgPtyDll.verticalFlip = 0;
		}
	}

	return IS_SUCCESS;
}

long DCxCamera::SetParameterIntoDevice()
{
	bool is_exposure_need_update = false;
	bool is_framerate_range_need_update = false;

	// Dont know why must set AOI first
	if (imgPtyDll.roi.Left != _imgPtySetSettings.roi.Left
		|| imgPtyDll.roi.Top != _imgPtySetSettings.roi.Top
		|| imgPtyDll.roi.Bottom != _imgPtySetSettings.roi.Bottom
		|| imgPtyDll.roi.Right != _imgPtySetSettings.roi.Right)
	{
		IS_RECT rect;
		rect.s32X = imgPtyDll.roi.Left;
		rect.s32Y = imgPtyDll.roi.Top;
		rect.s32Width = imgPtyDll.roi.Right - imgPtyDll.roi.Left;
		rect.s32Height = imgPtyDll.roi.Bottom - imgPtyDll.roi.Top;
		if (IS_SUCCESS != camera.SetAOI(rect))
		{
			imgPtyDll.roi = _imgPtySetSettings.roi;
		}
		else
		{
			_imgPtySetSettings.roi = imgPtyDll.roi;

			//when resized the AOI, realloc image memory, reset image memory, free previous one, point used memory address to this new one.
			ReAllocImageMemory();
		}
	}

	if (imgPtyDll.gain != _imgPtySetSettings.gain)
	{
		if (IS_SUCCESS != camera.SetGain(imgPtyDll.gain))
		{
			imgPtyDll.gain = _imgPtySetSettings.gain;	
		}
		else
		{
			_imgPtySetSettings.gain = imgPtyDll.gain;
		}
	}

	if (imgPtyDll.exposure.Current != _imgPtySetSettings.exposure.Current)
	{
		double newExposure;
		if (IS_SUCCESS != camera.SetExposureTime(imgPtyDll.exposure.Current, &newExposure))
		{
			imgPtyDll.exposure.Current = _imgPtySetSettings.exposure.Current;
		}
		else
		{
			_imgPtySetSettings.exposure.Current = imgPtyDll.exposure.Current = newExposure;
		}
	}

	if (imgPtyDll.blackLevel != _imgPtySetSettings.blackLevel)
	{
		if (IS_SUCCESS != camera.SetBlacklvlOffset(imgPtyDll.blackLevel))
		{
			imgPtyDll.blackLevel = _imgPtySetSettings.blackLevel;
		}
		else
		{
			_imgPtySetSettings.blackLevel = imgPtyDll.blackLevel;
		}
	}

	if (_imgPtySetSettings.horizontalFlip != imgPtyDll.horizontalFlip)
	{
		if (IS_SUCCESS != camera.SetFlipStatus(0, imgPtyDll.horizontalFlip))
		{
			imgPtyDll.horizontalFlip = _imgPtySetSettings.horizontalFlip;
		}
		else
		{
			_imgPtySetSettings.horizontalFlip = imgPtyDll.horizontalFlip;
		}
	}

	if (_imgPtySetSettings.verticalFlip != imgPtyDll.verticalFlip)
	{
		if (IS_SUCCESS != camera.SetFlipStatus(1, imgPtyDll.verticalFlip))
		{
			imgPtyDll.verticalFlip = _imgPtySetSettings.verticalFlip;
		}
		else
		{
			_imgPtySetSettings.verticalFlip = imgPtyDll.verticalFlip;
		}
	}

	if (_imgPtySetSettings.binning.xCurrent != imgPtyDll.binning.xCurrent)
	{
		if (imgPtyDll.binning.isBinning)
		{
			if (IS_SUCCESS != camera.SetBinning(0, imgPtyDll.binning.xCurrent))
			{
				imgPtyDll.binning.xCurrent = _imgPtySetSettings.binning.xCurrent;
			}
			else
			{
				_imgPtySetSettings.binning.xCurrent = imgPtyDll.binning.xCurrent;
				is_framerate_range_need_update = true;
			}
		}
		else
		{
			if (IS_SUCCESS != camera.SetSubSampling(0, imgPtyDll.binning.xCurrent))
			{
				imgPtyDll.binning.xCurrent = _imgPtySetSettings.binning.xCurrent;
			}
			else
			{
				_imgPtySetSettings.binning.xCurrent = imgPtyDll.binning.xCurrent;
				is_framerate_range_need_update = true;
			}
		}
	}

	if (_imgPtySetSettings.binning.yCurrent != imgPtyDll.binning.yCurrent)
	{
		if (imgPtyDll.binning.isBinning)
		{
			if (IS_SUCCESS != camera.SetBinning(1, imgPtyDll.binning.yCurrent))
			{
				imgPtyDll.binning.yCurrent = _imgPtySetSettings.binning.yCurrent;
			}
			else
			{
				_imgPtySetSettings.binning.yCurrent = imgPtyDll.binning.yCurrent;
				is_framerate_range_need_update = true;
			}
		}
		else
		{
			if (IS_SUCCESS != camera.SetSubSampling(1, imgPtyDll.binning.yCurrent))
			{
				imgPtyDll.binning.yCurrent = _imgPtySetSettings.binning.yCurrent;
			}
			else
			{
				_imgPtySetSettings.binning.yCurrent = imgPtyDll.binning.yCurrent;
				is_framerate_range_need_update = true;
			}
		}
	}

	if (imgPtyDll.hotPixelCorrection != _imgPtySetSettings.hotPixelCorrection)
	{
		if (IS_SUCCESS != camera.SetHotPixelCorrection(imgPtyDll.hotPixelCorrection))
		{
			imgPtyDll.hotPixelCorrection = _imgPtySetSettings.hotPixelCorrection;
		}
		else
		{
			_imgPtySetSettings.hotPixelCorrection = imgPtyDll.hotPixelCorrection;
		}
	}

	// when binning or subsampling is changed, the frame rate range will also change
	if (is_framerate_range_need_update)
	{
		double frameTimeMin, frameTimeMax, frameTimeInterval;
		if (IS_SUCCESS == camera.GetFrameTimeRange(&frameTimeMin, &frameTimeMax, &frameTimeInterval))
		{
			_imgPtySetSettings.frameTime.min = imgPtyDll.frameTime.min = frameTimeMin;
			_imgPtySetSettings.frameTime.max = imgPtyDll.frameTime.max = frameTimeMax;
			_imgPtySetSettings.frameTime.interval = imgPtyDll.frameTime.interval = frameTimeInterval;
		}

		//may be the current frame rate is bigger than set one, recorrect it
		double maxFrame = 1 / _imgPtySetSettings.frameTime.min;
		if (_imgPtySetSettings.frameRate > maxFrame)
		{
			_imgPtySetSettings.frameRate = maxFrame;
		}
	}

	if(imgPtyDll.frameRate != _imgPtySetSettings.frameRate)
	{
		double newFrameRate;
		if (IS_SUCCESS != camera.SetFrameRate(imgPtyDll.frameRate, &newFrameRate))
		{
			imgPtyDll.frameRate = _imgPtySetSettings.frameRate;
		}
		else
		{
			_imgPtySetSettings.frameRate = imgPtyDll.frameRate = newFrameRate;
			is_exposure_need_update = true;
		}
	}

	// Exposure range will be change when frame rate changed.
	if (is_exposure_need_update)
	{
		double expMin, expMax, expInterval;	
		if(IS_SUCCESS != camera.GetExposureRange(&expMin,&expMax,&expInterval))
		{
			imgPtyDll.exposure.Min = _imgPtySetSettings.exposure.Min;
			imgPtyDll.exposure.Max = _imgPtySetSettings.exposure.Max;
			imgPtyDll.exposure.Interval = _imgPtySetSettings.exposure.Interval;
		}
		else
		{
			_imgPtySetSettings.exposure.Min = imgPtyDll.exposure.Min = expMin;
			_imgPtySetSettings.exposure.Max = imgPtyDll.exposure.Max = expMax;
			_imgPtySetSettings.exposure.Interval = imgPtyDll.exposure.Interval = expInterval;
		}

		//When exposure range chaned, old exposure may out of range, need to get exposure again.
		double exposure;
		if (IS_SUCCESS != camera.GetExposure(&exposure))
		{
			imgPtyDll.exposure.Current = _imgPtySetSettings.exposure.Current;
		}
		else
		{
			_imgPtySetSettings.exposure.Current = imgPtyDll.exposure.Current = exposure;
		}
	}

	return TRUE;
}

long DCxCamera::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{	
	long ret = TRUE;

	switch(paramID)
	{
	case ICamera::PARAM_CAMERA_TYPE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = ICamera::CCD;
			paramMax = ICamera::CCD;
			paramDefault = ICamera::CCD;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_BINNING_X:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = BINNING_X_MIN;
			paramMax = BINNING_X_MAX;
			paramDefault = 1;
		}
		break;
	case ICamera::PARAM_BINNING_Y:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = BINNING_Y_MIN;
			paramMax = BINNING_Y_MAX;
			paramDefault = 1;
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_LEFT:
		{
			double width_min, width_max, width_step;
			GetUniqueParamInfo(ICamera::PARAM_CAPTURE_REGION_LEFT, width_min, width_max, width_step);

			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = width_max - width_step;
			paramDefault = 0;
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_RIGHT:
		{
			double width_min, width_max, width_step;
			GetUniqueParamInfo(ICamera::PARAM_CAPTURE_REGION_RIGHT, width_min, width_max, width_step);

			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = width_min;
			paramMax = width_max;
			paramDefault = width_max;
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_TOP:
		{
			double height_min, height_max, height_step;
			GetUniqueParamInfo(ICamera::PARAM_CAPTURE_REGION_TOP, height_min, height_max, height_step);

			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = height_max - height_step;
			paramDefault = 0;
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
		{
			double height_min, height_max, height_step;
			GetUniqueParamInfo(ICamera::PARAM_CAPTURE_REGION_BOTTOM, height_min, height_max, height_step);

			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = height_min;
			paramMax = height_max;
			paramDefault = height_max;
		}
		break;

	case ICamera::PARAM_CAMERA_AVERAGEMODE:
		{
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= ICamera::AVG_MODE_NONE;
			paramMax		= ICamera::AVG_MODE_CUMULATIVE;
			paramDefault	= ICamera::AVG_MODE_NONE;
			paramReadOnly	= TRUE;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_ANGLE:
		{
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramReadOnly	= FALSE;
			paramMin		= MIN_ANGLE;
			paramMax		= MAX_ANGLE;
			paramDefault	= 0;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP:
		{
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= FALSE;
			paramReadOnly	= FALSE;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_HEIGHT:
		{
			double height_min, height_max, height_step;
			GetUniqueParamInfo(ICamera::PARAM_CAMERA_IMAGE_HEIGHT, height_min, height_max, height_step);

			paramType=ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = height_min;
			paramMax = height_max;
			paramDefault = height_max;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_WIDTH:
		{
			double width_min, width_max, width_step;
			GetUniqueParamInfo(ICamera::PARAM_CAMERA_IMAGE_WIDTH, width_min, width_max, width_step);

			paramType=ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = width_min;
			paramMax = width_max;
			paramDefault = width_max;
		}
		break;
	case ICamera::PARAM_DROPPED_FRAMES:
		{
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;
			paramMin		= 0;
			paramMax		= INT_MAX;
			paramDefault	= 0;
		}
		break;
	case ICamera::PARAM_CAMERA_LED_AVAILABLE:
		{
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= TRUE;			
		}
		break;
	case ICamera::PARAM_CAMERA_LED_ENABLE:
		{
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramReadOnly	= FALSE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= TRUE;			
		}
		break;
	case ICamera::PARAM_READOUT_SPEED_INDEX:
		{
			paramType=ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 1;
		}
		break;
	case ICamera::PARAM_PIXEL_SIZE:
		{
			paramType=ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = imgPtyDll.sensorInfo.wPixelSize;
			paramMax = imgPtyDll.sensorInfo.wPixelSize;
			paramDefault = imgPtyDll.sensorInfo.wPixelSize;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP:
		{
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramReadOnly	= FALSE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= FALSE;
		}
		break;
	case ICamera::PARAM_GAIN:
		{
			double gain_min, gain_max, gain_default;
			GetUniqueParamInfo(ICamera::PARAM_GAIN, gain_min, gain_max, gain_default);

			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = gain_min;
			paramMax = gain_max;
			paramDefault = gain_default;
		}
		break;
	case ICamera::PARAM_TRIGGER_MODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = ICamera::SW_SINGLE_FRAME;
			paramMax = ICamera::HW_MULTI_FRAME_TRIGGER_FIRST;
			paramDefault = ICamera::SW_SINGLE_FRAME;
		}
		break;

	case ICamera::PARAM_EXPOSURE_TIME_MS:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = imgPtyDll.exposure.Min;
			paramMax = imgPtyDll.exposure.Max;
			paramDefault = imgPtyDll.exposure.Current;
		}
		break;

	case ICamera::PARAM_DETECTOR_NAME :
		{
			paramType		= ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= FALSE;
		}
		break;

	case ICamera::PARAM_DETECTOR_SERIAL_NUMBER :
		{
			paramType		= ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;
		}
		break;

	case ICamera::PARAM_FRAME_RATE:
		{
			paramType		= ICamera::TYPE_DOUBLE;
			paramAvailable	= TRUE;
			paramReadOnly	= FALSE;
			paramMin = 1 / imgPtyDll.frameTime.max;
			paramMax = 1 / imgPtyDll.frameTime.min;
			paramDefault = 1 / imgPtyDll.frameTime.max;
		}
		break;

	case ICamera::PARAM_BITS_PER_PIXEL :
		{
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;

			if (imgPtyDll.sensorInfo.nColorMode == IS_COLORMODE_BAYER) 
			{
				paramMin		= 8;
				paramMax		= 8;
				paramDefault	= 8;

			} else 
			{
				paramMin		= 10;
				paramMax		= 10;
				paramDefault	= 10;
			}
		}
		break;

	case ICamera::PARAM_CAMERA_AVERAGENUM:
		{
			paramType		= ICamera::TYPE_DOUBLE;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;
		}
		break;

	case ICamera::PARAM_CAMERA_CHANNEL:
		{
			int chan = 0;
			if (imgPtyDll.colorMode == IS_SET_CM_RGB24)
			{
				for (int i = 0; i < 3; i++)
				{
					int iComp = (0x1 << i);
					if ((imgPtyDll.channel & iComp) == iComp)
					{
						chan++;
					}
				}	
			}
			if (imgPtyDll.colorMode == IS_SET_CM_Y8)
			{
				chan = 1;
			}
					
			paramType		= ICamera::TYPE_DOUBLE;
			paramMin        = chan;
			paramMax        = chan;
			paramDefault    = chan;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;
		}
		break;

	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			paramType		= ICamera::TYPE_DOUBLE;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;
		}
		break;

	default:
		{
			ret = FALSE;
			paramAvailable = FALSE;
		}
	}

	return ret;

}

long DCxCamera::SetParam(const long paramID, const double param)
{
	long ret = FALSE;

	switch(paramID)
	{
	case ICamera::PARAM_BINNING_X:
		{		
			int binningx =  imgPtyDll.binning.isBinning ? camera.GetBinningCommand((int)param, 0) : camera.GetSubSamplingCommand((int)param, 0);

			if ((binningx & imgPtyDll.binning.supportBinning) == binningx)
			{
				imgPtyDll.binning.xCurrent  = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_BINNING_Y:
		{		
			int binningy =  imgPtyDll.binning.isBinning ? camera.GetBinningCommand((int)param, 1) : camera.GetSubSamplingCommand((int)param, 1);

			if ((binningy & imgPtyDll.binning.supportBinning) == binningy)
			{
				imgPtyDll.binning.yCurrent  = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_LEFT:
		{
			if((param >= 0) && (param < imgPtyDll.roi.Right))
			{
				imgPtyDll.roi.Left  = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_RIGHT:
		{
			double width_min, width_max, width_step;
			if (FALSE == GetUniqueParamInfo(ICamera::PARAM_CAPTURE_REGION_RIGHT, width_min, width_max, width_step))
				ret = FALSE;

			if((param > imgPtyDll.roi.Left) && (param <= width_max) && ((param - imgPtyDll.roi.Left) > width_min) && ((long)(param - imgPtyDll.roi.Left) % (long)width_step == 0))
			{
				imgPtyDll.roi.Right  = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_TOP:
		{
			if((param >= 0) && (param < imgPtyDll.roi.Bottom))
			{
				imgPtyDll.roi.Top  = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
		{
			double height_min, height_max, height_step;
			if (FALSE == GetUniqueParamInfo(ICamera::PARAM_CAPTURE_REGION_BOTTOM, height_min, height_max, height_step))
				ret = FALSE;

			if((param > imgPtyDll.roi.Top) && (param <= height_max) && ((param - imgPtyDll.roi.Top) > height_min) && ((long)(param - imgPtyDll.roi.Top) % (long)height_step == 0))
			{
				imgPtyDll.roi.Bottom = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAMERA_AVERAGEMODE:
		{
			if (param >= ICamera::AVG_MODE_NONE && param <= ICamera::AVG_MODE_CUMULATIVE)
			{
				imgPtyDll.averageMode = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_ANGLE:
		{
			long rem = static_cast<long>(param) % 90;
			if (0 == rem)
			{
				if (0 <= static_cast<long>(param) && 270 >= static_cast<long>(param))
				{
					imgPtyDll.imageAngle = static_cast<unsigned long>(param);
					ret = TRUE;
				}
				else if (-90 >= static_cast<long>(param))
				{
					imgPtyDll.imageAngle = 270;
					ret = TRUE;
				}
				else if (360 <= static_cast<long>(param))
				{
					imgPtyDll.imageAngle = 0;
					ret = TRUE;
				}
			}
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP:
		{
			if (TRUE == param || FALSE == param)
			{
				imgPtyDll.horizontalFlip = static_cast<uint32_t>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP :
		{
			if (TRUE == param || FALSE == param)
			{
				imgPtyDll.verticalFlip = static_cast<uint32_t>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_GAIN:
		{
			double gain_min, gain_max, gain_default;
			if (FALSE == GetUniqueParamInfo(ICamera::PARAM_GAIN, gain_min, gain_max, gain_default))
				ret = FALSE;

			if((param >= gain_min) && (param <= gain_max))
			{
				imgPtyDll.gain = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_TRIGGER_MODE:
		{
			if((param >= ICamera::FIRST_TRIGGER_MODE) && (param <= ICamera::LAST_TRIGGER_MODE))
			{
				imgPtyDll.triggerMode = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_FRAME_RATE:
		{
			if ((1 / imgPtyDll.frameTime.min) >= param && (1 / imgPtyDll.frameTime.max) <= param)
			{
				imgPtyDll.frameRate = param;
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_EXPOSURE_TIME_MS:
		{
			if((param >= imgPtyDll.exposure.Min) && (param <= imgPtyDll.exposure.Max))
			{
				imgPtyDll.exposure.Current = param;
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_OP_MODE:
		{
			/*if (TSI_OP_MODE_NORMAL <= param && TSI_MAX_OP_MODES >= param)
			{
			imgPtyDll.opMode = (TSI_OP_MODE)static_cast<unsigned long>(param);
			ret = TRUE;
			}	*/		
		}
		break;
	case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER:
		{
			if (0 <= param && UINT_MAX >= param)
			{
				imgPtyDll.numImagesToBuffer = static_cast<uint32_t>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_OPTICAL_BLACK_LEVEL:
		{					
			if (imgPtyDll.blackRange.s32Max >= param && imgPtyDll.blackRange.s32Min <= param)
			{
				imgPtyDll.blackLevel = static_cast<uint32_t>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAMERA_AVERAGENUM:
		{		
			//if (paramInfo->paramMax >= param && paramInfo->paramMin <= param)
			//{
			imgPtyDll.averageNum = static_cast<uint32_t>(param);
			ret = TRUE;
			//}
		}
		break;
	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			imgPtyDll.multiFrame = param;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_READOUT_SPEED_INDEX:
		{
			//const TSI_ParamInfo* paramInfo = cameraMapParams[PARAM_READOUT_SPEED_INDEX];
			//if (nullptr != paramInfo)
			//{
			//if (paramInfo->paramMax >= param && paramInfo->paramMin <= param)
			{
				imgPtyDll.readOutSpeedIndex = static_cast<uint32_t>(param);
				ret = TRUE;
			}
			//}
		}
		break;
	case ICamera::PARAM_CAMERA_CHANNEL:
		{
			imgPtyDll.channel = static_cast<uint32_t>(param);
			ret = TRUE;
		}
		break;
	}

	return ret;

}

long DCxCamera::SetParam(const long paramID, const double param, const long channelID)
{
	return FALSE;
}

long DCxCamera::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case ICamera::PARAM_CAMERA_TYPE:
		{
			param = ICamera::CCD;
		}
		break;
	case ICamera::PARAM_EXPOSURE_TIME_MS:
		{
			param =  imgPtyDll.exposure.Current;
		}
		break;
	case ICamera::PARAM_BINNING_X:
		{
			param = imgPtyDll.binning.xCurrent;
		}
		break;
	case ICamera::PARAM_BINNING_Y:
		{
			param = imgPtyDll.binning.yCurrent;
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_LEFT:
		{
			param = imgPtyDll.roi.Left;
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_RIGHT:
		{
			param =  imgPtyDll.roi.Right;
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_TOP:
		{
			param =  imgPtyDll.roi.Top;
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
		{
			param =  imgPtyDll.roi.Bottom;
		}
		break;
	case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER :
		{
			param = imgPtyDll.numImagesToBuffer;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_ANGLE :
		{
			param = imgPtyDll.imageAngle;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP :
		{
			param = imgPtyDll.horizontalFlip;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP :
		{
			param = imgPtyDll.verticalFlip;
		}
		break;
	case ICamera::PARAM_GAIN:
		{
			param =  imgPtyDll.gain;
		}
		break;
	case ICamera::PARAM_TRIGGER_MODE:
		{
			param =  imgPtyDll.triggerMode;
		}
		break;
	case ICamera::PARAM_OPTICAL_BLACK_LEVEL:
		{
			param = imgPtyDll.blackLevel;
		}
		break;
	case ICamera::PARAM_DROPPED_FRAMES:
		{
			/*param = static_cast<double>(_availableFramesCnt);
			ret = TRUE;*/
		}
		break;
	case ICamera::PARAM_CAMERA_AVERAGENUM:
		{
			param = imgPtyDll.averageNum;
		}
		break;
	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			param = imgPtyDll.multiFrame;
		}
		break;
	case ICamera::PARAM_CAMERA_AVERAGEMODE:
		{
			param = imgPtyDll.averageMode;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_WIDTH:
		{
			if (imgPtyDll.imageAngle != 0 && imgPtyDll.imageAngle != 180)
			{				
				param = imgPtyDll.roi.Bottom - imgPtyDll.roi.Top;
			}
			else
			{
				param = imgPtyDll.roi.Right - imgPtyDll.roi.Left;
			}
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_HEIGHT:
		{
			//if the angle is not 0 or 180 we need to change the binning and width/height accordingly
			if (imgPtyDll.imageAngle != 0 && imgPtyDll.imageAngle != 180)
			{
				param = imgPtyDll.roi.Right - imgPtyDll.roi.Left;
			}
			else
			{
				param = imgPtyDll.roi.Bottom - imgPtyDll.roi.Top;
			}
		}
		break;
	case ICamera::PARAM_BITS_PER_PIXEL :

		if (imgPtyDll.sensorInfo.nColorMode == IS_COLORMODE_BAYER) {
			param = 8;
		} else {
			param = 10;
		}
		break;
	case ICamera::PARAM_PIXEL_SIZE:
		{
			param = imgPtyDll.sensorInfo.wPixelSize / 100.0;
		}
		break;
	case ICamera::PARAM_READOUT_SPEED_INDEX:
		{
			param = imgPtyDll.readOutSpeedIndex;
		}
		break;
	case ICamera::PARAM_FRAME_RATE:
		{
			param = imgPtyDll.frameRate;
		}
		break;
	case ICamera::PARAM_CAMERA_CHANNEL:
		{
			param = imgPtyDll.channel;
		}
		break;
	default:
		{
			ret = FALSE;
		}
	}

	return ret;
}

long DCxCamera::GetParam(const long paramID, double &param, const long channelID)
{
	return FALSE;
}

long DCxCamera::PreflightAcquisition(char * pDataBuffer)
{
	if( !camera.IsInit() )
		OpenCamera();

	return camera.IsInit() && SetParameterIntoDevice();
}

long DCxCamera::SetupAcquisition(char * pDataBuffer)
{
	if( !camera.IsInit() ) return FALSE;

	if (IS_SUCCESS != camera.SetGain(imgPtyDll.gain))
	{
		imgPtyDll.gain = _imgPtySetSettings.gain;
	}
	else
	{
		_imgPtySetSettings.gain = imgPtyDll.gain;
	}

	double newExposure;
	if (IS_SUCCESS != camera.SetExposureTime(imgPtyDll.exposure.Current, &newExposure))
	{
		imgPtyDll.exposure.Current = _imgPtySetSettings.exposure.Current;
	}
	else
	{
		_imgPtySetSettings.exposure.Current = imgPtyDll.exposure.Current = newExposure;
	}

	if (IS_SUCCESS != camera.SetBlacklvlOffset(imgPtyDll.blackLevel))
	{
		imgPtyDll.blackLevel = _imgPtySetSettings.blackLevel;
	}
	else
	{
		_imgPtySetSettings.blackLevel = imgPtyDll.blackLevel;
	}

	return SetParameterIntoDevice();
}

long DCxCamera::StartAcquisition(char * pDataBuffer)
{
	if (!camera.IsInit()) return FALSE;

	if (TRUE == _running) return TRUE;

	if (IS_SUCCESS == StartLive())
	{
		_running = TRUE;
		return TRUE;
	}

	camera.SetErrorReport(IS_DISABLE_ERR_REP);
	_running = FALSE;
	return FALSE;
}

long DCxCamera::StatusAcquisition(long &status)
{
	status = camera_status;
	return TRUE;
}

long DCxCamera::CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	long bufferSize;
	return CopyAcquisitionEx(pDataBuffer, bufferSize);
}

long DCxCamera::CopyAcquisitionEx(char* pDataBuffer, long &bufferSize)
{
	bufferSize = 0;
	char* source =  _imageMemory;

	int l2r = imgPtyDll.roi.Right - imgPtyDll.roi.Left;
	int t2b = imgPtyDll.roi.Bottom - imgPtyDll.roi.Top;

	int inner_size = l2r * t2b;

	bool rEnable = ((imgPtyDll.channel & 0x1) == 0x1);
	bool gEnable = ((imgPtyDll.channel & 0x10) == 0x10);
	bool bEnable = ((imgPtyDll.channel & 0x100) == 0x100);

	long rOffset = 0;
	long gOffset = rOffset;
	if (rEnable)
		gOffset += inner_size;
	long bOffset = gOffset;
	if(gEnable)
		bOffset += inner_size;	

	USHORT *bufferR = NULL;
	USHORT *bufferG = NULL;
	USHORT *bufferB = NULL;

	if (imgPtyDll.colorMode == IS_SET_CM_RGB24)		//color, RGB
	{
		if (rEnable)
		{
			bufferR = new USHORT[inner_size];
			bufferSize += inner_size * sizeof(USHORT);
		}
		if (gEnable)
		{
			bufferG = new USHORT[inner_size];
			bufferSize += inner_size * sizeof(USHORT);
		}
		if (bEnable)
		{
			bufferB = new USHORT[inner_size];
			bufferSize += inner_size * sizeof(USHORT);
		}

		for (long i = 0; i < inner_size; i++)
		{
			// address offset
			// 1st 8 bits (1 byte) R; 2nd G; 3rd B ........cycling....
			if (rEnable)
			{
				//memcpy(bufferR + i, (_imageMemory + i * 3), 1);
				*(bufferR + i) = *(source + i * 3);
			}
			if (gEnable)
			{
				//memcpy(bufferG + i, (_imageMemory + i * 3 + 1), 1);
				*(bufferG + i) = *(source + i * 3 + 1);
			}
			if (bEnable)
			{
				//memcpy(bufferB + i, (_imageMemory + i * 3 + 2), 1);
				*(bufferB + i) = *(source + i * 3 + 2);
			}
		}
	}
	else
	{
		bufferSize = inner_size * sizeof(USHORT);
	}

	switch (imgPtyDll.imageAngle)
	{
	case 90:
		{
			IppiSize size;
			size.width = t2b * sizeof(unsigned short);
			size.height = l2r * sizeof(unsigned short);
			int stepSrc = t2b * sizeof(unsigned short);
			IppiRect  roiSrc = {0, 0, t2b, l2r};
			int stepDst = l2r * sizeof(unsigned short);
			IppiRect  roiDst = {0, 0, l2r, t2b};
			long angle = 90;
			long xOffset = 0;
			long yOffset = t2b - 1;
			if ( imgPtyDll.colorMode == IS_SET_CM_RGB24)
			{
				if (rEnable)
					ippiDll->ippiRotate_16u_C1R(bufferR, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer + rOffset * sizeof(USHORT), stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
				if (gEnable)
					ippiDll->ippiRotate_16u_C1R(bufferG, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer + gOffset * sizeof(USHORT), stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
				if (bEnable)
					ippiDll->ippiRotate_16u_C1R(bufferB, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer + bOffset * sizeof(USHORT), stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			}
			else
			{
				USHORT* dest = new USHORT[inner_size];				
				for(int i = 0; i < inner_size; i++)
				{
					*dest++ = *source++;
				}
				ippiDll->ippiRotate_16u_C1R(dest, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			}
		}
		break;
	case 180:
		{
			IppiSize size;
			size.width = l2r * sizeof(unsigned short);
			size.height = t2b * sizeof(unsigned short);
			int stepSrc = l2r * sizeof(unsigned short);
			IppiRect  roiSrc = {0, 0, l2r, t2b};
			int stepDst = l2r * sizeof(unsigned short);
			IppiRect  roiDst = {0, 0, l2r, t2b};
			long angle = 180;
			long xOffset = l2r - 1;
			long yOffset = t2b - 1;
			if ( imgPtyDll.colorMode == IS_SET_CM_RGB24)
			{
				if (rEnable)
					ippiDll->ippiRotate_16u_C1R(bufferR, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer + rOffset * sizeof(USHORT), stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
				if (gEnable)
					ippiDll->ippiRotate_16u_C1R(bufferG, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer + gOffset * sizeof(USHORT), stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
				if (bEnable)
					ippiDll->ippiRotate_16u_C1R(bufferB, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer + bOffset * sizeof(USHORT), stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			}
			else
			{
				USHORT* dest = new USHORT[inner_size];
				for(int i = 0; i < inner_size; i++)
				{
					*dest++ = *source++;
				}
				ippiDll->ippiRotate_16u_C1R(dest, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			}
		}
		break;
	case 270:
		{
			IppiSize size;
			size.width = t2b * sizeof(unsigned short);
			size.height = l2r * sizeof(unsigned short);
			int stepSrc = t2b * sizeof(unsigned short);
			IppiRect  roiSrc = {0, 0, t2b, l2r};
			int stepDst = l2r * sizeof(unsigned short);
			IppiRect  roiDst = {0, 0, l2r, t2b};
			long angle = 270;
			long xOffset = l2r - 1;
			long yOffset = 0;
			if ( imgPtyDll.colorMode == IS_SET_CM_RGB24)
			{
				if (rEnable)
					ippiDll->ippiRotate_16u_C1R(bufferR, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer + rOffset * sizeof(USHORT), stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
				if (gEnable)
					ippiDll->ippiRotate_16u_C1R(bufferG, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer + gOffset * sizeof(USHORT), stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
				if (bEnable)
					ippiDll->ippiRotate_16u_C1R(bufferB, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer + bOffset * sizeof(USHORT), stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			}
			else
			{
				USHORT* dest = new USHORT[inner_size];
				for(int i = 0; i < inner_size; i++)
				{
					*dest++ = *source++;
				}
				ippiDll->ippiRotate_16u_C1R(dest, size, stepSrc, roiSrc, (unsigned short*)pDataBuffer, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			}
		}
		break;
	default:	
		if (imgPtyDll.colorMode == IS_SET_CM_RGB24)		//color, RGB
		{			
			if (rEnable)
			{
				USHORT* dest = (USHORT*)pDataBuffer + rOffset * sizeof(USHORT);
				for (int i = 0; i < inner_size; i++)
				{
					*dest++ = *bufferR++;
				}
				//memcpy(pDataBuffer + rOffset, bufferR, l2r * t2b);
			}
			if (gEnable)
			{
				USHORT* dest = (USHORT*)pDataBuffer + gOffset * sizeof(USHORT);
				for (int i = 0; i < inner_size; i++)
				{
					*dest++ = *bufferG++;
				}
				//memcpy(pDataBuffer + gOffset, bufferG, l2r * t2b);
			}
			if (bEnable)
			{
				USHORT* dest = (USHORT*)pDataBuffer + bOffset * sizeof(USHORT);
				for (int i = 0; i < inner_size; i++)
				{
					*dest++ = *bufferB++;
				}
				//memcpy(pDataBuffer + bOffset, bufferB, l2r * t2b);
			}
		}
		else
		{
			USHORT* dest = (USHORT*)pDataBuffer;
			for(int i = 0; i < inner_size; i++)
			{
				*dest++ = *source++;
			}

			//camera.CopyImageMem(_imageMemory, _imageMemoryId, pDataBuffer);
		}
		break;
	}

	return TRUE;
}

long DCxCamera::PostflightAcquisition(char * pDataBuffer)
{
	if (TRUE == _running)
	{
		StopLive();	
		_running = FALSE;
	}
	return TRUE;
}

long DCxCamera::GetSensorName(char *name)
{
	name = NULL;
	return FALSE;
}

long DCxCamera::GetUniqueParamInfo(const long paramID, double &paramMin, double &paramMax, double &paramDefault)
{
	paramMin = paramMax = paramDefault = 0.0;
	return FALSE;
}

void DCxCamera::clear()
{
	return;
}