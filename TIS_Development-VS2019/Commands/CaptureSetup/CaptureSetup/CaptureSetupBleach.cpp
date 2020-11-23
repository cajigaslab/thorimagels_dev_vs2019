#include "stdafx.h"
#include "math.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"

std::unique_ptr<HDF5ioDLL> h5io(new HDF5ioDLL(L".\\Modules_Native\\HDF5IO.dll"));

void (*myFuncPtrBleach)() = NULL;
void (*myFuncPtrPreBleach)(long* status) = NULL;

const int THREAD_CHECKTIME = 10;

DllExportLiveImage InitCallBackBleach(completeCallback	dt, preCapturetype pc) 
{
	myFuncPtrBleach = dt;

	myFuncPtrPreBleach = pc;

	if((myFuncPtrBleach != NULL) && (myFuncPtrPreBleach != NULL))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup InitCallBackBleach");
	}

	return TRUE;
}

DllExportLiveImage InitializeBleach()
{
	stopBleach = FALSE;	

	//create bleach params:
	for (int id = 0; id < MAX_BLEACH_PARAMS_CNT; id++)
	{
		if(CaptureSetup::getInstance()->bleachParams[id] == NULL)
		{
			CaptureSetup::getInstance()->bleachParams[id] = new GGalvoWaveformParams();
			CaptureSetup::getInstance()->bleachParams[id]->lastLoaded = FALSE;
			CaptureSetup::getInstance()->bleachParams[id]->bufferHandle =  CreateMutex(NULL, false, NULL);
		}
	}

	return TRUE;
}

DllExportLiveImage GetBleachScannerParameterValueDouble(long paramID, double &val)
{
	ICamera * pCamera;
	pCamera = GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER);
	long ret = FALSE;

	if(NULL == pCamera)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL", __FUNCTION__, __LINE__);
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pCamera->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		if(paramAvailable)
		{
			pCamera->GetParam(paramID,val);

			if((val >= paramMin)||(val <= paramMax))
			{
				ret = TRUE;
			}
		}
	}
	return ret;
}

//bleach camera thread proc
UINT StatusBleachScannerProc( LPVOID pParam )
{
	long status = ICamera::STATUS_BUSY;

	while((FALSE == stopBleach) && (status == ICamera::STATUS_BUSY))
	{
		if(FALSE == GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER) ->StatusAcquisition(status))		
		{
			break;
		}
	}

	if(hStatusBleachScanner)
	{
		SetEvent(hStatusBleachScanner);
	}

	return 0;
}

long GetBleachScannerParameterValueRangeDouble(long paramID, double &valMin, double &valMax, double &valDefault)
{
	ICamera * pCamera;
	pCamera = GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER);
	long ret = FALSE;

	if(NULL == pCamera)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL", __FUNCTION__, __LINE__);
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pCamera->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		valMin = paramMin;
		valMax = paramMax;
		valDefault = paramDefault;
		ret = TRUE;
	}
	return ret;
}

DllExportLiveImage LoadH5BleachWaveform(const wchar_t* bleachfilePathName, int CycleNum, int id)
{
	//wait for bleach params:
	if(NULL != CaptureSetup::getInstance()->bleachParams[id]->bufferHandle)
	{
		if(WAIT_OBJECT_0 != WaitForSingleObject(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle, Constants::EVENT_WAIT_TIME))
		{return FALSE;}
	}
	//not necessary to clear since using realloc instead of malloc:
	//CaptureSetup::getInstance()->ClearBleachParams(id);
	CaptureSetup::getInstance()->bleachParams[id]->lastLoaded = FALSE;

	//retrieve user's H5 waveform:
	CaptureSetup::getInstance()->bleachParams[id]->Scanmode = ScanMode::BLEACH_SCAN;
	CaptureSetup::getInstance()->bleachParams[id]->Triggermode = ICamera::SW_MULTI_FRAME;
	CaptureSetup::getInstance()->bleachParams[id]->CycleNum = CycleNum;
	if(FALSE == h5io->OpenFileIO(bleachfilePathName,H5FileType::READONLY))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup Bleach Open BleachWaveform.raw failed");
		ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
		return FALSE;
	}
	unsigned long long dataSize;
	BOOL ret = TRUE;
	if(TRUE == h5io->CheckGroupDataset("/Analog","/Pockel",dataSize) && dataSize > 0)
	{
		inFileLoading = TRUE;			
		CaptureSetup::getInstance()->bleachParams[id]->analogPockelSize = static_cast<long>(dataSize);			//unit size
		CaptureSetup::getInstance()->bleachParams[id]->analogXYSize = static_cast<long>(dataSize * 2);			//2x unit size
		CaptureSetup::getInstance()->bleachParams[id]->digitalSize = static_cast<long>(dataSize * 6);			//pockels digital with complete, cycle, iteration, pattern, patternComplete lines
		CaptureSetup::getInstance()->bleachParams[id]->GalvoWaveformXY = (double*)realloc((void*)CaptureSetup::getInstance()->bleachParams[id]->GalvoWaveformXY, CaptureSetup::getInstance()->bleachParams[id]->analogXYSize*sizeof(double));
		if(NULL == CaptureSetup::getInstance()->bleachParams[id]->GalvoWaveformXY)
		{
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;	
		}
		CaptureSetup::getInstance()->bleachParams[id]->GalvoWaveformPockel = (double*)realloc((void*)CaptureSetup::getInstance()->bleachParams[id]->GalvoWaveformPockel, CaptureSetup::getInstance()->bleachParams[id]->analogPockelSize*sizeof(double));
		if(NULL == CaptureSetup::getInstance()->bleachParams[id]->GalvoWaveformPockel)
		{
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;	
		}
		CaptureSetup::getInstance()->bleachParams[id]->DigBufWaveform = (unsigned char*)realloc((void*)CaptureSetup::getInstance()->bleachParams[id]->DigBufWaveform, CaptureSetup::getInstance()->bleachParams[id]->digitalSize*sizeof(unsigned char));
		if(NULL == CaptureSetup::getInstance()->bleachParams[id]->DigBufWaveform)
		{
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;	
		}

		unsigned long * tmpClk = (unsigned long*)malloc(sizeof(unsigned long));
		if (FALSE == h5io->ReadData("","/ClockRate",tmpClk,H5DataTypeEnum::DATA_UINT32,0,1)) 
		{
			ret = FALSE;
		}
		CaptureSetup::getInstance()->bleachParams[id]->ClockRate = tmpClk[0];
		free(tmpClk);
		if (FALSE == h5io->ReadData("/Digital","/PockelDig",CaptureSetup::getInstance()->bleachParams[id]->DigBufWaveform,H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = FALSE;
		}
		if(TRUE == stopBleach)
		{
			h5io->CloseFileIO();
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;
		}
		if (FALSE == h5io->ReadData("/Digital","/CycleComplete",CaptureSetup::getInstance()->bleachParams[id]->DigBufWaveform+dataSize*sizeof(unsigned char),H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = FALSE;
		}
		if(TRUE == stopBleach)
		{
			h5io->CloseFileIO();
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return FALSE;
		}
		if (FALSE == h5io->ReadData("/Digital","/CycleEnvelope",CaptureSetup::getInstance()->bleachParams[id]->DigBufWaveform+2*dataSize*sizeof(unsigned char),H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = FALSE;
		}
		if(TRUE == stopBleach)
		{
			h5io->CloseFileIO();
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;
		}
		if (FALSE == h5io->ReadData("/Digital","/IterationEnvelope",CaptureSetup::getInstance()->bleachParams[id]->DigBufWaveform+3*dataSize*sizeof(unsigned char),H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = FALSE;
		}
		if(TRUE == stopBleach)
		{
			h5io->CloseFileIO();
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;
		}
		if (FALSE == h5io->ReadData("/Digital","/PatternEnvelope",CaptureSetup::getInstance()->bleachParams[id]->DigBufWaveform+4*dataSize*sizeof(unsigned char),H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = FALSE;
		}
		if(TRUE == stopBleach)
		{
			h5io->CloseFileIO();
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;
		}
		if (FALSE == h5io->ReadData("/Digital","/PatternComplete",CaptureSetup::getInstance()->bleachParams[id]->DigBufWaveform+5*dataSize*sizeof(unsigned char),H5DataTypeEnum::DATA_UCHAR,0,dataSize)) 
		{
			ret = FALSE;
		}
		if(TRUE == stopBleach)
		{
			h5io->CloseFileIO();
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;
		}
		if (FALSE == h5io->ReadData("/Analog","/Pockel",CaptureSetup::getInstance()->bleachParams[id]->GalvoWaveformPockel,H5DataTypeEnum::DATA_DOUBLE,0,dataSize))
		{
			ret = FALSE;
		}
		if(TRUE == stopBleach)
		{
			h5io->CloseFileIO();
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;
		}
		//interleave XY waveform for Galvo-Galvo:
		double * tmpWf = (double *) malloc(CaptureSetup::getInstance()->bleachParams[id]->analogPockelSize*sizeof(double));
		if (FALSE == h5io->ReadData("/Analog","/X",tmpWf,H5DataTypeEnum::DATA_DOUBLE,0,dataSize))
		{
			ret = FALSE;
		}
		if(TRUE == stopBleach)
		{
			free(tmpWf);
			h5io->CloseFileIO();
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;
		}
		for(unsigned long i=0;i<CaptureSetup::getInstance()->bleachParams[id]->analogPockelSize;i++)
		{
			CaptureSetup::getInstance()->bleachParams[id]->GalvoWaveformXY[2*i]=tmpWf[i];
		}
		if(TRUE == stopBleach)
		{
			free(tmpWf);
			h5io->CloseFileIO();
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;
		}
		if (FALSE == h5io->ReadData("/Analog","/Y",tmpWf,H5DataTypeEnum::DATA_DOUBLE,0,dataSize))
		{
			ret = FALSE;
		}
		if(TRUE == stopBleach)
		{
			free(tmpWf);
			h5io->CloseFileIO();
			ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
			return inFileLoading = FALSE;
		}
		for(unsigned long i=0;i<CaptureSetup::getInstance()->bleachParams[id]->analogPockelSize;i++)
		{
			CaptureSetup::getInstance()->bleachParams[id]->GalvoWaveformXY[2*i+1]=tmpWf[i];
		}
		free(tmpWf);
	}
	h5io->CloseFileIO();
	ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
	if(FALSE == ret)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup Bleach Read BleachWaveform.raw failed");
		ReleaseMutex(CaptureSetup::getInstance()->bleachParams[id]->bufferHandle);
		return inFileLoading = FALSE;
	}
	else
	{
		CaptureSetup::getInstance()->bleachParams[id]->lastLoaded = TRUE;
	}
	inFileLoading = FALSE;
	return ret;
}

DllExportLiveImage GetIsBleach() 
{
	return activeBleach;
}

UINT BleachThreadProc(LPVOID pParam)
{	
	long ret = TRUE;
	if(NULL == pParam)
	{
		SetEvent(hEventBleach);
		return FALSE;
	}

	GGalvoWaveformParams* bParams = (GGalvoWaveformParams*)pParam;

	if(NULL != bParams->bufferHandle)
	{
		WaitForSingleObject(bParams->bufferHandle, INFINITE);
	}
	//do load when buffer was last loaded:
	if(TRUE == bParams->lastLoaded)
	{
		//update bleach scanner parameters:
		if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_MULTI_FRAME_COUNT,static_cast<double>(bParams->CycleNum)))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup BleachThreadProc SetParam PARAM_MULTI_FRAME_COUNT failed");
			logDll->TLTraceEvent(WARNING_EVENT,1,message);
			ret = FALSE;
		}

		if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_LSM_SCANMODE,static_cast<double>(bParams->Scanmode)))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup BleachThreadProc SetParam PARAM_LSM_SCANMODE failed");
			logDll->TLTraceEvent(WARNING_EVENT,1,message);
			ret = FALSE;
		}
		if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_TRIGGER_MODE,static_cast<double>(bParams->Triggermode)))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup BleachThreadProc SetParam PARAM_TRIGGER_MODE failed");
			logDll->TLTraceEvent(WARNING_EVENT,1,message);
			ret = FALSE;
		}	
		if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS,static_cast<double>(bParams->PreCapStatus)))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup BleachThreadProc SetParam PARAM_LSM_WAVEFORM_PRECAPTURESTATUS failed");
			logDll->TLTraceEvent(WARNING_EVENT,1,message);
			ret = FALSE;
		}
		if(FALSE == SetCameraParamString(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_LSM_WAVEFORM_PATH_NAME,bParams->WaveFileName))
		{
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup BleachThreadProc SetParam PARAM_LSM_WAVEFORM_PATH_NAME failed");
			logDll->TLTraceEvent(WARNING_EVENT,1,message);
			ret = FALSE;
		}

		////The NI function inside the bleacher takes Int32(long) as the argument for the length of the waveform. Thus, the bleacher's takes a long as argument
		////for the length of the buffer. For this reason we cast the int64(size_t) variable to a long before sending it.
		////The variable in the bleach parameters is of size_t because it comforms to the H5 file read function that allows for size_t variables.
		////This applies to the bleach parameters: digtalSize, analogXYSize, analogPockelsSize
		////[Dropped]:use active load in waveform mode
		//if(FALSE == SetCameraParamBuffer(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_LSM_WAVEFORM_DIGITAL,(char*)bParams->DigBufWaveform,static_cast<long>(bParams->digitalSize)))
		//{
		//	StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup SLMBleach SetParam PARAM_LSM_WAVEFORM_DIGITAL failed");
		//	logDll->TLTraceEvent(WARNING_EVENT,1,message);
		//	ret = FALSE;
		//}
		////user asked to stop:
		//if (TRUE == stopBleach)
		//{	
		//	ReleaseMutex(bParams->bufferHandle);
		//	SetEvent(hEventBleach);
		//	return FALSE;
		//}
		//if(FALSE == SetCameraParamBuffer(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_LSM_WAVEFORM_ANALOG_XY,(char*)bParams->GalvoWaveformXY,static_cast<long>(bParams->analogXYSize)))
		//{
		//	StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup SLMBleach SetParam PARAM_LSM_WAVEFORM_ANALOG_XYP failed");
		//	logDll->TLTraceEvent(WARNING_EVENT,1,message);
		//	ret = FALSE;
		//}	
		////user asked to stop:
		//if (TRUE == stopBleach)
		//{	
		//	ReleaseMutex(bParams->bufferHandle);
		//	SetEvent(hEventBleach);
		//	return FALSE;
		//}
		//if(FALSE == SetCameraParamBuffer(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_LSM_WAVEFORM_ANALOG_POCKEL,(char*)bParams->GalvoWaveformPockel,static_cast<long>(bParams->analogPockelSize)))
		//{
		//	StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup SLMBleach SetParam PARAM_LSM_WAVEFORM_ANALOG_XYP failed");
		//	logDll->TLTraceEvent(WARNING_EVENT,1,message);
		//	ret = FALSE;
		//}	

		//user asked to stop:
		if (TRUE == stopBleach)
		{	
			ReleaseMutex(bParams->bufferHandle);
			SetEvent(hEventBleach);
			return FALSE;
		}
	}
	ReleaseMutex(bParams->bufferHandle);

	//user asked to stop:
	if (TRUE == stopBleach)
	{	
		SetEvent(hEventBleach);
		return FALSE;
	}

	if(FALSE == GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER)->PreflightAcquisition(pChan[0]))	
	{		
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup SLMBleach PreflightAcquisition failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);	
		SetEvent(hEventBleach);
		return FALSE;
	}	

	if(FALSE == GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER)->SetupAcquisition(pChan[0])) 
	{	
		PostflightCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER);
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup SLMBleach SetupAcquisition failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);	
		MessageBox(NULL,L"When bleaching and imaging at the same time, the pockels cell cannot be controlled by the same board as the Image Detector. A separate board is necessary.",L"Bleach Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY | MB_ICONERROR);
		SetEvent(hEventBleach);
		return FALSE;
	}
	if(FALSE == GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER)->StartAcquisition(pChan[0]))	
	{		
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup SLMBleach StartAcquisition failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);	
		SetEvent(hEventBleach);
		return FALSE;
	}

	//Status Thread:
	SAFE_DELETE_HANDLE(hStatusBleachScanner);
	hStatusBleachScanner = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThreadb;

	HANDLE hThreadb = NULL;

	hThreadb = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusBleachScannerProc, (LPVOID)GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER), 0, &dwThreadb );	

	while(WaitForSingleObject(hStatusBleachScanner, THREAD_CHECKTIME) != WAIT_OBJECT_0)
	{
		if(TRUE == stopBleach)
			break;
	}
	SAFE_DELETE_HANDLE(hThreadb);
	SAFE_DELETE_HANDLE(hStatusBleachScanner);

	//Done bleaching
	SetEvent(hEventBleach);

	return ret;
}

void CreateThenWaitBleacherThread()
{
	SAFE_DELETE_HANDLE(hEventBleach);
	hEventBleach = CreateEvent(0, FALSE, FALSE, 0);

	SAFE_DELETE_HANDLE(hBleachThread);

	hBleachThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) BleachThreadProc, (LPVOID)(CaptureSetup::getInstance()->bleachParams[0]), 0, &dwBleachThreadId );
	activeBleach = TRUE;

	//wait until done:
	while(WaitForSingleObject(hEventBleach, THREAD_CHECKTIME) != WAIT_OBJECT_0)
	{
		Sleep(1);
	}
	SAFE_DELETE_HANDLE(hEventBleach);
	SAFE_DELETE_HANDLE(hBleachThread);
}

BOOL ValidateBleacher()
{
	////Check active camera to do bleach:
	if(0 == GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER))
	{	
		logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup could not create bleach scanner");
		return FALSE;
	}
	double lsmType;
	if(TRUE == GetCameraParamDouble(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_LSM_TYPE,lsmType))
	{
		if(ICamera::LSMType::GALVO_GALVO != lsmType && ICamera::LSMType::STIMULATE_MODULATOR != lsmType)
		{
			logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup bleach scanner can only be galvo-galvo");
			return FALSE;
		}
	}

	//stop camera if it is GALVO_GALVO:
	if(TRUE == GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_TYPE,lsmType))
	{
		if(ICamera::LSMType::GALVO_GALVO == lsmType)
		{
			PostflightCamera(SelectedHardware::SELECTED_CAMERA1);
		}
	}

	return TRUE;
}

DllExportLiveImage Bleach(const wchar_t* bleachfilePathName, int cycleNum)
{	
	if(FALSE == ValidateBleacher())
	{
		return FALSE;
	}

	long PreCapStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE;

	//initialize for bleach requirements:
	InitializeBleach();

	do
	{
		(*myFuncPtrPreBleach)(&PreCapStatus);

		//user asked to stop:
		if(TRUE == stopBleach)
		{			
			PreCapStatus = PreCaptureStatus::PRECAPTURE_DONE;
			break;
		}

		inFileLoading = FALSE;

		CaptureSetup::getInstance()->bleachParams[0]->CycleNum = cycleNum;
		CaptureSetup::getInstance()->bleachParams[0]->lastLoaded = TRUE;
		CaptureSetup::getInstance()->bleachParams[0]->Scanmode = ScanMode::BLEACH_SCAN;
		CaptureSetup::getInstance()->bleachParams[0]->Triggermode = ICamera::TriggerMode::SW_MULTI_FRAME;
		CaptureSetup::getInstance()->bleachParams[0]->PreCapStatus = PreCapStatus;
		wcsncpy_s (CaptureSetup::getInstance()->bleachParams[0]->WaveFileName, bleachfilePathName,_MAX_PATH);

		//start thread for bleaching, work with single buffer only:
		CreateThenWaitBleacherThread();

		//Done bleaching:
		if(FALSE == PostflightCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER))		
		{		
			StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Bleach PostflightAcquisition failed");
			logDll->TLTraceEvent(ERROR_EVENT,1,message);	
		}

		//error in bleacher will reset capture status:
		double bleacherCapStatus = 0;
		GetCameraParamDouble(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS, bleacherCapStatus);

		if((TRUE == stopBleach) || 
			(PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE == PreCapStatus) ||
			((int)(PreCaptureStatus::PRECAPTURE_BLEACHER_ERROR) == (int)bleacherCapStatus))
		{
			//Done:
			PreCapStatus = PreCaptureStatus::PRECAPTURE_DONE;
		}
	}
	while(PreCaptureStatus::PRECAPTURE_DONE != PreCapStatus);

	//notify bleach finished:
	(*myFuncPtrBleach)();		
	activeBleach = FALSE;

	return TRUE;
}

DllExportLiveImage ReleaseBleachParams()
{
	for (int i = 0; i < MAX_BLEACH_PARAMS_CNT; i++)
	{
		if(CaptureSetup::getInstance()->bleachParams[i] != NULL)
		{
			if(CaptureSetup::getInstance()->bleachParams[i]->analogPockelSize > 0)
			{
				free(CaptureSetup::getInstance()->bleachParams[i]->GalvoWaveformPockel);
			}
			if(CaptureSetup::getInstance()->bleachParams[i]->analogXYSize > 0)
			{
				free(CaptureSetup::getInstance()->bleachParams[i]->GalvoWaveformXY);
			}
			if(CaptureSetup::getInstance()->bleachParams[i]->digitalSize > 0)
			{
				free(CaptureSetup::getInstance()->bleachParams[i]->DigBufWaveform);
			}

			SAFE_DELETE_HANDLE(CaptureSetup::getInstance()->bleachParams[i]->bufferHandle);

			delete CaptureSetup::getInstance()->bleachParams[i];
			CaptureSetup::getInstance()->bleachParams[i] = NULL;
		}
	}
	return TRUE;
}

DllExportLiveImage SetBleachPowerPosition(double pos)
{	
	long ret = FALSE;

	//try to bleach the sample with the pockels cell first
	// if the setparameter fails then attempt a bleach with the
	//power regulator
	ret = SetCameraParamDouble(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0,static_cast<double>(pos));

	if(FALSE == ret)
	{
		ret = SetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR,IDevice::PARAM_POWER_POS,pos,TRUE);
		Sleep(5000);
	}

	return ret;
}

DllExportLiveImage SetBleachWaveform(const wchar_t* bleachfilePathName, int CycleNum)
{
	//wait for bleach params:
	if(NULL != CaptureSetup::getInstance()->bleachParams[0]->bufferHandle)
	{
		if(WAIT_OBJECT_0 != WaitForSingleObject(CaptureSetup::getInstance()->bleachParams[0]->bufferHandle, Constants::EVENT_WAIT_TIME))
			return FALSE;
	}

	CaptureSetup::getInstance()->bleachParams[0]->CycleNum = CycleNum;
	CaptureSetup::getInstance()->bleachParams[0]->lastLoaded = TRUE;
	CaptureSetup::getInstance()->bleachParams[0]->Scanmode = ScanMode::BLEACH_SCAN;
	CaptureSetup::getInstance()->bleachParams[0]->Triggermode = ICamera::TriggerMode::SW_MULTI_FRAME;
	wcsncpy_s (CaptureSetup::getInstance()->bleachParams[0]->WaveFileName, bleachfilePathName,_MAX_PATH);
	ReleaseMutex(CaptureSetup::getInstance()->bleachParams[0]->bufferHandle);
	return TRUE;
}

///***	SLM Bleach	***///
// SLM will share with bleach since they are not intended to run simultaneously

void (*myFuncPtrBleachSLM)() = NULL;
void (*myFuncPtrPreBleachSLM)(long* slmPreCapStatus) = NULL;

// Connect SLM callbacks
DllExportLiveImage InitCallBackBleachSLM(completeCallback dt, preCapturetype pc) 
{
	myFuncPtrBleachSLM = dt;

	myFuncPtrPreBleachSLM = pc;

	if((myFuncPtrBleachSLM != NULL) && (myFuncPtrPreBleachSLM != NULL))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup InitCallBackBleachSLM");
	}

	return TRUE;
}

DllExportLiveImage PostflightSLMBleachScanner()
{
	long retVal = TRUE;
	//stop slm before bleach scanner first:
	IDevice* slm = GetDevice(SelectedHardware::SELECTED_SLM);
	if(NULL != slm)
	{
		retVal = slm->PostflightPosition();
	}

	if((retVal = PostflightCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER)) && (FALSE == retVal))		
	{		
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Bleach PostflightAcquisition failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);	
	}

	return retVal;
}

DllExportLiveImage SLMBleach()
{		
	if(FALSE == ValidateBleacher())
	{
		return FALSE;
	}

	long slmCapStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE; 

	//initialize for bleach requirements:
	InitializeBleach();

	do
	{
		//get slm capture status:
		(*myFuncPtrPreBleachSLM)(&slmCapStatus);

		//user asked to stop:
		if(TRUE == stopBleach)
		{			
			slmCapStatus = PreCaptureStatus::PRECAPTURE_DONE;
			break;
		}
		//set pre-capture status:
		if(NULL != CaptureSetup::getInstance()->bleachParams[0]->bufferHandle)
		{
			if(WAIT_OBJECT_0 != WaitForSingleObject(CaptureSetup::getInstance()->bleachParams[0]->bufferHandle, Constants::EVENT_WAIT_TIME))
				break;
		}
		CaptureSetup::getInstance()->bleachParams[0]->PreCapStatus = slmCapStatus;
		ReleaseMutex(CaptureSetup::getInstance()->bleachParams[0]->bufferHandle);

		//execute bleaching:
		CreateThenWaitBleacherThread();

		//error in bleacher will reset capture status:
		double bleacherCapStatus = 0;
		GetCameraParamDouble(SelectedHardware::SELECTED_BLEACHINGSCANNER,ICamera::PARAM_LSM_WAVEFORM_PRECAPTURESTATUS, bleacherCapStatus);

		if((TRUE == stopBleach) || 
			(PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE == slmCapStatus) ||
			((int)(PreCaptureStatus::PRECAPTURE_BLEACHER_ERROR) == (int)bleacherCapStatus))
		{
			//Done:
			slmCapStatus = PreCaptureStatus::PRECAPTURE_DONE;
		}
	}
	while(PreCaptureStatus::PRECAPTURE_DONE != slmCapStatus);

	PostflightSLMBleachScanner();

	//notify bleach finished:
	(*myFuncPtrBleachSLM)();		
	activeBleach = FALSE;

	return TRUE;
}

DllExportLiveImage CalibrateSLM(const wchar_t* bmpPatternName, float* xyPointFrom, float* xyPointTo, long size, long pixelX, long pixelY)
{
	long ret = TRUE;

	///***	SLM device	***///
	SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_FUNC_MODE,IDevice::SLMFunctionMode::PHASE_CALIBRATION, IDevice::DeviceSetParamType::NO_EXECUTION);
	SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_PIXEL_X,pixelX, IDevice::DeviceSetParamType::NO_EXECUTION);
	SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_PIXEL_Y,pixelY, IDevice::DeviceSetParamType::NO_EXECUTION);
	SetDeviceParamString(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_BMP_FILENAME,(wchar_t*)bmpPatternName, IDevice::DeviceSetParamType::NO_EXECUTION);
	SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_ARRAY_ID, 1, IDevice::DeviceSetParamType::NO_EXECUTION);
	SetDeviceParamBuffer(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_POINTS_ARRAY,(char*)xyPointTo,size, IDevice::DeviceSetParamType::NO_EXECUTION);
	//going to display id == 0:
	SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_ARRAY_ID, 0, IDevice::DeviceSetParamType::NO_EXECUTION);
	SetDeviceParamBuffer(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_POINTS_ARRAY,(char*)xyPointFrom,size, IDevice::DeviceSetParamType::EXECUTION_WAIT);

	///***	Galvo-Galvo	 ***///
	std::wstring pathName(bmpPatternName);
	pathName = pathName.substr(0,pathName.find_last_of(L"\\")) + L"\\SLMCalibWaveform.raw";

	//initialize:
	InitializeBleach();

	//set waveform params:
	if(FALSE == SetBleachWaveform(pathName.c_str(), 1))
	{
		return FALSE;
	}
	//set preCaptureStatus separately as callback did:
	if(NULL != CaptureSetup::getInstance()->bleachParams[0]->bufferHandle)
	{
		if(WAIT_OBJECT_0 != WaitForSingleObject(CaptureSetup::getInstance()->bleachParams[0]->bufferHandle, Constants::EVENT_WAIT_TIME))
			return FALSE;
	}
	CaptureSetup::getInstance()->bleachParams[0]->PreCapStatus = PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE;
	ReleaseMutex(CaptureSetup::getInstance()->bleachParams[0]->bufferHandle);

	//check scanner is Galvo-Galvo:
	if(FALSE == ValidateBleacher())
	{
		return FALSE;
	}

	if(stopBleach)
	{
		return FALSE;
	}

	//execute:
	CreateThenWaitBleacherThread();

	//done:
	if(FALSE == PostflightCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER))		
	{		
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup Bleach PostflightAcquisition failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);	
	}

	activeBleach = FALSE;
	return ret;
}

DllExportLiveImage LoadSLMPattern(long runtimeCal, long id, const wchar_t* bmpPatternName, long doStart, long timeout)
{
	long ret = TRUE;
	SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_RUNTIME_CALC, runtimeCal, IDevice::DeviceSetParamType::NO_EXECUTION);
	SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_FUNC_MODE,IDevice::SLMFunctionMode::LOAD_PHASE_ONLY, IDevice::DeviceSetParamType::NO_EXECUTION);
	ret = SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_ARRAY_ID,id,IDevice::DeviceSetParamType::NO_EXECUTION);
	ret = SetDeviceParamString(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_BMP_FILENAME,(wchar_t*)bmpPatternName,IDevice::DeviceSetParamType::NO_EXECUTION);
	ret = SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_TIMEOUT,timeout,IDevice::DeviceSetParamType::NO_EXECUTION);

	IDevice* slm = GetDevice(SelectedHardware::SELECTED_SLM);
	if(NULL != slm)
	{
		slm->PreflightPosition();
		ret = slm->SetupPosition();
		if(TRUE == doStart)
			ret = slm->StartPosition();
	}

	return ret;
}

DllExportLiveImage SaveSLMPattern(const wchar_t* bmpPatternName)
{
	long ret = TRUE;
	SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_FUNC_MODE,IDevice::SLMFunctionMode::SAVE_PHASE, IDevice::DeviceSetParamType::NO_EXECUTION);
	ret = SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_ARRAY_ID,0,IDevice::DeviceSetParamType::NO_EXECUTION);
	ret = SetDeviceParamString(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_BMP_FILENAME,(wchar_t*)bmpPatternName,IDevice::DeviceSetParamType::EXECUTION_WAIT);

	return ret;
}

DllExportLiveImage ResetAffineCalibration()
{
	return SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_RESET_AFFINE,TRUE, IDevice::DeviceSetParamType::NO_EXECUTION);
}

DllExportLiveImage SetSLMBlank()
{
	return SetDeviceParamLong(SelectedHardware::SELECTED_SLM,IDevice::PARAM_SLM_BLANK,TRUE, IDevice::DeviceSetParamType::NO_EXECUTION);
}

DllExportLiveImage StopLiveBleach() 
{
	stopBleach = TRUE;

	while((TRUE == inFileLoading) || (TRUE == activeBleach) || (NULL != hBleachThread))
	{
		Sleep(1);
	}

	PostflightSLMBleachScanner();
	return TRUE;
}
