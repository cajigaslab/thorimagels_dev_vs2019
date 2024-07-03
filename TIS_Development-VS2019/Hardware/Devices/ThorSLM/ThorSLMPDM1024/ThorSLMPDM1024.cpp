// ThorSLMPDM1024.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ThorSLMPDM1024.h"
#include "..\..\..\ThorSLM\ThorSLM\ThorSLMSetupXML.h"
#include "Strsafe.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <complex>
#include "Blink_C_wrapper.h"
//#include "ImageGen.h"


///static Members & global variables:
wchar_t MeadowlarkPDM::_errMsg[MSG_SIZE];
bool MeadowlarkPDM::_instanceFlag = false;
unique_ptr<MeadowlarkPDM> MeadowlarkPDM::_single(new MeadowlarkPDM());
unique_ptr<ThorSLMXML> pSetup(new ThorSLMXML());
long MeadowlarkPDM::_arrayOrFileID = 0;			//0-based buffer or sequence index
long MeadowlarkPDM::_bufferCount = 1;			//1-based total buffer size
MemoryStruct<unsigned char> MeadowlarkPDM::_intermediateBuf[MAX_ARRAY_CNT]; //intermediate buffers for transient or bmp
unsigned int MeadowlarkPDM::_slmTimeout = SLM_TIMEOUT_MIN;
MemoryStruct<unsigned char> MeadowlarkPDM::_slmTempBuf;		//tempearary buffer for transient calculation

MeadowlarkPDM::MeadowlarkPDM() :
	DEFAULT_TRUE_FRAMES(5),
	UNIT_TRANSIENT_FRAME_US(700U),
	DEFAULT_TRANSIENT_FRAMES(10U),
	MAX_TRANSIENT_FRAMES(20U)
{
	_deviceCount = 0;
	_transferBuf[0] = _transferBuf[1] = NULL;
	_transferBufSize[0] = _transferBufSize[1] = 0;
	_unitTimeoutMS = (int)ceil((double)UNIT_TRANSIENT_FRAME_US / Constants::MS_TO_SEC);
	SetDefault();
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
		double flatPowerRange[2] = { 0.0 };
		long pixelSize[2] = { 0 }, dmdMode = 0, persistHologramZone[2] = { 0 }, dualPatternShiftPx = 0;
		unsigned int transientFrames = DEFAULT_TRANSIENT_FRAMES;
		std::string lutFile, overDrivelutFile, wavefrontFile;

		if (FALSE == pSetup->GetBlank(dualPatternShiftPx, persistHologramZone[0], persistHologramZone[1]))
		{
			pSetup->GetLastErrorMsg(_errMsg, MSG_SIZE);
			return 0;
		}
		if (FALSE == pSetup->GetSpec(_pSlmName, dmdMode, _overDrive, transientFrames, pixelPitchUM, flatDiagRatio, flatPowerRange[0], flatPowerRange[1], pixelSize[0], pixelSize[1], lutFile, overDrivelutFile, wavefrontFile))
		{
			pSetup->GetLastErrorMsg(_errMsg, MSG_SIZE);
			return 0;
		}

		//overdrive will be using Blink_SDK dll:
		if (IsOverdrive() && 0 < overDrivelutFile.length())
		{
			const unsigned int	bits_per_pixel = 8U;
			unsigned int		n_boards_found = 0U;
			const bool			is_nematic_type = true;
			const bool			RAM_write_enable = true;
			const bool			use_GPU_if_available = true;
			int					constructed_okay = TRUE;

			transientFrames = ((transientFrames < MAX_TRANSIENT_FRAMES) && (0 < transientFrames)) ? transientFrames : DEFAULT_TRANSIENT_FRAMES;
			_unitTimeoutMS = (int)ceil((double)transientFrames * UNIT_TRANSIENT_FRAME_US / Constants::MS_TO_SEC);
			Create_SDK(bits_per_pixel, &n_boards_found, &constructed_okay, is_nematic_type, RAM_write_enable, use_GPU_if_available, static_cast<unsigned __int64>(transientFrames), (char*)overDrivelutFile.c_str());

			if (n_boards_found && constructed_okay)
			{
				_deviceCount = n_boards_found;
				Set_true_frames(DEFAULT_TRUE_FRAMES);
				SLM_power(true);
				if (0 < lutFile.length())
				{
					Load_LUT_file(1, (char*)lutFile.c_str());
				}
				else
				{
					Load_linear_LUT(1);
				}
				//}
				//else
				//{
				//	_deviceCount = 0;
				//	StringCbPrintfW(_errMsg, MSG_SIZE, L"Found SLM OD %d, constructed: %d, transient constructed: %d\nPlease check your SLM overdrive LUT path and settings.", n_boards_found, constructed_okay, Is_slm_transient_constructed());
				//	MessageBox(NULL, _errMsg, L"Find SLM Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
				//}
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
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMPDM1024 FindDevices failed.");
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
	case ISLM::SLMParams::RELEASE_TRANSIANT_BUFFER:
		if (TRUE == param) { ReleaseTransientBuf(); }
		break;
	case ISLM::SLMParams::WRITE_TRANSIANT_BUFFER:
		Write_image(1, _intermediateBuf[static_cast<int>(param)].GetMem(), _intermediateBuf[static_cast<int>(param)].GetSize(), false, false, false, false, _unitTimeoutMS);
		ImageWriteComplete(1, Constants::EVENT_WAIT_TIME);
		break;
	default:
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMPDM1024 Parameter (%d) not implemented", paramID);
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
			SLM_power(false);
			Delete_SDK();
		}
		SetDefault();
	}
	catch (...)
	{
	}

	ReleaseMem();
	return TRUE;
}

long MeadowlarkPDM::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;
	switch (paramID)
	{
	case ISLM::SLMParams::GET_CURRENT_BUFFER:
		if (0 > _slmTempBuf.GetIndex())
		{
			if (size <= _transferBufSize[0])
				SAFE_MEMCPY(pBuffer, size, _transferBuf[0]);
		}
		else
		{
			if ((DWORD)size < _slmTempBuf.GetSize())
				SAFE_MEMCPY(pBuffer, _slmTempBuf.GetSize(), _slmTempBuf.GetMem());
		}
		break;
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

long MeadowlarkPDM::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;
	switch (paramID)
	{
	case ISLM::SLMParams::WRITE_BUFFER:
		if (size != _transferBufSize[1])
		{
			_transferBuf[1] = (unsigned char*)realloc(_transferBuf[1], size);
			_transferBufSize[1] = size;
		}
		SAFE_MEMCPY(_transferBuf[1], _transferBufSize[1], pBuffer);
		Write_image(1, _transferBuf[1], static_cast<unsigned int>(_transferBufSize[1]), false, false, false, false, _unitTimeoutMS);
		ImageWriteComplete(1, Constants::EVENT_WAIT_TIME);
		break;
	case ISLM::SLMParams::WRITE_FIRST_BUFFER:
		if (size != _transferBufSize[0])
		{
			_transferBuf[0] = (unsigned char*)realloc(_transferBuf[0], size);
			_transferBufSize[0] = size;
		}
		SAFE_MEMCPY(_transferBuf[0], _transferBufSize[0], pBuffer);
		break;
	case ISLM::SLMParams::SET_TRANSIANT_BUFFER:
		if (size != _transferBufSize[1])
		{
			_transferBuf[1] = (unsigned char*)realloc(_transferBuf[1], size);
			_transferBufSize[1] = size;
		}
		SAFE_MEMCPY(_transferBuf[1], _transferBufSize[1], pBuffer);
		ret = SetIntermediateBuffer(_transferBuf[1], _transferBufSize[1]);
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
		//write first bmp
		if (1 < _bufferCount)
		{
			SetParam(ISLM::SLMParams::ARRAY_ID, 0);
			SetIntermediateBuffer(_transferBuf[0], _transferBufSize[0]);
		}
		Write_image(1, _transferBuf[0], static_cast<unsigned int>(_transferBufSize[0]), false, false, false, false, _unitTimeoutMS);
		ImageWriteComplete(1, Constants::EVENT_WAIT_TIME);

		//set current index for on-board buffer,
		//keep -1 if no updateSLM invoked, then _transferBuf[0] would be current
		_slmTempBuf.SetIndex(-1);
	}
	return TRUE;
}

long MeadowlarkPDM::StopSLM()
{
	Stop_sequence();
	return TRUE;
}

long MeadowlarkPDM::UpdateSLM(long arrayID)
{
	Write_image(1, _intermediateBuf[arrayID].GetMem(), _intermediateBuf[arrayID].GetSize(), true, false, false, false, _slmTimeout);
	ImageWriteComplete(1, _slmTimeout);

	//keep current index:
	_slmTempBuf.SetIndex(arrayID);
	return TRUE;
}

long MeadowlarkPDM::GetLastErrorMsg(wchar_t* msg, long size)
{
	string str = Get_last_error_message();
	StringCbPrintfW(msg, size, StringToWString(str).c_str());
	return TRUE;
}


/// ***		Private Functions	*** ///

void MeadowlarkPDM::ReleaseMem()
{
	ReleaseTransientBuf();
}

void MeadowlarkPDM::ReleaseTransientBuf()
{
	//release transient buffer
	for (int i = 0; i < MAX_ARRAY_CNT; i++)
	{
		_intermediateBuf[i].ReallocMemChk(0);
	}

	//release first/temp pattern buffer
	SAFE_DELETE_MEMORY(_transferBuf[0]);
	SAFE_DELETE_MEMORY(_transferBuf[1]);
	_transferBufSize[0] = _transferBufSize[1] = 0;
	_slmTempBuf.ReallocMemChk(0);
}

void MeadowlarkPDM::SetDefault()
{
	_errMsg[0] = 0;
	_pSlmName = "";
	_bufferCount = 1;
	_arrayOrFileID = 0;
	_overDrive = 0;
	ReleaseTransientBuf();
}

long MeadowlarkPDM::SetIntermediateBuffer(unsigned char* mem, size_t msize)
{
	if ((NULL == mem) || (0 == msize))
		return FALSE;

	//persist image buffers
	if (msize != _intermediateBuf[_arrayOrFileID].GetSize())
		_intermediateBuf[_arrayOrFileID].ReallocMemChk((DWORD)msize);
	if (NULL == _intermediateBuf[_arrayOrFileID].GetMem())
	{
		_slmTempBuf.ReallocMemChk(0);
		goto FALSE_RETURN;
	}
	SAFE_MEMCPY(_intermediateBuf[_arrayOrFileID].GetMem(), msize, mem);
	return TRUE;

FALSE_RETURN:
	return FALSE;
}
