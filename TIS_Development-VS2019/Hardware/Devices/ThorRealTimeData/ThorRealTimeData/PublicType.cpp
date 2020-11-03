#include "stdafx.h"
#include "RealTimeDataXML.h"
#include "strsafe.h"

#ifdef LOGGING_ENABLED
std::auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

//							     0AI	1DI	   2CI    3CT    4PFI    5DO    6VT    7FI    8VF
const char * SignalTypeStr[] = {"/AI", "/DI", "/CI", "/CT", "/PFI", "/DO", "/VT", "/FI", "/VF" };

std::unique_ptr<HDF5ioDLL> h5Loader(new HDF5ioDLL(L".\\Modules_Native\\HDF5IO.dll"));
std::unique_ptr<RealTimeDataXML> _realtimeXML = NULL;
std::unique_ptr<OTMDataXML> _otmXML = NULL;

wchar_t _episodefile[_MAX_PATH];
std::wstring ChannelCenter::_lastError;

ChannelCenter::ChannelCenter() 
{
	_isLive = FALSE;
	_isLoading = FALSE;
	_isLoaded = 0;			//[0: non-initialized, -1: partical-loaded, 1: full-loaded]
	_stopLoading = FALSE;
	_freqRangeIdx[0] = _freqRangeIdx[1] = 0;
	_threadTime = MIN_THREAD_TIME;
	_asyncParam.arrayPtr = NULL;
}

ChannelCenter::~ChannelCenter() 
{
	_instanceFlag = false;
	SAFE_DELETE_ARRAY(_asyncParam.arrayPtr);
}

//instance flag must initialize after constructor
bool ChannelCenter::_instanceFlag;
std::unique_ptr<ChannelCenter> ChannelCenter::_single;

ChannelCenter* ChannelCenter::getInstance()
{
	if(! _instanceFlag)
	{ 
		_single.reset(new ChannelCenter()); 
		_instanceFlag = true;
	} 
	return _single.get(); 
}

void ChannelCenter::EnabledChannels(std::vector<Channels> channelVec) 
{
	ClearAll();

	for (int i = 0; i < channelVec.size(); i++)
	{
		if(0 == channelVec.at(i).type.compare(SignalTypeStr[0]))
		{
			_enabledAI.push_back(channelVec.at(i).alias); 
		}
		else if(0 == channelVec.at(i).type.compare(SignalTypeStr[1]))
		{
			_enabledDI.push_back(channelVec.at(i).alias); 
		}
		else if(0 == channelVec.at(i).type.compare(SignalTypeStr[2]))
		{
			_enabledCI.push_back(channelVec.at(i).alias); 
		}
		else if(0 == channelVec.at(i).type.compare(SignalTypeStr[7]))
		{
			_enabledFI.push_back(channelVec.at(i).alias); 
		}
		else if(0 == channelVec.at(i).type.compare(SignalTypeStr[6]))
		{
			_enabledVT.push_back(channelVec.at(i).alias); 
		}
		else if(0 == channelVec.at(i).type.compare(SignalTypeStr[8]))
		{
			_enabledVF.push_back(channelVec.at(i).alias); 
		}
	} 
}

std::vector<VirtualVariable> ChannelCenter::YieldVariables(std::string sourceName)
{
	std::vector<VirtualVariable> varVec;

	//add variable if found reserved keyword "VAR"
	if (sourceName.find(VAR) != std::string::npos)
	{
		//std::regex baseRegex("VAR\\[\\d+\\]"); //if VAR[0]
		std::regex baseRegex("VAR\\d+");
		std::smatch baseMatch;
		std::sregex_iterator iter(sourceName.begin(), sourceName.end(), baseRegex);
		std::sregex_iterator end;

		while(iter != end)
		{
			baseMatch = *iter; 
			VirtualVariable var(baseMatch.str());

			std::regex intRegex("\\d+");
			std::smatch iMatch;
			if(std::regex_search(var.name, iMatch, intRegex))
			{
				var.sType = SignalType::LAST_SIGNAL_TYPE;
				var.pValue = _globalVar[std::stoi(iMatch[0].str())].value;
				var.offset = 0;
				varVec.push_back(var);
			}
			++iter;
		}
	}

	YieldVariable(sourceName, varVec, _enabledAI, SignalType::ANALOG_IN);

	YieldVariable(sourceName, varVec, _enabledDI, SignalType::DIGITAL_IN);

	YieldVariable(sourceName, varVec, _enabledVT, SignalType::VIRTUAL);

	YieldVariable(sourceName, varVec, _enabledFI, SignalType::SPECTRAL);

	YieldVariable(sourceName, varVec, _enabledVF, SignalType::SPECTRAL_VIRTUAL);

	return varVec;
}

long ChannelCenter::LoadXML(long reload, const wchar_t* targetXML)
{
	//decide if reload is necessary
	wchar_t xmlFile[_MAX_PATH];
	std::wstring tgtXML = L"ThorRealTimeDataSettings.xml", currentXML;
	auto t1 = Clock::now();
	if(NULL != targetXML)
	{
		tgtXML = std::wstring(targetXML);
	}
	if(NULL != _realtimeXML)
	{
		_realtimeXML.get()->GetPathFilename(xmlFile);
		currentXML = std::wstring(xmlFile);
	}
	if(reload || (0 != tgtXML.compare(currentXML) || (0 == tgtXML.size() && 0 == currentXML.size())))
	{

		if (0 < tgtXML.size())
		{
			StringCbPrintfW(xmlFile,MSG_LENGTH,tgtXML.c_str());
			_realtimeXML.reset(new RealTimeDataXML(xmlFile));
		}
		else
		{
			_realtimeXML.reset(new RealTimeDataXML());
		}

		//load xml settings
		if(_realtimeXML.get()->OpenConfigFile())
		{
			//clear before load
			_dataChannel.clear();
			_virChannel.clear();
			_specChannel.clear();
			_specVirChannel.clear();

			_realtimeXML.get()->GetBoard(_board);
			_realtimeXML.get()->GetFilePath(&_filePath);
			_realtimeXML.get()->GetMode(&_mode);
			_realtimeXML.get()->GetDataChannel(_dataChannel);
			_realtimeXML.get()->GetVirtualChannel(_virChannel);
			_realtimeXML.get()->GetSave(_threadTime);
			_threadTime = (MIN_THREAD_TIME <= _threadTime) ? _threadTime : MIN_THREAD_TIME;
			_realtimeXML.get()->GetAsyncMode(&_asyncParam);
			_asyncParam.board = _board;
			_realtimeXML.get()->GetInvert(&_invert);
			_realtimeXML.get()->GetVariables(_globalVar);

			//setup channels other than datas:
			if(_realtimeXML.get()->GetSpectralDomain(_specParam))
			{
				_realtimeXML.get()->GetSpecChannel(_specChannel);
				_realtimeXML.get()->GetSpecVirtualChannel(_specVirChannel);
			}

			_isLive = TRUE;
			EnabledChannels(_dataChannel + _virChannel + _specChannel + _specVirChannel);
		}

		//load OTM xml from default for OTM virtual channels
		_otmXML.reset(new OTMDataXML());
		_otmXML.get()->OpenConfigFile(TRUE);
		_otmXML.get()->GetFittings(&_otmFit);

	}
	auto t2 = Clock::now();

#if _DEBUG
	milliseconds ms = duration_cast<milliseconds>(t2-t1);
	StringCbPrintfW(message,MSG_LENGTH,L"LoadXML: %u ms", static_cast<unsigned long>(ms.count()));
	LogMessage(message, VERBOSE_EVENT);
#endif
	return TRUE;
}

void ChannelCenter::ReloadGlobalVariables()
{
	if(NULL != _realtimeXML.get())
	{
		_realtimeXML.get()->OpenConfigFile(TRUE);
		_realtimeXML.get()->GetVariables(_globalVar);
	}
}

long ChannelCenter::LoadEpisode()
{
	//initialize flags
	_isLoaded = _isLive = _stopLoading = FALSE;
	_isLoading = TRUE;

	if(FALSE == InitialEpisode())
	{
		_isLoading = FALSE;
		return FALSE;
	}

	//check size, will load percent by percent
	if(FALSE == h5Loader->OpenFileIO(_episodefile,H5FileType::READWRITE))
	{
		_isLoading = FALSE;
		return FALSE;
	}
	_timeSec.clear();
	unsigned long long size = 0, currentSize = 0;
	h5Loader->CheckGroupDataset("/Global", "/GCtr", size);
	_timeSec.reserve(size);
	unsigned long long percentSize = static_cast<unsigned long long>(size / HUNDRED_PERCENT);
	CompoundData* cData = new CompoundData(percentSize, _enabledAI.size()*percentSize, _enabledDI.size()*percentSize, _enabledCI.size()*percentSize, _enabledVT.size()*percentSize);
	unsigned long overflowGCtrCnt = 0;
	cData->SetupGlobalCounter(overflowGCtrCnt);
	h5Loader->CloseFileIO();

	try
	{
		std::vector<std::string> groupNameVec;
		if (FALSE == LoadEpisodeGroupDatasetNames(_episodefile, groupNameVec, _fileDatasetMap))
		{
			goto DONE_LOAD;
		}
		if(FALSE == VerifyFileChannels(_fileDatasetMap))
		{
			goto DONE_LOAD;
		}
		if(TRUE == h5Loader->OpenFileIO(_episodefile,H5FileType::READWRITE))
		{
			for (int j = 0; j < HUNDRED_PERCENT; j++)
			{
				//check if reset memory at last percent
				if(static_cast<int>(HUNDRED_PERCENT) - 1 == j)
				{
					unsigned long long lastSize = size - currentSize;
					if(lastSize != percentSize)
					{
						delete cData;
						cData = new CompoundData(lastSize, _enabledAI.size()*lastSize, _enabledDI.size()*lastSize, _enabledCI.size()*lastSize, _enabledVT.size()*lastSize);
						cData->SetupGlobalCounter(overflowGCtrCnt);
						percentSize = lastSize;
					}
				}

				//load time by global counter, also prepare time vector for spec analysis
				_isLoaded = h5Loader->ReadData("/Global", "/GCtr", cData->GetStrucData()->gCtr64Ptr, H5DataTypeEnum::DATA_UINT64, currentSize, percentSize);
				for (int k = 0; k < percentSize; k++)
				{
					_timeSec.emplace_back(static_cast<double>(*(cData->GetStrucData()->gCtr64Ptr + k)) / CLK_RATE_20MHZ);
				}

				//start loading from file for all enabled channels
				_isLoaded = LoadEpisodeDataOnly(h5Loader.get(), cData, 0, percentSize, currentSize, percentSize);

				//offset index and reset counter
				currentSize += cData->GetgcLengthComValue();

				//return if user request stop or failed
				if((1 != _isLoaded) || (_stopLoading))
					break;

				//notice buffer is ready
				(*dataPointer)(cData->GetStrucData());
			}
		}
		goto DONE_LOAD;
	}
	catch(...)
	{
		goto DONE_LOAD;
	}

DONE_LOAD:
	delete cData;
	h5Loader->CloseFileIO();
	_isLoading = FALSE;
	return _isLoaded;
}

long ChannelCenter::LoadEpisodeDataOnly(void* file, CompoundData* cData, unsigned long long dOffset, unsigned long long dLength, unsigned long long start, unsigned long long length)
{
	long ret = TRUE;
	HDF5ioDLL* h5File = (HDF5ioDLL*)file;

	std::vector<Channels> channels = _dataChannel + _virChannel;

	int counter[SignalType::LAST_SIGNAL_TYPE] = {0};

	for (int i = 0; i < channels.size(); i++)
	{
		switch ((SignalType)channels[i].signalType)
		{
		case SignalType::ANALOG_IN:
			ret = h5File->ReadData(channels[i].type.c_str(), ("/" + channels[i].alias).c_str(), cData->GetStrucData()->aiDataPtr + (counter[SignalType::ANALOG_IN] * dLength) + dOffset, H5DataTypeEnum::DATA_DOUBLE, start, length);
			counter[SignalType::ANALOG_IN]++;
			break;
		case SignalType::DIGITAL_IN:
			ret = h5File->ReadData(channels[i].type.c_str(), ("/" + channels[i].alias).c_str(), cData->GetStrucData()->diDataPtr + (counter[SignalType::DIGITAL_IN] * dLength) + dOffset, H5DataTypeEnum::DATA_UINT32, start, length);
			counter[SignalType::DIGITAL_IN]++;
			break;
		case SignalType::COUNTER_IN:
			ret = h5File->ReadData(channels[i].type.c_str(), ("/" + channels[i].alias).c_str(), cData->GetStrucData()->ciDataPtr + (counter[SignalType::COUNTER_IN] * dLength) + dOffset, H5DataTypeEnum::DATA_UINT32, start, length);
			counter[SignalType::COUNTER_IN]++;
			break;
		case SignalType::VIRTUAL:
			ret = h5File->ReadData(channels[i].type.c_str(), ("/" + channels[i].alias).c_str(), cData->GetStrucData()->viDataPtr + (counter[SignalType::VIRTUAL] * dLength) + dOffset, H5DataTypeEnum::DATA_DOUBLE, start, length);
			counter[SignalType::VIRTUAL]++;
			break;
		}

		//return if any failed
		if(0 == ret) 
			break;

		//return if user request stop
		if(_stopLoading)
		{
			ret = -1;	//partially loaded
			break;
		}
	}
	return ret;
}

long ChannelCenter::LoadEpisodeGroupDatasetNames(const wchar_t* episodeName, std::vector<std::string> &groupNameVec, std::map<std::string, std::vector<std::string>> &datasetMap)
{
	char*** groupNames = new char**[_MAX_PATH];
	long groupCount;
	char*** datasetNames = new char**[_MAX_PATH];
	long datasetCount;

	try
	{
		//locate groups and datasets
		if(TRUE == h5Loader->OpenFileIO(episodeName,H5FileType::READWRITE))
		{
			groupNameVec.clear();
			if(TRUE == h5Loader->GetGroupDatasetNames("/",groupNames,&groupCount,datasetNames,&datasetCount))
			{
				//group names only
				for (long i = 0; i < groupCount; i++)
				{
					groupNameVec.push_back(std::string(groupNames[0][i]));
					::CoTaskMemFree(groupNames[0][i]);
				}
				for (long i = 0; i < datasetCount; i++)
				{
					::CoTaskMemFree(datasetNames[0][i]);
				}
			}
			datasetMap.clear();
			for (int i = 0; i < static_cast<int>(groupNameVec.size()); i++)
			{
				std::string gpName = "/" + groupNameVec[i];
				if(TRUE == h5Loader->GetGroupDatasetNames(gpName.c_str(),groupNames,&groupCount,datasetNames,&datasetCount))
				{
					for (long i = 0; i < groupCount; i++)
					{
						::CoTaskMemFree(groupNames[0][i]);
					}
					//dataset names only
					std::vector<std::string> dataSet;
					for (long i = 0; i < datasetCount; i++)
					{
						dataSet.push_back("/" + std::string(datasetNames[0][i]));
						::CoTaskMemFree(datasetNames[0][i]);
					}
					datasetMap.insert(std::pair<std::string,std::vector<std::string>>(gpName,dataSet));
				}
			}
		}
		h5Loader->CloseFileIO();
		SAFE_DELETE_ARRAY(groupNames);
		SAFE_DELETE_ARRAY(datasetNames);
	}
	catch(...)
	{
		h5Loader->CloseFileIO();
		SAFE_DELETE_ARRAY(groupNames);
		SAFE_DELETE_ARRAY(datasetNames);
		StringCbPrintfW(message,MSG_LENGTH,L"Unable to load group and dataset from episode (%ls).", episodeName);
		LogMessage(message, ERROR_EVENT);
		_lastError = message;
		return FALSE;
	}
	return TRUE;
}

long ChannelCenter::SaveSpectral(FreqCompDataStruct* fData)
{
	//return if user request stop, _isLoading flag will be set by SpectralManager
	if((NULL == fData) || (ChannelCenter::getInstance()->_stopLoading))
		return FALSE;

	long ret = TRUE;
	int counter[SignalType::LAST_SIGNAL_TYPE] = {0};

	if(TRUE == h5Loader->OpenFileIO(_episodefile, H5FileType::READWRITE))
	{
		//push data in respective groups, freq, freqFit, then line data
		if(0 < fData->freqLength)
		{
			if(FALSE == h5Loader->ExtendData("/Freq", "/Hz", fData->freqData, H5DataType::DATA_DOUBLE, false, static_cast<unsigned long>(fData->freqLength)))
			{
				StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file frequency.");
				LogMessage(message,ERROR_EVENT);
			}
		}
		if(0 < fData->freqFitLength)
		{
			if(FALSE == h5Loader->ExtendData("/Freq", "/FitHz", fData->freqFitData, H5DataType::DATA_DOUBLE, false, static_cast<unsigned long>(fData->freqFitLength)))
			{
				StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file fit frequency.");
				LogMessage(message,ERROR_EVENT);
			}
		}
		for(int i = 0; i < _specChannel.size(); i++)
		{
			if(0 < fData->specDataLength)
			{
				if(FALSE == h5Loader->ExtendData(_specChannel[i].type.c_str(), ("/" + _specChannel[i].alias).c_str(), (fData->specDataRe + counter[SignalType::SPECTRAL]*fData->freqLength), H5DataType::DATA_DOUBLE, false, static_cast<unsigned long>(fData->freqLength)))
				{
					StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file spectral channel (%s)", _specChannel[i].alias);
					LogMessage(message,ERROR_EVENT);
				}
				counter[SignalType::SPECTRAL]++;
			}
			if(_stopLoading)
			{
				h5Loader->CloseFileIO();
				return FALSE;
			}
		}
		for(int i = 0; i < _specVirChannel.size(); i++)
		{
			if(0 < fData->vspecDataLength)
			{
				unsigned long tmpLength = ((_specVirChannel[i].lineId.find(LORENTZIANFITX) != std::string::npos) || (_specVirChannel[i].lineId.find(LORENTZIANFITY) != std::string::npos)) ? (static_cast<unsigned long>(fData->freqFitLength)) : (static_cast<unsigned long>(fData->freqLength));

				if(FALSE == h5Loader->ExtendData(_specVirChannel[i].type.c_str(), ("/" + _specVirChannel[i].alias).c_str(), (fData->vSpecData + counter[SignalType::SPECTRAL_VIRTUAL]*fData->freqLength), H5DataType::DATA_DOUBLE, false, tmpLength))
				{
					StringCbPrintfW(message,MSG_LENGTH,L"Error writing HDF5 file spectral virtual channel (%s)", _specVirChannel[i].alias);
					LogMessage(message,ERROR_EVENT);
				}
				counter[SignalType::SPECTRAL_VIRTUAL]++;
			}
			if(_stopLoading)
			{
				h5Loader->CloseFileIO();
				return FALSE;
			}
		}
	}
	h5Loader->CloseFileIO();
	return TRUE;
}

long ChannelCenter::SetupFileH5(unsigned long long length)
{
	long ret = TRUE;
	//since H5 file is unable to add nodes after creation,
	//use this function to create nodes:
	h5Loader->GetPathandFilename(_episodefile,_MAX_PATH);
	std::wstring tmpStr = _episodefile;
	if(tmpStr.size() == 0)
	{
		StringCbPrintfW(message,MSG_LENGTH,L"No HDF5 file is created.");
		LogMessage(message,VERBOSE_EVENT);
		_lastError = message;
		return FALSE;
	}
	try
	{
		if(h5Loader->CreateFileIO(_episodefile, H5FileType::OVERWRITE, length))
		{
			std::vector<std::string> aidataset, didataset, cidataset, gcdataset, vidataset;
			std::string aigroup, digroup, cigroup, gcgroup, vigroup;

			//put global counter in front:
			gcgroup = "/Global";
			gcdataset.push_back("/GCtr");
			const char ** gcdatachar = ConvertStrVec(gcdataset);
			h5Loader->CreateGroupDatasets(gcgroup.c_str(),gcdatachar,static_cast<long>(gcdataset.size()),H5DataType::DATA_UINT64);
			FreeCharVec(gcdatachar,gcdataset.size());
			for(int i=0;i<_dataChannel.size();i++)
			{
				if(0 == _dataChannel.at(i).type.compare(SignalTypeStr[0]))
				{
					aigroup = _dataChannel.at(i).type;
					aidataset.push_back("/" + _dataChannel.at(i).alias);
				}
				else if(0 == _dataChannel.at(i).type.compare(SignalTypeStr[1]))
				{
					digroup = _dataChannel.at(i).type;
					didataset.push_back("/" + _dataChannel.at(i).alias);
				}
				else if(0 == _dataChannel.at(i).type.compare(SignalTypeStr[2]))
				{
					cigroup = _dataChannel.at(i).type;
					cidataset.push_back("/" + _dataChannel.at(i).alias);
				}
			}
			for(int i = 0;i < _virChannel.size(); i++)
			{
				if(SignalType::VIRTUAL == _virChannel.at(i).signalType)
				{
					vigroup = _virChannel.at(i).type;
					vidataset.push_back("/" + _virChannel.at(i).alias);
				}
			}
			if(aidataset.size()>0)
			{
				const char ** aidatachar = ConvertStrVec(aidataset);
				ret = h5Loader->CreateGroupDatasets(aigroup.c_str(),aidatachar,static_cast<long>(aidataset.size()),H5DataType::DATA_DOUBLE);
				FreeCharVec(aidatachar,aidataset.size());
				if(FALSE == ret)
					goto __FILE_ERROR;
			}
			if(didataset.size()>0)
			{
				const char ** didatachar = ConvertStrVec(didataset);
				ret = h5Loader->CreateGroupDatasets(digroup.c_str(),didatachar,static_cast<long>(didataset.size()),H5DataType::DATA_UINT32);
				FreeCharVec(didatachar,didataset.size());
				if(FALSE == ret)
					goto __FILE_ERROR;
			}
			if(cidataset.size()>0)
			{
				const char ** cidatachar = ConvertStrVec(cidataset);
				ret = h5Loader->CreateGroupDatasets(cigroup.c_str(),cidatachar,static_cast<long>(cidataset.size()),H5DataType::DATA_UINT32);
				FreeCharVec(cidatachar,cidataset.size());
				if(FALSE == ret)
					goto __FILE_ERROR;
			}
			if(vidataset.size()>0)
			{
				const char ** vidatachar = ConvertStrVec(vidataset);
				ret = h5Loader->CreateGroupDatasets(vigroup.c_str(),vidatachar,static_cast<long>(vidataset.size()),H5DataType::DATA_DOUBLE);
				FreeCharVec(vidatachar,vidataset.size());
				if(FALSE == ret)
					goto __FILE_ERROR;
			}

			//set frequency nodes
			std::vector<std::string> fidataset, vfdataset, fdataset;
			std::string figroup, vfgroup, fgroup;
			fgroup = "/Freq";
			fdataset.push_back("/Hz");
			fdataset.push_back("/FitHz");
			const char ** fdatachar = ConvertStrVec(fdataset);
			ret = h5Loader->CreateGroupDatasets(fgroup.c_str(),fdatachar,static_cast<long>(fdataset.size()),H5DataType::DATA_DOUBLE);
			FreeCharVec(fdatachar,fdataset.size());
			if(FALSE == ret)
				goto __FILE_ERROR;

			//set frequency domains
			for(int i = 0;i < _specChannel.size(); i++)
			{
				figroup = _specChannel[i].type;
				fidataset.push_back("/" + _specChannel[i].alias);
			}
			if(fidataset.size()>0)
			{
				const char ** fidatachar = ConvertStrVec(fidataset);
				ret = h5Loader->CreateGroupDatasets(figroup.c_str(),fidatachar,static_cast<long>(fidataset.size()),H5DataType::DATA_DOUBLE);
				FreeCharVec(fidatachar,fidataset.size());
				if(FALSE == ret)
					goto __FILE_ERROR;
			}
			for (int i = 0; i < _specVirChannel.size(); i++)
			{
				vfgroup = _specVirChannel[i].type;
				vfdataset.push_back("/" + _specVirChannel[i].alias);
			}
			if(vfdataset.size()>0)
			{
				const char ** vfdatachar = ConvertStrVec(vfdataset);
				ret = h5Loader->CreateGroupDatasets(vfgroup.c_str(),vfdatachar,static_cast<long>(vfdataset.size()),H5DataType::DATA_DOUBLE);
				FreeCharVec(vfdatachar,vfdataset.size());
				if(FALSE == ret)
					goto __FILE_ERROR;
			}

			h5Loader->CloseFileIO();
		}
		else
		{
			StringCbPrintfW(message,MSG_LENGTH,L"Unable to create HDF5 file: (&s)\n", tmpStr);
			LogMessage(message,ERROR_EVENT);
			_lastError = message;
			return FALSE;
		}
	}
	catch(...)
	{
		ret = FALSE;
	}
	return ret;

__FILE_ERROR:
	h5Loader->CloseFileIO();
	StringCbPrintfW(message,MSG_LENGTH,L"\nInvalid line name for HDF5 file, no special characters allowed.");
	LogMessage(message,ERROR_EVENT);
	_lastError = message;
	return FALSE;
}

long ChannelCenter::SpectralAnalysis()
{
	long ret = TRUE;
	_isLive = _stopLoading = FALSE;
	_isLoading = TRUE;
	//skip load settings time domain data was properly loaded
	if(1 != _isLoaded)
	{
		if(0 == _isLoaded)
		{
			if(FALSE == InitialEpisode() || (FALSE == VerifyFileChannels(_fileDatasetMap)))
			{
				StringCbPrintfW(message,MSG_LENGTH,L"SpectralAnalysis Failed at initialization step.");
				LogMessage(message, ERROR_EVENT);
				goto FALSE_RETURN;	
			}
		}
		//reload all time range assuming it was not fully loaded
		if(TRUE == h5Loader->OpenFileIO(_episodefile,H5FileType::READWRITE))
		{
			_timeSec.clear();
			unsigned long long* tempTime = NULL;
			unsigned long long size = 0, currentSize = 0;
			h5Loader->CheckGroupDataset("/Global", "/GCtr", size);
			_timeSec.reserve(size);
			tempTime = (unsigned long long*)realloc(tempTime, size * sizeof(unsigned long long));
			if(h5Loader->ReadData("/Global", "/GCtr", tempTime, H5DataTypeEnum::DATA_UINT64, 0, size))
			{
				for (unsigned long long i = 0; i < size; i++)
				{
					_timeSec.emplace_back((double)*(tempTime + i) / _mode.sampleRate);
				}
			}
			SAFE_DELETE_MEMORY(tempTime);
		}
		h5Loader->CloseFileIO();

		//done reload of time, change flag for not repeating this step
		_isLoaded = 1;
	}

	//reload xml for specParam and spec channels
	if(!(_realtimeXML.get()->OpenConfigFile(true)) || !(_realtimeXML.get()->GetSpectralDomain(_specParam)))
		goto FALSE_RETURN;

	_realtimeXML.get()->GetSpecChannel(_specChannel);
	_realtimeXML.get()->GetSpecVirtualChannel(_specVirChannel);

	//reload OTM xml
	_otmXML.get()->OpenConfigFile(TRUE);
	_otmXML.get()->GetParameter(&_otmParam);
	_otmXML.get()->GetFittings(&_otmFit);

	//locate start and end indexes
	size_t idxStart = std::distance(_timeSec.begin(), std::lower_bound(_timeSec.begin(), _timeSec.end(), _specParam.sampleMinSec));
	size_t idxEnd = std::distance(_timeSec.begin(), std::lower_bound(_timeSec.begin(), _timeSec.end(), _specParam.sampleMaxSec));
	idxStart = (idxEnd > idxStart) ? idxStart : idxEnd;
	size_t length = (idxEnd > idxStart) ? (idxEnd-idxStart) : (idxStart-idxEnd);
	if((0 == length) || (0 == _specChannel.size()))
		goto FALSE_RETURN;

	//update spectral manager
	SpectralManager::getInstance()->SetSaveFile(TRUE);
	SpectralManager::getInstance()->UpdateSpectralAnalyzer(static_cast<long>(length));

	//load time data for DFT
	CompoundData* cData = new CompoundData(length, _enabledAI.size()*length, _enabledDI.size()*length, _enabledCI.size()*length, _enabledVT.size()*length);
	if(TRUE == h5Loader->OpenFileIO(_episodefile,H5FileType::READWRITE))
	{
		ret = _isLoaded = LoadEpisodeDataOnly(h5Loader.get(), cData, 0, length, idxStart, length);
	}
	h5Loader->CloseFileIO();
	if(ret)
		SpectralManager::getInstance()->WriteSource(cData);

	delete cData;
	return TRUE;

FALSE_RETURN:
	_isLoading = FALSE;
	return FALSE;
}

long ChannelCenter::VerifyFileChannels(std::map<std::string, std::vector<std::string>> fileNameMap)
{
	long ret = TRUE;
	for (std::map<std::string, std::vector<std::string>>::iterator it = fileNameMap.begin(); it != fileNameMap.end(); it++)
	{
		for (int j = 0; j < it->second.size(); j++)
		{
			if(0 == it->first.compare(SignalTypeStr[0]) && (std::find(_enabledAI.begin(),_enabledAI.end(),it->second.at(j)) != _enabledAI.end()))
			{
				ret = FALSE; 
				break;
			}
			if(0 == it->first.compare(SignalTypeStr[1]) && (std::find(_enabledDI.begin(),_enabledDI.end(),it->second.at(j)) != _enabledDI.end()))
			{
				ret = FALSE; 
				break;
			}
			if(0 == it->first.compare(SignalTypeStr[2]) && (std::find(_enabledCI.begin(),_enabledCI.end(),it->second.at(j)) != _enabledCI.end()))
			{
				ret = FALSE; 
				break;
			}
			if(0 == it->first.compare(SignalTypeStr[6]) && (std::find(_enabledVT.begin(),_enabledVT.end(),it->second.at(j)) != _enabledVT.end()))
			{
				ret = FALSE; 
				break;
			}
			if(0 == it->first.compare(SignalTypeStr[7]) && (std::find(_enabledFI.begin(),_enabledFI.end(),it->second.at(j)) != _enabledFI.end()))
			{
				ret = FALSE; 
				break;
			}
			if(0 == it->first.compare(SignalTypeStr[8]) && (std::find(_enabledVF.begin(),_enabledVF.end(),it->second.at(j)) != _enabledVF.end()))
			{
				ret = FALSE; 
				break;
			}
		}
		if (FALSE == ret)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"Channels are not consistent between file and settings: (%ls)", StringToWString(it->first).c_str());
			LogMessage(message, ERROR_EVENT);
			_lastError = message;
			return FALSE;
		}
	}
	return ret;
}

void ChannelCenter::UpdateOTM()
{
	if(!_isLive && _otmParam.isCurveFit)
	{
		_otmXML.get()->SetParameter(&_otmParam);
		_otmXML.get()->SetFittings(&_otmFit);

		//update variables for OTM virtual channels,
		//ID 1-4:beta1XY, kappaXY
		if(4 < _globalVar.size())
		{
			_globalVar[1].value = _otmFit.betaXY1[0];
			_globalVar[2].value = _otmFit.betaXY1[1];
			_globalVar[3].value = _otmFit.kappaXY[0];
			_globalVar[4].value = _otmFit.kappaXY[1];

			//save as global settings limited to part of variables
			_realtimeXML.get()->SetVariables(_globalVar);

			std::unique_ptr<RealTimeDataXML> tmpXML;
			tmpXML.reset(new RealTimeDataXML(L"ThorRealTimeDataSettings.xml"));
			tmpXML.get()->OpenConfigFile();
			tmpXML.get()->SetVariables(_globalVar);
		}
	}
}

///********************************	Private Functions	********************************///

void ChannelCenter::ClearAll()
{
	_enabledAI.clear();
	_enabledDI.clear();
	_enabledFI.clear();
	_enabledVT.clear();
	_enabledVF.clear();
}

long ChannelCenter::InitialEpisode()
{
	const int XML_TYPES = 2;
	h5Loader->GetPathandFilename(_episodefile,_MAX_PATH);
	std::wstring wPath(_episodefile);

	//locate settings file under the same folder
	std::wstring folder = wPath.substr(0, wPath.find_last_of(L"\\")) + L"\\";
	wPath = folder + L"*.xml";

	//bool found[XML_TYPES] = {false};
	std::wstring foundFiles[XML_TYPES];

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(wPath.c_str(), &ffd);
	if(INVALID_HANDLE_VALUE == hFind)
	{
		FindClose(hFind);
		StringCbPrintfW(message,MSG_LENGTH,L"Settings file is not available.");
		LogMessage(message, ERROR_EVENT);
		return FALSE;
	}
	do
	{
		if(std::wstring(ffd.cFileName).find(L"ThorRealTimeDataSettings.xml") != std::string::npos)
		{
			//found[0] = true;
			foundFiles[0] = std::wstring(folder + ffd.cFileName);
		}
		else if(std::wstring(ffd.cFileName).find(L"OTMSettings.xml") != std::string::npos)
		{
			//found[1] = true;
			foundFiles[1] = std::wstring(folder + ffd.cFileName);
		}

	}while(FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);

	if(0 < foundFiles[0].length())
		LoadXML(FALSE, foundFiles[0].c_str());

	if(0 < foundFiles[1].length())
		_otmXML.reset(new OTMDataXML(foundFiles[1].c_str()));

	return (0 < foundFiles[0].length()) ? TRUE : FALSE;
}

void ChannelCenter::YieldVariable(std::string sourceName, std::vector<VirtualVariable>& vars, std::vector<std::string> enabledLines, SignalType sType)
{
	//search enabled lines, add variable if found
	for (int j = 0; j < static_cast<int>(enabledLines.size()); j++)
	{
		if(sourceName.find(enabledLines[j]) != std::string::npos)
		{
			VirtualVariable var(enabledLines[j]);
			var.sType = sType;
			var.pValue = NULL;
			var.offset = j;
			vars.push_back(var);
		}
	}
}
