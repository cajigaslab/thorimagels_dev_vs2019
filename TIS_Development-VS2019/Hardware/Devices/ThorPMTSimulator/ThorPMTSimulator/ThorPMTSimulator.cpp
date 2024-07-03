// ThorPMTSimulator.cpp : Defines the exported functions for the DLL application.
//

//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorPMTSimulator.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "ThorPMTSimulator.h"
#include "ThorPMTSetupXML.h"
//#include <tchar.h>

#define READ_TIMEOUT 1000
#define PMTGAINPOWER 6.64
#define GAINLBOUND 0.0005
#define GAINHBOUND 0.25

wchar_t message[256];


ThorPMTSimulator::ThorPMTSimulator()
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
	_crsEnable_C=true;
	_alignPos = 0;

	_deviceDetected = FALSE;
}

ThorPMTSimulator::~ThorPMTSimulator()
{
	_instanceFlag = false;
}

bool ThorPMTSimulator:: _instanceFlag = false;

auto_ptr<ThorPMTSimulator> ThorPMTSimulator::_single(new ThorPMTSimulator());

ThorPMTSimulator *ThorPMTSimulator::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorPMTSimulator());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorPMTSimulator::FindDevices(long &deviceCount)
{
	long	ret = TRUE;

	//if(_deviceDetected)
	//{
	//	_serialPort.Close();
	//	_deviceDetected = FALSE;
	//}

	////Get filter parameters from hardware setup.xml
	//auto_ptr<ThorPMTXML> pSetup(new ThorPMTXML());

	//long portID=0;
	//pSetup->GetConnection(portID);

	//TCHAR PortName[32];
	//wsprintf(PortName, _T("COM%d"), portID);

	//if(FALSE == _serialPort.Open(PortName))
	//{
	//	logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ThorPMTSimulator FindDevices could not open serial port");

	//	// *TODO* perform an automatic scane of the available serial ports for the Z stepper
	//	deviceCount = 0;
	//}
	//else
	//{
	//	deviceCount = 1;
	//	_deviceDetected = TRUE;
	//}

	deviceCount = 1;
	_deviceCount = deviceCount;
	_deviceDetected = TRUE;

	return ret;
}

long ThorPMTSimulator::SelectDevice(const long device)
{
	long	ret = FALSE;

	if(FALSE == _deviceDetected || device < 0 || device > _deviceCount - 1)
	{
		return FALSE;
	}

	//switch(device)
	//{
	//case 0:		{ ret = TRUE; }break;
	//default:	{ }
	//}

	ret = TRUE;
	return ret;
}

long ThorPMTSimulator::TeardownDevice()
{
	//_serialPort.Close();
	return TRUE;
}

long ThorPMTSimulator::GetParamInfo
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
			paramReadOnly = FALSE;
			paramMin = CONNECTION_READY;
			paramMax = CONNECTION_READY;
			paramDefault = CONNECTION_READY;
		}
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
		
	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = PMT1 | PMT2 | PMT3 | PMT4 | CONTROL_UNIT;
		}
		break;

	case PARAM_SCANNER_ALIGN_POS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 255;
		paramDefault = 0;
	}
	break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long ThorPMTSimulator::SetParam(const long paramID, const double param)
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
				ret = FALSE;
			}
		}
		break;
	case PARAM_SCANNER_ALIGN_POS:
	{
		_alignPos = static_cast<long>(param);
	}
	break;
	}

	return ret;
}

long ThorPMTSimulator::GetParam(const long paramID, double &param)
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
	case PARAM_SCANNER_ENABLE:
		{
			param = static_cast<double>(_crsEnable);
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(PMT1 | PMT2 | PMT3 | PMT4 | CONTROL_UNIT);
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = static_cast<long>(CONNECTION_READY);
		}
		break;
	case PARAM_SCANNER_ALIGN_POS:
	{
		param = static_cast<double>(_alignPos);
	}
	break;
	default:
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
long ThorPMTSimulator::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPMTSimulator::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPMTSimulator::SetParamString(const long paramID, wchar_t* str)
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
long ThorPMTSimulator::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorPMTSimulator::PreflightPosition()
{
	return TRUE;
}

long ThorPMTSimulator::SetupPosition()
{
	return TRUE;
}

long ThorPMTSimulator::StartPosition()
{
	long ret = TRUE;

	/*if(_pmt1Enable != _pmt1Enable_C)
	{
		ret=SetPMTEnable(1, static_cast<bool> (_pmt1Enable));
		if(TRUE == ret)
		{
			_pmt1Enable_C = _pmt1Enable;
		}
	}

	if(_pmt1Gain != _pmt1Gain_C)
	{
		ret=SetPMTGain(1, static_cast<int> (_pmt1Gain));
		if(TRUE == ret)
		{
			_pmt1Gain_C = _pmt1Gain;
		}
	}

	if(_pmt2Enable != _pmt2Enable_C)
	{
		ret=SetPMTEnable(2, static_cast<bool> (_pmt2Enable));
		if(TRUE == ret)
		{
			_pmt2Enable_C = _pmt2Enable;
		}
	}

	if(_pmt2Gain != _pmt2Gain_C)
	{
		ret=SetPMTGain(2,static_cast<int> (_pmt2Gain));
		if(TRUE == ret)
		{
			_pmt2Gain_C = _pmt2Gain;
		}
	}

	if(_crsEnable != _crsEnable_C)
	{
		ret=SetCRSEnable(_crsEnable);
		if(TRUE == ret)
		{
			_crsEnable_C = _crsEnable;
		}
	}*/
	return ret;
}

long ThorPMTSimulator::StatusPosition(long &status)
{
	long	ret = TRUE;
	status = IDevice::STATUS_READY;
	return ret;
}

long ThorPMTSimulator::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	if(deviceType == PMT1)
	{
		pos = static_cast<double>(_pmt1Gain);
		ret = TRUE;
	}
	else if(deviceType == PMT2)
	{
		pos = static_cast<double>(_pmt2Gain);
		ret = TRUE;
	}
	else if(deviceType == PMT3)
	{
		pos = static_cast<double>(_pmt3Gain);
		ret = TRUE;
	}
	else if(deviceType == PMT4)
	{
		pos = static_cast<double>(_pmt4Gain);
		ret = TRUE;
	}

	return ret;
}

long ThorPMTSimulator::PostflightPosition()
{
	return TRUE;
}

long ThorPMTSimulator::SetPMTEnable(int PMTID, bool enable)
{
	/*bool ret;
	TCHAR inst[32];
	wsprintf(inst, _T("pmt%d=%d"), PMTID, enable);
	if (ret=_serialPort.Write(inst))
		ret=ReadPMTPort();
	return ret;*/

	return TRUE;
}

long ThorPMTSimulator::SetPMTGain(int PMTID, int gain)
{
	/*TCHAR inst[32];
	bool ret;
	if (gain<0||gain>100)
		return false;
	int bytegain= (int) GainPercent2Byte(gain);
	wsprintf(inst, _T("pmt%dgain=%d"), PMTID, bytegain);
	if (ret=_serialPort.Write(inst))
		ret=ReadPMTPort();
	return ret;*/

	return TRUE;
}

long ThorPMTSimulator::SetCRSEnable(bool enable)
{
	/*TCHAR inst[32];
	bool ret;
	wsprintf(inst, _T("scan=%d"), enable);
	if (ret=_serialPort.Write(inst))
		ret=ReadPMTPort();
	return ret;*/

	return TRUE;
}

long ThorPMTSimulator::QueryStatus(void)
{
	/*bool ret;
	if (ret=_serialPort.Write(_T("statword?")))
		if (ret=ReadPMTPort())
			ret=ParseReadBuffer(PMTSTATUS);
	return ret;*/

	return TRUE;
}


long ThorPMTSimulator::ReadPMTPort()
{	
	//return _serialPort.Read(_readBuffer, 1000);

	return TRUE;
}

long ThorPMTSimulator::ParseReadBuffer(int inquiryID)
{
	/*char c;
	int v[10];
	int ret;
	int dummy;
	int i;
	switch (inquiryID)
	{
	case PMT1GAIN:
		ret=swscanf_s(_readBuffer, _T("PMT1GAIN=%d"), &r_pmt1Gain);
		break;
	case PMT2GAIN:
		ret=swscanf_s(_readBuffer, _T("PMT2GAIN=%d"), &r_pmt2Gain);
		break;
	case PMT1ENABLE:
	case PMT2ENABLE:
		break;
	case PMTSTATUS:
		for (i=11; i<19; i++)
		{
			c=(char) _readBuffer[i];
			v[i-11]=atoi(&c);
		}
		r_pmt1Enable=(bool) v[0];
		r_pmt2Enable=(bool) v[1];
		r_crsEnable=(bool) v[5];
		ret=swscanf_s(_readBuffer, _T("statword?\r\n%d\r\n%d,%d"), &dummy, &r_pmt1Gain, &r_pmt2Gain);
		break;
	}
	if (ret==0||ret==EOF)
		return false;
	else */
	return true;
}

long ThorPMTSimulator::GainPercent2Byte(int percentageGain)
{
	/*double actualGain= (double) percentageGain/100.0*(GAINHBOUND-GAINLBOUND)+GAINLBOUND;
	double bytegain=pow((double) actualGain, 1/PMTGAINPOWER)*255;
	return (long) bytegain;*/

	return 1;

}
long ThorPMTSimulator::GainByte2Percent(int byteGain)
{/*
	double percentageGain=(pow((double) byteGain/255.0, PMTGAINPOWER)-GAINLBOUND)/(GAINHBOUND-GAINLBOUND)*100.0;
	return (long) percentageGain;*/

	return 1;
}


long ThorPMTSimulator::GetLastErrorMsg(wchar_t *msg, long size)
{
	msg = NULL;
	return TRUE;
}