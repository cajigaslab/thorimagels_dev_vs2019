#include "stdafx.h"
#include "..\..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "RunSample.h"
#include "math.h"
#include "AcquireZStack.h"
#include "AcquireFactory.h"
#include <functional>


long SetDeviceParameterValue(IDevice *pDevice,long paramID, double val,long bWait,HANDLE hEvent,long waitTime);

extern auto_ptr<CommandDll> shwDll;
extern auto_ptr<TiffLibDll> tiffDll;
extern "C" long __declspec(dllimport) GetZRange(double& zMin, double& zMax, double& zDefault);

AcquireZStack::AcquireZStack(IExperiment *exp,wstring path)
{
	_pExp = exp;
	_pCamera = NULL;
	_counter = 0;
	_tFrame = 1;
	_path = path;
}


UINT StatusZThreadProc3( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;

	IDevice * pDevice = (IDevice*)pParam;

	while((status == IDevice::STATUS_BUSY) && (FALSE == AcquireZStack::_stopZCapture))
	{
		if(FALSE == pDevice->StatusPosition(status))
		{
			break;
		}
	}

	SetEvent( AcquireZStack::hEventZ);

	return 0;
}


HANDLE AcquireZStack::hEvent = NULL;
HANDLE AcquireZStack::hEventZ = NULL;
BOOL AcquireZStack::_evenOdd = FALSE;
double AcquireZStack::_lastGoodFocusPosition = 0.0;
long AcquireZStack::_stopZCapture = FALSE;

long AcquireZStack::CallCaptureComplete(long captureComplete)
{
	return TRUE;
}

long AcquireZStack::CallCaptureImage(long index)
{
	CaptureImage(index);
	return TRUE;
}

long AcquireZStack::CallSaveImage(long index, BOOL isImageUpdate)
{
	SaveImage(index, isImageUpdate);
	return TRUE;
}

long AcquireZStack::CallCaptureSubImage(long index)
{
	CaptureSubImage(index);
	return TRUE;
}

long AcquireZStack::CallSaveSubImage(long index)
{
	SaveSubImage(index);
	return TRUE;
}

long AcquireZStack::CallSaveZImage(long index, double power0, double power1, double power2, double power3,double power4, double power5)
{
	SaveZImage(index, power0, power1, power2, power3,power4, power5);
	return TRUE;
}

long AcquireZStack::CallSaveTImage(long index)
{
	SaveTImage(index);
	return TRUE;
}

long AcquireZStack::CallSequenceStepCurrent(long index)
{
	SequenceStepCurrent(index);
	return TRUE;
}

long AcquireZStack::PreCaptureEventCheck(long &status)
{
	return TRUE;
}

long AcquireZStack::StopCaptureEventCheck(long &status)
{
	StopCapture(status);
	return TRUE;
}

long AcquireZStack::CallStartProgressBar(long index, long resetTotalCount)
{
	StartProgressBar(index, resetTotalCount);
	return TRUE;
}

long AcquireZStack::Execute(long index, long subWell, long zFrame, long tFrame)
{
	_tFrame = tFrame;
	return Execute(index, subWell);
}

long AcquireZStack::Execute(long index, long subWell)
{
	long ret = TRUE;

	double magnification;
	string objName;
	_pExp->GetMagnification(magnification,objName);
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
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample RunAutofocus failed");
		return FALSE;
	}

	_pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);

	if(NULL == _pCamera)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create camera");
		return FALSE;
	}

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

	long areaMode,scanMode,interleave,pixelX,pixelY,channel,fieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles,polarity[4], verticalFlip, horizontalFlip;
	double areaAngle,dwellTime, crsFrequencyHz = 0;
	long timeBasedLineScan = FALSE;
	long timeBasedLineScanMS = 0;
	long threePhotonEnable = FALSE;
	long numberOfPlanes = 1;
	_pExp->GetLSM(areaMode,areaAngle,scanMode,interleave,pixelX,pixelY,channel,fieldSize,offsetX,offsetY,averageMode,averageNum,clockSource, inputRange1, inputRange2, twoWayAlignment,extClockRate,dwellTime,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles,polarity[0],polarity[1],polarity[2],polarity[3], verticalFlip, horizontalFlip, crsFrequencyHz, timeBasedLineScan, timeBasedLineScanMS, threePhotonEnable, numberOfPlanes);

	ICamera::CameraType cameraType;

	double val;
	_pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,val);

	cameraType = (ICamera::CameraType)static_cast<long> (val);

	AcquireFactory factory;

	auto_ptr<IAcquire> acqZFrame(NULL);

	switch(cameraType)
	{
	case ICamera::CCD:
		{				
			//if(_pExp->GetNumberOfWavelengths() > 1)
			//{
			//	acqZFrame.reset(factory.getAcquireInstance(AcquireFactory::ACQ_MULTI_WAVELENGTH,afNone.get(),NULL,_pExp,_path));
			//}
			//else
			//{
			acqZFrame.reset(factory.getAcquireInstance(AcquireFactory::ACQ_SINGLE,NULL,_pExp,_path));
			//}

		}
		break;
	case ICamera::LSM:
		{			
			acqZFrame.reset(factory.getAcquireInstance(AcquireFactory::ACQ_SINGLE,NULL,_pExp,_path));
		}
		break;
	}

	long wavelengthIndex = 0, timePoints,triggerModeTimelapse;
	string wavelengthName, zstageName;
	double exposureTimeMS, intervalSec;
	_pExp->GetWavelength(wavelengthIndex,wavelengthName,exposureTimeMS);

	_pExp->GetTimelapse(timePoints,intervalSec,triggerModeTimelapse);
	switch (areaMode)
	{
	case 0: //Square
		{
			areaMode = ICamera::SQUARE;
			_pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,TRUE);
		}
		break;
	case 1: //Rect
		{
			areaMode = ICamera::RECTANGLE;
			_pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,TRUE);
		}
		break;
	case 2: //Kymopgragh
		{
			areaMode = ICamera::RECTANGLE;
			_pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,FALSE);
		}
		break;
	case 3: //LineScan
		{
			areaMode = ICamera::RECTANGLE;
			_pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,FALSE);
		}
		break;
	case 4: //Polyline
		{
			areaMode = ICamera::POLYLINE;
			_pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,FALSE);
		}
		break;
	default:
		{
			areaMode = ICamera::SQUARE;
			_pCamera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE,TRUE);
		}
		break;
	}

	_pCamera->SetParam(ICamera::PARAM_LSM_AREAMODE,areaMode);
	_pCamera->SetParam(ICamera::PARAM_LSM_SCANAREA_ANGLE,areaAngle);
	_pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X,pixelX);
	_pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y,pixelY);
	_pCamera->SetParam(ICamera::PARAM_LSM_DWELL_TIME,dwellTime);
	_pCamera->SetParam(ICamera::PARAM_BINNING_X,binningX);
	_pCamera->SetParam(ICamera::PARAM_BINNING_Y,binningY);
	_pCamera->SetParam(ICamera::PARAM_GAIN,gain);
	_pCamera->SetParam(ICamera::PARAM_LIGHT_MODE,lightMode);
	_pCamera->SetParam(ICamera::PARAM_EXPOSURE_TIME_MS,exposureTimeMS);	
	_pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_LEFT,left);
	_pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_TOP,top);
	_pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT,top);
	_pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM,bottom);

	_pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,scanMode);
	_pCamera->SetParam(ICamera::PARAM_LSM_INTERLEAVE_SCAN,interleave);
	_pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X,pixelX);
	_pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y,pixelY);
	_pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL,channel);
	_pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE,fieldSize);
	_pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_X,offsetX);
	_pCamera->SetParam(ICamera::PARAM_LSM_OFFSET_Y,offsetY);
	_pCamera->SetParam(ICamera::PARAM_LSM_AVERAGEMODE,averageMode);
	_pCamera->SetParam(ICamera::PARAM_LSM_AVERAGENUM,averageNum);
	_pCamera->SetParam(ICamera::PARAM_LSM_CLOCKSOURCE,clockSource);
	_pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE1,inputRange1);
	_pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE2,inputRange2);
	_pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE3,inputRange3);
	_pCamera->SetParam(ICamera::PARAM_LSM_INPUTRANGE4,inputRange4);
	_pCamera->SetParam(ICamera::PARAM_LSM_MINIMIZE_FLYBACK_CYCLES,minimizeFlybackCycles);
	_pCamera->SetParam(ICamera::PARAM_LSM_ALIGNMENT,twoWayAlignment);
	_pCamera->SetParam(ICamera::PARAM_LSM_EXTERNALCLOCKRATE,extClockRate);
	_pCamera->SetParam(ICamera::PARAM_LSM_FLYBACK_CYCLE,flybackCycles);
	_pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_SINGLE_FRAME);
	_pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF, FALSE);

	_pCamera->SetParam(ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION, verticalFlip);
	_pCamera->SetParam(ICamera::PARAM_LSM_HORIZONTAL_FLIP, horizontalFlip);
	_pCamera->SetParam(ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN, timeBasedLineScan);
	_pCamera->SetParam(ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS, timeBasedLineScanMS);

	_pCamera->SetParam(ICamera::PARAM_LSM_3P_ENABLE, threePhotonEnable);

	//notify the ECU of the zoom change also
	if(SetDeviceParamLong(SelectedHardware::SELECTED_CONTROLUNIT,IDevice::PARAM_SCANNER_ZOOM_POS,fieldSize,FALSE))
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

	SetPMT();
	ScannerEnable(TRUE);

	//assign the power device
	_pPower = GetDevice(SelectedHardware::SELECTED_POWERREGULATOR);

	OpenShutter();

	_pCamera->PreflightAcquisition(NULL);

	ret = CaptureZStack(index, subWell, acqZFrame);

	_pCamera->PostflightAcquisition(NULL);

	CloseShutter();

	return ret;

}

class DynamicRoutine
{
public:

	DynamicRoutine()
	{
	}

	static UINT StatusThreadProc( LPVOID pParam )
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

		SetEvent(hEvent);

		return 0;
	};

	static HANDLE hEvent;

};

HANDLE DynamicRoutine::hEvent = NULL;


long SetDevicePosition(IDevice *pDevice,long paramID,double pos,BOOL bWait)
{
	if(NULL == pDevice)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"SetDevicePosition-pDevice is NULL");
		return FALSE;
	}

	pDevice->SetParam(paramID, pos);
	StringCbPrintfW(message,MSG_LENGTH,L"ThorImager SetDevicePosition %d.%d",(int)pos,(int)((pos - static_cast<long>(pos))*1000));

	pDevice->PreflightPosition();

	pDevice->SetupPosition ();

	pDevice->StartPosition();

	if(TRUE == bWait)
	{
		auto_ptr<DynamicRoutine> dr(new DynamicRoutine);

		//don't wait for the z to finish its motion will overlap with the next XY movement
		dr->hEvent = CreateEvent(0, FALSE, FALSE, 0);

		DWORD dwThread;

		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) dr->StatusThreadProc, pDevice, 0, &dwThread );

		const long MAX_DEVICE_WAIT_TIME = 5000;

		DWORD dwWait = WaitForSingleObject( dr->hEvent, MAX_DEVICE_WAIT_TIME);

		if(dwWait != WAIT_OBJECT_0)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"SetDevicePosition WaitForSingleObject failed- timeout");
			//return FALSE;
		}		

		CloseHandle(hThread);
		CloseHandle(dr->hEvent);
	}

	pDevice->PostflightPosition();	

	return TRUE;

}

long AcquireZStack::CaptureZStack(long index, long subWell, auto_ptr<IAcquire>& acqZFrame)
{
	long ret = TRUE;
	double zStartPos, zStopPos, zTiltPos, zStepSizeMM;
	double zMin, zMax, zDefault;
	long zstageSteps, zStreamFrames, zStreamMode;
	GetZPositions(_pExp, NULL, zStartPos, zStopPos, zTiltPos, zStepSizeMM, zstageSteps, zStreamFrames, zStreamMode);

	double pos = zStartPos;
	vector<double> posList;

	// check if the read from file mode is enabled
	int zFileEnable;
	double zFilePosScale;
	_pExp->GetZFileInfo(zFileEnable, zFilePosScale);

	// create list of positions
	if (TRUE == zFileEnable)
	{
		_pExp->GetZPosList(posList);
		// multiply the whole vector by zFilePosScale
		std::transform(posList.begin(), posList.end(), posList.begin(),
			std::bind(std::multiplies<double>(), std::placeholders::_1, zFilePosScale));
		// make sure that the list of positions has the correct size
		if (zstageSteps != posList.size())
		{
			logDll->TLTraceEvent(WARNING_EVENT, 1, L"AcquireZStack Mismatched position list size");
			return FALSE;
		}
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"AcquireZStack Executed with posList read from file");
	}
	else
	{
		for (long z = 0; z < zstageSteps; z++) posList.push_back(pos + z * zStepSizeMM);
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"AcquireZStack Executed with uniform steps");
	}

	// check to make sure the z-positions are within range
	GetZRange(zMin, zMax, zDefault);
	for (long z = 0; z < zstageSteps; z++)
	{
		if (posList[z]<zMin || posList[z] > zMax)
		{
			logDll->TLTraceEvent(WARNING_EVENT, 1, L"AcquireZStack Requested position outside allowed z-range");
			MessageBox(NULL, L"At least one of the positions is out of range.", L"", MB_OK);
			return FALSE;
		}
	}

	//Reset _stopZCapture flag each time experiment starts 
	_stopZCapture = FALSE;
	double power0 = 0, power1 = 0, power2 = 0, power3 = 0, power4 = 0, power5 = 0;
	double ledPower1 = 0, ledPower2 = 0, ledPower3 = 0, ledPower4 = 0, ledPower5 = 0, ledPower6 = 0;
	//step through z and capture
	for (long z = 1; z <= zstageSteps; z++)
	{
		if (TRUE == zFileEnable)
		{
			pos = posList[(long long)z - 1];
		}
		if (FALSE == SetZPosition(pos, TRUE))
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"AcquireZStack Execute SetZPosition failed ");
			break;
		}

		SetPower(_pExp, _pCamera, pos, power0, power1, power2, power3, power4, power5);

		SetLEDs(_pExp, _pCamera, pos, ledPower1, ledPower2, ledPower3, ledPower4, ledPower5, ledPower6);

		if (FALSE == acqZFrame->Execute(index, subWell, z, 1))
		{
			ret = FALSE;
			break;
		}

		if (TRUE != zFileEnable)
		{
			pos += zStepSizeMM;
		}

		//update progress Z to observer.
		CallSaveZImage(z, power0, power1, power2, power3, power4, power5);

		//update progress to observer.
		CallSaveImage(index + z-1, TRUE);

		StringCbPrintfW(message,MSG_LENGTH,L"AcquireZStack Z position %d",(int)pos,(int)((pos - static_cast<long>(pos))*1000));
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
	}

	//update progress to observer.
	//This will complete the progress for the Z stack
	CallSaveImage(zstageSteps, FALSE);

	//return to the start power and position 
	SetPower(_pExp, _pCamera, zStartPos, power0, power1, power2, power3, power4, power5);

	SetLEDs(_pExp, _pCamera, zStartPos, ledPower1, ledPower2, ledPower3, ledPower4, ledPower5, ledPower6);

	//return to the start position if not already stopped by user
	if(FALSE == _stopZCapture)
	{
		if(FALSE == SetZPosition(zStartPos,TRUE))
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireZStack Execute SetZPosition failed ");
		}
	}
	else
	{
		StopZ();
	}
	return ret;
}

long AcquireZStack::ScannerEnable(long enable)
{
	return ScannerEnableProc(0,enable);
}

long AcquireZStack::SetPMT()
{
	return SetPMTProc(_pExp);
}

long AcquireZStack::StopZ()
{	
	long ret = TRUE;

	IDevice * pZStage = NULL;



	pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

	if(NULL == pZStage)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireZStack Execute could not create z stage");
		return FALSE;
	}

	pZStage->SetParam(IDevice::PARAM_Z_STOP, NULL);

	pZStage->PreflightPosition();

	pZStage->SetupPosition ();

	pZStage->StartPosition();
	pZStage->PostflightPosition();

	return ret;
}

long AcquireZStack::SetZPosition(double pos,BOOL bWait)
{
	long ret = TRUE;

	IDevice * pZStage = NULL;

	pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

	if(NULL == pZStage)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireZStack Execute could not create z stage");
		return FALSE;
	}

	pZStage->SetParam(IDevice::PARAM_Z_POS, pos);

	pZStage->PreflightPosition();

	pZStage->SetupPosition ();

	pZStage->StartPosition();

	if(TRUE == bWait)
	{
		//don't wait for the z to finish its motion will overlap with the next XY movement
		hEventZ = CreateEvent(0, FALSE, FALSE, 0);

		DWORD dwThread;

		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusZThreadProc3, pZStage, 0, &dwThread );

		const long MAX_Z_WAIT_TIME = 80000;

		DWORD startTime = GetTickCount();

		DWORD dwWait = WAIT_TIMEOUT;

		while((WAIT_OBJECT_0 != dwWait)&&((GetTickCount()-startTime)<MAX_Z_WAIT_TIME))
		{
			long status;
			StopCapture(status);

			if(1 == status)
			{
				_stopZCapture = TRUE;
				break;
			}

			dwWait = WaitForSingleObject( hEventZ, 10);
		}

		if(dwWait != WAIT_OBJECT_0)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireZStack Execute Z failed");

			WaitForSingleObject(hEventZ, INFINITE);
			//stop the z stepper if stop requested by user
			pZStage->SetParam(IDevice::PARAM_Z_STOP, 1);
			pZStage->PreflightPosition();
			pZStage->SetupPosition ();
			pZStage->StartPosition();
			ret = FALSE;
		}		

		CloseHandle(hThread);
		CloseHandle(hEventZ);
	}
	pZStage->PostflightPosition();	

	return ret;

}