// CameraManger.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "CameraManager.h"

auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
wchar_t message[256];

CameraManager::CameraManager()
{ 
	_initialFound = false;
}

CameraManager::~CameraManager()
{
	instanceFlag = false;

	cleanup();
}

bool CameraManager::instanceFlag = false;

long CameraManager::_dllIdFoundCounter = 1;		//1:based, 0 reserved for no device

long CameraManager::_dllIdNotFoundCounter = 1;

auto_ptr<CameraManager> CameraManager::_single(new CameraManager());

CritSect CameraManager::critSect;

CameraManager* CameraManager::getInstance()
{
	Lock lock(critSect);

	if(! instanceFlag)
	{
		try
		{
			_single.reset(new CameraManager());

			wsprintf(message,L"CameraManager Created");
			LogMessage(message, VERBOSE_EVENT);
		}
		catch(...)
		{
			//critically low on resources
			//do not proceed with application
			throw;
		}
		instanceFlag = true;
	}
	return _single.get();
}

void CameraManager::cleanup(void)
{
	//clear the dll maps
	for(_it = _cameraMap.begin(); _it != _cameraMap.end(); ++_it)
	{
		((CameraDll*)_it->second)->Uninitialize();

		wsprintf(message,L"CameraManager deleting camera object %d",_it->first);
		LogMessage(message, VERBOSE_EVENT);
	}

	for(_it = _cameraNotFoundMap.begin(); _it != _cameraNotFoundMap.end(); ++_it)
	{
		((CameraDll*)_it->second)->Uninitialize();

		wsprintf(message,L"CameraManager deleting camera object %d",_it->first);
		LogMessage(message, VERBOSE_EVENT);
	}

	_cameraMap.clear();
	_cameraNameMap.clear();
	_cameraNotFoundMap.clear();
	_cameraNotFoundNameMap.clear();
}

long CameraManager::FindCameras(const WCHAR *path, long &numCameras)
{
	Lock lock(critSect);

	long numCam = 0;

	numCameras = 0;

	//no redo dlls search to avoid memory leak because each new dll search creates a new dll pointer.
	if(_initialFound)
	{
		ReleaseCameras();

		//Reconnect the camerad found/mapped at start up.
		map<long,CameraDll*> tmpCameraMap;	
		map<long,wstring> tmpCameraNameMap;
		long tmpFoundCounter = 1;
		for(_it = _cameraMap.begin(); _it != _cameraMap.end(); ++_it)
		{
			if(TRUE == ((CameraDll*)_it->second)->FindCameras(numCam) && (0 < numCam))
			{
				for (int j = 0; j < numCam; j++)
				{
					tmpCameraMap.insert(pair<long,CameraDll*>(tmpFoundCounter,((CameraDll*)_it->second)));
					tmpCameraNameMap.insert(pair<long, wstring>(tmpFoundCounter, _cameraNameMap.at(_it->first)));
					tmpFoundCounter++;
					numCameras++;
				}
			}
			else
			{
				if (NULL != _it->second)
				{
					((CameraDll*)_it->second)->Uninitialize();
					delete ((CameraDll*)_it->second);
					(CameraDll*)_it->second = NULL;
				}
			}
		}
		_cameraMap.clear();
		_cameraNameMap.clear();
		_cameraMap = tmpCameraMap;
		_cameraNameMap = tmpCameraNameMap;
		_dllIdFoundCounter = tmpFoundCounter;

		wsprintf(message,L"CameraManager already found cameras %d", numCameras);
		LogMessage(message, VERBOSE_EVENT);

		//Look for any additional cameras that could have been connected. Update the found Cameras Map.
		map<long,CameraDll*> tmpNotFoundCameraMap;	
		map<long,wstring> tmpNotFoundCameraNameMap;
		long tmpNotFoundCounter = 1;
		for(_it = _cameraNotFoundMap.begin(); _it != _cameraNotFoundMap.end(); ++_it)
		{
			if(TRUE == ((CameraDll*)_it->second)->FindCameras(numCam) && (0 < numCam))
			{
				for(long i=0; i<numCam; i++)
				{
					_cameraMap.insert(pair<long,CameraDll*>(_dllIdFoundCounter,((CameraDll*)_it->second)));
					_cameraNameMap.insert(pair<long, wstring>(_dllIdFoundCounter, _cameraNotFoundNameMap.at(_it->first)));
					_dllIdFoundCounter++;
					numCameras++;
					wsprintf(message,L"CameraManager add camera dll %s",  _cameraNotFoundNameMap.at(_it->first).c_str());
					LogMessage(message, VERBOSE_EVENT);
				}
			}
			else
			{
				tmpNotFoundCameraMap.insert(pair<long,CameraDll*>(tmpNotFoundCounter,((CameraDll*)_it->second)));
				tmpNotFoundCameraNameMap.insert(pair<long, wstring>(tmpNotFoundCounter, _cameraNotFoundNameMap.at(_it->first)));
				tmpNotFoundCounter++;
			}
		}	
		_cameraNotFoundMap.clear();
		_cameraNotFoundNameMap.clear();
		_cameraNotFoundMap = tmpNotFoundCameraMap;
		_cameraNotFoundNameMap = tmpNotFoundCameraNameMap;
		_dllIdNotFoundCounter = tmpNotFoundCounter;

		wsprintf(message,L"CameraManager updated the number of cameras to %d", numCameras);
		LogMessage(message, VERBOSE_EVENT);
	}

	WIN32_FIND_DATA ffd;

	HANDLE hFind = NULL;

	hFind = FindFirstFile(path, &ffd);

	if(INVALID_HANDLE_VALUE == hFind)
	{
		//no files found
		return FALSE;
	}

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{

		}
		else
		{	
			WCHAR path_buffer[_MAX_PATH];
			WCHAR drive[_MAX_DRIVE];
			WCHAR dir[_MAX_DIR];
			WCHAR fname[_MAX_FNAME];
			WCHAR ext[_MAX_EXT];

			_wsplitpath_s(path,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

			_wmakepath_s(path_buffer,_MAX_PATH, drive, dir, ffd.cFileName, NULL );

			//remove dll file suffix
			wstring tempName = ffd.cFileName;
			int lastIndex = static_cast<int>(tempName.find_last_of(L"."));
			wstring dllName = tempName.substr(0, lastIndex);

			//search if already exist in maps
			bool alreadyExist = false;
			if (_initialFound)
			{
				for (long i = 1; i < _dllIdFoundCounter; i++)
				{
					if ((_cameraNameMap.end() != _cameraNameMap.find(i)) && (0 == _cameraNameMap[i].compare(dllName)))
					{
						alreadyExist = true;
						break;
					}
				}
				if(!alreadyExist)
				{
					for (long i = 1; i < _dllIdNotFoundCounter; i++)
					{
						if ((_cameraNotFoundNameMap.end() != _cameraNotFoundNameMap.find(i)) && (0 == _cameraNotFoundNameMap[i].compare(dllName)))
						{
							alreadyExist = true;
							break;
						}
					}
				}
			}

			if(alreadyExist)
				continue;

			//*NOTE* do not delete the camera dll memory. 
			//deletion of the dll will prevent 
			//the use of the functions exposed by the dll 
			CameraDll *camera = new CameraDll(path_buffer);

			if(NULL != camera)
			{
				if((true == camera->Initialize(0)) && (TRUE == camera->FindCameras(numCam)) && (numCam > 0))
				{
					for(int i=0; i < numCam; i++)
					{
						_cameraMap.insert(pair<long,CameraDll*>(_dllIdFoundCounter,camera));
						_cameraNameMap.insert(pair<long,wstring>(_dllIdFoundCounter,dllName));
						_dllIdFoundCounter++;
						numCameras++;
						wsprintf(message,L"CameraManager add camera dll %s", dllName.c_str());
						LogMessage(message, VERBOSE_EVENT);
					}
				}
				else
				{
					//If find Camera returned false, add the dll to the cameraNotFoundMap for future check in find Camera.
					//This map will be used as a reference to search if a new device was connected.
					_cameraNotFoundMap.insert(pair<long, CameraDll*>(_dllIdNotFoundCounter, camera));
					_cameraNotFoundNameMap.insert(pair<long, wstring>(_dllIdNotFoundCounter, dllName));
					_dllIdNotFoundCounter++;
					wsprintf(message,L"CameraManager add dll to list of unused dlls %s", dllName.c_str());
					LogMessage(message, VERBOSE_EVENT);
				}
			}
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);	
	FindClose(hFind);

	_initialFound = true;

	return TRUE;
}

long CameraManager::ReleaseCameras()
{
	for(_it = _cameraMap.begin(); _it != _cameraMap.end(); ++_it)
	{
		((CameraDll*)_it->second)->TeardownCamera();
	}
	return TRUE;
}

long CameraManager::GetCameraId(long i,long &id)
{
	long ret;
	//return value of id is zero if no camera is found
	id=0;
	map<long,CameraDll*>::const_iterator it;

	long size = static_cast<long>(_cameraMap.size());

	it = _cameraMap.begin();

	if(i<0)
	{
		return FALSE;
	}

	for(long j=0;j<i;j++,it++)
	{
	}

	if(it != _cameraMap.end())
	{
		id = it->first;
		ret = TRUE;
	}	
	else
	{
		ret = FALSE;
	}

	return ret;
}

ICamera* CameraManager::GetCamera(long id)
{
	ICamera * camera = NULL;	
	if(id <= _cameraMap.size())
	{
		map<long,CameraDll*>::const_iterator it;

		it = _cameraMap.find(id);

		if(it != _cameraMap.end())
		{
			camera = it->second;

			wsprintf(message,L"CameraManager GetCamera id %d found",id);
			LogMessage(message, VERBOSE_EVENT);
		}
	}
	return camera;
}

wstring CameraManager::GetCameraDllName(long id)
{
	map<long, wstring>::const_iterator itCamName;

	itCamName = _cameraNameMap.find(id);

	if(itCamName != _cameraNameMap.end())
	{
		return itCamName->second;
	}
	else
	{
		wsprintf(message,L"CameraManager GetCameraDllName did not find dll name for device %d",id);
		LogMessage(message, VERBOSE_EVENT);
		return std::wstring();
	}
}

void CameraManager::LogMessage(wchar_t *message, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, message);
#endif
}