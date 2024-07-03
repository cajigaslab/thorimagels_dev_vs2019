#include "stdafx.h"
#include "thordaqResonantGalvo.h"
#include "thordaqResonantGalvoSetupXML.h"

wchar_t message2[MSG_SIZE];

long CThordaqResonantGalvo::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
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
			paramMin = ICamera::GALVO_RESONANCE;
			paramMax = ICamera::STIMULATE_MODULATOR;
			paramDefault = ICamera::GALVO_RESONANCE;
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
			int skippedTopLines = _imgAcqPty.scanMode == TWO_WAY_SCAN ? _imgAcqPty.preImagingCalibrationCycles * 2 : _imgAcqPty.preImagingCalibrationCycles;
			int skippedBottomLines = PIXEL_Y_MULTIPLE - skippedTopLines % PIXEL_Y_MULTIPLE;
			int totalSkippedLines = skippedTopLines + skippedBottomLines;

			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_PIXEL_Y;
			paramMax = MAX_PIXEL_Y - totalSkippedLines;
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
			paramAvailable = TRUE == _rGGMode;
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
			paramMin = MIN_ALIGNMENT / 128;
			paramMax = MAX_ALIGNMENT / 128;
			paramDefault = MIN_ALIGNMENT / 128;
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
			paramReadOnly = LSMGRRotationAngle::DEG_0  == _rotationAnglePosition || LSMGRRotationAngle::DEG_180 == _rotationAnglePosition ? FALSE : TRUE;
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
	case ICamera::PARAM_LSM_RESET_FLYBACK_ENABLE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_RESET_FLYBACK_ENABLE;
			paramMax = MAX_RESET_FLYBACK_ENABLE;
			paramDefault = DEFAULT_RESET_FLYBACK_ENABLE;
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
	case ICamera:: PARAM_LSM_PULSE_MULTIPLEXING_ENABLE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = FALSE;
			paramMax = TRUE;
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
	case ICamera::PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 1;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_CAPTURE_WITHOUT_LINE_TRIGGER:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_1:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_2:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_3:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_4:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_5:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_6:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_7:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_8:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_9:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_10:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_11:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_12:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_13:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_14:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_15:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_16:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_17:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_18:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_19:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_20:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_21:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_22:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_23:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_24:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_25:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_26:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_27:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_28:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_29:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_30:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_31:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_32:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_33:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_34:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_35:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_36:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_37:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_38:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_39:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_40:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_41:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_42:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_43:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_44:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_45:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_46:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_47:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_48:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_49:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_50:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_51:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_52:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_53:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_54:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_55:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_56:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_57:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_58:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_59:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_60:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_61:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_62:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_63:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_64:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_65:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_66:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_67:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_68:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_69:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_70:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_71:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_72:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_73:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_74:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_75:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_76:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_77:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_78:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_79:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_80:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_81:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_82:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_83:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_84:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_85:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_86:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_87:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_88:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_89:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_90:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_91:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_92:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_93:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_94:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_95:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_96:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_97:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_98:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_99:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_100:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_101:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_102:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_103:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_104:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_105:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_106:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_107:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_108:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_109:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_110:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_111:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_112:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_113:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_114:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_115:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_116:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_117:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_118:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_119:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_120:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_121:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_122:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_123:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_124:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_125:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_126:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_127:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_128:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_129:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_130:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_131:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_132:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_133:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_134:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_135:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_136:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_137:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_138:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_139:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_140:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_141:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_142:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_143:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_144:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_145:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_146:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_147:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_148:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_149:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_150:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_151:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_152:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_153:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_154:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_155:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_156:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_157:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_158:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_159:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_160:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_161:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_162:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_163:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_164:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_165:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_166:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_167:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_168:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_169:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_170:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_171:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_172:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_173:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_174:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_175:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_176:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_177:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_178:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_179:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_180:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_181:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_182:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_183:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_184:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_185:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_186:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_187:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_188:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_189:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_190:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_191:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_192:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_193:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_194:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_195:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_196:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_197:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_198:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_199:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_200:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_201:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_202:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_203:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_204:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_205:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_206:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_207:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_208:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_209:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_210:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_211:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_212:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_213:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_214:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_215:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_216:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_217:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_218:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_219:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_220:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_221:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_222:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_223:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_224:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_225:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_226:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_227:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_228:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_229:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_230:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_231:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_232:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_233:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_234:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_235:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_236:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_237:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_238:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_239:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_240:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_241:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_242:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_243:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_244:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_245:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_246:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_247:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_248:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_249:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_250:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_251:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 255;
			paramDefault = 128;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_1:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_2:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_3:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_4:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_5:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_6:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_7:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_8:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_9:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_10:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_11:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_12:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_13:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_14:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_15:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_16:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_17:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_18:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_19:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_20:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_21:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_22:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_23:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_24:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_25:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_26:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_27:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_28:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_29:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_30:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_31:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_32:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_33:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_34:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_35:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_36:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_37:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_38:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_39:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_40:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_41:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_42:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_43:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_44:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_45:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_46:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_47:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_48:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_49:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_50:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_51:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_52:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_53:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_54:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_55:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_56:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_57:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_58:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_59:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_60:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_61:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_62:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_63:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_64:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_65:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_66:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_67:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_68:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_69:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_70:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_71:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_72:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_73:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_74:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_75:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_76:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_77:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_78:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_79:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_80:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_81:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_82:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_83:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_84:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_85:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_86:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_87:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_88:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_89:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_90:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_91:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_92:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_93:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_94:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_95:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_96:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_97:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_98:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_99:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_100:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_101:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_102:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_103:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_104:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_105:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_106:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_107:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_108:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_109:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_110:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_111:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_112:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_113:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_114:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_115:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_116:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_117:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_118:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_119:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_120:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_121:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_122:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_123:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_124:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_125:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_126:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_127:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_128:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_129:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_130:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_131:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_132:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_133:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_134:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_135:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_136:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_137:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_138:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_139:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_140:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_141:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_142:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_143:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_144:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_145:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_146:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_147:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_148:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_149:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_150:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_151:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_152:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_153:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_154:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_155:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_156:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_157:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_158:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_159:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_160:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_161:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_162:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_163:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_164:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_165:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_166:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_167:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_168:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_169:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_170:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_171:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_172:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_173:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_174:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_175:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_176:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_177:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_178:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_179:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_180:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_181:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_182:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_183:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_184:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_185:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_186:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_187:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_188:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_189:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_190:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_191:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_192:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_193:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_194:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_195:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_196:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_197:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_198:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_199:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_200:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_201:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_202:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_203:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_204:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_205:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_206:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_207:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_208:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_209:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_210:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_211:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_212:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_213:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_214:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_215:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_216:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_217:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_218:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_219:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_220:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_221:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_222:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_223:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_224:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_225:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_226:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_227:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_228:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_229:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_230:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_231:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_232:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_233:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_234:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_235:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_236:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_237:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_238:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_239:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_240:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_241:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_242:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_243:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_244:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_245:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_246:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_247:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_248:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_249:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_250:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_251:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = (double)TWO_WAY_FINE_ALIGNMENT_OFFSET * -1.0;
			paramMax = (double)SYS_CLOCK_FREQ /_crsFrequencyHighPrecision/2.0 - (double)TWO_WAY_FINE_ALIGNMENT_OFFSET;
			paramDefault = 0;
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
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = -10.0;
			paramMax = 10.0;
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
	/*case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_0:
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
		break;*/
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
	case ICamera::PARAM_LSM_POCKELS_BLANKING_PHASESHIFT_PERCENT:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMax = 100;
		paramMin = 0;
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
	case ICamera::PARAM_LIGHTPATH_TYPE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = LIGHTPATHTYPE_FIRT;
		paramMax = LIGHTPATHTYPE_LAST -1;
		paramDefault = GALVO_RESONANCE;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_CURRENT_CRS_FREQUENCY:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 50000;
		paramDefault = GALVO_RESONANCE;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_MESO_STRIP_COUNT:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = LONG_MAX;
		paramDefault = 0;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_MESO_EXP_PATH:
	{
		paramType = ICamera::TYPE_STRING;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_MESO_SCAN_INFO:
	{
		paramType = ICamera::TYPE_BUFFER;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_MROI_MODE_ENABLE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION_XML:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_FIELD_SIZE_X;
		paramMax = MAX_FIELD_SIZE_X;
		paramDefault = 120;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_MROI_TOTAL_LINES:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_PIXEL_Y;
		paramMax = MAX_PIXEL_Y;
		paramDefault = 2048;
		paramReadOnly = TRUE;
	}
	break;
	case ICamera::PARAM_LSM_PREIMAGING_CALIBRATION_CYCLES:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_PREIMAGING_CALIBRATION_CYCLES;
		paramMax = MAX_PREIMAGING_CALIBRATION_CYCLES;
		paramDefault = DEFAULT_PREIMAGING_CALIBRATION_CYCLES;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_IMAGING_RAMP_EXTENSION_CYCLES:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = MIN_IMAGING_RAMP_EXTENSION_CYCLES;
		paramMax = MAX_IMAGING_RAMP_EXTENSION_CYCLES;
		paramDefault = DEFAULT_IMAGING_RAMP_EXTENSION_CYCLES;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_ON_FLYBACK:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_ENABLE:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_GALVO_TILT_ANGLE:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMin = 0.0;
		paramMax = 60.0;
		paramDefault = 24.0;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_X_ANGLE_MAX:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMin = 0.0;
		paramMax = 10;
		paramDefault = 3.5;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_Y_ANGLE_MAX:
	{
		paramType = ICamera::TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramMin = 0.0;
		paramMax = 10;
		paramDefault = 3.0;
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

long CThordaqResonantGalvo::SetParam(const long paramID, const double param)
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

			int skippedTopLines = _imgAcqPty.scanMode == TWO_WAY_SCAN ? _imgAcqPty.preImagingCalibrationCycles * 2 : _imgAcqPty.preImagingCalibrationCycles;
			int skippedBottomLines = PIXEL_Y_MULTIPLE - skippedTopLines % PIXEL_Y_MULTIPLE;
			int totalSkippedLines = skippedTopLines + skippedBottomLines;

			if (_imgAcqPty.pixelY + totalSkippedLines > MAX_PIXEL_Y)
			{
				_imgAcqPty.pixelY = MAX_PIXEL_Y - totalSkippedLines;
			}
		}
		break;
	case ICamera::PARAM_LSM_AREAMODE:
		{
			if ((param >= MIN_AREAMODE) && (param <= MAX_AREAMODE) && (LSMGRRotationAngle::DEG_0  == _rotationAnglePosition || LSMGRRotationAngle::DEG_180 == _rotationAnglePosition))
			{
				_imgAcqPty.areaMode = static_cast<long> (param);
				ret = TRUE;
			}
			else if (ICamera::SQUARE == static_cast<long> (param))
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
			//long factor;
			//if (_imgAcqPty.scanMode == TWO_WAY_SCAN)
			//{
			//	factor = 2;
			//}
			//else
			//{
			//	factor = 1;
			//}

			if ((param >= MIN_PIXEL_X) && (param <= MAX_PIXEL_X))
			{
				//if (static_cast<long>(param) % 4 == 0)
				{
					_imgAcqPty.pixelX = static_cast<long>(param) - static_cast<long>(param) % 4;

					if (LSMGRRotationAngle::DEG_0  != _rotationAnglePosition && LSMGRRotationAngle::DEG_180 != _rotationAnglePosition)
					{
						_imgAcqPty.areaMode = ICamera::SQUARE;
					}

					if(ICamera::SQUARE == _imgAcqPty.areaMode)
					{
						_imgAcqPty.pixelY = _imgAcqPty.pixelX;
					}
					ret = TRUE;
				}
				//else
				//{
				//	StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_PIXEL_X must be a multiple of 16");
				//	LogMessage(_errMsg,ERROR_EVENT);
				//}
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_PIXEL_X %d outside range %d to %d",static_cast<long> (param), MIN_PIXEL_X, static_cast<long>(MAX_PIXEL_X));
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_Y:
		{
			//long factor;
			//if (_imgAcqPty.scanMode == TWO_WAY_SCAN)
			//{
			//	factor = 2;
			//}
			//else
			//{
			//	factor = 1;
			//}

			if (LSMGRRotationAngle::DEG_0  != _rotationAnglePosition && LSMGRRotationAngle::DEG_180 != _rotationAnglePosition)
			{
				_imgAcqPty.areaMode = ICamera::SQUARE;
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
			else // rectangle
			{
				if ((param >= MIN_PIXEL_Y) && (param <= MAX_PIXEL_Y))
				{
					if (static_cast<long>(param) % PIXEL_Y_MULTIPLE == 0)
					{
						//allow PixelY > PixelX only when Y Galvo is disabled:
						if((static_cast<long>(param) > _imgAcqPty.pixelX) && _imgAcqPty.galvoEnable == TRUE)
						{
							StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_PIXEL_Y cannot be greater than PARAM_LSM_PIXEL_X");
							LogMessage(_errMsg,ERROR_EVENT);
						}
						else
						{
							double fieldY_volt;
							double theta = _imgAcqPty.fieldSize * _field2Theta;
							fieldY_volt = theta * (double)param / (double)_imgAcqPty.pixelX / 2; // divide by 2 because the mechanical angle is half of the optical angle
							if (FALSE == _imgAcqPty.galvoEnable && TRUE == _pockelsEnable[0]) //if LineScan is enabled
							{
								fieldY_volt = 0;
							}
							if ((fieldY_volt <= 20.0) || (TRUE == _imgAcqPty.resetFlybackEnable))
							{
								_imgAcqPty.pixelY = static_cast<long>(param);
								ret = TRUE;
							}
						}
					}
					else
					{
						StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_PIXEL_Y must be a multiple of 4");
						LogMessage(_errMsg,ERROR_EVENT);
					}
				}
				else
				{
					StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_PIXEL_Y %d outside range %d to %d",static_cast<long> (param), MIN_PIXEL_Y, static_cast<long>(MAX_PIXEL_Y));
					LogMessage(_errMsg,ERROR_EVENT);
				}
			}
		}
		break;
	case ICamera::PARAM_LSM_FIELD_SIZE:
		{
			if ((param >= _fieldSizeMin) && (param <= _fieldSizeMax))
			{
				double fieldY_volt;
				double theta = param * _field2Theta;
				fieldY_volt = theta * (double) _imgAcqPty.pixelY / (double)_imgAcqPty.pixelX / 2; // divide by 2 because the mechanical angle is half of the optical angle
				if (FALSE == _imgAcqPty.galvoEnable && TRUE == _pockelsEnable[0]) //if LineScan is enabled
				{
					fieldY_volt = 0;
				}
				if ((fieldY_volt <= 20.0) || (TRUE == _imgAcqPty.resetFlybackEnable))
				{
					_imgAcqPty.fieldSize = static_cast<long>(param);
					ret = TRUE;
				}
				else
				{
					StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_FIELD_SIZE (%d) results in a voltage that is out of range (%d.%d) pixX (%d) pixY (%d) fToT (%d)",static_cast<long>(param),static_cast<long>(fieldY_volt),static_cast<long>((fieldY_volt - static_cast<long>(fieldY_volt))*1000),_imgAcqPty.pixelX,_imgAcqPty.pixelY,static_cast<long>(_field2Theta*1000));
					LogMessage(_errMsg,ERROR_EVENT);
				}
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_FIELD_SIZE %d outside range %d to %d",static_cast<long> (param), _fieldSizeMin,_fieldSizeMax);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_X:
		{
			if (_rGGMode)
			{
				if ((param >= -(_fieldSizeMax - _imgAcqPty.fieldSize) / 2) && (param <= (_fieldSizeMax - _imgAcqPty.fieldSize) / 2))
				{
					_imgAcqPty.offsetX = static_cast<long>(param);
					ret = TRUE;
				}
				else
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_OFFSET_X %d outside range %d to %d", static_cast<long> (param), -(_fieldSizeMax - _imgAcqPty.fieldSize) / 2, (_fieldSizeMax - _imgAcqPty.fieldSize) / 2);
					LogMessage(_errMsg, ERROR_EVENT);
				}
			}
			else
			{
				_imgAcqPty.offsetX = 0;
			}
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_Y:
		{
			//if ((param >= -(_fieldSizeMax - _imgAcqPty.fieldSize) / 2) && (param <= (_fieldSizeMax - _imgAcqPty.fieldSize) / 2))
			if (((param >= (2 * MIN_GALVO_VOLTAGE / _field2Theta + _imgAcqPty.fieldSize * _imgAcqPty.pixelY / _imgAcqPty.pixelX /2)) && (param <= (2 * MAX_GALVO_VOLTAGE / _field2Theta - _imgAcqPty.fieldSize * _imgAcqPty.pixelY / _imgAcqPty.pixelX /2))) || (TRUE == _imgAcqPty.resetFlybackEnable))
			{
				_imgAcqPty.offsetY = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_OFFSET_Y %d outside range %d to %d",static_cast<long> (param), -(_fieldSizeMax - _imgAcqPty.fieldSize) / 2,(_fieldSizeMax - _imgAcqPty.fieldSize) / 2);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_ALIGNMENT:
		{
			if ((param >= MIN_ALIGNMENT/ 128) && (param <= MAX_ALIGNMENT/ 128))
			{
				_imgAcqPty.alignmentForField = static_cast<long>(param) * 128;
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_ALIGNMENT %d outside range %d to %d",static_cast<long> (param), MIN_ALIGNMENT,MAX_ALIGNMENT);
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
			const double lineTime = 1 / _crsFrequencyHighPrecision;
			const double flybackTime = lineTime * param;

			if ((MIN_FLYBACK_CYCLE <= param  ) && (MAX_FLYBACK_CYCLE >= param) && (MAX_FLYBACK_TIME >= flybackTime))
			{

				SetFlybackCycle(static_cast<long>(param));
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_FLYBACK_CYCLE %d outside range %d to %d, and above %f seconds",static_cast<long> (param), MIN_FLYBACK_CYCLE,MAX_FLYBACK_CYCLE,MAX_FLYBACK_TIME);
				LogMessage(_errMsg,ERROR_EVENT);
			}
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_GALVO_ENABLE %d outside range %d to %d",static_cast<long> (param), MIN_GALVO_ENABLE,MAX_GALVO_ENABLE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
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
				
				ThordaqErrChk (L"ThorDAQAPISetAllADCChannelsGain", retVal = ThorDAQAPISetAllADCChannelsGain(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE, _imgAcqPty.ADCGain, true));
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
				
				ThordaqErrChk (L"ThorDAQAPISetAllADCChannelsGain", retVal = ThorDAQAPISetAllADCChannelsGain(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE, _imgAcqPty.ADCGain, true));
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
				
				ThordaqErrChk (L"ThorDAQAPISetAllADCChannelsGain", retVal = ThorDAQAPISetAllADCChannelsGain(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE, _imgAcqPty.ADCGain, true));
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
				
				ThordaqErrChk (L"ThorDAQAPISetAllADCChannelsGain", retVal = ThorDAQAPISetAllADCChannelsGain(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE, _imgAcqPty.ADCGain, true));
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_CLOCKSOURCE:
		{
			if (_imgAcqPty.clockSource != static_cast<long> (param))
			{
				_imgAcqPty.clockSource = static_cast<long> (param);
				if (_deviceNum != 0)
				{
					int32 error = 0, retVal = 0;
					if (_imgAcqPty.clockSource == INTERNAL_CLOCK)
					{
						ThordaqErrChk (L"ThorDAQAPISetClockSource", retVal = ThorDAQAPISetClockSource(_DAQDeviceIndex, CLOCK_SOURCE::INTERNAL_80MHZ_REF));
						ThordaqErrChk (L"ThorDAQAPISetGRClockRate", retVal = ThorDAQAPISetGRClockRate(_DAQDeviceIndex,DEFAULT_INTERNALCLOCKRATE, static_cast<ULONG32>(_crsFrequencyHighPrecision)));
					}
					else
					{
						ThordaqErrChk (L"ThorDAQAPISetClockSource", retVal = ThorDAQAPISetClockSource(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE));

						ULONG32 clock_rate = 0;
						ULONG32 clock_ref = 0;
						ULONG32 mode =  0;

						ThordaqErrChk (L"ThorDAQAPIMeasureExternClockRate", retVal = ThorDAQAPIMeasureExternClockRate(_DAQDeviceIndex, clock_rate,clock_ref, mode));

						if(retVal == THORDAQ_STATUS::STATUS_SUCCESSFUL)
						{
							_imgAcqPty.clockRateExternal = clock_rate;
							ThordaqErrChk (L"ThorDAQAPISetGRClockRate", retVal = ThorDAQAPISetGRClockRate(_DAQDeviceIndex, 2 * _imgAcqPty.clockRateExternal, static_cast<ULONG32>(_crsFrequencyHighPrecision)));
						}
					}					
				}
				// When clock source changes, the board internally changes the ADC gain to 21dB. The GUI needs to reflect this change
				for(int i = 0; i < MAX_CHANNEL_COUNT; i++)
				{
					_imgAcqPty.inputRange[i] = 3;
				}
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
	case ICamera::PARAM_LSM_EXTERNALCLOCKRATE:
		{
			//Leaving code here in case we want to be able to test different rates entered manually
			//like we do in the GG
			//if ((param >= MIN_EXTCLOCKRATE) && (param <= MAX_EXTCLOCKRATE))
			//{
			//	_imgAcqPty.clockRateExternal = static_cast<int> (param);
			//	if (_deviceNum != 0)
			//	{
			//		if (_imgAcqPty.clockSource == EXTERNAL_CLOCK)
			//		{
			//			ThorDAQAPISetClockSource(_DAQDeviceIndex, CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE, 2 * _imgAcqPty.clockRateExternal);
			//		}
			//		
			//	}
			//	ret = TRUE;
			//}
			//else
			//{
			//	StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_EXTERNALCLOCKRATE %d outside range %d to %d", static_cast<int>(param), MIN_EXTCLOCKRATE,MAX_EXTCLOCKRATE);
			//	LogMessage(_errMsg,ERROR_EVENT);
			//}
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
				_imgAcqPty.frameTriggerEnableWithHWTrig = static_cast<long> (param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_ENABLE_FRAME_TRIGGER_WITH_HW_TRIG %d outside range %d to %d",static_cast<long> (param), MIN_ENABLE_FRAME_TRIGGER,MAX_ENABLE_FRAME_TRIGGER);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;

	case ICamera::PARAM_LSM_APPEND_INDEX_TO_FRAME:
		{
			/*if ((param >= FALSE) && (param <= TRUE))
			{
				_appendIndexToFrame = static_cast<long> (param);*/
				ret = TRUE;
			/*}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_APPEND_INDEX_TO_FRAME %d outside range %d to %d",static_cast<long> (param), FALSE,TRUE);
				LogMessage(_errMsg,ERROR_EVENT);
			}*/
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
				case static_cast<long>(PockelsResponseType::SINE_RESPONSE):
					{
						//linearize the sine wave response of the pockels cell
						_imgAcqPty.pockelPty.pockelsPowerLevel[index] = acos(1 - static_cast<double>(Constants::AREA_UNDER_CURVE) * param/100.0)/M_PI;
					}
					break;
				case static_cast<long>(PockelsResponseType::LINEAR_RESPONSE):
					{
						//linear response
						_imgAcqPty.pockelPty.pockelsPowerLevel[index] = param/100.0;
					}
					break;
				}

				ret = TRUE;

				//the scanner is in centering mode. allow the power to be changed instantaneously
				if(_imgAcqPty.scanMode == ScanMode::CENTER)
				{
					MovePockelsToPowerLevel(&_imgAcqPty.pockelPty);
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
	case ICamera::PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_imgAcqPty.realTimeDataAverage = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE %d outside range %d to %d",static_cast<long> (param), 0,1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_CAPTURE_WITHOUT_LINE_TRIGGER:
		{
			if((param >= 0) && (param <= 1))
			{
				_imgAcqPty.captureWithoutLineTrigger = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_CAPTURE_WITHOUT_LINE_TRIGGER %d outside range %d to %d",static_cast<long> (param), 0,1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_0:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_1:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_2:
	case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_2:index=2;break;
			case ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_3:index=3;break;
			}

			if((param >= 0) && (param <= 1))
			{
				_imgAcqPty.pockelPty.pockelsMaskEnable[index] = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_POCKELS_MASK_ENABLE %d outside range %d to %d",static_cast<long> (param), 0,1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_0:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_1:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_2:
	case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_3:
		{
			long index = 0;

			switch( paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_0:index=0;break;
			case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_1:index=1;break;
			case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_2:index=2;break;
			case ICamera::PARAM_LSM_POCKELS_MASK_INVERT_3:index=3;break;
			}

			if((param >= 0) && (param <= 1))
			{
				_imgAcqPty.pockelPty.pockelsMaskInvert[index] = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_POCKELS_MASK_INVERT %d outside range %d to %d",static_cast<long> (param), 0,1);
				LogMessage(_errMsg,ERROR_EVENT);
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
			_imgAcqPty.pockelPty.pockelsMinVoltage[index] = param;
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
			_imgAcqPty.pockelPty.pockelsMaxVoltage[index] = param;
		}
		break;
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_1:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_2:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_3:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_4:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_5:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_6:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_7:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_8:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_9:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_10:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_11:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_12:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_13:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_14:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_15:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_16:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_17:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_18:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_19:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_20:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_21:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_22:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_23:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_24:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_25:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_26:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_27:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_28:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_29:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_30:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_31:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_32:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_33:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_34:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_35:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_36:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_37:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_38:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_39:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_40:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_41:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_42:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_43:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_44:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_45:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_46:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_47:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_48:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_49:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_50:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_51:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_52:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_53:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_54:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_55:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_56:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_57:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_58:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_59:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_60:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_61:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_62:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_63:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_64:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_65:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_66:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_67:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_68:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_69:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_70:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_71:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_72:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_73:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_74:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_75:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_76:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_77:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_78:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_79:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_80:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_81:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_82:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_83:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_84:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_85:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_86:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_87:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_88:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_89:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_90:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_91:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_92:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_93:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_94:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_95:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_96:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_97:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_98:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_99:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_100:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_101:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_102:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_103:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_104:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_105:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_106:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_107:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_108:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_109:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_110:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_111:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_112:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_113:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_114:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_115:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_116:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_117:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_118:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_119:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_120:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_121:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_122:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_123:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_124:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_125:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_126:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_127:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_128:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_129:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_130:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_131:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_132:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_133:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_134:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_135:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_136:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_137:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_138:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_139:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_140:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_141:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_142:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_143:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_144:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_145:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_146:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_147:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_148:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_149:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_150:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_151:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_152:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_153:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_154:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_155:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_156:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_157:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_158:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_159:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_160:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_161:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_162:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_163:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_164:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_165:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_166:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_167:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_168:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_169:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_170:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_171:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_172:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_173:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_174:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_175:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_176:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_177:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_178:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_179:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_180:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_181:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_182:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_183:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_184:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_185:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_186:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_187:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_188:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_189:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_190:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_191:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_192:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_193:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_194:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_195:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_196:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_197:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_198:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_199:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_200:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_201:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_202:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_203:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_204:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_205:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_206:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_207:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_208:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_209:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_210:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_211:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_212:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_213:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_214:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_215:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_216:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_217:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_218:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_219:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_220:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_221:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_222:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_223:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_224:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_225:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_226:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_227:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_228:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_229:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_230:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_231:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_232:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_233:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_234:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_235:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_236:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_237:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_238:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_239:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_240:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_241:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_242:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_243:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_244:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_245:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_246:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_247:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_248:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_249:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_250:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_251:
		{
			if((param >= 0) && (param < 255))
			{
				_imgAcqPty.twoWayZones[paramID - ICamera::PARAM_LSM_TWO_WAY_ZONE_1] = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_TWO_WAY_ZONE %d outside range %d to %d",static_cast<long> (param), 0,255);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_1:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_2:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_3:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_4:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_5:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_6:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_7:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_8:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_9:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_10:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_11:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_12:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_13:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_14:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_15:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_16:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_17:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_18:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_19:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_20:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_21:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_22:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_23:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_24:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_25:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_26:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_27:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_28:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_29:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_30:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_31:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_32:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_33:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_34:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_35:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_36:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_37:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_38:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_39:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_40:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_41:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_42:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_43:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_44:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_45:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_46:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_47:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_48:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_49:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_50:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_51:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_52:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_53:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_54:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_55:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_56:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_57:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_58:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_59:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_60:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_61:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_62:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_63:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_64:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_65:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_66:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_67:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_68:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_69:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_70:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_71:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_72:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_73:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_74:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_75:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_76:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_77:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_78:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_79:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_80:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_81:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_82:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_83:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_84:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_85:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_86:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_87:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_88:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_89:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_90:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_91:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_92:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_93:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_94:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_95:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_96:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_97:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_98:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_99:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_100:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_101:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_102:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_103:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_104:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_105:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_106:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_107:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_108:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_109:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_110:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_111:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_112:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_113:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_114:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_115:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_116:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_117:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_118:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_119:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_120:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_121:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_122:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_123:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_124:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_125:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_126:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_127:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_128:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_129:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_130:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_131:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_132:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_133:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_134:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_135:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_136:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_137:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_138:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_139:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_140:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_141:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_142:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_143:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_144:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_145:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_146:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_147:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_148:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_149:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_150:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_151:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_152:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_153:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_154:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_155:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_156:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_157:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_158:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_159:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_160:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_161:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_162:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_163:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_164:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_165:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_166:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_167:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_168:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_169:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_170:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_171:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_172:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_173:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_174:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_175:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_176:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_177:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_178:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_179:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_180:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_181:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_182:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_183:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_184:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_185:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_186:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_187:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_188:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_189:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_190:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_191:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_192:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_193:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_194:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_195:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_196:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_197:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_198:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_199:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_200:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_201:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_202:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_203:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_204:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_205:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_206:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_207:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_208:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_209:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_210:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_211:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_212:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_213:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_214:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_215:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_216:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_217:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_218:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_219:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_220:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_221:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_222:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_223:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_224:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_225:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_226:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_227:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_228:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_229:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_230:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_231:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_232:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_233:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_234:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_235:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_236:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_237:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_238:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_239:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_240:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_241:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_242:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_243:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_244:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_245:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_246:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_247:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_248:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_249:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_250:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_251:
		{
			if((param >= ((double)TWO_WAY_FINE_ALIGNMENT_OFFSET * -1.0)) && (param < (double)SYS_CLOCK_FREQ /_crsFrequencyHighPrecision/2.0 - (double)TWO_WAY_FINE_ALIGNMENT_OFFSET ))
			{
				_imgAcqPty.twoWayZonesFine[paramID - ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_1] = static_cast<long>(param + (double)TWO_WAY_FINE_ALIGNMENT_OFFSET);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_TWO_WAY_ZONE_FINE %d outside range %d to %d",static_cast<long> (param), ((double)TWO_WAY_FINE_ALIGNMENT_OFFSET * -1.0),(double)SYS_CLOCK_FREQ /_crsFrequencyHighPrecision/2.0 - (double)TWO_WAY_FINE_ALIGNMENT_OFFSET);
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
	case ICamera::PARAM_LSM_RESET_FLYBACK_ENABLE:
		{
			if ((param >= MIN_RESET_FLYBACK_ENABLE) && (param <= MAX_RESET_FLYBACK_ENABLE))
			{
				_imgAcqPty.resetFlybackEnable = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_RESET_FLYBACK_ENABLE %d outside range %d to %d",static_cast<long> (param), MIN_RESET_FLYBACK_ENABLE,MAX_RESET_FLYBACK_ENABLE);
				LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_DMA_BUFFER_COUNT:
		{
			if((param>=MIN_DMA_BUFFER_NUM) && (param <= MAX_DMA_BUFFER_NUM))
			{
				_imgAcqPty.dmaBufferCount = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;	
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_1:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				_imgAcqPty.channelPolarity[0] = static_cast<long>(param);
			}
		}
		break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_2:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				_imgAcqPty.channelPolarity[1] = static_cast<long>(param);
			}
		}
		break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_3:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				_imgAcqPty.channelPolarity[2] = static_cast<long>(param);
			}
		}
		break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_4:
		{
			if((param>=POL_NEG) && (param <= POL_POS))
			{
				_imgAcqPty.channelPolarity[3] = static_cast<long>(param);
			}
		}
		break;
    case ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY:
		{
			_imgAcqPty.rawSaveEnabledChannelOnly = static_cast<long>(param);
		}
		break;
	case ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION:
		{
			if((param >= 0) && (param <= 1))
			{
				_imgAcqPty.verticalScanDirection = static_cast<long>(param);
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
	case ICamera::PARAM_LSM_STOP_ACQUISITION:
		{
			StopHardwareWaits();
		}
		break;
	case ICamera::PARAM_SCANNER_INIT_MODE:
		{
			if((param >= 0) && (param <= 1))
			{
				_scannerInitMode = static_cast<long>(param);
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_SCANNER_INIT_MODE %d outside range %d to %d",static_cast<long> (param), 0,1);
				LogMessage(_errMsg,ERROR_EVENT);
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
				ThordaqErrChk (L"ThorDAQAPISetDCOffsetPreFIR", retVal = ThorDAQAPISetDCOffsetPreFIR(_DAQDeviceIndex,static_cast<short>(ceil(_preDcOffset[channelIndex]/2)),channelIndex));
				if (retVal == STATUS_SUCCESSFUL)
				{
					ret = TRUE;
				}
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_DIG_OFFSET_0 %d outside range %d to %d",static_cast<long> (param), MIN_DIG_OFFSET, MAX_DIG_OFFSET);
				LogMessage(_errMsg,ERROR_EVENT);
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
				ThordaqErrChk (L"ThorDAQAPISetDCOffsetPreFIR", retVal = ThorDAQAPISetDCOffsetPreFIR(_DAQDeviceIndex,static_cast<short>(ceil(_preDcOffset[channelIndex]/2)),channelIndex));
				if (retVal == STATUS_SUCCESSFUL)
				{
					ret = TRUE;
				}
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_DIG_OFFSET_1 %d outside range %d to %d",static_cast<long> (param), MIN_DIG_OFFSET, MAX_DIG_OFFSET);
				LogMessage(_errMsg,ERROR_EVENT);
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
				ThordaqErrChk (L"ThorDAQAPISetDCOffsetPreFIR", retVal = ThorDAQAPISetDCOffsetPreFIR(_DAQDeviceIndex,static_cast<short>(ceil(_preDcOffset[channelIndex]/2)),channelIndex));
				if (retVal == STATUS_SUCCESSFUL)
				{
					ret = TRUE;
				}
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_DIG_OFFSET_2 %d outside range %d to %d",static_cast<long> (param), MIN_DIG_OFFSET, MAX_DIG_OFFSET);
				LogMessage(_errMsg,ERROR_EVENT);
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
				ThordaqErrChk (L"ThorDAQAPISetDCOffsetPreFIR", retVal = ThorDAQAPISetDCOffsetPreFIR(_DAQDeviceIndex,static_cast<short>(ceil(_preDcOffset[channelIndex]/2)),channelIndex));
				if (retVal == STATUS_SUCCESSFUL)
				{
					ret = TRUE;
				}
			}
			else
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"PARAM_LSM_DIG_OFFSET_3 %d outside range %d to %d",static_cast<long> (param), MIN_DIG_OFFSET, MAX_DIG_OFFSET);
				LogMessage(_errMsg,ERROR_EVENT);
			}
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
					ULONG32 mode = 0;

					int32 error = 0, retVal = 0;

					ThordaqErrChk (L"ThorDAQAPIMeasureExternClockRate", retVal = ThorDAQAPIMeasureExternClockRate(_DAQDeviceIndex, clock_rate,clock_ref, mode));

					if(retVal == THORDAQ_STATUS::STATUS_SUCCESSFUL)
					{
						_imgAcqPty.clockRateExternal = clock_rate;
					}
				}
			}

			ret = TRUE;
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
	case ICamera::PARAM_LSM_POCKELS_BLANKING_PHASESHIFT_PERCENT:
	{
		if (0.0 <= param && 100.0 >= param)
		{
			_imgAcqPty.pockelsBlankingPhaseShiftPercent = param;
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
	case PARAM_MESO_RESONANT_AMPLITUDE:
		//_pCameraConfig->GetResonantVoltage(param, data);
		//_hDAQController->InvokeTask(_pCameraConfig->RESONANT_AO_CHANNEL, AO, &data, 1, 1);
		break;
	case ICamera::PARAM_MROI_MODE_ENABLE:
	{
		if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
		{
			_imgAcqPty.mROIModeEnable = TRUE == static_cast<long>(param) && TRUE == _rGGMode;
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_PREIMAGING_CALIBRATION_CYCLES:
	{
		//coarse it to the right values
		if (MIN_PREIMAGING_CALIBRATION_CYCLES <= param && MAX_PREIMAGING_CALIBRATION_CYCLES >= param)
		{
			_imgAcqPty.preImagingCalibrationCycles = (long)param;
			ret = TRUE;
		}
		else if (MIN_PREIMAGING_CALIBRATION_CYCLES > param)
		{
			_imgAcqPty.preImagingCalibrationCycles = MIN_PREIMAGING_CALIBRATION_CYCLES;
			ret = TRUE;
		}
		else if (MAX_PREIMAGING_CALIBRATION_CYCLES < param)
		{
			_imgAcqPty.preImagingCalibrationCycles = MAX_PREIMAGING_CALIBRATION_CYCLES;
			ret = TRUE;
		}

		int skippedTopLines = _imgAcqPty.scanMode == TWO_WAY_SCAN ? _imgAcqPty.preImagingCalibrationCycles * 2 : _imgAcqPty.preImagingCalibrationCycles;
		int skippedBottomLines = PIXEL_Y_MULTIPLE - skippedTopLines % PIXEL_Y_MULTIPLE;
		int totalSkippedLines = skippedTopLines + skippedBottomLines;

		if (_imgAcqPty.pixelY + totalSkippedLines > MAX_PIXEL_Y)
		{
			_imgAcqPty.pixelY = MAX_PIXEL_Y - totalSkippedLines;
		}
	}
	break;
	case ICamera::PARAM_LSM_IMAGING_RAMP_EXTENSION_CYCLES:
	{
		//coarse it to the right values
		if (MIN_IMAGING_RAMP_EXTENSION_CYCLES <= param && MAX_IMAGING_RAMP_EXTENSION_CYCLES >= param)
		{
			_imgAcqPty.imagingRampExtensionCycles = (long)param;
			ret = TRUE;
		}
		else if (MIN_IMAGING_RAMP_EXTENSION_CYCLES > param)
		{
			_imgAcqPty.imagingRampExtensionCycles = MIN_IMAGING_RAMP_EXTENSION_CYCLES;
			ret = TRUE;
		}
		else if (MAX_IMAGING_RAMP_EXTENSION_CYCLES < param)
		{
			_imgAcqPty.imagingRampExtensionCycles = MAX_IMAGING_RAMP_EXTENSION_CYCLES;
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_ON_FLYBACK:
	{
		if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
		{
			_imgAcqPty.imageOnFlyback = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_ENABLE:
	{
		if (TRUE == static_cast<long>(param) || FALSE == static_cast<long>(param))
		{
			_imgAcqPty.enableImageDistortionCorrection = static_cast<long>(param) == TRUE;
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_GALVO_TILT_ANGLE:
	{
		if (0.0 <= param && 60.0 >= param)
		{
			_imgAcqPty.ImageDistortionCorrectionCalibrationGalvoTiltAngle = param;
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_ANGLE_IN %f outside range %f to %f", param, 0, 60);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_X_ANGLE_MAX:
	{
		if (0 <= param && 10 >= param)
		{
			_imgAcqPty.ImageDistortionCorrectionCalibrationXAngleMax = param;
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_X_ANGLE_MAX %f outside range %f to %f", param, 0, 10);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_Y_ANGLE_MAX:
	{
		if (0 <= param && 10 >= param)
		{
			_imgAcqPty.ImageDistortionCorrectionCalibrationYAngleMax = param;
			ret = TRUE;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_Y_ANGLE_MAX %f outside range %f to %f", param, 0, 10);
			LogMessage(_errMsg, ERROR_EVENT);
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

	//Estimate the new frame time with any new set values. This is for RunSample, streaming, estimated time.
	long total_cycles  = (TWO_WAY_SCAN == _imgAcqPty.scanMode) ? (_imgAcqPty.pixelY / 2) + GetFlybackCycle() : _imgAcqPty.pixelY + GetFlybackCycle();
	total_cycles += _imgAcqPty.preImagingCalibrationCycles;
	total_cycles += _imgAcqPty.imagingRampExtensionCycles;	
	if (_imgAcqPty.imageOnFlyback)
	{
		total_cycles += _imgAcqPty.imagingRampExtensionCycles;
	}
	if(0 != total_cycles)
	{
		_frameRate = _current_resonant_scanner_frequency / total_cycles;
	}

	return ret;
}

long CThordaqResonantGalvo::GetParam(const long paramID, double &param)
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
			param = 1 == _rGGMode ? ICamera::RESONANCE_GALVO_GALVO : ICamera::GALVO_RESONANCE;
		}
		break;
	case ICamera::PARAM_LSM_SCANMODE:
		{
			param = _imgAcqPty.scanMode;
		}
		break;
	case ICamera::PARAM_LSM_AREAMODE:
		{
			if ((LSMGRRotationAngle::DEG_0  == _rotationAnglePosition || LSMGRRotationAngle::DEG_180 == _rotationAnglePosition))
			{
				param = _imgAcqPty.areaMode;
			}
			else
			{
				param = LSMAreaMode::SQUARE;
			}
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
			param = _imgAcqPty.alignmentForField / 128.0;
		}
		break;
	case ICamera::PARAM_LSM_Y_AMPLITUDE_SCALER:
		{
			param = _imgAcqPty.yAmplitudeScaler;
		}
		break;
	case ICamera::PARAM_LSM_FLYBACK_CYCLE:
		{
			_imgAcqPty.flybackCycle = GetFlybackCycle();
			param = _imgAcqPty.flybackCycle;
		}
		break;
	case ICamera::PARAM_LSM_FLYBACK_TIME:
		{
			param = GetFlybackTime();
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
			param  = _imgAcqPty.offsetX;
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_Y:
		{
			param  = _imgAcqPty.offsetY;
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
	case ICamera::PARAM_LSM_CLOCKSOURCE:
		{
			param  = _imgAcqPty.clockSource;
		}
		break;
	case ICamera::PARAM_LSM_INTERNALCLOCKRATE:
		{
			param  = _imgAcqPty.clockRateInternal;
		}
		break;
	case ICamera::PARAM_LSM_EXTERNALCLOCKRATE:
		{
			param = _imgAcqPty.clockRateExternal;
		}
		break;
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
			param = _imgAcqPty.frameTriggerEnableWithHWTrig;
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
					param = 100.0*(1 - cos(M_PI * _imgAcqPty.pockelPty.pockelsPowerLevel[index]))/static_cast<double>(Constants::AREA_UNDER_CURVE);
				}
				break;
			case static_cast<long>(PockelsResponseType::LINEAR_RESPONSE):
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
	case ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3:
		{
			param = _imgAcqPty.pockelPty.pockelsLineBlankingPercentage[3]*100.0;
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
			param = _imgAcqPty.pockelPty.pockelsMinVoltage[index];
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
			param =_imgAcqPty.pockelPty.pockelsMaxVoltage[index];
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
	case ICamera::PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE:
		{
			param = _imgAcqPty.realTimeDataAverage;
		}
		break;
	case ICamera::PARAM_LSM_CAPTURE_WITHOUT_LINE_TRIGGER:
		{
			param = _imgAcqPty.captureWithoutLineTrigger;
		}
		break;
	case ICamera::PARAM_LSM_DWELL_TIME:
		{
			const double AREA_UNDER_CURVE = 2.0;
			const double REDUCTION_FACTOR = 1.0;			
			const double FR_CORRECTION = _crsFrequencyHighPrecision/8000.0;
			const double LINE_TIME = 1000000.0/(16000.0) * FR_CORRECTION;
			const double CENTER_PIXEL = static_cast<double>(_imgAcqPty.pixelX)/2.0;
			const double P1 = 1.0/static_cast<double>(_imgAcqPty.pixelX);
			const double P2 = (CENTER_PIXEL - 1) * P1 * 100;
			const double P3 = CENTER_PIXEL * P1 * 100;
			const double P4 = acos(1 - AREA_UNDER_CURVE * REDUCTION_FACTOR * P2/100)/M_PI;
			const double P5 = acos(1 - AREA_UNDER_CURVE * REDUCTION_FACTOR * P3/100)/M_PI;
			param = (P5 - P4) * LINE_TIME;
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
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_1:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_2:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_3:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_4:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_5:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_6:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_7:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_8:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_9:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_10:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_11:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_12:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_13:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_14:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_15:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_16:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_17:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_18:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_19:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_20:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_21:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_22:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_23:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_24:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_25:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_26:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_27:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_28:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_29:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_30:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_31:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_32:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_33:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_34:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_35:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_36:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_37:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_38:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_39:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_40:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_41:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_42:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_43:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_44:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_45:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_46:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_47:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_48:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_49:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_50:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_51:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_52:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_53:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_54:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_55:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_56:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_57:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_58:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_59:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_60:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_61:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_62:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_63:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_64:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_65:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_66:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_67:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_68:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_69:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_70:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_71:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_72:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_73:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_74:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_75:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_76:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_77:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_78:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_79:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_80:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_81:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_82:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_83:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_84:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_85:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_86:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_87:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_88:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_89:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_90:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_91:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_92:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_93:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_94:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_95:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_96:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_97:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_98:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_99:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_100:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_101:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_102:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_103:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_104:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_105:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_106:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_107:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_108:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_109:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_110:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_111:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_112:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_113:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_114:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_115:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_116:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_117:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_118:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_119:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_120:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_121:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_122:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_123:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_124:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_125:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_126:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_127:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_128:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_129:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_130:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_131:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_132:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_133:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_134:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_135:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_136:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_137:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_138:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_139:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_140:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_141:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_142:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_143:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_144:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_145:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_146:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_147:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_148:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_149:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_150:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_151:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_152:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_153:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_154:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_155:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_156:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_157:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_158:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_159:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_160:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_161:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_162:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_163:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_164:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_165:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_166:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_167:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_168:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_169:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_170:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_171:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_172:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_173:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_174:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_175:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_176:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_177:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_178:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_179:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_180:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_181:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_182:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_183:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_184:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_185:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_186:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_187:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_188:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_189:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_190:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_191:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_192:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_193:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_194:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_195:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_196:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_197:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_198:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_199:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_200:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_201:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_202:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_203:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_204:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_205:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_206:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_207:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_208:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_209:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_210:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_211:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_212:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_213:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_214:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_215:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_216:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_217:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_218:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_219:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_220:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_221:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_222:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_223:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_224:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_225:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_226:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_227:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_228:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_229:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_230:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_231:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_232:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_233:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_234:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_235:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_236:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_237:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_238:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_239:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_240:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_241:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_242:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_243:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_244:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_245:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_246:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_247:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_248:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_249:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_250:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_251:
		{
			param = _imgAcqPty.twoWayZones[paramID - ICamera::PARAM_LSM_TWO_WAY_ZONE_1];
		}
		break;
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_1:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_2:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_3:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_4:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_5:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_6:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_7:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_8:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_9:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_10:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_11:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_12:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_13:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_14:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_15:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_16:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_17:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_18:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_19:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_20:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_21:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_22:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_23:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_24:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_25:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_26:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_27:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_28:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_29:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_30:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_31:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_32:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_33:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_34:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_35:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_36:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_37:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_38:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_39:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_40:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_41:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_42:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_43:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_44:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_45:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_46:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_47:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_48:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_49:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_50:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_51:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_52:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_53:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_54:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_55:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_56:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_57:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_58:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_59:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_60:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_61:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_62:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_63:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_64:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_65:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_66:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_67:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_68:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_69:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_70:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_71:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_72:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_73:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_74:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_75:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_76:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_77:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_78:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_79:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_80:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_81:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_82:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_83:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_84:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_85:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_86:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_87:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_88:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_89:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_90:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_91:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_92:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_93:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_94:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_95:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_96:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_97:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_98:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_99:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_100:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_101:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_102:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_103:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_104:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_105:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_106:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_107:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_108:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_109:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_110:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_111:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_112:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_113:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_114:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_115:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_116:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_117:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_118:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_119:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_120:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_121:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_122:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_123:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_124:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_125:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_126:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_127:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_128:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_129:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_130:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_131:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_132:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_133:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_134:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_135:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_136:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_137:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_138:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_139:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_140:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_141:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_142:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_143:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_144:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_145:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_146:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_147:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_148:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_149:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_150:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_151:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_152:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_153:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_154:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_155:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_156:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_157:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_158:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_159:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_160:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_161:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_162:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_163:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_164:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_165:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_166:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_167:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_168:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_169:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_170:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_171:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_172:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_173:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_174:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_175:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_176:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_177:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_178:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_179:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_180:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_181:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_182:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_183:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_184:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_185:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_186:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_187:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_188:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_189:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_190:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_191:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_192:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_193:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_194:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_195:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_196:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_197:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_198:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_199:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_200:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_201:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_202:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_203:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_204:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_205:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_206:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_207:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_208:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_209:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_210:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_211:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_212:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_213:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_214:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_215:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_216:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_217:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_218:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_219:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_220:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_221:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_222:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_223:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_224:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_225:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_226:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_227:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_228:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_229:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_230:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_231:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_232:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_233:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_234:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_235:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_236:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_237:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_238:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_239:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_240:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_241:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_242:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_243:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_244:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_245:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_246:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_247:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_248:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_249:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_250:
	case ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_251:
		{
			param = _imgAcqPty.twoWayZonesFine[paramID - ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_1] - (double)TWO_WAY_FINE_ALIGNMENT_OFFSET;
		}
		break;
	case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
		{
			param = _forceSettingsUpdate;
		}
		break;
	case ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION:
		{
			if(TRUE ==_useZoomArray)
			{
				//field size 120 is used in the calibration
				//adjust the calibration values to scale from the 120 value
				long offset = _zoomArray[120];
				_fieldSizeCalibration = _fieldSizeCalibrationXMLvalue + _fieldSizeCalibrationXMLvalue * (_zoomArray[_imgAcqPty.fieldSize] - offset) / 100.0;
			}
			else
			{ 
				_fieldSizeCalibration = _fieldSizeCalibrationXMLvalue; 
			}
			param = _fieldSizeCalibration;
		}
		break;
	case ICamera::PARAM_LSM_Y_COMMUNICATION_ENABLE:
		{
			param = _imgAcqPty.yChannelEnable;
		}
		break;
	case ICamera::PARAM_LSM_RESET_FLYBACK_ENABLE:
		{
			param = _imgAcqPty.resetFlybackEnable;
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
	case ICamera::PARAM_DROPPED_FRAMES:
		{
			param = 0;
		}
		break;
	case ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY:
		{
			param = _imgAcqPty.rawSaveEnabledChannelOnly;
		}
		break;
	case ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION:
		{
			param = _imgAcqPty.verticalScanDirection;
		}
		break;
	case ICamera::PARAM_LSM_HORIZONTAL_FLIP:
		{
			param = static_cast<double>(_imgAcqPty.horizontalFlip);
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MASK_WIDTH:
		{
			param = static_cast<double>(_imgAcqPty.pixelX);
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
	case ICamera::PARAM_LSM_POCKELS_BLANKING_PHASESHIFT_PERCENT:
	{
		param = _imgAcqPty.pockelsBlankingPhaseShiftPercent;
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
	case ICamera::PARAM_LIGHTPATH_TYPE:
	{
		if (TRUE == _rGGMode)
		{
			param = LightPathType::LIGHTPATHTYPE_RESONANCE_GALVO_GALVO;
		}
		else
		{
			param = LightPathType::LIGHTPATHTYPE_GALVO_RESONANCE;
		}
	}
	break;
	case ICamera::PARAM_LSM_SCANAREA_ANGLE:
	{
		param = _imgAcqPty.scanAreaAngle * 180 / M_PI; // send angle in degrees
	}
	break;
	case ICamera::PARAM_MESO_STRIP_COUNT:
	{
		param = 2;// _hMesoScanWaveform->GetStripCount();
	}
	break;
	case ICamera::PARAM_MROI_MODE_ENABLE:
	{
		param = _imgAcqPty.mROIModeEnable && TRUE == _rGGMode;
	}
	break;
	case ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION_XML:
	{
		param = _fieldSizeCalibrationXMLvalue;
	}
	break;
	case ICamera::PARAM_LSM_CURRENT_CRS_FREQUENCY:
	{
		int32 error = 0, retVal = 0;
		USHORT channelIndex = 2;
		double freq = _crsFrequencyHighPrecision;

		ThordaqErrChk(L"ThorDAQAPIGetLineTriggerFrequency", retVal = ThorDAQAPIGetLineTriggerFrequency(_DAQDeviceIndex, DEFAULT_INTERNALCLOCKRATE, freq, static_cast<ULONG32>(_crsFrequencyHighPrecision)));

		if (retVal == STATUS_SUCCESSFUL)
		{
			param = freq;
		}
		else
		{
			param = 0;
		}
		if (_saveCrsFrequencyToLog)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"GetParam->GetLineTriggerFrequency Samplerate: %ld ,Frequency read: %f, expected frequence: %f", DEFAULT_INTERNALCLOCKRATE, freq, _crsFrequencyHighPrecision);
			CThordaqResonantGalvo::LogMessage(errMsg, ERROR_EVENT);
		}
	}
	break;
	case ICamera::PARAM_MROI_TOTAL_LINES:
	{
		param = _totalLinesFormROI;
	}
	break;
	case ICamera::PARAM_LSM_PREIMAGING_CALIBRATION_CYCLES:
	{
		param = _imgAcqPty.preImagingCalibrationCycles;
	}
	break;
	case ICamera::PARAM_LSM_IMAGING_RAMP_EXTENSION_CYCLES:
	{
		param = _imgAcqPty.imagingRampExtensionCycles;
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_ON_FLYBACK:
	{
		param = _imgAcqPty.imageOnFlyback;
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_ENABLE:
	{
		param = _imgAcqPty.enableImageDistortionCorrection;
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_GALVO_TILT_ANGLE:
	{
		param = _imgAcqPty.ImageDistortionCorrectionCalibrationGalvoTiltAngle;
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_X_ANGLE_MAX:
	{
		param = _imgAcqPty.ImageDistortionCorrectionCalibrationXAngleMax;
	}
	break;
	case ICamera::PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_Y_ANGLE_MAX:
	{
		param = _imgAcqPty.ImageDistortionCorrectionCalibrationYAngleMax;
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

long CThordaqResonantGalvo::SetParamString(const long paramID, wchar_t* str)
{
	long ret = FALSE;
	switch (paramID)

	{
	case ICamera::PARAM_MESO_EXP_PATH:
		_mROIExpLoader.get()->ExpPath = wstring(str);
		ret = _mROIExpLoader.get()->LoadExperimentXML();
		if (FALSE == ret)
		{
			//try one more time to reload experiment
			Sleep(20);
			ret = _mROIExpLoader.get()->LoadExperimentXML();
		}
		if (TRUE == ret)
		{
			for (int i = 0; i < static_cast<int>(_mROIExpLoader.get()->Scans.size()); i++)
			{
				SetParamBuffer(ICamera::PARAM_MESO_SCAN_INFO, (char*)_mROIExpLoader.get()->Scans[i], sizeof(_mROIExpLoader.get()->Scans[i]));
			}
			vector<StripInfo*> mROIStripes = vector<StripInfo*>();
			mROIStripesManager* waveformManager = mROIStripesManager::GetInstance();
			waveformManager->GenerateStripList(_mROIScan, mROIStripes);
			int overallLines = 0;
			int vSize = 0;
			int hSize = 0;
			int imageXPixel = 0;
			int imageYPixel = 0;
			int flybackCycle = 0;

			CalculatemROITotalImagingLinesAndOtherProperties(&_imgAcqPty, mROIStripes, overallLines, vSize, hSize, imageXPixel, imageYPixel, flybackCycle);
			_totalLinesFormROI = vSize;
		}
		else
		{
			SetParamBuffer(ICamera::PARAM_MESO_SCAN_INFO, NULL, 0);
		}
		break;
	default:
		break;
	}

	return ret;
}
long CThordaqResonantGalvo::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	switch (paramID)
	{
		case ICamera::PARAM_DETECTOR_NAME:
		{

			const wchar_t* pDetectorName = 1 == _rGGMode ? L"ThorDAQ RGG" : L"ThorDAQResonantGalvo";
			wcscpy_s(str, 21, pDetectorName);
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
			wstring fw = to_wstring(_lowFreqTrigBoardInfo.FWVerMajor) + L"." + to_wstring(_lowFreqTrigBoardInfo.FWVerMinor) + L"." + to_wstring(_lowFreqTrigBoardInfo.FWVerSubMinor);
			wcscpy_s(str, size, fw.c_str());
			ret = TRUE;
		}
		break;
		case ICamera::PARAM_LSM_LOW_FREQ_TRIG_BOARD_CPLD_VER:
		{
			wstring cpld = to_wstring(_lowFreqTrigBoardInfo.CPLDUsercodeMajor) + L"." + to_wstring(_lowFreqTrigBoardInfo.CPLDUsercodeSubMajor) + L"." + to_wstring(_lowFreqTrigBoardInfo.CPLDUsercodeMinor) + L"." + to_wstring(_lowFreqTrigBoardInfo.CPLDUsercodeSubMinor);
			wcscpy_s(str, size, cpld.c_str());
			ret = TRUE;
		}
		break;
		default:
			break;
	}

	return ret;
}

long CThordaqResonantGalvo::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
		case ICamera::PARAM_MESO_SCAN_INFO:
		{
			if (_mROIScan != NULL)
			{
				SAFE_DELETE_PTR(_mROIScan);
			}
			if ((NULL != pBuffer) && 0 < size)
			{
				_mROIScan = new Scan(*static_cast<Scan*>((void*)pBuffer));
				ret = TRUE;
			}
			break;
		}
		default:
		{
			ret = FALSE;
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter (%d) not implemented", paramID);
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	return ret;
}

long CThordaqResonantGalvo::GetParamBuffer(const long paramID, char* pBuffer, long size)
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

		if (POCKELS_VOLTAGE_STEPS * sizeof(double) <= size)
		{
			memcpy(pBuffer, &_pockelsReadArray[index], POCKELS_VOLTAGE_STEPS * sizeof(double));
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