#include "pch.h"
#include "Camera1645CEx.h"

Camera1645CEx::Camera1645CEx(std::list<DCxCamera*> cameras) : 
	DCxCameraEx(cameras)
{	
}

long Camera1645CEx::GetSensorName(char* name)
{
	memcpy(name, "C1284R13C\0", 32);
	return TRUE;
}