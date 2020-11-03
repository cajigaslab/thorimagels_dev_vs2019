#pragma once

#include "DCxCamera.h"

class Camera1645C : public DCxCamera
{
public:
	Camera1645C(INT camera, const char* serialNo);

	long GetUniqueParamInfo(const long paramID, double &paramMin, double &paramMax, double &paramDefault);
};

