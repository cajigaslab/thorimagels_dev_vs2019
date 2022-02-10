#include "stdafx.h"
#include "PublicType.h"
#include "..\..\..\..\GUI\Controls\RealTimeLineChart\RealTimeLineChart\PublicEnum.cs"
#include "AcquireNIData.h"
#include <cmath>
#include "strsafe.h"
#include <iostream>
#include <string>

#define THREAD_WAIT			5000		//[mSec]
#define MIN_THREAD_CNT		4
#define MIN_VOLTAGE			-10.0		//[V]
#define MAX_VOLTAGE			10.0		//[V]

//#pragma warning(push)
//#pragma warning(disable:4251)
//#include "..\..\..\..\Tools\HDF5io\HDF5io\HDF5io.h"
//#pragma warning(pop)

std::unique_ptr<HDF5ioDLL> h5io(new HDF5ioDLL(L".\\Modules_Native\\HDF5IO.dll"));
std::wstring lastError;
wchar_t message[MSG_LENGTH];

///******	static members	******///
static CRITICAL_SECTION saveFileAccess;
static CRITICAL_SECTION displayUpdateAccess;
static CRITICAL_SECTION staticStopAccess;

taskHandles AcquireNIData::CItaskHandles;
TaskHandle AcquireNIData::AItaskHandle = NULL;
TaskHandle AcquireNIData::DItaskHandle = NULL;
TaskHandle AcquireNIData::DOtaskHandle = NULL;
TaskHandle AcquireNIData::AOtaskHandle = NULL;
TaskHandle AcquireNIData::primarySampleTask = NULL;

std::list<HANDLE> AcquireNIData::_hSaveThreads;
HANDLE AcquireNIData::_hProcessSaveThread = NULL;
volatile size_t AcquireNIData::_saveThreadCnt = 0;
volatile size_t AcquireNIData::_saveThreadFinishedCnt = 0;
volatile size_t AcquireNIData::_totalNumThreads = 0;
BOOL AcquireNIData::_inSaveThread = FALSE;
long AcquireNIData::_isSaving = FALSE;
BOOL AcquireNIData::_isAcquiring = FALSE;
bool AcquireNIData::_retrig = false;
bool AcquireNIData::_pauseTrig = false;
bool AcquireNIData::_analogTrig = false;
bool AcquireNIData::_invertEnabled = false;
long AcquireNIData::interleave = 0;
time_t AcquireNIData::triggerTime = 0;

StimulusSaveStruct* AcquireNIData::stimulusParams = NULL;
std::vector<std::string> AcquireNIData::counterNames;
uInt64 AcquireNIData::_samplesPerCallback = 0;
uInt64 AcquireNIData::_displayPerCallback = 0;
CompoundData *AcquireNIData::_displayCompData = NULL;
int32 AcquireNIData::sampleMode = 0;
char AcquireNIData::sampleChannel[_MAX_FNAME];
std::string AcquireNIData::triggerChannel;
std::string AcquireNIData::_primaryChanType; 
std::string AcquireNIData::_pmtShutter;
unsigned long AcquireNIData::gCtrOverflowCnt = 0;
int32 error = 0;
bool switchCounterTask = false;
bool initializedGCtr = false;
bool extendBuf = false;
size_t initialGCtr = 0;
size_t deltaGCtr = 0;
unsigned long currentFirstGCtr = 0; 
DWORD testTime = 0;
BOOL skipOverflowCheck = FALSE;
const uInt8 HighDO = true;
BOOL AcquireNIData::_isAsyncAcquiring = FALSE;
std::vector<std::string> AcquireNIData::_invLines;
HANDLE AcquireNIData::_hAsyncThread = NULL;
HANDLE AcquireNIData::_hProcessAsyncThread = NULL;
HANDLE AcquireNIData::_hHWTriggerAsyncEvent = CreateEvent(NULL, TRUE, false, NULL);		//reset option is true (MANUAL RESET)
HANDLE AcquireNIData::_hStopAsyncEvent = CreateEvent(NULL, TRUE, false, NULL);			//reset option is true (MANUAL RESET)
HANDLE AcquireNIData::_hAsyncEventFinished = CreateEvent(NULL, FALSE, false, NULL);		//reset option is false (AUTO RESET)
HANDLE AcquireNIData::_hAsyncCallbackInvoked = CreateEvent(NULL, FALSE, false, NULL);	//reset option is false (AUTO RESET)
TaskHandle AcquireNIData::asyncHWTrigTask = NULL;
wchar_t _localfile[_MAX_PATH];

///******	End static members	******///

///********************************	Run Synchronous Tasks	********************************///
UINT SaveFileThreadProc(LPVOID pParam)
{
	long ret = TRUE;

	EnterCriticalSection(&saveFileAccess);	
	AcquireNIData::_inSaveThread = TRUE;

	CompoundData* cdTemp = (CompoundData*) pParam;

	size_t read_ai = 0, read_di = 0, read_ci = 0, read_gc = 0;
	int idx_ai = 0, idx_di = 0, idx_ci = 0, idx_vi = 0;

	//check if global counter overflowed pass long time span:
	size_t maxUintValue = UINT_MAX;
	unsigned long timeLapCount = (cdTemp->GetCreateTime() > AcquireNIData::triggerTime) ? static_cast<unsigned long>((cdTemp->GetCreateTime()-AcquireNIData::triggerTime)/(maxUintValue/CLK_RATE_20MHZ)) : 0;
	if(timeLapCount > 0)
	{
		//determine how many missed global counters:
		AcquireNIData::gCtrOverflowCnt += timeLapCount - 1;
		deltaGCtr += maxUintValue - 1 - currentFirstGCtr;
	}
	AcquireNIData::triggerTime = cdTemp->GetCreateTime();

	//check if global counter overflowed in contineous capture:
	if((!skipOverflowCheck) && (*(unsigned long*)(cdTemp->GetgCtr())<currentFirstGCtr))
	{
		AcquireNIData::gCtrOverflowCnt++;
	}
	currentFirstGCtr = *(unsigned long*)(cdTemp->GetgCtr());
	skipOverflowCheck = FALSE;

	//save initial global counter for each batch of acquire sequences:
	if(!initializedGCtr)
	{
		//[USB] 1st gCtr callback not available with sample rate lower than 100 KHz, 
		//all gCtr be 0 and fill with expected counter values and add to offset:
		unsigned long* ptr = cdTemp->GetgCtr();
		if(0 == *(ptr+cdTemp->GetgcLengthValue()-1))
		{
			for (size_t i = 1; i < cdTemp->GetgcLengthValue(); i++)
			{
				ptr++;
				*ptr = static_cast<unsigned long>((i+1) * static_cast<unsigned long>(CLK_RATE_20MHZ/ChannelCenter::getInstance()->_mode.sampleRate) - 1);
			}
			deltaGCtr += *ptr;
		}
		//Combine counter into uInt64, for both CI and global counter:
		skipOverflowCheck = cdTemp->SetupGlobalCounter(AcquireNIData::gCtrOverflowCnt);
		initialGCtr = cdTemp->GetStrucData()->gCtr64Ptr[0];	
		initializedGCtr = true;
	}
	else
	{
		//Combine counter into uInt64 with offset, for both CI and global counter:
		skipOverflowCheck = cdTemp->SetupGlobalCounter(initialGCtr, deltaGCtr, AcquireNIData::gCtrOverflowCnt);
	}

	//compute for virtual channels:
	VirtualTimeChannelManager::getInstance()->Execute(cdTemp);

	//feed for spectral channels before user request stop:
	if(AcquireNIData::_isAcquiring)
		SpectralManager::getInstance()->WriteSource(cdTemp);

	//try to update display buffer:
	if(TryEnterCriticalSection(&displayUpdateAccess))
	{
		CompoundData* _dDataTemp = new CompoundData(cdTemp,AcquireNIData::interleave);
		AcquireNIData::_displayCompData->CopyCompoundData(_dDataTemp);
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireNIData display first counter %u",*(unsigned long*)_dDataTemp->GetStrucData()->gCtr64Ptr);
		LogMessage(message,VERBOSE_EVENT);
		delete _dDataTemp;
		_dDataTemp = NULL;
		LeaveCriticalSection(&displayUpdateAccess);
	}

	//get hdf5 path and filename (which user has defined):
	std::wstring tmpStr = ChannelCenter::getInstance()-> GetEpisodeName();

	//try write to file if user wants to:
	if((TRUE == cdTemp->GetSaving()) && (tmpStr.size()>0))
	{
		CompoundData* cdDataForSave = new CompoundData(cdTemp,AcquireNIData::stimulusParams);

		std::vector<Channels> niChannel = ChannelCenter::getInstance()->_dataChannel;
		if((TRUE == h5io->OpenFileIO(tmpStr.c_str(),H5FileType::READWRITE)) && (cdDataForSave->GetgcLengthComValue() > 0))
		{
			for(int i=0;i<niChannel.size();i++)
			{
				if(0 == niChannel.at(i).type.compare("/AI"))
				{			
					if(cdDataForSave->GetaiLengthValue() > 0)
					{
						if(FALSE == h5io->ExtendData(niChannel.at(i).type.c_str(),("/" + niChannel.at(i).alias).c_str(),cdDataForSave->GetStrucData()->aiDataPtr+idx_ai*cdDataForSave->GetgcLengthComValue(),H5DataType::DATA_DOUBLE,extendBuf,static_cast<unsigned long>(cdDataForSave->GetgcLengthComValue())))
						{
							StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file analog channel at thread (%d)",AcquireNIData::_saveThreadCnt);
							LogMessage(message,ERROR_EVENT);
							//logDll->TLTraceEvent(ERROR_EVENT,1,message);
							ret = FALSE;
						}
						idx_ai++;
					}
					else
					{
						StringCbPrintfW(message,MSG_LENGTH,L"Error reading analog circular buffer at thread (%d)",AcquireNIData::_saveThreadCnt);
						LogMessage(message,ERROR_EVENT);
						ret = FALSE;
					}
				}
				if(0 == niChannel.at(i).type.compare("/DI"))
				{			
					if(cdDataForSave->GetdiLengthValue() > 0)
					{
						if(FALSE == h5io->ExtendData(niChannel.at(i).type.c_str(),("/" + niChannel.at(i).alias).c_str(),cdDataForSave->GetStrucData()->diDataPtr+idx_di*cdDataForSave->GetgcLengthComValue(),H5DataType::DATA_UCHAR,extendBuf,static_cast<unsigned long>(cdDataForSave->GetgcLengthComValue())))
						{
							StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file digital channel at thread (%d)",AcquireNIData::_saveThreadCnt);
							LogMessage(message,ERROR_EVENT);
							ret = FALSE;
						}
						idx_di++;
					}
					else
					{
						StringCbPrintfW(message,MSG_LENGTH,L"Error reading digital circular buffer at thread (%d)",AcquireNIData::_saveThreadCnt);
						LogMessage(message,ERROR_EVENT);
						//ret = FALSE;
					}
				}
				if(0 == niChannel.at(i).type.compare("/CI"))
				{
					if(cdDataForSave->GetciLengthValue() > 0)
					{
						if(FALSE == h5io->ExtendData(niChannel.at(i).type.c_str(),("/" + niChannel.at(i).alias).c_str(),cdDataForSave->GetStrucData()->ciDataPtr+idx_ci*cdDataForSave->GetgcLengthComValue(),H5DataType::DATA_UINT32,extendBuf,static_cast<unsigned long>(cdDataForSave->GetgcLengthComValue())))
						{
							StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file counter channel at thread (%d)",AcquireNIData::_saveThreadCnt);
							LogMessage(message,ERROR_EVENT);
							ret = FALSE;
						}
						idx_ci++;
					}
					else
					{
						StringCbPrintfW(message,MSG_LENGTH,L"Error reading counter circular buffer at thread (%d)",AcquireNIData::_saveThreadCnt);
						LogMessage(message,ERROR_EVENT);
						//ret = FALSE;
					}
				}
			}
			for(int i=0;i<ChannelCenter::getInstance()->_virChannel.size();i++)
			{
				if(SignalType::VIRTUAL == ChannelCenter::getInstance()->_virChannel.at(i).signalType)
				{			
					if(cdDataForSave->GetviLengthValue() > 0)
					{
						if(FALSE == h5io->ExtendData(ChannelCenter::getInstance()->_virChannel.at(i).type.c_str(),("/" + ChannelCenter::getInstance()->_virChannel.at(i).alias).c_str(),cdDataForSave->GetStrucData()->viDataPtr+idx_vi*cdDataForSave->GetgcLengthComValue(),H5DataType::DATA_DOUBLE,extendBuf,static_cast<unsigned long>(cdDataForSave->GetgcLengthComValue())))
						{
							StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file virtual channel at thread (%d)",AcquireNIData::_saveThreadCnt);
							LogMessage(message,ERROR_EVENT);
						}
						idx_vi++;
					}
					else
					{
						StringCbPrintfW(message,MSG_LENGTH,L"Error reading analog circular buffer at thread (%d)",AcquireNIData::_saveThreadCnt);
						LogMessage(message,ERROR_EVENT);
					}
				}
			}
			//Global counter:
			if(cdDataForSave->GetgcLengthComValue() > 0)
			{
				if(FALSE == h5io->ExtendData("/Global",AcquireNIData::counterNames.at(0).c_str(),cdDataForSave->GetStrucData()->gCtr64Ptr, H5DataType::DATA_UINT64,extendBuf,static_cast<unsigned long>(cdDataForSave->GetgcLengthComValue())))
				{
					StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file global counter at thread (%d)",AcquireNIData::_saveThreadCnt);
					LogMessage(message,ERROR_EVENT);
					ret = FALSE;
				}
			}
			else
			{
				StringCbPrintfW(message,MSG_LENGTH,L"Error reading counter circular buffer at thread (%d)",AcquireNIData::_saveThreadCnt);
				LogMessage(message,ERROR_EVENT);
				ret = FALSE;
			}
		}
		else
		{
			ret = FALSE;
		}

		delete cdDataForSave;
		cdDataForSave = NULL;
		h5io->CloseFileIO();
	}
	//Done saving:
	if(!extendBuf)
	{
		extendBuf = true;
	}
	delete cdTemp;
	cdTemp = NULL;
	CloseHandle(AcquireNIData::_hSaveThreads.front());
	AcquireNIData::_hSaveThreads.erase(AcquireNIData::_hSaveThreads.begin());
	AcquireNIData::_saveThreadFinishedCnt++;

	StringCbPrintfW(message,MSG_LENGTH,L"AcquireNIData save file thread done (%d),save Thread Count (%d), save successful (%d)",AcquireNIData::_saveThreadFinishedCnt,AcquireNIData::_saveThreadCnt,ret);
	LogMessage(message,INFORMATION_EVENT);
	printf("AcquireNIData save file thread done (%d),save Thread Count (%d), finished sucessful (%d)\n",AcquireNIData::_saveThreadFinishedCnt,AcquireNIData::_saveThreadCnt,ret);

	AcquireNIData::_inSaveThread = FALSE;	
	LeaveCriticalSection(&saveFileAccess);

	return 0;
}

UINT ProcessSaveThreadProc()
{
	//We create Thread in suspension, finish in order of creation:
	while((TRUE == AcquireNIData::_isAcquiring) || (0 < AcquireNIData::_hSaveThreads.size()))
	{
		//try maintain at least 3 threads in continuous mode, 
		//so that no dereference of list's end:
		while((DAQmx_Val_ContSamps == AcquireNIData::sampleMode) && (TRUE == AcquireNIData::_isAcquiring) && (MIN_THREAD_CNT > AcquireNIData::_hSaveThreads.size()))
		{	
			Sleep(1);			
		}

		if((FALSE == AcquireNIData::_inSaveThread)&& (0 < AcquireNIData::_hSaveThreads.size()))
		{
			if(TryEnterCriticalSection(&saveFileAccess))
			{
				ResumeThread(AcquireNIData::_hSaveThreads.front());	
				LeaveCriticalSection(&saveFileAccess);
			}			
		}			
	}

	//Done processing save threads:
	AcquireNIData::_saveThreadCnt = 0;
	AcquireNIData::_saveThreadFinishedCnt = 0;
	AcquireNIData::_totalNumThreads = 0;

	//h5io->CloseFileIO();

	CloseHandle(AcquireNIData::_hProcessSaveThread);
	AcquireNIData::_hProcessSaveThread = NULL;

	return 0;
}

AcquireNIData::AcquireNIData()
{	
	error = 0;	
	stimulusParams = new StimulusSaveStruct();
}

AcquireNIData::~AcquireNIData()
{	
	SAFE_DELETE_HANDLE(_hAsyncThread);

	SAFE_DELETE_ARRAY(stimulusParams);

	if(AItaskHandle)
	{
		AcquireNIData::StopTask(AItaskHandle,TRUE);
	}
	if(DItaskHandle)
	{	
		AcquireNIData::StopTask(DItaskHandle,TRUE);
	}
	if(CItaskHandles.size()>0)
	{
		AcquireNIData::StopTaskArray(CItaskHandles,TRUE);
	}
	if(DOtaskHandle)
	{	
		AcquireNIData::StopTask(DOtaskHandle,TRUE);
	}
	if(AOtaskHandle)
	{	
		AcquireNIData::StopTask(AOtaskHandle,TRUE);
	}	
	if(AcquireNIData::counterNames.size()>0)
	{
		AcquireNIData::counterNames.clear();
	}
	if(AcquireNIData::_ailineName.size()>0)
	{
		AcquireNIData::_ailineName.clear();
	}
	if(AcquireNIData::_dilineName.size()>0)
	{
		AcquireNIData::_dilineName.clear();
	}
	if(_displayCompData)
	{
		delete _displayCompData;
		_displayCompData = NULL;
	}
	if(AcquireNIData::_hSaveThreads.size()>0)
	{
		std::list<HANDLE>::iterator it = AcquireNIData::_hSaveThreads.begin();
		for(long i=0; i<AcquireNIData::_hSaveThreads.size(); i++)
		{
			CloseHandle(*it);
			//*it = NULL;
			it++;
		}
		AcquireNIData::_hSaveThreads.clear();
	}
}

int32 CVICALLBACK AcquireNIData::DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
	return StaticStop();
}

int32 CVICALLBACK AcquireNIData::EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	long		ret = TRUE;
	const int	ACQ_SIGNAL_CNT = 4;					//[GC, AI, DI, CI]
	int32       err=0;
	char        errBuff[2048]={'\0'};
	int32       readCount[ACQ_SIGNAL_CNT] = {0};

	uInt64		readGC = 0;
	uInt32		availableSampleCount[ACQ_SIGNAL_CNT] = {static_cast<uInt32>(AcquireNIData::_samplesPerCallback), static_cast<uInt32>(AcquireNIData::_samplesPerCallback),
		static_cast<uInt32>(AcquireNIData::_samplesPerCallback),static_cast<uInt32>(AcquireNIData::_samplesPerCallback)};	//available counts from acquired signals
	uInt64		localCtrPerCallback = AcquireNIData::_samplesPerCallback;			//callback count to be limited by available counts
	time_t		eventTime = 0;

	CompoundData* callbackCompData = NULL;
	int ciCtrIdx = (DAQmx_Val_FiniteSamps == AcquireNIData::sampleMode) ? 2 : 1;	//CI counter task index

	BoardInfo* niboard = &ChannelCenter::getInstance()->_board;

	//terminate task if done:
	if((0 < AcquireNIData::_totalNumThreads) && (AcquireNIData::_totalNumThreads <= AcquireNIData::_saveThreadCnt))
	{
		return StaticStop();
	}

	//try catch error if not able to read from device:
	try
	{
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//***	handle special case on continuous counter with 1 value short at beginning or end of finite mode		***// 
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//DAQmxErrChk (L"DAQmxGetReadCurrReadPos",DAQmxGetReadCurrReadPos(CItaskHandles[i],&readGC));
		DAQmxErrChk (L"DAQmxGetReadAvailSampPerChan",err = DAQmxGetReadAvailSampPerChan(CItaskHandles[0],&availableSampleCount[0]));
		//DAQmxErrChk (L"DAQmxGetReadTotalSampPerChanAcquired",DAQmxGetReadTotalSampPerChanAcquired(CItaskHandles[i],&totalSamAcqd));
		if(AcquireNIData::AItaskHandle)
		{
			DAQmxErrChk (L"DAQmxGetReadAvailSampPerChan",err = DAQmxGetReadAvailSampPerChan(AcquireNIData::AItaskHandle,&availableSampleCount[1]));
		}
		if(AcquireNIData::DItaskHandle)
		{
			DAQmxErrChk (L"DAQmxGetReadAvailSampPerChan",err = DAQmxGetReadAvailSampPerChan(AcquireNIData::DItaskHandle,&availableSampleCount[2]));
		}
		if(1 < CItaskHandles.size())
		{			
			DAQmxErrChk (L"DAQmxGetReadAvailSampPerChan",err = DAQmxGetReadAvailSampPerChan(AcquireNIData::CItaskHandles[ciCtrIdx],&availableSampleCount[3]));
		}

		//set callback count based on minimum available amount from acquired signals 
		for (int i = 0; i < ACQ_SIGNAL_CNT; i++)
		{
			if((localCtrPerCallback > availableSampleCount[i]) && (0 < availableSampleCount[i]))
				localCtrPerCallback = availableSampleCount[i];
		}

		callbackCompData = new CompoundData(localCtrPerCallback,(niboard->totalAI)*localCtrPerCallback, (niboard->totalDI)*localCtrPerCallback, localCtrPerCallback, ChannelCenter::getInstance()->_virChannel.size()*localCtrPerCallback);

		//read global counter:
		if(availableSampleCount[0] > 0)
		{
			DAQmxErrChk (L"DAQmxReadCounterU32",err = DAQmxReadCounterU32(CItaskHandles[0],static_cast<int32>(localCtrPerCallback),MAX_TASK_WAIT_TIME,callbackCompData->GetgCtr(),static_cast<uInt32>(localCtrPerCallback),&readCount[0],NULL));
		}
		else
		{
			//[USB] 1st gCtr callback not available 
			//with sample rate lower than 100 KHz, fill with 0 for SaveFileThread:
			memset(callbackCompData->GetgCtr(), 0x0, sizeof(unsigned long)*localCtrPerCallback);
		}

		DWORD testTimeLocal = GetTickCount() - testTime;
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireNIData counter value %u time %d",*(unsigned long*)(callbackCompData->GetgCtr()),testTimeLocal);
		LogMessage(message,VERBOSE_EVENT);

		//read analog channels:	
		if(AcquireNIData::AItaskHandle)
		{
			DAQmxErrChk (L"DAQmxReadAnalogF64",err = DAQmxReadAnalogF64(AcquireNIData::AItaskHandle,static_cast<int32>(localCtrPerCallback),MAX_TASK_WAIT_TIME,DAQmx_Val_GroupByChannel,callbackCompData->GetStrucData()->aiDataPtr,static_cast<uInt32>(localCtrPerCallback*niboard->totalAI),&readCount[1],NULL));
		}
		//read digital channels:
		if(AcquireNIData::DItaskHandle)
		{
			//TODO: this should happen only once per acquisition
			int32* numBytesPerSamp = new int32[niboard->totalDI];

			for (int i = 0; i < niboard->totalDI; ++i)
			{
				numBytesPerSamp[i] = 1;
			}
			DAQmxErrChk(L"DAQmxReadDigitalLines", err = DAQmxReadDigitalLines(AcquireNIData::DItaskHandle, static_cast<int32>(localCtrPerCallback), MAX_TASK_WAIT_TIME, DAQmx_Val_GroupByChannel, callbackCompData->GetStrucData()->diDataPtr, static_cast<uInt32>(localCtrPerCallback * niboard->totalDI), &readCount[2], numBytesPerSamp, NULL));

			delete[] numBytesPerSamp;
		}

		//read CI counter:
		if(1 < CItaskHandles.size())
		{			
			if(availableSampleCount[3] > 0)
			{
				DAQmxErrChk (L"DAQmxReadCounterU32",err = DAQmxReadCounterU32(AcquireNIData::CItaskHandles[ciCtrIdx],static_cast<int32>(localCtrPerCallback),MAX_TASK_WAIT_TIME,callbackCompData->GetStrucData()->ciDataPtr,static_cast<uInt32>(localCtrPerCallback),&readCount[3],NULL));
			}
			else
			{
				//[USB] 1st CI callback not available
				//with sample rate lower than 100 KHz:
				StringCbPrintfW(message,MSG_LENGTH,L"Unable to read counter in current sampling rate.\nPlease disable Counter 01 or use higher sampling rate.");
				goto STOP_W_ERROR;
			}
		}
	}
	catch(...)
	{
		DAQmxGetExtendedErrorInfo(errBuff,MSG_LENGTH);
		LogMessage((wchar_t*)StringToWString(std::string(errBuff)).c_str(), ERROR_EVENT);
		goto STOP_W_ERROR;
	}

	//create thread:
	if (NULL != callbackCompData)
	{
		callbackCompData->SetSaving(AcquireNIData::_isSaving);

		//mark thread creation time:
		if(!initializedGCtr)
		{
			//mark start time in second:
			time(&triggerTime);
		}
		time(&eventTime);
		callbackCompData->SetCreateTime(eventTime);
		DWORD dwThread;
		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) SaveFileThreadProc, (LPVOID)callbackCompData, CREATE_SUSPENDED, &dwThread);
		SetThreadPriority(hThread,THREAD_PRIORITY_NORMAL);
		AcquireNIData::_hSaveThreads.push_back(hThread);
		AcquireNIData::_saveThreadCnt++;

		//Test to read display buffer:
		/*if(AcquireNIData::_saveThreadCnt> 2)
		{
		CompoundDataStruct* testDispCDS = new CompoundDataStruct();
		AcquireNIData::CopyStructData(testDispCDS);	
		printf("ai Length (%d), diLength (%d)\n",testDispCDS->aiLength,testDispCDS->diLength);
		delete testDispCDS;
		}*/
	}
	return ret;

STOP_W_ERROR:
	if(callbackCompData)
	{
		delete callbackCompData;
		callbackCompData = NULL;
	}
	StaticStop();
	//StringCbPrintfW(message,MSG_LENGTH,L"Unable to read data from card due to error: %d" , err);
	StringCbPrintfW(message,MSG_LENGTH,L"Unable to read data from card.\n");
	MessageBox(NULL,message,L"Read Data Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
	return FALSE;
}

void AcquireNIData::DeleteCriticalSections()
{
	DeleteCriticalSection(&saveFileAccess);
	DeleteCriticalSection(&displayUpdateAccess);
	DeleteCriticalSection(&staticStopAccess);
}

long AcquireNIData::EndTasks()
{
	long ret = TRUE;

	if(0 == _primaryChanType.compare("PrimaryAI"))
	{
		StopTask(AItaskHandle, TRUE);
		StopTask(DItaskHandle, TRUE);
	}
	else
	{
		StopTask(DItaskHandle, TRUE);
		StopTask(AItaskHandle, TRUE);
	}

	//StopTask(DOtaskHandle, TRUE);
	//StopTask(AOtaskHandle, TRUE);
	StopTaskArray(CItaskHandles, TRUE);

	return ret;
}

///Enter at loading of application:
long AcquireNIData::Enter()
{
	long ret = TRUE;
	InitializeCriticalSections();

	//Connect invert lines:
	ret = InvertLines(true);

	return ret;
}

long AcquireNIData::Execute()
{
	DWORD dwThread;
	HANDLE hThread;	

	//return if last session is not finished
	if(NULL != AcquireNIData::_hProcessSaveThread)
	{
		return FALSE;
	}

	testTime = GetTickCount();

	AcquireNIData::ExecuteTasks();

	//wait for save thread to be created
	Sleep((DWORD)(2*ChannelCenter::getInstance()->_threadTime*SEC2MSEC));	

	hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) ProcessSaveThreadProc, NULL, NORMAL_PRIORITY_CLASS, &dwThread );	
	SetThreadPriority(hThread,THREAD_PRIORITY_NORMAL);
	AcquireNIData::_hProcessSaveThread = hThread;

	return TRUE;
}

///Exit at unloading of application:
long AcquireNIData::Exit()
{
	long ret = TRUE;
	StaticStop();
	DeleteCriticalSections();

	//Disconnect invert lines:
	ret = InvertLines(false);

	return ret;
}

long AcquireNIData::ExecuteTasks()
{
	long ret = TRUE;
	try
	{
		//Register Events before start tasks:
		DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent",error = DAQmxRegisterEveryNSamplesEvent(AcquireNIData::primarySampleTask,DAQmx_Val_Acquired_Into_Buffer,static_cast<uInt32>(AcquireNIData::_samplesPerCallback),0, AcquireNIData::EveryNCallback,NULL));	
		DAQmxErrChk (L"DAQmxRegisterDoneEvent",error = DAQmxRegisterDoneEvent(AcquireNIData::primarySampleTask,0,AcquireNIData::DoneCallback,NULL));

		//Start tasks in order:
		StartTaskArray(AcquireNIData::CItaskHandles);
		//StartTask(AcquireNIData::DOtaskHandle);
		//StartTask(AcquireNIData::AOtaskHandle);

		if(0 == _primaryChanType.compare("PrimaryAI"))
		{
			StartTask(AcquireNIData::DItaskHandle);
			StartTask(AcquireNIData::AItaskHandle);
		}
		else
		{
			StartTask(AcquireNIData::AItaskHandle);
			StartTask(AcquireNIData::DItaskHandle);
		}

		AcquireNIData::_isAcquiring = TRUE;
		//_isDone = FALSE;
	}
	catch(RunTimeException ex)
	{
		AcquireNIData::_isAcquiring = FALSE;
		std::wstring str = ex.GetExceptionMessage();

		if(0 == str.compare(L"DAQmxTaskControl"))	//||(err == -200587)
		{
			switch (ex.GetErrorCode())
			{
			case DAQmxErrorBufferSizeNotMultipleOfEveryNSampsEventIntervalWhenDMA:	//(-200877): [USB] requires even number multiple of buffer size over eventN samples 
				StringCbPrintfW(message,MSG_LENGTH,L"Unable to start due to non-standard settings.\nPlease try with different callback time or sampling rate.");	
				break;
			default:
				StringCbPrintfW(message,MSG_LENGTH,L"Unable to start due to resources were reserved by other tasks.\nPlease choose lower sampling rate or less duration.");	
				break;
			}
			MessageBox(NULL,message,L"Info", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
		}
	}

	return ret;
}

long AcquireNIData::CopyStructData(void* ptr)
{
	long ret = TRUE;

	try
	{
		EnterCriticalSection(&displayUpdateAccess);
		_displayCompData->GetStrucData((CompDataStruct*)ptr);

	}
	catch(...)
	{
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireNIData unable to get display data");
		LogMessage(message,WARNING_EVENT);
		ret = FALSE;
	}

	LeaveCriticalSection(&displayUpdateAccess);
	return ret;
}

long AcquireNIData::GetErrorMessage()
{
	char errBuff[2048]={'\0'};
	int32 err =0;
	if( DAQmxFailed(err) ) 
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		StringCbPrintfW(message,MSG_LENGTH,L"DAQmx Error: %s\n",errBuff);
		LogMessage(message,ERROR_EVENT);
		//printf("DAQmx Error: %s\n",errBuff);
	}
	return TRUE;
}

std::wstring AcquireNIData::GetLastError()
{	
	return lastError;
}

//int32 AcquireNIData::GetTerminalNameWithDevPrefix(TaskHandle taskHandle, const char terminalName[], char sampleName[])
//{
//	int32	error=0;
//	char	device[256];
//	int32	productCategory;
//	uInt32	numDevices,i=1;
//
//	DAQmxErrChk (L"DAQmxGetTaskNumDevices",DAQmxGetTaskNumDevices(taskHandle,&numDevices));
//	while( i<=numDevices ) 
//	{
//		DAQmxErrChk (L"DAQmxGetNthTaskDevice",DAQmxGetNthTaskDevice(taskHandle,i++,device,256));
//		DAQmxErrChk (L"DAQmxGetDevProductCategory",DAQmxGetDevProductCategory(device,&productCategory));
//		if( productCategory!=DAQmx_Val_CSeriesModule && productCategory!=DAQmx_Val_SCXIModule ) 
//		{
//			*sampleName++ = '/';
//			strcat(strcat(strcpy(sampleName,device),"/"),terminalName);
//			break;
//		}
//	}
//	return TRUE;
//}

std::string AcquireNIData::GetNIDeviceAttribute(std::string devName, int32 attribute)
{
	int32 errVal=0;
	int buffersize;
	char* input = NULL;
	std::string str="";
	buffersize=DAQmxGetDeviceAttribute(devName.c_str(),attribute,NULL);
	if(buffersize > 0)	//-200604 DAQmxErrorNULLPtr
	{
		input = (char*)malloc(buffersize);
		errVal = DAQmxGetDeviceAttribute(devName.c_str(),attribute,input,buffersize);
		str = input;
	}
	if(input)
	{
		free(input);
		input = NULL;
	}

	return str;
}

long AcquireNIData::GetAcquiring()
{	
	return (AcquireNIData::_isAcquiring || (NULL != AcquireNIData::_hProcessSaveThread));
}

long AcquireNIData::GetSaving()
{	
	return AcquireNIData::_isSaving;
}

long AcquireNIData::InitCallbacks(SpectralUpdateCallback su, DataUpdateCallback du)
{
	functionPointer = su;
	dataPointer = du;
	if(functionPointer != NULL)
	{
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireNIData InitCallbacks");
		LogMessage(message,VERBOSE_EVENT);
	}
	return TRUE;
}

void AcquireNIData::InitializeCriticalSections()
{
	InitializeCriticalSection(&saveFileAccess);
	InitializeCriticalSection(&displayUpdateAccess);
	InitializeCriticalSection(&staticStopAccess);
}

/// each connect pairs will occupy one counter, it will occupy all counters when connect more than 4 pairs
long AcquireNIData::InvertLines(bool connect)
{
	long ret = TRUE;
	int32 retVal = 0;

	BoardInfo* board = &ChannelCenter::getInstance()->_board;
	Invert* invert = &ChannelCenter::getInstance()->_invert;

	if(FALSE == VerifyNIBoard(board))
		return FALSE;

	//Do reset device at enter & exit:
	DAQmxErrChk (L"DAQmxResetDevice",retVal = DAQmxResetDevice(board->devID.c_str()));

	//Check if settings avaiable, default not connected 
	//by setting 10 instead of /PFI10:
	if((5 > invert->inputLine.size()) || (0 != invert->inputLine.find_first_of("/")) 
		|| (5 > invert->outputLine.size()) || (0 != invert->outputLine.find_first_of("/")))
	{
		return FALSE;
	}
	invert->inputLine = "/" + board->devID + invert->inputLine;
	invert->outputLine = "/" + board->devID + invert->outputLine;		
	if((FALSE == AcquireNIData::VerifyNILine(NULL,SignalType::PFI,invert->inputLine)) || (FALSE == AcquireNIData::VerifyNILine(NULL,SignalType::PFI,invert->outputLine)))
	{
		return FALSE;
	}
	if(connect)
	{
		if(_invLines.size() > 0)
		{
			_invLines.clear();
		}
		_invLines.push_back(invert->inputLine);
		_invLines.push_back(invert->outputLine);
		retVal = DAQmxConnectTerms (_invLines.at(0).c_str(), _invLines.at(1).c_str(),DAQmx_Val_InvertPolarity);
		_invertEnabled = (0 == retVal) ? true : false;
	}
	else
	{
		if(_invLines.size() > 0)
		{
			retVal = DAQmxDisconnectTerms(_invLines.at(0).c_str(), _invLines.at(1).c_str());
			_invLines.clear();
			_invertEnabled = false;
		}
	}
	return ret;
}

long AcquireNIData::LoadXML()
{
	long ret = TRUE;
	ResetParams();
	ResetTimingParams();
	if(FALSE == ChannelCenter::getInstance()->LoadXML())
		return FALSE;

	BoardInfo* niboard = &ChannelCenter::getInstance()->_board;
	if(FALSE == VerifyNIBoard(niboard))
		return FALSE;

	Mode* nimode = &ChannelCenter::getInstance()->_mode;
	AcquireNIData::interleave = nimode->interleave;
	_samplesPerCallback = static_cast<uInt64>(nimode->sampleRate*ChannelCenter::getInstance()->_threadTime);
	AcquireNIData::_totalNumThreads = (0 < nimode->duration) ? static_cast<size_t>(ceil(nimode->duration/ChannelCenter::getInstance()->_threadTime)) : 0;

	//determine sample mode contineous or finite,
	//finite buffer will overflow beyond count of INT32: //unit: [sec]
	if((nimode->duration > 0) && (INT_MAX > (AcquireNIData::_totalNumThreads*nimode->sampleRate)) && (nimode->hwTrigMode != HWTriggerMode::HW_SYNCHRONIZABLE))
	{
		sampleMode = DAQmx_Val_FiniteSamps;
		//_samplesPerChannel = static_cast<uInt64>(floor(nimode->sampleRate*_threadTime + 0.5)/_threadTime*nimode->duration);
		_samplesPerChannel = static_cast<uInt64>(floor(nimode->sampleRate)*nimode->duration);
		AcquireNIData::_totalNumThreads = 0;
	}
	else
	{
		sampleMode = DAQmx_Val_ContSamps;
		_samplesPerChannel = _samplesPerCallback;
	}

	//determine trigger mode:
	std::string tempLine;
	switch(nimode->hwTrigMode)
	{
	case HWTriggerMode::SW_FREERUN:	//SW trigger:
		break;
	case HWTriggerMode::HW_TRIGGER_SINGLE:	//HW trigger once:
		_analogTrig = (HWTriggerType::ANALOG_INPUT == nimode->hwTrigType) ? true : false;
		AcquireNIData::triggerChannel = (true == _analogTrig) ? (niboard->devID + "/ai0") : ("/" + niboard->devID + nimode->hwTrigChannel);	
		_retrig = false;
		break;
	case HWTriggerMode::HW_RETRIGGERABLE:	//HW trigger each, only finite mode:
		_analogTrig = (HWTriggerType::ANALOG_INPUT == nimode->hwTrigType) ? true : false;
		AcquireNIData::triggerChannel = (true == _analogTrig) ? (niboard->devID + "/ai0") : ("/" + niboard->devID + nimode->hwTrigChannel);	
		_retrig = (DAQmx_Val_FiniteSamps == sampleMode) ? true : false;
		break;
	case HWTriggerMode::HW_SYNCHRONIZABLE:	//HW Synchronized, allow pause on contineous mode only:
		_analogTrig = (HWTriggerType::ANALOG_INPUT == nimode->hwTrigType) ? true : false;
		if(_analogTrig)
		{
			//Reason: Analog pause trigger can only be applied on itself, no other digital or counters.
			lastError = L"HW Synchronizable is not available with analog trigger. ";
			return FALSE;
		}
		else
		{
			AcquireNIData::triggerChannel = "/" + niboard->devID + nimode->hwTrigChannel;	
		}
		_pauseTrig = true;
		break;
	default:
		lastError = L"Invalid trigger mode. ";
		return FALSE;
	}

	//(0 == _samplesPerCallback % _interleave) is required:
	interleave = nimode->interleave;

	//virtual, spectral & spectral virtual channels:
	VirtualTimeChannelManager::getInstance()->UpdateProcessor();
	SpectralManager::getInstance()->SetSaveFile(FALSE);
	SpectralManager::getInstance()->UpdateSpectralAnalyzer();


	_displayPerCallback = (interleave > 0) ? static_cast<uInt64>(_samplesPerCallback/interleave) : static_cast<uInt64>(_samplesPerCallback);	
	_displayCompData = new CompoundData(_displayPerCallback,(niboard->totalAI)*_displayPerCallback,(niboard->totalDI)*_displayPerCallback, _displayPerCallback, ChannelCenter::getInstance()->_virChannel.size()*_displayPerCallback);
	_displayCompData->SetSaving(FALSE);

	return ret;
}

///Pause and Restart was not functional due to unknown NI error,
///drop these two functions...
long AcquireNIData::Pause()
{
	//stop tasks other than global counter:
	if(AcquireNIData::CItaskHandles.size()>1)
	{
		for(int i=1;i<AcquireNIData::CItaskHandles.size();i++)
		{
			StopTask(AcquireNIData::CItaskHandles.at(i), FALSE);
		}
	}
	//StopTask(AcquireNIData::DOtaskHandle, FALSE);
	//StopTask(AcquireNIData::AOtaskHandle, FALSE);

	StopTask(AcquireNIData::DItaskHandle, FALSE);
	StopTask(AcquireNIData::AItaskHandle, FALSE);

	_isAcquiring = FALSE;

	return TRUE;
}

long AcquireNIData::Restart()
{
	DWORD dwThread;
	HANDLE hThread;	

	//Restart tasks in order:
	if(AcquireNIData::CItaskHandles.size()>1)
	{
		for(int i=1;i<AcquireNIData::CItaskHandles.size();i++)
		{
			StartTask(AcquireNIData::CItaskHandles[i]);
		}
	}
	//StartTask(AcquireNIData::DOtaskHandle);
	//StartTask(AcquireNIData::AOtaskHandle);

	if(0 == _primaryChanType.compare("PrimaryAI"))
	{
		StartTask(AcquireNIData::DItaskHandle);
		StartTask(AcquireNIData::AItaskHandle);
	}
	else
	{
		StartTask(AcquireNIData::AItaskHandle);
		StartTask(AcquireNIData::DItaskHandle);
	}

	_isAcquiring = TRUE;
	//_isDone = FALSE;

	//wait for save thread to be created:
	Sleep((DWORD)(2*ChannelCenter::getInstance()->_threadTime*SEC2MSEC));	

	hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) ProcessSaveThreadProc, NULL, NORMAL_PRIORITY_CLASS, &dwThread );	
	SetThreadPriority(hThread,THREAD_PRIORITY_NORMAL);
	AcquireNIData::_hProcessSaveThread = hThread;

	return TRUE;
}

void AcquireNIData::ResetParams()
{
	//terminate previous tasks if exist:
	AcquireNIData::primarySampleTask = NULL;
	_primaryChanType="";
	AcquireNIData::triggerChannel="";
	AcquireNIData::Stop();

	//reset default params:
	counterNames.clear();
	_ailineName.clear();
	_dilineName.clear();
	extendBuf = false;
	switchCounterTask = false;
	skipOverflowCheck = TRUE;
	AcquireNIData::stimulusParams->enable = false;
	AcquireNIData::stimulusParams->signalType = 0;
	AcquireNIData::stimulusParams->stimChannelID = 0;
	AcquireNIData::stimulusParams->threshold = 0;
	AcquireNIData::stimulusParams->lineName = "";
	_retrig = false;
	_pauseTrig = false;
	_analogTrig = false;

	if(_displayCompData)	//DO NOT using SAFE_DELETE_MEMORY on CompoundData
	{
		delete _displayCompData;
		_displayCompData = NULL;
	}
}

void AcquireNIData::ResetTimingParams()
{
	//reset timing info,
	//don't do this if trying to run restart:
	initialGCtr = 0;
	currentFirstGCtr = 0;
	initializedGCtr = false;
	gCtrOverflowCnt = 0;
	skipOverflowCheck = FALSE;
	deltaGCtr = 0;
	triggerTime = 0;
}

long AcquireNIData::SetSaving(long toSave)
{
	AcquireNIData::_isSaving = toSave;
	return TRUE;
}

long AcquireNIData::SetupFileIO()
{	
	if(FALSE == SetupHDF5File())
	{
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireNIData unable to create HDF5 File");
		LogMessage(message, WARNING_EVENT);
		return FALSE;
	}
	return TRUE;
}

long AcquireNIData::SetupHDF5File()
{
	if(AcquireNIData::_isSaving)
	{
		if(FALSE == ChannelCenter::getInstance()->SetupFileH5(AcquireNIData::_samplesPerCallback))
		{
			lastError = ChannelCenter::getInstance()->GetLastError();
			StringCbPrintfW(message,MSG_LENGTH,L"Simulator unable to create HDF5 File");
			LogMessage(message, WARNING_EVENT);
			return FALSE;
		}
	}
	return TRUE;
}

long AcquireNIData::Start()
{
	return AcquireNIData::Execute();
}

long AcquireNIData::StartTaskArray(taskHandles handles)
{
	long ret = TRUE;
	if(handles.size()>0)
	{
		for(int i=0;i<handles.size();i++)
		{
			DAQmxErrChk (L"DAQmxTaskControl",error = DAQmxTaskControl(handles[i],DAQmx_Val_Task_Reserve));
			//retVal = DAQmxStartTask(handles[i]);			
			DAQmxErrChk (L"DAQmxStartTask",error = DAQmxStartTask(handles[i]));
		}
	}

	return ret;
}

long AcquireNIData::StartTask(TaskHandle handle)
{
	long ret = TRUE;
	if(handle)
	{
		DAQmxErrChk (L"DAQmxTaskControl",error = DAQmxTaskControl(handle,DAQmx_Val_Task_Reserve));
		//retVal = DAQmxStartTask(handle);
		DAQmxErrChk (L"DAQmxStartTask",error = DAQmxStartTask(handle));
	}
	return ret;
}

///Static function to execute stop:
long AcquireNIData::StaticStop()
{
	long ret = TRUE;
	EnterCriticalSection(&staticStopAccess);

	AcquireNIData::GetErrorMessage();

	AcquireNIData::EndTasks();
	AcquireNIData::_isAcquiring = FALSE;

	if((NULL != AcquireNIData::_hProcessSaveThread) && (WaitForSingleObject(AcquireNIData::_hProcessSaveThread, INFINITE) == WAIT_OBJECT_0))
	{
		h5io->DestroyFileIO();
	}

	LeaveCriticalSection(&staticStopAccess);
	return ret;
}

long AcquireNIData::Stop()
{
	return StaticStop();
}

long AcquireNIData::StopTaskArray(taskHandles &handles,long destroy)
{
	long ret = TRUE;
	if(handles.size()>0)
	{
		for(int i=0;i<handles.size();i++)
		{
			if(NULL != handles[i])
			{
				DAQmxErrChk (L"DAQmxStopTask",DAQmxStopTask(handles[i]));
			}
		}					
	}	
	if(destroy)
	{
		for(int i=0;i<handles.size();i++)
		{
			if(NULL != handles[i])
			{
				DAQmxErrChk (L"DAQmxClearTask",DAQmxClearTask(handles[i]));
				handles[i] = NULL;
			}
		}
		handles.clear();
	}
	return ret;
}

long AcquireNIData::StopTask(TaskHandle &handle,long destroy)
{
	long ret = TRUE;

	if(NULL != handle)
	{
		DAQmxErrChk (L"DAQmxStopTask",DAQmxStopTask(handle));
		//DAQmxErrChk (L"DAQmxClearTask",DAQmxClearTask(handle));
		if(destroy)
		{
			DAQmxErrChk (L"DAQmxClearTask",DAQmxClearTask(handle));
			handle = NULL;
		}
	}

	return ret;
}

long AcquireNIData::SetupChannels()
{
	long ret = TRUE;

	if(FALSE == SetupLineNames())
	{
		return FALSE;
	}

	if(FALSE == SetupLocalChannels())
	{
		return FALSE;
	}

	ret = SetupGlobalChannels();

	//always keep global counter at front:
	if((switchCounterTask) && (AcquireNIData::CItaskHandles.size()>1))
	{
		for(int i=static_cast<int>(AcquireNIData::CItaskHandles.size()-1);i>0;i--)
		{
			std::swap(CItaskHandles[i-1],CItaskHandles[i]);
		}		
	}

	//configure buffer size for contineous channels:
	SetupCfgBufferForContTasks();

	//set start trigger:	
	if(ChannelCenter::getInstance()->_mode.hwTrigMode > 0)
	{
		ret = SetupStartTrigger(AItaskHandle,_retrig);
		if(!_analogTrig)
		{
			ret = SetupStartTrigger(DItaskHandle,_retrig);
		}
		//ret = SetupStartTrigger(AOtaskHandle,_retrig);
		//ret = SetupStartTrigger(DOtaskHandle,_retrig);

		if((CItaskHandles.size()>1) && (DAQmx_Val_FiniteSamps == sampleMode))
		{			
			ret = SetupStartTrigger(CItaskHandles[1],_retrig);
			//ret = SetupPauseTrigger(CItaskHandles[1],_pauseTrig);		//No pause trigger on counter output.
		}		
	}

	//set arm start trigger:
	if(CItaskHandles.size()>1)
	{
		if(DAQmx_Val_FiniteSamps == sampleMode)
		{
			ret = SetupArmStartTrigger(CItaskHandles[2],AcquireNIData::sampleChannel);
			ret = SetupPauseTrigger(CItaskHandles[2],_pauseTrig);
		}
		else
		{
			ret = SetupArmStartTrigger(CItaskHandles[1],AcquireNIData::sampleChannel);
			ret = SetupPauseTrigger(CItaskHandles[1],_pauseTrig);
		}
	}

	//arm start global counter:
	ret = SetupArmStartTrigger(CItaskHandles[0],AcquireNIData::sampleChannel);
	//ignore buffer overflow error:
	//ret = SetSamClkBehavior(CItaskHandles[0]);

	//pause trigger of primary sample clock if necessary:
	if(_pauseTrig)
	{
		SetupPauseTrigger(AItaskHandle,_pauseTrig);
		SetupPauseTrigger(DItaskHandle,_pauseTrig);
	}

	return ret;
}

///use one counter for contineous counting:
long AcquireNIData::SetupCtrContineousMode()
{
	long ret = TRUE;
	TaskHandle tmpHandleCI = NULL;
	std::string devName = "/"+ ChannelCenter::getInstance()->_board.devID; 
	std::string lineName;

	DAQmxErrChk (L"DAQmxCreateTask",DAQmxCreateTask("DICtr",&tmpHandleCI));
	lineName = devName + "/ctr0";
	if(FALSE ==	AcquireNIData::VerifyNILine(NULL,SignalType::COUNTER,lineName))
	{
		return FALSE;
	}
	DAQmxErrChk (L"DAQmxCreateCICountEdgesChan",DAQmxCreateCICountEdgesChan(tmpHandleCI,lineName.c_str(),"",DAQmx_Val_Rising,0,DAQmx_Val_CountUp));
	lineName = devName + AcquireNIData::counterNames.at(1);
	DAQmxErrChk (L"DAQmxSetCICountEdgesTerm",DAQmxSetCICountEdgesTerm(tmpHandleCI,"",lineName.c_str()));
	DAQmxErrChk (L"DAQmxCfgSampClkTiming",DAQmxCfgSampClkTiming(tmpHandleCI,AcquireNIData::sampleChannel,ChannelCenter::getInstance()->_mode.sampleRate,DAQmx_Val_Rising,sampleMode,_samplesPerCallback));
	DAQmxErrChk (L"DAQmxSetCIDataXferMech",DAQmxSetCIDataXferMech(tmpHandleCI,"",DAQmx_Val_DMA));

	CItaskHandles.push_back(tmpHandleCI);
	tmpHandleCI = NULL;
	return ret;
}

///use one counter for CO gating time on counting one,
///may need post-processing of overflow (0).
long AcquireNIData::SetupCtrFiniteMode()
{
	long ret = TRUE;
	int32 retVal = 0;
	TaskHandle tmpHandleCI = NULL;
	std::string devName = "/"+ ChannelCenter::getInstance()->_board.devID; 
	std::string lineName;

	if((true == _invertEnabled) && (NULL != AcquireNIData::asyncHWTrigTask))
	{
		lastError = L"When invert is enabled, to enable Counter 01 in finite mode conflicts with HW triggering of bleach. ";
		return FALSE;
	}

	//InvertLines(false);

	DAQmxErrChk (L"DAQmxCreateTask",DAQmxCreateTask("",&tmpHandleCI));
	lineName = devName +"/ctr2";
	if(FALSE ==	AcquireNIData::VerifyNILine(NULL,SignalType::COUNTER,lineName))
	{
		return FALSE;
	}
	float64 delayTime = 0.00005;
	float64 ctrGateOnTime = static_cast<float64>(ChannelCenter::getInstance()->_threadTime*_samplesPerChannel/_samplesPerCallback);
	DAQmxErrChk (L"DAQmxCreateCOPulseChanTime",retVal = DAQmxCreateCOPulseChanTime(tmpHandleCI,lineName.c_str(),"",DAQmx_Val_Seconds,DAQmx_Val_Low,delayTime,delayTime,ctrGateOnTime+delayTime));
	DAQmxErrChk (L"DAQmxWaitUntilTaskDone",retVal = DAQmxWaitUntilTaskDone(tmpHandleCI,MAX_TASK_WAIT_TIME));
	//retVal = DAQmxTaskControl(tmpHandleCI,DAQmx_Val_Task_Reserve);
	CItaskHandles.push_back(tmpHandleCI);
	tmpHandleCI = NULL;

	DAQmxErrChk (L"DAQmxCreateTask",DAQmxCreateTask("DICtr",&tmpHandleCI));
	lineName = devName + "/ctr0";
	if(FALSE ==	AcquireNIData::VerifyNILine(NULL,SignalType::COUNTER,lineName))
	{
		return FALSE;
	}
	DAQmxErrChk (L"DAQmxCreateCICountEdgesChan",DAQmxCreateCICountEdgesChan(tmpHandleCI,lineName.c_str(),"",DAQmx_Val_Rising,0,DAQmx_Val_CountUp));
	lineName = devName + AcquireNIData::counterNames.at(1);
	DAQmxErrChk (L"DAQmxSetCICountEdgesTerm",DAQmxSetCICountEdgesTerm(tmpHandleCI,"",lineName.c_str()));
	DAQmxErrChk (L"DAQmxCfgSampClkTiming",DAQmxCfgSampClkTiming(tmpHandleCI,AcquireNIData::sampleChannel,ChannelCenter::getInstance()->_mode.sampleRate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,_samplesPerCallback));
	SetDataTransferMech(tmpHandleCI, SignalType::COUNTER_IN);
	lineName = devName + "/PFI14";	///***Ctr2 output channel***///
	DAQmxErrChk (L"DAQmxSetPauseTrigType",DAQmxSetPauseTrigType(tmpHandleCI,DAQmx_Val_DigLvl));
	DAQmxErrChk (L"DAQmxSetDigLvlPauseTrigSrc",DAQmxSetDigLvlPauseTrigSrc(tmpHandleCI,lineName.c_str()));
	DAQmxErrChk (L"DAQmxSetDigLvlPauseTrigWhen",DAQmxSetDigLvlPauseTrigWhen(tmpHandleCI,DAQmx_Val_Low));

	CItaskHandles.push_back(tmpHandleCI);
	tmpHandleCI = NULL;	

	//InvertLines(true);
	return ret;
}

///prepare all line names for acquisition:
long AcquireNIData::SetupLineNames()
{
	long ret = TRUE;
	std::vector<Channels> niChannel = ChannelCenter::getInstance()->_dataChannel;
	if(0 == niChannel.size())
	{
		lastError = L"No channels being specified. ";
		return FALSE;
	}

	std::string devName = ChannelCenter::getInstance()->_board.devID;
	int nAI=0, nDI=0, nCI=0;
	//default global counter name:
	AcquireNIData::counterNames.push_back("/GCtr");

	for(int i=0;i<niChannel.size();i++)
	{
		//Verify user line settings on NI Card:
		if(FALSE ==	AcquireNIData::VerifyNILine(&niChannel.at(i),0,""))
		{
			return FALSE;
		}
		//all listed channels will be captured:		
		long channelType = 0;
		if(0 == niChannel.at(i).type.compare("/AI"))
		{	
			_ailineName.push_back(devName + niChannel.at(i).lineId);
			if(TRUE == niChannel.at(i).sample)
			{
				_primaryChanType = "PrimaryAI";
			}
			if(niChannel.at(i).Stimulus > 0)
			{
				AcquireNIData::stimulusParams->enable=true;
				AcquireNIData::stimulusParams->signalType=0;
				AcquireNIData::stimulusParams->stimChannelID = nAI;
				AcquireNIData::stimulusParams->threshold = ChannelCenter::getInstance()->_mode.StimulusLimit;
				AcquireNIData::stimulusParams->lineName = "/" + devName + niChannel.at(i).lineId;
			}
			//if(AcquireNIData::niChannel.at(i).aiTrigger > 0)
			//{
			//	AcquireNIData::triggerChannel = niboard->devID + AcquireNIData::niChannel.at(i).lineId;	
			//}
			nAI++;
		}
		if(0 == niChannel.at(i).type.compare("/DI"))
		{	
			_dilineName.push_back(devName + niChannel.at(i).lineId);
			if(TRUE == niChannel.at(i).sample)
			{
				_primaryChanType = "PrimaryDI";
			}
			if(niChannel.at(i).Stimulus > 0)
			{
				AcquireNIData::stimulusParams->enable=true;
				AcquireNIData::stimulusParams->signalType=1;
				AcquireNIData::stimulusParams->stimChannelID = nDI;
				AcquireNIData::stimulusParams->threshold = 0;
				AcquireNIData::stimulusParams->lineName = "/" + devName + niChannel.at(i).lineId;
			}
			nDI++;
		}
		if(0 == niChannel.at(i).type.compare("/DO"))
		{	
		}
		if(0 == niChannel.at(i).type.compare("/CI"))
		{	
			AcquireNIData::counterNames.push_back(niChannel.at(i).lineId);
			switchCounterTask = true;
			nCI++;
		}
		if(0 == niChannel.at(i).type.compare("/AO"))
		{	
		}		
	}

	//Sample channel is now non-configurable, and default to be primaryAI. 
	//Consider no ai is selected, go with primaryDI:
	if(0 == _primaryChanType.size())
	{ 
		_primaryChanType = (_ailineName.size()>0) ? "PrimaryAI" : "PrimaryDI";
	}

	/*NOTE: user input validation; now is repeated in GUI (RealTimeLineChart)*/
	if((ChannelCenter::getInstance()->_board.totalAI != nAI) || (ChannelCenter::getInstance()->_board.totalDI != nDI))
	{
		lastError = L"Inconsistent total channel numbers. ";
		ret = FALSE;
	}
	if(_analogTrig && (nCI > 0) && (DAQmx_Val_FiniteSamps == sampleMode))
	{
		//Reason: Counters are not analog-triggerable.
		lastError = L"Finite counter is not availble under analog HW trigger. ";
		ret = FALSE;
	}
	if(_analogTrig && (nAI > 1) && _retrig)
	{   
		//Reason: AD convertor will fail sometime after execution and cause error at very next run.
		lastError = L"Only the analog trigger channel is allowed in retriggerable finite mode. ";
		ret = FALSE;
	}
	return ret;
}

long AcquireNIData::SetupLocalChannels()
{
	long ret = TRUE;
	int32 retVal = 0;
	std::string devName = "/"+ChannelCenter::getInstance()->_board.devID; 
	std::string lineName;
	Mode* nimode = &ChannelCenter::getInstance()->_mode;

	if(0 == ChannelCenter::getInstance()->_dataChannel.size())
	{
		return FALSE;
	}

	if(_ailineName.size()>0)
	{
		long channelType = 0;

		//combine all ai line names:
		lineName = _ailineName.at(0);
		for(int j=1;j<_ailineName.size();j++)
		{
			lineName +="," +_ailineName.at(j);
		}
		//setup primary sample task if not exist:
		if((AcquireNIData::primarySampleTask == NULL)&&(0 == _primaryChanType.compare("PrimaryAI")))
		{	
			DAQmxErrChk (L"DAQmxCreateTask",DAQmxCreateTask(_primaryChanType.c_str(),&AcquireNIData::AItaskHandle));
			DAQmxErrChk (L"DAQmxCreateAIVoltageChan",DAQmxCreateAIVoltageChan(AcquireNIData::AItaskHandle,lineName.c_str(),"",DAQmx_Val_Cfg_Default,MIN_VOLTAGE,MAX_VOLTAGE,DAQmx_Val_Volts,NULL));
			//DAQmxErrChk (L"GetTerminalNameWithDevPrefix",GetTerminalNameWithDevPrefix(AcquireNIData::AItaskHandle,"ai/SampleClock",AcquireNIData::sampleChannel));
			lineName = devName + "/ai/SampleClock";
			strcpy_s<_MAX_FNAME>(AcquireNIData::sampleChannel,lineName.c_str());
			AcquireNIData::primarySampleTask = AcquireNIData::AItaskHandle;					
		}
		else
		{
			if(NULL == AcquireNIData::AItaskHandle)
			{
				DAQmxErrChk (L"DAQmxCreateTask",DAQmxCreateTask("",&AcquireNIData::AItaskHandle));
				DAQmxErrChk (L"DAQmxCreateAIVoltageChan",DAQmxCreateAIVoltageChan(AcquireNIData::AItaskHandle,lineName.c_str(),"",DAQmx_Val_Cfg_Default,MIN_VOLTAGE,MAX_VOLTAGE,DAQmx_Val_Volts,NULL));
			}
		}				
	}

	if(_dilineName.size()>0)
	{
		//combine all di line names:
		lineName = _dilineName.at(0);
		for(int j=1;j<_dilineName.size();j++)
		{
			lineName +="," +_dilineName.at(j);
		}
		//setup primary sample task if not exist:
		if((AcquireNIData::primarySampleTask == NULL)&&(0 == _primaryChanType.compare("PrimaryDI")))
		{	
			DAQmxErrChk (L"DAQmxCreateTask",DAQmxCreateTask(_primaryChanType.c_str(),&AcquireNIData::DItaskHandle));
			DAQmxErrChk (L"DAQmxCreateDIChan",DAQmxCreateDIChan(AcquireNIData::DItaskHandle,lineName.c_str(),"",DAQmx_Val_ChanPerLine));
			//DAQmxErrChk (L"GetTerminalNameWithDevPrefix",GetTerminalNameWithDevPrefix(AcquireNIData::DItaskHandle,"di/SampleClock",AcquireNIData::sampleChannel));
			lineName = devName + "/di/SampleClock";
			strcpy_s<_MAX_FNAME>(AcquireNIData::sampleChannel,lineName.c_str());
			AcquireNIData::primarySampleTask = AcquireNIData::DItaskHandle;
		}
		else
		{
			if(NULL == AcquireNIData::DItaskHandle)
			{
				DAQmxErrChk (L"DAQmxCreateTask",DAQmxCreateTask("",&AcquireNIData::DItaskHandle));
				DAQmxErrChk (L"DAQmxCreateDIChan",DAQmxCreateDIChan(AcquireNIData::DItaskHandle,lineName.c_str(),"",DAQmx_Val_ChanPerLine));
			}
		}
	}

	if(true == switchCounterTask)
	{
		if(DAQmx_Val_ContSamps == sampleMode)
		{
			SetupCtrContineousMode();
		}
		else
		{
			if(FALSE ==SetupCtrFiniteMode())
			{
				return FALSE;
			}
		}
	}
	//Configure Timing:
	if(0 == _primaryChanType.compare("PrimaryAI"))
	{
		DAQmxErrChk (L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(AcquireNIData::AItaskHandle,"",nimode->sampleRate,DAQmx_Val_Rising,sampleMode,_samplesPerChannel));
		if(AcquireNIData::DItaskHandle)
		{
			if(_analogTrig && _retrig)		//analog retriggerable requires DI to be contineous mode to avoid error 200278
			{	
				DAQmxErrChk (L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(AcquireNIData::DItaskHandle,AcquireNIData::sampleChannel,nimode->sampleRate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,_samplesPerCallback));
			}
			else
			{
				DAQmxErrChk (L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(AcquireNIData::DItaskHandle,AcquireNIData::sampleChannel,nimode->sampleRate,DAQmx_Val_Rising,sampleMode,_samplesPerChannel));
			}

			SetDataTransferMech(AcquireNIData::DItaskHandle, SignalType::DIGITAL_IN);
		}

		SetDataTransferMech(AcquireNIData::AItaskHandle, SignalType::ANALOG_IN);
	}
	else
	{
		if(AcquireNIData::AItaskHandle)
		{
			DAQmxErrChk (L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(AcquireNIData::AItaskHandle,AcquireNIData::sampleChannel,nimode->sampleRate,DAQmx_Val_Rising,sampleMode,_samplesPerChannel));

			SetDataTransferMech(AcquireNIData::AItaskHandle, SignalType::ANALOG_IN);
		}
		DAQmxErrChk (L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(AcquireNIData::DItaskHandle,"",nimode->sampleRate,DAQmx_Val_Rising,sampleMode,_samplesPerChannel));

		SetDataTransferMech(AcquireNIData::DItaskHandle, SignalType::DIGITAL_IN);
	}

	return ret;
}

long AcquireNIData::SetupGlobalChannels()
{
	long ret = TRUE;
	TaskHandle tmpC1Handle = NULL;
	std::string devName = "/"+ChannelCenter::getInstance()->_board.devID; 
	std::string lineName1,lineName2;

	//Global Counter: (Drop two counters feature, use single counter and handle overflow by ourselves.)
	DAQmxErrChk (L"DAQmxCreateTask",DAQmxCreateTask("GlobalCtr",&tmpC1Handle));
	lineName1 = devName + "/ctr1";
	if(FALSE ==	AcquireNIData::VerifyNILine(NULL,SignalType::COUNTER,lineName1))
	{
		return FALSE;
	}
	DAQmxErrChk (L"DAQmxCreateCICountEdgesChan",error = DAQmxCreateCICountEdgesChan(tmpC1Handle,lineName1.c_str(),"",DAQmx_Val_Rising,0,DAQmx_Val_CountUp));
	lineName2 = devName + "/20MHzTimebase";
	DAQmxErrChk (L"DAQmxSetCICountEdgesTerm",error = DAQmxSetCICountEdgesTerm(tmpC1Handle,lineName1.c_str(),lineName2.c_str()));
	DAQmxErrChk (L"DAQmxCfgSampClkTiming",error = DAQmxCfgSampClkTiming(tmpC1Handle,AcquireNIData::sampleChannel,ChannelCenter::getInstance()->_mode.sampleRate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,_samplesPerCallback));
	SetDataTransferMech(tmpC1Handle, SignalType::COUNTER_IN);

	CItaskHandles.push_back(tmpC1Handle);
	tmpC1Handle = NULL;

	return ret;
}

long AcquireNIData::SetupArmStartTrigger(TaskHandle taskhandle,const char* lineName)
{
	if(taskhandle)
	{
		DAQmxErrChk (L"DAQmxSetArmStartTrigType",DAQmxSetArmStartTrigType(taskhandle,DAQmx_Val_DigEdge));
		DAQmxErrChk (L"DAQmxSetDigEdgeArmStartTrigSrc",DAQmxSetDigEdgeArmStartTrigSrc(taskhandle,lineName));
		DAQmxErrChk (L"DAQmxSetDigEdgeArmStartTrigEdge",DAQmxSetDigEdgeArmStartTrigEdge(taskhandle,DAQmx_Val_Rising));
	}
	return TRUE;
}

long AcquireNIData::SetupPauseTrigger(TaskHandle taskhandle,bool pauseTrig)
{
	long ret = TRUE;
	int32 retVal = 0;
	int32 support = 0;

	if((taskhandle) && (true == pauseTrig) && (!_analogTrig))
	{
		//Both DI and CI tasks are not analog pausable, drop HW Synchronizable mode with aiTrigger.
		/*if(_analogTrig)
		{
		DAQmxErrChk (L"DAQmxSetPauseTrigType",DAQmxSetPauseTrigType(taskhandle,DAQmx_Val_AnlgLvl));
		retVal = DAQmxSetAnlgLvlPauseTrigSrc(taskhandle,AcquireNIData::triggerChannel.c_str());
		retVal = DAQmxSetAnlgLvlPauseTrigWhen(taskhandle,DAQmx_Val_BelowLvl);
		retVal = DAQmxSetAnlgLvlPauseTrigLvl(taskhandle,nimode->StimulusLimit);
		retVal = DAQmxGetPauseTrigType(taskhandle,&support);
		}*/

		DAQmxErrChk (L"DAQmxSetPauseTrigType",DAQmxSetPauseTrigType(taskhandle,DAQmx_Val_DigLvl));
		DAQmxErrChk (L"DAQmxSetDigLvlPauseTrigSrc",DAQmxSetDigLvlPauseTrigSrc(taskhandle,AcquireNIData::triggerChannel.c_str()));
		DAQmxErrChk (L"DAQmxSetDigLvlPauseTrigWhen",DAQmxSetDigLvlPauseTrigWhen(taskhandle,DAQmx_Val_Low));
	}
	if((10230 == support) || (-1 == support))
	{
		lastError = L"Task not analog pausable. ";
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireNIData Task not analog pausable. ");
		LogMessage(message, ERROR_EVENT);
		ret = FALSE;
	}
	return ret;
}

long AcquireNIData::SetupStartTrigger(TaskHandle taskhandle,bool retrig)
{
	int32 ret = 0;
	int32 support = 0;
	if(taskhandle)	//&& (!_pauseTrig)
	{
		if(_analogTrig)
		{
			DAQmxErrChk (L"DAQmxSetStartTrigType",ret = DAQmxSetStartTrigType(taskhandle, DAQmx_Val_AnlgEdge));
			DAQmxErrChk (L"DAQmxCfgAnlgEdgeStartTrig",ret = DAQmxCfgAnlgEdgeStartTrig(taskhandle,AcquireNIData::triggerChannel.c_str(),DAQmx_Val_Rising,ChannelCenter::getInstance()->_mode.StimulusLimit));	//DAQmx_Val_RisingSlope
			DAQmxErrChk (L"DAQmxGetStartTrigType",ret = DAQmxGetStartTrigType(taskhandle,&support));
		}
		else
		{
			DAQmxErrChk (L"DAQmxCfgDigEdgeStartTrig",ret = DAQmxCfgDigEdgeStartTrig(taskhandle,AcquireNIData::triggerChannel.c_str(),DAQmx_Val_Rising));	
		}
		if(retrig)
		{
			DAQmxErrChk (L"DAQmxSetStartTrigRetriggerable",ret = DAQmxSetStartTrigRetriggerable(taskhandle,retrig));
		}
	}
	if((10230 == support) ||(-1 == support))
	{
		lastError = L"Task not analog triggerable. ";
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireNIData Task not analog triggerable. ");
		LogMessage(message, ERROR_EVENT);
	}
	return TRUE;
}

long AcquireNIData::SetSamClkBehavior(TaskHandle taskhandle)
{
	DAQmxErrChk (L"DAQmxSetSampClkOverrunBehavior",DAQmxSetSampClkOverrunBehavior(taskhandle,DAQmx_Val_IgnoreOverruns));
	return TRUE;
}

long AcquireNIData::SetDataTransferMech(TaskHandle taskhandle, SignalType sType)
{
	if(NULL == taskhandle)
		return FALSE;

	int32 dataXferType = (BoardStyle::USB == ChannelCenter::getInstance()->_board.bStyle) ? DAQmx_Val_USBbulk : DAQmx_Val_DMA;

	switch (sType)
	{
	case ANALOG_IN:
		DAQmxErrChk (L"DAQmxSetAIDataXferMech", error = DAQmxSetAIDataXferMech(taskhandle,"",dataXferType));
		break;
	case DIGITAL_IN:
		DAQmxErrChk (L"DAQmxSetDIDataXferMech", error = DAQmxSetDIDataXferMech(taskhandle,"",dataXferType));
		break;
	case COUNTER_IN:
		DAQmxErrChk (L"DAQmxSetCIDataXferMech", error = DAQmxSetCIDataXferMech(taskhandle,"",dataXferType));
		break;
	case DIGITAL_OUT:
		DAQmxErrChk (L"DAQmxSetDODataXferMech", error = DAQmxSetDODataXferMech(taskhandle,"",dataXferType));
		break;
	default:
		return FALSE;
		break;
	}
	return TRUE;
}

///only continuous task need to set buffer size:
long AcquireNIData::SetupCfgBufferForContTasks()
{
	long ret = TRUE;
	//For some reason, configure buffers in contineous mode will fail somewhere at EventNCallback,
	//and it won't fail if not configuring buffer but running "1 MHz + 1 Hz" sampling rate.
	//if(DAQmx_Val_ContSamps == sampleMode)
	//{
	//	if(AcquireNIData::AItaskHandle)
	//	{
	//		DAQmxErrChk (L"DAQmxCfgInputBuffer",DAQmxCfgInputBuffer(AcquireNIData::AItaskHandle,(ChannelCenter::getInstance()->_board.totalAI+5)*_samplesPerCallback));	
	//	}
	//	if(AcquireNIData::DItaskHandle)
	//	{
	//		DAQmxErrChk (L"DAQmxCfgInputBuffer",DAQmxCfgInputBuffer(AcquireNIData::DItaskHandle,(ChannelCenter::getInstance()->_board.totalDI+5)*_samplesPerCallback));	
	//	}
	//	if(AcquireNIData::CItaskHandles.size()>2)
	//	{
	//		DAQmxErrChk (L"DAQmxCfgInputBuffer",DAQmxCfgInputBuffer(AcquireNIData::CItaskHandles[2],2*_samplesPerCallback));	
	//	}
	//}

	//Above is true for PrimaryAI, but necessary for PrimaryDI to avoid buffer overflow:
	if(0 == _primaryChanType.compare("PrimaryDI"))
	{
		if(AcquireNIData::AItaskHandle)
		{
			DAQmxErrChk (L"DAQmxCfgInputBuffer",error = DAQmxCfgInputBuffer(AcquireNIData::AItaskHandle,static_cast<uInt32>((ChannelCenter::getInstance()->_board.totalAI+1)*_samplesPerCallback)));	
		}
		if(AcquireNIData::DItaskHandle)
		{
			DAQmxErrChk (L"DAQmxCfgInputBuffer",error = DAQmxCfgInputBuffer(AcquireNIData::DItaskHandle,static_cast<uInt32>((ChannelCenter::getInstance()->_board.totalDI+1)*_samplesPerCallback)));	
		}
		if(DAQmx_Val_ContSamps == sampleMode)
		{
			for(int i=0; i<AcquireNIData::CItaskHandles.size(); i++)
			{
				DAQmxErrChk (L"DAQmxCfgInputBuffer",error = DAQmxCfgInputBuffer(AcquireNIData::CItaskHandles[i],static_cast<uInt32>(2*_samplesPerCallback)));
			}			
		}
	}

	//For continuous counters in finite mode:
	if(DAQmx_Val_FiniteSamps == sampleMode)
	{		
		DAQmxErrChk (L"DAQmxCfgInputBuffer",error = DAQmxCfgInputBuffer(AcquireNIData::CItaskHandles[0],static_cast<uInt32>(2*_samplesPerCallback)));	//	CItaskHandles[0] is Global Counter
		//DAQmxErrChk (L"DAQmxCfgInputBuffer",DAQmxCfgInputBuffer(AcquireNIData::CItaskHandles[1],2*_samplesPerCallback));	
		if(AcquireNIData::CItaskHandles.size()>1)
		{
			DAQmxErrChk (L"DAQmxCfgInputBuffer",error = DAQmxCfgInputBuffer(AcquireNIData::CItaskHandles[2],static_cast<uInt32>(2*_samplesPerCallback)));
		}
	}
	ret = (error != 0) ? FALSE : TRUE;

	return ret;
}

long AcquireNIData::Status()
{
	return TRUE;
}

long AcquireNIData::VerifyNIBoard(BoardInfo* board)
{
	long ret = TRUE;
	int32 errVal = 0;
	int buffersize;
	char* devicenames = NULL;
	char* nxdevicenames = NULL;
	char * pickDevicename = NULL; 
	std::string typeStr, nameStr;
	std::string searchDev = "";
	std::map<std::wstring,std::string> deviceTypeMap;

	if(0 == board->devID.size())
	{
		lastError = L"Empty Device ID. ";
		return FALSE;
	}
	//get device names:
	DAQmxErrChk (L"DAQmxGetSysDevNames",buffersize=DAQmxGetSystemInfoAttribute(DAQmx_Sys_DevNames,devicenames));
	devicenames=(char*)malloc(buffersize);
	DAQmxErrChk (L"DAQmxGetSysDevNames",errVal=DAQmxGetSystemInfoAttribute(DAQmx_Sys_DevNames,devicenames,buffersize));

	//check Dev number:	
	typeStr = devicenames;
	if(std::string::npos == typeStr.find(board->devID))
	{
		lastError = L"Invalid Device ID. ";
		free(devicenames);
		return FALSE;
	}
	//Get information about the device
	pickDevicename = strtok_s (devicenames,", ",&nxdevicenames);
	board->bStyle = BoardStyle::PCI;

	while(pickDevicename != NULL)
	{
		//check Product Type of user's device:
		nameStr = pickDevicename;
		typeStr = GetNIDeviceAttribute(nameStr,DAQmx_Dev_ProductType);

		if(std::string::npos != nameStr.find(board->devID.c_str()))
		{
			if(0 == strcmp(board->name.c_str(),"NI6363"))
			{
				if(std::string::npos == typeStr.find("PCIe-6363"))
				{
					searchDev = "PCIe-6363";
				}
			}
			else if(0 == strcmp(board->name.c_str(),"NI6363-USB"))			    
			{
				board->bStyle = BoardStyle::USB;
				if(std::string::npos == typeStr.find("USB-6363 (BNC)"))
				{
					searchDev = "USB-6363 (BNC)";
				}
			}
			else if(0 == strcmp(board->name.c_str(),"NI6361"))
			{
				if(std::string::npos == typeStr.find("PCIe-6361"))
				{
					searchDev = "PCIe-6361";
				}				
			}	
			else if(0 == strcmp(board->name.c_str(),"NI6321"))
			{
				if(std::string::npos == typeStr.find("PCIe-6321"))
				{
					searchDev = "PCIe-6321";
				}				
			}	
			else if(0 == strcmp(board->name.c_str(),"NI6341"))
			{
				board->bStyle = BoardStyle::USB;
				if(std::string::npos == typeStr.find("USB-6341 (BNC)"))
				{
					searchDev = "USB-6341 (BNC)";
				}				
			}	
		}		

		//generate device map for later check:
		std::wstring wnameStr(nameStr.begin(),nameStr.end());
		deviceTypeMap.insert(std::pair<std::wstring,std::string>(wnameStr,typeStr));

		pickDevicename = strtok_s(NULL,", ",&nxdevicenames);
	}

	//Prepare error message:
	if(searchDev.size()>0)
	{	
		bool found = false;
		std::wstring tempError = L"Invalid Board Type; valid candidates: ";
		for(std::map<std::wstring,std::string>::iterator it = deviceTypeMap.begin();it!=deviceTypeMap.end();++it)
		{
			if(0 == strcmp(searchDev.c_str(),it->second.c_str()))
			{
				lastError += it->first + L",";
				found = true;
			}
		}
		lastError = (found) ? (tempError) : (L"Invalid Board Type, please check settings.");
		free(devicenames);
		return FALSE;
	}

	if(devicenames)
	{
		free(devicenames);
		devicenames = NULL;
	}
	deviceTypeMap.clear();
	return ret;
}

long AcquireNIData::VerifyNILine(Channels* channel, int customType, std::string targetName)
{
	long ret = TRUE;
	int32 errVal = 0;
	std::string compStr1,compStr2;
	BoardInfo* board = &ChannelCenter::getInstance()->_board;

	if(NULL != channel)
	{
		//Verify users' settings:
		switch (channel->signalType)
		{
		case SignalType::ANALOG_IN:
			compStr1 = GetNIDeviceAttribute(board->devID,DAQmx_Dev_AI_PhysicalChans);
			compStr2 = board->devID + channel->lineId;
			break;
		case SignalType::DIGITAL_IN:
			compStr1 = GetNIDeviceAttribute(board->devID,DAQmx_Dev_DI_Lines);
			compStr2 = board->devID + channel->lineId;
			break;
		case SignalType::COUNTER_IN:
			compStr1 = GetNIDeviceAttribute(board->devID,DAQmx_Dev_Terminals);
			compStr2 = "/" + board->devID + channel->lineId;
			break;
		default:
			break;
		}
	}
	else
	{
		//Verify hard-coded settings:
		switch (customType)
		{
		case SignalType::COUNTER:
			compStr1 = GetNIDeviceAttribute(board->devID,DAQmx_Dev_CI_PhysicalChans);
			//counter line could be started with "/":
			compStr2 = (0 == targetName.find("/")) ? (targetName.substr(1,targetName.size()-1)) : (targetName);
			break;
		case SignalType::PFI:
			compStr1 = GetNIDeviceAttribute(board->devID,DAQmx_Dev_Terminals);
			compStr2 = targetName;
			break;
		default:
			break;
		}
	}

	//Determine whether line is available or not:
	//compStr1 = iLineName;
	if((compStr1.size() == 0) || (compStr1.find(compStr2) == std::string::npos))
	{
		lastError = std::wstring(compStr2.begin(),compStr2.end());
		lastError += L" is not available on current device. ";
		StringCbPrintfW(message,MSG_LENGTH,lastError.c_str());
		LogMessage(message,ERROR_EVENT);
		ret = FALSE;
	}
	return ret;
}
///********************************	END Run Synchronous Tasks	********************************///

///********************************	Run Asynchronous Tasks		********************************///
UINT bleachProc()
{
	AsyncParams* asyncSingle = &ChannelCenter::getInstance()->_asyncParam;

	int firstTriggerMode = asyncSingle->bleach.hwTrigMode;

	while(WaitForSingleObject(AcquireNIData::_hStopAsyncEvent,0) != WAIT_OBJECT_0)
	{
		for(int i=0;i<asyncSingle->bleach.cycle;i++)
		{
			if(WaitForSingleObject(AcquireNIData::_hStopAsyncEvent,0) == WAIT_OBJECT_0)
			{
				AcquireNIData::_isAsyncAcquiring = FALSE;
				SAFE_DELETE_HANDLE(AcquireNIData::_hProcessAsyncThread);
				return 0;
			}
			if(i == 0)
			{
				////re-state first trigger mode:
				asyncSingle->bleach.hwTrigMode = firstTriggerMode;
				//no delay at first:
				SetEvent(AcquireNIData::_hAsyncEventFinished);
			}				

			//execute bleach one cycle:
			if(WaitForSingleObject(AcquireNIData::_hAsyncEventFinished, INFINITE) == WAIT_OBJECT_0)
			{
				//wait until previous cycle finished:
				if(NULL != AcquireNIData::_hAsyncThread)
				{
					WaitForSingleObject(AcquireNIData::_hAsyncThread, INFINITE);
				}
				//run cycle other than the first:
				if(i != 0)
				{
					//HW-trigger the first cycle, and finish the rest by FREERUN:
					if((HWTriggerMode::HW_RETRIGGERABLE == asyncSingle->bleach.hwTrigMode) || 
						(HWTriggerMode::HW_TRIGGER_SINGLE == asyncSingle->bleach.hwTrigMode))
					{
						asyncSingle->bleach.hwTrigMode = HWTriggerMode::SW_FREERUN;
					}
					//wait interval time:
					Sleep(static_cast<DWORD>(asyncSingle->bleach.interval*SEC2MSEC));
					//return if stop requested:
					if(WaitForSingleObject(AcquireNIData::_hStopAsyncEvent,0) == WAIT_OBJECT_0)
					{
						AcquireNIData::_isAsyncAcquiring = FALSE;
						SAFE_DELETE_HANDLE(AcquireNIData::_hProcessAsyncThread);
						return 0;
					}
				}
				//create thread for bleaching:
				switch (asyncSingle->bleach.hwTrigMode)
				{
				case HWTriggerMode::HW_TRIGGER_SINGLE:						
				case HWTriggerMode::HW_RETRIGGERABLE:						
					if(WaitForSingleObject(AcquireNIData::_hHWTriggerAsyncEvent, INFINITE) == WAIT_OBJECT_0)
					{
						//return if stop requested:
						if(WaitForSingleObject(AcquireNIData::_hStopAsyncEvent,0) == WAIT_OBJECT_0)
						{
							AcquireNIData::_isAsyncAcquiring = FALSE;
							SAFE_DELETE_HANDLE(AcquireNIData::_hProcessAsyncThread);
							return 0;
						}
						AcquireNIData::createWriteDigitalOutputThread();
					}
					break;
				case HWTriggerMode::SW_FREERUN:
					AcquireNIData::createWriteDigitalOutputThread();
					break;
				default:
					break;
				}		
			}	
			if(i == asyncSingle->bleach.cycle-1)
			{
				if(HWTriggerMode::HW_RETRIGGERABLE == firstTriggerMode)
				{
					//Repeat cycles for HW-retriggerable mode,
					//restart next batch until user stop:
					i = -1;
					//AcquireNIData::ResetBleachHWTrigger();
					if(NULL != AcquireNIData::_hAsyncThread)
					{
						WaitForSingleObject(AcquireNIData::_hAsyncThread, INFINITE);
					}
				}
				else
				{
					//Signal stop:
					SetEvent(AcquireNIData::_hStopAsyncEvent);	
				}
			}
		}		
	}		

	SAFE_DELETE_HANDLE(AcquireNIData::_hProcessAsyncThread);
	return 0;
}

UINT AcquireNIData::createWriteDigitalOutputThread()
{
	DWORD dwThreadID;
	HANDLE hThread;	

	//wait until previous cycle finished:
	if(NULL != AcquireNIData::_hAsyncThread)
	{
		WaitForSingleObject(AcquireNIData::_hAsyncThread, INFINITE);
	}

	hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) AcquireNIData::writeDigitalOutputProc, NULL, NORMAL_PRIORITY_CLASS, &dwThreadID);
	SetThreadPriority(hThread,THREAD_PRIORITY_ABOVE_NORMAL);
	AcquireNIData::_hAsyncThread = hThread;

	return 0;
}

AsyncParams* AcquireNIData::AllocAsyncParams()
{
	//UINT32 bLine = 0;
	//UINT32 oLine = 0;
	int highDOLength = 0, lowDOLength = 0, iterateLength = 0;
	int outDelayLength = 0, outEarlyLength = 0;
	unsigned long long outFinalLengthMin = 1;
	AsyncParams* asyncSingle = &ChannelCenter::getInstance()->_asyncParam;

	if(asyncSingle->bleach.bleachTime == 0)
	{
		lastError = L"Bleach time cannot be 0. ";
		return NULL;
	}

	//DWORD ti = GetTickCount();

	//Prepare DO signal, determine if output line is buffered:
	asyncSingle->outBuffered = (asyncSingle->bleach.outputLine.find("/port0") != std::string::npos) ? true : false;

	highDOLength = (int)(MHZ * asyncSingle->bleach.bleachTime / SEC2MSEC);
	lowDOLength = (int)(MHZ * asyncSingle->bleach.bleachIdleTime / SEC2MSEC);
	iterateLength = highDOLength + lowDOLength;
	//bLine = static_cast<UINT32>(asyncSingle->bleach.bleachLine.at(asyncSingle->bleach.bleachLine.find_last_of('/')+5)-'0');
	if(asyncSingle->outBuffered)
	{
		//oLine = static_cast<UINT32>(asyncSingle->bleach.outputLine.at(asyncSingle->bleach.outputLine.find_last_of('/')+5)-'0');
		outDelayLength = (asyncSingle->bleach.outDelayTime > 0) ? (int)(MHZ * asyncSingle->bleach.outDelayTime / SEC2MSEC) : 0;
		outEarlyLength = (asyncSingle->bleach.outDelayTime < 0) ? (int)(MHZ * std::abs(asyncSingle->bleach.outDelayTime) / SEC2MSEC) : 0;
	}

	asyncSingle->callbackLength = (highDOLength * asyncSingle->bleach.bleachIteration) + (lowDOLength * (asyncSingle->bleach.bleachIteration));

	//[USB] require arrayLength to be even multiple of callbackLength, 
	//pad both line to match the length: 
	if((BoardStyle::USB == ChannelCenter::getInstance()->_board.bStyle) && (0 < asyncSingle->callbackLength))
	{
		int a2cCount = std::max(1, static_cast<int>(outDelayLength/asyncSingle->callbackLength));
		a2cCount = (a2cCount % 2) ? a2cCount : (a2cCount + 1);
		outFinalLengthMin = a2cCount * asyncSingle->callbackLength - outDelayLength;
	}

	asyncSingle->arrayLength = asyncSingle->callbackLength + outDelayLength + outFinalLengthMin;

	//NI limits size of data length:
	if(asyncSingle->arrayLength > INT_MAX)
	{
		lastError = L"Bleach array size exceeds limit. ";
		return NULL;
	}

	SAFE_DELETE_ARRAY(asyncSingle->arrayPtr);

	//Consecutive arrays for bleach + complete lines:
	asyncSingle->arrayPtr = (true == asyncSingle->outBuffered) ? new unsigned char[asyncSingle->arrayLength*2] : new unsigned char[asyncSingle->arrayLength];
	(true == asyncSingle->outBuffered) ? (memset(asyncSingle->arrayPtr, false, sizeof(unsigned char) * asyncSingle->arrayLength*2)) : (memset(asyncSingle->arrayPtr, false, sizeof(unsigned char) * asyncSingle->arrayLength));

	//generate bleach array (including iterations of bleach time and idle time):
	for(int it = 0;it<asyncSingle->bleach.bleachIteration;it++)
	{
		for(int i=0; i<highDOLength;i++)
		{
			asyncSingle->arrayPtr[i+it*iterateLength]=true;
		}
	}
	//generate complete array:	
	if(asyncSingle->outBuffered)
	{		
		if(outEarlyLength>0)
		{
			//early trigger complete line:
			for(unsigned long long i=(asyncSingle->arrayLength+asyncSingle->callbackLength-outEarlyLength);i<(asyncSingle->arrayLength*2);i++)
			{
				asyncSingle->arrayPtr[i]=true;
			}
		}
		else
		{
			//trigger complete line after delay:
			for(int i=(int)outFinalLengthMin;i>0;i--)
			{
				asyncSingle->arrayPtr[asyncSingle->arrayLength*2-i]=true;
			}
		}
	}

	//DWORD tf = GetTickCount() - ti;
	//StringCbPrintfW(message,MSG_LENGTH,L"Setup Time %d ms.",(int)tf);
	//MessageBox(NULL,message,L"Info", MB_OK | MB_SETFOREGROUND | MB_ICONINFORMATION);

	return asyncSingle;
}

long AcquireNIData::GetAsyncAcquiring()
{	
	return AcquireNIData::_isAsyncAcquiring;
}

long AcquireNIData::ResetBleachHWTrigger()
{
	long ret = TRUE;
	int32 retVal = 0;
	std::string asyncCILine = "";
	AsyncParams* asyncSingle = &ChannelCenter::getInstance()->_asyncParam;
	std::string hwTrigLine = (asyncSingle->bleach.hwTrigLine.size() > 0) ? ("/" + asyncSingle->board.devID + asyncSingle->bleach.hwTrigLine):("");

	//If invert enabled, Leave ctr3 for default indirect path (such as sample clock), 
	//share ctr2 with finite counter (frame-counter), and return if finite counting is running.
	//Tasks can be reserved before connect of terminals, however, it will try path through other counters (eg> ctr2).
	if(_invertEnabled)
	{
		if(CItaskHandles.size() > 2)
		{
			lastError = L"When invert is enabled, to enable HW trigger of bleach conflicts with Counter 01 running in finite mode. ";
			return FALSE;
		}
		asyncCILine = (asyncSingle->board.devID.size() > 0) ? ("/" + asyncSingle->board.devID + "/ctr2"):("");
	}
	else
	{
		asyncCILine = (asyncSingle->board.devID.size() > 0) ? ("/" + asyncSingle->board.devID + "/ctr3"):("");
	}

	if(hwTrigLine.size() <= 0)
	{
		lastError = L"Invalid HW trigger line. ";
		return FALSE;
	}
	else
	{
		ResetEvent(AcquireNIData::_hHWTriggerAsyncEvent);		
		StopTask(AcquireNIData::asyncHWTrigTask,TRUE);
		//InvertLines(false);
		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("",&AcquireNIData::asyncHWTrigTask));
		DAQmxErrChk(L"DAQmxCreateCICountEdgesChan",retVal = DAQmxCreateCICountEdgesChan (AcquireNIData::asyncHWTrigTask, asyncCILine.c_str(), "", DAQmx_Val_Rising, 0, DAQmx_Val_CountUp ));
		DAQmxErrChk (L"DAQmxSetCICountEdgesTerm",retVal = DAQmxSetCICountEdgesTerm(AcquireNIData::asyncHWTrigTask,asyncCILine.c_str(),hwTrigLine.c_str()));
		DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(AcquireNIData::asyncHWTrigTask,hwTrigLine.c_str(),1000,DAQmx_Val_Rising,DAQmx_Val_HWTimedSinglePoint, 0));		
		DAQmxErrChk(L"DAQmxRegisterSignalEvent",retVal = DAQmxRegisterSignalEvent(AcquireNIData::asyncHWTrigTask, DAQmx_Val_SampleClock , 0, AcquireNIData::HWTriggerAsyncCallback, NULL));
		//retVal = DAQmxTaskControl(AcquireNIData::asyncHWTrigTask,DAQmx_Val_Task_Reserve);
		retVal = DAQmxStartTask(AcquireNIData::asyncHWTrigTask);
		//InvertLines(true);
	}	

	if(0 != retVal)
	{
		lastError = L"Unable to reset bleach hw trigger task. ";
		return FALSE;	
	}
	return ret;
}

void AcquireNIData::ResetAsyncParams()
{
	SAFE_DELETE_ARRAY (ChannelCenter::getInstance()->_asyncParam.arrayPtr);
}

long AcquireNIData::StartAsync()
{
	long ret = TRUE;

	ResetAsyncParams();

	ChannelCenter::getInstance()->LoadXML();
	if(FALSE == VerifyNIBoard(&ChannelCenter::getInstance()->_board))
		return FALSE;

	AsyncParams* asyncSingle = &ChannelCenter::getInstance()->_asyncParam;
	_pmtShutter = (asyncSingle->bleach.shutterLine.size()>0) ? ("/" + asyncSingle->board.devID + asyncSingle->bleach.shutterLine) : ("");

	ResetEvent(AcquireNIData::_hStopAsyncEvent);
	ResetEvent(AcquireNIData::_hAsyncEventFinished);

	asyncSingle = AcquireNIData::AllocAsyncParams();
	if(NULL == asyncSingle)
	{
		return FALSE;
	}

	switch (asyncSingle->bleach.hwTrigMode)
	{
	case HWTriggerMode::SW_FREERUN:	
		break;
	case HWTriggerMode::HW_TRIGGER_SINGLE:
	case HWTriggerMode::HW_RETRIGGERABLE:
		ret = AcquireNIData::ResetBleachHWTrigger();
		break;
	default:
		lastError = L"Invalid trigger mode. "; 
		return FALSE;
	}
	if(ret)
	{
		if(NULL != AcquireNIData::_hProcessAsyncThread)
		{
			lastError = L"Unable to start bleach due to unfinished previous bleaching. "; 
			return FALSE;
		}
		AcquireNIData::_isAsyncAcquiring = TRUE;

		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) bleachProc, NULL, NORMAL_PRIORITY_CLASS, NULL);
		SetThreadPriority(hThread,THREAD_PRIORITY_NORMAL);
		AcquireNIData::_hProcessAsyncThread = hThread;
	}

	return ret;
}

long AcquireNIData::StopAsync()
{
	//Current Bleach mode involves non-buffered line which
	//requires important SW timing, 
	//so stop async thread is not allowed.
	SetEvent(AcquireNIData::_hStopAsyncEvent);
	SetEvent(AcquireNIData::_hHWTriggerAsyncEvent);
	AcquireNIData::StopTask(AcquireNIData::asyncHWTrigTask,TRUE);
	AcquireNIData::StopTask(AcquireNIData::DOtaskHandle,TRUE);
	//return until all threads are done:
	if(NULL != AcquireNIData::_hAsyncThread)
	{
		WaitForSingleObject(AcquireNIData::_hAsyncThread, INFINITE);
	}
	if(NULL != AcquireNIData::_hProcessAsyncThread)
	{
		WaitForSingleObject(AcquireNIData::_hProcessAsyncThread, THREAD_WAIT);
	}
	AcquireNIData::_isAsyncAcquiring = FALSE;
	return TRUE;
}

UINT AcquireNIData::writeDigitalOutputProc()
{	
	//Digital Output tasks' procedure:
	//(1)PMT_Shutter(low) -> (2)Sleep(closeT) -> (3)bleach -> (4)PMT_Sutter(high) -> (5)Sleep(openT) -> (6)Done(high)
	AsyncParams* asyncSingle = &ChannelCenter::getInstance()->_asyncParam;
	std::string done = (asyncSingle->bleach.outputLine.size()>0) ? ("/" + asyncSingle->board.devID + asyncSingle->bleach.outputLine) : ("");
	std::string bleachLine = (asyncSingle->bleach.bleachLine.size() > 0) ? ("/" + asyncSingle->board.devID + asyncSingle->bleach.bleachLine) : ("");
	std::string sampleLine = "/" + asyncSingle->board.devID + "/do/SampleClockTimebase";
	std::string dualLine = bleachLine;

	try
	{
		//Reset event to see if bleach callback is invoked,
		//e.g> cycle:5, interval:0, bleach:8ms, iteration:1; won't invoke callback everytime:
		ResetEvent(AcquireNIData::_hAsyncCallbackInvoked);

		//initialize signals:
		AcquireNIData::writeDigitalOutputTask(_pmtShutter,"",HighDO,0,NULL);	//0x1<<1

		//Start procedure:(1)
		AcquireNIData::writeDigitalOutputTask(_pmtShutter,"",0,0,NULL);
		//(2)
		Sleep(static_cast<DWORD>(asyncSingle->bleach.pmtCloseTime));
		//don't check for stop event until cycle finished, early return occurs at last cycle otherwise.
		//(3)
		if(asyncSingle->outBuffered)
		{
			dualLine = bleachLine + "," + done;
		}
		else
		{
			AcquireNIData::writeDigitalOutputTask(done,"",0,0,NULL);
		}
		AcquireNIData::writeDigitalOutputTask(dualLine,sampleLine,static_cast<UINT32>(asyncSingle->callbackLength),static_cast<int>(asyncSingle->arrayLength),asyncSingle->arrayPtr);
		if(!asyncSingle->outBuffered)
		{
			//(4)
			AcquireNIData::writeDigitalOutputTask(_pmtShutter,"",HighDO,0,NULL);	//0x1<<1
			//(5)
			Sleep(static_cast<DWORD>(asyncSingle->bleach.outDelayTime));
			//don't check for stop event until cycle is finished, early return occurs at last cycle otherwise.
			//(6)
			AcquireNIData::writeDigitalOutputTask(done,"",HighDO,0,NULL);		//0x1<<2
		}

		//Done Tasks:
		if(WaitForSingleObject(AcquireNIData::_hStopAsyncEvent,0) == WAIT_OBJECT_0)
		{	
			AcquireNIData::_isAsyncAcquiring = FALSE;
		}
		if(HWTriggerMode::HW_RETRIGGERABLE == asyncSingle->bleach.hwTrigMode)
		{
			AcquireNIData::ResetBleachHWTrigger();
		}
	}
	catch(RunTimeException ex)
	{
		AcquireNIData::_isAsyncAcquiring = FALSE;
		std::wstring str = ex.GetExceptionMessage();
		if(0 == str.compare(L"DAQmxTaskControl"))
		{
			switch (ex.GetErrorCode())
			{
			case DAQmxErrorBufferSizeNotMultipleOfEveryNSampsEventIntervalWhenDMA:	//(-200877): [USB] requires even number multiple of buffer size over eventN samples 
				StringCbPrintfW(message,MSG_LENGTH,L"Unable to start bleach due to non-standard settings.");	
				break;
			default:
				StringCbPrintfW(message,MSG_LENGTH,L"Unable to start bleach due to resources were reserved by other tasks.");
				break;
			}
			MessageBox(NULL,message,L"Info", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
		}
	}

	SAFE_DELETE_HANDLE(AcquireNIData::_hAsyncThread);

	//Bleach callback might not be invoked (means PMT shutter was not opened):
	if(WaitForSingleObject(AcquireNIData::_hAsyncCallbackInvoked,0) != WAIT_OBJECT_0)
	{
		AcquireNIData::pmtShutterTask(HighDO);
		SetEvent(AcquireNIData::_hAsyncEventFinished);
	}
	return 0;
}

long AcquireNIData::writeDigitalOutputTask(std::string lineName, std::string sampleLine, uInt32 val, int length, uInt8* array)
{
	long ret = TRUE;
	int32 written = 0;
	bool32 isTaskDone = 0;
	//bool32 sup = FALSE;
	if(lineName.size() <= 0)
	{
		lastError = L"Empty line is invalid. ";
		return FALSE;
	}
	StopTask(AcquireNIData::DOtaskHandle,TRUE);

	DAQmxErrChk (L"DAQmxCreateTask",error = DAQmxCreateTask("",&AcquireNIData::DOtaskHandle));
	DAQmxErrChk (L"DAQmxCreateDOChan",error = DAQmxCreateDOChan(AcquireNIData::DOtaskHandle,lineName.c_str(),"",DAQmx_Val_ChanPerLine));

	if(sampleLine.size()>0)
	{
		DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent",error = DAQmxRegisterEveryNSamplesEvent(AcquireNIData::DOtaskHandle,DAQmx_Val_Transferred_From_Buffer,val,0, AcquireNIData::EveryNBleachCallback,(void*)HighDO));	
		DAQmxErrChk (L"DAQmxCfgSampClkTiming",error = DAQmxCfgSampClkTiming(AcquireNIData::DOtaskHandle,sampleLine.c_str(),MHZ,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,length));
		//Drop DAQmxWriteDigitalU32: (no DAQmx_Val_ChanPerLine for multiple lines)
		SetDataTransferMech(AcquireNIData::DOtaskHandle, SignalType::DIGITAL_OUT);
		DAQmxErrChk (L"DAQmxCfgOutputBuffer",error = DAQmxCfgOutputBuffer(AcquireNIData::DOtaskHandle,static_cast<uInt32>(length)));
		DAQmxErrChk (L"DAQmxTaskControl",error = DAQmxTaskControl(AcquireNIData::DOtaskHandle,DAQmx_Val_Task_Reserve));
		DAQmxErrChk (L"DAQmxWriteDigitalLines",error = DAQmxWriteDigitalLines(AcquireNIData::DOtaskHandle,length,FALSE,0,DAQmx_Val_GroupByChannel,array,&written,NULL));
		DAQmxErrChk (L"DAQmxStartTask",error = DAQmxStartTask(AcquireNIData::DOtaskHandle));
		//error = DAQmxStartTask(AcquireNIData::DOtaskHandle);
		//error = DAQmxGetPhysicalChanDOSampClkSupported(lineName.c_str(),&sup);
	}
	else
	{
		DAQmxErrChk (L"DAQmxWriteDigitalLines",error = DAQmxWriteDigitalLines(AcquireNIData::DOtaskHandle,1,TRUE,0,DAQmx_Val_GroupByChannel,(uInt8*)(&val),&written,NULL));
	}
	//wait indefinitely until bleach is finished: 
	//DAQmxWaitUntilTaskDone will continue blocking even trying to stopTask at EveryNBleachCallback
	//DAQmxErrChk (L"DAQmxWaitUntilTaskDone",error = DAQmxWaitUntilTaskDone(AcquireNIData::DOtaskHandle,DAQmx_Val_WaitInfinitely));
	while (!isTaskDone)
	{
		if (NULL == AcquireNIData::DOtaskHandle)
			break;

		DAQmxIsTaskDone(AcquireNIData::DOtaskHandle, &isTaskDone);
	}
	StopTask(AcquireNIData::DOtaskHandle,TRUE);

	if(error != 0)
	{
		lastError = L"Unable to run digital output task. ";
		ret = FALSE;
	}
	return ret;
}

int32 CVICALLBACK AcquireNIData::EveryNBleachCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32 retVal = 0;

	//signal callback is invoked:
	SetEvent(AcquireNIData::_hAsyncCallbackInvoked);

	//When bleach time is longer than 8.2ms,
	//Shutter line will be closed ~5ms before end of bleach,
	//bleach with multi-iteration will also need to delay:
	switch (ChannelCenter::getInstance()->_board.bStyle)
	{
	case BoardStyle::USB:
		//[USB] will be already delayed for 5~8 ms, no need to sleep before open PMT shutter:
		AcquireNIData::pmtShutterTask((uInt8)callbackData);

		//outDelayTime is not part of callback length:
		if(0 < ChannelCenter::getInstance()->_asyncParam.bleach.outDelayTime)
		{
			Sleep((DWORD)ChannelCenter::getInstance()->_asyncParam.bleach.outDelayTime);
		}
		//stop task to cut padding:
		StopTask(AcquireNIData::DOtaskHandle,TRUE);
		break;
	case BoardStyle::PCI:
	default:
		if((ChannelCenter::getInstance()->_asyncParam.bleach.bleachTime >= 8.2) || (ChannelCenter::getInstance()->_asyncParam.bleach.bleachIteration > 1))
		{
			Sleep(7);
			//Let task finish without checking stop event, otherwise early return at the last cycle.
		}
		//open PMT shutter:
		AcquireNIData::pmtShutterTask((uInt8)callbackData);
		break;
	}

	//signal cycle done:
	SetEvent(AcquireNIData::_hAsyncEventFinished);
	return retVal;
}

///set value (1/0) to (open/close) PMT shutter line
void AcquireNIData::pmtShutterTask(uInt8 value)
{
	int32 written = 0;
	TaskHandle DOtaskHandle2 = NULL;

	if(_pmtShutter.size()>0)
	{		
		try
		{
			DAQmxErrChk (L"DAQmxCreateTask",error = DAQmxCreateTask("",&DOtaskHandle2));
			DAQmxErrChk (L"DAQmxCreateDOChan",error = DAQmxCreateDOChan(DOtaskHandle2,_pmtShutter.c_str(),"",DAQmx_Val_ChanPerLine));
			DAQmxErrChk (L"DAQmxWriteDigitalLines",error = DAQmxWriteDigitalLines(DOtaskHandle2,1,TRUE,0,DAQmx_Val_GroupByChannel,&value,&written,NULL));
			DAQmxErrChk (L"DAQmxWaitUntilTaskDone",error = DAQmxWaitUntilTaskDone(DOtaskHandle2,MAX_TASK_WAIT_TIME));
			StopTask(DOtaskHandle2,TRUE);
		}
		catch(RunTimeException ex)
		{
			lastError = L"Unable to close shutter line. ";
		}
	}
}

int32 CVICALLBACK AcquireNIData::HWTriggerAsyncCallback(TaskHandle taskHandle, int32 signalID, void *callbackData)
{
	SetEvent(AcquireNIData::_hHWTriggerAsyncEvent);
	StopTask(AcquireNIData::asyncHWTrigTask,TRUE);

	return 0;
}

///********************************	END Run Asynchronous Tasks	********************************///
