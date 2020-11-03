// ThorSLMPDM512.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ThorSLMPDM512.h"
#include "ThorSLMPDM512SetupXML.h"
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
unique_ptr<ThorSLMPDM512XML> pSetup(new ThorSLMPDM512XML());
float* ThorSLM::_fpPointsXY[MAX_ARRAY_CNT] = {NULL};
long ThorSLM::_arrayOrFileID = 0;		//0-based buffer or sequence index
long ThorSLM::_bufferCount = 1;			//1-based total buffer size
long ThorSLM::_slmRuntimeCalculate = 0;	//[1]:calculate transient frames at runtime
unique_ptr<WinDVIDLL> winDVI(new WinDVIDLL(L".\\WinDVI.dll"));
unique_ptr<HoloGenDLL> holoGen(new HoloGenDLL(L".\\HologramGenerator.dll"));
TaskHandle ThorSLM::_taskHandleCI = NULL;
wchar_t message[_MAX_PATH];
MemoryStruct ThorSLM::_intermediateBuf[MAX_ARRAY_CNT] = {NULL, 0};	//intermediate buffers for transient or bmp
Blink_SDK* ThorSLM::_blinkSDK = NULL;
unsigned int ThorSLM::_slmTimeout = SLM_TIMEOUT_MIN;
MemoryStruct ThorSLM::_slmTempBuf;		//tempearary buffer for transient calculation
std::vector<unsigned int> ThorSLM::_slmSeqVec;	//sequence of pattern order, normal[0,1,2... & size == _bufferCount] if not doing runtime calculation
mutex ThorSLM::_callbackMutex;			///<mutex lock to update static params used in callback

ThorSLM::ThorSLM() :
	DEFAULT_PIXEL_X(512),
	DEFAULT_PIXEL_Y(512),
	DEFAULT_TRUE_FRAMES(5),
	DEFAULT_TRANSIENT_FRAMES(10U),
	MAX_TRANSIENT_FRAMES(20U)
{
	_fitCoeff = new double[PROJECT_COEFF_CNT];
	_deviceCount = 0;
	_deviceDetected = FALSE;
	_fileSettingsLoaded = FALSE;
	_pixelRange = new long[4];	//minX, maxX, minY, maxY
	_fpPointsXYSize = new long[MAX_ARRAY_CNT];
	_tableLUT = new unsigned short[65536];
	_imgWavefront = NULL;
	_slmTempBuf.memPtr = NULL;
	_firstBuf.memPtr = NULL;
	SetDefault();
	_blinkSDK = NULL;
	_transientFrames = DEFAULT_TRANSIENT_FRAMES;
	_slm3D = FALSE;
}

ThorSLM* ThorSLM::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorSLM());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

ThorSLM::~ThorSLM()
{
	_instanceFlag = false;

	if(NULL != _fitCoeff)
	{
		delete[] _fitCoeff;
		_fitCoeff = NULL;
	}
	if(NULL != _pixelRange)
	{
		delete[] _pixelRange;
		_pixelRange = NULL;
	}

	ReleaseMem();
	if(NULL != _fpPointsXYSize)
	{
		delete[] _fpPointsXYSize;
		_fpPointsXYSize = NULL;
	}

	delete[] _tableLUT;
	_tableLUT = NULL;
}

long ThorSLM::FindDevices(long &deviceCount)
{
	_deviceCount = 1;
	_deviceDetected = TRUE;
	try
	{
		if(FALSE == pSetup->GetSpec(_pSlmName,_overDrive,_transientFrames,_pixelRange[0],_pixelRange[1],_pixelRange[2],_pixelRange[3], _lutFile, _overDrivelutFile, _wavefrontFile))
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"GetSpec from ThorSLMPDM512Settings failed.");
			LogMessage(_errMsg,ERROR_EVENT);
			return 0;
		}

		if(NULL != _blinkSDK)
		{
			delete _blinkSDK;
			_blinkSDK = NULL;
		}

		//overdrive will be using Blink_SDK dll:
		if((_overDrive) && (0 == _pSlmName.compare("PDM512")))
		{
			if(0 < _overDrivelutFile.length())
			{
				const unsigned int bits_per_pixel  = 8U;
				//const unsigned int pixel_dimension = 512U;
				const bool         is_nematic_type      = true;
				const bool         RAM_write_enable     = true;
				const bool         use_GPU_if_available = true;

				unsigned int n_boards_found   = 0U;
				bool         constructed_okay = true;		


				_transientFrames = ((_transientFrames < MAX_TRANSIENT_FRAMES) && (0 < _transientFrames)) ? _transientFrames : DEFAULT_TRANSIENT_FRAMES;

				_blinkSDK = new Blink_SDK(bits_per_pixel, &n_boards_found, &constructed_okay, is_nematic_type, RAM_write_enable, use_GPU_if_available, static_cast<unsigned __int64>(_transientFrames), _overDrivelutFile.c_str());

				if(n_boards_found && constructed_okay)
				{
					//check hardware:
					if(_blinkSDK->Is_overdrive_available() && _blinkSDK->Is_slm_transient_constructed())
					{
						_deviceCount = n_boards_found;
						_blinkSDK->Set_true_frames(DEFAULT_TRUE_FRAMES);
						_blinkSDK->SLM_power(1, true);
						if(0 < _lutFile.length())
						{
							_blinkSDK->Load_LUT_file(1, _lutFile.c_str());
						}
						else
						{
							_blinkSDK->Load_linear_LUT(1);
						}
					}
					else
					{
						_deviceCount = 0;
						_deviceDetected = FALSE;
						StringCbPrintfW(_errMsg,MSG_SIZE,L"Found SLM OD %d, constructed: %d, overdrive available: %d transient constructed: %d\nPlease check your SLM overdrive LUT path and settings.",n_boards_found,constructed_okay,_blinkSDK->Is_overdrive_available(),_blinkSDK->Is_slm_transient_constructed());
						MessageBox(NULL,_errMsg,L"Find SLM Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
					}
				}
				else
				{
					_deviceCount = 0;
					_deviceDetected = FALSE;
					StringCbPrintfW(_errMsg,MSG_SIZE,L"Found SLM OD %d, constructed: %d\nPlease check your SLM hardware.",n_boards_found,constructed_okay);
					MessageBox(NULL,_errMsg,L"Find SLM Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
				}
			}
		}
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorSLMPDM512 FindDevices failed.");
		LogMessage(_errMsg,ERROR_EVENT);
	}
	deviceCount = _deviceCount;
	return deviceCount;
}

long ThorSLM::SelectDevice(const long device)
{
	long ret = TRUE;

	if(!_fileSettingsLoaded)
	{
		//XML settings retrieval functions will throw an exception if tags or attributes are missing
		//catch each independetly so that as many tags as possible can be read
		try
		{
			if(FALSE == pSetup->GetPostTransform(_verticalFlip, _rotateAngle, _scaleFactor[0], _scaleFactor[1], _offsetPixels[0], _offsetPixels[1]))
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"GetPostTransform from ThorSLMPDM512Settings failed.");
				LogMessage(_errMsg,ERROR_EVENT);
			}
			if(FALSE == pSetup->GetCalibration(_fitCoeff[0],_fitCoeff[1],_fitCoeff[2],_fitCoeff[3],_fitCoeff[4],_fitCoeff[5],_fitCoeff[6],_fitCoeff[7]))
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"GetCalibration from ThorSLMPDM512Settings failed.");
				LogMessage(_errMsg,ERROR_EVENT);
			}
			if(FALSE == pSetup->GetSpec(_pSlmName,_overDrive,_transientFrames,_pixelRange[0],_pixelRange[1],_pixelRange[2],_pixelRange[3], _lutFile, _overDrivelutFile, _wavefrontFile))
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"GetSpec from ThorSLMPDM512Settings failed.");
				LogMessage(_errMsg,ERROR_EVENT);
			}
			if(FALSE == pSetup->GetTrigger(_counterLine, _hwTriggerInput))
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"GetTrigger from ThorSLMPDM512Settings failed.");
				LogMessage(_errMsg,ERROR_EVENT);
			}

			//load LUT and calibration:
			ReadLUTFile(ConvertStringToWString(_lutFile));

			if((!_overDrive) || (0 != _pSlmName.compare("PDM512")))
			{
				ReadWavefrontFile(ConvertStringToWString(_wavefrontFile));
			}
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SelectCamera from ThorSLM failed");
			LogMessage(_errMsg,ERROR_EVENT);
		}

		_fileSettingsLoaded = TRUE;

	}

	return ret;
}

long ThorSLM::TeardownDevice()
{
	try
	{
		if((_overDrive) && (0 == _pSlmName.compare("PDM512")) && (NULL != _blinkSDK))
		{
			_blinkSDK->SLM_power(1, false);
		}
		else
		{
			ReleaseDVI();
		}
		SetDefault();

		//reset flag:
		_fileSettingsLoaded = FALSE;
	}
	catch(...)
	{
	}

	ReleaseMem();
	_deviceDetected = FALSE;
	return TRUE;
}

long ThorSLM::GetLastErrorMsg(wchar_t * msg, long size)
{
	wcsncpy_s(msg,size,_errMsg,MSG_SIZE);

	//reset the error message
	_errMsg[0] = 0;
	return TRUE;
}

long ThorSLM::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
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
			paramMax = MAX_ARRAY_CNT - 1;
			paramDefault = 0;
			paramReadOnly = FALSE;
		}
		break;		
	case IDevice::PARAM_SLM_PIXEL_X:
		{
			paramType = IDevice::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _pixelRange[0];
			paramMax = _pixelRange[1];
			paramDefault = DEFAULT_PIXEL_X;
			paramReadOnly = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_PIXEL_Y:
		{
			paramType = IDevice::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = _pixelRange[2];
			paramMax = _pixelRange[3];
			paramDefault = DEFAULT_PIXEL_Y;
			paramReadOnly = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_CALIB_Z:
		{
			paramType = IDevice::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = -2;
			paramMax = 2;
			paramDefault = 0;
			paramReadOnly = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_NA:
		{
			paramType = IDevice::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 10;
			paramDefault = 1;
			paramReadOnly = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_WAVELENGTH:
		{
			paramType = IDevice::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramMin = 0;
			paramMax = 2000;
			paramDefault = 1040;
			paramReadOnly = TRUE;
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
	long ret = FALSE;

	switch (paramID)
	{
	case IDevice::PARAM_SLM_FUNC_MODE:
		if ((param >= SLMFunctionMode::LOAD_PHASE_ONLY) && (param < SLMFunctionMode::LAST_FUNCTION))
		{
			_slmFuncMode = static_cast<long> (param);
			ret = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_ARRAY_ID:
		if ((param >= 0) && (param < MAX_ARRAY_CNT))
		{
			_arrayOrFileID = static_cast<long> (param);
			_bufferCount = (_bufferCount < (_arrayOrFileID+1)) ? (_arrayOrFileID+1) : _bufferCount;
			ret = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_PIXEL_X:
		if ((param >= _pixelRange[0]) && (param <= _pixelRange[1]))
		{
			_pixelX = static_cast<long> (param);
			ret = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_PIXEL_Y:
		if ((param >= _pixelRange[2]) && (param <= _pixelRange[3]))
		{
			_pixelY = static_cast<long> (param);
			ret = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_CALIB_Z:
		_calibZ = param;
		ret = TRUE;
		break;
	case IDevice::PARAM_SLM_NA:
		if (param > 0)
		{
			_na = param;
			ret = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_WAVELENGTH:
		if (param > 0)
		{
			_wavelength = static_cast<double> (param);
			ret = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_3D:
		if ((param >= FALSE) && (param <= TRUE))
		{
			_slm3D = static_cast<long> (param);
			ret = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_RESET_AFFINE:
		if ((param >= FALSE) && (param <= TRUE))
		{
			if(static_cast<long> (param))
			{
				//reset affine coefficients:
				_fitCoeff[0] = 1; _fitCoeff[1] = 0; _fitCoeff[2] = 0; 
				_fitCoeff[3] = 0; _fitCoeff[4] = 1; _fitCoeff[5] = 0; 
				_fitCoeff[6] = 0; _fitCoeff[7] = 0; 

				//reset post affine transform params:
				_rotateAngle = 0.0;
				_scaleFactor[0] = _scaleFactor[1] = 1.0;
				_verticalFlip = _offsetPixels[0] = _offsetPixels[1] = 0;
				PersistAffineValues();
			}
			ret = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_BLANK:
		if ((param >= FALSE) && (param <= TRUE))
		{
			if(static_cast<long> (param))
			{
				//set blank image:
				size_t imgSize = _pixelX * _pixelY * RGB_CNT* sizeof(unsigned char);
				unsigned char* pImg = (unsigned char*)malloc(imgSize);
				memset(pImg, 0, imgSize);

				if((_overDrive) && (0 == _pSlmName.compare("PDM512")) && (NULL != _blinkSDK))
				{
					_blinkSDK->Write_overdrive_image(1, pImg, false, false);
				}
				else
				{
					BITMAPINFO bmi;
					bmi.bmiHeader.biSize = 40;
					bmi.bmiHeader.biWidth = _pixelX;
					bmi.bmiHeader.biHeight = _pixelY;
					bmi.bmiHeader.biPlanes = 1;
					bmi.bmiHeader.biBitCount = CHAR_BIT * RGB_CNT;
					bmi.bmiHeader.biSizeImage = (DWORD)imgSize;
					bmi.bmiHeader.biCompression = 0;
					bmi.bmiHeader.biXPelsPerMeter = 3780;
					bmi.bmiHeader.biYPelsPerMeter = 3780;
					bmi.bmiHeader.biClrUsed = 256;
					bmi.bmiHeader.biClrImportant = 256;
					winDVI->EditBMP(0, pImg, bmi);
					winDVI->CreateDVIWindow(_pixelX, _pixelY);
					winDVI->DisplayBMP(0);
				}

				free(pImg);
			}
			ret = TRUE;
		}
		break;
	case IDevice::PARAM_SLM_TIMEOUT:
		_slmTimeout = (param >= SLM_TIMEOUT_MIN) ? static_cast<unsigned int>(ceil(param/SLM_TIMEOUT_MIN)*SLM_TIMEOUT_MIN) : SLM_TIMEOUT_MIN;
		ret = TRUE;
		break;
	case IDevice::PARAM_SLM_RUNTIME_CALC:
		if ((param >= FALSE) && (param <= TRUE))
		{
			_slmRuntimeCalculate = static_cast<long>(param);
			ret = TRUE;
		}
		break;
	default:
		StringCbPrintfW(_errMsg,MSG_SIZE,L"SLM Parameter (%d) not implemented",paramID);
		LogMessage(_errMsg,ERROR_EVENT);
		break;
	}

	return ret;
}

long ThorSLM::SetParamBuffer(const long paramID, char * pBuffer, long size)
{
	long ret = TRUE;

	switch (paramID)
	{
	case IDevice::PARAM_SLM_POINTS_ARRAY:
		_fpPointsXY[_arrayOrFileID] = (float *) realloc((void*)_fpPointsXY[_arrayOrFileID], size * sizeof(float));
		if(NULL == _fpPointsXY[_arrayOrFileID])
			return FALSE;

		//Expect this waveform include X and Y (interleaved):
		std::memcpy((void*)_fpPointsXY[_arrayOrFileID], pBuffer, size * sizeof(float));
		_fpPointsXYSize[_arrayOrFileID] = size;
		break;
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

long ThorSLM::SetParamString(const long paramID, wchar_t * str)
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
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorSLM cannot set sequence file while not in PARAM_SLM_RUNTIME_CALC mode.");
			LogMessage(_errMsg,ERROR_EVENT);
			return FALSE;
		}

		//load sequence file
		_callbackMutex.lock();
		ret = ResetSequence(str);
		_seqPathAndName = (TRUE == ret) ? L"" : std::wstring(str);
		if (FALSE == ret)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorSLM unable to load sequence file.");
			LogMessage(_errMsg,ERROR_EVENT);
			_callbackMutex.unlock();
			return FALSE;
		}

		//update first bmp
		if((_overDrive) && (0 == _pSlmName.compare("PDM512")) && (NULL != _blinkSDK))
		{
			_blinkSDK->Write_overdrive_image(1, _intermediateBuf[_slmSeqVec.at(_arrayOrFileID)].memPtr, false, false);
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

long ThorSLM::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch (paramID)
	{
	case IDevice::PARAM_DEVICE_TYPE:
		param = IDevice::SLM;
		break;
	case IDevice::PARAM_CONNECTION_STATUS:
		param = (_deviceDetected) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
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
	case IDevice::PARAM_SLM_3D:
		param = _slm3D;
		break;
	case IDevice::PARAM_SLM_RESET_AFFINE:
		param = FALSE;
		break;
	case IDevice::PARAM_SLM_BLANK:
		param = FALSE;
		break;
	case IDevice::PARAM_SLM_TIMEOUT:
		param = _slmTimeout;
		break;
	case IDevice::PARAM_SLM_RUNTIME_CALC:
		param = _slmRuntimeCalculate;
		break;
	default:
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) not implemented",paramID);
		LogMessage(_errMsg,ERROR_EVENT);
		ret = FALSE;
		break;
	}

	return ret;
}

long ThorSLM::GetParamBuffer(const long paramID, char * pBuffer, long size)
{
	long ret = TRUE;
	std::string str;

	switch (paramID)
	{
	case IDevice::PARAM_SLM_POINTS_ARRAY:
		size = _fpPointsXYSize[_arrayOrFileID];
		pBuffer = (char*)_fpPointsXY[_arrayOrFileID];
		break;
	default:
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) not implemented",paramID);
		LogMessage(_errMsg,ERROR_EVENT);
		break;
	}

	return ret;
}

long ThorSLM::GetParamString(const long paramID, wchar_t * str, long size)
{
	long ret = TRUE;

	switch (paramID)
	{
	case IDevice::PARAM_SLM_BMP_FILENAME:
		str = (wchar_t *)_bmpPathAndName.c_str();
		size = static_cast<long>(_bmpPathAndName.size());
		break;
	case IDevice::PARAM_SLM_SEQ_FILENAME:
		str = (wchar_t *)_seqPathAndName.c_str();
		size = static_cast<long>(_seqPathAndName.size());
		break;
	default:
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) not implemented",paramID);
		LogMessage(_errMsg,ERROR_EVENT);
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
			ret = LoadHologram();
			break;
		case SLMFunctionMode::PHASE_CALIBRATION:
			if((NULL != _fpPointsXY[0]) && (NULL != _fpPointsXY[1]) && (_fpPointsXYSize[0] == _fpPointsXYSize[1]))
			{
				bool doAffine = false;
				//not changing affine coeffs if the two point arrays are identical:
				for (int i = 0; i < _fpPointsXYSize[1]; i++)
				{
					if(_fpPointsXY[0][i] != _fpPointsXY[1][i])
					{
						doAffine = true;
						break;
					}
				}
				//affine transform from source to target: _fpPointsXY[0] is source, 
				//_fpPointsXY[1] is target with the same length:
				if(doAffine)
				{
					//flip, scale & rotate before affine coeffs calculation:
					if(_verticalFlip)
					{
						CoordinatesVerticalFlip(_fpPointsXY[0],_fpPointsXYSize[0]);
						CoordinatesVerticalFlip(_fpPointsXY[1],_fpPointsXYSize[1]);
					}
					if(((1.0 != _scaleFactor[0]) || (1.0 != _scaleFactor[1])) && ((0.0 < _scaleFactor[0]) && (0.0 < _scaleFactor[1])))
					{
						CoordinatesScale(_fpPointsXY[0],_fpPointsXYSize[0],_scaleFactor[0],_scaleFactor[1]);
						CoordinatesScale(_fpPointsXY[1],_fpPointsXYSize[1],_scaleFactor[0],_scaleFactor[1]);
					}
					if(0.0 != _rotateAngle)
					{
						CoordinatesRotate(_fpPointsXY[0],_fpPointsXYSize[0], _rotateAngle);
						CoordinatesRotate(_fpPointsXY[1],_fpPointsXYSize[1], _rotateAngle);
					}
					if(TRUE == holoGen->CalculateCoeffs(_fpPointsXY[0],_fpPointsXY[1],_fpPointsXYSize[1], GeoFittingAlg::PROJECTIVE, _fitCoeff))
					{
						PersistAffineValues();
					}
				}
			}

			//reset buffer count since only write one calibration frame:
			_bufferCount = 1;
			ret = LoadHologram();
			break;
		case SLMFunctionMode::SAVE_PHASE:
			ret = SaveHologram(true);
			break;
		default:
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetupSLM Failed: Invalid SLM Function Mode");
			LogMessage(_errMsg,VERBOSE_EVENT);
			ret = FALSE;
			break;
		}
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"SetupSLM failed.");
		LogMessage(_errMsg,ERROR_EVENT);
		ret = FALSE;
	}
	return ret;
}

long ThorSLM::StartPosition()
{
	long ret = TRUE;

	try
	{
		switch (_slmFuncMode)
		{
		case SLMFunctionMode::LOAD_PHASE_ONLY:
			//set normal sequence
			if (FALSE == ResetSequence())
				return FALSE;

			if((_overDrive) && (0 == _pSlmName.compare("PDM512")) && (NULL != _blinkSDK))
			{
				if(1 < _bufferCount)
				{
					ret = SetIntermediateBuffer(_firstBuf);

					SetupHWTriggerIn();
				}
				//write first bmp
				_blinkSDK->Write_overdrive_image(1, _firstBuf.memPtr, false, false);
			}
			else
			{
				winDVI->DisplayBMP(_slmSeqVec.at(_arrayOrFileID));

				SetupHWTriggerIn();
			}
			break;
		case SLMFunctionMode::PHASE_CALIBRATION:
			if((_overDrive) && (0 == _pSlmName.compare("PDM512")))
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
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Invalid SLM function mode for StartPosition");
			LogMessage(_errMsg,VERBOSE_EVENT);
			break;
		}
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"SLM Failed to StartAcquisition");
		LogMessage(_errMsg,ERROR_EVENT);
		ret = FALSE;
	}
	return ret;
}

long ThorSLM::StatusPosition(long &status)
{
	switch (_slmFuncMode)
	{
	case SLMFunctionMode::LOAD_PHASE_ONLY:
	case SLMFunctionMode::PHASE_CALIBRATION:
		if((_overDrive) && (0 == _pSlmName.compare("PDM512")))
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

long ThorSLM::ReadPosition(DeviceType deviceType, double &pos)
{
	return TRUE;
}

long ThorSLM::PostflightPosition()
{
	long ret = TRUE;

	switch (_slmFuncMode)
	{
	case SLMFunctionMode::LOAD_PHASE_ONLY:
		//terminate HW trigger task, and clear mem:
		CloseNITasks();
		_bufferCount = 1;
		if((_overDrive) && (0 == _pSlmName.compare("PDM512")))
		{
			_blinkSDK->Stop_sequence();
		}
		else
		{
			winDVI->ClearBMPs();
		}
		break;
	case SLMFunctionMode::PHASE_CALIBRATION:
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

int32 CVICALLBACK ThorSLM::HWTriggerCallback(TaskHandle taskHandle, int32 signalID, void *callbackData)
{
	_callbackMutex.lock();

	if (0 >= _slmSeqVec.size())
		goto ERROR_STATE;

	_arrayOrFileID = ((_arrayOrFileID + 1) % _slmSeqVec.size());	//circular index upto sequence count (== _bufferCount if not runtime calculation)

	if(NULL != _blinkSDK)
	{
		if (TRUE == _slmRuntimeCalculate)
		{
			//calculate transient frames then write:
			unsigned int tmpCount = 0U;
			if(false == _blinkSDK->Calculate_transient_frames(_intermediateBuf[_slmSeqVec.at(_arrayOrFileID)].memPtr, &tmpCount))
				goto ERROR_STATE;

			ReallocMemChk(_slmTempBuf, tmpCount*sizeof(unsigned char));
			if ((0 == tmpCount) || (false == _blinkSDK->Retrieve_transient_frames(_slmTempBuf.memPtr)))
				goto ERROR_STATE;

			_blinkSDK->Write_transient_frames(1, _slmTempBuf.memPtr, 0U, true, false, _slmTimeout);
		}
		else
		{
			//write pre-calculated transient frames:
			_blinkSDK->Write_transient_frames(1, _intermediateBuf[_slmSeqVec.at(_arrayOrFileID)].memPtr, 0U, true, false, _slmTimeout);
		}
	}
	else
	{
		winDVI->DisplayBMP(_slmSeqVec.at(_arrayOrFileID));
	}
	_callbackMutex.unlock();
	return 0;

ERROR_STATE:
	_callbackMutex.unlock();
	StringCbPrintfW(_errMsg,MSG_SIZE,L"HWTriggerCallback failed, could be unable to calculate transient frames.");
	LogMessage(_errMsg,ERROR_EVENT);
	return 0;
}

void ThorSLM::CloseNITasks()
{
	if(_taskHandleCI)
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
	if((NULL == ptArrays) || (0 != (size % 2)))
		return FALSE;

	//offset to center for rotation:
	for (long i = 0; i < (size/2); i++)
	{
		float x = ptArrays[2*i] - (_pixelX/2);
		float y = ptArrays[2*i+1] - (_pixelY/2);
		ptArrays[2*i] = static_cast<float>((x*cos(angleRad)) + (y*sin(angleRad))) + (_pixelX/2);
		ptArrays[2*i+1] = static_cast<float>((y*cos(angleRad)) - (x*sin(angleRad))) + (_pixelY/2);
	}
	return retVal;
}

//scale interleaved x, y coordinates from center
long ThorSLM::CoordinatesScale(float* ptArrays, long size, double scaleX, double scaleY)
{
	long retVal = TRUE;

	//expect x,y interleaved array with doubled size:
	if((NULL == ptArrays) || (0 != (size % 2)))
		return FALSE;

	//offset to center for scale:
	for (long i = 0; i < (size/2); i++)
	{
		float x = ptArrays[2*i] - (_pixelX/2);
		float y = ptArrays[2*i+1] - (_pixelY/2);
		ptArrays[2*i] = static_cast<float>(x*scaleX) + (_pixelX/2);
		ptArrays[2*i+1] = static_cast<float>(y*scaleY) + (_pixelY/2);
	}
	return retVal;
}

//vertical flip interleaved x, y coordinates
long ThorSLM::CoordinatesVerticalFlip(float* ptArrays, long size)
{
	long retVal = TRUE;

	//expect x,y interleaved array with doubled size:
	if((NULL == ptArrays) || (0 != (size % 2)))
		return FALSE;

	for (long i = 0; i < (size/2); i++)
	{
		float x = ptArrays[2*i];
		float y = ptArrays[2*i+1];
		ptArrays[2*i] = x;
		ptArrays[2*i+1] = _pixelY - y;
	}
	return retVal;
}

long ThorSLM::LoadHologram()
{
	if(0 == _pSlmName.compare("PDM512"))
	{
		ReadAndScaleBitmap(1);
	}
	else if(0 == _pSlmName.compare("EXULUS"))
	{
		ReadAndScaleBitmap(0);
	}
	return TRUE;
}

long ThorSLM::PersistAffineValues()
{
	long retVal = TRUE;
	//persist affine coefficients:
	if(FALSE == pSetup->SetCalibration(_fitCoeff[0],_fitCoeff[1],_fitCoeff[2],_fitCoeff[3],_fitCoeff[4],_fitCoeff[5],_fitCoeff[6],_fitCoeff[7]))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"SetCalibration from ThorSLMPDM512Settings failed");
		LogMessage(_errMsg,ERROR_EVENT);
		retVal = FALSE;
	}

	if(FALSE == pSetup->SetPostTransform(_verticalFlip, _rotateAngle, _scaleFactor[0], _scaleFactor[1], _offsetPixels[0], _offsetPixels[1]))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"SetPreTransform from ThorSLMPDM512Settings failed");
		LogMessage(_errMsg,ERROR_EVENT);
		retVal = FALSE;
	}

	//reset flag after files saved
	_fileSettingsLoaded = FALSE;
	return retVal;
}

BOOL ThorSLM::ReadLUTFile(std::wstring fileName)
{
	const int LUT_COLUMN_CNT = 2;
	FILE *stream;
	int seqnum, ReturnVal, tmpLUT;
	bool errorFlag=false;

	std::string fname = ConvertWStringToString(fileName);

	if(0 != fopen_s(&stream, fname.c_str(),"r"))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorSLMPDM512: open LUT file failed.");
		LogMessage(_errMsg, ERROR_EVENT);
	}

	//read LUT file or generate linear LUT:
	if ((stream != NULL) && (errorFlag == false))
	{
		for (int i=0; i<LUT_SIZE; i++)
		{
			ReturnVal=fscanf_s(stream, "%d %d", &seqnum, &tmpLUT); 
			if ((ReturnVal!=LUT_COLUMN_CNT) || (seqnum!=i) || (tmpLUT < 0) || (tmpLUT > (LUT_SIZE-1)))
			{
				errorFlag=true;

				//close the file we opened
				fclose(stream);
				break;
			}
			_tableLUT[i] = (unsigned short)tmpLUT;
		}

		//close the file we opened
		fclose(stream);
	}
	if ((stream == NULL)||(errorFlag == true))                    
	{
		//otherwise hardcode a linear LUT
		for (int i=0; i<LUT_SIZE; i++)
		{
			_tableLUT[i]=i;
		}
		return FALSE;
	}
	return TRUE;
}

BOOL ThorSLM::ReadWavefrontFile(std::wstring fileName)
{
	int imgWidth, imgHeight;
	long size, newSize;
	BITMAPINFO bmi;

	//clear memory:
	if(_imgWavefront)
	{
		delete[] _imgWavefront;
		_imgWavefront = NULL;
	}

	//load calibration file:
	if(0 < fileName.length())
	{		
		unsigned char* imgCalRead = LoadBMP(&imgWidth, &imgHeight, &size, &bmi.bmiHeader, fileName.c_str());
		if(NULL == imgCalRead)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorSLMPDM512 LoadWavefrontFile: load bmp file failed.");
			LogMessage(_errMsg, ERROR_EVENT);
			return FALSE;
		}
		//force calibration file to be color:
		if(RGB_CNT*CHAR_BIT != bmi.bmiHeader.biBitCount)
		{
			delete[] imgCalRead;
			return FALSE;
		}
		//convert buffer:
		_imgWavefront = ConvertBGRToRGBBuffer(imgCalRead, bmi.bmiHeader, &newSize);
		delete[] imgCalRead;
	}
	return TRUE;
}

BOOL ThorSLM::ReadAndScaleBitmap(int mode)
{
	long ret = TRUE, size;
	BITMAPINFO bmi;

	unsigned char* imgRead = (FALSE == _slm3D) ? GetAndProcessBMP(size, bmi) : GetAndProcessText(size, bmi);
	if(NULL == imgRead)
	{
		return FALSE;
	}

	//meadowlark overdrive use provided dll:
	if(_overDrive && mode)
	{
		if(NULL == _blinkSDK)
		{
			delete[] imgRead;
			return FALSE;
		}

		unsigned char* imgBGR = ConvertRGBToBGRBuffer(imgRead, bmi.bmiHeader, &size);
		if(NULL == imgBGR)
		{
			delete[] imgRead;
			return FALSE;
		}

		try
		{			
			if(1 == _bufferCount)
			{
				//release transient buffers:
				ReleaseTransientBuf();

				//persist first buffer:
				ReallocMemChk(_firstBuf, size*sizeof(unsigned char));
				memcpy_s(_firstBuf.memPtr, size*sizeof(unsigned char), imgBGR, size*sizeof(unsigned char));
			}
			else
			{
				MemoryStruct tmpBuf = {imgBGR, size*sizeof(unsigned char)};
				ret = SetIntermediateBuffer(tmpBuf);
			}

			//write image for proper transient frame calculation:
			_blinkSDK->Write_overdrive_image(1, imgBGR, false, false);
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ReadAndScaleBitmap of overdrive SLM failed.");
			LogMessage(_errMsg,ERROR_EVENT);
			string str = _blinkSDK->Get_last_error_message();
			StringCbPrintfW(_errMsg,MSG_SIZE,ConvertStringToWString(str).c_str());
			LogMessage(_errMsg,ERROR_EVENT);
			ret = FALSE;
		}
		delete[] imgRead;
		delete[] imgBGR;
		return ret;
	}

	long tSize = _pixelX * _pixelY * RGB_CNT;
	BYTE* pImg = new BYTE[tSize];
	if(NULL == pImg)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorSLMPDM512 ReadAndScaleBitmap: new alloc buffer failed.");
		LogMessage(_errMsg, ERROR_EVENT);
		delete[] imgRead;
		return FALSE;
	}

	switch (mode)
	{
	case 0:		//General: RGB are identical
		for (int row=0; row<_pixelY; row++)
		{
			for (int col=0; col<_pixelX; col++)
			{
				pImg[row*_pixelX*RGB_CNT + col*RGB_CNT + 0] = imgRead[row*_pixelX + col];
				pImg[row*_pixelX*RGB_CNT + col*RGB_CNT + 1] = pImg[row*_pixelX*RGB_CNT + col*RGB_CNT + 0];
				pImg[row*_pixelX*RGB_CNT + col*RGB_CNT + 2] = pImg[row*_pixelX*RGB_CNT + col*RGB_CNT + 0];
			}
		}
		break;
	case 1:		//Meadowlark: (R: Volt, G: Hologram, B: 0)
		switch ((int)(size/_pixelX/_pixelY))
		{
			int PixVal;
		case 1:
			//make the info we have our most significan bits - GREEN
			for (int row=0; row<_pixelY; row++)
			{
				for (int col=0; col<_pixelX; col++)
				{
					PixVal = imgRead[row*_pixelX + col]<<CHAR_BIT;
					PixVal = (NULL == _imgWavefront) ? _tableLUT[PixVal] :	//GREEN (high 8 bits) + RED (low 8 bits)
						_tableLUT[((PixVal + static_cast<int>((_imgWavefront[row*_pixelX*RGB_CNT + col*RGB_CNT + 1]<<CHAR_BIT) + (_imgWavefront[row*_pixelX*RGB_CNT + col*RGB_CNT + 0]))) % (LUT_SIZE - 1))];
					pImg[row*_pixelX*RGB_CNT + col*RGB_CNT + 0] = (unsigned char)PixVal;	//RED
					pImg[row*_pixelX*RGB_CNT + col*RGB_CNT + 1] = (PixVal>>CHAR_BIT);		//GREEN
					pImg[row*_pixelX*RGB_CNT + col*RGB_CNT + 2] = 0;						//BLUE
				}
			}		
			break;
		case RGB_CNT:
			//not tested yet, should already be RGB:
			BYTE bByte, gByte, rByte;
			for (int row=0; row<_pixelY; row++)
			{
				for (int col=0; col<_pixelX; col++)
				{
					rByte = imgRead[row*_pixelX*RGB_CNT + col*RGB_CNT + 0];
					gByte = imgRead[row*_pixelX*RGB_CNT + col*RGB_CNT + 1];
					bByte = imgRead[row*_pixelX*RGB_CNT + col*RGB_CNT + 2];
					PixVal = (gByte<<8) + rByte;
					PixVal = (NULL == _imgWavefront) ? _tableLUT[PixVal] :	//GREEN (high 8 bits) + RED (low 8 bits)
						_tableLUT[((PixVal + static_cast<int>((_imgWavefront[row*_pixelX*RGB_CNT + col*RGB_CNT + 1]<<CHAR_BIT) + (_imgWavefront[row*_pixelX*RGB_CNT + col*RGB_CNT + 0]))) % (LUT_SIZE - 1))];
					pImg[row*_pixelX*RGB_CNT + col*RGB_CNT + 0] = (unsigned char)PixVal;	//RED
					pImg[row*_pixelX*RGB_CNT + col*RGB_CNT + 1] = (PixVal>>8);				//GREEN
					pImg[row*_pixelX*RGB_CNT + col*RGB_CNT + 2] = 0;						//BLUE
				}
			}
			break;
		} 
		break;
	default:
		break;
	}

	//add to DVI with converted-back bmp:
	bmi.bmiHeader.biSizeImage = tSize;
	bmi.bmiHeader.biBitCount = CHAR_BIT * RGB_CNT;
	BYTE* imgBGR = ConvertRGBToBGRBuffer(pImg, bmi.bmiHeader, &size);
	winDVI->EditBMP(_arrayOrFileID, imgBGR, bmi);

	//clear:
	delete[] imgRead;
	delete[] pImg;
	delete[] imgBGR;

	//create window, let winDVI to decide 
	//whether to create a new one or not:
	winDVI->CreateDVIWindow(_pixelX, _pixelY);
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
		if (_fpPointsXY[i])
		{
			free((void*)_fpPointsXY[i]);
			_fpPointsXY[i] = NULL;
			_fpPointsXYSize[i] = 0;
		}
	}
	//overdrive SLM SDK will be reconstructed in FindDevice:
	if(NULL != _blinkSDK)
	{
		delete _blinkSDK;
		_blinkSDK = NULL;
	}

	ReleaseTransientBuf();
}

void ThorSLM::ReleaseTransientBuf()
{
	//release transient buffer
	for (int i = 0; i < MAX_ARRAY_CNT; i++)
	{
		ReallocMemChk(_intermediateBuf[i], 0);
	}

	//release first pattern buffer
	ReallocMemChk(_firstBuf, 0);
}

long ThorSLM::SaveHologram(bool saveInSubFolder)
{
	long size;
	BITMAPINFO bmi;

	unsigned char* imgRead = GetAndProcessBMP(size, bmi);
	if(NULL == imgRead)
	{
		return FALSE;
	}

	//save grayscale phase mask:
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	wchar_t rawPath[_MAX_PATH];
	_wsplitpath_s(_bmpPathAndName.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
	if(wcsstr (dir,L"SLMWaveforms"))	//only create sub-folder under "SLMWaveforms"
	{
		StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s%s",drive,dir,L"PhaseMask\\");
		CreateDirectory(rawPath, NULL);
	}
	if(saveInSubFolder)
	{
		StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s%s%s%s",drive,dir,L"PhaseMask\\",fname,ext);
	}
	else
	{
		StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s%s%s",drive,dir,fname,ext);
	}

	unsigned char* imgBGR = ConvertRGBToBGRBuffer(imgRead, bmi.bmiHeader, &size);
	SaveBMP(imgBGR, _pixelX, _pixelY, size, rawPath);

	delete[] imgRead;
	delete[] imgBGR;
	return TRUE;
}

unsigned char* ThorSLM::GetAndProcessBMP(long& size, BITMAPINFO& bmi)
{
	long ret = TRUE;
	int imgWidth, imgHeight;
	const int ITERATIONS_2D = 10;

	unsigned char* imgRead = LoadBMP(&imgWidth, &imgHeight, &size, &bmi.bmiHeader, _bmpPathAndName.c_str());
	if(NULL == imgRead)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorSLMPDM512 GetAndProcessBMP: load bmp file failed.");
		LogMessage(_errMsg, ERROR_EVENT);
		return NULL;
	}

	//convert buffer:
	long newSize;
	unsigned char* imgRGB = ConvertBGRToRGBBuffer(imgRead, bmi.bmiHeader, &newSize);

	//update local sizes:	
	if ((imgHeight >= _pixelRange[2]) && (imgHeight <= _pixelRange[3]) && 
		(imgWidth >= _pixelRange[0]) && (imgWidth <= _pixelRange[1]) &&
		(newSize == imgWidth*imgHeight))
	{
		_pixelY = imgHeight;
		_pixelX = imgWidth;
	}
	else
	{
		delete[] imgRead;
		delete[] imgRGB;
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorSLMPDM512 GetAndProcessBMP: invalid bmp size.");
		LogMessage(_errMsg, ERROR_EVENT);
		return NULL;
	}

	//apply affine & generate phase:
	ret = holoGen->SetSize(imgWidth, imgHeight);
	ret = holoGen->SetCoeffs(GeoFittingAlg::PROJECTIVE, _fitCoeff);
	ret = holoGen->SetAlgorithm(HoloGenAlg::GerchbergSaxton);

	float* fImg = (float*)std::malloc(imgWidth*imgHeight*sizeof(float));

	//copy to buffer:
	unsigned char* pSrc = imgRGB;
	float* pDst = fImg;
	for (int i = 0; i < imgWidth*imgHeight; i++)
	{
		*pDst = *pSrc;
		pDst++;
		pSrc++;
	}
	delete[] imgRGB;

	//pre-offset, flip, scale or rotate before affine:
	if((0 != _offsetPixels[0]) || (0 != _offsetPixels[1]))
	{
		ret = holoGen->OffsetByPixels(fImg,_offsetPixels[0],_offsetPixels[1]);
	}
	if(_verticalFlip)
	{
		ret = holoGen->VerticalFlip(fImg);
	}
	if((0.0 < _scaleFactor[0]) && (0.0 < _scaleFactor[1]) &&  
		((1.0 != _scaleFactor[0]) || (1.0 != _scaleFactor[1])))
	{
		ret = holoGen->ScaleByFactor(fImg, _scaleFactor[0], _scaleFactor[1]);
	}
	if(0.0 != _rotateAngle)
	{
		ret = holoGen->RotateForAngle(fImg, _rotateAngle);
	}
	ret = holoGen->FittingTransform(fImg);
	ret = holoGen->GenerateHologram(fImg, ITERATIONS_2D, 0);

	//copy back from buffer, scale if necessary:
	pSrc = imgRead;
	pDst = fImg;
	for (int i = 0; i < imgWidth*imgHeight; i++)
	{
		*pSrc = ((MAX_ARRAY_CNT-1) < (*pDst)) ? static_cast<unsigned char>((*pDst)*(MAX_ARRAY_CNT-1)/(LUT_SIZE-1)) : static_cast<unsigned char>((*pDst));
		pSrc++;
		pDst++;
	}
	std::free(fImg);

	return imgRead;
}

unsigned char* ThorSLM::GetAndProcessText(long& size, BITMAPINFO& bmi)	//for 3D pattern, we should use textfile instead of bmp
{
	long ret = TRUE;
	int imgWidth, imgHeight;
	const int ITERATIONS_3D = 100;

	ifstream myReadFile;

	imgWidth=512;
	imgHeight=512;
	wstring tempPath=L".txt";
	unsigned char* txtRead = LoadBMP(&imgWidth, &imgHeight, &size, &bmi.bmiHeader, _bmpPathAndName.c_str());

	wchar_t* textFileName=(wchar_t*)_bmpPathAndName.c_str();
	std::wstring temp=textFileName;
	std::wstring key (L".bmp");
	std::wstring::size_type found = temp.find(key);


	_pixelY = imgHeight;
	_pixelX = imgWidth;


	if (found!=std::string::npos)
		temp.erase(found);


	temp=temp+tempPath;
	textFileName=(wchar_t*)temp.c_str();



	myReadFile.open(textFileName);


	std::string tempStr;
	float z;
	float x;
	float y;
	float* fImg;
	float* mImg = (float*)std::calloc(imgWidth*imgHeight,sizeof(float));

	ret = holoGen->SetAlgorithm(HoloGenAlg::CompressiveSensing);
	ret = holoGen->Set3DParam(_na, _wavelength);

	if (myReadFile.is_open())
	{

		while (std::getline(myReadFile,tempStr))
		{
			fImg = (float*)std::calloc(imgWidth*imgHeight,sizeof(float));
			std::stringstream ss(tempStr);
			while (std::getline(ss,tempStr,' '))
			{
				if (tempStr.find(',')==std::string::npos)
					z=std::stof(tempStr);
				else
				{
					x=std::stof(tempStr.substr(0,tempStr.find(',')));
					y=std::stof(tempStr.substr(tempStr.find(',')+1,tempStr.length()));
					fImg[(int)x+512*(int)y]=255;

				}





			}
			//apply affine & generate phase:
			ret = holoGen->SetSize(imgWidth, imgHeight);
			ret	= holoGen->SetCoeffs(GeoFittingAlg::PROJECTIVE, _fitCoeff);



			//pre-offset, flip, scale or rotate before affine:
			if((0 != _offsetPixels[0]) || (0 != _offsetPixels[1]))
			{
				ret = holoGen->OffsetByPixels(fImg,_offsetPixels[0],_offsetPixels[1]);
			}
			if(_verticalFlip)
			{
				ret = holoGen->VerticalFlip(fImg);
			}
			if((0.0 < _scaleFactor[0]) && (0.0 < _scaleFactor[1]) &&  
				((1.0 != _scaleFactor[0]) || (1.0 != _scaleFactor[1])))
			{
				ret = holoGen->ScaleByFactor(fImg, _scaleFactor[0], _scaleFactor[1]);

			}
			if(0.0 != _rotateAngle)
			{
				ret = holoGen->RotateForAngle(fImg, _rotateAngle);
			}
			ret = holoGen->FittingTransform(fImg);
			//ret = holoGen->GenerateHologram(fImg, ITERATIONS,-z*1000/2.1);	//hardcoding for Nick Robinson
			ret = holoGen->GenerateHologram(fImg, ITERATIONS_3D,z*Constants::UM_TO_MM);
			for (int q=0;q<imgHeight*imgWidth;q++)
			{
				mImg[q]=mImg[q]+fImg[q];

			}
			free(fImg);
		}
		myReadFile.close();
	}	
	else
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMPDM512 GetAndProcessText: load text file failed.");
		LogMessage(_errMsg, ERROR_EVENT);
		return NULL;
	}
	
	//free(fullFileName);




	unsigned char* pSrc = txtRead;
	float* pDst = mImg;

	for (int i = 0; i < imgWidth*imgHeight; i++)
	{
		*pSrc = ((MAX_ARRAY_CNT-1) < (*pDst)) ? static_cast<unsigned char>((*pDst)*(MAX_ARRAY_CNT-1)/(LUT_SIZE-1)) : static_cast<unsigned char>((*pDst));
		pSrc++;
		pDst++;
	}

	std::free(mImg);





	return txtRead;
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
	ReallocMemChk(_slmTempBuf, 0);
	ReallocMemChk(_firstBuf, 0);
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
			FILE *sequenceFile = NULL;
			if (0 == fopen_s(&sequenceFile, ConvertWStringToString(seqName).c_str(), "r"))
			{
				//determine total count
				char line[_MAX_PATH];
				unsigned int iVal = 0, maxID = 0;
				while(NULL != fgets(line, sizeof(line), sequenceFile) && 0 != strcmp(line,"\n") &&  0 != atoi(line))
				{
					maxID = max(maxID, static_cast<unsigned int>(atoi(line)));	//pattern ID is 1-based
					_slmSeqVec.push_back(atoi(line)-1);							//array ID is 0-based
				}

				//validate sequence, pattern ID should be below maximum count
				if (maxID > static_cast<unsigned int>(_bufferCount))
					_slmSeqVec.clear();

				fclose(sequenceFile);
			}
		}
		catch(...)
		{
			_slmSeqVec.clear();
			_callbackMutex.unlock();
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorSLM:%hs@%u: ResetSequence failed: %s", __FILE__, __LINE__, seqName.c_str());
			LogMessage(_errMsg,ERROR_EVENT);
			return FALSE;
		}
	}

	ret = (0 >= _slmSeqVec.size()) ? FALSE : TRUE;

	//re-start from the first
	_arrayOrFileID = 0;
	return ret;
}

long ThorSLM::SetIntermediateBuffer(MemoryStruct memStruct)
{
	long ret = TRUE;
	unsigned int tmpCount = 0U;

	if((NULL == memStruct.memPtr) || (0 == memStruct.size))
		return FALSE;

#ifdef _DEBUG
	HighPerfTimer timer;
	timer.Start();
#endif
	if (TRUE == _slmRuntimeCalculate)
	{
		//persist image buffers, then calculate transient at callback
		ReallocMemChk(_intermediateBuf[_arrayOrFileID], memStruct.size);
		memcpy_s(_intermediateBuf[_arrayOrFileID].memPtr, memStruct.size, memStruct.memPtr, memStruct.size);
	}
	else
	{
		//calculate transient to current frame, buffer size including header:
		if(false == _blinkSDK->Calculate_transient_frames(memStruct.memPtr, &tmpCount))
			return FALSE;

		ReallocMemChk(_slmTempBuf, tmpCount*sizeof(unsigned char));
		if ((0 == tmpCount) || (false == _blinkSDK->Retrieve_transient_frames(_slmTempBuf.memPtr)))
		{
			ReallocMemChk(_slmTempBuf, 0);
			return FALSE;
		}

		//persist transient buffers:
		ReallocMemChk(_intermediateBuf[_arrayOrFileID], tmpCount*sizeof(unsigned char));

		if(NULL == _intermediateBuf[_arrayOrFileID].memPtr)
		{
			ReallocMemChk(_slmTempBuf, 0);
			return FALSE;
		}

		memcpy_s(_intermediateBuf[_arrayOrFileID].memPtr, tmpCount*sizeof(unsigned char), _slmTempBuf.memPtr, tmpCount*sizeof(unsigned char));
	}
#ifdef _DEBUG
	timer.Stop();
	StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorSLM:%hs@%u: SetIntermediateBuffer time: %d ms", __FILE__, __LINE__, static_cast<int>(timer.ElapsedMilliseconds()));
	LogMessage(_errMsg,INFORMATION_EVENT);
#endif
	return ret;
}

long ThorSLM::SetupHWTriggerIn()
{
	int32 retVal = 0, error = 0;
	if((0 < _counterLine.size()) && (string::npos != _counterLine.find("ctr")) && (0 < _hwTriggerInput.size()))
	{
		if(_taskHandleCI)
		{
			retVal = DAQmxStopTask(_taskHandleCI);
			retVal = DAQmxClearTask(_taskHandleCI);
			_taskHandleCI = NULL;
		}
		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleCI));
		DAQmxErrChk(L"DAQmxCreateCICountEdgesChan",retVal = DAQmxCreateCICountEdgesChan (_taskHandleCI, _counterLine.c_str(), "", DAQmx_Val_Rising, 0, DAQmx_Val_CountUp ));
		DAQmxErrChk(L"DAQmxSetCICountEdgesTerm",retVal = DAQmxSetCICountEdgesTerm(_taskHandleCI,_counterLine.c_str(),_hwTriggerInput.c_str()));
		DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleCI,_hwTriggerInput.c_str(),1000,DAQmx_Val_Rising,DAQmx_Val_HWTimedSinglePoint, 0));
		DAQmxErrChk(L"DAQmxRegisterSignalEvent",retVal = DAQmxRegisterSignalEvent(_taskHandleCI, DAQmx_Val_SampleClock , 0, ThorSLM::HWTriggerCallback, NULL));	
		DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleCI));
	}
	return retVal;
}
