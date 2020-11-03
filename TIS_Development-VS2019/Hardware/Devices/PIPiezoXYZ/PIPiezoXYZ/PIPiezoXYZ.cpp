// PIPiezoXYZ.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "PIPiezoXYZ.h"
#include "XYZPiezoSetupXML.h"
#include "Strsafe.h"

PIPiezoXYZ::PIPiezoXYZ()
{    
	_is_xonline=false;			//always flase for analog input
	_is_yonline=false;			//always flase for analog input
	_is_zonline=false;			//always flase for analog input
	_is_xservo_on=true;			//always true
	_is_yservo_on=true;			//always true
	_is_zservo_on=true;			//always true
	_deviceDetected = FALSE;
	_numDevices = 0;

	_devNameX = "/Dev1/";
	_devNameY = "/Dev1/";
	_devNameZ = "/Dev2/";

	_analogLineX = "ao0";
	_analogLineY = "ao1";
	_analogLineZ = "ao0";

	_taskHandleAO0 = NULL;
	_taskHandleCO0 = NULL;

	_taskHandleAO1 = NULL;
	_taskHandleCO1 = NULL;

	_xPos=0;
	_xPos_C=0;
	_xPos_min=0;
	_xPos_max=60;
	_xPos_home=0;
	_xPos_target=0;

	_xVol=0;
	_xVol_C=0;
	_xVol_min=0;
	_xVol_max=3;
	_xVol_home=0;
	_xVol_target=0;
	_xOpenloop_value=0;

	_xConnection_status=FALSE;
	_xServo_status=FALSE;
	_xStepsize=0;
	_xVelocity=0;

	_x_analog_mode = ANALOG_MODE_SINGLE_POINT;

	_yPos=0;
	_yPos_C=0;
	_yPos_min=0;
	_yPos_max=60;
	_yPos_home=0;
	_yPos_target=0;

	_yVol=0;
	_yVol_C=0;
	_yVol_min=0;
	_yVol_max=3;
	_yVol_home=0;
	_yVol_target=0;
	_yOpenloop_value=0;

	_yConnection_status=FALSE;
	_yServo_status=FALSE;
	_yStepsize=0;
	_yVelocity=0;

	_y_analog_mode = ANALOG_MODE_SINGLE_POINT;

	_zPos=0;
	_zPos_C=0;
	_zPos_min=0;
	_zPos_max=30;
	_zPos_home=0;
	_zPos_target=0;

	_zVol=0;
	_zVol_C=0;
	_zVol_min=0;
	_zVol_max=3;
	_zVol_home=0;
	_zVol_target=0;
	_zOpenloop_value=0;

	_zConnection_status=FALSE;
	_zServo_status=FALSE;
	_zStepsize=0;
	_zVelocity=0;

	_z_analog_mode = ANALOG_MODE_SINGLE_POINT;

	_volts2mm=0.02;

}

PIPiezoXYZ::~PIPiezoXYZ()
{
	_instanceFlag = false;
}

bool PIPiezoXYZ::_instanceFlag = false;
bool PIPiezoXYZ::_is_xonline= false;
bool PIPiezoXYZ::_is_yonline= false;
bool PIPiezoXYZ::_is_zonline= false;
bool PIPiezoXYZ::_is_xservo_on= false;
bool PIPiezoXYZ::_is_yservo_on= false;
bool PIPiezoXYZ::_is_zservo_on= false;

auto_ptr<PIPiezoXYZ> PIPiezoXYZ::_single(new PIPiezoXYZ());

wchar_t message[_MAX_PATH];

PIPiezoXYZ* PIPiezoXYZ::getInstance()
{
	if(! _instanceFlag)
	{
		_single.reset(new PIPiezoXYZ());
		_instanceFlag = true;

	}
	return _single.get();
}

long PIPiezoXYZ::FindDevices(long &deviceCount)
{
	long ret = FALSE;

	deviceCount = 1;
	//_numDevices=1;

	
	auto_ptr<XYZPiezoXML> pSetup(new XYZPiezoXML());

	double volts2mm=0.02;
	double offsetmm=0;
	double xPos_min=0;
	double xPos_max=0.06;
	double yPos_min=0;
	double yPos_max=0.06;
	double zPos_min=0;
	double zPos_max=0.06;

	if (ret=pSetup->OpenConfigFile())
	{
		string piezoXLine;
		string piezoYLine;
		string piezoZLine;
		string XanalogLine;
		string YanalogLine;
		string ZanalogLine;
		
		if(pSetup->GetIO(piezoXLine, XanalogLine, piezoYLine, YanalogLine, piezoZLine, ZanalogLine))
		{
			if(piezoXLine.length() > 0)
			{
				char devName[256];
				DAQmxGetSysDevNames(devName, 256);
				std::string temp = devName;

				// trim the string for leading and trailng slashes '/'
				if(piezoXLine.find('/') == 0)
					piezoXLine.erase(0, 1);

				if(piezoXLine.rfind('/') == piezoXLine.length()-1)
					piezoXLine.erase(piezoXLine.length()-1, 1);

				// check if specified dev name matches real device names read from NI card
				if(string::npos != temp.find(piezoXLine))
				{
					deviceCount=1;
					_numDevices=1;
					_devNameX = '/' + piezoXLine + '/';
					_analogLineX = XanalogLine;
					
					ret = TRUE;
				}

				if(piezoYLine.find('/') == 0)
					piezoYLine.erase(0, 1);

				if(piezoYLine.rfind('/') == piezoYLine.length()-1)
					piezoYLine.erase(piezoYLine.length()-1, 1);

				// check if specified dev name matches real device names read from NI card
				if(string::npos != temp.find(piezoYLine))
				{
					deviceCount=1;
					_numDevices=1;
					_devNameY = '/' + piezoYLine + '/';
					_analogLineY = YanalogLine;
					
					ret = TRUE;
				}


				if(piezoZLine.find('/') == 0)
					piezoZLine.erase(0, 1);

				if(piezoZLine.rfind('/') == piezoZLine.length()-1)
					piezoZLine.erase(piezoZLine.length()-1, 1);

				// check if specified dev name matches real device names read from NI card
				if(string::npos != temp.find(piezoZLine))
				{
					deviceCount=1;
					_numDevices=1;
					_devNameZ = '/' + piezoZLine + '/';
					_analogLineZ = ZanalogLine;
					ret = TRUE;
				}


				if (ret=pSetup->GetConversion(volts2mm,xPos_min,xPos_max,yPos_min, yPos_max,zPos_min,zPos_max))
				{
					_volts2mm=volts2mm;
					_xPos_min=xPos_min;
					_xPos_max=xPos_max;
					_yPos_min=yPos_min;
					_yPos_max=yPos_max;
					_zPos_min=zPos_min;
					_zPos_max=zPos_max;
				}
			}
			else
			{
				ret = FALSE;
			}
		}
		else
		{
			ret = FALSE;
		}
		
	}
	
	
	//ret = TRUE;	

	return ret;
}

long PIPiezoXYZ::SelectDevice(const long device)
{
	long ret = FALSE;

	switch(device)
	{
	case 0:
		{
			ret = TRUE;
			
		}
		break;
	default:
		{
		}
	}
	return ret;
}

long PIPiezoXYZ::TeardownDevice()
{
	_xPos=0;
	_yPos=0;
	_zPos=0;	
	SetAO0(_xPos);
	SetAO1(_yPos);
	SetAO2(_zPos);

	return TRUE;
}

long PIPiezoXYZ::GetParamInfo
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
			paramMin = (double)ConnectionStatusType::CONNECTION_READY;
			paramMax = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
			paramDefault = (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
		}
		break;
	case PARAM_X_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _xPos_min;
			paramMax = _xPos_max;
			paramDefault = _xPos_home;
		}
		break;
	case PARAM_Y_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _yPos_min;
			paramMax = _yPos_max;
			paramDefault = _yPos_home;
		}
		break;
	case PARAM_Z_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _zPos_min;
			paramMax = _zPos_max;
			paramDefault = _zPos_home;
		}
		break;
	case PARAM_X_POS_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _xPos_min;
			paramMax = _xPos_max;
			paramDefault = _xPos_home;
		}
		break;
	case PARAM_Y_POS_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _yPos_min;
			paramMax = _yPos_max;
			paramDefault = _yPos_home;
		}
		break;
	case PARAM_Z_POS_CURRENT:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = _zPos_min;
			paramMax = _zPos_max;
			paramDefault = _zPos_home;
		}
		break;	
	case PARAM_X_STAGE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = PIEZO;
		}
		break;
	case PARAM_Y_STAGE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = PIEZO;
		}
		break;
	case PARAM_Z_STAGE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = PIEZO;
		}
		break;
	case PARAM_X_ANALOG_MODE:	
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
	case PARAM_Y_ANALOG_MODE:	
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
	case PARAM_Z_ANALOG_MODE:	
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
	case PARAM_X_SERVO_MODE:	
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
	case PARAM_Y_SERVO_MODE:	
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
	case PARAM_Z_SERVO_MODE:	
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1;
			paramDefault = 0;
		}
		break;
	case PARAM_X_FAST_START_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _xPos_min;
			paramMax = _xPos_max;
			paramDefault = _xPos_min;
		}
		break;
	case PARAM_Y_FAST_START_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _yPos_min;
			paramMax = _yPos_max;
			paramDefault = _yPos_min;
		}
		break;
	case PARAM_Z_FAST_START_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _zPos_min;
			paramMax = _zPos_max;
			paramDefault = _zPos_min;
		}
		break;
	case PARAM_X_FAST_STOP_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _xPos_min;
			paramMax = _xPos_max;
			paramDefault = _xPos_min;
		}
		break;
	case PARAM_Y_FAST_STOP_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _yPos_min;
			paramMax = _yPos_max;
			paramDefault = _yPos_min;
		}
		break;
	case PARAM_Z_FAST_STOP_POS:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _zPos_min;
			paramMax = _zPos_max;
			paramDefault = _zPos_min;
		}
		break;
	case PARAM_X_FAST_VOLUME_TIME:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _x_fast_volume_time_min;
			paramMax = _x_fast_volume_time_max;
			paramDefault = _x_fast_volume_time_min;
		}
		break;
	case PARAM_Y_FAST_VOLUME_TIME:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _y_fast_volume_time_min;
			paramMax = _y_fast_volume_time_max;
			paramDefault = _y_fast_volume_time_min;
		}
		break;
	case PARAM_Z_FAST_VOLUME_TIME:
		{
			paramType = TYPE_DOUBLE;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _z_fast_volume_time_min;
			paramMax = _z_fast_volume_time_max;
			paramDefault = _z_fast_volume_time_min;
		}
		break;
	case PARAM_X_FAST_FLYBACK_TIME:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _x_fast_flyback_time_min;
			paramMax = _x_fast_flyback_time_max;
			paramDefault = _x_fast_flyback_time_min;
		}
		break;
	case PARAM_Y_FAST_FLYBACK_TIME:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _y_fast_flyback_time_min;
			paramMax = _y_fast_flyback_time_max;
			paramDefault = _y_fast_flyback_time_min;
		}
		break;
	case PARAM_Z_FAST_FLYBACK_TIME:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = _z_fast_flyback_time_min;
			paramMax = _z_fast_flyback_time_max;
			paramDefault = _z_fast_flyback_time_min;
		}
		break;
	case PARAM_DEVICE_TYPE:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = paramMax = paramDefault = STAGE_Z | STAGE_X | STAGE_Y;
		}
		break;
	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}


/************************************************************************************************************/

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long PIPiezoXYZ::SetParam(const long paramID, const double param)
{
	long	ret = TRUE;

	switch(paramID)
	{

	case PARAM_X_POS:
		{
			if((param >= _xPos_min) && (param <= _xPos_max))
			{
				_xPos = static_cast<double>(param);

			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_Y_POS:
		{
			if((param >= _yPos_min) && (param <= _yPos_max))
			{
				_yPos = static_cast<double>(param);

			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_Z_POS:
		{
			if((param >= _zPos_min) && (param <= _zPos_max))
			{
				_zPos = static_cast<double>(param);

			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_X_ANALOG_MODE:
		{
			if((param >= 0) && (param <= 1))

			{
				_x_analog_mode = static_cast<long>(param);

			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_Y_ANALOG_MODE:
		{
			if((param >= 0) && (param <= 1))

			{
				_y_analog_mode = static_cast<long>(param);

			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_Z_ANALOG_MODE:
		{
			if((param >= 0) && (param <= 1))

			{
				_z_analog_mode = static_cast<long>(param);

			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_X_FAST_STOP_POS:
		{
			if((param >= _xPos_min) && (param <= _xPos_max))
			{
				_x_fast_stop_pos = static_cast<double>(param);

			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_Y_FAST_STOP_POS:
		{
			if((param >= _yPos_min) && (param <= _yPos_max))
			{
				_y_fast_stop_pos = static_cast<double>(param);

			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_Z_FAST_STOP_POS:
		{
			if((param >= _zPos_min) && (param <= _zPos_max))
			{
				_z_fast_stop_pos = static_cast<double>(param);

			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_X_FAST_VOLUME_TIME:
		{
			if((param >= _x_fast_volume_time_min) && (param <= _x_fast_volume_time_max))
			{
				_x_fast_volume_time = static_cast<double>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_Y_FAST_VOLUME_TIME:
		{
			if((param >= _y_fast_volume_time_min) && (param <= _y_fast_volume_time_max))
			{
				_y_fast_volume_time = static_cast<double>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;

	case PARAM_Z_FAST_VOLUME_TIME:
		{
			if((param >= _z_fast_volume_time_min) && (param <= _z_fast_volume_time_max))
			{
				_z_fast_volume_time = static_cast<double>(param);
			}
			else
			{
				ret = FALSE;
			}
		}
		break;


	case PARAM_X_FAST_FLYBACK_TIME:
		{
			if((param >= _x_fast_flyback_time_min) && (param <= _x_fast_flyback_time_max))
			{
				if((param >= _x_fast_flyback_time_min) && (param <= _x_fast_flyback_time_max))
				{
					_x_fast_flyback_time = static_cast<double>(param);
				}
			}
		}
		break;

	case PARAM_Y_FAST_FLYBACK_TIME:
		{
			if((param >= _y_fast_flyback_time_min) && (param <= _y_fast_flyback_time_max))
			{
				if((param >= _y_fast_flyback_time_min) && (param <= _y_fast_flyback_time_max))
				{
					_y_fast_flyback_time = static_cast<double>(param);
				}
			}
		}
		break;

	case PARAM_Z_FAST_FLYBACK_TIME:
		{
			if((param >= _z_fast_flyback_time_min) && (param <= _z_fast_flyback_time_max))
			{
				if((param >= _z_fast_flyback_time_min) && (param <= _z_fast_flyback_time_max))
				{
					_z_fast_flyback_time = static_cast<double>(param);
				}
			}
		}
		break;

	case PARAM_X_OUTPUT_POCKELS_REFERENCE:
		{
			if((param == FALSE) || (param == TRUE))
			{
				_outputPockelsReference = static_cast<long>(param);;
			}
		}
		break;

	case PARAM_Y_OUTPUT_POCKELS_REFERENCE:
		{
			if((param == FALSE) || (param == TRUE))
			{
				_outputPockelsReference = static_cast<long>(param);;
			}
		}
		break;

	case PARAM_Z_OUTPUT_POCKELS_REFERENCE:
		{
			if((param == FALSE) || (param == TRUE))
			{
				_outputPockelsReference = static_cast<long>(param);;
			}
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{

			
			ret=TRUE;
		}

	default:
		
		ret = FALSE;
	}



	return ret;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long PIPiezoXYZ::GetParam(const long paramID, double &param)
{
	long	ret = TRUE;

	switch(paramID)
	{

	case PARAM_X_POS:	
		{ 
			param = static_cast<double>(_xPos); 
		}
		break;

	case PARAM_Y_POS:	
		{ 
			param = static_cast<double>(_yPos); 
		}
		break;

	case PARAM_Z_POS:	
		{ 
			param = static_cast<double>(_zPos); 
		}
		break;

	case PARAM_X_POS_CURRENT:
		{
			ReadPosition(STAGE_X,param);
		}
		break;

	case PARAM_Y_POS_CURRENT:
		{
			ReadPosition(STAGE_Y,param);
		}
		break;

	case PARAM_Z_POS_CURRENT:
		{
			ReadPosition(STAGE_Z,param);
		}
		break;

	case PARAM_X_ANALOG_MODE:
		{
		}
		param = static_cast<long>(_x_analog_mode_C);
		break;

	case PARAM_Y_ANALOG_MODE:
		{
		}
		param = static_cast<long>(_y_analog_mode_C);
		break;

	case PARAM_Z_ANALOG_MODE:
		{
		}
		param = static_cast<long>(_z_analog_mode_C);
		break;

	case PARAM_X_FAST_START_POS:
		{
			param = static_cast<double>(_x_fast_start_pos_C);
		}
		break;

	case PARAM_Y_FAST_START_POS:
		{
			param = static_cast<double>(_y_fast_start_pos_C);
		}
		break;

	case PARAM_Z_FAST_START_POS:
		{
			param = static_cast<double>(_z_fast_start_pos_C);
		}
		break;

	case PARAM_X_FAST_STOP_POS:
		{
			param = static_cast<double>(_x_fast_stop_pos_C);
		}
		break;

	case PARAM_Y_FAST_STOP_POS:
		{
			param = static_cast<double>(_y_fast_stop_pos_C);
		}
		break;

	case PARAM_Z_FAST_STOP_POS:
		{
			param = static_cast<double>(_z_fast_stop_pos_C);
		}
		break;

	case PARAM_X_FAST_VOLUME_TIME:
		{
			param = static_cast<double>(_x_fast_volume_time_C);
		}
		break;

	case PARAM_Y_FAST_VOLUME_TIME:
		{
			param = static_cast<double>(_y_fast_volume_time_C);
		}
		break;

	case PARAM_Z_FAST_VOLUME_TIME:
		{
			param = static_cast<double>(_z_fast_volume_time_C);
		}
		break;

	case PARAM_X_FAST_FLYBACK_TIME:
		{
			param = static_cast<double>(_x_fast_flyback_time_C);
		}
		break;

	case PARAM_Y_FAST_FLYBACK_TIME:
		{
			param = static_cast<double>(_y_fast_flyback_time_C);
		}
		break;

	case PARAM_Z_FAST_FLYBACK_TIME:
		{
			param = static_cast<double>(_z_fast_flyback_time_C);
		}
		break;

	case PARAM_X_OUTPUT_POCKELS_REFERENCE:
		{
			param = static_cast<double>(_outputPockelsReference);
		}
		break;

	case PARAM_Y_OUTPUT_POCKELS_REFERENCE:
		{
			param = static_cast<double>(_outputPockelsReference);
		}
		break;

	case PARAM_Z_OUTPUT_POCKELS_REFERENCE:
		{
			param = static_cast<double>(_outputPockelsReference);
		}
		break;

	case PARAM_X_STAGE_TYPE:
		{
			param = static_cast<double>(PIEZO);
		}
		break;

	case PARAM_Y_STAGE_TYPE:
		{
			param = static_cast<double>(PIEZO);
		}
		break;

	case PARAM_Z_STAGE_TYPE:
		{
			param = static_cast<double>(PIEZO);
		}
		break;	

	case PARAM_DEVICE_TYPE:
		{
			param =static_cast<double>(STAGE_Z | STAGE_Y | STAGE_X);
		}
		break;
	case PARAM_CONNECTION_STATUS:
		{
			param = (_numDevices > 0) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
			
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
long PIPiezoXYZ::SetParamBuffer(const long paramID, char * pBuffer, long size)
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
long PIPiezoXYZ::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long PIPiezoXYZ::SetParamString(const long paramID, wchar_t* str)
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
long PIPiezoXYZ::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long PIPiezoXYZ::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long PIPiezoXYZ::SetupPosition()
{
	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long PIPiezoXYZ::StartPosition()
{
	long	ret = FALSE;
	double voltage_x;
	double voltage_y;
	double voltage_z;
	double _volts2um=0.02;

	//perform position to volts conversion
	voltage_x=(_xPos)/_volts2mm;	
	voltage_y=(_yPos)/_volts2mm;
	voltage_z=(_zPos)/_volts2mm;

	if (0==SetAO0(voltage_x))
	{
		_xPos_C=_xPos;
		ret = TRUE;
	}
	if (0==SetAO1(voltage_y))
	{
		_yPos_C=_yPos;
		ret = TRUE;
	}
	if (0==SetAO2(voltage_z))
	{
		_zPos_C=_zPos;
		ret = TRUE;
	}

	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long PIPiezoXYZ::StatusPosition(long &status)
{
	status = IDevice::STATUS_READY;
	return TRUE;
}



/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long PIPiezoXYZ::PostflightPosition()
{

	return TRUE;
}

long PIPiezoXYZ::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}



/**************************************************************************************************************************************/


/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>

long PIPiezoXYZ::ReadPosition(DeviceType deviceType, double &pos)
{
	long	ret = FALSE;

	if(deviceType & (STAGE_X))
	{
		pos=_xPos_C;
		ret = TRUE;
	}
	else if(deviceType & (STAGE_Y)) 
	{
		pos=_yPos_C;
		ret = TRUE;
	}
	else if(deviceType & (STAGE_Z))
	{
		pos=_zPos_C;
		ret = TRUE;
	}
	else
	{

	}
	return ret;
}

long PIPiezoXYZ::SetAO0(double voltage)
{
	int32 error = 0;
	const float64* tmpVec;
	float64 pos;

	//These values are not used
	const double MIN_VOLTAGE = 0.0;
	const double MAX_VOLTAGE = 10.0;


	//change for accommodate DAQmx 9.8:
	try 
	{
		DAQmxStopTask(_taskHandleAO0);
		DAQmxClearTask(_taskHandleAO0);
		DAQmxErrChk(L"",error = DAQmxCreateTask("",&_taskHandleAO0));
		DAQmxErrChk(L"",error = DAQmxCreateAOVoltageChan(_taskHandleAO0, (_devNameX + _analogLineX).c_str(),"",VOLTAGE_MIN,VOLTAGE_MAX,DAQmx_Val_Volts,NULL));
		//DAQmxErrChk(L"",error = DAQmxStartTask(_taskHandleAO0));
		if ((voltage>=VOLTAGE_MIN)&&(voltage<=VOLTAGE_MAX))
		{	
			pos = static_cast<float64>(voltage);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO0, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			error = 0;
		}
		else if (voltage>VOLTAGE_MAX)
		{
			pos = static_cast<float64>(VOLTAGE_MAX);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO0, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			StringCbPrintfW(message,MAX_PATH,L"Trying to apply a voltage higher than uppper limit");
			LogMessage(message,VERBOSE_EVENT);
			StringCbPrintfW(_errMsg,MAX_PATH,L"Trying to apply a voltage higher than uppper limit, ouput has been truncated");
		}
		else
		{
			pos = static_cast<float64>(VOLTAGE_MIN);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO0, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			StringCbPrintfW(message,MAX_PATH,L"Trying to apply a voltage lower than lower limit");
			LogMessage(message,VERBOSE_EVENT);
			StringCbPrintfW(_errMsg,MAX_PATH,L"Trying to apply a voltage lower than lower limit, ouput has been truncated");
		}
		DAQmxErrChk(L"DAQmxWaitUntilTaskDone",error = DAQmxWaitUntilTaskDone(_taskHandleAO0,100));
	} catch (...) {
		if( DAQmxFailed(error)) 
		{
			StringCbPrintfW(message,MAX_PATH,L"DAQ Analog Output Error, error code %d", error);
			LogMessage(message,VERBOSE_EVENT);
			StringCbPrintfW(_errMsg,MAX_PATH,L"DAQ Analog Output Error, error code %d", error);
			return error;
		}
	}
	return error;
}


long PIPiezoXYZ::SetAO1(double voltage)
{
	int32 error = 0;
	const float64* tmpVec;
	float64 pos;

	//These values are not used
	const double MIN_VOLTAGE = 0.0;
	const double MAX_VOLTAGE = 10.0;


	//change for accommodate DAQmx 9.8:
	try 
	{
		DAQmxStopTask(_taskHandleAO1);
		DAQmxClearTask(_taskHandleAO1);
		DAQmxErrChk(L"",error = DAQmxCreateTask("",&_taskHandleAO1));
		DAQmxErrChk(L"",error = DAQmxCreateAOVoltageChan(_taskHandleAO1, (_devNameY + _analogLineY).c_str(),"",VOLTAGE_MIN,VOLTAGE_MAX,DAQmx_Val_Volts,NULL));
		//DAQmxErrChk(L"",error = DAQmxStartTask(_taskHandleAO0));
		if ((voltage>=VOLTAGE_MIN)&&(voltage<=VOLTAGE_MAX))
		{	
			pos = static_cast<float64>(voltage);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO1, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			error = 0;
		}
		else if (voltage>VOLTAGE_MAX)
		{
			pos = static_cast<float64>(VOLTAGE_MAX);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO1, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			StringCbPrintfW(message,MAX_PATH,L"Trying to apply a voltage higher than uppper limit");
			LogMessage(message,VERBOSE_EVENT);
			StringCbPrintfW(_errMsg,MAX_PATH,L"Trying to apply a voltage higher than uppper limit, ouput has been truncated");
		}
		else
		{
			pos = static_cast<float64>(VOLTAGE_MIN);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO1, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			StringCbPrintfW(message,MAX_PATH,L"Trying to apply a voltage lower than lower limit");
			LogMessage(message,VERBOSE_EVENT);
			StringCbPrintfW(_errMsg,MAX_PATH,L"Trying to apply a voltage lower than lower limit, ouput has been truncated");
		}
		DAQmxErrChk(L"DAQmxWaitUntilTaskDone",error = DAQmxWaitUntilTaskDone(_taskHandleAO1,100));
	} catch (...) {
		if( DAQmxFailed(error)) 
		{
			StringCbPrintfW(message,MAX_PATH,L"DAQ Analog Output Error, error code %d", error);
			LogMessage(message,VERBOSE_EVENT);
			StringCbPrintfW(_errMsg,MAX_PATH,L"DAQ Analog Output Error, error code %d", error);
			return error;
		}
	}
	return error;
}

long PIPiezoXYZ::SetAO2(double voltage)
{
	int32 error = 0;
	const float64* tmpVec;
	float64 pos;

	//These values are not used
	const double MIN_VOLTAGE = 0.0;
	const double MAX_VOLTAGE = 10.0;


	//change for accommodate DAQmx 9.8:
	try 
	{
		DAQmxStopTask(_taskHandleAO0);
		DAQmxClearTask(_taskHandleAO0);
		DAQmxErrChk(L"",error = DAQmxCreateTask("",&_taskHandleAO0));
		DAQmxErrChk(L"",error = DAQmxCreateAOVoltageChan(_taskHandleAO0, (_devNameZ + _analogLineZ).c_str(),"",VOLTAGE_MIN,VOLTAGE_MAX,DAQmx_Val_Volts,NULL));
		//DAQmxErrChk(L"",error = DAQmxStartTask(_taskHandleAO0));
		if ((voltage>=VOLTAGE_MIN)&&(voltage<=VOLTAGE_MAX))
		{	
			pos = static_cast<float64>(voltage);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO0, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			error = 0;
		}
		else if (voltage>VOLTAGE_MAX)
		{
			pos = static_cast<float64>(VOLTAGE_MAX);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO0, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			StringCbPrintfW(message,MAX_PATH,L"Trying to apply a voltage higher than uppper limit");
			LogMessage(message,VERBOSE_EVENT);
			StringCbPrintfW(_errMsg,MAX_PATH,L"Trying to apply a voltage higher than uppper limit, ouput has been truncated");
		}
		else
		{
			pos = static_cast<float64>(VOLTAGE_MIN);
			tmpVec = &pos;
			DAQmxErrChk(L"",error = DAQmxWriteAnalogF64(_taskHandleAO0, 1, true, 100, DAQmx_Val_GroupByChannel, tmpVec, NULL, NULL));
			StringCbPrintfW(message,MAX_PATH,L"Trying to apply a voltage lower than lower limit");
			LogMessage(message,VERBOSE_EVENT);
			StringCbPrintfW(_errMsg,MAX_PATH,L"Trying to apply a voltage lower than lower limit, ouput has been truncated");
		}
		DAQmxErrChk(L"DAQmxWaitUntilTaskDone",error = DAQmxWaitUntilTaskDone(_taskHandleAO0,100));
	} catch (...) {
		if( DAQmxFailed(error)) 
		{
			StringCbPrintfW(message,MAX_PATH,L"DAQ Analog Output Error, error code %d", error);
			LogMessage(message,VERBOSE_EVENT);
			StringCbPrintfW(_errMsg,MAX_PATH,L"DAQ Analog Output Error, error code %d", error);
			return error;
		}
	}
	return error;
}


long PIPiezoXYZ::GetControlMode(DeviceType deviceType)
{

	if(deviceType & (STAGE_X))
	{
		return _is_xonline;


	}
	else if(deviceType & (STAGE_Y)) 
	{
		return _is_yonline;

	}
	else if(deviceType & (STAGE_Z))
	{
		return _is_zonline;

	}
	else
	{
		return FALSE;
	}
}

/****************************************************************************
from here, I developed some functions for future use.
Following functions are not used this time, but it maybe useful for future applications

****************************************************************************/

void PIPiezoXYZ::SetControlMode(DeviceType deviceType,bool _control_mode)
{

	if(deviceType & (STAGE_X))
	{
		_is_xonline=_control_mode;


	}
	else if(deviceType & (STAGE_Y)) 
	{
		_is_yonline=_control_mode;

	}
	else if(deviceType & (STAGE_Z))
	{
		_is_zonline=_control_mode;

	}
	else
	{

	}
}

void PIPiezoXYZ::ToggleControlMode(DeviceType deviceType)
{

	if(deviceType & (STAGE_X))
	{
		_is_xonline=!_is_xonline;


	}
	else if(deviceType & (STAGE_Y)) 
	{
		_is_yonline=!_is_yonline;

	}
	else if(deviceType & (STAGE_Z))
	{
		_is_zonline=!_is_zonline;

	}
	else
	{

	}
}




long PIPiezoXYZ::GetServoMode(DeviceType deviceType)
{
	if(deviceType & (STAGE_X))
	{
		return _is_xservo_on;


	}
	else if(deviceType & (STAGE_Y)) 
	{
		return _is_yservo_on;

	}
	else if(deviceType & (STAGE_Z))
	{
		return _is_zservo_on;

	}
	else
	{
		return FALSE;
	}

}

void PIPiezoXYZ::SetServoMode(DeviceType deviceType, bool _servo_mode)
{
	if(deviceType & (STAGE_X))
	{
		_is_xservo_on=_servo_mode;


	}
	else if(deviceType & (STAGE_Y)) 
	{
		_is_yservo_on=_servo_mode;

	}
	else if(deviceType & (STAGE_Z))
	{
		_is_zservo_on=_servo_mode;

	}
	else
	{

	}
}

void PIPiezoXYZ::ToggleServoMode(DeviceType deviceType)
{

	if(deviceType & (STAGE_X))
	{
		_is_xservo_on=!_is_xservo_on;


	}
	else if(deviceType & (STAGE_Y)) 
	{
		_is_yservo_on=!_is_yservo_on;

	}
	else if(deviceType & (STAGE_Z))
	{
		_is_zservo_on=!_is_zservo_on;

	}
	else
	{

	}
}