#include "..\..\..\Tools\WinUnit\Include\WinUnit.h"
#include "..\..\..\Common\Device.h"

#include <stdlib.h>
#include <time.h>

const int QUANTS = 6;
CRITICAL_SECTION cs;

namespace
{
	DeviceDll * pDevice = NULL; 
	long numDevices = 0;
	HANDLE hEvent;
	wchar_t DevicePathandName[MAX_PATH] = _T("");
	const long MSG_SIZE= 256;
	wchar_t errMsg[MSG_SIZE];
	long stopThreadProc = FALSE;

	#define DevErrCheck(fnCall) ((TRUE == (fnCall)) ? (TRUE) : (ReportError()))
}

long ReportError()
{
	pDevice->GetLastErrorMsg(errMsg,MSG_SIZE);

	WIN_TRACE(_T("%s\n"),errMsg);
	return FALSE;
}

UINT StatusThreadProc( LPVOID pParam )
{
	if(pDevice == NULL)
	{
		SetEvent(hEvent);
		return 1;
	}

	WIN_TRACE("StatusThreadProc Created\n");

	long status = IDevice::STATUS_BUSY;

	while((status == IDevice::STATUS_BUSY) && (FALSE == stopThreadProc) )
	{
		if(FALSE == pDevice->StatusPosition(status))
		{
			break;
		}
		Sleep(2);
	}
	stopThreadProc = FALSE;
	WIN_TRACE("StatusThreadProc Exiting\n");
	SetEvent(hEvent);

	return 0;
}

long GetParam(IDevice* pDev, long paramID, double& param)
{
	long paramType = 0;
	long paramAvailable = 0;
	long paramReadOnly = 0;
	double paramMin = 0.0;
	double paramMax = 0.0;
	double paramDefault = 0.0;
	long ret = FALSE;

	if(pDev->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
	{
		if(TRUE == paramAvailable)
		{
			return pDev->GetParam(paramID, param);
		}

		WIN_TRACE("GetParam(paramID=%d) skipped. Pamameter not available.\n", paramID);
		ret = TRUE;
	}
	return ret;
}

long SetParam(IDevice* pDev, long paramID, double param)
{
	long paramType = 0;
	long paramAvailable = 0;
	long paramReadOnly = 0;
	double paramMin = 0.0;
	double paramMax = 0.0;
	double paramDefault = 0.0;
	long ret = FALSE;

	if(pDev->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
	{
		if(TRUE == paramAvailable)
		{
			return pDev->SetParam(paramID, param);
		}

		WIN_TRACE("SetParam(paramID=%d) skipped. Pamameter not available.\n", paramID);
		ret = TRUE;
	}
	return ret;
}

long MoveDevice(const long devParam, const double to)
{
	long lDummy = 0;
	long paramAvailable = 0;
	double dDummy = 0;
	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Z_POS, lDummy, paramAvailable, lDummy, dDummy, dDummy, dDummy)==TRUE);
	WIN_ASSERT_TRUE(paramAvailable == TRUE);
	WIN_ASSERT_TRUE(pDevice->SetParam(devParam,to)==TRUE);
	WIN_ASSERT_TRUE(pDevice->SetupPosition());
	WIN_ASSERT_TRUE(pDevice->StartPosition());
	WIN_TRACE("Move Device with parameter %d to %f\n",devParam,to);

	return TRUE;
}

long WaitAndStopDevice(const long stopParam, DWORD maxWait)
{
	DWORD dwThreadId;
	HANDLE hThread;
	hEvent = CreateEvent(0, FALSE, FALSE, 0);
	WIN_TRACE("WaitAndStopDevice maxWait=%d\n", maxWait);
	hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
	try
	{
	DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, maxWait);

	switch(dwWait)
	{
	case WAIT_OBJECT_0:
		{
		WIN_TRACE("MoveDevice completed successfully. ResetLastError ...\n");
		SetLastError(0);
		}
		break;
	case WAIT_FAILED:
		{
		WIN_TRACE("WaitAndStopDevice FAILED\n");
		stopThreadProc = TRUE;

		while(stopThreadProc)
		{
			Sleep(10);
		}
				//stop the z stepper
		WIN_ASSERT_TRUE(DevErrCheck(SetParam(pDevice, stopParam, 1)));
		WIN_ASSERT_TRUE(pDevice->SetupPosition());
		WIN_ASSERT_TRUE(DevErrCheck(pDevice->StartPosition()));
		WIN_TRACE("MoveDevice device stopped\n");
		}
		break;
	case WAIT_TIMEOUT:
		{
		WIN_TRACE("WaitAndStopDevice TIMEOUT\n");
		stopThreadProc = TRUE;
		
		while(stopThreadProc)
		{
			Sleep(10);
		}
				//stop the z stepper
		WIN_ASSERT_TRUE(DevErrCheck(SetParam(pDevice, stopParam, 1)));
		WIN_ASSERT_TRUE(pDevice->SetupPosition());
		WIN_ASSERT_TRUE(DevErrCheck(pDevice->StartPosition()));
		WIN_TRACE("MoveDevice device stopped\n");
		}
		break;
	default:
		{
		WIN_TRACE("MoveDevice not completed - OTHER ERROR %d\n", dwWait);
		WIN_TRACE("MoveDevice trying to stop device\n");
		stopThreadProc = TRUE;
		
		while(stopThreadProc)
		{
			Sleep(10);
		}
		//stop the z stepper
		WIN_ASSERT_TRUE(DevErrCheck(SetParam(pDevice, stopParam, 1)));
		WIN_ASSERT_TRUE(pDevice->SetupPosition());
		WIN_ASSERT_TRUE(DevErrCheck(pDevice->StartPosition()));
		WIN_TRACE("MoveDevice device stopped\n");
		}
	}
	}
	catch(...)
	{
	CloseHandle(hThread);
	ResetEvent(hEvent);
	return FALSE;
	}
	CloseHandle(hThread);
	ResetEvent(hEvent);
	return TRUE;
}

FIXTURE(DeviceTestFixture)


SETUP(DeviceTestFixture)
{
	WIN_ASSERT_TRUE(WinUnit::Environment::GetVariable(_T("DevicePathandName"),DevicePathandName,ARRAYSIZE(DevicePathandName)), _T("Environment variable DevicePathandName was not set.  Use --DevicePathandName option."));

	//load the device dll
	pDevice = NULL;
	pDevice = new DeviceDll(DevicePathandName);
	WIN_ASSERT_TRUE(pDevice != NULL);

	WIN_TRACE("DeviceTestFixture SETUP\n");

	//verify that devices are found
	WIN_ASSERT_TRUE(DevErrCheck(pDevice->FindDevices(numDevices)==TRUE));

	WIN_TRACE("Number of devices found %d\n",numDevices);

	WIN_ASSERT_TRUE(numDevices > 0);	

	//select the first device
	WIN_TRACE("DeviceTestFixture SETUP select device 0. \n");
	WIN_ASSERT_TRUE(pDevice->SelectDevice(0) == TRUE);

}


TEARDOWN(DeviceTestFixture)
{
	WIN_TRACE("DeviceTestFixture TEARDOWN\n");
	WIN_ASSERT_TRUE(pDevice->TeardownDevice()==TRUE);

	//delete pDevice;
	pDevice = NULL;
}


#define	DBL_EPSILON 1e-10
#define	DOUBLE_EQ(x,v) (((v - DBL_EPSILON) < x) && (x <( v + DBL_EPSILON)))

BEGIN_TESTF(SelectDeviceTest,DeviceTestFixture)
{
	long i;

	//select each of the devices
	for(i=0; i<numDevices; i++)
	{
		WIN_ASSERT_TRUE(pDevice->SelectDevice(i) == TRUE);
	}

	//check devices outside the limits
	WIN_ASSERT_FALSE(pDevice->SelectDevice(-1) == TRUE);
	WIN_ASSERT_FALSE(pDevice->SelectDevice(numDevices) == TRUE);
}
END_TESTF

BEGIN_TESTF(GetSetParamTest,DeviceTestFixture)
{
	long i;

	long paramID = 0;
	long paramType = 0;
	long paramAvailable = 0;
	long paramReadOnly = 0;
	double paramMin = 0.0;
	double paramMax = 0.0;
	double paramDefault = 0.0;

	//for every Device iterate through all of the parameters and do get/set tests and verify the results
	for(i=0; i<numDevices; i++)
	{
		for(paramID=IDevice::PARAM_FIRST_PARAM; paramID < IDevice::PARAM_LAST_PARAM; paramID++)
		{
			WIN_ASSERT_TRUE(pDevice->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

			if(TRUE == paramAvailable)
			{
				WIN_TRACE("Parameter %d available\n",paramID);
				double val;

				//if the parameter is read only make sure the get param function works
				if(TRUE == paramReadOnly)
				{
					WIN_TRACE("Parameter %d read only\n",paramID);
					WIN_ASSERT_TRUE(pDevice->GetParam(paramID,val)==TRUE,_T("paramID %d readonly value %f"),paramID,val);
				}
				else
				{
					//min
					WIN_ASSERT_TRUE(pDevice->SetParam(paramID,paramMin)==TRUE);
					WIN_ASSERT_TRUE(pDevice->SetupPosition()==TRUE);
					WIN_ASSERT_TRUE(pDevice->GetParam(paramID,val)==TRUE);
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMin,val)==TRUE);

					//max
					WIN_ASSERT_TRUE(pDevice->SetParam(paramID,paramMax)==TRUE);
					WIN_ASSERT_TRUE(pDevice->SetupPosition()==TRUE);
					WIN_ASSERT_TRUE(pDevice->GetParam(paramID,val)==TRUE);
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMax,val)==TRUE);

					//default
					WIN_ASSERT_TRUE(pDevice->SetParam(paramID,paramDefault)==TRUE);
					WIN_ASSERT_TRUE(pDevice->SetupPosition()==TRUE);
					WIN_ASSERT_TRUE(pDevice->GetParam(paramID,val)==TRUE);
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramDefault,val)==TRUE);

					//if min max are not equal send a random value in between
					if(DOUBLE_EQ(paramMin,paramMax)==FALSE)
					{
						double newVal;
						double randDouble;

						srand(static_cast<unsigned int>(time(NULL)));

						randDouble = (   static_cast<double>(rand()) / (static_cast<double>(RAND_MAX)+static_cast<double>(1)) ); 

						newVal = randDouble * (paramMax - paramMin) + paramMin;

						//if long data type cast before setting the parameter
						if(paramType == IDevice::TYPE_LONG)
						{
							newVal = static_cast<long>(newVal);
						}

						//random value
						WIN_ASSERT_TRUE(pDevice->SetParam(paramID,newVal)==TRUE,_T("paramID %d newVal %f val %f"),paramID,newVal,val);
						WIN_ASSERT_TRUE(pDevice->SetupPosition()==TRUE);
						WIN_ASSERT_TRUE(pDevice->GetParam(paramID,val)==TRUE,_T("paramID %d newVal %f val %f"),paramID,newVal,val);
						WIN_ASSERT_TRUE(DOUBLE_EQ(newVal,val)==TRUE,_T("paramID %d newVal %f val %f"),paramID,newVal,val);
					}

					//below min
					WIN_ASSERT_TRUE(pDevice->SetParam(paramID,paramMin - 1.0)==FALSE);
					WIN_ASSERT_TRUE(pDevice->SetupPosition()==TRUE);
					WIN_ASSERT_TRUE(pDevice->GetParam(paramID,val)==TRUE);
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMin-1,val)==FALSE);

					//above max
					WIN_ASSERT_TRUE(pDevice->SetParam(paramID,paramMax + 1.0)==FALSE);
					WIN_ASSERT_TRUE(pDevice->SetupPosition()==TRUE);
					WIN_ASSERT_TRUE(pDevice->GetParam(paramID,val)==TRUE);
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMax+1,val)==FALSE);		
				}
			}
			else
			{
			}
		}
	}
}
END_TESTF

//Test the homing of Beam Expander
BEGIN_TESTF(BmExpanHomeTest, DeviceTestFixture)
{	
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
	WIN_ASSERT_TRUE(paramAvailable==TRUE);


	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::BEAM_EXPANDER)))
	{
		WIN_TRACE("Device is not a Beam Expander\n");
	}
	else
	{
		WIN_TRACE("Beam Expander Homing Test\n");
		hEvent = CreateEvent(0, FALSE, FALSE, 0);
		WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_MOT0_POS,0)==TRUE);
		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_MOT1_POS,0)==TRUE);
		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_BMEXP_MODE,0)==TRUE);
		WIN_ASSERT_TRUE(pDevice->SetupPosition());
		WIN_ASSERT_TRUE(pDevice->StartPosition());

		DWORD dwThreadId;
		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

		DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );
		WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

		CloseHandle(hThread);
		WIN_ASSERT_TRUE(pDevice->PostflightPosition());
	}

}
END_TESTF


//Test the homing and position of Beam Expander
BEGIN_TESTF(BmExpanPosTest, DeviceTestFixture)
{
	long i;
	long j;
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	//must select a device. choose the first one
	WIN_ASSERT_TRUE(pDevice->SelectDevice(0)==TRUE);
	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::BEAM_EXPANDER)))
	{
		WIN_TRACE("Device is not a Beam Expander\n");
	}
	else
	{
		for (j=0; j<10; j++)
		{
			WIN_TRACE("Beam Expander Homing Test\n");
			hEvent = CreateEvent(0, FALSE, FALSE, 0);
			WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_MOT0_POS,0)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_MOT1_POS,0)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_BMEXP_MODE,0)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());

			DWORD dwThreadId;
			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			WIN_ASSERT_TRUE(pDevice->PostflightPosition());
			WIN_TRACE("Beam Expander Homing Test Done\n");

			WIN_TRACE("Beam Expander Position Test\n");
			hEvent = CreateEvent(0, FALSE, FALSE, 0);
			for (i=0; i<6; i++)
			{
				WIN_TRACE("Beam Expander Position %d Test\n", i);
				WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

				WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_EXP_RATIO,i)==TRUE);
				WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_BMEXP_MODE,1)==TRUE);
				WIN_ASSERT_TRUE(pDevice->SetupPosition());
				WIN_ASSERT_TRUE(pDevice->StartPosition());

				DWORD dwThreadId;
				HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

				DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );
				WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

				CloseHandle(hThread);
				WIN_ASSERT_TRUE(pDevice->PostflightPosition());
				WIN_TRACE("Beam Expander Position %d Reached\n", i);
			}
			WIN_TRACE("Beam Expander Position Test Done\n");

		}
	}
}
END_TESTF


//Test the positioning capabilities of the shutter
BEGIN_TESTF(ShutterPositionTest,DeviceTestFixture)
{
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;


	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::SHUTTER)))
	{
		WIN_TRACE("Device is not a Shutter\n");
	}
	else
	{
		WIN_TRACE("Device has a shutter\n");

		hEvent = CreateEvent(0, FALSE, FALSE, 0);

		WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

		long i;
		long j;
		long wait = 10;

		//initial wait time of 10 msec. Increment by 10 for 2 iterations
		for(j=0; j<2; j++,wait+=10)
		{
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_SHUTTER_WAIT_TIME,wait)==TRUE);

			for(i=0; i<10; i++)
			{
				WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_SHUTTER_POS,0)==TRUE);

				WIN_ASSERT_TRUE(pDevice->SetupPosition());

				WIN_ASSERT_TRUE(pDevice->StartPosition());

				DWORD dwThreadId;

				HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

				DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );

				WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

				CloseHandle(hThread);

				WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_SHUTTER_POS,1)==TRUE);

				WIN_ASSERT_TRUE(pDevice->SetupPosition());

				WIN_ASSERT_TRUE(pDevice->StartPosition());

				hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

				dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );

				WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

				CloseHandle(hThread);
			}
		}

		WIN_ASSERT_TRUE(pDevice->PostflightPosition());
	}
}
END_TESTF


//Test the positioning capabilities of the R stage
BEGIN_TESTF(StageRPositionTest,DeviceTestFixture)
{
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::POWER_REG)))
	{
		WIN_TRACE("Device is not a Power Regulator\n");
	}
	else
	{
		//get the travel limits for the stage
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_POWER_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

		hEvent = CreateEvent(0, FALSE, FALSE, 0);

		WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

		long j;
		double posStep = (paramMax-paramMin) * .1;
		double posStart = paramMin + posStep * 1;
		double posEnd = paramMin + posStep * 9;
		double currentPos = 0;

		//go back and forth 4 times
		for(j=0; j<4; j++)
		{
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_POWER_POS,posStart+posStep*j)==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetupPosition());

			WIN_ASSERT_TRUE(pDevice->StartPosition());

			WIN_TRACE("Move Device to %f\n", posStart+posStep*j);

			WIN_ASSERT_TRUE(pDevice->ReadPosition(IDevice::POWER_REG, currentPos));

			WIN_TRACE("Device current position is %f\n", currentPos);

			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);

			ResetEvent(hEvent);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_POWER_POS,posStep*j)==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetupPosition());

			WIN_ASSERT_TRUE(pDevice->StartPosition());

			WIN_TRACE("Move Device to %f\n", posStart+posStep*j);

			WIN_ASSERT_TRUE(pDevice->ReadPosition(IDevice::POWER_REG, currentPos));

			WIN_TRACE("Device current position is %f\n", currentPos);

			hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

			dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);

			ResetEvent(hEvent);
		}

		WIN_ASSERT_TRUE(pDevice->PostflightPosition());
	}
}
END_TESTF


//Test the positioning capabilities of the X stage
BEGIN_TESTF(StageXPositionTest,DeviceTestFixture)
{
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::STAGE_X)))
	{
		WIN_TRACE("Device is not an X Stage\n");
	}
	else
	{
		//get the travel limits for the stage
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
		hEvent = CreateEvent(0, FALSE, FALSE, 0);
		WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);
		long j;
		double posStart = paramMin + (paramMax-paramMin)*.1;
		double posEnd = paramMin + (paramMax-paramMin) * .9;
		double pos=0;
		//go back and forth 2 times
		for(j=0; j<3; j++)
		{
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posStart)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());
			DWORD dwThreadId;
			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
			CloseHandle(hThread);
			pDevice->ReadPosition(IDevice::STAGE_X,pos);
			WIN_TRACE("Set X Pos %f Read X Pos %f\n",posStart,pos);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posEnd)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());
			hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
			CloseHandle(hThread);
			pDevice->ReadPosition(IDevice::STAGE_X,pos);
			WIN_TRACE("Set X Pos %f Read X Pos %f\n",posEnd,pos);
		}

		WIN_ASSERT_TRUE(pDevice->PostflightPosition());
	}
}
END_TESTF


//Test the positioning capabilities of the Y stage
BEGIN_TESTF(StageYPositionTest,DeviceTestFixture)
{
	long paramType = 0;
	long paramAvailable = 0;
	long paramReadOnly = 0;
	double paramMin = 0.0;
	double paramMax = 0.0;
	double paramDefault = 0.0;

	// set to stage y
	WIN_ASSERT_TRUE(pDevice->SelectDevice(0)==TRUE);
	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
	WIN_ASSERT_TRUE(paramAvailable==TRUE);


	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::STAGE_Y)))
	{
		WIN_TRACE("Device is not a Y Stage\n");
	}
	else
	{
		//get the travel limits for the stage
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
		hEvent = CreateEvent(0, FALSE, FALSE, 0);
		WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);
		long j;
		double posStart = paramMin + (paramMax-paramMin)*.1;
		double posEnd = paramMin + (paramMax-paramMin) * .9;
		double pos=0;
		//go back and forth 2 times
		for(j=0; j<3; j++)
		{
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS,posStart)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());
			DWORD dwThreadId;
			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
			CloseHandle(hThread);
			pDevice->ReadPosition(IDevice::STAGE_Y,pos);
			WIN_TRACE("Set Y Pos %f Read Y Pos %f\n",posStart,pos);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS,posEnd)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());
			hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
			CloseHandle(hThread);
			pDevice->ReadPosition(IDevice::STAGE_Y,pos);
			WIN_TRACE("Set Y Pos %f Read Y Pos %f\n",posEnd,pos);
		}

		WIN_ASSERT_TRUE(pDevice->PostflightPosition());
	}
}
END_TESTF

//Test the positioning capabilities of the Z stage
BEGIN_TESTF(StageZPositionTest,DeviceTestFixture)
{
	long paramType = 0;
	long paramAvailable = 0;
	long paramReadOnly = 0;
	double paramMin = 0.0;
	double paramMax = 0.0;
	double paramDefault = 0.0;

	WIN_ASSERT_TRUE(pDevice->SelectDevice(0)==TRUE);
	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::STAGE_Z)))
	{
		WIN_TRACE("Device is not a Z Stage\n");
	}
	else
	{
		//get the travel limits for the stage
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Z_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
		//hEvent = CreateEvent(0, FALSE, FALSE, 0);
		WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);
		long j;
		double posStart = paramMin + (paramMax-paramMin)*.3;
		double posEnd = paramMin + (paramMax-paramMin) * .5;
		double pos=0;
		//go back and forth 2 times
		for(j=0; j<3; j++)
		{
			MoveDevice(IDevice::PARAM_Z_POS, posStart);

			WaitAndStopDevice(IDevice::PARAM_Z_STOP, 15000);
	
			pDevice->ReadPosition(IDevice::STAGE_Z,pos);
			WIN_TRACE("Set Z Pos %f Read Z Pos %f\n",posStart,pos);

			MoveDevice(IDevice::PARAM_Z_POS, posEnd);
			WaitAndStopDevice(IDevice::PARAM_Z_STOP, 5000);

			pDevice->ReadPosition(IDevice::STAGE_Z,pos);
			WIN_TRACE("Set Z Pos %f Read Z Pos %f\n",posEnd,pos);
		}

		WIN_ASSERT_TRUE(pDevice->PostflightPosition());
	}
}
END_TESTF

//Test the positioning capabilities of the XY stage
BEGIN_TESTF(StageXYPositionTest,DeviceTestFixture)
{
	long paramXType = 0, paramYType = 0;
	long paramXAvailable = 0, paramYAvailable = 0;
	long paramXReadOnly = 0, paramYReadOnly = 0;
	double paramXMin = 0.0, paramYMin = 0.0;
	double paramXMax = 0.0, paramYMax = 0.0;
	double paramXDefault = 0.0, paramYDefault = 0.0;

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramXType, paramXAvailable, paramXReadOnly, paramXMin, paramXMax, paramXDefault)==TRUE);
	WIN_ASSERT_TRUE(paramXAvailable==TRUE);
	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramYType, paramYAvailable, paramYReadOnly, paramYMin, paramYMax, paramYDefault)==TRUE);
	WIN_ASSERT_TRUE(paramYAvailable==TRUE);

	if(!(static_cast<long>(paramXDefault) & static_cast<long>(IDevice::STAGE_X))||
		!(static_cast<long>(paramYDefault) & static_cast<long>(IDevice::STAGE_Y)))
	{
		WIN_TRACE("Device is not an X and Y Stage\n");
	}
	else
	{
		//get the travel limits for the stage X
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_POS, paramXType, paramXAvailable, paramXReadOnly, paramXMin, paramXMax, paramXDefault)==TRUE);
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_POS, paramYType, paramYAvailable, paramYReadOnly, paramYMin, paramYMax, paramYDefault)==TRUE);
		WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

		//go back and forth 2 times
		long j;
		double posXStart = 0.0, posYStart = 0.0;
		double posXEnd = 0.0, posYEnd = 0.0;
		double pos = 0.0;

		posXStart = paramXMin + (paramXMax-paramXMin)*.1;
		posXEnd = paramXMin + (paramXMax-paramXMin) * .9;
		posYStart = paramYMin + (paramYMax-paramYMin)*.1;
		posYEnd = paramYMin + (paramYMax-paramYMin) * .9;


		DWORD dwThreadId;
		hEvent = CreateEvent(0, FALSE, FALSE, 0);

		for(j=0; j<3; j++)
		{
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posXStart)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, posYStart)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			pDevice->ReadPosition(IDevice::STAGE_X, pos);
			WIN_TRACE("Set X Pos %f Read X Pos %f\n",posXStart, pos);
			pDevice->ReadPosition(IDevice::STAGE_Y, pos);
			WIN_TRACE("Set Y Pos %f Read Y Pos %f\n",posYStart, pos);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posXEnd)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, posYEnd)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());

			hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);

			pDevice->ReadPosition(IDevice::STAGE_X, pos);
			WIN_TRACE("Set X Pos %f Read X Pos %f\n",posXEnd, pos);
			pDevice->ReadPosition(IDevice::STAGE_Y, pos);
			WIN_TRACE("Set Y Pos %f Read Y Pos %f\n",posYEnd, pos);
		}

		WIN_ASSERT_TRUE(pDevice->PostflightPosition());
	}
}
END_TESTF

//Test the positioning capabilities of the XY stage
BEGIN_TESTF(StageXYZPositionTest,DeviceTestFixture)
{
	long paramXType = 0, paramYType = 0, paramZType = 0;
	long paramXAvailable = 0, paramYAvailable = 0, paramZAvailable = 0;
	long paramXReadOnly = 0, paramYReadOnly = 0, paramZReadOnly = 0;
	double paramXMin = 0.0, paramYMin = 0.0, paramZMin = 0.0;
	double paramXMax = 0.0, paramYMax = 0.0, paramZMax = 0.0;
	double paramXDefault = 0.0, paramYDefault = 0.0, paramZDefault = 0.0;

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramZType, paramZAvailable, paramZReadOnly, paramZMin, paramZMax, paramZDefault)==TRUE);
	WIN_ASSERT_TRUE(paramZAvailable==TRUE);

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramXType, paramXAvailable, paramXReadOnly, paramXMin, paramXMax, paramXDefault)==TRUE);
	WIN_ASSERT_TRUE(paramXAvailable==TRUE);

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramYType, paramYAvailable, paramYReadOnly, paramYMin, paramYMax, paramYDefault)==TRUE);
	WIN_ASSERT_TRUE(paramYAvailable==TRUE);

	if(!(static_cast<long>(paramXDefault) & static_cast<long>(IDevice::STAGE_X))||
		!(static_cast<long>(paramYDefault) & static_cast<long>(IDevice::STAGE_Y))||
		!(static_cast<long>(paramZDefault) & static_cast<long>(IDevice::STAGE_Z)))
	{
		WIN_TRACE("Device is not an X Y Z Stage\n");
	}
	else
	{
		//get the travel limits for the stage XYZ
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_POS, paramXType, paramXAvailable, paramXReadOnly, paramXMin, paramXMax, paramXDefault)==TRUE);
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_POS, paramYType, paramYAvailable, paramYReadOnly, paramYMin, paramYMax, paramYDefault)==TRUE);
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Z_POS, paramZType, paramZAvailable, paramZReadOnly, paramZMin, paramZMax, paramZDefault)==TRUE);
		WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

		//go back and forth 2 times
		long j;
		double posXStart = 0.0, posYStart = 0.0, posZStart = 0.0;
		double posXEnd = 0.0, posYEnd = 0.0, posZEnd = 0.0;
		double pos = 0.0;

		posXStart = paramXMin + (paramXMax-paramXMin)*.1;
		posXEnd = paramXMin + (paramXMax-paramXMin) * .9;
		posYStart = paramYMin + (paramYMax-paramYMin)*.1;
		posYEnd = paramYMin + (paramYMax-paramYMin) * .9;
		posZStart = paramZMin + (paramZMax-paramZMin)*.1;
		posZEnd = paramZMin + (paramZMax-paramZMin) * .9;

		DWORD dwThreadId;
		hEvent = CreateEvent(0, FALSE, FALSE, 0);

		for(j=0; j<3; j++)
		{
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posXStart)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, posYStart)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS, posZStart)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			pDevice->ReadPosition(IDevice::STAGE_X, pos);
			WIN_TRACE("Set X Pos %f Read X Pos %f\n",posXStart, pos);
			pDevice->ReadPosition(IDevice::STAGE_Y, pos);
			WIN_TRACE("Set Y Pos %f Read Y Pos %f\n",posYStart, pos);
			pDevice->ReadPosition(IDevice::STAGE_Z, pos);
			WIN_TRACE("Set Z Pos %f Read Z Pos %f\n",posZStart, pos);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posXEnd)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, posYEnd)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS, posZEnd)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());

			hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);

			pDevice->ReadPosition(IDevice::STAGE_X, pos);
			WIN_TRACE("Set X Pos %f Read X Pos %f\n",posXEnd, pos);
			pDevice->ReadPosition(IDevice::STAGE_Y, pos);
			WIN_TRACE("Set Y Pos %f Read Y Pos %f\n",posYEnd, pos);
			pDevice->ReadPosition(IDevice::STAGE_Z, pos);
			WIN_TRACE("Set Z Pos %f Read Z Pos %f\n",posZEnd, pos);

			posXEnd = paramXMin + (paramXMax-paramXMin)*.1;
			posXStart = paramXMin + (paramXMax-paramXMin) * .9;
			posYStart = paramYMin + (paramYMax-paramYMin)*.1;
			posYEnd = paramYMin + (paramYMax-paramYMin) * .9;
			posZEnd = paramZMin + (paramZMax-paramZMin)*.1;
			posZStart = paramZMin + (paramZMax-paramZMin) * .9;
		}

		double midY = (paramYMax + paramYMin)/2;
		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, midY)==TRUE);
		WIN_ASSERT_TRUE(pDevice->SetupPosition());
		WIN_ASSERT_TRUE(pDevice->StartPosition());
		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
		DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );
		WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

		WIN_ASSERT_TRUE(pDevice->PostflightPosition());
	}
}
END_TESTF

//Test the positioning capabilities of the Z stage - horizontal version
BEGIN_TESTF(StageHZPositionTest,DeviceTestFixture)
{
	long paramType = 0;
	long paramAvailable = 0;
	long paramReadOnly = 0;
	double paramMin = 0.0;
	double paramMax = 0.0;
	double paramDefault = 0.0;

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::STAGE_Z)))
	{
		WIN_TRACE("Device is not a Z Stage\n");
	}
	else
	{
		//get the travel limits for the stage
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Z_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
		WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

		double posStart = paramMin + (paramMax-paramMin)*.3;
		double posEnd = paramMin + (paramMax-paramMin) * .7;
		double pos;

		//move to the minimum position, stop the stepper after waiting for 10s
		MoveDevice(IDevice::PARAM_Z_POS, paramMin+0.1);
		WaitAndStopDevice(IDevice::PARAM_Z_STOP, 5000);

		pDevice->ReadPosition(IDevice::STAGE_Z,pos);
		WIN_TRACE("Set Z Pos %f Read Z Pos %f\n",paramMin+0.1,pos);

		//move to maximum position, stop the stepper after 10s regardless of whether finished
		MoveDevice(IDevice::PARAM_Z_POS, paramMax-0.1);
		WaitAndStopDevice(IDevice::PARAM_Z_STOP, 5000);

		pDevice->ReadPosition(IDevice::STAGE_Z,pos);
		WIN_TRACE("Set Z Pos %f Read Z Pos %f\n",paramMax-0.1,pos);

		//set a new zero reference at the start position
		WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_Z_ZERO,1)==TRUE);

		for(int i =0; i<2; i++)
		{
			//move to the start offset position
			MoveDevice(IDevice::PARAM_Z_POS, posStart);
			WaitAndStopDevice(IDevice::PARAM_Z_STOP, 10000);

			pDevice->ReadPosition(IDevice::STAGE_Z,pos);
			WIN_TRACE("Set Z Pos %f Read Z Pos %f\n",posStart,pos);

			//move to the end offset position
			MoveDevice(IDevice::PARAM_Z_POS, posEnd);
			WaitAndStopDevice(IDevice::PARAM_Z_STOP, 10000);

			pDevice->ReadPosition(IDevice::STAGE_Z,pos);
			WIN_TRACE("Set Z Pos %f Read Z Pos %f\n",posEnd,pos);
		}
		WIN_ASSERT_TRUE(pDevice->PostflightPosition());
	}
}
END_TESTF

//Test the zero set positioning and relative movements in the Z stage
BEGIN_TESTF(ZeroPositionTest, DeviceTestFixture)
{
	HANDLE hThread;
	DWORD dwThreadId;
	DWORD dwWait;

	long paramXType = 0, paramYType = 0, paramZType = 0;
	long paramXAvailable = 0, paramYAvailable = 0, paramZAvailable = 0;
	long paramXReadOnly = 0, paramYReadOnly = 0, paramZReadOnly = 0;
	double paramXMin = 0.0, paramYMin = 0.0, paramZMin = 0.0;
	double paramXMax = 0.0, paramYMax = 0.0, paramZMax = 0.0;
	double paramXDefault = 0.0, paramYDefault = 0.0, paramZDefault = 0.0;

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramZType, paramZAvailable, paramZReadOnly, paramZMin, paramZMax, paramZDefault)==TRUE);
	WIN_ASSERT_TRUE(paramZAvailable==TRUE);

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramXType, paramXAvailable, paramXReadOnly, paramXMin, paramXMax, paramXDefault)==TRUE);
	WIN_ASSERT_TRUE(paramXAvailable==TRUE);

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramYType, paramYAvailable, paramYReadOnly, paramYMin, paramYMax, paramYDefault)==TRUE);
	WIN_ASSERT_TRUE(paramYAvailable==TRUE);

	if(!(static_cast<long>(paramXDefault) & static_cast<long>(IDevice::STAGE_X))||
		!(static_cast<long>(paramYDefault) & static_cast<long>(IDevice::STAGE_Y))||
		!(static_cast<long>(paramZDefault) & static_cast<long>(IDevice::STAGE_Z)))
	{
		WIN_TRACE("Device is not an X Y Z Stage\n");
	}
	else
	{
		//get the travel limits for the stage X
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_POS, paramXType, paramXAvailable, paramXReadOnly, paramXMin, paramXMax, paramXDefault)==TRUE);
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_POS, paramYType, paramYAvailable, paramYReadOnly, paramYMin, paramYMax, paramYDefault)==TRUE);
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Z_POS, paramZType, paramZAvailable, paramZReadOnly, paramZMin, paramZMax, paramZDefault)==TRUE);
		WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

		hEvent = CreateEvent(0, FALSE, FALSE, 0);

		WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);
		double pos = 0.0;
		double intervalX = (paramXMax - paramXMin) / QUANTS;
		double intervalY = (paramYMax - paramYMin) / QUANTS;
		double intervalZ = (paramZMax - paramZMin) / QUANTS;

		for(int i = 0; i < QUANTS; i++)
		{
			//move to the end offset position
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, paramXMin + 1 * intervalX) == TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, paramXMin + 1 * intervalY) == TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS, paramZMin + 1 * intervalZ) == TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());
			hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
			CloseHandle(hThread);
			pDevice->ReadPosition(IDevice::STAGE_X, pos);
			WIN_TRACE("Set Zero Pos %f Read X Pos %f\n", paramXMin + 1 * intervalX, pos);
			pDevice->ReadPosition(IDevice::STAGE_Y, pos);
			WIN_TRACE("Set Zero Pos %f Read Y Pos %f\n", paramYMin + 1 * intervalY, pos);
			pDevice->ReadPosition(IDevice::STAGE_Z, pos);
			WIN_TRACE("Set Zero Pos %f Read Z Pos %f\n", paramYMin + 1 * intervalZ, pos);
			Sleep(1000);
			//set a new zero reference at the start position
			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_X_ZERO, 1) == TRUE);
			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_X_POS, 2.0 * intervalX ) == TRUE);
			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_Y_ZERO, 1) == TRUE);
			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_Y_POS, 2.0 * intervalY ) == TRUE);

			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_Z_ZERO, 1) == TRUE);

			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_Z_POS, 2.0 * intervalZ ) == TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());
			hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
			CloseHandle(hThread);
			pDevice->ReadPosition(IDevice::STAGE_X, pos);
			WIN_TRACE("Set + 2 intervals Pos %f Read X Pos %f\n", intervalX * 2.0, pos);
			pDevice->ReadPosition(IDevice::STAGE_Y, pos);
			WIN_TRACE("Set + 2 intervals Pos %f Read Y Pos %f\n", intervalY * 2.0, pos);
			pDevice->ReadPosition(IDevice::STAGE_Z, pos);
			WIN_TRACE("Set + 2 intervals Pos %f Read Z Pos %f\n", intervalZ * 2.0, pos);
			Sleep(1000);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, -2.0 * intervalX) == TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, -2.0 * intervalY) == TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS, -2.0 * intervalZ) == TRUE);
			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_ASSERT_TRUE(pDevice->StartPosition());
			hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
			CloseHandle(hThread);
			pDevice->ReadPosition(IDevice::STAGE_X, pos);
			WIN_TRACE("Set -2 intervals Pos %f Read X Pos %f\n", intervalX * - 2.0, pos);
			pDevice->ReadPosition(IDevice::STAGE_Y, pos);
			WIN_TRACE("Set -2 intervals Pos %f Read Y Pos %f\n", intervalY * - 2.0, pos);
			pDevice->ReadPosition(IDevice::STAGE_Z, pos);
			WIN_TRACE("Set -2 intervals Pos %f Read Z Pos %f\n", intervalZ * - 2.0, pos);
			Sleep(1000);
		}

		double midY = -50.0;
		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, midY)==TRUE);
		WIN_ASSERT_TRUE(pDevice->SetupPosition());
		WIN_ASSERT_TRUE(pDevice->StartPosition());
		hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
		dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );
		WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

		WIN_ASSERT_TRUE(pDevice->PostflightPosition());
	}

}
END_TESTF
/*
//Test the velocity and acceleration of the Z stage
BEGIN_TESTF(VelocityAccelTest, DeviceTestFixture)
{
long paramXType = 0, paramYType = 0, paramZType = 0;
long paramXAvailable = 0, paramYAvailable = 0, paramZAvailable = 0;
long paramXReadOnly = 0, paramYReadOnly = 0, paramZReadOnly = 0;
double paramXMin = 0.0, paramYMin = 0.0, paramZMin = 0.0;
double paramXMax = 0.0, paramYMax = 0.0, paramZMax = 0.0;
double paramXDefault = 0.0, paramYDefault = 0.0, paramZDefault = 0.0;

HANDLE hThread;
DWORD dwThreadId;
DWORD dwWait;

if(numDevices > 2)
{
WIN_ASSERT_TRUE(pDevice->SelectDevice(2)==TRUE);
WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramZType, paramZAvailable, paramZReadOnly, paramZMin, paramZMax, paramZDefault) == TRUE);
WIN_ASSERT_TRUE(paramZAvailable==TRUE);
}
WIN_ASSERT_TRUE(pDevice->SelectDevice(0)==TRUE);
WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramXType, paramXAvailable, paramXReadOnly, paramXMin, paramXMax, paramXDefault) == TRUE);
WIN_ASSERT_TRUE(paramXAvailable==TRUE);
if(numDevices > 1)
{
WIN_ASSERT_TRUE(pDevice->SelectDevice(1)==TRUE);
WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramYType, paramYAvailable, paramYReadOnly, paramYMin, paramYMax, paramYDefault) == TRUE);
WIN_ASSERT_TRUE(paramYAvailable==TRUE);
}
if(static_cast<long>(IDevice::STAGE_X) & static_cast<long>(IDevice::STAGE_X) << static_cast<long>(paramXDefault) &&
static_cast<long>(IDevice::STAGE_Y) & static_cast<long>(IDevice::STAGE_X) << static_cast<long>(paramYDefault) &&
static_cast<long>(IDevice::STAGE_Z) & static_cast<long>(IDevice::STAGE_X) << static_cast<long>(paramZDefault))
{
WIN_TRACE("Device has a XYZ stage\n");
//get the travel limits for the stage
WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_POS, paramXType, paramXAvailable, paramXReadOnly, paramXMin, paramXMax, paramXDefault) == TRUE);
WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_POS, paramYType, paramYAvailable, paramYReadOnly, paramYMin, paramYMax, paramYDefault) == TRUE);
WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Z_POS, paramZType, paramZAvailable, paramZReadOnly, paramZMin, paramZMax, paramZDefault) == TRUE);

double pos = 0.0;
double posXStart = paramXMin + (paramXMax-paramXMin) * .1;
double posXEnd = paramXMin + (paramXMax-paramXMin) * .9;
double posYStart = paramYMin + (paramYMax-paramYMin) * .1;
double posYEnd = paramYMin + (paramYMax-paramYMin) * .9;
double posZStart = paramZMin + (paramZMax-paramZMin) * .1;
double posZEnd = paramZMin + (paramZMax-paramZMin) * .9;

WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_VELOCITY, paramXType, paramXAvailable, paramXReadOnly, paramXMin, paramXMax, paramXDefault) == TRUE);
WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_VELOCITY, paramYType, paramYAvailable, paramYReadOnly, paramYMin, paramYMax, paramYDefault) == TRUE);
WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Z_VELOCITY, paramZType, paramZAvailable, paramZReadOnly, paramZMin, paramZMax, paramZDefault) == TRUE);

hEvent = CreateEvent(0, FALSE, FALSE, 0);
WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);
double velocityX = paramXMin;
double velocityY = paramYMin;
double velocityZ = paramZMin;

// test velocity
for(int i = 0; i < QUANTS; i++)
{
//move to the end offset position
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_VELOCITY, velocityX) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posXStart) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_VELOCITY, velocityY) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, posYStart) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_VELOCITY, velocityZ) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS, posZStart) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetupPosition());
WIN_ASSERT_TRUE(pDevice->StartPosition());
hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 50000 );
WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
CloseHandle(hThread);
pDevice->ReadPosition(IDevice::STAGE_X, pos);
WIN_TRACE("Set Zero Pos %f Read X Pos %f\n", posXStart, pos);
pDevice->ReadPosition(IDevice::STAGE_Y, pos);
WIN_TRACE("Set Zero Pos %f Read Y Pos %f\n", posYStart, pos);
pDevice->ReadPosition(IDevice::STAGE_Z, pos);
WIN_TRACE("Set Zero Pos %f Read Z Pos %f\n", posZStart, pos);

//move to the end offset position
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posXEnd) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, posYEnd) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS, posZEnd) == TRUE);

WIN_ASSERT_TRUE(pDevice->SetupPosition());
WIN_ASSERT_TRUE(pDevice->StartPosition());
hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 50000 );
WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
CloseHandle(hThread);
pDevice->ReadPosition(IDevice::STAGE_X, pos);
WIN_TRACE("Set Zero Pos %f Read X Pos %f\n", posXEnd, pos);
pDevice->ReadPosition(IDevice::STAGE_Y, pos);
WIN_TRACE("Set Zero Pos %f Read Y Pos %f\n", posYEnd, pos);
pDevice->ReadPosition(IDevice::STAGE_Z, pos);
WIN_TRACE("Set Zero Pos %f Read Z Pos %f\n", posZEnd, pos);

//move to the end offset position
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posXStart) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, posYStart) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS, posZStart) == TRUE);

WIN_ASSERT_TRUE(pDevice->SetupPosition());
WIN_ASSERT_TRUE(pDevice->StartPosition());
hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 50000 );
WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
CloseHandle(hThread);
pDevice->ReadPosition(IDevice::STAGE_X, pos);
WIN_TRACE("Set Zero Pos %f Read X Pos %f\n", posXStart, pos);
pDevice->ReadPosition(IDevice::STAGE_Y, pos);
WIN_TRACE("Set Zero Pos %f Read Y Pos %f\n", posYStart, pos);
pDevice->ReadPosition(IDevice::STAGE_Z, pos);
WIN_TRACE("Set Zero Pos %f Read Z Pos %f\n", posZStart, pos);
velocityX *= 2;
velocityY *= 2;
velocityZ *= 2;
}

WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_ACCEL, paramXType, paramXAvailable, paramXReadOnly, paramXMin, paramXMax, paramXDefault) == TRUE);
double accelX = paramXMin;
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_VELOCITY, velocityX) == TRUE);
WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_ACCEL, paramYType, paramYAvailable, paramYReadOnly, paramYMin, paramYMax, paramYDefault) == TRUE);
double accelY = paramYMin;
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_VELOCITY, velocityY) == TRUE);
WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Z_ACCEL, paramZType, paramZAvailable, paramZReadOnly, paramZMin, paramZMax, paramZDefault) == TRUE);
double accelZ = paramZMin;
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_VELOCITY, velocityZ) == TRUE);

// test acceleration
for(int i = 0; i < QUANTS; i++)
{
//move to the end offset position
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_ACCEL, accelX) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posXStart) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_ACCEL, accelY) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, posYStart) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_ACCEL, accelZ) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS, posZStart) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetupPosition());
WIN_ASSERT_TRUE(pDevice->StartPosition());
hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 50000 );
WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
CloseHandle(hThread);
pDevice->ReadPosition(IDevice::STAGE_X, pos);
WIN_TRACE("Set Zero Pos %f Read X Pos %f\n", posXStart, pos);
pDevice->ReadPosition(IDevice::STAGE_Y, pos);
WIN_TRACE("Set Zero Pos %f Read Y Pos %f\n", posYStart, pos);
pDevice->ReadPosition(IDevice::STAGE_Z, pos);
WIN_TRACE("Set Zero Pos %f Read Y Pos %f\n", posZStart, pos);

//move to the end offset position
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posXEnd) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, posYEnd) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS, posZEnd) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetupPosition());
WIN_ASSERT_TRUE(pDevice->StartPosition());
hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 50000 );
WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
CloseHandle(hThread);
pDevice->ReadPosition(IDevice::STAGE_X, pos);
WIN_TRACE("Set Zero Pos %f Read X Pos %f\n", posXEnd, pos);
pDevice->ReadPosition(IDevice::STAGE_Y, pos);
WIN_TRACE("Set Zero Pos %f Read Y Pos %f\n", posYEnd, pos);
pDevice->ReadPosition(IDevice::STAGE_Z, pos);
WIN_TRACE("Set Zero Pos %f Read Y Pos %f\n", posZStart, pos);

//move to the end offset position
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS, posXStart) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, posYStart) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS, posZStart) == TRUE);
WIN_ASSERT_TRUE(pDevice->SetupPosition());
WIN_ASSERT_TRUE(pDevice->StartPosition());
hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 50000 );
WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
CloseHandle(hThread);
pDevice->ReadPosition(IDevice::STAGE_X, pos);
WIN_TRACE("Set Zero Pos %f Read X Pos %f\n", posXStart, pos);
accelX *= 2;
WIN_TRACE("Set Zero Pos %f Read Y Pos %f\n", posYStart, pos);
accelY *= 2;
WIN_TRACE("Set Zero Pos %f Read Z Pos %f\n", posZStart, pos);
accelZ *= 2;
}
double midY = (paramYMax + paramYMin)/2;
WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS, midY)==TRUE);
WIN_ASSERT_TRUE(pDevice->SetupPosition());
WIN_ASSERT_TRUE(pDevice->StartPosition());
hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );
WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

WIN_ASSERT_TRUE(pDevice->PostflightPosition());
}
else
{
WIN_TRACE("Device does not have a X stage\n");
}

}
END_TESTF

*/

//Test the positioning capabilities of the X stage
BEGIN_TESTF(StageXHomeTest,DeviceTestFixture)
{
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	WIN_ASSERT_TRUE(pDevice->SelectDevice(0)==TRUE);
	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::STAGE_X)))
	{
		WIN_TRACE("Device is not an X Stage\n");
	}
	else
	{
		//get the travel limits for the stage
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_HOME, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

		if(paramAvailable)
		{
			hEvent = CreateEvent(0, FALSE, FALSE, 0);

			WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_HOME, paramMin)==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetupPosition());

			WIN_ASSERT_TRUE(pDevice->StartPosition());

			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);

			WIN_ASSERT_TRUE(pDevice->PostflightPosition());
		}
	}
}
END_TESTF



//Test the positioning capabilities of the Y stage
BEGIN_TESTF(StageYHomeTest,DeviceTestFixture)
{
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	WIN_ASSERT_TRUE(pDevice->SelectDevice(0)==TRUE);

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::STAGE_Y)))
	{
		WIN_TRACE("Device is not an X Y Z Stage\n");
	}
	else
	{
		//get the travel limits for the stage
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_HOME, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

		if(paramAvailable)
		{
			hEvent = CreateEvent(0, FALSE, FALSE, 0);

			WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_HOME, paramMin)==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetupPosition());

			WIN_ASSERT_TRUE(pDevice->StartPosition());

			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);

			WIN_ASSERT_TRUE(pDevice->PostflightPosition());
		}
	}
}
END_TESTF

BEGIN_TESTF(ThorPinholePositionTest, DeviceTestFixture)
{
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	long status;
	double paramMin;
	double paramMax;
	double paramDefault;
	long dev=0;
	int testcase=1;

	for(; dev<numDevices; dev++) {
		WIN_ASSERT_TRUE(pDevice->SelectDevice(dev)==TRUE);
		WIN_TRACE("Test Case %d: select device %d passed\n", testcase++, dev);
	}
	dev=-1;
	WIN_ASSERT_FALSE(pDevice->SelectDevice(dev));
	WIN_TRACE("Test Case %d: select device %d passed\n", testcase++, dev);
	dev=numDevices+1;
	WIN_ASSERT_FALSE(pDevice->SelectDevice(dev));
	WIN_TRACE("Test Case %d: select device %d passed\n", testcase++, dev);
	dev=numDevices+3542;
	WIN_ASSERT_FALSE(pDevice->SelectDevice(dev));
	WIN_TRACE("Test Case %d: select device %d passed\n", testcase++, dev);
	dev=numDevices+99999;
	WIN_ASSERT_FALSE(pDevice->SelectDevice(dev));
	WIN_TRACE("Test Case %d: select device %d passed\n", testcase++, dev);

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	WIN_ASSERT_TRUE(pDevice->StatusPosition(status)==TRUE);

	WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::PINHOLE_WHEEL)))
	{
		WIN_TRACE("Device is not a Pinhole Wheel");
	}
	else
	{
		long status = IDevice::STATUS_READY;

		long mode=-11;
		WIN_ASSERT_FALSE(pDevice->SetParam(IDevice::PARAM_PINHOLE_ALIGNMENT_MODE, mode));
		WIN_TRACE("Test Case %d: set to mode %d passed\n", testcase++, mode);
		mode=-1;
		WIN_ASSERT_FALSE(pDevice->SetParam(IDevice::PARAM_PINHOLE_ALIGNMENT_MODE, mode));
		WIN_TRACE("Test Case %d: set to mode %d passed\n", testcase++, mode);
		mode=0;
		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PINHOLE_ALIGNMENT_MODE, mode)==TRUE);
		WIN_TRACE("Test Case %d: set to mode %d passed\n", testcase++, mode);
		mode=1;
		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PINHOLE_ALIGNMENT_MODE, mode)==TRUE);
		WIN_TRACE("Test Case %d: set to mode %d passed\n", testcase++, mode);

		double pos=-234;
		WIN_ASSERT_FALSE(pDevice->SetParam(IDevice::PARAM_PINHOLE_ALIGNMENT_POS, pos));
		WIN_ASSERT_TRUE(pDevice->SetupPosition()==TRUE);
		WIN_TRACE("Test Case %d: set position to %g in mode 1 passed\n", testcase++, pos);

		pos=12340;
		WIN_ASSERT_FALSE(pDevice->SetParam(IDevice::PARAM_PINHOLE_ALIGNMENT_POS, pos));
		WIN_ASSERT_TRUE(pDevice->SetupPosition()==TRUE);
		WIN_TRACE("Test Case %d: set position to %g in mode 1 passed\n", testcase++, pos);

		pos=230;
		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PINHOLE_ALIGNMENT_POS, pos)==TRUE);
		WIN_ASSERT_TRUE(pDevice->SetupPosition()==TRUE);
		WIN_ASSERT_TRUE(pDevice->StartPosition()==TRUE);
		do
		{
			WIN_ASSERT_TRUE(pDevice->StatusPosition(status));
		}while(status == IDevice::STATUS_BUSY);
		WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_PINHOLE_ALIGNMENT_POS,pos)==TRUE);
		WIN_TRACE("Test Case %d: set position to %g in mode 1 passed\n", testcase++, pos);


		mode=0;

		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PINHOLE_ALIGNMENT_MODE, mode)==TRUE);

		for(int d=-20; d<100; d+=3) 
		{
			double pos = static_cast<double>(d % 23);
			if(static_cast<long>(pos)>=0 && static_cast<long>(pos)<=15)
			{
				WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PINHOLE_POS, pos)==TRUE);
				WIN_ASSERT_TRUE(pDevice->SetupPosition()==TRUE);
				WIN_ASSERT_TRUE(pDevice->StartPosition()==TRUE);
				do
				{
					WIN_ASSERT_TRUE(pDevice->StatusPosition(status));
				}while(status == IDevice::STATUS_BUSY);
				WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_PINHOLE_POS,pos));
			}
			else 
			{
				WIN_ASSERT_FALSE(pDevice->SetParam(IDevice::PARAM_PINHOLE_POS, pos));
				WIN_ASSERT_TRUE(pDevice->SetupPosition()==TRUE);
			}
			WIN_TRACE("Test Case %d: move to pinhole index %g passed\n", testcase++, pos);
		}


		WIN_ASSERT_TRUE(pDevice->PostflightPosition()==TRUE);
	}

}
END_TESTF

BEGIN_TESTF(ThorECUTest,DeviceTestFixture)
{
	long paramType;
	long paramAvailable1,paramAvailable2,paramAvailable3;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	double param;

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable1, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

	WIN_ASSERT_TRUE(paramAvailable1==TRUE);

	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::PMT1 | IDevice::PMT2 | IDevice::PMT3 | IDevice::PMT4)))
	{
		WIN_TRACE("Device is not an ECU\n");
	}
	else
	{
		hEvent = CreateEvent(0, FALSE, FALSE, 0);
		
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_PMT2_SAFETY, paramType, paramAvailable1, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
		if(paramAvailable1)
		{
			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_PMT1_SAFETY,paramDefault)==TRUE);
			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_PMT2_SAFETY,paramDefault)==TRUE);
			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_PMT3_SAFETY,paramDefault)==TRUE);
			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_PMT4_SAFETY,paramDefault)==TRUE);
		}

		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_PMT3_ENABLE, paramType, paramAvailable3, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_PMT1_GAIN_POS, paramType, paramAvailable2, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

		if(paramAvailable2 && paramAvailable3)
		{
			WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT1_ENABLE, 1)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT1_GAIN_POS, paramMin)==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT2_ENABLE, 1)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT2_GAIN_POS, (paramMin+paramMax)/40)==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT3_ENABLE, 1)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT3_GAIN_POS, (paramMin+paramMax)/20)==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT4_ENABLE, 1)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT4_GAIN_POS, (paramMin+paramMax)/10)==TRUE);

			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_SCANNER_ZOOM_POS, paramType, paramAvailable1, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
	
			if(paramAvailable1)
			{
				WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_SCANNER_ENABLE, 1)==TRUE);
				WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_SCANNER_ZOOM_POS, (paramMin+paramMax)/3)==TRUE);
			}

			WIN_ASSERT_TRUE(pDevice->SetupPosition());

			WIN_ASSERT_TRUE(pDevice->StartPosition());

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT1_GAIN_POS, paramMin)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT1_ENABLE, 0)==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT2_GAIN_POS, paramMin)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT2_ENABLE, 0)==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT3_GAIN_POS, paramMin)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT3_ENABLE, 0)==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT4_GAIN_POS, paramMin)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_PMT4_ENABLE, 0)==TRUE);
		}

		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_SCANNER_ZOOM_POS, paramType, paramAvailable1, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
	
		if(paramAvailable1)
		{
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_SCANNER_ENABLE, 0)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_SCANNER_ZOOM_POS, paramMin)==TRUE);
		}

		WIN_ASSERT_TRUE(pDevice->SetupPosition());

		WIN_ASSERT_TRUE(pDevice->StartPosition());

		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_PMT2_SAFETY, paramType, paramAvailable1, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
		if(paramAvailable1)
		{
			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_PMT1_SAFETY,param)==TRUE);
			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_PMT2_SAFETY,param)==TRUE);
			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_PMT3_SAFETY,param)==TRUE);
			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_PMT4_SAFETY,param)==TRUE);
		}

		DWORD dwThreadId;

		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

		DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );

		WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

		CloseHandle(hThread);

		WIN_ASSERT_TRUE(pDevice->PostflightPosition());
	}
}
END_TESTF

BEGIN_TESTF(ThorMLSStageTest,DeviceTestFixture)
{
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	double param;

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	if(!(static_cast<long>(paramDefault) & static_cast<long>(IDevice::STAGE_X | IDevice::STAGE_Y)))
	{
		WIN_TRACE("Device is not a ThorMLSStage\n");
	}
	else
	{
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

		if(paramAvailable)
		{
			hEvent = CreateEvent(0, FALSE, FALSE, 0);

			WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);
			WIN_TRACE("PreflightPosition Succeeded.");

			WIN_ASSERT_TRUE(GetParam(pDevice,IDevice::PARAM_X_VELOCITY,paramDefault)==TRUE);
			WIN_TRACE("GetParam(X Velocity) Succeeded. Default x velocity is %g\n", paramDefault);

			WIN_ASSERT_TRUE(GetParam(pDevice,IDevice::PARAM_X_ACCEL,paramDefault)==TRUE);
			WIN_TRACE("GetParam(X Acceleration) Succeeded. Default x acceleration is %g\n", paramDefault);

			WIN_ASSERT_TRUE(GetParam(pDevice,IDevice::PARAM_Y_ACCEL,paramDefault)==TRUE);
			WIN_TRACE("GetParam(Y Velocity) Succeeded. Default y velocity is %g\n", paramDefault);

			WIN_ASSERT_TRUE(GetParam(pDevice,IDevice::PARAM_Y_VELOCITY,paramDefault)==TRUE);
			WIN_TRACE("GetParam(Y Acceleration) Succeeded. Default y acceleration is %g\n", paramDefault);

			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_X_POS,paramMax)==TRUE);
			WIN_TRACE("SetParam(X Position) Succeeded. set X pos is %g\n", paramMax);

			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_Y_POS,paramMin)==TRUE);
			WIN_TRACE("SetParam(Y Position) Succeeded. set Y pos is %g\n", paramMin);

			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_TRACE("SetupPosition Succeeded.\n");

			WIN_ASSERT_TRUE(pDevice->StartPosition());
			WIN_TRACE("StartPosition ...\n\n");

			DWORD dwThreadId;
			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
			CloseHandle(hThread);

			WIN_ASSERT_TRUE(pDevice->ReadPosition(IDevice::STAGE_X, paramDefault)==TRUE);
			WIN_TRACE("ReadPosition(X) Succeeded. X pos is %g\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->ReadPosition(IDevice::STAGE_Y, paramDefault)==TRUE);
			WIN_TRACE("ReadPosition(Y) Succeeded. Y pos is %g\n\n", paramDefault);

			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_X_VELOCITY,paramDefault/2)==TRUE);
			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_Y_VELOCITY,paramDefault/4)==TRUE);

			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_X_POS,paramMin)==TRUE);
			WIN_TRACE("SetParam(X Position) Succeeded. set X pos is %g\n", paramMin);

			WIN_ASSERT_TRUE(SetParam(pDevice,IDevice::PARAM_Y_POS,paramMax)==TRUE);
			WIN_TRACE("SetParam(Y Position) Succeeded. set Y pos is %g\n", paramMax);

			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_TRACE("SetupPosition Succeeded.\n");

			WIN_ASSERT_TRUE(pDevice->StartPosition());
			WIN_TRACE("StartPosition ...\n");

			
			hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );
			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
			CloseHandle(hThread);

			WIN_ASSERT_TRUE(pDevice->ReadPosition(IDevice::STAGE_X, param)==TRUE);
			WIN_TRACE("ReadPosition(X) Succeeded. X pos is %g\n", param);

			WIN_ASSERT_TRUE(pDevice->ReadPosition(IDevice::STAGE_Y, param)==TRUE);
			WIN_TRACE("ReadPosition(Y) Succeeded. Y pos is %g\n\n", param);

			WIN_ASSERT_TRUE(GetParam(pDevice,IDevice::PARAM_X_VELOCITY,paramDefault)==TRUE);
			WIN_TRACE("GetParam(X Velocity) Succeeded. Default x velocity is %g\n", paramDefault);

			WIN_ASSERT_TRUE(GetParam(pDevice,IDevice::PARAM_X_ACCEL,paramDefault)==TRUE);
			WIN_TRACE("GetParam(X Acceleration) Succeeded. Default x acceleration is %g\n", paramDefault);

			WIN_ASSERT_TRUE(GetParam(pDevice,IDevice::PARAM_Y_ACCEL,paramDefault)==TRUE);
			WIN_TRACE("GetParam(Y Velocity) Succeeded. Default y acceleration is %g\n", paramDefault);

			WIN_ASSERT_TRUE(GetParam(pDevice,IDevice::PARAM_Y_VELOCITY,paramDefault)==TRUE);
			WIN_TRACE("GetParam(Y Acceleration) Succeeded. Default y velocity is %g\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->PostflightPosition());
			WIN_TRACE("PostflightPosition(Y) Succeeded. \n");
		}
	}
}
END_TESTF

BEGIN_TESTF(ThorBScopeTest,DeviceTestFixture)
{
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	double param;

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	long targetDevType = static_cast<long>(IDevice::STAGE_X | IDevice::STAGE_Y | IDevice::STAGE_Z | IDevice::STAGE_R);
	if((static_cast<long>(paramDefault) & targetDevType) != targetDevType)
	{
		WIN_TRACE("Device is not a ThorBScope\n");
	}
	else
	{
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

		if(paramAvailable)
		{
			hEvent = CreateEvent(0, FALSE, FALSE, 0);
	
			WIN_ASSERT_TRUE(pDevice->PreflightPosition()==TRUE);
			WIN_TRACE("PreflightPosition Succeeded.\n");

			// Display current position:
			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_X_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(X) Succeeded. X current pos is %g\n", param);

			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_Y_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(Y) Succeeded. Y current pos is %g\n", param);

			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_Z_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(Z) Succeeded. Z current pos is %g\n", param);

			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_R_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(R) Succeeded. R current pos is %g\n\n", param);

			// Move to 1st negative position with slower than default velocity:
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS,(paramMin)/20)==TRUE);
			WIN_TRACE("SetParam(X Position) Succeeded. set X pos is %g\n", (paramMin)/20);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS,(paramMin)/20)==TRUE);
			WIN_TRACE("SetParam(Y Position) Succeeded. set Y pos is %g\n", (paramMin)/20);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS,(paramMin)/20)==TRUE);
			WIN_TRACE("SetParam(Z Position) Succeeded. set Z pos is %g\n", (paramMin)/20);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_R_POS,(paramMin)/20)==TRUE);
			WIN_TRACE("SetParam(R Position) Succeeded. set R pos is %g\n\n", (paramMin)/20);
			
			//Assert negative velocity is false & set slower velocity:
			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_VELOCITY, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
						
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_VELOCITY,-1)==FALSE);			
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_VELOCITY,(paramMax)/3)==TRUE);
			WIN_TRACE("SetParam(X Velocity) Succeeded. set X Velocity is %g\n", (paramMax)/3);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_VELOCITY,-1)==FALSE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_VELOCITY,(paramMax)/3)==TRUE);
			WIN_TRACE("SetParam(Y Velocity) Succeeded. set Y Velocity is %g\n", (paramMax)/3);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_VELOCITY,-1)==FALSE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_VELOCITY,(paramMax)/3)==TRUE);
			WIN_TRACE("SetParam(Z Velocity) Succeeded. set Z Velocity is %g\n", (paramMax)/3);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_R_VELOCITY,-1)==FALSE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_R_VELOCITY,(paramMax)/3)==TRUE);
			WIN_TRACE("SetParam(R Velocity) Succeeded. set R Velocity is %g\n\n", (paramMax)/3);

			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_TRACE("SetupPosition Succeeded.\n");

			WIN_ASSERT_TRUE(pDevice->StartPosition());
			WIN_TRACE("StartPosition ...\n\n");

			DWORD dwThreadId;
			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );
			//WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
			CloseHandle(hThread);
			
			// Confirm 1st position is corrent: -5 mm, -5 degree
			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
			
			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_X_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(X) Succeeded. X current pos is %g\n", param);

			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_Y_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(Y) Succeeded. Y current pos is %g\n", param);

			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_Z_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(Z) Succeeded. Z current pos is %g\n", param);

			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_R_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(R) Succeeded. R current pos is %g\n\n", param);
			
			// Move to 2nd/default/zero position with default velocity:
			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_VELOCITY, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
			
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_VELOCITY,paramDefault)==TRUE);
			WIN_TRACE("SetParam(X Velocity) Succeeded. set X Velocity is %g\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_VELOCITY,paramDefault)==TRUE);
			WIN_TRACE("SetParam(Y Velocity) Succeeded. set Y Velocity is %g\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_VELOCITY,paramDefault)==TRUE);
			WIN_TRACE("SetParam(Z Velocity) Succeeded. set Z Velocity is %g\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_R_VELOCITY,paramDefault)==TRUE);
			WIN_TRACE("SetParam(R Velocity) Succeeded. set R Velocity is %g\n\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS,paramDefault)==TRUE);
			WIN_TRACE("SetParam(X Position) Succeeded. set X pos is %g\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS,paramDefault)==TRUE);
			WIN_TRACE("SetParam(Y Position) Succeeded. set Y pos is %g\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_POS,paramDefault)==TRUE);
			WIN_TRACE("SetParam(Z Position) Succeeded. set Z pos is %g\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_R_POS,paramDefault)==TRUE);
			WIN_TRACE("SetParam(R Position) Succeeded. set R pos is %g\n\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->SetupPosition());
			WIN_TRACE("SetupPosition Succeeded.\n");
			WIN_ASSERT_TRUE(pDevice->StartPosition());
			WIN_TRACE("StartPosition ...\n\n");
			
			long status=IDevice::STATUS_READY;
			WIN_ASSERT_TRUE(pDevice->StatusPosition(status));			
			if(status == IDevice::STATUS_BUSY)
			{
				hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );
				dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );
				WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);
				CloseHandle(hThread);
			}
			
			// Confirm 2nd position is corrent: 0 mm, 0 degree
			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Z_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
			
			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_X_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(X) Succeeded. X current pos is %g\n", param);

			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_Y_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(Y) Succeeded. Y current pos is %g\n", param);

			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_Z_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(Z) Succeeded. Z current pos is %g\n", param);

			WIN_ASSERT_TRUE(pDevice->GetParam(IDevice::PARAM_R_POS_CURRENT, param)==TRUE);
			WIN_TRACE("GetParam(R) Succeeded. R current pos is %g\n\n", param);

			//Display default velocities and accelerations & Assert negative acceleration setting is false:
			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_VELOCITY, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
			WIN_TRACE("Default X velocity is %g (steps/s)\n", paramDefault);
			
			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_VELOCITY, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
			WIN_TRACE("Default Y velocity is %g (steps/s)\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Z_VELOCITY, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
			WIN_TRACE("Default Z velocity is %g (steps/s)\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_R_VELOCITY, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
			WIN_TRACE("Default R velocity is %g (steps/s)\n\n", paramDefault);
			
			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_X_ACCEL, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_ACCEL,-1)==FALSE);
			WIN_TRACE("Default X acceleration is %g (steps/s^2)\n", paramDefault);
		
			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_ACCEL, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_ACCEL,-1)==FALSE);
			WIN_TRACE("Default Y acceleration is %g (steps/s^2)\n", paramDefault);			

			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Z_ACCEL, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Z_ACCEL,-1)==FALSE);
			WIN_TRACE("Default Z acceleration is %g (steps/s^2)\n", paramDefault);
			
			WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_R_ACCEL, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_R_ACCEL,-1)==FALSE);
			WIN_TRACE("Default R acceleration is %g (steps/s^2)\n\n", paramDefault);

			WIN_ASSERT_TRUE(pDevice->PostflightPosition());
			WIN_TRACE("PostflightPosition(Y) Succeeded. \n");
		}
	}
}
END_TESTF
