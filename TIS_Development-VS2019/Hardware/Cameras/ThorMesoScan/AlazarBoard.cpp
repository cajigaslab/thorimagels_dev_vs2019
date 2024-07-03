#include "stdafx.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"
#include "AlazarBoard.h"
#include "Logger.h"
#include <string>
#include <fstream>
#include <omp.h>
#include "..\..\..\Common\HighPerfTimer.h"

using namespace std;

#define ATS_PASSWORD 0x32145876
#define FPGA_MAJOR_VERSION 6
#define FPGA_MINOR_VERSION 7
#define PI	3.1415926535897932384626433832795
#define MINIMUM_SIZE (1024*1024)

const U32 C_CONTROL_FLAGS = ADMA_NPT | ADMA_INTERLEAVE_SAMPLES | ADMA_EXTERNAL_STARTCAPTURE;
const U32 C_MAX_SAMPLES_CLOCKS_PER_RECORD = 64 * 1024;
const U32 C_MAX_BYTES_PER_TABLE = 8192;

bool AlazarBoard9440::IsCorrectFPGA(HANDLE hdl)
{
	if (hdl == NULL)	return FALSE;
	U32 value = 0;
	U32 fpga_major_ver = 0;
	U32 fpga_minor_ver = 0;
	RETURN_CODE retCode = AlazarReadRegister(hdl, 0, &value, ATS_PASSWORD); // Read register 0
	fpga_major_ver = (value & 0x00FF0000) >> 16; // FPGA major version is contained in bits [23..16] of register 0
	fpga_minor_ver = (value & 0x0F000000) >> 24; // FPGA minor version is contained in bits [27..24] of register 0
	if ((fpga_major_ver != FPGA_MAJOR_VERSION) || (fpga_minor_ver != FPGA_MINOR_VERSION))
	{
		Logger::getInstance().LogMessage(L"Wrong FPGA version.", ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long AlazarBoard9440::SetCrsFrequency(double crsFrequency)
{
	_samplePerRecord = (U32)floor(125000000.0 / crsFrequency / 16) * 16;
	if (_samplePerRecord > C_MAX_SAMPLES_CLOCKS_PER_RECORD)
	{
		Logger::getInstance().LogMessage(L"Alazar sample per record out of range.", ERROR_EVENT);
		return FALSE;
	}
	_frameTimeOut = 1000;// (U32)(1.0*1000.0* _recordPerBuffer / _crsFrequencyHighPrecision * 2 + 10);
	return TRUE;
}
long AlazarBoard9440::InitAlazarBoard()
{
	RETURN_CODE retCode;
	retCode = AlazarSetTriggerDelay(_hAlazarSys, 0);
	if (retCode != ApiSuccess) 
	{ 
		Logger::getInstance().LogMessage(L"Alazar set trigger delay failed.", ERROR_EVENT);
		return FALSE;
	}
	retCode = AlazarSetExternalTrigger(_hAlazarSys, DC_COUPLING, ETR_5V);
	if (retCode != ApiSuccess)
	{ 
		Logger::getInstance().LogMessage(L"Alazar set external trigger failed.", ERROR_EVENT);
		return FALSE;
	}
	retCode = AlazarSetTriggerTimeOut(_hAlazarSys, 0);
	if (retCode != ApiSuccess) 
	{ 
		Logger::getInstance().LogMessage(L"Alazar set trigger timeout failed.", ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}
long AlazarBoard9440::SetDataSkippingWithPixelAveraging(bool enable)
{
	RETURN_CODE retCode;
	U32 value;
	retCode = AlazarReadRegister(_hAlazarSys, 15, &value, ATS_PASSWORD);
	if (retCode != ApiSuccess) 
	{
		Logger::getInstance().LogMessage(L"Alazar read register 15 failed.", ERROR_EVENT);
		return FALSE;
	}
	if (enable)
		value |= (1U << 22);
	else
		value &= ~(1U << 22);
	retCode = AlazarWriteRegister(_hAlazarSys, 15, value, ATS_PASSWORD);
	if (retCode != ApiSuccess)
	{
		Logger::getInstance().LogMessage(L"Alazar write register 15 failed.", ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}
long AlazarBoard9440::SetNumRawPoints(long numPoints)
{
	RETURN_CODE retCode;
	retCode = AlazarWriteRegister(_hAlazarSys, 9, numPoints, ATS_PASSWORD); // Register 9 tells the hardware what the record length is supposed to be
	if (retCode != ApiSuccess) 
	{
		Logger::getInstance().LogMessage(L"Alazar write register 9 failed.", ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}
long AlazarBoard9440::SetDataSkip(U32 samplePerRecord, ATSParams* atsParams)
{
	long ret = TRUE;
	long pixelPerRecord = atsParams->PixelPerRecord;
	long recordPixelSize = _atsParams->GetActualRecordPixels();
	RETURN_CODE retCode = AlazarSetRecordSize(_hAlazarSys, 0, recordPixelSize);
	if (retCode != ApiSuccess)
	{
		Logger::getInstance().LogMessage(L"Alazar set record size failed.", ERROR_EVENT);
		return FALSE;
	}
	retCode = AlazarSetCaptureClock(_hAlazarSys,
		INTERNAL_CLOCK,
		SAMPLE_RATE_125MSPS,
		CLOCK_EDGE_RISING,
		0
		);
	if (retCode != ApiSuccess)
	{
		Logger::getInstance().LogMessage(L"Alazar set capture clock failed.", ERROR_EVENT);
		return FALSE;
	}
	double dutyCycle = atsParams->DutyCycle;
	U32 actualSample = samplePerRecord + atsParams->RemapShift;
	U32 xpixels = (U32)(pixelPerRecord / 2.0);
	double lastXOffset = 0.5 / dutyCycle / xpixels + (1.0 - dutyCycle) / 2.0;
	if (acos(min(1.0, max(-1.0, 1.0 - 2.0*lastXOffset)))*actualSample / (2.0 * PI) < atsParams->RemapShift)	//last X point over range
	{
		Logger::getInstance().LogMessage(L"Alazar remap out of range.", ERROR_EVENT);
		return FALSE;
	}

	U32* remap = (U32*)malloc(sizeof(int) * (recordPixelSize));
	memset(remap, 0, sizeof(int) * (recordPixelSize));
	double blockSize = (1.0 - dutyCycle) / 2.0;
	double posPerPixel = dutyCycle / xpixels;

	for (U32 i = 0; i < xpixels; i++)
	{
		double x = (double)(i + 0.5)*posPerPixel + blockSize;
		remap[i + 1] = (U32)(acos(min(1.0, max(-1.0, 1.0 - 2.0 * x))) / (2.0 * PI) *actualSample);
		remap[pixelPerRecord - i + 1] = (U32)((actualSample - remap[i + 1]));
	}

	//additional data
	// add first point for average mode
	remap[0] = (U32)(acos(min(1.0, max(-1.0, 1.0 - 2.0 * blockSize))) / (2.0 * PI) *actualSample);
	// add back line first point for average mode;
	remap[xpixels + 1] = actualSample - ((U32)(acos(min(1.0, max(-1.0, 1 - 2 * (blockSize + dutyCycle)))) / (2.0 * PI)*actualSample));

	int lastIndex = pixelPerRecord + 1;
	U32 lastValue = remap[lastIndex] + 1; // add one for average mode with problem that skipe one 
	for (int j = 1; j < atsParams->IgnorePixels - 1; j++)
	{
		remap[lastIndex + j] = ++lastValue;
	}

	U16 *pSkipTable = (U16*)malloc(C_MAX_BYTES_PER_TABLE);
	memset(pSkipTable, 0x0, C_MAX_BYTES_PER_TABLE);

	for (int i = 0; i<recordPixelSize; i++)
	{
		U32 entryInTable = remap[i] / 16;
		U32 bitInEntry = remap[i] % 16;
		*(pSkipTable + entryInTable) |= (1U << bitInEntry);
	}
	if (SetNumRawPoints(samplePerRecord) == FALSE)
		return FALSE;

	retCode = AlazarConfigureSampleSkipping(_hAlazarSys, SSM_ENABLE, samplePerRecord, pSkipTable);
	if (retCode != ApiSuccess)
	{
		Logger::getInstance().LogMessage(L"Alazar configure sample skipping failed.", ERROR_EVENT);
		ret = FALSE;
	}
	free(remap);
	free(pSkipTable);
	return ret;
}

AlazarBoard9440::AlazarBoard9440(HANDLE hdl)
{
	_hAlazarSys = hdl;
	_runningHandle = CreateEvent(NULL, true, false, NULL);
	_actualStopHandle = CreateEvent(NULL, true, true, NULL);
	_stopHandle = CreateEvent(NULL, true, false, NULL);
	_atsParams = new ATSParams;
	memset(_datamap, 0x0, sizeof(unsigned short) * 65536);
	for (int i = 0; i < ACQUIRE_BUFFER_COUNT; i++) {
		_acquireBuffer[i] = NULL;
	}
	for (int i = 0; i < TRANSFER_BUFFER_COUNT; i++) {
		_transferBuffer[i] = NULL;
	}
	_isReadyToStart = false;
	_inputRangeMap.insert(pair<double, U32>(0.02,INPUT_RANGE_PM_20_MV));
	_inputRangeMap.insert(pair<double, U32>(0.04, INPUT_RANGE_PM_40_MV));
	_inputRangeMap.insert(pair<double, U32>(0.05, INPUT_RANGE_PM_50_MV));
	_inputRangeMap.insert(pair<double, U32>(0.08, INPUT_RANGE_PM_80_MV));
	_inputRangeMap.insert(pair<double, U32>(0.1, INPUT_RANGE_PM_100_MV));
	_inputRangeMap.insert(pair<double, U32>(0.2, INPUT_RANGE_PM_200_MV));
	_inputRangeMap.insert(pair<double, U32>(0.4, INPUT_RANGE_PM_400_MV));
	_inputRangeMap.insert(pair<double, U32>(0.5, INPUT_RANGE_PM_500_MV));
	_inputRangeMap.insert(pair<double, U32>(0.8, INPUT_RANGE_PM_800_MV));
	_inputRangeMap.insert(pair<double, U32>(1, INPUT_RANGE_PM_1_V));
	_inputRangeMap.insert(pair<double, U32>(2, INPUT_RANGE_PM_2_V));
	_inputRangeMap.insert(pair<double, U32>(4, INPUT_RANGE_PM_4_V));
	_inputRangeMap.insert(pair<double, U32>(5, INPUT_RANGE_PM_5_V));
	_inputRangeMap.insert(pair<double, U32>(8, INPUT_RANGE_PM_8_V));
	_inputRangeMap.insert(pair<double, U32>(10, INPUT_RANGE_PM_10_V));
	_inputRangeMap.insert(pair<double, U32>(20, INPUT_RANGE_PM_20_V));
	_inputRangeMap.insert(pair<double, U32>(40, INPUT_RANGE_PM_40_V));
	_inputRangeMap.insert(pair<double, U32>(16, INPUT_RANGE_PM_16_V));
}
AlazarBoard9440::~AlazarBoard9440()
{
	if (_atsParams != NULL)
		delete _atsParams;
	SAFE_DELETE_HANDLE(_actualStopHandle);
	SAFE_DELETE_HANDLE(_runningHandle);
	SAFE_DELETE_HANDLE(_stopHandle);
	for (int i = 0; i < ACQUIRE_BUFFER_COUNT; i++) {
		if (_acquireBuffer[i] != NULL)
		{
			free(_acquireBuffer[i]);
			_acquireBuffer[i] = NULL;
		}
	}
	for (int i = 0; i < TRANSFER_BUFFER_COUNT; i++) {
		if (_transferBuffer[i] != NULL)
		{
			free(_transferBuffer[i]);
			_transferBuffer[i] = NULL;
		}
	}
}
long AlazarBoard9440::FindAlazarBoard()
{
	U32 sysCount = AlazarNumOfSystems();
	U32 availableCount = 0;
	for (U32 i = 1; i <= sysCount; i++)
	{
		HANDLE hdl = AlazarGetSystemHandle(i);
		if (hdl != NULL&&IsCorrectFPGA(hdl))
			availableCount++;
	}
	return availableCount;
}
AlazarBoard9440* AlazarBoard9440::SelectAlazarBoard(uint32_t sid)
{
	HANDLE hdl = AlazarGetSystemHandle(sid);
	if (hdl == NULL)
	{
		Logger::getInstance().LogMessage(L"Alazar remap out of range.", ERROR_EVENT);
		return NULL;
	}
	AlazarBoard9440* board = new AlazarBoard9440(hdl);
	if (board->InitAlazarBoard() == FALSE)
	{
		delete(board);
		return NULL;
	}
	return board;
}
long AlazarBoard9440::VerifyParams(Scan* scan, CameraConfig* cameraConfig, ATSParams** atsParams)
{
	//acquire single or all channels
	long channelID = (0 < scan->Channels.size()) ? 0xF : 0x0;
	long channelCount = (0 < scan->Channels.size()) ? CHANNEL_COUNT : 0;
	if (1 == scan->Channels.size())
	{
		channelCount = 1;
		channelID = 1 << scan->Channels.at(0)->ChannelRefID;
	}
	long delaySignals = (long)ceil(cameraConfig->GetMaxDelayTime() * (double)cameraConfig->ResonanceFrequency);
	uint32_t pixelPerRecord = scan->ScanConfig.StripLength * 2;
	double resonanceFrequency = cameraConfig->ResonanceFrequency;
	double dutyCycle = cameraConfig->PockelDutyCycle;
	uint16_t numberOfAverageFrame = (ICamera::AverageMode::AVG_MODE_NONE == cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_AVERAGEMODE)) ? 
		1 : cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_AVERAGENUM);
	if (channelID < 0x01 || channelID > 0xF
		|| channelCount <= 0 || channelCount > CHANNEL_COUNT
		|| resonanceFrequency < 0
		|| delaySignals < 0
		|| pixelPerRecord < 0
		|| dutyCycle <= 0 || dutyCycle>1)
	{
		Logger::getInstance().LogMessage(L"Alazar error parameters.");
		return FALSE;
	}
	auto inputRange = _inputRangeMap.find(cameraConfig->InputRange);
	if (inputRange == _inputRangeMap.end())
	{
		Logger::getInstance().LogMessage(L"Alazar error parameters.");
		return FALSE;
	}
	if (abs(cameraConfig->MaxPMTVoltage) > cameraConfig->InputRange)
	{
		Logger::getInstance().LogMessage(L"Alazar error parameters.");
		return FALSE;
	}
	if (abs(cameraConfig->MinPMTVoltage) > cameraConfig->InputRange)
	{
		Logger::getInstance().LogMessage(L"Alazar error parameters.");
		return FALSE;
	}
	(*atsParams)->ChannelCount = channelCount;
	(*atsParams)->Channel = channelID;
	(*atsParams)->CrsFrequency = resonanceFrequency;
	(*atsParams)->DelaySignals = delaySignals;
	(*atsParams)->PixelPerRecord = pixelPerRecord;
	(*atsParams)->DutyCycle = dutyCycle;
	(*atsParams)->NumberOfAverageFrame = numberOfAverageFrame;
	double remapShift = 0;
	cameraConfig->GetTwoWayAlignmentPoint(scan->ScanConfig.PhysicalFieldSize / cameraConfig->PockelDutyCycle, remapShift);
	(*atsParams)->RemapShift = scan->ScanConfig.RemapShift + (long)(remapShift+0.000001);
	(*atsParams)->IsDataSkippingAveraging = cameraConfig->IsDataSkippingAveraging;
	(*atsParams)->RecordPerBuffer = cameraConfig->RecordPerBuffer;
	(*atsParams)->TriggerLevel = cameraConfig->TriggerLevel;
	(*atsParams)->InputRange = cameraConfig->InputRange;
	(*atsParams)->InputRangePM = inputRange->second;
	(*atsParams)->MaxPMTVoltage = cameraConfig->MaxPMTVoltage;
	(*atsParams)->MinPMTVoltage = cameraConfig->MinPMTVoltage;
	return TRUE;
}

long AlazarBoard9440::CheckParams(Scan* scan, CameraConfig* cameraConfig)
{
	long ret = TRUE;
	if (WaitForSingleObject(_runningHandle, 0) == WAIT_OBJECT_0)
	{
		ATSParams* atsParams=new ATSParams;
		if (!VerifyParams(scan, cameraConfig, &atsParams))
		{
			ret = FALSE;
		}
		else
		{
			if (atsParams->Channel != _atsParams->Channel
				|| atsParams->ChannelCount != _atsParams->ChannelCount
				|| atsParams->CrsFrequency != _atsParams->CrsFrequency
				|| atsParams->PixelPerRecord != _atsParams->PixelPerRecord
				|| atsParams->DelaySignals != _atsParams->DelaySignals
				|| atsParams->DutyCycle != _atsParams->DutyCycle
				)
			{
				ret = FALSE;
			}
		}
		delete atsParams;
	}
	return ret;
}
long AlazarBoard9440::SetParams(Scan* scan, CameraConfig* cameraConfig, uint32_t maxBufferSize)
{
	if (!VerifyParams(scan, cameraConfig, &_atsParams))
	{
		return FALSE;
	}

	GenerateDataMap(_datamap, _atsParams);
	if (FALSE == SetCrsFrequency(_atsParams->CrsFrequency))
	{
		return FALSE;
	}
	RETURN_CODE retCode = AlazarAbortCapture(_hAlazarSys);
	if (retCode != ApiSuccess) { return FALSE; }
	retCode = AlazarAbortAsyncRead(_hAlazarSys);
	if (retCode != ApiSuccess) { return FALSE; }
	retCode = AlazarSetTriggerOperation(_hAlazarSys,
		TRIG_ENGINE_OP_J, TRIG_ENGINE_J, TRIG_EXTERNAL, TRIGGER_SLOPE_POSITIVE, _atsParams->TriggerLevel,
		TRIG_ENGINE_K, TRIG_DISABLE, TRIGGER_SLOPE_POSITIVE, 128);
	if (retCode != ApiSuccess)
	{
		Logger::getInstance().LogMessage(L"Alazar set trigger operation failed.");
		return FALSE;
	}
	////*** Input Range control by Get / Set Params ***////
	//for (U8 i = 0; i < CHANNEL_COUNT; i++)
	//{
	//	retCode = AlazarInputControl(_hAlazarSys,
	//		1 << i,
	//		DC_COUPLING,
	//		_atsParams->InputRangePM,
	//		IMPEDANCE_50_OHM);
	//	if (retCode != ApiSuccess)
	//	{
	//		Logger::getInstance().LogMessage(L"Alazar set input control failed.");
	//		return FALSE;
	//	}
	//}*//
	if (FALSE == SetDataSkippingWithPixelAveraging(_atsParams->IsDataSkippingAveraging))
	{
		return FALSE;
	}

	if (FALSE == SetDataSkip(_samplePerRecord, _atsParams))
	{
		return FALSE;
	}

	retCode = AlazarBeforeAsyncRead(_hAlazarSys,
		_atsParams->Channel,
		0,//transfer offset
		_atsParams->GetActualRecordPixels(),//sample per record,
		_atsParams->RecordPerBuffer,//record per buffer,    
		0x7FFFFFFF,//record per acquisition
		C_CONTROL_FLAGS
		);
	if (retCode != ApiSuccess)
	{
		Logger::getInstance().LogMessage(L"Alazar before async read failed.");
		return FALSE;
	}
	size_t bufSize = _atsParams->GetPostBufferSize();
	for (int i = 0; i < ACQUIRE_BUFFER_COUNT; i++) {
		_acquireBuffer[i] = (U16*)realloc(_acquireBuffer[i], bufSize);
		AlazarPostAsyncBuffer(_hAlazarSys, _acquireBuffer[i], (U32)bufSize);
	}
	AlazarBoard::clearStripInfo();

	if (maxBufferSize < MINIMUM_SIZE) maxBufferSize = MINIMUM_SIZE;
	for (int i = 0; i < TRANSFER_BUFFER_COUNT; i++) {
		_transferBuffer[i] = (U16*)realloc(_transferBuffer[i], maxBufferSize * sizeof(U16));
	}
	memset(_transferBuffer[0], 0x0, maxBufferSize * sizeof(U16));	//zero first to expedite HW trigger start
	_isReadyToStart = true;
	return TRUE;
}

long AlazarBoard9440::SetInputRange(U8 channelId, U32 Range)
{
	RETURN_CODE retCode = AlazarInputControl(_hAlazarSys,
		channelId,
		DC_COUPLING,
		Range,
		IMPEDANCE_50_OHM);
	if (retCode != ApiSuccess)
	{
		Logger::getInstance().LogMessage(L"Alazar set input range failed.");
		return FALSE;
	}
	return TRUE;
}

long AlazarBoard::AppendStripInfo(StripInfo* stripInfo)
{
	StripRecordInfo* stripRecordInfo = new StripRecordInfo;
	stripRecordInfo->SkipRecordCount = stripInfo->SkipSignal;
	stripRecordInfo->DataRecordCount = stripInfo->IncludeSignal;
	for (int c = 0; c < CHANNEL_COUNT; c++)
	{
		stripRecordInfo->ChanBufInfo[c] = stripInfo->ChanBufInfo[c];
	}
	stripRecordInfo->FrameROI = stripInfo->FrameROI;
	stripRecordInfo->IsAverageEnd = stripInfo->IsAverageEnd;
	stripRecordInfo->IsFrameEnd = stripInfo->IsFrameEnd;
	stripRecordInfo->IsEnd = stripInfo->IsEnd;
	stripRecordInfo->ScanMode = stripInfo->ScanMode;
	if (_endStripInfo == NULL)
	{
		_firstStripInfo = stripRecordInfo;
		_currentStripInfo = stripRecordInfo;
		_endStripInfo = stripRecordInfo;
	}
	else
	{
		_endStripInfo->nextStrip = stripRecordInfo;
		_endStripInfo = stripRecordInfo;
	}
	return TRUE;
}

long AlazarBoard9440::StartAsync()
{
	SetEvent(_runningHandle);
	RETURN_CODE retCode = ApiSuccess;
	U32 stripRecordsCopied = 0;
	U32 acquireRecordsCopied = 0;
	U32 transferBufferIndex = 0;
	U32 acquireBufferIndex = 0;
	bool isSpaceRecord = true;
	ChanBufferInfo fInfo = {}; FrameROI roi = {};
	if (_currentStripInfo == NULL)
	{
		Logger::getInstance().LogMessage(L"No more strip to scan.", WARNING_EVENT);
		//_imageCompleteCallback(SCAN_COMPLETE, {}, {}, NULL, 0);
		_imageCompleteCallback(SCAN_ABORT, fInfo, roi, NULL, 0);	//SCAN_COMPLETE
		Terminate();
		return FALSE;
	}
	_currentStripInfo->SkipRecordCount += _atsParams->DelaySignals;

	_imageCompleteCallback(SCAN_BUSY, _currentStripInfo->ChanBufInfo[0], _currentStripInfo->FrameROI, NULL, 0);
	AlazarStartCapture(_hAlazarSys);
	while (true)
	{
		if (WaitForSingleObject(_stopHandle, 0) == WAIT_OBJECT_0)
		{
			//_imageCompleteCallback(SCAN_ABORT, {}, {}, NULL, 0);
			_imageCompleteCallback(SCAN_ABORT, fInfo, roi, NULL, 0);
			break;
		}
		retCode = AlazarWaitAsyncBufferComplete(_hAlazarSys, _acquireBuffer[acquireBufferIndex], _frameTimeOut);
		if (retCode != ApiSuccess)
		{
			Logger::getInstance().LogMessage(L"Alazar read failed.");
			//_imageCompleteCallback(SCAN_ERROR, {}, {}, NULL, 0);
			_imageCompleteCallback(SCAN_ERROR, fInfo, roi, NULL, 0);
			break;
		}

		acquireRecordsCopied = 0;
		while (acquireRecordsCopied < _atsParams->RecordPerBuffer)
		{
			if (isSpaceRecord)	//space
			{
				if (_atsParams->RecordPerBuffer - acquireRecordsCopied >= _currentStripInfo->SkipRecordCount - stripRecordsCopied)
				{
					acquireRecordsCopied += _currentStripInfo->SkipRecordCount - stripRecordsCopied;
					isSpaceRecord = !isSpaceRecord;
					stripRecordsCopied = 0;
				}
				else
				{
					stripRecordsCopied += _atsParams->RecordPerBuffer - acquireRecordsCopied;
					acquireRecordsCopied = _atsParams->RecordPerBuffer;
				}
			}
			else	//strip
			{
				U16* pDst[CHANNEL_COUNT];
				if (_atsParams->RecordPerBuffer - acquireRecordsCopied >= _currentStripInfo->DataRecordCount - stripRecordsCopied)
				{
					U16* pSrc = _acquireBuffer[acquireBufferIndex] + acquireRecordsCopied*_atsParams->GetActualRecordPixels()*_atsParams->ChannelCount;
					U32 pixelPerRecord = _currentStripInfo->ScanMode == TWO_WAY_SCAN ? _atsParams->PixelPerRecord : _atsParams->PixelPerRecord / 2;
					for (int c = 0; c < _atsParams->ChannelCount; c++)
					{
						pDst[c] = _transferBuffer[transferBufferIndex] + stripRecordsCopied*pixelPerRecord + pixelPerRecord*_currentStripInfo->DataRecordCount * c;
					}

					ProcessBuffer(pSrc, _atsParams->ChannelCount, _atsParams->PixelPerRecord,(_currentStripInfo->DataRecordCount - stripRecordsCopied), pDst, _currentStripInfo->ScanMode, _atsParams->IgnorePixels);

					if (_currentStripInfo->IsAverageEnd)
					{
						for (int c = 0; c < _atsParams->ChannelCount; c++)
						{
							_imageCompleteCallback((_currentStripInfo->IsFrameEnd&&c == _atsParams->ChannelCount - 1) ? SCAN_IMAGE_STRIPEEND : SCAN_IMAGE, _currentStripInfo->ChanBufInfo[c], 
								_currentStripInfo->FrameROI, _transferBuffer[transferBufferIndex] + pixelPerRecord * _currentStripInfo->DataRecordCount * c, pixelPerRecord * _currentStripInfo->DataRecordCount);
						}
						transferBufferIndex = (transferBufferIndex + 1) % TRANSFER_BUFFER_COUNT;
						if (_currentStripInfo->nextStrip != NULL)
						{
							memset(_transferBuffer[transferBufferIndex], 0, pixelPerRecord * _currentStripInfo->nextStrip->DataRecordCount* _atsParams->ChannelCount * sizeof(U16));
						}
					}
					acquireRecordsCopied += _currentStripInfo->DataRecordCount - stripRecordsCopied;
					if (_currentStripInfo->nextStrip == NULL)
					{
						_currentStripInfo = NULL;
						_endStripInfo = NULL;
						break;
					}

					_currentStripInfo = _currentStripInfo->nextStrip;
					isSpaceRecord = !isSpaceRecord;
					stripRecordsCopied = 0;
				}
				else
				{
					U16* pSrc = _acquireBuffer[acquireBufferIndex] + acquireRecordsCopied* _atsParams->GetActualRecordPixels()*_atsParams->ChannelCount;
					U32 pixelPerRecord = _currentStripInfo->ScanMode == TWO_WAY_SCAN ? _atsParams->PixelPerRecord : _atsParams->PixelPerRecord / 2;
					for (int c = 0; c < _atsParams->ChannelCount; c++)
					{
						pDst[c] = _transferBuffer[transferBufferIndex] + stripRecordsCopied*pixelPerRecord + pixelPerRecord*_currentStripInfo->DataRecordCount * c;
					}
					ProcessBuffer(pSrc, _atsParams->ChannelCount, _atsParams->PixelPerRecord,(_atsParams->RecordPerBuffer - acquireRecordsCopied), pDst, _currentStripInfo->ScanMode, _atsParams->IgnorePixels);
					stripRecordsCopied += _atsParams->RecordPerBuffer - acquireRecordsCopied;
					acquireRecordsCopied = _atsParams->RecordPerBuffer;
					_imageCompleteCallback(SCAN_BUSY, _currentStripInfo->ChanBufInfo[0], _currentStripInfo->FrameROI, NULL, 0);
				}
			}
			if (WaitForSingleObject(_stopHandle, 0) == WAIT_OBJECT_0)
				break;
		}
		if (WAIT_OBJECT_0 == WaitForSingleObject(_stopHandle, 0))
		{
			_imageCompleteCallback(SCAN_ABORT, fInfo, roi, NULL, 0);
			break;
		}
		if (_currentStripInfo == NULL)
		{
			//_imageCompleteCallback(SCAN_COMPLETE, {}, {}, NULL, 0);
			_imageCompleteCallback(SCAN_COMPLETE, fInfo, roi, NULL, 0);
			break;
		}
		retCode = AlazarPostAsyncBuffer(_hAlazarSys, _acquireBuffer[acquireBufferIndex], _atsParams->GetPostBufferSize());
		if (retCode != ApiSuccess)
		{
			Logger::getInstance().LogMessage(L"Alazar post buffer failed.");
			//_imageCompleteCallback(SCAN_ERROR, {}, {}, NULL, 0);
			_imageCompleteCallback(SCAN_ERROR, fInfo, roi, NULL, 0);
			break;
		}
		acquireBufferIndex = (acquireBufferIndex + 1) % (ACQUIRE_BUFFER_COUNT);
	}
	Terminate();
	return TRUE;
}

long AlazarBoard9440::ProcessBuffer(U16* pSrc, int chNum, int width, int height, U16** pDst, ScanMode scanMode, int ignorePixels)
{
	if (width % 2 != 0) return FALSE;
	long ret = TRUE;
	int pixelWidth = width / 2;
	int pixelsPerRecord = width + ignorePixels;
	pixelsPerRecord *= chNum;
	switch (chNum)
	{
	case 1:
		if (scanMode == ScanMode::TWO_WAY_SCAN)
		{
#pragma omp parallel for
			for (int h = 0; h < height; h++)
			{
				// add 1 for skip first point for average mode 
				U16 * pSrcForward = (pSrc + pixelsPerRecord * h) + chNum;
				U16 * pSrcBack = pSrcForward + width * chNum + chNum;	//+ width;
				U16 * pDstFirstLine = pDst[0] + pixelWidth * h * 2;
				U16 * pDstNextLine = pDstFirstLine + pixelWidth;
				for (int i = 0; i < pixelWidth; i++)
				{
					*pDstFirstLine++ += _datamap[(*pSrcForward++)];
					*pDstNextLine++ += _datamap[(*--pSrcBack)];			//[(*pSrcBack--)];
				}
			}
		}
		else
		{
#pragma omp parallel for
			for (int h = 0; h < height; h++)
			{
				U16 * pDstFirstLine = pDst[0] + pixelWidth * h;
				U16 * pSrcForward = (pSrc + pixelsPerRecord * h) + chNum;
				for (int i = 0; i < pixelWidth; i++)
				{
					*pDstFirstLine++ += _datamap[(*pSrcForward++)];
				}
			}
		}
		break;
	case 2:
		if (scanMode == ScanMode::TWO_WAY_SCAN) 
		{
#pragma omp parallel for
			for (int h = 0; h < height; h++)
			{
				U16 * pSrcForward = (pSrc + pixelsPerRecord * h) + chNum;
				U16 * pSrcBack = pSrcForward + width * chNum + chNum;

				U16 * pDstFirstLine1 = pDst[0] + pixelWidth * h * 2;
				U16 * pDstNextLine1 = pDstFirstLine1 + pixelWidth;
				U16 * pDstFirstLine2 = pDst[1] + pixelWidth * h * 2;
				U16 * pDstNextLine2 = pDstFirstLine2 + pixelWidth;
				for (int i = 0; i < pixelWidth; i++)
				{
					*pDstFirstLine1++ += _datamap[(*pSrcForward++)];
					*pDstFirstLine2++ += _datamap[(*pSrcForward++)];
					*pDstNextLine2++ += _datamap[(*--pSrcBack)];
					*pDstNextLine1++ += _datamap[(*--pSrcBack)];
				}
			}
		}
		else
		{
#pragma omp parallel for
			for (int h = 0; h < height; h++)
			{
				U16 * pDstFirstLine1 = pDst[0] + pixelWidth * h;
				U16 * pDstFirstLine2 = pDst[1] + pixelWidth * h;
				U16 * pSrcForward = (pSrc + pixelsPerRecord * h) + chNum;
				for (int i = 0; i < width / 2; i++)
				{
					*pDstFirstLine1++ += _datamap[(*pSrcForward++)];
					*pDstFirstLine2++ += _datamap[(*pSrcForward++)];
				}
			}
		}
		break;
	case 4:
		if (scanMode == ScanMode::TWO_WAY_SCAN)
		{
#pragma omp parallel for
			for (int h = 0; h < height; h++)
			{
				U16 * pSrcForward = (pSrc + pixelsPerRecord * h) + chNum;
				U16 * pSrcBack = pSrcForward + width * chNum + chNum;

				U16 * pDstFirstLine1 = pDst[0] + pixelWidth * h * 2;
				U16 * pDstNextLine1 = pDstFirstLine1 + pixelWidth;
				U16 * pDstFirstLine2 = pDst[1] + pixelWidth * h * 2;
				U16 * pDstNextLine2 = pDstFirstLine2 + pixelWidth;
				U16 * pDstFirstLine3 = pDst[2] + pixelWidth * h * 2;
				U16 * pDstNextLine3 = pDstFirstLine3 + pixelWidth;
				U16 * pDstFirstLine4 = pDst[3] + pixelWidth * h * 2;
				U16 * pDstNextLine4 = pDstFirstLine4 + pixelWidth;
				for (int i = 0; i < pixelWidth; i++)
				{
					*pDstFirstLine1++ += _datamap[(*pSrcForward++)];
					*pDstFirstLine2++ += _datamap[(*pSrcForward++)];
					*pDstFirstLine3++ += _datamap[(*pSrcForward++)];
					*pDstFirstLine4++ += _datamap[(*pSrcForward++)];
					*pDstNextLine4++ += _datamap[(*--pSrcBack)];
					*pDstNextLine3++ += _datamap[(*--pSrcBack)];
					*pDstNextLine2++ += _datamap[(*--pSrcBack)];
					*pDstNextLine1++ += _datamap[(*--pSrcBack)];
				}
			}
		}
		else
		{
#pragma omp parallel for
			for (int h = 0; h < height; h++)
			{
				U16 * pDstFirstLine1 = pDst[0] + pixelWidth * h;
				U16 * pDstFirstLine2 = pDst[1] + pixelWidth * h;
				U16 * pDstFirstLine3 = pDst[2] + pixelWidth * h;
				U16 * pDstFirstLine4 = pDst[3] + pixelWidth * h;
				U16 * pSrcForward = (pSrc + pixelsPerRecord * h) + chNum;
				for (int i = 0; i < width / 2; i++)
				{
					*pDstFirstLine1++ += _datamap[(*pSrcForward++)];
					*pDstFirstLine2++ += _datamap[(*pSrcForward++)];
					*pDstFirstLine3++ += _datamap[(*pSrcForward++)];
					*pDstFirstLine4++ += _datamap[(*pSrcForward++)];
				}
			}
		}
		break;
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

long AlazarBoard9440::GenerateDataMap(U16* datamap, ATSParams* atsParams)
{
	U16 aveNum = atsParams->NumberOfAverageFrame;
	double srcMax = atsParams->InputRange;
	double srcMin = -1* atsParams->InputRange;
	double dstMax = atsParams->MaxPMTVoltage;
	double dstMin = atsParams->MinPMTVoltage;
	for (int i = 0; i < 65536; i++)
	{
		double value = (((double)i) / 65536 + (srcMin - dstMin) / (srcMax - srcMin))*(srcMax - srcMin) / (dstMax - dstMin) * 16384;
		if (value > 16383)
			datamap[i] = 16383 / aveNum;
		else if (value < 0)
			datamap[i] = 0;
		else
			datamap[i] = (U16)(value / aveNum);
	}
	return TRUE;
}

void AlazarBoard::clearStripInfo()
{
	if (_firstStripInfo != NULL)
	{
		StripRecordInfo::DeleteStripLoop(_firstStripInfo);
		_firstStripInfo = NULL;
		_currentStripInfo = NULL;
		_endStripInfo = NULL;
	}
}

long AlazarBoard::CloseAlazarBoard(AlazarBoard* alazar)
{
	alazar->StopAcquisition();
	delete(alazar);
	return TRUE;
}

long AlazarBoard::StartAcquisition()
{
	if (WaitForSingleObject(_runningHandle, 0) == WAIT_OBJECT_0)
		return FALSE;
	if (!_isReadyToStart) return FALSE;
	ResetEvent(_stopHandle);
	ResetEvent(_actualStopHandle);
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartThreadFunc, this, 0, NULL);
	if (hThread == NULL) return FALSE;
	SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
	return TRUE;
}

long AlazarBoard::StopAcquisition()
{
	SetEvent(_stopHandle);
	if (WAIT_OBJECT_0 != WaitForSingleObject(_actualStopHandle, Constants::EVENT_WAIT_TIME))
	{
		Terminate();
		Logger::getInstance().LogMessage(L"Alazar stop failed.");
		return FALSE;
	}
	return TRUE;
}

long AlazarBoard::StartThreadFunc(LPVOID pParam)
{
	AlazarBoard *pObj = reinterpret_cast<AlazarBoard*>(pParam);
	return pObj->StartAsync();
}

void AlazarBoard::Terminate()
{
	clearStripInfo();
	AlazarAbortCapture(_hAlazarSys);
	AlazarAbortAsyncRead(_hAlazarSys);
	_isReadyToStart = false;
	ResetEvent(_runningHandle);
	SetEvent(_actualStopHandle);
}
