#include "stdafx.h"
#include "CameraManager.h"
#include "DCxCameraEx.h"
#include "Camera1545M.h"
#include "Camera3240M.h"
#include "Camera1645C.h"

CameraManager* CameraManager::_instance = NULL;

CameraManager::CameraManager()
{	
	_cameraMap.clear();
	_currentCamera = NULL;
	_currentModel = NULL;
}

CameraManager::~CameraManager()
{	
	_cameraMap.clear();
	_currentCamera = NULL;
	_currentModel = NULL;
}

CameraManager* CameraManager::getInstance()
{
	if (_instance == NULL)
		_instance = new CameraManager();
	return _instance;
}

bool CameraManager::LocateDirectories(int sensorID, list<DCxCamera*> &camelist)
{
	const string file = ".\\Modules_Native\\DCxCamera.xml";
	auto_ptr<ticpp::Document> obj(new ticpp::Document(file));

	try
	{
		obj->LoadFile();
	}
	catch(ticpp::Exception ex)
	{
		return false;
	}

	ticpp::Element *configObj = obj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return false;
	}

	bool target_cameras_found = false;

	ticpp::Iterator< ticpp::Element > cameras(configObj, "DCxCameras");
	for (cameras = cameras.begin( configObj ); cameras != cameras.end(); cameras++)
	{			
		string sensor;
		cameras->GetAttribute("SensorID", &sensor);
		int camera_sensor_id = -1;
		stringstream sssensor(sensor);
		sssensor >> camera_sensor_id;

		if (sensorID == camera_sensor_id)
		{
			target_cameras_found = true;

			ticpp::Iterator< ticpp::Element > camera(&(*cameras), "DCxCamera");			
			int extend_index = 1024;		//it's impossible number for camera channel in cameralist

			for (list<DCxCamera*>::iterator it = camelist.begin(); it != camelist.end(); it++)
			{
				char* serial_no = new char[SERIAL_NUMBER_MAX_LENGHT];
				memset(serial_no, 0, SERIAL_NUMBER_MAX_LENGHT);
				if (!(*it)->GetSerialNO(serial_no))
					return false;

				bool camera_found = false;

				for (camera = camera.begin(&(*cameras)); camera != camera.end(); camera++)
				{
					string serial;				
					camera->GetAttribute("SerialNo", &serial);
					char* serial_attr = new char[SERIAL_NUMBER_MAX_LENGHT];
					memcpy(serial_attr, serial.c_str(), SERIAL_NUMBER_MAX_LENGHT);				

					//serial number is found, deal with then break
					if (strcmp(serial_attr, serial_no) == 0)
					{
						string channel;
						camera->GetAttribute("Channel", &channel);
						int camerachannel = -1;
						stringstream sschannel(channel);
						sschannel >> camerachannel;
						(*it)->channel = camerachannel;

						camera_found = true;
						break;
					}
				}	

				if (camera_found == false)
				{
					(*it)->channel = extend_index;
					extend_index++;
				}
			}

			camelist.sort();
			break;
		}
	}
	
	if (target_cameras_found)
		configObj->RemoveChild(&(*cameras));

	//create element and add camera into.
	{
		ticpp::Element * elements = new ticpp::Element("DCxCameras");

		elements->SetAttribute("SensorID", sensorID);

		configObj->LinkEndChild(elements);

		int channel = 0;
		for (list<DCxCamera*>::iterator it = camelist.begin(); it != camelist.end(); it++, channel++)
		{
			char* serial_no = new char[16];
			memset(serial_no, 0, SERIAL_NUMBER_MAX_LENGHT);
			if (!(*it)->GetSerialNO(serial_no))
				return false;

			(*it)->channel = channel;

			ticpp::Element * element = new ticpp::Element("DCxCamera");
			element->SetAttribute("SerialNo", serial_no);
			element->SetAttribute("Channel", channel);
			elements->LinkEndChild(element);
		}
	}

	obj->SaveFile(file);

	return true;
}

long CameraManager::FindCameras(long &sensorCount)
{
	int cameraNum;
	if (is_GetNumberOfCameras(&cameraNum) != IS_SUCCESS)
		return FALSE;

	UC480_CAMERA_LIST cameraList;
	cameraList.dwCount = (ULONG)cameraNum;

	if (is_GetCameraList(&cameraList) != IS_SUCCESS)
		return FALSE;

	for (int i = 0; i < cameraNum; i++)
	{
		UC480_CAMERA_INFO info = cameraList.uci[i];
		int sensor = info.dwSensorID;
		DCxCamera *camera = NULL;
		switch (sensor)
		{
		case IS_SENSOR_C1285R12M:
			camera = new Camera1545M( i + 1, info.SerNo);
			break;
		case IS_SENSOR_C1280G12M:
			camera = new Camera3240M( i + 1, info.SerNo);
			break;
		case IS_SENSOR_C1284R13C:
			camera = new Camera1645C( i + 1, info.SerNo);
			break;
		default:
			break;
		}

		if (camera == NULL)
			continue;

		map<int, list<DCxCamera*>>::iterator m_it = _cameraMap.find(sensor);
		if(m_it == _cameraMap.end())
		{
			list<DCxCamera*> cameraList;
			cameraList.push_back(camera);
			_cameraMap.insert(make_pair(sensor, cameraList));			
		}
		else
		{
			m_it->second.push_back(camera);
		}
	}

	sensorCount = (long)(_cameraMap.size());

	return TRUE;
}

long CameraManager::SelectCamera(const long index)
{
	if (index < 0 || index >= (const long)(_cameraMap.size())) return FALSE;

	map<int, list<DCxCamera*>>::iterator m_it = _cameraMap.begin();
	long index_map = index;
	while (index_map-- > 0)
	{
		m_it++;
	}

	list<DCxCamera*> cameras = m_it->second;
	int sensor = m_it->first;
	LocateDirectories(sensor, cameras);	//in this function judge and arrange the camera list.

	_currentCamera = new DCxCameraEx(cameras, sensor);

	_currentModel = new char[32];
	_currentCamera->GetSensorName(_currentModel);

	if (IS_NO_SUCCESS == _currentCamera->OpenCamera() || IS_NO_SUCCESS == _currentCamera->InitCamera())
		return FALSE;

	return TRUE;
}

long CameraManager::TeardownCamera()
{
	if (IS_NO_SUCCESS == _currentCamera->ExitCamera())
		return FALSE;	

	_currentCamera = NULL;
	_currentModel = NULL;

	return TRUE;
}

long CameraManager::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	if (IS_NO_SUCCESS == _currentCamera->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
		return FALSE;
	return TRUE;
}
long CameraManager::SetParam(const long paramID, const double param)
{
	if (IS_NO_SUCCESS == _currentCamera->SetParam(paramID, param))
		return FALSE;

	return TRUE;
}

long CameraManager::SetParam(const long paramID, const double param, const long channelID)
{
	if (IS_NO_SUCCESS == _currentCamera->SetParam(paramID, param, channelID))
		return FALSE;

	return TRUE;
}

long CameraManager::GetParam(const long paramID, double &param)
{
	if (IS_NO_SUCCESS == _currentCamera->GetParam(paramID, param))
		return FALSE;

	return TRUE;
}

long CameraManager::GetParam(const long paramID, double &param, const long channelID)
{
	if (IS_NO_SUCCESS == _currentCamera->GetParam(paramID, param, channelID))
		return FALSE;

	return TRUE;
}

long CameraManager::PreflightAcquisition(char * pDataBuffer)
{
	if (IS_NO_SUCCESS == _currentCamera->PreflightAcquisition(pDataBuffer))
		return FALSE;

	return TRUE;
}

long CameraManager::SetupAcquisition(char * pDataBuffer)
{
	if (IS_NO_SUCCESS == _currentCamera->SetupAcquisition(pDataBuffer))
		return FALSE;

	return TRUE;
}

long CameraManager::StartAcquisition(char * pDataBuffer)
{
	if (IS_NO_SUCCESS == _currentCamera->StartAcquisition(pDataBuffer))
		return FALSE;

	return TRUE;
}

long CameraManager::StatusAcquisition(long &status)
{
	if (IS_NO_SUCCESS == _currentCamera->StatusAcquisition(status))
		return FALSE;

	return TRUE;
}

long CameraManager::StatusAcquisitionEx(long &status,long &indexOfLastFrame)
{
	return TRUE;
}

long CameraManager::CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	size_t length = strlen(pDataBuffer);
	if (IS_NO_SUCCESS == _currentCamera->CopyAcquisition(pDataBuffer, frameInfo))
		return FALSE;

	return TRUE;
}

long CameraManager::PostflightAcquisition(char * pDataBuffer)
{
	if (IS_NO_SUCCESS == _currentCamera->PostflightAcquisition(pDataBuffer))
		return FALSE;

	return TRUE;
}

long CameraManager::GetLastErrorMsg(wchar_t * errMsg, long size)
{
	return TRUE;
}

long CameraManager::SetParamString(const long paramID, wchar_t * str)
{
	switch(paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME :
		{
			rsize_t _cameraNameLen = (wcsnlen (str, 32) + 1);

			if (NULL != _cameraName) {
				free (_cameraName);
			}

			_cameraName = (wchar_t *) malloc (_cameraNameLen * sizeof (wchar_t));
			if (NULL != _cameraName) {
				wcsncpy_s (_cameraName, _cameraNameLen, str, _cameraNameLen);
			}

		}
		break;
	}

	return TRUE;
}

long CameraManager::GetParamString(const long paramID, wchar_t *str, long size)
{
	switch(paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME :

		if (NULL != _cameraName) {
			size_t _cameraNameLen = (wcsnlen (_cameraName, 32) + 1);
			wcsncpy_s (str, _cameraNameLen, _cameraName, _cameraNameLen);
		}
		else
		{
			wcsncpy_s(str, 10, L"DCxCamera\0", 10);
		}

		break;

	case ICamera::PARAM_DETECTOR_SERIAL_NUMBER :
		{
			size_t SerNoLen = strnlen (_currentModel, 32) + 1;

			size_t i;
			for (i = 0; i < SerNoLen; i++) {
				str [i] = _currentModel[i];
			}

			str[i] = '\0';
		}
		break;
	}

	return TRUE;
}

long CameraManager::SetParamBuffer(const long paramID, char * buffer, long size)
{
	return TRUE;
}

long CameraManager::GetParamBuffer(const long paramID, char * buffer, long size)
{
	return TRUE;
}