// ThorMCM3000.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Serial.h"
#include "ThorMCM3000.h"
#include "ThorMCM3000XML.h"
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

//MOTOR_COUNTER_COEFF const is used to convert the
//counts to nm depending on the type of motor.
//Motor Types: //no_stage, pls_stage, PMP_stage, bergamo_x, bergamo_y, bergamo_z, CSA1200, vbe_stage, vbe_shutter
const double ThorMCM3000::MOTOR_COUNTER_COEFF[9] = { 0, 211.66666, 39.0625, 500, 500, 100.0, 5555.555, 211.66666, 11.90625};

//MOTOR_COUNTER_COEFF_OLDFIRMWARE (use this for firmware version < 2) const is used to convert the
//counts to nm depending on the type of motor.
//Motor Types: No Motor, LNR, PLS, AScope Z, BScope, BScope Z, Objetive Mover (1 Count == MOTOR_COUNTER_COEFF[i]) ..... old
//const double ThorMCM3000::MOTOR_COUNTER_COEFF_OLDFIRMWARE[7] = { 0.0, 39.0625, 211.6667, 1.0, 500.0, 100.0, 1.0};


//Motor Types: No Motor, LNR, PLS, AScope Z, BScope, BScope Z, Bergamo Z High Torque (1 Count == MOTOR_COUNTER_COEFF[i]) ..... new
const double ThorMCM3000::MOTOR_COUNTER_COEFF_OLDFIRMWARE[7] = { 0.0, 39.0625, 211.6667, 1.0, 500.0, 100.0, 100.0};

/// <summary>
/// Prevents a default instance of the <see cref="ThorMCM3000"/> class from being created.
/// </summary>
ThorMCM3000::ThorMCM3000()
{
	_numOfAxes = 3;
	_xThreshold = 0.400;
	_yThreshold = 0.400;
	_zThreshold = 0.400;
	_motorsParamsInvert = new double[3];
	memset(_motorsParamsInvert,1,3*sizeof(double));
	_blockUpdateParam = FALSE;
	_sleepTimeAfterMoveComplete = 0;
	memset(_motorsParams,0,sizeof(_motorsParams));
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorMCM3000"/> class.
/// </summary>
ThorMCM3000::~ThorMCM3000()
{
	delete[] _motorsParamsInvert;
	_motorsParamsInvert = NULL;
	_instanceFlag = false;
	_errMsg[0] = 0;	
}

bool ThorMCM3000::_instanceFlag = false;

auto_ptr<ThorMCM3000> ThorMCM3000::_single (new ThorMCM3000());

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorMCM3000 *.</returns>
ThorMCM3000* ThorMCM3000::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorMCM3000());
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
long ThorMCM3000::FindDevices(long &deviceCount)
{
	long ret = TRUE;

	long portID=0;
	long baudRate=0;
	long address=0;

	try
	{
		//Get portID, etc from hardware ThorMCM3000Settings.xml
		auto_ptr<ThorMCM3000XML> pSetup(new ThorMCM3000XML());

		pSetup->GetConnection(portID,baudRate,address);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorMCM3000Settings.xml file");
		return FALSE;
	}

	if(FALSE == _serialPort.Open(portID, baudRate))
	{
		StringCbPrintfW(message,MSG_SIZE,L"ThorMCM3000 FindDevices could not open serial port");
		LogMessage(message);
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorMCM3000 FindDevices could not open serial port or configuration file is not avaible.");

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
long ThorMCM3000::SelectDevice(const long device)
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

			bool xInvert = false;
			bool yInvert = false;
			bool zInvert = false;

			try
			{				
				auto_ptr<ThorMCM3000XML> pSetup(new ThorMCM3000XML());

				//Get portID, etc from  ThorMCM3000Settings.xml
				pSetup->GetConnection(portID,baudRate,address);

				//get the min and max and threshold for the XStage, YStage, ZStage from ThorMCM3000Settings.xml
				pSetup->GetXRangeConfig(_xMin,_xMax, _xThreshold,xInvert);
				pSetup->GetYRangeConfig(_yMin,_yMax, _yThreshold,yInvert);
				pSetup->GetZRangeConfig(_zMin,_zMax, _zThreshold,zInvert);

				//get the stage type for each motorAxis from ThorMCM3000Settings.xml
				pSetup->GetXYZStageAxes(_motorsParams[0].motorAxis, _motorsParams[1].motorAxis, _motorsParams[2].motorAxis); 
				for (int i = 0; i < 3; i++)
				{
					switch (_motorsParams[i].motorAxis)
					{
					case STAGE_X: _motorsParamsInvert[i] = (xInvert == false)?1:-1; break;
					case STAGE_Y: _motorsParamsInvert[i] = (yInvert == false)?1:-1; break;
					case STAGE_Z: _motorsParamsInvert[i] = (zInvert == false)?1:-1; break;
					default:
						break;
					}
				}

				pSetup->GetSleepAfterMove(_sleepTimeAfterMoveComplete);
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorMCM3000Settings.xml file");
				return FALSE;
			}

			if(FALSE == _serialPort.Open(portID, baudRate))
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorMCM3000 SelectDevice could not open serial port");
				LogMessage(message);
				StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorMCM3000 SelectDevice could not open serial port or configuration file is not avaible.");

				// *TODO* perform an automatic scan of the available serial ports
				ret = FALSE;
			}

			GetDeviceParameters(); //Get motor parameters such as conversion factors
			QueryInitialMotorPos(); //Query the Pos of motor for each axis
			SetMotorParameters(); //assign the motor parameters for each axis			
			BuildParamTable();	//build table for parameters

			SetParam(IDevice::PARAM_X_MOTOR_TYPE,_motorsParams[0].motorType);
			SetParam(IDevice::PARAM_Y_MOTOR_TYPE,_motorsParams[1].motorType);
			SetParam(IDevice::PARAM_Z_MOTOR_TYPE,_motorsParams[2].motorType);

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

//Set the Motor Parameters for each axis
/// <summary>
/// Sets the motor parameters depending on the Axis-Stage combination (x, y, z ; 0, 1, 2) each motor is on
/// </summary>
/// <returns>long.</returns>
long ThorMCM3000::SetMotorParameters()
{
	switch(_motorsParams[0].motorAxis)
	{
	case STAGE_X:
		{
			_motorsParams[0].ParamHome = PARAM_X_HOME;
			_motorsParams[0].ParamPos = PARAM_X_POS;
			_motorsParams[0].ParamPosCurrent = PARAM_X_POS_CURRENT;
			_motorsParams[0].ParamStop = PARAM_X_STOP;
			_motorsParams[0].ParamZero = PARAM_X_ZERO;
			_motorsParams[0].StagePosMax = _xMax;
			_motorsParams[0].StagePosMin = _xMin;
			_motorsParams[0].ParamInvert = PARAM_X_INVERT;
		}
		break;
	case STAGE_Y:
		{
			_motorsParams[0].ParamHome = PARAM_Y_HOME;
			_motorsParams[0].ParamPos = PARAM_Y_POS;
			_motorsParams[0].ParamPosCurrent = PARAM_Y_POS_CURRENT;
			_motorsParams[0].ParamStop = PARAM_Y_STOP;
			_motorsParams[0].ParamZero = PARAM_Y_ZERO;
			_motorsParams[0].StagePosMax = _yMax;
			_motorsParams[0].StagePosMin = _yMin;
			_motorsParams[0].ParamInvert = PARAM_Y_INVERT;
		}
		break;
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
	case STAGE_X:
		{
			_motorsParams[1].ParamHome = PARAM_X_HOME;
			_motorsParams[1].ParamPos = PARAM_X_POS;
			_motorsParams[1].ParamPosCurrent = PARAM_X_POS_CURRENT;
			_motorsParams[1].ParamStop = PARAM_X_STOP;
			_motorsParams[1].ParamZero = PARAM_X_ZERO;
			_motorsParams[1].StagePosMax = _xMax;
			_motorsParams[1].StagePosMin = _xMin;
			_motorsParams[1].ParamInvert = PARAM_X_INVERT;
		}
		break;
	case STAGE_Y:
		{
			_motorsParams[1].ParamHome = PARAM_Y_HOME;
			_motorsParams[1].ParamPos = PARAM_Y_POS;
			_motorsParams[1].ParamPosCurrent = PARAM_Y_POS_CURRENT;
			_motorsParams[1].ParamStop = PARAM_Y_STOP;
			_motorsParams[1].ParamZero = PARAM_Y_ZERO;
			_motorsParams[1].StagePosMax = _yMax;
			_motorsParams[1].StagePosMin = _yMin;;
			_motorsParams[1].ParamInvert = PARAM_Y_INVERT;
		}
		break;
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
	case STAGE_X:
		{
			_motorsParams[2].ParamHome = PARAM_X_HOME;
			_motorsParams[2].ParamPos = PARAM_X_POS;
			_motorsParams[2].ParamPosCurrent = PARAM_X_POS_CURRENT;
			_motorsParams[2].ParamStop = PARAM_X_STOP;
			_motorsParams[2].ParamZero = PARAM_X_ZERO;
			_motorsParams[2].StagePosMax = _xMax;
			_motorsParams[2].StagePosMin = _xMin;
			_motorsParams[2].ParamInvert = PARAM_X_INVERT;
		}
		break;
	case STAGE_Y:
		{
			_motorsParams[2].ParamHome = PARAM_Y_HOME;
			_motorsParams[2].ParamPos = PARAM_Y_POS;
			_motorsParams[2].ParamPosCurrent = PARAM_Y_POS_CURRENT;
			_motorsParams[2].ParamStop = PARAM_Y_STOP;
			_motorsParams[2].ParamZero = PARAM_Y_ZERO;
			_motorsParams[2].StagePosMax = _yMax;
			_motorsParams[2].StagePosMin = _yMin;
			_motorsParams[2].ParamInvert = PARAM_Y_INVERT;
		}
		break;
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
void ThorMCM3000::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorMCM3000::TeardownDevice()
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
long ThorMCM3000::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
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
long ThorMCM3000::GetDeviceParameters()
{
	double firmwareVersion = 0.0;
	GetFirmwareVersion(firmwareVersion);
	if (2 <= firmwareVersion)
	{
		GetMotorType();
	}
	else
	{
		QueryMotorTypeOldFirmware();
	}
	return TRUE;
}

/// <summary>
/// Gets the type of the motor for firmware version > 2.0.
/// </summary>
/// <returns>long.</returns>
long ThorMCM3000::GetMotorType()
{
	Lock lock(critSect);
	for(int i=0; i < _numOfAxes; i++)
	{
		char buf[100];

		memset(buf,0,sizeof(buf));

		unsigned char cmd[] = { 0xF1, 0x04, static_cast<unsigned char>(i), 0x00, 0x00, 0x00 }; //MGMSG_MOT_GET_PMDSTAGEAXISPARAMS

#pragma warning(push)
#pragma warning(disable:4996)

		string cmdStr(cmd,cmd + sizeof(cmd)/sizeof(cmd[0]));
		size_t len = cmdStr.copy(buf, cmdStr.length(), 0);

#pragma warning(pop)

		_serialPort.SendData((const unsigned char*)buf, static_cast<int>(len));

		Sleep(30);

		memset(buf,0,sizeof(buf));

		_serialPort.ReadData(buf, 100);

		if (0xF2 == (unsigned char)buf[0] && 0x04 == (unsigned char)buf[1])
		{
			long typeIndex = buf[8] | buf[9] << 8;
			if (0x00 == (unsigned char)buf[6])
			{
				_motorsParams[0].motorType = typeIndex;
				_motorsParams[0].MotorEncCoef = MOTOR_COUNTER_COEFF[typeIndex];
			}
			else if (0x01 == (unsigned char)buf[6])
			{
				_motorsParams[1].motorType = typeIndex;
				_motorsParams[1].MotorEncCoef = MOTOR_COUNTER_COEFF[typeIndex];
			}
			else if (0x02 == (unsigned char)buf[6])
			{
				_motorsParams[2].motorType = typeIndex;
				_motorsParams[2].MotorEncCoef = MOTOR_COUNTER_COEFF[typeIndex];
			}			
		}
	}
	return TRUE;
}

/// <summary>
/// Gets the device firmware version
/// </summary>
/// <returns>long.</returns>
long ThorMCM3000::GetFirmwareVersion(double &firmwareversion)
{
	Lock lock(critSect);
	unsigned char cmd[] = { 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 }; //Query motor type command
	char buf[100];
#pragma warning(push)
#pragma warning(disable:4996)

	string cmdStr(cmd,cmd + sizeof(cmd)/sizeof(cmd[0]));
	size_t len = cmdStr.copy(buf, cmdStr.length(), 0);

#pragma warning(pop)
	_serialPort.SendData((const unsigned char*)buf, static_cast<int>(len));
	Sleep(30);
	memset(buf,0,sizeof(buf));
	_serialPort.ReadData(buf, 100);
	firmwareversion = static_cast<long>(buf[22]);
	return TRUE;
}

/// <summary>
/// Queries the type of the motor for firmware version < 2.0.
/// </summary>
/// <returns>long.</returns>
long ThorMCM3000::QueryMotorTypeOldFirmware()
{
	Lock lock(critSect);
	for(int i=0; i < _numOfAxes; i++)
	{
		char buf[100];

		memset(buf,0,sizeof(buf));

		unsigned char cmd[] = { 0x3B, 0x04, static_cast<unsigned char>(i), 0x00, 0x00, 0x00 }; //Query motor type command

#pragma warning(push)
#pragma warning(disable:4996)

		string cmdStr(cmd,cmd + sizeof(cmd)/sizeof(cmd[0]));
		size_t len = cmdStr.copy(buf, cmdStr.length(), 0);

#pragma warning(pop)

		_serialPort.SendData((const unsigned char*)buf, static_cast<int>(len));

		Sleep(100);

		memset(buf,0,sizeof(buf));

		_serialPort.ReadData(buf, 100);

		if (0x3C == (unsigned char)buf[0] && 0x04 == (unsigned char)buf[1])
		{
			if (0x00 == (unsigned char)buf[6])
			{
				_motorsParams[0].motorType = buf[8];
				_motorsParams[0].MotorEncCoef = MOTOR_COUNTER_COEFF_OLDFIRMWARE[buf[8]];
			}
			else if (0x01 == (unsigned char)buf[6])
			{
				_motorsParams[1].motorType = buf[8];
				_motorsParams[1].MotorEncCoef = MOTOR_COUNTER_COEFF_OLDFIRMWARE[buf[8]];
			}
			else if (0x02 == (unsigned char)buf[6])
			{
				_motorsParams[2].motorType = buf[8];
				_motorsParams[2].MotorEncCoef = MOTOR_COUNTER_COEFF_OLDFIRMWARE[buf[8]];
			}			
		}
	}
	return TRUE;
}

/// <summary>
/// Queries the initial motor position.
/// </summary>
/// <returns>long.</returns>
long ThorMCM3000::QueryInitialMotorPos()
{
	Lock lock(critSect);
	for(int i=0; i < _numOfAxes; i++)
	{
		char buf[100];

		memset(buf,0,sizeof(buf));
		unsigned char cmd[] = { 0x0A, 0x04, static_cast<unsigned char>(i), 0x00, 0x00, 0x00 };  //Query motor type command

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
long ThorMCM3000::SetParam(const long paramID, const double param)
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
				case PARAM_X_POS: 
					{
						long xInvert = FALSE;
						if(_tableParams.count(IDevice::PARAM_X_INVERT)!=0)
						{
							xInvert = static_cast<long>(_tableParams[IDevice::PARAM_X_INVERT]->GetParamVal());
						}
						value *= (xInvert == FALSE)?1:-1;
					}
					break;
				case PARAM_Y_POS:
					{
						long yInvert = FALSE;
						if(_tableParams.count(IDevice::PARAM_Y_INVERT)!=0)
						{
							yInvert = static_cast<long>(_tableParams[IDevice::PARAM_Y_INVERT]->GetParamVal());
						}
						value *= (yInvert == FALSE)?1:-1;
					}
					break;
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
				StringCbPrintfW(message,MSG_SIZE,L"ThorMCM3000 SetParam failed. paramID: %d", paramID);
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
long ThorMCM3000::GetParam(const long paramID, double &param)
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
			ExecuteCmd(_tableParams[paramID]->GetCmdBytes(), ret); //execute and parse the current position from read back
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
		else if (paramID == _motorsParams[1].ParamPosCurrent)
		{
			double convertFactor = _tableParams[paramID]->GetConvertFactor();
			ExecuteCmd(_tableParams[paramID]->GetCmdBytes(), ret); //execute and parse the current position from read back
			param = ret * convertFactor* _motorsParamsInvert[1];

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
				if (NULL != _tableParams[_motorsParams[1].ParamPos])
				{
					_tableParams[_motorsParams[1].ParamPos]->UpdateParam(param);
					_tableParams[_motorsParams[1].ParamPos]->UpdateParam_C();
				}
			}
		}
		else if (paramID == _motorsParams[2].ParamPosCurrent)
		{
			double convertFactor = _tableParams[paramID]->GetConvertFactor();
			ExecuteCmd(_tableParams[paramID]->GetCmdBytes(), ret); //execute and parse the current position from read back
			param = ret * convertFactor * _motorsParamsInvert[2];

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
				if (NULL != _tableParams[_motorsParams[2].ParamPos])
				{
					_tableParams[_motorsParams[2].ParamPos]->UpdateParam(param);
					_tableParams[_motorsParams[2].ParamPos]->UpdateParam_C();
				}
			}
		}
		else if(paramID == PARAM_CONNECTION_STATUS)
		{
			param = (_deviceDetected) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
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
long ThorMCM3000::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorMCM3000::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorMCM3000::SetParamString(const long paramID, wchar_t* str)
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
long ThorMCM3000::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorMCM3000::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorMCM3000::SetupPosition()
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
long ThorMCM3000::StartPosition()
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
			if(PARAM_X_POS == (iter->second)->GetParamID())
			{
				double convertFactor = (iter->second)->GetConvertFactor();
				double xVal = Round((iter->second)->GetParamVal() * convertFactor, 4);

				long xInvert = FALSE;

				//verify that the parameter exisits in the table
				//If the user doesn't setup all of the axes the param may
				//not be available
				if(_tableParams.count(IDevice::PARAM_X_INVERT)!=0)
				{
					xInvert = static_cast<long>(_tableParams[IDevice::PARAM_X_INVERT]->GetParamVal());
				}

				if (xInvert == TRUE)
				{
					InvertMinMaxRange(_xMin,_xMax);
				}
				if(xVal >= _xMin && xVal <= _xMax)
				{
					double currentXPos;
					GetParam(PARAM_X_POS_CURRENT,currentXPos);
					currentXPos *= (xInvert == FALSE)?1:-1;
					if (currentXPos > _xMax || currentXPos < _xMin) 
					{
						if (xInvert == TRUE)// invert back
						{
							InvertMinMaxRange(_xMin,_xMax);
						}
						continue; // do nothing if command is out of range
					}
					double diff = (xVal - currentXPos)*((xInvert == FALSE)?1:-1);
					wchar_t stageMessage[512];
					StringCbPrintfW(stageMessage,512,L"Large X stage move (%d.%03d mm). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the X stage.\nPower cycling the Controller Stage may resolve the issue.",static_cast<long>(diff),static_cast<long>(abs(static_cast<long>((diff-static_cast<long>(diff)) * 1000))));
					if((diff > _xThreshold)||(diff < (_xThreshold*-1.0)))
					{
						if(IDNO == MessageBox(NULL,stageMessage,L"",MB_YESNO))
						{
							double param_c = (iter->second)->GetParamCurrent() * convertFactor; // convert back to mm before setting parameter
							SetParam(PARAM_X_POS, param_c);
							(iter->second)->SetParamBool(FALSE);
							if (xInvert == TRUE)// invert back
							{
								InvertMinMaxRange(_xMin,_xMax);
							}
							continue;
						}
					}
				}
				if (xInvert == TRUE)// invert back
				{
					InvertMinMaxRange(_xMin,_xMax);
				}
			}
			else if(PARAM_Y_POS == (iter->second)->GetParamID())
			{
				double convertFactor = (iter->second)->GetConvertFactor();
				double yVal = Round((iter->second)->GetParamVal() * convertFactor, 4);

				long yInvert = FALSE;
				//verify that the parameter exisits in the table
				//If the user doesn't setup all of the axes the param may
				//not be available
				if(_tableParams.count(IDevice::PARAM_Y_INVERT)!=0)
				{
					yInvert = static_cast<long>(_tableParams[IDevice::PARAM_Y_INVERT]->GetParamVal());
				}

				if (yInvert == TRUE)
				{
					InvertMinMaxRange(_yMin,_yMax);
				}
				if(yVal >= _yMin && yVal <= _yMax)
				{
					double currentYPos;
					GetParam(PARAM_Y_POS_CURRENT,currentYPos);
					currentYPos *= (yInvert == FALSE)?1:-1;
					if (currentYPos > _yMax || currentYPos < _yMin) 
					{
						if (yInvert == TRUE)
						{
							InvertMinMaxRange(_yMin,_yMax);
						}
						continue; // do nothing if command is out of range
					}
					double diff = (yVal - currentYPos)*((yInvert == FALSE)?1:-1);
					wchar_t stageMessage[512];
					StringCbPrintfW(stageMessage,512,L"Large Y stage move (%d.%03d mm). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the Y stage.\nPower cycling the Controller Stage may resolve the issue.",static_cast<long>(diff),static_cast<long>(abs(static_cast<long>((diff-static_cast<long>(diff)) * 1000))));
					if((diff > _yThreshold)||(diff < (_yThreshold*-1.0)))
					{
						if(IDNO == MessageBox(NULL,stageMessage,L"",MB_YESNO))
						{
							double param_c = (iter->second)->GetParamCurrent() * convertFactor; // convert back to mm before setting parameter
							SetParam(PARAM_Y_POS, param_c);
							(iter->second)->SetParamBool(FALSE);
							if (yInvert == TRUE)
							{
								InvertMinMaxRange(_yMin,_yMax);
							}
							continue;
						}
					}
				}
				if (yInvert == TRUE)
				{
					InvertMinMaxRange(_yMin,_yMax);
				}
			}
			else if(PARAM_Z_POS == (iter->second)->GetParamID())
			{
				double convertFactor = (iter->second)->GetConvertFactor();
				double zVal = Round((iter->second)->GetParamVal() * convertFactor, 4);

				long zInvert = FALSE;
				//verify that the parameter exisits in the table
				//If the user doesn't setup all of the axes the param may
				//not be available
				if(_tableParams.count(IDevice::PARAM_Y_INVERT)!=0)
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
long ThorMCM3000::StatusPosition(long &status)
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

		unsigned char cmd[] = { 0x80, 0x04, static_cast<unsigned char>(i), 0x00, 0x00, 0x00 }; //MGMSG_MOT_REQ_STATUSUPDATE 

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
long ThorMCM3000::ReadPosition(DeviceType deviceType,double &pos)
{
	return FALSE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorMCM3000::PostflightPosition()
{
	return TRUE;
}


/// <summary>
/// Executes the command.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorMCM3000::ExecuteCmd(ParamInfo* pParamInfo)
{

	long paramID = pParamInfo->GetParamID();
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();	

	//if the command is set zero, then devide the cmdbytes into to commands
	//The first one to set the counter counts to zero and the second one to
	//set the position counts to zero
	if (paramID == PARAM_X_ZERO || paramID == PARAM_Y_ZERO || paramID == PARAM_Z_ZERO)		
	{

		std::vector<unsigned char> cmd1(cmd.begin(), cmd.end() - 12);

		std::vector<unsigned char> cmd2(cmd.begin() + 12, cmd.end());

		double readBackVal = -1;

		ExecuteCmd(cmd1, readBackVal);

		ExecuteCmd(cmd2, readBackVal);
	}
	else if (paramID == PARAM_X_POS || paramID == PARAM_Y_POS || paramID == PARAM_Z_POS)
	{
		std::vector<unsigned char> paramValBytes = GetBytes((int)Round(pParamInfo->GetParamVal(),0));
		cmd[8] =  paramValBytes[0];
		cmd[9] =  paramValBytes[1];
		cmd[10] =  paramValBytes[2];
		cmd[11] =  paramValBytes[3];
		double readBackVal = -1;
		ExecuteCmd(cmd, readBackVal);
	}
	else if((paramID == PARAM_X_MOTOR_TYPE) || (paramID == PARAM_Y_MOTOR_TYPE) || (paramID == PARAM_Z_MOTOR_TYPE) )
	{	
		std::vector<unsigned char> paramValBytes = GetBytes((int)Round(pParamInfo->GetParamVal(),0));
		cmd[8] =  paramValBytes[0];
		cmd[9] =  paramValBytes[1];
		cmd[10] =  paramValBytes[2];
		cmd[11] =  paramValBytes[3];

		double readBackVal = -1;

		std::vector<unsigned char> cmdforhightorque = pParamInfo->GetCmdBytes();

		if (pParamInfo->GetParamVal() == 6)
			{
				    std::vector<unsigned char> commandBytes;
					unsigned char kval_run_t = 55;
					signed short int int_spd_t = 2800;
					unsigned char st_slp_t = 24;
					unsigned char fn_slp_acc_t = 68;
					unsigned char fn_slp_dec_t = 68;
					uint8_t int_spd[2];
					int_spd[0] = (unsigned char)int_spd_t & 0xFF;
					int_spd[1] = (unsigned char)((int_spd_t >> 8) & 0xFF);
					unsigned char commandBytesZNewMotor[] = { 0x26, 0x04, 0x1E, 0x00, 0x00, 0x00,
                    0x00, //channel
                    0x06, //type - B-Scope Z High Torque
                    kval_run_t, //kval_run
                    0x08, //kval_hold
                    kval_run_t, //kval_acc
                    kval_run_t, //kval_dec
                    0x88, 0x0E, //config
                    0x20, 0x00, //max_speed
                    0x00, 0x8f, //acc
                    0x00, 0x8f, //dec
                    0xFF, 0x03, //fs_spd    
                    int_spd[0], int_spd[1], //int_spd ******
                    st_slp_t, //st_slp  ******
                    fn_slp_acc_t, //fn_slp_acc    ******
                    fn_slp_dec_t, //fn_slp_dec    ******
                    0x01, //limits
                    0x42, 0x0F, 0x05, 0x00, //coef_step 39.06, except the third byte is changed to 0x05 for proper motor factor
                    0x10, 0x27, 0x00, 0x00, //coef_qei 100
                    0x00};

					commandBytes.assign(commandBytesZNewMotor, commandBytesZNewMotor + sizeof(commandBytesZNewMotor)/sizeof(commandBytesZNewMotor[0]));
					cmdforhightorque = commandBytes;
		   }

		ExecuteCmd(cmdforhightorque, readBackVal);

		Sleep(200);

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
long ThorMCM3000::ExecuteCmd(std::vector<unsigned char> cmd, double &readBackValue)
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

	Sleep(30); //Determined by testing... Shorter sleep time may cause bad communications


	memset(buf, 0, sizeof(buf));

	_serialPort.ReadData(buf, 100);
	readBackValue = ((unsigned char)buf[8] | (unsigned char)buf[9] << 8 | (unsigned char)buf[10] << 16 | (unsigned char)buf[11] << 24);
	return TRUE;
}

/// <summary>
/// Executes the position now.
/// </summary>
void ThorMCM3000::ExecutePositionNow()
{
	PreflightPosition();
	SetupPosition();
	StartPosition();
	long status = IDevice::STATUS_BUSY;
	do
	{
		if (FALSE == StatusPosition(status))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorMCM3000::statusPosition failed");
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
long ThorMCM3000::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}

/// <summary>
/// Rounds up to the specified decimal place value.
/// </summary>
/// <param name="val">The value.</param>
/// <param name="decPlace">The decimal place.</param>
/// <returns>double.</returns>
double ThorMCM3000::rndup(double val,int decPlace)
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
std::vector<unsigned char> ThorMCM3000::GetBytes(int value)
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
double ThorMCM3000::Round(double number, int decimals)
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
long ThorMCM3000::ResetStageZeroPosition(long stageID)
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
				StringCbPrintfW(message,MSG_SIZE,L"ThorBScope::ResetStageZeroPosition GetParam of PARAM_Z_POS_CURRENT failed");
				LogMessage(message);
			}

			if ((currentPos < _zMin) || (currentPos > _zMax))
			{
				if (FALSE == SetParam(PARAM_Z_ZERO,static_cast<double>(TRUE)))
				{
					StringCbPrintfW(message,MSG_SIZE,L"ThorBScope::ResetStageZeroPosition SetParam of PARAM_Z_ZERO failed");
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
//long ThorMCM3000::CheckMotorConnection()
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
long ThorMCM3000::BuildParamTable()
{
	_tableParams.clear();
	//Execution in order of top to bottom of list:
	std::vector<unsigned char> commandBytes;
	commandBytes.push_back(0x00);
	ParamInfo* tempParamInfo = new ParamInfo(	
		PARAM_DEVICE_TYPE,						//ID
		STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2,	//VAL
		STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2,	//PARAM C
		FALSE,									//PARAM B
		TYPE_LONG,								//TYPE
		TRUE,									//AVAILABLE
		TRUE,									//READ ONLY
		FALSE,									//CONVERSION (YES/NO)
		FALSE,									//CONVERSION FACTOR
		STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2,	//MIN
		STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2,	//MAX
		STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2,	//DEFAULT
		-1,										//Motor ID
		commandBytes);							//Command
	_tableParams.insert(std::pair<long, ParamInfo*>(Params::PARAM_DEVICE_TYPE,tempParamInfo));

	//build table entries for Axis0 parameters
	unsigned char commandBytesA0Zero[] = { 0x09, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	 //MGMSG_MOT_SET_POSCOUNTER Axis 0 (Stage 1)
		0x10, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOT_SET_POSCOUNTER Axis 0 (Stage 1)
	commandBytes.assign(commandBytesA0Zero, commandBytesA0Zero + sizeof(commandBytesA0Zero)/sizeof(commandBytesA0Zero[0]));

	tempParamInfo = new ParamInfo(	
		_motorsParams[0].ParamZero,
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
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[0].ParamZero,tempParamInfo));

	unsigned char commandBytesA0Home[] = { 0x09, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOT_SET_POSCOUNTER Axis 0 (Stage 1)
	commandBytes.assign(commandBytesA0Zero, commandBytesA0Zero + sizeof(commandBytesA0Zero)/sizeof(commandBytesA0Zero[0]));
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

	unsigned char commandBytesA0Pos[] = { 0x53, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOT_MOVE_ABSOLUTE Axis 0 (Stage 1)
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
		_motorsParams[0].MotorEncCoef / 1000000, // Convert to mm
		_motorsParams[0].StagePosMin,
		_motorsParams[0].StagePosMax,
		STAGE_POSITION_DEFAULT,
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[0].ParamPos,tempParamInfo));

	unsigned char commandBytesA0PosCurrent[] = { 0x0A, 0x04, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOT_REQ_ENCCOUNTER Axis 0 (Stage 1)
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
		_motorsParams[0].MotorEncCoef / 1000000, // Convert to mm
		_motorsParams[0].StagePosMin,
		_motorsParams[0].StagePosMax,
		STAGE_POSITION_DEFAULT,
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[0].ParamPosCurrent,tempParamInfo));

	unsigned char commandBytesA0Stop[] = { 0x65, 0x04, 0x00, 0x01, 0x00, 0x00 }; //MGMSG_MOT_MOVE_STOP  Axis 0 (Stage 1)
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

	//build table entries for Axi1 parameters
	unsigned char commandBytesA1Zero[] = { 0x09, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,	 //MGMSG_MOT_SET_ENCCOUNTER Axis 1 (Stage 2)
		0x10, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOT_SET_POSCOUNTER Axis 1 (Stage 2)
	commandBytes.assign(commandBytesA1Zero, commandBytesA1Zero + sizeof(commandBytesA1Zero)/sizeof(commandBytesA1Zero[0]));
	tempParamInfo = new ParamInfo(	
		_motorsParams[1].ParamZero,
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
		1,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[1].ParamZero,tempParamInfo));

	unsigned char commandBytesA1Home[] = { 0x09, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOT_SET_ENCCOUNTER Axis 1 (Stage 2)
	commandBytes.assign(commandBytesA1Zero, commandBytesA1Zero + sizeof(commandBytesA1Zero)/sizeof(commandBytesA1Zero[0]));
	tempParamInfo = new ParamInfo(	
		_motorsParams[1].ParamHome,
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
		1,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[1].ParamHome,tempParamInfo));

	unsigned char commandBytesA1Pos[] = { 0x53, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOT_MOVE_STOP Axis 1 (Stage 2)
	commandBytes.assign(commandBytesA1Pos, commandBytesA1Pos + sizeof(commandBytesA1Pos)/sizeof(commandBytesA1Pos[0]));
	tempParamInfo = new ParamInfo(
		_motorsParams[1].ParamPos,
		_motorsParams[1].initialPos,
		_motorsParams[1].initialPos,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		TRUE,
		_motorsParams[1].MotorEncCoef / 1000000, // Convert to mm
		_motorsParams[1].StagePosMin,
		_motorsParams[1].StagePosMax,
		STAGE_POSITION_DEFAULT,
		1,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[1].ParamPos,tempParamInfo));

	unsigned char commandBytesA1PosCurrent[] = { 0x0A, 0x04, 0x01, 0x00, 0x00, 0x00 }; //MGMSG_MOT_SET_ENCCOUNTER Axis 1 (Stage 2)
	commandBytes.assign(commandBytesA1PosCurrent, commandBytesA1PosCurrent + sizeof(commandBytesA1PosCurrent)/sizeof(commandBytesA1PosCurrent[0]));
	tempParamInfo = new ParamInfo(
		_motorsParams[1].ParamPosCurrent,
		STAGE_POSITION_DEFAULT,
		STAGE_POSITION_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		FALSE,
		_motorsParams[1].MotorEncCoef / 1000000, // Convert to mm
		_motorsParams[1].StagePosMin,
		_motorsParams[1].StagePosMax,
		STAGE_POSITION_DEFAULT,
		1,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[1].ParamPosCurrent,tempParamInfo));

	unsigned char commandBytesA1Stop[] = { 0x65, 0x04, 0x01, 0x01, 0x00, 0x00 }; //MGMSG_MOT_MOVE_STOP  Axis 1 (Stage 2)
	commandBytes.assign(commandBytesA1Stop, commandBytesA1Stop + sizeof(commandBytesA1Stop)/sizeof(commandBytesA1Stop[0]));
	tempParamInfo = new ParamInfo(	
		_motorsParams[1].ParamStop,
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
		1,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[1].ParamStop,tempParamInfo));


	//build table entries for Axis2 parameters
	unsigned char commandBytesA2Zero[] = { 0x09, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,	 //MGMSG_MOT_SET_ENCCOUNTER Axis 2 (Stage 3)
		0x10, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOT_SET_POSCOUNTER Axis 2 (Stage 3)
	commandBytes.assign(commandBytesA2Zero, commandBytesA2Zero + sizeof(commandBytesA2Zero)/sizeof(commandBytesA2Zero[0]));
	tempParamInfo = new ParamInfo(	
		_motorsParams[2].ParamZero,
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
		2,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[2].ParamZero,tempParamInfo));

	unsigned char commandBytesA2Home[] = { 0x09, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOT_SET_ENCCOUNTER Axis 2 (Stage 3)
	commandBytes.assign(commandBytesA2Zero, commandBytesA2Zero + sizeof(commandBytesA2Zero)/sizeof(commandBytesA2Zero[0]));
	tempParamInfo = new ParamInfo(	
		_motorsParams[2].ParamHome,
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
		2,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[2].ParamHome,tempParamInfo));

	unsigned char commandBytesA2Pos[] = { 0x53, 0x04, 0x06, 0x00, 0x11 | 0x80, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOT_MOVE_ABSOLUTE Axis 2 (Stage 3)
	commandBytes.assign(commandBytesA2Pos, commandBytesA2Pos + sizeof(commandBytesA2Pos)/sizeof(commandBytesA2Pos[0]));
	tempParamInfo = new ParamInfo(
		_motorsParams[2].ParamPos,
		_motorsParams[2].initialPos,
		_motorsParams[2].initialPos,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		TRUE,
		_motorsParams[2].MotorEncCoef / 1000000, // Convert to mm
		_motorsParams[2].StagePosMin,
		_motorsParams[2].StagePosMax,
		STAGE_POSITION_DEFAULT,
		2,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[2].ParamPos,tempParamInfo));

	unsigned char commandBytesA2PosCurrent[] = { 0x0A, 0x04, 0x02, 0x00, 0x00, 0x00 }; //MGMSG_MOT_REQ_ENCCOUNTER Axis 2 (Stage 3)
	commandBytes.assign(commandBytesA2PosCurrent, commandBytesA2PosCurrent + sizeof(commandBytesA2PosCurrent)/sizeof(commandBytesA2PosCurrent[0]));
	tempParamInfo = new ParamInfo(
		_motorsParams[2].ParamPosCurrent,
		STAGE_POSITION_DEFAULT,
		STAGE_POSITION_DEFAULT,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		FALSE,
		_motorsParams[2].MotorEncCoef / 1000000, // Convert to mm
		_motorsParams[2].StagePosMin,
		_motorsParams[2].StagePosMax,
		STAGE_POSITION_DEFAULT,
		2,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[2].ParamPosCurrent,tempParamInfo));

	unsigned char commandBytesA2Stop[] = { 0x65, 0x04, 0x02, 0x01, 0x00, 0x00 }; //MGMSG_MOT_MOVE_STOP  Axis 2 (Stage 3)
	commandBytes.assign(commandBytesA2Stop, commandBytesA2Stop + sizeof(commandBytesA2Stop)/sizeof(commandBytesA2Stop[0]));
	tempParamInfo = new ParamInfo(	
		_motorsParams[2].ParamStop,
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
		2,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[2].ParamStop,tempParamInfo));

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

	std::vector<unsigned char> commandBytes1Invert;
	commandBytes1Invert.push_back(0x00);
	tempParamInfo = new ParamInfo(	
		_motorsParams[1].ParamInvert,
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
		1,
		commandBytes1Invert);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[1].ParamInvert,tempParamInfo));

	std::vector<unsigned char> commandBytes2Invert;
	commandBytes2Invert.push_back(0x00);
	tempParamInfo = new ParamInfo(	
		_motorsParams[2].ParamInvert,
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
		2,
		commandBytes2Invert);
	_tableParams.insert(std::pair<long, ParamInfo*>(_motorsParams[2].ParamInvert,tempParamInfo));

	//build table entry for the device connection status
	unsigned char commandBytesCON_STA[] = { 0x00}; // Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesCON_STA, commandBytesCON_STA + sizeof(commandBytesCON_STA)/sizeof(commandBytesCON_STA[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_CONNECTION_STATUS,	//ID
		FALSE,					//VAL
		FALSE,					//PARAM C
		TRUE,					//PARAM B
		TYPE_LONG,				//TYPE
		TRUE,					//AVAILABLE
		TRUE,					//READ ONLY
		FALSE,					//CONVERSION (YES/NO)
		FALSE,					//CONVERSION FACTOR
		(double)ConnectionStatusType::CONNECTION_READY,		//MIN
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,	//MAX
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,	//DEFAULT
		1,						//Motor ID
		commandBytes);			//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTION_STATUS, tempParamInfo));

	unsigned char commandBytesA0Motor[] = { 0x3A, 0x04, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOTOR TYPE Axis 0 (Stage 1)
	commandBytes.assign(commandBytesA0Motor, commandBytesA0Motor + sizeof(commandBytesA0Motor)/sizeof(commandBytesA0Motor[0]));
	tempParamInfo = new ParamInfo(
		PARAM_X_MOTOR_TYPE,
		_motorsParams[0].motorType,
		_motorsParams[0].motorType,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		0,
		6,
		0,
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_X_MOTOR_TYPE,tempParamInfo));

	unsigned char commandBytesA1Motor[] = { 0x3A, 0x04, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOTOR TYPE Axis 1 (Stage 2)
	commandBytes.assign(commandBytesA1Motor, commandBytesA1Motor + sizeof(commandBytesA1Motor)/sizeof(commandBytesA1Motor[0]));
	tempParamInfo = new ParamInfo(
		PARAM_Y_MOTOR_TYPE,
		_motorsParams[1].motorType,
		_motorsParams[1].motorType,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		0,
		6,
		0,
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Y_MOTOR_TYPE,tempParamInfo));

	unsigned char commandBytesA2Motor[] = { 0x3A, 0x04, 0x06, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 }; //MGMSG_MOTOR TYPE Axis 2 (Stage 3)
	commandBytes.assign(commandBytesA2Motor, commandBytesA2Motor + sizeof(commandBytesA2Motor)/sizeof(commandBytesA2Motor[0]));
	tempParamInfo = new ParamInfo(
		PARAM_Z_MOTOR_TYPE,
		_motorsParams[2].motorType,
		_motorsParams[2].motorType,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		FALSE,
		0,
		6,
		0,
		0,
		commandBytes);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_Z_MOTOR_TYPE,tempParamInfo));

	return TRUE;
}
/// <summary>
/// Swap max and min value
/// </summary>
/// <param name="min">The min value.</param>
/// <param name="max">The max value.</param>
/// <returns>void.</returns>
void ThorMCM3000::InvertMinMaxRange(double& min, double& max)
{
	double temp = min;
	min = -1 * max;
	max = -1 * temp;
}