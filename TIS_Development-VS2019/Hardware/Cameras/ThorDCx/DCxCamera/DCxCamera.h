#ifndef _DCXcamera_H_
#define _DCXcamera_H_

#include "stdafx.h"

class DCxCamera
{
private:
	INT _cameraID;
	char _serialNo[16];

	char* _imageMemory;		// image memory - pointer to buffer
	INT _imageMemoryId;			// image memory - buffer ID
	ImgPty _imgPtySetSettings; //<settings data structure	

	bool _running;

	//return IS_SUCCESS or IS_NO_SUCCESS
	long StartLive();

	//return IS_SUCCESS
	long StopLive();

	//return IS_SUCCESS or IS_NO_SUCCESS
	long ReAllocImageMemory();

	//return IS_SUCCESS or IS_NO_SUCCESS
	long GetDefaultParameters();

	//return IS_SUCCESS or IS_NO_SUCCESS
	long SetDefaultParameters();

	//return TRUE or FALSE
	long SetParameterIntoDevice();

protected:	
	Cuc480 camera;
	ImgPty imgPtyDll;			//<settings data structure

public:	
	static HANDLE eventHandle;
	static long camera_status;

	int channel;

	DCxCamera(int camera, const char* serialNo);

	~DCxCamera();

	long GetSerialNO(char* serialNo); 

	long GetSensorInfo(SENSORINFO *senserInfo);

	virtual long OpenCamera();
	 
	virtual long ExitCamera();	
	 
	virtual long InitCamera();	
	 
	virtual long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	 
	virtual long SetParam(const long paramID, const double param);

	virtual long SetParam(const long paramID, const double param, const long channelID);
	 
	virtual long GetParam(const long paramID, double &param);
	 
	virtual long GetParam(const long paramID, double &param, const long channelID);
	 
	virtual long PreflightAcquisition(char* pDataBuffer);
	 
	virtual long SetupAcquisition(char* pDataBuffer);
	 
	virtual long StartAcquisition(char* pDataBuffer);
	 
	virtual long StatusAcquisition(long &status);
	 
	virtual long CopyAcquisition(char* pDataBuffer, void* frameInfo);

	long CopyAcquisitionEx(char* pDataBuffer, long &bufferSize);
	 
	virtual long PostflightAcquisition(char* pDataBuffer);

	virtual long GetSensorName(char *name);

	virtual long GetUniqueParamInfo(const long paramID, double &paramMin, double &paramMax, double &paramDefault);

	virtual void clear();

	bool operator < (const DCxCamera& n)
    {
		return channel < n.channel;
    }
};

#endif