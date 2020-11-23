#include "stdafx.h"
#include "ORCA.h"

long ORCA::SetParam(const long paramID, const double param)
{
	long ret = TRUE;
	DCAMERR err;
	try
	{
		switch (paramID)
		{
		case ICamera::PARAM_BINNING_X:
		{
			if ((_hbinRange[1] >= static_cast<int>(param)) && (_hbinRange[0] <= static_cast<int>(param)))
			{
				if (FALSE == _singleBinning)
				{
					_ImgPty_SetSettings.roiBinX = static_cast<int>(param);
				}
			}
		}
		break;
		case ICamera::PARAM_BINNING_Y:
		{
			if ((_vbinRange[1] >= static_cast<int>(param)) && (_vbinRange[0] <= static_cast<int>(param)))
			{
				if (FALSE == _singleBinning)
				{
					_ImgPty_SetSettings.roiBinY = static_cast<int>(param);
				}
			}
		}
		break;

		case ICamera::PARAM_CAPTURE_REGION_LEFT:
		{
			int val = static_cast<int>(param);
			if ((_xRangeL[1] >= val) && (_xRangeL[0] <= val) && (_ImgPty_SetSettings.roiRight >= val))
			{
				int imgWidth = (_ImgPty_SetSettings.roiRight + 1) - val;
				// Check if the image width is less than the allowed minimum, in this case for the ORCA the minimum is 1
				if (_xRangeR[0] <= imgWidth)
				{
					// Check if the new width of the image is a multiple of 4
					if (0 != imgWidth % 4)
					{
						// If it isn't, substract from the entered value(round up the width), in order to make the width a multiple of 4
						_ImgPty_SetSettings.roiLeft = val - (4 - (imgWidth % 4));
						// Check if the new value is within limits, reset to full frame if it isn't
						if (_ImgPty_SetSettings.roiLeft < _xRangeL[0])
						{
							_ImgPty_SetSettings.roiLeft = _xRangeL[0];
							_ImgPty_SetSettings.roiRight = _xRangeR[1];
						}
					}
					else
					{
						_ImgPty_SetSettings.roiLeft = val;
					}
				}
				else
				{
					_ImgPty_SetSettings.roiLeft = _ImgPty_SetSettings.roiRight - _xRangeR[0];
				}
			}
		}
		break;
		case ICamera::PARAM_CAPTURE_REGION_RIGHT:
		{
			int val = static_cast<int>(param) - 1;
			if ((_xRangeR[1] >= val) && ((_xRangeR[0]) <= val) && (_ImgPty_SetSettings.roiLeft <= val))
			{
				int imgWidth = (val + 1) - _ImgPty_SetSettings.roiLeft;
				// Check if the image width is less than the allowed minimum, in this case for the ORCA the minimum is 1
				if (_xRangeR[0] <= imgWidth)
				{
					// Check if the new width of the image is a multiple of 4
					if (0 != imgWidth % 4)
					{
						// If it isn't, substract from the entered value(round up the width), in order to make the width a multiple of 4
						_ImgPty_SetSettings.roiRight = val + (4 - (imgWidth % 4));
						// Check if the new value is within limits, reset to full frame if it isn't
						if (_ImgPty_SetSettings.roiRight > _xRangeR[1])
						{
							_ImgPty_SetSettings.roiLeft = _xRangeL[0];
							_ImgPty_SetSettings.roiRight = _xRangeR[1];
						}
					}
					else
					{
						_ImgPty_SetSettings.roiRight = val;
					}
				}
				else
				{
					_ImgPty_SetSettings.roiRight = _ImgPty_SetSettings.roiLeft + _xRangeR[0];
				}
			}
		}
		break;
		case ICamera::PARAM_CAPTURE_REGION_TOP:
		{
			int val = static_cast<int>(param);
			if ((_yRangeT[1] >= val) && (_yRangeT[0] <= val) && (_ImgPty_SetSettings.roiBottom >= val))
			{
				int imgHeight = (_ImgPty_SetSettings.roiBottom + 1) - val;
				// Check if the minimun height is less than the allowed minimum (for ORCA is 1 pixel)
				if (_yRangeB[0] <= imgHeight)
				{
					// Check if the new width of the image is a multiple of 4
					if (0 != imgHeight % 4)
					{
						// If it isn't, substract from the entered value(round up the width), in order to make the width a multiple of 4
						_ImgPty_SetSettings.roiTop = val - (4 - (imgHeight % 4));
						// Check if the new value is within limits, reset to full frame if it isn't
						if (_ImgPty_SetSettings.roiTop < _yRangeT[0])
						{
							_ImgPty_SetSettings.roiTop = _yRangeT[0];
							_ImgPty_SetSettings.roiBottom = _yRangeB[1];
						}
					}
					else
					{
						_ImgPty_SetSettings.roiTop = val;
					}
				}
				else
				{
					_ImgPty_SetSettings.roiTop = _ImgPty_SetSettings.roiBottom - _yRangeB[0];
				}
			}
		}
		break;
		case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
		{
			int val = static_cast<int>(param) - 1;
			if ((_yRangeB[1] >= val) && ((_yRangeB[0]) <= val) && (_ImgPty_SetSettings.roiTop <= val))
			{
				int imgHeight = (val + 1) - _ImgPty_SetSettings.roiTop;
				// Check if the minimun height is less than the allowed minimum (for ORCA is 1 pixel)
				if (_yRangeB[0] <= imgHeight)
				{
					// Check if the new width of the image is a multiple of 4
					if (0 != imgHeight % 4)
					{
						// If it isn't, substract from the entered value(round up the width), in order to make the width a multiple of 4
						_ImgPty_SetSettings.roiBottom = val + (4 - (imgHeight % 4));
						// Check if the new value is within limits, reset to full frame if it isn't
						if (_ImgPty_SetSettings.roiBottom > _yRangeB[1])
						{
							_ImgPty_SetSettings.roiTop = _yRangeT[0];
							_ImgPty_SetSettings.roiBottom = _yRangeB[1];
						}
					}
					else
					{
						_ImgPty_SetSettings.roiBottom = val;
					}
				}
				else
				{
					// if it isn't, then push the bottom so the height is equal to the minimum
					_ImgPty_SetSettings.roiBottom = _ImgPty_SetSettings.roiTop + _yRangeB[0];
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
			if (UINT_MAX >= param && 0 <= param)
			{
				_ImgPty_SetSettings.numFrame = max(1, static_cast<long>(param));
			}
			//}
		}
		break;
		case ICamera::PARAM_EXPOSURE_TIME_MS:
		{
			DCAMERR err;
			int valUS = static_cast<int>(param * MS_TO_SEC); //Convert from miliseconds from GUI to microseconds
			if (valUS < _expUSRange[0])
			{
				valUS = _expUSRange[0];
			}
			if (valUS > _expUSRange[1])
			{
				valUS = _expUSRange[1];
			}
			double exposureSec = (double)valUS / (double)US_TO_SEC;
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_EXPOSURETIME, exposureSec));
			_ImgPty_SetSettings.exposureTime_us = valUS;
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
		case ICamera::PARAM_CAMERA_IMAGE_ANGLE:
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
		case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP:
		{
			if (TRUE == param || FALSE == param)
			{
				_ImgPty_SetSettings.horizontalFlip = static_cast<int>(param);
			}
		}
		break;
		case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP:
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
			int val = static_cast<int>(param) + 1;
			if (val >= _readOutSpeedRange[0] && val <= _readOutSpeedRange[1])
			{
				_ImgPty_SetSettings.readOutSpeedIndex = val;
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
				//OrcaErrChk();
			}
		}
		break;
		case ICamera::PARAM_HOT_PIXEL_THRESHOLD_VALUE:
		{
			if (_hotPixelRange[1] >= param && _hotPixelRange[0] <= param)
			{
				//OrcaErrChk();
				_ImgPty_SetSettings.hotPixelThreshold = static_cast<int>(param);
			}
		}
		break;
		case ICamera::PARAM_HOT_PIXEL_ENABLED:
		{
			if (1 >= param && 0 <= param)
			{
				//OrcaErrChk(;
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
		case ICamera::PARAM_BIN_INDEX:
		{
			if (TRUE == _singleBinning)
			{
				// Binning needs to be set when the camera has stopped, SetBDMA sets binning into the camera
				_ImgPty_SetSettings.binIndex = static_cast<long>(param);
				switch (_ImgPty_SetSettings.binIndex)
				{
				case 0:
				{
					_ImgPty_SetSettings.roiBinY = _ImgPty_SetSettings.roiBinX = 1;
				}
				break;
				case 1:
				{
					_ImgPty_SetSettings.roiBinY = _ImgPty_SetSettings.roiBinX = 2;
				}
				break;
				case 2:
				{
					_ImgPty_SetSettings.roiBinY = _ImgPty_SetSettings.roiBinX = 4;
				}
				break;
				default:
				{
					_ImgPty_SetSettings.roiBinY = _ImgPty_SetSettings.roiBinX = 1;
				}
				}
			}
		}
		break;
		case ICamera::PARAM_HOT_PIXEL_INDEX:
		{
			_ImgPty_SetSettings.hotPixelLevelIndex = static_cast<long>(param);
			switch (_ImgPty_SetSettings.hotPixelLevelIndex)
			{
			case 0:
			{
				OrcaErrChk(L"dcamprop_getvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_HOTPIXELCORRECT_LEVEL, DCAMPROP_HOTPIXELCORRECT_LEVEL__STANDARD));
			}
			break;
			case 1:
			{
				OrcaErrChk(L"dcamprop_getvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_HOTPIXELCORRECT_LEVEL, DCAMPROP_HOTPIXELCORRECT_LEVEL__MINIMUM));
			}
			break;
			case 2:
			{
				OrcaErrChk(L"dcamprop_getvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_HOTPIXELCORRECT_LEVEL, DCAMPROP_HOTPIXELCORRECT_LEVEL__AGGRESSIVE));
			}
			break;
			default:
			{
				OrcaErrChk(L"dcamprop_getvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_HOTPIXELCORRECT_LEVEL, DCAMPROP_HOTPIXELCORRECT_LEVEL__STANDARD));
			}
			}
		}
		break;
		default:
		{
			ret = FALSE;
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter (%d) not implemented", paramID);
			LogMessage(_errMsg, ERROR_EVENT);
		}
		break;
		}
	}
	catch (...)
	{
		ret = FALSE;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Set Parameter (%d) failed", paramID);
		LogMessage(_errMsg, ERROR_EVENT);
	}
	return ret;
}

long ORCA::GetParam(const long paramID, double& param)
{
	long ret = TRUE;
	DCAMERR err;

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
		case ICamera::PARAM_CCD_TYPE:
			param = ICamera::ORCA;
			break;
		case ICamera::PARAM_EXPOSURE_TIME_MS:
		{
			double exposure = static_cast<double>(_expUSRange[0]);
			OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_EXPOSURETIME, &exposure));
			_ImgPty_SetSettings.exposureTime_us = static_cast<int>(exposure * US_TO_SEC);
			param = (double)_ImgPty_SetSettings.exposureTime_us / Constants::MS_TO_SEC;
		}
		break;
		// ROI and Binning are special cases 
		case ICamera::PARAM_BINNING_X:
			param = _ImgPty_SetSettings.roiBinX;
			break;
		case ICamera::PARAM_BINNING_Y:
			param = _ImgPty_SetSettings.roiBinY;
			break;
		case ICamera::PARAM_CAPTURE_REGION_LEFT:
			param = (double)_ImgPty_SetSettings.roiLeft;
			break;
		case ICamera::PARAM_CAPTURE_REGION_RIGHT:
			param = (double)_ImgPty_SetSettings.roiRight + 1;
			break;
		case ICamera::PARAM_CAPTURE_REGION_TOP:
			param = (double)_ImgPty_SetSettings.roiTop;
			break;
		case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
			param = (double)_ImgPty_SetSettings.roiBottom + 1;
			break;
		case ICamera::PARAM_TRIGGER_MODE:
		{
			param = _ImgPty_SetSettings.triggerMode;
		}
		break;
		case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER:
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
			param = 0;
		}
		break;
		case ICamera::PARAM_DMA_BUFFER_AVAILABLE_FRAMES:
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
				if (0 != _ImgPty_SetSettings.roiBinY)
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
				if (0 != _ImgPty_SetSettings.roiBinX)
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
				if (0 != _ImgPty_SetSettings.roiBinY)
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
			param = static_cast<double>(_ImgPty_SetSettings.readOutSpeedIndex) - 1;
		}
		break;
		case ICamera::PARAM_FRAME_RATE:
		{
			double frameRate = 0;
			DCAMERR err;
			OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_INTERNALFRAMERATE, &frameRate));
			param = frameRate;
		}
		break;
		case ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT:
		{
			param = _imgPtyDll.dmaBufferCount;
		}
		break;
		case ICamera::PARAM_CAMERA_CHANNEL:
		{
			param = _ImgPty_SetSettings.channel;
		}
		break;
		case ICamera::PARAM_CAMERA_LED_AVAILABLE:
		{
			bool ledAvailable = FALSE;
			//OrcaErrChk();
			param = ledAvailable;
		}
		break;
		case ICamera::PARAM_CAMERA_LED_ENABLE:
		{
			int ledEnable = 1;
			//OrcaErrChk();
			param = ledEnable;
		}
		break;
		case ICamera::PARAM_HOT_PIXEL_THRESHOLD_VALUE:
		{
			int hotPixelThresholdVal = 0;
			//OrcaErrChk();
			param = hotPixelThresholdVal;
		}
		break;
		case ICamera::PARAM_HOT_PIXEL_ENABLED:
		{
			int hotPixelEnabled = 0;
			//OrcaErrChk();
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
		case ICamera::PARAM_BIN_INDEX:
		{
			double binning = 1.0;
			OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_BINNING, &binning));
			if (failed(err))
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"GetParam Parameter (%d) PARAM_BIN_INDEX failed", paramID);
				LogMessage(_errMsg, ERROR_EVENT);
				ret = FALSE;
			}
			//_ImgPty_SetSettings.roiBinY =_ImgPty_SetSettings.roiBinX = static_cast<long>(binning);
			//_ImgPty_SetSettings.binIndex = static_cast<int>(floor(binning / 2.0));
			param = _ImgPty_SetSettings.binIndex;
		}
		break;
		case ICamera::PARAM_HOT_PIXEL_INDEX:
		{
			if (TRUE == _paramHotPixelAvailable)
			{
				double hotPixelCorrection = DCAMPROP_HOTPIXELCORRECT_LEVEL__STANDARD;
				OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_HOTPIXELCORRECT_LEVEL, &hotPixelCorrection));
				if (failed(err))
				{
					_paramHotPixelAvailable = FALSE;
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetParam Parameter (%d) PARAM_HOT_PIXEL_INDEX failed", paramID);
					LogMessage(_errMsg, ERROR_EVENT);
					ret = FALSE;
				}
				_ImgPty_SetSettings.hotPixelLevelIndex = static_cast<int>(hotPixelCorrection - 1);
				param = _ImgPty_SetSettings.hotPixelLevelIndex;
			}
		}
		break;
		default:
			ret = FALSE;
			break;
		}
	}
	catch (...)
	{
		ret = FALSE;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetParam Parameter (%d) failed", paramID);
		LogMessage(_errMsg, ERROR_EVENT);
	}

	return ret;
}

long ORCA::GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault)
{
	long ret = TRUE;

	try
	{
		switch (paramID)
		{
		case ICamera::PARAM_CAMERA_TYPE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = ICamera::CCD;
			paramMax = ICamera::CCD;
			paramDefault = ICamera::CCD;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_CCD_TYPE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = ICamera::ORCA;
			paramMax = ICamera::ORCA;
			paramDefault = ICamera::ORCA;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_CONSOLE_WRITE:
			paramType = ICamera::TYPE_STRING;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CONSOLE_READ:
			paramType = ICamera::TYPE_STRING;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_DETECTOR_NAME:
			paramType = ICamera::TYPE_STRING;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_DETECTOR_SERIAL_NUMBER:
			paramType = ICamera::TYPE_STRING;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_TRIGGER_MODE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = ICamera::FIRST_TRIGGER_MODE;
			paramMax = static_cast<double>(ICamera::LAST_TRIGGER_MODE) - 1;
			paramDefault = ICamera::SW_FREE_RUN_MODE;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_IMAGE_BUFFERS;
			paramMax = UINT_MAX;
			paramDefault = MIN_IMAGE_BUFFERS;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAPTURE_REGION_LEFT:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _xRangeL[0];
			paramMax = _xRangeL[1];
			paramDefault = _xRangeL[0];
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAPTURE_REGION_RIGHT:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _xRangeR[0];
			paramMax = static_cast<double>(_xRangeR[1]) + 1;	//width = _xRangeR[1]-_xRangeR[0]+1
			paramDefault = _xRangeR[1];
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAPTURE_REGION_TOP:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _yRangeT[0];
			paramMax = _yRangeT[1];
			paramDefault = _yRangeT[0];
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _yRangeB[0];
			paramMax = static_cast<double>(_yRangeB[1]) + 1;	//height = _yRangeB[1]-_yRangeB[0]+1
			paramDefault = _yRangeB[1];
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_BINNING_X:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _hbinRange[0];
			paramMax = _hbinRange[1];
			paramDefault = DEFAULT_XBIN;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_BINNING_Y:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _vbinRange[0];
			paramMax = _vbinRange[1];
			paramDefault = DEFAULT_YBIN;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_MULTI_FRAME_COUNT:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = MAXLONG32;
			paramDefault = DEFAULT_FRM_PER_TRIGGER;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_ANGLE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_ANGLE;
			paramMax = MAX_ANGLE;
			paramDefault = DEFAULT_ANGLE;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_EXPOSURE_TIME_MS:
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = static_cast<double>(_expUSRange[0] / Constants::MS_TO_SEC);
			paramMax = static_cast<double>(_expUSRange[1] / Constants::MS_TO_SEC);
			paramDefault = DEFAULT_EXPOSURE_MS;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_BITS_PER_PIXEL:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_BITS_PERPIXEL;
			paramMax = MAX_BITS_PERPIXEL;
			paramDefault = _ImgPty_SetSettings.bitPerPixel;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_WIDTH:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _widthRange[0];
			paramMax = _widthRange[1];
			paramDefault = _ImgPty_SetSettings.widthPx;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_CAMERA_IMAGE_HEIGHT:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _heightRange[0];
			paramMax = _heightRange[1];
			paramDefault = _ImgPty_SetSettings.heightPx;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_DROPPED_FRAMES:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = INT_MAX;
			paramDefault = 0;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_DMA_BUFFER_AVAILABLE_FRAMES:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = INT_MAX;
			paramDefault = 0;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_CAMERA_AVERAGENUM:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_AVGNUM;
			paramMax = MAX_AVGNUM;
			paramDefault = DEFAULT_AVGNUM;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_AVERAGEMODE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = ICamera::AVG_MODE_NONE;
			paramMax = ICamera::AVG_MODE_CUMULATIVE;
			paramDefault = ICamera::AVG_MODE_NONE;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_PIXEL_SIZE:
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = MAXULONG32;
			paramDefault = _ImgPty_SetSettings.pixelSizeXUM;		//could be different in X | Y in future
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_READOUT_SPEED_INDEX:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = static_cast<double>(_readOutSpeedRange[0]) - 1;
			paramMax = static_cast<double>(_readOutSpeedRange[1]) - 1;
			paramDefault = static_cast<double>(_readOutSpeedRange[0]) - 1;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_FRAME_RATE:
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = MAXULONG32;
			paramDefault = 0;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT:
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = MIN_DMABUFNUM;
			paramMax = MAX_DMABUFNUM;
			paramDefault = DEFAULT_DMABUFNUM;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_CHANNEL:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_CHANNEL;
			paramMax = MAX_CHANNEL;
			paramDefault = DEFAULT_CHANNEL;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_CAMERA_LED_AVAILABLE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = TRUE;
			paramReadOnly = TRUE;
			break;
		case ICamera::PARAM_CAMERA_LED_ENABLE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = TRUE;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_HOT_PIXEL_THRESHOLD_VALUE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _hotPixelRange[0];
			paramMax = _hotPixelRange[1];
			paramDefault = _ImgPty_SetSettings.hotPixelThreshold;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_HOT_PIXEL_ENABLED:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = _ImgPty_SetSettings.hotPixelEnabled;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_GAIN:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _gainRange[0];
			paramMax = _gainRange[1];
			paramDefault = _ImgPty_SetSettings.gain;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_OPTICAL_BLACK_LEVEL:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _blackLevelRange[0];
			paramMax = _blackLevelRange[1];
			paramDefault = _ImgPty_SetSettings.blackLevel;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_BIN_INDEX:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 0;
			paramReadOnly = FALSE;
			break;
		case ICamera::PARAM_HOT_PIXEL_INDEX:
			paramType = ICamera::TYPE_LONG;
			paramAvailable = _paramHotPixelAvailable;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 0;
			paramReadOnly = FALSE;
			break;
		default:
			ret = FALSE;
			paramAvailable = FALSE;
			break;
		}
	}
	catch (...)
	{
		ret = FALSE;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetParamInfo Parameter (%d) failed", paramID);
		LogMessage(_errMsg, ERROR_EVENT);
	}
	return ret;
}

long ORCA::GetLastErrorMsg(wchar_t* msg, long size)
{
	wcsncpy_s(msg, size, _errMsg, MSG_SIZE);

	//reset the error message
	_errMsg[0] = 0;
	return TRUE;
}

long ORCA::SetParamString(const long paramID, wchar_t* str)
{
	//no parameter set, return FALSE
	return FALSE;
}

long ORCA::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = TRUE;
	switch (paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME:
	{
		wcscpy_s(str, size, _pDetectorName);
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

long ORCA::SetParamBuffer(const long paramID, char* buf, long size)
{
	//no parameter set, return FALSE
	return FALSE;
}

long ORCA::GetParamBuffer(const long paramID, char* buf, long size)
{
	//no parameter found, return FALSE
	return FALSE;
}
