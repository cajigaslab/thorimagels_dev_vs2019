#include "stdafx.h"
#include "math.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"
#include "ImageRoutineLSM.h"
#include "ImageRoutineSciCam.h"

void (*myFunctionPointer)(char * buffer,FrameInfoStruct &imageInfo) = NULL;
//mechanism to notify listeners that the z Stack capture is finished
void (*myFuncPtrZStack)() = NULL;

char * pMemoryBuffer = NULL;
char * pBackgroundSubtractionBuffer = NULL;
char * pFlatFieldBuffer = NULL;
char * pPockelsPlot = NULL;
char * pChan[32] = {NULL};

unsigned long long imageBufferSize = 0;
FrameInfoStruct imageInfo = {1392, 1040, 1, 1};
long disableZRead = FALSE;
long _autoDisplayChannel = 0;

DWORD dwLiveThreadId = NULL;
HANDLE hLiveThread = NULL;

DWORD dwZStackCaptureThreadId = NULL;
HANDLE hZStackCaptureThread = NULL;

DWORD dwBleachThreadId = NULL;
HANDLE hBleachThread = NULL;

DWORD _dwAutoFocusCaptureThreadId = NULL;
HANDLE _hAutoFocusCaptureThread = NULL;

DWORD _dwAutoFocusStatusThreadId = NULL;
HANDLE _hAutoFocusStatusThread = NULL;

DWORD _dwSafetyInterLockCheckThreadId = NULL;
HANDLE _hSafetyInterLockCheckThread = NULL;

//events
HANDLE hStatusEvent[3];
HANDLE hStatusBleachScanner = NULL;

HANDLE hCaptureActive = NULL;
BOOL stopCapture = FALSE;
HANDLE hEventBleach = NULL;
BOOL stopBleach = FALSE;
BOOL activeBleach = FALSE;
BOOL InterruptCapture = FALSE;
BOOL inFileLoading = FALSE;
atomic<BOOL> _shutterOpened = FALSE;

const long MSG_SIZE = 256;
wchar_t message[MSG_SIZE];

auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
auto_ptr<CommandDll> shwDll(new CommandDll(L".\\Modules_Native\\SelectHardware.dll"));
auto_ptr<TiffLibDll> tiffDll(new TiffLibDll(L".\\libtiff3.dll"));

struct StatusDeviceProcParams
{
	IDevice *pDevice;
	HANDLE *pEventHandle;
};

UINT StatusDeviceThreadProc( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;
	StatusDeviceProcParams * pStatus = (StatusDeviceProcParams*)malloc(sizeof(StatusDeviceProcParams));
	memcpy(pStatus, (StatusDeviceProcParams*)pParam, sizeof(StatusDeviceProcParams));

	while((status == IDevice::STATUS_BUSY) && (FALSE == InterruptCapture))
	{
		if(FALSE == pStatus->pDevice->StatusPosition(status))
		{
			break;
		}
	}
	if (FALSE == InterruptCapture)
	{
		SetEvent( *pStatus->pEventHandle);	
	}	

	free(pStatus);
	return 0;
}

long StatusHandlerDevice(IDevice * pDevice, HANDLE *pEvent, long maxWaitTime)
{
	long ret = TRUE;

	if(NULL == pEvent)
	{
		*pEvent = CreateEvent(0, FALSE, FALSE, 0);
	}

	DWORD dwThread;

	StatusDeviceProcParams statusParams;

	statusParams.pDevice = pDevice;
	statusParams.pEventHandle = pEvent;

	HANDLE hThread = ::CreateThread( NULL, 0,(LPTHREAD_START_ROUTINE)StatusDeviceThreadProc, &statusParams, 0, &dwThread );

	DWORD dwWait = WaitForSingleObject(*pEvent, maxWaitTime);

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"StatusHandlerDevice  StatusThreadProc  failed");
		ret = FALSE;
	}		

	SAFE_DELETE_HANDLE(hThread);
	CloseHandle(*pEvent);
	return ret;
}

int Call_TiffVSetField(TIFF* out, uint32 ttag_t, ...)
{
	int retv;
	va_list marker;

	va_start( marker, ttag_t );  

	retv = tiffDll->TIFFVSetField(out, ttag_t, marker ); //actual setting of the colormap into the TIFF images

	va_end( marker );             

	return retv;
}

string CreateOMEMetadata(int width, int height,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string * acquiredDateTime, double deltaT, string * omeTiffData)
{


	string tagData = "<?xml version=\"1.0\"?><OME xmlns=\"http://www.openmicroscopy.org/Schemas/OME/2010-06\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.openmicroscopy.org/Schemas/OME/2010-06 http://www.openmicroscopy.org/Schemas/OME/2010-06/ome.xsd\">";

	tagData += "<Image ID=\"Image:0\" AcquiredDate=";
	std::ostringstream ssDT;
	if(NULL == acquiredDateTime)
	{
		ssDT<<"\" \"";
	}
	else
	{
		ssDT<<"\"" << *acquiredDateTime <<"\"";
	}
	tagData += ssDT.str();
	tagData += "><Pixels";
	tagData += " DimensionOrder=";
	tagData += "\"XYCZT\"";
	tagData += " ID=\"Pixels:0\"";
	// tagData += " PhysicalSizeX=\"1.0\"";
	// tagData += " PhysicalSizeY=\"1.0\"";
	// tagData += " PhysicalSizeZ=\"1.0\"";			
	tagData += " SizeC=";
	std::ostringstream ssC;
	ssC << "\"" << nc << "\"";
	tagData += ssC.str();
	tagData += " SizeT=";
	std::ostringstream ssT;
	ssT << "\"" << nt << "\"";
	tagData += ssT.str();
	tagData += " SizeX=";
	std::ostringstream ssX;
	ssX << "\"" << width << "\"";
	tagData += ssX.str();
	tagData += " SizeY=";
	std::ostringstream ssY;
	ssY << "\"" << height << "\"";
	tagData += ssY.str();
	tagData += " SizeZ=";
	std::ostringstream ssZ;
	ssZ << "\"" << nz << "\"";
	tagData += ssZ.str();
	tagData += " TimeIncrement=";
	std::ostringstream ssTi;
	ssTi << "\"" << timeIncrement << "\"";
	tagData += ssTi.str();
	tagData += " Type=";
	tagData += "\"uint16\"";
	tagData += ">";
	tagData += "<Channel ID=";
	tagData += "\"Channel:0:0\"";
	tagData += " SamplesPerPixel=\"1\"><LightPath/></Channel>";
	tagData += "<BinData BigEndian=\"false\" Length = \"0\" xmlns=\"http://www.openmicroscopy.org/Schemas/BinaryFile/2010-06\"/>";
	tagData += "<Plane TheZ=";
	std::ostringstream ssTheZ;
	ssTheZ << "\"" << z << "\" ";
	tagData += ssTheZ.str();
	tagData += "TheT=";
	std::ostringstream ssTheT;
	ssTheT << "\"" << t << "\" ";
	tagData += ssTheT.str();
	tagData += "TheC=";
	std::ostringstream ssTheC;
	ssTheC << "\"" << c << "\" ";
	tagData += ssTheC.str();
	tagData += "DeltaT=";
	std::ostringstream ssDeltaT;
	ssDeltaT.setf(3);
	ssDeltaT << "\"" << deltaT << "\"/>";
	tagData += ssDeltaT.str();


	if(NULL == omeTiffData)
	{
		std::ostringstream ssFirst;
		ssFirst << "<TiffData FirstC=\"" << c << "\"" << " FirstT=\"" << t << "\"" << " FirstZ=\"" << z << "\"" << " IFD=\"0\" PlaneCount=\"1\">";
		tagData += ssFirst.str();
		tagData += "</TiffData>";
	}
	else
	{
		tagData += *omeTiffData;
	}
	tagData += "</Pixels>";
	tagData += "</Image>";
	tagData += "<StructuredAnnotations xmlns=\"http://www.openmicroscopy.org/Schemas/SA/2010-06\"/>";
	tagData += "</OME>";

	return tagData;
}

/// <summary> Save multiple channels as multipage TIFF </summary>
long SaveMultiPageTIFF(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut,double umPerPixel,int nc, int nt, int nz, double timeIncrement, int t, int z,string * acquiredDateTime,double deltaT, string * omeTiffData, long doCompression)
{
	TIFF *out= tiffDll->TIFFOpenW(filePathAndName, "w"); 
	if(!out)
		return FALSE;

	int sampleperpixel = 1;    // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon. 

	tsize_t linebytes = sampleperpixel * width * 2;     // length in memory of one row of pixel in the image. 

	unsigned char *buf = NULL;        // buffer used to store the row of pixel information for writing to file

	for (int page = 0; page < nc; ++page)
	{
		tiffDll->TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
		tiffDll->TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
		tiffDll->TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel

		tiffDll->TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);    // set the size of the channels
		tiffDll->TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.

		int compressionMode = (TRUE == doCompression)? COMPRESSION_LZW : COMPRESSION_NONE;
		tiffDll->TIFFSetField(out, TIFFTAG_COMPRESSION, compressionMode);
		tiffDll->TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

		tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK); 

		//units are pixels per inch
		const int RESOLUTION_UNIT = 2;
		tiffDll->TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESOLUTION_UNIT); 

		const double UM_PER_INCH_CONVERSION = 25399.99999997256800000002962656;

		float pixelsPerInch = static_cast<float>(UM_PER_INCH_CONVERSION/umPerPixel);	
		Call_TiffVSetField(out, TIFFTAG_XRESOLUTION, pixelsPerInch);
		Call_TiffVSetField(out, TIFFTAG_YRESOLUTION, pixelsPerInch);

		string str = CreateOMEMetadata(width, height, nc, nt, nz, timeIncrement, page+1, t, z, acquiredDateTime, deltaT, omeTiffData);

		Call_TiffVSetField(out, TIFFTAG_IMAGEDESCRIPTION, str.c_str());

		// We set the strip size of the file to be size of one row of pixels
		tiffDll->TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, tiffDll->TIFFDefaultStripSize(out, width*sampleperpixel));

		//specify page filetype and number:
		tiffDll->TIFFSetField(out, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE); 
		Call_TiffVSetField(out,TIFFTAG_PAGENUMBER, page, nc);

		// Allocating memory to store the pixels of current row once
		if(0 == page)
		{
			if (tiffDll->TIFFScanlineSize(out)!=linebytes)
				return FALSE;
			buf =(unsigned char *)tiffDll->_TIFFmalloc(linebytes);
		}

		//Now writing image to the file one strip at a time
		for (uint32 row = 0; row < static_cast<uint32>(height); row++)
		{
			memcpy(buf, &pMemoryBuffer[(page*linebytes*height)+(row*linebytes)], linebytes);
			if (tiffDll->TIFFWriteScanline(out, buf, row, 0) < 0)
				break;
		}

		tiffDll->TIFFWriteDirectory(out);

		tiffDll->TIFFFlush(out);		
	}

	//Finally we destroy the buffer and close the output file
	if (buf)
		tiffDll->_TIFFfree(buf);

	tiffDll->TIFFClose(out); 

	return TRUE;
}

long SaveTIFF(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut,double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string * acquiredDateTime,double deltaT, string * omeTiffData,long doCompression)
{
	TIFF *out= tiffDll->TIFFOpenW(filePathAndName, "w"); 

	//XML-READ
	int sampleperpixel = 1;    // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon. 

	tiffDll->TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
	tiffDll->TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
	tiffDll->TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
	//XML-READ
	tiffDll->TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);    // set the size of the channels
	tiffDll->TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.

	int compressionMode = (TRUE == doCompression)? COMPRESSION_LZW : COMPRESSION_NONE;
	tiffDll->TIFFSetField(out, TIFFTAG_COMPRESSION, compressionMode);
	tiffDll->TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK); 

	//units are pixels per inch
	const int RESOLUTION_UNIT = 2;
	tiffDll->TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESOLUTION_UNIT); 

	const double UM_PER_INCH_CONVERSION = 25399.99999997256800000002962656;

	float pixelsPerInch = static_cast<float>(UM_PER_INCH_CONVERSION/umPerPixel);	
	Call_TiffVSetField(out, TIFFTAG_XRESOLUTION, pixelsPerInch);
	Call_TiffVSetField(out, TIFFTAG_YRESOLUTION, pixelsPerInch);

	// Color Palette is not working with the OME TIFF. /*TODO*/ determine the issue and reinsert at a later date
	//	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
	//	Call_TiffVSetField(out, TIFFTAG_COLORMAP, rlut, glut, blut);

	string str = CreateOMEMetadata(width, height,nc, nt, nz, timeIncrement, c, t, z,acquiredDateTime, deltaT, omeTiffData);

	Call_TiffVSetField(out, TIFFTAG_IMAGEDESCRIPTION, str.c_str());

	tsize_t linebytes = sampleperpixel * width * 2;     // length in memory of one row of pixel in the image. 

	unsigned char *buf = NULL;        // buffer used to store the row of pixel information for writing to file

	// Allocating memory to store the pixels of current row
	if (tiffDll->TIFFScanlineSize(out)==linebytes)
		buf =(unsigned char *)tiffDll->_TIFFmalloc(linebytes);
	else
		buf = (unsigned char *)tiffDll->_TIFFmalloc(tiffDll->TIFFScanlineSize(out));

	// We set the strip size of the file to be size of one row of pixels
	tiffDll->TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, tiffDll->TIFFDefaultStripSize(out, width*sampleperpixel));


	//Now writing image to the file one strip at a time
	for (uint32 row = 0; row < static_cast<uint32>(height); row++)
	{
		memcpy(buf, &pMemoryBuffer[row*linebytes], linebytes);    // check the index here, and figure out why not using h*linebytes
		if (tiffDll->TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
	}

	if (buf)
		tiffDll->_TIFFfree(buf);

	//Finally we close the output file, and destroy the buffer 
	tiffDll->TIFFClose(out); 

	return TRUE;
}

CaptureSetup::CaptureSetup()
{
	//private constructor

	_params.version = 1.0;
	_params.showDialog = FALSE;

	memset(&_paramsCustom,0,sizeof _paramsCustom);	
	_enablePincushionCorrection = FALSE;
	_enableBackgroundSubtraction = FALSE;
	_enableFlatField = FALSE;

	_coefficientK1 = 0;
	_coefficientK2 = 0;
	_coefficientK3 = 0;
	_coefficientK4 = 1;

	_frameRate = 0;

	_enableROIStats = FALSE;

	_statusError[0] = FALSE;
	_statusError[1] = FALSE;
	_statusError[2] = FALSE;

	for (long i = 0; i <  MAX_BLEACH_PARAMS_CNT; ++i)
	{
		bleachParams[i] = NULL;
	}

	for (long i = 0; i <  MAX_IMAGE_ROUTINES; ++i)
	{	
		_pImageRoutines[i] = NULL;
	}
	_pActiveImageRoutine = NULL;
}

GUID CaptureSetup::_guid = { 0xbed71593, 0xce3d, 0x4909, { 0x9d, 0xe5, 0x63, 0xce, 0x3a, 0x5, 0x28, 0x9b } };

bool CaptureSetup::_instanceFlag = false;

bool CaptureSetup::_setupFlag = false;

auto_ptr<CaptureSetup> CaptureSetup::_single(NULL);

CritSect CaptureSetup::critSect;

CritSect CaptureSetup::critSectLiveImage;

CaptureSetup* CaptureSetup::getInstance()
{
	if(! _instanceFlag)
	{
		StringCbPrintfW(message,MSG_SIZE,L"Creating CaptureSetup Singleton");
		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

		_single.reset(new CaptureSetup());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

std::string CaptureSetup::ConvertWStringToString(wstring ws)
{
	size_t origsize = wcslen(ws.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t convertedChars = 0;
	char nstring[newsize];
	wcstombs_s(&convertedChars, nstring, origsize, ws.c_str(), _TRUNCATE);

	string str(nstring);

	return str;
}

bool CaptureSetup::GetSetupFlagState()
{
	return _setupFlag;
}

long CaptureSetup::GetCommandGUID(GUID *guidRet)
{
	memcpy(guidRet,&_guid,sizeof GUID);

	return TRUE;
}

long CaptureSetup::GetParamInfo(const long paramID, long &paramType,long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_VERSION:
		{
			paramType = ICommand::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 1.0;
			paramMax = 1.0;
			paramDefault = 1.0;
		}
		break;
	case PARAM_SHOWDIALOG:
		{
			paramType = ICommand::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1.0;
			paramDefault = 1.0;
		}
		break;
	default :
		{
			ret = FALSE;
		}
	}

	return ret;
}

long CaptureSetup::SetParam(const long paramID, const double param)
{
	long ret = FALSE;

	switch(paramID)
	{
	case PARAM_VERSION:
		{
			ret = FALSE;
		}
		break;
	case PARAM_SHOWDIALOG:
		{
			if((param >= ICommand::PARAM_SHOWDIALOG_MIN) && (param<=ICommand::PARAM_SHOWDIALOG_MAX))
			{
				_params.showDialog = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	default :
		{
		}
	}

	return ret;
}

long CaptureSetup::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_VERSION:
		{
			param = _params.version;
		}
		break;
	case PARAM_SHOWDIALOG:
		{
			param = _params.showDialog;
		}
		break;
	default :
		{
			ret = FALSE;
		}
	}

	return ret;
}

long CaptureSetup::SetCustomParamsBinary(const char *buf)
{
	memcpy(&_paramsCustom,buf,sizeof (_paramsCustom));

	return TRUE;
}

long CaptureSetup::GetCustomParamsBinary(char *buf)
{
	memcpy(buf,&_paramsCustom,sizeof (_paramsCustom));

	return TRUE;
}

long CaptureSetup::SaveCustomParamsXML(void *fileHandle)
{
	return FALSE;
}

long CaptureSetup::LoadCustomParamXML(void *fileHandle)
{
	return FALSE;
}

long CaptureSetup::Execute()
{
	return TRUE;
}

long CaptureSetup::Status(long &status)
{
	status = ICommand::STATUS_READY;

	return TRUE;
}

long CaptureSetup::FindHardware()
{
	return TRUE;
}


long CaptureSetup::SetupCommand()
{
	Lock(CaptureSetup::getInstance()->critSectLiveImage);

	if(FALSE == _setupFlag)
	{
		if(NULL == GetCamera(SelectedHardware::SELECTED_CAMERA1))
		{	
			logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup could not create camera");
			return FALSE;
		}	

		//image routines have not been allocated
		//create once
		if(NULL == _pImageRoutines[0])
		{
			_pImageRoutines[0] = (IImageRoutine*)new ImageRoutineLSM();
			_pImageRoutines[1] = (IImageRoutine*)new ImageRoutineSciCam();

			if((NULL == _pImageRoutines[0])||(NULL == _pImageRoutines[1])||(NULL == _pImageRoutines[2]))
			{
				logDll->TLTraceEvent(ERROR_EVENT,1,L"CaptureSetup unable to create image routines");
				return FALSE;
			}		
		}

		//assign the appropriate image routine
		long camType = 0;
		GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_TYPE,camType);

		switch(camType)
		{
		case ICamera::LSM:_pActiveImageRoutine = _pImageRoutines[0]; break;
		case ICamera::CCD:
		case ICamera::CCD_MOSAIC:
			_pActiveImageRoutine = _pImageRoutines[1]; break;
		}

		if(_pActiveImageRoutine->InitParameters() == FALSE)
		{
			return FALSE;
		}

		_setupFlag = TRUE;
		for (int i = 0; i < MAX_BLEACH_PARAMS_CNT; i++)
		{
			bleachParams[i] = NULL;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

long CaptureSetup::TeardownCommand()
{
	if(TRUE == _setupFlag)
	{
		_setupFlag = FALSE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

long CaptureSetup::Stop()
{
	return TRUE;
}

void CaptureSetup::SetFrameRate(double val)
{
	_frameRate = val;
}

double CaptureSetup::GetFrameRate()
{
	//if the frame rate is supported by the cameras use it. Otherwise use the calculated value
	GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_FRAME_RATE, _frameRate);

	return _frameRate;
}

long CaptureSetup::SetDisplayChannels(int channelEnable)
{	
	CHECK_PACTIVEIMAGEROUTINE(SetDisplayChannels(channelEnable));
}

long CaptureSetup::ImgProGenConf(int maxRoiNum, int minSnr)
{
	StatsManager::_maxRoiNum = maxRoiNum;
	StatsManager::_minSnr = minSnr;
	return TRUE;
}

long CaptureSetup::EnableMinAreaFilter(bool minAreaActive, int minAreaValue)
{
	StatsManager::_minAreaFilterActive = minAreaActive;
	if (minAreaActive == true)
	{
		StatsManager::_minAreaFilterValue = minAreaValue;
	}
	return TRUE;
}
