// OTMLaser.cpp : Defines the exported functions for the DLL application.
//

//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// OTMLaser.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "math.h"
#include "OTMLaser.h"
#include "OTMLaserSetupXML.h"
#include "Strsafe.h"
#include <fstream>



#define READ_TIMEOUT 1000

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

wchar_t message[MSG_SIZE];


OTMLaser::OTMLaser()
{
	_laserPower = 0;
	_laserPowerMin = 0;
	_laserPowerMax = 2;
	_laser1Min=0;
	_laser1Max=100;
	_laser2Min=0;
	_laser2Max=100;
	_laser3Min=0;
	_laser3Max=100;

	_laser1Enable=FALSE;
	_laser2Enable=FALSE;
	_laser3Enable=FALSE;

	_laser1Power=_laser1Min;
	_laser2Power=_laser2Min;
	_laser3Power=_laser3Min;

	_laser1Pos_C = 0;
	_laser2Pos_C = 0;
	_laser3Pos_C = 0;

	_laser1Pos = 0;
	_laser2Pos = 0;
	_laser3Pos = 0;

	_laser1Enable_C=FALSE;
	_laser2Enable_C=FALSE;
	_laser3Enable_C=FALSE;

	_laserPower_C = _laserPowerMin - 1;
	_laser1Power_C = _laser1Min - 1;
	_laser2Power_C = _laser2Min - 1;
	_laser3Power_C = _laser3Min - 1;

	_shutter1Flag=0;
	_shutter2Flag = 0;

	_deviceDetected = FALSE;
	_IsRS232 = TRUE;

	_bEnableSet = FALSE;
	_flag=0;

	_dataBuffer[0] = NULL;
	_readBuffer[0] = NULL;
	_errMsg[0] = NULL;

	for(InterpolationTable t : linearizationTables)
	{
		t.setEdgeBehavior(InterpolationTable::EdgeBehavior::RETURN_BOUNDS);
	}
}

OTMLaser::~OTMLaser()
{
	_instanceFlag = false;
}

bool OTMLaser:: _instanceFlag = false;

auto_ptr<OTMLaser> OTMLaser::_single(new OTMLaser());

OTMLaser *OTMLaser::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new OTMLaser());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long OTMLaser::FindDevices(long &deviceCount)
{
	long	ret = TRUE;

	//if(_deviceDetected)
	//{
	//	_serialPort.Close();
	//	_deviceDetected = FALSE;
	//}

	deviceCount = 0;

	long portID=10;

	try
	{
		auto_ptr<OTMLaserXML> pSetup(new OTMLaserXML());

		pSetup->GetConnection(portID);
		pSetup->GetLaserConfig(_IsRS232);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate OTMLaserSettings.xml file");
		deviceCount = 0;
		return FALSE;
	}

	TCHAR PortName[32];
	StringCbPrintfW(PortName,32, L"COM%d", portID);

	if(FALSE == _serialPort.Open(portID,115200))
	{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"OTMLaser FindDevices could not open serial port");
#endif
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to open com port %d",portID);
		ret = FALSE;

		// *TODO* perform an automatic scane of the available serial ports for the Z stepper
	}
	else
	{
		deviceCount = 1;
		_deviceDetected = TRUE;
		_serialPort.Close();
	}
	return ret;
}

void OTMLaser::GetMinMaxFromReadBuffer(double &minVal, double &maxVal)
{

	maxVal=100;
	minVal=0;
	//*TODO* the firmware for the OTM has a bug where using the 
	//maximum value will disable the laser. To prevent this 
	//from happening. Restrict the upper limit of the laser
	//range to 98% of the total range.
	//if(maxVal > minVal)
	//{
	//	maxVal = minVal + .98 * (maxVal - minVal);
	//}
}

long OTMLaser::LaserPosHome(double laserId)
{
	UCHAR slot_number = 0;
	UCHAR cmd[6] = { 0x43, 0x04, slot_number, 0x00, 0x24, 0x01 };
	int r = _serialPort.SendData(cmd, sizeof(cmd) / sizeof(UCHAR));
	return TRUE;
}

long OTMLaser::SelectDevice(const long device)
{
	long	ret = FALSE;

	if(FALSE == _deviceDetected)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"No devices found");
		return FALSE;
	}

	long portID=0;

	try
	{
		auto_ptr<OTMLaserXML> pSetup(new OTMLaserXML());

		pSetup->GetConnection(portID);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate OTMLaserSettings.xml file");
		return FALSE;
	}

	TCHAR PortName[32];
	StringCbPrintfW(PortName,32,L"COM%d", portID);

	if(FALSE == _serialPort.Open(portID,115200))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"OTMLaser unable to open port %d",portID);
		return FALSE;
	}

	LaserPosHome(1);
	_laser1Pos_C = 0;

	GetMinMaxFromReadBuffer(_laser1Min, _laser1Max);

	switch(device)
	{
	case 0:		{ ret = TRUE; }break;

	default:	{ }
	}

	if (TRUE == ret){
		//CalibrateOTMLaser();
	}
	return ret;
}

long OTMLaser::TeardownDevice()
{
	//Turn off all lasers when the device is released
	if (TRUE == SetOTMLaserEnable(1, 0))
	{
		_laser1Enable_C = 0;

	}
	if (TRUE == SetOTMLaserEnable(2, 0))
	{

		_laser2Enable_C = 0;

	}
	if (TRUE == SetOTMLaserEnable(3, 0))
	{

		_laser3Enable_C = 0;
	}

	_serialPort.Close();
	return TRUE;
}

long OTMLaser::GetParamInfo
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
	case PARAM_LASER_POWER:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _laserPowerMin;
			paramMax = _laserPowerMax;
			paramDefault = _laserPowerMin;
		}
		break;

	case PARAM_LASER1_POWER:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _laser1Min;
			paramMax = _laser1Max;
			paramDefault = _laser1Min;
		}
		break;

	case PARAM_LASER1_POWER_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _laser1Min;
			paramMax = _laser1Max;
			paramDefault = _laser1Min;
		}
		break;

	case PARAM_LASER1_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0.02;
			paramMax = 100;
			paramDefault = 0;
		}
		break;

	case PARAM_LASER1_POS_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;
		}
		break;

	case PARAM_LASER1_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;

	case PARAM_LASER2_POWER:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _laser2Min;
			paramMax = _laser2Max;
			paramDefault = _laser2Min;
		}
		break;

	case PARAM_LASER2_POWER_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _laser2Min;
			paramMax = _laser2Max;
			paramDefault = _laser2Min;
		}
		break;

	case PARAM_LASER2_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;
		}
		break;

	case PARAM_LASER2_POS_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;
		}
		break;

	case PARAM_LASER2_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;	

	case PARAM_LASER3_POWER:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _laser3Min;
			paramMax = _laser3Max;
			paramDefault = _laser3Min;
		}
		break;

	case PARAM_LASER3_POWER_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _laser3Min;
			paramMax = _laser3Max;
			paramDefault = _laser3Min;
		}
		break;

	case PARAM_LASER3_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;
		}
		break;

	case PARAM_LASER3_POS_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 0;
			paramMax = 100;
			paramDefault = 0;
		}
		break;

	case PARAM_LASER3_ENABLE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;

	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = LASER1|LASER2|LASER3;
		}
		break;

	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = (double)ConnectionStatusType::CONNECTION_READY;
			paramMax = paramDefault = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
		break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long OTMLaser::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_LASER_POWER:
		{
			if((param >= _laserPowerMin) && (param <= _laserPowerMax))
			{
				_laserPower = (long)param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER_POWER out of range %d to %d",static_cast<long>(_laserPowerMin),static_cast<long>(_laserPowerMax));
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER1_POWER:
		{
			if((param >= _laser1Min) && (param <= _laser1Max))
			{
				_laser1Power = param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER1_POWER out of range %d to %d",static_cast<long>(_laser1Min*1000),static_cast<long>(_laser1Max*1000));
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER1_POS:
		{
			if((param >= 0.02) && (param <= 100))
			{
				_laser1Pos = param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER1_POS out of range %d to %d",static_cast<long>(0),static_cast<long>(100));
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER1_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_laser1Enable = static_cast<long>(param);
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER1_ENABLE out of range 0 to 1");
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER1_POWER_CURRENT:
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter is read only");
			ret = FALSE;
		}
		break;
	case PARAM_LASER1_POS_CURRENT:
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter is read only");
			ret = FALSE;
		}
		break;
	case PARAM_LASER2_POWER:
		{
			if((param >= _laser2Min) && (param <= _laser2Max))
			{
				_laser2Power = param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER2_POWER out of range %d to %d",static_cast<long>(_laser2Min*1000),static_cast<long>(_laser2Max*1000));
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER2_POS:
		{
			if((param >= 0) && (param <= 100))
			{
				_laser2Pos = param;
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER2_POS out of range %d to %d",static_cast<long>(0),static_cast<long>(100));
				ret = FALSE;
			}
		}
		break;
	case PARAM_LASER2_ENABLE:
		{
			if((param >= 0) && (param <= 1))
			{
				_laser2Enable = static_cast<long>(param);
			}
			else
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LASER2_ENABLE out of range 0 to 1");
				ret = FALSE;
			}
		}
		break;	
	case PARAM_LASER2_POWER_CURRENT:
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter is read only");
			ret = FALSE;
		}
		break;
	case PARAM_LASER2_POS_CURRENT:
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter is read only");
			ret = FALSE;
		}
		break;
	default:
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter not implemented");
			ret = FALSE;
		}
	}

	return ret;
}

long OTMLaser::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{
	case PARAM_LASER_POWER:
		{ 
			param = static_cast<double>(_laserPower_C); 
		}
		break;
	case PARAM_LASER1_POWER:	
		{ 
			param = static_cast<double>(_laser1Power); 
		}
		break;	
	case PARAM_LASER1_POS:	
		{ 
			param = static_cast<double>(_laser1Pos_C); 
		}
		break;	
	case PARAM_LASER1_ENABLE:	
		{ 
			param = static_cast<double>(_laser1Enable); 
		}
		break;
	case PARAM_LASER1_POWER_CURRENT:	
		{ 
			param = static_cast<double>(_laser1Power_C); 
		}
		break;
	case PARAM_LASER1_POS_CURRENT:	
		{ 
			param = static_cast<double>(_laser1Pos_C); 
		}
		break;
	case PARAM_LASER2_POWER:	
		{ 
			param = static_cast<double>(_laser2Power); 
		}
		break;
	case PARAM_LASER2_POS:	
		{ 
			param = static_cast<double>(_laser2Pos_C); 
		}
		break;
	case PARAM_LASER2_ENABLE:	
		{ 
			param = static_cast<double>(_laser2Enable); 
		}
		break;
	case PARAM_LASER2_POWER_CURRENT:	
		{ 
			param = static_cast<double>(_laser2Power_C); 
		}
		break;	
	case PARAM_LASER2_POS_CURRENT:	
		{ 
			param = static_cast<double>(_laser2Pos_C); 
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			param = static_cast<double>(LASER1|LASER2|LASER3);
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = (_deviceDetected) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
		break;
	default:
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter not implemented");
			ret = FALSE;
		}
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
long OTMLaser::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long OTMLaser::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long OTMLaser::SetParamString(const long paramID, wchar_t* str)
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
long OTMLaser::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;
	return ret;
}

long OTMLaser::PreflightPosition()
{
	return TRUE;
}

long OTMLaser::SetupPosition()
{
	return TRUE;
}

long OTMLaser::StartPosition()
{
	long ret = TRUE;

#ifdef LOGGING_ENABLED
	StringCbPrintfW(_errMsg,MSG_SIZE,L"SetOTMLaser1 Enable %d",_laser1Enable);	
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, _errMsg);		
	StringCbPrintfW(_errMsg,MSG_SIZE,L"SetOTMLaser1 Power %d",static_cast<long>(_laser1Power *1000));
	logDll->TLTraceEvent(VERBOSE_EVENT, 1,_errMsg);						
#endif
	if(_laser1Pos != _laser1Pos_C)
	{
		ret = SetOTMLaserFocus(abs(_laser1Pos - _laser1Pos_C), _laser1Pos > _laser1Pos_C);
		if(TRUE == ret)
		{
			_laser1Pos_C = _laser1Pos;
		}
		else
		{	 
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetOTMLaserFocus 0 failed");
			return FALSE;
		}
	}

	if(_laserPower != _laserPower_C)
	{		
		ret = SetOTMLaserEnable(0, (0 != _laserPower));
		if(TRUE == ret)
		{
			_laserPower_C = _laserPower;
		}
		else
		{	 
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetOTMLaserEnable 0 failed");
			return FALSE;
		}
	}

	if(_laser1Enable != _laser1Enable_C)
	{		
		ret = SetOTMLaserEnable(1, (TRUE ==_laser1Enable));
		if(TRUE == ret)
		{
			_bEnableSet = _laser1Enable_C = _laser1Enable;
		}
		else
		{	 
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetOTMLaserEnable 1 failed");
			return FALSE;
		}
	}

	if(_laser1Power != _laser1Power_C)
	{
		double powerToSet = _laser1Power;
		if(linearizationTables[0].isValid())
		{
			powerToSet = linearizePower(_laser1Min, _laser1Max, _laser1Power, 0);
		}
		ret = SetOTMPower(1, powerToSet);
		if(TRUE == ret)
		{
			_laser1Power_C = _laser1Power;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetOTMPower 1 failed");
			return FALSE;
		}
	}


	if(_laser2Enable != _laser2Enable_C)
	{		
		ret=SetOTMLaserEnable(2, (TRUE == _laser2Enable) );
		if(TRUE == ret)
		{
			_bEnableSet = _laser2Enable_C = _laser2Enable;
		}
		else
		{	 
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetOTMLaserEnable 2 failed");
			return FALSE;
		}
	}

	if(_laser2Power != _laser2Power_C)
	{
		double powerToSet = _laser2Power;
		if(linearizationTables[1].isValid())
		{
			powerToSet = linearizePower(_laser2Min, _laser2Max, _laser2Power, 0);
		}
		ret=SetOTMPower(2, powerToSet);
		if(TRUE == ret)
		{
			_laser2Power_C = _laser2Power;
		}
		else
		{	
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetOTMPower 2 failed");
			return FALSE;
		}
	}

	return ret;
}

long OTMLaser::StatusPosition(long &status)
{
	if(TRUE == _bEnableSet) // wait only if any Laser enable changed state
	{
		Sleep(3000); //Wait time for laser(s) to be ready		
	}	
	status = IDevice::STATUS_READY;
	return TRUE;
}

long OTMLaser::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;
	StringCbPrintfW(_errMsg,MSG_SIZE,L"ReadPosition failed! Use the PARAM_LASER#_POWER_CURRENT to read the power of the laser");
	return ret;
}

long OTMLaser::PostflightPosition()
{
	_bEnableSet = FALSE; //Set to FALSE at this stage which is always called after a new command was passed and completed
	return TRUE;
}

/*
slot_id 1 values:
BOARD 1 = 0x21	with extended data (0x21 OR 0x80) = 0xA1
BOARD 2 = 0x22	with extended data (0x22 OR 0x80) = 0xA2
BOARD 3 = 0x23	with extended data (0x23 OR 0x80) = 0xA3
BOARD 4 = 0x24	with extended data (0x24 OR 0x80) = 0xA4
*/
long OTMLaser::SetOTMLaserFocus(double delta, bool isCCW)
{
	UCHAR slot_number = 0;
	const double NmPerCount = 17.77778;
	INT32 value = (INT32)(1000.0 * delta / NmPerCount + 0.5);
	UCHAR data[4];
	for(int i = 0; i < 4; i++)
		data[i] = (value >> (i * 8));
	UCHAR cmd[28] = { 0x16, 0x04, 22, 0x00, 0xA1, 0x01, 
		slot_number, 0,  // Chan Ident
		2,0,                    // Jog Mode - single step jogging
		data[0], data[1], data[2], data[3],    // Jog Step Size in encoder counts
		0x00,0x00,0x00,0x00,    // Jog Min Velocity (not used)
		0x00,0x00,0x00,0x00,    // Jog Acceleration (not used)
		0x00,0x00,0x00,0x00,    // Jog Max Velocity (not used)
		0,0                    // Stop Mode (not used) - 1 for immediate (abrupt) stop or 2 for profiled stop (with controlled deceleration).
	};

	int r = _serialPort.SendData(cmd, sizeof(cmd) / sizeof(UCHAR));

	Sleep(200);

	UCHAR mov[6] = { 0x6A, 0x04, static_cast<UCHAR>(slot_number), (isCCW) ? static_cast<UCHAR>(1) : static_cast<UCHAR>(0), 0x21, 0x01 };
	r = _serialPort.SendData(mov, sizeof(mov) / sizeof(UCHAR));
	return TRUE;
}

long OTMLaser::SetOTMLaserEnable(int OTMID, bool enable)
{
	switch(OTMID)
	{
	case 0:
		{
			USHORT dat{};
			UCHAR cmdOnOff[6] = {0x00, 0x08, 0x03, 0x00, 0x24, 0x01};
			if(TRUE == _IsRS232) // If the laser uses the RS232 cable we need to send the command to the 5th slot
			{
				cmdOnOff[2] = 0x04;
				cmdOnOff[4] = 0x25;
			}
			if(!enable)
			{
				//Disable Laser command
				cmdOnOff[0] = 0x12;
				int  r = _serialPort.SendData(cmdOnOff, sizeof(cmdOnOff) / sizeof(UCHAR));
			}
			else 
			{		
				//2W power
				if(_laserPower == 1)
				{
					//60% power was measured to be ~2.3W. Keep in mind that this value can shift between lasers and
					// it can be changed if necessary.
					dat = (USHORT)(0.6 * 32767 + 0.5); 
				}
				//5W power
				else if(_laserPower == 2)
				{
					dat = 32767;
				}		
				//Set Laser power
				UCHAR cmd[10] = { 0x00, 0x08, 0x04, 0x00, 0xA4, 0x01, 0x03, 0x02, (UCHAR)dat, (UCHAR)(dat >> 8)};	
				if(TRUE == _IsRS232)
				{	
					cmd[4] = 0xA5;
				}
				int  r = _serialPort.SendData(cmd, sizeof(cmd) / sizeof(UCHAR));

				Sleep(1000);
				//Enable Laser command
				cmdOnOff[0] = 0x11;
				r = _serialPort.SendData(cmdOnOff, sizeof(cmdOnOff) / sizeof(UCHAR));
			}
		}
		break;
		//Shutter1 Control commmand, BOARD 2
	case 1:
		{
			UCHAR cmdOnOff[6] = {0xCB, 0x04, (UCHAR)OTMID, 0x00, 0x22, 0x01};
			if(enable) cmdOnOff[3] = 0x01;
			int r = _serialPort.SendData(cmdOnOff, sizeof(cmdOnOff) / sizeof(UCHAR));
		}
		break;
		//Shutter2 control command, BOARD 3
	case 2:
		{
			UCHAR cmdOnOff[6] = {0xCB, 0x04, (UCHAR)OTMID, 0x00, 0x23, 0x01};
			if(enable) cmdOnOff[3] = 0x01;
			int r = _serialPort.SendData(cmdOnOff, sizeof(cmdOnOff) / sizeof(UCHAR));
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

//Set Noise Eaters
long OTMLaser::SetOTMPower(int OTMID, double power)
{
	// The Laser power setpoint (0 to 32767 -> 0% to 100% power).
	USHORT dat = (USHORT)(power * 32767 / 100 + 0.5);
	UCHAR cmd[10] = { 0x00, 0x08, 0x04, 0x00, 0xA4, 0x01, 0x03, (UCHAR)(OTMID - 1), (UCHAR)dat, (UCHAR)(dat >> 8)};
	int r = _serialPort.SendData(cmd, sizeof(cmd) / sizeof(UCHAR));
	Sleep(20);
	return TRUE;
}

long OTMLaser::GetLastErrorMsg(wchar_t *msg, long size)
{	
	wcsncpy_s(msg,MSG_SIZE,_errMsg,MSG_SIZE);
	return TRUE;
}

std::wstring OTMLaser::getTableDirectoryPath()
{

	HINSTANCE resourceManager = LoadLibrary(L"ResourceManager.dll");
	if(resourceManager)
	{
		typedef bool(CALLBACK* GetApplicationSettingsPathFunction)(wchar_t* buffer, unsigned int bufferSize);
		GetApplicationSettingsPathFunction getApplicationSettingsPath = (GetApplicationSettingsPathFunction)GetProcAddress(resourceManager, "GetApplicationSettingsPath");
		if(getApplicationSettingsPath)
		{

			const int BUFFER_SIZE = 1000;
			wchar_t pathBuffer[BUFFER_SIZE];
			bool pathRetrieved = getApplicationSettingsPath(pathBuffer, BUFFER_SIZE);

			if(pathRetrieved)
			{
				std::wstring directoryPath(pathBuffer);
				directoryPath +=  std::wstring(L"OTM\\");
				return directoryPath;
			}
		}
		FreeLibrary(resourceManager);
	}

	return L".\\";
}

/// <summary> Loads linearization tables from the application settings folder to be used to linearize the laser light intensity </summary>
/// <returns> True if a valid table was loaded for all laser channels </returns>
long OTMLaser::CalibrateOTMLaser()
{
	std::wstring applicationSettingsPath = getTableDirectoryPath();
	std::wstring tablePrefix = L"OTM_LinearizationTable_";
	std::wstring tablePostfix = L".txt";
	std::wstringstream filePath;

	bool allTablesValid = true;
	for(int i=0; i<MAX_CHANNELS; i++)
	{
		filePath.str(std::wstring());
		filePath << applicationSettingsPath << tablePrefix << i+1 << tablePostfix;
		linearizationTables[i].loadFromFile(filePath.str());

		allTablesValid |= linearizationTables[i].isValid(); //Check That All Tables are Loaded Correctly
	}

	return allTablesValid;

}

/// <summary> Linearizes the input power based on loaded data tables and the power range. The tables are setup to
///           translate a %input power to a %output power. Therefore, the current %power is calculated based on the
///           min and max provided, a %output power is retrieved using the input from the table, and a actual power
///           is returned based on the range
/// </summary>
/// <param name="minPower"> The minimum power of the selected laser channel </param>
/// <param name="maxPower"> The maximum power of the selected laser channel </param>
/// <param name="curPower"> The desired real power of the selected laser channel, assuming a linear scale between min and max </param>
/// <param name="channelIndex"> The laser channel that is being linearized </param>
/// <returns> The necessary power needed for this input channel to output the light power that would be expected if the laser light
///           scaled linearly with the input power. ie. the linearized laser power </returns>
double OTMLaser::linearizePower(double minPower, double maxPower, double curPower, int channelIndex)
{	
	double powerRange = maxPower - minPower;
	double powerPct = (curPower - minPower) / powerRange;
	double linearizedPct = linearizationTables[channelIndex].interpolate(powerPct, 0);
	double linearizedPower = linearizedPct * powerRange + minPower;
	return linearizedPower;

}


