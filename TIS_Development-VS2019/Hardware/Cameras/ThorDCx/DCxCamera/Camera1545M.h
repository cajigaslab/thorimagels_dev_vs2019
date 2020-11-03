#pragma once

#include "DCxCamera.h"

class Camera1545M : public DCxCamera
{
public:
	Camera1545M(INT cameraId, const char* serialNo);

	long GetUniqueParamInfo(const long paramID, double &paramMin, double &paramMax, double &paramDefault);
};

