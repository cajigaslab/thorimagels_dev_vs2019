// ThorElectroPhys.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include "math.h"
#include "Strsafe.h"
#include "ThorElectroPhys.h"
#include "ElectroPhysSetupXML.h"
#include "..\..\..\..\Tools\National Instruments\NIDAQmx\include\BoardInfoNI.h"

ThorElectroPhys::ThorElectroPhys()
{
	_deviceDetected = FALSE;
	_numDevices = 0;
	_ringBufferSize = 1;
	_ringBufferUnit = (long)Constants::ACTIVE_LOAD_UNIT_SIZE;
	_targetCount = 0;

	_taskHandleDI0 = NULL;
	for(long i=0; i<MAX_DIG_PORT_OUTPUT; i++)
		_taskHandleDO[i] = NULL;

	_taskTriggerHandle[0] = _taskTriggerHandle[1] = NULL;
	_taskTriggerCO[0] = _taskTriggerCO[1] = NULL;
	_parkAnalogLineAtLastVoltage = FALSE;
	_analogTriggerSet = false;
	ResetParams();
}

ThorElectroPhys::~ThorElectroPhys()
{
	_instanceFlag = false;
}

bool ThorElectroPhys:: _instanceFlag = false;

std::auto_ptr<ThorElectroPhys> ThorElectroPhys::_single(new ThorElectroPhys());

wchar_t message[_MAX_PATH]; ///<reserved for GUI display error message
wchar_t messageLog[_MAX_PATH]; ///<used for logging
char errMsg[_MAX_PATH]; ///<used to get error from NI API

EPhysTriggerStruct* _triggerStruct = NULL;
std::unique_ptr<BoardInfoNI> boardInfoNI;
std::unique_ptr<WaveformBuilderDLL> waveformDOBuilder(new WaveformBuilderDLL(L".\\Modules_Native\\GeometryUtilitiesCPP.dll"));
std::unique_ptr<WaveformBuilderDLL> waveformAOBuilder(new WaveformBuilderDLL(L".\\Modules_Native\\GeometryUtilitiesCPP.dll"));
std::unique_ptr<BlockRingBuffer> ThorElectroPhys::_bRingBuf[2] = {NULL, NULL};	//[0:DO,1:AO]
uInt8* ThorElectroPhys::_localBuffer = NULL;
long ThorElectroPhys::_localBufferSizeInBytes = 0;
std::vector<std::string> ThorElectroPhys::_triggerConfig;
TaskHandle ThorElectroPhys::_taskTriggerHandle[2] = {NULL, NULL};	//[0:DO,1:AO]
TaskHandle ThorElectroPhys::_taskTriggerCO[2] = {NULL, NULL};		//[0:DO,1:AO]
TaskHandle ThorElectroPhys::_taskTriggerCI = NULL;
unsigned long long ThorElectroPhys::_outputCount = 0;
double* ThorElectroPhys::_freqMeasure = NULL;
unsigned int ThorElectroPhys::_freqBufSize = 0;
std::unique_ptr<CircularBuffer> ThorElectroPhys::_freqCirBuf(new CircularBuffer("FreqProbe"));
HANDLE ThorElectroPhys::_freqThreadStopped = CreateEvent(NULL, TRUE, TRUE, NULL);	// (MANUAL RESET)
HANDLE ThorElectroPhys::_freqThread = NULL;
const uInt8 TASKDO = 0;
const uInt8 TASKAO = 1;


ThorElectroPhys *ThorElectroPhys::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorElectroPhys());
		_instanceFlag = true;
	}
	return _single.get();
}

int32 CVICALLBACK ThorElectroPhys::TriggerCOCallback (TaskHandle taskHandle, int32 signalID, void *callbackData) 
{	
	_outputCount++;
	return 0;
}

int32 CVICALLBACK ThorElectroPhys::EveryNTriggerCallback (TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	try
	{
		uInt8 id = (uInt8)callbackData;

		//terminate if necessary
		if(!_triggerStruct->enable)
		{
			goto TERM_TASK;
		}

		if(_taskTriggerHandle[id])
		{
			//return if no more buffer to read
			if(0 == _bRingBuf[id]->GetReadableBlockCounts())
				return (-1);

			//read then write all available buffer
			if(FALSE == FillupAvailableBuffer(id))
				return (-1);
		}
	}
	catch(...)
	{
		goto TERM_TASK;
	}
	return 0;

TERM_TASK:
	CloseTriggerTasks();
	return 0;
}

UINT ThorElectroPhys::FreqMeasureAsync(void)
{
	int32 error = 0, read = 0;
	try
	{
		while ((0 == error) && (NULL != _freqMeasure))
		{
			DAQmxErrChk (L"DAQmxReadCounterF64", error = DAQmxReadCounterF64(_taskTriggerCI,_freqBufSize,10.0,_freqMeasure,_freqBufSize,&read,0));
			if (0 < read && _freqCirBuf.get()->TryLock())
			{
				_freqCirBuf.get()->Write((const char*)&_freqMeasure[0], sizeof(double));
				_freqCirBuf.get()->ReleaseLock();
			}
		}
	}
	catch(...)
	{
		_freqCirBuf.get()->ReleaseLock();
		StringCbPrintfW(message,_MAX_PATH, L"ThorElectroPhys break at FreqMeasureAsync due to error(%d).", error);
		LogMessage(message, VERBOSE_EVENT);
	}
	//terminate task before leaving and notice the thread is stopped
	TerminateTask(_taskTriggerCI);
	SetEvent(_freqThreadStopped);
	return 0;
}

long ThorElectroPhys::FindDevices(long &deviceCount)
{
	long ret = TRUE;
	int32 error = 0;

	deviceCount = 0;
	//Get filter parameters from hardware setup.xml
	std::auto_ptr<ElectroPhysXML> pSetup(new ElectroPhysXML());

	double volts2mm=0;
	double offsetmm=0;
	double zPos_min=0;
	double zPos_max=0;

	if (ret=pSetup->OpenConfigFile())
	{
		//prepare to reset tasks
		CloseNITasks();
		CloseMeasureTasks();
		CloseTriggerTasks();
		ResetParams();

		//get & set electro phys digital input
		if(pSetup->GetIO(_devName,_digitalPort))
		{
			try
			{
				if((_devName.size() == 0) || (_digitalPort.size() == 0))
				{
					//StringCbPrintfW(message,_MAX_PATH, L"ThorElectroPhys unable to start due to empty line setting.");
					//LogMessage(message, ERROR_EVENT);
				}
				else
				{
					DAQmxErrChk(L"DAQmxCreateTask",error = DAQmxCreateTask("", &_taskHandleDI0));	
					DAQmxErrChk(L"DAQmxCreateDIChan",error = DAQmxCreateDIChan(_taskHandleDI0, (_devName + _digitalPort).c_str(), "", DAQmx_Val_ChanPerLine));
					DAQmxErrChk(L"DAQmxStartTask",error = DAQmxStartTask(_taskHandleDI0));
				}
			}
			catch(...)
			{
				StringCbPrintfW(message,_MAX_PATH, L"ThorElectroPhys unable to create DI due to error (%d).", error);
				LogMessage(message, ERROR_EVENT);
			}

		}
		//get & set electro phys digital output
		if(pSetup->GetDigOutput(_digitalPortOutput[0],_digitalPortOutput[1],_digitalPortOutput[2],_digitalPortOutput[3],_digitalPortOutput[4],_digitalPortOutput[5],_digitalPortOutput[6],_digitalPortOutput[7]))
		{
			for(int i=0; i<MAX_DIG_PORT_OUTPUT; i++)
			{
				try
				{
					_digitalPortOutputAvailable[i] = (true == _digitalPortOutput[i].empty()) ? false : true;
					if(_digitalPortOutputAvailable[i])
					{
						DAQmxErrChk(L"DAQmxCreateTask",error = DAQmxCreateTask("", &_taskHandleDO[i]));	
						DAQmxErrChk(L"DAQmxCreateDOChan",error = DAQmxCreateDOChan(_taskHandleDO[i], _digitalPortOutput[i].c_str(), "", DAQmx_Val_ChanPerLine));
					}
				}
				catch(...)
				{
					StringCbPrintfW(message,_MAX_PATH, L"ThorElectroPhys unable to create DO due to error (%d).", error);
					LogMessage(message, ERROR_EVENT);
				}
			}			
		}
		//get & set electro phys frequency measurement
		long freqAverageCnt = 0;
		if(pSetup->GetFrequencyProbe(_freqIntervalSec, freqAverageCnt, _freqCounterLine, _freqMeasureLine) && 0 < _freqCounterLine.size() && 0 < _freqMeasureLine.size())
		{
			//find out all NI boards' info
			boardInfoNI.get()->getInstance()->GetAllBoardsInfo();
			BoardInfo* bInfo1 = boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_freqCounterLine));
			BoardInfo* bInfo2 = boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_freqMeasureLine));
			if (NULL != bInfo1 && NULL != bInfo2)
			{
				if (0 != bInfo1->devName.compare(bInfo2->devName) && 0 == bInfo1->rtsiConfigure)
				{
					//do nothing if rtsi is not configured while using separate card to measure frequency
				}
				else
				{
					//prepare buffers for frequency measure task
					_freqBufSize = static_cast<unsigned int>(DEFAULT_DO_SAMPLE_RATE * std::max(0.01,_freqIntervalSec));
					_freqMeasure = new double[_freqBufSize];

					_freqCirBuf.get()->AllocMem(sizeof(double) * std::max((long)1, freqAverageCnt));
					_freqCirBuf.get()->SetOverflow(TRUE);

				}
			}
		}

		if (NULL == _triggerStruct)
		{
			_triggerStruct = new EPhysTriggerStruct();
			_triggerStruct->startEdge = 1;
			_triggerStruct->outputType = _triggerStruct->configured = _triggerStruct->enable = _triggerStruct->mode = _triggerStruct->repeats = _triggerStruct->iterations = 0;
			_triggerStruct->durationMS = _triggerStruct->idleMS = _triggerStruct->minIdleMS = _triggerStruct->startIdleMS = 0.0;
			_triggerStruct->stepEdge[0] = 0;
			_triggerStruct->powerPercent[0] = -1;
			_triggerStruct->voltageRange[0] = 0.0;	//default voltage range
			_triggerStruct->voltageRange[1] = 5.0;
			_triggerStruct->responseType = PockelsResponseType::LINEAR_RESPONSE;
		}
		try
		{
			//find out all NI boards' info
			boardInfoNI.get()->getInstance()->GetAllBoardsInfo();
			//get digital trigger configuration,
			//need to reconstruct task when param changed
			double unitMS = 1.0;
			pSetup->GetGeneralSettings(_ringBufferSize, unitMS, _triggerStruct->voltageRange[0], _triggerStruct->voltageRange[1], _triggerStruct->responseType, _parkAnalogLineAtLastVoltage);
			if(pSetup->GetTriggerConfig(&_triggerConfig))
			{
				std::string compStr1, compStr2;
				BoardInfo* compInfo = NULL;

				for (int i = 0; i < static_cast<int>(_triggerConfig.size()); i++)
				{
					BoardInfo* bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_triggerConfig[i]));
					if (NULL != bInfo)
					{
						switch (i)
						{
						case (int)EPhysTriggerLines::DIGITAL_COUNTER:
							//unable to verify counter name, assume user has correct setting
							if(EPhysModeConfig::NONE_TRIG < _triggerConfig[i].length())
							{
								_triggerStruct->configured = EPhysModeConfig::MANUAL_TRIG;
							}
							break;
						case (int)EPhysTriggerLines::DIGITAL_OUTPUT:
							compStr1 = GetNIDeviceAttribute(bInfo->devName,DAQmx_Dev_Terminals);
							if(EPhysModeConfig::MANUAL_TRIG == _triggerStruct->configured)
							{
								//output configured, allow free run
								_triggerStruct->configured = ((0 < compStr1.size()) && (std::string::npos != compStr1.find(_triggerConfig[i]))) ? EPhysModeConfig::MANUAL_TRIG : EPhysModeConfig::NONE_TRIG;
							}
							break;
						case (int)EPhysTriggerLines::ANALOG_COUNTER:
							_analogCounterInternalOutput = "";
							//unable to verify counter name, assume user has correct setting
							if(EPhysModeConfig::NONE_TRIG < _triggerConfig[i].length())
							{
								_triggerStruct->configured = EPhysModeConfig::MANUAL_TRIG;
								_analogCounterInternalOutput = "/" + GetDevIDName(_triggerConfig[i]) + "/Ctr" + _triggerConfig[i].at(_triggerConfig[i].length()-1) + "InternalOutput";
							}
							break;
						case (int)EPhysTriggerLines::ANALOG_OUTPUT:
							compStr1 = GetNIDeviceAttribute(bInfo->devName,DAQmx_Dev_AO_PhysicalChans);
							if(EPhysModeConfig::MANUAL_TRIG == _triggerStruct->configured)
							{
								//output configured, allow free run; no starting "/" in ao lines
								_triggerConfig[i] = ('/' == _triggerConfig[i].at(0)) ? (_triggerConfig[i].substr(1, _triggerConfig[i].length()-1)) : (_triggerConfig[i]);
								_triggerStruct->configured = ((0 < compStr1.size()) && (std::string::npos != compStr1.find(_triggerConfig[i]))) ? EPhysModeConfig::MANUAL_TRIG : EPhysModeConfig::NONE_TRIG;
								_ringBufferUnit = static_cast<long>(unitMS * bInfo->aoClockRateHz / Constants::MS_TO_SEC);
								_triggerStruct->clockRateHz = bInfo->aoClockRateHz;
							}
							break;
						case (int)EPhysTriggerLines::EDGE_OUTPUT:
							compStr1 = GetNIDeviceAttribute(bInfo->devName,DAQmx_Dev_Terminals);
							if(EPhysModeConfig::MANUAL_TRIG == _triggerStruct->configured)
							{
								_triggerStruct->configured = ((0 < compStr1.size()) && (std::string::npos != compStr1.find(_triggerConfig[i]))) ? EPhysModeConfig::EDGE_MONITOR : EPhysModeConfig::MANUAL_TRIG;
							}
							break;
						case (int)EPhysTriggerLines::BUFFER_OUTPUT:
							compStr1 = GetNIDeviceAttribute(bInfo->devName,DAQmx_Dev_DI_Lines);
							if(EPhysModeConfig::EDGE_MONITOR == _triggerStruct->configured)
							{
								//no '/' begin for DI lines for search
								compStr2 = ('/' == _triggerConfig[i].at(0)) ? _triggerConfig[i].substr(1,_triggerConfig[i].length()-1) : _triggerConfig[i];

								//output configured, allow trigger
								_triggerStruct->configured = ((0 < compStr1.size()) && (std::string::npos != compStr1.find(compStr2))) ? EPhysModeConfig::EDGE_MONITOR : EPhysModeConfig::MANUAL_TRIG;	
							}
							break;
						case (int)EPhysTriggerLines::CUSTOM_INPUT:
							if(EPhysModeConfig::EDGE_MONITOR == _triggerStruct->configured)
							{
								compStr1 = GetNIDeviceAttribute(bInfo->devName,DAQmx_Dev_Terminals);
								_triggerStruct->configured = ((0 < compStr1.size()) && (std::string::npos != compStr1.find(_triggerConfig[i]))) ? EPhysModeConfig::CUSTOM_CONFIG : EPhysModeConfig::EDGE_MONITOR;
							}
							break;
						default:
							break;
						}

						//only allow different devices if RTSI configured
						if (NULL != compInfo)
						{
							if (0 != bInfo->devName.compare(compInfo->devName) && 0 == bInfo->rtsiConfigure) 
							{
								_triggerStruct->configured = EPhysModeConfig::NONE_TRIG;
								break;
							}
						}
						compInfo = bInfo;
					}
				}
				StringCbPrintfW(message,_MAX_PATH, L"");
			}
		}
		catch(...)
		{
			StringCbPrintfW(message,_MAX_PATH, L"ThorElectroPhys error on data structure configuration.");
			LogMessage(message, ERROR_EVENT);
		}
		//return device found when dll exist, 
		//but revoke tasks at preflight if invalid configuration
		_numDevices = deviceCount = 1;
	}
	return ret;
}

long ThorElectroPhys::SelectDevice(const long device)
{
	if((device < 0) || (device >= _numDevices))
	{
		return FALSE;
	}

	return TRUE;
}

long ThorElectroPhys::TeardownDevice()
{
	_parkAnalogLineAtLastVoltage = FALSE;
	CloseNITasks();
	CloseMeasureTasks();
	CloseTriggerTasks();
	return TRUE;
}

long ThorElectroPhys::GetParamInfo
	(
	const long	paramID,
	long		&paramType,
	long		&paramAvailable,
	long		&paramReadOnly,
	double		&paramMin,
	double		&paramMax,
	double		&paramDefault
	)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_EPHYS_MEASURE_RATE:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = DEFAULT_DO_SAMPLE_RATE;
			paramDefault = 0;
		}
		break;
	case PARAM_EPHYS_MEASURE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 0x1;
			paramDefault = 0;
		}
		break;
	case PARAM_EPHYS_MEASURE_CONFIGURE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 0x1;
			paramDefault = 0;
		}
		break;
	case PARAM_EPHYS_DIG_LINE_IN_1:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 0x1;
			paramDefault = 0;
		}
		break;
	case PARAM_EPHYS_DIG_LINE_OUT_1:	
	case PARAM_EPHYS_DIG_LINE_OUT_2:	
	case PARAM_EPHYS_DIG_LINE_OUT_3:
	case PARAM_EPHYS_DIG_LINE_OUT_4:
	case PARAM_EPHYS_DIG_LINE_OUT_5:
	case PARAM_EPHYS_DIG_LINE_OUT_6:
	case PARAM_EPHYS_DIG_LINE_OUT_7:	
	case PARAM_EPHYS_DIG_LINE_OUT_8:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 0x1;
			paramDefault = 0;
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = EPHYS;
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = CONNECTION_READY;
			paramMax = paramDefault = CONNECTION_UNAVAILABLE;
		}
		break;
	case PARAM_EPHYS_TRIG_BUFFER:
		{
			paramType = TYPE_BUFFER;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
		}
		break;
	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long ThorElectroPhys::SetParam(const long paramID, const double param)
{
	long ret = TRUE;
	int32 error = 0;

	switch(paramID)
	{
	case PARAM_EPHYS_DIG_LINE_OUT_1:
	case PARAM_EPHYS_DIG_LINE_OUT_2:
	case PARAM_EPHYS_DIG_LINE_OUT_3:
	case PARAM_EPHYS_DIG_LINE_OUT_4:
	case PARAM_EPHYS_DIG_LINE_OUT_5:
	case PARAM_EPHYS_DIG_LINE_OUT_6:
	case PARAM_EPHYS_DIG_LINE_OUT_7:
	case PARAM_EPHYS_DIG_LINE_OUT_8:
		{			
			long index = paramID - PARAM_EPHYS_DIG_LINE_OUT_1;

			_digitalPortState[index] = static_cast<long>(param);			
		}
		break;
	case PARAM_EPHYS_MEASURE:
		try
		{
			if ((NULL != _taskTriggerCI) || (NULL != _freqThread))
			{
				//stop probe
				CloseMeasureTasks();
			}
			else
			{
				//setup frequency probe and create a thread to read since it is not a buffered task
				DAQmxErrChk (L"DAQmxCreateTask",error = DAQmxCreateTask("",&_taskTriggerCI));
				DAQmxErrChk (L"DAQmxCreateCIFreqChan",error = DAQmxCreateCIFreqChan(_taskTriggerCI,_freqCounterLine.c_str(),"",0.01,DEFAULT_DO_SAMPLE_RATE,DAQmx_Val_Hz,DAQmx_Val_Rising,DAQmx_Val_LowFreq1Ctr,_freqIntervalSec,1,NULL));
				DAQmxErrChk (L"DAQmxSetCIFreqTerm",error = DAQmxSetCIFreqTerm(_taskTriggerCI, "", _freqMeasureLine.c_str()));
				DAQmxErrChk (L"DAQmxCfgImplicitTiming",error = DAQmxCfgImplicitTiming(_taskTriggerCI,DAQmx_Val_ContSamps,DEFAULT_DO_SAMPLE_RATE));
				DAQmxErrChk (L"DAQmxCfgInputBuffer",error = DAQmxCfgInputBuffer(_taskTriggerCI, 4 * _freqBufSize));
				DAQmxErrChk (L"DAQmxStartTask",error = DAQmxStartTask(_taskTriggerCI));
				ResetEvent(_freqThreadStopped);
				_freqThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &(ThorElectroPhys::FreqMeasureAsync), NULL, 0, NULL);
			}
		}
		catch(...)
		{
			StringCbPrintfW(message,_MAX_PATH, L"ThorElectroPhys unable to start or stop frequency measurement.");
			LogMessage(message, ERROR_EVENT);
			CloseMeasureTasks();
		}
		break;
	default:
		ret = FALSE;
	}
	StringCbPrintfW(message,_MAX_PATH, L"");
	return ret;
}

long ThorElectroPhys::GetParam(const long paramID, double &param)
{
	long ret = TRUE;
	int32 error = 0;
	uInt32 samples[1] = {0};
	int32 samplesRead = 0;
	int32 bytesPerSamp = 0;
	uInt8 readVal[1] = {0};
	long count = 0;
	double dVal = 0, dAvg = 0;
	param = static_cast<double>(samples[0]);
	try
	{
		switch(paramID)
		{
		case PARAM_EPHYS_MEASURE:
			samples[0] = (NULL == _taskTriggerCI) ? 0x0 : 0x1;
			break;
		case PARAM_EPHYS_MEASURE_RATE:
			if (_freqCirBuf.get()->IsFull())
			{
				_freqCirBuf.get()->Lock();
				while (_freqCirBuf.get()->Read((char*)&dVal, sizeof(double)))
				{
					dAvg += dVal;
					count++;
				}
				_freqAveraged = (0 < count) ? (dAvg / count) : dAvg;
				_freqCirBuf.get()->ReleaseLock();
			}
			param = _freqAveraged;
			break;
		case PARAM_EPHYS_MEASURE_CONFIGURE:
			samples[0] = (NULL == _freqMeasure) ? 0x0 : 0x1;
			break;
		case PARAM_EPHYS_DIG_LINE_IN_1:
			{
				if(NULL != _taskHandleDI0)
				{
					DAQmxErrChk(L"DAQmxReadDigitalLines",error = DAQmxReadDigitalLines(_taskHandleDI0,1,0.0,DAQmx_Val_GroupByChannel,readVal,1,&samplesRead,&bytesPerSamp,NULL));
					samples[0] = readVal[0];
#ifdef LOGGING_ENABLED
					StringCbPrintfW(messageLog,_MAX_PATH, L"ThorElectroPhys Line read result %d",samples[0]);
					LogMessage(messageLog, INFORMATION_EVENT);
#endif
				}
			}
			break;
		case PARAM_EPHYS_DIG_LINE_OUT_1:
		case PARAM_EPHYS_DIG_LINE_OUT_2:
		case PARAM_EPHYS_DIG_LINE_OUT_3:
		case PARAM_EPHYS_DIG_LINE_OUT_4:
		case PARAM_EPHYS_DIG_LINE_OUT_5:
		case PARAM_EPHYS_DIG_LINE_OUT_6:
		case PARAM_EPHYS_DIG_LINE_OUT_7:
		case PARAM_EPHYS_DIG_LINE_OUT_8:
			{	
				long index = paramID - PARAM_EPHYS_DIG_LINE_OUT_1;
				samples[0] = _digitalPortState[index];
			}
			break;
		case PARAM_DEVICE_TYPE:
			{
				samples[0] = EPHYS;
			}
			break;
		case PARAM_CONNECTION_STATUS:
			{
				samples[0] = (1 ==_numDevices) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
			}
			break;
		default:
			ret = FALSE;
		}

		//assign param if not double data type
		if (PARAM_EPHYS_MEASURE_RATE != paramID)
			param = static_cast<double>(samples[0]);
	}
	catch(...)
	{
		if (PARAM_EPHYS_MEASURE_RATE == paramID)
			_freqCirBuf.get()->ReleaseLock();

		StringCbPrintfW(messageLog,_MAX_PATH, L"ThorElectroPhys unable to get param (%d) error (%d).", paramID, error);
		LogMessage(messageLog, ERROR_EVENT);
		ret = FALSE;
	}
	return ret;
}

/// <summary>
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorElectroPhys::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_EPHYS_TRIG_BUFFER:
		if (NULL == _triggerStruct)
		{
			_triggerStruct = new EPhysTriggerStruct();
		}
		memcpy_s(_triggerStruct, size, pBuffer, size);
		SetTriggerTasks();
		break;
	default:
		ret = FALSE;
	}
	return ret;
}

/// <summary>
/// Gets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorElectroPhys::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_EPHYS_TRIG_BUFFER:
		if((NULL == _triggerStruct) || (size < sizeof(EPhysTriggerStruct)))
			return FALSE;

		memcpy_s(pBuffer, sizeof(EPhysTriggerStruct), _triggerStruct, sizeof(EPhysTriggerStruct));
		break;
	default:
		ret = FALSE;
	}
	return ret;
}

/// <summary>
/// Sets the parameter string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <returns>long.</returns>
long ThorElectroPhys::SetParamString(const long paramID, wchar_t* str)
{
	return FALSE;
}

/// <summary>
/// Gets the parameter of type string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorElectroPhys::GetParamString(const long paramID, wchar_t* str, long size)
{
	return FALSE;
}

long ThorElectroPhys::PreflightPosition()
{
	//task is configured at find device, 
	//return false if invalid configuration
	if (NULL == _taskHandleDI0)
	{
		return FALSE;
	}
	else
	{
		//check if able to read
		double tVal = 0;
		return GetParam(PARAM_EPHYS_DIG_LINE_IN_1, tVal);
	}
}

long ThorElectroPhys::SetupPosition()
{
	return TRUE;
}

long ThorElectroPhys::StartPosition()
{
	uInt8 sample;
	int32 written = 0;
	int32 error = 0;
	try
	{
		for(long i=0; i<MAX_DIG_PORT_OUTPUT; i++)
		{
			if((NULL !=_taskHandleDO[i]) && (true == _digitalPortOutputAvailable[i]))
			{
				sample = static_cast<uInt8>(_digitalPortState[i]);
				DAQmxErrChk(L"DAQmxWriteDigitalLines",error = DAQmxWriteDigitalLines(_taskHandleDO[i],1,TRUE,0,DAQmx_Val_GroupByChannel,&sample,&written,NULL));
			}
		}
	}
	catch(...)
	{
		StringCbPrintfW(message,_MAX_PATH, L"ThorElectroPhys unable to start due to error (%d).", error);
		LogMessage(message, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long ThorElectroPhys::StatusPosition(long &status)
{
	status = IDevice::STATUS_READY;
	bool32 isDone = 0;
	int32 error = 0;
	try
	{
		if (NULL != _triggerStruct)
		{
			if (NULL != _taskTriggerHandle[TASKDO])
			{
				//check DO task for HW trigger edge mode, CO is retriggerable finite,
				//no NI error (underflow -200290) is allowed.
				DAQmxErrChk(L"DAQmxIsTaskDone", error = DAQmxIsTaskDone(_taskTriggerHandle[TASKDO], &isDone));
				status = isDone;
			}
			//continue check CO or AO task until done
			if (IDevice::STATUS_READY == status)
			{
				if ((NULL != _taskTriggerCO[TASKDO]) && ((int)EPhysOutputType::DIGITAL_ONLY == _triggerStruct->outputType || (int)EPhysOutputType::BOTH == _triggerStruct->outputType))
				{
					DAQmxErrChk(L"DAQmxIsTaskDone", error = DAQmxIsTaskDone(_taskTriggerCO[TASKDO], &isDone));
					status &= isDone;
				}
				if ((NULL != _taskTriggerCO[TASKAO]) && ((int)EPhysOutputType::ANALOG_ONLY == _triggerStruct->outputType || (int)EPhysOutputType::BOTH == _triggerStruct->outputType))
				{
					DAQmxErrChk(L"DAQmxIsTaskDone", error = DAQmxIsTaskDone(_taskTriggerCO[TASKAO], &isDone));
					status &= isDone;
				}
				if ((EPhysTriggerMode::MANUAL == (EPhysTriggerMode)_triggerStruct->mode) ||
					((1 == _triggerStruct->startEdge) && (0 == _triggerStruct->stepEdge[0]) && (0 > _triggerStruct->stepEdge[1])) && (0 == _triggerStruct->repeats))
				{
					//manual or special case [1,0,0] should keep CO task status
				}
				else if ((0 != _triggerStruct->iterations) && (_outputCount >= _targetCount))
				{
					status = IDevice::STATUS_READY;
				}
			}
		}
	}catch(...)
	{
		CloseTriggerTasks();
		//no update this error in GUI
		StringCbPrintfW(message,_MAX_PATH, L"");
	}
	return TRUE;
}

long ThorElectroPhys::ReadPosition(DeviceType deviceType, double &pos)
{
	return FALSE;
}

long ThorElectroPhys::PostflightPosition()
{
	return TRUE;
}

long ThorElectroPhys::GetLastErrorMsg(wchar_t * msg, long size)
{	
	wcsncpy_s(msg,size,message,_MAX_PATH);
	return TRUE;
}

int ThorElectroPhys::CloseNITasks()
{
	TerminateTask(_taskHandleDI0);

	for(long i=0; i<MAX_DIG_PORT_OUTPUT; i++)
	{		
		TerminateTask(_taskHandleDO[i]);
	}
	return TRUE;
}

void ThorElectroPhys::CloseMeasureTasks()
{
	//wait until frequency probe thread is terminated
	TerminateTask(_taskTriggerCI);
	WaitForSingleObject(_freqThreadStopped, Constants::EVENT_WAIT_TIME);
	SAFE_DELETE_HANDLE(_freqThread);
}

void ThorElectroPhys::CloseTriggerTasks(long bringDownLines)
{
	TerminateTask(_taskTriggerHandle[TASKDO]);	//[0:DO,1:AO]
	TerminateTask(_taskTriggerHandle[TASKAO]);
	TerminateTask(_taskTriggerCO[TASKDO]);
	TerminateTask(_taskTriggerCO[TASKAO]);

	//return w/o bring lines down
	if(FALSE == bringDownLines)
		return;

	if(NULL != _triggerStruct)
	{
		_triggerStruct->enable = FALSE;
	}
	//need to bring lines to low, consider outputMode
	if(NULL != _triggerStruct)
	{
		if ((int)EPhysOutputType::DIGITAL_ONLY == _triggerStruct->outputType || (int)EPhysOutputType::BOTH == _triggerStruct->outputType)
		{
			TogglePulseToDigitalLine(_taskTriggerHandle[TASKDO], _triggerConfig[EPhysTriggerLines::DIGITAL_OUTPUT], 1, TogglePulseMode::ToggleLow);
		}
		if ((int)EPhysOutputType::ANALOG_ONLY == _triggerStruct->outputType || (int)EPhysOutputType::BOTH == _triggerStruct->outputType)
		{
			if (TRUE == ThorElectroPhys::getInstance()->_parkAnalogLineAtLastVoltage && _triggerStruct->powerPercent[0] > 0 && ThorElectroPhys::getInstance()->_analogTriggerSet)
			{
				double intermediateValue = 0;
				switch (_triggerStruct->responseType)
				{
				case SINE_RESPONSE:
					intermediateValue = acos(1 - static_cast<double>(Constants::AREA_UNDER_CURVE) * _triggerStruct->powerPercent[0] / (double)Constants::HUNDRED_PERCENT) / PI;
					break;
				case LINEAR_RESPONSE:
				default:
					intermediateValue = _triggerStruct->powerPercent[0] / (double)Constants::HUNDRED_PERCENT;
					break;
				}

				double powerVal = intermediateValue * (_triggerStruct->voltageRange[1] - _triggerStruct->voltageRange[0]) + _triggerStruct->voltageRange[0];

				powerVal = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, powerVal));

				SetAnalogVoltage(_taskTriggerHandle[TASKAO], _triggerConfig[EPhysTriggerLines::ANALOG_OUTPUT], 1, &powerVal);
			}
			else if (!ThorElectroPhys::getInstance()->_analogTriggerSet && FALSE == ThorElectroPhys::getInstance()->_parkAnalogLineAtLastVoltage)
			{
				SetAnalogVoltage(_taskTriggerHandle[TASKAO], _triggerConfig[EPhysTriggerLines::ANALOG_OUTPUT], 1, &_triggerStruct->voltageRange[0]);
			}
			ThorElectroPhys::getInstance()->_analogTriggerSet = false;
		}
		if(EPhysModeConfig::EDGE_MONITOR == _triggerStruct->configured)
		{
			TogglePulseToDigitalLine(_taskTriggerHandle[0], _triggerConfig[EPhysTriggerLines::EDGE_OUTPUT], 1, TogglePulseMode::ToggleLow);
		}
	}
}

void ThorElectroPhys::ResetParams()
{
	_devName = "";	//"/Dev3/";
	_digitalPort = "";	//"PFI6";

	for(long i=0; i<MAX_DIG_PORT_OUTPUT; i++)
	{
		_digitalPortOutput[i].clear();
		_digitalPortOutputAvailable[i] = false;
		_digitalPortState[i] = FALSE;
	}

	_freqAveraged = 0;
	_freqBufSize = 0;
	SAFE_DELETE_ARRAY(_freqMeasure);
	if (_freqCirBuf.get())
		_freqCirBuf.get()->ReleaseMem();

	SAFE_DELETE_ARRAY(_triggerStruct);
}

std::string ThorElectroPhys::GetTriggerInputLine()
{
	if (NULL != _triggerStruct)
		return (EPhysTriggerMode::CUSTOM == (EPhysTriggerMode)_triggerStruct->mode) ? _triggerConfig[EPhysTriggerLines::CUSTOM_INPUT] : WStringToString(std::wstring(_triggerStruct->triggerLine));

	return _triggerConfig[EPhysTriggerLines::CUSTOM_INPUT];
}

long ThorElectroPhys::SetDigitalTriggerTask()
{
	int32 error = DAQmxSuccess;
	long blockCount = 0;		//# of blocks in one ring buffer
	uInt32 multipleRatio = 4;	//multiple of buffer sizes set to on-board memory
	uInt32 bufferSize = 0;		//max possible total buffer count be written per callback
	uint64_t totalSize = 0;		//total length of units in the waveform

	if(EPhysModeConfig::MANUAL_TRIG <= _triggerStruct->configured && (EPhysOutputType::DIGITAL_ONLY == (EPhysOutputType)_triggerStruct->outputType || EPhysOutputType::BOTH == (EPhysOutputType)_triggerStruct->outputType))
	{
		///*************************************************************///
		///******	set counter output with specified durations   ******///
		///*************************************************************///

		//limit idle time by max duty cycle that card can accept
		const double MAX_DUTY_CYCLE = 0.985;
		_triggerStruct->minIdleMS = ((1-MAX_DUTY_CYCLE)/MAX_DUTY_CYCLE) * _triggerStruct->durationMS;
		_triggerStruct->idleMS = (_triggerStruct->minIdleMS > _triggerStruct->idleMS) ? _triggerStruct->minIdleMS : _triggerStruct->idleMS;
		float64 period = (_triggerStruct->durationMS + _triggerStruct->idleMS) / Constants::MS_TO_SEC;	//[sec]
		float64 freq = 1 / period;																		//[Hz]
		float64 dutyCycle = _triggerStruct->durationMS / (_triggerStruct->durationMS + _triggerStruct->idleMS);

		DAQmxErrChk (L"DAQmxCreateTask", error = DAQmxCreateTask("",&_taskTriggerCO[TASKDO]));

		DAQmxErrChk(L"DAQmxCreateCOPulseChanFreq", error = DAQmxCreateCOPulseChanFreq(_taskTriggerCO[TASKDO], _triggerConfig[DIGITAL_COUNTER].c_str(), "", DAQmx_Val_Hz, DAQmx_Val_Low, _triggerStruct->startIdleMS / Constants::MS_TO_SEC, freq, dutyCycle));

		DAQmxErrChk(L"DAQmxSetChanAttribute", error = DAQmxSetChanAttribute (_taskTriggerCO[TASKDO], "", DAQmx_CO_Pulse_Term, _triggerConfig[DIGITAL_OUTPUT].c_str()));

		if (0 < _triggerStruct->iterations)
		{
			DAQmxErrChk(L"DAQmxCfgImplicitTiming", error = DAQmxCfgImplicitTiming (_taskTriggerCO[TASKDO], DAQmx_Val_FiniteSamps, _triggerStruct->iterations));
		}
		else
		{
			DAQmxErrChk(L"DAQmxCfgImplicitTiming", error = DAQmxCfgImplicitTiming (_taskTriggerCO[TASKDO], DAQmx_Val_ContSamps, DEFAULT_DO_SAMPLE_RATE));
		}

		if (EPhysTriggerMode::MANUAL == (EPhysTriggerMode)_triggerStruct->mode)
		{
			return TRUE;		//start later
		}

		//[special case]: retriggerable by every triggers if 1 start 0 gaps 0 repeats configured
		if ((0 == _triggerStruct->stepEdge[0]) && (0 > _triggerStruct->stepEdge[1]))
		{
			if((1 == _triggerStruct->startEdge) && (0 == _triggerStruct->repeats))
			{
				return TRUE;	//start later
			}
		}

		if (EPhysTriggerMode::CUSTOM == (EPhysTriggerMode)_triggerStruct->mode && EPhysModeConfig::CUSTOM_CONFIG != _triggerStruct->configured)
		{
			StringCbPrintfW(message,_MAX_PATH, L"ThorElectroPhys not configured for custom input line.");
			LogMessage(message, ERROR_EVENT);
			goto TERMINATE_TRIGGER;
		}
		DAQmxErrChk(L"DAQmxSetDigEdgeArmStartTrigSrc", error = DAQmxSetDigEdgeArmStartTrigSrc(_taskTriggerCO[TASKDO],GetTriggerInputLine().c_str()));
		DAQmxErrChk(L"DAQmxSetArmStartTrigType", error = DAQmxSetArmStartTrigType(_taskTriggerCO[TASKDO],DAQmx_Val_DigEdge));
		DAQmxErrChk(L"DAQmxSetDigEdgeArmStartTrigEdge", error = DAQmxSetDigEdgeArmStartTrigEdge(_taskTriggerCO[TASKDO],DAQmx_Val_Rising));

		DAQmxErrChk(L"DAQmxCfgDigEdgeStartTrig", error = DAQmxCfgDigEdgeStartTrig(_taskTriggerCO[TASKDO],_triggerConfig[EPhysTriggerLines::EDGE_OUTPUT].c_str(),DAQmx_Val_Rising));
		DAQmxErrChk(L"DAQmxSetStartTrigRetriggerable", error = DAQmxSetStartTrigRetriggerable(_taskTriggerCO[TASKDO],true));

		DAQmxErrChk(L"DAQmxRegisterSignalEvent", error = DAQmxRegisterSignalEvent(_taskTriggerCO[TASKDO],DAQmx_Val_CounterOutputEvent,0,TriggerCOCallback,NULL));
	}
	return TRUE;

TERMINATE_TRIGGER:
	TerminateTask(_taskTriggerHandle[TASKDO]);	//[0:DO,1:AO]
	TerminateTask(_taskTriggerCO[TASKDO]);
	return FALSE;
}

long ThorElectroPhys::SetAnalogTriggerTask()
{
	int32 error = DAQmxSuccess;
	long blockCount = 0;		//# of blocks in one ring buffer
	uInt32 multipleRatio = 4;	//multiple of buffer sizes set to on-board memory
	uInt32 bufferSize = 0;		//max possible total buffer count be written per callback
	uint64_t totalSize = 0;		//total length of units in the waveform

	if(EPhysModeConfig::MANUAL_TRIG <= _triggerStruct->configured && (EPhysOutputType::ANALOG_ONLY == (EPhysOutputType)_triggerStruct->outputType || EPhysOutputType::BOTH == (EPhysOutputType)_triggerStruct->outputType))
	{
		///*****************************************************************************************///
		///******	  	  configure timing with specified durations,            	  	      ******///
		///******	  	  build analog waveform and wait for it to be initialized	  	      ******///
		///*****************************************************************************************///
		_triggerStruct->minIdleMS = 0; // allow no idle in one period, instead of one clock time: (Constants::MS_TO_SEC / _triggerStruct->clockRateHz)
		_triggerStruct->idleMS = std::max(_triggerStruct->minIdleMS, _triggerStruct->idleMS);

		waveformAOBuilder->SetBuilderType(BuilderType::EPHYS_AO);
		waveformAOBuilder->SetWaveformParams(_triggerStruct);
		if(FALSE == waveformAOBuilder->TryBuildWaveform(totalSize))
		{
			StringCbPrintfW(message,_MAX_PATH, L"Invalid settings to build AO waveform.");
			LogMessage(message, ERROR_EVENT);
			goto TERMINATE_TRIGGER;
		}

		blockCount = std::min(std::max((long)Constants::ACTIVE_LOAD_BLKSIZE_DEFAULT, _ringBufferSize), static_cast<long>(ceil((double)totalSize / (double)_ringBufferUnit)));
		bufferSize = static_cast<uInt32>(blockCount * _ringBufferUnit);
		_bRingBuf[TASKAO].reset(new BlockRingBuffer(SignalType::ANALOG_POCKEL, sizeof(double), blockCount, _ringBufferUnit));
		waveformAOBuilder->ConnectCallback(_bRingBuf[TASKAO].get());
		_bRingBuf[TASKAO].get()->CheckWritableBlockCounts(TRUE);

		if(WAIT_OBJECT_0 != WaitForSingleObject(waveformAOBuilder->GetSignalHandle(), Constants::EVENT_WAIT_TIME))
		{
			StringCbPrintfW(message,_MAX_PATH, L"Failed to build AO waveform in time.");
			LogMessage(message, ERROR_EVENT);
			goto TERMINATE_TRIGGER;
		}

		///***************************************************************************************************************///
		///******	set counter output for analog output task clock timing,											******///
		///******	leave clock finite retriggerable and set finite allow-regen analog output in edge monitor mode	******///
		///***************************************************************************************************************///
		DAQmxErrChk(L"DAQmxCreateTask", error = DAQmxCreateTask("", &_taskTriggerCO[TASKAO]));

		DAQmxErrChk(L"DAQmxCreateCOPulseChanFreq", error = DAQmxCreateCOPulseChanFreq(_taskTriggerCO[TASKAO], _triggerConfig[ANALOG_COUNTER].c_str(), "", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, _triggerStruct->clockRateHz, 0.5));

		DAQmxErrChk(L"DAQmxCfgImplicitTiming", error = DAQmxCfgImplicitTiming (_taskTriggerCO[TASKAO], (0 < _triggerStruct->iterations) ? DAQmx_Val_FiniteSamps : DAQmx_Val_ContSamps, totalSize));

		///****************************************		configure Task AO		****************************************///
		DAQmxErrChk(L"DAQmxCreateTask",error = DAQmxCreateTask("", &_taskTriggerHandle[TASKAO]));
		DAQmxErrChk(L"DAQmxCreateAOVoltageChan",error = DAQmxCreateAOVoltageChan(_taskTriggerHandle[TASKAO], _triggerConfig[ANALOG_OUTPUT].c_str(), "", MIN_AO_VOLTAGE, MAX_AO_VOLTAGE, DAQmx_Val_Volts, NULL));

		int32 dataXferType = DAQmx_Val_DMA;
		BoardInfo* bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_triggerConfig[ANALOG_OUTPUT]));
		if (NULL != bInfo)
		{
			dataXferType = (BoardStyle::USB == bInfo->boardStyle) ? DAQmx_Val_USBbulk : DAQmx_Val_DMA;
			DAQmxErrChk (L"DAQmxSetAODataXferMech", error = DAQmxSetAODataXferMech(_taskTriggerHandle[TASKAO],"",dataXferType));	
		}

		DAQmxErrChk (L"DAQmxCfgOutputBuffer", error = DAQmxCfgOutputBuffer(_taskTriggerHandle[TASKAO], multipleRatio * bufferSize));

		DAQmxErrChk (L"DAQmxSetWriteAttribute",error = DAQmxSetWriteAttribute (_taskTriggerHandle[TASKAO], DAQmx_Write_RegenMode, DAQmx_Val_DoNotAllowRegen));

		if (bufferSize < totalSize || 0 >= _triggerStruct->iterations)
		{
			DAQmxErrChk (L"DAQmxCfgSampClkTiming", error = DAQmxCfgSampClkTiming(_taskTriggerHandle[TASKAO], _analogCounterInternalOutput.c_str(), static_cast<float64>(_triggerStruct->clockRateHz), DAQmx_Val_Rising, (0 < _triggerStruct->iterations) ? DAQmx_Val_FiniteSamps : DAQmx_Val_ContSamps, totalSize));

			DAQmxErrChk(L"DAQmxSetChanAttribute", error = DAQmxSetChanAttribute(_taskTriggerHandle[TASKAO], "", DAQmx_AO_DataXferReqCond, DAQmx_Val_OnBrdMemNotFull));
		}
		else
		{
			DAQmxErrChk(L"DAQmxCfgSampClkTiming",error = DAQmxCfgSampClkTiming(_taskTriggerHandle[TASKAO], _analogCounterInternalOutput.c_str(), static_cast<float64>(_triggerStruct->clockRateHz), DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, totalSize));
		}

		if (FALSE == FillupAvailableBuffer(TASKAO))
			goto TERMINATE_TRIGGER;

		//error if register event before write 
		if (bufferSize < totalSize || 0 >= _triggerStruct->iterations)
		{					
			DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent",error = DAQmxRegisterEveryNSamplesEvent(_taskTriggerHandle[TASKAO], DAQmx_Val_Transferred_From_Buffer, bufferSize, 0, EveryNTriggerCallback, (void*)TASKAO));
		}

		_analogTriggerSet = true;

		if (EPhysTriggerMode::MANUAL == (EPhysTriggerMode)_triggerStruct->mode)
		{
			return TRUE;		//start later
		}

		//[special case]: retriggerable by every triggers if 1 start 0 gaps 0 repeats configured
		if ((0 == _triggerStruct->stepEdge[0]) && (0 > _triggerStruct->stepEdge[1]))
		{
			if((1 == _triggerStruct->startEdge) && (0 == _triggerStruct->repeats))
			{
				return TRUE;	//start later
			}
		}

		if (EPhysTriggerMode::CUSTOM == (EPhysTriggerMode)_triggerStruct->mode && EPhysModeConfig::CUSTOM_CONFIG != _triggerStruct->configured)
		{
			StringCbPrintfW(message,_MAX_PATH, L"ThorElectroPhys not configured for custom input line.");
			LogMessage(message, ERROR_EVENT);
			goto TERMINATE_TRIGGER;
		}

		DAQmxErrChk(L"DAQmxSetDigEdgeArmStartTrigSrc", error = DAQmxSetDigEdgeArmStartTrigSrc(_taskTriggerCO[TASKAO],GetTriggerInputLine().c_str()));

		DAQmxErrChk(L"DAQmxSetArmStartTrigType", error = DAQmxSetArmStartTrigType(_taskTriggerCO[TASKAO],DAQmx_Val_DigEdge));
		DAQmxErrChk(L"DAQmxSetDigEdgeArmStartTrigEdge", error = DAQmxSetDigEdgeArmStartTrigEdge(_taskTriggerCO[TASKAO],DAQmx_Val_Rising));

		DAQmxErrChk(L"DAQmxCfgDigEdgeStartTrig", error = DAQmxCfgDigEdgeStartTrig(_taskTriggerCO[TASKAO],_triggerConfig[EPhysTriggerLines::EDGE_OUTPUT].c_str(),DAQmx_Val_Rising));
		DAQmxErrChk(L"DAQmxSetStartTrigRetriggerable", error = DAQmxSetStartTrigRetriggerable(_taskTriggerCO[TASKAO],true));

		//only need one task registered
		if (EPhysOutputType::ANALOG_ONLY == (EPhysOutputType)_triggerStruct->outputType)
			DAQmxErrChk(L"DAQmxRegisterSignalEvent", error = DAQmxRegisterSignalEvent(_taskTriggerCO[TASKAO],DAQmx_Val_CounterOutputEvent,0,TriggerCOCallback,NULL));

		//AO cannot be armStarted
		DAQmxErrChk(L"DAQmxCfgDigEdgeStartTrig", error = DAQmxCfgDigEdgeStartTrig(_taskTriggerHandle[TASKAO],_triggerConfig[EPhysTriggerLines::EDGE_OUTPUT].c_str(),DAQmx_Val_Rising));
		DAQmxErrChk(L"DAQmxSetStartTrigRetriggerable", error = DAQmxSetStartTrigRetriggerable(_taskTriggerHandle[TASKAO],true));
	}
	return TRUE;

TERMINATE_TRIGGER:
	TerminateTask(_taskTriggerHandle[TASKAO]);
	TerminateTask(_taskTriggerCO[TASKAO]);
	return FALSE;
}

long ThorElectroPhys::SetEdgeMonitorTask()
{
	int32 error = DAQmxSuccess;	
	long blockCount = std::max((long)Constants::ACTIVE_LOAD_BLKSIZE_DEFAULT, _ringBufferSize);//# of blocks in one ring buffer	
	uInt32 multipleRatio = 4;	//multiple of buffer sizes set to on-board memory	
	uInt32 bufferSize = static_cast<uInt32>(blockCount * Constants::ACTIVE_LOAD_UNIT_SIZE);	//max possible total buffer count be written per callback
	uint64_t totalSize = 0;		//total length of units in the waveform

	if (EPhysModeConfig::EDGE_MONITOR <= _triggerStruct->configured)
	{
		///*********************************************************************************************///
		///******	  	  EDGE MONITOR: build waveform and wait for it to be initialized	      ******///
		///*********************************************************************************************///
		_bRingBuf[TASKDO].reset(new BlockRingBuffer(SignalType::DIGITAL_LINES, sizeof(unsigned char), blockCount, Constants::ACTIVE_LOAD_UNIT_SIZE));
		waveformDOBuilder->SetBuilderType(BuilderType::EPHYS_DO);
		waveformDOBuilder->SetWaveformParams(_triggerStruct);
		waveformDOBuilder->TryBuildWaveform(totalSize);
		waveformDOBuilder->ConnectCallback(_bRingBuf[TASKDO].get());
		_bRingBuf[TASKDO].get()->CheckWritableBlockCounts(TRUE);

		if(WAIT_OBJECT_0 != WaitForSingleObject(waveformDOBuilder->GetSignalHandle(), Constants::EVENT_WAIT_TIME))
		{
			StringCbPrintfW(message,_MAX_PATH, L"Failed to build waveform in time.");
			LogMessage(message, ERROR_EVENT);
			goto TERMINATE_TRIGGER;
		}

		///*************************************************************///
		///******	  	  configure trigger edges by DO	  	      ******///
		///*************************************************************///
		DAQmxErrChk (L"DAQmxCreateTask", error = DAQmxCreateTask("",&_taskTriggerHandle[TASKDO]));

		DAQmxErrChk (L"DAQmxCreateDOChan", error = DAQmxCreateDOChan(_taskTriggerHandle[TASKDO],_triggerConfig[BUFFER_OUTPUT].c_str(),"",DAQmx_Val_ChanPerLine));

		int32 dataXferType = DAQmx_Val_DMA;
		BoardInfo* bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_triggerConfig[BUFFER_OUTPUT]));
		if (NULL != bInfo)
			dataXferType = (BoardStyle::USB == bInfo->boardStyle) ? DAQmx_Val_USBbulk : DAQmx_Val_DMA;
		DAQmxErrChk (L"DAQmxSetDODataXferMech", error = DAQmxSetDODataXferMech(_taskTriggerHandle[TASKDO],"",dataXferType));	

		DAQmxErrChk (L"DAQmxCfgOutputBuffer", error = DAQmxCfgOutputBuffer(_taskTriggerHandle[TASKDO], multipleRatio * bufferSize));

		DAQmxErrChk (L"DAQmxSetWriteAttribute", error = DAQmxSetWriteAttribute (_taskTriggerHandle[TASKDO], DAQmx_Write_RegenMode, DAQmx_Val_DoNotAllowRegen));

		if (bufferSize < totalSize)
		{
			DAQmxErrChk (L"DAQmxCfgSampClkTiming", error = DAQmxCfgSampClkTiming(_taskTriggerHandle[TASKDO], GetTriggerInputLine().c_str(), DEFAULT_DO_SAMPLE_RATE, DAQmx_Val_Rising, DAQmx_Val_ContSamps, bufferSize));

			DAQmxErrChk(L"DAQmxSetChanAttribute", error = DAQmxSetChanAttribute(_taskTriggerHandle[TASKDO], "", DAQmx_DO_DataXferReqCond, DAQmx_Val_OnBrdMemNotFull));
		}
		else
		{
			DAQmxErrChk(L"DAQmxCfgSampClkTiming", error = DAQmxCfgSampClkTiming(_taskTriggerHandle[TASKDO], GetTriggerInputLine().c_str(), DEFAULT_DO_SAMPLE_RATE, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, totalSize));
		}

		if (FALSE == FillupAvailableBuffer(TASKDO))
			goto TERMINATE_TRIGGER;

		//error:[-200848] if register event before DAQmxWriteDigitalLines 
		if (bufferSize < totalSize)
		{
			DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent", error = DAQmxRegisterEveryNSamplesEvent(_taskTriggerHandle[TASKDO], DAQmx_Val_Transferred_From_Buffer, bufferSize, 0, EveryNTriggerCallback, (void*)TASKDO));
		}
	}
	return TRUE;

TERMINATE_TRIGGER:
	TerminateTask(_taskTriggerHandle[TASKDO]);
	return FALSE;
}

long ThorElectroPhys::StartTriggerTasks()
{
	int32 error = DAQmxSuccess;
	if (EPhysTriggerMode::MANUAL == (EPhysTriggerMode)_triggerStruct->mode)
	{
		goto START_TRIGGERS;
	}

	//[special case]: retriggerable by every triggers if 1 start 0 gaps 0 repeats configured
	if ((0 == _triggerStruct->stepEdge[0]) && (0 > _triggerStruct->stepEdge[1]))
	{
		if((1 == _triggerStruct->startEdge) && (0 == _triggerStruct->repeats))
		{
			//[special case]: retriggerable by every triggers if 1 start 0 gaps 0 repeats configured
			if(NULL != _taskTriggerCO[TASKDO] && (EPhysOutputType::DIGITAL_ONLY == (EPhysOutputType)_triggerStruct->outputType || EPhysOutputType::BOTH == (EPhysOutputType)_triggerStruct->outputType))
			{
				DAQmxErrChk(L"DAQmxCfgDigEdgeStartTrig", error = DAQmxCfgDigEdgeStartTrig(_taskTriggerCO[TASKDO],GetTriggerInputLine().c_str(),DAQmx_Val_Rising));
				DAQmxErrChk(L"DAQmxSetStartTrigRetriggerable", error = DAQmxSetStartTrigRetriggerable(_taskTriggerCO[TASKDO],true));

				DAQmxErrChk(L"DAQmxTaskControl",error = DAQmxTaskControl(_taskTriggerCO[TASKDO],DAQmx_Val_Task_Reserve));
				DAQmxErrChk(L"DAQmxStartTask", error = DAQmxStartTask(_taskTriggerCO[TASKDO]));
			}
			if(NULL != _taskTriggerCO[TASKAO] && NULL != _taskTriggerHandle[TASKAO] && (EPhysOutputType::ANALOG_ONLY == (EPhysOutputType)_triggerStruct->outputType || EPhysOutputType::BOTH == (EPhysOutputType)_triggerStruct->outputType))
			{
				DAQmxErrChk(L"DAQmxCfgDigEdgeStartTrig", error = DAQmxCfgDigEdgeStartTrig(_taskTriggerCO[TASKAO],GetTriggerInputLine().c_str(),DAQmx_Val_Rising));
				DAQmxErrChk(L"DAQmxSetStartTrigRetriggerable", error = DAQmxSetStartTrigRetriggerable(_taskTriggerCO[TASKAO],true));

				DAQmxErrChk(L"DAQmxCfgDigEdgeStartTrig", error = DAQmxCfgDigEdgeStartTrig(_taskTriggerHandle[TASKAO],GetTriggerInputLine().c_str(),DAQmx_Val_Rising));
				DAQmxErrChk(L"DAQmxSetStartTrigRetriggerable", error = DAQmxSetStartTrigRetriggerable(_taskTriggerHandle[TASKAO],true));

				DAQmxErrChk(L"DAQmxTaskControl", error = DAQmxTaskControl(_taskTriggerHandle[TASKAO],DAQmx_Val_Task_Reserve));
				DAQmxErrChk(L"DAQmxStartTask", error = DAQmxStartTask(_taskTriggerHandle[TASKAO]));

				DAQmxErrChk(L"DAQmxTaskControl",error = DAQmxTaskControl(_taskTriggerCO[TASKAO],DAQmx_Val_Task_Reserve));
				DAQmxErrChk(L"DAQmxStartTask", error = DAQmxStartTask(_taskTriggerCO[TASKAO]));	//actual start of AO
			}
			return TRUE;
		}
		else if (1 != _triggerStruct->repeats)
		{
			StringCbPrintfW(message,_MAX_PATH, L"Repeat can only be 1 when Gaps = 0.");
			LogMessage(message, ERROR_EVENT);
			goto TERMINATE_TRIGGERS;
		}
	}
	if (NULL != _taskTriggerHandle[TASKDO])
	{
		DAQmxErrChk(L"DAQmxTaskControl", error = DAQmxTaskControl(_taskTriggerHandle[TASKDO],DAQmx_Val_Task_Reserve));
		DAQmxErrChk(L"DAQmxStartTask", error = DAQmxStartTask(_taskTriggerHandle[TASKDO]));
		goto START_TRIGGERS;
	}
	return FALSE;

START_TRIGGERS:
	if(NULL != _taskTriggerCO[TASKDO] && (EPhysOutputType::DIGITAL_ONLY == (EPhysOutputType)_triggerStruct->outputType || EPhysOutputType::BOTH == (EPhysOutputType)_triggerStruct->outputType))
	{
		DAQmxErrChk(L"DAQmxTaskControl",error = DAQmxTaskControl(_taskTriggerCO[TASKDO],DAQmx_Val_Task_Reserve));
		DAQmxErrChk(L"DAQmxStartTask", error = DAQmxStartTask(_taskTriggerCO[TASKDO]));
	}
	if(NULL != _taskTriggerHandle[TASKAO] && (EPhysOutputType::ANALOG_ONLY == (EPhysOutputType)_triggerStruct->outputType || EPhysOutputType::BOTH == (EPhysOutputType)_triggerStruct->outputType))
	{
		DAQmxErrChk(L"DAQmxTaskControl",error = DAQmxTaskControl(_taskTriggerHandle[TASKAO],DAQmx_Val_Task_Reserve));
		DAQmxErrChk(L"DAQmxStartTask", error = DAQmxStartTask(_taskTriggerHandle[TASKAO]));

		DAQmxErrChk(L"DAQmxTaskControl",error = DAQmxTaskControl(_taskTriggerCO[TASKAO],DAQmx_Val_Task_Reserve));
		DAQmxErrChk(L"DAQmxStartTask", error = DAQmxStartTask(_taskTriggerCO[TASKAO]));	//actual start of AO
	}
	return TRUE;

TERMINATE_TRIGGERS:
	CloseTriggerTasks(FALSE);
	return FALSE;
}

long ThorElectroPhys::SetTriggerTasks()
{
	long ret = TRUE;
	int32 error = 0;
	long blockCount = 0;		//# of blocks in one ring buffer
	uInt32 multipleRatio = 4;	//multiple of buffer sizes set to on-board memory
	uInt32 bufferSize = 0;		//max possible total buffer count be written per callback
	uint64_t totalSize = 0;		//total length of units in the waveform

	CloseTriggerTasks(FALSE);

	if(NULL == _triggerStruct)
		return FALSE;

	try
	{
		//reset error message
		StringCbPrintfW(message,_MAX_PATH, L"");

		//return if mode none
		if(EPhysTriggerMode::NONE == (EPhysTriggerMode)_triggerStruct->mode)
			goto TERMINATE_TRIGGER;

		if((TRUE == _triggerStruct->enable) && EPhysModeConfig::NONE_TRIG < _triggerStruct->configured)
		{
			///***********************************************************************************************///
			///******	register & count counter output event to estimate end of retriggerable CO task,	******///
			///******	find out number of non-zero gaps												******///
			///***********************************************************************************************///
			_targetCount = _outputCount = 0;
			for (int i = 0; i < _MAX_PATH; i++)
			{
				if (0 > _triggerStruct->stepEdge[i])
					break;
				else if (0 < _triggerStruct->stepEdge[i])
					_targetCount++;
			}
			//determine # of rising edges based on non-zero gaps, then x 2 x iterations for # of CO event
			if (0 == _triggerStruct->stepEdge[0])
			{
				_targetCount = (0 == _triggerStruct->iterations) ? (-1) : (1 + _targetCount) * _triggerStruct->repeats * 2 * _triggerStruct->iterations;
			}
			else
			{
				_targetCount = (0 == _triggerStruct->iterations) ? (-1) : (1 + _targetCount * _triggerStruct->repeats) * 2 * _triggerStruct->iterations;
			}

			// set up all tasks and start at once
			ret &= SetDigitalTriggerTask();
			ret &= SetAnalogTriggerTask();
			ret &= SetEdgeMonitorTask();
			ret &= StartTriggerTasks();
		}
		else
			goto TERMINATE_TRIGGER;
	}
	catch(...)
	{
#ifdef _DEBUG
		DAQmxGetExtendedErrorInfo(errMsg,_MAX_PATH);
		LogMessage((wchar_t*)StringToWString(std::string(errMsg)).c_str(), ERROR_EVENT);
#endif
		CloseTriggerTasks();
		StringCbPrintfW(message,_MAX_PATH, L"ThorElectroPhys unable to set trigger task due to error (%d).", error);
		LogMessage(message, ERROR_EVENT);
		StringCbPrintfW(message,_MAX_PATH, L"Unable to generate signal!");
		return FALSE;
	}
	return ret;

TERMINATE_TRIGGER:
	CloseTriggerTasks();
	return FALSE;
}

//fill buffer up to current block count, don't allow refill until writing to NI
long ThorElectroPhys::FillupAvailableBuffer(int id)
{
	int32 error = 0;
	long blocksToRead = _bRingBuf[id]->GetReadableBlockCounts();
	long curBufferSizeInBytes = (0 < _bRingBuf[id]->GetUnitsInLastBlock()) ? _bRingBuf[id]->GetUnitSizeInBytes() * ((_bRingBuf[id]->GetBlockSize() * (blocksToRead-1)) + _bRingBuf[id]->GetUnitsInLastBlock()) : (_bRingBuf[id]->GetBlockSizeInByte() * blocksToRead);
	int32 curBufferSize = curBufferSizeInBytes / _bRingBuf[id]->GetUnitSizeInBytes();

	if (_localBufferSizeInBytes != curBufferSizeInBytes)
	{
		_localBuffer = (uInt8*)realloc((void*)_localBuffer, curBufferSizeInBytes);
		_localBufferSizeInBytes = curBufferSizeInBytes;
	}
	if((NULL == _localBuffer) || (0 >= curBufferSizeInBytes))
	{
		StringCbPrintfW(message,_MAX_PATH, L"Failed to allocate memory, please check settings.");
		LogMessage(message, ERROR_EVENT);
		return FALSE;
	}

	UCHAR* pDst = _localBuffer;
	for (long i = 0; i < blocksToRead; i++)
	{
		//in case last block is not full
		long unitsToReadinLastBlock = _bRingBuf[id]->GetUnitsInLastBlock();
		if ((blocksToRead-1 == i) && (0 < unitsToReadinLastBlock))
		{
			//partially available:
			long offset = 0;
			while (0 < _bRingBuf[id]->GetUnitsInLastBlock())
			{
				unitsToReadinLastBlock = _bRingBuf[id]->GetUnitsInLastBlock();
				if(FALSE == _bRingBuf[id]->ReadUnits((UCHAR*)pDst, offset, unitsToReadinLastBlock, FALSE))
				{
					StringCbPrintfW(message,_MAX_PATH, L"Failed to read ring buffer units in ThorElectroPhys.");
					LogMessage(message, ERROR_EVENT);
					return FALSE;
				}
				pDst += unitsToReadinLastBlock * _bRingBuf[id]->GetUnitSizeInBytes();
				offset += unitsToReadinLastBlock;
			}
		}
		else
		{
			if(FALSE == _bRingBuf[id]->ReadBlocks((UCHAR*)pDst, FALSE))
			{
				StringCbPrintfW(message,_MAX_PATH, L"Failed to read ring buffer blocks in ThorElectroPhys.");
				LogMessage(message, ERROR_EVENT);
				return FALSE;
			}
			pDst += _bRingBuf[id]->GetBlockSizeInByte();
		}
	}

	switch (id)
	{
	case TASKDO:
		//DAQmxErrChk (L"DAQmxWriteDigitalLines", error = DAQmxResetWriteNextWriteIsLast(_taskTriggerHandle[id]));
		DAQmxErrChk (L"DAQmxWriteDigitalLines", error = DAQmxWriteDigitalLines(_taskTriggerHandle[id],static_cast<int32>(curBufferSizeInBytes),false,-1,DAQmx_Val_GroupByScanNumber,_localBuffer,NULL,NULL));
		break;
	case TASKAO:
		DAQmxErrChk(L"DAQmxWriteAnalogF64", error = DAQmxWriteAnalogF64(_taskTriggerHandle[id], curBufferSize, false, -1, DAQmx_Val_GroupByScanNumber, (float64*)_localBuffer, NULL, NULL));
		break;
	default:
		break;
	}

	//start fill ring buffer after writing to NI
	_bRingBuf[id]->CheckWritableBlockCounts(TRUE);

	return TRUE;
}

