// ThorSLMPDM512.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ThorSLMPDM512.h"
#include "..\..\..\ThorSLM\ThorSLM\ThorSLMSetupXML.h"
#include "Strsafe.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <complex>


///static Members & global variables:
wchar_t MeadowlarkPDM::_errMsg[MSG_SIZE];
bool MeadowlarkPDM::_instanceFlag = false;
unique_ptr<MeadowlarkPDM> MeadowlarkPDM::_single(new MeadowlarkPDM());
unique_ptr<ThorSLMXML> pSetup(new ThorSLMXML());
long MeadowlarkPDM::_arrayOrFileID = 0;		//0-based buffer or sequence index
long MeadowlarkPDM::_bufferCount = 1;			//1-based total buffer size
long MeadowlarkPDM::_slmRuntimeCalculate = FALSE;	//[1]:calculate transient frames at runtime
MemoryStruct<unsigned char> MeadowlarkPDM::_intermediateBuf[MAX_ARRAY_CNT]; //intermediate buffers for transient or bmp
Blink_SDK* MeadowlarkPDM::_blinkSDK = NULL;
unsigned int MeadowlarkPDM::_slmTimeout = SLM_TIMEOUT_MIN;
MemoryStruct<unsigned char> MeadowlarkPDM::_slmTempBuf;		//tempearary buffer for transient calculation
mutex MeadowlarkPDM::_callbackMutex;						//mutex lock to update static params used in callback

MeadowlarkPDM::MeadowlarkPDM() :
	DEFAULT_TRUE_FRAMES(5),
	DEFAULT_TRANSIENT_FRAMES(10U),
	MAX_TRANSIENT_FRAMES(20U)
{
	_deviceCount = 0;
	SetDefault();
	_blinkSDK = NULL;
}

MeadowlarkPDM* MeadowlarkPDM::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new MeadowlarkPDM());
		_instanceFlag = true;
	}
	return _single.get();
}

MeadowlarkPDM::~MeadowlarkPDM()
{
	_instanceFlag = false;
	ReleaseMem();
}

long MeadowlarkPDM::FindSLM(char* xml)
{
	_deviceCount = 0;
	ThorSLMXML* pSetup = (ThorSLMXML*)xml;
	try
	{
		double pixelPitchUM = 1.0, flatDiagRatio = 1.0;
		double flatPowerRange[2] = {0.0};
		long pixelSize[2] = {0}, dmdMode = 0, persistHologramZone1 = 0, persistHologramZone2 = 0;
		unsigned int transientFrames = DEFAULT_TRANSIENT_FRAMES;
		std::string lutFile, overDrivelutFile, wavefrontFile;

		if (FALSE == pSetup->GetSpec(_pSlmName, dmdMode, _overDrive, transientFrames, pixelPitchUM, flatDiagRatio, flatPowerRange[0], flatPowerRange[1], pixelSize[0], pixelSize[1], lutFile, overDrivelutFile, wavefrontFile, persistHologramZone1, persistHologramZone2))
		{
			pSetup->GetLastErrorMsg(_errMsg, MSG_SIZE);
			return 0;
		}

		if (NULL != _blinkSDK)
		{
			delete _blinkSDK;
			_blinkSDK = NULL;
		}

		//overdrive will be using Blink_SDK dll:
		if ((_overDrive) && (0 == _pSlmName.compare("PDM512")))
		{
			if (0 < overDrivelutFile.length())
			{
				const unsigned int bits_per_pixel = 8U;
				//const unsigned int pixel_dimension = 512U;
				const bool         is_nematic_type = true;
				const bool         RAM_write_enable = true;
				const bool         use_GPU_if_available = true;

				unsigned int n_boards_found = 0U;
				bool         constructed_okay = true;


				transientFrames = ((transientFrames < MAX_TRANSIENT_FRAMES) && (0 < transientFrames)) ? transientFrames : DEFAULT_TRANSIENT_FRAMES;

				_blinkSDK = new Blink_SDK(bits_per_pixel, &n_boards_found, &constructed_okay, is_nematic_type, RAM_write_enable, use_GPU_if_available, static_cast<unsigned __int64>(transientFrames), overDrivelutFile.c_str());

				if (n_boards_found && constructed_okay)
				{
					//check hardware:
					if (_blinkSDK->Is_overdrive_available() && _blinkSDK->Is_slm_transient_constructed())
					{
						_deviceCount = n_boards_found;
						_blinkSDK->Set_true_frames(DEFAULT_TRUE_FRAMES);
						_blinkSDK->SLM_power(1, true);
						if (0 < lutFile.length())
						{
							_blinkSDK->Load_LUT_file(1, lutFile.c_str());
						}
						else
						{
							_blinkSDK->Load_linear_LUT(1);
						}
					}
					else
					{
						_deviceCount = 0;
						StringCbPrintfW(_errMsg, MSG_SIZE, L"Found SLM OD %d, constructed: %d, overdrive available: %d transient constructed: %d\nPlease check your SLM overdrive LUT path and settings.", n_boards_found, constructed_okay, _blinkSDK->Is_overdrive_available(), _blinkSDK->Is_slm_transient_constructed());
						MessageBox(NULL, _errMsg, L"Find SLM Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
					}
				}
				else
				{
					_deviceCount = 0;
					StringCbPrintfW(_errMsg, MSG_SIZE, L"Found SLM OD %d, constructed: %d\nPlease check your SLM hardware.", n_boards_found, constructed_okay);
					MessageBox(NULL, _errMsg, L"Find SLM Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
				}
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"OverDriveLUT is required for overdrive.");
				MessageBox(NULL, _errMsg, L"Find SLM Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
			}
		}
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMPDM512 FindDevices failed.");
	}
	return _deviceCount;
}

long MeadowlarkPDM::GetParam(const long paramID, double& param)
{
	long ret = TRUE;
	switch (paramID)
	{
	case ISLM::SLMParams::IS_AVAILABLE:
		param = (0 < _deviceCount && TRUE == IsOverdrive()) ? TRUE : FALSE;
		break;
	case ISLM::SLMParams::ARRAY_ID:
		param = _arrayOrFileID;
		break;
	case ISLM::SLMParams::TIMEOUT:
		param = _slmTimeout;
		break;
	case ISLM::SLMParams::RUNTIME_CALC:
		param = _slmRuntimeCalculate;
		break;
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

long MeadowlarkPDM::SetParam(const long paramID, const double param)
{
	long ret = TRUE;
	switch (paramID)
	{
	case ISLM::SLMParams::ARRAY_ID:
		if ((param >= 0) && (param < MAX_ARRAY_CNT))
		{
			_arrayOrFileID = static_cast<long> (param);
			_bufferCount = (_bufferCount < (_arrayOrFileID + 1)) ? (_arrayOrFileID + 1) : _bufferCount;
		}
		break;
	case ISLM::SLMParams::TIMEOUT:
		_slmTimeout = static_cast<unsigned int>(param);
		break;
	case ISLM::SLMParams::RUNTIME_CALC:
		_slmRuntimeCalculate = static_cast<long>(param);
		break;
	case ISLM::SLMParams::RELEASE_TRANSIANT_BUFFER:
		if (TRUE == param) { ReleaseTransientBuf(); }
		break;
	case ISLM::SLMParams::WRITE_TRANSIANT_BUFFER:
		_blinkSDK->Write_overdrive_image(1, _intermediateBuf[static_cast<int>(param)].GetMem(), false, false);
		break;	
	default:
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMPDM512 Parameter (%d) not implemented", paramID);
		ret = FALSE;
		break;
	}
	return ret;
}

long MeadowlarkPDM::TeardownSLM()
{
	try
	{
		if (TRUE == IsOverdrive())
		{
			_blinkSDK->SLM_power(1, false);
		}
		SetDefault();
	}
	catch (...)
	{
	}

	ReleaseMem();
	return TRUE;
}

long MeadowlarkPDM::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;
	MemoryStruct<unsigned char> tmpBuf;
	switch (paramID)
	{
	case ISLM::SLMParams::WRITE_BUFFER:
		_blinkSDK->Write_overdrive_image(1, (const unsigned char*)pBuffer, false, false);
		break;
	case ISLM::SLMParams::WRITE_FIRST_BUFFER:
		_firstBuf.ReallocMemChk(size);
		SAFE_MEMCPY(_firstBuf.GetMem(), size, pBuffer);
		break;
	case ISLM::SLMParams::SET_TRANSIANT_BUFFER:
		tmpBuf = MemoryStruct<unsigned char> { (unsigned char*)pBuffer, (DWORD)size };
		ret = SetIntermediateBuffer(tmpBuf);
		break;
	default:
		ret = FALSE;
		break;
	}
	return ret;

}

long MeadowlarkPDM::StartSLM()
{
	if (IsOverdrive())
	{
		if (1 < _bufferCount)
		{
			SetIntermediateBuffer(_firstBuf);
		}
		//write first bmp
		_blinkSDK->Write_overdrive_image(1, _firstBuf.GetMem(), false, false);
	}
	return TRUE;
}

long MeadowlarkPDM::StopSLM()
{
	_blinkSDK->Stop_sequence();
	return TRUE;
}

long MeadowlarkPDM::UpdateSLM(long arrayID)
{
	if (TRUE == _slmRuntimeCalculate)
	{
		//calculate transient frames then write:
		unsigned int tmpCount = 0U;
		if (false == _blinkSDK->Calculate_transient_frames(_intermediateBuf[arrayID].GetMem(), &tmpCount))
			return FALSE;

		_slmTempBuf.ReallocMemChk(tmpCount * sizeof(unsigned char));
		if ((0 == tmpCount) || (false == _blinkSDK->Retrieve_transient_frames(_slmTempBuf.GetMem())))
			return FALSE;

		_blinkSDK->Write_transient_frames(1, _slmTempBuf.GetMem(), 0U, true, false, _slmTimeout);
	}
	else
	{
		//write pre-calculated transient frames:
		_blinkSDK->Write_transient_frames(1, _intermediateBuf[arrayID].GetMem(), 0U, true, false, _slmTimeout);
	}
	return TRUE;
}

long MeadowlarkPDM::GetLastErrorMsg(wchar_t* msg, long size)
{
	string str = _blinkSDK->Get_last_error_message();
	StringCbPrintfW(msg, size, StringToWString(str).c_str());
	return TRUE;
}


/// ***		Private Functions	*** ///

void MeadowlarkPDM::ReleaseMem()
{
	//overdrive SLM SDK will be reconstructed in FindSLM
	if (NULL != _blinkSDK)
	{
		delete _blinkSDK;
		_blinkSDK = NULL;
	}

	ReleaseTransientBuf();
}

void MeadowlarkPDM::ReleaseTransientBuf()
{
	//release transient buffer
	for (int i = 0; i < MAX_ARRAY_CNT; i++)
	{
		_intermediateBuf[i].ReallocMemChk(0);
	}

	//release first pattern buffer
	_firstBuf.ReallocMemChk(0);
}

void MeadowlarkPDM::SetDefault()
{
	_errMsg[0] = 0;
	_pSlmName = "";
	_bufferCount = 1;
	_arrayOrFileID = 0;
	_overDrive = 0;
	_slmTempBuf.ReallocMemChk(0);
	_firstBuf.ReallocMemChk(0);
}

long MeadowlarkPDM::SetIntermediateBuffer(MemoryStruct<unsigned char> memStruct)
{
	unsigned int tmpCount = 0U;

	if ((NULL == memStruct.GetMem()) || (0 == memStruct.GetSize()))
		return FALSE;

#ifdef _DEBUG
	HighPerfTimer timer;
	timer.Start();
#endif
	_callbackMutex.lock();

	if (TRUE == _slmRuntimeCalculate)
	{
		//persist image buffers, then calculate transient at callback
		_intermediateBuf[_arrayOrFileID].ReallocMemChk(memStruct.GetSize());
		SAFE_MEMCPY(_intermediateBuf[_arrayOrFileID].GetMem(), memStruct.GetSize(), memStruct.GetMem());
	}
	else
	{
		//calculate transient to current frame, buffer size including header:
		if (false == _blinkSDK->Calculate_transient_frames(memStruct.GetMem(), &tmpCount))
			goto FALSE_RETURN;

		_slmTempBuf.ReallocMemChk(tmpCount * sizeof(unsigned char));
		if ((0 == tmpCount) || (false == _blinkSDK->Retrieve_transient_frames(_slmTempBuf.GetMem())))
		{
			_slmTempBuf.ReallocMemChk(0);
			goto FALSE_RETURN;
		}

		//persist transient buffers:
		_intermediateBuf[_arrayOrFileID].ReallocMemChk(tmpCount * sizeof(unsigned char));

		if (NULL == _intermediateBuf[_arrayOrFileID].GetMem())
		{
			_slmTempBuf.ReallocMemChk(0);
			goto FALSE_RETURN;
		}

		SAFE_MEMCPY(_intermediateBuf[_arrayOrFileID].GetMem(), tmpCount * sizeof(unsigned char), _slmTempBuf.GetMem());
	}
#ifdef _DEBUG
	timer.Stop();
	StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMPDM512:%u: SetIntermediateBuffer time: %d ms", __LINE__, static_cast<int>(timer.ElapsedMilliseconds()));
#endif
	_callbackMutex.unlock();
	return TRUE;

FALSE_RETURN:
	_callbackMutex.unlock();
	return FALSE;
}