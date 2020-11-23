//BoardInfoNI.cpp: Defines BoardInfoNI functions.
//

#include "BoardInfoNI.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"

///Initialize static members
bool BoardInfoNI::_instanceFlag = false;
std::unique_ptr<BoardInfoNI> BoardInfoNI::_single(new BoardInfoNI());

BoardInfoNI* BoardInfoNI::getInstance()
{ 
	if (!_instanceFlag) 
	{ 
		_single.reset(new BoardInfoNI());	
		_instanceFlag = true;
	}	
	return _single.get();
}

void BoardInfoNI::GetAllBoardsInfo()
{
	long ret = TRUE;
	int buffersize;
	char* devicenames = NULL;
	char* nxdevicenames = NULL;
	char * pickDevicename = NULL; 

	_boardVec.clear();

	//get all devices' name
	buffersize = DAQmxGetSystemInfoAttribute(DAQmx_Sys_DevNames, devicenames);
	devicenames=(char*)malloc(buffersize);
	DAQmxGetSystemInfoAttribute(DAQmx_Sys_DevNames, devicenames, buffersize);

	//Get information about all devices
	pickDevicename = strtok_s (devicenames,", ",&nxdevicenames);
	while(pickDevicename != NULL)
	{
		//add found board info
		BoardInfo bInfo;
		bInfo.devName = pickDevicename;
		bInfo.devType = GetNIDeviceAttribute(bInfo.devName, DAQmx_Dev_ProductType);
		bInfo.boardStyle = (std::string::npos != bInfo.devType.find("USB")) ? BoardStyle::USB : BoardStyle::PCI;

		std::string compStr1 = GetNIDeviceAttribute(bInfo.devName,DAQmx_Dev_Terminals);
		bInfo.rtsiConfigure = ((0 < compStr1.size()) && (std::string::npos != compStr1.find("RTSI"))) ? 1 : 0;
		
		compStr1 = GetNIDeviceCIPhysicalChans(bInfo.devName);
		std::string toSearch = "ctr";
		std::size_t pos = compStr1.find("ctr");
		int i = 0;
		while (pos!=std::string::npos)
		{
			i++;
			pos = compStr1.find("ctr", pos + toSearch.size());
		}
		bInfo.counterCount = i;

		_boardVec.push_back(bInfo);

		pickDevicename = strtok_s(NULL,", ",&nxdevicenames);
	}

	if(devicenames)
	{
		free(devicenames);
		devicenames = NULL;
	}
}

BoardInfo* BoardInfoNI::GetBoardInfo(std::string devName)
{
	if (0 >= devName.size())
		return NULL;

	for (unsigned int i = 0; i < _boardVec.size(); i++)
	{
		if(std::string::npos != _boardVec.at(i).devName.find(devName))
		{
			return &_boardVec.at(i);
		}
	}
	return NULL;
}

long BoardInfoNI::VerifyLineNI(BoardInfo* bInfo, LineTypeNI signalType, std::string lineName)
{
	if (NULL == bInfo)
		return FALSE;

	std::string compStr1,compStr2;
	switch (signalType)
	{
	case LineTypeNI::ANALOG_IN:
		compStr1 = GetNIDeviceAttribute(bInfo->devName,DAQmx_Dev_AI_PhysicalChans);
		compStr2 = (0 == lineName.find("/")) ? (lineName.substr(1, lineName.size()-1)) : (lineName);
		break;
	case LineTypeNI::DIGITAL_IN:
		compStr1 = GetNIDeviceAttribute(bInfo->devName,DAQmx_Dev_DI_Lines);
		compStr2 = (0 == lineName.find("/")) ? (lineName.substr(1, lineName.size()-1)) : (lineName);
		break;
	case LineTypeNI::TERMINAL:
		compStr1 = GetNIDeviceAttribute(bInfo->devName,DAQmx_Dev_Terminals);
		compStr2 = (0 != lineName.find("/")) ? ("/" + lineName) : (lineName);
		break;
	case LineTypeNI::ANALOG_OUT:
		compStr1 = GetNIDeviceAttribute(bInfo->devName,DAQmx_Dev_AO_PhysicalChans);
		compStr2 = (0 == lineName.find("/")) ? (lineName.substr(1, lineName.size()-1)) : (lineName);
		break;
	case LineTypeNI::COUNTER:
		compStr1 = GetNIDeviceCIPhysicalChans(bInfo->devName);
		compStr2 = (0 == lineName.find("/")) ? (lineName.substr(1, lineName.size()-1)) : (lineName);
		break;
	default:
		return FALSE;
	}
	//consider line name case ai14:ai15
	size_t pos = compStr2.find(":");
	if (pos != std::string::npos)
	{
		std::string str = compStr2.substr(0, pos);
		compStr2.erase(0, pos+1);
		if((0 == compStr1.size()) || (std::string::npos == compStr1.find(str)) || (std::string::npos == compStr1.find(compStr2)))
		{
			goto INVALID_NAME;
		}
	}
	else
	{
		if((0 == compStr1.size()) || (std::string::npos == compStr1.find(compStr2)))
		{
			goto INVALID_NAME;
		}
	}
	return TRUE;

INVALID_NAME:
	std::wstring error= std::wstring(lineName.begin(),lineName.end()) + L" is not available on device. ";
	LogMessage((wchar_t*)error.c_str(),ERROR_EVENT);
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////

bool AnalogReaderNI::_instanceFlag = false;
std::unique_ptr<AnalogReaderNI> AnalogReaderNI::_single(new AnalogReaderNI());

AnalogReaderNI* AnalogReaderNI::getInstance()
{ 
	if (!_instanceFlag) 
	{ 
		_single.reset(new AnalogReaderNI());	
		_instanceFlag = true;
	}	
	return _single.get();
}

long AnalogReaderNI::AddLine(std::string lineName)
{
	signed long retVal = 0, error = 0;

	//no leading '/' allowed
	lineName = (0 == lineName.find("/")) ? (lineName.substr(1, lineName.size()-1)) : lineName;

	//unique task per line name 
	if (_lineNames.end() == std::find(_lineNames.begin(), _lineNames.end(), lineName))
	{
		_lock.lock();
		try
		{
			boardsInfo.get()->getInstance()->GetAllBoardsInfo();
			if (boardsInfo.get()->getInstance()->VerifyLineNI(boardsInfo.get()->getInstance()->GetBoardInfo(GetDevIDName(lineName)), LineTypeNI::ANALOG_IN, lineName))
			{
				TaskHandle hTask;
				DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &hTask));
				DAQmxErrChk(L"DAQmxCreateAIVoltageChan",retVal = DAQmxCreateAIVoltageChan (hTask, lineName.c_str(), "", DAQmx_Val_Cfg_Default, MIN_AO_VOLTAGE, MAX_AO_VOLTAGE, DAQmx_Val_Volts, NULL));

				_taskHandles.push_back(hTask);
				_lineNames.push_back(lineName);
			}
		}
		catch(...)
		{
			DAQmxFailed(error);
			StringCbPrintfW(message,_MAX_PATH,L"AnalogReaderNI:%hs@%u error: (%d)",__FUNCTION__, __LINE__,retVal);
			LogMessage(message,ERROR_EVENT);
		}
		_lock.unlock();
	}
	return (0 == retVal) ? TRUE : FALSE;
}

void AnalogReaderNI::RemoveLine(std::string lineName)
{
	//no leading '/' allowed
	lineName = (0 == lineName.find("/")) ? (lineName.substr(1, lineName.size()-1)) : lineName;

	auto it = std::find(_lineNames.begin(), _lineNames.end(), lineName);
	if (_lineNames.end() != it)
	{
		int id = static_cast<int>(std::distance(_lineNames.begin(), it));
		TerminateTask(_taskHandles.at(id));
		_taskHandles.erase(_taskHandles.begin() + id);
		_lineNames.erase(_lineNames.begin() + id);
	}
}

long AnalogReaderNI::ReadLine(std::string lineName, int count, double* data)
{
	const int REPEAT_TRIAL = 10;
	signed long retVal = 0, samplesRead = 0;

	_lock.lock();

	//no leading '/' allowed
	lineName = (0 == lineName.find("/")) ? (lineName.substr(1, lineName.size()-1)) : lineName;

	auto it = std::find(_lineNames.begin(), _lineNames.end(), lineName);
	if (_lineNames.end() == it)
	{
		//not found, unable to read line
		goto _UNLOCK;
	}
	else
	{
		if (NULL == _taskHandles.at(std::distance(_lineNames.begin(), it)))
			goto _UNLOCK;

		//repeat trial since it could fail at the first time
		for (int i = 0; i < REPEAT_TRIAL; i++)
		{
			retVal = DAQmxReadAnalogF64(_taskHandles.at(std::distance(_lineNames.begin(), it)), 1, MAX_TASK_WAIT_TIME, DAQmx_Val_GroupByChannel, data, count, &samplesRead, NULL);
			if (0 == retVal && 1 <= samplesRead)
				break;
		}
		if (0 != retVal)
		{
			StringCbPrintfW(message,_MAX_PATH,L"AnalogReaderNI:%hs@%u error: (%d)",__FUNCTION__, __LINE__,retVal);
			LogMessage(message,ERROR_EVENT);
			goto _UNLOCK;
		}
	}

	_lock.unlock();
	return TRUE;

_UNLOCK:
	_lock.unlock();
	return FALSE;
}
