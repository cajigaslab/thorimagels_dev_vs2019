#include "pch.h"
#include "Camera1545MEx.h"

Camera1545MEx::Camera1545MEx(std::list<DCxCamera*> cameras) : 
	DCxCameraEx(cameras)
{	
}

long Camera1545MEx::GetSensorName(char* name)
{
	memcpy(name, "C1285R12M\0", 32);
	return TRUE;
}