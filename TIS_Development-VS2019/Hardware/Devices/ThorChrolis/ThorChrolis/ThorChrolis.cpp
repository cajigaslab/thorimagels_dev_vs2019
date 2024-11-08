// ThorChrolis.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "ThorChrolis.h"
#include "ThorChrolisXML.h"
#include "TL6WL.h"

bool CThorChrolis::_instanceFlag = false;
std::auto_ptr<CThorChrolis> CThorChrolis::_singleton(nullptr);

ViSession _deviceSession = -1;
ViStatus _lastDriverFunctionReturnCode = VI_SUCCESS;

IDevice::ConnectionStatusType _connectionState = IDevice::ConnectionStatusType::CONNECTION_UNAVAILABLE;
IDevice::StatusType _status = IDevice::StatusType::STATUS_BUSY;

#ifdef LOGGING_ENABLED
std::auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

HANDLE CThorChrolis::_hGetAdaptationStatus = NULL; // Initialize status thread
int CThorChrolis::_ledIndex = 0;

/*****************************************************************************
* CThorChrolis Class Stuff
*****************************************************************************/
CThorChrolis::CThorChrolis(void)
{
	_deviceSession = -1;
	_lastDriverFunctionReturnCode = VI_SUCCESS;
	_connectionState = IDevice::CONNECTION_UNAVAILABLE;
	_status = IDevice::StatusType::STATUS_BUSY;
	_led1Temp = 0; 
	_led2Temp = 0; 
	_led3Temp = 0;
	_led4Temp = 0;
	_led5Temp = 0;
	_led6Temp = 0;
	_runInAdaptationMode = FALSE; //Need to set this to TRUE to run the adaptation process for each dll
	_errMsg[0] = NULL;
	_ledsEnabled = FALSE;
	_led1State = false;
	_led2State = false;
	_led3State = false;
	_led4State = false;
	_led5State = false;
	_led6State = false;
	_led1NominalWavelength = 0; 
	_led2NominalWavelength = 0; 
	_led3NominalWavelength = 0; 
	_led4NominalWavelength = 0; 
	_led5NominalWavelength = 0; 
	_led6NominalWavelength = 0; 
}

CThorChrolis::~CThorChrolis(void)
{
	_instanceFlag = false;
}

CThorChrolis* CThorChrolis::getInstance(void)
{
	if(!_instanceFlag)
	{
		_singleton.reset(new CThorChrolis());
		_instanceFlag = true;
	}
	return _singleton.get();
}

/************************************************************************************************
* @fn	HANDLE MCM6000::GetStatusThread(DWORD &threadID)
*
* @brief	Create a Thread to request and read the status of the adaptation process.
* @param 	threadID	  	GetStatus Thread ID.
* @return	Thread Handle.
**************************************************************************************************/
HANDLE CThorChrolis::GetAdaptationStatus(DWORD &threadID)
{
	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &(CThorChrolis::GetAdaptionProgress), (void *) this, 0, &threadID);
	SetThreadPriority(handle, THREAD_PRIORITY_NORMAL);
	return handle;
}

void CThorChrolis::GetAdaptionProgress(LPVOID instance)
{
	wchar_t errMsg[MSG_SIZE]; 
	ViReal64 progress = 0;
	ViStatus rc1 = TL6WL_readAdaptionProgressLED (_deviceSession, &progress);
	if (VI_SUCCESS == rc1)
	{
		while(progress < 100)
		{
			StringCbPrintfW(errMsg,MSG_SIZE,L"LED %d Adaptation progress: %f",_ledIndex ,static_cast<float>(progress));
			LogMessage(errMsg,ERROR_EVENT);
			Sleep(200);
			rc1 = TL6WL_readAdaptionProgressLED (_deviceSession, &progress);
		}
		StringCbPrintfW(errMsg,MSG_SIZE,L"LED %d Adaptation progress: %f",_ledIndex ,static_cast<float>(progress));
		LogMessage(errMsg,ERROR_EVENT);
	}
	else
	{
		StringCbPrintfW(errMsg,MSG_SIZE,L"Could not read LED %d adaptation progress", _ledIndex);
		LogMessage(errMsg,ERROR_EVENT);
	}
}


/*****************************************************************************
* Device Handling
*****************************************************************************/
long CThorChrolis::FindDevices(long & deviceCount)
{
	Lock lock(_critSect);
	if (IDevice::CONNECTION_READY == _connectionState)
	{
		deviceCount = 1;
		return TRUE;
	}

	long ret = FALSE;
	ViUInt32 devCount = 0;

	// search for connected devices
	_lastDriverFunctionReturnCode = TL6WL_findRsrc(VI_NULL, &devCount);
	if ((VI_SUCCESS == _lastDriverFunctionReturnCode) &&
		(0 < devCount))
	{
		// get the resource string of first found device
		ViChar rscName[1024];
		_lastDriverFunctionReturnCode = TL6WL_getRsrcName(VI_NULL, 0, rscName);
		if (VI_SUCCESS == _lastDriverFunctionReturnCode)
		{
			// open first found device
			_deviceSession = -1;
			_lastDriverFunctionReturnCode = TL6WL_init(rscName, true, true, &_deviceSession);
			if (VI_SUCCESS == _lastDriverFunctionReturnCode)
			{
				_connectionState = IDevice::CONNECTION_READY;
				ret = TRUE;
			}
			else
			{
				devCount = 0;
				_connectionState = IDevice::CONNECTION_UNAVAILABLE;
				_deviceSession = -1;
				// TODO: throw exception
			}
		}
	}

	try
	{
		auto_ptr<ThorChrolisXML> pSetup(new ThorChrolisXML());
		//retrieve the nominal wavelengths from the settings xml
		//these wavelengths will be used to be displayed as labels in the GUI
		pSetup->GetNominalWavelengths(_led1NominalWavelength, _led2NominalWavelength, _led3NominalWavelength, _led4NominalWavelength, _led5NominalWavelength, _led6NominalWavelength);			
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorChrolisSettings.xml file");
		LogMessage(_errMsg,ERROR_EVENT);
		deviceCount = 0;
		return FALSE;
	}

	deviceCount = devCount;
	return ret;
}

long CThorChrolis::SelectDevice(const long device)
{
	//Initalize Linear Value (Master control value) to 100% (1000), recommended by Germany team
	SetParam(IDevice::PARAM_LEDS_LINEAR_VALUE, 1000);
	TL6WL_TU_ResetSequence(_deviceSession);
	TL6WL_TU_AddGeneratedSelfRunningSignal(_deviceSession, 1, VI_FALSE, 0, 4294967290, 0, 0);
	TL6WL_TU_AddGeneratedSelfRunningSignal(_deviceSession, 2, VI_FALSE, 0, 4294967290, 0, 0);
	TL6WL_TU_AddGeneratedSelfRunningSignal(_deviceSession, 3, VI_FALSE, 0, 4294967290, 0, 0);
	TL6WL_TU_AddGeneratedSelfRunningSignal(_deviceSession, 4, VI_FALSE, 0, 4294967290, 0, 0);
	TL6WL_TU_AddGeneratedSelfRunningSignal(_deviceSession, 5, VI_FALSE, 0, 4294967290, 0, 0);
	TL6WL_TU_AddGeneratedSelfRunningSignal(_deviceSession, 6, VI_FALSE, 0, 4294967290, 0, 0);

	long ret = TRUE;

	return ret;
}

long CThorChrolis::TeardownDevice(void)
{
	long ret = FALSE;
	SetParam(PARAM_LAMP_MODE_1, 0);
	SetParam(PARAM_LAMP_MODE_2, 0);
	SetParam(PARAM_LAMP_MODE_3, 0);
	SetParam(PARAM_LAMP_MODE_4, 0);
	SetParam(PARAM_LAMP_MODE_5, 0);
	SetParam(PARAM_LAMP_MODE_6, 0);
	TL6WL_TU_StartStopGeneratorOutput_TU(_deviceSession, VI_FALSE);
	if (IDevice::CONNECTION_READY == _connectionState)
	{
		_lastDriverFunctionReturnCode = TL6WL_close(_deviceSession);
		if (VI_SUCCESS != _lastDriverFunctionReturnCode)
		{
			_connectionState = IDevice::CONNECTION_ERROR_STATE;
		}
		else
		{
			ret = TRUE;
			_connectionState = IDevice::CONNECTION_UNAVAILABLE;
		}
		_deviceSession = -1;
	}

	return ret;
}


/*****************************************************************************
* General Device Information
*****************************************************************************/
// Error Handling
long CThorChrolis::GetLastErrorMsg(wchar_t * msg, long size)
{
	Lock lock(_critSect);
	if (IDevice::CONNECTION_READY != _connectionState)
	{
		return FALSE;
	}

	long ret = TRUE;

	ViChar errmsg[1024] = "";
	ViStatus rc = TL6WL_errorMessage(_deviceSession, _lastDriverFunctionReturnCode, errmsg);
	if (VI_SUCCESS != rc)
	{
		ret = FALSE;
		sprintf_s(errmsg, "%#x", _lastDriverFunctionReturnCode);
		_lastDriverFunctionReturnCode = rc;
	}
	const size_t msgLen = std::strlen(errmsg);
	size_t copyLen = 0, sizeT = size;
	errno_t errCode = mbstowcs_s(&copyLen, msg, sizeT, errmsg, msgLen);

	return ret;
}

// Parameter Information
long CThorChrolis::GetParamInfo(const long paramID, long & paramType,
								long & paramAvailable, long & paramReadOnly,
								double & paramMin, double & paramMax,
								double & paramDefault)
{
	if (IDevice::CONNECTION_READY != _connectionState)
	{
		return FALSE;
	}

	long ret = TRUE;
	switch(paramID)
	{
	case IDevice::PARAM_CONNECTION_STATUS:
		{
			paramType = IDevice::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = IDevice::CONNECTION_READY;
			paramMax = IDevice::CONNECTION_UNAVAILABLE;
			paramDefault = IDevice::CONNECTION_UNAVAILABLE;
		}
		break;

	case IDevice::PARAM_DEVICE_STATUS:
		{
			paramType = IDevice::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = IDevice::STATUS_BUSY;
			paramMax = IDevice::STATUS_ERROR;
			paramDefault = IDevice::STATUS_READY;
		}
		break;

	case IDevice::PARAM_LED1_CONTROL_NAME:
	case IDevice::PARAM_LED2_CONTROL_NAME:
	case IDevice::PARAM_LED3_CONTROL_NAME:
	case IDevice::PARAM_LED4_CONTROL_NAME:
	case IDevice::PARAM_LED5_CONTROL_NAME:
	case IDevice::PARAM_LED6_CONTROL_NAME:
		{
			paramType = IDevice::TYPE_STRING;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
		}
		break;

	case IDevice::PARAM_LED1_HEADS_COLOR_NAME:
	case IDevice::PARAM_LED2_HEADS_COLOR_NAME:
	case IDevice::PARAM_LED3_HEADS_COLOR_NAME:
	case IDevice::PARAM_LED4_HEADS_COLOR_NAME:
	case IDevice::PARAM_LED5_HEADS_COLOR_NAME:
	case IDevice::PARAM_LED6_HEADS_COLOR_NAME:
		{
			paramType = IDevice::TYPE_STRING;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
		}
		break;

	case IDevice::PARAM_LED1_CONTROL_CUSTOM_NAME:
	case IDevice::PARAM_LED2_CONTROL_CUSTOM_NAME:
	case IDevice::PARAM_LED3_CONTROL_CUSTOM_NAME:
	case IDevice::PARAM_LED4_CONTROL_CUSTOM_NAME:
	case IDevice::PARAM_LED5_CONTROL_CUSTOM_NAME:
	case IDevice::PARAM_LED6_CONTROL_CUSTOM_NAME:
		{
			paramType = IDevice::TYPE_STRING;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
		}
		break;

	case IDevice::PARAM_LED1_SN:
	case IDevice::PARAM_LED2_SN:
	case IDevice::PARAM_LED3_SN:
	case IDevice::PARAM_LED4_SN:
	case IDevice::PARAM_LED5_SN:
	case IDevice::PARAM_LED6_SN:
		{
			paramType = IDevice::TYPE_STRING;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
		}
		break;

	case IDevice::PARAM_LED1_SOCKEL_ID:
	case IDevice::PARAM_LED2_SOCKEL_ID:
	case IDevice::PARAM_LED3_SOCKEL_ID:
	case IDevice::PARAM_LED4_SOCKEL_ID:
	case IDevice::PARAM_LED5_SOCKEL_ID:
	case IDevice::PARAM_LED6_SOCKEL_ID:
		{
			paramType = IDevice::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
		}
		break;

	case IDevice::PARAM_LED1_PEAK_WAVELENGTH:
	case IDevice::PARAM_LED2_PEAK_WAVELENGTH:
	case IDevice::PARAM_LED3_PEAK_WAVELENGTH:
	case IDevice::PARAM_LED4_PEAK_WAVELENGTH:
	case IDevice::PARAM_LED5_PEAK_WAVELENGTH:
	case IDevice::PARAM_LED6_PEAK_WAVELENGTH:
		{
			paramType = IDevice::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 1000;
			paramDefault = 0;
		}
		break;

	case IDevice::PARAM_LED1_NOMINAL_WAVELENGTH:
	case IDevice::PARAM_LED2_NOMINAL_WAVELENGTH:
	case IDevice::PARAM_LED3_NOMINAL_WAVELENGTH:
	case IDevice::PARAM_LED4_NOMINAL_WAVELENGTH:
	case IDevice::PARAM_LED5_NOMINAL_WAVELENGTH:
	case IDevice::PARAM_LED6_NOMINAL_WAVELENGTH:
		{
			paramType = IDevice::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 1000;
			paramDefault = 0;
		}
		break;

	case IDevice::PARAM_LED1_LIGHT_COLOR:
	case IDevice::PARAM_LED2_LIGHT_COLOR:
	case IDevice::PARAM_LED3_LIGHT_COLOR:
	case IDevice::PARAM_LED4_LIGHT_COLOR:
	case IDevice::PARAM_LED5_LIGHT_COLOR:
	case IDevice::PARAM_LED6_LIGHT_COLOR:
		{
			paramType = IDevice::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0x000000;
			paramMax = 0xFFFFFF;
			paramDefault = 0;
		}
		break;

	case IDevice::PARAM_LED1_SPECTRUM_DATA:
	case IDevice::PARAM_LED2_SPECTRUM_DATA:
	case IDevice::PARAM_LED3_SPECTRUM_DATA:
	case IDevice::PARAM_LED4_SPECTRUM_DATA:
	case IDevice::PARAM_LED5_SPECTRUM_DATA:
	case IDevice::PARAM_LED6_SPECTRUM_DATA:
		{
			paramType = IDevice::TYPE_BUFFER;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
		}
		break;

	case IDevice::PARAM_LED1_TEMP:
	case IDevice::PARAM_LED2_TEMP:
	case IDevice::PARAM_LED3_TEMP:
	case IDevice::PARAM_LED4_TEMP:
	case IDevice::PARAM_LED5_TEMP:
	case IDevice::PARAM_LED6_TEMP:
		{
			paramType = IDevice::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;
		}
		break;

	case IDevice::PARAM_LED1_POWER_STATE:
	case IDevice::PARAM_LED2_POWER_STATE:
	case IDevice::PARAM_LED3_POWER_STATE:
	case IDevice::PARAM_LED4_POWER_STATE:
	case IDevice::PARAM_LED5_POWER_STATE:
	case IDevice::PARAM_LED6_POWER_STATE:
	case IDevice::PARAM_LEDS_ENABLE_DISABLE:
		{
			paramType = IDevice::TYPE_BOOL;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
		}
		break;

		// in promille [‰ >> 'Alt' + 0137
	case IDevice::PARAM_LED1_POWER:
	case IDevice::PARAM_LED2_POWER:
	case IDevice::PARAM_LED3_POWER:
	case IDevice::PARAM_LED4_POWER:
	case IDevice::PARAM_LED5_POWER:
	case IDevice::PARAM_LED6_POWER:
	case IDevice::PARAM_LEDS_LINEAR_VALUE:
		{
			paramType = IDevice::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1000;
			paramDefault = 0;
		}
		break;

	case IDevice::PARAM_LED1_CURRENT:
	case IDevice::PARAM_LED2_CURRENT:
	case IDevice::PARAM_LED3_CURRENT:
	case IDevice::PARAM_LED4_CURRENT:
	case IDevice::PARAM_LED5_CURRENT:
	case IDevice::PARAM_LED6_CURRENT:
		{
			paramType = IDevice::TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1000;
			paramDefault = 0;
		}
		break;

	case IDevice::PARAM_UPDATE_TEMPERATURES:
		{
			paramType = IDevice::TYPE_BOOL;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = FALSE;
			paramMax = TRUE;
			paramDefault = FALSE;
		}
		break;
	case IDevice::PARAM_LAMP_MODE_1:
	case IDevice::PARAM_LAMP_MODE_2:
	case IDevice::PARAM_LAMP_MODE_3:
	case IDevice::PARAM_LAMP_MODE_4:
	case IDevice::PARAM_LAMP_MODE_5:
		paramType = IDevice::TYPE_BOOL;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}
	return ret;
}


/*****************************************************************************
* Parameter Functions
*****************************************************************************/
long CThorChrolis::SetParam(const long paramID, const double param)
{
	if (IDevice::CONNECTION_READY != _connectionState)
	{
		return FALSE;
	}

	ViBoolean powerState = VI_FALSE;
	ViInt16 paramToSet = -1;
	long ret = TRUE;
	switch(paramID)
	{
	case IDevice::PARAM_LED1_POWER_STATE:
		{
			_led1State = TRUE == param;

			//Update LED state while imaging
			if(TRUE == _ledsEnabled)
			{
				SetLEDPowerState(TRUE);
			}

			if(TRUE == _runInAdaptationMode && TRUE == param)
			{
				RunAdaptationMode(1);
			}
		}
		break;
	case IDevice::PARAM_LED2_POWER_STATE:
		{
			_led2State = TRUE == param;

			//Update LED state while imaging
			if(TRUE == _ledsEnabled)
			{
				SetLEDPowerState(TRUE);
			}

			if(TRUE == _runInAdaptationMode && TRUE == param)
			{
				RunAdaptationMode(2);
			}
		}
		break;
	case IDevice::PARAM_LED3_POWER_STATE:
		{
			_led3State = TRUE == param;

			//Update LED state while imaging
			if(TRUE == _ledsEnabled)
			{
				SetLEDPowerState(TRUE);
			}

			if(TRUE == _runInAdaptationMode && TRUE == param)
			{
				RunAdaptationMode(3);
			}
		}
		break;
	case IDevice::PARAM_LED4_POWER_STATE:
		{
			_led4State = TRUE == param;

			//Update LED state while imaging
			if(TRUE == _ledsEnabled)
			{
				SetLEDPowerState(TRUE);
			}

			if(TRUE == _runInAdaptationMode && TRUE == param)
			{
				RunAdaptationMode(4);
			}
		}
		break;
	case IDevice::PARAM_LED5_POWER_STATE:
		{
			_led5State = TRUE == param;

			//Update LED state while imaging
			if(TRUE == _ledsEnabled)
			{
				SetLEDPowerState(TRUE);
			}

			if(TRUE == _runInAdaptationMode && TRUE == param)
			{
				RunAdaptationMode(5);
			}
		}
		break;
	case IDevice::PARAM_LED6_POWER_STATE:
		{
			_led6State = TRUE == param;

			//Update LED state while imaging
			if(TRUE == _ledsEnabled)
			{
				SetLEDPowerState(TRUE);
			}

			if(TRUE == _runInAdaptationMode && TRUE == param)
			{
				RunAdaptationMode(6);
			}
		}
		break;
	case IDevice::PARAM_LEDS_ENABLE_DISABLE:
		ret = SetLEDPowerState(param);
		if(TRUE == ret)
		{
			_ledsEnabled = static_cast<long>(param);
		}
		break;

	case IDevice::PARAM_LED1_POWER:
	case IDevice::PARAM_LED2_POWER:
	case IDevice::PARAM_LED3_POWER:
	case IDevice::PARAM_LED4_POWER:
	case IDevice::PARAM_LED5_POWER:
	case IDevice::PARAM_LED6_POWER:
		if(0 > param) // Less than 0 set to 0
		{
			ret = SetLEDPower(paramID, 0);
		}
		else if(0 <= param && 1000 >= param)
		{
			ret = SetLEDPower(paramID, param);
		}
		else // Bigger than 1000 set to 1000
		{
			ret = SetLEDPower(paramID, 1000);
		}
		break;

	case IDevice::PARAM_LEDS_LINEAR_VALUE:
		if(0 > param) // Less than 0 set to 0
		{
			ret = SetLinearValue(0);
		}
		else if(0 <= param && 1000 >= param)
		{
			ret = SetLinearValue(param);
		}
		else // Bigger than 1000 set to 1000
		{
			ret = SetLinearValue(1000);
		}
		break;

	case IDevice::PARAM_LAMP_MODE_1:
		if (SetExternal(1, param))
		{

			ret = TRUE;
			_externalMode1 = param;
			break;
		};
		ret = FALSE;
		break;
		
	case IDevice::PARAM_LAMP_MODE_2:
		if (SetExternal(2, param))
		{

			ret = TRUE;
			_externalMode2 = param;
			break;
		};
		ret = FALSE;
		break;
		
	case IDevice::PARAM_LAMP_MODE_3:
		if (SetExternal(3, param))
		{

			ret = TRUE;
			_externalMode3 = param;
			break;
		};
		ret = FALSE;
		break;
		
	case IDevice::PARAM_LAMP_MODE_4:
		if (SetExternal(4, param))
		{

			ret = TRUE;
			_externalMode4 = param;
			break;
		};
		ret = FALSE;
		break;
		
	case IDevice::PARAM_LAMP_MODE_5:
		if (SetExternal(5, param))
		{

			ret = TRUE;
			_externalMode5 = param;
			break;
		};
		ret = FALSE;
		break;
	case IDevice::PARAM_LAMP_MODE_6:
		if (SetExternal(6, param))
		{

			ret = TRUE;
			_externalMode6 = param;
			break;
		};
		ret = FALSE;
		break;

	default:
		ret = FALSE;
	}
	return ret;
}

long CThorChrolis::GetParam(const long paramID, double & param)
{
	if (IDevice::CONNECTION_READY != _connectionState)
	{
		return FALSE;
	}

	ViStatus rc = VI_SUCCESS;
	long ret = TRUE;
	switch(paramID)
	{
	case IDevice::PARAM_DEVICE_TYPE:
		param =  static_cast<double>(LAMP);
		break;

	case IDevice::PARAM_CONNECTION_STATUS:
		param = _connectionState;
		break;

	case IDevice::PARAM_DEVICE_STATUS:
		param = _status;
		break;

	case IDevice::PARAM_LED1_NOMINAL_WAVELENGTH:
		param = _led1NominalWavelength;
		break;
	case IDevice::PARAM_LED2_NOMINAL_WAVELENGTH:
		param = _led2NominalWavelength;
		break;
	case IDevice::PARAM_LED3_NOMINAL_WAVELENGTH:
		param = _led3NominalWavelength;
		break;
	case IDevice::PARAM_LED4_NOMINAL_WAVELENGTH:
		param = _led4NominalWavelength;
		break;
	case IDevice::PARAM_LED5_NOMINAL_WAVELENGTH:
		param = _led5NominalWavelength;
		break;
	case IDevice::PARAM_LED6_NOMINAL_WAVELENGTH:
		param = _led6NominalWavelength;
		break;

	case IDevice::PARAM_LED1_PEAK_WAVELENGTH:
	case IDevice::PARAM_LED2_PEAK_WAVELENGTH:
	case IDevice::PARAM_LED3_PEAK_WAVELENGTH:
	case IDevice::PARAM_LED4_PEAK_WAVELENGTH:
	case IDevice::PARAM_LED5_PEAK_WAVELENGTH:
	case IDevice::PARAM_LED6_PEAK_WAVELENGTH:
		{
			ret = GetLEDPeakWaveLength(paramID, param);
		}
		break;

	case IDevice::PARAM_LED1_POWER_STATE:
	case IDevice::PARAM_LED2_POWER_STATE:
	case IDevice::PARAM_LED3_POWER_STATE:
	case IDevice::PARAM_LED4_POWER_STATE:
	case IDevice::PARAM_LED5_POWER_STATE:
	case IDevice::PARAM_LED6_POWER_STATE:
		{
			ret = GetLEDPowerState(paramID, param);
		}
		break;

	case IDevice::PARAM_LEDS_ENABLE_DISABLE:
		{
			param = _ledsEnabled;
		}
		break;

	case IDevice::PARAM_LED1_POWER:
	case IDevice::PARAM_LED2_POWER:
	case IDevice::PARAM_LED3_POWER:
	case IDevice::PARAM_LED4_POWER:
	case IDevice::PARAM_LED5_POWER:
	case IDevice::PARAM_LED6_POWER:
		{
			ret = GetLEDPower(paramID, param);
		}
		break;
	case IDevice::PARAM_LEDS_LINEAR_VALUE:
		{
			ret = GetLinearValue(paramID, param);
		}
		break;
	case IDevice::PARAM_LED1_LIGHT_COLOR:
	case IDevice::PARAM_LED2_LIGHT_COLOR:
	case IDevice::PARAM_LED3_LIGHT_COLOR:
	case IDevice::PARAM_LED4_LIGHT_COLOR:
	case IDevice::PARAM_LED5_LIGHT_COLOR:
	case IDevice::PARAM_LED6_LIGHT_COLOR:
		{
			ret = GetLEDLightColor(paramID, param);
		}
		break;
	case IDevice::PARAM_LED1_TEMP:
		{
			param = _led1Temp;
		}
		break;
	case IDevice::PARAM_LED2_TEMP:
		{
			param = _led2Temp;
		}
		break;
	case IDevice::PARAM_LED3_TEMP:
		{
			param = _led3Temp;
		}
		break;
	case IDevice::PARAM_LED4_TEMP:
		{
			param = _led4Temp;
		}
		break;
	case IDevice::PARAM_LED5_TEMP:
		{
			param = _led5Temp;
		}
		break;
	case IDevice::PARAM_LED6_TEMP:
		{
			param = _led6Temp;
		}
		break;
	case IDevice::PARAM_UPDATE_TEMPERATURES:
		{
			//Don't check for the temperature while in adaptation mode. 
			param = (TRUE == _runInAdaptationMode) ? TRUE : UpdateLEDTemperatures();
		}
		break;
	case IDevice::PARAM_LAMP_MODE_1:
		param = _externalMode1;
		break;
	case IDevice::PARAM_LAMP_MODE_2:
		param = _externalMode2;
		ret = TRUE;
		break;
	case IDevice::PARAM_LAMP_MODE_3:
		param = _externalMode3;
		ret = TRUE;
		break;
	case IDevice::PARAM_LAMP_MODE_4:
		param = _externalMode4;
		ret = TRUE;
		break;
	case IDevice::PARAM_LAMP_MODE_5:
		param = _externalMode5;
		ret = TRUE;
		break;
	case IDevice::PARAM_LAMP_MODE_6:
		param = _externalMode6;
		ret = TRUE;
		break;

	}

	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		return FALSE;
	}

	return ret;
}

long CThorChrolis::SetParamString(const long paramID, wchar_t * str)
{
	if (IDevice::CONNECTION_READY != _connectionState)
	{
		return FALSE;
	}

	long ret = TRUE;

	return ret;
}

long CThorChrolis::GetParamString(const long paramID, wchar_t * str, long size)
{
	if (IDevice::CONNECTION_READY != _connectionState)
	{
		return FALSE;
	}

	ViStatus rc = VI_SUCCESS;
	long ret = TRUE;

	ViChar paramStr[MSG_SIZE] = "";
	switch(paramID)
	{
	case IDevice::PARAM_LED1_SN:
	case IDevice::PARAM_LED2_SN:
	case IDevice::PARAM_LED3_SN:
	case IDevice::PARAM_LED4_SN:
	case IDevice::PARAM_LED5_SN:
	case IDevice::PARAM_LED6_SN:
		{
			ret = GetLEDSerialNumber(paramID, paramStr);
		}
		break;

	case IDevice::PARAM_LED1_CONTROL_NAME:
	case IDevice::PARAM_LED2_CONTROL_NAME:
	case IDevice::PARAM_LED3_CONTROL_NAME:
	case IDevice::PARAM_LED4_CONTROL_NAME:
	case IDevice::PARAM_LED5_CONTROL_NAME:
	case IDevice::PARAM_LED6_CONTROL_NAME:
		{
			ret = GetLEDControlName(paramID, paramStr);
		}
		break;

	case IDevice::PARAM_LED1_HEADS_COLOR_NAME:
	case IDevice::PARAM_LED2_HEADS_COLOR_NAME:
	case IDevice::PARAM_LED3_HEADS_COLOR_NAME:
	case IDevice::PARAM_LED4_HEADS_COLOR_NAME:
	case IDevice::PARAM_LED5_HEADS_COLOR_NAME:
	case IDevice::PARAM_LED6_HEADS_COLOR_NAME:
		{
			ret = GetLEDHeadsColorName(paramID, paramStr);
		}
		break;

	case IDevice::PARAM_LED1_CONTROL_CUSTOM_NAME:
		{
			rc = TL6WL_readLED_HeadCustomName(_deviceSession, 0, paramStr);
		}
		break;
	case IDevice::PARAM_LED2_CONTROL_CUSTOM_NAME:
		{
			rc = TL6WL_readLED_HeadCustomName(_deviceSession, 1, paramStr);
		}
		break;
	case IDevice::PARAM_LED3_CONTROL_CUSTOM_NAME:
		{
			rc = TL6WL_readLED_HeadCustomName(_deviceSession, 2, paramStr);
		}
		break;
	case IDevice::PARAM_LED4_CONTROL_CUSTOM_NAME:
		{
			rc = TL6WL_readLED_HeadCustomName(_deviceSession, 3, paramStr);
		}
		break;
	case IDevice::PARAM_LED5_CONTROL_CUSTOM_NAME:
		{
			rc = TL6WL_readLED_HeadCustomName(_deviceSession, 4, paramStr);
		}
		break;
	case IDevice::PARAM_LED6_CONTROL_CUSTOM_NAME:
		{
			rc = TL6WL_readLED_HeadCustomName(_deviceSession, 5, paramStr);
		}
		break;

	default:
		ret = FALSE;
	}

	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		return FALSE;
	}

	size_t msgLen = std::strlen(paramStr);
	size_t copyLen = 0, sizeT = size;
	errno_t errCode = mbstowcs_s(&copyLen, str, sizeT, paramStr, msgLen);
	return ret;
}

long CThorChrolis::SetParamBuffer(const long paramID, char * buffer, long size)
{
	if (IDevice::CONNECTION_READY != _connectionState)
	{
		return FALSE;
	}

	long ret = TRUE;

	return ret;
}

long CThorChrolis::GetParamBuffer(const long paramID, char * buffer, long size)
{
	if (IDevice::CONNECTION_READY != _connectionState)
	{
		return FALSE;
	}

	long ret = TRUE;

	return ret;
}


/*****************************************************************************
* Experiment-Flow Control
*****************************************************************************/
long CThorChrolis::PreflightPosition(void)
{
	long ret = TRUE;

	return ret;
}

long CThorChrolis::SetupPosition(void)
{
	long ret = TRUE;

	return ret;
}

long CThorChrolis::StartPosition(void)
{
	long ret = TRUE;
	/*ViStatus rc = TL6WL_setLED_HeadBrightness(_deviceSession, _led1Power, _led2Power, _led3Power, _led4Power, _led5Power, _led6Power);
	if (VI_SUCCESS != rc)
	{
	_lastDriverFunctionReturnCode = rc;
	return FALSE;
	}
	rc = TL6WL_setLED_HeadPowerStates(_deviceSession, _led1State, _led2State, _led3State, _led4State, _led5State, _led6State);
	if (VI_SUCCESS != rc)
	{
	_lastDriverFunctionReturnCode = rc;
	return FALSE;
	}*/

	return ret;
}

long CThorChrolis::StatusPosition(long & status)
{
	long ret = TRUE;

	status = IDevice::STATUS_READY;

	return ret;
}

long CThorChrolis::PostflightPosition(void)
{
	long ret = TRUE;

	return ret;
}


// UNCLEAR
long CThorChrolis::ReadPosition(DeviceType deviceType, double & pos)
{
	long ret = TRUE;

	return ret;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void CThorChrolis::LogMessage(wchar_t *message,long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, message);
#endif
}

/*****************************************************************************
* Private Helper Functions
*****************************************************************************/
long CThorChrolis::GetLEDSerialNumber(const long paramID, char * pStr)
{
	Lock lock(_critSect);
	long ret = TRUE;

	ViChar led1[1024] = "", led2[1024] = "", led3[1024] = "", led4[1024] = "", led5[1024] = "", led6[1024] = "";
	ViStatus rc = TL6WL_readLED_HeadsSerial(_deviceSession, led1, led2, led3, led4, led5, led6);
	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Failed to Get LED SerialNumber");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	ViChar paramStr[1024] = "";
	switch (paramID)
	{
	case IDevice::PARAM_LED1_SN:
		strcpy_s(paramStr, led1);
		break;

	case IDevice::PARAM_LED2_SN:
		strcpy_s(paramStr, led2);
		break;

	case IDevice::PARAM_LED3_SN:
		strcpy_s(paramStr, led3);
		break;

	case IDevice::PARAM_LED4_SN:
		strcpy_s(paramStr, led4);
		break;

	case IDevice::PARAM_LED5_SN:
		strcpy_s(paramStr, led5);
		break;

	case IDevice::PARAM_LED6_SN:
		strcpy_s(paramStr, led6);
		break;
	}

	strcpy(pStr, paramStr);

	return ret;
}

long CThorChrolis::GetLEDControlName(const long paramID, char * pStr)
{
	Lock lock(_critSect);
	long ret = TRUE;

	ViChar led1[MSG_SIZE] = "", led2[MSG_SIZE] = "", led3[MSG_SIZE] = "", led4[MSG_SIZE] = "", led5[MSG_SIZE] = "", led6[MSG_SIZE] = "";
	ViStatus rc = TL6WL_readLED_HeadsSerial(_deviceSession, led1, led2, led3, led4, led5, led6);

	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Failed to Get Control Name");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	ViChar paramStr[1024] = "";
	switch (paramID)
	{
	case IDevice::PARAM_LED1_CONTROL_NAME:
		strcpy_s(paramStr, led1);
		break;

	case IDevice::PARAM_LED2_CONTROL_NAME:
		strcpy_s(paramStr, led2);
		break;

	case IDevice::PARAM_LED3_CONTROL_NAME:
		strcpy_s(paramStr, led3);
		break;

	case IDevice::PARAM_LED4_CONTROL_NAME:
		strcpy_s(paramStr, led4);
		break;

	case IDevice::PARAM_LED5_CONTROL_NAME:
		strcpy_s(paramStr, led5);
		break;

	case IDevice::PARAM_LED6_CONTROL_NAME:
		strcpy_s(paramStr, led6);
		break;
	}

	strcpy(pStr, paramStr);

	return ret;
}

long CThorChrolis::GetLEDHeadsColorName(const long paramID, char * pStr)
{
	Lock lock(_critSect);
	long ret = TRUE;

	ViChar led1[MSG_SIZE] = "", led2[MSG_SIZE] = "", led3[MSG_SIZE] = "", led4[MSG_SIZE] = "", led5[MSG_SIZE] = "", led6[MSG_SIZE] = "";
	ViStatus rc = TL6WL_readLED_HeadsColorName(_deviceSession, led1, led2, led3, led4, led5, led6);

	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Failed to Get LED Heads Color Name");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	ViChar paramStr[1024] = "";
	switch (paramID)
	{
	case IDevice::PARAM_LED1_HEADS_COLOR_NAME:
		strcpy_s(paramStr, led1);
		break;

	case IDevice::PARAM_LED2_HEADS_COLOR_NAME:
		strcpy_s(paramStr, led2);
		break;

	case IDevice::PARAM_LED3_HEADS_COLOR_NAME:
		strcpy_s(paramStr, led3);
		break;

	case IDevice::PARAM_LED4_HEADS_COLOR_NAME:
		strcpy_s(paramStr, led4);
		break;

	case IDevice::PARAM_LED5_HEADS_COLOR_NAME:
		strcpy_s(paramStr, led5);
		break;

	case IDevice::PARAM_LED6_HEADS_COLOR_NAME:
		strcpy_s(paramStr, led6);
		break;
	}

	strcpy(pStr, paramStr);

	return ret;
}

long CThorChrolis::GetLEDPeakWaveLength(const long paramID, double & param)
{
	Lock lock(_critSect);
	long ret = FALSE;

	ViUInt8 ledID = 0;

	switch (paramID)
	{
	case IDevice::PARAM_LED1_PEAK_WAVELENGTH:
		ledID = 1;
		break;
	case IDevice::PARAM_LED2_PEAK_WAVELENGTH:
		ledID = 2;
		break;
	case IDevice::PARAM_LED3_PEAK_WAVELENGTH:
		ledID = 3;
		break;
	case IDevice::PARAM_LED4_PEAK_WAVELENGTH:
		ledID = 4;
		break;
	case IDevice::PARAM_LED5_PEAK_WAVELENGTH:
		ledID = 5;
		break;
	case IDevice::PARAM_LED6_PEAK_WAVELENGTH:
		ledID = 6;
		break;

	default:
		break;
	}

	ViUInt16 wl = 0;
	ViStatus rc = TL6WL_readLED_HeadPeakWL(_deviceSession, ledID, &wl);
	if (VI_SUCCESS == rc)
	{
		param = static_cast<double>(wl);
		ret = TRUE;
	} 
	else
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Failed to Get LED Peak Wavelength");
		LogMessage(_errMsg,ERROR_EVENT);
		ret = FALSE;
	}

	return ret;
}

long CThorChrolis::GetLEDLightColor(const long paramID, double & param)
{
	Lock lock(_critSect);
	long ret = TRUE;

	ViUInt32 ledColorCode1 = 0, ledColorCode2 = 0, ledColorCode3 = 0, ledColorCode4 = 0, ledColorCode5 = 0, ledColorCode6 = 0;

	ViStatus rc = TL6WL_readLED_HeadsColorCode(_deviceSession, &ledColorCode1, &ledColorCode2, &ledColorCode3, &ledColorCode4, &ledColorCode5, &ledColorCode6);
	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Failed to Get LED Light Color");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	switch (paramID)
	{
	case IDevice::PARAM_LED1_LIGHT_COLOR:
		param = ledColorCode1;
		break;
	case IDevice::PARAM_LED2_LIGHT_COLOR:
		param = ledColorCode2;
		break;
	case IDevice::PARAM_LED3_LIGHT_COLOR:
		param = ledColorCode3;
		break;
	case IDevice::PARAM_LED4_LIGHT_COLOR:
		param = ledColorCode4;
		break;
	case IDevice::PARAM_LED5_LIGHT_COLOR:
		param = ledColorCode5;
		break;
	case IDevice::PARAM_LED6_LIGHT_COLOR:
		param = ledColorCode6;
		break;
	default:
		param = 0;
		ret = FALSE;
		break;
	}

	return ret;
}

long CThorChrolis::UpdateLEDTemperatures()
{
	Lock lock(_critSect);
	long ret = TRUE;

	ViStatus rc = TL6WL_getLED_HeadTemperature(_deviceSession, &_led1Temp, &_led2Temp, &_led3Temp, &_led4Temp, &_led5Temp, &_led6Temp);
	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Failed to Get LED Temperatures");
		LogMessage(_errMsg,ERROR_EVENT);
		ret = FALSE;
	}

	return ret;
}

long CThorChrolis::GetLEDPowerState(const long paramID, double & param)
{
	Lock lock(_critSect);
	long ret = TRUE;

	ViBoolean powerState = VI_FALSE;
	ViBoolean led1 = VI_FALSE, led2 = VI_FALSE, led3 = VI_FALSE, led4 = VI_FALSE, led5 = VI_FALSE, led6 = VI_FALSE;

	ViStatus rc = TL6WL_getLED_HeadPowerStates(_deviceSession, &led1, &led2, &led3, &led4, &led5, &led6);
	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Failed to Get LED PowerState");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	switch (paramID)
	{
	case IDevice::PARAM_LED1_POWER_STATE:
		powerState = led1;
		break;
	case IDevice::PARAM_LED2_POWER_STATE:
		powerState = led2;
		break;
	case IDevice::PARAM_LED3_POWER_STATE:
		powerState = led3;
		break;
	case IDevice::PARAM_LED4_POWER_STATE:
		powerState = led4;
		break;
	case IDevice::PARAM_LED5_POWER_STATE:
		powerState = led5;
		break;
	case IDevice::PARAM_LED6_POWER_STATE:
		powerState = led6;
		break;
	}
	param = static_cast<double>(powerState);

	return ret;
}

// If adaptation mode is enabled, instead of enabling the power state of the LED the software
// will run the adaptation process for the enabled LED 
long CThorChrolis::SetLEDPowerState(const double state)
{
	Lock lock(_critSect);
	long ret = TRUE;

	ViBoolean led1 = VI_FALSE, led2 = VI_FALSE, led3 = VI_FALSE, led4 = VI_FALSE, led5 = VI_FALSE, led6 = VI_FALSE;

	if(FALSE == state)  // If the flag is set to false, turn off all LEDs
	{
		ViStatus rc = TL6WL_setLED_HeadPowerStates(_deviceSession, led1, led2, led3, led4, led5, led6);
		if (VI_SUCCESS != rc)
		{
			_lastDriverFunctionReturnCode = rc;
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Failed to Set LED PowerState");
			LogMessage(_errMsg,ERROR_EVENT);
			return FALSE;
		}
	}
	else
	{
		ViStatus rc = TL6WL_getLED_HeadPowerStates(_deviceSession, &led1, &led2, &led3, &led4, &led5, &led6);
		if (VI_SUCCESS != rc)
		{
			_lastDriverFunctionReturnCode = rc;
			return FALSE;
		}

		led1 = static_cast<ViBoolean>(_led1State);
		led2 = static_cast<ViBoolean>(_led2State);
		led3 = static_cast<ViBoolean>(_led3State);
		led4 = static_cast<ViBoolean>(_led4State);
		led5 = static_cast<ViBoolean>(_led5State);
		led6 = static_cast<ViBoolean>(_led6State);

		if(FALSE == _runInAdaptationMode)
		{
			rc = TL6WL_setLED_HeadPowerStates(_deviceSession, led1, led2, led3, led4, led5, led6);
			if (VI_SUCCESS != rc)
			{
				_lastDriverFunctionReturnCode = rc;
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Failed to Set LED PowerState");
				LogMessage(_errMsg,ERROR_EVENT);
				return FALSE;
			}
		}
	}
	TL6WL_TU_StartStopGeneratorOutput_TU(_deviceSession, VI_TRUE);

	return ret;
}

long CThorChrolis::GetLEDPower(const long paramID, double & param)
{
	Lock lock(_critSect);
	long ret = TRUE;

	ViUInt16 led1 = 0, led2 = 0, led3 = 0, led4 = 0, led5 = 0, led6 = 0, paramToGet = 0;

	ViStatus rc = TL6WL_getLED_HeadBrightness(_deviceSession, &led1, &led2, &led3, &led4, &led5, &led6);

	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Failed to get LED Power");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	switch (paramID)
	{
	case IDevice::PARAM_LED1_POWER:
		paramToGet = led1;
		break;
	case IDevice::PARAM_LED2_POWER:
		paramToGet = led2;
		break;
	case IDevice::PARAM_LED3_POWER:
		paramToGet = led3;
		break;
	case IDevice::PARAM_LED4_POWER:
		paramToGet = led4;
		break;
	case IDevice::PARAM_LED5_POWER:
		paramToGet = led5;
		break;
	case IDevice::PARAM_LED6_POWER:
		paramToGet = led6;
		break;
	}

	param = static_cast<double>(paramToGet);

	return ret;
}

long CThorChrolis::SetLEDPower(const long paramID, const double param)
{
	Lock lock(_critSect);
	long ret = TRUE;

	ViInt16 paramToSet = static_cast<ViInt16>(param);
	ViInt16 led1 = -1, led2 = -1, led3 = -1, led4 = -1, led5 = -1, led6 = -1;

	switch (paramID)
	{
	case IDevice::PARAM_LED1_POWER:
		led1 = paramToSet;
		break;
	case IDevice::PARAM_LED2_POWER:
		led2 = paramToSet;
		break;
	case IDevice::PARAM_LED3_POWER:
		led3 = paramToSet;
		break;
	case IDevice::PARAM_LED4_POWER:
		led4 = paramToSet;
		break;
	case IDevice::PARAM_LED5_POWER:
		led5 = paramToSet;
		break;
	case IDevice::PARAM_LED6_POWER:
		led6 = paramToSet;
		break;
	}

	ViStatus rc = TL6WL_setLED_HeadBrightness(_deviceSession, led1, led2, led3, led4, led5, led6);
	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Failed to set LED Power");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	return ret;
}

long CThorChrolis::GetLinearValue(const long paramID, double & param)
{
	Lock lock(_critSect);
	long ret = TRUE;

	ViUInt16 masterValue = 0;

	ViStatus rc = TL6WL_getLED_LinearModeValue(_deviceSession, &masterValue);
	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		return FALSE;
	}

	param = static_cast<double>(masterValue);

	return ret;
}

long CThorChrolis::SetLinearValue(const double param)
{
	Lock lock(_critSect);
	long ret = TRUE;

	ViInt16 masterValue = static_cast<ViInt16>(param);

	ViStatus rc = TL6WL_setLED_LinearModeValue(_deviceSession, masterValue);
	if (VI_SUCCESS != rc)
	{
		_lastDriverFunctionReturnCode = rc;
		ret = FALSE;
	}

	return ret;
}

long CThorChrolis::SetExternal(long ledIndex, bool param)
{
	char buffer[MSG_SIZE];
	ViBoolean powerState = VI_FALSE;
	ViInt16 paramToSet = -1;
	long ret = FALSE;
	ViUInt8 signalNr = ledIndex;
	ViBoolean activeLow = VI_FALSE;
	ViBoolean activeHigh = VI_TRUE;
	ViUInt32 startDelayus = 0;
	ViUInt32 activeTimeus = 4294967295;
	ViUInt32 inactiveTimeus = 0;
	ViUInt32 repititionCount = 0;
	ViBoolean startsLow = VI_FALSE;
	ViBoolean enableGen = TRUE;
	ViUInt32 edgeCount = 1;
	ViPInt16 selfTestResult = 0;
	ViUInt16 affectedSignalBitmask = 0;
	ViStatus status = 0;
	/*status = TL6WL_TU_AddTriggerPoint(_deviceSession, signalNr, activeHigh, edgeCount,
		affectedSignalBitmask);
		
	TL6WL_errorMessage(_deviceSession, status, buffer);*/
	status = TL6WL_TU_ResetSequence(_deviceSession);
	int externalList[6] = {_externalMode1, _externalMode2, _externalMode3, _externalMode4, _externalMode5, _externalMode6};
	for (int i = 0; i < 6; i++)
	{
		/*Skip adding self-running signal if the current led is being enabled for external mode */
		if (ledIndex == i + 1 )
			if(param) continue;
			else
			{
				status = TL6WL_TU_AddGeneratedSelfRunningSignal(_deviceSession, i+1, activeLow, 
					startDelayus, activeTimeus, inactiveTimeus, repititionCount);
				if (status)
				{
					TL6WL_errorMessage(_deviceSession, status, buffer);
					StringCbPrintfW(_errMsg, MSG_SIZE, L"Failed to reset LED for ThorChrolis");
					LogMessage(_errMsg, ERROR_EVENT);
					return FALSE;
				}
				continue;
			}
		/*Reset all signals and add the signal for LED's not in external mode. Since there is no 'delete' function of signal.*/
		if (!externalList[i])
			status = TL6WL_TU_AddGeneratedSelfRunningSignal(_deviceSession, i+1, activeLow, startDelayus, activeTimeus, inactiveTimeus, repititionCount);
	}
	status = TL6WL_TU_StartStopGeneratorOutput_TU(_deviceSession, activeHigh);
	return TRUE;
}

long CThorChrolis::RunAdaptationMode(long ledIndex)
{
	Lock lock(_critSect);
	_ledIndex = ledIndex;
	ViStatus rc = TL6WL_startAdaptionLED (_deviceSession, static_cast<ViUInt8>(ledIndex));
	if (VI_SUCCESS == rc)
	{
		DWORD ThreadForStatus;
		if(NULL != _hGetAdaptationStatus)
		{
			SAFE_DELETE_HANDLE(_hGetAdaptationStatus);	
		}

		_hGetAdaptationStatus = GetAdaptationStatus(ThreadForStatus);
	}
	else
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Could not start LED %d adaptation", ledIndex);
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}