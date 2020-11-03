

#include "..\..\..\Tools\WinUnit\Include\WinUnit.h"
#include "..\..\..\Common\Camera.h"
#include "Strsafe.h"

#pragma warning( push )
#pragma warning( disable : 4995 )

#include <time.h>
#include <memory>

using namespace std;

namespace
{
	char * pMemoryBuffer = NULL;
	long maxCameraWidth = 4096;
	long maxCameraHeight = 4096;
	const long BYTES_PER_PIXEL = 2;
	const long INDEX_SIZE_BYTES = 4;
	CameraDll * pCamera = NULL;
	long numCameras = 0;
	long colorChannels = 3;
	long maxLSMChannels = 2;
	HANDLE hEvent = NULL;
	wchar_t cameraPathandName[MAX_PATH] = _T("");
	long status = 0;
	const long TWO_WAY_SCAN= 0;
	const long FORWARD_SCAN = 1;
	wchar_t errMsg[_MAX_PATH];

#define SAVE_TEST_IMAGES
#ifdef SAVE_TEST_IMAGES
#include "..\..\..\Tools\tiff-3.8.2\libtiff\tiffio.h"
#include "..\..\..\Tools\tiff-3.8.2\include\tifflib.h"
	auto_ptr<TiffLibDll> tiffDll(new TiffLibDll(L".\\libtiff3.dll"));
#endif

#define CamErrCheck(fnCall) ((TRUE == (fnCall)) ? (TRUE) : (ReportError()))
}

void SaveImage(wchar_t *baseName,long index,long width, long height, char * p1);

long ReportError()
{
	pCamera->GetLastErrorMsg(errMsg,_MAX_PATH);

	WIN_TRACE(_T("%s\n"),errMsg);
	return FALSE;
}


FIXTURE(CameraTestFixture);


SETUP(CameraTestFixture)
{
	WIN_ASSERT_TRUE(WinUnit::Environment::GetVariable(_T("CameraPathandName"),cameraPathandName,ARRAYSIZE(cameraPathandName)), _T("Environment variable CameraPathandName was not set.  Use --CameraPathandName option."));

	pCamera = new CameraDll(cameraPathandName);

	WIN_ASSERT_TRUE(pCamera != NULL);


	WIN_ASSERT_TRUE(CamErrCheck(pCamera->FindCameras(numCameras)));

	WIN_ASSERT_TRUE(numCameras > 0);

	WIN_TRACE(_T("Number of Cameras %d\n"),numCameras);

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->SelectCamera(0)));


	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal == ICamera::LSM)
		{
			long paramType;
			long paramAvailable;
			long paramReadOnly;
			double paramMin;
			double paramMax;
			double paramDefault;

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParamInfo(ICamera::PARAM_LSM_CHANNEL, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)));

			maxLSMChannels = static_cast<long>(paramMax);

			switch(maxLSMChannels)
			{
			case 0x3:
				colorChannels = 2;
				break;
			default:
				colorChannels = 4;
			}
		}
	}

	pMemoryBuffer = new char[maxCameraWidth * maxCameraHeight * BYTES_PER_PIXEL * colorChannels + INDEX_SIZE_BYTES];

	WIN_ASSERT_TRUE(pMemoryBuffer != NULL);

	memset(pMemoryBuffer,0,maxCameraWidth * maxCameraHeight * BYTES_PER_PIXEL * colorChannels +INDEX_SIZE_BYTES);

	WIN_TRACE(_T("CameraTestFixture Setup\n"),status);
}

TEARDOWN(CameraTestFixture)
{
	WIN_ASSERT_TRUE(pCamera->TeardownCamera()==TRUE);

	//delete pCamera;
	//delete pMemoryBuffer;

	pCamera = NULL;
	pMemoryBuffer = NULL;

	WIN_TRACE(_T("CameraTestFixture Teardown\n"),status);
}

UINT StatusThreadProc( void *pParam )
{
	long status = ICamera::STATUS_BUSY;

	while(status == ICamera::STATUS_BUSY)
	{
		if(FALSE == CamErrCheck(pCamera->StatusAcquisition(status)))
		{
			break;
		}
	}
	long* pStatus=(long*) pParam;
	*pStatus=status;

	SetEvent( hEvent);

	return 0;
}
#undef DBL_EPSILON
#define	DBL_EPSILON 1e-10
#define	DOUBLE_EQ(x,v) (((v - DBL_EPSILON) < x) && (x <( v + DBL_EPSILON)))

//This test is for the parameters associated with a CCD camera
BEGIN_TESTF(GetSetParamTestCCD,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::CCD)
		{
			return 1;
		}
	}

	long i;

	long paramID;
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	//for every camera iterate through all of the parameters and do get/set tests and verify the results
	for(i=0; i<numCameras; i++)
	{
		for(paramID=ICamera::PARAM_FIRST_PARAM; paramID < ICamera::PARAM_LAST_PARAM; paramID++)
		{
			WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)));

			if(TRUE == paramAvailable)
			{
				double val;

				//if the parameter is read only make sure the get param function works
				if(TRUE == paramReadOnly)
				{
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)),_T("paramID %d readonly value %f"),paramID,val);
				}
				else
				{
					//min
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,paramMin)));
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)));
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMin,val)==TRUE);

					//max
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,paramMax)));
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)));
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMax,val)==TRUE);

					//default
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,paramDefault)));
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)));
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
						if(paramType == ICamera::TYPE_LONG)
						{
							newVal = static_cast<long>(newVal);
						}

						//random value
						WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,newVal)),_T("paramID %d newVal %f val %f"),paramID,newVal,val);
						WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)),_T("paramID %d newVal %f val %f"),paramID,newVal,val);
						WIN_ASSERT_TRUE(DOUBLE_EQ(newVal,val)==TRUE,_T("paramID %d newVal %f val %f"),paramID,newVal,val);
					}



					WIN_TRACE(_T("Below Min Test %f\n"),paramMin - 1.0);
					//below min
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,paramMin - 1.0))==FALSE);
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)));
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMin-1,val)==FALSE);

					WIN_TRACE(_T("Above Max Test %f\n"),paramMin + 1.0);
					//above max
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,paramMax + 1.0))==FALSE);
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)));
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMax+1,val)==FALSE);
				}
			}
			else
			{
				double val;

				WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,paramDefault))==FALSE);
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val))==FALSE);
			}
		}
	}
}
END_TESTF

//Test Get/Set for all the parameters for the laser scanning camera
BEGIN_TESTF(GetSetParamTestLSM,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	long i;


	//for every camera iterate through all of the parameters and do get/set tests and verify the results
	for(i=0; i<numCameras; i++)
	{
		long paramID=0;
		long paramType=0;
		long paramAvailable=0;
		long paramReadOnly=0;
		double paramMin=0;
		double paramMax=0;
		double paramDefault=0;
		for(paramID=ICamera::PARAM_FIRST_PARAM; paramID < ICamera::PARAM_LAST_PARAM; paramID++)
		{
			WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)));

			if(TRUE == paramAvailable)
			{
				WIN_TRACE(L"paramID %d\n paramType %d\n paramAvail %d\n paramReadOnly %d\n paramMin %f\n paramMax %f\n paramDefault %f\n",paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);

				double val;

				//if the parameter is read only make sure the get param function works
				if(TRUE == paramReadOnly)
				{
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)));
					WIN_TRACE(_T("paramID %d readonly value %d\n"),paramID,val);
				}
				else
				{
					if((paramID == ICamera::PARAM_MULTI_FRAME_COUNT)||
						(paramID == ICamera::PARAM_LSM_PIXEL_X)||
						(paramID == ICamera::PARAM_LSM_PIXEL_Y)||
						(paramID == ICamera::PARAM_LSM_OFFSET_X)||
						(paramID == ICamera::PARAM_LSM_OFFSET_Y)||
						(paramID == ICamera::PARAM_LSM_INPUTRANGE3)||
						(paramID == ICamera::PARAM_LSM_INPUTRANGE4)||
						(paramID == ICamera::PARAM_LSM_INTERNALCLOCKRATE)||
						(paramID == ICamera::PARAM_LSM_EXTERNALCLOCKRATE)
						)
					{
						continue;
					}
					//min
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,paramMin)),_T("paramID %d"),paramID);
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)),_T("paramID %d"),paramID);
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMin,val)==TRUE,_T("paramID %d"),paramID);

					//max
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,paramMax)),_T("paramID %d"),paramID);
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)),_T("paramID %d"),paramID);
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMax,val)==TRUE,_T("paramID %d"),paramID);

					//default
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,paramDefault)),_T("paramID %d"),paramID);
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)),_T("paramID %d"),paramID);
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramDefault,val)==TRUE,_T("paramID %d"),paramID);

					//if min max are not equal send a random value in between
					if(DOUBLE_EQ(paramMin,paramMax)==FALSE)
					{
						double newVal;
						double randDouble;

						srand(static_cast<unsigned int>(time(NULL)));

						randDouble = (   static_cast<double>(rand()) / (static_cast<double>(RAND_MAX)+static_cast<double>(1)) );

						newVal = randDouble * (paramMax - paramMin) + paramMin;

						//if long data type cast before setting the parameter
						if(paramType == ICamera::TYPE_LONG)
						{
							newVal = static_cast<long>(newVal);
						}

						//random value
						WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,newVal)),_T("paramID %d newVal %f val %f"),paramID,newVal,val);
						WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)),_T("paramID %d newVal %f val %f"),paramID,newVal,val);
						WIN_ASSERT_TRUE(DOUBLE_EQ(newVal,val)==TRUE,_T("paramID %d newVal %f val %f"),paramID,newVal,val);
					}

					WIN_TRACE(_T("Below Min Test %f\n"),paramMin - 1.0);
					//below min
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,paramMin - 1.0))==FALSE,_T("paramID %d"),paramID);
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)),_T("paramID %d"),paramID);
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMin-1,val)==FALSE,_T("paramID %d"),paramID);

					WIN_TRACE(_T("Above Max Test %f\n"),paramMax + 1.0);
					//above max
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(paramID,paramMax + 1.0))==FALSE,_T("paramID %d"),paramID);
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParam(paramID,val)),_T("paramID %d"),paramID);
					WIN_ASSERT_TRUE(DOUBLE_EQ(paramMax+1,val)==FALSE),_T("paramID %d"),paramID;
				}
			}
			else
			{
				double val;

				WIN_ASSERT_TRUE(pCamera->SetParam(paramID,paramDefault)==FALSE,_T("paramID %d should not be available"),paramID);
				WIN_ASSERT_TRUE(pCamera->GetParam(paramID,val)==FALSE,_T("paramID %d should not be available"),paramID);
			}
		}
	}
}
END_TESTF

//Test the single image capture mode for a CCD camera
BEGIN_TESTF(SingleCaptureTestCCD,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::CCD)
		{
			return 1;
		}
	}

	long paramID=0;
	long paramType=0;
	long paramAvailable=0;
	long paramReadOnly=0;
	double paramMin=0;
	double paramMax=0;
	double paramDefault=0;

	long imageWidth;
	long imageHeight;

	WIN_ASSERT_TRUE(pCamera->GetParamInfo(ICamera::PARAM_CAPTURE_REGION_RIGHT,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault));
	imageWidth = static_cast<long>(paramMax);
	WIN_ASSERT_TRUE(pCamera->GetParamInfo(ICamera::PARAM_CAPTURE_REGION_BOTTOM,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault));
	imageHeight = static_cast<long>(paramMax);
	WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_LEFT,0)));
	WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT,imageWidth)));
	WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_TOP,0)));
	WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM,imageHeight)));

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)));

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));

	SaveImage(L"SingleCCD",0,imageWidth, imageHeight, pMemoryBuffer);

}
END_TESTF



//Test will scan through the range of pixel settings for forward scan and two way scan
//capturing and saving as it loops
BEGIN_TESTF(LSCapturePixelSizeTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)) == TRUE);

	int i=0;
	int x;
	hEvent = CreateEvent(0, FALSE, FALSE, 0);
	long status=ICamera::STATUS_READY;


	////x y pixel number validation
	//forward scan loop
	for(x=64; x<=4096; x+=128)
	{
		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, 120))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
		{
			WIN_TRACE(_T("PARAM_LSM_PIXEL_X %d\n"),x);

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));
			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, INFINITE );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			if (status==ICamera::STATUS_ERROR)
			{
				WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
			}
			else
			{
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));

				wchar_t baseName[_MAX_PATH];
				StringCbPrintfW(baseName,_MAX_PATH,L"LSMPixSizeR_%04d_x_%04d",x,x);
				SaveImage(baseName,i,x, x, pMemoryBuffer);
				i++;
				StringCbPrintfW(baseName,_MAX_PATH,L"LSMPixSizeG_%04d_x_%04d",x,x);
				SaveImage(baseName,i,x, x, pMemoryBuffer+x*x*2);
			}
			ResetEvent( hEvent );
		}
	}

	//two way scan loop
	for(x=64; x<=2048; x+=128)
	{
		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,TWO_WAY_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, 120))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
		{
			WIN_TRACE(_T("PARAM_LSM_PIXEL_X %d\n"),x);

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));
			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, INFINITE );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			if (status==ICamera::STATUS_ERROR)
			{
				WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
			}
			else
			{
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));

				wchar_t baseName[_MAX_PATH];
				StringCbPrintfW(baseName,_MAX_PATH,L"LSMPixSizeR_%04d_x_%04d",x,x);
				SaveImage(baseName,i,x, x, pMemoryBuffer);
				i++;
				StringCbPrintfW(baseName,_MAX_PATH,L"LSMPixSizeG_%04d_x_%04d",x,x);
				SaveImage(baseName,i,x, x, pMemoryBuffer+x*x*2);
			}
			ResetEvent( hEvent );
		}
	}


	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));
}
END_TESTF

//Test the NON square area mode options
BEGIN_TESTF(LSCaptureAreaModeTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)) == TRUE);

	int i=0;
	int x = 2048;
	int y = 64;
	hEvent = CreateEvent(0, FALSE, FALSE, 0);
	long status=ICamera::STATUS_READY;


	////x y pixel number validation
	//rectangle
	for(y=64; y<=2048; y+=128)
	{
		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_AREAMODE, ICamera::RECTANGLE))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, 120))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
		{
			WIN_TRACE(_T("PARAM_LSM_PIXEL_Y %d\n"),x);

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));

			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

			DWORD timeToWait = INFINITE;

			DWORD dwWait = WaitForMultipleObjects( 1, &hThread, TRUE, timeToWait );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);

			if (status==ICamera::STATUS_ERROR)
			{
				WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
			}
			else
			{
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));

				wchar_t baseName[_MAX_PATH];
				StringCbPrintfW(baseName,_MAX_PATH,L"LSMAreaSizeR_%04d_x_%04d",x,y);
				SaveImage(baseName,i,x, x, pMemoryBuffer);
				i++;
				StringCbPrintfW(baseName,_MAX_PATH,L"LSMAreaSizeG_%04d_x_%04d",x,y);
				SaveImage(baseName,i,x, x, pMemoryBuffer+x*x*2);
			}
			ResetEvent( hEvent );
		}
	}

	for(x=64; x<=4096; x+=128)
	{
		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_AREAMODE, ICamera::LINE))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, 1))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, 120))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
		{
			WIN_TRACE(_T("PARAM_LSM_PIXEL_X %d\n"),x);

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));
			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, INFINITE );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			if (status==ICamera::STATUS_ERROR)
			{
				WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
			}
			else
			{
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));

				wchar_t baseName[_MAX_PATH];
				StringCbPrintfW(baseName,_MAX_PATH,L"LSMAreaSizeR_%04d_x_%04d",x,y);
				SaveImage(baseName,i,x, x, pMemoryBuffer);
				i++;
				StringCbPrintfW(baseName,_MAX_PATH,L"LSMAreaSizeG_%04d_x_%04d",x,y);
				SaveImage(baseName,i,x, x, pMemoryBuffer+x*x*2);
			}
			ResetEvent( hEvent );
		}
	}

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));
}
END_TESTF

//Test the field size options for the laser scanner
BEGIN_TESTF(LSCaptureFieldSizeTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParamInfo(ICamera::PARAM_LSM_FIELD_SIZE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)));

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)));

	int i=0;
	int x;
	hEvent = CreateEvent(0, FALSE, FALSE, 0);
	long status=ICamera::STATUS_READY;
	long width = 512;
	long height = 512;

	for(x=static_cast<long>(paramMin); x<static_cast<long>(paramMax); x+=16)
	{
		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, width))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
		{
			WIN_TRACE(_T("PARAM_LSM_FIELD_SIZE %d\n"),x);

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));
			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, INFINITE );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			if (status==ICamera::STATUS_ERROR)
			{
				WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
			}
			else
			{
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));
				SaveImage(L"LSMFieldSizeR",x,width, height, pMemoryBuffer);
				SaveImage(L"LSMFieldSizeG",x,width, height, pMemoryBuffer + width*height*2);
			}
			ResetEvent( hEvent );
		}
	}
	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));
}
END_TESTF

//Test the offset options for the laser scanner
BEGIN_TESTF(LSCaptureOffsetTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)));

	int i=0;
	int x,y;
	hEvent = CreateEvent(0, FALSE, FALSE, 0);
	long status=ICamera::STATUS_READY;

	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParamInfo(ICamera::PARAM_LSM_FIELD_SIZE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)));

	//calculated using the min/max field size
	int offsetMin = static_cast<int>(-1*(paramMax - paramMin)/2);
	int offsetMax = static_cast<int>((paramMax - paramMin)/2);

	long width = 512;
	long height = 512;

	for(y=static_cast<long>(paramMin);y<=static_cast<long>(paramMax/2.0);y+=32)
	{
		WIN_TRACE(_T("PARAM_LSM_FIELD_SIZE %d\n"),y);
		//offsetX
		for(x=offsetMin; x<=offsetMax; x+=32)
		{
			WIN_TRACE(_T("PARAM_LSM_OFFSET_X %d\n"),x);
			if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
				CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, width))&&
				CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, y))&&
				CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
				CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, x))&&
				CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
			{

				WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

				WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));
				DWORD dwThreadId;

				HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

				DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, INFINITE );

				WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

				CloseHandle(hThread);
				if (status==ICamera::STATUS_ERROR)
				{
					WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
				}
				else
				{
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));
					wchar_t baseName[_MAX_PATH];
					StringCbPrintfW(baseName,_MAX_PATH,L"LSMOffsetXR_%04d_x_%04d",x,0);
					SaveImage(baseName,y,width, height, pMemoryBuffer);
					StringCbPrintfW(baseName,_MAX_PATH,L"LSMOffsetXG_%04d_x_%04d",x,0);
					SaveImage(baseName,y,width, height, pMemoryBuffer + width*height*2);
				}
				ResetEvent( hEvent );
			}
		}

		//offset Y
		for(x=offsetMin; x<=offsetMax; x+=32)
		{
			WIN_TRACE(_T("PARAM_LSM_OFFSET_Y %d\n"),x);
			if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
				CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, width))&&
				CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, y))&&
				CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
				CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
				CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, x)))
			{
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

				WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));
				DWORD dwThreadId;

				HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

				DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, INFINITE );

				WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

				CloseHandle(hThread);
				if (status==ICamera::STATUS_ERROR)
				{
					WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
				}
				else
				{
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));
					wchar_t baseName[_MAX_PATH];
					StringCbPrintfW(baseName,_MAX_PATH,L"LSMOffsetYR_%04d_x_04%d",0,x);
					SaveImage(baseName,y,width, height, pMemoryBuffer);
					StringCbPrintfW(baseName,_MAX_PATH,L"LSMOffsetYG_%04d_x_%04d",0,x);
					SaveImage(baseName,y,width, height, pMemoryBuffer + width*height*2);
				}
				ResetEvent( hEvent );
			}
		}
	}
	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));
}
END_TESTF

//Test the channel options for the laser scanner
BEGIN_TESTF(LSCaptureChannelTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)));

	int x;
	int c;
	hEvent = CreateEvent(0, FALSE, FALSE, 0);
	long status=ICamera::STATUS_READY;
	long width=512;
	long height=512;
	for(x=1; x<=3; x++)
	{		
		switch(x)
		{
		case 1:
			c= 0x1;
			break;
		case 2:
			c= 0x2;
			break;
		case 3:
			c = maxLSMChannels;
			break;
		}

		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, width))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, width))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, 120))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, c))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
		{
			WIN_TRACE(_T("PARAM_LSM_CHANNEL %d\n"),x);

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));
			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, INFINITE );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			if (status==ICamera::STATUS_ERROR)
			{
				WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
			}
			else
			{
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));
				SaveImage(L"LSMChannelR",x,width, height, pMemoryBuffer);
				SaveImage(L"LSMChannelG",x,width, height, pMemoryBuffer + 2*width*height);
			}
			ResetEvent( hEvent );
		}
	}
	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));
}
END_TESTF

//Test the scan mode options for the laser scanner
BEGIN_TESTF(LSCaptureScanModeTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->GetParamInfo(ICamera::PARAM_LSM_SCANMODE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)));

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)));

	int i=0;
	int x;
	hEvent = CreateEvent(0, FALSE, FALSE, 0);
	long status=ICamera::STATUS_READY;
	long width=512;
	long height=512;
	for(x=static_cast<long>(paramMin); x<=static_cast<long>(paramMax); x++)
	{
		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, width))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, width))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, 120))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
		{
			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));
			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, INFINITE );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			if (status==ICamera::STATUS_ERROR)
			{
				WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
			}
			else
			{
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));
				SaveImage(L"LSMScanR",x,width, height, pMemoryBuffer);
				SaveImage(L"LSMScanG",x,width, height, pMemoryBuffer + 2*width*height);
			}
			ResetEvent( hEvent );
		}
	}
	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));
}
END_TESTF

//Test will scan through the range of pixel settings for forward scan and two way scan
//capturing and saving as it loops
BEGIN_TESTF(LSCaptureMultiFrameTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE,ICamera::SW_MULTI_FRAME));

	long k=0,j;

	long fieldSize=30;


	const long SEQUENCE_SIZE = 100;
	const long SEQUENCE_SIZE_MAX = 300;


	for(k=SEQUENCE_SIZE; k<=SEQUENCE_SIZE_MAX; k+=SEQUENCE_SIZE, fieldSize+=20)
	{
		CamErrCheck(pCamera->SetParam(ICamera::PARAM_MULTI_FRAME_COUNT,k));

		WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)) == TRUE);

		int i=0;
		int x;
		long status=ICamera::STATUS_READY;

		hEvent = CreateEvent(0, FALSE, FALSE, 0);

		x = 1280;

		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, fieldSize))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
		{
			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));

			for(j=0; j<k; j++)
			{
				DWORD dwThreadId;

				HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );
				DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );

				WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

				CloseHandle(hThread);
				if (status==ICamera::STATUS_ERROR)
				{
					WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
				}
				else
				{
					long indexOfLastCompletedFrame=-1;
					pCamera->StatusAcquisitionEx(status, indexOfLastCompletedFrame);

					if(indexOfLastCompletedFrame != j)
					{
						WIN_TRACE(_T("Last completed frame does not equal the current index forward scan %d, %d\n"),indexOfLastCompletedFrame,j);
						break;
					}
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));
				}
				ResetEvent( hEvent );
			}
		}

		WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));

		WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)) == TRUE);
		//two way scan loop
		x = 1280;

		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,TWO_WAY_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, fieldSize))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
		{
			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));


			for(j=0; j<k; j++)
			{
				DWORD dwThreadId;

				HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

				DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );

				WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

				CloseHandle(hThread);
				if (status==ICamera::STATUS_ERROR)
				{
					WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
				}
				else
				{
					long indexOfLastCompletedFrame=-1;
					pCamera->StatusAcquisitionEx(status, indexOfLastCompletedFrame);

					if(indexOfLastCompletedFrame != j)
					{
						WIN_TRACE(_T("Last completed frame does not equal the current index two way %d, %d\n"),indexOfLastCompletedFrame,j);
						break;
					}

					WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));
				}
				ResetEvent( hEvent );
			}
		}

		WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));

	}
}
END_TESTF

//Test the single frame hardware trigger of the laser scanner camera
BEGIN_TESTF(LSCaptureSingleFrameHWTriggerTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE,ICamera::HW_SINGLE_FRAME));

	long fieldSize=120;

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)) == TRUE);

	int i=0;
	int x;
	long status=ICamera::STATUS_READY;

	hEvent = CreateEvent(0, FALSE, FALSE, 0);

	x = 16;

	if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, fieldSize))&&
		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
	{
		WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

		WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));

		WIN_TRACE("Trigger received!\n");
		DWORD dwThreadId;

		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );
		DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );

		WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

		CloseHandle(hThread);
		if (status==ICamera::STATUS_ERROR)
		{
			WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
		}
		else
		{
			WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));
		}
		ResetEvent( hEvent );
	}

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));

}
END_TESTF


//Test the multi frame hardware trigger option for the laser scanner camera
BEGIN_TESTF(LSCaptureMultiFrameHWTriggerTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE,ICamera::PARAM_LSM_APPEND_INDEX_TO_FRAME));

	long k=0,j;

	long fieldSize=120;

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_MULTI_FRAME_COUNT,k));

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)) == TRUE);

	int i=0;
	int x;
	long status=ICamera::STATUS_READY;

	hEvent = CreateEvent(0, FALSE, FALSE, 0);

	x = 16;

	if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, fieldSize))&&
		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
	{
		WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

		WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));

		WIN_TRACE("Trigger received!\n");
		for(j=0; j<k; j++)
		{
			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );
			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 5000 );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			if (status==ICamera::STATUS_ERROR)
			{
				WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
			}
			else
			{
				long indexOfLastCompletedFrame=-1;
				pCamera->StatusAcquisitionEx(status, indexOfLastCompletedFrame);

				if(indexOfLastCompletedFrame != j)
				{
					WIN_TRACE(_T("Last completed frame does not equal the current index forward scan %d, %d\n"),indexOfLastCompletedFrame,j);
					break;
				}
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));
			}
			ResetEvent( hEvent );
		}
	}

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));

}
END_TESTF

//Test the append index to frame option with single frame capture
BEGIN_TESTF(LSCaptureSingleFrameWithIndexTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_APPEND_INDEX_TO_FRAME,1));

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE,ICamera::SW_FREE_RUN_MODE));

	const long MULTI_FRAME_NUM = 50;
	//CamErrCheck(pCamera->SetParam(ICamera::PARAM_MULTI_FRAME_COUNT,MULTI_FRAME_NUM));

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_AREAMODE,ICamera::SQUARE));

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_Y_AMPLITUDE_SCALER,100));


	long k=0;

	long fieldSize=120;

	LARGE_INTEGER freqInt;
	QueryPerformanceFrequency(&freqInt);
	double dfrq=(double) freqInt.QuadPart;

	int x=512;
	long c=1;
	for(long i=0; i<3; i++)
	{
		switch(i)
		{
		case 0:
			c= 0x1;
			break;
		case 1:
			c= 0x2;
			break;
		case 2:
			c = maxLSMChannels;
			break;
		}

		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE1, 5))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE2, 5))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE3, 5))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE4, 5))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,TWO_WAY_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, fieldSize))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, c))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
		{
			WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)) == TRUE);

			long lastFrameCopied=0;

			for(k=0; k<=MULTI_FRAME_NUM; k++)
			{
				DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
				LARGE_INTEGER largeint;
				QueryPerformanceCounter(&largeint);
				LONGLONG QPart1=largeint.QuadPart;

				::SetThreadAffinityMask(::GetCurrentThread(), oldmask);

				WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

				WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));

				if((k % 20)==0)
				{
					Sleep(100);
				}


				long status=ICamera::STATUS_READY;


				long lastCopiedFrame = -1;

				hEvent = CreateEvent(0, FALSE, FALSE, 0);

				DWORD dwThreadId;

				HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );
				DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );

				WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

				CloseHandle(hThread);
				CloseHandle(hEvent);

				long val=0;
				do
				{
					WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));

					switch(i)
					{
					case 0:
						memcpy(&val,pMemoryBuffer + x * x * BYTES_PER_PIXEL * 1, sizeof(long));;
						break;
					case 1:
						memcpy(&val,pMemoryBuffer + x * x * BYTES_PER_PIXEL * 1, sizeof(long));
						break;
					case 2:
						memcpy(&val,pMemoryBuffer + x * x * BYTES_PER_PIXEL * 4, sizeof(long));
						WIN_TRACE(_T("buffer offset %d\n"),x * x * BYTES_PER_PIXEL * 4);
						break;
					}



					if(val < k)
					{
						WIN_TRACE(_T("Behind the loop count %d val %d\n"),k,val);
					}
					else if (val > k)
					{
						WIN_TRACE(_T("Ahead of the loop count %d val %d\n"),k,val);
					}
					else
					{
						WIN_TRACE(_T("Last frame copied %d This frame copied %d channels %d\n"),lastFrameCopied,val,c);
						lastFrameCopied = val;
					}
				}
				while(val < k);

				oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
				QueryPerformanceCounter(&largeint);
				::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
				LONGLONG QPart2=largeint.QuadPart;
				double dcountdiv=(double) (QPart2-QPart1);
				double dTime=dcountdiv/dfrq;
			}
		}


		WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));
	}
}
END_TESTF

BEGIN_TESTF(LSCaptureDataMapModeTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_AREAMODE, ICamera::SQUARE));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, 512));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, 512));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, 84));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0));				
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE,ICamera::SW_SINGLE_FRAME));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE1, 6));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE2, 6));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE3, 6));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE4, 6));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_DATAMAP_MODE, 0));

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)) == TRUE);

	int i=0;
	int x = 512;
	int y = 512;
	hEvent = CreateEvent(0, FALSE, FALSE, 0);
	long status=ICamera::STATUS_READY;


	////x y pixel number validation
	//rectangle
	for(long d=ICamera::POLARITY_INDEPENDENT; d<ICamera::LAST_MAPPING_MODE; d++)
	{
		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_AREAMODE, ICamera::SQUARE))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, y))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, 84))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0))&&				
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE,ICamera::SW_SINGLE_FRAME))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE1, 6))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE2, 6))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE3, 6))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE4, 6))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_DATAMAP_MODE, d)))
		{
			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));
			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, INFINITE );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			if (status==ICamera::STATUS_ERROR)
			{
				WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
			}
			else
			{
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));

				wchar_t baseName[_MAX_PATH];
				StringCbPrintfW(baseName,_MAX_PATH,L"LSMDataMapMode",d);
				SaveImage(baseName,i,x, x, pMemoryBuffer);
				i++;
			}
			ResetEvent( hEvent );
		}
	}

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));
}
END_TESTF

BEGIN_TESTF(LSCaptureRealtimeDataAveragingTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

		CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_AREAMODE, ICamera::SQUARE));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, 512));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, 512));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, 84));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0));				
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE,ICamera::SW_SINGLE_FRAME));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE1, 6));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE2, 6));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE3, 6));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE4, 6));
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_DATAMAP_MODE, 0));

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)) == TRUE);

	int i=0;
	int x = 512;
	int y = 512;
	hEvent = CreateEvent(0, FALSE, FALSE, 0);
	long status=ICamera::STATUS_READY;


	////x y pixel number validation
	//rectangle
	for(long d=0; d<2; d++)
	{
		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,FORWARD_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_AREAMODE, ICamera::SQUARE))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, y))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, 84))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, maxLSMChannels))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0))&&				
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE,ICamera::SW_SINGLE_FRAME))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE1, 6))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE2, 6))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE3, 6))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE4, 6))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE, d)))
		{
			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));
			DWORD dwThreadId;

			HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );

			DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, INFINITE );

			WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

			CloseHandle(hThread);
			if (status==ICamera::STATUS_ERROR)
			{
				WIN_TRACE(_T("Line Trigger Timeout, Make sure scanner is wired properly and powered up\n"));
			}
			else
			{
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));

				wchar_t baseName[_MAX_PATH];
				StringCbPrintfW(baseName,_MAX_PATH,L"LSMRealtimeDataAveraging",d);
				SaveImage(baseName,i,x, x, pMemoryBuffer);
				i++;
			}
			ResetEvent( hEvent );
		}
	}

	WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));
}
END_TESTF


//Test the append index to frame option with single frame capture
BEGIN_TESTF(LSCaptureFreeRunAndReArmwWithIndexTest,CameraTestFixture)
{
	double camType;

	if(CamErrCheck(pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType)))
	{
		long lVal = static_cast<long>(camType);

		if(lVal != ICamera::LSM)
		{
			return 1;
		}
	}

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_APPEND_INDEX_TO_FRAME,1));

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE,ICamera::SW_FREE_RUN_MODE));

	const long MULTI_FRAME_NUM = 50;
	CamErrCheck(pCamera->SetParam(ICamera::PARAM_MULTI_FRAME_COUNT,1));

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_AREAMODE,ICamera::SQUARE));

	CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_Y_AMPLITUDE_SCALER,100));

	long k=0;

	long fieldSize=120;

	LARGE_INTEGER freqInt;
	QueryPerformanceFrequency(&freqInt);
	double dfrq=(double) freqInt.QuadPart;

	int x=512;
	long c=maxLSMChannels;

		if (CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE1, 5))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE2, 5))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE3, 5))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE4, 5))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, x))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,TWO_WAY_SCAN))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, fieldSize))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL, c))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X, 0))&&
			CamErrCheck(pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, 0)))
		{
			WIN_ASSERT_TRUE(CamErrCheck(pCamera->PreflightAcquisition(pMemoryBuffer)) == TRUE);

			long lastFrameCopied=0;

			WIN_ASSERT_TRUE(CamErrCheck(pCamera->SetupAcquisition(pMemoryBuffer)));
			DWORD startCount = GetTickCount();

			for(k=0; k<=MULTI_FRAME_NUM; k++)
			{
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->StartAcquisition(pMemoryBuffer)));

				long status=ICamera::STATUS_READY;

				long lastCopiedFrame = -1;

				hEvent = CreateEvent(0, FALSE, FALSE, 0);

				DWORD dwThreadId;

				HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, (LPVOID) &status, 0, &dwThreadId );
				DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, 10000 );

				WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

				CloseHandle(hThread);
				CloseHandle(hEvent);

				long val=0;
				WIN_ASSERT_TRUE(CamErrCheck(pCamera->CopyAcquisition(pMemoryBuffer, NULL)));

				memcpy(&val,pMemoryBuffer + x * x * BYTES_PER_PIXEL * 4, sizeof(long));
				WIN_TRACE(_T("buffer offset %d\n"),x * x * BYTES_PER_PIXEL * 4);

				WIN_TRACE(_T("Last frame copied %d This frame copied %d channels %d\n"),lastFrameCopied,val,c);
				lastFrameCopied = val;

						
				DWORD currentCount = GetTickCount();
				WIN_TRACE(_T("Elapsed Tick Count %d\n"),currentCount - startCount);
				startCount = currentCount;
			}
		}

		WIN_ASSERT_TRUE(CamErrCheck(pCamera->PostflightAcquisition(pMemoryBuffer)));

}
END_TESTF


//save the tiff image
void SaveImage(wchar_t *baseName,long index,long width, long height, char * p1)
{

#ifdef SAVE_TEST_IMAGES

	wchar_t filePathAndName[_MAX_PATH];

	//StringCbPrintfW(filePathAndName,_MAX_PATH,L"\\OutputImages\\%s_%04d.tif",baseName,index);
	StringCbPrintfW(filePathAndName,_MAX_PATH,L".\\OutputImages\\%s_%04d.tif",baseName,index);

	TIFF *out= tiffDll->TIFFOpenW(filePathAndName, "w");

	int sampleperpixel = 1; // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon.

	tiffDll->TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width); // set the width of the image
	tiffDll->TIFFSetField(out, TIFFTAG_IMAGELENGTH, height); // set the height of the image
	tiffDll->TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel); // set number of channels per pixel
	tiffDll->TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16); // set the size of the channels
	tiffDll->TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT); // set the origin of the image.

	// Some other essential fields to set that you do not have to understand for now.
	tiffDll->TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	tsize_t linebytes = sampleperpixel * width * 2; // length in memory of one row of pixel in the image.

	unsigned char *buf = NULL; // buffer used to store the row of pixel information for writing to file

	// Allocating memory to store the pixels of current row
	if (tiffDll->TIFFScanlineSize(out)==linebytes)
		buf =(unsigned char *)tiffDll->_TIFFmalloc(linebytes);
	else
		buf = (unsigned char *)tiffDll->_TIFFmalloc(tiffDll->TIFFScanlineSize(out));

	// We set the strip size of the file to be size of one row of pixels
	tiffDll->TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, tiffDll->TIFFDefaultStripSize(out, width*sampleperpixel));


	//Now writing image to the file one strip at a time
	for (uint32 row = 0; row < static_cast<uint32>(height); row++)
	{
		memcpy(buf, &p1[(height-row-1)*linebytes], linebytes); // check the index here, and figure out why not using h*linebytes
		if (tiffDll->TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
	}

	//Finally we close the output file, and destroy the buffer
	tiffDll->TIFFClose(out);

	if (buf)
		tiffDll->_TIFFfree(buf);

#else

	//do nothing
	return;

#endif
}


#pragma warning( pop )