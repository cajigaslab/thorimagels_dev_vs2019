#pragma once

#include "DCxCamera.h"

class Camera3240M : public DCxCamera
{
public:
	Camera3240M(INT camera, const char* serialNo);

	long GetUniqueParamInfo(const long paramID, double &paramMin, double &paramMax, double &paramDefault);
};

