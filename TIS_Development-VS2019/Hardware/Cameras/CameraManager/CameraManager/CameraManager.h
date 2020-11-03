#pragma once
#include <string>
#include <map>
#include "..\..\..\..\Common\thread.h"

using namespace std;

#if defined(CAMERA_MANAGER)
#define DllExport_CameraManager __declspec(dllexport)
#else
//    definitions used when using DLL
#define DllExport_CameraManager __declspec(dllimport)
#endif

#pragma warning( push )
#pragma warning( disable : 4251 )

class DllExport_CameraManager CameraManager
{
public:
	~CameraManager();
	static CameraManager* getInstance();
	long FindCameras(const WCHAR * path,long &numCameras);
	long ReleaseCameras();
	long GetCameraId(long i,long &id);	
	ICamera* GetCamera(long id);
	wstring GetCameraDllName(long id);

private:
	CameraManager();

	static bool instanceFlag;
	static auto_ptr<CameraManager> _single;
	static CritSect critSect;
	static long _dllIdFoundCounter;
	static long _dllIdNotFoundCounter;
	void cleanup(void);
	static void LogMessage(wchar_t* message, long eventLevel);

	bool _initialFound;
	map<long,CameraDll*> _cameraMap;			//found cameras' map, unique id but not unique dll memory, eg> [0 1 2 3] = [*p0 *p1 *p1 *p2]
	map<long,wstring> _cameraNameMap;			//found cameras' map name, unique id but not unique dll name, eg> [0 1 2 3] = [A B B C]
	map<long,CameraDll*> _cameraNotFoundMap;	//not found cameras' map, should be unique device per id
	map<long,wstring> _cameraNotFoundNameMap;	//not found cameras' map name, should be unique device per id
	map<long,CameraDll*>::const_iterator _it;

};

#pragma warning( pop )
