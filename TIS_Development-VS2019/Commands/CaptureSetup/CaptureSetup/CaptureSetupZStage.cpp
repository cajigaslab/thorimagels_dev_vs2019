#include "stdafx.h"
#include "math.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"


DllExportLiveImage GetZStageName(char* &deviceName, const int maxNameLen)
{
	long selIndex = 0;
	GetDeviceSelectedIndex(SelectedHardware::SELECTED_ZSTAGE,selIndex);
	wstring name = DeviceManager::getInstance()->GetDeviceDllName(selIndex);	
	string str = ConvertWStringToString(name);
	if (maxNameLen >= (str.length() + 1) )
	{
		strcpy_s(deviceName, maxNameLen, str.c_str());
	}
	else
	{
		strcpy_s(deviceName, maxNameLen, "");
	}
	return TRUE;
}

DllExportLiveImage SetZPosition(double pos)
{	
	return SetDeviceParamDouble(SelectedHardware::SELECTED_ZSTAGE,IDevice::PARAM_Z_POS,pos,TRUE);
}

DllExportLiveImage GetZPosition(double &pos)
{	
	if((TRUE == disableZRead) || (FALSE == CaptureSetup::getInstance()->GetSetupFlagState()))
		return TRUE;

	return GetDeviceParamDouble(SelectedHardware::SELECTED_ZSTAGE,IDevice::PARAM_Z_POS_CURRENT,pos);
}

DllExportLiveImage GetZStage2Name(char* &deviceName, const int maxNameLen)
{	
	IDevice *device = NULL;

	device = GetDevice(SelectedHardware::SELECTED_ZSTAGE2);

	if(NULL == device)
	{
		strcpy_s(deviceName, maxNameLen, "");
		return FALSE;
	}

	long selIndex = 0;

	GetDeviceSelectedIndex(SelectedHardware::SELECTED_ZSTAGE2, selIndex);

	wstring name = DeviceManager::getInstance()->GetDeviceDllName(selIndex);	
	string str = ConvertWStringToString(name);
	if (maxNameLen >= (str.length() + 1) )
	{
		strcpy_s(deviceName, maxNameLen, str.c_str());
	}
	else
	{
		strcpy_s(deviceName, maxNameLen, "");
	}
	return TRUE;
}

DllExportLiveImage GetZ2Position(double &pos)
{	
	if((TRUE == disableZRead) || (FALSE == CaptureSetup::getInstance()->GetSetupFlagState()))
		return TRUE;

	return GetDeviceParamDouble(SelectedHardware::SELECTED_ZSTAGE2,IDevice::PARAM_Z_POS_CURRENT,pos);
}

