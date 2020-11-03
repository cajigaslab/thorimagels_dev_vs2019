// DeviceManger.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "DeviceManager.h"

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif


DeviceManager::DeviceManager()
{    
	_initialFound = false;
}

DeviceManager::~DeviceManager()
{
	_instanceFlag = false;

	cleanup();
}

wchar_t message[256];

bool DeviceManager::_instanceFlag = false;

long DeviceManager::_dllIdFoundCounter = 1;		//1:based, 0 reserved for no device

long DeviceManager::_dllIdNotFoundCounter = 1;

auto_ptr<DeviceManager> DeviceManager::_single(new DeviceManager());

CritSect DeviceManager::_critSect;

DeviceManager* DeviceManager::getInstance()
{
	Lock lock(_critSect);

	if(! _instanceFlag)
	{
		try
		{
			_single.reset(new DeviceManager());

			wsprintf(message,L"DeviceManager Created");
			LogMessage(message, VERBOSE_EVENT);
		}
		catch(...)
		{
			//critically low on resources
			//do not proceed with application
			throw;
		}
		_instanceFlag = true;
	}
	return _single.get();
}

void DeviceManager::cleanup(void)
{
	for(_it = _deviceMap.begin(); _it != _deviceMap.end(); ++_it)
	{
		((DeviceDll*)_it->second)->Uninitialize();
		delete ((DeviceDll*)_it->second);
		wsprintf(message,L"DeviceManager deleting device object %d",_it->first);
		LogMessage(message, VERBOSE_EVENT);
	}

	for(_it = _deviceNotFoundMap.begin(); _it != _deviceNotFoundMap.end(); ++_it)
	{
		((DeviceDll*)_it->second)->Uninitialize();
		delete ((DeviceDll*)_it->second);
		wsprintf(message,L"DeviceManager deleting device object %d",_it->first);
		LogMessage(message, VERBOSE_EVENT);
	}

	_deviceMap.clear();
	_deviceNameMap.clear();
	_deviceNotFoundMap.clear();
	_deviceNotFoundNameMap.clear();
}

long DeviceManager::FindDevices(const WCHAR *path, long &numDevices)
{
	Lock lock(_critSect);

	long numDev = 0;

	numDevices = 0;

	//no redo dlls search to avoid memory leak because each new dll search creates a new dll pointer.
	if(_initialFound)
	{
		ReleaseDevices();

		//Reconnect the devices found/mapped at start up.
		map<long,DeviceDll*> tmpDeviceMap;	
		map<long,wstring> tmpDeviceNameMap;
		long tmpFoundCounter = 1;
		for(_it = _deviceMap.begin(); _it != _deviceMap.end(); ++_it)
		{
			if(TRUE == ((DeviceDll*)_it->second)->FindDevices(numDev) && (0 < numDev))
			{
				tmpDeviceMap.insert(pair<long,DeviceDll*>(tmpFoundCounter,((DeviceDll*)_it->second)));
				tmpDeviceNameMap.insert(pair<long, wstring>(tmpFoundCounter, _deviceNameMap.at(_it->first)));
				tmpFoundCounter++;
				numDevices++;
			}
			else
			{
				if (NULL != _it->second)
				{
					((DeviceDll*)_it->second)->Uninitialize();
					delete ((DeviceDll*)_it->second);
					(DeviceDll*)_it->second = NULL;
				}
			}
		}
		_deviceMap.clear();
		_deviceNameMap.clear();
		_deviceMap = tmpDeviceMap;
		_deviceNameMap = tmpDeviceNameMap;
		_dllIdFoundCounter = tmpFoundCounter;

		wsprintf(message,L"DeviceManager already found devices %d", numDevices);
		LogMessage(message, VERBOSE_EVENT);

		//Look for any additional devices that could have been connected. Update the found Devices Map.
		map<long,DeviceDll*> tmpNotFoundDeviceMap;	
		map<long,wstring> tmpNotFoundDeviceNameMap;
		long tmpNotFoundCounter = 1;
		for(_it = _deviceNotFoundMap.begin(); _it != _deviceNotFoundMap.end(); ++_it)
		{
			if(TRUE == ((DeviceDll*)_it->second)->FindDevices(numDev) && (0 < numDev))
			{
				for(long i=0; i<numDev; i++)
				{
					_deviceMap.insert(pair<long,DeviceDll*>(_dllIdFoundCounter,((DeviceDll*)_it->second)));
					_deviceNameMap.insert(pair<long, wstring>(_dllIdFoundCounter, _deviceNotFoundNameMap.at(_it->first)));
					_dllIdFoundCounter++;
					numDevices++;
					wsprintf(message,L"DeviceManager add device dll %s",  _deviceNotFoundNameMap.at(_it->first).c_str());
					LogMessage(message, VERBOSE_EVENT);
				}
			}
			else
			{
				tmpNotFoundDeviceMap.insert(pair<long,DeviceDll*>(tmpNotFoundCounter,((DeviceDll*)_it->second)));
				tmpNotFoundDeviceNameMap.insert(pair<long, wstring>(tmpNotFoundCounter, _deviceNotFoundNameMap.at(_it->first)));
				tmpNotFoundCounter++;
			}
		}
		_deviceNotFoundMap.clear();
		_deviceNotFoundNameMap.clear();
		_deviceNotFoundMap = tmpNotFoundDeviceMap;
		_deviceNotFoundNameMap = tmpNotFoundDeviceNameMap;
		_dllIdNotFoundCounter = tmpNotFoundCounter;

		wsprintf(message,L"DeviceManager updated the number of devices to %d", numDevices);
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
					if ((_deviceNameMap.end() != _deviceNameMap.find(i)) && (0 == _deviceNameMap[i].compare(dllName)))
					{
						alreadyExist = true;
						break;
					}
				}
				if(!alreadyExist)
				{
					for (long i = 1; i < _dllIdNotFoundCounter; i++)
					{
						if ((_deviceNotFoundNameMap.end() != _deviceNotFoundNameMap.find(i)) && (0 == _deviceNotFoundNameMap[i].compare(dllName)))
						{
							alreadyExist = true;
							break;
						}
					}
				}
			}

			if(alreadyExist)
				continue;

			//*NOTE* do not delete the device dll memory. 
			//deletion of the dll will prevent 
			//the use of the functions exposed by the dll 
			DeviceDll *device = new DeviceDll(path_buffer);

			if(NULL != device)
			{
				if((true == device->Initialize(0)) && (TRUE == device->FindDevices(numDev)) && (numDev > 0))
				{
					for(long i=0; i<numDev; i++)
					{
						_deviceMap.insert(pair<long,DeviceDll*>(_dllIdFoundCounter,device));
						_deviceNameMap.insert(pair<long, wstring>(_dllIdFoundCounter, dllName));
						_dllIdFoundCounter++;
						numDevices++;
						wsprintf(message,L"DeviceManager add device dll %s", dllName.c_str());
						LogMessage(message, VERBOSE_EVENT);
					}
				}
				else
				{
					//If find device returned false, add the dll to the deviceNotFoundMap for future check in find device.
					//In case a new device is connected, only this map will be used to search.
					_deviceNotFoundMap.insert(pair<long, DeviceDll*>(_dllIdNotFoundCounter, device));
					_deviceNotFoundNameMap.insert(pair<long, wstring>(_dllIdNotFoundCounter, dllName));
					_dllIdNotFoundCounter++;
					wsprintf(message,L"DeviceManager add dll to list of unused dlls %s", dllName.c_str());
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

long DeviceManager::ReleaseDevices()
{
	for(_it = _deviceMap.begin(); _it != _deviceMap.end(); ++_it)
	{
		((DeviceDll*)_it->second)->TeardownDevice();
	}
	return TRUE;
}

long DeviceManager::GetDeviceId(long i,long &id)
{
	long ret;
	//return value of id is zero if no Device is found
	id = 0;

	_it = _deviceMap.begin();

	if(i<0)
	{
		return FALSE;
	}

	for(long j=0;j<i;j++,_it++)
	{
	}

	if(_it != _deviceMap.end())
	{
		id = _it->first;
		ret = TRUE;
	}	
	else
	{
		ret = FALSE;
	}

	return ret;
}

IDevice* DeviceManager::GetDevice(long id)
{
	IDevice * device = NULL;	
	if(id <= static_cast<long>(_deviceMap.size()))
	{
		map<long,DeviceDll*>::const_iterator itDev;

		itDev = _deviceMap.find(id);

		if(itDev != _deviceMap.end())
		{
			device = itDev->second;

			wsprintf(message,L"DeviceManager GetDevice id %d found",id);
			LogMessage(message, VERBOSE_EVENT);
		}
	}

	return device;
}

wstring DeviceManager::GetDeviceDllName(long id)
{
	if(id <= static_cast<long>(_deviceNameMap.size()))
	{
		map<long, wstring>::const_iterator itDevName;

		itDevName = _deviceNameMap.find(id);

		if(itDevName != _deviceNameMap.end())
		{
			return itDevName->second;
		}
	}

	wsprintf(message,L"DeviceManager GetDeviceDllName did not find dll name for device %d",id);
	LogMessage(message, VERBOSE_EVENT);
	return std::wstring();

}

void DeviceManager::LogMessage(wchar_t *message, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, message);
#endif
}
