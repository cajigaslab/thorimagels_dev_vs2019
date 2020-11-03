#include "stdafx.h"
#include "DCxCameraEx.h"

DCxCameraEx::DCxCameraEx(list<DCxCamera*> cameras, int sensorID):
	DCxCamera(0, NULL)
{	
	dcxCameras.clear();

	for (list<DCxCamera*>::iterator it = cameras.begin(); it != cameras.end(); it++)
	{
		dcxCameras.push_back(*it);
	}		

	sensorName = "\0";

	switch (sensorID)
	{
	case IS_SENSOR_C1285R12M:
		sensorName = "C1285R12M\0";
		break;
	case IS_SENSOR_C1280G12M:
		sensorName = "C1280G12M\0";
		break;
	case IS_SENSOR_C1284R13C:
		sensorName = "C1284R13C\0";
		break;
	default:
		break;
	}
}

long DCxCameraEx::GetSensorName(char* name)
{		
	memcpy(name, sensorName, 32);
	return TRUE;
}

long DCxCameraEx::OpenCamera()
{
	for (list<DCxCamera*>::iterator it = dcxCameras.begin(); it != dcxCameras.end(); it++)
	{
		if (IS_NO_SUCCESS ==(*it)->OpenCamera())
			return IS_NO_SUCCESS;
	}

	return IS_SUCCESS;
}

long DCxCameraEx::ExitCamera()
{
	for (list<DCxCamera*>::iterator it = dcxCameras.begin(); it != dcxCameras.end(); it++)
	{
		if (IS_NO_SUCCESS ==(*it)->ExitCamera())
			return IS_NO_SUCCESS;
	}

	return IS_SUCCESS;
}

long DCxCameraEx::InitCamera()
{
	for (list<DCxCamera*>::iterator it = dcxCameras.begin(); it != dcxCameras.end(); it++)
	{
		if (IS_NO_SUCCESS ==(*it)->InitCamera())
			return IS_NO_SUCCESS;
	}

	return IS_SUCCESS;
}

long DCxCameraEx::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	list<DCxCamera*>::iterator it = dcxCameras.begin();
	if (it == dcxCameras.end())
		return IS_NO_SUCCESS;
	if (paramID == ICamera::PARAM_CAMERA_CHANNEL)
	{
		if (FALSE == (*it)->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
			return IS_NO_SUCCESS;
		paramMin = dcxCameras.size() * paramMin;
		paramMax = dcxCameras.size() * paramMax;
		paramDefault = dcxCameras.size() * paramDefault;
		return IS_SUCCESS;
	}
	else
	{
		return (TRUE == (*it)->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault) ? IS_SUCCESS : IS_NO_SUCCESS);
	}
}

long DCxCameraEx::SetParam(const long paramID, const double param)
{
	// when set channel enable

	// For colorful cameras
	// cameraIndex******4th**3rd**2nd**1st*************
	// (long)param = 0x 100  010  101  011 
	// setparam*********Bgr**bGr**BgR**bGR*************	//lower means disable, upper means enable

	// For simple cameras
	// cameraIndex****4th**3rd**2nd**1st*************
	// channelID = 0x  1    0    1    1  
	// setparam********T****F****T****T**************	//T means enable, F means disable
	list<DCxCamera*>::iterator it = dcxCameras.begin();
	if (it == dcxCameras.end()) return FALSE;

	SENSORINFO sInfo;
	(*it)->GetSensorInfo(&sInfo);

	int judgeFlag = 0x1;
	int judgeSize = 1;
	if (sInfo.nColorMode == IS_COLORMODE_BAYER)
	{
		judgeFlag = 0x111;
		judgeSize = 3;
	}

	double dParam = param;
	int index = 0;

	for (list<DCxCamera*>::iterator it = dcxCameras.begin(); it != dcxCameras.end(); it++, index++)
	{
		if (paramID == ICamera::PARAM_CAMERA_CHANNEL)
		{
			long lParam = static_cast<long>(param);
			dParam = static_cast<double>((lParam >> (index * judgeSize)) & judgeFlag);
		}
		if (IS_NO_SUCCESS == (*it)->SetParam(paramID, dParam))
			return IS_NO_SUCCESS;
	}

	return IS_SUCCESS;
}

long DCxCameraEx::SetParam(const long paramID, const double param, const long channelID)
{
	// For colorful cameras
	// cameraIndex****4th**3rd**2nd**1st*************
	// channelID = 0x 111  000  101  010 
	// setparam********T****F****T****T**************	//T means true, F means false

	// For simple cameras
	// cameraIndex****4th**3rd**2nd**1st*************
	// channelID = 0x  1    0    1    1  
	// setparam********T****F****T****T**************	//T means true, F means false

	list<DCxCamera*>::iterator it = dcxCameras.begin();
	if (it == dcxCameras.end()) return FALSE;

	SENSORINFO sInfo;
	(*it)->GetSensorInfo(&sInfo);

	int judgeFlag = 0x1;
	int judgeSize = 1;
	if (sInfo.nColorMode == IS_COLORMODE_BAYER)
	{
		judgeFlag = 0x111;
		judgeSize = 3;
	}

	double dParam = param;
	int index = 0;

	for (; it != dcxCameras.end(); it++, index++)
	{
		long result = (channelID >> (index * judgeSize)) & (judgeFlag);
		if (result != 0x0)
		{
			if (IS_NO_SUCCESS == (*it)->SetParam(paramID, dParam))
				return IS_NO_SUCCESS;
		}
	}
	return IS_SUCCESS;
}

long DCxCameraEx::GetParam(const long paramID, double &param)
{
	list<DCxCamera*>::iterator it = dcxCameras.begin();
	if (it == dcxCameras.end())	return IS_NO_SUCCESS;

	if (paramID == ICamera::PARAM_CAMERA_CHANNEL)
	{
		int result = 0;

		SENSORINFO sInfo;
		(*it)->GetSensorInfo(&sInfo);
		int judgeSize = sInfo.nColorMode == IS_COLORMODE_BAYER ? 3 : 1;
		int index = 0;
		for (; it != dcxCameras.end(); it++, index++)
		{
			double paramIn;
			if (FALSE == (*it)->GetParam(paramID, paramIn))
				return IS_NO_SUCCESS;

			result += ((int)paramIn) << (index * judgeSize);
		}
		param = (double)result;
		return IS_SUCCESS;
	}
	else
	{
		return (TRUE == (*it)->GetParam(paramID, param) ? IS_SUCCESS : IS_NO_SUCCESS);
	}	
}

long DCxCameraEx::GetParam(const long paramID, double &param, const long channelID)
{
	// cameraIndex****4th**3rd**2nd**1st*************
	// channelID = 0x 100  010  000  001 
	// getparam********T****T****F****T**************	//T means true, F means false
	// only get the param that channelID first matched, from 1st -> last
	int index = 0;

	for (list<DCxCamera*>::iterator it = dcxCameras.begin(); it != dcxCameras.end(); it++, index++)
	{
		long result = (channelID >> (index * 3)) & (0x111);
		if (result != 0x0)
		{
			return (TRUE == (*it)->GetParam(paramID, param) ? IS_SUCCESS : IS_NO_SUCCESS);
		}
	}
	return IS_NO_SUCCESS;
}

long DCxCameraEx::PreflightAcquisition(char* pDataBuffer)
{
	for (list<DCxCamera*>::iterator it = dcxCameras.begin(); it != dcxCameras.end(); it++)
	{
		if (IS_NO_SUCCESS == (*it)->PreflightAcquisition(pDataBuffer))
			return IS_NO_SUCCESS;
	}

	return IS_SUCCESS;
}

long DCxCameraEx::SetupAcquisition(char* pDataBuffer)
{
	for (list<DCxCamera*>::iterator it = dcxCameras.begin(); it != dcxCameras.end(); it++)
	{
		if (IS_NO_SUCCESS == (*it)->SetupAcquisition(pDataBuffer))
			return IS_NO_SUCCESS;
	}

	return IS_SUCCESS;
}

long DCxCameraEx::StartAcquisition(char* pDataBuffer)
{
	// need Synchronize
	for (list<DCxCamera*>::iterator it = dcxCameras.begin(); it != dcxCameras.end(); it++)
	{
		if (IS_NO_SUCCESS == (*it)->StartAcquisition(pDataBuffer))
			return IS_NO_SUCCESS;
	}

	return IS_SUCCESS;
}

long DCxCameraEx::StatusAcquisition(long &status)
{
	for (list<DCxCamera*>::iterator it = dcxCameras.begin(); it != dcxCameras.end(); it++)
	{
		if (IS_NO_SUCCESS == (*it)->StatusAcquisition(status) || status == ICamera::STATUS_BUSY)
			return IS_NO_SUCCESS;
	}

	return IS_SUCCESS;
}

long DCxCameraEx::CopyAcquisition(char* pDataBuffer, void* frameInfo)
{
	long bufferoffset = 0;
	for (list<DCxCamera*>::iterator it = dcxCameras.begin(); it != dcxCameras.end(); it++)
	{
		if (IS_NO_SUCCESS == (*it)->CopyAcquisitionEx(pDataBuffer, bufferoffset))
			return IS_NO_SUCCESS;

		pDataBuffer = pDataBuffer + bufferoffset;		//make pointer to the offset location
	}

	return IS_SUCCESS;
}
	
long DCxCameraEx::PostflightAcquisition(char* pDataBuffer)
{
	for (list<DCxCamera*>::iterator it = dcxCameras.begin(); it != dcxCameras.end(); it++)
	{
		if (IS_NO_SUCCESS == (*it)->PostflightAcquisition(pDataBuffer))
			return IS_NO_SUCCESS;
	}

	return IS_SUCCESS;
}