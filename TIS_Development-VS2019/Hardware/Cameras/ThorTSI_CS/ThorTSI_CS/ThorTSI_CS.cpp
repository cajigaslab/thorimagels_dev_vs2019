// ThorTSI_CS.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "ThorTSI_CS.h"
#include <wchar.h>

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
	MAX_CHANNEL(0b0111),
	DEFAULT_CHANNEL(1),
	MIN_BITS_PERPIXEL(8),
	MAX_BITS_PERPIXEL(16)
{	
	for (long i = 0; i < MAX_CAM_NUM; ++i)
	{
		_cameraParams[i].cameraHandle = nullptr;
		_cameraParams[i].colorProcessorHandle = nullptr;
		_cameraParams[i].colorProcessorHandleForWhiteBalance = nullptr;
		_cameraParams[i].polarProcessorHandle = nullptr;
	}
	_errMsg[0] = 0;

	_CameraProperties_UI.exposureTime_us = 30u * Constants::MS_TO_SEC;	
	_CameraProperties_UI.triggerMode = ICamera::TriggerMode::SW_FREE_RUN_MODE;
	_CameraProperties_UI.triggerPolarity = TL_CAMERA_TRIGGER_POLARITY::TL_CAMERA_TRIGGER_POLARITY_ACTIVE_HIGH;
	_CameraProperties_UI.bitPerPixel = MAX_BITS_PERPIXEL;
	_CameraProperties_UI.pixelSizeXUM = 0;
	_CameraProperties_UI.pixelSizeYUM = 0;
	_CameraProperties_UI.roiBinX = 1;
	_CameraProperties_UI.roiBinY = 1;
	_CameraProperties_UI.roiBottom = 0;
	_CameraProperties_UI.roiLeft = 0;
	_CameraProperties_UI.roiRight = 0;
	_CameraProperties_UI.roiTop = 0;
	_CameraProperties_UI.widthPx = 0;
	_CameraProperties_UI.heightPx = 0;
	_CameraProperties_UI.numImagesToBuffer = MIN_IMAGE_BUFFERS;
	_CameraProperties_UI.readOutSpeedIndex = 1;
	_CameraProperties_UI.numChannels = 1;
	_CameraProperties_UI.channelBitmask = 0b0001;
	_CameraProperties_UI.averageMode = 0;
	_CameraProperties_UI.averageNum = 1;
	_CameraProperties_UI.numFrame = 0;
	_CameraProperties_UI.dmaBufferCount = DEFAULT_DMABUFNUM;
	_CameraProperties_UI.verticalFlip = FALSE;
	_CameraProperties_UI.horizontalFlip = FALSE;
	_CameraProperties_UI.imageAngle = 0;
	_CameraProperties_UI.hotPixelEnabled = 0;
	_CameraProperties_UI.hotPixelThreshold = 0;
	_CameraProperties_UI.gain = 0;
	_CameraProperties_UI.blackLevel = 0;
	_CameraProperties_UI.frameRateControlEnabled = FALSE;
	_CameraProperties_UI.frameRateControlValue = 0;
	_CameraProperties_UI.colorImageType = 0;
	_CameraProperties_UI.redGain = 1.0;
	_CameraProperties_UI.greenGain = 1.0;
	_CameraProperties_UI.blueGain = 1.0;
	_CameraProperties_UI.polarImageType = 0;
	_CameraProperties_UI.isEqualExposurePulseEnabled = 0;
	_CameraProperties_UI.equalExposurePulseWidth = 1.0;

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
CameraProperties ThorCam::_CameraProperties_Active = CameraProperties();
shared_ptr <ThorCam> ThorCam::_single(NULL);
bool ThorCam::_isCameraSdkOpen = false;
bool ThorCam::_isColorSdkOpen = false;
bool ThorCam::_isPolarSdkOpen = false;
ThreadSafeMem<USHORT> ThorCam::_pFrmDllBuffer[MAX_DMABUFNUM];
unsigned long long ThorCam::_bufferImageIndex = 0;
unsigned long ThorCam::_expectedImageSizeFromCamera = 0;
unsigned long ThorCam::_expectedProcessedImageSize = 0;
unsigned long ThorCam::_lastCopiedImageSize = 0;
unsigned long long ThorCam::_frameCountOffset = 0;
long ThorCam::_1stSet_Frame = 0;
ThreadSafeQueue<ImageProperties> ThorCam::_imagePropertiesQueue;
StaticCameraParams ThorCam::_cameraParams[MAX_CAM_NUM] = { StaticCameraParams() };
long ThorCam::_numCameras = 0;
long ThorCam::_camID = -1;
wchar_t ThorCam::_errMsg[MSG_SIZE] = {NULL};
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
	if (_isCameraSdkOpen)
	{
		Sleep(10);
		if (tl_camera_close_sdk())
		{
			MessageBox(NULL,L"Failed to close SDK", L"SDK Close error", MB_OK);
		}
		tl_camera_sdk_dll_terminate();
		_isCameraSdkOpen = false;
	}

	if (_isColorSdkOpen)
	{
		if (tl_mono_to_color_processing_terminate())
		{
			MessageBox(NULL, L"Failed to terminate Color Processing SDK", L"Color Processing SDK termination error", MB_OK);
		}
		_isColorSdkOpen = false;
	}

	if (_isPolarSdkOpen)
	{
		if (tl_polarization_processor_terminate())
		{
			MessageBox(NULL, L"Failed to terminate Polar Processing SDK", L"Polar Processing SDK termination error", MB_OK);
		}
		_isPolarSdkOpen = false;
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
		auto* cameraParams = &_cameraParams[cameraIndex];
		cameraParams->camSerial.clear();
		cameraParams->camName.clear();
		cameraParams->camModel.clear();
		cameraParams->cameraSensorType = TL_CAMERA_SENSOR_TYPE_MONOCHROME;
		cameraParams->cameraInterfaceType = TL_CAMERA_USB_PORT_TYPE_USB1_0;
		cameraParams->cameraPolarPhase = TL_POLARIZATION_PROCESSOR_POLAR_PHASE_0_DEGREES;
		ThorTSIErrChk(tl_camera_disarm(cameraParams->cameraHandle), 0);
		ThorTSIErrChk(tl_camera_close_camera(cameraParams->cameraHandle), 0);
		cameraParams->cameraHandle = nullptr;
		if (nullptr != cameraParams->colorProcessorHandle)
		{
			ThorTSIColorErrChk(tl_mono_to_color_destroy_mono_to_color_processor(cameraParams->colorProcessorHandle), 0);
			cameraParams->colorProcessorHandle = nullptr;
		}
		if (nullptr != cameraParams->colorProcessorHandleForWhiteBalance)
		{
			ThorTSIColorErrChk(tl_mono_to_color_destroy_mono_to_color_processor(cameraParams->colorProcessorHandleForWhiteBalance), 0);
			cameraParams->colorProcessorHandleForWhiteBalance = nullptr;
		}
		if (nullptr != cameraParams->polarProcessorHandle)
		{
			ThorTSIPolarErrChk(tl_polarization_processor_destroy_polarization_processor(cameraParams->polarProcessorHandle), 0);
			cameraParams->polarProcessorHandle = nullptr;
		}
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
		if(!_isCameraSdkOpen)
		{
			int initializeResult = tl_camera_sdk_dll_initialize();
			if (0 != initializeResult) 
			{
				wstring errorMsg = L"Failed to initialize camera SDK. Error code: ";
				errorMsg += to_wstring(initializeResult);
				MessageBox(NULL, errorMsg.c_str(), L"Camera SDK initialization error", MB_OK);
			}
			int openSdkResult = tl_camera_open_sdk();
			if (0 != openSdkResult) 
			{
				wstring errorMsg = L"Failed to open camera SDK. Error message: ";
				wstring sdkError;
				CharStringIntoWString(tl_camera_get_last_error(), sdkError);
				errorMsg += sdkError;
				MessageBox(NULL, errorMsg.c_str(), L"Camera SDK Open Error", MB_OK);
			}
			ThorTSIErrChk(tl_camera_set_camera_connect_callback(CameraConnectedCallback, NULL), 0);
			ThorTSIErrChk(tl_camera_set_camera_disconnect_callback(CameraDisconnectedCallback, NULL), 0);
			_isCameraSdkOpen = true;
		}
		ThorTSIErrChk(tl_camera_discover_available_cameras(serialNum, TSI_TEXT_CMD_SIZE), 1);

		//open color SDK
		if (!_isColorSdkOpen)
		{
			int initializeResult = tl_mono_to_color_processing_initialize();
			if (0 != initializeResult)
			{
				wstring errorMsg = L"Failed to initialize color SDK. Error code: ";
				errorMsg += to_wstring(initializeResult);
				MessageBox(NULL, errorMsg.c_str(), L"Color SDK initialization error", MB_OK);
			}
			_isColorSdkOpen = true;
		}

		//open polar SDK
		if (!_isPolarSdkOpen)
		{
			int initializeResult = tl_polarization_processor_initialize();
			if (0 != initializeResult)
			{
				wstring errorMsg = L"Failed to initialize polar SDK. Error code: ";
				errorMsg += to_wstring(initializeResult);
				MessageBox(NULL, errorMsg.c_str(), L"Polar SDK initialization error", MB_OK);
			}
			_isPolarSdkOpen = true;
		}

		string camera_ids(serialNum);
		char *next_token;
		char* tok = strtok_s(serialNum, " ", &next_token);
		int i = 0;
		while (tok)
		{
			_cameraParams[i].camSerial = tok;
			tok = strtok_s(NULL, " ", &next_token);
			i++;
		}
		for (int j = 0; j < MAX_CAM_NUM; j++)
		{
			if(0 < _cameraParams[j].camSerial.size())
			{
				char camera_id[TSI_TEXT_CMD_SIZE];
				strcpy_s(camera_id, _cameraParams[j].camSerial.c_str());

				if(0 == tl_camera_open_camera(camera_id, &_cameraParams[j].cameraHandle))
				{
					_numCameras++;
					ThorTSIErrChk(tl_camera_get_model(_cameraParams[j].cameraHandle, model, MSG_SIZE), 1);
					_cameraParams[j].camName = _cameraParams[j].camModel = model;// TODO: why not use the camera name?
					ThorTSIErrChk(tl_camera_get_usb_port_type(_cameraParams[j].cameraHandle, &_cameraParams[j].cameraInterfaceType), 1);
					ThorTSIErrChk(tl_camera_get_camera_sensor_type(_cameraParams[j].cameraHandle, &_cameraParams[j].cameraSensorType), 1);
					ThorTSIErrChk(tl_camera_get_is_eep_supported(_cameraParams[j].cameraHandle, (int *)&_cameraParams[j].isEqualExposurePulseSupported), 1);
					//  if camera supports color then initialize color processor
					if (_cameraParams[j].cameraSensorType == TL_CAMERA_SENSOR_TYPE_BAYER)
					{
						enum TL_COLOR_FILTER_ARRAY_PHASE color_filter_array_phase;
						ThorTSIErrChk(tl_camera_get_color_filter_array_phase(_cameraParams[j].cameraHandle, &color_filter_array_phase), 1);
						float color_correction_matrix[9];
						ThorTSIErrChk(tl_camera_get_color_correction_matrix(_cameraParams[j].cameraHandle, color_correction_matrix), 1);
						float default_white_balance_matrix[9];
						ThorTSIErrChk(tl_camera_get_default_white_balance_matrix(_cameraParams[j].cameraHandle, default_white_balance_matrix), 1);
						int bit_depth;
						ThorTSIErrChk(tl_camera_get_bit_depth(_cameraParams[j].cameraHandle, &bit_depth), 1);
						ThorTSIColorErrChk(
							tl_mono_to_color_create_mono_to_color_processor(
								_cameraParams[j].cameraSensorType,
								color_filter_array_phase,
								color_correction_matrix,
								default_white_balance_matrix,
								bit_depth,
								&_cameraParams[j].colorProcessorHandle
							), 1);
						ThorTSIColorErrChk(tl_mono_to_color_set_output_format(_cameraParams[j].colorProcessorHandle, TL_COLOR_FORMAT_RGB_PLANAR), 1);
						// also make a neutral color processor for white balancing
						static float white_balance_matrix_all_zeroes[9] = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
						ThorTSIColorErrChk(
							tl_mono_to_color_create_mono_to_color_processor(
								_cameraParams[j].cameraSensorType,
								color_filter_array_phase,
								color_correction_matrix,
								white_balance_matrix_all_zeroes,
								bit_depth,
								&_cameraParams[j].colorProcessorHandleForWhiteBalance
							), 1);
						ThorTSIColorErrChk(tl_mono_to_color_set_output_format(_cameraParams[j].colorProcessorHandleForWhiteBalance, TL_COLOR_FORMAT_BGR_PIXEL), 1); // TODO: would easier to vectorize BGR PLANAR?
						ThorTSIColorErrChk(tl_mono_to_color_set_color_space(_cameraParams[j].colorProcessorHandleForWhiteBalance, TL_MONO_TO_COLOR_SPACE_LINEAR_SRGB), 1);
					}
					//  if camera supports polar then initialize polar processor
					if (_cameraParams[j].cameraSensorType == TL_CAMERA_SENSOR_TYPE_MONOCHROME_POLARIZED)
					{
						ThorTSIPolarErrChk(tl_polarization_processor_create_polarization_processor(&_cameraParams[j].polarProcessorHandle), 1);
					}

					_cameraParams[j].resyncFlag = false;
					
					double minFrameRate;
					double maxFrameRate;
					ThorTSIErrChk(tl_camera_get_frame_rate_control_value_range(_cameraParams[j].cameraHandle, &minFrameRate, &maxFrameRate), 1);
					_cameraParams[j].isFrameRateControlSupported = maxFrameRate != 0.0;
					
					//ThorTSIErrChk(tl_camera_get_name(_camera[j], name, MSG_SIZE), 1);
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
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorCam Error: %S", err.c_str());
		LogMessage(_errMsg, ERROR_EVENT);
	}
}

void ThorCam::InitialParamInfo(void)
{
	_cameraParams[_camID].cameraHandle;
	ThorTSIErrChk(tl_camera_get_roi_range(_cameraParams[_camID].cameraHandle,&_xRangeL[0],&_yRangeT[0],&_xRangeR[0],&_yRangeB[0],&_xRangeL[1],&_yRangeT[1],&_xRangeR[1],&_yRangeB[1]), 1);
	ThorTSIErrChk(tl_camera_get_binx_range(_cameraParams[_camID].cameraHandle,&_hbinRange[0], &_hbinRange[1]), 1);
	ThorTSIErrChk(tl_camera_get_biny_range(_cameraParams[_camID].cameraHandle,&_vbinRange[0], &_vbinRange[1]), 1);
	ThorTSIErrChk(tl_camera_get_binx(_cameraParams[_camID].cameraHandle,&_CameraProperties_UI.roiBinX), 1);
	ThorTSIErrChk(tl_camera_get_biny(_cameraParams[_camID].cameraHandle,&_CameraProperties_UI.roiBinY), 1);
	ThorTSIErrChk(tl_camera_get_frames_per_trigger_range(_cameraParams[_camID].cameraHandle,&_frmPerTriggerRange[0], &_frmPerTriggerRange[1]), 1);
	ThorTSIErrChk(tl_camera_get_exposure_time_range(_cameraParams[_camID].cameraHandle, &_expUSRange[0], &_expUSRange[1]), 1);
	ThorTSIErrChk(tl_camera_get_bit_depth(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.bitPerPixel), 1);
	ThorTSIErrChk(tl_camera_get_image_width_range(_cameraParams[_camID].cameraHandle, &_widthRange[0], &_widthRange[1]), 1);
	ThorTSIErrChk(tl_camera_get_sensor_width(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.widthPx), 1);
	ThorTSIErrChk(tl_camera_get_image_height_range(_cameraParams[_camID].cameraHandle, &_heightRange[0], &_heightRange[1]), 1);
	ThorTSIErrChk(tl_camera_get_sensor_height(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.heightPx), 1);
	ThorTSIErrChk(tl_camera_get_sensor_pixel_width(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.pixelSizeXUM), 1);
	ThorTSIErrChk(tl_camera_get_sensor_pixel_height(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.pixelSizeYUM), 1);
	ThorTSIErrChk(tl_camera_get_hot_pixel_correction_threshold_range(_cameraParams[_camID].cameraHandle, &_hotPixelRange[0], &_hotPixelRange[1]), FALSE);
	if (0 != _hotPixelRange[1])
	{
		ThorTSIErrChk(tl_camera_get_hot_pixel_correction_threshold(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.hotPixelThreshold), FALSE);
	}
	ThorTSIErrChk(tl_camera_get_is_hot_pixel_correction_enabled(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.hotPixelEnabled), FALSE);
	ThorTSIErrChk(tl_camera_get_gain_range(_cameraParams[_camID].cameraHandle, &_gainRange[0], &_gainRange[1]), FALSE);
	ThorTSIErrChk(tl_camera_get_gain(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.gain), FALSE);
	ThorTSIErrChk(tl_camera_get_black_level_range(_cameraParams[_camID].cameraHandle, &_blackLevelRange[0], &_blackLevelRange[1]), FALSE);
	ThorTSIErrChk(tl_camera_get_black_level(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.blackLevel), FALSE);
	ThorTSIErrChk(tl_camera_get_frame_rate_control_value_range(_cameraParams[_camID].cameraHandle, &_frameRateControlValueRange[0], &_frameRateControlValueRange[1]), FALSE);
	ThorTSIErrChk(tl_camera_get_is_frame_rate_control_enabled(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.frameRateControlEnabled), FALSE);
	ThorTSIErrChk(tl_camera_get_frame_rate_control_value(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.frameRateControlValue), FALSE);
	//ThorTSIErrChk(tl_camera_get_camera_sensor_type(_cameraParams[_camID].cameraHandle, &_CameraProperties_UI.sensorType), FALSE);
	if (TL_CAMERA_SENSOR_TYPE_BAYER == _cameraParams[_camID].cameraSensorType)
	{
		_CameraProperties_UI.colorImageType = 1; //sRGB
		float defaultWhiteBalanceMatrix[9];
		ThorTSIErrChk(tl_camera_get_default_white_balance_matrix(_cameraParams[_camID].cameraHandle, defaultWhiteBalanceMatrix), 1);
		_CameraProperties_UI.redGain = defaultWhiteBalanceMatrix[0];
		_CameraProperties_UI.greenGain = defaultWhiteBalanceMatrix[4];
		_CameraProperties_UI.blueGain = defaultWhiteBalanceMatrix[8];
	}
	else if (TL_CAMERA_SENSOR_TYPE_MONOCHROME_POLARIZED == _cameraParams[_camID].cameraSensorType)
	{
		_CameraProperties_UI.polarImageType = 1; // Intensity
		ThorTSIPolarErrChk(tl_camera_get_polar_phase(_cameraParams[_camID].cameraHandle, &_cameraParams[_camID].cameraPolarPhase), 1);
	}
	else
	{
		_CameraProperties_UI.colorImageType = 0;
		_CameraProperties_UI.polarImageType = 0;
	}
	_CameraProperties_UI.isAutoWhiteBalanceEnabled = false;
	_CameraProperties_UI.whiteBalanceFrameCount = 5;
	_CameraProperties_UI.oneShotWhiteBalanceFlag = false;

	_CameraProperties_UI.numChannels = 1;
	_CameraProperties_UI.channelBitmask = 0b0001;
	if (TL_CAMERA_SENSOR_TYPE_BAYER == _cameraParams[_camID].cameraSensorType && 0 != _CameraProperties_UI.colorImageType)
	{
		_CameraProperties_UI.numChannels = 3;
		_CameraProperties_UI.channelBitmask = 0b0111;
	}
	
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
	if(nullptr == _cameraParams[cameraIndex].cameraHandle)
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
	// NOTE: numChannels is not accounted for here because averaging takes place before color processing
	unsigned long imageSize = ((_CameraProperties_Active.roiRight - _CameraProperties_Active.roiLeft + 1) / _CameraProperties_Active.roiBinX) * ((_CameraProperties_Active.roiBottom - _CameraProperties_Active.roiTop + 1) / _CameraProperties_Active.roiBinY);
	unsigned long sizeInBytes = imageSize * sizeof(unsigned short);

	if (!_pFrmDllBuffer[previousDMAIndex].TryLockMem(Constants::TIMEOUT_MS))
	{
		return;
	}
	USHORT* pCur = dst;
	USHORT* pPre = _pFrmDllBuffer[previousDMAIndex].GetMem();	//previous frame
	for (unsigned long i = 0; i < imageSize; i++)
	{
		*pCur = ((*pCur)*(_CameraProperties_Active.averageNum - 1) + (*pPre)) / _CameraProperties_Active.averageNum;
		pCur++;
		pPre++;
	}
	//output with averaged buffer
	_pFrmDllBuffer[previousDMAIndex].UnlockMem();
}

/// <summary>
/// 
/// Set the Active camera properties to the target properties.
/// Active camera properties are synced with camera.
/// This means active properties may be nudged or rejected by camera.
/// 
/// </summary>
/// <param name="pImgPty"></param>
/// <returns></returns>
long ThorCam::SyncActiveCameraProperties(CameraProperties *pTargetCameraProperties)
{
	std::lock_guard<std::mutex>  paramLock(_paramSyncLock);
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

		bool isColorImageTypeChanged = pTargetCameraProperties->colorImageType != _CameraProperties_Active.colorImageType;
		bool isPolarImageTypeChanged = pTargetCameraProperties->polarImageType != _CameraProperties_Active.polarImageType;
		
		//Set new Cam parameters
		_CameraProperties_Active.CopyFrom(*pTargetCameraProperties);

		//Only average frame when the averageMode is set
		long avgNum = (ICamera::AVG_MODE_NONE == _CameraProperties_Active.averageMode) ? 1  : _CameraProperties_Active.averageNum;

		//set the trigger mode and total number of frames,
		//since we are doing average after capture
		enum TL_CAMERA_OPERATION_MODE current_mode;
		enum TL_CAMERA_TRIGGER_POLARITY current_polarity;

		if (_cameraParams[_camID].isHardwareTriggerSupported || _cameraParams[_camID].isBulbModeSupported)
		{
			ThorTSIErrChk(tl_camera_get_trigger_polarity(_cameraParams[_camID].cameraHandle, &current_polarity), 1);
		}

		ThorTSIErrChk(tl_camera_get_operation_mode(_cameraParams[_camID].cameraHandle, &current_mode), 1);

		switch((TriggerMode)(_CameraProperties_Active.triggerMode))
		{
		case SW_SINGLE_FRAME:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_SOFTWARE_TRIGGERED;
				_CameraProperties_Active.numFrame = avgNum;
				_CameraProperties_Active.numImagesToBuffer = MIN_IMAGE_BUFFERS;
			}
			break;
		case SW_MULTI_FRAME:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_SOFTWARE_TRIGGERED;
				_CameraProperties_Active.numFrame = _CameraProperties_Active.numFrame * avgNum;
				_CameraProperties_Active.numImagesToBuffer = DEFAULT_IMAGE_BUFFERS;
			}
			break;
		case SW_FREE_RUN_MODE:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_SOFTWARE_TRIGGERED;
				_CameraProperties_Active.numFrame = 0;
				_CameraProperties_Active.numImagesToBuffer = MIN_IMAGE_BUFFERS;
			}
			break;
		case HW_SINGLE_FRAME:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_HARDWARE_TRIGGERED;
				_CameraProperties_Active.numFrame = avgNum;
				_CameraProperties_Active.numImagesToBuffer = MIN_IMAGE_BUFFERS;
			}
			break;
		case HW_MULTI_FRAME_TRIGGER_FIRST:
		case HW_MULTI_FRAME_TRIGGER_EACH:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_HARDWARE_TRIGGERED;
				_CameraProperties_Active.numFrame = _CameraProperties_Active.numFrame * avgNum;
				_CameraProperties_Active.numImagesToBuffer = DEFAULT_IMAGE_BUFFERS;
			}
			break;
		case HW_MULTI_FRAME_TRIGGER_EACH_BULB:
			{
				current_mode = TL_CAMERA_OPERATION_MODE::TL_CAMERA_OPERATION_MODE_BULB;
				_CameraProperties_Active.numFrame = _CameraProperties_Active.numFrame * avgNum;
				_CameraProperties_Active.numImagesToBuffer = DEFAULT_IMAGE_BUFFERS;
			}
			break;
		}
		//////****Set/Get Camera Parameters****//////
		//send the parameters to the camera, check if they were set successfuly, log if there was an error
		if (_cameraParams[_camID].isHardwareTriggerSupported || _cameraParams[_camID].isBulbModeSupported)
		{
			current_polarity = TL_CAMERA_TRIGGER_POLARITY::TL_CAMERA_TRIGGER_POLARITY_ACTIVE_HIGH;	//trigger by rising edge
			ThorTSIErrChk(tl_camera_set_trigger_polarity(_cameraParams[_camID].cameraHandle, current_polarity), 1);
		}

		int isOperationModeSupported = 0;
		ThorTSIErrChk(tl_camera_get_is_operation_mode_supported(_cameraParams[_camID].cameraHandle, current_mode, &isOperationModeSupported), 1);

		if (isOperationModeSupported)
		{
			ThorTSIErrChk(tl_camera_set_operation_mode(_cameraParams[_camID].cameraHandle, current_mode), 1);
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SyncActiveCameraProperties attempted to set an invalid operation mode");
			LogMessage(_errMsg, ERROR_EVENT);
		}
		
		//TODO: ??? what is this
		long acceptedFramePerTrigger = ((0 == _cameraParams[_camID].camModel.compare(0, 6, "CS2100")) && (MAX_FRAMENUM < _CameraProperties_Active.numFrame)) ? 0 : _CameraProperties_Active.numFrame;	//[CS2100] use free run mode if more than max frame limit
		ThorTSIErrChk(tl_camera_set_frames_per_trigger_zero_for_unlimited(_cameraParams[_camID].cameraHandle, acceptedFramePerTrigger), 1);

		ThorTSIErrChk(tl_camera_set_binx(_cameraParams[_camID].cameraHandle, _CameraProperties_Active.roiBinX), 1);

		ThorTSIErrChk(tl_camera_set_biny(_cameraParams[_camID].cameraHandle, _CameraProperties_Active.roiBinY), 1);

		ThorTSIErrChk(tl_camera_set_roi(_cameraParams[_camID].cameraHandle, _CameraProperties_Active.roiLeft, _CameraProperties_Active.roiTop, _CameraProperties_Active.roiRight, _CameraProperties_Active.roiBottom), 1);

		ThorTSIErrChk(tl_camera_set_data_rate(_cameraParams[_camID].cameraHandle, (TL_CAMERA_DATA_RATE)_CameraProperties_Active.readOutSpeedIndex), FALSE);

		ThorTSIErrChk(tl_camera_set_gain(_cameraParams[_camID].cameraHandle, (TL_CAMERA_DATA_RATE)_CameraProperties_Active.gain), FALSE);

		ThorTSIErrChk(tl_camera_set_black_level(_cameraParams[_camID].cameraHandle, (TL_CAMERA_DATA_RATE)_CameraProperties_Active.blackLevel), FALSE);

		ThorTSIErrChk(tl_camera_set_is_frame_rate_control_enabled(_cameraParams[_camID].cameraHandle, _CameraProperties_Active.frameRateControlEnabled), FALSE);

		ThorTSIErrChk(tl_camera_set_frame_rate_control_value(_cameraParams[_camID].cameraHandle, _CameraProperties_Active.frameRateControlValue), FALSE);

		// handle Equal Exposure Pulse
		if (_cameraParams[_camID].isEqualExposurePulseSupported)
		{
			// get the sensor readout time now that everything is set on the camera 
			int sensorReadoutTimeNs = 0;
			ThorTSIErrChk(tl_camera_get_sensor_readout_time(_cameraParams[_camID].cameraHandle, &sensorReadoutTimeNs), TRUE);

			if (_CameraProperties_Active.isEqualExposurePulseEnabled)
			{
				// exposure time is sensor read out + pulse width
				double pulseWidthMs = _CameraProperties_Active.equalExposurePulseWidth;
				long long exposureTimeUs = static_cast<long long>(pulseWidthMs * 1000) + static_cast<long long>(sensorReadoutTimeNs / 1000.0);
				ThorTSIErrChk(tl_camera_set_exposure_time(_cameraParams[_camID].cameraHandle, exposureTimeUs), TRUE);
				/*_imgPtyDll.exposureTime_us = _CameraProperties.exposureTime_us = _ImgPty_SetSettings.exposureTime_us = exposureTimeUs;*/
			}
		}

		// readback parameters that could be nudged and set RESYNC flag

		//IMG WIDTH / HEIGHT
		int imageWidth = 0;
		int imageHeight = 0;
		ThorTSIErrChk(tl_camera_get_image_width(_cameraParams[_camID].cameraHandle, &imageWidth), 1);
		ThorTSIErrChk(tl_camera_get_image_height(_cameraParams[_camID].cameraHandle, &imageHeight), 1);
		if (imageWidth != _CameraProperties_Active.widthPx)
		{
			_CameraProperties_Active.widthPx = imageWidth;
			_cameraParams[_camID].resyncFlag = true;
		}
		if (imageWidth != _CameraProperties_Active.heightPx)
		{
			_CameraProperties_Active.heightPx = imageHeight;
			_cameraParams[_camID].resyncFlag = true;
		}
		//ROI
		int roi_x1 = 0;
		int roi_y1 = 0;
		int roi_x2 = 0;
		int roi_y2 = 0;
		ThorTSIErrChk(tl_camera_get_roi(_cameraParams[_camID].cameraHandle, &roi_x1, &roi_y1, &roi_x2, &roi_y2), 1);
		if (_CameraProperties_Active.roiLeft != roi_x1)
		{
			_CameraProperties_Active.roiLeft = roi_x1;
			_cameraParams[_camID].resyncFlag = true;
		}
		if (_CameraProperties_Active.roiTop != roi_y1)
		{
			_CameraProperties_Active.roiTop = roi_y1;
			_cameraParams[_camID].resyncFlag = true;
		}
		if (_CameraProperties_Active.roiRight != roi_x2)
		{
			_CameraProperties_Active.roiRight = roi_x2;
			_cameraParams[_camID].resyncFlag = true;
		}
		if (_CameraProperties_Active.roiBottom != roi_y2)
		{
			_CameraProperties_Active.roiBottom = roi_y2;
			_cameraParams[_camID].resyncFlag = true;
		}
		//EXPOSURE
		long long exposureTime_us;
		ThorTSIErrChk(tl_camera_get_exposure_time(_cameraParams[_camID].cameraHandle, &exposureTime_us), 1);
		if (_CameraProperties_Active.exposureTime_us != exposureTime_us)
		{
			_CameraProperties_Active.exposureTime_us = exposureTime_us;
			_cameraParams[_camID].resyncFlag = true;
		}
		//FRAME RATE CONTROL
		if (_cameraParams[_camID].isFrameRateControlSupported)
		{
			double frameRateControl_fps;
			if (_CameraProperties_Active.frameRateControlEnabled)
			{
				ThorTSIErrChk(tl_camera_get_frame_rate_control_value(_cameraParams[_camID].cameraHandle, &frameRateControl_fps), 1);
				if (_CameraProperties_Active.frameRateControlValue != frameRateControl_fps)
				{
					_CameraProperties_Active.frameRateControlValue = frameRateControl_fps;
					_cameraParams[_camID].resyncFlag = true;
				}
			}
			// only update frame rate control value when it is enabled - causes the UI value to be saved and re-applied when it is re-enabled
			
		}


		//////****End Set/Get Camera Parameters****//////

		//In the future use chan for multi-channel cameras
		_CameraProperties_Active.numChannels = 1; // TODO: all cameras return 1-channel data, even color and polar. after processing, they may be 3~4 channel
		_CameraProperties_Active.channelBitmask = 0b0001;
		if (TL_CAMERA_SENSOR_TYPE_BAYER == _cameraParams[_camID].cameraSensorType && 0 != _CameraProperties_Active.colorImageType)
		{
			_CameraProperties_Active.numChannels = 3;
			_CameraProperties_Active.channelBitmask = 0b0111;
		}

		/*
		keep the expected image size to make sure we don't over step the boundaries of the allocated memory
		when copying to the buffer.
		Note that expected size from camera is always 1-channel; this is because the data from the camera
		is unprocessed, and will be expanded to 3-channel data later for color data.
		*/
		
		_expectedImageSizeFromCamera = 1 * _CameraProperties_Active.widthPx * _CameraProperties_Active.heightPx * sizeof(unsigned short);
		_expectedProcessedImageSize = _CameraProperties_Active.numChannels * _CameraProperties_Active.widthPx * (_CameraProperties_Active.heightPx) * sizeof(unsigned short);

		if((_lastCopiedImageSize != _expectedImageSizeFromCamera) || (0 == _lastCopiedImageSize) || (_lastDMABufferCount != _CameraProperties_Active.dmaBufferCount))
		{		
			for(int k=0; k<_CameraProperties_Active.dmaBufferCount; ++k)
			{	
				if (FALSE == _pFrmDllBuffer[k].SetMem(_expectedImageSizeFromCamera))
				{
					StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorTSI SyncActiveCameraProperties unable to allocate pFrmDllBuffer #(%d), size(%d)", k, _expectedImageSizeFromCamera);
					LogMessage(_errMsg,ERROR_EVENT);
					return FALSE;
				}
			}
			_lastDMABufferCount = _CameraProperties_Active.dmaBufferCount;
		}	

		//use intermediate if need flip, rotate or average:
		if((TRUE == _CameraProperties_Active.horizontalFlip || TRUE == _CameraProperties_Active.verticalFlip) || (0 != _CameraProperties_Active.imageAngle) ||
			((AverageMode::AVG_MODE_CUMULATIVE == _CameraProperties_Active.averageMode) && (1 < _CameraProperties_Active.averageNum)))
		{
			_intermediateBuffer = (USHORT*) realloc(_intermediateBuffer, _expectedProcessedImageSize);
		}
		else
		{
			SAFE_DELETE_MEMORY(_intermediateBuffer);
		}

		// modify the color processor
		if (TL_CAMERA_SENSOR_TYPE_BAYER == _cameraParams[_camID].cameraSensorType)
		{
			if (nullptr == _cameraParams[_camID].colorProcessorHandle)
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorTSI SyncActiveCameraProperties Color Processor was not created for color camera.");
				ThorCam::getInstance()->LogMessage(_errMsg, ERROR_EVENT);
				return FALSE;
			}

			// color image type changed
			if (isColorImageTypeChanged)
			{
				switch (_CameraProperties_Active.colorImageType)
				{
				case(0):
					break; // unprocessed
				case(1):
					ThorTSIColorErrChk(tl_mono_to_color_set_color_space(_cameraParams[_camID].colorProcessorHandle, TL_MONO_TO_COLOR_SPACE_SRGB), 1);
					break;
				case(2):
					ThorTSIColorErrChk(tl_mono_to_color_set_color_space(_cameraParams[_camID].colorProcessorHandle, TL_MONO_TO_COLOR_SPACE_LINEAR_SRGB), 1);
					break;
				default:
					// Error: invalid color image type
					StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorTSI SyncActiveCameraProperties Invalid color image type: %d", _CameraProperties_Active.colorImageType);
					ThorCam::getInstance()->LogMessage(_errMsg, ERROR_EVENT);
					return FALSE;
				}
			}
			float redGain = static_cast<float>(_CameraProperties_Active.redGain);
			float greenGain = static_cast<float>(_CameraProperties_Active.greenGain);
			float blueGain = static_cast<float>(_CameraProperties_Active.blueGain);
			ThorTSIColorErrChk(tl_mono_to_color_set_red_gain(_cameraParams[_camID].colorProcessorHandle, redGain), 1);
			ThorTSIColorErrChk(tl_mono_to_color_set_green_gain(_cameraParams[_camID].colorProcessorHandle, greenGain), 1);
			ThorTSIColorErrChk(tl_mono_to_color_set_blue_gain(_cameraParams[_camID].colorProcessorHandle, blueGain), 1);
			ThorTSIColorErrChk(tl_mono_to_color_get_red_gain(_cameraParams[_camID].colorProcessorHandle, &redGain), 1);
			ThorTSIColorErrChk(tl_mono_to_color_get_green_gain(_cameraParams[_camID].colorProcessorHandle, &greenGain), 1);
			ThorTSIColorErrChk(tl_mono_to_color_get_blue_gain(_cameraParams[_camID].colorProcessorHandle, &blueGain), 1);
			_CameraProperties_Active.redGain = redGain;
			_CameraProperties_Active.greenGain = greenGain;
			_CameraProperties_Active.blueGain = blueGain;
		}

		// modify the polar processor
		if (TL_CAMERA_SENSOR_TYPE_MONOCHROME_POLARIZED == _cameraParams[_camID].cameraSensorType)
		{
			// TODO: since this doesn't impact any resources until acquisition actually occurs, we probably don't need all these checks in SyncActiveCameraProperties
			if (nullptr == _cameraParams[_camID].polarProcessorHandle)
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorTSI SyncActiveCameraProperties Polar Processor was not created for polar camera.");
				ThorCam::getInstance()->LogMessage(_errMsg, ERROR_EVENT);
				return FALSE;
			}
		}



		//de-signal Stop Acquisition Event at the end
		ResetEvent(_hStopAcquisition);
	}
	catch(...)
	{
		ThorTSIErrChk(tl_camera_disarm(_cameraParams[_camID].cameraHandle), 0);
		ret = FALSE;
		GetLastError();
		StringCbPrintfW(_errMsg,MSG_SIZE,L"SyncActiveCameraProperties failed");
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return ret;
}

void ThorCam::StopCamera(long cameraIndex)
{
	if(IsOpen(cameraIndex))
	{
		ThorTSIErrChk(tl_camera_disarm(_cameraParams[cameraIndex].cameraHandle), 0);
		ThorTSIErrChk(tl_camera_set_frame_available_callback(_cameraParams[cameraIndex].cameraHandle, NULL, NULL), 0);
		SetEvent(_hStopAcquisition);
		_cameraParams[cameraIndex].isCameraRunning = false;

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
	unsigned long long ProcessFrameCount = ((1 == _cameraParams[_camID].isCameraRunning) && (frameCount > _bufferImageIndex)) ? (frameCount - _bufferImageIndex) : 0;

	switch ((TriggerMode)_CameraProperties_Active.triggerMode)
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
		if(_CameraProperties_Active.numFrame + _frameCountOffset < frameCount)
		{
			ThorCam::_maxFrameCountReached = TRUE;
			return;
		}
		break;
	}

	if (IsOpen(cameraIndex) && (WAIT_OBJECT_0 != WaitForSingleObject(_hStopAcquisition, 0)))
	{
		ThorCam::_lastCopiedImageSize = ThorCam::_expectedImageSizeFromCamera;

		for (int i = 0; i < ProcessFrameCount; i++)
		{
			long currentDMAIndex = (ThorCam::_bufferImageIndex+ _CameraProperties_Active.dmaBufferCount)% _CameraProperties_Active.dmaBufferCount;

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

			ImageProperties.width = _CameraProperties_Active.widthPx;
			ImageProperties.height = _CameraProperties_Active.heightPx;
			ImageProperties.frameNumber = frameCount - ProcessFrameCount + (i + 1);

			//keep the global index of the buffer where the image was copied
			ImageProperties.bufferIndex = ThorCam::_bufferImageIndex;

			//increase the index of the circular buffer
			++ThorCam::_bufferImageIndex;

			//put the image metadata in a queue for retrieval at a later point
			ThorCam::_imagePropertiesQueue.push(ImageProperties);

			StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorTSI_CS imagePropertiesQueue push buffer index: %llu, size: %llu", ImageProperties.bufferIndex, _imagePropertiesQueue.size());
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
		if(0 == camSN.compare(_cameraParams[i].camSerial))
		{
			_cameraParams[i].cameraInterfaceType = usb_bus_speed;
		}
	}
}

void ThorCam::CameraDisconnectedCallback(char* cameraSerialNumber, void* context)
{
	std::string camSN = cameraSerialNumber;
	for (int i = 0; i < MAX_CAM_NUM; i++)
	{
		if(0 == camSN.compare(_cameraParams[i].camSerial))
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

	if(0 >= cameraCount && _isCameraSdkOpen)
	{
		Sleep(10);
		if (tl_camera_close_sdk())
		{
			MessageBox(NULL,L"Failed to close SDK", L"SDK Close error", MB_OK);
		}
		tl_camera_sdk_dll_terminate();
		_isCameraSdkOpen = false;

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
	std::wstring wCamName = StringToWString(_cameraParams[_camID].camName);
	_pDetectorName = new wchar_t[wCamName.length() + 1];
	SAFE_MEMCPY(_pDetectorName, (wCamName.length() + 1)*sizeof(wchar_t), wCamName.c_str());

	SAFE_DELETE_ARRAY (_pSerialNumber);
	std::wstring snWideString = StringToWString(_cameraParams[_camID].camSerial);
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

	if (FALSE == SyncActiveCameraProperties(&_CameraProperties_UI))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"SyncACtiveCameraProperties failed");
		LogMessage(_errMsg, ERROR_EVENT);
		ret = FALSE;
	}

	// copy the results of the Sync back to the UI properties
	_CameraProperties_UI.CopyFrom(_CameraProperties_Active);

	return ret;
}

long ThorCam::SetupAcquisition(char* pData)
{
	long ret = TRUE;
	if (!IsOpen(_camID)) return FALSE;

	CameraProperties localCopyOfCameraProperties;
	localCopyOfCameraProperties.CopyFrom(_CameraProperties_UI);

	if (!_CameraProperties_Active.CanBeChangedWithoutStop(localCopyOfCameraProperties) || (TRUE == _forceSettingsUpdate))
	{
		if (FALSE == SyncActiveCameraProperties(&localCopyOfCameraProperties))
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SyncActiveCameraProperties failed");
			LogMessage(_errMsg, ERROR_EVENT);
			ret = FALSE;
		}

		// copy the results of the Sync back to the UI properties
		_CameraProperties_UI.CopyFrom(_CameraProperties_Active);
	}

	return ret;
}

long ThorCam::StartAcquisition(char* pDataBuffer)
{
	long ret = TRUE;

	if (!IsOpen(_camID)) return FALSE;

	//Only start the camera once after stopping it
	if (_cameraParams[_camID].isCameraRunning) return TRUE;

	try
	{
		//set the image callback function
		ThorTSIErrChk(tl_camera_set_frame_available_callback(_cameraParams[_camID].cameraHandle, FrameAvailableCallback, (void*)_camID), 1);

		//arm camera
		_frameCountOffset = 0;
		_1stSet_Frame = 0;
		ThorTSIErrChk(tl_camera_arm(_cameraParams[_camID].cameraHandle, _CameraProperties_Active.numImagesToBuffer), 1);
		_cameraParams[_camID].isCameraRunning = true;

		//image size cannot change once armed
		_expectedImageSizeFromCamera = 1 * _CameraProperties_Active.widthPx * _CameraProperties_Active.heightPx * sizeof(unsigned short);
		_expectedProcessedImageSize = _CameraProperties_Active.numChannels * _CameraProperties_Active.widthPx * (_CameraProperties_Active.heightPx) * sizeof(unsigned short);

		_cameraParams[_camID].framesProcessedSinceStart = 0;

		//If set to HW trigger mode, allow some time for the camera to be ready to receive the trigger
		switch (_CameraProperties_Active.triggerMode)
		{
		case ICamera::HW_SINGLE_FRAME:
		case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
		case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		case ICamera::HW_TDI_TRIGGER_MODE:
			Sleep(200);
			break;
		default:
			ThorTSIErrChk(tl_camera_issue_software_trigger(_cameraParams[_camID].cameraHandle), 1);
			break;
		}
	}
	catch(...)
	{
		ThorTSIErrChk(tl_camera_disarm(_cameraParams[_camID].cameraHandle), 0);
		ret = FALSE;
		_cameraParams[_camID].isCameraRunning = false;
		GetLastError();
		StringCbPrintfW(_errMsg,MSG_SIZE,L"StartAcquisition failed");
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return ret;
}

long ThorCam::StatusAcquisition(long &status)
{
	// terminate when over frame count limit
	if((TRUE == ThorCam::_maxFrameCountReached) && (_cameraParams[_camID].isCameraRunning))
	{
		_single->getInstance()->PostflightAcquisition(NULL);
	}

	size_t size = _imagePropertiesQueue.size();
	if (0 < size)
	{
		if (ICamera::SW_FREE_RUN_MODE == ThorCam::_CameraProperties_Active.triggerMode)
		{
			status = ICamera::STATUS_READY;
		}
		else
		{
			if (size > ThorCam::_CameraProperties_Active.dmaBufferCount)
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"RAM buffers could not keep pace with image queue; frames dropped. _lastImage == %d, _previousLastImage == %d, queueSize == %d, _dmaBufferCount == &d", _lastImage, _previousLastImage, size, ThorCam::_CameraProperties_Active.dmaBufferCount);
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

	// TODO: this frameInfo isn't used during snapshot TODO: does this have to be void *?
	FrameInfoStruct* frameInfoStruct = static_cast<FrameInfoStruct*>(frameInfo);

	ImageProperties imageProperties;
	unsigned short* dst = (unsigned short*)pDataBuffer;

	//get the image metadata from the front of the queue
	imageProperties = _imagePropertiesQueue.pop();
	StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorTSI_CS imagePropertiesQueue pop buffer size: (%llu)", _imagePropertiesQueue.size());
	LogMessage(_errMsg,VERBOSE_EVENT);

	//setup frameInfo return value

	frameInfoStruct->fullFrame = imageProperties.sizeInBytes;
	frameInfoStruct->polarImageType = _CameraProperties_Active.polarImageType;
	frameInfoStruct->copySize = imageProperties.sizeInBytes;
	frameInfoStruct->numberOfPlanes = 1;
	frameInfoStruct->isNewMROIFrame = 1;
	frameInfoStruct->totalScanAreas = 1;
	frameInfoStruct->scanAreaIndex = 0;
	frameInfoStruct->scanAreaID = 0;
	frameInfoStruct->isMROI = FALSE;
	frameInfoStruct->topInFullImage = 0;
	frameInfoStruct->leftInFullImage = 0;
	frameInfoStruct->mROIStripeFieldSize = 0;

	const bool isColorActive = TL_CAMERA_SENSOR_TYPE_BAYER == _cameraParams[_camID].cameraSensorType && 0 != _CameraProperties_Active.colorImageType;
	const bool isPolarActive = TL_CAMERA_SENSOR_TYPE_MONOCHROME_POLARIZED == _cameraParams[_camID].cameraSensorType && 0 != _CameraProperties_Active.polarImageType;

	if (!isColorActive && !isPolarActive)
	{
		frameInfoStruct->channels = 1;
	}

	//only copy when the size matches, leave blank on dropping frames
	if ((imageProperties.sizeInBytes != _expectedImageSizeFromCamera)) 
	{
		_previousLastImage = _lastImage;
		_lastImage = imageProperties.frameNumber;
		return FALSE;
	}

	unsigned long currentDMAIndex = (imageProperties.bufferIndex+ _CameraProperties_Active.dmaBufferCount)% _CameraProperties_Active.dmaBufferCount;
	long isDoAverage = (((ICamera::AVG_MODE_CUMULATIVE == _CameraProperties_Active.averageMode) && (1 < _CameraProperties_Active.averageNum)) && (0 < imageProperties.bufferIndex)) ? TRUE : FALSE;

	WaitForSingleObject(_hFrmBufHandle, INFINITE);

	//lock memory before process
	if (!_pFrmDllBuffer[currentDMAIndex].TryLockMem(Constants::TIMEOUT_MS))
	{
		ReleaseMutex(_hFrmBufHandle);
		return FALSE;
	}
	bool isCurrentDMAIndexLocked = true;

	unsigned short* imageProcessingInput = _pFrmDllBuffer[currentDMAIndex].GetMem();

	// TODO: should probably skip averaging if AE is running
	if (isDoAverage)
	{
		if (isColorActive || isPolarActive)
		{
			// Color & Polar processing algorithms cannot use the same buffer for input and output so we need the intermediate buffer
			imageProcessingInput = _intermediateBuffer;
			
			if (_intermediateBuffer == nullptr)
			{
				LogMessage(L"Intermediate buffer was null for averaging", ERROR_EVENT);
				_pFrmDllBuffer[currentDMAIndex].UnlockMem();
				ReleaseMutex(_hFrmBufHandle);
				return FALSE;
			}
		}
		else
		{
			// Mono can work within the destination buffer directly to avoid another copy later.
			imageProcessingInput = dst; 
		}

		// copy the memory to our processing input buffer. we can unlock the data after this because we've copied it out.
		SAFE_MEMCPY(imageProcessingInput, imageProperties.sizeInBytes, _pFrmDllBuffer[currentDMAIndex].GetMem());
		_pFrmDllBuffer[currentDMAIndex].UnlockMem();
		isCurrentDMAIndexLocked = false;

		unsigned long previousDMAIndex = (imageProperties.bufferIndex - 1 + _CameraProperties_Active.dmaBufferCount) % _CameraProperties_Active.dmaBufferCount;

		if (SW_FREE_RUN_MODE == (TriggerMode)(_CameraProperties_Active.triggerMode))
		{
			for (unsigned long avgID = 0; avgID < min(static_cast<unsigned long>(_CameraProperties_Active.averageNum), imageProperties.bufferIndex); avgID++)
			{
				previousDMAIndex = (imageProperties.bufferIndex - 1 - avgID + _CameraProperties_Active.dmaBufferCount) % _CameraProperties_Active.dmaBufferCount;
				ProcessAverageFrame(imageProcessingInput, previousDMAIndex);
			}
		}
		else
		{
			ProcessAverageFrame(imageProcessingInput, previousDMAIndex);
		}
	}
	else
	{
		imageProcessingInput = _pFrmDllBuffer[currentDMAIndex].GetMem();
	}

	if (isColorActive)
	{
		ProcessColor(imageProcessingInput, dst, imageProperties, frameInfoStruct);
	}
	else if (isPolarActive)
	{
		ProcessPolar(imageProcessingInput, dst, imageProperties, frameInfoStruct);
	}
	else if (!isDoAverage) // averaging has already copied the data out
	{
		assert(isCurrentDMAIndexLocked == true);
		SAFE_MEMCPY(dst, imageProperties.sizeInBytes, imageProcessingInput);
	}
	// From here on, dst contains the processed image

	if (isCurrentDMAIndexLocked)
	{
		// make certain to unlock the memory now that we've copied it out
		_pFrmDllBuffer[currentDMAIndex].UnlockMem();
		isCurrentDMAIndexLocked = false;
	}

	ReleaseMutex(_hFrmBufHandle);

	//start image processing: flip, rotate
	USHORT* pLocal = _intermediateBuffer;
	IppiSize size;
	size.width = imageProperties.width;
	size.height = imageProperties.height;
	int stepSrc = size.width * sizeof(unsigned short);
	IppiRect roiSrc = {0, 0, size.width, size.height};

	const int numChannels = frameInfoStruct->channels;
	const int channelNumPixels = size.width * size.height;
	const int totalImageSize = channelNumPixels * numChannels * sizeof(unsigned short);

	//flip
	if (TRUE == _CameraProperties_Active.horizontalFlip || TRUE == _CameraProperties_Active.verticalFlip)
	{
		if(NULL == pLocal)	return FALSE;

		if (TRUE == _CameraProperties_Active.horizontalFlip && TRUE == _CameraProperties_Active.verticalFlip)
		{
			for (int i = 0; i < numChannels; ++i)
			{
				const int offset = i * channelNumPixels;
				ippiDll->ippiMirror_16u_C1R(dst + offset, stepSrc, pLocal + offset, stepSrc, size, ippAxsBoth);
			}
		}
		else if(TRUE == _CameraProperties_Active.horizontalFlip)
		{
			for (int i = 0; i < numChannels; ++i)
			{
				const int offset = i * channelNumPixels;
				ippiDll->ippiMirror_16u_C1R(dst + offset, stepSrc, pLocal + offset, stepSrc, size, ippAxsVertical);
			}
		}
		else
		{
			for (int i = 0; i < numChannels; ++i)
			{
				const int offset = i * channelNumPixels;
				ippiDll->ippiMirror_16u_C1R(dst + offset, stepSrc, pLocal + offset, stepSrc, size, ippAxsHorizontal);
			}
		}
		//update after flipped
		SAFE_MEMCPY(dst, totalImageSize, pLocal);
	}

	int width = imageProperties.width;
	int height = imageProperties.height;
	//rotate
	if (_CameraProperties_Active.imageAngle != 0)
	{
		if (NULL == pLocal)	return FALSE;

		switch (_CameraProperties_Active.imageAngle)
		{
		case 90:
		{
			
			int stepDst = imageProperties.height * sizeof(unsigned short);
			IppiRect  roiDst = { 0, 0, size.height, size.width };
			long angle = 90;
			long xOffset = 0;
			long yOffset = imageProperties.width - 1;
			for (int i = 0; i < numChannels; ++i)
			{
				const int offset = i * channelNumPixels;
				ippiDll->ippiRotate_16u_C1R(dst + offset, size, stepSrc, roiSrc, pLocal + offset, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			}
			SAFE_MEMCPY(dst, totalImageSize, pLocal);	//update after rotated
			width = imageProperties.height;
			height = imageProperties.width;
		}
		break;
		case 180:
		{
			int stepDst = imageProperties.width * sizeof(unsigned short);
			IppiRect  roiDst = { 0, 0, size.width, size.height };
			long angle = 180;
			long xOffset = imageProperties.width - 1;
			long yOffset = imageProperties.height - 1;
			for (int i = 0; i < numChannels; ++i)
			{
				const int offset = i * channelNumPixels;
				ippiDll->ippiRotate_16u_C1R(dst + offset, size, stepSrc, roiSrc, pLocal + offset, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			}
			SAFE_MEMCPY(dst, totalImageSize, pLocal);	//update after rotated
		}
		break;
		case 270:
		{
			int stepDst = imageProperties.height * sizeof(unsigned short);
			IppiRect  roiDst = { 0, 0, size.height, size.width };
			long angle = 270;
			long xOffset = imageProperties.height - 1;
			long yOffset = 0;
			for (int i = 0; i < numChannels; ++i)
			{
				const int offset = i * channelNumPixels;
				ippiDll->ippiRotate_16u_C1R(dst + offset, size, stepSrc, roiSrc, pLocal + offset, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
			}
			SAFE_MEMCPY(dst, totalImageSize, pLocal);	//update after rotated

			width = imageProperties.height;
			height = imageProperties.width;
		}
		break;
		}
	}
	
	frameInfoStruct->imageWidth = width;
	frameInfoStruct->imageHeight = height;
	frameInfoStruct->fullImageWidth = width;
	frameInfoStruct->fullImageHeight = height;
	//keep track of the frame number, and the previous frame number
	_previousLastImage = _lastImage;
	_lastImage = imageProperties.frameNumber;
	_availableFramesCnt = _imagePropertiesQueue.size();
	++_cameraParams[_camID].framesProcessedSinceStart;
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

void ThorCam::ApplyGreyWorldColorBalance(unsigned short* imageData, int xOrigin, int yOrigin, int width, int height, int imageWidth, float* redGain, float* greenGain, float* blueGain)
{
	float rSum = 0.0;
	float gSum = 0.0;
	float bSum = 0.0;
	
	struct bgrPixel
	{
		unsigned short blue;
		unsigned short green;
		unsigned short  red;
	};

	bgrPixel* bgrPixelData = reinterpret_cast<bgrPixel*>(imageData);

	for (int row = yOrigin; row < yOrigin + height; ++row)
	{
		for (int col = xOrigin; col < xOrigin + width; ++col)
		{
			bgrPixel* pixel = &bgrPixelData[(row * imageWidth) + col];
			rSum += pixel->red;
			gSum += pixel->green;
			bSum += pixel->blue;
		}
	}

	double luminosity = (0.2126 * rSum) + (0.7152 * gSum) + (0.0722 * bSum);

	if (rSum < 0.0000001)
	{
		*redGain = 1.0;
	}
	else
	{
		*redGain = luminosity / rSum;
	}

	if (gSum < 0.0000001)
	{
		*greenGain = 1.0;
	}
	else
	{
		*greenGain = luminosity / gSum;
	}

	if (bSum < 0.0000001)
	{
		*blueGain = 1.0;
	}
	else
	{
		*blueGain = luminosity / bSum;
	}

	//TODO: Max limits appear arbitrary
	*redGain = min(*redGain, 10.0f);
	*greenGain = min(*greenGain, 10.0f);
	*blueGain = min(*blueGain, 10.0f);
}


void ThorCam::ProcessColor(unsigned short* input, unsigned short* output, const ImageProperties& imageProperties, FrameInfoStruct* frameInfo)
{
	// if white balance is needed, transform image using the neutral white balancer first
	bool isWhiteBalance = _cameraParams[_camID].framesProcessedSinceStart % _CameraProperties_Active.whiteBalanceFrameCount == 0;
	if (_CameraProperties_Active.oneShotWhiteBalanceFlag || _CameraProperties_Active.isAutoWhiteBalanceEnabled && isWhiteBalance)
	{
		void* neutralColorProcessorHandle = _cameraParams[_camID].colorProcessorHandleForWhiteBalance;
		ThorTSIColorErrChk(
			tl_mono_to_color_transform_to_48( // transform to 3-channel 16-bit color image data
				neutralColorProcessorHandle,
				input, //input
				imageProperties.width,
				imageProperties.height,
				output //output
			), 1);

		// NOTE: these are for an optional ROI feature of white balancing in the future
		int xOrigin = 0;
		int yOrigin = 0;
		int width = frameInfo->imageWidth;
		int height = frameInfo->imageHeight;

		float redGain = 1.0, greenGain = 1.0, blueGain = 1.0;
		int fullImageWIdth = frameInfo->imageWidth;

		ApplyGreyWorldColorBalance(output, 0, 0, width, height, fullImageWIdth, &redGain, &greenGain, &blueGain);

		ThorTSIColorErrChk(tl_mono_to_color_set_red_gain(_cameraParams[_camID].colorProcessorHandle, redGain), 1);
		ThorTSIColorErrChk(tl_mono_to_color_set_green_gain(_cameraParams[_camID].colorProcessorHandle, greenGain), 1);
		ThorTSIColorErrChk(tl_mono_to_color_set_blue_gain(_cameraParams[_camID].colorProcessorHandle, blueGain), 1);
		ThorTSIColorErrChk(tl_mono_to_color_get_red_gain(_cameraParams[_camID].colorProcessorHandle, &redGain), 1);
		ThorTSIColorErrChk(tl_mono_to_color_get_green_gain(_cameraParams[_camID].colorProcessorHandle, &greenGain), 1);
		ThorTSIColorErrChk(tl_mono_to_color_get_blue_gain(_cameraParams[_camID].colorProcessorHandle, &blueGain), 1);
		_CameraProperties_Active.redGain = redGain;
		_CameraProperties_Active.greenGain = greenGain;
		_CameraProperties_Active.blueGain = blueGain;
		_CameraProperties_UI.redGain = redGain;
		_CameraProperties_UI.greenGain = greenGain;
		_CameraProperties_UI.blueGain = blueGain;

		_CameraProperties_Active.oneShotWhiteBalanceFlag = false;
		_CameraProperties_UI.oneShotWhiteBalanceFlag = false;
		_cameraParams->resyncFlag = true; // another way to sync without syncing everything?
	}
	// run color processing with output pointing towards pDataBuffer
	void* colorProcessorHandle = _cameraParams[_camID].colorProcessorHandle;
	ThorTSIColorErrChk(
		tl_mono_to_color_transform_to_48( // transform to 3-channel 16-bit color image data
			colorProcessorHandle,
			input, //input
			imageProperties.width,
			imageProperties.height,
			output //output
		), 1);

	frameInfo->channels = 3;
	frameInfo->copySize = imageProperties.sizeInBytes * frameInfo->channels;
}

void ThorCam::ProcessPolar(unsigned short* input, unsigned short* output, const ImageProperties& imageProperties, FrameInfoStruct* frameInfo)
{
	unsigned short maxValue = (1 << _CameraProperties_Active.bitPerPixel) - 1;
	frameInfo->channels = 1; // all polar image types are technically 1-channel
	switch (_CameraProperties_Active.polarImageType)
	{
	case(1):
	{
		ThorTSIPolarErrChk(tl_polarization_processor_transform(
			_cameraParams[_camID].polarProcessorHandle,
			_cameraParams[_camID].cameraPolarPhase,
			input, // input
			_CameraProperties_Active.roiLeft, // ROI is used to adjust origin polar phase
			_CameraProperties_Active.roiTop,
			imageProperties.width,
			imageProperties.height,
			_CameraProperties_Active.bitPerPixel,
			maxValue,
			nullptr,
			output, // Intensity
			nullptr,
			nullptr,
			nullptr,
			nullptr), 1);
		break;
	}
	case(2):
	{
		ThorTSIPolarErrChk(tl_polarization_processor_transform(
			_cameraParams[_camID].polarProcessorHandle,
			_cameraParams[_camID].cameraPolarPhase,
			input, // input
			_CameraProperties_Active.roiLeft, // ROI is used to adjust origin polar phase
			_CameraProperties_Active.roiTop,
			imageProperties.width,
			imageProperties.height,
			_CameraProperties_Active.bitPerPixel,
			maxValue,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			output), 1); // DoLP
		break;
	}
	case(3):
	{
		ThorTSIPolarErrChk(tl_polarization_processor_transform(
			_cameraParams[_camID].polarProcessorHandle,
			_cameraParams[_camID].cameraPolarPhase,
			input, // input
			_CameraProperties_Active.roiLeft, // ROI is used to adjust origin polar phase
			_CameraProperties_Active.roiTop,
			imageProperties.width,
			imageProperties.height,
			_CameraProperties_Active.bitPerPixel,
			maxValue,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			output, // Azimuth
			nullptr), 1);
		break;
	}
	case(4):
	{
		//Quadview
		if (nullptr == input || nullptr == output)
		{
			LogMessage(L"Destination or source was NULL during polar quadview calculation.", ERROR_EVENT);
			break;
		}
		static const TL_POLARIZATION_PROCESSOR_POLAR_PHASE polarPhaseMap[2][2] = { TL_POLARIZATION_PROCESSOR_POLAR_PHASE_0_DEGREES, TL_POLARIZATION_PROCESSOR_POLAR_PHASE_45_DEGREES, TL_POLARIZATION_PROCESSOR_POLAR_PHASE_135_DEGREES, TL_POLARIZATION_PROCESSOR_POLAR_PHASE_90_DEGREES };
		int xIdx = _CameraProperties_Active.roiLeft;
		int yIdx = _CameraProperties_Active.roiTop;
		switch (_cameraParams[_camID].cameraPolarPhase)
		{
		case(TL_POLARIZATION_PROCESSOR_POLAR_PHASE_45_DEGREES):
			yIdx++;
			break;
		case(TL_POLARIZATION_PROCESSOR_POLAR_PHASE_135_DEGREES):
			xIdx++;
			break;
		case(TL_POLARIZATION_PROCESSOR_POLAR_PHASE_90_DEGREES):
			xIdx++;
			yIdx++;
			break;
		}
		xIdx %= 2;
		yIdx %= 2;
		TL_POLARIZATION_PROCESSOR_POLAR_PHASE startingPolarPhase = polarPhaseMap[xIdx][yIdx];

		pair<int, int> offset0 = { 0, 0 }; // .first == x, .second == y
		pair<int, int> offset45 = { 0, 0 };
		pair<int, int> offset135 = { 0, 0 };
		pair<int, int> offset90 = { 0, 0 };
		switch (startingPolarPhase)
		{
		case(TL_POLARIZATION_PROCESSOR_POLAR_PHASE_0_DEGREES):
		{
			offset0 = { 0, 0 };
			offset45 = { 0, 1 };
			offset135 = { 1, 0 };
			offset90 = { 1, 1 };
			break;
		}
		case(TL_POLARIZATION_PROCESSOR_POLAR_PHASE_45_DEGREES):
		{
			offset0 = { 0, 1 };
			offset45 = { 0, 0 };
			offset135 = { 1, 1 };
			offset90 = { 0, 1 };
			break;
		}
		case(TL_POLARIZATION_PROCESSOR_POLAR_PHASE_135_DEGREES):
		{
			offset0 = { 1, 0 };
			offset45 = { 1, 1 };
			offset135 = { 0, 0 };
			offset90 = { 0, 1 };
			break;
		}
		case(TL_POLARIZATION_PROCESSOR_POLAR_PHASE_90_DEGREES):
		{
			offset0 = { 1, 1 };
			offset45 = { 1, 0 };
			offset135 = { 0, 1 };
			offset90 = { 0, 0 };
			break;
		}
		}
		// TODO: all the information up to until this point is available before acquisition starts. Could save this at start and re-use it here

		int width = imageProperties.width;
		int height = imageProperties.height;
		int halfWidth = imageProperties.width / 2;
		int halfHeight = imageProperties.height / 2;

		for (int y = 0; y < halfHeight; y++)
		{
			for (int x = 0; x < halfWidth; x++)
			{
				// 0 degrees (top left)
				output[x + (y * width)] = input[(x * 2 + offset0.first) + (((y * 2) + offset0.second) * width)];
				// 45 degrees (top right)
				output[(x + halfWidth) + (y * width)] = input[(x * 2 + offset45.first) + (((y * 2) + offset45.second) * width)];
				// -45 degrees (bottom left)
				output[x + ((y + halfHeight) * width)] = input[(x * 2 + offset135.first) + (((y * 2) + offset135.second) * width)];
				// 90 degrees (bottom right)
				output[(x + halfWidth) + ((y + halfHeight) * width)] = input[(x * 2 + offset90.first) + (((y * 2) + offset90.second) * width)];
			}
		}
		break;
	}
	default:
	{
		LogMessage(L"Unknown polar image type was selected.", ERROR_EVENT);
		break;
	}
	}
}

long ThorCam::SetROI(int left, int right, int top, int bottom)
{
	if (_cameraParams[_camID].isCameraRunning) return FALSE; // we can't do anything to ROI if the camera is armed

	_CameraProperties_Active.roiLeft = left;
	_CameraProperties_Active.roiRight = right;
	_CameraProperties_Active.roiTop = top;
	_CameraProperties_Active.roiBottom = bottom;

	ThorTSIErrChk(tl_camera_set_roi(_cameraParams[_camID].cameraHandle, _CameraProperties_Active.roiLeft, _CameraProperties_Active.roiTop, _CameraProperties_Active.roiRight, _CameraProperties_Active.roiBottom), 1);

	//IMG WIDTH / HEIGHT
	int imageWidth = 0;
	int imageHeight = 0;
	ThorTSIErrChk(tl_camera_get_image_width(_cameraParams[_camID].cameraHandle, &imageWidth), 1);
	ThorTSIErrChk(tl_camera_get_image_height(_cameraParams[_camID].cameraHandle, &imageHeight), 1);
	if (imageWidth != _CameraProperties_Active.widthPx)
	{
		_CameraProperties_Active.widthPx = imageWidth;
		_cameraParams[_camID].resyncFlag = true;
	}
	if (imageWidth != _CameraProperties_Active.heightPx)
	{
		_CameraProperties_Active.heightPx = imageHeight;
		_cameraParams[_camID].resyncFlag = true;
	}
	//ROI
	int roi_x1 = 0;
	int roi_y1 = 0;
	int roi_x2 = 0;
	int roi_y2 = 0;
	ThorTSIErrChk(tl_camera_get_roi(_cameraParams[_camID].cameraHandle, &roi_x1, &roi_y1, &roi_x2, &roi_y2), 1);
	if (_CameraProperties_Active.roiLeft != roi_x1)
	{
		_CameraProperties_Active.roiLeft = roi_x1;
		_cameraParams[_camID].resyncFlag = true;
	}
	if (_CameraProperties_Active.roiTop != roi_y1)
	{
		_CameraProperties_Active.roiTop = roi_y1;
		_cameraParams[_camID].resyncFlag = true;
	}
	if (_CameraProperties_Active.roiRight != roi_x2)
	{
		_CameraProperties_Active.roiRight = roi_x2;
		_cameraParams[_camID].resyncFlag = true;
	}
	if (_CameraProperties_Active.roiBottom != roi_y2)
	{
		_CameraProperties_Active.roiBottom = roi_y2;
		_cameraParams[_camID].resyncFlag = true;
	}

	return TRUE;
}

