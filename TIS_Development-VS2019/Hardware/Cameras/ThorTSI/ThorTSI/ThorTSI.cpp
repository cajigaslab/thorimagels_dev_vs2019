// ThorTSI.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "ThorTSI.h"

using namespace std;

unique_ptr<IPPIDll> ippiDll(new IPPIDll(L".\\ippiu8-7.0.dll"));

ThorCam::ThorCam() 
{	
	for (long i = 0; i < MAX_CAM_NUM; ++i)
	{
		_camera[i]	= nullptr;
		_cameraRunning[i] = false;
	}
	for(int i = 0; i < MAX_CAM_NUM; i++)
	{
		_saved_Top[i] = 0;
		_saved_Left[i] =  0;
		_saved_Bottom[i] = MAX_IMAGE_HEIGHT; //4096
		_saved_Right[i] = MAX_IMAGE_WIDTH; //4096
	}
	_numCameras	= 0;
	_errMsg[0] = 0;
	_sdkOpen = false;
	_sdk = nullptr;

	_ImgPty_SetSettings.exposureTime_us = 30u * 1000;	
	_ImgPty_SetSettings.roiBin.XOrigin = 0;
	_ImgPty_SetSettings.roiBin.YOrigin = 0;
	_ImgPty_SetSettings.roiBin.XPixels = MAX_IMAGE_WIDTH; //4096
	_ImgPty_SetSettings.roiBin.YPixels = MAX_IMAGE_HEIGHT; //4096
	_ImgPty_SetSettings.roiBin.XBin = 1;
	_ImgPty_SetSettings.roiBin.YBin = 1;
	_ImgPty_SetSettings.opMode = TSI_OP_MODE::TSI_OP_MODE_NORMAL;
	_ImgPty_SetSettings.triggerMode = ICamera::TriggerMode::SW_FREE_RUN_MODE;
	_ImgPty_SetSettings.triggerPolarity = TSI_HW_TRIG_POLARITY::TSI_HW_TRIG_ACTIVE_HIGH;
	_ImgPty_SetSettings.triggerSource = TSI_HW_TRIG_SOURCE::TSI_HW_TRIG_OFF;
	_ImgPty_SetSettings.gain = 90;
	_ImgPty_SetSettings.blackLevel = 48;
	_ImgPty_SetSettings.tapsIndex = 0;
	_ImgPty_SetSettings.tapBalanceEnable = TRUE;
	_ImgPty_SetSettings.numImagesToBuffer = 64;
	_ImgPty_SetSettings.readOutSpeedIndex = 1;
	_ImgPty_SetSettings.channel = 1;
	_ImgPty_SetSettings.averageMode = 0;
	_ImgPty_SetSettings.averageNum = 1;
	_ImgPty_SetSettings.numFrame = 0;
	_ImgPty_SetSettings.dmaBufferCount = 128;
	_ImgPty_SetSettings.verticalFlip = FALSE;
	_ImgPty_SetSettings.horizontalFlip = FALSE;
	_ImgPty_SetSettings.imageAngle = 0;
	_ImgPty_SetSettings.bitsPerPixel = DEFAULT_BITS_PERPIXEL;

	_pDetectorName = nullptr;
	_pSerialNumber = nullptr;

	_selectedCam = -1;
	_running = FALSE;
	_cameraInterfaceType = TSI_USB_INTERFACE;
	_availableFramesCnt = 0;
	_lastDMABufferCount = 0;
	_forceSettingsUpdate = FALSE;
}

///Initialize Static Members
bool ThorCam::_instanceFlag = false;
ImgPty ThorCam::_imgPtyDll = ImgPty();
shared_ptr <ThorCam> ThorCam::_single(nullptr);
unsigned short* ThorCam::_pFrmDllBuffer[MAX_DMABUFNUM] = {NULL};
unsigned long long ThorCam::_bufferImageIndex = 0;
unsigned long ThorCam::_expectedImageSize = 0;
unsigned long ThorCam::_lastCopiedImageSize = 0;
bool ThorCam::_cameraRunning[MAX_CAM_NUM] = {nullptr};
ThreadSafeQueue<ImageProperties> ThorCam::_imagePropertiesQueue;
TsiCamera* ThorCam::_camera[MAX_CAM_NUM] = {nullptr};
HANDLE ThorCam::_hFrmBufHandle = CreateMutex(NULL, false, NULL);


ThorCam* ThorCam::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorCam());
		_instanceFlag = true;
	}
	return _single.get();
}

ThorCam::~ThorCam()
{
	_instanceFlag = false;
}

bool ThorCam::InitTsiSDK()
{

	bool ret = true;

	if (nullptr == _sdk)
	{
		ret = _sdkOpen = false;
		_sdk = get_tsi_sdk(0);
	}

	if(nullptr != _sdk && !_sdkOpen)
	{
		ret = _sdkOpen = _sdk->Open();
	}

	return ret;
}

long ThorCam::FindCameras(long &cameraCount)
{
	long ret = FALSE;
	if (!_sdkOpen) 
	{
		InitTsiSDK();
	}

	if (_sdkOpen) 
	{
		_numCameras = _sdk->GetNumberOfCameras();

		cameraCount = _numCameras;

		if(0 == cameraCount)
		{
			_sdk->Close();
			_sdkOpen = false;
		}

		ret = TRUE;
	} 

	else 
	{
		cameraCount = (long) 0L;
	}

	return ret;
}

long ThorCam::SelectCamera(const long camera)
{
	if (_numCameras == 0) 
	{
		return FALSE;
	}

	if (camera > _numCameras) 
	{
		return FALSE;
	}

	if (!IsOpen(camera)) 
	{
		_camera[camera] = _sdk->GetCamera((int) camera);

		if (nullptr == _camera[camera]) 
		{
			return FALSE;
		}

		if (!_camera[camera]->Open()) 
		{
			return FALSE;
		} 
	}

	//if the camera is open successfully, then change the selectedCam index to the new one
	_selectedCam = camera;

	//set the width and height with the previously stored values
	ThorCam::SetParam(PARAM_CAPTURE_REGION_TOP, _saved_Top[_selectedCam]);
	ThorCam::SetParam(PARAM_CAPTURE_REGION_LEFT, _saved_Left[_selectedCam]);
	ThorCam::SetParam(PARAM_CAPTURE_REGION_BOTTOM, _saved_Bottom[_selectedCam]);
	ThorCam::SetParam(PARAM_CAPTURE_REGION_RIGHT, _saved_Right[_selectedCam]);

	//build a map of TSI camera parameter info
	BuildTsiParamInfoMap(camera);

	//Set the bits per pixel from the camera for the getter. Higher level periodically requests this value
	unsigned int bitsPerPixel;
	if(GetTsiParameter_uint(_selectedCam, TSI_PARAM_BITS_PER_PIXEL, bitsPerPixel))
	{
		_ImgPty_SetSettings.bitsPerPixel = bitsPerPixel;
	}

	//Get the camera name convert it to wtchar and put it on the _pDetectorName variable
	std::string cameraNameStr(_camera[camera]->GetCameraName());

	//=== Convert cameraName to Wide String ===
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &cameraNameStr[0], (int)cameraNameStr.size(), nullptr, 0);
	std::wstring cameraNameWideString(size_needed, 0 );
	MultiByteToWideChar(CP_UTF8, 0, &cameraNameStr[0], (int)cameraNameStr.size(), &cameraNameWideString[0], size_needed);

	if (nullptr != _pDetectorName)
	{
		delete[] _pDetectorName;
	}
	_pDetectorName = new wchar_t[cameraNameWideString.length() + 1];
	memcpy(_pDetectorName, cameraNameWideString.c_str(),(cameraNameWideString.length() + 1)*sizeof(wchar_t));

	//Get the camera serial number convert it to wtchar and put it on the _pSerialNumber variable
	char cameraSerialNumber [TSI_MAX_CAM_NAME_LEN] = "UNKNOWN";
	_camera[camera]->GetParameter (TSI_PARAM_HW_SER_NUM, sizeof (cameraSerialNumber), (void*) &cameraSerialNumber);

	std::string snStr = cameraSerialNumber;
	//=== Convert serial number to Wide String ===
	size_needed = MultiByteToWideChar(CP_UTF8, 0, &snStr[0], (int)snStr.size(), nullptr, 0);
	std::wstring snWideString(size_needed, 0 );
	MultiByteToWideChar(CP_UTF8, 0, &snStr[0], (int)snStr.size(), &snWideString[0], size_needed);

	if (nullptr != _pSerialNumber)
	{
		delete[] _pSerialNumber;
	}
	_pSerialNumber = new wchar_t[snWideString.length() + 1];
	memcpy(_pSerialNumber, snWideString.c_str(),(snWideString.length() + 1)*sizeof(wchar_t));
	_cameraInterfaceType = ((TsiCameraInternal*)_camera[camera])->GetCameraInterfaceType();

	return TRUE;
}

long ThorCam::TeardownCamera()
{
	for (long i = 0; i < MAX_CAM_NUM; ++i)
	{
		DeselectCamera(i);
	}

	if (nullptr != _sdk)
	{
		_sdk->Close();
	}

	_sdkOpen = false;
	_selectedCam = -1;

	for(unsigned long i=0; i<_imgPtyDll.dmaBufferCount; ++i)
	{
		SAFE_DELETE_MEMORY(_pFrmDllBuffer[i]);
	}
	_lastCopiedImageSize = 0;
	_lastDMABufferCount = 0;
	return TRUE;
}

long ThorCam::PreflightAcquisition(char* pData)
{
	long ret = TRUE;

	//copy the new settings
	_ImgPty.exposureTime_us =_ImgPty_SetSettings.exposureTime_us;	
	_ImgPty.roiBin = _ImgPty_SetSettings.roiBin;
	_ImgPty.opMode = _ImgPty_SetSettings.opMode;
	_ImgPty.triggerMode = _ImgPty_SetSettings.triggerMode;
	_ImgPty.triggerPolarity = _ImgPty_SetSettings.triggerPolarity;
	_ImgPty.triggerSource = _ImgPty_SetSettings.triggerSource;
	_ImgPty.gain = _ImgPty_SetSettings.gain;
	_ImgPty.blackLevel = _ImgPty_SetSettings.blackLevel;
	_ImgPty.tapsIndex = _ImgPty_SetSettings.tapsIndex;
	_ImgPty.tapBalanceEnable = _ImgPty_SetSettings.tapBalanceEnable;	
	_ImgPty.numImagesToBuffer = _ImgPty_SetSettings.numImagesToBuffer;
	_ImgPty.readOutSpeedIndex = _ImgPty_SetSettings.readOutSpeedIndex;
	_ImgPty.channel = _ImgPty_SetSettings.channel;
	_ImgPty.averageMode = _ImgPty_SetSettings.averageMode;
	_ImgPty.averageNum = _ImgPty_SetSettings.averageNum;
	_ImgPty.numFrame = _ImgPty_SetSettings.numFrame;
	_ImgPty.dmaBufferCount = _ImgPty_SetSettings.dmaBufferCount;	
	_ImgPty.verticalFlip = _ImgPty_SetSettings.verticalFlip;
	_ImgPty.horizontalFlip = _ImgPty_SetSettings.horizontalFlip;
	_ImgPty.imageAngle = _ImgPty_SetSettings.imageAngle;

	//set the the camera settings and allocate the DMA buffer
	if (TRUE == SetBdDMA(&_ImgPty))
	{
		//if the camera parameters are set and the DMA buffer is allocated successfully,
		//copy the new settings to compare later on.
		_ImgPty_Pre.exposureTime_us = _ImgPty.exposureTime_us;	
		_ImgPty_Pre.roiBin = _ImgPty.roiBin;
		_ImgPty_Pre.opMode = _ImgPty.opMode;
		_ImgPty_Pre.triggerMode = _ImgPty.triggerMode;
		_ImgPty_Pre.triggerPolarity = _ImgPty.triggerPolarity;
		_ImgPty_Pre.triggerSource = _ImgPty.triggerSource;
		_ImgPty_Pre.gain = _ImgPty.gain;
		_ImgPty_Pre.blackLevel = _ImgPty.blackLevel;
		_ImgPty_Pre.tapsIndex = _ImgPty.tapsIndex;
		_ImgPty_Pre.tapBalanceEnable = _ImgPty.tapBalanceEnable;	
		_ImgPty_Pre.numImagesToBuffer = _ImgPty.numImagesToBuffer;
		_ImgPty_Pre.readOutSpeedIndex = _ImgPty.readOutSpeedIndex;
		_ImgPty_Pre.channel = _ImgPty.channel;
		_ImgPty_Pre.averageMode = _ImgPty.averageMode;
		_ImgPty_Pre.averageNum = _ImgPty.averageNum;
		_ImgPty_Pre.numFrame = _ImgPty.numFrame;
		_ImgPty_Pre.dmaBufferCount = _ImgPty.dmaBufferCount;
		_ImgPty_Pre.verticalFlip = _ImgPty.verticalFlip;
		_ImgPty_Pre.horizontalFlip = _ImgPty.horizontalFlip;
		_ImgPty_Pre.imageAngle = _ImgPty.imageAngle;
	}
	else
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"SetBdDMA failed");
		LogMessage(_errMsg,ERROR_EVENT);
		ret = FALSE;
	}

	return ret;
}

long ThorCam::SetupAcquisition(char* pData)
{
	if (0 > _selectedCam) return FALSE;
	//Exposure time, black level, gain, and tap balance can be changed without stoping the acquisition
	long ret = TRUE;
	if (_ImgPty_Pre.exposureTime_us != _ImgPty_SetSettings.exposureTime_us)
	{
		_ImgPty.exposureTime_us = _ImgPty_SetSettings.exposureTime_us;		
		_camera[_selectedCam]->SetParameter(TSI_PARAM_EXPOSURE_TIME_US, &_ImgPty.exposureTime_us);
		_ImgPty_Pre.exposureTime_us = _ImgPty.exposureTime_us;
	}

	if (_ImgPty_Pre.blackLevel != _ImgPty_SetSettings.blackLevel)
	{
		_ImgPty.blackLevel = _ImgPty_SetSettings.blackLevel;		
		_camera[_selectedCam]->SetParameter(TSI_PARAM_GAIN, &_ImgPty.blackLevel);
		_ImgPty_Pre.blackLevel = _ImgPty.blackLevel;
	}

	if (_ImgPty_Pre.gain != _ImgPty_SetSettings.gain)
	{
		_ImgPty.gain = _ImgPty_SetSettings.gain;		
		_camera[_selectedCam]->SetParameter(TSI_PARAM_OPTICAL_BLACK_LEVEL, &_ImgPty.gain);
		_ImgPty_Pre.gain = _ImgPty.gain;
	}

	if (_ImgPty_Pre.tapBalanceEnable != _ImgPty_SetSettings.tapBalanceEnable)
	{
		_ImgPty.tapBalanceEnable = _ImgPty_SetSettings.tapBalanceEnable;
		_camera[_selectedCam]->SetParameter(TSI_PARAM_TAP_BALANCE_ENABLE, &_ImgPty.tapBalanceEnable);
		_ImgPty_Pre.tapBalanceEnable = _ImgPty.tapBalanceEnable;
	}

	//check if there have been any changes that need the experiment to be stopped
	if ((_ImgPty_Pre.roiBin.XBin != _ImgPty_SetSettings.roiBin.XBin ) ||
		(_ImgPty_Pre.roiBin.YBin != _ImgPty_SetSettings.roiBin.YBin ) ||
		(_ImgPty_Pre.roiBin.XOrigin != _ImgPty_SetSettings.roiBin.XOrigin) ||
		(_ImgPty_Pre.roiBin.YOrigin != _ImgPty_SetSettings.roiBin.YOrigin) ||
		(_ImgPty_Pre.roiBin.XPixels != _ImgPty_SetSettings.roiBin.XPixels) ||
		(_ImgPty_Pre.roiBin.YPixels != _ImgPty_SetSettings.roiBin.YPixels) ||
		(_ImgPty_Pre.opMode != _ImgPty_SetSettings.opMode) ||
		(_ImgPty_Pre.triggerMode != _ImgPty_SetSettings.triggerMode) ||
		(_ImgPty_Pre.triggerPolarity != _ImgPty_SetSettings.triggerPolarity) ||
		(_ImgPty_Pre.triggerSource != _ImgPty_SetSettings.triggerSource) ||
		(_ImgPty_Pre.tapsIndex != _ImgPty_SetSettings.tapsIndex) ||
		(_ImgPty_Pre.numImagesToBuffer != _ImgPty_SetSettings.numImagesToBuffer) ||
		(_ImgPty_Pre.readOutSpeedIndex != _ImgPty_SetSettings.readOutSpeedIndex) ||
		(_ImgPty_Pre.averageMode != _ImgPty_SetSettings.averageMode) ||
		(_ImgPty_Pre.channel != _ImgPty_SetSettings.channel) ||
		(_ImgPty_Pre.averageNum != _ImgPty_SetSettings.averageNum) ||
		(_ImgPty_Pre.numFrame != _ImgPty_SetSettings.numFrame) ||
		(_ImgPty_Pre.dmaBufferCount != _ImgPty_SetSettings.dmaBufferCount) ||
		(_ImgPty_Pre.verticalFlip != _ImgPty_SetSettings.verticalFlip) ||
		(_ImgPty_Pre.horizontalFlip != _ImgPty_SetSettings.horizontalFlip) ||
		(_ImgPty_Pre.imageAngle != _ImgPty_SetSettings.imageAngle) ||
		(TRUE == _forceSettingsUpdate)
		)
	{
		_forceSettingsUpdate = FALSE;
		//if there was any change, copy the new settings
		_ImgPty.roiBin = _ImgPty_SetSettings.roiBin;
		_ImgPty.opMode = _ImgPty_SetSettings.opMode;
		_ImgPty.triggerMode = _ImgPty_SetSettings.triggerMode;
		_ImgPty.triggerPolarity = _ImgPty_SetSettings.triggerPolarity;
		_ImgPty.triggerSource = _ImgPty_SetSettings.triggerSource;
		_ImgPty.tapsIndex = _ImgPty_SetSettings.tapsIndex;	
		_ImgPty.numImagesToBuffer = _ImgPty_SetSettings.numImagesToBuffer;
		_ImgPty.readOutSpeedIndex = _ImgPty_SetSettings.readOutSpeedIndex;
		_ImgPty.channel = _ImgPty_SetSettings.channel;
		_ImgPty.averageMode = _ImgPty_SetSettings.averageMode;
		_ImgPty.averageNum = _ImgPty_SetSettings.averageNum;
		_ImgPty.numFrame = _ImgPty_SetSettings.numFrame;
		_ImgPty.dmaBufferCount = _ImgPty_SetSettings.dmaBufferCount;
		_ImgPty.verticalFlip = _ImgPty_SetSettings.verticalFlip;
		_ImgPty.horizontalFlip = _ImgPty_SetSettings.horizontalFlip;
		_ImgPty.imageAngle = _ImgPty_SetSettings.imageAngle;

		//set the the camera settings and allocate the DMA buffer
		if (TRUE == SetBdDMA(&_ImgPty))
		{
			//if the camera parameters are set and the DMA buffer is allocated successfully,
			//copy the new settings to compare later on.
			_ImgPty_Pre.roiBin = _ImgPty.roiBin;
			_ImgPty_Pre.opMode = _ImgPty.opMode;
			_ImgPty_Pre.triggerMode = _ImgPty.triggerMode;
			_ImgPty_Pre.triggerPolarity = _ImgPty.triggerPolarity;
			_ImgPty_Pre.triggerSource = _ImgPty.triggerSource;
			_ImgPty_Pre.tapsIndex = _ImgPty.tapsIndex;
			_ImgPty_Pre.numImagesToBuffer = _ImgPty.numImagesToBuffer;
			_ImgPty_Pre.readOutSpeedIndex = _ImgPty.readOutSpeedIndex;
			_ImgPty_Pre.channel = _ImgPty.channel;
			_ImgPty_Pre.averageMode = _ImgPty.averageMode;
			_ImgPty_Pre.averageNum = _ImgPty.averageNum;
			_ImgPty_Pre.numFrame = _ImgPty.numFrame;
			_ImgPty_Pre.dmaBufferCount = _ImgPty.dmaBufferCount;
			_ImgPty_Pre.verticalFlip = _ImgPty.verticalFlip;
			_ImgPty_Pre.horizontalFlip = _ImgPty.horizontalFlip;
			_ImgPty_Pre.imageAngle = _ImgPty.imageAngle;

			//allow the camera/sdk enough time to allocate buffers when the settings have changed
			Sleep(600);
		}
		else
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"SetBdDMA failed");
			LogMessage(_errMsg,ERROR_EVENT);
			ret = FALSE;
		}
	}	

	return ret;
}

void ThorTSIImageNotificationCallbackFunction(int notification, void *context)
{
	unsigned long cameraIndex = (unsigned long)(uintmax_t)context;
	TsiCamera* camera = ThorCam::_camera[cameraIndex];
	switch (notification) 
	{
	case TSI_IMAGE_NOTIFICATION_PENDING_IMAGE:		
		if (nullptr != camera)
		{
			//for live mode (SW_FREE_RUN_MODE) get the last pending image, no need to get every single image.
			//For an experiment, get the images in order without dropping any
			TsiImage* temp_image = (ICamera::SW_FREE_RUN_MODE == ThorCam::_imgPtyDll.triggerMode) ? camera->GetLastPendingImage() : camera->GetPendingImage();

			//check if the image is valid
			if (nullptr == temp_image) return;

			//consider overflow count from camera
			unsigned long long frameCount = ((0 < ThorCam::_bufferImageIndex) && (ThorCam::_bufferImageIndex > temp_image->m_FrameNumber)) ? 
				(static_cast<unsigned long long>(temp_image->m_FrameNumber + UINT_MAX + 1)) : static_cast<unsigned long long>(temp_image->m_FrameNumber);

			//process frames include the current frame (one frame per callback) and dropped frames
			unsigned long long ProcessFrameCount = ((1 == ThorCam::_cameraRunning[cameraIndex]) && (frameCount > ThorCam::_bufferImageIndex)) ? (frameCount - ThorCam::_bufferImageIndex) : 0;
			ProcessFrameCount =(ICamera::SW_FREE_RUN_MODE == ThorCam::_imgPtyDll.triggerMode) ? 1 : ProcessFrameCount;

			//check the size matches the size of the allocated buffer. If the size doesn't match don't copy further than the data goes, or the allocated buffer size
			ThorCam::_lastCopiedImageSize = (ThorCam::_expectedImageSize <= temp_image->m_SizeInBytes) ? ThorCam::_expectedImageSize : temp_image->m_SizeInBytes;

			for (int i = 0; i < ProcessFrameCount; i++)
			{
				long currentDMAIndex = (ThorCam::_bufferImageIndex + ThorCam::_imgPtyDll.dmaBufferCount) % ThorCam::_imgPtyDll.dmaBufferCount;

				//Keep the important image metadata such as width, height, and frame number.			
				ImageProperties ImageProperties;

				if((ProcessFrameCount - 1) == i)
				{
					WaitForSingleObject(ThorCam::_hFrmBufHandle, Constants::TIMEOUT_MS);

					//copy the image
					memcpy(ThorCam::_pFrmDllBuffer[currentDMAIndex], 
						temp_image->m_PixelData.vptr, 
						ThorCam::_lastCopiedImageSize);

					//keep the size of the copied image, in case it doesn't match the allocated size
					ImageProperties.sizeInBytes = ThorCam::_lastCopiedImageSize;		

					ReleaseMutex(ThorCam::_hFrmBufHandle);
				}
				else
				{
					//skip queue by zero size
					ImageProperties.sizeInBytes = 0;			
				}

				ImageProperties.width = temp_image->m_Width;
				ImageProperties.height = temp_image->m_Height;
				ImageProperties.frameNumber = frameCount - ProcessFrameCount + (i + 1);

				//free the image as soon as possible
				camera->FreeImage(temp_image);						

				//keep the global index of the buffer where the image was copied
				ImageProperties.bufferIndex = ThorCam::_bufferImageIndex;

				//increase the index of the circular buffer
				++ThorCam::_bufferImageIndex;

				//put the image metadata in a queue for retrieval at a later point
				ThorCam::_imagePropertiesQueue.push(ImageProperties);
			}
			return;
		}
		return;
	case TSI_IMAGE_NOTIFICATION_ACQUISITION_ERROR:
		break;
	default :
		break;
	}
}

long ThorCam::StartAcquisition(char* pDataBuffer)
{
	if (0 > _selectedCam) return FALSE;

	//Only start the camera once after stopping it
	if (_running) return TRUE;

	//set the image callback function
	_camera[_selectedCam]->SetImageNotificationCallback(ThorTSIImageNotificationCallbackFunction, (void*)_selectedCam);
	if (_camera[_selectedCam]->Start())
	{
		_running = TRUE;
		_cameraRunning[_selectedCam] = true;
		//If set to HW trigger mode, allow half a second for the camera to be ready to receive the trigger
		switch (_imgPtyDll.triggerMode)
		{
		case ICamera::HW_SINGLE_FRAME:
		case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
		case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		case ICamera::HW_TDI_TRIGGER_MODE:
			Sleep(500);
			break;
		default:
			Sleep(50);
			break;
		}
		return TRUE;
	}
	else 
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error starting acquistion. ThorTSI.");
		LogMessage(_errMsg,ERROR_EVENT);
		_camera[_selectedCam]->ResetCamera();
		_camera[_selectedCam]->ClearError();
		_running = FALSE;
		_camera[_selectedCam]->SetImageNotificationCallback(nullptr, 0);
		return FALSE;
	}
}

long ThorCam::StatusAcquisition(long &status)
{
	size_t size = _imagePropertiesQueue.size();
	if (size > 0)
	{
		if (ICamera::SW_FREE_RUN_MODE == ThorCam::_imgPtyDll.triggerMode)
		{
			status = ICamera::STATUS_READY;
		}
		else
		{
			if (size > ThorCam::_imgPtyDll.dmaBufferCount)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Frame dropped _lastImage == %d, _previousLastImage == %d, queueSize == %d", _lastImage, _previousLastImage, size);
				LogMessage(_errMsg,ERROR_EVENT);
				status = ICamera::STATUS_ERROR;
			}	
			else if ((_lastImage - 1) >= _previousLastImage)
			{
				status = ICamera::STATUS_READY;
			}
		}
	}
	else
	{
		status = ICamera::STATUS_BUSY;
	}
	return TRUE;
}

long ThorCam::StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame)
{
	return TRUE;
}

long ThorCam::CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	//return if no more image in queue, 
	//user must check status before copy
	if(0 >= _imagePropertiesQueue.size())
	{
		_availableFramesCnt = 0;
		return FALSE;
	}
	unsigned short* dst = (unsigned short*)pDataBuffer;

	//get the image metadata from the front of the queue
	ImageProperties imageProperties = _imagePropertiesQueue.pop();

	//make sure we are not copying a bigger image than the buffer size
	if (imageProperties.sizeInBytes > _expectedImageSize) return FALSE;

	//leave dropped frames blank
	if (0 == imageProperties.sizeInBytes)
	{
		_previousLastImage = _lastImage;
		_lastImage = imageProperties.frameNumber;
		return FALSE;
	}

	//only copy when the size matches in all levels for the set-setup-copy settings to ensure there is no size mismatch and there is no memory overstep
	if ((imageProperties.width != (_imgPtyDll.roiBin.XPixels / _imgPtyDll.roiBin.XBin))  &&  (imageProperties.height != (_imgPtyDll.roiBin.YPixels / _imgPtyDll.roiBin.YBin)) &&
		(imageProperties.width != (_ImgPty_SetSettings.roiBin.XPixels  / _ImgPty_SetSettings.roiBin.XBin))  &&  (imageProperties.height != (_ImgPty_SetSettings.roiBin.YPixels / _ImgPty_SetSettings.roiBin.YBin)) &&
		(imageProperties.width != (_ImgPty.roiBin.XPixels / _ImgPty.roiBin.XBin))  &&  (imageProperties.height != (_ImgPty.roiBin.YPixels / _ImgPty.roiBin.YBin)) &&
		(imageProperties.width != (_ImgPty_Pre.roiBin.XPixels / _ImgPty_Pre.roiBin.XBin))  &&  (imageProperties.height != (_ImgPty_Pre.roiBin.YPixels / _ImgPty_Pre.roiBin.YBin)))
	{
		return FALSE;
	}

	unsigned short* intermediateBuffer;
	long flipped = FALSE;		
	unsigned long currentDMAIndex = (imageProperties.bufferIndex+_imgPtyDll.dmaBufferCount)%_imgPtyDll.dmaBufferCount;

	WaitForSingleObject(ThorCam::_hFrmBufHandle, INFINITE);

	if (TRUE == _imgPtyDll.horizontalFlip || TRUE == _imgPtyDll.verticalFlip)
	{
		IppiSize size;
		size.width = imageProperties.width;
		size.height = imageProperties.height;

		//if the angle is not changing, then no need to create an intermediate buffer, use the dst buffer
		if (0 == _imgPtyDll.imageAngle)
		{
			intermediateBuffer = dst;
		}
		else
		{
			//if the angle is changing and there is a flip then we need an intermediate buffer
			intermediateBuffer = (unsigned short*)malloc(imageProperties.sizeInBytes);
		}

		int step = imageProperties.width * sizeof(unsigned short);
		if (TRUE == _imgPtyDll.horizontalFlip && TRUE == _imgPtyDll.verticalFlip)
		{
			ippiDll->ippiMirror_16u_C1R(_pFrmDllBuffer[currentDMAIndex], step, intermediateBuffer, step,  size, ippAxsBoth);
		}
		else if(TRUE == _imgPtyDll.horizontalFlip)
		{
			ippiDll->ippiMirror_16u_C1R(_pFrmDllBuffer[currentDMAIndex], step, intermediateBuffer, step,  size, ippAxsVertical);
		}
		else
		{
			ippiDll->ippiMirror_16u_C1R(_pFrmDllBuffer[currentDMAIndex], step, intermediateBuffer, step,  size, ippAxsHorizontal);
		}
		flipped = TRUE;
	}
	else
	{
		//if there is no flip then the intermediate buffer can be the captured image
		intermediateBuffer =  _pFrmDllBuffer[currentDMAIndex];
	}

	switch (_imgPtyDll.imageAngle)
	{
	case 0:
		{
			if(FALSE == flipped)
			{
				//only need to copy it if there was no flip, otherwise  dst == intermediateBuffer
				memcpy(dst,intermediateBuffer, imageProperties.sizeInBytes);
			}
		}
		break;
	case 90:
		{
			IppiSize size;
			size.width = static_cast<int>(imageProperties.width * sizeof(unsigned short));
			size.height = static_cast<int>(imageProperties.height * sizeof(unsigned short));
			int stepSrc = static_cast<int>(imageProperties.width * sizeof(unsigned short));
			IppiRect  roiSrc = {0, 0, static_cast<int>(imageProperties.width), static_cast<int>(imageProperties.height)};
			int stepDst = static_cast<int>(imageProperties.height * sizeof(unsigned short));
			IppiRect  roiDst = {0, 0, static_cast<int>(imageProperties.height), static_cast<int>(imageProperties.width)};
			int angle = 90;
			int xOffset = 0;
			int yOffset = static_cast<int>(imageProperties.width) - 1;
			ippiDll->ippiRotate_16u_C1R(intermediateBuffer, size, stepSrc, roiSrc, dst, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
		}
		break;
	case 180:
		{
			IppiSize size;
			size.width = static_cast<int>(imageProperties.width * sizeof(unsigned short));
			size.height = static_cast<int>(imageProperties.height * sizeof(unsigned short));
			int stepSrc = static_cast<int>(imageProperties.width * sizeof(unsigned short));
			IppiRect  roiSrc = {0, 0, static_cast<int>(imageProperties.width), static_cast<int>(imageProperties.height)};
			int stepDst = static_cast<int>(imageProperties.width * sizeof(unsigned short));
			IppiRect  roiDst = {0, 0, static_cast<int>(imageProperties.width), static_cast<int>(imageProperties.height)};
			int angle = 180;
			int xOffset = static_cast<int>(imageProperties.width) - 1;
			int yOffset = static_cast<int>(imageProperties.height) - 1;
			ippiDll->ippiRotate_16u_C1R(intermediateBuffer, size, stepSrc, roiSrc, dst, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
		}
		break;
	case 270:
		{
			IppiSize size;
			size.width = static_cast<int>(imageProperties.width * sizeof(unsigned short));
			size.height = static_cast<int>(imageProperties.height * sizeof(unsigned short));
			int stepSrc = static_cast<int>(imageProperties.width * sizeof(unsigned short));
			IppiRect  roiSrc = {0, 0, static_cast<int>(imageProperties.width), static_cast<int>(imageProperties.height)};
			int stepDst = static_cast<int>(imageProperties.height * sizeof(unsigned short));
			IppiRect  roiDst = {0, 0, static_cast<int>(imageProperties.height), static_cast<int>(imageProperties.width)};
			int angle = 270;
			int xOffset = static_cast<int>(imageProperties.height) - 1;
			int yOffset = 0;
			ippiDll->ippiRotate_16u_C1R(intermediateBuffer, size, stepSrc, roiSrc, dst, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
		}
		break;
	default:
		if(FALSE == flipped)
		{
			//only need to copy the image if there was no flip, otherwise  dst == intermediateBuffer
			memcpy(dst,intermediateBuffer, imageProperties.sizeInBytes);
		}			
		break;
	}

	ReleaseMutex(ThorCam::_hFrmBufHandle);

	//only free the intermediate buffer if its memory was allocated in this function
	if (TRUE == flipped && 0 != _imgPtyDll.imageAngle)
	{
		free(intermediateBuffer);
	}

	//keep track of the frame number, and the previous frame number
	_previousLastImage = _lastImage;
	_lastImage = imageProperties.frameNumber;
	_availableFramesCnt = _imagePropertiesQueue.size();
	return TRUE;

}

long ThorCam::PostflightAcquisition(char * pDataBuffer)
{
	//Stop the camera, clear any errors, set the image callback function to null, and clear any pending images
	for (long i = 0; i < MAX_CAM_NUM; ++i)
	{
		if (nullptr == _camera[i]) continue;
		if (_cameraRunning[i])
		{
			Sleep(200);
			_camera[i]->SetImageNotificationCallback(nullptr, nullptr);
			_camera[i]->Stop();		
			_camera[i]->FreeAllPendingImages();
			_running = FALSE;
			//reset the global buffer index
			_bufferImageIndex = 0;
		}
	}
	return TRUE;
}

long ThorCam::SetBdDMA(ImgPty *pImgPty)
{
	if (0 > _selectedCam) return FALSE;

	//only access set the parameters when the camera is open
	if (false == IsOpen(_selectedCam))
		return FALSE;

	//Stop the camera, clear any errors, set the image callback function to null, and clear any pending images
	_camera[_selectedCam]->Stop();
	_camera[_selectedCam]->ClearError();
	_camera[_selectedCam]->SetImageNotificationCallback(nullptr, nullptr);
	_camera[_selectedCam]->FreeAllPendingImages();
	_running = FALSE;

	//reset available frame count, no more copy frames for last session
	_availableFramesCnt = _bufferImageIndex = 0;
	_imagePropertiesQueue.clear();

	//reset the indexes of the last image and the previous last image.
	//these are used to ensure there are no dropped frames
	_lastImage = 0;
	_previousLastImage = -1;

	//Set new Cam parameters
	_imgPtyDll = *pImgPty;

	//Only average frame when the averageMode is set
	uint32_t avgNum = (ICamera::AVG_MODE_NONE == _imgPtyDll.averageMode) ? 1  : _imgPtyDll.averageNum;

	uint32_t tsiTriggerActive;
	TSI_OP_MODE opMode =  _imgPtyDll.opMode;
	uint32_t numFrame = _imgPtyDll.numFrame;
	//use the trigger mode to set the trigger active flag and op mode when in trigger each bulb trigger mode
	//set the OpMode based on the trigger mode
	switch (_imgPtyDll.triggerMode)
	{
	case ICamera::SW_SINGLE_FRAME:
	case ICamera::SW_MULTI_FRAME:
		{	
			opMode = TSI_OP_MODE::TSI_OP_MODE_NORMAL;
			tsiTriggerActive = FALSE;
		}
		break;
	case ICamera::SW_FREE_RUN_MODE:
		{	
			opMode = TSI_OP_MODE::TSI_OP_MODE_NORMAL;
			tsiTriggerActive = FALSE;

			//setting numFrame to 0 indicates acquire until a StopCamera call is made
			numFrame = 0;			
		}
		break;
	case ICamera::HW_SINGLE_FRAME:
	case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
	case ICamera::HW_TDI_TRIGGER_MODE:
		{
			opMode = TSI_OP_MODE::TSI_OP_MODE_NORMAL;
			tsiTriggerActive = TRUE;
		}
		break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH_BULB:
		{
			opMode = TSI_OP_MODE::TSI_OP_MODE_PDX;
			tsiTriggerActive = TRUE;
		}
		break;
	default:
		{
			opMode = TSI_OP_MODE::TSI_OP_MODE_NORMAL;
			tsiTriggerActive = FALSE;
		}
		break;
	}

	//////****Set Camera Parameters****//////
	//send the parameters to the camera, check if they were set successfuly, log if there was an error
	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_EXPOSURE_TIME_US, &_imgPtyDll.exposureTime_us))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_EXPOSURE_TIME_US:(%d) in SetBdDMA for ThorTSI", _imgPtyDll.exposureTime_us);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_GAIN, &_imgPtyDll.gain))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_GAIN:(%d) in SetBdDMA for ThorTSI", _imgPtyDll.gain);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_OPTICAL_BLACK_LEVEL, &_imgPtyDll.blackLevel))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_OPTICAL_BLACK_LEVEL:(%d) in SetBdDMA for ThorTSI", _imgPtyDll.blackLevel);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_OP_MODE, &opMode))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_OP_MODE:(%d) in SetBdDMA for ThorTSI", opMode);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_HW_TRIGGER_ACTIVE, &tsiTriggerActive))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_HW_TRIGGER_ACTIVE:(%d) in SetBdDMA for ThorTSI", tsiTriggerActive);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_HW_TRIG_POLARITY, &_imgPtyDll.triggerPolarity))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_HW_TRIG_POLARITY:(%d) in SetBdDMA for ThorTSI", _imgPtyDll.triggerPolarity);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_ROI_BIN, &_imgPtyDll.roiBin))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_ROI_BIN:(%d) in SetBdDMA for ThorTSI", _imgPtyDll.roiBin);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_TAPS_INDEX, &_imgPtyDll.tapsIndex))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_TAPS_INDEX:(%d) in SetBdDMA for ThorTSI", _imgPtyDll.tapsIndex);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_TAP_BALANCE_ENABLE, &_imgPtyDll.tapBalanceEnable))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_TAPS_INDEX:(%d) in SetBdDMA for ThorTSI", _imgPtyDll.tapsIndex);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_FRAME_COUNT, &numFrame))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_FRAME_COUNT:(%d) in SetBdDMA for ThorTSI", numFrame);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!((TsiCameraInternal*)_camera[_selectedCam])->SetImageAvgNumFrames(avgNum))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting the number of average frames:(%d) in SetBdDMA for ThorTSI", avgNum);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_READOUT_SPEED_INDEX, &_imgPtyDll.readOutSpeedIndex))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_READOUT_SPEED_INDEX:(%d) in SetBdDMA for ThorTSI", _imgPtyDll.readOutSpeedIndex);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	if (!_camera[_selectedCam]->SetParameter(TSI_PARAM_NUM_IMAGE_BUFFERS, &_imgPtyDll.numImagesToBuffer))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Error setting TSI_PARAM_NUM_IMAGE_BUFFERS:(%d) in SetBdDMA for ThorTSI", _imgPtyDll.numImagesToBuffer);
		LogMessage(_errMsg,ERROR_EVENT);
	}	
	//////****End Set Camera Parameters****//////

	//In the future use chan for multi-channel cameras
	long chan = 1;

	//keep the expected image size to make sure we don't over step the boundaries of the allocated memory
	//when copying to the buffer
	_expectedImageSize = chan * ((int)(_imgPtyDll.roiBin.XPixels / _imgPtyDll.roiBin.XBin)) * ((int)( _imgPtyDll.roiBin.YPixels / _imgPtyDll.roiBin.YBin)) * sizeof(unsigned short);

	if((_lastCopiedImageSize != _expectedImageSize) || (0 == _lastCopiedImageSize) || (_lastDMABufferCount != _imgPtyDll.dmaBufferCount))
	{		
		for(unsigned long k=0; k<_imgPtyDll.dmaBufferCount; ++k)
		{
			//assign Memory buffer for ordered and mapped 16 bits frame data
			_pFrmDllBuffer[k] = (unsigned short*) realloc(_pFrmDllBuffer[k], _expectedImageSize);

			if(NULL == _pFrmDllBuffer[k])
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorTSI SetBdDMA unable to allocate pFrmDllBuffer size(%d)", _expectedImageSize);
				LogMessage(_errMsg,ERROR_EVENT);
				return FALSE;
			}
		}
		_lastDMABufferCount = _imgPtyDll.dmaBufferCount;
	}

	//reset the running flag
	_running = FALSE;

	return TRUE;
}

void ThorCam::LogMessage(wchar_t *logMsg,long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

bool ThorCam::IsOpen(const unsigned long  cameraIndex)
{
	if (0 > cameraIndex) return false;
	bool ret = false;
	TSI_CAMERA_STATUS Status;

	if (nullptr != _camera[cameraIndex]) 
	{
		_camera[cameraIndex]->Status (&Status);

		ret = (Status == TSI_STATUS_OPEN);
	}

	return ret;
}

long ThorCam::DeselectCamera(long index)
{
	if (0 > _selectedCam) return FALSE;
	long ret	= TRUE;

	if (nullptr != _camera[index]) {
		_camera[index]->Stop();
		if (_sdkOpen)
		{
			if (IsOpen(index))
			{
				_camera[index]->Close();
			}
		}

		_camera[index] = nullptr;
	}

	return ret;
}