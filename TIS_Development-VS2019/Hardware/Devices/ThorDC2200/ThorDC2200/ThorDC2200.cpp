#include "stdafx.h"
#include "ThorDC2200.h"

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

ThorDC2200::ThorDC2200()
{
	_maxCurrent1 = 1.0;
	_maxCurrent2 = 1.5;
	_terminalNumber = 1;
	_brightnessToSet = 0;
	_modeToSet = MODE_CB;
	_mode = MODE_CB;
	_status = (long)ConnectionStatusType::CONNECTION_UNAVAILABLE;
	_brightness = 0;
	_lamp1enable = FALSE;
	_lamp2enable = FALSE;
	_session = NULL;
	_updateBrightness = FALSE;
	_updateLEDState = FALSE;
	_updateMode = FALSE;
	_updateTerminal = FALSE;
	_viRM = NULL;
}

ThorDC2200::~ThorDC2200()
{
	_instanceFlag = false;
}

bool ThorDC2200::_instanceFlag = false;
std::auto_ptr<ThorDC2200> ThorDC2200::_single(new ThorDC2200());
long ThorDC2200::_ledOutputEnabled = FALSE;

ThorDC2200* ThorDC2200::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorDC2200());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorDC2200::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

long ThorDC2200::FindDevices(long& deviceCount)
{
	long ret = TRUE;
	if (!ThorDC2200Visa::FindAndOpenDevice(_viRM, _session, _status))
	{
		return FALSE;
	}
	deviceCount = 1;
	return ret;
}

long ThorDC2200::SelectDevice(const long Device)
{
	long ret = TRUE;
	ViChar str[256];
	ThorDC2200Visa::GetMode(_session, str);
	_mode = ModeStrToEnum(str);
	_modeToSet = _mode;
	if (_mode == MODE_CB)
	{
		ThorDC2200Visa::SetBrightness(_session, 0);
	}
	else if (_mode == MODE_TTL)
	{
		ThorDC2200Visa::SetTTLCurrent(_session, 0);
	}
	else
	{
		ThorDC2200Visa::SetMode(_session, ModeEnumToStr(2));
	}
	return ret;
}

long ThorDC2200::TeardownDevice()
{
	long state = LED_OUTPUT_OFF;
	ThorDC2200Visa::GetOutputState(_session, state);
	if (LED_OUTPUT_ON == state)
	{
		ThorDC2200Visa::SetOutputState(_session, LED_OUTPUT_OFF);
	}
	ThorDC2200Visa::CloseDevice(_viRM, _session);
	long ret = TRUE;
	return ret;
}

long ThorDC2200::GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault)
{
	long ret = TRUE;
	switch (paramID)
	{
	case PARAM_DEVICE_STATUS:
	{
		paramType = TYPE_BOOL;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
	}
	break;
	case PARAM_LAMP_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 100;
		paramDefault = 0;
	}
	break;
	case PARAM_LAMP_MODE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 1;
		paramMax = 7;
		paramDefault = 0;
	}
	break;
	case PARAM_LAMP_ERRORCODE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = -32768;
		paramMax = 32767;
		paramDefault = 0;
	}
	break;
	case PARAM_CONNECTION_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = (double)ConnectionStatusType::CONNECTION_READY;
		paramMax = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		paramDefault = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
	}
	break;
	case PARAM_LAMP_TERMINAL:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 1;
		paramMax = 2;
		paramDefault = 0;
	}
	break;
	case PARAM_LAMP1_CONNECTION:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
	}
	break;
	case PARAM_LAMP2_CONNECTION:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
	}
	break;
	case IDevice::PARAM_LEDS_ENABLE_DISABLE:
	{
		paramType = IDevice::TYPE_BOOL;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
	}
	break;
	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}
	return ret;
}

long ThorDC2200::SetParam(const long paramID, const double param)
{
	long ret = TRUE;

	switch (paramID)
	{
	case PARAM_LAMP_POS:
		if (_brightnessToSet >= 0 && _brightnessToSet <= 100)
		{
			_brightnessToSet = param;
		}
		break;
	case PARAM_LAMP_MODE:
		_modeToSet = static_cast<int>(param);
		_updateMode = TRUE;
		break;
	case PARAM_LAMP_TERMINAL:
	{
		if (1 == param || 2 == param)
		{
			_terminalNumber = static_cast<int>(param);
			_updateTerminal = TRUE;
		}
	}
	break;
	case PARAM_LAMP1_CONNECTION:
	{
		_lamp1enable = static_cast<int>(param);
	}
	break;
	case PARAM_LAMP2_CONNECTION:
	{
		_lamp2enable = static_cast<int>(param);
	}
	break;
	case IDevice::PARAM_LEDS_ENABLE_DISABLE:
	{
		if (TRUE == param || FALSE == param)
		{
			_ledOutputEnabled = static_cast<long>(param);
			_updateLEDState = TRUE;
		}
	}
	break;
	}
	return ret;
}

long ThorDC2200::GetParam(const long paramID, double& param)
{
	long ret = TRUE;
	long code = 0;
	long state = LED_OUTPUT_OFF;

	switch (paramID)
	{
	case PARAM_DEVICE_TYPE:
		param = static_cast<double>(LAMP);
		break;
	case PARAM_DEVICE_STATUS:
		ThorDC2200Visa::GetOutputState(_session, state);
		param = state;
		break;
	case PARAM_LAMP_POS:
		if (MODE_CB == _mode)
		{
			ThorDC2200Visa::GetBrightness(_session, param);
		}
		else if (MODE_TTL == _mode)
		{
			double current;
			ThorDC2200Visa::GetTTLCurrent(_session, current);
			if (_terminalNumber == 2)
			{
				ThorDC2200Visa::GetTTLMaxCurrent(_session, _maxCurrent2);
			}
			else
			{
				ThorDC2200Visa::GetTTLMaxCurrent(_session, _maxCurrent1);
			}
			if (_terminalNumber == 2)
				param = current * 100 / _maxCurrent2;
			else
				param = current * 100 / _maxCurrent1;
		}
		else
		{
			param = 0;
		}
		break;
	case PARAM_LAMP_MODE:
		ViChar str[256];
		ThorDC2200Visa::GetMode(_session, str);
		param = ModeStrToEnum(str);
		break;
	case PARAM_LAMP_ERRORCODE:
		param = GetLastError();
		break;
	case PARAM_CONNECTION_STATUS:
		param = _status;
		break;
	case PARAM_LAMP_TERMINAL:
		ThorDC2200Visa::GetTerminal(_session, _terminalNumber);
		param = _terminalNumber;
		break;
	case PARAM_LAMP1_CONNECTION:
		ThorDC2200Visa::GetLed1Connection(_session, code);
		if (code == 3)
			param = 1;
		else
			param = 0;
		break;
	case PARAM_LAMP2_CONNECTION:
		ThorDC2200Visa::GetLed2Connection(_session, code);
		if (code == 3)
			param = 1;
		else
			param = 0;
		break;
	case IDevice::PARAM_LEDS_ENABLE_DISABLE:
		ThorDC2200Visa::GetOutputState(_session, state);
		param = static_cast<double>(state);
		break;
	}
	return ret;
}

long ThorDC2200::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;
	return ret;
}

long ThorDC2200::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;
	return ret;
}

long ThorDC2200::SetParamString(const long paramID, wchar_t* str)
{
	long ret = FALSE;
	return ret;
}

long ThorDC2200::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;
	return ret;
}

long ThorDC2200::PreflightPosition()
{
	return TRUE;
}

long ThorDC2200::SetupPosition()
{
	_updateBrightness = TRUE;
	return TRUE;
}

long ThorDC2200::StartPosition()
{
	long ret = FALSE;
	long tempType1;
	long tempType2;
	long state = LED_OUTPUT_OFF;
	if (_updateMode == TRUE)
	{
		char* str = ModeEnumToStr(_modeToSet);
		ThorDC2200Visa::GetOutputState(_session, state);
		if (LED_OUTPUT_ON == state)
		{
			ThorDC2200Visa::SetOutputState(_session, LED_OUTPUT_OFF);
		}
		ThorDC2200Visa::SetMode(_session, str);
		if (TRUE == _ledOutputEnabled)
		{
			ThorDC2200Visa::SetOutputState(_session, LED_OUTPUT_ON);
		}
		_mode = _modeToSet;
		ret = TRUE;
		_updateMode = FALSE;
	}

	if (TRUE == _updateTerminal)
	{
		long state = LED_OUTPUT_OFF;
		ThorDC2200Visa::GetOutputState(_session, state);
		if (LED_OUTPUT_ON == state)
		{
			ThorDC2200Visa::SetOutputState(_session, LED_OUTPUT_OFF);
		}

		ThorDC2200Visa::SetTerminal(_session, _terminalNumber);

		ThorDC2200Visa::GetLed1Connection(_session, tempType1);
		ThorDC2200Visa::GetLed2Connection(_session, tempType2);

		if (tempType1 != 3)
		{
			_lamp1enable = 0;
		}
		else
		{
			_lamp1enable = 1;
		}

		if (tempType2 != 3)
		{
			_lamp2enable = 0;
		}
		else
		{
			_lamp2enable = 1;
		}

		if ((_lamp1enable == 1 && _terminalNumber == 1) || (_lamp2enable == 1 && _terminalNumber == 2))
		{
			if (TRUE == _ledOutputEnabled)
			{
				ThorDC2200Visa::SetOutputState(_session, LED_OUTPUT_ON);
			}
		}
		else if (_lamp1enable == 1)
		{
			ThorDC2200Visa::SetTerminal(_session, 1);
		}
		else if (_lamp2enable == 1)
		{
			ThorDC2200Visa::SetTerminal(_session, 2);
		}

		_updateTerminal = FALSE;
	}

	if (_updateBrightness == TRUE)
	{
		if (_mode == MODE_CB)
		{
			ThorDC2200Visa::SetBrightness(_session, _brightnessToSet);
		}
		else if (_mode == MODE_TTL)
		{
			double current;
			if (_terminalNumber == 2)
			{
				ThorDC2200Visa::GetTTLMaxCurrent(_session, _maxCurrent2);
			}
			else
			{
				ThorDC2200Visa::GetTTLMaxCurrent(_session, _maxCurrent1);
			}
			if (_terminalNumber == 2)
				current = _brightnessToSet * _maxCurrent2 / 100;
			else
				current = _brightnessToSet * _maxCurrent1 / 100;

			ThorDC2200Visa::SetTTLCurrent(_session, current);
		}
		_brightness = _brightnessToSet;
		ret = TRUE;
		_updateBrightness = FALSE;
	}

	if (TRUE == _updateLEDState)
	{
		ThorDC2200Visa::SetOutputState(_session, _ledOutputEnabled);
	}
	return ret;
}
long ThorDC2200::StatusPosition(long& status)
{
	long ret = TRUE;
	status = IDevice::STATUS_READY;
	return ret;
}

long ThorDC2200::ReadPosition(DeviceType deviceType, double& pos)
{
	return TRUE;
}

long ThorDC2200::PostflightPosition()
{
	return TRUE;
}

long ThorDC2200::GetLastErrorMsg(wchar_t* msg, long size)
{
	return TRUE;
}

char* ThorDC2200::ModeEnumToStr(int mode)
{
	switch (mode)
	{
	case MODE_CC:
		return "CC";
	case MODE_CB:
		return "CB";
	case MODE_PWM:
		return "PWM";
	case MODE_PULS:
		return "PULS";
	case MODE_IMOD:
		return "IMOD";
	case MODE_EMOD:
		return "EMOD";
	case MODE_TTL:
		return "TTL";
	default:
		return "";
	}
}

int ThorDC2200::ModeStrToEnum(char* str)
{
	if (strncmp(str, "CC", 2) == 0)
	{
		return MODE_CC;
	}
	else if (strncmp(str, "CB", 2) == 0)
	{
		return MODE_CB;
	}
	else if (strncmp(str, "PWM", 3) == 0)
	{
		return MODE_PWM;
	}
	else if (strncmp(str, "PULS", 4) == 0)
	{
		return MODE_PULS;
	}
	else if (strncmp(str, "IMOD", 4) == 0)
	{
		return MODE_IMOD;
	}
	else if (strncmp(str, "EMOD", 4) == 0)
	{
		return MODE_EMOD;
	}
	else if (strncmp(str, "TTL", 3) == 0)
	{
		return MODE_TTL;
	}
	return 0;
}

long ThorDC2200::GetLastError()
{
	return ThorDC2200Visa::StatusCode;
}
