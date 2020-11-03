#include "stdafx.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"


extern BOOL stopCapture;

void ComputeHistogram16bit(const unsigned short * pBuffer,long size,double *pHist)
{
	for(long i=0;i<size;i++)
	{
		pHist[*pBuffer]++;
		pBuffer++;		
	}
}

BOOL StopActiveCapture()
{
	//interupt a live capture with a ten second timout
	long r = FALSE;
	CHECK_INEXPRESSION_PACTIVEIMAGEROUTINE(GetCaptureActive(), r);
	if(r)
	{
		stopCapture = TRUE;

		const int ACTIVECAPTURE_TIMEOUT = 10000;

		DWORD startTime = GetTickCount();
		DWORD totalTime;
		do
		{
			//allow thread to process
			Sleep(10);
			totalTime = GetTickCount() - startTime;
			CHECK_INEXPRESSION_PACTIVEIMAGEROUTINE(GetCaptureActive(), r);
		}while(r && (totalTime < ACTIVECAPTURE_TIMEOUT));

		CHECK_INEXPRESSION_PACTIVEIMAGEROUTINE(GetCaptureActive(), r);
		if(r)
		{
			return FALSE;
		}

	}
	return TRUE;
}

DllExportLiveImage AutoExposure(double exposure, double &exposureResult, double &multiplier)
{
	BOOL restartLiveCapture = FALSE;

	if(StopActiveCapture() == FALSE)
	{
		return FALSE;
	}

	restartLiveCapture = TRUE;

	CaptureSetupCustomParams lidcp;
	CaptureSetup::getInstance()->GetCustomParamsBinary((char*)&lidcp);

	//capture initial image
	char * pMemoryBuffer = NULL;

	Dimensions d;

	d.c = 1;
	d.dType = INT_16BIT;
	d.m = 1;
	d.mType = DETACHED_CHANNEL;
	d.t = 1;
	d.x = (lidcp.right.val - lidcp.left.val)/lidcp.binX.val;
	d.y = (lidcp.bottom.val - lidcp.top.val)/lidcp.binX.val;
	d.z = 1;

	long width = d.x;
	long height = d.y;

	long imageID;

	if(ImageManager::getInstance()->CreateImage(imageID,d)== FALSE)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"CaptureSetup AutoExposure createimage failed");
		return FALSE;
	}

	pMemoryBuffer = ImageManager::getInstance()->GetImagePtr(imageID,0,0,0,0);

	//if(FALSE == CaptureSingleImage((char*)pMemoryBuffer,exposure,lidcp.binX.val))
	//{
	//	logDll->TLTraceEvent(INFORMATION_EVENT,1,L"CaptureSetup AutoExposure CaptureSingleImage failed");
	//}
	
	d.c = 1;
	d.dType = FLOAT_64BIT;
	d.m = 1;
	d.mType = DETACHED_CHANNEL;
	d.t = 1;
	d.x = 65536;
	d.y = 1;
	d.z = 1;

	long histID;

	if(ImageManager::getInstance()->CreateImage(histID,d)== FALSE)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"CaptureSetup AutoExposure could not create memory buffer");
		return FALSE;
	}

	double * pHistogram = (double*)ImageManager::getInstance()->GetImagePtr(histID,0,0,0,0);

	memset(pHistogram,0,sizeof(double)*65536);

	ComputeHistogram16bit((unsigned short*)pMemoryBuffer,width*height,pHistogram);

	const long INTENSITY_VALUE_TARGET = 3800;

	const double PERCENT_HISTOGRAM_THRESHOLD = .05;

	long pixCount = 0;
	long histLocation = 65535;

	while(true)
	{
		pixCount += static_cast<long>(pHistogram[histLocation]);
		
		if((PERCENT_HISTOGRAM_THRESHOLD * width * height) < pixCount)
		{
			break;
		}

		histLocation--;
	}

	multiplier = INTENSITY_VALUE_TARGET/static_cast<double>(histLocation);

	//if the image is too bright. reduce the multiplier
	if(multiplier < .98)
	{
		const double OVER_EXPOSURE_REDUCTION_MULTIPLIER = .01;
		multiplier = OVER_EXPOSURE_REDUCTION_MULTIPLIER;
	}

	double newExposure = exposure * multiplier;

	//capture with multiplied exposure exposure

	//if(FALSE == CaptureSingleImage(pMemoryBuffer,newExposure,lidcp.binX.val))
	//{
	//	logDll->TLTraceEvent(INFORMATION_EVENT,1,L"CaptureSetup AutoExposure CaptureSingleImage failed");
	//}
	
	memset(pHistogram,0,sizeof(unsigned short)*65536);

	ComputeHistogram16bit((const unsigned short*)pMemoryBuffer,width*height,pHistogram);

	pixCount = 0;
	histLocation = 65535;

	while(true)
	{
		pixCount += static_cast<long>(pHistogram[histLocation]);
		
		if((PERCENT_HISTOGRAM_THRESHOLD * width * height) < pixCount)
		{
			break;
		}

		histLocation--;
	}

	multiplier = INTENSITY_VALUE_TARGET/static_cast<double>(histLocation);

	exposureResult = newExposure*multiplier;

	ImageManager::getInstance()->DestroyImage(imageID);
	ImageManager::getInstance()->DestroyImage(histID);


	if(restartLiveCapture)
	{
		StartLiveCapture();
	}

	return TRUE;
}
