//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorPinholeStepperSimulator.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "Serial.h"
#include "ThorPinholeStepperSimulator.h"
#include "PinholeStepperSetupXML.h"

#include "Strsafe.h"

wchar_t message[256];

#define CLK_FRQ 16000000.0
#define FULL_STEPS_PER_ROTATION 200.0
#define KEY_ACCEL_INTERVAL 800
#define NUMSTEPS_CALIBRATION 3200
#define DURATION_CALIBRATION 1.5
#define PANEL_UPDATE_INTERVAL 250

ThorPinholeStepperSimulator::ThorPinholeStepperSimulator():
PINHOLE_LOCATIONS(16)
{
	_pPos = 0;
	_pPos_C = -1;
	_pPos_B = FALSE;
	_deviceDetected = FALSE;
	_numDevices = 0;
	_pMin=0;
	_pMax=15;
	 _pPosAlign = 0;
	 _pPosAlign_C = 0;
	 _pPosAlign_B = FALSE;
	
	 _pMode = 0;
	 _pMode_C = 0;
	 _pMode_B = FALSE;
}

ThorPinholeStepperSimulator::~ThorPinholeStepperSimulator()
{
	_instanceFlag = false;
	_errMsg[0] = 0;
}

bool ThorPinholeStepperSimulator:: _instanceFlag = false;

auto_ptr<ThorPinholeStepperSimulator> ThorPinholeStepperSimulator::_single(new ThorPinholeStepperSimulator());


ThorPinholeStepperSimulator *ThorPinholeStepperSimulator::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorPinholeStepperSimulator());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorPinholeStepperSimulator::FindDevices(long &deviceCount)
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
			//StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator GetPinholeLocation (%d) returned %d",i,val);
			//LogMessage(message);
			_pinholeLocations.push_back(val);
		}
	}
	_pMin = 0;
	_pMax = 15;


	//if(FALSE == _serialPort.Open(portID, baudRate))
	//{
	//	StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator FindDevices could not open serial port");
	//	LogMessage(message);
	//	StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorPinholeStepperSimulator FindDevices could not open serial port or configuration file is not avaible.");

	//	// *TODO* perform an automatic scan of the available serial ports for the Z stepper
	//	deviceCount = 0;
	//	ret = FALSE;
	//}
	//else
	//{
		deviceCount = 1;
		_numDevices = deviceCount;
		_deviceDetected = TRUE;
		_address = static_cast<unsigned char>(address);


	return ret;
}

long ThorPinholeStepperSimulator::SelectDevice(const long device)
{
	long	ret = TRUE;

	if(FALSE == _deviceDetected)
	{
		//StringCbPrintfW(_errMsg,MSG_SIZE,L"The device has not been detected");
		return FALSE;
	}

	if((device < 0) || (device >= _numDevices))
	{
		ret = FALSE;
	}

	return ret;
}
void ThorPinholeStepperSimulator::LogMessage(wchar_t *message)
{
//#ifdef LOGGING_ENABLED
//		logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
//#endif
}

long ThorPinholeStepperSimulator::TeardownDevice()
{	
	return TRUE;
}

long ThorPinholeStepperSimulator::GetParamInfo
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
			paramMin = CONNECTION_READY;
			paramMax = CONNECTION_READY;
			paramDefault = CONNECTION_READY;
		}
		break;
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
			const long DISABLED = 0;
			const long ENABLED = 1;
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = DISABLED;
			paramMax = ENABLED;
			paramDefault = DISABLED;
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

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long ThorPinholeStepperSimulator::SetParam(const long paramID, const double param)
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
			if((static_cast<long>(param) >= 0) && (static_cast<long>(param) <= 1))
			{
				_pMode = static_cast<long>(param);				
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

	return ret;
}

long ThorPinholeStepperSimulator::GetParam(const long paramID, double &param)
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
			param = static_cast<double>(_pPos_C); 
		}
		break;
		
	case PARAM_PINHOLE_ALIGNMENT_POS:	
		{ 
			param = static_cast<double>(_pPosAlign); 
		}
		break;
		
	case PARAM_PINHOLE_ALIGNMENT_MODE:	
		{ 
			param = static_cast<double>(_pMode_C); 
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
	case PARAM_DEVICE_TYPE:
		{
			param = PINHOLE_WHEEL;
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = static_cast<double>(CONNECTION_READY);
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
long ThorPinholeStepperSimulator::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPinholeStepperSimulator::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPinholeStepperSimulator::SetParamString(const long paramID, wchar_t* str)
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
long ThorPinholeStepperSimulator::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorPinholeStepperSimulator::PreflightPosition()
{
	return TRUE;
}

long ThorPinholeStepperSimulator::SetupPosition()
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


	return TRUE;
}

long ThorPinholeStepperSimulator::StartPosition()
{
	long	ret = FALSE;

	if(_pMode_B)
	{
		_pMode_C = _pMode;
		_pMode_B = FALSE;
	}

	if(0 == _pMode)
	{
		if(_pPos_B)
		{
			long pos = static_cast<long>(_pPos);

			//StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator StartPosition (%d) locations size %d",pos,_pinholeLocations.size());
			//LogMessage(message);

			if(pos < static_cast<long>(_pinholeLocations.size()))
			{
				/*StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator MoveStepper (%d)",_pinholeLocations[pos]);
				LogMessage(message);*/

				MoveStepper(_pinholeLocations[pos]);

				DWORD startTime = GetTickCount();

				do
				{
					char sendBuf[100];

					//sprintf((char*)sendBuf,"/1?8\r");

					//_serialPort.SendData((const unsigned char*)sendBuf, strlen((const char*)sendBuf));

					Sleep(100);
					//memset(sendBuf,0,sizeof(sendBuf));
					//_serialPort.ReadData(sendBuf, 100);

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
								/*StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator Motor at position. Current pos (%d)",currentPos);
								LogMessage(message);*/
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
	/*								StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator Motor still moving. Current pos (%d)",currentPos);
									LogMessage(message);*/
								}
							}
						}
					}
					catch(...)
					{
					}
				}
				while((GetTickCount() - startTime) < 1000);

				//StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator Dither correction executing");
				//LogMessage(message);

				const long DITHERING_OFFSET = 30;

				MoveStepper(_pinholeLocations[pos] + 30);

				//Sleep(1000);

				MoveStepper(_pinholeLocations[pos]);

				_pPos_C = _pPos;
				_pPos_B = FALSE;
			}
			ret = TRUE;
		}
	}
	else
	{
		MoveStepper(_pPosAlign);

		long offset = 0;
		const long NON_PINHOLE = 4;
		long count = 0;
		const long PINHOLE_SEPERATION_STEPS = 500;
		for(long i=0; i<PINHOLE_LOCATIONS; i++,offset+=PINHOLE_SEPERATION_STEPS,count++)
		{
			//make sure the non pinhole locations are skipped
			if(NON_PINHOLE == count)
			{
				offset += PINHOLE_SEPERATION_STEPS;
				count = 0;
			}

			_pinholeLocations[i] = (_pPosAlign + offset) % 10000;
		}

		DWORD startTime = GetTickCount();

		do
		{
			char sendBuf[100];

			//sprintf((char*)sendBuf,"/1?8\r");

			//_serialPort.SendData((const unsigned char*)sendBuf, strlen((const char*)sendBuf));

			Sleep(100);
			//memset(sendBuf,0,sizeof(sendBuf));
			//_serialPort.ReadData(sendBuf, 100);

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
						//StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator Motor at position. Current pos (%d)",currentPos);
						//LogMessage(message);
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
							//StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator Motor still moving. Current pos (%d)",currentPos);
							//LogMessage(message);
						}
					}
				}
			}
			catch(...)
			{
			}
		}
		while((GetTickCount() - startTime) < 1000);

		//StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator Dither correction executing");
		//LogMessage(message);

		const long DITHERING_OFFSET = 30;

		MoveStepper(_pPosAlign + 30);

		//Sleep(1000);
		MoveStepper(_pPosAlign);

		_pPosAlign_C = _pPosAlign;
		_pPosAlign_B = FALSE;

		ret = TRUE;
	}

	return ret;
}

long ThorPinholeStepperSimulator::StatusPosition(long &status)
{
	long	ret = TRUE;

	status = IDevice::STATUS_READY;

	char sendBuf[100];

	//sprintf((char*)sendBuf,"/1?8\r");

	//_serialPort.SendData((const unsigned char*)sendBuf, strlen((const char*)sendBuf));

	Sleep(100);
	memset(sendBuf,0,sizeof(sendBuf));
	//_serialPort.ReadData(sendBuf, 100);

	std::string str3 = std::string(sendBuf);
	//std::wstring str4(str3.length(), L' '); // Make room for characters

	// Copy string to wstring.
	//std::copy(str3.begin(), str3.end(), str4.begin());

	//StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator Encoder Read String (%s) length (%d)",str4.c_str(),str3.size());
	//LogMessage(message);

	long target = 0;

	if(0 == _pMode_C)
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


	//sscanf(sendBuf,"/0%d%*s",&currentPos);

	//StringCbPrintfW(message,MSG_SIZE,L"ThorPinholeStepperSimulator Encoder Read (%d) Target (%d)",currentPos,target);
	//LogMessage(message);*/

	//if((currentPos >= (target -1))&&(currentPos <= (target + 1)))
	//{
	//	status = IDevice::STATUS_READY;
	//}
	//else
	//{
	//	status = IDevice::STATUS_BUSY;
	//}

	return ret;
}

long ThorPinholeStepperSimulator::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = TRUE;

	//if(deviceType & PINHOLE_WHEEL)
	//{
	//	pos = _pPos_C;
	//	ret = TRUE;
	//}

	return ret;
}

long ThorPinholeStepperSimulator::PostflightPosition()
{
	return TRUE;
}

long ThorPinholeStepperSimulator::MoveStepper(long pos)
{
	//char sendBuf[100];

	//memset(sendBuf,0,sizeof(sendBuf));

	//sprintf((char*)sendBuf,"/1A%dR\r",pos);

	//_serialPort.SendData((const unsigned char*)sendBuf, strlen((const char*)sendBuf));

	Sleep(100);

	//_serialPort.ReadData(sendBuf, 100);

	return TRUE;
}

long ThorPinholeStepperSimulator::GetLastErrorMsg(wchar_t * msg, long size)
{	
    //wcsncpy_s(msg,size,_errMsg,256);
	return TRUE;
}