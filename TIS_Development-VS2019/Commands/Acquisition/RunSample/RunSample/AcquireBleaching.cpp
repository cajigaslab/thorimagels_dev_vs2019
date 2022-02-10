#include "stdafx.h"
#include "..\..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "RunSample.h"
#include "AcquireBleaching.h"
#include "AcquireFactory.h"
#include "ImageCorrection.h"
#include "..\..\..\Common\HDF5IOdll.h"

extern auto_ptr<CommandDll> shwDll;
extern auto_ptr<TiffLibDll> tiffDll;
unique_ptr<HDF5ioDLL> h5io(new HDF5ioDLL(L".\\Modules_Native\\HDF5IO.dll"));

void GetLookUpTables(unsigned short * rlut, unsigned short * glut, unsigned short *blut,long red, long green, long blue, long bp, long wp, long bitdepth);
long SaveTIFFWithoutOME(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut, double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string *acquiredDate, double dt, long doCompression);
long SaveTIFF(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut, double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string *acquiredDate, double dt, string * omeTiffData, PhysicalSize physicalSize, long doCompression);
long SetDeviceParameterValue(IDevice *pDevice,long paramID, double val,long bWait,HANDLE hEvent,long waitTime);
string ConvertWStringToString(wstring ws);

AcquireBleaching::AcquireBleaching(IExperiment *exp,wstring path)
{
	_pExp = exp;
	_counter = 0;
	_tFrame = 1;
	_path = path;
	_lastImageUpdateTime = 0;
	_stopStatus = 0;
	_zstageStepSize = 0;
	_capturedImageID = 0;
	_pTemp = NULL;
	_digiShutterEnabled = FALSE;
}

UINT BleachThreadProc( LPVOID pParam )
{
	ICamera * pBScanner = (ICamera *)pParam;

	char * pMemoryBuffer = NULL;

	if(NULL != AcquireBleaching::hEventPostBleachSetupComplete)
	{
		WaitForSingleObject(AcquireBleaching::hEventPostBleachSetupComplete,TIMEOUT_MS);
	}
	//user asked to stop bleach:
	if(WaitForSingleObject(AcquireBleaching::hStopBleach,0) == WAIT_OBJECT_0)
	{
		SetEvent(AcquireBleaching::hEventBleach);
		return 0;
	}	

	if(FALSE == pBScanner->StartAcquisition(pMemoryBuffer))
	{
		pBScanner->PostflightAcquisition(NULL);
		SetEvent(AcquireBleaching::hEventBleach);
		return 0;
	}

	long status = ICamera::STATUS_BUSY;

	DWORD frameStartTime = GetTickCount();

	while(status == ICamera::STATUS_BUSY)
	{
		if((WaitForSingleObject(AcquireBleaching::hStopBleach,0) == WAIT_OBJECT_0) || (FALSE == pBScanner->StatusAcquisition(status)))
		{
			break;
		}
	}

	SetEvent(AcquireBleaching::hEventBleach);
	return 0;
}

UINT CaptureThreadProc( LPVOID pParam )
{
	AcquireBleaching::CaptureParams* cParam = (AcquireBleaching::CaptureParams*)pParam;
	AcquireBleaching::captureStatus = AcquireBleaching::CaptureWrap(cParam->acqB, cParam->pCam,cParam->currentT,cParam->streaming,cParam->sp.totFrames,cParam->timeInterval,cParam->d,&cParam->sp,cParam->simultaneous);
	if(NULL != AcquireBleaching::hEventCapture)	
	{
		SetEvent(AcquireBleaching::hEventCapture);
	}
	return 0;
}

UINT StatusZThreadProc7( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;
	IDevice * pDevice = (IDevice*)pParam;
	while(status == IDevice::STATUS_BUSY)
	{
		if(FALSE == pDevice->StatusPosition(status))
			break;
	}
	SetEvent( AcquireBleaching::hEventZ);
	return 0;
}

bool FindAllFilesInFolder(wstring wpath)
{
	//vector<wstring> filesInFolder;
	bool found = false;
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	wchar_t rawPath[_MAX_PATH];

	_wsplitpath_s(wpath.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
	StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s%s",drive,dir,L"SLMWaveforms\\*.raw");

	WIN32_FIND_DATA fd; 
	HANDLE hFind = ::FindFirstFile(rawPath, &fd); 
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
			{
				StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s%s%s",drive,dir,L"SLMWaveforms\\",fd.cFileName);
				//filesInFolder.push_back(rawPath);
				found = true;
				break;
			}
		}while(::FindNextFile(hFind, &fd));
	}

	//std::sort(filesInFolder.begin(), filesInFolder.end());

	//return (0 < filesInFolder.size()) ? true : false;
	return found;
}

///******************	static members		******************///
HANDLE AcquireBleaching::hEvent = NULL;
HANDLE AcquireBleaching::hEventZ = NULL;
HANDLE AcquireBleaching::hEventBleach = NULL;
HANDLE AcquireBleaching::hStopBleach = NULL;
HANDLE AcquireBleaching::hEventPostBleachSetupComplete = NULL;
HANDLE AcquireBleaching::hEventStopLoad = NULL;
CRITICAL_SECTION AcquireBleaching::loadAccess;
HANDLE AcquireBleaching::hEventCapture = NULL;
HANDLE AcquireBleaching::hStopCapture = NULL;
char* AcquireBleaching::_pTemp = NULL;
BOOL AcquireBleaching::_evenOdd = FALSE;
double AcquireBleaching::_lastGoodFocusPosition = 0.0;

GGalvoWaveformParams* AcquireBleaching::bleachParams[MAX_BLEACH_PARAMS_CNT] = {NULL};
char * AcquireBleaching::bleachMem = NULL;
streampos AcquireBleaching::bleachMemSize = 0;
BOOL AcquireBleaching::captureStatus = TRUE;
long lastImageID = 0;
shared_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML());

///******************	end static members	******************///

long AcquireBleaching::CallCaptureComplete(long captureComplete)
{
	return TRUE;
}

long AcquireBleaching::CallCaptureImage(long index)
{
	CaptureImage(index);
	return TRUE;
}

long AcquireBleaching::CallSaveImage(long index, BOOL isImageUpdate)
{
	SaveImage(index, isImageUpdate);
	return TRUE;
}

long AcquireBleaching::CallCaptureSubImage(long index)
{
	CaptureSubImage(index);
	return TRUE;
}

long AcquireBleaching::CallSaveSubImage(long index)
{
	SaveSubImage(index);
	return TRUE;
}

long AcquireBleaching::CallSaveZImage(long index, double power0, double power1, double power2, double power3, double power4, double power5)
{
	SaveZImage(index, power0, power1, power2, power3, power4, power5);
	return TRUE;
}

long AcquireBleaching::CallSaveTImage(long index)
{
	SaveTImage(index);
	return TRUE;
}

long AcquireBleaching::CallSequenceStepCurrent(long index)
{
	SequenceStepCurrent(index);
	return TRUE;
}

long AcquireBleaching::PreCaptureEventCheck(long &status)
{
	PreCapture(status);
	return TRUE;
}

long AcquireBleaching::StopCaptureEventCheck(long &status)
{
	StopCapture(status);
	if((1 == status) && (NULL != hStopCapture))
	{
		SetEvent(hStopCapture);
	}
	return TRUE;
}

long AcquireBleaching::CallStartProgressBar(long index, long resetTotalCount)
{
	StartProgressBar(index, resetTotalCount);
	return TRUE;
}

long AcquireBleaching::CallInformMessage(wchar_t* message)
{
	InformMessage(message);
	return TRUE;
}

long AcquireBleaching::CallNotifySavedFileIPC(wchar_t* message)
{
	NotifySavedFileIPC(message);
	return TRUE;
}

long AcquireBleaching::Execute(long index, long subWell, long zFrame, long tFrame)
{
	AcquireBleaching::_lastImageUpdateTime = 0;
	AcquireBleaching::captureStatus = TRUE;
	_tFrame = tFrame;
	_capturedImageID = 0;
	return Execute(index, subWell);
}

long AcquireBleaching::SetupCameraParams(ICamera * pCamera,long &bufferChannels, long &width, long &height, double &umPerPixel, long &displayImage, long triggerMode)
{
	long streamEnable,streamFrames,rawData,streamTriggerMode,storageMode,zFastEnable,zFastMode,flybackFrames,flybackLines,previewIndex,stimulusTriggering,dmaFrames,stimulusMaxFrames,useReferenceVoltageForFastZPockels;
	double flybackTimeAdjustMS,volumeTimeAdjustMS,stepTimeAdjustMS;
	long streamDisplayCumulativeAveragePreview = FALSE;
	_pExp->GetStreaming(streamEnable,streamFrames,rawData,streamTriggerMode,displayImage,storageMode,zFastEnable,zFastMode,flybackFrames,flybackLines,flybackTimeAdjustMS,volumeTimeAdjustMS,stepTimeAdjustMS,previewIndex,stimulusTriggering,dmaFrames,stimulusMaxFrames,useReferenceVoltageForFastZPockels, streamDisplayCumulativeAveragePreview);

	ICamera::CameraType cameraType;
	double val;
	pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,val);
	cameraType = (ICamera::CameraType)static_cast<long> (val);

	long zstageSteps, timePoints,triggerModeTimelapse,zEnable;
	string zstageName;
	double zstageStepSize, intervalSec, zStartPos;
	long zStreamFrames,zStreamMode;
	_pExp->GetZStage(zstageName,zEnable,zstageSteps,zstageStepSize,zStartPos,zStreamFrames,zStreamMode);
	_pExp->GetTimelapse(timePoints,intervalSec,triggerModeTimelapse);

	_zstageStepSize = zstageStepSize;
	//post flight the camera before the setup to ensure all tasks are properly stopped
	//before starting new ones
	pCamera->PostflightAcquisition(NULL);
	umPerPixel = 1.0;

	double temp = 0;

	//Operating modes for camera control
	const long NORMAL_OPERATING_MODE = 0;
	const long BULB_OPERATING_MODE = 1;

	switch(triggerMode)
	{
	case ICamera::SW_SINGLE_FRAME:
	case ICamera::SW_MULTI_FRAME:
	case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		{
			pCamera->SetParam(ICamera::PARAM_OP_MODE, NORMAL_OPERATING_MODE);
			pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, triggerMode);
		}
		break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH_BULB:
		{
			pCamera->SetParam(ICamera::PARAM_OP_MODE, BULB_OPERATING_MODE);
			pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::HW_MULTI_FRAME_TRIGGER_EACH);
		}
		break;
	default:
		{
			StringCbPrintfW(message,MSG_LENGTH,L"RunSample Execute trigger mode (%d) is not supported",triggerMode);
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
			return FALSE;
		}
	}

	if (ICamera::CCD == cameraType)
	{
		string camName;
		long camImageWidth;
		long camImageHeight;
		double camPixelSize;
		double camExposureTimeMS;		
		long gain, blackLevel, lightMode;				
		long left,top,right,bottom;
		long binningX, binningY;
		long tapsIndex, tapsBalance;
		long readoutSpeedIndex;
		long camAverageMode, camAverageNum;
		long camVericalFlip, camHorizontalFlip, imageAngle;

		//getting the values from the experiment setup XML files
		_pExp->GetCamera(camName,camImageWidth,camImageHeight,camPixelSize,camExposureTimeMS,gain,blackLevel,lightMode,left,top,right,bottom,binningX,binningY,tapsIndex,tapsBalance,readoutSpeedIndex,camAverageMode,camAverageNum,camVericalFlip,camHorizontalFlip,imageAngle);

		pCamera->SetParam(ICamera::PARAM_EXPOSURE_TIME_MS,camExposureTimeMS);
		pCamera->SetParam(ICamera::PARAM_GAIN,gain);
		pCamera->SetParam(ICamera::PARAM_OPTICAL_BLACK_LEVEL,blackLevel);
		pCamera->SetParam(ICamera::PARAM_LIGHT_MODE,lightMode);	
		pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_LEFT,left);
		pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_TOP,top);
		pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT,right);
		pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM,bottom);
		pCamera->SetParam(ICamera::PARAM_BINNING_X,binningX);
		pCamera->SetParam(ICamera::PARAM_BINNING_Y,binningY);
		pCamera->SetParam(ICamera::PARAM_TAP_INDEX,tapsIndex);
		pCamera->SetParam(ICamera::PARAM_TAP_BALANCE_MODE,tapsBalance);
		pCamera->SetParam(ICamera::PARAM_READOUT_SPEED_INDEX,readoutSpeedIndex);

		pCamera->SetParam(ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP,camVericalFlip);
		pCamera->SetParam(ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP,camHorizontalFlip);
		pCamera->SetParam(ICamera::PARAM_CAMERA_IMAGE_ANGLE,imageAngle);
		pCamera->SetParam(ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT, dmaFrames);
		//will average after acquisition is complete. Set to none at camera level
		pCamera->SetParam(ICamera::PARAM_CAMERA_AVERAGEMODE,ICamera::AVG_MODE_NONE);
		pCamera->SetParam(ICamera::PARAM_CAMERA_AVERAGENUM,1);

		bufferChannels = 1;
		width = camImageWidth;
		height = camImageHeight;
		umPerPixel = camPixelSize;
	}
	else
	{
		long areaMode, scanMode, interleave, pixelX, pixelY, chan, lsmFieldSize, offsetX, offsetY, averageMode, averageNum, clockSource, inputRange1, inputRange2, twoWayAlignment, extClockRate, flybackCycles, inputRange3, inputRange4, minimizeFlybackCycles, polarity[4], verticalFlip, horizontalFlip;
		double areaAngle,dwellTime,crsFrequencyHz = 0;
		long timeBasedLineScan = FALSE;
		long timeBasedLineScanMS = 0;
		long threePhotonEnable = FALSE;
		long numberOfPlanes = 1;
		//getting the values from the experiment setup XML files
		_pExp->GetLSM(areaMode,areaAngle,scanMode,interleave,pixelX,pixelY,chan,lsmFieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,dwellTime,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles, polarity[0],polarity[1],polarity[2],polarity[3], verticalFlip, horizontalFlip, crsFrequencyHz, timeBasedLineScan, timeBasedLineScanMS, threePhotonEnable, numberOfPlanes);

		//notify the ECU of the zoom change also
		IDevice * pControlUnitDevice = NULL;
		pControlUnitDevice = GetDevice(SelectedHardware::SELECTED_CONTROLUNIT);
		if(NULL != pControlUnitDevice)
		{
			if(SetDeviceParameterValue(pControlUnitDevice,IDevice::PARAM_SCANNER_ZOOM_POS,lsmFieldSize,FALSE,NULL,0))
			{
				//If the device is an ECU2, read the alignment value from the ECU and set it back to itself, same way as capture setup LSMFieldSize->Setter
				long minFS,maxFS,fieldSizeDefault, alignment, zone, zoneECU;
				GetDeviceParamRangeLong(SelectedHardware::SELECTED_CONTROLUNIT,IDevice::PARAM_SCANNER_ZOOM_POS,minFS,maxFS,fieldSizeDefault);
				zone = maxFS - lsmFieldSize;
				zoneECU = IDevice::PARAM_ECU_TWO_WAY_ZONE_1 + zone;
				if(GetDeviceParamLong(SelectedHardware::SELECTED_CONTROLUNIT, zoneECU, alignment))
				{
					SetDeviceParamDouble(SelectedHardware::SELECTED_CONTROLUNIT, zoneECU, alignment, FALSE);
				}
			}
		}

		//Force the averaging mode to be NONE for all bleaching captures
		averageMode = ICamera::AVG_MODE_NONE;
		averageNum = 1;
		switch (areaMode)
		{
		case 0: //Square
			{
				areaMode = ICamera::SQUARE;
				pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,TRUE);
			}
			break;
		case 1: //Rect
			{
				areaMode = ICamera::RECTANGLE;
				pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,TRUE);
			}
			break;
		case 2: //Kymopgragh
			{
				areaMode = ICamera::RECTANGLE;
				pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,FALSE);
			}
			break;
		case 3: //LineScan
			{
				areaMode = ICamera::RECTANGLE;
				pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,FALSE);
			}
			break;
		case 4: //Polyline
			{
				areaMode = ICamera::POLYLINE;
				pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,FALSE);
			}
			break;
		default:
			{
				areaMode = ICamera::SQUARE;
				pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,TRUE);
			}
			break;
		}

		pCamera->SetParam(ICamera::PARAM_LSM_AREAMODE,areaMode);
		pCamera->SetParam(ICamera::PARAM_LSM_SCANAREA_ANGLE,areaAngle);
		pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X,pixelX);
		pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y,pixelY);
		pCamera->SetParam(ICamera::PARAM_LSM_DWELL_TIME,dwellTime);
		pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,scanMode);
		pCamera->SetParam(ICamera::PARAM_LSM_INTERLEAVE_SCAN,interleave);
		pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL,chan);
		pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE,lsmFieldSize);
		pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X,offsetX);
		pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y,offsetY);
		//will average after acquisition is complete. Set to none at camera level
		pCamera->SetParam(ICamera::PARAM_LSM_AVERAGEMODE,ICamera::AVG_MODE_NONE);
		pCamera->SetParam(ICamera::PARAM_LSM_AVERAGENUM,averageNum);
		pCamera->SetParam(ICamera::PARAM_LSM_CLOCKSOURCE,clockSource);
		pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE1,inputRange1);
		pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE2,inputRange2);
		pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE3,inputRange3);
		pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE4,inputRange4);
		pCamera->SetParam(ICamera::PARAM_LSM_MINIMIZE_FLYBACK_CYCLES,minimizeFlybackCycles);
		pCamera->SetParam(ICamera::PARAM_LSM_ALIGNMENT,twoWayAlignment);
		pCamera->SetParam(ICamera::PARAM_LSM_EXTERNALCLOCKRATE,extClockRate);
		pCamera->SetParam(ICamera::PARAM_LSM_DMA_BUFFER_COUNT,dmaFrames);
		pCamera->SetParam(ICamera::PARAM_LSM_FLYBACK_CYCLE,flybackCycles);		
		pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF,FALSE);
		pCamera->SetParam(ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION, verticalFlip);
		pCamera->SetParam(ICamera::PARAM_LSM_HORIZONTAL_FLIP, horizontalFlip);
		pCamera->SetParam(ICamera::PARAM_LSM_3P_ENABLE, threePhotonEnable);
		pCamera->SetParam(ICamera::PARAM_LSM_NUMBER_OF_PLANES, numberOfPlanes);

		switch(chan)
		{
		case 0x1: bufferChannels = 1; break;
		case 0x2: bufferChannels = 1; break;
		case 0x4: bufferChannels = 1; break;
		case 0x8: bufferChannels = 1; break;
		default:				
			long paramType, paramAvailable, paramReadOnly;
			double paramMin, paramMax, paramDefault;
			pCamera->GetParamInfo(ICamera::PARAM_LSM_CHANNEL,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
			switch(static_cast<long>(paramMax))
			{
			case 0x3: bufferChannels = 3; break;
			case 0xF: bufferChannels = 4; break;
			default:  bufferChannels = 3; break;
			}				
			break;
		}

		width = pixelX;		

		//if its a timebased line scan then we want to get the pixel Y from the camera instead of assuming that it is what we set it as
		if (pCamera->SetParam(ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN, timeBasedLineScan) && TRUE == timeBasedLineScan)
		{
			pCamera->SetParam(ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS, timeBasedLineScanMS);
			double lsmHeight;
			pCamera->GetParam(ICamera::PARAM_LSM_PIXEL_Y, lsmHeight);
			height = pixelY = static_cast<long>(lsmHeight);
		}
		else
		{
			height = pixelY;
		}

		double fieldSizeCalibration = 100.0;
		pCamera->GetParam(ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION,fieldSizeCalibration);
		double magnification;
		string objName;
		_pExp->GetMagnification(magnification, objName);
		umPerPixel = (lsmFieldSize * fieldSizeCalibration)/(pixelX * magnification);
	}
	return TRUE;
}

long AcquireBleaching::LoadBleachWaveform(const wchar_t* bleachH5PathName, int CycleNum, int id)
{
	EnterCriticalSection(&AcquireBleaching::loadAccess);

	if(NULL != AcquireBleaching::bleachParams[id]->bufferHandle)
	{
		if(WAIT_OBJECT_0 != WaitForSingleObject(AcquireBleaching::bleachParams[id]->bufferHandle, TIMEOUT_MS))
		{
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}
	}
	////do realloc instead:
	//ClearBleachParams();
	AcquireBleaching::bleachParams[id]->lastLoaded = FALSE;

	if(FALSE == h5io->OpenFileIO(bleachH5PathName,H5FileType::READONLY))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"AcquireBleaching Open BleachWaveform.raw failed");
		ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
		LeaveCriticalSection(&AcquireBleaching::loadAccess);
		return FALSE;
	}
	unsigned long long dataSize;
	bool ret = true;
	if(TRUE == h5io->CheckGroupDataset("/Analog","/Pockel",dataSize) && dataSize > 0)
	{
		AcquireBleaching::bleachParams[id]->Scanmode = static_cast<long>(ScanMode::BLEACH_SCAN);
		AcquireBleaching::bleachParams[id]->CycleNum = CycleNum;										//Cycle number
		AcquireBleaching::bleachParams[id]->analogPockelSize = static_cast<unsigned long>(dataSize);	//unit size
		AcquireBleaching::bleachParams[id]->analogXYSize = static_cast<unsigned long>(dataSize * 2);	//2x unit size
		AcquireBleaching::bleachParams[id]->digitalSize = static_cast<unsigned long>(dataSize * 6);		//with complete line,cycle envelope,iteraton envelope, pattern envelope, pattern complete


		_pTemp = (char*)realloc((void*)AcquireBleaching::bleachParams[id]->GalvoWaveformXY, AcquireBleaching::bleachParams[id]->analogXYSize * sizeof(double));
		if (NULL == _pTemp)
		{
			logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"AcquireBleaching realloc GalvoWaveformXY failed");
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}


		AcquireBleaching::bleachParams[id]->GalvoWaveformXY = (double*)_pTemp;

		_pTemp = (char*)realloc(AcquireBleaching::bleachParams[id]->GalvoWaveformPockel, AcquireBleaching::bleachParams[id]->analogPockelSize * sizeof(double));
		if (NULL == _pTemp)
		{
			logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"AcquireBleaching realloc GalvoWaveformXY failed");
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}

		AcquireBleaching::bleachParams[id]->GalvoWaveformPockel = (double*)_pTemp;

		_pTemp = (char*)realloc(AcquireBleaching::bleachParams[id]->DigBufWaveform, AcquireBleaching::bleachParams[id]->digitalSize * sizeof(unsigned char));
	
		if(NULL == _pTemp)
		{
			logDll->TLTraceEvent(VERBOSE_EVENT,1,L"AcquireBleaching realloc DigBufWaveform failed");
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}

		AcquireBleaching::bleachParams[id]->DigBufWaveform = (unsigned char*)_pTemp;


		unsigned long * tmpClk = (unsigned long*)malloc(sizeof(unsigned long));
		if (FALSE == h5io->ReadData("","/ClockRate",tmpClk,H5DataTypeEnum::DATA_UINT32,0,1)) 
		{
			ret = false;
		}
		AcquireBleaching::bleachParams[id]->ClockRate = tmpClk[0];
		free(tmpClk);
		if (FALSE == h5io->ReadData("/Digital","/PockelDig",AcquireBleaching::bleachParams[id]->DigBufWaveform,H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = false;
		}
		if(WAIT_OBJECT_0 == WaitForSingleObject(hEventStopLoad, 0))
		{
			h5io->CloseFileIO();
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}
		if (FALSE == h5io->ReadData("/Digital","/CycleComplete",AcquireBleaching::bleachParams[id]->DigBufWaveform+dataSize*sizeof(unsigned char),H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = false;
		}
		if(WAIT_OBJECT_0 == WaitForSingleObject(hEventStopLoad, 0))
		{
			h5io->CloseFileIO();
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}
		if (FALSE == h5io->ReadData("/Digital","/CycleEnvelope",AcquireBleaching::bleachParams[id]->DigBufWaveform+2*dataSize*sizeof(unsigned char),H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = false;
		}
		if(WAIT_OBJECT_0 == WaitForSingleObject(hEventStopLoad, 0))
		{
			h5io->CloseFileIO();
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}
		if (FALSE == h5io->ReadData("/Digital","/IterationEnvelope",AcquireBleaching::bleachParams[id]->DigBufWaveform+3*dataSize*sizeof(unsigned char),H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = false;
		}
		if(WAIT_OBJECT_0 == WaitForSingleObject(hEventStopLoad, 0))
		{
			h5io->CloseFileIO();
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}
		if (FALSE == h5io->ReadData("/Digital","/PatternEnvelope",AcquireBleaching::bleachParams[id]->DigBufWaveform+4*dataSize*sizeof(unsigned char),H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = false;
		}
		if(WAIT_OBJECT_0 == WaitForSingleObject(hEventStopLoad, 0))
		{
			h5io->CloseFileIO();
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}
		if (FALSE == h5io->ReadData("/Digital","/PatternComplete",AcquireBleaching::bleachParams[id]->DigBufWaveform+5*dataSize*sizeof(unsigned char),H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = false;
		}
		if(WAIT_OBJECT_0 == WaitForSingleObject(hEventStopLoad, 0))
		{
			h5io->CloseFileIO();
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}
		if (FALSE == h5io->ReadData("/Analog","/Pockel",AcquireBleaching::bleachParams[id]->GalvoWaveformPockel,H5DataTypeEnum::DATA_DOUBLE,0,dataSize))
		{
			ret = false;
		}
		if(WAIT_OBJECT_0 == WaitForSingleObject(hEventStopLoad, 0))
		{
			h5io->CloseFileIO();
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}
		//interleave XY waveform for Galvo-Galvo:
		double * tmpWf = (double *) malloc(AcquireBleaching::bleachParams[id]->analogPockelSize*sizeof(double));
		if (FALSE == h5io->ReadData("/Analog","/X",tmpWf,H5DataTypeEnum::DATA_DOUBLE,0,dataSize))
		{
			ret = false;
		}
		if(WAIT_OBJECT_0 == WaitForSingleObject(hEventStopLoad, 0))
		{
			h5io->CloseFileIO();
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}
		for(unsigned int i=0;i<AcquireBleaching::bleachParams[id]->analogPockelSize;i++)
		{
			AcquireBleaching::bleachParams[id]->GalvoWaveformXY[2*i]=tmpWf[i];
		}
		if (FALSE == h5io->ReadData("/Analog","/Y",tmpWf,H5DataTypeEnum::DATA_DOUBLE,0,dataSize))
		{
			ret = false;
		}
		if(WAIT_OBJECT_0 == WaitForSingleObject(hEventStopLoad, 0))
		{
			h5io->CloseFileIO();
			ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
			LeaveCriticalSection(&AcquireBleaching::loadAccess);
			return FALSE;
		}
		for(unsigned int i=0;i<AcquireBleaching::bleachParams[id]->analogPockelSize;i++)
		{
			AcquireBleaching::bleachParams[id]->GalvoWaveformXY[2*i+1]=tmpWf[i];
		}
		free(tmpWf);
	}
	h5io->CloseFileIO();
	if(false == ret)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"AcquireBleaching Read BleachWaveform.raw failed");
		ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
		LeaveCriticalSection(&AcquireBleaching::loadAccess);
		return FALSE;
	}
	else
	{
		AcquireBleaching::bleachParams[id]->lastLoaded = TRUE;
	}

	ReleaseMutex(AcquireBleaching::bleachParams[id]->bufferHandle);
	LeaveCriticalSection(&AcquireBleaching::loadAccess);
	return TRUE;
}

long AcquireBleaching::PreCaptureAutoFocus(long index, long subWell)
{
	double magnification;
	string objName;
	_pExp->GetMagnification(magnification,objName);

	long position=0;
	double numAperture;
	double afStartPos=0;
	double afFocusOffset=0;
	double afAdaptiveOffset=0;
	long beamExpPos=0;
	long beamExpWavelength=0;
	long beamExpPos2=0;
	long beamExpWavelength2=0;
	long turretPosition=0;
	long zAxisToEscape=0;
	double zAxisEscapeDistance=0;
	double fineAutoFocusPercentage = 0.15;

	//Get filter parameters from hardware setup.xml
	pHardware->GetMagInfoFromName(objName,magnification,position,numAperture,afStartPos,afFocusOffset,afAdaptiveOffset,beamExpPos,beamExpWavelength,beamExpPos2,beamExpWavelength2,turretPosition,zAxisToEscape,zAxisEscapeDistance,fineAutoFocusPercentage);
	_adaptiveOffset = afAdaptiveOffset;

	long aftype,repeat;
	double expTimeMS,stepSizeUM,startPosMM,stopPosMM;

	_pExp->GetAutoFocus(aftype,repeat,expTimeMS,stepSizeUM,startPosMM,stopPosMM);


	//Determine if we are capturing the first image for the experiment. If so make sure an autofocus is executed if enabled.
	//After the first iteration the Z position will overlap with the XY motion
	if((aftype != IAutoFocus::AF_NONE)&&(subWell==1))
	{
		_evenOdd = FALSE;
		_lastGoodFocusPosition = afStartPos + _adaptiveOffset;
		if(FALSE == SetAutoFocusStartZPosition(afStartPos,TRUE,FALSE))
		{
			return FALSE;
		}
	}

	BOOL afFound = FALSE;

	if (FALSE == RunAutofocus(index, aftype, afFound))
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample RunAutoFocus failed");
		return FALSE;
	}

	return TRUE;
}

long AcquireBleaching::CaptureStream(ICamera* pCamera, char* pMemoryBuffer, long currentT, long numFrames, long avgFrames, SaveParams* sp)
{
	long ret = TRUE, stopStatus = FALSE;
	long t = currentT;
	long lastFrame = currentT + (numFrames - 1);
	FrameInfo frameInfo = { 0, 0, 0, 0 };

	//Force the averaging mode to be NONE for all bleaching captures
	avgFrames = 1;

	pCamera->SetupAcquisition(pMemoryBuffer);
	if (NULL != AcquireBleaching::hEventPostBleachSetupComplete)
	{
		SetEvent(hEventPostBleachSetupComplete);
	}
	//HW timed out or failed to start:
	if (FALSE == pCamera->StartAcquisition(pMemoryBuffer))
	{
		StringCbPrintfW(message, MSG_LENGTH, L"AcquireBleaching StartAcquisition Failed.");
		logDll->TLTraceEvent(ERROR_EVENT, 1, message);
		return FALSE;
	}

	long imageIndex = 1;
	//this is the dropped frame count:
	double droppedFrameCnt = 0;
	//this is the number of frames available to be copied in the lower level DMA circular buffer:
	double dmaBufferAvailableFrames = 0;
	//This is the type of CCD camera that is currently selected
	double ccdType = ICamera::CMOS;

	//Read the size of the dma buffer from the lower level and use that if it is larger
	double dmaFrm = 0;
	pCamera->GetParam(ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT, dmaFrm);
	long dmaFrames = max(static_cast<long>(dmaFrm), dmaFrames);

	//No preview if occupied buffer is over the limit
	double dmaInUseLimit = (20 <= dmaFrames) ? 0.90 : 0.60;
	//Change the limit if it is an ORCA to throttle the preview image rate sooner if the dma buffer is filling up quickly
	pCamera->GetParam(ICamera::PARAM_CCD_TYPE, ccdType);
	dmaInUseLimit = (ICamera::ORCA == ccdType) ? 0.3 : dmaInUseLimit;

	for (; t <= lastFrame; t++, imageIndex++)
	{
		for (long i = 0; i < avgFrames; i++)
		{
			if (i == 0)
			{
				if ((sp->subWell == 1) && (t == 1))
				{
					_acquireSaveInfo->getInstance()->SetExperimentStartCount();	//set current timer count as start of the experiment
				}
				else
				{
					_acquireSaveInfo->getInstance()->AddTimingInfo();	//add the current time to the list
				}
				_acquireSaveInfo->getInstance()->AddTimestamp();
			}

			//This is the buffer being locked
			int totalFrame = (imageIndex - 1) * avgFrames + i;

			//status of last frame
			long status = ICamera::STATUS_BUSY;
			const long FRAME_WAIT_TIMEOUT = 5000;
			double cameraType;
			pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE, cameraType);

			DWORD frameStartTime = GetTickCount();

			while ((ICamera::STATUS_BUSY == status) || (ICamera::STATUS_PARTIAL == status))
			{
				if (FALSE == pCamera->StatusAcquisition(status) || (0 < static_cast<long>(droppedFrameCnt)))
				{
					StringCbPrintfW(message, MSG_LENGTH, L"AcquireBleaching status acquisition returned false or number of dropped frames is bigger than 1");
					logDll->TLTraceEvent(ERROR_EVENT, 1, message);
					break;
				}

				//LSM will wait for triggers at StartAcquisition, break when timed out.
				//camera will wait for triggers here instead.
				DWORD currentTime = GetTickCount();
				if ((ICamera::LSM == (ICamera::CameraType)static_cast<long>(cameraType))
					&& (currentTime - frameStartTime) > FRAME_WAIT_TIMEOUT)
				{
					break;
				}

				//check if user has asked to stop the capture
				if (NULL != hStopCapture)
				{
					if (WAIT_OBJECT_0 == WaitForSingleObject(hStopCapture, 0))
						return FALSE;
				}
				else
				{
					StopCaptureEventCheck(stopStatus);
					if (1 == stopStatus)
						return FALSE;
				}
			}

			if (ICamera::STATUS_ERROR == status)
			{
				StringCbPrintfW(message, MSG_LENGTH, L"AcquireBleaching status returned ERROR, stopping acquisition");
				logDll->TLTraceEvent(ERROR_EVENT, 1, message);
				SetEvent(hStopCapture);
				SetEvent(hStopBleach);
				break;
			}

			pMemoryBuffer = ImageManager::getInstance()->GetImagePtr(sp->imageID, 0, 0, 0, totalFrame); //**Update to actual frame number*//
			if (pMemoryBuffer == NULL)
			{
				StringCbPrintfW(message, MSG_LENGTH, L"AcquireBleaching invalid memory buffer");
				logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);
			}

			pCamera->CopyAcquisition(pMemoryBuffer, &frameInfo);

			pCamera->GetParam(ICamera::PARAM_DROPPED_FRAMES, droppedFrameCnt);

			if (ICamera::ORCA == ccdType)
			{
				pCamera->GetParam(ICamera::PARAM_DMA_BUFFER_AVAILABLE_FRAMES, dmaBufferAvailableFrames);
			}

			if ((GetTickCount() - _lastImageUpdateTime) > 1000 * (1 / sp->previewRate))
			{
				if ((dmaInUseLimit > static_cast<double>(droppedFrameCnt / dmaFrames)) && (dmaInUseLimit > static_cast<double>(dmaBufferAvailableFrames / dmaFrames)))	//no preview if camera is close to overflow limit
				{
					if (sp->displayImage)
					{
						SavePreviewImage(sp, t, pMemoryBuffer);
					}

					_lastImageUpdateTime = GetTickCount();
				}
			}

			//synchrnously start the unlock process for the frame
			ImageManager::getInstance()->UnlockImagePtr(sp->imageID, 0, 0, 0, totalFrame);

		}

		CallSaveTImage(t);
		CallSaveImage(t, TRUE);
		_capturedImageID++;

		//check if user has asked to stop the capture
		if (NULL != hStopCapture)
		{
			if (WAIT_OBJECT_0 == WaitForSingleObject(hStopCapture, 0))
				return FALSE;
		}
		else
		{
			StopCaptureEventCheck(stopStatus);
			if (1 == stopStatus)
				return FALSE;
		}
	}

	return ret;
}

long AcquireBleaching::CaptureTSeries(ICamera *pCamera, long currentT, long tFrames, long avgFrames, SaveParams *sp, double timeInterval, long simultaneous)
{
	long ret = TRUE, stopStatus = FALSE;
	long t = currentT;
	long lastFrame = currentT + (tFrames-1);

	//Force the averaging mode to be NONE for all bleaching captures
	avgFrames = 1;

	AcquireFactory factory;

	auto_ptr<IAcquire> acqFrame(NULL);

	ICamera::CameraType cameraType;

	double val;
	pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,val);

	cameraType = (ICamera::CameraType)static_cast<long> (val);

	switch(cameraType)
	{
	case ICamera::CCD:
		{				
			if(_pExp->GetNumberOfWavelengths() > 1)
			{
				acqFrame.reset(factory.getAcquireInstance(AcquireFactory::ACQ_MULTI_WAVELENGTH,NULL,_pExp,_path));
			}
			else
			{
				acqFrame.reset(factory.getAcquireInstance(AcquireFactory::ACQ_SINGLE,NULL,_pExp,_path));
			}

		}
		break;
	case ICamera::LSM:
		{			
			acqFrame.reset(factory.getAcquireInstance(AcquireFactory::ACQ_SINGLE,NULL,_pExp,_path));
		}
		break;
	}

	if(NULL != AcquireBleaching::hEventPostBleachSetupComplete)
	{
		SetEvent(hEventPostBleachSetupComplete);
	}

	for(;t<=lastFrame;t++)
	{		
		HANDLE hTimer = NULL;
		LARGE_INTEGER liDueTime;

		liDueTime.QuadPart = static_cast<LONGLONG>(-10000000LL * timeInterval);

		// Create an unnamed waitable timer.
		hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
		if (NULL == hTimer)
		{
			return FALSE;
		}

		// Set a timer to wait for 10 seconds.
		if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
		{
			return FALSE;
		}
		long z=1;
		long updateIndex = 0;

		updateIndex = sp->index + t-1;

		//don't move the z axis
		if(FALSE == acqFrame->Execute(sp->index,sp->subWell,1,t))
		{
			return FALSE;
		}		

		//update progress T to observer.
		CallSaveTImage(t);
		_capturedImageID++;

		//update progress to observer.
		CallSaveImage(updateIndex, TRUE);

		//don't wait for the last frame
		//only use a wait when the hardware trigger mode is disabled
		if((t != lastFrame))
		{
			while(WaitForSingleObject(hTimer, 1) != WAIT_OBJECT_0)
			{
				//update progress to observer.
				CallSaveImage(updateIndex, FALSE);
				Sleep(10);

				//check if user has asked to stop the capture
				if(NULL != hStopCapture)
				{
					if(WAIT_OBJECT_0 == WaitForSingleObject(hStopCapture, 0))
					{
						CloseHandle(hTimer);
						return FALSE;
					}
				}
				else
				{
					StopCaptureEventCheck(stopStatus);

					if(stopStatus == TRUE)
					{
						CloseHandle(hTimer);
						return FALSE;
					}
				}
			}
			Sleep(30);
		}
		else
		{
			//update progress to observer.
			CallSaveImage(updateIndex, FALSE);
			Sleep(10);
		}

		CloseHandle(hTimer);

		StopCaptureEventCheck(stopStatus);

		if(stopStatus == TRUE)
		{
			return FALSE;
		}

		//only check the bleach status when in non simultaneous mode,
		//or in using same type of camera and bleacher:
		long checkBleachStatus = (FALSE == simultaneous) ? TRUE : FALSE;

		ICamera *pBleachScanner = NULL;
		pBleachScanner = GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER);

		if(ICamera::LSM == cameraType)
		{
			double camVal, bleachVal;
			pCamera->GetParam(ICamera::PARAM_LSM_TYPE,camVal);
			pBleachScanner->GetParam(ICamera::PARAM_LSM_TYPE,bleachVal);
			checkBleachStatus = ((ICamera::GALVO_GALVO == (ICamera::LSMType)static_cast<long>(bleachVal)) && 
				((ICamera::GALVO_GALVO == (ICamera::LSMType)static_cast<long> (camVal)))) ? TRUE : FALSE;
		}

		//Continue capture after triggered first:
		pCamera->GetParam(ICamera::PARAM_TRIGGER_MODE,val);
		if((ICamera::TriggerMode::HW_MULTI_FRAME_TRIGGER_FIRST == val) && (currentT == t))
		{
			if ((checkBleachStatus) && (hEventBleach))
			{
				//wait after bleaching is done:
				while(WaitForSingleObject(hEventBleach, 10) != WAIT_OBJECT_0)
				{
					//bleaching may be done already before trigger
					if(NULL == hEventBleach)
						break;

					//check if user has asked to stop the capture
					if(NULL != hStopCapture)
					{
						if(WAIT_OBJECT_0 == WaitForSingleObject(hStopCapture, 0))
						{
							ret = FALSE;
							break;
						}
					}
					else
					{
						StopCaptureEventCheck(stopStatus);
						if(1 == stopStatus)
						{
							ret = FALSE;
							break;
						}
					}
				};
			}
			//switch back to software trigger after the first frame
			pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_SINGLE_FRAME);			
		}		
	}	

	return ret;
}

void AcquireBleaching::AverageStream(long numFrames, char *pMemoryBuffer, long avgFrames, long imageID, long size, unsigned long *pSumMemoryBuffer)
{
	for(long t = 0; t < numFrames; t++)
	{
		/* process averaging */
		unsigned long* pSum = 0;
		unsigned short* pMem = 0;

		for(long i = 0; i < avgFrames; i++)
		{
			int totalFrame = t * avgFrames + i;

			pMemoryBuffer = ImageManager::getInstance()->GetImagePtr(imageID, 0, 0, 0, totalFrame); //**Update to actual frame number*//

			if(NULL != pSumMemoryBuffer && NULL != pMemoryBuffer)
			{
				pSum = pSumMemoryBuffer;
				pMem = (unsigned short*) pMemoryBuffer;
				for(long j = 0; j < size; ++j)
				{
					*(pSum + j) += *(pMem + j);
				}
			}
			//synchrnously start the unlock process for the frame
			ImageManager::getInstance()->UnlockImagePtr(imageID, 0, 0, 0, totalFrame);
		}

		pMemoryBuffer = ImageManager::getInstance()->GetImagePtr(imageID, 0, 0, 0, t); //**Update to actual frame number*//

		if (NULL != pSumMemoryBuffer && NULL != pMemoryBuffer)
		{
			pSum = pSumMemoryBuffer;
			pMem = (unsigned short*)pMemoryBuffer;

			for (long j = 0; j < size; ++j)
			{
				*(pMem + j) = static_cast<unsigned short>(*(pSum + j) / avgFrames);
			}

			//send the sum buffer back to zero for the next set of averages
			memset(pSumMemoryBuffer, 0, sizeof(unsigned long) * size);
			ImageManager::getInstance()->UnlockImagePtr(imageID, 0, 0, 0, t);
		}
	}
}

void AcquireBleaching::SaveStream(long currentT, long numFrames, SaveParams *sp)
{
	//for loop is to update ui events. no logic consequence
	//for(long t=1;t<numFrames;t++)
	//{
	//	CallSaveTImage(t);
	//	CallSaveImage(t, TRUE);
	//}
	long capturedFrames = _capturedImageID - currentT + 1;	//_capturedImageID (0-based), currentT (1-based) 

	ImageManager::getInstance()->DestroyImage(sp->imageID);

	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	wchar_t rawName[_MAX_PATH];
	wchar_t newName[_MAX_PATH];


	_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
	StringCbPrintfW(rawName,_MAX_PATH,L"%s%s\\*.tmp",drive,dir);

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream rawNameFormat;
	rawNameFormat << L"%s%s\\Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";
	std::wstringstream rawBaseNameFormat;
	rawBaseNameFormat << L"%s%s\\Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_";

	hFind = FindFirstFile(rawName, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		StringCbPrintfW(rawName,_MAX_PATH,L"%s%s\\%s",drive,dir,FindFileData.cFileName);
		//cut the file if partially captured:
		if(numFrames != capturedFrames)
		{
			//resize file to the captured frame size:
			std::vector<int> intVec;
			RawFile<short> rawFile(wstring(rawName), sp->width, sp->height, 1 ,sp->bufferChannels,1, true, intVec, GenericImage<short>::CONTIGUOUS_CHANNEL);
			rawFile.shortenSeriesTo(capturedFrames);
			rawFile.releaseFile();
		}
		StringCbPrintfW(newName,_MAX_PATH,rawNameFormat.str().c_str(),drive,dir,sp->index,sp->subWell,currentT);
		_wrename(rawName,newName);
		StringCbPrintfW(_rawBaseName,_MAX_PATH,rawBaseNameFormat.str().c_str(),drive,dir,sp->index,sp->subWell);
		FindClose(hFind);
	}

	//clear the lists
	_acquireSaveInfo->getInstance()->ClearTimingInfo();
	_acquireSaveInfo->getInstance()->ClearTimestamps();

}

long AcquireBleaching::PreCaptureProtocol(ICamera * pCamera, long index, long subWell, long streaming, long numFrames, long triggerEnable, long lsmChannel, Dimensions *d, SaveParams *sp)
{
	long ret = TRUE;
	long captureTriggerMode;
	if(triggerEnable)
	{
		captureTriggerMode = ICamera::HW_MULTI_FRAME_TRIGGER_FIRST;		
	}
	else
	{   
		captureTriggerMode =  (TRUE == streaming) ? ICamera::SW_MULTI_FRAME :ICamera::SW_SINGLE_FRAME;			
	}

	long bufferChannels,width,height, displayImage;
	double umPerPixel;
	//push the settings to the camera
	if(FALSE == SetupCameraParams(pCamera,bufferChannels,width,height,umPerPixel,displayImage,captureTriggerMode))
	{
		return FALSE;
	}

	//Force the averaging mode to be NONE for all bleaching captures
	long avgFrames = 1;
	pCamera->SetParam(ICamera::PARAM_LSM_AVERAGEMODE,ICamera::AVG_MODE_NONE);
	pCamera->SetParam(ICamera::PARAM_CAMERA_AVERAGEMODE,ICamera::AVG_MODE_NONE);

	if (TRUE == triggerEnable && FALSE == streaming)
	{
		pCamera->SetParam(ICamera::PARAM_MULTI_FRAME_COUNT, avgFrames);
	}

	wstring streamPath; double previewRate = 4;

	d->c = bufferChannels;
	d->dType = INT_16BIT;
	d->m = 1;

	switch(streaming)
	{
	case 1:
		{
			//save the raw files
			d->mType = CONTIGUOUS_CHANNEL_MEM_MAP_NO_FILE_DELETE;

			//notify camera to capture finite number of images
			pCamera->SetParam(ICamera::PARAM_MULTI_FRAME_COUNT, numFrames*avgFrames);
			d->t = numFrames * avgFrames;
			pHardware->GetStreaming(streamPath, previewRate);

			//for raw mode output to the experiment folder
			wchar_t drive[_MAX_DRIVE];
			wchar_t dir[_MAX_DIR];
			wchar_t fname[_MAX_FNAME];
			wchar_t ext[_MAX_EXT];
			wchar_t rawPath[_MAX_PATH];

			_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
			StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s",drive,dir);
			ImageManager::getInstance()->SetMemMapPath(rawPath);
		}
		break;
	default:
		{
			d->mType = CONTIGUOUS_CHANNEL;
			d->t = 1;
		}
		break;
	}

	d->x = width;
	d->y = height;
	d->z = 1;
	d->imageBufferType = 0;
	string lambdaName;
	double exposureTimeMS;

	//populate all of the wavelength names
	for(long w=0; w<_pExp->GetNumberOfWavelengths(); w++)
	{
		_pExp->GetWavelength(w,lambdaName,exposureTimeMS);
		AcquireBleaching::_wavelengthName[w] = lambdaName;
	}

	sp->index = index;
	sp->subWell = subWell;
	sp->width = width;
	sp->height = height;
	sp->red[0]=255;
	sp->green[0]=255;
	sp->blue[0]=255;
	sp->bp[0]=0;
	sp->wp[0]=255;
	sp->colorChannels = _pExp->GetNumberOfWavelengths();
	sp->bufferChannels = bufferChannels;
	sp->umPerPixel = umPerPixel;
	sp->totFrames = numFrames;
	sp->lsmChannels = lsmChannel;
	sp->displayImage = displayImage;
	sp->previewRate = previewRate;

	return ret;
}

void  AcquireBleaching::PostCaptureProtocol(long streaming, long currentT, long numFrames, SaveParams *sp)
{
	//save timing information to file after capture is done
	SaveTimingToExperimentFolder();

	//sp is not updated if capture was in thread:
	sp->imageID = lastImageID;

	if(1 == streaming)
	{
		SaveStream(currentT, numFrames, sp);
	}
	else
	{
		//clear the lists
		_acquireSaveInfo->getInstance()->ClearTimingInfo();
		_acquireSaveInfo->getInstance()->ClearTimestamps();
	}
}

long AcquireBleaching::Capture(ICamera * pCamera, long currentT, long streaming, long numFrames, double timeInterval, Dimensions d, SaveParams *sp, long simultaneous)
{	
	long ret = TRUE;
	if (0 == numFrames)
	{
		if(NULL != AcquireBleaching::hEventPostBleachSetupComplete)
			SetEvent(hEventPostBleachSetupComplete);
		return ret;
	}

	char * pMemoryBuffer = NULL;

	//Force the averaging mode to be NONE for all bleaching captures
	long avgFrames = 1;

	long imageID=0;
	if(ImageManager::getInstance()->CreateImage(imageID,d)== FALSE)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create memory buffer");
		return FALSE;
	}

	lastImageID = sp->imageID = imageID;

	pCamera->PreflightAcquisition(pMemoryBuffer);

	switch(streaming)
	{
	case 1:
		{
			if(FALSE == CaptureStream(pCamera, pMemoryBuffer, currentT, numFrames, avgFrames, sp))
			{
				StringCbPrintfW(message,MSG_LENGTH,L"AcquireBleaching invalid memory buffer %s %d",__FILE__,__LINE__);
				logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
				ret = FALSE;
			}
		}
		break;
	default:
		{
			if(FALSE == CaptureTSeries(pCamera, currentT, numFrames, avgFrames, sp, timeInterval, simultaneous))
			{
				StringCbPrintfW(message,MSG_LENGTH,L"AcquireBleaching invalid memory buffer %s %d",__FILE__,__LINE__);
				logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
				ret = FALSE;
			}
		}
		break;
	}

	pCamera->PostflightAcquisition(NULL);

	return ret;
}

long AcquireBleaching::CompareROIFile(wchar_t* path)
{
	long ret = TRUE;
	bool reset = false;
	char * tmp = NULL;
	streampos size;

	//read file:
	ifstream file(path,ios::in|ios::binary|ios::ate);
	if(file.is_open())
	{
		size = file.tellg();
		tmp = (char *)malloc(static_cast<int>(size)*sizeof(char *));
		file.seekg(0,ios::beg);
		file.read(tmp,size);
		file.close();
	}

	//do comparison:
	if((bleachMem == NULL) || ((static_cast<int>(bleachMemSize) > 0) && (static_cast<int>(size) != static_cast<int>(bleachMemSize))))
	{
		reset = true;
	}
	else
	{
		for(int i=0;i<size;i++)
		{
			if(bleachMem[i] != tmp[i])
			{
				reset = true;
				break;
			}
		}
	}
	//reset memory:
	if(reset)
	{
		_pTemp = (char*)realloc(bleachMem, static_cast<int>(size) * sizeof(char*));
		if (NULL != _pTemp)
		{
			bleachMem = _pTemp;
			if (bleachMem != NULL)
			{
				memcpy(bleachMem, tmp, static_cast<int>(size) * sizeof(char*));
				bleachMemSize = size;
			}
		}
		ret = FALSE;
	}
	free(tmp);
	tmp = NULL;
	return ret;
}

long AcquireBleaching::Execute(long index, long subWell)
{	
	//perform an autofocus
	if(FALSE == PreCaptureAutoFocus(index, subWell))
	{
		return FALSE;
	}

	ICamera *pCamera = NULL;

	pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);

	if(NULL == pCamera)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not retrieve the active camera");
		return FALSE;
	}

	ICamera *pBleachScanner = NULL;

	pBleachScanner = GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER);

	if(NULL == pBleachScanner)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not retrieve the active bleach scanner");
		return FALSE;
	}

	long photoBleachingEnable,laserPositiion,durationMS, bleachTrigger, preBleachingFrames, bleachWidth, bleachHeight,bleachOffsetX,bleachOffsetY, bleachingFrames, bleachFieldSize, postBleachingFrames1, postBleachingFrames2,preBleachingStream,postBleachingStream1,postBleachingStream2,powerEnable,laserEnable,bleachQuery,bleachPostTrigger,enableSimultaneousBleachingAndImaging,pmtEnableDuringBleach[4];
	double powerPosition,preBleachingInterval,postBleachingInterval1,postBleachingInterval2,areaAngle,dwellTime,crsFrequencyHz = 0;
	long areaMode,scanMode,interleave,pixelX,pixelY,channel,fieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles, polarity[4];
	long verticalFlip, horizontalFlip;
	long timeBasedLineScan = FALSE;
	long timeBasedLineScanMS = 0;
	long threePhotonEnable = FALSE;
	long numberOfPlanes = 1;
	_pExp->GetPhotobleaching(photoBleachingEnable, laserPositiion, durationMS, powerPosition, bleachWidth,bleachHeight,bleachOffsetX,bleachOffsetY, bleachingFrames, bleachFieldSize, bleachTrigger,preBleachingFrames, preBleachingInterval,preBleachingStream, postBleachingFrames1, postBleachingInterval1,postBleachingStream1, postBleachingFrames2, postBleachingInterval2,postBleachingStream2,powerEnable,laserEnable,bleachQuery,bleachPostTrigger,enableSimultaneousBleachingAndImaging,pmtEnableDuringBleach[0],pmtEnableDuringBleach[1],pmtEnableDuringBleach[2],pmtEnableDuringBleach[3]);
	_pExp->GetLSM(areaMode,areaAngle,scanMode,interleave,pixelX,pixelY,channel,fieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,dwellTime,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles, polarity[0],polarity[1],polarity[2],polarity[3], verticalFlip, horizontalFlip, crsFrequencyHz, timeBasedLineScan, timeBasedLineScanMS, threePhotonEnable, numberOfPlanes);
	pCamera->SetParam(ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION, verticalFlip);
	pCamera->SetParam(ICamera::PARAM_LSM_HORIZONTAL_FLIP, horizontalFlip);
	pHardware->GetShutterOptions(_digiShutterEnabled);
	//convert bleach trigger mode into a bleach scanner trigger mode
	long trigMode;
	switch(bleachTrigger)
	{
	case 1:
		{
			trigMode = ICamera::HW_MULTI_FRAME_TRIGGER_FIRST;
		}
		break;
	case 2:
		{
			trigMode = ICamera::HW_MULTI_FRAME_TRIGGER_EACH;
		}
		break;
	default:
		{
			trigMode = ICamera::SW_MULTI_FRAME;
		}
	}
	//convert bleach post trigger mode into a camera trigger mode
	long postTrigMode;
	switch(bleachPostTrigger)
	{
	case 1:
		{
			postTrigMode = ICamera::HW_MULTI_FRAME_TRIGGER_FIRST;
		}
		break;
	default:
		{
			postTrigMode = ICamera::SW_MULTI_FRAME;
		}
	}

	double ledPower1 = 0, ledPower2 = 0, ledPower3 = 0, ledPower4 = 0, ledPower5 = 0, ledPower6 = 0;
	SetLEDs(_pExp, pCamera, 0, ledPower1, ledPower2, ledPower3, ledPower4, ledPower5, ledPower6);

	//enable and set the relevent PMTs
	//also start the resonance scanner
	SetPMT();
	//The scanner is turned off and then on. This may
	//be due to the scanner not accepting the ON
	ScannerEnable(0,TRUE);
	ScannerEnable(1,TRUE);

	//initialize the progress indicator
	CallSaveImage(0, FALSE);

	//open the shutter before the first stream
	if (TRUE == _digiShutterEnabled)
	{
		OpenShutter();
	}

	//master time index
	long currentT = 1;

	SaveParams sp;

	//Verify configuration before start:
	if(ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == postTrigMode || TRUE == enableSimultaneousBleachingAndImaging)
	{
		//return if using bleacher(Galvo/Galvo) same as detector:
		double camVal, bleachVal;
		pCamera->GetParam(ICamera::PARAM_LSM_TYPE,camVal);
		pBleachScanner->GetParam(ICamera::PARAM_LSM_TYPE,bleachVal);
		if((ICamera::GALVO_GALVO == (ICamera::LSMType)static_cast<long> (bleachVal)) && ((ICamera::GALVO_GALVO == (ICamera::LSMType)static_cast<long> (camVal))))
		{
			MessageBox(NULL,L"Bleach Scanner and Image Detector cannot both be GalvoGalvo when using Hardware Trigger or Simultaneous mode during bleaching acquisition.",L"HW Trigger Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY | MB_ICONWARNING);
			//Close the shutter before returning
			if (TRUE == _digiShutterEnabled)
			{
				CloseShutter();
			}
			return FALSE;
		}

		//It is not possible to use the HW trigger option for the post bleach imaging when
		//using a bleach scanner pockels on the same board as the Image detector (Dev1 for GR)
		//To avoid an error in the acquisition, first check if any bleach line for the bleach
		//scanner is on Dev1. If it is on Dev1 then show a message and return FALSE
		const long MAX_POCKELS_LINE_LEN = 100;
		const long MAX_POCKELS = 3;
		wchar_t pockelsLineBleachScanner[MAX_POCKELS][MAX_POCKELS_LINE_LEN];
		pBleachScanner->GetParamString(ICamera::PARAM_LSM_POCKELS_LINE_0, pockelsLineBleachScanner[0], MAX_POCKELS_LINE_LEN);
		pBleachScanner->GetParamString(ICamera::PARAM_LSM_POCKELS_LINE_1, pockelsLineBleachScanner[1], MAX_POCKELS_LINE_LEN);
		pBleachScanner->GetParamString(ICamera::PARAM_LSM_POCKELS_LINE_2, pockelsLineBleachScanner[2], MAX_POCKELS_LINE_LEN);

		wstring pockelsLineBleachScannerWS[3] = {wstring(pockelsLineBleachScanner[0]), wstring(pockelsLineBleachScanner[1]), wstring(pockelsLineBleachScanner[2])};
		long pockelsLineRepeated = FALSE;
		for (long i = 0; i < MAX_POCKELS; i++)
		{
			if(pockelsLineBleachScannerWS[i].find(L"Dev1") != wstring::npos)
			{
				MessageBox(NULL,L"A pockels for the Bleach Scanner cannot use the same card as the Image Detector.",L"HW Trigger Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY | MB_ICONWARNING);
				//Close the shutter before returning
				if (TRUE == _digiShutterEnabled)
				{
					CloseShutter();
				}
				return FALSE;
			}
		}
	}

	//prebleach acquisition
	Dimensions d;
	PreCaptureProtocol(pCamera, index, subWell, preBleachingStream, preBleachingFrames, FALSE, channel, &d, &sp);
	AcquireBleaching::captureStatus = Capture(pCamera,currentT,preBleachingStream,preBleachingFrames,preBleachingInterval,d,&sp,FALSE);
	PostCaptureProtocol(preBleachingStream, currentT, preBleachingFrames, &sp);
	if(FALSE == AcquireBleaching::captureStatus)
	{
		//Close the shutter before returning
		if (TRUE == _digiShutterEnabled)
		{
			CloseShutter();
		}
		return FALSE;
	}
	currentT += preBleachingFrames;

	PreBleachProtocol(_pExp,pBleachScanner);

	//bleach
	long preCapStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE;
	long isPostBleachSetupDone = FALSE;
	char* buf = NULL;
	DWORD dwThread;
	HANDLE hThread, hCaptureThread;
	InitializeCriticalSection(&loadAccess);

	//initialize waveform params:
	for (int id = 0; id < MAX_BLEACH_PARAMS_CNT; id++)
	{
		if(AcquireBleaching::bleachParams[id] == NULL)
		{
			AcquireBleaching::bleachParams[id] = new GGalvoWaveformParams();
			AcquireBleaching::bleachParams[id]->bufferHandle = CreateMutex(NULL, false, NULL);
		}
	}

	//initialize events:
	if(hStopBleach)
	{
		CloseHandle(hStopBleach);
		hStopBleach = NULL;
	}

	if(hEventCapture)
	{
		CloseHandle(hEventCapture);
		hEventCapture = NULL;
	}

	if(hStopCapture)
	{
		CloseHandle(hStopCapture);
		hStopCapture = NULL;
	}

	if (hEventPostBleachSetupComplete)
	{
		CloseHandle(hEventPostBleachSetupComplete);
		hEventPostBleachSetupComplete = NULL;
	}
	hEventPostBleachSetupComplete = CreateEvent(0, TRUE, FALSE, 0);

	if(hEventStopLoad)
	{
		CloseHandle(hEventStopLoad);
		hEventStopLoad = NULL;
	}
	hEventStopLoad = CreateEvent(0, TRUE, FALSE, 0);

	//Galvo-Galvo bleach scanner only:
	ICamera::CameraType cameraType;
	ICamera::LSMType lsmType;
	double val;
	pBleachScanner->GetParam(ICamera::PARAM_CAMERA_TYPE,val);
	cameraType = (ICamera::CameraType)static_cast<long> (val);
	pBleachScanner->GetParam(ICamera::PARAM_LSM_TYPE,val);
	lsmType = (ICamera::LSMType)static_cast<long>(val);
	if ((ICamera::LSM != cameraType) || ((ICamera::LSMType::GALVO_GALVO != lsmType) && (ICamera::LSMType::STIMULATE_MODULATOR != lsmType)))
	{
		StringCbPrintfW(message, MSG_LENGTH, L"RunSample Bleach scanner is not Galvo-Galvo or Stimulator.");
		logDll->TLTraceEvent(ERROR_EVENT, 1, message);
		//Close the shutter before returning
		if (TRUE == _digiShutterEnabled)
		{
			CloseShutter();
		}
		return FALSE;
	}

	//Ensure the bleacher is ready to receive new settings
	//and setup for new waveforms
	//i.e. forcing the galvos to be at the park position as well as the pockels
	//this is necessary when GG prebleach imaging since the power may differ
	pBleachScanner->PostflightAcquisition(NULL);

	do
	{
		//request next capture status:
		preCapStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE;
		PreCaptureEventCheck(preCapStatus);

		//user request to stop or done:
		StopCaptureEventCheck(_stopStatus);
		if(1 == _stopStatus)
		{	
			PostflightSLMBleachScanner(pBleachScanner);
			//Close the shutter before returning
			if (TRUE == _digiShutterEnabled)
			{
				CloseShutter();
			}
			return FALSE;
		}
		if(PreCaptureStatus::PRECAPTURE_DONE == preCapStatus)
		{
			break;
		}

		//retrieve bleach param:
		while(WAIT_OBJECT_0 != WaitForSingleObject(bleachParams[0]->bufferHandle, TIMEOUT_MS))
		{
			StopCaptureEventCheck(_stopStatus);
			if(1 == _stopStatus)
			{
				SetEvent(hEventStopLoad);
			}
		}

		//load params to bleach scanner:
		pBleachScanner->SetParam(ICamera::PARAM_MULTI_FRAME_COUNT, bleachParams[0]->CycleNum);
		pBleachScanner->SetParam(ICamera::PARAM_LSM_SCANMODE, static_cast<double>(ScanMode::BLEACH_SCAN));
		pBleachScanner->SetParam(ICamera::PARAM_TRIGGER_MODE, trigMode);
		pBleachScanner->SetParam(ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS, preCapStatus);
		pBleachScanner->SetParamString(ICamera::PARAM_LSM_WAVEFORM_PATH_NAME, bleachParams[0]->WaveFileName);

		ReleaseMutex(bleachParams[0]->bufferHandle);

		//send bleach scanner:
		pBleachScanner->PreflightAcquisition(NULL);

		if(FALSE == pBleachScanner->SetupAcquisition(NULL))
		{
			PostflightSLMBleachScanner(pBleachScanner);
			//Close the shutter before returning
			if (TRUE == _digiShutterEnabled)
			{
				CloseShutter();
			}
			return FALSE;
		}

		hEventBleach = CreateEvent(0, TRUE, FALSE, 0);
		hStopBleach = CreateEvent(0, TRUE, FALSE, 0);

		hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) BleachThreadProc, (LPVOID)pBleachScanner, 0, &dwThread );

		//start post bleach capture if necessary, invoke once only:
		long enableHWtrigger = (ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == postTrigMode);
		if (FALSE == isPostBleachSetupDone)
		{
			if (enableHWtrigger || enableSimultaneousBleachingAndImaging)
			{
				//setup data structures before postbleach acquisition 1:
				PreCaptureProtocol(pCamera, index, subWell, postBleachingStream1, postBleachingFrames1, enableHWtrigger, channel, &d, &sp);

				CaptureParams cp;
				cp.acqB = this;
				cp.pCam = pCamera;
				cp.d = d;
				cp.sp = sp;
				cp.currentT = currentT;
				cp.streaming = postBleachingStream1;
				cp.timeInterval = postBleachingInterval1;
				cp.simultaneous = enableSimultaneousBleachingAndImaging;

				//start the capture immediately
				//postbleach acquisition 1
				hStopCapture = CreateEvent(0, TRUE, FALSE, 0);
				hEventCapture = CreateEvent(0, TRUE, FALSE, 0);

				hCaptureThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) CaptureThreadProc, (LPVOID)&cp, 0, &dwThread );
			}
			else
			{
				//if not HW trigger then the setup can happen after bleaching is complete
				//set event to allow bleaching to continue
				SetEvent(hEventPostBleachSetupComplete);
			}	
		}
		//set flag to invoke post bleach capture setup once only:
		isPostBleachSetupDone = TRUE;

		while(WaitForSingleObject( hEventBleach, 10) != WAIT_OBJECT_0)
		{
			StopCaptureEventCheck(_stopStatus);
			if(1 == _stopStatus)
			{
				pBleachScanner->PostflightAcquisition(NULL);
				SetEvent(hStopBleach);
			}
		};

		SAFE_DELETE_HANDLE(hThread);
		SAFE_DELETE_HANDLE(hEventBleach);
		SAFE_DELETE_HANDLE(hStopBleach);

		//done bleach: trigger first will finish all after first trigger
		if(ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == trigMode)
		{
			trigMode = ICamera::SW_MULTI_FRAME;
		}

		//error in bleacher will reset capture status:
		double bleacherCapStatus = 0;
		GetCameraParamDouble(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS, bleacherCapStatus);

		if((PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE == preCapStatus) || ((int)(PreCaptureStatus::PRECAPTURE_BLEACHER_ERROR) == (int)bleacherCapStatus))
		{
			//finished bleaching:
			preCapStatus = PreCaptureStatus::PRECAPTURE_DONE;
			if (ICamera::HW_MULTI_FRAME_TRIGGER_FIRST != postTrigMode)
			{
				PreCaptureEventCheck(preCapStatus);
			}
		}

		//check user stop request:
		StopCaptureEventCheck(_stopStatus);
		if(1 == _stopStatus)
		{
			PostflightSLMBleachScanner(pBleachScanner);
			if(NULL != hEventCapture)
			{
				SetEvent(hStopCapture);
			}
			//Close the shutter before returning
			if (TRUE == _digiShutterEnabled)
			{
				CloseShutter();
			}
			//let it return outside the while loop:
			preCapStatus = PreCaptureStatus::PRECAPTURE_DONE;
		}
	}
	while(PreCaptureStatus::PRECAPTURE_DONE != preCapStatus);

	PostflightSLMBleachScanner(pBleachScanner);

	PostBleachProtocol(_pExp,pCamera);

	//PMT settings may have changed during bleaching. 
	//Set the PMTs to the acquisition state after bleaching is done
	SetPMT();

	//wait for post1Capture if invoked:
	if(NULL != hEventCapture)
	{
		while(WaitForSingleObject(hEventCapture, 10) != WAIT_OBJECT_0)
		{
			StopCaptureEventCheck(_stopStatus);
			if(1 == _stopStatus)
			{
				SetEvent(hStopCapture);
			}
		};
		SAFE_DELETE_HANDLE(hCaptureThread);
		SAFE_DELETE_HANDLE(hEventCapture);
		SAFE_DELETE_HANDLE(hStopCapture);

		PostCaptureProtocol(postBleachingStream1, currentT, postBleachingFrames1, &sp);
	}
	CloseHandle(hEventPostBleachSetupComplete);
	hEventPostBleachSetupComplete = NULL;

	if ((FALSE == enableSimultaneousBleachingAndImaging) && (ICamera::HW_MULTI_FRAME_TRIGGER_FIRST != postTrigMode))
	{
		//postbleach acquisition 1
		PreCaptureProtocol(pCamera, index, subWell, postBleachingStream1, postBleachingFrames1, FALSE, channel, &d, &sp);
		AcquireBleaching::captureStatus = Capture(pCamera,currentT,postBleachingStream1,postBleachingFrames1,postBleachingInterval1,d,&sp,FALSE);
		PostCaptureProtocol(postBleachingStream1, currentT, postBleachingFrames1, &sp);
	}

	//postbleach acquisition 2
	if(AcquireBleaching::captureStatus)
	{
		currentT += postBleachingFrames1;

		PreCaptureProtocol(pCamera, index, subWell, postBleachingStream2, postBleachingFrames2, FALSE, channel, &d, &sp);
		AcquireBleaching::captureStatus = Capture(pCamera,currentT,postBleachingStream2,postBleachingFrames2,postBleachingInterval2,d,&sp,FALSE);
		PostCaptureProtocol(postBleachingStream2, currentT, postBleachingFrames2, &sp);
	}

	//Close the shutter before returning
	if (TRUE == _digiShutterEnabled)
	{
		CloseShutter();
	}

	//stop the scanner before processing the images
	ScannerEnable(0,FALSE);
	ScannerEnable(1,FALSE);

	//notice user of converting (or combining) files
	StringCbPrintfW(message, MSG_LENGTH, L"Preparing output files, please wait ...");
	CallInformMessage(message);
	_pExp->GetPhotoBleachingAttr("rawOption", val);
	if((long)val == 0)
	{
		ConvertRawFiles(&sp, preBleachingFrames, postBleachingFrames1, postBleachingFrames2, AcquireBleaching::captureStatus);
	}
	else
	{
		CatRawFiles(&sp, preBleachingFrames, postBleachingFrames1, postBleachingFrames2);
	}

	//the final image update after conversion
	CallSaveImage(preBleachingFrames+postBleachingFrames1+postBleachingFrames2, TRUE);

	return TRUE;
}

string WStringToStringABL(wstring ws)
{	
	size_t origsize = wcslen(ws.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t convertedChars = 0;
	char nstring[newsize];
	wcstombs_s(&convertedChars, nstring, origsize, ws.c_str(), _TRUNCATE);

	string str(nstring);

	StringCbPrintfW(message,MSG_LENGTH,L"ExperimentManager WStringToStringABL: %S",nstring);
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	return str;
}

long AcquireBleaching::GetConfigurableOption(string settingFile, string element, string attr, long& val)
{
	long ret = FALSE;
	wstring asPath = ResourceManager::getInstance()->GetApplicationSettingsPath();
	string settingFilePath = ConvertWStringToString(asPath) + settingFile + ".xml";
	// load the ApplicationSettings.xml 
	ticpp::Document doc(settingFilePath.c_str());
	doc.LoadFile();
	// parse through all children
	ticpp::Iterator<ticpp::Element> child;
	for(child = child.begin(doc.FirstChildElement()); child != child.end(); child++)
	{
		std::string strName;
		std::string strValue;
		child->GetValue(&strName);

		if (!element.compare(strName))
		{
			// now parse through all the attributes of this fruit
			ticpp::Iterator< ticpp::Attribute > attribute;
			for(attribute = attribute.begin(child.Get()); attribute != attribute.end(); attribute++)
			{
				attribute->GetName(&strName);
				attribute->GetValue(&strValue);
				if(!attr.compare(strName))
				{
					val = stoi(strValue);
					ret = TRUE;
				}
			}
		}
	}

	return ret;	
}

void AcquireBleaching::SaveTIFFChannels(SaveParams *sp, long size, char *buffer, long j, string &strOme, string &timeStamp, double dt)
{
	long doOME = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"OMETIFFTag",L"value", FALSE);
	long doCompression = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"TIFFCompressionEnable",L"value", FALSE);

	SaveTIFFChannels(sp, size, buffer, j, strOme, timeStamp, dt, doOME, doCompression);
}

void AcquireBleaching::SaveTIFFChannels(SaveParams *sp, long size, char *buffer, long j, string &strOme, string &timeStamp, double dt, long doOME, long doCompression)
{
	long channel = 0;

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	imgNameFormat << L"%s%s%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";

	PhysicalSize physicalSize;	// unit: um
	double res = floor(sp->umPerPixel*1000+0.5)/1000;	// keep 2 figures after decimal point, that is why 100 (10^2) is multiplied
	physicalSize.x = res;
	physicalSize.y = res;
	physicalSize.z = _zstageStepSize;

	for(long i=0; i<sp->colorChannels; i++)
	{
		wchar_t filePathAndName[_MAX_PATH];
		const int COLOR_MAP_SIZE = 65536;
		unsigned short rlut[COLOR_MAP_SIZE];
		unsigned short glut[COLOR_MAP_SIZE];
		unsigned short blut[COLOR_MAP_SIZE];

		const int COLOR_MAP_BIT_DEPTH_TIFF = 8;

		wchar_t drive[_MAX_DRIVE];
		wchar_t dir[_MAX_DIR];
		wchar_t fname[_MAX_FNAME];
		wchar_t ext[_MAX_EXT];
		GetLookUpTables(rlut, glut, blut, sp->red[i], sp->green[i], sp->blue[i], sp->bp[i], sp->wp[i],COLOR_MAP_BIT_DEPTH_TIFF);

		_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

		StringCbPrintfW(filePathAndName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir,_wavelengthName[i].c_str(),sp->index,sp->subWell,1,j);

		logDll->TLTraceEvent(VERBOSE_EVENT,1,filePathAndName);

		for (long ch=0;ch<sp->bufferChannels;ch++)
		{
			if(0 == _wavelengthName[i].compare(AcquireFactory::bufferChannelName[ch]))
			{	channel = ch;	}
		}

		if(_wavelengthName[i].size() > 0)
		{
			long bufferOffset = channel*sp->width*sp->height*2;

			if (TRUE == doOME)
			{
				SaveTIFF(filePathAndName,buffer+bufferOffset,sp->width,sp->height,rlut,glut,blut, sp->umPerPixel,sp->colorChannels, size, 1, 0, channel, j, 0, &timeStamp, dt, &strOme, physicalSize, doCompression);
			}
			else
			{
				SaveTIFFWithoutOME(filePathAndName,buffer+bufferOffset,sp->width,sp->height,rlut,glut,blut, sp->umPerPixel,sp->colorChannels, size, 1, 0, channel, j, 0, &timeStamp, dt,doCompression);
			}

			//SaveTIFF(filePathAndName,buffer+bufferOffset,sp->width,sp->height,rlut,glut,blut, sp->umPerPixel,sp->colorChannels, size, 1, 0, channel, j, 0, &timeStamp, dt, &strOme, physicalSize);
		}
	}
}

long AcquireBleaching::SaveRawFiles(SaveParams *sp, wchar_t *name, long size, long groupFrames, long startIndex,long totalFrames)
{
	int fh;
	char * buffer = new char[size];

	if (0 != _wsopen_s(&fh,name,_O_RDONLY |O_APPEND|_O_BINARY,_SH_DENYNO,_S_IREAD | _S_IWRITE ))
	{
		return FALSE;
	}

	long readSize = 0;
	for(long j = 0; j<groupFrames; j++)
	{
		if (-1 == _lseeki64(fh, ((ULONG64)j) * ((ULONG64)size), SEEK_SET))
		{
			delete[] buffer;
			_close(fh);
			return FALSE;
		}

		readSize = _read(fh, buffer, size);

		if (readSize != size)
		{
			delete[] buffer;
			_close(fh);
			return FALSE;
		}

		string strOme = uUIDSetup(sp, sp->colorChannels, totalFrames, 1, sp->index, sp->subWell);
		string timeStamp = "";
		double dt = 0;

		SaveTIFFChannels(sp, size, buffer, j+startIndex, strOme, timeStamp, dt);

		//update progress:
		if(j+startIndex < totalFrames)
		{
			CallSaveTImage(j+startIndex);
			CallSaveImage(j+startIndex, FALSE);
		}
	}

	if(NULL != buffer)
	{
		delete[] buffer;
	}

	_close(fh);

	_wremove(name);

	return TRUE;
}

long AcquireBleaching::CatRawFiles(SaveParams *sp, long preBleachingFrames, long postBleachingFrames1, long postBleachingFrames2)
{
	wchar_t name[_MAX_PATH];
	long compactSaveEnabled = 0;
	GetConfigurableOption("ApplicationSettings", "RawFileOptions", "saveEnabledChannelsOnly", compactSaveEnabled);

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream nameFormat;
	nameFormat << L"%s%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";

	if(sp->lsmChannels == 1 || sp->lsmChannels == 2 || sp->lsmChannels == 4 || sp->lsmChannels == 8 || sp->lsmChannels == 15 || compactSaveEnabled == 0)
	{

		StringCbPrintfW(name, _MAX_PATH, nameFormat.str().c_str(), _rawBaseName, 1);

		std::ofstream of(name, std::ios_base::binary | std::ios_base::app);
		of.seekp(0, std::ios_base::end);

		if (preBleachingFrames > 0)
		{
			StringCbPrintfW(name, _MAX_PATH, nameFormat.str().c_str(), _rawBaseName, preBleachingFrames + 1);
			std::ifstream if_1(name, std::ios_base::binary);
			of << if_1.rdbuf();
			if_1.close();
			_wremove(name);
		}

		if (postBleachingFrames1 > 0)
		{
			StringCbPrintfW(name, _MAX_PATH, nameFormat.str().c_str(), _rawBaseName, preBleachingFrames + postBleachingFrames1 + 1);
			std::ifstream if_2(name, std::ios_base::binary);
			of << if_2.rdbuf();
			if_2.close();

			_wremove(name);
		}

		of.close();
	}
	else
	{
		StringCbPrintfW(name, _MAX_PATH, L"%s.raw", _rawBaseName);
		std::ofstream of(name, std::ios_base::binary);

		int transFrames = (preBleachingFrames < _capturedImageID) ? preBleachingFrames : _capturedImageID;
		if(0 < preBleachingFrames)
		{
			StringCbPrintfW(name, _MAX_PATH, nameFormat.str().c_str(), _rawBaseName, 1);
			std::ifstream if_0(name, std::ios_base::binary);
			TransferRawFileByFrame(sp, of, if_0, transFrames);
			if_0.close();
			_wremove(name);		
		}
		transFrames = ((preBleachingFrames + postBleachingFrames1) < _capturedImageID) ? postBleachingFrames1 : (_capturedImageID - preBleachingFrames);
		if(0 < postBleachingFrames1)
		{
			StringCbPrintfW(name, _MAX_PATH, nameFormat.str().c_str(), _rawBaseName, preBleachingFrames + 1);
			std::ifstream if_1(name, std::ios_base::binary);
			TransferRawFileByFrame(sp, of, if_1, transFrames);
			if_1.close();
			_wremove(name);
		}
		transFrames = ((preBleachingFrames + postBleachingFrames1 + postBleachingFrames2) <= _capturedImageID) ? postBleachingFrames2 : (_capturedImageID - preBleachingFrames - postBleachingFrames1);
		if(0 < postBleachingFrames2)
		{
			StringCbPrintfW(name, _MAX_PATH, nameFormat.str().c_str(), _rawBaseName, preBleachingFrames + postBleachingFrames1 + 1);
			std::ifstream if_2(name, std::ios_base::binary);
			TransferRawFileByFrame(sp, of, if_2, transFrames);
			if_2.close();
			_wremove(name);
		}
		of.close(); 
	} 

	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
	wchar_t fullTifPath[_MAX_PATH];
	StringCbPrintfW(fullTifPath, _MAX_PATH, L"%s%s\\*.tif",drive,dir);
	wstring ws(fullTifPath); 
	string command = "del /Q ";
	std::system(command.append(ConvertWStringToString(ws)).c_str());

	return TRUE;
}

long AcquireBleaching::TransferRawFileByFrame(SaveParams *sp, ofstream& os, ifstream& is, int frameNum)
{
	long ret = true;
	int size = sp->width * sp->height * sizeof(USHORT);
	char* buf = (char*)malloc(size);
	for(int j = 0; j < frameNum; j++)
	{
		for(int i = 0; i < 4; i++)
		{
			if((0x1<<i) & ((UCHAR)sp->lsmChannels))
			{
				is.read(buf, size);
				os.write(buf, size);
			}
			else
			{
				is.seekg(size, is.cur);
			}
		}
	}

	free(buf);
	return ret;
}

long AcquireBleaching::ConvertRawFiles(SaveParams *sp, long preBleachingFrames,long postBleachingFrames1,long postBleachingFrames2, long skipMessage)
{
	wchar_t name[_MAX_PATH];
	if(!skipMessage)
	{
		if(IDNO == MessageBox(NULL,L"Would like to save available images?",L"Save Experiment Files",MB_YESNO | MB_SETFOREGROUND | MB_ICONWARNING | MB_SYSTEMMODAL))
			return FALSE;
	}
	long size = sp->width * sp->height * sp->bufferChannels * 2;
	long groupFrames = (preBleachingFrames < _capturedImageID) ? preBleachingFrames : _capturedImageID;
	long totalFrames = preBleachingFrames + postBleachingFrames1 + postBleachingFrames2;

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream nameFormat;
	nameFormat << L"%s%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";

	if (groupFrames > 0)
	{
		StringCbPrintfW(name ,_MAX_PATH,nameFormat.str().c_str(),_rawBaseName,1);
		if(FALSE == SaveRawFiles(sp, name, size, groupFrames,1,totalFrames))
		{
			StringCbPrintfW(message,MSG_LENGTH,L"ConvertRawFiles file not available %s",name);
			logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		}
	}

	groupFrames = ((preBleachingFrames + postBleachingFrames1) < _capturedImageID) ? postBleachingFrames1 : (_capturedImageID - preBleachingFrames);

	if (groupFrames > 0)
	{
		StringCbPrintfW(name ,_MAX_PATH,nameFormat.str().c_str(),_rawBaseName,preBleachingFrames+1);
		if(FALSE == SaveRawFiles(sp, name, size, groupFrames,preBleachingFrames+1,totalFrames))
		{
			StringCbPrintfW(message,MSG_LENGTH,L"ConvertRawFiles file not available %s",name);
			logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		}
	}

	groupFrames = ((preBleachingFrames + postBleachingFrames1 + postBleachingFrames2) <= _capturedImageID) ? postBleachingFrames2 : (_capturedImageID - preBleachingFrames - postBleachingFrames1);

	if (groupFrames > 0)
	{
		StringCbPrintfW(name ,_MAX_PATH,nameFormat.str().c_str(),_rawBaseName,preBleachingFrames + postBleachingFrames1+1);
		if(FALSE == SaveRawFiles(sp, name, size, groupFrames,preBleachingFrames + postBleachingFrames1+1,totalFrames))
		{
			StringCbPrintfW(message,MSG_LENGTH,L"ConvertRawFiles file not available %s",name);
			logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		}
	}
	return TRUE;
}

void AcquireBleaching::SavePreviewImage(SaveParams *sp, long tFrameOneBased, char * buffer)
{
	string timeStamp = "";
	string strOme = "";
	double dt = 0;
	long size = sp->totFrames;
	FrameInfo frameInfo;
	frameInfo.bufferType = BufferType::INTENSITY;
	frameInfo.imageWidth = sp->width;
	frameInfo.imageHeight = sp->height;
	//TODO: add DFLIM capabilities to Bleaching acquisition
	//send buffer to statsManager for ROIStats calculation and storage
	StatsManager::getInstance()->ComputeStats( (unsigned short*)buffer, 
		frameInfo,
		sp->lsmChannels,FALSE,TRUE,FALSE);

	long doOME = FALSE;//no OME for preview images
	long doCompression = FALSE;//Don't Compress Preview Images

	SaveTIFFChannels(sp, size, buffer, tFrameOneBased, strOme, timeStamp, dt, doOME, doCompression); 
}

string AcquireBleaching::uUIDSetup(SaveParams *sp, long bufferChannels, long timePoints, long zstageSteps, long index, long subWell)
{
	wchar_t filePathAndName[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	string strOME;

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	imgNameFormat << L"%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";

	long bufferOffsetIndex =0;
	for(long t=0; t<timePoints; t++)
	{
		for(long z=0; z<zstageSteps; z++)
		{
			for(long c=0; c<bufferChannels; c++)
			{	
				RPC_WSTR guidStr = 0x00;

				GUID *pguid = 0x00;

				pguid = new GUID;

				CoCreateGuid(pguid);

				OLECHAR* bstrGuid;
				StringFromCLSID(*pguid, &bstrGuid);

				wstring ws(bstrGuid);

				//remove the curly braces at the end and start of the guid
				ws.erase(ws.size()-1,1);
				ws.erase(0,1);

				string strGuid = ConvertWStringToString(ws);

				ostringstream ss;
				ss << "<TiffData" << " FirstT=\"" << t << "\"" << " FirstZ=\"" << z << "\"" << " FirstC=\"" << c << "\">" ;

				_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

				StringCbPrintfW(filePathAndName,_MAX_PATH,imgNameFormat.str().c_str(),_wavelengthName[c].c_str(),index,subWell,z+1,t+1);

				wstring wsPath(filePathAndName);
				string strFilePathAndName =  ConvertWStringToString(wsPath);
				ss << "<UUID FileName=\"" << strFilePathAndName.c_str() << "\">" << "urn:uuid:" << strGuid.c_str()  << "</UUID>" << "</TiffData>"; 
				strOME += ss.str();
				// ensure memory is freed
				::CoTaskMemFree(bstrGuid);

				delete pguid; 
			}
		}
	}

	return strOME;
}

long AcquireBleaching::SaveTimingToExperimentFolder()
{
	if(_path.empty())
	{
		return FALSE;
	}

	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

	wchar_t timingFile[_MAX_PATH];
	StringCbPrintfW(timingFile,_MAX_PATH, L"%s%stiming.txt", drive, dir);

	string timeFile = ConvertWStringToString(timingFile);
	_acquireSaveInfo->getInstance()->SaveTimingToFile(timeFile);

	return TRUE;
}

//void AcquireBleaching::SaveData( SaveParams *sp, long currentT)
//{
//	string wavelengthName;
//	long size = sp->totFrames;
//
//	string strOme = uUIDSetup(sp, sp->colorChannels, size, 1, sp->index, sp->subWell);
//	PhysicalSize physicalSize;	// unit: um
//	double res = floor(sp->umPerPixel*1000+0.5)/1000;	// keep 2 figures after decimal point, that is why 100 (10^2) is multiplied
//	physicalSize.x = res;
//	physicalSize.y = res;
//	physicalSize.z = _zstageStepSize;
//
//	long doOME = TRUE;
//	long doCompression = TRUE;
//	GetTIFFConfiguration(doOME,doCompression);						
//
//	//#pragma omp parallel for
//	for(long j=0; j<size; j++)
//	{
//		char * buffer = ImageManager::getInstance()->GetImagePtr(sp->imageID,0,0,0,j);
//
//		string timeStamp = _acquireSaveInfo->getInstance()->RemoveTimestamp();
//		double dt= _acquireSaveInfo->getInstance()->RemoveTimingInfo();
//
//		for(long i=0; i<sp->colorChannels; i++)
//		{
//			wchar_t filePathAndName[_MAX_PATH];
//			const int COLOR_MAP_SIZE = 65536;
//			unsigned short rlut[COLOR_MAP_SIZE];
//			unsigned short glut[COLOR_MAP_SIZE];
//			unsigned short blut[COLOR_MAP_SIZE];
//
//			const int COLOR_MAP_BIT_DEPTH_TIFF = 8;
//
//			wchar_t drive[_MAX_DRIVE];
//			wchar_t dir[_MAX_DIR];
//			wchar_t fname[_MAX_FNAME];
//			wchar_t ext[_MAX_EXT];
//			GetLookUpTables(rlut, glut, blut, sp->red[i], sp->green[i], sp->blue[i], sp->bp[i], sp->wp[i],COLOR_MAP_BIT_DEPTH_TIFF);
//
//			_wsplitpath_s(sp->path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
//
//			StringCbPrintfW(filePathAndName,_MAX_PATH,L"%s%s%S_%04d_%04d_%04d_%04d.tif",drive,dir,sp->wavelengthName[i].c_str(),sp->index,sp->subWell,1,j+currentT);
//
//			logDll->TLTraceEvent(VERBOSE_EVENT,1,filePathAndName);
//
//			if(sp->wavelengthName[i].size() > 0)
//			{
//				long bufferOffset = i*sp->width*sp->height*2;
//
//				if (TRUE == doOME)
//				{
//					SaveTIFF(filePathAndName,buffer+bufferOffset,sp->width,sp->height,rlut,glut,blut, sp->umPerPixel,sp->colorChannels, size, 1, 0, 0, j+currentT-1, 0, &timeStamp, dt, &strOme, physicalSize, doCompression);
//				}		
//				else
//				{
//					SaveTIFFWithoutOME(filePathAndName,buffer+bufferOffset,sp->width,sp->height,rlut,glut,blut, sp->umPerPixel,sp->colorChannels, size, 1, 0, 0, j+currentT-1, 0, &timeStamp, dt, doCompression);
//				}	
//
//				//SaveTIFF(filePathAndName,buffer+bufferOffset,sp->width,sp->height,rlut,glut,blut, sp->umPerPixel,sp->colorChannels, size, 1, 0, 0, j+currentT-1, 0, &timeStamp, dt, &strOme);
//			}
//		}
//
//		ImageManager::getInstance()->UnlockImagePtr(sp->imageID,0,0,0,j);
//
//		if(j<(size-1))
//		{
//			//update progress T to observer.
//			CallSaveTImage(j+1);
//			CallSaveImage(j+1, TRUE);
//		}
//		long status;
//		StopCaptureEventCheck(status);
//
//		//user has asked to stop the capture
//		if(1 == status)
//			break;
//	}
//
//}

long AcquireBleaching::ScannerEnable(long cameraOrBleachScanner, long enable)
{
	return ScannerEnableProc(cameraOrBleachScanner,enable);
}

long AcquireBleaching::SetPMT()
{
	return SetPMTProc( _pExp);
}

void AcquireBleaching::PreBleachProtocol(IExperiment  *exp, ICamera * pBleachScanner)
{
	long photoBleachingEnable,laserPositiion,durationMS, bleachTrigger, preBleachingFrames, bleachWidth, bleachHeight,bleachOffsetX,bleachOffsetY, bleachingFrames, bleachFieldSize, postBleachingFrames1, postBleachingFrames2,preBleachingStream,postBleachingStream1,postBleachingStream2,powerEnable,laserEnable,bleachQuery,bleachPostTrigger,enableSimultaneousBleachingAndImaging,pmtEnableDuringBleach[4];
	double powerPosition,preBleachingInterval,postBleachingInterval1,postBleachingInterval2;

	_pExp->GetPhotobleaching(photoBleachingEnable, laserPositiion, durationMS, powerPosition, bleachWidth,bleachHeight,bleachOffsetX,bleachOffsetY, bleachingFrames, bleachFieldSize, bleachTrigger,preBleachingFrames, preBleachingInterval,preBleachingStream, postBleachingFrames1, postBleachingInterval1,postBleachingStream1, postBleachingFrames2, postBleachingInterval2,postBleachingStream2,powerEnable,laserEnable,bleachQuery,bleachPostTrigger,enableSimultaneousBleachingAndImaging,pmtEnableDuringBleach[0],pmtEnableDuringBleach[1],pmtEnableDuringBleach[2],pmtEnableDuringBleach[3]);

	if(TRUE == laserEnable)
	{
		IDevice *pLaser = NULL;

		pLaser = GetDevice(SelectedHardware::SELECTED_LASER1);

		if(NULL == pLaser)
		{	
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create Laser for PreBleachProtocol");
		}
		else
		{
			SetDevicePosition(pLaser,IDevice::PARAM_LASER1_POS,laserPositiion,TRUE);
		}
	}

	long enable,type;
	double start,stop,zeroOffset;
	string path;

	if(TRUE == powerEnable)
	{
		IDevice *pPowerReg = NULL;

		pPowerReg = GetDevice(SelectedHardware::SELECTED_POWERREGULATOR);


		double p0 = 0, p1 = 0, p2 = 0, p3 = 0,p4= 0,p5=0;

		exp->GetPower(enable,type,start,stop,path,zeroOffset,0);
		p4 = start;
		exp->GetPower(enable,type,start,stop,path,zeroOffset,1);
		p5 = start;

		double param = 0;

		if(pBleachScanner->GetParam(ICamera::PARAM_LSM_POCKELS_CONNECTED_0,param) && (1 == static_cast<long>(param)))
		{
			//set the pockels power
			for(long i=0; i<4; i++)
			{
				long param;
				switch(i)
				{
				case 0: param = ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0; break;
				case 1: param = ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1; break;
				case 2: param = ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2; break;
				case 3: param = ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2; break;
				}

				pBleachScanner->SetParam(param, powerPosition);				
			}
			p0 = powerPosition;
			p1 = powerPosition;
			p2 = powerPosition;
			p3 = powerPosition;
		}
		else if(NULL != pPowerReg)
		{
			SetDevicePosition(pPowerReg,IDevice::PARAM_POWER_POS,powerPosition,TRUE);
			p4 = powerPosition;
		}

		//update progress Z to observer.
		CallSaveZImage(1,p0,p1,p2,p3,p4,p5);
	}	

	//set the enable state of the pmts based on the user input
	SetDeviceParamLong(SelectedHardware::SELECTED_PMT1,IDevice::PARAM_PMT1_ENABLE,pmtEnableDuringBleach[0],TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_PMT2,IDevice::PARAM_PMT2_ENABLE,pmtEnableDuringBleach[1],TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_PMT3,IDevice::PARAM_PMT3_ENABLE,pmtEnableDuringBleach[2],TRUE);
	SetDeviceParamLong(SelectedHardware::SELECTED_PMT4,IDevice::PARAM_PMT4_ENABLE,pmtEnableDuringBleach[3],TRUE);	

	if(1 == bleachQuery)
	{ 
		MessageBox(NULL,L"The scanner is now ready to bleach. Make your manual light path adjustments now. Then press OK",L"Ready to Bleach",MB_OK);
	}
}

void AcquireBleaching::PostBleachProtocol(IExperiment  *exp,ICamera *pCamera)
{
	long photoBleachingEnable,laserPositiion,durationMS, bleachTrigger, preBleachingFrames, bleachWidth, bleachHeight,bleachOffsetX,bleachOffsetY, bleachingFrames, bleachFieldSize, postBleachingFrames1, postBleachingFrames2,preBleachingStream,postBleachingStream1,postBleachingStream2,powerEnable,laserEnable,bleachQuery,bleachPostTrigger,enableSimultaneousBleachingAndImaging,pmtEnableDuringBleach[4];
	double powerPosition,preBleachingInterval,postBleachingInterval1,postBleachingInterval2;

	exp->GetPhotobleaching(photoBleachingEnable, laserPositiion, durationMS, powerPosition, bleachWidth,bleachHeight,bleachOffsetX,bleachOffsetY, bleachingFrames, bleachFieldSize, bleachTrigger,preBleachingFrames, preBleachingInterval,preBleachingStream, postBleachingFrames1, postBleachingInterval1,postBleachingStream1, postBleachingFrames2, postBleachingInterval2,postBleachingStream2,powerEnable,laserEnable,bleachQuery,bleachPostTrigger,enableSimultaneousBleachingAndImaging,pmtEnableDuringBleach[0],pmtEnableDuringBleach[1],pmtEnableDuringBleach[2],pmtEnableDuringBleach[3]);

	if(TRUE == laserEnable)
	{
		long multiphotonEnable,multiphotonPos,multiphotonSeqEnable,multiphotonSeqPos1,multiphotonSeqPos2;

		exp->GetMultiPhotonLaser(multiphotonEnable,multiphotonPos,multiphotonSeqEnable,multiphotonSeqPos1,multiphotonSeqPos2);

		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1,IDevice::PARAM_LASER1_POS,multiphotonPos,TRUE);
	}

	//in the event that the power control is shared by the image/bleach scanners
	//reset the image scanners power	
	if(TRUE == powerEnable)
	{
		long enable,type,blankPercent;
		double start,stop,zeroOffset, powerRegPosition, powerReg2Position;
		string path;


		double p0 = 0, p1 = 0, p2 = 0, p3 = 0, p4 = 0, p5 = 0;

		exp->GetPower(enable,type,start,stop,path,zeroOffset,0);
		p4 = start;
		exp->GetPower(enable,type,start,stop,path,zeroOffset,1);
		p5 = start;

		double param = 0;
		if(pCamera->GetParam(ICamera::PARAM_LSM_POCKELS_CONNECTED_0,param) && (1 == static_cast<long>(param)))
		{
			//set the pockels power
			for(long i=0; i<4; i++)
			{
				long param1, param2;
				switch(i)
				{
				case 0: 
					param1 = ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0; 
					param2 = ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0;
					break;
				case 1:
					param1 = ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1; 
					param2 = ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1;
					break;
				case 2:
					param1 = ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2; 
					param2 = ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2;
					break;
				case 3:
					param1 = ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3; 
					param2 = ICamera::PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3;
					break;
				}

				_pExp->GetPockels(i,type,start,stop,path,blankPercent);

				pCamera->SetParam(param1, start);
				pCamera->SetParam(param2, blankPercent);

				switch(i)
				{
				case 0: p0 = start; break;
				case 1: p1 = start; break;
				case 2: p2 = start; break;
				case 3: p3 = start; break;
				}
			}
		}

		GetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR,IDevice::PARAM_POWER_POS_CURRENT,powerRegPosition);
		if(p4 != powerRegPosition)
		{
			SetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR,IDevice::PARAM_POWER_POS,p4,TRUE);
		}
		GetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR,IDevice::PARAM_POWER_POS_CURRENT,powerReg2Position);
		if(p5 != powerReg2Position)
		{
			SetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR2,IDevice::PARAM_POWER_POS,p5,TRUE);
		}
		//update progress Z to observer.
		CallSaveZImage(1,p0,p1,p2,p3,p4,p5);
	}	

	if(1 == bleachQuery)
	{
		MessageBox(NULL,L"The scanner has completed the bleach. Make your manual light path adjustments now. Then press OK", L"Completed Bleach",MB_OK);
	}

	SetPMTProc(exp);
}

void AcquireBleaching::PostflightSLMBleachScanner(ICamera * pBleachScanner)
{
	//stop SLM before bleach scanner:
	IDevice* slm = GetDevice(SelectedHardware::SELECTED_SLM);
	if(slm)
	{
		slm->PostflightPosition();
	}

	//bleaching is done, stop bleach scanner:
	pBleachScanner->PostflightAcquisition(NULL);
}

DllExport_RunSample ReleaseBleachParams()
{
	for (int i = 0; i < MAX_BLEACH_PARAMS_CNT; i++)
	{
		if(AcquireBleaching::bleachParams[i] != NULL)
		{
			if(AcquireBleaching::bleachParams[i]->analogPockelSize > 0)
			{
				free(AcquireBleaching::bleachParams[i]->GalvoWaveformPockel);
			}
			if(AcquireBleaching::bleachParams[i]->analogXYSize > 0)
			{
				free(AcquireBleaching::bleachParams[i]->GalvoWaveformXY);
			}
			if(AcquireBleaching::bleachParams[i]->digitalSize > 0)
			{
				free(AcquireBleaching::bleachParams[i]->DigBufWaveform);
			}
			if(NULL != AcquireBleaching::bleachParams[i]->bufferHandle)
			{
				CloseHandle(AcquireBleaching::bleachParams[i]->bufferHandle);
			}
			delete AcquireBleaching::bleachParams[i];
			AcquireBleaching::bleachParams[i] = NULL;
		}
	}

	//also release memory for comparing ROIs:
	if(AcquireBleaching::bleachMem != NULL)
	{
		free(AcquireBleaching::bleachMem);
		AcquireBleaching::bleachMem = NULL;
		AcquireBleaching::bleachMemSize = 0;
	}
	return TRUE;
}

DllExport_RunSample SetBleachWaveformFile(const wchar_t* bleachH5PathName, int CycleNum)
{
	if(NULL != AcquireBleaching::bleachParams[0]->bufferHandle)
	{
		if(WAIT_OBJECT_0 != WaitForSingleObject(AcquireBleaching::bleachParams[0]->bufferHandle, TIMEOUT_MS))
		{return FALSE;}
	}

	AcquireBleaching::bleachParams[0]->Scanmode = static_cast<long>(ScanMode::BLEACH_SCAN);
	AcquireBleaching::bleachParams[0]->CycleNum = CycleNum;
	AcquireBleaching::bleachParams[0]->lastLoaded = TRUE;	
	wcsncpy_s (AcquireBleaching::bleachParams[0]->WaveFileName, bleachH5PathName,_MAX_PATH);

	ReleaseMutex(AcquireBleaching::bleachParams[0]->bufferHandle);
	return TRUE;
}

DllExport_RunSample LoadSLMPattern(long runtimeCal, long id, const wchar_t* bleachH5PathName, long doStart, long phaseDirect, long timeout)
{
	long ret = TRUE;
	string slmDLLName;
	if((TRUE == pHardware->GetActiveHardwareDllName("Devices","SLM", slmDLLName)) && 
		(0 == slmDLLName.compare("ThorSLMPDM512")))
	{
		SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_RUNTIME_CALC, runtimeCal, IDevice::DeviceSetParamType::NO_EXECUTION);
		SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_FUNC_MODE,IDevice::SLMFunctionMode::LOAD_PHASE_ONLY, IDevice::DeviceSetParamType::NO_EXECUTION);
		ret = SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_ARRAY_ID,id,IDevice::DeviceSetParamType::NO_EXECUTION);
		ret = SetDeviceParamString(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_BMP_FILENAME,(wchar_t*)bleachH5PathName,IDevice::DeviceSetParamType::NO_EXECUTION);
		ret = SetDeviceParamLong(SelectedHardware::SELECTED_SLM, IDevice::PARAM_SLM_PHASE_DIRECT, phaseDirect, IDevice::DeviceSetParamType::NO_EXECUTION);
		ret = SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_TIMEOUT,timeout,IDevice::DeviceSetParamType::NO_EXECUTION);

		IDevice* slm = GetDevice(SelectedHardware::SELECTED_SLM);
		if(NULL != slm)
		{
			slm->PreflightPosition();
			ret = slm->SetupPosition();
			if(TRUE == doStart)
				ret = slm->StartPosition();
		}
	}
	return ret;
}