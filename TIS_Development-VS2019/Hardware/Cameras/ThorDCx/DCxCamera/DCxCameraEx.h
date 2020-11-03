#pragma once
#include "DCxCamera.h"

class DCxCameraEx : public DCxCamera
{
private:
	const char* sensorName;
	list<DCxCamera*> dcxCameras;

public:
	DCxCameraEx(list<DCxCamera*> cameras, int sensorID);

	long GetSensorName(char *name);

	long OpenCamera();

	long ExitCamera();	

	long InitCamera();	

	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);

	long SetParam(const long paramID, const double param);

	long SetParam(const long paramID, const double param, const long channelID);	

	long GetParam(const long paramID, double &param);

	long GetParam(const long paramID, double &param, const long channelID);

	long PreflightAcquisition(char* pDataBuffer);

	long SetupAcquisition(char* pDataBuffer);

	long StartAcquisition(char* pDataBuffer);

	long StatusAcquisition(long &status);

	long CopyAcquisition(char* pDataBuffer, void* frameInfo);

	long PostflightAcquisition(char* pDataBuffer);
};