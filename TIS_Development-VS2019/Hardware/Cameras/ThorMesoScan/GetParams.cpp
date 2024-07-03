#include "ThorMesoScan.h"
#include "Strsafe.h"
#include "Logger.h"

long ThorLSMCam::GetLastErrorMsg(wchar_t * msg, long size)
{
	return TRUE;
}

long ThorLSMCam::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;
	switch (paramID)
	{
	case ICamera::PARAM_CAMERA_TYPE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = ICamera::LSM;
			paramMax = ICamera::LSM;
			paramDefault = ICamera::LSM;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_TYPE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = ICamera::RESONANCE_GALVO_GALVO;
			paramMax = ICamera::RESONANCE_GALVO_GALVO;
			paramDefault = ICamera::RESONANCE_GALVO_GALVO;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_DETECTOR_NAME:
		{
			paramType		= ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= TRUE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;
			paramReadOnly = FALSE;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_SCANNER_INIT_MODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_FRAME_RATE:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = Constants::MHZ;
			paramDefault = 1.0;
			paramReadOnly = TRUE;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_MESO_STRIP_COUNT:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = LONG_MAX;
			paramDefault = 0;
			paramReadOnly = TRUE;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_MESO_EXP_PATH:
		{
			paramType = ICamera::TYPE_STRING;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_X:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_PIXEL;
			paramMax = MAX_PIXEL;
			paramDefault = DEFAULT_PIXEL;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_Y:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_PIXEL;
			paramMax = MAX_PIXEL;
			paramDefault = DEFAULT_PIXEL;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_STOP_ACQUISITION:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 0;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_MESO_SCAN_INFO:
		{
			paramType = ICamera::TYPE_BUFFER;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE1:
	case ICamera::PARAM_LSM_INPUTRANGE2:
	case ICamera::PARAM_LSM_INPUTRANGE3:
	case ICamera::PARAM_LSM_INPUTRANGE4:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_INPUTRANGE;
			paramMax = MAX_INPUTRANGE;
			paramDefault = DEFAULT_INPUTRANGE;
			paramReadOnly = FALSE;
		}
		break;
	default:
		{
			ret = FALSE;
			if (_pCameraConfig.get()->IsUIntValue((ICamera::Params)paramID))
			{
				paramType = ICamera::TYPE_LONG;
				uint32_t min, max, value;
				ret = _pCameraConfig.get()->GetParameterRange((ICamera::Params)paramID, min, max, value);
				paramMin = min; paramMax = max; paramDefault = value;
			}
			else
			{
				paramType = ICamera::TYPE_DOUBLE;
				ret = _pCameraConfig.get()->GetParameterRange((ICamera::Params)paramID, paramMin, paramMax, paramDefault);
			}

			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			break;
		}
	}
	return ret;
}

long ThorLSMCam::GetParam(const long paramID, double &param)
{
	long ret = TRUE, timeMS = 0;
	switch (paramID)
	{
	case ICamera::PARAM_CAMERA_TYPE:
		{
			param = ICamera::LSM;
		}
		break;
	case ICamera::PARAM_LSM_TYPE:
		{
			param = ICamera::RESONANCE_GALVO_GALVO;
		}
		break;
	case ICamera::PARAM_SCANNER_INIT_MODE:
		{
			param = _rsInitMode;
		}
		break;
	case ICamera::PARAM_MESO_STRIP_COUNT:
		{
			param = _hMesoScanWaveform->GetStripCount();
		}
		break;
	case ICamera::PARAM_FRAME_RATE:
		{
			//param = (NULL != _preScan && GetCaptureTime(_preScan, 1, timeMS)) ? ((double)Constants::MS_TO_SEC / timeMS) : 0.0;
			param = (0 < _hMesoScanWaveform->GetTotalScanTime()) ? 1 / _hMesoScanWaveform->GetTotalScanTime() : 0.0;
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_X:
		{
			param = _pixelX;
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_Y:
		{
			param = _pixelY;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:
		{
			long index = 0;
			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:index = 0;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:index = 1;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:index = 2;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:index = 3;break;
			}
			param = _pockelsPowerLevel[index];
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE1:
	case ICamera::PARAM_LSM_INPUTRANGE2:
	case ICamera::PARAM_LSM_INPUTRANGE3:
	case ICamera::PARAM_LSM_INPUTRANGE4:
		{
			param = _inputRangeChannel[paramID-ICamera::PARAM_LSM_INPUTRANGE1];
		}
		break;
	default:
		{
			ret = FALSE;
			if (_pCameraConfig.get()->IsUIntValue((ICamera::Params)paramID))
			{
				uint32_t value;
				ret = _pCameraConfig.get()->GetParameter((ICamera::Params)paramID, value);
				param = value;
			}
			else
			{
				ret = _pCameraConfig.get()->GetParameter((ICamera::Params)paramID, param);
			}
			break;
		}

	}
	return ret;
}

long ThorLSMCam::GetParamString(const long paramID, wchar_t * str, long size)
{
	long ret = FALSE;

	switch(paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME:
		{
			wcscpy_s(str,20, L"ResonanceGalvoGalvo"); //tempID[20] in SelectHardware
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_MESO_EXP_PATH:
		{
			wcscpy_s(str,size, _expLoader.get()->ExpPath.c_str());
			ret = TRUE;
		}
		break;
	default:
		break;
	}

	return ret;
}

long ThorLSMCam::GetParamBuffer(const long paramID, char * pBuffer, long size)
{
	return TRUE;
}

long ThorLSMCam::GetMapParam(const long paramID, double inputValue, double &param)
{
	if (paramID == PARAM_MESO_TWOWAY_ALIGNMENT_SHIFT)
	{
		return _pCameraConfig.get()->GetTwoWayAlignmentPoint(inputValue, param);
	}
	else if (paramID == PARAM_MESO_RESONANT_FIELD_TO_VOLTAGE)
	{
		return _pCameraConfig.get()->GetResonantVoltage(inputValue, param);
	}
	return FALSE;
}

long ThorLSMCam::GetDeviceConfigration(const long paramID, void** param)
{
	long ret = FALSE;
	switch (paramID)
	{
	case ICamera::PARAM_MESO_CAMERA_CONFIG:
		{
			*param = (void*)_pCameraConfig.get();
			ret = TRUE;
		}
		break;
	default:
		break;
	}
	return ret;
}

long ThorLSMCam::GetCaptureTime(Scan scans[], const uint8_t scanSize, long& timeMillisecond)
{
	ReadPosition();
	timeMillisecond = _hMesoScanWaveform->GetTotalScanTime(scans, scanSize, _pCameraConfig.get());
	return TRUE;
}
