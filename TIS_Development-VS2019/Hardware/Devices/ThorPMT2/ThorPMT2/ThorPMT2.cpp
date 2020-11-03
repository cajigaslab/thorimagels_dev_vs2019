// ThorPMT2.cpp : Defines the exported functions for the DLL application.
//

//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorPMT2.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "ThorPMT2.h"
#include "ThorPMT2SetupXML.h"
#include "Strsafe.h"

#define READ_TIMEOUT 1000

//*TODO* This gain power will change based on the PMT sensor. Must add this as a setting.
//(H7422P-40) High Sensitivity = 6.64
//(H9305-03) Low Sensitivity = 7.29
#define PMTGAINPOWER 6.64

//linearization changes the lower bound to be at a digital value of 50 that is sent to the controller
//reference u1 = V^(alpha*n)

#define GAINLBOUND 0.00002
#define GAINHBOUND 0.25
#define GAINMAX 255

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

wchar_t message[MSG_SIZE];


ThorPMT2::ThorPMT2()
{
	_pmt3Enable=false;
	_pmt4Enable=false;
	_pmt3Gain=PMT_GAIN_DEFAULT;
	_pmt4Gain=PMT_GAIN_DEFAULT;

	_pmt3Enable_C=true;
	_pmt4Enable_C=true;
	_pmt3Gain_C=PMT_GAIN_DEFAULT-1;
	_pmt4Gain_C=PMT_GAIN_DEFAULT-1;

	_deviceDetected = FALSE;
}

ThorPMT2::~ThorPMT2()
{
	_instanceFlag = false;
}

bool ThorPMT2:: _instanceFlag = false;

auto_ptr<ThorPMT2> ThorPMT2::_single(new ThorPMT2());

ThorPMT2 *ThorPMT2::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorPMT2());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorPMT2::FindDevices(long &deviceCount)
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
		auto_ptr<ThorPMT2XML> pSetup(new ThorPMT2XML());

		pSetup->GetConnection(portID);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorPMT2Settings.xml file");
		deviceCount = 0;
		return FALSE;
	}

	TCHAR PortName[32];
	StringCbPrintfW(PortName,32, _T("COM%d"), portID);

	if(FALSE == _serialPort.Open(PortName))
	{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorPMT2 FindDevices could not open serial port");
#endif
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to open com port %d",portID);

		// *TODO* perform an automatic scane of the available serial ports for the Z stepper
		deviceCount = 0;
	}
	else
	{
		deviceCount = 1;
		_deviceDetected = TRUE;
		_serialPort.Close();
	}

	return ret;
}

long ThorPMT2::SelectDevice(const long device)
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

			try
			{
				auto_ptr<ThorPMT2XML> pSetup(new ThorPMT2XML());

				pSetup->GetConnection(portID);
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorPMT2Settings.xml file");
				return FALSE;
			}

			TCHAR PortName[32];
			StringCbPrintfW(PortName,32, _T("COM%d"), portID);

			if(FALSE == _serialPort.Open(PortName))
			{
#ifdef LOGGING_ENABLED
				logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorPMT2 FindDevices could not open serial port");
#endif
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to open com port %d",portID);
			}

			ret = TRUE; 
		}
		break;
	default:	{ }
	}

	return ret;
}

long ThorPMT2::TeardownDevice()
{
	//Disable the PMTs
	SetParam(IDevice::PARAM_PMT1_GAIN_POS, 0);
	SetParam(IDevice::PARAM_PMT2_GAIN_POS, 0);
	SetParam(IDevice::PARAM_PMT1_ENABLE, FALSE);
	SetParam(IDevice::PARAM_PMT2_ENABLE, FALSE);

	SetupPosition();
	StartPosition();

	_serialPort.Close();
	return TRUE;
}

long ThorPMT2::GetParamInfo
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
	case PARAM_PMT3_GAIN_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PMT_GAIN_MIN;
			paramMax = PMT_GAIN_MAX;
			paramDefault = PMT_GAIN_DEFAULT;
		}
		break;

	case PARAM_PMT4_GAIN_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PMT_GAIN_MIN;
			paramMax = PMT_GAIN_MAX;
			paramDefault = PMT_GAIN_DEFAULT;
		}
		break;

	case PARAM_PMT3_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
	case PARAM_PMT4_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;

	case PARAM_PMT3_SAFETY:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 1;
		}
		break;
		
	case PARAM_PMT4_SAFETY:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 1;
		}
		break;

	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = PMT3 | PMT4;
		}
		break;

	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = CONNECTION_READY;
			paramMax = CONNECTION_UNAVAILABLE;
			paramDefault = CONNECTION_UNAVAILABLE;
		}
		break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long ThorPMT2::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_PMT3_GAIN_POS:
		{
			if((param >= PMT_GAIN_MIN) && (param <= PMT_GAIN_MAX))
			{
				_pmt3Gain = static_cast<long>(param);

			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_PMT3_GAIN_POS out of range %d to %d",PMT_GAIN_MIN,PMT_GAIN_MAX);
				ret = FALSE;
			}
		}
		break;
	case PARAM_PMT4_GAIN_POS:
		{
			if((param >= PMT_GAIN_MIN) && (param <= PMT_GAIN_MAX))
			{
				_pmt4Gain = static_cast<long>(param);

			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_PMT4_GAIN_POS out of range %d to %d",PMT_GAIN_MIN,PMT_GAIN_MAX);
				ret = FALSE;
			}
		}
		break;

	case PARAM_PMT3_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_pmt3Enable = static_cast<long>(param);

			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_PMT3_ENABLE out of range 0 to 1");
				ret = FALSE;
			}
		}
		break;
	case PARAM_PMT4_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_pmt4Enable = static_cast<long>(param);

			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_PMT4_ENABLE out of range 0 to 1");
				ret = FALSE;
			}
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

long ThorPMT2::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_PMT3_GAIN_POS:	
		{ 
			param = static_cast<double>(_pmt3Gain); 
		}
		break;
	case PARAM_PMT4_GAIN_POS:	
		{ 
			param = static_cast<double>(_pmt4Gain); 
		}
		break;

	case PARAM_PMT3_ENABLE:	
		{ 
			param = static_cast<double>(_pmt3Enable); 
		}
		break;
	case PARAM_PMT4_ENABLE:	
		{ 
			param = static_cast<double>(_pmt4Enable); 
		}
		break;

	case PARAM_PMT3_SAFETY:
		{
			long	ret = FALSE;

			Lock lock(_critSect);
			param = 1;
			return TRUE;
			if(QueryStatus())
			{
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
			else
			{
				param = 1;
				ret = TRUE;
			}
		}
		break;
	case PARAM_PMT4_SAFETY:
		{
			long	ret = FALSE;
			
			Lock lock(_critSect);
			if(QueryStatus())
			{
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
			else
			{
				param = 1;
				ret = FALSE;
			}
		}
		break;

	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(PMT3 | PMT4);
		}
		break;

	case PARAM_CONNECTION_STATUS:
		{
			param = (_deviceDetected) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
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
long ThorPMT2::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPMT2::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPMT2::SetParamString(const long paramID, wchar_t* str)
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
long ThorPMT2::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorPMT2::PreflightPosition()
{
	return TRUE;
}

long ThorPMT2::SetupPosition()
{
	return TRUE;
}

long ThorPMT2::StartPosition()
{
	long ret = TRUE;
	
			Lock lock(_critSect);

	if(_pmt3Enable != _pmt3Enable_C)
	{
		ret=SetPMTEnable(1, (TRUE==_pmt3Enable));
		if(TRUE == ret)
		{
			_pmt3Enable_C = _pmt3Enable;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetPMTEnable 3 failed");
			return FALSE;
		}
	}

	if(_pmt3Gain != _pmt3Gain_C)
	{
		ret=SetPMTGain(1, static_cast<int> (_pmt3Gain));
		if(TRUE == ret)
		{
			_pmt3Gain_C = _pmt3Gain;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetPMTGain 3 failed");
			return FALSE;
		}
	}

	if(_pmt4Enable != _pmt4Enable_C)
	{
		ret=SetPMTEnable(2, (TRUE==_pmt4Enable));
		if(TRUE == ret)
		{
			_pmt4Enable_C = _pmt4Enable;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetPMTEnable 4 failed");
			return FALSE;
		}
	}

	if(_pmt4Gain != _pmt4Gain_C)
	{
		ret=SetPMTGain(2,static_cast<int> (_pmt4Gain));
		if(TRUE == ret)
		{
			_pmt4Gain_C = _pmt4Gain;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetPMTGain 4 failed");
			return FALSE;
		}
	}


	return ret;
}

long ThorPMT2::StatusPosition(long &status)
{
	long	ret = TRUE;
	status = IDevice::STATUS_READY;
	return ret;
}

long ThorPMT2::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	if(deviceType == PMT3)
	{
		pos = static_cast<double>(_pmt3Gain_C);
	}
	else if(deviceType == PMT4)
	{
		pos = static_cast<double>(_pmt4Gain_C);		
	}	
	else
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorPMT2 Invalid device specified for reading");
	}

	return ret;
}

long ThorPMT2::PostflightPosition()
{
	return TRUE;
}

long ThorPMT2::SetPMTEnable(int PMTID, bool enable)
{
	bool ret;
	TCHAR inst[32];
	StringCbPrintfW(inst,32, _T("pmt%d=%d"), PMTID, enable);
	if (ret=(0 < _serialPort.Write(inst)))
		ret=(0 < ReadPMTPort());
	return ret;
}

long ThorPMT2::SetPMTGain(int PMTID, int gain)
{
	TCHAR inst[32];
	bool ret;
	if (gain<0||gain>100)
		return false;
	int bytegain= (int) GainPercent2Byte(gain);
	StringCbPrintfW(inst,32, _T("pmt%dgain=%d"), PMTID, bytegain);
	if (ret=(0 < _serialPort.Write(inst)))
		ret=(0 < ReadPMTPort());
	return ret;
}

long ThorPMT2::SetCRSEnable(int enable)
{
	TCHAR inst[32];
	bool ret;
	StringCbPrintfW(inst,32, _T("scan=%d"), enable);
	if (ret=(0 < _serialPort.Write(inst)))
		ret=(0 < ReadPMTPort());
	return ret;
}

long ThorPMT2::QueryStatus(void)
{
	bool ret;
	if (ret=(0 < _serialPort.Write(_T("statword?"))))
	{
		if (ret=(0 < ReadPMTPort()))
		{
			ret=(0 < ParseReadBuffer(PMTSTATUS));
		}
	}
	return ret;
}


long ThorPMT2::ReadPMTPort()
{	
	return _serialPort.Read(_readBuffer, 1000);
}

long ThorPMT2::ParseReadBuffer(int inquiryID)
{
	char c;
	int v[10];
	int ret;
	int dummy;
	int i;
	switch (inquiryID)
	{
	case PMT3GAIN:
		ret=swscanf_s(_readBuffer, _T("PMT3GAIN=%d"), &_r_pmt3Gain);
		break;
	case PMT4GAIN:
		ret=swscanf_s(_readBuffer, _T("PMT4GAIN=%d"), &_r_pmt4Gain);
		break;
	case PMT3ENABLE:
	case PMT4ENABLE:
		break;
	case PMTSTATUS:
		for (i=11; i<19; i++)
		{
			c=(char) _readBuffer[i];

			//due to corrupted buffer returns we do an exact character match
			//only trip the error if the character is '1'
			if(17 == i)
			{
				if(c == '1')
				{
					v[6] = 1;
				}
				else
				{ 
					v[6] = 0;
				}
			}
			else
			{
				v[i-11]=atoi(&c);
			}
		}
		_r_pmt3Enable=(TRUE==v[0]);
		_r_pmt4Enable=(TRUE==v[1]);
		_r_crsEnable=(TRUE==v[5]);
		_r_pmtError=(TRUE==v[6]);
		ret=swscanf_s(_readBuffer, _T("statword?\r\n%d\r\n%d,%d,%d"), &dummy, &_r_pmt3Gain, &_r_pmt4Gain,&_r_pmtError);
		break;
	}
	if (ret==0||ret==EOF)
		return false;
	else return true;
}

long ThorPMT2::GainPercent2Byte(int percentageGain)
{
	double actualGain= static_cast<double>(percentageGain)/100.0*(GAINHBOUND-GAINLBOUND)+GAINLBOUND;
	double bytegain=pow(actualGain/GAINHBOUND,1/PMTGAINPOWER)*GAINMAX;
	return static_cast<long>(ceil(bytegain-0.5));
}

long ThorPMT2::GainByte2Percent(int byteGain)
{
	double percentageGain=(GAINHBOUND*pow(static_cast<double>(byteGain)/GAINMAX, PMTGAINPOWER)-GAINLBOUND)/(GAINHBOUND-GAINLBOUND)*100.0;
	return static_cast<long>(ceil(percentageGain - 0.5));
}

long ThorPMT2::GetLastErrorMsg(wchar_t *msg, long size)
{	
	wcsncpy_s(msg,MSG_SIZE,_errMsg,MSG_SIZE);
	return TRUE;
}