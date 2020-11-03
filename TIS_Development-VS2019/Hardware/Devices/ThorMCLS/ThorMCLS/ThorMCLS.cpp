// ThorMCLS.cpp : Defines the exported functions for the DLL application.
//

//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorMCLS.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "ThorMCLS.h"
#include "ThorMCLSSetupXML.h"
#include "Strsafe.h"
#include <fstream>


#define READ_TIMEOUT 1000

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

wchar_t message[MSG_SIZE];


ThorMCLS::ThorMCLS()
{

	_laser1Min=0;
	_laser1Max=0;
	_laser2Min=0;
	_laser2Max=0;
	_laser3Min=0;
	_laser3Max=0;
	_laser4Min=0;
	_laser4Max=0;

	_laser1Enable=FALSE;
	_laser2Enable=FALSE;
	_laser3Enable=FALSE;
	_laser4Enable=FALSE;
	_laser1Power=_laser1Min;
	_laser2Power=_laser2Min;
	_laser3Power=_laser3Min;
	_laser4Power=_laser4Min;

	_laser1Enable_C=FALSE;
	_laser2Enable_C=FALSE;
	_laser3Enable_C=FALSE;
	_laser4Enable_C=FALSE;
	_laser1Power_C=_laser1Min-1;
	_laser2Power_C=_laser2Min-1;
	_laser3Power_C=_laser3Min-1;
	_laser4Power_C=_laser4Min-1;

	_deviceDetected = FALSE;

	_bEnableSet = FALSE;

	for(InterpolationTable t : linearizationTables)
	{
		t.setEdgeBehavior(InterpolationTable::RETURN_BOUNDS);
	}

	_errMsg[0] = NULL;
	_dataBuffer[0] = NULL;
	_readBuffer[0] = NULL;
}

ThorMCLS::~ThorMCLS()
{
	_instanceFlag = false;
}

bool ThorMCLS:: _instanceFlag = false;

auto_ptr<ThorMCLS> ThorMCLS::_single(new ThorMCLS());

ThorMCLS *ThorMCLS::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorMCLS());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorMCLS::FindDevices(long &deviceCount)
{
	long	ret = TRUE;

	if(_deviceDetected)
	{
		_serialPort.Close();
		_deviceDetected = FALSE;
	}

	deviceCount = 0;

	long portID=0;

	try
	{
		auto_ptr<ThorMCLSXML> pSetup(new ThorMCLSXML());

		pSetup->GetConnection(portID);

		TCHAR PortName[32];
		StringCbPrintfW(PortName,32, _T("COM%d"), portID);

		if(FALSE == _serialPort.Open(PortName))
		{
#ifdef LOGGING_ENABLED
			logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorMCLS FindDevices could not open serial port");
#endif
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to open com port %d",portID);
			ret = FALSE;

			// *TODO* perform an automatic scane of the available serial ports for the Z stepper
		}
		else
		{
			if(QueryID())
			{
				deviceCount = 1;
				_deviceDetected = TRUE;
			}
			_serialPort.Close();

		}
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorMCLSSettings.xml file");
		deviceCount = 0;
		return FALSE;
	}

	return ret;
}

void ThorMCLS::GetMinMaxFromReadBuffer(double &minVal, double &maxVal)
{
	const std::regex pattern("(.*)\r\nWavelength = (.*)\r\nPOut = (.*)\r\nIOp = (.*)\r\nIMon = (.*)\r\nITh = (.*)\r\nSerialNumber = (.*)\r\n(.*)");

	string s = ConvertWStringToString(_readBuffer);

	std::match_results<std::string::const_iterator> result;
	std::regex_match(s, result, pattern);
	if(result.size() >= 7)
	{

		stringstream ssIop(result[4]);
		ssIop>>maxVal;
		stringstream ssIth(result[6]);
		ssIth>>minVal;	
	}
	else
	{
		//try the pattern from a previous firmware		
		const std::regex pattern2("(.*)\r\nWavelLength = (.*)\r\nPOut = (.*)\r\nIOp = (.*)\r\nIMon = (.*)\r\nITh = (.*)\r\nSerialNumber = (.*)\r\n(.*)");

		string s2 = ConvertWStringToString(_readBuffer);

		std::regex_match(s2, result, pattern2);

		if(result.size() >= 7)
		{

			stringstream ssIop(result[4]);
			ssIop>>maxVal;
			stringstream ssIth(result[6]);
			ssIth>>minVal;	
		}
	}

	//*TODO* the firmware for the MCLS has a bug where using the 
	//maximum value will disable the laser. To prevent this 
	//from happening. Restrict the upper limit of the laser
	//range to 98% of the total range.
	if(maxVal > minVal)
	{
		maxVal = minVal + .98 * (maxVal - minVal);
	}
}

long ThorMCLS::SelectDevice(const long device)
{
	long	ret = FALSE;

	if(FALSE == _deviceDetected)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"No devices found");
		return FALSE;
	}

	long portID=0;

	try
	{
		auto_ptr<ThorMCLSXML> pSetup(new ThorMCLSXML());

		pSetup->GetConnection(portID);

		TCHAR PortName[32];
		StringCbPrintfW(PortName,32, _T("COM%d"), portID);

		if(FALSE == _serialPort.Open(PortName))
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorMCLS unable to open port %d",portID);
			return FALSE;
		}

		TCHAR inst[32];
		StringCbPrintfW(inst,32, _T("channel=1"));
		if (ret=_serialPort.Write(inst))
			ret=ReadMCLSPort();

		StringCbPrintfW(inst,32, _T("specs?"));
		if (ret=_serialPort.Write(inst))
			ret=ReadMCLSPort();

		GetMinMaxFromReadBuffer(_laser1Min, _laser1Max);

		StringCbPrintfW(inst,32, _T("channel=2"));
		if (ret=_serialPort.Write(inst))
			ret=ReadMCLSPort();

		StringCbPrintfW(inst,32, _T("specs?"));
		if (ret=_serialPort.Write(inst))
			ret=ReadMCLSPort();

		GetMinMaxFromReadBuffer(_laser2Min, _laser2Max);

		StringCbPrintfW(inst,32, _T("channel=3"));
		if (ret=_serialPort.Write(inst))
			ret=ReadMCLSPort();

		StringCbPrintfW(inst,32, _T("specs?"));
		if (ret=_serialPort.Write(inst))
			ret=ReadMCLSPort();

		GetMinMaxFromReadBuffer(_laser3Min, _laser3Max);

		StringCbPrintfW(inst,32, _T("channel=4"));
		if (ret=_serialPort.Write(inst))
			ret=ReadMCLSPort();

		StringCbPrintfW(inst,32, _T("specs?"));
		if (ret=_serialPort.Write(inst))
			ret=ReadMCLSPort();

		GetMinMaxFromReadBuffer(_laser4Min, _laser4Max);

		SetSystemEnable(1);

		switch(device)
		{
		case 0:		{ ret = TRUE; }break;
		default:	{ }
		}

		if (TRUE == ret){
			CalibrateMCLS();
		}
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorMCLSSettings.xml file");
		return FALSE;
	}

	return ret;
}

long ThorMCLS::TeardownDevice()
{
	try
	{
		//Turn off all lasers when the device is released
		if (TRUE == SetMCLSLaserEnable(1, 0))
		{
			_laser1Enable_C = 0;
		}
		if (TRUE == SetMCLSLaserEnable(2, 0))
		{
			_laser2Enable_C = 0;
		}
		if (TRUE == SetMCLSLaserEnable(3, 0))
		{
			_laser3Enable_C = 0;
		}
		if (TRUE == SetMCLSLaserEnable(4, 0))
		{
			_laser4Enable_C = 0;
		}
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error at teardown of ThorMCLS.");
	}
	_serialPort.Close();
	return TRUE;
}

long ThorMCLS::GetParamInfo
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
	case PARAM_LASER1_POWER:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _laser1Min;
			paramMax = _laser1Max;
			paramDefault = _laser1Min;
		}
		break;

	case PARAM_LASER2_POWER:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _laser2Min;
			paramMax = _laser2Max;
			paramDefault = _laser2Min;
		}
		break;
	case PARAM_LASER3_POWER:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _laser3Min;
			paramMax = _laser3Max;
			paramDefault = _laser3Min;
		}
		break;
	case PARAM_LASER4_POWER:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _laser4Min;
			paramMax = _laser4Max;
			paramDefault = _laser4Min;
		}
		break;

	case PARAM_LASER1_POWER_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _laser1Min;
			paramMax = _laser1Max;
			paramDefault = _laser1Min;
		}
		break;

	case PARAM_LASER2_POWER_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _laser1Min;
			paramMax = _laser1Max;
			paramDefault = _laser1Min;
		}
		break;

	case PARAM_LASER3_POWER_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _laser1Min;
			paramMax = _laser1Max;
			paramDefault = _laser1Min;
		}
		break;

	case PARAM_LASER4_POWER_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _laser1Min;
			paramMax = _laser1Max;
			paramDefault = _laser1Min;
		}
		break;

	case PARAM_LASER1_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;

	case PARAM_LASER2_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;

	case PARAM_LASER3_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;

	case PARAM_LASER4_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;

	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = LASER1|LASER2|LASER3|LASER4;
		}
		break;

	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = (double)ConnectionStatusType::CONNECTION_READY;
			paramMax = paramDefault = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
		break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long ThorMCLS::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_LASER1_POWER:
		{
			if((param >= _laser1Min) && (param <= _laser1Max))
			{
				_laser1Power = param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER1_POWER out of range %d to %d",static_cast<long>(_laser1Min*1000),static_cast<long>(_laser1Max*1000));
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER2_POWER:
		{
			if((param >= _laser2Min) && (param <= _laser2Max))
			{
				_laser2Power = param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER2_POWER out of range %d to %d",static_cast<long>(_laser2Min*1000),static_cast<long>(_laser2Max*1000));
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER3_POWER:
		{
			if((param >= _laser3Min) && (param <= _laser3Max))
			{
				_laser3Power = param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER3_POWER out of range %d to %d",static_cast<long>(_laser3Min*1000),static_cast<long>(_laser3Max*1000));
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER4_POWER:
		{
			if((param >= _laser4Min) && (param <= _laser4Max))
			{
				_laser4Power = param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER4_POWER out of range %d to %d",static_cast<long>(_laser4Min*1000),static_cast<long>(_laser4Max*1000));
				ret = FALSE;
			}
		}
		break;

	case PARAM_LASER1_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_laser1Enable = static_cast<long>(param);
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER1_ENABLE out of range 0 to 1");
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER2_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_laser2Enable = static_cast<long>(param);
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER2_ENABLE out of range 0 to 1");
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER3_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_laser3Enable = static_cast<long>(param);
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER3_ENABLE out of range 0 to 1");
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER4_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_laser4Enable = static_cast<long>(param);
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER4_ENABLE out of range 0 to 1");
				ret = FALSE;
			}
		}
		break;

	case PARAM_LASER1_POWER_CURRENT:
	case PARAM_LASER2_POWER_CURRENT:
	case PARAM_LASER3_POWER_CURRENT:
	case PARAM_LASER4_POWER_CURRENT:
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter is read only");
			ret = FALSE;
		}
		break;

	default:
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter not implemented");
			ret = FALSE;
		}
	}

	return ret;
}

long ThorMCLS::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_LASER1_POWER:	
		{ 
			param = static_cast<double>(_laser1Power); 
		}
		break;
	case PARAM_LASER2_POWER:	
		{ 
			param = static_cast<double>(_laser2Power); 
		}
		break;
	case PARAM_LASER3_POWER:	
		{ 
			param = static_cast<double>(_laser3Power); 
		}
		break;
	case PARAM_LASER4_POWER:	
		{ 
			param = static_cast<double>(_laser4Power); 
		}
		break;
	case PARAM_LASER1_ENABLE:	
		{ 
			param = static_cast<double>(_laser1Enable); 
		}
		break;
	case PARAM_LASER2_ENABLE:	
		{ 
			param = static_cast<double>(_laser2Enable); 
		}
		break;
	case PARAM_LASER3_ENABLE:	
		{ 
			param = static_cast<double>(_laser3Enable); 
		}
		break;
	case PARAM_LASER4_ENABLE:	
		{ 
			param = static_cast<double>(_laser4Enable); 
		}
		break;

	case PARAM_LASER1_POWER_CURRENT:	
		{ 
			param = static_cast<double>(_laser1Power_C); 
		}
		break;

	case PARAM_LASER2_POWER_CURRENT:	
		{ 
			param = static_cast<double>(_laser2Power_C); 
		}
		break;

	case PARAM_LASER3_POWER_CURRENT:	
		{ 
			param = static_cast<double>(_laser3Power_C); 
		}
		break;

	case PARAM_LASER4_POWER_CURRENT:	
		{ 
			param = static_cast<double>(_laser4Power_C); 
		}
		break;

	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(LASER1|LASER2|LASER3|LASER4);
		}
		break;

	case PARAM_CONNECTION_STATUS:
		{
			param = (_deviceDetected) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
		break;

	default:
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter not implemented");

			ret = FALSE;
		}
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
long ThorMCLS::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorMCLS::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorMCLS::SetParamString(const long paramID, wchar_t* str)
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
long ThorMCLS::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorMCLS::PreflightPosition()
{
	return TRUE;
}

long ThorMCLS::SetupPosition()
{
	return TRUE;
}

long ThorMCLS::StartPosition()
{
	long ret = TRUE;

#ifdef LOGGING_ENABLED
	StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaser1 Enable %d",_laser1Enable);	
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);		
	StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaser2 Enable %d",_laser2Enable);
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);
	StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaser3 Enable %d",_laser3Enable);
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);
	StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaser4 Enable %d",_laser4Enable);
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);
	StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaser1 Power %d",static_cast<long>(_laser1Power *1000));
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);			
	StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaser2 Power %d",static_cast<long>(_laser2Power *1000));
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);
	StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaser3 Power %d",static_cast<long>(_laser3Power *1000));
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);
	StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaser4 Power %d",static_cast<long>(_laser4Power *1000));
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);
#endif

	if(_laser1Enable != _laser1Enable_C)
	{		
		ret=SetMCLSLaserEnable(1, (TRUE==_laser1Enable));
		if(TRUE == ret)
		{
			_laser1Enable_C = _laser1Enable;
			if(TRUE == _laser1Enable)
				_bEnableSet = TRUE;
		}
		else
		{	 
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaserEnable 1 failed");
			return FALSE;
		}
	}

	if(_laser1Power != _laser1Power_C)
	{

		double powerToSet = _laser1Power;
		if(linearizationTables[0].isValid())
		{
			powerToSet = linearizePower(_laser1Min, _laser1Max, _laser1Power, 0);
		}
		ret=SetMCLSPower(1, powerToSet);

		if(TRUE == ret)
		{
			_laser1Power_C = _laser1Power;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSPower 1 failed");
			return FALSE;
		}
	}

	if(_laser2Enable != _laser2Enable_C)
	{

		ret=SetMCLSLaserEnable(2, (TRUE==_laser2Enable));
		if(TRUE == ret)
		{
			_laser2Enable_C = _laser2Enable;
			if(TRUE == _laser2Enable)
				_bEnableSet = TRUE;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaserEnable 2 failed");
			return FALSE;
		}
	}

	if(_laser2Power != _laser2Power_C)
	{
		double powerToSet = _laser2Power;
		if(linearizationTables[1].isValid())
		{
			powerToSet = linearizePower(_laser2Min, _laser2Max, _laser2Power, 0);
		}
		ret=SetMCLSPower(2, powerToSet);

		if(TRUE == ret)
		{
			_laser2Power_C = _laser2Power;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSPower 2 failed");
			return FALSE;
		}
	}

	if(_laser3Enable != _laser3Enable_C)
	{
		ret=SetMCLSLaserEnable(3, (TRUE==_laser3Enable));
		if(TRUE == ret)
		{
			_laser3Enable_C = _laser3Enable;
			if(TRUE == _laser3Enable)
				_bEnableSet = TRUE;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaserEnable 3 failed");
			return FALSE;
		}
	}

	if(_laser3Power != _laser3Power_C)
	{
		double powerToSet = _laser3Power;
		if(linearizationTables[2].isValid())
		{
			powerToSet = linearizePower(_laser3Min, _laser3Max, _laser3Power, 0);
		}
		ret=SetMCLSPower(3, powerToSet);

		if(TRUE == ret)
		{
			_laser3Power_C = _laser3Power;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSPower3 failed");
			return FALSE;
		}
	}

	if(_laser4Enable != _laser4Enable_C)
	{
		ret=SetMCLSLaserEnable(4, (TRUE==_laser4Enable));
		if(TRUE == ret)
		{
			_laser4Enable_C = _laser4Enable;
			if(TRUE == _laser4Enable)
				_bEnableSet = TRUE;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaserEnable 4 failed");
			return FALSE;
		}
	}

	if(_laser4Power != _laser4Power_C)
	{
		double powerToSet = _laser4Power;
		if(linearizationTables[3].isValid())
		{
			powerToSet = linearizePower(_laser4Min, _laser4Max, _laser4Power, 0);
		}
		ret=SetMCLSPower(4, powerToSet);

		if(TRUE == ret)
		{
			_laser4Power_C = _laser4Power;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSPower 4 failed");
			return FALSE;
		}
	}

	return ret;
}

long ThorMCLS::StatusPosition(long &status)
{
	long	ret = TRUE;
	if(TRUE == _bEnableSet) // wait only if any Laser enable changed state
	{
		Sleep(3000); //Wait time for laser(s) to be ready		
	}	

	status = IDevice::STATUS_READY;

	return ret;
}

long ThorMCLS::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;


	StringCbPrintfW(_errMsg,MSG_SIZE,L"ReadPosition failed! Use the PARAM_LASER#_POWER_CURRENT to read the power of the laser");

	return ret;
}

long ThorMCLS::PostflightPosition()
{
	_bEnableSet = FALSE; //Set to FALSE at this stage which is always called after a new command was passed and completed
	return TRUE;
}

long ThorMCLS::SetMCLSLaserEnable(int MCLSID, bool enable)
{
	bool ret;
	TCHAR inst[32];
	StringCbPrintfW(inst,32, _T("channel=%d"), MCLSID);
	if (ret=(TRUE==_serialPort.Write(inst)))
	{
		ret=(0 != ReadMCLSPort());
	}
	StringCbPrintfW(inst,32, _T("enable=%d"), enable);
	if (ret=(TRUE==_serialPort.Write(inst)))
	{		
		ret=(0 != ReadMCLSPort());
	}
	return ret;
}

long ThorMCLS::SetMCLSPower(int MCLSID, double power)
{
	TCHAR inst[32];
	bool ret;
	//	if (gain<0||gain>100)
	//		return false;
	//	int bytegain= (int) GainPercent2Byte(gain);
	StringCbPrintfW(inst,32, _T("channel=%d"), MCLSID);
	if (ret=(TRUE==_serialPort.Write(inst)))
	{
		ret=(0 != ReadMCLSPort());
	}

	StringCbPrintfW(inst,32, _T("current=%d.%04d"), static_cast<long>(power),static_cast<long>(abs(power - static_cast<long>(power))*10000));
	if (ret=(TRUE==_serialPort.Write(inst)))
	{
		ret=(0 != ReadMCLSPort());
	}
	return ret;
}

long ThorMCLS::SetSystemEnable(int enable)
{
	TCHAR inst[32];
	bool ret;
	StringCbPrintfW(inst,32, _T("system=%d"), enable);
	if (ret=(TRUE==_serialPort.Write(inst)))
	{
		ret=(0 != ReadMCLSPort());
	}
	return ret;
}

long ThorMCLS::QueryStatus(void)
{
	bool ret = true;
	/*
	if (ret=_serialPort.Write(_T("statword?")))
	if (ret=ReadMCLSPort())
	ret=ParseReadBuffer(MCLSSTATUS);
	*/
	return ret;
}


long ThorMCLS::ReadMCLSPort()
{	
	return _serialPort.Read(_readBuffer, 1000);
}

long ThorMCLS::ParseReadBuffer(int inquiryID)
{/*
 char c;
 int v[10];
 int ret;
 int dummy;
 int i;
 switch (inquiryID)
 {
 case LASER1_POWER:
 ret=swscanf_s(_readBuffer, _T("LASER1_POWER=%d"), &r_laser1Power);
 break;
 case LASER2_POWER:
 ret=swscanf_s(_readBuffer, _T("LASER2_POWER=%d"), &r_laser2Power);
 break;
 case LASER1_ENABLE:
 case LASER2_ENABLE:
 break;
 case MCLSSTATUS:
 for (i=11; i<19; i++)
 {
 c=(char) _readBuffer[i];
 v[i-11]=atoi(&c);
 }
 r_laser1Enable=(bool) v[0];
 r_laser2Enable=(bool) v[1];
 r_systemEnable=(bool) v[5];
 ret=swscanf_s(_readBuffer, _T("statword?\r\n%d\r\n%d,%d"), &dummy, &r_laser1Power, &r_laser2Power);
 break;
 }
 if (ret==0||ret==EOF)
 return false;
 else return true;
 */
	return true;
}

long ThorMCLS::QueryID()
{
	long ret = FALSE;

	if (_serialPort.Write(_T("id?")))
	{
		if (ReadMCLSPort())
		{
			if(wcsstr(_readBuffer,L"MCLS"))
			{
				ret = TRUE;
			}
		}
	}

	return ret;

}

long ThorMCLS::GetLastErrorMsg(wchar_t *msg, long size)
{	
	wcsncpy_s(msg,MSG_SIZE,_errMsg,MSG_SIZE);
	return TRUE;
}

std::wstring ThorMCLS::getTableDirectoryPath()
{

	HINSTANCE resourceManager = LoadLibrary(L"ResourceManager.dll");
	if(resourceManager)
	{
		typedef bool(CALLBACK* GetApplicationSettingsPathFunction)(wchar_t* buffer, unsigned int bufferSize);
		GetApplicationSettingsPathFunction getApplicationSettingsPath = (GetApplicationSettingsPathFunction)GetProcAddress(resourceManager, "GetApplicationSettingsPath");
		if(getApplicationSettingsPath)
		{

			const int BUFFER_SIZE = 1000;
			wchar_t pathBuffer[BUFFER_SIZE];
			bool pathRetrieved = getApplicationSettingsPath(pathBuffer, BUFFER_SIZE);

			if(pathRetrieved)
			{
				std::wstring directoryPath(pathBuffer);
				directoryPath +=  std::wstring(L"MCLS\\");
				return directoryPath;
			}
		}
		FreeLibrary(resourceManager);
	}

	return L".\\";
}


/// <summary> Loads linearization tables from the application settings folder to be used to linearize the laser light intensity </summary>
/// <returns> True if a valid table was loaded for all laser channels </returns>
long ThorMCLS::CalibrateMCLS()
{
	std::wstring applicationSettingsPath = getTableDirectoryPath();
	std::wstring tablePrefix = L"MCLS_LinearizationTable_";
	std::wstring tablePostfix = L".txt";
	std::wstringstream filePath;

	bool allTablesValid = true;
	for(int i=0; i<MAX_CHANNELS; i++)
	{
		filePath.str(std::wstring());
		filePath << applicationSettingsPath << tablePrefix << i+1 << tablePostfix;
		linearizationTables[i].loadFromFile(filePath.str());

		allTablesValid |= linearizationTables[i].isValid(); //Check That All Tables are Loaded Correctly
	}

	return allTablesValid;

}


/// <summary> Linearizes the input power based on loaded data tables and the power range. The tables are setup to
///           translate a %input power to a %output power. Therefore, the current %power is calculated based on the
///           min and max provided, a %output power is retrieved using the input from the table, and a actual power
///           is returned based on the range
/// </summary>
/// <param name="minPower"> The minimum power of the selected laser channel </param>
/// <param name="maxPower"> The maximum power of the selected laser channel </param>
/// <param name="curPower"> The desired real power of the selected laser channel, assuming a linear scale between min and max </param>
/// <param name="channelIndex"> The laser channel that is being linearized </param>
/// <returns> The necessary power needed for this input channel to output the light power that would be expected if the laser light
///           scaled linearly with the input power. ie. the linearized laser power </returns>
double ThorMCLS::linearizePower(double minPower, double maxPower, double curPower, int channelIndex)
{	
	double powerRange = maxPower - minPower;
	double powerPct = (curPower - minPower) / powerRange;
	double linearizedPct = linearizationTables[channelIndex].interpolate(powerPct, 0);
	double linearizedPower = linearizedPct * powerRange + minPower;
	return linearizedPower;

}