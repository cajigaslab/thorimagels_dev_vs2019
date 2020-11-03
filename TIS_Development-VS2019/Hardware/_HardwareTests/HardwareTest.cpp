#include "..\..\Tools\WinUnit\Include\WinUnit.h"
#include "..\..\Common\Camera.h"
#include "..\..\Common\Device.h"

#include "..\..\Tools\tiff-3.8.2\libtiff\tiffio.h"
#include "..\..\Tools\tiff-3.8.2\include\tifflib.h"

#include <stdlib.h>
#include <time.h>


namespace
{
	char * pMemoryBuffer = NULL;
	const long MAX_IMAGE_COUNT = 60;
	const long BYTES_PER_PIXEL = 2;
	CameraDll * pCamera = NULL;
	long numCameras = 0;
	HANDLE hCameraEvent = NULL;
	HANDLE hStageEvent = NULL;
	wchar_t cameraPathandName[MAX_PATH] = _T("");
	wchar_t devicePathandName[MAX_PATH] = _T("");
	DeviceDll * pDevice = NULL;
	long numDevices = 0;
	TiffLibDll *tiffDll = new TiffLibDll(L".\\libtiff3.dll");
}


FIXTURE(HardwareTestFixture);


SETUP(HardwareTestFixture)
{
	WIN_ASSERT_TRUE(WinUnit::Environment::GetVariable(_T("CameraPathandName"),cameraPathandName,ARRAYSIZE(cameraPathandName)), _T("Environment variable CameraPathandName was not set. Use --CameraPathandName option."));

	pCamera = new CameraDll(cameraPathandName);

	WIN_ASSERT_TRUE(pCamera != NULL);

	WIN_ASSERT_TRUE(pCamera->FindCameras(numCameras));

	WIN_ASSERT_TRUE(numCameras > 0);

	WIN_TRACE(_T("Number of Cameras found %d\n"),numCameras);

	//select the first camera
	WIN_ASSERT_TRUE(pCamera->SelectCamera(0) == TRUE);

	double l,r,t,b;

	//get the camera bounds
	pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_LEFT,l);
	pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT,r);
	pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_TOP,t);
	pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM,b);

	long width = static_cast<long>(r-l);
	long height = static_cast<long>(b-t);

	//allocate multiple image buffers (MAX_IMAGE_COUNT)
	pMemoryBuffer = new char[width * height * MAX_IMAGE_COUNT * BYTES_PER_PIXEL];

	WIN_ASSERT_TRUE(pMemoryBuffer != NULL);

	memset(pMemoryBuffer,0,width * height * MAX_IMAGE_COUNT * BYTES_PER_PIXEL);

	WIN_ASSERT_TRUE(WinUnit::Environment::GetVariable(_T("DevicePathandName"),devicePathandName,ARRAYSIZE(devicePathandName)), _T("Environment variable DevicePathandName was not set. Use --DevicePathandName option."));

	//load the device dll
	pDevice = new DeviceDll(devicePathandName);

	WIN_ASSERT_TRUE(pDevice != NULL);

	//verify that devices are found
	WIN_ASSERT_TRUE(pDevice->FindDevices(numDevices)==TRUE);

	WIN_TRACE("Number of devices found %d\n",numDevices);

	WIN_ASSERT_TRUE(numDevices > 0);
}

TEARDOWN(HardwareTestFixture)
{
	WIN_ASSERT_TRUE(pCamera->TeardownCamera(NULL)==TRUE);

	delete pCamera;

	delete pMemoryBuffer;

	WIN_ASSERT_TRUE(pDevice->TeardownDevice(NULL)==TRUE);

	delete pDevice;
}


#define DBL_EPSILON 1e-10
#define DOUBLE_EQ(x,v) (((v - DBL_EPSILON) < x) && (x <( v + DBL_EPSILON)))


UINT StatusThreadProc( LPVOID pParam )
{
	long status = ICamera::STATUS_BUSY;

	while(status == ICamera::STATUS_BUSY)
	{
		if(FALSE == pCamera->StatusAcquisition(status))
		{
			break;
		}
	}

	SetEvent( hCameraEvent);

	return 0;
}

UINT StageStatusThreadProc( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;

	while(status == IDevice::STATUS_BUSY)
	{
		if(FALSE == pDevice->StatusPosition(status))
		{
			break;
		}
	}

	SetEvent( hStageEvent);

	return 0;
}

void MoveStage(DWORD &dwThreadId,HANDLE &hThread)
{
	WIN_ASSERT_TRUE(pDevice->SetupPosition());

	WIN_ASSERT_TRUE(pDevice->StartPosition());

	hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StageStatusThreadProc, 0, 0, &dwThreadId );
}

void WaitForStage(HANDLE hThread)
{
	DWORD dwWait = WaitForMultipleObjects( 1, &hStageEvent, TRUE, INFINITE );

	WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

	CloseHandle(hThread);
}

void SaveImage(long index,long width, long height, char * p1)
{
	wchar_t filePathAndName[_MAX_PATH];

	wsprintf(filePathAndName,L"C:\\Temp\\TDI\\Image%03d.tif",index);
	TIFF *out= tiffDll->TIFFOpenW(filePathAndName, "w");

	int sampleperpixel = 1; // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon.

	tiffDll->TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width); // set the width of the image
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
}


void CaptureImage(char * p1)
{
	DWORD dwThreadId;
	HANDLE hThread;
	DWORD dwWait;

	WIN_ASSERT_TRUE(pCamera->StartAcquisition(p1));

	hCameraEvent = CreateEvent(0, FALSE, FALSE, 0);


	hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

	dwWait = WaitForMultipleObjects( 1, &hCameraEvent, TRUE, INFINITE );

	WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

	CloseHandle(hThread);

	WIN_ASSERT_TRUE(pCamera->CopyAcquisition(p1));

	ResetEvent( hCameraEvent );
}

double CalculateTdiExposureTimeMilliseconds(double cameraPixelSize, double tdiHeight, double stageVelocityMmPerSec, double magnification)
{
	double val;

	val = ((cameraPixelSize/magnification) * tdiHeight)/(stageVelocityMmPerSec);

	return val;
}

double CalculateTdiWidthMillimeters(double cameraPixelSize, double cameraWidth, double magnification)
{
	double val;

	val = (cameraPixelSize/magnification) * cameraWidth/1000.0;

	return val;
}

//using an xy stage and a camera scan the entire stage area
BEGIN_TESTF(TDICaptureTest,HardwareTestFixture)
{
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_DEVICE_TYPE, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

	WIN_ASSERT_TRUE(paramAvailable==TRUE);

	//verify the the xy stage
	if((static_cast<long>(paramDefault) & static_cast<long>(IDevice::STAGE_X))&&
		(static_cast<long>(paramDefault) & static_cast<long>(IDevice::STAGE_Y)))
	{
		WIN_TRACE("Device has an XY Stage\n");

		double l,r,t,b;

		//get the camera bounds
		pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_LEFT,l);
		pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT,r);
		pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_TOP,t);
		pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM,b);

		//retrieve width height for buffer offset calculation
		long width = static_cast<long>(r-l);
		long height = static_cast<long>(b-t);

		const long BUFFER_FRAME_OFFSET = width * height * BYTES_PER_PIXEL;

		WIN_ASSERT_TRUE(pDevice->SelectDevice(0)==TRUE);

		//get the travel limits for the stage
		WIN_ASSERT_TRUE(pDevice->GetParamInfo(IDevice::PARAM_Y_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

		hStageEvent = CreateEvent(0, FALSE, FALSE, 0);

		const double MAGNIFICATION = 4;
		const double STAGE_SPEED_MM_PER_SEC_FAST = 7;
		const double STAGE_SPEED_MM_PER_SEC_NORMAL = 5;

		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_VELOCITY,STAGE_SPEED_MM_PER_SEC_NORMAL)==TRUE);
		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_VELOCITY,STAGE_SPEED_MM_PER_SEC_NORMAL)==TRUE);
		
		double pixelSize;
		double tdiHeight;
		
		WIN_ASSERT_TRUE(pCamera->GetParam(ICamera::PARAM_PIXEL_SIZE,pixelSize)==TRUE);		
		WIN_ASSERT_TRUE(pCamera->GetParam(ICamera::PARAM_TDI_HEIGHT,tdiHeight)==TRUE);
		
		double exposureTimeMS = CalculateTdiExposureTimeMilliseconds(pixelSize,tdiHeight,STAGE_SPEED_MM_PER_SEC_FAST,MAGNIFICATION);

		WIN_ASSERT_TRUE(pCamera->SetParam(ICamera::PARAM_EXPOSURE_TIME_MS, exposureTimeMS)==TRUE);

		long i,n;
		const long NUM_ROW = 11;
		long imageCount = MAX_IMAGE_COUNT;
		DWORD dwThreadId;
		HANDLE hThread;

		paramMax = paramMax - (.05*(paramMax-paramMin));

		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_HOME,0)==TRUE);
		MoveStage(dwThreadId,hThread);
		WaitForStage(hThread);

		//move xy stage to the starting position
		double xPos = 15;
		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS,xPos)==TRUE);
		MoveStage(dwThreadId,hThread);
		WaitForStage(hThread);

		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_HOME,0)==TRUE);
		MoveStage(dwThreadId,hThread);
		WaitForStage(hThread);

		//prepare the camera
		WIN_ASSERT_TRUE(pCamera->PreflightAcquisition(pMemoryBuffer)==TRUE);

		//index used for naming the images
		n=0;

		for(i=0; i<NUM_ROW; i++)
		{
			char *p1 = pMemoryBuffer;

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_VELOCITY,STAGE_SPEED_MM_PER_SEC_FAST)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_VELOCITY,STAGE_SPEED_MM_PER_SEC_FAST)==TRUE);

			//start the stage movement
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS,paramMax)==TRUE);
			MoveStage(dwThreadId,hThread);

			double position;

			const double START_LOCATION = 10.0;

			//wait for the stage to the start location before capturing images 
			do
			{
				pDevice->ReadPosition(IDevice::STAGE_Y,position);
			}while(position <= START_LOCATION);

			WIN_ASSERT_TRUE(pCamera->SetupAcquisition(pMemoryBuffer));

			long count=0;
			long j=0;

			//capture while the stage is moving
			do
			{
				CaptureImage(p1);

				if(j < imageCount)
				{
					p1 += BUFFER_FRAME_OFFSET;
				}

				j++;
				count++;
			}
			while(WaitForMultipleObjects( 1, &hStageEvent, TRUE, 0 ) != WAIT_OBJECT_0);

			//the first iteration determines the number of images we will save per row
			//store this count and use in subsequent loops
			if(i==0)
			{
				imageCount = min(count,MAX_IMAGE_COUNT);
			}


			//move close to the home position
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_POS,5)==TRUE);
			MoveStage(dwThreadId,hThread);

			//save the image buffers
			p1 = pMemoryBuffer;

			for(j=0; j<imageCount;j++)
			{
				SaveImage(n,width,height,p1);
				p1 += BUFFER_FRAME_OFFSET;
				n++;
			}

			//wait for stage to complete its movement
			WaitForStage(hThread);

			double tdiWidth = CalculateTdiWidthMillimeters(pixelSize,width,MAGNIFICATION);

			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_VELOCITY,STAGE_SPEED_MM_PER_SEC_NORMAL)==TRUE);
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_VELOCITY,STAGE_SPEED_MM_PER_SEC_NORMAL)==TRUE);

			//step the x stage
			xPos += tdiWidth;
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_POS,xPos)==TRUE);
			MoveStage(dwThreadId,hThread);
			WaitForStage(hThread);

			//home the y axis
			WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_Y_HOME,0)==TRUE);
			//start the stage movement
			MoveStage(dwThreadId,hThread);
			///wait for the ystage
			WaitForStage(hThread);
		}

		WIN_ASSERT_TRUE(pCamera->PostflightAcquisition(pMemoryBuffer));

		WIN_ASSERT_TRUE(pDevice->SetParam(IDevice::PARAM_X_HOME,0)==TRUE);
		MoveStage(dwThreadId,hThread);
		WaitForStage(hThread);
	}

}
END_TESTF
