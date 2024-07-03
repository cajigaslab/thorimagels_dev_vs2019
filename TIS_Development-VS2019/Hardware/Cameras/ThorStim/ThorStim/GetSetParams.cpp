// GetSetParams.cpp : Defines Get/Set functions for the DLL application.
//

#include "stdafx.h"
#include "ThorStim.h"
#include "Strsafe.h"

long ThorStim::GetParamInfo
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
	long index = 0;

	switch(paramID)
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
			paramMin = ICamera::STIMULATE_MODULATOR;
			paramMax = ICamera::STIMULATE_MODULATOR;
			paramDefault = ICamera::STIMULATE_MODULATOR;
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
	case ICamera::PARAM_TRIGGER_MODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = ICamera::TriggerMode::FIRST_TRIGGER_MODE;
			paramMax = ICamera::TriggerMode::LAST_TRIGGER_MODE;
			paramDefault = ICamera::TriggerMode::SW_MULTI_FRAME;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 1;
			paramMax = INT_MAX;
			paramDefault = 1;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_INTERNALCLOCKRATE:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0.001;
			paramMax = INT_MAX;
			paramDefault = RATE6321;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE;
			paramMax = PreCaptureStatus::PRECAPTURE_DONE;
			paramDefault = PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_WAVEFORM_PATH_NAME:
		{
			paramType = ICamera::TYPE_STRING;
			paramAvailable	= TRUE;
			paramReadOnly	= FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:
		{
			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:index = 0;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:index = 1;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:index = 2;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:index = 3;break;
			}
			paramType = ICamera::TYPE_LONG;
			paramAvailable = (0 < _pockelsLine[index].length()) ? TRUE : FALSE;
			paramMin = 0;
			paramMax = Constants::HUNDRED_PERCENT;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0:
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_1:
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_2:
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_3:
		{
			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0:index = 0;break;
			case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_1:index = 1;break;
			case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_2:index = 2;break;
			case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_3:index = 3;break;
			}
			paramType = ICamera::TYPE_LONG;
			paramAvailable = (0 < _pockelsLine[index].length()) ? TRUE : FALSE;
			paramMin = PockelsResponseType::SINE_RESPONSE;
			paramMax = PockelsResponseType::LAST_POCKELS_RESPONSE;
			paramDefault = PockelsResponseType::SINE_RESPONSE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:
		{
			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:index = 0;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:index = 1;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:index = 2;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:index = 3;break;
			}
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = (0 < _pockelsLine[index].length()) ? TRUE : FALSE;
			paramMin = MIN_AO_VOLTAGE;
			paramMax = MAX_AO_VOLTAGE;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:
		{
			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:index = 0;break;
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:index = 1;break;
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:index = 2;break;
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:index = 3;break;
			}
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = (0 < _pockelsLine[index].length()) ? TRUE : FALSE;
			paramMin = MIN_AO_VOLTAGE;
			paramMax = MAX_AO_VOLTAGE;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_0:
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_1:
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_2:
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_3:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = TRUE;
		}
		break;
	default:
		paramAvailable = FALSE;
		ret = FALSE;
	}

	return ret;
}

long ThorStim::GetParam(const long paramID, double &param)
{
	long ret = TRUE;
	int32 error = 0;
	uInt32 samples[1] = {0};
	int32 samplesRead = 0;
	int32 bytesPerSamp = 0;
	uInt8 readVal[1] = {0};
	long count = 0, index = 0;
	double dVal = 0, dAvg = 0;
	param = static_cast<double>(samples[0]);
	try
	{
		switch(paramID)
		{
		case ICamera::PARAM_CAMERA_TYPE:
			{
				param = ICamera::LSM;
			}
			break;
		case ICamera::PARAM_LSM_TYPE:
			{
				param = ICamera::STIMULATE_MODULATOR;
			}
			break;
		case ICamera::PARAM_TRIGGER_MODE:
			{
				param  = _triggerMode;
			}
			break;
		case ICamera::PARAM_MULTI_FRAME_COUNT:
			{
				param  = _frameCount;
			}
			break;
		case ICamera::PARAM_LSM_INTERNALCLOCKRATE:
			{
				param = _sampleRateHz;
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
				if (0< _pockelsLine[index].length())
				{
					switch(_pockelsResponseType[index])
					{
					case static_cast<long>(PockelsResponseType::SINE_RESPONSE):
						{
							//linearize the sine wave response of the pockels cell
							param = 100.0*(1 - cos(PI * _pockelsPowerLevel[index]))/static_cast<double>(Constants::AREA_UNDER_CURVE);
						}
						break;
					case static_cast<long>(PockelsResponseType::LINEAR_RESPONSE):
						{
							//linear response
							param = 100.0*_pockelsPowerLevel[index];
						}
						break;
					}
				}
				else
					ret = FALSE;
			}
			break;
		case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0:
		case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_1:
		case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_2:
		case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_3:
			{
				long index = 0;
				switch(paramID)
				{
				case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0:index = 0;break;
				case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_1:index = 1;break;
				case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_2:index = 2;break;
				case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_3:index = 3;break;
				}
				if (0< _pockelsLine[index].length())
				{
					param = _pockelsResponseType[index];
				}
				else
					ret = FALSE;
			}
			break;
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:
			{
				long index = 0;

				switch( paramID)
				{
				case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:index=0;break;
				case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:index=1;break;
				case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:index=2;break;
				case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:index=3;break;
				}
				if (0< _pockelsLine[index].length())
				{
					param = _pockelsMinVoltage[index];
				}
				else 
					ret = FALSE;
			}
			break;
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:
			{
				long index = 0;

				switch( paramID)
				{
				case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:index=0;break;
				case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:index=1;break;
				case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:index=2;break;
				case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:index=3;break;
				}
				if (0< _pockelsLine[index].length())
				{
					param = _pockelsMaxVoltage[index];
				}
				else
					ret = FALSE;
			}
			break;
		case ICamera::PARAM_LSM_POCKELS_CONNECTED_0:
		case ICamera::PARAM_LSM_POCKELS_CONNECTED_1:
		case ICamera::PARAM_LSM_POCKELS_CONNECTED_2:
		case ICamera::PARAM_LSM_POCKELS_CONNECTED_3:
			{
				switch(paramID)
				{
				case ICamera::PARAM_LSM_POCKELS_CONNECTED_0:index = 0;break;
				case ICamera::PARAM_LSM_POCKELS_CONNECTED_1:index = 1;break;
				case ICamera::PARAM_LSM_POCKELS_CONNECTED_2:index = 2;break;
				case ICamera::PARAM_LSM_POCKELS_CONNECTED_3:index = 3;break;
				}
				param = (0 < _pockelsLine[index].length()) ? TRUE : FALSE;
			}
			break;
		case ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS:
			{
				param = _precaptureStatus;
			}
			break;
		default:
			ret = FALSE;
		}
		StringCbPrintfW(message,_MAX_PATH, L"");
	}
	catch(...)
	{
		StringCbPrintfW(message,_MAX_PATH, L"ThorStim unable to get param (%d) error (%d).", paramID, error);
		LogMessage(message, ERROR_EVENT);
		ret = FALSE;
	}
	return ret;
}

long ThorStim::SetParam(const long paramID, const double param)
{
	long ret = TRUE;
	int32 error = 0;
	double dVal = 0.0;

	switch(paramID)
	{
	case ICamera::PARAM_TRIGGER_MODE:
		{
			if ((static_cast<long> (param) >= ICamera::TriggerMode::FIRST_TRIGGER_MODE) && (static_cast<long> (param) < ICamera::TriggerMode::LAST_TRIGGER_MODE))
			{
				_triggerMode = static_cast<long>(param);
			}
		}
		break;
	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			if (param >= 1)
			{
				_frameCount = static_cast<long>(param);
			}
		}
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

			if((param >= 0) && (param <= 100) && 0 < _pockelsLine[index].length())
			{	
				switch(_pockelsResponseType[index])
				{
				case PockelsResponseType::SINE_RESPONSE:
					{
						//linearize the sine wave response of the pockels cell
						dVal = acos(1 - static_cast<double>(Constants::AREA_UNDER_CURVE) * param/100.0)/PI;
					}
					break;
				case PockelsResponseType::LINEAR_RESPONSE:
					{
						//linear response
						dVal = param/100.0;
					}
					break;
				}
				CheckNewValue<double>(_pockelsPowerLevel[index], dVal);
				return MovePockelsToPowerLevel(index);
			}
			else
				ret = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0:
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_1:
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_2:
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_3:
		{
			long index=0;
			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_2:index=2;break;
			case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_3:index=3;break;
			}
			if (param >= PockelsResponseType::SINE_RESPONSE && param < PockelsResponseType::LAST_POCKELS_RESPONSE && 0 < _pockelsLine[index].length())
				CheckNewValue<long>(_pockelsResponseType[index], static_cast<long>(param));
			else
				ret = FALSE;
		}
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:index=2;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:index=3;break;
			}
			if (param >= 0 && param <= Constants::HUNDRED_PERCENT && 0 < _pockelsLine[index].length())
				CheckNewValue<double>(_pockelsMinVoltage[index], param);
			else
				ret = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:index=2;break;
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:index=3;break;
			}
			if (param >= 0 && param <= Constants::HUNDRED_PERCENT && 0 < _pockelsLine[index].length())
				CheckNewValue<double>(_pockelsMaxVoltage[index], param);
			else
				ret = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS:
		{
			if((PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE <= param) || (PreCaptureStatus::PRECAPTURE_DONE >= param))
			{
				_precaptureStatus = static_cast<long>(param);
			}
		}
		break;
	default:
		ret = FALSE;
	}
	StringCbPrintfW(message,_MAX_PATH, L"");
	return ret;
}

/// <summary>
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorStim::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	return TRUE;
}

/// <summary>
/// Gets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorStim::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	return TRUE;
}

/// <summary>
/// Sets the parameter string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <returns>long.</returns>
long ThorStim::SetParamString(const long paramID, wchar_t* str)
{
	long ret = TRUE;

	switch (paramID)
	{
	case ICamera::PARAM_LSM_WAVEFORM_PATH_NAME:
		{
			_waveformPathName = std::wstring(str);
		}
		break;
	default: 
		ret = FALSE;
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
long ThorStim::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = TRUE;
	switch(paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME:
		{
			std::wregex wreg (L"\\b(Thor)([^ ]*)"); 
			wcscpy_s(str,size, std::regex_replace(GetDLLName(ThorStim::getInstance()->hDLLInstance), wreg, L"$2").c_str());
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_0:
		{
			//wstring wsTemp = utf8toUtf16(_pockelsLine[0]);
			//wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_1:
		{
			//wstring wsTemp = utf8toUtf16(_pockelsLine[1]);
			//wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_2:
		{
			//wstring wsTemp = utf8toUtf16(_pockelsLine[2]);
			//wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_3:
		{
			//wstring wsTemp = utf8toUtf16(_pockelsLine[3]);
			//wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case ICamera::PARAM_LSM_WAVEFORM_PATH_NAME:
		{
			wcscpy_s(str,size, _waveformPathName.c_str());
		}
		break;
	default:
		ret = FALSE;
		break;
	}
	return ret;
}
