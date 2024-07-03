#include "stdafx.h"
#include "ThorDAQGalvoGalvo.h"
#include "thordaqGalvoGalvoSetupXML.h"
wchar_t message2[MSG_SIZE];
/**********************************************************************************************//**
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
long CThorDAQGalvoGalvo::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
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
			paramMin = MIN_FIELD_SIZE_X;
			paramMax = MAX_FIELD_SIZE_X;
			paramDefault = DEFAULT_FIELD_SIZE_X;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_X:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = -(MAX_FIELD_SIZE_X - MIN_FIELD_SIZE_X) / 2;
			paramMax = (MAX_FIELD_SIZE_X - MIN_FIELD_SIZE_X) / 2;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_Y:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = -(MAX_FIELD_SIZE_X - MIN_FIELD_SIZE_X) / 2;
			paramMax = (MAX_FIELD_SIZE_X - MIN_FIELD_SIZE_X) / 2;
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
			paramMin = MIN_ALIGNMENT / 32;
			paramMax = MAX_ALIGNMENT / 32;
			paramDefault = MIN_ALIGNMENT / 32;
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
	case ICamera::PARAM_LSM_INPUTRANGE1:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = FALSE;
			paramMin = MIN_INPUTRANGE;
			paramMax = MAX_INPUTRANGE;
			paramDefault = DEFAULT_INPUTRANGE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE2:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = FALSE;
			paramMin = MIN_INPUTRANGE;
			paramMax = MAX_INPUTRANGE;
			paramDefault = DEFAULT_INPUTRANGE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE3:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = FALSE;
			paramMin = MIN_INPUTRANGE;
			paramMax = MAX_INPUTRANGE;
			paramDefault = DEFAULT_INPUTRANGE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE4:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = FALSE;
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
			paramMin = INTERNAL_CLOCK;         
			paramMax = EXTERNAL_CLOCK;         
			paramDefault = INTERNAL_CLOCK;
			paramReadOnly = TRUE;
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
			if (_imgAcqPty.scanMode == TWO_WAY_SCAN_MODE)         //two way scan
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
			paramMin = MIN_DWELL_TIME;
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
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = MAX_POCKELS_LINE_BLANKING_PERCENTAGE;
			paramDefault = 10;
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
	case ICamera::PARAM_LSM_1X_FIELD_SIZE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_FIELD_SIZE_X;
			paramMax = MAX_FIELD_SIZE_X;
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
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_1:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = POL_NEG;
			paramMax = POL_POS;
			paramDefault = POL_NEG;
			paramReadOnly = FALSE;
		}
		break;		
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_2:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = POL_NEG;
			paramMax = POL_POS;
			paramDefault = POL_NEG;
			paramReadOnly = FALSE;
		}
		break;	
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_3:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = POL_NEG;
			paramMax = POL_POS;
			paramDefault = POL_NEG;
			paramReadOnly = FALSE;
		}
		break;	
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
	case ICamera::PARAM_DFLIM_ACQUISITION_MODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_DFLIM_ACQUISITION_MODE;
			paramMax = MAX_DFLIM_ACQUISITION_MODE;
			paramDefault = DEFAULT_DFLIM_AQUISITION_MODE;
			paramReadOnly = FALSE;
		}
		break;	
	case ICamera::PARAM_DFLIM_RESYNC:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 1;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_DFLIM_COARSE_SHIFT1:
	case ICamera::PARAM_DFLIM_COARSE_SHIFT2:
	case ICamera::PARAM_DFLIM_COARSE_SHIFT3:
	case ICamera::PARAM_DFLIM_COARSE_SHIFT4:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_DFLIM_COARSE_SHIFT;
			paramMax = MAX_DFLIM_COARSE_SHIFT;
			paramDefault = DEFAULT_DFLIM_COARSE_SHIFT;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_DFLIM_FINE_SHIFT1:
	case ICamera::PARAM_DFLIM_FINE_SHIFT2:
	case ICamera::PARAM_DFLIM_FINE_SHIFT3:
	case ICamera::PARAM_DFLIM_FINE_SHIFT4:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_DFLIM_FINE_SHIFT;
			paramMax = MAX_DFLIM_FINE_SHIFT;
			paramDefault = DEFAULT_DFLIM_FINE_SHIFT;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_DFLIM_QUERY_CLOCK_FREQS:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = TRUE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_DFLIM_FREQ_CLOCK0:
	case ICamera::PARAM_DFLIM_FREQ_CLOCK1:
	case ICamera::PARAM_DFLIM_FREQ_CLOCK2:
	case ICamera::PARAM_DFLIM_FREQ_CLOCK3:
	case ICamera::PARAM_DFLIM_FREQ_CLOCK4:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_DFLIM_CLOCK_FREQ;
			paramMax = MAX_DFLIM_CLOCK_FREQ;
			paramDefault = DEFAULT_DFLIM_CLOCK_FREQ;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_DFLIM_IMPLIED_FREQ_CLOCK0:
	case ICamera::PARAM_DFLIM_IMPLIED_FREQ_CLOCK1:
	case ICamera::PARAM_DFLIM_IMPLIED_FREQ_CLOCK2:
	case ICamera::PARAM_DFLIM_IMPLIED_FREQ_CLOCK3:
	case ICamera::PARAM_DFLIM_IMPLIED_FREQ_CLOCK4:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_DFLIM_CLOCK_FREQ;
			paramMax = MAX_DFLIM_CLOCK_FREQ;
			paramDefault = DEFAULT_DFLIM_CLOCK_FREQ;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_DFLIM_SYNC_DELAY:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_DFLIM_SYNC_DELAY;
			paramMax = MAX_DFLIM_SYNC_DELAY;
			paramDefault = DEFAULT_DFLIM_SYNC_DELAY;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_DFLIM_RESYNC_DELAY:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_DFLIM_RESYNC_DELAY;
			paramMax = MAX_DFLIM_RESYNC_DELAY;
			paramDefault = DEFAULT_DFLIM_RESYNC_DELAY;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_DFLIM_RESYNC_EVERYLINE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_DFLIM_SAVE_SETTINGS:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_DFLIM_SAVE_IMAGES_ON_LIVE_MODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_DFLIM_FRAME_TYPE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 1;
			paramReadOnly = TRUE;
		}
		break;
	default:
		{
			ret = TRUE;
			paramAvailable = FALSE;
			paramReadOnly = TRUE;
		}
	}
	return ret;
}

long CThorDAQGalvoGalvo::SetParam(const long paramID, const double param)
{
	long ret = FALSE;

	switch (paramID)
	{
	case ICamera::PARAM_LSM_SCANMODE:
		{
			if ((param >= MIN_SCANMODE) && (param <= MAX_SCANMODE))
			{
				_imgAcqPty.scanMode = static_cast<long> (param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_SCANMODE %d outside range %d to %d",static_cast<long> (param), MIN_SCANMODE,MAX_SCANMODE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_AREAMODE:
		{
			if ((param >= MIN_AREAMODE) && (param <= MAX_AREAMODE))
			{
				_imgAcqPty.areaMode = static_cast<long> (param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_AREAMODE %d outside range %d to %d",static_cast<long> (param), MIN_AREAMODE,MAX_AREAMODE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_X:
		{
			long factor;
			//TODO: remove factor if it all works at 2048
			if (_imgAcqPty.scanMode == TWO_WAY_SCAN_MODE)
			{
				factor = 1;
			}
			else
			{
				factor = 1;
			}

			if ((param >= MIN_PIXEL_X) && (param <= MAX_PIXEL_X / factor))
			{
				if (static_cast<long>(param) % 16 == 0)
				{
					_imgAcqPty.pixelX = static_cast<long>(param);

					//min dwell time is partly dependent on pixel X, after updating pixel X,
					//make sure the current dwell time is still acceptable, otherwise it will get increased
					//to an acceptable number
					SetParam(PARAM_LSM_DWELL_TIME, _imgAcqPty.dwellTime);

					if(ICamera::SQUARE == _imgAcqPty.areaMode)
					{
						_imgAcqPty.pixelY = _imgAcqPty.pixelX;
					}
					ret = TRUE;
				}
				else
				{
					StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_PIXEL_X must be a multiple of 16");
					LogMessage(_errMsg,ERROR_EVENT);
				}
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_PIXEL_X %d outside range %d to %d",static_cast<long> (param), MIN_PIXEL_X, static_cast<long>(MAX_PIXEL_X / factor));
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_Y:
		{
			long factor;
			//TODO: remove factor if it all works at 2048
			if (_imgAcqPty.scanMode == TWO_WAY_SCAN_MODE)
			{
				factor = 1;
			}
			else
			{
				factor = 1;
			}

			if(ICamera::SQUARE == _imgAcqPty.areaMode)
			{
				_imgAcqPty.pixelY = _imgAcqPty.pixelX;
				ret = TRUE;
			}
			else if(ICamera::LINE == _imgAcqPty.areaMode)
			{
				if ((param > 1) && (param < 1))
				{
					StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_PIXEL_Y must equal 1 in AreaMode LINE");
					LogMessage(_errMsg,ERROR_EVENT);
				}
				else
				{
					_imgAcqPty.pixelY = static_cast<long>(param);
					ret = TRUE;
				}
			}
			else
			{
				if ((param >= MIN_PIXEL_Y) && (param <= MAX_PIXEL_Y / factor))
				{

					if (static_cast<long>(param) % 16 == 0 || 
						1 == static_cast<long>(param)  ||
						2 == static_cast<long>(param))
					{

						if(static_cast<long>(param) > _imgAcqPty.pixelX)
						{
							StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_PIXEL_Y cannot be greater than PARAM_LSM_PIXEL_X");
							LogMessage(_errMsg,ERROR_EVENT);
						}
						else
						{
							_imgAcqPty.pixelY = static_cast<long>(param);
							ret = TRUE;
						}
					}
					else
					{
						StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_PIXEL_Y must be a multiple of 16");
						LogMessage(_errMsg,ERROR_EVENT);
					}
				}
				else
				{
					StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_PIXEL_Y %d outside range %d to %d",static_cast<long> (param), MIN_PIXEL_Y, static_cast<long>(MAX_PIXEL_Y / factor));
					LogMessage(_errMsg,ERROR_EVENT);
				}
			}
		}
		break;

	case ICamera::PARAM_LSM_FIELD_SIZE:
		{
			if ((param >= MIN_FIELD_SIZE_X) && (param <= MAX_FIELD_SIZE_X))
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_FIELD_SIZE %d outside range %d to %d",static_cast<long> (param), MIN_FIELD_SIZE_X,MAX_FIELD_SIZE_X);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_X:
		{
			if ((param >= -(MAX_FIELD_SIZE_X - _imgAcqPty.fieldSize) / 2) && (param <= (MAX_FIELD_SIZE_X - _imgAcqPty.fieldSize) / 2))
			{
				_imgAcqPty.offsetX = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_OFFSET_X %d outside range %d to %d",static_cast<long> (param), -(MAX_FIELD_SIZE_X - _imgAcqPty.fieldSize) / 2,(MAX_FIELD_SIZE_X - _imgAcqPty.fieldSize) / 2);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_Y:
		{
			if ((param >= -(MAX_FIELD_SIZE_X - _imgAcqPty.fieldSize) / 2) && (param <= (MAX_FIELD_SIZE_X - _imgAcqPty.fieldSize) / 2))
			{
				_imgAcqPty.offsetY = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_OFFSET_Y %d outside range %d to %d",static_cast<long> (param), -(MAX_FIELD_SIZE_X - _imgAcqPty.fieldSize) / 2,(MAX_FIELD_SIZE_X - _imgAcqPty.fieldSize) / 2);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_ALIGNMENT:
		{
			if ((param >= MIN_ALIGNMENT / 32) && (param <= MAX_ALIGNMENT / 32))
			{
				_imgAcqPty.alignmentForField = static_cast<long>(param * 32);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_ALIGNMENT %d outside range %d to %d",static_cast<long> (param * 100), MIN_ALIGNMENT,MAX_ALIGNMENT);
				LogMessage(_errMsg,ERROR_EVENT);
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_Y_AMPLITUDE_SCALER %d outside range %d to %d",static_cast<long> (param), MIN_Y_AMPLITUDE_SCALER,MAX_Y_AMPLITUDE_SCALER);
				LogMessage(_errMsg,ERROR_EVENT);
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
			if(param>0)
			{
				_minimizeFlybackCycles = true;
			}
			else
			{
				_minimizeFlybackCycles = false;
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_GALVO_ENABLE %d outside range %d to %d",static_cast<long> (param), MIN_GALVO_ENABLE,MAX_GALVO_ENABLE);
				LogMessage(_errMsg,ERROR_EVENT);
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_INTERNALCLOCKRATE %d outside range %d to %d", static_cast<long>(param), MIN_INTERNALCLOCKRATE,MAX_INTERNALCLOCKRATE);
				LogMessage(_errMsg,ERROR_EVENT);
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_CHANNEL %d outside range %d to %d",static_cast<long> (param), MIN_CHANNEL,MAX_CHANNEL);
				LogMessage(_errMsg,ERROR_EVENT);
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_TRIGGER_MODE %d outside range %d to %d", static_cast<long> (param),MIN_TRIGGER_MODE,MAX_TRIGGER_MODE);
				LogMessage(_errMsg,ERROR_EVENT);
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_MULTI_FRAME_COUNT %d outside range %d to %d",static_cast<long> (param), 1,INT_MAX);
				LogMessage(_errMsg,ERROR_EVENT);
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_AVERAGEMODE %d outside range %d to %d",static_cast<long> (param), NO_AVERAGE,FRM_CUMULATIVE_MOVING);
				LogMessage(_errMsg,ERROR_EVENT);
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_AVERAGENUM %d outside range %d to %d",static_cast<long> (param), MIN_AVERAGENUM,MAX_AVERAGENUM);
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_TRIGGER_TIMEOUT_SEC %d outside range %d to %d",static_cast<long> (param), MIN_TRIGGER_TIMEOUT,MAX_TRIGGER_TIMEOUT);
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_ENABLE_FRAME_TRIGGER_WITH_HW_TRIG %d outside range %d to %d",static_cast<long> (param), MIN_ENABLE_FRAME_TRIGGER,MAX_ENABLE_FRAME_TRIGGER);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;

	case ICamera::PARAM_LSM_DWELL_TIME:
		{
			if ((param >= MIN_DWELL_TIME) && (param <= MAX_DWELL_TIME))
			{
				double minDwell = CalculateMinimumDwellTime(_imgAcqPty.fieldSize, _imgAcqPty.fineFieldSizeScaleX, _imgAcqPty.pixelX, _imgAcqPty.turnAroundTimeUS, _field2Theta, MAX_GALVO_OPTICAL_ANGLE, _maxAngularVelocityRadPerSec, _maxAngularAccelerationRadPerSecSq, MIN_DWELL_TIME, MAX_DWELL_TIME, DWELL_TIME_STEP);

				if (minDwell > MAX_DWELL_TIME)
				{
					minDwell = MAX_DWELL_TIME;
				}
				if (minDwell < param || TRUE == _ggSuperUserMode)
				{
					_imgAcqPty.dwellTime = param;
				}
				else
				{
					_imgAcqPty.dwellTime = minDwell;
				}
				
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_DWELL_TIME %d.%d outside range .4 to 20",static_cast<long> (param),static_cast<long>(10 * (param - static_cast<long> (param))));
				LogMessage(_errMsg,ERROR_EVENT);
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_GALVO_RASTERANGLE %d outside range %d to %d",static_cast<long> (param), MIN_RASTERANGLE, MAX_RASTERANGLE);
				LogMessage(_errMsg,ERROR_EVENT);
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_GALVO_LINEDUTY %d outside range %d to %d",static_cast<long> (param), MIN_FORWARD_LINE_DUTY, MAX_FORWARD_LINE_DUTY);
				LogMessage(_errMsg,ERROR_EVENT);
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
				if(MAX_GALVO_ENABLE == _imgAcqPty.galvoEnable)
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_Y_COMMUNICATION_ENABLE %d outside range %d to %d",static_cast<long> (param), MIN_Y_CHANNEL_ENABLE,MAX_Y_CHANNEL_ENABLE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;

	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:
	//case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:
		{
			long index=0;

			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:index = 0;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:index = 1;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:index = 2;break;
			//case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:index = 3;break;
			}

			if((param >= 0) && (param <= 100))
			{			
				switch(_pockelsResponseType[index])
				{
				case 0:
					{
						//linearize the sine wave response of the pockels cell
						const double AREA_UNDER_CURVE = 2.0;

						_imgAcqPty.pockelPty.pockelsPowerLevel[index] = acos(1 - AREA_UNDER_CURVE * param/100.0)/M_PI;
					}
					break;
				case 1:
					{
						//linear response
						_imgAcqPty.pockelPty.pockelsPowerLevel[index] = param/100.0;
					}
					break;
				}

				ret = TRUE;

				//the scanner is in centering mode. allow the power to be changed instantaneously
				if(_imgAcqPty.scanMode == ScanMode::CENTER_SCAN_MODE)
				{
					MovePockelsToPowerLevel(index,&_imgAcqPty.pockelPty);
				}
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE %d outside range %d to %d",static_cast<long> (param), 0,100);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;

	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2:index=2;break;
			}

			if((param >= 0) && (param <= MAX_POCKELS_LINE_BLANKING_PERCENTAGE))
			{
				_imgAcqPty.pockelPty.pockelsLineBlankingPercentage[index] = param/100.0;
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"MAX_POCKELS_LINE_BLANKING_PERCENTAGE %d outside range %d to %d",static_cast<long> (param), 0,MAX_POCKELS_LINE_BLANKING_PERCENTAGE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0:
		{
			_pockelsResponseType[0] = static_cast<long>(param);
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_1:
		{
			_pockelsResponseType[1] = static_cast<long>(param);
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_2:
		{
			_pockelsResponseType[2] = static_cast<long>(param);
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_3:
		{
			_pockelsResponseType[3] = static_cast<long>(param);
		}
		break;
	case ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION:
		{
			if((param >= 0) && (param <= 1))
			{
				if(0 == static_cast<long>(param))
				{
					_imgAcqPty.verticalScanDirection = 1.0;
				}
				else
				{
					_imgAcqPty.verticalScanDirection = -1.0;
				}
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_VERTICAL_SCAN_DIRECTION %d outside range %d to %d",static_cast<long> (param), 0,1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_HORIZONTAL_FLIP:
		{
			if((param >= 0) && (param <= 1))
			{
				_imgAcqPty.horizontalFlip = static_cast<long>(param);				
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_HORIZONTAL FLIP %d outside range %d to %d",static_cast<long> (param), 0,1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FINE_OFFSET_X:
		{
			if((param >= -1.0) && (param <= 1.0))
			{
				_imgAcqPty.fineOffsetX = param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_FINE_OFFSET_X %d outside range %d to %d",static_cast<long> (param), -.1,.1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FINE_OFFSET_Y:
		{
			if((param >= -1.0) && (param <= 1.0))
			{
				_imgAcqPty.fineOffsetY = param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_FINE_OFFSET_Y %d outside range %d to %d",static_cast<long> (param), -.1,.1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_X:
		{
			if((param >= .8) && (param <= 1.2))
			{
				_imgAcqPty.fineFieldSizeScaleX = param;
				SetParam(PARAM_LSM_DWELL_TIME, _imgAcqPty.dwellTime);
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_FINE_FIELD_SIZE_SCALE_X %d outside range %d to %d",static_cast<long> (param), .9,1.1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y:
		{
			if((param >= .8) && (param <= 1.2))
			{
				_imgAcqPty.fineFieldSizeScaleY = param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y %d outside range %d to %d",static_cast<long> (param), .9,1.1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera:: PARAM_LSM_PULSE_MULTIPLEXING_ENABLE:
		{
			
			_imgAcqPty.laserCoherentSamplingEnable = static_cast<long>(param);
		}
		break;
	case ICamera::PARAM_LSM_EXTERNAL_CLOCK_PHASE_OFFSET:
		{
			_imgAcqPty.laserCoherentSamplingPhase = static_cast<long>(param);
		}
		break;
	case ICamera::PARAM_LSM_DATAMAP_MODE:
		{
			if((param >= ICamera::FIRST_MAPPING_MODE) && (param < ICamera::LAST_MAPPING_MODE))
			{
				_imgAcqPty.dataMapMode = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_DATAMAP_MODE %d outside range %d to %d",static_cast<long> (param), ICamera::FIRST_MAPPING_MODE,ICamera::LAST_MAPPING_MODE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_DMA_BUFFER_COUNT:
		{
			if((param>=MIN_DMA_BUFFER_NUM) && (param <= MAX_DMA_BUFFER_NUM))
			{
				_imgAcqPty.dmaBufferCount = static_cast<long>(param);
			}
		}
		break;	
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_1:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				_channelPolarity[0] = static_cast<long>(param);
			}
		}
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_2:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				_channelPolarity[1] = static_cast<long>(param);
			}
		}
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_3:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				_channelPolarity[2] = static_cast<long>(param);
			}
		}
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_4:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				_channelPolarity[3] = static_cast<long>(param);
			}
		}
		break;
	case ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY:
		{
			if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
			{
				_imgAcqPty.rawSaveEnabledChannelOnly = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_0:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_1:
	case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_2:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_2:index=2;break;
			}

			if(1 == static_cast<long>(param))
			{
				ret = FindPockelsMinMax(index,&_imgAcqPty.pockelPty);
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_POCKELS_FIND_MIN_MAX failed");
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:index=2;break;
			}
			_pockelsMinVoltage[index] = param;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:index=2;break;
			}
			_pockelsMaxVoltage[index] = param;
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_SCANAEREA_ANGLE %f outside range %f to %f",param, MIN_SCANAREA_ANGLE,MAX_SCANAREA_ANGLE);
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
				_imgAcqPty.pockelPty.useReferenceForPockelsOutput = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_3P_ENABLE:
		{
 		//	if((FALSE == param) || (TRUE == param))
			//{
			//	if (param)
			//	{
			//		if (_deviceNum != 0)
			//		{
			//			ULONG32 clock_rate = 0;
			//			ULONG32 clock_ref = 0;
			//			if( ThordaqMeasureExternClockRate(_DAQDeviceIndex, clock_rate,clock_ref, 1) == THORDAQ_STATUS::STATUS_SUCCESSFUL)
			//			{
			//				_imgAcqPty.clockRateExternal = clock_rate;
			//				_imgAcqPty.maxSampleRate = static_cast<long>(clock_ref * 2.0);
			//			}
			//		}
			//	}else
			//	{
			//		_imgAcqPty.maxSampleRate = DEFAULT_INTERNALCLOCKRATE;
			//	}
			//	_imgAcqPty.threePhotonModeEnable = static_cast<long>(param);
			//}
		}
		break;
	case ICamera::PARAM_LSM_3P_ALIGN_FINE:
		{
			_imgAcqPty.laserCoherentSamplingPhase = static_cast<long>(param);
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
	case ICamera::PARAM_DFLIM_ACQUISITION_MODE:
		{
			if (static_cast<long>(param) <= MAX_DFLIM_ACQUISITION_MODE && static_cast<long>(param) >= MIN_DFLIM_ACQUISITION_MODE)
			{
				_imgAcqPty.acquistionMode = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_RESYNC:
		{
			long error = 0, status = 0;
			ThordaqErrChk2 (L"ThordaqReSync", status = ThordaqReSync(_DAQDeviceIndex));
		}
		break;
	case ICamera::PARAM_DFLIM_COARSE_SHIFT1:
		{
			if (static_cast<long>(param) <= MAX_DFLIM_COARSE_SHIFT && static_cast<long>(param) >= MIN_DFLIM_COARSE_SHIFT)
			{
				_dflimCoarseShiftA[0] = static_cast<long>(param);
				long error = 0, status = 0;
				int channel = 0;
				ThordaqErrChk2 (L"ThordaqSetCoarseShift", status = ThordaqSetCoarseShift(_DAQDeviceIndex, (ULONG32)_dflimCoarseShiftA[0], channel));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_COARSE_SHIFT2:
		{
			if (static_cast<long>(param) <= MAX_DFLIM_COARSE_SHIFT && static_cast<long>(param) >= MIN_DFLIM_COARSE_SHIFT)
			{
				_dflimCoarseShiftA[1] = static_cast<long>(param);
				long error = 0, status = 0;
				int channel = 1;
				ThordaqErrChk2 (L"ThordaqSetCoarseShift", status = ThordaqSetCoarseShift(_DAQDeviceIndex, (ULONG32)_dflimCoarseShiftA[1], channel));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_COARSE_SHIFT3:
		{
			if (static_cast<long>(param) <= MAX_DFLIM_COARSE_SHIFT && static_cast<long>(param) >= MIN_DFLIM_COARSE_SHIFT)
			{
				_dflimCoarseShiftA[2] = static_cast<long>(param);
				long error = 0, status = 0;
				int channel = 2;
				ThordaqErrChk2 (L"ThordaqSetCoarseShift", status = ThordaqSetCoarseShift(_DAQDeviceIndex, (ULONG32)_dflimCoarseShiftA[2], channel));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_COARSE_SHIFT4:
		{
			if (static_cast<long>(param) <= MAX_DFLIM_COARSE_SHIFT && static_cast<long>(param) >= MIN_DFLIM_COARSE_SHIFT)
			{
				_dflimCoarseShiftA[3] = static_cast<long>(param);
				long error = 0, status = 0;
				int channel = 3;
				ThordaqErrChk2 (L"ThordaqSetCoarseShift", status = ThordaqSetCoarseShift(_DAQDeviceIndex, (ULONG32)_dflimCoarseShiftA[3], channel));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_FINE_SHIFT1:
		{
			if (static_cast<long>(param) <= MAX_DFLIM_FINE_SHIFT && static_cast<long>(param) >= MIN_DFLIM_FINE_SHIFT)
			{
				_dflimFineShiftA[0] = static_cast<long>(param);
				long error = 0, status = 0;
				int channel = 0;
				ThordaqErrChk2 (L"ThordaqSetFineShift", status = ThordaqSetFineShift(_DAQDeviceIndex, (LONG32)_dflimFineShiftA[0], channel));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_FINE_SHIFT2:
		{
			if (static_cast<long>(param) <= MAX_DFLIM_FINE_SHIFT && static_cast<long>(param) >= MIN_DFLIM_FINE_SHIFT)
			{
				_dflimFineShiftA[1] = static_cast<long>(param);
				long error = 0, status = 0;
				int channel = 1;
				ThordaqErrChk2 (L"ThordaqSetFineShift", status = ThordaqSetFineShift(_DAQDeviceIndex, (LONG32)_dflimFineShiftA[1], channel));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_FINE_SHIFT3:
		{
			if (static_cast<long>(param) <= MAX_DFLIM_FINE_SHIFT && static_cast<long>(param) >= MIN_DFLIM_FINE_SHIFT)
			{
				_dflimFineShiftA[2] = static_cast<long>(param);
				long error = 0, status = 0;
				int channel = 2;
				ThordaqErrChk2 (L"ThordaqSetFineShift", status = ThordaqSetFineShift(_DAQDeviceIndex, (LONG32)_dflimFineShiftA[2], channel));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_FINE_SHIFT4:
		{
			if (static_cast<long>(param) <= MAX_DFLIM_FINE_SHIFT && static_cast<long>(param) >= MIN_DFLIM_FINE_SHIFT)
			{
				_dflimFineShiftA[3] = static_cast<long>(param);
				long error = 0, status = 0;
				int channel = 3;
				ThordaqErrChk2 (L"ThordaqSetFineShift", status = ThordaqSetFineShift(_DAQDeviceIndex, (LONG32)_dflimFineShiftA[3], channel));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_QUERY_CLOCK_FREQS:
		{
			for (int i = 0; i < NUM_DFLIM_CLOCKS; ++i)
			{
				long error = 0, status = 0;
				ThordaqErrChk2 (L"ThordaqGetClockFrequency", status = ThordaqGetClockFrequency(_DAQDeviceIndex, i, _dflimClockFrequencies[i]));
			}
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_DFLIM_SYNC_DELAY:
		{
			if (static_cast<long>(param) <= MAX_DFLIM_SYNC_DELAY && static_cast<long>(param) >= MIN_DFLIM_SYNC_DELAY)
			{
				_dflimSyncDelay = static_cast<long>(param);
				long error = 0, status = 0;
				ThordaqErrChk2 (L"ThordaqSetDFLIMSyncingSettings", status = ThordaqSetDFLIMSyncingSettings(_DAQDeviceIndex, static_cast<ULONG32>(_dflimSyncDelay), static_cast<ULONG32>(_dflimResyncDelay), TRUE == _dflimResyncEveryLine));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_RESYNC_DELAY:
		{
			if (static_cast<long>(param) <= MAX_DFLIM_RESYNC_DELAY && static_cast<long>(param) >= MIN_DFLIM_RESYNC_DELAY)
			{
				_dflimResyncDelay = static_cast<long>(param);
				long error = 0, status = 0;
				ThordaqErrChk2 (L"ThordaqSetDFLIMSyncingSettings", status = ThordaqSetDFLIMSyncingSettings(_DAQDeviceIndex, static_cast<ULONG32>(_dflimSyncDelay), static_cast<ULONG32>(_dflimResyncDelay), TRUE == _dflimResyncEveryLine));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_RESYNC_EVERYLINE:
		{
			if (static_cast<long>(param) == TRUE || static_cast<long>(param) == FALSE)
			{
				_dflimResyncEveryLine = static_cast<long>(param);
				long error = 0, status = 0;
				ThordaqErrChk2 (L"ThordaqSetDFLIMSyncingSettings", status = ThordaqSetDFLIMSyncingSettings(_DAQDeviceIndex, static_cast<ULONG32>(_dflimSyncDelay), static_cast<ULONG32>(_dflimResyncDelay), TRUE == _dflimResyncEveryLine));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_DFLIM_SAVE_SETTINGS:
		{
			if (static_cast<long>(param) == TRUE || static_cast<long>(param) == FALSE)
			{
				auto_ptr<ThorGalvoGalvoXML> pSetup(new ThorGalvoGalvoXML());
				if(FALSE == pSetup->SetChannelSettings(_dflimFineShiftA,_dflimFineShiftB,_dflimCoarseShiftA,_dflimCoarseShiftB,_dflimEnableIntAdj,_dflimThreshold,_dflimBaselineTolerance,_dflimMaxLevel0,_dflimMaxLevel1))
				{
					StringCbPrintfW(_errMsg,_MAX_PATH,L"SetChannelSettings from DFLIM ThorDAQGalvoGalvoSettings failed");
					LogMessage(_errMsg,ERROR_EVENT);
				}

				//for now save resyncEveryLine as non active (FALSE), might need to change in the future
				if(FALSE == pSetup->SetFrontEndTuning(_dflimSyncDelay, _dflimResyncDelay, FALSE))
				{
					StringCbPrintfW(_errMsg,_MAX_PATH,L"SetFrontEndTuning from DFLIM ThorDAQGalvoGalvoSettings failed");
					LogMessage(_errMsg,ERROR_EVENT);
				}
				ret = TRUE;
			}
		}
		break;	
	case ICamera::PARAM_DFLIM_SAVE_IMAGES_ON_LIVE_MODE:
		{
			if (static_cast<long>(param) == TRUE || static_cast<long>(param) == FALSE)
			{
				_saveLiveImage = static_cast<long>(param) == TRUE;
			}
		}
		break;
	default:
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) not implemented",paramID);
			LogMessage(_errMsg,ERROR_EVENT);
		}
		break;
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
long CThorDAQGalvoGalvo::GetParam(const long paramID, double &param)
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

	case ICamera::PARAM_LSM_FIELD_SIZE:
		{
			param = _imgAcqPty.fieldSize;
		}
		break;
	case ICamera::PARAM_LSM_ALIGNMENT:
		{
			param = _imgAcqPty.alignmentForField / 32.0;
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
			param = static_cast<long>(_minimizeFlybackCycles);
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
			param  = _imgAcqPty.offsetX;
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_Y:
		{
			param  = _imgAcqPty.offsetY;
		}
		break;
	case ICamera::PARAM_LSM_CLOCKSOURCE:
		{
			param  = EXTERNAL_CLOCK;
		}
		break;
	case ICamera::PARAM_LSM_INTERNALCLOCKRATE:
		{
			param  = _imgAcqPty.clockRateInternal;
		}
		break;
	//case ICamera::PARAM_LSM_EXTERNALCLOCKRATE:
	//	{
	//		if (_imgAcqPty.threePhotonModeEnable)
	//		{
	//			if (_deviceNum != 0)
	//			{
	//				ULONG32 clock_rate = 0;
	//				ULONG32 clock_ref = 0;
	//				long error = 0, status = 0;
	//				 
	//				ThordaqErrChk2 (L"ThordaqMeasureExternClockRate", status = ThordaqMeasureExternClockRate(_DAQDeviceIndex, clock_rate,clock_ref, 1));
	//				if(THORDAQ_STATUS::STATUS_SUCCESSFUL == status)
	//				{
	//					_imgAcqPty.clockRateExternal = clock_rate;
	//					_imgAcqPty.maxSampleRate = static_cast<long>(clock_ref * 2.0);
	//				}
	//			}
	//		}else
	//		{
	//			_imgAcqPty.maxSampleRate = DEFAULT_INTERNALCLOCKRATE;
	//		}
	//		param = _imgAcqPty.clockRateExternal;
	//	}
	//	break;
	case ICamera::PARAM_TRIGGER_MODE:
		{
			param  = _imgAcqPty.triggerMode;
		}
		break;
	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			param  = static_cast<double>(_imgAcqPty.numFrame);
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
			param = _imgAcqPty.dwellTime;
		}
		break;

	case ICamera::PARAM_LSM_DWELL_TIME_STEP:
		{
			param = DWELL_TIME_STEP;
		}
		break;

	case ICamera::PARAM_LSM_GALVO_ENABLE:
		{
			param = _imgAcqPty.galvoEnable;
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
		{
			long index = 0;

			//linearize the sine wave response of the pockels cell
			const double AREA_UNDER_CURVE = 2.0;

			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:index = 0;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:index = 1;break;
			case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:index = 2;break;
			}

			switch(_pockelsResponseType[index])
			{
			case 0:
				{
					//sinusoidal response
					param = 100.0*(1 - cos(M_PI * _imgAcqPty.pockelPty.pockelsPowerLevel[index]))/AREA_UNDER_CURVE;
				}
				break;
			case 1:
				{
					//linear response
					param = 100.0*_imgAcqPty.pockelPty.pockelsPowerLevel[index];
				}
				break;
			}
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0:
		{
			param = _imgAcqPty.pockelPty.pockelsLineBlankingPercentage[0]*100.0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1:
		{
			param = _imgAcqPty.pockelPty.pockelsLineBlankingPercentage[1]*100.0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2:
		{
			param = _imgAcqPty.pockelPty.pockelsLineBlankingPercentage[2]*100.0;
		}
		break;
	//case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3:
	//	{
	//		//param = _imgAcqPty._pockelsLineBlankingPercentage[3]*100.0;
	//	}
	//	break;
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
			param = (_imgAcqPty.verticalScanDirection < 0)? 1 : 0;
		}
		break;
	case ICamera::PARAM_LSM_HORIZONTAL_FLIP:
		{
			param = static_cast<double>(_imgAcqPty.horizontalFlip);
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
			param =	_imgAcqPty.fineFieldSizeScaleX;
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y:
		{
			param =	_imgAcqPty.fineFieldSizeScaleY;
		}
		break;
	case ICamera::PARAM_FRAME_RATE:
		{
			param =  _frameRate;
		}
		break;
	case ICamera::PARAM_LSM_1X_FIELD_SIZE:
		{
			param = _oneXFieldSize;
		}
		break;
	case ICamera:: PARAM_LSM_PULSE_MULTIPLEXING_ENABLE:
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
	//case ICamera::PARAM_LSM_POCKELS_FIND_MIN_MAX_3:
		{
			param = 1.0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:
	//case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:index=2;break;
			//case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:index=3;break;
			}
			param = _pockelsMinVoltage[index];
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:
	//case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:index=2;break;
			//case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:index=3;break;
			}
			param = _pockelsMaxVoltage[index];
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_2:
	//case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_2:index=2;break;
			//case ICamera::PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_3:index=3;break;
			}
			param = _pockelsScanVoltageStart[index];
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_0:
	case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_1:
	case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_2:
	//case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_2:index=2;break;
			//case ICamera::PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_3:index=3;break;
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
	/*case ICamera::PARAM_LSM_POCKELS_CONNECTED_3:
		{
			param = _pockelsEnable[3];
		}
		break;*/
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
	case ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF:
		{
			param = _imgAcqPty.pockelPty.useReferenceForPockelsOutput;
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
	case ICamera::PARAM_DFLIM_ACQUISITION_MODE:
		{
			param = _imgAcqPty.acquistionMode;
		}
		break;
	case ICamera::PARAM_DFLIM_COARSE_SHIFT1:
		{
			param = _dflimCoarseShiftA[0];
		}
		break;
	case ICamera::PARAM_DFLIM_COARSE_SHIFT2:
		{
			param = _dflimCoarseShiftA[1];
		}
		break;
	case ICamera::PARAM_DFLIM_COARSE_SHIFT3:
		{
			param = _dflimCoarseShiftA[2];
		}
		break;
	case ICamera::PARAM_DFLIM_COARSE_SHIFT4:
		{
			param = _dflimCoarseShiftA[3];
		}
		break;
	case ICamera::PARAM_DFLIM_FINE_SHIFT1:
		{
			param = _dflimFineShiftA[0];
		}
		break;
	case ICamera::PARAM_DFLIM_FINE_SHIFT2:
		{
			param = _dflimFineShiftA[1];
		}
		break;
	case ICamera::PARAM_DFLIM_FINE_SHIFT3:
		{
			param = _dflimFineShiftA[2];
		}
		break;
	case ICamera::PARAM_DFLIM_FINE_SHIFT4:
		{
			param = _dflimFineShiftA[3];
		}
		break;
	case ICamera::PARAM_DFLIM_FREQ_CLOCK0:
		{
			param = _dflimClockFrequencies[0];
		}
		break;
	case ICamera::PARAM_DFLIM_FREQ_CLOCK1:
		{
			param = _dflimClockFrequencies[1];
		}
		break;
	case ICamera::PARAM_DFLIM_FREQ_CLOCK2:
		{
			param = _dflimClockFrequencies[2];
		}
		break;
	case ICamera::PARAM_DFLIM_FREQ_CLOCK3:
		{
			param = _dflimClockFrequencies[3];
		}
		break;
	case ICamera::PARAM_DFLIM_FREQ_CLOCK4:
		{
			param = _dflimClockFrequencies[4];
		}
		break;
	case ICamera::PARAM_DFLIM_IMPLIED_FREQ_CLOCK0:
		{
			const double NOMINAL = 200.0; // MHz
			param = 80 * _dflimClockFrequencies[0] / NOMINAL;
		}
		break;
	case ICamera::PARAM_DFLIM_IMPLIED_FREQ_CLOCK1:
		{
			const double NOMINAL = 0.625; // MHz
			param = 80 * _dflimClockFrequencies[1] / NOMINAL;
		}
		break;
	case ICamera::PARAM_DFLIM_IMPLIED_FREQ_CLOCK2:
		{
			const double NOMINAL = 150.625; // MHz
			param = 80 * _dflimClockFrequencies[2] / NOMINAL;
		}
		break;
	case ICamera::PARAM_DFLIM_IMPLIED_FREQ_CLOCK3:
		{
			const double NOMINAL = 602.5; // MHz
			param = 80 * _dflimClockFrequencies[3] / NOMINAL;
		}
		break;
	case ICamera::PARAM_DFLIM_IMPLIED_FREQ_CLOCK4:
		{
			const double NOMINAL = 5; // MHz
			param = 80 * _dflimClockFrequencies[4] / NOMINAL;
		}
		break;
	case ICamera::PARAM_DFLIM_SYNC_DELAY:
		{
			param = _dflimSyncDelay;
		}
		break;
	case ICamera::PARAM_DFLIM_RESYNC_DELAY:
		{
			param = _dflimResyncDelay;
		}
		break;
	case ICamera::PARAM_DFLIM_RESYNC_EVERYLINE:
		{
			param = _dflimResyncEveryLine;
		}
		break;
	case ICamera::PARAM_DFLIM_SAVE_IMAGES_ON_LIVE_MODE:
		{
			param = _saveLiveImage;
		}
		break;
	case ICamera::PARAM_DFLIM_FRAME_TYPE:
		{
			param = 1;
		}
		break;
	default:
		{
			ret = FALSE;
		}
	}

	if(FALSE == ret)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) not implemented",paramID);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	return ret;
}