// ThorMCM6000_Condenser.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "ThorMCM6000_CondenserXML.h"
#include "ThorMCM6000_Condenser.h"

/// <summary>
/// Prevents a default instance of the <see cref="ThorMCM6000_Condenser"/> class from being created.
/// </summary>
ThorMCM6000_Condenser::ThorMCM6000_Condenser()
{
	_errMsg[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorMCM6000_Condenser"/> class.
/// </summary>
ThorMCM6000_Condenser::~ThorMCM6000_Condenser()
{
	_instanceFlag = false;
	_errMsg[0] = 0;
}

bool ThorMCM6000_Condenser::_instanceFlag = false;
auto_ptr<ThorMCM6000_Condenser> ThorMCM6000_Condenser::_single(new ThorMCM6000_Condenser());
CritSect ThorMCM6000_Condenser::critSec;
long ThorMCM6000_Condenser::_selectedDevice = -1;
long ThorMCM6000_Condenser::_deviceFound = FALSE;

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorMCM6000_Condenser *.</returns>
ThorMCM6000_Condenser* ThorMCM6000_Condenser::getInstance()
{
	Lock lock(critSec);

	if (!_instanceFlag)
	{
		_single.reset(new ThorMCM6000_Condenser());
		_instanceFlag = true;
	}
	return _single.get();
}

/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="deviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::FindDevices(long& deviceCount)
{
	Lock lock(critSec);
	long ret = FALSE;
	MCM6000Stage::getInstance()->FindDevices(deviceCount);

	//Only display the secondary Z if an MCM6000 is found and the Condenser is set up in ThorMCM6000Settings.xml
	if (deviceCount > 0)
	{
		if (TRUE == MCM6000Stage::getInstance()->IsCondenserAvailable())
		{
			_deviceFound = ret = TRUE;
		}
		else
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000_Condenser: FindDevice->Condenser is not set up in ThorMCM6000Settings.xml");
			LogMessage(errMsg, ERROR_EVENT);
		}
	}
	else
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000_Condenser: FindDevice->ThorMCM6000.dll is not connected to any device");
		LogMessage(errMsg, ERROR_EVENT);
	}
	/*try
	{
		//Get portID, etc from hardware ThorMCM6000_CondenserSettings.xml
		auto_ptr<ThorMCM6000_CondenserXML> pSetup(new ThorMCM6000_CondenserXML());
		pSetup->GetConnection(portID[i], baudRate, _settingsSerialNumber[i]);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Unable to locate ThorMCM6000_CondenserSettings.xml file");
		LogMessage(_errMsg);
	}*/

	return ret;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::SelectDevice(const long device)
{
	Lock lock(critSec);
	long ret = FALSE;
	if (TRUE == _deviceFound && MCM6000Stage::getInstance()->IsMCM6000Connected())
	{
		_selectedDevice = device;
		ret = TRUE;
	}
	return ret;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorMCM6000_Condenser::LogMessage(wchar_t* message, long eventType)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventType, 1, message);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::TeardownDevice()
{
	_deviceFound = FALSE;
	_selectedDevice = -1;
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
long ThorMCM6000_Condenser::GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault)
{
	long	ret = TRUE;
	if (MCM6000Stage::getInstance()->IsMCM6000Connected())
	{
		switch (paramID)
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
		break;

		case PARAM_Z_POS:
		{
			ret = MCM6000Stage::getInstance()->GetParamInfo(PARAM_CONDENSER_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
		}
		break;

		case PARAM_Z_POS_CURRENT:
		{
			ret = MCM6000Stage::getInstance()->GetParamInfo(PARAM_CONDENSER_POS_CURRENT, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
		}
		break;

		case PARAM_Z_STATUS:
		{
			ret = MCM6000Stage::getInstance()->GetParamInfo(PARAM_CONDENSER_STATUS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
		}
		break;

		case PARAM_Z_ZERO:
		{
			ret = MCM6000Stage::getInstance()->GetParamInfo(PARAM_CONDENSER_ZERO, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
		}
		break;

		case PARAM_Z_INVERT:
		{
			ret = MCM6000Stage::getInstance()->GetParamInfo(PARAM_CONDENSER_INVERT, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
		}
		break;

		case PARAM_Z_STOP:
		{
			ret = MCM6000Stage::getInstance()->GetParamInfo(PARAM_CONDENSER_STOP, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
		}
		break;


		case PARAM_Z_POS_MOVE_BY:
		{
			ret = MCM6000Stage::getInstance()->GetParamInfo(PARAM_CONDENSER_POS_MOVE_BY, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
		}
		break;

		case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = (STAGE_Z | STAGE_Z2);
		}
		break;

		default:
			paramAvailable = FALSE;
			ret = TRUE;
		}
	}
	else
	{
		ret = FALSE;
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000_Condenser: GetParamInfo failed, ThorMCM6000.dll is not connected to any device");
		LogMessage(errMsg, ERROR_EVENT);
	}

	return ret;
}

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::SetParam(const long paramID, const double param)
{
	if (MCM6000Stage::getInstance()->IsMCM6000Connected())
	{
		switch (paramID)
		{
		case PARAM_Z_POS:
		{
			MCM6000Stage::getInstance()->SetParam(PARAM_CONDENSER_POS, param);
			break;
		}
		case PARAM_Z_JOG:
		{
			MCM6000Stage::getInstance()->SetParam(PARAM_CONDENSER_JOG, param);
			break;
		}
		case PARAM_Z_ZERO:
		{
			MCM6000Stage::getInstance()->SetParam(PARAM_CONDENSER_ZERO, param);
			break;
		}
		case PARAM_Z_INVERT:
		{
			MCM6000Stage::getInstance()->SetParam(PARAM_CONDENSER_INVERT, param);
			break;
		}
		case PARAM_Z_STOP:
		{
			MCM6000Stage::getInstance()->SetParam(PARAM_CONDENSER_STOP, param);
			break;
		}
		case PARAM_Z_POS_MOVE_BY:
		{
			MCM6000Stage::getInstance()->SetParam(PARAM_CONDENSER_POS_MOVE_BY, param);
			break;
		}
		}
	}
	else
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000_Condenser: SetParam failed, ThorMCM6000.dll is not connected to any device");
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

/// <summary>
/// Rounds the specified number.
/// </summary>
/// <param name="number">The number.</param>
/// <param name="decimals">The decimals.</param>
/// <returns>double.</returns>
double ThorMCM6000_Condenser::Round(double number, int decimals)
{
	double decP = std::pow(10, decimals);
	double ret;
	if (number < 0.0)
		ret = -std::floor(-number * decP + 0.5) / decP;
	else
		ret = std::floor(number * decP + 0.5) / decP;
	return ret;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::GetParam(const long paramID, double& param)
{
	long	ret = TRUE;
	if (MCM6000Stage::getInstance()->IsMCM6000Connected())
	{
		switch (paramID)
		{
		case PARAM_Z_POS:
			MCM6000Stage::getInstance()->GetParam(PARAM_CONDENSER_POS, param);
			break;
		case PARAM_Z_POS_CURRENT:
			MCM6000Stage::getInstance()->GetParam(PARAM_CONDENSER_POS_CURRENT, param);
			break;
		case PARAM_DEVICE_TYPE:
			param = static_cast<double>(STAGE_Z | STAGE_Z2);
			break;
		case PARAM_CONNECTION_STATUS:
			param = (MCM6000Stage::getInstance()->IsMCM6000Connected()) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
			break;
		case PARAM_Z_INVERT:
			MCM6000Stage::getInstance()->GetParam(PARAM_CONDENSER_INVERT, param);
			break;
		default:
			ret = FALSE;
		}
	}
	else
	{
		ret = FALSE;
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000_Condenser: GetParam failed, ThorMCM6000.dll is not connected to any device");
		LogMessage(errMsg, ERROR_EVENT);
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
long ThorMCM6000_Condenser::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	return TRUE;
}

/// <summary>
/// Gets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	return TRUE;
}

/// <summary>
/// Sets the parameter string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::SetParamString(const long paramID, wchar_t* str)
{
	return TRUE;
}

/// <summary>
/// Gets the parameter of type string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;
	switch (paramID)
	{

	}
	return ret;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::StartPosition()
{
	//Only send the command if the MCM6000 is connected
	if (MCM6000Stage::getInstance()->IsMCM6000Connected())
	{
		MCM6000Stage::getInstance()->StartPosition();
	}
	else
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000_Condenser: StartPosition failed, ThorMCM6000.dll is not connected to any device");
		LogMessage(errMsg, ERROR_EVENT);
	}
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::SetupPosition()
{
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::GetLastErrorMsg(wchar_t* msg, long size)
{
	return TRUE;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::StatusPosition(long& status)
{
	if (MCM6000Stage::getInstance()->IsMCM6000Connected())
	{
		MCM6000Stage::getInstance()->StatusPosition(status);
	}
	else
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000_Condenser: StatusPosition failed, ThorMCM6000.dll is not connected to any device");
		LogMessage(errMsg, ERROR_EVENT);
	}

	return TRUE;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::ReadPosition(DeviceType deviceType, double& pos)
{
	return FALSE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorMCM6000_Condenser::PreflightPosition()
{
	return TRUE;
}


