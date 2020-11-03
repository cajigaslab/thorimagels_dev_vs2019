#include "stdafx.h"
#include "math.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"

DllExportLiveImage GetLightPath(int id, long &val)
{
	long param=IDevice::PARAM_LIGHTPATH_GG;
	switch(id)
	{
	case 0:	param=IDevice::PARAM_LIGHTPATH_GG;break;
	case 1:	param=IDevice::PARAM_LIGHTPATH_GR;break;
	case 2:	param=IDevice::PARAM_LIGHTPATH_CAMERA;break;	
	}
	return GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH,param,val);
}

DllExportLiveImage SetLightPath(int id, long val)
{
	long param=IDevice::PARAM_LIGHTPATH_GG;
	switch(id)
	{
	case 0:	param=IDevice::PARAM_LIGHTPATH_GG;break;
	case 1:	param=IDevice::PARAM_LIGHTPATH_GR;break;
	case 2:	param=IDevice::PARAM_LIGHTPATH_CAMERA;break;	
	}
	return SetDeviceParamDouble(SelectedHardware::SELECTED_LIGHTPATH,param,val,FALSE);		
}

