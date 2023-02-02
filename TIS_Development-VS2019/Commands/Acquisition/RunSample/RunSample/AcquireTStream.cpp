#include "stdafx.h"
#include "..\..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "RunSample.h"
#include "AcquireTStream.h"
#include "AcquireFactory.h"
#include "AcquireSingle.h"
#include "ImageCorrection.h"
#include "..\..\..\..\Common\PublicFuncs.h"

extern auto_ptr<CommandDll> shwDll;
extern auto_ptr<TiffLibDll> tiffDll;
extern unique_ptr<ImageStoreLibraryDLL> bigTiff;
extern vector<ScanRegion> activeScanAreas;
extern long viewMode;

void GetLookUpTables(unsigned short * rlut, unsigned short * glut, unsigned short *blut,long red, long green, long blue, long bp, long wp, long bitdepth);
long SaveTIFFWithoutOME(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut, double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string *acquiredDate, double dt, long doCompression);
long SaveTIFF(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut, double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string *acquiredDate, double dt, string * omeTiffData, PhysicalSize physicaSize, long doCompression);
long SaveTiledTiff(wchar_t *filePathAndName, char *pMemoryBuffer, long bufferSizeBytes, long imageWidth, long imageHeight, long tileWidth, long tileHeight, long totalChannels, long channelIndex, double umPerPixel, string imageDescription, bool compress=true);
string CreateOMEMetadata(int width, int height,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string * acquiredDateTime, double deltaT, string * omeTiffData, PhysicalSize physicalSize);
string ConvertWStringToString(wstring ws);
long SetDeviceParameterValue(IDevice *pDevice,long paramID, double val,long bWait,HANDLE hEvent,long waitTime);
static CRITICAL_SECTION saveAccess;
std::vector<long> AcquireTStream::savedFrameNumVec;
//#define USE_VIRTUAL_ALLOC
bool useVirtualMemory = false;
bool threadRawFileContainsDisabledChannels = false;
//depending on the dflim type capture we will do a regular
//intensity capture image extraction (0), or a dflim image extraction (1)
long imageMethod = 0;

AcquireTStream::AcquireTStream(IExperiment *exp,wstring path)
{
	_pExp = exp;
	_counter = 0;
	_tFrame = 1;
	_path = path;
	_lastImageUpdateTime = 0;
	_zstageStepSize = 0;
}

UINT StatusZThreadProc5( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;
	IDevice * pDevice = (IDevice*)pParam;
	while(status == IDevice::STATUS_BUSY)
	{
		if(FALSE == pDevice->StatusPosition(status))
			break;
	}
	SetEvent( AcquireTStream::hEventZ);
	return 0;
}


UINT SaveStreamThreadProc( LPVOID pParam )
{
	EnterCriticalSection(&saveAccess);
	AcquireTStream::_saveThreadActive = TRUE;

	AcquireTStream::SaveParams *sp = (AcquireTStream::SaveParams*)pParam;

	if(TRUE == sp->deleteFileOnThreadCompletion)	
	{
		//#ifdef USE_VIRTUAL_ALLOC
		if(useVirtualMemory)
		{
			SIZE_T memSize = sp->regionMap.begin()->second.SizeX;
			memSize *= sp->regionMap.begin()->second.SizeY;
			memSize *= sp->colorChannels;
			memSize *= sp->regionMap.begin()->second.SizeT;
			memSize *= 2; 
			VirtualFree(sp->pMemoryBuffer,0, MEM_RELEASE);		//MEM_RELEASE, dwSize has to be 0
		}	
		//#else
		else
		{
			if(FALSE == AcquireTStream::HandleStimStreamRawFile(sp,RESIZE_SINGLE_STIMULUS, std::wstring(), std::wstring(), threadRawFileContainsDisabledChannels))
			{
				StringCbPrintfW(message,MSG_LENGTH,L"AcquireTStream save thread failed resize imageID(%d)",sp->imageIDsMap[BufferType::INTENSITY]);
				logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
			}	
			if(FALSE == AcquireTStream::HandleStimStreamRawFile(sp,CONCATENATE_PREVIOUS_STIMULUS, std::wstring(), std::wstring(), threadRawFileContainsDisabledChannels))
			{
				StringCbPrintfW(message,MSG_LENGTH,L"AcquireTStream save thread failed cancatenate of imageID(%d)",sp->imageIDsMap[BufferType::INTENSITY]);
				logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
			}
		}
		//#endif
	}
	delete sp;
	AcquireTStream::_saveThreadCount--;
	AcquireTStream::_saveThreadCountFinished++;

	StringCbPrintfW(message,MSG_LENGTH,L"AcquireTStream save thread count (%d)",AcquireTStream::_saveThreadCount);
	logDll->TLTraceEvent(INFORMATION_EVENT,1,message);

	AcquireTStream::_saveThreadActive = FALSE;
	LeaveCriticalSection(&saveAccess);

	return 0;
}

struct UnlockParams
{
	ImageManager * pManager;
	long imageID;
	long index;
};

string WStringToStringATST(wstring ws)	//ATS: AcquireTStream
{	
	size_t origsize = wcslen(ws.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t convertedChars = 0;
	char nstring[newsize];
	wcstombs_s(&convertedChars, nstring, origsize, ws.c_str(), _TRUNCATE);

	string str(nstring);

	StringCbPrintfW(message,MSG_LENGTH,L"ExperimentManager ConvertWStringToString: %S",nstring);
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	return str;
}

HANDLE AcquireTStream::hEvent = NULL;
HANDLE AcquireTStream::hEventZ = NULL;
unsigned long AcquireTStream::_saveIndex = 0;
volatile unsigned long AcquireTStream::_saveThreadCount = 0;
volatile unsigned long AcquireTStream::_saveThreadCountFinished = 0;
AcquireSaveInfo* AcquireTStream::_acquireSaveInfo = NULL;

BOOL AcquireTStream::_evenOdd = FALSE;
double AcquireTStream::_lastGoodFocusPosition = 0.0;
BOOL AcquireTStream::_saveThreadActive = FALSE;
const long STORAGE_FINITE = 0;
const long STORAGE_STIMULUS = 1;

long AcquireTStream::CallCaptureComplete(long captureComplete)
{
	return TRUE;
}

long AcquireTStream::CallCaptureImage(long index)
{
	CaptureImage(index);
	return TRUE;
}

long AcquireTStream::CallSaveImage(long index, BOOL isImageUpdate)
{
	SaveImage(index, isImageUpdate);
	return TRUE;
}

long AcquireTStream::CallCaptureSubImage(long index)
{
	CaptureSubImage(index);
	return TRUE;
}

long AcquireTStream::CallSaveSubImage(long index)
{
	SaveSubImage(index);
	return TRUE;
}

long AcquireTStream::CallSaveZImage(long index, double power0, double power1, double power2, double power3,double power4, double power5)
{
	SaveZImage(index, power0, power1, power2, power3, power4, power5);
	return TRUE;
}

long AcquireTStream::CallSaveTImage(long index)
{
	SaveTImage(index);
	return TRUE;
}

long AcquireTStream::CallSequenceStepCurrent(long index)
{
	SequenceStepCurrent(index);
	return TRUE;
}

long AcquireTStream::PreCaptureEventCheck(long &status)
{
	return TRUE;
}

long AcquireTStream::StopCaptureEventCheck(long &status)
{
	StopCapture(status);
	return TRUE;
}

long AcquireTStream::CallStartProgressBar(long index, long resetTotalCount)
{
	StartProgressBar(index, resetTotalCount);
	return TRUE;
}

long AcquireTStream::CallInformMessage(wchar_t* message)
{
	InformMessage(message);
	return TRUE;
}

long AcquireTStream::CallNotifySavedFileIPC(wchar_t* message)
{
	NotifySavedFileIPC(message);
	return TRUE;
}

/// <return> return next created image ID, -1 if failed. </return>
long AcquireTStream::CreateSaveThread(SaveParams& sp, char* pMem)
{
	long ret = FALSE;
	DWORD dwThread;
	HANDLE hThread = NULL;
	SaveParams *spOut = new SaveParams();
	*spOut = sp;

	//#ifdef USE_VIRTUAL_ALLOC
	if(useVirtualMemory)
		spOut->pMemoryBuffer = pMem;
	//#endif

	//we will be saving the data as its captured in stimulus mode
	hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) SaveStreamThreadProc, spOut, CREATE_SUSPENDED, &dwThread );	
	if(NULL != hThread)
	{
		_threadHandles.push_back(hThread);
		_saveThreadCount++;
		ret = TRUE;
	}

	//#ifdef USE_VIRTUAL_ALLOC
	if(useVirtualMemory)
	{
		SIZE_T memSize = sp.regionMap.begin()->second.SizeX;
		memSize *= sp.regionMap.begin()->second.SizeY;
		memSize *= sp.bufferChannels.begin()->second;
		memSize *= sp.regionMap.begin()->second.SizeT;
		memSize *= 2; 
		pMem = (char*)VirtualAlloc(NULL, memSize,MEM_COMMIT,PAGE_READWRITE);
		DWORD getError = GetLastError();
	}
	//#else
	else
	{
		DestroyImages(sp.imageIDsMap);
		DestroyImages(sp.regionImageIDsMap); 
		long imageID = 0;
		for (std::map<long, Dimensions>::iterator it = sp.dimensionsMap.begin(); it != sp.dimensionsMap.end(); it++)
		{
			//TODO: still need to name the stream string correctly
			ImageManager::getInstance()->CreateImage(imageID,it->second,L"Stream");
			sp.imageIDsMap[it->first] = imageID;
		}
		for (std::map<long, Dimensions>::iterator it = sp.regionDimensionsMap.begin(); it != sp.regionDimensionsMap.end(); it++)
		{
			ImageManager::getInstance()->CreateImage(imageID,it->second,L"Stream");
			sp.regionImageIDsMap[it->first] = imageID;
		}
	}
	//#endif

	return ret;
}

void AcquireTStream::ResumeSaveThread(int threadPriority)
{
	if((FALSE == _saveThreadActive)&&(_threadHandles.size() > _saveThreadCountFinished))
	{
		if(TryEnterCriticalSection(&saveAccess))
		{
			vector<HANDLE>::iterator it = _threadHandles.begin();

			it += _saveThreadCountFinished;

			SetThreadPriority(*it,threadPriority);

			ResumeThread(*it);
			LeaveCriticalSection(&saveAccess);
		}
	}
}

long AcquireTStream::Execute(long index, long subWell, long zFrame, long tFrame)
{

	_tFrame = tFrame;
	long result = Execute(index, subWell);

	return result;
}

long AcquireTStream::SetupZStage(int setupMode, ICamera* pCamera, ZRangeInfo* zRange)
{
	IDevice* pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);
	if (NULL == pZStage)
	{
		StringCbPrintfW(message, MSG_LENGTH, L"RunSample Execute Z stage invalid");
		logDll->TLTraceEvent(ERROR_EVENT, 1, message);
		return FALSE;
	}

	long streamEnable, streamFramesRead, rawData, triggerMode, displayImage, storageMode, zFastEnableGUI, zFastMode, flybackFrames, flybackLines, previewIndex, stimulusTriggering, stimulusMaxFrames, useReferenceVoltageForFastZPockels, dmaFrames;
	double flybackTimeAdjustMS = 0, volumeTimeAdjustMS = 0, stepTimeAdjustMS = 0;
	double z_max = 0, z_min = 0, zDefault = 0;
	long zType = 0, zAvailable = 0, zReadOnly = 0;
	double zStartPos, zStopPos, zTiltPos, zStepSizeMM;
	long zstageSteps, zStreamFrames, zStreamMode;
	double frameRate = 1.0;
	double flybackTime = 0.0;
	double* zPosBuffer = NULL;
	long areaMode, scanMode, interleave, pixelX, pixelY, chan, lsmFieldSize, offsetX, offsetY, averageMode, averageNum, clockSource, inputRange1, inputRange2, twoWayAlignment, extClockRate, flybackCycles, inputRange3, inputRange4, minimizeFlybackCycles, polarity[4], verticalFlip, horizontalFlip, timebasedLineScan = FALSE, timebasedLineScanMS = 0;
	double cameraType, lsmType, lsmFlybackLines, areaAngle, dwellTime, crsFrequencyHz = 0;
	long threePhotonEnable = FALSE;
	long numberOfPlanes = 1;
	long selectedPlane = 0;
	long displayCumulativeAveragePreview = FALSE;

	switch (setupMode)
	{
		case 0:
		{
			//get fast z params
			_pExp->GetStreaming(streamEnable, streamFramesRead, rawData, triggerMode, displayImage, storageMode, zFastEnableGUI, zFastMode, flybackFrames, flybackLines, flybackTimeAdjustMS, volumeTimeAdjustMS, stepTimeAdjustMS, previewIndex, stimulusTriggering, dmaFrames, stimulusMaxFrames, useReferenceVoltageForFastZPockels, displayCumulativeAveragePreview);

			//get z positions
			GetZPositions(_pExp, pZStage, zStartPos, zStopPos, zTiltPos, zStepSizeMM, zstageSteps, zStreamFrames, zStreamMode);
			if (zRange)
			{
				zRange->zFramesPerVolume = zstageSteps;
				zRange->zStartPosUM = zStartPos * Constants::UM_TO_MM;
				zRange->zStopPosUM = zStopPos * Constants::UM_TO_MM;
				zRange->zStepSizeUM = zStepSizeMM * Constants::UM_TO_MM;
			}

			//set z stage range
			if ((TRUE == pZStage->GetParam(IDevice::PARAM_Z_FAST_START_POS, z_max)) && (FALSE == pZStage->SetParam(IDevice::PARAM_Z_FAST_START_POS, zStartPos)))
			{
				//MessageBox(NULL,L"Invalid Z start position.",L"Z Stage Error",MB_OK | MB_SETFOREGROUND | MB_ICONERROR);	
				//return FALSE;
			}

			long powerRampMode = ICamera::PowerRampMode::POWER_RAMP_MODE_CONTINUOUS;
			pZStage->SetParam(IDevice::PARAM_Z_ANALOG_MODE, zFastMode);
			switch (static_cast<ZPiezoAnalogMode>(zFastMode))
			{
				case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_WAVEFORM:
					break;
				case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
					//allow change flyback cycles in camera if in z staircase mode
					if (FALSE == pCamera->SetParam(ICamera::Params::PARAM_LSM_MINIMIZE_FLYBACK_CYCLES, 0))
					{
						StringCbPrintfW(message, MSG_LENGTH, L"RunSample Execute Minimize Flyback Cycles not implemented for camera");
						logDll->TLTraceEvent(ERROR_EVENT, 1, message);
					}
					if (FALSE == pCamera->SetParam(ICamera::Params::PARAM_LSM_FLYBACK_CYCLE, flybackLines))
					{
						StringCbPrintfW(message, MSG_LENGTH, L"RunSample Execute Flyback Cycle not implemented for camera");
						logDll->TLTraceEvent(ERROR_EVENT, 1, message);
					}
					powerRampMode = ICamera::PowerRampMode::POWER_RAMP_MODE_STAIRCASE;
					break;
			}

			//set z stage positions for staircase mode, allow future random z positions
			//min steps of 2 for ramping reference voltage
			zstageSteps = (1 >= zstageSteps) ? 2 : zstageSteps;
			zPosBuffer = new double[zstageSteps];
			for (long z = 0; z < zstageSteps; z++) //looping through all Z positions
			{
				zPosBuffer[z] = zStartPos + z * zStepSizeMM;
			}
			pZStage->SetParamBuffer(IDevice::PARAM_Z_FAST_STEP_BUFFER, (char*)zPosBuffer, zstageSteps);
			long typePower, blankPercent;
			double startPower, stopPower;
			string pathPower;
			_pExp->GetPockels(0, typePower, startPower, stopPower, pathPower, blankPercent);
			//Only user reference voltage if pockels power type is custom power ramp
			const long EXPONENTIAL_POWER_RAMP = 1;
			if (EXPONENTIAL_POWER_RAMP == typePower)
			{
				//pass pockels response type from camera to zStage
				double pockelsResType = 0.0;
				if (TRUE == pCamera->GetParam(ICamera::PARAM_LSM_POCKELS_RESPONSE_TYPE_0, pockelsResType))
				{
					pZStage->SetParam(IDevice::PARAM_Z_OUTPUT_POCKELS_RESPONSE_TYPE, pockelsResType);
				}
				//set power ramp buffer
				double* zPowerBuffer = new double[zstageSteps];
				for (long z = 0; z < zstageSteps; z++) //looping through all Z positions
				{
					zPowerBuffer[z] = GetCustomPowerValue(zStartPos, zStopPos, zPosBuffer[z], pathPower);
				}

				long paramType, powerRampAvailable, paramReadOnly;
				double paramMax, paramMin, paramDefault;
				//if power ramp is available in the camera then allow the camera to control the pockels power ramp
				if (TRUE == pCamera->GetParamInfo(ICamera::PARAM_LSM_POWER_RAMP_ENABLE, paramType, powerRampAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
				{
					if (TRUE == powerRampAvailable)
					{
						pCamera->SetParam(ICamera::PARAM_LSM_POWER_RAMP_ENABLE, TRUE);
						pCamera->SetParam(ICamera::PARAM_LSM_POWER_RAMP_NUM_FRAMES, zstageSteps);
						pCamera->SetParam(ICamera::PARAM_LSM_POWER_RAMP_NUM_FLYBACK_FRAMES, flybackFrames);
						pCamera->SetParam(ICamera::PARAM_LSM_POWER_RAMP_MODE, powerRampMode);
						pCamera->SetParamBuffer(ICamera::PARAM_LSM_POWER_RAMP_PERCENTAGE_BUFFER, (char*)zPowerBuffer, zstageSteps);
					}
				}

				if (FALSE == powerRampAvailable && TRUE == useReferenceVoltageForFastZPockels)
				{
					pZStage->SetParam(IDevice::PARAM_Z_OUTPUT_POCKELS_REFERENCE, useReferenceVoltageForFastZPockels);

					pZStage->SetParamBuffer(IDevice::PARAM_POWER_RAMP_BUFFER, (char*)zPowerBuffer, zstageSteps);

					double pockelsMin = 0.0;
					if (TRUE == pCamera->GetParam(ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0, pockelsMin))
					{
						pZStage->SetParam(IDevice::PARAM_Z_POCKELS_MIN, pockelsMin);
					}
				}

				delete[] zPowerBuffer;
			}
			else
			{
				pZStage->SetParam(IDevice::PARAM_Z_OUTPUT_POCKELS_REFERENCE, FALSE);
			}

			delete[] zPosBuffer;
		}
		break;
		case 1:
			//get fast z params
			_pExp->GetStreaming(streamEnable, streamFramesRead, rawData, triggerMode, displayImage, storageMode, zFastEnableGUI, zFastMode, flybackFrames, flybackLines, flybackTimeAdjustMS, volumeTimeAdjustMS, stepTimeAdjustMS, previewIndex, stimulusTriggering, dmaFrames, stimulusMaxFrames, useReferenceVoltageForFastZPockels, displayCumulativeAveragePreview);
			//get z positions
			GetZPositions(_pExp, pZStage, zStartPos, zStopPos, zTiltPos, zStepSizeMM, zstageSteps, zStreamFrames, zStreamMode);

			//set frame rate or others which is acurate only after preflight of camera
			if (FALSE == pCamera->GetParam(ICamera::PARAM_FRAME_RATE, frameRate))
			{
				StringCbPrintfW(message, MSG_LENGTH, L"RunSample Execute Frame Rate not implemented for camera");
				logDll->TLTraceEvent(ERROR_EVENT, 1, message);
			}
			//update frame rate if resonance frequency is measured
			if ((pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE, cameraType) && (double)ICamera::CameraType::LSM == cameraType) &&
				(pCamera->GetParam(ICamera::PARAM_LSM_TYPE, lsmType) && ((double)ICamera::LSMType::GALVO_RESONANCE == lsmType || (double)ICamera::LSMType::RESONANCE_GALVO_GALVO == lsmType))
				)
			{
				_pExp->GetLSM(areaMode, areaAngle, scanMode, interleave, pixelX, pixelY, chan, lsmFieldSize, offsetX, offsetY, averageMode, averageNum, clockSource, inputRange1, inputRange2,
					twoWayAlignment, extClockRate, dwellTime, flybackCycles, inputRange3, inputRange4, minimizeFlybackCycles, polarity[0], polarity[1], polarity[2], polarity[3], verticalFlip, horizontalFlip, crsFrequencyHz, timebasedLineScan, timebasedLineScanMS, threePhotonEnable, numberOfPlanes);

				//if its a timebased line scan then we want to get the pixel Y from the camera instead of assuming that it is what we set it as
				if (timebasedLineScan)
				{
					pCamera->SetParam(ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS, timebasedLineScanMS);
					double lsmHeight;
					pCamera->GetParam(ICamera::PARAM_LSM_PIXEL_Y, lsmHeight);
					pixelY = static_cast<long>(lsmHeight);
				}

				if (pCamera->GetParam(ICamera::Params::PARAM_LSM_FLYBACK_CYCLE, lsmFlybackLines))
				{
					frameRate = (0 < crsFrequencyHz) ? (crsFrequencyHz / ((((long)ScanMode::TWO_WAY_SCAN == scanMode) ? (pixelY / 2) : pixelY) + lsmFlybackLines)) : frameRate;
				}
			}
			StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream SetupZStage frame rate %d.%d", static_cast<long>(frameRate), static_cast<long>(1000 * (frameRate - static_cast<long>(frameRate))));
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);

			long paramType, paramAvailable, paramReadOnly, minZFrames, maxZFrames;
			double paramMin, paramMax, paramDefault;
			switch (static_cast<ZPiezoAnalogMode>(zFastMode))
			{
				case ZPiezoAnalogMode::ANALOG_MODE_SINGLE_WAVEFORM:
				{
					StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream SetupZStage volume frames %d flyback frames %d", zstageSteps, flybackFrames);
					logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);

					if (FALSE == pZStage->SetParam(IDevice::PARAM_Z_FAST_VOLUME_TIME, zstageSteps / frameRate))
					{
						pZStage->GetParamInfo(IDevice::PARAM_Z_FAST_VOLUME_TIME, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
						if (FALSE == paramAvailable)
						{
							StringCbPrintfW(message, MSG_LENGTH, L"Invalid Z Stage.");
						}
						else
						{
							minZFrames = static_cast<long>(ceil(paramMin * frameRate));
							maxZFrames = static_cast<long>(floor(paramMax * frameRate));
							if (minZFrames > zstageSteps)
							{
								StringCbPrintfW(message, MSG_LENGTH, L"Invalid Number of Slices.\nAcceptable minimum under current configuraton: %d", minZFrames);
							}
							else if (maxZFrames < zstageSteps)
							{
								StringCbPrintfW(message, MSG_LENGTH, L"Invalid Number of Slices.\nAcceptable maximum under current configuraton: %d", maxZFrames);
							}
							else
							{
								StringCbPrintfW(message, MSG_LENGTH, L"Invalid Number of Slices.");
							}
						}
						MessageBox(NULL, message, L"Capture Streaming FastZ Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
						return FALSE;
					}
					//set volume time adjustment
					pZStage->SetParam(IDevice::PARAM_Z_FAST_VOLUME_TIME_ADJUST_MS, volumeTimeAdjustMS);
				}
				break;
				case ZPiezoAnalogMode::ANALOG_MODE_STAIRCASE_WAVEFORM:
				{
					if (FALSE == pCamera->GetParam(ICamera::Params::PARAM_LSM_FLYBACK_TIME, flybackTime))
					{
						StringCbPrintfW(message, MSG_LENGTH, L"RunSample Execute Flyback Time not implemented for camera");
						logDll->TLTraceEvent(ERROR_EVENT, 1, message);
					}
					if (FALSE == pZStage->SetParam(IDevice::PARAM_Z_FAST_INTRA_STEP_TIME, flybackTime))
					{
						StringCbPrintfW(message, MSG_LENGTH, L"RunSample Intra Step Time not implemented for z stage.");
						logDll->TLTraceEvent(ERROR_EVENT, 1, message);
					}
					double frameTime = 1.0 / frameRate - flybackTime;
					if (FALSE == pZStage->SetParam(IDevice::PARAM_Z_FAST_STEP_TIME, frameTime))
					{
						StringCbPrintfW(message, MSG_LENGTH, L"Invalid Z Stage for Staircase mode.");
						MessageBox(NULL, message, L"Capture Streaming FastZ Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
						return FALSE;
					}
					//set frame time adjustment
					pZStage->SetParam(IDevice::PARAM_Z_FAST_STEP_TIME_ADJUST_MS, stepTimeAdjustMS);
				}
				break;
			}
			//set fast z flyback time
			if (FALSE == pZStage->SetParam(IDevice::PARAM_Z_FAST_FLYBACK_TIME, flybackFrames / frameRate))
			{
				pZStage->GetParamInfo(IDevice::PARAM_Z_FAST_FLYBACK_TIME, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
				pZStage->SetParamString(IDevice::PARAM_WAVEFORM_OUTPATH, (wchar_t*)GetDir(_pExp->GetPathAndName()).c_str());
				minZFrames = static_cast<long>(ceil(paramMin * frameRate));
				maxZFrames = static_cast<long>(floor(paramMax * frameRate));
				if (minZFrames > flybackFrames)
				{
					StringCbPrintfW(message, MSG_LENGTH, L"Invalid Flyback Frames.\nAcceptable minimum under current configuraton: %d", minZFrames);
				}
				else if (maxZFrames < flybackFrames)
				{
					StringCbPrintfW(message, MSG_LENGTH, L"Invalid Flyback Frames.\nAcceptable maximum under current configuraton: %d", maxZFrames);
				}
				else
				{
					StringCbPrintfW(message, MSG_LENGTH, L"Invalid Flyback Frames.");
				}
				MessageBox(NULL, message, L"Capture Streaming FastZ Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
				return FALSE;
			}
			pZStage->SetParam(IDevice::PARAM_Z_FAST_FLYBACK_TIME_ADJUST_MS, flybackTimeAdjustMS);
			pZStage->PreflightPosition();
			pZStage->SetupPosition();
			if (FALSE == pZStage->StartPosition())
			{
				pZStage->GetLastErrorMsg(message, MSG_LENGTH);
				MessageBox(NULL, message, L"Capture Streaming FastZ Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
				return FALSE;
			}

			StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream SetupZStage complete");
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);

			break;
		case 2:
			//stop z stage:
			pZStage->PostflightPosition();
			pZStage->SetParam(IDevice::PARAM_Z_ANALOG_MODE, ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT);
			break;
	}
	return TRUE;
}

void AcquireTStream::SaveImagesPostCapture(long index, long subWell, long streamFrames, SaveParams *sp, bool rawContainsDisabledChannels, long subwell)
{
	long captureMode;
	_pExp->GetCaptureMode(captureMode);
	if(CaptureFile::FILE_RAW == (CaptureFile)sp->fileType)
	{
		//save timing information to file after capture is done
		SaveTimingToExperimentFolder();

		//#ifndef USE_VIRTUAL_ALLOC
		if(!useVirtualMemory)
		{	
			//ImageManager::getInstance()->DestroyImage(imageID);
		}
		//#endif

		HandleStimStreamRawFile(sp,RENAME_SINGLE_TIS, std::wstring(), std::wstring(), rawContainsDisabledChannels);

		//clear the lists
		_acquireSaveInfo->getInstance()->ClearTimingInfo();
		_acquireSaveInfo->getInstance()->ClearTimestamps();


		//for loop is to update ui events. no logic consequence
		for(long t=1;t<=streamFrames;t++)
		{
			CallSaveTImage(t);
			// if the capture mode is hyperspectral, check if it tiling is enabled or not
			if(IExperiment::HYPERSPECTRAL == captureMode)
			{
				if(2 > subwell)
				{
					CallSaveImage(t, TRUE);
				}
				else
				{
					// if the capture mode is hyperspectral with tiling and the subwell index 
					// is bigger than 2 then only CallSaveImage on the last image in the hs sequence.
					if(streamFrames == t)
					{
						CallSaveImage(t, TRUE);
					}
				}
			}
			else
			{
				CallSaveImage(t, TRUE);
			}
		}

		if(sp->_pHeaderInfo != nullptr)
		{
			wchar_t hdrName[_MAX_FNAME];
			long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
			std::wstringstream imgNameFormat;
			imgNameFormat << L"Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.hdr";
			StringCbPrintfW(hdrName,_MAX_FNAME,imgNameFormat.str().c_str(),index,subWell);
			wstring fullPath = wstring(sp->path.begin(), sp->path.end());
			wstring hdrFileName = fullPath.substr(0, fullPath.find_last_of('\\') + 1) + wstring(hdrName);			
			if(!saveHeaderHdr(hdrFileName, *(sp->_pHeaderInfo)))
			{
				//TODO
			}
		}

	}
	else
	{	
		//eliiminate flyback frames from timing info
		for(long j=(sp->regionMap.begin()->second.SizeT-1); j>=0; j--)
		{
			//computer zero based indexes for z and t
			long curT = static_cast<long>(floor(j/(double)(sp->regionMap.begin()->second.SizeZ+sp->fastFlybackFrames)));
			long curZ = j % (sp->regionMap.begin()->second.SizeZ + sp->fastFlybackFrames);

			if(curZ >= static_cast<long>(sp->regionMap.begin()->second.SizeZ))
			{
				_acquireSaveInfo->getInstance()->RemoveTimestampAt(j);
				_acquireSaveInfo->getInstance()->RemoveTimingInfoAt(j);

			}
		}

		//save timing information to file after capture is done
		SaveTimingToExperimentFolder();

		//if the raw data is disabled. convert the buffer to individual files
		//SaveData also deletes the list items after they are saved in TIFFs
		long imageMode = FALSE;
		bool stopCheckAllowed = false;
		if(0 == _messageID)
		{	stopCheckAllowed = true;	}
		SaveData(sp,imageMode,stopCheckAllowed, rawContainsDisabledChannels, subwell);

		//#ifdef USE_VIRTUAL_ALLOC
		if(useVirtualMemory)
		{
			SIZE_T memSize = sp->regionMap.begin()->second.SizeX;
			memSize *= sp->regionMap.begin()->second.SizeY;
			memSize *= sp->colorChannels;
			memSize *= sp->regionMap.begin()->second.SizeT;
			memSize *= 2; 
			VirtualFree(sp->pMemoryBuffer, 0, MEM_RELEASE);
		}	 
		//#else
		//else
		//{ImageManager::getInstance()->DestroyImage(imageID);}
		//#endif
		//the final image update after the memory has been destroyed
		CallSaveImage(streamFrames, TRUE);
	}
}

void AcquireTStream::SetupSaveParams(long index, long subWell, long streamFrames, double exposureTimeMS, long width, long height, long numberOfPlanes, double umPerPixel, long zFastEnableGUI, long zStageSteps, long zFlybackFrames,long bufferChannels, long lsmChannels, long storageMode, long hyperSpectralWavelengths, long rawData, long previewID, SaveParams* sp)
{
	ScanRegion sregionMap;
	sregionMap.ScanID = 1;
	sregionMap.RegionID = 0;
	sregionMap.SizeX = width;
	sregionMap.SizeY = height * numberOfPlanes;
	sregionMap.SizeZ = (TRUE == zFastEnableGUI) ? zStageSteps : 1;
	sregionMap.SizeT = streamFrames;
	sregionMap.SizeS = 1;
	sregionMap.BufferSize = (size_t)width * height * bufferChannels * numberOfPlanes * sizeof(USHORT);
	sregionMap.NumberOfPlanes = 1;//numberOfPlanes;
	sp->regionMap.clear();
	sp->imageIDsMap.clear();
	sp->dimensionsMap.clear();
	sp->regionImageIDsMap.clear();
	sp->regionDimensionsMap.clear();

	string wavelengthName;
	sp->path = _path;
	for(long w=0; w<_pExp->GetNumberOfWavelengths(); w++)
	{
		_pExp->GetWavelength(w,wavelengthName,exposureTimeMS);
		sp->wavelengthName[w] =wavelengthName;
	}
	sp->index  = index;
	sp->subWell  = subWell;
	sp->red[0]  =255;
	sp->green[0]  =255;
	sp->blue[0] =255;
	sp->bp[0] =0;
	sp->wp[0] =255;
	sp->colorChannels = _pExp->GetNumberOfWavelengths();	
	sp->umPerPixel = umPerPixel;
	sp->fastFlybackFrames = (TRUE == zFastEnableGUI) ? zFlybackFrames : 0;
	sp->lsmChannels = lsmChannels;
	sp->storageMode = storageMode;
	sp->doCompression = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"TIFFCompressionEnable",L"value", FALSE);
	sp->hyperSpectralWavelengths = hyperSpectralWavelengths;
	sp->fileType = rawData;
	sp->imageMethod = imageMethod;
	sp->photonsBufferTotalSize = 0;
	sp->previewRegionID = previewID;

	//must have at least one region in regionMap either meso or micro view:
	if (0 >= activeScanAreas.size())
	{
		sp->regionMap.insert(make_pair(0, sregionMap));
		sp->bufferChannels.insert(make_pair(0, bufferChannels));
	}
	else
	{
		for (int i = 0; i < activeScanAreas.size(); i++)
		{
			//set Z & T locally instead of updating settings file due to no support on RGG's Z
			activeScanAreas[i].SizeZ = (TRUE == zFastEnableGUI) ? zStageSteps : 1;
			activeScanAreas[i].SizeT = streamFrames;
			activeScanAreas[i].BufferSize = activeScanAreas[i].SizeX * activeScanAreas[i].SizeY * bufferChannels * sizeof(USHORT);
			sp->bufferChannels.insert(make_pair(activeScanAreas[i].RegionID, bufferChannels));
			sp->regionMap.insert(make_pair(activeScanAreas[i].RegionID, activeScanAreas[i]));
		}
	}
	//create temporary largest buffer for multi-area:
	SAFE_DELETE_MEMORY(sp->pMemoryBuffer);
	sp->pMemoryBuffer = (MesoScanTypes::Micro == viewMode) ? 
		(char*)realloc((void*)sp->pMemoryBuffer, sizeof(USHORT) * activeScanAreas[0].SizeX * activeScanAreas[0].SizeY * sp->bufferChannels[activeScanAreas[0].RegionID]) : NULL;
}

long AcquireTStream::SetupImageData(wstring streamPath, ICamera *pCamera, long averageMode, SaveParams* sp, long zFastEnableGUI, long bufferChannels, double pixelDwellTime, long avgFrames, Dimensions &baseDimensions)
{
	long tempImageID = 0;

	baseDimensions.c = bufferChannels;
	baseDimensions.dType = INT_16BIT;
	baseDimensions.m = 1;
	baseDimensions.mType = CONTIGUOUS_CHANNEL_MEM_MAP_NO_FILE_DELETE;	

	if(ICamera::AVG_MODE_CUMULATIVE == averageMode)
	{
		if((STORAGE_STIMULUS == sp->storageMode) && (static_cast<long>(sp->regionMap.begin()->second.SizeT) < avgFrames))
		{
			logDll->TLTraceEvent(ERROR_EVENT,1,L"RunSample Max Frames per Stimulus cannot be less than Average Frames.");
			return FALSE;
		}
	}

	baseDimensions.t = sp->regionMap.begin()->second.SizeT * avgFrames;
	baseDimensions.x = sp->regionMap.begin()->second.SizeX;
	baseDimensions.y = sp->regionMap.begin()->second.SizeY;
	baseDimensions.z = 1;
	baseDimensions.imageBufferType = BufferType::INTENSITY;


	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	wchar_t rawPath[_MAX_PATH];

	_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
	StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s",drive,dir);
	ImageManager::getInstance()->SetMemMapPath(rawPath);

	//#ifndef USE_VIRTUAL_ALLOC
	if(!useVirtualMemory)
	{	
		//will create ome tiff when settings is configured, use name "Image.tif" for either finite or stimulus
		StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s%s",drive,dir,L"Image");
		bigTiff->SetupImageStore(rawPath, _pExp, sp->doCompression);

		//create image with name of Image for a finite experiment:
		if (STORAGE_FINITE == sp->storageMode)
		{
			if (1 == imageMethod)	// dFlim capture
			{
				Dimensions ddflimIntensity = baseDimensions;
				ddflimIntensity.imageBufferType = BufferType::INTENSITY;
				if(FALSE == ImageManager::getInstance()->CreateImage(tempImageID,ddflimIntensity,L"Image",sp->index,sp->subWell))
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create memory buffer, disc space may not be sufficient.");
					return FALSE;
				}

				sp->imageIDsMap[BufferType::INTENSITY] = tempImageID;
				sp->dimensionsMap[BufferType::INTENSITY] = ddflimIntensity;

				//sp->imageIDsMap.insert(std::pair<long, long>(BufferType::INTENSITY, tempImageID));
				//sp->dimensionsMap.insert(std::pair<long, Dimensions>(BufferType::INTENSITY, ddflimIntensity));					

				const long DFLIM_HISTOGRAM_BINS = 256;
				Dimensions ddflimHisto= baseDimensions;
				ddflimHisto.imageBufferType = BufferType::DFLIM_HISTOGRAM;
				ddflimHisto.x = DFLIM_HISTOGRAM_BINS;
				ddflimHisto.y = 1;
				ddflimHisto.dType = DataType::INT_32BIT;
				if(FALSE == ImageManager::getInstance()->CreateImage(tempImageID,ddflimHisto,L"Image_dFLIMHisto",sp->index,sp->subWell))
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create memory buffer, disc space may not be sufficient.");
					return FALSE;
				}			
				sp->imageIDsMap.insert(std::pair<long, long>(BufferType::DFLIM_HISTOGRAM, tempImageID));
				sp->dimensionsMap.insert(std::pair<long, Dimensions>(BufferType::DFLIM_HISTOGRAM, ddflimHisto));

				Dimensions ddflimSinglePhoton= baseDimensions;
				ddflimSinglePhoton.imageBufferType = BufferType::DFLIM_IMAGE_SINGLE_PHOTON;
				ddflimSinglePhoton.dType = DataType::INT_16BIT;
				if(FALSE == ImageManager::getInstance()->CreateImage(tempImageID,ddflimSinglePhoton,L"Image_dFLIMSinglePhoton",sp->index,sp->subWell))
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create memory buffer, disc space may not be sufficient.");
					return FALSE;
				}			
				sp->imageIDsMap.insert(std::pair<long, long>(BufferType::DFLIM_IMAGE_SINGLE_PHOTON, tempImageID));
				sp->dimensionsMap.insert(std::pair<long, Dimensions>(BufferType::DFLIM_IMAGE_SINGLE_PHOTON, ddflimSinglePhoton));

				Dimensions ddflimArrivalTimeSum = baseDimensions;
				ddflimArrivalTimeSum.imageBufferType = BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM;
				ddflimArrivalTimeSum.dType = DataType::INT_32BIT;
				if(FALSE == ImageManager::getInstance()->CreateImage(tempImageID,ddflimArrivalTimeSum,L"Image_dFLIMArrivalTimeSum",sp->index,sp->subWell))
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create memory buffer, disc space may not be sufficient.");
					return FALSE;
				}			
				sp->imageIDsMap.insert(std::pair<long, long>(BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM, tempImageID));
				sp->dimensionsMap.insert(std::pair<long, Dimensions>(BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM, ddflimArrivalTimeSum));


				long maxPhotonsSize = static_cast<long>(round(baseDimensions.x * baseDimensions.y * (1 + 30 * pixelDwellTime)));
				Dimensions ddflimPhotons = baseDimensions;
				ddflimPhotons.x = maxPhotonsSize;
				ddflimPhotons.y = 1;
				ddflimPhotons.imageBufferType = BufferType::DFLIM_PHOTONS;
				ddflimPhotons.dType = DataType::INT_8BIT;
				if(FALSE == ImageManager::getInstance()->CreateImage(tempImageID,ddflimPhotons,L"Image_photons",sp->index,sp->subWell))
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create memory buffer, disc space may not be sufficient.");
					return FALSE;
				}
				sp->imageIDsMap[BufferType::DFLIM_PHOTONS] = tempImageID;
				sp->dimensionsMap[BufferType::DFLIM_PHOTONS] = ddflimPhotons;					
			}
			if (MesoScanTypes::Micro != viewMode)	//single area (meso) scan
			{
				if(FALSE == ImageManager::getInstance()->CreateImage(tempImageID,baseDimensions,L"Image",sp->index,sp->subWell))
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create memory buffer, disc space may not be sufficient.");
					return FALSE;
				}
				sp->imageIDsMap[BufferType::INTENSITY] = tempImageID;
				sp->dimensionsMap[BufferType::INTENSITY] = baseDimensions;
			}

			//add scan to big tiff before save data in finite mode, will add more scans in each stimulus
			bigTiff->AddScan(sp->_zInfo.zStartPosUM, sp->_zInfo.zStopPosUM, sp->_zInfo.zStepSizeUM, 
				static_cast<long>(floor(sp->regionMap[0 < activeScanAreas.size() ? activeScanAreas[0].RegionID : 0].SizeT/(double)(sp->regionMap[0 < activeScanAreas.size() ? activeScanAreas[0].RegionID : 0].SizeZ+sp->fastFlybackFrames))));
			//scan ID here is the number of scans, not MesoScanTypes (meso or micro), especially for stimulus streaming
			bigTiff->SetScan(1);
			//keep region at the first
			bigTiff->SetRegion((0 < activeScanAreas.size()) ? activeScanAreas[0].RegionID : 0);
		}
		else //create image with name of stream for an stimulus experiment:
		{
			//TODO: add dflim logic for stimulus
			if (MesoScanTypes::Micro != viewMode)	//single area (meso) scan
			{
				if(FALSE == ImageManager::getInstance()->CreateImage(tempImageID,baseDimensions,L"Stream"))
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create memory buffer, disc space may not be sufficient.");
					return FALSE;
				}
				sp->imageIDsMap[BufferType::INTENSITY] =  tempImageID;
				sp->dimensionsMap[BufferType::INTENSITY] = baseDimensions;
			}
		}
		//multi-area scan: (imageIDsMap already kept meso scan, reserve regionImageIDsMap for micro scan only)
		if (MesoScanTypes::Micro == viewMode)
		{
			std::wstring fname = (STORAGE_FINITE == sp->storageMode) ? L"Image" : L"Stream";
			for (int i = 0; i < activeScanAreas.size(); i++)
			{
				baseDimensions.x = activeScanAreas[i].SizeX;
				baseDimensions.y = activeScanAreas[i].SizeY;
				if(FALSE == ImageManager::getInstance()->CreateImage(tempImageID,baseDimensions,fname))
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create memory buffer, disc space may not be sufficient.");
					return FALSE;
				}
				////[DFlim] keep the first imageID in INTENSITY BufferType, later use regionImageIDsMap to search for next imageID in stimulus mode
				//if ((0 == i) && (1 == imageMethod))
				//{
				//	sp->imageIDsMap.insert(std::pair<long, long>(BufferType::INTENSITY, tempImageID));
				//	sp->dimensionsMap.insert(std::pair<long, Dimensions>(BufferType::INTENSITY, baseDimensions));
				//}
				sp->regionImageIDsMap.insert(std::pair<long, long>(activeScanAreas[i].RegionID, tempImageID));
				sp->regionDimensionsMap.insert(std::pair<long, Dimensions>(activeScanAreas[i].RegionID, baseDimensions));
			}
		}
	}
	//#endif

	return TRUE;
}

long AcquireTStream::PreCaptureAutoFocus(long index, long subWell, double afStartPos, double afAdaptiveOffset)
{
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
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample RunAutofocus failed");
		return FALSE;
	}

	return TRUE;
}

// This is used in streamin stimulus mode to open the shutter as fast as it can once it receives the 
// hardware trigger in. Measured delay time between trigger and shutter going high ~2ms.
void AcquireTStream::ControlShutterInStream(long zFastEnableGUI)
{
	if(TRUE == _swStimulusActive)
	{
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireTStream Stimulus is Active");
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);

		if(1 == zFastEnableGUI)
		{
			//do nothing we will close the shutter when the volume completes
		}
		else
		{
			if((false == RunSample::getInstance()->_isSaving)&&(FALSE == _hwStimulusActive))
			{
				CloseShutter();
			}							
		}
	}
	else
	{
		if((true == RunSample::getInstance()->_isSaving)||(TRUE == _hwStimulusActive))
		{
			StringCbPrintfW(message,MSG_LENGTH,L"AcquireTStream ControlShutterInStream opening shutter");
			logDll->TLTraceEvent(INFORMATION_EVENT,1,message);	
			OpenShutter();
		}
	}
}

long AcquireTStream::GetStimulusActive()
{
	IDevice * pStimulusDevice = GetDevice(SelectedHardware::SELECTED_EPHYS);
	if(NULL != pStimulusDevice)
	{
		double val;
		//if any line is high then the stimulus is active
		_hwStimulusActive = (pStimulusDevice->GetParam(IDevice::PARAM_EPHYS_DIG_LINE_IN_1,val) && (0 != static_cast<long>(val))) ? TRUE : FALSE;
		return TRUE;
	}
	return FALSE;
}

long AcquireTStream::SetGetCameraSettings(ICamera* pCamera, long &channel, long &bufferChannels, long &width, long &height, long &avgMode, long &avgNum, double &umPerPixel, long &fieldSize, double &pixelDwellTime, long& numPlanes)
{
	long cameraType = ICamera::CameraType::LAST_CAMERA_TYPE;
	cameraType = (GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_TYPE,cameraType)) ? cameraType : ICamera::CameraType::LAST_CAMERA_TYPE;

	long streamEnable,streamFrames,rawData,triggerMode,displayImage,storageMode,zFastEnableGUI,zFastMode,flybackFrames,flybackLines,previewIndex,stimulusTriggering,stimulusMaxFrames,useReferenceVoltageForFastZPockels,dmaFrames;
	double flybackTimeAdjustMS, volumeTimeAdjustMS, stepTimeAdjustMS;
	long displayCumulativeAveragePreview = FALSE;
	_pExp->GetStreaming(streamEnable,streamFrames,rawData,triggerMode,displayImage,storageMode,zFastEnableGUI,zFastMode,flybackFrames,flybackLines,flybackTimeAdjustMS,volumeTimeAdjustMS,stepTimeAdjustMS,previewIndex,stimulusTriggering,dmaFrames,stimulusMaxFrames,useReferenceVoltageForFastZPockels, displayCumulativeAveragePreview);
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
		pCamera->SetParam(ICamera::PARAM_CAMERA_AVERAGENUM,camAverageNum);
		pCamera->SetParam(ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP,camVericalFlip);
		pCamera->SetParam(ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP,camHorizontalFlip);
		pCamera->SetParam(ICamera::PARAM_CAMERA_IMAGE_ANGLE,imageAngle);
		pCamera->SetParam(ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT, dmaFrames);
		//will average after acquisition is complete. Set to none at camera level
		pCamera->SetParam(ICamera::PARAM_CAMERA_AVERAGEMODE,ICamera::AVG_MODE_NONE);

		channel = 1;
		bufferChannels = 1;
		width = camImageWidth;
		height = camImageHeight;
		avgMode = camAverageMode;
		avgNum = camAverageNum;
		umPerPixel = camPixelSize;
		//Field size is irrelevant for CCD camera, set to smallest
		fieldSize = 5;
	}
	else
	{
		long areaMode,scanMode,interleave,pixelX,pixelY,chan,lsmFieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles,polarity[4], verticalFlip, horizontalFlip, timebasedLineScan = FALSE, timebasedLineScanMS = 0;
		double areaAngle,dwellTime, crsFrequencyHz = 0;
		long threePhotonEnable = FALSE;
		long numberOfPlanes = 1;
		long selectedPlane = 0;
		wstring pathAndName = _pExp->GetPathAndName();
		//getting the values from the experiment setup XML files
		_pExp->GetLSM(areaMode,areaAngle,scanMode,interleave,pixelX,pixelY,chan,lsmFieldSize,offsetX,offsetY,averageMode,averageNum,clockSource, inputRange1, inputRange2, 
			twoWayAlignment,extClockRate,dwellTime,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles,polarity[0],polarity[1],polarity[2],polarity[3], verticalFlip, horizontalFlip, crsFrequencyHz, timebasedLineScan, timebasedLineScanMS, threePhotonEnable, numberOfPlanes);
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
		pCamera->SetParamString(ICamera::PARAM_MESO_EXP_PATH,(wchar_t*)pathAndName.c_str());
		pCamera->SetParamString(ICamera::PARAM_WAVEFORM_OUTPATH, (wchar_t*)GetDir(pathAndName).c_str());
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
		pCamera->SetParam(ICamera::PARAM_LSM_AVERAGEMODE,averageMode);
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
		//will average after acquisition is complete. Set to none at camera level
		pCamera->SetParam(ICamera::PARAM_LSM_AVERAGEMODE,ICamera::AVG_MODE_NONE);
		pCamera->SetParam(ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION, verticalFlip);
		pCamera->SetParam(ICamera::PARAM_LSM_HORIZONTAL_FLIP, horizontalFlip);
		
		pCamera->SetParam(ICamera::PARAM_LSM_3P_ENABLE, threePhotonEnable);
		pCamera->SetParam(ICamera::PARAM_LSM_NUMBER_OF_PLANES, numberOfPlanes);

		long typePower,blankPercent;
		double startPower,stopPower;
		string pathPower;
		_pExp->GetPockels(0,typePower,startPower,stopPower,pathPower,blankPercent);

		//Only user reference voltage if pockels power type is custom power ramp
		if (1 == typePower)
		{
			pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF,useReferenceVoltageForFastZPockels);
		}
		else
		{
			pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF,FALSE);
		}

		channel = chan;		
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
		if (pCamera->SetParam(ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN, timebasedLineScan) && TRUE == timebasedLineScan)
		{
			pCamera->SetParam(ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS, timebasedLineScanMS);
			double lsmHeight;
			pCamera->GetParam(ICamera::PARAM_LSM_PIXEL_Y, lsmHeight);
			height = pixelY = static_cast<long>(lsmHeight);
		}
		else
		{
			height = pixelY;
		}
		
		avgMode = averageMode;
		avgNum = averageNum;

		double fieldSizeCalibration = 100.0;
		pCamera->GetParam(ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION,fieldSizeCalibration);
		double magnification;
		string objName;
		_pExp->GetMagnification(magnification, objName);
		umPerPixel = (lsmFieldSize * fieldSizeCalibration)/(pixelX * magnification);		
		fieldSize = lsmFieldSize;
		pixelDwellTime = dwellTime;
		numPlanes = numberOfPlanes >= 1 && TRUE == threePhotonEnable ? numberOfPlanes : 1;
	}

	return TRUE;
}


long AcquireTStream::SetCameraTriggerMode(ICamera* pCamera)
{
	long captureMode;
	_pExp->GetCaptureMode(captureMode);

	if (IExperiment::HYPERSPECTRAL == captureMode)
	{
		_triggerMode = ICamera::HW_MULTI_FRAME_TRIGGER_EACH_BULB;
	}
	//Operating modes for camera control
	const long NORMAL_OPERATING_MODE = 0;
	const long BULB_OPERATING_MODE = 1;

	switch(_triggerMode)
	{
	case ICamera::SW_MULTI_FRAME:
	case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		{
			pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, _triggerMode);
		}
		break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH_BULB:
		{
			pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::HW_MULTI_FRAME_TRIGGER_EACH_BULB);
		}
		break;
	default:
		{
			//revert back to not using pockels reference
			pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF,FALSE);
			StringCbPrintfW(message,MSG_LENGTH,L"RunSample Execute trigger mode (%d) is not supported",_triggerMode);
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
			return FALSE;
		}
	}

	return TRUE;
}

long AcquireTStream::PrepareAndStartAcquisition(ICamera* pCamera, long& dmaFrames, SaveParams& sp, long zFastEnableGUI,												
												long cameraType, long stimulusTriggering, long captureMode)
{
	//setup stimulus device 
	if((TRUE == stimulusTriggering) && (IExperiment::HYPERSPECTRAL != captureMode))
	{
		long startStimDev;
		IDevice * pStimulusDevice = GetDevice(SelectedHardware::SELECTED_EPHYS);
		if(NULL == pStimulusDevice)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"RunSample Ephys device is invalid");
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
			FailedAcquisition(pCamera, sp, zFastEnableGUI);
			MessageBox(NULL,L"ThorElectroPhysSettings.xml is not correctly configured. Please update it before starting a Stimulus capture.",L"Ephys Settings not configured",MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);	
			return FALSE;
		}

		startStimDev = pStimulusDevice->PreflightPosition();
		//read stimulus device by GetParam, no need to invoke start which is reserved for digital switch
		if(FALSE == startStimDev)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"RunSample unable to setup Ephys device");
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
			pStimulusDevice->PostflightPosition();
			FailedAcquisition(pCamera, sp, zFastEnableGUI);
			MessageBox(NULL,L"Resource conflict or ThorElectroPhysSettings.xml is not correctly configured. Please correct it before starting a Stimulus capture.",L"Invalid Ephys Settings",MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);	
			return FALSE;
		}	
		StringCbPrintfW(message,MSG_LENGTH,L"RunSample Setup Ephys device complete");
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
	}

	//if the capture mode is hyperspectral, get the filter started before the camera gets started.
	if (IExperiment::HYPERSPECTRAL == captureMode)
	{
		pCamera->SetParam(ICamera::PARAM_EXPOSURE_TIME_MS, 1.00);
		// make sure the camera is off
		pCamera->PostflightAcquisition(NULL); 
		SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_CONTROLMODE, IDevice::KuriosControlMode::MANUAL, false);
		long wavelengthStart, wavelengthStop, stepSize, bandwidthMode;
		string path;
		_pExp->GetSpectralFilter(wavelengthStart, wavelengthStop, stepSize, bandwidthMode, path);

		//There is a bug in the firmware where if the kurios is already set to the first wavelength of the sequence
		//it will error out. Set to a different value to avoid this bug
		//Also, We need to start the camera and set the trigger once if it is in bulb mode (it is for hyperSpectral)
		//The need to do this seems to be an issue in the TSI SDK. TODO: Find a prettier way to get around this bug
		if(wavelengthStart <= wavelengthStop)
		{
			SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_WAVELENGTH, wavelengthStart+1, false);
		}
		else
		{
			SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_WAVELENGTH, wavelengthStart-1, false);
		}

		//SWITCHDELAY parameter is set to 300. This number seems to be the best for overall reliability of the bulb exposure trigger. 
		SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_SWITCHDELAY, 300, false);

		//set the trigger time mode to ENABLE_BULB
		SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_TRIGGEROUTTIMEMODE, IDevice::KuriosTriggerOutTimeMode::ENABLE_BULB, false);
		Sleep(10);

		//We need to set up the filter and change the mode to SEQUENCE_EXT before we start the camera. Because the change of mode can trigger the camera.
		SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_CONTROLMODE, IDevice::KuriosControlMode::SEQUENCE_EXT, false);
	}

	//Preflight the camera to get all the settings ready
	if (FALSE == pCamera->PreflightAcquisition(_pMemoryBuffer))
	{
		FailedAcquisition(pCamera, sp, zFastEnableGUI);
		return FALSE;
	}
	//Setup the Z Stage after the preflight. This ensures the settings have
	//been preflighted to the camera and the responses for frame rate are accurate
	if(TRUE == zFastEnableGUI)
	{
		if(FALSE == SetupZStage(1, pCamera, NULL))
		{
			StringCbPrintfW(message,MSG_LENGTH,L"RunSample SetupZStage Failed");
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
			FailedAcquisition(pCamera, sp, zFastEnableGUI);
			return FALSE;
		}
	}

	SetPMT();


	double ledPower1 = 0, ledPower2 = 0, ledPower3 = 0, ledPower4 = 0, ledPower5 = 0, ledPower6 = 0;
	SetLEDs(_pExp, pCamera, 0, ledPower1, ledPower2, ledPower3, ledPower4, ledPower5, ledPower6);

	//Start the scanner if LSM:
	if(cameraType == ICamera::CameraType::LSM)
	{
		ScannerEnable(TRUE);
	}

	if (FALSE == pCamera->SetupAcquisition(_pMemoryBuffer))
	{
		FailedAcquisition(pCamera, sp, zFastEnableGUI);
		return FALSE;
	}

	//get real dma buffer from camera since it could be limited
	double dVal = 0;
	if (pCamera->GetParam(ICamera::PARAM_LSM_DMA_BUFFER_COUNT,dVal) && (0 < dVal))
	{
		dmaFrames = static_cast<long>(dVal);
	}

	//open the shutter before finite stream if LSM:
	if((STORAGE_FINITE == sp.storageMode) && (cameraType == ICamera::CameraType::LSM))
	{
		OpenShutter();
	}

	if(FALSE == pCamera->StartAcquisition(_pMemoryBuffer))
	{
		//HW timed out:
		FailedAcquisition(pCamera, sp, zFastEnableGUI);
		return FALSE;
	}

	// The camera misses the first trigger from the Kurios at it's lowest settings. Need to give it some time to arm.
	if(ICamera::CameraType::CCD == cameraType && IExperiment::HYPERSPECTRAL == captureMode)
	{
		Sleep(1500); //This is measured to be the minimum required for the cameras that we had available.
	}

	if (IExperiment::HYPERSPECTRAL == captureMode)
	{
		SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_FORCETRIGGER, TRUE, false);	
		//long switchDelay = 0;
		//GetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_SWITCHDELAY, switchDelay);
		//std::ofstream outfile ("c:/Temp/switch.txt");
		//outfile << "Switch Delay: " << switchDelay << endl; // I also tried replacing endl with a "\n"
		//outfile.close();
	}

	return TRUE;
}

void AcquireTStream::UnlockImages(map<long, long>& idsMap, long unlockFrameID)
{
	//assuming the values of <key,value> map are image ids:
	std::map<long, long>::iterator it = idsMap.begin();
	while (it != idsMap.end())
	{
		try
		{
			long imageID = it->second; // Accessing VALUE from element pointed by it.
			if (0 <= unlockFrameID)	ImageManager::getInstance()->UnlockImagePtr(imageID,0,0,0,unlockFrameID);			
		}
		catch(...)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"RunSample AcquireTStream::UnlockImages unable to unlock images.");
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		it++;
	}
}

void AcquireTStream::DestroyImages(map<long, long>& idsMap)
{
	//assuming the values of <key,value> map are image ids:
	std::map<long, long>::iterator it = idsMap.begin();
	while (it != idsMap.end())
	{
		try
		{
			long imageID = it->second; // Accessing VALUE from element pointed by it.
			ImageManager::getInstance()->DestroyImage(imageID);
		}
		catch(...)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"RunSample AcquireTStream::DestroyImages unable to detroy images.");
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		it++;
	}
	idsMap.clear();	//clear map since no image available after destroy
}

void AcquireTStream::FailedAcquisition(ICamera* pCamera, SaveParams& sp, long zFastEnableGUI)
{
	CloseShutter();
	StringCbPrintfW(message,MSG_LENGTH,L"RunSample pCamera or pZStage Failed acquisition");
	logDll->TLTraceEvent(ERROR_EVENT,1,message);
	pCamera->PostflightAcquisition(NULL);
	ScannerEnable(FALSE);

	DestroyImages(sp.imageIDsMap); 
	DestroyImages(sp.regionImageIDsMap); 

	if(TRUE == zFastEnableGUI)
	{
		//Reset the fastZ device before returning
		IDevice* pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);
		pZStage->PostflightPosition();
		pZStage->SetParam(IDevice::PARAM_Z_ANALOG_MODE, ZPiezoAnalogMode::ANALOG_MODE_SINGLE_POINT);
	}
	pCamera->SetParam(ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY, FALSE);
	pCamera->SetParam(ICamera::PARAM_LSM_POWER_RAMP_ENABLE, FALSE);
	//revert back to not using pockels reference
	pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF,FALSE);
}

long AcquireTStream::BreakOutWaitCameraStatus(ICamera* pCamera, SaveParams& sp, long& status, double& droppedFrameCnt, long totalFrame, long zFastEnableGUI, long saveEnabledChannelsOnly, long captureMode)
{
	long doBreak = FALSE;
	long stopStatus = 0;
	status = ICamera::STATUS_BUSY;
	while((ICamera::STATUS_BUSY == status) || (ICamera::STATUS_PARTIAL == status))
	{
		//Try to control shutter, but will repeat at rising/falling edge:
		if(STORAGE_STIMULUS == sp.storageMode)
		{
			GetStimulusActive();				
			ControlShutterInStream(zFastEnableGUI);
		}

		if(FALSE == pCamera->StatusAcquisition(status) || (0 < static_cast<long>(droppedFrameCnt)))
		{
			break;
		}
		StopCaptureEventCheck(stopStatus);
		//user has asked to stop the capture
		if(1 == stopStatus)
		{
			//close shutter before waiting for  
			//user's response regarding file:
			CloseShutter();
			//stop ZStage:
			if(1 == zFastEnableGUI)
			{
				SetupZStage(2, NULL, NULL);
			}
			if(STORAGE_FINITE == sp.storageMode)
			{
				if(0 == _messageID)
				{
					pCamera->PostflightAcquisition(NULL);
					ScannerEnable(FALSE);
					_messageID = MessageBox(NULL,L"Experiment stopped. Would you like to save the already acquired images?",L"Save Experiment Files",MB_YESNO | MB_SETFOREGROUND | MB_ICONWARNING | MB_SYSTEMMODAL);	
				}			
				//Allow save files based on user's decision:
				if(IDYES == _messageID)			
				{							
					break;
				}
				else if(IDNO == _messageID)
				{	
					pCamera->SetParam(ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY, FALSE);

					//revert back to not using pockels reference
					pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF,FALSE);

					// This is a work-around for a problem found with an 8050 CamLink camera. Consecutive experiments
					// would not trigger the camera correctly. This solution seems to solve that problem which might
					// happen for CamLink or old cameras. It doesn't seem to have any impact on the other cameras.
					if (IExperiment::HYPERSPECTRAL == captureMode)
					{
						pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, _triggerMode);
						pCamera->PreflightAcquisition(_pMemoryBuffer);
					}

					UnlockImages(sp.imageIDsMap, totalFrame);

					return TRUE;	
				}
			}
			else
			{	
				break;
			}
		}
	}
	//============================================================
	//    End Camera Busy Wait Loop
	//============================================================
	if (ICamera::STATUS_ERROR == status)
	{
		if(!useVirtualMemory)
		{
			UnlockImages(sp.imageIDsMap, totalFrame);
		}

		//no copy dropped frames when camera has error
		droppedFrameCnt = 0;
		doBreak = TRUE;
	}
	return doBreak;
}


long AcquireTStream::Execute(long index, long subWell)
{	
	double magnification;
	string objName;

	_pExp->GetMagnification(magnification, objName);
	//Get filter parameters from hardware setup.xml
	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML());

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
	long currentTCount = 0;

	pHardware->GetMagInfoFromName(objName,magnification,position,numAperture,afStartPos,afFocusOffset,afAdaptiveOffset,beamExpPos,beamExpWavelength,beamExpPos2,beamExpWavelength2,turretPosition,zAxisToEscape,zAxisEscapeDistance,fineAutoFocusPercentage);

	if(FALSE == PreCaptureAutoFocus(index, subWell,  afStartPos, afAdaptiveOffset))
	{
		return FALSE;
	}

	ICamera *pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);
	if(NULL == pCamera)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create camera");
		return FALSE;
	}

	long cameraType = ICamera::CameraType::LAST_CAMERA_TYPE;
	cameraType = (GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_TYPE,cameraType)) ? cameraType : ICamera::CameraType::LAST_CAMERA_TYPE;

	long lsmType = ICamera::LSMType::LSMTYPE_LAST;
	lsmType = (GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_TYPE,lsmType)) ? lsmType : ICamera::LSMType::LSMTYPE_LAST;

	long streamEnable,streamFrames,rawData,displayImage,storageMode,zFastEnableGUI,zFastMode,flybackFrames,flybackLines,previewIndex,stimulusTriggering,stimulusMaxFrames,useReferenceVoltageForFastZPockels,dmaFrames;
	double flybackTimeAdjustMS = 0, volumeTimeAdjustMS = 0, stepTimeAdjustMS = 0;
	long displayRollingAveragePreview = FALSE;
	_pExp->GetStreaming(streamEnable, streamFrames, rawData, _triggerMode, displayImage, storageMode, zFastEnableGUI, zFastMode, flybackFrames, flybackLines, flybackTimeAdjustMS, volumeTimeAdjustMS, stepTimeAdjustMS, previewIndex, stimulusTriggering, dmaFrames, stimulusMaxFrames, useReferenceVoltageForFastZPockels, displayRollingAveragePreview);
	StringCbPrintfW(message,MSG_LENGTH,L"AcquireTStream streamFrames %d",streamFrames);
	logDll->TLTraceEvent(INFORMATION_EVENT,1,message);	

	long captureMode;
	_pExp->GetCaptureMode(captureMode);

	if (ICamera::CameraType::LSM == cameraType && ICamera::LSMType::RESONANCE_GALVO_GALVO == lsmType)
	{
		_pExp->GetScanRegions(viewMode, activeScanAreas);
		//do meso scan if single scan area
		if (MesoScanTypes::Micro == viewMode && 1 >= activeScanAreas.size())
			viewMode = MesoScanTypes::Meso;
	}
	//keep preview index then update for all scan areas:
	long previewScanAreaID = (ICamera::CameraType::LSM == cameraType && ICamera::LSMType::RESONANCE_GALVO_GALVO == lsmType && 0 < activeScanAreas.size()) ? activeScanAreas[0].RegionID : 0;

	//only allow fastZ when in a streaming capture
	//Other types of capture should not do fast z (i.e. Hyperspectral capture).
	if (IExperiment::STREAMING != captureMode)
	{
		zFastEnableGUI = FALSE;
	}
	else if(CaptureFile::FILE_BIG_TIFF == (CaptureFile)rawData)
	{
		//[STREAM ONLY] get active scan area once only since all will be enabled afterward
		GetActiveScanAreaThenEnableAll(_pExp);
		//update scan areas after all are enabled
		if (ICamera::LSMType::RESONANCE_GALVO_GALVO == lsmType)
			_pExp->GetScanRegions(viewMode, activeScanAreas);
	}

	long width, height, channel, bufferChannels, averageMode, averageNum, fieldSize;
	double umPerPixel, dwelltime;
	long numberOfPlanes = 1;
	//Set camera settings and get settings needed in this function
	SetGetCameraSettings(pCamera, channel, bufferChannels, width, height, averageMode, averageNum, umPerPixel, fieldSize, dwelltime, numberOfPlanes);

	long saveEnabledChannelsOnlyLong;
	_pExp->GetRaw(saveEnabledChannelsOnlyLong);

	long wavelengthIndex = 0, zstageSteps, timePoints,triggerModeTimelapse,zEnable;
	string wavelengthName, zstageName;
	double exposureTimeMS, zstageStepSize, intervalSec, zStartPos;
	long zStreamFrames,zStreamMode;

	_pExp->GetWavelength(wavelengthIndex,wavelengthName,exposureTimeMS);
	_pExp->GetZStage(zstageName, zEnable ,zstageSteps,zstageStepSize,zStartPos,zStreamFrames,zStreamMode);
	_pExp->GetTimelapse(timePoints,intervalSec,triggerModeTimelapse);

	_zstageStepSize = zstageStepSize;	

	SaveParams sp;
	SaveParams spThread;

	if(TRUE == zFastEnableGUI)
	{
		if(STORAGE_STIMULUS == storageMode)
		{
			streamFrames = stimulusMaxFrames;
		}
		if(streamFrames < (zstageSteps+flybackFrames))
		{
			return FALSE;
		}
		long volCnt = streamFrames/(zstageSteps + flybackFrames);
		if(FALSE == SetupZStage(0, pCamera, &sp._zInfo))
		{
			return FALSE;
		}
		//update stream frame accepted by zstage:
		zstageSteps = sp._zInfo.zFramesPerVolume;
		streamFrames = volCnt * (zstageSteps + flybackFrames);
	}

	//we are now going to save the raw data for
	//the stimulus storage mode, and do conversion at the end
	if(STORAGE_STIMULUS == storageMode)
	{
		streamFrames = stimulusMaxFrames;
		useVirtualMemory = false;	
	}
	else
	{
		//we cannot use virtual memory for finite storage mode 
		//to save the raw data
		useVirtualMemory = false;
	}		
	//Check non-zero frame num. settings:
	if((0 == streamFrames)||(0 == dmaFrames))
	{
		return FALSE;
	}

	//If Fast Z is not enabled, keep flybackFrames = 0 for SaveData:
	if(zFastEnableGUI == 0)
	{
		flybackFrames = 0;
	}

	//============================================
	// Modify Behavior if not saving raw files
	//============================================
	long saveEnabledChannelsOnly = (1 == saveEnabledChannelsOnlyLong && bufferChannels != 1) ? TRUE : FALSE;
	int totalBufferChannels = bufferChannels;
	if(saveEnabledChannelsOnly)
	{
		bufferChannels = static_cast<long>(ChannelManipulator<short>::getEnabledChannels(channel).size());
	}

	threadRawFileContainsDisabledChannels = !saveEnabledChannelsOnly;
	pCamera->SetParam(ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY, saveEnabledChannelsOnly);

	IDevice *pControlUnitDevice = NULL;
	//notify the ECU of the zoom change also
	pControlUnitDevice = GetDevice(SelectedHardware::SELECTED_CONTROLUNIT);
	if(NULL != pControlUnitDevice && ICamera::CameraType::LSM == cameraType && ICamera::LSMType::RESONANCE_GALVO_GALVO != lsmType)
	{
		if(SetDeviceParameterValue(pControlUnitDevice,IDevice::PARAM_SCANNER_ZOOM_POS,fieldSize,FALSE,NULL,0))
		{
			//If the device is an ECU2, read the alignment value from the ECU and set it back to itself, same way as capture setup LSMFieldSize->Setter
			long minFS,maxFS,fieldSizeDefault, alignment, zone, zoneECU;
			GetDeviceParamRangeLong(SelectedHardware::SELECTED_CONTROLUNIT,IDevice::PARAM_SCANNER_ZOOM_POS,minFS,maxFS,fieldSizeDefault);
			zone = maxFS - fieldSize;
			zoneECU = IDevice::PARAM_ECU_TWO_WAY_ZONE_1 + zone;
			if(GetDeviceParamLong(SelectedHardware::SELECTED_CONTROLUNIT, zoneECU, alignment))
			{
				SetDeviceParamDouble(SelectedHardware::SELECTED_CONTROLUNIT, zoneECU, alignment, FALSE);
			}
		}
	}

	SetCameraTriggerMode(pCamera);

	long startWavelength, stopWavelength, wavelengthStepSize, specrtalBandwidthMode;
	string spectralSequencePath;
	_pExp->GetSpectralFilter(startWavelength, stopWavelength, wavelengthStepSize, specrtalBandwidthMode, spectralSequencePath);
	long spectralSequenceSteps = 0;
	HeaderInfo* pHeaderInfo = nullptr;
	if (IExperiment::HYPERSPECTRAL == captureMode)
	{
		spectralSequenceSteps = abs(stopWavelength - startWavelength) / wavelengthStepSize + 1;
		//if this is a hyperspectral capture, the number of frames must match the number of wavelengths
		streamFrames = spectralSequenceSteps;

		//if this is a hyperspectral capture, there should not be any averaging
		averageMode = ICamera::AVG_MODE_NONE;
		averageNum = 1;

		pHeaderInfo = new HeaderInfo();
		pHeaderInfo->Description = "ThorImage Hyper Spectrum Image";
		pHeaderInfo->SensorType = "Camera";
		pHeaderInfo->FileType = "ENVI Standard";
		pHeaderInfo->HeaderOffset = 0;
		pHeaderInfo->Samples = width;
		pHeaderInfo->Lines = height;
		pHeaderInfo->bands = spectralSequenceSteps;
		pHeaderInfo->DataType = 12; // unsigned 16-bit integer
		pHeaderInfo->Interleave = "bsq";
		pHeaderInfo->ByteOrder = 0; // 0: little endian; 1: big endian.
		pHeaderInfo->WaveLength->clear();
		for(int wl_i = 0; wl_i < spectralSequenceSteps; wl_i++)
		{
			pHeaderInfo->WaveLength->push_back(static_cast<float>(startWavelength + wl_i * wavelengthStepSize));
		}
	}

	long setupFrames = (STORAGE_FINITE == storageMode)? streamFrames : stimulusMaxFrames;

	double dflimType = 0;
	if (pCamera->GetParam(ICamera::PARAM_DFLIM_FRAME_TYPE, dflimType))
	{
		imageMethod = static_cast<long>(dflimType);
	}

	long hyperSpectralWavelengths = (IExperiment::HYPERSPECTRAL == captureMode) ? spectralSequenceSteps : 1;

	sp._pHeaderInfo = pHeaderInfo;

	wstring streamPath;	double previewRate = 4;
	pHardware->GetStreaming(streamPath, previewRate);

	Dimensions d;
	//do not average if the fast Z is enabled, RGG can do average in camera:
	long avgFrames = ((ICamera::AVG_MODE_CUMULATIVE == averageMode && FALSE == zFastEnableGUI) && ICamera::LSMType::RESONANCE_GALVO_GALVO != lsmType) ? averageNum : 1;

	//do not average if the fast Z is enabled, RGG can do average in camera, no cumulative preview for dflim 
	//TODO: do the work for cumulative dflim, to bring this feature faster we will leave dflim for later
	long rollingAvgPreviewFrames = (TRUE == displayRollingAveragePreview && FALSE == zFastEnableGUI && averageNum > 1 && 1 != imageMethod && ICamera::LSMType::RESONANCE_GALVO_GALVO != lsmType)  ? averageNum : 1;

	long multiFrameCount = (STORAGE_STIMULUS == storageMode) ? static_cast<long>(floor((double)streamFrames / avgFrames))*avgFrames	//keep the same frame count per stimulus
		: (IExperiment::HYPERSPECTRAL == captureMode) ? 1 : streamFrames*avgFrames;
	pCamera->SetParam(ICamera::PARAM_MULTI_FRAME_COUNT, multiFrameCount);

	//keep the maximum size on top for buffer assignment safety, 
	//activeScanAreas will be decedent order by frame size afterward:
	std::sort(activeScanAreas.begin(),activeScanAreas.end(),[](const ScanRegion &lhs, const ScanRegion &rhs){ return lhs.SizeX * lhs.SizeY > rhs.SizeX * rhs.SizeY; });

	SetupSaveParams(index, subWell, setupFrames, exposureTimeMS, width, height, numberOfPlanes, umPerPixel, zFastEnableGUI, zstageSteps, flybackFrames, totalBufferChannels, channel, storageMode, hyperSpectralWavelengths, rawData, previewScanAreaID, &sp);

	if(FALSE == SetupImageData(streamPath, pCamera, averageMode, &sp, zFastEnableGUI, bufferChannels, dwelltime, avgFrames ,d))
	{
		pCamera->SetParam(ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY, false);
		return FALSE;
	}

	//#ifdef USE_VIRTUAL_ALLOC
	char * pVirtualMemory = NULL;
	if(useVirtualMemory)
	{	
		SIZE_T memSize = width;
		memSize *= height;
		memSize *= bufferChannels;
		memSize *= streamFrames;
		memSize *= avgFrames;
		memSize *= 2; 
		pVirtualMemory = (char*)VirtualAlloc(NULL, memSize,MEM_COMMIT,PAGE_READWRITE);
		DWORD getError = GetLastError();
	}
	//#endif

	char* pMemoryBufferDFLIMHisto = NULL; 
	char* pMemoryBufferDFLIMSinglePhoton = NULL;
	char* pMemoryBufferDFLIMArrivalTimeSum = NULL;
	char* pMemoryBufferDFLIMPhotons = NULL;

	char* pMemoryBufferDFLIMPreview = NULL;

	UINT64 dflimPhotonListOffset = 0; //offset to extract the right pointer to copy to

	//initialize the progress indicator, check if it is a hyperspectral capture with tiling
	if(IExperiment::HYPERSPECTRAL == captureMode)
	{
		// if tiling is enabled, initialize the progess indicator only for the first subwell
		if(2 > subWell)
		{
			CallSaveImage(0, TRUE);
		}
	}
	else
	{
		//initialize the progress indicator
		CallSaveImage(0, TRUE);
	}

	long streamCount;	
	if(STORAGE_FINITE == storageMode)
	{ 
		streamCount = streamFrames;	
		//open the shutter for finite stream:
		if((cameraType != ICamera::CameraType::LSM))
		{
			OpenShutter();
		}
	}
	else 
	{ 
		streamCount = INT_MAX;
	}

	// image preview buffer to fast z top, middle, bottom images
	char * pTilesImagePreviewBuffer = NULL;
	int tilesInPreview = 1;
	long camBufSize = width * height * bufferChannels;
	long tileBufferSizeBytes = width * height * totalBufferChannels * sizeof(USHORT);

	// allocate fast z tiled preview image buffer
	if(zFastEnableGUI) {
		tilesInPreview = 4; // top, middle, bottom, user
		pTilesImagePreviewBuffer = new char[tilesInPreview * tileBufferSizeBytes];
		memset(pTilesImagePreviewBuffer, 0, tilesInPreview * tileBufferSizeBytes);
	}

	//1 datalength for photon num buffer (intensity) (USHORT)
	//1 datalength for single photon sum buffer (USHORT)
	//2 datalength for arrival time sum buffer (UINT32)
	//2 DFLIM_HISTOGRAM_BINS for dflim histogram (UINT32)	
	const int intensityBufferSize = width * height * bufferChannels * sizeof(USHORT);		
	const int DFLIM_HISTOGRAM_BINS = 256;
	const int dflimHistrogramBufferSize = DFLIM_HISTOGRAM_BINS * bufferChannels * sizeof(UINT32);				
	const int dflimArrivalTimeSumBufferSize = width * height * bufferChannels * sizeof(UINT32);
	const int dflimSinglePhotonSumBufferSize = width * height * bufferChannels * sizeof(USHORT);
	const int dflimPreviewSize = intensityBufferSize + dflimHistrogramBufferSize + dflimArrivalTimeSumBufferSize + dflimSinglePhotonSumBufferSize;
	if (1 == imageMethod) //if dflim capture
	{
		pMemoryBufferDFLIMPreview = new char[dflimPreviewSize];
		memset(pMemoryBufferDFLIMPreview, 0, dflimPreviewSize);
	}

	//allocate a summing buffer if averaging is going to occur
	UCHAR* pSumMemoryBuffer = NULL;
	UCHAR* pSumBufferDFLIMHisto = NULL;
	UCHAR* pSumBufferDFLIMArrivalTimeSum = NULL;
	UCHAR* pSumBufferDFLIMSinglePhoton = NULL;

	long previewAverageCount = 0;
	if(avgFrames > 1 && rollingAvgPreviewFrames <= 1)
	{
		if (1 == imageMethod)
		{
			//1 datalength for photon num buffer (intensity) (USHORT)
			//1 datalength for single photon sum buffer (USHORT)
			//2 datalength for arrival time sum buffer (UINT32)
			//2 DFLIM_HISTOGRAM_BINS for dflim histogram (UINT32)
			const int intensitySumBufferSize = intensityBufferSize * avgFrames;		
			const int DFLIM_HISTOGRAM_BINS = 256;
			const int dflimHistrogramSumBufferSize = dflimHistrogramBufferSize * avgFrames;				
			const int dflimArrivalTimeSumSumBufferSize = dflimArrivalTimeSumBufferSize * avgFrames;
			const int dflimSinglePhotonSumSumBufferSize = dflimSinglePhotonSumBufferSize * avgFrames;

			pSumMemoryBuffer = new UCHAR[intensitySumBufferSize];
			memset(pSumMemoryBuffer, 0, intensitySumBufferSize);

			pSumBufferDFLIMHisto = new UCHAR[dflimHistrogramSumBufferSize];
			memset(pSumBufferDFLIMHisto, 0, dflimHistrogramSumBufferSize);

			pSumBufferDFLIMArrivalTimeSum = new UCHAR[dflimArrivalTimeSumSumBufferSize];
			memset(pSumBufferDFLIMArrivalTimeSum, 0, dflimArrivalTimeSumSumBufferSize);

			pSumBufferDFLIMSinglePhoton = new UCHAR[dflimSinglePhotonSumSumBufferSize];
			memset(pSumBufferDFLIMSinglePhoton, 0, dflimSinglePhotonSumSumBufferSize);
		}
		else
		{
			pSumMemoryBuffer = new UCHAR[sizeof(USHORT) * camBufSize * avgFrames];
			memset(pSumMemoryBuffer, 0, sizeof(USHORT) * camBufSize * avgFrames);
		}
	}

	if (rollingAvgPreviewFrames > 1)
	{
		pSumMemoryBuffer = new UCHAR[sizeof(USHORT) * camBufSize];
		memset(pSumMemoryBuffer, 0, sizeof(USHORT) * camBufSize);

		if (1 == imageMethod)
		{
			pSumBufferDFLIMHisto = new UCHAR[dflimHistrogramBufferSize];
			memset(pSumBufferDFLIMHisto, 0, dflimHistrogramBufferSize);

			pSumBufferDFLIMArrivalTimeSum = new UCHAR[dflimArrivalTimeSumBufferSize];
			memset(pSumBufferDFLIMArrivalTimeSum, 0, dflimArrivalTimeSumBufferSize);

			pSumBufferDFLIMSinglePhoton = new UCHAR[dflimSinglePhotonSumBufferSize];
			memset(pSumBufferDFLIMSinglePhoton, 0, dflimSinglePhotonSumBufferSize);
		}
	}

	sp.avgFrames = avgFrames;
	sp.useVirtualMemory = useVirtualMemory;

	//=================================================
	//    Initialize PARAMS Before Start
	//=================================================

	//reset history time for preview:
	_lastImageUpdateTime = 0;
	//this is the total number of images acquired in a stimulus
	long tStimulus = 1;
	long currentStreamFrames = 1;
	long stopStatus = 0, startCamStatus = 1;
	//this is camera status at latest check
	long status = ICamera::STATUS_BUSY;
	long stimulusTransitionCount = 0;
	//this is actual number of frames retrieved from the camera
	long currentStreamCount = 1;
	//this is the number frames saved from the retrieved camera images 
	long t = 1;
	//this is the Z frame number in one FastZ volume (including flybackFrames, 0 if FastZ is not enabled):
	long tFastZStimulus = 0;
	//this is number of times exceeding max stream frames:
	long saveMaxStreamFramesCount = 0;
	//this is the last fast z frame before next max stream frames:
	long fastzLastFrameID = 0;
	//this is number of fast z volume number within current max stream frames:
	long fastzVolumeNum = 0;
	//this is the last stream frame if average enabled:
	long avgStimulusFrames = 1;
	//this is user defined message response:
	_messageID = 0;
	//post-processing with ImageManager:
	long imageMode = TRUE;
	//this is the dropped frame count:
	double droppedFrameCnt = 0;
	//this is the number of frames available to be copied in the lower level DMA circular buffer:
	double dmaBufferAvailableFrames = 0;
	//This is the total number of frames put in to the final raw file (including flyback frames)
	long totalSavedImages = 0;
	//This is the total number of captured frames used to update the GUI
	long capturedImagesCtrStimulusGUI = 1;
	//This is the type of CCD camera that is currently selected
	double ccdType = ICamera::CMOS;
	//This is the thread handle collection of saving threads
	_threadHandles.clear();

	//this will be true in doing FastZ Stimulus:
	BOOL fastZActive = FALSE;	
	//this will be true if scanner is stopped already:
	BOOL scannerStopped = FALSE;
	//this will be true if hardware trigger is present:
	_hwStimulusActive = FALSE;
	//this will be true if software stimulus is present:
	_swStimulusActive = FALSE;

	_saveThreadCount = 0;
	_saveThreadCountFinished = 0;

	InitializeCriticalSection(&saveAccess);

	PhysicalSize physicalSize;	// unit: um

	double res = round(umPerPixel*Constants::UM_TO_MM)/Constants::UM_TO_MM;	// keep 2 figures after decimal point, that is why 100 (10^2) is multiplied
	physicalSize.x = res;
	physicalSize.y = res;
	physicalSize.z = _zstageStepSize;

	enum
	{
		zTop = 1,
		zMid = 0,
		zBot = 2,
		zUsr = 3,
	};
	long updateRestTiles = FALSE;

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	imgNameFormat << L"Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";

	vector<pair<long,long> > activeChannels = getActiveChannels(sp);
	//this will be updated by scanner after CopyAcquisition:
	FrameInfo frameInfo = {-1};

	if (!PrepareAndStartAcquisition(pCamera,dmaFrames,sp,zFastEnableGUI,cameraType,stimulusTriggering,captureMode))
	{
		wchar_t drive[_MAX_DRIVE];
		wchar_t dir[_MAX_DIR];
		wchar_t fname[_MAX_FNAME];
		wchar_t ext[_MAX_EXT];
		wchar_t rawPath[_MAX_PATH];
		_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
		StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s",drive,dir);
		wchar_t tempFileName[_MAX_FNAME];
		StringCbPrintfW(tempFileName,_MAX_FNAME,imgNameFormat.str().c_str(),index,subWell);
		DeleteFile((wstring(rawPath) + wstring(tempFileName)).c_str());

		return FALSE;
	}

	//=================================================
	//    PARAMS After Setup & Start Camera
	//=================================================

	//No preview if occupied buffer is over the limit
	double dmaInUseLimit = (20 <= dmaFrames) ? 0.90 : 0.60;

	//Change the limit if it is an ORCA to throttle the preview image rate sooner if the dma buffer is filling up quickly
	pCamera->GetParam(ICamera::PARAM_CCD_TYPE, ccdType);
	dmaInUseLimit = (ICamera::ORCA == ccdType) ? 0.3 : dmaInUseLimit;

	//Read the size of the dma buffer from the lower level and use that if it is larger
	double dmaFrm = 0;
	pCamera->GetParam(ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT, dmaFrm);
	dmaFrames = max(static_cast<long>(dmaFrm), dmaFrames);

	//=================================================
	//    Aqcuire Loop
	//=================================================

	int currentAcquired = 0;
	while(t<=streamCount)
	{

		//===============================================
		//   Average Loop
		//===============================================
		for(long i = 0; i < avgFrames; i++)
		{
			if((i == 0) && (STORAGE_FINITE == storageMode))
			{
				if((subWell == 1) && (t == 1))
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
			int totalFrame = (t - 1) * avgFrames + i;		


			//=========================================================
			//   Get Image Memory Buffer
			//==========================================================
			//#ifdef USE_VIRTUAL_ALLOC
			if(useVirtualMemory)
			{
				_pMemoryBuffer = pVirtualMemory + width * height * bufferChannels * totalFrame*2;
			}
			//#else
			else
			{
				if (1 == imageMethod) //dFlim capture
				{
					pMemoryBufferDFLIMHisto = ImageManager::getInstance()->GetImagePtr(sp.imageIDsMap[BufferType::DFLIM_HISTOGRAM], 0, 0, 0, totalFrame);
					pMemoryBufferDFLIMSinglePhoton = ImageManager::getInstance()->GetImagePtr(sp.imageIDsMap[BufferType::DFLIM_IMAGE_SINGLE_PHOTON], 0, 0, 0, totalFrame);
					pMemoryBufferDFLIMArrivalTimeSum = ImageManager::getInstance()->GetImagePtr(sp.imageIDsMap[BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM], 0, 0, 0, totalFrame);
					pMemoryBufferDFLIMPhotons = ImageManager::getInstance()->GetImagePtr(sp.imageIDsMap[BufferType::DFLIM_PHOTONS], 0, 0, 0, totalFrame, dflimPhotonListOffset);
				}
				//**Update to actual frame number*//
				_pMemoryBuffer = (MesoScanTypes::Micro == viewMode) ? sp.pMemoryBuffer : ImageManager::getInstance()->GetImagePtr(sp.imageIDsMap[BufferType::INTENSITY], 0, 0, 0, totalFrame);
			} 
			//#endif		
			if(_pMemoryBuffer == NULL)
			{
				StringCbPrintfW(message,MSG_LENGTH,L"AcquireTStream invalid memory buffer");
				logDll->TLTraceEvent(ERROR_EVENT,1,message);
				status = ICamera::STATUS_ERROR;
				break;
			}

			//=================================================
			//    Camera Busy Condition
			//=================================================			
			DWORD frameStartTime = GetTickCount();

			if (TRUE == BreakOutWaitCameraStatus(pCamera, sp, status, droppedFrameCnt, totalFrame, zFastEnableGUI, saveEnabledChannelsOnly, captureMode))
			{
				break;
			}
			//=================================================
			//    Copy Buffer from Camera
			//=================================================
			if ((MesoScanTypes::Micro == viewMode))
			{
				for (int i = 0; i < sp.regionImageIDsMap.size(); i++)
				{
					//wait before copy next other than the first:
					if (0 < i)
					{
						if (TRUE == BreakOutWaitCameraStatus(pCamera, sp, status, droppedFrameCnt, totalFrame, zFastEnableGUI, saveEnabledChannelsOnly, captureMode))
							break;
					}
					pCamera->CopyAcquisition(_pMemoryBuffer, &frameInfo);

					//redirect copied buffer to target region by get-then-unlock, no need to unlock later on:
					if (sp.regionImageIDsMap.find(frameInfo.scanAreaID) != sp.regionImageIDsMap.end())
					{
						StringCbPrintfW(message,MSG_LENGTH,L"RunSample:%hs@%u: copying scan area %d, with camera status %d", __FILE__, __LINE__, frameInfo.scanAreaID, status);
						logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

						sp.regionMap[frameInfo.scanAreaID].SizeX = frameInfo.imageWidth;
						sp.regionMap[frameInfo.scanAreaID].SizeY = frameInfo.imageHeight;
						sp.bufferChannels[frameInfo.scanAreaID] = frameInfo.channels;
						size_t bufSize = sp.regionMap[frameInfo.scanAreaID].SizeX * sp.regionMap[frameInfo.scanAreaID].SizeY * sp.bufferChannels[frameInfo.scanAreaID] * sizeof(USHORT);
						if (bufSize != frameInfo.copySize)
						{
							StringCbPrintfW(message,MSG_LENGTH,L"RunSample:%hs@%u: different copied buffer size.", __FILE__, __LINE__);
							logDll->TLTraceEvent(WARNING_EVENT,1,message);
						}
						sp.regionMap[frameInfo.scanAreaID].BufferSize = bufSize;
						char* pTgtBuffer = ImageManager::getInstance()->GetImagePtr(sp.regionImageIDsMap[frameInfo.scanAreaID], 0, 0, 0, totalFrame);
						SAFE_MEMCPY(pTgtBuffer, bufSize, _pMemoryBuffer);
						ImageManager::getInstance()->UnlockImagePtr(sp.regionImageIDsMap[frameInfo.scanAreaID], 0, 0, 0, totalFrame);
					}

					//check dropped frames per copy:
					pCamera->GetParam(ICamera::PARAM_DROPPED_FRAMES,droppedFrameCnt);
				}
			}
			else
			{
				frameInfo.bufferType = BufferType::INTENSITY;
				pCamera->CopyAcquisition(_pMemoryBuffer, &frameInfo);
			}
			if (1 == imageMethod)	//dFlim capture
			{
				frameInfo.bufferType = BufferType::DFLIM_HISTOGRAM;
				pCamera->CopyAcquisition(pMemoryBufferDFLIMHisto, &frameInfo);
				frameInfo.bufferType = BufferType::DFLIM_IMAGE_SINGLE_PHOTON;
				pCamera->CopyAcquisition(pMemoryBufferDFLIMSinglePhoton, &frameInfo);
				frameInfo.bufferType = BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM;
				pCamera->CopyAcquisition(pMemoryBufferDFLIMArrivalTimeSum, &frameInfo);
				frameInfo.bufferType = BufferType::DFLIM_PHOTONS;
				pCamera->CopyAcquisition(pMemoryBufferDFLIMPhotons, &frameInfo);
				dflimPhotonListOffset += frameInfo.copySize;
			}
			spThread = sp;							//set thread's saveparams after scanner update
			++currentAcquired;
			//save timing info:
			if ((STORAGE_STIMULUS == storageMode))
			{
				if((subWell == 1) && (tStimulus == 1))
				{
					_acquireSaveInfo->getInstance()->SetExperimentStartCount();	//set current timer count as start of the experiment
				}
				_acquireSaveInfo->getInstance()->AddTimingInfo();
				_acquireSaveInfo->getInstance()->AddTimestamp();
			}

			//Start counting Z Frames when FastZ is enabled for Stimulus Streaming:
			if(1 == zFastEnableGUI)
			{
				(tFastZStimulus % (zstageSteps+flybackFrames) == 0) ? (tFastZStimulus=1) : (tFastZStimulus++);				
			}

			if (TRUE == displayRollingAveragePreview && rollingAvgPreviewFrames > 1)
			{
				long currentAvg = rollingAvgPreviewFrames > currentAcquired ? currentAcquired : rollingAvgPreviewFrames;
				if (1 == imageMethod) //if dflim acquisition
				{
					CalculateRollingAverageAndSumDFLIM((USHORT*)pSumMemoryBuffer, (USHORT*)pSumBufferDFLIMSinglePhoton, (ULONG32*)pSumBufferDFLIMArrivalTimeSum, (ULONG32*)pSumBufferDFLIMHisto, (USHORT*)_pMemoryBuffer, (USHORT*)pMemoryBufferDFLIMSinglePhoton, (ULONG32*)pMemoryBufferDFLIMArrivalTimeSum, (ULONG32*)pMemoryBufferDFLIMHisto, currentAvg, camBufSize);
				}
				else
				{
					CalculateRollingAverage((USHORT*)pSumMemoryBuffer, (USHORT*)_pMemoryBuffer, currentAvg, camBufSize);
				}
			}

			//==============================================================================================================================
			//  Preview Image: preview first frame, then throttle display rate afterward; also only preview active scan area in micro view.
			//==============================================================================================================================
			if((0 == _lastImageUpdateTime) || (GetTickCount() - _lastImageUpdateTime) > Constants::MS_TO_SEC*(1 / previewRate))
			{
				if((displayImage) && (dmaInUseLimit > static_cast<double>(droppedFrameCnt / dmaFrames)) && (dmaInUseLimit > static_cast<double>(dmaBufferAvailableFrames / dmaFrames)))	//no preview if camera is close to overflow limit
				{
					//only preview on selected scan area by get-then-unlock
					if (MesoScanTypes::Micro == viewMode && sp.regionImageIDsMap.find(sp.previewRegionID) != sp.regionImageIDsMap.end())
					{
						char* pTmpBuffer = ImageManager::getInstance()->GetImagePtr(sp.regionImageIDsMap[sp.previewRegionID], 0, 0, 0, totalFrame);
						SAFE_MEMCPY(_pMemoryBuffer, sp.regionMap[sp.previewRegionID].BufferSize, pTmpBuffer);
						ImageManager::getInstance()->UnlockImagePtr(sp.regionImageIDsMap[sp.previewRegionID], 0, 0, 0, totalFrame);
					}
					if(!zFastEnableGUI) 
					{
						if(avgFrames > 1 && rollingAvgPreviewFrames <= 1)
						{
							memcpy(pSumMemoryBuffer + ((previewAverageCount % avgFrames) * camBufSize * sizeof(USHORT)), _pMemoryBuffer, (camBufSize * sizeof(USHORT)));
							if (1 == imageMethod) //if dflim acquisition
							{
								//1 datalength for photon num buffer (intensity) (USHORT)
								//1 datalength for single photon sum buffer (USHORT)
								//2 datalength for arrival time sum buffer (UINT32)
								//2 DFLIM_HISTOGRAM_BINS for dflim histogram (UINT32)
								memcpy(pSumBufferDFLIMHisto + ((previewAverageCount % avgFrames) * dflimHistrogramBufferSize), pMemoryBufferDFLIMHisto, dflimHistrogramBufferSize);
								memcpy(pSumBufferDFLIMArrivalTimeSum + ((previewAverageCount % avgFrames) * dflimArrivalTimeSumBufferSize), pMemoryBufferDFLIMArrivalTimeSum, dflimArrivalTimeSumBufferSize);
								memcpy(pSumBufferDFLIMSinglePhoton + ((previewAverageCount % avgFrames) * dflimSinglePhotonSumBufferSize), pMemoryBufferDFLIMSinglePhoton, dflimSinglePhotonSumBufferSize);
							}

							if(previewAverageCount >= avgFrames - 1)
							{
								USHORT* pSum = (USHORT*) pSumMemoryBuffer;
								UINT32* pSumDFLIMHisto = (UINT32*) pSumBufferDFLIMHisto;
								UINT32* pSumDFLIMArrivalTimeSum = (UINT32*) pSumBufferDFLIMArrivalTimeSum;
								USHORT* pSumDFLIMSinglePhotonSum = (USHORT*) pSumBufferDFLIMSinglePhoton;
								for(int n = 0; n < camBufSize; n++)
								{
									double averSum = 0.0;
									UINT32 sumDFLIMArrivalTimeSum = 0;
									USHORT sumDFLIMSinglePhotonSum = 0;
									for(int m = 0; m < avgFrames; m++)
									{
										averSum += *(pSum + m * camBufSize + n);
										if (1 == imageMethod) //if dflim acquisition
										{
											sumDFLIMArrivalTimeSum += *(pSumDFLIMArrivalTimeSum + m * camBufSize + n);
											sumDFLIMSinglePhotonSum += *(pSumDFLIMSinglePhotonSum + m * camBufSize + n);
										}
									}

									*(pSum + n) = (int)(averSum / avgFrames + 0.5);
									if (1 == imageMethod) //if dflim acquisition
									{
										*(pSumDFLIMArrivalTimeSum + n) = sumDFLIMArrivalTimeSum;
										*(pSumDFLIMSinglePhotonSum + n) = sumDFLIMSinglePhotonSum;
									}
								}

								if (1 == imageMethod) //if dflim acquisition
								{
									const int DFLIM_HISTOGRAM_BINS = 256;
									for(int n = 0; n < DFLIM_HISTOGRAM_BINS; n++)
									{
										double averSum = 0.0;
										UINT32 sumDFLIMHisto = 0;

										for(int m = 0; m < avgFrames; m++)
										{
											averSum += *(pSum + m * camBufSize + n);
											if (1 == imageMethod) //if dflim acquisition
											{
												sumDFLIMHisto += *(pSumDFLIMHisto + m * DFLIM_HISTOGRAM_BINS + n);
											}
										}

										*(pSumDFLIMHisto + n) = sumDFLIMHisto;
									}

									memcpy(pMemoryBufferDFLIMPreview, pSumDFLIMHisto, dflimHistrogramBufferSize);
									memcpy(pMemoryBufferDFLIMPreview + dflimHistrogramBufferSize, pSum, intensityBufferSize);
									memcpy(pMemoryBufferDFLIMPreview + dflimHistrogramBufferSize + intensityBufferSize, pSumDFLIMSinglePhotonSum, dflimSinglePhotonSumBufferSize);
									memcpy(pMemoryBufferDFLIMPreview + dflimHistrogramBufferSize + intensityBufferSize + dflimSinglePhotonSumBufferSize, pSumDFLIMArrivalTimeSum, dflimArrivalTimeSumBufferSize);
									SavePreviewImage(&sp, t, pMemoryBufferDFLIMPreview, saveEnabledChannelsOnly);
								}
								else
								{
									SavePreviewImage(&sp, t, (char*)pSumMemoryBuffer, saveEnabledChannelsOnly);
								}

								previewAverageCount = 0;
								_lastImageUpdateTime = GetTickCount();
							}
							else
							{
								previewAverageCount++;
							}
						}
						else if (rollingAvgPreviewFrames > 1)
						{
							if (1 == imageMethod) //if dflim acquisition
							{
								memcpy(pMemoryBufferDFLIMPreview, pSumBufferDFLIMHisto, dflimHistrogramBufferSize);
								memcpy(pMemoryBufferDFLIMPreview + dflimHistrogramBufferSize, pSumMemoryBuffer, intensityBufferSize);
								memcpy(pMemoryBufferDFLIMPreview + dflimHistrogramBufferSize + intensityBufferSize, pSumBufferDFLIMSinglePhoton, dflimSinglePhotonSumBufferSize);
								memcpy(pMemoryBufferDFLIMPreview + dflimHistrogramBufferSize + intensityBufferSize + dflimSinglePhotonSumBufferSize, pSumBufferDFLIMArrivalTimeSum, dflimArrivalTimeSumBufferSize);
								SavePreviewImage(&sp, t, pMemoryBufferDFLIMPreview, saveEnabledChannelsOnly);
							}
							else
							{
								SavePreviewImage(&sp, t, (char*)pSumMemoryBuffer, saveEnabledChannelsOnly);								
							}

							_lastImageUpdateTime = GetTickCount();

							//only update the UI here when the real cumulative average frames > 1
							if (avgFrames > 1)
							{
								CallSaveTImage(capturedImagesCtrStimulusGUI);
							}

						}
						else
						{
							if (1 == imageMethod) //if dflim acquisition
							{
								memcpy(pMemoryBufferDFLIMPreview, pMemoryBufferDFLIMHisto, dflimHistrogramBufferSize);
								memcpy(pMemoryBufferDFLIMPreview + dflimHistrogramBufferSize, _pMemoryBuffer, intensityBufferSize);
								memcpy(pMemoryBufferDFLIMPreview + dflimHistrogramBufferSize + intensityBufferSize, pMemoryBufferDFLIMSinglePhoton, dflimSinglePhotonSumBufferSize);
								memcpy(pMemoryBufferDFLIMPreview + dflimHistrogramBufferSize + intensityBufferSize + dflimSinglePhotonSumBufferSize, pMemoryBufferDFLIMArrivalTimeSum, dflimArrivalTimeSumBufferSize);
								SavePreviewImage(&sp, t, pMemoryBufferDFLIMPreview, saveEnabledChannelsOnly);
							}
							else
							{
								SavePreviewImage(&sp, t, _pMemoryBuffer, saveEnabledChannelsOnly);
							}
							_lastImageUpdateTime = GetTickCount();
						}
					}
					else
					{
						if(tFastZStimulus == 1) // top	//zIndex
						{
							char* fullChannelImageBuffer = _pMemoryBuffer;
							shared_ptr<InternallyStoredImage<unsigned short> > fullChannelImage;
							if(saveEnabledChannelsOnly)
							{
								fullChannelImage = createFullChannelCopy(_pMemoryBuffer, sp);
								fullChannelImageBuffer = (char*)fullChannelImage->getDirectPointerToData(0,0,0,0,0);
							}
							memcpy(pTilesImagePreviewBuffer + zTop*tileBufferSizeBytes, fullChannelImageBuffer, tileBufferSizeBytes);
							//TODO: Add fastZ functionality for DFLIM
							SaveFastZPreviewImageWithoutOME(&sp, pTilesImagePreviewBuffer, tilesInPreview * tileBufferSizeBytes);
							updateRestTiles = TRUE;
						}
						//long zIndex = currentStreamCount % (zstageSteps+flybackFrames);	//zIndex is duplicate of tFastZStimulus

						if(tFastZStimulus == static_cast<long>(zstageSteps / 2.0 + 0.5) && updateRestTiles == TRUE) // middle	//zIndex
						{
							char* fullChannelImageBuffer = _pMemoryBuffer;
							shared_ptr<InternallyStoredImage<unsigned short> > fullChannelImage;
							if(saveEnabledChannelsOnly)
							{
								fullChannelImage = createFullChannelCopy(_pMemoryBuffer, sp);
								fullChannelImageBuffer = (char*)fullChannelImage->getDirectPointerToData(0,0,0,0,0);
							}
							memcpy(pTilesImagePreviewBuffer + zMid*tileBufferSizeBytes, fullChannelImageBuffer, tileBufferSizeBytes);
							//TODO: Add fastZ functionality for DFLIM
							SaveFastZPreviewImageWithoutOME(&sp, pTilesImagePreviewBuffer, tilesInPreview * tileBufferSizeBytes);
						}

						if(tFastZStimulus == previewIndex && updateRestTiles == TRUE) // user specify
						{
							char* fullChannelImageBuffer = _pMemoryBuffer;
							shared_ptr<InternallyStoredImage<unsigned short> > fullChannelImage;
							if(saveEnabledChannelsOnly)
							{
								fullChannelImage = createFullChannelCopy(_pMemoryBuffer, sp);
								fullChannelImageBuffer = (char*)fullChannelImage->getDirectPointerToData(0,0,0,0,0);
							}
							memcpy(pTilesImagePreviewBuffer + zUsr * tileBufferSizeBytes, fullChannelImageBuffer, tileBufferSizeBytes);
							//TODO: Add fastZ functionality for DFLIM
							SaveFastZPreviewImageWithoutOME(&sp, pTilesImagePreviewBuffer, tilesInPreview * tileBufferSizeBytes);
						}

						if(tFastZStimulus == zstageSteps && updateRestTiles == TRUE) //bottom	//zIndex
						{
							char* fullChannelImageBuffer = _pMemoryBuffer;
							shared_ptr<InternallyStoredImage<unsigned short> > fullChannelImage;
							if(saveEnabledChannelsOnly)
							{
								fullChannelImage = createFullChannelCopy(_pMemoryBuffer, sp);
								fullChannelImageBuffer = (char*)fullChannelImage->getDirectPointerToData(0,0,0,0,0);
							}
							memcpy(pTilesImagePreviewBuffer + zBot * tileBufferSizeBytes, fullChannelImageBuffer, tileBufferSizeBytes);
							//TODO: Add fastZ functionality for DFLIM
							SaveFastZPreviewImageWithoutOME(&sp, pTilesImagePreviewBuffer, tilesInPreview * tileBufferSizeBytes);

							_lastImageUpdateTime = GetTickCount();

							updateRestTiles = FALSE;
						}
					}
				}

				//We create Thread in suspension, finish one-by-one in order of creation:
				ResumeSaveThread(THREAD_PRIORITY_LOWEST);
			}
			//=====================================================
			// End Preview Image
			//======================================================

			//#ifdef USE_VIRTUAL_ALLOC == FALSE
			if(!useVirtualMemory)
				//synchrnously start the unlock process for the frame
			{
				UnlockImages(sp.imageIDsMap, totalFrame);
			}
			//#endif

			currentStreamCount++;

			pCamera->GetParam(ICamera::PARAM_DROPPED_FRAMES,droppedFrameCnt);

			if(ICamera::ORCA == ccdType)
			{
				pCamera->GetParam(ICamera::PARAM_DMA_BUFFER_AVAILABLE_FRAMES, dmaBufferAvailableFrames);
			}

			//Repeat commands done while camera is busy in case of missing:
			if(STORAGE_STIMULUS == storageMode)
			{
				GetStimulusActive();
				ControlShutterInStream(zFastEnableGUI);
			}
		}
		//==================================================
		//   END Average Loop
		//==================================================
		if (ICamera::STATUS_ERROR == status)
			break;

		//Update the GUI with current image index
		//stay below the maximum number of frames
		capturedImagesCtrStimulusGUI++;
		if(t <= streamCount && tStimulus <= streamCount)
		{
			CallSaveTImage(capturedImagesCtrStimulusGUI);
			// if the capture mode is hyperspectral, check if it tiling is enabled or not
			if(IExperiment::HYPERSPECTRAL == captureMode)
			{
				// if the tiling position is the first subwell, or tiling is not enabled, update
				// the first subwell normally. Otherwise send the TRUE flag so observer.cpp uses 
				// the total image_completed count instead of the index.
				if(2 > subWell)
				{
					CallSaveImage(capturedImagesCtrStimulusGUI, FALSE);
				}
				else
				{
					CallSaveImage(capturedImagesCtrStimulusGUI, TRUE);
				}
			}
			else
			{
				CallSaveImage(capturedImagesCtrStimulusGUI, FALSE);
			}
		}
		switch (storageMode)
		{
		case STORAGE_FINITE:
		{

			//======================================================
			// Save Image, no compression, no average, Big Tiff only
			//======================================================
			char* bufferAtPos = NULL;
			if ((FALSE == sp.doCompression) &&
				(((1 == avgFrames) && (FALSE == zFastEnableGUI)) ||
					((tFastZStimulus <= zstageSteps) && (TRUE == zFastEnableGUI))))	//ignore flyback frames since (tFastZStimulus % (zstageSteps+flybackFrames))
			{
				for (map<long, ScanRegion>::iterator imageIt = sp.regionMap.begin(); imageIt != sp.regionMap.end(); imageIt++) //regionMap available for both meso and micro scans
				{
					long tVolume = static_cast<long>(floor(t / (double)(imageIt->second.SizeZ + sp.fastFlybackFrames))) + (TRUE == zFastEnableGUI);	//1-based
					long zIndex = (FALSE == zFastEnableGUI) ? 1 : tFastZStimulus;																//1-based

					//save current t count for later use to update ome tiff xml
					currentTCount = tVolume;
					
					//Call Save
					if (useVirtualMemory)
						bufferAtPos = pVirtualMemory + imageIt->second.SizeX * imageIt->second.SizeY * sp.bufferChannels[imageIt->first] * (t - 1) * 2;
					else
						bufferAtPos = ImageManager::getInstance()->GetImagePtr((MesoScanTypes::Micro == viewMode) ? sp.regionImageIDsMap[imageIt->first] : sp.imageIDsMap[BufferType::INTENSITY], 0, 0, 0, t - 1);

					bigTiff->SetRegion(max(0, imageIt->first));
					for (auto it = activeChannels.begin(); it != activeChannels.end(); ++it)
					{
						long i = it->first;														//channel count index
						long channel = (sp.bufferChannels[imageIt->first] > 1 ? it->second : 0);	//channel ID

						//=== Save Tiff ===
						if ((sp.wavelengthName[i].size() > 0))
						{
							char* pBuf = (saveEnabledChannelsOnly) ? bufferAtPos + (imageIt->second.SizeX * imageIt->second.SizeY * sizeof(short) * i) : bufferAtPos + (imageIt->second.SizeX * imageIt->second.SizeY * sizeof(short) * channel);
							switch ((CaptureFile)sp.fileType)
							{
							case CaptureFile::FILE_BIG_TIFF:
								if (IExperiment::HYPERSPECTRAL == captureMode)
								{
									bigTiff->SaveData(pBuf, static_cast<uint16_t>(i), static_cast<uint16_t>(zIndex), 1, static_cast<uint16_t>(tVolume));
								}
								else
								{
									bigTiff->SaveData(pBuf, static_cast<uint16_t>(i), static_cast<uint16_t>(zIndex), static_cast<uint16_t>(tVolume), 1);
								}
								break;
							default:
								break;
							}
						}
					}
					if (!useVirtualMemory)
						ImageManager::getInstance()->UnlockImagePtr(((MesoScanTypes::Micro == viewMode) ? sp.regionImageIDsMap[imageIt->first] : sp.imageIDsMap[BufferType::INTENSITY]), 0, 0, 0, t - 1);
				}
			}

			//advance to the next frame
			t++;
		}
		break;
		case STORAGE_STIMULUS:
		{
			//if the software trigger or hardware stimulus lines are active
			//save the data
			if (TRUE == _hwStimulusActive)
			{
				//delayFrames == total fastZ scan - tFastZStimulus location
				long delayFrames = (zstageSteps + flybackFrames) - tFastZStimulus + 1;
				StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream POINT 2 tFastZStimulus %d stimulusActive %d zFastEnableGUI %d", tFastZStimulus, _swStimulusActive, zFastEnableGUI);
				logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);

				//this is the first index of the cycle
				//store this for the save thread
				if (FALSE == _swStimulusActive)
				{
					StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream POINT 3 tFastZStimulus %d stimulusActive %d zFastEnableGUI %d", tFastZStimulus, _swStimulusActive, zFastEnableGUI);
					logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);

					spThread.firstFrameIndex = tStimulus;
					avgStimulusFrames = static_cast<long>(floor((double)streamFrames / avgFrames));
					if (1 == zFastEnableGUI)
					{
						fastzVolumeNum = static_cast<long>(floor((double)(streamFrames) / (zstageSteps + flybackFrames)));
						if (fastzVolumeNum < 1)
						{
							FailedAcquisition(pCamera, sp, zFastEnableGUI);
							return FALSE;
						}
						fastzLastFrameID = delayFrames + fastzVolumeNum * (zstageSteps + flybackFrames);
					}

					StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream stimulus activated on index %d", tStimulus);
					logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);
					_swStimulusActive = TRUE;
					//open shutter at rising edge if not already opened:
					OpenShutter();
				}

				StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream POINT 4 tFastZStimulus %d stimulusActive %d zFastEnableGUI %d", tFastZStimulus, _swStimulusActive, zFastEnableGUI);
				logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);

				if ((1 == tFastZStimulus))	//Only active when zFastEnableGUI == 1
				{
					fastZActive = TRUE;
				}

				//reach max stimulus stream frames: (fastzLastFrameID = 0 if not fastZ; avgStimulusFrames could be 1.)
				if ((avgStimulusFrames == 1) || (avgStimulusFrames == t) || (fastzLastFrameID == t))
				{
					StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream stimulus reached the end of the buffer %d or fast z last frame %d", streamFrames, fastzLastFrameID);
					logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);

					if (1 == zFastEnableGUI)
					{
						fastzVolumeNum = static_cast<long>(floor((double)streamFrames / (zstageSteps + flybackFrames)));
						fastzLastFrameID = fastzVolumeNum * (zstageSteps + flybackFrames);
					}
					spThread.firstFrameIndex = tStimulus - t + 1;
					spThread.numFramesToSave = tStimulus - spThread.firstFrameIndex + 1;
					totalSavedImages += spThread.numFramesToSave;
					capturedImagesCtrStimulusGUI = totalSavedImages;
					AcquireTStream::savedFrameNumVec.push_back(tStimulus);
					spThread.deleteFileOnThreadCompletion = TRUE;//(0 == stimulusTransitionCount) ? FALSE : TRUE;

					if (TRUE == CreateSaveThread(spThread, pVirtualMemory))
						saveMaxStreamFramesCount++;

					for (std::map<long, long>::iterator it = spThread.imageIDsMap.begin(); it != spThread.imageIDsMap.end(); it++)
					{
						sp.imageIDsMap[it->first] = it->second;
					}

					t = 1;	//currentStreamCount
					tStimulus = tStimulus + 1;

					//Update the GUI with current image index
					//stay below the maximum number of frames
					if (tStimulus < streamCount)
					{
						CallSaveTImage(capturedImagesCtrStimulusGUI);
						CallSaveImage(capturedImagesCtrStimulusGUI, FALSE);
					}

					//need to rearm only for trigger first mode:
					if (ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == _triggerMode)
					{
						pCamera->PostflightAcquisition(NULL);
						pCamera->PreflightAcquisition(_pMemoryBuffer);
						startCamStatus = pCamera->StartAcquisition(_pMemoryBuffer);
						//break out of the case since it is done saving and rearming
						break;
					}
				}
				else
				{
					StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream (HWStimulusActive==TRUE) stimulus incrementing t frame %d tStimulus %d", t, tStimulus);
					logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);
					tStimulus++;
					t++;
				}
			}
			else
			{
				//***no lines are active***//

				//Postpone stop saving until complete of current z volume:
				if (TRUE == fastZActive)
				{
					if (1 == tFastZStimulus)		//finished z volume
					{
						fastZActive = FALSE;
						StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream Finished stimulus FastZ volume frame %d", tFastZStimulus);
						logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);
					}
					else						//not yet finish z volume
					{
						StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream stimulus FastZ frame %d after falling edge", tFastZStimulus);
						logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);

						//reach max stimulus stream frames: (fastzLastFrameID = 0 if not fastZ; avgStimulusFrames could be 1.)
						if ((avgStimulusFrames == 1) || (avgStimulusFrames == t) || (fastzLastFrameID == t))
						{
							//Reset the swStimulusActive flag
							_swStimulusActive = FALSE;
							StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream stimulus reached the end of the buffer %d or fast z last frame %d", streamFrames, fastzLastFrameID);
							logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);

							if (1 == zFastEnableGUI)
							{
								fastzVolumeNum = static_cast<long>(floor((double)streamFrames / (zstageSteps + flybackFrames)));
								fastzLastFrameID = fastzVolumeNum * (zstageSteps + flybackFrames);
							}
							spThread.firstFrameIndex = tStimulus - t + 1;
							spThread.numFramesToSave = tStimulus - spThread.firstFrameIndex + 1;
							totalSavedImages += spThread.numFramesToSave;
							capturedImagesCtrStimulusGUI = totalSavedImages;
							AcquireTStream::savedFrameNumVec.push_back(tStimulus);
							spThread.deleteFileOnThreadCompletion = TRUE;//(0 == stimulusTransitionCount) ? FALSE : TRUE;

							if (TRUE == CreateSaveThread(spThread, pVirtualMemory))
								saveMaxStreamFramesCount++;

							for (std::map<long, long>::iterator it = spThread.imageIDsMap.begin(); it != spThread.imageIDsMap.end(); it++)
							{
								sp.imageIDsMap[it->first] = it->second;
							}

							t = 1;	//currentStreamCount
							tStimulus = tStimulus + 1;

							//Update the GUI with current image index
							//stay below the maximum number of frames
							if (tStimulus < streamCount)
							{
								CallSaveTImage(capturedImagesCtrStimulusGUI);
								CallSaveImage(capturedImagesCtrStimulusGUI, FALSE);
							}

							//need to rearm only for trigger first mode:
							if (ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == _triggerMode)
							{
								pCamera->PostflightAcquisition(NULL);
								pCamera->PreflightAcquisition(_pMemoryBuffer);
								startCamStatus = pCamera->StartAcquisition(_pMemoryBuffer);
								//break out of the case since it is done saving and rearming
								break;
							}
						}
					}
				}
				
				//postpone stop saving until fulfilled average stimulus,
				//only in HW trigger mode since it does rearm every falling edge:
				if (ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == _triggerMode)
				{
					//stop scanner and zstage once
					if (FALSE == scannerStopped)
					{
						pCamera->PostflightAcquisition(NULL);
						IDevice* pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);
						if (NULL != pZStage)
						{
							pZStage->SetupPosition();
							pZStage->StartPosition();
						}
						scannerStopped = TRUE;
					}
					if (0 < static_cast<long>(droppedFrameCnt))
					{
						StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream Dropped (HWStimulusActive==FALSE) stimulus incrementing t frame %d tStimulus %d droppedFrames %d", t, tStimulus, static_cast<long>(droppedFrameCnt));
						logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);
						tStimulus++;
						t++;
					}
				}

				//create a save thread if this is a stimulus falling edge
				if ((TRUE == _swStimulusActive) && (0 >= static_cast<long>(droppedFrameCnt)))
				{
					StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream stimulus finished. Saving images");
					logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);
					_swStimulusActive = FALSE;
					//closing of the shutter at falling edge if not already closed,
					//for either fast z or stimulus
					CloseShutter();

					long framesToCut = 0;	//cut non-complete volume frames
					if ((TRUE == zFastEnableGUI) && ((t % (zstageSteps + flybackFrames)) != 0))
					{
						framesToCut = tFastZStimulus;
					}

					spThread.firstFrameIndex = tStimulus - t + 1;
					long numImagesBeforCut = tStimulus - spThread.firstFrameIndex + 1;

					//removed time data for extra points that will not go into the final experiment						
					for (long j = (totalSavedImages + numImagesBeforCut - 1); j >= (numImagesBeforCut + totalSavedImages - framesToCut); j--)
					{
						_acquireSaveInfo->getInstance()->RemoveTimestampAt(j);
						_acquireSaveInfo->getInstance()->RemoveTimingInfoAt(j);
					}

					spThread.numFramesToSave = numImagesBeforCut - framesToCut;
					totalSavedImages += spThread.numFramesToSave;
					capturedImagesCtrStimulusGUI = totalSavedImages;
					AcquireTStream::savedFrameNumVec.push_back(tStimulus - framesToCut);
					spThread.deleteFileOnThreadCompletion = TRUE;

					StringCbPrintfW(message, MSG_LENGTH, L"t:%d, spThread.numFramesToSave:%d", t, spThread.numFramesToSave);
					logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);

					if (TRUE == CreateSaveThread(spThread, pVirtualMemory))
						saveMaxStreamFramesCount++;

					for (std::map<long, long>::iterator it = spThread.imageIDsMap.begin(); it != spThread.imageIDsMap.end(); it++)
					{
						sp.imageIDsMap[it->first] = it->second;
					}

					//move the T back to the start
					t = 1;	//currentStreamCount
					saveMaxStreamFramesCount = 0;
					fastzLastFrameID = 0;
					tFastZStimulus = 0;
				}

				//stay below the maximum number of frames
				if (tStimulus < streamCount)
				{
					CallSaveTImage(capturedImagesCtrStimulusGUI);
					CallSaveImage(capturedImagesCtrStimulusGUI, FALSE);
				}

				//restart scanner when done with all dropped frames:
				if ((ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == _triggerMode) && (0 >= static_cast<long>(droppedFrameCnt)))
				{
					StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream (HWStimulusActive==FALSE) all frames accounted for retrieved all dropped frames. Postflight and Rearm");
					logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);
					pCamera->PreflightAcquisition(_pMemoryBuffer);
					startCamStatus = pCamera->StartAcquisition(_pMemoryBuffer);
					scannerStopped = FALSE;
					break;
				}				
			}

			//we've reached the end of the defined number
			//of stream counts for the stimulus. We will
			//restart another.
			if ((currentStreamCount == streamCount))
			{
				StringCbPrintfW(message, MSG_LENGTH, L"AcquireTStream stimulus currentStreamCount reached the streamCount %d", streamCount);
				logDll->TLTraceEvent(INFORMATION_EVENT, 1, message);

				currentStreamCount = 1;
				pCamera->PostflightAcquisition(NULL);
				pCamera->PreflightAcquisition(_pMemoryBuffer);
				startCamStatus = pCamera->StartAcquisition(_pMemoryBuffer);
			}
		}
		break;
		}
		
		//=================================================
		//   Handle Premature Stopping Of Capture
		//=================================================
		StopCaptureEventCheck(stopStatus);
		//user has asked to stop the capture
		if(1 == stopStatus)
		{
			//close shutter before waiting for  
			//user's response regarding file:
			CloseShutter();
			//stop ZStage:
			if(1 == zFastEnableGUI)
			{
				SetupZStage(2, NULL, NULL);
			}
			if(STORAGE_FINITE == storageMode)
			{
				if(0 == _messageID)
				{
					_messageID = MessageBox(NULL,L"Experiment stopped. Would you like to save the already acquired images?",L"Save Experiment Files",MB_YESNO | MB_SETFOREGROUND | MB_ICONWARNING | MB_SYSTEMMODAL);	
				}			
				//Allow save files based on user's decision:
				if(IDYES == _messageID)			
				{							
					break;
				}
				else if(IDNO == _messageID)
				{	
					pCamera->PostflightAcquisition(NULL);
					ScannerEnable(FALSE);
					if(!useVirtualMemory)
					{
						DestroyImages(sp.imageIDsMap);
						DestroyImages(sp.regionImageIDsMap);
						bigTiff->ClearImageStore();
						HandleStimStreamRawFile(&sp,DELETE_RAW, std::wstring(), std::wstring(), !saveEnabledChannelsOnly);
						HandleStimStreamRawFile(&sp,DELETE_TIF, std::wstring(), std::wstring(), !saveEnabledChannelsOnly);
					}
					pCamera->SetParam(ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY, FALSE);
					pCamera->SetParam(ICamera::PARAM_LSM_POWER_RAMP_ENABLE, FALSE);
					//revert back to not using pockels reference
					pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF,FALSE);

					// This is a work-around for a problem found with an 8050 CamLink camera. Consecutive experiments
					// would not trigger the camera correctly. This solution seems to solve that problem which might
					// happen for CamLink or old cameras. It doesn't seem to have any impact on the other cameras.
					if (IExperiment::HYPERSPECTRAL == captureMode)
					{
						pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, _triggerMode);
						pCamera->PreflightAcquisition(_pMemoryBuffer);
					}

					return TRUE;	
				}
			}
			else
			{
				//user stop while stimulus is still high,
				//create save thread to save current stimulus:
				if((TRUE == _hwStimulusActive) && (1 < t))
				{
					long framesToCut = 0;	//cut non-complete volume frames
					if ((TRUE==zFastEnableGUI) && ((t % (zstageSteps+flybackFrames)) != 0))
					{
						framesToCut = tFastZStimulus;
					}

					spThread.firstFrameIndex = tStimulus - t + 1;
					long numImagesBeforCut = tStimulus-spThread.firstFrameIndex;	//no +1 since last frame is empty

					//removed time data for extra points that will not go into the final experiment						
					for(long j=(totalSavedImages + numImagesBeforCut - 1); j>=(numImagesBeforCut + totalSavedImages - framesToCut); j--)
					{
						_acquireSaveInfo->getInstance()->RemoveTimestampAt(j);
						_acquireSaveInfo->getInstance()->RemoveTimingInfoAt(j);
					}

					spThread.numFramesToSave = numImagesBeforCut - framesToCut;
					totalSavedImages += spThread.numFramesToSave;
					capturedImagesCtrStimulusGUI = totalSavedImages;
					AcquireTStream::savedFrameNumVec.push_back(tStimulus);
					spThread.deleteFileOnThreadCompletion = TRUE;

					if(TRUE == CreateSaveThread(spThread, pVirtualMemory))
						saveMaxStreamFramesCount++;

					for (std::map<long, long>::iterator it = spThread.imageIDsMap.begin(); it != spThread.imageIDsMap.end(); it++)
					{
						sp.imageIDsMap[it->first] = it->second;
					}
				}
				//ready to stop
				break;
			}
		}
		//break out while loop if unable to start camera, 
		//could be HW timed out
		if(0 == startCamStatus)
			break;
	}
	//=================================================
	//    END Aqcuire Loop
	//=================================================

	//==========================================
	//   Reset Z if Z moved
	//==========================================
	if(TRUE == zFastEnableGUI)
	{
		SetupZStage(2, NULL, NULL);
	}

	pCamera->PostflightAcquisition(NULL);	

	//Close the shutter after finite streaming
	CloseShutter();

	//stop the scanner before processing the images
	ScannerEnable(FALSE);

	sp.photonsBufferTotalSize = dflimPhotonListOffset;
	SAFE_DELETE_MEMORY(sp.pMemoryBuffer);

	SAFE_DELETE_ARRAY (pTilesImagePreviewBuffer);

	SAFE_DELETE_ARRAY (pSumMemoryBuffer);

	SAFE_DELETE_ARRAY (pSumBufferDFLIMHisto);

	SAFE_DELETE_ARRAY (pSumBufferDFLIMArrivalTimeSum);

	SAFE_DELETE_ARRAY (pSumBufferDFLIMSinglePhoton);

	// Adjust T if OME-Tiff and stopped manually
	if (IDYES == _messageID && STORAGE_FINITE == storageMode)
	{
		bigTiff->AdjustScanTCount((int)currentTCount);
	}

	bigTiff->ClearImageStore();

	//========================================================
	//   Process Memory Stream After All Images Aqcuired
	//========================================================
	switch (storageMode)
	{
	case STORAGE_FINITE:
		//#ifdef USE_VIRTUAL_ALLOC
		if(useVirtualMemory)
		{sp.pMemoryBuffer = pVirtualMemory;}
		//#else
		else
		{sp.pMemoryBuffer = _pMemoryBuffer;}
		//#endif
		DestroyImages(sp.imageIDsMap);
		DestroyImages(sp.regionImageIDsMap);
		//no post process if Big tiff without compression or average
		if ((1 < t)	&& ((CaptureFile::FILE_BIG_TIFF != (CaptureFile)sp.fileType) || (TRUE == sp.doCompression) || (1 < avgFrames)))
		{
			//images are acquired, otherwise could be due to camera HW timed out or error
			sp.numFramesToSave = sp.regionMap.begin()->second.SizeT = t-1;		//No matter stopStatus, total frames will be equal to t-1.
			HandleStimStreamRawFile(&sp,RESIZE_SINGLE_STIMULUS, std::wstring(), std::wstring(), !saveEnabledChannelsOnly);
			SaveImagesPostCapture(index, subWell, t-1, &sp, !saveEnabledChannelsOnly, subWell);
		}
		if((1 >= t) || (CaptureFile::FILE_RAW != (CaptureFile)sp.fileType))
			HandleStimStreamRawFile(&sp,DELETE_RAW, std::wstring(), std::wstring(), !saveEnabledChannelsOnly);
		if (MesoScanTypes::Micro == viewMode)
		{
			//[ToDo] do compression of big tiff...
			if (TRUE == sp.doCompression)
			{}
			//clear all temp files
			HandleStimStreamRawFile(&sp,DELETE_ALL_TEMP);
		}
		break;
	case STORAGE_STIMULUS:
		//resume and wait for all saving threads:
		while(AcquireTStream::_saveThreadCount > 0)
		{
			ResumeSaveThread(THREAD_PRIORITY_ABOVE_NORMAL);

			if((FALSE == _saveThreadActive) && (_saveThreadCountFinished>0) && (AcquireTStream::savedFrameNumVec.size()>_saveThreadCountFinished))
			{
				CallSaveImage(AcquireTStream::savedFrameNumVec.at(_saveThreadCountFinished-1), FALSE);
			}	
		}
		//clear all save thread handles
		vector<HANDLE>::iterator it = _threadHandles.begin();
		for(long i=0; i<static_cast<long>(_threadHandles.size()); i++)
		{
			SAFE_DELETE_HANDLE(*it);
			if(i<static_cast<long>(_threadHandles.size()-1))
			{
				it++;
			}
		}
		_threadHandles.clear();
		AcquireTStream::savedFrameNumVec.clear();

		//clear the last dummy image:
		//#ifdef USE_VIRTUAL_ALLOC
		if(useVirtualMemory)
		{
			SIZE_T memSize = sp.regionMap.begin()->second.SizeX;
			memSize *= sp.regionMap.begin()->second.SizeY;
			memSize *= sp.colorChannels;
			memSize *= setupFrames;
			memSize *= 2; 
			VirtualFree(sp.pMemoryBuffer, 0, MEM_RELEASE);
		}
		//#else
		else
		{
			DestroyImages(sp.imageIDsMap);
			DestroyImages(sp.regionImageIDsMap);
			sp.regionMap.begin()->second.SizeT = totalSavedImages;					
			imageMode = FALSE;	//NOT using ImageManager
			_messageID = 1;		//NOT allow stop status check				
			HandleStimStreamRawFile(&sp,COMBINE_ALL_TIS_TEMP, std::wstring(), std::wstring(), !saveEnabledChannelsOnly);
			SaveImagesPostCapture(index, subWell, sp.regionMap.begin()->second.SizeT, &sp, !saveEnabledChannelsOnly, subWell);
			sp.imageIDsMap.clear();
			sp.regionImageIDsMap.clear();
			if(CaptureFile::FILE_TIFF == (CaptureFile)sp.fileType)
			{
				CallSaveTImage(totalSavedImages);
				CallSaveImage(totalSavedImages, FALSE);
			}
			HandleStimStreamRawFile(&sp,DELETE_ALL_TEMP, std::wstring(), std::wstring(), !saveEnabledChannelsOnly);

			if(CaptureFile::FILE_RAW != (CaptureFile)sp.fileType)
				HandleStimStreamRawFile(&sp,DELETE_RAW, std::wstring(), std::wstring(), !saveEnabledChannelsOnly);
		}
		//#endif	
		break;
	}

	pCamera->SetParam(ICamera::PARAM_LSM_AVERAGEMODE,averageMode);

	pCamera->SetParam(ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY, FALSE);
	pCamera->SetParam(ICamera::PARAM_LSM_POWER_RAMP_ENABLE, FALSE);
	//revert back to not using pockels reference
	pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF,FALSE);

	// This is a work-around for a problem found with an 8050 CamLink camera. Consecutive experiments
	// would not trigger the camera correctly. This solution seems to solve that problem which might
	// happen for CamLink or old cameras. It doesn't seem to have any impact on the other cameras.
	if (IExperiment::HYPERSPECTRAL == captureMode)
	{
		pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, _triggerMode);
		pCamera->PreflightAcquisition(_pMemoryBuffer);
	}

	return TRUE;
}

/// <summary>
/// This function calculates the rolling averarage and puts it on the pHistory buffer that is passed
/// </summary>
/// <param name="pHistoryBuffer"></param>
/// <param name="pBuffer"></param>
/// <param name="averageNum"></param>
/// <param name="frameSize"></param>
/// <returns></returns>
long AcquireTStream::CalculateRollingAverage(USHORT* averageBuffer, USHORT* newBuffer, long averageNum, long frameLength)
{
	if (averageNum <= 1)
	{
		memcpy(averageBuffer, newBuffer, frameLength * sizeof(USHORT));
	}
	else
	{
		for (int pixel = 0; pixel < frameLength; ++pixel)
		{
			averageBuffer[pixel] = (USHORT)round(((double)averageBuffer[pixel] * ((double)averageNum - 1) + (double)newBuffer[pixel]) / (double)averageNum);
		}
	}
	return TRUE;
}

/// <summary>
/// This function calculates the rolling sum and puts it on the sum buffers that are passed
/// </summary>
/// <param name="pHistoryBuffer"></param>
/// <param name="pBuffer"></param>
/// <param name="averageNum"></param>
/// <param name="frameSize"></param>
/// <returns></returns>
long AcquireTStream::CalculateRollingAverageAndSumDFLIM(USHORT* averageIntensityBuffer, USHORT* sumSinglePhotonBuffer, ULONG32* sumArrivalTimeSumBuffer, ULONG32* sumHistogramBuffer, USHORT* newIntensityBuffer, USHORT* newSinglePhotonBuffer, ULONG32* newArrivalTimeSumBuffer, ULONG32* newHistogramBuffer, long averageNum, long frameLength)
{
	const long DFLIM_HISTO_BINS = 256;
	if (averageNum <= 1)
	{		
		memcpy(averageIntensityBuffer, newIntensityBuffer, frameLength * sizeof(USHORT));
		memcpy(sumSinglePhotonBuffer, newSinglePhotonBuffer, frameLength * sizeof(USHORT));
		memcpy(sumArrivalTimeSumBuffer, newArrivalTimeSumBuffer, frameLength * sizeof(ULONG32));
		memcpy(sumHistogramBuffer, newHistogramBuffer, frameLength * sizeof(DFLIM_HISTO_BINS));
	}
	else
	{
		for (int pixel = 0; pixel < frameLength; ++pixel)
		{
			averageIntensityBuffer[pixel] = (USHORT)round(((double)averageIntensityBuffer[pixel] * ((double)averageNum - 1) + (double)newIntensityBuffer[pixel]) / (double)averageNum);
			sumSinglePhotonBuffer[pixel] += static_cast<USHORT>(round(newSinglePhotonBuffer[pixel] - sumSinglePhotonBuffer[pixel] / (double)averageNum));
			sumArrivalTimeSumBuffer[pixel] += static_cast<ULONG32>(round(newArrivalTimeSumBuffer[pixel] - sumArrivalTimeSumBuffer[pixel] / (double)averageNum));
		}

		for (int bin = 0; bin < DFLIM_HISTO_BINS; ++bin)
		{
			//data between 241 and 253 should not be summed up
			if (bin >= 241 && bin <= 253)
			{
				continue;
			}

			{
				sumHistogramBuffer[bin] += static_cast<ULONG32>(round(newHistogramBuffer[bin] - sumHistogramBuffer[bin] / (double)averageNum));
			}
		}
	}
	return TRUE;
}

///Calculate memory offset based on system granularity:
DWORDLONG CalculateMemOffset(LARGE_INTEGER mapOffset,DWORD &lowOffset,DWORD &highOffset)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	long granularityOfMapView = si.dwAllocationGranularity;

	LARGE_INTEGER tmpVal;
	tmpVal.QuadPart = mapOffset.QuadPart/granularityOfMapView;
	LARGE_INTEGER allowedOffset;
	allowedOffset.QuadPart = tmpVal.QuadPart*granularityOfMapView;

	//Determine the difference between allowed offset and input offset:
	DWORDLONG memOffset = static_cast<DWORDLONG>(mapOffset.QuadPart - allowedOffset.QuadPart);	

	//return allowed offset for file map view:
	lowOffset = static_cast<DWORD>(allowedOffset.LowPart);
	highOffset = static_cast<DWORD>(allowedOffset.HighPart);

	return memOffset;
}
void CalculateFileSize(DWORD highOrderIn,DWORD lowOrderIn,LARGE_INTEGER sizeLong,DWORD &lowOrderSize,DWORD &highOrderSize)
{	
	LARGE_INTEGER localSize;
	localSize.QuadPart = sizeLong.QuadPart & 0xFFFFFFFF;
	DWORDLONG dwlhighOrderSize = (sizeLong.QuadPart & 0xFFFFFFFF00000000)>>32;
	localSize.QuadPart += lowOrderIn;
	lowOrderSize = static_cast<DWORD>(localSize.LowPart);
	highOrderSize = static_cast<DWORD>(dwlhighOrderSize);
	highOrderSize += static_cast<DWORD>(localSize.HighPart);
	highOrderSize += highOrderIn;
}
long AcquireTStream::HandleStimStreamRawFile(SaveParams *sp,long jobID, std::wstring lhsFilename, std::wstring rhsFilename, bool rawContainsDisabledChannels)
{
	long ret = TRUE;
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	wchar_t rawName[_MAX_PATH], rawdFLIMHistoName[_MAX_PATH], rawdFLIMSinglePhotonName[_MAX_PATH], rawdFLIMArrivalTimeSumName[_MAX_PATH], rawdFLIMPhotonsName[_MAX_PATH], newName[_MAX_PATH], newdFLIMImageName[_MAX_PATH], newdFLIMPhotonsName[_MAX_PATH];

	_wsplitpath_s(sp->path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

	WIN32_FIND_DATA FindFileData;
	HANDLE hFile = NULL, hAppend = NULL, hFileMap = NULL, hAppendMap = NULL;
	long byteSize = sizeof(unsigned short)/sizeof(char);
	long bufSize = sp->regionMap.begin()->second.SizeX*sp->regionMap.begin()->second.SizeY;
	long imageSize = bufSize*sp->bufferChannels.begin()->second;
	long colorimageSize = bufSize*sp->colorChannels;
	DWORDLONG dwlsize = imageSize*byteSize;
	DWORDLONG dwlcolorsize = colorimageSize*byteSize;
	DWORD dsize = static_cast<DWORD>(dwlsize);
	DWORD dcolorsize = static_cast<DWORD>(dwlcolorsize);
	wstring compFile;
	long channel = 0; 
	DWORD lowOffset = 0, highOffset = 0, lowOrderSize = 0, highOrderSize = 0;
	DWORDLONG memOffset = 0;	
	LARGE_INTEGER sizeLong;
	SIZE_T filemapSize = 0;

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	if (MesoScanTypes::Micro == viewMode)
		imgNameFormat << L"%s%s\\Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";
	else
		imgNameFormat << L"%s%s\\Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";

	std::wstringstream dflimImageNameFormat;	
	dflimImageNameFormat << L"%s%s\\Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.dFLIM";

	std::wstringstream dflimHistoNameFormatOriginal;
	dflimHistoNameFormatOriginal << L"%s%s\\Image_dFLIMHisto_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";

	std::wstringstream dflimSinglePhotonNameFormatOriginal;
	dflimSinglePhotonNameFormatOriginal << L"%s%s\\Image_dFLIMSinglePhoton_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";

	std::wstringstream dflimArrivalTimeSumNameFormatOriginal;
	dflimArrivalTimeSumNameFormatOriginal << L"%s%s\\Image_dFLIMArrivalTimeSum_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";

	std::wstringstream dflimPhotonsNameFormat;
	dflimPhotonsNameFormat << L"%s%s\\Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.photons";		
	std::wstringstream dflimPhotonsNameFormatOriginal;
	dflimPhotonsNameFormatOriginal << L"%s%s\\Image_photons_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";

	std::wstringstream strmNameFormat;
	strmNameFormat << L"%s%s\\Stream_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tmp";
	std::wstringstream tisNameFormat;
	tisNameFormat << L"%s%s\\TIS_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tmp";

	switch (jobID)
	{
		//******	Resize & Average single stimulus tmp file	******//
	case RESIZE_SINGLE_STIMULUS:
		try
		{
			//=== Get File Name ===
			if(STORAGE_FINITE == sp->storageMode)
			{
				if (MesoScanTypes::Micro == viewMode)
					StringCbPrintfW(rawName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir);
				else
					StringCbPrintfW(rawName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir,sp->index,sp->subWell);
				if (sp->imageMethod == 1)  //if dflim capture
				{
					StringCbPrintfW(rawdFLIMHistoName,_MAX_PATH,dflimHistoNameFormatOriginal.str().c_str(),drive,dir,sp->index,sp->subWell);
					StringCbPrintfW(rawdFLIMSinglePhotonName,_MAX_PATH,dflimSinglePhotonNameFormatOriginal.str().c_str(),drive,dir,sp->index,sp->subWell);
					StringCbPrintfW(rawdFLIMArrivalTimeSumName,_MAX_PATH,dflimArrivalTimeSumNameFormatOriginal.str().c_str(),drive,dir,sp->index,sp->subWell);
					StringCbPrintfW(rawdFLIMPhotonsName,_MAX_PATH,dflimPhotonsNameFormatOriginal.str().c_str(),drive,dir,sp->index,sp->subWell);
				}				
			}
			else
			{
				StringCbPrintfW(rawName,_MAX_PATH,strmNameFormat.str().c_str(),drive,dir,sp->imageIDsMap[BufferType::INTENSITY]);
			}

			//=== Open Raw File ===
			int actualDepth = (sp->regionMap.begin()->second.SizeZ > 0) ? sp->regionMap.begin()->second.SizeZ + sp->fastFlybackFrames : 1;
			RawFile<unsigned short> rawFile(wstring(rawName), sp->regionMap.begin()->second.SizeX, sp->regionMap.begin()->second.SizeY, actualDepth ,sp->bufferChannels.begin()->second,1, rawContainsDisabledChannels, getEnabledChannelIndices(*sp), GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);

			//=== Resize the File ===
			rawFile.shortenSeriesToTotalZSlices(sp->numFramesToSave * sp->avgFrames);

			//=== Average Images ===
			if(sp->avgFrames > 1)
			{
				rawFile.averageImages(sp->avgFrames);
			}

			bool dflimFilesValid = false;
			if (sp->imageMethod == 1)  //if dflim capture
			{
				long histoWidth = sp->dimensionsMap[BufferType::DFLIM_HISTOGRAM].x;
				long histoHeight = sp->dimensionsMap[BufferType::DFLIM_HISTOGRAM].y;
				RawFile<UINT32> rawFileDFLIMHisto(wstring(rawdFLIMHistoName), histoWidth, histoHeight, actualDepth ,sp->bufferChannels.begin()->second,1, rawContainsDisabledChannels, getEnabledChannelIndices(*sp), GenericImage<UINT32>::CONTIGUOUS_CHANNEL_DFLIM_HISTO);

				RawFile<unsigned short> rawFileDFLIMSinglePhoton(wstring(rawdFLIMSinglePhotonName), sp->regionMap.begin()->second.SizeX, sp->regionMap.begin()->second.SizeY, actualDepth ,sp->bufferChannels.begin()->second,1, rawContainsDisabledChannels, getEnabledChannelIndices(*sp), GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);

				RawFile<UINT32> rawFileDFLIMArrivalTimeSum(wstring(rawdFLIMArrivalTimeSumName), sp->regionMap.begin()->second.SizeX, sp->regionMap.begin()->second.SizeY, actualDepth ,sp->bufferChannels.begin()->second,1, rawContainsDisabledChannels, getEnabledChannelIndices(*sp), GenericImage<UINT32>::CONTIGUOUS_CHANNEL);

				long photonListWidth = sp->dimensionsMap[BufferType::DFLIM_PHOTONS].x;
				long photonListHeight = sp->dimensionsMap[BufferType::DFLIM_PHOTONS].y;
				RawFile<UCHAR> rawFileDFLIMPhotons(wstring(rawdFLIMPhotonsName), photonListWidth, photonListHeight, actualDepth ,sp->bufferChannels.begin()->second,1, rawContainsDisabledChannels, getEnabledChannelIndices(*sp), GenericImage<UCHAR>::CONTIGUOUS_CHANNEL);

				//=== Resize the Files ===
				rawFileDFLIMHisto.shortenSeriesToTotalZSlices(sp->numFramesToSave * sp->avgFrames);
				rawFileDFLIMSinglePhoton.shortenSeriesToTotalZSlices(sp->numFramesToSave * sp->avgFrames);
				rawFileDFLIMArrivalTimeSum.shortenSeriesToTotalZSlices(sp->numFramesToSave * sp->avgFrames);
				rawFileDFLIMPhotons.shortenSeriesToSize(sp->photonsBufferTotalSize);

				//=== Average Images ===
				if(sp->avgFrames > 1)
				{
					rawFileDFLIMHisto.SumImages(sp->avgFrames);
					rawFileDFLIMSinglePhoton.SumImages(sp->avgFrames);
					rawFileDFLIMArrivalTimeSum.SumImages(sp->avgFrames);
					//the dflim photons file doesn't need averaging or summation
				}

				if (rawFileDFLIMHisto.isValid() && 
					rawFileDFLIMSinglePhoton.isValid() && 
					rawFileDFLIMArrivalTimeSum.isValid() && 
					rawFileDFLIMPhotons.isValid())
				{
					dflimFilesValid = true;
					rawFileDFLIMHisto.releaseFile();
					rawFileDFLIMSinglePhoton.releaseFile();
					rawFileDFLIMArrivalTimeSum.releaseFile();
					rawFileDFLIMPhotons.releaseFile();
				}
			}

			switch ((CaptureFile)sp->fileType)
			{
			case CaptureFile::FILE_TIFF:
			case CaptureFile::FILE_RAW:
				//=== Rename File ===
				if (rawFile.isValid())
				{
					//Release the raw file before renaming it
					rawFile.releaseFile();

					//TODO: for now dflim will work only with streaming finite but this should change in the future to
					//allo for streaming stimulus
					if (sp->imageMethod == 1 && (1 == sp->firstFrameIndex || STORAGE_FINITE == sp->storageMode))
					{
						if (MesoScanTypes::Micro == viewMode)
							StringCbPrintfW(rawName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir);
						else
							StringCbPrintfW(newName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir,sp->index,sp->subWell);
						StringCbPrintfW(newdFLIMImageName,_MAX_PATH,dflimImageNameFormat.str().c_str(),drive,dir,sp->index,sp->subWell);	
						StringCbPrintfW(newdFLIMPhotonsName,_MAX_PATH,dflimPhotonsNameFormat.str().c_str(),drive,dir,sp->index,sp->subWell);				

						//hFile = FindFirstFile(rawName, &FindFileData);
						//_wrename(rawName,newName);					
						//FindClose(hFile);
						//hFile = NULL;

						hFile = FindFirstFile(rawdFLIMHistoName, &FindFileData);
						_wrename(rawdFLIMHistoName,newdFLIMImageName);					
						FindClose(hFile);
						hFile = NULL;	

						HandleStimStreamRawFile(sp, COMBINE_TWO_STIMULUS, newdFLIMImageName, rawName, rawContainsDisabledChannels);
						HandleStimStreamRawFile(sp, COMBINE_TWO_STIMULUS, newdFLIMImageName, rawdFLIMSinglePhotonName, rawContainsDisabledChannels);
						HandleStimStreamRawFile(sp, COMBINE_TWO_STIMULUS, newdFLIMImageName, rawdFLIMArrivalTimeSumName, rawContainsDisabledChannels);

						hFile = FindFirstFile(rawdFLIMPhotonsName, &FindFileData);
						_wrename(rawdFLIMPhotonsName, newdFLIMPhotonsName);					
						FindClose(hFile);
						hFile = NULL;
					}
					else
					{
						//Rename file:
						if (1 == sp->firstFrameIndex || STORAGE_FINITE == sp->storageMode)
						{
							if (MesoScanTypes::Micro == viewMode)
								StringCbPrintfW(rawName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir);
							else
								StringCbPrintfW(newName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir,sp->index,sp->subWell);
						}
						else
						{
							StringCbPrintfW(newName,_MAX_PATH,tisNameFormat.str().c_str(),drive,dir,sp->imageIDsMap[BufferType::INTENSITY]);
						}					

						hFile = FindFirstFile(rawName, &FindFileData);
						_wrename(rawName,newName);					
						FindClose(hFile);
						hFile = NULL;
					}
				}
				else
				{
					SAFE_DELETE_HANDLE(hFile);
					ret = FALSE;
				}
				break;
			case CaptureFile::FILE_BIG_TIFF:
				//=== save buffer to big tiff ===
				if(STORAGE_STIMULUS == sp->storageMode)
				{
					if (rawFile.isValid())
					{
						FrameInfo frameInfo = {-1};
						bigTiff->AddScan(sp->_zInfo.zStartPosUM,sp->_zInfo.zStopPosUM,sp->_zInfo.zStepSizeUM, rawFile.getNumImages());
						bigTiff->SetScan(AcquireTStream::_saveThreadCountFinished + 1);

						vector<pair<long,long> > activeChannels = getActiveChannels(*sp);
						for(uint16_t t=0; t<rawFile.getNumImages(); t++)
						{
							GenericImage<unsigned short>& img = rawFile.getImageAtIndex(t);
							for(uint16_t z=0; z<sp->regionMap.begin()->second.SizeZ; z++)
							{
								for(map<long,long>::iterator imageIt = sp->imageIDsMap.begin(); imageIt != sp->imageIDsMap.end(); imageIt++)
								{
									bigTiff->SetRegion(imageIt->first);
									for(auto it=activeChannels.begin(); it!=activeChannels.end(); ++it)
									{
										uint16_t i= static_cast<uint16_t>(it->first);
										uint16_t channel=(sp->bufferChannels[imageIt->first] > 1 ? static_cast<uint16_t>(it->second) : 0);
										char* bufferAtPos = (char*)img.getDirectPointerToData(0,0,z,channel,0);
										//use headerInfo to determine if hyperspectral capture
										if(nullptr == sp->_pHeaderInfo)
										{
											bigTiff->SaveData(bufferAtPos, i, z+1, t+1, 1);
										}
										else
										{
											bigTiff->SaveData(bufferAtPos, i, z+1, 1, t+1);
										}
									}
								}
							}
						}

						rawFile.releaseFile();
					}
					else
					{
						SAFE_DELETE_HANDLE(hFile);
						ret = FALSE;
					}
					//=== delete raw ===
					CFuncErrChk(L"DeleteFile", DeleteFile(rawName), false);
					hFile = NULL;
				}
				break;
			}
		}
		catch(...)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"Unable to resize stimulus %s",rawName);
			ret = FALSE;
		}
		break;
		//******	Append right-hand-side to left-hand-side stimulus tmp file	******//
	case COMBINE_TWO_STIMULUS:
		try
		{	
			//create handle of previous file:
			StringCbPrintfW(rawName,_MAX_PATH,L"%s",lhsFilename.c_str());
			hAppend = CreateFile(rawName,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING,NULL);
			if(hAppend == INVALID_HANDLE_VALUE)
			{
				SAFE_DELETE_HANDLE(hAppend);
				return FALSE;
			}

			//create handle of current file:
			StringCbPrintfW(newName,_MAX_PATH,L"%s",rhsFilename.c_str());
			hFile = CreateFile(newName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING,NULL);
			if(hFile == INVALID_HANDLE_VALUE)
			{	
				SAFE_DELETE_HANDLE(hAppend);
				SAFE_DELETE_HANDLE(hFile);
				return FALSE;
			}

			//determine curent file size:
			if(!GetFileSizeEx(hFile, &sizeLong))
			{	return FALSE;	}

			//Create mapping of current file:
			CalculateFileSize(0,0,sizeLong,lowOrderSize,highOrderSize);
			hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, highOrderSize, lowOrderSize,L"FileMapping");

			//Get previous file's size:
			LARGE_INTEGER oldFileSize;
			if(!GetFileSizeEx(hAppend, &oldFileSize))
			{	return FALSE;	}

			//Extend previous file:
			if(SetFilePointerEx(hAppend, sizeLong, NULL, FILE_END) != INVALID_SET_FILE_POINTER)
			{	SetEndOfFile(hAppend);	}

			//Get previous file's new size:
			LARGE_INTEGER newFileSize;
			if(!GetFileSizeEx(hAppend, &newFileSize))
			{	return FALSE;	}
			highOrderSize = static_cast<DWORD>(newFileSize.HighPart);
			lowOrderSize = static_cast<DWORD>(newFileSize.LowPart);

			//Create mapping of previous file:
			hAppendMap = CreateFileMapping(hAppend, NULL, PAGE_READWRITE, highOrderSize, lowOrderSize,L"AppendMapping");

			if((hFile != INVALID_HANDLE_VALUE) && (hAppend != INVALID_HANDLE_VALUE) && (hFileMap != INVALID_HANDLE_VALUE) && (hAppendMap != INVALID_HANDLE_VALUE))
			{	
				memOffset = CalculateMemOffset(oldFileSize,lowOffset,highOffset);
				char* hAppendMapBuffer = (char*)MapViewOfFile(hAppendMap, FILE_MAP_ALL_ACCESS, highOffset, lowOffset, 0);

				//have to offset pointer to real end of the file:
				hAppendMapBuffer += memOffset;

				char* hFileMapBuffer = (char*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

				//Do copy:	
				CopyMemory((PVOID)hAppendMapBuffer,(const void*)hFileMapBuffer,sizeLong.QuadPart);

				UnmapViewOfFile(hFileMapBuffer);	
				UnmapViewOfFile(hAppendMapBuffer);

				if(hFileMapBuffer)
				{	hFileMapBuffer = NULL;	}
				if(hAppendMapBuffer)
				{	hAppendMapBuffer = NULL;	}
				SAFE_DELETE_HANDLE(hFileMap);
				SAFE_DELETE_HANDLE(hAppendMap);
				SAFE_DELETE_HANDLE(hFile);
				SAFE_DELETE_HANDLE(hAppend);

				//delete second file:
				CFuncErrChk(L"DeleteFile", DeleteFile(newName), false);
				hFile = FindFirstFile(rawName, &FindFileData);
				FindClose(hFile);
				hFile = NULL;
			}
			else
			{
				SAFE_DELETE_HANDLE(hFileMap);
				SAFE_DELETE_HANDLE(hAppendMap);
				SAFE_DELETE_HANDLE(hFile);
				SAFE_DELETE_HANDLE(hAppend);
				ret = FALSE;
			}
		}
		catch(...)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"Error cancatenating file: %s",newName);
			ret = FALSE;
		}
		break;	
		//******	Append to the end of previous stimulus tmp file	******//
	case CONCATENATE_PREVIOUS_STIMULUS:		
		try
		{
			StringCbPrintfW(rawName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir,sp->index,sp->subWell);	
			lhsFilename.assign(rawName);
			StringCbPrintfW(newName,_MAX_PATH,tisNameFormat.str().c_str(),drive,dir,sp->imageIDsMap[BufferType::INTENSITY]);		
			rhsFilename.assign(newName);
			HandleStimStreamRawFile(sp,COMBINE_TWO_STIMULUS, lhsFilename, rhsFilename, rawContainsDisabledChannels);

		}
		catch(...)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"Error cancatenating file: %s",newName);
			ret = FALSE;
		}
		break;	
		//******	Rename single .tmp file	******//
	case RENAME_SINGLE_TIS:			
		try
		{	
			bool doRename = false;
			StringCbPrintfW(rawName,_MAX_PATH,tisNameFormat.str().c_str(),drive,dir,sp->imageIDsMap[BufferType::INTENSITY]);
			hFile = FindFirstFile(rawName, &FindFileData);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				doRename = true;
			}
			else
			{	
				StringCbPrintfW(rawName,_MAX_PATH,L"%s%s\\TIS_*.tmp",drive,dir);
				hFile = FindFirstFile(rawName, &FindFileData);
				if(hFile != INVALID_HANDLE_VALUE)
				{
					doRename = true;
				}
				else
				{
					ret = FALSE;
				}
			}
			if(doRename)
			{
				StringCbPrintfW(rawName,_MAX_PATH,L"%s%s\\%s",drive,dir,FindFileData.cFileName);
				StringCbPrintfW(newName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir,sp->index,sp->subWell);
				_wrename(rawName,newName);	
			}
			FindClose(hFile);
			hFile = NULL;
		}
		catch(...)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"Unable to rename file: %s",rawName);
			ret = FALSE;
		}
		break;
		//******	Combine all TIS .tmp files in order	******//
	case COMBINE_ALL_TIS_TEMP:		
		try
		{
			std::vector<FileNameStruc> nameStrucVec;
			FileNameStruc tempNameStruc;
			StringCbPrintfW(rawName,_MAX_PATH,L"%s%s\\TIS_*.tmp",drive,dir);
			HANDLE hSearchFile = FindFirstFile(rawName, &FindFileData);
			while (hSearchFile != INVALID_HANDLE_VALUE)
			{					
				StringCbPrintfW(rawName,_MAX_PATH,L"%s%s\\%s",drive,dir,FindFileData.cFileName);
				tempNameStruc.fileName = rawName;
				nameStrucVec.push_back(tempNameStruc);
				compFile =FindFileData.cFileName;
				FindNextFile(hSearchFile,&FindFileData);
				if(0 == compFile.compare(FindFileData.cFileName))
				{
					break;
				}
			}

			if(nameStrucVec.size() > 1)
			{
				//More than one TIS .tmp are found, do combine:
				std::wstring targetStr = L"TIS_";
				std::wstring fileType = L".tmp";

				//Reorder FileNameStruc vector based on parsed imageIDs,
				//could be overflowed more than 4 digits:
				ReAscendFileNameVec(nameStrucVec,targetStr,fileType);

				for(int i=0;i<nameStrucVec.size()-1;i++)
				{								
					lhsFilename.assign(nameStrucVec.at(i).fileName);
					rhsFilename.assign(nameStrucVec.at(i+1).fileName);
					HandleStimStreamRawFile(sp,COMBINE_TWO_STIMULUS,lhsFilename,rhsFilename, rawContainsDisabledChannels);	
				}
			}
			nameStrucVec.clear();
			FindClose(hSearchFile);
			hSearchFile = NULL;
		}
		catch(...)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"Error combining TIS .tmp files");
			ret = FALSE;
		}
		break;
		//******	Delete all .tmp files	******//
	case DELETE_ALL_TEMP:		
		try
		{
			StringCbPrintfW(rawName,_MAX_PATH,L"%s%s\\*.tmp",drive,dir);
			lhsFilename.assign(rawName);
			HandleStimStreamRawFile(sp,DELETE_LEFT_FILETYPE, lhsFilename, rhsFilename, rawContainsDisabledChannels);
		}
		catch(...)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"Error deleting .tmp files");
			ret = FALSE;
		}
		break;
		//******	Delete all .raw files	******//
	case DELETE_RAW:		
		//TODO: need to delete dflim raw
		try
		{
			StringCbPrintfW(rawName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir,sp->index,sp->subWell);
			lhsFilename.assign(rawName);
			HandleStimStreamRawFile(sp,DELETE_LEFT_FILETYPE, lhsFilename, rhsFilename, rawContainsDisabledChannels);
		}
		catch(...)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"Error deleting .raw file");
			ret = FALSE;
		}
		break;
		//******	Delete all .tif files	******//
	case DELETE_TIF:
		try
		{
			StringCbPrintfW(rawName,_MAX_PATH,L"%s%s\\*.tif",drive,dir);
			lhsFilename.assign(rawName);
			HandleStimStreamRawFile(sp,DELETE_LEFT_FILETYPE, lhsFilename, rhsFilename, rawContainsDisabledChannels);
		}
		catch(...)
		{
			StringCbPrintfW(message,MSG_LENGTH,L"Error deleting .tif files");
			ret = FALSE;
		}		
		break;
		//******	Delete all files of left file type, no error catch for others to handle, not intended for user	******//
	case DELETE_LEFT_FILETYPE:
		StringCbPrintfW(rawName,_MAX_PATH,L"%s",lhsFilename.c_str());

		hFile = FindFirstFile(rawName, &FindFileData);

		while (hFile != INVALID_HANDLE_VALUE)
		{
			StringCbPrintfW(rawName,_MAX_PATH,L"%s%s\\%s",drive,dir,FindFileData.cFileName);
			CFuncErrChk (L"DeleteFile", DeleteFile(rawName), false);
			compFile = FindFileData.cFileName;
			FindNextFile(hFile,&FindFileData);
			if(0 == compFile.compare(FindFileData.cFileName))
			{break;}
		}
		FindClose(hFile);
		hFile = NULL;
		break;
	default:
		StringCbPrintfW(message,MSG_LENGTH,L"Incorrect jobID");
		ret = FALSE;
		break;
	}
	return ret;
}

void AcquireTStream::ReAscendFileNameVec(std::vector<FileNameStruc> &nameStrucVec, std::wstring targetStr, std::wstring fileType)
{
	//std::vector<std::pair<std::wstring, long>> nameIndexPairList;
	//char buffer[_MAX_FNAME];
	std::wstring tmpStr;

	int foundOffset = static_cast<int>(targetStr.length());
	int endOffset = static_cast<int>(fileType.length());
	for(int i=0;i<nameStrucVec.size();i++)
	{
		std::wstring::size_type found = nameStrucVec.at(i).fileName.find(targetStr);
		std::wstring::size_type end = nameStrucVec.at(i).fileName.find(fileType);
		if((found!=std::string::npos) && (end!=std::string::npos))
		{
			tmpStr = nameStrucVec.at(i).fileName.substr(found+foundOffset,end-found-endOffset);
			nameStrucVec.at(i).index = std::stol(tmpStr);
		}
	}
	std::sort(nameStrucVec.begin(),nameStrucVec.end(), SortFileNameStrucByIndex);

}

void AcquireTStream::SaveFastZPreviewImageWithoutOME(SaveParams *sp, char *buffer, long bufferSizeBytes)
{
	wchar_t filePathAndName[_MAX_PATH];

	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	//TODO: Add fastZ functionality for DFLIM
	long size = sp->regionMap[sp->previewRegionID].SizeT;
	FrameInfo frameInfo;
	frameInfo.bufferType = BufferType::INTENSITY;
	frameInfo.imageWidth = sp->regionMap[sp->previewRegionID].SizeX;
	frameInfo.imageHeight = sp->regionMap[sp->previewRegionID].SizeY;
	//send buffer to statsManager for ROIStats calculation and storage
	StatsManager::getInstance()->ComputeStats( (unsigned short*)buffer,
		frameInfo,
		sp->lsmChannels,FALSE,TRUE,FALSE);

	long channel = 0;
	for(long i=0; i<sp->colorChannels; i++)
	{
		_wsplitpath_s(sp->path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
		StringCbPrintfW(filePathAndName,_MAX_PATH,L"%s%s%S_Preview.tif",drive,dir,sp->wavelengthName[i].c_str());

		logDll->TLTraceEvent(VERBOSE_EVENT,1,filePathAndName);

		string imageDescription = "";

		for (long ch=0;ch<sp->bufferChannels.begin()->second;ch++)
		{
			if(0 == sp->wavelengthName[i].compare(AcquireFactory::bufferChannelName[ch]))
			{	channel = ch;	}
		}

		SaveTiledTiff(filePathAndName, buffer, bufferSizeBytes, 2 * sp->regionMap[sp->previewRegionID].SizeX, 2 * sp->regionMap[sp->previewRegionID].SizeY, sp->regionMap[sp->previewRegionID].SizeX, sp->regionMap[sp->previewRegionID].SizeY, sp->colorChannels, channel, sp->umPerPixel, imageDescription, false);
	}
}

void AcquireTStream::SavePreviewImage(SaveParams *sp, long tFrameOneBased, char * buffer, long saveEnabledChannelsOnly)
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

	_wsplitpath_s(sp->path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

	if (1 == sp->imageMethod) //dflim acquisition
	{
		//1 datalength for photon num buffer (intensity) (USHORT)
		//1 datalength for single photon sum buffer (USHORT)
		//2 datalength for arrival time sum buffer (UINT32)
		//2 DFLIM_HISTOGRAM_BINS for dflim histogram (UINT32)	
		const int camBufSize = sp->colorChannels  * sp->regionMap[sp->previewRegionID].SizeX * sp->regionMap[sp->previewRegionID].SizeY;

		const int intensityBufferSize = camBufSize * sizeof(USHORT);		
		const int DFLIM_HISTOGRAM_BINS = 256;
		const int dflimHistrogramBufferSize = DFLIM_HISTOGRAM_BINS * sp->colorChannels * sizeof(UINT32);				
		const int dflimArrivalTimeSumBufferSize = camBufSize * sizeof(UINT32);
		const int dflimSinglePhotonSumBufferSize = camBufSize * sizeof(USHORT);
		const int dflimPreviewSize = intensityBufferSize + dflimHistrogramBufferSize + dflimArrivalTimeSumBufferSize + dflimSinglePhotonSumBufferSize;

		const int SHORTS_PER_DFLIMHISTO_BIN = 2;
		FrameInfoStruct frameInfo;
		frameInfo.bufferType = BufferType::DFLIM_IMAGE;
		frameInfo.imageWidth = sp->regionMap[sp->previewRegionID].SizeX;
		frameInfo.imageHeight = sp->regionMap[sp->previewRegionID].SizeY;
		//send buffer to statsManager for ROIStats calculation and storage
		StatsManager::getInstance()->ComputeStats(((unsigned short*)buffer), 
			frameInfo,
			sp->lsmChannels,FALSE,TRUE,saveEnabledChannelsOnly);

		StringCbPrintfW(filePathAndName,_MAX_PATH,L"%s%sPreview.dFLIM",drive,dir);

		ofstream my_file1(filePathAndName, ios_base::binary | ios::out);
		my_file1.write(buffer,dflimPreviewSize);
		my_file1.close();	
		CallNotifySavedFileIPC(filePathAndName);

	}
	else
	{
		long size = sp->regionMap[sp->previewRegionID].SizeT;
		long channel = 0;

		PhysicalSize physicalSize;	// unit: um
		double res = round(sp->umPerPixel*Constants::UM_TO_MM)/Constants::UM_TO_MM;	// keep 2 figures after decimal point, that is why 100 (10^2) is multiplied
		physicalSize.x = res;
		physicalSize.y = res;
		physicalSize.z = _zstageStepSize;

		long doOME = FALSE;//No OME for preview images
		long doCompression = FALSE;//Never Compress Preview Images
		FrameInfoStruct frameInfo;
		frameInfo.bufferType = BufferType::INTENSITY;
		frameInfo.imageWidth = sp->regionMap[sp->previewRegionID].SizeX;
		frameInfo.imageHeight = sp->regionMap[sp->previewRegionID].SizeY;
		//send buffer to statsManager for ROIStats calculation and storage
		StatsManager::getInstance()->ComputeStats((unsigned short*)buffer, 
			frameInfo,
			sp->lsmChannels,FALSE,TRUE,saveEnabledChannelsOnly);


		for(long i=0; i<sp->colorChannels; i++)
		{
			GetLookUpTables(rlut, glut, blut, sp->red[i], sp->green[i], sp->blue[i], sp->bp[i], sp->wp[i],COLOR_MAP_BIT_DEPTH_TIFF);		

			//		StringCbPrintfW(filePathAndName,_MAX_PATH,L"%s%s%S_%04d_%04d_%04d_%04d.tif",drive,dir,sp->wavelengthName[i].c_str(),sp->index,sp->subWell,1,tFrameOneBased);
			StringCbPrintfW(filePathAndName,_MAX_PATH,L"%s%s%S_Preview.tif",drive,dir,sp->wavelengthName[i].c_str());
			//determine channel:
			for (long ch=0;ch<sp->bufferChannels[sp->previewRegionID];ch++)
			{
				if(0 == sp->wavelengthName[i].compare(AcquireFactory::bufferChannelName[ch]))
				{	channel = ch;	}
			}
			logDll->TLTraceEvent(VERBOSE_EVENT,1,filePathAndName);

			if(sp->wavelengthName[i].size() > 0)
			{
				long bufferOffset = (saveEnabledChannelsOnly) ? i*sp->regionMap[sp->previewRegionID].SizeX*sp->regionMap[sp->previewRegionID].SizeY*sizeof(USHORT) : channel*sp->regionMap[sp->previewRegionID].SizeX*sp->regionMap[sp->previewRegionID].SizeY*sizeof(USHORT);

				string timeStamp = "";
				double dt = 0;

				SaveTIFFWithoutOME(filePathAndName,
					buffer+bufferOffset,
					sp->regionMap[sp->previewRegionID].SizeX,
					sp->regionMap[sp->previewRegionID].SizeY,
					rlut,
					glut,
					blut, 
					sp->umPerPixel,
					sp->colorChannels, 
					size, 
					1, 
					0, 
					channel, 
					tFrameOneBased-1, 
					0, 
					&timeStamp, 
					dt,
					doCompression);			
			}
		}
	}
}

string AcquireTStream::uUIDSetup(SaveParams *sp, long bufferChannels, long timePoints, long zstageSteps, long index, long subWell)
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

				_wsplitpath_s(sp->path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

				StringCbPrintfW(filePathAndName,_MAX_PATH,imgNameFormat.str().c_str(),sp->wavelengthName[c].c_str(),index,subWell,z+1,t+1);

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

long AcquireTStream::SaveTimingToExperimentFolder()
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

void AcquireTStream::SaveData( SaveParams *sp, long imageMode, bool stopCheckAllowed, bool rawContainsDisabledChannels, long subwell)		//imageMode: imageManager TRUE, not imageManager FALSE
{
	char * buffer = NULL;

	DWORDLONG dwlsize = sp->regionMap.begin()->second.SizeX*sp->regionMap.begin()->second.SizeY*sp->bufferChannels.begin()->second*sizeof(unsigned short)/sizeof(char);
	DWORD dsize = static_cast<DWORD>(dwlsize);
	HANDLE hFile;

	long channel =0;
	long bufferOffset = 0;

	wchar_t rawName[_MAX_PATH];
	wchar_t filePathAndName[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	_wsplitpath_s(sp->path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

	string wavelengthName;
	long size = sp->regionMap.begin()->second.SizeT;

	//computer zero based totals for z and t
	long totT = static_cast<long>(floor(size/(double)(sp->regionMap.begin()->second.SizeZ+sp->fastFlybackFrames)));
	long totZ = sp->regionMap.begin()->second.SizeZ;

	string strOme = uUIDSetup(sp, sp->colorChannels, totT, totZ, sp->index, sp->subWell);
	string timeStamp ="";
	double dt = 0;

	const int COLOR_MAP_SIZE = 65536;
	unsigned short rlut[COLOR_MAP_SIZE];
	unsigned short glut[COLOR_MAP_SIZE];
	unsigned short blut[COLOR_MAP_SIZE];
	for(long i=0; i<sp->colorChannels; i++)
	{
		const int COLOR_MAP_BIT_DEPTH_TIFF = 8;
		GetLookUpTables(rlut, glut, blut, sp->red[i], sp->green[i], sp->blue[i], sp->bp[i], sp->wp[i],COLOR_MAP_BIT_DEPTH_TIFF);
	}

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormatR;
	imgNameFormatR << L"%s%s\\Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";

	if(FALSE == imageMode)		//Not using imageManager
	{
		buffer = new char[dsize];
		StringCbPrintfW(rawName,_MAX_PATH,imgNameFormatR.str().c_str(),drive,dir,sp->index,sp->subWell);
		hFile = CreateFile(rawName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);	//FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING|FILE_FLAG_SEQUENTIAL_SCAN
	}

	long captureMode;
	_pExp->GetCaptureMode(captureMode);

	//====================
	//    SAVE IMAGES
	//====================

	//=== Set Image Parameters ===
	//resolution
	PhysicalSize physicalSize;	// unit: um
	double res = round(sp->umPerPixel * Constants::UM_TO_MM)/Constants::UM_TO_MM;	// keep 2 figures after decimal point, that is why 100 (10^2) is multiplied
	physicalSize.x = res;
	physicalSize.y = res;
	physicalSize.z = _zstageStepSize;

	//compression
	long doOME = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"OMETIFFTag",L"value", FALSE);
	long doCompression = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"TIFFCompressionEnable",L"value", FALSE);

	std::wstringstream imgNameFormat;
	imgNameFormat << L"%s%s%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";

	//=== Get Active Channels ===
	vector<pair<long,long> > activeChannels = getActiveChannels(*sp);

	//=== Open Image Raw File ===
	boolean chunky = true;
	int actualDepth = (sp->regionMap.begin()->second.SizeZ > 0 ? sp->regionMap.begin()->second.SizeZ + sp->fastFlybackFrames : 1);
	RawFile<unsigned short> rawFile(wstring(rawName), sp->regionMap.begin()->second.SizeX, sp->regionMap.begin()->second.SizeY, actualDepth ,sp->bufferChannels.begin()->second,1,rawContainsDisabledChannels, getEnabledChannelIndices(*sp), GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
	long status = FALSE;

	//=== Save every channel in every image at every depth;
	for(int t=0; t<rawFile.getNumImages(); t++)
	{
		//Open image at time
		GenericImage<unsigned short>& img = rawFile.getImageAtIndex(t);

		for(long z=0; z<static_cast<long>(sp->regionMap.begin()->second.SizeZ); z++)
		{
			try
			{
				for(auto it=activeChannels.begin(); it!=activeChannels.end(); ++it)
				{
					long i=it->first;
					long channel=(sp->bufferChannels.begin()->second > 1 ? it->second : 0);

					if (IExperiment::HYPERSPECTRAL == captureMode)
					{
						StringCbPrintfW(filePathAndName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir,sp->wavelengthName[i].c_str(),t + 1,sp->subWell,z+1,1);
					}
					else
					{
						StringCbPrintfW(filePathAndName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir,sp->wavelengthName[i].c_str(),sp->index,sp->subWell,z+1,t+1);
					}
					logDll->TLTraceEvent(VERBOSE_EVENT,1,filePathAndName);

					//=== Save Tiff ===
					if((sp->wavelengthName[i].size() > 0))
					{
						if(i==0)
						{
							timeStamp = _acquireSaveInfo->getInstance()->RemoveTimestamp();
							dt= _acquireSaveInfo->getInstance()->RemoveTimingInfo();
						}

						//Call Save
						char* bufferAtPos = (char*)img.getDirectPointerToData(0,0,z,channel,0);

						switch ((CaptureFile)sp->fileType)
						{
						case CaptureFile::FILE_TIFF:
							if (TRUE == doOME)
							{
								SaveTIFF(filePathAndName, bufferAtPos, sp->regionMap.begin()->second.SizeX,sp->regionMap.begin()->second.SizeY,rlut,glut,blut, sp->umPerPixel,sp->colorChannels, totT, sp->regionMap.begin()->second.SizeZ, 0, channel, t, z, &timeStamp, dt, &strOme, physicalSize,doCompression);
							}
							else
							{
								SaveTIFFWithoutOME(filePathAndName, bufferAtPos, sp->regionMap.begin()->second.SizeX,sp->regionMap.begin()->second.SizeY,rlut,glut,blut, sp->umPerPixel,sp->colorChannels, totT, sp->regionMap.begin()->second.SizeZ, 0, channel, t, z, &timeStamp, dt,doCompression);
							}
							break;
						case CaptureFile::FILE_BIG_TIFF:
							if (IExperiment::HYPERSPECTRAL == captureMode)
							{
								bigTiff->SaveData(bufferAtPos, static_cast<uint16_t>(i), static_cast<uint16_t>(z+1), 1, static_cast<uint16_t>(t+1));
							}
							else
							{
								bigTiff->SaveData(bufferAtPos, static_cast<uint16_t>(i), static_cast<uint16_t>(z+1), static_cast<uint16_t>(t+1), 1);
							}
							break;
						default:
							break;
						}
					}
				}

				//=== Update Progress ===
				if(z+t<(size-1))
				{
					CallSaveTImage(z+t+1);
					// If it is as Hyperspectral capture with tiling, only callSaveImage for 
					// the first subwell, no need to call it for other subwells.
					if(!(1 < subwell && IExperiment::HYPERSPECTRAL == captureMode))
					{
						CallSaveImage(z+t+1, TRUE);
					}
				}

				//=== Check For Stop ===
				if(true == stopCheckAllowed)
				{
					StopCaptureEventCheck(status);

					//user has asked to stop the capture
					if(TRUE == status)
					{	break;	}
				}
			}
			catch(...)
			{
				StringCbPrintfW(filePathAndName,_MAX_PATH,L"Saving tiff failed.");
				logDll->TLTraceEvent(ERROR_EVENT,1,filePathAndName);
				break;
			}
		}
		if(TRUE == status)
		{	break;	}
	}

	if(FALSE == imageMode)
	{		
		delete[] buffer;
		SAFE_DELETE_HANDLE(hFile);
	}
}

long AcquireTStream::ScannerEnable(long enable)
{
	return ScannerEnableProc(0,enable);
}

long AcquireTStream::SetPMT()
{
	return SetPMTProc( _pExp);
}

vector<int> AcquireTStream::getEnabledChannelIndices(SaveParams& sp)
{
	vector<int> enabledChannels;
	for(auto i : getActiveChannels(sp))
	{
		enabledChannels.push_back(i.second);
	}

	return enabledChannels;
}

vector<pair<long,long> > AcquireTStream::getActiveChannels(SaveParams& sp)
{

	vector<pair<long,long> > activeChannels;

	//=== Get Active Channels ===
	for(int i=0; i<sp.colorChannels; i++)
	{
		for (long ch=0; ch < MAX_CHANNELS; ch++)
		{
			if(0 == sp.wavelengthName[i].compare(AcquireFactory::bufferChannelName[ch]))
			{
				activeChannels.push_back(pair<long, long>(i, ch));
			}
		}
	}	

	return activeChannels;
}

shared_ptr<InternallyStoredImage <unsigned short> > AcquireTStream::createFullChannelCopy(char* bufferWithOnlyEnabledChannels, SaveParams& sp)
{

	InternallyStoredImage<unsigned short>* allChannelImage = new InternallyStoredImage<unsigned short>(sp.regionMap.begin()->second.SizeX, 
		sp.regionMap.begin()->second.SizeY, 
		1, 
		sp.bufferChannels.begin()->second, 
		1, 
		GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
	shared_ptr<InternallyStoredImage <unsigned short> > fullChannelCopy(allChannelImage);

	GenericImage<unsigned short> existingImage(sp.regionMap.begin()->second.SizeX, 
		sp.regionMap.begin()->second.SizeY, 
		1, 
		sp.bufferChannels.begin()->second, 
		1, 
		GenericImage<unsigned short>::CONTIGUOUS_CHANNEL, 
		(sp.bufferChannels.begin()->second==1? std::vector<int>() : getEnabledChannelIndices(sp)));
	existingImage.setMemoryBuffer((unsigned short*) bufferWithOnlyEnabledChannels);
	allChannelImage->copyFrom(existingImage);

	return fullChannelCopy;

}

AcquireTStream::HeaderInfo::HeaderInfo()
{
	Init();
}

AcquireTStream::HeaderInfo::HeaderInfo(string headerFilename)
{
	Init();
	if(!parseHeaderInfo(headerFilename))
	{
		printf("Load hdr file failed!");
	}
}

AcquireTStream::HeaderInfo::~HeaderInfo()
{
}

bool AcquireTStream::HeaderInfo::parseHeaderInfo(string headerFilename)
{
	int i = 0;
	ifstream fin(headerFilename, ios::in);
	char line[2048] = { 0 };
	auto isWavelength = false;

	while (fin.getline(line, sizeof line))
	{
		// strip trailing whitespace from the string
		i = static_cast<long>(strlen(line)) - 1;
		if (i > 2047)
		{
			return false;
		}

		int j = 0;
		for (j = i; j >= 0; j--)
		{
			if (static_cast<unsigned char>(line[j]) < 9)
			{
				return false;
			}
		}

		while (i >= 0 && isspace(line[i]))
		{
			line[i--] = 0;
		}

		if (strlen(line) == 0)
		{
			continue;
		}

		// find the '=' position
		auto EqIndex = -1;
		for (j = i; j >= 0; j--)
		{
			if (line[j] == '=')
			{
				EqIndex = j;
				break;
			}
		}

		if (EqIndex <= 0)
		{
			if (!isWavelength)
			{
				continue;   //no equal found.
			}
			string wlstr(line);
			FillWavelength(wlstr);
			continue;
		}

		string lineStr(line);
		auto strKey = lineStr.substr(0, EqIndex - 1);
		auto strVal = lineStr.substr(EqIndex + 1, lineStr.length() - EqIndex);

		strKey = trim(strKey);
		strVal = trim(strVal);
		transform(strKey.begin(), strKey.end(), strKey.begin(), toupper);

		isWavelength = strKey == "WAVELENGTH";
		FillProps(strKey, strVal);
	}
	fin.clear();
	fin.close();

	return true;
}

string AcquireTStream::HeaderInfo::trim(string& s)
{
	const string drop = " ";
	// trim right
	s.erase(s.find_last_not_of(drop) + 1);
	// trim left
	return s.erase(0, s.find_first_not_of(drop));
}

void AcquireTStream::HeaderInfo::FillProps(string sKey, string sVal)
{

	if (sKey == "DESCRIPTION")
	{
		this->Description = sVal;
	}
	if (sKey == "SENSOR TYPE")
	{
		this->SensorType = sVal;
	}
	if (sKey == "FILE TYPE")
	{
		this->FileType = sVal;
	}

	if (sKey == "HEADER OFFSET")
	{
		this->HeaderOffset = atoi(sVal.c_str());
	}
	if (sKey == "SAMPLES")
	{
		this->Samples = atoi(sVal.c_str());
	}
	if (sKey == "LINES")
	{
		this->Lines = atoi(sVal.c_str());
	}
	if (sKey == "BANDS")
	{
		this->bands = atoi(sVal.c_str());
	}
	if (sKey == "DATA TYPE")
	{
		this->DataType = atoi(sVal.c_str());
	}
	if (sKey == "INTERLEAVE")
	{
		this->Interleave = sVal;
	}
	if (sKey == "BYTE ORDER")
	{
		this->ByteOrder = atoi(sVal.c_str());
	}
}

void AcquireTStream::HeaderInfo::FillWavelength(string wlstr) const
{
	wlstr.erase(wlstr.find_last_not_of(",") + 1);
	wlstr.erase(wlstr.find_last_not_of("}") + 1);
	this->WaveLength->push_back(static_cast<float>(atof(wlstr.c_str())));
}

void AcquireTStream::HeaderInfo::Init()
{
	Description = "";
	SensorType = "";
	FileType = "";
	HeaderOffset = 0;
	Samples = 0;
	Lines = 0;
	bands = 0;
	DataType = 0;
	Interleave = "BIL";
	ByteOrder = 0;
	WaveLength = new vector<float>;
}

bool AcquireTStream::saveHeaderHdr(wstring filename, HeaderInfo& hinfo)
{
	ofstream fout(filename);
	if (!fout)return false;

	fout << "ENVI" << endl;
	fout << "description = " << hinfo.Description << endl;
	fout << "sensor type = " << hinfo.SensorType << endl;
	fout << "file type = " << hinfo.FileType << endl;
	fout << "header offset = " << hinfo.HeaderOffset << endl;
	fout << "samples = " << hinfo.Samples << endl;
	fout << "lines = " << hinfo.Lines << endl;
	fout << "bands = " << hinfo.bands << endl;
	fout << "data type = " << hinfo.DataType << endl;
	fout << "interleave = " << hinfo.Interleave << endl;
	fout << "byte order = " << hinfo.ByteOrder << endl;
	fout << "wavelength = {" << endl;


	auto bandCnt = 0;
	for (vector<float>::const_iterator wlItr = hinfo.WaveLength->begin();
		wlItr != hinfo.WaveLength->end(); ++wlItr)
	{
		bandCnt++;
		auto wl = *wlItr;

		if (bandCnt < hinfo.bands)
		{
			fout << " " << setiosflags(ios::fixed) << setprecision(4) << wl << "," << endl;
		}
		else
		{
			fout << " " << setiosflags(ios::fixed) << setprecision(4) << wl << "}" << endl;
		}
	}
	fout.clear();
	fout.close();

	return true;
}

