//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorPinholeStepper.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "Serial.h"
#include "ThorPinholeStepper.h"
#include "PinholeStepperSetupXML.h"

#include "Strsafe.h"

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

/// <summary>
/// The message
/// </summary>
wchar_t message[256];

#define CLK_FRQ 16000000.0
#define FULL_STEPS_PER_ROTATION 200.0
#define KEY_ACCEL_INTERVAL 800
#define NUMSTEPS_CALIBRATION 3200
#define DURATION_CALIBRATION 1.5
#define PANEL_UPDATE_INTERVAL 250

/// <summary>
/// Prevents a default instance of the <see cref="ThorPinholeStepper"/> class from being created.
/// </summary>
ThorPinholeStepper::ThorPinholeStepper():
PINHOLE_LOCATIONS(16),
PINHOLE_SEPARATION_DEFAULT(500),
NON_PINHOLE_FREQ_DEFAULT(4)
{
	_pPos = 0;
	_pPos_C = -1;
	_pPos_B = FALSE;
	_deviceDetected = FALSE;
	_pMin=0;
	_pMax=15;
	 _pPosAlign = 0;
	 _pPosAlign_C = 0;
	 _pPosAlign_B = FALSE;
	
	 _pMode = POS_LOOKUP;
	 _pMode_C = POS_LOOKUP;
	 _pMode_B = FALSE;

	_pPinholeSeparation = PINHOLE_SEPARATION_DEFAULT;
	_pPinholeSeparation_C = PINHOLE_SEPARATION_DEFAULT;
	_pPinholeSeparation_B = FALSE;
	
	_pNonPinholeFreq = NON_PINHOLE_FREQ_DEFAULT;
	_pNonPinholeFreq_C = NON_PINHOLE_FREQ_DEFAULT;
	_pNonPinholeFreq_B = FALSE;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorPinholeStepper"/> class.
/// </summary>
ThorPinholeStepper::~ThorPinholeStepper()
{
	_instanceFlag = false;
	_errMsg[0] = 0;
}

bool ThorPinholeStepper:: _instanceFlag = false;

auto_ptr<ThorPinholeStepper> ThorPinholeStepper::_single(new ThorPinholeStepper());


/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorPinholeStepper *.</returns>
ThorPinholeStepper *ThorPinholeStepper::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorPinholeStepper());
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
long ThorPinholeStepper::FindDevices(long &deviceCount)
{
	long	ret = TRUE;

	//Get filter parameters from hardware setup.xml
	auto_ptr<PinholeStepperXML> pSetup(new PinholeStepperXML());

	long portID=0;
	long baudRate=0;
	long address=0;
	pSetup->GetConnection(portID,baudRate,address);

	_pinholeLocations.clear();
	for(long i=0; i<PINHOLE_LOCATIONS; i++)
	{
		long val=0;
		if(TRUE == pSetup->GetPinholeLocation(i,val))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper GetPinholeLocation (%d) returned %d",i,val);
			LogMessage(message);
			_pinholeLocations.push_back(val);
		}
	}
	_pMin = 0;
	_pMax = 15;


	if(FALSE == _serialPort.Open(portID, baudRate))
	{
		StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper FindDevices could not open serial port");
		LogMessage(message);
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorPinholeStepper FindDevices could not open serial port or configuration file is not avaible.");

		// *TODO* perform an automatic scan of the available serial ports for the pinhole stepper
		deviceCount = 0;
		ret = FALSE;
	}
	else
	{
		char sendBuf[100];
		memset(sendBuf,0,sizeof(sendBuf));

		sprintf_s((char*)sendBuf,100,"/1&R\r");

		_serialPort.SendData((const unsigned char*)sendBuf, static_cast<int>(strlen((const char*)sendBuf)));

		Sleep(100);
		memset(sendBuf,0,sizeof(sendBuf));

		_serialPort.ReadData(sendBuf, 100);

		char identifier[] = "EZStepper AllMotion";

		char *pch = strstr(sendBuf, identifier);

		if(pch == NULL)
			ret = FALSE;
		else
		{
			deviceCount = 1;
			_deviceDetected = TRUE;
			_address = static_cast<unsigned char>(address);
			ret = TRUE; 
		}

		_serialPort.Close();

	}

	return ret;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorPinholeStepper::SelectDevice(const long device)
{
	long	ret = FALSE;

	if(FALSE == _deviceDetected)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"The device has not been detected");
		return FALSE;
	}

	switch(device)
	{
	case 0:	
		{
	
		//Get filter parameters from hardware setup.xml
		auto_ptr<PinholeStepperXML> pSetup(new PinholeStepperXML());

		long portID=0;
		long baudRate=0;
		long address=0;
		pSetup->GetConnection(portID,baudRate,address);

		if(FALSE == _serialPort.Open(portID, baudRate))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper SelectDevice could not open serial port");
			LogMessage(message);
			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorPinholeStepper SelectDevice could not open serial port or configuration file is not avaible.");

			return FALSE;
		}

		ret = TRUE; 
		}
		break;
	default:
		{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"The device index %d is invalid",device);
		}
	}

	return ret;
}
/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorPinholeStepper::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorPinholeStepper::TeardownDevice()
{	
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
long ThorPinholeStepper::GetParamInfo
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
	case PARAM_PINHOLE_POS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _pMin;
			paramMax = _pMax;
			paramDefault = _pMin;
		}
		break;		

	case PARAM_PINHOLE_POS_CURRENT:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _pMin;
			paramMax = _pMax;
			paramDefault = _pMin;
		}
		break;		

	case PARAM_PINHOLE_ALIGNMENT_POS:
		{
			const long ALIGNMENT_MIN = 0;
			const long ALIGNMENT_MAX = 10000;
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = ALIGNMENT_MIN;
			paramMax = ALIGNMENT_MAX;
			paramDefault = ALIGNMENT_MIN;
		}
		break;
		
	case PARAM_PINHOLE_ALIGNMENT_POS_CURRENT:
		{
			const long ALIGNMENT_MIN = 0;
			const long ALIGNMENT_MAX = 10000;
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = ALIGNMENT_MIN;
			paramMax = ALIGNMENT_MAX;
			paramDefault = ALIGNMENT_MIN;
		}
		break;

	case PARAM_PINHOLE_ALIGNMENT_MODE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = POS_LOOKUP;
			paramMax = POS_MOVE_ONLY;
			paramDefault = POS_LOOKUP;
		}
		break;

	case PARAM_PINHOLE_SEPARATION:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 10000;
			paramDefault = PINHOLE_SEPARATION_DEFAULT;
		}
		break;
		
	case PARAM_PINHOLE_NON_PINHOLE_FREQUENCY:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = NON_PINHOLE_FREQ_DEFAULT;
		}
		break;

	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = PINHOLE_WHEEL;
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

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorPinholeStepper::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;
	long foundParam = TRUE;

	switch(paramID)
	{
	case PARAM_PINHOLE_POS:
		{
			if((static_cast<long>(param) >= _pMin) && (static_cast<long>(param) <= _pMax) && (static_cast<long>(param)<static_cast<long>(_pinholeLocations.size())))
			{
				_pPos = static_cast<long>(param);
				
			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_PINHOLE_ALIGNMENT_POS:
		{
			if((static_cast<long>(param) >= 0) && (static_cast<long>(param) <= 10000))
			{
				_pPosAlign = static_cast<long>(param);				
			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_PINHOLE_ALIGNMENT_MODE:
		{
			if((static_cast<long>(param) >= POS_LOOKUP) && (static_cast<long>(param) <= POS_MOVE_ONLY))
			{
				_pMode = static_cast<long>(param);				
			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_PINHOLE_SEPARATION:
		{
			if((static_cast<long>(param) >= 0) && (static_cast<long>(param) <= 10000))
			{
				_pPinholeSeparation = static_cast<long>(param);				
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
		
	case PARAM_PINHOLE_NON_PINHOLE_FREQUENCY:
		{
			if((static_cast<long>(param) >= 0) && (static_cast<long>(param) <= 100))
			{
				_pNonPinholeFreq = static_cast<long>(param);				
			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	default:
		{
		ret = FALSE;
		foundParam = FALSE;
		}
	}

	if(FALSE == ret)
	{
		if(TRUE == foundParam)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) out of range",paramID);
		}
		else
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) not implemented",paramID);
		}
	}

	return ret;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorPinholeStepper::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_PINHOLE_POS:	
		{ 
			param = static_cast<double>(_pPos); 
		}
		break;
	case PARAM_PINHOLE_POS_CURRENT:	
		{ 
			param = static_cast<double>(_pPos); 
		}
		break;
		
	case PARAM_PINHOLE_ALIGNMENT_POS:	
		{ 
			param = static_cast<double>(_pPosAlign); 
		}
		break;
		
	case PARAM_PINHOLE_ALIGNMENT_MODE:	
		{ 
			param = static_cast<double>(_pMode); 
		}
		break;
	case PARAM_PINHOLE_ALIGNMENT_POS_CURRENT:
		{	
			if(_pinholeLocations.size() > 0)
			{
				param = static_cast<double>(_pinholeLocations[0]);
			}
		}
		break;
	case PARAM_PINHOLE_SEPARATION:
		{
				param = static_cast<double>(_pPinholeSeparation);
		}
		break;		
	case PARAM_PINHOLE_NON_PINHOLE_FREQUENCY:
		{
				param = static_cast<double>(_pNonPinholeFreq);
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			param = PINHOLE_WHEEL;
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
long ThorPinholeStepper::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPinholeStepper::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPinholeStepper::SetParamString(const long paramID, wchar_t* str)
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
long ThorPinholeStepper::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorPinholeStepper::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorPinholeStepper::SetupPosition()
{
	if(_pPos != _pPos_C)
	{
		_pPos_B = TRUE;
	}
	
	if(_pPosAlign != _pPosAlign_C)
	{
		_pPosAlign_B = TRUE;
	}

	if(_pMode != _pMode_C)
	{
		_pMode_B = TRUE;
	}
	
	if(_pPinholeSeparation != _pPinholeSeparation_C)
	{
		_pPinholeSeparation_B = TRUE;
	}
	
	if(_pNonPinholeFreq != _pNonPinholeFreq_C)
	{
		_pNonPinholeFreq_B = TRUE;
	}

	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorPinholeStepper::StartPosition()
{
	long	ret = FALSE;

	if(_pMode_B)
	{
		_pMode_C = _pMode;
		_pMode_B = FALSE;
	}

	if(_pPinholeSeparation_B)
	{
		_pPinholeSeparation_C = _pPinholeSeparation;
		_pPinholeSeparation_B = FALSE;
	}
	
	if(_pNonPinholeFreq_B)
	{
		_pNonPinholeFreq_C = _pNonPinholeFreq;
		_pNonPinholeFreq_B = FALSE;
	}

	if(POS_LOOKUP == _pMode)
	{
		if(_pPos_B)
		{
			long pos = static_cast<long>(_pPos);

			StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper StartPosition (%d) locations size %d",pos,_pinholeLocations.size());
			LogMessage(message);

			if(pos < static_cast<long>(_pinholeLocations.size()))
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper MoveStepper (%d)",_pinholeLocations[pos]);
				LogMessage(message);

				//Move to the requested index
				MoveStepper(_pinholeLocations[pos]);

				DWORD startTime = GetTickCount();

				//wait for the device to reach the requested position
				do
				{
					char sendBuf[100];

					sprintf_s((char*)sendBuf,100,"/1?8\r");

					_serialPort.SendData((const unsigned char*)sendBuf, static_cast<int>(strlen((const char*)sendBuf)));

					Sleep(100);
					memset(sendBuf,0,sizeof(sendBuf));
					_serialPort.ReadData(sendBuf, 100);

					std::string str3 = std::string(sendBuf);

					std::regex rx("(.*)(`)([0-9]{1,})(.*)");

					try
					{
						std::cmatch res;

						if( std::regex_search(str3.c_str(), res, rx  ) )
						{
							if( res.size() == 5 )
							{
								stringstream ss(res[3]);

								long currentPos;
								ss>>currentPos;			   
								StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper Motor at position. Current pos (%d)",currentPos);
								LogMessage(message);
								_pPos_C = _pPos;
								_pPos_B = FALSE;
								return TRUE;
							}
						}
						else
						{
							std::regex rxDith("(.*)(@)([0-9]{1,})(.*)");

							std::cmatch resDith;

							if( std::regex_search(str3.c_str(), resDith, rxDith  ) )
							{
								if( resDith.size() == 5 )
								{
									stringstream ss(resDith[3]);

									long currentPos;
									ss>>currentPos;			   
									StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper Motor still moving. Current pos (%d)",currentPos);
									LogMessage(message);
								}
							}
						}
					}
					catch(...)
					{
					}
				}
				while((GetTickCount() - startTime) < 10000);

				StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper Dither correction executing");
				LogMessage(message);

				const long DITHERING_OFFSET = 30;
				//The device timed out getting to the position
				//Nudge the device and move again
				MoveStepper(_pinholeLocations[pos] + 30);

				Sleep(1000);

				MoveStepper(_pinholeLocations[pos]);

				_pPos_C = _pPos;
				_pPos_B = FALSE;
			}
			ret = TRUE;
		}
	}
	else if((POS_AND_ALIGNMENT == _pMode)||(POS_MOVE_ONLY == _pMode))
	{
		MoveStepper(_pPosAlign);

		//if mode 1 using the input position. set the alignment locations for all of the pinholes
		//if mode 2 just move the device do not update the alignment
		if(POS_AND_ALIGNMENT == _pMode)
		{
			long offset = 0;
			long count = 0;

			for(long i=0; i<PINHOLE_LOCATIONS; i++,offset+=_pPinholeSeparation_C,count++)
			{
				//make sure the non pinhole locations are skipped
				if(_pNonPinholeFreq == count)
				{
					offset += _pPinholeSeparation_C;
					count = 0;
				}

				_pinholeLocations[i] = (_pPosAlign + offset) % 10000;
			}

			//Get filter parameters from hardware setup.xml
			auto_ptr<PinholeStepperXML> pSetup(new PinholeStepperXML());

			for(long i=0; i<PINHOLE_LOCATIONS; i++)
			{
				long val=0;
				if(TRUE == pSetup->SetPinholeLocation(i,_pinholeLocations[i]))
				{

				}
			}

			pSetup->SaveConfigFile();
		}

		DWORD startTime = GetTickCount();
		
		//wait for the device to reach the requested position
		do
		{
			char sendBuf[100];

			sprintf_s((char*)sendBuf,100,"/1?8\r");

			_serialPort.SendData((const unsigned char*)sendBuf, static_cast<int>(strlen((const char*)sendBuf)));

			Sleep(100);
			memset(sendBuf,0,sizeof(sendBuf));
			_serialPort.ReadData(sendBuf, 100);

			std::string str3 = std::string(sendBuf);

			std::regex rx("(.*)(`)([0-9]{1,})(.*)");

			try
			{
				std::cmatch res;

				if( std::regex_search(str3.c_str(), res, rx  ) )
				{
					if( res.size() == 5 )
					{
						stringstream ss(res[3]);

						long currentPos;
						ss>>currentPos;			   
						StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper Motor at position. Current pos (%d)",currentPos);
						LogMessage(message);
						_pPosAlign_C = _pPosAlign;
						_pPosAlign_B = FALSE;
						return TRUE;
					}
				}
				else
				{
					std::regex rxDith("(.*)(@)([0-9]{1,})(.*)");

					std::cmatch resDith;

					if( std::regex_search(str3.c_str(), resDith, rxDith  ) )
					{
						if( resDith.size() == 5 )
						{
							stringstream ss(resDith[3]);

							long currentPos;
							ss>>currentPos;			   
							StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper Motor still moving. Current pos (%d)",currentPos);
							LogMessage(message);
						}
					}
				}
			}
			catch(...)
			{
			}
		}
		while((GetTickCount() - startTime) < 10000);

		StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper Dither correction executing");
		LogMessage(message);

		const long DITHERING_OFFSET = 30;

		//The device timed out getting to the position
		//Nudge the device and move again
		MoveStepper(_pPosAlign + 30);

		Sleep(1000);
		MoveStepper(_pPosAlign);

		_pPosAlign_C = _pPosAlign;
		_pPosAlign_B = FALSE;

		ret = TRUE;
	}

	return ret;
}

/// <summary>
/// Status of the positioning.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorPinholeStepper::StatusPosition(long &status)
{
	long	ret = TRUE;

	status = IDevice::STATUS_READY;

	char sendBuf[100];

	sprintf_s((char*)sendBuf,100,"/1?8\r");

	_serialPort.SendData((const unsigned char*)sendBuf, static_cast<int>(strlen((const char*)sendBuf)));

	Sleep(100);
	memset(sendBuf,0,sizeof(sendBuf));
	_serialPort.ReadData(sendBuf, 100);

	std::string str3 = std::string(sendBuf);
	std::wstring str4(str3.length(), L' '); // Make room for characters

	// Copy string to wstring.
	std::copy(str3.begin(), str3.end(), str4.begin());

	StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper Encoder Read String (%s) length (%d)",str4.c_str(),str3.size());
	LogMessage(message);

	long target = 0;

	if(POS_LOOKUP == _pMode_C)
	{
		if(_pPos_C < static_cast<long>(_pinholeLocations.size()))
		{
			target = _pinholeLocations[_pPos_C];
		}
	}
	else
	{
		target = _pPosAlign_C;
	}

	long currentPos=0;

	try
	{

		std::regex rx("(.*)(`)([0-9]{1,})(.*)");

		std::cmatch res;

		if( std::regex_search(str3.c_str(), res, rx  ) )
		{
			if( res.size() == 5 )
			{
				stringstream ss(res[3]);

				ss>>currentPos;			   
			}
		}
	}
	catch(...)
	{
	}


	sscanf_s(sendBuf,"/0%d%*s",&currentPos);

	StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepper Encoder Read (%d) Target (%d)",currentPos,target);
	LogMessage(message);

	if((currentPos >= (target -1))&&(currentPos <= (target + 1)))
	{
		status = IDevice::STATUS_READY;
	}
	else
	{
		status = IDevice::STATUS_BUSY;
	}

	return ret;
}

long ThorPinholeStepper::ReadPosition(DeviceType deviceType, double &pos)
{
	//**DEPRECATED**//
	long	ret = FALSE;

	if(deviceType & PINHOLE_WHEEL)
	{
		pos = _pPos_C;
		ret = TRUE;
	}

	return ret;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorPinholeStepper::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Moves the stepper.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorPinholeStepper::MoveStepper(long pos)
{
	char sendBuf[100];

	memset(sendBuf,0,sizeof(sendBuf));

	sprintf_s((char*)sendBuf,100,"/1A%dR\r",pos);

	_serialPort.SendData((const unsigned char*)sendBuf, static_cast<int>(strlen((const char*)sendBuf)));

	Sleep(100);

	_serialPort.ReadData(sendBuf, 100);

	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorPinholeStepper::GetLastErrorMsg(wchar_t * msg, long size)
{	
    wcsncpy_s(msg,size,_errMsg,256);
	return TRUE;
}