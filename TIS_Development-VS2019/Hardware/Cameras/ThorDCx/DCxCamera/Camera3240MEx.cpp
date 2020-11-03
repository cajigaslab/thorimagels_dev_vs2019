#include "pch.h"
#include "Camera3240MEX.h"

Camera3240MEx::Camera3240MEx(std::list<DCxCamera*> cameras) : 
	DCxCameraEx(cameras)
{	
}

long Camera3240MEx::GetSensorName(char* name)
{
	memcpy(name, "SC1280G12M\0", 32);
	return TRUE;
}