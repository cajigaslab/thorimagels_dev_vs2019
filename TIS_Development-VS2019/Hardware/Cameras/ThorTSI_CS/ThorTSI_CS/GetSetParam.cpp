#include "stdafx.h"
#include "ThorTSI_CS.h"

long ThorCam::SetParam(const long paramID, const double param)
{
	long ret = TRUE;
	long widthValidation = (0 == _camName[_camID].compare(0, 5, "CS135")) ? 16 : 4;
	long heightValidation = (0 == _camName[_camID].compare(0, 5, "CC505")) ? 4 : 2;
	try
	{
		switch (paramID) 
		{
		case ICamera::PARAM_BINNING_X:
			{
				if((_hbinRange[1] >= static_cast<int>(param)) && (_hbinRange[0] <= static_cast<int>(param)))
				{
					_ImgPty_SetSettings.roiBinX = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_BINNING_Y:
			{
				if((_vbinRange[1] >= static_cast<int>(param)) && (_vbinRange[0] <= static_cast<int>(param)))
				{					
					_ImgPty_SetSettings.roiBinY = static_cast<int>(param);
				}
			}
			break;

		case ICamera::PARAM_CAPTURE_REGION_LEFT:
			{			
				int val = static_cast<int>(param);
				if(_ImgPty_SetSettings.roiRight >= val)
				{
					int imgWidth = (_ImgPty_SetSettings.roiRight + 1) - val;
					// Check if the image width is less than the allowed minimum width
					if(_xRangeR[0] <= imgWidth)
					{
						// Check if the new width of the image is a multiple of widthValidation
						if(0 != imgWidth % widthValidation)
						{
							// If it isn't, substract from the entered value(round up the width), in order to make the new width a multiple of widthValidation
							_ImgPty_SetSettings.roiLeft = val - (widthValidation - (imgWidth % widthValidation));
						}	
						else
						{
							_ImgPty_SetSettings.roiLeft = val;
						}
					}
					else
					{
						//Change the Left value, so the image size is at the minimum width
						_ImgPty_SetSettings.roiLeft = _ImgPty_SetSettings.roiRight - _xRangeR[0];
					}

					// Check if the new value is within limits, reset to full frame value if it isn't
					if(_ImgPty_SetSettings.roiLeft < _xRangeL[0] || _ImgPty_SetSettings.roiLeft >  _xRangeL[1])
					{
						_ImgPty_SetSettings.roiLeft = _xRangeL[0];
					}
				}
			}
			break;
		case ICamera::PARAM_CAPTURE_REGION_RIGHT: 
			{			
				int val = static_cast<int>(param) - 1;
				if(_ImgPty_SetSettings.roiLeft <= val)
				{
					int imgWidth = (val + 1) - _ImgPty_SetSettings.roiLeft;
					// Check if the image width is less than the allowed minimum width
					if(_xRangeR[0] <= imgWidth)
					{
						// Check if the new width of the image is a multiple of widthValidation
						if(0 != imgWidth % widthValidation)
						{
							// If it isn't, substract from the entered value(round up the width), in order to make the new width a multiple of widthValidation
							_ImgPty_SetSettings.roiRight = val + (widthValidation - (imgWidth % widthValidation));
						}
						else
						{
							_ImgPty_SetSettings.roiRight = val;
						}
					}
					else
					{
						//Change the Right value, so the image size is at the minimum width
						_ImgPty_SetSettings.roiRight = _ImgPty_SetSettings.roiLeft + _xRangeR[0];
					}

					// Check if the new value is within limits, reset to full frame if it isn't
					if(_ImgPty_SetSettings.roiRight < _xRangeR[0] || _ImgPty_SetSettings.roiRight > _xRangeR[1])
					{
						_ImgPty_SetSettings.roiRight = _xRangeR[1];
					}
				}
			}
			break;
		case ICamera::PARAM_CAPTURE_REGION_TOP:
			{			
				int val = static_cast<int>(param);
				if(_ImgPty_SetSettings.roiBottom >= val)
				{
					int imgHeight = (_ImgPty_SetSettings.roiBottom + 1) - val;
					// Check if the minimun height is less than the allowed minimum height
					if(_yRangeB[0] <= imgHeight)
					{
						// Check if the new height of the image is a multiple of heightValidation
						if(0 != imgHeight % heightValidation)
						{
							// If it isn't, substract from the entered value(round up the height), in order to make the new height a multiple of heightValidation
							_ImgPty_SetSettings.roiTop = val - (heightValidation - (imgHeight % heightValidation));
						}	
						else
						{
							_ImgPty_SetSettings.roiTop = val;
						}
					}
					else
					{
						//Change the Top value, so the image size is at the minimum height
						_ImgPty_SetSettings.roiTop = _ImgPty_SetSettings.roiBottom - _yRangeB[0];
					}

					// Check if the new value is within limits, reset to full frame if it isn't
					if(_ImgPty_SetSettings.roiTop < _yRangeT[0] || _ImgPty_SetSettings.roiTop  > _yRangeT[1])
					{
						_ImgPty_SetSettings.roiTop = _yRangeT[0];
					}
				}
			}
			break;
		case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
			{			
				int val = static_cast<int>(param) - 1;
				if(_ImgPty_SetSettings.roiTop <= val)
				{
					int imgHeight = (val + 1) - _ImgPty_SetSettings.roiTop;
					// Check if the minimun height is less than the allowed minimum height
					if(_yRangeB[0] <= imgHeight)
					{
						// Check if the new height of the image is a multiple of heightValidation
						if(0 != imgHeight % heightValidation)
						{
							// If it isn't, substract from the entered value(round up the height), in order to make the new height a multiple of heightValidation
							_ImgPty_SetSettings.roiBottom = val + (heightValidation - (imgHeight % heightValidation));
						}
						else
						{
							_ImgPty_SetSettings.roiBottom = val;	
						}
					}
					else
					{
						//Change the Bottom value, so the image size is at the minimum height
						_ImgPty_SetSettings.roiBottom = _ImgPty_SetSettings.roiTop + _yRangeB[0];
					}

					// Check if the new value is within limits, reset to full frame if it isn't
					if(_ImgPty_SetSettings.roiBottom < _yRangeB[0] || _ImgPty_SetSettings.roiBottom  > _yRangeB[1])
					{
						_ImgPty_SetSettings.roiBottom = _yRangeB[1];
					}
				}
			}
			break;
		case ICamera::PARAM_TRIGGER_MODE:
			{
				if (LAST_TRIGGER_MODE >= param && FIRST_TRIGGER_MODE <= param)
				{					
					_ImgPty_SetSettings.triggerMode = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_MULTI_FRAME_COUNT:
			{
				//*** lift the limit to acquire more by free run mode ***//
				//long maxRange = (ICamera::AVG_MODE_CUMULATIVE == _ImgPty_SetSettings.averageMode) ? _frmPerTriggerRange[1] / max(1,_ImgPty_SetSettings.averageNum) : _frmPerTriggerRange[1];

				//if (maxRange >= param && _frmPerTriggerRange[0] <= param)
				//{
				_ImgPty_SetSettings.numFrame = static_cast<long>(param);
				//}
			}
			break;
		case ICamera::PARAM_EXPOSURE_TIME_MS:
			{
				int valUS = static_cast<int>(param * Constants::MS_TO_SEC);
				if (_expUSRange[1] >= valUS && _expUSRange[0] <= valUS)
				{
					ThorTSIErrChk(L"tl_camera_set_exposure_time", tl_camera_set_exposure_time(_camera[_camID], valUS), 1);
					_ImgPty_SetSettings.exposureTime_us = valUS;
				}
			}
			break;
		case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER:
			{
				if (MIN_IMAGE_BUFFERS <= param && UINT_MAX >= param)
				{
					_ImgPty_SetSettings.numImagesToBuffer = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_ANGLE :
			{
				long rem = static_cast<long>(param) % 90;
				if (0 == rem)
				{
					if (0 <= static_cast<long>(param) && 
						270 >= static_cast<long>(param))
					{
						_ImgPty_SetSettings.imageAngle = static_cast<unsigned long>(param);
					}
					else if (-90 >= static_cast<long>(param))
					{
						_ImgPty_SetSettings.imageAngle = 270;
					}
					else if (360 <= static_cast<long>(param))
					{
						_ImgPty_SetSettings.imageAngle = 0;
					}
					else
					{
						ret = FALSE;
					}
				}
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP :
			{
				if (TRUE == param || FALSE == param)
				{
					_ImgPty_SetSettings.horizontalFlip = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP :
			{
				if (TRUE == param || FALSE == param)
				{
					_ImgPty_SetSettings.verticalFlip = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_AVERAGENUM:
			{		
				if (MAX_AVGNUM >= param && MIN_AVGNUM <= param)
				{
					_ImgPty_SetSettings.averageNum = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_AVERAGEMODE:
			{
				if (ICamera::AVG_MODE_NONE == param || ICamera::AVG_MODE_CUMULATIVE == param)
				{
					_ImgPty_SetSettings.averageMode = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_READOUT_SPEED_INDEX:
			{
				int val = static_cast<int>(param) + TL_CAMERA_DATA_RATE_FPS_30;
				if (TL_CAMERA_DATA_RATE_MAX >= param && TL_CAMERA_DATA_RATE_READOUT_FREQUENCY_20 <= param)
				{
					_ImgPty_SetSettings.readOutSpeedIndex = static_cast<int>(val);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT:
			{
				if (MAX_DMABUFNUM >= param && MIN_DMABUFNUM <= param)
				{
					_ImgPty_SetSettings.dmaBufferCount = static_cast<long>(param);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_CHANNEL:
			{
				if (MAX_CHANNEL >= param && MIN_CHANNEL <= param)
				{
					_ImgPty_SetSettings.channel = static_cast<long>(param);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_LED_ENABLE:
			{
				if (1 >= param && 0 <= param)
				{
					ThorTSIErrChk(L"tl_camera_set_is_led_on", tl_camera_set_is_led_on(_camera[_camID], static_cast<int>(param)), 1);
				}
			}
			break;
		case ICamera::PARAM_HOT_PIXEL_THRESHOLD_VALUE:
			{
				if (_hotPixelRange[1] >= param && _hotPixelRange[0] <= param)
				{
					ThorTSIErrChk(L"tl_camera_set_hot_pixel_correction_threshold", tl_camera_set_hot_pixel_correction_threshold(_camera[_camID], static_cast<int>(param)), 1);
					_ImgPty_SetSettings.hotPixelThreshold = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_HOT_PIXEL_ENABLED:
			{
				if (1 >= param && 0 <= param)
				{
					ThorTSIErrChk(L"tl_camera_set_is_frame_rate_control_enabled", tl_camera_set_is_hot_pixel_correction_enabled(_camera[_camID], static_cast<int>(param)), 1);
					_ImgPty_SetSettings.hotPixelEnabled = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_GAIN:
			{
				if (_gainRange[1] >= param && _gainRange[0] <= param)
				{
					_ImgPty_SetSettings.gain = static_cast<long>(param);
				}
			}
			break;
		case ICamera::PARAM_OPTICAL_BLACK_LEVEL:
			{
				if (_blackLevelRange[1] >= param && _blackLevelRange[0] <= param)
				{
					_ImgPty_SetSettings.blackLevel = static_cast<long>(param);
				}
			}
			break;
		case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
			{
				_forceSettingsUpdate = static_cast<long>(param);
			}
			break;
		case ICamera::PARAM_CAMERA_FRAME_RATE_CONTROL_ENABLED:
		{
			if (TRUE >= param && FALSE <= param)
			{
				_ImgPty_SetSettings.frameRateControlEnabled = static_cast<long>(param);
				//ThorTSIErrChk(L"tl_camera_set_is_frame_rate_control_enabled", tl_camera_set_is_frame_rate_control_enabled(_camera[_camID], _imgPtyDll.frameRateControlEnabled), 1);
			}
		}
		break;
		case ICamera::PARAM_CAMERA_FRAME_RATE_CONTROL_VALUE:
		{
			if (_frameRateControlValueRange[1] >= param && _frameRateControlValueRange[0] <= param)
			{
				_ImgPty_SetSettings.frameRateControlValue = param;
				//ThorTSIErrChk(L"tl_camera_set_frame_rate_control_value", tl_camera_set_frame_rate_control_value(_camera[_camID], _imgPtyDll.frameRateControlValue), 1);
			}
		}
		break;
		default:
			{
				ret = FALSE;
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) not implemented",paramID);
				LogMessage(_errMsg,ERROR_EVENT);
			}
			break;
		}	
	}
	catch(...)
	{
		ret = FALSE;
		GetLastError();
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Set Parameter (%d) failed",paramID);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return ret;
}

long ThorCam::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	// If the camera isn't selected, we can't continue.
	if (!IsOpen(_camID))
		return FALSE;

	try
	{
		switch (paramID) 
		{
		case ICamera::PARAM_CAMERA_TYPE:
			param = ICamera::CCD;
			break;
		case ICamera::PARAM_EXPOSURE_TIME_MS:
			param = (double) _ImgPty_SetSettings.exposureTime_us / Constants::MS_TO_SEC;
			break;
			// ROI and Binning are special cases 
		case ICamera::PARAM_BINNING_X:
			param = _ImgPty_SetSettings.roiBinX;
			break;
		case ICamera::PARAM_BINNING_Y:
			param = _ImgPty_SetSettings.roiBinY;
			break;
		case ICamera::PARAM_CAPTURE_REGION_LEFT:
			param = (double) _ImgPty_SetSettings.roiLeft;
			break;
		case ICamera::PARAM_CAPTURE_REGION_RIGHT:
			param = (double) _ImgPty_SetSettings.roiRight + 1;
			break;
		case ICamera::PARAM_CAPTURE_REGION_TOP:
			param = (double) _ImgPty_SetSettings.roiTop;
			break;
		case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
			param = (double) _ImgPty_SetSettings.roiBottom + 1;
			break;
		case ICamera::PARAM_TRIGGER_MODE:
			{
				param = _ImgPty_SetSettings.triggerMode;
			}
			break;
		case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER :
			{
				param = _ImgPty_SetSettings.numImagesToBuffer;
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_ANGLE:
			{
				param = _ImgPty_SetSettings.imageAngle;
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP:
			{
				param = _ImgPty_SetSettings.horizontalFlip;
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP:
			{
				param = _ImgPty_SetSettings.verticalFlip;
			}
			break;
		case ICamera::PARAM_DROPPED_FRAMES:
			{
				param = static_cast<double>(_availableFramesCnt);
			}
			break;
		case ICamera::PARAM_CAMERA_AVERAGENUM:
			{
				param = _ImgPty_SetSettings.averageNum;
			}
			break;
		case ICamera::PARAM_CAMERA_AVERAGEMODE:
			{
				param = _ImgPty_SetSettings.averageMode;
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_WIDTH:
			{
				//if the angle is not 0 or 180 we need to change the binning and width/height accordingly
				if (0 != _ImgPty_SetSettings.imageAngle && 180 != _ImgPty_SetSettings.imageAngle)
				{ 
					if(0 != _ImgPty_SetSettings.roiBinY)
					{
						param = _ImgPty_SetSettings.heightPx = (_ImgPty_SetSettings.roiBottom - _ImgPty_SetSettings.roiTop + 1) / _ImgPty_SetSettings.roiBinY;
					}
					else
					{
						param = _ImgPty_SetSettings.heightPx = _ImgPty_SetSettings.roiBottom - _ImgPty_SetSettings.roiTop + 1;
					}
				}
				else
				{
					if(0 != _ImgPty_SetSettings.roiBinX)
					{
						param = _ImgPty_SetSettings.widthPx = (_ImgPty_SetSettings.roiRight - _ImgPty_SetSettings.roiLeft + 1) / _ImgPty_SetSettings.roiBinX;
					}
					else
					{
						param = _ImgPty_SetSettings.widthPx = _ImgPty_SetSettings.roiRight - _ImgPty_SetSettings.roiLeft + 1;
					}
				}
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_HEIGHT:
			{
				//if the angle is not 0 or 180 we need to change the binning and width/height accordingly
				if (0 != _ImgPty_SetSettings.imageAngle && 180 != _ImgPty_SetSettings.imageAngle)
				{
					if (0 != _ImgPty_SetSettings.roiBinX)
					{
						param = _ImgPty_SetSettings.widthPx = (_ImgPty_SetSettings.roiRight - _ImgPty_SetSettings.roiLeft + 1) / _ImgPty_SetSettings.roiBinX;
					}
					else
					{
						param = _ImgPty_SetSettings.widthPx = _ImgPty_SetSettings.roiRight - _ImgPty_SetSettings.roiLeft + 1;
					}
				}
				else
				{
					if(0 != _ImgPty_SetSettings.roiBinY)
					{
						param = _ImgPty_SetSettings.heightPx = (_ImgPty_SetSettings.roiBottom - _ImgPty_SetSettings.roiTop + 1) / _ImgPty_SetSettings.roiBinY;
					}
					else
					{
						param = _ImgPty_SetSettings.heightPx = _ImgPty_SetSettings.roiBottom - _ImgPty_SetSettings.roiTop + 1;
					}
				}
			}
			break;
		case ICamera::PARAM_BITS_PER_PIXEL:
			{
				param = _ImgPty_SetSettings.bitPerPixel;
			}
			break;
		case ICamera::PARAM_PIXEL_SIZE:
			{
				param = _ImgPty_SetSettings.pixelSizeXUM;		//could be different in X | Y in future
			}
			break;
		case ICamera::PARAM_READOUT_SPEED_INDEX:
			{
				param = _ImgPty_SetSettings.readOutSpeedIndex - TL_CAMERA_DATA_RATE_FPS_30;
			}
			break;
		case ICamera::PARAM_FRAME_RATE:
			{
				double frameRate = 0;
				ThorTSIErrChk(L"tl_camera_get_measured_frame_rate", tl_camera_get_measured_frame_rate(_camera[_camID], &frameRate), 1);
				param = frameRate;
			}
			break;
		case ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT:
			{
				param = _ImgPty_SetSettings.dmaBufferCount;
			}
			break;
		case ICamera::PARAM_CAMERA_CHANNEL:
			{
				param = _ImgPty_SetSettings.channel;
			}
			break;
		case ICamera::PARAM_CAMERA_LED_AVAILABLE:
			{
				int ledAvailable = FALSE;
				ThorTSIErrChk(L"tl_camera_get_is_led_supported", tl_camera_get_is_led_supported(_camera[_camID], &ledAvailable), 1);
				param = ledAvailable;
			}
			break;
		case ICamera::PARAM_CAMERA_LED_ENABLE:
			{
				int ledEnable = 1;
				ThorTSIErrChk(L"tl_camera_get_is_led_on", tl_camera_get_is_led_on(_camera[_camID], &ledEnable), 1);
				param = ledEnable;
			}
			break;
		case ICamera::PARAM_HOT_PIXEL_THRESHOLD_VALUE:
			{
				int hotPixelThresholdVal = 0;
				ThorTSIErrChk(L"tl_camera_get_hot_pixel_correction_threshold", tl_camera_get_hot_pixel_correction_threshold(_camera[_camID], &hotPixelThresholdVal), FALSE);
				param = hotPixelThresholdVal;
			}
			break;
		case ICamera::PARAM_HOT_PIXEL_ENABLED:
			{
				int hotPixelEnabled = 0;
				ThorTSIErrChk(L"tl_camera_get_is_hot_pixel_correction_enabled", tl_camera_get_is_hot_pixel_correction_enabled(_camera[_camID], &hotPixelEnabled), FALSE);
				param = hotPixelEnabled;
			}
			break;
		case ICamera::PARAM_GAIN:
			{
				param = _ImgPty_SetSettings.gain;
			}
			break;
		case ICamera::PARAM_OPTICAL_BLACK_LEVEL:
			{
				param = _ImgPty_SetSettings.blackLevel;
			}
			break;
		case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
			{
				param = _forceSettingsUpdate;
			}
			break;
		case ICamera::PARAM_CAMERA_FRAME_RATE_CONTROL_ENABLED:
		{
			int frameRateControlEnabled = FALSE;
			param = _ImgPty_SetSettings.frameRateControlEnabled;
			//ThorTSIErrChk(L"tl_camera_get_is_frame_rate_control_enabled", tl_camera_get_is_frame_rate_control_enabled(_camera[_camID], &frameRateControlEnabled), 1);
			//param = frameRateControlEnabled;
		}
		break;
		case ICamera::PARAM_CAMERA_FRAME_RATE_CONTROL_VALUE:
		{
			double frameRateControlValue = 0;
			param = _ImgPty_SetSettings.frameRateControlValue;
			/*ThorTSIErrChk(L"tl_camera_get_frame_rate_control_value", tl_camera_get_frame_rate_control_value(_camera[_camID], &frameRateControlValue), 1);
			param = frameRateControlValue;*/
		}
		break;
		default:
			ret = FALSE;
			break;
		}
	}
	catch(...)
	{
		ret = FALSE;
		GetLastError();
		StringCbPrintfW(_errMsg,MSG_SIZE,L"GetParam Parameter (%d) failed", paramID);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	return ret;
}

long ThorCam::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;

	try
	{
		switch (paramID) 
		{
		case ICamera::PARAM_CAMERA_TYPE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= ICamera::CCD;
			paramMax		= ICamera::CCD;
			paramDefault	= ICamera::CCD;
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_CONSOLE_WRITE:
			paramType		= ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CONSOLE_READ:
			paramType		= ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_DETECTOR_NAME:
			paramType		= ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_DETECTOR_SERIAL_NUMBER:
			paramType		= ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_TRIGGER_MODE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= ICamera::FIRST_TRIGGER_MODE;
			paramMax		= ICamera::LAST_TRIGGER_MODE - 1;
			paramDefault	= ICamera::SW_FREE_RUN_MODE;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER :
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_IMAGE_BUFFERS;
			paramMax		= UINT_MAX;
			paramDefault	= MIN_IMAGE_BUFFERS;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CAPTURE_REGION_LEFT:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _xRangeL[0];
			paramMax		= _xRangeL[1];
			paramDefault	= _xRangeL[0];
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CAPTURE_REGION_RIGHT:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _xRangeR[0];
			paramMax		= _xRangeR[1]+1;	//width = _xRangeR[1]-_xRangeR[0]+1
			paramDefault	= _xRangeR[1];
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CAPTURE_REGION_TOP:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _yRangeT[0];
			paramMax		= _yRangeT[1];
			paramDefault	= _yRangeT[0];
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _yRangeB[0];
			paramMax		= _yRangeB[1]+1;	//height = _yRangeB[1]-_yRangeB[0]+1
			paramDefault	= _yRangeB[1];
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_BINNING_X:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _hbinRange[0];
			paramMax		= _hbinRange[1];
			paramDefault	= DEFAULT_XBIN;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_BINNING_Y:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _vbinRange[0];
			paramMax		= _vbinRange[1];
			paramDefault	= DEFAULT_YBIN;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_MULTI_FRAME_COUNT:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _frmPerTriggerRange[0];
			paramMax		= _frmPerTriggerRange[1];
			paramDefault	= DEFAULT_FRM_PER_TRIGGER;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_ANGLE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_ANGLE;
			paramMax		= MAX_ANGLE;
			paramDefault	= DEFAULT_ANGLE;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= FALSE;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= FALSE;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_EXPOSURE_TIME_MS:
			paramType		= ICamera::TYPE_DOUBLE;
			paramAvailable	= TRUE;
			paramMin		= static_cast<double>(_expUSRange[0] / Constants::MS_TO_SEC);
			paramMax		= static_cast<double>(_expUSRange[1] / Constants::MS_TO_SEC);
			paramDefault	= DEFAULT_EXPOSURE_MS;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_BITS_PER_PIXEL:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_BITS_PERPIXEL;
			paramMax		= MAX_BITS_PERPIXEL;
			paramDefault	= _ImgPty_SetSettings.bitPerPixel;
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_WIDTH:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _widthRange[0];
			paramMax		= _widthRange[1];
			paramDefault	= _ImgPty_SetSettings.widthPx;
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_HEIGHT:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _heightRange[0];
			paramMax		= _heightRange[1];
			paramDefault	= _ImgPty_SetSettings.heightPx;
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_DROPPED_FRAMES:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= INT_MAX;
			paramDefault	= 0;
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_CAMERA_AVERAGENUM:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_AVGNUM;
			paramMax		= MAX_AVGNUM;
			paramDefault	= DEFAULT_AVGNUM;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CAMERA_AVERAGEMODE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= ICamera::AVG_MODE_NONE;
			paramMax		= ICamera::AVG_MODE_CUMULATIVE;
			paramDefault	= ICamera::AVG_MODE_NONE;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_PIXEL_SIZE:
			paramType		= ICamera::TYPE_DOUBLE;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= MAXULONG32;
			paramDefault	= _ImgPty_SetSettings.pixelSizeXUM;		//could be different in X | Y in future
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_READOUT_SPEED_INDEX:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= TL_CAMERA_DATA_RATE_READOUT_FREQUENCY_20;
			paramMax		= TL_CAMERA_DATA_RATE_MAX;
			paramDefault	= TL_CAMERA_DATA_RATE_FPS_30;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_FRAME_RATE:
			paramType		= ICamera::TYPE_DOUBLE;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= MAXULONG32;
			paramDefault	= 0;
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT:
			paramType		= ICamera::TYPE_DOUBLE;
			paramAvailable	= TRUE;
			paramMin		= MIN_DMABUFNUM;
			paramMax		= MAX_DMABUFNUM;
			paramDefault	= DEFAULT_DMABUFNUM;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CAMERA_CHANNEL:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_CHANNEL;
			paramMax		= MAX_CHANNEL;
			paramDefault	= DEFAULT_CHANNEL;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CAMERA_LED_AVAILABLE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= TRUE;
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_CAMERA_LED_ENABLE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= TRUE;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_HOT_PIXEL_THRESHOLD_VALUE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _hotPixelRange[0];
			paramMax		= _hotPixelRange[1];
			paramDefault	= _ImgPty_SetSettings.hotPixelThreshold;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_HOT_PIXEL_ENABLED:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= _ImgPty_SetSettings.hotPixelEnabled;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_GAIN:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _gainRange[0];
			paramMax		= _gainRange[1];
			paramDefault	= _ImgPty_SetSettings.gain;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_OPTICAL_BLACK_LEVEL:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _blackLevelRange[0];
			paramMax		= _blackLevelRange[1];
			paramDefault	= _ImgPty_SetSettings.blackLevel;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= 1;
			paramDefault	= 0;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_CAMERA_FRAME_RATE_CONTROL_ENABLED:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_FRAME_RATE_CONTROL_VALUE:
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = _frameRateControlValueRange[0];
			paramMax = _frameRateControlValueRange[1];
			paramDefault = _frameRateControlValueRange[0];
			paramReadOnly = FALSE;
			break;
		default:
			ret				= FALSE;
			paramAvailable	= FALSE;
			break;
		}
	}
	catch(...)
	{
		ret = FALSE;
		GetLastError();
		StringCbPrintfW(_errMsg,MSG_SIZE,L"GetParamInfo Parameter (%d) failed", paramID);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return ret;
}

long ThorCam::GetLastErrorMsg(wchar_t*msg,long size)
{
	wcsncpy_s(msg,size,_errMsg,MSG_SIZE);

	//reset the error message
	_errMsg[0] = 0;
	return TRUE;
}

long ThorCam::SetParamString(const long paramID, wchar_t * str)
{
	//no parameter set, return FALSE
	return FALSE;
}

long ThorCam::GetParamString(const long paramID, wchar_t * str, long size)
{
	long ret = TRUE;
	switch(paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME:
		{
			wcscpy_s(str,size, _pDetectorName);
		}
		break;
	case ICamera::PARAM_DETECTOR_SERIAL_NUMBER:
		{
			wcscpy_s(str, size, _pSerialNumber);
		}
		break;
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

long ThorCam::SetParamBuffer(const long paramID, char * buf, long size)
{
	//no parameter set, return FALSE
	return FALSE;
}

long ThorCam::GetParamBuffer(const long paramID, char * buf, long size)
{
	//no parameter found, return FALSE
	return FALSE;
}
