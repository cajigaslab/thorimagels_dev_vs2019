// ThorTSI_CS.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "ThorTSI_CS.h"

using namespace std;

unique_ptr<IPPIDll> ippiDll(new IPPIDll(L".\\ippiu8-7.0.dll"));

ThorCam::ThorCam() : 
	DEFAULT_XBIN(1),
	DEFAULT_YBIN(1),
	DEFAULT_FRM_PER_TRIGGER(1),
	MIN_ANGLE(-90),
	MAX_ANGLE(360),
	DEFAULT_ANGLE(0),
	DEFAULT_EXPOSURE_MS(30),
	DEFAULT_WIDTH(512),
	DEFAULT_HEIGHT(512),
	MIN_CHANNEL(0),
	MAX_CHANNEL(1),
	DEFAULT_CHANNEL(1),
	MIN_BITS_PERPIXEL(8),
	MAX_BITS_PERPIXEL(16)
{	
	for (long i = 0; i < MAX_CAM_NUM; ++i)
	{
		_camera[i]	= NULL;
	}
	_errMsg[0] = 0;

	_ImgPty_SetSettings.exposureTime_us = 30u * Constants::MS_TO_SEC;	
	_ImgPty_SetSettings.triggerMode = ICamera::TriggerMode::SW_FREE_RUN_MODE;
	_ImgPty_SetSettings.triggerPolarity = TL_CAMERA_TRIGGER_POLARITY::TL_CAMERA_TRIGGER_POLARITY_ACTIVE_HIGH;
	_ImgPty_SetSettings.bitPerPixel = MAX_BITS_PERPIXEL;
	_ImgPty_SetSettings.pixelSizeXUM = 0;
	_ImgPty_SetSettings.pixelSizeYUM = 0;
	_ImgPty_SetSettings.roiBinX = 1;
	_ImgPty_SetSettings.roiBinY = 1;
	_ImgPty_SetSettings.roiBottom = 0;
	_ImgPty_SetSettings.roiLeft = 0;
	_ImgPty_SetSettings.roiRight = 0;
	_ImgPty_SetSettings.roiTop = 0;
	_ImgPty_SetSettings.widthPx = 0;
	_ImgPty_SetSettings.heightPx = 0;
	_ImgPty_SetSettings.numImagesToBuffer = MIN_IMAGE_BUFFERS;
	_ImgPty_SetSettings.readOutSpeedIndex = 1;
	_ImgPty_SetSettings.channel = 1;
	_ImgPty_SetSettings.averageMode = 0;
	_ImgPty_SetSettings.averageNum = 1;
	_ImgPty_SetSettings.numFrame = 0;
	_ImgPty_SetSettings.dmaBufferCount = DEFAULT_DMABUFNUM;
	_ImgPty_SetSettings.verticalFlip = FALSE;
	_ImgPty_SetSettings.horizontalFlip = FALSE;
	_ImgPty_SetSettings.imageAngle = 0;
	_ImgPty_SetSettings.hotPixelEnabled = 0;
	_ImgPty_SetSettings.hotPixelThreshold = 0;
	_ImgPty_SetSettings.gain = 0;
	_ImgPty_SetSettings.blackLevel = 0;
	_ImgPty_SetSettings.frameRateControlEnabled = FALSE;
	_ImgPty_SetSettings.frameRateControlValue = 0;

	_pDetectorName = NULL;
	_pSerialNumber = NULL;
	_intermediateBuffer = NULL;
	_availableFramesCnt = 0;
	_lastDMABufferCount = 0;
	_forceSettingsUpdate = FALSE;
	_frameRateControlValueRange[0] = 0;
	_frameRateControlValueRange[1] = 0;
}

///Initialize Static Members
bool ThorCam::_instanceFlag = false;
ImgPty ThorCam::_imgPtyDll = ImgPty();
shared_ptr <ThorCam> ThorCam::_single(NULL);
bool ThorCam::_sdkIsOpen = false;
ThreadSafeMem<USHORT> ThorCam::_pFrmDllBuffer[MAX_DMABUFNUM];
unsigned long long ThorCam::_bufferImageIndex = 0;
unsigned long ThorCam::_expectedImageSize = 0;
unsigned long ThorCam::_lastCopiedImageSize = 0;
unsigned long long ThorCam::_frameCountOffset = 0;
long ThorCam::_1stSet_Frame = 0;
ThreadSafeQueue<ImageProperties> ThorCam::_imagePropertiesQueue;
void* ThorCam::_camera[MAX_CAM_NUM] = {NULL};
std::string ThorCam::_camSerial[MAX_CAM_NUM] = {""};
std::string ThorCam::_camName[MAX_CAM_NUM] = {""};
TL_CAMERA_USB_PORT_TYPE	ThorCam::_cameraInterfaceType[MAX_CAM_NUM] = {TL_CAMERA_USB_PORT_TYPE::TL_CAMERA_USB_PORT_TYPE_USB3_0};
long ThorCam::_numCameras = 0;
long ThorCam::_camID = -1;
wchar_t ThorCam::_errMsg[MSG_SIZE] = {NULL};
bool ThorCam::_cameraRunning[MAX_CAM_NUM] = {false};
HANDLE ThorCam::_hStopAcquisition = CreateEvent(NULL, true, false, NULL);  //2nd parameter "true" so it needs manual "Reset" after "Set (signal)" event
long ThorCam::_maxFrameCountReached = FALSE;
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

void ThorCam::ClearAllCameras()
{
	for (int i = 0; i < MAX_CAM_NUM; i++)
	{
		DeSelectCamera(i);
	}

	//no camera is avialble afterward
	_camID = -1;

	//close sdk
	if(_sdkIsOpen)
	{
		if (tl_camera_close_sdk())
		{
			MessageBox(NULL,L"Failed to close SDK", L"SDK Close error", MB_OK);
		}
		tl_camera_sdk_dll_terminate();
		_sdkIsOpen = false;
	}
}

void ThorCam::ClearMem()
{
	for (int i = 0; i < MAX_DMABUFNUM; i++)
	{
		_pFrmDllBuffer[i].ReleaseMem();
	}
	_lastCopiedImageSize = 0;
	_lastDMABufferCount = 0;
	SAFE_DELETE_MEMORY(_intermediateBuffer);
}

long ThorCam::DeSelectCamera(long cameraIndex)
{
	long ret = FALSE;

	StopCamera(cameraIndex);

	if(IsOpen(cameraIndex))
	{
		_camSerial[cameraIndex].clear();
		_camName[cameraIndex].clear();
		tl_camera_disarm(_camera[cameraIndex]);
		tl_camera_close_camera(_camera[cameraIndex]);
		_camera[cameraIndex] = NULL;
		ret = TRUE;
	}
	return ret;
}

void ThorCam::FindAllCameras()
{
	char serialNum[TSI_TEXT_CMD_SIZE];
	char model[TSI_TEXT_CMD_SIZE];
	//char name[TSI_TEXT_CMD_SIZE];	

	try
	{
		//get all available cameras
		_numCameras = 0L;

		//open camera SDK
		if(!_sdkIsOpen)
		{
			if (tl_camera_sdk_dll_initialize()) 
			{
				MessageBox(NULL,L"Failed to init SDK", L"SDK Init error", MB_OK);
			}
			if (tl_camera_open_sdk()) 
			{
				MessageBox(NULL,L"Failed to open SDK", L"SDK Open error", MB_OK);
			}
			ThorTSIErrChk(L"tl_camera_set_camera_connect_callback", tl_camera_set_camera_connect_callback(CameraConnectedCallback, NULL), 0);
			ThorTSIErrChk(L"tl_camera_set_camera_disconnect_callback", tl_camera_set_camera_disconnect_callback(CameraDisconnectedCallback, NULL), 0);
			_sdkIsOpen = true;
		}
		ThorTSIErrChk(L"tl_camera_discover_available_cameras", tl_camera_discover_available_cameras(serialNum, TSI_TEXT_CMD_SIZE), 1);

		string camera_ids(serialNum);
		char *next_token;
		char* tok = strtok_s(serialNum, " ", &next_token);
		int i = 0;
		while (tok)
		{
			_camSerial[i] = tok;
			tok = strtok_s(NULL, " ", &next_token);
			i++;
		}
		for (int j = 0; j < MAX_CAM_NUM; j++)
		{
			if(0 < _camSerial[j].size())
			{
				char camera_id[TSI_TEXT_CMD_SIZE];
				strcpy_s(camera_id, _camSerial[j].c_str());

				if(0 == tl_camera_open_camera(camera_id, &_camera[j]))
				{
					_numCameras++;
					ThorTSIErrChk(L"tl_camera_get_model", tl_camera_get_model(_camera[j],model, MSG_SIZE), 1);
					_camName[j] = model;
					ThorTSIErrChk(L"tl_camera_get_usb_port_type", tl_camera_get_usb_port_type(_camera[j],&_cameraInterfaceType[j]), 1);
					//ThorTSIErrChk(L"tl_camera_get_name", tl_camera_get_name(_camera[j], name, MSG_SIZE), 1);
					//std::string strName = name;
					//_camName[j] = (0 == strName.size()) ? std::string(model) : strName;
				}
			}
		}
	}
	catch(...)
	{
		GetLastError();
		StringCbPrintfW(_errMsg,MSG_SIZE,L"FindAllCameras failed");
		LogMessage(_errMsg,ERROR_EVENT);
	}
}

void ThorCam::GetLastError()
{
	std::string err = tl_camera_get_last_error();
	if(0 != err.compare("no error"))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorCam Error: %S",StringToWString(err));
		LogMessage(_errMsg,ERROR_EVENT);
	}
}

void ThorCam::InitialParamInfo(void)
{
	ThorTSIErrChk(L"tl_camera_get_roi_range", tl_camera_get_roi_range(_camera[_camID],&_xRangeL[0],&_yRangeT[0],&_xRangeR[0],&_yRangeB[0],&_xRangeL[1],&_yRangeT[1],&_xRangeR[1],&_yRangeB[1]), 1);
	ThorTSIErrChk(L"tl_camera_get_binx_range", tl_camera_get_binx_range(_camera[_camID],&_hbinRange[0], &_hbinRange[1]), 1);
	ThorTSIErrChk(L"tl_camera_get_biny_range", tl_camera_get_biny_range(_camera[_camID],&_vbinRange[0], &_vbinRange[1]), 1);
	ThorTSIErrChk(L"tl_camera_get_binx", tl_camera_get_binx(_camera[_camID],&_ImgPty_SetSettings.roiBinX), 1);
	ThorTSIErrChk(L"tl_camera_get_biny", tl_camera_get_biny(_camera[_camID],&_ImgPty_SetSettings.roiBinY), 1);
	ThorTSIErrChk(L"tl_camera_get_frames_per_trigger_range", tl_camera_get_frames_per_trigger_range(_camera[_camID],&_frmPerTriggerRange[0], &_frmPerTriggerRange[1]), 1);
	ThorTSIErrChk(L"tl_camera_get_exposure_time_range", tl_camera_get_exposure_time_range(_camera[_camID], &_expUSRange[0], &_expUSRange[1]), 1);
	ThorTSIErrChk(L"tl_camera_get_bit_depth", tl_camera_get_bit_depth(_camera[_camID], &_ImgPty_SetSettings.bitPerPixel), 1);
	ThorTSIErrChk(L"tl_camera_get_image_width_range", tl_camera_get_image_width_range(_camera[_camID], &_widthRange[0], &_widthRange[1]), 1);
	ThorTSIErrChk(L"tl_camera_get_sensor_width", tl_camera_get_sensor_width(_camera[_camID], &_ImgPty_SetSettings.widthPx), 1);
	ThorTSIErrChk(L"tl_camera_get_image_height_range", tl_camera_get_image_height_range(_camera[_camID], &_heightRange[0], &_heightRange[1]), 1);
	ThorTSIErrChk(L"tl_camera_get_sensor_height", tl_camera_get_sensor_height(_camera[_camID], &_ImgPty_SetSettings.heightPx), 1);
	ThorTSIErrChk(L"tl_camera_get_sensor_pixel_width", tl_camera_get_sensor_pixel_width(_camera[_camID], &_ImgPty_SetSettings.pixelSizeXUM), 1);
	ThorTSIErrChk(L"tl_camera_get_sensor_pixel_height", tl_camera_get_sensor_pixel_height(_camera[_camID], &_ImgPty_SetSettings.pixelSizeYUM), 1);
	ThorTSIErrChk(L"tl_camera_get_hot_pixel_correction_threshold_range", tl_camera_get_hot_pixel_correction_threshold_range(_camera[_camID], &_hotPixelRange[0], &_hotPixelRange[1]), 1);
	ThorTSIErrChk(L"tl_camera_get_hot_pixel_correction_threshold", tl_camera_get_hot_pixel_correction_threshold(_camera[_camID], &_ImgPty_SetSettings.hotPixelThreshold), 1);
	ThorTSIErrChk(L"tl_camera_get_is_hot_pixel_correction_enabled", tl_camera_get_is_hot_pixel_correction_enabled(_camera[_camID], &_ImgPty_SetSettings.hotPixelEnabled), 1);
	ThorTSIErrChk(L"tl_camera_get_gain_range", tl_camera_get_gain_range(_camera[_camID], &_gainRange[0], &_gainRange[1]), FALSE);
	ThorTSIErrChk(L"tl_camera_get_gain", tl_camera_get_gain(_camera[_camID], &_ImgPty_SetSettings.gain), FALSE);
	ThorTSIErrChk(L"tl_camera_get_black_level_range", tl_camera_get_black_level_range(_camera[_camID], &_blackLevelRange[0], &_blackLevelRange[1]), FALSE);
	ThorTSIErrChk(L"tl_camera_get_black_level", tl_camera_get_black_level(_camera[_camID], &_ImgPty_SetSettings.blackLevel), FALSE);
	ThorTSIErrChk(L"tl_camera_get_frame_rate_control_value_range", tl_camera_get_frame_rate_control_value_range(_camera[_camID], &_frameRateControlValueRange[0], &_frameRateControlValueRange[1]), FALSE);
	ThorTSIErrChk(L"tl_camera_get_is_frame_rate_control_enabled", tl_camera_get_is_frame_rate_control_enabled(_camera[_camID], &_ImgPty_SetSettings.frameRateControlEnabled), FALSE);
	ThorTSIErrChk(L"tl_camera_get_frame_rate_control_value", tl_camera_get_frame_rate_control_value(_camera[_camID], &_ImgPty_SetSettings.frameRateControlValue), FALSE);
}

bool ThorCam::IsOpen(const long cameraIndex)
{
	//return if no camera found
	if (_numCameras == 0)
		return FALSE;
	//return if invalid selection
	if ((0 > cameraIndex) || (_numCameras <= cameraIndex))
		return FALSE;
	//return if camera is not available
	if(NULL == _camera[cameraIndex]) 
		return FALSE;

	return TRUE;
}

void ThorCam::LogMessage(wchar_t *logMsg,long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

/// <summary> function to do buffer average with previous acquired frame </summary>
void ThorCam::ProcessAverageFrame(unsigned short* dst, unsigned long previousDMAIndex)
{
	unsigned long imageSize =  _imgPtyDll.channel * ((_imgPtyDll.roiRight - _imgPtyDll.roiLeft + 1) / _imgPtyDll.roiBinX) * ((_imgPtyDll.roiBottom - _imgPtyDll.roiTop + 1) / _imgPtyDll.roiBinY);
	unsigned long sizeInBytes = imageSize * sizeof(unsigned short);

	if (!_pFrmDllBuffer[previousDMAIndex].TryLockMem(Constants::TIMEOUT_MS))
	{
		return;
	}
	USHORT* pCur = dst;
	USHORT* pPre = _pFrmDllBuffer[previousDMAIndex].GetMem();	//previous frame
	for (unsigned long i = 0; i < imageSize; i++)
	{
		*pCur = ((*pCur)*(_imgPtyDll.averageNum - 1) + (*pPre)) / _imgPtyDll.averageNum;
		pCur++;
		pPre++;
	}
	//output with averaged buffer
	_pFrmDllBuffer[previousDMAIndex].UnlockMem();
}

long ThorCam::SetBdDMA(ImgPty *pImgPty)
{
	long ret = TRUE;

	//only access set the parameters when the camera is selected
	if (!IsOpen(_camID))
		return FALSE;
	try
	{
		//Stop the camera, clear any errors, set the image callback function to null, and clear any pending images
		StopCamera(_camID);

		//reset available frame count, no more copy frames for last session
		_availableFramesCnt = _bufferImageIndex = 0;
		_imagePropertiesQueue.clear();

		//reset the indexes of the last image and the previous last image.
		//these are used to ensure there are no dropped frames
		_lastImage = 0;
		_previousLastImage = -1;

		//reset max frame count reached flag
		ThorCam::_maxFrameCountReached = FALSE;

		//Set new Cam parameters
		_imgPtyDll = *pImgPty;

		//Only average frame when the averageMode is set
		long avgNum = (ICamera::AVG_MODE_NONE == _imgPtyDll.averageMode) ? 1  : _imgPtyDll.averageNum;

		//set the trigger mode and total number of frames,
		//since we are doing average after capture
		enum TL_CAMERA_OPERATION_MODE current_mode;
		enum TL_CAMERA_TRIGGER_POLARITY current_polarity;

		ThorTSIErrChk(L"tl_camera_get_trigger_polarity", tl_camera_get_trigger_polarity(_camera[_camID], &current_polarity), 1); 
		ThorTSIErrChk(L"tl_camera_get_operation_mode", tl_camera_get_operation_mode(_camera[_camID], &current_mode), 1); 

		switch((TriggerMode)(_imgPtyDll.triggerMode))
		{
		case SW_SINGLE_FRAME:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_SOFTWARE_TRIGGERED;
				_imgPtyDll.numFrame = avgNum;
				_imgPtyDll.numImagesToBuffer = MIN_IMAGE_BUFFERS;
			}
			break;
		case SW_MULTI_FRAME:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_SOFTWARE_TRIGGERED;
				_imgPtyDll.numFrame = _imgPtyDll.numFrame * avgNum;
				_imgPtyDll.numImagesToBuffer = DEFAULT_IMAGE_BUFFERS;
			}
			break;
		case SW_FREE_RUN_MODE:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_SOFTWARE_TRIGGERED;
				_imgPtyDll.numFrame = 0;
				_imgPtyDll.numImagesToBuffer = MIN_IMAGE_BUFFERS;
			}
			break;
		case HW_SINGLE_FRAME:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_HARDWARE_TRIGGERED;
				_imgPtyDll.numFrame = avgNum;
				_imgPtyDll.numImagesToBuffer = MIN_IMAGE_BUFFERS;
			}
			break;
		case HW_MULTI_FRAME_TRIGGER_FIRST:
		case HW_MULTI_FRAME_TRIGGER_EACH:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_HARDWARE_TRIGGERED;
				_imgPtyDll.numFrame = _imgPtyDll.numFrame * avgNum;
				_imgPtyDll.numImagesToBuffer = DEFAULT_IMAGE_BUFFERS;
			}
			break;
		case HW_MULTI_FRAME_TRIGGER_EACH_BULB:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_BULB;
				_imgPtyDll.numFrame = _imgPtyDll.numFrame * avgNum;
				_imgPtyDll.numImagesToBuffer = DEFAULT_IMAGE_BUFFERS;
			}
			break;
		}
		//////****Set/Get Camera Parameters****//////
		//send the parameters to the camera, check if they were set successfuly, log if there was an error
		current_polarity = TL_CAMERA_TRIGGER_POLARITY::TL_CAMERA_TRIGGER_POLARITY_ACTIVE_HIGH;	//trigger by rising edge
		ThorTSIErrChk(L"tl_camera_set_trigger_polarity", tl_camera_set_trigger_polarity(_camera[_camID], current_polarity), 1); 
		ThorTSIErrChk(L"tl_camera_set_operation_mode", tl_camera_set_operation_mode(_camera[_camID], current_mode), 1); 

		long acceptedFramePerTrigger = ((0 == _camName[_camID].compare(0, 6, "CS2100")) && (MAX_FRAMENUM < _imgPtyDll.numFrame)) ? 0 : _imgPtyDll.numFrame;	//[CS2100] use free run mode if more than max frame limit
		ThorTSIErrChk(L"tl_camera_set_frames_per_trigger_zero_for_unlimited", tl_camera_set_frames_per_trigger_zero_for_unlimited(_camera[_camID], acceptedFramePerTrigger), 1); 

		ThorTSIErrChk(L"tl_camera_set_binx", tl_camera_set_binx(_camera[_camID], _imgPtyDll.roiBinX), 1);

		ThorTSIErrChk(L"tl_camera_set_biny", tl_camera_set_biny(_camera[_camID], _imgPtyDll.roiBinY), 1);

		ThorTSIErrChk(L"tl_camera_set_roi", tl_camera_set_roi(_camera[_camID], _imgPtyDll.roiLeft, _imgPtyDll.roiTop, _imgPtyDll.roiRight, _imgPtyDll.roiBottom), 1);

		ThorTSIErrChk(L"tl_camera_set_data_rate", tl_camera_set_data_rate(_camera[_camID], (TL_CAMERA_DATA_RATE)_imgPtyDll.readOutSpeedIndex), FALSE);

		ThorTSIErrChk(L"tl_camera_set_gain", tl_camera_set_gain(_camera[_camID], (TL_CAMERA_DATA_RATE)_imgPtyDll.gain), FALSE);

		ThorTSIErrChk(L"tl_camera_set_black_level", tl_camera_set_black_level(_camera[_camID], (TL_CAMERA_DATA_RATE)_imgPtyDll.blackLevel), FALSE);

		ThorTSIErrChk(L"tl_camera_set_is_frame_rate_control_enabled", tl_camera_set_is_frame_rate_control_enabled(_camera[_camID], _imgPtyDll.frameRateControlEnabled), FALSE);

		ThorTSIErrChk(L"tl_camera_set_frame_rate_control_value", tl_camera_set_frame_rate_control_value(_camera[_camID], _imgPtyDll.frameRateControlValue), FALSE);

		//update width and height after setting roi
		ThorTSIErrChk(L"tl_camera_get_sensor_width", tl_camera_get_sensor_width(_camera[_camID], &_ImgPty_SetSettings.widthPx), 1);
		_imgPtyDll.widthPx = _ImgPty.widthPx = _ImgPty_SetSettings.widthPx;

		ThorTSIErrChk(L"tl_camera_get_sensor_height", tl_camera_get_sensor_height(_camera[_camID], &_ImgPty_SetSettings.heightPx), 1);
		_imgPtyDll.heightPx = _ImgPty.heightPx = _ImgPty_SetSettings.heightPx;

		//////****End Set/Get Camera Parameters****//////

		//In the future use chan for multi-channel cameras
		_imgPtyDll.channel = 1;

		//keep the expected image size to make sure we don't over step the boundaries of the allocated memory
		//when copying to the buffer
		_expectedImageSize = _imgPtyDll.channel * (_imgPtyDll.widthPx) * (_imgPtyDll.heightPx) * sizeof(unsigned short);

		if((_lastCopiedImageSize != _expectedImageSize) || (0 == _lastCopiedImageSize) || (_lastDMABufferCount != _imgPtyDll.dmaBufferCount))
		{		
			for(int k=0; k<_imgPtyDll.dmaBufferCount; ++k)
			{	
				if (FALSE == _pFrmDllBuffer[k].SetMem(_expectedImageSize))
				{
					StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorTSI SetBdDMA unable to allocate pFrmDllBuffer #(%d), size(%d)", k, _expectedImageSize);
					LogMessage(_errMsg,ERROR_EVENT);
					return FALSE;
				}
			}
			_lastDMABufferCount = _imgPtyDll.dmaBufferCount;
		}	

		//use intermediate if need flip, rotate or average:
		if((TRUE == _imgPtyDll.horizontalFlip || TRUE == _imgPtyDll.verticalFlip) || (0 != _imgPtyDll.imageAngle) ||
			((AverageMode::AVG_MODE_CUMULATIVE == _imgPtyDll.averageMode) && (1 < _imgPtyDll.averageNum)))
		{
			_intermediateBuffer = (USHORT*) realloc(_intermediateBuffer, _expectedImageSize);
		}
		else
		{
			SAFE_DELETE_MEMORY(_intermediateBuffer);
		}

		//de-signal Stop Acquisition Event at the end
		ResetEvent(_hStopAcquisition);
	}
	catch(...)
	{
		ThorTSIErrChk(L"tl_camera_disarm", tl_camera_disarm(_camera[_camID]), 0);
		ret = FALSE;
		GetLastError();
		StringCbPrintfW(_errMsg,MSG_SIZE,L"SetBdDMA failed");
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return ret;
}

void ThorCam::StopCamera(long cameraIndex)
{
	if(IsOpen(cameraIndex))
	{
		tl_camera_disarm(_camera[cameraIndex]);
		tl_camera_set_frame_available_callback(_camera[cameraIndex], NULL, NULL);
		SetEvent(_hStopAcquisition);
		_cameraRunning[cameraIndex] = false;

		//reset the copy buffer index here,
		//for camera to restart without preflight->setup:
		_bufferImageIndex = 0;
	}
}

/// ********************************************	Callback Functions	 ******************************************** ///

void ThorCam::FrameAvailableCallback(void* sender, unsigned short* image_buffer, int frame_count, unsigned char* metadata, int metadata_size_in_bytes, void* context)
{
	long cameraIndex = (long)(intmax_t)context;

	//consider overflow count from camera
	unsigned long long frameCount = (0 > frame_count) ? (static_cast<unsigned long long>(frame_count) + INT_MAX + 1 + INT_MAX + 1) : static_cast<unsigned long long>(frame_count);

	//frame index from camera callback sometimes not starting from 1
	//after frequently restart of camera, mark offset value in 0-based
	if (0 == _1stSet_Frame)
	{
		_frameCountOffset = frameCount - 1;
		_bufferImageIndex += _frameCountOffset;
	}

	//process frames include the current frame (one frame per callback) and dropped frames
	unsigned long long ProcessFrameCount = ((1 == _cameraRunning[_camID]) && (frameCount > _bufferImageIndex)) ? (frameCount - _bufferImageIndex) : 0;

	switch ((TriggerMode)_imgPtyDll.triggerMode)
	{
	case SW_FREE_RUN_MODE:
		//ignore dropped frames
		ProcessFrameCount = 1;

		//keep the lastest one for copy in free run
		if (0 < ThorCam::_imagePropertiesQueue.size())
			return;
		break;
	case HW_MULTI_FRAME_TRIGGER_EACH_BULB:
		break;
	default:
		//return if exceed target frame count
		if(_imgPtyDll.numFrame + _frameCountOffset < frameCount)
		{
			ThorCam::_maxFrameCountReached = TRUE;
			return;
		}
		break;
	}

	if (IsOpen(cameraIndex) && (WAIT_OBJECT_0 != WaitForSingleObject(_hStopAcquisition, 0)))
	{
		/// IMPORTANT: The following code needs to be implemented once the metadata includes all the necessary values to calculate the size of the frame.
		/// The current version of the API doesn't have enough data to calculate this. According to TSI in future releases of the API, the metadata
		/// will include these values.
		//unsigned long frameSizeInBytes = image_width * image_height * (bit_depth / Constants::BITS_PER_BYTE) * number_of_color_channels;
		///check the size matches the size of the allocated buffer. If the size doesn't match don't copy further than the data goes, or the allocated buffer size
		//ThorCam::_lastCopiedImageSize = (ThorCam::_expectedImageSize <= frameSizeInBytes) ? ThorCam::_expectedImageSize : frameSizeInBytes;

		ThorCam::_lastCopiedImageSize = ThorCam::_expectedImageSize;

		for (int i = 0; i < ProcessFrameCount; i++)
		{
			long currentDMAIndex = (ThorCam::_bufferImageIndex+_imgPtyDll.dmaBufferCount)%_imgPtyDll.dmaBufferCount;

			//Keep the important image metadata such as width, height, and frame number.			
			ImageProperties ImageProperties;

			if((ProcessFrameCount - 1) == i)
			{
				WaitForSingleObject(_hFrmBufHandle, Constants::TIMEOUT_MS);

				//copy the image
				_pFrmDllBuffer[currentDMAIndex].LockMem();
				_pFrmDllBuffer[currentDMAIndex].CopyMem(image_buffer, ThorCam::_lastCopiedImageSize);
				_pFrmDllBuffer[currentDMAIndex].UnlockMem();

				//keep the size of the copied image, in case it doesn't match the allocated size
				ImageProperties.sizeInBytes = ThorCam::_lastCopiedImageSize;

				ReleaseMutex(_hFrmBufHandle);
			}
			else
			{
				//skip queue by zero size
				ImageProperties.sizeInBytes = 0;			
			}

			ImageProperties.width = ((_imgPtyDll.roiRight - _imgPtyDll.roiLeft + 1) / _imgPtyDll.roiBinX);
			ImageProperties.height = ((_imgPtyDll.roiBottom - _imgPtyDll.roiTop + 1) / _imgPtyDll.roiBinY);
			ImageProperties.frameNumber = frameCount - ProcessFrameCount + (i + 1);

			//keep the global index of the buffer where the image was copied
			ImageProperties.bufferIndex = ThorCam::_bufferImageIndex;

			//increase the index of the circular buffer
			++ThorCam::_bufferImageIndex;

			//put the image metadata in a queue for retrieval at a later point
			ThorCam::_imagePropertiesQueue.push(ImageProperties);

			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorTSI_CS imagePropertiesQueue push buffer (%llu)", ImageProperties.bufferIndex);
			LogMessage(_errMsg,VERBOSE_EVENT);
		}
	}

	//mark camera callback is done
	_1stSet_Frame = 1;
}

void ThorCam::CameraConnectedCallback(char* cameraSerialNumber, enum TL_CAMERA_USB_PORT_TYPE usb_bus_speed, void* context)
{
	std::string camSN = cameraSerialNumber;
	FindAllCameras();
	for (int i = 0; i < MAX_CAM_NUM; i++)
	{
		if(0 == camSN.compare(_camSerial[i]))
		{
			_cameraInterfaceType[i] = usb_bus_speed;
		}
	}
}

void ThorCam::CameraDisconnectedCallback(char* cameraSerialNumber, void* context)
{
	std::string camSN = cameraSerialNumber;
	for (int i = 0; i < MAX_CAM_NUM; i++)
	{
		if(0 == camSN.compare(_camSerial[i]))
		{
			DeSelectCamera(i);
		}
	}
}

/// ********************************************	Generic Functions	 ******************************************** ///

long ThorCam::FindCameras(long &cameraCount)
{
	FindAllCameras();
	cameraCount = _numCameras;

	if(0 >= cameraCount && _sdkIsOpen)
	{
		if (tl_camera_close_sdk())
		{
			MessageBox(NULL,L"Failed to close SDK", L"SDK Close error", MB_OK);
		}
		tl_camera_sdk_dll_terminate();
		_sdkIsOpen = false;
		return FALSE;
	}
	return TRUE;
}

long ThorCam::SelectCamera(const long camera)
{
	//validate selection
	if(!IsOpen(camera))
		return FALSE;

	//if the camera is open successfully, then change the selectedCam index to the new one
	_camID = camera;
	InitialParamInfo();

	//update all params' ranges
	for (long i = ICamera::PARAM_FIRST_PARAM; i < ICamera::PARAM_LAST_PARAM; i++)
	{
		long paramLong;
		double paramDouble, paramMax;
		GetParamInfo(i,paramLong,paramLong,paramLong,paramDouble,paramMax,paramDouble);
	}

	SAFE_DELETE_ARRAY (_pDetectorName);
	std::wstring wCamName = StringToWString(_camName[_camID]);
	_pDetectorName = new wchar_t[wCamName.length() + 1];
	SAFE_MEMCPY(_pDetectorName, (wCamName.length() + 1)*sizeof(wchar_t), wCamName.c_str());

	SAFE_DELETE_ARRAY (_pSerialNumber);
	std::wstring snWideString = StringToWString(_camSerial[_camID]);
	_pSerialNumber = new wchar_t[snWideString.length() + 1];
	SAFE_MEMCPY(_pSerialNumber, (snWideString.length() + 1)*sizeof(wchar_t), snWideString.c_str());

	return TRUE;
}

long ThorCam::TeardownCamera()
{
	ClearAllCameras();
	ClearMem();
	return TRUE;
}

long ThorCam::PreflightAcquisition(char* pData)
{
	long ret = TRUE;

	//copy the new settings
	_ImgPty.exposureTime_us =_ImgPty_SetSettings.exposureTime_us;	
	_ImgPty.roiBinX = _ImgPty_SetSettings.roiBinX;
	_ImgPty.roiBinY = _ImgPty_SetSettings.roiBinY;
	_ImgPty.roiBottom = _ImgPty_SetSettings.roiBottom;
	_ImgPty.roiLeft = _ImgPty_SetSettings.roiLeft;
	_ImgPty.roiRight = _ImgPty_SetSettings.roiRight;
	_ImgPty.roiTop = _ImgPty_SetSettings.roiTop;
	_ImgPty.triggerMode = _ImgPty_SetSettings.triggerMode;
	_ImgPty.triggerPolarity = _ImgPty_SetSettings.triggerPolarity;
	_ImgPty.bitPerPixel = _ImgPty_SetSettings.bitPerPixel;
	_ImgPty.pixelSizeXUM = _ImgPty_SetSettings.pixelSizeXUM;	
	_ImgPty.pixelSizeYUM = _ImgPty_SetSettings.pixelSizeYUM;
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
	_ImgPty.hotPixelEnabled = _ImgPty_SetSettings.hotPixelEnabled;
	_ImgPty.hotPixelThreshold = _ImgPty_SetSettings.hotPixelThreshold;
	_ImgPty.gain = _ImgPty_SetSettings.gain;
	_ImgPty.blackLevel = _ImgPty_SetSettings.blackLevel;
	_ImgPty.frameRateControlEnabled = _ImgPty_SetSettings.frameRateControlEnabled;
	_ImgPty.frameRateControlValue = _ImgPty_SetSettings.frameRateControlValue;

	//set the the camera settings and allocate the DMA buffer
	if (TRUE == SetBdDMA(&_ImgPty))
	{
		//if the camera parameters are set and the DMA buffer is allocated successfully,
		//copy the new settings to compare later on.
		_ImgPty_Pre.exposureTime_us = _ImgPty.exposureTime_us;	
		_ImgPty_Pre.roiBinX = _ImgPty.roiBinX;
		_ImgPty_Pre.roiBinY = _ImgPty.roiBinY;
		_ImgPty_Pre.roiBottom = _ImgPty.roiBottom;
		_ImgPty_Pre.roiLeft = _ImgPty.roiLeft;
		_ImgPty_Pre.roiRight = _ImgPty.roiRight;
		_ImgPty_Pre.roiTop = _ImgPty.roiTop;
		_ImgPty_Pre.widthPx = _ImgPty.widthPx;
		_ImgPty_Pre.heightPx = _ImgPty.heightPx;
		_ImgPty_Pre.triggerMode = _ImgPty.triggerMode;
		_ImgPty_Pre.triggerPolarity = _ImgPty.triggerPolarity;
		_ImgPty_Pre.bitPerPixel = _ImgPty.bitPerPixel;
		_ImgPty_Pre.pixelSizeXUM = _ImgPty.pixelSizeXUM;	
		_ImgPty_Pre.pixelSizeYUM = _ImgPty.pixelSizeYUM;
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
		_ImgPty_Pre.hotPixelEnabled = _ImgPty.hotPixelEnabled;
		_ImgPty_Pre.hotPixelThreshold = _ImgPty.hotPixelThreshold;
		_ImgPty_Pre.gain = _ImgPty.gain;
		_ImgPty_Pre.blackLevel = _ImgPty.blackLevel;
		_ImgPty_Pre.frameRateControlEnabled = _ImgPty.frameRateControlEnabled;
		_ImgPty_Pre.frameRateControlValue = _ImgPty.frameRateControlValue;
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
	long ret = TRUE;
	if (!IsOpen(_camID)) return FALSE;

	//check if there have been any changes that need the experiment to be stopped
	if ((_ImgPty_Pre.roiBinX != _ImgPty_SetSettings.roiBinX) ||
		(_ImgPty_Pre.roiBinY != _ImgPty_SetSettings.roiBinY) ||
		(_ImgPty_Pre.roiBottom != _ImgPty_SetSettings.roiBottom) ||
		(_ImgPty_Pre.roiLeft != _ImgPty_SetSettings.roiLeft) ||
		(_ImgPty_Pre.roiRight != _ImgPty_SetSettings.roiRight) ||
		(_ImgPty_Pre.roiTop != _ImgPty_SetSettings.roiTop) ||
		(_ImgPty_Pre.triggerMode != _ImgPty_SetSettings.triggerMode) ||
		(_ImgPty_Pre.triggerPolarity != _ImgPty_SetSettings.triggerPolarity) ||
		(_ImgPty_Pre.bitPerPixel != _ImgPty_SetSettings.bitPerPixel) ||
		(_ImgPty_Pre.pixelSizeXUM != _ImgPty_SetSettings.pixelSizeXUM) ||
		(_ImgPty_Pre.pixelSizeYUM != _ImgPty_SetSettings.pixelSizeYUM) ||
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
		(_ImgPty_Pre.hotPixelEnabled != _ImgPty_SetSettings.hotPixelEnabled) ||
		(_ImgPty_Pre.hotPixelThreshold != _ImgPty_SetSettings.hotPixelThreshold)||
		(_ImgPty_Pre.gain != _ImgPty_SetSettings.gain) ||
		(_ImgPty_Pre.blackLevel != _ImgPty_SetSettings.blackLevel) ||
		(_ImgPty_Pre.frameRateControlEnabled != _ImgPty_SetSettings.frameRateControlEnabled) ||
		(_ImgPty_Pre.frameRateControlValue != _ImgPty_SetSettings.frameRateControlValue) ||
		(TRUE == _forceSettingsUpdate)
		)
	{
		_forceSettingsUpdate = FALSE;
		//if there was any change, copy the new settings
		_ImgPty.roiBinX = _ImgPty_SetSettings.roiBinX;
		_ImgPty.roiBinY = _ImgPty_SetSettings.roiBinY;
		_ImgPty.roiBottom = _ImgPty_SetSettings.roiBottom;
		_ImgPty.roiLeft = _ImgPty_SetSettings.roiLeft;
		_ImgPty.roiRight = _ImgPty_SetSettings.roiRight;
		_ImgPty.roiTop = _ImgPty_SetSettings.roiTop;
		_ImgPty.triggerMode = _ImgPty_SetSettings.triggerMode;
		_ImgPty.triggerPolarity = _ImgPty_SetSettings.triggerPolarity;
		_ImgPty.bitPerPixel = _ImgPty_SetSettings.bitPerPixel;	
		_ImgPty.pixelSizeXUM = _ImgPty_SetSettings.pixelSizeXUM;
		_ImgPty.pixelSizeYUM = _ImgPty_SetSettings.pixelSizeYUM;
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
		_ImgPty.hotPixelEnabled = _ImgPty_SetSettings.hotPixelEnabled;
		_ImgPty.hotPixelThreshold = _ImgPty_SetSettings.hotPixelThreshold;
		_ImgPty.gain = _ImgPty_SetSettings.gain;
		_ImgPty.blackLevel = _ImgPty_SetSettings.blackLevel;
		_ImgPty.frameRateControlEnabled = _ImgPty_SetSettings.frameRateControlEnabled;
		_ImgPty.frameRateControlValue = _ImgPty_SetSettings.frameRateControlValue;

		//set the the camera settings and allocate the DMA buffer
		if (TRUE == SetBdDMA(&_ImgPty))
		{
			//if the camera parameters are set and the DMA buffer is allocated successfully,
			//copy the new settings to compare later on.
			_ImgPty_Pre.roiBinX = _ImgPty.roiBinX;
			_ImgPty_Pre.roiBinY = _ImgPty.roiBinY;
			_ImgPty_Pre.roiBottom = _ImgPty.roiBottom;
			_ImgPty_Pre.roiLeft = _ImgPty.roiLeft;
			_ImgPty_Pre.roiRight = _ImgPty.roiRight;
			_ImgPty_Pre.roiTop = _ImgPty.roiTop;
			_ImgPty_Pre.widthPx = _ImgPty.widthPx;
			_ImgPty_Pre.heightPx = _ImgPty.heightPx;
			_ImgPty_Pre.triggerMode = _ImgPty.triggerMode;
			_ImgPty_Pre.triggerPolarity = _ImgPty.triggerPolarity;
			_ImgPty_Pre.bitPerPixel = _ImgPty.bitPerPixel;
			_ImgPty_Pre.pixelSizeXUM = _ImgPty.pixelSizeXUM;
			_ImgPty_Pre.pixelSizeYUM = _ImgPty.pixelSizeYUM;
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
			_ImgPty_Pre.hotPixelEnabled = _ImgPty.hotPixelEnabled;
			_ImgPty_Pre.hotPixelThreshold = _ImgPty.hotPixelThreshold;
			_ImgPty_Pre.gain = _ImgPty.gain;
			_ImgPty_Pre.blackLevel = _ImgPty.blackLevel;
			_ImgPty_Pre.frameRateControlEnabled = _ImgPty.frameRateControlEnabled;
			_ImgPty_Pre.frameRateControlValue = _ImgPty.frameRateControlValue;
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

long ThorCam::StartAcquisition(char* pDataBuffer)
{
	long ret = TRUE;

	if (!IsOpen(_camID)) return FALSE;

	//Only start the camera once after stopping it
	if (_cameraRunning[_camID]) return TRUE;

	try
	{
		/// IMPORTANT: The following code was implemented to keep the _expectedImageSize Updated for FrameAvailableCallback, otherwise Capture Setup 
		/// will crash after any pixel change. This can be deleted once TSI updates the metadata paramater from the callback function
		_expectedImageSize = _imgPtyDll.channel * ((_imgPtyDll.roiRight - _imgPtyDll.roiLeft + 1) / _imgPtyDll.roiBinX) * ((_imgPtyDll.roiBottom - _imgPtyDll.roiTop + 1) / _imgPtyDll.roiBinY) * sizeof(unsigned short);

		//set the image callback function
		tl_camera_set_frame_available_callback(_camera[_camID], FrameAvailableCallback, (void*)_camID);

		//arm camera
		_frameCountOffset = 0;
		_1stSet_Frame = 0;
		ThorTSIErrChk(L"tl_camera_arm", tl_camera_arm(_camera[_camID], _imgPtyDll.numImagesToBuffer), 1);
		_cameraRunning[_camID] = true;

		//If set to HW trigger mode, allow some time for the camera to be ready to receive the trigger
		switch (_imgPtyDll.triggerMode)
		{
		case ICamera::HW_SINGLE_FRAME:
		case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
		case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		case ICamera::HW_TDI_TRIGGER_MODE:
			Sleep(200);
			break;
		default:
			ThorTSIErrChk(L"tl_camera_issue_software_trigger", tl_camera_issue_software_trigger(_camera[_camID]), 1);
			break;
		}
	}
	catch(...)
	{
		ThorTSIErrChk(L"tl_camera_disarm", tl_camera_disarm(_camera[_camID]), 0);
		ret = FALSE;
		_cameraRunning[_camID] = false;
		GetLastError();
		StringCbPrintfW(_errMsg,MSG_SIZE,L"StartAcquisition failed");
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return ret;
}

long ThorCam::StatusAcquisition(long &status)
{
	// terminate when over frame count limit
	if((TRUE == ThorCam::_maxFrameCountReached) && (_cameraRunning[_camID]))
	{
		_single->getInstance()->PostflightAcquisition(NULL);
	}

	size_t size = _imagePropertiesQueue.size();
	if (0 < size)
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
		status = (TRUE == ThorCam::_maxFrameCountReached) ? ICamera::STATUS_READY : ICamera::STATUS_BUSY;
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

	ImageProperties imageProperties;
	unsigned short* dst = (unsigned short*)pDataBuffer;

	//get the image metadata from the front of the queue
	imageProperties = _imagePropertiesQueue.pop();

	//only copy when the size matches, leave blank on dropping frames
	if ((imageProperties.sizeInBytes != _expectedImageSize)) 
	{
		_previousLastImage = _lastImage;
		_lastImage = imageProperties.frameNumber;
		return FALSE;
	}

	unsigned long currentDMAIndex = (imageProperties.bufferIndex+_imgPtyDll.dmaBufferCount)%_imgPtyDll.dmaBufferCount;
	long average = (((ICamera::AVG_MODE_CUMULATIVE == _imgPtyDll.averageMode) && (1 < _imgPtyDll.averageNum)) && (0 < imageProperties.bufferIndex)) ? TRUE : FALSE;

	WaitForSingleObject(_hFrmBufHandle, INFINITE);

	//lock memory before process
	if(!_pFrmDllBuffer[currentDMAIndex].TryLockMem(Constants::TIMEOUT_MS))
	{
		ReleaseMutex(_hFrmBufHandle);
		return FALSE;
	}

	//copy current to output buffer and process on output directly
	SAFE_MEMCPY(dst, imageProperties.sizeInBytes, _pFrmDllBuffer[currentDMAIndex].GetMem());

	//unlock current after copy
	_pFrmDllBuffer[currentDMAIndex].UnlockMem();

	//average before flip/rotate, but no average first frame; use intermediate buffer
	if((average) && (NULL != _intermediateBuffer))
	{
		unsigned long previousDMAIndex = (imageProperties.bufferIndex-1+_imgPtyDll.dmaBufferCount)%_imgPtyDll.dmaBufferCount;

		if(SW_FREE_RUN_MODE == (TriggerMode)(_imgPtyDll.triggerMode))
		{
			for (unsigned long avgID = 0; avgID < min(static_cast<unsigned long>(_imgPtyDll.averageNum), imageProperties.bufferIndex); avgID++)
			{
				previousDMAIndex = (imageProperties.bufferIndex-1-avgID+_imgPtyDll.dmaBufferCount)%_imgPtyDll.dmaBufferCount;
				ProcessAverageFrame(dst, previousDMAIndex);
			}
		}
		else
		{				
			ProcessAverageFrame(dst, previousDMAIndex);
		}
	}

	ReleaseMutex(_hFrmBufHandle);

	//start image processing: flip, rotate
	USHORT* pLocal = _intermediateBuffer;
	IppiSize size;
	size.width = imageProperties.width;
	size.height = imageProperties.height;
	int stepSrc = size.width * sizeof(unsigned short);
	IppiRect roiSrc = {0, 0, size.width, size.height};

	//flip
	if (TRUE == _imgPtyDll.horizontalFlip || TRUE == _imgPtyDll.verticalFlip)
	{
		if(NULL == pLocal)	return FALSE;

		if (TRUE == _imgPtyDll.horizontalFlip && TRUE == _imgPtyDll.verticalFlip)
		{
			ippiDll->ippiMirror_16u_C1R(dst, stepSrc, pLocal, stepSrc,  size, ippAxsBoth);
		}
		else if(TRUE == _imgPtyDll.horizontalFlip)
		{
			ippiDll->ippiMirror_16u_C1R(dst, stepSrc, pLocal, stepSrc,  size, ippAxsVertical);
		}
		else
		{
			ippiDll->ippiMirror_16u_C1R(dst, stepSrc, pLocal, stepSrc,  size, ippAxsHorizontal);
		}
		//update after flipped
		SAFE_MEMCPY(dst, imageProperties.sizeInBytes, pLocal);
	}

	//rotate
	switch (_imgPtyDll.imageAngle)
	{
	case 90:
		{
			if(NULL == pLocal)	return FALSE;
			int stepDst = imageProperties.height * sizeof(unsigned short);
			IppiRect  roiDst = {0, 0, size.height, size.width};
			long angle = 90;
			long xOffset = 0;
			long yOffset = imageProperties.width - 1;
			ippiDll->ippiRotate_16u_C1R(dst, size, stepSrc, roiSrc, pLocal, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			SAFE_MEMCPY(dst, imageProperties.sizeInBytes, pLocal);	//update after rotated
		}
		break;
	case 180:
		{
			if(NULL == pLocal)	return FALSE;
			int stepDst = imageProperties.width * sizeof(unsigned short);
			IppiRect  roiDst = {0, 0, size.width, size.height};
			long angle = 180;
			long xOffset = imageProperties.width - 1;
			long yOffset = imageProperties.height - 1;
			ippiDll->ippiRotate_16u_C1R(dst, size, stepSrc, roiSrc, pLocal, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			SAFE_MEMCPY(dst, imageProperties.sizeInBytes, pLocal);	//update after rotated
		}
		break;
	case 270:
		{
			if(NULL == pLocal)	return FALSE;
			int stepDst = imageProperties.height * sizeof(unsigned short);
			IppiRect  roiDst = {0, 0, size.height, size.width};
			long angle = 270;
			long xOffset = imageProperties.height - 1;
			long yOffset = 0;
			ippiDll->ippiRotate_16u_C1R(dst, size, stepSrc, roiSrc, pLocal, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			SAFE_MEMCPY(dst, imageProperties.sizeInBytes, pLocal);	//update after rotated
		}
		break;
	}

	//keep track of the frame number, and the previous frame number
	_previousLastImage = _lastImage;
	_lastImage = imageProperties.frameNumber;
	_availableFramesCnt = _imagePropertiesQueue.size();
	return TRUE;
}

long ThorCam::PostflightAcquisition(char * pDataBuffer)
{
	long ret = TRUE;
	try
	{
		//Stop the camera, set the image callback function to null
		for (long i = 0; i < MAX_CAM_NUM; ++i)
		{
			StopCamera(i);
		}
	}
	catch(...)
	{
		ret = FALSE;
		GetLastError();
		StringCbPrintfW(_errMsg,MSG_SIZE,L"PostflightAcquisition failed");
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return ret;
}
