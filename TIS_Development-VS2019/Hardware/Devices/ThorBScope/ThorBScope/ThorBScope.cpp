#include "stdafx.h"
#include "Serial.h"
#include "ThorBScope.h"
#include "ThorBScopeXML.h"
#include "Strsafe.h"
#include "ParamInfo.h"
#include <string>
#include <math.h>

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

/// <summary>
/// The message
/// </summary>
wchar_t message[256];


/// <summary>
/// Prevents a default instance of the <see cref="ThorBScope"/> class from being created.
/// </summary>
ThorBScope::ThorBScope()
{
	_numOfAxes = 4;
	_xThreshold = 0.400;
	_yThreshold = 0.400;
	_zThreshold = 0.400;
	_motorsParamsInvert = new double[3];
	memset(_motorsParamsInvert,1,3*sizeof(double));
	_blockUpdateParam = FALSE;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorBScope"/> class.
/// </summary>
ThorBScope::~ThorBScope()
{
	delete[] _motorsParamsInvert;
	_motorsParamsInvert = NULL;
	_instanceFlag = false;
	_errMsg[0] = 0;
}

bool ThorBScope::_instanceFlag = false;

auto_ptr<ThorBScope> ThorBScope::_single (new ThorBScope());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorBScope *.</returns>
ThorBScope* ThorBScope::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorBScope());
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
long ThorBScope::FindDevices(long &deviceCount)
{
	long ret = TRUE;

	long portID=0;
	long baudRate=0;
	long address=0;

	try
	{
		//Get portID, etc from hardware setup.xml
		auto_ptr<ThorBScopeXML> pSetup(new ThorBScopeXML());

		pSetup->GetConnection(portID,baudRate,address);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorBScopeSettings.xml file");
		return FALSE;
	}

	if(FALSE == _serialPort.Open(portID, baudRate))
	{
		StringCbPrintfW(message,MSG_SIZE,L"ThorBScope FindDevices could not open serial port");
		LogMessage(message);
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBScope FindDevices could not open serial port or configuration file is not avaible.");

		// *TODO* perform an automatic scan of the available serial ports
		deviceCount = 0;
		_deviceDetected = FALSE;
		ret = FALSE;
	}
	//else if(FALSE == CheckMotorConnection())
	//{
	//	StringCbPrintfW(message,MSG_SIZE,L"ThorBScope CheckMotorConnection motor connection error");
	//	LogMessage(message);
	//	StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBScope FindDevices one or more motors are not connected properly.");

	//	deviceCount = 0;
	//	ret = FALSE;
	//}
	else
	{
		deviceCount = 1;
		_deviceDetected = TRUE;
		_serialPort.Close();

	}

	return ret;
}


/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorBScope::SelectDevice(const long device)
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
			long portID=0;
			long baudRate=0;
			long address=0;

			bool   xInvert = false;
			bool   yInvert = false;
			bool   zInvert = false;

			try
			{
				//Get portID, etc from hardware setup.xml
				auto_ptr<ThorBScopeXML> pSetup(new ThorBScopeXML());

				pSetup->GetConnection(portID,baudRate,address);
				//get the min and max and threshold for the XStage, YStage, ZStage from ThorMCM3000Settings.xml
				pSetup->GetXRangeConfig(_xMin,_xMax, _xThreshold,xInvert);
				pSetup->GetYRangeConfig(_yMin,_yMax, _yThreshold,yInvert);
				pSetup->GetZRangeConfig(_zMin,_zMax, _zThreshold,zInvert);
				pSetup->GetRRangeConfig(_rMin,_rMax);
				_motorsParamsInvert[0] = (xInvert == FALSE)?1:-1;
				_motorsParamsInvert[1] = (yInvert == FALSE)?1:-1;
				_motorsParamsInvert[2] = (zInvert == FALSE)?1:-1;
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorBScopeSettings.xml file");
				return FALSE;
			}

			if(FALSE == _serialPort.Open(portID, baudRate))
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorBScope SelectDevice could not open serial port");
				LogMessage(message);
				StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBScope SelectDevice could not open serial port or configuration file is not avaible.");

				// *TODO* perform an automatic scan of the available serial ports
				ret = FALSE;
			}
			QueryInitialMotorPos();
			BuildParamTable();	//build table for parameters

			//Reset stages to zero if position is out of range: currently z only.
			//Also reset Camera Light Path to be 1.
			for (int id=0;id<5;id++)
			{
				if(FALSE == ResetStageZeroPosition(id))
				{
					StringCbPrintfW(message,MSG_SIZE,L"ThorBScope ResetStage %d failed",id);
					LogMessage(message);

					StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBScope ResetStage %d failed",id);
				}
			}

			SetParam(IDevice::PARAM_X_INVERT,xInvert);
			SetParam(IDevice::PARAM_Y_INVERT,yInvert);
			SetParam(IDevice::PARAM_Z_INVERT,zInvert);

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

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorBScope::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorBScope::TeardownDevice()
{	
	//Set LightPaths to their default position before clearing the table of parameters
	SetParam(PARAM_LIGHTPATH_GG, 0);
	SetParam(PARAM_LIGHTPATH_GR, 0);
	SetParam(PARAM_LIGHTPATH_CAMERA, 0);
	PreflightPosition();
	SetupPosition();
	StartPosition();
	PostflightPosition();
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
long ThorBScope::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	if (NULL != _tableParams[paramID])
	{
		if(_tableParams[paramID]->GetParamID() == paramID)
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

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorBScope::SetParam(const long paramID, const double param)
{
	double value = param;
	if (NULL != _tableParams[paramID])
	{
		if(_tableParams[paramID]->GetParamID() == paramID)
		{
			if((FALSE == _tableParams[paramID]->GetParamAvailable()) || (TRUE == _tableParams[paramID]->GetParamReadOnly()))
			{
				return FALSE;
			}
			else if((_tableParams[paramID]->GetParamMin() <= value) && (_tableParams[paramID]->GetParamMax() >= value))
			{
				switch (paramID)
				{
				case PARAM_X_POS: value *= (_tableParams[IDevice::PARAM_X_INVERT]->GetParamVal() == FALSE)?1:-1;break;
				case PARAM_Y_POS: value *= (_tableParams[IDevice::PARAM_Y_INVERT]->GetParamVal() == FALSE)?1:-1;break;
				case PARAM_Z_POS: value *= (_tableParams[IDevice::PARAM_Z_INVERT]->GetParamVal() == FALSE)?1:-1;break;
				default:
					break;
				}
				_tableParams[paramID]->UpdateParam(value);

				//Use _blockUpdateParam to to block updating any paremeter in the getParam funtion until after the new position
				//has been sent to the device. This variable allows GetParam calls in between SetParam and StartPosition, without
				//changing the new set position, until the command has been sent to the device. This device must be set to FALSE
				//at the end of StartPosition to allow updating the parameters when reading positions from the device. 
				_blockUpdateParam = TRUE;
				return TRUE;
			}
			else
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorBScope SetParam failed. paramID: %d", paramID);
				LogMessage(message);
				StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBScope SetParam failed. paramID: %d", paramID);
				return FALSE;
			}
		}
	}

	return FALSE;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorBScope::GetParam(const long paramID, double &param)
{
	long ret = -1;
	if (NULL != _tableParams[paramID])
	{
		if(_tableParams[paramID]->GetParamID() == paramID)
		{
			if((_tableParams[paramID]->GetParamAvailable() == FALSE))
			{
				return FALSE;
			}
			else if(PARAM_X_POS_CURRENT == paramID) //|| (PARAM_Y_POS_CURRENT == paramID) || (PARAM_Z_POS_CURRENT == paramID) || (PARAM_R_POS_CURRENT == paramID))
			{				
				if (FALSE == ExecuteCmdStr(_tableParams[paramID]->GetCmdStr(), ret)) //execute and parse the current position from read back
				{
					return FALSE;
				}
				param = static_cast<double>(ret);
				param = param/STAGE_X_POSITION_READFCR/100 * _motorsParamsInvert[0];		//unit: mm
				//param = rndup(param*10/STAGE_X_POSITION_READFCR,2);

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
					if (NULL != _tableParams[PARAM_X_POS])
					{
						_tableParams[PARAM_X_POS]->UpdateParam(param);
						_tableParams[PARAM_X_POS]->UpdateParam_C();
					}
				}
			}			
			else if (PARAM_Y_POS_CURRENT == paramID)
			{
				if (FALSE == ExecuteCmdStr(_tableParams[paramID]->GetCmdStr(), ret)) //execute and parse the current position from read back
				{
					return FALSE;
				}
				param = static_cast<double>(ret);
				param = param/STAGE_Y_POSITION_READFCR/100* _motorsParamsInvert[1];		//unit: mm
				//param = rndup(param*10/STAGE_Y_POSITION_READFCR,2);

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
					if (NULL != _tableParams[PARAM_Y_POS])
					{
						_tableParams[PARAM_Y_POS]->UpdateParam(param);
						_tableParams[PARAM_Y_POS]->UpdateParam_C();
					}
				}
			}
			else if (PARAM_Z_POS_CURRENT == paramID)
			{
				if (FALSE == ExecuteCmdStr(_tableParams[paramID]->GetCmdStr(), ret)) //execute and parse the current position from read back
				{
					return FALSE;
				}
				param = static_cast<double>(ret);
				param = param/STAGE_Z_POSITION_READFCR/100* _motorsParamsInvert[2];		//unit: mm
				//param = rndup(param*10/STAGE_Z_POSITION_READFCR,2);

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
					if (NULL != _tableParams[PARAM_Z_POS])
					{
						_tableParams[PARAM_Z_POS]->UpdateParam(param);
						_tableParams[PARAM_Z_POS]->UpdateParam_C();
					}
				}
			}
			else if (PARAM_R_POS_CURRENT == paramID)
			{
				if (FALSE == ExecuteCmdStr(_tableParams[paramID]->GetCmdStr(), ret)) //execute and parse the current position from read back
				{
					return FALSE;
				}
				param = static_cast<double>(ret);
				//param = param*10/STAGE_R_POSITION_READFCR;
				param = rndup(param*10/STAGE_R_POSITION_READFCR,2);		//unit: degree
			}
			else if((PARAM_X_VELOCITY_CURRENT == paramID)|| (PARAM_Y_VELOCITY_CURRENT == paramID) || (PARAM_Z_VELOCITY_CURRENT == paramID) || (PARAM_R_VELOCITY_CURRENT == paramID))
			{				
				if (FALSE == ExecuteCmdStr(_tableParams[paramID]->GetCmdStr(), ret)) //execute and parse the current position from read back
				{
					return FALSE;
				}
				param = static_cast<double>(ret) / 1000.0;
			}
			else if(PARAM_CONNECTION_STATUS == paramID)
			{
				param = (_deviceDetected) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
				return TRUE;
			}
			else 
			{
				param = static_cast<double>(_tableParams[paramID]->GetParamVal());
			}
			return TRUE;
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
long ThorBScope::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorBScope::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorBScope::SetParamString(const long paramID, wchar_t* str)
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
long ThorBScope::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorBScope::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorBScope::SetupPosition()
{
	for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if(((iter->second)->GetParamAvailable() == FALSE) || ((iter->second)->GetParamReadOnly() == TRUE))
		{
			continue;
		}
		// We special case the PMT/CAM mirror because it is inverted. If the MCM5000 is turned off
		//ThorImageLS has no way to know the TTL is down, so we should move this mirror regardless
		else if((iter->second)->GetParamVal() != (iter->second)->GetParamCurrent() || PARAM_LIGHTPATH_CAMERA == (iter->second)->GetParamID())
		{
			(iter->second)->SetParamBool(TRUE);
		}
		else
		{
			(iter->second)->SetParamBool(FALSE);
		}		
	}

	return TRUE;
}

/// <summary>
/// Queries the initial motor position.
/// </summary>
/// <returns>long.</returns>
long ThorBScope::QueryInitialMotorPos()
{
	Lock lock(_critSect);

	char buf[100];
	string cmd;

	for (int i = 0; i < _numOfAxes; i++)
	{
		if (i == 0)
			cmd = "/1?8R\r";
		else if(i == 1)
			cmd = "/2?8R\r";
		else if(i == 2)
			cmd = "/3?8R\r";
		else
			cmd = "/4?8R\r";

		memset(buf, 0, sizeof(buf));
#pragma warning(push)
#pragma warning(disable:4996)
		size_t len = cmd.copy(buf, cmd.length(), 0);
#pragma warning(pop)
		_serialPort.SendData((const unsigned char*)(buf), static_cast<int>(len));

		Sleep(100);

		memset(buf, 0, sizeof(buf));

		_serialPort.ReadData(buf, 100);

		string str = buf;
		//sscanf_s(buf, "%c/0%c%d%s", &temp, &temp, &readBackValue, junk);	
		int foundi = static_cast<int>(str.find("/0"));
		int founde = static_cast<int>(str.find("R"));
		string str2 = str.substr(foundi+2,(founde-foundi-2));
		long readval = atoi(str2.c_str());
		_stageStartingPos[i] = static_cast<double>(readval);
		if (i == 0)
			_stageStartingPos[i] = _stageStartingPos[i]/STAGE_X_POSITION_READFCR/100*STAGE_X_POSITION_SENDFCR/10 * _motorsParamsInvert[i];
		else if(i == 1)
			_stageStartingPos[i] = _stageStartingPos[i]/STAGE_Y_POSITION_READFCR/100*STAGE_Y_POSITION_SENDFCR/10 * _motorsParamsInvert[i];
		else if(i == 2)
			_stageStartingPos[i] = _stageStartingPos[i]/STAGE_Z_POSITION_READFCR/100*STAGE_Z_POSITION_SENDFCR/10 * _motorsParamsInvert[i];
		else
			_stageStartingPos[i] = _stageStartingPos[i]/STAGE_R_POSITION_READFCR/100*STAGE_R_POSITION_SENDFCR/10;

	}

	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorBScope::StartPosition()
{
	long ret = FALSE;

	//Use _blockUpdateParam to to block updating any paremeter in the getParam funtion until after the new position
	//has been sent to the device. This variable allows GetParam calls in between SetParam and StartPosition, without
	//changing the new set position, until the command has been sent to the device. This device must be set to FALSE
	//at the end of StartPosition to allow updating the parameters when reading positions from the device. 
	_blockUpdateParam = TRUE;

	//iterate through map and set the parameters in reverse order because we want the position to be set at last
	for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if(((iter->second)->GetParamAvailable() == FALSE) || ((iter->second)->GetParamReadOnly() == TRUE))
		{
			continue;
		}
		else if((iter->second)->GetParamBool() == TRUE) 
		{	
			if((iter->second)->GetParamID() == PARAM_X_POS)
			{
				double xVal = (iter->second)->GetParamVal()/STAGE_X_POSITION_SENDFCR * 10;
				if (_tableParams[IDevice::PARAM_X_INVERT]->GetParamVal() == TRUE)
				{
					InvertMinMaxRange(_xMin,_xMax);
				}
				if(xVal >= _xMin && xVal <= _xMax)
				{					
					double currentXPos;
					if (FALSE == GetParam(PARAM_X_POS_CURRENT,currentXPos))
					{
						if (_tableParams[IDevice::PARAM_X_INVERT]->GetParamVal() == TRUE)
						{
							InvertMinMaxRange(_xMin,_xMax);
						}
						continue;
					}
					currentXPos *= (_tableParams[IDevice::PARAM_X_INVERT]->GetParamVal() == FALSE)?1:-1;
					if (currentXPos > _xMax || currentXPos < _xMin) 
					{
						if (_tableParams[IDevice::PARAM_X_INVERT]->GetParamVal() == TRUE)
						{
							InvertMinMaxRange(_xMin,_xMax);
						}
						continue; // do nothing if command is out of range
					}
					double diff = (xVal - currentXPos)*((_tableParams[IDevice::PARAM_X_INVERT]->GetParamVal() == FALSE)?1:-1);
					wchar_t stageMessage[512];
					StringCbPrintfW(stageMessage,512,L"Large X stage move (%d.%03d mm). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the X stage.\nPower cycling the Controller Stage may resolve the issue.",static_cast<long>(diff),static_cast<long>(abs((diff-static_cast<long>(diff)) * 1000)));
					if((diff > _xThreshold)||(diff < (_xThreshold*-1.0)))
					{
						if(IDNO == MessageBox(NULL,stageMessage,L"",MB_YESNO))
						{
							double param_c = (iter->second)->GetParamCurrent() / STAGE_X_POSITION_SENDFCR * 10; // convert back to mm before setting parameter
							SetParam(PARAM_X_POS, (iter->second)->GetParamCurrent());
							(iter->second)->SetParamBool(FALSE);
							if (_tableParams[IDevice::PARAM_X_INVERT]->GetParamVal() == TRUE)
							{
								InvertMinMaxRange(_xMin,_xMax);
							}
							continue;
						}
					}
				}
				if (_tableParams[IDevice::PARAM_X_INVERT]->GetParamVal() == TRUE)// invert back
				{
					InvertMinMaxRange(_xMin,_xMax);
				}
			}
			else if((iter->second)->GetParamID() == PARAM_Y_POS)
			{
				double yVal = (iter->second)->GetParamVal()/STAGE_Y_POSITION_SENDFCR * 10;
				if (_tableParams[IDevice::PARAM_Y_INVERT]->GetParamVal() == TRUE)
				{
					InvertMinMaxRange(_yMin,_yMax);
				}
				if(yVal >= _yMin && yVal <= _yMax)
				{					
					double currentYPos;
					if (FALSE == GetParam(PARAM_Y_POS_CURRENT,currentYPos))
					{
						if (_tableParams[IDevice::PARAM_Y_INVERT]->GetParamVal() == TRUE)
						{
							InvertMinMaxRange(_yMin,_yMax);
						}
						continue;
					}
					currentYPos *= (_tableParams[IDevice::PARAM_Y_INVERT]->GetParamVal() == FALSE)?1:-1;
					if (currentYPos > _yMax || currentYPos < _yMin) 
					{
						if (_tableParams[IDevice::PARAM_Y_INVERT]->GetParamVal() == TRUE)
						{
							InvertMinMaxRange(_yMin,_yMax);
						}
						continue; // do nothing if command is out of range
					}
					double diff = (yVal - currentYPos)*((_tableParams[IDevice::PARAM_Y_INVERT]->GetParamVal() == FALSE)?1:-1);
					wchar_t stageMessage[512];
					StringCbPrintfW(stageMessage,512,L"Large Y stage move (%d.%03d mm). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the Y stage.\nPower cycling the Controller Stage may resolve the issue.",static_cast<long>(diff),static_cast<long>(abs((diff-static_cast<long>(diff)) * 1000)));
					if((diff > _yThreshold)||(diff < (_yThreshold*-1.0)))
					{
						if(IDNO == MessageBox(NULL,stageMessage,L"",MB_YESNO))
						{
							double param_c = (iter->second)->GetParamCurrent() / STAGE_Y_POSITION_SENDFCR * 10; // convert back to mm before setting parameter
							SetParam(PARAM_Y_POS, param_c);
							(iter->second)->SetParamBool(FALSE);
							if (_tableParams[IDevice::PARAM_Y_INVERT]->GetParamVal() == TRUE)
							{
								InvertMinMaxRange(_yMin,_yMax);
							}
							continue;
						}
					}
				}
				if (_tableParams[IDevice::PARAM_Y_INVERT]->GetParamVal() == TRUE)
				{
					InvertMinMaxRange(_yMin,_yMax);
				}
			}
			else if((iter->second)->GetParamID() == PARAM_Z_POS)
			{
				double zVal = (iter->second)->GetParamVal() / STAGE_Z_POSITION_SENDFCR * 10;
				if (_tableParams[IDevice::PARAM_Z_INVERT]->GetParamVal() == TRUE)
				{
					InvertMinMaxRange(_zMin,_zMax);
				}
				if(zVal >= _zMin && zVal <= _zMax)
				{					
					double currentZPos;
					if (FALSE == GetParam(PARAM_Z_POS_CURRENT,currentZPos))
					{
						if (_tableParams[IDevice::PARAM_Z_INVERT]->GetParamVal() == TRUE)
						{
							InvertMinMaxRange(_zMin,_zMax);
						}
						continue;
					}
					currentZPos *= (_tableParams[IDevice::PARAM_Z_INVERT]->GetParamVal() == FALSE)?1:-1;
					if (currentZPos > _zMax || currentZPos < _zMin) 
					{
						if (_tableParams[IDevice::PARAM_Z_INVERT]->GetParamVal() == TRUE)
						{
							InvertMinMaxRange(_zMin,_zMax);
						}
						continue; // do nothing if command is out of range
					}

					double diff = (zVal - currentZPos)*((_tableParams[IDevice::PARAM_Z_INVERT]->GetParamVal() == FALSE)?1:-1);

					wchar_t stageMessage[512];
					StringCbPrintfW(stageMessage,512,L"Large Z stage move (%d.%03d um). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the Z stage.\nPower cycling the Controller Stage may resolve the issue.",static_cast<long>(diff*1000),static_cast<long>(abs((diff-static_cast<long>(diff)) * 1000000)));
					if((diff > _zThreshold)||(diff < (_zThreshold*-1.0)))
					{
						if(IDNO == MessageBox(NULL,stageMessage,L"",MB_YESNO))
						{
							double param_c = (iter->second)->GetParamCurrent() / STAGE_Z_POSITION_SENDFCR * 10; // convert back to mm before setting parameter
							SetParam(PARAM_Z_POS, param_c);
							(iter->second)->SetParamBool(FALSE);
							if (_tableParams[IDevice::PARAM_Z_INVERT]->GetParamVal() == TRUE)
							{
								InvertMinMaxRange(_zMin,_zMax);
							}
							continue;
						}
					}
				}
				if (_tableParams[IDevice::PARAM_Z_INVERT]->GetParamVal() == TRUE)
				{
					InvertMinMaxRange(_zMin,_zMax);
				}
			}
			else if((iter->second)->GetParamID() == PARAM_R_POS)
			{
				double rVal = (iter->second)->GetParamVal()/STAGE_R_POSITION_SENDFCR * 10;
				if(rVal >= _rMin && rVal <= _rMax)
				{					
					double currentRPos;
					if (FALSE == GetParam(PARAM_R_POS_CURRENT,currentRPos))
					{
						continue;
					}
					if (currentRPos > _rMax || currentRPos < _rMin) 
					{
						continue; // do nothing if command is out of range
					}
				}
			}
			else if ((iter->second)->GetParamID() == PARAM_X_ZERO)
			{
				SetParam(PARAM_X_STOP,TRUE);				
				ExecutePositionNow(PARAM_X_STOP);
			}
			else if ((iter->second)->GetParamID() == PARAM_Y_ZERO)
			{
				SetParam(PARAM_Y_STOP,TRUE);
				ExecutePositionNow(PARAM_Y_STOP);
			}
			else if ((iter->second)->GetParamID() == PARAM_Z_ZERO)
			{
				SetParam(PARAM_Z_STOP,TRUE);				
				ExecutePositionNow(PARAM_Z_STOP);
			}
			else if ((iter->second)->GetParamID() == PARAM_R_ZERO)
			{
				SetParam(PARAM_R_STOP,TRUE);				
				ExecutePositionNow(PARAM_R_STOP);
			}

			ExecuteCmdStr(iter->second); //no need to parse read back
			if ((iter->second)->GetParamType() == TYPE_BOOL)
			{	(iter->second)->ResetVal();	}
			else
			{	(iter->second)->UpdateParam_C();	}

			ret = TRUE;
			(iter->second)->SetParamBool(FALSE);

			StringCbPrintfW(message,MSG_SIZE,L"StartPosition succeeded at paramID: %d",(iter->second)->GetParamID());
			LogMessage(message);
		}
	}

	_blockUpdateParam = FALSE; //After all commands have been sent to the device, allow the params to be updated when read from the device
	return ret;
}

/// <summary>
/// Starts the position single command.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <returns>long.</returns>
long ThorBScope::StartPositionSingleCommand(const long paramID)
{
	long ret = FALSE;

	if (NULL != _tableParams[paramID])
	{
		if(FALSE == (_tableParams[paramID]->GetParamAvailable()) || (TRUE == _tableParams[paramID]->GetParamReadOnly()))
		{
			return FALSE;
		}
		else if(TRUE == _tableParams[paramID]->GetParamBool()) 
		{	
			if (_tableParams[paramID]->GetParamID() == paramID)
			{
				ExecuteCmdStr(_tableParams[paramID]); //no need to parse read back
				if (_tableParams[paramID]->GetParamType() == TYPE_BOOL)
				{	_tableParams[paramID]->ResetVal();	}
				else
				{	_tableParams[paramID]->UpdateParam_C();	}

				ret = TRUE;
				_tableParams[paramID]->SetParamBool(FALSE);

				StringCbPrintfW(message,MSG_SIZE,L"StartPosition succeeded at paramID: %d",_tableParams[paramID]->GetParamID());
				LogMessage(message);
			}
		}
	}

	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorBScope::StatusPosition(long &status)
{
	long ret = TRUE;

	Lock lock(_critSect);

	status = IDevice::STATUS_READY;

	char statusByte[4];
	memset(statusByte, 0, sizeof(statusByte));

	for(int i=0; i < _numOfAxes; i++)
	{
		char sendBuf[100];

		memset(sendBuf,0,sizeof(sendBuf));

		string cmd = "/%dQR\r";
		sprintf_s((char*)sendBuf, cmd.length(), cmd.c_str(), i+1);

		size_t len = strlen((const char*)sendBuf);
		_serialPort.SendData((const unsigned char*)sendBuf, static_cast<int>(len));

		Sleep(50);//Determined by testing... Shorter sleep time may cause bad communications

		memset(sendBuf,0,sizeof(sendBuf));

		_serialPort.ReadData(sendBuf, 100);

		string str = string(sendBuf);
		int found = static_cast<int>(str.find("/0"));	//find the first match for "/0"

		if((0 <= found) && (static_cast<int>(str.length()) > found+2))
		{ 
			statusByte[i] = str.at(found+2);	//get the status byte after "/0"
		}

		if((statusByte[i]=='f')||(statusByte[i]=='F'))		//((statusByte[i] & 0x0F) > 0) // error state occurs: flag
		{
			status = IDevice::STATUS_ERROR;
			//return ret;
		}
		if((statusByte[i]=='b')||(statusByte[i]=='B'))		//((statusByte[i] & 0x20) == 0)
		{
			status = IDevice::STATUS_BUSY;
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
long ThorBScope::ReadPosition(DeviceType deviceType,double &pos)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorBScope::PostflightPosition()
{
	return TRUE;
}


/// <summary>
/// Executes the command string.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorBScope::ExecuteCmdStr(ParamInfo* pParamInfo)
{

	string str = pParamInfo->GetCmdStr();
	ostringstream converter;
	converter << floor(pParamInfo->GetParamVal());
	string valStr = converter.str();

	int found = static_cast<int>(str.find("%d"));
	if(found > 0)
	{
		str.replace(found, 2, valStr);
	}

	long readBackVal = -1;
	ExecuteCmdStr(str, readBackVal);

	return TRUE;
}

/// <summary>
/// Executes the command string.
/// </summary>
/// <param name="cmd">The command.</param>
/// <param name="readBackValue">The read back value.</param>
/// <returns>long.</returns>
long ThorBScope::ExecuteCmdStr(string cmd, long &readBackValue)
{
	Lock lock(_critSect);

	char buf[100];

	memset(buf, 0, sizeof(buf));
#pragma warning(push)
#pragma warning(disable:4996)
	size_t len = cmd.copy(buf, cmd.length(), 0);
#pragma warning(pop)
	_serialPort.SendData((const unsigned char*)(buf), static_cast<int>(len));

	Sleep(50); //Determined by testing... Shorter sleep time may cause bad communications

	memset(buf, 0, sizeof(buf));

	_serialPort.ReadData(buf, 100);

	string str = buf;	

	if (0 < static_cast<int>(str.find("LLR"))) return FALSE; // Return FALSE if stage is at the Lower Limit
	if (0 < static_cast<int>(str.find("ULR"))) return FALSE; // Return FALSE if stage is at the Upper Limit
	int foundi = static_cast<int>(str.find("/0"));
	int founde = static_cast<int>(str.find("R"));
	if ((foundi!=-1)||(founde!=-1))
	{
		string str2 = str.substr(foundi+2,(founde-foundi-2));
		if (str2.compare("ok")!=0)
		{readBackValue = atoi(str2.c_str());}
	}
	else 
	{	readBackValue=-1;	}
	return TRUE;
}

/// <summary>
/// Executes the position now.
/// </summary>
void ThorBScope::ExecutePositionNow()
{
	PreflightPosition();
	SetupPosition();
	StartPosition();
	long status = IDevice::STATUS_BUSY;
	do
	{
		if (FALSE == StatusPosition(status))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorBScope::statusPosition failed");
			LogMessage(message);

			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBScope::statusPosition failed");
			break;
		}
	}while(status != IDevice::STATUS_READY);

	PostflightPosition();	
}

/// <summary>
/// Executes the position now.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
void ThorBScope::ExecutePositionNow(const long paramID) // use for single command
{
	PreflightPosition();
	SetupPosition();
	StartPositionSingleCommand(paramID);
	long status = IDevice::STATUS_BUSY;
	do
	{
		if (FALSE == StatusPosition(status))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorBScope::statusPosition failed");
			LogMessage(message);

			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBScope::statusPosition failed");
			break;
		}
	}while(status != IDevice::STATUS_READY);

	PostflightPosition();	
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorBScope::GetLastErrorMsg(wchar_t *msg, long size)
{
	wcsncpy_s(msg,size,_errMsg,MSG_SIZE);
	return TRUE;
}

/// <summary>
/// Rndups the specified value.
/// </summary>
/// <param name="val">The value.</param>
/// <param name="decPlace">The decimal place.</param>
/// <returns>double.</returns>
double ThorBScope::rndup(double val,int decPlace)
{
	val *= pow(10.0,decPlace);
	int istack = (int)floor(val);
	double delta = val-istack;
	if (delta < 0.5)
	{	val = floor(val);	}
	if (delta > 0.4) 
	{	val = ceil(val);	}
	val /= pow(10.0,decPlace);
	return val;
}

/// <summary>
/// Resets the stage zero position.
/// </summary>
/// <param name="stageID">The stage identifier.</param>
/// <returns>long.</returns>
long ThorBScope::ResetStageZeroPosition(long stageID)
{
	int ret = TRUE;
	switch(stageID)
	{
	case 0:			//X stage,
		{
		}
		break;
	case 1:			//Y stage, 
		{			
		}
		break;
	case 2:			//Z stage
		{			
		}
		break;
	case 3:			//R Stage,
		{
		}
		break;
	default:
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"The stage index %d is invalid", stageID);
			ret = FALSE;
		}
	}
	return ret;
}
///CheckMotorConnection() is currently not working
///movement may have to be performed in order to know the connection
//long ThorBScope::CheckMotorConnection()
//{
//	for(int i=3; i<=_numOfAxes; i++)
//	{
//		char rxBuf[100];
//		char buffer[20];
//
//		//stop in case any command is already executing
//		memset(buffer,0,sizeof(buffer));
//		memset(rxBuf,0,sizeof(rxBuf));
//		sprintf_s(buffer, sizeof(buffer), "/%dTR\r", i);
//		string cmd = string(buffer);
//		ExecuteCmdStr(cmd,rxBuf);
//
//		memset(buffer,0,sizeof(buffer));
//		memset(rxBuf,0,sizeof(rxBuf));
//		sprintf_s(buffer, sizeof(buffer), "/%d?0R\r", i);
//		cmd = string(buffer);
//		ExecuteCmdStr(cmd,rxBuf);
//
//		char temp=' '; long val=0;
//		char junk[10];
//		sscanf(rxBuf, "%c/0%c%d%s", &temp, &temp, &val, junk);
//
//		sprintf_s(buffer, sizeof(buffer), "/%dA%dR", i, val);
//		cmd=string(buffer);
//		ExecuteCmdStr(cmd,rxBuf);
//
//		memset(buffer,0,sizeof(buffer));
//		memset(rxBuf,0,sizeof(rxBuf));
//		sprintf_s(buffer, sizeof(buffer), "/%dQR\r", i);
//		cmd = string(buffer);
//		ExecuteCmdStr(cmd,rxBuf);
//
//		string str = string(rxBuf);
//		int found = str.find("/0");
//
//		str = string(rxBuf);
//		found = str.find("/0");	//find the first match for "/0"
//
//		char statusByte;
//		if((0 <= found) && (str.length() > found+2))
//		{ 
//			statusByte = str.at(found+2);	//get the status byte after "/0"
//		}
//
//		if((statusByte & 0x20) == 0) // status busy
//		{
//			StringCbPrintfW(message,MSG_SIZE,L"ThorBScope CheckMotorConnection motor connection error for Axes: %d.", i);
//			LogMessage(message);
//			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorBScope FindDevices Motor #%d is not connected properly.", i);
//			return FALSE;
//		}
//	}
//	return TRUE;
//}

/// <summary>
/// Builds the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorBScope::BuildParamTable()
{
	_tableParams.clear();

	//Execution in order of top to bottom of list:
	ParamInfo* tempParamInfo = new ParamInfo(	
		PARAM_DEVICE_TYPE,						//ID
		STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2 | STAGE_R | LIGHT_PATH,	//VAL
		STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2 | STAGE_R | LIGHT_PATH,	//PARAM C
		FALSE,									//PARAM B
		TYPE_LONG,								//TYPE
		TRUE,									//AVAILABLE
		TRUE,									//READ ONLY
		FALSE,									//CONVERSION
		FALSE,									//CONVERSION FACTOR
		STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2 | STAGE_R | LIGHT_PATH,	//MIN
		STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2 | STAGE_R | LIGHT_PATH,	//MAX
		STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2 | STAGE_R | LIGHT_PATH,	//DEFAULT
		"");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_DEVICE_TYPE,tempParamInfo));

	//build table entries for StageX parameters
	tempParamInfo = new ParamInfo(	
		PARAM_X_ZERO,
		FALSE,
		FALSE,
		FALSE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/1z%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_X_ZERO,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_X_VELOCITY,
		STAGE_X_VELOCITY_DEFAULT,
		STAGE_X_VELOCITY_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_VELACCEL_SEND,
		STAGE_VELOCITY_FCR,
		STAGE_VELOCITY_MIN,
		STAGE_VELOCITY_MAX,
		STAGE_X_VELOCITY_DEFAULT,
		"/1V%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_X_VELOCITY,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_X_ACCEL,
		STAGE_X_ACCEL_DEFAULT,
		STAGE_X_ACCEL_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_VELACCEL_SEND,
		STAGE_ACCEL_FCR,
		STAGE_ACCEL_MIN,
		STAGE_ACCEL_MAX,
		STAGE_X_ACCEL_DEFAULT,
		"/1L%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_X_ACCEL,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_X_HOME,
		STAGE_HOME_DEFAULT,
		STAGE_HOME_DEFAULT,
		FALSE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/1Z%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_X_HOME,tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_X_POS,
		_stageStartingPos[0],
		_stageStartingPos[0],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_POSITION_SEND,
		STAGE_X_POSITION_SENDFCR,
		_xMin,
		_xMax,
		STAGE_POSITION_DEFAULT,
		"/1A%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_X_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_X_POS_CURRENT,
		STAGE_POSITION_DEFAULT,
		STAGE_POSITION_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		FALSE,
		FALSE,
		STAGE_POSITION_MIN,
		STAGE_POSITION_MAX,
		STAGE_POSITION_DEFAULT,
		"/1?8R\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_X_POS_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_X_STOP,
		FALSE,
		FALSE,
		TRUE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/1T%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_X_STOP,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_X_VELOCITY_CURRENT,
		STAGE_X_VELOCITY_DEFAULT,
		STAGE_X_VELOCITY_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		FALSE,
		FALSE,
		STAGE_VELOCITY_MIN,
		STAGE_VELOCITY_MAX,
		STAGE_X_VELOCITY_DEFAULT,
		"/1?VR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_X_VELOCITY_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_X_POSITIVE,
		STAGE_POSITION_DEFAULT,
		STAGE_POSITION_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_POSITION_SEND,
		STAGE_X_POSITION_SENDFCR,
		STAGE_POSITION_MIN,
		STAGE_POSITION_MAX,
		STAGE_POSITION_DEFAULT,
		"/1P%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_X_POSITIVE,tempParamInfo));

	//build table entries for StageY parameters
	tempParamInfo = new ParamInfo(	
		PARAM_Y_ZERO,
		FALSE,
		FALSE,
		FALSE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/2z%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Y_ZERO,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Y_VELOCITY,
		STAGE_Y_VELOCITY_DEFAULT,
		STAGE_Y_VELOCITY_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_VELACCEL_SEND,
		STAGE_VELOCITY_FCR,
		STAGE_VELOCITY_MIN,
		STAGE_VELOCITY_MAX,
		STAGE_Y_VELOCITY_DEFAULT,
		"/2V%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Y_VELOCITY,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Y_ACCEL,
		STAGE_Y_ACCEL_DEFAULT,
		STAGE_Y_ACCEL_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_VELACCEL_SEND,
		STAGE_ACCEL_FCR,
		STAGE_ACCEL_MIN,
		STAGE_ACCEL_MAX,
		STAGE_Y_ACCEL_DEFAULT,
		"/2L%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Y_ACCEL,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Y_HOME,
		STAGE_HOME_DEFAULT,
		STAGE_HOME_DEFAULT,
		FALSE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/2Z%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Y_HOME,tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_Y_POS,
		_stageStartingPos[1],
		_stageStartingPos[1],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_POSITION_SEND,
		STAGE_Y_POSITION_SENDFCR,
		_yMin,
		_yMax,
		STAGE_POSITION_DEFAULT,
		"/2A%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Y_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_Y_POS_CURRENT,
		STAGE_POSITION_DEFAULT,
		STAGE_POSITION_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		FALSE,
		FALSE,
		STAGE_POSITION_MIN,
		STAGE_POSITION_MAX,
		STAGE_POSITION_DEFAULT,
		"/2?8R\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Y_POS_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Y_STOP,
		FALSE,
		FALSE,
		TRUE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/2T%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Y_STOP,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Y_VELOCITY_CURRENT,
		STAGE_Y_VELOCITY_DEFAULT,
		STAGE_Y_VELOCITY_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		FALSE,
		FALSE,
		STAGE_VELOCITY_MIN,
		STAGE_VELOCITY_MAX,
		STAGE_Y_VELOCITY_DEFAULT,
		"/2?VR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Y_VELOCITY_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Y_POSITIVE,
		STAGE_POSITION_DEFAULT,
		STAGE_POSITION_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_POSITION_SEND,
		STAGE_Y_POSITION_SENDFCR,
		STAGE_POSITION_MIN,
		STAGE_POSITION_MAX,
		STAGE_POSITION_DEFAULT,
		"/2P%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Y_POSITIVE,tempParamInfo));

	//build table entries for StageZ parameters
	tempParamInfo = new ParamInfo(	
		PARAM_Z_ZERO,
		FALSE,
		FALSE,
		FALSE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/3z%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Z_ZERO,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Z_VELOCITY,
		STAGE_Z_VELOCITY_DEFAULT,
		STAGE_Z_VELOCITY_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_VELACCEL_SEND,
		STAGE_VELOCITY_FCR,
		STAGE_VELOCITY_MIN,
		STAGE_VELOCITY_MAX,
		STAGE_Z_VELOCITY_DEFAULT,
		"/3V%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Z_VELOCITY,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Z_ACCEL,
		STAGE_Z_ACCEL_DEFAULT,
		STAGE_Z_ACCEL_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_VELACCEL_SEND,
		STAGE_ACCEL_FCR,
		STAGE_ACCEL_MIN,
		STAGE_ACCEL_MAX,
		STAGE_Z_ACCEL_DEFAULT,
		"/3L%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Z_ACCEL,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Z_HOME,
		STAGE_HOME_DEFAULT,
		STAGE_HOME_DEFAULT,
		FALSE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/3Z%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Z_HOME,tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_Z_POS,
		_stageStartingPos[2],
		_stageStartingPos[2],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_POSITION_SEND,
		STAGE_Z_POSITION_SENDFCR,
		_zMin,
		_zMax,
		STAGE_POSITION_DEFAULT,
		"/3A%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Z_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_Z_POS_CURRENT,
		STAGE_POSITION_DEFAULT,
		STAGE_POSITION_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		FALSE,
		FALSE,
		STAGE_POSITION_MIN,
		STAGE_POSITION_MAX,
		STAGE_POSITION_DEFAULT,
		"/3?8R\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Z_POS_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Z_STOP,
		FALSE,
		FALSE,
		TRUE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/3T%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Z_STOP,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Z_VELOCITY_CURRENT,
		STAGE_Z_VELOCITY_DEFAULT,
		STAGE_Z_VELOCITY_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		FALSE,
		FALSE,
		STAGE_VELOCITY_MIN,
		STAGE_VELOCITY_MAX,
		STAGE_Z_VELOCITY_DEFAULT,
		"/3?VR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Z_VELOCITY_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Z_POSITIVE,
		STAGE_POSITION_DEFAULT,
		STAGE_POSITION_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_POSITION_SEND,
		STAGE_Z_POSITION_SENDFCR,
		STAGE_POSITION_MIN,
		STAGE_POSITION_MAX,
		STAGE_POSITION_DEFAULT,
		"/3P%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Z_POSITIVE,tempParamInfo));

	//build table entries for StageR parameters
	tempParamInfo = new ParamInfo(	
		PARAM_R_ZERO,
		FALSE,
		FALSE,
		FALSE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/4z%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_R_ZERO,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_R_VELOCITY,
		STAGE_R_VELOCITY_DEFAULT,
		STAGE_R_VELOCITY_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_VELACCEL_SEND,
		STAGE_VELOCITY_FCR,
		STAGE_VELOCITY_MIN,
		STAGE_VELOCITY_MAX,
		STAGE_R_VELOCITY_DEFAULT,
		"/4V%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_R_VELOCITY,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_R_ACCEL,
		STAGE_R_ACCEL_DEFAULT,
		STAGE_R_ACCEL_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_VELACCEL_SEND,
		STAGE_ACCEL_FCR,
		STAGE_ACCEL_MIN,
		STAGE_ACCEL_MAX,
		STAGE_R_ACCEL_DEFAULT,
		"/4L%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_R_ACCEL,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_R_HOME,
		STAGE_HOME_DEFAULT,
		STAGE_HOME_DEFAULT,
		FALSE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		STAGER_POSITION_MIN,
		STAGER_POSITION_MAX,
		STAGE_HOME_DEFAULT,
		"/4Z%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_R_HOME,tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_R_POS,
		_stageStartingPos[3],
		_stageStartingPos[3],
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		CONVERT_POSITION_SEND,
		STAGE_R_POSITION_SENDFCR,
		_rMin,
		_rMax,
		STAGE_POSITION_DEFAULT,
		"/4A%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_R_POS,tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_R_POS_CURRENT,
		STAGE_POSITION_DEFAULT,
		STAGE_POSITION_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		FALSE,
		FALSE,
		STAGER_POSITION_MIN,
		STAGER_POSITION_MAX,
		STAGE_POSITION_DEFAULT,
		"/4?8R\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_R_POS_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_R_STOP,
		FALSE,
		FALSE,
		TRUE,
		TYPE_BOOL,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/4T%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_R_STOP,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_R_VELOCITY_CURRENT,
		STAGE_R_VELOCITY_DEFAULT,
		STAGE_R_VELOCITY_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		FALSE,
		FALSE,
		STAGE_VELOCITY_MIN,
		STAGE_VELOCITY_MAX,
		STAGE_R_VELOCITY_DEFAULT,
		"/4?VR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_R_VELOCITY_CURRENT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_GG,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/1MG%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LIGHTPATH_GG,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_GR,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/1MR%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LIGHTPATH_GR,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_LIGHTPATH_CAMERA,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"/1ML%dR\r/1M%dR\r");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_LIGHTPATH_CAMERA,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_X_INVERT,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_X_INVERT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Y_INVERT,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Y_INVERT,tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_Z_INVERT,
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		"");
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Z_INVERT, tempParamInfo));

	tempParamInfo = new ParamInfo(	
		PARAM_CONNECTION_STATUS,			//ID
		FALSE,					//VAL
		FALSE,					//Param C
		FALSE,					//Param B
		TYPE_LONG,				//Type
		TRUE,					//Available
		TRUE,					//Read only
		FALSE,					//Conversion (Yes/No)
		FALSE,					//Conversion factor
		(double)ConnectionStatusType::CONNECTION_READY,		//Min
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,	//Max
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,	//Default
		"");					//Command
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTION_STATUS, tempParamInfo));

	return TRUE;
}

/// <summary>
/// Swap max and min value
/// </summary>
/// <param name="min">The min value.</param>
/// <param name="max">The max value.</param>
/// <returns>void.</returns>
void ThorBScope::InvertMinMaxRange(double& min, double& max)
{
	double temp = min;
	min = -1 * max;
	max = -1 * temp;
}