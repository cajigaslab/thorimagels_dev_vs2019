#include "stdafx.h"
#include "ThorDAQRemoteFocus.h"
#include "ThorDAQRemoteFocusXML.h"

#ifdef LOGGING_ENABLED
std::auto_ptr<LogDll> qlogDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

/// <summary>
/// Prevents a default instance of the <see cref="ThorDAQRemoteFocus" /> class from being created.
/// </summary>
ThorDAQRemoteFocus::ThorDAQRemoteFocus()
{
	_deviceDetected = FALSE;
	_numDevices = 0;

	_zPos = 0.0;
	_zPos_C = 0.0;
	_waveformOutPath = L"";
	_analog_mode = static_cast<long>(ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT);
	_analog_mode_C = _analog_mode;
	_volume_time_min = .001;
	_volume_time_max = 425000;					//[sec], limit from NI card buffer size LONG_MAX(2147483647)/_piezoSampleRate - _z_fast_flyback_time_max
	_volume_time = _volume_time_min;
	_positionBufferSize = 0;

	_flyback_time_min = 0.0;
	_flyback_time_max = 4400;					//[sec]
	_flyback_time = _flyback_time_min;

	_flyback_timeAdjustMS_min = -static_cast<double>(Constants::EVENT_WAIT_TIME);	//[-5 sec]
	_flyback_timeAdjustMS_max = static_cast<double>(Constants::EVENT_WAIT_TIME);		//[+5 sec]

	_flybackTimeAdjustMS = 0.0;

	_step_timeAdjustMS_min = -static_cast<double>(Constants::EVENT_WAIT_TIME);		//[-5 sec]
	_step_timeAdjustMS_max = static_cast<double>(Constants::EVENT_WAIT_TIME);		//[+5 sec]
	_volumeTimeAdjustMS = _stepTimeAdjustMS = 0.0;
	_zStepTime = _zIntraStepTime = 0.0;
	_zPockelsPowerBuffer = _zPositionBuffer = NULL;
	_volumePoints = _stepPoints = _flybackPoints = 0;
	_errMsg[0] = NULL;
	_DAQDeviceIndex = DEFAULT_CARD_NUMBER;

	_boardInfo = BOARD_INFO_STRUCT();
	_numberOfPlanes = MIN_PLANES;
	_positionsVoltageValues = { };
}

wchar_t ThorDAQRemoteFocus::thordaqLogMessage[MSG_SIZE];
USHORT* ThorDAQRemoteFocus::_pThorDAQRemoteFocusWaveform = NULL;
long ThorDAQRemoteFocus::_totalPoints = 0;

std::unique_ptr<WaveformSaverDLL> WaveformSaver(new WaveformSaverDLL(L".\\Modules_Native\\GeometryUtilitiesCPP.dll"));

/// <summary>
/// Finalizes an instance of the <see cref="ThorDAQRemoteFocus" /> class.
/// </summary>
ThorDAQRemoteFocus::~ThorDAQRemoteFocus()
{
	_instanceFlag = false;
}

/// <summary>
/// The _instance flag{CC2D43FA-BBC4-448A-9D0B-7B57ADF2655C}, must initialize after constructor
/// </summary>
bool ThorDAQRemoteFocus::_instanceFlag = false;
unique_ptr<ThorDAQRemoteFocus> ThorDAQRemoteFocus::_single(new ThorDAQRemoteFocus());

wchar_t message[MAX_PATH];

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorDAQRemoteFocus *.</returns>
ThorDAQRemoteFocus* ThorDAQRemoteFocus::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorDAQRemoteFocus());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="DeviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::FindDevices(long& deviceCount)
{
	deviceCount = 0;
	//Get filter parameters from hardware setup.xml

	long numOfPlanes = 1;
	int error = 0, retVal = 0;
	try
	{
		retVal = ThorDAQAPIBindBoard(_DAQDeviceIndex);
		BOARD_INFO_STRUCT boardInfo = BOARD_INFO_STRUCT();
		retVal = ThorDAQAPIGetBoardCfg(_DAQDeviceIndex, &boardInfo);
		ThordaqErrChk(L"ThorDAQAPIBindBoard", retVal = ThorDAQAPIBindBoard(_DAQDeviceIndex));

		ThordaqErrChk(L"ThorDAQAPIGetBoardCfg", retVal = ThorDAQAPIGetBoardCfg(_DAQDeviceIndex, &boardInfo));
		_boardInfo = boardInfo;

		UINT8 major = static_cast<UINT8>((boardInfo.UserVersion & THORDAQ_FIRMWARE_VERSION_RELEASE_MAJOR_BIT_SELECT) >> 28);
		UINT8 minor = static_cast<UINT8>((boardInfo.UserVersion & THORDAQ_FIRMWARE_VERSION_RELEASE_MINOR_BIT_SELECT) >> 24);
		UINT32 date = static_cast<UINT32>(boardInfo.UserVersion & THORDAQ_FIRMWARE_VERSION_RELEASE_DATE_BIT_SELECT);

		if ((major < THORDAQ_FIRMWARE_VERSION_RELEASE_MAJOR_MIN || (major == THORDAQ_FIRMWARE_VERSION_RELEASE_MAJOR_MIN && minor < THORDAQ_FIRMWARE_VERSION_RELEASE_MINOR_MIN) || date < THORDAQ_FIRMWARE_VERSION_RELEASE_DATE_MIN) &&
			(boardInfo.UserVersion < THORDAQ_FIRMWARE_VERSION_DEBUG_MIN || boardInfo.UserVersion > THORDAQ_FIRMWARE_VERSION_DEBUG_MAX))
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, _MAX_PATH, L"ThorDAQ firmware version Should be greater or equal to 0x%X", THORDAQ_FIRMWARE_VERSION_RELEASE_MIN);
			MessageBox(NULL, errMsg, L"Out of date firmware", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
			_numDevices = 0;
			deviceCount = 0;
			return FALSE;
		}

		if (retVal == STATUS_SUCCESSFUL)
		{
			deviceCount = 1;
			_numDevices = 1;
		}
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Hardware communication error ThorDAQ ConnectToBoard failed");
		LogMessage(_errMsg, ERROR_EVENT);
		return FALSE;
	}

	_positionsVoltageValues.clear();
	auto_ptr<ThorDAQRemoteFocusXML> pSetup(new ThorDAQRemoteFocusXML());
	if (pSetup->ReadPositionVoltages(numOfPlanes, &_positionsVoltageValues))
	{
		_numberOfPlanes = numOfPlanes;
	}
	else
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetConfig from ThorDAQSettings.XML failed");
		LogMessage(_errMsg, ERROR_EVENT);
		_numDevices = 0;
		deviceCount = 0;

		return FALSE;
	}

	unique_ptr<ThorDAQIOXML> pSetupIO(new ThorDAQIOXML());

	if (!pSetupIO->GetAOLinesConfiguration(_thordaqAOSelection) || _thordaqAOSelection[AO::REMOTE_FOCUS] < 0)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetAOLinesConfiguration from ThorDAQIOSettings.XML failed");
		LogMessage(_errMsg, ERROR_EVENT);
		_numDevices = 0;
		deviceCount = 0;
		return FALSE;
	}

	return TRUE;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="Device">The device.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::SelectDevice(const long device)
{
	if ((device < 0) || (device >= _numDevices))
	{
		return FALSE;
	}

	return TRUE;
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::TeardownDevice()
{
	SAFE_DELETE_ARRAY(_pThorDAQRemoteFocusWaveform);
	return TRUE;
}

/// <summary>
/// Gets the parameter information.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="paramType">Type of the parameter.</param>
/// <param name="paramAvailable">The parameter available.</param>
/// <param name="paramReadOnly">The parameter read only.</param>
/// <param name="paramMin">The parameter minimum.</param>
/// <param name="paramMax">The parameter maximum.</param>
/// <param name="paramDefault">The parameter default.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::GetParamInfo
(
	const long	paramID,
	long& paramType,
	long& paramAvailable,
	long& paramReadOnly,
	double& paramMin,
	double& paramMax,
	double& paramDefault
)
{
	long ret = TRUE;

	switch (paramID)
	{
	case PARAM_Z_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = MIN_PLANES;
		paramMax = _numberOfPlanes;
		paramDefault = MIN_PLANES;
	}
	break;
	case PARAM_Z_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = MIN_PLANES;
		paramMax = _numberOfPlanes;
		paramDefault = MIN_PLANES;
	}
	break;
	case PARAM_REMOTE_FOCUS_NUMBER_OF_PLANES:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = _numberOfPlanes;
		paramMax = _numberOfPlanes;
		paramDefault = _numberOfPlanes;
	}
	break;
	case PARAM_DEVICE_TYPE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = paramMax = paramDefault = STAGE_Z;
	}
	break;
	case PARAM_CONNECTION_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = CONNECTION_READY;
		paramMax = paramDefault = CONNECTION_UNAVAILABLE;
	}
	break;
	case PARAM_Z_FAST_VOLUME_TIME:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _volume_time_min;
		paramMax = _volume_time_max;
		paramDefault = _volume_time_min;
	}
	break;
	case PARAM_Z_FAST_FLYBACK_TIME:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _flyback_time_min;
		paramMax = _flyback_time_max;
		paramDefault = _flyback_time_min;
	}
	break;
	case PARAM_Z_FAST_STEP_BUFFER:
	{
		paramType = TYPE_BUFFER;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
	}
	break;
	case PARAM_Z_STAGE_TYPE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = paramMax = paramDefault = REMOTE_FOCUS;
	}
	break;
	case PARAM_Z_FAST_FLYBACK_TIME_ADJUST_MS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _flyback_timeAdjustMS_min;
		paramMax = _flyback_timeAdjustMS_max;
		paramDefault = 0.0;
	}
	break;
	case PARAM_Z_FAST_STEP_TIME_ADJUST_MS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _step_timeAdjustMS_min;
		paramMax = _step_timeAdjustMS_max;
		paramDefault = 0.0;
	}
	break;
	case PARAM_Z_FAST_STEP_TIME:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0.0;
		paramMax = _volume_time_max;
		paramDefault = 0.0;
	}
	break;
	case PARAM_Z_FAST_INTRA_STEP_TIME:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0.0;
		paramMax = _volume_time_max;
		paramDefault = 0.0;
	}
	break;
	case PARAM_Z_POCKELS_MIN:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0.0;
		paramMax = MAX_AO_VOLTAGE;
		paramDefault = 0.0;
	}
	break;
	case PARAM_WAVEFORM_OUTPATH:
	{
		paramType = TYPE_STRING;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
	}
	break;
	case PARAM_Z_ANALOG_MODE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT;
		paramMax = ZPiezoAnalogMode::ANALOG_MODE_LAST;
		paramDefault = ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT;
	}
	break;
	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::SetParam(const long paramID, const double param)
{
	long ret = FALSE;

	switch (paramID)
	{
	case PARAM_Z_POS:
	{
		if ((param >= MIN_PLANES) && (param <= _numberOfPlanes))
		{
			_zPos = static_cast<double>(param);
			ret = TRUE;
		}
		else
		{
			wstring message = L"ThorDAQRemoteFocus SetParam failed. paramID: PARAM_Z_POS out of bounds. Value: %f Min: 1 Max: %f", param, _numberOfPlanes;
			StringCbPrintfW(_errMsg, MSG_SIZE, message.c_str());
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case PARAM_Z_ANALOG_MODE:
	{
		if ((param >= ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT) && (param <= ZPiezoAnalogMode::ANALOG_MODE_LAST))
		{
			_analog_mode = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_VOLUME_TIME:
	{
		if ((param >= _volume_time_min) && (param <= _volume_time_max))
		{
			_volume_time = static_cast<double>(param);
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_FLYBACK_TIME:
	{
		if ((param >= _flyback_time_min) && (param <= _flyback_time_max))
		{
			_flyback_time = static_cast<double>(param);
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_FLYBACK_TIME_ADJUST_MS:
	{
		if ((_flyback_timeAdjustMS_min <= param) && (_flyback_timeAdjustMS_max >= param))
		{
			_flybackTimeAdjustMS = param;
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_STEP_TIME_ADJUST_MS:
	{
		if ((_step_timeAdjustMS_min <= param) && (_step_timeAdjustMS_max >= param))
		{
			_stepTimeAdjustMS = param;
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_STEP_TIME:
	{
		_zStepTime = param;
		ret = TRUE;
	}
	break;
	case PARAM_Z_FAST_INTRA_STEP_TIME:
	{
		_zIntraStepTime = param;
		ret = TRUE;
	}
	break;
	default:
		break;
	}
	return ret;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::GetParam(const long paramID, double& param)
{
	long ret = TRUE;

	switch (paramID)
	{
	case PARAM_Z_POS:
	{
		param = static_cast<double>(_zPos);
	}
	break;
	case PARAM_Z_POS_CURRENT:
	{
		param = static_cast<double>(_zPos_C);
	}
	break;
	case PARAM_Z_ANALOG_MODE:
	{
		param = static_cast<long>(_analog_mode_C);
	}
	break;
	case PARAM_REMOTE_FOCUS_NUMBER_OF_PLANES:
	{
		param = static_cast<long>(_numberOfPlanes);
	}
	break;
	case PARAM_Z_STAGE_TYPE:
	{
		param = static_cast<double>(REMOTE_FOCUS);
	}
	break;
	case PARAM_DEVICE_TYPE:
	{
		param = static_cast<double>(STAGE_Z);
	}
	break;
	case PARAM_CONNECTION_STATUS:
	{
		param = (_numDevices > 0) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
	}
	break;
	case PARAM_Z_FAST_VOLUME_TIME:
	{
		param = static_cast<double>(_volume_time_C);
	}
	break;
	case PARAM_Z_FAST_FLYBACK_TIME:
	{
		param = static_cast<double>(_flyback_time_C);
	}
	break;
	case PARAM_Z_FAST_FLYBACK_TIME_ADJUST_MS:
	{
		param = static_cast<double>(_flybackTimeAdjustMS);
	}
	break;
	case PARAM_Z_FAST_STEP_TIME_ADJUST_MS:
	{
		param = static_cast<double>(_stepTimeAdjustMS);
	}
	break;
	case PARAM_Z_FAST_STEP_TIME:
	{
		param = static_cast<double>(_zStepTime);
	}
	break;
	case PARAM_Z_FAST_INTRA_STEP_TIME:
	{
		param = static_cast<double>(_zIntraStepTime);
	}
	break;
	default:
		ret = FALSE;
	}

	return ret;
}

/// <summary>
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;
	switch (paramID)
	{
	case IDevice::PARAM_Z_FAST_STEP_BUFFER:
	{
		SAFE_DELETE_ARRAY(_zPositionBuffer);

		try
		{
			_zPositionBuffer = new double[size];
			_positionBufferSize = size;
			SAFE_MEMCPY((void*)(_zPositionBuffer), size * sizeof(double), (void*)(pBuffer));
		}
		catch (...)
		{
			StringCbPrintfW(_errMsg, MAX_PATH, L"Could not allocate PARAM_Z_FAST_STEP_BUFFER");
			LogMessage(_errMsg, VERBOSE_EVENT);
		}
	}
	break;
	default:
	{
		ret = FALSE;
		StringCbPrintfW(_errMsg, MAX_PATH, L"Parameter (%d) not implemented", paramID);
		LogMessage(_errMsg, VERBOSE_EVENT);
	}
	}
	return ret;
}

/// <summary>
/// Gets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	return ret;
}

/// <summary>
/// Sets the parameter string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::SetParamString(const long paramID, wchar_t* str)
{
	long ret = TRUE;
	switch (paramID)
	{
	case IDevice::PARAM_WAVEFORM_OUTPATH:
		_waveformOutPath = std::wstring(str);
		break;
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

/// <summary>
/// Gets the parameter of type string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = TRUE;
	switch (paramID)
	{
	case IDevice::PARAM_WAVEFORM_OUTPATH:
		wcscpy_s(str, size, _waveformOutPath.c_str());
		break;
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::PreflightPosition()
{
	switch (static_cast<ZPiezoAnalogMode>(_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
		break;
	default:
		break;
	}
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::SetupPosition()
{
	switch (static_cast<ZPiezoAnalogMode>(_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
	{
		//place the signal at the first point in the waveform
		//before running the task
		int error = 0;
		int retVal = 0;

		double startV = _positionsVoltageValues[_zPositionBuffer[0] - 1];
		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _thordaqAOSelection[AO::REMOTE_FOCUS], startV));
		ThordaqErrChk(L"ThorDAQAPISetDACOffsetValue", retVal = ThorDAQAPISetDACOffsetValue(_DAQDeviceIndex, _thordaqAOSelection[AO::REMOTE_FOCUS], startV));

		//move to the start position
		Sleep(100);
	}
	break;
	default:
		break;
	}
	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::StartPosition()
{
	long ret = FALSE;

	switch (static_cast<ZPiezoAnalogMode>(_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT:
	{
		//perform position to volts conversion
		double voltage = _positionsVoltageValues[_zPos - 1];
		int error = 0;
		int retVal = 0;

		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _thordaqAOSelection[AO::REMOTE_FOCUS], voltage));
		ThordaqErrChk(L"ThorDAQAPISetDACOffsetValue", retVal = ThorDAQAPISetDACOffsetValue(_DAQDeviceIndex, _thordaqAOSelection[AO::REMOTE_FOCUS], voltage));
		if (retVal == STATUS_SUCCESSFUL)
		{
			_zPos_C = _zPos;
			ret = TRUE;
		}
		break;
	}
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
	{
		_useWaveformMode = true;
		ret = TRUE;
		break;
	}
	default:
		break;
	}

	_analog_mode_C = _analog_mode;
	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::StatusPosition(long& status)
{
	status = IDevice::STATUS_READY;
	return TRUE;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::ReadPosition(DeviceType deviceType, double& pos)
{
	long	ret = FALSE;

	if (deviceType & (STAGE_Z))
	{
		pos = _zPos_C;
		ret = TRUE;
	}
	return ret;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::PostflightPosition()
{
	int error = 0, retVal = 0;
	_useWaveformMode = false;

	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="errMsg">The error MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorDAQRemoteFocus::GetLastErrorMsg(wchar_t* msg, long size)
{
	wcsncpy_s(msg, size, _errMsg, MAX_PATH);
	return TRUE;
}


void ThorDAQRemoteFocus::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	qlogDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

/// <summary>
/// Builds Rmote Focus waveform for ANALOG_MODE_STAIRCASE_WAVEFORM.
/// </summary>
double* ThorDAQRemoteFocus::BuildStaircaseWaveform()
{
	double posV = 0;

	double* pWaveform = (double*)malloc(_totalPoints * sizeof(double));
	if (NULL == pWaveform)
		return NULL;

	memset(pWaveform, 0x0, _totalPoints * sizeof(double));

	//build volume of multiple steps
	long k = 0, j = 0;
	for (long i = 0; i < _positionBufferSize; i++)
	{
		posV = _positionsVoltageValues[_zPositionBuffer[i] - 1];
		//build step
		for (j = 0; j < _stepPoints; j++)
		{
			pWaveform[k++] = max(MIN_AO_VOLTAGE, min(posV, MAX_AO_VOLTAGE));
		}
		//build intra step, except the last step
		double lastStepV = pWaveform[k - 1];
		if (_positionBufferSize - 1 > i)
		{
			posV = _positionsVoltageValues[_zPositionBuffer[i + 1] - 1];
			for (j = 0; j < _intraStepPoints; j++)
			{
				pWaveform[k++] = max(MIN_AO_VOLTAGE, min(posV, MAX_AO_VOLTAGE));
			}
		}
	}
	//build flyback
	posV = _positionsVoltageValues[_zPositionBuffer[0] - 1];
	for (; k < _totalPoints; k++)
	{
		pWaveform[k] = max(MIN_AO_VOLTAGE, min(posV, MAX_AO_VOLTAGE));
	}
	return pWaveform;
}

/// <summary>
/// Build the fast Z Waveform and pass it up in the waveformParams struct with all of it's parameters
/// </summary>
/// <param name="updateRate"></param>
/// <param name="samplesFrame"></param>
/// <param name="samplesFlyback"></param>
/// <param name="waveformParams"></param>
/// <returns></returns>
long ThorDAQRemoteFocus::GetRemoteFocusFastZWaveform(const double updateRate, const long samplesFrame, const long samplesFlyback, const double frameRate, ThorDAQZWaveformParams* waveformParams)
{
	if (nullptr == waveformParams || FALSE == _useWaveformMode)
	{
		return FALSE;
	}
	//build piezo waveform based on analog modes
	double* zWaveform = NULL;

	switch (static_cast<ZPiezoAnalogMode>(_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
	{
		//user defined positions
		if ((0 >= _positionBufferSize) || (NULL == _zPositionBuffer))
			return FALSE;

		_stepPoints = static_cast<long>(samplesFrame + updateRate * ((_stepTimeAdjustMS / Constants::MS_TO_SEC)));	//points per step
		_intraStepPoints = static_cast<long>(samplesFlyback);
		long flybackFrames = static_cast<long>(round((_flyback_time + _flybackTimeAdjustMS / Constants::MS_TO_SEC) * frameRate));
		_flybackPoints = static_cast<long>(flybackFrames * (updateRate * (_flybackTimeAdjustMS / Constants::MS_TO_SEC) + (samplesFrame + samplesFlyback)));
		_volumePoints = _stepPoints * _positionBufferSize + _intraStepPoints * _positionBufferSize;
		_totalPoints = _volumePoints + _flybackPoints;
		if (0 >= _totalPoints)
		{
			StringCbPrintfW(_errMsg, MAX_PATH, L"Remote Focus waveform size is too large.");
			LogMessage(_errMsg, VERBOSE_EVENT);
			return FALSE;
		}
		zWaveform = BuildStaircaseWaveform();
	}
	break;
	}

	//return if waveform building failed
	if (NULL == zWaveform)
	{
		StringCbPrintfW(_errMsg, MAX_PATH, L"ThorDAQRemoteFocus staircase waveform building failed");
		LogMessage(_errMsg, ERROR_EVENT);
		return FALSE;
	}

	//save waveform if requested, once only
	if (0 < _waveformOutPath.length() && TRUE == WaveformSaver.get()->SaveData(_waveformOutPath, SignalType::ANALOG_Z, zWaveform, _totalPoints))
	{
		_waveformOutPath = L"";
	}

	SAFE_DELETE_ARRAY(_pThorDAQRemoteFocusWaveform);

	_pThorDAQRemoteFocusWaveform = new USHORT[_totalPoints + DAC_FIFO_DEPTH]();

	if (_totalPoints > 0)
	{
		SHORT offset = (SHORT)round(zWaveform[0] / DAC_RESOLUTION) + 0x8000;
		for (int i = 0; i < _totalPoints; ++i)
		{
			_pThorDAQRemoteFocusWaveform[i] = (USHORT)(round(zWaveform[i] / DAC_RESOLUTION) + 0x8000 - offset);
		}
	}

	///--------------- FOR TESTING PURPOSES ---------------------
	string waveformFile = "piezoTDQ_waveform.txt";
	ofstream myfile(waveformFile);
	if (myfile.is_open())
	{
		for (int i = 0; i < _totalPoints; i++)
		{
			myfile << std::fixed << std::setprecision(8) << (*(_pThorDAQRemoteFocusWaveform + i));
			myfile << "\n";
		}
		myfile.close();
	}

	string waveformFile2 = "piezo_waveform.txt";
	ofstream myfile2(waveformFile2);
	if (myfile2.is_open())
	{
		for (int i = 0; i < _totalPoints; i++)
		{
			myfile2 << std::fixed << std::setprecision(8) << (*(zWaveform + i));
			myfile2 << "\n";
		}
		myfile2.close();
	}
	///--------------------------------------------------------

	SAFE_DELETE_MEMORY(zWaveform);

	long framesPerVolume = static_cast<long>(round(frameRate * (_totalPoints / updateRate)));

	double park = _positionsVoltageValues[_zPositionBuffer[0] - 1];

	waveformParams->samplesPerFrame = samplesFrame + samplesFlyback;
	waveformParams->framesPerWaveform = framesPerVolume;
	waveformParams->updateRate = updateRate;
	waveformParams->waveformChannel = _thordaqAOSelection[AO::REMOTE_FOCUS];
	waveformParams->waveform = _pThorDAQRemoteFocusWaveform;
	waveformParams->waveformLength = _totalPoints;
	waveformParams->parkPosition = park;
	waveformParams->offsetPosition = park;

	return TRUE;
}