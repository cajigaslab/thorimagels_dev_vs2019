// ThorMesoScanSimulator.cpp : Defines the exported functions for the DLL application.
//

#include "ThorMesoScanSimulator.h"
#include "Strsafe.h"
#include "stb_font_consolas_40_usascii.h"
#include <ctime>
#include <algorithm>
#include <filesystem>
#include <Shlobj.h>
#include "..\..\..\Common\BinaryImageDataUtilities\GenericImage.h"

void Output(const wchar_t* szFormat, ...)
{
	wchar_t szBuff[1024];
	va_list arg;
	va_start(arg, szFormat);
	_vsnwprintf_s(szBuff, sizeof(szBuff), szFormat, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}

void StaticImageComplete(ImageCompleteStates state, ChanBufferInfo info, FrameROI roi, unsigned short* buffer, unsigned int size)
{
	ThorMesoScanSimulator::getInstance()->ImageComplete(state, info, roi, buffer, size);
}

wchar_t message[_MAX_PATH];
static stb_fontchar  fontdata[STB_SOMEFONT_NUM_CHARS];
static unsigned char fontpixels[STB_SOMEFONT_BITMAP_HEIGHT][STB_SOMEFONT_BITMAP_WIDTH];

// This is the constructor of a class that has been exported.
// see ThorMesoScanSimulator.h for the class definition
ThorMesoScanSimulator::ThorMesoScanSimulator() :
	MAX_PIXEL(17024),	//16384 [4Ch], 32768 [1Ch]
	MIN_PIXEL(64),
	DEFAULT_PIXEL(512)
{
	_cameraConfig = new CameraConfig();
	_waveformParams = new WaveformParams;

	_runningHandle = CreateEvent(NULL, true, false, NULL);
	_stopHandle = CreateEvent(NULL, true, false, NULL);
	_actualStopHandle = CreateEvent(NULL, true, true, NULL);
	GetSimulatorImages();
	STB_SOMEFONT_CREATE(fontdata, fontpixels, STB_SOMEFONT_BITMAP_HEIGHT);
	_expLoader.reset(new LoadMeso);
	InitCallBack(&StaticImageComplete);

	_acquireStatus = (int)ICamera::STATUS_BUSY;
	_indexOfLastCompletedFrame = _indexOfLastCopiedFrame = -1;
	_bufferInfo.scanAreaCount = _bufferInfo.sizeX = _bufferInfo.sizeY = 0;
	_bufferInfo.bufferSize = _bufferInfo.dmaCount = 0;
	for (int i = 0; i < MAX_DMABUFNUM; i++) { _pFrmDllBuffer[i] = NULL;	}
	_pixelX = _pixelY = DEFAULT_PIXEL;
	_scanMode = _triggerMode = _isLivingMode = 0;
}

ThorMesoScanSimulator::~ThorMesoScanSimulator()
{
	for (size_t i = 0; i < _simulatorImages.size(); i++)
	{
		if (_simulatorImages.at(i) != NULL)
		{
			delete (_simulatorImages.at(i));
			_simulatorImages.at(i) = NULL;
		}

	}
	_simulatorImages.clear();

	if (_cameraConfig != NULL)	delete _cameraConfig;
	if (_scan != NULL)	delete _scan;
	if (_waveformParams != NULL)	delete _waveformParams;
	SAFE_DELETE_HANDLE(_hThread);
	SAFE_DELETE_HANDLE(_runningHandle);
	SAFE_DELETE_HANDLE(_stopHandle);
	SAFE_DELETE_HANDLE(_actualStopHandle);
}

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

once_flag ThorMesoScanSimulator::_onceFlag;
unique_ptr<ThorMesoScanSimulator> ThorMesoScanSimulator::_single;
CRITICAL_SECTION ThorMesoScanSimulator::_accessSection;

ThorMesoScanSimulator * ThorMesoScanSimulator::getInstance()
{
	std::call_once(_onceFlag,
		[] {
			_single.reset(new ThorMesoScanSimulator);
	});
	return _single.get();
}

void ThorMesoScanSimulator::LogMessage(wchar_t *logMsg,long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

long ThorMesoScanSimulator::FindCameras(long &cameraCount) 
{
	cameraCount = 1;
	::InitializeCriticalSection(&_accessSection);
	return TRUE;
}

long ThorMesoScanSimulator::SelectCamera(const long camera) { return TRUE; }

long ThorMesoScanSimulator::TeardownCamera() 
{
	_bufferInfo.scanAreaCount = _bufferInfo.sizeX = _bufferInfo.sizeY = 0;
	_bufferInfo.bufferSize = _bufferInfo.dmaCount = 0;
	for (int j = 0; j < MAX_DMABUFNUM; j++)
	{
		if (NULL != _pFrmDllBuffer[j])
		{
			SAFE_DELETE_MEMORY(_pFrmDllBuffer[j]);
		}
	}
	StopScan();
	ClearScan();

	::DeleteCriticalSection(&_accessSection);
	return TRUE;
}

long ThorMesoScanSimulator::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault) 
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
			paramMin = ICamera::RESONANCE_GALVO_GALVO;
			paramMax = ICamera::RESONANCE_GALVO_GALVO;
			paramDefault = ICamera::RESONANCE_GALVO_GALVO;
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
	case ICamera::PARAM_LSM_PIXEL_X:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_PIXEL;
			paramMax = MAX_PIXEL;
			paramDefault = DEFAULT_PIXEL;
			paramReadOnly = FALSE;
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_Y:
		{
			paramType = ICamera::TYPE_LONG;
			paramAvailable = TRUE;
			paramMin = MIN_PIXEL;
			paramMax = MAX_PIXEL;
			paramDefault = DEFAULT_PIXEL;
			paramReadOnly = FALSE;
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
	case ICamera::PARAM_MESO_SCAN_INFO:
		{
			paramType = ICamera::TYPE_BUFFER;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
		}
		break;
	default:
		{
			ret = FALSE;
			if (_cameraConfig->IsUIntValue((ICamera::Params)paramID))
			{
				paramType = ICamera::TYPE_LONG;
				uint32_t min, max, value;
				ret = _cameraConfig->GetParameterRange((ICamera::Params)paramID, min, max, value);
				paramMin = min; paramMax = max; paramDefault = value;
			}
			else
			{
				paramType = ICamera::TYPE_DOUBLE;
				ret = _cameraConfig->GetParameterRange((ICamera::Params)paramID, paramMin, paramMax, paramDefault);
			}

			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			break;
		}
	}
	return ret;
}
long ThorMesoScanSimulator::SetParam(const long paramID, const double param) 
{
	long ret = TRUE;
	switch (paramID)
	{
	case ICamera::PARAM_LSM_PIXEL_X:
		{
			int factor = (_cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_SCANMODE) == TWO_WAY_SCAN) ? 2 : 1;

			if ((param >= MIN_PIXEL) && (param <= MAX_PIXEL / factor))
			{
				_pixelX = static_cast<long>(param);
				if(ICamera::SQUARE == _cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_AREAMODE))
				{
					_pixelY = _pixelX;
				}
			}
		}
		break;
	case ICamera::PARAM_LSM_PIXEL_Y:
		{
			int factor = (_cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_SCANMODE) == TWO_WAY_SCAN) ? 2 : 1;

			if(ICamera::SQUARE == _cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_AREAMODE))
			{
				_pixelY = _pixelX;
			}
			else if(ICamera::LINE == _cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_AREAMODE))
			{
				_pixelY = 1;
			}
			else
			{
				if ((param >= MIN_PIXEL) && (param <= MAX_PIXEL / factor))
				{
					_pixelY = static_cast<long>(param);
				}
			}
		}
		break;
	case ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE:
		{
			if (TRUE == param)
			{
				if(TRUE == _expLoader.get()->LoadExperimentXML())
				{
					::EnterCriticalSection(&_accessSection);

					StopScan();
					ClearScan();
					for (int i = 0; i < _expLoader.get()->Scans.size(); i++)
					{
						SetParamBuffer(ICamera::PARAM_MESO_SCAN_INFO, (char*)_expLoader.get()->Scans[i], sizeof(_expLoader.get()->Scans[i]));
					}
					ret = ResolveScanInfo();

					::LeaveCriticalSection(&_accessSection);
				}
			}
		}
		break;
	case PARAM_LSM_POCKELS_FIND_MIN_MAX_0:
	case PARAM_POCKELS_STOP_CALIBRATION:
		return CameraStates::POCKELS_CALIBRATION_FAILED_SIMULATOR;
	default:
		{
			ret = FALSE;
			if (_cameraConfig->IsUIntValue((ICamera::Params)paramID))
			{
				return _cameraConfig->SetParameter((ICamera::Params)paramID, (uint32_t)param);
			}
			else
			{
				return _cameraConfig->SetParameter((ICamera::Params)paramID, param);
			}
			break;
		}
	}
	return ret;
}
long ThorMesoScanSimulator::GetParam(const long paramID, double &param)
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
			param = ICamera::RESONANCE_GALVO_GALVO;
		}
		break;
	case ICamera::PARAM_MESO_STRIP_COUNT:
		{
			param = _stripCount;
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
	default:
		{
			ret = FALSE;
			if (_cameraConfig->IsUIntValue((ICamera::Params)paramID))
			{
				unsigned int value;
				ret = _cameraConfig->GetParameter((ICamera::Params)paramID, value);
				param = value;
			}
			else
			{
				ret = _cameraConfig->GetParameter((ICamera::Params)paramID, param);
			}
			break;
		}
	}
	return ret;
}
long ThorMesoScanSimulator::PreflightAcquisition(char * pDataBuffer) 
{
	if (_cameraConfig == NULL || _scan == NULL)
	{
		StringCbPrintfW(message,_MAX_PATH, L"Invalid parameters.");
		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}
	StopScan();
	return ResolveScanInfo();
}
long ThorMesoScanSimulator::SetupAcquisition(char * pDataBuffer)
{
	return TRUE;
}
long ThorMesoScanSimulator::StartAcquisition(char * pDataBuffer)
{
	if (WaitForSingleObject(_runningHandle, 0) == WAIT_OBJECT_0)
	{
		StringCbPrintfW(message,_MAX_PATH, L"Can not start acquisition while running.");
		LogMessage(message,WARNING_EVENT);
		return TRUE;
	}

	//could competing with setup stripes, control by critical section
	::EnterCriticalSection(&_accessSection);
	_acquireStatus = (int)ICamera::STATUS_BUSY;
	SAFE_DELETE_HANDLE(_hThread);
	_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartThreadFunc, this, 0, NULL);
	if (_hThread != NULL)
	{
		SetEvent(_runningHandle);
		ResetEvent(_stopHandle);
		ResetEvent(_actualStopHandle);
	}
	::LeaveCriticalSection(&_accessSection);
	return TRUE;
}
long ThorMesoScanSimulator::StartThreadFunc(LPVOID pParam)
{
	ThorMesoScanSimulator *pObj = reinterpret_cast<ThorMesoScanSimulator*>(pParam);
	return pObj->StartAsync();
}
long ThorMesoScanSimulator::StartAsync()
{
	unsigned short value = 39;
	StripInfo * lastStrip = NULL;
	while (WAIT_OBJECT_0 != WaitForSingleObject(_stopHandle, 0))
	{
		if (_currentStrip == NULL)
		{
			//_imageCompleteCallback(SCAN_COMPLETE, {}, {}, NULL, 0);
			ChanBufferInfo fInfo = {}; FrameROI roi = {};
			_imageCompleteCallback(SCAN_COMPLETE, fInfo, roi, NULL, 0);
			break;
		}
		unsigned int size = _stripWidth*_currentStrip->YSize;
		unsigned short* buffer = (unsigned short*)malloc(size * sizeof(unsigned short));

		bool isPrintInfo = false;
		if (lastStrip == NULL || _currentStrip == _currentStrip->nextStrip)
		{
			isPrintInfo = true;
		}
		else
		{
			if (_currentStrip->ChanBufInfo[0].ScanAreaID != lastStrip->ChanBufInfo[0].ScanAreaID ||
				_currentStrip->ChanBufInfo[0].ZID != lastStrip->ChanBufInfo[0].ZID ||
				_currentStrip->ChanBufInfo[0].StreamID != lastStrip->ChanBufInfo[0].StreamID ||
				_currentStrip->ChanBufInfo[0].TimeID != lastStrip->ChanBufInfo[0].TimeID ||
				_currentStrip->FrameROI.x == 0
				)
			{
				isPrintInfo = true;
			}
			else
			{
				isPrintInfo = false;
			}
		}
		for (unsigned int c = 0; c < _channels.size(); c++)
		{
			if (_simulatorImages.size() > c)
			{
				_simulatorImages[c]->GetImageBuffer(_currentStrip, buffer, _stripWidth);
			}
			else
			{
				memset(buffer, value, size * sizeof(unsigned short));
				value = (value + 8) % 64;
				if (isPrintInfo)
				{
					PrintInformation(_currentStrip, buffer, _currentStrip->ChanBufInfo[c].ChannelID);
				}
			}
			_imageCompleteCallback((_currentStrip->IsFrameEnd&&c == _channels.size() - 1) ? SCAN_IMAGE_STRIPEEND : SCAN_IMAGE, _currentStrip->ChanBufInfo[c], _currentStrip->FrameROI, buffer, size);
		}
		std::free(buffer);

		int timePerStrip = (int)(1000.0 / 12000.0 * _currentStrip->YSize /  2 *_waveformParams->NumberOfAverageFrame);
		if (timePerStrip == 0)
		{
			timePerStrip = 1;
		}
		_imageCompleteCallback(SCAN_BUSY,_currentStrip->ChanBufInfo[0], _currentStrip->FrameROI, NULL, 0);
		Sleep(timePerStrip);
		if (WaitForSingleObject(_stopHandle, 0) == WAIT_OBJECT_0)
		{
			StringCbPrintfW(message,_MAX_PATH, L"Stop scan.");
			LogMessage(message,INFORMATION_EVENT);
			//_imageCompleteCallback(SCAN_ABORT, {}, {}, NULL, 0);
			_imageCompleteCallback(SCAN_ABORT, _currentStrip->ChanBufInfo[0], _currentStrip->FrameROI, NULL, 0);
			break;
		}
		_mtx.lock();
		lastStrip = _currentStrip;
		_currentStrip = _currentStrip->nextStrip;
		_mtx.unlock();
	}
	ClearStripeList();
	ResetEvent(_runningHandle);
	SetEvent(_actualStopHandle);
	_acquireStatus = (int)ICamera::STATUS_READY;
	return TRUE;
}

long ThorMesoScanSimulator::StatusAcquisition(long &status)
{
	try
	{
		//for the first frame, return actual status in acquiring current frame for potential partial update
		if((0 > _indexOfLastCompletedFrame) || (_indexOfLastCopiedFrame == _indexOfLastCompletedFrame))
		{
			status = _acquireStatus;
		}
		else
		{
			//for the remaing frames check the index of the frame against the last copied frame,
			//last completed DMA buffer index is (_bufCompleteID-1) since _bufCompleteID is incremented after complete of one frame
			long long comparisonIndex = _bufferOrder.at(((_bufCompleteID-1+static_cast<long>(_bufferInfo.dmaCount))%static_cast<long>(_bufferInfo.dmaCount)));

			status = ICamera::STATUS_READY;

			//move the source pointer if the indexes don't match
			if(comparisonIndex - _indexOfLastCopiedFrame > _bufferInfo.scanAreaCount)
			{
				//If in live mode, change index to the last acquired image from the same scan area
				//this allows for a more realtime feel when displaying the latest image
				if ((int)ICamera::SW_FREE_RUN_MODE == _triggerMode)
				{
					_indexOfLastCopiedFrame = comparisonIndex - _bufferInfo.scanAreaCount + 1;
				}

				long long frameDifference = max(0,min(static_cast<long long>(_bufferInfo.dmaCount)-1, (comparisonIndex-_indexOfLastCopiedFrame)));
#ifdef LOGGING_ENABLED 
				StringCbPrintfW(_errMsg,_MAX_PATH,L"StatusProtocol: frameDifference %d comparisonIndex %d _indexOfLastCopiedFrame %d indexOfLastCompletedFrame %d",frameDifference,comparisonIndex,_indexOfLastCopiedFrame,_indexOfLastCompletedFrame);
				LogMessage(_errMsg,INFORMATION_EVENT);
#endif
				//if the index is beyond the dma buffer count, return an error.
				//Frames have been lost at this point
				if((0 < frameDifference) && (comparisonIndex-_indexOfLastCopiedFrame > static_cast<long long>(_bufferInfo.dmaCount) -1))
				{				
					StringCbPrintfW(message,_MAX_PATH,L"ThorMesoScanSimulator::%hs@%u Error: Overflow",__FUNCTION__, __LINE__);
					LogMessage(message,VERBOSE_EVENT);
					status = ICamera::STATUS_ERROR;
				}
			}			
		}
		return TRUE;
	}
	catch(...)
	{
		StringCbPrintfW(message,_MAX_PATH,L"ThorMesoScanSimulator::StatusAcquisition Unhandled exception");
		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}
}
long ThorMesoScanSimulator::StatusAcquisitionEx(long &status, long &indexOfLastCompletedFrame) { return 0; }

long ThorMesoScanSimulator::CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	try
	{
		//do not capture data if you are in the centering or bleaching scan mode
		if((ScanMode::CENTER == _scanMode) || (ScanMode::BLEACH_SCAN == _scanMode))
		{
			return FALSE;
		}
		//return without copy if user changed params in free-run mode
		if ((ICamera::SW_FREE_RUN_MODE == _triggerMode) && (WAIT_OBJECT_0 == WaitForSingleObject(_stopHandle, 0)))
		{
			return FALSE;
		}

		long long frameDifference = -1;		//0 or greater: a full frame is ready, otherwise: partial frame or busy in acquire
		unsigned short *pS =  NULL;

		if(_indexOfLastCopiedFrame == _indexOfLastCompletedFrame)
		{
			//wait for a short period of time since period update is a lot faster than actual frame rate
			if(_dmaMutex.try_lock())
			{
#if defined (LOGGING_ENABLED)
				//current is in acquire
				StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorMesoScanSimulator:%hs@%u: copying partial dma frame of %d", __FILE__, __LINE__, _bufCompleteID);
				LogMessage(_errMsg,INFORMATION_EVENT);
#endif
				//move the source to current acquiring dma buffer
				pS =  _pFrmDllBuffer[_bufCompleteID];
			}
		}
		else if (_indexOfLastCopiedFrame < _indexOfLastCompletedFrame)
		{
			//last completed DMA buffer index is (_bufCompleteID-1) since _bufCompleteID is incremented after complete of one frame
			long long comparisonIndex = _bufferOrder.at(((_bufCompleteID-1+static_cast<long>(_bufferInfo.dmaCount))%static_cast<long>(_bufferInfo.dmaCount)));

			//we are about to copy a new frame, increment the copied frame count
			_indexOfLastCopiedFrame++;

			frameDifference = max(0,min(static_cast<long long>(_bufferInfo.dmaCount)-1, (comparisonIndex-_indexOfLastCopiedFrame)));
#ifdef LOGGING_ENABLED 
			StringCbPrintfW(_errMsg,_MAX_PATH,L"CopyProtocol: frameDifference %d comparisonIndex %d indexOfLastCopiedFrame %d indexOfLastCompletedFrame %d",frameDifference,comparisonIndex,_indexOfLastCopiedFrame,_indexOfLastCompletedFrame);
			LogMessage(_errMsg,INFORMATION_EVENT);
#endif
			if((0 < frameDifference) && (comparisonIndex-_indexOfLastCopiedFrame > static_cast<long long>(_bufferInfo.dmaCount) - 1))
			{
				//this is an error message
				//indicating that a buffer overflow has occured
				StringCbPrintfW(message,_MAX_PATH,L"BUFFER OVERFLOW frameDifference %d is greater than dmabuffercount %d",comparisonIndex-_indexOfLastCopiedFrame,_bufferInfo.dmaCount-1);
				LogMessage(message,ERROR_EVENT);
			}
			else
			{
				//wait if the frame is locked, must copy completed frame
				_dmaMutex.lock();

				//move the source to the appropriate history buffer,
				//last completed DMA buffer index is (_bufCompleteID-1) since _bufCompleteID is incremented after complete of one frame
				pS = _pFrmDllBuffer[((_bufCompleteID-1+_bufferInfo.dmaCount-frameDifference)%_bufferInfo.dmaCount)];
			}
		}

		//copy buffer, allow copy current for partial frame update, acquired enabled channel only
		if(NULL != pS)
		{
			unsigned short *pD = (unsigned short*)pDataBuffer;
			std::vector<int> channels;
			std::transform(_channels.begin(),_channels.end(), std::back_inserter(channels),[](Channel const x) { return x.ChannelRefID; });
			//if(_imgPtyDll.rawSaveEnabledChannelOnly==FALSE)
			{
				GenericImage<unsigned short> sourceImage(_bufferInfo.sizeX,_bufferInfo.sizeY,1,1,static_cast<int>(_channels.size()),1,GenericImage<unsigned short>::INTERLACED_CHANNEL, std::vector<int>(), pS);
				GenericImage<unsigned short> destImage  (_bufferInfo.sizeX,_bufferInfo.sizeY,1,1,static_cast<int>(_channels.size()),1,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL, channels, pD);
				destImage.copyFrom(sourceImage);

				//CopyAcquisitionBufferIncludeDisabled(_bufferInfo.sizeX, _bufferInfo.sizeY, _totalChannelsForAllBoards, pS, pD);
			}
			//else
			//{
			//GenericImage<unsigned short> sourceImage(_bufferInfo.sizeX,_bufferInfo.sizeY,1,static_cast<int>(_scan->Channels.size()),1,GenericImage<unsigned short>::INTERLACED_CHANNEL, channels, pS);
			//GenericImage<unsigned short> destImage  (_bufferInfo.sizeX,_bufferInfo.sizeY,1,static_cast<int>(_scan->Channels.size()),1,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL,	channels, pD);
			//destImage.copyFrom(sourceImage);
			//}
			_dmaMutex.unlock();
		}
	}
	catch(...)
	{
		StringCbPrintfW(message,_MAX_PATH,L"ThorMesoScanSimulator CopyAcquisition error.");
		LogMessage(message,ERROR_EVENT);
		_dmaMutex.unlock();
	}
	return TRUE;
}

long ThorMesoScanSimulator::PostflightAcquisition(char * pDataBuffer) 
{
	StopScan();
	ClearScan();
	return TRUE;
}
long ThorMesoScanSimulator::GetLastErrorMsg(wchar_t * errMsg, long size) { return 0; }

long ThorMesoScanSimulator::SetParamString(const long paramID, wchar_t * str)
{
	long ret = FALSE;
	switch (paramID)
	{
	case ICamera::PARAM_MESO_EXP_PATH:
		_expLoader.get()->ExpPath = wstring(str);
		_expLoader.get()->LoadExperimentXML();
		for (int i = 0; i < _expLoader.get()->Scans.size(); i++)
		{
			SetParamBuffer(ICamera::PARAM_MESO_SCAN_INFO, (char*)_expLoader.get()->Scans[i], sizeof(_expLoader.get()->Scans[i]));
		}
		ret = TRUE;
		break;
	default:
		break;
	}

	return ret;
}

long ThorMesoScanSimulator::GetParamString(const long paramID, wchar_t * str, long size) 
{
	long ret = FALSE;

	switch(paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME:
		{
			wcscpy_s(str,20, L"ResGalvoGalvoSim"); //tempID[20] in SelectHardware
			ret = TRUE;
		}
		break;
	case ICamera::PARAM_MESO_EXP_PATH:
		{
			wcscpy_s(str,size, _expLoader.get()->ExpPath.c_str());
			ret = TRUE;
		}
		break;
	default:
		break;
	}

	return ret;

}
long ThorMesoScanSimulator::SetParamBuffer(const long paramID, char * buffer, long size)
{
	long ret = FALSE;
	switch (paramID)
	{
	case ICamera::PARAM_MESO_SCAN_INFO:
		{
			_scan = new Scan(*static_cast<Scan*>((void*)buffer));
			_scansCache.push_back(_scan);

			//calculate calibration value for field size
			_cameraConfig->SetParameter(ICamera::Params::PARAM_LSM_FIELD_SIZE_CALIBRATION, 
				static_cast<double>(_pixelX) * _scan->ScanConfig.PhysicalFieldSize / static_cast<double>(_scan->ScanConfig.StripLength) / static_cast<double>(_cameraConfig->FieldSize));

			ret = TRUE;
			break;
		}
	}

	return ret;
}
long ThorMesoScanSimulator::GetParamBuffer(const long paramID, char * buffer, long size)
{
	return TRUE;
}
long ThorMesoScanSimulator::InitCallBack(ImageCompleteCallback imageCompleteCallback) {
	_imageCompleteCallback = imageCompleteCallback;
	return TRUE;
}
long ThorMesoScanSimulator::GetMapParam(const long paramID, double inputValue, double & param)
{
	if (paramID == PARAM_MESO_TWOWAY_ALIGNMENT_SHIFT)
	{
		return _cameraConfig->GetTwoWayAlignmentPoint(inputValue, param);
	}
	else if (paramID == PARAM_MESO_RESONANT_FIELD_TO_VOLTAGE)
	{
		return _cameraConfig->GetResonantVoltage(inputValue, param);
	}
	return FALSE;
}
long ThorMesoScanSimulator::SetMapParam(const long paramID, double inputValue, double param)
{
	return 0;
}
long ThorMesoScanSimulator::ResolveScanInfo()
{
	long ret = TRUE;
	if (WaitForSingleObject(_runningHandle, 0) != WAIT_OBJECT_0)
	{
		_waveformParams->FieldWidth = _cameraConfig->MaxPosX;
		_waveformParams->FieldHeight = _cameraConfig->MaxPosY;
		_waveformParams->F2TGx1 = _cameraConfig->F2VGx1;
		_waveformParams->F2TGx2 = _cameraConfig->F2VGx2;
		_waveformParams->F2TGy = _cameraConfig->F2VGy;
		_waveformParams->SamplesPerLine = _cameraConfig->SamplesPerLine;
		_waveformParams->PockelDutyCycle = _cameraConfig->PockelDutyCycle;
		_waveformParams->PhysicalStripeWidth = _scan->ScanConfig.PhysicalFieldSize;
		_waveformParams->StripWidth = _scan->ScanConfig.StripLength;
		_waveformParams->DelayTimeGx1 = _cameraConfig->DelayTimeGx1;
		_waveformParams->DelayTimeGx2 = _cameraConfig->DelayTimeGx2;
		_waveformParams->DelayTimeGy = _cameraConfig->DelayTimeGy;
		_waveformParams->DelayTimeVC = _cameraConfig->DelayTimeVC;
		_waveformParams->DelayTimePC = _cameraConfig->DelayTimePC;
		_waveformParams->CenterShiftX = _cameraConfig->CenterShiftX;
		_waveformParams->CenterShiftY = _cameraConfig->CenterShiftY;
		_waveformParams->VCPointsPerLine = _cameraConfig->VCPointsPerLine;
		_waveformParams->CurveParameterA = _cameraConfig->CurveParamA;
		_waveformParams->CurveParameterB = _cameraConfig->CurveParamB;
		_waveformParams->NumberOfAverageFrame = (AverageMode::AVG_MODE_CUMULATIVE == _scan->ScanConfig.AverageMode) ? _scan->ScanConfig.NumberOfAverageFrame : 1;
		if (_firstStrip != NULL)
		{
			StripInfo::DeleteStripLoop(_firstStrip);
			_firstStrip = NULL;
			_currentStrip = NULL;
		}
	}

	//no more copy allowed after preflight by reset of indexes
	_indexOfLastCompletedFrame = _indexOfLastCopiedFrame = -1;
	_bufCompleteID = 0;
	_stripWidth = _waveformParams->StripWidth;
	_scanMode = _scan->ScanConfig.ScanMode;
	_triggerMode = _cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_TRIGGER_MODE);
	_isLivingMode = _scan->ScanConfig.IsLivingMode;
	_channels.clear();
	for (int i = 0; i < _scan->Channels.size(); i++)
	{
		Channel ch;
		memcpy_s(&ch,sizeof(Channel),_scan->Channels[i],sizeof(Channel));
		_channels.push_back(ch);
	}

	StripInfo* firstStripInLoop = NULL;
	StripInfo* endStripInLoop = NULL;
	_stripCount = 0;
	size_t  bufferSize = 0;
	long sizeX = 0, sizeY = 0;

	for (unsigned int i = 0; i < ((_isLivingMode) ? 1 : _cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_MULTI_FRAME_COUNT)); i++)
	{
		for (vector<ScanArea*>::iterator iter = _scan->ScanAreas.begin(); iter != _scan->ScanAreas.end(); ++iter)
		{
			ScanArea* scanArea = *iter;
			uint32_t stripXOffsetInPixel = 0;
			size_t saBufferSize = (_scan->Channels.size() * scanArea->SizeX * scanArea->SizeY * _scan->GetPixelBytes());
			if (bufferSize < saBufferSize)
			{
				sizeX = scanArea->SizeX;
				sizeY = scanArea->SizeY;
				bufferSize = saBufferSize;
			}
			for (int s = 1; s <= scanArea->SizeS; s++)
			{
				for (uint32_t zIndex = 0; zIndex < scanArea->SizeZ; zIndex++)
				{
					stripXOffsetInPixel = 0;
					while (stripXOffsetInPixel < scanArea->SizeX)
					{
						StripInfo* strip = new StripInfo();
						strip->XPos = scanArea->PositionX + stripXOffsetInPixel*_scan->XPixelSize;
						strip->YPos = scanArea->PositionY;
						strip->ZPos = scanArea->PositionZ;
						strip->XPhysicalSize = min(_waveformParams->PhysicalStripeWidth, scanArea->PhysicalSizeX - strip->XPos);
						strip->YPhysicalSize = scanArea->PhysicalSizeY;
						strip->ZPhysicalSize = 0;
						strip->XSize = min(_waveformParams->StripWidth, scanArea->SizeX - stripXOffsetInPixel);
						strip->YSize = scanArea->SizeY;
						/*
						strip->Power
						strip->ROIPower
						*/
						int c = 0;
						for (vector<Channel*>::iterator iter = _scan->Channels.begin(); iter != _scan->Channels.end(); iter++)
						{
							strip->ChanBufInfo[c].ScanID = _scan->ScanID;
							strip->ChanBufInfo[c].ScanAreaID = scanArea->ScanAreaID;
							strip->ChanBufInfo[c].StreamID = s;
							strip->ChanBufInfo[c].TimeID = _scan->ScanConfig.CurrentT;
							strip->ChanBufInfo[c].ZID = zIndex + 1;
							strip->ChanBufInfo[c].ChannelID = (*iter)->ChannelID;
							c++;
						}
						strip->FrameROI.x = stripXOffsetInPixel;
						strip->FrameROI.y = 0;
						strip->FrameROI.width = min(scanArea->SizeX - stripXOffsetInPixel, _waveformParams->StripWidth);
						strip->FrameROI.height = scanArea->SizeY;
						strip->FrameROI.frameWidth = scanArea->SizeX;
						strip->FrameROI.frameHeight = scanArea->SizeY;
						strip->SkipSignal = 0;
						strip->IncludeSignal = strip->YSize;
						if (endStripInLoop == NULL)
						{
							firstStripInLoop = strip;
							endStripInLoop = strip;
						}
						else
						{
							endStripInLoop->nextStrip = strip;
							strip->preStrip = endStripInLoop;
							endStripInLoop = strip;
						}
						stripXOffsetInPixel += _waveformParams->StripWidth;
						_stripCount++;
					}
				}
			}
		}
	}
	endStripInLoop->IsFrameEnd = true;
	if (_scan->ScanConfig.IsLivingMode)
	{
		//circular loop for live imaging
		endStripInLoop->nextStrip = firstStripInLoop;
		firstStripInLoop->preStrip = endStripInLoop;
	}	
	_mtx.lock();
	if (_currentStrip == NULL)
	{
		_firstStrip = firstStripInLoop;
		_currentStrip = firstStripInLoop;
	}
	else
	{
		if (_currentStrip->nextStrip != NULL)
			StripInfo::DeleteStripLoop(_currentStrip->nextStrip);
		_currentStrip->nextStrip = firstStripInLoop;
		firstStripInLoop->preStrip = _currentStrip;
	}
	_mtx.unlock();

	//limit DMA unit count based on buffer size [8-32MB,8][32-64MB,4][64MB,2]
	unsigned int dmaUnits = _cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_DMA_BUFFER_COUNT);	// GetAvailableMemorySize() / bufferSize;
	if ((0x800000 < bufferSize) && (0x2000000 > bufferSize))
	{
		dmaUnits = min(8, dmaUnits);
	}
	else if ((0x2000000 < bufferSize) && (0x4000000 > bufferSize))
	{
		dmaUnits = min(4, dmaUnits);
	}
	else if (0x4000000 < bufferSize)
	{
		dmaUnits = min(2, dmaUnits);
	}

	//*************************************************************//
	// reset dma buffers, make param copies for multi thread check //
	//*************************************************************//
	_dmaMutex.lock();
	try
	{
		size_t dmaCount = _scan->ScanAreas.size() * dmaUnits;
		if ((_bufferInfo.bufferSize != bufferSize) || (_bufferInfo.dmaCount != dmaCount) || (_bufferInfo.scanAreaCount != static_cast<long>(_scan->ScanAreas.size())))
		{
			for (unsigned int i = 0; i < dmaCount; i++)
			{
				_pFrmDllBuffer[i] = (unsigned short*) realloc(_pFrmDllBuffer[i], bufferSize);
				if(_pFrmDllBuffer[i] == NULL)
				{
					StringCbPrintfW(message,_MAX_PATH,L"ThorMesoScanSimulator unable to allocate pFrmDllBuffer size(%d) at dma(%u)", bufferSize, i);
					LogMessage(message,ERROR_EVENT);
				}
			}

			_bufferInfo.bufferSize = bufferSize;
			_bufferInfo.dmaCount = dmaCount;
			_bufferInfo.sizeX = sizeX;
			_bufferInfo.sizeY = sizeY;
			_bufferInfo.scanAreaCount = static_cast<long>(_scan->ScanAreas.size());
		}
		//blank the first since the rest will be updated by previous
		memset(_pFrmDllBuffer[0], 0x0, _bufferInfo.bufferSize);
		//<key, value>: <dma buffer index, completed frame index>
		_bufferOrder.clear();
		for (long i = 0; i < dmaCount; i++)
		{
			_bufferOrder.insert(pair<long, long long>(i, _indexOfLastCompletedFrame));
		}
	}
	catch(...)
	{
		LogMessage(L"ThorMesoScanSimulator ResolveScanInfo set dma buffer error.", ERROR_EVENT);
		ret = FALSE;
	}
	_dmaMutex.unlock();
	return ret;
}

long ThorMesoScanSimulator::SaveConfigration()
{
	return _cameraConfig->SaveConfigFile();
}

long ThorMesoScanSimulator::GetDeviceConfigration(const long paramID, void** param)
{
	long ret = FALSE;
	switch (paramID)
	{
	case ICamera::PARAM_MESO_CAMERA_CONFIG:
		{
			*param = (void*)_cameraConfig;
			ret = TRUE;
		}
		break;
	default:
		break;
	}
	return ret;
}

void PrintText(uint16_t *pDataBuffer, int x, int y, wchar_t *str,int width, int height)
{
	int           char_value = 0;
	stb_fontchar *cd = 0;
	int           index = 0;
	int           w = 0;
	int           h = 0;
	int           i = 0;
	int           j = 0;
	int           x_advance = 0;
	int           value = 0;

	if (pDataBuffer == 0) return;
	if (str == NULL) return;
	unsigned short * pImage = (unsigned short *)pDataBuffer;
	while (*str != 0)
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
		if ((x + x_advance >= width) || (y + h >= height))
			return;

		index = (y * (int)width) + x + x_advance;

		for (i = 0; i<h; i++)
		{
			for (j = 0; j<w; j++)
			{
				// assign image data value from font pixels
				value = fontpixels[i + cd->y0][j + cd->x0];

				if (value == 0) continue;
				//value = value | (value << 8);

				pImage[(index + j)] = 16383; //(value & 0x0FFF);
			}

			index += (int)width;
		}

		x_advance += cd->advance_int;
	}

	return;
}

void ThorMesoScanSimulator::PrintInformation(StripInfo* stripe, uint16_t* buffer, int cId)
{
	// scan info
	memset(message, 0, sizeof(message));
	wsprintf(message, L"SC : %d", stripe->ChanBufInfo[0].ScanID);
	PrintText(buffer, 10, 10, message, stripe->XSize, _currentStrip->YSize);

	memset(message, 0, sizeof(message));
	wsprintf(message, L"SA : %d", stripe->ChanBufInfo[0].ScanAreaID);
	PrintText(buffer, 10, 50, message, stripe->XSize, _currentStrip->YSize);

	memset(message, 0, sizeof(message));
	wsprintf(message, L"C : %d", cId);
	PrintText(buffer, 10, 90, message, stripe->XSize, _currentStrip->YSize);

	memset(message, 0, sizeof(message));
	wsprintf(message, L"Z : %d", stripe->ChanBufInfo[0].ZID);
	PrintText(buffer, 10, 130, message, stripe->XSize, _currentStrip->YSize);

	memset(message, 0, sizeof(message));
	wsprintf(message, L"S : %d", stripe->ChanBufInfo[0].StreamID);
	PrintText(buffer, 10, 170, message, stripe->XSize, _currentStrip->YSize);

	memset(message, 0, sizeof(message));
	wsprintf(message, L"T : %d", stripe->ChanBufInfo[0].TimeID);
	PrintText(buffer, 10, 210, message, stripe->XSize, _currentStrip->YSize);

	time_t now = time(0);
	struct tm timeinfo;
	localtime_s(&timeinfo, &now);

	memset(message, 0, sizeof(message));
	wsprintf(message, L"%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	PrintText(buffer, 0, 250, message, stripe->XSize, _currentStrip->YSize);
}

bool ThorMesoScanSimulator::GetSimulatorImages()
{
	bool result = false;
	char buffer[_MAX_PATH];
	memset(buffer, 0, _MAX_PATH);
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, buffer)))
	{
		string pathA = string(buffer) + "\\Thorlabs\\MesoscopeSys\\SimulatorA.jpg";
		if (INVALID_FILE_ATTRIBUTES != GetFileAttributesA(pathA.c_str()))
		{	
			_simulatorImages.push_back(new SimulatorImage(pathA.c_str(), 5000, 5000));
			result = true;
		}
		string pathB = string(buffer) + "\\Thorlabs\\MesoscopeSys\\SimulatorB.jpg";
		if (INVALID_FILE_ATTRIBUTES != GetFileAttributesA(pathB.c_str()))
		{
			_simulatorImages.push_back(new SimulatorImage(pathB.c_str(), 5000, 5000));
			result = true;
		}
	}	
	return result;
}

long ThorMesoScanSimulator::GetCaptureTime(Scan scans[], const uint8_t scanSize, long& timeMillisecond)
{
	long stripCount=0;
	for (int i = 0; i < scanSize; ++i)
	{
		Scan* scan = &scans[i];
		uint32_t stripWidth = scan->ScanConfig.StripLength;
		for (vector<ScanArea*>::iterator iter = scan->ScanAreas.begin(); iter != scan->ScanAreas.end(); ++iter)
		{
			ScanArea* scanArea = *iter;
			stripCount += static_cast<long>(scanArea->SizeZ* scanArea->SizeS* ceil((double)scanArea->SizeX / stripWidth));
		}
	}
	timeMillisecond = stripCount * 100;
	return TRUE;
}

//**********************************	Additional Functions	**********************************//

void ThorMesoScanSimulator::ImageComplete(ImageCompleteStates state, ChanBufferInfo info, FrameROI roi, unsigned short* buffer, unsigned int size)
{
	unsigned short* dstBuffer = NULL;
	switch (state)
	{
	case SCAN_ERROR:
		_acquireStatus = (int)ICamera::STATUS_ERROR;
		break;
	case SCAN_ABORT:
	case SCAN_COMPLETE:
		//for status to return correctly
		_acquireStatus = (int)ICamera::STATUS_READY;
		break;
	case SCAN_IMAGE:
	case SCAN_IMAGE_STRIPEEND:
		_dmaMutex.lock();

		//circular dma buffer index
		_bufCompleteID %= _bufferInfo.dmaCount;

		dstBuffer = _pFrmDllBuffer[_bufCompleteID] + (_channels[info.ChannelID].ChannelRefID * roi.frameWidth * roi.frameHeight);
		//map to target ROI of current scan area
		for (unsigned int j = 0; j < roi.height; j++)
		{
			for (unsigned int i = 0; i < roi.width; i++)
			{
				dstBuffer[(roi.y + j)*roi.frameWidth + roi.x + i] = buffer[j*roi.width + i];
			}
		}
		//full frame is acquired, copy to next for update
		if ((SCAN_IMAGE_STRIPEEND == state) &&
			(roi.x + roi.width == roi.frameWidth) && (roi.y + roi.height == roi.frameHeight))
		{
			memcpy_s(_pFrmDllBuffer[(_bufCompleteID+1+static_cast<long>(_bufferInfo.dmaCount))%static_cast<long>(_bufferInfo.dmaCount)],_bufferInfo.bufferSize,_pFrmDllBuffer[_bufCompleteID],_bufferInfo.bufferSize);
			_bufferOrder.at(_bufCompleteID) = _indexOfLastCompletedFrame + 1;
			_acquireStatus = (int)ICamera::STATUS_READY;
			_bufCompleteID++;
			_indexOfLastCompletedFrame++;
		}
		else
		{
			_acquireStatus = (int)ICamera::STATUS_PARTIAL;
		}
		_dmaMutex.unlock();
		break;
	case SCAN_BUSY:
		_acquireStatus = (int)ICamera::STATUS_BUSY;
		break;
	default:
		break;
	}
}

void ThorMesoScanSimulator::ClearStripeList()
{
	_mtx.lock();
	if (_firstStrip != NULL)
		StripInfo::DeleteStripLoop(_firstStrip);
	_firstStrip = NULL;
	_currentStrip = NULL;
	_mtx.unlock();
}

void ThorMesoScanSimulator::ClearScan()
{
	for (vector<Scan*>::iterator iter = _scansCache.begin(); iter != _scansCache.end(); )
	{
		delete *iter;
		iter = _scansCache.erase(iter);
	}
	_scan = NULL;
}

void ThorMesoScanSimulator::StopScan()
{
	SetEvent(_stopHandle);
	if(NULL != _hThread)
	{
		WaitForSingleObject(_actualStopHandle, INFINITE);
		ClearStripeList();
		SAFE_DELETE_HANDLE(_hThread);
	}
}
