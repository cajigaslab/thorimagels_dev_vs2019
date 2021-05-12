// ThorPMT2100.cpp : Defines the exported functions for the DLL application.
//

//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorPMT2100.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "ThorPMT2100.h"
#include "ThorPMT2100XML.h"
#include "Strsafe.h"

#define H10721_GAINPOWER_MIN 0.50
#define H10721_GAINPOWER_MAX 1.10
#define H10721_GAINPOWER 7.29
#define H10721_GAINMAXCOUNTS 620

#define H10770PA40_GAINPOWER_MIN 0.50
#define H10770PA40_GAINPOWER_MAX 0.90
#define H10770PA40_GAINPOWER 6.64
#define H10770PA40_GAINMAXCOUNTS 510

#define H11706P40_GAINPOWER_MIN 0.50
#define H11706P40_GAINPOWER_MAX 0.90
#define H11706P40_GAINPOWER 6.64
#define H11706P40_GAINMAXCOUNTS 510

#define APD_GAINPOWER_MIN 0.00
#define APD_GAINPOWER_MAX 2.00
#define APD_GAINPOWER 1.0
#define APD_GAINMAXCOUNTS 1

#define DIODE_GAINPOWER_MIN 0.00
#define DIODE_GAINPOWER_MAX 2.00
#define DIODE_GAINPOWER 1.0
#define DIODE_GAINMAXCOUNTS 1

#define GAIN_OFFSET_MIN -1.5
#define GAIN_OFFSET_MAX 1.5
#define GAIN_OFFSET_DEFAULT 0

#define CMD_LEN 256
#define ADDRESS_LEN 256
#define TIME_OUT 10 //miliseconds
#define LOCK_TIME_OUT 100
#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

/// <summary>
/// The message
/// </summary>
wchar_t message[MSG_SIZE];


/// <summary>
/// Prevents a default instance of the <see cref="ThorPMT2100"/> class from being created.
/// </summary>
ThorPMT2100::ThorPMT2100()
{

}

/// <summary>
/// Finalizes an instance of the <see cref="ThorPMT2100"/> class.
/// </summary>
ThorPMT2100::~ThorPMT2100()
{
	_instanceFlag = false;
	_errMsg[0] = 0;
}

CritSect ThorPMT2100::_critSect;

bool ThorPMT2100:: _instanceFlag = false;

auto_ptr<ThorPMT2100> ThorPMT2100::_single(new ThorPMT2100());

const string ThorPMT2100::_deviceSignature[DEVICE_NUM] = {"PMT1", "PMT2", "PMT3", "PMT4", "PMT5", "PMT6"};
const long ThorPMT2100::_pmtSelect[DEVICE_NUM] = {PMT1, PMT2, PMT3, PMT4, PMT5, PMT6};
const wstring ThorPMT2100::_addressStructure = L"USB0::0x1313::0x2F00::%S::0::INSTR";

wstring utf8toUtf16(const string &str)
{
	if (str.empty())
		return wstring();

	size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0, 
		str.data(), (int)str.size(), NULL, 0);
	if (charsNeeded == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	vector<wchar_t> buffer(charsNeeded);
	int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0, 
		str.data(), static_cast<long>(str.size()), &buffer[0], static_cast<long>(buffer.size()));
	if (charsConverted == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	return wstring(&buffer[0], charsConverted);
}

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorPMT2100 *.</returns>
ThorPMT2100 *ThorPMT2100::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorPMT2100());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="deviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorPMT2100::FindDevices(long &deviceCount)
{
	Lock lock(_critSect);
	long ret = FALSE;	

	long baudRate=0;
	deviceCount=0;

	//Reset detected devices and types
	for (long i = 0; i < DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
		_pmtType[i] = "";
		_serialNumber[i] = "";
	} 
	_deviceDetected[DEVICE_NUM] = FALSE;

	ViStatus status = 0;

	//Open the Resourse manager Visa driver session
	status = viClose(_viRM);
	status = viOpenDefaultRM(&_viRM);
	ViFindList detectedDeviceList;
	ViUInt32 nDetectedDevices;
	ViChar desc[VI_FIND_BUFLEN];

	//Scan all available usb devices using the Visa driver
	status = viFindRsrc(_viRM,"USB0::?*INSTR",&detectedDeviceList,&nDetectedDevices,desc);
	string detectedAddress[DEVICE_NUM];
	string detectedSN[DEVICE_NUM];
	long detectedDeviceStatus[DEVICE_NUM];

	if (nDetectedDevices > 0)
	{
		ViRsrc deviceAddress=desc;
		detectedAddress[0] = string(deviceAddress);	

		//get the next available devices from the detectedDeviceList
		for (long i= 1; i < min(DEVICE_NUM,static_cast<long>(nDetectedDevices)); i++)
		{	
			status = viFindNext(detectedDeviceList,deviceAddress);
			detectedAddress[i] = string(deviceAddress);			
		}

		//Check for the model number on each device
		for (long i= 0; i < min(DEVICE_NUM,static_cast<long>(nDetectedDevices)); i++)
		{
			ViRsrc address = new char[detectedAddress[i].length() + 1];
			strcpy_s(address, detectedAddress[i].length() + 1, detectedAddress[i].c_str());			
			ViSession session;			
			status = viOpen(_viRM, address, 0, TIME_OUT, &session);
			vector<string> idnResponse;
			if (TRUE == QueryDeviceIDN(session, idnResponse))
			{
				//If the devices model number is pmt2100 then mark its status as TRUE
				//indicating that this dll can connect to it appropiately
				if ("pmt2100" == idnResponse[1])
				{
					detectedDeviceStatus[i] = TRUE;
					detectedSN[i] = idnResponse[2];
				}
				else
				{
					detectedDeviceStatus[i] = FALSE;
					detectedSN[i] = "";
				}
			}
		}
	}
	else
	{
		deviceCount = 0;
		return FALSE;
	}

	string deviceSN[DEVICE_NUM];
	for(long i=0; i<DEVICE_NUM; i++)
	{
		_deviceAddress[i] = "";

		try
		{
			auto_ptr<ThorPMT2100XML> pSetup(new ThorPMT2100XML());
			//retrieve the serial device serial numbers from the settings xml
			//this serial numbers will be used to complete the Address to connect
			//to the devices using the Visa driver
			pSetup->GetDeviceAddress(_deviceSignature[i],deviceSN[i]);			
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorPMT2100Settings.xml file");
			deviceCount = 0;
			return FALSE;
		}

		//use the address structure to complete the address with the serial number
		TCHAR wsAddress[ADDRESS_LEN];		
		StringCbPrintf(wsAddress, ADDRESS_LEN, _addressStructure.c_str(), deviceSN[i].c_str());
		wstring wAddress(wsAddress);
		_deviceAddress[i] = ConvertWStringToString(wAddress);

		//compare the detected device serial number to the serial number in the settings file
		//if the serial number is repeated then set the detectedDeviceStatus for the
		//corresponding detected device
		for (long j = 0; j < min(DEVICE_NUM,static_cast<long>(nDetectedDevices)); j++)
		{
			if (TRUE == detectedDeviceStatus[j])
			{
				if (_deviceAddress[i] == detectedAddress[j] || detectedSN[j] == deviceSN[i])
				{
					detectedDeviceStatus[j] = FALSE;
				}
			}
		}
	}

	for(long i=0; i <DEVICE_NUM; i++)
	{
		//if the serial number is NA try to fill up the address with one of the
		//detected devices. This will only succed if there are extra detected devices
		//which serial number is not in the settings file.
		if ("NA" == deviceSN[i]) 
		{
			for (long j = 0; j < min(DEVICE_NUM,static_cast<long>(nDetectedDevices)); j++)
			{
				if (TRUE == detectedDeviceStatus[j])
				{
					detectedDeviceStatus[j] = FALSE;
					_deviceAddress[i] = detectedAddress[j];
					break;
				}
			}
		}
	}

	//try to connect to each of the six devices. If successful then add the device type to the deviceCount.
	//also get the serial number of the device and set devicesDetected[i] to true
	for(long i=0; i<DEVICE_NUM; i++)
	{		
		ViRsrc address = new char[_deviceAddress[i].length() + 1];
		strcpy_s(address, _deviceAddress[i].length() + 1, _deviceAddress[i].c_str());
		status = viOpen(_viRM, address, 0, TIME_OUT, &_viSession[i]);

		if(0 > status)
		{
#ifdef LOGGING_ENABLED
			logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorPMT2100 Unable to connect to device with address: %S");
#endif
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to connect to device with address: %S",_deviceAddress[i]);
		}
		else
		{
			deviceCount = 1;
			ret = _deviceDetected[DEVICE_NUM] = _deviceDetected[i] = TRUE;
			vector<string> idnResponse;
			QueryDeviceIDN(_viSession[i], idnResponse);
			if (4 <= idnResponse.size())
			{
				//Retrieve the Serial Number
				_serialNumber[i] = idnResponse[2];

				//Retrieve the firmware Version
				std::stringstream ssFV(idnResponse[3]);
				ssFV >> _firmwareVersion[i];
			}
		}
	}
	return ret;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorPMT2100::SelectDevice(const long device)
{
	Lock lock(_critSect);
	long ret = FALSE;
	_connectedPMTs = 0;
	//Check if any device detected before continuing
	if(_deviceDetected[DEVICE_NUM]==FALSE) 
	{
		return FALSE;
	}

	//Reset detected devices and types
	for (int i = 0; i < DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
		_pmtType[i] = "";
	}
	_deviceDetected[DEVICE_NUM] = FALSE;

	//try connecting to each device
	for(int i=0; i<DEVICE_NUM; i++)
	{
		ViStatus status = 0;
		ViRsrc address = new char[_deviceAddress[i].length() + 1];
		strcpy_s(address, _deviceAddress[i].length() + 1, _deviceAddress[i].c_str());
		status = viOpen(_viRM, address, 0, TIME_OUT, &_viSession[i]);

		if(0 > status)
		{
#ifdef LOGGING_ENABLED
			logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorPMT2100 FindDevices could not connect to any device");
#endif
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to connect to device with address: %S",_deviceAddress[i]);
		}
		else
		{
			ret = _deviceDetected[DEVICE_NUM] = _deviceDetected[i] = TRUE;
			_connectedPMTs |= _pmtSelect[i];
		}
	}

	if (TRUE == ret)
	{
		GetPMTTypeAndRanges();
		CreateParamTable();
	}

	return ret;
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorPMT2100::TeardownDevice()
{
	try
	{
		//Disable the PMTs
		SetParam(IDevice::PARAM_PMT1_ENABLE, FALSE);
		SetParam(IDevice::PARAM_PMT2_ENABLE, FALSE);
		SetParam(IDevice::PARAM_PMT3_ENABLE, FALSE);
		SetParam(IDevice::PARAM_PMT4_ENABLE, FALSE);
		SetParam(IDevice::PARAM_PMT5_ENABLE, FALSE);
		SetParam(IDevice::PARAM_PMT6_ENABLE, FALSE);

		SetupPosition();
		StartPosition();


		DestroyParamTable();
		for (long i=0; i < DEVICE_NUM; i++)
		{		
			if(TRUE == _deviceDetected[i])
			{
				viClose(_viSession[i]);				
			}
		}		
		viClose(_viRM);
	}
	catch (...)
	{
	}
	return TRUE;
}

/// <summary>
/// Gets the parameter information.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="paramType">Type of the parameter.</param>
/// <param name="paramAvailable">The parameter available.</param>
/// <param name="paramReadOnly">The parameter read only.</param>
/// <param name="paramMin">The parameter minimum.</param>
/// <param name="paramMax">The parameter maximum.</param>
/// <param name="paramDefault">The parameter default.</param>
/// <returns>long.</returns>
long ThorPMT2100::GetParamInfo
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
	if (NULL != _tableParams[paramID])
	{
		if(_tableParams[paramID]->GetParamID() == paramID)
		{
			paramType = _tableParams[paramID]->GetParamType();
			paramAvailable = _tableParams[paramID]->GetParamAvailable();
			paramReadOnly = _tableParams[paramID]->GetParamReadOnly();
			paramMin = _tableParams[paramID]->GetParamMin();
			paramMax = _tableParams[paramID]->GetParamMax();
			paramDefault = _tableParams[paramID]->GetParamDefault();
			return TRUE;
		}
	}

	paramAvailable = FALSE;
	return FALSE;	
}

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorPMT2100::SetParam(const long paramID, const double param)
{
	if (NULL != _tableParams[paramID])
	{
		if(_tableParams[paramID]->GetParamID() == paramID)
		{
			if(FALSE == (_tableParams[paramID]->GetParamAvailable()) || (TRUE == _tableParams[paramID]->GetParamReadOnly()))
			{
				return FALSE;
			}
			else if((_tableParams[paramID]->GetParamMin() <= param) && (_tableParams[paramID]->GetParamMax() >= param))
			{
				_tableParams[paramID]->UpdateParam(param);
				return TRUE;
			}
			else
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorPMT2100 SetParam failed. paramID: %d", paramID);
				LogMessage(message);
				return FALSE;
			}
		}
	}

	return FALSE;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorPMT2100::GetParam(const long paramID, double &param)
{
	if (NULL != _tableParams[paramID])
	{
		if (FALSE == _tableParams[paramID]->GetParamAvailable())
		{
			return FALSE;
		}
		switch (_tableParams[paramID]->GetParamID())
		{
			case PARAM_PMT1_SAFETY:
			case PARAM_PMT2_SAFETY:
			case PARAM_PMT3_SAFETY:
			case PARAM_PMT4_SAFETY:
			case PARAM_PMT5_SAFETY:
			case PARAM_PMT6_SAFETY:
			{
				long status = 0;
				long ret = QueryPMTStatus(_tableParams[paramID]->GetDeviceIndex(), status);
				param = status;
				return ret;
			}
			case PARAM_PMT1_GAIN_POS_CURRENT_VOLTS:
			case PARAM_PMT2_GAIN_POS_CURRENT_VOLTS:
			case PARAM_PMT3_GAIN_POS_CURRENT_VOLTS:
			case PARAM_PMT4_GAIN_POS_CURRENT_VOLTS:
			case PARAM_PMT5_GAIN_POS_CURRENT_VOLTS:
			case PARAM_PMT6_GAIN_POS_CURRENT_VOLTS:
			case PARAM_PMT1_OUTPUT_OFFSET_CURRENT:
			case PARAM_PMT2_OUTPUT_OFFSET_CURRENT:
			case PARAM_PMT3_OUTPUT_OFFSET_CURRENT:
			case PARAM_PMT4_OUTPUT_OFFSET_CURRENT:
			case PARAM_PMT5_OUTPUT_OFFSET_CURRENT:
			case PARAM_PMT6_OUTPUT_OFFSET_CURRENT:
			case PARAM_PMT1_BANDWIDTH_POS_CURRENT:
			case PARAM_PMT2_BANDWIDTH_POS_CURRENT:
			case PARAM_PMT3_BANDWIDTH_POS_CURRENT:
			case PARAM_PMT4_BANDWIDTH_POS_CURRENT:
			case PARAM_PMT5_BANDWIDTH_POS_CURRENT:
			case PARAM_PMT6_BANDWIDTH_POS_CURRENT:
			{
				long ret = QueryPMTSetting(_tableParams[paramID], param);
				return ret;
			}
			case PARAM_CONNECTION_STATUS:
			{
				param = (_deviceDetected[DEVICE_NUM]) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
				return TRUE;
			}
			default:
				param = _tableParams[paramID]->GetParamVal();
				return TRUE;
		}
	}

	return FALSE;
}

/// <summary>
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorPMT2100::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	return ret;
}

/// <summary>
/// Gets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorPMT2100::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	return ret;
}

/// <summary>
/// Sets the parameter string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <returns>long.</returns>
long ThorPMT2100::SetParamString(const long paramID, wchar_t* str)
{
	long ret = FALSE;
	if(paramID == PARAM_CONNECTED_PMTS)
	{
		wstring ws(str);
		if(ws.substr(0, 4) == wstring(L"Save"))
		{
			int index = _wtoi(ws.substr(4, 1).c_str()) - 1;
			fstream fTemp;
			fTemp.open("pmtTemp.txt", fstream::out);

			if(_deviceDetected[index])
			{
				fTemp<<_pmtType[index]<<endl;
				fTemp<<_serialNumber[index]<<endl;
				fTemp<<_firmwareVersion[index]<<endl;
			}
			fTemp.close();
		}
		else if(ws.substr(0, 7) == wstring(L"Restore"))
		{
			int index = _wtoi(ws.substr(7, 1).c_str()) - 1;
			fstream fTemp;
			fTemp.open("pmtTemp.txt", fstream::in);

			fTemp>>_pmtType[index];
			fTemp>>_serialNumber[index];

			string response = "";
			ExecuteCmd(string("SENS:DET ") + _pmtType[index] + "\n", index, FALSE, response);
			ExecuteCmd(string("SET:SER ") +  _serialNumber[index] + "\n", index, FALSE, response);
			if("H10770PA-40" == _pmtType[index])
			{
				ExecuteCmd(string("INST GAIN;:SOUR:VOLT:LIM:LOW 0.5V\n"), index, FALSE, response);
				ExecuteCmd(string("INST GAIN;:SOUR:VOLT:LIM:HIGH 0.9V\n"), index, FALSE, response);
				ExecuteCmd(string("INST OFFSET;:SOUR:VOLT:LIM:LOW -1.5V\n"), index, FALSE, response);
				ExecuteCmd(string("INST OFFSET;:SOUR:VOLT:LIM:HIGH 1.5V\n"), index, FALSE, response);
				ret = TRUE;
			}
			else if ("H10721" == _pmtType[index])
			{
				ExecuteCmd(string("INST GAIN;:SOUR:VOLT:LIM:LOW 0.5V\n"), index, FALSE, response);
				ExecuteCmd(string("INST GAIN;:SOUR:VOLT:LIM:HIGH 1.1V\n"), index, FALSE, response);
				ExecuteCmd(string("INST OFFSET;:SOUR:VOLT:LIM:LOW -1.5V\n"), index, FALSE, response);
				ExecuteCmd(string("INST OFFSET;:SOUR:VOLT:LIM:HIGH 1.5V\n"), index, FALSE, response);
				ret = TRUE;
			}

			fTemp.close();
			remove("pmtTemp.txt");
		}
	}

	return ret;
}

/// <summary>
/// Gets the parameter of type string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorPMT2100::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_PMT1_SERIALNUMBER:
		{
			wstring wsTemp = utf8toUtf16(_serialNumber[0]);
			wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case PARAM_PMT2_SERIALNUMBER:
		{
			wstring wsTemp = utf8toUtf16(_serialNumber[1]);
			wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case PARAM_PMT3_SERIALNUMBER:
		{
			wstring wsTemp = utf8toUtf16(_serialNumber[2]);
			wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case PARAM_PMT4_SERIALNUMBER:
		{
			wstring wsTemp = utf8toUtf16(_serialNumber[3]);
			wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case PARAM_PMT5_SERIALNUMBER:
		{
			wstring wsTemp = utf8toUtf16(_serialNumber[4]);
			wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	case PARAM_PMT6_SERIALNUMBER:
		{
			wstring wsTemp = utf8toUtf16(_serialNumber[5]);
			wcscpy_s(str,size, wsTemp.c_str());
		}
		break;
	default:
		ret = FALSE;
	}

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorPMT2100::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorPMT2100::SetupPosition()
{
	//iterate through map and set the parameters
	for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		if(((iter->second)->GetParamAvailable() == TRUE) && ((iter->second)->GetParamReadOnly() == FALSE))
		{
			if((iter->second)->GetParamVal() != (iter->second)->GetParamCurrent())
			{
				(iter->second)->SetParamBool(TRUE);				
			}
			else
			{
				(iter->second)->SetParamBool(FALSE);				
			}
		}
	}
	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorPMT2100::StartPosition()
{
	long ret = TRUE;

	//iterate through map and set the parameters
	for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if(((iter->second)->GetParamAvailable() == FALSE) || ((iter->second)->GetParamReadOnly() == TRUE))
		{
			continue;
		}
		else if((iter->second)->GetParamBool() == TRUE)
		{	
			ret = ExecuteCmd(iter->second);
			(iter->second)->UpdateParam_C();

			(iter->second)->SetParamBool(FALSE);

			StringCbPrintfW(message,MSG_SIZE,L"StartPosition succeeded at paramID: %d",(iter->second)->GetParamID());
		}
	}

	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorPMT2100::StatusPosition(long &status)
{
	long ret = TRUE;
	status = IDevice::STATUS_READY;
	return ret;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorPMT2100::ReadPosition(DeviceType deviceType, double &pos)
{
	return TRUE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorPMT2100::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Queries the PMT gain.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorPMT2100::QueryPMTSetting(ParamInfo* pParamInfo, double &param)
{	
	wstring command =  pParamInfo->GetCmdStr();
	string response ="";
	long ret = ExecuteCmd(ConvertWStringToString(command), pParamInfo->GetDeviceIndex(), TRUE, response);
	if(TRUE == ret)
	{
		try
		{
			stringstream ss(response);
			ss>>param;
			return TRUE;
		}
		catch(...)
		{
			return FALSE;
		}
	}
	return ret;
}

/// <summary>
/// Queries the PMT status.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorPMT2100::QueryPMTStatus(const long pmtIndex, long &status)
{	
	return QueryPMTSafety(pmtIndex, status);
}

/// <summary>
/// Queries the PMT status.
/// </summary>
/// <param name="pmtIndex">Index of the pmt to be queried.</param>
/// <param name="safetyStatus">safety status of the pmt (0 or 1).</param>
/// <returns>long.</returns>
long ThorPMT2100::QueryPMTSafety(const long pmtIndex, long &safetyStatus)
{	
	string response ="";
	long ret = ExecuteCmd("*STB?",pmtIndex,TRUE,response);
	if (0 != strcmp(response.c_str(), "error"))
	{
		safetyStatus = 1;
		_tableParams[PARAM_PMT1_SATURATIONS + pmtIndex]->UpdateParam(0);
		_tableParams[PARAM_PMT1_SATURATIONS + pmtIndex]->UpdateParam_C();
	}
	else
	{
		string command = "SENS:CURR:PROT:TRIP?\n";

		ret = ExecuteCmd(command, pmtIndex, TRUE, response);
		
		safetyStatus = ('0' == response[0]) ? 1 : 0;

		//the error might also be that there are saturations
		//the number of saturations will be after the letter s
		auto saturationsStrPos = response.find("s");

		if (saturationsStrPos != std::string::npos && response.length() > 2)
		{
			string saturationsStr = response.substr(saturationsStrPos + 1);
			stringstream ss(saturationsStr);
			int saturations = 0;
			ss >> saturations;
			_tableParams[PARAM_PMT1_SATURATIONS + pmtIndex]->UpdateParam(saturations);
			_tableParams[PARAM_PMT1_SATURATIONS + pmtIndex]->UpdateParam_C();
		}
		else
		{
			_tableParams[PARAM_PMT1_SATURATIONS + pmtIndex]->UpdateParam(0);
			_tableParams[PARAM_PMT1_SATURATIONS + pmtIndex]->UpdateParam_C();
		}
	}

	return ret;
}

/// <summary>
/// Executes the command.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorPMT2100::ExecuteCmd(ParamInfo* pParamInfo)
{
	TCHAR wsCommand[CMD_LEN];
	long pmtIndex = pParamInfo->GetDeviceIndex();
	switch(pParamInfo->GetParamID())
	{
	case PARAM_PMT1_GAIN_POS:
	case PARAM_PMT2_GAIN_POS:
	case PARAM_PMT3_GAIN_POS:
	case PARAM_PMT4_GAIN_POS:
	case PARAM_PMT5_GAIN_POS:
	case PARAM_PMT6_GAIN_POS:
		{
			double val;
			if (0.0 == pParamInfo->GetParamVal())
			{
				val = 0.0;				
			}
			else
			{
				val = GainPercentToVolts(pmtIndex, static_cast<long>(pParamInfo->GetParamVal()));
			}
			StringCbPrintf(wsCommand, CMD_LEN, pParamInfo->GetCmdStr().c_str(), val);
			wstring wcommand(wsCommand);
			string response = "";
			return ExecuteCmd(ConvertWStringToString(wcommand),pmtIndex,FALSE,response);
		}
	case PARAM_PMT1_OUTPUT_OFFSET:
	case PARAM_PMT2_OUTPUT_OFFSET:
	case PARAM_PMT3_OUTPUT_OFFSET:
	case PARAM_PMT4_OUTPUT_OFFSET:
	case PARAM_PMT5_OUTPUT_OFFSET:
	case PARAM_PMT6_OUTPUT_OFFSET:
		{

			StringCbPrintf(wsCommand, CMD_LEN, pParamInfo->GetCmdStr().c_str(), pParamInfo->GetParamVal());

			wstring wcommand(wsCommand);
			string response = "";
			return ExecuteCmd(ConvertWStringToString(wcommand),pmtIndex,FALSE,response);
		}
	case PARAM_PMT1_ENABLE:
	case PARAM_PMT2_ENABLE:
	case PARAM_PMT3_ENABLE:
	case PARAM_PMT4_ENABLE:
	case PARAM_PMT5_ENABLE:
	case PARAM_PMT6_ENABLE:
		{
			double v = 0.0;
			std::map<long, ParamInfo*>::iterator iter = _tableParams.find(pParamInfo->GetParamID() - 1);
			ParamInfo* tmpInfo = iter->second;

			long paramVal = static_cast<long>(pParamInfo->GetParamVal());
			if (TRUE == paramVal)
			{
				StringCbPrintf(wsCommand, CMD_LEN, pParamInfo->GetCmdStr().c_str(), "ON");
				wstring wcommandEnable(wsCommand);
				string response = "";
				long r = ExecuteCmd(ConvertWStringToString(wcommandEnable),pmtIndex,FALSE,response);

				//also set the gain voltage after enabling the detector
				if(0.0 != tmpInfo->GetParamVal())
				{
					v = GainPercentToVolts(pmtIndex, static_cast<long>(tmpInfo->GetParamVal()));
					StringCbPrintf(wsCommand, CMD_LEN, tmpInfo->GetCmdStr().c_str(), v);
					wstring wcommandGainV(wsCommand);
					string response_v = "";
					ExecuteCmd(ConvertWStringToString(wcommandGainV),pmtIndex,FALSE,response_v);
				}

				return r;
			}
			else
			{
				//set the gain voltage to zero before desabling the detector
				if(tmpInfo->GetParamCurrent() > v)
				{
					StringCbPrintf(wsCommand, CMD_LEN, tmpInfo->GetCmdStr().c_str(), v);
					wstring wcommandGainV(wsCommand);
					string response_v = "";
					ExecuteCmd(ConvertWStringToString(wcommandGainV),pmtIndex,FALSE,response_v);
				}

				StringCbPrintf(wsCommand, CMD_LEN, pParamInfo->GetCmdStr().c_str(), "OFF");
				wstring wcommandEnable(wsCommand);
				string response = "";
				long ret = ExecuteCmd(ConvertWStringToString(wcommandEnable),pmtIndex,FALSE,response);
				//clear the safety trip after disabling the PMT
				if (TRUE == ret)
				{
					return ExecuteCmd("SENS:CURR:PROT:CLE\n",pmtIndex,FALSE,response);
				}
				else
				{
					return ret;
				}
			}			
		}
	case PARAM_PMT1_BANDWIDTH_POS:
	case PARAM_PMT2_BANDWIDTH_POS:
	case PARAM_PMT3_BANDWIDTH_POS:
	case PARAM_PMT4_BANDWIDTH_POS:
	case PARAM_PMT5_BANDWIDTH_POS:
	case PARAM_PMT6_BANDWIDTH_POS:
	case PARAM_PMT1_FIRMWAREUPDATE:
	case PARAM_PMT2_FIRMWAREUPDATE:
	case PARAM_PMT3_FIRMWAREUPDATE:
	case PARAM_PMT4_FIRMWAREUPDATE:
	case PARAM_PMT5_FIRMWAREUPDATE:
	case PARAM_PMT6_FIRMWAREUPDATE:
		{
			long paramVal = static_cast<long>(pParamInfo->GetParamVal());
			StringCbPrintf(wsCommand, CMD_LEN, pParamInfo->GetCmdStr().c_str(), paramVal);
			wstring wcommand(wsCommand);
			string response = "";
			return ExecuteCmd(ConvertWStringToString(wcommand),pmtIndex,FALSE,response);
		}
	}
	return FALSE;
}

/// <summary>
/// Executes the command.
/// Use only this function to communicate with the devices
/// after the connection has been stablished in select device.
/// </summary>
/// <param name="cmd">The command in a string.</param>
/// <param name="pmtIndex">The index of the PMT where the command is to be sent.</param>
/// <param name="bQuery">Query required Flag.</param>
/// <param name="response">String with the response of the query.</param>
/// <returns>long.</returns>
long ThorPMT2100::ExecuteCmd(const string cmd, const long pmtIndex, const long bQuery, string &response)
{
	Lock lock(_critSect);
	long ret = FALSE;
	if (_deviceDetected[pmtIndex]==TRUE)
	{	
		ViStatus status = 0;		
		ViString command = new char[cmd.length() + 1];	
		strcpy_s(command, cmd.length() + 1, cmd.c_str());
		viLock(_viSession[pmtIndex], VI_EXCLUSIVE_LOCK, LOCK_TIME_OUT, VI_NULL, VI_NULL);
		if (FALSE == bQuery)
		{
			status = viPrintf(_viSession[pmtIndex],(ViString)(cmd.c_str()));			
		}
		else
		{
			if (!strcmp(cmd.c_str(), "*STB?"))
			{
				//query status byte
				ViUInt16 pmtstatus = 0;
				status = viReadSTB(_viSession[pmtIndex], &pmtstatus);
				response = (0 == pmtstatus) ? "good" : "error"; 
			}
			else
			{
				//query using command string
				ViChar str[256];
				status = viQueryf(_viSession[pmtIndex],command,"%t",str);
				response = string(str);
			}
		}
		Sleep(10);
		viUnlock(_viSession[pmtIndex]);
		if (0 <= status)
		{
			ret = TRUE;
		}		
	}

	return ret;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorPMT2100::GetLastErrorMsg(wchar_t *msg, long size)
{	
	wcsncpy_s(msg,MSG_SIZE,_errMsg,MSG_SIZE);
	return TRUE;
}

/// <summary>
/// Gets the PMT type from the device ad sets the gain limits respectively
/// </summary>
/// <returns>long.</returns>
long ThorPMT2100::GetPMTTypeAndRanges()
{
	long ret = FALSE;

	//Get PMT type and set the ranges
	for (long i = 0; i < DEVICE_NUM; i++)
	{
		_pmtMaxGainVolts[i] = 0.1;
		_pmtMinGainVolts[i] = 0;
		_gainCountsMax[i] = 0;
		_gainPower[i] = 0;		
		if (TRUE == ExecuteCmd("SENS:DET:FUNC?", i, TRUE, _pmtType[i]))
		{
			if("H10770PA-40" == _pmtType[i])
			{
				_pmtMaxGainVolts[i] = H10770PA40_GAINPOWER_MAX;
				_pmtMinGainVolts[i] = H10770PA40_GAINPOWER_MIN;
				_gainCountsMax[i] = H10770PA40_GAINMAXCOUNTS;
				_gainPower[i] = H10770PA40_GAINPOWER;
				ret = TRUE;
			}
			else if ("H10721" == _pmtType[i])
			{
				_pmtMaxGainVolts[i] = H10721_GAINPOWER_MAX;
				_pmtMinGainVolts[i] = H10721_GAINPOWER_MIN;
				_gainCountsMax[i] = H10721_GAINMAXCOUNTS;
				_gainPower[i] = H10721_GAINPOWER;
				ret = TRUE;
			}
			else if ("H11706P-40" == _pmtType[i])
			{
				_pmtMaxGainVolts[i] = H11706P40_GAINPOWER_MAX;
				_pmtMinGainVolts[i] = H11706P40_GAINPOWER_MIN;
				_gainCountsMax[i] = H11706P40_GAINMAXCOUNTS;
				_gainPower[i] = H11706P40_GAINPOWER;
				ret = TRUE;
			}
			else if ("ADP" == _pmtType[i])
			{
				_pmtMaxGainVolts[i] = APD_GAINPOWER_MAX;
				_pmtMinGainVolts[i] = APD_GAINPOWER_MIN;
				_gainCountsMax[i] = APD_GAINMAXCOUNTS;
				_gainPower[i] = APD_GAINPOWER;
				ret = TRUE;
			}
			else if ("Diode" == _pmtType[i])
			{
				_pmtMaxGainVolts[i] = DIODE_GAINPOWER_MAX;
				_pmtMinGainVolts[i] = DIODE_GAINPOWER_MIN;
				_gainCountsMax[i] = DIODE_GAINMAXCOUNTS;
				_gainPower[i] = DIODE_GAINPOWER;
				ret = TRUE;
			}
		}			
	}

	//get the current pmt gain
	for (long i = 0; i < DEVICE_NUM; i++)
	{
		_startupGainPercent[i] = 0;
		string response = "";
		if (TRUE == ExecuteCmd("INST GAIN;:SOUR:VOLT?\n",i ,TRUE, response))
		{
			stringstream ss(response);
			double gainVolts = 0;
			ss>>gainVolts;
			_startupGainPercent[i] =  GainVoltsToPercent(i, gainVolts);
		}
	}

	return ret;
}

/// <summary>
/// Converts the gain from volts to percent
/// </summary>
/// <param name="pmtIndex">long that is the index of the pmt</param>
/// <param name="gainPercent">long that is the gain in percent.</param>
/// <returns>double.</returns>
double ThorPMT2100::GainPercentToVolts(long pmtIndex, long gainPercent) //returns the gain in Volts
{
	//linearization changes the lower bound to be at a digital value of 50 that is sent to the controller
	//reference u1 = V^(alpha*n)
	//We then conver this to voltage since the PMT2100 expects voltage values
	//We do this whole conversion to match the gains to those used by ThorECU
	const double GAINCOUNTSMAX = _gainCountsMax[pmtIndex];
	if(GAINCOUNTSMAX == 0)
		return 0;
	const double GAINPOWER = _gainPower[pmtIndex];
	const double GAINHBOUND = pow(_pmtMaxGainVolts[pmtIndex],GAINPOWER);
	const double GAINLBOUND = pow(_pmtMinGainVolts[pmtIndex],GAINPOWER);
	double actualGain = gainPercent/100.0 * (GAINHBOUND - GAINLBOUND) + GAINLBOUND;
	double bytegain = pow(actualGain/GAINHBOUND, 1/GAINPOWER) * GAINCOUNTSMAX;
	double roundedByteGain = ceil(bytegain - 0.5);
	double val = ceil(1.8 * roundedByteGain/1024.0 *10000 - 0.5) / 10000;
	return val;
}

/// <summary>
/// Converts the gain from volts to percent
/// </summary>
/// <param name="pmtIndex">long that is the index of the pmt</param>
/// <param name="gainVolts">double that is the gain in volts.</param>
/// <returns>long.</returns>
long ThorPMT2100::GainVoltsToPercent(long pmtIndex, double gainVolts) //returns the gain in Percent
{
	const double GAINCOUNTSMAX = _gainCountsMax[pmtIndex];
	if(GAINCOUNTSMAX == 0)
		return 0;
	const double GAINPOWER = _gainPower[pmtIndex];
	const double GAINHBOUND = pow(_pmtMaxGainVolts[pmtIndex],GAINPOWER);
	const double GAINLBOUND = pow(_pmtMinGainVolts[pmtIndex],GAINPOWER);
	double byteGain = gainVolts * 1024.0/(1.8);
	double roundedByteGain  = ceil(byteGain - 0.5);
	double percentageGain = (GAINHBOUND * pow(roundedByteGain/GAINCOUNTSMAX, GAINPOWER) - GAINLBOUND)/(GAINHBOUND - GAINLBOUND) * 100.0;
	return static_cast<long>(ceil(percentageGain - 0.5));
}

/// <summary>
/// Splits a string using a delimitator
/// </summary>
/// <param name="s">string to be split.</param>
/// <param name="delim">char which is the string delimitator.</param>
/// <param name="elems">vector<string> which which has the split string.</param>
/// <returns>long.</returns>
vector<string> &splitString(const string &s, char delim, vector<string> &elems) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

/// <summary>
/// Queries the IDN of the device.
/// Use to get the device's manufacturer, model, serial number, and firmware version.
/// </summary>
/// <param name="deviceAddress">Complete address of the device to query.</param>
/// <param name="parsedIDN">vector<string> with the parsed fields of the *IDN? query return.</param>
/// <returns>long.</returns>
long ThorPMT2100::QueryDeviceIDN(ViSession session, vector<string> &parsedIDN)
{
	Lock lock(_critSect);
	long ret = FALSE;

	//ViSession session;
	//ViRsrc address = new char[deviceAddress.length() + 1];
	//strcpy_s(address, deviceAddress.length() + 1, deviceAddress.c_str());
	//ViStatus status = viOpen(_viRM, address, 0, TIME_OUT, &session);
	ViChar str[256];

	viLock(session, VI_EXCLUSIVE_LOCK, LOCK_TIME_OUT, VI_NULL, VI_NULL);
	ViStatus status = viQueryf(session,"*IDN?","%t",str);
	viUnlock(session);

	string idn = string(str);

	splitString(idn, ',',parsedIDN);

	if (0 <= status && parsedIDN.size() >= 3)
	{
		ret = TRUE;
	}
	return ret;
}


/// <summary>
/// Queries the device for the model numer.
/// Use to check if the device is a pmt2100 before accepting it as one of the
/// PMTs to be used.
/// </summary>
/// <param name="deviceAddress">Complete address of the device to query.</param>
/// <param name="modelNumber">String with the model number of the device.</param>
/// <returns>long.</returns>
long ThorPMT2100::QueryDeviceModelNumber(string deviceAddress, string &modelNumber)
{
	long ret = FALSE;

	std::vector<std::string> parsedIDN;
	ViSession session;
	ViRsrc address = new char[deviceAddress.length() + 1];
	strcpy_s(address, deviceAddress.length() + 1, deviceAddress.c_str());
	ViStatus status = viOpen(_viRM, address, 0, TIME_OUT, &session);
	ret = QueryDeviceIDN(session, parsedIDN);

	if (2 <= parsedIDN.size() && TRUE == ret)
	{
		if (parsedIDN[1].find("pmt2100") != string::npos)
		{
			modelNumber = "pmt2100";
		}
		else
		{
			modelNumber = "other";
		}
	}
	else
	{
		ret = FALSE;
	}
	return ret;
}

/// <summary>
/// Creates the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorPMT2100::CreateParamTable()
{
	DestroyParamTable();
	ParamInfo* tempParamInfo = new ParamInfo(
		PARAM_DEVICE_TYPE,							//ID
		_connectedPMTs,								//VAL
		_connectedPMTs,								//PARAM C
		FALSE,										//PARAM B
		TYPE_LONG,									//TYPE
		TRUE,										//AVAILABLE
		TRUE,										//READ ONLY
		0,											//MIN
		PMT1 | PMT2 | PMT3 | PMT4 | PMT5 | PMT6,	//MAX
		0,											//DEFAULT
		L"",										//Command
		-1);										//Device Index
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_DEVICE_TYPE,tempParamInfo));

	//***Connected pmts Command***//
	tempParamInfo = new ParamInfo(	
		PARAM_CONNECTED_PMTS,
		_connectedPMTs,
		_connectedPMTs,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		PMT1 | PMT2 | PMT3 | PMT4 | PMT5 | PMT6,
		0,
		L"",
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTED_PMTS,tempParamInfo));
	//***end Connected pmts Command***//

	//***Gain Commands***//
	wstring gainCmd = wstring(L"INST GAIN;:SOUR:VOLT %fV\n");
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_GAIN_POS,
		_startupGainPercent[0],
		_startupGainPercent[0],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_MIN,
		GAIN_MAX,
		GAIN_MIN,
		gainCmd,
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_GAIN_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_GAIN_POS,
		_startupGainPercent[1],
		_startupGainPercent[1],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_MIN,
		GAIN_MAX,
		GAIN_MIN,
		gainCmd,
		1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_GAIN_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_GAIN_POS,
		_startupGainPercent[2],
		_startupGainPercent[2],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_MIN,
		GAIN_MAX,
		GAIN_MIN,
		gainCmd,
		2);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_GAIN_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_GAIN_POS,
		_startupGainPercent[3],
		_startupGainPercent[3],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_MIN,
		GAIN_MAX,
		GAIN_MIN,
		gainCmd,
		3);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_GAIN_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT5_GAIN_POS,
		_startupGainPercent[4],
		_startupGainPercent[4],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_MIN,
		GAIN_MAX,
		GAIN_MIN,
		gainCmd,
		4);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_GAIN_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT6_GAIN_POS,
		_startupGainPercent[5],
		_startupGainPercent[5],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_MIN,
		GAIN_MAX,
		GAIN_MIN,
		gainCmd,
		5);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_GAIN_POS,tempParamInfo));
	//***end Gain Commands***//

	//***Current Gain Commands***//
	wstring gainQueryCommand = L"INST GAIN;:SOUR:VOLT?\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_GAIN_POS_CURRENT_VOLTS,
		_pmtMinGainVolts[0],
		_pmtMinGainVolts[0],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		_pmtMinGainVolts[0],
		_pmtMaxGainVolts[0],
		_pmtMinGainVolts[0],
		gainQueryCommand,
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_GAIN_POS_CURRENT_VOLTS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_GAIN_POS_CURRENT_VOLTS,
		_pmtMinGainVolts[1],
		_pmtMinGainVolts[1],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_pmtMinGainVolts[1],
		_pmtMaxGainVolts[1],
		_pmtMinGainVolts[1],
		gainQueryCommand,
		1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_GAIN_POS_CURRENT_VOLTS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_GAIN_POS_CURRENT_VOLTS,
		_pmtMinGainVolts[2],
		_pmtMinGainVolts[2],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_pmtMinGainVolts[2],
		_pmtMaxGainVolts[2],
		_pmtMinGainVolts[2],
		gainQueryCommand,
		2);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_GAIN_POS_CURRENT_VOLTS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_GAIN_POS_CURRENT_VOLTS,
		_pmtMinGainVolts[3],
		_pmtMinGainVolts[3],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_pmtMinGainVolts[3],
		_pmtMaxGainVolts[3],
		_pmtMinGainVolts[3],
		gainQueryCommand,
		3);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_GAIN_POS_CURRENT_VOLTS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT5_GAIN_POS_CURRENT_VOLTS,
		_pmtMinGainVolts[4],
		_pmtMinGainVolts[4],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_pmtMinGainVolts[4],
		_pmtMaxGainVolts[4],
		_pmtMinGainVolts[4],
		gainQueryCommand,
		4);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_GAIN_POS_CURRENT_VOLTS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT6_GAIN_POS_CURRENT_VOLTS,
		_pmtMinGainVolts[5],
		_pmtMinGainVolts[5],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_pmtMinGainVolts[5],
		_pmtMaxGainVolts[5],
		_pmtMinGainVolts[5],
		gainQueryCommand,
		5);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_GAIN_POS_CURRENT_VOLTS,tempParamInfo));
	//***end Current Gain Commands***//

	//***Gain Offset Commands***//
	wstring gainOffsetCmd = L"INST OFFSET;:SOUR:VOLT %.3fV\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_OUTPUT_OFFSET,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetCmd,
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_OUTPUT_OFFSET,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_OUTPUT_OFFSET,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetCmd,
		1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_OUTPUT_OFFSET,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_OUTPUT_OFFSET,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetCmd,
		2);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_OUTPUT_OFFSET,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_OUTPUT_OFFSET,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetCmd,
		3);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_OUTPUT_OFFSET,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT5_OUTPUT_OFFSET,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetCmd,
		4);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_OUTPUT_OFFSET,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT6_OUTPUT_OFFSET,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetCmd,
		5);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_OUTPUT_OFFSET,tempParamInfo));
	//***end Gain Offset Commands***//

	//***Current Gain Offset Commands***//
	wstring gainOffsetQueryCommand = L"INST OFFSET;:SOUR:VOLT?\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_OUTPUT_OFFSET_CURRENT,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetQueryCommand,
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_OUTPUT_OFFSET_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_OUTPUT_OFFSET_CURRENT,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetQueryCommand,
		1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_OUTPUT_OFFSET_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_OUTPUT_OFFSET_CURRENT,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetQueryCommand,
		2);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_OUTPUT_OFFSET_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_OUTPUT_OFFSET_CURRENT,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetQueryCommand,
		3);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_OUTPUT_OFFSET_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT5_OUTPUT_OFFSET_CURRENT,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetQueryCommand,
		4);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_OUTPUT_OFFSET_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT6_OUTPUT_OFFSET_CURRENT,
		GAIN_OFFSET_DEFAULT,
		GAIN_OFFSET_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		GAIN_OFFSET_MIN,
		GAIN_OFFSET_MAX,
		GAIN_OFFSET_DEFAULT,
		gainOffsetQueryCommand,
		5);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_OUTPUT_OFFSET_CURRENT,tempParamInfo));
	//***end Current Gain Offset Commands***//

	//***Enable Commands***//	
	string enableCmd1 = "SENS:FUNC:%S " + _pmtType[0] + "\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_ENABLE,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		wstring(enableCmd1.begin(), enableCmd1.end()),
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_ENABLE,tempParamInfo));

	string enableCmd2 = "SENS:FUNC:%S " + _pmtType[1] + "\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_ENABLE,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		wstring(enableCmd2.begin(), enableCmd2.end()),
		1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_ENABLE,tempParamInfo));

	string enableCmd3 = "SENS:FUNC:%S " + _pmtType[2] + "\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_ENABLE,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		wstring(enableCmd3.begin(), enableCmd3.end()),
		2);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_ENABLE,tempParamInfo));

	string enableCmd4 = "SENS:FUNC:%S " + _pmtType[3] + "\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_ENABLE,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		wstring(enableCmd4.begin(), enableCmd4.end()),
		3);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_ENABLE,tempParamInfo));

	string enableCmd5 = "SENS:FUNC:%S " + _pmtType[4] + "\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT5_ENABLE,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		wstring(enableCmd5.begin(), enableCmd5.end()),
		4);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_ENABLE,tempParamInfo));

	string enableCmd6 = "SENS:FUNC:%S " + _pmtType[5] + "\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT6_ENABLE,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		wstring(enableCmd6.begin(), enableCmd6.end()),
		5);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_ENABLE,tempParamInfo));
	//***end Enable Commands***//

	//***Safety Tripped Commands***//
	wstring safetyTrippedCmd = L"SENS:CURR:PROT:TRIP?\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_SAFETY,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_SAFETY,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_SAFETY,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_SAFETY,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_SAFETY,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		2);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_SAFETY,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_SAFETY,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		3);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_SAFETY,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT5_SAFETY,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		4);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_SAFETY,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT6_SAFETY,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		5);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_SAFETY,tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT1_SATURATIONS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_SATURATIONS, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT2_SATURATIONS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_SATURATIONS, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT3_SATURATIONS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		2);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_SATURATIONS, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT4_SATURATIONS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		3);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_SATURATIONS, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT5_SATURATIONS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		4);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_SATURATIONS, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT6_SATURATIONS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		1,
		0,
		safetyTrippedCmd,
		5);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_SATURATIONS, tempParamInfo));

	//***end Safety Tripped Commands***//

	//***Frequency Commands***//
	wstring bandWidthCommnad = L"SENS:FILT:FREQ %dHz\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_BANDWIDTH_POS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthCommnad,
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_BANDWIDTH_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_BANDWIDTH_POS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthCommnad,
		1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_BANDWIDTH_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_BANDWIDTH_POS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthCommnad,
		2);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_BANDWIDTH_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_BANDWIDTH_POS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthCommnad,
		3);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_BANDWIDTH_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT5_BANDWIDTH_POS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthCommnad,
		4);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_BANDWIDTH_POS,tempParamInfo));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT6_BANDWIDTH_POS,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthCommnad,
		5);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_BANDWIDTH_POS,tempParamInfo));
	//***end Frequency Commands***//

	//***Frequency Commands***//
	wstring bandWidthQueryCommnad = L"SENS:FILT:FREQ?\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_BANDWIDTH_POS_CURRENT,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthQueryCommnad,
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_BANDWIDTH_POS_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_BANDWIDTH_POS_CURRENT,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthQueryCommnad,
		1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_BANDWIDTH_POS_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_BANDWIDTH_POS_CURRENT,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthQueryCommnad,
		2);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_BANDWIDTH_POS_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_BANDWIDTH_POS_CURRENT,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthQueryCommnad,
		3);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_BANDWIDTH_POS_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT5_BANDWIDTH_POS_CURRENT,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthQueryCommnad,
		4);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_BANDWIDTH_POS_CURRENT,tempParamInfo));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT6_BANDWIDTH_POS_CURRENT,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		BANDWIDTH_MIN,
		BANDWIDTH_MAX,
		BANDWIDTH_MAX,
		bandWidthQueryCommnad,
		5);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_BANDWIDTH_POS_CURRENT,tempParamInfo));
	//***end Frequency Commands***//

	//***Serial Number Commands***//
	wstring snQuery = L"";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_SERIALNUMBER,
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		snQuery,
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_SERIALNUMBER,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_SERIALNUMBER,
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		snQuery,
		1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_SERIALNUMBER,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_SERIALNUMBER,
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		snQuery,
		2);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_SERIALNUMBER,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_SERIALNUMBER,
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		snQuery,
		3);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_SERIALNUMBER,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT5_SERIALNUMBER,
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		snQuery,
		4);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_SERIALNUMBER,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT6_SERIALNUMBER,
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		snQuery,
		5);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_SERIALNUMBER,tempParamInfo));
	//***end Serial Number Commands***//

	//***Update Firmware Commands***//
	wstring firmwareUpdateCommand = L"FIRMWA:UP\n";
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_FIRMWAREUPDATE,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		firmwareUpdateCommand,
		0);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_FIRMWAREUPDATE,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_FIRMWAREUPDATE,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		firmwareUpdateCommand,
		1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_FIRMWAREUPDATE,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_FIRMWAREUPDATE,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		firmwareUpdateCommand,
		2);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_FIRMWAREUPDATE,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_FIRMWAREUPDATE,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		firmwareUpdateCommand,
		3);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_FIRMWAREUPDATE,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT5_FIRMWAREUPDATE,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		firmwareUpdateCommand,
		4);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_FIRMWAREUPDATE,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_PMT6_FIRMWAREUPDATE,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		firmwareUpdateCommand,
		5);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_FIRMWAREUPDATE,tempParamInfo));
	//***end Update Firmware Commands***//

	tempParamInfo = new ParamInfo(	
		PARAM_CONNECTION_STATUS,	//ID
		FALSE,						//VAL
		FALSE,						//PARAM C
		FALSE,						//PARAM B
		TYPE_LONG,					//TYPE
		TRUE,						//AVAILABLE
		TRUE,						//READ ONLY
		CONNECTION_READY,			//MIN
		CONNECTION_UNAVAILABLE,		//MAX
		CONNECTION_UNAVAILABLE,		//DEFAULT
		L"",						//Command
		-1);						//Device Index
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTION_STATUS, tempParamInfo));

	return TRUE;
}

/// <summary>
/// Destroys the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorPMT2100::DestroyParamTable()
{
	try
	{
		for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
		{
			delete iter->second;
		}
		_tableParams.clear();
	}
	catch(...)
	{
#ifdef LOGGING_ENABLED
		StringCbPrintfW(message,MSG_SIZE,L"ThorPMT2100 DestroyParamTable unable to destroy the table created on heap");
		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
#endif
		return FALSE;
	}

	return TRUE;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorPMT2100::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}