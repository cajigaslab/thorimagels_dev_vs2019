// ThorPowerControl.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Serial.h"
#include "ThorPowerControl.h"
#include "PowerControlXML.h"

#include "Strsafe.h"

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

wchar_t message[256];

ThorPowerControl::ThorPowerControl()
{
	_rPos=0;
	_rPos_C=POWER_MIN-1;
	_rPos_B=FALSE;

	_rHome_B=TRUE;

	_paramZeroPos = 0;

	_paramZero = FALSE;
	_paramZero_C = FALSE;
	_paramZero_B = FALSE;
	_blockUpdateParam = FALSE;
}

ThorPowerControl::~ThorPowerControl()
{
	_instanceFlag = false;
	_errMsg[0] = 0;
}

bool ThorPowerControl::_instanceFlag = false;

auto_ptr<ThorPowerControl> ThorPowerControl::_single (new ThorPowerControl());

CritSect ThorPowerControl::critSec;

ThorPowerControl* ThorPowerControl::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorPowerControl());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}


long ThorPowerControl::FindDevices(long &deviceCount)
{
	long ret = TRUE;

	//Get portID, etc from hardware setup.xml
	auto_ptr<PowerControlXML> pSetup(new PowerControlXML());

	long portID=0;
	long baudRate=0;
	long address=0;
	pSetup->GetConnection(portID, baudRate, address);


	if(FALSE == _serialPort.Open(portID, baudRate))
	{
		StringCbPrintfW(message,MSG_SIZE,L"ThorPowerControl FindDevices could not open serial port");
		LogMessage(message);
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorPowerControl FindDevices could not open serial port or configuration file is not avaible.");

		// *TODO* perform an automatic scan of the available serial ports
		deviceCount = 0;
		ret = FALSE;
	}
	else
	{
		char sendBuf[100];
		memset(sendBuf,0,sizeof(sendBuf));

		const char* READ_POS_CMD = "/1?8\r";
		sprintf_s((char*)sendBuf,100,READ_POS_CMD);

		_serialPort.SendData((const unsigned char*)sendBuf, static_cast<int>(strlen((const char*)sendBuf)));

		Sleep(100);
		memset(sendBuf,0,sizeof(sendBuf));

		_serialPort.ReadData(sendBuf, 100);


		//close serial port connection if wrong port is talking to
		if(FALSE == ValidateResponseFormat(sendBuf))
		{
			ret = FALSE;
			_serialPort.Close(); 
		}
		else
		{
			deviceCount = 1;
			_deviceDetected = TRUE;
			_address = static_cast<unsigned char>(address);
			ret = TRUE; 
		}
	}

	return ret;
}


long ThorPowerControl::SelectDevice(const long device)
{
	long ret = FALSE;

	if(FALSE == _deviceDetected)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"The device has not been detected");
		return FALSE;
	}

	switch(device)
	{
	case 0:
		{
			if(_rHome_B)
			{
				char sendBuf[100];

				memset(sendBuf,0,sizeof(sendBuf));

				const char* HOME_CMD = "/1ar5073R\r";
				sprintf_s((char*)sendBuf,100,HOME_CMD); 

				_serialPort.SendData((const unsigned char*)sendBuf, static_cast<int>(strlen((const char*)sendBuf)));

				Sleep(1500);

				_serialPort.ReadData(sendBuf, 100);

				_rPos_C = 0;
				ret = TRUE;

				_rHome_B = FALSE;
			}

			ret = TRUE;
		}
		break;
	default:
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"The device index %d is invalid", device);
		}
	}
	return ret;
}

void ThorPowerControl::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

long ThorPowerControl::TeardownDevice()
{	
	_serialPort.Close();
	return TRUE;
}

long ThorPowerControl::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_POWER_POS_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = POWER_MIN;
			paramMax = POWER_MAX;
			paramDefault = POWER_DEFAULT;
		}
		break;
	case PARAM_POWER_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = POWER_MIN;
			paramMax = POWER_MAX;
			paramDefault = POWER_DEFAULT;
		}
		break;

	case PARAM_POWER_HOME:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = POWER_HOME_MIN;
			paramMax = POWER_HOME_MAX;
			paramDefault = POWER_HOME_DEFAULT;
		}
		break;

	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = POWER_REG ;
		}
		break;

	case PARAM_POWER_VELOCITY:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = POWER_VELOCITY_MIN;
			paramMax = POWER_VELOCITY_MAX;
			paramDefault = POWER_VELOCITY_DEFAULT;
		}
		break;
	case PARAM_POWER_ZERO_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = POWER_ZERO_POS_MIN;
			paramMax = POWER_ZERO_POS_MAX;
			paramDefault = POWER_ZERO_POS_DEFAULT;
		}
		break;
	case PARAM_POWER_ZERO:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
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
	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}
	return ret;
}

long ThorPowerControl::SetParam(const long paramID, const double param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_POWER_POS:
		{
			if((param >= POWER_MIN) && (param <= POWER_MAX))
			{							
				_rPos = param;
			}
			else
			{
				ret = FALSE;
			}
			if(FALSE == ret)
			{
				StringCbPrintfW(message,MSG_SIZE, L"ThorPowerControl Set R Pos failed");
				LogMessage(message);

				StringCbPrintfW(_errMsg,MSG_SIZE, L"ThorPowerControl Set R Pos failed");
				//logDll->TLTraceEvent(ERROR_EVENT, 1, message);
			}
		}
		break;

	case PARAM_POWER_HOME:
		{
			if((param >= POWER_HOME_MIN) && (param <= POWER_HOME_MAX))
			{
				_rHome_B = TRUE;
			}
			else
			{
				ret = FALSE;
			}
			if(FALSE == ret)
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorPowerControl Set R Home failed");
				LogMessage(message);

				StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorPowerControl Set R Home failed");
				//logDll->TLTraceEvent(ERROR_EVENT,1,message);
			}
		}
		break;

	case PARAM_POWER_ZERO:
		{
			if((param >= FALSE) && (param <= TRUE))
			{
				_paramZero = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
			if(FALSE == ret)
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorPowerControl Set PARAM_POWER_ZERO_POS failed");
				LogMessage(message);

				StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorPowerControl Set PARAM_POWER_ZERO_POS failed");
				//logDll->TLTraceEvent(ERROR_EVENT,1,message);
			}
		}
		break;

	case PARAM_POWER_ZERO_POS:
		{
			if((param >= POWER_ZERO_POS_MIN) && (param <= POWER_ZERO_POS_MAX))
			{
				_paramZeroPos = static_cast<long>(param);
			}
			else
			{
				ret = FALSE;
			}
			if(FALSE == ret)
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorPowerControl Set PARAM_POWER_ZERO_POS failed");
				LogMessage(message);

				StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorPowerControl Set PARAM_POWER_ZERO_POS failed");
				//logDll->TLTraceEvent(ERROR_EVENT,1,message);
			}
		}
		break;

	default:
		ret = FALSE;
	}

	return ret;
}

long ThorPowerControl::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_POWER_POS_CURRENT:
		{		
			ReadPosition(POWER_REG, param);		
			//Only update param and _rPos and _rPos_C when not executing StartPosition()
			if (FALSE == _blockUpdateParam)
			{
				_rPos = _rPos_C = param;
			}
		}
		break;
	case PARAM_POWER_HOME:
		{
			param = POWER_HOME_MIN;
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			param = POWER_REG;
		}
		break;
	case PARAM_POWER_ZERO_POS:
		{
			param = static_cast<double>(_paramZeroPos);
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = (_deviceDetected) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
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
long ThorPowerControl::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPowerControl::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPowerControl::SetParamString(const long paramID, wchar_t* str)
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
long ThorPowerControl::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorPowerControl::PreflightPosition()
{
	return TRUE;
}

long ThorPowerControl::SetupPosition()
{
	if(_rPos != _rPos_C)
	{
		_rPos_B = TRUE;
	}

	if(TRUE == _paramZero && _paramZero_C != _paramZero)
	{
		_paramZero_B = TRUE;
	}

	return TRUE;
}

long ThorPowerControl::StartPosition()
{
	_blockUpdateParam = TRUE; //Do not allow the param to be updated in GetParam when reading while in this function
	long ret = FALSE;
	if(_rHome_B)
	{
		char sendBuf[100];

		memset(sendBuf,0,sizeof(sendBuf));

		sprintf_s((char*)sendBuf,100,"/1ar5073R\r");

		_serialPort.SendData((const unsigned char*)sendBuf, static_cast<int>(strlen((const char*)sendBuf)));

		_serialPort.ReadData(sendBuf, 100);

		_rPos_C = 0;
		ret = TRUE;

		_rHome_B = FALSE;
	}	
	long movedR = FALSE;

	if(_rPos_B)
	{
		double ret = 0;

		LinearizeSine(0,_rPos,ret);
		long p = static_cast<long>(ret) + _paramZeroPos;
		long currentPos = 0;
		const char* READ_POS_CMD = "/1?8\r";
		ExecuteCmd(PARAM_POWER_POS_CURRENT,READ_POS_CMD, currentPos);

		if (0 > p)
		{			
			long ret = 0;
			char cmd[100];
			memset(cmd,0,sizeof(cmd));

			const double TOTAL_COUNTS_PER_MOTOR_REVOLUTION = 691710.0;
			long moveCounts = static_cast<long>(TOTAL_COUNTS_PER_MOTOR_REVOLUTION)+p;

			const char* MOVE_ABS_CMD = "/1A%dR\r";
			sprintf_s((char*)cmd,100,MOVE_ABS_CMD,moveCounts);
			ExecuteCmd(PARAM_POWER_POS,(const char *)cmd, ret);
			const long MIN_WAIT_MS = 500;
			Sleep(max(MIN_WAIT_MS,static_cast<long>(abs(currentPos - moveCounts))/(18)));
		}
		else
		{
			long ret = 0;
			char cmd[100];
			memset(cmd,0,sizeof(cmd));
			const char* MOVE_ABS_CMD = "/1A%dR\r";			
			sprintf_s((char*)cmd,100,MOVE_ABS_CMD,p);
			ExecuteCmd(PARAM_POWER_POS,(const char *)cmd, ret);
			const long MIN_WAIT_MS = 500;

			Sleep(max(MIN_WAIT_MS, static_cast<long>(abs(currentPos - p))/(18)));
		}



		_rPos_C = _rPos;

		StringCbPrintfW(message,MSG_SIZE,L"MoveAbsoluteR %d.%d",(int)_rPos_C,(int)((_rPos_C - static_cast<long>(_rPos_C))*1000));
		//logDll->TLTraceEvent(VERBOSE_EVENT,1,message);			

		ret = TRUE;

		_rPos_B = FALSE;
		movedR = TRUE;
	}
	else
	{
		if (_rPos == _rPos_C)
		{
			ret = TRUE;
		}
	}

	if (_paramZero_B)
	{
		long ret = 0;
		const char* READ_POS_CMD = "/1?8\r";
		ExecuteCmd(PARAM_POWER_ZERO,READ_POS_CMD, ret);
		_paramZero_B = FALSE;
		_paramZero_C = _paramZero;
	}
	else if (_paramZero != _paramZero_C)
	{
		_paramZero_C = _paramZero;
	}
	_blockUpdateParam = FALSE; //Allow the param to be updated in GetParam when StartPosition end

	return ret;
}

long ThorPowerControl::StatusPosition(long &status)
{
	long ret = TRUE;

	status = IDevice::STATUS_READY;

	return ret;
}

long ThorPowerControl::ReadPosition(DeviceType deviceType,double &pos)
{
	long ret = FALSE;

	if(deviceType & POWER_REG)
	{		
		long currentPos = 0;
		const char* READ_POS_CMD = "/1?8\r";
		ExecuteCmd(PARAM_POWER_POS_CURRENT,READ_POS_CMD, currentPos);
		const double TOTAL_COUNTS_PER_MOTOR_REVOLUTION = 691710.0;
		if (TOTAL_COUNTS_PER_MOTOR_REVOLUTION/8 > TOTAL_COUNTS_PER_MOTOR_REVOLUTION - currentPos)
		{
			currentPos = currentPos - static_cast<long>(TOTAL_COUNTS_PER_MOTOR_REVOLUTION);
		}
		LinearizeSine(1,currentPos - _paramZeroPos,pos);

		ret = TRUE;
	}

	return ret;
}

void ThorPowerControl::LinearizeSine(long direction, double val, double &ret)
{
	//linearize the sine wave response
	const double AREA_UNDER_CURVE = 2.0;
	const double NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION = 8.0;
	const double MAX_POWER_INPUT_POSITION = 100.0;
	const double TOTAL_COUNTS_PER_MOTOR_REVOLUTION = 691710.0;
	const double MOTOR_CONVERSION_VALUE = MAX_POWER_INPUT_POSITION/(TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION);

	switch(direction)
	{
	case 0://GUI to device
		{
			ret = 2 * asin(sqrt(val/MAX_POWER_INPUT_POSITION)) * MAX_POWER_INPUT_POSITION /  PI;
			ret = ret / MOTOR_CONVERSION_VALUE;
		}
		break;

	case 1://device to GUI
		{
			double offset = 0.0;

			if(val > TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION)
			{
				offset = floor(val/(TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION));
				val -= offset * (TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION);
			}
			else if(val < 0)
			{
				offset =  floor(val/(TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION));
				val += -1.0 * offset * (TOTAL_COUNTS_PER_MOTOR_REVOLUTION/NUMBER_OF_POWER_ZONES_PER_MOTOR_REVOLUTION);
			}

			ret = val * MOTOR_CONVERSION_VALUE;

			ret =  offset * MAX_POWER_INPUT_POSITION + MAX_POWER_INPUT_POSITION * pow(sin(ret * PI /(2*MAX_POWER_INPUT_POSITION)),2);
		}
		break;
	}
}

long ThorPowerControl::PostflightPosition()
{
	return TRUE;
}

long ThorPowerControl::GetLastErrorMsg(wchar_t *msg, long size)
{
	wcsncpy_s(msg,size,_errMsg,MSG_SIZE);
	return TRUE;
}

long ThorPowerControl::ExecuteCmd(long paramID, const char* cmd, long &readBackValue)
{
	Lock lock(critSec);

	switch(paramID)		
	{
	case PARAM_POWER_POS:
		{	
			_serialPort.SendData((const unsigned char*)(cmd), static_cast<int>(strlen((const char*)cmd)));
			Sleep(50);
			char buf[100];
			memset(buf, 0, sizeof(buf));
			_serialPort.ReadData(buf, 100);
		}
		break;
	case PARAM_POWER_POS_CURRENT:
		{
			_serialPort.SendData((const unsigned char*)(cmd), static_cast<int>(strlen((const char*)cmd)));
			Sleep(50);

			char buf[100];
			memset(buf, 0, sizeof(buf));
			_serialPort.ReadData(buf, 100);

			char tmp;
			char tmp2;
			sscanf_s(buf,"%c/0%*c%ld%*s", &tmp, &tmp2, &readBackValue, sizeof(buf));

		}
		break;
	case PARAM_POWER_ZERO:
		{
			//the user has asked to set the current location as the zero
			//query the position and set the zeroes
			_serialPort.SendData((const unsigned char*)(cmd), static_cast<int>(strlen((const char*)cmd)));
			Sleep(100);

			char buf[100];
			memset(buf, 0, sizeof(buf));

			_serialPort.ReadData(buf, 100);

			char tmp;
			char tmp2;
			sscanf_s(buf,"%c/0%*c%ld%*s", &tmp, &tmp2, &readBackValue, sizeof(buf));
			_paramZeroPos = readBackValue;
		}
		break;
	default:
		{
			_serialPort.SendData((const unsigned char*)(cmd), static_cast<int>(strlen((const char*)cmd)));
			Sleep(100);
			char buf[100];
			memset(buf, 0, sizeof(buf));
			_serialPort.ReadData(buf, 100);
		}
		break;
	}

	return TRUE;
}

long ThorPowerControl::ValidateResponseFormat(char* response)
{
	int len = static_cast<int>(strlen(response));

	if(MSG_SIZE < len)
		return FALSE;
	char header[] = "\xFF/0";
	char end[] = "\x03\x0D\x0A";

	char* pHeader = strstr(response, header);
	char* pEnd = strstr(response, end);

	if((pHeader == NULL) || (pEnd == NULL))
	{
		return FALSE;
	}
	else
		return TRUE;
}