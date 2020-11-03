#pragma once

#include <string>
#include <map>
#include "..\..\..\..\Common\thread.h"

using namespace std;

#if defined(Device_MANAGER)
#define DllExport_DeviceManager __declspec(dllexport)
#else
//    definitions used when using DLL
#define DllExport_DeviceManager __declspec(dllimport)
#endif

#pragma warning( push )
#pragma warning( disable : 4251 )

class DllExport_DeviceManager DeviceManager
{
public:
	~DeviceManager();
	static DeviceManager* getInstance();
	long FindDevices(const WCHAR * path,long &numDevices);
	long ReleaseDevices();
	long GetDeviceId(long i,long &id);	
	IDevice* GetDevice(long id);
	wstring GetDeviceDllName(long id);

private:
	DeviceManager();

	static bool _instanceFlag;
	static auto_ptr<DeviceManager> _single;
	static CritSect _critSect;
	static long _dllIdFoundCounter;
	static long _dllIdNotFoundCounter;
	void cleanup(void);
	static void LogMessage(wchar_t* message, long eventLevel);

	bool _initialFound;
	map<long,DeviceDll*> _deviceMap;			//found devices' map, unique id but not unique dll memory, eg> [0 1 2 3] = [*p0 *p1 *p1 *p2]
	map<long,wstring> _deviceNameMap;			//found devices' map name, unique id but not unique dll name, eg> [0 1 2 3] = [A B B C]
	map<long,DeviceDll*> _deviceNotFoundMap;	//not found devices' map, should be unique device per id
	map<long,wstring> _deviceNotFoundNameMap;	//not found devices' map name, should be unique device per id
	map<long,DeviceDll*>::const_iterator _it;

};


#pragma warning( push )