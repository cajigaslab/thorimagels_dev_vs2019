#include "..\..\..\..\Tools\WinUnit\Include\WinUnit.h"
#include "..\..\..\..\Common\Device.h"
#include "..\..\DeviceManager\DeviceManager\DeviceManager.h"

#include <stdlib.h>
#include <time.h>

//#include "tiffio.h"
//#include "TiffLib.h"

FIXTURE(DeviceManagerTestFixture);

long numDevices=0;


SETUP(DeviceManagerTestFixture)
{	
	WIN_ASSERT_TRUE(DeviceManager::getInstance()->FindDevices(_T("C:\\TIS\\Tools\\WinUnit\\Debug\\Modules_Native\\*.dll"),numDevices)==TRUE);

	WIN_TRACE("Found %d Devices\n",numDevices);	
}

TEARDOWN(DeviceManagerTestFixture)
{
}

//Find Devices
BEGIN_TESTF(FindDevices,DeviceManagerTestFixture)
{
	//**TODO** The following test fails with an exception upon exit
	
	WIN_ASSERT_TRUE(DeviceManager::getInstance()->FindDevices(_T("C:\\TIS\\Tools\\WinUnit\\Debug\\Modules_Native\\*.dll"),numDevices)==TRUE);

	WIN_TRACE("Found %d Devices\n",numDevices);	

	WIN_ASSERT_TRUE(DeviceManager::getInstance()->FindDevices(_T("C:\\TIS\\Tools\\WinUnit\\Debug\\Modules_Native\\*.dll"),numDevices)==TRUE);

	WIN_TRACE("Found %d Devices\n",numDevices);	

		
}
END_TESTF
/*
//Get Devices
BEGIN_TESTF(GetDevices,DeviceManagerTestFixture)
{

	long i;
	long id;
	for(i=0; i<numDevices; i++)
	{
		WIN_ASSERT_TRUE(DeviceManager::getInstance()->GetDeviceId(i,id)==TRUE);
		WIN_TRACE("Device %d id %d\n",i,id);

		IDevice *Device;
		
		Device = DeviceManager::getInstance()->GetDevice(id);
		
		WIN_ASSERT_TRUE((Device != NULL)==TRUE);
	}

	//failure checks
	i = -1;
	WIN_ASSERT_FALSE(DeviceManager::getInstance()->GetDeviceId(i,id)==TRUE);
	WIN_TRACE("Device %d id %d\n",i,id);

	IDevice *Device;

	Device = DeviceManager::getInstance()->GetDevice(id);
	WIN_ASSERT_FALSE((Device != NULL)==TRUE);

	i=numDevices;
	WIN_ASSERT_FALSE(DeviceManager::getInstance()->GetDeviceId(i,id)==TRUE);
	WIN_TRACE("Device %d id %d\n",i,id);

	Device = DeviceManager::getInstance()->GetDevice(id);
	WIN_ASSERT_FALSE((Device != NULL)==TRUE);

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

	for(i=0; i<numDevices; i++)
	{
		id=0;
		WIN_ASSERT_TRUE(DeviceManager::getInstance()->GetDeviceId(i,id)==TRUE);

		IDevice *Device;
		
		Device = DeviceManager::getInstance()->GetDevice(id);
		
		WIN_ASSERT_TRUE((Device != NULL)==TRUE);

		WIN_TRACE("Thread %d\n",::GetCurrentThreadId());
	}

	SetEvent( info.hEventThread );
	return 0;
}

//Access same Device from multiple threads
BEGIN_TESTF(DeviceManagerThreading,DeviceManagerTestFixture)
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


BEGIN_TESTF(DeviceTestSH05,DeviceManagerTestFixture)
{
	long i=0;
	long id;

	WIN_ASSERT_TRUE(DeviceManager::getInstance()->GetDeviceId(i,id)==TRUE);

		printf("device id %d\n",id);

		IDevice *device;
		
		device = DeviceManager::getInstance()->GetDevice(id);
		
		WIN_ASSERT_TRUE((device != NULL)==TRUE);


		device->PreflightPosition();

		printf("set wait time\n");
		device->SetParam(IDevice::PARAM_SHUTTER_WAIT_TIME,30);

		printf("shutter open\n");
		device->SetParam(IDevice::PARAM_SHUTTER_POS,0);
		device->SetupPosition();
		printf("shutter setup\n");
		device->StartPosition();
		printf("shutter status\n");
		long status;
		device->StatusPosition(status);

		printf("shutter close\n");
		device->SetParam(IDevice::PARAM_SHUTTER_POS,1);
		device->SetupPosition();
		device->StartPosition();

		double pos;
		device->ReadPosition(pos);

		device->PostflightPosition();
}
END_TESTF
*/