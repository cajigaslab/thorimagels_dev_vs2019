#include "stdafx.h"
#include "SlmManager.h"

SlmManager::SlmManager()
{
	_initialFound = false;
}

SlmManager::~SlmManager()
{
	_instanceFlag = false;

	cleanup();
}

wchar_t msgInfo[MSG_SIZE];

bool SlmManager::_instanceFlag = false;

long SlmManager::_dllIdFoundCounter = 1;		//1:based, 0 reserved for no slm

long SlmManager::_dllIdNotFoundCounter = 1;

std::unique_ptr<SlmManager> SlmManager::_single(new SlmManager());

CritSect SlmManager::_critSect;

SlmManager* SlmManager::getInstance()
{
	Lock lock(_critSect);

	if (!_instanceFlag)
	{
		try
		{
			_single.reset(new SlmManager());
		}
		catch (...)
		{
			throw;
		}
		_instanceFlag = true;
	}
	return _single.get();
}

void SlmManager::cleanup(void)
{
	for (_it = _slmMap.begin(); _it != _slmMap.end(); ++_it)
	{
		((SLMDll*)_it->second)->Uninitialize();
		delete ((SLMDll*)_it->second);
	}

	for (_it = _slmNotFoundMap.begin(); _it != _slmNotFoundMap.end(); ++_it)
	{
		((SLMDll*)_it->second)->Uninitialize();
		delete ((SLMDll*)_it->second);
	}

	_slmMap.clear();
	_slmNameMap.clear();
	_slmNotFoundMap.clear();
	_slmNotFoundNameMap.clear();
}

long SlmManager::FindSLMs(char* xml)
{
	Lock lock(_critSect);

	long numSLMs = 0;
	//no redo dlls search to avoid memory leak because each new dll search creates a new dll pointer.
	if (_initialFound)
	{
		ReleaseSLMs();

		//Reconnect the devices found/mapped at start up.
		map<long, SLMDll*> tmpDeviceMap;
		map<long, wstring> tmpDeviceNameMap;
		long tmpFoundCounter = 1;
		for (_it = _slmMap.begin(); _it != _slmMap.end(); ++_it)
		{
			if (0 < ((SLMDll*)_it->second)->FindSLM(xml))
			{
				tmpDeviceMap.insert(pair<long, SLMDll*>(tmpFoundCounter, ((SLMDll*)_it->second)));
				tmpDeviceNameMap.insert(pair<long, wstring>(tmpFoundCounter, _slmNameMap.at(_it->first)));
				tmpFoundCounter++;
				numSLMs++;
			}
			else
			{
				if (NULL != _it->second)
				{
					((SLMDll*)_it->second)->Uninitialize();
					delete ((SLMDll*)_it->second);
					(SLMDll*)_it->second = NULL;
				}
			}
		}
		_slmMap.clear();
		_slmNameMap.clear();
		_slmMap = tmpDeviceMap;
		_slmNameMap = tmpDeviceNameMap;
		_dllIdFoundCounter = tmpFoundCounter;

		//Look for any additional devices that could have been connected. Update the found Devices Map.
		map<long, SLMDll*> tmpNotFoundDeviceMap;
		map<long, wstring> tmpNotFoundDeviceNameMap;
		long tmpNotFoundCounter = 1;
		for (_it = _slmNotFoundMap.begin(); _it != _slmNotFoundMap.end(); ++_it)
		{
			if (0 < ((SLMDll*)_it->second)->FindSLM(xml))
			{
				_slmMap.insert(pair<long, SLMDll*>(_dllIdFoundCounter, ((SLMDll*)_it->second)));
				_slmNameMap.insert(pair<long, wstring>(_dllIdFoundCounter, _slmNotFoundNameMap.at(_it->first)));
				_dllIdFoundCounter++;
				wsprintf(msgInfo, L"SlmManager add SLM dll %s", _slmNotFoundNameMap.at(_it->first).c_str());
				LogMessage(msgInfo, VERBOSE_EVENT);
			}
			else
			{
				tmpNotFoundDeviceMap.insert(pair<long, SLMDll*>(tmpNotFoundCounter, ((SLMDll*)_it->second)));
				tmpNotFoundDeviceNameMap.insert(pair<long, wstring>(tmpNotFoundCounter, _slmNotFoundNameMap.at(_it->first)));
				tmpNotFoundCounter++;
			}
		}
		_slmNotFoundMap.clear();
		_slmNotFoundNameMap.clear();
		_slmNotFoundMap = tmpNotFoundDeviceMap;
		_slmNotFoundNameMap = tmpNotFoundDeviceNameMap;
		_dllIdNotFoundCounter = tmpNotFoundCounter;

	}

	//retrieve the application directory
	wchar_t appDir[_MAX_PATH];
	WCHAR path_buffer[_MAX_PATH];
	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	WCHAR fname[_MAX_FNAME];
	WCHAR ext[_MAX_EXT];

	GetModuleFileNameW(NULL, appDir, _MAX_PATH);
	_wsplitpath_s(appDir, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
	wstring path = wstring(drive) + wstring(dir) + L"Modules_Native\\*.dll";

	WIN32_FIND_DATA ffd;

	HANDLE hFind = NULL;

	hFind = FindFirstFile(path.c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
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

			_wsplitpath_s(path.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

			_wmakepath_s(path_buffer, _MAX_PATH, drive, dir, ffd.cFileName, NULL);

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
					if ((_slmNameMap.end() != _slmNameMap.find(i)) && (0 == _slmNameMap[i].compare(dllName)))
					{
						alreadyExist = true;
						break;
					}
				}
				if (!alreadyExist)
				{
					for (long i = 1; i < _dllIdNotFoundCounter; i++)
					{
						if ((_slmNotFoundNameMap.end() != _slmNotFoundNameMap.find(i)) && (0 == _slmNotFoundNameMap[i].compare(dllName)))
						{
							alreadyExist = true;
							break;
						}
					}
				}
			}

			if (alreadyExist)
				continue;

			//*NOTE* do not delete the device dll memory. 
			//deletion of the dll will prevent 
			//the use of the functions exposed by the dll 
			SLMDll* slm = new SLMDll(path_buffer);

			if (NULL != slm)
			{
				if ((true == slm->Initialize(0)) && (0 < slm->FindSLM(xml)))
				{
					_slmMap.insert(pair<long, SLMDll*>(_dllIdFoundCounter, slm));
					_slmNameMap.insert(pair<long, wstring>(_dllIdFoundCounter, dllName));
					_dllIdFoundCounter++;
					wsprintf(msgInfo, L"SlmManager add device dll %s", dllName.c_str());
					LogMessage(msgInfo, VERBOSE_EVENT);
				}
				else
				{
					//If find slm returned false, add the dll to the slmNotFoundMap for future check in find slm.
					//In case a new slm is connected, only this map will be used to search.
					_slmNotFoundMap.insert(pair<long, SLMDll*>(_dllIdNotFoundCounter, slm));
					_slmNotFoundNameMap.insert(pair<long, wstring>(_dllIdNotFoundCounter, dllName));
					_dllIdNotFoundCounter++;
					wsprintf(msgInfo, L"SlmManager add dll to list of unused dlls %s", dllName.c_str());
					LogMessage(msgInfo, VERBOSE_EVENT);
				}
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);

	_initialFound = true;

	return TRUE;
}

long SlmManager::ReleaseSLMs()
{
	for (_it = _slmMap.begin(); _it != _slmMap.end(); ++_it)
	{
		((SLMDll*)_it->second)->TeardownSLM();
	}
	return TRUE;
}

ISLM* SlmManager::GetSLM(long id)
{
	ISLM* slm = NULL;
	if (id <= static_cast<long>(_slmMap.size()))
	{
		map<long, SLMDll*>::const_iterator itSlm;

		itSlm = _slmMap.find(id);

		if (itSlm != _slmMap.end())
		{
			slm = itSlm->second;

			wsprintf(msgInfo, L"SlmManager GetSLM id %d found", id);
			LogMessage(msgInfo, VERBOSE_EVENT);
		}
	}

	return slm;
}
