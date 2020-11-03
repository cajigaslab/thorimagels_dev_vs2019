#include "stdafx.h"
#include "Camera1645C.h"

#define IMAGE_HEIGHT_MAX		1024
#define IMAGE_HEIGHT_MIN		4
#define IMAGE_HEIGHT_STEP		2

#define IMAGE_WIDTH_MAX			1280
#define IMAGE_WIDTH_MIN			32
#define IMAGE_WIDTH_STEP		4

#define GAIN_MIN				1
#define GAIN_MAX				4.27
#define GAIN_RGB_MAX			3.1

Camera1645C::Camera1645C(INT cameraId, const char* serialNo) : 
	DCxCamera(cameraId, serialNo)
{	
}

long Camera1645C::GetUniqueParamInfo(const long paramID, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;

	switch(paramID)
	{
	case ICamera::PARAM_CAPTURE_REGION_LEFT:
	case ICamera::PARAM_CAPTURE_REGION_RIGHT:
	case ICamera::PARAM_CAMERA_IMAGE_WIDTH:
		{
			paramMin = IMAGE_WIDTH_MIN;
			paramMax = IMAGE_WIDTH_MAX;
			paramDefault = IMAGE_WIDTH_STEP;
		}
		break;

	case ICamera::PARAM_CAPTURE_REGION_TOP:
	case ICamera::PARAM_CAPTURE_REGION_BOTTOM:
	case ICamera::PARAM_CAMERA_IMAGE_HEIGHT:
		{
			paramMin = IMAGE_HEIGHT_MIN;
			paramMax = IMAGE_HEIGHT_MAX;
			paramDefault = IMAGE_HEIGHT_STEP;
		}
		break;

	case ICamera::PARAM_GAIN:
		{
			paramMin = GAIN_MIN;
			paramMax = GAIN_MAX;
			paramDefault = GAIN_MIN;
		}
		break;
	default:
		{
			paramMin = 0;
			paramMax = 0;
			paramDefault = 0;
			ret = FALSE;
		}
	}

	return ret;
}
