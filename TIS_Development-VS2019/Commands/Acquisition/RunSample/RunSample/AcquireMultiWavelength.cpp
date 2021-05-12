#include "stdafx.h"
#include "RunSample.h"
#include "AcquireMultiWavelength.h"

#include "ImageCorrection.h"

extern auto_ptr<CommandDll> shwDll;
extern auto_ptr<TiffLibDll> tiffDll;
wchar_t messageWave[256];


int Call_TiffVSetField(TIFF* out, uint32 ttag_t, ...);
long SaveTIFF(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut, double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string *acquiredDate,double dt, string * omeTiffData, PhysicalSize physicalSize, long doCompression);
long SaveTIFFWithoutOME(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut, double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string *acquiredDate,double dt, long doCompression);
long SaveJPEG(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height,unsigned short * rlut, unsigned short * glut, unsigned short * blut,long bitDepth);
void GetColorInfo(HardwareSetupXML *pHardware,string wavelengthName, long &red, long &green, long &blue,long &bp, long &wp);
void GetLookUpTables(unsigned short * rlut, unsigned short * glut, unsigned short *blut,long red, long green, long blue, long bp, long wp, long bitdepth);
long SetupDimensions(ICamera *pCamera,IExperiment *pExperiment,double fieldSizeCalibration, double magnification, Dimensions &d, long &avgFrames, long &bufferChannels, long &avgMode, double &umPerPixe, long &numOfPlanes);
AcquireMultiWavelength::AcquireMultiWavelength(IExperiment *pExperiment, wstring path)
{
	_pExp = pExperiment;
	_zFrame = 1;
	_tFrame = 1;
	_path = path;
	_zstageStepSize = 0;
}

UINT StatusMultiThreadProc( LPVOID pParam )
{
	long status = ICamera::STATUS_BUSY;

	ICamera * pCamera = (ICamera*)pParam;

	while(status == ICamera::STATUS_BUSY)
	{
		if(FALSE == pCamera->StatusAcquisition(status))
		{
			break;
		}
	}

	SetEvent( AcquireMultiWavelength::hEvent);

	return 0;
}

UINT StatusExThreadProc( LPVOID pParam )
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

	SetEvent( AcquireMultiWavelength::hEventFilter[0]);

	return 0;
}

UINT StatusEmThreadProc( LPVOID pParam )
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

	SetEvent( AcquireMultiWavelength::hEventFilter[1]);

	return 0;
}

UINT StatusDicThreadProc( LPVOID pParam )
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

	SetEvent( AcquireMultiWavelength::hEventFilter[2]);

	return 0;
}

UINT StatusShutterThreadProc( LPVOID pParam )
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

	SetEvent( AcquireMultiWavelength::hEventShutter);

	return 0;
}

UINT StatusZThreadProc( LPVOID pParam )
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

	SetEvent( AcquireMultiWavelength::hEventZ);

	return 0;
}

HANDLE AcquireMultiWavelength::hEvent = NULL;
HANDLE AcquireMultiWavelength::hEventFilter[] = {NULL, NULL, NULL};
HANDLE AcquireMultiWavelength::hEventShutter = NULL;
HANDLE AcquireMultiWavelength::hEventZ = NULL;
BOOL AcquireMultiWavelength::_evenOdd = FALSE;
double AcquireMultiWavelength::_lastGoodFocusPosition = 0.0;

long AcquireMultiWavelength::CallCaptureComplete(long captureComplete)
{
	return TRUE;
}

long AcquireMultiWavelength::CallCaptureImage(long index)
{
	CaptureImage(index);
	return TRUE;
}

long AcquireMultiWavelength::CallSaveImage(long index, BOOL isImageUpdate)
{
	SaveImage(index, isImageUpdate);
	return TRUE;
}

long AcquireMultiWavelength::CallCaptureSubImage(long index)
{
	CaptureSubImage(index);
	return TRUE;
}

long AcquireMultiWavelength::CallSaveSubImage(long index)
{
	SaveSubImage(index);
	return TRUE;
}


long AcquireMultiWavelength::CallSaveZImage(long index, double power0, double power1, double power2, double power3,double power4, double power5)
{
	SaveZImage(index, power0, power1, power2, power3, power4, power5);
	return TRUE;
}

long AcquireMultiWavelength::CallSaveTImage(long index)
{
	SaveTImage(index);
	return TRUE;
}

long AcquireMultiWavelength::CallSequenceStepCurrent(long index)
{
	SequenceStepCurrent(index);
	return TRUE;
}

long AcquireMultiWavelength::PreCaptureEventCheck(long &status)
{
	return TRUE;
}

long AcquireMultiWavelength::StopCaptureEventCheck(long &status)
{
	StopCapture(status);
	return TRUE;
}

long AcquireMultiWavelength::CallStartProgressBar(long index, long resetTotalCount)
{
	StartProgressBar(index, resetTotalCount);
	return TRUE;
}

long AcquireMultiWavelength::Execute(long index, long subWell, long zFrame, long tFrame)
{
	_zFrame = zFrame;
	_tFrame = tFrame;
	return Execute(index,subWell);
}

string WStringToStringAMW(wstring ws)
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

long ParseApplicationSettingsXMLAMW(const char* pcFilename)	// get OMETiffTag Enable
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

long GetOMETIFFTagEnableFlagAMW()
{
	wchar_t fileName[MAX_PATH];
	wstring tempPath = ResourceManager::getInstance()->GetApplicationSettingsPath();
	tempPath += wstring(L"ApplicationSettings.xml");
	StringCbPrintfW(fileName,_MAX_PATH, tempPath.c_str());

	long ret = ParseApplicationSettingsXMLAMW(WStringToStringAMW(fileName).c_str());

	return ret;	
}

long AcquireMultiWavelength::Execute(long index, long subWell)
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
		//move to the autofocus start
		if(FALSE == SetAutoFocusStartZPosition(afStartPos,TRUE,FALSE))
		{
			return FALSE;
		}
	}

	BOOL afFound=FALSE;
	if (FALSE == RunAutofocus(index, aftype, afFound))
	{
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample RunAutoFocus failed");
		return FALSE;
	}

	//get the camera
	ICamera *pCamera = NULL;

	pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);

	if(NULL == pCamera)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute could not create camera");
		return FALSE;
	}

	// getting the filter wheel positions
	/*IDevice *pExcitation = NULL;	

	pExcitation = GetDevice(SelectedHardware::SELECTED_EXCITATION);

	if(NULL == pExcitation)
	{	
	logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute could not create exitation filter wheel");
	return FALSE;
	}*/

	IDevice *pEmission = NULL;

	pEmission = GetDevice(SelectedHardware::SELECTED_EMISSION);

	if(NULL == pEmission)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute could not create emission filter wheel");
		return FALSE;
	}

	IDevice *pDichroic = NULL;	

	pDichroic = GetDevice(SelectedHardware::SELECTED_DICHROIC);

	if(NULL == pDichroic)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute could not create dichroic filter wheel");
		return FALSE;
	}

	IDevice *pShutter = NULL;	
	pShutter = GetDevice(SelectedHardware::SELECTED_SHUTTER1);

	if(NULL == pShutter)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute could not create shutter filter wheel");
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

	long areaMode;
	double areaAngle;
	long scanMode;
	long interleave;
	long pixelX;
	long pixelY;
	long lsmChannel;
	long fieldSize;
	long offsetX; 
	long offsetY;
	long averageMode;
	long averageNum;
	long clockSource;
	long inputRange1;
	long inputRange2;
	long twoWayAlignment;
	long extClockRate;
	double dwellTime;
	long flybackCycles;
	long inputRange3;
	long inputRange4;
	long minimizeFlybackCycles;
	long polarity[4];
	long verticalFlip;
	long horizontalFlip;
	double crsFrequencyHz = 0;
	long timeBasedLineScan = FALSE;
	long timeBasedLineScanMS = 0;
	long threePhotonEnable = FALSE;
	long numberOfPlanes = 1;
	_pExp->GetLSM(areaMode,areaAngle,scanMode,interleave,pixelX,pixelY,lsmChannel, fieldSize, offsetX, offsetY,averageMode, averageNum, clockSource,inputRange1, inputRange2, twoWayAlignment,extClockRate,dwellTime,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles, polarity[0],polarity[1],polarity[2],polarity[3], verticalFlip, horizontalFlip, crsFrequencyHz, timeBasedLineScan, timeBasedLineScanMS, threePhotonEnable, numberOfPlanes);
	pCamera->SetParam(ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION, verticalFlip);
	pCamera->SetParam(ICamera::PARAM_LSM_HORIZONTAL_FLIP, horizontalFlip);
	long width = 0;
	long height = 0;

	double typeVal;

	pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,typeVal);

	//Read number of wavelength
	long wavelengths = _pExp->GetNumberOfWavelengths();

	Dimensions d;
	long avgFrames=1;
	long bufferChannels=1;
	long avgMode=ICamera::AVG_MODE_NONE;
	double umPerPixel = 1.0;

	double fieldSizeCalibration=100.0;

	SetupDimensions(pCamera,_pExp, fieldSizeCalibration,magnification, d, avgFrames,bufferChannels, avgMode, umPerPixel, numberOfPlanes);

	char * pMemoryBuffer = NULL;
	FrameInfo frameInfo = {0, 0, 0, 0};
	long imageID;

	if(ImageManager::getInstance()->CreateImage(imageID,d)== FALSE)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute could not create memory buffer");
		return FALSE;
	}

	pMemoryBuffer = ImageManager::getInstance()->GetImagePtr(imageID,0,0,0,0);

	pCamera->PreflightAcquisition(pMemoryBuffer);

	unsigned short maxRed=0;
	unsigned short maxGreen=0;
	unsigned short maxBlue=0;
	unsigned short maxColor=0;

	wchar_t filePathAndName[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	long zstageSteps, timePoints,triggerModeTimelapse;
	string zstageName;
	double zstageStepSize, intervalSec, zStartPos;	

	PhysicalSize physicalSize;	// unit: um
	double res = floor(umPerPixel*1000+0.5)/1000;	// keep 2 figures after decimal point, that is why 100 (10^2) is multiplied
	physicalSize.x = res;
	physicalSize.y = res;

	long doOME = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"OMETIFFTag",L"value", FALSE);
	long doCompression = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"TIFFCompressionEnable",L"value", FALSE);

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

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	imgNameFormat << L"%s%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";
	std::wstringstream jpgNameFormat;
	jpgNameFormat << L"%s%sjpeg\\%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.jpg";

	//capture and save each wavelength
	for(long i=0; i< wavelengths; i++)
	{	
		long wavelengthIndex = i;
		string wavelengthName;
		double exposureTimeMS, ex, em, dic;
		long fluor,bp,wp;
		string color;
		_pExp->GetWavelength(wavelengthIndex,wavelengthName,exposureTimeMS);

		pHardware->GetWavelength(wavelengthName, ex, em, dic,fluor,color,bp,wp);

		//set the filters
		//PositionFilters(pExcitation, ex, pEmission, em, pDichroic, dic);	

		//Setting the exposure time
		pCamera->SetParam(ICamera::PARAM_EXPOSURE_TIME_MS, exposureTimeMS);

		pCamera->SetupAcquisition(pMemoryBuffer);		

		//OpenShutter(pShutter);
		pCamera->StartAcquisition(pMemoryBuffer);		

		Sleep(static_cast<DWORD>(exposureTimeMS));		

		//Time reaches exposure time close shutter
		//CloseShutter(pShutter);

		hEvent = CreateEvent(0, FALSE, FALSE, 0);

		DWORD dwThreadId;

		HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusMultiThreadProc, pCamera, 0, &dwThreadId );

		const long MAX_CAMERA_WAIT_TIME = 2000;

		long waitTime = static_cast<long>(exposureTimeMS) + MAX_CAMERA_WAIT_TIME;

		DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, waitTime );

		if(dwWait != WAIT_OBJECT_0)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute StatusMultiThreadProc failed");
			return FALSE;
		}

		CloseHandle(hThread);		
		CloseHandle(hEvent);

		pCamera->CopyAcquisition(pMemoryBuffer, &frameInfo);	

		ImageCorrections(_pExp, pMemoryBuffer, d.x, d.y, 1);

		long zStreamFrames,zStreamMode,zEnable;
		_pExp->GetZStage(zstageName,zEnable,zstageSteps,zstageStepSize,zStartPos,zStreamFrames,zStreamMode);
		_pExp->GetTimelapse(timePoints,intervalSec,triggerModeTimelapse);
		physicalSize.z = _zstageStepSize;

		long red;
		long green;
		long blue;

		GetColorInfo(pHardware.get(),wavelengthName,red,green,blue,bp,wp);

		const int COLOR_MAP_SIZE = 65536;
		unsigned short rlut[COLOR_MAP_SIZE];
		unsigned short glut[COLOR_MAP_SIZE];
		unsigned short blut[COLOR_MAP_SIZE];

		const int COLOR_MAP_BIT_DEPTH_TIFF = 8;

		GetLookUpTables(rlut, glut, blut,red, green, blue, bp, wp,COLOR_MAP_BIT_DEPTH_TIFF);

		_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

		StringCbPrintfW(filePathAndName,_MAX_PATH,imgNameFormat.str().c_str(),drive,dir,wavelengthName.c_str(),index,subWell,zstageSteps,timePoints);

		logDll->TLTraceEvent(INFORMATION_EVENT,1,filePathAndName);

		if (TRUE == doOME)
		{
			SaveTIFF(filePathAndName, pMemoryBuffer, width, height,rlut,glut,blut, umPerPixel,wavelengths,timePoints,zstageSteps,intervalSec,i,_tFrame,_zFrame,NULL,0,NULL, physicalSize, doCompression);
		}
		else
		{
			SaveTIFFWithoutOME(filePathAndName, pMemoryBuffer, width, height,rlut,glut,blut, umPerPixel,wavelengths,timePoints,zstageSteps,intervalSec,i,_tFrame,_zFrame,NULL,0, doCompression);
		}

		//SaveTIFF(filePathAndName, pMemoryBuffer, width, height,rlut,glut,blut, umPerPixel,wavelengths,timePoints,zstageSteps,intervalSec,i,_tFrame,_zFrame,NULL,0,NULL);

		const int COLOR_MAP_BIT_DEPTH_JPEG = 16;

		GetLookUpTables(rlut, glut, blut,red, green, blue, bp, wp,COLOR_MAP_BIT_DEPTH_JPEG);

		StringCbPrintfW(filePathAndName,_MAX_PATH,jpgNameFormat.str().c_str(),drive,dir,wavelengthName.c_str(),index,subWell,zstageSteps,timePoints);

		SaveJPEG(filePathAndName, pMemoryBuffer, width, height,rlut,glut ,blut,bitDepth);
	}

	pCamera->PostflightAcquisition(NULL);		

	if(_evenOdd == FALSE)
	{
		_evenOdd = TRUE;
		afStartPos += 0.01;
	}
	else
	{
		_evenOdd = FALSE;
	}

	if(TRUE == AutofocusExecuteNextIteration(aftype))
	{
		//move to an offset of of the start location	
		if(FALSE == SetAutoFocusStartZPosition(afStartPos,FALSE,afFound))
		{
			return FALSE;
		}
	}

	/*

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPARRAY buffer;

	wstring wss;
	wss = filePathAndName;
	//writing into color jpeg
	StringCbPrintfW(message,MSG_LENGTH,L"AcquireMultiWavelength Execute color jpeg start");
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	StringCbPrintfW(filePathAndName,_MAX_PATH,L"%s%sjpeg\\%S_%04d_%04d_%04d_%04d.jpg",drive,dir,"color",index,subWell,zstageSteps,timePoints);

	int bytes_per_pixel = 3;   
	int color_space = JCS_RGB; 

	wss = filePathAndName;

	size_t origsize = wcslen(wss.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t convertedChars = 0;
	char nstring[newsize];
	wcstombs_s(&convertedChars, nstring, origsize, wss.c_str(), _TRUNCATE);

	string str(nstring);

	FILE *outfile = fopen(str.c_str(), "wb");

	if ( !outfile )
	{
	StringCbPrintfW(message,MSG_LENGTH,L"AcquireMultiWavelength Execute Error opening output jpeg %s",filePathAndName);
	logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
	return FALSE;
	}

	cinfo.err = jpeg_std_error( &jerr );

	jpeg_CreateCompress(&cinfo,JPEG_LIB_VERSION, (size_t) sizeof(struct jpeg_compress_struct));

	jpeg_stdio_dest(&cinfo, outfile);

	// Setting the parameters of the output file here 
	cinfo.image_width = width;	
	cinfo.image_height = height;
	cinfo.input_components = bytes_per_pixel;
	cinfo.in_color_space = (J_COLOR_SPACE)color_space;

	// default compression parameters, we shouldn't be worried about these 
	jpeg_set_defaults( &cinfo );		

	jpeg_start_compress( &cinfo, TRUE );

	unsigned char * rowBuffer = new unsigned char[cinfo.image_width * cinfo.input_components];
	unsigned short *pColorR = (unsigned short*)pColorMemoryBuffer[0];
	unsigned short *pColorG = (unsigned short*)pColorMemoryBuffer[1];
	unsigned short *pColorB = (unsigned short*)pColorMemoryBuffer[2];
	unsigned char *p2;

	while( cinfo.next_scanline < cinfo.image_height )
	{
	p2 = rowBuffer;

	for(unsigned int i = 0; i<cinfo.image_width; i++,p2+=3,pColorR++,pColorG++,pColorB++)
	{
	*(p2+0) = static_cast<unsigned char>((*pColorR/maxColor) * 256);
	*(p2+1) = static_cast<unsigned char>((*pColorG/maxColor) * 256);					
	*(p2+2) = static_cast<unsigned char>((*pColorB/maxColor)* 256);					
	}

	buffer = (JSAMPARRAY)&rowBuffer;
	jpeg_write_scanlines( &cinfo, buffer, 1 );

	}

	delete rowBuffer;

	// similar to read file, clean up after we're done compressing 
	jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );

	fclose( outfile );
	*/
	ImageManager::getInstance()->DestroyImage(imageID);

	return TRUE;

}



void AcquireMultiWavelength::PositionFilters(IDevice *pExcitation, double ex, IDevice *pEmission, double em, IDevice *pDichroic, double dic)
{
	//put filters into position
	pExcitation->SetParam(IDevice::PARAM_FW_EX_POS, ex );

	pExcitation->PreflightPosition();

	pExcitation->SetupPosition ();

	pExcitation->StartPosition();

	hEventFilter[0] = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThreadEx;

	HANDLE hThreadEx = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusExThreadProc, pExcitation, 0, &dwThreadEx );


	pEmission->SetParam(IDevice::PARAM_FW_EM_POS,em);

	pEmission->PreflightPosition();

	pEmission->SetupPosition ();

	pEmission->StartPosition();

	hEventFilter[1] = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThreadEm;

	HANDLE hThreadEm = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusEmThreadProc, pEmission, 0, &dwThreadEm );


	pDichroic->SetParam(IDevice::PARAM_FW_DIC_POS, dic );

	pDichroic->PreflightPosition();

	pDichroic->SetupPosition ();

	pDichroic->StartPosition();

	hEventFilter[2] = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThreadDi;

	HANDLE hThreadDi = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusDicThreadProc, pDichroic, 0, &dwThreadDi );

	const long MAX_FILTER_WAIT_TIME = 10000;

	DWORD dwWait = WaitForMultipleObjects( 3, hEventFilter, TRUE, MAX_FILTER_WAIT_TIME);

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute Filter ExEmDic failed");
		//return FALSE;
	}		

	CloseHandle(hThreadEx);
	CloseHandle(hThreadEm);
	CloseHandle(hThreadDi);
	CloseHandle(hEventFilter[0]);
	CloseHandle(hEventFilter[1]);
	CloseHandle(hEventFilter[2]);

	pExcitation->PostflightPosition();	
	pEmission->PostflightPosition();	
	pDichroic->PostflightPosition();	
}

void AcquireMultiWavelength::OpenShutter(IDevice *pShutter)
{
	pShutter->SetParam(IDevice::PARAM_SHUTTER_POS,1 /*From xml file*/ );

	pShutter->PreflightPosition();

	pShutter->SetupPosition ();

	pShutter->StartPosition();

	hEventShutter = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThreadShutter;

	HANDLE hThreadShutter = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusShutterThreadProc, pShutter, 0, &dwThreadShutter );

	const long MAX_SHUTTER_WAIT_TIME = 5000;

	DWORD dwWait = WaitForMultipleObjects( 1, &hEventShutter, TRUE, MAX_SHUTTER_WAIT_TIME );

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireMultiWavelength Execute StatusShutter failed");
		//return FALSE;
	}

	CloseHandle(hThreadShutter);
	CloseHandle(hEventShutter);
}

void AcquireMultiWavelength::CloseShutter(IDevice *pShutter)
{
	//pShutter->PostflightPosition();
}
