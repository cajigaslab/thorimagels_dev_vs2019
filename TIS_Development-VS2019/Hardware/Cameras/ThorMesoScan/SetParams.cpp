#include "ThorMesoScan.h"
#include "Strsafe.h"
#include "Logger.h"
#include "PockelsCalibration.h"

long ThorLSMCam::SetParam(const long paramID, const double param)
{
	long ret = TRUE;
	float64  data = 0.0;
	float64 GYVPValueArr[3] = { _pCameraConfig->GetYCenterVoltage(), _pCameraConfig->GetZCenterVoltage(), _pCameraConfig->GetPockelPowerVoltage(param) };
	float64 GX1X2ValueArr[2] = { _pCameraConfig->GetX1CenterVoltage(), _pCameraConfig->GetX2CenterVoltage() };

	switch (paramID)
	{
	case PARAM_MESO_RESONANT_AMPLITUDE:
		_pCameraConfig->GetResonantVoltage(param, data);
		_hDAQController->InvokeTask(_pCameraConfig->RESONANT_AO_CHANNEL, AO, &data, 1, 1);
		break;
	case PARAM_LSM_POCKELS_FIND_MIN_MAX_0:
		_isPockelsCalibrationRunning = true;
		ret =StartPockelsCalibration(_hDAQController, _pCameraConfig.get(), _isPockelsCalibrationRunning);
		_isPockelsCalibrationRunning = false;
		break;
	case PARAM_POCKELS_STOP_CALIBRATION:
		_isPockelsCalibrationRunning = false;
		break;
	case ICamera::PARAM_LSM_PIXEL_X:
		{
			if ((param >= MIN_PIXEL) && (param <= MAX_PIXEL))
			{
				_pixelX = static_cast<long>(param);
				if(ICamera::SQUARE == _pCameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_AREAMODE))
				{
					_pixelY = _pixelX;
				}
			}
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_Y:
		{
			if(ICamera::SQUARE == _pCameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_AREAMODE))
			{
				_pixelY = _pixelX;
			}
			else if(ICamera::LINE == _pCameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_AREAMODE))
			{
				_pixelY = 1;
			}
			else
			{
				if ((param >= MIN_PIXEL) && (param <= MAX_PIXEL))
				{
					_pixelY = static_cast<long>(param);
				}
			}
		}
		break;
	case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
		//used to update scan from loading of active experiment, [TRUE: restart scanner, FALSE: set w/o stopping scanner]
		::EnterCriticalSection(&_accessSection);
		if(TRUE == _expLoader.get()->LoadExperimentXML())
		{
			if (TRUE == static_cast<long>(param))
				StopScan();

			for (int i = 0; i < static_cast<int>(_expLoader.get()->Scans.size()); i++)
			{
				SetParamBuffer(ICamera::PARAM_MESO_SCAN_INFO, (char*)_expLoader.get()->Scans[i], sizeof(_expLoader.get()->Scans[i]));
			}
			ret = (TRUE == static_cast<long>(param)) ? SetupParamAndWaveforms() : SetParameters();
		}
		else
		{
			SetParamBuffer(ICamera::PARAM_MESO_SCAN_INFO, NULL, 0);
		}
		::LeaveCriticalSection(&_accessSection);
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:
		{
			long index=0;

			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:index=2;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:index=3;break;
			}
			_pockelsPowerLevel[index] = param;

			//the scanner is in centering mode. allow the power to be changed instantaneously
			if ((unsigned int)ScanMode::CENTER == _pCameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_SCANMODE))
			{
				float64 data = _pCameraConfig->GetPockelPowerVoltage(_pockelsPowerLevel[index]);
				_hDAQController->InvokeTask(_pCameraConfig->POCKEL_AO_CHANNEL, AO, &data, 1, 1);
			}
		}
		break;
	case ICamera::PARAM_SCANNER_INIT_MODE:
		{
			_rsInitMode = static_cast<long>(param);
			double data = (_rsInitMode) ? (double)DEFAULT_RES_VOLT : 0;
			_hDAQController->InvokeTask(_pCameraConfig.get()->RESONANT_AO_CHANNEL, AO, &data, 1, 1);
		}
		break;
	case ICamera::PARAM_LSM_STOP_ACQUISITION:
		{
			PostflightAcquisition(NULL);
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE1:
	case ICamera::PARAM_LSM_INPUTRANGE2:
	case ICamera::PARAM_LSM_INPUTRANGE3:
	case ICamera::PARAM_LSM_INPUTRANGE4:
		{
			if ((param >= MIN_INPUTRANGE) && (param <= MAX_INPUTRANGE) && NULL != _hAlazarBoard)
			{
				U8 id = static_cast<U8>(paramID - ICamera::PARAM_LSM_INPUTRANGE1);
				if (_hAlazarBoard->SetInputRange(1 << id, static_cast<U32> (param)))
				{
					_inputRangeChannel[id] = static_cast<long> (param);
				}
			}
		}
		break;
	default:
		ret = FALSE;
		if(_pCameraConfig.get()->IsUIntValue((ICamera::Params)paramID))
		{
			return _pCameraConfig.get()->SetParameter((ICamera::Params)paramID, (uint32_t)param);
		}
		else
		{
			return _pCameraConfig.get()->SetParameter((ICamera::Params)paramID, param);
		}
		break;
	}
	return ret;
}

long ThorLSMCam::SetParamBuffer(const long paramID, char * buffer, long size)
{
	long ret = FALSE;
	switch (paramID)
	{
	case ICamera::PARAM_MESO_SCAN_INFO:
		{
			if (_preScan != NULL)
			{
				delete _preScan;
				_preScan = NULL;
			}
			if ((NULL != buffer) && 0 < size)
			{
				_preScan = new Scan(*static_cast<Scan*>((void*)buffer));
				ret = TRUE;
			}
			break;
		}
	default:
		break;
	}
	return ret;
}

long ThorLSMCam::SetParamString(const long paramID, wchar_t * str)
{
	long ret = FALSE;
	switch (paramID)
	{
	case ICamera::PARAM_MESO_EXP_PATH:
		_expLoader.get()->ExpPath = wstring(str);
		ret =_expLoader.get()->LoadExperimentXML();
		if(TRUE == ret)
		{
			for (int i = 0; i < static_cast<int>(_expLoader.get()->Scans.size()); i++)
			{
				SetParamBuffer(ICamera::PARAM_MESO_SCAN_INFO, (char*)_expLoader.get()->Scans[i], sizeof(_expLoader.get()->Scans[i]));
			}
		}
		else
		{
			SetParamBuffer(ICamera::PARAM_MESO_SCAN_INFO, NULL, 0);
		}
		break;
	default:
		break;
	}

	return ret;
}

void ThorLSMCam::SetStatusHandle(HANDLE handle)
{
}

long ThorLSMCam::SetMapParam(const long paramID, double inputValue, double param)
{
	if (paramID == PARAM_MESO_TWOWAY_ALIGNMENT_SHIFT)
	{
		return _pCameraConfig.get()->SetTwoWayAlignmentPoint(inputValue, param);
	}
	else if (paramID == PARAM_MESO_RESONANT_FIELD_TO_VOLTAGE)
	{
		return _pCameraConfig.get()->SetReosnantVoltage(inputValue, param);
	}
	return FALSE;
}
