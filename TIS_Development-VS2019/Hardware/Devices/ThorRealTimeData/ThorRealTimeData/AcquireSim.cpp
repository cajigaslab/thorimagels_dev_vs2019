#include "stdafx.h"
#include "AcquireSim.h"
#include <cmath>
#include "strsafe.h"

std::wstring lastErrorSim;
std::unique_ptr<HDF5ioDLL> simH5io(new HDF5ioDLL(L".\\Modules_Native\\HDF5IO.dll"));
std::unique_ptr<HDF5ioDLL> simLoader(new HDF5ioDLL(L".\\Modules_Native\\HDF5IO.dll"));

///******	static members	******///
static CRITICAL_SECTION saveThreadAccess;
static CRITICAL_SECTION fileAccess;
static CRITICAL_SECTION displayUpdateAccess;
static CRITICAL_SECTION staticStopAccess;
StimulusSaveStruct* AcquireSim::simStimulusParams = NULL;
std::vector<std::string> AcquireSim::counterNames;
CompoundData* AcquireSim::_displayCompData = NULL;
double AcquireSim::_threadTime = MIN_THREAD_TIME;
long AcquireSim::sampleMode = 0;
unsigned long long AcquireSim::_samplesPerCallback = 0;
unsigned long long AcquireSim::_displayPerCallback = 0;
long AcquireSim::interleave = 0;
BOOL AcquireSim::_isAcquiring = FALSE;
BOOL AcquireSim::_isAsyncAcquiring = FALSE;
BOOL AcquireSim::_isSaving = FALSE;
UINT_PTR AcquireSim::_hTimer = NULL;
volatile size_t AcquireSim::_saveThreadCnt = 0;
volatile size_t AcquireSim::_saveThreadFinishedCnt = 0;
volatile size_t AcquireSim::_totalNumThreads = 0;
BOOL AcquireSim::_inSaveThread = FALSE;
SimDataStruct* AcquireSim::_simData = NULL;
std::list<HANDLE> AcquireSim::_hSaveThreads;
HANDLE AcquireSim::_hProcessSaveThread = NULL;
unsigned long AcquireSim::gCtrOverflowCnt = 0;
unsigned long AcquireSim::lastCountValue = 0;
std::wstring AcquireSim::ciLogFile = L"";
unsigned int AcquireSim::ciLogSuffix = 0;
unsigned long simCurrentFirstGCtr = 0; 
bool simInitializedGCtr = false;
bool simExtendBuf = false;
size_t simInitialGCtr = 0;
BOOL simSkipOverflowCheck = FALSE;
unsigned long AcquireSim::elapsedTimeUS = 0;

///******	End static members	******///

///********************************	Run Synchronous Tasks	********************************///
UINT SimSaveFileThreadProc(LPVOID pParam)
{
	EnterCriticalSection(&saveThreadAccess);	
	AcquireSim::_inSaveThread = TRUE;

	CompoundData* cdTemp = (CompoundData*) pParam;

	char* ptr = NULL;
	int idx_ai = 0, idx_di = 0, idx_ci = 0, idx_vi = 0;

	//check if global counter overflowed in contineous capture:
	if((!simSkipOverflowCheck) && (*(unsigned long*)(cdTemp->GetgCtr())<simCurrentFirstGCtr))
	{
		AcquireSim::gCtrOverflowCnt++;
	}
	simCurrentFirstGCtr = *(unsigned long*)(cdTemp->GetgCtr());
	simSkipOverflowCheck = FALSE;

	//save initial global counter for each batch of acquire sequences:
	if(!simInitializedGCtr)
	{	
		//Combine counter into uInt64, for both CI and global counter:
		simSkipOverflowCheck = cdTemp->SetupGlobalCounter(AcquireSim::gCtrOverflowCnt);
		simInitialGCtr = cdTemp->GetStrucData()->gCtr64Ptr[0];	
		simInitializedGCtr = true;
	}
	else
	{
		//Combine counter into uInt64 with offset, for both CI and global counter:
		simSkipOverflowCheck = cdTemp->SetupGlobalCounter(simInitialGCtr, 0, AcquireSim::gCtrOverflowCnt);
	}

	//compute for virtual channels:
	VirtualTimeChannelManager::getInstance()->Execute(cdTemp);

	//feed for spectral channels before user request stop:
	if(AcquireSim::_isAcquiring)
		SpectralManager::getInstance()->WriteSource(cdTemp);

	//try to update display buffer:
	if(TryEnterCriticalSection(&displayUpdateAccess))
	{
		CompoundData* _dDataTemp = new CompoundData(cdTemp,AcquireSim::interleave);
		AcquireSim::_displayCompData->CopyCompoundData(_dDataTemp);
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireSim display first counter %u",*(unsigned long*)_dDataTemp->GetStrucData()->gCtr64Ptr);
		LogMessage(message,VERBOSE_EVENT);
		delete _dDataTemp;
		LeaveCriticalSection(&displayUpdateAccess);
	}

	//get hdf5 path and filename (which user has defined):
	std::wstring tmpStr = ChannelCenter::getInstance()-> GetEpisodeName();
	
	//get frame timing log in same folder
	if (AcquireSim::ciLogSuffix == 0)
		AcquireSim::ciLogFile = tmpStr.substr(0, tmpStr.find_last_of('\\') + 1) + L"FrameTiming.txt";
	else
		AcquireSim::ciLogFile = tmpStr.substr(0, tmpStr.find_last_of('\\') + 1) + L"FrameTiming_" + to_wstring(AcquireSim::ciLogSuffix) + L".txt";

	// Judge whether excceeds the file size limitation
	ifstream ifs(AcquireSim::ciLogFile, ios::in);
	if (ifs.is_open())
	{
		ifs.seekg(0, ios::end);
		if (ifs.tellg() >= 10 * 1024 * 1024)
		{
			AcquireSim::ciLogSuffix++;
			AcquireSim::ciLogFile = tmpStr.substr(0, tmpStr.find_last_of('\\') + 1) + L"FrameTiming_" + to_wstring(AcquireSim::ciLogSuffix) + L".txt";
		}
		ifs.close();
	}
	ofstream cistream;

	//try write to file if user wants to:
	if((TRUE == cdTemp->GetSaving()) && (tmpStr.size()>0))
	{
		CompoundData* cdDataForSave = new CompoundData(cdTemp,AcquireSim::simStimulusParams);

		EnterCriticalSection(&fileAccess);	

		if((TRUE == simH5io->OpenFileIO(tmpStr.c_str(),H5FileType::READWRITE)) && (cdDataForSave->GetgcLengthComValue() > 0))
		{
			for(int i=0;i<ChannelCenter::getInstance()->_dataChannel.size();i++)
			{
				if(0 == ChannelCenter::getInstance()->_dataChannel.at(i).type.compare("/AI"))
				{			
					if(cdDataForSave->GetaiLengthValue() > 0)
					{
						if(FALSE == simH5io->ExtendData(ChannelCenter::getInstance()->_dataChannel.at(i).type.c_str(),("/" + ChannelCenter::getInstance()->_dataChannel.at(i).alias).c_str(),cdDataForSave->GetStrucData()->aiDataPtr+idx_ai*cdDataForSave->GetgcLengthComValue(),H5DataType::DATA_DOUBLE,simExtendBuf,static_cast<unsigned long>(cdDataForSave->GetgcLengthComValue())))
						{
							StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file analog channel at thread (%d)",AcquireSim::_saveThreadCnt);
							LogMessage(message,ERROR_EVENT);
						}
						idx_ai++;
					}
					else
					{
						StringCbPrintfW(message,MSG_LENGTH,L"Error reading analog circular buffer at thread (%d)",AcquireSim::_saveThreadCnt);
						LogMessage(message,ERROR_EVENT);
					}
				}
				if(0 == ChannelCenter::getInstance()->_dataChannel.at(i).type.compare("/DI"))
				{			
					if(cdDataForSave->GetdiLengthValue() > 0)
					{
						if(FALSE == simH5io->ExtendData(ChannelCenter::getInstance()->_dataChannel.at(i).type.c_str(),("/" + ChannelCenter::getInstance()->_dataChannel.at(i).alias).c_str(),cdDataForSave->GetStrucData()->diDataPtr+idx_di*cdDataForSave->GetgcLengthComValue(),H5DataType::DATA_UCHAR,simExtendBuf,static_cast<unsigned long>(cdDataForSave->GetgcLengthComValue())))
						{
							StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file digital channel at thread (%d)",AcquireSim::_saveThreadCnt);
							LogMessage(message,ERROR_EVENT);
						}
						idx_di++;
					}
					else
					{
						StringCbPrintfW(message,MSG_LENGTH,L"Error reading digital circular buffer at thread (%d)",AcquireSim::_saveThreadCnt);
						LogMessage(message,ERROR_EVENT);
						//ret = FALSE;
					}
				}
				if(0 == ChannelCenter::getInstance()->_dataChannel.at(i).type.compare("/CI"))
				{
					if(cdDataForSave->GetciLengthValue() > 0)
					{
						if (ChannelCenter::getInstance()->_dataChannel.at(i).saveTiming == 1) {
							size_t nNumber = cdDataForSave->GetgcLengthComValue();
							unsigned long* startCounter = cdDataForSave->GetStrucData()->ciDataPtr + idx_ci * cdDataForSave->GetgcLengthComValue();
							unsigned long* endCounter = startCounter + nNumber - 1;

							if (*endCounter > AcquireSim::lastCountValue)
							{
								cistream.open(AcquireSim::ciLogFile, ofstream::app);

								// calculate for each time count increasing 1
								for (int i = 0; i < nNumber; i++)
								{
									if (*startCounter > AcquireSim::lastCountValue) {
										string str = AcquireSim::CalcTimeString((unsigned int)(i * 1000000 / ChannelCenter::getInstance()->_mode.sampleRate)); // add the time by i/freq (unit us)
										AcquireSim::lastCountValue = *startCounter;
										// write to file
										cistream << str << " " << *startCounter << endl;
									}
									startCounter++;
								}
								AcquireSim::elapsedTimeUS += (unsigned long) (nNumber * 1000000 / ChannelCenter::getInstance()->_mode.sampleRate);
								cistream.close();
							}
						}

						if(FALSE == simH5io->ExtendData(ChannelCenter::getInstance()->_dataChannel.at(i).type.c_str(),("/" + ChannelCenter::getInstance()->_dataChannel.at(i).alias).c_str(), cdDataForSave->GetStrucData()->ciDataPtr + idx_ci * cdDataForSave->GetgcLengthComValue(),H5DataType::DATA_UINT32,simExtendBuf,static_cast<unsigned long>(cdDataForSave->GetgcLengthComValue())))
						{
							StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file counter channel at thread (%d)",AcquireSim::_saveThreadCnt);
							LogMessage(message,ERROR_EVENT);
						}
						idx_ci++;
					}
					else
					{
						StringCbPrintfW(message,MSG_LENGTH,L"Error reading counter circular buffer at thread (%d)",AcquireSim::_saveThreadCnt);
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
						if(FALSE == simH5io->ExtendData(ChannelCenter::getInstance()->_virChannel.at(i).type.c_str(),("/" + ChannelCenter::getInstance()->_virChannel.at(i).alias).c_str(),cdDataForSave->GetStrucData()->viDataPtr+idx_vi*cdDataForSave->GetgcLengthComValue(),H5DataType::DATA_DOUBLE,simExtendBuf,static_cast<unsigned long>(cdDataForSave->GetgcLengthComValue())))
						{
							StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file virtual channel at thread (%d)",AcquireSim::_saveThreadCnt);
							LogMessage(message,ERROR_EVENT);
						}
						idx_vi++;
					}
					else
					{
						StringCbPrintfW(message,MSG_LENGTH,L"Error reading analog circular buffer at thread (%d)",AcquireSim::_saveThreadCnt);
						LogMessage(message,ERROR_EVENT);
					}
				}
			}
			//Global counter:
			if(cdDataForSave->GetgcLengthComValue() > 0)
			{
				if(FALSE == simH5io->ExtendData("/Global",AcquireSim::counterNames.at(0).c_str(),cdDataForSave->GetStrucData()->gCtr64Ptr, H5DataType::DATA_UINT64,simExtendBuf,static_cast<unsigned long>(cdDataForSave->GetgcLengthComValue())))
				{
					StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file global counter at thread (%d)",AcquireSim::_saveThreadCnt);
					LogMessage(message,ERROR_EVENT);
				}
			}
			else
			{
				StringCbPrintfW(message,MSG_LENGTH,L"Error reading counter circular buffer at thread (%d)",AcquireSim::_saveThreadCnt);
				LogMessage(message,ERROR_EVENT);
			}
		}
		delete cdDataForSave;
		simH5io->CloseFileIO();
		LeaveCriticalSection(&fileAccess);
	}
	//Done saving:
	if(!simExtendBuf)
	{
		simExtendBuf = true;
	}
	delete cdTemp;	
	CloseHandle(AcquireSim::_hSaveThreads.front());
	AcquireSim::_hSaveThreads.erase(AcquireSim::_hSaveThreads.begin());
	AcquireSim::_saveThreadFinishedCnt++;

	StringCbPrintfW(message,MSG_LENGTH,L"AcquireSim save file thread done (%d),save Thread Count (%d)",AcquireSim::_saveThreadFinishedCnt,AcquireSim::_saveThreadCnt);
	LogMessage(message,INFORMATION_EVENT);

	AcquireSim::_inSaveThread = FALSE;	
	LeaveCriticalSection(&saveThreadAccess);
	return 0;
}

void CALLBACK AcquireSim::TimerCallback(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime)
{
	DWORD dwThread;
	HANDLE hThread;	
	time_t eventTime = 0;

	//Finite Mode:
	if((SampleMode::SimFinite == sampleMode) && (AcquireSim::_saveThreadCnt >= _totalNumThreads))
	{
		StaticStop();
		return;
	}
	CompoundData* _callbackCompData = new CompoundData(_samplesPerCallback,_simData->nAI*_samplesPerCallback, _simData->nDI*_samplesPerCallback, _samplesPerCallback,_simData->nVI*_samplesPerCallback);
	double tmpRatio = CLK_RATE_20MHZ/ChannelCenter::getInstance()->_mode.sampleRate;
	double freq = ChannelCenter::getInstance()->_globalVar[0].value;

	if (0 == ChannelCenter::getInstance()->_board.devID.compare("FILE"))
	{	
		EnterCriticalSection(&fileAccess);

		//load from episode
		if (TRUE == simLoader.get()->OpenFileIO(ChannelCenter::getInstance()->_filePath.episodePath.c_str(), H5FileType::READWRITE))
		{
			unsigned long long size = 0;
			simLoader.get()->CheckGroupDataset("/Global", "/GCtr", size);

			//prepare gCtr time info
			for(unsigned long long i = 0; i < AcquireSim::_samplesPerCallback; i++)
			{
				_simData->gCtr[i]=static_cast<unsigned long>(i*tmpRatio) + static_cast<unsigned long>(AcquireSim::_saveThreadCnt*static_cast<double>(CLK_RATE_20MHZ*_threadTime));
			}
			memcpy(_callbackCompData->GetgCtr(),_simData->gCtr,sizeof(unsigned long)*_samplesPerCallback);

			//load data from episode
			unsigned long long currentRead = 0;
			while (currentRead < AcquireSim::_samplesPerCallback)
			{
				unsigned long long currentSize = (AcquireSim::_saveThreadCnt * AcquireSim::_samplesPerCallback) + currentRead;
				unsigned long long currentInFile = currentSize % size;
				unsigned long long remainInFile = size - (currentSize % size);
				unsigned long long sizeToRead = min(remainInFile, (AcquireSim::_samplesPerCallback - currentRead));

				if(FALSE == ChannelCenter::getInstance()->LoadEpisodeDataOnly(simLoader.get(), _callbackCompData, currentRead, _samplesPerCallback, currentInFile, sizeToRead))
				{
					simLoader.get()->CloseFileIO();
					StaticStop();
					StringCbPrintfW(message,MSG_LENGTH,L"Simulator unable to load HDF5 File (%ls)", ChannelCenter::getInstance()->_filePath.episodePath.c_str());
					LogMessage(message, ERROR_EVENT);
					lastErrorSim = message;
					return;
				}
				currentRead += sizeToRead;
			}
		}
		simLoader.get()->CloseFileIO();
		LeaveCriticalSection(&fileAccess);
	}
	else
	{	
		//generate data:
		for(unsigned long long i=0;i<_samplesPerCallback;i++)
		{
			//gCtr:
			_simData->gCtr[i]=static_cast<unsigned long>(i*tmpRatio) + static_cast<unsigned long>(AcquireSim::_saveThreadCnt*static_cast<double>(CLK_RATE_20MHZ*_threadTime));
			//aiData: freq = T_FREQ
			if(_simData->nAI>0)
			{
				for(int j=0;j<_simData->nAI;j++)
				{
					_simData->aiData[i+j*_samplesPerCallback] = AMPLITUDE*std::sin(static_cast<double>(2*PI*((j+1)*freq)*_simData->gCtr[i]/CLK_RATE_20MHZ + PI/4*j));
				}
			}
			//diData: period = (nDI, threadCnt dep.)
			if(_simData->nDI>0)
			{
				for(int j=0;j<_simData->nDI;j++)
				{
					_simData->diData[i+j*_samplesPerCallback] = ((AcquireSim::_saveThreadCnt % (j+2)) == 0) ? static_cast<unsigned char>(1) : 0;
				}
			}
			//ciData: counting threadCnt
			if(_simData->nCI>0)
			{
				for(int j=0;j<_simData->nCI;j++)
				{
					_simData->ciData[i+j*_samplesPerCallback] = static_cast<unsigned long>(j) + static_cast<unsigned long>(AcquireSim::_saveThreadCnt);
				}
			}
		}

		//copy data:
		memcpy(_callbackCompData->GetgCtr(),_simData->gCtr,sizeof(unsigned long)*_samplesPerCallback);
		if(_simData->nAI>0) {memcpy(_callbackCompData->GetStrucData()->aiDataPtr,_simData->aiData,sizeof(double)*_simData->nAI*_samplesPerCallback);}
		if(_simData->nDI>0) {memcpy(_callbackCompData->GetStrucData()->diDataPtr,_simData->diData,sizeof(unsigned char)*_simData->nDI*_samplesPerCallback);}
		if(_simData->nCI>0) {memcpy(_callbackCompData->GetStrucData()->ciDataPtr,_simData->ciData,sizeof(unsigned long)*_simData->nCI*_samplesPerCallback);}
	}

	//create save thread:
	_callbackCompData->SetSaving(AcquireSim::_isSaving);
	std::time(&eventTime);
	_callbackCompData->SetCreateTime(eventTime);
	hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) SimSaveFileThreadProc, (LPVOID)_callbackCompData, CREATE_SUSPENDED, &dwThread);
	SetThreadPriority(hThread,THREAD_PRIORITY_NORMAL);
	AcquireSim::_hSaveThreads.push_back(hThread);
	AcquireSim::_saveThreadCnt++;
}

UINT SimProcessSaveThreadProc()
{
	//We create Thread in suspension, finish in order of creation:
	while((TRUE == AcquireSim::_isAcquiring) || (0 < AcquireSim::_hSaveThreads.size()))
	{
		//try maintain at least 3 threads in continuous mode, 
		//so that no dereference of list's end:
		while((SimContinuous == AcquireSim::sampleMode) && (TRUE == AcquireSim::_isAcquiring) && (4 > AcquireSim::_hSaveThreads.size()))
		{	
			Sleep(1);			
		}

		if((FALSE == AcquireSim::_inSaveThread)&& (0 < AcquireSim::_hSaveThreads.size()))
		{
			if(TryEnterCriticalSection(&saveThreadAccess))
			{
				ResumeThread(AcquireSim::_hSaveThreads.front());	
				LeaveCriticalSection(&saveThreadAccess);
			}			
		}			
	}

	//Done processing save threads:
	AcquireSim::_saveThreadCnt = 0;
	AcquireSim::_saveThreadFinishedCnt = 0;

	//h5io->CloseFileIO();

	CloseHandle(AcquireSim::_hProcessSaveThread);
	AcquireSim::_hProcessSaveThread = NULL;

	return 0;
}

AcquireSim::AcquireSim()
{
	simStimulusParams = new StimulusSaveStruct();
}

AcquireSim::~AcquireSim()
{	
	SAFE_DELETE_ARRAY(simStimulusParams);

	if(_hTimer)
	{
		KillTimer(NULL,_hTimer);
		_hTimer = NULL;
	}
	if(_simData)
	{
		if(_simData->gCtr)
		{
			delete[] _simData->gCtr;
			_simData->gCtr = NULL;
		}
		if(_simData->aiData)
		{
			delete[] _simData->aiData;
			_simData->aiData = NULL;
		}
		if(_simData->diData)
		{
			delete[] _simData->diData;
			_simData->diData = NULL;
		}
		if(_simData->ciData)
		{
			delete[] _simData->ciData;
			_simData->ciData = NULL;
		}
		if(_simData->viData)
		{
			delete[] _simData->viData;
			_simData->viData = NULL;
		}
		delete _simData;
		_simData = NULL;
	}
	if(AcquireSim::counterNames.size()>0)
	{
		AcquireSim::counterNames.clear();
	}
	if(AcquireSim::_ailineName.size()>0)
	{
		AcquireSim::_ailineName.clear();
	}
	if(AcquireSim::_dilineName.size()>0)
	{
		AcquireSim::_dilineName.clear();
	}
	if(_displayCompData)
	{
		delete _displayCompData;
		_displayCompData = NULL;
	}
	if(AcquireSim::_hSaveThreads.size()>0)
	{
		std::list<HANDLE>::iterator it = AcquireSim::_hSaveThreads.begin();
		for(long i=0; i<AcquireSim::_hSaveThreads.size(); i++)
		{
			CloseHandle(*it);
			//*it = NULL;
			it++;
		}
		AcquireSim::_hSaveThreads.clear();
	}
}

void AcquireSim::DeleteCriticalSections()
{
	DeleteCriticalSection(&saveThreadAccess);
	DeleteCriticalSection(&fileAccess);
	DeleteCriticalSection(&displayUpdateAccess);
	DeleteCriticalSection(&staticStopAccess);
}

void AcquireSim::InitializeCriticalSections()
{
	InitializeCriticalSection(&saveThreadAccess);
	InitializeCriticalSection(&fileAccess);	
	InitializeCriticalSection(&displayUpdateAccess);
	InitializeCriticalSection(&staticStopAccess);
}

long AcquireSim::LoadXML()
{
	long ret = TRUE;
	ResetParams();
	ResetTimingParams();
	if(FALSE == ChannelCenter::getInstance()->LoadXML())
		return FALSE;

	AcquireSim::interleave = ChannelCenter::getInstance()->_mode.interleave;
	_samplesPerCallback = static_cast<unsigned long long>(ChannelCenter::getInstance()->_mode.sampleRate*_threadTime);
	//determine sample mode contineous or finite:
	if(ChannelCenter::getInstance()->_mode.duration > 0)
	{
		sampleMode = SampleMode::SimFinite;
		_totalNumThreads = static_cast<size_t>(ceil(ChannelCenter::getInstance()->_mode.duration/_threadTime));
	}
	else
	{
		sampleMode = SampleMode::SimContinuous;
		_totalNumThreads = 0;
	}

	//(0 == _samplesPerCallback % _interleave) is required:
	interleave = ChannelCenter::getInstance()->_mode.interleave;

	_displayPerCallback = (interleave > 0) ? static_cast<unsigned long long>(_samplesPerCallback/interleave) : static_cast<unsigned long long>(_samplesPerCallback);	
	_displayCompData = new CompoundData(_displayPerCallback,(ChannelCenter::getInstance()->_board.totalAI)*_displayPerCallback,(ChannelCenter::getInstance()->_board.totalDI)*_displayPerCallback, _displayPerCallback, ChannelCenter::getInstance()->_virChannel.size()*_displayPerCallback);
	_displayCompData->SetSaving(FALSE);

	return ret;
}

long AcquireSim::Enter()
{
	InitializeCriticalSections();
	return TRUE;
}

long AcquireSim::Exit()
{
	DeleteCriticalSections();
	return TRUE;
}

long AcquireSim::SetupChannels()
{
	long ret = TRUE;
	int nAI=0, nDI=0, nCI=0;
	//default global counter name:
	AcquireSim::counterNames.push_back("/GCtr");

	if(0 == ChannelCenter::getInstance()->_dataChannel.size())
	{
		lastErrorSim = L"No channels being specified. ";
		return FALSE;
	}

	std::string devName = ChannelCenter::getInstance()->_board.devID;
	std::vector<Channels> dataChannel = ChannelCenter::getInstance()->_dataChannel;

	for(int i=0;i<dataChannel.size();i++)
	{
		long channelType = 0;
		if(0 == dataChannel.at(i).type.compare("/AI"))
		{	
			_ailineName.push_back(devName + dataChannel.at(i).lineId);
			if(dataChannel.at(i).Stimulus > 0)
			{
				AcquireSim::simStimulusParams->enable=true;
				AcquireSim::simStimulusParams->signalType=0;
				AcquireSim::simStimulusParams->stimChannelID = nAI;
				AcquireSim::simStimulusParams->threshold = ChannelCenter::getInstance()->_mode.StimulusLimit;
				AcquireSim::simStimulusParams->lineName = "/" + devName + ChannelCenter::getInstance()->_dataChannel.at(i).lineId;
			}
			nAI++;
		}
		if(0 == dataChannel.at(i).type.compare("/DI"))
		{	
			_dilineName.push_back(devName + dataChannel.at(i).lineId);
			if(dataChannel.at(i).Stimulus > 0)
			{
				AcquireSim::simStimulusParams->enable=true;
				AcquireSim::simStimulusParams->signalType=1;
				AcquireSim::simStimulusParams->stimChannelID = nDI;
				AcquireSim::simStimulusParams->threshold = 0;
				AcquireSim::simStimulusParams->lineName = "/" + devName + ChannelCenter::getInstance()->_dataChannel.at(i).lineId;
			}
			nDI++;
		}
		if(0 == dataChannel.at(i).type.compare("/DO"))
		{	
		}
		if(0 == dataChannel.at(i).type.compare("/CI"))
		{	
			AcquireSim::counterNames.push_back(dataChannel.at(i).lineId);
			nCI++;
		}
		if(0 == dataChannel.at(i).type.compare("/AO"))
		{	
		}
	}

	int nVI = static_cast<int>(ChannelCenter::getInstance()->_virChannel.size());
	VirtualTimeChannelManager::getInstance()->UpdateProcessor();

	//spectral & spectral virtual channels:
	SpectralManager::getInstance()->SetSaveFile(FALSE);
	SpectralManager::getInstance()->UpdateSpectralAnalyzer();

	//prepare callback data:
	_simData = new SimDataStruct();
	_simData->gCtr = new unsigned long[_samplesPerCallback];
	_simData->nAI=nAI;
	_simData->aiData = (_simData->nAI > 0) ? (new double[_simData->nAI*_samplesPerCallback]) : NULL;
	_simData->nDI=nDI;
	_simData->diData = (_simData->nDI > 0) ? (new unsigned char[_simData->nDI*_samplesPerCallback]) : NULL;
	_simData->nCI=nCI;
	_simData->ciData = (_simData->nCI > 0) ? (new unsigned long[_simData->nCI*_samplesPerCallback]) : NULL;
	_simData->nVI=nVI;
	_simData->viData = (_simData->nVI > 0) ? (new double[_simData->nVI*_samplesPerCallback]) : NULL;

	return ret;

}

long AcquireSim::SetupFileIO()
{
	if(AcquireSim::_isSaving)
	{
		if(FALSE == ChannelCenter::getInstance()->SetupFileH5(AcquireSim::_samplesPerCallback))
		{
			lastErrorSim = ChannelCenter::getInstance()->GetLastError();
			StringCbPrintfW(message,MSG_LENGTH,L"Simulator unable to create HDF5 File");
			LogMessage(message, WARNING_EVENT);
			return FALSE;
		}
	}

	//verify file to be load
	if (0==ChannelCenter::getInstance()->_board.devID.compare("FILE"))
	{
		std::vector<std::string> groupNameVec;
		std::map<std::string, std::vector<std::string>> datasetMap;
		if (FALSE == ChannelCenter::getInstance()->LoadEpisodeGroupDatasetNames(ChannelCenter::getInstance()->_filePath.episodePath.c_str(), groupNameVec, datasetMap))
		{
			lastErrorSim = ChannelCenter::getInstance()->GetLastError();
			return FALSE;
		}
		if(FALSE == ChannelCenter::getInstance()->VerifyFileChannels(datasetMap))
		{
			lastErrorSim = ChannelCenter::getInstance()->GetLastError();
			return FALSE;
		}
	}

	return TRUE;
}

long AcquireSim::Start()
{
	long ret = TRUE;
	DWORD dwThread;
	HANDLE hThread;	

	_hTimer = SetTimer(NULL,0,static_cast<UINT>(_threadTime*SEC2MSEC),(TIMERPROC)TimerCallback);
	AcquireSim::_isAcquiring = TRUE;	

	//wait for save thread to be created:
	Sleep((DWORD)(2*_threadTime*SEC2MSEC));	

	hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) SimProcessSaveThreadProc, NULL, NORMAL_PRIORITY_CLASS, &dwThread);	
	SetThreadPriority(hThread,THREAD_PRIORITY_NORMAL);
	AcquireSim::_hProcessSaveThread = hThread;

	return ret;
}

long AcquireSim::CopyStructData(void* ptr)
{
	long ret = TRUE;
	try
	{
		EnterCriticalSection(&displayUpdateAccess);
		_displayCompData->GetStrucData((CompDataStruct*)ptr);

	}
	catch(...)
	{
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireSim unable to get display data");
		LogMessage(message,WARNING_EVENT);
		ret = FALSE;
	}

	LeaveCriticalSection(&displayUpdateAccess);
	return ret;
}

///Dropped Pause and Restart:
long AcquireSim::Pause()
{
	long ret = TRUE;
	return ret;
}
long AcquireSim::Restart()
{
	long ret = TRUE;
	return ret;
}

long AcquireSim::StaticStop()
{
	EnterCriticalSection(&staticStopAccess);

	if(_hTimer)
	{
		KillTimer(NULL,_hTimer);
		_hTimer = NULL;
	}
	AcquireSim::_isAcquiring = FALSE;

	if((NULL != AcquireSim::_hProcessSaveThread) && (WaitForSingleObject(AcquireSim::_hProcessSaveThread, INFINITE) == WAIT_OBJECT_0))
	{
		simH5io->DestroyFileIO();
	}

	LeaveCriticalSection(&staticStopAccess);
	return TRUE;
}

long AcquireSim::Stop()
{
	return StaticStop();
}

long AcquireSim::GetAcquiring()
{
	return AcquireSim::_isAcquiring;
}

long AcquireSim::GetSaving()
{
	return AcquireSim::_isSaving;
}

long AcquireSim::SetSaving(long toSave)
{
	AcquireSim::_isSaving = toSave;
	return TRUE;
}

long AcquireSim::InitCallbacks(SpectralUpdateCallback su, DataUpdateCallback du)
{
	functionPointer = su;
	dataPointer = du;
	if(functionPointer != NULL)
	{
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireSim InitCallbacks");
		LogMessage(message,VERBOSE_EVENT);
	}
	return TRUE;
}

std::wstring AcquireSim::GetLastError()
{			
	return lastErrorSim;
}

long AcquireSim::Status()
{
	long ret = TRUE;
	return ret;
}


// Calculate baseTimeMS+deltaTimeMS and return a time format string
std::string AcquireSim::CalcTimeString(unsigned int deltaTimeUS)
{
	unsigned long timeUS = AcquireSim::elapsedTimeUS + deltaTimeUS;
	long long cvrt = 60 * 1000000;//64 bit integer not large enough TODO check for warning here
	cvrt *= 60;
	int hour = timeUS / cvrt;
	int minute = timeUS % cvrt / (60 * 1000000);
	int second = timeUS % (60 * 1000000) / 1000000;
	int mSecond = timeUS % 1000000 / 1000;
	int uSecond = timeUS % 1000000 % 1000;

	string hourStr = hour < 10 ? "0" + to_string(hour) : to_string(hour);
	string minuteStr = minute < 10 ? "0" + to_string(minute) : to_string(minute);
	string secondStr = second < 10 ? "0" + to_string(second) : to_string(second);
	string mSecondStr = mSecond < 10 ? "00" + to_string(mSecond) : mSecond < 100 ? "0" + to_string(mSecond) : to_string(mSecond);
	string uSecondStr = uSecond < 10 ? "00" + to_string(uSecond) : uSecond < 100 ? "0" + to_string(uSecond) : to_string(uSecond);

	stringstream str(stringstream::out | stringstream::binary);
	str << hourStr << ":" << minuteStr << ":" << secondStr << ":" << mSecondStr << ":" << uSecondStr;

	return str.str();
}

///******	Private Functions	******///

void AcquireSim::ResetParams()
{
	//terminate previous tasks if exist:
	AcquireSim::Stop();

	//reset default params:
	simExtendBuf = false;
	AcquireSim::_totalNumThreads = 0;
	simSkipOverflowCheck = TRUE;
	AcquireSim::counterNames.clear();
	AcquireSim::_ailineName.clear();
	AcquireSim::_dilineName.clear();
	AcquireSim::simStimulusParams->enable = false;
	AcquireSim::simStimulusParams->signalType = 0;
	AcquireSim::simStimulusParams->stimChannelID = 0;
	AcquireSim::simStimulusParams->threshold = 0;
	AcquireSim::simStimulusParams->lineName = "";
	if(AcquireSim::_simData)
	{
		SAFE_DELETE_ARRAY(AcquireSim::_simData->gCtr);

		SAFE_DELETE_ARRAY(AcquireSim::_simData->aiData);

		SAFE_DELETE_ARRAY(AcquireSim::_simData->diData);

		SAFE_DELETE_ARRAY(AcquireSim::_simData->ciData);

		delete AcquireSim::_simData;
		AcquireSim::_simData = NULL;
	}

	if(_displayCompData)
	{
		delete _displayCompData;
		_displayCompData = NULL;
	}

}

void AcquireSim::ResetTimingParams()
{
	//reset timing info,
	//don't do this if trying to run restart:
	simInitialGCtr = 0;
	simCurrentFirstGCtr = 0;
	AcquireSim::gCtrOverflowCnt = 0;
	AcquireSim::ciLogFile = L"";
	AcquireSim::ciLogSuffix = 0;
	simInitializedGCtr = false;
	simSkipOverflowCheck = FALSE;
	AcquireSim::lastCountValue = 0;
	AcquireSim::elapsedTimeUS = 0;
}

///********************************	END Run Synchronous Tasks	********************************///

///********************************	Run Asynchronous Tasks		********************************///
long AcquireSim::GetAsyncAcquiring()
{
	return AcquireSim::_isAsyncAcquiring;
}

long AcquireSim::StartAsync()
{
	long ret = TRUE;
	return ret;
}

long AcquireSim::StopAsync()
{
	long ret = TRUE;
	return ret;
}
///********************************	END Run Asynchronous Tasks	********************************///
