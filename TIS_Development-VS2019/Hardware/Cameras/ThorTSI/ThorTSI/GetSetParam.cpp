#include "stdafx.h"
#include "ThorTSI.h"


double TsiConvertToDouble (TSI_DATA_TYPE TsiParamDataType, void *pData)
{
	double ReturnValue = 0.0;
	switch (TsiParamDataType) 
	{

	case TSI_TYPE_UNS8  : ReturnValue = (double) *((uint8_t *)  pData); break;
	case TSI_TYPE_UNS16 : ReturnValue = (double) *((uint16_t *) pData); break;
	case TSI_TYPE_UNS32 : ReturnValue = (double) *((uint32_t *) pData); break;
	case TSI_TYPE_UNS64 : ReturnValue = (double) *((uint64_t *) pData); break;
	case TSI_TYPE_INT8  : ReturnValue = (double) *((int8_t *)   pData); break;
	case TSI_TYPE_INT16 : ReturnValue = (double) *((int16_t *)  pData); break;
	case TSI_TYPE_INT32 : ReturnValue = (double) *((int32_t *)  pData); break;
	case TSI_TYPE_INT64 : ReturnValue = (double) *((int64_t *)  pData); break;
	case TSI_TYPE_FP    : ReturnValue = (double) *((float *)    pData); break;
	default :
		break;
	}

	return ReturnValue;
}

long ThorCam::SetParam(const long paramID, const double param)
{
	long ret = FALSE;

	switch (paramID) 
	{
	case ICamera::PARAM_BINNING_X:
		{
			const TSI_ParamInfo* paramInfo = _cameraMapParams[PARAM_BINNING_X];
			if (nullptr != paramInfo)
			{
				if (paramInfo->paramMax >= param && paramInfo->paramMin <= param)
				{
					_ImgPty_SetSettings.roiBin.XBin = static_cast<uint32_t>(param);
					ret = TRUE;
				}
			}
		}
		break;

	case ICamera::PARAM_BINNING_Y:
		{
			const TSI_ParamInfo* paramInfo = _cameraMapParams[PARAM_BINNING_Y];
			if (nullptr != paramInfo)
			{
				if (paramInfo->paramMax >= param && paramInfo->paramMin <= param)
				{
					_ImgPty_SetSettings.roiBin.YBin = static_cast<uint32_t>(param);
					ret = TRUE;
				}
			}
		}
		break;

	case ICamera::PARAM_CAPTURE_REGION_LEFT:
		{
			if (_saved_Right[_selectedCam] > param)
			{
				const TSI_ParamInfo* paramInfoLeft = _cameraMapParams[PARAM_CAPTURE_REGION_LEFT];			
				if (nullptr != paramInfoLeft)
				{
					_saved_Left[_selectedCam] = param;
					double width = _saved_Right[_selectedCam] - _saved_Left[_selectedCam];
					if (paramInfoLeft->paramMax >= param && paramInfoLeft->paramMin <= param)
					{
						_ImgPty_SetSettings.roiBin.XOrigin = static_cast<uint32_t>(param);
						ret = TRUE;
					}


					const TSI_ParamInfo* paramInfoRight = _cameraMapParams[PARAM_CAPTURE_REGION_RIGHT];
					if (nullptr != paramInfoRight)
					{
						if(paramInfoRight->paramMax >= width && paramInfoRight->paramMin <= width)
						{
							_ImgPty_SetSettings.roiBin.XPixels = static_cast<uint32_t>(width);
							ret = TRUE;
						}
					}
				}
			}
		}
		break;

	case ICamera::PARAM_CAPTURE_REGION_RIGHT: 
		{
			if (_saved_Left[_selectedCam] < param)
			{
				const TSI_ParamInfo* paramInfo = _cameraMapParams[PARAM_CAPTURE_REGION_RIGHT];
				if (nullptr != paramInfo)
				{
					_saved_Right[_selectedCam] = param;
					double width = _saved_Right[_selectedCam] - _saved_Left[_selectedCam];
					if(width <= paramInfo->paramMax && width >= paramInfo->paramMin)
					{
						_ImgPty_SetSettings.roiBin.XPixels = static_cast<uint32_t>(width);
						ret = TRUE;
					}
				}
			}
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_TOP:
		{
			const TSI_ParamInfo* paramInfoTop = _cameraMapParams[PARAM_CAPTURE_REGION_TOP];
			if (_saved_Bottom[_selectedCam] > param)
			{
				if (nullptr != paramInfoTop)
				{
					_saved_Top[_selectedCam] = param;
					double height = _saved_Bottom[_selectedCam] - _saved_Top[_selectedCam];
					if (paramInfoTop->paramMax >= param && paramInfoTop->paramMin <= param)
					{
						_ImgPty_SetSettings.roiBin.YOrigin = static_cast<uint32_t>(param);
						ret = TRUE;
					}			

					const TSI_ParamInfo* paramInfoBottom = _cameraMapParams[PARAM_CAPTURE_REGION_BOTTOM];
					if (nullptr != paramInfoBottom)
					{
						if(paramInfoBottom->paramMax >= height && paramInfoBottom->paramMin <= height)
						{
							_ImgPty_SetSettings.roiBin.YPixels = static_cast<uint32_t>(height);
							ret = TRUE;
						}
					}
				}
			}
		}
		break;
	case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
		{
			if (_saved_Top[_selectedCam] < param)
			{
				const TSI_ParamInfo* paramInfo = _cameraMapParams[PARAM_CAPTURE_REGION_BOTTOM];
				if (nullptr != paramInfo)
				{
					_saved_Bottom[_selectedCam] = param;
					double height = _saved_Bottom[_selectedCam] - _saved_Top[_selectedCam];
					if(height <= paramInfo->paramMax && height >= paramInfo->paramMin)
					{
						_ImgPty_SetSettings.roiBin.YPixels = static_cast<uint32_t>(height);
						ret = TRUE;
					}
				}
			}
		}
		break;
	case ICamera::PARAM_TRIGGER_MODE:
		if (FIRST_TRIGGER_MODE <= param && LAST_TRIGGER_MODE)
		{
			_ImgPty_SetSettings.triggerMode = static_cast<uint32_t>(param);
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			if (UINT_MAX >= param && 0 <= param)
			{
				_ImgPty_SetSettings.numFrame = static_cast<uint32_t>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_EXPOSURE_TIME_MS:
		{
			if (MAX_EXPOSURETIME_MS >= param && MIN_EXPOSURETIME_MS <= param)
			{
				_ImgPty_SetSettings.exposureTime_us = static_cast<uint32_t>(param * 1000);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_OP_MODE:
		{
			if (TSI_OP_MODE_NORMAL <= param && TSI_MAX_OP_MODES >= param)
			{
				_ImgPty_SetSettings.opMode = (TSI_OP_MODE)static_cast<unsigned long>(param);
				ret = TRUE;
			}					
		}
		break;
	case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER:
		{
			if (0 <= param && UINT_MAX >= param)
			{
				_ImgPty_SetSettings.numImagesToBuffer = static_cast<uint32_t>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_TAP_BALANCE_MODE :
		{
			if (FALSE == param || TRUE == param)
			{
				_ImgPty_SetSettings.tapBalanceEnable = static_cast<uint32_t>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_TAP_INDEX :
		{
			const TSI_ParamInfo* paramInfo = _cameraMapParams[PARAM_TAP_INDEX];
			if (nullptr != paramInfo)
			{
				if (paramInfo->paramMin <= param && paramInfo->paramMax >= param)
				{
					_ImgPty_SetSettings.tapsIndex = static_cast<uint32_t>(param);
					ret = TRUE;
				}
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
					ret = TRUE;
				}
				else if (-90 >= static_cast<long>(param))
				{
					_ImgPty_SetSettings.imageAngle = 270;
					ret = TRUE;
				}
				else if (360 <= static_cast<long>(param))
				{
					_ImgPty_SetSettings.imageAngle = 0;
					ret = TRUE;
				}
			}
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP :
		{
			if (TRUE == param || FALSE == param)
			{
				_ImgPty_SetSettings.horizontalFlip = static_cast<uint32_t>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP :
		{
			if (TRUE == param || FALSE == param)
			{
				_ImgPty_SetSettings.verticalFlip = static_cast<uint32_t>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_GAIN:
		{
			const TSI_ParamInfo* paramInfo = _cameraMapParams[PARAM_GAIN];
			if (nullptr != paramInfo)
			{	
				if (paramInfo->paramMax >= param && paramInfo->paramMin <= param)
				{
					_ImgPty_SetSettings.gain = static_cast<uint32_t>(param);
					ret = TRUE;
				}
			}
		}
		break;
	case ICamera::PARAM_OPTICAL_BLACK_LEVEL:
		{		
			const TSI_ParamInfo* paramInfo = _cameraMapParams[PARAM_OPTICAL_BLACK_LEVEL];
			if (nullptr != paramInfo)
			{
				if (paramInfo->paramMax >= param && paramInfo->paramMin <= param)
				{
					_ImgPty_SetSettings.blackLevel = static_cast<uint32_t>(param);
					ret = TRUE;
				}
			}
		}
		break;
	case ICamera::PARAM_CAMERA_AVERAGENUM:
		{		
			const TSI_ParamInfo* paramInfo = _cameraMapParams[PARAM_CAMERA_AVERAGENUM];
			if (nullptr != paramInfo)
			{
				if (paramInfo->paramMax >= param && paramInfo->paramMin <= param)
				{
					_ImgPty_SetSettings.averageNum = static_cast<uint32_t>(param);
					ret = TRUE;
				}
			}
		}
		break;
	case ICamera::PARAM_CAMERA_AVERAGEMODE:
		{
			if (ICamera::AVG_MODE_NONE == param || ICamera::AVG_MODE_CUMULATIVE == param)
			{
				_ImgPty_SetSettings.averageMode = static_cast<uint32_t>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_READOUT_SPEED_INDEX:
		{
			const TSI_ParamInfo* paramInfo = _cameraMapParams[PARAM_READOUT_SPEED_INDEX];
			if (nullptr != paramInfo)
			{
				if (paramInfo->paramMax >= param && paramInfo->paramMin <= param)
				{
					_ImgPty_SetSettings.readOutSpeedIndex = static_cast<uint32_t>(param);
					ret = TRUE;
				}
			}
		}
		break;
	case ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT:
		{
			if (MAX_DMABUFNUM >= param && MIN_DMABUFNUM <= param)
			{
				_ImgPty_SetSettings.dmaBufferCount = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAMERA_CHANNEL:
		{
			if (MAX_CHANNEL >= param && MIN_CHANNEL <= param)
			{
				_ImgPty_SetSettings.channel = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAMERA_LED_ENABLE:
		{
			unsigned int ledEnable = static_cast<unsigned int>(param);
			if (_camera[_selectedCam]->SetParameter(TSI_PARAM_USB_ENABLE_LED, &ledEnable))
			{
				ret = TRUE;				
			}
			else 
			{				
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_USB_ENABLE_LED:(%d) in SetParam for ThorTSI", ledEnable);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
		{
			_forceSettingsUpdate = static_cast<long>(param);
			ret = TRUE;
		}
		break;
	default :
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) not implemented",paramID);
			LogMessage(_errMsg,ERROR_EVENT);
		}
		break;
	}	
	return ret;
}

long ThorCam::GetParam(const long paramID, double &param)
{
	if (0 > _selectedCam) return FALSE;
	long ret = FALSE;

	//only continue if we have a selected camera
	if (nullptr == _camera[_selectedCam]) {
		return FALSE;
	}

	// If the camera isn't open, we can't continue either.
	if (!IsOpen(_selectedCam)) {
		return FALSE;
	}

	switch (paramID) 
	{

	case ICamera::PARAM_CAMERA_TYPE:
		param = ICamera::CCD;
		ret   = TRUE;
		break;

	case ICamera::PARAM_EXPOSURE_TIME_MS:
		param = (double) _ImgPty_SetSettings.exposureTime_us / 1000.0;
		ret = TRUE;
		break;

		// ROI and Binning are special cases 
	case ICamera::PARAM_BINNING_X:
		param = _ImgPty_SetSettings.roiBin.XBin;
		ret = TRUE;
		break;

	case ICamera::PARAM_BINNING_Y:
		param = _ImgPty_SetSettings.roiBin.YBin;
		ret = TRUE;
		break;

	case ICamera::PARAM_CAPTURE_REGION_LEFT:
		param = (double) _ImgPty_SetSettings.roiBin.XOrigin;
		ret = TRUE;
		break;

	case ICamera::PARAM_CAPTURE_REGION_RIGHT:
		param = _saved_Right[_selectedCam];
		ret = TRUE;
		break;

	case ICamera::PARAM_CAPTURE_REGION_TOP:
		param = (double) _ImgPty_SetSettings.roiBin.YOrigin;
		ret = TRUE;
		break;

	case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
		param = _saved_Bottom[_selectedCam];
		ret = TRUE;
		break;

	case ICamera::PARAM_TRIGGER_MODE:
		{
			param = _ImgPty_SetSettings.triggerMode;
			ret   = TRUE;
		}
		break;

	case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER :
		{
			param = _ImgPty_SetSettings.numImagesToBuffer;
			ret   = TRUE;
		}
		break;

	case ICamera::PARAM_TAP_BALANCE_MODE :
		{
			param = _ImgPty_SetSettings.tapBalanceEnable;
			ret   = TRUE;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_ANGLE :
		{
			param = _ImgPty_SetSettings.imageAngle;
			ret   = TRUE;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP :
		{
			param = _ImgPty_SetSettings.horizontalFlip;
			ret   = TRUE;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP :
		{
			param = _ImgPty_SetSettings.verticalFlip;
			ret   = TRUE;
		}
		break;
	case ICamera::PARAM_TAP_INDEX:
		{
			param = _ImgPty_SetSettings.tapsIndex;
			ret   = TRUE;
		}
		break;
	case ICamera::PARAM_GAIN:
		{
			param = _ImgPty_SetSettings.gain;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_OPTICAL_BLACK_LEVEL:
		{
			param = _ImgPty_SetSettings.blackLevel;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_DROPPED_FRAMES:
		{
			param = static_cast<double>(_availableFramesCnt);
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_CAMERA_AVERAGENUM:
		{
			param = _ImgPty_SetSettings.averageNum;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_CAMERA_AVERAGEMODE:
		{
			param = _ImgPty_SetSettings.averageMode;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_WIDTH:
		{
			//if the angle is not 0 or 180 we need to change the binning and width/height accordingly
			if (_ImgPty_SetSettings.imageAngle != 0 && _ImgPty_SetSettings.imageAngle != 180)
			{
				if(0 !=  _ImgPty_SetSettings.roiBin.YBin)
				{
					param = _ImgPty_SetSettings.roiBin.YPixels / _ImgPty_SetSettings.roiBin.YBin;
				}
				else
				{
					param = _ImgPty_SetSettings.roiBin.YPixels;
				}
			}
			else
			{
				if(0 !=  _ImgPty_SetSettings.roiBin.XBin)
				{
					param = _ImgPty_SetSettings.roiBin.XPixels / _ImgPty_SetSettings.roiBin.XBin;
				}
				else
				{
					param = _ImgPty_SetSettings.roiBin.XPixels;
				}
			}
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_CAMERA_IMAGE_HEIGHT:
		{
			//if the angle is not 0 or 180 we need to change the binning and width/height accordingly
			if (_ImgPty_SetSettings.imageAngle != 0 && _ImgPty_SetSettings.imageAngle != 180)
			{
				if(0 !=  _ImgPty_SetSettings.roiBin.XBin)
				{
					param = _ImgPty_SetSettings.roiBin.XPixels / _ImgPty_SetSettings.roiBin.XBin;		
				}
				else
				{
					param = _ImgPty_SetSettings.roiBin.XPixels;
				}
			}
			else
			{
				if(0 !=  _ImgPty_SetSettings.roiBin.YBin)
				{
					param = _ImgPty_SetSettings.roiBin.YPixels / _ImgPty_SetSettings.roiBin.YBin;
				}
				else
				{
					param = _ImgPty_SetSettings.roiBin.YPixels;
				}
			}
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_BITS_PER_PIXEL:
		{
			param = _ImgPty_SetSettings.bitsPerPixel;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_PIXEL_SIZE:
		{
			float paramf;
			if (GetTsiParameter_float(_selectedCam, TSI_PARAM_PIXEL_SIZE, paramf))
			{
				param = paramf;
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_READOUT_SPEED_INDEX:
		{
			param = _ImgPty_SetSettings.readOutSpeedIndex;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_FRAME_RATE:
		{
			float paramf;
			if (GetTsiParameter_float(_selectedCam, TSI_PARAM_FRAME_RATE, paramf))
			{
				param = paramf;
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT:
		{
			param = _ImgPty_SetSettings.dmaBufferCount;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_CAMERA_CHANNEL:
		{
			param = _ImgPty_SetSettings.channel;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_CAMERA_LED_AVAILABLE:
		{
			param = (TSI_USB_INTERFACE == _cameraInterfaceType);
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_CAMERA_LED_ENABLE:
		{
			unsigned int paramul;
			if (GetTsiParameter_uint(_selectedCam, TSI_PARAM_USB_ENABLE_LED, paramul))
			{
				param = paramul;
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
		{
			param = _forceSettingsUpdate;
			ret = TRUE;
		}
		break;
	default:
		{
			ret = FALSE;
		}
		break;
	}

	return ret;

}

long ThorCam::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = FALSE;


	paramType		= (long) TYPE_LONG;
	paramAvailable	= (long) FALSE;
	paramReadOnly	= (long) FALSE;
	paramMin		= 0.0;
	paramMax		= 0.0;
	paramDefault	= 0.0;

	ret = GetTsiMappedParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);

	//Only do the default when GetMappedParamInfo returns FALSE, meaning there is no match for the parameter in the paraminfo map
	if (FALSE == ret) 
	{

		ICamera::Params TypedParameter = (ICamera::Params) paramID;

		// No match, handle manually or return unsupported.

		switch (paramID) {

		case ICamera::PARAM_CAMERA_TYPE:

			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= ICamera::CCD;
			paramMax		= ICamera::CCD;
			paramDefault	= ICamera::CCD;
			paramReadOnly	= TRUE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_CONSOLE_WRITE :

			paramType		= ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_CONSOLE_READ :

			paramType		= ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_DETECTOR_NAME :

			paramType		= ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_DETECTOR_SERIAL_NUMBER :

			paramType		= ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_TRIGGER_MODE :

			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= ICamera::FIRST_TRIGGER_MODE;
			paramMax		= ICamera::LAST_TRIGGER_MODE - 1;
			paramDefault	= ICamera::SW_FREE_RUN_MODE;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_LIGHT_MODE :

			paramAvailable	= FALSE;
			ret				= TRUE;

			break;			

		case ICamera::PARAM_NUMBER_OF_IMAGES_TO_BUFFER :

			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MAX_TSI_SDK_IMAGES;
			paramMax		= UINT_MAX;
			paramDefault	= 0;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_TAP_BALANCE_MODE :
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= TRUE;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_CAPTURE_REGION_LEFT:

			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= MAX_IMAGE_WIDTH;
			paramDefault	= 0;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_CAPTURE_REGION_RIGHT:

			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= MAX_IMAGE_WIDTH;
			paramDefault	= DEFAULT_IMAGE_WIDTH;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_CAPTURE_REGION_TOP:

			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= MAX_IMAGE_HEIGHT;
			paramDefault	= 0;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_CAPTURE_REGION_BOTTOM:

			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= MAX_IMAGE_HEIGHT;
			paramDefault	= DEFAULT_IMAGE_HEIGHT;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_BINNING_X:

			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_XBIN;
			paramMax		= MAX_XBIN;
			paramDefault	= DAFAULT_XBIN;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_BINNING_Y:

			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_YBIN;
			paramMax		= MAX_YBIN;
			paramDefault	= DAFAULT_YBIN;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_MULTI_FRAME_COUNT:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= MAXLONG32;
			paramDefault	= 1;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_CAMERA_IMAGE_ANGLE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_ANGLE;
			paramMax		= MAX_ANGLE;
			paramDefault	= DAFAULT_ANGLE;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= FALSE;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;

		case ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= FALSE;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_EXPOSURE_TIME_MS:
			paramType		= ICamera::TYPE_DOUBLE;
			paramAvailable	= TRUE;
			paramMin		= MIN_EXPOSURETIME_MS;
			paramMax		= MAX_EXPOSURETIME_MS;
			paramDefault	= DAFAULT_EXPOSURETIME_MS;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_BITS_PER_PIXEL:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_BITS_PERPIXEL;
			paramMax		= MAX_BITS_PERPIXEL;
			paramDefault	= DEFAULT_BITS_PERPIXEL;
			paramReadOnly	= TRUE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_CAMERA_IMAGE_WIDTH:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_IMAGE_WIDTH;
			paramMax		= MAX_IMAGE_WIDTH;
			paramDefault	= DEFAULT_IMAGE_WIDTH;
			paramReadOnly	= TRUE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_CAMERA_IMAGE_HEIGHT:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_IMAGE_HEIGHT;
			paramMax		= MAX_IMAGE_HEIGHT;
			paramDefault	= DEFAULT_IMAGE_HEIGHT;
			paramReadOnly	= TRUE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_DROPPED_FRAMES:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= INT_MAX;
			paramDefault	= 0;
			paramReadOnly	= TRUE;
			ret				= TRUE;
			break;
		case ICamera::PARAM_CAMERA_AVERAGEMODE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= ICamera::AVG_MODE_NONE;
			paramMax		= ICamera::AVG_MODE_CUMULATIVE;
			paramDefault	= ICamera::AVG_MODE_NONE;
			paramReadOnly	= TRUE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_FRAME_RATE:
			paramType		= ICamera::TYPE_DOUBLE;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= MAXULONG32;
			paramDefault	= 0;
			paramReadOnly	= TRUE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT:
			paramType		= ICamera::TYPE_DOUBLE;
			paramAvailable	= TRUE;
			paramMin		= MIN_DMABUFNUM;
			paramMax		= MAX_DMABUFNUM;
			paramDefault	= DEFAULT_DMABUFNUM;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_CAMERA_CHANNEL:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= MIN_CHANNEL;
			paramMax		= MAX_CHANNEL;
			paramDefault	= DEFAULT_CHANNEL;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_CAMERA_LED_AVAILABLE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= TRUE;
			paramReadOnly	= TRUE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_CAMERA_LED_ENABLE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= FALSE;
			paramMax		= TRUE;
			paramDefault	= TRUE;
			paramReadOnly	= FALSE;

			ret				= TRUE;

			break;
		case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
			paramType		= ICamera::TYPE_LONG;
			paramAvailable	= TRUE;
			paramMin		= 0;
			paramMax		= 1;
			paramDefault	= 0;
			paramReadOnly	= FALSE;
			ret				= TRUE;
			break;
		default:
			ret				= FALSE;
			paramAvailable	= FALSE;
			break;
		}
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
	long ret = FALSE;
	switch(paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME:
		{
			wcscpy_s(str,size, _pDetectorName);
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_DETECTOR_SERIAL_NUMBER:
		{
			wcscpy_s(str, size, _pSerialNumber);
			ret = TRUE;
		}
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

long ThorCam::BuildTsiParamInfoMap(const unsigned long cameraIndex)
{
	if (0 > cameraIndex) return FALSE;
	const unsigned long i = cameraIndex;
	_cameraMapParams.clear();

	//Query te camera for parameter ranges and availability
	TSI_ParamInfo* tsiParamInfo = new TSI_ParamInfo;;
	tsiParamInfo->tsiParamID = TSI_PARAM_READOUT_SPEED;
	GetTsiParamInfo(tsiParamInfo, i);
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_READOUT_SPEED_VALUE, tsiParamInfo));

	tsiParamInfo = new TSI_ParamInfo;;
	tsiParamInfo->tsiParamID = TSI_PARAM_READOUT_SPEED_INDEX;
	GetTsiParamInfo(tsiParamInfo, i);
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_READOUT_SPEED_INDEX, tsiParamInfo));

	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_GAIN;
	GetTsiParamInfo(tsiParamInfo, i);
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_GAIN, tsiParamInfo));

	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_TAPS_INDEX;
	GetTsiParamInfo(tsiParamInfo, i);
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_TAP_INDEX, tsiParamInfo));

	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_TAPS_VALUE;
	GetTsiParamInfo(tsiParamInfo, i);
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_TAP_VALUE, tsiParamInfo));

	unsigned int param = 0;

	//Some parameters don't have a range option. Get the maximum sizes. HSize and VSize are the max pixels for X and Y
	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_HSIZE;
	GetTsiParameter_uint(i, tsiParamInfo->tsiParamID, param);
	tsiParamInfo->paramAvailable = TRUE;
	tsiParamInfo->paramReadOnly = FALSE;
	tsiParamInfo->paramType = ICamera::ParamType::TYPE_LONG;
	tsiParamInfo->paramMax = param;
	tsiParamInfo->paramMin = 1;
	tsiParamInfo->paramDefault = param;
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_CAPTURE_REGION_RIGHT, tsiParamInfo));
	//check if stored values are inside bounds, otherwise set to full frame
	if (param < _saved_Right[cameraIndex] || param < _ImgPty_SetSettings.roiBin.XPixels || 0 > _saved_Left[cameraIndex] || 0 > _ImgPty_SetSettings.roiBin.XOrigin)
	{
		_saved_Left[cameraIndex] = _ImgPty_SetSettings.roiBin.XOrigin = 0;
		_saved_Right[cameraIndex] = _ImgPty_SetSettings.roiBin.XPixels = param;
	}
	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_VSIZE;
	GetTsiParameter_uint(i, tsiParamInfo->tsiParamID, param);
	tsiParamInfo->paramAvailable = TRUE;
	tsiParamInfo->paramReadOnly = FALSE;
	tsiParamInfo->paramType = ICamera::ParamType::TYPE_LONG;
	tsiParamInfo->paramMax = param;
	tsiParamInfo->paramMin = 1;
	tsiParamInfo->paramDefault = param;
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_CAPTURE_REGION_BOTTOM, tsiParamInfo));
	//check if stored values are inside bounds, otherwise set to full frame
	if (param < _saved_Bottom[cameraIndex] || param < _ImgPty_SetSettings.roiBin.YPixels || 0 > _saved_Top[cameraIndex] || 0 > _ImgPty_SetSettings.roiBin.YOrigin)
	{
		_saved_Top[cameraIndex] = _ImgPty_SetSettings.roiBin.YOrigin = 0;
		_saved_Bottom[cameraIndex] = _ImgPty_SetSettings.roiBin.YPixels = param;
	}
	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_HSIZE;
	GetTsiParameter_uint(i, tsiParamInfo->tsiParamID, param);
	tsiParamInfo->paramAvailable = TRUE;
	tsiParamInfo->paramReadOnly = FALSE;
	tsiParamInfo->paramType = ICamera::ParamType::TYPE_LONG;
	tsiParamInfo->paramMax = param - 1;
	tsiParamInfo->paramMin = 0;
	tsiParamInfo->paramDefault = 0;
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_CAPTURE_REGION_LEFT, tsiParamInfo));

	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_VSIZE;
	GetTsiParameter_uint(i, tsiParamInfo->tsiParamID, param);
	tsiParamInfo->paramAvailable = TRUE;
	tsiParamInfo->paramReadOnly = FALSE;
	tsiParamInfo->paramType = ICamera::ParamType::TYPE_LONG;
	tsiParamInfo->paramMax = param - 1;
	tsiParamInfo->paramMin = 0;
	tsiParamInfo->paramDefault = 0;
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_CAPTURE_REGION_TOP, tsiParamInfo));

	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_XBIN;
	tsiParamInfo->paramAvailable = TRUE;
	tsiParamInfo->paramReadOnly = FALSE;
	tsiParamInfo->paramType = ICamera::ParamType::TYPE_LONG;
	tsiParamInfo->paramMax = MAX_XBIN;
	tsiParamInfo->paramMin = MIN_XBIN;
	tsiParamInfo->paramDefault = DAFAULT_XBIN;
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_BINNING_X, tsiParamInfo));

	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_YBIN;
	tsiParamInfo->paramAvailable = TRUE;
	tsiParamInfo->paramReadOnly = FALSE;
	tsiParamInfo->paramType = ICamera::ParamType::TYPE_LONG;
	tsiParamInfo->paramMax = MAX_YBIN;
	tsiParamInfo->paramMin = MIN_YBIN;
	tsiParamInfo->paramDefault = DAFAULT_YBIN;
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_BINNING_Y, tsiParamInfo));

	//move frome here
	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_BITS_PER_PIXEL;
	GetTsiParameter_uint(i, tsiParamInfo->tsiParamID, param);
	tsiParamInfo->paramAvailable = TRUE;
	tsiParamInfo->paramReadOnly = TRUE;
	tsiParamInfo->paramType = ICamera::ParamType::TYPE_LONG;
	tsiParamInfo->paramMax = param;
	tsiParamInfo->paramMin = param;
	tsiParamInfo->paramDefault = param;
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_BITS_PER_PIXEL, tsiParamInfo));

	float paramf = 0;
	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_PIXEL_SIZE;
	GetTsiParameter_float(i, tsiParamInfo->tsiParamID, paramf);
	tsiParamInfo->paramAvailable = TRUE;
	tsiParamInfo->paramReadOnly = TRUE;
	tsiParamInfo->paramType = ICamera::ParamType::TYPE_DOUBLE;
	tsiParamInfo->paramMax = paramf;
	tsiParamInfo->paramMin = paramf;
	tsiParamInfo->paramDefault = paramf;
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_PIXEL_SIZE, tsiParamInfo));

	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->tsiParamID = TSI_PARAM_OPTICAL_BLACK_LEVEL;
	tsiParamInfo->paramAvailable = TRUE;
	tsiParamInfo->paramReadOnly = FALSE;
	tsiParamInfo->paramType = ICamera::ParamType::TYPE_LONG;
	tsiParamInfo->paramMax = MAX_BLACK_LEVEL;
	tsiParamInfo->paramMin = MIN_BLACK_LEVEL;
	tsiParamInfo->paramDefault = DEFAULT_BLACK_LEVEL;
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_OPTICAL_BLACK_LEVEL, tsiParamInfo));

	tsiParamInfo = new TSI_ParamInfo;
	tsiParamInfo->paramAvailable = TRUE;
	tsiParamInfo->paramReadOnly = FALSE;
	tsiParamInfo->paramType = ICamera::ParamType::TYPE_LONG;
	tsiParamInfo->paramMax = MAX_AVGNUM;
	tsiParamInfo->paramMin = MIN_AVGNUM;
	tsiParamInfo->paramDefault = DEFAULT_AVGNUM;
	_cameraMapParams.insert(std::pair<long, TSI_ParamInfo*>(PARAM_CAMERA_AVERAGENUM, tsiParamInfo));

	return TRUE;
}



long ThorCam::GetTsiParamInfo(TSI_ParamInfo* paramInfo, const unsigned long cameraIndex)
{
	if (0 > cameraIndex) return FALSE;
	const unsigned long i = cameraIndex;

	void* pMinData = nullptr;
	void* pMaxData = nullptr;
	void* pDefaultData = nullptr;

	TSI_PARAM_FLAGS	TsiParamFlags			= (TSI_PARAM_FLAGS) 0;
	TSI_DATA_TYPE   TsiParamDataType		= TSI_TYPE_NONE;
	uint32_t		TsiParamDataArrayCount	= 0;	

	TSI_PARAM_ATTR_ID paramAttrID;
	paramAttrID.ParamID = paramInfo->tsiParamID;
	size_t attrDataLength = 0;
	size_t ParamDataLength = 0;
	void* pAttrData = nullptr;

	long ret = TRUE;
	for (long j = 0; j < TSI_MAX_ATTR; j++)
	{

		paramAttrID.AttrID  = (TSI_ATTR_ID)j;

		switch (paramAttrID.AttrID) 
		{
		case TSI_ATTR_DATA_TYPE:
			pAttrData		= (void *) &TsiParamDataType;
			attrDataLength	= sizeof (TsiParamDataType);
			break;

		case TSI_ATTR_ARRAY_COUNT:
			pAttrData		= (void *) &TsiParamDataArrayCount;
			attrDataLength	= sizeof (TsiParamDataArrayCount);
			break;

		case TSI_ATTR_FLAGS:
			pAttrData		= (void*)&TsiParamFlags;
			attrDataLength	= sizeof(TsiParamFlags);
			break;

		case TSI_ATTR_MIN_VALUE:
			ParamDataLength = _camera[i]->GetDataTypeSize (TsiParamDataType) * TsiParamDataArrayCount;
			pMinData		= (void *) malloc (ParamDataLength);
			pAttrData		= pMinData;
			attrDataLength	= ParamDataLength;
			break;

		case TSI_ATTR_MAX_VALUE:
			pMaxData		= (void *) malloc (ParamDataLength);
			pAttrData		= pMaxData;
			attrDataLength	= ParamDataLength;
			break;

		case TSI_ATTR_DEFAULT_VALUE:
			pDefaultData	= (void *) malloc (ParamDataLength);
			pAttrData		= pDefaultData;
			attrDataLength	= ParamDataLength;
			break;
		default:
			continue;
		}

		if (false == _camera[i]->SetParameter(TSI_PARAM_CMD_ID_ATTR_ID, &paramAttrID))
		{
			ret = FALSE;
		}

		if (false == _camera[i]->GetParameter(TSI_PARAM_ATTR, attrDataLength, pAttrData))
		{
			ret = FALSE;
		}
	}

	if (TRUE == ret)
	{
		// Tsi has more data types than the ICamera model does, we'll have
		// to map to what it supports, which is double or long, and we'll have to
		// allocate storage within this object.
		switch (TsiParamDataType) {
		case TSI_TYPE_UNS8  : 
		case TSI_TYPE_UNS16 : 
		case TSI_TYPE_UNS32 : 
		case TSI_TYPE_UNS64 : 
		case TSI_TYPE_INT8  : 
		case TSI_TYPE_INT16 : 
		case TSI_TYPE_INT32 : 
		case TSI_TYPE_INT64 : 
			paramInfo->paramType	= ICamera::TYPE_LONG;
			break;
		case TSI_TYPE_FP    : 
			paramInfo->paramType	= ICamera::TYPE_DOUBLE;
			break;
		default :
			break;
		}

		paramInfo->paramAvailable = (long) ((TsiParamFlags & TSI_FLAG_UNSUPPORTED) == 0);
		paramInfo->paramReadOnly	= (long) ((TsiParamFlags & TSI_FLAG_READ_ONLY)   != 0);

		if (paramInfo->paramAvailable)
		{
			if (pMinData)
			{
				paramInfo->paramMin = TsiConvertToDouble (TsiParamDataType, pMinData);
			}
			if (pMaxData) 
			{
				paramInfo->paramMax = TsiConvertToDouble (TsiParamDataType, pMaxData);
			}
			if (pDefaultData) 
			{
				paramInfo->paramDefault = TsiConvertToDouble (TsiParamDataType, pDefaultData);
			}
		}
	}

	if (nullptr != pMinData) {
		free(pMinData);
	}

	if (nullptr != pMaxData) {
		free(pMaxData);
	}

	if (nullptr != pDefaultData) {
		free(pDefaultData);
	}

	return ret;

}

long ThorCam::GetTsiParameter_uint(const unsigned long cameraIndex, TSI_PARAM_ID paramID, unsigned int &param)
{
	if (0 > cameraIndex) return FALSE;
	const unsigned long i = cameraIndex;
	TSI_DATA_TYPE tsi_param_data_type = TSI_TYPE_NONE;
	TSI_PARAM_ATTR_ID AttributeDescriptor;
	AttributeDescriptor.AttrID = TSI_ATTR_DATA_TYPE;
	AttributeDescriptor.ParamID = paramID;

	if (!_camera[i]->SetParameter(TSI_PARAM_CMD_ID_ATTR_ID, &AttributeDescriptor)) 
	{
		//Fail
		return FALSE;
	} 
	else if (!_camera[i]->GetParameter(TSI_PARAM_ATTR, sizeof (tsi_param_data_type), &tsi_param_data_type)) 
	{
		//Fail
		return FALSE;
	}
	if(tsi_param_data_type != TSI_TYPE_UNS32)
	{
		//Fail
		return FALSE;
	}

	unsigned int* out = &param;
	long ret = _camera[i]->GetParameter(paramID, sizeof(unsigned int), out);

	return ret;
}

long ThorCam::GetTsiParameter_float(const unsigned long cameraIndex,TSI_PARAM_ID paramID, float &param)
{
	if (0 > cameraIndex) return FALSE;
	const unsigned long i = cameraIndex;
	TSI_DATA_TYPE tsi_param_data_type = TSI_TYPE_NONE;
	TSI_PARAM_ATTR_ID AttributeDescriptor;
	AttributeDescriptor.AttrID = TSI_ATTR_DATA_TYPE;
	AttributeDescriptor.ParamID = paramID;

	if (!_camera[i]->SetParameter(TSI_PARAM_CMD_ID_ATTR_ID, &AttributeDescriptor)) 
	{
		//Fail
		return FALSE;
	} 
	else if (!_camera[i]->GetParameter(TSI_PARAM_ATTR, sizeof (tsi_param_data_type), &tsi_param_data_type)) 
	{
		//Fail
		return FALSE;
	}
	if(tsi_param_data_type != TSI_TYPE_FP)
	{
		//Fail
		return FALSE;
	}

	float* out = &param;

	long ret = _camera[i]->GetParameter(paramID, sizeof(float), out);

	return ret;
}

long ThorCam::GetTsiMappedParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = FALSE;
	if (nullptr !=  _cameraMapParams[paramID])
	{
		if (_cameraMapParams[paramID]->paramAvailable)
		{
			paramType = _cameraMapParams[paramID]->paramType;
			paramAvailable = _cameraMapParams[paramID]->paramAvailable;
			paramReadOnly = _cameraMapParams[paramID]->paramReadOnly;
			paramMin = _cameraMapParams[paramID]->paramMin;
			paramMax = _cameraMapParams[paramID]->paramMax;
			paramDefault = _cameraMapParams[paramID]->paramDefault;
			ret = TRUE;
		}
	}
	return ret;
}