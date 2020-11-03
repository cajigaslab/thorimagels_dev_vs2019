// ThorMCLSSimulator.cpp : Defines the exported functions for the DLL application.
//

//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorMCLSSimulator.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "ThorMCLSSimulator.h"
#include "ThorMCLSSetupXML.h"
#include "Strsafe.h"

#define READ_TIMEOUT 1000


wchar_t message[MSG_SIZE];


ThorMCLSSimulator::ThorMCLSSimulator()
{
	_laser1Min=0;
	_laser1Max=35;
	_laser2Min=0;
	_laser2Max=35;
	_laser3Min=0;
	_laser3Max=35;
	_laser4Min=0;
	_laser4Max=35;

	_laser1Enable=false;
	_laser2Enable=false;
	_laser3Enable=false;
	_laser4Enable=false;
	_laser1Power=_laser1Min;
	_laser2Power=_laser2Min;
	_laser3Power=_laser3Min;
	_laser4Power=_laser4Min;

	_laser1Enable_C=true;
	_laser2Enable_C=true;
	_laser3Enable_C=true;
	_laser4Enable_C=true;
	_laser1Power_C=_laser1Min-1;
	_laser2Power_C=_laser2Min-1;
	_laser3Power_C=_laser3Min-1;
	_laser4Power_C=_laser4Min-1;

	_deviceDetected = FALSE;
	_numDevices = 0;

	_dataBuffer[0] = NULL;
	_errMsg[0] = NULL;
	_readBuffer[0] = NULL;
}

ThorMCLSSimulator::~ThorMCLSSimulator()
{
	_instanceFlag = false;
}

bool ThorMCLSSimulator:: _instanceFlag = false;

auto_ptr<ThorMCLSSimulator> ThorMCLSSimulator::_single(new ThorMCLSSimulator());

ThorMCLSSimulator *ThorMCLSSimulator::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorMCLSSimulator());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorMCLSSimulator::FindDevices(long &deviceCount)
{
	long	ret = TRUE;

	deviceCount = 1;
	_deviceDetected = TRUE;
	_numDevices = 1;

	return ret;
}

void ThorMCLSSimulator::GetMinMaxFromReadBuffer(double &minVal, double &maxVal)
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

long ThorMCLSSimulator::SelectDevice(const long device)
{
	long	ret = FALSE;

	if(FALSE == _deviceDetected)
	{
		//StringCbPrintfW(_errMsg,MSG_SIZE,L"No devices found");
		return FALSE;
	}

	if((device < 0) || (device >= _numDevices))
		return FALSE;

	ret = TRUE;
	return ret;
}

long ThorMCLSSimulator::TeardownDevice()
{
	return TRUE;
}

long ThorMCLSSimulator::GetParamInfo
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
	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = (double)ConnectionStatusType::CONNECTION_READY;
			paramMax = (double)ConnectionStatusType::CONNECTION_READY;
			paramDefault = (double)ConnectionStatusType::CONNECTION_READY;
		}
		break;
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

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long ThorMCLSSimulator::SetParam(const long paramID, const double param)
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
				//StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER1_POWER out of range %d to %d",static_cast<long>(_laser1Min*1000),static_cast<long>(_laser1Max*1000));
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
				//StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER2_POWER out of range %d to %d",static_cast<long>(_laser2Min*1000),static_cast<long>(_laser2Max*1000));
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
				//StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER3_POWER out of range %d to %d",static_cast<long>(_laser3Min*1000),static_cast<long>(_laser3Max*1000));
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
				//StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER4_POWER out of range %d to %d",static_cast<long>(_laser4Min*1000),static_cast<long>(_laser4Max*1000));
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
				//StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER1_ENABLE out of range 0 to 1");
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
				//StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER2_ENABLE out of range 0 to 1");
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
				//StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER3_ENABLE out of range 0 to 1");
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
				//StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER4_ENABLE out of range 0 to 1");
				ret = FALSE;
			}
		}
		break;

	case PARAM_LASER1_POWER_CURRENT:
	case PARAM_LASER2_POWER_CURRENT:
	case PARAM_LASER3_POWER_CURRENT:
	case PARAM_LASER4_POWER_CURRENT:
		{
			//StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter is read only");
			ret = FALSE;
		}
		break;

	default:
		{	
			//StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter not implemented");
			ret = FALSE;
		}
	}

	return ret;
}

long ThorMCLSSimulator::GetParam(const long paramID, double &param)
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
			param = static_cast<double>(ConnectionStatusType::CONNECTION_READY);
		}
		break;
	default:
		{	
			//StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter not implemented");

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
long ThorMCLSSimulator::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorMCLSSimulator::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorMCLSSimulator::SetParamString(const long paramID, wchar_t* str)
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
long ThorMCLSSimulator::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorMCLSSimulator::PreflightPosition()
{
	return TRUE;
}

long ThorMCLSSimulator::SetupPosition()
{
	return TRUE;
}

long ThorMCLSSimulator::StartPosition()
{
	long ret = TRUE;

	if(_laser1Enable != _laser1Enable_C)
	{
		ret=SetMCLSLaserEnable(1, (TRUE==_laser1Enable));
		if(TRUE == ret)
		{
			_laser1Enable_C = _laser1Enable;
		}
		else
		{	
			//StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaserEnable 1 failed");
			return FALSE;
		}
	}

	if(_laser1Power != _laser1Power_C)
	{
		ret=SetMCLSPower(1, static_cast<double> (_laser1Power));
		if(TRUE == ret)
		{
			_laser1Power_C = _laser1Power;
		}
		else
		{	
			//StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSPower 1 failed");
			return FALSE;
		}
	}

	if(_laser2Enable != _laser2Enable_C)
	{
		ret=SetMCLSLaserEnable(2, (TRUE==_laser2Enable));
		if(TRUE == ret)
		{
			_laser2Enable_C = _laser2Enable;
		}
		else
		{	
			//StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaserEnable 2 failed");
			return FALSE;
		}
	}

	if(_laser2Power != _laser2Power_C)
	{
		ret=SetMCLSPower(2,static_cast<double> (_laser2Power));
		if(TRUE == ret)
		{
			_laser2Power_C = _laser2Power;
		}
		else
		{	
			//StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSPower 2 failed");
			return FALSE;
		}
	}

	if(_laser3Enable != _laser3Enable_C)
	{
		ret=SetMCLSLaserEnable(3, (TRUE==_laser3Enable));
		if(TRUE == ret)
		{
			_laser3Enable_C = _laser3Enable;
		}
		else
		{	
			//StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaserEnable 3 failed");
			return FALSE;
		}
	}

	if(_laser3Power != _laser3Power_C)
	{
		ret=SetMCLSPower(3,static_cast<double> (_laser3Power));
		if(TRUE == ret)
		{
			_laser3Power_C = _laser3Power;
		}
		else
		{	
			//StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSPower3 failed");
			return FALSE;
		}
	}

	if(_laser4Enable != _laser4Enable_C)
	{
		ret=SetMCLSLaserEnable(4, (TRUE==_laser4Enable));
		if(TRUE == ret)
		{
			_laser4Enable_C = _laser4Enable;
		}
		else
		{	
			//StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSLaserEnable 4 failed");
			return FALSE;
		}
	}

	if(_laser4Power != _laser4Power_C)
	{
		ret=SetMCLSPower(4,static_cast<double> (_laser4Power));
		if(TRUE == ret)
		{
			_laser4Power_C = _laser4Power;
		}
		else
		{	
			//StringCbPrintfW(_errMsg,MSG_SIZE,L"SetMCLSPower 4 failed");
			return FALSE;
		}
	}

	return ret;
}

long ThorMCLSSimulator::StatusPosition(long &status)
{
	long	ret = TRUE;
	status = IDevice::STATUS_READY;
	return ret;
}

long ThorMCLSSimulator::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;


	//StringCbPrintfW(_errMsg,MSG_SIZE,L"ReadPosition failed! Use the PARAM_LASER#_POWER_CURRENT to read the power of the laser");

	return ret;
}

long ThorMCLSSimulator::PostflightPosition()
{
	return TRUE;
}

long ThorMCLSSimulator::SetMCLSLaserEnable(int MCLSID, bool enable)
{
	bool ret = enable;
	//TCHAR inst[32];
	//wsprintf(inst, _T("channel=%d"), MCLSID);
	//if (ret=_serialPort.Write(inst))
	//	ret=ReadMCLSPort();
	//
	//wsprintf(inst, _T("enable=%d"), enable);
	//if (ret=_serialPort.Write(inst))
	//	ret=ReadMCLSPort();
	return ret;
}

long ThorMCLSSimulator::SetMCLSPower(int MCLSID, double power)
{
	//TCHAR inst[32];
	bool ret = TRUE;
	//	if (gain<0||gain>100)
	//		return false;
	//	int bytegain= (int) GainPercent2Byte(gain);
	//wsprintf(inst, _T("channel=%d"), MCLSID);
	//if (ret=_serialPort.Write(inst))
	//	ret=ReadMCLSPort();

	//wsprintf(inst, _T("current=%d.%d"), static_cast<long>(power),abs(power - static_cast<long>(power))*100);
	//if (ret=_serialPort.Write(inst))
	//	ret=ReadMCLSPort();
	return ret;
}

long ThorMCLSSimulator::SetSystemEnable(int enable)
{
	/*TCHAR inst[32];*/
	bool ret = (TRUE==enable);
	//wsprintf(inst, _T("system=%d"), enable);
	//if (ret=_serialPort.Write(inst))
	//	ret=ReadMCLSPort();
	return ret;
}

long ThorMCLSSimulator::QueryStatus(void)
{
	bool ret = true;
	/*
	if (ret=_serialPort.Write(_T("statword?")))
	if (ret=ReadMCLSPort())
	ret=ParseReadBuffer(MCLSSTATUS);
	*/
	return ret;
}


long ThorMCLSSimulator::ReadMCLSPort()
{	
	//return _serialPort.Read(_readBuffer, 1000);
	return TRUE;
}

long ThorMCLSSimulator::ParseReadBuffer(int inquiryID)
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


long ThorMCLSSimulator::GetLastErrorMsg(wchar_t *msg, long size)
{	
	//wcsncpy_s(msg,MSG_SIZE,_errMsg,MSG_SIZE);
	return TRUE;
}