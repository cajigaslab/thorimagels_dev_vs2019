#pragma once
#include "pch.h"
#include "DCxCameraEx.h"

class Camera1545MEx : public DCxCameraEx
{
public:
	Camera1545MEx(std::list<DCxCamera*> cameras);

	long GetSensorName(char *name);	
};

