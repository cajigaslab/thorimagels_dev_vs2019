#include "..\..\..\..\Tools\WinUnit\Include\WinUnit.h"
#include "..\..\..\..\Common\Camera.h"
#include "..\..\CameraManager\CameraManager\CameraManager.h"

#include <stdlib.h>
#include <time.h>

//#include "tiffio.h"
//#include "TiffLib.h"



FIXTURE(CameraManagerTestFixture);

long numCameras=0;


SETUP(CameraManagerTestFixture)
{	
	WIN_ASSERT_TRUE(CameraManager::getInstance()->FindCameras(_T("C:\\TIS\\Tools\\WinUnit\\Debug\\Modules_Native\\*.dll"),numCameras)==TRUE);

	WIN_TRACE("Found %d cameras\n",numCameras);	
}

TEARDOWN(CameraManagerTestFixture)
{
}

//Find Cameras
BEGIN_TESTF(FindCameras,CameraManagerTestFixture)
{
	WIN_ASSERT_TRUE(CameraManager::getInstance()->FindCameras(_T("C:\\TIS\\Tools\\WinUnit\\Debug\\Modules_Native\\*.dll"),numCameras)==TRUE);

	WIN_TRACE("Found %d cameras\n",numCameras);	

	WIN_ASSERT_TRUE(CameraManager::getInstance()->FindCameras(_T("C:\\TIS\\Tools\\WinUnit\\Debug\\Modules_Native\\*.dll"),numCameras)==TRUE);

	WIN_TRACE("Found %d cameras\n",numCameras);	
}
END_TESTF

//Get Cameras
BEGIN_TESTF(GetCameras,CameraManagerTestFixture)
{
	long i;
	long id;
	for(i=0; i<numCameras; i++)
	{
		WIN_ASSERT_TRUE(CameraManager::getInstance()->GetCameraId(i,id)==TRUE);
		WIN_TRACE("Camera %d id %d\n",i,id);

		ICamera *camera;
		
		camera = CameraManager::getInstance()->GetCamera(id);
		
		WIN_ASSERT_TRUE((camera != NULL)==TRUE);
	}

	//failure checks
	i = -1;
	WIN_ASSERT_FALSE(CameraManager::getInstance()->GetCameraId(i,id)==TRUE);
	WIN_TRACE("Camera %d id %d\n",i,id);

	ICamera *camera;

	camera = CameraManager::getInstance()->GetCamera(id);
	WIN_ASSERT_FALSE((camera != NULL)==TRUE);

	i=numCameras;
	WIN_ASSERT_FALSE(CameraManager::getInstance()->GetCameraId(i,id)==TRUE);
	WIN_TRACE("Camera %d id %d\n",i,id);

	camera = CameraManager::getInstance()->GetCamera(id);
	WIN_ASSERT_FALSE((camera != NULL)==TRUE);

}
END_TESTF


#define NUM_THREADS 10

namespace
{
	HANDLE hEvent[NUM_THREADS];
	long val;
}

struct Info
{
	long id;
	HANDLE hEventThread;
};

UINT Thread1Proc( LPVOID pParam )
{
	Info info;

	info = *(Info*)(pParam);

	long i;
	long id;

	for(i=0; i<numCameras; i++)
	{
		id=0;
		WIN_ASSERT_TRUE(CameraManager::getInstance()->GetCameraId(i,id)==TRUE);

		ICamera *camera;
		
		camera = CameraManager::getInstance()->GetCamera(id);
		
		WIN_ASSERT_TRUE((camera != NULL)==TRUE);

		WIN_TRACE("Thread %d\n",::GetCurrentThreadId());
	}

	SetEvent( info.hEventThread );
	return 0;
}

//Access same Camera from multiple threads
BEGIN_TESTF(CameraManagerThreading,CameraManagerTestFixture)
{
	long i;

	for(i=0; i<NUM_THREADS; i++)
	{
	hEvent[i] = CreateEvent(0, FALSE, FALSE, 0);
	}

	DWORD dwThreadId[NUM_THREADS];

	Info info[NUM_THREADS];
	
	for(i=0; i<NUM_THREADS; i++)
	{
	info[i].id = 0;
	info[i].hEventThread = hEvent[i];
	}

	HANDLE hThread[NUM_THREADS];
	
	for(i=0; i<NUM_THREADS; i++)
	{
	hThread[i] = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) Thread1Proc, (LPVOID)&info[i], 0, &dwThreadId[i] );
	}

	DWORD dwWait = WaitForMultipleObjects( NUM_THREADS, hEvent, TRUE, INFINITE );

	WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

	for(i=0; i<NUM_THREADS; i++)
	{
	CloseHandle(hThread[i]);
	}

}
END_TESTF
