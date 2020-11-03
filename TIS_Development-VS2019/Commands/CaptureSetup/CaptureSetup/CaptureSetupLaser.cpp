#include "stdafx.h"
#include "math.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"


DllExportLiveImage GetMCLSLaserRange(long id, double &lMin, double &lMax)
{

	long ret = FALSE;

	double lDefault;

	switch(id)
	{
	case 0:	ret = GetDeviceParamRangeDouble(SelectedHardware::SELECTED_LASER1,IDevice::PARAM_LASER1_POWER,lMin,lMax,lDefault);break;
	case 1:	ret = GetDeviceParamRangeDouble(SelectedHardware::SELECTED_LASER2,IDevice::PARAM_LASER1_POWER,lMin,lMax,lDefault);break;
	case 2:	ret = GetDeviceParamRangeDouble(SelectedHardware::SELECTED_LASER3,IDevice::PARAM_LASER1_POWER,lMin,lMax,lDefault);break;
	case 3:	ret = GetDeviceParamRangeDouble(SelectedHardware::SELECTED_LASER4,IDevice::PARAM_LASER1_POWER,lMin,lMax,lDefault);break;
	}

	return ret;

}

DllExportLiveImage GetMCLSLaserPower(long id, double &power)
{	
	long ret = FALSE;

	switch(id)
	{
	case 0:	ret = GetDeviceParamDouble(SelectedHardware::SELECTED_LASER1,IDevice::PARAM_LASER1_POWER_CURRENT,power);break;
	case 1:	ret = GetDeviceParamDouble(SelectedHardware::SELECTED_LASER2,IDevice::PARAM_LASER2_POWER_CURRENT,power);break;
	case 2:	ret = GetDeviceParamDouble(SelectedHardware::SELECTED_LASER3,IDevice::PARAM_LASER3_POWER_CURRENT,power);break;
	case 3:	ret = GetDeviceParamDouble(SelectedHardware::SELECTED_LASER4,IDevice::PARAM_LASER4_POWER_CURRENT,power);break;
	}

	return ret;
}

DllExportLiveImage SetMCLSLaserPower(long id, double power)
{	
	long ret = FALSE;

	switch(id)
	{
	case 0:	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1,IDevice::PARAM_LASER1_POWER,power,FALSE);break;
	case 1:	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_LASER2,IDevice::PARAM_LASER2_POWER,power,FALSE);break;
	case 2:	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_LASER3,IDevice::PARAM_LASER3_POWER,power,FALSE);break;
	case 3:	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_LASER4,IDevice::PARAM_LASER4_POWER,power,FALSE);break;
	}

	return ret;
}

DllExportLiveImage GetMCLSLaserEnable(long id, long &enable)
{	
	long ret = FALSE;

	switch(id)
	{
	case 0:	ret = GetDeviceParamLong(SelectedHardware::SELECTED_LASER1,IDevice::PARAM_LASER1_ENABLE,enable);break;
	case 1:	ret = GetDeviceParamLong(SelectedHardware::SELECTED_LASER2,IDevice::PARAM_LASER2_ENABLE,enable);break;
	case 2:	ret = GetDeviceParamLong(SelectedHardware::SELECTED_LASER3,IDevice::PARAM_LASER3_ENABLE,enable);break;
	case 3:	ret = GetDeviceParamLong(SelectedHardware::SELECTED_LASER4,IDevice::PARAM_LASER4_ENABLE,enable);break;
	}

	return ret;
}

DllExportLiveImage SetMCLSLaserEnable(long id, long pos)
{	
	long ret = FALSE;

	switch(id)
	{
	case 0:	ret = SetDeviceParamLong(SelectedHardware::SELECTED_LASER1,IDevice::PARAM_LASER1_ENABLE,pos,FALSE);break;
	case 1:	ret = SetDeviceParamLong(SelectedHardware::SELECTED_LASER2,IDevice::PARAM_LASER2_ENABLE,pos,FALSE);break;
	case 2:	ret = SetDeviceParamLong(SelectedHardware::SELECTED_LASER3,IDevice::PARAM_LASER3_ENABLE,pos,FALSE);break;
	case 3:	ret = SetDeviceParamLong(SelectedHardware::SELECTED_LASER4,IDevice::PARAM_LASER4_ENABLE,pos,FALSE);break;
	}

	return ret;
}

DllExportLiveImage GetLaserShutterPosition(long &pos)
{
	return GetDeviceParamLong(SelectedHardware::SELECTED_LASER1,IDevice::PARAM_LASER1_SHUTTER_POS_CURRENT,pos);
}

DllExportLiveImage SetLaserShutterPosition(long pos)
{
	return SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1,IDevice::PARAM_LASER1_SHUTTER_POS,pos,FALSE);
}

DllExportLiveImage SetLaserPosition(long id, long pos)
{	

	long ret = FALSE;

	switch(id)
	{
	case 0:	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1,IDevice::PARAM_LASER1_POS,pos,FALSE);break;
	case 1:	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_LASER2,IDevice::PARAM_LASER2_POS,pos,FALSE);break;
	case 2:	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_LASER3,IDevice::PARAM_LASER3_POS,pos,FALSE);break;
	case 3:	ret = SetDeviceParamDouble(SelectedHardware::SELECTED_LASER4,IDevice::PARAM_LASER4_POS,pos,FALSE);break;
	}

	return ret;
}

DllExportLiveImage GetLaserPosition(long id, long &pos)
{	
	long ret = FALSE;

	switch(id)
	{
	case 0:	ret = GetDeviceParamLong(SelectedHardware::SELECTED_LASER1,IDevice::PARAM_LASER1_POS_CURRENT,pos);break;
	case 1:	ret = GetDeviceParamLong(SelectedHardware::SELECTED_LASER2,IDevice::PARAM_LASER2_POS_CURRENT,pos);break;
	case 2:	ret = GetDeviceParamLong(SelectedHardware::SELECTED_LASER3,IDevice::PARAM_LASER3_POS_CURRENT,pos);break;
	case 3:	ret = GetDeviceParamLong(SelectedHardware::SELECTED_LASER4,IDevice::PARAM_LASER4_POS_CURRENT,pos);break;
	}

	return ret;
}