#include "stdafx.h"
#include "..\..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "RunSample.h"
#include "math.h"
#include "AutoFocus.h"
#include "AutoFocusNone.h"
#include "AcquireTSeries.h"
#include "AcquireFactory.h"
#include "ImageCorrection.h"
#include <functional>

extern auto_ptr<CommandDll> shwDll;
extern auto_ptr<TiffLibDll> tiffDll;
extern "C" long __declspec(dllimport) GetZRange(double& zMin, double& zMax, double& zDefault);


int Call_TiffVSetField(TIFF* out, uint32 ttag_t, ...);
long SaveTIFF(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut, double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string *acquiredDate,double dt, string * omeTiffData, PhysicalSize physicalSize, long doCompression);
long SaveTIFFWithoutOME(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut, double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string *acquiredDate,double dt, long doCompression);

long SaveJPEG(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height,unsigned short * rlut, unsigned short * glut, unsigned short * blut,long bitDepth);
void GetColorInfo(HardwareSetupXML *pHardware,string wavelengthName, long &red, long &green, long &blue,long &bp, long &wp);
void GetLookUpTables(unsigned short * rlut, unsigned short * glut, unsigned short *blut,long red, long green, long blue, long bp, long wp, long bitdepth);
long SetupDimensions(ICamera *pCamera,IExperiment *pExperiment,double fieldSizeCalibration, double magnification, Dimensions &d, long &avgFrames, long &bufferChannels, long &avgMode, double &umPerPixel, long& numOfPlanes);
long SetDeviceParameterValue(IDevice *pDevice,long paramID, double val,long bWait,HANDLE hEvent,long waitTime);
extern string ConvertWStringToString(wstring ws);
AcquireTSeries::AcquireTSeries(IAutoFocus * pAF,IExperiment *exp,wstring path)
{
	_pAutoFocus = pAF;
	_pExp = exp;
	_pCamera = NULL;
	_counter = 0;
	_tFrame = 1;
	_path = path;
}

UINT StatusZThreadProc4( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;

	IDevice * pDevice = (IDevice*)pParam;

	while(status == IDevice::STATUS_BUSY)
	{
		if((FALSE == pDevice->StatusPosition(status)) && (FALSE == AcquireTSeries::_stopTCapture))
		{
			break;
		}
	}

	SetEvent( AcquireTSeries::hEventZ);

	return 0;
}

long ParseApplicationSettingsXMLATSE(const char* pcFilename)	// get OMETiffTag Enable
{
	long ret = FALSE;

	// load the ApplicationSettings.xml 
	ticpp::Document doc(pcFilename);
	doc.LoadFile();

	// parse through all children
	ticpp::Iterator<ticpp::Element> child;
	for(child = child.begin(doc.FirstChildElement()); child != child.end(); child++)
	{
		std::string strName;
		std::string strValue;
		child->GetValue(&strName);

		if ("OMETIFFTag" == strName)
		{
			// now parse through all the attributes of this fruit
			ticpp::Iterator< ticpp::Attribute > attribute;
			for(attribute = attribute.begin(child.Get()); attribute != attribute.end(); attribute++)
			{
				attribute->GetName(&strName);
				attribute->GetValue(&strValue);
				if ("1" == strValue)
				{
					ret = TRUE;
				}				
			}

			return ret;
		}
	}

	return ret;
}

string WStringToStringATSE(wstring ws)	//ATS: AcquireTStream
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

long GetOMETIFFTagEnableFlagATSE()	
{
	wchar_t fileName[MAX_PATH];
	wstring tempPath = ResourceManager::getInstance()->GetApplicationSettingsPath();
	tempPath += wstring(L"ApplicationSettings.xml");
	StringCbPrintfW(fileName,_MAX_PATH, tempPath.c_str());

	long ret = ParseApplicationSettingsXMLATSE(WStringToStringATSE(fileName).c_str());

	return ret;	
}

HANDLE AcquireTSeries::hEvent = NULL;
HANDLE AcquireTSeries::hEventAutoFocus = NULL;
HANDLE AcquireTSeries::hEventZ = NULL;
BOOL AcquireTSeries::_evenOdd = FALSE;
double AcquireTSeries::_lastGoodFocusPosition = 0.0;
long AcquireTSeries::_stopTCapture = FALSE;

long AcquireTSeries::CallCaptureComplete(long captureComplete)
{
	return TRUE;
}

long AcquireTSeries::CallCaptureImage(long index)
{
	CaptureImage(index);
	return TRUE;
}

long AcquireTSeries::CallSaveImage(long index, BOOL isImageUpdate)
{
	SaveImage(index, isImageUpdate);
	return TRUE;
}

long AcquireTSeries::CallCaptureSubImage(long index)
{
	CaptureSubImage(index);
	return TRUE;
}

long AcquireTSeries::CallSaveSubImage(long index)
{
	SaveSubImage(index);
	return TRUE;
}

long AcquireTSeries::CallSaveZImage(long index, double power0, double power1, double power2, double power3, double power4, double power5)
{
	SaveZImage(index, power0, power1, power2, power3, power4, power5);
	return TRUE;
}

long AcquireTSeries::CallSaveTImage(long index)
{
	SaveTImage(index);
	return TRUE;
}

long AcquireTSeries::CallSequenceStepCurrent(long index)
{
	SequenceStepCurrent(index);
	return TRUE;
}

long AcquireTSeries::PreCaptureEventCheck(long &status)
{
	return TRUE;
}

long AcquireTSeries::StopCaptureEventCheck(long &status)
{
	StopCapture(status);
	return TRUE;
}

long AcquireTSeries::CallStartProgressBar(long index, long resetTotalCount)
{
	StartProgressBar(index, resetTotalCount);
	return TRUE;
}

long AcquireTSeries::Execute(long index, long subWell, long zFrame, long tFrame)
{
	_tFrame = tFrame;
	ResourceManager::getInstance()->LoadSettings();
	return Execute(index, subWell);
}

long AcquireTSeries::Execute(long index, long subWell)
{	
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

	pHardware->GetMagInfoFromName(objName,magnification,position,numAperture,afStartPos,afFocusOffset,afAdaptiveOffset,beamExpPos,beamExpWavelength,beamExpPos2,beamExpWavelength2,turretPosition,zAxisToEscape,zAxisEscapeDistance);

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

	IDevice * pAutoFocusDevice = NULL;

	pAutoFocusDevice = GetDevice(SelectedHardware::SELECTED_AUTOFOCUS);

	if(NULL == pAutoFocusDevice)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create autofocus device");
		return FALSE;
	}
	BOOL afFound = FALSE;

	if(aftype != IAutoFocus::AF_NONE)
	{
		if (FALSE == AutoFocusAndRetry(index, pAutoFocusDevice, afFound))
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample AutoFocusAndRetry failed");
			return FALSE;
		}
	}	

	_pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);

	if(NULL == _pCamera)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create camera");
		return FALSE;
	}

	auto_ptr<AutoFocusNone> afNone(new AutoFocusNone());

	AcquireFactory factory;	

	auto_ptr<IAcquire> acqZFrame(NULL);

	ICamera::CameraType cameraType;
	ICamera::LSMType lsmType;
	double val;
	_pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,val);
	cameraType = (ICamera::CameraType)static_cast<long> (val);
	_pCamera->GetParam(ICamera::PARAM_LSM_TYPE,val);
	lsmType = (ICamera::LSMType)static_cast<long> (val);
	switch(cameraType)
	{
	case ICamera::CCD:
		{				
			if(_pExp->GetNumberOfWavelengths() > 1)
			{
				acqZFrame.reset(factory.getAcquireInstance(AcquireFactory::ACQ_MULTI_WAVELENGTH,afNone.get(),NULL,_pExp,_path));
			}
			else
			{
				acqZFrame.reset(factory.getAcquireInstance(AcquireFactory::ACQ_SINGLE,afNone.get(),NULL,_pExp,_path));
			}

		}
		break;
	case ICamera::LSM:
		{			
			//if (zStreamFrames <= 1)
			//{
			//	
			//}
			//else
			//{
			//	// do nothing leave acqZFrame as NULL
			//}
			acqZFrame.reset(factory.getAcquireInstance(AcquireFactory::ACQ_SINGLE,afNone.get(),NULL,_pExp,_path));
		}
		break;		
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

		camAverageMode = (1 == camAverageMode) ? camAverageNum : 1;
		_pCamera->SetParam(ICamera::PARAM_EXPOSURE_TIME_MS,camExposureTimeMS);
		_pCamera->SetParam(ICamera::PARAM_GAIN,gain);
		_pCamera->SetParam(ICamera::PARAM_OPTICAL_BLACK_LEVEL,blackLevel);
		_pCamera->SetParam(ICamera::PARAM_LIGHT_MODE,lightMode);	
		_pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_LEFT,left);
		_pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_TOP,top);
		_pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT,right);
		_pCamera->SetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM,bottom);
		_pCamera->SetParam(ICamera::PARAM_BINNING_X,binningX);
		_pCamera->SetParam(ICamera::PARAM_BINNING_Y,binningY);
		_pCamera->SetParam(ICamera::PARAM_TAP_INDEX,tapsIndex);
		_pCamera->SetParam(ICamera::PARAM_TAP_BALANCE_MODE,tapsBalance);
		_pCamera->SetParam(ICamera::PARAM_READOUT_SPEED_INDEX,readoutSpeedIndex);
		_pCamera->SetParam(ICamera::PARAM_CAMERA_AVERAGENUM,camAverageNum);
		_pCamera->SetParam(ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP,camVericalFlip);
		_pCamera->SetParam(ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP,camHorizontalFlip);
		_pCamera->SetParam(ICamera::PARAM_CAMERA_IMAGE_ANGLE,imageAngle);
		_pCamera->SetParam(ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT, camAverageNum);
		//will average after acquisition is complete. Set to none at camera level
		_pCamera->SetParam(ICamera::PARAM_CAMERA_AVERAGEMODE,ICamera::AVG_MODE_NONE);

		_lsmChannel = 1;
	}
	else
	{
		wstring pathAndName = _pExp->GetPathAndName();
		_pCamera->SetParamString(ICamera::PARAM_MESO_EXP_PATH, (wchar_t*)pathAndName.c_str());
		_pCamera->SetParamString(ICamera::PARAM_WAVEFORM_OUTPATH, (wchar_t*)GetDir(pathAndName).c_str());

		long areaMode,scanMode,interleave,pixelX,pixelY,chan,lsmFieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles,polarity[4], verticalFlip, horizontalFlip;
		double areaAngle,dwellTime, crsFrequencyHz = 0;
		long timeBasedLineScan = FALSE;
		long timeBasedLineScanMS = 0;
		long threePhotonEnable = FALSE;
		long numberOfPlanes = 1;
		//getting the values from the experiment setup XML files
		_pExp->GetLSM(areaMode,areaAngle,scanMode,interleave,pixelX,pixelY,chan,lsmFieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,dwellTime,
			flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles,polarity[0],polarity[1],polarity[2],polarity[3], verticalFlip, horizontalFlip, crsFrequencyHz, timeBasedLineScan, timeBasedLineScanMS, threePhotonEnable, numberOfPlanes);
		_lsmChannel = chan;
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

		averageNum = (1 == averageMode) ? averageNum : 1;
		_pCamera->SetParam(ICamera::PARAM_LSM_AREAMODE,areaMode);
		_pCamera->SetParam(ICamera::PARAM_LSM_SCANAREA_ANGLE,areaAngle);
		_pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X,pixelX);
		_pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y,pixelY);
		_pCamera->SetParam(ICamera::PARAM_LSM_DWELL_TIME,dwellTime);
		_pCamera->SetParam(ICamera::PARAM_LSM_SCANMODE,scanMode);
		_pCamera->SetParam(ICamera::PARAM_LSM_INTERLEAVE_SCAN,interleave);
		_pCamera->SetParam(ICamera::PARAM_LSM_CHANNEL,chan);
		_pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE,lsmFieldSize);
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
		_pCamera->SetParam(ICamera::PARAM_LSM_DMA_BUFFER_COUNT,averageNum);
		_pCamera->SetParam(ICamera::PARAM_LSM_FLYBACK_CYCLE,flybackCycles);
		//will average after acquisition is complete. Set to none at camera level
		_pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_SINGLE_FRAME);
		_pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF, FALSE);

		_pCamera->SetParam(ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION, verticalFlip);
		_pCamera->SetParam(ICamera::PARAM_LSM_HORIZONTAL_FLIP, horizontalFlip);
		_pCamera->SetParam(ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN, timeBasedLineScan);
		_pCamera->SetParam(ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS, timeBasedLineScanMS);

		_pCamera->SetParam(ICamera::PARAM_LSM_3P_ENABLE, threePhotonEnable);
		_pCamera->SetParam(ICamera::PARAM_LSM_NUMBER_OF_PLANES, numberOfPlanes);
		
		//notify the ECU of the zoom change also
		IDevice *pControlUnitDevice = NULL;
		pControlUnitDevice = GetDevice(SelectedHardware::SELECTED_CONTROLUNIT);
		if(NULL != pControlUnitDevice && ICamera::LSMType::RESONANCE_GALVO_GALVO != lsmType)
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

		SetPMT();
		ScannerEnable(TRUE);
	}

	return CaptureTSeries(index, subWell,  acqZFrame, _pCamera);
}

long AcquireTSeries::CaptureTSeries(long index, long subWell, auto_ptr<IAcquire> &acqZFrame, ICamera *pCamera)
{
	//Reset _stopTCapture flag each time experiment starts 
	_stopTCapture = FALSE;

	double zStartPos, zStopPos, zTiltPos, zStepSizeMM;
	double zMin, zMax, zDefault;
	long zstageSteps, zStreamFrames, zStreamMode;
	GetZPositions(_pExp, NULL, zStartPos, zStopPos, zTiltPos, zStepSizeMM, zstageSteps, zStreamFrames, zStreamMode);

	double intervalSec;
	long timePoints,triggerModeTimelapse;
	_pExp->GetTimelapse(timePoints,intervalSec,triggerModeTimelapse);

	//Operating modes for camera control
	const long NORMAL_OPERATING_MODE = 0;
	const long BULB_OPERATING_MODE = 1;

	//default to normal mode for the cameras
	pCamera->SetParam(ICamera::PARAM_OP_MODE, NORMAL_OPERATING_MODE);
	pCamera->SetParam(ICamera::PARAM_MULTI_FRAME_COUNT, 1);


	const long TIMLAPSE_TRIG_MODE_NONE = 0;
	const long TIMLAPSE_TRIG_MODE_TRIG_FIRST = 1;
	const long TIMLAPSE_TRIG_MODE_TRIG_EACH = 2;
	const long TIMLAPSE_TRIG_MODE_TRIG_BULB = 3;

	//Setup the 'read from file' ZStack acquisition
	vector<double> posList;
	// check if the read from file mode is enabled
	int zFileEnable = FALSE;
	double zFilePosScale = 1.0;
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
			logDll->TLTraceEvent(WARNING_EVENT, 1, L"AcquireTSeries Mismatched position list size");
			return FALSE;
		}
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"AcquireTSeries Executed with posList read from file");

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
	}

	switch(triggerModeTimelapse)
	{
	case TIMLAPSE_TRIG_MODE_NONE://none
		{
			pCamera->SetParam(ICamera::PARAM_OP_MODE, NORMAL_OPERATING_MODE);
			pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_SINGLE_FRAME);
		}
		break;
	case TIMLAPSE_TRIG_MODE_TRIG_FIRST://trigger first			
	case TIMLAPSE_TRIG_MODE_TRIG_EACH://trigger each
		{
			pCamera->SetParam(ICamera::PARAM_OP_MODE, NORMAL_OPERATING_MODE);
			pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::HW_MULTI_FRAME_TRIGGER_FIRST);	
		}
		break;
	case TIMLAPSE_TRIG_MODE_TRIG_BULB://trigger each bulb mode
		{
			pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_SINGLE_FRAME);
			pCamera->SetParam(ICamera::PARAM_OP_MODE, BULB_OPERATING_MODE);
		}
		break;
	}

	double power0 = 0, power1 =0, power2 = 0, power3 = 0, power4 = 0, power5 = 0;
	double ledPower1 = 0, ledPower2 = 0, ledPower3 = 0, ledPower4 = 0, ledPower5 = 0, ledPower6 = 0;
	CallStartProgressBar(index);

	//The timepoint location can be manipulated at runtime. Do
	//not rely solely upon the timlapse tag. Instead use the 
	//TimelapseTOffset value
	//Now the timeLapse is handled from the higher level,
	//Here we only capture one time point at a time
	long tOffset = 0;	
	_pExp->GetTimelapseTOffset(tOffset);
	for(long t=1;t<=1;t++)
	{
		pCamera->PreflightAcquisition(NULL);

		long z=1;
		long updateIndex = 0;
		if(zstageSteps > 1)	// looping through all Z positions
		{
			double pos = zStartPos;

			OpenShutter();

			//step through z and capture
			for(z=1;z<=zstageSteps;z++)
			{
				switch(triggerModeTimelapse)
				{ 
				case TIMLAPSE_TRIG_MODE_TRIG_FIRST:
					{
						//if operating in trigger first mode switch back to software trigger 
						//after the first frame of the first subwell/tile and Z 
						if(2 <= z || 2 <= (t + tOffset) || 2 <= subWell)
						{
							pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_SINGLE_FRAME);
						}
					}
					break;
				case TIMLAPSE_TRIG_MODE_TRIG_EACH:
					{
						//if operating in trigger each mode. switch back to software trigger after the first frame
						//and switch into HW_MULTI_FRAME_TRIGGER_FIRST for the first plane of each z stack of the first tile
						if(1 == z && 1 == subWell)
						{		
							pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::HW_MULTI_FRAME_TRIGGER_FIRST);
						}
						else
						{	
							pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_SINGLE_FRAME);
						}
					}
					break;
				}

				//update progress to observer.
				updateIndex = index + (z-1) + (zstageSteps)*(t+tOffset-1);

				//Set the position to the next position of the list if z position read from file is enabled
				if (TRUE == zFileEnable)
				{
					pos = posList[(long long)z - 1];
				}

				if(FALSE == SetZPosition(pos,TRUE,FALSE))
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireTSeries Execute SetZPosition failed ");
					break;
				}

				SetPower(_pExp, pCamera, pos, power0, power1, power2, power3, power4, power5);

				SetLEDs(_pExp, pCamera, pos, ledPower1, ledPower2, ledPower3, ledPower4, ledPower5, ledPower6);

				if (zStreamMode == 0)	// if z stream is not enabled, call Execute() (old logic) anyway 
				{
					if (FALSE == acqZFrame->Execute(index,subWell,z,t+tOffset))
					{
						CloseShutter();
						StringCbPrintfW(message,MSG_LENGTH,L"AcquireTSeries Execute acqZFrame z = %d t = %d failed",(int)z,(int)t);
						logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
						//Ensure all tasks are stopped in the camera before returning
						pCamera->PostflightAcquisition(NULL);
						return FALSE;
					}
				}
				else
				{
					if (zStreamFrames > 1)	// if z stream is enable and number of z stream frames > 1, use zStreamExecute() the new logic 
					{
						if (FALSE == ZStreamExecute(index, subWell, pCamera, z,t+tOffset, 1))
						{
							CloseShutter();
							StringCbPrintfW(message,MSG_LENGTH,L"AcquireTSeries ZStreamExecute z = %d t = %d failed",(int)z,(int)t);
							logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
							//Ensure all tasks are stopped in the camera before returning
							pCamera->PostflightAcquisition(NULL);
							return FALSE;
						}
					}
					else	// if z stream is enabled, but number of the z stream frame = 1, use old logic
					{
						if(FALSE == acqZFrame->Execute(index,subWell,z,t+tOffset))
						{
							CloseShutter();
							StringCbPrintfW(message,MSG_LENGTH,L"AcquireTSeries Execute acqZFrame z = %d t = %d failed",(int)z,(int)t);
							logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
							//Ensure all tasks are stopped in the camera before returning
							pCamera->PostflightAcquisition(NULL);
							return FALSE;
						}
					}
				}

				if (TRUE != zFileEnable)
				{
					pos += zStepSizeMM;
				}

				//update progress to observer.
				CallSaveImage(updateIndex, TRUE);			

				//update progress T to observer.
				CallSaveTImage(t+tOffset);

				//update progress Z to observer.
				CallSaveZImage(z, power0, power1, power2, power3, power4, power5);

				StringCbPrintfW(message,MSG_LENGTH,L"AcquireTSeries Z position %d",(int)pos,(int)((pos - static_cast<long>(pos))*1000));
				logDll->TLTraceEvent(INFORMATION_EVENT,1,message);

				long status = FALSE;
				StopCapture(status);

				if(status == TRUE)
				{
					CloseShutter();
					pCamera->PostflightAcquisition(NULL);
					return TRUE;
				}
			}

			CloseShutter();

			//immediately return to the start position between time points if not already stopped by user
			if(FALSE == _stopTCapture)
			{
				if(FALSE == SetZPosition(zStartPos,TRUE,TRUE))
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireTSeries Execute SetZPosition failed ");
				}
			}
		}
		else
		{
			switch(triggerModeTimelapse)
			{ 
			case TIMLAPSE_TRIG_MODE_TRIG_FIRST:
				{
					//if operating in trigger first mode switch back to software trigger 
					//after the first frame of the first subwell/tile
					if(2 <= (t + tOffset) || 2 <= subWell)
					{
						pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_SINGLE_FRAME);
					}
				}
				break;
			case TIMLAPSE_TRIG_MODE_TRIG_EACH:
				{
					//if operating in trigger each mode. switch back to software trigger after the first frame
					//and switch into HW_MULTI_FRAME_TRIGGER_FIRST for the first tile
					if(1 == subWell)
					{		
						pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::HW_MULTI_FRAME_TRIGGER_FIRST);
					}
					else
					{	
						pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_SINGLE_FRAME);
					}
				}
				break;
			}

			OpenShutter();

			updateIndex = index + t+tOffset-1;

			SetPower(_pExp, pCamera, zStartPos, power0, power1, power2, power3, power4, power5);

			SetLEDs(_pExp, pCamera, zStartPos, ledPower1, ledPower2, ledPower3, ledPower4, ledPower5, ledPower6);

			//don't move the z axis
			if ( (zStreamMode == 1) && (zStreamFrames > 1) )
			{
				if(FALSE == ZStreamExecute(index, subWell, pCamera, z, t+tOffset, 1))
				{
					CloseShutter();
					StringCbPrintfW(message,MSG_LENGTH,L"AcquireTSeries ZStreamExecute z = %d t = %d failed",(int)z,(int)t);
					logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
					//Ensure all tasks are stopped in the camera before returning
					pCamera->PostflightAcquisition(NULL);
					return FALSE;
				}
			}
			else
			{
				if(FALSE == acqZFrame->Execute(index,subWell,1,t+tOffset))
				{
					CloseShutter();
					StringCbPrintfW(message,MSG_LENGTH,L"AcquireTSeries Execute acqZFrame z = %d t = %d failed",(int)1,(int)t);
					logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
					//Ensure all tasks are stopped in the camera before returning
					pCamera->PostflightAcquisition(NULL);
					return FALSE;
				}
			}

			StringCbPrintfW(message,MSG_LENGTH,L"AcquireTSeries acquire frame");
			logDll->TLTraceEvent(INFORMATION_EVENT,1,message);

			CloseShutter();

			//update progress to observer.
			CallSaveImage(updateIndex, TRUE);
		}

		pCamera->PostflightAcquisition(NULL);
		Sleep(10);

		//update progress Z to observer.
		CallSaveZImage((zstageSteps > 1) ? zstageSteps : 1, power0, power1, power2, power3, power4, power5);

		//update progress T to observer.
		CallSaveTImage(t+tOffset);

		long status = FALSE;
		StopCapture(status);

		if(status == TRUE)
		{
			return TRUE;
		}
	}	

	//return to stage start position
	if(zstageSteps > 1)
	{
		if(FALSE == _stopTCapture)
		{
			if(FALSE == SetZPosition(zStartPos,TRUE,TRUE))
			{
				logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireTSeries Execute SetZPosition failed ");
			}
		}
		else
		{
			StopZ();
		}
	}

	return TRUE;
}

long AcquireTSeries::ScannerEnable(long enable)
{
	return ScannerEnableProc(0,enable);
}

long AcquireTSeries::SetPMT()
{
	return SetPMTProc(_pExp);
}

long AcquireTSeries::StopZ()
{
	long ret = TRUE;
	IDevice * pZStage = NULL;

	pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

	if(NULL == pZStage)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute could not create z stage");
		return FALSE;
	}

	pZStage->SetParam(IDevice::PARAM_Z_STOP, NULL);

	pZStage->PreflightPosition();

	pZStage->SetupPosition ();

	pZStage->StartPosition();
	pZStage->PostflightPosition();	
	return ret;
}

long AcquireTSeries::SetZPosition(double pos,BOOL bWait, BOOL bPostflight)
{
	long ret = TRUE;

	IDevice * pZStage = NULL;

	pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

	if(NULL == pZStage)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute could not create z stage");
		return FALSE;
	}

	pZStage->SetParam(IDevice::PARAM_Z_POS, pos);
	StringCbPrintfW(message,MSG_LENGTH,L"ThorImager SetAutoFocusStartZPosition new af start position %d.%d",(int)pos,(int)((pos - static_cast<long>(pos))*1000));

	pZStage->PreflightPosition();

	pZStage->SetupPosition ();

	pZStage->StartPosition();

	if(TRUE == bWait)
	{
		hEventZ = CreateEvent(0, FALSE, FALSE, 0);

		DWORD dwThread;

		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusZThreadProc4, pZStage, 0, &dwThread );

		const long MAX_Z_WAIT_TIME = 80000;

		DWORD startTime = GetTickCount();

		DWORD dwWait = WAIT_TIMEOUT;

		while((WAIT_OBJECT_0 != dwWait)&&((GetTickCount()-startTime)<MAX_Z_WAIT_TIME))
		{
			dwWait = WaitForSingleObject( hEventZ, 10);

			long status;
			StopCapture(status);
			_stopTCapture = status;

			if(1 == status)
			{
				break;
			}
		}

		if(dwWait != WAIT_OBJECT_0)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireZStack Execute Z failed");

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

	if (TRUE == bPostflight)
	{
		pZStage->PostflightPosition();	
	}

	return TRUE;
}

long AcquireTSeries::SetAutoFocusStartZPosition(double afStartPos,BOOL bWait,BOOL afFound)
{
	IDevice * pZStage = NULL;

	pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

	if(NULL == pZStage)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute could not create z stage");
		return FALSE;
	}

	//if found use a relative offset from the current position
	if(afFound)
	{
		double pos;
		pZStage->GetParam(IDevice::PARAM_Z_POS_CURRENT,pos);

		_lastGoodFocusPosition = pos;

		pos -= _adaptiveOffset;

		pZStage->SetParam(IDevice::PARAM_Z_POS, pos);

		StringCbPrintfW(message,MSG_LENGTH,L"ThorImager SetAutoFocusStartZPosition new af start position %d.%d",(int)pos,(int)((pos - static_cast<long>(pos))*1000));
	}
	else
	{
		double pos = _lastGoodFocusPosition;

		//modify the last good focus position each pass to ensure its unique
		_lastGoodFocusPosition = _lastGoodFocusPosition - .001;

		pos -= _adaptiveOffset;

		pZStage->SetParam(IDevice::PARAM_Z_POS, pos);

		StringCbPrintfW(message,MSG_LENGTH,L"ThorImager SetAutoFocusStartZPosition new af start position %d.%d",(int)pos,(int)((pos - static_cast<long>(pos))*1000));
	}

	pZStage->PreflightPosition();

	pZStage->SetupPosition ();

	pZStage->StartPosition();

	if(TRUE == bWait)
	{
		//don't wait for the z to finish its motion will overlap with the next XY movement
		hEventZ = CreateEvent(0, FALSE, FALSE, 0);

		DWORD dwThread;

		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusZThreadProc4, pZStage, 0, &dwThread );

		const long MAX_Z_WAIT_TIME = 5000;

		DWORD dwWait = WaitForSingleObject( hEventZ, MAX_Z_WAIT_TIME);

		if(dwWait != WAIT_OBJECT_0)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute Z failed");
			//return FALSE;
		}		

		CloseHandle(hThread);
		CloseHandle(hEventZ);
	}
	pZStage->PostflightPosition();	

	return TRUE;
}

long AcquireTSeries::AutoFocusAndRetry(long index, IDevice *pAutoFocusDevice, BOOL &afFound)
{
	IDevice * pZStage = NULL;

	pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

	if(NULL == pZStage)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AutoFocusAndRetry Execute could not create z stage");
		return FALSE;
	}

	afFound = FALSE;

	_pAutoFocus->Execute(index, pAutoFocusDevice,afFound);

	double val;

	pZStage->GetParam(IDevice::PARAM_Z_POS,val);

	const long RETRIES = 1;
	const double RESULT_LOCATION_THREASHOLD_MM = .10;//must be within 50um of the previous location
	const long SIG_DIGITS_MULTIPLIER = 1000;
	long count = 0;
	double lower=0;
	double upper=0;

	do
	{
		lower = _lastGoodFocusPosition - RESULT_LOCATION_THREASHOLD_MM;
		upper = _lastGoodFocusPosition + RESULT_LOCATION_THREASHOLD_MM;

		//check if the focus was found or if the result location is within the threshold
		//if not perform retires for the count of retries specified
		if((val <lower)||(val >upper)||(FALSE == afFound))
		{

			StringCbPrintfW(message,MSG_LENGTH,L"AutoFocusAndRetry z position val %d.%03d outside lower %d.%03d upper %d.%03d",(int)val,(int)((val - static_cast<long>(val))*SIG_DIGITS_MULTIPLIER),(int)lower,(int)((lower - static_cast<long>(lower))*SIG_DIGITS_MULTIPLIER),(int)upper,(int)((upper - static_cast<long>(upper))*SIG_DIGITS_MULTIPLIER));
			logDll->TLTraceEvent(INFORMATION_EVENT,1,message);

			_pAutoFocus->Execute(index, pAutoFocusDevice,afFound);
			pZStage->GetParam(IDevice::PARAM_Z_POS,val);
		}
		else
		{
			StringCbPrintfW(message,MSG_LENGTH,L"AutoFocusAndRetry passed z position %d.%03d",(int)val,(int)((val - static_cast<long>(val))*SIG_DIGITS_MULTIPLIER));
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
			break;
		}
		count++;
	}
	while(count <= RETRIES);

	if((val <lower)||(val >upper)||(FALSE == afFound))
	{
		pZStage->SetParam(IDevice::PARAM_Z_POS,_lastGoodFocusPosition);

		pZStage->PreflightPosition();

		pZStage->SetupPosition ();

		pZStage->StartPosition();

		//don't wait for the z to finish its motion will overlap with the next XY movement
		hEventZ = CreateEvent(0, FALSE, FALSE, 0);

		DWORD dwThread;

		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusZThreadProc4, pZStage, 0, &dwThread );

		const long MAX_Z_WAIT_TIME = 5000;

		DWORD dwWait = WaitForSingleObject( hEventZ, MAX_Z_WAIT_TIME);

		if(dwWait != WAIT_OBJECT_0)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute Z failed");
			//return FALSE;
		}

		CloseHandle(hThread);
		CloseHandle(hEventZ);
		pZStage->PostflightPosition();
	}

	return TRUE;
}

string AcquireTSeries::uUIDSetup(auto_ptr<HardwareSetupXML> &pHardware, long timePoints, long zstageSteps, long zStreamFrames, long index, long subWell)
{
	wchar_t filePathAndName[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	string strOME;

	//build wavelengthname list for quick query later
	vector<string> wavelengthNames;

	//Get the Capture Sequence Settings.
	//If it is a sequential capture, with more than one step the wavelenth
	//information for the steps in the sequence needs to be retrieved to
	//have the the corrent information in the OME.
	long captureSequenceEnable = FALSE;
	_pExp->GetCaptureSequence(captureSequenceEnable);
	vector<IExperiment::SequenceStep> captureSequence;
	_pExp->GetSequenceSteps(captureSequence);
	if (TRUE == captureSequenceEnable && captureSequence.size() > 1)
	{
		for (long i=0; i < captureSequence.size(); i++)
		{
			for (long j=0; j < captureSequence[i].Wavelength.size(); j++)
			{
				wavelengthNames.push_back(captureSequence[i].Wavelength[j].name);
			}
		}
		//Because the wavelengths might be out of order in the sequence
		//Sort them, so the channels are in alphabetical order
		sort(wavelengthNames.begin(), wavelengthNames.end());
	}
	else
	{
		long bufferChannels = _pExp->GetNumberOfWavelengths();
		for(long c=0; c<bufferChannels; c++)
		{
			string wavelengthName;
			double exposureTimeMS;
			_pExp->GetWavelength(c, wavelengthName,exposureTimeMS);
			wavelengthNames.push_back(wavelengthName);
		}
	}

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	imgNameFormat << L"%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";

	long zst = 0;	// zst is the absolute time index that never duplicates for one frame

	for(long t=0; t<timePoints; t++)
	{
		for(long z=0; z<zstageSteps; z++)
		{
			for (long zs=0; zs<zStreamFrames; zs++)
			{
				for(long c=0; c<wavelengthNames.size(); c++)
				{	
					string wavelengthName = wavelengthNames.at(c);

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
					ss << "<TiffData" << " FirstT=\"" << t << "\"" << " FirstZ=\"" << z << "\""  << " FirstZS=\"" << zs << "\""<< " FirstC=\"" << c << "\">" ;

					_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

					//zst = (t - 1) * zstageSteps * zStreamFrames + (z - 1) * zStreamFrames + (zs + 1);  
					zst = t*zstageSteps*zStreamFrames + z*zStreamFrames + zs;  
					StringCbPrintfW(filePathAndName,_MAX_PATH,imgNameFormat.str().c_str(),wavelengthName.c_str(),index,subWell,z+1,zst+1);

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
	}

	return strOME;
}

long AcquireTSeries::ZStreamExecute(long index, long subWell, ICamera *pCamera,long zstageSteps, long timePoints, long undefinedVar)
{	
	_zFrame = zstageSteps;	// indices for "current" location/time pt/stream pt
	_tFrame = timePoints;	
	_pCamera = pCamera;

	return ZStreamExecute(index, subWell);
}

long AcquireTSeries::ZStreamExecute(long index, long subWell)
{	
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

	pHardware->GetMagInfoFromName(objName,magnification,position,numAperture,afStartPos,afFocusOffset,afAdaptiveOffset,beamExpPos,beamExpWavelength,beamExpPos2,beamExpWavelength2,turretPosition,zAxisToEscape,zAxisEscapeDistance);

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

	IDevice * pAutoFocusDevice = NULL;

	pAutoFocusDevice = GetDevice(SelectedHardware::SELECTED_AUTOFOCUS);

	if(NULL == pAutoFocusDevice)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create autofocus device");
		return FALSE;
	}

	BOOL afFound = FALSE;

	if(aftype != IAutoFocus::AF_NONE)
	{
		if (FALSE == AutoFocusAndRetry(index, pAutoFocusDevice, afFound))
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample AutoFocusAndRetry failed");
			return FALSE;
		}
	}

	ICamera *pCamera = NULL;

	pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);

	if(NULL == pCamera)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create camera");
		return FALSE;
	}

	double fieldSizeCalibration = 100.0;
	pCamera->GetParam(ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION,fieldSizeCalibration);

	const int MAX_TIME_LENGTH = 30;
	long wavelengthIndex = 0, zStreamFrames,zStreamMode,zstageSteps, timePoints,triggerModeTimelapse,zEnable;
	string wavelengthName, zstageName;
	double exposureTimeMS, zstageStepSize, zStartPos, intervalSec;

	_pExp->GetWavelength(wavelengthIndex,wavelengthName,exposureTimeMS);
	_pExp->GetZStage(zstageName,zEnable,zstageSteps,zstageStepSize,zStartPos,zStreamFrames,zStreamMode);
	_pExp->GetTimelapse(timePoints,intervalSec,triggerModeTimelapse);

	Dimensions d;
	long avgFrames=1;
	long bufferChannels = 4;
	long avgMode=ICamera::AVG_MODE_NONE;
	double umPerPixel = 1.0;
	long numberOfPlanes = 1;
	SetupDimensions(pCamera,_pExp, fieldSizeCalibration,magnification, d, avgFrames,bufferChannels, avgMode, umPerPixel, numberOfPlanes);
	d.t = zStreamFrames;

	string strOME;
	//build the ome image guid list on the first acquisition only
	if((_zFrame == 1)&&(_tFrame == 1))
	{
		strOME = uUIDSetup(pHardware, timePoints, zstageSteps, zStreamFrames, index,  subWell);
	}

	//Set temp path to streaming temp path:
	wstring tempPath; double previewRate;
	pHardware->GetStreaming(tempPath, previewRate);
	if(NULL != tempPath.c_str())
	{
		ImageManager::getInstance()->SetMemMapPath(tempPath.c_str());
	}
	
	double dflimType = 0;
	if (pCamera->GetParam(ICamera::PARAM_DFLIM_FRAME_TYPE, dflimType))
	{
	//	d.frameType = static_cast<long>(dflimType);
	//}
	//else
	//{
	//	d.frameType = 0;
	}

	long zsImageID;

	if(FALSE == ImageManager::getInstance()->CreateImage(zsImageID,d))
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create average memory buffer");
		return FALSE;
	}

	long maxFrameWaitTime = 30000;
	double typeVal;

	pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,typeVal);

	long cameraType = static_cast<long>(typeVal);
	avgMode = ICamera::AVG_MODE_NONE;

	long currentTriggerMode;
	double dVal;
	pCamera->GetParam(ICamera::PARAM_TRIGGER_MODE,dVal);

	currentTriggerMode = static_cast<long>(dVal);

	if(zStreamFrames > 1)
	{
		pCamera->SetParam(ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE, 1);
		pCamera->SetParam(ICamera::PARAM_MULTI_FRAME_COUNT, zStreamFrames);
	}

	switch(cameraType)
	{
	case ICamera::CCD:
		{
			switch(currentTriggerMode)
			{
			case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
			case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
			case ICamera::HW_SINGLE_FRAME:
				{
					maxFrameWaitTime = INFINITE;
					pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::HW_MULTI_FRAME_TRIGGER_FIRST);
				}
				break;
			default:
				{					
					const long ADDITIONAL_TRANSFER_TIME_MS = 500;
					maxFrameWaitTime = static_cast<long>(exposureTimeMS + ADDITIONAL_TRANSFER_TIME_MS);
					pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_MULTI_FRAME);
				}
				break;
			}

		}
		break;
	case ICamera::LSM:
		{
			if(zStreamFrames > 1)
			{
				switch(currentTriggerMode)
				{
				case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
				case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
				case ICamera::HW_SINGLE_FRAME:
					{
						pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::HW_MULTI_FRAME_TRIGGER_FIRST);
					}
					break;
				default:
					{
						pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_MULTI_FRAME);
					}
					break;
				}
			}
		}
		break;
	}

	double deltaT = 0;
	string acquiredDateTime;
	FrameInfo frameInfo = {0, 0, 0, 0};

	unsigned short *pZStream = (unsigned short*)ImageManager::getInstance()->GetImagePtr(zsImageID,0,0,0,0);

	pCamera->PreflightAcquisition((char*)pZStream);

	pCamera->SetupAcquisition((char*)pZStream);

	//HW time out:
	if(FALSE == pCamera->StartAcquisition((char*)pZStream))
	{
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireTSeries ZStreamExecute pCamera StartAcquisition failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
		pCamera->PostflightAcquisition(NULL);
		ImageManager::getInstance()->UnlockImagePtr(zsImageID,0,0,0,0);
		ImageManager::getInstance()->DestroyImage(zsImageID);
		return FALSE;
	}

	for(_zsFrame=0; _zsFrame<zStreamFrames; _zsFrame++)
	{
		//hEvent = CreateEvent(0, FALSE, FALSE, 0);
		if(_zsFrame == 0)
		{
			// TO DO: 
			// and (index == 1) condition when it is supported to capture multiple sample locations in a later release
			if((subWell == 1) && (_zFrame == 1) && (_tFrame == 1))
			{
				// set current as the start time of the experiment
				_acquireSaveInfo->getInstance()->SetExperimentStartCount();
			}
			else
			{
				deltaT = _acquireSaveInfo->getInstance()->AddTimingInfo();
			}

			acquiredDateTime = _acquireSaveInfo->getInstance()->AddTimestamp();
		}

		long stopStatus = 0;
		long status = ICamera::STATUS_BUSY;

		while((ICamera::STATUS_BUSY == status) || (ICamera::STATUS_PARTIAL == status))
		{
			if(FALSE == pCamera->StatusAcquisition(status))
			{
				break;
			}

			//if the capture is still busy check the stop status
			if(ICamera::STATUS_BUSY == status)
			{
				StopCaptureEventCheck(stopStatus);

				//user has asked to stop the capture
				if(1 == stopStatus)
					break;
			}
		}

		if(1 == stopStatus)
		{
			break;
		}

		pCamera->CopyAcquisition((char*)pZStream, &frameInfo);

		ImageManager::getInstance()->UnlockImagePtr(zsImageID,0,0,0,_zsFrame);

		if(_zsFrame < (zStreamFrames - 1))
		{
			pZStream = (unsigned short*)ImageManager::getInstance()->GetImagePtr(zsImageID,0,0,0,_zsFrame+1);
		}	
	}

	pCamera->PostflightAcquisition(NULL);

	if((aftype != IAutoFocus::AF_NONE)&&(TRUE == _pAutoFocus->WillAFExecuteNextIteration()))
	{
		////move to an offset of of the start location	
		if(FALSE == SetAutoFocusStartZPosition(afStartPos,FALSE,afFound))
		{
			return FALSE;
		}
	}

	PhysicalSize physicalSize;	// unit: um
	double res = floor(umPerPixel*1000+0.5)/1000;	// keep 2 figures after decimal point, that is why 100 (10^2) is multiplied
	physicalSize.x = res;
	physicalSize.y = res;
	physicalSize.z = zstageStepSize;

	long doOME = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"OMETIFFTag",L"value", FALSE);
	long doCompression = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"TIFFCompressionEnable",L"value", FALSE);

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	imgNameFormat << L"%s%s%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";
	std::wstringstream jpgNameFormat;
	jpgNameFormat << L"%s%sjpeg\\%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.jpg";

	for(_zsFrame=0; _zsFrame<zStreamFrames; _zsFrame++)
	{
		pZStream = (unsigned short*)ImageManager::getInstance()->GetImagePtr(zsImageID,0,0,0,_zsFrame);

		ImageCorrections(_pExp, (char*)pZStream, d.x, d.y, bufferChannels);

		FrameInfo frameInfo;
		frameInfo.bufferType = BufferType::INTENSITY;
		frameInfo.imageWidth = d.x;
		frameInfo.imageHeight = d.y;
		//TODO: add DFLIM capabilities to ZStream acquisition
		StatsManager::getInstance()->ComputeStats(pZStream, frameInfo, _lsmChannel,FALSE,TRUE,FALSE);

		//save the deltaT to timing.txt
		wchar_t drive[_MAX_DRIVE];
		wchar_t dir[_MAX_DIR];
		wchar_t fname[_MAX_FNAME];
		wchar_t ext[_MAX_EXT];
		wchar_t timingFile[_MAX_PATH];

		_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

		if((_zFrame == zstageSteps) && (_tFrame == timePoints))
		{
			StringCbPrintfW(timingFile,_MAX_PATH, L"%s%stiming.txt", drive, dir);

			string timeFile = ConvertWStringToString(timingFile);
			//string timeFile = expPath + "\\timing.txt";
			_acquireSaveInfo->getInstance()->SaveTimingToFile(timeFile);
			_acquireSaveInfo->getInstance()->ClearTimingInfo();
			_acquireSaveInfo->getInstance()->ClearTimestamps();
		}


		long red;
		long green;
		long blue;
		long bp;
		long wp;

		GetColorInfo(pHardware.get(),wavelengthName,red,green,blue,bp,wp);

		const int COLOR_MAP_SIZE = 65536;
		unsigned short rlut[COLOR_MAP_SIZE];
		unsigned short glut[COLOR_MAP_SIZE];
		unsigned short blut[COLOR_MAP_SIZE];

		const int COLOR_MAP_BIT_DEPTH_TIFF = 8;

		GetLookUpTables(rlut, glut, blut,red, green, blue, bp, wp,COLOR_MAP_BIT_DEPTH_TIFF);

		wchar_t filePathAndName[_MAX_PATH];

		string redChan,greenChan,blueChan,cyanChan,magentaChan,yellowChan,grayChan;
		pHardware->GetColorChannels(redChan,greenChan,blueChan,cyanChan,magentaChan,yellowChan,grayChan);

		long totalExperimentWavelengths = 0;
		if (TRUE == doOME)
		{
			//Get the Capture Sequence Settings.
			//If it is a sequential capture, with more than one step the wavelenth
			//information for the steps in the sequence needs to be retrieved to
			//have the corrent information in the OME.
			long captureSequenceEnable = FALSE;
			_pExp->GetCaptureSequence(captureSequenceEnable);

			//Get the Capture Sequence Settings
			vector<IExperiment::SequenceStep> captureSequence;
			_pExp->GetSequenceSteps(captureSequence);
			if (TRUE == captureSequenceEnable && captureSequence.size() > 1)
			{

				for (long i=0; i < captureSequence.size(); i++)
				{
					totalExperimentWavelengths += static_cast<long>(captureSequence[i].Wavelength.size());
				}
			}
			else
			{
				totalExperimentWavelengths = _pExp->GetNumberOfWavelengths();
			}
		}

		long bufferOffsetIndex =0;

		pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,typeVal);
		long cameraType = static_cast<long>(typeVal);
		long bitDepth=14;
		switch(cameraType)
		{
		case ICamera::CCD:
		case ICamera::CCD_MOSAIC:
			{
				double bitsPerPixel = 12;
				pCamera->GetParam(ICamera::PARAM_BITS_PER_PIXEL,bitsPerPixel);
				bitDepth = static_cast<long>(bitsPerPixel);
			}
			break;
		case ICamera::LSM:
			{
				bitDepth = 14; //%TODO%  Retrieve the bitdepth from the camera
			}
			break;
		}

		for(long i=0; i<bufferChannels; i++)
		{
			if(i < _pExp->GetNumberOfWavelengths())
			{			
				_pExp->GetWavelength(i,wavelengthName,exposureTimeMS);

				//A single buffer is used for one channel capture so no offset is needed for that case
				if(bufferChannels > 1)
				{
					pHardware->GetWavelengthIndex(wavelengthName,bufferOffsetIndex);
				}

				GetColorInfo(pHardware.get(),wavelengthName,red,green,blue,bp,wp);
				GetLookUpTables(rlut, glut, blut,red, green, blue, bp, wp,COLOR_MAP_BIT_DEPTH_TIFF);

				_tzsFrame = (_tFrame - 1) * zstageSteps * zStreamFrames + (_zFrame - 1) * zStreamFrames + (_zsFrame + 1);	// get the absolite time index which does not duplicate
				StringCbPrintfW(filePathAndName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir,wavelengthName.c_str(),index,subWell,_zFrame,_tzsFrame);

				logDll->TLTraceEvent(INFORMATION_EVENT,1,filePathAndName);				

				string acquiredDate = acquiredDateTime;	// save image buffer as TIFF


				if (TRUE == doOME)
				{
					SaveTIFF(filePathAndName,(char*)pZStream + (bufferOffsetIndex * d.x * d.y * 2),d.x,d.y,rlut,glut,blut,umPerPixel,totalExperimentWavelengths, timePoints, zstageSteps, intervalSec, i, _tFrame-1, _zFrame-1,&acquiredDate,deltaT,&strOME, physicalSize, doCompression);
				}
				else				
				{
					SaveTIFFWithoutOME(filePathAndName,(char*)pZStream + (bufferOffsetIndex * d.x * d.y * 2),d.x,d.y,rlut,glut,blut,umPerPixel,_pExp->GetNumberOfWavelengths(), timePoints, zstageSteps, intervalSec, i, _tFrame-1, _zFrame-1,&acquiredDate,deltaT,doCompression);
				}
				//SaveTIFF(filePathAndName,(char*)pZStream + (bufferOffsetIndex * d.x * d.y * 2),d.x,d.y,rlut,glut,blut,umPerPixel,_pExp->GetNumberOfWavelengths(), timePoints, zstageSteps, intervalSec, i, _tFrame-1, _zFrame-1,&acquiredDate,deltaT,&strOME);

				//if its the first frame of the ZStream save a preview image without OME
				if(0 == _zsFrame)
				{
					StringCbPrintfW(filePathAndName,_MAX_PATH,L"%s%s%S_Preview.tif",drive,dir,wavelengthName.c_str());

					SaveTIFFWithoutOME(filePathAndName,(char*)pZStream + (bufferOffsetIndex * d.x * d.y * 2),d.x,d.y,rlut,glut,blut,umPerPixel,_pExp->GetNumberOfWavelengths(), timePoints, zstageSteps, intervalSec, i, _tFrame-1, _zFrame-1,&acquiredDate,deltaT,FALSE);
				}

				const int COLOR_MAP_BIT_DEPTH_JPEG = 16;	// save image buffer as JPEG
				GetLookUpTables(rlut, glut, blut,red, green, blue, bp, wp,COLOR_MAP_BIT_DEPTH_JPEG);
				StringCbPrintfW(filePathAndName,_MAX_PATH,jpgNameFormat.str().c_str(),drive,dir,wavelengthName.c_str(),index,subWell,_zFrame,_tzsFrame);
				SaveJPEG(filePathAndName,(char*)pZStream + (bufferOffsetIndex * d.x * d.y * 2),d.x,d.y,rlut,glut,blut,bitDepth);
			}
		}

		ImageManager::getInstance()->UnlockImagePtr(zsImageID,0,0,0,_zsFrame);
	}

	//reset the trigger mode in the event it was changed for averaging
	pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE,currentTriggerMode);

	//restore the average mode since it was switched to AVG_MODE_NONE
	pCamera->SetParam(ICamera::PARAM_LSM_AVERAGEMODE,avgMode);	

	ImageManager::getInstance()->DestroyImage(zsImageID);

	return TRUE;

}