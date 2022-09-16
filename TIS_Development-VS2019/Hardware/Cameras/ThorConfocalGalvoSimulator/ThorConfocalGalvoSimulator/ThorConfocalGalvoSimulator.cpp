// ThorConfocalGalvoSimulator.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <functional>

using namespace std;

#include "ThorConfocalGalvoSimulator.h"
#include "ThorConfocalGalvoSimulatorXML.h"
#include "stb_font_consolas_40_usascii.h"

#define INFINITE_COUNT 0x7FFFFFFF
#define TIMEOUT_MS 1500

#define FOUR_CHANNEL_SIMULATION

//auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
wchar_t message[256];

static stb_fontchar  fontdata   [STB_SOMEFONT_NUM_CHARS];
static unsigned char fontpixels [STB_SOMEFONT_BITMAP_HEIGHT][STB_SOMEFONT_BITMAP_WIDTH];

ThorLSMCam::ThorLSMCam():
	MIN_PIXEL_X(64),
	MAX_PIXEL_X(4096),
	MIN_PIXEL_Y(1),
	MAX_PIXEL_Y(4096),
	DEFAULT_PIXEL_X(1024),
	DEFAULT_PIXEL_Y(1024),
	MIN_FIELD_SIZE_X(5),
	MAX_FIELD_SIZE_X(255),
	DEFAULT_FIELD_SIZE_X(120),
	MIN_ALIGNMENT(-128),
	MAX_ALIGNMENT(128),
	DEFAULT_ALIGNMENT(0),
	MIN_Y_AMPLITUDE_SCALER(0),
	MAX_Y_AMPLITUDE_SCALER(1000),
	DEFAULT_Y_AMPLITUDE_SCALER(100),
	MIN_INPUTRANGE(INPUT_RANGE_20_MV),
	MAX_INPUTRANGE(INPUT_RANGE_10_V),
	DEFAULT_INPUTRANGE(INPUT_RANGE_500_MV),
	MIN_EXTCLOCKRATE(10000000), //10MSPS
	MAX_EXTCLOCKRATE(125000000), //125MSPS
	DEFAULT_EXTCLOCKRATE(100000000), //100MSPS
	MIN_INTERNALCLOCKRATE(SAMPLE_CLOCK_20MSPS),
	MAX_INTERNALCLOCKRATE(SAMPLE_CLOCK_125MSPS),
	DEFAULT_INTERNALCLOCKRATE(SAMPLE_CLOCK_125MSPS),
	MIN_AVERAGENUM(2),
	MAX_AVERAGENUM(1024),
	DEFAULT_AVERAGENUM(8),
	DEFAULT_FIELD_SCALE_XYRATIO(25.74),
	MIN_SCANMODE(0),
	MAX_SCANMODE(4),
	DEFAULT_SCANMODE(0),
	MIN_TRIGGER_MODE(0),
	MAX_TRIGGER_MODE(5),
	DEFAULT_TRIGGER_MODE(2), //SW_FREE_RUN_MODE
	MIN_GALVO_VOLTAGE(-10.0),
	MAX_GALVO_VOLTAGE(10.0),
	MIN_CHANNEL(1),
	NUM_TWOWAY_ZONES(251),
#ifdef FOUR_CHANNEL_SIMULATION
	MAX_CHANNEL(15)
#else
	MAX_CHANNEL(3)
#endif
{

	_pixelX = DEFAULT_PIXEL_X;
	_pixelY = DEFAULT_PIXEL_Y;
	_fieldSize = DEFAULT_FIELD_SIZE_X;
	_offsetX = 0;
	_offsetY = 0;
	_channel = 0x0001;
	_alignmentForField = DEFAULT_ALIGNMENT;
	_yAmplitudeScaler = DEFAULT_Y_AMPLITUDE_SCALER;
	_inputRangeChannel1 = DEFAULT_INPUTRANGE;
	_inputRangeChannel2 = DEFAULT_INPUTRANGE;
	_inputRangeChannel3 = DEFAULT_INPUTRANGE;
	_inputRangeChannel4 = DEFAULT_INPUTRANGE;
	//  _clockSource = INTERNAL_CLOCK;
	_clockRateInternal = DEFAULT_INTERNALCLOCKRATE;
	_clockRateExternal = DEFAULT_EXTCLOCKRATE;
	_scanMode = DEFAULT_SCANMODE;
	_averageMode = 0;
	_averageNum = 2;
	_triggerMode = DEFAULT_TRIGGER_MODE;
	_frameCount = 1;
	_frameIndex = 1;
	_pBitmapBuffer = NULL;
	_areaMode = SQUARE;

	_realTimeDataAverage = 0;
	_galvoEnable = 0;
	_clockSource = 1;
	_clockRateExternal = 80000000;

	_pockelsPower0 = 0;
	_pockelsPower1 = 0;
	_pockelsPower2 = 0;
	_pockelsPower3 = 0;

	_longLineScanTime = 0;
	_longLineScan = FALSE;

	_numberOfPlanes = 1;
	_selectedPlane = 0;

	_threePhotonEnabled = FALSE;

	_dwellTime = DEFAULT_DWELL_TIME;

	_indexOfLastFrame = -1;

	_pMemoryBuffer = NULL;

	_pDetectorName = L"GalvoGalvoSim";

	_rawSaveEnabledChannelsOnly = FALSE;

	_channelPolarity[0] = ICamera::Polarity::POL_NEG;
	_channelPolarity[1] = ICamera::Polarity::POL_NEG;
	_channelPolarity[2] = ICamera::Polarity::POL_NEG;
	_channelPolarity[3] = ICamera::Polarity::POL_NEG;

	/*   galvo_waveform = NULL;
	frameTrigger = NULL;
	hThread = NULL;*/
	for(long i=0; i<NUM_TWOWAY_ZONES; i++)
	{
		_twoWayZones[i] = 0;
		_twoWayZonesFine[i] = 0;
	}

	_imageUpdateIntervalMS = 40;
	_lastImageUpdateTime = 0;
}

///Initialize Static Members
bool ThorLSMCam::_instanceFlag = false;
auto_ptr<ThorLSMCam> ThorLSMCam::_single(new ThorLSMCam());
long num_alazar_bd = 0;


ThorLSMCam* ThorLSMCam::getInstance()
{
	if (! _instanceFlag)
	{
		_single.reset(new ThorLSMCam());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

ThorLSMCam::~ThorLSMCam()
{
	_instanceFlag = false;
}

long ThorLSMCam::FindCameras(long &cameraCount)
{

	_tileRow = _tileCol = _zSteps = 1;

	num_alazar_sys = 1;

	num_NI_DAQ = 1;

	if (num_alazar_sys > 0 && num_NI_DAQ > 0)
	{
		cameraCount = 1;
	}
	else
		cameraCount = 0;	

	return num_alazar_sys;
}

long ThorLSMCam::SelectCamera(const long camera)
{
	if (camera != 0)
	{
		return FALSE;
	}

	string simPath;
	try
	{
		auto_ptr<ThorConfocalGalvoSimulatorXML> pSetup(new ThorConfocalGalvoSimulatorXML());

		pSetup->GetConnection(simPath, _imageUpdateIntervalMS);
		if(simPath.compare("") != 0)
		{
			pSetup->GetTilesDimension(simPath, _tileRow, _tileCol, _zSteps);
		}
	}
	catch(...)
	{
	}

	_simulatorDataPath.clear();

	wstring ws(simPath.begin(),simPath.end());
	wstring wsFind;
	wstring wsResult;

	for(long i=0; i<4; i++)
	{
		switch(i)
		{
		case 0:{wsFind = ws + L"\\ChanA*.tif";}break;
		case 1:{wsFind = ws + L"\\ChanB*.tif";}break;
		case 2:{wsFind = ws + L"\\ChanC*.tif";}break;
		case 3:{wsFind = ws + L"\\ChanD*.tif";}break;
		}

		_simulatorData[i].clear();
		_frameIndex = 1;

		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile(wsFind.c_str(),&FindFileData);

		if(hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				wsResult = ws + L"\\" + FindFileData.cFileName;
				_simulatorData[i].push_back(wsResult);
			}
			while(TRUE == FindNextFile(hFind,&FindFileData));


			long c=1;
			ReadImageInfo((wchar_t*)wsResult.c_str(),_fileWidth,_fileHeight,c);

			if(_pMemoryBuffer)
			{
				delete _pMemoryBuffer;
				_pMemoryBuffer = NULL;
			}

			const long	BYTES_PER_PIXEL = 2;
			_pMemoryBuffer = new char[_fileWidth * _fileHeight * BYTES_PER_PIXEL];

			FindClose(hFind);
		}
	}

	return TRUE;
}

long ThorLSMCam::TeardownCamera()
{
	if(_pBitmapBuffer != NULL)
	{
		delete _pBitmapBuffer;
		_pBitmapBuffer = NULL;
	}

	if(_pMemoryBuffer != NULL)
	{
		delete _pMemoryBuffer;
		_pMemoryBuffer = NULL;
	}
	return 1;
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
			paramMin = ICamera::SW_SINGLE_FRAME;
			paramMax = ICamera::HW_MULTI_FRAME_TRIGGER_EACH;
			paramDefault = ICamera::LSM;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 2;
			paramMax = INT_MAX;
			paramDefault = 100;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 1;
			paramMax = 1000;
			paramDefault = 100;
			paramReadOnly = TRUE;
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
			//paramMin = -(MAX_FIELD_SIZE_X - MIN_FIELD_SIZE_X) / 2;
			paramMin = 2 * MIN_GALVO_VOLTAGE / FIELD2THETA + _fieldSize * _pixelY / _pixelX /2;
			//paramMax = (MAX_FIELD_SIZE_X - MIN_FIELD_SIZE_X) / 2;
			paramMax = 2 * MAX_GALVO_VOLTAGE / FIELD2THETA - _fieldSize * _pixelY / _pixelX /2;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_CHANNEL:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_CHANNEL; //at least one channel
			paramMax = MAX_CHANNEL;
			//  paramMax = (1 << (num_alazar_bd * 2)) - 1;  //bitwise selection
			paramDefault = MIN_CHANNEL;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_ALIGNMENT:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = -128;
			paramMax = + 128;
			paramDefault = 0;
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
	case ICamera::PARAM_LSM_GALVO_ENABLE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
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
			paramMax = 2;
			paramDefault = 1;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_INTERNALCLOCKRATE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			/*paramMin = SAMPLE_RATE_1MSPS;
			paramMax = SAMPLE_RATE_125MSPS;
			paramDefault = SAMPLE_RATE_125MSPS;*/
			paramMin = MIN_EXTCLOCKRATE;
			paramMax = MAX_EXTCLOCKRATE;
			paramDefault = DEFAULT_EXTCLOCKRATE;
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
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_SCANMODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 4;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_AVERAGEMODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_AVERAGENUM:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 2;
			paramMax = 1024;
			paramDefault = 8;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_AREAMODE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = SQUARE;
			paramMax = LINE;
			paramDefault = SQUARE;
			paramReadOnly = FALSE;

		}
		break;
	case ICamera::PARAM_LSM_1X_FIELD_SIZE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_FIELD_SIZE_X;
			paramMax = MAX_FIELD_SIZE_X;
			paramDefault = 90.0;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_X:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 1;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 1;
			paramReadOnly = TRUE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 1;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;

	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 1;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 1;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;

	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2;
			paramDefault = 1;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;
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
			paramMin = 0;
			paramMax = 255;
			paramDefault = 128;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_FRAME_RATE:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0.000001;
			paramMax = 100000;
			paramDefault = 30.333333333333;
			paramReadOnly = FALSE;

		}
		break;
	case ICamera::PARAM_LSM_DWELL_TIME:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = MIN_DWELL_TIME;
			paramMax = MAX_DWELL_TIME;
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
	case ICamera::PARAM_LSM_SIM_INDEX:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = MAXINT32;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS:
		{
			paramType = ICamera::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 60000;
			paramDefault = 0;
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
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_1:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = ICamera::Polarity::POL_NEG;
		paramMax = ICamera::Polarity::POL_BI;
		paramDefault = ICamera::Polarity::POL_NEG;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_2:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = ICamera::Polarity::POL_NEG;
		paramMax = ICamera::Polarity::POL_BI;
		paramDefault = ICamera::Polarity::POL_NEG;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_3:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = ICamera::Polarity::POL_NEG;
		paramMax = ICamera::Polarity::POL_BI;
		paramDefault = ICamera::Polarity::POL_NEG;
		paramReadOnly = FALSE;
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_4:
	{
		paramType = ICamera::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = ICamera::Polarity::POL_NEG;
		paramMax = ICamera::Polarity::POL_BI;
		paramDefault = ICamera::Polarity::POL_NEG;
		paramReadOnly = FALSE;
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

long ThorLSMCam::SetParam(const long paramID, const double param)
{
	long ret = FALSE;
	switch (paramID)
	{
	case ICamera::PARAM_LSM_SIM_INDEX:
		_frameIndex = (long)param;
		break;
	case ICamera::PARAM_LSM_SCANMODE:
		{
			if ((param >= MIN_SCANMODE) && (param <= MAX_SCANMODE))
			{
				_scanMode = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_X:
		{
			int factor;
			if(_scanMode == TWO_WAY_SCAN)
			{
				factor = 2;
			}
			else
			{
				factor = 1;
			}

			if ((param >= MIN_PIXEL_X) && (param <= MAX_PIXEL_X / factor))
				if (static_cast<long>(param) % 16 == 0)
				{
					_pixelX = static_cast<long>(param);

					switch(_areaMode)
					{
					case SQUARE:
						{
							_pixelY = static_cast<long>(param);					
						}
						break;
					case LINE:
						{
							_pixelY = 1;
						}
						break;
					}
					ret = TRUE;
				}
		}
		break;

	case ICamera::PARAM_LSM_PIXEL_Y:
		{
			if ((param >= MIN_PIXEL_Y) && (param <= MAX_PIXEL_Y))

				if(_areaMode == LINE)
				{
					_pixelY = 1;
					ret = TRUE;
				}
				else
				{
					if (static_cast<long>(param) % 16 == 0)
					{
						switch(_areaMode)
						{
						case SQUARE:
							{
								_pixelY = _pixelX;
							}
							break;
						default:
							{
								_pixelY  = static_cast<long>(param);
							}
						}
						ret = TRUE;
					}
				}
		}
		break;

	case ICamera::PARAM_LSM_FIELD_SIZE:
		{
			if ((param >= MIN_FIELD_SIZE_X) && (param <= MAX_FIELD_SIZE_X))
			{
				double fieldY_volt;
				double theta = param * FIELD2THETA;
				fieldY_volt = theta * (double) _pixelY / (double)_pixelX / 2; // divide by 2 because the mechanical angle is half of the optical angle
				//if (fieldY_volt <= 20.0)
				{
					_fieldSize = static_cast<long>(param);
					ret = TRUE;
				}
			}
			else
			{
			}
		}
		break;
	case ICamera::PARAM_LSM_ALIGNMENT:
		{
			if ((param >= MIN_ALIGNMENT) && (param <= MAX_ALIGNMENT))
			{
				_alignmentForField = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_Y_AMPLITUDE_SCALER:
		{
			if ((param >= MIN_Y_AMPLITUDE_SCALER) && (param <= MAX_Y_AMPLITUDE_SCALER))
			{
				_yAmplitudeScaler = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;

	case ICamera::PARAM_LSM_GALVO_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_galvoEnable = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE1:
		{
			if ((param >= MIN_INPUTRANGE) && (param <= MAX_INPUTRANGE))
			{
				_inputRangeChannel1 = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE2:
		{
			if ((param >= MIN_INPUTRANGE) && (param <= MAX_INPUTRANGE))
			{
				_inputRangeChannel2 = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE3:
		{
			if ((param >= MIN_INPUTRANGE) && (param <= MAX_INPUTRANGE))
			{
				_inputRangeChannel3 = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE4:
		{
			if ((param >= MIN_INPUTRANGE) && (param <= MAX_INPUTRANGE))
			{
				_inputRangeChannel4 = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_INTERNALCLOCKRATE:
		{
			if ((param >= MIN_INTERNALCLOCKRATE) && (param <= MAX_INTERNALCLOCKRATE))
			{
				_clockRateInternal = static_cast<int> (param);
				ret = TRUE;
			}
		}
		break;

	case ICamera::PARAM_LSM_CHANNEL:
		{
			if ((param >= MIN_CHANNEL) && (param <= MAX_CHANNEL))
			{
				_channel = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_TRIGGER_MODE:
		{
			if ((param >= MIN_TRIGGER_MODE) && (param <= MAX_TRIGGER_MODE))
			{
				_triggerMode = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			if (param >= 1)
			{
				_frameCount = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_AVERAGEMODE:
		{
			if((param >=ICamera::AVG_MODE_NONE) && (param <= ICamera::AVG_MODE_CUMULATIVE))
			{
				_averageMode = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_AVERAGENUM:
		{
			if((param >=2) && (param <= 1024))
			{
				_averageNum = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_CLOCKSOURCE:
		{
			if((param >= 1) && (param <=2))
			{
				_clockSource = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_EXTERNALCLOCKRATE:
		{
			if((param >= MIN_EXTCLOCKRATE) && (param <= MAX_EXTCLOCKRATE))
			{
				_clockRateExternal = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_DWELL_TIME:
	{
		if ((param >= MIN_DWELL_TIME) && (param <= MAX_DWELL_TIME))
		{
			_dwellTime = param;
			ret = TRUE;
		}
	}
	break;

	case ICamera::PARAM_LSM_3P_ENABLE:
	{
		if ((param >= FALSE) && (param <= TRUE))
		{
			_threePhotonEnabled = static_cast<long> (param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_NUMBER_OF_PLANES:
	{
		if (MIN_NUMBER_OF_PLANES <= param && MAX_NUMBER_OF_PLANES >= param)
		{
			_numberOfPlanes = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_OFFSET_X:
		{
			if ((param >= -(MAX_FIELD_SIZE_X - _fieldSize) / 2) && (param <= (MAX_FIELD_SIZE_X - _fieldSize) / 2))
			{
				const long MAXIMUM_PIXEL_SAMPLES = 7812;

				double samplingOffsetLimit = abs(_fieldSize*((MAXIMUM_PIXEL_SAMPLES/(PI*_pixelX))-.5));
				if(abs(param) < samplingOffsetLimit)
				{
					_offsetX = static_cast<long>(param);
					ret = TRUE;
				}
				else
				{
				}
			}
			else
			{
			}
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_Y:
		{
			//if ((param >= -(MAX_FIELD_SIZE_X - _fieldSize) / 2) && (param <= (MAX_FIELD_SIZE_X - _fieldSize) / 2))
			if ((param >= (2 * MIN_GALVO_VOLTAGE / FIELD2THETA + _fieldSize * _pixelY / _pixelX /2)) && (param <= (2 * MAX_GALVO_VOLTAGE / FIELD2THETA - _fieldSize * _pixelY / _pixelX /2)))
			{
				_offsetY = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
			}
		}
		break;

	case ICamera::PARAM_LSM_AREAMODE:
		{
			if((param >= SQUARE) && (param <= LINE))
			{	
				_areaMode = (AreaMode)static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_realTimeDataAverage = static_cast<long> (param);
				ret = TRUE;
			}
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:
		{
			_pockelsPower0 = param;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:
		{
			_pockelsPower1 = param;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:
		{
			_pockelsPower2 = param;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:
		{
			_pockelsPower3 = param;
		}
		break;
	case ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS:
		{
			_longLineScanTime = param;
		}
		break;
	case ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN:
		{
			_longLineScan = static_cast<long>(param);
		}
		break;
	case ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY:
		{
			_rawSaveEnabledChannelsOnly = static_cast<long>(param);
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
				_twoWayZones[paramID - ICamera::PARAM_LSM_TWO_WAY_ZONE_1] = static_cast<long>(param);
				ret = TRUE;
			}	
			else
			{
				//StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_TWO_WAY_ZONE %d outside range %d to %d",static_cast<long> (param), 0,255);
				//LogMessage(_errMsg,ERROR_EVENT);
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
			if((param >= 0) && (param < 255))
			{
				_twoWayZonesFine[paramID - ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_1] = static_cast<long>(param);
				ret = TRUE;
			}
			else
			{
				//StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_TWO_WAY_ZONE_FINE %d outside range %d to %d",static_cast<long> (param), 0,255);
				//LogMessage(_errMsg,ERROR_EVENT);
			}
		}
		break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_1:
	{
		if ((param >= ICamera::Polarity::POL_NEG) && (param <= ICamera::Polarity::POL_BI))
		{
			_channelPolarity[0] = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_2:
	{
		if ((param >= ICamera::Polarity::POL_NEG) && (param <= ICamera::Polarity::POL_BI))
		{
			_channelPolarity[1] = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_3:
	{
		if ((param >= ICamera::Polarity::POL_NEG) && (param <= ICamera::Polarity::POL_BI))
		{
			_channelPolarity[2] = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
	case ICamera::PARAM_LSM_CHANNEL_POLARITY_4:
	{
		if ((param >= ICamera::Polarity::POL_NEG) && (param <= ICamera::Polarity::POL_BI))
		{
			_channelPolarity[3] = static_cast<long>(param);
			ret = TRUE;
		}
	}
	break;
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


	return ret;
}

long ThorLSMCam::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch (paramID)
	{
	case ICamera::PARAM_TRIGGER_MODE:
		{
			param = _triggerMode;
		}
		break;
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
	case ICamera::PARAM_MULTI_FRAME_COUNT:
		{
			param = _frameCount;
		}
		break;
	case ICamera::PARAM_LSM_SCANMODE:
		{
			param = _scanMode;
		}
		break;
	case ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION:
		{		
			param = 100.0;
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

	case ICamera::PARAM_LSM_OFFSET_X:
		{
			param = _offsetX;
		}
		break;
	case ICamera::PARAM_LSM_OFFSET_Y:
		{
			param = _offsetY;
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
	case ICamera::PARAM_LSM_CLOCKSOURCE:
		{
			param = _clockSource;
		}
		break;
	case ICamera::PARAM_LSM_EXTERNALCLOCKRATE:
		{
			param = _clockRateExternal;
		}
		break;
	case ICamera::PARAM_LSM_AREAMODE:
		{
			param = _areaMode;
		}
		break;
	case ICamera::PARAM_LSM_GALVO_ENABLE:
		{
			param = _galvoEnable;			
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE1:
		{
			param = _inputRangeChannel1;
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE2:
		{
			param = _inputRangeChannel2;
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE3:
		{
			param = _inputRangeChannel3;
		}
		break;
	case ICamera::PARAM_LSM_INPUTRANGE4:
		{
			param = _inputRangeChannel4;
		}
		break;
	case ICamera::PARAM_LSM_1X_FIELD_SIZE:
		{
			param = 90;
		}
		break;
	case ICamera::PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE:
		{
			param = _realTimeDataAverage;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_0:
		{
			param = 1;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_1:
		{
			param = 1;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_2:
		{
			param = 1;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_CONNECTED_3:
		{
			param = 1;
		}
		break;

	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0:
		{
			param = 0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0:
		{
			param = 1.0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0:
		{
			param = _pockelsPower0;
		}
		break;

	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_1:
		{
			param = 0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_1:
		{
			param = 1.0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1:
		{
			param = _pockelsPower1;
		}
		break;

	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_2:
		{
			param = 0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_2:
		{
			param = 1.0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2:
		{
			param = _pockelsPower2;
		}
		break;

	case ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_3:
		{
			param = 0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_3:
		{
			param = 1.0;
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3:
		{
			param = _pockelsPower3;
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_X:
		{
			param = 1;
		}
		break;
	case ICamera::PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y:
		{
			param = 1;
		}
		break;
	case ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS:
		{
			param = _longLineScanTime;
		}
		break;
	case ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN:
		{
			param = _longLineScan;
		}
		break;
	case ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY:
		{
			param = _rawSaveEnabledChannelsOnly;
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
			param = _twoWayZones[paramID - ICamera::PARAM_LSM_TWO_WAY_ZONE_1];
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
			param = _twoWayZonesFine[paramID - ICamera::PARAM_LSM_TWO_WAY_ZONE_FINE_1];
		}
		break;
	case ICamera::PARAM_FRAME_RATE:
		{
			param = 30.33333333333333;
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
	case ICamera::PARAM_LSM_3P_ENABLE:
	{
		param = _threePhotonEnabled;
	}
	break;
	case ICamera::PARAM_LSM_NUMBER_OF_PLANES:
	{
		param = _numberOfPlanes;
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
	default:
		{
			ret = FALSE;
		}
	}

	return ret;
}

long ThorLSMCam::GetParamString(const long paramID, wchar_t * str, long size)
{
	long ret = FALSE;

	switch(paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME:
		{			
			wcscpy_s(str,20, _pDetectorName);	//tempID[20] in SelectHardware
			ret = TRUE;
		}
		break;
	default:
		break;
	}

	return ret;
}


long ThorLSMCam::PreflightAcquisition(char * pDataBuffer)
{
	int ret = true;

	LARGE_INTEGER freqInt;
	QueryPerformanceFrequency(&freqInt);
	_dfrq=(double) freqInt.QuadPart;

	QueryPerformanceCounter(&_largeint);
	_qPart1=_largeint.QuadPart;

	_indexOfLastFrame = -1;

	return ret;
}

long ThorLSMCam::SetupAcquisition(char * pDataBuffer)
{
	int ret = TRUE;

	return ret;
}

long ThorLSMCam::StartAcquisition(char *pDataBuffer)
{	
	if((_pBitmapBuffer == NULL)||(_pixelX_C != _pixelX)||(_pixelY_C != _pixelY))
	{
		_pixelX_C = _pixelX;
		_pixelY_C = _pixelY;

		if(_pBitmapBuffer != NULL)
		{
			delete _pBitmapBuffer;
		}

		_pBitmapBuffer = new BYTE[_pixelX_C*_pixelY_C];
	}

	if(_channel_C != _channel)
	{
		_channel_C = _channel;
	}

	STB_SOMEFONT_CREATE(fontdata, fontpixels, STB_SOMEFONT_BITMAP_HEIGHT);

	_indexOfLastFrame++;
	_lastImageUpdateTime = GetTickCount64();
	return 1;
}

long ThorLSMCam::StatusAcquisition(long &status)
{
	long ret = TRUE;
	if ((GetTickCount64() - _lastImageUpdateTime) > static_cast<UINT64>(_imageUpdateIntervalMS))
	{
		status = ICamera::STATUS_READY;
		_lastImageUpdateTime = GetTickCount64();
	}
	else
	{
		status = ICamera::STATUS_BUSY;
	}
	return ret;
}

long ThorLSMCam::StatusAcquisitionEx(long &status, long &indexOfLastFrame)
{
	long ret = TRUE;
	if ((GetTickCount64() - _lastImageUpdateTime) > static_cast<UINT64>(_imageUpdateIntervalMS))
	{
		status = ICamera::STATUS_READY;
		indexOfLastFrame = _indexOfLastFrame;
		_lastImageUpdateTime = GetTickCount64();
	}
	else
	{
		status = ICamera::STATUS_BUSY;
	}	
	return ret;
}

#define BITMAP_OVERLAY

/// draw the specified string as text into buffer pointed by pDataBuffer
/// c - specified channels, 0, 1, 2, 3 correspond to R, G, B, A, respectively
void ThorLSMCam::PrintText(char *pDataBuffer, int x, int y, wchar_t *str, int c) 
{
	int           char_value = 0;
	stb_fontchar *cd         = 0;
	int           index      = 0;
	int           w          = 0;
	int           h          = 0;
	int           i          = 0;
	int           j          = 0;
	int           x_advance  = 0;
	int           value      = 0;

	int imageWidth = (int)_pixelX_C;
	int imageHeight = (int) _pixelY_C;


	if(pDataBuffer == 0) return;
	if(str   == 0) return;

	unsigned short * pImage = (unsigned short *) pDataBuffer;


	while(*str != 0)
	{
		char_value = *str++;
		cd = &fontdata[char_value - STB_SOMEFONT_FIRST_CHAR];

		cd->x0 = static_cast<short>(cd->s0 * (float)STB_SOMEFONT_BITMAP_WIDTH);
		cd->y0 = static_cast<short>(cd->t0 * (float)STB_SOMEFONT_BITMAP_HEIGHT);
		cd->x1 = static_cast<short>(cd->s1 * (float)STB_SOMEFONT_BITMAP_WIDTH);
		cd->y1 = static_cast<short>(cd->t1 * (float)STB_SOMEFONT_BITMAP_HEIGHT);

		w = (cd->x1 - cd->x0);
		h = (cd->y1 - cd->y0);

		// do not draw the part of text longer than image
		if((x + x_advance >= imageWidth) || (y + h >= imageHeight))
			return;

		index = c * imageWidth * imageHeight + (y * (int)_pixelX_C) + x + x_advance;

		for(i=0;i<h;i++)
		{
			for(j=0;j<w;j++)
			{
				// assign image data value from font pixels
				value = fontpixels[i+cd->y0][j+cd->x0];

				if(value == 0) continue;
				//value = value | (value << 8);

				pImage[(index + j)] = 16383; //(value & 0x0FFF);
			}

			index += (int)_pixelX_C;
		}

		x_advance += cd->advance_int;
	}

	return;
}

unsigned short ThorLSMCam::GetGradientValue(int x, int y, int channelNum)
{
	switch(channelNum)
	{
	case 0:
		{
			int intensity = ((y<<2)%16384);
			return (unsigned short)min(16383,max(0,intensity-1));
		}
		break;
	case 1:
		{
			return min(16383,max(0,(x<<2)%16384));	
		}
	case 2:
		{
			int intensity2 = ((y<<4)%16384);
			return (unsigned short)min(16383,max(0,(intensity2-1)));
		}
		break;
	case 3:
		{
			return min(16383,max(0,(x<<4)%16384));
		}
		break;
	default:
		{
			int intensity = ((y<<2)%16384);
			return (unsigned short)min(16383,max(0,intensity-1));
		}
		break;
	}

}

void ThorLSMCam::FillImageWithGradient(GenericImage<unsigned short>& image)
{

	for(int chan=0; chan<image.getNumChannels(); chan++)
	{
		if(image.isChannelEnabled(chan))
		{
			auto chanIt = image.channelBegin(chan,0,0);
			for (long y = 0; y<image.getHeight(); y++)
			{
				for(long x=0; x<image.getWidth(); x++)
				{
					*chanIt = GetGradientValue(x,y,chan);
					++chanIt;
				}
			}
		}
	}
}

void ThorLSMCam::FillImageWithData(GenericImage<unsigned short>& image, int chanIndex)
{
	int chan = 0;
	for(int i=0; i < image.getNumChannels(); i++)
	{
		// Check if it is a single channel to set the index to the channel's file 
		// MAX_CHANNEL != _channel_C means it's a single channel experiment
		if ( 0 == _channel_C || 2 == _channel_C || 4 == _channel_C || 8 == _channel_C)
		{
			chan = chanIndex;
		}
		else 
		{
			chan = i;
		}	
		long size;

		size = static_cast<long>(_simulatorData[chan].size());

		if(_simulatorData[chan].size() <= 0)
		{
			continue;
		}

		if(_frameIndex > size) _frameIndex = 1;

		long imageIndex = (_frameIndex - 1) % size;

		long tt = imageIndex / (_tileCol * _zSteps);
		long dd = (imageIndex % (_tileCol * _zSteps));
		vector<wstring>::iterator it = _simulatorData[chan].begin();
		if(tt % 2 == 0)
		{
			it += imageIndex;
		}
		else
		{
			it = it + (tt + 1 ) * (_tileCol * _zSteps) -  (dd / _zSteps + 1) * _zSteps + dd % _zSteps;
		}

		ReadImage((char*)(*it).c_str(),(char*&)_pMemoryBuffer);

		unsigned short * pData = (unsigned short*)_pMemoryBuffer;

		// The variable i will only be 0 for a single channel, or more for multichannels 
		auto chanIt = image.channelBegin(i,0,0);

		for (long y = 0; y<image.getHeight(); y++)
		{
			for(long x=0; x<image.getWidth(); x++)
			{
				if((x < _fileWidth)&&(y <_fileHeight))
				{
					*chanIt = *(pData + x + y * _fileWidth);
				}
				else
				{
					*chanIt = 0;
				}
				++chanIt;
			}
		}
	}

}

void FillGradientBackground(char *pDataBuffer, int imageWidth, int imageHeight, int totChannels, int channelIndex)
{// channelIndex: which channel icon is selected: A-0; B-1; C-2; D-3; all-4.
	unsigned short *pD = (unsigned short*)pDataBuffer;
	unsigned short *pG = pD + imageWidth * imageHeight;
	unsigned short *pB = pD + 2 * imageWidth * imageHeight;
	unsigned short *p4 = pD + 3 * imageWidth * imageHeight;	
	unsigned short intensity;
	unsigned short intensity2;

	for (long y = 0; y < imageHeight; y++)
	{
		for(long x=0; x<imageWidth; x++)
		{
			if(totChannels == 4)	// all channel display	
			{
				intensity = ((y<<2)%16384);
				*pD++ = (unsigned short)min(16383,max(0,intensity-1));

				*pG++ = min(16383,max(0,(x<<2)%16384));	

				intensity2 = ((y<<4)%16384);
				*pB++ = (unsigned short)min(16383,max(0,(intensity2-1)));

				*p4++ = min(16383,max(0,(x<<4)%16384));
			}
			else  // single channel display	
			{						
				switch (channelIndex)
				{
				case 0:
					{
						intensity = ((y<<2)%16384);
						*pD++ = (unsigned short)min(16383,max(0,intensity-1));
					}
					break;
				case 1:
					{
						*pD++ = min(16383,max(0,(x<<2)%16384));	
					}
					break;
				case 2:
					{
						intensity2 = ((y<<4)%16384);
						*pD++ = (unsigned short)min(16383,max(0,(intensity2-1)));
					}
					break;
				case 3:
					{
						*pD++ = min(16383,max(0,(x<<4)%16384));
					}
					break;
				default:
					break;
				}
			}		
		}
	}
}

/// function to get the number of 1's in binary representaion of the passed parameter
int CountSetBits(long bitset)
{
	int count = 0;
	while(bitset)
	{
		bitset &= (bitset - 1);
		count ++;
	}
	return count;
}

long ThorLSMCam::CopyAcquisition(char *pDataBuffer, void* frameInfo)
{
	_indexOfLastFrame++;

	//===========================
	//   Get Image Parameters
	//===========================
	int imageWidth = _pixelX_C;
	int imageHeight = _pixelY_C;
	int numAllChannels = CountSetBits(MAX_CHANNEL);
	int selectedChannels = CountSetBits(_channel_C);		
	int numChannels = (selectedChannels > 1)? numAllChannels : selectedChannels;
	int channelIndex; 	// index of selected channel (icon clicked) on 
	// channel ListView, CaptureSetup; 
	// Ex: ChanA icon clicked -> channelIndex = 0; 
	//     ChanB icon clicked -> channelIndex = 1;
	//     ......
	//     AllChan icon clicked -> channelIndex = 4;

	if (_channel_C == 15)	// all channel icon selected by user
	{
		channelIndex = 4;
	}
	else  // any single channel icon selected
	{
		channelIndex = (int)(log10(_channel_C)/log10(2));
	}

	//===========================
	//   Create Image
	//===========================
	GenericImage<unsigned short> image(imageWidth, imageHeight, 1, numChannels, 1, GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
	if(_rawSaveEnabledChannelsOnly)
	{
		image = GenericImage<unsigned short>(imageWidth, imageHeight, 1, numChannels, 1, GenericImage<unsigned short>::CONTIGUOUS_CHANNEL,ChannelManipulator<unsigned short>::getEnabledChannels(_channel_C));
	}
	image.setMemoryBuffer((unsigned short*)pDataBuffer);

	if(_simulatorData[0].size() > 0 || _simulatorData[1].size() > 0 || _simulatorData[2].size() > 0 || _simulatorData[3].size() > 0)
	{
		//=====================================
		//   Fill Image With Experienment Data
		//=====================================
		static bool doOnce = true;
		static InternallyStoredImage<unsigned short> gradientImage(imageWidth, imageHeight, 1, numChannels, 1, GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);

		gradientImage = InternallyStoredImage<unsigned short>(imageWidth, imageHeight, 1, numChannels, 1, GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
		FillImageWithData(gradientImage, channelIndex);

		image.copyFrom(gradientImage);
	}
	else
	{
		//=====================================
		//   Fill Image With Simulated Data
		//=====================================
		static bool doOnce = true;
		static InternallyStoredImage<unsigned short> gradientImage(imageWidth, imageHeight, 1, numChannels, 1, GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
		if(gradientImage.getWidth() != imageWidth || gradientImage.getHeight() != imageHeight || gradientImage.getNumChannels() != numChannels || doOnce)
		{
			gradientImage = InternallyStoredImage<unsigned short>(imageWidth, imageHeight, 1, numChannels, 1, GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			FillImageWithGradient(gradientImage);
			doOnce = false;
		}

		image.copyFrom(gradientImage);
		//========================================
		//   Get Time Since Last Frame
		//========================================
		QueryPerformanceCounter(&_largeint);
		LONGLONG QPart2=_largeint.QuadPart;
		double dcountdiv=(double) (QPart2-_qPart1);
		double dTime=dcountdiv/_dfrq;


		//========================================
		//   Add Info Message to the Image
		//========================================
		PrintMessage(image, dTime);
	}

	_frameIndex++;

	return 1;
}


void ThorLSMCam::PrintMessage(GenericImage<unsigned short>& image, double printTime)
{
	char* imageBuffer = (char*)(image.getDirectPointerToData(0,0,0,0,0));

	for(int chan=0; chan<image.getNumEnabledChannels(); ++chan)
	{
		vector<int> enabledChannel = ChannelManipulator<unsigned short>::getEnabledChannels(_channel_C);

		//=== Fill Frame Message ===
		memset(message, 0, sizeof(message));
		wsprintf(message,L"F %06d",_frameIndex);
		PrintText(imageBuffer, 10, 10, message, chan);

		//=== Fill Time Message ===
		double dVal = floor(printTime);
		wsprintf(message,L"T %06d.%06d",static_cast<long>(printTime),static_cast<long>(1000000 * (printTime - dVal)));
		PrintText(imageBuffer, 10, 50, message, chan);

		//=== Fill Channels Enabled: A,B,C,D
		if(chan < enabledChannel.size())
		{
			switch(enabledChannel[chan])
			{
			case 0: 
				{
					wsprintf(message,L"Ch A");
					if (2 == _channel_C)
					{
						wsprintf(message,L"Ch   B");
					}
					if (4 == _channel_C)
					{
						wsprintf(message,L"Ch     C");
					}
					if (8 == _channel_C)
					{
						wsprintf(message,L"Ch       D");
					}
					PrintText(imageBuffer, 10, 90, message, ((1 == image.getNumEnabledChannels() || _rawSaveEnabledChannelsOnly) ? chan : enabledChannel[chan]));
					break;
				}
			case 1: 
				{    
					wsprintf(message,L"Ch   B");
					PrintText(imageBuffer, 10, 90, message, ((1 == image.getNumEnabledChannels() || _rawSaveEnabledChannelsOnly) ? chan : enabledChannel[chan]));
					break;
				}

			case 2: 
				{    
					wsprintf(message,L"Ch     C");
					PrintText(imageBuffer, 10, 90, message, ((1 == image.getNumEnabledChannels() || _rawSaveEnabledChannelsOnly) ? chan : enabledChannel[chan]));
					break;
				}


			case 3: 
				{    
					wsprintf(message,L"Ch       D");
					PrintText(imageBuffer, 10, 90, message, ((1 == image.getNumEnabledChannels() || _rawSaveEnabledChannelsOnly) ? chan : enabledChannel[chan]));
					break;
				}
				break;

			}
		}
		//=== Fill AreaMode ===
		switch (_areaMode)
		{
		case SQUARE:
			{
				wsprintf(message,L"AM: SQR");
				PrintText(imageBuffer, 10, 130, message, chan);
				break;
			}
		case RECTANGLE:
			{
				wsprintf(message,L"AM: REC"); 
				PrintText(imageBuffer, 10, 130, message, chan);
				break;
			}
		case LINE:
			{
				wsprintf(message,L"AM: LINE");
				PrintText(imageBuffer, 10, 130, message, chan);
				break;
			}
			break;
		}

		//=== Fill FieldSizee ===
		wsprintf(message,L"FS %d",static_cast<long>(_fieldSize));
		PrintText(imageBuffer, 10, 170, message, chan);

		//=== Fill Piexel X and Y ===
		wsprintf(message,L"PX: %d PY: %d",static_cast<long>(_pixelX_C),static_cast<long>(_pixelY_C));
		PrintText(imageBuffer, 10, 210, message, chan);

		//=== Scan Mode ===
		wsprintf(message,L"SM %d way",static_cast<long>(-1*_scanMode+2));
		PrintText(imageBuffer, 10, 250, message, chan);

		//=== Fill Offset X and Y ===
		wsprintf(message,L"OX: %d OY: %d",static_cast<long>(_offsetX),static_cast<long>(_offsetY));
		PrintText(imageBuffer, 10, 290, message, chan);

		//=== Fill Average Mode and Average Count ===
		if (_averageMode==0)
		{
			wsprintf(message,L"Av?: N Av#: %d",static_cast<long>(_averageNum));
		}
		else
		{
			wsprintf(message,L"Av?: C Av#: %d",static_cast<long>(_averageNum));
		}
		PrintText(imageBuffer, 10, 330, message, chan);

		//=== Fill input range A,B,C,D ===
		wsprintf(message,L"Input Range:");
		PrintText(imageBuffer, 10, 370, message, chan);
		wsprintf(message,L"A:%d B:%d C:%d D:%d",static_cast<long>(_inputRangeChannel1),static_cast<long>(_inputRangeChannel2),static_cast<long>(_inputRangeChannel3),static_cast<long>(_inputRangeChannel4));
		PrintText(imageBuffer, 10, 410, message, chan);

	}
}


long ThorLSMCam::PostflightAcquisition(char * pDataBuffer)
{
	return 1;
}

