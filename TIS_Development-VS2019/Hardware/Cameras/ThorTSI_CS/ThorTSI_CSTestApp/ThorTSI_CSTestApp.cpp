// CameraManger.cpp : Defines the exported functions for the DLL application.
//

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <direct.h>
#include <windows.h>
#include <tchar.h>

//#include "ThorTSI_CS.h"

//#include "stdafx.h"
#include "Camera.h"
#include "CameraManager.h"
#include "ThorTSI_CSTestApp.h"

//auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
wchar_t message[256];

#define TCHARS(a)											(sizeof(a)/sizeof(TCHAR))

#define CH_ENTER											0x000D
#define CH_ESC												0x001B
#define CH_BKSP												0x0008

#define TSI_TEST_DEBUG_ENABLED								1
#define TSI_TEST_DEBUG_LVL									0
#define TSI_TEST_ERROR_LVL									0
#define TSI_TEST_TRACE_DBG_LVL								0

int TSI_DebugLevel											= 0;

static CRITICAL_SECTION	eTSIMutex;
static BOOL				bTSIErrInit							= FALSE ;

bool					event_tracing_on					= false;

typedef enum _IMAGE_FORMAT {
	IMAGE_FORMAT_RAW,
	IMAGE_FORMAT_PNG,
	IMAGE_FORMAT_TIFF,
	MAX_IMAGE_FORMATS
} IMAGE_FORMAT, *PIMAGE_FORMAT;

IMAGE_FORMAT			image_format						= IMAGE_FORMAT_RAW;

volatile int			images_to_buffer					= 0;
int						images_buffered						= 0;
HANDLE					BufferHeap							= NULL;
VOID					**BufferImagePointers				= NULL;

typedef struct _CTL_EVENT_COUNTER_DEF {

	unsigned long exposure_start;
	unsigned long exposure_complete;
	unsigned long sequence_start;
	unsigned long sequence_complete;
	unsigned long readout_start;
	unsigned long readout_complete;
	unsigned long spurious;

} CTL_EVENT_COUNTER_DEF, *PCTL_EVENT_COUNTER_DEF;

typedef struct _IMG_EVENT_COUNTER_DEF {

	unsigned long notifications;
	unsigned long retrieved;
	unsigned long errors;
	unsigned long spurious;

} IMG_EVENT_COUNTER_DEF, *PIMG_EVENT_COUNTER_DEF;

volatile CTL_EVENT_COUNTER_DEF ControlEventCounters			= {0};
volatile IMG_EVENT_COUNTER_DEF ImageEventCounters			= {0};

typedef struct _IMAGING_TEST {
	long TestNumber;
	long ExposureTime;
	long TotalFrames;
	long NumberOfFrames;
	long Left;
	long Right;
	long Top;
	long Bottom;
	long XBin;
	long YBin;
	long TrigMode;
	long OpMode;
	char *Desc;
} IMAGING_TEST, *PIMAGING_TEST;

#define IMAGING_TEST_DEF(TestNumber,ExposureTime,TotalFrames,NumberOfFrames,Left,Right,Top,Bottom,XBin,YBin,TrigMode,OpMode) \
{TestNumber,ExposureTime,TotalFrames,NumberOfFrames,Left,Right,Top,Bottom,XBin,YBin,TrigMode,OpMode,           \
	"CT_#" #TestNumber "_" #ExposureTime "_" #TotalFrames "_" #NumberOfFrames "_" #Left "_" #Right "_" #Top "_" #Bottom "_" #XBin "_" #YBin "_" #OpMode}

#if TSI_TEST_DEBUG_ENABLED

#define TSITest_DbgPrintFunc(lvl,fmt,...) TSITest_DbgPrintEx(lvl, __FUNCTION__, __LINE__ , fmt, __VA_ARGS__)

char *TSITest_DbgPrintEx (int ErrorLevel, char *FunctionName, int LineNumber, TCHAR *FormatStr, ...);

static void
	TSITest_CheckErrInit (void)
{

	if (bTSIErrInit) {
		return ;
	}

	bTSIErrInit = TRUE;

	InitializeCriticalSection (&eTSIMutex);

}

char *TSITest_DbgPrintEx (int ErrorLevel, char *FunctionName, int LineNumber, TCHAR *FormatStr, ...)
{

	// DBG_ENTRY();

	if (ErrorLevel > TSI_DebugLevel) {
		return NULL;
	}

	if (FormatStr != NULL) {

		TCHAR TempMsg [4096]; 
		TCHAR OutputMsg [4096]; 

		va_list stack;

		TSITest_CheckErrInit ();

		EnterCriticalSection (&eTSIMutex);

		va_start (stack, FormatStr);
		_vstprintf_s (TempMsg, TCHARS (TempMsg), FormatStr, stack);
		va_end (stack);

		_stprintf_s (OutputMsg, TCHARS (OutputMsg), _T("%hs@%d - %s"), FunctionName, LineNumber, TempMsg);


#ifdef _MSC_VER  // The following only works in MSVC/C++
#ifdef _DEBUG
		OutputDebugString (OutputMsg);
#endif
#endif
		_tprintf (_T("%s"), OutputMsg);

		LeaveCriticalSection (&eTSIMutex);

	}

	return NULL;

}

#else
#define TSITest_DbgPrintFunc(lvl,fmt,...) 
#define TSITest_GetDebugLevel				100

#endif

ThorTSI_CSTest::ThorTSI_CSTest()
{    

}

ThorTSI_CSTest::~ThorTSI_CSTest()
{
	instanceFlag = false;

	for(map<long,CameraDll*>::const_iterator it = cameraMap.begin(); it != cameraMap.end(); ++it)
	{
		((CameraDll*)it->second)->Uninitialize();

		//	wsprintf(message,L"CameraManager deleting camera object %d",it->first);
		//	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
	}

	cameraMap.clear();
}

bool ThorTSI_CSTest::instanceFlag = false;

long ThorTSI_CSTest::idCounter = 1;

auto_ptr<ThorTSI_CSTest> ThorTSI_CSTest::_single(new ThorTSI_CSTest());

CritSect ThorTSI_CSTest::critSect;

ThorTSI_CSTest* ThorTSI_CSTest::getInstance()
{
	Lock lock(critSect);

	if(! instanceFlag)
	{
		try
		{
			_single.reset(new ThorTSI_CSTest());

			TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("ThorTSI_CSTest Created\n"));

		}
		catch(...)
		{
			//critically low on resources
			//do not proceed with application
			throw;
		}
		instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

long ThorTSI_CSTest::FindCameras(const WCHAR *path, long &numCameras)
{
	Lock lock(critSect);

	WIN32_FIND_DATA ffd;

	HANDLE hFind;

	hFind = FindFirstFile(path, &ffd);

	if(INVALID_HANDLE_VALUE == hFind)
	{
		//no files found
		return FALSE;
	}

	//remove any preexisting Cameras
	ReleaseCameras();	

	idCounter = 1;
	numCameras = 0;
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{

		}
		else
		{	
			WCHAR path_buffer[_MAX_PATH];
			WCHAR drive[_MAX_DRIVE];
			WCHAR dir[_MAX_DIR];
			WCHAR fname[_MAX_FNAME];
			WCHAR ext[_MAX_EXT];

			_wsplitpath(path,drive,dir,fname,ext);

			_wmakepath(path_buffer, drive, dir, ffd.cFileName, NULL );

			//*NOTE* do not delete the camera dll memory. 
			//deletion of the dll will prevent 
			//the use of the functions exposed by the dll 
			CameraDll *camera = new CameraDll(path_buffer);

			if(NULL != camera)
			{
				if(camera->Initialize(0))
				{
					long numCam;

					if(TRUE == camera->FindCameras(numCam))
					{
						if(numCam > 0)
						{
							cameraMap.insert(pair<long,CameraDll*>(idCounter,camera));
							idCounter++;
							numCameras++;
						}
					}
				}
			}
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);


	FindClose(hFind);

	return TRUE;
}

long ThorTSI_CSTest::ReleaseCameras()
{
	//remove any preexisting cameras
	for(it = cameraMap.begin(); it != cameraMap.end(); ++it)
	{

		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("CameraManager deleting camera object %d\n"), it->first);

		it->second->TeardownCamera();
		((CameraDll*)it->second)->Uninitialize();

	}

	cameraMap.clear();
	return TRUE;
}

long ThorTSI_CSTest::GetCameraId(long i,long &id)
{
	long ret;
	//return value of id is zero if no camera is found
	id=0;
	map<long,CameraDll*>::const_iterator it;

	long size = (long) cameraMap.size();

	it = cameraMap.begin();

	if(i<0)
	{
		return FALSE;
	}

	for(long j=0;j<i;j++,it++)
	{
	}

	if(it != cameraMap.end())
	{
		id = it->first;
		ret = TRUE;
	}	
	else
	{
		ret = FALSE;
	}

	return ret;
}

ICamera* ThorTSI_CSTest::GetCamera(long id)
{
	ICamera * camera = NULL;	

	map<long,CameraDll*>::const_iterator it;

	it = cameraMap.find(id);

	if(it != cameraMap.end())
	{
		camera = it->second;

		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("CameraManager GetCamera id %d found\n"), id);

	}

	return camera;
}


char *GetParamName (ICamera* CameraToTest, ICamera::Params Param)
{

	char *ReturnValue = "UNKNOWN";

	switch (Param) {


	case CameraToTest->PARAM_MULTI_FRAME_COUNT:			ReturnValue = "PARAM_MULTI_FRAME_COUNT";		break;
	case CameraToTest->PARAM_CAMERA_TYPE:				ReturnValue = "PARAM_CAMERA_TYPE";				break;
	case CameraToTest->PARAM_BINNING_X:					ReturnValue = "PARAM_BINNING_X";				break;
	case CameraToTest->PARAM_BINNING_Y:					ReturnValue = "PARAM_BINNING_Y";				break;
	case CameraToTest->PARAM_CAPTURE_REGION_LEFT:		ReturnValue = "PARAM_CAPTURE_REGION_LEFT";		break;
	case CameraToTest->PARAM_CAPTURE_REGION_RIGHT:		ReturnValue = "PARAM_CAPTURE_REGION_RIGHT";		break;
	case CameraToTest->PARAM_CAPTURE_REGION_TOP:		ReturnValue = "PARAM_CAPTURE_REGION_TOP";		break;
	case CameraToTest->PARAM_CAPTURE_REGION_BOTTOM:		ReturnValue = "PARAM_CAPTURE_REGION_BOTTOM";	break;
	case CameraToTest->PARAM_GAIN:						ReturnValue = "PARAM_GAIN";						break;
	case CameraToTest->PARAM_EM_GAIN:					ReturnValue = "PARAM_EM_GAIN";					break;
	case CameraToTest->PARAM_TRIGGER_MODE:				ReturnValue = "PARAM_TRIGGER_MODE";				break;
	case CameraToTest->PARAM_EXPOSURE_TIME_MS:			ReturnValue = "PARAM_EXPOSURE_TIME_MS";			break;
	case CameraToTest->PARAM_PIXEL_SIZE:				ReturnValue = "PARAM_PIXEL_SIZE";				break;
	case CameraToTest->PARAM_TDI_HEIGHT:				ReturnValue = "PARAM_TDI_HEIGHT";				break;
	case CameraToTest->PARAM_LIGHT_MODE:				ReturnValue = "PARAM_LIGHT_MODE";				break;
	case CameraToTest->PARAM_READOUT_SPEED_INDEX:		ReturnValue = "PARAM_READOUT_SPEED_INDEX";		break;
	case CameraToTest->PARAM_READOUT_SPEED_VALUE:		ReturnValue = "PARAM_READOUT_SPEED_VALUE";		break;
	case CameraToTest->PARAM_OPTICAL_BLACK_LEVEL:		ReturnValue = "PARAM_OPTICAL_BLACK_LEVEL";		break;
	case CameraToTest->PARAM_COOLING_MODE:				ReturnValue = "PARAM_COOLING_MODE";				break;
	case CameraToTest->PARAM_TDI_TRIGGERS:				ReturnValue = "PARAM_TDI_TRIGGERS";				break;
	case CameraToTest->PARAM_TDI_LINESHIFTS:			ReturnValue = "PARAM_TDI_LINESHIFTS";			break; 
	case CameraToTest->PARAM_TDI_TRIM_MODE:				ReturnValue = "PARAM_TDI_TRIM_MODE";			break;
	case CameraToTest->PARAM_CONSOLE_WRITE:				ReturnValue = "PARAM_CONSOLE_WRITE";			break;
	case CameraToTest->PARAM_CONSOLE_READ:				ReturnValue = "PARAM_CONSOLE_READ";				break;
	case CameraToTest->PARAM_TDI_LINETRIM:				ReturnValue = "PARAM_TDI_LINETRIM";				break;
	case CameraToTest->PARAM_NIR_BOOST:					ReturnValue = "PARAM_NIR_BOOST";				break;
	case CameraToTest->PARAM_FRAME_RATE:				ReturnValue = "PARAM_FRAME_RATE";				break;
	case CameraToTest->PARAM_BITS_PER_PIXEL:			ReturnValue = "PARAM_BITS_PER_PIXEL";			break;
	case CameraToTest->PARAM_DETECTOR_NAME:				ReturnValue = "PARAM_DETECTOR_NAME";			break;
	case CameraToTest->PARAM_DETECTOR_SERIAL_NUMBER:	ReturnValue = "PARAM_DETECTOR_SERIAL_NUMBER";	break;

	default :
		break;

	}

	return ReturnValue;

}

char *GetParamTypeName(ICamera::ParamType paramType)
{

	char *ReturnValue = "UNKNOWN";

	switch (paramType) {

	case ICamera::TYPE_LONG :	ReturnValue = "TYPE_LONG";		break;
	case ICamera::TYPE_DOUBLE :	ReturnValue = "TYPE_DOUBLE";	break;
	case ICamera::TYPE_STRING :	ReturnValue = "TYPE_STRING";	break;
	case ICamera::TYPE_BUFFER:	ReturnValue = "TYPE_BUFFER";	break;

	}

	return ReturnValue;

}

void CameraParameterTest (ICamera* CameraToTest)
{

	//	for (long i = CameraToTest->PARAM_FIRST_CCD_PARAM; i < CameraToTest->PARAM_LAST_PARAM; i++) {
	for (long i = CameraToTest->PARAM_FIRST_PARAM; i < CameraToTest->PARAM_LAST_PARAM; i++) {

		long paramType;
		long paramAvailable;
		long paramReadOnly;
		double paramMin;
		double paramMax;
		double paramDefault;
		double paramValue;
		wchar_t paramStrValue [1024];

		if (TRUE == CameraToTest->GetParamInfo (i, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)) {

			TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("   \n\n----------------------------------------------------------------------------------------------\n\n"));

			TSITest_DbgPrintFunc(
				TSI_TEST_DEBUG_LVL, 
				_T("Parameter %8d: Name: %hs\n"),
				i,
				GetParamName (CameraToTest, (ICamera::Params) i)
				);

			TSITest_DbgPrintFunc(
				TSI_TEST_DEBUG_LVL, 
				_T("                    Type(%hs), Avail(%hs), RO/RW(%hs)\n"),
				GetParamTypeName((ICamera::ParamType) paramType),  
				((paramAvailable > 0) ? "AVAIL  " : "UNAVAIL"),
				((paramReadOnly > 0)  ? "RO" : "RW")
				);

			if (paramAvailable > 0) {

				TSITest_DbgPrintFunc(
					TSI_TEST_DEBUG_LVL, 
					_T("                    Min: %f, Max %f, Def %f\n"),
					paramMin,
					paramMax,
					paramDefault
					);

				switch (paramType) {

				case ICamera::TYPE_LONG :
				case ICamera::TYPE_DOUBLE :
					if (CameraToTest->GetParam (i, paramValue)) {
						switch (paramType) {
						case ICamera::TYPE_LONG :
							TSITest_DbgPrintFunc(
								TSI_TEST_DEBUG_LVL, 
								_T("                    Val: %d\n"),
								(long) paramValue
								);
							break;
						case ICamera::TYPE_DOUBLE :
							TSITest_DbgPrintFunc(
								TSI_TEST_DEBUG_LVL, 
								_T("                    Val: %f\n"),
								paramValue
								);
							break;
						}
					}
					break;

				case ICamera::TYPE_STRING :
					if (CameraToTest->GetParamString (i, paramStrValue, sizeof (paramStrValue))) {
						TSITest_DbgPrintFunc(
							TSI_TEST_DEBUG_LVL, 
							_T("                    Val: %s\n"),
							paramStrValue
							);
					} else {
						TSITest_DbgPrintFunc(
							TSI_TEST_DEBUG_LVL, 
							_T("                    Val: %s\n"),
							_T("ERROR: Could not retrieve value")
							);
					}
					break;

				case ICamera::TYPE_BUFFER :
					TSITest_DbgPrintFunc(
						TSI_TEST_DEBUG_LVL, 
						_T("                    Buf: %s\n"),
						_T("UNSUPPORTED")
						);
					break;

				}

			}

			TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("\n"));
			TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("   \n\n----------------------------------------------------------------------------------------------\n\n"));

		} else { 
			//			TSITest_DbgPrintFunc(TSI_TEST_ERROR_LVL, _T("CameraToTest->GetParamInfo (%d) failed\n"), i);
		}

	}

}

bool CameraGetParam (ICamera* CameraToTest, const long paramID, long &param)
{

	bool Result;

	double paramDouble;

	Result = (CameraToTest->GetParam (paramID, paramDouble) > 0);
	if (Result) {
		param = (long) paramDouble;
	}

	return Result;

}

//==============================================================================
// save_images_to_disk - saves the supplied images to disk
//------------------------------------------------------------------------------
//==============================================================================
bool save_images_to_disk (char *base_file_name, size_t image_size, long number_of_images)
{

	bool Result							= true;
	char output_file_name [MAX_PATH]	= "";
	FILE *output_file					= NULL;
	long images_left					= number_of_images;

	for (long image_number = 0; image_number < number_of_images; image_number++) {

		char *image_data = (char *) BufferImagePointers [image_number];

		if (NULL == image_data) {
			break;
		}

		switch (image_format) {

		case IMAGE_FORMAT_RAW :

			if (image_number == 0) {

				sprintf (output_file_name, "%s_%d_imgs.raw", base_file_name, number_of_images);

				output_file = fopen (output_file_name, "wb");

				if (NULL == output_file) {
					printf ("Save Error (%u): Opening %s\n", errno, output_file_name);
				}

			}

			if (output_file) {

				size_t BytesToWrite = image_size;
				size_t BytesToFill  = 0;

				printf ("Saving image %d - size in bytes (%u)\n", image_number, image_size);

				fwrite (image_data, 1, BytesToWrite, output_file);

				image_data += image_size;
				images_left--;

			}

			if (images_left == 0) {

				printf ("%u images saved to %s\n", image_number + 1, output_file_name);

				if (output_file) {
					fclose (output_file);
				}

			}

			break;
#if 0
		case IMAGE_FORMAT_PNG :
		case IMAGE_FORMAT_TIFF :
			{

				bool Result;

				images_left--;

				if (IMAGE_FORMAT_PNG == image_format) {
					sprintf (output_file_name, "%s_%d.png", base_file_name, image_number);
					Result = tsi_util->WriteImageToPNG  (output_file_name, image);
				} else {
					sprintf (output_file_name, "%s_%d.tiff", base_file_name, image_number);
					Result = tsi_util->WriteImageToTIFF (output_file_name, image);
				}

				if (Result) {
					printf ("Saved image %d - size in bytes (%u) to %s\n", image_number, image_size, output_file_name);
				} else {
					printf ("*** ERROR *** - Saving image %d - size in bytes (%u) to %s\n", image_number, image_size, output_file_name);
				}

			}

			break;
#endif
		default :
			image_number = number_of_images;
			break;

		}

	}

	return Result;

}

//==============================================================================
// destroy_buffer_heap - destroy image buffer heap used to buffer images 
//------------------------------------------------------------------------------
//==============================================================================
void destroy_buffer_heap (char *CallingFunction, int CallingLine)
{

	//	printf ("%s @ %d - BufferHeap(%p), BufferImagePointers(%p)\n", CallingFunction, CallingLine, BufferHeap, BufferImagePointers);

	if (NULL != BufferHeap) {

		images_to_buffer	= 0;
		images_buffered		= 0;

		HeapDestroy (BufferHeap);
		BufferHeap = NULL;

	}

	if (NULL != BufferImagePointers) {

		images_to_buffer	= 0;
		images_buffered		= 0;

		free (BufferImagePointers);
		BufferImagePointers = NULL;

	}

}

//==============================================================================
// create_buffer_heap - create image buffer heap used to buffer images 
//------------------------------------------------------------------------------
//==============================================================================
bool create_buffer_heap (size_t expected_image_size_in_bytes)
{

	bool Result = true;

	destroy_buffer_heap (__FUNCTION__, __LINE__);

	//	printf ("expected_image_size_in_bytes(%u) * images_to_buffer(%d) == %d\n", expected_image_size_in_bytes, images_to_buffer, expected_image_size_in_bytes * images_to_buffer);

	printf ("Buffering %d images - %I64u bytes required\n", images_to_buffer, (SIZE_T) ((SIZE_T) expected_image_size_in_bytes * (SIZE_T) images_to_buffer));
	BufferHeap = HeapCreate (HEAP_NO_SERIALIZE, (SIZE_T) ((SIZE_T) expected_image_size_in_bytes * (SIZE_T) images_to_buffer), 0); //expected_image_size_in_bytes * images_to_buffer);

	if (NULL == BufferHeap) {

		Result = false;
		printf ("Unable to allocate heap for image buffer - error (0x%08x)\n", GetLastError ());

	} else {

		BufferImagePointers = (VOID **) malloc (sizeof (VOID *) * images_to_buffer);

		if (NULL == BufferImagePointers) {

			Result = false;
			printf ("Unable to allocate memory for image buffer heap pointers\n");

		} else {

			for (int i = 0; i < images_to_buffer; i++) {
				BufferImagePointers [i] = NULL;
			}

			for (int i = 0; i < images_to_buffer; i++) {

				BufferImagePointers [i] = HeapAlloc (BufferHeap, HEAP_ZERO_MEMORY, expected_image_size_in_bytes);

				if (NULL == BufferImagePointers [i]) {
					Result = false;
					printf ("Unable to allocate memory for image buffer heap pointers - error (0x%08x)\n", GetLastError ());
				}

			}

		}
	}

	if (!Result) {
		destroy_buffer_heap (__FUNCTION__, __LINE__);
	}

	return Result;

}


bool CameraImageSetup (ICamera* CameraToTest, IMAGING_TEST &Test)
{

	bool Result		 = false;

	if (!CameraToTest->SetParam (ICamera::PARAM_OP_MODE, (const double) Test.OpMode)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Set ICamera::PARAM_OP_MODE failed\n"));
		goto Exit;
	}

	if (!CameraToTest->SetParam (ICamera::PARAM_TRIGGER_MODE , (const double) Test.TrigMode)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Set ICamera::PARAM_TRIGGER_MODE failed\n"));
		goto Exit;
	}

	if (!CameraToTest->SetParam (ICamera::PARAM_CAPTURE_REGION_LEFT, (const double) Test.Left)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Set ICamera::PARAM_CAPTURE_REGION_LEFT failed\n"));
		goto Exit;
	}

	if (!CameraToTest->SetParam (ICamera::PARAM_CAPTURE_REGION_TOP, (const double) Test.Top)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Set ICamera::PARAM_CAPTURE_REGION_TOP failed\n"));
		goto Exit;
	}

	if (!CameraToTest->SetParam (ICamera::PARAM_CAPTURE_REGION_RIGHT, (const double) Test.Right)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Set ICamera::PARAM_CAPTURE_REGION_RIGHT failed\n"));
		goto Exit;
	}

	if (!CameraToTest->SetParam (ICamera::PARAM_CAPTURE_REGION_BOTTOM, (const double) Test.Bottom)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Set ICamera::PARAM_CAPTURE_REGION_BOTTOM failed\n"));
		goto Exit;
	}

	if (!CameraToTest->SetParam (ICamera::PARAM_BINNING_X, (const double) Test.XBin)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Set ICamera::PARAM_BINNING_X failed\n"));
		goto Exit;
	}

	if (!CameraToTest->SetParam (ICamera::PARAM_BINNING_Y, (const double) Test.YBin)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Set ICamera::PARAM_BINNING_Y failed\n"));
		goto Exit;
	}

	if (Test.OpMode != 1) {
		if (!CameraToTest->SetParam (ICamera::PARAM_EXPOSURE_TIME_MS, (const double) Test.ExposureTime)) {
			TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Set ICamera::PARAM_EXPOSURE_TIME_MS failed\n"));
			goto Exit;
		}
	}

	if (!CameraToTest->SetParam (ICamera::PARAM_MULTI_FRAME_COUNT, (const double) Test.NumberOfFrames)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Set ICamera::PARAM_MULTI_FRAME_COUNT failed\n"));
		goto Exit;
	}

	Result = true;

Exit:

	return Result;

}

bool CameraImage (ICamera* CameraToTest, IMAGING_TEST &Test) 
{

	bool			Result				= false;

	long			OpMode				= 0;
	long			NumberOfImages		= 0;
	long			BytesPerPixel		= 0;
	long			XOrigin				= 0;
	long			YOrigin				= 0;
	long			XPixels				= 0;
	long			YPixels				= 0;
	long			XBin				= 0;
	long			YBin				= 0;

	size_t			image_size_in_bytes = 0;

	long			status				= ICamera::STATUS_BUSY;
	char			*pImageData			= NULL;

	DWORD			StartTime			= GetTickCount ();
	DWORD			CurrentTime;

	if (!CameraGetParam (CameraToTest, ICamera::PARAM_OP_MODE, OpMode)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Get ICamera::PARAM_OP_MODE failed\n"));
		goto Exit;
	}

	if (!CameraGetParam (CameraToTest, ICamera::PARAM_MULTI_FRAME_COUNT, NumberOfImages)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Get ICamera::PARAM_MULTI_FRAME_COUNT failed\n"));
		goto Exit;
	} else {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Get ICamera::PARAM_MULTI_FRAME_COUNT returned (%d)\n"), NumberOfImages);
	}

#if 0
	if (!CameraGetParam (CameraToTest, ICamera::param, BytesPerPixel)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Get ICamera::PARAM_MULTI_FRAME_COUNT failed\n"));
		goto Exit;
	}
#else
	BytesPerPixel = 2;
#endif

	if (!CameraGetParam (CameraToTest, ICamera::PARAM_CAPTURE_REGION_LEFT, XOrigin)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Get ICamera::PARAM_CAPTURE_REGION_LEFT failed\n"));
		goto Exit;
	}

	if (!CameraGetParam (CameraToTest, ICamera::PARAM_CAPTURE_REGION_TOP, YOrigin)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Get ICamera::PARAM_CAPTURE_REGION_TOP failed\n"));
		goto Exit;
	}

	if (!CameraGetParam (CameraToTest, ICamera::PARAM_CAPTURE_REGION_RIGHT, XPixels)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Get ICamera::PARAM_CAPTURE_REGION_RIGHT failed\n"));
		goto Exit;
	}

	if (!CameraGetParam (CameraToTest, ICamera::PARAM_CAPTURE_REGION_BOTTOM, YPixels)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Get ICamera::PARAM_CAPTURE_REGION_BOTTOM failed\n"));
		goto Exit;
	}

	if (!CameraGetParam (CameraToTest, ICamera::PARAM_BINNING_X, XBin)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Get ICamera::PARAM_BINNING_X failed\n"));
		goto Exit;
	}

	if (!CameraGetParam (CameraToTest, ICamera::PARAM_BINNING_Y, YBin)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   Get ICamera::PARAM_BINNING_Y failed\n"));
		goto Exit;
	}

	if (0 == XBin) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   ICamera::PARAM_BINNING_X returned zero\n"));
		goto Exit;
	}

	if (0 == YBin) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   ICamera::PARAM_BINNING_Y returned zero\n"));
		goto Exit;
	}

	image_size_in_bytes = ((XPixels / XBin) * (YPixels / YBin) * BytesPerPixel);
	images_to_buffer    = Test.TotalFrames;

	if (!create_buffer_heap (image_size_in_bytes)) {
		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("   create_buffer_heap failed\n"));
		goto Exit;
	}

	CameraToTest->PreflightAcquisition (NULL);
	CameraToTest->SetupAcquisition (NULL);
	CameraToTest->StartAcquisition (NULL);

	Result = true;

	long i = 0;

	while (true) {

		int ch = 0;

		do {

			CameraToTest->StatusAcquisition (status);

			if (status != ICamera::STATUS_READY) {
				if (_kbhit ()) {
					ch = _getch ();
				} else {
					Sleep (1);
				}
			}

		} while ((status != ICamera::STATUS_READY) && (ch != CH_ESC));

		if (CH_ESC == ch) {
			Result = false;
			break;
		}

		CameraToTest->CopyAcquisition ((char *) BufferImagePointers [i], NULL);

		CurrentTime	= GetTickCount ();
		if (0 == i) {
			StartTime = CurrentTime;
		}

		i++;
		if (Test.TotalFrames > 0) {
			if (i >= Test.TotalFrames) {
				break;
			}
		}

		DWORD ElapsedTime = ((CurrentTime >= StartTime) ? (CurrentTime - StartTime) : (StartTime - CurrentTime));

		TSITest_DbgPrintFunc(
			TSI_TEST_DEBUG_LVL,
			_T("   Got %d images in %5.2fs - fps %5.2f\r"), 
			i,
			(double) ElapsedTime / 1000.0, 
			((double) ElapsedTime / (double) i) / 1000.0
			);

		if (ICamera::SW_SINGLE_FRAME == Test.TrigMode) {
			CameraToTest->StartAcquisition (NULL);
		}

	}

	CameraToTest->PostflightAcquisition (NULL);

	TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL,_T("\n\n   Done\n\n"));

	if (Result) {
		save_images_to_disk (Test.Desc, image_size_in_bytes, Test.TotalFrames);
	}

Exit :

	destroy_buffer_heap (__FUNCTION__, __LINE__);

	return Result;

}

char *TriggerModeName (long TrigMode)
{

	char *RetVal = "UKNW";

	switch (TrigMode) {
	case ICamera::SW_SINGLE_FRAME :					RetVal = "SWSF"; break;
	case ICamera::SW_MULTI_FRAME :					RetVal = "SWMF"; break;
	case ICamera::SW_FREE_RUN_MODE :				RetVal = "SWFR"; break;
	case ICamera::HW_SINGLE_FRAME :					RetVal = "HWSF"; break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST :	RetVal = "HWTF"; break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH :		RetVal = "HWTE"; break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH_BULB :RetVal = "HBLB"; break;
	case ICamera::HW_TDI_TRIGGER_MODE :				RetVal = "HTDI"; break;
	}

	return RetVal;

}

char *OpModeName (long TrigMode)
{

	char *RetVal = "UKN";

	switch (TrigMode) {
	case 0 :	RetVal = "NRM"; break;
	case 1 :	RetVal = "PDX"; break;
	case 2 :	RetVal = "TOE"; break;
	case 3 :	RetVal = "TDI"; break;
	}

	return RetVal;

}

#define MAX_TEST_STR_LEN		3

void CameraImagingTest (ICamera* CameraToTest)
{

	bool	Done				= false;
	bool	Prompt				= false;
	wint_t	ch					= '?';
	int		MaxTests			= 0;
	char	TestNumberStr[MAX_TEST_STR_LEN + 1]	= {0};
	int		TestNumberStrPos	= 0;
	long	TestNumber			= (ch - L'1');

	IMAGING_TEST ImagingTests [] = 
	{
		//               T#  Exp	TF    #F  Left   Right  Top   Bot  XBin  YBin   Trigger Mode              OpMode
		IMAGING_TEST_DEF(1,    5,    1,    1,    0,  1392,    0, 1040,    1,    1,	ICamera::SW_MULTI_FRAME,	0),
		IMAGING_TEST_DEF(2,    5,    5,    5,    0,  1392,    0, 1040,    1,    1,	ICamera::SW_MULTI_FRAME,	0),
		IMAGING_TEST_DEF(3,    5,    1,    1,  256,  1136,    0, 1040,    1,    1,	ICamera::SW_MULTI_FRAME,	0),
		IMAGING_TEST_DEF(4,    2,    1,    1,    0,  1392,    0, 1040,    2,    2,	ICamera::SW_MULTI_FRAME,	0),
		IMAGING_TEST_DEF(5,    1,    1,    1,    0,  1392,    0, 1040,    4,    4,	ICamera::SW_SINGLE_FRAME,	0),
		IMAGING_TEST_DEF(6,    1,    1,    1,    0,  1392,    0, 1040,   10,   10,	ICamera::SW_SINGLE_FRAME,	0),
		IMAGING_TEST_DEF(7,    1,    9,    1,    0,  1392,    0, 1040,   10,   10,	ICamera::SW_SINGLE_FRAME,	0),
		IMAGING_TEST_DEF(8,    1,   99,    1,    0,  1392,    0, 1040,   10,   10,	ICamera::SW_SINGLE_FRAME,	0),
		IMAGING_TEST_DEF(9,    1,  999,    1,    0,  1392,    0, 1040,    1,    1,	ICamera::SW_SINGLE_FRAME,	0),
		IMAGING_TEST_DEF(10,   5,   30,    1,    0,  1392,    0, 1040,    1,    1,	ICamera::HW_SINGLE_FRAME,	1),
		IMAGING_TEST_DEF(11,   5,   30,    0,    0,  1392,    0, 1040,    1,    1,	ICamera::HW_SINGLE_FRAME,	1),
		IMAGING_TEST_DEF(12,   0,    0,    0,    0,     0,    0,    0,    0,    0,	0,							0)
	};

	while (!Done) {

		bool Result   = true;

		Prompt = false;

		switch (ch) {

		case CH_ESC :

			Done   = true;

			_cputws (L"\n\n");

			break;

		case CH_BKSP :

			if (TestNumberStrPos > 0) {

				_putwch (ch);
				_putwch (L' ');
				_putwch (ch);

				TestNumberStrPos--;
				TestNumberStr[TestNumberStrPos]	= 0;

			} else {
				Beep (2000, 500);
			}

			break;

		case '?' :

			_cputws (L"\n");
			_cputws (L"===============================================================================================================\n");
			_cputws (L"                                       Imaging Test Menu                                                       \n");
			_cputws (L"===============================================================================================================\n");
			_cputws (L"T#     Exp   #TF    #F  Left Right   Top   Bot  XBin  YBin     T    O -             Output File Name            \n");
			_cputws (L"===============================================================================================================\n");

			{

				wchar_t	output_buffer [256] = {0};
				int		i					= 0;

				while (ImagingTests [i].TotalFrames > 0) {

					swprintf_s (
						output_buffer, 
						sizeof (output_buffer) / sizeof (wchar_t), 
						L"%2d - %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4hs, %3hs - %hs_%d_imgs.raw\n",
						ImagingTests [i].TestNumber,
						ImagingTests [i].ExposureTime,
						ImagingTests [i].TotalFrames,
						ImagingTests [i].NumberOfFrames,
						ImagingTests [i].Left,
						ImagingTests [i].Right,
						ImagingTests [i].Top,
						ImagingTests [i].Bottom,
						ImagingTests [i].XBin,
						ImagingTests [i].YBin,
						TriggerModeName (ImagingTests [i].TrigMode),
						OpModeName      (ImagingTests [i].OpMode),
						ImagingTests [i].Desc,
						ImagingTests [i].TotalFrames
						);

					_cputws (output_buffer);

					MaxTests++;
					i++;

				}

				MaxTests = i;

			}

			_cputws (L"======================================================================================\n");

			Prompt = true;

			break;

		case CH_ENTER :

			if (TestNumberStrPos > 0) {

				TestNumber = atol (TestNumberStr);
				if (TestNumber > 0) {
					TestNumber--;
				}

				if (TestNumber < MaxTests) {

					wchar_t	output_buffer [256] = {0};

					swprintf_s (
						output_buffer, 
						sizeof (output_buffer) / sizeof (wchar_t), 
						L"\n\nPerforming Test #%d - %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %1d, %1d - Image Data in %hs_%d_imgs.raw\n\n",
						ImagingTests [TestNumber].TestNumber,
						ImagingTests [TestNumber].ExposureTime,
						ImagingTests [TestNumber].TotalFrames,
						ImagingTests [TestNumber].NumberOfFrames,
						ImagingTests [TestNumber].Left,
						ImagingTests [TestNumber].Right,
						ImagingTests [TestNumber].Top,
						ImagingTests [TestNumber].Bottom,
						ImagingTests [TestNumber].XBin,
						ImagingTests [TestNumber].YBin,
						ImagingTests [TestNumber].TrigMode,
						ImagingTests [TestNumber].OpMode,
						ImagingTests [TestNumber].Desc,
						ImagingTests [TestNumber].TotalFrames
						);

					_cputws (output_buffer);

					Result = CameraImageSetup (CameraToTest, ImagingTests [TestNumber]);

					if (Result) {
						Result = CameraImage (CameraToTest, ImagingTests [TestNumber]);
					}

					if (Result) {
						_cputws( L"\n\nTest Passed...");
					} else {
						_cputws( L"\n\nTest Failed...");
					}

					_cputws( L"\n\n");

				} else {

					Beep (2000, 500);

				}

			}

			Prompt = true;

			break;

		default :

			if ((ch >= L'0') && (ch <= L'9')) {

				if (TestNumberStrPos < MAX_TEST_STR_LEN) {

					_putwch (ch);

					TestNumberStr [TestNumberStrPos++] = (char) ch;
					TestNumberStr [TestNumberStrPos] = 0;

				} else {
					Beep (2000, 500);
				}

			}

			break;

		}

		if (!Done) {

			if (Prompt) {

				wchar_t prompt [256] = {0};

				swprintf_s (
					prompt,
					sizeof (prompt) / sizeof (wchar_t),
					L"\n\nCamera Imaging Test (?, 1..%d) - Press Esc key to exit test >",
					MaxTests
					);

				_cputws (prompt);

				memset (TestNumberStr, 0, sizeof (TestNumberStr));
				TestNumberStrPos = 0;

			}

			ch = _getwch ();

		}

	}

	return;

}

void CameraConsoleTest (ICamera* CameraToTest)
{

	wchar_t Command [1024];
	int     CommandPos		= 0;

	wchar_t Response [1024];

	bool	Done			= false;
	bool	DisplayPrompt	= true;

	while (!Done) {

		wint_t ch;

		if (DisplayPrompt) {
			DisplayPrompt = false;
			_cputws( L"\nCamera Console - Enter Command, Esc key to exit console >" );
		}

		if (_kbhit()) {

			ch = _getwch ();

			if (CH_ESC == ch) {

				Done = true;

			} else if (CH_BKSP == ch) {

				if (CommandPos > 0) {

					Command [--CommandPos] = 0;

					_putwch (ch);
					_putwch ((wchar_t) ' ');
					_putwch (ch);

				}

			} else if (0x000D == ch) {

				if (_wcsnicmp(L"exit", Command, 4) == 0) {

					Done = true;

				} else if (_wcsnicmp(L"quit", Command, 4) == 0) {

					Done = true;

				} else {

					if (CommandPos < 1024) {

						Command [CommandPos++] = ch;
						_putwch (ch);
						_putwch ((wchar_t) 0x0A);

					}

					if (CommandPos < 1024) {
						Command [CommandPos++] = 0;
						CameraToTest->SetParamString (ICamera::PARAM_CONSOLE_WRITE, Command);
					}

				}

				CommandPos = 0;

			} else {

				if (CommandPos < 1024) {
					if ((ch >= (wint_t) ' ') && (ch <= (wint_t) '~')) {

						Command [CommandPos++] = ch;

						_putwch (ch);

					}
				}

			}

		} else if (CameraToTest->GetParamString (ICamera::PARAM_CONSOLE_READ, Response, 1023)) {

			wchar_t *p = Response;
			wchar_t ch;

			while (*p) {

				ch = *p;
				_putwch (*p);

				p++;

				if ((ch == 0x000D) && (*p != 0x000A))  {
					_putwch (0x000A);
				}

			}

			DisplayPrompt = true;

		} else {

			Sleep (100);

		}

	}

}

void CameraTestMenu (ICamera* CameraToTest)
{

	wchar_t Response [1024];

	bool	Done			= false;

	_cputws( L"\nCamera Test - Testing Camera: " );

	CameraToTest->GetParamString (ICamera::PARAM_DETECTOR_NAME, Response, 1024);
	_cputws( Response );

	_cputws( L" - " );

	CameraToTest->GetParamString (ICamera::PARAM_DETECTOR_SERIAL_NUMBER, Response, 1024);
	_cputws( Response );

	_cputws( L"\n" );

	while (!Done) {

		wint_t ch;

		_cputws( L"\nThorTSI_CS Test - C)onsole, P)arameter, I)mage, Esc key to exit program >" );

		do {

			ch = toupper (_getwch ());

		} while (NULL == wcschr (L"CPI\x1B", ch));

		if (CH_ESC == ch) {

			Done = true;

		} else {

			_putwch (ch);
			_cputws( L"\n");

			switch (ch) {

			case L'P'	: CameraParameterTest (CameraToTest);		break;
			case L'I'	: CameraImagingTest   (CameraToTest);		break;
			case L'C'	: CameraConsoleTest   (CameraToTest);		break;
			default		:											break;

			}

		}

	}

}

int main (int argv, char *argc [], char *env)
{
	long numCameraTypes;
	long CameraID;
	long Result;

	wchar_t *DLL_Path = NULL;

	ThorTSI_CSTest* Tester = ThorTSI_CSTest::getInstance();

	if (NULL == Tester) {
		TSITest_DbgPrintFunc(TSI_TEST_ERROR_LVL, _T("Unable to create Tester object\n"));
		goto Exit;
	}

	for (int i = 0; i < 10; i++) {

		switch (i) {

		case 0 :
#ifdef _WIN64 
			DLL_Path = L"ThorTSI_CS.dll";
#else
			DLL_Path = L"..\\Debug\\ThorTSI_CS.dll";
#endif
			break;

		case 1 :
			DLL_Path = L".\\Modules_Native\\ThorTSI_CS.dll";
			break;

		default :
			DLL_Path = NULL;
			break;

		}

		if (NULL == DLL_Path) {
			break;
		}

		Result = Tester->FindCameras(DLL_Path, numCameraTypes);
		if (FALSE != Result) {
			TSITest_DbgPrintFunc(TSI_TEST_ERROR_LVL, _T("Found Camera DLL at %s\n"), DLL_Path);
			break;
		}

	}

	if (FALSE == Result) {
		TSITest_DbgPrintFunc(TSI_TEST_ERROR_LVL, _T("Could not find Camera DLL\n"));
		goto Exit;
	}

	char CurrentDir [MAX_PATH];
	_getcwd (CurrentDir, MAX_PATH);

	TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("Current Directory: %hs\n"), CurrentDir);
	TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("Number of camera types %d\n"), numCameraTypes);

	for (int CameraType = 0; CameraType < numCameraTypes; CameraType++ ) {

		TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("Getting Camera ID for camera type %d\n"), CameraType);

		if (TRUE == Tester->GetCameraId (CameraType, CameraID)) {

			TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("Getting Camera object for camera id %d\n"), CameraID);

			ICamera* CameraToTest = Tester->GetCamera (CameraID);

			if (NULL != CameraToTest) {

				long CameraCount;

				TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("Got Camera object - ready to select\n"));

				CameraToTest->FindCameras (CameraCount);

				for (long i = 0; i < CameraCount; i++) {

					if (TRUE == CameraToTest->SelectCamera (i)) {

						TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("\n\n====================================================================================================\n\n"));

						TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("Camera (%u) Selected - Ready to Test\n"), i);

						CameraTestMenu (CameraToTest);

						TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("\n\n====================================================================================================\n\n"));

					} else {

						TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("Select Camera(%u) Failed\n"), i);

					}

				}

			} else {

				TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("CameraToTest == NULL\n"));

			}

		}

	}

	TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("calling Tester->ReleaseCameras()\n"));
	Tester->ReleaseCameras();
	TSITest_DbgPrintFunc(TSI_TEST_DEBUG_LVL, _T("returned from Tester->ReleaseCameras()\n"));

Exit :

	return 0;

}