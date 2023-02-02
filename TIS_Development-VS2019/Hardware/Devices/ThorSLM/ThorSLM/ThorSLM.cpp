// ThorSLM.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ThorSLM.h"
#include "ThorSLMSetupXML.h"
#include "Strsafe.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <complex>


///static Members & global variables:
wchar_t ThorSLM::_errMsg[MSG_SIZE];
HANDLE ThorSLM::_hStatusHandle = NULL;
bool ThorSLM::_instanceFlag = false;
unique_ptr<ThorSLM> ThorSLM::_single(new ThorSLM());
CRITICAL_SECTION ThorSLM::_accessCritSection = { 0 };
ISLM* ThorSLM::_slmDevice = NULL;
unique_ptr<ThorSLMXML> pSetup(new ThorSLMXML());
float* ThorSLM::_fpPointsXY[MAX_ARRAY_CNT] = { NULL };
float* ThorSLM::_fpPointsXYZ[MAX_ARRAY_CNT] = { NULL };
long ThorSLM::_arrayOrFileID = 0;		//0-based buffer or sequence index
long ThorSLM::_bufferCount = 1;			//1-based total buffer size
long ThorSLM::_slmRuntimeCalculate = FALSE;	//[1]:calculate transient frames at runtime
unique_ptr<WinDVIDLL> winDVI(new WinDVIDLL(L".\\WinDVI.dll"));
unique_ptr<HoloGenDLL> holoGen(new HoloGenDLL(L".\\HologramGenerator.dll"));
TaskHandle ThorSLM::_taskHandleCI = NULL;
wchar_t message[_MAX_PATH];
unsigned int ThorSLM::_slmTimeout = SLM_TIMEOUT_MIN;
std::vector<unsigned int> ThorSLM::_slmSeqVec;			//sequence of pattern order, normal[0,1,2... & size == _bufferCount] if not doing runtime calculation
mutex ThorSLM::_callbackMutex;							//mutex lock to update static params used in callback
long ThorSLM::_overDrive = 0;
std::string ThorSLM::_pSlmName = "";
HighPerfTimer _timer;
vector<MemoryStruct<float>> _p3DHoloBufVec;

wchar_t drive[_MAX_DRIVE];
wchar_t dir[_MAX_DIR];
wchar_t fname[_MAX_FNAME];
wchar_t ext[_MAX_EXT];
wchar_t rawPath[_MAX_PATH];

ThorSLM::ThorSLM() :
	DEFAULT_PIXEL_X(512),
	DEFAULT_PIXEL_Y(512),
	DEFAULT_TRUE_FRAMES(5),
	DEFAULT_TRANSIENT_FRAMES(10U),
	MAX_TRANSIENT_FRAMES(20U)
{
	_deviceCount = 0;
	_deviceDetected = FALSE;
	_fileSettingsLoaded = FALSE;
	_pixelSize[0] = _pixelSize[1] = 512;	//X, Y
	_fpPointsXYSize = new long[MAX_ARRAY_CNT];
	_fpPointsXYZSize = new long[MAX_ARRAY_CNT];
	_tableLUT = new unsigned short[LUT_SIZE];
	_imgWavefront = NULL;
	SetDefault();
	_transientFrames = DEFAULT_TRANSIENT_FRAMES;
	_dmdMode = _loadPhaseDirectly = _slm3D = FALSE;
	_doHologram = false;
	_pixelPitchUM = 15.0;
	_flatDiagRatio = 1;
	_flatPowerRange[0] = 25;
	_flatPowerRange[1] = 75;
	_selectWavelength = 0;
	_power2Px = 0;
	_skipFitting = FALSE;
	for (int i = 0; i < (int)Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT; i++)
	{
		_fitCoeff[i] = new double[PROJECT_COEFF_CNT];
		_fitCoeff3D[i] = new double[HOMOGENEOUS_COEFF_CNT];
		_phaseMax[i] = 255;
		_wavelength[i] = 0;
		_defocusParam[i][0] = 0.0;		//applied z defocus [um]
		_defocusParam[i][1] = 1.33;		//refractive index
		_defocusParam[i][2] = 6.25;		//effective focal length [mm]
		_defocusParam[i][3] = 0.0;		//saved z defocus [um]

	}
	_persistHologramZone[0] = _persistHologramZone[1] = FALSE;
}

ThorSLM* ThorSLM::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorSLM());
		_instanceFlag = true;
		::InitializeCriticalSection(&_accessCritSection);
	}
	return _single.get();
}

ThorSLM::~ThorSLM()
{
	_instanceFlag = false;
	::DeleteCriticalSection(&_accessCritSection);

	for (int i = 0; i < Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT; i++)
	{
		SAFE_DELETE_ARRAY(_fitCoeff[i]);
		SAFE_DELETE_ARRAY(_fitCoeff3D[i]);
	}

	ReleaseMem();
	SAFE_DELETE_ARRAY(_fpPointsXYSize);
	SAFE_DELETE_ARRAY(_fpPointsXYZSize);

	SAFE_DELETE_ARRAY(_tableLUT);
}

long ThorSLM::FindDevices(long& deviceCount)
{
	_deviceCount = 1;	//make simulation mode available
	_deviceDetected = TRUE;
	try
	{
		pSetup.reset(new ThorSLMXML());
		pSetup.get()->OpenConfigFile();

		if (FALSE == pSetup.get()->GetBlank(_dualPatternShiftPx, _persistHologramZone[0], _persistHologramZone[1]))
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"GetBlank from ThorSLMSettings failed.");
			LogMessage(_errMsg, ERROR_EVENT);
			return 0;
		}
		if (FALSE == pSetup.get()->GetSpec(_pSlmName, _dmdMode, _overDrive, _transientFrames, _pixelPitchUM, _flatDiagRatio, _flatPowerRange[0], _flatPowerRange[1], _pixelSize[0], _pixelSize[1], _lutFile, _overDrivelutFile, _wavefrontFile))
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"GetSpec from ThorSLMSettings failed.");
			LogMessage(_errMsg, ERROR_EVENT);
			return 0;
		}

		//expect to locate 1 SLM device, may extend to support more later
		_slmManager.get()->getInstance()->FindSLMs((char*)pSetup.get());
		_slmDevice = _slmManager.get()->getInstance()->GetSLM(1);
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLM FindDevices failed.");
		LogMessage(_errMsg, ERROR_EVENT);
	}
	deviceCount = _deviceCount;
	return deviceCount;
}

long ThorSLM::SelectDevice(const long device)
{
	long ret = TRUE;

	if (!_fileSettingsLoaded)
	{
		//XML settings retrieval functions will throw an exception if tags or attributes are missing
		//catch each independetly so that as many tags as possible can be read
		try
		{
			_wavelength[0] = _wavelength[1] = 0;
			for (int i = 0; i < Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT; i++)
			{
				if (FALSE == pSetup.get()->GetCalibration(i + 1, _wavelength[i], _phaseMax[i], _defocusParam[i][3], _fitCoeff[i][0], _fitCoeff[i][1], _fitCoeff[i][2], _fitCoeff[i][3], _fitCoeff[i][4], _fitCoeff[i][5], _fitCoeff[i][6], _fitCoeff[i][7]))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetCalibration from ThorSLMSettings failed.");
					LogMessage(_errMsg, ERROR_EVENT);
				}
				if (FALSE == pSetup.get()->GetCalibration3D(i + 1, _wavelength[i], _phaseMax[i], _defocusParam[i][3], _fitCoeff3D[i], (int)HOMOGENEOUS_COEFF_CNT))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetCalibration3D from ThorSLMSettings failed.");
					LogMessage(_errMsg, ERROR_EVENT);
				}
				_defocusParam[i][0] = _defocusParam[i][3];		//update z defocus[0] value to be changed by user
				_phaseMax[i] = max(0, min(255, _phaseMax[i]));
				if (FALSE == pSetup.get()->GetPostTransform(i + 1, _verticalFlip[i], _rotateAngle[i], _scaleFactor[i][0], _scaleFactor[i][1], _offsetPixels[i][0], _offsetPixels[i][1]))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetPostTransform from ThorSLMSettings failed.");
					LogMessage(_errMsg, ERROR_EVENT);
				}
			}
			if (FALSE == pSetup.get()->GetBlank(_dualPatternShiftPx, _persistHologramZone[0], _persistHologramZone[1]))
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"GetBlank from ThorSLMSettings failed.");
				LogMessage(_errMsg, ERROR_EVENT);
			}
			if (FALSE == pSetup.get()->GetSpec(_pSlmName, _dmdMode, _overDrive, _transientFrames, _pixelPitchUM, _flatDiagRatio, _flatPowerRange[0], _flatPowerRange[1], _pixelSize[0], _pixelSize[1], _lutFile, _overDrivelutFile, _wavefrontFile))
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"GetSpec from ThorSLMSettings failed.");
				LogMessage(_errMsg, ERROR_EVENT);
			}
			if (FALSE == pSetup.get()->GetTrigger(_counterLine, _hwTriggerInput))
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"GetTrigger from ThorSLMSettings failed.");
				LogMessage(_errMsg, ERROR_EVENT);
			}
			if (FALSE == pSetup.get()->GetWinDVI(_monitorID))
			{
				_monitorID = L"";  //select last if not configured
				StringCbPrintfW(_errMsg, MSG_SIZE, L"GetWinDVI from ThorSLMSettings failed.");
				LogMessage(_errMsg, ERROR_EVENT);
			}
			winDVI->ChooseDVI(_monitorID.c_str());

			//load LUT and calibration:
			ReadLUTFile(StringToWString(_lutFile));

			if ((!_overDrive) || (0 != _pSlmName.compare("PDM512")))
			{
				ReadWavefrontFile(StringToWString(_wavefrontFile));
			}
		}
		catch (...)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SelectCamera from ThorSLM failed");
			LogMessage(_errMsg, ERROR_EVENT);
		}

		_fileSettingsLoaded = TRUE;

	}

	return ret;
}

long ThorSLM::TeardownDevice()
{
	try
	{
		if (IsOverdrive())
		{
			_slmDevice->TeardownSLM();
		}
		else
		{
			ReleaseDVI();
		}
		SetDefault();

		//reset flag:
		_fileSettingsLoaded = FALSE;
	}
	catch (...)
	{
	}

	ReleaseMem();
	_deviceDetected = FALSE;
	return TRUE;
}

long ThorSLM::GetLastErrorMsg(wchar_t* msg, long size)
{
	wcsncpy_s(msg, size, _errMsg, MSG_SIZE);

	//reset the error message
	_errMsg[0] = 0;
	return TRUE;
}

long ThorSLM::GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault)
{
	long ret = TRUE;
	switch (paramID)
	{
	case IDevice::PARAM_DEVICE_TYPE:
	{
		paramType = IDevice::SLM;
		paramAvailable = TRUE;
		paramMin = IDevice::SLM;
		paramMax = IDevice::SLM;
		paramDefault = IDevice::SLM;
		paramReadOnly = TRUE;
	}
	break;
	case PARAM_CONNECTION_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = CONNECTION_READY;
		paramMax = CONNECTION_UNAVAILABLE;
		paramDefault = CONNECTION_UNAVAILABLE;
		paramReadOnly = TRUE;
	}
	break;
	case IDevice::PARAM_SLM_FUNC_MODE:
	{
		paramAvailable = TRUE;
		paramMin = SLMFunctionMode::LOAD_PHASE_ONLY;
		paramMax = SLMFunctionMode::LAST_FUNCTION;
		paramDefault = SLMFunctionMode::LOAD_PHASE_ONLY;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_POINTS_ARRAY:
	{
		paramType = IDevice::TYPE_BUFFER;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 0;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_ARRAY_ID:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = (double)MAX_ARRAY_CNT - 1;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_PIXEL_X:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 1;
		paramMax = _pixelSize[0];
		paramDefault = DEFAULT_PIXEL_X;
		paramReadOnly = TRUE;
	}
	break;
	case IDevice::PARAM_SLM_PIXEL_Y:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 1;
		paramMax = _pixelSize[1];
		paramDefault = DEFAULT_PIXEL_Y;
		paramReadOnly = TRUE;
	}
	break;
	case IDevice::PARAM_SLM_SKIP_FITTING:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_WAVELENGTH:
	{
		paramType = IDevice::TYPE_BUFFER;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 10000;
		paramDefault = 1040;
		paramReadOnly = TRUE;
	}
	break;
	case IDevice::PARAM_SLM_WAVELENGTH_SELECT:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_3D:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_PHASE_DIRECT:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_RESET_AFFINE:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_BMP_FILENAME:
	{
		paramType = IDevice::TYPE_STRING;
		paramAvailable = TRUE;
		paramMin = NULL;
		paramMax = NULL;
		paramDefault = NULL;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_BLANK:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_DEFOCUS:
	{
		paramType = IDevice::TYPE_BUFFER;
		paramAvailable = TRUE;
		paramMin = 0;
		paramMax = 0;
		paramDefault = 0;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_SAVE_DEFOCUS:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case IDevice::PARAM_SLM_TIMEOUT:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = SLM_TIMEOUT_MIN;
		paramMax = INT_MAX;
		paramDefault = SLM_TIMEOUT_MIN;
		paramReadOnly = FALSE;
	}
	break;
	case PARAM_SLM_RUNTIME_CALC:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		paramReadOnly = FALSE;
	}
	break;
	case PARAM_SLM_SEQ_FILENAME:
	{
		paramType = IDevice::TYPE_STRING;
		paramAvailable = TRUE;
		paramMin = NULL;
		paramMax = NULL;
		paramDefault = NULL;
		paramReadOnly = FALSE;
	}
	break;
	case PARAM_SLM_DUAL_SHIFT_PX:
	{
		paramType = IDevice::TYPE_LONG;
		paramAvailable = TRUE;
		paramMin = -_pixelSize[0];
		paramMax = _pixelSize[0];
		paramDefault = 0;
		paramReadOnly = TRUE;
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

long ThorSLM::SetParam(const long paramID, const double param)
{
	long ret = TRUE;
	switch (paramID)
	{
	case IDevice::PARAM_SLM_FUNC_MODE:
		if ((param >= SLMFunctionMode::LOAD_PHASE_ONLY) && (param < SLMFunctionMode::LAST_FUNCTION))
		{
			_slmFuncMode = static_cast<long> (param);
		}
		break;
	case IDevice::PARAM_SLM_ARRAY_ID:
		if ((param >= 0) && (param < MAX_ARRAY_CNT))
		{
			_arrayOrFileID = static_cast<long> (param);
			_bufferCount = (_bufferCount < (_arrayOrFileID + 1)) ? (_arrayOrFileID + 1) : _bufferCount;
			if (IsOverdrive())
				_slmDevice->SetParam(ISLM::SLMParams::ARRAY_ID, param);
		}
		break;
	case IDevice::PARAM_SLM_PIXEL_X:
		if ((static_cast<long>(param) >= 1) && (static_cast<long>(param) <= _pixelSize[0]))
		{
			_pixelX = static_cast<long> (param);
		}
		break;
	case IDevice::PARAM_SLM_PIXEL_Y:
		if ((static_cast<long>(param) >= 1) && (static_cast<long>(param) <= _pixelSize[1]))
		{
			_pixelY = static_cast<long> (param);
		}
		break;
	case IDevice::PARAM_SLM_SKIP_FITTING:
		if ((param >= FALSE) && (param <= TRUE))
		{
			_skipFitting = static_cast<long> (param);
		}
		break;
	case IDevice::PARAM_SLM_WAVELENGTH_SELECT:
		if (0 <= param || param < Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT)
		{
			if (0 < (long)_wavelength[static_cast<long>(param)])
				_selectWavelength = static_cast<long>(param);
		}
		break;
	case IDevice::PARAM_SLM_3D:
		if ((param >= FALSE) && (param <= TRUE))
		{
			_slm3D = static_cast<long> (param);
		}
		break;
	case IDevice::PARAM_SLM_PHASE_DIRECT:
		if (FALSE <= param || param <= TRUE)
		{
			_loadPhaseDirectly = static_cast<long> (param);
		}
		break;
	case IDevice::PARAM_SLM_RESET_AFFINE:
		if ((param >= FALSE) && (param <= TRUE))
		{
			if (static_cast<long> (param))
			{
				//reset affine coefficients:
				if (_slm3D)
				{
					memset(_fitCoeff3D[_selectWavelength], 0x0, sizeof(double) * HOMOGENEOUS_COEFF_CNT);
					_fitCoeff3D[_selectWavelength][0] = _fitCoeff3D[_selectWavelength][5] = 1;
					_fitCoeff3D[_selectWavelength][10] = _fitCoeff3D[_selectWavelength][15] = 1;
				}
				else
				{
					_fitCoeff[_selectWavelength][0] = 1; _fitCoeff[_selectWavelength][1] = 0; _fitCoeff[_selectWavelength][2] = 0;
					_fitCoeff[_selectWavelength][3] = 0; _fitCoeff[_selectWavelength][4] = 1; _fitCoeff[_selectWavelength][5] = 0;
					_fitCoeff[_selectWavelength][6] = 0; _fitCoeff[_selectWavelength][7] = 0;

					//reset post affine transform params:
					_rotateAngle[_selectWavelength] = 0.0;
					_scaleFactor[_selectWavelength][0] = _scaleFactor[_selectWavelength][1] = 1.0;
					_verticalFlip[_selectWavelength] = _offsetPixels[_selectWavelength][0] = _offsetPixels[_selectWavelength][1] = 0;
				}
				PersistAffineValues();
			}
		}
		break;
	case IDevice::PARAM_SLM_BLANK:
		if ((param >= FALSE) && (param <= TRUE) && static_cast<long> (param))
			BlankSLM(ISLM::BLANK_ALL);
		break;
	case IDevice::PARAM_SLM_SAVE_DEFOCUS:
		if (static_cast<long> (param))
		{
			pSetup.get()->SetDefocus(_selectWavelength + 1, _defocusParam[_selectWavelength][0]);
			_defocusParam[_selectWavelength][3] = _defocusParam[_selectWavelength][0];	//update saved defocus value
		}
		break;
	case IDevice::PARAM_SLM_TIMEOUT:
		_slmTimeout = (param >= SLM_TIMEOUT_MIN) ? static_cast<unsigned int>(ceil(param / SLM_TIMEOUT_MIN) * SLM_TIMEOUT_MIN) : SLM_TIMEOUT_MIN;
		if (IsOverdrive())
			_slmDevice->SetParam(ISLM::SLMParams::TIMEOUT, _slmTimeout);
		break;
	case IDevice::PARAM_SLM_RUNTIME_CALC:
		if ((param >= FALSE) && (param <= TRUE))
		{
			_slmRuntimeCalculate = static_cast<long>(param);
			if (IsOverdrive())
				_slmDevice->SetParam(ISLM::SLMParams::RUNTIME_CALC, _slmRuntimeCalculate);
		}
		break;
	default:
		StringCbPrintfW(_errMsg, MSG_SIZE, L"SLM Parameter (%d) not implemented", paramID);
		LogMessage(_errMsg, ERROR_EVENT);
		ret = FALSE;
		break;
	}
	return ret;
}

long ThorSLM::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;
	float* pSrc = (float*)pBuffer;
	long centerTransVec[XY_COORD] = { 0 }; ///<translate vector [x,y] from image center to power of 2 square center
	BYTE* pImg = NULL;

	switch (paramID)
	{
	case IDevice::PARAM_SLM_POINTS_ARRAY:
		//image center at the first [x, y]
		_power2Px = static_cast<long>(max(max(pow(2, ceil(log2(_pixelSize[0]))), pow(2, ceil(log2(_pixelSize[1])))),
			max(pow(2, ceil(log2(*pSrc * 2))), pow(2, ceil(log2(*(pSrc + 1) * 2))))));
		for (int i = 0; i < XY_COORD; i++)
		{
			centerTransVec[i] = max(0, static_cast<long>(floor(_power2Px / 2)) - static_cast<long>(*pSrc));
			pSrc++;
		}
		if (_slm3D)
		{
			//skip z value of center
			pSrc++;
			//Expect this waveform include X, Y, Z (interleaved), 
			//translate both from[0] and target[1] to power of 2 square coordinate
			_fpPointsXYZ[_arrayOrFileID] = (float*)realloc((void*)_fpPointsXYZ[_arrayOrFileID], (size - XYZ_COORD) * sizeof(float));
			if (NULL == _fpPointsXYZ[_arrayOrFileID])
				return FALSE;
			for (long i = 0; i < (size - (long)XYZ_COORD); i++)
			{
				_fpPointsXYZ[_arrayOrFileID][i] = (XY_COORD == i % XYZ_COORD) ? (*pSrc) : (*pSrc + centerTransVec[i % (long)XY_COORD]);
				pSrc++;
			}
			_fpPointsXYZSize[_arrayOrFileID] = (size - XYZ_COORD);
		}
		else
		{
			//Expect this waveform include X and Y (interleaved), 
			//translate both from[0] and target[1] to power of 2 square coordinate
			_fpPointsXY[_arrayOrFileID] = (float*)realloc((void*)_fpPointsXY[_arrayOrFileID], (size - XY_COORD) * sizeof(float));
			if (NULL == _fpPointsXY[_arrayOrFileID])
				return FALSE;
			for (long i = 0; i < (size - (long)XY_COORD); i++)
			{
				_fpPointsXY[_arrayOrFileID][i] = *pSrc + centerTransVec[i % (long)XY_COORD];
				pSrc++;
			}
			_fpPointsXYSize[_arrayOrFileID] = (size - XY_COORD);
		}
		break;
	case IDevice::PARAM_SLM_DEFOCUS:
		SAFE_MEMCPY((void*)_defocusParam[_selectWavelength], size * sizeof(double), (void*)pBuffer);

		//**********************************************//
		//*************	   Try defocus	   *************//
		//**********************************************//

		//return if no image acquired
		if (0 == _lastHoloBuf[1].GetSize())
			return FALSE;

#ifdef _DEBUG
		_timer.Start();
#endif
		::EnterCriticalSection(&_accessCritSection);

		//defocus[0] from focus[1] buffer
		CopyDefocus(1, 0);

		//apply defocus phase
		DefocusNormalizeHologram(FALSE);

		//push to display
		BITMAPINFO bmi = _lastHoloBuf[0].GetInfo();
		pImg = CropHologramBMP(NULL, _lastHoloBuf[0].GetMem(), bmi);
		ReadAndScaleBitmap(pImg, bmi);
		if (!IsOverdrive()) { winDVI->DisplayBMP(_arrayOrFileID); }

		::LeaveCriticalSection(&_accessCritSection);
#ifdef _DEBUG
		_timer.Stop();
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLM apply defocus time: %d ms", static_cast<int>(_timer.ElapsedMilliseconds()));
		LogMessage(_errMsg, INFORMATION_EVENT);
#endif
		break;
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

long ThorSLM::SetParamString(const long paramID, wchar_t* str)
{
	long ret = TRUE;
	switch (paramID)
	{
	case IDevice::PARAM_SLM_BMP_FILENAME:
		_bmpPathAndName = std::wstring(str);
		break;
	case IDevice::PARAM_SLM_SEQ_FILENAME:
		if (FALSE == _slmRuntimeCalculate)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLM cannot set sequence file while not in PARAM_SLM_RUNTIME_CALC mode.");
			LogMessage(_errMsg, ERROR_EVENT);
			return FALSE;
		}

		//load sequence file
		_callbackMutex.lock();
		ret = ResetSequence(str);
		_seqPathAndName = (TRUE == ret) ? L"" : std::wstring(str);
		if (FALSE == ret)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLM unable to load sequence file.");
			LogMessage(_errMsg, ERROR_EVENT);
			_callbackMutex.unlock();
			return FALSE;
		}

		//update first bmp
		if (IsOverdrive())
		{
			_slmDevice->SetParam(ISLM::SLMParams::WRITE_TRANSIANT_BUFFER, _slmSeqVec.at(_arrayOrFileID));
		}
		else
		{
			winDVI->DisplayBMP(_slmSeqVec.at(_arrayOrFileID));
		}
		_callbackMutex.unlock();
		break;
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

long ThorSLM::GetParam(const long paramID, double& param)
{
	long ret = TRUE;
	double dVal = 0;
	switch (paramID)
	{
	case IDevice::PARAM_DEVICE_TYPE:
		param = IDevice::SLM;
		break;
	case IDevice::PARAM_CONNECTION_STATUS:
		param = (_deviceDetected) ? (double)CONNECTION_READY : (double)CONNECTION_UNAVAILABLE;
		break;
	case IDevice::PARAM_SLM_FUNC_MODE:
		param = _slmFuncMode;
		break;
	case IDevice::PARAM_SLM_ARRAY_ID:
		param = _arrayOrFileID;
		break;
	case IDevice::PARAM_SLM_PIXEL_X:
		param = _pixelX;
		break;
	case IDevice::PARAM_SLM_PIXEL_Y:
		param = _pixelY;
		break;
	case IDevice::PARAM_SLM_WAVELENGTH_SELECT:
		param = _selectWavelength;
		break;
	case IDevice::PARAM_SLM_3D:
		param = _slm3D;
		break;
	case IDevice::PARAM_SLM_PHASE_DIRECT:
		param = _loadPhaseDirectly;
		break;
	case IDevice::PARAM_SLM_RESET_AFFINE:
	case IDevice::PARAM_SLM_BLANK:
	case IDevice::PARAM_SLM_SAVE_DEFOCUS:
		param = FALSE;
		break;
	case IDevice::PARAM_SLM_TIMEOUT:
		if (IsOverdrive())
		{
			_slmDevice->GetParam(ISLM::SLMParams::TIMEOUT, dVal);
			_slmTimeout = static_cast<unsigned int>(dVal);
		}
		param = _slmTimeout;
		break;
	case IDevice::PARAM_SLM_RUNTIME_CALC:
		param = _slmRuntimeCalculate;
		break;
	case IDevice::PARAM_SLM_SKIP_FITTING:
		param = _skipFitting;
		break;
	case IDevice::PARAM_SLM_DUAL_SHIFT_PX:
		param = _dualPatternShiftPx;
		break;
	default:
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter (%d) not implemented", paramID);
		LogMessage(_errMsg, ERROR_EVENT);
		ret = FALSE;
		break;
	}

	return ret;
}

long ThorSLM::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;
	switch (paramID)
	{
	case IDevice::PARAM_SLM_POINTS_ARRAY:
		if (_slm3D)
		{
			size = _fpPointsXYZSize[_arrayOrFileID];
			pBuffer = (char*)_fpPointsXYZ[_arrayOrFileID];
		}
		else
		{
			size = _fpPointsXYSize[_arrayOrFileID];
			pBuffer = (char*)_fpPointsXY[_arrayOrFileID];
		}
		break;
	case IDevice::PARAM_SLM_WAVELENGTH:
		size = Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT;
		SAFE_MEMCPY((void*)pBuffer, size * sizeof(double), _wavelength);
		break;
	case IDevice::PARAM_SLM_DEFOCUS:
		SAFE_MEMCPY((void*)pBuffer, size * sizeof(double), _defocusParam[_selectWavelength]);
		break;
	default:
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter (%d) not implemented", paramID);
		LogMessage(_errMsg, ERROR_EVENT);
		ret = FALSE;
		break;
	}

	return ret;
}

long ThorSLM::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = TRUE;
	switch (paramID)
	{
	case IDevice::PARAM_SLM_BMP_FILENAME:
		str = (wchar_t*)_bmpPathAndName.c_str();
		size = static_cast<long>(_bmpPathAndName.size());
		break;
	case IDevice::PARAM_SLM_SEQ_FILENAME:
		str = (wchar_t*)_seqPathAndName.c_str();
		size = static_cast<long>(_seqPathAndName.size());
		break;
	default:
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Parameter (%d) not implemented", paramID);
		LogMessage(_errMsg, ERROR_EVENT);
		ret = FALSE;
		break;
	}
	return ret;
}

long ThorSLM::PreflightPosition()
{
	return TRUE;
}

long ThorSLM::SetupPosition()
{
	long ret = TRUE;
	try
	{
		switch (_slmFuncMode)
		{
		case SLMFunctionMode::LOAD_PHASE_ONLY:
			_doHologram = (FALSE == _loadPhaseDirectly && FALSE == _dmdMode && FALSE == _slm3D);
			ret = LoadHologram();
			break;
		case SLMFunctionMode::PHASE_CALIBRATION:
			if (_slm3D)
			{
				if ((NULL != _fpPointsXYZ[0]) && (NULL != _fpPointsXYZ[1]) && (_fpPointsXYZSize[0] == _fpPointsXYZSize[1]))
				{
					holoGen->SetSize(_power2Px, _power2Px, 1.0);	//pixel size is not critical in coefficient calculation
					if (TRUE == holoGen->CalculateCoeffs(_fpPointsXYZ[0], _fpPointsXYZ[1], _fpPointsXYZSize[1], GeoFittingAlg::HOMOGENEOUS, _fitCoeff3D[_selectWavelength]))
					{
						PersistAffineValues();
					}
				}
			}
			else
			{
				if ((NULL != _fpPointsXY[0]) && (NULL != _fpPointsXY[1]) && (_fpPointsXYSize[0] == _fpPointsXYSize[1]))
				{
					bool doAffine = false;
					//not changing affine coeffs if the two point arrays are identical:
					for (int i = 0; i < _fpPointsXYSize[1]; i++)
					{
						if (_fpPointsXY[0][i] != _fpPointsXY[1][i])
						{
							doAffine = true;
							break;
						}
					}
					//affine transform from source to target: _fpPointsXY[0] is source, 
					//_fpPointsXY[1] is target with the same length:
					if (doAffine)
					{
						//flip, scale & rotate before affine coeffs calculation:
						if (_verticalFlip[_selectWavelength])
						{
							CoordinatesVerticalFlip(_fpPointsXY[0], _fpPointsXYSize[0]);
							CoordinatesVerticalFlip(_fpPointsXY[1], _fpPointsXYSize[1]);
						}
						if (((1.0 != _scaleFactor[_selectWavelength][0]) || (1.0 != _scaleFactor[_selectWavelength][1])) && ((0.0 < _scaleFactor[_selectWavelength][0]) && (0.0 < _scaleFactor[_selectWavelength][1])))
						{
							CoordinatesScale(_fpPointsXY[0], _fpPointsXYSize[0], _scaleFactor[_selectWavelength][0], _scaleFactor[_selectWavelength][1]);
							CoordinatesScale(_fpPointsXY[1], _fpPointsXYSize[1], _scaleFactor[_selectWavelength][0], _scaleFactor[_selectWavelength][1]);
						}
						if (0.0 != _rotateAngle[_selectWavelength])
						{
							CoordinatesRotate(_fpPointsXY[0], _fpPointsXYSize[0], _rotateAngle[_selectWavelength]);
							CoordinatesRotate(_fpPointsXY[1], _fpPointsXYSize[1], _rotateAngle[_selectWavelength]);
						}
						holoGen->SetSize(_power2Px, _power2Px, 1.0);	//pixel size is not critical in coefficient calculation
						if (TRUE == holoGen->CalculateCoeffs(_fpPointsXY[0], _fpPointsXY[1], _fpPointsXYSize[1], GeoFittingAlg::PROJECTIVE, _fitCoeff[_selectWavelength]))
						{
							PersistAffineValues();
						}
					}
				}

				//reset buffer count since only write one calibration frame:
				_bufferCount = 1;
				_doHologram = true;
				ret = LoadHologram();
			}
			break;
		case SLMFunctionMode::SAVE_PHASE:
			_doHologram = (TRUE == _loadPhaseDirectly && FALSE == _dmdMode) || (TRUE == _slm3D);
			ret = (TRUE == _slm3D) ? Save3DHologram() : SaveHologram(FALSE == _loadPhaseDirectly);
			break;
		default:
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetupSLM Failed: Invalid SLM Function Mode");
			LogMessage(_errMsg, VERBOSE_EVENT);
			ret = FALSE;
			break;
		}
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"SetupSLM failed.");
		LogMessage(_errMsg, ERROR_EVENT);
		ret = FALSE;
	}
	return ret;
}

long ThorSLM::StartPosition()
{
	long ret = TRUE;
	_callbackMutex.lock();

	try
	{
		switch (_slmFuncMode)
		{
		case SLMFunctionMode::LOAD_PHASE_ONLY:
			//set normal sequence
			if (FALSE == ResetSequence())
			{
				_callbackMutex.unlock();
				return FALSE;
			}
			if (IsOverdrive())
			{
				if (1 < _bufferCount)
				{
					SetupHWTriggerIn();
				}
				_slmDevice->StartSLM();
			}
			else
			{
				winDVI->DisplayBMP(_slmSeqVec.at(_arrayOrFileID));

				SetupHWTriggerIn();
			}
			break;
		case SLMFunctionMode::PHASE_CALIBRATION:
			if ((_overDrive) && (0 == _pSlmName.compare("PDM512")))
			{
				//already wrote, do nothing
			}
			else
			{
				//Move CreateDVIWindow to Setup for status thread to be created...
				winDVI->DisplayBMP(_arrayOrFileID);
			}
			break;
		default:
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Invalid SLM function mode for StartPosition");
			LogMessage(_errMsg, VERBOSE_EVENT);
			break;
		}
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"SLM Failed to StartAcquisition");
		LogMessage(_errMsg, ERROR_EVENT);
		ret = FALSE;
	}
	_callbackMutex.unlock();
	return ret;
}

long ThorSLM::StatusPosition(long& status)
{
	switch (_slmFuncMode)
	{
	case SLMFunctionMode::LOAD_PHASE_ONLY:
	case SLMFunctionMode::PHASE_CALIBRATION:
		if ((_overDrive) && (0 == _pSlmName.compare("PDM512")))
		{
			//already wrote or transient buf prepared:
			status = StatusType::STATUS_READY;
			return TRUE;
		}
		else
		{
			return winDVI->GetStatus(status);
		}
	default:
		status = StatusType::STATUS_READY;
		return TRUE;
	}
	return FALSE;
}

long ThorSLM::ReadPosition(DeviceType deviceType, double& pos)
{
	return TRUE;
}

long ThorSLM::PostflightPosition()
{
	long ret = TRUE;

	//terminate HW trigger task if any
	CloseNITasks();
	//reset buffer count before next load session
	_bufferCount = 1;

	switch (_slmFuncMode)
	{
	case SLMFunctionMode::LOAD_PHASE_ONLY:
		if (IsOverdrive())
			_slmDevice->StopSLM();

		//do blank
		if (FALSE == _persistHologramZone[0] && FALSE == _persistHologramZone[1])
		{
			SetParam((long)(IDevice::PARAM_SLM_BLANK), TRUE);
			if (!IsOverdrive())
				winDVI->ClearBMPs();
		}
		else if (FALSE == _persistHologramZone[0])
		{
			BlankSLM(ISLM::BLANK_LEFT);
		}
		else if (FALSE == _persistHologramZone[1])
		{
			BlankSLM(ISLM::BLANK_RIGHT);
		}
		break;
	case SLMFunctionMode::PHASE_CALIBRATION:
		_doHologram = true;
		SaveHologram(false);
		return TRUE;
	}
	return ret;
}

void ThorSLM::SetStatusHandle(HANDLE handle)
{
	_hStatusHandle = handle;
}

/// ***		Private Functions	*** ///

int32 CVICALLBACK ThorSLM::HWTriggerCallback(TaskHandle taskHandle, int32 signalID, void* callbackData)
{
	_callbackMutex.lock();

	if (0 >= _slmSeqVec.size())
		goto ERROR_STATE;

	_arrayOrFileID = (_arrayOrFileID + (long)1) % static_cast<long>(_slmSeqVec.size());	//circular index upto sequence count (== _bufferCount if not runtime calculation)

	if (TRUE == ThorSLM::IsOverdrive())
	{
		if (FALSE == _slmDevice->UpdateSLM(_slmSeqVec.at(_arrayOrFileID)))
			goto ERROR_STATE;
	}
	else
	{
		winDVI->DisplayBMP(_slmSeqVec.at(_arrayOrFileID));
	}
	_callbackMutex.unlock();
	return 0;

ERROR_STATE:
	_callbackMutex.unlock();
	StringCbPrintfW(_errMsg, MSG_SIZE, L"HWTriggerCallback failed, could be unable to calculate transient frames.");
	LogMessage(_errMsg, ERROR_EVENT);
	return 0;
}

BOOL ThorSLM::IsOverdrive()
{
	double dVal = 0;
	if (NULL == _slmDevice)
		return FALSE;

	return ((_overDrive) && (0 == _pSlmName.compare("PDM512")) && (TRUE == _slmDevice->GetParam(ISLM::SLMParams::IS_AVAILABLE, dVal)) && TRUE == (int)dVal) ? TRUE : FALSE;
}

void ThorSLM::BlankSLM(ISLM::SLMBlank bmode)
{
	unsigned char* pImg = NULL;
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = 40;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = 0;
	bmi.bmiHeader.biXPelsPerMeter = 3780;
	bmi.bmiHeader.biYPelsPerMeter = 3780;
	bmi.bmiHeader.biClrUsed = 256;
	bmi.bmiHeader.biClrImportant = 256;
	bmi.bmiHeader.biWidth = _pixelSize[0];
	bmi.bmiHeader.biHeight = _pixelSize[1];
	bmi.bmiHeader.biBitCount = CHAR_BIT * RGB_CNT;
	bmi.bmiHeader.biSizeImage = (DWORD)(_pixelSize[0] * _pixelSize[1] * (long)RGB_CNT * sizeof(unsigned char));
	size_t shiftRGB = _dualPatternShiftPx * RGB_CNT;	//[+] extend left region to right, [-] extend right region to left
	size_t widthRGB = 0;

	if (ISLM::BLANK_ALL == bmode)
	{
		::EnterCriticalSection(&_accessCritSection);

		//blank need to consider defocus calibration,
		//need rework in partial blanking mode
		_power2Px = static_cast<long>(max(max(pow(2, ceil(log2(_pixelSize[0]))), pow(2, ceil(log2(_pixelSize[1])))), _power2Px));
		holoGen->SetSize(_power2Px, _power2Px, 1.0);	//pixel size is not critical in coefficient calculation

		bmi.bmiHeader.biWidth = _power2Px;
		bmi.bmiHeader.biHeight = _power2Px;
		bmi.bmiHeader.biBitCount = CHAR_BIT;
		bmi.bmiHeader.biSizeImage = (DWORD)(pow(_power2Px, 2) * sizeof(float));
		_lastHoloBuf[0].CallocMemChk(bmi.bmiHeader.biSizeImage);
		_lastHoloBuf[0].SetInfo(bmi);

		CopyDefocus(0, 1);

		DefocusNormalizeHologram(TRUE);

		pImg = CropHologramBMP(NULL, _lastHoloBuf[0].GetMem(), bmi);
		ReadAndScaleBitmap(pImg, bmi);
		if (!IsOverdrive()) { winDVI->DisplayBMP(_arrayOrFileID); }

		::LeaveCriticalSection(&_accessCritSection);
	}
	else
	{
		//prepare buffer
		_lastBmpBuf.CallocMemChk(bmi.bmiHeader.biSizeImage);
		_lastBmpBuf.SetInfo(bmi);

		//fetch for on-board buffer
		_callbackMutex.lock();
		if (IsOverdrive())
		{
			_slmDevice->GetParamBuffer(ISLM::SLMParams::GET_CURRENT_BUFFER, (char*)_lastBmpBuf.GetMem(), static_cast<long>(_lastBmpBuf.GetInfo().bmiHeader.biSizeImage));
		}
		else
		{
			winDVI->GetBMP(_slmSeqVec.at(_arrayOrFileID), _lastBmpBuf.GetMem(), _lastBmpBuf.GetInfo());
		}

		//do half blanking
		pImg = _lastBmpBuf.GetMem();
		widthRGB = _lastBmpBuf.GetInfo().bmiHeader.biWidth * RGB_CNT;
		if (ISLM::BLANK_LEFT == bmode)
		{
			for (int y = 0; y < _lastBmpBuf.GetInfo().bmiHeader.biHeight; ++y)
				memset(pImg + y * widthRGB, 0x0, min(widthRGB / 2 + shiftRGB, widthRGB));
		}
		else if (ISLM::BLANK_RIGHT == bmode)
		{
			for (int y = 0; y < _lastBmpBuf.GetInfo().bmiHeader.biHeight; ++y)
				memset(pImg + y * widthRGB + min(widthRGB / 2 + shiftRGB, widthRGB), 0x0, max(widthRGB / 2 - shiftRGB, 0));
		}

		//push buffer to device
		if (IsOverdrive())
		{
			_slmDevice->SetParamBuffer(ISLM::SLMParams::WRITE_BUFFER, (char*)_lastBmpBuf.GetMem(), static_cast<long>(_lastBmpBuf.GetInfo().bmiHeader.biSizeImage));
		}
		else
		{
			winDVI->EditBMP(0, _lastBmpBuf.GetMem(), _lastBmpBuf.GetInfo());
			winDVI->CreateDVIWindow((int)_lastBmpBuf.GetInfo().bmiHeader.biWidth, (int)_lastBmpBuf.GetInfo().bmiHeader.biHeight);
			winDVI->DisplayBMP(0);
		}
		_callbackMutex.unlock();
	}
}

void ThorSLM::CloseNITasks()
{
	if (_taskHandleCI)
	{
		DAQmxStopTask(_taskHandleCI);
		DAQmxClearTask(_taskHandleCI);
		_taskHandleCI = NULL;
	}
}

//rotate interleaved x, y coordinates with angle (+) in clockwise
long ThorSLM::CoordinatesRotate(float* ptArrays, long size, double angle)
{
	long retVal = TRUE;
	double angleRad = angle * PI / HALF_CIRCLE;

	//expect x,y interleaved array with doubled size:
	if ((NULL == ptArrays) || (0 != (size % 2)))
		return FALSE;

	//offset to center for rotation:
	for (long i = 0; i < (size / 2); i++)
	{
		float x = ptArrays[2 * i] - (_power2Px / 2);
		float y = ptArrays[2 * i + 1] - (_power2Px / 2);
		ptArrays[2 * i] = static_cast<float>((x * cos(angleRad)) + (y * sin(angleRad))) + (_power2Px / 2);
		ptArrays[2 * i + 1] = static_cast<float>((y * cos(angleRad)) - (x * sin(angleRad))) + (_power2Px / 2);
	}
	return retVal;
}

//scale interleaved x, y coordinates from center
long ThorSLM::CoordinatesScale(float* ptArrays, long size, double scaleX, double scaleY)
{
	long retVal = TRUE;

	//expect x,y interleaved array with doubled size:
	if ((NULL == ptArrays) || (0 != (size % 2)))
		return FALSE;

	//offset to center for scale:
	for (long i = 0; i < (size / 2); i++)
	{
		float x = ptArrays[2 * i] - (_power2Px / 2);
		float y = ptArrays[2 * i + 1] - (_power2Px / 2);
		ptArrays[2 * i] = static_cast<float>(x * scaleX) + (_power2Px / 2);
		ptArrays[2 * i + 1] = static_cast<float>(y * scaleY) + (_power2Px / 2);
	}
	return retVal;
}

//vertical flip interleaved x, y coordinates
long ThorSLM::CoordinatesVerticalFlip(float* ptArrays, long size)
{
	long retVal = TRUE;

	//expect x,y interleaved array with doubled size:
	if ((NULL == ptArrays) || (0 != (size % 2)))
		return FALSE;

	for (long i = 0; i < (size / 2); i++)
	{
		float x = ptArrays[2 * i];
		float y = ptArrays[2 * i + 1];
		ptArrays[2 * i] = x;
		ptArrays[2 * i + 1] = _power2Px - y;
	}
	return retVal;
}

//Copy defocus struct buffer from one to the other
void ThorSLM::CopyDefocus(int from, int to)
{
	if (_lastHoloBuf[to].GetSize() != _lastHoloBuf[from].GetSize())
		_lastHoloBuf[to].ReallocMemChk(_lastHoloBuf[from].GetSize());
	SAFE_MEMCPY((void*)_lastHoloBuf[to].GetMem(), _lastHoloBuf[from].GetSize(), (void*)_lastHoloBuf[from].GetMem());
	_lastHoloBuf[to].SetInfo(_lastHoloBuf[from].GetInfo());
}

//Apply defocus on 2D hologram, savedZ(TRUE) to use calibrated z offset value
long ThorSLM::DefocusNormalizeHologram(long savedZ)
{
	double defocusZum = savedZ ? _defocusParam[_selectWavelength][3] : _defocusParam[_selectWavelength][0];
	if (0 != defocusZum)
	{
		double kz = 2 * PI * defocusZum * (double)Constants::UM_TO_MM / _wavelength[_selectWavelength];
		double NAeff = _power2Px * _pixelPitchUM / (2 * _defocusParam[_selectWavelength][2] * (double)Constants::UM_TO_MM);
		holoGen->SetDefocus(_defocusParam[_selectWavelength][1], NAeff, _power2Px);
		//expect buffer in unit [rad] to apply defocus
		holoGen->DefocusHologram(_lastHoloBuf[0].GetMem(), kz);
	}
	//map rad to pixel value
	return holoGen->NormalizePhase(_lastHoloBuf[0].GetMem());
}

long ThorSLM::LoadHologram()
{
	BITMAPINFO bmi;
	::EnterCriticalSection(&_accessCritSection);
	unsigned char* imgRead = GetAndProcessBMP(bmi);
	if (NULL == imgRead)
	{
		::LeaveCriticalSection(&_accessCritSection);
		return FALSE;
	}
	long ret = ReadAndScaleBitmap(imgRead, bmi);
	::LeaveCriticalSection(&_accessCritSection);
	return ret;
}

double ThorSLM::ParseZUM(wstring filename)
{
	size_t zfound1 = filename.find_last_of(L"[");
	size_t zfound2 = filename.find_last_of(L"]");
	return (wstring::npos != zfound1 && wstring::npos != zfound2) ? (double)_wtof(filename.substr(zfound1 + 1, zfound2 - zfound1 - 1).c_str()) : 0.0;
}

long ThorSLM::PersistAffineValues()
{
	long retVal = TRUE;
	//persist affine coefficients:
	if (_slm3D)
	{
		if (FALSE == pSetup.get()->SetCalibration3D(_selectWavelength + 1, _fitCoeff3D[_selectWavelength], HOMOGENEOUS_COEFF_CNT))
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetCalibration3D from ThorSLMSettings failed");
			LogMessage(_errMsg, ERROR_EVENT);
			retVal = FALSE;
		}
	}
	else
	{
		if (FALSE == pSetup.get()->SetCalibration(_selectWavelength + 1, _fitCoeff[_selectWavelength][0], _fitCoeff[_selectWavelength][1], _fitCoeff[_selectWavelength][2], _fitCoeff[_selectWavelength][3], _fitCoeff[_selectWavelength][4], _fitCoeff[_selectWavelength][5], _fitCoeff[_selectWavelength][6], _fitCoeff[_selectWavelength][7]))
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetCalibration from ThorSLMSettings failed");
			LogMessage(_errMsg, ERROR_EVENT);
			retVal = FALSE;
		}

		if (FALSE == pSetup.get()->SetPostTransform(_selectWavelength + 1, _verticalFlip[_selectWavelength], _rotateAngle[_selectWavelength], _scaleFactor[_selectWavelength][0], _scaleFactor[_selectWavelength][1], _offsetPixels[_selectWavelength][0], _offsetPixels[_selectWavelength][1]))
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetPreTransform from ThorSLMSettings failed");
			LogMessage(_errMsg, ERROR_EVENT);
			retVal = FALSE;
		}
	}
	//reset flag after files saved
	_fileSettingsLoaded = FALSE;
	return retVal;
}

BOOL ThorSLM::ReadLUTFile(std::wstring fileName)
{
	const int LUT_COLUMN_CNT = 2;
	FILE* stream;
	int seqnum, ReturnVal, tmpLUT;
	bool errorFlag = false;

	std::string fname = ConvertWStringToString(fileName);

	if (0 != fopen_s(&stream, fname.c_str(), "r"))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLM: open LUT file failed.");
		LogMessage(_errMsg, VERBOSE_EVENT);
	}

	//read LUT file or generate linear LUT:
	if ((stream != NULL) && (errorFlag == false))
	{
		for (int i = 0; i < LUT_SIZE; i++)
		{
			ReturnVal = fscanf_s(stream, "%d %d", &seqnum, &tmpLUT);
			if ((ReturnVal != LUT_COLUMN_CNT) || (seqnum != i) || (tmpLUT < 0) || (tmpLUT > (LUT_SIZE - 1)))
			{
				errorFlag = true;

				//close the file we opened
				fclose(stream);
				break;
			}
			_tableLUT[i] = (unsigned short)tmpLUT;
		}

		//close the file we opened
		fclose(stream);
	}
	if ((stream == NULL) || (errorFlag == true))
	{
		//otherwise hardcode a linear LUT
		for (int i = 0; i < LUT_SIZE; i++)
		{
			_tableLUT[i] = i;
		}
		return FALSE;
	}
	return TRUE;
}

BOOL ThorSLM::ReadWavefrontFile(std::wstring fileName)
{
	long newSize;
	BITMAPINFO bmi;

	//clear memory:
	SAFE_DELETE_ARRAY(_imgWavefront);

	//load calibration file:
	if (0 < fileName.length())
	{
		unsigned char* imgCalRead = LoadBMP(&bmi.bmiHeader, fileName.c_str());
		if (NULL == imgCalRead)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLM LoadWavefrontFile: load bmp file failed.");
			LogMessage(_errMsg, ERROR_EVENT);
			return FALSE;
		}
		//force calibration file to be color:
		if (RGB_CNT * CHAR_BIT != bmi.bmiHeader.biBitCount)
		{
			SAFE_DELETE_ARRAY(imgCalRead);
			return FALSE;
		}
		//convert buffer:
		_imgWavefront = ConvertBGRToRGBBuffer(imgCalRead, bmi.bmiHeader, &newSize);
		SAFE_DELETE_ARRAY(imgCalRead);
	}
	return TRUE;
}

BOOL ThorSLM::ReadAndScaleBitmap(unsigned char* imgRead, BITMAPINFO& bmi)
{
	long ret = TRUE, size = 0;
	unsigned char* imgBGR = ConvertRGBToBGRBuffer(imgRead, bmi.bmiHeader, &size);
	if (NULL == imgBGR)
	{
		SAFE_DELETE_ARRAY(imgRead);
		return FALSE;
	}

	_callbackMutex.lock();
	//meadowlark overdrive use provided dll:
	if (IsOverdrive())
	{
		try
		{
			if (1 == _bufferCount)
			{
				//release transient buffers:
				_slmDevice->SetParam(ISLM::SLMParams::RELEASE_TRANSIANT_BUFFER, TRUE);

				//persist first buffer:
				_slmDevice->SetParamBuffer(ISLM::SLMParams::WRITE_FIRST_BUFFER, (char*)imgBGR, static_cast<long>(size * sizeof(unsigned char)));
			}
			else
			{
				_slmDevice->SetParamBuffer(ISLM::SLMParams::SET_TRANSIANT_BUFFER, (char*)imgBGR, static_cast<long>(size * sizeof(unsigned char)));
			}

			//write image for proper transient frame calculation:
			_slmDevice->SetParamBuffer(ISLM::SLMParams::WRITE_BUFFER, (char*)imgBGR, static_cast<long>(size * sizeof(unsigned char)));
		}
		catch (...)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ReadAndScaleBitmap of overdrive SLM failed.");
			LogMessage(_errMsg, ERROR_EVENT);
			_slmDevice->GetLastErrorMsg(_errMsg, MSG_SIZE);
			LogMessage(_errMsg, ERROR_EVENT);
			ret = FALSE;
		}
		SAFE_DELETE_ARRAY(imgRead);
		SAFE_DELETE_ARRAY(imgBGR);
		_callbackMutex.unlock();
		return ret;
	}

	size_t tSize = (size_t)bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight * (int)RGB_CNT;
	BYTE* pImg = new BYTE[tSize];
	if (NULL == pImg)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLM ReadAndScaleBitmap: new alloc buffer failed.");
		LogMessage(_errMsg, ERROR_EVENT);
		SAFE_DELETE_ARRAY(imgRead);
		_callbackMutex.unlock();
		return FALSE;
	}

	if (0 == _pSlmName.compare("PDM512"))
	{
		//Meadowlark: (R: Volt, G: Hologram, B: 0)
		int PixVal;
		BYTE bByte, gByte, rByte;
		switch ((int)(size / bmi.bmiHeader.biWidth / bmi.bmiHeader.biHeight))
		{
		case 1:
			//make the info we have our most significan bits - GREEN
			for (int row = 0; row < bmi.bmiHeader.biHeight; row++)
			{
				for (int col = 0; col < bmi.bmiHeader.biWidth; col++)
				{
					PixVal = imgRead[row * bmi.bmiHeader.biWidth + col] << CHAR_BIT;
					PixVal = (NULL == _imgWavefront) ? _tableLUT[PixVal] :	//GREEN (high 8 bits) + RED (low 8 bits)
						_tableLUT[((PixVal + static_cast<int>((_imgWavefront[row * bmi.bmiHeader.biWidth * RGB_CNT + col * RGB_CNT + 1] << CHAR_BIT) + (_imgWavefront[row * bmi.bmiHeader.biWidth * RGB_CNT + col * RGB_CNT + 0]))) % (LUT_SIZE - 1))];
					pImg[row * bmi.bmiHeader.biWidth * (int)RGB_CNT + col * (int)RGB_CNT + 0] = static_cast<unsigned char>(PixVal);	//RED
					pImg[row * bmi.bmiHeader.biWidth * (int)RGB_CNT + col * (int)RGB_CNT + 1] = (PixVal >> CHAR_BIT);				//GREEN
					pImg[row * bmi.bmiHeader.biWidth * (int)RGB_CNT + col * (int)RGB_CNT + 2] = 0;									//BLUE
				}
			}
			break;
		case RGB_CNT:
			//not tested yet, should already be RGB:
			for (int row = 0; row < bmi.bmiHeader.biHeight; row++)
			{
				for (int col = 0; col < bmi.bmiHeader.biWidth; col++)
				{
					rByte = imgRead[row * bmi.bmiHeader.biWidth * RGB_CNT + col * RGB_CNT + 0];
					gByte = imgRead[row * bmi.bmiHeader.biWidth * RGB_CNT + col * RGB_CNT + 1];
					bByte = imgRead[row * bmi.bmiHeader.biWidth * RGB_CNT + col * RGB_CNT + 2];
					PixVal = (gByte << 8) + rByte;
					PixVal = (NULL == _imgWavefront) ? _tableLUT[PixVal] :	//GREEN (high 8 bits) + RED (low 8 bits)
						_tableLUT[((PixVal + static_cast<int>((_imgWavefront[row * bmi.bmiHeader.biWidth * RGB_CNT + col * RGB_CNT + 1] << CHAR_BIT) + (_imgWavefront[row * bmi.bmiHeader.biWidth * RGB_CNT + col * RGB_CNT + 0]))) % (LUT_SIZE - 1))];
					pImg[row * bmi.bmiHeader.biWidth * RGB_CNT + col * RGB_CNT + 0] = static_cast<unsigned char>(PixVal);	//RED
					pImg[row * bmi.bmiHeader.biWidth * RGB_CNT + col * RGB_CNT + 1] = (PixVal >> 8);						//GREEN
					pImg[row * bmi.bmiHeader.biWidth * RGB_CNT + col * RGB_CNT + 2] = 0;									//BLUE
				}
			}
			break;
		}
	}
	else
	{
		//General: RGB are identical
		for (int row = 0; row < bmi.bmiHeader.biHeight; row++)
		{
			for (int col = 0; col < bmi.bmiHeader.biWidth; col++)
			{
				for (int k = 0; k < (int)RGB_CNT; k++)
				{
					pImg[row * bmi.bmiHeader.biWidth * (int)RGB_CNT + col * (int)RGB_CNT + k] = imgRead[row * bmi.bmiHeader.biWidth + col];
				}
			}
		}

	}

	//add to DVI with converted-back bmp:
	bmi.bmiHeader.biSizeImage = static_cast<DWORD>(tSize);
	bmi.bmiHeader.biBitCount = CHAR_BIT * (int)RGB_CNT;
	imgBGR = ConvertRGBToBGRBuffer(pImg, bmi.bmiHeader, &size);
	winDVI->EditBMP(_arrayOrFileID, imgBGR, bmi);

	//clear:
	SAFE_DELETE_ARRAY(imgRead);
	SAFE_DELETE_ARRAY(pImg);
	SAFE_DELETE_ARRAY(imgBGR);

	//create window, let winDVI to decide 
	//whether to create a new one or not:
	winDVI->CreateDVIWindow(bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight);
	_callbackMutex.unlock();
	return TRUE;
}

void ThorSLM::ReleaseDVI()
{
	winDVI->ClearBMPs();
	winDVI->DestroyDVIWindow();
}

void ThorSLM::ReleaseMem()
{
	//leave destruction of pointers in destructor since 
	//user may reactivate device after teardown, although not recommanded:
	for (int i = 0; i < MAX_ARRAY_CNT; i++)
	{
		if (NULL != _fpPointsXY[i])
		{
			SAFE_DELETE_MEMORY(_fpPointsXY[i]);
			_fpPointsXYSize[i] = 0;
		}
		if (NULL != _fpPointsXYZ[i])
		{
			SAFE_DELETE_MEMORY(_fpPointsXYZ[i]);
			_fpPointsXYZSize[i] = 0;
		}
	}
	_lastHoloBuf[0].ReallocMemChk(0);
	_lastHoloBuf[1].ReallocMemChk(0);
	_lastBmpBuf.ReallocMemChk(0);

	if (IsOverdrive())
	{
		_callbackMutex.lock();
		_slmManager->getInstance()->ReleaseSLMs();
		_callbackMutex.unlock();
	}
}

long ThorSLM::SaveHologram(bool saveInSubFolder)
{
	long size;
	BITMAPINFO bmi;
	::EnterCriticalSection(&_accessCritSection);
	unsigned char* imgRead = GetAndProcessBMP(bmi);
	if (NULL == imgRead)
	{
		::LeaveCriticalSection(&_accessCritSection);
		return FALSE;
	}
	//save grayscale phase mask:
	_wsplitpath_s(_bmpPathAndName.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
	if (wcsstr(dir, L"SLMWaveforms"))	//only create sub-folder under "SLMWaveforms"
	{
		StringCbPrintfW(rawPath, _MAX_PATH, L"%s%s%s", drive, dir, L"PhaseMask\\");
		CreateDirectory(rawPath, NULL);
	}
	if (saveInSubFolder)
	{
		StringCbPrintfW(rawPath, _MAX_PATH, L"%s%s%s%s%s", drive, dir, L"PhaseMask\\", fname, ext);
	}
	else
	{
		StringCbPrintfW(rawPath, _MAX_PATH, L"%s%s%s%s", drive, dir, fname, ext);
	}

	unsigned char* imgBGR = ConvertRGBToBGRBuffer(imgRead, bmi.bmiHeader, &size);
	SaveBMP(imgBGR, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, size, rawPath);

	SAFE_DELETE_ARRAY(imgRead);
	SAFE_DELETE_ARRAY(imgBGR);
	::LeaveCriticalSection(&_accessCritSection);
	return TRUE;
}

long ThorSLM::Save3DHologram(bool doSearch, bool reset)
{
	WIN32_FIND_DATA ffd;
	std::vector<wstring> zfileNames;
	std::vector<float> kzValues, notFoundKz;
	unsigned char* imgRead = NULL;
	long size;

	_wsplitpath_s(_bmpPathAndName.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
	wstring baseName = fname;

	//determine z displacement [UM] including default defocus
	float kz = static_cast<float>(2 * PI * (ParseZUM(baseName) + _defocusParam[_selectWavelength][3]) * (double)Constants::UM_TO_MM / _wavelength[_selectWavelength]);

	//find file name base
	size_t found = baseName.find_last_of(L"_");
	if (wstring::npos != found)
		baseName = baseName.substr(0, found);

	//**********************************************************//
	//		discover all z frames or add one on the fly			//
	//**********************************************************//
	wstring searchPath = wstring(drive) + wstring(dir) + L"*.bmp";
	HANDLE hFind = FindFirstFile(searchPath.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
		return FALSE;	//no files found

	if (reset)
	{
		//reset all buffers for restart searching
		for (size_t i = 0; i < _p3DHoloBufVec.size(); i++)	_p3DHoloBufVec[i].ReallocMemChk(0);
		_p3DHoloBufVec.clear();
	}

	if (doSearch)
	{
		do
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				_wmakepath_s(rawPath, _MAX_PATH, drive, dir, ffd.cFileName, NULL);
				wstring foundName = ffd.cFileName;
				if (wstring::npos != foundName.find(baseName))
				{
					zfileNames.push_back(wstring(drive) + wstring(dir) + foundName);
					kzValues.push_back(static_cast<float>(2 * PI * (ParseZUM(foundName) + _defocusParam[_selectWavelength][3]) * (double)Constants::UM_TO_MM / _wavelength[_selectWavelength]));
				}
			}
		} while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
		notFoundKz = kzValues;
	}
	else
	{
		zfileNames.push_back(_bmpPathAndName);
		kzValues.push_back(kz);
	}

	//**************************************************************************************//
	//		create array of image buffers < z_value, buffer* > for hologram gen in 3D,		//
	//		save result as the last.														//
	//**************************************************************************************//

	_callbackMutex.lock();

#ifdef _DEBUG
	_timer.Start();
#endif
	try
	{
		//support either adding or replacing z frames
		if (!reset)
		{
			//replace existing, leave out the last since it holds the result
			for (size_t i = 0; i < kzValues.size(); i++)
			{
				bool found = false;
				for (size_t j = 0; j < _p3DHoloBufVec.size() - 1; j++)
				{
					if (kzValues[i] == _p3DHoloBufVec[j].GetKz())
					{
						imgRead = MapCalibrateHologram(zfileNames[i].c_str(), &_p3DHoloBufVec[j]);
						SAFE_DELETE_ARRAY(imgRead);
						found = true;
						break;
					}
				}
				if (!found)
					notFoundKz.push_back(kzValues[i]);
			}
		}

		//add if not found
		if (0 < notFoundKz.size())
		{
			//remove last since it is for holding the result
			if (0 < _p3DHoloBufVec.size())
			{
				_p3DHoloBufVec[_p3DHoloBufVec.size() - 1].ReallocMemChk(0);
				_p3DHoloBufVec.erase(_p3DHoloBufVec.end());
			}
			//resize to cover new items
			size_t currentCount = _p3DHoloBufVec.size();
			_p3DHoloBufVec.resize(currentCount + notFoundKz.size() + 1);
			for (size_t i = 0; i < notFoundKz.size(); i++)
			{
				_p3DHoloBufVec[currentCount + i] = MemoryStruct<float>();
				_p3DHoloBufVec[currentCount + i].SetKz(notFoundKz[i]);
				imgRead = MapCalibrateHologram(zfileNames[i].c_str(), &_p3DHoloBufVec[currentCount + i]);
				SAFE_DELETE_ARRAY(imgRead);
			}
			//prepare last to hold result
			_p3DHoloBufVec[_p3DHoloBufVec.size() - 1] = MemoryStruct<float>();
			_p3DHoloBufVec[_p3DHoloBufVec.size() - 1].SetInfo(_p3DHoloBufVec[0].GetInfo());
			_p3DHoloBufVec[_p3DHoloBufVec.size() - 1].CallocMemChk(_p3DHoloBufVec[0].GetInfo().bmiHeader.biWidth * _p3DHoloBufVec[0].GetInfo().bmiHeader.biHeight);
		}

		//execute hologram gen
		double NAeff = _power2Px * _pixelPitchUM / (2 * _defocusParam[_selectWavelength][2] * (double)Constants::UM_TO_MM);
		holoGen->SetCoeffs(GeoFittingAlg::HOMOGENEOUS, _fitCoeff3D[_selectWavelength]);
		holoGen->SetDefocus(_defocusParam[_selectWavelength][1], NAeff, _power2Px);
		holoGen->Generate3DHologram(_p3DHoloBufVec.data(), static_cast<int>(_p3DHoloBufVec.size()));
		holoGen->NormalizePhase(_p3DHoloBufVec[_p3DHoloBufVec.size() - 1].GetMem());

		//copy out the result and save as bmp
		BITMAPINFO bmi = _p3DHoloBufVec[_p3DHoloBufVec.size() - 1].GetInfo();
		unsigned char* imgRead = CropHologramBMP(NULL, _p3DHoloBufVec[_p3DHoloBufVec.size() - 1].GetMem(), bmi);
		if (NULL != imgRead)
		{
			wstring parent = wstring(dir).substr(0, wstring(dir).find_last_of(L"/\\"));
			searchPath = wstring(drive) + wstring(parent).substr(0, wstring(parent).find_last_of(L"/\\") + 1) + baseName + L".bmp";
			_p3DHoloBufVec[_p3DHoloBufVec.size() - 1].SetInfo(bmi);
			unsigned char* imgBGR = ConvertRGBToBGRBuffer(imgRead, _p3DHoloBufVec[_p3DHoloBufVec.size() - 1].GetInfo().bmiHeader, &size);
			SaveBMP(imgBGR, _p3DHoloBufVec[_p3DHoloBufVec.size() - 1].GetInfo().bmiHeader.biWidth, _p3DHoloBufVec[_p3DHoloBufVec.size() - 1].GetInfo().bmiHeader.biHeight, size, searchPath.c_str());

			SAFE_DELETE_ARRAY(imgRead);
			SAFE_DELETE_ARRAY(imgBGR);
		}
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLM Save3DHologram failed.");
		LogMessage(_errMsg, ERROR_EVENT);
	}
	_callbackMutex.unlock();
#ifdef _DEBUG
	_timer.Stop();
	StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLM 3D calculation time: %d ms", static_cast<int>(_timer.ElapsedMilliseconds()));
	LogMessage(_errMsg, INFORMATION_EVENT);
#endif
	return TRUE;
}

//Map from image array to SLM or DMD dimension
unsigned char* ThorSLM::MapImageHologram(const wchar_t* pathAndFilename, PBITMAPINFO pbmi)
{
	//test load of bmp
	unsigned char* imgRead = LoadBMP(&pbmi->bmiHeader, pathAndFilename);
	if (NULL == imgRead)
		return NULL;

	//map to power of 2 square image, then crop later
	//consider case with image size larger than SLM
	_power2Px = static_cast<long>(max(max(pow(2, ceil(log2(_pixelSize[0]))), pow(2, ceil(log2(_pixelSize[1])))),
		max(pow(2, ceil(log2(pbmi->bmiHeader.biWidth))), pow(2, ceil(log2(pbmi->bmiHeader.biHeight))))));

	//return if the same size
	if (!_doHologram || _power2Px == pbmi->bmiHeader.biWidth && _power2Px == pbmi->bmiHeader.biHeight)
	{
		_pixelX = pbmi->bmiHeader.biWidth;
		_pixelY = pbmi->bmiHeader.biHeight;
		return imgRead;
	}

	//convert buffer
	long newSize;
	unsigned char* imgRGB = ConvertBGRToRGBBuffer(imgRead, pbmi->bmiHeader, &newSize);
	double pixelUM = (0 >= pbmi->bmiHeader.biXPelsPerMeter) ? 1.0 : (double)Constants::M_TO_UM / pbmi->bmiHeader.biXPelsPerMeter; //[um]

	//--- START mapping to maximum image size for hologram calculation ---//	
	//               power of 2 square               //
	//          ___________________________          //
	//          |                         |          //
	//          |      Image Width        |          //
	//          |   __________________    |          //
	//          |   |                |    |          //
	//          |   |Image Height    |    |power of 2 square //
	//          |   |                |    |          //
	//          |   |________________|    |          //
	//          |                         |          //
	//          |                         |          //
	//          |_________________________|          //
	long channelCnt = static_cast<long>(pbmi->bmiHeader.biBitCount / CHAR_BIT);
	size_t imgMapRGBBufSize = static_cast<long>(pow(_power2Px, (long)2)) * channelCnt;
	BYTE* imgMapRGB = new BYTE[imgMapRGBBufSize];
	memset(imgMapRGB, 0x0, imgMapRGBBufSize * sizeof(BYTE));

	//copy to buffer, vector: center of source image -> center of power of 2 square
	double vecX = static_cast<int>(floor(_power2Px / (double)2)) - static_cast<int>(floor(pbmi->bmiHeader.biWidth / (double)2));
	double vecY = static_cast<int>(floor(_power2Px / (double)2)) - static_cast<int>(floor(pbmi->bmiHeader.biHeight / (double)2));

	BYTE* pSrc = imgRGB;
	BYTE* pDst = imgMapRGB;
	vector<pair<int, int>> target;	//target coordinates X-first,Y-second
	for (long j = 0; j < pbmi->bmiHeader.biHeight; j++)
	{
		for (long i = 0; i < channelCnt * pbmi->bmiHeader.biWidth; i += channelCnt)
		{
			if (0 < (*pSrc))
			{
				int sourceX = (i - (i % channelCnt)) / channelCnt;
				int sourceY = j;
				target.clear();
				target.push_back(pair<int, int>(static_cast<int>(floor(sourceX + vecX)), static_cast<int>(floor(sourceY + vecY))));
				//avoid filling if single dot
				if ((j > 1 && i > channelCnt && 0 < *(imgRGB + ((j - 1) * pbmi->bmiHeader.biWidth * channelCnt + (i - channelCnt))))
					|| (0 < *(imgRGB + (j * pbmi->bmiHeader.biWidth * channelCnt + (i - channelCnt))))
					|| (0 < *(imgRGB + ((j + 1) * pbmi->bmiHeader.biWidth * channelCnt + (i - channelCnt))))
					|| (j > 1 && 0 < *(imgRGB + ((j - 1) * pbmi->bmiHeader.biWidth * channelCnt + i)))
					|| (0 < *(imgRGB + ((j + 1) * pbmi->bmiHeader.biWidth * channelCnt + i)))
					|| (j > 1 && 0 < *(imgRGB + ((j - 1) * pbmi->bmiHeader.biWidth * channelCnt + (i + channelCnt))))
					|| 0 < *(imgRGB + (j * pbmi->bmiHeader.biWidth * channelCnt + (i + channelCnt)))
					|| 0 < *(imgRGB + ((j + 1) * pbmi->bmiHeader.biWidth * channelCnt + (i + channelCnt)))
					)
				{
					target.push_back(pair<int, int>(static_cast<int>(ceil(sourceX + vecX)), static_cast<int>(ceil(sourceY + vecY))));
					target.push_back(pair<int, int>(static_cast<int>(floor(sourceX + vecX)), static_cast<int>(ceil(sourceY + vecY))));
					target.push_back(pair<int, int>(static_cast<int>(ceil(sourceX + vecX)), static_cast<int>(floor(sourceY + vecY))));
				}
				for (int i = 0; i < target.size(); i++)
				{
					//map from image to slm by overlapping center locations, validate data range
					if (imgMapRGBBufSize >= (target[i].second * _power2Px + target[i].first) * channelCnt + 2 &&
						0 <= (target[i].second * _power2Px + target[i].first))
					{
						for (int k = 0; k < channelCnt; k++)
						{
							*(pDst + (target[i].second * _power2Px + target[i].first) * channelCnt + k) = *(pSrc + k);
						}
					}
				}
			}
			pSrc += channelCnt;
		}
	}
	SAFE_DELETE_ARRAY(imgRGB);
	SAFE_DELETE_ARRAY(imgRead);

	//******************************************************************************//
	//set pixel size to be SLM dimension and bmi header to hologram power of 2,		//
	//we will crop from hologram power of 2 to SLM dimension at ReadAndScaleBitmap	//
	//******************************************************************************//
	//keep image size for later use for holoGen
	_pixelX = pbmi->bmiHeader.biWidth;
	_pixelY = pbmi->bmiHeader.biHeight;
	pbmi->bmiHeader.biWidth = pbmi->bmiHeader.biHeight = _power2Px;
	pbmi->bmiHeader.biSizeImage = static_cast<int>(pow(_power2Px, 2)) * channelCnt + 2;
	pbmi->bmiHeader.biXPelsPerMeter = static_cast<LONG>((double)Constants::M_TO_UM / pixelUM);
	pbmi->bmiHeader.biYPelsPerMeter = pbmi->bmiHeader.biXPelsPerMeter;

	//return after convert back
	unsigned char* bufRead = ConvertRGBToBGRBuffer(imgMapRGB, pbmi->bmiHeader, &newSize);
	SAFE_DELETE_ARRAY(imgMapRGB);
	return bufRead;
}

//Get calibrated image after mapping according to SLM or DMD dimension
unsigned char* ThorSLM::MapCalibrateHologram(const wchar_t* pathAndFilename, MemoryStruct<float>* pbuf)
{
	BITMAPINFO tbmi;
	unsigned char* imgRead = MapImageHologram(pathAndFilename, &tbmi);
	if (NULL == imgRead)
		return NULL;

	//convert buffer:
	pbuf->SetInfo(tbmi);
	long newSize;
	unsigned char* imgRGB = ConvertBGRToRGBBuffer(imgRead, pbuf->GetInfo().bmiHeader, &newSize);

	//return if no need to apply hologram & transform, [LoadPhaseDirect or DMD]
	if (!_doHologram)
	{
		SAFE_DELETE_ARRAY(imgRead);
		return imgRGB;
	}

	//apply affine & generate phase, hologram size should be power of 2:
	holoGen->SetSize(pbuf->GetInfo().bmiHeader.biWidth, pbuf->GetInfo().bmiHeader.biHeight, (double)Constants::M_TO_UM / pbuf->GetInfo().bmiHeader.biXPelsPerMeter);
	if (_skipFitting)
	{
		if (_slm3D)
		{
			double coeffs[HOMOGENEOUS_COEFF_CNT] = { 0.0 };
			coeffs[0] = coeffs[5] = coeffs[10] = coeffs[15] = 1.0;
			holoGen->SetCoeffs(GeoFittingAlg::HOMOGENEOUS, coeffs);
		}
		else
		{
			double coeffs[PROJECT_COEFF_CNT] = { 0.0 };
			coeffs[0] = coeffs[4] = 1.0;
			holoGen->SetCoeffs(GeoFittingAlg::PROJECTIVE, coeffs);
		}
	}
	else
	{
		if (_slm3D)
		{
			holoGen->SetCoeffs(GeoFittingAlg::HOMOGENEOUS, _fitCoeff3D[_selectWavelength]);
		}
		else
		{
			holoGen->SetCoeffs(GeoFittingAlg::PROJECTIVE, _fitCoeff[_selectWavelength]);
		}
	}
	holoGen->SetAlgorithm(HoloGenAlg::GerchbergSaxton);

	//copy to reset buffer:
	pbuf->CallocMemChk(pbuf->GetInfo().bmiHeader.biWidth * pbuf->GetInfo().bmiHeader.biHeight);
	unsigned char* pSrc = imgRGB;
	float* pDst = pbuf->GetMem();
	for (int i = 0; i < pbuf->GetInfo().bmiHeader.biWidth * pbuf->GetInfo().bmiHeader.biHeight; i++)
	{
		*pDst = *pSrc;
		pDst++;
		pSrc++;
	}
	SAFE_DELETE_ARRAY(imgRGB);

	//pre-offset, flip, scale or rotate before affine:
	if ((0 != _offsetPixels[_selectWavelength][0]) || (0 != _offsetPixels[_selectWavelength][1]))
	{
		holoGen->OffsetByPixels(pbuf->GetMem(), _offsetPixels[_selectWavelength][0], _offsetPixels[_selectWavelength][1]);
	}
	if (_verticalFlip[_selectWavelength])
	{
		holoGen->VerticalFlip(pbuf->GetMem());
	}
	if ((0.0 < _scaleFactor[_selectWavelength][0]) && (0.0 < _scaleFactor[_selectWavelength][1]) &&
		((1.0 != _scaleFactor[_selectWavelength][0]) || (1.0 != _scaleFactor[_selectWavelength][1])))
	{
		holoGen->ScaleByFactor(pbuf->GetMem(), _scaleFactor[_selectWavelength][0], _scaleFactor[_selectWavelength][1]);
	}
	if (0.0 != _rotateAngle[_selectWavelength])
	{
		holoGen->RotateForAngle(pbuf->GetMem(), _rotateAngle[_selectWavelength]);
	}
	//it will do fitting when generating 3D hologram
	if (!_slm3D)
		holoGen->FittingTransform(pbuf->GetMem());

	return imgRead;
}

//Crop hologram array to SLM or DMD dimension, BITMAPINFO will be updated before return
unsigned char* ThorSLM::CropHologramBMP(unsigned char* pMem, float* pS, BITMAPINFO& bmi)
{
	int channelCount = bmi.bmiHeader.biBitCount / CHAR_BIT;

	if (NULL == pMem)
		pMem = new BYTE[(size_t)bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight * channelCount];

	//copy back from buffer, expect unit already in pixel byte[0-255]
	//consider phase scaling per wavelength and scale for LUT if necessary:
	float* pSrc = pS;
	BYTE* pDst = pMem;
	for (int i = 0; i < bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight; i++)
	{
		for (int j = 0; j < channelCount; j++)
		{
			*pDst = static_cast<unsigned char>(((MAX_ARRAY_CNT - 1) < (*pSrc) ? (*pSrc) * (MAX_ARRAY_CNT - 1) / (LUT_SIZE - 1) : *pSrc) * _phaseMax[_selectWavelength] / UCHAR_MAX);
			pDst++;
		}
		pSrc++;
	}

	//crop from power of 2 side square to SLM dimension
	//***** power of 2 square *****//
	// ___________________________ //
	// |                         | //
	// |       SLM Width         | //
	// |   __________________    | //
	// |   |                |    | //
	// |   |SLM Height      |    |power of 2 square //
	// |   |                |    | //
	// |   |________________|    | //
	// |                         | //
	// |                         | //
	// |_________________________| //
	// set to SLM dimension (_pixelX x _pixelY)
	_pixelX = _pixelSize[0];
	_pixelY = _pixelSize[1];
	if (_pixelX == bmi.bmiHeader.biWidth && _pixelY == bmi.bmiHeader.biHeight)
	{
		return pMem;
	}
	else
	{
		int channelCnt = static_cast<int>(bmi.bmiHeader.biBitCount / CHAR_BIT);
		size_t imgMapRGBBufSize = (size_t)_pixelX * _pixelY * channelCnt;
		BYTE* imgMapRGB = new BYTE[imgMapRGBBufSize];
		memset(imgMapRGB, 0x0, imgMapRGBBufSize * sizeof(BYTE));

		int sourceCx = static_cast<int>(floor(bmi.bmiHeader.biWidth / 2));
		int sourceCy = static_cast<int>(floor(bmi.bmiHeader.biHeight / 2));
		int targetCx = static_cast<int>(floor(_pixelX / 2));
		int targetCy = static_cast<int>(floor(_pixelY / 2));
		int vecX = abs(sourceCx - targetCx);
		int vecY = abs(sourceCy - targetCy);

		BYTE* pSrc = pMem + static_cast<int>(channelCnt * ((int)bmi.bmiHeader.biWidth * vecY + vecX));
		BYTE* pDst = imgMapRGB;
		for (int j = vecY; j < (vecY + _pixelY); j++)
		{
			for (int i = vecX; i < (vecX + _pixelX) * channelCnt; i += channelCnt)
			{
				for (int c = 0; c < channelCnt; c++)
				{
					*pDst = *pSrc;
					pSrc++;
					pDst++;
				}
			}
			pSrc += ((int)2 * channelCnt * vecX);
		}
		SAFE_DELETE_ARRAY(pMem);

		//set bmi header to SLM dimension
		bmi.bmiHeader.biWidth = _pixelX;
		bmi.bmiHeader.biHeight = _pixelY;
		bmi.bmiHeader.biSizeImage = _pixelX * _pixelY * channelCnt + 2;
		return imgMapRGB;
	}
}

unsigned char* ThorSLM::GetAndProcessBMP(BITMAPINFO& bmi)
{
	const int ITERATIONS_2D = 10;

	unsigned char* imgRead = MapCalibrateHologram(_bmpPathAndName.c_str(), &_lastHoloBuf[0]);
	if (NULL == imgRead)
		return NULL;

	//return if no need to apply hologram & transform, [LoadPhaseDirect or DMD]
	bmi = _lastHoloBuf[0].GetInfo();
	if (!_doHologram)
		return imgRead;

	holoGen->GenerateHologram(_lastHoloBuf[0].GetMem(), ITERATIONS_2D, static_cast<int>(ceil(max(sqrt(pow(_pixelX, 2) + pow(_pixelY, 2)) / 2 * max(0.0, min(1.0, _flatDiagRatio)), 0))), _flatPowerRange[0], _flatPowerRange[1], 0);	//183 for default 512x512, image dimension (_pixelX,_pixelY)

	//keep [1]:on focus, [0]: to be defocused
	CopyDefocus(0, 1);

	DefocusNormalizeHologram(TRUE);

	return CropHologramBMP(imgRead, _lastHoloBuf[0].GetMem(), bmi);
}

void ThorSLM::SetDefault()
{
	_pixelX = DEFAULT_PIXEL_X;
	_pixelY = DEFAULT_PIXEL_Y;
	_errMsg[0] = 0;
	_pSlmName = "";
	_bmpPathAndName = _seqPathAndName = L"";
	_bufferCount = 1;
	_arrayOrFileID = 0;
	_overDrive = 0;
	_lastHoloBuf[0].ReallocMemChk(0);
	_lastHoloBuf[1].ReallocMemChk(0);
	_lastBmpBuf.ReallocMemChk(0);
	_slmSeqVec.clear();
}

//<summary> reset pattern sequence in normal or custom order </summary>
long ThorSLM::ResetSequence(wchar_t* filename)
{
	long ret = TRUE;
	std::wstring seqName = std::wstring(filename);

	_slmSeqVec.clear();

	if (0 == seqName.size())
	{
		//normal sequence
		for (unsigned int i = 0; i < static_cast<unsigned int>(_bufferCount); i++)
			_slmSeqVec.push_back(i);
	}
	else
	{
		try
		{
			//custom sequence
			FILE* sequenceFile = NULL;
			if (0 == fopen_s(&sequenceFile, ConvertWStringToString(seqName).c_str(), "r"))
			{
				//determine total count
				char line[_MAX_PATH];
				unsigned int iVal = 0, maxID = 0;
				while (NULL != fgets(line, sizeof(line), sequenceFile) && 0 != strcmp(line, "\n") && 0 != atoi(line))
				{
					maxID = max(maxID, static_cast<unsigned int>(atoi(line)));	//pattern ID is 1-based
					_slmSeqVec.push_back(atoi(line) - 1);						//array ID is 0-based
				}

				//validate sequence, pattern ID should be below maximum count
				if (maxID > static_cast<unsigned int>(_bufferCount))
					_slmSeqVec.clear();

				fclose(sequenceFile);
			}
		}
		catch (...)
		{
			_slmSeqVec.clear();
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLM:%hs@%u: ResetSequence failed: %s", __FILE__, __LINE__, seqName.c_str());
			LogMessage(_errMsg, ERROR_EVENT);
			return FALSE;
		}
	}

	ret = (0 >= _slmSeqVec.size()) ? FALSE : TRUE;

	//re-start from the first
	_arrayOrFileID = 0;
	return ret;
}

long ThorSLM::SetupHWTriggerIn()
{
	int32 retVal = 0, error = 0;
	if ((0 < _counterLine.size()) && (string::npos != _counterLine.find("ctr")) && (0 < _hwTriggerInput.size()))
	{
		if (_taskHandleCI)
		{
			retVal = DAQmxStopTask(_taskHandleCI);
			retVal = DAQmxClearTask(_taskHandleCI);
			_taskHandleCI = NULL;
		}
		DAQmxErrChk(L"DAQmxCreateTask", retVal = DAQmxCreateTask("", &_taskHandleCI));
		DAQmxErrChk(L"DAQmxCreateCICountEdgesChan", retVal = DAQmxCreateCICountEdgesChan(_taskHandleCI, _counterLine.c_str(), "", DAQmx_Val_Rising, 0, DAQmx_Val_CountUp));
		DAQmxErrChk(L"DAQmxSetCICountEdgesTerm", retVal = DAQmxSetCICountEdgesTerm(_taskHandleCI, _counterLine.c_str(), _hwTriggerInput.c_str()));
		DAQmxErrChk(L"DAQmxCfgSampClkTiming", retVal = DAQmxCfgSampClkTiming(_taskHandleCI, _hwTriggerInput.c_str(), 1000, DAQmx_Val_Rising, DAQmx_Val_HWTimedSinglePoint, 0));
		DAQmxErrChk(L"DAQmxRegisterSignalEvent", retVal = DAQmxRegisterSignalEvent(_taskHandleCI, DAQmx_Val_SampleClock, 0, ThorSLM::HWTriggerCallback, NULL));
		DAQmxErrChk(L"DAQmxStartTask", retVal = DAQmxStartTask(_taskHandleCI));
	}
	return retVal;
}
