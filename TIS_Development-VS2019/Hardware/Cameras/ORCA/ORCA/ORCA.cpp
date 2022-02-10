// ThorTSI_CS.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "ORCA.h"

using namespace std;

unique_ptr<IPPIDll> ippiDll(new IPPIDll(L".\\ippiu8-7.0.dll"));

ORCA::ORCA() :
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
		_hdcam[i] = NULL;
	}
	_errMsg[0] = 0;

	_ImgPty_SetSettings.exposureTime_us = 30u * Constants::MS_TO_SEC;
	_ImgPty_SetSettings.triggerMode = ICamera::TriggerMode::SW_FREE_RUN_MODE;
	_ImgPty_SetSettings.triggerPolarity = DCAMPROP_TRIGGERPOLARITY__POSITIVE;
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
	_ImgPty_SetSettings.numFrame = 1;
	_ImgPty_SetSettings.dmaBufferCount = 256;
	_ImgPty_SetSettings.verticalFlip = FALSE;
	_ImgPty_SetSettings.horizontalFlip = FALSE;
	_ImgPty_SetSettings.imageAngle = 0;
	_ImgPty_SetSettings.hotPixelEnabled = 0;
	_ImgPty_SetSettings.hotPixelThreshold = 0;
	_ImgPty_SetSettings.gain = 0;
	_ImgPty_SetSettings.blackLevel = 0;
	_ImgPty_SetSettings.binIndex = 0;
	_ImgPty_SetSettings.hotPixelLevelIndex = 0;
	_ImgPty_SetSettings.masterPulseEnabled = FALSE;
	_ImgPty_SetSettings.staticFrameRateVal = 30;

	_pDetectorName = NULL;
	_pSerialNumber = NULL;
	_intermediateBuffer = NULL;
	_lastDMABufferCount = 0;
	_forceSettingsUpdate = FALSE;
	_readInitialValues = FALSE;
	_paramHotPixelAvailable = TRUE;
	_frameNumberLiveImage = 0;
	_availableFramesCnt = 0;
	_timeoutMS = 30000;

	_ImgPty = ImgPty();
	_ImgPty_Pre = ImgPty();
	_binRange[0] = 0;
	_binRange[1] = 0;
	_blackLevelRange[0] = 0;
	_blackLevelRange[1] = 0;
	_expUSRange[0] = 0;
	_expUSRange[1] = 0;
	_frmPerTriggerRange[0] = 0;
	_frmPerTriggerRange[1] = 0;
	_gainRange[0] = 0;
	_gainRange[1] = 0;
	_hbinRange[0] = 0;
	_hbinRange[1] = 0;
	_heightRange[0] = 0;
	_heightRange[1] = 0;
	_hotPixelRange[0] = 0;
	_hotPixelRange[1] = 0;
	_lastImage = 0;
	_pFrames = NULL;
	_previousLastImage = 0;
	_readOutSpeedRange[0] = 0;
	_readOutSpeedRange[1] = 0;
	_singleBinning = FALSE;
	_vbinRange[0] = 0;
	_vbinRange[1] = 0;
	_widthRange[0] = 0;
	_widthRange[1] = 0;
	_xRangeL[0] = 0;
	_xRangeL[1] = 0;
	_xRangeR[0] = 0;
	_xRangeR[1] = 0;
	_yRangeB[0] = 0;
	_yRangeB[1] = 0;
	_yRangeT[0] = 0;
	_yRangeT[1] = 0;
}

///Initialize Static Members
HANDLE ORCA::_hFrameAcqThread = NULL;
bool ORCA::_instanceFlag = false;
ImgPty ORCA::_imgPtyDll = ImgPty();
shared_ptr <ORCA> ORCA::_single(NULL);
bool ORCA::_sdkIsOpen = false;
ThreadSafeMem<USHORT> ORCA::_pFrmDllBuffer[MAX_DMABUFNUM];
unsigned long long ORCA::_bufferImageIndex = 0;
unsigned long ORCA::_expectedImageSize = 0;
unsigned long ORCA::_lastCopiedImageSize = 0;
unsigned long long ORCA::_frameCountOffset = 0;
unsigned long long ORCA::_copiedFrameNumber = 0;
long ORCA::_1stSet_Frame = 0;
ThreadSafeQueue<ImageProperties> ORCA::_imagePropertiesQueue;
std::string ORCA::_camSerial[MAX_CAM_NUM] = { "" };
std::string ORCA::_camName[MAX_CAM_NUM] = { "" };
std::string ORCA::_cameraInterfaceType[MAX_CAM_NUM] = { "" };
long ORCA::_numCameras = 0;
long ORCA::_camID = -1;
wchar_t ORCA::_errMsg[MSG_SIZE] = { NULL };
bool ORCA::_cameraRunning[MAX_CAM_NUM] = { false };
HANDLE ORCA::_hStopAcquisition = CreateEvent(NULL, true, false, NULL);  //2nd parameter "true" so it needs manual "Reset" after "Set (signal)" event
long ORCA::_maxFrameCountReached = FALSE;
HANDLE ORCA::_hFrmBufHandle = CreateMutex(NULL, false, NULL);
HANDLE ORCA::_hFrmBufReady = CreateEvent(NULL, true, false, NULL);
HDCAM ORCA::_hdcam[MAX_CAM_NUM] = { NULL };
HDCAMWAIT ORCA::_hwait = NULL;
DCAMWAIT_START	ORCA::_waitstart;
DCAMCAP_TRANSFERINFO ORCA::_captransferinfo;
std::atomic<long> ORCA::_frameReady = 0;
long ORCA::_statusError = FALSE;
long ORCA::_threadRunning = FALSE;

ORCA* ORCA::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ORCA());
		_instanceFlag = true;
	}
	return _single.get();
}

ORCA::~ORCA()
{
	_instanceFlag = false;
}

void ORCA::ClearAllCameras()
{
	DCAMERR err;
	_camID = -1;

	if (_sdkIsOpen)
	{
		for (int i = 0; i < _numCameras; i++)
		{
			DeSelectCamera(i);
		}

		//close sdk
		OrcaErrChk(L"dcamapi_uninit", err = dcamapi_uninit());
		if (failed(err))
		{
			MessageBox(NULL, L"Failed to close Hamamatsu DCAM SDK", L"DCAM SDK Close error", MB_OK);
		}
		_sdkIsOpen = false;
	}
}

void ORCA::ClearMem()
{
	for (int i = 0; i < MAX_DMABUFNUM; i++)
	{
		_pFrmDllBuffer[i].ReleaseMem();
	}
	_lastCopiedImageSize = 0;
	_lastDMABufferCount = 0;
	SAFE_DELETE_MEMORY(_intermediateBuffer);
}

long ORCA::DeSelectCamera(long cameraIndex)
{
	long ret = FALSE;
	DCAMERR	err;

	StopCamera(cameraIndex);

	if (IsOpen(cameraIndex))
	{
		_camSerial[cameraIndex].clear();
		_camName[cameraIndex].clear();
		// close device	
		OrcaErrChk(L"dcamdev_close", dcamdev_close(_hdcam[cameraIndex]));
		_hdcam[cameraIndex] = NULL;
		ret = TRUE;
	}
	return ret;
}

/*
get_dcamdev_string is from DCAM examples common.cpp. Request a string message from camera.
*/
inline const int ORCA::get_dcamdev_string(DCAMERR& err, HDCAM hdcam, int32 idStr, char* text, int32 textbytes)
{
	DCAMDEV_STRING param;
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.text = text;
	param.textbytes = textbytes;
	param.iString = idStr;

	OrcaErrChk(L"dcamdev_getstring", err = dcamdev_getstring(hdcam, &param));
	return (FALSE == failed(err));
}

/*
Get the model, serial number, and bus type from the nth camera.
*/
void ORCA::GetCameraInfo(HDCAM hdcam, long index)
{
	char model[256];
	char cameraid[64];
	char bus[64];

	DCAMERR	err;
	if (get_dcamdev_string(err, hdcam, DCAM_IDSTR_MODEL, model, sizeof(model)))
	{
		_camName[index] = model;
	}
	if (get_dcamdev_string(err, hdcam, DCAM_IDSTR_CAMERAID, cameraid, sizeof(cameraid)))
	{
		_camSerial[index] = cameraid;
	}
	if (get_dcamdev_string(err, hdcam, DCAM_IDSTR_BUS, bus, sizeof(bus)))
	{
		_cameraInterfaceType[index] = bus;
	}
}

/*
This Could be useful for future implementation. If we want to show device specifics like driver version and fw version.
// show HDCAM camera information by text.
void dcamcon_show_dcamdev_info_detail( HDCAM hdcam )
{
char	buf[ 256 ];

DCAMERR	err;
if( ! my_dcamdev_string( err, hdcam, DCAM_IDSTR_VENDOR, buf, sizeof(buf) ) )
dcamcon_show_dcamerr( hdcam, err, "dcamdev_getstring(DCAM_IDSTR_VENDOR)\n" );
else
printf( "DCAM_IDSTR_VENDOR         = %s\n", buf );

if( ! my_dcamdev_string( err, hdcam, DCAM_IDSTR_MODEL, buf, sizeof(buf) ) )
dcamcon_show_dcamerr( hdcam, err, "dcamdev_getstring(DCAM_IDSTR_MODEL)\n" );
else
printf( "DCAM_IDSTR_MODEL          = %s\n", buf );

if( ! my_dcamdev_string( err, hdcam, DCAM_IDSTR_CAMERAID, buf, sizeof(buf) ) )
dcamcon_show_dcamerr( hdcam, err, "dcamdev_getstring(DCAM_IDSTR_CAMERAID)\n" );
else
printf( "DCAM_IDSTR_CAMERAID       = %s\n", buf );

if( ! my_dcamdev_string( err, hdcam, DCAM_IDSTR_BUS, buf, sizeof(buf) ) )
dcamcon_show_dcamerr( hdcam, err, "dcamdev_getstring(DCAM_IDSTR_BUS)\n" );
else
printf( "DCAM_IDSTR_BUS            = %s\n", buf );


if( ! my_dcamdev_string( err, hdcam, DCAM_IDSTR_CAMERAVERSION, buf, sizeof(buf) ) )
dcamcon_show_dcamerr( hdcam, err, "dcamdev_getstring(DCAM_IDSTR_CAMERAVERSION)\n" );
else
printf( "DCAM_IDSTR_CAMERAVERSION  = %s\n", buf );

if( ! my_dcamdev_string( err, hdcam, DCAM_IDSTR_DRIVERVERSION, buf, sizeof(buf) ) )
dcamcon_show_dcamerr( hdcam, err, "dcamdev_getstring(DCAM_IDSTR_DRIVERVERSION)\n" );
else
printf( "DCAM_IDSTR_DRIVERVERSION  = %s\n", buf );

if( ! my_dcamdev_string( err, hdcam, DCAM_IDSTR_MODULEVERSION, buf, sizeof(buf) ) )
dcamcon_show_dcamerr( hdcam, err, "dcamdev_getstring(DCAM_IDSTR_MODULEVERSION)\n" );
else
printf( "DCAM_IDSTR_MODULEVERSION  = %s\n", buf );

if( ! my_dcamdev_string( err, hdcam, DCAM_IDSTR_DCAMAPIVERSION, buf, sizeof(buf) ) )
dcamcon_show_dcamerr( hdcam, err, "dcamdev_getstring(DCAM_IDSTR_DCAMAPIVERSION)\n" );
else
printf( "DCAM_IDSTR_DCAMAPIVERSION = %s\n", buf );
}
*/


//Initialize DCAM sdk, once initialized open all cameras and map their handle to _hdcam
void ORCA::FindAllCameras()
{
	DCAMERR err;

	try
	{
		//get all available cameras
		_numCameras = 0L;

		//open camera SDK
		if (!_sdkIsOpen)
		{
			// initialize DCAM-API
			DCAMAPI_INIT	apiinit;
			memset(&apiinit, 0, sizeof(apiinit));
			apiinit.size = sizeof(apiinit);

#if USE_INITOPTION
			// set option of initialization
			int32 initoption[] = {
				DCAMAPI_INITOPTION_APIVER__LATEST,
				DCAMAPI_INITOPTION_ENDMARK			// it is necessary to set as the last value.
			};

			apiinit.initoption = initoption;
			apiinit.initoptionbytes = sizeof(initoption);
#endif

#if USE_INITGUID
			// set GUID parameter
			DCAM_GUID	guid = DCAM_GUID_MYAPP;

			apiinit.guid = &guid;
#endif
			//This is the minimum best time that works in Yoav's system
			Sleep(350);

			OrcaErrChk(L"dcamapi_init", err = dcamapi_init(&apiinit));

			if (failed(err))
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ORCA DCAM SDK Init error: Failed to initialize Hamamatsu DCAM SDK. Make sure there is no other software using the camera.");
				LogMessage(_errMsg, ERROR_EVENT);
			}
			else
			{
				_sdkIsOpen = true;
				_numCameras = apiinit.iDeviceCount;

				for (long i = 0; i < _numCameras; i++)
				{
					// open device
					DCAMDEV_OPEN	devopen;
					memset(&devopen, 0, sizeof(devopen));
					devopen.size = sizeof(devopen);
					devopen.index = i;

					OrcaErrChk(L"dcamdev_open", err = dcamdev_open(&devopen));
					if (failed(err))
					{
						MessageBox(NULL, L"Failed to open Hamamatsu camera. Make sure there is no other software using the camera.", L"DCAM SDK Init error", MB_OK);
					}
					else
					{
						_hdcam[i] = devopen.hdcam;

						GetCameraInfo(_hdcam[i], i);
					}
				}
			}
		}
	}
	catch (exception e)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ORCA: FindAllCameras failed");
		LogMessage(_errMsg, ERROR_EVENT);
	}
}

long ORCA::GetAttributeFromCamera(HDCAM hdcam, long requestedProperty, DCAMPROP_ATTR& returnedAttr)
{
	DCAMERR err;

	DCAMPROP_ATTR	propattr;
	memset(&propattr, 0, sizeof(propattr));
	propattr.cbSize = sizeof(propattr);
	propattr.iProp = requestedProperty;

	OrcaErrChk(L"dcamprop_getattr", err = dcamprop_getattr(hdcam, &propattr));
	if (failed(err))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ORCA: Failed to request attributes from property: %dl", requestedProperty);
		LogMessage(_errMsg, ERROR_EVENT);
		return FALSE;
	}
	else
	{
		returnedAttr = propattr;
		return TRUE;
	}
}

void ORCA::InitialParamInfo(void)
{
	DCAMPROP_ATTR widthAttr, heightAttr, binningInfo, binningInfoHorz, binningInfoVert, exposureAttr, speedAttr;
	double pixelType = 0, rowBytes, width = 0, height = 0, exposure = 0, binning = 1;
	DCAMERR err;
	_paramHotPixelAvailable = TRUE;

	//Get Binning range
	if (TRUE == GetAttributeFromCamera(_hdcam[_camID], DCAM_IDPROP_BINNING, binningInfo))
	{
		_vbinRange[0] = _hbinRange[0] = _binRange[0] = (int32)binningInfo.valuemin;
		_vbinRange[1] = _hbinRange[1] = _binRange[1] = (int32)binningInfo.valuemax;
		_singleBinning = TRUE;
	}
	else
	{
		if (TRUE == GetAttributeFromCamera(_hdcam[_camID], DCAM_IDPROP_BINNING_HORZ, binningInfoHorz))
		{
			_hbinRange[0] = (int32)binningInfoHorz.valuemin;
			_hbinRange[1] = (int32)binningInfoHorz.valuemax;
			_singleBinning = FALSE;
		}
		if (TRUE == GetAttributeFromCamera(_hdcam[_camID], DCAM_IDPROP_BINNING_VERT, binningInfoVert))
		{
			_vbinRange[0] = (int32)binningInfoVert.valuemin;
			_vbinRange[1] = (int32)binningInfoVert.valuemax;
			_singleBinning = FALSE;
		}
	}

	if (TRUE == _singleBinning)
	{
		OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_BINNING, &binning));
		_ImgPty_SetSettings.roiBinY = _ImgPty_SetSettings.roiBinX = static_cast<long>(binning);
	}
	else
	{
		OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_BINNING_HORZ, &binning));
		_ImgPty_SetSettings.roiBinX = static_cast<long>(binning);
		OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_BINNING_VERT, &binning));
		_ImgPty_SetSettings.roiBinY = static_cast<long>(binning);
	}

	//Get Height and Width range
	if (TRUE == GetAttributeFromCamera(_hdcam[_camID], DCAM_IDPROP_IMAGE_WIDTH, widthAttr))
	{
		_widthRange[0] = (int32)widthAttr.valuemin;
		_widthRange[1] = (int32)widthAttr.valuemax;
	}
	if (TRUE == GetAttributeFromCamera(_hdcam[_camID], DCAM_IDPROP_IMAGE_HEIGHT, heightAttr))
	{
		_heightRange[0] = (int32)heightAttr.valuemin;
		_heightRange[1] = (int32)heightAttr.valuemax;
	}

	if (TRUE == GetAttributeFromCamera(_hdcam[_camID], DCAM_IDPROP_READOUTSPEED, speedAttr))
	{
		_readOutSpeedRange[0] = (int32)speedAttr.valuemin;
		_readOutSpeedRange[1] = (int32)speedAttr.valuemax;
	}

	//Get Exposure range
	if (TRUE == GetAttributeFromCamera(_hdcam[_camID], DCAM_IDPROP_EXPOSURETIME, exposureAttr))
	{
		_expUSRange[0] = static_cast<long long>(ceil(exposureAttr.valuemin * US_TO_SEC));
		_expUSRange[1] = static_cast<long long>(floor(exposureAttr.valuemax * US_TO_SEC));
	}

	OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_IMAGE_WIDTH, &width));
	_ImgPty_SetSettings.widthPx = static_cast<int>(width);
	_xRangeL[0] = _xRangeR[0] = 0;
	_xRangeL[1] = _xRangeR[1] = static_cast<int>(width) * _ImgPty_SetSettings.roiBinX - 1;

	OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_IMAGE_HEIGHT, &height));
	_ImgPty_SetSettings.heightPx = static_cast<int>(height);
	_yRangeB[0] = _yRangeT[0] = 0;
	_yRangeB[1] = _yRangeT[1] = static_cast<int>(height) * _ImgPty_SetSettings.roiBinY - 1;

	OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_IMAGEDETECTOR_PIXELWIDTH, &_ImgPty_SetSettings.pixelSizeXUM));
	OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_IMAGEDETECTOR_PIXELHEIGHT, &_ImgPty_SetSettings.pixelSizeYUM));
	OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_IMAGE_PIXELTYPE, &pixelType));
	OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_IMAGE_ROWBYTES, &rowBytes));
	OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_EXPOSURETIME, &exposure));
	_ImgPty_SetSettings.exposureTime_us = static_cast<int>(exposure * US_TO_SEC);

	switch ((DCAM_PIXELTYPE)static_cast<long>(pixelType))
	{
	case DCAM_PIXELTYPE::DCAM_PIXELTYPE_MONO8:
		_ImgPty_SetSettings.bitPerPixel = 8;
		break;
	case DCAM_PIXELTYPE::DCAM_PIXELTYPE_MONO12:
		_ImgPty_SetSettings.bitPerPixel = 12;
		break;
	case DCAM_PIXELTYPE::DCAM_PIXELTYPE_MONO16:
		_ImgPty_SetSettings.bitPerPixel = 16;
		break;
	default:
		_ImgPty_SetSettings.bitPerPixel = 16;
	}

	double hotPixelCorrection = DCAMPROP_HOTPIXELCORRECT_LEVEL__STANDARD;
	OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_HOTPIXELCORRECT_LEVEL, &hotPixelCorrection));
	if (failed(err))
	{
		_paramHotPixelAvailable = FALSE;
	}
	_ImgPty_SetSettings.hotPixelLevelIndex = static_cast<int>(hotPixelCorrection - 1);

	//Set default external trigger output configuration, positive polarity and the kind to Programable
	OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE));
	OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_OUTPUTTRIGGER_KIND, DCAMPROP_OUTPUTTRIGGER_KIND__PROGRAMABLE));
}

bool ORCA::IsOpen(const long cameraIndex)
{
	//return if no camera found
	if (_numCameras == 0)
		return FALSE;
	//return if invalid selection
	if ((0 > cameraIndex) || (_numCameras <= cameraIndex))
		return FALSE;
	//return if camera is not available
	if (NULL == _hdcam[cameraIndex])
		return FALSE;

	return TRUE;
}

void ORCA::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

/// <summary> function to do buffer average with previous acquired frame </summary>
void ORCA::ProcessAverageFrame(unsigned short* dst, unsigned long previousDMAIndex)
{
	unsigned long imageSize = _imgPtyDll.channel * ((_imgPtyDll.roiRight - _imgPtyDll.roiLeft + 1) / _imgPtyDll.roiBinX) * ((_imgPtyDll.roiBottom - _imgPtyDll.roiTop + 1) / _imgPtyDll.roiBinY);
	unsigned long sizeInBytes = imageSize * sizeof(unsigned short);

	if (!_pFrmDllBuffer[previousDMAIndex].TryLockMem(Constants::TIMEOUT_MS))
	{
		return;
	}
	USHORT* pCur = dst;
	USHORT* pPre = _pFrmDllBuffer[previousDMAIndex].GetMem();	//previous frame
	for (unsigned long i = 0; i < imageSize; i++)
	{
		*pCur = ((*pCur) * (_imgPtyDll.averageNum - 1) + (*pPre)) / _imgPtyDll.averageNum;
		pCur++;
		pPre++;
	}
	//output with averaged buffer
	_pFrmDllBuffer[previousDMAIndex].UnlockMem();
}

long ORCA::SetBdDMA(ImgPty* pImgPty)
{
	DCAMERR err;
	long ret = TRUE;

	//only access set the parameters when the camera is selected
	if (!IsOpen(_camID))
		return FALSE;
	try
	{
		//Stop the camera, clear any errors, set the image callback function to null, and clear any pending images
		StopCamera(_camID);

		while (NULL != _hFrameAcqThread || TRUE == _threadRunning)
		{
			Sleep(10);
		}

		//In case the pixel size changed while we were waiting for the frame acquisition thread to finish, set the roi size again
		_ImgPty.roiBottom = _ImgPty_SetSettings.roiBottom;
		_ImgPty.roiLeft = _ImgPty_SetSettings.roiLeft;
		_ImgPty.roiRight = _ImgPty_SetSettings.roiRight;
		_ImgPty.roiTop = _ImgPty_SetSettings.roiTop;
		_ImgPty.widthPx = _ImgPty_SetSettings.widthPx;
		_ImgPty.heightPx = _ImgPty_SetSettings.heightPx;

		//reset available frame count, no more copy frames for last session
		_availableFramesCnt = _bufferImageIndex = 0;
		_imagePropertiesQueue.clear();

		//reset the indexes of the last image and the previous last image.
		//these are used to ensure there are no dropped frames
		_lastImage = 0;
		_previousLastImage = -1;

		//reset max frame count reached flag
		ORCA::_maxFrameCountReached = FALSE;

		//Set new Cam parameters
		_imgPtyDll = *pImgPty;

		//Need to set the speed to default first
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_READOUTSPEED, 0));

		//Need to set the binning to 1 before setting the image area size. Because the camera max area size changes internally with the binning
		OrcaErrChk(L"dcamprop_getvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_BINNING, DCAMPROP_BINNING__1));

		//////****Set/Get Camera Parameters****//////
		//send the parameters to the camera, check if they were set successfuly, log if there was an error
		//Set the speed of the camera
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_READOUTSPEED, _imgPtyDll.readOutSpeedIndex));

		//Set the trigger polarity to positive and set the trigger mode to start, which is equivalent to External Start trigger mode in HCImageLive
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERPOLARITY, _imgPtyDll.triggerPolarity));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGER_MODE, DCAMPROP_TRIGGER_MODE__START));

		//Set the sub array mode on to enable the use of Top, Left, Right and Bottom
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_SUBARRAYMODE, DCAMPROP_MODE__ON));

		//We need to set the subarray values twice for live mode, in order to scan in the correct zone
		double width = 1 + static_cast<double>(_imgPtyDll.roiRight) - static_cast<double>(_imgPtyDll.roiLeft);
		double height = 1 + static_cast<double>(_imgPtyDll.roiBottom) - static_cast<double>(_imgPtyDll.roiTop);
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_SUBARRAYHPOS, _imgPtyDll.roiLeft));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_SUBARRAYHSIZE, width));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_SUBARRAYHPOS, _imgPtyDll.roiLeft));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_SUBARRAYHSIZE, width));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_SUBARRAYVPOS, _imgPtyDll.roiTop));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_SUBARRAYVSIZE, height));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_SUBARRAYVPOS, _imgPtyDll.roiTop));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_SUBARRAYVSIZE, height));

		//Set the binning in the camera when the camera is not imagings, also set the binning after setting the image size with binning of 1
		if (TRUE == _singleBinning)
		{
			switch (_imgPtyDll.binIndex)
			{
			case 0:
			{
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_BINNING, DCAMPROP_BINNING__1));
			}
			break;
			case 1:
			{
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_BINNING, DCAMPROP_BINNING__2));
			}
			break;
			case 2:
			{
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_BINNING, DCAMPROP_BINNING__4));
			}
			break;
			default:
			{
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_BINNING, DCAMPROP_BINNING__1));
			}
			}
		}

		double frameRate = 0;
		OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_INTERNALFRAMERATE, &frameRate));
		if (_imgPtyDll.masterPulseEnabled)
		{
			frameRate = _imgPtyDll.staticFrameRateVal;
		}

		//////****End Set/Get Camera Parameters****//////

		//Only average frame when the averageMode is set
		long avgNum = (ICamera::AVG_MODE_NONE == _imgPtyDll.averageMode) ? 1 : _imgPtyDll.averageNum;

		//set the trigger mode and total number of frames,
		//since we are doing average after capture
		switch ((TriggerMode)(_imgPtyDll.triggerMode))
		{
		case SW_SINGLE_FRAME:
		{
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__INTERNAL));
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERACTIVE, DCAMPROP_TRIGGERACTIVE__EDGE));
			if (_imgPtyDll.masterPulseEnabled)
			{
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__MASTERPULSE));
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_MASTERPULSE_TRIGGERSOURCE, DCAMPROP_MASTERPULSE_TRIGGERSOURCE__SOFTWARE));
			}

			_imgPtyDll.numFrame = avgNum;
			_imgPtyDll.numImagesToBuffer = MIN_IMAGE_BUFFERS;
			_imgPtyDll.dmaBufferCount = avgNum;
		}
		break;
		case SW_MULTI_FRAME:
		{
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__INTERNAL));
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERACTIVE, DCAMPROP_TRIGGERACTIVE__EDGE));
			if (_imgPtyDll.masterPulseEnabled)
			{
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__MASTERPULSE));
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_MASTERPULSE_TRIGGERSOURCE, DCAMPROP_MASTERPULSE_TRIGGERSOURCE__SOFTWARE));
			}

			_imgPtyDll.numFrame = _imgPtyDll.numFrame * avgNum;
			_imgPtyDll.numImagesToBuffer = DEFAULT_IMAGE_BUFFERS;
			_imgPtyDll.dmaBufferCount = min(4096, static_cast<int>(frameRate * 10)); //Make the DMA buffer 10 times the size of the frame rate without exceeding 4096
		}
		break;
		case SW_FREE_RUN_MODE:
		{
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__INTERNAL));
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERACTIVE, DCAMPROP_TRIGGERACTIVE__EDGE));
			if (_imgPtyDll.masterPulseEnabled)
			{
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__MASTERPULSE));
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_MASTERPULSE_TRIGGERSOURCE, DCAMPROP_MASTERPULSE_TRIGGERSOURCE__SOFTWARE));
			}

			_imgPtyDll.numFrame = 0;
			_imgPtyDll.numImagesToBuffer = MIN_IMAGE_BUFFERS;
			_imgPtyDll.dmaBufferCount = avgNum;
		}
		break;
		case HW_SINGLE_FRAME:
		{
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__INTERNAL));
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERACTIVE, DCAMPROP_TRIGGERACTIVE__EDGE));
			if (_imgPtyDll.masterPulseEnabled)
			{
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__MASTERPULSE));
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_MASTERPULSE_TRIGGERSOURCE, DCAMPROP_MASTERPULSE_TRIGGERSOURCE__SOFTWARE));
			}

			_imgPtyDll.numFrame = avgNum;
			_imgPtyDll.numImagesToBuffer = MIN_IMAGE_BUFFERS;
			_imgPtyDll.dmaBufferCount = avgNum;
		}
		break;
		case HW_MULTI_FRAME_TRIGGER_FIRST:
		case HW_MULTI_FRAME_TRIGGER_EACH:
		{
			if (_imgPtyDll.masterPulseEnabled)
			{
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__MASTERPULSE));
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_MASTERPULSE_TRIGGERSOURCE, DCAMPROP_MASTERPULSE_TRIGGERSOURCE__EXTERNAL));
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_MASTERPULSE_MODE, DCAMPROP_MASTERPULSE_MODE__START));
			}
			else
			{
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__EXTERNAL));
				OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERACTIVE, DCAMPROP_TRIGGERACTIVE__EDGE));
			}
			_imgPtyDll.numFrame = _imgPtyDll.numFrame * avgNum;
			_imgPtyDll.numImagesToBuffer = DEFAULT_IMAGE_BUFFERS;
			_imgPtyDll.dmaBufferCount = min(4096, static_cast<int>(frameRate * 10)); //Make the DMA buffer 10 times the size of the frame rate without exceeding 4096
		}
		break;
		case HW_MULTI_FRAME_TRIGGER_EACH_BULB:
		{
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERSOURCE, DCAMPROP_TRIGGERSOURCE__EXTERNAL));
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERACTIVE, DCAMPROP_TRIGGERACTIVE__LEVEL));
			_imgPtyDll.numFrame = _imgPtyDll.numFrame * avgNum;
			_imgPtyDll.numImagesToBuffer = DEFAULT_IMAGE_BUFFERS;
			_imgPtyDll.dmaBufferCount = min(4096, static_cast<int>(frameRate * 10)); //Make the DMA buffer 10 times the size of the frame rate without exceeding 4096
		}
		break;
		}

		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_OUTPUTTRIGGER_KIND, DCAMPROP_OUTPUTTRIGGER_KIND__EXPOSURE));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_OUTPUTTRIGGER_SOURCE, DCAMPROP_OUTPUTTRIGGER_SOURCE__EXPOSURE));

		//We need to program frame trigger out because the camera doesn't have a default digital line like that
		//To program the output frame trigger, set the kind to programable, and the source to readout end
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_OUTPUTTRIGGER_POLARITY + DCAM_IDPROP__OUTPUTTRIGGER, DCAMPROP_OUTPUTTRIGGER_POLARITY__POSITIVE));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_OUTPUTTRIGGER_KIND + DCAM_IDPROP__OUTPUTTRIGGER, DCAMPROP_OUTPUTTRIGGER_KIND__PROGRAMABLE));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_OUTPUTTRIGGER_SOURCE + DCAM_IDPROP__OUTPUTTRIGGER, DCAMPROP_OUTPUTTRIGGER_SOURCE__EXPOSURE));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_OUTPUTTRIGGER_PROGRAMABLESTART + DCAM_IDPROP__OUTPUTTRIGGER, DCAMPROP_OUTPUTTRIGGER_PROGRAMABLESTART__FIRSTREADOUT));
		OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_OUTPUTTRIGGER_PERIOD + DCAM_IDPROP__OUTPUTTRIGGER, ((double)_ImgPty_SetSettings.exposureTime_us / (double)US_TO_SEC)));

		//Set up the masterpulse
		if (_imgPtyDll.masterPulseEnabled && 0 != _imgPtyDll.staticFrameRateVal)
		{
			double internalLineInterval, masterPulseInterval, frameInterval;
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_MASTERPULSE_INTERVAL, (1.0 / _imgPtyDll.staticFrameRateVal)));
			OrcaErrChk(L"dcamprop_setvalue", dcamprop_setvalue(_hdcam[_camID], DCAM_IDPROP_TRIGGERACTIVE, DCAMPROP_TRIGGERACTIVE__SYNCREADOUT));
			OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_INTERNAL_LINEINTERVAL, &internalLineInterval));
			OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_INTERNAL_FRAMEINTERVAL, &frameInterval));
			OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_MASTERPULSE_INTERVAL, &masterPulseInterval));
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ORCA lineInterval: %f frameInterval: %f pixelY: %d masterPulseInterval: %f", internalLineInterval, frameInterval, _imgPtyDll.roiBottom - _imgPtyDll.roiTop, masterPulseInterval);
			LogMessage(_errMsg, INFORMATION_EVENT);
		}

		///------------------------------------------------------------------------------------///
		// The comments that have step numbers are all required steps in DCAM for image acquisition. 
		// For more information see access_image.cpp and live_average.cpp
		//STEP 1:
		// open wait handle
		DCAMWAIT_OPEN	waitopen;
		memset(&waitopen, 0, sizeof(waitopen));
		waitopen.size = sizeof(waitopen);
		waitopen.hdcam = _hdcam[_camID];

		OrcaErrChk(L"dcamwait_open", err = dcamwait_open(&waitopen));

		if (failed(err))
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ORCA SetBdDMA unable to open wait handle");
			LogMessage(_errMsg, ERROR_EVENT);
			ret = FALSE;
		}
		else
		{
			_hwait = waitopen.hwait;

			//STEP 2:
			// allocate buffer
			double	getBufframebytes;
			OrcaErrChk(L"dcamprop_getvalue", err = dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_BUFFER_FRAMEBYTES, &getBufframebytes));
			if (failed(err))
			{
				ret = FALSE;
			}
			else
			{

				int32 bufframebytes = static_cast<int32>(getBufframebytes);
				int	number_of_buffer = _imgPtyDll.numImagesToBuffer;

				err = dcambuf_alloc(_hdcam[_camID], number_of_buffer);
				if (failed(err))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"ORCA SetBdDMA unable to allocate dcambuf_alloc, buffer size: %d, buffer frames: %d, number of frames: %d", bufframebytes, number_of_buffer, _imgPtyDll.numFrame);
					LogMessage(_errMsg, ERROR_EVENT);
					ret = FALSE;
				}
			}
		}

		//In the future use chan for multi-channel cameras
		_imgPtyDll.channel = 1;

		//keep the expected image size to make sure we don't over step the boundaries of the allocated memory
		//when copying to the buffer
		_expectedImageSize = _imgPtyDll.channel * (_imgPtyDll.widthPx) * (_imgPtyDll.heightPx) * sizeof(unsigned short);

		if ((_lastCopiedImageSize != _expectedImageSize) || (0 == _lastCopiedImageSize) || (_lastDMABufferCount != _imgPtyDll.dmaBufferCount))
		{
			for (int k = 0; k < _imgPtyDll.dmaBufferCount; ++k)
			{
				if (FALSE == _pFrmDllBuffer[k].SetMem(_expectedImageSize))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"Orca SetBdDMA unable to allocate pFrmDllBuffer #(%d), size(%d)", k, _expectedImageSize);
					LogMessage(_errMsg, ERROR_EVENT);
					return FALSE;
				}
			}
			_lastDMABufferCount = _imgPtyDll.dmaBufferCount;
		}

		//use intermediate if need flip, rotate or average:
		if ((TRUE == _imgPtyDll.horizontalFlip || TRUE == _imgPtyDll.verticalFlip) || (0 != _imgPtyDll.imageAngle) ||
			((AverageMode::AVG_MODE_CUMULATIVE == _imgPtyDll.averageMode) && (1 < _imgPtyDll.averageNum)))
		{
			USHORT* tmp;
			tmp = (USHORT*)realloc(_intermediateBuffer, _expectedImageSize);
			if (NULL != tmp)
			{
				_intermediateBuffer = tmp;
			}
		}
		else
		{
			SAFE_DELETE_MEMORY(_intermediateBuffer);
		}

		//de-signal Stop Acquisition Event at the end
		ResetEvent(_hStopAcquisition);
	}
	catch (...)
	{
		ret = FALSE;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"SetBdDMA failed");
		LogMessage(_errMsg, ERROR_EVENT);
	}
	return ret;
}

void ORCA::StopCamera(long cameraIndex)
{
	if (IsOpen(cameraIndex))
	{
		// stop capture
		dcamcap_stop(_hdcam[_camID]);
		SetEvent(_hStopAcquisition);
		_cameraRunning[cameraIndex] = false;

		// release buffer
		dcambuf_release(_hdcam[_camID]);
		//reset the copy buffer index here,
		//for camera to restart without preflight->setup:
		_bufferImageIndex = 0;
		// close wait handle
		if (NULL != _hwait)
		{
			dcamwait_close(_hwait);
			_hwait = NULL;
		}
	}
	SAFE_DELETE_HANDLE(_hFrameAcqThread);
}

/// ********************************************	Generic Functions	 ******************************************** ///

long ORCA::FindCameras(long& cameraCount)
{
	DCAMERR err;

	FindAllCameras();
	cameraCount = _numCameras;
	_readInitialValues = TRUE;

	//If the SDK was succefully initialized but no camera was found, uninitialize the SDK.
	if (0 >= cameraCount && _sdkIsOpen)
	{
		OrcaErrChk(L"dcamapi_uninit", err = dcamapi_uninit());
		if (failed(err))
		{
			MessageBox(NULL, L"Failed to close Hamamatsu DCAM SDK", L"DCAM SDK Close error", MB_OK);
		}
		_sdkIsOpen = false;
		return FALSE;
	}
	return TRUE;
}

long ORCA::SelectCamera(const long camera)
{
	//validate selection
	if (!IsOpen(camera))
		return FALSE;

	//if the camera is open successfully, then change the selectedCam index _camID to the new one
	_camID = camera;

	//Only read the camera initial values if the SDK was just initialized and connected to the camera
	if (TRUE == _readInitialValues)
	{
		InitialParamInfo();
		_readInitialValues = FALSE;
	}

	SAFE_DELETE_ARRAY(_pDetectorName);
	std::wstring wCamName = StringToWString(_camName[_camID]);
	_pDetectorName = new wchar_t[wCamName.length() + 1];
	SAFE_MEMCPY(_pDetectorName, (wCamName.length() + 1) * sizeof(wchar_t), wCamName.c_str());

	SAFE_DELETE_ARRAY(_pSerialNumber);
	std::wstring snWideString = StringToWString(_camSerial[_camID]);
	_pSerialNumber = new wchar_t[snWideString.length() + 1];
	SAFE_MEMCPY(_pSerialNumber, (snWideString.length() + 1) * sizeof(wchar_t), snWideString.c_str());

	return TRUE;
}

long ORCA::TeardownCamera()
{
	ClearAllCameras();
	ClearMem();
	return TRUE;
}

long ORCA::PreflightAcquisition(char* pData)
{
	long ret = TRUE;

	//copy the new settings
	_ImgPty.exposureTime_us = _ImgPty_SetSettings.exposureTime_us;
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
	_ImgPty.widthPx = _ImgPty_SetSettings.widthPx;
	_ImgPty.heightPx = _ImgPty_SetSettings.heightPx;
	_ImgPty.binIndex = _ImgPty_SetSettings.binIndex;
	_ImgPty.hotPixelLevelIndex = _ImgPty_SetSettings.hotPixelLevelIndex;
	_ImgPty.masterPulseEnabled = _ImgPty_SetSettings.masterPulseEnabled;
	_ImgPty.staticFrameRateVal = _ImgPty_SetSettings.staticFrameRateVal;

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
		_ImgPty_Pre.binIndex = _ImgPty.binIndex;
		_ImgPty_Pre.hotPixelLevelIndex = _ImgPty.hotPixelLevelIndex;
		_ImgPty_Pre.masterPulseEnabled = _ImgPty.masterPulseEnabled;
		_ImgPty_Pre.staticFrameRateVal = _ImgPty.staticFrameRateVal;
	}
	else
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"SetBdDMA failed");
		LogMessage(_errMsg, ERROR_EVENT);
		ret = FALSE;
	}

	return ret;
}

long ORCA::SetupAcquisition(char* pData)
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
		(_ImgPty_Pre.widthPx != _ImgPty_SetSettings.widthPx) ||
		(_ImgPty_Pre.heightPx != _ImgPty_SetSettings.heightPx) ||
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
		(_ImgPty_Pre.hotPixelThreshold != _ImgPty_SetSettings.hotPixelThreshold) ||
		(_ImgPty_Pre.gain != _ImgPty_SetSettings.gain) ||
		(_ImgPty_Pre.blackLevel != _ImgPty_SetSettings.blackLevel) ||
		(_ImgPty_Pre.binIndex != _ImgPty_SetSettings.binIndex) ||
		(_ImgPty_Pre.hotPixelLevelIndex != _ImgPty_SetSettings.hotPixelLevelIndex) ||
		(_ImgPty_Pre.masterPulseEnabled != _ImgPty_SetSettings.masterPulseEnabled) ||
		(_ImgPty_Pre.staticFrameRateVal != _ImgPty_SetSettings.staticFrameRateVal) ||
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
		_ImgPty.widthPx = _ImgPty_SetSettings.widthPx;
		_ImgPty.heightPx = _ImgPty_SetSettings.heightPx;
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
		_ImgPty.binIndex = _ImgPty_SetSettings.binIndex;
		_ImgPty.hotPixelLevelIndex = _ImgPty_SetSettings.hotPixelLevelIndex;
		_ImgPty.masterPulseEnabled = _ImgPty_SetSettings.masterPulseEnabled;
		_ImgPty.staticFrameRateVal = _ImgPty_SetSettings.staticFrameRateVal;

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
			_ImgPty_Pre.binIndex = _ImgPty.binIndex;
			_ImgPty_Pre.hotPixelLevelIndex = _ImgPty.hotPixelLevelIndex;
			_ImgPty_Pre.masterPulseEnabled = _ImgPty.masterPulseEnabled;
			_ImgPty_Pre.staticFrameRateVal = _ImgPty.staticFrameRateVal;
		}
		else
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"SetBdDMA failed");
			LogMessage(_errMsg, ERROR_EVENT);
			ret = FALSE;
		}
	}

	return ret;
}

long ORCA::StartAcquisition(char* pDataBuffer)
{
	DCAMERR err;
	DWORD frameAcq;
	long ret = TRUE;

	if (!IsOpen(_camID)) return FALSE;

	//Only start the camera once after stopping it
	if (_cameraRunning[_camID]) return TRUE;

	try
	{
		_frameCountOffset = 0;
		_1stSet_Frame = 0;
		_cameraRunning[_camID] = true;
		_statusError = FALSE;
		_frameReady = 0;
		_copiedFrameNumber = 0;

		//STEP 3:
		// start capture
		OrcaErrChk(L"dcamcap_start", err = dcamcap_start(_hdcam[_camID], DCAMCAP_START_SEQUENCE));
		if (failed(err))
		{
			ret = FALSE;
		}

		//STEP 4:
		//Set WaitStart param 
		memset(&_waitstart, 0, sizeof(_waitstart));
		_waitstart.size = sizeof(_waitstart);
		_waitstart.eventmask = DCAMWAIT_CAPEVENT_FRAMEREADY;
		_waitstart.timeout = _timeoutMS;

		//Set transferinfo param
		memset(&_captransferinfo, 0, sizeof(_captransferinfo));
		_captransferinfo.size = sizeof(_captransferinfo);

		//If set to HW trigger mode, allow some time for the camera to be ready to receive the trigger
		//:TODO: check if the 200ms sleep is still required
		switch (_imgPtyDll.triggerMode)
		{
		case ICamera::HW_SINGLE_FRAME:
		case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
		case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		case ICamera::HW_TDI_TRIGGER_MODE:
			Sleep(200);
			break;
		default:
			//OrcaErrChk(L"dcamcap_firetrigger", dcamcap_firetrigger(_hdcam[_camID]));
			//OrcaErrChk(L"tl_camera_issue_software_trigger", tl_camera_issue_software_trigger(_camera[_camID]), 1);
			break;
		}

		SAFE_DELETE_HANDLE(_hFrameAcqThread);
		//Start the status request thread once the hardware info and stage parameters have been queried 
		_hFrameAcqThread = FrameAcqThread(frameAcq);
	}
	catch (...)
	{
		ret = FALSE;
		_cameraRunning[_camID] = false;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"StartAcquisition failed");
		LogMessage(_errMsg, ERROR_EVENT);
	}
	return ret;
}

HANDLE ORCA::FrameAcqThread(DWORD& threadID)
{
	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) & (ORCA::ReadFramesFromCamera), (void*)this, 0, &threadID);
	if (0 != handle)
	{
		SetThreadPriority(handle, THREAD_PRIORITY_ABOVE_NORMAL);
	}
	return handle;
}

void ORCA::ReadFramesFromCamera(void* instance)
{
	DCAMERR err;
	DCAMBUF_FRAME bufframe;
	wchar_t errMessage[MSG_SIZE];
	double rowBytes = 0, imgWidth = 0, imgHeight = 0;
	long currFrame = 0, droppedFrames = 0, temp = 0;
	_threadRunning = TRUE;

	OrcaErrChk(L"dcamprop_getvalue", err = dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_BUFFER_ROWBYTES, &rowBytes));
	OrcaErrChk(L"dcamprop_getvalue", err = dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_IMAGE_WIDTH, &imgWidth));
	OrcaErrChk(L"dcamprop_getvalue", err = dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_IMAGE_HEIGHT, &imgHeight));

	while (_cameraRunning[_camID] && (currFrame < _imgPtyDll.numFrame || ICamera::SW_FREE_RUN_MODE == ORCA::_imgPtyDll.triggerMode))
	{
		//Step 5:
		// wait for an image
		OrcaErrChk(L"dcamwait_start", err = dcamwait_start(_hwait, &_waitstart));
		if (failed(err))
		{
			if (err == DCAMERR_ABORT)
			{
				// receive abort signal
				_statusError = TRUE;
				StringCbPrintfW(errMessage, MSG_SIZE, L"ORCA ReadFramesFromCamera: Received abort signal");
				LogMessage(errMessage, ERROR_EVENT);
				break;
			}
			else
			{
				_statusError = TRUE;

				//Get the error message
				DCAMDEV_STRING	param;
				DCAMERR errId = err;
				char errtext[256];
				memset(&param, 0, sizeof(param));
				param.size = sizeof(param);
				param.text = errtext;
				param.textbytes = sizeof(errtext);
				param.iString = err;
				OrcaErrChk(L"dcamdev_getstring", dcamdev_getstring(_hdcam[_camID], &param));

				StringCbPrintfW(errMessage, MSG_SIZE, L"ORCA ReadFramesFromCamera: Cannot start, unkown error err = (DCAMERR)0x%08X", errId);
				LogMessage(errMessage, ERROR_EVENT);
				break;
			}
		}

		OrcaErrChk(L"dcamcap_transferinfo", dcamcap_transferinfo(_hdcam[_camID], &_captransferinfo));

		if (_captransferinfo.nFrameCount < 0)
		{
			_statusError = TRUE;
			StringCbPrintfW(errMessage, MSG_SIZE, L"ORCA ReadFramesFromCamera: Camera could not acquire any frames, _captransferinfo.nFrameCount = %d", _captransferinfo.nFrameCount);
			LogMessage(errMessage, ERROR_EVENT);
			break;
		}

		if (0 != _captransferinfo.nFrameCount && _captransferinfo.nFrameCount - _copiedFrameNumber >= _imgPtyDll.dmaBufferCount && ICamera::SW_FREE_RUN_MODE != ORCA::_imgPtyDll.triggerMode)
		{
			_statusError = TRUE;
			StringCbPrintfW(errMessage, MSG_SIZE, L"ORCA: DMA buffer not big enough. Try dissabling image preview, or closing other applications. DMABuffer size: %d", _imgPtyDll.dmaBufferCount);
			LogMessage(errMessage, ERROR_EVENT);
			break;
		}

		//If the camera dropped a frame, print it to the log and let the user know which and how many frames were dropped
		if (ICamera::SW_FREE_RUN_MODE != ORCA::_imgPtyDll.triggerMode && _captransferinfo.nFrameCount > currFrame + 1 + droppedFrames)
		{
			temp = _captransferinfo.nFrameCount - (currFrame + 1 + droppedFrames);
			droppedFrames += temp;
			StringCbPrintfW(errMessage, MSG_SIZE, L"ORCA ReadFramesFromCamera: dropped %d frames, at frame number %d", temp, currFrame + 1);
			LogMessage(errMessage, ERROR_EVENT);
		}

		if (0 < _captransferinfo.size)
		{
			unsigned long currentDMAIndex = (currFrame + _imgPtyDll.dmaBufferCount) % _imgPtyDll.dmaBufferCount;

			//lock memory before process
			if (!_pFrmDllBuffer[currentDMAIndex].TryLockMem(Constants::TIMEOUT_MS))
			{
				_statusError = TRUE;
				StringCbPrintfW(errMessage, MSG_SIZE, L"ORCA ReadFramesFromCamera: Failed to lock buffer");
				LogMessage(errMessage, ERROR_EVENT);
				break;
			}

			//Step 6:
			// prepare frame param
			memset(&bufframe, 0, sizeof(bufframe));
			bufframe.size = sizeof(bufframe);
			bufframe.iFrame = _captransferinfo.nNewestFrameIndex;

			// set user buffer information and copied ROI
			bufframe.buf = _pFrmDllBuffer[currentDMAIndex].GetMem();
			bufframe.rowbytes = static_cast<int32>(rowBytes);
			bufframe.left = 0;
			bufframe.top = 0;
			bufframe.width = static_cast<long>(imgWidth);
			bufframe.height = static_cast<long>(imgHeight);

			err = dcambuf_copyframe(_hdcam[_camID], &bufframe);
			if (failed(err))
			{
				_statusError = TRUE;
				StringCbPrintfW(errMessage, MSG_SIZE, L"ORCA ReadFramesFromCamera: Failed to copy frame dcambuf_copyframe");
				LogMessage(errMessage, ERROR_EVENT);
				//unlock current after copy
				_pFrmDllBuffer[currentDMAIndex].UnlockMem();
				break;
			}
			else
			{
				//Successfully copied a frame to DMA buffer, increase the number of frames ready to be copied
				currFrame++;
				_frameReady++;
				StringCbPrintfW(errMessage, MSG_SIZE, L"ORCA: frameNumber = %d, nNewestFrame = %d, currentDMAIndex = %d _frameReady = %d size = %d curr = %d", _captransferinfo.nFrameCount, _captransferinfo.nNewestFrameIndex, currentDMAIndex, _frameReady.load(), _captransferinfo.size, currFrame);
				LogMessage(errMessage, INFORMATION_EVENT);
			}

			//unlock current after copy
			_pFrmDllBuffer[currentDMAIndex].UnlockMem();
		}
	}
	if (ICamera::SW_FREE_RUN_MODE != ORCA::_imgPtyDll.triggerMode)
	{
		StringCbPrintfW(errMessage, MSG_SIZE, L"ORCA ReadFramesFromCamera: Camera dropped %d frames", droppedFrames);
		LogMessage(errMessage, ERROR_EVENT);
	}
	_threadRunning = FALSE;
}

long ORCA::StatusAcquisition(long& status)
{
	// terminate when an error is found in the frame acquisition thread
	if (TRUE == _statusError)
	{
		_single->getInstance()->PostflightAcquisition(NULL);
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ORCA STATUS ERROR: Frame dropped _lastImage = %d, queueSize = %d, nFrameCount = %d, nNewestFrame = %d", _lastImage, _captransferinfo.size, _captransferinfo.nFrameCount, _captransferinfo.nNewestFrameIndex);
		LogMessage(_errMsg, ERROR_EVENT);
		status = ICamera::STATUS_ERROR;
		return FALSE;
	}

	if (_frameReady > 0 || WAIT_OBJECT_0 == WaitForSingleObject(_hStopAcquisition, 0))
	{
		status = ICamera::STATUS_READY;
	}
	else
	{
		status = ICamera::STATUS_BUSY;
	}

	return TRUE;
}

long ORCA::StatusAcquisitionEx(long& status, long& indexOfLastCompletedFrame)
{
	return TRUE;
}

long ORCA::CopyAcquisition(char* pDataBuffer, void* frameInfo)
{
	DCAMERR err;
	ImageProperties imageProperties;
	unsigned short* dst = (unsigned short*)pDataBuffer;

	//TODO: Use the imageProperties queue to handle frame information, to keep frame count instead of _frameReady

	if (_captransferinfo.nFrameCount < 0)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ORCA: CopyAcquisition No captured image");
		LogMessage(_errMsg, ERROR_EVENT);
		return FALSE;
	}

	double rowBytes, imgWidth, imgHeight, getBufframebytes, top, left;
	OrcaErrChk(L"dcamprop_getvalue", err = dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_BUFFER_FRAMEBYTES, &getBufframebytes));
	OrcaErrChk(L"dcamprop_getvalue", err = dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_BUFFER_ROWBYTES, &rowBytes));
	OrcaErrChk(L"dcamprop_getvalue", err = dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_IMAGE_WIDTH, &imgWidth));
	OrcaErrChk(L"dcamprop_getvalue", err = dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_IMAGE_HEIGHT, &imgHeight));
	OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_SUBARRAYHPOS, &left));
	OrcaErrChk(L"dcamprop_getvalue", dcamprop_getvalue(_hdcam[_camID], DCAM_IDPROP_SUBARRAYVPOS, &top));
	imageProperties.bufferIndex = _captransferinfo.nNewestFrameIndex;
	imageProperties.frameNumber = _captransferinfo.nFrameCount;
	imageProperties.height = static_cast<long>(imgHeight);
	imageProperties.width = static_cast<long>(imgWidth);
	imageProperties.sizeInBytes = static_cast<long>(getBufframebytes);

	_expectedImageSize = _imgPtyDll.channel * (_imgPtyDll.widthPx) * (_imgPtyDll.heightPx) * sizeof(unsigned short);

	//only copy when the size matches, leave blank on dropping frames
	if (imageProperties.sizeInBytes != _expectedImageSize)
	{
		_previousLastImage = _lastImage;
		_lastImage = _captransferinfo.nNewestFrameIndex;
		return FALSE;
	}

	unsigned long currentDMAIndex = (_copiedFrameNumber + _imgPtyDll.dmaBufferCount) % _imgPtyDll.dmaBufferCount;
	long average = (((ICamera::AVG_MODE_CUMULATIVE == _imgPtyDll.averageMode) && (1 < _imgPtyDll.averageNum)) && (0 < _copiedFrameNumber)) ? TRUE : FALSE;

	//lock memory before process
	if (!_pFrmDllBuffer[currentDMAIndex].TryLockMem(Constants::TIMEOUT_MS))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ORCA:CopyAcquisition could not lock memory at currentDMAIndex %d", currentDMAIndex);
		LogMessage(_errMsg, ERROR_EVENT);
		return FALSE;
	}

	//copy current to output buffer and process on output directly
	SAFE_MEMCPY(dst, _expectedImageSize, _pFrmDllBuffer[currentDMAIndex].GetMem());

	//unlock current after copy
	_pFrmDllBuffer[currentDMAIndex].UnlockMem();

	_copiedFrameNumber++;
	_frameReady--;
	_availableFramesCnt = static_cast<unsigned long long>(_frameReady);

	StringCbPrintfW(_errMsg, MSG_SIZE, L"ORCA:CopyAcquisition currentDMAIndex = %ul, _copiedFrameNumber = %I64u, dmaSize = %d, frameReady: %d _availableFramesCnt %I64u", currentDMAIndex, _copiedFrameNumber, _imgPtyDll.dmaBufferCount, _frameReady.load(), _availableFramesCnt);
	LogMessage(_errMsg, INFORMATION_EVENT);

	//average before flip/rotate, but no average first frame; use intermediate buffer
	if ((average) && (NULL != _intermediateBuffer))
	{
		unsigned long previousDMAIndex = (_copiedFrameNumber - 1 + _imgPtyDll.dmaBufferCount) % _imgPtyDll.dmaBufferCount;
		if (SW_FREE_RUN_MODE == (TriggerMode)(_imgPtyDll.triggerMode))
		{
			//In live mode use the _frameNumberLiveImage
			previousDMAIndex = (_frameNumberLiveImage - 1 + _imgPtyDll.dmaBufferCount) % _imgPtyDll.dmaBufferCount;
			for (unsigned long avgID = 0; avgID < min(static_cast<unsigned long>(_imgPtyDll.averageNum), _frameNumberLiveImage); avgID++)
			{
				previousDMAIndex = (_frameNumberLiveImage - 1 - avgID + _imgPtyDll.dmaBufferCount) % _imgPtyDll.dmaBufferCount;
				ProcessAverageFrame(dst, previousDMAIndex);
			}
			_frameNumberLiveImage++;
		}
		else
		{
			ProcessAverageFrame(dst, previousDMAIndex);
		}
	}

	//start image processing: flip, rotate
	USHORT* pLocal = _intermediateBuffer;
	IppiSize size;
	size.width = imageProperties.width;
	size.height = imageProperties.height;
	int stepSrc = size.width * sizeof(unsigned short);
	IppiRect roiSrc = { 0, 0, size.width, size.height };

	//flip
	if (TRUE == _imgPtyDll.horizontalFlip || TRUE == _imgPtyDll.verticalFlip)
	{
		if (NULL == pLocal)	return FALSE;

		if (TRUE == _imgPtyDll.horizontalFlip && TRUE == _imgPtyDll.verticalFlip)
		{
			ippiDll->ippiMirror_16u_C1R(dst, stepSrc, pLocal, stepSrc, size, ippAxsBoth);
		}
		else if (TRUE == _imgPtyDll.horizontalFlip)
		{
			ippiDll->ippiMirror_16u_C1R(dst, stepSrc, pLocal, stepSrc, size, ippAxsVertical);
		}
		else
		{
			ippiDll->ippiMirror_16u_C1R(dst, stepSrc, pLocal, stepSrc, size, ippAxsHorizontal);
		}
		//update after flipped
		SAFE_MEMCPY(dst, imageProperties.sizeInBytes, pLocal);
	}

	//rotate
	switch (_imgPtyDll.imageAngle)
	{
	case 90:
	{
		if (NULL == pLocal)	return FALSE;
		int stepDst = imageProperties.height * sizeof(unsigned short);
		IppiRect  roiDst = { 0, 0, size.height, size.width };
		long angle = 90;
		long xOffset = 0;
		long yOffset = imageProperties.width - 1;
		ippiDll->ippiRotate_16u_C1R(dst, size, stepSrc, roiSrc, pLocal, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
		SAFE_MEMCPY(dst, imageProperties.sizeInBytes, pLocal);	//update after rotated
	}
	break;
	case 180:
	{
		if (NULL == pLocal)	return FALSE;
		int stepDst = imageProperties.width * sizeof(unsigned short);
		IppiRect  roiDst = { 0, 0, size.width, size.height };
		long angle = 180;
		long xOffset = imageProperties.width - 1;
		long yOffset = imageProperties.height - 1;
		ippiDll->ippiRotate_16u_C1R(dst, size, stepSrc, roiSrc, pLocal, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
		SAFE_MEMCPY(dst, imageProperties.sizeInBytes, pLocal);	//update after rotated
	}
	break;
	case 270:
	{
		if (NULL == pLocal)	return FALSE;
		int stepDst = imageProperties.height * sizeof(unsigned short);
		IppiRect  roiDst = { 0, 0, size.height, size.width };
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
	_lastImage = _copiedFrameNumber;
	//_availableFramesCnt = _imagePropertiesQueue.size();
	return TRUE;
}

long ORCA::PostflightAcquisition(char* pDataBuffer)
{
	long ret = TRUE;
	try
	{
		//Stop the camera, set the image callback function to null
		for (long i = 0; i < _numCameras; ++i)
		{
			StopCamera(i);
		}
	}
	catch (...)
	{
		ret = FALSE;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"PostflightAcquisition failed");
		LogMessage(_errMsg, ERROR_EVENT);
	}
	return ret;
}
