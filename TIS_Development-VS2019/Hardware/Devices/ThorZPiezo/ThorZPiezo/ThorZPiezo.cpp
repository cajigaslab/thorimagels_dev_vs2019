// ThorZPiezo.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "Strsafe.h"
#include "ThorZPiezo.h"
#include "ZPiezoSetupXML.h"

/// <summary>
/// Prevents a default instance of the <see cref="ThorZPiezo" /> class from being created.
/// </summary>
ThorZPiezo::ThorZPiezo()
{
	_deviceDetected = FALSE;
	_numDevices = 0;
	_devName = "/Dev1/";
	_analogLine = "ao0";
	_triggerLine = "PFI0";
	_waveformOutPath = L"";

	_zPos = 0;
	_zPos_C = 0;
	_zPos_min = 0;
	_zPos_max = 0;

	_z_fast_start_pos = _z_fast_stop_pos = 0;
	_piezoSampleRate = 5000;							//default 5KHz (single waveform), 50KHz (staircase waveform)
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

	_z_analog_mode = static_cast<long>(ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT);

	_taskHandleAO0 = NULL;

	_zPockelsPowerBufferSize = _zPositionBufferSize = 0;
	_outputPockelsReference = FALSE;
	_outputPockelsResponseType = PockelsResponseType::SINE_RESPONSE;
	_referenceWaveformEnable = FALSE;
	_activeLoadMS = 100;
	_preLoadCount = 8;
	_staircaseDelayPercentage = 15;
	_zPockelsPowerBuffer = _zPositionBuffer = NULL;
	_piezoVolumePoints = _piezoStepPoints = _piezoFlybackPoints = 0;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorZPiezo" /> class.
/// </summary>
ThorZPiezo::~ThorZPiezo()
{
	_instanceFlag = false;
}

/// <summary>
/// The _instance flag{CC2D43FA-BBC4-448A-9D0B-7B57ADF2655C}, must initialize after constructor
/// </summary>
bool ThorZPiezo:: _instanceFlag = false;
auto_ptr<ThorZPiezo> ThorZPiezo::_single(new ThorZPiezo());
TaskHandle ThorZPiezo::_taskHandleAO0 = NULL;
long ThorZPiezo::_totalPoints = 0;
long ThorZPiezo::_callbackPoints = 0;
long ThorZPiezo::_index = 0;
long ThorZPiezo::_outputLineCount = 1;
float64* ThorZPiezo::_pWaveform = NULL;
wchar_t ThorZPiezo::_errMsg[MAX_PATH] = {NULL};
wchar_t message[MAX_PATH];
std::unique_ptr<WaveformSaverDLL> WaveformSaver(new WaveformSaverDLL(L".\\Modules_Native\\GeometryUtilitiesCPP.dll"));

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorZPiezo *.</returns>
ThorZPiezo *ThorZPiezo::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorZPiezo());
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
long ThorZPiezo::FindDevices(long &deviceCount)
{
	long	ret = FALSE;

	deviceCount=0;
	//Get filter parameters from hardware setup.xml
	auto_ptr<ZPiezoXML> pSetup(new ZPiezoXML());

	double volts2mm=0;
	double offsetmm=0;
	double zPos_min=0;
	double zPos_max=0;
	double pockelsRefThreshold=0;
	long staircaseDelayPercentage=0;

	if (ret=pSetup->OpenConfigFile())
	{
		string piezoLine;
		string analogLine;
		string triggerLine;
		string pockelsReferenceAnalogLine;
		if(pSetup->GetIO(piezoLine, analogLine, triggerLine, pockelsReferenceAnalogLine))
		{
			if(piezoLine.length() > 0)
			{
				char devName[256];
				DAQmxGetSysDevNames(devName, 256);
				std::string temp = devName;

				// trim the string for leading and trailng slashes '/'
				if(piezoLine.find('/') == 0)
					piezoLine.erase(0, 1);

				if(piezoLine.rfind('/') == piezoLine.length()-1)
					piezoLine.erase(piezoLine.length()-1, 1);

				// check if specified dev name matches real device names read from NI card
				if(string::npos != temp.find(piezoLine))
				{
					deviceCount=1;
					_numDevices=1;
					_devName = '/' + piezoLine + '/';
					_analogLine = analogLine;
					_triggerLine = triggerLine;
					_pockelsReferenceAnalogLine = pockelsReferenceAnalogLine;
					ret = TRUE;
				}

				if (pSetup->GetConversion(volts2mm,offsetmm, zPos_min,zPos_max, staircaseDelayPercentage, pockelsRefThreshold))
				{
					_volts2mm=volts2mm;
					_offsetmm=offsetmm;
					_zPos_min=zPos_min;
					_zPos_max=zPos_max;
					_staircaseDelayPercentage=staircaseDelayPercentage;
					_pockelsRefThreshold = pockelsRefThreshold;
					ret = TRUE;
				}
			}
			else
			{
				ret = FALSE;
			}
		}
		else
		{
			ret = FALSE;
		}

		pSetup->GetDMA(_activeLoadMS, _preLoadCount);
	}
	return ret;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="Device">The device.</param>
/// <returns>long.</returns>
long ThorZPiezo::SelectDevice(const long device)
{
	if((device < 0) || (device >= _numDevices))
	{
		return FALSE;
	}

	return TRUE;
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorZPiezo::TeardownDevice()
{
	// Move Piezo to 0 volts before closing ThorImage
	if (0 == SetAO0(0))
	{
		_zPos_C = _offsetmm;
	}

	CloseNITasks();
	SAFE_DELETE_MEMORY (_pWaveform);
	SAFE_DELETE_ARRAY (_zPockelsPowerBuffer);
	SAFE_DELETE_ARRAY (_zPositionBuffer);
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
long ThorZPiezo::GetParamInfo
	(
	const long	paramID,
	long		&paramType,
	long		&paramAvailable,
	long		&paramReadOnly,
	double		&paramMin,
	double		&paramMax,
	double		&paramDefault
	)
{
	long ret = TRUE;

	switch(paramID)
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
	case PARAM_Z_OUTPUT_POCKELS_REFERENCE:
		{
			paramType = TYPE_BOOL;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
		}
		break;
	case PARAM_Z_OUTPUT_POCKELS_RESPONSE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PockelsResponseType::SINE_RESPONSE;
			paramMax = PockelsResponseType::LAST_POCKELS_RESPONSE;
			paramDefault = PockelsResponseType::SINE_RESPONSE;
		}
		break;
	case PARAM_POWER_RAMP_BUFFER:
		{
			paramType = TYPE_BUFFER;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
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
			paramType		= TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= FALSE;
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
long ThorZPiezo::SetParam(const long paramID, const double param)
{
	long ret = FALSE;

	switch(paramID)
	{
	case PARAM_Z_POS:
		{
			if((param >= _zPos_min) && (param <= _zPos_max))
			{
				_zPos = static_cast<double>(param);
				ret = TRUE;
			}
		}
		break;
	case PARAM_Z_ANALOG_MODE:
		{
			if((param >= ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT) && (param <= ZPiezoAnalogMode::ANALOG_MODE_LAST))
			{
				_z_analog_mode = static_cast<long>(param);
				switch (static_cast<ZPiezoAnalogMode>(_z_analog_mode))
				{
				case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
					_z_fast_flyback_time_min = 0;
					_piezoSampleRate = 50000;		//50KHz
					break;
				default:
					_z_fast_flyback_time_min = .001;
					_piezoSampleRate = 5000;		//5KHz
					break;
				}
				ret = TRUE;
			}
		}
		break;
	case PARAM_Z_FAST_START_POS:
		{
			if((param >= _zPos_min) && (param <= _zPos_max))
			{
				_z_fast_start_pos = static_cast<double>(param);
				ret = TRUE;
			}
		}
		break;
	case PARAM_Z_FAST_STOP_POS:
		{
			if((param >= _zPos_min) && (param <= _zPos_max))
			{
				_z_fast_stop_pos = static_cast<double>(param);
				ret = TRUE;
			}
		}
		break;
	case PARAM_Z_FAST_VOLUME_TIME:
		{
			if((param >= _z_fast_volume_time_min) && (param <= _z_fast_volume_time_max))
			{
				_z_fast_volume_time = static_cast<double>(param);
				ret = TRUE;
			}
		}
		break;
	case PARAM_Z_FAST_FLYBACK_TIME:
		{
			if((param >= _z_fast_flyback_time_min) && (param <= _z_fast_flyback_time_max))
			{
				_z_fast_flyback_time = static_cast<double>(param);
				ret = TRUE;
			}
		}
		break;
	case PARAM_Z_OUTPUT_POCKELS_REFERENCE:
		{
			if((param == FALSE) || (param == TRUE))
			{
				_outputPockelsReference = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case PARAM_Z_OUTPUT_POCKELS_RESPONSE_TYPE:
		{
			if((PockelsResponseType::SINE_RESPONSE <= param) || (PockelsResponseType::LAST_POCKELS_RESPONSE >= param))
			{
				_outputPockelsResponseType = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case PARAM_Z_FAST_FLYBACK_TIME_ADJUST_MS:
		{
			if((_z_fast_flyback_timeAdjustMS_min <= param) && (_z_fast_flyback_timeAdjustMS_max >= param))
			{
				_flybackTimeAdjustMS = param;
				ret = TRUE;
			}
		}
		break;
	case PARAM_Z_FAST_VOLUME_TIME_ADJUST_MS:
		{
			if((_z_fast_volumeOrStep_timeAdjustMS_min <= param) && (_z_fast_volumeOrStep_timeAdjustMS_max >= param))
			{
				_volumeTimeAdjustMS = param;
				ret = TRUE;
			}
		}
		break;
	case PARAM_Z_FAST_STEP_TIME_ADJUST_MS:
		{
			if((_z_fast_volumeOrStep_timeAdjustMS_min <= param) && (_z_fast_volumeOrStep_timeAdjustMS_max >= param))
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
	case PARAM_Z_POCKELS_MIN:
		{			
			_pockelsMin = param;
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
long ThorZPiezo::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch(paramID)
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
	case PARAM_Z_OUTPUT_POCKELS_REFERENCE:
		{
			param = static_cast<double>(_outputPockelsReference);
		}
		break;
	case PARAM_Z_OUTPUT_POCKELS_RESPONSE_TYPE:
		{
			param = static_cast<double>(_outputPockelsResponseType);
		}
		break;
	case PARAM_Z_STAGE_TYPE:
		{
			param = static_cast<double>(PIEZO);
		}
		break;	
	case PARAM_DEVICE_TYPE:
		{
			param =static_cast<double>(STAGE_Z | STAGE_Z2);
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
long ThorZPiezo::SetParamBuffer(const long paramID, char * pBuffer, long size)
{
	long ret = TRUE;
	switch (paramID)
	{
	case IDevice::PARAM_POWER_RAMP_BUFFER:
		{
			SAFE_DELETE_ARRAY(_zPockelsPowerBuffer);

			try
			{
				_zPockelsPowerBuffer = new double[size];
				_zPockelsPowerBufferSize = size;
				SAFE_MEMCPY((void*)(_zPockelsPowerBuffer),size*sizeof(double),(void*)(pBuffer));
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MAX_PATH,L"Could not allocate Z power buffer");
				LogMessage(_errMsg,VERBOSE_EVENT);
			}
		}
		break;
	case IDevice::PARAM_Z_FAST_STEP_BUFFER:
		{
			SAFE_DELETE_ARRAY(_zPositionBuffer);

			try
			{
				_zPositionBuffer = new double[size];
				_zPositionBufferSize = size;
				SAFE_MEMCPY((void*)(_zPositionBuffer),size*sizeof(double),(void*)(pBuffer));
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MAX_PATH,L"Could not allocate Z power buffer");
				LogMessage(_errMsg,VERBOSE_EVENT);
			}
		}
		break;
	default:
		{
			ret = FALSE;
			StringCbPrintfW(_errMsg,MAX_PATH,L"Parameter (%d) not implemented",paramID);
			LogMessage(_errMsg,VERBOSE_EVENT);
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
long ThorZPiezo::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorZPiezo::SetParamString(const long paramID, wchar_t* str)
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
long ThorZPiezo::GetParamString(const long paramID, wchar_t* str, long size)
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
long ThorZPiezo::PreflightPosition()
{
	switch (static_cast<ZPiezoAnalogMode>(_z_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_WAVEFORM:
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
		//waveform modes
		if(FALSE == BuildWaveforms())
			return FALSE;

		_z_fast_start_pos_C=_z_fast_start_pos;
		_z_fast_stop_pos_C=_z_fast_stop_pos;
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
long ThorZPiezo::SetupPosition()
{
	try
	{
		switch (static_cast<ZPiezoAnalogMode>(_z_analog_mode))
		{
		case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_WAVEFORM:
		case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
			//place the signal at the first point in the waveform
			//before running the task
			SetAO0(_pWaveform[0]);

			//move to the start position
			Sleep(100);
			break;
		default:
			break;
		}
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorZPiezo::StartPosition()
{
	long ret = FALSE;

	switch (static_cast<ZPiezoAnalogMode>(_z_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT:
		double voltage;

		//perform position to volts conversion
		voltage=(_zPos-_offsetmm)/_volts2mm;
		if (0==SetAO0(voltage))
		{
			_zPos_C=_zPos;
			ret = TRUE;
		}
		break;
	case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_WAVEFORM:
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
		ret = LoadWaveformsAndArmDAQ();
		break;
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
long ThorZPiezo::StatusPosition(long &status)
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
long ThorZPiezo::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	if(deviceType & (STAGE_Z | STAGE_Z2))
	{
		pos=_zPos_C;
		ret = TRUE;
	}
	return ret;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorZPiezo::PostflightPosition()
{
	CloseNITasks();
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="errMsg">The error MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorZPiezo::GetLastErrorMsg(wchar_t * msg, long size)
{	
	wcsncpy_s(msg,size,_errMsg,MAX_PATH);
	return TRUE;
}

/// <summary>
/// Builds the reference waveform for pockels, must release memory afterward.
/// </summary>
double* ThorZPiezo::BuildPockelsReferenceWaveform()
{
	//return if position and power are not equal length
	if((_zPositionBufferSize != _zPockelsPowerBufferSize) || (0 >= _zPockelsPowerBufferSize))
		return NULL;

	double* pRefWaveform = (float64*)malloc(_totalPoints * sizeof(float64));
	if(NULL == pRefWaveform)
		return NULL;

	memset(pRefWaveform, 0x0, _totalPoints * sizeof(float64));

	//linearize the response of the pockels cell
	double* voltageOutBuffer = new double[_zPockelsPowerBufferSize];
	for (long j = 0; j < _zPockelsPowerBufferSize; j++)
	{
		switch (_outputPockelsResponseType)
		{
		case static_cast<long>(PockelsResponseType::SINE_RESPONSE):
			voltageOutBuffer[j] = acos(1 - static_cast<double>(Constants::AREA_UNDER_CURVE) * _zPockelsPowerBuffer[j]/100.0)/PI * MAX_AO_VOLTAGE;
			break;
		case static_cast<long>(PockelsResponseType::LINEAR_RESPONSE):
			voltageOutBuffer[j] = _zPockelsPowerBuffer[j]/100.0 * MAX_AO_VOLTAGE;
			break;
		default:
			voltageOutBuffer[j] = 0;
			break;
		}
	}


	//build pockels waveform based on analog modes
	long i = 0;
	//Check if the first point is below the Threshold before assigning it as the lowestPower
	if((voltageOutBuffer[0]/MAX_AO_VOLTAGE*_pockelsMin) < _pockelsRefThreshold && 0 != _pockelsMin && 0 != _pockelsRefThreshold)
	{
		voltageOutBuffer[0] = _pockelsRefThreshold / _pockelsMin * MAX_AO_VOLTAGE;
	}
	double lowestPower = voltageOutBuffer[0];
	switch (static_cast<ZPiezoAnalogMode>(_z_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_WAVEFORM:
		{
			const long nStepsPerImage = static_cast<long>((double)_piezoVolumePoints / (double)(_zPockelsPowerBufferSize -1));
			for (long k = 0; k < _zPockelsPowerBufferSize - 1; k++)
			{
				//If for any reason _pockelsMin is less than 0 and the threshold is 0 don't apply it because it most likely means the threshold was never changed from default
				if((voltageOutBuffer[k]/MAX_AO_VOLTAGE*_pockelsMin) < _pockelsRefThreshold && 0 != _pockelsMin && 0 != _pockelsRefThreshold)
				{
					voltageOutBuffer[k] = _pockelsRefThreshold / _pockelsMin * MAX_AO_VOLTAGE;
				}
				if((voltageOutBuffer[k + 1]/MAX_AO_VOLTAGE*_pockelsMin) < _pockelsRefThreshold && 0 != _pockelsMin && 0 != _pockelsRefThreshold)
				{
					voltageOutBuffer[k + 1] = _pockelsRefThreshold / _pockelsMin * MAX_AO_VOLTAGE;
				}
				double powerStep = (voltageOutBuffer[k + 1] - voltageOutBuffer[k])/(double)nStepsPerImage;
				double power = voltageOutBuffer[k];			
				for (long j = 0; j < nStepsPerImage; j++)
				{
					pRefWaveform[i++]= max(MIN_AO_VOLTAGE, min(power,MAX_AO_VOLTAGE));
					power += powerStep;
					if (lowestPower > power)
					{
						lowestPower = power;
					}
				}
			}
			//match the power waveform to the piezo _pWaveform
			//hold the last power until the end of the last image before the flyback
			double lastPower = pRefWaveform[i-1];
			for(;i <= _piezoVolumePoints; i++)
			{
				pRefWaveform[i] = max(MIN_AO_VOLTAGE, min(lastPower,MAX_AO_VOLTAGE));
			}
		}
		break;
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
		{
			//locate the minimum power
			for (long k = 0; k < _zPockelsPowerBufferSize; k++)
			{
				//If for any reason _pockelsMin is less than 0 and the threshold is 0 don't apply it because it most likely means the threshold was never changed from default
				if((voltageOutBuffer[k]/MAX_AO_VOLTAGE*_pockelsMin) < _pockelsRefThreshold && 0 != _pockelsMin && 0 != _pockelsRefThreshold)
				{
					voltageOutBuffer[k] = _pockelsRefThreshold / _pockelsMin * MAX_AO_VOLTAGE;
				}
				if(lowestPower > voltageOutBuffer[k])
				{
					lowestPower = voltageOutBuffer[k];
				}
			}
			for (long k = 0; k < _zPockelsPowerBufferSize; k++)
			{
				double power = voltageOutBuffer[k];	
				//build step power
				for (long j = 0; j < _piezoStepPoints; j++)
				{
					pRefWaveform[i++]= max(MIN_AO_VOLTAGE, min(power,MAX_AO_VOLTAGE));
				}
				//build intra step power, except the last step
				if(_zPockelsPowerBufferSize - 1 > k)
				{					
					for (long j = 0; j < _piezoIntraStepPoints; j++)
					{
						pRefWaveform[i++] = max(MIN_AO_VOLTAGE, min(lowestPower,MAX_AO_VOLTAGE));
					}
				}
			}
		}
		break;
	}

	//for the flyback, set the power to the lowest of the waveform
	//to avoid any sample bleaching or PMT tripping
	for(;i < _totalPoints; i++)
	{
		pRefWaveform[i] = max(MIN_AO_VOLTAGE, min(lowestPower,MAX_AO_VOLTAGE));
	}

	SAFE_DELETE_ARRAY(voltageOutBuffer);
	return pRefWaveform;
}

/// <summary>
/// Builds Z piezo waveform for ANALOG_MODE_SINGLE_WAVEFORM.
/// </summary>
double* ThorZPiezo::BuildSingleWaveform()
{
	//build z piezo output
	double* pWaveform = (float64*)malloc(_totalPoints * sizeof(float64));
	if(NULL == pWaveform)
		return NULL;

	memset(pWaveform, 0x0, _totalPoints * sizeof(float64));

	double startV = (_z_fast_start_pos-_offsetmm)/_volts2mm;
	double stopV = (_z_fast_stop_pos - _offsetmm)/_volts2mm;

	double rangeV = abs(stopV - startV);

	double zStep = (stopV - startV)/_piezoVolumePoints;

	//shift the data to the apppriate start location
	double offset = startV + rangeV/2;
	double flip = 1.0;
	if(startV > stopV)
	{
		flip =  -1.0;
		offset = startV - rangeV/2;
	}

	long i;
	for(i = 0; i<_piezoVolumePoints; i++)
	{
		pWaveform[i] = max(MIN_AO_VOLTAGE, min(offset + flip * (rangeV/_piezoVolumePoints * i - rangeV/2),MAX_AO_VOLTAGE));
	}

	for(; i<_totalPoints; i++)
	{
		pWaveform[i] = max(MIN_AO_VOLTAGE, min(offset + flip * (rangeV/2.0 * cos (PI / (double)_piezoFlybackPoints * ((double)i-_piezoVolumePoints))),MAX_AO_VOLTAGE));
	}

	return pWaveform;
}

/// <summary>
/// Builds Z piezo waveform for ANALOG_MODE_STAIRCASE_WAVEFORM.
/// </summary>
double* ThorZPiezo::BuildStaircaseWaveform()
{
	double startV = 0, stopV = 0, rangeV = 0, offset = 0,flip = 0;
	long piezoDelayPoints = static_cast<long>(_piezoIntraStepPoints * _staircaseDelayPercentage / 100.0);

	//build z piezo output
	double* pWaveform = (float64*)malloc(_totalPoints * sizeof(float64));
	if(NULL == pWaveform)
		return NULL;

	memset(pWaveform, 0x0, _totalPoints * sizeof(float64));

	//build z volume of multiple steps
	long k = 0, j = 0;
	for (long i = 0; i < _zPositionBufferSize; i++)
	{
		double posV = (_zPositionBuffer[i]-_offsetmm)/_volts2mm;
		//build step
		for (j = 0; j < _piezoStepPoints; j++)
		{
			pWaveform[k++] = max(MIN_AO_VOLTAGE, min(posV, MAX_AO_VOLTAGE));
		}
		//build intra step, except the last step
		double lastStepV = pWaveform[k-1];
		if(_zPositionBufferSize - 1 > i)
		{
			startV = (_zPositionBuffer[i]-_offsetmm)/_volts2mm;
			stopV = (_zPositionBuffer[i+1]-_offsetmm)/_volts2mm;
			rangeV = abs(stopV - startV);
			offset = (startV > stopV) ? (startV - rangeV/2) : (startV + rangeV/2);
			flip = (startV > stopV) ? -1.0 : 1.0;
			long transitionPoints = _piezoIntraStepPoints - piezoDelayPoints;
			for (j = 0; j < transitionPoints; j++)
			{
				pWaveform[k++] = max(MIN_AO_VOLTAGE, min(offset + flip * (rangeV/2.0 * sin (PI / (double)transitionPoints*j - PI/2)),MAX_AO_VOLTAGE));
			}
			// Stay at the stop Voltage level and wait for the piezo to reach the position
			for (j = 0; j < piezoDelayPoints; j++)
			{
				pWaveform[k++] = max(MIN_AO_VOLTAGE, min(stopV, MAX_AO_VOLTAGE));
			}
		}
	}
	//build flyback
	startV = (_zPositionBuffer[0]-_offsetmm)/_volts2mm;
	stopV = (_zPositionBuffer[_zPositionBufferSize-1]-_offsetmm)/_volts2mm;
	rangeV = abs(stopV - startV);
	offset = (startV > stopV) ? (startV - rangeV/2) : (startV + rangeV/2);
	flip = (startV > stopV) ? -1.0 : 1.0;
	long flybackPoints = _piezoFlybackPoints - piezoDelayPoints;
	long flybackSinePoints = _totalPoints - piezoDelayPoints;
	for(j = k; j < flybackSinePoints; j++)
	{
		pWaveform[k++] = max(MIN_AO_VOLTAGE, min(offset + flip * (rangeV/2.0 * cos (PI / (double)flybackPoints * ((double)j-_piezoVolumePoints))),MAX_AO_VOLTAGE));
	}
	// Stay at the start Voltage level (initial position) and wait for the piezo to reach the position
	for (j = k; j < _totalPoints; j++)
	{
		pWaveform[k++] = max(MIN_AO_VOLTAGE, min(startV, MAX_AO_VOLTAGE));
	}
	return pWaveform;
}

/// <summary>
/// Builds the waveform that will control the piezo.
/// </summary>
long ThorZPiezo::BuildWaveforms()
{
	if((TRUE == _outputPockelsReference) && (_pockelsReferenceAnalogLine.length() > 0) &&
		(_zPockelsPowerBufferSize > 0) && (NULL != _zPockelsPowerBuffer))
	{
		_referenceWaveformEnable = TRUE;
		_outputLineCount = 2;
	}
	else
	{
		_referenceWaveformEnable = FALSE;
		_outputLineCount = 1;
	}

	//active load
	_callbackPoints = static_cast<long>(_piezoSampleRate * _activeLoadMS / Constants::MS_TO_SEC);

	//build piezo waveform based on analog modes
	double* zWaveform = NULL;
	switch (static_cast<ZPiezoAnalogMode>(_z_analog_mode))
	{
	case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_WAVEFORM:
		{
			//create a waveform that covers the forward sweep and flyback for the sawtooth
			_piezoVolumePoints = static_cast<long>(_piezoSampleRate * (_z_fast_volume_time + _volumeTimeAdjustMS / Constants::MS_TO_SEC));
			_piezoFlybackPoints = static_cast<long>(_piezoSampleRate * (_z_fast_flyback_time + _flybackTimeAdjustMS / Constants::MS_TO_SEC));
			_totalPoints = _piezoVolumePoints + _piezoFlybackPoints;
			zWaveform = BuildSingleWaveform();
		}
		break;
	case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
		{
			//user defined positions
			if((0 >= _zPositionBufferSize) || (NULL == _zPositionBuffer))
				return FALSE;

			_piezoStepPoints = static_cast<long>(_piezoSampleRate * (_zStepTime + (_stepTimeAdjustMS / Constants::MS_TO_SEC)));	//points per step
			_piezoIntraStepPoints = static_cast<long>(_piezoSampleRate * _zIntraStepTime);											//points per intra step
			_piezoFlybackPoints = static_cast<long>(_piezoSampleRate * (_zIntraStepTime + _z_fast_flyback_time + _flybackTimeAdjustMS / Constants::MS_TO_SEC));
			_piezoVolumePoints = _piezoStepPoints*_zPositionBufferSize + _piezoIntraStepPoints*(_zPositionBufferSize - 1);
			_totalPoints = _piezoVolumePoints + _piezoFlybackPoints;
			//NI cannot accept overflow of data length, not necessary for ANALOG_MODE_SINGLE_WAVEFORM
			//since limit of _z_fast_volume_time_max is applied:
			if(0 >= _totalPoints)
			{
				StringCbPrintfW(_errMsg,MAX_PATH,L"Fast Z waveform size is too large.");
				LogMessage(_errMsg,VERBOSE_EVENT);
				return FALSE;
			}
			zWaveform = BuildStaircaseWaveform();
		}
		break;
	}

	//return if waveform building failed
	if(NULL == zWaveform)
		return FALSE;

	//save waveform if requested, once only
	if (0 <_waveformOutPath.length() && TRUE == WaveformSaver.get()->SaveData(_waveformOutPath, SignalType::ANALOG_Z, zWaveform, _totalPoints))
		_waveformOutPath = L"";

	_pWaveform = (float64*)realloc(_pWaveform, _outputLineCount * _totalPoints * sizeof(float64));

	//interleave if pockels reference is also built
	switch (_outputLineCount)
	{
	case 1:
		{
			SAFE_MEMCPY((void*)(_pWaveform), _outputLineCount * _totalPoints * sizeof(float64), (void*)(zWaveform));
		}
		break;
	case 2:
		{
			double* pPockelsRefWaveform = BuildPockelsReferenceWaveform();
			if(NULL == pPockelsRefWaveform)
			{
				SAFE_DELETE_MEMORY(zWaveform);
				return FALSE;
			}
			for (long i = 0; i < _totalPoints; i++)
			{
				_pWaveform[2*i] = zWaveform[i];
				_pWaveform[2*i+1] = pPockelsRefWaveform[i];
			}
			SAFE_DELETE_MEMORY(pPockelsRefWaveform);
		}
		break;
	}

	SAFE_DELETE_MEMORY(zWaveform);
	return TRUE;
}

int32 CVICALLBACK ThorZPiezo::EveryNDataCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32 retVal = 0, copyCount = 0, written = 0;

	if(_taskHandleAO0)
	{
		//no more buffer to copy
		if(_totalPoints <= _index)
		{
			return (0);
		}
		else if (_totalPoints <= _index + _callbackPoints) //determine count to be copied
		{
			copyCount = _totalPoints - _index;
		}
		else
		{
			copyCount = _callbackPoints;
		}

		retVal = DAQmxWriteAnalogF64(_taskHandleAO0, copyCount, false, -1, DAQmx_Val_GroupByScanNumber, (_pWaveform + _index*_outputLineCount), &written, NULL);
		if(0 != retVal)
		{
			CloseNITasks();
			StringCbPrintfW(_errMsg,MAX_PATH,L"ThorZPiezo unable to write piezo waveform.\n");
			LogMessage(_errMsg,VERBOSE_EVENT);
			return retVal;
		}

		_index += written;
	}
	return retVal;
}

/// <summary>
/// Arm the NI card with the analog waveforms and confifure the triggers
/// </summary>
long ThorZPiezo::LoadWaveformsAndArmDAQ()
{
	//waveform will execute when the _triggerLine is high. This is typically wired to the frame trigger out of the ECU	
	int32 error = 0, retVal = 0, written = 0;
	string analogChanName = _devName + _analogLine;
	long writePoints = _callbackPoints * _preLoadCount;

	try
	{
		CloseNITasks(0);

		if (TRUE == _referenceWaveformEnable)
		{
			analogChanName += "," + _devName + _pockelsReferenceAnalogLine;
		}

		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleAO0));
		DAQmxErrChk(L"DAQmxCreateAOVoltageChan",retVal = DAQmxCreateAOVoltageChan(_taskHandleAO0, analogChanName.c_str() , "", MIN_AO_VOLTAGE, MAX_AO_VOLTAGE, DAQmx_Val_Volts, NULL));

		//active load finite buffer in callback and buffer can't be changed after full,
		//write once instead if not necessary
		if(writePoints < _totalPoints)
		{
			DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent",retVal = DAQmxRegisterEveryNSamplesEvent(_taskHandleAO0, DAQmx_Val_Transferred_From_Buffer, _callbackPoints, 0, EveryNDataCallback, NULL));
			DAQmxErrChk(L"DAQmxSetChanAttribute",retVal = DAQmxSetChanAttribute(_taskHandleAO0, "", DAQmx_AO_DataXferReqCond, DAQmx_Val_OnBrdMemNotFull));
		}
		else
		{
			writePoints = _totalPoints;
		}
		DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleAO0,  "", _piezoSampleRate, DAQmx_Val_Rising,  DAQmx_Val_FiniteSamps, _totalPoints));
		DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleAO0,_totalPoints));
		DAQmxErrChk(L"DAQmxCfgDigEdgeStartTrig ",retVal = DAQmxCfgDigEdgeStartTrig (_taskHandleAO0, (_devName + _triggerLine).c_str(), DAQmx_Val_Rising));
		DAQmxErrChk(L"DAQmxSetStartTrigRetriggerable",retVal = DAQmxSetStartTrigRetriggerable(_taskHandleAO0, true));

		_index = 0;
		DAQmxErrChk(L"DAQmxWriteAnalogF64",retVal = DAQmxWriteAnalogF64(_taskHandleAO0, writePoints, false, -1, DAQmx_Val_GroupByScanNumber, (_pWaveform + _index*_outputLineCount), &written, NULL));
		_index += written;
		DAQmxErrChk(L"DAQmxTaskControl",retVal = DAQmxTaskControl(_taskHandleAO0,DAQmx_Val_Task_Reserve));
		DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleAO0));
	}
	catch(...)
	{
		if( DAQmxFailed(error)) 
		{
			StringCbPrintfW(_errMsg,MAX_PATH,L"DAQ Analog Output Error, error code %d", error);
			LogMessage(_errMsg,VERBOSE_EVENT);
		}
	}
	return (0 == error) ? TRUE : FALSE;
}


/// <summary>
/// Sets a o0.
/// </summary>
/// <param name="voltage">The voltage.</param>
/// <returns>long.</returns>
long ThorZPiezo::SetAO0(double voltage)
{
	int32 error = 0;
	const float64* tmpVec;
	float64 pos;

	//change for accommodate DAQmx 9.8:
	try 
	{
		CloseNITasks(0);

		DAQmxErrChk(L"",error = DAQmxCreateTask("",&_taskHandleAO0));
		DAQmxErrChk(L"",error = DAQmxCreateAOVoltageChan(_taskHandleAO0, (_devName + _analogLine).c_str(),"",MIN_AO_VOLTAGE,MAX_AO_VOLTAGE,DAQmx_Val_Volts,NULL));

		if ((MIN_AO_VOLTAGE <= voltage) && (MAX_AO_VOLTAGE >= voltage))
		{	
			pos = static_cast<float64>(voltage);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO0, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			error = 0;
		}
		else if (MAX_AO_VOLTAGE < voltage)
		{
			pos = static_cast<float64>(MAX_AO_VOLTAGE);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO0, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			StringCbPrintfW(_errMsg,MAX_PATH,L"Trying to apply a voltage higher than uppper limit");
			LogMessage(_errMsg,VERBOSE_EVENT);
		}
		else
		{
			pos = static_cast<float64>(MIN_AO_VOLTAGE);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO0, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			StringCbPrintfW(_errMsg,MAX_PATH,L"Trying to apply a voltage lower than lower limit");
			LogMessage(_errMsg,VERBOSE_EVENT);
		}
		DAQmxErrChk(L"DAQmxWaitUntilTaskDone",error = DAQmxWaitUntilTaskDone(_taskHandleAO0,100));
	} catch (...) {
		if( DAQmxFailed(error)) 
		{
			StringCbPrintfW(_errMsg,MAX_PATH,L"DAQ Analog Output Error, error code %d", error);
			LogMessage(_errMsg,VERBOSE_EVENT);
			return error;
		}
	}
	return error;
}

/// <summary>
/// Closes the ni tasks.
/// </summary>
/// <returns>int.</returns>
void ThorZPiezo::CloseNITasks(double waitTime)
{
	if (NULL != _taskHandleAO0)
	{
		if (0.0 < waitTime)
			DAQmxWaitUntilTaskDone(_taskHandleAO0, waitTime);

		DAQmxStopTask(_taskHandleAO0);

		DAQmxClearTask(_taskHandleAO0);

		_taskHandleAO0 = NULL;
	}
}

