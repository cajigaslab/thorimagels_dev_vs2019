#include "MesoScanWaveform.h"
#include "GalvoYWaveForm.h"
#include "VoiceCoilWaveForm.h"
#include "PokelsCellWaveForm.h"
#include "GalvoXWaveForm.h"

//#define DEBUG_TIME
//#define DEBUG_LOG
#ifdef DEBUG_LOG
#define LOG(...) printf(__VA_ARGS__)
#define SHOW_VALUE_INT(v) printf(#v##" = %d\n", v)
#else
#define LOG(...)
#define SHOW_VALUE_INT(v)
#endif

#ifdef DEBUG_TIME
#include "MyTimer.h"
#define TIME_START \
	MyTimer mt; \
	mt.Start();
#define TIME_END(name) \
	mt.End(); \
	printf(#name##" time = %ld\n", mt.costTime);
#else
#define TIME_START
#define TIME_END()
#endif

#define BUFFER_WRITE_LENGTH (10600 * 160 *2)

#define VCTABWIDTH 360
#define VCTABHEIGHT 256

WaveformBuffer::WaveformBuffer()
{
	NewBufferCallback = NULL;
	RemoveBufferCallback = NULL;
	waveformBufferData = NULL;
	waveformBufferLength = 0;
	waveformBufferIndex = 0;
	waveformBufferDataNew = NULL;
	waveformBufferLengthNew = 0;
	lastGetBufLength = BUFFER_READ_LENGTH;
}
bool WaveformBuffer::InitBuffer(double* buffer, long length)
{
	bool ret = true;
	if (waveformBufferData != NULL)
	{
		LOG("waveformBufferData not empty!\n");
		(pCBObj->*RemoveBufferCallback)(waveformBufferData);
		ret = false;
	}
	if (waveformBufferDataNew != NULL)
	{
		LOG("waveformBufferDataNew not empty!\n");
		(pCBObj->*RemoveBufferCallback)(waveformBufferDataNew);
		waveformBufferDataNew = NULL;
		waveformBufferLengthNew = 0;
		ret = false;
	}
	waveformBufferData = buffer;
	waveformBufferLength = length;
	newBufferNeeded = true;
	return ret;
}
bool WaveformBuffer::AddBuffer(double* buffer, long length)
{
	LOG("AddBuffer\n");
	if(length <= lastGetBufLength/* * 2*/)
	{
		LOG("Buffer length too small\n");
		return false;
	}
	if (waveformBufferDataNew == NULL)
	{
		if(waveformBufferData == NULL)
		{
			LOG("waveformBufferData already empty, output may not be continous\n");
			return false;
		}
		waveformBufferDataNew = buffer;
		waveformBufferLengthNew = length;
	LOG("AddBuffer: %d, %d\n", waveformBufferData, waveformBufferDataNew);
		return true;
	}
	else
	{
		LOG("Buffer full\n");
		return false;
	}
}
bool WaveformBuffer::GetData(double* dataBuffer, long dataBufferLength, long &outputLength)
{
	lastGetBufLength = dataBufferLength;
	if(waveformBufferData == NULL)
	{
		LOG("Empty buffer\n");
		return false;
	}

	if(newBufferNeeded)
	{
		LOG("NewBufferCallback\n");
		(pCBObj->*NewBufferCallback)(this);
		LOG("NewBufferCallback end\n");
		newBufferNeeded = false;
	}

	bool ret = true;
	if (waveformBufferIndex + dataBufferLength <= waveformBufferLength)
	{
		memcpy(dataBuffer, waveformBufferData + waveformBufferIndex, dataBufferLength * sizeof(double));
		waveformBufferIndex += dataBufferLength;
		outputLength = dataBufferLength;
		return true;
	}
	else
	{
		if (waveformBufferDataNew == NULL)
		{
			ret = false;
			LOG("buffer end\n");
		}
		else
		{
			newBufferNeeded = true;
		}
		LOG("switch buffer\n");
		int length1 = waveformBufferLength - waveformBufferIndex;
		memcpy(dataBuffer, waveformBufferData + waveformBufferIndex, length1 * sizeof(double));
		(pCBObj->*RemoveBufferCallback)(waveformBufferData);
		waveformBufferData = waveformBufferDataNew;
		waveformBufferDataNew = NULL;
		if(dataBufferLength < length1)
		{
			LOG("New buffer too small\n");
			ret = false;
		}
		if (dataBufferLength != length1 && ret)
		{
			waveformBufferIndex = dataBufferLength - length1;
			memcpy(dataBuffer + length1, waveformBufferData, waveformBufferIndex * sizeof(double));
			outputLength = dataBufferLength;
		}
		else
		{
			waveformBufferIndex = 0;
			outputLength = length1;
		}
		waveformBufferLength = waveformBufferLengthNew;
		waveformBufferLengthNew = 0;
	}
	return ret;
}
void WaveformBuffer::ResetBuffer()
{
	if(RemoveBufferCallback != NULL)
	{
		(pCBObj->*RemoveBufferCallback)(waveformBufferData);
		(pCBObj->*RemoveBufferCallback)(waveformBufferDataNew);
	}
	waveformBufferData = NULL;
	waveformBufferDataNew = NULL;
	waveformBufferLength = 0;
	waveformBufferIndex = 0;
}
WaveformBuffer::~WaveformBuffer()
{
}

void WaveformBuffer::RegisterCallback(MesoScanWaveform* pObj, void(MesoScanWaveform::*newCallback)(WaveformBuffer *), void(MesoScanWaveform::*removeCallback)(double *buffer))
{
	pCBObj = pObj;
	NewBufferCallback = newCallback;
	RemoveBufferCallback = removeCallback;
}

MesoScanWaveform::MesoScanWaveform()
{
	_pockelWaveform = NULL;
	_galvoWaveformMemory1 = (float64 *)malloc(BUFFER_WRITE_LENGTH * sizeof(float64));
	_pokelsWaveformMemory1 = (float64 *)malloc(BUFFER_WRITE_LENGTH * sizeof(float64));
	_voiceCoilWaveformMemory1 = (float64 *)malloc(BUFFER_WRITE_LENGTH * sizeof(float64));
	_galvoX1WaveformMemory1 = (float64 *)malloc(BUFFER_WRITE_LENGTH * sizeof(float64));
	_galvoX2WaveformMemory1 = (float64 *)malloc(BUFFER_WRITE_LENGTH * sizeof(float64));
	_galvoWaveformMemory2 = (float64 *)malloc(BUFFER_WRITE_LENGTH * sizeof(float64));
	_pokelsWaveformMemory2 = (float64 *)malloc(BUFFER_WRITE_LENGTH * sizeof(float64));
	_voiceCoilWaveformMemory2 = (float64 *)malloc(BUFFER_WRITE_LENGTH * sizeof(float64));
	_galvoX1WaveformMemory2 = (float64 *)malloc(BUFFER_WRITE_LENGTH * sizeof(float64));
	_galvoX2WaveformMemory2 = (float64 *)malloc(BUFFER_WRITE_LENGTH * sizeof(float64));
	_voiceCoilTableWidth = VCTABWIDTH;
	_voiceCoilTableHeight = VCTABHEIGHT;
	_voiceCoilTable = (float64 *)malloc(VCTABWIDTH * VCTABHEIGHT * sizeof(float64));
	_galvoVoiceCoilWaveform = (double*)malloc(BUFFER_READ_LENGTH * sizeof(double) * 3);
	_galvoXWaveform = (double*)malloc(BUFFER_READ_LENGTH * sizeof(double) * 2);

	_theta2Volts = 0.1;
	_f2TGy = 0.02;
	_f2TGx1 = 0.01; 
	_f2TGx2 = 0.01; 
	_hStartWriteBuffer = CreateEvent(NULL, true, false, NULL);
	_hEndWriteBuffer = CreateEvent(NULL, true, false, NULL);
	_hWritingBuffer = CreateMutex(NULL, false, NULL);
	_endflag = true;
}

MesoScanWaveform::~MesoScanWaveform()
{
	ReleaseBuffer();
	free(_galvoWaveformMemory1);
	free(_pokelsWaveformMemory1);
	free(_voiceCoilWaveformMemory1);
	free(_galvoX1WaveformMemory1);
	free(_galvoX2WaveformMemory1);
	free(_galvoWaveformMemory2);
	free(_pokelsWaveformMemory2);
	free(_voiceCoilWaveformMemory2);
	free(_galvoX1WaveformMemory2);
	free(_galvoX2WaveformMemory2);
	if (_voiceCoilTable != NULL)
	{
		free(_voiceCoilTable);
		_voiceCoilTable = NULL;
	}
	free(_galvoVoiceCoilWaveform);
	free(_galvoXWaveform);
}

long MesoScanWaveform::InitHardwareInfo(double f2TGy, double f2TGx1, double f2TGx2, double gRfrequence, double waveformSampleClock)
{
	_f2TGx1 = f2TGx1;
	_f2TGx2 = f2TGx2;
	_f2TGy = f2TGy;
    return TRUE;
}
 
double* MesoScanWaveform::GetGalvoVoiceCoilWaveForm(long length, long &lengthOut, bool& isContinue)
{
	isContinue = true;
	TIME_START
	isContinue = ReadGalvoVoiceCoilBuffer(_galvoVoiceCoilWaveform, length, lengthOut);
	TIME_END(GetWaveform)
	return _galvoVoiceCoilWaveform;
}

double* MesoScanWaveform::GetGalvoXWaveForm(long length, long &lengthOut, bool& isContinue)
{
	isContinue = true;
	TIME_START
	isContinue = ReadGalvoXBuffer(_galvoXWaveform, length, lengthOut);
	TIME_END(GetWaveform)
	return _galvoXWaveform;
}

double* MesoScanWaveform::GetPockelWaveForm()
{
	return _pockelWaveform;
}

long MesoScanWaveform::GenerateWaveForms(WaveformParams* wP)
{
	_params.dutyCycle = wP->PockelDutyCycle;
	_params.pockelsPointCount = wP->PockelsPointCount;
	_params.stripWidth = wP->StripWidth;
	_params.physicalStripeWidth = wP->PhysicalStripeWidth;
	_params.yPixelSize =  wP->YPixelSize;
	_params.pokelsVolt = wP->PokelsVolt;
	_params.zPixelSize = wP->ZPixelSize;
	_params.isTwoWay = wP->CScanMode == TWO_WAY_SCAN;

	GalvoYWaveForm::SetParameters(_f2TGy, _theta2Volts, _params.pockelsPointCount, _params.isTwoWay);
	PokelsCellWaveForm::SetParameters(_params.pokelsVolt, _params.dutyCycle, _params.pockelsPointCount, _params.isTwoWay);
	VoiceCoilWaveForm::SetParameters(_params.pokelsVolt, _params.physicalStripeWidth, _params.pockelsPointCount);
	GalvoXWaveForm::SetParameters(_f2TGx1, _f2TGx2, _theta2Volts, _params.physicalStripeWidth, _params.dutyCycle, _params.pockelsPointCount);

	SetROIs(wP->ScanAreas);
	RestartGeneration();
	return TRUE;
}
long MesoScanWaveform::RestartGeneration()
{
	SetWaveformParam();
	_endflag = false;

	_galvoCallBackFlag = false;
	_pokelsCallBackFlag = false;
	_voicecoilCallBackFlag = false;
	_galvoX1CallBackFlag = false;
	_galvoX2CallBackFlag = false;

	_threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadWriteBuffer, (LPVOID)this, 0, &_threadID);
	return TRUE;
}
void MesoScanWaveform::threadWriteBuffer(LPVOID lParam)
{
	MesoScanWaveform* pInstance = (MesoScanWaveform*)lParam;
	LOG("thread\n");
	while(1)
	{
		LOG("threadWriteBuffer\n");
		LOG("WaitForSingleObject _hStartWriteBuffer\n");
		WaitForSingleObject(pInstance->_hStartWriteBuffer, INFINITE);
		ResetEvent(pInstance->_hStartWriteBuffer);
		LOG("GalvoWaveformNewCallback\n");
		WaitForSingleObject(pInstance->_hWritingBuffer, INFINITE);
		if(pInstance->_nextROI == pInstance->_rois.end() || pInstance->_endflag)
		{
			break;
		}

		long bufferLength;
		pInstance->_currentROI = pInstance->_nextROI;
		pInstance->_currentZIdx = pInstance->_nextZIdx;
		pInstance->_currentStripeIdx = pInstance->_nextStripeIdx;
		vector<Stripe> stripeList;
		Stripe* pNextStripe = NULL;
		pInstance->GetStripeList(pInstance->_currentROI, pInstance->_currentZIdx, pInstance->_currentStripeIdx, BUFFER_WRITE_LENGTH, pInstance->_nextROI, pInstance->_nextZIdx, pInstance->_nextStripeIdx, stripeList, pNextStripe, bufferLength);
		pInstance->SetLength(bufferLength);

		pInstance->_galvoWaveformData = pInstance->SelectBuffer(GALVOBUFFER);
		pInstance->_pokelsWaveformData = pInstance->SelectBuffer(POKELSBUFFER);
		pInstance->_voiceCoilWaveformData = pInstance->SelectBuffer(VOICECOILBUFFER);
		pInstance->_galvoX1WaveformData = pInstance->SelectBuffer(GALVOX1BUFFER);
		pInstance->_galvoX2WaveformData = pInstance->SelectBuffer(GALVOX2BUFFER);

		GalvoYWaveForm::CreateGalvoWaveformFromStripeList(pInstance->_galvoWaveformData, stripeList, pNextStripe);
		PokelsCellWaveForm::CreatePokelsWaveformFromStripeList(pInstance->_pokelsWaveformData, stripeList, pNextStripe);
		VoiceCoilWaveForm::CreateVoiceCoilWaveformFromStripeList(pInstance->_voiceCoilWaveformData, stripeList, pNextStripe);
		GalvoXWaveForm::CreateGalvoX1WaveformFromStripeList(pInstance->_galvoX1WaveformData, stripeList, pNextStripe);
		GalvoXWaveForm::CreateGalvoX1WaveformFromStripeList(pInstance->_galvoX2WaveformData, stripeList, pNextStripe);

		pInstance->_galvoBufferQueue.AddBuffer(pInstance->_galvoWaveformData, bufferLength);
		pInstance->_pokelsBufferQueue.AddBuffer(pInstance->_pokelsWaveformData, bufferLength);
		pInstance->_voicecoilBufferQueue.AddBuffer(pInstance->_voiceCoilWaveformData, bufferLength);
		pInstance->_galvoX1BufferQueue.AddBuffer(pInstance->_galvoX1WaveformData, bufferLength);
		pInstance->_galvoX2BufferQueue.AddBuffer(pInstance->_galvoX2WaveformData, bufferLength);

		if (pNextStripe != NULL)
			delete pNextStripe;

		ReleaseMutex(pInstance->_hWritingBuffer);
		SetEvent(pInstance->_hEndWriteBuffer);
		LOG("SetEvent _hEndWriteBuffer\n");
		LOG("threadWriteBuffer end loop\n");
	}
	ReleaseMutex(pInstance->_hWritingBuffer);
	SetEvent(pInstance->_hEndWriteBuffer);
}

long MesoScanWaveform::GetBlankLengthByPoint(double xOffset, double yOffset, double zOffset)
{
	return GetBlankLengthByLine(xOffset, yOffset, zOffset) * _params.pockelsPointCount;
}
long MesoScanWaveform::GetBlankLengthByLine(double xOffset, double yOffset, double zOffset)
{
	//
	if (zOffset > 1)
		return 16;
	else if (zOffset > 0)
		return 8;
	else
		return 4;
}
long MesoScanWaveform::GetCurrentStripeBlankLengthByLine(vector<ScanROI>::iterator pROI, long zIdx, long stripeIdx)
{
	vector<ScanROI>::iterator pROINext;
	if ((stripeIdx + 1) * _params.physicalStripeWidth < pROI->XWidth)
	{
		return GetBlankLengthByLine(_params.physicalStripeWidth, pROI->YHeight, 0);
	}
	else if (zIdx + 1 < pROI->ZHeight / _params.zPixelSize)
	{
		return GetBlankLengthByLine(_params.physicalStripeWidth * GetStripeCount(pROI), pROI->YHeight, _params.zPixelSize);
	}
	else if ((pROINext = pROI + 1) != _rois.end())
	{
		return GetBlankLengthByLine(pROI->XPos + _params.physicalStripeWidth * GetStripeCount(pROI) - pROINext->XPos,
			pROI->YPos + pROI->YHeight - pROINext->YPos,
			pROI->ZPos + pROI->ZHeight - pROINext->ZPos);
	}
	else
	{
		return GetBlankLengthByLine(0, 0, 0);
	}
}
long MesoScanWaveform::GetCurrentStripeBlankLengthByPoint(vector<ScanROI>::iterator pROI, long zIdx, long stripeIdx)
{
	return GetCurrentStripeBlankLengthByLine(pROI, zIdx, stripeIdx) * _params.pockelsPointCount;
}

void MesoScanWaveform::GetStartPos(double &x1, double &x2, double &y)
{
	if(_rois.empty())
	{
		x1 = 0;
		x2 = 0;
		y = 0;
	}
	else
	{
		x1 = GalvoX1FieldToVolts(_rois.begin()->XPos) - GetStripeShift();
		x2 = GalvoX2FieldToVolts(_rois.begin()->XPos) - GetStripeShift();
		y = GalvoYFieldToVolts(_rois.begin()->YPos);
	}
}

long MesoScanWaveform::ClearFrameBuffers()
{
	LOG("ClearFrameBuffers\n");
	if(_endflag)
	{
		ReleaseBuffer();
	}
	else
	{
		WaitForSingleObject(_hWritingBuffer, 5000);
		_endflag = true;
		ReleaseBuffer();
		ReleaseMutex(_hWritingBuffer);
		SetEvent(_hStartWriteBuffer);
	}
	LOG("ClearFrameBuffers end\n");
	return TRUE;
}

StripInformation MesoScanWaveform::GetStripInformation()
{
	return _stripInformation;
}


void MesoScanWaveform::SetWaveformParam()
{
	LOG("SetWaveformParam\n");
	TIME_START
	_currentROI = _rois.begin();
	_currentStripeIdx = 0;
	_currentZIdx = 0;
	_nextROI = _rois.begin();
	_nextStripeIdx = 0;
	_nextZIdx = 0;
	LOG("InitializeVoiceCoilWaveform\n");
	InitializeVoiceCoilWaveform();
	LOG("InitializeVoiceCoilWaveform end\n");
	long bufferLength;
	vector<Stripe> stripeList;
	Stripe* pNextStripe = NULL;
	GetStripeList(_currentROI, _currentZIdx, _currentStripeIdx, BUFFER_WRITE_LENGTH, _nextROI, _nextZIdx, _nextStripeIdx, stripeList, pNextStripe, bufferLength);

	SetLength(bufferLength);
	int length = GetBufferLength();
	_galvoWaveformData = SelectBuffer(GALVOBUFFER);
	GalvoYWaveForm::CreateGalvoWaveformFromStripeList(_galvoWaveformData, stripeList, pNextStripe);
	TIME_END()
	_pokelsWaveformData = SelectBuffer(POKELSBUFFER);
	PokelsCellWaveForm::CreatePokelsWaveformFromStripeList(_pokelsWaveformData, stripeList, pNextStripe);
	TIME_END()
	_voiceCoilWaveformData = SelectBuffer(VOICECOILBUFFER);
	VoiceCoilWaveForm::CreateVoiceCoilWaveformFromStripeList(_voiceCoilWaveformData, stripeList, pNextStripe);
	TIME_END()
	_galvoX1WaveformData = SelectBuffer(GALVOX1BUFFER);
	GalvoXWaveForm::CreateGalvoX1WaveformFromStripeList(_galvoX1WaveformData, stripeList, pNextStripe);
	TIME_END()
	_galvoX2WaveformData = SelectBuffer(GALVOX2BUFFER);
	GalvoXWaveForm::CreateGalvoX1WaveformFromStripeList(_galvoX2WaveformData, stripeList, pNextStripe);
	TIME_END()

	_galvoBufferQueue.InitBuffer(_galvoWaveformData, length);
	_pokelsBufferQueue.InitBuffer(_pokelsWaveformData, length);
	_voicecoilBufferQueue.InitBuffer(_voiceCoilWaveformData, length);
	_galvoX1BufferQueue.InitBuffer(_galvoX1WaveformData, length);
	_galvoX2BufferQueue.InitBuffer(_galvoX2WaveformData, length);

	if (pNextStripe != NULL)
		delete pNextStripe;

	SetEvent(_hEndWriteBuffer);
	ResetEvent(_hStartWriteBuffer);
	LOG("SetEvent _hEndWriteBuffer\n");

	_galvoBufferQueue.RegisterCallback(this, &MesoScanWaveform::GalvoWaveformNewCallback, &MesoScanWaveform::GalvoWaveformRemoveCallback);
	_pokelsBufferQueue.RegisterCallback(this, &MesoScanWaveform::PokelsWaveformNewCallback, &MesoScanWaveform::PokelsWaveformRemoveCallback);
	_voicecoilBufferQueue.RegisterCallback(this, &MesoScanWaveform::VoiceCoilWaveformNewCallback, &MesoScanWaveform::VoiceCoilWaveformRemoveCallback);
	_galvoX1BufferQueue.RegisterCallback(this, &MesoScanWaveform::GalvoX1WaveformNewCallback, &MesoScanWaveform::GalvoX1WaveformRemoveCallback);
	_galvoX2BufferQueue.RegisterCallback(this, &MesoScanWaveform::GalvoX2WaveformNewCallback, &MesoScanWaveform::GalvoX2WaveformRemoveCallback);

	TIME_END()
	return;
}
void MesoScanWaveform::SetROIs(vector<ScanROI>& roisIn)
{
	_stripInformation.stripLengthArry.clear();
	_stripInformation.stripSpaceArry.clear();

	_rois.assign(roisIn.begin(), roisIn.end());
	_stripInformation.StripSize = _params.stripWidth;
	for(vector<ScanROI>::iterator iter = _rois.begin(); iter != _rois.end(); iter++)
	{
		int z = 0;
		int stripe = 0;
		for(z = 0; z < iter->ZHeight/_params.zPixelSize; z++)
		{
			auto height = iter->YHeight/_params.yPixelSize;
			for(stripe = 0; stripe * _params.physicalStripeWidth < iter->XWidth; stripe++)
			{
				_stripInformation.stripLengthArry.push_back(height);
				_stripInformation.stripSpaceArry.push_back(GetCurrentStripeBlankLengthByLine(iter, z, stripe));
			}
		}
	}
	if(_stripInformation.stripSpaceArry.size()>0)
	{
		_stripInformation.stripSpaceArry[_stripInformation.stripSpaceArry.size()-1] = 0;
	}
}
bool MesoScanWaveform::ReadGalvoVoiceCoilBuffer(double* data, long channelLength, long &realLength)
{
	LOG("GetWaveform\n");
	TIME_START
	realLength = 0;
	bool ret = _galvoBufferQueue.GetData(data, channelLength, realLength);
	TIME_END()
	if(realLength == 0)
	{
		return false;
	}
	else
	{
		long outputLength = 0;
		bool ret1 = _voicecoilBufferQueue.GetData(data + realLength, realLength, outputLength);
		ret = ret && ret1;
		TIME_END()
		bool ret2 = _pokelsBufferQueue.GetData(data + realLength *2, realLength, outputLength);
		ret = ret && ret2;

		TIME_END()
	}
	return ret;
}
bool MesoScanWaveform::ReadGalvoXBuffer(double* data, long channelLength, long &realLength)
{
	LOG("GetWaveform\n");
	TIME_START
	realLength = 0;
	bool ret = _galvoX1BufferQueue.GetData(data, channelLength, realLength);
	TIME_END()
	if (realLength == 0)
	{
		return false;
	}
	else
	{
		long outputLength = 0;
		bool ret1 = _galvoX2BufferQueue.GetData(data + realLength, realLength, outputLength);
		ret = ret && ret1;
		TIME_END()
	}
	return ret;
}


void MesoScanWaveform::GetStripeList(vector<ScanROI>::iterator pROI1, long zIdx1, long stripeIdx1, long length, vector<ScanROI>::iterator &pROI2, long &zIdx2, long &stripeIdx2, vector<Stripe>& stripeList, Stripe* &pNextStripe, long &lengthOut)
{
	vector<ScanROI>::iterator pROI = pROI1;
	long zIdx = zIdx1;
	long stripeIdx = stripeIdx1;
	lengthOut = 0;
	while (pROI != _rois.end())
	{
		while (zIdx < GetZCount(pROI))
		{
			while (stripeIdx < GetStripeCount(pROI))
			{
				int blankLines = GetCurrentStripeBlankLengthByLine(pROI, zIdx, stripeIdx);
				long stripelength = GetStripeLength(pROI);
				long totalLength = stripelength + blankLines * _params.pockelsPointCount;
				int yLines = LineCount(pROI);
				double stripeWidth = _params.physicalStripeWidth;
				double xPos = pROI->XPos + stripeWidth * stripeIdx;
				double yStart = pROI->YPos;
				double yPixSize = _params.yPixelSize;
				double zPos = pROI->ZPos + _params.zPixelSize * zIdx;
				if (lengthOut + totalLength > length)
				{
					pNextStripe = new Stripe(1, xPos, yStart, yPixSize, zPos, stripeWidth, yLines, blankLines, stripelength, pROI);
					pROI2 = pROI;
					zIdx2 = zIdx;
					stripeIdx2 = stripeIdx;
					return;
				}
				stripeList.push_back(Stripe(1, xPos, yStart, yPixSize, zPos, stripeWidth, yLines, blankLines, stripelength, pROI));
				lengthOut += totalLength;
				stripeIdx++;
			}
			stripeIdx = 0;
			zIdx++;
		}
		zIdx = 0;
		pROI++;
	}
	pROI2 = pROI;
	zIdx2 = zIdx;
	stripeIdx2 = stripeIdx;
	return;
}

double* MesoScanWaveform::SelectBuffer(BufferType type)
{
	switch(type)
	{
	case GALVOBUFFER:
		if(_galvoWaveformMemory1Available)
		{
			_galvoWaveformMemory1Available = false;
			LOG("buffer1 selected\n");
			return _galvoWaveformMemory1;
		}
		else if(_galvoWaveformMemory2Available)
		{
			_galvoWaveformMemory2Available = false;
			LOG("buffer2 selected\n");
			return _galvoWaveformMemory2;
		}
		else
			return NULL;
	case POKELSBUFFER:
		if(_pokelsWaveformMemory1Available)
		{
			_pokelsWaveformMemory1Available = false;
			return _pokelsWaveformMemory1;
		}
		else if(_pokelsWaveformMemory2Available)
		{
			_pokelsWaveformMemory2Available = false;
			return _pokelsWaveformMemory2;
		}
		else
			return NULL;
	case VOICECOILBUFFER:
		if(_voiceCoilWaveformMemory1Available)
		{
			_voiceCoilWaveformMemory1Available = false;
			return _voiceCoilWaveformMemory1;
		}
		else if(_voiceCoilWaveformMemory2Available)
		{
			_voiceCoilWaveformMemory2Available = false;
			return _voiceCoilWaveformMemory2;
		}
		else
			return NULL;
	case GALVOX1BUFFER:
		if (_galvoX1WaveformMemory1Available)
		{
			_galvoX1WaveformMemory1Available = false;
			return _galvoX1WaveformMemory1;
		}
		else if (_galvoX1WaveformMemory2Available)
		{
			_galvoX1WaveformMemory2Available = false;
			return _galvoX1WaveformMemory2;
		}
		else
			return NULL;
	case GALVOX2BUFFER:
		if (_galvoX2WaveformMemory1Available)
		{
			_galvoX2WaveformMemory1Available = false;
			return _galvoX2WaveformMemory1;
		}
		else if (_galvoX2WaveformMemory2Available)
		{
			_galvoX2WaveformMemory2Available = false;
			return _galvoX2WaveformMemory2;
		}
		else
			return NULL;
	}
}

void MesoScanWaveform::GalvoWaveformNewCallback(WaveformBuffer* waveformBufferQueue)
{
	CheckForWaveFormUpdate(GALVOBUFFER);
	return;
}
void MesoScanWaveform::PokelsWaveformNewCallback(WaveformBuffer* waveformBufferQueue)
{
	CheckForWaveFormUpdate(POKELSBUFFER);
	return;
}
void MesoScanWaveform::VoiceCoilWaveformNewCallback(WaveformBuffer* waveformBufferQueue)
{
	CheckForWaveFormUpdate(VOICECOILBUFFER);
	return;
}
void MesoScanWaveform::GalvoX1WaveformNewCallback(WaveformBuffer* waveformBufferQueue)
{
	CheckForWaveFormUpdate(GALVOX1BUFFER);
	return;
}
void MesoScanWaveform::GalvoX2WaveformNewCallback(WaveformBuffer* waveformBufferQueue)
{
	CheckForWaveFormUpdate(GALVOX2BUFFER);
	return;
}

void MesoScanWaveform::CheckForWaveFormUpdate(BufferType type)
{
	if (!(_galvoCallBackFlag || _pokelsCallBackFlag || _voicecoilCallBackFlag || _galvoX1CallBackFlag || _galvoX2CallBackFlag))
	{
		LOG("GalvoWaveformNewCallback WaitForSingleObject _hEndWriteBuffer\n");
		WaitForSingleObject(_hEndWriteBuffer, INFINITE);
		ResetEvent(_hEndWriteBuffer);
		SetEvent(_hStartWriteBuffer);
		LOG("GalvoWaveformNewCallback SetEvent _hStartWriteBuffer\n");
	}
	switch (type)
	{
	case GALVOBUFFER:
		_galvoCallBackFlag = true;
		break;
	case POKELSBUFFER:
		_pokelsCallBackFlag = true;
		break;
	case VOICECOILBUFFER:
		_voicecoilCallBackFlag = true;
		break;
	case GALVOX1BUFFER:
		_galvoX1CallBackFlag = true;
		break;
	case GALVOX2BUFFER:
		_galvoX2CallBackFlag = true;
		break;
	default:
		break;
	}
	if (_galvoCallBackFlag && _pokelsCallBackFlag && _voicecoilCallBackFlag && _galvoX1CallBackFlag && _galvoX2CallBackFlag)
	{
		_galvoCallBackFlag = false;
		_pokelsCallBackFlag = false;
		_voicecoilCallBackFlag = false;
		_galvoX1CallBackFlag = false;
		_galvoX2CallBackFlag = false;
	}
}

void MesoScanWaveform::GalvoWaveformRemoveCallback(double* buffer)
{
	LOG("GalvoWaveformRemoveCallback\n");
	if(buffer == _galvoWaveformMemory1)
	{
		_galvoWaveformMemory1Available = true;
		LOG("buffer1 released\n");
	}
	else if(buffer == _galvoWaveformMemory2)
	{
		_galvoWaveformMemory2Available = true;
		LOG("buffer2 released\n");
	}
}
void MesoScanWaveform::PokelsWaveformRemoveCallback(double* buffer)
{
	LOG("PokelsWaveformRemoveCallback\n");
	if(buffer == _pokelsWaveformMemory1)
	{
		_pokelsWaveformMemory1Available = true;
	}
	else if(buffer == _pokelsWaveformMemory2)
	{
		_pokelsWaveformMemory2Available = true;
	}
}
void MesoScanWaveform::VoiceCoilWaveformRemoveCallback(double* buffer)
{
	LOG("VoiceCoilWaveformRemoveCallback\n");
	if(buffer == _voiceCoilWaveformMemory1)
	{
		_voiceCoilWaveformMemory1Available = true;
	}
	else if(buffer == _voiceCoilWaveformMemory2)
	{
		_voiceCoilWaveformMemory2Available = true;
	}
}
void MesoScanWaveform::GalvoX1WaveformRemoveCallback(double* buffer)
{
	LOG("GalvoWaveformRemoveCallback\n");
	if (buffer == _galvoX1WaveformMemory1)
	{
		_galvoX1WaveformMemory1Available = true;
		LOG("buffer1 released\n");
	}
	else if (buffer == _galvoX1WaveformMemory2)
	{
		_galvoX1WaveformMemory2Available = true;
		LOG("buffer2 released\n");
	}
}
void MesoScanWaveform::GalvoX2WaveformRemoveCallback(double* buffer)
{
	LOG("GalvoWaveformRemoveCallback\n");
	if (buffer == _galvoX2WaveformMemory1)
	{
		_galvoX2WaveformMemory1Available = true;
		LOG("buffer1 released\n");
	}
	else if (buffer == _galvoX2WaveformMemory2)
	{
		_galvoX2WaveformMemory2Available = true;
		LOG("buffer2 released\n");
	}
}

void MesoScanWaveform::InitializeVoiceCoilWaveform()
{
	static bool initialized = false;
	if(!initialized)
	{
		int y;
		for (y = 0; y < _voiceCoilTableHeight; y++)
		{
			int x = 0;
			for (; x < _voiceCoilTableWidth; x++)
			{
				double xOff = (double)x / _voiceCoilTableWidth - 0.5;
				double yOff = (double)y / _voiceCoilTableHeight - 0.5;
				_voiceCoilTable[y*_voiceCoilTableWidth + x] = (xOff*xOff + yOff*yOff) * 5;
			}
		}
		initialized = true;
	}
}


void MesoScanWaveform::ReleaseBuffer()
{
	_galvoBufferQueue.ResetBuffer();
	_pokelsBufferQueue.ResetBuffer();
	_voicecoilBufferQueue.ResetBuffer();
	_galvoX1BufferQueue.ResetBuffer();
	_galvoX2BufferQueue.ResetBuffer();

	_galvoWaveformMemory1Available = true;
	_pokelsWaveformMemory1Available = true;
	_voiceCoilWaveformMemory1Available = true;
	_galvoX1WaveformMemory1Available = true;
	_galvoX2WaveformMemory1Available = true;

	_galvoWaveformMemory2Available = true;
	_pokelsWaveformMemory2Available = true;
	_voiceCoilWaveformMemory2Available = true;
	_galvoX1WaveformMemory2Available = true;
	_galvoX2WaveformMemory2Available = true;

	_galvoCallBackFlag = false;
	_pokelsCallBackFlag = false;
	_voicecoilCallBackFlag = false;
	_galvoX1CallBackFlag = false;
	_galvoX2CallBackFlag = false;

	_params = WaveformGenerateParams();
}
