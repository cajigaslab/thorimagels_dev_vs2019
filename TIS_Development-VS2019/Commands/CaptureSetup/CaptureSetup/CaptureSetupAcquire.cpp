#include "stdafx.h"
#include "math.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"

//camera 0 thread proc
UINT StatusThreadProc0( LPVOID pParam )
{
	ICamera* iCam = (ICamera*)pParam;

	long status = ICamera::STATUS_BUSY;

	//limit update of partial frames in [msec]
	const LONGLONG PARTIAL_UPDATE_RATE = 30;
	LARGE_INTEGER freq;		// ticks per second
	LARGE_INTEGER t1, t2;   // ticks
	LONGLONG elapsedTime;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&t1);

	while((FALSE == stopCapture) && (ICamera::STATUS_BUSY == status))
	{
		//get status
		if(FALSE == iCam->StatusAcquisition(status))
		{
			break;
		}
		//check status
		if(ICamera::STATUS_ERROR == status)
		{
			CaptureSetup::getInstance()->_statusError[0] = TRUE;
			break;
		}
		else if (ICamera::STATUS_READY == status)
		{
			break;
		}
		else if ((ICamera::STATUS_PARTIAL == status) && (FALSE == stopCapture))
		{
			//compute elapsed time in [msec]
			QueryPerformanceCounter(&t2);
			elapsedTime = (t2.QuadPart - t1.QuadPart) * Constants::MS_TO_SEC / freq.QuadPart;
			if (PARTIAL_UPDATE_RATE <= elapsedTime)
			{
				CHECK_INLINE_PACTIVEIMAGEROUTINE(CopyAcquisition(FALSE));
				t1 = t2;
			}
			//wait until next section started
			while(ICamera::STATUS_PARTIAL == status)
			{
				Sleep(10);
				if((TRUE == stopCapture) || (FALSE == iCam->StatusAcquisition(status)))
				{
					break;
				}
			}
		}
	}
	SetEvent( hStatusEvent[0]);

	return 0;
}

/// <summary> Get the corresponding channel letter for each channel, following a simple ABC => 012 scheme </summary>
/// <param name="channel"> The channel to get a letter for </param>
/// <returns> Char letter representing the channel input </returns>
wchar_t channelLetter(int channel)
{
	return channel + 'A';
}

/// <summary> Get the count of channels in bit-wise mode, following ...CBA => 0...111 scheme </summary>
/// <param name="lsmChan"> Bit-wise format of lsm channels </param>
/// <param name="enabledChannels"> Vector of enabled channels </param>
/// <param name="chanNames"> Vector of enabled channel names </param>
/// <param name="basename"> The name goes before channel characters </param>
/// <returns> Total count of enabled channels </returns>
long ParseLSMChannels(int lsmChan, vector<int>* enabledChannels, vector<wstring>* chanNames, wstring basename=L"")
{
	long count=0;
	long bitComp = 0x1;

	for(long i=0; i<32; i++)
	{
		if(0 == lsmChan)
			break;
		if(1 == ((lsmChan) & bitComp))
		{
			if(NULL != enabledChannels)
				enabledChannels->push_back(i);
			if(NULL != chanNames)
				chanNames->push_back(basename+channelLetter(i));
			count++;
		}
		lsmChan = lsmChan >> 1;
	}
	return count;
}

long PreflightAcquisition()
{
	const long NORMAL_OPERATING_MODE = 0;
	long successTrigger = GetCamera(SelectedHardware::SELECTED_CAMERA1)->SetParam(ICamera::PARAM_TRIGGER_MODE,ICamera::SW_FREE_RUN_MODE);
	long successMode = GetCamera(SelectedHardware::SELECTED_CAMERA1)->SetParam(ICamera::PARAM_OP_MODE, NORMAL_OPERATING_MODE);

	if(successTrigger != TRUE && successMode != TRUE)
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup PreflightAcquisition failed");
		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
	}

	if(FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->PreflightAcquisition(pChan[0]))
		return FALSE;

	return TRUE;
}

void PreflightPMT()
{
	SetPMTScannerEnable(TRUE);
}

void PostflightPMT()
{	
	SetPMTScannerEnable(FALSE);
}

long SetupBuffers(long colorChans)
{
	imageInfo.channels = colorChans;
	const int HISTOGRAM_SIZE_BYTES = 256 * sizeof(ULONG);
	const int DFLIM_BUFFER_CNT = 4;

	//limited image size and reset buffer if size is different
	//check param PARAM_DFLIM_ACQUISITION_MODE For dflim capability:
	//[dflim] 1 datalength for photon num buffer (intensity) (USHORT)
	//[dflim] 1 datalength for single photon sum buffer (USHORT)
	//[dflim] 2 datalength for arrival time sum buffer (UINT32)
	//[REMARK]: image could be large from RGG, limit size below 2GB for bitmap display
	long dflimMode = 0, dflimCapable = (TRUE == GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_DFLIM_ACQUISITION_MODE,dflimMode)) ? TRUE : FALSE;
	unsigned long long bufferSize = (TRUE == dflimCapable) ? 
		((sizeof(unsigned short) * imageInfo.imageWidth * imageInfo.imageHeight * DFLIM_BUFFER_CNT  + HISTOGRAM_SIZE_BYTES) * imageInfo.channels) :
	(sizeof(unsigned short) * imageInfo.imageWidth * imageInfo.imageHeight * imageInfo.channels);

	if (MAX_IMAGE_SIZE < bufferSize)
	{
		imageInfo.imageWidth = imageInfo.imageHeight = (TRUE == dflimCapable) ? 
			static_cast<long>(sqrt(((MAX_IMAGE_SIZE / imageInfo.channels) - HISTOGRAM_SIZE_BYTES) / sizeof(unsigned short) / DFLIM_BUFFER_CNT)) : 
		static_cast<long>(sqrt(MAX_IMAGE_SIZE / sizeof(unsigned short) / imageInfo.channels));

		bufferSize = (TRUE == dflimCapable) ? 
			((sizeof(unsigned short) * imageInfo.imageWidth * imageInfo.imageHeight * DFLIM_BUFFER_CNT  + HISTOGRAM_SIZE_BYTES) * imageInfo.channels) :
		(sizeof(unsigned short) * imageInfo.imageWidth * imageInfo.imageHeight * imageInfo.channels);
	}

	if (TRUE == dflimCapable)
	{
		if (1 == dflimMode)
		{
			imageInfo.bufferType = BufferType::DFLIM_DIAGNOSTIC;
		}
		else
		{
			imageInfo.bufferType = BufferType::DFLIM_IMAGE;
		}		
	}
	else
	{
		imageInfo.bufferType = BufferType::INTENSITY;
	}

	if (imageBufferSize != bufferSize)
	{
		SAFE_DELETE_ARRAY(pMemoryBuffer);

		imageBufferSize = bufferSize;
		pMemoryBuffer = new char[imageBufferSize];

		if (NULL == pMemoryBuffer)
		{
			StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pMemoryBuffer malloc failed", __FUNCTION__, __LINE__);
			logDll->TLTraceEvent(ERROR_EVENT,1,message);
			imageBufferSize = 0;
			return FALSE;
		}
	}
	memset(pMemoryBuffer, 0x0, imageBufferSize);

	pChan[0] = pMemoryBuffer;
	return TRUE;
}

void WaitForAcquisition()
{
	long ret = 0;

	hStatusEvent[0] = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThreadId;

	HANDLE hThread0;

	hThread0 = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc0, (LPVOID)GetCamera(SelectedHardware::SELECTED_CAMERA1), 0, &dwThreadId );

	DWORD dwWait = WaitForMultipleObjects((DWORD) 1, hStatusEvent, TRUE, INFINITE );

	if(dwWait != (WAIT_OBJECT_0))
	{
		logDll->TLTraceEvent(WARNING_EVENT,1,L"CaptureSetup WaitForMultipleObjects failed");
	}

	SAFE_DELETE_HANDLE(hThread0);
	SAFE_DELETE_HANDLE(hStatusEvent[0]);

	if(CaptureSetup::getInstance()->_statusError[0])
	{
		stopCapture = TRUE;
		CaptureSetup::getInstance()->_statusError[0] = FALSE;
	}
}

//#define OUTPUT_PERFORMANCE

UINT LiveThreadProc( LPVOID pParam )
{
	Lock(CaptureSetup::getInstance()->critSectLiveImage);

#ifdef OUTPUT_PERFORMANCE
	long frameCount = 0;
#endif 

	CaptureSetup* instancePtr = CaptureSetup::getInstance();

	//store the shutter object so that it can be refrenced independent
	//of modality changes
	IDevice* pShutter = GetDevice(SelectedHardware::SELECTED_SHUTTER1);

	if(FALSE == SetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_MULTI_FRAME_COUNT,MAXINT32))
	{
		StringCbPrintfW(message,MSG_SIZE,L"CaptureSetup CaptureSingleImage SetParam PARAM_MULTI_FRAME_COUNT failed");
		logDll->TLTraceEvent(WARNING_EVENT,1,message);
		goto RETURN;
	}

	long camType = ICamera::CameraType::LAST_CAMERA_TYPE;
	camType = (GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_TYPE,camType)) ? camType : ICamera::CameraType::LAST_CAMERA_TYPE;

	//return if falied preflight
	if (FALSE == PreflightAcquisition())
		goto RETURN;

	PreflightPMT();

	//open shutter
	SetShutterPosition(SHUTTER_OPEN);

	//enable LEDs
	SetBFLampPosition(ENABLE_LEDS);


	//Enable laser emission if TTL mode is off
	long laserTTL = 0;
	GetLaserTTL(laserTTL);
	if (laserTTL == false)
	{
		SetLaser1Emission(ENABLE_EMISSION);
		SetLaser2Emission(ENABLE_EMISSION);
		SetLaser3Emission(ENABLE_EMISSION);
		SetLaser4Emission(ENABLE_EMISSION);
	}

	LARGE_INTEGER freqInt;
	QueryPerformanceFrequency(&freqInt);
	double dfrq=(double) freqInt.QuadPart;
	double ratePrev = 0;

	LONGLONG QPartLast = 0;
	do
	{		
		LARGE_INTEGER largeint;
		DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
		QueryPerformanceCounter(&largeint);
		::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
		LONGLONG QPart1=largeint.QuadPart;

		if (FALSE ==  GetCamera(SelectedHardware::SELECTED_CAMERA1)->SetupAcquisition(pChan[0]))
			break;

		if (FALSE == GetCamera(SelectedHardware::SELECTED_CAMERA1)->StartAcquisition(pChan[0]))
			break;

		WaitForAcquisition();

		if(FALSE == stopCapture)
		{
			CHECK_INLINE_PACTIVEIMAGEROUTINE(CopyAcquisition(TRUE));
		}

#ifdef OUTPUT_PERFORMANCE
		frameCount++;
#endif 		

		oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
		QueryPerformanceCounter(&largeint);
		::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
		LONGLONG QPart2=largeint.QuadPart;
		double dcountdiv=(double) (QPart2-QPart1);
		double dTime=dcountdiv/dfrq;
		double currentRate = 1.0/dTime;

#ifdef OUTPUT_PERFORMANCE
		switch(camType)
		{
		case ICamera::CCD:
		case ICamera::CCD_MOSAIC:
			{
				if(currentRate < 5.0)
					ratePrev = currentRate;
				else
					ratePrev = currentRate*.125 + ratePrev * .875;
			}

		case ICamera::LSM:
			{		
				ratePrev = currentRate*.125 + ratePrev * .875;
			}

		default:
			{
				if(currentRate < 5.0)
					ratePrev = currentRate;
				else
					ratePrev = currentRate*.125 + ratePrev * .875;
			}
		}

		instancePtr->SetFrameRate(ratePrev);
#endif

		dTime = (QPart2 - QPartLast)/dfrq;

		const double UPDATE_STATS_INTERVAL_SEC = .2;
		if(dTime > UPDATE_STATS_INTERVAL_SEC)
		{
			////// temporarily commented out since auto roi identification has not been released.
			//////StatsManager::getInstance()->ComputeContours( (unsigned short*)pMemoryBuffer, 
			//////	imageInfo.imageWidth,
			//////	imageInfo.imageHeight,
			//////	CaptureSetup::getInstance()->_channelEnable,
			//////	_autoDisplayChannel); 
			long r = FALSE;
			CHECK_INEXPRESSION_PACTIVEIMAGEROUTINE(GetDisplayChannels(), r);
			StatsManager::getInstance()->ComputeStats( (unsigned short*)pMemoryBuffer, 
				imageInfo,																
				r,TRUE,TRUE,FALSE);	

			QPartLast = QPart2;
		}

#ifdef OUTPUT_PERFORMANCE
		const long OUTPUT_SIZE = 256;
		wchar_t buffer[OUTPUT_SIZE];
		StringCbPrintfW(buffer,OUTPUT_SIZE,L"CaptureSetup frame rate count(%d) (%d)fps",frameCount,static_cast<long>(1.0/dTime));
		logDll->TLTraceEvent(VERBOSE_EVENT,1,buffer);
#endif

	}while(stopCapture == FALSE);

	goto RETURN;

RETURN:
	//its possible that the modality was switched while the capture was still running
	//use the selected shutter from the start of the acquisition to ensure
	//the proper shutter is closed
	//close the shutter
	pShutter->SetParam(IDevice::PARAM_SHUTTER_POS,SHUTTER_CLOSE);
	pShutter->PreflightPosition();
	pShutter->SetupPosition();
	pShutter->StartPosition();
	_shutterOpened = FALSE;

	PostflightCamera(SelectedHardware::SELECTED_CAMERA1);

	PostflightPMT();

	//disable LEDs
	SetBFLampPosition(DISABLE_LEDS);

	//disable Laser Emission
	SetLaser1Emission(DISABLE_EMISSION);
	SetLaser2Emission(DISABLE_EMISSION);
	SetLaser3Emission(DISABLE_EMISSION);
	SetLaser4Emission(DISABLE_EMISSION);

	CHECK_INLINE_PACTIVEIMAGEROUTINE(SetCaptureActive(FALSE));

	SetEvent(hCaptureActive);

	return 0;
}

DllExportLiveImage GetActiveCapture()
{
	CHECK_PACTIVEIMAGEROUTINE(GetCaptureActive());
}

DllExportLiveImage GetActiveZStack()
{
	CHECK_PACTIVEIMAGEROUTINE(GetZStackActive());
}

DllExportLiveImage SetupCaptureBuffers()
{
	CHECK_PACTIVEIMAGEROUTINE(SetupCaptureBuffers());
}

DllExportLiveImage StartLiveCapture() 
{
	CHECK_PACTIVEIMAGEROUTINE(StartLiveCapture());
}

DllExportLiveImage StopLiveCapture() 
{
	CHECK_PACTIVEIMAGEROUTINE(StopLiveCapture());
}

/// <summary> Retrieve TIFF Configuration from Application Settings </summary>
/// <param name="OME_Enabled"> Include OME TIFF Info </param>
/// <param name="TiFF_Compression_Enabled"> Enable TIFF compression </param>
void GetTIFFConfiguration(long &OME_Enabled, long &TiFF_Compression_Enabled)
{
	wchar_t fileName[MAX_PATH];
	wstring tempPath = ResourceManager::getInstance()->GetApplicationSettingsFilePathAndName();
	StringCbPrintfW(fileName,_MAX_PATH, tempPath.c_str());

	// load the ApplicationSettings.xml 
	ticpp::Document doc(CaptureSetup::ConvertWStringToString(fileName).c_str());
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
				OME_Enabled = (long)(strValue.at(0) == '1');			
			}
		}
		if ("TIFFCompressionEnable" == strName)
		{
			// now parse through all the attributes of this fruit
			ticpp::Iterator< ticpp::Attribute > attribute;
			for(attribute = attribute.begin(child.Get()); attribute != attribute.end(); attribute++)
			{
				attribute->GetName(&strName);
				attribute->GetValue(&strValue);
				TiFF_Compression_Enabled = (long)(strValue.at(0) == '1');			
			}
		}
	}
}

string uUIDSetup(vector<wstring> wavelengthNames,bool appendName, long bufferChannels, long zstageSteps, long index, long subWell)
{
	wchar_t filePathAndName[_MAX_PATH];

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	imgNameFormat << L"%ls_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";	

	string strOME;
	for(long z=0; z<zstageSteps; z++)
	{
		for(long c=0; c<bufferChannels; c++)
		{	
			wstring wavelengthName = wavelengthNames.at(c);

			GUID *pguid = 0x00;

			pguid = new GUID;

			CoCreateGuid(pguid);

			OLECHAR* bstrGuid;
			StringFromCLSID(*pguid, &bstrGuid);

			wstring ws(bstrGuid);

			//remove the curly braces at the end and start of the guid
			ws.erase(ws.size()-1,1);
			ws.erase(0,1);

			string strGuid = CaptureSetup::ConvertWStringToString(ws);

			ostringstream ss;
			ss << "<TiffData" << " FirstT=\"" << 0 << "\"" << " FirstZ=\"" << z << "\"" << " FirstC=\"" << c << "\">" ;

			if(appendName)
			{
				StringCbPrintfW(filePathAndName,_MAX_PATH,imgNameFormat.str().c_str(),wavelengthName.c_str(),index,subWell,z+1,1);
			}
			else
			{
				StringCbPrintfW(filePathAndName,_MAX_PATH,L"%ls",wavelengthName.c_str());
			}

			wstring wsPath(filePathAndName);
			string strFilePathAndName =  CaptureSetup::ConvertWStringToString(wsPath);
			ss << "<UUID FileName=\"" << strFilePathAndName.c_str() << "\">" << "urn:uuid:" << strGuid.c_str()  << "</UUID>" << "</TiffData>"; 
			strOME += ss.str();
			// ensure memory is freed
			::CoTaskMemFree(bstrGuid);	

			delete pguid; 
		}
	}

	return strOME;
}

/// <summary> Saves a tiff for each channel in the input image buffer </summary>
/// <param name="destinationPath"> The path at which the tiff will be saved </param>
/// <param name="baseName"> The postfix name that will be used as a base for the individual channel names </param>
/// <param name="imageMemory"> The buffer containing the image data for all possible enabled or disabled channels </param>
/// <param name="enabledChannelsBitmask"> Bitmask of which channels in the buffer to save </param>
DllExportLiveImage SimplifiedSaveTIFFs(wchar_t* destinationPath, wchar_t* baseName, char* imageMemory, int enabledChannelsBitmask, int saveMultiPage)
{
	//Enabled Channels & Names
	std::vector<int> enabledChannels;
	vector<wstring> wavelengthNames;
	wstring wstr(baseName);
	long chanCnt = ParseLSMChannels(enabledChannelsBitmask, &enabledChannels, &wavelengthNames, wstr);

	//Image Dimensions
	long width = 0, height = 0;
	CHECK_INLINE_PACTIVEIMAGEROUTINE(GetImageDimensions(width, height));
	long doOME = TRUE;
	long doCompression = TRUE;	
	GetTIFFConfiguration(doOME, doCompression);

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	imgNameFormat << L"%s_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";	

	string strOME = "";

	//save as multipage tiff if requested, will use path as file name: 
	if(TRUE == saveMultiPage)
	{
		//use current file name in OME
		wchar_t fNameT[FILENAME_MAX]; 
		wcsncpy_s(fNameT,destinationPath,FILENAME_MAX);
		wstring fName(fNameT);
		std::size_t found = fName.find_last_of(L"\\");
		if(found != std::wstring::npos)
		{
			wstring subStr = fName.substr(found+1,fName.size()-found);
			wavelengthNames.clear();
			for (int i = 0; i < chanCnt; i++)
			{
				wavelengthNames.push_back(subStr);
			}
			strOME = (TRUE == doOME) ? uUIDSetup(wavelengthNames, false, static_cast<long>(chanCnt), 1, 1, 1) : "";
		}
		return SaveMultiPageTIFF(destinationPath, imageMemory, width, height, NULL, NULL, NULL, 0,static_cast<int>(chanCnt),1,1,0,0,0,NULL,0, &strOME, doCompression);
	}

	//save separate tiffs for 3 channels or less:
	strOME = (TRUE == doOME) ? uUIDSetup(wavelengthNames, true, chanCnt, 1, 1, 1) : "";
	wchar_t filePathAndName[_MAX_PATH];
	for(int channel : enabledChannels)
	{
		std::wstringstream nameStream;
		nameStream << destinationPath << L"\\" << baseName << channelLetter(channel);
		std::wstring name = nameStream.str();
		StringCbPrintfW(filePathAndName,_MAX_PATH,imgNameFormat.str().c_str(),name.c_str(),1,1,1,1);

		//If only one channel, access only the data at the front.
		if(1 == chanCnt)
		{
			SaveTIFF(filePathAndName, imageMemory, width, height, NULL, NULL, NULL, 0,static_cast<int>(chanCnt),1,1,0,0,0,channel,NULL,0, &strOME, doCompression);
		}
		else
		{
			long long memoryOffset = static_cast<long long>(channel) * width * height * 2;
			SaveTIFF(filePathAndName, imageMemory + memoryOffset, width, height, NULL, NULL, NULL, 0,static_cast<int>(chanCnt),1,1,0,0,0,channel,NULL,0, &strOME, doCompression);
		}
	}

	return TRUE;
}


DllExportLiveImage LiveSnapshot()
{
	CHECK_PACTIVEIMAGEROUTINE(Snapshot(NULL));
}

/// <summary> Takes a snapshot and saves it in the destination folder. Each channel is saved as a separate image </summary>
/// <param name="destinationPath"> The folder to save the snapshot images in </param>
/// <param name="enabledChannelBitmask"> Bitmask of which channels to save </param>
DllExportLiveImage SnapshotAndSave(wchar_t* destinationPath, int enabledChannelBitmask, int saveMultiPage)
{
	SnapshotSaveParams* tParam = new SnapshotSaveParams();	
	wmemcpy(tParam->path,destinationPath,MAX_PATH);
	tParam->channelBitMask = enabledChannelBitmask;
	tParam->saveMultiPage = saveMultiPage;

	CHECK_PACTIVEIMAGEROUTINE(Snapshot(tParam));
}

DllExportLiveImage UpdateStats() 
{
	long r = FALSE;	
	CHECK_INEXPRESSION_PACTIVEIMAGEROUTINE(GetDisplayChannels(), r);
	if((NULL != pMemoryBuffer)&&
		(0 != imageInfo.imageWidth)&&
		(0 != imageInfo.imageHeight)&&																
		(0 != r))
	{
		////// temporarily commented out since auto roi identification has not been released.
		/////*		StatsManager::getInstance()->ComputeContours( (unsigned short*)pMemoryBuffer, 
		////			imageInfo.imageWidth,
		////			imageInfo.imageHeight,																	
		////			CaptureSetup::getInstance()->_channelEnable,
		////			_autoDisplayChannel);*/			
		StatsManager::getInstance()->ComputeStats( (unsigned short*)pMemoryBuffer, 
			imageInfo,																	
			r,TRUE,TRUE,FALSE);	
	}

	return TRUE;
}

DllExportLiveImage InitCallBack(imageCompleteCallback dm, completeCallback dt) 
{	
	CHECK_PACTIVEIMAGEROUTINE(InitCallbacks(dm,dt));
}

DllExportLiveImage EnableCopyToExternalBuffer()
{
	CHECK_PACTIVEIMAGEROUTINE(EnableCopyToExternalBuffer());
}

DllExportLiveImage StopZStackCapture() 
{
	CHECK_PACTIVEIMAGEROUTINE(StopZStackCapture());
}

DllExportLiveImage CaptureZStack(double zStartPos, double zStopPos, double zstageStepSize, long zstageSteps)
{	
	CHECK_PACTIVEIMAGEROUTINE(CaptureZStack(zStartPos,zStopPos,zstageStepSize,zstageSteps));
}

DllExportLiveImage SetDisplayChannels(int channelEnable)
{
	return CaptureSetup::getInstance()->SetDisplayChannels(channelEnable);
}

DllExportLiveImage AutoTrackingEnable(long channelIndex)//0 = disable 1,2,3,4 A,B,C,D
{
	_autoDisplayChannel = channelIndex;
	return TRUE;
}

DllExportLiveImage ImgProGenConf(int maxRoiNum, int minSnr)
{
	return CaptureSetup::getInstance()->ImgProGenConf(maxRoiNum, minSnr);
}

DllExportLiveImage EnableMinAreaFilter(bool minAreaActive, int minAreaValue)
{
	return CaptureSetup::getInstance()->EnableMinAreaFilter(minAreaActive, minAreaValue);
}

//TODO: This code is not used at all. Make sure it won't be needed in the future and kill it
//void ContrastScore(char * pBuffer, long width, long height, long skipSize, long &score)
//{
//	unsigned short high1;//highest value in the kernel
//	unsigned short high2;
//	unsigned short high3;
//	unsigned short low1;//lowest value in the kernel
//	unsigned short low2;
//	unsigned short low3;
//
//	unsigned short * tempPtr;
//	unsigned short * imageBuffer = (unsigned short*)pBuffer;
//
//	long      fx,fy;
//
//	int filterWidth = 3;
//	int filterHeight = 3;
//
//	long fWidth2,fHeight2;
//
//	fWidth2 = (filterWidth-1)/2;
//	fHeight2 = (filterHeight-1)/2;
//
//	unsigned short *dataArr; 
//
//	dataArr = new unsigned short[static_cast<long long>(filterWidth)*filterHeight]; 
//
//	long sum = 0;
//	unsigned short *startPtr = imageBuffer;
//
//	for(int y=fHeight2;y<(height-fHeight2);y+=skipSize)
//	{
//		for(int x=fWidth2;x<(width-fWidth2);x+=skipSize) 
//		{
//			startPtr = imageBuffer + static_cast<size_t>(y)*width + x;
//			high1 = *startPtr;
//			high2 = *startPtr;
//			high3 = *startPtr;
//			low1 = *startPtr;
//			low2 = *startPtr;
//			low3 = *startPtr;
//
//			for(fy=0;fy<filterHeight;fy++)
//			{
//				tempPtr = startPtr;
//				tempPtr +=  static_cast<unsigned short>((-fWidth2) + ((fy-fHeight2)*(width)));
//
//				for(fx=0;fx<filterWidth;fx++)
//				{
//					if(*tempPtr > high1)
//					{
//						high3 = high2;
//						high2 = high1;
//						high1 = *tempPtr;
//					}
//					if((high1 > *tempPtr) &&(*tempPtr > high2))
//					{
//						high3 = high2;
//						high2 = *tempPtr;
//					}
//
//					if(*tempPtr < low1)
//					{
//						low3 = low2;
//						low2 = low1;
//						low1 = *tempPtr;
//					}
//					if((low1 > *tempPtr) && (*tempPtr < low2))
//					{
//						low3 = low2;
//						low2 = *tempPtr;
//					}
//					tempPtr++;
//				}
//			}
//
//			//use the third highest/lowest values for the contrast score
//			sum += (high3-low3);
//		}
//	}
//
//	score = sum;
//}

DllExportLiveImage StartAutoFocus(double magnification, int autoFocusType, BOOL& afFound)
{
	//auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML);

	//string objName;
	//long position=0;
	//double numAperture;
	//double afStartPos=0;
	//double afFocusOffset=0;
	//double afAdaptiveOffset=0;
	//long beamExpPos=0;
	//long beamExpWavelength=0;
	//long beamExpPos2=0;
	//long beamExpWavelength2=0;
	//long turretPosition=0;
	//long zAxisToEscape=0;
	//double zAxisEscapeDistance=0;

	//pHardware->GetMagInfoFromMagVal(magnification,objName,position,numAperture,afStartPos,afFocusOffset,afAdaptiveOffset,beamExpPos,beamExpWavelength,beamExpPos2,beamExpWavelength2, turretPosition, zAxisToEscape, zAxisEscapeDistance);

	//SetDeviceParamDouble(SelectedHardware::SELECTED_AUTOFOCUS,IDevice::PARAM_AUTOFOCUS_OFFSET,afFocusOffset,false);
	////set autofocus to execute
	//double dPos = 1.0;
	//SetDeviceParamDouble(SelectedHardware::SELECTED_AUTOFOCUS,IDevice::PARAM_AUTOFOCUS_POS,dPos,TRUE);
	//return TRUE;

	CHECK_PACTIVEIMAGEROUTINE(StartAutoFocus(magnification, autoFocusType, afFound));
}
