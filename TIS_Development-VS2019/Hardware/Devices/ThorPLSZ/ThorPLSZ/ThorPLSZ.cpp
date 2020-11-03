// ThorPLSZ.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Serial.h"
#include "ThorPLSZ.h"
#include "ThorPLSZXML.h"
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
/// Prevents a default instance of the <see cref="ThorPLSZ"/> class from being created.
/// </summary>
ThorPLSZ::ThorPLSZ()
{
	_numOfAxes = 1;
	_zThreshold = 0.400;
	_motorsParamsInvert = new double[MAX_AXIS_COUNT];
	memset(_motorsParamsInvert,1,MAX_AXIS_COUNT*sizeof(double));
	_blockUpdateParam = FALSE;
	_sleepTimeAfterMoveComplete = 0;
	memset(_motorsParams,0,sizeof(_motorsParams));
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorPLSZ"/> class.
/// </summary>
ThorPLSZ::~ThorPLSZ()
{
	delete[] _motorsParamsInvert;
	_motorsParamsInvert = NULL;
	_instanceFlag = false;
	_errMsg[0] = 0;	
}

bool ThorPLSZ::_instanceFlag = false;

auto_ptr<ThorPLSZ> ThorPLSZ::_single (new ThorPLSZ());

const long ThorPLSZ::MAX_AXIS_COUNT = 3;

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorPLSZ *.</returns>
ThorPLSZ* ThorPLSZ::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorPLSZ());
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
long ThorPLSZ::FindDevices(long &deviceCount)
{
	long ret = TRUE;

	long portID=0;
	long baudRate=0;
	long address=0;

	try
	{
		//Get portID, etc from hardware ThorPLSZSettings.xml
		auto_ptr<ThorPLSZXML> pSetup(new ThorPLSZXML());

		pSetup->GetConnection(portID,baudRate,address);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorPLSZSettings.xml file");
		return FALSE;
	}

	if(FALSE == _serialPort.Open(portID, baudRate))
	{
		StringCbPrintfW(message,MSG_SIZE,L"ThorPLSZ FindDevices could not open serial port");
		LogMessage(message);
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorPLSZ FindDevices could not open serial port or configuration file is not avaible.");

		// *TODO* perform an automatic scan of the available serial ports
		deviceCount = 0;
		ret = FALSE;
	}
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
long ThorPLSZ::SelectDevice(const long device)
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

			bool zInvert = false;
			bool zHome = false;;

			try
			{				
				auto_ptr<ThorPLSZXML> pSetup(new ThorPLSZXML());

				//Get portID, etc from  ThorPLSZSettings.xml
				pSetup->GetConnection(portID,baudRate,address);
				
				//get the min and max and threshold for the XStage, YStage, ZStage from ThorPLSZSettings.xml
				pSetup->GetZRangeConfig(_zMin,_zMax, _zThreshold,zInvert,zHome);

				//get the stage type for each motorAxis from ThorPLSZSettings.xml
				_motorsParams[0].motorAxis = STAGE_Z;

				for (int i = 0; i < MAX_AXIS_COUNT; i++)
				{
					switch (_motorsParams[i].motorAxis)
					{
						case STAGE_Z: _motorsParamsInvert[i] = (zInvert == false)?1:-1; break;
						default:
							break;
					}
				}

				pSetup->GetSleepAfterMove(_sleepTimeAfterMoveComplete);
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorPLSZSettings.xml file");
				return FALSE;
			}

			if(FALSE == _serialPort.Open(portID, baudRate))
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorPLSZ SelectDevice could not open serial port");
				LogMessage(message);
				StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorPLSZ SelectDevice could not open serial port or configuration file is not avaible.");

				// *TODO* perform an automatic scan of the available serial ports
				ret = FALSE;
			}

			GetDeviceParameters(); //Get motor parameters such as conversion factors
			QueryInitialMotorPos(); //Query the Pos of motor for each axis
			SetMotorParameters(); //assign the motor parameters for each axis			
			BuildParamTable();	//build table for parameters
			
			SetParam(IDevice::PARAM_Z_INVERT,zInvert);

			if(zHome)
			{
				SetParam(IDevice::PARAM_Z_HOME,zHome);
				
				ExecutePositionNow();
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

//Set the Motor Parameters for each axis
/// <summary>
/// Sets the motor parameters depending on the Axis-Stage combination (x, y, z ; 0, 1, 2) each motor is on
/// </summary>
/// <returns>long.</returns>
long ThorPLSZ::SetMotorParameters()
{
	switch(_motorsParams[0].motorAxis)
	{
		case STAGE_Z:
			{
				_motorsParams[0].ParamHome = PARAM_Z_HOME;
				_motorsParams[0].ParamPos = PARAM_Z_POS;
				_motorsParams[0].ParamPosCurrent = PARAM_Z_POS_CURRENT;
				_motorsParams[0].ParamStop = PARAM_Z_STOP;
				_motorsParams[0].ParamZero = PARAM_Z_ZERO;
				_motorsParams[0].StagePosMax = _zMax;
				_motorsParams[0].StagePosMin = _zMin;
				_motorsParams[0].ParamInvert = PARAM_Z_INVERT;
			}
		break;
	}

	switch(_motorsParams[1].motorAxis)
	{
		case STAGE_Z:
			{
				_motorsParams[1].ParamHome = PARAM_Z_HOME;
				_motorsParams[1].ParamPos = PARAM_Z_POS;
				_motorsParams[1].ParamPosCurrent = PARAM_Z_POS_CURRENT;
				_motorsParams[1].ParamStop = PARAM_Z_STOP;
				_motorsParams[1].ParamZero = PARAM_Z_ZERO;
				_motorsParams[1].StagePosMax = _zMax;
				_motorsParams[1].StagePosMin = _zMin;
				_motorsParams[1].ParamInvert = PARAM_Z_INVERT;
			}
		break;
	}

	switch(_motorsParams[2].motorAxis)
	{
		case STAGE_Z:
			{
				_motorsParams[2].ParamHome = PARAM_Z_HOME;
				_motorsParams[2].ParamPos = PARAM_Z_POS;
				_motorsParams[2].ParamPosCurrent = PARAM_Z_POS_CURRENT;
				_motorsParams[2].ParamStop = PARAM_Z_STOP;
				_motorsParams[2].ParamZero = PARAM_Z_ZERO;
				_motorsParams[2].StagePosMax = _zMax;
				_motorsParams[2].StagePosMin = _zMin;
				_motorsParams[2].ParamInvert = PARAM_Z_INVERT;
			}
		break;
	}

	return TRUE;
}


/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorPLSZ::LogMessage(wchar_t *message)
{
	#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
	#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorPLSZ::TeardownDevice()
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
long ThorPLSZ::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
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
/// Gets the Devices Parameters.
/// </summary>
/// <returns>long.</returns>
long ThorPLSZ::GetDeviceParameters()
{
	GetMotorType();
	
	return TRUE;
}

/// <summary>
/// Gets the type of the motor for firmware version > 2.0.
/// </summary>
/// <returns>long.</returns>
long ThorPLSZ::GetMotorType()
{
	Lock lock(critSect);
	for(int i=0; i < _numOfAxes; i++)
	{
		char buf[100];

		memset(buf,0,sizeof(buf));
		
		unsigned char cmd[] = { 0x3B, 0x04, static_cast<unsigned char>(i), 0x00, 0x50, 0x01 }; //MGMSG_MOT_GET_PMDSTAGEAXISPARAMS

#pragma warning(push)
#pragma warning(disable:4996)

		string cmdStr(cmd,cmd + sizeof(cmd)/sizeof(cmd[0]));
		size_t len = cmdStr.copy(buf, cmdStr.length(), 0);

#pragma warning(pop)

		_serialPort.SendData((const unsigned char*)buf, static_cast<int>(len));

		Sleep(30);

		memset(buf,0,sizeof(buf));

		_serialPort.ReadData(buf, 100);

		if (0x3C == (unsigned char)buf[0] && 0x04 == (unsigned char)buf[1])
		{
			long typeIndex = 1;//buf[8] | buf[9] << 8;
            _motorsParams[0].motorType = typeIndex;
			_motorsParams[0].MotorEncCoef =  1.0/1.343832;
		}
	}
	return TRUE;
}

/// <summary>
/// Queries the initial motor position.
/// </summary>
/// <returns>long.</returns>
long ThorPLSZ::QueryInitialMotorPos()
{
	Lock lock(critSect);
	for(int i=0; i < _numOfAxes; i++)
	{
		char buf[100];

		memset(buf,0,sizeof(buf));
		unsigned char cmd[] = { 0x90, 0x04, static_cast<unsigned char>(i), 0x00, 0x50, 0x01 };  //Query motor type command

#pragma warning(push)
#pragma warning(disable:4996)

		string cmdStr(cmd,cmd + sizeof(cmd)/sizeof(cmd[0]));
		size_t len = cmdStr.copy(buf, cmdStr.length(), 0);

#pragma warning(pop)

		_serialPort.SendData((const unsigned char*)buf, static_cast<int>(len));

		Sleep(100);

		memset(buf,0,sizeof(buf));

		_serialPort.ReadData(buf, 100);

        _motorsParams[i].initialPos = ((unsigned char)buf[8] | (unsigned char)buf[9] << 8 | (unsigned char)buf[10] << 16 | (unsigned char)buf[11] << 24) * _motorsParamsInvert[i];
	}
	return TRUE;
}


/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorPLSZ::SetParam(const long paramID, const double param)
{
	double value = param;
	if (NULL != _tableParams[paramID])
	{
		if(_tableParams[paramID]->GetParamID() == paramID)
		{
			if(FALSE == (_tableParams[paramID]->GetParamAvailable()) || (TRUE == _tableParams[paramID]->GetParamReadOnly()))
			{
				return FALSE;
			}
			else if((_tableParams[paramID]->GetParamMin() <= value) && (_tableParams[paramID]->GetParamMax() >= value))
			{
				switch (paramID)
			    {
					case PARAM_Z_POS: 
					{
						long zInvert = FALSE;
						if(_tableParams.count(IDevice::PARAM_Z_INVERT)!=0)
						{
							zInvert = static_cast<long>(_tableParams[IDevice::PARAM_Z_INVERT]->GetParamVal());
						}
						value *= (zInvert == FALSE)?1:-1;
					}
					break;
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
				StringCbPrintfW(message,MSG_SIZE,L"ThorPLSZ SetParam failed. paramID: %d", paramID);
				LogMessage(message);
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
long ThorPLSZ::GetParam(const long paramID, double &param)
{
	double ret = -1;
	if (NULL != _tableParams[paramID])
	{
		if(FALSE == _tableParams[paramID]->GetParamAvailable())
		{
			return FALSE;
		}
		else if(paramID == _motorsParams[0].ParamPosCurrent)
		{				
			ExecuteCmd(_tableParams[paramID]->GetCmdBytes(), ret);
			
				//execute and parse the current position from read back
			double convertFactor = _tableParams[paramID]->GetConvertFactor();
			param = ret * convertFactor * _motorsParamsInvert[0];

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
				if (NULL != _tableParams[_motorsParams[0].ParamPos])
				{
					_tableParams[_motorsParams[0].ParamPos]->UpdateParam(param);
					_tableParams[_motorsParams[0].ParamPos]->UpdateParam_C();
				}
			}
		}
		else if(paramID == PARAM_CONNECTION_STATUS)
		{
			param = (_deviceDetected) ? CONNECTION_READY : CONNECTION_UNAVAILABLE;
		}
		else 
		{
			param = static_cast<double>(_tableParams[paramID]->GetParamVal());
		}
		return TRUE;
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
long ThorPLSZ::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPLSZ::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPLSZ::SetParamString(const long paramID, wchar_t* str)
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
long ThorPLSZ::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorPLSZ::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorPLSZ::SetupPosition()
{	
	for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if((FALSE == (iter->second)->GetParamAvailable()) || (TRUE == (iter->second)->GetParamReadOnly()))
		{
			continue;
		}
		else if((iter->second)->GetParamVal() != (iter->second)->GetParamCurrent())
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
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorPLSZ::StartPosition()
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
		else if((FALSE == (iter->second)->GetParamAvailable()) || (TRUE == (iter->second)->GetParamReadOnly()))
		{
			continue;
		}
		else if(TRUE == (iter->second)->GetParamBool()) 
		{	
			if(PARAM_Z_POS == (iter->second)->GetParamID())
			{
				double convertFactor = (iter->second)->GetConvertFactor();
				double zVal = Round((iter->second)->GetParamVal() * convertFactor, 4);
				
				long zInvert = FALSE;
				//verify that the parameter exisits in the table
				//If the user doesn't setup all of the axes the param may
				//not be available
				if(_tableParams.count(IDevice::PARAM_Z_INVERT)!=0)
				{
					zInvert = static_cast<long>(_tableParams[IDevice::PARAM_Z_INVERT]->GetParamVal());
				}

				if (zInvert == TRUE)
				{
					InvertMinMaxRange(_zMin,_zMax);
				}
				if(zVal >= _zMin && zVal <= _zMax)
				{
					double currentZPos;
					GetParam(PARAM_Z_POS_CURRENT,currentZPos);
					currentZPos *= (zInvert == FALSE)?1:-1;
					if (currentZPos > _zMax || currentZPos < _zMin) 
					{
						if (zInvert == TRUE)
						{
							InvertMinMaxRange(_zMin,_zMax);
						}
						continue; // do nothing if command is out of range
					}
					double diff = (zVal - currentZPos)*((zInvert == false)?1:-1);

					//convert the Z position message to micrometers
					double diffUM = diff * 1000;

					wchar_t stageMessage[512];
					StringCbPrintfW(stageMessage,512,L"Large Z stage move (%d.%03d um). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the Z stage.\nPower cycling the Controller Stage may resolve the issue.",static_cast<long>(diffUM),static_cast<long>(abs(static_cast<long>((diffUM-static_cast<long>(diffUM)) * 1000))));
					if((diff > _zThreshold)||(diff < (_zThreshold*-1.0)))
					{
						if(IDNO == MessageBox(NULL,stageMessage,L"",MB_YESNO))
						{
							double param_c = (iter->second)->GetParamCurrent() * convertFactor; // convert back to mm before setting parameter
							SetParam(PARAM_Z_POS, param_c);
							(iter->second)->SetParamBool(FALSE);
							if (zInvert == TRUE)
							{
								InvertMinMaxRange(_zMin,_zMax);
							}
							continue;
						}
					}
				}
				if (zInvert == TRUE)
				{
					InvertMinMaxRange(_zMin,_zMax);
				}
			}
	
			ExecuteCmd(iter->second); //no need to parse read back

			if (TYPE_BOOL == (iter->second)->GetParamType())
			{	(iter->second)->ResetVal();	}

			(iter->second)->UpdateParam_C();

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
/// Status of the positioning.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorPLSZ::StatusPosition(long &status)
{
	long ret = TRUE;
	
	Lock lock(critSect);

	status = IDevice::STATUS_READY;

	char statusByte[4];
	memset(statusByte, 0, sizeof(statusByte));

	for(int i=0; i < _numOfAxes; i++)
	{
		char buf[100];

		memset(buf,0,sizeof(buf));
		
		unsigned char cmd[] = { 0x90, 0x04, static_cast<unsigned char>(i), 0x00, 0x50, 0x01 }; //MGMSG_MOT_REQ_STATUSUPDATE 

#pragma warning(push)
#pragma warning(disable:4996)

		string cmdStr(cmd,cmd + sizeof(cmd)/sizeof(cmd[0]));
		size_t len = cmdStr.copy(buf, cmdStr.length(), 0);

#pragma warning(pop)

		_serialPort.SendData((const unsigned char*)buf, static_cast<int>(len));

		Sleep(30); //Determined by testing... Shorter sleep time may cause bad communications

		memset(buf,0,sizeof(buf));

		_serialPort.ReadData(buf, 100);

		//if the motor is connected (motorType is not 0:no motor) but stage is moving
		if ((0 != _motorsParams[i].motorType) && (0x30 & (unsigned char)buf[16]))
		{
			status = IDevice::STATUS_BUSY;
		}
	}

	if (IDevice::STATUS_READY == status && _sleepTimeAfterMoveComplete > 0)
	{
		Sleep(min(_sleepTimeAfterMoveComplete, MAX_SLEEPTIME));
	}

	return ret;
}

/// <summary>
/// DEPRECATED
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorPLSZ::ReadPosition(DeviceType deviceType,double &pos)
{
	return FALSE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorPLSZ::PostflightPosition()
{
	return TRUE;
}


/// <summary>
/// Executes the command.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorPLSZ::ExecuteCmd(ParamInfo* pParamInfo)
{

	long paramID = pParamInfo->GetParamID();
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();	

	//if the command is set zero, then devide the cmdbytes into to commands
	//The first one to set the counter counts to zero and the second one to
	//set the position counts to zero
	if (paramID == PARAM_Z_ZERO)		
	{

		std::vector<unsigned char> cmd1(cmd.begin(), cmd.end() - 12);

		std::vector<unsigned char> cmd2(cmd.begin() + 12, cmd.end());

		double readBackVal = -1;

		ExecuteCmd(cmd1, readBackVal);

		ExecuteCmd(cmd2, readBackVal);
	}
	else if (paramID == PARAM_Z_POS)
	{
		std::vector<unsigned char> paramValBytes = GetBytes((int)Round(pParamInfo->GetParamVal(),0));
		cmd[8] =  paramValBytes[0];
		cmd[9] =  paramValBytes[1];
		cmd[10] =  paramValBytes[2];
		cmd[11] =  paramValBytes[3];
		double readBackVal = -1;
		ExecuteCmd(cmd, readBackVal);
	}
	else
	{		
		double readBackVal = -1;
		ExecuteCmd(cmd, readBackVal);
	}
	
	return TRUE;
}


/// <summary>
/// Executes the command.
/// </summary>
/// <param name="cmd">The command.</param>
/// <param name="readBackValue">The read back value.</param>
/// <returns>long.</returns>
long ThorPLSZ::ExecuteCmd(std::vector<unsigned char> cmd, double &readBackValue)
{
	Lock lock(critSect);

	char buf[100];

	memset(buf, 0, sizeof(buf));
#pragma warning(push)
#pragma warning(disable:4996)


	string cmdStr(cmd.begin(),cmd.end());
	size_t len = cmdStr.copy(buf, cmdStr.length(), 0);
#pragma warning(pop)
	_serialPort.SendData((const unsigned char*)(buf), static_cast<int>(len));

	Sleep(60); //Determined by testing... Shorter sleep time may cause bad communications
	
	memset(buf, 0, sizeof(buf));

	 _serialPort.ReadData(buf, 100);
	
	 readBackValue = ((unsigned char)buf[8] | (unsigned char)buf[9] << 8 | (unsigned char)buf[10] << 16 | (unsigned char)buf[11] << 24);
	 
	return TRUE;
}

/// <summary>
/// Executes the position now.
/// </summary>
void ThorPLSZ::ExecutePositionNow()
{
	PreflightPosition();
	SetupPosition();
	StartPosition();
	long status = IDevice::STATUS_BUSY;
	do
	{
		if (FALSE == StatusPosition(status))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorPLSZ::statusPosition failed");
			LogMessage(message);
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
long ThorPLSZ::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}

/// <summary>
/// Rounds up to the specified decimal place value.
/// </summary>
/// <param name="val">The value.</param>
/// <param name="decPlace">The decimal place.</param>
/// <returns>double.</returns>
double ThorPLSZ::rndup(double val,int decPlace)
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
/// Gets the bytes.
/// </summary>
/// <param name="value">The value.</param>
/// <returns>std.vector&lt;unsigned char&gt;.</returns>
std::vector<unsigned char> ThorPLSZ::GetBytes(int value)
{
    std::vector<unsigned char> bytes(sizeof(int));
    std::memcpy(&bytes[0], &value, sizeof(int));
    return bytes;
}

/// <summary>
/// Rounds to the specified number.
/// </summary>
/// <param name="number">The number.</param>
/// <param name="decimals">The decimals.</param>
/// <returns>double.</returns>
double ThorPLSZ::Round(double number, int decimals)
{
	double decP = std::pow(10,decimals);
	double ret;
	if (number < 0.0)
		ret = -std::floor(-number * decP + 0.5) / decP;
	else
		ret = std::floor(number * decP + 0.5) / decP;
	return ret;
}


/// <summary>
/// Resets the stage zero position.
/// </summary>
/// <param name="stageID">The stage identifier.</param>
/// <returns>long.</returns>
long ThorPLSZ::ResetStageZeroPosition(long stageID)
{
	int ret = TRUE;
	double currentPos;
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
		case 2:			//Z stage, Set to zero based on ZRangeConfig from xml.
		{			
			if (FALSE == GetParam(PARAM_Z_POS_CURRENT,currentPos))
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorPLSZ::ResetStageZeroPosition GetParam of PARAM_Z_POS_CURRENT failed");
				LogMessage(message);
			}

			if ((currentPos < _zMin) || (currentPos > _zMax))
			{
				if (FALSE == SetParam(PARAM_Z_ZERO,static_cast<double>(TRUE)))
				{
					StringCbPrintfW(message,MSG_SIZE,L"ThorPLSZ::ResetStageZeroPosition SetParam of PARAM_Z_ZERO failed");
					LogMessage(message);
				}
				ExecutePositionNow();
			}
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
//long ThorPLSZ::CheckMotorConnection()
//{
//	for(int i=3; i<=_numOfAxes; i++)
//	{
//	}
//	return TRUE;
//}

/// <summary>
/// Builds the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorPLSZ::BuildParamTable()
{
	_tableParams.clear();
	//Execution in order of top to bottom of list:
	std::vector<unsigned char> commandBytes;
	commandBytes.push_back(0x00);
	ParamInfo* tempParamInfo = new ParamInfo(	
					PARAM_DEVICE_TYPE,						//ID
					 STAGE_Z | STAGE_Z2,	//VAL
					 STAGE_Z | STAGE_Z2,	//PARAM C
					FALSE,									//PARAM B
					TYPE_LONG,								//TYPE
					TRUE,									//AVAILABLE
					TRUE,									//READ ONLY
					FALSE,									//CONVERSION (YES/NO)
					FALSE,									//CONVERSION FACTOR
					STAGE_Z | STAGE_Z2,	//MIN
					 STAGE_Z | STAGE_Z2,	//MAX
					 STAGE_Z | STAGE_Z2,	//DEFAULT
					-1,										//Motor ID
					commandBytes);							//Command
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_DEVICE_TYPE,tempParamInfo));
		
	commandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(	
					PARAM_CONNECTION_STATUS,						//ID
					 CONNECTION_READY,	//VAL
					 CONNECTION_READY,	//PARAM C
					FALSE,									//PARAM B
					TYPE_LONG,								//TYPE
					TRUE,									//AVAILABLE
					TRUE,									//READ ONLY
					FALSE,									//CONVERSION (YES/NO)
					FALSE,									//CONVERSION FACTOR
					CONNECTION_WARMING_UP,	//MIN
					CONNECTION_ERROR_STATE,	//MAX
					 CONNECTION_READY,	//DEFAULT
					-1,										//Motor ID
					commandBytes);							//Command
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTION_STATUS,tempParamInfo));

	unsigned char commandBytesA0Home[] = { 0x43, 0x04, 0x01, 0x00, 0x50, 0x01}; 
	commandBytes.assign(commandBytesA0Home, commandBytesA0Home + sizeof(commandBytesA0Home)/sizeof(commandBytesA0Home[0]));
	tempParamInfo = new ParamInfo(	
					_motorsParams[0].ParamHome,
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
					0,
					commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[0].ParamHome,tempParamInfo));

	unsigned char commandBytesA0Pos[] = { 0x53, 0x04, 0x06, 0x00, 0xD0 , 0x01, 0x00, 0x00, 0x00, 0x00, 0x50, 0x01 }; //MGMSG_MOT_MOVE_ABSOLUTE Axis 0 (Stage 1)
	commandBytes.assign(commandBytesA0Pos, commandBytesA0Pos + sizeof(commandBytesA0Pos)/sizeof(commandBytesA0Pos[0]));
	tempParamInfo = new ParamInfo(
					_motorsParams[0].ParamPos,
					_motorsParams[0].initialPos,
					_motorsParams[0].initialPos,
					FALSE,
					TYPE_DOUBLE,
					TRUE,
					FALSE,
					TRUE,
					_motorsParams[0].MotorEncCoef/1000000.0, // Convert to mm
					_motorsParams[0].StagePosMin,
					_motorsParams[0].StagePosMax,
					STAGE_POSITION_DEFAULT,
					0,
					commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[0].ParamPos,tempParamInfo));

	unsigned char commandBytesA0PosCurrent[] = { 0x90, 0x04, 0x00, 0x00, 0x50, 0x01 }; //MGMSG_MOT_REQ_ENCCOUNTER Axis 0 (Stage 1)
	commandBytes.assign(commandBytesA0PosCurrent, commandBytesA0PosCurrent + sizeof(commandBytesA0PosCurrent)/sizeof(commandBytesA0PosCurrent[0]));
	tempParamInfo = new ParamInfo(
					_motorsParams[0].ParamPosCurrent,
					STAGE_POSITION_DEFAULT,
					STAGE_POSITION_DEFAULT,
					FALSE,
					TYPE_DOUBLE,
					TRUE,
					TRUE,
					FALSE,
					_motorsParams[0].MotorEncCoef/1000000.0, // Convert to mm
					_motorsParams[0].StagePosMin,
					_motorsParams[0].StagePosMax,
					STAGE_POSITION_DEFAULT,
					0,
					commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[0].ParamPosCurrent,tempParamInfo));

	unsigned char commandBytesA0Stop[] = { 0x65, 0x04, 0x00, 0x01, 0x50, 0x01 }; //MGMSG_MOT_MOVE_STOP  Axis 0 (Stage 1)
	commandBytes.assign(commandBytesA0Stop, commandBytesA0Stop + sizeof(commandBytesA0Stop)/sizeof(commandBytesA0Stop[0]));
	tempParamInfo = new ParamInfo(	
					_motorsParams[0].ParamStop,
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
					0,
					commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[0].ParamStop,tempParamInfo));
	
	std::vector<unsigned char> commandBytesInvert;
	commandBytesInvert.push_back(0x00);
	tempParamInfo = new ParamInfo(	
		_motorsParams[0].ParamInvert,
					FALSE,
					FALSE,
					TRUE,
					TYPE_LONG,
					TRUE,
					FALSE,
					FALSE,
					FALSE,
					FALSE,
					TRUE,
					FALSE,
					0,
					commandBytesInvert);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[0].ParamInvert,tempParamInfo));

	return TRUE;
}
/// <summary>
/// Swap max and min value
/// </summary>
/// <param name="min">The min value.</param>
/// <param name="max">The max value.</param>
/// <returns>void.</returns>
void ThorPLSZ::InvertMinMaxRange(double& min, double& max)
{
	double temp = min;
	min = -1 * max;
	max = -1 * temp;
}