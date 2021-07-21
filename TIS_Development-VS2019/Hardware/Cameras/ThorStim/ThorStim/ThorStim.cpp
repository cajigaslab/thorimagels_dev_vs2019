// ThorStim.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include "math.h"
#include "Strsafe.h"
#include "ThorStim.h"
#include "ThorStimSetupXML.h"

ThorStim::ThorStim()
{
	_numCam = _numDigiLines = 0;
	_counter = "";
	_activeLoadCount = Constants::ACTIVE_LOAD_BLKSIZE_DEFAULT;

	for(long i=0; i<MAX_GG_POCKELS_CELL_COUNT; i++)
	{
		_paramsPty.pockelsMinVoltage[i] = 0.0;
		_paramsPty.pockelsMaxVoltage[i] = 0.0;
		_pockelsPowerLevel[i] = 0.0;
		_pockelsMinVoltage[i] = 0.0;
		_pockelsMaxVoltage[i] = 1.0;
		_pockelsLine[i] = "";
		_pockelsResponseType[i] = PockelsResponseType::LINEAR_RESPONSE;
	}
}

ThorStim::~ThorStim()
{
	_instanceFlag = false;
}

bool ThorStim:: _instanceFlag = false;

std::auto_ptr<ThorStim> ThorStim::_single(new ThorStim());

HMODULE ThorStim::hDLLInstance = NULL;

std::unique_ptr<WaveformBuilderDLL> waveformBuilder(new WaveformBuilderDLL(L".\\Modules_Native\\GeometryUtilitiesCPP.dll"));

wchar_t message[_MAX_PATH];
char logMsg[_MAX_PATH];	//used for logging, use message for GetLastErrorMsg instead

std::unique_ptr<BoardInfoNI> boardInfoNI;
std::auto_ptr<ThorStimXML> settings;
std::string ThorStim::_pockelsLineStr;
std::string ThorStim::_digiLineStr;
std::string ThorStim::_counter;
std::string ThorStim::_triggerIn;
std::unique_ptr<ImageWaveformBuilderDLL> ImageWaveformBuilder(new ImageWaveformBuilderDLL(L".\\Modules_Native\\GeometryUtilitiesCPP.dll"));
int ThorStim::_digiBleachSelect = 0x0;
int ThorStim::_numPockelsLines = 0x0;
long ThorStim::_activeLoadMS = 1;
long ThorStim::_sampleRateHz = 250000;
long ThorStim::_triggerMode = ICamera::SW_FREE_RUN_MODE;
long ThorStim::_frameCount = 1;
long ThorStim::_driverType = WaveformDriverType::WaveformDriver_NI;
HANDLE ThorStim::_hStopAcquisition = CreateEvent(NULL, true, false, NULL);  //2nd parameter "true" so it needs manual "Reset" after "Set (signal)" event
HANDLE ThorStim::_hThreadStopped = CreateEvent(NULL, true, true, NULL);
HANDLE ThorStim::_hHardwareTriggerInEvent = CreateEvent(NULL, false, false, NULL);


ThorStim *ThorStim::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorStim());
		_instanceFlag = true;
	}
	return _single.get();
}


long ThorStim::FindCameras(long &cameraCount)
{
	_numCam = cameraCount = 0;
	settings.reset(new ThorStimXML());
	if (TRUE == settings->OpenConfigFile())
	{
		try
		{
			if(FALSE == settings->GetConfigures(_driverType, _activeLoadMS, _activeLoadCount, _sampleRateHz))
			{
				StringCbPrintfW(message,_MAX_PATH,L"GetConfigures from Thor%sSettings failed", ThorStimXML::_libName);
				LogMessage(message,ERROR_EVENT);
				return FALSE;
			}
			_activeLoadMS = max(1, _activeLoadMS);
			_activeLoadCount = max(1, _activeLoadCount);
			_sampleRateHz *= Constants::KHZ;
			switch ((WaveformDriverType)_driverType)
			{
			case WaveformDriverType::WaveformDriver_NI:
				if (FALSE == CheckConfigNI())
					return FALSE;
				break;
			case WaveformDriverType::WaveformDriver_ThorDAQ:
			default:
				return FALSE;
			}
		}
		catch(...)
		{
			StringCbPrintfW(message,_MAX_PATH, L"Thor%s error on data structure configuration.", ThorStimXML::_libName);
			LogMessage(message, ERROR_EVENT);
			return FALSE;
		}
		//return device found when dll exist, 
		//but revoke tasks at preflight if invalid configuration
		StringCbPrintfW(message,_MAX_PATH, L"");
		_numCam = cameraCount = 1;
	}
	return TRUE;
}

long ThorStim::SelectCamera(const long camera)
{
	if((camera < 0) || (camera >= _numCam))
	{
		return FALSE;
	}

	return TRUE;
}

long ThorStim::TeardownCamera()
{
	CloseNITasks();

	bool doSet = false;
	for (int i = 0; i < MAX_GG_POCKELS_CELL_COUNT; i++)
	{
		if (_paramsPty.pockelsMinVoltage[i] != _pockelsMinVoltage[i] || _paramsPty.pockelsMaxVoltage[i] != _pockelsMaxVoltage[i])
		{
			//properties changed by setter, do update settings
			doSet = true;
			break;
		}
	}
	if (doSet)
	{
		if(FALSE == settings->SetModulations(_pockelsMinVoltage[0],_pockelsMaxVoltage[0],_pockelsMinVoltage[1],_pockelsMaxVoltage[1],_pockelsMinVoltage[2],_pockelsMaxVoltage[2],_pockelsMinVoltage[3],_pockelsMaxVoltage[3]))
		{
			StringCbPrintfW(message,_MAX_PATH,L"SetModulations from Thor%sSettings failed", ThorStimXML::_libName);
			LogMessage(message,ERROR_EVENT);
		}
	}
	return TRUE;
}

long ThorStim::PreflightAcquisition(char * pDataBuffer)
{
	//[TaskMaster] reset counts before setting up NI tasks:
	ThorStim::_finishedCycleCnt = ThorStim::_triggeredCycleCnt = 0;
	for (int i = 0; i < SignalType::SIGNALTYPE_LAST; i++)
	{
		ThorStim::_currentIndex[i] = ThorStim::_totalLength[i] = 0;
	}
	//force trigger first if single frame in trigger each:
	if((1 == _frameCount) && (ICamera::HW_MULTI_FRAME_TRIGGER_EACH == _triggerMode))
	{
		_triggerMode = ICamera::HW_MULTI_FRAME_TRIGGER_FIRST;
	}
	//check pre-capture status:
	if (1 < _frameCount)
	{
		ThorStim::_precaptureStatus = PreCaptureStatus::PRECAPTURE_WAVEFORM_MID_CYCLE;
	}
	//reset params to force rebuild waveform:
	ImageWaveformBuilder->ResetGGalvoWaveformParams();
	return TRUE;
}

long ThorStim::SetupAcquisition(char * pDataBuffer)
{
	return SetupProtocol();
}

long ThorStim::StartAcquisition(char * pDataBuffer)
{
	ResetEvent(_hStopAcquisition);
	ResetEvent(_hThreadStopped);
	_waveformTaskStatus = ICamera::STATUS_BUSY;
	return SyncCustomWaveformOnOff(true);
}

long ThorStim::StatusAcquisition(long &status)
{
	status = _waveformTaskStatus;
	return TRUE;
}

long ThorStim::StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame)
{
	return TRUE;
}

long ThorStim::CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	return TRUE;
}

long ThorStim::PostflightAcquisition(char * pDataBuffer)
{
	//force the hardware trigger event if the post flight function is called
	SetEvent(_hHardwareTriggerInEvent);
	ResetEvent(_hHardwareTriggerInEvent);

	WaveformModeFinished();

	PostflightProtocol();

	//reset params:
	_precaptureStatus = PreCaptureStatus::PRECAPTURE_DONE;
	return TRUE;
}

long ThorStim::GetLastErrorMsg(wchar_t * msg, long size)
{	
	wcsncpy_s(msg,size,message,_MAX_PATH);
	return TRUE;
}

long ThorStim::CheckConfigNI()
{
	std::string compStr[ThorStimXML::NUM_WAVEFORM_ATTRIBUTES];

	//find out all NI boards' info
	boardInfoNI.get()->getInstance()->GetAllBoardsInfo();

	// get pockels lines
	if(FALSE == settings->GetModulations(_counter, _triggerIn, 
		_pockelsLine[0], _pockelsMinVoltage[0], _pockelsMaxVoltage[0], _pockelsResponseType[0],
		_pockelsLine[1], _pockelsMinVoltage[1], _pockelsMaxVoltage[1], _pockelsResponseType[1],
		_pockelsLine[2], _pockelsMinVoltage[2], _pockelsMaxVoltage[2], _pockelsResponseType[2],
		_pockelsLine[3], _pockelsMinVoltage[3], _pockelsMaxVoltage[3], _pockelsResponseType[3]))
	{
		StringCbPrintfW(message,_MAX_PATH,L"GetModulations from Thor%sSettings failed, no gap line is allowed.", ThorStimXML::_libName);
		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}

	//backup param properties
	for (int i = 0; i < MAX_GG_POCKELS_CELL_COUNT; i++)
	{
		_paramsPty.pockelsMinVoltage[i] = _pockelsMinVoltage[i];
		_paramsPty.pockelsMaxVoltage[i] = _pockelsMaxVoltage[i];
	}

	std::map<std::string, int> devMaps;
	std::map<bool, int> rtsiAvailable;
	rtsiAvailable[1] = rtsiAvailable[0] = 0;

	//verify clock rate
	_sampleRateHz = (string::npos != GetNIDeviceProductType(GetDevIDName(_counter)).find("6321") && RATE6321 < _sampleRateHz) ? RATE6321 : _sampleRateHz;

	//verify counter
	compStr[0] = GetDevIDName(_counter);
	devMaps[compStr[0]] += 1;
	BoardInfo* bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(compStr[0]);
	if (NULL != bInfo)
	{
		if (FALSE == boardInfoNI.get()->getInstance()->VerifyLineNI(bInfo, LineTypeNI::COUNTER, _counter))
		{
			StringCbPrintfW(message,_MAX_PATH,L"GetModulations from Thor%sSettings failed, counter not available.", ThorStimXML::_libName);
			LogMessage(message,ERROR_EVENT);
			return FALSE;
		}
		rtsiAvailable[1 == bInfo->rtsiConfigure]++;
	}

	//verify trigger
	compStr[0] = GetDevIDName(_triggerIn);
	devMaps[compStr[0]] += 1;
	bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(compStr[0]);
	if (NULL != bInfo)
	{
		if (FALSE == boardInfoNI.get()->getInstance()->VerifyLineNI(bInfo, LineTypeNI::TERMINAL, _triggerIn))
		{
			StringCbPrintfW(message,_MAX_PATH,L"GetModulations from Thor%sSettings failed, triggerIn not available.", ThorStimXML::_libName);
			LogMessage(message,ERROR_EVENT);
			return FALSE;
		}
		rtsiAvailable[1 == bInfo->rtsiConfigure]++;
	}

	//verify analog lines
	_numPockelsLines = 0;
	_pockelsLineStr = "";
	for (int i = 0, j = 0; i < MAX_GG_POCKELS_CELL_COUNT; i++)
	{
		bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_pockelsLine[i]));
		if (NULL != bInfo)
		{
			if (FALSE == boardInfoNI.get()->getInstance()->VerifyLineNI(bInfo, LineTypeNI::ANALOG_OUT, _pockelsLine[i]))
			{
				StringCbPrintfW(message,_MAX_PATH,L"GetModulations from Thor%sSettings failed, pockelsLine(%d) not available.", ThorStimXML::_libName, i+1);
				LogMessage(message,ERROR_EVENT);
				_pockelsLine[i] = "";
				return FALSE;
			}
			if (0 < _pockelsLineStr.size()) _pockelsLineStr += ",";
			_pockelsLineStr += _pockelsLine[i].c_str();
			rtsiAvailable[1 == bInfo->rtsiConfigure]++;
			_numPockelsLines++;
			j++;
		}
		compStr[i] = GetDevIDName(_pockelsLine[i]);
		if (0 < compStr[i].size())
		{
			devMaps[compStr[i]] += 1;
			_sampleRateHz = (string::npos != GetNIDeviceProductType(compStr[i]).find("6321") && RATE6321 < _sampleRateHz) ? RATE6321 : _sampleRateHz;
		}
		//check if different devices
		if ((0 < i) && 0 < compStr[i].size() && 0 != compStr[i].compare(compStr[i-1]))
		{
			StringCbPrintfW(message,_MAX_PATH,L"PockelsLines have to be on the same device.");
			LogMessage(message,ERROR_EVENT);
			return FALSE;
		}
	}

	//get digital lines, set bitwise line selections
	//keep the first dummy line trigger for future application
	if(FALSE == settings->GetWaveform(&_digiLines))
	{
		StringCbPrintfW(message,_MAX_PATH,L"GetWaveform from Thor%sSettings failed", ThorStimXML::_libName);
		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}
	_numDigiLines = 1;	//include dummy line for future application
	_digiBleachSelect = 0x1;
	_digiLineStr = "";
	//verify pockels digital lines count compatible with pockels lines count
	for (int i = 0; i < MAX_GG_POCKELS_CELL_COUNT; i++)
	{
		BLEACHSCAN_DIGITAL_LINENAME eEnum = BLEACHSCAN_DIGITAL_LINENAME::DIGITAL_LINENAME_LAST;
		string eStr = "POCKEL_DIG";
		eStr = (0 == i) ? eStr : eStr + "_" + std::to_string(i);
		if (EnumString<BLEACHSCAN_DIGITAL_LINENAME>::To(eEnum, eStr))
			_digiLines[eEnum-1] = (0 >= _pockelsLine[i].length()) ? "" : _digiLines[eEnum-1];
	}
	//verity pockels digital lines
	for (int i = 0, j = 0; i < ThorStimXML::NUM_WAVEFORM_ATTRIBUTES; i++)
	{
		bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_digiLines[i]));
		if (NULL != bInfo)
		{
			if (FALSE == boardInfoNI.get()->getInstance()->VerifyLineNI(bInfo, LineTypeNI::DIGITAL_IN, _digiLines[i]))
			{
				StringCbPrintfW(message,_MAX_PATH,L"GetWaveform from Thor%sSettings failed, digital line(%d) not available.", ThorStimXML::_libName, i+1);
				LogMessage(message,ERROR_EVENT);
				return FALSE;
			}
			if (0 == j)	_digiLineStr = GetDevIDName(_digiLines[i]) + "/port0/line6";
			_digiLineStr += ",";
			_digiLineStr += _digiLines[i].c_str();
			_digiBleachSelect |= (0x1 << (i+1));
			rtsiAvailable[1 == bInfo->rtsiConfigure]++;
			_numDigiLines++;
			j++;
		}
		compStr[i] = GetDevIDName(_digiLines[i]);
		if (0 < compStr[i].size())	devMaps[compStr[i]]++;
		//check if different devices
		if (0 < i && 0 < compStr[i].size() && 0 < compStr[i-1].size() && 0 != compStr[i].compare(compStr[i-1]))
		{
			StringCbPrintfW(message,_MAX_PATH,L"Waveform digital lines have to be on the same device.");
			LogMessage(message,ERROR_EVENT);
			return FALSE;
		}
	}
	if (1 < devMaps.size() && 1 < rtsiAvailable[0])
	{
		StringCbPrintfW(message,_MAX_PATH,L"RTSI is not configured, all must be on the same device.");
		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}
