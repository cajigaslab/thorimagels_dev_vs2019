#include "Strsafe.h"
#include "ThorKurios.h"

ThorKurios::ThorKurios()
{
	_wavelength = 0;
	_wavelengthMin = 0;
	_wavelengthMax = 0;
	_bandwidthMode = 0;
	_bandwidthModeMin = 0;
	_bandwidthModeMax = 0;
	_controlMode = 1;
	_controlModeMin = 1;
	_controlModeMax = 5;
	_temperatureStatusMin = 0;
	_temperatureStatusMax = 2;
	_triggerOutSignalMode = 0;
	_triggerOutSignalModeMin = 0;
	_triggerOutSignalModeMax = 1;
	_forceTrigger = 1;
	_forceTriggerMin = 1;
	_forceTriggerMax = 1;
	_switchDelay=1;
	_switchDelayMin = 1;
	_switchDelayMax = 255;
	_triggerOutTimeMode = 0;
	_triggerOutTimeModeMin = 0;
	_triggerOutTimeModeMax = 1;
	_deleteSequence = 0;
	_deleteSequenceMin = 0;
	_deleteSequenceMax = 1024;

	_cmdWL = FALSE;
	_cmdOM = FALSE;
	_cmdBW = FALSE;
	_cmdSS = FALSE;
	_cmdIS = FALSE;
	_cmdDS = FALSE;
	_cmdTO = FALSE;
	_cmdET = FALSE;
	_cmdVA = FALSE;
	_cmdTT = FALSE;
	_kurios = new kurios_handler;
	_isInit = _kurios->init() == SUCCESS;

	_cmdFS = 0;
	_errMsg[0] = NULL;
	_insertSequence = nullptr;
	_setSequence = nullptr;
}

ThorKurios::~ThorKurios()
{
	_instanceFlag = false;
	delete _kurios;
	_kurios = NULL;
}

bool ThorKurios::_instanceFlag = false;

auto_ptr<ThorKurios> ThorKurios::_single(new ThorKurios());

ThorKurios *ThorKurios::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorKurios());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorKurios::FindDevices(long &deviceCount)
{
	if (!_isInit) return FALSE;
	_devices.clear();
	char *serialNo = new char[MSG_SIZE];
	_kurios->List(serialNo, MSG_SIZE);
	char *p;
	char *token = strtok_s(serialNo, ",", &p);
	while (token != NULL)
	{
		char *sn = new char[strlen(token) + 1];
		strcpy_s(sn, strlen(token) + 1, token);
		token = strtok_s(NULL, ",", &p);
		if (token != NULL)
		{
			if (strstr(token, "Kurios") != NULL || strstr(token, "KURIOS") != NULL)
			{
				_devices.push_back(sn);
			}
			token = strtok_s(NULL, ",", &p);
		}
	}
	deviceCount = (long)_devices.size();
	if (deviceCount == 0)
		return FALSE;
	else
		return TRUE;
}

long ThorKurios::SelectDevice(const long device)
{
	if (!_isInit) return FALSE;
	if (_devices.size() <= 0)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"No KURIOS device connected.");
		return FALSE;
	}
	if ((device < 0) || ((size_t)device >= _devices.size()))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Paramter device should be between %d and %d", 0, _devices.size());
		return FALSE;
	}
	if (_kurios->Open(_devices[device], DEFAULTBAUDRATE, DEFAULTTIMEOUT) != SUCCESS)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Connection failed.");
		return FALSE;
	}
	if (_kurios->GetWavelengthRange(_wavelengthMax, _wavelengthMin) != SUCCESS)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Connection failed.");
		return FALSE;
	}
	if (_kurios->GetAvailableBandwidthMode(_bandwidthModeMax, _bandwidthModeMin) != SUCCESS)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Connection failed.");
		return FALSE;
	}
	return TRUE;
}

long ThorKurios::TeardownDevice()
{
	if (!_isInit) return FALSE;
	if (!_kurios) return FALSE;
	_kurios->Close();
	return TRUE;
}

long ThorKurios::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	if (!_isInit) return FALSE;
	long ret = TRUE;
	switch (paramID)
	{
	case PARAM_KURIOS_WAVELENGTH:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _wavelengthMin;
			paramMax = _wavelengthMax;
			paramDefault = _wavelengthMin;
		}
		break;
	case PARAM_KURIOS_BANDWIDTHMODE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _bandwidthModeMin;
			paramMax = _bandwidthModeMax;
			paramDefault = _bandwidthModeMin;
		}
		break;
	case PARAM_KURIOS_TEMPERATURE:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 0;
			paramDefault = 0;
		}
		break;
	case PARAM_KURIOS_TEMPERATURESTATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _temperatureStatusMin;
			paramMax = _temperatureStatusMax;
			paramDefault = _temperatureStatusMin;
		}
		break;
	case PARAM_KURIOS_CONTROLMODE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _controlModeMin;
			paramMax = _controlModeMax;
			paramDefault = _controlModeMin;
		}
		break;
	case PARAM_KURIOS_GETSEQUENCE:
		{
			paramType = TYPE_BUFFER;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 0;
			paramDefault = 0;
		}
		break;
	case PARAM_KURIOS_SETSEQUENCE:
		{
			paramType = TYPE_BUFFER;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 0;
			paramDefault = 0;
		}
		break;
	case PARAM_KURIOS_INSERTSEQUENCE:
		{
			paramType = TYPE_BUFFER;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 0;
			paramDefault = 0;
		}
		break;
	case PARAM_KURIOS_DELETESEQUENCE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _deleteSequenceMin;
			paramMax = _deleteSequenceMax;
			paramDefault = _deleteSequenceMin;
		}
		break;
	case PARAM_KURIOS_FASTSWITCHINGDATA:
		{
			paramType = TYPE_BUFFER;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 0;
			paramDefault = 0;
		}
		break;
	case PARAM_KURIOS_TRIGGEROUTSIGNALMODE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _triggerOutSignalModeMin;
			paramMax = _triggerOutSignalModeMax;
			paramDefault = _triggerOutSignalModeMin;
		}
		break;
	case PARAM_KURIOS_FORCETRIGGER:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _forceTriggerMin;
			paramMax = _forceTriggerMax;
			paramDefault = _forceTriggerMin;
		}
		break;
	case PARAM_KURIOS_TRIGGEROUTTIMEMODE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _triggerOutTimeModeMin;
			paramMax = _triggerOutTimeModeMax;
			paramDefault = _triggerOutTimeModeMin;
		}
		break;
	case PARAM_KURIOS_SWITCHDELAY:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _switchDelayMin;
			paramMax = _switchDelayMax;
			paramDefault = _switchDelayMin;
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = (double)ConnectionStatusType::CONNECTION_WARMING_UP;
			paramMax = (double)ConnectionStatusType::CONNECTION_ERROR_STATE;
			paramDefault = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
		break;
	default:
		paramAvailable = FALSE;
		ret = FALSE;
	}
	return ret;
}

long ThorKurios::SetParam(const long paramID, const double param)
{
	if (!_isInit) return FALSE;
	long ret = TRUE;
	switch (paramID)
	{
	case PARAM_KURIOS_WAVELENGTH:
		{
			if ((param >= _wavelengthMin) && (param <= _wavelengthMax))
			{
				_wavelength = static_cast<long>(param);
				_cmdWL = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_KURIOS_WAVELENGTH out of range %d to %d", _wavelengthMin, _wavelengthMax);
				ret = FALSE;
			}
		}
		break;
	case PARAM_KURIOS_BANDWIDTHMODE:
		{
			if ((param >= _bandwidthModeMin) && (param <= _bandwidthModeMax))
			{
				_bandwidthMode = static_cast<long>(param);
				_cmdBW = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_KURIOS_BANDWIDTHMODE out of range %d to %d", _bandwidthModeMin, _bandwidthModeMax);
				ret = FALSE;
			}
		}
		break;
	case PARAM_KURIOS_CONTROLMODE:
		{
			if ((param >= _controlModeMin) && (param <= _controlModeMax))
			{
				_controlMode = static_cast<long>(param);
				_cmdOM = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_KURIOS_CONTROLMODE out of range %d to %d", _controlModeMin, _controlModeMax);
				ret = FALSE;
			}
		}
		break;
	case PARAM_KURIOS_DELETESEQUENCE:
		{
			if ((param >= _deleteSequenceMin) && (param <= _deleteSequenceMax))
			{
				_deleteSequence = static_cast<long>(param);
				_cmdDS = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_KURIOS_DELETESEQUENCE out of range %d to %d", _deleteSequenceMin, _deleteSequenceMax);
				ret = FALSE;
			}
		}
		break;
	case PARAM_KURIOS_TRIGGEROUTSIGNALMODE:
		{
			if ((param >= _triggerOutSignalModeMin) && (param <= _triggerOutSignalModeMax))
			{
				_triggerOutSignalMode = static_cast<long>(param);
				_cmdTO = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_KURIOS_TRIGGEROUTSIGNALMODE out of range %d to %d", _triggerOutSignalModeMin, _triggerOutSignalModeMax);
				ret = FALSE;
			}
		}
		break;
	case PARAM_KURIOS_FORCETRIGGER:
		{
			if ((param >= _forceTriggerMin) && (param <= _forceTriggerMax))
			{
				_forceTrigger = static_cast<long>(param);
				_cmdET = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_KURIOS_FORCETRIGGER out of range %d to %d", _forceTriggerMin, _forceTriggerMax);
				ret = FALSE;
			}
		}
		break;
	case PARAM_KURIOS_TRIGGEROUTTIMEMODE:
		{
			if ((param >= _triggerOutTimeModeMin) && (param <= _triggerOutTimeModeMax))
			{
				_triggerOutTimeMode = static_cast<long>(param);
				_cmdVA = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_KURIOS_TRIGGEROUTTIMEMODE out of range %d to %d", _triggerOutTimeModeMin, _triggerOutTimeModeMax);
				ret = FALSE;
			}
		}
		break;
	case PARAM_KURIOS_SWITCHDELAY:
		{
			if ((param >= _switchDelayMin) && (param <= _switchDelayMax))
			{
				_switchDelay = static_cast<long>(param);
				_cmdTT = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_KURIOS_SWITCHDELAY out of range %d to %d", _switchDelayMin, _switchDelayMax);
				ret = FALSE;
			}
		}
		break;
	case PARAM_KURIOS_TEMPERATURE:
	case PARAM_KURIOS_TEMPERATURESTATUS:
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter is read only");
			ret = FALSE;
		}
		break;
	default:
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter not implemented");
			ret = FALSE;
		}
	}
	return ret;
}

long ThorKurios::SetParamString(const long paramID, wchar_t * str)
{
	if (!_isInit) return FALSE;
	long ret = TRUE;
	return ret;
}

long ThorKurios::SetParamBuffer(const long paramID, char * buffer, long size)
{
	if (!_isInit) return FALSE;
	long ret = TRUE;
	switch (paramID)
	{
	case PARAM_KURIOS_SETSEQUENCE:
		{
			_setSequence = (char*)malloc(size* sizeof(char));
			memset(_setSequence, '\0', size* sizeof(char));
			if (_setSequence != NULL)
			{
				memcpy(_setSequence, buffer, size * sizeof(char) + 1);
				_cmdSS = TRUE;
			}
		}
		break;
	case PARAM_KURIOS_INSERTSEQUENCE:
		{
			_insertSequence = (char*)malloc(size* sizeof(char));
			memset(_insertSequence, '\0', size* sizeof(char));
			if (_insertSequence != NULL)
			{
				memcpy(_insertSequence, buffer, size * sizeof(char) + 1);
				_cmdIS = TRUE;
			}
		}
		break;
	case PARAM_KURIOS_FASTSWITCHINGDATA:
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter is read only");
			ret = FALSE;
		}
		break;
	default:
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter not implemented");
			ret = FALSE;
		}
	}
	return ret;
}

long ThorKurios::GetParam(const long paramID, double &param)
{
	if (!_isInit) return FALSE;
	long ret = TRUE;
	switch (paramID)
	{
	case PARAM_DEVICE_TYPE:
		{
			param = SPECTRUM_FILTER;
		}
		break;
	case PARAM_KURIOS_WAVELENGTH:
		{
			long val = 0;
			_kurios->GetValue(KURIOS_GET_WAVELENGTH, val);
			param = val;
		}
		break;
	case PARAM_KURIOS_BANDWIDTHMODE:
		{
			long val = 0;
			_kurios->GetValue(KURIOS_GET_BANDWIDTHMODE, val);
			param = val;
		}
		break;
	case PARAM_KURIOS_CONTROLMODE:
		{
			long val = 0;
			_kurios->GetValue(KURIOS_GET_CONTROLMODE, val);
			param = val;
		}
		break;
	case PARAM_KURIOS_TEMPERATURE:
		{
			_kurios->GetValue(KURIOS_GET_TEMPERATURE, param);
		}
		break;
	case PARAM_KURIOS_TEMPERATURESTATUS:
		{
			long val = 0;
			_kurios->GetValue(KURIOS_GET_TEMPERATURESTATUS, val);
			param = val;
		}
		break;
	case PARAM_KURIOS_TRIGGEROUTSIGNALMODE:
		{
			long val = 0;
			_kurios->GetValue(KURIOS_GET_TRIGGEROUTSIGNALMODE, val);
			param = val;
		}
		break;
	case PARAM_KURIOS_FORCETRIGGER:
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter not implemented");
			ret = FALSE;
		}
		break;
	case PARAM_KURIOS_TRIGGEROUTTIMEMODE:
		{
			long val = 0;
			_kurios->GetValue(KURIOS_GET_TRIGGEROUTTIMEMODE, val);
			param = val;
		}
		break;
	case PARAM_KURIOS_SWITCHDELAY:
		{
			long val = 0;
			_kurios->GetValue(KURIOS_GET_SWITCHDELAY, val);
			param = val;
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = (_devices.size() > 0) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
		break;
	default:
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter not implemented");
			ret = FALSE;
		}
	}
	return ret;
}

long ThorKurios::GetParamString(const long paramID, wchar_t * str, long size)
{
	if (!_isInit) return FALSE;
	long ret = TRUE;
	return ret;
}

long ThorKurios::GetParamBuffer(const long paramID, char * buffer, long size)
{
	if (!_isInit) return FALSE;
	long ret = TRUE;
	switch (paramID)
	{
	case PARAM_KURIOS_GETSEQUENCE:
		{
			_kurios->GetValue(KURIOS_GET_ALLSEQUENCES, buffer);
		}
		break;
	case PARAM_KURIOS_FASTSWITCHINGDATA:
		{
			_kurios->GetValue(KURIOS_GET_FASTSWITCHINGDATA, buffer);
		}
		break;
	default:
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter not implemented");
			ret = FALSE;
		}
	}
	return ret;
}

long ThorKurios::PreflightPosition()
{
	if (!_isInit) return FALSE;
	return TRUE;
}

long ThorKurios::SetupPosition()
{
	if (!_isInit) return FALSE;
	return TRUE;
}

long ThorKurios::StartPosition()
{
	if (!_isInit) return FALSE;
	if (_cmdWL)
	{
		if(_kurios->SetValue(KURIOS_SET_WAVELENGTH, _wavelength) == SUCCESS)
		{
			_cmdWL = FALSE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetKuriosWavelength failed");
			return FALSE;
		}
	}
	if (_cmdBW)
	{
		if (_kurios->SetValue(KURIOS_SET_BANDWIDTHMODE, _bandwidthMode) == SUCCESS)
		{
			_cmdBW = FALSE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetKuriosBandwidthMode failed");
			return FALSE;
		}
	}
	if (_cmdOM)
	{
		if (_kurios->SetValue(KURIOS_SET_CONTROLMODE, _controlMode) == SUCCESS)
		{
			_cmdOM = FALSE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetKuriosControlMode failed");
			return FALSE;
		}
	}
	if (_cmdTO)
	{
		if (_kurios->SetValue(KURIOS_SET_TRIGGEROUTSIGNALMODE, _triggerOutSignalMode) == SUCCESS)
		{
			_cmdTO = FALSE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetKuriosTriggerOutSignalMode failed");
			return FALSE;
		}
	}
	if (_cmdET)
	{
		if (_kurios->SetValue(KURIOS_SET_FORCETRIGGER, _forceTrigger) == SUCCESS)
		{
			_cmdET = FALSE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetKuriosSwitchToNextWavelength failed");
			return FALSE;
		}
	}
	if (_cmdVA)
	{
		if (_kurios->SetValue(KURIOS_SET_TRIGGEROUTTIMEMODE, _triggerOutTimeMode) == SUCCESS)
		{
			_cmdVA = FALSE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetKuriosTriggerOutTimeMode failed");
			return FALSE;
		}
	}
	if (_cmdTT)
	{
		if (_kurios->SetValue(KURIOS_SET_SWITCHDELAY, _switchDelay) == SUCCESS)
		{
			_cmdTT = FALSE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetKuriosSwitchDelay failed");
			return FALSE;
		}
	}
	if (_cmdSS)
	{
		if (_kurios->SetValue(KURIOS_SET_SETSEQUENCE, _setSequence) == SUCCESS)
		{
			_cmdSS = FALSE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetSequence failed");
			return FALSE;
		}
	}
	if (_cmdIS)
	{
		if (_kurios->SetValue(KURIOS_SET_INSERTSEQUENCE, _insertSequence) == SUCCESS)
		{
			_cmdIS = FALSE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"InsertSequence failed");
			return FALSE;
		}
	}
	if (_cmdDS)
	{
		if (_kurios->SetValue(KURIOS_SET_DELETESEQUENCE, _deleteSequence) == SUCCESS)
		{
			_cmdDS = FALSE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"DeleteKuriosSequence failed");
			return FALSE;
		}
	}
	return TRUE;
}

long ThorKurios::StatusPosition(long &status)
{
	if (!_isInit) return FALSE;
	return TRUE;
}

long ThorKurios::ReadPosition(DeviceType deviceType, double &pos)
{
	if (!_isInit) return FALSE;
	return TRUE;
}

long ThorKurios::PostflightPosition()
{
	if (!_isInit) return FALSE;
	return TRUE;
}

long ThorKurios::GetLastErrorMsg(wchar_t *msg, long size)
{
	if (!_isInit) return FALSE;
	wcsncpy_s(msg, size, _errMsg, MSG_SIZE);
	return TRUE;
}

