#include "stdafx.h"
#include "ThorDAQGalvoGalvo.h"
#include "thordaqGalvoGalvoSetupXML.h"

/************************************************************************************************
* @fn	long CThorDAQGalvoGalvo::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)

*
* @brief	Gets parameter information.
*
* @param	paramID				  	Identifier for the parameter.
* @param [in,out]	paramType	  	Type of the parameter.
* @param [in,out]	paramAvailable	The parameter available.
* @param [in,out]	paramReadOnly 	The parameter read only.
* @param [in,out]	paramMin	  	The parameter minimum.
* @param [in,out]	paramMax	  	The parameter maximum.
* @param [in,out]	paramDefault  	The parameter default.
*
* @return	The parameter information.
**************************************************************************************************/
long CThorDAQGalvoGalvo::GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault)
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
	case ICamera::PARAM_LSM_PIXEL_Y_MULTIPLE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = PIXEL_Y_MULTIPLE;
		paramMax = PIXEL_Y_MULTIPLE;
		paramDefault = PIXEL_Y_MULTIPLE;
		paramReadOnly = TRUE;
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
		paramMin = MIN_CHANNEL;         // at least one channel
		paramMax = MAX_CHANNEL;         // 0xf : all channels are selected
		paramDefault = MIN_CHANNEL;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_ALIGNMENT:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_ALIGNMENT / ALIGNMENT_MULTIPLIER;
		paramMax = MAX_ALIGNMENT / ALIGNMENT_MULTIPLIER;
		paramDefault = MIN_ALIGNMENT / ALIGNMENT_MULTIPLIER;
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
		paramMin = MIN_FLYBACK_CYCLE / 100000.0;
		paramMax = MAX_FLYBACK_CYCLE / 0.01;
		paramDefault = DEFAULT_FLYBACK_CYCLE / 10;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_MINIMIZE_FLYBACK_CYCLES:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_GALVO_ENABLE: //Vertical Galvo
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_GALVO_ENABLE;
		paramMax = MAX_GALVO_ENABLE;
		paramDefault = DEFAULT_GALVO_ENABLE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_CLOCKSOURCE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = INTERNAL_CLOCK;
		paramMax = EXTERNAL_CLOCK;
		paramDefault = INTERNAL_CLOCK;
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
		if (_imgAcqPty.scanMode == TWO_WAY_SCAN)         //two way scan
		{
			lineTotal = MAX_BACKLINE_COUNT + _imgAcqPty.pixelY / 2;
		}
		else
		{
			lineTotal = MAX_BACKLINE_COUNT + _imgAcqPty.pixelY;
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
		paramDefault = paramMin;
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
		paramType = ICamera::TYPE_STRING;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
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
	case ICamera::PARAM_LSM_FINE_OFFSET_X:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMin = -1.0;
		paramMax = 1.0;
		paramDefault = 0.0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_FINE_OFFSET_Y:
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
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMin = .8;
		paramMax = 1.2;
		paramDefault = 1.0;
		paramReadOnly = FALSE;
	}
	break;
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
	case ICamera::PARAM_LSM_DMA_BUFFER_COUNT:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_DMA_BUFFER_NUM;
		paramMax = MAX_DMA_BUFFER_NUM;
		paramDefault = DEFAULT_DMA_BUFFER_NUM;
		paramReadOnly = FALSE;
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
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_1:
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_2:
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
	case ICamera::PARAM_LSM_SCANAREA_ANGLE:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMin = MIN_SCANAREA_ANGLE * 180 / M_PI; //Convert to degrees
		paramMax = MAX_SCANAREA_ANGLE * 180 / M_PI; //Convert to degrees
		paramDefault = DEFAULT_SCANAREA_ANGLE * 180 / M_PI; //Convert to degrees
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 1;
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
	case ICamera::PARAM_LSM_POCKELS_MASK_WIDTH:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_PIXEL_X;
		paramMax = MAX_PIXEL_X;
		paramDefault = DEFAULT_PIXEL_X;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_3P_ENABLE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_FINE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 100;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_0:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_3P_ALIGNCOARSE;
		paramMax = MAX_3P_ALIGNCOARSE;
		paramDefault = DEFAULT_3P_ALIGNCOARSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_1:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_3P_ALIGNCOARSE;
		paramMax = MAX_3P_ALIGNCOARSE;
		paramDefault = DEFAULT_3P_ALIGNCOARSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_2:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_3P_ALIGNCOARSE;
		paramMax = MAX_3P_ALIGNCOARSE;
		paramDefault = DEFAULT_3P_ALIGNCOARSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_3:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_3P_ALIGNCOARSE;
		paramMax = MAX_3P_ALIGNCOARSE;
		paramDefault = DEFAULT_3P_ALIGNCOARSE;
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
	case ICamera::PARAM_LSM_3P_FIR_CHANNEL:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = MAX_CHANNEL_COUNT - 1;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_3P_FIR_INDEX:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = FIR_FILTER_COUNT - 1;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_3P_FIR_TAP_INDEX:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = FIR_FILTER_TAP_COUNT - 1;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_3P_FIR_TAP_VALUE:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMin = MIN_FIR_TAP_COEFFICIENT;
		paramMax = MAX_FIR_TAP_COEFFICIENT;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_3P_MANUAL_FIR1_CONTROL_ENABLE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_QUERY_EXTERNALCLOCKRATE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = TRUE;
		paramMax = TRUE;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_GG_TURNAROUNDTIME_US:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_TURN_AROUND_TIME_US;
		paramMax = MAX_TURN_AROUND_TIME_US;
		paramDefault = DEFAULT_TURN_AROUND_TIME_US;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_0:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_DIG_OFFSET;
		paramMax = MAX_DIG_OFFSET;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_1:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_DIG_OFFSET;
		paramMax = MAX_DIG_OFFSET;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_2:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_DIG_OFFSET;
		paramMax = MAX_DIG_OFFSET;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_3:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_DIG_OFFSET;
		paramMax = MAX_DIG_OFFSET;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_0:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_1:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_2:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_3:
	{
		paramType = ICamera::TYPE_BUFFER;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_0:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_1:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_2:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_3:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MASK_0:
	case ICamera::PARAM_LSM_POCKELS_MASK_1:
	case ICamera::PARAM_LSM_POCKELS_MASK_2:
	case ICamera::PARAM_LSM_POCKELS_MASK_3:
	{
		paramType = ICamera::TYPE_BUFFER;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 0;
		paramDefault = 0;
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
		paramMin = MIN_DWELL_TIME;
		paramDefault = MIN_DWELL_TIME;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_WAVEFORM_DRIVER_TYPE:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMax = WaveformDriverType::WaveformDriverLast;
		paramMin = WaveformDriverType::WaveformDriverFirst;
		paramDefault = WaveformDriverType::WaveformDriver_ThorDAQ;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_NUMBER_OF_PLANES:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMax = MAX_NUMBER_OF_PLANES;
		paramMin = MIN_NUMBER_OF_PLANES;
		paramDefault = MIN_NUMBER_OF_PLANES;
		paramReadOnly = FALSE;
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
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_POWER_RAMP_ENABLE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMax = TRUE;
		paramMin = FALSE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_POWER_RAMP_NUM_FRAMES:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMax = MAX_NUMBER_OF_POWER_RAMP_FRAMES;
		paramMin = MIN_NUMBER_OF_POWER_RAMP_FRAMES;
		paramDefault = MIN_NUMBER_OF_POWER_RAMP_FRAMES;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_POWER_RAMP_NUM_FLYBACK_FRAMES:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMax = MAX_NUMBER_OF_POWER_RAMP_FLYBACK_FRAMES;
		paramMin = MIN_NUMBER_OF_POWER_RAMP_FLYBACK_FRAMES;
		paramDefault = DEFAULT_NUMBER_OF_POWER_RAMP_FLYBACK_FRAMES;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_POWER_RAMP_MODE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMax = PowerRampMode::LAST_POWER_RAMP_MODE - 1;
		paramMin = PowerRampMode::FIRST_POWER_RAMP_MODE;
		paramDefault = PowerRampMode::POWER_RAMP_MODE_CONTINUOUS;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_POWER_RAMP_PERCENTAGE_BUFFER:
	{
		paramType = ICamera::TYPE_BUFFER;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_FAST_ONEWAY_MODE_ENABLE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMax = TRUE;
		paramMin = FALSE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_EXTERNAL_CLOCK_PHASE_OFFSET:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_EXTERNAL_CLOCK_PHASE_OFFSET;
		paramMax = MAX_EXTERNAL_CLOCK_PHASE_OFFSET;
		paramDefault = MIN_EXTERNAL_CLOCK_PHASE_OFFSET;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_GG_ACQUIRE_DURING_TURAROUND:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMax = TRUE;
		paramMin = FALSE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_3P_LUT_OFFSET:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMax = 65535;
		paramMin = -65535;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_DAQ_NAME:
	{
		paramType = ICamera::TYPE_STRING;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 0;
		paramDefault = 0;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_DAQ_PART_NUMBER:
	{
		paramType = ICamera::TYPE_STRING;
		paramAvailable = FALSE;
		paramMin = 0;
		paramMax = 0;
		paramDefault = 0;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_DAQ_FW_VER:
	{
		paramType = ICamera::TYPE_STRING;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 0;
		paramDefault = 0;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_DAQ_DRIVER_VER:
	{
		paramType = ICamera::TYPE_STRING;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 0;
		paramDefault = 0;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_LOW_FREQ_TRIG_BOARD_FW_VER:
	{
		paramType = ICamera::TYPE_STRING;
		paramAvailable = _lowFreqTrigBoardInfo.boardConnected;
		paramMin = 0;
		paramMax = 0;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_LOW_FREQ_TRIG_BOARD_CPLD_VER:
	{
		paramType = ICamera::TYPE_STRING;
		paramAvailable = _lowFreqTrigBoardInfo.boardConnected;
		paramMin = 0;
		paramMax = 0;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_0:
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_1:
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_2:
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_3:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMin = MIN_POCKELS_DELAY_US;
		paramMax = MAX_POCKELS_DELAY_US;
		paramDefault = DEFAULT_POCKELS_DELAY_US;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS_ENABLE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMax = TRUE;
		paramMin = FALSE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS0_PHASE:
	case ICamera::PARAM_3P_REVERB_DDS1_PHASE:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMin = 1;
		paramMax = 360;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS0_AMPLITUDE:
	case ICamera::PARAM_3P_REVERB_DDS1_AMPLITUDE:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMin = -10;
		paramMax = 0;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_SELECTED_IMAGING_GG:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_SELECTED_STIM_GG:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_SECONDARY_GG_AVAILABLE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_RESEARCH_CAMERA_0:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMax = TRUE;
		paramMin = FALSE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_RESEARCH_CAMERA_1:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMax = 100000;
		paramMin = 0;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_3P_ENABLE_DOWNSAMPLING_RATE_CHANGE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_3P_DOWNSAMPLING_RATE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 70;
		paramMax = 4095;
		paramDefault = 2100;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_LINE_AVERAGING_ENABLE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_LINE_AVERAGING_NUMBER:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_LINE_AVERAGING_NUMBER;
		paramMax = MAX_LINE_AVERAGING_NUMBER;
		paramDefault = DEFAULT_LINE_AVERAGING_NUMBE;
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

long CThorDAQGalvoGalvo::SetParam(const long paramID, const double param)
{
	long ret = FALSE;
	bool alreadyUpdating = false;
	if (_updatingParam)
	{
		alreadyUpdating = true;
	}

	_updatingParam = true;

	switch (paramID)
	{
	case ICamera::PARAM_LSM_SCANMODE:
	{
		if ((param >= MIN_SCANMODE) && (param <= MAX_SCANMODE))
		{
			_imgAcqPty.scanMode = static_cast<long> (param);

			SetParam(ICamera::PARAM_LSM_LINE_AVERAGING_NUMBER, _imgAcqPty.lineAveragingNumber);

			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_SCANMODE %d outside range %d to %d", static_cast<long> (param), MIN_SCANMODE, MAX_SCANMODE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_AREAMODE:
	{
		if ((param >= MIN_AREAMODE) && (param <= MAX_AREAMODE))
		{
			_imgAcqPty.areaMode = static_cast<long> (param);

			switch ((ICamera::LSMAreaMode)_imgAcqPty.areaMode)
			{
			case ICamera::LSMAreaMode::SQUARE:
				SetParam(ICamera::PARAM_LSM_PIXEL_Y, _imgAcqPty.pixelX);
				break;
			case ICamera::LSMAreaMode::RECTANGLE:
				if (2 >= _imgAcqPty.pixelY)	//line scan, switch between two-way or one-way
				{
					long pY = (ScanMode::TWO_WAY_SCAN == (ScanMode)_imgAcqPty.scanMode) ? 2 : 1;
					SetParam(ICamera::PARAM_LSM_PIXEL_Y, pY);
				}
				break;
			case ICamera::LSMAreaMode::POLYLINE:
			case ICamera::LSMAreaMode::LINE:
				SetParam(ICamera::PARAM_LSM_PIXEL_Y, 1);
				SetParam(ICamera::PARAM_LSM_INTERLEAVE_SCAN, 0.0);
				break;
			}
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_AREAMODE %d outside range %d to %d", static_cast<long> (param), MIN_AREAMODE, MAX_AREAMODE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_PIXEL_X:
	{
		if ((param >= MIN_PIXEL_X) && (param <= MAX_PIXEL_X))
		{
			if (_imgAcqPty.lineAveragingEnable == FALSE ||
				_imgAcqPty.lineAveragingNumber * param <= MAX_PIXEL_X || ICamera::SQUARE != _imgAcqPty.areaMode)
			{
				if (static_cast<long>(param) % PIXEL_X_MULTIPLE == 0)
				{
					_imgAcqPty.pixelX = static_cast<long>(param);

					//min dwell time is partly dependent on pixel X, after updating pixel X,
					//make sure the current dwell time is still acceptable, otherwise it will get increased
					//to an acceptable number
					SetParam(PARAM_LSM_DWELL_TIME, _imgAcqPty.dwellTime);
					if (ICamera::SQUARE == _imgAcqPty.areaMode)
					{
						_imgAcqPty.pixelY = _imgAcqPty.pixelX;
					}
					ret = TRUE;
				}
				else
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_PIXEL_X must be a multiple of 4");
					LogMessage(_errMsg, ERROR_EVENT);
				}
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_PIXEL_X %d outside range %d to %d", static_cast<long> (param), MIN_PIXEL_X, static_cast<long>(MAX_PIXEL_X / _imgAcqPty.lineAveragingNumber));
				LogMessage(_errMsg, ERROR_EVENT);
			}
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_PIXEL_X %d outside range %d to %d", static_cast<long> (param), MIN_PIXEL_X, static_cast<long>(MAX_PIXEL_X));
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_PIXEL_Y:
	{
		if (_imgAcqPty.lineAveragingEnable == FALSE || _imgAcqPty.lineAveragingNumber * param <= MAX_PIXEL_Y)
		{
			if (ICamera::SQUARE == _imgAcqPty.areaMode)
			{
				_imgAcqPty.pixelY = _imgAcqPty.pixelX;
				ret = TRUE;
			}
			else if (ICamera::LINE == _imgAcqPty.areaMode)
			{
				if ((param > 1) && (param < 1))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_PIXEL_Y must equal 1 in AreaMode LINE");
					LogMessage(_errMsg, ERROR_EVENT);
				}
				else
				{
					_imgAcqPty.pixelY = static_cast<long>(param);
					ret = TRUE;
				}
			}
			else
			{
				if ((param >= MIN_PIXEL_Y) && (param <= MAX_PIXEL_Y))
				{

					if (static_cast<long>(param) % PIXEL_Y_MULTIPLE == 0 ||
						1 == static_cast<long>(param) ||
						2 == static_cast<long>(param))
					{
						if (static_cast<long>(param) > _imgAcqPty.pixelX)
						{
							StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_PIXEL_Y cannot be greater than PARAM_LSM_PIXEL_X");
							LogMessage(_errMsg, ERROR_EVENT);
						}
						else
						{
							_imgAcqPty.pixelY = static_cast<long>(param);

							ret = TRUE;
						}
					}
					else
					{
						StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_PIXEL_Y must be a multiple of 4");
						LogMessage(_errMsg, ERROR_EVENT);
					}
				}
				else
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_PIXEL_Y %d outside range %d to %d", static_cast<long> (param), MIN_PIXEL_Y, static_cast<long>(MAX_PIXEL_Y));
					LogMessage(_errMsg, ERROR_EVENT);
				}
			}
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_PIXEL_Y %d outside range %d to %d", static_cast<long> (param), MIN_PIXEL_Y, static_cast<long>(MAX_PIXEL_Y / _imgAcqPty.lineAveragingNumber));
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_FIELD_SIZE:
	{
		if ((param >= _fieldSizeMin) && (param <= _fieldSizeMax))
		{
			long originalFieldSize = _imgAcqPty.fieldSize;
			double originalDwell = _imgAcqPty.dwellTime;
			_imgAcqPty.fieldSize = static_cast<long>(param);

			double fieldSizeParam = param;
			SetParam(PARAM_LSM_DWELL_TIME, _imgAcqPty.dwellTime);

			double galvoRetraceTime = _imgAcqPty.turnAroundTimeUS / 2;

			double paddedAmplitude = fieldSizeParam * galvoRetraceTime * 4 / (_imgAcqPty.pixelX * _imgAcqPty.dwellTime * 2 * M_PI);
			double fieldX_angle = (fieldSizeParam + paddedAmplitude * 2) * _field2Theta;

			double paddedAmplitudeY = fieldSizeParam * galvoRetraceTime * 4 / (_imgAcqPty.pixelY * _imgAcqPty.dwellTime * 2 * M_PI);
			double fieldY_angle = (fieldSizeParam + paddedAmplitudeY * 2) * _field2Theta;

			StringCbPrintfW(_errMsg, MSG_SIZE, L"fieldX angle %d.%d", static_cast<long> (fieldX_angle), static_cast<long>(1000 * (fieldX_angle - static_cast<long> (fieldX_angle))));
			LogMessage(_errMsg, VERBOSE_EVENT);

			double theta = (double)fieldSizeParam * _field2Theta;
			double offsetX = abs(((double)_imgAcqPty.offsetX * _field2Theta) * _theta2Volts + _imgAcqPty.fineOffsetX); //horizontal galvo offset
			double maxVoltsX = offsetX + fieldX_angle / 2.0;

			double amplitudeY = theta * _theta2Volts * (double)_imgAcqPty.pixelY / (double)_imgAcqPty.pixelX;
			amplitudeY = (_imgAcqPty.yAmplitudeScaler / 100.0) * amplitudeY * _imgAcqPty.fineFieldSizeScaleY; // Vertical galvo amplitude
			double offsetY = ((double)_imgAcqPty.verticalScanDirection * _imgAcqPty.offsetY * _field2Theta) * _theta2Volts + _imgAcqPty.fineOffsetY;// Vertical galvo offset
			double maxVoltsY = offsetY + amplitudeY / 2.0;
			if ((maxVoltsX <= (MAX_GALVO_VOLTAGE) && maxVoltsY <= (MAX_GALVO_VOLTAGE)) || TRUE == _ggSuperUserMode)
			{
				ret = TRUE;
			}
			else
			{
				_imgAcqPty.fieldSize = originalFieldSize;
				SetParam(PARAM_LSM_DWELL_TIME, originalDwell);
			}
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_FIELD_SIZE %d outside range %d to %d", static_cast<long> (param), _fieldSizeMin, _fieldSizeMax);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_OFFSET_X:
	{
		if ((param >= -(_fieldSizeMax - _imgAcqPty.fieldSize) / 2) && (param <= (_fieldSizeMax - _imgAcqPty.fieldSize) / 2))
		{
			double offset = abs(((double)param * _field2Theta) * _theta2Volts + _imgAcqPty.fineOffsetX); //horizontal galvo offset
			double theta = (double)_imgAcqPty.fieldSize * _field2Theta;
			double amplitude = theta * _theta2Volts * _imgAcqPty.fineFieldSizeScaleX; // horizontal galvo amplitude

			double galvoRetraceTime = _imgAcqPty.turnAroundTimeUS / 2;

			double paddedAmplitude = (double)_imgAcqPty.fieldSize * galvoRetraceTime * 4 / (_imgAcqPty.pixelX * _imgAcqPty.dwellTime * 2 * M_PI);
			double fieldX_angle = ((double)_imgAcqPty.fieldSize + paddedAmplitude * 2) * _field2Theta;

			double maxVolts = offset + fieldX_angle / 2.0;
			//only allow when the offset is within the range of the analog output or in super-user mode
			if (maxVolts <= MAX_GALVO_VOLTAGE || TRUE == _ggSuperUserMode)
			{
				_imgAcqPty.offsetX = static_cast<long>(param);
				ret = TRUE;
			}
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_OFFSET_X %d outside range %d to %d", static_cast<long> (param), -(_fieldSizeMax - _imgAcqPty.fieldSize) / 2, (_fieldSizeMax - _imgAcqPty.fieldSize) / 2);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_OFFSET_Y:
	{
		if ((param >= -(_fieldSizeMax - _imgAcqPty.fieldSize) / 2) && (param <= (_fieldSizeMax - _imgAcqPty.fieldSize) / 2))
		{
			double theta = (double)_imgAcqPty.fieldSize * _field2Theta;
			double amplitudeY = theta * _theta2Volts * (double)_imgAcqPty.pixelY / (double)_imgAcqPty.pixelX;
			amplitudeY = (_imgAcqPty.yAmplitudeScaler / 100.0) * amplitudeY * _imgAcqPty.fineFieldSizeScaleY; // Vertical galvo amplitude

			double galvoRetraceTime = _imgAcqPty.turnAroundTimeUS / 2;

			double paddedAmplitudeY = (double)_imgAcqPty.fieldSize * galvoRetraceTime * 4 / (_imgAcqPty.pixelX * _imgAcqPty.dwellTime * 2 * M_PI);
			double fieldY_angle = ((double)_imgAcqPty.fieldSize + paddedAmplitudeY * 2) * _field2Theta;

			double offsetY = ((double)_imgAcqPty.verticalScanDirection * param * _field2Theta) * _theta2Volts + _imgAcqPty.fineOffsetY;// Vertical galvo offset
			double maxVoltsY = offsetY + fieldY_angle / 2.0;
			//only allow when the offset is within the range of the analog output or in super-user mode
			if (maxVoltsY <= MAX_GALVO_VOLTAGE || TRUE == _ggSuperUserMode)
			{
				_imgAcqPty.offsetY = static_cast<long>(param);
				ret = TRUE;
			}
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_OFFSET_Y %d outside range %d to %d", static_cast<long> (param), -(_fieldSizeMax - _imgAcqPty.fieldSize) / 2, (_fieldSizeMax - _imgAcqPty.fieldSize) / 2);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_ALIGNMENT:
	{
		long loadedShiftValue = 0;
		if (_imgAcqPty.threePhotonModeEnable == TRUE)
		{
			loadedShiftValue = _shiftArray[_ratio - 1];
		}
		else
		{
			loadedShiftValue = _shiftArray[static_cast<long>(_imgAcqPty.dwellTime * 5 - 2)];
		}
		//Thordaq takes an unsigned short number for alignment
		if ((param + loadedShiftValue >= 0) && (param + loadedShiftValue <= MAX_ALIGNMENT / ALIGNMENT_MULTIPLIER))
		{
			_imgAcqPty.alignmentForField = static_cast<long>(param * ALIGNMENT_MULTIPLIER);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_ALIGNMENT %d outside range %lf to %lf", static_cast<long>(param), MIN_ALIGNMENT / ALIGNMENT_MULTIPLIER, ((MAX_ALIGNMENT + 1) / ALIGNMENT_MULTIPLIER) - 1);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_Y_AMPLITUDE_SCALER:
	{
		if ((param >= MIN_Y_AMPLITUDE_SCALER) && (param <= MAX_Y_AMPLITUDE_SCALER))
		{
			_imgAcqPty.yAmplitudeScaler = static_cast<long>(param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_Y_AMPLITUDE_SCALER %d outside range %d to %d", static_cast<long> (param), MIN_Y_AMPLITUDE_SCALER, MAX_Y_AMPLITUDE_SCALER);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_FLYBACK_CYCLE:
	{
		SetFlybackCycle(static_cast<long>(param));
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_MINIMIZE_FLYBACK_CYCLES:
	{
		if (param > 0)
		{
			_imgAcqPty.minimizeFlybackCycles = true;
		}
		else
		{
			_imgAcqPty.minimizeFlybackCycles = false;
		}
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_GALVO_ENABLE:
	{
		if ((param >= MIN_GALVO_ENABLE) && (param <= MAX_GALVO_ENABLE))
		{
			_imgAcqPty.galvoEnable = static_cast<long>(param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_GALVO_ENABLE %d outside range %d to %d", static_cast<long> (param), MIN_GALVO_ENABLE, MAX_GALVO_ENABLE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_CLOCKSOURCE:
	{
		if ((param >= INTERNAL_CLOCK) && (param <= EXTERNAL_CLOCK))
		{
			if (_imgAcqPty.clockSource != static_cast<long> (param))
			{
				_imgAcqPty.clockSource = static_cast<long> (param);
				if (_deviceNum != 0)
				{
					int32 error = 0, retVal = 0;
					if (_imgAcqPty.clockSource == INTERNAL_CLOCK)
					{
						ThordaqErrChk(L"ThorDAQAPISetClockSource", retVal = ThorDAQAPISetClockSource(_DAQDeviceIndex, CLOCK_SOURCE::INTERNAL_80MHZ_REF));
					}
					else
					{
						//need to set twice to ensure it works correctly (especially for 3P).
						//TODO: this problem should be addressed in the FW
						ThordaqErrChk(L"ThorDAQAPISetClockSource", retVal = ThorDAQAPISetClockSource(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE));
						ThordaqErrChk(L"ThorDAQAPISetClockSource", retVal = ThorDAQAPISetClockSource(_DAQDeviceIndex, CLOCK_SOURCE::INTERNAL_80MHZ_REF));
						ThordaqErrChk(L"ThorDAQAPISetClockSource", retVal = ThorDAQAPISetClockSource(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE));
					}
				}
				// When clock source changes, the board internally changes the ADC gain to 21dB. The GUI needs to reflect this change
				for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
				{
					_imgAcqPty.inputRange[i] = 3;
				}
			}
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_CLOCKSOURCE %d outside range %d to %d", static_cast<long>(param), INTERNAL_CLOCK, EXTERNAL_CLOCK);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_INTERNALCLOCKRATE:
	{
		if ((param >= MIN_INTERNALCLOCKRATE) && (param <= MAX_INTERNALCLOCKRATE))
		{
			_imgAcqPty.clockRateInternal = static_cast<long> (param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_INTERNALCLOCKRATE %d outside range %d to %d", static_cast<long>(param), MIN_INTERNALCLOCKRATE, MAX_INTERNALCLOCKRATE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_EXTERNALCLOCKRATE:
	{
		if ((param >= MIN_EXTCLOCKRATE) && (param <= MAX_EXTCLOCKRATE))
		{
			if (TRUE == _useExternalBoxFrequency3P)
			{
				_imgAcqPty.clockRateExternal = static_cast<int> (param);
			}
			//if (_deviceNum != 0)
			//{
			//	if (_imgAcqPty.clockSource == EXTERNAL_CLOCK)
			//	{
			//		ThorDAQAPISetClockSource(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE, 2 * _imgAcqPty.clockRateExternal);
			//	}
			//	
			//}
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_EXTERNALCLOCKRATE %d outside range %d to %d", static_cast<int>(param), MIN_EXTCLOCKRATE, MAX_EXTCLOCKRATE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL:
	{
		if ((param >= MIN_CHANNEL) && (param <= MAX_CHANNEL))
		{
			_imgAcqPty.channel = static_cast<long> (param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_CHANNEL %d outside range %d to %d", static_cast<long> (param), MIN_CHANNEL, MAX_CHANNEL);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_TRIGGER_MODE:
	{
		if ((static_cast<long> (param) >= MIN_TRIGGER_MODE) && (static_cast<long> (param) <= MAX_TRIGGER_MODE))
		{
			_imgAcqPty.triggerMode = static_cast<long> (param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_TRIGGER_MODE %d outside range %d to %d", static_cast<long> (param), MIN_TRIGGER_MODE, MAX_TRIGGER_MODE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_MULTI_FRAME_COUNT:
	{
		if (param >= 1)
		{
			_imgAcqPty.numFrame = static_cast<long> (param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_MULTI_FRAME_COUNT %d outside range %d to %d", static_cast<long> (param), 1, INT_MAX);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;

	case ICamera::PARAM_LSM_AVERAGEMODE:
	{
		if ((param >= NO_AVERAGE) && (param <= FRM_CUMULATIVE_MOVING))
		{
			_imgAcqPty.averageMode = static_cast<long> (param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_AVERAGEMODE %d outside range %d to %d", static_cast<long> (param), NO_AVERAGE, FRM_CUMULATIVE_MOVING);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;

	case ICamera::PARAM_LSM_AVERAGENUM:
	{
		if ((param >= MIN_AVERAGENUM) && (param <= MAX_AVERAGENUM))
		{
			_imgAcqPty.averageNum = static_cast<long> (param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_AVERAGENUM %d outside range %d to %d", static_cast<long> (param), MIN_AVERAGENUM, MAX_AVERAGENUM);
			LogMessage(_errMsg, ERROR_EVENT);
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
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_TRIGGER_TIMEOUT_SEC %d outside range %d to %d", static_cast<long> (param), MIN_TRIGGER_TIMEOUT, MAX_TRIGGER_TIMEOUT);
			LogMessage(_errMsg, ERROR_EVENT);
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
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_ENABLE_FRAME_TRIGGER_WITH_HW_TRIG %d outside range %d to %d", static_cast<long> (param), MIN_ENABLE_FRAME_TRIGGER, MAX_ENABLE_FRAME_TRIGGER);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;

	case ICamera::PARAM_LSM_DWELL_TIME:
	{
		if ((param >= MIN_DWELL_TIME) && (param <= MAX_DWELL_TIME))
		{
			const double CLOCK_PRECISION_200MHZ = 5.0; //5ns
			const double COMPARISON_PRECISION = 0.001;
			double dwellParam = param;
			double minDwell = CalculateMinimumDwellTime(_imgAcqPty.fieldSize, _imgAcqPty.fineFieldSizeScaleX, _imgAcqPty.pixelX, _imgAcqPty.turnAroundTimeUS, _field2Theta, MAX_GALVO_OPTICAL_ANGLE, _maxAngularVelocityRadPerSec, _maxAngularAccelerationRadPerSecSq, MIN_DWELL_TIME, MAX_DWELL_TIME, DWELL_TIME_STEP);

			if (minDwell > MAX_DWELL_TIME)
			{
				minDwell = MAX_DWELL_TIME;
			}
			if (_imgAcqPty.threePhotonModeEnable == TRUE)
			{
				double tempDwellns = 1000000.0 / static_cast<double>(_imgAcqPty.clockRateExternal);

				double result = 1000 * tempDwellns + static_cast<double>(CLOCK_PRECISION_200MHZ) / 2.0;
				tempDwellns = static_cast<long>(result - std::fmod(result, CLOCK_PRECISION_200MHZ));
				//convert to seconds
				tempDwellns = tempDwellns / 1000;

				if (dwellParam > tempDwellns)
				{
					_ratio = static_cast<long>(ceil(dwellParam / tempDwellns - 0.5));
					_minDwellTime = _dwellTimeStep = tempDwellns;
					tempDwellns *= _ratio;
				}
				else
				{
					_ratio = 1;
					_minDwellTime = _dwellTimeStep = tempDwellns;
				}

				if (tempDwellns < minDwell && FALSE == _ggSuperUserMode)
				{
					// If the dwell time passed is below the calculated minimum, set it to the minimum allowed value
					_ratio = static_cast<long>(ceil(minDwell / _dwellTimeStep));
					tempDwellns = _dwellTimeStep * _ratio;
				}

				result = 1000 * tempDwellns + static_cast<double>(CLOCK_PRECISION_200MHZ) / 2.0;
				tempDwellns = static_cast<long>(result - std::fmod(result, CLOCK_PRECISION_200MHZ));
				//convert to seconds
				_displayedDwellTime = _imgAcqPty.dwellTime = tempDwellns / 1000;
			}
			else
			{
				_minDwellTime = MIN_DWELL_TIME;
				if (dwellParam < minDwell && FALSE == _ggSuperUserMode)
				{
					// If the dwell time passed is below the calculated minimum, set it to the minimum allowed value
					dwellParam = minDwell;// MIN_DWELL_TIME + DWELL_TIME_STEP * ceil((minDwell - MIN_DWELL_TIME) / DWELL_TIME_STEP);
				}
				//convert to nanoseconds
				double tempDwellns = dwellParam * 1000;
				double result = tempDwellns + static_cast<double>(CLOCK_PRECISION_200MHZ) / 2.0;
				tempDwellns = static_cast<long>(result - std::fmod(result, CLOCK_PRECISION_200MHZ));

				//convert back microsecons
				_displayedDwellTime = _imgAcqPty.dwellTime = tempDwellns / 1000;//static_cast<double>(param);			
			}
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_DWELL_TIME %d.%d outside range %f to 20", static_cast<long> (param), static_cast<long>(10 * (param - static_cast<long> (param))), _minDwellTime);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;

	case ICamera::PARAM_LSM_GALVO_RASTERANGLE:
	{
		if ((param >= MIN_RASTERANGLE) && (param <= MAX_RASTERANGLE))
		{
			_imgAcqPty.rasterAngle = static_cast<double> (param);
			ret = true;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_GALVO_RASTERANGLE %d outside range %d to %d", static_cast<long> (param), MIN_RASTERANGLE, MAX_RASTERANGLE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;

	case ICamera::PARAM_LSM_GALVO_LINEDUTY:
	{
		if ((param >= MIN_FORWARD_LINE_DUTY) && (param <= MAX_FORWARD_LINE_DUTY))
		{
			_imgAcqPty.galvoForwardLineDuty = static_cast<double> (param);
			ret = true;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_GALVO_LINEDUTY %d outside range %lf to %d", static_cast<long> (param), MIN_FORWARD_LINE_DUTY, MAX_FORWARD_LINE_DUTY);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;

	case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
	{
		_forceSettingsUpdate = static_cast<long>(param);
		ret = TRUE;
	}
	break;

	case ICamera::PARAM_LSM_Y_COMMUNICATION_ENABLE:
	{
		if ((param >= MIN_Y_CHANNEL_ENABLE) && (param <= MAX_Y_CHANNEL_ENABLE))
		{
			if (MAX_GALVO_ENABLE == _imgAcqPty.galvoEnable)
			{
				_imgAcqPty.yChannelEnable = MAX_Y_CHANNEL_ENABLE;
			}
			else
			{
				_imgAcqPty.yChannelEnable = static_cast<long>(param);
			}
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_Y_COMMUNICATION_ENABLE %d outside range %d to %d", static_cast<long> (param), MIN_Y_CHANNEL_ENABLE, MAX_Y_CHANNEL_ENABLE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;

	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:
	{
		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:index = 3; break;
		}

		if ((param >= 0) && (param <= 100))
		{
			switch (_pockelsResponseType[index])
			{
			case static_cast<long>(PockelsResponseType::SINE_RESPONSE):
			{
				//linearize the sine wave response of the pockels cell
				_imgAcqPty.pockelPty.pockelsPowerLevel[index] = acos(1 - static_cast<double>(Constants::AREA_UNDER_CURVE) * param / 100.0) / M_PI;
			}
			break;
			case static_cast<long>(PockelsResponseType::LINEAR_RESPONSE):
			{
				//linear response
				_imgAcqPty.pockelPty.pockelsPowerLevel[index] = param / 100.0;
			}
			break;
			}

			ret = TRUE;

			//the scanner is in centering mode. allow the power to be changed instantaneously
			if (_imgAcqPty.scanMode == ScanMode::CENTER)
			{
				MovePockelsToPowerLevel(index, &_imgAcqPty.pockelPty);
			}
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE %d outside range %d to %d", static_cast<long> (param), 0, 100);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;

	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3:
	{
		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3:index = 3; break;
		}

		if ((param >= 0) && (param <= MAX_POCKELS_LINE_BLANKING_PERCENTAGE))
		{
			_imgAcqPty.pockelPty.pockelsLineBlankingPercentage[index] = param / 100.0;
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"MAX_POCKELS_LINE_BLANKING_PERCENTAGE %d outside range %d to %d", static_cast<long> (param), 0, MAX_POCKELS_LINE_BLANKING_PERCENTAGE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0:
	{
		_pockelsResponseType[0] = static_cast<long>(param);
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_1:
	{
		_pockelsResponseType[1] = static_cast<long>(param);
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_2:
	{
		_pockelsResponseType[2] = static_cast<long>(param);
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_3:
	{
		_pockelsResponseType[3] = static_cast<long>(param);
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION:
	{
		if ((param >= 0) && (param <= 1))
		{
			if (0 == static_cast<long>(param))
			{
				_imgAcqPty.verticalScanDirection = 1.0;
			}
			else
			{
				_imgAcqPty.verticalScanDirection = -1.0;
			}
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_VERTICAL_SCAN_DIRECTION %d outside range %d to %d", static_cast<long> (param), 0, 1);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_HORIZONTAL_FLIP:
	{
		if ((param >= 0) && (param <= 1))
		{
			_imgAcqPty.horizontalFlip = static_cast<long>(param) == 0 ? 1 : 0;
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_HORIZONTAL FLIP %d outside range %d to %d", static_cast<long> (param), 0, 1);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_FINE_OFFSET_X:
	{
		if ((param >= -1.0) && (param <= 1.0))
		{
			_imgAcqPty.fineOffsetX = param;
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_FINE_OFFSET_X %d outside range %lf to %lf", static_cast<long> (param), -0.1, 0.1);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_FINE_OFFSET_Y:
	{
		if ((param >= -1.0) && (param <= 1.0))
		{
			_imgAcqPty.fineOffsetY = param;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_FINE_OFFSET_Y %d outside range %lf to %lf", static_cast<long> (param), -0.1, 0.1);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_X:
	{
		if ((param >= .8) && (param <= 1.2))
		{
			_imgAcqPty.fineFieldSizeScaleX = param;
			SetParam(PARAM_LSM_DWELL_TIME, _imgAcqPty.dwellTime);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_FINE_FIELD_SIZE_SCALE_X %d outside range %lf to %lf", static_cast<long> (param), 0.9, 1.1);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y:
	{
		if ((param >= .8) && (param <= 1.2))
		{
			_imgAcqPty.fineFieldSizeScaleY = param;
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y %d outside range %lf to %lf", static_cast<long> (param), 0.9, 1.2);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_PULSE_MULTIPLEXING_ENABLE:
	{
		_imgAcqPty.laserCoherentSamplingEnable = static_cast<long>(param);
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_EXTERNAL_CLOCK_PHASE_OFFSET:
	{
		if ((param >= MIN_EXTERNAL_CLOCK_PHASE_OFFSET) && (param <= MAX_EXTERNAL_CLOCK_PHASE_OFFSET))
		{
			_imgAcqPty.laserCoherentSamplingPhase = static_cast<long>(param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_PULSE_MULTIPLEXING_ENABLE %d outside range %d to %d", static_cast<long>(param), MIN_EXTERNAL_CLOCK_PHASE_OFFSET, MAX_EXTERNAL_CLOCK_PHASE_OFFSET);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_DATAMAP_MODE:
	{
		if ((param >= ICamera::FIRST_MAPPING_MODE) && (param < ICamera::LAST_MAPPING_MODE))
		{
			_imgAcqPty.dataMapMode = static_cast<long>(param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_DATAMAP_MODE %d outside range %d to %d", static_cast<long> (param), ICamera::FIRST_MAPPING_MODE, ICamera::LAST_MAPPING_MODE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_DMA_BUFFER_COUNT:
	{
		if ((param >= MIN_DMA_BUFFER_NUM) && (param <= MAX_DMA_BUFFER_NUM))
		{
			_imgAcqPty.dmaBufferCount = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_1:
	{
		if ((param >= POL_NEG) && (param <= POL_POS))
		{
			_imgAcqPty.channelPolarity[0] = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_2:
	{
		if ((param >= POL_NEG) && (param <= POL_POS))
		{
			_imgAcqPty.channelPolarity[1] = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_3:
	{
		if ((param >= POL_NEG) && (param <= POL_POS))
		{
			_imgAcqPty.channelPolarity[2] = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_4:
	{
		if ((param >= POL_NEG) && (param <= POL_POS))
		{
			_imgAcqPty.channelPolarity[3] = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY:
	{
		_imgAcqPty.rawSaveEnabledChannelOnly = static_cast<long>(param);
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_0:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_1:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_2:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_3:
	{
		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_3:index = 3; break;
		}

		if ((param >= 0) && (param <= 1))
		{
			_imgAcqPty.pockelPty.pockelsMaskEnable[index] = static_cast<long>(param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_POCKELS_MASK_ENABLE %d outside range %d to %d", static_cast<long> (param), 0, 1);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_0:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_1:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_2:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_3:
	{
		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_3:index = 3; break;
		}

		if ((param >= 0) && (param <= 1))
		{
			_imgAcqPty.pockelPty.pockelsMaskInvert[index] = static_cast<long>(param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_POCKELS_MASK_INVERT %d outside range %d to %d", static_cast<long> (param), 0, 1);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_0:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_1:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_2:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_3:
	{
		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_3:index = 3; break;
		}

		if (1 == static_cast<long>(param))
		{
			ret = FindPockelsMinMax(index, &_imgAcqPty.pockelPty);
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_POCKELS_FIND_MIN_MAX failed");
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:
	{
		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:index = 3; break;
		}
		_imgAcqPty.pockelPty.pockelsMinVoltage[index] = param;
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:
	{
		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:index = 3; break;
		}
		_imgAcqPty.pockelPty.pockelsMaxVoltage[index] = param;
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_SCANAREA_ANGLE:
	{
		double angle = param * M_PI / 180; // convert angle from degrees to radians
		if (MIN_SCANAREA_ANGLE <= angle && MAX_SCANAREA_ANGLE >= angle)
		{
			_imgAcqPty.scanAreaAngle = angle;
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_SCANAEREA_ANGLE %f outside range %f to %f", param, MIN_SCANAREA_ANGLE, MAX_SCANAREA_ANGLE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_STOP_ACQUISITION:
	{
		StopHardwareWaits();
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_3P_ENABLE:
	{
		if ((FALSE == param) || (TRUE == param))
		{
			int32 error = 0, retVal = 0;
			if (param)
			{
				if (_deviceNum != 0)
				{
					ULONG32 clock_rate = 0;
					ULONG32 clock_ref = 0;
					SetParam(ICamera::PARAM_LSM_CLOCKSOURCE, EXTERNAL_CLOCK);

					//ThordaqErrChk(L"ThorDAQAPISetThreePhotonEnable", retVal = ThorDAQAPISetThreePhotonEnable(_DAQDeviceIndex, false));

					//ThordaqErrChk(L"ThorDAQAPISetThreePhotonEnable", retVal = ThorDAQAPISetThreePhotonEnable(_DAQDeviceIndex, true));
					ThordaqErrChk(L"ThorDAQAPISetAllADCChannelsGain", retVal = ThorDAQAPISetAllADCChannelsGain(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE, _imgAcqPty.ADCGain, true));
					//ThordaqErrChk(L"ThorDAQAPISetThreePhotonEnable", retVal = ThorDAQAPISetThreePhotonEnable(_DAQDeviceIndex, false));
					ThordaqErrChk(L"ThorDAQAPISetThreePhotonEnable", retVal = ThorDAQAPISetThreePhotonEnable(_DAQDeviceIndex, true));

					//ThordaqErrChk(L"ThorDAQAPISetClockSource", retVal = ThorDAQAPISetClockSource(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE));

					//_imgAcqPty.clockSource = EXTERNAL_CLOCK;

					ThordaqErrChk(L"ThorDAQAPIMeasureExternClockRate", retVal = ThorDAQAPIMeasureExternClockRate(_DAQDeviceIndex, clock_rate, clock_ref, 1));

					if (retVal == THORDAQ_STATUS::STATUS_SUCCESSFUL)
					{
						_imgAcqPty.clockRateExternal = clock_rate;
						_imgAcqPty.maxSampleRate = static_cast<long>(clock_ref * 2.0);
					}
					// When clock source changes, the board internally changes the ADC gain to 21dB. The GUI needs to reflect this change
					for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
					{
						_imgAcqPty.inputRange[i] = 3;
					}
				}
			}
			else
			{
				ThordaqErrChk(L"ThorDAQAPISetThreePhotonEnable", retVal = ThorDAQAPISetThreePhotonEnable(_DAQDeviceIndex, false));

				SetParam(ICamera::PARAM_LSM_CLOCKSOURCE, INTERNAL_CLOCK);
				_imgAcqPty.maxSampleRate = DEFAULT_INTERNALCLOCKRATE;
				_minDwellTime = MIN_DWELL_TIME;
				_dwellTimeStep = DWELL_TIME_STEP;
			}
			_imgAcqPty.threePhotonModeEnable = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_FINE:
	{
		_imgAcqPty.laserCoherentSamplingPhase = static_cast<long>(param);
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_0:
	{
		if (_imgAcqPty.threePhotonModeEnable == TRUE && _imgAcqPty.clockRateExternal > 0 && param < (160.0 / (_imgAcqPty.clockRateExternal / 1000000.0) - 4.0))
		{
			_imgAcqPty.threePhotonModeAlignmentPhase[0] = static_cast<long>(param);

			//TODO: currently the FW is not working well when using this option. Check with Bill Radtke and test again with new FW
			int32 error = 0, retVal = 0;
			ThordaqErrChk(L"ThorDAQAPISetThreePhotonSampleOffset", retVal = ThorDAQAPISetThreePhotonSampleOffset(_DAQDeviceIndex, 0,  static_cast<UINT8>(_imgAcqPty.threePhotonModeAlignmentPhase[0])));
		}
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_1:
	{
		if (_imgAcqPty.threePhotonModeEnable == TRUE && _imgAcqPty.clockRateExternal > 0 && param < (160.0 / (_imgAcqPty.clockRateExternal / 1000000.0) - 4.0))
		{
			_imgAcqPty.threePhotonModeAlignmentPhase[1] = static_cast<long>(param);

			//TODO: currently the FW is not working well when using this option. Check with Bill Radtke and test again with new FW
			int32 error = 0, retVal = 0;
			ThordaqErrChk(L"ThorDAQAPISetThreePhotonSampleOffset", retVal = ThorDAQAPISetThreePhotonSampleOffset(_DAQDeviceIndex, 1, static_cast<UINT8>(_imgAcqPty.threePhotonModeAlignmentPhase[1])));
		}
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_2:
	{
		if (_imgAcqPty.threePhotonModeEnable == TRUE && _imgAcqPty.clockRateExternal > 0 && param < (160.0 / (_imgAcqPty.clockRateExternal / 1000000.0) - 4.0))
		{
			_imgAcqPty.threePhotonModeAlignmentPhase[2] = static_cast<long>(param);

			//TODO: currently the FW is not working well when using this option. Check with Bill Radtke and test again with new FW
			int32 error = 0, retVal = 0;
			ThordaqErrChk(L"ThorDAQAPISetThreePhotonSampleOffset", retVal = ThorDAQAPISetThreePhotonSampleOffset(_DAQDeviceIndex, 2, static_cast<UINT8>(_imgAcqPty.threePhotonModeAlignmentPhase[2])));
		}
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_3:
	{
		if (_imgAcqPty.threePhotonModeEnable == TRUE && _imgAcqPty.clockRateExternal > 0 && param < (160.0 / (_imgAcqPty.clockRateExternal / 1000000.0) - 4.0))
		{
			_imgAcqPty.threePhotonModeAlignmentPhase[3] = static_cast<long>(param);

			//TODO: currently the FW is not working well when using this option. Check with Bill Radtke and test again with new FW
			int32 error = 0, retVal = 0;
			ThordaqErrChk(L"ThorDAQAPISetThreePhotonSampleOffset", retVal = ThorDAQAPISetThreePhotonSampleOffset(_DAQDeviceIndex, 3, static_cast<UINT8>(_imgAcqPty.threePhotonModeAlignmentPhase[3])));
		}
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_INPUTRANGE1:
	{
		if (param >= MIN_INPUTRANGE && param <= MAX_INPUTRANGE)
		{
			_imgAcqPty.inputRange[0] = static_cast<long>(param);

			switch (_imgAcqPty.inputRange[0])
			{
			case 0:
				_imgAcqPty.ADCGain[0] = 0;
				break;
			default:
				// Start from higher dB. The lower the dB value is, the higher the intensity will be.
				// The slider step values are: 0 ->29dB -> 25dB -> 21dB -> 17dB -> 13dB -> 9dB -> 5dB -> 1dB
				_imgAcqPty.ADCGain[0] = ADC_GAIN::ADC_GAIN_LAST - _imgAcqPty.inputRange[0];
				break;
			}
			int32 error = 0, retVal = 0;
			//TODO: it should work without lying about it being external clock source, need to change the code so it works logically
			ThordaqErrChk(L"ThorDAQAPISetAllADCChannelsGain", retVal = ThorDAQAPISetAllADCChannelsGain(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE, _imgAcqPty.ADCGain, true));
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_INPUTRANGE2:
	{
		if (param >= MIN_INPUTRANGE && param <= MAX_INPUTRANGE)
		{

			_imgAcqPty.inputRange[1] = static_cast<long>(param);

			switch (_imgAcqPty.inputRange[1])
			{
			case 0:
				_imgAcqPty.ADCGain[1] = 0;
				break;
			default:
				// Start from higher dB. The lower the dB value is, the higher the intensity will be.
				// The slider step values are: 0 ->29dB -> 25dB -> 21dB -> 17dB -> 13dB -> 9dB -> 5dB -> 1dB
				_imgAcqPty.ADCGain[1] = ADC_GAIN::ADC_GAIN_LAST - _imgAcqPty.inputRange[1];
				break;
			}

			int32 error = 0, retVal = 0;
			ThordaqErrChk(L"ThorDAQAPISetAllADCChannelsGain", retVal = ThorDAQAPISetAllADCChannelsGain(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE, _imgAcqPty.ADCGain, true));

			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_INPUTRANGE3:
	{
		if (param >= MIN_INPUTRANGE && param <= MAX_INPUTRANGE)
		{
			_imgAcqPty.inputRange[2] = static_cast<long>(param);

			switch (_imgAcqPty.inputRange[2])
			{
			case 0:
				_imgAcqPty.ADCGain[2] = 0;
				break;
			default:
				// Start from higher dB. The lower the dB value is, the higher the intensity will be.
				// The slider step values are: 0 ->29dB -> 25dB -> 21dB -> 17dB -> 13dB -> 9dB -> 5dB -> 1dB
				_imgAcqPty.ADCGain[2] = ADC_GAIN::ADC_GAIN_LAST - _imgAcqPty.inputRange[2];
				break;
			}

			int32 error = 0, retVal = 0;
			ThordaqErrChk(L"ThorDAQAPISetAllADCChannelsGain", retVal = ThorDAQAPISetAllADCChannelsGain(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE, _imgAcqPty.ADCGain, true));

			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_INPUTRANGE4:
	{
		if (param >= MIN_INPUTRANGE && param <= MAX_INPUTRANGE)
		{
			_imgAcqPty.inputRange[3] = static_cast<long>(param);

			switch (_imgAcqPty.inputRange[3])
			{
			case 0:
				_imgAcqPty.ADCGain[3] = 0;
				break;
			default:
				// Start from higher dB. The lower the dB value is, the higher the intensity will be.
				// The slider step values are: 0 ->29dB -> 25dB -> 21dB -> 17dB -> 13dB -> 9dB -> 5dB -> 1dB
				_imgAcqPty.ADCGain[3] = ADC_GAIN::ADC_GAIN_LAST - _imgAcqPty.inputRange[3];
				break;
			}

			int32 error = 0, retVal = 0;
			ThordaqErrChk(L"ThorDAQAPISetAllADCChannelsGain", retVal = ThorDAQAPISetAllADCChannelsGain(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE, _imgAcqPty.ADCGain, true));

			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_3P_FIR_CHANNEL:
	{
		if (param >= 0 && param <= MAX_CHANNEL_COUNT)
		{
			_FIRFilterSelectedSettingChannel = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_3P_FIR_INDEX:
	{
		if (param >= 0 && param < FIR_FILTER_COUNT)
		{
			_FIRFilterSelectedIndex = static_cast<long>(param);
			ret = TRUE;
		}
		break;
	}
	case ICamera::PARAM_LSM_3P_FIR_TAP_INDEX:
	{
		if (param >= 0 && param < FIR_FILTER_TAP_COUNT)
		{
			_FIRFilterSelectedTapIndex = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_3P_FIR_TAP_VALUE:
	{
		if (param >= MIN_FIR_TAP_COEFFICIENT && param <= MAX_FIR_TAP_COEFFICIENT)
		{
			if (_FIRFilterSelectedIndex >= 0 && _FIRFilterSelectedIndex < FIR_FILTER_COUNT &&
				_FIRFilterSelectedTapIndex >= 0 && _FIRFilterSelectedTapIndex < FIR_FILTER_TAP_COUNT)
			{
				//currently not allowing individual channel control
				for (int i = 0; i < MAX_CHANNEL_COUNT; ++i)
				{
					_imgAcqPty.FIRFilters[_FIRFilterSelectedIndex][i][_FIRFilterSelectedTapIndex] = param;
				}
				ret = TRUE;
			}
		}
	}
	break;
	case ICamera::PARAM_LSM_3P_MANUAL_FIR1_CONTROL_ENABLE:
	{
		if (param == TRUE || param == FALSE)
		{
			_imgAcqPty.FIR1ManualControlenable = static_cast<long>(param);
			ret = TRUE;
		}
		break;
	}
	break;
	case ICamera::PARAM_LSM_QUERY_EXTERNALCLOCKRATE:
	{
		if (_imgAcqPty.clockSource == EXTERNAL_CLOCK)
		{
			if (_deviceNum != 0)
			{
				ULONG32 clock_rate = 0;
				ULONG32 clock_ref = 0;
				ULONG32 mode = _imgAcqPty.threePhotonModeEnable == TRUE ? 1 : 0;

				if (FALSE == _useExternalBoxFrequency3P)
				{
					int32 error = 0, retVal = 0;

					ThordaqErrChk(L"ThorDAQAPIMeasureExternClockRate", retVal = ThorDAQAPIMeasureExternClockRate(_DAQDeviceIndex, clock_rate, clock_ref, mode));

					if (retVal == THORDAQ_STATUS::STATUS_SUCCESSFUL)
					{
						_imgAcqPty.clockRateExternal = clock_rate;
						_imgAcqPty.maxSampleRate = static_cast<long>(clock_ref * 2.0);
					}
				}
			}
		}

		if (_imgAcqPty.threePhotonModeEnable == FALSE)
		{
			_imgAcqPty.maxSampleRate = MAX_EXTCLOCKRATE;
		}

		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_GG_TURNAROUNDTIME_US:
	{
		if (param >= MIN_TURN_AROUND_TIME_US && param <= MAX_TURN_AROUND_TIME_US)
		{
			_imgAcqPty.turnAroundTimeUS = static_cast<long>(param);

			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"PARAM_LSM_GG_TURNAROUNDTIME_US %d outside range %d to %d", static_cast<long> (param), MIN_TURN_AROUND_TIME_US, MAX_TURN_AROUND_TIME_US);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_0:
	{
		if (MIN_DIG_OFFSET <= param && MAX_DIG_OFFSET >= param)
		{
			int32 error = 0, retVal = 0;
			USHORT channelIndex = 0;

			_preDcOffset[channelIndex] = static_cast<short>(param);
			//Divide by 2 to keep the value from higher level consistent with 14-bit histogram
			ThordaqErrChk(L"ThorDAQAPISetDCOffsetPreFIR", retVal = ThorDAQAPISetDCOffsetPreFIR(_DAQDeviceIndex, static_cast<short>(ceil(_preDcOffset[channelIndex] / 2)), channelIndex));
			if (retVal == STATUS_SUCCESSFUL)
			{
				ret = TRUE;
			}
		}
		else
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"PARAM_LSM_DIG_OFFSET_0 %d outside range %d to %d", static_cast<long> (param), MIN_DIG_OFFSET, MAX_DIG_OFFSET);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_1:
	{
		if (MIN_DIG_OFFSET <= param && MAX_DIG_OFFSET >= param)
		{
			int32 error = 0, retVal = 0;
			USHORT channelIndex = 1;

			_preDcOffset[channelIndex] = static_cast<short>(param);
			//Divide by 2 to keep the value from higher level consistent with 14-bit histogram
			ThordaqErrChk(L"ThorDAQAPISetDCOffsetPreFIR", retVal = ThorDAQAPISetDCOffsetPreFIR(_DAQDeviceIndex, static_cast<short>(ceil(_preDcOffset[channelIndex] / 2)), channelIndex));
			if (retVal == STATUS_SUCCESSFUL)
			{
				ret = TRUE;
			}
		}
		else
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"PARAM_LSM_DIG_OFFSET_1 %d outside range %d to %d", static_cast<long> (param), MIN_DIG_OFFSET, MAX_DIG_OFFSET);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_2:
	{
		if (MIN_DIG_OFFSET <= param && MAX_DIG_OFFSET >= param)
		{
			int32 error = 0, retVal = 0;
			USHORT channelIndex = 2;

			_preDcOffset[channelIndex] = static_cast<short>(param);
			//Divide by 2 to keep the value from higher level consistent with 14-bit histogram
			ThordaqErrChk(L"ThorDAQAPISetDCOffsetPreFIR", retVal = ThorDAQAPISetDCOffsetPreFIR(_DAQDeviceIndex, static_cast<short>(ceil(_preDcOffset[channelIndex] / 2)), channelIndex));
			if (retVal == STATUS_SUCCESSFUL)
			{
				ret = TRUE;
			}
		}
		else
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"PARAM_LSM_DIG_OFFSET_2 %d outside range %d to %d", static_cast<long> (param), MIN_DIG_OFFSET, MAX_DIG_OFFSET);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_3:
	{
		if (MIN_DIG_OFFSET <= param && MAX_DIG_OFFSET >= param)
		{
			int32 error = 0, retVal = 0;
			USHORT channelIndex = 3;

			_preDcOffset[channelIndex] = static_cast<short>(param);
			//Divide by 2 to keep the value from higher level consistent with 14-bit histogram
			ThordaqErrChk(L"ThorDAQAPISetDCOffsetPreFIR", retVal = ThorDAQAPISetDCOffsetPreFIR(_DAQDeviceIndex, static_cast<short>(ceil(_preDcOffset[channelIndex] / 2)), channelIndex));
			if (retVal == STATUS_SUCCESSFUL)
			{
				ret = TRUE;
			}
		}
		else
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"PARAM_LSM_DIG_OFFSET_3 %d outside range %d to %d", static_cast<long> (param), MIN_DIG_OFFSET, MAX_DIG_OFFSET);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_GG_SUPER_USER:
	{
		if (FALSE == static_cast<long>(param) || TRUE == static_cast<long>(param))
		{
			_ggSuperUserMode = static_cast<long>(param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"PARAM_LSM_GG_SUPER_USER %d outside range %d to %d", static_cast<long>(param), FALSE, TRUE);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_NUMBER_OF_PLANES:
	{
		if (MIN_NUMBER_OF_PLANES <= static_cast<long>(param) && MAX_NUMBER_OF_PLANES >= static_cast<long>(param))
		{
			_imgAcqPty.numberOfPlanes = static_cast<long>(param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"PARAM_LSM_NUMBER_OF_PLANES %d outside range %d to %d", static_cast<long>(param), MIN_NUMBER_OF_PLANES, MAX_NUMBER_OF_PLANES);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS:
	{
		if ((PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE <= param) || (PreCaptureStatus::PRECAPTURE_DONE >= param))
		{
			_precaptureStatus = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_POWER_RAMP_ENABLE:
	{
		if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
		{
			_imgAcqPty.powerRampEnable = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_POWER_RAMP_NUM_FRAMES:
	{
		if (MIN_NUMBER_OF_POWER_RAMP_FRAMES <= static_cast<long>(param) && MAX_NUMBER_OF_POWER_RAMP_FRAMES >= static_cast<long>(param))
		{
			_imgAcqPty.powerRampNumFrames = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_POWER_RAMP_NUM_FLYBACK_FRAMES:
	{
		if (MIN_NUMBER_OF_POWER_RAMP_FLYBACK_FRAMES <= static_cast<long>(param) && MAX_NUMBER_OF_POWER_RAMP_FLYBACK_FRAMES >= static_cast<long>(param))
		{
			_imgAcqPty.powerRampNumFlybackFrames = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_POWER_RAMP_MODE:
	{
		if (PowerRampMode::FIRST_POWER_RAMP_MODE <= static_cast<long>(param) && PowerRampMode::LAST_POWER_RAMP_MODE > static_cast<long>(param))
		{
			_imgAcqPty.powerRampMode = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_FAST_ONEWAY_MODE_ENABLE:
	{
		if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
		{
			_imgAcqPty.fastOneWayEnable = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_GG_ACQUIRE_DURING_TURAROUND:
	{
		if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
		{
			_imgAcqPty.acquireDuringTurnAround = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_0:
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_1:
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_2:
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_3:
	{
		if (MIN_POCKELS_DELAY_US <= param && MAX_POCKELS_DELAY_US >= param)
		{
			long index = 0;

			switch (paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_0:index = 0; break;
			case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_1:index = 1; break;
			case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_2:index = 2; break;
			case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_3:index = 3; break;
			}
			_imgAcqPty.pockelPty.pockelsDelayUS[index] = param;
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_3P_LUT_OFFSET:
	{
		if (param >= -65535 && param <= 65535)
		{
			_imgAcqPty.sampleOffsetStartLUT3PTI = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS_ENABLE:
	{
		if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
		{
			_ddsClockEnable = TRUE == static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS0_PHASE:
	{
		if (param >= 1.0 && param <= 360.0)
		{
			_ddsClockPhase0 = param;

			int32 error = 0, retVal = 0;
			ThordaqErrChk(L"ThorDAQAPISetDDSClockPhase DDS0", retVal = ThorDAQAPISetDDSClockPhase(_DAQDeviceIndex, 0, _ddsClockPhase0));
			if (retVal == STATUS_SUCCESSFUL)
			{
				ret = TRUE;
			}
		}
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS1_PHASE:
	{
		if (param >= 1.0 && param <= 360.0)
		{
			_ddsClockPhase1 = param;
			int32 error = 0, retVal = 0;
			ThordaqErrChk(L"ThorDAQAPISetDDSClockPhase DDS1", retVal = ThorDAQAPISetDDSClockPhase(_DAQDeviceIndex, 1, _ddsClockPhase1));
			if (retVal == STATUS_SUCCESSFUL)
			{
				ret = TRUE;
			}
		}
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS0_AMPLITUDE:
	{
		if (param >= -10.0 && param <= 0.0)
		{
			_ddsClockAmplitude0 = param;

			int32 error = 0, retVal = 0;
			//channel 10 is DDS0, the amplitude is controlled by settings the park value
			ThordaqErrChk(L"ThorDAQAPISetDACParkValue DDS0", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, 10, _ddsClockAmplitude0));
			if (retVal == STATUS_SUCCESSFUL)
			{
				ret = TRUE;
			}
		}
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS1_AMPLITUDE:
	{
		if (param >= -10.0 && param <= 0.0)
		{
			_ddsClockAmplitude1 = param;

			int32 error = 0, retVal = 0;
			//channel 10 is DDS0, the amplitude is controlled by settings the park value
			ThordaqErrChk(L"ThorDAQAPISetDACParkValue DDS1", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, 11, _ddsClockAmplitude1));
			if (retVal == STATUS_SUCCESSFUL)
			{
				ret = TRUE;
			}
		}
	}
	break;
	case ICamera::PARAM_RESEARCH_CAMERA_0:
	{
		if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
		{
			_imgAcqPty.movingAverageFilterEnable = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_RESEARCH_CAMERA_1:
	{
		//if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
		{
			_imgAcqPty.movingAverageFilterMultiplier = param;
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_SELECTED_IMAGING_GG:
	{
		if (param >= 0 && param <= 1)
		{
			_imgAcqPty.selectedImagingGG = static_cast<long>(param);

			SetSelectedImagingAOs(_imgAcqPty.selectedImagingGG);
			ret = true;
		}
	}
	break;
	case ICamera::PARAM_LSM_SELECTED_STIM_GG:
	{
		if (param >= 0 && param <= 1)
		{
			_imgAcqPty.selectedStimGG = static_cast<long>(param);

			SetSelectedStimAOs(_imgAcqPty.selectedStimGG);
			ret = true;
		}
	}
	break;
	case ICamera::PARAM_3P_ENABLE_DOWNSAMPLING_RATE_CHANGE:
	{
		if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
		{
			_imgAcqPty.enableDownsamplingRateChange = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_3P_DOWNSAMPLING_RATE:
	{
		if (param >= 70 && param <= 4095)
		{
			_imgAcqPty.threePhotonDownsamplingRate = static_cast<long>(param);
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"PARAM_3P_DOWNSAMPLING_RATE %d outside range %d to %d", static_cast<long>(param), 70, 4095);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_LINE_AVERAGING_ENABLE:
	{
		if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
		{
			_imgAcqPty.lineAveragingEnable = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_LINE_AVERAGING_NUMBER:
	{
		if (param >= MIN_LINE_AVERAGING_NUMBER && param <= MAX_LINE_AVERAGING_NUMBER)
		{
			if (_imgAcqPty.scanMode == ScanMode::TWO_WAY_SCAN)
			{
				if (static_cast<long>(param) % 2 == 0)
				{
					_imgAcqPty.lineAveragingNumber = static_cast<long>(param);
				}
				else if (_imgAcqPty.lineAveragingNumber > param)
				{
					if (static_cast<long>(param) - 1 >= MIN_LINE_AVERAGING_NUMBER)
					{
						_imgAcqPty.lineAveragingNumber = static_cast<long>(param) - 1;
					}
					else
					{
						_imgAcqPty.lineAveragingNumber = MIN_LINE_AVERAGING_NUMBER;
					}
				}
				else
				{
					if (static_cast<long>(param) + 1 <= MAX_LINE_AVERAGING_NUMBER)
					{
						_imgAcqPty.lineAveragingNumber = static_cast<long>(param) + 1;
					}
					else
					{
						_imgAcqPty.lineAveragingNumber = MAX_LINE_AVERAGING_NUMBER;
					}
				}
			}
			else
			{
				_imgAcqPty.lineAveragingNumber = static_cast<long>(param);
			}

			if (_imgAcqPty.lineAveragingEnable == TRUE &&
				_imgAcqPty.pixelY * _imgAcqPty.lineAveragingNumber > MAX_PIXEL_Y)
			{
				long newPixels = MAX_PIXEL_Y / _imgAcqPty.lineAveragingNumber - ((MAX_PIXEL_Y / _imgAcqPty.lineAveragingNumber) % PIXEL_Y_MULTIPLE);
				if (_imgAcqPty.areaMode == LSMAreaMode::SQUARE)
				{

					SetParam(ICamera::PARAM_LSM_PIXEL_X, newPixels);
				}

				SetParam(ICamera::PARAM_LSM_PIXEL_Y, newPixels);
			}

			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"PARAM_LSM_LINE_AVERAGING_NUMBER %d outside range %d to %d", static_cast<long>(param), MIN_LINE_AVERAGING_NUMBER, MAX_LINE_AVERAGING_NUMBER);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	default:
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"TD GG SetParam Parameter (%d) not implemented", paramID);
		LogMessage(_errMsg, ERROR_EVENT);
	}
	break;
	}
	if (!alreadyUpdating)
	{
		bool oneWayLineScan = _imgAcqPty.pixelY == 1 && TWO_WAY_SCAN != _imgAcqPty.scanMode;

		long vSize = oneWayLineScan ? 2 : static_cast<USHORT>(_imgAcqPty.pixelY);

		ScanStruct scan_info = ScanStruct(vSize, _imgAcqPty.flybackCycle, 1);

		double dwell_time = _imgAcqPty.dwellTime / 1000000.0;
		double linetime = 0;
		double dac_rate = 0;
		ScanLineStruct scanLine = ScanLineStruct();
		bool useFastOneway = false;
		if (_imgAcqPty.threePhotonModeEnable)
		{
			if (FALSE == GetDACSamplesPerLine3P(&scanLine, &_imgAcqPty, dac_rate, dwell_time, linetime, oneWayLineScan, useFastOneway))
			{
				if (alreadyUpdating)
				{
					_updatingParam = false;
				}
				return FALSE;
			}
		}
		else
		{
			if (FALSE == GetDACSamplesPerLine(&scanLine, &_imgAcqPty, dac_rate, dwell_time, linetime, oneWayLineScan, useFastOneway))
			{
				if (alreadyUpdating)
				{
					_updatingParam = false;
				}
				return FALSE;
			}
		}
		long maxFlybackCycle = (long)(65535.0 / (linetime * dac_rate));
		//_flybackCycles = _imgAcqPty.flybackCycle = (long)min(_imgAcqPty.flybackCycle, maxFlybackCycle);

		double flybackTime = 2 * linetime * GetFlybackCycle();

		if (TWO_WAY_SCAN == _imgAcqPty.scanMode || useFastOneway)
		{
			_frameRate = 1.0 / (_scan_info.forward_lines * linetime * _scan_info.average_lines_num + flybackTime);
		}
		else
		{
			_frameRate = 1.0 / (_scan_info.forward_lines * 2 * linetime * _scan_info.average_lines_num + flybackTime); // One way is 2 linetimes for each Y pixel
		}

		//if free run, allow the params to be changed before finish current frame 
		if ((!_imgAcqPty.KeyPropertiesAreEqual(_imgAcqPty_Pre)) && ICamera::SW_FREE_RUN_MODE == _imgAcqPty.triggerMode && _imgAcqPty.scanMode != BLEACHING_SCAN)
		{
			int32 error = 0, retVal = 0;

			SetEvent(_hStopAcquisition);
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ThorDAQGalvoGalvo Restart for PARAM ID %d with value %f", paramID, param);
			LogMessage(_errMsg, VERBOSE_EVENT);

			if (_hExperimentThread)
			{
				ThordaqErrChk(L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
				if (WAIT_OBJECT_0 != WaitForSingleObject(_hThreadStopped, INFINITE))
				{
					StringCbPrintfW(_errMsg, _MAX_PATH, L"Failed to stop acquisition thread.");
					LogMessage(_errMsg, ERROR_EVENT);
				}
			}
		}
	}
	if (alreadyUpdating)
	{
		_updatingParam = false;
	}
	return ret;
}

/**********************************************************************************************//**
* @fn	long CThorDAQGalvoGalvo::GetParam(const long paramID, double &param)
*
* @brief	Gets parameter information.
*
* @param	paramID				  	Identifier for the parameter.
* @param [in,out]	param 			The parameter value.
*
* @return	A long.
**************************************************************************************************/
long CThorDAQGalvoGalvo::GetParam(const long paramID, double& param)
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
		param = _imgAcqPty.scanMode;
	}
	break;
	case ICamera::PARAM_LSM_AREAMODE:
	{
		param = _imgAcqPty.areaMode;
	}
	break;
	case ICamera::PARAM_LSM_PIXEL_X:
	{
		param = _imgAcqPty.pixelX;
	}
	break;
	case ICamera::PARAM_LSM_PIXEL_Y:
	{
		param = _imgAcqPty.pixelY;
	}
	break;
	case ICamera::PARAM_LSM_PIXEL_Y_MULTIPLE:
	{
		param = PIXEL_Y_MULTIPLE;
	}
	break;
	case ICamera::PARAM_LSM_FIELD_SIZE:
	{
		param = _imgAcqPty.fieldSize;
	}
	break;
	case ICamera::PARAM_LSM_ALIGNMENT:
	{
		param = _imgAcqPty.alignmentForField / ALIGNMENT_MULTIPLIER;
	}
	break;
	case ICamera::PARAM_LSM_Y_AMPLITUDE_SCALER:
	{
		param = _imgAcqPty.yAmplitudeScaler;
	}
	break;
	case ICamera::PARAM_LSM_FLYBACK_CYCLE:
	{
		param = GetFlybackCycle();
	}
	break;
	case ICamera::PARAM_LSM_FLYBACK_TIME:
	{
		param = GetFlybackTime(GetFlybackCycle());
	}
	break;
	case ICamera::PARAM_LSM_MINIMIZE_FLYBACK_CYCLES:
	{
		param = static_cast<long>(_imgAcqPty.minimizeFlybackCycles);
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL:
	{
		param = _imgAcqPty.channel;
	}
	break;
	case ICamera::PARAM_LSM_AVERAGEMODE:
	{
		param = _imgAcqPty.averageMode;
	}
	break;
	case ICamera::PARAM_LSM_AVERAGENUM:
	{
		param = _imgAcqPty.averageNum;
	}
	break;
	case ICamera::PARAM_LSM_OFFSET_X:
	{
		param = _imgAcqPty.offsetX;
	}
	break;
	case ICamera::PARAM_LSM_OFFSET_Y:
	{
		param = _imgAcqPty.offsetY;
	}
	break;
	case ICamera::PARAM_LSM_CLOCKSOURCE:
	{
		param = _imgAcqPty.clockSource;
	}
	break;
	case ICamera::PARAM_LSM_INTERNALCLOCKRATE:
	{
		param = _imgAcqPty.clockRateInternal;
	}
	break;
	case ICamera::PARAM_LSM_EXTERNALCLOCKRATE:
	{
		//if (_imgAcqPty.clockSource == EXTERNAL_CLOCK)
		//{
		//	if (_deviceNum != 0)
		//	{
		//		ULONG32 clock_rate = 0;
		//		ULONG32 clock_ref = 0;
		//		ULONG32 mode = _imgAcqPty.threePhotonModeEnable == TRUE ? 1 : 0;

		//		if(FALSE == _useExternalBoxFrequency3P)
		//		{			
		//			if( ThorDAQAPIMeasureExternClockRate(_DAQDeviceIndex, clock_rate,clock_ref, mode) == THORDAQ_STATUS::STATUS_SUCCESSFUL)
		//			{
		//				_imgAcqPty.clockRateExternal = clock_rate;
		//				_imgAcqPty.maxSampleRate = static_cast<long>(clock_ref * 2.0);
		//			}
		//		}
		//	}
		//}
		//
		//if (_imgAcqPty.threePhotonModeEnable == FALSE)
		//{
		//	_imgAcqPty.maxSampleRate = MAX_EXTCLOCKRATE;
		//}
		param = _imgAcqPty.clockRateExternal;
	}
	break;
	case ICamera::PARAM_TRIGGER_MODE:
	{
		param = _imgAcqPty.triggerMode;
	}
	break;
	case ICamera::PARAM_MULTI_FRAME_COUNT:
	{
		param = static_cast<double>(_imgAcqPty.numFrame);
	}
	break;
	case ICamera::PARAM_LSM_DMA_BUFFER_COUNT:
	{
		param = static_cast<double>(_imgAcqPty.dmaBufferCount);
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
		param = _displayedDwellTime;
	}
	break;
	case ICamera::PARAM_LSM_DWELL_TIME_STEP:
	{
		param = _dwellTimeStep;
	}
	break;
	case ICamera::PARAM_LSM_GALVO_ENABLE:
	{
		param = _imgAcqPty.galvoEnable;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_0:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_1:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_2:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_3:
	{
		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_3:index = 3; break;
		}

		param = _imgAcqPty.pockelPty.pockelsMaskEnable[index];
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_0:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_1:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_2:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_3:
	{

		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_3:index = 3; break;
		}

		param = _imgAcqPty.pockelPty.pockelsMaskInvert[index];
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
		param = _imgAcqPty.yChannelEnable;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:
	{
		long index = 0;
		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:index = 3; break;
		}

		switch (_pockelsResponseType[index])
		{
		case static_cast<long>(PockelsResponseType::SINE_RESPONSE):
		{
			//linearize the sine wave response of the pockels cell
			//sinusoidal response
			param = 100.0 * (1 - cos(M_PI * _imgAcqPty.pockelPty.pockelsPowerLevel[index])) / static_cast<double>(Constants::AREA_UNDER_CURVE);
		}
		break;
		case static_cast<long>(PockelsResponseType::LINEAR_RESPONSE):
		{
			//linear response
			param = 100.0 * _imgAcqPty.pockelPty.pockelsPowerLevel[index];
		}
		break;
		}
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0:
	{
		param = _imgAcqPty.pockelPty.pockelsLineBlankingPercentage[0] * 100.0;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1:
	{
		param = _imgAcqPty.pockelPty.pockelsLineBlankingPercentage[1] * 100.0;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2:
	{
		param = _imgAcqPty.pockelPty.pockelsLineBlankingPercentage[2] * 100.0;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3:
	{
		param = _imgAcqPty.pockelPty.pockelsLineBlankingPercentage[3] * 100.0;
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
		param = (_imgAcqPty.verticalScanDirection < 0) ? 1 : 0;
	}
	break;
	case ICamera::PARAM_LSM_HORIZONTAL_FLIP:
	{
		param = static_cast<double>(_imgAcqPty.horizontalFlip) == 0 ? 1 : 0;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MASK_WIDTH:
	{
		param = static_cast<double>(_imgAcqPty.pixelX);
	}
	break;
	case ICamera::PARAM_LSM_FINE_OFFSET_X:
	{
		param = _imgAcqPty.fineOffsetX;
	}
	break;
	case ICamera::PARAM_LSM_FINE_OFFSET_Y:
	{
		param = _imgAcqPty.fineOffsetY;
	}
	break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_X:
	{
		param = _imgAcqPty.fineFieldSizeScaleX;
	}
	break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y:
	{
		param = _imgAcqPty.fineFieldSizeScaleY;
	}
	break;
	case ICamera::PARAM_FRAME_RATE:
	{
		param = _frameRate;
	}
	break;
	case ICamera::PARAM_LSM_1X_FIELD_SIZE:
	{
		param = _oneXFieldSize;
	}
	break;
	case ICamera::PARAM_LSM_PULSE_MULTIPLEXING_ENABLE:
	{
		param = _imgAcqPty.laserCoherentSamplingEnable;
	}
	break;
	case ICamera::PARAM_LSM_EXTERNAL_CLOCK_PHASE_OFFSET:
	{
		param = _imgAcqPty.laserCoherentSamplingPhase;
	}
	break;
	case ICamera::PARAM_LSM_DATAMAP_MODE:
	{
		param = _imgAcqPty.dataMapMode;
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_1:
	{
		param = _imgAcqPty.channelPolarity[0];
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_2:
	{
		param = _imgAcqPty.channelPolarity[1];
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_3:
	{
		param = _imgAcqPty.channelPolarity[2];
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_4:
	{
		param = _imgAcqPty.channelPolarity[3];
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

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:index = 3; break;
		}
		param = _imgAcqPty.pockelPty.pockelsMinVoltage[index];
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:
	{
		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:index = 3; break;
		}
		param = _imgAcqPty.pockelPty.pockelsMaxVoltage[index];
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_2:
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_3:
	{
		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_3:index = 3; break;
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

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_3:index = 3; break;
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
		param = _imgAcqPty.scanAreaAngle * 180 / M_PI; // send angle in degrees
	}
	break;
	case ICamera::PARAM_DROPPED_FRAMES:
	{
		param = _droppedFramesCnt;
	}
	break;
	case ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY:
	{
		param = _imgAcqPty.rawSaveEnabledChannelOnly;
	}
	break;
	case ICamera::PARAM_LSM_3P_ENABLE:
	{
		param = _imgAcqPty.threePhotonModeEnable;
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_FINE:
	{
		param = _imgAcqPty.laserCoherentSamplingPhase;
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_0:
	{
		param = _imgAcqPty.threePhotonModeAlignmentPhase[0];
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_1:
	{
		param = _imgAcqPty.threePhotonModeAlignmentPhase[1];
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_2:
	{
		param = _imgAcqPty.threePhotonModeAlignmentPhase[2];
	}
	break;
	case ICamera::PARAM_LSM_3P_ALIGN_COARSE_3:
	{
		param = _imgAcqPty.threePhotonModeAlignmentPhase[3];
	}
	break;
	case ICamera::PARAM_LSM_INPUTRANGE1:
	{
		param = _imgAcqPty.inputRange[0];
	}
	break;
	case ICamera::PARAM_LSM_INPUTRANGE2:
	{
		param = _imgAcqPty.inputRange[1];
	}
	break;
	case ICamera::PARAM_LSM_INPUTRANGE3:
	{
		param = _imgAcqPty.inputRange[2];
	}
	break;
	case ICamera::PARAM_LSM_INPUTRANGE4:
	{
		param = _imgAcqPty.inputRange[3];
	}
	break;
	case ICamera::PARAM_LSM_3P_FIR_CHANNEL:
	{
		param = _FIRFilterSelectedSettingChannel;
	}
	break;
	case ICamera::PARAM_LSM_3P_FIR_INDEX:
	{
		param = _FIRFilterSelectedIndex;
	}
	break;
	case ICamera::PARAM_LSM_3P_FIR_TAP_INDEX:
	{
		param = _FIRFilterSelectedTapIndex;
	}
	break;
	case ICamera::PARAM_LSM_3P_FIR_TAP_VALUE:
	{
		if (_FIRFilterSelectedIndex >= 0 && _FIRFilterSelectedIndex < FIR_FILTER_COUNT &&
			_FIRFilterSelectedTapIndex >= 0 && _FIRFilterSelectedTapIndex < FIR_FILTER_TAP_COUNT)
		{
			param = _imgAcqPty.FIRFilters[_FIRFilterSelectedIndex][0][_FIRFilterSelectedTapIndex];
		}
	}
	break;
	case ICamera::PARAM_LSM_3P_MANUAL_FIR1_CONTROL_ENABLE:
	{
		param = _imgAcqPty.FIR1ManualControlenable;
	}
	break;
	case ICamera::PARAM_LSM_GG_TURNAROUNDTIME_US:
	{
		param = _imgAcqPty.turnAroundTimeUS;
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_0:
	{
		param = _preDcOffset[0];
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_1:
	{
		param = _preDcOffset[1];
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_2:
	{
		param = _preDcOffset[2];
	}
	break;
	case ICamera::PARAM_LSM_DIG_OFFSET_3:
	{
		param = _preDcOffset[3];
	}
	break;
	case ICamera::PARAM_LSM_GG_SUPER_USER:
	{
		param = _ggSuperUserMode;
	}
	break;
	case ICamera::PARAM_LSM_CALCULATED_MIN_DWELL:
	{
		double val = CalculateMinimumDwellTime(_imgAcqPty.fieldSize, _imgAcqPty.fineFieldSizeScaleX, _imgAcqPty.pixelX, _imgAcqPty.turnAroundTimeUS, _field2Theta, MAX_GALVO_OPTICAL_ANGLE, _maxAngularVelocityRadPerSec, _maxAngularAccelerationRadPerSecSq, MIN_DWELL_TIME, MAX_DWELL_TIME, DWELL_TIME_STEP);
		param = val;
	}
	break;
	case ICamera::PARAM_LSM_WAVEFORM_DRIVER_TYPE:
	{
		param = WaveformDriverType::WaveformDriver_ThorDAQ;
	}
	break;
	case ICamera::PARAM_LSM_NUMBER_OF_PLANES:
	{
		param = _imgAcqPty.numberOfPlanes;
	}
	break;
	case ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS:
	{
		param = _precaptureStatus;
	}
	break;
	case ICamera::PARAM_LSM_FAST_ONEWAY_MODE_ENABLE:
	{
		param = _imgAcqPty.fastOneWayEnable;
	}
	break;
	case ICamera::PARAM_LSM_GG_ACQUIRE_DURING_TURAROUND:
	{
		param = _imgAcqPty.acquireDuringTurnAround;
	}
	break;
	case ICamera::PARAM_3P_LUT_OFFSET:
	{
		param = _imgAcqPty.sampleOffsetStartLUT3PTI;
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_0:
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_1:
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_2:
	case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_3:
	{
		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_LINE_DELAY_US_3:index = 3; break;
		}

		param = _imgAcqPty.pockelPty.pockelsDelayUS[index];
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS_ENABLE:
	{
		param = _ddsClockEnable;
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS0_PHASE:
	{
		param = _ddsClockPhase0;
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS1_PHASE:
	{
		param = _ddsClockPhase1;
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS0_AMPLITUDE:
	{
		param = _ddsClockAmplitude0;
	}
	break;
	case ICamera::PARAM_3P_REVERB_DDS1_AMPLITUDE:
	{
		param = _ddsClockAmplitude1;
	}
	break;
	case ICamera::PARAM_LSM_SELECTED_IMAGING_GG:
	{
		param = _imgAcqPty.selectedImagingGG;
	}
	break;
	case ICamera::PARAM_LSM_SELECTED_STIM_GG:
	{
		param = _imgAcqPty.selectedStimGG;
	}
	break;
	case ICamera::PARAM_LSM_SECONDARY_GG_AVAILABLE:
	{
		param = _secondaryGGAvailable;
	}
	break;
	case ICamera::PARAM_RESEARCH_CAMERA_0:
	{
		param = _imgAcqPty.movingAverageFilterEnable;
	}
	break;
	case ICamera::PARAM_RESEARCH_CAMERA_1:
	{
		param = _imgAcqPty.movingAverageFilterMultiplier;
	}
	break;
	case ICamera::PARAM_3P_ENABLE_DOWNSAMPLING_RATE_CHANGE:
	{
		param = _imgAcqPty.enableDownsamplingRateChange;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"TD GG GetParam Enable Downsampling Rate Change - %d", param);
		LogMessage(_errMsg, VERBOSE_EVENT);
	}
	break;
	case ICamera::PARAM_3P_DOWNSAMPLING_RATE:
	{
		param = _imgAcqPty.threePhotonDownsamplingRate;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"TD GG GetParam Downsampling Rate - %d", param);
		LogMessage(_errMsg, VERBOSE_EVENT);
	}
	break;
	case ICamera::PARAM_LSM_LINE_AVERAGING_ENABLE:
	{
		param = _imgAcqPty.lineAveragingEnable;
	}
	break;
	case ICamera::PARAM_LSM_LINE_AVERAGING_NUMBER:
	{
		param = _imgAcqPty.lineAveragingNumber;
	}
	break;
	default:
	{
		ret = FALSE;
	}
	}

	if (FALSE == ret)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"TD GG GetParam Parameter (%d) not implemented", paramID);
		LogMessage(_errMsg, ERROR_EVENT);
	}

	return ret;
}


long CThorDAQGalvoGalvo::SetParamString(const long paramID, wchar_t* str)
{
	long ret = FALSE;

	switch (paramID)
	{
	case ICamera::PARAM_LSM_WAVEFORM_PATH_NAME:
	{
		_stimWaveformPath = std::wstring(str);
		//TODO: see in ThorGGNI if line below is necessary
		//SAFE_DELETE_MEMORY(_pGalvoStartPos);
		ret = TRUE;
	}
	break;
	}

	return ret;
}

long CThorDAQGalvoGalvo::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	switch (paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME:
	{
		wcscpy_s(str, 20, _pDetectorName);
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_WAVEFORM_PATH_NAME:
	{
		wcscpy_s(str, size, _stimWaveformPath.c_str());
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_DAQ_NAME:
	{
		wcscpy_s(str, size, L"ThorDAQ");
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_DAQ_FW_VER:
	{
		std::wstringstream stream;
		stream << std::hex << _boardInfo.UserVersion;
		std::wstring result(stream.str());
		wstring fw = L"0x" + result;
		wcscpy_s(str, size, fw.c_str());
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_DAQ_DRIVER_VER:
	{
		wstring driver = to_wstring(_boardInfo.DriverVersionMajor) + L"." + to_wstring(_boardInfo.DriverVersionMinor) + L"." + to_wstring(_boardInfo.DriverVersionSubMinor);
		wcscpy_s(str, size, driver.c_str());
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_LOW_FREQ_TRIG_BOARD_FW_VER:
	{
		wstring fw = _lowFreqTrigBoardInfo.AppFW + L" Ver. " + to_wstring(_lowFreqTrigBoardInfo.FWVerMajor) + L"." + to_wstring(_lowFreqTrigBoardInfo.FWVerMinor) + L"." + to_wstring(_lowFreqTrigBoardInfo.FWVerSubMinor);
		wcscpy_s(str, size, fw.c_str());
		ret = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_LOW_FREQ_TRIG_BOARD_CPLD_VER:
	{
		wstring cpld = std::wstring() + (wchar_t)_lowFreqTrigBoardInfo.CPLDUsercodeMajor + L"." + (wchar_t)_lowFreqTrigBoardInfo.CPLDUsercodeSubMajor + L"." + (wchar_t)_lowFreqTrigBoardInfo.CPLDUsercodeMinor + L"." + (wchar_t)_lowFreqTrigBoardInfo.CPLDUsercodeSubMinor;
		wcscpy_s(str, size, cpld.c_str());
		ret = TRUE;
	}
	break;
	default:
		break;
	}

	return ret;
}

long CThorDAQGalvoGalvo::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;
	switch (paramID)
	{
	case ICamera::PARAM_LSM_POCKELS_MASK_0:
	case ICamera::PARAM_LSM_POCKELS_MASK_1:
	case ICamera::PARAM_LSM_POCKELS_MASK_2:
	case ICamera::PARAM_LSM_POCKELS_MASK_3:
	{

		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_MASK_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_3:index = 3; break;
		}


		SAFE_DELETE_ARRAY(_pPockelsMask[index]);

		try
		{
			_pPockelsMask[index] = new char[size];
			_pockelsMaskSize[index] = size;
			memcpy(_pPockelsMask[index], pBuffer, size);
			_pockelsMaskChanged = TRUE;
		}
		catch (...)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Could not allocate pockels mask");
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_POWER_RAMP_PERCENTAGE_BUFFER:
	{
		try
		{
			long length = size;
			double* percentValues = (double*)pBuffer;
			_imgAcqPty.powerRampPercentValues.clear();
			for (int i = 0; i < length; ++i)
			{
				_imgAcqPty.powerRampPercentValues.push_back(percentValues[i]);
			}
		}
		catch (...)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Could not allocate power ram perfentage buffer");
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	default:
	{
		ret = FALSE;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter (%d) not implemented", paramID);
		LogMessage(_errMsg, ERROR_EVENT);
	}
	}
	return ret;
}

long CThorDAQGalvoGalvo::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	switch (paramID)
	{
	case ICamera::PARAM_LSM_POCKELS_MASK_0:
	case ICamera::PARAM_LSM_POCKELS_MASK_1:
	case ICamera::PARAM_LSM_POCKELS_MASK_2:
	case ICamera::PARAM_LSM_POCKELS_MASK_3:
	{

		long index = 0;

		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_MASK_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_MASK_3:index = 3; break;
		}
		if ((pBuffer != NULL) && (_pPockelsMask[index] != NULL))
		{
			if (_pockelsMaskSize[index] <= size)
			{
				memcpy(pBuffer, _pPockelsMask[index], _pockelsMaskSize[index]);
			}
		}
	}
	break;
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_0:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_1:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_2:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_3:
	{
		long index = 0;
		switch (paramID)
		{
		case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_0:index = 0; break;
		case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_1:index = 1; break;
		case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_2:index = 2; break;
		case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_3:index = 3; break;
		}

		if (POCKELS_VOLTAGE_STEPS * sizeof(float64) <= size)
		{
			std::memcpy(pBuffer, &_pockelsReadArray[index], POCKELS_VOLTAGE_STEPS * sizeof(float64));
		}
	}
	break;
	default:
	{
		ret = FALSE;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter (%d) not implemented", paramID);
		LogMessage(_errMsg, ERROR_EVENT);
	}

	}
	return ret;
}
