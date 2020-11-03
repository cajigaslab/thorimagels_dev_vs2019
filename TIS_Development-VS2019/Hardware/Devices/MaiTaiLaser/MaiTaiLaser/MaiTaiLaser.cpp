// MaiTaiLaser.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MaiTaiLaser.h"
#include "MaiTaiLaserSetupXML.h"
#include <string>
#include "Strsafe.h"

#define SLEEP_TIME_100					100
#define SLEEP_TIME_200					200
#define SLEEP_TIME_500					500
#define WATCHDOG_TIMER_SECS				3600 

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll ( new LogDll ( L".\\ThorLoggingUnmanaged.dll" ) );
#endif

/**********************************************************************************************//**
																								* @fn	CMaiTaiLaser::CMaiTaiLaser()
																								*
																								* @brief	Default constructor.
																								*
																								**************************************************************************************************/
CMaiTaiLaser::CMaiTaiLaser()
{
	_serialPortBuffer = nullptr;
	_deviceDetected = FALSE;
	_waveLengthMin = 700;
	_waveLengthMax = 900;
	_isLaserOff = true;
	if ( _serialPortBuffer == nullptr )
	{
		_serialPortBuffer = new char[256];
		memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) ); //Initiate the serial communication buffer.
	}
	_curShutterStatus = 0;
	_curShutter2Status = 0;
	_polling = NULL;
	_blockUpdateParam = FALSE;
	_errMsg[0] = NULL;
}

/**********************************************************************************************//**
																								* @fn	CMaiTaiLaser::~CMaiTaiLaser()
																								*
																								* @brief	Destructor.
																								*
																								**************************************************************************************************/
CMaiTaiLaser::~CMaiTaiLaser()
{
	delete[] _serialPortBuffer ;
	_serialPortBuffer = nullptr;
	_instanceFlag = false;
}

bool CMaiTaiLaser:: _instanceFlag = false;

auto_ptr<CMaiTaiLaser> CMaiTaiLaser::_single ( new CMaiTaiLaser() );

/**********************************************************************************************//**
																								* @fn	CMaiTaiLaser* CMaiTaiLaser::getInstance()
																								*
																								* @brief	Gets the instance. Singlton design pattern
																								*
																								* @return	null if it fails, else the instance.
																								**************************************************************************************************/

CMaiTaiLaser* CMaiTaiLaser::getInstance()
{
	if ( !_instanceFlag )
	{
		_single.reset ( new CMaiTaiLaser() );
		_instanceFlag = true;
		return _single.get();
	}

	else
	{
		return _single.get();
	}
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::FindDevices ( long& DeviceCount )
																								*
																								* @brief	Searches for the devices.
																								*
																								* @param [in,out]	DeviceCount	Index of devices.
																								*
																								* @return	False if no device found. True if device found.
																								**************************************************************************************************/

long CMaiTaiLaser::FindDevices ( long& DeviceCount )
{
	long ret = TRUE;
	_blockUpdateParam = FALSE;
	DeviceCount = 0;
	if ( _deviceDetected )
	{
		_serialPort.Close();
		_deviceDetected = FALSE;
	}

	long portID = 0;
	long baudRate = 0;

	try
	{
		auto_ptr<CMaiTaiLaserSetupXML> pSetup ( new CMaiTaiLaserSetupXML() );
		pSetup->GetConnection ( portID , baudRate );
	}
	catch ( ... )
	{
		StringCbPrintfW ( _errMsg, MSG_SIZE, L"Unable to locate MaiTaiLaserSettings.xml file" );
		return FALSE;
	}

	if ( FALSE == _serialPort.Open ( portID, baudRate ) ) // Try baudRate 9600 and 38400( Note: 38400 is demonstration program bandrate, 9600 is normal bandrate)
	{
		if ( 9600 == baudRate )
		{
			baudRate = 38400;
		}
		else
		{
			baudRate = 9600;
		}

		if ( FALSE == _serialPort.Open ( portID, baudRate ) )
		{
			StringCbPrintfW ( _errMsg, MSG_SIZE, L"MaiTaiLaser FindDevices could not open serial port" );
			LogMessage ( _errMsg );
			ret = FALSE;
			_deviceDetected = FALSE;
		}
	}
	//try laser query:
	if (FALSE == QueryLaserManufacturer())
	{
		_serialPort.Close();

		if ( 9600 == baudRate )
		{
			baudRate = 38400; //The demonstration program uses 38400 baud.
		}
		else
		{
			baudRate = 9600; //Normally, 9600 baud.
		}

		if ( FALSE == _serialPort.Open ( portID, baudRate ) )
		{
			return FALSE;
		}
		else
		{
			if ( FALSE == QueryLaserManufacturer() ) //make sure it's maitai laser.
			{
				_serialPort.Close();
				return FALSE;
			}
		}
	}

	if ( TRUE == ret )
	{
		DeviceCount = 1;
		_deviceDetected = TRUE;
		_serialPort.Close();
	}

	return ret;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::SelectDevice ( const long Device )
																								*
																								* @brief	Select device.
																								*
																								* @param	Device	The device index .
																								*
																								* @return	A long.
																								**************************************************************************************************/

long CMaiTaiLaser::SelectDevice ( const long Device )
{
	if (_isLaserOff){
		long	ret = FALSE;

		if ( FALSE == _deviceDetected )
		{
			StringCbPrintfW ( _errMsg, MSG_SIZE, L"No devices found" );
			return FALSE;
		}

		long portID = 0;
		long baudRate = 9600;

		try
		{
			auto_ptr<CMaiTaiLaserSetupXML> pSetup ( new CMaiTaiLaserSetupXML() );
			pSetup->GetConnection ( portID, baudRate );
		}

		catch ( ... )
		{
			StringCbPrintfW ( _errMsg, MSG_SIZE, L"Unable to locate MaiTaiLaserSettings.xml file" );
			return FALSE;
		}

		if ( FALSE == _serialPort.Open ( portID, baudRate ) )
		{
			StringCbPrintfW ( _errMsg, MSG_SIZE, L"MaiTaiLaser is unable to open port %d", portID );
			return FALSE;
		}

		if ( FALSE == QueryLaserManufacturer() )
		{
			_serialPort.Close();

			if ( 9600 == baudRate )
			{
				baudRate = 38400; //The demonstration program uses 38400 baud.
			}

			else
			{
				baudRate = 9600; //Normally, 9600 baud.
			}

			if ( FALSE == _serialPort.Open ( portID, baudRate ) )
			{
				StringCbPrintfW ( _errMsg, MSG_SIZE, L"MaiTaiLaser is unable to open port %d", portID );
				return FALSE;
			}

			else
			{
				if ( FALSE == QueryLaserManufacturer() || GetWarmUpStatus() != 100 ) //make sure it's maitai laser and it's warmed-up.
				{
					return FALSE;
				}
			}
		}

		if ( FALSE == QueryLaserParams() ) // Get the Min and Max value of laser wavelength
		{
			return FALSE;
		}

		GetParametersID(); //Assign the Table Parameter ID
		BuildParamTable(); //Initiate the Parameter Table
		// TurnOnLaser();     //Turn on the laser
		_isLaserOff = false;
		//the laser will sleep if there is not consistent
		//communication. Create a thread that send ON command
		//to keep the laser com awake
		if(_polling)
		{
			if(_polling->joinable())
			{
				_polling->join();
				delete(_polling);
			}
		}
		_polling = new thread(staticPollingHandler, this);
	}
	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::TeardownDevice()
																								*
																								* @brief	Teardown device.
																								*
																								* @return	A long.
																								**************************************************************************************************/

long CMaiTaiLaser::TeardownDevice()
{
	UpdateParameter ( _laserParams.ShutterStatus, 0 ); //close the shutter
	UpdateParameter ( _laserParams.Shutter2Status, 0); //close second shutter (for Insight laser, 1040nm shutter)
	//_polling->join();
	_isLaserOff = true;
	Sleep(100);

	TurnOffLaser(); // turn off the laser
	_serialPort.Close();

	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::GetParamInfo ( const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault )
																								*
																								* @brief	Gets parameter information.
																								*
																								* @param	paramID				  	Identifier for the parameter.
																								* @param [in,out]	paramType	  	Type of the parameter.
																								* @param [in,out]	paramAvailable	The parameter available.
																								* @param [in,out]	paramReadOnly 	The parameter read only.
																								* @param [in,out]	paramMin	  	The parameter minimum.
																								* @param [in,out]	paramMax	  	The parameter maximum.
																								* @param [in,out]	paramDefault  	The parameter default.
																								*
																								* @return	The parameter information.
																								**************************************************************************************************/

long CMaiTaiLaser::GetParamInfo ( const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault )
{
	if ( NULL != _tableParams[paramID] )
	{
		if ( _tableParams[paramID]->GetParamID() == paramID )
		{
			paramType = _tableParams[paramID]->GetParamType();
			paramAvailable = _tableParams[paramID]->GetParamAvailable();
			paramReadOnly = _tableParams[paramID]->GetParamReadOnly();
			paramMin = _tableParams[paramID]->GetParamMin();
			paramMax = _tableParams[paramID]->GetParamMax();
			paramDefault = _tableParams[paramID]->GetParamDefault();
			return TRUE;
		}
	}

	paramAvailable = FALSE;
	return FALSE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::PreflightPosition()
																								*
																								* @brief	Preflight position.
																								*
																								* @return	A long.
																								**************************************************************************************************/

long CMaiTaiLaser::PreflightPosition()
{
	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::StatusPosition ( long& status )
																								*
																								* @brief	Status position.
																								*
																								* @param [in,out]	status	The status.
																								*
																								* @return	A long.
																								**************************************************************************************************/

long CMaiTaiLaser::StatusPosition ( long& status )
{
	status = 1;

	for ( std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter )
	{
		if ( NULL == iter->second )
		{
			continue;
		}

		else
			if ( TRUE == ( iter->second )->GetParamSetEnable() )
			{
				double params = 0;
				GetParam ( ( iter -> second )->GetParamID(), params );

				if ( params != ( iter -> second )->GetParamVal() )
				{
					status = 0;
				}

				else
				{
					( iter->second )->UpdateParam_C ( params );
					( iter->second )->SetParamSetEnable ( FALSE );
				}
			}
	}

	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::ReadPosition ( DeviceType deviceType, double& pos )
																								*
																								* @brief	Reads a position.
																								*
																								* @param	deviceType 	Type of the device.
																								* @param [in,out]	pos	The position.
																								*
																								* @return	The position.
																								**************************************************************************************************/

long CMaiTaiLaser::ReadPosition ( DeviceType deviceType, double& pos )
{
	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::PostflightPosition()
																								*
																								* @brief	Postflight position.
																								*
																								* @return	A long.
																								**************************************************************************************************/

long CMaiTaiLaser::PostflightPosition()
{
	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::GetLastErrorMsg ( wchar_t* msg, long size )
																								*
																								* @brief	Gets the last error message.
																								*
																								* @param [in,out]	msg	If non-null, the message.
																								* @param	size	   	The size.
																								*
																								* @return	The last error message.
																								**************************************************************************************************/

long CMaiTaiLaser::GetLastErrorMsg ( wchar_t* msg, long size )
{
	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	void CMaiTaiLaser::LogMessage ( wchar_t* message )
																								*
																								* @brief	Logs a message.
																								*
																								* @param [in,out]	message	If non-null, the message.
																								**************************************************************************************************/

void CMaiTaiLaser::LogMessage ( wchar_t* message )
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent ( VERBOSE_EVENT, 1, message );
#endif
}

/**********************************************************************************************//**
																								* @fn	wchar_t* CMaiTaiLaser::convertCharArrayToLPCWSTR ( const char* charArray )
																								*
																								* @brief	Convert character array to lpcwstr.
																								*
																								* @param	charArray	Array of characters.
																								*
																								* @return	null if it fails, else the character converted array to lpcwstr.
																								**************************************************************************************************/

wchar_t* CMaiTaiLaser::convertCharArrayToLPCWSTR ( const char* charArray )
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar ( CP_ACP, 0, charArray, -1, wString, 4096 );
	return wString;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::QueryLaserManufacturer ( void )
																								*
																								* @brief	Queries Laser Manufacturer.
																								*
																								* @return	The Laser Manufacturer.
																								**************************************************************************************************/

long CMaiTaiLaser::QueryLaserManufacturer ( void )
{
	try
	{
		Lock lock ( critSect );
		const char* _sendBuffer = "*IDN?\r";
		int nBytesSent = _serialPort.SendData((const unsigned char*) _sendBuffer, static_cast<int>(strlen(_sendBuffer)));
		Sleep (SLEEP_TIME_200);
		memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
		_serialPort.ReadData ( _serialPortBuffer, 100 );

		if ( 0x4D == ( unsigned char ) _serialPortBuffer[16] && 0x54 == ( unsigned char ) _serialPortBuffer[19] ) // Check Maitai in the read string
		{
			SetWatchDog(WATCHDOG_TIMER_SECS); // set watch dog timer to be 1hour
			return TRUE;
		}
		else if(strlen(_serialPortBuffer) > 1)
		{
			SetWatchDog(0); // set watch dog timer to be 0 if it is a newer Laser (Insight)
			return TRUE;
		}
		else
		{
			nBytesSent = _serialPort.SendData ( ( const unsigned char* ) _sendBuffer, static_cast<int> ( strlen ( _sendBuffer ) ) ); // try again
			Sleep (SLEEP_TIME_200);
			memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
			_serialPort.ReadData ( _serialPortBuffer, 100 );

			if ( 0x4D == ( unsigned char ) _serialPortBuffer[16] && 0x54 == ( unsigned char ) _serialPortBuffer[19] )
			{
				SetWatchDog(WATCHDOG_TIMER_SECS); // set watch dog timer to be 1hour
				return TRUE;
			}
		}
	}
	catch(...)
	{
		StringCbPrintfW ( _errMsg, MSG_SIZE, L"MaiTaiLaser QueryLaserManufacturer failed" );
	}
	return FALSE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::QueryLaserParams ( void )
																								*
																								* @brief	Queries laser parameters.
																								*
																								*
																								* @return	The laser parameters.
																								**************************************************************************************************/

long CMaiTaiLaser::QueryLaserParams ( void )
{
	Lock lock ( critSect );
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
	char _sendBuffer[] = "WAVelength:MAX?\r";
	int nBytesSent = _serialPort.SendData ( ( const unsigned char* ) _sendBuffer, static_cast<int> ( strlen ( _sendBuffer ) ) ); //Get the max wavelength value
	Sleep (SLEEP_TIME_100);
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
	_serialPort.ReadData ( _serialPortBuffer, 256 );
	char wavemax[4] = {0};
	memcpy ( wavemax, _serialPortBuffer, 4 );
	_waveLengthMax = atol ( wavemax );
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
	_sendBuffer[12] = 'I';
	_sendBuffer[13] = 'N';
	nBytesSent = _serialPort.SendData ( ( const unsigned char* ) _sendBuffer, static_cast<int> ( strlen ( _sendBuffer ) ) ); //Get the min wavelength value
	Sleep (SLEEP_TIME_100);
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
	_serialPort.ReadData ( _serialPortBuffer, 256 );
	char wavemin[4] = {0};
	memcpy ( wavemin, _serialPortBuffer, 4 );
	_waveLengthMin = atol ( wavemin );
	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::SetParam ( const long paramID, const double param )
																								*
																								* @brief	Sets a parameter.
																								*
																								* @param	paramID	Identifier for the parameter.
																								* @param	param  	The parameter value.
																								*
																								* @return	A long.
																								**************************************************************************************************/

long CMaiTaiLaser::SetParam ( const long paramID, const double param )
{
	if ( NULL != _tableParams[paramID] )
	{
		if ( _tableParams[paramID]->GetParamID() == paramID ) //check read only and available properties
		{
			if ( FALSE == ( _tableParams[paramID]->GetParamAvailable() ) || ( TRUE == _tableParams[paramID]->GetParamReadOnly() ) )
			{
				return FALSE;
			}

			else
				if ( ( _tableParams[paramID]->GetParamMin() <= param ) && ( _tableParams[paramID]->GetParamMax() >= param ) ) // check available range
				{
					_tableParams[paramID]->UpdateParam ( param );
					if(PARAM_LASER1_SHUTTER_POS == paramID)
					{
						_curShutterStatus = param;
					}
					else if(PARAM_LASER1_SHUTTER2_POS == paramID)
					{
						_curShutter2Status = param;
					}
					return TRUE;
				}

				else
				{
					StringCbPrintfW ( _errMsg, MSG_SIZE, L"MaiTaiLaser SetParam failed. paramID: %d", paramID );
					LogMessage ( _errMsg );
					return FALSE;
				}
		}
	}

	return FALSE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::SetupPosition()
																								*
																								* @brief	Sets up the position.
																								*
																								* @return	A long.
																								**************************************************************************************************/

long CMaiTaiLaser::SetupPosition()
{
	long ret = false;

	for ( std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter )
	{
		if ( NULL == iter->second )
		{
			continue;
		}

		else
			if ( ( FALSE == ( iter->second )->GetParamAvailable() ) || ( TRUE == ( iter->second )->GetParamReadOnly() ) )
			{
				continue;
			}

			else
				if ( ( iter->second )->GetParamVal() != ( iter->second )->GetParamCurrent() ) //set only when value is different
				{
					( iter->second )->SetParamSetEnable ( TRUE );
					ret = true;
				}

				else
				{
					( iter->second )->SetParamSetEnable ( FALSE );
				}
	}

	return ret;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::StartPosition()
																								*
																								* @brief	Starts a position.
																								*
																								*
																								* @return	A long.
																								**************************************************************************************************/

long CMaiTaiLaser::StartPosition()
{
	long ret = FALSE;
	_blockUpdateParam = TRUE; //Do not allow the param to be updater when reading while in this function

	for ( std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter )
	{
		if ( NULL == iter->second )
		{
			continue;
		}

		else
			if ( ( FALSE == ( iter->second )->GetParamAvailable() ) || ( TRUE == ( iter->second )->GetParamReadOnly() ) )
			{
				continue;
			}

			else
				if ( TRUE == ( iter->second )->GetParamSetEnable() )
				{
					SetParams ( iter->second );
					( iter->second )->UpdateParam_C ( ( iter->second )->GetParamVal() );
					( iter->second )->SetParamSetEnable ( FALSE );
					ret = TRUE;
					StringCbPrintfW ( _errMsg, MSG_SIZE, L"StartPosition succeeded at paramID: %d", ( iter->second )->GetParamID() );
					LogMessage ( _errMsg );
				}
	}

	_blockUpdateParam = FALSE; //Allow the param to be updater when reading at end of this function
	return ret;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::SetParams ( ParamInfo* pParamInfo )
																								*
																								* @brief	Sets the parameters.
																								*
																								* @param [in,out]	pParamInfo	If non-null, information describing the parameter.
																								*
																								* @return	A long.
																								**************************************************************************************************/

long CMaiTaiLaser::SetParams ( ParamInfo* pParamInfo )
{
	Lock lock ( critSect );
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
#pragma warning(push)
#pragma warning(disable:4996)
	string cmdStr ( cmd.begin(), cmd.end() - 1 );
	cmdStr = cmdStr + " " + std::to_string ( ( long ) ( pParamInfo->GetParamVal() ) ) + "\r";
	size_t len = cmdStr.copy ( _serialPortBuffer, cmdStr.length(), 0 );
#pragma warning(pop)
	_serialPort.SendData ( ( const unsigned char* ) ( _serialPortBuffer ), static_cast<int> ( len ) );
	Sleep (SLEEP_TIME_200); //Determined by testing... Shorter sleep time may cause bad communication
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
	_serialPort.ReadData ( _serialPortBuffer, 256 );
	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::GetParam ( const long paramID, double& param )
																								*
																								* @brief	Gets a parameter.
																								*
																								* @param	paramID		 	Identifier for the parameter.
																								* @param [in,out]	param	The parameter value.
																								*
																								* @return	The parameter.
																								**************************************************************************************************/

long CMaiTaiLaser::GetParam ( const long paramID, double& param )
{
	long	ret = TRUE;

	if ( NULL != _tableParams[paramID] )
	{
		if ( FALSE ==  _tableParams[paramID]->GetParamAvailable() )
		{
			return FALSE;
		}
		else if ( paramID == _laserParams.WaveLength || PARAM_LASER1_POS_CURRENT == paramID )
		{
			QueryParams ( _tableParams[paramID] );
			char wave[4] = {0};
			memcpy ( wave, _serialPortBuffer, 4 );
			param = atol ( wave );

			//Only update param and param_c when not executing StartPosition()
			if ( FALSE == _blockUpdateParam )
			{
				_tableParams[paramID]->UpdateParam_C ( param );
			}
		}

		else if ( paramID == _laserParams.ShutterStatus || PARAM_LASER1_SHUTTER_POS_CURRENT == paramID )
		{
			if(!_isLaserOff && PARAM_LASER1_SHUTTER_POS_CURRENT == paramID)
			{
				param = _curShutterStatus;
			}
			else
			{

				QueryParams ( _tableParams[paramID] );
				char ShutStatus[1] = {0};
				memcpy ( ShutStatus, _serialPortBuffer, 1 );
				param = atoi ( ShutStatus );

				//Only update param and param_c when not executing StartPosition()
				if ( FALSE == _blockUpdateParam )
				{
					_tableParams[paramID]->UpdateParam_C ( param );
				}
			}
		}

		else if ( paramID == _laserParams.Shutter2Status || PARAM_LASER1_SHUTTER2_POS_CURRENT == paramID )
		{
			if(!_isLaserOff && PARAM_LASER1_SHUTTER2_POS_CURRENT == paramID)
			{
				param = _curShutter2Status;
			}
			else
			{

				QueryParams ( _tableParams[paramID] );
				char ShutStatus[1] = {0};
				memcpy ( ShutStatus, _serialPortBuffer, 1 );
				param = atoi ( ShutStatus );

				//Only update param and param_c when not executing StartPosition()
				if ( FALSE == _blockUpdateParam )
				{
					_tableParams[paramID]->UpdateParam_C ( param );
				}
			}
		}

		else if(paramID == PARAM_CONNECTION_STATUS)
		{					
			if(_deviceDetected)
			{
				double p = GetWarmUpStatus();
				if(0 <= p && 100 >= p)
				{
					param = (100 == p)? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_WARMING_UP;
				}
				else
				{
					param = (double)ConnectionStatusType::CONNECTION_ERROR_STATE;
				}
			}
			else
			{
				param = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
			}
		}

		else
		{
			param = static_cast<double> ( _tableParams[paramID]->GetParamVal() );
		}

		return TRUE;
	}

	StringCbPrintfW ( _errMsg, MSG_SIZE, L"Parameter not implemented" );
	return FALSE;
}

/// <summary>
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long CMaiTaiLaser::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long CMaiTaiLaser::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long CMaiTaiLaser::SetParamString(const long paramID, wchar_t* str)
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
long CMaiTaiLaser::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::QueryParams ( std::vector<unsigned char> cmd )
																								*
																								* @brief	Queries the parameters.
																								*
																								* @param	cmd	The command.
																								*
																								* @return	The parameters.
																								**************************************************************************************************/

long CMaiTaiLaser::QueryParams ( std::vector<unsigned char> cmd )
{
	Lock lock ( critSect );
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
#pragma warning(push)
#pragma warning(disable:4996)
	string cmdStr ( cmd.begin(), cmd.end() );
	size_t len = cmdStr.copy ( _serialPortBuffer, cmdStr.length(), 0 );
#pragma warning(pop)
	_serialPort.SendData ( ( const unsigned char* ) ( _serialPortBuffer ), static_cast<int> ( cmd.size() - 1 ) );
	Sleep (SLEEP_TIME_200); //Determined by testing... Shorter sleep time may cause bad communication
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
	_serialPort.ReadData ( _serialPortBuffer, 256 );
	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::QueryParams ( ParamInfo* pParamInfo )
																								*
																								* @brief	Queries the parameters.
																								*
																								* @param [in,out]	pParamInfo	If non-null, information describing the parameter.
																								*
																								* @return	The parameters.
																								**************************************************************************************************/

long CMaiTaiLaser::QueryParams ( ParamInfo* pParamInfo )
{
	Lock lock ( critSect );
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
#pragma warning(push)
#pragma warning(disable:4996)
	string cmdStr ( cmd.begin(), cmd.end() - 1 );
	cmdStr = cmdStr + "?\r";
	size_t len = cmdStr.copy ( _serialPortBuffer, cmdStr.length(), 0 );
#pragma warning(pop)
	_serialPort.SendData ( ( const unsigned char* ) ( _serialPortBuffer ), static_cast<int> ( len ) );
	Sleep (SLEEP_TIME_200); //Determined by testing... Shorter sleep time may cause bad communication
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
	_serialPort.ReadData ( _serialPortBuffer, 256 );
	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	double CMaiTaiLaser::GetWarmUpStatus()
																								*
																								* @brief	Gets warmup status.
																								*
																								* @return	The warmup status.
																								**************************************************************************************************/

double CMaiTaiLaser::GetWarmUpStatus()
{
	char* warmUpStatus = NULL;
	try
	{
		unsigned char commandByte[] = {"READ:PCTWarmedup?\r"}; // read wavelength status
		std::vector<unsigned char> commandBytes;
		commandBytes.assign ( commandByte, commandByte + sizeof ( commandByte ) / sizeof ( unsigned char ) );
		QueryParams ( commandBytes );
		long long index = 0;

		for ( int i = 0; i < 256; i++ )
		{
			if ( '%' == _serialPortBuffer[i] )
			{
				index = i;
				break;
			}
			else if ( '\0' == _serialPortBuffer[i] )
			{
				index = i;
				break;
			}
		}

		warmUpStatus = new char[index - 1];
		memcpy ( warmUpStatus, _serialPortBuffer, index - 1 );
		double param = atoi ( warmUpStatus );
		delete[] warmUpStatus;
		warmUpStatus = NULL;
		return param;
	}
	catch(...)
	{
		if(NULL != warmUpStatus)
		{
			delete[] warmUpStatus;
			warmUpStatus = NULL;
		}
		StringCbPrintfW ( _errMsg, MSG_SIZE, L"MaiTaiLaser GetWarmUpStatus failed.");
		return 0;
	}
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::BuildParamTable()
																								*
																								* @brief	Builds parameter table.
																								*

																								* @return	A long.
																								**************************************************************************************************/

long CMaiTaiLaser::BuildParamTable()
{
	_tableParams.clear();
	std::vector<unsigned char> commandBytes;
	ParamInfo* tempParamInfo  = new ParamInfo (
		_laserParams.DeviceType,							        //ID
		static_cast<double> ( LASER1 ),								//VAL
		static_cast<double> ( LASER1 ),								//PARAM C
		FALSE,										                //PARAM B
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,
		FALSE,
		FALSE,                                                      //READ ONLY
		static_cast<double> ( LASER1 ),								//MIN
		static_cast<double> ( LASER1 ),	                         //MAX
		static_cast<double> ( LASER1 ),								//DEFAULT
		commandBytes );
	_tableParams.insert ( std::pair<long, ParamInfo*> ( Params::PARAM_DEVICE_TYPE, tempParamInfo ) );
	unsigned char commandByteWaveSet[] = {"WAVelength"}; // read wavelength status
	commandBytes.assign ( commandByteWaveSet, commandByteWaveSet + sizeof ( commandByteWaveSet ) / sizeof ( commandByteWaveSet[0] ) );
	tempParamInfo  = new ParamInfo (
		PARAM_LASER1_POS_CURRENT,							        //ID
		WAVELENGTH_DEFAULT,								            //VAL
		WAVELENGTH_DEFAULT,								            //PARAM C
		FALSE,										                //PARAM B
		TYPE_LONG,									                //TYPE
		TRUE,										                //AVAILABLE
		TRUE,                                                       //READ ONLY
		FALSE,
		FALSE,
		_waveLengthMin,								                //MIN
		_waveLengthMax,	                                            //MAX
		WAVELENGTH_DEFAULT,								            //DEFAULT
		commandBytes );
	_tableParams.insert ( std::pair<long, ParamInfo*> ( PARAM_LASER1_POS_CURRENT, tempParamInfo ) );
	tempParamInfo = new ParamInfo (
		_laserParams.WaveLength,
		WAVELENGTH_DEFAULT,
		WAVELENGTH_DEFAULT,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		_waveLengthMin,
		_waveLengthMax,
		WAVELENGTH_DEFAULT,
		commandBytes );
	_tableParams.insert ( std::pair<long, ParamInfo*> ( _laserParams.WaveLength, tempParamInfo ) );
	unsigned char commandByteShutterSet[] = {"SHUTter"}; // read wavelength status
	commandBytes.assign ( commandByteShutterSet, commandByteShutterSet + sizeof ( commandByteShutterSet ) / sizeof ( commandByteShutterSet[0] ) );
	tempParamInfo = new ParamInfo (
		_laserParams.ShutterStatus,
		0,
		0,
		FALSE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		0,
		1,
		0,
		commandBytes );
	_tableParams.insert ( std::pair<long, ParamInfo*> ( _laserParams.ShutterStatus, tempParamInfo ) );
	tempParamInfo  = new ParamInfo (
		PARAM_LASER1_SHUTTER_POS_CURRENT,							        //ID
		0,								                                    //VAL
		0,								                                    //PARAM C
		FALSE,										                        //PARAM B
		TYPE_LONG,									                         //TYPE
		TRUE,										                        //AVAILABLE
		TRUE,
		FALSE,
		FALSE,                                                               //READ ONLY
		0,															         //MIN
		1,	                                                                 //MAX
		0,								                                     //DEFAULT
		commandBytes );
	_tableParams.insert ( std::pair<long, ParamInfo*> ( PARAM_LASER1_SHUTTER_POS_CURRENT, tempParamInfo ) );

	unsigned char commandByteShutter2Set[] = {"IRSHUTter"}; // read wavelength status
	commandBytes.assign ( commandByteShutter2Set, commandByteShutter2Set + sizeof ( commandByteShutter2Set ) / sizeof ( commandByteShutter2Set[0] ) );
	tempParamInfo = new ParamInfo (
		_laserParams.Shutter2Status,
		0,
		0,
		FALSE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		0,
		1,
		0,
		commandBytes );
	_tableParams.insert ( std::pair<long, ParamInfo*> ( _laserParams.Shutter2Status, tempParamInfo ) );
	tempParamInfo  = new ParamInfo (
		PARAM_LASER1_SHUTTER2_POS_CURRENT,							        //ID
		0,								                                    //VAL
		0,								                                    //PARAM C
		FALSE,										                        //PARAM B
		TYPE_LONG,									                         //TYPE
		TRUE,										                        //AVAILABLE
		TRUE,
		FALSE,
		FALSE,                                                               //READ ONLY
		0,															         //MIN
		1,	                                                                 //MAX
		0,								                                     //DEFAULT
		commandBytes );
	_tableParams.insert ( std::pair<long, ParamInfo*> ( PARAM_LASER1_SHUTTER2_POS_CURRENT, tempParamInfo ) );

	unsigned char commandByteConnectionStatus[] = {"read:pctwarmedup?"}; // read wavelength status
	commandBytes.assign(commandByteConnectionStatus, commandByteConnectionStatus + sizeof(commandByteConnectionStatus) / sizeof(commandByteConnectionStatus[0]));
	tempParamInfo = new ParamInfo (
		PARAM_CONNECTION_STATUS,	//ID
		0,						//val
		0,						//param C
		0,						//param B
		TYPE_LONG,				//type
		TRUE,					//available
		TRUE,					//read only
		FALSE,					//conversion
		FALSE,					//conversion factor
		(double)ConnectionStatusType::CONNECTION_WARMING_UP,	//min
		(double)ConnectionStatusType::CONNECTION_ERROR_STATE,	//max
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,	//default
		commandBytes );
	_tableParams.insert ( std::pair<long, ParamInfo*> ( PARAM_CONNECTION_STATUS, tempParamInfo ) );

	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	long CMaiTaiLaser::GetParametersID()
																								*
																								* @brief	Gets parameters identifier.
																								*
																								* @return	The parameters identifier.
																								**************************************************************************************************/

long CMaiTaiLaser::GetParametersID()
{
	_laserParams.ShutterStatus = PARAM_LASER1_SHUTTER_POS,
		_laserParams.Shutter2Status = PARAM_LASER1_SHUTTER2_POS,
		_laserParams.WaveLength = PARAM_LASER1_POS;
	_laserParams.DeviceType = PARAM_DEVICE_TYPE;
	return TRUE;
}

/**********************************************************************************************//**
																								* @fn	void CMaiTaiLaser::TurnOnLaser()
																								*
																								* @brief	Turn on laser.
																								*
																								**************************************************************************************************/

void CMaiTaiLaser::TurnOnLaser()
{
	Lock lock ( critSect );
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
	char _sendBuffer[] = "ON\r";
	int nBytesSent = _serialPort.SendData ( ( const unsigned char* ) _sendBuffer, static_cast<int> ( strlen ( _sendBuffer ) ) ); //Get the max wavelength value
	//Sleep (SLEEP_TIME_100);
	//memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
	//_serialPort.ReadData ( _serialPortBuffer, 256 );
}

/**********************************************************************************************//**
																								* @fn	void CMaiTaiLaser::TurnOffLaser()
																								*
																								* @brief	Turn off laser.
																								*
																								**************************************************************************************************/

void CMaiTaiLaser::TurnOffLaser()
{
	Lock lock ( critSect );
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
	char _sendBuffer[] = "OFF\r";
	int nBytesSent = _serialPort.SendData ( ( const unsigned char* ) _sendBuffer, static_cast<int> ( strlen ( _sendBuffer ) ) ); //Get the max wavelength value
	Sleep (SLEEP_TIME_500);
	memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
	_serialPort.ReadData ( _serialPortBuffer, 256 );
}

void* CMaiTaiLaser::staticPollingHandler(void* obj)
{
	((CMaiTaiLaser*) obj)->pollingHandler(); 
	return NULL;
}

void CMaiTaiLaser::pollingHandler()
{
	DWORD startTime = 0;
	const DWORD POLLING_GAP_MS = 30000;

	while(true)
	{
		if(_isLaserOff)
		{
			break;
		}

		if((GetTickCount() - startTime)>POLLING_GAP_MS)
		{
			TurnOnLaser();
			startTime = GetTickCount();
		}
	}
}

/**********************************************************************************************//**
																								* @fn	void CMaiTaiLaser::UpdateParameter ( const long paramID, const double param )
																								*
																								* @brief	Updates the parameter.
																								*
																								*
																								* @param	paramID	Identifier for the parameter.
																								* @param	param  	The parameter.
																								**************************************************************************************************/

void CMaiTaiLaser::UpdateParameter ( const long paramID, const double param )
{
	SetParam ( paramID, param );
	PreflightPosition();
	SetupPosition();
	StartPosition();
	PostflightPosition();
}

long CMaiTaiLaser::SetWatchDog (long seconds )
{
	Lock lock(critSect);

	if(0 < seconds)
	{
		string cmdStr = "Timer:watchdog " + std::to_string(seconds) + "\r";
		_serialPort.SendData((const unsigned char*)(cmdStr.c_str()), static_cast<int>(cmdStr.length()));
	}
	else
	{
		string cmdStrChk = "Timer:watchdog?\r";
		_serialPort.SendData((const unsigned char*)(cmdStrChk.c_str()), static_cast<int>(cmdStrChk.length()));
		Sleep (SLEEP_TIME_200); //Determined by testing... Shorter sleep time may cause bad communication
		memset ( _serialPortBuffer, 0, 256 * sizeof ( char ) );
		_serialPort.ReadData ( _serialPortBuffer, 256 );

		if(std::atof(_serialPortBuffer) > seconds)
		{
			string cmdStr = "Timer:watchdog " + std::to_string(seconds) + "\r";
			_serialPort.SendData((const unsigned char*)(cmdStr.c_str()), static_cast<int>(cmdStr.length()));
		}
	}

	return TRUE;
}