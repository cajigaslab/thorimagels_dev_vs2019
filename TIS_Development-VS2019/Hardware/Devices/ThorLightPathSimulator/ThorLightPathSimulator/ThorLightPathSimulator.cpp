// ThorLightPathSimulator.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Serial.h"
#include "ThorLightPathSimulator.h"
#include "ThorLightPathSimulatorXML.h"
#include "Strsafe.h"
#include "ParamInfo.h"
#include <string>
#include <math.h>
#include <iomanip>

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

/// <summary>
/// The message
/// </summary>
wchar_t message[256];
const string ThorLightPathSimulator::_deviceSignature[DEVICE_NUM] = {"GG", "GR", "CAM"};


/// <summary>
/// Prevents a default instance of the <see cref="ThorLightPathSimulator"/> class from being created.
/// </summary>
ThorLightPathSimulator::ThorLightPathSimulator()
{
	_deviceDetected[0] = FALSE;
	for (int i = 0; i < DEVICE_NUM; i++)
	{
		_deviceDetected[i + 1] = FALSE;
	}
	_errMsg[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorLightPathSimulator"/> class.
/// </summary>
ThorLightPathSimulator::~ThorLightPathSimulator()
{
	_instanceFlag = false;
	_errMsg[0] = 0;	
}

bool ThorLightPathSimulator::_instanceFlag = false;
auto_ptr<ThorLightPathSimulator> ThorLightPathSimulator::_single (new ThorLightPathSimulator());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorLightPathSimulator *.</returns>
ThorLightPathSimulator* ThorLightPathSimulator::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorLightPathSimulator());
		_instanceFlag = true;
	}
	return _single.get();
}

/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="deviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorLightPathSimulator::FindDevices(long &deviceCount)
{
	Lock lock(_critSect);
	long ret = FALSE;
	long baudRate=0;
	deviceCount=0;

	for(int i=0; i<DEVICE_NUM; i++)
	{
		//_deviceDetected[DEVICE_NUM] works as a flag to indicate some device has been found.
		_deviceDetected[DEVICE_NUM] = _deviceDetected[i] = TRUE;
		deviceCount = 1;
		ret = TRUE;
	}

	return ret;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorLightPathSimulator::SelectDevice(const long device)
{
	Lock lock(_critSect);
	long ret = FALSE;
	long portID=0;
	long baudRate=0;

	//Check if any device detected before continuing
	if(FALSE == _deviceDetected[DEVICE_NUM]) 
	{
		return FALSE;
	}

	for(int i=0; i< DEVICE_NUM; i++)
	{
			_deviceDetected[DEVICE_NUM] = _deviceDetected[i] = TRUE;
			ret = TRUE;
	}
	BuildParamTable();
	return ret;
}


/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorLightPathSimulator::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorLightPathSimulator::TeardownDevice()
{	
	//Set LightPaths to their default position before clearing the table of parameters
	SetParam(PARAM_LIGHTPATH_GG, 0);
	SetParam(PARAM_LIGHTPATH_GR, 0);
	SetParam(PARAM_LIGHTPATH_CAMERA, 0);
	PreflightPosition();
	SetupPosition();
	StartPosition();
	PostflightPosition();

	for(int i=0; i<DEVICE_NUM; i++)
	{
		if(_deviceDetected[i] == TRUE)
		{
			_deviceDetected[i] = FALSE;
		}
	}
	_deviceDetected[DEVICE_NUM] = FALSE;

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
long ThorLightPathSimulator::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if((*iter)->GetParamID() == paramID)
		{
			paramType = (*iter)->GetParamType();
			paramAvailable = (*iter)->GetParamAvailable();
			paramReadOnly = (*iter)->GetParamReadOnly();
			paramMin = (*iter)->GetParamMin();
			paramMax = (*iter)->GetParamMax();
			paramDefault = (*iter)->GetParamDefault();
			return TRUE;
		}
	}
	paramAvailable = FALSE;
	return FALSE;	
}

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorLightPathSimulator::SetParam(const long paramID, const double param)
{
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if((*iter)->GetParamID() == paramID)
		{
			if(((*iter)->GetParamAvailable() == TRUE) && ((*iter)->GetParamReadOnly() == FALSE))
			{
				if(((*iter)->GetParamMin() <= param) && ((*iter)->GetParamMax() >= param))
				{
					(*iter)->UpdateParam(param);

					return TRUE;
				}
			}
		}
	}

	StringCbPrintfW(message,MSG_SIZE,L"ThorLightPathSimulator SetParam failed. paramID: %d", paramID);
	LogMessage(message);
	return FALSE;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorLightPathSimulator::GetParam(const long paramID, double &param)
{
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if((*iter)->GetParamID() == paramID)
		{
			if(((*iter)->GetParamAvailable() != FALSE))
			{
				switch (paramID)
				{
				case PARAM_LIGHTPATH_GG_SERIALNUMBER:
				case PARAM_LIGHTPATH_GR_SERIALNUMBER:
				case PARAM_LIGHTPATH_CAMERA_SERIALNUMBER:
					{
						return ExecuteCmd(paramID, (*iter)->GetCmdBytes(), param);
					}
					break;
				case PARAM_CONNECTION_STATUS:
					param = (_deviceDetected[DEVICE_NUM]) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
					return TRUE;
				default:
					param = (*iter)->GetParamVal();
					return TRUE;
				}				
			}			
		}
	}

	return FALSE;
}

/// <summary>
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorLightPathSimulator::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorLightPathSimulator::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorLightPathSimulator::SetParamString(const long paramID, wchar_t* str)
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
long ThorLightPathSimulator::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorLightPathSimulator::StartPosition()
{
	//iterate through map and set the parameters in reverse order because we want the position to be set at last
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if(((*iter)->GetParamAvailable() == TRUE) && ((*iter)->GetParamReadOnly() == FALSE) && ((*iter)->GetParamBool() == TRUE))
		{	
			ExecuteCmd(*iter); //no need to parse read back
			(*iter)->UpdateParam_C();
			StringCbPrintfW(message,MSG_SIZE,L"StartPosition succeeded at paramID: %d",(*iter)->GetParamID());
			LogMessage(message);
		}
	}

	return TRUE;
}

/// <summary>
/// Executes the command.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorLightPathSimulator::ExecuteCmd(ParamInfo* pParamInfo)
{
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();
	long paramID = pParamInfo->GetParamID();	
	double readBackVal = -1;

	unsigned char p = (unsigned char)pParamInfo->GetParamVal();
	cmd[2] += p;

	ExecuteCmd(paramID, cmd, readBackVal);
	return TRUE;
}

/// <summary>
/// Executes the command.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="cmd">The command.</param>
/// <param name="readBackValue">The read back value.</param>
/// <returns>long.</returns>
long ThorLightPathSimulator::ExecuteCmd(long paramID, std::vector<unsigned char> cmd, double &readBackValue)
{
	Lock lock(_critSect);
	long ret = TRUE;
	string cmdStr(cmd.begin(),cmd.end());	

	return ret;
}

/// <summary>
/// Builds the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorLightPathSimulator::BuildParamTable()
{
	_tableParams.clear();
	//Execution in order of top to bottom of list:
	std::vector<unsigned char> commandBytes;
	commandBytes.push_back(0x00);
	ParamInfo* tempParamInfo = new ParamInfo(	
		PARAM_DEVICE_TYPE,	//ID
		LIGHT_PATH,			//VAL
		LIGHT_PATH,			//PARAM C
		FALSE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		LIGHT_PATH,			//MIN
		LIGHT_PATH,			//MAX
		LIGHT_PATH,			//DEFAULT
		-1,					//Motor ID
		commandBytes);		//Command
	_tableParams.push_back(tempParamInfo);


	//build table entries for Galvo-Galvo fliper mirror

	unsigned char commandBytesGGTo0[] = { 0x2f, 0x4d, 0x30, 0x52, 0x0d }; //Hex for "/M0R<cr>" -> Move GG Mirror to position 0
	commandBytes.assign(commandBytesGGTo0, commandBytesGGTo0 + sizeof(commandBytesGGTo0)/sizeof(commandBytesGGTo0[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_GG,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		FALSE,				//MIN
		TRUE,				//MAX
		FALSE,				//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entries for Galvo-Resonant fliper mirror
	unsigned char commandBytesGRTo0[] = { 0x2f, 0x4d, 0x30, 0x52, 0x0d }; //Hex for "/M0R<cr>" -> Move GR Mirror to position 0
	commandBytes.assign(commandBytesGRTo0, commandBytesGRTo0 + sizeof(commandBytesGRTo0)/sizeof(commandBytesGRTo0[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_GR,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		FALSE,				//MIN
		TRUE,				//MAX
		FALSE,				//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entries for Camera fliper mirror
	unsigned char commandBytesCamTo0[] = { 0x2f, 0x4d, 0x30, 0x52, 0x0d }; //Hex for "/M0R<cr>" -> Move Camera Mirror to position 0
	commandBytes.assign(commandBytesCamTo0, commandBytesCamTo0 + sizeof(commandBytesCamTo0)/sizeof(commandBytesCamTo0[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_CAMERA,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		FALSE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		FALSE,				//MIN
		TRUE,				//MAX
		FALSE,				//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entry for GG Flipper Mirror Serial Number Request
	unsigned char commandBytesGG_SN[] = { 0x2f, 0x53, 0x52, 0x0d }; // Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesGG_SN, commandBytesGG_SN + sizeof(commandBytesGG_SN)/sizeof(commandBytesGG_SN[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_GG_SERIALNUMBER,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		0,					//MIN
		MAXUINT32,			//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entry for GR Flipper Mirror Serial Number Request
	unsigned char commandBytesGR_SN[] = { 0x2f, 0x53, 0x52, 0x0d }; // Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesGR_SN, commandBytesGR_SN + sizeof(commandBytesGR_SN)/sizeof(commandBytesGR_SN[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_GR_SERIALNUMBER,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		0,					//MIN
		MAXUINT32,			//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	//build table entry for Camera Flipper Mirror Serial Number Request
	unsigned char commandBytesCAM_SN[] = { 0x2f, 0x53, 0x52, 0x0d }; // Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesGR_SN, commandBytesGR_SN + sizeof(commandBytesGR_SN)/sizeof(commandBytesGR_SN[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_CAMERA_SERIALNUMBER,	//ID
		FALSE,				//VAL
		FALSE,				//PARAM C
		TRUE,				//PARAM B
		TYPE_LONG,			//TYPE
		TRUE,				//AVAILABLE
		TRUE,				//READ ONLY
		FALSE,				//CONVERSION (YES/NO)
		FALSE,				//CONVERSION FACTOR
		0,					//MIN
		MAXUINT32,			//MAX
		0,					//DEFAULT
		1,					//Motor ID
		commandBytes);		//COMMAND
	_tableParams.push_back(tempParamInfo);

	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(	
		PARAM_CONNECTION_STATUS,	//ID
		0,							//VAL
		0,							//PARAM C
		FALSE,						//PARAM B
		TYPE_LONG,					//TYPE
		TRUE,						//AVAILABLE
		TRUE,						//READ ONLY
		FALSE,						//CONVERSION (YES/NO)
		FALSE,						//CONVERSION FACTOR
		(double)ConnectionStatusType::CONNECTION_READY,			//MIN
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,		//MAX
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,		//DEFAULT
		-1,							//Motor ID
		commandBytes);				//Command
	_tableParams.push_back(tempParamInfo);

	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorLightPathSimulator::SetupPosition()
{
	for(std::list<ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if(((*iter)->GetParamAvailable() == TRUE) && ((*iter)->GetParamReadOnly() == FALSE))
		{
			if((*iter)->GetParamVal() != (*iter)->GetParamCurrent())
			{
				(*iter)->SetParamBool(TRUE);
			}
			else
			{
				(*iter)->SetParamBool(FALSE);
			}
		}
	}
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorLightPathSimulator::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorLightPathSimulator::StatusPosition(long &status)
{
	status = IDevice::STATUS_READY;
	return TRUE;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorLightPathSimulator::ReadPosition(DeviceType deviceType,double &pos)
{
	return FALSE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorLightPathSimulator::PostflightPosition()
{
	return TRUE;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorLightPathSimulator::PreflightPosition()
{
	return TRUE;
}

