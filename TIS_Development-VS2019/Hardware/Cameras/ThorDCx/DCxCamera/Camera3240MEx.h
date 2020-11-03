#pragma once
#include "pch.h"
#include "DCxCameraEx.h"

class Camera3240MEx : public DCxCameraEx
{
public:
	Camera3240MEx(std::list<DCxCamera*> cameras);

	long GetSensorName(char *name);
};

