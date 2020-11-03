// ThorECU.cpp : Defines the exported functions for the DLL application.
//

//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorECU.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "ThorECU.h"
#include "ThorECUSetupXML.h"
#include "Strsafe.h"

#define READ_TIMEOUT 1000

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

/// <summary>
/// The message
/// </summary>
wchar_t message[MSG_SIZE];


/// <summary>
/// Prevents a default instance of the <see cref="ThorECU"/> class from being created.
/// </summary>
ThorECU::ThorECU() : 
	_pmt1GainMax_R(0), 
	_pmt2GainMax_R(0), 
	_pmt3GainMax_R(0), 
	_pmt4GainMax_R(0), 
	_scanAlignMax_R(0),
	NUM_TWOWAY_ZONES(251)
{
	_deviceDetected = FALSE;
#pragma warning(push)
#pragma warning(disable:4996)
	// create two way alignment data array and initialize it.
	_twoWayZones = new long[NUM_TWOWAY_ZONES];
	std::fill_n(_twoWayZones, NUM_TWOWAY_ZONES, 0);
	_twoWayZonesFine = new long[NUM_TWOWAY_ZONES];
	std::fill_n(_twoWayZonesFine, NUM_TWOWAY_ZONES, 0);
#pragma warning(pop)

	_pmtType[0] = 0;;
	for (long i = 0; i < PMT_NUM; i++)
	{
		_pmtType[i] = 1;
	}

	_dataBuffer[0] = NULL;
	_errMsg[0] = NULL;
	_r_crsEnable = FALSE;
	_r_crsZoom = 0;
	_r_pmt1Enable = FALSE;
	_r_pmt1Gain = PMT_GAIN_MIN;
	_r_pmt2Enable = FALSE;
	_r_pmt2Gain = PMT_GAIN_MIN;
	_r_pmt3Enable = FALSE;
	_r_pmt3Gain = PMT_GAIN_MIN;
	_r_pmt4Enable = FALSE;
	_r_pmt4Gain = PMT_GAIN_MIN;
	_r_pmtError = FALSE;
	_readBuffer[0] = NULL;
	_rsInitMode = FALSE;
	_scannerZoomMax_R = SCANNER_ZOOM_DEFAULT;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorECU"/> class.
/// </summary>
ThorECU::~ThorECU()
{
	_instanceFlag = false;

	delete[] _twoWayZones;
	delete[] _twoWayZonesFine;
}

CritSect ThorECU::_critSect;

bool ThorECU::_instanceFlag = false;

//The gain max counts value and gain power, and gain range will change based on the PMT sensor.
//(H9305-03) Low Sensitivity: GainCountsMax = 568, GainPower =7.29, GainMaxVolts = 1.0, GainMinVolts = 0.25
//(H7422P-40) High Sensitivity: GainCountsMax = 510, GainPower = 6.64, GainMaxVolts = 0.9, GainMinVolts = 0.5
const long ThorECU::_pmtGainCountsMax[PMT_TYPES_NUM] = {568, 510};
const double ThorECU::_pmtGainPower[PMT_TYPES_NUM] = {7.29, 6.64};
const double ThorECU::_pmtGainMaxVolts[PMT_TYPES_NUM] = {1.0, 0.9};
const double ThorECU::_pmtGainMinVolts[PMT_TYPES_NUM] = {0.25, 0.5};

auto_ptr<ThorECU> ThorECU::_single(new ThorECU());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorECU *.</returns>
ThorECU *ThorECU::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorECU());
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
long ThorECU::FindDevices(long &deviceCount)
{
	long	ret = TRUE;

	if(_deviceDetected)
	{
		_serialPort.Close();
		_deviceDetected = FALSE;
	}

	long portID=0;

	try
	{
		auto_ptr<ThorECUXML> pSetup(new ThorECUXML());

		pSetup->GetConnection(portID);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorECUSettings.xml file");
		deviceCount = 0;
		return FALSE;
	}

	TCHAR PortName[32];
	StringCbPrintf(PortName, 32, _T("COM%d"), portID);

	if(FALSE == _serialPort.Open(PortName))
	{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorECU FindDevices could not open serial port");
#endif
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to open com port %d",portID);

		// *TODO* perform an automatic scane of the available serial ports for the Z stepper
		deviceCount = 0;
		ret = FALSE;
	}
	else
	{
		deviceCount = 1;
		_deviceDetected = TRUE;
		_serialPort.Close();
	}

	return ret;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorECU::SelectDevice(const long device)
{
	long	ret = FALSE;

	if(FALSE == _deviceDetected)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"No devices found");
		return FALSE;
	}

	switch(device)
	{
	case 0:		
		{
			long portID=0;
			// resonance scanner initial mode

			long initMode = 0;
			_rsInitMode=0;
			try
			{
				auto_ptr<ThorECUXML> pSetup(new ThorECUXML());

				pSetup->GetConnection(portID);
				pSetup->GetConfiguration(initMode);	
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorECUSettings.xml file");
				return FALSE;
			}

			TCHAR PortName[32];
			StringCbPrintf(PortName, 32, _T("COM%d"), portID);

			if(FALSE == _serialPort.Open(PortName))
			{
#ifdef LOGGING_ENABLED
				logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorECU FindDevices could not open serial port");
#endif
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to open com port %d",portID);
			}

			//send Help command and parse the response for the max gain values for each PMT
			//Device is not a ECU if response is not in the right format
			if(TRUE == ReadHelpCommandSet())
			{
				CreateParamTable();	//build table for parameters
				CoarseAlignDataLoadFile(); //load data for two way coarse alignment if it exists
				FineAlignDataLoadFile();
				ret = TRUE;
			}

			if(1 == initMode)
			{		
				SetParam(IDevice::PARAM_SCANNER_INIT_MODE, 1);

				//turn the scanner on
				SetParam(IDevice::PARAM_SCANNER_ENABLE, 1);
				SetupPosition();
				StartPosition();

			}
			else if ( 0 == initMode)
			{
				SetParam(IDevice::PARAM_SCANNER_INIT_MODE, 0);

				//turn the scanner on
				SetParam(IDevice::PARAM_SCANNER_ENABLE, 0);
				SetupPosition();
				StartPosition();
				
			}
		}
		break;
	default:	{ }
	}

	return ret;
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorECU::TeardownDevice()
{
	double val = 0;
	
	GetParam(IDevice::PARAM_SCANNER_INIT_MODE, val);

	if(1 == static_cast<long>(val))
	{
		//Stop the resonance scanner if it is running
		SetParam(IDevice::PARAM_SCANNER_INIT_MODE, 0);
		SetParam(IDevice::PARAM_SCANNER_ENABLE, 1);
		SetParam(IDevice::PARAM_SCANNER_ENABLE, 0);		
	}

	//Disable the PMTs
	SetParam(IDevice::PARAM_PMT1_GAIN_POS, 0);
	SetParam(IDevice::PARAM_PMT2_GAIN_POS, 0);
	SetParam(IDevice::PARAM_PMT3_GAIN_POS, 0);
	SetParam(IDevice::PARAM_PMT4_GAIN_POS, 0);

	SetupPosition();
	StartPosition();

	DestroyParamTable();

	_serialPort.Close();

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
long ThorECU::GetParamInfo
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
	long parameterID = paramID;

	// update the _twoWayZones array and select the paramID to set the alignment

	if((paramID >= IDevice::PARAM_ECU_TWO_WAY_ZONE_1) && (paramID <= IDevice::PARAM_ECU_TWO_WAY_ZONE_251))
	{
		parameterID = IDevice::PARAM_SCANNER_ALIGN_POS;
	}

	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if((*iter)->GetParamID() == parameterID)
		{
			paramType = (*iter)->GetParamType();
			paramAvailable = (*iter)->GetParamAvailable();
			paramReadOnly = (*iter)->GetParamReadOnly();

			if((parameterID == PARAM_PMT1_GAIN_POS) || (parameterID == PARAM_PMT2_GAIN_POS) || 
				(parameterID == PARAM_PMT3_GAIN_POS) || (parameterID == PARAM_PMT4_GAIN_POS))
			{
				paramMin = PMT_GAIN_MIN;
				paramMax = PMT_GAIN_MAX;
				paramDefault = PMT_GAIN_MIN;
			}
			else if(parameterID == PARAM_SCANNER_ZOOM_POS)
			{
				paramMin = SCANNER_ZOOM_MIN;
				paramMax = SCANNER_ZOOM_MAX;
				paramDefault = SCANNER_ZOOM_DEFAULT;
			}
			else
			{
				paramMin = (*iter)->GetParamMin();
				paramMax = (*iter)->GetParamMax();
				paramDefault = (*iter)->GetParamDefault();
			}
			return TRUE;
		}
	}

	paramAvailable = FALSE;
	return FALSE;	
}

/// <summary>
/// Disables the PMT for.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <returns>long.</returns>
long ThorECU::DisablePMTFor(const long paramID)
{
	long ret = FALSE;

	ret = SetParam(_gainEnableMap.find(paramID)->second, 0);

	return ret;
}

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorECU::SetParam(const long paramID, const double param)
{
	double paramTemp = param;
	long parameterID = paramID;

	// update the _twoWayZones array and select the paramID to set the alignment
	if(((paramID % IDevice::PARAM_LAST_PARAM) >= IDevice::PARAM_ECU_TWO_WAY_ZONE_1) && ((paramID % IDevice::PARAM_LAST_PARAM) <= IDevice::PARAM_ECU_TWO_WAY_ZONE_251))
	{
		parameterID = IDevice::PARAM_SCANNER_ALIGN_POS;
	}

	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if((*iter)->GetParamID() == parameterID)
		{
			if(((*iter)->GetParamAvailable() == FALSE) || ((*iter)->GetParamReadOnly() == TRUE))
			{
				return FALSE;
			}

			if((parameterID == PARAM_PMT1_GAIN_POS) || (parameterID == PARAM_PMT2_GAIN_POS) || 
				(parameterID == PARAM_PMT3_GAIN_POS) || (parameterID == PARAM_PMT4_GAIN_POS))
			{
				if(param == 0)
				{
					paramTemp = 0;
					DisablePMTFor(parameterID); //diable PMTs when gain is requested to be set to 0
				}
				else
				{
					paramTemp = (double) GainPercent2Byte(paramID, static_cast<int>(param)); //conversion for gain values from percentage to real gain values
				}
			}
			else if(parameterID == PARAM_SCANNER_ZOOM_POS)
			{
				paramTemp = paramTemp * 4.0; //conversion for zoom/field size from 256 to 1024
			}

			if(((*iter)->GetParamMin() <= paramTemp) && ((*iter)->GetParamMax() >= paramTemp))
			{
				(*iter)->SetParamVal(paramTemp);
				_parameterMap[parameterID] = static_cast<long>(param);

				// update the _twoWayZones array
				if(parameterID == IDevice::PARAM_SCANNER_ALIGN_POS)
				{
					if(paramID > IDevice::PARAM_LAST_PARAM)
					{
						_twoWayZonesFine[paramID % IDevice::PARAM_LAST_PARAM - IDevice::PARAM_ECU_TWO_WAY_ZONE_1] = static_cast<long>(param);
					}
					else
					{
						_twoWayZones[paramID - IDevice::PARAM_ECU_TWO_WAY_ZONE_1] = static_cast<long>(param);
					}
				}

				// update scanner enable
				if(parameterID == IDevice::PARAM_SCANNER_ENABLE)
				{
					(*iter)->SetParamBool(TRUE);
				}

				switch (parameterID)
				{
					case PARAM_PMT1_TYPE: _pmtType[0]= static_cast<long>(paramTemp); break;
					case PARAM_PMT2_TYPE: _pmtType[1]= static_cast<long>(paramTemp); break;
					case PARAM_PMT3_TYPE: _pmtType[2]= static_cast<long>(paramTemp); break;
					case PARAM_PMT4_TYPE: _pmtType[3]= static_cast<long>(paramTemp); break;
				}

				return TRUE;
			}

			StringCbPrintfW(message,MSG_SIZE,L"ThorECU SetParam failed. paramID: %d", paramID);
			return FALSE;
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
long ThorECU::GetParam(const long paramID, double &param)
{
	if(paramID > IDevice::PARAM_LAST_PARAM)
	{
		if(((paramID % IDevice::PARAM_LAST_PARAM) >= IDevice::PARAM_ECU_TWO_WAY_ZONE_1) && ((paramID % IDevice::PARAM_LAST_PARAM) <= IDevice::PARAM_ECU_TWO_WAY_ZONE_251))
		{
			param = _twoWayZonesFine[paramID % IDevice::PARAM_LAST_PARAM - IDevice::PARAM_ECU_TWO_WAY_ZONE_1];
			return TRUE;
		}
	}
	// update the _twoWayZones array and select the paramID to set the alignment
	if((paramID >= IDevice::PARAM_ECU_TWO_WAY_ZONE_1) && (paramID <= IDevice::PARAM_ECU_TWO_WAY_ZONE_251))
	{
		param = _twoWayZones[paramID - IDevice::PARAM_ECU_TWO_WAY_ZONE_1];
		return TRUE;
	}

	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if((*iter)->GetParamID() == paramID)
		{
			if(((*iter)->GetParamAvailable() == FALSE))
			{
				return FALSE;
			}

			if((PARAM_PMT1_SAFETY == paramID) || (PARAM_PMT2_SAFETY == paramID) ||
				(PARAM_PMT3_SAFETY == paramID) || (PARAM_PMT4_SAFETY == paramID))
			{
				QueryPMTStatus(*iter);

				//if the pmt is not tripped
				if(0 == _r_pmtError)
				{
					param = 1;
				}
				else
				{
					param = 0;
				}
			}
			//else if((paramID == PARAM_PMT1_GAIN_POS) || (paramID == PARAM_PMT2_GAIN_POS) || 
			//	(paramID == PARAM_PMT3_GAIN_POS) || (paramID == PARAM_PMT4_GAIN_POS))
			//{
			//	long temp = static_cast<long>((*iter)->GetParamVal());
			//	param = static_cast<double>(GainByte2Percent(paramID, temp));
			//}
			else if(paramID == PARAM_CONNECTION_STATUS)
			{
				param = (_deviceDetected) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
			}
			else
			{
				//param = static_cast<double>((*iter)->GetParamVal());
				param = static_cast<double>(_parameterMap.find(paramID)->second);
			}
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
long ThorECU::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorECU::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorECU::SetParamString(const long paramID, wchar_t* str)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Gets the parameter of type string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorECU::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorECU::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorECU::SetupPosition()
{
	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorECU::StartPosition()
{

	long ret = TRUE;

	//iterate through map and set the parameters in reverse order because we want the position to be set at last
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if(((*iter)->GetParamAvailable() == FALSE) || ((*iter)->GetParamReadOnly() == TRUE))
		{
			continue;
		}
		else if((*iter)->GetParamBool() == TRUE)
		{	
			ret = ExecuteCmdStr(*iter) > 0 ? TRUE : FALSE; //no need to parse read back
			(*iter)->UpdateParam_C();

			//ret = TRUE;
			(*iter)->SetParamBool(FALSE);

			StringCbPrintfW(message,MSG_SIZE,L"StartPosition succeeded at paramID: %d",(*iter)->GetParamID());
		}
	}

	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorECU::StatusPosition(long &status)
{
	long	ret = TRUE;
	status = IDevice::STATUS_READY;
	return ret;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorECU::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	if(deviceType == PMT1)
	{
		pos = static_cast<double>(_parameterMap.find(PARAM_PMT1_GAIN_POS)->second);
	}
	else if(deviceType == PMT2)
	{
		pos = static_cast<double>(_parameterMap.find(PARAM_PMT2_GAIN_POS)->second);
	}	
	else if(deviceType == PMT3)
	{
		pos = static_cast<double>(_parameterMap.find(PARAM_PMT3_GAIN_POS)->second);
	}
	else if(deviceType == PMT4)
	{
		pos = static_cast<double>(_parameterMap.find(PARAM_PMT4_GAIN_POS)->second);
	}	
	else
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorECU Invalid device specified for reading");
	}

	return ret;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorECU::PostflightPosition()
{
	return TRUE;
}


/// <summary>
/// Queries the PMT status.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorECU::QueryPMTStatus(ParamInfo * pParamInfo)
{
	bool ret;

	TCHAR inst[32];
	StringCbPrintf(inst, 32, _T("%ls"), pParamInfo->GetCmdStr().c_str());

	Lock lock(_critSect);

	if (ret=(TRUE==_serialPort.Write(inst)))
	{
		//#ifdef LOGGING_ENABLED
		//		StringCbPrintfW(message,MSG_SIZE,L"ThorECU Write inst =  %ls", inst);
		//		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		//#endif

		ret=(TRUE==ReadECUPort(500));
		memset(_dataBuffer, 0, READ_BUFFER_SIZE);
		StringCchCat(_dataBuffer,READ_BUFFER_SIZE,inst); //add 
		StringCchCat(_dataBuffer,READ_BUFFER_SIZE,_readBuffer);

		ret=(TRUE==ParseReadBufferStatus(PMTSTATUS));

	}

	return TRUE;
}

/// <summary>
/// Executes the command string.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorECU::ExecuteCmdStr(ParamInfo* pParamInfo)
{
	long ret = TRUE;
	TCHAR inst[32];
	StringCbPrintf(inst, 32, _T("%ls"), pParamInfo->GetCmdStr().c_str());
	

	//when the scanner init mode is enabled block commands trying to 
	//change the state of the scanner
	double val = 0;
	
	GetParam(IDevice::PARAM_SCANNER_INIT_MODE, val);

	if((pParamInfo->GetParamID()==PARAM_SCANNER_ENABLE) && (static_cast<long>(pParamInfo->GetParamVal()) == 0) && (1 == static_cast<long>(val)))
	{		
		return TRUE;
	}

	Lock lock(_critSect);

	if (ret=_serialPort.Write(inst))
	{
		ret=ReadECUPort(100);
		StringCchCat(_dataBuffer,READ_BUFFER_SIZE, _readBuffer);
	}
	return ret;
}


/// <summary>
/// Reads the ecu port.
/// </summary>
/// <param name="timeout">The timeout.</param>
/// <returns>long.</returns>
long ThorECU::ReadECUPort(long timeout)
{	
	//wait for 10ms so that we do not get any echo
	Sleep(10); 
	memset(_readBuffer, 0, READ_BUFFER_SIZE);
	long ret = _serialPort.Read(_readBuffer, timeout);

	return ret;
}

/// <summary>
/// Parses the read buffer status.
/// </summary>
/// <param name="inquiryID">The inquiry identifier.</param>
/// <returns>long.</returns>
long ThorECU::ParseReadBufferStatus(int inquiryID)
{
	if((_dataBuffer[0] == 0) || (_dataBuffer[1] == 0))	//return if readBuffer is empty
		return FALSE;

	long ret = FALSE;
	int whichPmt = 0;
	int echoPmt = 0;
	bool pmtEnable = false;
	int gain = -1;
	TCHAR pmterr[64];
	TCHAR pelerr[64];
	bool pmtError = false;
	TCHAR tempBuffer[4096];
	tempBuffer[0] = 0;

	pmterr[0] = 0;
	pelerr[0] = 0;

	// get which pmt
	ret = swscanf_s(_dataBuffer, L"pmt%d?pmt%d?%*[ ]%lld", &echoPmt, &whichPmt, tempBuffer, _countof(tempBuffer));
	if(ret < 2)
	{
		StringCbPrintfW(message,MSG_SIZE,L"ThorECU ParseReadBufferStatus _dataBuffer not in correct format = %ls", _dataBuffer);
		LogMessage(message);
		return FALSE;
	}

	//get pmt enable from ECU
	if(wcsstr(_dataBuffer,_T("On")) > 0)
	{
		pmtEnable=true;
	}
	else if(wcsstr(_dataBuffer,_T("Off")) > 0)
	{
		pmtEnable=false;
	}
	else
	{
		StringCbPrintfW(message,MSG_SIZE,L"ThorECU ParseReadBufferStatus _dataBuffer not in correct format = %ls", _dataBuffer);
		LogMessage(message);
		return FALSE;
	}

	//match the response starting from "Gain:"
	TCHAR* pInterest = wcsstr(_dataBuffer, _T("Gain:"));
	if(0 == pInterest)
	{
		StringCbPrintfW(message,MSG_SIZE,L"ThorECU ParseReadBufferStatus _dataBuffer not in correct format = %ls", _dataBuffer);
		LogMessage(message);
		return FALSE;
	}

	ret = swscanf_s(pInterest, _T("Gain: %d PMT: %ls Peltier: %lld"), &gain, pmterr, _countof(pmterr), pelerr, _countof(pelerr));

	if(wcsstr(pmterr,_T("OK")) && (3 == ret))	//pmt normal
	{
		pmtError = false;
	}
	else
	{
		pmterr[0] = 0;
		pelerr[0] = 0;
		gain = -1;
		ret = swscanf_s(pInterest, _T("Gain: %d PMT: PMT %ls Peltier: %lld"), &gain, pmterr, _countof(pmterr), pelerr, _countof(pelerr));

		if(wcsstr(pmterr,_T("ERROR")))	//pmt tripped
		{
			pmtError = true;
		}
		else	//pmt response incorrect
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorECU ParseReadBufferStatus _dataBuffer not in correct format = %ls", _dataBuffer);
			LogMessage(message);
			return FALSE;
		}
	}

	StringCbPrintfW(message,MSG_SIZE,L"ThorECU ParseReadBufferStatus pmt%d enable=%d, gain=%d, pmterr=%ls, pelerr=%ls", whichPmt, pmtEnable,gain,pmterr,pelerr);
	LogMessage(message);

	switch (inquiryID)
	{
	case PMT1GAIN:
		_r_pmt1Gain=gain;
		break;
	case PMT2GAIN:
		_r_pmt2Gain=gain;
		break;
	case PMT3GAIN:
		_r_pmt3Gain=gain;
		break;
	case PMT4GAIN:
		_r_pmt4Gain=gain;
		break;
	case PMT1ENABLE:
		_r_pmt1Enable = pmtEnable;
		break;
	case PMT2ENABLE:
		_r_pmt2Enable = pmtEnable;
		break;
	case PMT3ENABLE:
		_r_pmt3Enable = pmtEnable;
		break;
	case PMT4ENABLE:
		_r_pmt4Enable = pmtEnable;
		break;
	case PMTSTATUS:
		_r_pmtError = pmtError;

		break;
	}
	return TRUE;
}

// create entry for the given key and value if not exist already, 
// otherwise, update the value
/// <summary>
/// Updates the parameter map entry.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="paramValue">The parameter value.</param>
/// <returns>long.</returns>
long ThorECU::UpdateParameterMapEntry(long paramID, long paramValue)
{
	_parameterMap[paramID] = paramValue;

	return TRUE;
}

/// <summary>
/// Parses the help response.
/// </summary>
/// <returns>long.</returns>
long ThorECU::ParseHelpResponse()
{
	wchar_t *found;
	wchar_t *pEnd;
	found = wcsstr(_dataBuffer, _T("pmt1gain=<0..."));
	//Device is not ECU if response does not contain specified string
	if(found)
	{	
		pEnd = found + wcslen(_T("pmt1gain=<0..."));
		_pmt1GainMax_R = wcstol(pEnd, &pEnd, 10);
		UpdateParameterMapEntry(IDevice::PARAM_PMT1_GAIN_POS, 0);
	}
	else
	{
		return FALSE;
	}

	found = wcsstr(_dataBuffer, _T("pmt2gain=<0..."));
	if(found)
	{
		pEnd = found + wcslen(_T("pmt2gain=<0..."));
		_pmt2GainMax_R = wcstol(pEnd, &pEnd, 10);
		UpdateParameterMapEntry(IDevice::PARAM_PMT2_GAIN_POS, 0);
	}

	found = wcsstr(_dataBuffer, _T("pmt3gain=<0..."));
	if(found)
	{
		pEnd = found + wcslen(_T("pmt3gain=<0..."));
		_pmt3GainMax_R = wcstol(pEnd, &pEnd, 10);
		UpdateParameterMapEntry(IDevice::PARAM_PMT3_GAIN_POS, 0);
	}

	found = wcsstr(_dataBuffer, _T("pmt4gain=<0..."));
	if(found)
	{
		pEnd = found + wcslen(_T("pmt4gain=<0..."));
		_pmt4GainMax_R = wcstol(pEnd, &pEnd, 10);
		UpdateParameterMapEntry(IDevice::PARAM_PMT4_GAIN_POS, 0);
	}

	found = wcsstr(_dataBuffer, _T("zoom=<0.."));
	if(found)
	{
		pEnd = found + wcslen(_T("zoom=<0.."));
		_scannerZoomMax_R = wcstol(pEnd, &pEnd, 10);
		UpdateParameterMapEntry(IDevice::PARAM_SCANNER_ZOOM_POS, 0);
	}

	found = wcsstr(_dataBuffer, _T("scanalign=<0.."));
	if(found)
	{
		pEnd = found + wcslen(_T("scanalign=<0.."));
		_scanAlignMax_R = wcstol(pEnd, &pEnd, 10);
		UpdateParameterMapEntry(IDevice::PARAM_SCANNER_ALIGN_POS, 0);
	}

#ifdef LOGGING_ENABLED
	StringCbPrintfW(message,MSG_SIZE,L"ThorECU ParseHelpResponse _pmt1GainMax_R=%d,_pmt2GainMax_R=%d,_pmt3GainMax_R=%d,_pmt4GainMax_R=%d", _pmt1GainMax_R,_pmt2GainMax_R,_pmt3GainMax_R,_pmt4GainMax_R);
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
#endif

	return TRUE;
}

/// <summary>
/// Reads the help command set.
/// </summary>
/// <returns>long.</returns>
long ThorECU::ReadHelpCommandSet()
{
	bool ret;
	TCHAR inst[32];
	memset(_dataBuffer, 0, READ_BUFFER_SIZE);

	StringCbPrintf(inst, 32, _T("help?"));

	Lock lock(_critSect);

	if (ret=(TRUE==_serialPort.Write(inst)))
	{
		Sleep(10);
		do 
		{
			ret=(TRUE==ReadECUPort(500));
			StringCchCat(_dataBuffer,READ_BUFFER_SIZE,_readBuffer);
		} while (_readBuffer[0] != 0);
	}
	//#ifdef LOGGING_ENABLED
	//		StringCbPrintfW(message,MSG_SIZE,L"ThorECU help response =  %ls", _dataBuffer);
	//		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
	//#endif
	if(ret=(TRUE==ParseHelpResponse()))
	{
		memset(_dataBuffer, 0, READ_BUFFER_SIZE);
	}

	return ret;
}

/// <summary>
/// Percent to gain byte.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="percentageGain">The percentage gain.</param>
/// <returns>long.</returns>
long ThorECU::GainPercent2Byte(int paramID, int percentageGain)
{
	if(percentageGain == 0) //force the gain to be zero
		return 0;

	long pmtIndex = 0;
	switch(paramID)
	{
	case PARAM_PMT1_GAIN_POS: pmtIndex = 0;	break;
	case PARAM_PMT2_GAIN_POS: pmtIndex = 1;	break;
	case PARAM_PMT3_GAIN_POS: pmtIndex = 2;	break;
	case PARAM_PMT4_GAIN_POS: pmtIndex = 3; break;
	}
	long pmtType = _pmtType[pmtIndex];
	const double GAINMAX = _pmtGainCountsMax[pmtType];

	if(GAINMAX == 0)
		return 0;
	const double GAINPOWER = _pmtGainPower[pmtType];
	const double GAINHBOUND = pow(_pmtGainMaxVolts[pmtType],GAINPOWER);
	const double GAINLBOUND = pow(_pmtGainMinVolts[pmtType],GAINPOWER);
	
	//linearization changes the lower bound to be at a digital value of 50 that is sent to the controller
	//reference u1 = V^(alpha*n)
	double actualGain = static_cast<double>(percentageGain)/100.0*(GAINHBOUND-GAINLBOUND)+GAINLBOUND;
	double bytegain = pow(actualGain/GAINHBOUND,1/GAINPOWER)*GAINMAX;
	return static_cast<long>(ceil(bytegain - 0.5));  
}


/// <summary>
/// Converts Gain to percent.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="byteGain">The byte gain.</param>
/// <returns>long.</returns>
long ThorECU::GainByte2Percent(int paramID, int byteGain)
{
	long pmtIndex = 0;
	switch(paramID)
	{
	case PARAM_PMT1_GAIN_POS: pmtIndex = 0;	break;
	case PARAM_PMT2_GAIN_POS: pmtIndex = 1;	break;
	case PARAM_PMT3_GAIN_POS: pmtIndex = 2;	break;
	case PARAM_PMT4_GAIN_POS: pmtIndex = 3; break;
	}

	long pmtType = _pmtType[pmtIndex];
	const double GAINMAX = _pmtGainCountsMax[pmtType];

	if(GAINMAX == 0)
		return 0;
	const double GAINPOWER = _pmtGainPower[pmtType];
	const double GAINHBOUND = pow(_pmtGainMaxVolts[pmtType],GAINPOWER);
	const double GAINLBOUND = pow(_pmtGainMinVolts[pmtType],GAINPOWER);

	if(byteGain == 0)
		return 0;
	double percentageGain=(GAINHBOUND * pow(static_cast<double>(byteGain)/GAINMAX, GAINPOWER)-GAINLBOUND)/(GAINHBOUND-GAINLBOUND)*100.0;
	return static_cast<long>(ceil(percentageGain - 0.5));
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorECU::GetLastErrorMsg(wchar_t *msg, long size)
{	
	wcsncpy_s(msg,MSG_SIZE,_errMsg,MSG_SIZE);
	return TRUE;
}

/// <summary>
/// Creates the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorECU::CreateParamTable()
{
	TCHAR wsCommand[32];
	memset(wsCommand, 0, 32);

	ParamInfo* tempParamInfo = new ParamInfo(
		PARAM_DEVICE_TYPE,
		PMT1 | PMT2 | PMT3 | PMT4 | CONTROL_UNIT,
		PMT1 | PMT2 | PMT3 | PMT4 | CONTROL_UNIT,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		PMT1 | PMT2 | PMT3 | PMT4 | CONTROL_UNIT,
		PMT1 | PMT2 | PMT3 | PMT4 | CONTROL_UNIT,
		PMT1 | PMT2 | PMT3 | PMT4 | CONTROL_UNIT,
		L""
		);
	_tableParams.push_back(tempParamInfo);
	UpdateParameterMapEntry(PARAM_DEVICE_TYPE, PMT1 | PMT2 | PMT3 | PMT4 | CONTROL_UNIT);

	StringCbPrintf(wsCommand, 32, _T("pmt1gain=%d"), GainByte2Percent(PARAM_PMT1_GAIN_POS, PMT_GAIN_DEFAULT));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_GAIN_POS,
		PMT_GAIN_MIN,
		PMT_GAIN_MIN,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		PMT_GAIN_MIN,
		_pmt1GainMax_R,
		PMT_GAIN_MIN,
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT1_GAIN_POS, PMT_GAIN_DEFAULT);
	_gainEnableMap.insert(pair<long, long>(PARAM_PMT1_GAIN_POS, Params::PARAM_PMT1_ENABLE));

	StringCbPrintf(wsCommand, 32, _T("pmt2gain=%d"), GainByte2Percent(PARAM_PMT2_GAIN_POS, PMT_GAIN_DEFAULT));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_GAIN_POS,
		PMT_GAIN_MIN,
		PMT_GAIN_MIN,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		PMT_GAIN_MIN,
		_pmt2GainMax_R,
		PMT_GAIN_MIN,
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT2_GAIN_POS, PMT_GAIN_DEFAULT);
	_gainEnableMap.insert(pair<long, long>(PARAM_PMT2_GAIN_POS, PARAM_PMT2_ENABLE));

	StringCbPrintf(wsCommand, 32, _T("pmt3gain=%d"), GainByte2Percent(PARAM_PMT3_GAIN_POS, PMT_GAIN_DEFAULT));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_GAIN_POS,
		PMT_GAIN_MIN,
		PMT_GAIN_MIN,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		PMT_GAIN_MIN,
		_pmt3GainMax_R,
		PMT_GAIN_MIN,
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT3_GAIN_POS, PMT_GAIN_DEFAULT);
	_gainEnableMap.insert(pair<long, long>(PARAM_PMT3_GAIN_POS, PARAM_PMT3_ENABLE));

	StringCbPrintf(wsCommand, 32, _T("pmt4gain=%d"), GainByte2Percent(PARAM_PMT4_GAIN_POS, PMT_GAIN_DEFAULT));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_GAIN_POS,
		PMT_GAIN_MIN,
		PMT_GAIN_MIN,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		PMT_GAIN_MIN,
		_pmt4GainMax_R,
		PMT_GAIN_MIN,
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT4_GAIN_POS, PMT_GAIN_DEFAULT);
	_gainEnableMap.insert(pair<long, long>(PARAM_PMT4_GAIN_POS, PARAM_PMT4_ENABLE));

	StringCbPrintf(wsCommand, 32, _T("pmt1=%d"), 0);
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
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT1_ENABLE, 0);

	StringCbPrintf(wsCommand, 32, _T("pmt2=%d"), 0);
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
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT2_ENABLE, 0);

	StringCbPrintf(wsCommand, 32, _T("pmt3=%d"), 0);
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
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT3_ENABLE, 0);

	StringCbPrintf(wsCommand, 32, _T("pmt4=%d"), 0);
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
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT4_ENABLE, 0);

	StringCbPrintf(wsCommand, 32, _T("pmt1?"));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_SAFETY,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT1_SAFETY, 0);

	StringCbPrintf(wsCommand, 32, _T("pmt2?"));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_SAFETY,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT2_SAFETY, 0);

	StringCbPrintf(wsCommand, 32, _T("pmt3?"));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_SAFETY,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT3_SAFETY, 0);

	StringCbPrintf(wsCommand, 32, _T("pmt4?"));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_SAFETY,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT4_SAFETY, 0);

	StringCbPrintf(wsCommand, 32, _T(""), 0);
	tempParamInfo = new ParamInfo(	
					PARAM_PMT1_TYPE,
					0,
					0,
					FALSE,
					TYPE_LONG,
					TRUE,
					FALSE,
					0,
					1,
					0,
					wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT1_TYPE, 0);

	StringCbPrintf(wsCommand, 32, _T(""), 0);
	tempParamInfo = new ParamInfo(	
					PARAM_PMT2_TYPE,
					0,
					0,
					FALSE,
					TYPE_LONG,
					TRUE,
					FALSE,
					0,
					1,
					0,
					wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT2_TYPE, 0);

	StringCbPrintf(wsCommand, 32, _T(""), 0);
	tempParamInfo = new ParamInfo(	
					PARAM_PMT3_TYPE,
					0,
					0,
					FALSE,
					TYPE_LONG,
					TRUE,
					FALSE,
					0,
					1,
					0,
					wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT3_TYPE, 0);

	StringCbPrintf(wsCommand, 32, _T(""), 0);
	tempParamInfo = new ParamInfo(	
					PARAM_PMT4_TYPE,
					0,
					0,
					FALSE,
					TYPE_LONG,
					TRUE,
					FALSE,
					0,
					1,
					0,
					wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_PMT4_TYPE, 0);

	StringCbPrintf(wsCommand, 32, _T("scan=%d"), 0);
	tempParamInfo = new ParamInfo(	
		PARAM_SCANNER_ENABLE,
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_SCANNER_ENABLE, 0);

	StringCbPrintf(wsCommand, 32, _T("zoom=%d"), SCANNER_ZOOM_DEFAULT*4);
	tempParamInfo = new ParamInfo(	
		PARAM_SCANNER_ZOOM_POS,
		SCANNER_ZOOM_DEFAULT,
		SCANNER_ZOOM_DEFAULT,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		(double)SCANNER_ZOOM_MIN * 4,
		_scannerZoomMax_R,
		SCANNER_ZOOM_DEFAULT,
		wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_SCANNER_ZOOM_POS, SCANNER_ZOOM_DEFAULT);
		
	StringCbPrintf(wsCommand, 32, _T(""), 0);
	tempParamInfo = new ParamInfo(	
					PARAM_SCANNER_INIT_MODE,
					0,
					0,
					FALSE,
					TYPE_LONG,
					TRUE,
					FALSE,
					0,
					1,
					0,
					wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_SCANNER_INIT_MODE, 0);

	// create this ParamInfo only when it is supported by hardware
	if(1 == _parameterMap.count(PARAM_SCANNER_ALIGN_POS))
	{
		StringCbPrintf(wsCommand, 32, _T("scanalign=%d"), 0);
		tempParamInfo = new ParamInfo(	
			PARAM_SCANNER_ALIGN_POS,
			0,
			0,
			FALSE,
			TYPE_LONG,
			TRUE,
			FALSE,
			0,
			_scanAlignMax_R,
			128,
			wstring(wsCommand));
		_tableParams.push_back(tempParamInfo);
		memset(wsCommand, 0, 32);
		UpdateParameterMapEntry(PARAM_SCANNER_ALIGN_POS, 128);
	}

	StringCbPrintf(wsCommand, 32, _T(""), 0);
	tempParamInfo = new ParamInfo(	
					PARAM_CONNECTION_STATUS,
					0,
					0,
					FALSE,
					TYPE_LONG,
					TRUE,
					TRUE,
					(double)ConnectionStatusType::CONNECTION_READY,
					(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,
					(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,
					wstring(wsCommand));
	_tableParams.push_back(tempParamInfo);
	memset(wsCommand, 0, 32);
	UpdateParameterMapEntry(PARAM_CONNECTION_STATUS, 0);

	return TRUE;
}

/// <summary>
/// Destroys the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorECU::DestroyParamTable()
{
	try
	{
		for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
		{
			delete *iter;
		}
		_tableParams.clear();
	}
	catch(...)
	{
#ifdef LOGGING_ENABLED
		StringCbPrintfW(message,MSG_SIZE,L"ThorECU DestroyParamTable unable to destroy the table created on heap");
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
void ThorECU::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Loades in the coarse alignment file.
/// </summary>
/// <returns>long.</returns>
long ThorECU::CoarseAlignDataLoadFile()
{
	char appPath[256]; ///<the char of application path

	char filePath[256]; ///<char of the whole file path
	_getcwd(appPath, 256);
	strcpy_s(filePath, appPath);
	strcat_s(filePath, "\\AlignDataCoarse.txt");

	FILE *AlignDataFile;
	if (fopen_s(&AlignDataFile, filePath, "r") != 0)
	{
	}
	else
	{
		const long MIN_COARSE_ALIGNMENT = 0;
		const long MAX_COARSE_ALIGNMENT = 255;
		for (int i = 0; i < 256; i++)
		{
			long lVal = 0;
			if (fscanf_s(AlignDataFile, "%d", &lVal) == EOF)
			{
			}
			else if (lVal < MIN_COARSE_ALIGNMENT)
			{
				lVal = MIN_COARSE_ALIGNMENT;
			}
			else if (lVal > MAX_COARSE_ALIGNMENT)
			{
				lVal = MAX_COARSE_ALIGNMENT;
			}
			if(i >= 5)
			{
				_twoWayZones[255-i] = lVal;
			}
		}
		fclose(AlignDataFile);
	}
	return TRUE;
}

long ThorECU::FineAlignDataLoadFile()
{
	char appPath[256]; ///<the char of application path

	char filePath[256]; ///<char of the whole file path
	_getcwd(appPath, 256);
	strcpy_s(filePath, appPath);
	strcat_s(filePath, "\\AlignData.txt");

	FILE *AlignDataFile;
	if (fopen_s(&AlignDataFile, filePath, "r") != 0)
	{
	}
	else
	{
		const long MIN_FINE_ALIGNMENT = 0;
		const long MAX_FINE_ALIGNMENT = 255;
		for (int i = 0; i < 256; i++)
		{
			long lVal = 0;
			if (fscanf_s(AlignDataFile, "%d", &lVal) == EOF)
			{
			}
			else if (lVal < MIN_FINE_ALIGNMENT)
			{
				lVal = MIN_FINE_ALIGNMENT;
			}
			else if (lVal > MAX_FINE_ALIGNMENT)
			{
				lVal = MAX_FINE_ALIGNMENT;
			}
			if(i >= 5)
			{
				_twoWayZonesFine[255-i] = lVal;
			}
		}
		fclose(AlignDataFile);
	}
	return TRUE;
}