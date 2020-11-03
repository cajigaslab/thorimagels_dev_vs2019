#ifndef _DCXcamera_MANAGER_H_
#define _DCXcamera_MANAGER_H_

#include "DCxCamera.h"

class CameraManager : ICamera
{
private:
	wchar_t *_cameraName;

	map<int, list<DCxCamera*>> _cameraMap;
	char* _currentModel;
	DCxCamera *_currentCamera;

	static CameraManager* _instance;
	CameraManager();

	static bool LocateDirectories(int sensorID, list<DCxCamera*> &camelist);

public:
	static CameraManager* getInstance();
	~CameraManager();

	long FindCameras(long &sensorCount);

	long SelectCamera(const long index);

	long TeardownCamera();

	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);

	long SetParam(const long paramID, const double param);

	long SetParam(const long paramID, const double param, const long channelID);

	long GetParam(const long paramID, double &param);

	long GetParam(const long paramID, double &param, const long channelID);

	long PreflightAcquisition(char * pDataBuffer);

	long SetupAcquisition(char * pDataBuffer);

	long StartAcquisition(char * pDataBuffer);

	long StatusAcquisition(long &status);

	long StatusAcquisitionEx(long &status,long &indexOfLastFrame);

	long CopyAcquisition(char * pDataBuffer, void* frameInfo);

	long PostflightAcquisition(char * pDataBuffer);

	long GetLastErrorMsg(wchar_t * errMsg, long size);

	long SetParamString(const long paramID, wchar_t * str);

	long GetParamString(const long paramID, wchar_t *str, long size);

	long SetParamBuffer(const long paramID, char * buffer, long size);

	long GetParamBuffer(const long paramID, char * buffer, long size);

};

#endif