#include "stdafx.h"
#include "math.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"



DllExportLiveImage GetRPosition(double &pos)
{	
	if(FALSE == CaptureSetup::getInstance()->GetSetupFlagState())
		return TRUE;
		
	return GetDeviceParamDouble(SelectedHardware::SELECTED_RSTAGE,IDevice::PARAM_R_POS_CURRENT,pos);
}
