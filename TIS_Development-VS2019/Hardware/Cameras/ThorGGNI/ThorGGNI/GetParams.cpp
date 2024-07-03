// GetParams.cpp : Defines Get functions for the DLL application.
//

#include "stdafx.h"
#include "ThorGGNI.h"
#include "Strsafe.h"

wstring utf8toUtf16(const string & str)
{
	if (str.empty())
		return wstring();

	size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0, 
		str.data(), (int)str.size(), NULL, 0);
	if (charsNeeded == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	vector<wchar_t> buffer(charsNeeded);
	int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0, 
		str.data(), static_cast<long>(str.size()), &buffer[0], static_cast<long>(buffer.size()));
	if (charsConverted == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	return wstring(&buffer[0], charsConverted);
}

long ThorLSMCam::GetLastErrorMsg(wchar_t * msg, long size)
{
	wcsncpy_s(msg,size,_errMsg,_MAX_PATH);

	//reset the error message
	_errMsg[0] = 0;
	return TRUE;
}

long ThorLSMCam::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch (paramID)
	{
	case ICamera::PARAM_CAMERA_TYPE:
		{
			param = ICamera::LSM;
		}
		break;
	case ICamera::PARAM_LSM_TYPE:
		{
			param = ICamera::GALVO_GALVO;
		}
		break;
	case ICamera::PARAM_LSM_SCANMODE:
		{
			param = _scanMode;
		}
		break;
	case ICamera::PARAM_LSM_AREAMODE:
		{
			param = _areaMode;
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

	case ICamera::PARAM_LSM_FIELD_SIZE:
		{
			param = _fieldSize;
		}
		break;
	case ICamera::PARAM_LSM_ALIGNMENT:
		{
			param = _alignmentForField;
		}
		break;
	case ICamera::PARAM_LSM_Y_AMPLITUDE_SCALER:
		{
			param = _yAmplitudeScaler;
		}
		break;
	case ICamera::PARAM_LSM_FLYBACK_CYCLE:
		{
			param = getFlybackCycle();
		}
		break;
	case ICamera::PARAM_LSM_FLYBACK_TIME:
		{
			param = getFlybackTime();
		}
		break;
	case ICamera::PARAM_LSM_MINIMIZE_FLYBACK_CYCLES:
		{
			param = static_cast<long>(_minimizeFlybackCycles);
		}
		break;
	case ICamera::PARAM_LSM_CHANNEL:
		{
			param = _channel;
		}
		break;
	case ICamera::PARAM_LSM_AVERAGEMODE:
		{
			param = _averageMode;
		}
		break;
	case ICamera::PARAM_LSM_AVERAGENUM:
		{
			param = _averageNum;
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_X:
		{
			param  = _offsetX;
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_Y:
		{
			param  = _offsetY;
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE1:
		{
			param  = _inputRangeChannel[0];
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE2:
		{
			param  = _inputRangeChannel[1];
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE3:
		{
			param  = _inputRangeChannel[2];
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE4:
		{
			param  = _inputRangeChannel[3];
		}
		break;
	case ICamera::PARAM_LSM_CLOCKSOURCE:
		{
			param  = _clockSource;
		}
		break;
	case ICamera::PARAM_LSM_INTERNALCLOCKRATE:
		{
			param  = _clockRateInternal;
		}
		break;
	case ICamera::PARAM_LSM_EXTERNALCLOCKRATE:
		{
			param = _clockRateExternal;
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
	case ICamera::PARAM_LSM_DMA_BUFFER_COUNT:
		{
			param = static_cast<double>(_imgPtyDll.dmaBufferCount);
		}
		break;
	case ICamera::PARAM_TRIGGER_TIMEOUT_SEC:
		{
			param = _triggerWaitTimeout;
		}
		break;
	case ICamera::PARAM_ENABLE_FRAME_TRIGGER_WITH_HW_TRIG:
		{
			param = _frameTriggerEnableWithHWTrig;
		}
		break;

	case ICamera::PARAM_LSM_DWELL_TIME:
		{
			param = _dwellTime;
		}
		break;
	case ICamera::PARAM_LSM_DWELL_TIME_STEP:
		{
			param = DWELL_TIME_STEP;
		}
		break;
	case ICamera::PARAM_LSM_GALVO_ENABLE:
		{
			param = _galvoEnable;
		}
		break;
	case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
		{
			param = _forceSettingsUpdate;
		}
		break;
	case ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION:
		{
			param = _fieldSizeCalibration;
		}
		break;
	case ICamera::PARAM_LSM_Y_COMMUNICATION_ENABLE:
		{
			param = _yChannelEnable;
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
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0:
		{
			param = _pockelsLineBlankingPercentage[0]*100.0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1:
		{
			param = _pockelsLineBlankingPercentage[1]*100.0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2:
		{
			param = _pockelsLineBlankingPercentage[2]*100.0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3:
		{
			param = _pockelsLineBlankingPercentage[3]*100.0;
		}
		break;

	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0:
		{
			param = _pockelsResponseType[0];
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_1:
		{
			param = _pockelsResponseType[1];
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_2:
		{
			param = _pockelsResponseType[2];
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_3:
		{
			param = _pockelsResponseType[3];
		}
		break;
	case ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION:
		{
			param = (_verticalScanDirection < 0)? 1 : 0;
		}
		break;
	case ICamera::PARAM_LSM_HORIZONTAL_FLIP:
		{
			param = static_cast<double>(_horizontalFlip);
		}
		break;
	case ICamera::PARAM_LSM_FINE_OFFSET_X:
		{
			param = _fineOffset[0];
		}
		break;
	case ICamera::PARAM_LSM_FINE_OFFSET_Y:
		{
			param = _fineOffset[1];
		}
		break;
	case ICamera::PARAM_LSM_FINE_OFFSET_X2:
		{
			param = _fineOffset2[0];
		}
		break;
	case ICamera::PARAM_LSM_FINE_OFFSET_Y2:
		{
			param = _fineOffset2[1];
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_X:
		{
			param =	_fineFieldSizeScaleX;
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y:
		{
			param =	_fineFieldSizeScaleY;
		}
		break;
	case ICamera::PARAM_LSM_CENTER_WITH_OFFSET:
		{
			param =	_centerWithOffsets;
		}
		break;
	case ICamera::PARAM_FRAME_RATE:
		{
			if(0 < ImageWaveformBuilder->GetFrameTime())
			{
				param = 1.0 / ImageWaveformBuilder->GetFrameTime();
			}
			else
			{
				param = 0;
			}
		}
		break;
	case ICamera::PARAM_LSM_1X_FIELD_SIZE:
		{
			param = _oneXFieldSize;
		}
		break;
	case ICamera::PARAM_LSM_DATAMAP_MODE:
		{
			param = _dataMapMode;
		}
		break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_1:
		{
			param = _channelPolarity[0];
		}
		break;		
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_2:
		{
			param = _channelPolarity[1];
		}
		break;		
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_3:
		{
			param = _channelPolarity[2];
		}
		break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_4:
		{
			param = _channelPolarity[3];
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_0:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_1:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_2:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_3:
		{
			param = 1.0;
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
			param = _pockelsMinVoltage[index];
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
			param = _pockelsMaxVoltage[index];
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_2:index=2;break;
			case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_3:index=3;break;
			}
			param = _pockelsScanVoltageStart[index];
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_2:index=2;break;
			case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_3:index=3;break;
			}
			param = _pockelsScanVoltageStop[index];
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_0:
		{
			param = _pockelsEnable[0];
		}
		break;		
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_1:
		{
			param = _pockelsEnable[1];
		}
		break;		
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_2:
		{
			param = _pockelsEnable[2];
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_3:
		{
			param = _pockelsEnable[3];
		}
		break;
	case ICamera::PARAM_LSM_SCANAREA_ANGLE:
		{
			param = _scanAreaAngle * 180 / PI; // send angle in degrees
		}
		break;
	case ICamera::PARAM_DROPPED_FRAMES:
		{
			param = _droppedFramesCnt;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF:
		{
			param = _useReferenceForPockelsOutput;
		}
		break;
	case ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS:
		{
			param = _precaptureStatus;
		}
		break;
	case ICamera::PARAM_LSM_INTERLEAVE_SCAN:
		{
			param = _interleaveScan;
		}
		break;
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_X:
		{
			param = _highResOffset[0];
		}
		break;
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_Y:
		{
			param = _highResOffset[1];
		}
		break;
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_X2:
		{
			param = _highResOffset2[0];
		}
		break;
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_Y2:
		{
			param = _highResOffset2[1];
		}
		break;
	case ICamera::PARAM_LSM_GG_SUPER_USER:
		{
			param = _ggSuperUserMode;
		}
		break;
	case ICamera::PARAM_LSM_CALCULATED_MIN_DWELL:
		{
			double val = CalculateMinimumDwellTime(_fieldSize, _fineFieldSizeScaleX, _pixelX, 2*static_cast<long>(_galvoRetraceTime), _field2Theta, static_cast<long>(_maxGalvoOpticalAngle), _maxAngularVelocityRadPerSec, _maxAngularAccelerationRadPerSecSq, _minDwellTime, MAX_DWELL_TIME, DWELL_TIME_STEP);
			param = val;
		}
		break;
	case ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS:
		{
			param = _timebasedLSTimeMS;
		}
		break;
	case ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN:
		{
			param = _timeBasedLineScanEnabled;
		}
		break;
	case ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN_INCREMENT_TIME_MS:
		{
			double lineTimeMS = (_pixelX * _dwellTime + 2 * _galvoRetraceTime) / MS_TO_SEC;
			param = floor(lineTimeMS * 2 + 0.5);
		}
		break;
	default:
		{
			ret = FALSE;
		}
	}

	if(FALSE == ret)
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"Parameter (%d) not implemented",paramID);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	return ret;
}

long ThorLSMCam::GetParamBuffer(const long paramID, char * pBuffer, long size)
{
	long ret = TRUE;

	switch (paramID)
	{
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_0:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_1:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_2:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_3:
		{
			long index = 0;
			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_0:index = 0;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_1:index = 1;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_2:index = 2;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_3:index = 3;break;
			}

			if(POCKELS_VOLTAGE_STEPS * sizeof(float64) <= size)
			{
				memcpy(pBuffer,&_pockelsReadArray[index],POCKELS_VOLTAGE_STEPS * sizeof(float64));
			}
		}
		break;
	default:
		{
			ret = FALSE;
			StringCbPrintfW(_errMsg,_MAX_PATH,L"Parameter (%d) not implemented",paramID);
			LogMessage(_errMsg,ERROR_EVENT);
		}
	}

	return ret;
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
			paramMin = ICamera::GALVO_GALVO;
			paramMax = ICamera::GALVO_GALVO;
			paramDefault = ICamera::GALVO_GALVO;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_TRIGGER_MODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_TRIGGER_MODE;
			paramMax = MAX_TRIGGER_MODE;
			paramDefault = DEFAULT_TRIGGER_MODE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_X:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_PIXEL_X;
			paramMax = MAX_PIXEL_X;
			paramDefault = DEFAULT_PIXEL_X;
			paramReadOnly = FALSE;
		}
		break;

	case ICamera::PARAM_LSM_PIXEL_Y:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_PIXEL_Y;
			paramMax = MAX_PIXEL_Y;
			paramDefault = DEFAULT_PIXEL_Y;
			paramReadOnly = FALSE;
		}
		break;

	case ICamera::PARAM_LSM_FIELD_SIZE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _fieldSizeMin;
			paramMax = _fieldSizeMax;
			paramDefault = DEFAULT_FIELD_SIZE_X;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_X:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = -(_fieldSizeMax - _fieldSizeMin) / 2;
			paramMax = (_fieldSizeMax - _fieldSizeMin) / 2;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_Y:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = -(_fieldSizeMax - _fieldSizeMin) / 2;
			paramMax = (_fieldSizeMax - _fieldSizeMin) / 2;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_CHANNEL:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_CHANNEL;         //at least one channel
			paramMax = _maxChannel;
			paramDefault = MIN_CHANNEL;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_ALIGNMENT:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_ALIGNMENT;
			paramMax = MAX_ALIGNMENT;
			paramDefault = MIN_ALIGNMENT;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_Y_AMPLITUDE_SCALER:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_Y_AMPLITUDE_SCALER;
			paramMax = MAX_Y_AMPLITUDE_SCALER;
			paramDefault = DEFAULT_Y_AMPLITUDE_SCALER;
			paramReadOnly = FALSE;
		}
		break;	
	case ICamera::PARAM_LSM_FLYBACK_CYCLE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_FLYBACK_CYCLE;
			paramMax = MAX_FLYBACK_CYCLE;
			paramDefault = DEFAULT_FLYBACK_CYCLE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_FLYBACK_TIME:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_FLYBACK_CYCLE / 100000.0 ;
			paramMax = MAX_FLYBACK_CYCLE / 0.01;
			paramDefault = DEFAULT_FLYBACK_CYCLE / 10;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_MINIMIZE_FLYBACK_CYCLES:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_GALVO_ENABLE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_GALVO_ENABLE;
			paramMax = MAX_GALVO_ENABLE;
			paramDefault = DEFAULT_GALVO_ENABLE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE1:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_INPUTRANGE;
			paramMax = MAX_INPUTRANGE;
			paramDefault = DEFAULT_INPUTRANGE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE2:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_INPUTRANGE;
			paramMax = MAX_INPUTRANGE;
			paramDefault = DEFAULT_INPUTRANGE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE3:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_INPUTRANGE;
			paramMax = MAX_INPUTRANGE;
			paramDefault = DEFAULT_INPUTRANGE;
			paramReadOnly = FALSE;
		}
		break;
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
	case ICamera::PARAM_LSM_CLOCKSOURCE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 1;         
			paramMax = 1;         
			paramDefault = 1;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_INTERNALCLOCKRATE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_INTERNALCLOCKRATE;
			paramMax = MAX_INTERNALCLOCKRATE;
			paramDefault = DEFAULT_INTERNALCLOCKRATE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_EXTERNALCLOCKRATE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_EXTCLOCKRATE;
			paramMax = MAX_EXTCLOCKRATE;
			paramDefault = DEFAULT_EXTCLOCKRATE;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_SCANMODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_SCANMODE;
			paramMax = MAX_SCANMODE;
			paramDefault = MIN_SCANMODE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_AREAMODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_AREAMODE;
			paramMax = MAX_AREAMODE;
			paramDefault = MIN_AREAMODE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_AVERAGEMODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = NO_AVERAGE;
			paramMax = FRM_CUMULATIVE_MOVING;
			paramDefault = NO_AVERAGE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_AVERAGENUM:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_AVERAGENUM;
			paramMax = MAX_AVERAGENUM;
			paramDefault = DEFAULT_AVERAGENUM;
			paramReadOnly = FALSE;
		}
		break;

	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 1;

			long lineTotal;

			//determined using a field size of 255 for the a field2theta of .1
			const long MAX_BACKLINE_COUNT = 48;

			//Set Galvo Related Line number per frame
			if (_imgPtyDll.scanMode == TWO_WAY_SCAN)         //two way scan
			{
				lineTotal = MAX_BACKLINE_COUNT + _imgPtyDll.pixelY / 2;
			}
			else
			{
				lineTotal = MAX_BACKLINE_COUNT + _imgPtyDll.pixelY;
			}

			paramMax = INT_MAX / (lineTotal);
			paramDefault = 10;
			paramReadOnly = FALSE;
		}
		break;

	case ICamera::PARAM_TRIGGER_TIMEOUT_SEC:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_TRIGGER_TIMEOUT;
			paramMax = MAX_TRIGGER_TIMEOUT;        //LONG_MAX/1000   The limit is restricted by the maximum wait time for a win32 event
			paramDefault = 30;
			paramReadOnly = FALSE;
		}
		break;

	case ICamera::PARAM_ENABLE_FRAME_TRIGGER_WITH_HW_TRIG:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_ENABLE_FRAME_TRIGGER;
			paramMax = MAX_ENABLE_FRAME_TRIGGER;
			paramDefault = 1;
			paramReadOnly = FALSE;
		}
		break;

	case ICamera::PARAM_LSM_DWELL_TIME:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMax = MAX_DWELL_TIME; 
			paramMin = _minDwellTime;
			paramDefault = DEFAULT_DWELL_TIME;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_DWELL_TIME_STEP:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMax = DWELL_TIME_STEP; 
			paramMin = DWELL_TIME_STEP;
			paramDefault = DWELL_TIME_STEP;
			paramReadOnly = TRUE;
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

	case ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = _fieldSizeCalibrationAvailable;
			paramMin = 1;
			paramMax = 1000;
			paramDefault = 1;
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
	case ICamera::PARAM_LSM_Y_COMMUNICATION_ENABLE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_Y_CHANNEL_ENABLE;
			paramMax = MAX_Y_CHANNEL_ENABLE;
			paramDefault = DEFAULT_Y_CHANNEL_ENABLE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;			//100;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = MAX_POCKELS_LINE_BLANKING_PERCENTAGE;
			paramDefault = 10;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0:
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_1:
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_2:
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_3:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = PockelsResponseType::SINE_RESPONSE;
			paramMax = PockelsResponseType::LAST_POCKELS_RESPONSE;
			paramDefault = PockelsResponseType::SINE_RESPONSE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_HORIZONTAL_FLIP:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_FINE_OFFSET_X:
	case ICamera::PARAM_LSM_FINE_OFFSET_Y:
	case ICamera::PARAM_LSM_FINE_OFFSET_X2:
	case ICamera::PARAM_LSM_FINE_OFFSET_Y2:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = -1.0;
			paramMax = 1.0;
			paramDefault = 0.0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_X:
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = .8;
			paramMax = 1.2;
			paramDefault = 1.0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_CENTER_WITH_OFFSET:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_FRAME_RATE:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0.0;
			paramMax = 100000.0;
			paramDefault = 1.0;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_1X_FIELD_SIZE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _fieldSizeMin;
			paramMax = _fieldSizeMax;
			paramDefault = 160;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_DATAMAP_MODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = ICamera::POLARITY_INDEPENDENT;
			paramMax = ICamera::POLARITY_NEGATIVE;
			paramDefault = ICamera::POLARITY_INDEPENDENT;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_1:
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_2:
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_3:
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_4:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = POL_NEG;
			paramMax = POL_POS;
			paramDefault = POL_NEG;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_0:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_1:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_2:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_3:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 1;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_0:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_1:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_2:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_3:
		{
			paramType = ICamera::TYPE_BUFFER;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 0;
			paramDefault = 0;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = -10.0;
			paramMax = 10.0;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = -10.0;
			paramMax = 10.0;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_3:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = -10.0;
			paramMax = 10.0;
			paramDefault = 0;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_3:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = -10.0;
			paramMax = 10.0;
			paramDefault = 0;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_0:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_1:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_2:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_3:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
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
	case ICamera::PARAM_LSM_SCANAREA_ANGLE:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = MIN_SCANAREA_ANGLE * 180 / PI; //Convert to degrees
			paramMax = MAX_SCANAREA_ANGLE * 180 / PI; //Convert to degrees
			paramDefault = DEFAULT_SCANAREA_ANGLE * 180 / PI; //Convert to degrees
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_DROPPED_FRAMES:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = INT_MAX;
			paramDefault = 0;
			paramReadOnly = TRUE;
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
	case ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_INTERLEAVE_SCAN:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_X:
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_Y:
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_X2:
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_Y2:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = _highResOffsetMinMax[0];
			paramMax = _highResOffsetMinMax[1];
			paramDefault = 0.0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_GG_SUPER_USER:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_CALCULATED_MIN_DWELL:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMax = MAX_DWELL_TIME; 
			paramMin = _minDwellTime;
			paramDefault = _minDwellTime;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = MIN_TB_LINE_SCAN_TIME;
			paramMax = MAX_TB_LINE_SCAN_TIME;
			paramDefault = MIN_TB_LINE_SCAN_TIME;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN_INCREMENT_TIME_MS:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 1;
			paramMax = MAX_TB_LINE_SCAN_TIME;
			paramDefault = 1;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_IS_LIVE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_WAVEFORM_OUTPATH:
		{
			paramType = ICamera::TYPE_STRING;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
		}
		break;
	default:
		{
			ret = FALSE;
			paramAvailable = FALSE;
			paramReadOnly = TRUE;
		}
	}

	return ret;
}

long ThorLSMCam::GetParamString(const long paramID, wchar_t * str, long size)
{
	long ret = TRUE;

	switch(paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME:
		{
			std::wregex wreg (L"\\b(Thor)([^ ]*)"); 
			wcscpy_s(str,size, std::regex_replace(GetDLLName(ThorLSMCam::getInstance()->hDLLInstance), wreg, L"$2").c_str());
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_0:
		{
			wstring wsTemp = utf8toUtf16(_pockelsLine[0]);
			wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_1:
		{
			wstring wsTemp = utf8toUtf16(_pockelsLine[1]);
			wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_2:
		{
			wstring wsTemp = utf8toUtf16(_pockelsLine[2]);
			wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_3:
		{
			wstring wsTemp = utf8toUtf16(_pockelsLine[3]);
			wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case ICamera::PARAM_LSM_WAVEFORM_PATH_NAME:
		{
			wcscpy_s(str,size, _waveformPathName.c_str());
		}
		break;
	case ICamera::PARAM_WAVEFORM_OUTPATH:
		{
			wcscpy_s(str, size, _waveformOutPath.c_str());
		}
		break;
	default:
		ret = FALSE;
		break;
	}

	return ret;
}

long ThorLSMCam::GetActionResult(ActionType actionType, long& paramVal)
{
	long ret = FALSE;
	switch (actionType)
	{
	case STATUS:
		paramVal = _waveformTaskStatus;
		return TRUE;
	case STATUS_PROTOCOL:
		return StatusProtocol(paramVal);
	default:
		break;
	}
	return ret;
}

long ThorLSMCam::GetActionResult(ActionType actionType, char* pDataBuffer)
{
	switch (actionType)
	{
	case COPY_PROTOCOL:
		return CopyProtocol(pDataBuffer);
	default:
		break;
	}
	return FALSE;
}

/// <summary> Calculate the flyback time needed for the currently set flyback cycles </summary>
/// <returns> The time in seconds that it would take to do the currently set flyback cycles </returns>
double ThorLSMCam::getFlybackTime()
{
	return getFlybackTime(getFlybackCycle());
}

/// <summary> Calculate the flyback time the input number of flyback cycles will take </summary>
/// <param name="flybackCycle"> The number of flyback cycles to account for </param>
/// <returns> The time in seconds that it would take to do the requested number of flyback cycles </returns>
double ThorLSMCam::getFlybackTime(long flybackCycle)
{
	long forwardLines;

	//Set Galvo Related Line number per frame
	if (_scanMode == TWO_WAY_SCAN) //two way scan
		forwardLines = _pixelY / 2;
	else
		forwardLines = _pixelY;

	long backwardLines = flybackCycle;

	long overalLines = forwardLines + backwardLines;

	//clock the galvos according to the entered dwell time (non-waveform scan mode)
	const long CONVERT_uS_TO_SEC = 1000000;
	long clockRate = static_cast<long>(CONVERT_uS_TO_SEC/_dwellTime);			

	//Set Galvo driving waveform
	long galvoSamplesPadding = static_cast<long> (_galvoRetraceTime/_dwellTime);  //used fixed amount of time for x galvo retrace
	long galvoSamplesEffective = _pixelX;
	long galvoFwdSamplesPerLine = static_cast<long>(galvoSamplesEffective + galvoSamplesPadding *2);
	long galvoBwdSamplesPerLine = static_cast<long>(galvoSamplesEffective + galvoSamplesPadding *2);
	long galvoSamplesPerLine = galvoFwdSamplesPerLine + galvoBwdSamplesPerLine;
	long galvoDataForward = forwardLines * galvoSamplesPerLine;
	long galvoDataBack = backwardLines * galvoSamplesPerLine;
	long galvoDataLength;
	if(FALSE == _galvoEnable || ICamera::LSMAreaMode::POLYLINE == _areaMode)
	{
		//If galvoEnable is false (meaning line scan) there is no need for a backscan
		//only a need to keep the frame trigger down for a few time points (LINE_FRAMETRIGGER_TIMEPOINTS)
		long flybackLength = (0 < backwardLines) ? backwardLines * galvoSamplesPerLine : LINE_FRAMETRIGGER_LOW_TIMEPOINTS;
		galvoDataLength = forwardLines * galvoSamplesPerLine + flybackLength;
	}
	else
	{
		galvoDataLength = overalLines * galvoSamplesPerLine;
	}
	double flybackTime = flybackCycle / ((static_cast<double>(clockRate)/ galvoDataLength) * overalLines);
	return flybackTime;

}

/// <summary> Calculates the minimum value for flyback cycles the current settings can support </summary>
/// <returns> The calculated minimum flyback cycles </returns>
long ThorLSMCam::getMinFlybackCycle()
{
	if (ICamera::LSMAreaMode::POLYLINE == _areaMode || FALSE == _galvoEnable)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/// <summary> Returns the currently number of flyback cycles to use, modified if needed to fall within
/// the minimum the current settings support </summary>
/// <returns> The flybackCycle parameter the hardware should use </returns>
long ThorLSMCam::getFlybackCycle()
{
	long minFlybackCycle = getMinFlybackCycle();
	if(_minimizeFlybackCycles || minFlybackCycle > _rawFlybackCycle)
	{
		return minFlybackCycle;
	}
	else
	{
		return _rawFlybackCycle;
	}
}

