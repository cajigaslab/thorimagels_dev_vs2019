#pragma once

#include <map>
#include "..\..\..\..\Common\thread.h"

using namespace std;

#pragma warning( push )
#pragma warning( disable : 4251 )

class ThorTsiTest
{
public:
	~ThorTsiTest();
    static ThorTsiTest* getInstance();
	long FindCameras(const WCHAR * path,long &numCameras);
	long ReleaseCameras();
	long GetCameraId(long i,long &id);	
	ICamera* GetCamera(long id);

private:
    ThorTsiTest();
	
    static bool instanceFlag;
    static auto_ptr<ThorTsiTest> _single;
	static CritSect critSect;
	static long idCounter;
	static void cleanup(void);

	map<long,CameraDll*> cameraMap;
	map<long,CameraDll*>::const_iterator it;

};

#pragma warning( pop )
