#include "stdafx.h"
#include "ThorTSI_CS.h"

long ThorCam::SetParam(const long paramID, const double param)
{
	std::lock_guard<std::mutex>  paramLock(_paramSyncLock);
	long ret = TRUE;
	// TODO: The camera is already checking this. If a ROI is not a correct multiple, it will be nudged to a close value. Adopt a policy of reading back the ROI after setting it and the work will be done for you.
	long widthValidation = (0 == _cameraParams[_camID].camModel.compare(0, 5, "CS135")) ? 16 : 4;
	long heightValidation = (0 == _cameraParams[_camID].camModel.compare(0, 5, "CC505")) ? 4 : 2;
	
	// dynamic parameters
	bool isExposureNudged = false;
	bool isFrameRateControlNudged = false;
	
	try
	{
		switch (paramID) 
		{
		case ICamera::PARAM_BINNING_X:
			{
				if((_hbinRange[1] >= static_cast<int>(param)) && (_hbinRange[0] <= static_cast<int>(param)))
				{
					_CameraProperties_UI.roiBinX = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_BINNING_Y:
			{
				if((_vbinRange[1] >= static_cast<int>(param)) && (_vbinRange[0] <= static_cast<int>(param)))
				{					
					_CameraProperties_UI.roiBinY = static_cast<int>(param);
				}
			}
			break;

		case ICamera::PARAM_CAPTURE_REGION_LEFT:
			{			
				int val = static_cast<int>(param);
				if(_CameraProperties_UI.roiRight >= val)
				{
					int imgWidth = (_CameraProperties_UI.roiRight + 1) - val;
					// Check if the image width is less than the allowed minimum width
					if(_xRangeR[0] <= imgWidth)
					{
						// Check if the new width of the image is a multiple of widthValidation
						if(0 != imgWidth % widthValidation)
						{
							// If it isn't, substract from the entered value(round up the width), in order to make the new width a multiple of widthValidation
							_CameraProperties_UI.roiLeft = val - (widthValidation - (imgWidth % widthValidation));
						}	
						else
						{
							_CameraProperties_UI.roiLeft = val;
						}
					}
					else
					{
						_CameraProperties_UI.roiLeft = _CameraProperties_UI.roiRight - _xRangeR[0];
					}

					// Check if the new value is within limits, reset to full frame value if it isn't
					if(_CameraProperties_UI.roiLeft < _xRangeL[0] || _CameraProperties_UI.roiLeft >  _xRangeL[1])
					{
						_CameraProperties_UI.roiLeft = _xRangeL[0];
					}

					if (TRUE == SetROI(_CameraProperties_UI.roiLeft, _CameraProperties_UI.roiRight, _CameraProperties_UI.roiTop, _CameraProperties_UI.roiBottom))
					{
						_CameraProperties_UI.roiLeft = _CameraProperties_Active.roiLeft;
						_CameraProperties_UI.widthPx = _CameraProperties_Active.widthPx;
					}
				}
			}
			break;
		case ICamera::PARAM_CAPTURE_REGION_RIGHT: 
			{			
				int val = static_cast<int>(param) - 1;
				if(_CameraProperties_UI.roiLeft <= val)
				{
					int imgWidth = (val + 1) - _CameraProperties_UI.roiLeft;
					// Check if the image width is less than the allowed minimum width
					if(_xRangeR[0] <= imgWidth)
					{
						// Check if the new width of the image is a multiple of widthValidation
						if(0 != imgWidth % widthValidation)
						{
							// If it isn't, substract from the entered value(round up the width), in order to make the new width a multiple of widthValidation
							_CameraProperties_UI.roiRight = val + (widthValidation - (imgWidth % widthValidation));
						}
						else
						{
							_CameraProperties_UI.roiRight = val;
						}
					}
					else
					{
						//Change the Right value, so the image size is at the minimum width
						_CameraProperties_UI.roiRight = _CameraProperties_UI.roiLeft + _xRangeR[0];
					}

					// Check if the new value is within limits, reset to full frame if it isn't
					if(_CameraProperties_UI.roiRight < _xRangeR[0] || _CameraProperties_UI.roiRight > _xRangeR[1])
					{
						_CameraProperties_UI.roiRight = _xRangeR[1];
					}

					if (TRUE == SetROI(_CameraProperties_UI.roiLeft, _CameraProperties_UI.roiRight, _CameraProperties_UI.roiTop, _CameraProperties_UI.roiBottom))
					{
						_CameraProperties_UI.roiRight = _CameraProperties_Active.roiRight;
						_CameraProperties_UI.widthPx = _CameraProperties_Active.widthPx;
					}
				}
			}
			break;
		case ICamera::PARAM_CAPTURE_REGION_TOP:
			{			
				int val = static_cast<int>(param);
				if(_CameraProperties_UI.roiBottom >= val)
				{
					int imgHeight = (_CameraProperties_UI.roiBottom + 1) - val;
					// Check if the minimun height is less than the allowed minimum height
					if(_yRangeB[0] <= imgHeight)
					{
						// Check if the new height of the image is a multiple of heightValidation
						if(0 != imgHeight % heightValidation)
						{
							// If it isn't, substract from the entered value(round up the height), in order to make the new height a multiple of heightValidation
							_CameraProperties_UI.roiTop = val - (heightValidation - (imgHeight % heightValidation));
						}	
						else
						{
							_CameraProperties_UI.roiTop = val;
						}
					}
					else
					{
						//Change the Top value, so the image size is at the minimum height
						_CameraProperties_UI.roiTop = _CameraProperties_UI.roiBottom - _yRangeB[0];
					}

					// Check if the new value is within limits, reset to full frame if it isn't
					if(_CameraProperties_UI.roiTop < _yRangeT[0] || _CameraProperties_UI.roiTop  > _yRangeT[1])
					{
						_CameraProperties_UI.roiTop = _yRangeT[0];
					}
					if (TRUE == SetROI(_CameraProperties_UI.roiLeft, _CameraProperties_UI.roiRight, _CameraProperties_UI.roiTop, _CameraProperties_UI.roiBottom))
					{
						SetROI(_CameraProperties_UI.roiLeft, _CameraProperties_UI.roiRight, _CameraProperties_UI.roiTop, _CameraProperties_UI.roiBottom);
						_CameraProperties_UI.roiTop = _CameraProperties_Active.roiTop;
						_CameraProperties_UI.heightPx = _CameraProperties_Active.heightPx;
					}
				}
			}
			break;
		case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
			{			
				int val = static_cast<int>(param) - 1;
				if(_CameraProperties_UI.roiTop <= val)
				{
					int imgHeight = (val + 1) - _CameraProperties_UI.roiTop;
					// Check if the minimun height is less than the allowed minimum height
					if(_yRangeB[0] <= imgHeight)
					{
						// Check if the new height of the image is a multiple of heightValidation
						if(0 != imgHeight % heightValidation)
						{
							// If it isn't, substract from the entered value(round up the height), in order to make the new height a multiple of heightValidation
							_CameraProperties_UI.roiBottom = val + (heightValidation - (imgHeight % heightValidation));
						}
						else
						{
							_CameraProperties_UI.roiBottom = val;	
						}
					}
					else
					{
						//Change the Bottom value, so the image size is at the minimum height
						_CameraProperties_UI.roiBottom = _CameraProperties_UI.roiTop + _yRangeB[0];
					}

					// Check if the new value is within limits, reset to full frame if it isn't
					if(_CameraProperties_UI.roiBottom < _yRangeB[0] || _CameraProperties_UI.roiBottom  > _yRangeB[1])
					{
						_CameraProperties_UI.roiBottom = _yRangeB[1];
					}

					if (TRUE == SetROI(_CameraProperties_UI.roiLeft, _CameraProperties_UI.roiRight, _CameraProperties_UI.roiTop, _CameraProperties_UI.roiBottom))
					{
						SetROI(_CameraProperties_UI.roiLeft, _CameraProperties_UI.roiRight, _CameraProperties_UI.roiTop, _CameraProperties_UI.roiBottom);
						_CameraProperties_UI.roiBottom = _CameraProperties_Active.roiBottom;
						_CameraProperties_UI.heightPx = _CameraProperties_Active.heightPx;
					}
				}
			}
			break;
		case ICamera::PARAM_TRIGGER_MODE:
			{
				if (LAST_TRIGGER_MODE >= param && FIRST_TRIGGER_MODE <= param)
				{					
					_CameraProperties_UI.triggerMode = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_MULTI_FRAME_COUNT:
			{
				//*** lift the limit to acquire more by free run mode ***//
				//long maxRange = (ICamera::AVG_MODE_CUMULATIVE == _CameraProperties_UI.averageMode) ? _frmPerTriggerRange[1] / max(1,_CameraProperties_UI.averageNum) : _frmPerTriggerRange[1];

				//if (maxRange >= param && _frmPerTriggerRange[0] <= param)
				//{
				_CameraProperties_UI.numFrame = static_cast<long>(param);
				//}
			}
			break;
		case ICamera::PARAM_EXPOSURE_TIME_MS:
			{
				long long valUS = static_cast<long long>(param * Constants::MS_TO_SEC);
				if (_expUSRange[1] >= valUS && _expUSRange[0] <= valUS)
				{
					ThorTSIErrChk(tl_camera_set_exposure_time(_cameraParams[_camID].cameraHandle, valUS), 1);
					isExposureNudged = true;
				}
			}
			break;
		case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER:
			{
				if (MIN_IMAGE_BUFFERS <= param && UINT_MAX >= param)
				{
					_CameraProperties_UI.numImagesToBuffer = static_cast<int>(param);
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
						_CameraProperties_UI.imageAngle = static_cast<unsigned long>(param);
					}
					else if (-90 >= static_cast<long>(param))
					{
						_CameraProperties_UI.imageAngle = 270;
					}
					else if (360 <= static_cast<long>(param))
					{
						_CameraProperties_UI.imageAngle = 0;
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
					_CameraProperties_UI.horizontalFlip = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP :
			{
				if (TRUE == param || FALSE == param)
				{
					_CameraProperties_UI.verticalFlip = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_AVERAGENUM:
			{		
				if (MAX_AVGNUM >= param && MIN_AVGNUM <= param)
				{
					_CameraProperties_UI.averageNum = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_AVERAGEMODE:
			{
				if (ICamera::AVG_MODE_NONE == param || ICamera::AVG_MODE_CUMULATIVE == param)
				{
					_CameraProperties_UI.averageMode = static_cast<int>(param);
				}
			}
			break;
		case ICamera::PARAM_READOUT_SPEED_INDEX:
			{
				int val = static_cast<int>(param) + TL_CAMERA_DATA_RATE_FPS_30;
				if (TL_CAMERA_DATA_RATE_MAX >= param && TL_CAMERA_DATA_RATE_READOUT_FREQUENCY_20 <= param)
				{
					_CameraProperties_UI.readOutSpeedIndex = static_cast<int>(val);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT:
			{
				if (MAX_DMABUFNUM >= param && MIN_DMABUFNUM <= param)
				{
					_CameraProperties_UI.dmaBufferCount = static_cast<long>(param);
				}
			}
			break;
		case ICamera::PARAM_CAMERA_LED_ENABLE:
			{
				if (1 >= param && 0 <= param)
				{
					ThorTSIErrChk(tl_camera_set_is_led_on(_cameraParams[_camID].cameraHandle, static_cast<int>(param)), 1);
				}
			}
			break;
		case ICamera::PARAM_HOT_PIXEL_THRESHOLD_VALUE:
			{
				if (_hotPixelRange[1] >= param && _hotPixelRange[0] <= param)
				{
					int hotPixelThreshold = static_cast<int>(param);
					ThorTSIErrChk(tl_camera_set_hot_pixel_correction_threshold(_cameraParams[_camID].cameraHandle, hotPixelThreshold), 1);
					ThorTSIErrChk(tl_camera_get_hot_pixel_correction_threshold(_cameraParams[_camID].cameraHandle, &hotPixelThreshold), 1);
					_CameraProperties_UI.hotPixelThreshold = hotPixelThreshold;
					_CameraProperties_Active.hotPixelThreshold = hotPixelThreshold;
					_cameraParams[_camID].resyncFlag = 1;
				}
			}
			break;
		case ICamera::PARAM_HOT_PIXEL_ENABLED:
			{
				if (1 >= param && 0 <= param)
				{
					ThorTSIErrChk(tl_camera_set_is_hot_pixel_correction_enabled(_cameraParams[_camID].cameraHandle, static_cast<int>(param)), 1);
					_CameraProperties_UI.hotPixelThreshold = static_cast<int>(param);
					_CameraProperties_Active.hotPixelThreshold = static_cast<int>(param);
					_cameraParams[_camID].resyncFlag = 1;
				}
			}
			break;
		case ICamera::PARAM_GAIN:
			{
				if (_gainRange[1] >= param && _gainRange[0] <= param)
				{
					_CameraProperties_UI.gain = static_cast<long>(param);
				}
			}
			break;
		case ICamera::PARAM_GAIN_DECIBELS:
			{
				int gainIdx = 0;
				ThorTSIErrChk(tl_camera_convert_decibels_to_gain(_cameraParams[_camID].cameraHandle, param, &gainIdx) ,FALSE);
				if (_gainRange[1] >= gainIdx && _gainRange[0] <= gainIdx)
				{
					_CameraProperties_UI.gain = static_cast<long>(gainIdx);
				}
			}
			break;
		case ICamera::PARAM_OPTICAL_BLACK_LEVEL:
			{
				if (_blackLevelRange[1] >= param && _blackLevelRange[0] <= param)
				{
					_CameraProperties_UI.blackLevel = static_cast<long>(param);
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
				_CameraProperties_UI.frameRateControlEnabled = static_cast<long>(param);
				//ThorTSIErrChk(tl_camera_set_is_frame_rate_control_enabled(_cameraParams[_camID].cameraHandle, _imgPtyDll.frameRateControlEnabled), 1);
			}
		}
		break;
		case ICamera::PARAM_CAMERA_FRAME_RATE_CONTROL_VALUE:
		{
			if (_frameRateControlValueRange[1] >= param && _frameRateControlValueRange[0] <= param)
			{
				ThorTSIErrChk(tl_camera_set_frame_rate_control_value(_cameraParams[_camID].cameraHandle, param), 1);
				isFrameRateControlNudged = true;
				isExposureNudged = true;
			}
		}
		break;
		case ICamera::PARAM_CAMERA_COLOR_IMAGE_TYPE:
		{
			// 0 = Unprocessed
			// 1 = sRGB
			// 2 = Linear sRGB
			if (0 <= param && 2 >= param)
			{
				_CameraProperties_UI.colorImageType = static_cast<int>(param);
				_CameraProperties_UI.channelBitmask = _CameraProperties_UI.colorImageType == 0 ? 0b0001 : 0b0111;
			}
		}
		break;
		case ICamera::PARAM_CAMERA_POLAR_IMAGE_TYPE:
		{
			// 0=Unprocessed
			// 1=Intensity 
			// 2=DoLP
			// 3=Azimuth
			// 4=Quadview

			if (0 <= param && 4 >= param)
			{
				_CameraProperties_UI.polarImageType = static_cast<int>(param);
			}
		}
		break;
		case ICamera::PARAM_CAMERA_EEP_ENABLE:
		{
			// TODO: runtime variables?
			_CameraProperties_UI.isEqualExposurePulseEnabled = static_cast<int>(param);
		}
		break;
		case ICamera::PARAM_CAMERA_EEP_WIDTH:
		{
			if (0.0 <= param && _expUSRange[1] >= param)
			{
				_CameraProperties_UI.equalExposurePulseWidth = param;
			}
		}
		break;
		case ICamera::PARAM_CAMERA_RESYNC_FLAG:
		{
			_cameraParams[_camID].resyncFlag = param != 0;
		}
		break;
		case ICamera::PARAM_CAMERA_RED_GAIN:
		{
			_CameraProperties_UI.redGain = param;
		}
		break;
		case ICamera::PARAM_CAMERA_GREEN_GAIN:
		{
			_CameraProperties_UI.greenGain = param;
		}
		break;
		case ICamera::PARAM_CAMERA_BLUE_GAIN:
		{
			_CameraProperties_UI.blueGain = param;
		}
		break;
		case ICamera::PARAM_CAMERA_IS_CONTINUOUS_WHITE_BALANCE_ENABLED:
		{
			_CameraProperties_UI.isAutoWhiteBalanceEnabled = param;
			_CameraProperties_Active.isAutoWhiteBalanceEnabled = param;
		}
		break;
		case ICamera::PARAM_CAMERA_CONTINUOUS_WHITE_BALANCE_NUM_FRAMES:
		{
			_CameraProperties_UI.whiteBalanceFrameCount = (int)param;
			_CameraProperties_Active.whiteBalanceFrameCount = (int)param;
		}
		break;
		case ICamera::PARAM_CAMERA_ONE_SHOT_WHITE_BALANCE_FLAG:
		{
			_CameraProperties_UI.oneShotWhiteBalanceFlag = param;
			_CameraProperties_Active.oneShotWhiteBalanceFlag = param;
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

		// Handle nudges from dynamic parameters:
		if (isFrameRateControlNudged)
		{
			double val = 0.0;
			ThorTSIErrChk(tl_camera_get_frame_rate_control_value(_cameraParams[_camID].cameraHandle, &val), 1);
			_CameraProperties_Active.frameRateControlValue = val;
			_CameraProperties_UI.frameRateControlValue = val;
			_cameraParams[_camID].resyncFlag = true;
		}
		if (isExposureNudged)
		{
			long long val = 0;
			ThorTSIErrChk(tl_camera_get_exposure_time(_cameraParams[_camID].cameraHandle, &val), 1);
			_CameraProperties_Active.exposureTime_us = (int)val;
			_CameraProperties_UI.exposureTime_us = (int)val;
			_cameraParams[_camID].resyncFlag = true;
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
	// TODO: make sure members are not modified in getter

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
			param = (double) _CameraProperties_UI.exposureTime_us / Constants::MS_TO_SEC;
			break;
			// ROI and Binning are special cases 
		case ICamera::PARAM_BINNING_X:
			param = _CameraProperties_UI.roiBinX;
			break;
		case ICamera::PARAM_BINNING_Y:
			param = _CameraProperties_UI.roiBinY;
			break;
		case ICamera::PARAM_CAPTURE_REGION_LEFT:
			param = (double) _CameraProperties_UI.roiLeft;
			break;
		case ICamera::PARAM_CAPTURE_REGION_RIGHT:
			param = (double) _CameraProperties_UI.roiRight + 1;
			break;
		case ICamera::PARAM_CAPTURE_REGION_TOP:
			param = (double) _CameraProperties_UI.roiTop;
			break;
		case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
			param = (double) _CameraProperties_UI.roiBottom + 1;
			break;
		case ICamera::PARAM_TRIGGER_MODE:
			{
				param = _CameraProperties_UI.triggerMode;
			}
			break;
		case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER :
			{
				param = _CameraProperties_UI.numImagesToBuffer;
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_ANGLE:
			{
				param = _CameraProperties_UI.imageAngle;
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP:
			{
				param = _CameraProperties_UI.horizontalFlip;
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP:
			{
				param = _CameraProperties_UI.verticalFlip;
			}
			break;
		case ICamera::PARAM_DROPPED_FRAMES:
			{
				param = static_cast<double>(_availableFramesCnt);
			}
			break;
		case ICamera::PARAM_CAMERA_AVERAGENUM:
			{
				param = _CameraProperties_UI.averageNum;
			}
			break;
		case ICamera::PARAM_CAMERA_AVERAGEMODE:
			{
				param = _CameraProperties_UI.averageMode;
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_WIDTH:
			{
				// TODO: commented out the tl_camera_get_sensor... until the dependency with _CameraProperties is understood
				// ex: ROI could be set in UI, but the tl_camera_get will not reflect that unless acquisition is started

				//if the angle is not 0 or 180 we need to change the binning and width/height accordingly
				if (0 != _CameraProperties_UI.imageAngle && 180 != _CameraProperties_UI.imageAngle)
				{ 
					//ThorTSIErrChk(tl_camera_get_sensor_height(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.heightPx), 1);
					param = _CameraProperties_UI.heightPx;
				}
				else
				{
					//ThorTSIErrChk(tl_camera_get_sensor_width(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.widthPx), 1);
					param = _CameraProperties_UI.widthPx;
				}
			}
			break;
		case ICamera::PARAM_CAMERA_IMAGE_HEIGHT:
			{
				//if the angle is not 0 or 180 we need to change the binning and width/height accordingly
				if (0 != _CameraProperties_UI.imageAngle && 180 != _CameraProperties_UI.imageAngle)
				{
					//ThorTSIErrChk(tl_camera_get_sensor_width(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.widthPx), 1);
					param = _CameraProperties_UI.widthPx;
				}
				else
				{
					//ThorTSIErrChk(tl_camera_get_sensor_height(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.heightPx), 1);
					param = _CameraProperties_UI.heightPx;
				}
			}
			break;
		case ICamera::PARAM_BITS_PER_PIXEL:
			{
				param = _CameraProperties_UI.bitPerPixel;
			}
			break;
		case ICamera::PARAM_PIXEL_SIZE:
			{
				param = _CameraProperties_UI.pixelSizeXUM;		//could be different in X | Y in future
			}
			break;
		case ICamera::PARAM_READOUT_SPEED_INDEX:
			{
				param = _CameraProperties_UI.readOutSpeedIndex - TL_CAMERA_DATA_RATE_FPS_30;
			}
			break;
		case ICamera::PARAM_FRAME_RATE:
			{
				// get the measured frame rate, which is calculated on the host as frames arrive from the camera
				// if it is zero (if no frames have been acquired yet), then use the camera's reported frame time
				double frameRate = 0;
				ThorTSIErrChk(tl_camera_get_measured_frame_rate(_cameraParams[_camID].cameraHandle, &frameRate), 1);
				if (0 == frameRate)
				{
					int frameTime_us = 0;
					ThorTSIErrChk(tl_camera_get_frame_time(_cameraParams[_camID].cameraHandle, &frameTime_us), 1);
					if (0 == frameTime_us)
					{
						ThorCam::getInstance()->LogMessage(L"CS camera reported a zero frame time", INFORMATION_EVENT);
					}
					else
					{
						frameRate = 1.0 / (static_cast<double>(frameTime_us) / 1000000);
					}
				}
				param = frameRate;
			}
			break;
		case ICamera::PARAM_CAMERA_FRAME_TIME:
		{
			int frameTime_us = 0;
			ThorTSIErrChk(tl_camera_get_frame_time(_cameraParams[_camID].cameraHandle, &frameTime_us), 1);
			param = frameTime_us * 1000;
		}
		break;
		case ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT:
			{
				param = _CameraProperties_UI.dmaBufferCount;
			}
			break;
		case ICamera::PARAM_CAMERA_CHANNEL: 
			{
				// bitmask; NOT number of channels
				param = _CameraProperties_UI.channelBitmask;
			}
			break;
		case ICamera::PARAM_CAMERA_LED_AVAILABLE:
			{
				int ledAvailable = FALSE;
				ThorTSIErrChk(tl_camera_get_is_led_supported(_cameraParams[_camID].cameraHandle, &ledAvailable), 1);
				param = ledAvailable;
			}
			break;
		case ICamera::PARAM_CAMERA_LED_ENABLE:
			{
				int ledEnable = 1;
				ThorTSIErrChk(tl_camera_get_is_led_on(_cameraParams[_camID].cameraHandle, &ledEnable), 1);
				param = ledEnable;
			}
			break;
		case ICamera::PARAM_HOT_PIXEL_THRESHOLD_VALUE:
			{
				int hotPixelThresholdVal = 0;
				ThorTSIErrChk(tl_camera_get_hot_pixel_correction_threshold(_cameraParams[_camID].cameraHandle, &hotPixelThresholdVal), FALSE);
				param = hotPixelThresholdVal;
			}
			break;
		case ICamera::PARAM_HOT_PIXEL_ENABLED:
			{
				int hotPixelEnabled = 0;
				ThorTSIErrChk(tl_camera_get_is_hot_pixel_correction_enabled(_cameraParams[_camID].cameraHandle, &hotPixelEnabled), FALSE);
				param = hotPixelEnabled;
			}
			break;
		case ICamera::PARAM_GAIN:
			{
				param = _CameraProperties_UI.gain;
			}
			break;
		case ICamera::PARAM_GAIN_DECIBELS:
			{
				ThorTSIErrChk(tl_camera_convert_gain_to_decibels(_cameraParams[_camID].cameraHandle, _CameraProperties_UI.gain, &param), FALSE);
			}
			break;
		case ICamera::PARAM_GAIN_TYPE:
			{
				int gainMin = 0; int gainMax = 0;
				ThorTSIErrChk(tl_camera_get_gain_range(_cameraParams[_camID].cameraHandle, &gainMin, &gainMax), FALSE);
				int gainRange = gainMax - gainMin;
				if (gainRange == 0)
				{
					param = GainType::UNSUPPORTED;
				}
				else if (gainRange < 4)
				{
					param = GainType::DISCRETE_DECIBELS;
				}
				else
				{
					param = GainType::CONTIGUOUS_DECIBELS;
				}
			}
			break;
		case ICamera::PARAM_OPTICAL_BLACK_LEVEL:
			{
				param = _CameraProperties_UI.blackLevel;
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
			param = _CameraProperties_UI.frameRateControlEnabled;
			//ThorTSIErrChk(tl_camera_get_is_frame_rate_control_enabled(_cameraParams[_camID].cameraHandle, &frameRateControlEnabled), 1);
			//param = frameRateControlEnabled;
		}
		break;
		case ICamera::PARAM_CAMERA_FRAME_RATE_CONTROL_VALUE:
		{
			double frameRateControlValue = 0;
			param = _CameraProperties_UI.frameRateControlValue;
			/*ThorTSIErrChk(tl_camera_get_frame_rate_control_value(_cameraParams[_camID].cameraHandle, &frameRateControlValue), 1);
			param = frameRateControlValue;*/
		}
		break;
		case ICamera::PARAM_CAMERA_COLOR_IMAGE_TYPE:
		{
			param = _CameraProperties_UI.colorImageType;
		}
		break;
		case ICamera::PARAM_CAMERA_POLAR_IMAGE_TYPE:
		{
			param = _CameraProperties_UI.polarImageType;
		}
		break;
		case ICamera::PARAM_CAMERA_SENSOR_TYPE:
		{
			param = _cameraParams[_camID].cameraSensorType;
		}
		break;
		case ICamera::PARAM_CAMERA_RED_GAIN:
		{
			param = _CameraProperties_UI.redGain;
		}
		break;
		case ICamera::PARAM_CAMERA_GREEN_GAIN:
		{
			param = _CameraProperties_UI.greenGain;
		}
		break;
		case ICamera::PARAM_CAMERA_BLUE_GAIN:
		{
			param = _CameraProperties_UI.blueGain;
		}
		break;
		case ICamera::PARAM_CAMERA_EEP_ENABLE:
		{
			param = _CameraProperties_UI.isEqualExposurePulseEnabled;
		}
		break;
		case ICamera::PARAM_CAMERA_EEP_WIDTH:
		{
			param = _CameraProperties_UI.equalExposurePulseWidth;
		}
		break;
		case ICamera::PARAM_CAMERA_EEP_STATUS:
		{
			TL_CAMERA_EEP_STATUS status;
			ThorTSIErrChk(tl_camera_get_eep_status(_cameraParams[_camID].cameraHandle, &status), TRUE);
			param = static_cast<double>(status);
		}
		break;
		case ICamera::PARAM_CAMERA_RESYNC_FLAG:
		{
			param = static_cast<long>(_cameraParams[_camID].resyncFlag);
		}
		break;
		case ICamera::PARAM_CAMERA_IS_CONTINUOUS_WHITE_BALANCE_ENABLED:
		{
			param = _CameraProperties_UI.isAutoWhiteBalanceEnabled;
		}
		break;
		case ICamera::PARAM_CAMERA_CONTINUOUS_WHITE_BALANCE_NUM_FRAMES:
		{
			param = _CameraProperties_UI.whiteBalanceFrameCount;
		}
		break;
		case ICamera::PARAM_CAMERA_ONE_SHOT_WHITE_BALANCE_FLAG:
		{
			param = _CameraProperties_UI.oneShotWhiteBalanceFlag;
		}
		break;
		case ICamera::PARAM_CAMERA_IS_AUTOEXPOSURE_SUPPORTED:
		{
			param = true;
		}
		break;
		default:
			ret = FALSE;
			break;
		}

		// Handle dynamic parameters here; parameters that can be changed without disarming the camera

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
			paramDefault	= _CameraProperties_UI.bitPerPixel;
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_WIDTH:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _widthRange[0];
			paramMax		= _widthRange[1];
			paramDefault	= _CameraProperties_UI.widthPx;
			paramReadOnly	= TRUE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_HEIGHT:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _heightRange[0];
			paramMax		= _heightRange[1];
			paramDefault	= _CameraProperties_UI.heightPx;
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
			paramDefault	= _CameraProperties_UI.pixelSizeXUM;		//could be different in X | Y in future
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
		case ICamera::PARAM_CAMERA_FRAME_TIME:
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = MAXULONG32;
			paramDefault = 0;
			paramReadOnly = TRUE;
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
			paramDefault	= _CameraProperties_UI.hotPixelThreshold;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_HOT_PIXEL_ENABLED:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= _CameraProperties_UI.hotPixelEnabled;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_GAIN:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _gainRange[0];
			paramMax		= _gainRange[1];
			paramDefault	= _CameraProperties_UI.gain;
			paramReadOnly	= FALSE;
			break;
		case ICamera::PARAM_GAIN_DECIBELS:
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			ThorTSIErrChk(tl_camera_convert_gain_to_decibels(_cameraParams[_camID].cameraHandle, _gainRange[0], &paramMin), FALSE);
			ThorTSIErrChk(tl_camera_convert_gain_to_decibels(_cameraParams[_camID].cameraHandle, _gainRange[1], &paramMax), FALSE);
			ThorTSIErrChk(tl_camera_convert_gain_to_decibels(_cameraParams[_camID].cameraHandle, _CameraProperties_UI.gain, &paramDefault), FALSE);
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_GAIN_DISCRETE_DECIBELS_VALUES:
			paramType = ICamera::TYPE_BUFFER;
			paramAvailable = TRUE;
			ThorTSIErrChk(tl_camera_convert_gain_to_decibels(_cameraParams[_camID].cameraHandle, _gainRange[0], &paramMin), FALSE);
			ThorTSIErrChk(tl_camera_convert_gain_to_decibels(_cameraParams[_camID].cameraHandle, _gainRange[1], &paramMax), FALSE);
			paramDefault = 0.0;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_GAIN_TYPE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = (int)GainType::GAIN_TYPE_LAST;
			paramDefault = (int)GainType::UNSUPPORTED;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_OPTICAL_BLACK_LEVEL:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= _blackLevelRange[0];
			paramMax		= _blackLevelRange[1];
			paramDefault	= _CameraProperties_UI.blackLevel;
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
		case ICamera::PARAM_CAMERA_COLOR_IMAGE_TYPE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 0; //Unprocessed
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_POLAR_IMAGE_TYPE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 4;
			paramDefault = 0; //Unprocessed
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_RED_GAIN:
			float matrixR[9];
			ThorTSIErrChk(tl_camera_get_default_white_balance_matrix(_cameraParams[_camID].cameraHandle, matrixR), TRUE);
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = _cameraParams[_camID].cameraSensorType == TL_CAMERA_SENSOR_TYPE_BAYER;
			paramMin = 0.0; // Comes from Thorcam
			paramMax = 10.0; // Comes from Thorcam
			paramDefault = static_cast<double>(matrixR[0]);
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_GREEN_GAIN:
			float matrixG[9];
			ThorTSIErrChk(tl_camera_get_default_white_balance_matrix(_cameraParams[_camID].cameraHandle, matrixG), TRUE);
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = _cameraParams[_camID].cameraSensorType == TL_CAMERA_SENSOR_TYPE_BAYER;
			paramMin = 0.0; // Comes from Thorcam
			paramMax = 10.0; // Comes from Thorcam
			paramDefault = static_cast<double>(matrixG[4]);
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_BLUE_GAIN:
			float matrixB[9];
			ThorTSIErrChk(tl_camera_get_default_white_balance_matrix(_cameraParams[_camID].cameraHandle, matrixB), TRUE);
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = _cameraParams[_camID].cameraSensorType == TL_CAMERA_SENSOR_TYPE_BAYER;
			paramMin = 0.0; // Comes from Thorcam
			paramMax = 10.0; // Comes from Thorcam
			paramDefault = static_cast<double>(matrixB[8]);
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_SENSOR_TYPE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 0; // Monochrome
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_LSM_NUMBER_OF_PLANES:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 1;
			paramMax = 1;
			paramDefault = 1;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_CAMERA_EEP_ENABLE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = _cameraParams[_camID].isEqualExposurePulseSupported;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_EEP_WIDTH:
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = _cameraParams[_camID].isEqualExposurePulseSupported;
			paramMin = 0.0;//static_cast<double>(_expUSRange[0] / Constants::MS_TO_SEC);
			paramMax = static_cast<double>(_expUSRange[1] / Constants::MS_TO_SEC);
			paramDefault = 1.0;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_EEP_STATUS:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = _cameraParams[_camID].isEqualExposurePulseSupported;
			paramMin = TL_CAMERA_EEP_STATUS_DISABLED;
			paramMax = TL_CAMERA_EEP_STATUS_MAX;
			TL_CAMERA_EEP_STATUS defaultVal;
			ThorTSIErrChk(tl_camera_get_eep_status(_cameraParams[_camID].cameraHandle, &defaultVal), TRUE);
			paramDefault = defaultVal;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_CAMERA_RESYNC_FLAG:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = true;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_IS_CONTINUOUS_WHITE_BALANCE_ENABLED:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = _cameraParams[_camID].cameraSensorType == TL_CAMERA_SENSOR_TYPE_BAYER;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_CONTINUOUS_WHITE_BALANCE_NUM_FRAMES:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = _cameraParams[_camID].cameraSensorType == TL_CAMERA_SENSOR_TYPE_BAYER;
			paramMin = 1;
			paramMax = 16; // TODO: what is the max?
			paramDefault = 5;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_ONE_SHOT_WHITE_BALANCE_FLAG:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = _cameraParams[_camID].cameraSensorType == TL_CAMERA_SENSOR_TYPE_BAYER;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_IS_AUTOEXPOSURE_SUPPORTED:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = true;
			paramMin = 0;
			paramMax = 1;
			paramDefault = true;
			paramReadOnly = TRUE;
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
	long ret = TRUE;
	switch (paramID)
	{
	case ICamera::PARAM_GAIN_DISCRETE_DECIBELS_VALUES:
	{
		int gain_min = 0; int gain_max = 0;
		ThorTSIErrChk(tl_camera_get_gain_range(_cameraParams[_camID].cameraHandle, &gain_min, &gain_max), FALSE);
		int ptr = 0;
		double* dblBuf = (double*)buf; // note: we trust that the caller knows that PARAM_GAIN_DISCRETE_DECIBELS_VALUES requires a double array, and "size" refers to that double array (NOT buffer size in bytes)
		for (int i = gain_min; i <= gain_max && ptr < size; i++, ptr++)
		{
			ThorTSIErrChk(tl_camera_convert_gain_to_decibels(_cameraParams[_camID].cameraHandle, i, &dblBuf[ptr]), FALSE);
		}
		break;
	}
	default:
	{
		//no parameter found, return FALSE
		ret = FALSE;
	}
	}
	return ret;
}
