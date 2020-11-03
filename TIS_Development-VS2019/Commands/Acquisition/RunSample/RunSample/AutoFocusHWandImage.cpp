#include "stdafx.h"
#include "..\..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "RunSample.h"
#include "AutoFocusHWandImage.h"


extern auto_ptr<CommandDll> shwDll;
extern auto_ptr<TiffLibDll> tiffDll;

UINT StatusHWandImageAutoFocusThreadProc( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;

	IDevice * pDevice = (IDevice*)pParam;

	while(status == IDevice::STATUS_BUSY)
	{
		if(FALSE == pDevice->StatusPosition(status))
		{
			break;
		}
	}

	SetEvent( AutoFocusHWandImage::hEventAutoFocus);

	return 0;
}

UINT StatusHWandImageZThreadProc( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;

	IDevice * pDevice = (IDevice*)pParam;

	while(status == IDevice::STATUS_BUSY)
	{
		if(FALSE == pDevice->StatusPosition(status))
		{
			break;
		}
	}

	SetEvent( AutoFocusHWandImage::hEventZ);

	return 0;
}


UINT StatusHWandImageCameraThreadProc( LPVOID pParam )
{
	long status = ICamera::STATUS_BUSY;

	ICamera * pCamera = (ICamera*)pParam;

	while(status == ICamera::STATUS_BUSY)
	{
		if(FALSE == pCamera->StatusAcquisition(status))
		{
			break;
		}
	}

	SetEvent( AutoFocusHWandImage::hEventCamera);

	return 0;
}


AutoFocusHWandImage::AutoFocusHWandImage()
{
}

HANDLE AutoFocusHWandImage::hEventAutoFocus = NULL;

HANDLE AutoFocusHWandImage::hEventCamera = NULL;

HANDLE AutoFocusHWandImage::hEventZ = NULL;

void AutoFocusHWandImage::SetupParameters(ICamera * pCamera, IDevice * pZStage,long repeat, double focusOffset, double expTime, double stepSizeUM,double startPosMM, double stopPosMM,long binning)
{
	_pCamera = pCamera;
	_pZStage = pZStage;
	_expTimeMS = expTime;
	_stepSizeUM = stepSizeUM;
	_startPosMM = startPosMM;
	_stopPosMM = stopPosMM;
	_repeat = repeat;
	_focusOffset = focusOffset;
	_counter = 0;
	_binning = binning;
}

long AutoFocusHWandImage::WillAFExecuteNextIteration()
{
	if ((_counter % _repeat) != 0)
	{	
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

long AutoFocusHWandImage::Execute(long index, IDevice * pAutoFocus, BOOL &afFound)
{	
	//the first image will always execute autofocus with (counter == 0) 
	if ((_counter % _repeat) != 0)
	{	
		_counter++;
		return TRUE;
	}
	_counter++;
	
	pAutoFocus->SetParam(IDevice::PARAM_AUTOFOCUS_OFFSET,_focusOffset);

	//enable autofocus
	pAutoFocus->SetParam(IDevice::PARAM_AUTOFOCUS_POS, 1.0);

	pAutoFocus->PreflightPosition();

	pAutoFocus->SetupPosition ();

	pAutoFocus->StartPosition();

	hEventAutoFocus = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThread;

	HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusHWandImageAutoFocusThreadProc, pAutoFocus, 0, &dwThread );

	const long MAX_AF_WAIT_TIME = 10000;

	DWORD dwWait = WaitForSingleObject( hEventAutoFocus, MAX_AF_WAIT_TIME);

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AutoFocusHWandImage Execute HW AutoFocus failed");
		//return FALSE;
	}		

	CloseHandle(hThread);
	CloseHandle(hEventAutoFocus);

	pAutoFocus->PostflightPosition();	

	double param=0;

	pAutoFocus->GetParam(IDevice::PARAM_AUTOFOCUS_FOUND,param);
	
	afFound = static_cast<int>(param);

	if(FALSE == afFound)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"AutoFocusHWandImage HW focus failed");
		return FALSE;
	}

	//begin image based autofocus

	double focusPosition;
	pAutoFocus->GetParam(IDevice::PARAM_Z_POS_CURRENT,focusPosition);

	StringCbPrintfW(message,MSG_LENGTH,L"AutoFocusHWandImage HW Focus Location %d.%03d",static_cast<long>(focusPosition),abs(static_cast<long>((focusPosition - static_cast<long>(focusPosition))*1000)));
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	double start = _startPosMM + focusPosition;
	double stop = _stopPosMM + focusPosition;
	double step = _stepSizeUM / 1000;

	StringCbPrintfW(message,MSG_LENGTH,L"AutoFocusHWandImage Start %d.%03d Stop %d.%03d Step %d.%03d",static_cast<long>(start),abs(static_cast<long>((start - static_cast<long>(start))*1000)),static_cast<long>(stop),abs(static_cast<long>((stop - static_cast<long>(stop))*1000)),static_cast<long>(step),abs(static_cast<long>((step - static_cast<long>(step))*1000)));
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	//verify parameters
	if(start < stop)
	{
		if(step <= 0)
		{
			logDll->TLTraceEvent(VERBOSE_EVENT,1,L"AutoFocusHWandImage AutoFocusImageBased failed stepsize must be positive");
			return FALSE;
		}
	}
	else
	{
		if(step >= 0)
		{
			logDll->TLTraceEvent(VERBOSE_EVENT,1,L"AutoFocusHWandImage AutoFocusImageBased failed stepsize must be negative");
			return FALSE;
		}
	}

	
	double left,right,top,bottom;
	long width,height;

	_pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_LEFT,left);
	_pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT,right);
	_pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_TOP,top);
	_pCamera->GetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM,bottom);

	width = static_cast<long>((right - left)/_binning);
	height = static_cast<long>((bottom - top)/_binning);

	char * pMemoryBuffer = NULL;

	Dimensions d;

	d.c = 1;
	d.dType = INT_16BIT;
	d.m = 1;
	d.mType = DETACHED_CHANNEL;
	d.t = 1;
	d.x = width;
	d.y = height;
	d.z = 1;

	long imageID;

	if(ImageManager::getInstance()->CreateImage(imageID,d)== FALSE)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AutoFocusHWandImage could not create memory buffer");
		return FALSE;
	}

	pMemoryBuffer = ImageManager::getInstance()->GetImagePtr(imageID,0,0,0,0);

	double loc;
	long score =0;
	long highScore=0;
	//start with a focus point at the midpoint of the range
	double focusLoc= start + (stop - start)/2.0;
	const long PIXEL_SKIP_SIZE = 16;

	if(step > 0)
	{
		for(loc = start; loc<= stop; loc += step)
		{			
			SetZPosition(loc);

			//take picture
			if(FALSE == CaptureSingleImage(pMemoryBuffer,_expTimeMS))
			{
				logDll->TLTraceEvent(VERBOSE_EVENT,1,L"AutoFocusHWandImage AutoFocusImageBased CaptureSingleImage failed");
				return FALSE;
			}

			ContrastScore(pMemoryBuffer, width, height, PIXEL_SKIP_SIZE, score);
			
			StringCbPrintfW(message,MSG_LENGTH,L"AutoFocusHWandImage ContrastScore %d",score);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

			if(score > highScore)
			{
				focusLoc = loc;
				highScore = score;
			}
		}
	}
	else
	{
		for(loc = start; loc>= stop; loc += step)
		{
			SetZPosition(loc);

			//take picture
			if(FALSE == CaptureSingleImage(pMemoryBuffer,_expTimeMS))
			{
				logDll->TLTraceEvent(VERBOSE_EVENT,1,L"AutoFocusHWandImage AutoFocusImageBased CaptureSingleImage failed");
				return FALSE;
			}
			
			ContrastScore(pMemoryBuffer, width, height, PIXEL_SKIP_SIZE, score);

			StringCbPrintfW(message,MSG_LENGTH,L"AutoFocusHWandImage ContrastScore %d",score);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

			if(score > highScore)
			{
				focusLoc = loc;
				highScore = score;
			}
		}		
	}

	StringCbPrintfW(message,MSG_LENGTH,L"AutoFocusHWandImage Image Focus Location %d.%03d",static_cast<long>(focusLoc),abs(static_cast<long>((focusLoc - static_cast<long>(focusLoc))*1000)));
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
	
	SetZPosition(focusLoc);

	ImageManager::getInstance()->DestroyImage(imageID);

	return TRUE;
}

long AutoFocusHWandImage::SetZPosition(double pos)
{	
	_pZStage->SetParam(IDevice::PARAM_Z_POS,pos);
	_pZStage->PreflightPosition();
	_pZStage->SetupPosition();
	_pZStage->StartPosition();

	hEventZ = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThreadId;

	HANDLE hThread0;

	hThread0 = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusHWandImageZThreadProc, (LPVOID)_pZStage, 0, &dwThreadId );

	DWORD dwWait = WaitForSingleObject(hEventZ, INFINITE);

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"SetZPosition WaitForSingleObject failed");
		return FALSE;
	}

	CloseHandle(hThread0);

	CloseHandle(hEventZ);

	_pZStage->PostflightPosition();

	return TRUE;
}

long AutoFocusHWandImage::CaptureSingleImage( char *buffer,double expTime)
{
	FrameInfo frameInfo = {0, 0, 0, 0};
	if(FALSE == _pCamera->SetParam(ICamera::PARAM_EXPOSURE_TIME_MS,expTime))
	{
		return FALSE;
	}

	if(FALSE == _pCamera->PreflightAcquisition(buffer))
	{
		return FALSE;
	}

	if(FALSE == _pCamera->SetupAcquisition(buffer))
	{
		return FALSE;
	}
	
	if(FALSE == _pCamera->StartAcquisition(buffer))
	{
		return FALSE;
	}
	
	hEventCamera = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThreadId;

	HANDLE hThread0;

	hThread0 = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusHWandImageCameraThreadProc, (LPVOID)_pCamera, 0, &dwThreadId );

	DWORD dwWait = WaitForSingleObject(hEventCamera, INFINITE);

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSingleImage WaitForSingleObject failed");
		return FALSE;
	}

	CloseHandle(hThread0);	
	CloseHandle(hEventCamera);

	if(FALSE == _pCamera->CopyAcquisition(buffer, &frameInfo))
	{
		return FALSE;
	}

	ResetEvent(hEventCamera);
	
	if(FALSE == _pCamera->PostflightAcquisition(NULL))
	{
		return FALSE;
	}

	return TRUE;

}

void AutoFocusHWandImage::ContrastScore(char * pBuffer, long width, long height, long skipSize, long &score)
{
	unsigned short high1;//highest value in the kernel
	unsigned short high2;
	unsigned short high3;
	unsigned short low1;//lowest value in the kernel
	unsigned short low2;
	unsigned short low3;

	unsigned short * tempPtr;
	unsigned short * imageBuffer = (unsigned short*)pBuffer;

	long      fx,fy;

	int filterWidth = 3;
	int filterHeight = 3;

	long fWidth2,fHeight2;

	fWidth2 = (filterWidth-1)/2;
	fHeight2 = (filterHeight-1)/2;

	unsigned short *dataArr; 

	dataArr = new unsigned short[filterWidth*filterHeight]; 

	long sum = 0;
	unsigned short *startPtr = imageBuffer;

	for(int y=fHeight2;y<(height-fHeight2);y+=skipSize)
	{
		for(int x=fWidth2;x<(width-fWidth2);x+=skipSize) 
		{
			startPtr = imageBuffer + (y*width) + x;
			high1 = *startPtr;
			high2 = *startPtr;
			high3 = *startPtr;
			low1 = *startPtr;
			low2 = *startPtr;
			low3 = *startPtr;

			for(fy=0;fy<filterHeight;fy++)
			{
				tempPtr = startPtr;
				tempPtr +=  (-fWidth2) + ((fy-fHeight2)*(width));

				for(fx=0;fx<filterWidth;fx++)
				{
					if(*tempPtr > high1)
					{
						high3 = high2;
						high2 = high1;
						high1 = *tempPtr;
					}
					if((high1 > *tempPtr) &&(*tempPtr > high2))
					{
						high3 = high2;
						high2 = *tempPtr;
					}

					if(*tempPtr < low1)
					{
						low3 = low2;
						low2 = low1;
						low1 = *tempPtr;
					}
					if((low1 > *tempPtr) && (*tempPtr < low2))
					{
						low3 = low2;
						low2 = *tempPtr;
					}
					tempPtr++;
				}
			}

			//use the third highest/lowest values for the contrast score
			sum += (high3-low3);
		}
	}

	score = sum;
}
