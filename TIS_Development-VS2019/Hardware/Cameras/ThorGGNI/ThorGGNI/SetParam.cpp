// SetParams.cpp : Defines Set functions for the DLL application.
//

#include "stdafx.h"
#include "ThorGGNI.h"
#include "ThorGGNISetupXML.h"
#include "Strsafe.h"

//Update parameters based on area mode.
void ThorLSMCam::SetAreaMode()
{
	switch ((ICamera::LSMAreaMode)_areaMode)
	{
	case ICamera::LSMAreaMode::SQUARE:
		_pixelY = _pixelX;
		break;
	case ICamera::LSMAreaMode::RECTANGLE:
		if(2 >= _pixelY)	//line scan, switch between two-way or one-way
		{
			_pixelY = (ScanMode::TWO_WAY_SCAN == (ScanMode)_scanMode) ? 2 : 1;
		}
		break;
	case ICamera::LSMAreaMode::POLYLINE:
		_scanMode = ScanMode::FORWARD_SCAN;
		SetParam(ICamera::PARAM_LSM_INTERLEAVE_SCAN, 0.0);
		break;
	case ICamera::LSMAreaMode::LINE:
		_pixelY = 1;
		SetParam(ICamera::PARAM_LSM_INTERLEAVE_SCAN, 0.0);
		break;
	}
}

long ThorLSMCam::SetParam(const long paramID, const double param)
{
	long ret = FALSE, resetFreeRun = FALSE;
	double dVal = 0;
	switch (paramID)
	{
	case ICamera::PARAM_LSM_SCANMODE:
		{
			if ((param >= MIN_SCANMODE) && (param <= MAX_SCANMODE))
			{
				_scanMode = static_cast<long> (param);
				SetAreaMode();
				CloseThread();
				_behaviorPtr = _behaviorFac->GetBehaviorInstance(this, (ScanMode)_scanMode, (AverageMode)_averageMode);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_SCANMODE %d outside range %d to %d",static_cast<long> (param), MIN_SCANMODE,MAX_SCANMODE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_AREAMODE:
		{
			if ((param >= MIN_AREAMODE) && (param <= MAX_AREAMODE))
			{
				_areaMode = static_cast<long>(param);
				SetAreaMode();
				CloseThread();
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_AREAMODE %d outside range %d to %d",static_cast<long> (param), MIN_AREAMODE,MAX_AREAMODE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_X:
		{
			long factor = 1;
			if (_behaviorPtr)
				_behaviorPtr->GetParam(BehaviorProp::LINE_FACTOR, factor);

			if ((param >= MIN_PIXEL_X) && (param <= MAX_PIXEL_X / factor))
			{
				if (static_cast<long>(param) % 16 == 0)
				{
					_pixelX = static_cast<long>(param);
					SetAreaMode();
					CloseThread();
					ret = TRUE;
				}
				else
				{
					StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_PIXEL_X must be a multiple of 16");
					LogMessage(_errMsg,ERROR_EVENT);
				}
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_PIXEL_X %d outside range %d to %d",static_cast<long> (param), MIN_PIXEL_X, static_cast<long>(MAX_PIXEL_X / factor));
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;

	case ICamera::PARAM_LSM_PIXEL_Y:
		{
			long factor = 1;
			if (_behaviorPtr)
				_behaviorPtr->GetParam(BehaviorProp::LINE_FACTOR, factor);

			if(FALSE == _timeBasedLineScanEnabled)
			{
				if ((param >= MIN_PIXEL_Y) && (param <= MAX_PIXEL_Y / factor))
				{
					if ((0 == static_cast<long>(param) % 16) && (static_cast<long>(param) <= _pixelX))
					{
						_pixelY = static_cast<long>(param);
						SetAreaMode();
						CloseThread();
					}
					else if (1 == static_cast<long>(param) || 2 == static_cast<long>(param))
					{
						_pixelY = static_cast<long>(param);
						SetAreaMode();
						SetParam(ICamera::PARAM_LSM_INTERLEAVE_SCAN, 0.0);	//no interleave in line scan
					}
					else
					{
						StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_PIXEL_Y must be a multiple of 16");
						LogMessage(_errMsg,ERROR_EVENT);
					}
				}
				else
				{
					StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_PIXEL_Y %d outside range %d to %d",static_cast<long> (param), MIN_PIXEL_Y, static_cast<long>(MAX_PIXEL_Y / factor));
					LogMessage(_errMsg,ERROR_EVENT);
				}
			}
			ret = TRUE;
		}
		break;

	case ICamera::PARAM_LSM_FIELD_SIZE:
		{
			if ((param >= _fieldSizeMin) && (param <= _fieldSizeMax))
			{
				double fieldSizeParam = param;
				long minFieldSize = CalculateMinimumFieldSize(_dwellTime, _pixelX, 2*static_cast<long>(_galvoRetraceTime), _field2Theta, static_cast<long>(_maxGalvoOpticalAngle));
				if(param > minFieldSize && FALSE == _ggSuperUserMode)
				{
					//Calculate the minimum field size allowed at this pixel density and dwell time, if the passed value is bigger than the allowed min, 
					//set the current field size to the minimum allowed value
					fieldSizeParam = minFieldSize;
					StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_FIELD_SIZE results in a voltage that is out of range. Moving to minimum safe value.");
					LogMessage(_errMsg,ERROR_EVENT);
				}

				double paddedAmplitude = fieldSizeParam * _galvoRetraceTime * 4 / ( _pixelX * _dwellTime * 2 * PI);
				double fieldX_angle = (fieldSizeParam +paddedAmplitude *2) * _field2Theta;

				StringCbPrintfW(_errMsg,_MAX_PATH,L"fieldX angle %d.%d",static_cast<long> (fieldX_angle), static_cast<long>(1000 * (fieldX_angle - static_cast<long> (fieldX_angle))));
				LogMessage(_errMsg,VERBOSE_EVENT);

				resetFreeRun = CheckNewValue<long>(_fieldSize, static_cast<long>(fieldSizeParam));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_FIELD_SIZE %d outside range %d to %d",static_cast<long> (param), _fieldSizeMin,_fieldSizeMax);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_X:
		{
			if ((param >= -(_fieldSizeMax - _fieldSize) / 2) && (param <= (_fieldSizeMax - _fieldSize) / 2))
			{
				resetFreeRun = CheckNewValue<long>(_offsetX, static_cast<long>(param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_OFFSET_X %d outside range %d to %d",static_cast<long> (param), -(_fieldSizeMax - _fieldSize) / 2,(_fieldSizeMax - _fieldSize) / 2);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_Y:
		{
			if ((param >= -(_fieldSizeMax - _fieldSize) / 2) && (param <= (_fieldSizeMax - _fieldSize) / 2))
			{
				resetFreeRun = CheckNewValue<long>(_offsetY, static_cast<long>(param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_OFFSET_Y %d outside range %d to %d",static_cast<long> (param), -(_fieldSizeMax - _fieldSize) / 2,(_fieldSizeMax - _fieldSize) / 2);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_ALIGNMENT:
		{
			//the dwell time should not be greater than avgCount*4*galvoSamplesPadding 
			//(number of samples per galvo point times acceleration and deceleration on each side)
			//This number varies with the pixel dwell time
			const long CONVERT_uS_TO_SEC = 1000000;
			long galvoSamplesPadding = static_cast<long> (_galvoRetraceTime/_dwellTime); 
			long clockRate = static_cast<long>(floor((CONVERT_uS_TO_SEC/_dwellTime)));// 1s/us
			long avgCount = DATA_SAMPLE_RATE/clockRate;
			long maxAligmengForDwellTime = avgCount*galvoSamplesPadding*4;
			long loadedShift = _shiftArray[static_cast<long>((_dwellTime - _minDwellTime)/DWELL_TIME_STEP)];

			if ((loadedShift + param >= 0) &&  (loadedShift + param <= maxAligmengForDwellTime))
			{
				resetFreeRun = CheckNewValue<long>(_alignmentForField, static_cast<long>(param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_ALIGNMENT %d outside range %d to %d",static_cast<long> (param), MIN_ALIGNMENT,MAX_ALIGNMENT);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_Y_AMPLITUDE_SCALER:
		{
			if ((param >= MIN_Y_AMPLITUDE_SCALER) && (param <= MAX_Y_AMPLITUDE_SCALER))
			{
				resetFreeRun = CheckNewValue<double>(_yAmplitudeScaler, param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_Y_AMPLITUDE_SCALER %d outside range %d to %d",static_cast<long> (param), MIN_Y_AMPLITUDE_SCALER,MAX_Y_AMPLITUDE_SCALER);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FLYBACK_CYCLE:
		{
			setFlybackCycle(static_cast<long>(param));
			ret = resetFreeRun = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_MINIMIZE_FLYBACK_CYCLES:
		{
			resetFreeRun = CheckNewValue<bool>(_minimizeFlybackCycles, ((static_cast<long> (param) > 0) ? true : false));
			ret = TRUE;
		}				
		break;
	case ICamera::PARAM_LSM_GALVO_ENABLE:
		{
			if ((param >= MIN_GALVO_ENABLE) && (param <= MAX_GALVO_ENABLE))
			{
				resetFreeRun = CheckNewValue<long>(_galvoEnable, static_cast<long> (param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_GALVO_ENABLE %d outside range %d to %d",static_cast<long> (param), MIN_GALVO_ENABLE,MAX_GALVO_ENABLE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE1:
		{
			if ((param >= MIN_INPUTRANGE) && (param <= MAX_INPUTRANGE))
			{
				_inputRangeChannel[0] = static_cast<long> (param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_INPUTRANGE1 %d outside range %d to %d",static_cast<long> (param), MIN_INPUTRANGE,MAX_INPUTRANGE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE2:
		{
			if ((param >= MIN_INPUTRANGE) && (param <= MAX_INPUTRANGE))
			{
				_inputRangeChannel[1] = static_cast<long> (param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_INPUTRANGE2 %d outside range %d to %d",static_cast<long> (param), MIN_INPUTRANGE,MAX_INPUTRANGE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE3:
		{
			ret = TRUE;	
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE4:
		{
			ret = TRUE;	

		}
		break;
	case ICamera::PARAM_LSM_CLOCKSOURCE:
		{
			_clockSource = DEFAULT_CLOCKSOURCE;
			ret = TRUE;		
		}
		break;

	case ICamera::PARAM_LSM_INTERNALCLOCKRATE:
		{
			if ((param >= MIN_INTERNALCLOCKRATE) && (param <= MAX_INTERNALCLOCKRATE))
			{
				resetFreeRun = CheckNewValue<long>(_clockRateInternal, static_cast<long> (param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_INTERNALCLOCKRATE %d outside range %d to %d", static_cast<long>(param), MIN_INTERNALCLOCKRATE,MAX_INTERNALCLOCKRATE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;    
	case ICamera::PARAM_LSM_EXTERNALCLOCKRATE:
		{
			if ((param >= MIN_EXTCLOCKRATE) && (param <= MAX_EXTCLOCKRATE))
			{
				resetFreeRun = CheckNewValue<long>(_clockRateExternal, static_cast<long> (param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_EXTERNALCLOCKRATE %d outside range %d to %d", static_cast<long>(param), MIN_EXTCLOCKRATE,MAX_EXTCLOCKRATE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_CHANNEL:
		{
			if ((param >= MIN_CHANNEL) && (param <= _maxChannel))
			{
				resetFreeRun = CheckNewValue<long>(_channel, static_cast<long> (param));
				_minDwellTime = (_channel > 8) ? 2.0 : 1.0;
				resetFreeRun = CheckNewValue<double>(_dwellTime, max(_minDwellTime, _dwellTime));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_CHANNEL %d outside range %d to %d",static_cast<long> (param), MIN_CHANNEL,_maxChannel);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_TRIGGER_MODE:
		{
			if ((static_cast<long> (param) >= MIN_TRIGGER_MODE) && (static_cast<long> (param) <= MAX_TRIGGER_MODE))
			{
				resetFreeRun = CheckNewValue<long>(_triggerMode, static_cast<long>(param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_TRIGGER_MODE %d outside range %d to %d", static_cast<long> (param),MIN_TRIGGER_MODE,MAX_TRIGGER_MODE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			if (param >= 1)
			{
				resetFreeRun = CheckNewValue<long>(_frameCount, static_cast<long>(param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_MULTI_FRAME_COUNT %d outside range %d to %d",static_cast<long> (param), 1,INT_MAX);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;

	case ICamera::PARAM_LSM_AVERAGEMODE:
		{
			if ((param >= AverageMode::NO_AVERAGE) && (param <= AverageMode::FRM_CUMULATIVE_MOVING))
			{
				resetFreeRun = CheckNewValue<long>(_averageMode, static_cast<long> (param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_AVERAGEMODE %d outside range %d to %d",static_cast<long> (param), NO_AVERAGE,FRM_CUMULATIVE_MOVING);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;

	case ICamera::PARAM_LSM_AVERAGENUM:
		{
			if ((param >= MIN_AVERAGENUM) && (param <= MAX_AVERAGENUM))
			{
				resetFreeRun = CheckNewValue<long>(_averageNum, static_cast<long> (param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_AVERAGENUM %d outside range %d to %d",static_cast<long> (param), MIN_AVERAGENUM,MAX_AVERAGENUM);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;

	case ICamera::PARAM_TRIGGER_TIMEOUT_SEC:
		{
			if ((param >= MIN_TRIGGER_TIMEOUT) && (param <= MAX_TRIGGER_TIMEOUT))
			{
				_triggerWaitTimeout = static_cast<long> (param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_TRIGGER_TIMEOUT_SEC %d outside range %d to %d",static_cast<long> (param), MIN_TRIGGER_TIMEOUT,MAX_TRIGGER_TIMEOUT);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;

	case ICamera::PARAM_ENABLE_FRAME_TRIGGER_WITH_HW_TRIG:
		{
			if ((param >= MIN_ENABLE_FRAME_TRIGGER) && (param <= MAX_ENABLE_FRAME_TRIGGER))
			{
				_frameTriggerEnableWithHWTrig = static_cast<long> (param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_ENABLE_FRAME_TRIGGER_WITH_HW_TRIG %d outside range %d to %d",static_cast<long> (param), MIN_ENABLE_FRAME_TRIGGER,MAX_ENABLE_FRAME_TRIGGER);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;

	case ICamera::PARAM_LSM_DWELL_TIME:
		{
			double dwellTimeActualMin = CalculateMinimumDwellTime(_fieldSize, _pixelX, 2*static_cast<long>(_galvoRetraceTime), _field2Theta, static_cast<long>(_maxGalvoOpticalAngle));
			if ((param >= _minDwellTime) && (param <= MAX_DWELL_TIME))
			{
				double dwellParam = param;
				if(dwellParam < dwellTimeActualMin && FALSE == _ggSuperUserMode)
				{
					// If the dwell time passed is below the calculated minimum, set it to the minimum allowed value
					dwellParam = _minDwellTime + DWELL_TIME_STEP * ceil((dwellTimeActualMin - _minDwellTime) / DWELL_TIME_STEP);
				}
				resetFreeRun = CheckNewValue<double>(_dwellTime, dwellParam);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_DWELL_TIME %d.%d outside range %0.1f to 20",static_cast<long> (param),static_cast<long>(10 * (param - static_cast<long> (param))),dwellTimeActualMin);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		} 
		break;

	case ICamera::PARAM_LSM_GALVO_RASTERANGLE:
		{
			if ((param >= MIN_RASTERANGLE) && (param <= MAX_RASTERANGLE))
			{
				resetFreeRun = CheckNewValue<double>(_rasterAngle, param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_GALVO_RASTERANGLE %d outside range %d to %d",static_cast<long> (param), MIN_RASTERANGLE, MAX_RASTERANGLE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		} 
		break;

	case ICamera::PARAM_LSM_GALVO_LINEDUTY:
		{
			if ((param >= MIN_FORWARD_LINE_DUTY) && (param <= MAX_FORWARD_LINE_DUTY))
			{
				resetFreeRun = CheckNewValue<double>(_galvoForwardLineDuty, param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_GALVO_LINEDUTY %d outside range %d to %d",static_cast<long> (param), MIN_FORWARD_LINE_DUTY, MAX_FORWARD_LINE_DUTY);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		} 
		break;

	case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
		{
			resetFreeRun = CheckNewValue<long>(_forceSettingsUpdate, static_cast<long>(param));
			ret = TRUE;
		}
		break;

	case ICamera::PARAM_LSM_Y_COMMUNICATION_ENABLE:
		{
			if ((param >= MIN_Y_CHANNEL_ENABLE) && (param <= MAX_Y_CHANNEL_ENABLE))
			{
				resetFreeRun = CheckNewValue<long>(_yChannelEnable, ((MAX_GALVO_ENABLE == _galvoEnable) ? MAX_Y_CHANNEL_ENABLE : static_cast<long>(param)));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_Y_COMMUNICATION_ENABLE %d outside range %d to %d",static_cast<long> (param), MIN_Y_CHANNEL_ENABLE,MAX_Y_CHANNEL_ENABLE);
				LogMessage(_errMsg,ERROR_EVENT);
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

			if((param >= 0) && (param <= 100))
			{		
				switch(_pockelsResponseType[index])
				{
				case 0:
					{
						//linearize the sine wave response of the pockels cell
						dVal = acos(1 - static_cast<double>(Constants::AREA_UNDER_CURVE) * param/100.0)/PI;
					}
					break;
				case 1:
					{
						//linear response
						dVal = param/100.0;
					}
					break;
				}
				resetFreeRun = CheckNewValue<double>(_pockelsPowerLevel[index], dVal);
				ret = TRUE;

				//the scanner is in centering mode. allow the power to be changed instantaneously
				if(ScanMode::CENTER == _scanMode)
				{
					return MovePockelsToPowerLevel(index);
				}
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE %d outside range %d to %d",static_cast<long> (param), 0,100);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;

	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2:index=2;break;
			case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3:index=3;break;
			}

			if((param >= 0) && (param <= MAX_POCKELS_LINE_BLANKING_PERCENTAGE))
			{
				resetFreeRun = CheckNewValue<double>(_pockelsLineBlankingPercentage[index], param/100.0);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"MAX_POCKELS_LINE_BLANKING_PERCENTAGE %d outside range %d to %d",static_cast<long> (param), 0,MAX_POCKELS_LINE_BLANKING_PERCENTAGE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0:
		{
			resetFreeRun = CheckNewValue<long>(_pockelsResponseType[0], static_cast<long>(param));
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_1:
		{
			resetFreeRun = CheckNewValue<long>(_pockelsResponseType[1], static_cast<long>(param));
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_2:
		{
			resetFreeRun = CheckNewValue<long>(_pockelsResponseType[2], static_cast<long>(param));
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_3:
		{
			resetFreeRun = CheckNewValue<long>(_pockelsResponseType[3], static_cast<long>(param));
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION:
		{
			if((param >= 0) && (param <= 1))
			{
				resetFreeRun = CheckNewValue<double>(_verticalScanDirection, ((0 == static_cast<long>(param)) ? 1.0 : -1.0));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_VERTICAL_SCAN_DIRECTION %d outside range %d to %d",static_cast<long> (param), 0,1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_HORIZONTAL_FLIP:
		{
			if((param >= 0) && (param <= 1))
			{
				resetFreeRun = CheckNewValue<long>(_horizontalFlip, static_cast<long> (param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_HORIZONTAL_FLIP %d outside range %d to %d",static_cast<long> (param), 0,1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FINE_OFFSET_X:
		{
			if((param >= -1.0) && (param <= 1.0))
			{
				resetFreeRun = CheckNewValue<double>(_fineOffset[0], param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_FINE_OFFSET_X %f outside range %d to %d", param, -1, 1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FINE_OFFSET_Y:
		{
			if((param >= -1.0) && (param <= 1.0))
			{
				resetFreeRun = CheckNewValue<double>(_fineOffset[1], param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_FINE_OFFSET_Y %f outside range %d to %d", param, - 1, 1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FINE_OFFSET_X2:
		{
			if((param >= -1.0) && (param <= 1.0))
			{
				resetFreeRun = CheckNewValue<double>(_fineOffset2[0], param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_FINE_OFFSET_X2 %f outside range %d to %d", param, -1, 1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FINE_OFFSET_Y2:
		{
			if((param >= -1.0) && (param <= 1.0))
			{
				resetFreeRun = CheckNewValue<double>(_fineOffset2[1], param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_FINE_OFFSET_Y2 %f outside range %d to %d", param, -1, 1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_X:
		{
			if((param >= .8) && (param <= 1.2))
			{
				resetFreeRun = CheckNewValue<double>(_fineFieldSizeScaleX, param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_FINE_FIELD_SIZE_SCALE_X %f outside range %d to %d", param, .8, 1.2);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y:
		{
			if((param >= .8) && (param <= 1.2))
			{
				resetFreeRun = CheckNewValue<double>(_fineFieldSizeScaleY, param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y %f outside range %d to %d", param, .8, 1.2);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_CENTER_WITH_OFFSET:
		{
			if((param >= 0) && (param <= 1))
			{
				resetFreeRun = CheckNewValue<long>(_centerWithOffsets, static_cast<long>(param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_CENTER_WITH_OFFSET %d outside range %d to %d",static_cast<long> (param), 0, 1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_DATAMAP_MODE:
		{
			if((param >= ICamera::FIRST_MAPPING_MODE) && (param < ICamera::LAST_MAPPING_MODE))
			{
				resetFreeRun = CheckNewValue<long>(_dataMapMode, static_cast<long>(param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_DATAMAP_MODE %d outside range %d to %d",static_cast<long> (param), ICamera::FIRST_MAPPING_MODE,ICamera::LAST_MAPPING_MODE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_DMA_BUFFER_COUNT:
		{
			if((param>=2) && (param <= MAX_DMABUFNUM))
			{
				resetFreeRun = CheckNewValue<long>(_dMABufferCount, static_cast<long>(param));
				ret = TRUE;
			}
		}
		break;	
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_1:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				resetFreeRun = CheckNewValue<long>(_channelPolarity[0], static_cast<long>(param));
				ret = TRUE;
			}
		}
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_2:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				resetFreeRun = CheckNewValue<long>(_channelPolarity[1], static_cast<long>(param));
				ret = TRUE;
			}
		}
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_3:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				resetFreeRun = CheckNewValue<long>(_channelPolarity[2], static_cast<long>(param));
				ret = TRUE;
			}
		}
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_4:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				resetFreeRun = CheckNewValue<long>(_channelPolarity[3], static_cast<long>(param));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_0:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_1:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_2:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_2:index=2;break;
			case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_3:index=3;break;
			}

			if(1 == static_cast<long>(param))
			{
				ret = FindPockelsMinMax(index);
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_POCKELS_FIND_MIN_MAX failed");
				LogMessage(_errMsg,ERROR_EVENT);
			}
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
			_pockelsMinVoltage[index] = param;
			ret = TRUE;
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
			_pockelsMaxVoltage[index] = param;
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_SCANAREA_ANGLE:
		{
			double angle = param * PI / 180; // convert angle from degrees to radians
			if (MIN_SCANAREA_ANGLE <= angle && MAX_SCANAREA_ANGLE >= angle)
			{
				resetFreeRun = CheckNewValue<double>(_scanAreaAngle, angle);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_SCANAEREA_ANGLE %f outside range %f to %f",param, MIN_SCANAREA_ANGLE,MAX_SCANAREA_ANGLE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_STOP_ACQUISITION:
		{
			StopHardwareWaits();
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF:
		{
			if((FALSE == param) || (TRUE == param))
			{
				resetFreeRun = CheckNewValue<long>(_useReferenceForPockelsOutput, static_cast<long>(param));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS:
		{
			if((PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE <= param) || (PreCaptureStatus::PRECAPTURE_DONE >= param))
			{
				_precaptureStatus = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_INTERLEAVE_SCAN:
		{
			if ((0 <= param) && (1 >= param))
			{
				long lVal = 0;
				switch ((ICamera::LSMAreaMode)_areaMode)
				{
				case ICamera::LINE:
				case ICamera::POLYLINE:
					lVal = 0;
					break;
				default:
					lVal = (2 < _pixelY) ? static_cast<long>(param) : 0;
					break;
				}
				_interleaveScan = lVal;
				CloseThread();
				_behaviorPtr->SetParam(BehaviorProp::INTERLEAVE_BUF, _interleaveScan);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_X:
		{
			if((param >= _highResOffsetMinMax[0]) && (param <= _highResOffsetMinMax[1]))
			{
				resetFreeRun = CheckNewValue<double>(_highResOffset[0], param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_HIGHRES_OFFSET_X %f outside range %d to %d", param, _highResOffsetMinMax[0],_highResOffsetMinMax[1]);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_Y:
		{
			if((param >= _highResOffsetMinMax[0]) && (param <= _highResOffsetMinMax[1]))
			{
				resetFreeRun = CheckNewValue<double>(_highResOffset[1], param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_HIGHRES_OFFSET_Y %f outside range %d to %d", param, _highResOffsetMinMax[0],_highResOffsetMinMax[1]);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_X2:
		{
			if((param >= _highResOffsetMinMax[0]) && (param <= _highResOffsetMinMax[1]))
			{
				resetFreeRun = CheckNewValue<double>(_highResOffset2[0], param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_HIGHRES_OFFSET_X2 %f outside range %d to %d", param, _highResOffsetMinMax[0],_highResOffsetMinMax[1]);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_HIGHRES_OFFSET_Y2:
		{
			if((param >= _highResOffsetMinMax[0]) && (param <= _highResOffsetMinMax[1]))
			{
				resetFreeRun = CheckNewValue<double>(_highResOffset2[1], param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_HIGHRES_OFFSET_Y2 %f outside range %d to %d", param, _highResOffsetMinMax[0],_highResOffsetMinMax[1]);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_GG_SUPER_USER:
		{
			if (FALSE == param || TRUE == param)
			{
				_ggSuperUserMode = static_cast<long>(param); 
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_GG_SUPER_USER %d outside range %d to %d",static_cast<long>(param), FALSE, TRUE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS:
		{
			if (MIN_TB_LINE_SCAN_TIME <= param && MAX_TB_LINE_SCAN_TIME >= param)
			{
				if (TRUE == _timeBasedLineScanEnabled && (ICamera::LSMAreaMode::RECTANGLE == _areaMode || ICamera::LSMAreaMode::POLYLINE == _areaMode))
				{
					_timebasedLSTimeMS = static_cast<double>(floor(param + 0.5));
					double lineTimeMS = (_pixelX * _dwellTime + 2 * _galvoRetraceTime) / MS_TO_SEC;
					lineTimeMS = (ScanMode::TWO_WAY_SCAN == (ScanMode)_scanMode) ? lineTimeMS : 2 * lineTimeMS;
					long numberOfLines = static_cast<long>(floor(_timebasedLSTimeMS / lineTimeMS + 0.5));

					//make sure that when it is in twoway mode we are acquireing at least 2 lines, and that
					//we are acquiring a multiple of two
					if (numberOfLines % 2 != 0 && (ScanMode::TWO_WAY_SCAN == (ScanMode)_scanMode))
					{
						if (numberOfLines < 2)
						{
							numberOfLines = 2;
						}
						else if (_pixelY > numberOfLines)
						{
							--numberOfLines;
						}
						else
						{
							++numberOfLines;
							if (floor(numberOfLines * lineTimeMS + 0.5) > MAX_TB_LINE_SCAN_TIME)
							{
								numberOfLines -= 2;
							}
						}
					}

					_pixelY = numberOfLines;
					if (MAX_TIMED_BASED_SCAN_PIXEL_Y < _pixelY)
					{
						_pixelY = MAX_TIMED_BASED_SCAN_PIXEL_Y;
					}

					_timebasedLSTimeMS = floor(_pixelY * lineTimeMS + 0.5);

					SetParam(ICamera::PARAM_LSM_INTERLEAVE_SCAN, 0.0);	//no interleave in line scan
				}
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, _MAX_PATH, L"PARAM_LSM_TB_LINE_SCAN_TIME_MS %d outside range %d to %d", static_cast<double>(param), MIN_TB_LINE_SCAN_TIME, MAX_TB_LINE_SCAN_TIME);
				LogMessage(_errMsg, ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN:
		{
			if (FALSE == param || TRUE == param)
			{
				_timeBasedLineScanEnabled = static_cast<long>(param); 
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_TIME_BASED_LINE_SCAN %d outside range %d to %d",static_cast<long>(param), FALSE, TRUE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_IS_LIVE:
		{
			if (FALSE == param || TRUE == param)
			{
				_isLiveScan = static_cast<long>(param); 
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_IS_LIVE %d outside range %d to %d",static_cast<long>(param), FALSE, TRUE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	default:
		{
			StringCbPrintfW(_errMsg,_MAX_PATH,L"Parameter (%d) not implemented",paramID);
			LogMessage(_errMsg,ERROR_EVENT);
		}
		break;
	}
	//if free run, allow the params to be changed before finish current frame 
	if((resetFreeRun) && (ICamera::SW_FREE_RUN_MODE == _triggerMode))
	{
		CloseThread();
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI Restart for PARAM ID %d with value %f", paramID, param);
		LogMessage(_errMsg,VERBOSE_EVENT);
	}
	return ret;
}

long ThorLSMCam::SetParamBuffer(const long paramID, char * pBuffer, long size)
{
	long ret = TRUE;
	return ret;
}

long ThorLSMCam::SetParamString(const long paramID, wchar_t * str)
{
	long ret = FALSE;

	switch (paramID)
	{
	case ICamera::PARAM_LSM_WAVEFORM_PATH_NAME:
		{
			_waveformPathName = std::wstring(str);
			SAFE_DELETE_MEMORY (_pGalvoStartPos);
			ret = TRUE;
		}
		break;
	}

	return ret;
}

long ThorLSMCam::SetAction(ActionType actionType)
{
	long ret = TRUE;
	switch (actionType)
	{
	case MOVE_GALVO_TO_PARK:
		MoveGalvoToParkPosition();
		break;
	case MOVE_GALVO_TO_START:
		MoveGalvoToStart();
		break;
	case MOVE_GALVO_TO_CENTER:
		MoveGalvoToCenter();
		break;
	case MOVE_POCKEL_TO_PARK:
		MovePockelsToParkPosition();
		break;
	case INITIAL_PROPERTIES:
		return InitialImageProperties();
		break;
	case PERSIST_PROPERTIES:
	case PREFLIGHT_PROPERTIES:
		PersistImageProperties();
		break;
	case SETUP_CHECK:
		return SetupCheck();
	case SETUP_BOARD:
		return SetupBoards();
	case START_PROTOCOL:
		return StartProtocol();
	case SETUP_TASK_MASTER_CLOCK:
		return (DAQmxSuccess == SetupTaskMasterClock()) ? TRUE : FALSE;
	case SETUP_TASK_MASTER_GALVO:
		return (DAQmxSuccess == SetupTaskMasterGalvo()) ? TRUE : FALSE;
	case SETUP_TASK_MASTER_POCKEL:
		return (DAQmxSuccess == SetupTaskMasterPockel()) ? TRUE : FALSE;
	case SETUP_TASK_MASTER_DIGITAL:
		//open bleach shutter at setup, and set cycle complementary high
		//before set digital task due to shared NI task
		TogglePulseToDigitalLine(_taskHandleDO1, _bleachShutterLine, 1, TogglePulseMode::ToggleHigh, _bleachShutterIdle[0]);
		TogglePulseToDigitalLine(_taskHandleDO1, _bleachCycleInverse, 1, TogglePulseMode::ToggleHigh);
		return (DAQmxSuccess == SetupTaskMasterDigital()) ? TRUE : FALSE;
	case SETUP_CLOCK_MASTER_CLOCK:
		return (DAQmxSuccess == SetupClockMasterClock()) ? TRUE : FALSE;
	case SETUP_CLOCK_MASTER_GALVO:
		return (DAQmxSuccess == SetupClockMasterGalvo()) ? TRUE : FALSE;
	case SETUP_CLOCK_MASTER_POCKEL:
		return (DAQmxSuccess == SetupClockMasterPockel()) ? TRUE : FALSE;
	case SETUP_CLOCK_MASTER_DIGITAL:
		switch(_imgPtyDll.triggerMode)
		{
		case ICamera::HW_SINGLE_FRAME:
		case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
			if(TRUE == _frameTriggerEnableWithHWTrig)
			{
				return (DAQmxSuccess == SetupClockMasterDigital()) ? TRUE : FALSE;
			}
			break;
		case ICamera::SW_FREE_RUN_MODE:
		case ICamera::SW_MULTI_FRAME:
		default:
			return (DAQmxSuccess == SetupClockMasterDigital()) ? TRUE : FALSE;
		}
	case SETUP_CLOCK_MASTER_TRIGGER:
		return (DAQmxSuccess == SetupFrameTriggerInput()) ? TRUE : FALSE;
	case BUILD_TASK_MASTER:
		ForceUpdateProperties();
		BuildTaskMaterDigital();						//determine digital line selections
		return TryBuildTaskMaster();
	case WRITE_TASK_MASTER_GALVO:
		TryWriteTaskMasterGalvoWaveform(FALSE);
		break;
	case WRITE_TASK_MASTER_POCKEL:
		TryWriteTaskMasterPockelWaveform(FALSE);
		break;
	case WRITE_TASK_MASTER_LINE:
		TryWriteTaskMasterLineWaveform(FALSE);
		break;
	case DONE_SETUP:
		ResetEvent(ThorLSMCam::_hStopAcquisition); 
		break;
	case READY_TO_START:
		ResetEvent(_hStopAcquisition);
		ResetEvent(_hThreadStopped);
		ThorLSMCam::_waveformTaskStatus = ICamera::STATUS_BUSY;
		break;
	case START_SCAN:
		return (DAQmxSuccess == SyncCustomWaveformOnOff(true)) ? TRUE : FALSE;
	case STOP_SCAN:
		WaveformModeFinished();
		break;
	case PROC_BUFFER_NO_AVG:
		return ProcessBufferNoAve();
	case PROC_BUFFER_FRM_CMA:
		return ProcessBufferFrmCMA();
	case DONE_PROC_ONE_BUFFER:
		//at cumulative moving average, the first frame is displayed normally, 
		//the first two frames if interleave scan
		if((!_imgPtyDll.interleaveScan) || (2 <= _bufCompleteID))
		{
			_1stSet_CMA_Frame = 0;
		}
		break;
	case PROC_BUFFER_FUNC1:
		return ProcessBufferFunc1();
	case PROC_BUFFER_FUNC2:
		return ProcessBufferFunc2();
	case PROC_BUFFER_FUNC3:		
		return ProcessBufferFunc3();
	default:
		break;
	}
	return ret;
}

long ThorLSMCam::SetActionWithParam(ActionType actionType, long paramVal)
{
	long ret = TRUE;

	switch (actionType)
	{
	case MOVE_POCKEL_TO_POWER_LEVEL:
		MovePockelsToPowerLevel(paramVal);
		break;
	case STATUS:
		_waveformTaskStatus = paramVal;
		break;
	case POSTFLIGHT_PROTOCOL:
		return PostflightProtocol(paramVal);
	default:
		break;
	}
	return ret;
}

/// <summary> Sets the current flyback cycle </summary>
/// <param name="flybackCycle"> The new value for flyback cycle </param>
void ThorLSMCam::setFlybackCycle(long flybackCycle)
{
	if (MAX_FLYBACK_CYCLE < flybackCycle || MAX_FLYBACK_TIME < getFlybackTime(flybackCycle))
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_FLYBACK_CYCLE %d outside range %d to %d, and above %f seconds",static_cast<long> (flybackCycle), getMinFlybackCycle(), MAX_FLYBACK_CYCLE,MAX_FLYBACK_TIME);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	else
	{
		long minFlybackCycle = getMinFlybackCycle();
		_rawFlybackCycle = (flybackCycle > minFlybackCycle ? flybackCycle : minFlybackCycle);
	}
}

void ThorLSMCam::SetStatusHandle(HANDLE handle)
{
	_hStatusHandle = handle;
}

