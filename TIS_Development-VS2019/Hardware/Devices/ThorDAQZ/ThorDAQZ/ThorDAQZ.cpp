#include "stdafx.h"
#include "ThorDAQZ.h"

#include "ThorDAQZXML.h"
#include "..\..\..\Cameras\thordaq\Dll\ThorDAQIOXML.h" //TODO: need to move this file to a more common place just as we need to move the thordaq dll project to a more common place

#ifdef LOGGING_ENABLED
#include "..\..\..\..\..\Common\Log.h"
std::auto_ptr<LogDll> qlogDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

/// <summary>
/// Prevents a default instance of the <see cref="ThorDAQZ" /> class from being created.
/// </summary>
ThorDAQZ::ThorDAQZ()
{
	_deviceDetected = FALSE;
	_numDevices = 0;

	_zPos = 0.0;
	_zPos_C = 0.0;
	_zPos_min = 0.0;
	_zPos_max = 0.0;
	_zVolts2mm = 0.0;
	_zOffsetmm = 0.0;
	_waveformOutPath = L"";
	_z_analog_mode = static_cast<long>(ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT);
	_z_analog_mode_C = _z_analog_mode;
	_z_fast_start_pos = _z_fast_stop_pos = 0;
	_z_fast_volume_time_min = .001;
	_z_fast_volume_time_max = 425000;					//[sec], limit from NI card buffer size LONG_MAX(2147483647)/_piezoSampleRate - _z_fast_flyback_time_max
	_z_fast_volume_time = _z_fast_volume_time_min;

	_z_fast_flyback_time_min = .001;
	_z_fast_flyback_time_max = 4400;					//[sec]
	_z_fast_flyback_time = _z_fast_flyback_time_min;

	_z_fast_flyback_timeAdjustMS_min = -static_cast<double>(Constants::EVENT_WAIT_TIME);	//[-5 sec]
	_z_fast_flyback_timeAdjustMS_max = static_cast<double>(Constants::EVENT_WAIT_TIME);		//[+5 sec]

	_flybackTimeAdjustMS = 0.0;

	_z_fast_volumeOrStep_timeAdjustMS_min = -static_cast<double>(Constants::EVENT_WAIT_TIME);		//[-5 sec]
	_z_fast_volumeOrStep_timeAdjustMS_max = static_cast<double>(Constants::EVENT_WAIT_TIME);		//[+5 sec]
	_volumeTimeAdjustMS = _stepTimeAdjustMS = 0.0;
	_zStepTime = _zIntraStepTime = 0.0;
	_staircaseDelayPercentage = 15;
	_zPockelsPowerBuffer = _zPositionBuffer = NULL;
	_volumePoints = _stepPoints = _flybackPoints = 0;
	_errMsg[0] = NULL;
	_DAQDeviceIndex = DEFAULT_CARD_NUMBER;
	_useWaveformMode = false;

	_boardInfo = BOARD_INFO_STRUCT();
}

wchar_t ThorDAQZ::thordaqLogMessage[MSG_SIZE];
USHORT* ThorDAQZ::_pThorDAQZWaveform = NULL;
long ThorDAQZ::_totalPoints = 0;
std::unique_ptr<WaveformSaverDLL> WaveformSaver(new WaveformSaverDLL(L".\\Modules_Native\\GeometryUtilitiesCPP.dll"));

/// <summary>
/// Finalizes an instance of the <see cref="ThorDAQZ" /> class.
/// </summary>
ThorDAQZ::~ThorDAQZ()
{
	_instanceFlag = false;
}

/// <summary>
/// The _instance flag{CC2D43FA-BBC4-448A-9D0B-7B57ADF2655C}, must initialize after constructor
/// </summary>
bool ThorDAQZ::_instanceFlag = false;
unique_ptr<ThorDAQZ> ThorDAQZ::_single(new ThorDAQZ());

wchar_t message[MAX_PATH];

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorDAQZ *.</returns>
ThorDAQZ* ThorDAQZ::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorDAQZ());
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
long ThorDAQZ::FindDevices(long& deviceCount)
{
	deviceCount = 0;
	//Get filter parameters from hardware setup.xml

	double volts2mm = 0;
	double offsetmm = 0;
	double zPos_min = 0;
	double zPos_max = 0;

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


	auto_ptr<ThorDAQZXML> pSetup(new ThorDAQZXML());
	if (pSetup->GetConversion(volts2mm, offsetmm, zPos_min, zPos_max))
	{
		_zVolts2mm = volts2mm;
		_zOffsetmm = offsetmm;
		_zPos_min = zPos_min;
		_zPos_max = zPos_max;
	}
	else
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetConversion from ThorDAQSettings.XML failed");
		LogMessage(_errMsg, ERROR_EVENT);
		_numDevices = 0;
		deviceCount = 0;

		return FALSE;
	}

	unique_ptr<ThorDAQIOXML> pSetupIO(new ThorDAQIOXML());

	if (!pSetupIO->GetAOLinesConfiguration(_thordaqAOSelection) || _thordaqAOSelection[AO::Z] < 0)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetAOLinesConfiguration from ThorDAQIOSettings.XML failed");
		LogMessage(_errMsg, ERROR_EVENT);
		_numDevices = 0;
		deviceCount = 0;
		return FALSE;
	}
	;
	//pSetup->GetDMA(_activeLoadMS, _preloadCount); //TODO: remove including function

	return TRUE;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="Device">The device.</param>
/// <returns>long.</returns>
long ThorDAQZ::SelectDevice(const long device)
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
long ThorDAQZ::TeardownDevice()
{
	SAFE_DELETE_ARRAY(_pThorDAQZWaveform);
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
long ThorDAQZ::GetParamInfo
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
		paramMin = _zPos_min;
		paramMax = _zPos_max;
		paramDefault = Z_DEFAULT;
	}
	break;
	case PARAM_Z_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = _zPos_min;
		paramMax = _zPos_max;
		paramDefault = Z_DEFAULT;
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
	case PARAM_Z_FAST_START_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _zPos_min;
		paramMax = _zPos_max;
		paramDefault = _zPos_min;
	}
	break;
	case PARAM_Z_FAST_STOP_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _zPos_min;
		paramMax = _zPos_max;
		paramDefault = _zPos_min;
	}
	break;
	case PARAM_Z_FAST_VOLUME_TIME:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _z_fast_volume_time_min;
		paramMax = _z_fast_volume_time_max;
		paramDefault = _z_fast_volume_time_min;
	}
	break;
	case PARAM_Z_FAST_FLYBACK_TIME:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _z_fast_flyback_time_min;
		paramMax = _z_fast_flyback_time_max;
		paramDefault = _z_fast_flyback_time_min;
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
		paramMin = paramMax = paramDefault = PIEZO;
	}
	break;
	case PARAM_DEVICE_TYPE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = paramMax = paramDefault = STAGE_Z | STAGE_Z2;
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
	case PARAM_Z_FAST_FLYBACK_TIME_ADJUST_MS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _z_fast_flyback_timeAdjustMS_min;
		paramMax = _z_fast_flyback_timeAdjustMS_max;
		paramDefault = 0.0;
	}
	break;
	case PARAM_Z_FAST_VOLUME_TIME_ADJUST_MS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _z_fast_volumeOrStep_timeAdjustMS_min;
		paramMax = _z_fast_volumeOrStep_timeAdjustMS_max;
		paramDefault = 0.0;
	}
	break;
	case PARAM_Z_FAST_STEP_TIME_ADJUST_MS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _z_fast_volumeOrStep_timeAdjustMS_min;
		paramMax = _z_fast_volumeOrStep_timeAdjustMS_max;
		paramDefault = 0.0;
	}
	break;
	case PARAM_Z_FAST_STEP_TIME:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0.0;
		paramMax = _z_fast_volume_time_max;
		paramDefault = 0.0;
	}
	break;
	case PARAM_Z_FAST_INTRA_STEP_TIME:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0.0;
		paramMax = _z_fast_volume_time_max;
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
long ThorDAQZ::SetParam(const long paramID, const double param)
{
	long ret = FALSE;

	switch (paramID)
	{
	case PARAM_Z_POS:
	{
		if ((param >= _zPos_min) && (param <= _zPos_max))
		{
			_zPos = static_cast<double>(param);
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_ANALOG_MODE:
	{
		if ((param >= ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT) && (param <= ZPiezoAnalogMode::ANALOG_MODE_LAST))
		{
			int ur = 0;
			_z_analog_mode = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_START_POS:
	{
		if ((param >= _zPos_min) && (param <= _zPos_max))
		{
			_z_fast_start_pos = static_cast<double>(param);
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_STOP_POS:
	{
		if ((param >= _zPos_min) && (param <= _zPos_max))
		{
			_z_fast_stop_pos = static_cast<double>(param);
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_VOLUME_TIME:
	{
		if ((param >= _z_fast_volume_time_min) && (param <= _z_fast_volume_time_max))
		{
			_z_fast_volume_time = static_cast<double>(param);
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_FLYBACK_TIME:
	{
		if ((param >= _z_fast_flyback_time_min) && (param <= _z_fast_flyback_time_max))
		{
			_z_fast_flyback_time = static_cast<double>(param);
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_FLYBACK_TIME_ADJUST_MS:
	{
		if ((_z_fast_flyback_timeAdjustMS_min <= param) && (_z_fast_flyback_timeAdjustMS_max >= param))
		{
			_flybackTimeAdjustMS = param;
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_VOLUME_TIME_ADJUST_MS:
	{
		if ((_z_fast_volumeOrStep_timeAdjustMS_min <= param) && (_z_fast_volumeOrStep_timeAdjustMS_max >= param))
		{
			//_volumeTimeAdjustMS = param;
			_volumeTimeAdjustMS = 0;
			ret = TRUE;
		}
	}
	break;
	case PARAM_Z_FAST_STEP_TIME_ADJUST_MS:
	{
		if ((_z_fast_volumeOrStep_timeAdjustMS_min <= param) && (_z_fast_volumeOrStep_timeAdjustMS_max >= param))
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
long ThorDAQZ::GetParam(const long paramID, double& param)
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
		param = static_cast<long>(_z_analog_mode_C);
	}
	break;
	case PARAM_Z_FAST_START_POS:
	{
		param = static_cast<double>(_z_fast_start_pos_C);
	}
	break;
	case PARAM_Z_FAST_STOP_POS:
	{
		param = static_cast<double>(_z_fast_stop_pos_C);
	}
	break;
	case PARAM_Z_FAST_VOLUME_TIME:
	{
		param = static_cast<double>(_z_fast_volume_time_C);
	}
	break;
	case PARAM_Z_FAST_FLYBACK_TIME:
	{
		param = static_cast<double>(_z_fast_flyback_time_C);
	}
	break;
	case PARAM_Z_STAGE_TYPE:
	{
		param = static_cast<double>(PIEZO);
	}
	break;
	case PARAM_DEVICE_TYPE:
	{
		param = static_cast<double>(STAGE_Z | STAGE_Z2);
	}
	break;
	case PARAM_CONNECTION_STATUS:
	{
		param = (_numDevices > 0) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
	}
	break;

	case PARAM_Z_FAST_FLYBACK_TIME_ADJUST_MS:
	{
		param = static_cast<double>(_flybackTimeAdjustMS);
	}
	break;
	case PARAM_Z_FAST_VOLUME_TIME_ADJUST_MS:
	{
		param = static_cast<double>(_volumeTimeAdjustMS);
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
long ThorDAQZ::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
			_zPositionBufferSize = size;
			SAFE_MEMCPY((void*)(_zPositionBuffer), size * sizeof(double), (void*)(pBuffer));
		}
		catch (...)
		{
			StringCbPrintfW(_errMsg, MAX_PATH, L"Could not allocate Z power buffer");
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
long ThorDAQZ::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorDAQZ::SetParamString(const long paramID, wchar_t* str)
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
long ThorDAQZ::GetParamString(const long paramID, wchar_t* str, long size)
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
long ThorDAQZ::PreflightPosition()
{
	switch (static_cast<ZPiezoAnalogMode>(_z_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_WAVEFORM:
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
		_z_fast_start_pos_C = _z_fast_start_pos;
		_z_fast_stop_pos_C = _z_fast_stop_pos;
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
long ThorDAQZ::SetupPosition()
{
	switch (static_cast<ZPiezoAnalogMode>(_z_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_WAVEFORM:
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
	{
		//place the signal at the first point in the waveform
		//before running the task
		int error = 0;
		int retVal = 0;
		double startV = (_z_fast_start_pos - _zOffsetmm) / _zVolts2mm;
		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _thordaqAOSelection[AO::Z], startV));
		ThordaqErrChk(L"ThorDAQAPISetDACOffsetValue", retVal = ThorDAQAPISetDACOffsetValue(_DAQDeviceIndex, _thordaqAOSelection[AO::Z], startV));

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
long ThorDAQZ::StartPosition()
{
	long ret = FALSE;

	switch (static_cast<ZPiezoAnalogMode>(_z_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT:
	{
		//perform position to volts conversion
		double voltage = (_zPos - _zOffsetmm) / _zVolts2mm;

		int error = 0;
		int retVal = 0;

		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _thordaqAOSelection[AO::Z], voltage));
		ThordaqErrChk(L"ThorDAQAPISetDACOffsetValue", retVal = ThorDAQAPISetDACOffsetValue(_DAQDeviceIndex, _thordaqAOSelection[AO::Z], voltage));
		if (retVal == STATUS_SUCCESSFUL)
		{
			_zPos_C = _zPos;
			ret = TRUE;
		}
		break;
	}
	case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_WAVEFORM:
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
	{
		_useWaveformMode = true;
		ret = TRUE;
		break;
	}
	default:
		break;
	}

	_z_analog_mode_C = _z_analog_mode;
	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorDAQZ::StatusPosition(long& status)
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
long ThorDAQZ::ReadPosition(DeviceType deviceType, double& pos)
{
	long	ret = FALSE;

	if (deviceType & (STAGE_Z | STAGE_Z2))
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
long ThorDAQZ::PostflightPosition()
{
	int error = 0, retVal = 0, written = 0;

	//SAFE_DELETE_ARRAY(_pThorDAQZWaveform);

	_useWaveformMode = false;

	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="errMsg">The error MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorDAQZ::GetLastErrorMsg(wchar_t* msg, long size)
{
	wcsncpy_s(msg, size, _errMsg, MAX_PATH);
	return TRUE;
}


void ThorDAQZ::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	qlogDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

/// <summary>
/// Builds Z piezo waveform for ANALOG_MODE_SINGLE_WAVEFORM.
/// </summary>
double* ThorDAQZ::BuildSingleWaveform()
{
	//build z piezo output
	double* pWaveform = (double*)malloc(_totalPoints * sizeof(double));
	if (NULL == pWaveform)
		return NULL;

	memset(pWaveform, 0x0, _totalPoints * sizeof(double));

	double startV = (_z_fast_start_pos - _zOffsetmm) / _zVolts2mm;
	double stopV = (_z_fast_stop_pos - _zOffsetmm) / _zVolts2mm;

	double rangeV = abs(stopV - startV);

	double zStep = (stopV - startV) / _volumePoints;

	//shift the data to the apppriate start location
	double offset = startV + rangeV / 2;
	double flip = 1.0;
	if (startV > stopV)
	{
		flip = -1.0;
		offset = startV - rangeV / 2;
	}

	long i;
	for (i = 0; i < _volumePoints; i++)
	{
		pWaveform[i] = max(MIN_AO_VOLTAGE, min(offset + flip * (rangeV / _volumePoints * i - rangeV / 2), MAX_AO_VOLTAGE));
	}

	for (; i < _totalPoints; i++)
	{
		pWaveform[i] = max(MIN_AO_VOLTAGE, min(offset + flip * (rangeV / 2.0 * cos(PI / (double)_flybackPoints * ((double)i - _volumePoints))), MAX_AO_VOLTAGE));
	}

	return pWaveform;
}

/// <summary>
/// Builds Z piezo waveform for ANALOG_MODE_STAIRCASE_WAVEFORM.
/// </summary>
double* ThorDAQZ::BuildStaircaseWaveform()
{
	double startV = 0, stopV = 0, rangeV = 0, offset = 0, flip = 0;
	long piezoDelayPoints = static_cast<long>(_intraStepPoints * _staircaseDelayPercentage / 100.0);

	//build z piezo output
	double* pWaveform = new double[_totalPoints];
	if (NULL == pWaveform)
		return NULL;

	memset(pWaveform, 0x0, _totalPoints * sizeof(double));

	//build z volume of multiple steps
	long k = 0, j = 0;
	for (long i = 0; i < _zPositionBufferSize; i++)
	{
		double posV = (_zPositionBuffer[i] - _zOffsetmm) / _zVolts2mm;
		//build step
		for (j = 0; j < _stepPoints; j++)
		{
			pWaveform[k++] = max(MIN_AO_VOLTAGE, min(posV, MAX_AO_VOLTAGE));
		}
		//build intra step, except the last step
		double lastStepV = pWaveform[k - 1];
		if (_zPositionBufferSize - 1 > i)
		{
			startV = (_zPositionBuffer[i] - _zOffsetmm) / _zVolts2mm;
			stopV = (_zPositionBuffer[i + 1] - _zOffsetmm) / _zVolts2mm;
			rangeV = abs(stopV - startV);
			offset = (startV > stopV) ? (startV - rangeV / 2) : (startV + rangeV / 2);
			flip = (startV > stopV) ? -1.0 : 1.0;
			long transitionPoints = _intraStepPoints - piezoDelayPoints;
			for (j = 0; j < transitionPoints; j++)
			{
				pWaveform[k++] = max(MIN_AO_VOLTAGE, min(offset + flip * (rangeV / 2.0 * sin(PI / (double)transitionPoints * j - PI / 2)), MAX_AO_VOLTAGE));
			}
			// Stay at the stop Voltage level and wait for the piezo to reach the position
			for (j = 0; j < piezoDelayPoints; j++)
			{
				pWaveform[k++] = max(MIN_AO_VOLTAGE, min(stopV, MAX_AO_VOLTAGE));
			}
		}
	}
	//build flyback
	startV = (_zPositionBuffer[0] - _zOffsetmm) / _zVolts2mm;
	stopV = (_zPositionBuffer[_zPositionBufferSize - 1] - _zOffsetmm) / _zVolts2mm;
	rangeV = abs(stopV - startV);
	offset = (startV > stopV) ? (startV - rangeV / 2) : (startV + rangeV / 2);
	flip = (startV > stopV) ? -1.0 : 1.0;
	long flybackPoints = _flybackPoints - piezoDelayPoints;
	long flybackSinePoints = _totalPoints - piezoDelayPoints;
	for (j = k; j < flybackSinePoints; j++)
	{
		pWaveform[k++] = max(MIN_AO_VOLTAGE, min(offset + flip * (rangeV / 2.0 * cos(PI / (double)flybackPoints * ((double)j - _volumePoints))), MAX_AO_VOLTAGE));
	}
	// Stay at the start Voltage level (initial position) and wait for the piezo to reach the position
	for (j = k; j < _totalPoints; j++)
	{
		pWaveform[k++] = max(MIN_AO_VOLTAGE, min(startV, MAX_AO_VOLTAGE));
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
long ThorDAQZ::BuildAndGetFastZWaveform(const double updateRate, const long samplesFrame, const long samplesFlyback, const double frameRate, ThorDAQZWaveformParams* waveformParams)
{
	if (nullptr == waveformParams || FALSE == _useWaveformMode)
	{
		return FALSE;
	}
	//build piezo waveform based on analog modes
	double* zWaveform = NULL;

	switch (static_cast<ZPiezoAnalogMode>(_z_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_WAVEFORM:
	{
		//create a waveform that covers the forward sweep and flyback for the sawtooth
		long volumeFrames = static_cast<long>(round((_z_fast_volume_time + _volumeTimeAdjustMS / Constants::MS_TO_SEC) * frameRate));
		_volumePoints = static_cast<long>(volumeFrames * (updateRate * (_volumeTimeAdjustMS / Constants::MS_TO_SEC) + (samplesFrame + samplesFlyback)));
		long flybackFrames = static_cast<long>(round((_z_fast_flyback_time + _flybackTimeAdjustMS / Constants::MS_TO_SEC) * frameRate));

		_flybackPoints = static_cast<long>(flybackFrames * (updateRate * (_flybackTimeAdjustMS / Constants::MS_TO_SEC) + (samplesFrame + samplesFlyback)));

		_totalPoints = _volumePoints + _flybackPoints;

		zWaveform = BuildSingleWaveform();
	}
	break;
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
	{
		//user defined positions
		if ((0 >= _zPositionBufferSize) || (NULL == _zPositionBuffer))
			return FALSE;

		_stepPoints = static_cast<long>(round(samplesFrame + updateRate * ((_stepTimeAdjustMS / Constants::MS_TO_SEC))));	//points per step TODO: what is _stepTimeAdjustMS used for?
		_intraStepPoints = samplesFlyback;											//points per intra step

		long flybackFrames = static_cast<long>(round((_z_fast_flyback_time + _flybackTimeAdjustMS / Constants::MS_TO_SEC) * frameRate));
		_flybackPoints = static_cast<long>(flybackFrames * (updateRate * (_flybackTimeAdjustMS / Constants::MS_TO_SEC) + (samplesFrame + samplesFlyback)));
		_volumePoints = _stepPoints * _zPositionBufferSize + _intraStepPoints * (_zPositionBufferSize -1);
		_totalPoints = _volumePoints + _flybackPoints;
		if (0 >= _totalPoints)
		{
			StringCbPrintfW(_errMsg, MAX_PATH, L"Fast Z waveform size is too large.");
			LogMessage(_errMsg, VERBOSE_EVENT);
			return FALSE;
		}
		zWaveform = BuildStaircaseWaveform();
	}
	break;
	}

	//return if waveform building failed
	if (NULL == zWaveform)
		return FALSE;

	//save waveform if requested, once only
	if (0 < _waveformOutPath.length() && TRUE == WaveformSaver.get()->SaveData(_waveformOutPath, SignalType::ANALOG_Z, zWaveform, _totalPoints))
	{
		_waveformOutPath = L"";
	}

	SAFE_DELETE_ARRAY(_pThorDAQZWaveform);
	_pThorDAQZWaveform = new USHORT[_totalPoints + DAC_FIFO_DEPTH]();

	if (_totalPoints > 0)
	{
		SHORT offset = (SHORT)round(zWaveform[0] / DAC_RESOLUTION) + 0x8000;
		for (int i = 0; i < _totalPoints; ++i)
		{
			_pThorDAQZWaveform[i] = (USHORT)(round(zWaveform[i] / DAC_RESOLUTION) + 0x8000 - offset);
		}
	}

	SAFE_DELETE_ARRAY(zWaveform);

	long framesPerVolume = static_cast<long>(round(frameRate * (_totalPoints / updateRate)));

	double startV = (_z_fast_start_pos - _zOffsetmm) / _zVolts2mm;
	double park = startV;

	waveformParams->samplesPerFrame = samplesFrame + samplesFlyback;
	waveformParams->framesPerWaveform = framesPerVolume;
	waveformParams->updateRate = updateRate;
	waveformParams->waveformChannel = _thordaqAOSelection[AO::Z];
	waveformParams->waveform = _pThorDAQZWaveform;
	waveformParams->waveformLength = _totalPoints;
	waveformParams->parkPosition = park;
	waveformParams->offsetPosition = park;


	return TRUE;
}