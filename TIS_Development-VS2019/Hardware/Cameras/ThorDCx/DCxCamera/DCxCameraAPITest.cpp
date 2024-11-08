// DCxCamera.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "DCxCameraManager.h"

//#define BASIC_API_TEST_MAIN

#ifdef BASIC_API_TEST_MAIN

typedef void (__cdecl *NOTIFICATION_CALLBACK) (int  notification);

int i_notif = 0;
HCAM cameraHandler;
HANDLE hEvent;
DWORD interval = 50;

NOTIFICATION_CALLBACK _call;

void NotificationCallbackFunction(int notification)
{
	int j = notification;
}

void LiveThread111()
{
	while (true)
	{	
		DWORD dwRet = WaitForSingleObject(hEvent, 1000);
		if (dwRet == WAIT_TIMEOUT)
		{			
			break;
		}
		else if (dwRet == IS_FRAME)
		{
			/* event signalled */
			_call(i_notif++);
		}
		
		//Sleep(interval);
	}

	return;
}

void SnapImage(int index)
{
	//is_FreezeVideo(cameraHandler, IS_WAIT);
	IMAGE_FILE_PARAMS ImageFileParams;
	ImageFileParams.nFileType = IS_IMG_PNG;

	wchar_t str[32];
	swprintf(str, 32, L"image%d.png", index);

	ImageFileParams.pwchFileName = str;
	ImageFileParams.pnImageID = NULL;
	ImageFileParams.ppcImageMem = NULL;

	INT ret = is_ImageFile(cameraHandler, IS_IMAGE_FILE_CMD_SAVE, (void*)&ImageFileParams, sizeof(ImageFileParams));


	//ret = is_CaptureVideo(cameraHandler, IS_WAIT);
}

void StartLiveCapture()
{
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	is_InitEvent(cameraHandler, hEvent, IS_SET_EVENT_FRAME);
	is_EnableEvent(cameraHandler, IS_SET_EVENT_FRAME);

	DWORD dwLiveThreadID;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LiveThread111, NULL, 0, &dwLiveThreadID);

	INT ret = is_CaptureVideo(cameraHandler, IS_DONT_WAIT);
}

void StopLiveCapture()
{
	is_StopLiveVideo(cameraHandler, IS_WAIT);
	is_DisableEvent(cameraHandler, IS_SET_EVENT_FRAME);
	is_ExitEvent(cameraHandler, IS_SET_EVENT_FRAME);
	CloseHandle(hEvent);
}

int main()
{

	_call = NotificationCallbackFunction;

	UC480_CAMERA_LIST cameraList;

	INT ret;

	Cuc480 cuc480;
	cuc480.GetCameraList(&cameraList);

	//is_GetCameraList(&cameraList);
	for (ULONG i = 0; i < cameraList.dwCount; ++i)
	{
		std::cout << cameraList.uci[i].FullModelName << "Serial:" << cameraList.uci[i].SerNo << "Id:" << cameraList.uci[i].dwCameraID << std::endl;
	}
	cameraHandler = cameraList.uci[0].dwCameraID;
	if (is_InitCamera(&cameraHandler, NULL) == IS_SUCCESS)
	{
		std::cout << "open success" << std::endl;		

		BOARDINFO info;
		ret = is_GetCameraInfo(cameraHandler, &info);

		double parm = 1;
		is_SetAutoParameter(cameraHandler, IS_SET_ENABLE_AUTO_WHITEBALANCE, &parm, &parm);

		/*INT nFastAOI = 0;
		ret = is_AOI(cameraHandler, IS_AOI_IMAGE_SET_POS_FAST_SUPPORTED, (void*)&nFastAOI, sizeof(nFastAOI));

		INT nMode;
		ret = is_HotPixel(cameraHandler, IS_HOTPIXEL_GET_SUPPORTED_CORRECTION_MODES, (void*)&nMode, sizeof(nMode));

		ret = is_HotPixel(cameraHandler, IS_HOTPIXEL_DISABLE_CORRECTION, NULL, NULL);
		ret = is_HotPixel(cameraHandler, IS_HOTPIXEL_DISABLE_SENSOR_CORRECTION, NULL, NULL);

		INT hMode;
		ret = is_HotPixel(cameraHandler, IS_HOTPIXEL_GET_CORRECTION_MODE, (void*)&hMode, sizeof(hMode));*/

		//IS_POINT_2D point2d;
		//IS_SIZE_2D size2d;
		//ret = is_AOI(cameraHandler, IS_AOI_IMAGE_GET_POS, (void*)&point2d, sizeof(point2d));
		//ret = is_AOI(cameraHandler, IS_AOI_IMAGE_GET_SIZE, (void*)&size2d, sizeof(size2d));
		//ret = is_AOI(cameraHandler, IS_AOI_IMAGE_GET_POS_MIN, (void*)&point2d, sizeof(point2d));
		//ret = is_AOI(cameraHandler, IS_AOI_IMAGE_GET_SIZE_MIN, (void*)&size2d, sizeof(size2d));
		//ret = is_AOI(cameraHandler, IS_AOI_IMAGE_GET_POS_MAX, (void*)&point2d, sizeof(point2d));
		//ret = is_AOI(cameraHandler, IS_AOI_IMAGE_GET_SIZE_MAX, (void*)&size2d, sizeof(size2d));

		IS_RECT rectAOI;		
		ret = is_AOI(cameraHandler, IS_AOI_IMAGE_GET_ORIGINAL_AOI, (void*)&rectAOI, sizeof(rectAOI));

		rectAOI.s32X = 100;
		rectAOI.s32Y = 100;
		rectAOI.s32Width = 640;
		rectAOI.s32Height = 480;
		ret = is_AOI(cameraHandler, IS_AOI_IMAGE_SET_AOI, (void*)&rectAOI, sizeof(rectAOI));
		ret = is_AOI(cameraHandler, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));

		//point2d.s32X=20;
		//point2d.s32Y=30;
		//ret = is_AOI(cameraHandler, IS_AOI_IMAGE_SET_POS_FAST, (void*)&point2d, sizeof(point2d));
		//ret = is_AOI(cameraHandler, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));

		//INT xAbs, yAbs;
		//ret = is_AOI(cameraHandler, IS_AOI_IMAGE_GET_POS_X_ABS, (void*)&xAbs, sizeof(xAbs));
		//ret = is_AOI(cameraHandler, IS_AOI_IMAGE_GET_POS_Y_ABS, (void*)&yAbs, sizeof(yAbs));

		SENSORSCALERINFO seninfo;
		ret = is_GetSensorScalerInfo(cameraHandler, &seninfo, sizeof(seninfo));

		ret = is_SetSensorScaler(cameraHandler, IS_ENABLE_SENSOR_SCALER, seninfo.dblMinFactor);

		INT iBinning = is_SetBinning(cameraHandler, IS_GET_SUPPORTED_BINNING);
		iBinning = is_SetBinning(cameraHandler, IS_GET_BINNING);
		if ((iBinning & IS_BINNING_2X_HORIZONTAL) == IS_BINNING_2X_HORIZONTAL && (iBinning & IS_BINNING_2X_VERTICAL) == IS_BINNING_2X_VERTICAL)
		{
			ret = is_SetBinning(cameraHandler, IS_BINNING_2X_HORIZONTAL | IS_BINNING_2X_VERTICAL);
		}

		INT sub = is_SetSubSampling(cameraHandler, IS_GET_SUPPORTED_SUBSAMPLING);
		sub = is_SetSubSampling(cameraHandler, IS_GET_SUBSAMPLING_FACTOR_HORIZONTAL);
		if ((sub & IS_SUBSAMPLING_2X_VERTICAL) == IS_SUBSAMPLING_2X_VERTICAL && (sub & IS_SUBSAMPLING_2X_HORIZONTAL) == IS_SUBSAMPLING_2X_HORIZONTAL)
		{
			is_SetSubSampling(cameraHandler, IS_SUBSAMPLING_2X_VERTICAL | IS_SUBSAMPLING_2X_HORIZONTAL);
		}

		//INT zoom;
		//ret = is_Zoom(cameraHandler, ZOOM_CMD_DIGITAL_GET_VALUE_DEFAULT, (void*)&zoom, sizeof(zoom));

		ret = is_SetErrorReport(cameraHandler, IS_ENABLE_ERR_REP);
		ret = is_SetExternalTrigger(cameraHandler, IS_SET_TRIGGER_SOFTWARE);
		UC480_AUTO_INFO uai;
		ret = is_GetAutoInfo(cameraHandler, &uai);

		ret = is_SetGainBoost(cameraHandler, IS_GET_SUPPORTED_GAINBOOST);
		ret = is_SetGainBoost(cameraHandler, IS_GET_GAINBOOST);
		ret = is_SetGainBoost(cameraHandler, IS_SET_GAINBOOST_ON);

		ret = is_SetHardwareGain(cameraHandler, IS_GET_DEFAULT_MASTER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);

		INT nAOISuppoted = 0;
		BOOL bAOISupported = TRUE;
		INT sizeX = 0;
		INT sizeY = 0;
		if (is_ImageFormat(cameraHandler, IMGFRMT_CMD_GET_ARBITRARY_AOI_SUPPORTED, (void*)&nAOISuppoted, sizeof(nAOISuppoted)) == IS_SUCCESS)
			bAOISupported = (nAOISuppoted != 0);
		if (bAOISupported)
		{
			SENSORINFO sInfo;
			is_GetSensorInfo(cameraHandler, &sInfo);
			//sizeX = sInfo.nMaxWidth;
			//sizeY = sInfo.nMaxHeight;
			if (sInfo.bBGain)
			{		
				ret = is_SetHWGainFactor(cameraHandler, IS_GET_BLUE_GAIN_FACTOR, 10);
				ret = is_SetHWGainFactor(cameraHandler, IS_GET_DEFAULT_BLUE_GAIN_FACTOR, 5);
			}
			if (sInfo.bGGain)
			{
				ret = is_SetHWGainFactor(cameraHandler, IS_GET_GREEN_GAIN_FACTOR, 10);
				ret = is_SetHWGainFactor(cameraHandler, IS_GET_DEFAULT_GREEN_GAIN_FACTOR, 5);
			}
			if (sInfo.bRGain)
			{
				ret = is_SetHWGainFactor(cameraHandler, IS_GET_RED_GAIN_FACTOR, 10);
				ret = is_SetHWGainFactor(cameraHandler, IS_GET_DEFAULT_RED_GAIN_FACTOR, 5);
			}
			if (sInfo.bMasterGain)
			{
				//ret = is_SetHWGainFactor(cameraHandler, IS_GET_MASTER_GAIN_FACTOR, 100);	//get current
				//ret = is_SetHWGainFactor(cameraHandler, IS_GET_DEFAULT_MASTER_GAIN_FACTOR, 100);	//get default
				//ret = is_SetHWGainFactor(cameraHandler, IS_INQUIRE_MASTER_GAIN_FACTOR, 100);	//get max
				//ret = is_SetHWGainFactor(cameraHandler, IS_SET_MASTER_GAIN_FACTOR, 200);	//set max or other
				//ret = is_SetHWGainFactor(cameraHandler, IS_GET_MASTER_GAIN_FACTOR, 100);	//get current
			}
		}

		sizeX = is_SetImageSize(cameraHandler, IS_GET_IMAGE_SIZE_X, 0);
		sizeY = is_SetImageSize(cameraHandler, IS_GET_IMAGE_SIZE_Y, 0);

		INT bitsPerPixel = 0;
		switch (is_SetColorMode(cameraHandler, IS_GET_COLOR_MODE))
		{
		case IS_SET_CM_RGB32:
			bitsPerPixel = 32;
			break;
		case IS_SET_CM_RGB24:
			bitsPerPixel = 24;
			break;
		case IS_SET_CM_RGB16:
		case IS_SET_CM_RGB15:
		case IS_SET_CM_UYVY:
			bitsPerPixel = 16;
			break;
		case IS_SET_CM_Y8:
		case IS_SET_CM_BAYER:
		default:
			bitsPerPixel = 8;
			break;
		}
		char * memory;
		int memoryId;
		ret = is_AllocImageMem(cameraHandler, sizeX, sizeY, bitsPerPixel, &memory, &memoryId);
		ret = is_SetImageMem(cameraHandler, memory, memoryId);

		ret = is_SetRopEffect(cameraHandler, IS_SET_ROP_MIRROR_LEFTRIGHT, 1, 0);

		ret = is_SetRopEffect(cameraHandler, IS_SET_ROP_MIRROR_UPDOWN, 1, 0);

		ret = is_SetRopEffect(cameraHandler, IS_GET_ROP_EFFECT, 1, 0);

		int andRes = ret & IS_SET_ROP_MIRROR_LEFTRIGHT;

		int pnMin, pnMax;
		ret = is_GetPixelClockRange(cameraHandler, &pnMin, &pnMax);

		INT picClock;
		ret = is_PixelClock(cameraHandler, IS_PIXELCLOCK_CMD_GET, (void*)&picClock, sizeof(picClock));

		ret = is_PixelClock(cameraHandler, IS_PIXELCLOCK_CMD_SET, (void*)&pnMax, sizeof(pnMax));
		//check set
		ret = is_PixelClock(cameraHandler, IS_PIXELCLOCK_CMD_GET, (void*)&picClock, sizeof(picClock));

		double expMin, expMax, expInt;
		ret = is_GetExposureRange(cameraHandler, &expMin, &expMax, &expInt);
		
		double expouse;
		ret = is_Exposure(cameraHandler, IS_EXPOSURE_CMD_GET_EXPOSURE, (void*)&expouse, sizeof(expouse));

		// frame time for frame rate
		double minRate, maxRate, rateInt;
		ret = is_GetFrameTimeRange(cameraHandler, &minRate, &maxRate, &rateInt);
		
		double minFrame = 1 / maxRate;
		double maxFrame = 1 / minRate;

		double cFrame;

		ret = is_SetFrameRate(cameraHandler, maxFrame, &cFrame);

		ret = is_Exposure(cameraHandler, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*)&expMax, sizeof(expMax));

		ret = is_Exposure(cameraHandler, IS_EXPOSURE_CMD_GET_EXPOSURE, (void*)&expouse, sizeof(expouse));

		ret = is_GetFrameTimeRange(cameraHandler, &minRate, &maxRate, &rateInt);
		
		minFrame = 1 / maxRate;
		maxFrame = 1 / minRate;

		ret = is_SetFrameRate(cameraHandler, maxFrame, &cFrame);

		interval = 1000 / (DWORD)cFrame;

		StartLiveCapture();

		Sleep(5000);

		SnapImage(0);

		Sleep(5000);

		SnapImage(1);

		Sleep(5000);

		SnapImage(2);

		StopLiveCapture();

		ret = is_SetTriggerCounter (cameraHandler, IS_GET_TRIGGER_COUNTER);		

		int retx = is_CameraStatus(cameraHandler, IS_EXT_TRIGGER_EVENT_CNT, IS_GET_STATUS);

		if (ret == retx)
		{
			int missed = ret - i_notif;
		}
		
		char *image;
		int imageMemoryId;
		ret = is_GetActiveImageMem(cameraHandler, &image, &imageMemoryId);
		
		UC480IMAGEINFO imageinfo;
		ret = is_GetImageInfo(cameraHandler, imageMemoryId, &imageinfo, sizeof(imageinfo));

		INT bpp, bm;
		ret = is_GetColorDepth(cameraHandler, &bpp, &bm);

		//int offset;
		//ret = is_GetImageMemPitch(cameraHandler, &offset);
		int length = sizeX * sizeY * bitsPerPixel / 8;
		std::fstream stream("sample.bin", std::ios::in | std::ios::out | std::ios::binary);
		if (stream.good())
		{
			std::cout << "Open file success" << std::endl;
			for (int i = 0; i < length; ++i)
			{
				stream << image[i];
			}
			stream.seekg(std::ios::beg);
			stream.clear();
			stream.write(image, length);
			stream.close();
		}

		is_FreeImageMem(cameraHandler, memory, memoryId);
		is_ExitCamera(cameraHandler);
	}
	//std::fstream stream("image.txt", std::ios::binary | std::ios::in);
	//std::fstream outStream("new.txt", std::ios::out);
	//char *image = new char[1280 * 1024 * 3];
	//if (stream.good() && outStream.good())
	//{
	//	std::cout << "open success" << std::endl;
	//	stream.read(image, 1280 * 1024 * 3);
	//	for (int i = 0; i < 1280 * 1024 * 3; ++i)
	//	{
	//		outStream << std::to_string((int)image[i]);
	//		if (i % 3 == 0)
	//			outStream << std::endl;
	//		else
	//			outStream << ",";
	//	}
	//	stream.close();
	//	outStream.close();
	//}
	//delete image;
	//std::cout << "Finished" << std::endl;
	//int t;
	//std::cin >> t;
	return 0;
}

#else 
int main()
{
	long camerasCount;
	if (FALSE == DCxCameraManager::getInstance()->FindCameras(camerasCount) || camerasCount < 1)
		return -1;
	
	if (FALSE == DCxCameraManager::getInstance()->SelectCamera(0))
		return -1;

	//double value;
	//camera->GetParam(ICamera::PARAM_EXPOSURE_TIME_MS, value);
	//camera->GetParam(ICamera::PARAM_BINNING_X, value);
	//camera->GetParam(ICamera::PARAM_BINNING_Y, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_LEFT, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_TOP, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM, value);
	//camera->GetParam(ICamera::PARAM_CAMERA_IMAGE_WIDTH, value);
	//camera->GetParam(ICamera::PARAM_CAMERA_IMAGE_HEIGHT, value);
	//camera->GetParam(ICamera::PARAM_PIXEL_SIZE, value);
	//camera->GetParam(ICamera::PARAM_GAIN, value);
	//camera->GetParam(ICamera::PARAM_TRIGGER_MODE, value);
	//camera->GetParam(ICamera::PARAM_OPTICAL_BLACK_LEVEL, value);
	//camera->GetParam(ICamera::PARAM_BITS_PER_PIXEL, value);
	//camera->GetParam(ICamera::PARAM_FRAME_RATE, value);

	//camera->SetParam(ICamera::PARAM_EXPOSURE_TIME_MS, 10);
	//camera->SetParam(ICamera::PARAM_BINNING_X, 4);
	//camera->SetParam(ICamera::PARAM_BINNING_Y, 4);
	////camera->SetParam(ICamera::PARAM_CAPTURE_REGION_LEFT, 100);
	////camera->SetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT, 740);
	////camera->SetParam(ICamera::PARAM_CAPTURE_REGION_TOP, 100);
	////camera->SetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM, 580);
	//camera->SetParam(ICamera::PARAM_OPTICAL_BLACK_LEVEL, 1);
	//camera->SetParam(ICamera::PARAM_GAIN, 13);

	//camera->GetParam(ICamera::PARAM_EXPOSURE_TIME_MS, value);
	//camera->GetParam(ICamera::PARAM_BINNING_X, value);
	//camera->GetParam(ICamera::PARAM_BINNING_Y, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_LEFT, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_TOP, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM, value);
	//camera->GetParam(ICamera::PARAM_CAMERA_IMAGE_WIDTH, value);
	//camera->GetParam(ICamera::PARAM_CAMERA_IMAGE_HEIGHT, value);
	//camera->GetParam(ICamera::PARAM_OPTICAL_BLACK_LEVEL, value);
	//camera->GetParam(ICamera::PARAM_GAIN, value);
	//camera->GetParam(ICamera::PARAM_FRAME_RATE, value);

	char pData[128];
	//camera->PreflightAcquisition(pData);

	//camera->GetParam(ICamera::PARAM_EXPOSURE_TIME_MS, value);
	//camera->GetParam(ICamera::PARAM_BINNING_X, value);
	//camera->GetParam(ICamera::PARAM_BINNING_Y, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_LEFT, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_TOP, value);
	//camera->GetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM, value);
	//camera->GetParam(ICamera::PARAM_CAMERA_IMAGE_WIDTH, value);
	//camera->GetParam(ICamera::PARAM_CAMERA_IMAGE_HEIGHT, value);
	//camera->GetParam(ICamera::PARAM_OPTICAL_BLACK_LEVEL, value);
	//camera->GetParam(ICamera::PARAM_GAIN, value);
	//camera->GetParam(ICamera::PARAM_FRAME_RATE, value);
	
	DCxCameraManager::getInstance()->StartAcquisition(pData);

	Sleep(1000);

	DCxCameraManager::getInstance()->CopyAcquisition(pData);

	DCxCameraManager::getInstance()->PostflightAcquisition(pData);

	DCxCameraManager::getInstance()->TeardownCamera();

	return 0;
}
#endif

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
