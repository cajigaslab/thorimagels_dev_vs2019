// ThorMLSStage.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "serial.h"
#include "ThorMLSStage.h"
#include "Strsafe.h"
#include "MLSStageSetupXML.h"

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

#define VERBOSE					0

/// <summary>
/// The message
/// </summary>
wchar_t message[256];

/// <summary>
/// Prevents a default instance of the <see cref="ThorMLSStage"/> class from being created.
/// </summary>
ThorMLSStage::ThorMLSStage()
{
	_xMin = 0;
	_xMax = 110;
	_yMin = 0;
	_yMax = 75;

	_xDefault = 0;
	_yDefault = 0;

	_xPos=0;
	_xPos_C=_xMin-1;
	_xPos_B=FALSE;

	_yPos=0;
	_yPos_C=_yMin-1;
	_yPos_B=FALSE;

	_xHome_B=FALSE;

	_yHome_B=FALSE;

	_xVel=X_VELOCITY_DEFAULT;
	_xVel_C=(double)X_VELOCITY_DEFAULT-1;
	_xAcc=ACCELERATION_DEFAULT;
	_xAcc_C=(double)ACCELERATION_DEFAULT-1;
	_xVel_B=FALSE;

	_yVel=Y_VELOCITY_DEFAULT;
	_yVel_C=(double)Y_VELOCITY_DEFAULT-1;
	_yAcc=ACCELERATION_DEFAULT;
	_yAcc_C=(double)ACCELERATION_DEFAULT-1;
	_yVel_B=FALSE;

	_blockUpdateParam = FALSE;

	_comAliveCounter=0;

	_xEncoderToMM = 20000;
	_yEncoderToMM = 20000;
	_xInvert = false;
	_yInvert = false;

	_motorsParamsInvert = new double[2];
	memset(_motorsParamsInvert,1,2*sizeof(double));
	_deviceDetected = FALSE;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorMLSStage"/> class.
/// </summary>
ThorMLSStage::~ThorMLSStage()
{
	delete[] _motorsParamsInvert;
	_motorsParamsInvert = NULL;
	_instanceFlag = false;
}

bool ThorMLSStage::_instanceFlag = false;

ThorMLSStage* ThorMLSStage::_single = NULL;

const long ThorMLSStage::MAX_SERIAL_BUFFER_SIZE = 256;
const long ThorMLSStage::ENCODER_TO_MM_PER_SEC_CONVERSION = 134218;
//const long ThorMLSStage::ENCODER_TO_MM_PER_SEC_CONVERSION = 77175.35;
const double ThorMLSStage::ENCODER_TO_MM_PER_SEC_SQR_CONVERSION = 13.7439;
const long ThorMLSStage::WAIT_TO_READ_DELAY = 50;
DWORD ThorMLSStage::_lastReadTime = 0;

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorMLSStage *.</returns>
ThorMLSStage* ThorMLSStage::getInstance()
{
	if(! _instanceFlag)
	{
		_single = new ThorMLSStage();
		_instanceFlag = true;

		return _single;
	}
	else
	{
		return _single;
	}
}


/// <summary>
/// Sets the  hinstance.
/// </summary>
/// <param name="hinst">The hinst.</param>
void ThorMLSStage::SetHInstance(HINSTANCE hinst)
{
	_hinstance = hinst;
}


/// <summary>
/// Gets the hinstance.
/// </summary>
/// <returns>HINSTANCE.</returns>
HINSTANCE ThorMLSStage::GetHInstance()
{
	return _hinstance;
}


/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="deviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorMLSStage::FindDevices(long &deviceCount)
{
	long ret = FALSE;

	long portID=0;
	long baudRate=0;
	long address=0;

	try
	{
		//Get filter parameters from hardware setup.xml
		auto_ptr<MLSStageXML> pSetup(new MLSStageXML());
		pSetup->GetConnection(portID,baudRate,address);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorMLSStageSettings.xml file");
		std::cout<<"Unable to locate ThorMLSStageSettings.xml file. portID="<<portID<<std::endl;
		deviceCount = 0;
		return FALSE;
	}

	if(FALSE == _serialPort.Open(portID, baudRate))
	{
		StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage FindDevices could not open serial port or configuration file is not avaible.");
		LogMessage(message,ERROR_EVENT);

		deviceCount = 0;
		ret = FALSE;
	}
	else 
	{
		// check the status on both X and Y to verify the connection is correct

		long status = 0;
		StatusPosition(status);

		if(status == STATUS_READY)
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage FindDevices Status Ready");
			LogMessage(message,ERROR_EVENT);

			deviceCount = 1;
			_deviceDetected = TRUE;
			_serialPort.Close();
		}
		else
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage FindDevices does not find XY stage");
			LogMessage(message,ERROR_EVENT);

			deviceCount = 0;
			_serialPort.Close();
		}
	}

	ret = TRUE;

	return ret;
}


/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorMLSStage::SelectDevice(const long device)
{
	long ret = FALSE;

	switch(device)
	{
	case 0:
		{		
			long portID=0;
			long baudRate=0;
			long address=0;

			try
			{
				//Get filter parameters from hardware setup.xml
				auto_ptr<MLSStageXML> pSetup(new MLSStageXML());

				pSetup->GetConnection(portID,baudRate,address);
				pSetup->GetXAxisConfig(_xInvert);
				pSetup->GetYAxisConfig(_yInvert);
				_motorsParamsInvert[0] = (_xInvert == false)?1:-1;
				_motorsParamsInvert[1] = (_yInvert == false)?1:-1;
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorMLSStageSettings.xml file");
				return FALSE;
			}

			if(FALSE == _serialPort.Open(portID, baudRate))
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage FindDevices could not open serial port");
				LogMessage(message,ERROR_EVENT);
				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage FindDevices could not open serial port or configuration file is not avaible.");

				// *TODO* perform an automatic scan of the available serial ports for the Z stepper
				ret = FALSE;
				return ret;
			}


			GetXYStageRange();			
			_blockUpdateParam = FALSE;

			//Set _xPos and _yPos positions to the current stage position
			ReadPosition(STAGE_X,_xPos);
			ReadPosition(STAGE_Y,_yPos);
			ret = TRUE;
		}
		break;
	default:
		{
		}
	}
	return ret;
}


long ThorMLSStage::GetXYStageRange()
{
	long ret = 0;

	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];
	unsigned char readBuf[MAX_SERIAL_BUFFER_SIZE];

	memset(sendBuf,0,sizeof(sendBuf));

	sendBuf[0] = 0xF1;//message id
	sendBuf[1] = 0x04;//message id
	sendBuf[2] = 0x0;//channel id
	sendBuf[3] = 0x0;//channel id
	sendBuf[4] = 0xA1;//destination address
	sendBuf[5] = 0x1;//source address (host)

	_serialPort.SendData((const unsigned char*)sendBuf, 6);

	Sleep(WAIT_TO_READ_DELAY);

	memset(readBuf,0,sizeof(readBuf));
	if(_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE) &&((readBuf[0] == 0xF2)&&(readBuf[1] == 0x04)&&(readBuf[5] == 0x21)))
	{
		_xEncoderToMM = (unsigned char)readBuf[32] | (unsigned char)readBuf[33] << 8 | (unsigned char)readBuf[34] << 16 | (unsigned char)readBuf[35] << 24;
		if (_xEncoderToMM == 0)
		{
			_xEncoderToMM = 20000;
		}
		int minPos = (unsigned char)readBuf[36] | (unsigned char)readBuf[37] << 8 | (unsigned char)readBuf[38] << 16 | (unsigned char)readBuf[39] << 24;
		int maxPos = (unsigned char)readBuf[40] | (unsigned char)readBuf[41] << 8 | (unsigned char)readBuf[42] << 16 | (unsigned char)readBuf[43] << 24;
		_xMin = minPos / _xEncoderToMM ;
		_xMax = maxPos / _xEncoderToMM;
	}else
	{
		_xMin = 0;
		_xMax = 110;
		_xEncoderToMM = 20000;
	}
	if (_xInvert)
	{
		double temp = _xMin;
		_xMin = _xMax * -1;
		_xMax = temp * -1;
	}
	Sleep(WAIT_TO_READ_DELAY);

	memset(sendBuf,0,sizeof(sendBuf));

	sendBuf[0] = 0xF1;//message id
	sendBuf[1] = 0x04;//message id
	sendBuf[2] = 0x0;//channel id
	sendBuf[3] = 0x0;//channel id
	sendBuf[4] = 0xA2;//destination address
	sendBuf[5] = 0x1;//source address (host)

	_serialPort.SendData((const unsigned char*)sendBuf, 6);

	Sleep(WAIT_TO_READ_DELAY);

	memset(readBuf,0,sizeof(readBuf));
	if(_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE) &&((readBuf[0] == 0xF2)&&(readBuf[1] == 0x04)&&(readBuf[5] == 0x22)))
	{
		_yEncoderToMM = (unsigned char)readBuf[32] | (unsigned char)readBuf[33] << 8 | (unsigned char)readBuf[34] << 16 | (unsigned char)readBuf[35] << 24;
		if (_yEncoderToMM == 0)
		{
			_yEncoderToMM = 20000;
		}
		int minPos = (unsigned char)readBuf[36] | (unsigned char)readBuf[37] << 8 | (unsigned char)readBuf[38] << 16 | (unsigned char)readBuf[39] << 24;
		int maxPos = (unsigned char)readBuf[40] | (unsigned char)readBuf[41] << 8 | (unsigned char)readBuf[42] << 16 | (unsigned char)readBuf[43] << 24;
		_yMin = minPos / _yEncoderToMM;
		_yMax = maxPos / _yEncoderToMM;
	}else
	{
		_yMin = 0;
		_yMax = 75;
		_yEncoderToMM = 20000;
	}
	if (_yInvert)
	{
		double temp = _yMin;
		_yMin = _yMax * -1;
		_yMax = temp * -1;
	}
	return ret;
}


/// <summary>
/// Teardown the device.
/// </summary>
/// <returns>long.</returns>
long ThorMLSStage::TeardownDevice()
{
	_serialPort.Close();
	_deviceDetected = FALSE;
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorMLSStage::GetLastErrorMsg(wchar_t *msg, long size)
{
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
long ThorMLSStage::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_X_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _xMin;
			paramMax = _xMax;
			paramDefault = _xDefault;
		}
		break;

	case PARAM_Y_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _yMin;
			paramMax = _yMax;
			paramDefault = _yDefault;
		}
		break;

	case PARAM_X_HOME:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = X_HOME_MIN;
			paramMax = X_HOME_MAX;
			paramDefault = X_HOME_DEFAULT;
		}
		break;

	case PARAM_Y_HOME:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = Y_HOME_MIN;
			paramMax = Y_HOME_MAX;
			paramDefault = Y_HOME_DEFAULT;
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = STAGE_X | STAGE_Y;
		}
		break;

	case PARAM_X_VELOCITY:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = X_VELOCITY_MIN;
			paramMax = X_VELOCITY_MAX;
			paramDefault = X_VELOCITY_DEFAULT;
		}
		break;

	case PARAM_Y_VELOCITY:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = Y_VELOCITY_MIN;
			paramMax = Y_VELOCITY_MAX;
			paramDefault = Y_VELOCITY_DEFAULT;
		}
		break;
	case PARAM_Y_ACCEL:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = ACCELERATION_DEFAULT;
			paramDefault = ACCELERATION_DEFAULT;
		}
		break;

	case PARAM_X_POS_CURRENT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _xMin;
			paramMax = _xMax;
			paramDefault = _xDefault;
		}
		break;		
	case PARAM_Y_POS_CURRENT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _yMin;
			paramMax = _yMax;
			paramDefault = _yDefault;
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = (double)ConnectionStatusType::CONNECTION_READY;
			paramMax = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
			paramDefault = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
		break;
	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorMLSStage::SetParam(const long paramID, const double param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_X_POS:
		{
			if((param >= _xMin) && (param <= _xMax))
			{
				_xPos = static_cast<double>(param);
			}
			else
			{
				ret = FALSE;
			}
			if(FALSE == ret)
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Set X Pos failed");				
				LogMessage(message,ERROR_EVENT);
			}
		}
		break;

	case PARAM_Y_POS:
		{
			if((param >= _yMin) && (param <= _yMax))
			{
				_yPos = static_cast<double>(param);
			}
			else
			{
				ret = FALSE;
			}
			if(FALSE == ret)
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Set Y Pos failed");
				LogMessage(message,ERROR_EVENT);
			}
		}
		break;
	case PARAM_X_HOME:
		{
			if((param >= X_HOME_MIN) && (param <= X_HOME_MAX))
			{
				_xHome_B = TRUE;
			}
			else
			{
				ret = FALSE;
			}
			if(FALSE == ret)
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Set X Home failed");
				LogMessage(message,ERROR_EVENT);
			}
		}
		break;
	case PARAM_Y_HOME:
		{
			if((param >= Y_HOME_MIN) && (param <= Y_HOME_MAX))
			{
				_yHome_B = TRUE;
			}
			else
			{
				ret = FALSE;
			}

			if(FALSE == ret)
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Set Y Home failed");
				LogMessage(message,ERROR_EVENT);
			}
		}
		break;

	case PARAM_X_VELOCITY:
		{
			if((param >= X_VELOCITY_MIN) && (param <= X_VELOCITY_MAX))
			{
				_xVel = static_cast<double>(param);
			}		
			else
			{
				ret = FALSE;
			}

			if(FALSE == ret)
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Set X Velocity failed");
				LogMessage(message,ERROR_EVENT);
			}
		}
		break;

	case PARAM_Y_VELOCITY:
		{
			if((param >= Y_VELOCITY_MIN) && (param <= Y_VELOCITY_MAX))
			{
				_yVel = static_cast<double>(param);
			}		
			else
			{
				ret = FALSE;
			}

			if(FALSE == ret)
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Set Y Velocity failed");
				LogMessage(message,ERROR_EVENT);
			}
		}
		break;
	default:
		ret = FALSE;
	}

	if (TRUE == ret)
	{
		//Use _blockUpdateParam to to block updating any paremeter in the getParam funtion until after the new position
		//has been sent to the device. This variable allows GetParam calls in between SetParam and StartPosition, without
		//changing the new set position, until the command has been sent to the device. This device must be set to FALSE
		//at the end of StartPosition to allow updating the parameters when reading positions from the device. 
		_blockUpdateParam = TRUE;
	}

	return ret;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorMLSStage::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_X_POS:
		{
			param = static_cast<double>(_xPos);
		}
		break;
	case PARAM_Y_POS:
		{
			param = static_cast<double>(_yPos);
		}
		break;
	case PARAM_X_HOME:
		{
			param = X_HOME_MIN;
		}
		break;
	case PARAM_Y_HOME:
		{
			param = Y_HOME_MIN;
		}
		break;
	case PARAM_X_VELOCITY:
		{
			param = static_cast<double>(_xVel);
		}
		break;
	case PARAM_Y_VELOCITY:
		{
			param = static_cast<double>(_yVel);
		}
		break;
	case PARAM_X_ACCEL:
		{
			param = static_cast<double>(_xAcc);
		}
		break;
	case PARAM_Y_ACCEL:
		{
			param = static_cast<double>(_yAcc);
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			param = STAGE_X | STAGE_Y;
		}
		break;
		
	case PARAM_X_POS_CURRENT:
		{
			ReadPosition(STAGE_X,param);
		}
		break;
		
	case PARAM_Y_POS_CURRENT:
		{
			ReadPosition(STAGE_Y,param);
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = (_deviceDetected) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
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
long ThorMLSStage::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorMLSStage::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorMLSStage::SetParamString(const long paramID, wchar_t* str)
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
long ThorMLSStage::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorMLSStage::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorMLSStage::SetupPosition()
{
	double roundedXPos =  ceil(_xPos * 100000 - 0.5)/100000.0;
	double roundedXPos_C =  ceil(_xPos_C * 100000 - 0.5)/100000.0;
	if(roundedXPos != roundedXPos_C)
	{
		_xPos_B = TRUE;
	}

	double roundedYPos =  ceil(_yPos * 100000 - 0.5)/100000.0;
	double roundedYPos_C =  ceil(_yPos_C * 100000 - 0.5)/100000.0;
	if(roundedYPos != roundedYPos_C)
	{
		_yPos_B = TRUE;
	}

	if(_xVel != _xVel_C)
	{
		_xVel_B = TRUE;
	}

	if(_yVel != _yVel_C)
	{
		_yVel_B = TRUE;
	}

	return TRUE;
}


/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorMLSStage::StartPosition()
{
	Lock lock(_critSect);

	//Use _blockUpdateParamto to block updating any paremeter in the getParam funtion until after the new position
	//has been sent to the device. This variable allows GetParam calls in between SetParam and StartPosition, without
	//changing the new set position, until the command has been sent to the device. This device must be set to FALSE
	//at the end of StartPosition to allow updating the parameters when reading positions from the device. 
	_blockUpdateParam = TRUE;

	long ret = FALSE;

	const long ACCELERATION_RULE_OF_THUMB_MULTIPLIER = 10;
	if(_xVel_B)
	{
		//ReqVelocityX(xVel, xAcc);
		SetVelocityX(static_cast<long>(_xVel*ENCODER_TO_MM_PER_SEC_CONVERSION),
						static_cast<long>(_xVel*ACCELERATION_RULE_OF_THUMB_MULTIPLIER*ENCODER_TO_MM_PER_SEC_SQR_CONVERSION));
		_xVel_C = _xVel;
		_xAcc_C = _xAcc = _xVel*ACCELERATION_RULE_OF_THUMB_MULTIPLIER;

		ret = TRUE;

		_xVel_B = FALSE;
	}

	if(_yVel_B)
	{
		//ReqVelocityY(yVel, yAcc);
		SetVelocityY(static_cast<long>(_yVel*ENCODER_TO_MM_PER_SEC_CONVERSION), 
						static_cast<long>(_yVel*ACCELERATION_RULE_OF_THUMB_MULTIPLIER*ENCODER_TO_MM_PER_SEC_SQR_CONVERSION));
		_yVel_C = _yVel;
		_yAcc_C = _yAcc = _yVel*ACCELERATION_RULE_OF_THUMB_MULTIPLIER;

		ret = TRUE;

		_yVel_B = FALSE;
	}

	if(_xHome_B)
	{
		MoveStageX(0);
		_xPos_C = 0;

		_xHome_B = FALSE;
	}

	if(_yHome_B)
	{		
		MoveStageY(0);
		_yPos_C = 0;

		_yHome_B = FALSE;
	}

	long movedX = FALSE;

	if(_xPos_B)
	{
		MoveStageX(static_cast<long>(_xPos*_xEncoderToMM*_motorsParamsInvert[0]));
		_xPos_C = _xPos;
		ret = TRUE;		
		_xPos_B = FALSE;
	}

	if(_yPos_B && (FALSE == movedX))
	{

		
		MoveStageY(static_cast<long>(_yPos*_yEncoderToMM*_motorsParamsInvert[1]));
		_yPos_C = _yPos;
		ret = TRUE;
		_yPos_B = FALSE;
	}

	//CommAlive();
	_blockUpdateParam = FALSE; //After all commands have been sent to the device, allow the params to be updated when read from the device
	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorMLSStage::StatusPosition(long &status)
{	
	Lock lock(_critSect);

	//CommAlive();

	long ret = TRUE;

	//bit mask meaning
	const long FORWARD_LIMIT_SWITCH_ACTIVE	= 0x00000001;//0x00000001 forward hardware limit switch is active
	const long REVERSE_LIMIT_SWITCH_ACTIVE	= 0x00000002;//0x00000002 reverse hardware limit switch is active
	const long MOVING_FORWARD				= 0x00000010;//0x00000010 in motion, moving forward
	const long MOVING_REVERSE				= 0x00000020;//0x00000020 in motion, moving reverse
	const long JOGGING_FORWARD				= 0x00000040;//0x00000040 in motion, jogging forward
	const long JOGGING_REVERSE				= 0x00000080;//0x00000080 in motion, jogging reverse
	const long HOMING						= 0x00000200;//0x00000200 in motion, homing
	const long HOMING_COMPLETE				= 0x00000400;//0x00000400 homed (homing has been completed)
	const long TRACKING						= 0x00001000;//0x00001000 tracking
	const long SETTLED						= 0x00002000;//0x00002000 settled
	const long EXCESSIVE_POSITION_ERROR		= 0x00004000;//0x00004000 motion error (excessive position error)
	const long CURRENT_LIMIT_REACHED		= 0x01000000;//0x01000000 motor current limit reached
	const long CHANNEL_ENABLED				= 0x80000000;//0x80000000 channel is enabled

	status = IDevice::STATUS_BUSY;

	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];

	memset(sendBuf,0,sizeof(sendBuf));

	sendBuf[0] = 0x29;//message id
	sendBuf[1] = 0x4;//message id
	sendBuf[2] = 0x0;//channel identifier
	sendBuf[3] = 0x0;//channel identifier
	sendBuf[4] = 0xA1;//destination address
	sendBuf[5] = 0x1;//source address (host)

	long xReady = FALSE;
	long yReady = FALSE;

	unsigned char readBuf[MAX_SERIAL_BUFFER_SIZE];
	unsigned char devAddr;
	memset(readBuf,0,sizeof(readBuf));

	//check status for x and y axis
	for(long i = 0; i<2; i++)
	{
		switch(i)
		{
		case 0:	sendBuf[4]= 0xA1;devAddr = 0x21; 
				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage X Status");
				LogMessage(message,VERBOSE_EVENT);break;
		case 1:	sendBuf[4] = 0xA2;devAddr = 0x22;
				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Y Status");
				LogMessage(message,VERBOSE_EVENT); break;
		}

		_serialPort.SendData((const unsigned char*)sendBuf, 6);

		Sleep(WAIT_TO_READ_DELAY);

		if(_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE))
		{
			if((0x2A == readBuf[0])&&(0x04 == readBuf[1]))
			{
				//build the 4 byte status mask
				long statusVal;

				unsigned char * pVal = (unsigned char*)&statusVal;

				pVal[0] = readBuf[8];
				pVal[1] = readBuf[9];
				pVal[2] = readBuf[10];
				pVal[3] = readBuf[11];

				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Status Value %d",statusVal);
				LogMessage(message,ERROR_EVENT);

				if((MOVING_FORWARD & statusVal)||(MOVING_REVERSE & statusVal))
				{
					status = IDevice::STATUS_BUSY;
					StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Busy");
					LogMessage(message,VERBOSE_EVENT);
					break;
				}
				else if((EXCESSIVE_POSITION_ERROR & statusVal) ||
					(CURRENT_LIMIT_REACHED & statusVal))
				{
					status = IDevice::STATUS_ERROR;
					StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Error");
					LogMessage(message,ERROR_EVENT);
					break;
				}
				else
				{
					if(0x21 == readBuf[5])
					{
						xReady = TRUE;
						StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage X Ready");
						LogMessage(message,VERBOSE_EVENT);
					}
					else if(0x22 == readBuf[5])
					{
						yReady = TRUE;
						StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Y Ready");
						LogMessage(message,VERBOSE_EVENT);
					}

					if(xReady && yReady)
					{
						status = IDevice::STATUS_READY;
						StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Motion complete");
						LogMessage(message,VERBOSE_EVENT);
					}
				}
			}
			else
			{				
				StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Status Response unknown (%d) (%d) (%d)",readBuf[0],readBuf[1],readBuf[5]);
				LogMessage(message,ERROR_EVENT);
			}
		}
		else {
			StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage StatusPosition SerialPort ReadData failed");
			LogMessage(message,ERROR_EVENT);
			ret = FALSE;
		}
		Sleep(1);
	}

	return ret;
}

/// <summary>
/// Waits the until settled.
/// </summary>
/// <returns>long.</returns>
long ThorMLSStage::WaitUntilSettled ()
{	

	//CommAlive();

	long ret = TRUE;

	//bit mask meaning

	const long FORWARD_LIMIT_SWITCH_ACTIVE	= 0x00000001;//0x00000001 forward hardware limit switch is active
	const long REVERSE_LIMIT_SWITCH_ACTIVE	= 0x00000002;//0x00000002 reverse hardware limit switch is active
	const long MOVING_FORWARD				= 0x00000010;//0x00000010 in motion, moving forward
	const long MOVING_REVERSE				= 0x00000020;//0x00000020 in motion, moving reverse
	const long JOGGING_FORWARD				= 0x00000040;//0x00000040 in motion, jogging forward
	const long JOGGING_REVERSE				= 0x00000080;//0x00000080 in motion, jogging reverse
	const long HOMING						= 0x00000200;//0x00000200 in motion, homing
	const long HOMING_COMPLETE				= 0x00000400;//0x00000400 homed (homing has been completed)
	const long TRACKING						= 0x00001000;//0x00001000 tracking
	const long SETTLED						= 0x00002000;//0x00002000 settled
	const long EXCESSIVE_POSITION_ERROR		= 0x00004000;//0x00004000 motion error (excessive position error)
	const long CURRENT_LIMIT_REACHED		= 0x01000000;//0x01000000 motor current limit reached
	const long CHANNEL_ENABLED				= 0x80000000;//0x80000000 channel is enabled

	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];

	memset(sendBuf,0,sizeof(sendBuf));

	sendBuf[0] = 0x29;//message id
	sendBuf[1] = 0x4;//message id
	sendBuf[2] = 0x0;//channel identifier
	sendBuf[3] = 0x0;//channel identifier
	sendBuf[4] = 0xA1;//destination address
	sendBuf[5] = 0x1;//source address (host)

	unsigned char readBuf [MAX_SERIAL_BUFFER_SIZE];
	unsigned char devAddr;
	memset (readBuf,0,sizeof(readBuf));

	int SettledCountX = 0;
	int SettledCountY = 0;

#if VERBOSE
	{
		wchar_t OutputMsg [80];
		swprintf_s (OutputMsg, 80, L"Entering %hs\n", __FUNCTION__);
		OutputDebugString (OutputMsg);
	}
#endif

	while ((SettledCountX < 1) || (SettledCountY < 1)) {

		//check status for x and y axis

		for (long i = 0; i < 2; i++) {

			switch(i) {

				case 0:	
					sendBuf[4]	= 0xA1;
					devAddr		= 0x21; 
					StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage X Status");
					LogMessage(message,VERBOSE_EVENT);
					break;

				case 1:	
					sendBuf[4]	= 0xA2;
					devAddr		= 0x22;
					StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Y Status");
					LogMessage(message,VERBOSE_EVENT); 
					break;

			}

			_serialPort.SendData ((const unsigned char*) sendBuf, 6);

			Sleep (WAIT_TO_READ_DELAY);

			if (_serialPort.ReadData (readBuf, MAX_SERIAL_BUFFER_SIZE)) {

#if VERBOSE
	{
		wchar_t OutputMsg [1024];
		swprintf_s (
			OutputMsg, 
			1024, 
			L"%hs: readBuf(%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n", 
			__FUNCTION__,
			readBuf [ 0], readBuf [ 1], readBuf [ 2], readBuf [ 3], readBuf [ 4], readBuf [ 5], readBuf [ 6], readBuf [ 7], 
			readBuf [ 8], readBuf [ 9], readBuf [10], readBuf [11], readBuf [12], readBuf [13], readBuf [14], readBuf [15]
			);
		OutputDebugString (OutputMsg);
	}
#endif

				if ((0x2A == readBuf[0]) && (0x04 == readBuf[1])) {

					//build the 4 byte status mask

					long statusVal;

					unsigned char * pVal = (unsigned char*) &statusVal;

					pVal[0] = readBuf[8];
					pVal[1] = readBuf[9];
					pVal[2] = readBuf[10];
					pVal[3] = readBuf[11];

					if ((statusVal & SETTLED) != SETTLED) {
						if (0 == i) {
							SettledCountX = 0;
						} else {
							SettledCountY = 0;
						}
					} else {
						if (0 == i) {
							SettledCountX++;
						} else {
							SettledCountY++;
						}
					}

#if VERBOSE
	{
		wchar_t OutputMsg [256];
		swprintf_s (OutputMsg, 256, L"%hs: statusVal(0x%08x) SettledCountX(%d), SettledCountY(%d)\n", __FUNCTION__, statusVal, SettledCountX, SettledCountY);
		OutputDebugString (OutputMsg);
	}
#endif

				} else {

					StringCbPrintfW(message,MSG_SIZE,L"ThorMLSStage Status Response unknown (%d) (%d) (%d)",readBuf[0],readBuf[1],readBuf[5]);
					LogMessage(message,ERROR_EVENT);

				}

			}

			Sleep(1);

		}

	}

	return ret;

}

/// <summary>
/// Converts the string to w string.
/// </summary>
/// <param name="s">The s.</param>
/// <returns>wstring.</returns>
wstring ConvertStringToWString(string s)
{
	size_t origsize = strlen(s.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t  convertedChars = 0;
	wchar_t wcstring[newsize];
	mbstowcs_s(&convertedChars, wcstring, origsize, s.c_str(), _TRUNCATE);

	wstring ws(wcstring);

	return ws;
}
/// <summary>
/// Comms the alive.
/// </summary>
/// <returns>long.</returns>
long ThorMLSStage::CommAlive()
{
	long ret = TRUE;

	////reduce the frequency of com alive events
	//const long MESSAGE_MAX_WITOUT_COMM_ALIVE_EVENT = 30;

	//if(_comAliveCounter > MESSAGE_MAX_WITOUT_COMM_ALIVE_EVENT)
	//{
	//	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];

	//	memset(sendBuf,0,sizeof(sendBuf));

	//	sendBuf[0] = 0x92;//message id
	//	sendBuf[1] = 0x4;//message id
	//	sendBuf[2] = 0x0;//channel id
	//	sendBuf[3] = 0x0;//channel id
	//	sendBuf[4] = 0xA1;//destination address
	//	sendBuf[5] = 0x1;//source address (host)

	//	_serialPort.SendData((const unsigned char*)sendBuf, 6);

	//	sendBuf[4] = 0xA2;//destination address
	//	_serialPort.SendData((const unsigned char*)sendBuf, 6);

	//	Sleep(WAIT_TO_READ_DELAY);
	//	_comAliveCounter = 0;
	//}
	//else
	//{
	//	_comAliveCounter++;
	//}
	return ret;

}

/// <summary>
/// Reqs the hardware information.
/// </summary>
/// <returns>long.</returns>
long ThorMLSStage::ReqHardwareInfo()
{

	long ret = FALSE;

	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];
	unsigned char readBuf[MAX_SERIAL_BUFFER_SIZE];

	memset(sendBuf,0,sizeof(sendBuf));

	sendBuf[0] = 0x05;//message id
	sendBuf[1] = 0x0;//message id
	sendBuf[2] = 0x0;//channel id
	sendBuf[3] = 0x0;//channel id
	sendBuf[4] = 0xA1;//destination address
	sendBuf[5] = 0x1;//source address (host)

	_serialPort.SendData((const unsigned char*)sendBuf, 6);

	Sleep(WAIT_TO_READ_DELAY);

	memset(readBuf,0,sizeof(readBuf));
	if(_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE))
	{
		//confirm the response id is correct

		if((readBuf[0] == 0x06)&&(readBuf[1] == 0x0)&&(readBuf[2] == 0x54)&&(readBuf[3] == 0x0))
		{
			char model[8];
			memcpy(model, readBuf+10, 8);
			ret = TRUE;
		}
	}

	return ret;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorMLSStage::ReadPosition(DeviceType deviceType,double &pos)
{
	Lock lock(_critSect);

	long ret = TRUE;


	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];
	unsigned char readBuf[MAX_SERIAL_BUFFER_SIZE];

	if(deviceType & STAGE_X)
	{
		//unusual bug where the pos value comes in as zero instead of
		//the current value. Set the pos value to the current value just in case
		pos = _xPos_C;

		memset(sendBuf,0,sizeof(sendBuf));

		sendBuf[0] = 0x90;//message id
		sendBuf[1] = 0x4;//message id
		sendBuf[2] = 0x0;//channel id
		sendBuf[3] = 0x0;//channel id
		sendBuf[4] = 0xA1;//destination address
		sendBuf[5] = 0x1;//source address (host)

		_serialPort.SendData((const unsigned char*)sendBuf, 6);

		Sleep(WAIT_TO_READ_DELAY);

		memset(readBuf,0,sizeof(readBuf));
		if(_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE) >= 12)
		{
			//confirm the response id is correct
			if((readBuf[0] == 0x91)&&(readBuf[1] == 0x04)&&(readBuf[5] == 0x21))
			{
				long lVal;

				unsigned char * pVal = (unsigned char*)&lVal;

				pVal[0] = readBuf[8];
				pVal[1] = readBuf[9] ;
				pVal[2] = readBuf[10];
				pVal[3] = readBuf[11];
				
				pos = static_cast<double>(lVal)/_xEncoderToMM*_motorsParamsInvert[0];

				//Only update param and param_c when not in the middle of preparing and sending new params to the device
				if (FALSE == _blockUpdateParam)
				{
					//Previously when the user would repeate a send position
					//and move the joystick between sends the device would not move.
					//This was due to the storing of the last send position and comparing it
					//to the current position as a test for. whether to not send the position
					//on to the device.
					//
					//To ensure joystick position values are considered in the comparision
					//overwrite the last sent position any time the position is queried
					_xPos_C = _xPos = pos;
				}
			}
		}
	}
	else if(deviceType & STAGE_Y)
	{	
		//unusual bug where the pos value comes in as zero instead of
		//the current value. Set the pos value to the current value just in case
		pos = _yPos_C;

		memset(sendBuf,0,sizeof(sendBuf));
		sendBuf[0] = 0x90;//message id
		sendBuf[1] = 0x4;//message id
		sendBuf[2] = 0x0;//channel id
		sendBuf[3] = 0x0;//channel id
		sendBuf[4] = 0xA2;//destination address
		sendBuf[5] = 0x1;//source address (host)

		_serialPort.SendData((const unsigned char*)sendBuf, 6);

		Sleep(WAIT_TO_READ_DELAY);

		memset(readBuf,0,sizeof(readBuf));
		if(_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE)>=12)
		{
			//confirm the response id is correct
			if((readBuf[0] == 0x91)&&(readBuf[1] == 0x04)&&(readBuf[5] == 0x22))
			{
				long lVal;

				unsigned char * pVal = (unsigned char*)&lVal;

				pVal[0] = readBuf[8];
				pVal[1] = readBuf[9] ;
				pVal[2] = readBuf[10];
				pVal[3] = readBuf[11];

				pos = static_cast<double>(lVal)/_yEncoderToMM*_motorsParamsInvert[1];
				
				//Only update param and param_c when not in the middle of preparing and sending new params to the device
				if (FALSE == _blockUpdateParam)
				{
					//Previously when the user would repeate a send position
					//and move the joystick between sends the device would not move.
					//This was due to the storing of the last send position and comparing it
					//to the current position as a test for. whether to not send the position
					//on to the device.
					//
					//To ensure joystick position values are considered in the comparision
					//overwrite the last sent position any time the position is queried
					_yPos_C = _yPos = pos;
				}
			}
		}	
	}

	//Sleep(30);
	//CommAlive();

	return TRUE;
}

/// <summary>
/// Moves the stage x.
/// </summary>
/// <param name="x">The x.</param>
/// <returns>long.</returns>
long ThorMLSStage::MoveStageX(long x)
{	
	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];

	memset(sendBuf,0,sizeof(sendBuf));

	sendBuf[0] = 0x53;//message id
	sendBuf[1] = 0x4;//message id
	sendBuf[2] = 0x6;//6 byte data packet
	sendBuf[3] = 0x0;//6 byte data packet
	sendBuf[4] = 0xA1;//destination address
	sendBuf[5] = 0x1;//source address (host)
	sendBuf[6] = 0x1;//channel identifier
	sendBuf[7] = 0x0;//channel identifier

	long lVal = x;

	unsigned char * pVal = (unsigned char*)&lVal;

	sendBuf[8] = pVal[0];
	sendBuf[9] = pVal[1];
	sendBuf[10] = pVal[2];
	sendBuf[11] = pVal[3];

	_serialPort.SendData((const unsigned char*)sendBuf, 12);

	Sleep(WAIT_TO_READ_DELAY);

	char readBuf[MAX_SERIAL_BUFFER_SIZE];
	memset(readBuf,0,sizeof(readBuf));
	_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE);
	
	return TRUE;
}

/// <summary>
/// Moves the stage y.
/// </summary>
/// <param name="y">The y.</param>
/// <returns>long.</returns>
long ThorMLSStage::MoveStageY(long y)
{	
	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];

	memset(sendBuf,0,sizeof(sendBuf));

	sendBuf[0] = 0x53;//message id
	sendBuf[1] = 0x4;//message id
	sendBuf[2] = 0x6;//6 byte data packet
	sendBuf[3] = 0x0;//6 byte data packet
	sendBuf[4] = 0xA2;//destination address
	sendBuf[5] = 0x1;//source address (host)
	sendBuf[6] = 0x2;//channel identifier
	sendBuf[7] = 0x0;//channel identifier

	long lVal = y;

	unsigned char * pVal = (unsigned char*)&lVal;

	sendBuf[8] = pVal[0];
	sendBuf[9] = pVal[1];
	sendBuf[10] = pVal[2];
	sendBuf[11] = pVal[3];

	_serialPort.SendData((const unsigned char*)sendBuf, 12);

	Sleep(WAIT_TO_READ_DELAY);

	char readBuf[MAX_SERIAL_BUFFER_SIZE];
	memset(readBuf,0,sizeof(readBuf));
	_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE);

	return TRUE;
}

/// <summary>
/// Sets the velocity x.
/// </summary>
/// <param name="vx">The vx.</param>
/// <param name="ax">The ax.</param>
/// <returns>long.</returns>
long ThorMLSStage::SetVelocityX(long vx, long ax)
{	
	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];

	memset(sendBuf,0,sizeof(sendBuf));

	const long SET_VELOCITY_COMMAND_LENGTH = 20;

	sendBuf[0] = 0x13;//message id
	sendBuf[1] = 0x4;//message id
	sendBuf[2] = 0xE;//14 byte data packet
	sendBuf[3] = 0x0;
	sendBuf[4] = 0xA1;//destination address
	sendBuf[5] = 0x1;//source address (host)
	sendBuf[6] = 0x1;//channel identifier
	sendBuf[7] = 0x0;//channel identifier

	// Min Velocity (start velocity), this 4 byte value is always zero
	long minVal = 0;
	unsigned char * pVal = (unsigned char*)&minVal;
	sendBuf[8] = pVal[0];
	sendBuf[9] = pVal[1];
	sendBuf[10] = pVal[2];
	sendBuf[11] = pVal[3];

	// Acceleration, in encoder counts/sec/sec
	long accVal = ax;
	pVal = (unsigned char*)&accVal;
	sendBuf[12] = pVal[0];
	sendBuf[13] = pVal[1];
	sendBuf[14] = pVal[2];
	sendBuf[15] = pVal[3];

	// Maximum Velocity (final velocity), encoder counts/sec
	long finalVal = vx;
	pVal = (unsigned char*)&finalVal;

	sendBuf[16] = pVal[0];
	sendBuf[17] = pVal[1];
	sendBuf[18] = pVal[2];
	sendBuf[19] = pVal[3];

	_serialPort.SendData((const unsigned char*)sendBuf, SET_VELOCITY_COMMAND_LENGTH);

	Sleep(WAIT_TO_READ_DELAY);

	char readBuf[MAX_SERIAL_BUFFER_SIZE];
	memset(readBuf,0,sizeof(readBuf));
	_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE);

	return TRUE;
}

/// <summary>
/// Sets the velocity y.
/// </summary>
/// <param name="vy">The vy.</param>
/// <param name="ay">The ay.</param>
/// <returns>long.</returns>
long ThorMLSStage::SetVelocityY(long vy, long ay)
{	
	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];

	const long SET_VELOCITY_COMMAND_LENGTH = 20;

	memset(sendBuf,0,sizeof(sendBuf));

	sendBuf[0] = 0x13;//message id
	sendBuf[1] = 0x4;//message id
	sendBuf[2] = 0xE;//14 byte data packet
	sendBuf[3] = 0x0;
	sendBuf[4] = 0xA2;//destination address
	sendBuf[5] = 0x1;//source address (host)
	sendBuf[6] = 0x2;//channel identifier
	sendBuf[7] = 0x0;//channel identifier

	// Min Velocity (start velocity), this 4 byte value is always zero
	long minVal = 0;
	unsigned char * pVal = (unsigned char*)&minVal;
	sendBuf[8] = pVal[0];
	sendBuf[9] = pVal[1];
	sendBuf[10] = pVal[2];
	sendBuf[11] = pVal[3];

	// Acceleration, in encoder counts/sec/sec
	long accVal = ay;
	pVal = (unsigned char*)&accVal;
	sendBuf[12] = pVal[0];
	sendBuf[13] = pVal[1];
	sendBuf[14] = pVal[2];
	sendBuf[15] = pVal[3];

	// Maximum Velocity (final velocity), encoder counts/sec
	long finalVal = vy;
	pVal = (unsigned char*)&finalVal;

	sendBuf[16] = pVal[0];
	sendBuf[17] = pVal[1];
	sendBuf[18] = pVal[2];
	sendBuf[19] = pVal[3];

	_serialPort.SendData((const unsigned char*)sendBuf, SET_VELOCITY_COMMAND_LENGTH);

	Sleep(WAIT_TO_READ_DELAY);

	char readBuf[MAX_SERIAL_BUFFER_SIZE];
	memset(readBuf,0,sizeof(readBuf));
	_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE);

	return TRUE;
}

/// <summary>
/// Reqs the velocity x.
/// </summary>
/// <param name="vx">The vx.</param>
/// <param name="ax">The ax.</param>
/// <returns>long.</returns>
long ThorMLSStage::ReqVelocityX(double &vx, double &ax)
{
	long ret = TRUE;

	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];
	unsigned char readBuf[MAX_SERIAL_BUFFER_SIZE];

	const long REQ_VELOCITY_COMMAND_LENGTH = 6;
	
	memset(sendBuf,0,sizeof(sendBuf));

	sendBuf[0] = 0x14;//message id
	sendBuf[1] = 0x4;//message id
	sendBuf[2] = 0x00;//channel id
	sendBuf[3] = 0x0;
	sendBuf[4] = 0xA1;//destination address
	sendBuf[5] = 0x1;//source address (host)

	_serialPort.SendData((const unsigned char*)sendBuf, REQ_VELOCITY_COMMAND_LENGTH);

	Sleep(WAIT_TO_READ_DELAY);

	memset(readBuf,0,sizeof(readBuf));
	if(_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE))
	{
		//confirm the response id is correct
		if((readBuf[0] == 0x15)&&(readBuf[1] == 0x04)&&(readBuf[5] == 0x21))
		{
			long lAccel;
			unsigned char * pAcc = (unsigned char*)&lAccel;

			pAcc[0] = readBuf[12];
			pAcc[1] = readBuf[13] ;
			pAcc[2] = readBuf[14];
			pAcc[3] = readBuf[15];

			_xAcc_C = _xAcc = static_cast<double>(lAccel)/ENCODER_TO_MM_PER_SEC_SQR_CONVERSION;

			long lVeloc;
			unsigned char * pVel = (unsigned char*)&lVeloc;

			pVel[0] = readBuf[16];
			pVel[1] = readBuf[17];
			pVel[2] = readBuf[18];
			pVel[3] = readBuf[19];

			_xVel_C = _xVel = static_cast<double>(lVeloc)/ENCODER_TO_MM_PER_SEC_CONVERSION;
		}
	}
	ax = _xAcc_C;
	vx = _xVel_C;

	//Sleep(30);
	//CommAlive();

	return TRUE;
}

/// <summary>
/// Reqs the velocity y.
/// </summary>
/// <param name="vy">The vy.</param>
/// <param name="ay">The ay.</param>
/// <returns>long.</returns>
long ThorMLSStage::ReqVelocityY(double &vy, double &ay)
{
	long ret = TRUE;

	unsigned char sendBuf[MAX_SERIAL_BUFFER_SIZE];
	unsigned char readBuf[MAX_SERIAL_BUFFER_SIZE];

	const long REQ_VELOCITY_COMMAND_LENGTH = 6;
	memset(sendBuf,0,sizeof(sendBuf));

	sendBuf[0] = 0x14;//message id
	sendBuf[1] = 0x4;//message id
	sendBuf[2] = 0x0;//channel id
	sendBuf[3] = 0x0;//channel id
	sendBuf[4] = 0xA2;//destination address
	sendBuf[5] = 0x1;//source address (host)

	_serialPort.SendData((const unsigned char*)sendBuf, REQ_VELOCITY_COMMAND_LENGTH);

	Sleep(WAIT_TO_READ_DELAY);

	memset(readBuf,0,sizeof(readBuf));
	if(_serialPort.ReadData(readBuf,MAX_SERIAL_BUFFER_SIZE))
	{
		//confirm the response id is correct
		if((readBuf[0] == 0x15)&&(readBuf[1] == 0x04)&&(readBuf[5] == 0x22))
		{
			long lAccel;
			unsigned char * pAcc = (unsigned char*)&lAccel;

			pAcc[0] = readBuf[12];
			pAcc[1] = readBuf[13] ;
			pAcc[2] = readBuf[14];
			pAcc[3] = readBuf[15];

			_yAcc_C = _yAcc = static_cast<double>(lAccel)/ENCODER_TO_MM_PER_SEC_SQR_CONVERSION;

			long lVeloc;
			unsigned char * pVel = (unsigned char*)&lVeloc;

			pVel[0] = readBuf[16];
			pVel[1] = readBuf[17];
			pVel[2] = readBuf[18];
			pVel[3] = readBuf[19];

			_yVel_C = _yVel = static_cast<double>(lVeloc)/ENCODER_TO_MM_PER_SEC_CONVERSION;
		}
	}
	ay = _yAcc_C;
	vy = _yVel_C;

	//Sleep(30);
	//CommAlive();

	return TRUE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorMLSStage::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="logMsg">The log MSG.</param>
/// <param name="eventLevel">The event level.</param>
void ThorMLSStage::LogMessage(wchar_t *logMsg,long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}
