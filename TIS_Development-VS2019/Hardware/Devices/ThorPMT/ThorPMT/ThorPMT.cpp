// ThorPMT.cpp : Defines the exported functions for the DLL application.
//

//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorPMT.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "ThorPMT.h"
#include "ThorPMTSetupXML.h"
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



ThorPMT::ThorPMT()
{
	_pmt1Enable=false;
	_pmt2Enable=false;
	_pmt3Enable=false;
	_pmt4Enable=false;
	_pmt1Gain=PMT_GAIN_DEFAULT;
	_pmt2Gain=PMT_GAIN_DEFAULT;
	_pmt3Gain=PMT_GAIN_DEFAULT;
	_pmt4Gain=PMT_GAIN_DEFAULT;
	_crsEnable=false;

	_pmt1Enable_C=true;
	_pmt2Enable_C=true;
	_pmt3Enable_C=true;
	_pmt4Enable_C=true;
	_pmt1Gain_C=PMT_GAIN_DEFAULT-1;
	_pmt2Gain_C=PMT_GAIN_DEFAULT-1;
	_pmt3Gain_C=PMT_GAIN_DEFAULT-1;
	_pmt4Gain_C=PMT_GAIN_DEFAULT-1;
	_crsEnable_C=-1;

	_deviceDetected = FALSE;
}

ThorPMT::~ThorPMT()
{
	_instanceFlag = false;
}

bool ThorPMT:: _instanceFlag = false;

auto_ptr<ThorPMT> ThorPMT::_single(new ThorPMT());

ThorPMT *ThorPMT::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorPMT());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorPMT::FindDevices(long &deviceCount)
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
		auto_ptr<ThorPMTXML> pSetup(new ThorPMTXML());

		pSetup->GetConnection(portID);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorPMTSettings.xml file");
		deviceCount = 0;
		return FALSE;
	}

	TCHAR PortName[32];
	StringCbPrintfW(PortName,32, _T("COM%d"), portID);

	if(FALSE == _serialPort.Open(PortName))
	{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorPMT FindDevices could not open serial port");
#endif
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to open com port %d",portID);
		ret = FALSE;

		// *TODO* perform an automatic scane of the available serial ports for the Z stepper
		deviceCount = 0;
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

	return ret;
}

long ThorPMT::SelectDevice(const long device)
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
			long initMode = 0;
			_rsInitMode=0;

			try
			{
				auto_ptr<ThorPMTXML> pSetup(new ThorPMTXML());

				pSetup->GetConnection(portID);
				pSetup->GetConfiguration(initMode);	// resonance scanner initial mode
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorPMTSettings.xml file");
				return FALSE;
			}

			TCHAR PortName[32];
			StringCbPrintfW(PortName,32, _T("COM%d"), portID);

			if(FALSE == _serialPort.Open(PortName))
			{
#ifdef LOGGING_ENABLED
				logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorPMT FindDevices could not open serial port");
#endif
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to open com port %d",portID);
			}

			// turn on/off resonance scanner based on initial mode param 
			if(1 == initMode)
			{
				//turn the scanner on
				SetParam(IDevice::PARAM_SCANNER_ENABLE, 1);
				SetupPosition();
				StartPosition();
				//set the PARAM_SCANNER_INIT_MODE parameter after enabling the device
				SetParam(IDevice::PARAM_SCANNER_INIT_MODE, 1);
			}
			else if ( 0 == initMode)
			{
				//set the PARAM_SCANNER_INIT_MODE parameter before disabling the device
				SetParam(IDevice::PARAM_SCANNER_INIT_MODE, 0);
				//turn the scanner on
				SetParam(IDevice::PARAM_SCANNER_ENABLE, 0);
				SetupPosition();
				StartPosition();
			}

			ret = TRUE; 
		}
		break;
	default:	{ }
	}

	return ret;
}

long ThorPMT::TeardownDevice()
{
	double val = 0;
	
	GetParam(IDevice::PARAM_SCANNER_INIT_MODE, val);

	if(1 == static_cast<long>(val))
	{
		//Stop the resonance scanner if it is running
		SetParam(IDevice::PARAM_SCANNER_INIT_MODE, 0);
		SetParam(IDevice::PARAM_SCANNER_ENABLE, 1);
		SetParam(IDevice::PARAM_SCANNER_ENABLE, 0);
		SetParam(IDevice::PARAM_SCANNER_ENABLE, 1);
		SetParam(IDevice::PARAM_SCANNER_ENABLE, 0);
	}
	
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

long ThorPMT::GetParamInfo
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
	case PARAM_PMT1_GAIN_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PMT_GAIN_MIN;
			paramMax = PMT_GAIN_MAX;
			paramDefault = PMT_GAIN_DEFAULT;
		}
		break;

	case PARAM_PMT2_GAIN_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = PMT_GAIN_MIN;
			paramMax = PMT_GAIN_MAX;
			paramDefault = PMT_GAIN_DEFAULT;
		}
		break;

	case PARAM_PMT1_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
	case PARAM_PMT2_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;

	case PARAM_PMT1_SAFETY:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 1;
		}
		break;
		
	case PARAM_PMT2_SAFETY:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 1;
		}
		break;

	case PARAM_SCANNER_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
		
	case PARAM_SCANNER_INIT_MODE:
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
			paramMin = paramMax = paramDefault = PMT1 | PMT2 | CONTROL_UNIT;
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

long ThorPMT::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_PMT1_GAIN_POS:
		{
			if((param >= PMT_GAIN_MIN) && (param <= PMT_GAIN_MAX))
			{
				_pmt1Gain = static_cast<long>(param);

			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_PMT1_GAIN_POS out of range %d to %d",PMT_GAIN_MIN,PMT_GAIN_MAX);
				ret = FALSE;
			}
		}
		break;
	case PARAM_PMT2_GAIN_POS:
		{
			if((param >= PMT_GAIN_MIN) && (param <= PMT_GAIN_MAX))
			{
				_pmt2Gain = static_cast<long>(param);

			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_PMT2_GAIN_POS out of range %d to %d",PMT_GAIN_MIN,PMT_GAIN_MAX);
				ret = FALSE;
			}
		}
		break;
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

	case PARAM_PMT1_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_pmt1Enable = static_cast<long>(param);

			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_PMT1_ENABLE out of range 0 to 1");
				ret = FALSE;
			}
		}
		break;
	case PARAM_PMT2_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_pmt2Enable = static_cast<long>(param);

			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_PMT2_ENABLE out of range 0 to 1");
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

	case PARAM_SCANNER_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_crsEnable = static_cast<long>(param);		
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_SCANNER_ENABLE out of range 0 to 1");
				ret = FALSE;
			}
		}
		break;
	case PARAM_SCANNER_INIT_MODE:
		{
			if((param >= 0) && (param <= 1))
			{
				_rsInitMode = static_cast<long>(param);
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_SCANNER_INIT_MODE out of range 0 to 1");
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

long ThorPMT::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_PMT1_GAIN_POS:	
		{ 
			param = static_cast<double>(_pmt1Gain); 
		}
		break;
	case PARAM_PMT2_GAIN_POS:	
		{ 
			param = static_cast<double>(_pmt2Gain); 
		}
		break;
	case PARAM_PMT1_ENABLE:	
		{ 
			param = static_cast<double>(_pmt1Enable); 
		}
		break;
	case PARAM_PMT2_ENABLE:	
		{ 
			param = static_cast<double>(_pmt2Enable); 
		}
		break;
	case PARAM_PMT1_SAFETY:
		{
			long	ret = FALSE;

			Lock lock(_critSect);

			param = 1;
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
				ret = TRUE;
			}
			else
			{
				param = 1;
				ret = TRUE;
			}
		}
		break;
	case PARAM_PMT2_SAFETY:
		{
			long	ret = FALSE;
			
			Lock lock(_critSect);
			param = 1;
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
				ret = TRUE;
			}
			else
			{
				param = 1;
				ret = FALSE;
			}
		}
		break;
	case PARAM_SCANNER_ENABLE:
		{
			param = static_cast<double>(_crsEnable);
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(PMT1 | PMT2 | CONTROL_UNIT);
		}
		break;

	case PARAM_SCANNER_INIT_MODE:
		{
			param = _rsInitMode;
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
long ThorPMT::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPMT::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPMT::SetParamString(const long paramID, wchar_t* str)
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
long ThorPMT::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorPMT::PreflightPosition()
{
	return TRUE;
}

long ThorPMT::SetupPosition()
{
	return TRUE;
}

long ThorPMT::StartPosition()
{
	long ret = TRUE;
	
	Lock lock(_critSect);

	if(_pmt1Enable != _pmt1Enable_C)
	{
		ret=SetPMTEnable(1, (TRUE==_pmt1Enable));
		if(TRUE == ret)
		{
			_pmt1Enable_C = _pmt1Enable;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetPMTEnable 1 failed");
			return FALSE;
		}
	}

	if(_pmt1Gain != _pmt1Gain_C)
	{
		ret=SetPMTGain(1, static_cast<int> (_pmt1Gain));
		if(TRUE == ret)
		{
			_pmt1Gain_C = _pmt1Gain;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetPMTGain 1 failed");
			return FALSE;
		}
	}

	if(_pmt2Enable != _pmt2Enable_C)
	{
		ret=SetPMTEnable(2, (TRUE==_pmt2Enable));
		if(TRUE == ret)
		{
			_pmt2Enable_C = _pmt2Enable;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetPMTEnable 2 failed");
			return FALSE;
		}
	}

	if(_pmt2Gain != _pmt2Gain_C)
	{
		ret=SetPMTGain(2,static_cast<int> (_pmt2Gain));
		if(TRUE == ret)
		{
			_pmt2Gain_C = _pmt2Gain;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetPMTGain 2 failed");
			return FALSE;
		}
	}

	if(_crsEnable != _crsEnable_C)
	{
		if((1 == _rsInitMode) )
		{
			if (TRUE == _crsEnable)
			{
				ret=SetCRSEnable(static_cast<int>(_crsEnable));
				if(TRUE == ret)
				{
					_crsEnable_C = _crsEnable;
				}
				else
				{
					StringCbPrintfW(_errMsg,MSG_SIZE,L"SetCRSEnable failed");
					return FALSE;
				}
			}
		}
		else
		{
			ret=SetCRSEnable(static_cast<int>(_crsEnable));
			if(TRUE == ret)
			{
				_crsEnable_C = _crsEnable;
				Sleep(200);
			}
			else
			{	
				StringCbPrintfW(_errMsg,MSG_SIZE,L"SetCRSEnable failed");
				return FALSE;
			}
		}
	}
	return ret;
}

long ThorPMT::StatusPosition(long &status)
{
	long	ret = TRUE;
	status = IDevice::STATUS_READY;
	return ret;
}

long ThorPMT::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	if(deviceType == PMT1)
	{
		pos = static_cast<double>(_pmt1Gain_C);
	}
	else if(deviceType == PMT2)
	{
		pos = static_cast<double>(_pmt2Gain_C);		
	}	
	else
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorPMT Invalid device specified for reading");
	}

	return ret;
}

long ThorPMT::PostflightPosition()
{
	return TRUE;
}

long ThorPMT::SetPMTEnable(int PMTID, bool enable)
{
	bool ret;
	TCHAR inst[32];
	StringCbPrintfW(inst,32, _T("pmt%d=%d"), PMTID, enable);
	if (ret=(TRUE==_serialPort.Write(inst)))
		ret=(0 < ReadPMTPort());
	return ret;
}

long ThorPMT::SetPMTGain(int PMTID, int gain)
{
	TCHAR inst[32];
	bool ret;
	if (gain<0||gain>100)
		return false;
	int bytegain= (int) GainPercent2Byte(gain);
	StringCbPrintfW(inst,32, _T("pmt%dgain=%d"), PMTID, bytegain);
	if (ret=(TRUE==_serialPort.Write(inst)))
		ret=(0 < ReadPMTPort());
	return ret;
}

long ThorPMT::SetCRSEnable(int enable)
{
	TCHAR inst[32];
	bool ret;
	StringCbPrintfW(inst,32, _T("scan=%d"), enable);
	if (ret=(TRUE==_serialPort.Write(inst)))
		ret=(0 < ReadPMTPort());
	return ret;
}

long ThorPMT::QueryStatus(void)
{
	bool ret;
	if (ret=(TRUE==_serialPort.Write(_T("statword?"))))
	{
		if (ret=(0 < ReadPMTQuery()))
		{
			ret=(TRUE==ParseReadBuffer(PMTSTATUS));
		}
	}
	return ret;
}

long ThorPMT::QueryID()
{
	long ret = FALSE;

	if (_serialPort.Write(_T("id?")))
	{
		if (ReadPMTPort())
		{
			if(wcsstr(_readBuffer,L"PCU2A"))
			{
				ret = TRUE;
			}
		}
	}

	return ret;

}

long ThorPMT::ReadPMTPort()
{	
	return _serialPort.Read(_readBuffer, 1000);
}

long ThorPMT::ReadPMTQuery()
{
	return _serialPort.ReadQuery(_readBuffer, 1000);
}

long ThorPMT::ParseReadBuffer(int inquiryID)
{
	char c;
	int v[10];
	int ret;
	int dummy;
	int i;
	switch (inquiryID)
	{
	case PMT1GAIN:
		ret=swscanf_s(_readBuffer, _T("PMT1GAIN=%d"), &_r_pmt1Gain);
		break;
	case PMT2GAIN:
		ret=swscanf_s(_readBuffer, _T("PMT2GAIN=%d"), &_r_pmt2Gain);
		break;
	case PMT1ENABLE:
	case PMT2ENABLE:
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
		_r_pmt1Enable=(TRUE==v[0]);
		_r_pmt2Enable=(TRUE==v[1]);
		_r_crsEnable=(TRUE==v[5]);
		_r_pmtError=(TRUE==v[6]);
		ret=swscanf_s(_readBuffer, _T("statword?\r\n%d\r\n%d,%d,%d"), &dummy, &_r_pmt1Gain, &_r_pmt2Gain,&_r_pmtError);
		break;
	}
	if (ret==0||ret==EOF)
		return false;
	else return true;
}

long ThorPMT::GainPercent2Byte(int percentageGain)
{
	double actualGain= static_cast<double>(percentageGain)/100.0*(GAINHBOUND-GAINLBOUND)+GAINLBOUND;
	double bytegain=pow(actualGain/GAINHBOUND,1/PMTGAINPOWER)*GAINMAX;
	return static_cast<long>(ceil(bytegain-0.5));
}

long ThorPMT::GainByte2Percent(int byteGain)
{
	double percentageGain=(GAINHBOUND*pow(static_cast<double>(byteGain)/GAINMAX, PMTGAINPOWER)-GAINLBOUND)/(GAINHBOUND-GAINLBOUND)*100.0;
	return static_cast<long>(ceil(percentageGain - 0.5));
}

long ThorPMT::GetLastErrorMsg(wchar_t *msg, long size)
{	
	wcsncpy_s(msg,MSG_SIZE,_errMsg,MSG_SIZE);
	return TRUE;
}