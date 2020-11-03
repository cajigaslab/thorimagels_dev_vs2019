#include "stdafx.h"
#include "math.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"


DllExportLiveImage IsXYStageAvailable()
{	
	return TRUE;
}

DllExportLiveImage GetXPosition(double &pos)
{	
	if(FALSE == CaptureSetup::getInstance()->GetSetupFlagState())
		return TRUE;

	return GetDeviceParamDouble(SelectedHardware::SELECTED_XYSTAGE,IDevice::PARAM_X_POS_CURRENT,pos);
}

DllExportLiveImage GetYPosition(double &pos)
{	
	if(FALSE == CaptureSetup::getInstance()->GetSetupFlagState())
		return TRUE;

	return GetDeviceParamDouble(SelectedHardware::SELECTED_XYSTAGE,IDevice::PARAM_Y_POS_CURRENT,pos);
}

