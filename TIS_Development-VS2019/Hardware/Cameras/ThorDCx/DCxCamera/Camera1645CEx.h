#pragma once
#include "pch.h"
#include "DCxCameraEx.h"

class Camera1645CEx : public DCxCameraEx
{
private:
	std::list<DCxCamera*> dcxCameras;

public:
	Camera1645CEx(std::list<DCxCamera*> cameras);

	long GetSensorName(char *name);
};

