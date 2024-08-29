// RunSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "..\..\..\..\Common\Sample\Sample\SampleBuilder.h"
#include "..\..\..\..\Common\Sample\Sample\SampleDll.h"
#include "..\..\..\..\Common\Sample\Sample\SampleDllConcrete.h"

#include "RunSample.h"
#include "AcquireFactory.h"
#include "ImageCorrection.h"
#include "ippi.h"

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max
#include <grpcpp/grpcpp.h>
#include <absl/strings/str_format.h>
#pragma pop_macro("min")
#pragma pop_macro("max")

#include <thread>
#include "thalamus.grpc.pb.h"

typedef void (_cdecl *myPrototype)(long* index, long* completed, long* total, long* timeElapsed, long* timeRemaining, long* captureCompletee);
void (*updateProgressBarCallback)(long* index, long* completed, long* total, long* timeElapsed, long* timeRemaining, long* captureComplete) = NULL;

//typedef void (_cdecl *myPrototypeCaptureComplete)(long* index);
////funtion pointer for sending back the index of the completed capture
//void (*myFunctionPointerCaptureComplete)(long* index) = NULL;

typedef void (_cdecl *myPrototypeBeginImage)(long* index);
//funtion pointer for sending back the index of the current WELL in progress
void (*myFunctionPointerBeginImage)(long* index) = NULL;

typedef void (_cdecl *myPrototypeBeginSubImage)(long* index);
//funtion pointer for sending back the index of the current started subWELL in progress
void (*myFunctionPointerBeginSubImage)(long* index) = NULL;

typedef void (_cdecl *myPrototypeEndSubImage)(long* index);
//funtion pointer for sending back the index of the current completed subWELL in progress
void (*myFunctionPointerEndSubImage)(long* index) = NULL;

typedef void (_cdecl *myPrototypeSaveZImage)(long* index, double* power0, double* power1, double* power2, double* power3, double* power4, double* power5);
//funtion pointer for sending back the index of the current completed Z, and power postion
void (*myFunctionPointerSaveZImage)(long* index, double* power0, double* power1, double* power2, double* power3, double* power4, double* power5) = NULL;

typedef void (_cdecl *myPrototypeSaveTImage)(long* index);
//funtion pointer for sending back the index of the current completed T 
void (*myFunctionPointerSaveTImage)(long* index) = NULL;

typedef void (_cdecl *myPrototypePreCapture)(long* status);
//funtion pointer for getting back the status of pre capture
void (*myFunctionPointerPreCapture)(long* status) = NULL;

typedef void (_cdecl *myPrototypeSequenceStepCurrentIndex)(long* index);
//funtion pointer for getting back the index of completed channel Step
void (*myFunctionPointerSequenceStepCurrent)(long* index) = NULL;

typedef void(_cdecl* myPrototypeInformMessage)(wchar_t* message);
//funtion pointer for sending message to notice user
void (*myFunctionPointerInformMessage)(wchar_t* message) = NULL;

typedef void(_cdecl* myPrototypeFileSavedNameAndPathForIPC)(wchar_t* message);
//funtion pointer for sending an IPC command out with the name and path when a file has been saved. 
void (*myFunctionPointerFileSavedNameAndPathForIPC)(wchar_t* message) = NULL;

typedef void(_cdecl* myPrototypeAutoFocusStatus)(long * isRunning, long * bestScore, double * bestZPos, double * nextZPos, long * currRepeat);
//funtion pointer for sending back the index of the current completed T 
void (*myFunctionPointerAutoFocusStatus)(long* isRunning, long* bestScore, double* bestZPos, double* nextZPos, long* currRepeat) = NULL;

long SetDeviceParameterValue(IDevice *pDevice,long paramID, double val,long bWait,HANDLE hEvent,long waitTime);

DWORD dwRunSampleThreadId = NULL;
HANDLE hRunSampleThread = NULL;
DWORD dwThreadId = NULL;
HANDLE hThread = NULL;
HANDLE hStatusEvent;
BOOL stopRun = FALSE;
BOOL activeRun = FALSE;
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
wchar_t message[MSG_LENGTH];
auto_ptr<CommandDll> shwDll(new CommandDll(L".\\SelectHardware.dll"));
auto_ptr<SampleDll> sampleDll(new SampleDll(L".\\Sample.dll"));
auto_ptr<TiffLibDll> tiffDll(new TiffLibDll(L"..\\libtiff3.dll"));
unique_ptr<ImageStoreLibraryDLL> bigTiff(new ImageStoreLibraryDLL(L".\\ImageStoreLibrary.dll"));
vector<ScanRegion> activeScanAreas;		//for ResonanceGalvoGalvo (multi-area scan) preview on the first active scan area for now
long viewMode = MesoScanTypes::Meso;
DWORD _dwSafetyInterLockCheckThreadId = NULL;
HANDLE _hSafetyInterLockCheckThread = NULL;
atomic<BOOL> _shutterOpened = FALSE;

DllExport_RunSample InitCallBack(myPrototype dm, myPrototypeBeginImage di, myPrototypeBeginSubImage bsi, myPrototypeEndSubImage esi, myPrototypeSaveZImage szi, myPrototypeSaveTImage sti, myPrototypePreCapture pc, myPrototypeSequenceStepCurrentIndex cs, myPrototypeInformMessage sm, myPrototypeFileSavedNameAndPathForIPC savedFileIPC, myPrototypeAutoFocusStatus afs) //myPrototypeCaptureComplete cc,
{
	updateProgressBarCallback = dm;

	myFunctionPointerBeginImage = di;

	myFunctionPointerBeginSubImage = bsi;

	myFunctionPointerEndSubImage = esi;

	myFunctionPointerSaveZImage = szi;

	myFunctionPointerSaveTImage = sti;

	//myFunctionPointerCaptureComplete = cc;

	myFunctionPointerPreCapture = pc;

	myFunctionPointerSequenceStepCurrent = cs;

	myFunctionPointerInformMessage = sm;

	myFunctionPointerFileSavedNameAndPathForIPC = savedFileIPC;

	myFunctionPointerAutoFocusStatus = afs;

	if(updateProgressBarCallback != NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"RunSample InitCallBack");		
	}

	return TRUE;
}

DllExport_RunSample IsRunning()
{
	if(RunSample::getInstance()->_isActive)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

DllExport_RunSample GetSaving(bool &_getSave)
{
	_getSave = RunSample::getInstance()->_isSaving;		
	return TRUE;
}

DllExport_RunSample GetZRange(double &zMin, double &zMax, double &zDefault)
{
	IDevice *pZStage = NULL;	

	pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

	if(NULL == pZStage)
	{
		return FALSE;
	}

	return GetDeviceParameterValueRangeDouble(pZStage,IDevice::PARAM_Z_POS,zMin,zMax,zDefault);
}

DllExport_RunSample GetZ2Range(double& zMin, double& zMax, double& zDefault)
{
	IDevice* pZStage = NULL;

	pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE2);

	if (NULL == pZStage)
	{
		return FALSE;
	}

	return GetDeviceParameterValueRangeDouble(pZStage, IDevice::PARAM_Z_POS, zMin, zMax, zDefault);
}

DllExport_RunSample GetXPosition(double &xPosition)
{
	IDevice *pXYStage = NULL;	

	pXYStage = GetDevice(SelectedHardware::SELECTED_XYSTAGE);

	if(NULL == pXYStage)
	{
		return FALSE;
	}

	return GetDeviceParameterValueDouble(pXYStage,IDevice::PARAM_X_POS_CURRENT,xPosition);
}

DllExport_RunSample GetYPosition(double &yPosition)
{
	IDevice *pXYStage = NULL;	

	pXYStage = GetDevice(SelectedHardware::SELECTED_XYSTAGE);

	if(NULL == pXYStage)
	{
		return FALSE;
	}

	return GetDeviceParameterValueDouble(pXYStage,IDevice::PARAM_Y_POS_CURRENT,yPosition);
}

DllExport_RunSample SetSaving(bool _toSave)
{
	RunSample::getInstance()->_isSaving = _toSave;		
	return TRUE;
}

struct RunSample::Impl : public thalamus_grpc::Thalamus::Service, Publisher {
  std::mutex mutex;
  std::set<::grpc::ServerWriter< ::thalamus_grpc::Image>*> clients;
  bool bigendian;
  bool running;
  std::thread grpc_thread;
	std::unique_ptr<grpc::Server> server;

  Impl() {
  	int num = 1;
  	bigendian = *(char*)&num != 1;
  }

  ~Impl() {
    shutdown();
  }

  void setup() {
    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:50060", grpc::InsecureServerCredentials());
    builder.RegisterService(this);
    server = builder.BuildAndStart();
    running = true;

    grpc_thread = std::thread([&] {
      server->Wait();
    });
  }

  void shutdown() {
    running = false;
    if(grpc_thread.joinable()) {
      server->Shutdown();
      grpc_thread.join();
    }
  }

  static const size_t image_chunk_size = 524288;

  void publish_image(int width, int height, int bufferChannels, int bytesPerChannel, unsigned char* data) override {
    std::lock_guard<std::mutex> lock(mutex);

		thalamus_grpc::Image::Format format;
		if (bytesPerChannel == 1) {
			format = bufferChannels == 1 ? thalamus_grpc::Image_Format_Gray : thalamus_grpc::Image_Format_RGB;
		}
		else {
			format = bufferChannels == 2 ? thalamus_grpc::Image_Format_Gray16 : thalamus_grpc::Image_Format_RGB16;
		}
    auto position = 0ull;
    auto length = width*height*bufferChannels*bytesPerChannel;

    while(position < length) {
      auto step_size = min(image_chunk_size, length-position);
      thalamus_grpc::Image image;
      image.set_width(width);
      image.set_height(height);
      image.set_format(format);
      image.set_bigendian(bigendian);
      image.add_data(data + position, step_size);
      position += step_size;
      image.set_last(position == length);

      for(auto client : clients) {
        client->Write(image);
      }
    }
  }

  ::grpc::Status get_type_name(::grpc::ServerContext* context, const ::thalamus_grpc::StringMessage* request, ::thalamus_grpc::StringMessage* response) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status node_request(::grpc::ServerContext* context, const ::thalamus_grpc::NodeRequest* request, ::thalamus_grpc::NodeResponse* response) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status events(::grpc::ServerContext* context, ::grpc::ServerReader< ::thalamus_grpc::Event>* reader, ::util_grpc::Empty*) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status log(::grpc::ServerContext* context, ::grpc::ServerReader< ::thalamus_grpc::Text>* reader, ::util_grpc::Empty*) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status observable_bridge(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::thalamus_grpc::ObservableChange, ::thalamus_grpc::ObservableChange>* stream) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status graph(::grpc::ServerContext* context, const ::thalamus_grpc::GraphRequest* request, ::grpc::ServerWriter< ::thalamus_grpc::GraphResponse>* writer) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status get_recommended_channels(::grpc::ServerContext* context, const ::thalamus_grpc::NodeSelector* request, ::thalamus_grpc::StringListMessage* response) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status analog(::grpc::ServerContext* context, const ::thalamus_grpc::AnalogRequest* request, ::grpc::ServerWriter< ::thalamus_grpc::AnalogResponse>* writer) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status spectrogram(::grpc::ServerContext* context, const ::thalamus_grpc::SpectrogramRequest* request, ::grpc::ServerWriter< ::thalamus_grpc::SpectrogramResponse>* writer) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status channel_info(::grpc::ServerContext* context, const ::thalamus_grpc::AnalogRequest* request, ::grpc::ServerWriter< ::thalamus_grpc::AnalogResponse>* writer) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status xsens(::grpc::ServerContext* context, const ::thalamus_grpc::NodeSelector* request, ::grpc::ServerWriter< ::thalamus_grpc::XsensResponse>* writer) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status image(::grpc::ServerContext* context, const ::thalamus_grpc::ImageRequest* request, ::grpc::ServerWriter< ::thalamus_grpc::Image>* writer) override {
    {
      std::lock_guard<std::mutex> lock(mutex);
      clients.insert(writer);
    }

    while(!context->IsCancelled() && running) {
      std::this_thread::sleep_for(1s);
    }

    std::lock_guard<std::mutex> lock(mutex);
    clients.erase(writer);
    return ::grpc::Status::OK;
  }
  ::grpc::Status replay(::grpc::ServerContext* context, const ::thalamus_grpc::ReplayRequest* request, ::util_grpc::Empty* response) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status eval(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::thalamus_grpc::EvalRequest, ::thalamus_grpc::EvalResponse>* stream) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status remote_node(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::thalamus_grpc::RemoteNodeMessage, ::thalamus_grpc::RemoteNodeMessage>* stream) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status notification(::grpc::ServerContext* context, const ::util_grpc::Empty* request, ::grpc::ServerWriter< ::thalamus_grpc::Notification>* writer) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status inject_analog(::grpc::ServerContext* context, ::grpc::ServerReader< ::thalamus_grpc::InjectAnalogRequest>* reader, ::util_grpc::Empty*) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status get_modalities(::grpc::ServerContext* context, const ::thalamus_grpc::NodeSelector* request, ::thalamus_grpc::ModalitiesMessage* response) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status ping(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::thalamus_grpc::Pong, ::thalamus_grpc::Ping>* stream) override {
    return ::grpc::Status::OK;
  }
};

RunSample::RunSample() : impl(new Impl())
{
	//private constructor

	_params.version = 1.0;
	_params.showDialog = FALSE;
	_totalNumOfTiles = 0;

	memset(&_paramsCustom,0,sizeof _paramsCustom);	

}

GUID RunSample::_guid = { 0x8b7c9b19, 0xfdab, 0x4e27, { 0xa8, 0xf8, 0xa2, 0x52, 0xff, 0x63, 0xf9, 0x58 } };

bool RunSample::_instanceFlag = false;

bool RunSample::_setupFlag = false;

auto_ptr<RunSample> RunSample::_single(new RunSample());

bool RunSample::_isActive = false;
bool RunSample::_isSaving = false;

HANDLE RunSample::_hStopStatusThread = CreateEvent(NULL, TRUE, false, NULL);	//MANUAL RESET

RunSample* RunSample::getInstance()
{
	if(! _instanceFlag)
	{
		StringCbPrintfW(message,MSG_LENGTH,L"Creating RunSample Singleton");
		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

		_single.reset(new RunSample());
		_instanceFlag = true;
	}
	return _single.get();
}

RunSample::~RunSample()
{
	_instanceFlag = false;

	SAFE_DELETE_HANDLE(hRunSampleThread);
}

long RunSample::GetCommandGUID(GUID *guidRet)
{
	memcpy(guidRet,&_guid,sizeof GUID);

	return TRUE;
}

long GetDeviceParameterValueDouble(IDevice *pDevice,long paramID, double &val)
{
	long ret = FALSE;

	if(NULL == pDevice)
	{
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pDevice->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		if(paramAvailable)
		{
			pDevice->GetParam(paramID,val);
			ret = TRUE;
		}
	}
	return ret;
}

long GetDeviceParameterValueRangeDouble(IDevice * pDevice,long paramID, double &valMin, double &valMax, double &valDefault)
{
	long ret = FALSE;

	if(NULL == pDevice)
	{
		StringCbPrintfW(message,MSG_LENGTH,L"%hs@%u: pDevice is NULL", __FUNCTION__, __LINE__);
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

	if(pDevice->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		if(TRUE == paramAvailable)
		{
			valMin = paramMin;
			valMax = paramMax;
			valDefault = paramDefault;
			ret = TRUE;
		}
		else
		{
			ret = FALSE;
		}
	}

	return ret;
}

void GetActiveScanAreaThenEnableAll(IExperiment* exp)
{
	long cameraType = (GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_TYPE,cameraType)) ? cameraType : ICamera::CameraType::LAST_CAMERA_TYPE;
	long lsmType = (GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_TYPE,lsmType)) ? lsmType : ICamera::LSMType::LSMTYPE_LAST;
	if(ICamera::CameraType::LSM == cameraType && ICamera::LSMType::RESONANCE_GALVO_GALVO == lsmType && exp->GetScanRegions(viewMode, activeScanAreas) && (long)MesoScanTypes::Micro == viewMode)
	{
		//use experiment manager to enable all scan areas in micro view
		exp->SetAllScanAreas(viewMode, 1);
		exp->Update();
	}
}

long RunSample::GetParamInfo(const long paramID, long &paramType,long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
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

long RunSample::SetParam(const long paramID, const double param)
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

long RunSample::GetParam(const long paramID, double &param)
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

long RunSample::SetCustomParamsBinary(const char *buf)
{
	memcpy(&_paramsCustom,buf,sizeof (_paramsCustom));

	return TRUE;
}

long RunSample::GetCustomParamsBinary(char *buf)
{
	memcpy(buf,&_paramsCustom,sizeof (_paramsCustom));

	return TRUE;
}

long RunSample::SaveCustomParamsXML(void *fileHandle)
{
	return FALSE;
}

long RunSample::LoadCustomParamXML(void *fileHandle)
{
	return FALSE;
}

long IsXyViewVisible()
{
	long ret = FALSE;

	wstring tempPath = ResourceManager::getInstance()->GetApplicationSettingsPath();
	tempPath += wstring(L"ApplicationSettings.xml");
	string str = ConvertWStringToString(tempPath);

	// load the ApplicationSettings.xml 
	ticpp::Document doc(str.c_str());
	doc.LoadFile();
	ticpp::Element *AppSettingsObj = doc.FirstChildElement(false);
	ticpp::Element* root = AppSettingsObj->FirstChildElement("DisplayOptions"); 
	ticpp::Element* parent = root->FirstChildElement("CaptureSetup");
	ticpp::Iterator<ticpp::Element> child(parent->FirstChildElement("XYView"), "XYView");
	// parse through all children
	string attrVal;
	child->GetAttribute(string("Visibility"), &attrVal);
	if(attrVal.compare("Visible") == 0)
		ret = TRUE;

	return ret;
}

long CreateSample(IExperiment *exp)
{
	long totalNumOfTiles = 0;
	double sampleOffsetX, sampleOffsetY, sampleOffsetZ;
	double currentX,currentY;
	long tiltAdjustment;
	double fPt[3][3];
	exp->GetSample(sampleOffsetX,sampleOffsetY,sampleOffsetZ,tiltAdjustment,fPt[0][0],fPt[0][1],fPt[0][2],fPt[1][0],fPt[1][1],fPt[1][2],fPt[2][0],fPt[2][1],fPt[2][2]);

	long camType = 0, lsmType = ICamera::LSMType::LSMTYPE_LAST, mROIEnabled;
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_TYPE, camType);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_TYPE, lsmType);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_MROI_MODE_ENABLE, mROIEnabled);

	vector<IExperiment::SubImage> SubImages; 
	if(IsXyViewVisible())
		exp->GetSubImages(SubImages, camType, lsmType, mROIEnabled);
	GetXPosition(currentX);
	GetYPosition(currentY);

	sampleDll->CreatePlateMosaicSample(sampleOffsetX, sampleOffsetY,sampleOffsetZ, SubImages);
	for (int i = 0; i < SubImages.size(); i++)
	{
		totalNumOfTiles += SubImages[i].subRows * SubImages[i].subColumns;
	}
	if (SubImages.size() == 0)
	{
		totalNumOfTiles = 1;
	}
	return(totalNumOfTiles);
}

void SetupImageCorrection(IExperiment *exp, long camWidth, long camHeight)
{
	ICamera * pCamera = NULL;

	pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);

	//setup the image correction buffers according to the camera type
	double camType;
	pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,camType);

	if(static_cast<long>(camType) == ICamera::CCD)
	{
		SetupImageCorrectionBuffers(exp,camWidth, camHeight);
	}
	else
	{
		double lsmWidth;
		double lsmHeight;

		pCamera->GetParam(ICamera::PARAM_LSM_PIXEL_X,lsmWidth);
		pCamera->GetParam(ICamera::PARAM_LSM_PIXEL_Y,lsmHeight);
		SetupImageCorrectionBuffers(exp,static_cast<long>(lsmWidth), static_cast<long>(lsmHeight));
	}
}

struct StatusDeviceProcParams
{
	IDevice *pDevice;
	HANDLE *pEventHandle;
};

UINT StatusDeviceThreadProc( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;

	StatusDeviceProcParams * pStatus = (StatusDeviceProcParams*)pParam;

	while(status == IDevice::STATUS_BUSY)
	{	
		if(WAIT_OBJECT_0 == WaitForSingleObject(RunSample::_hStopStatusThread, 0))
		{
			break;
		}
		if(FALSE == pStatus->pDevice->StatusPosition(status))
		{
			break;
		}
	}

	SetEvent( *pStatus->pEventHandle);

	return 0;
}

void StatusHandler(IDevice * pDevice, HANDLE *pEvent, long maxWaitTime)
{
	DWORD dwThread;

	StatusDeviceProcParams statusParams;

	statusParams.pDevice = pDevice;
	statusParams.pEventHandle = pEvent;

	ResetEvent(RunSample::_hStopStatusThread);

	HANDLE hThread = ::CreateThread( NULL, 0,(LPTHREAD_START_ROUTINE)StatusDeviceThreadProc, &statusParams, 0, &dwThread );

	DWORD dwWait = WaitForSingleObject(*pEvent, maxWaitTime);

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"StatusHandler  StatusThreadProc  failed");

		//signal to terminate the status thread:
		SetEvent(RunSample::_hStopStatusThread);
		if(WaitForSingleObject(*pEvent, maxWaitTime) != WAIT_OBJECT_0)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"Terminate  StatusDeviceThreadProc  failed");
		}
	}		

	CloseHandle(hThread);
	CloseHandle(*pEvent);
}

int fileSize(wstring fileName){
	ifstream mySource;
	mySource.open(fileName, ios_base::binary);
	mySource.seekg(0,ios_base::end);
	long size = static_cast<long>(mySource.tellg());
	mySource.close();
	return size;
}

void ResizeImage(char * pSource, long srcWidth, long srcHeight, char * pDestination, long dstWidth, long dstHeight)
{
	IppiSize size  = {srcWidth, srcHeight}; 
	IppiRect srect = {0, 0, srcWidth, srcHeight}; 
	IppiRect drect = {0, 0, dstWidth, dstHeight};

	double Xscale = static_cast<double>(dstWidth) / static_cast<double>(srcWidth);
	double Yscale = static_cast<double>(dstHeight) / static_cast<double>(srcHeight); 

	int nChannel  = 1;	

	int INTERPOLATIONMODE = IPPI_INTER_CUBIC2P_CATMULLROM;


	/* interploation options:
	09	       IPPI_INTER_NN||IPPI_INTER_LINEAR|| IPPI_INTER_CUBIC
	10	       IPPI_INTER_CUBIC2P_BSPLINE||IPPI_INTER_CUBIC2P_CATMULLROM||PPI_INTER_CUBIC2P_B05C03
	11	       IPPI_INTER_SUPER||IPPI_INTER_LANCZOS */
	char * buf;
	int bufSize;

	// calculation of work buffer size 
	ippiResizeGetBufSize( srect, drect, nChannel, INTERPOLATIONMODE, &bufSize );
	//ippiResizeGetBufSize( srect, drect, 1, IPPI_INTER_CUBIC2P_CATMULLROM, &bufsize );

	// memory allocate 
	buf = (char*)malloc( bufSize );

	IppStatus status = ippStsErr;

	// function call 
	if( NULL != buf )
		status = ippiResizeSqrPixel_16u_C1R((Ipp16u*)pSource, size, srcWidth*sizeof(Ipp16u), srect, (Ipp16u*)pDestination, dstWidth*sizeof(Ipp16u), drect, Xscale, Yscale, 0, 0, INTERPOLATIONMODE, (Ipp8u*)buf );

}

void RunSample::SetPockelsMask(ICamera* camera, IExperiment* exp, string maskPath)
{
	//=== Convert Filename to Wide String ===
	std::string fileNameString(maskPath);
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &fileNameString[0], (int)fileNameString.size(), NULL, 0);
	std::wstring fileNameWideString(size_needed, 0 );
	MultiByteToWideChar(CP_UTF8, 0, &fileNameString[0], (int)fileNameString.size(), &fileNameWideString[0], size_needed);

	try
	{
		double param=0;
		if(FALSE == camera->GetParam(ICamera::PARAM_LSM_POCKELS_MASK_WIDTH,param))
		{
			logDll->TLTraceEvent(VERBOSE_EVENT,1,L"CaptureSetup SetPockelsMaskFile GetParam failed PARAM_LSM_PIXEL_X");
			return;
		}
		long pockelsMaskWidth = static_cast<long>(param);

		wstring ext(PathFindExtension(fileNameWideString.c_str()));
		long width = 0,height = 0,colorChannels = 0, bitsPerPixel = 0;
		char * pBuffer;
		char * pMask;
		const long BYTES_PER_PIXEL = 2;		
		if (ext == L".raw" || ext == L".Raw")
		{
			long areaMode,scanMode,interleave,pixelX,pixelY,channel,fieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles,polarity[4], verticalFlip, horizontalFlip;
			double areaAngle,dwellTime,crsFrequencyHz = 0;
			long timeBasedLineScan = FALSE;
			long timeBasedLineScanMS = 0;
			long threePhotonEnable = FALSE;
			long numberOfPlanes = 1;
			long selectedImagingGG = 0;
			long selectedStimGG = 0;
			double pixelAspectRatioYScale = 1;

			exp->GetLSM(areaMode,areaAngle,scanMode,interleave,pixelX,pixelY,channel,fieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,dwellTime,
				flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles,polarity[0],polarity[1],polarity[2],polarity[3], verticalFlip, horizontalFlip, crsFrequencyHz, timeBasedLineScan, timeBasedLineScanMS, threePhotonEnable, numberOfPlanes, selectedImagingGG, selectedStimGG, pixelAspectRatioYScale);
			width = static_cast<long>(pixelX);
			height = static_cast<long>(pixelY);

			if (fileSize(fileNameWideString) != width*height*BYTES_PER_PIXEL)
			{
				return;
			}

			pBuffer = new char[width*height*BYTES_PER_PIXEL];
			pMask = new char[pockelsMaskWidth*height*BYTES_PER_PIXEL];

			// Open and read the raw image file
			std::ifstream inFile (fileNameWideString, ios::in|ios::binary);
			if(inFile.is_open())
			{
				inFile.seekg(0, ios::beg);
				inFile.read(pBuffer, width*height*BYTES_PER_PIXEL);
				inFile.close();
			}
		}
		else if (ext == L".tif" || ext == L".Tif")
		{
			if(FALSE == ReadImageInfo((wchar_t*)fileNameWideString.c_str(), width, height, colorChannels, bitsPerPixel))
			{
				return;
			}

			pBuffer = new char[width*height*colorChannels*BYTES_PER_PIXEL];
			pMask = new char[pockelsMaskWidth*height*BYTES_PER_PIXEL];

			//Extract the buffer fromt the specified file and assign the buffer
			//to the camera dll
			ReadImage((char*)fileNameWideString.c_str(),(char*&)pBuffer);
		}
		else
		{
			return;
		}

		ResizeImage(pBuffer, width, height, pMask, pockelsMaskWidth, height);

		camera->SetParamBuffer(ICamera::PARAM_LSM_POCKELS_MASK_0,pMask,pockelsMaskWidth*height*BYTES_PER_PIXEL);

		delete[] pBuffer;
		delete[] pMask;
	}
	catch(...)
	{
		StringCbPrintfW(message,MSG_LENGTH,L"CaptureSetup unable to create/assign pockels mask");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
	}
}

void PreCaptureProtocol(IExperiment* exp, long zCount)
{	
	//This function will go through both types of Lasers (MCLS and Multiphoton)
	//and will set the parameters as found in the Active.xml file

	//MCLS Laser parameters
	long enable1;
	double power1;
	long enable2; 
	double power2;
	long enable3; 
	double power3;
	long enable4; 
	double power4;
	long allTTL;
	long allAnalog;
	long wavelength1;
	long wavelength2;
	long wavelength3;
	long wavelength4;
	long captureSequenceEnable = FALSE;
	long sequentialType = 0;
	exp->GetCaptureSequence(captureSequenceEnable, sequentialType);
	exp->GetMCLS(enable1, power1, enable2, power2, enable3, power3, enable4, power4, allTTL, allAnalog, wavelength1, wavelength2, wavelength3, wavelength4);
	//only set the power if TTL mode is on and analog mode is off, otherwise set it to 100
	if (allAnalog == 0)
	{
		//Makes sure analog mode is off and laser power is set before imaging
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_ANALOG_MODE, FALSE, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_POWER, power1, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER2_POWER, power2, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER3_POWER, power3, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER4_POWER, power4, FALSE);
	}
	else
	{
		//Makes sure analog mode is on and power is set to 100 for all lasers before imaging
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_ANALOG_MODE, TRUE, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_POWER, 100, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER2_POWER, 100, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER3_POWER, 100, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER4_POWER, 100, FALSE);
	}
	
	//only set the emission if TTL mode is off
	if (allTTL == 0)
	{
		//Makes sure TTL mode is off and emission is turned on before imaging
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_TTL_MODE, FALSE, FALSE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_EMISSION, TRUE, FALSE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER2_EMISSION, TRUE, FALSE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER3_EMISSION, TRUE, FALSE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER4_EMISSION, TRUE, FALSE);
	}
	else 
	{
		//Makes sure TTL mode is on before imaging and the laser power is set
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_TTL_MODE, TRUE, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_POWER, power1, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER2_POWER, power2, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER3_POWER, power3, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER4_POWER, power4, FALSE);
	}
	//Make sure TTL and Analog mode are turned off before imaging if neither are enabled
	if (allAnalog != 1 && allTTL != 1)  
	{
		//Makes sure TTL and Analog modes are turned off before imaging via USB
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_ANALOG_MODE, FALSE, FALSE);
		SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER_ALL_TTL_MODE, FALSE, FALSE);
	}
	//Pass whether sequential is enabled and other parameters when applying the laser settings
	RunSample::getInstance()->SetLaser(enable1, power1, enable2, power2, enable3, power3, enable4, power4, captureSequenceEnable, allTTL, allAnalog, wavelength1, wavelength2, wavelength3, wavelength4);

	//Multiphoton Laser Parameters
	long multiphotonEnable,multiphotonPos,multiphotonSeqEnable,multiphotonSeqPos1,multiphotonSeqPos2;

	exp->GetMultiPhotonLaser(multiphotonEnable,multiphotonPos,multiphotonSeqEnable,multiphotonSeqPos1,multiphotonSeqPos2);

	//Set the laser to the position for imaging (for Multiphoton)		
	SetDeviceParamLong(SelectedHardware::SELECTED_LASER1,IDevice::PARAM_LASER1_POS,multiphotonPos,TRUE);

	if(multiphotonSeqEnable)
	{
		char seqBuf[4];

		seqBuf[0]=(multiphotonSeqPos1 & 0xFF00)>>8;
		seqBuf[1]=(multiphotonSeqPos1 & 0xFF);
		seqBuf[2]=(multiphotonSeqPos2 & 0xFF00)>>8;
		seqBuf[3]=(multiphotonSeqPos2 & 0xFF);

		SetDeviceParamBuffer(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_SEQ,seqBuf,sizeof(seqBuf),TRUE);
	}
	else
	{
		char seqBuf[4];
		seqBuf[0]=seqBuf[1]=seqBuf[2]=seqBuf[3]=0;

		SetDeviceParamBuffer(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_SEQ,seqBuf,sizeof(seqBuf),TRUE);
	}

	double p0 = 0, p1 = 0, p2 = 0, p3 = 0,p4 = 0,p5 = 0;

	const long POWER_REG_COUNT = 2;
	for(long i=0; i<POWER_REG_COUNT; i++)
	{
		long enable,type;
		double start,stop,zeroOffset;
		string path;

		exp->GetPower(enable,type, start, stop, path,zeroOffset,i);

		IDevice *pPowerReg = NULL;

		switch(i)
		{
		case 0:	
			pPowerReg = GetDevice(SelectedHardware::SELECTED_POWERREGULATOR);
			p4 = start;
			break;
		case 1:	
			pPowerReg = GetDevice(SelectedHardware::SELECTED_POWERREGULATOR2);
			p5 = start;
			break;
		}

		if(NULL == pPowerReg)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create PowerRegulator for PreCaptureProtocol");
		}
		else
		{
			//Set the power to the position for imaging
			pPowerReg->SetParam(IDevice::PARAM_POWER_ZERO_POS,zeroOffset);
			pPowerReg->SetupPosition();
			pPowerReg->StartPosition();
			pPowerReg->SetParam(IDevice::PARAM_POWER_POS,start);
			pPowerReg->SetupPosition();
			pPowerReg->StartPosition();
			RunSample::_hEventPowerReg = CreateEvent(0, FALSE, FALSE, 0);
			const long MAX_POWER_WAIT_TIME = 30000;
			StatusHandler(pPowerReg,&RunSample::_hEventPowerReg,MAX_POWER_WAIT_TIME);

			pPowerReg->PostflightPosition();
		}
	}

	ICamera * pCamera = NULL;

	pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);

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

		long type, blankPercent;
		double start,stop;
		string path;

		exp->GetPockels(i,type,start,stop,path,blankPercent);

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

	//Set pockels mask
	long pockelsMaskEnable = FALSE;
	long pockelsInvert = FALSE;
	string pockelsMaskPath = "";
	exp->GetPockelsMask(pockelsMaskEnable, pockelsInvert, pockelsMaskPath);
	pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_MASK_ENABLE_0, pockelsMaskEnable);
	pCamera->SetParam(ICamera::PARAM_LSM_POCKELS_MASK_INVERT_0, pockelsInvert);
	if(TRUE == pockelsMaskEnable)
	{
		RunSample::getInstance()->SetPockelsMask(pCamera, exp, pockelsMaskPath);
	}

	//This also updates image view between sequences
	long zPos = zCount;
	myFunctionPointerSaveZImage(&zPos, &p0, &p1, &p2, &p3, &p4, &p5);

	//Set Lighpath mirrors in position
	IDevice* pLightPath = NULL;
	double epiTurretAvailable = FALSE;
	long MAX_LIGHTPATH_WAIT_TIME = 4000;

	pLightPath = GetDevice(SelectedHardware::SELECTED_LIGHTPATH);

	if (NULL == pLightPath)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create LightPath for PreCaptureProtocol");
	}
	else
	{
		long galvoGalvo, galvoRes, camera, invertedLightPathPos, ndd;
		double scopeType = ScopeType::UPRIGHT, nddAvailable = FALSE;

		exp->GetLightPath(galvoGalvo, galvoRes, camera, invertedLightPathPos, ndd);

		if (TRUE == pLightPath->GetParam(IDevice::PARAM_SCOPE_TYPE, scopeType) && static_cast<long>(ScopeType::INVERTED) == scopeType)
		{
			pLightPath->SetParam(IDevice::PARAM_LIGHTPATH_INVERTED_POS, invertedLightPathPos);
			MAX_LIGHTPATH_WAIT_TIME = 9000;
		}
		else
		{
			//Set the laser to the position for imaging
			pLightPath->SetParam(IDevice::PARAM_LIGHTPATH_GG, galvoGalvo);
			pLightPath->SetParam(IDevice::PARAM_LIGHTPATH_GR, galvoRes);
			pLightPath->SetParam(IDevice::PARAM_LIGHTPATH_CAMERA, camera);
		}
		//Move the NDD mirror if the MCM6000 has the card configured
		if (TRUE == pLightPath->GetParam(IDevice::PARAM_LIGHTPATH_NDD_AVAILABLE, nddAvailable) && TRUE == nddAvailable)
		{
			pLightPath->SetParam(IDevice::PARAM_LIGHTPATH_NDD, ndd);
		}

		//Skip the movement commands for now if this is the MCM6000, the epi turret will move all stages including light path at once
		if (FALSE == pLightPath->GetParam(IDevice::PARAM_EPI_TURRET_AVAILABLE, epiTurretAvailable) || FALSE == epiTurretAvailable)
		{
			pLightPath->PreflightPosition();
			pLightPath->SetupPosition();
			pLightPath->StartPosition();

			RunSample::_hEventPowerReg = CreateEvent(0, FALSE, FALSE, 0);
			StatusHandler(pLightPath, &RunSample::_hEventPowerReg, MAX_LIGHTPATH_WAIT_TIME);

			pLightPath->PostflightPosition();
		}
	}

	//Set EpiTurret wheel position
	IDevice* pEpiTurret = NULL;

	pEpiTurret = GetDevice(SelectedHardware::SELECTED_EPITURRET);

	if (NULL == pEpiTurret)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create EpiTurret for PreCaptureProtocol");
	}
	else
	{
		long position;
		string name;
		long MAX_EPITURRET_WAIT_TIME = 4000;

		exp->GetEpiTurret(position, name);

		double pos = max(0, position - 1);

		pEpiTurret->SetParam(IDevice::PARAM_EPI_TURRET_POS, pos);


		if (TRUE == epiTurretAvailable)
		{
			//Check if the selected epi turret is the MCM6000, if it isn't call the movement commands from lightpath that we skipped
			if (TRUE == pEpiTurret->GetParam(IDevice::PARAM_EPI_TURRET_AVAILABLE, epiTurretAvailable))
			{
				MAX_EPITURRET_WAIT_TIME = 9000;
			}
			else
			{
				pLightPath->PreflightPosition();
				pLightPath->SetupPosition();
				pLightPath->StartPosition();

				RunSample::_hEventPowerReg = CreateEvent(0, FALSE, FALSE, 0);
				StatusHandler(pLightPath, &RunSample::_hEventPowerReg, MAX_LIGHTPATH_WAIT_TIME);

				pLightPath->PostflightPosition();
			}
		}

		pEpiTurret->PreflightPosition();
		pEpiTurret->SetupPosition();
		pEpiTurret->StartPosition();

		RunSample::_hEventPowerReg = CreateEvent(0, FALSE, FALSE, 0);
		StatusHandler(pEpiTurret, &RunSample::_hEventPowerReg, MAX_EPITURRET_WAIT_TIME);

		pEpiTurret->PostflightPosition();
	}

	//Set the pinhole position
	long pinholePosition, pinholeConnected;
	exp->GetPinholeWheel(pinholePosition);
SetDeviceParamLong(SelectedHardware::SELECTED_PINHOLEWHEEL, IDevice::PARAM_PINHOLE_POS, pinholePosition, TRUE);
// Get the position of the pinhole to check if it is connected. If not connected, and the capture mode is 
// sequential then pause for 50 miliseconds in order to display the captured image for every sequence.
if (!(GetDeviceParamLong(SelectedHardware::SELECTED_PINHOLEWHEEL, IDevice::PARAM_PINHOLE_POS, pinholeConnected)) && captureSequenceEnable)
{
	Sleep(50); //This number has been measured to be the minimum working number
}
}

void RunSample::PostCaptureProtocol(IExperiment* exp)
{
	//Stop the scanner if its connected

	IDevice* pControlUnit = NULL;

	pControlUnit = GetDevice(SelectedHardware::SELECTED_CONTROLUNIT);

	if (NULL != pControlUnit)
	{
		pControlUnit->SetParam(IDevice::PARAM_SCANNER_ENABLE, FALSE);
		pControlUnit->PreflightPosition();
		pControlUnit->SetupPosition();
		pControlUnit->StartPosition();
		pControlUnit->PostflightPosition();
	}

	IDevice* pPMT = NULL;

	pPMT = GetDevice(SelectedHardware::SELECTED_PMT1);

	if (NULL != pPMT)
	{
		pPMT->SetParam(IDevice::PARAM_SCANNER_ENABLE, FALSE);
		pPMT->SetParam(IDevice::PARAM_PMT1_ENABLE, FALSE);
		pPMT->PreflightPosition();
		pPMT->SetupPosition();
		pPMT->StartPosition();
		pPMT->PostflightPosition();
	}

	pPMT = GetDevice(SelectedHardware::SELECTED_PMT2);

	if (NULL != pPMT)
	{

		pPMT->SetParam(IDevice::PARAM_PMT2_ENABLE, FALSE);
		pPMT->PreflightPosition();
		pPMT->SetupPosition();
		pPMT->StartPosition();
		pPMT->PostflightPosition();
	}

	pPMT = GetDevice(SelectedHardware::SELECTED_PMT3);

	if (NULL != pPMT)
	{

		pPMT->SetParam(IDevice::PARAM_PMT3_ENABLE, FALSE);
		pPMT->PreflightPosition();
		pPMT->SetupPosition();
		pPMT->StartPosition();
		pPMT->PostflightPosition();
	}

	pPMT = GetDevice(SelectedHardware::SELECTED_PMT4);

	if (NULL != pPMT)
	{

		pPMT->SetParam(IDevice::PARAM_PMT4_ENABLE, FALSE);
		pPMT->PreflightPosition();
		pPMT->SetupPosition();
		pPMT->StartPosition();
		pPMT->PostflightPosition();
	}

	long photoBleachingEnable, laserPositiion, durationMS, bleachTrigger, preBleachingFrames, bleachWidth, bleachHeight, bleachOffsetX, bleachOffsetY, bleachingFrames, bleachFieldSize, postBleachingFrames1, postBleachingFrames2, preBleachingStream, postBleachingStream1, postBleachingStream2, powerEnable, laserEnable, bleachQuery, bleachPostTrigger, enableSimultaneousBleachingAndImaging, pmtEnableDuringBleach[4];
	double powerPosition, preBleachingInterval, postBleachingInterval1, postBleachingInterval2;

	long enable1, enable2, enable3, enable4, allTTL, allAnalog, wavelength1, wavelength2, wavelength3, wavelength4;
	double power1, power2, power3, power4;

	exp->GetPhotobleaching(photoBleachingEnable, laserPositiion, durationMS, powerPosition, bleachWidth, bleachHeight, bleachOffsetX, bleachOffsetY, bleachingFrames, bleachFieldSize, bleachTrigger, preBleachingFrames, preBleachingInterval, preBleachingStream, postBleachingFrames1, postBleachingInterval1, postBleachingStream1, postBleachingFrames2, postBleachingInterval2, postBleachingStream2, powerEnable, laserEnable, bleachQuery, bleachPostTrigger, enableSimultaneousBleachingAndImaging, pmtEnableDuringBleach[0], pmtEnableDuringBleach[1], pmtEnableDuringBleach[2], pmtEnableDuringBleach[3]);

	long multiphotonEnable, multiphotonPos, multiphotonSeqEnable, multiphotonSeqPos1, multiphotonSeqPos2;

	exp->GetMultiPhotonLaser(multiphotonEnable, multiphotonPos, multiphotonSeqEnable, multiphotonSeqPos1, multiphotonSeqPos2);

	exp->GetMCLS(enable1, power1, enable2, power2, enable3, power3, enable4, power4, allTTL, allAnalog, wavelength1, wavelength2, wavelength3, wavelength4);

	//disable the sequence mode at the end of the capture
	if (multiphotonSeqEnable)
	{
		char seqBuf[4];
		seqBuf[0] = seqBuf[1] = seqBuf[2] = seqBuf[3] = 0;

		SetDeviceParamBuffer(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_SEQ, seqBuf, sizeof(seqBuf), TRUE);
	}

	//Turn OFF all LEDs once acquisition is finished 
	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP, IDevice::PARAM_LEDS_ENABLE_DISABLE, FALSE, TRUE);

	//Disable Laser Emission and enable state once aquisition is finished if not MCLS
	if ((405 <= wavelength1) && (wavelength2 <= 785))
	{
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_EMISSION, DISABLE_EMISSION, FALSE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_EMISSION, DISABLE_EMISSION, FALSE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_EMISSION, DISABLE_EMISSION, FALSE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_EMISSION, DISABLE_EMISSION, FALSE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_ENABLE, DISABLE_EMISSION, FALSE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_ENABLE, DISABLE_EMISSION, FALSE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_ENABLE, DISABLE_EMISSION, FALSE);
		SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_ENABLE, DISABLE_EMISSION, FALSE);
	}
}

struct RunSampleThreadProcArgs {
	Observer* observer;
	Publisher* publisher;
};

UINT RunSampleThreadProc( LPVOID pParam )
{	
	IDevice *xyStage = GetDevice(SelectedHardware::SELECTED_XYSTAGE);

	if(NULL != xyStage)
	{	
		RunSample::getInstance()->_isActive = true;
		try
		{
			//reading from the experiment setup XML files
			IExperiment  *exp;

			RunSampleCustomParams rscp;
			RunSample::getInstance()->GetCustomParamsBinary((char*)&rscp);

			wstring ws(rscp.path);

			StringCbPrintfW(message,MSG_LENGTH,L"RunSample path is: %s",ws.c_str());
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

			exp = ExperimentManager::getInstance()->GetExperiment(ws);

			string camName;
			long camImageWidth;
			long camImageHeight;
			double pixelSize;
			double exposureTimeMS;		
			long gain, blackLevel, lightMode;				
			long left,top,right,bottom;
			long binningX, binningY;
			long tapsIndex, tapsBalance;
			long readoutSpeedIndex;
			long averageMode, averageNum;
			long vericalFlip, horizontalFlip, imageAngle, camChannel;
			long colorImageType, polarImageType;
			long isContinuousWhiteBalance, continuousWhiteBalanceNumFrames;
			double redGain, greenGain, blueGain;
			//getting the values from the experiment setup XML files
			exp->GetCamera(camName, camImageWidth, camImageHeight, pixelSize, exposureTimeMS, gain, blackLevel, lightMode, left, top, right, bottom, binningX, binningY, tapsIndex, tapsBalance, readoutSpeedIndex, averageMode, averageNum, vericalFlip, horizontalFlip, imageAngle, camChannel, colorImageType, polarImageType, isContinuousWhiteBalance, continuousWhiteBalanceNumFrames, redGain, greenGain, blueGain);

			long totalNumOfTiles = 0;
			long binning = 1;
			string objName;
			double magnification;
			exp->GetMagnification(magnification,objName);

			RunSample::getInstance()->SetMagnification(magnification,objName);

			const long ENABLE_Z_HOLDING_VOLOTAGE = TRUE;
			RunSample::getInstance()->SetupZStage(ENABLE_Z_HOLDING_VOLOTAGE);

			totalNumOfTiles = CreateSample(exp);
			RunSample::getInstance()->_totalNumOfTiles = totalNumOfTiles;

			long wavelengths = exp->GetNumberOfWavelengths();	

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

			//make sure the file only gets loaded once
			pHardware->SetFastLoad(TRUE);

			pHardware->GetMagInfoFromName(objName,magnification,position,numAperture,afStartPos,afFocusOffset,afAdaptiveOffset,beamExpPos,beamExpWavelength,beamExpPos2,beamExpWavelength2,turretPosition,zAxisToEscape,zAxisEscapeDistance, fineAutoFocusPercentage);

			long type,repeat;
			double expTimeMS,stepSizeUM,startPosMM,stopPosMM;

			//retrieve the autofocus parameters
			exp->GetAutoFocus(type,repeat,expTimeMS,stepSizeUM,startPosMM,stopPosMM);
			//Setup AutoFocus Module with the active parameters
			SetupAutofocus(type, repeat, afFocusOffset, expTimeMS, stepSizeUM, startPosMM, stopPosMM, binning, fineAutoFocusPercentage, FALSE);

			ICamera *pCamera = NULL;
			IDevice *pZStage = NULL;

			pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);
			pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

			SetupImageCorrection(exp, camImageWidth, camImageHeight);

			long zstageSteps, timePoints;
			string zstageName;
			double zstageStepSize, intervalSec,zStartPos,flybackTimeAdjustMS,volumeTimeAdjustMS,stepTimeAdjustMS;
			long streamEnable,streamFrames,rawData,triggerMode,triggerModeTimelapse,displayImage,storageMode,zFastEnable,zFastMode,flybackFrames,flybackLines,previewIndex,stimulusTriggering,dmaFrames,stimulusMaxFrames,useReferenceVoltageForFastZPockels;
			long photoBleachingEnable,laserPositiion,durationMS, bleachTrigger, preBleachingFrames,bleachWidth,bleachHeight,bleachOffsetX,bleachOffsetY, bleachingFrames,fieldSize, postBleachingFrames1, postBleachingFrames2,preBleachingStream,postBleachingStream1,postBleachingStream2,powerEnable,laserEnable,bleachQuery,bleachPostTrigger,enableSimultaneousBleachingAndImaging,pmtEnableDuringBleach[4];
			double powerPosition,preBleachingInterval,postBleachingInterval1,postBleachingInterval2;
			long zStreamFrames,zStreamMode,zEnable;
			long streamDisplayCumulativeAveragePreview = FALSE;
			exp->GetZStage(zstageName,zEnable,zstageSteps,zstageStepSize,zStartPos,zStreamFrames,zStreamMode);
			exp->GetTimelapse(timePoints,intervalSec,triggerModeTimelapse);
			exp->GetStreaming(streamEnable, streamFrames, rawData, triggerMode, displayImage, storageMode, zFastEnable, zFastMode, flybackFrames, flybackLines, flybackTimeAdjustMS, volumeTimeAdjustMS, stepTimeAdjustMS, previewIndex, stimulusTriggering, dmaFrames, stimulusMaxFrames, useReferenceVoltageForFastZPockels, streamDisplayCumulativeAveragePreview);
			exp->GetPhotobleaching(photoBleachingEnable, laserPositiion, durationMS, powerPosition, bleachWidth,bleachHeight,bleachOffsetX,bleachOffsetY, bleachingFrames, fieldSize, bleachTrigger,preBleachingFrames, preBleachingInterval,preBleachingStream, postBleachingFrames1, postBleachingInterval1,postBleachingStream1, postBleachingFrames2, postBleachingInterval2,postBleachingStream2,powerEnable,laserEnable,bleachQuery,bleachPostTrigger,enableSimultaneousBleachingAndImaging,pmtEnableDuringBleach[0],pmtEnableDuringBleach[1],pmtEnableDuringBleach[2],pmtEnableDuringBleach[3]);

			//SequentialTypes: 0 - between stacks, 1 - between frames
			long captureSequenceEnable = FALSE, sequentialType = 0;
			exp->GetCaptureSequence(captureSequenceEnable, sequentialType);

			//Get the Capture Sequence Settings
			vector<IExperiment::SequenceStep> captureSequence;
			exp->GetSequenceSteps(captureSequence);

			long captureMode;
			exp->GetCaptureMode(captureMode);

			//Z Stage steps based on th enable state of the z stage
			//Z & T  or Streaming with fastZ
			if(((0 == captureMode) && zEnable) || ((1 == captureMode)&& zFastEnable))
			{
				//use value stored in exp file
			}
			else
			{
				//force to a single z step
				zstageSteps = 1;
			}

			auto args = static_cast<RunSampleThreadProcArgs*>(pParam);
			Observer *pOb = (Observer*)args->observer;
			//passing the values from the XML as parameters

			AcquireFactory factory;

			auto_ptr<IAcquire> acq(NULL);

			switch(captureMode)
			{
			case IExperiment::STREAMING:
				{
					if(1 == storageMode)
					{
						pOb->SetTotalImagecount(INT_MAX, 0);
					}
					else
					{
						pOb->SetTotalImagecount(totalNumOfTiles * streamFrames, 0);
					}
					break;
				}
			case IExperiment::BLEACHING:
				{
					pOb->SetTotalImagecount(preBleachingFrames+postBleachingFrames1+postBleachingFrames2, 0);
					break;
				}
			case IExperiment::HYPERSPECTRAL:
				{
					pOb->SetTotalImagecount(totalNumOfTiles * streamFrames, 0);
					break;
				}
			default:
				{
					if((TRUE == captureSequenceEnable) &&(0 < captureSequence.size()))
					{
						pOb->SetTotalImagecount(totalNumOfTiles * zstageSteps * timePoints, static_cast<long>(captureSequence.size()));
					}
					else
					{
						pOb->SetTotalImagecount(totalNumOfTiles * zstageSteps * timePoints, 0);
					}

				}
			}

			if (1 == sequentialType && TRUE == captureSequenceEnable)
			{
				acq.reset(factory.getAcquireInstance(AcquireFactory::ACQ_T_SERIES, pOb, exp, ws));
			}
			else
			{
				acq.reset(factory.getAcquireInstance(AcquireFactory::ACQ_SEQUENCE, pOb, exp, ws));
			}
			acq->SetPublisher(args->publisher);

			//capture all of the well images
			sampleDll->GoToAllWellSites(xyStage, acq.get(), exp);

			RunSample::getInstance()->PostCaptureProtocol(exp);

			const long DISABLE_Z_HOLDING_VOLOTAGE = FALSE;
			RunSample::getInstance()->SetupZStage(DISABLE_Z_HOLDING_VOLOTAGE);
		}
		catch(...)
		{
			logDll->TLTraceEvent(CRITICAL_EVENT,1,L"RunSample Execute failed for an unknown reason");
		}
	}

	RunSample::getInstance()->_isActive = false;
	SAFE_DELETE_HANDLE(hRunSampleThread);
	return 0;
}

HANDLE RunSample::_hEventTurret = NULL;
HANDLE RunSample::_hEventBeamExpander = NULL;

void RunSample::SetMagnification(double mag,string objName)
{
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

	pHardware->GetMagInfoFromName(objName,mag,position,numAperture,afStartPos,afFocusOffset,afAdaptiveOffset,beamExpPos,beamExpWavelength,beamExpPos2,beamExpWavelength2,turretPosition,zAxisToEscape,zAxisEscapeDistance, fineAutoFocusPercentage);
	// getting the filter wheel positions
	IDevice *pBeamExpander = NULL;	

	pBeamExpander = GetDevice(SelectedHardware::SELECTED_BEAMEXPANDER);

	if(NULL == pBeamExpander)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample RunSampleThreadProc could not create beam expander");
		return;
	}

	//put expander into the proper mode and index
	const long BE_AUTO_MODE = 1;
	pBeamExpander->SetParam(IDevice::PARAM_BMEXP_MODE, BE_AUTO_MODE );
	pBeamExpander->SetParam(IDevice::PARAM_EXP_RATIO, beamExpPos );
	pBeamExpander->SetParam(IDevice::PARAM_EXP_RATIO2, beamExpPos2);
	pBeamExpander->SetParam(IDevice::PARAM_EXP_WAVELENGTH,beamExpWavelength);
	pBeamExpander->SetParam(IDevice::PARAM_EXP_WAVELENGTH2,beamExpWavelength2);

	pBeamExpander->PreflightPosition();

	pBeamExpander->SetupPosition ();

	pBeamExpander->StartPosition();

	_hEventBeamExpander = CreateEvent(0, FALSE, FALSE, 0);
	const long MAX_BEAMEXPANDER_WAIT_TIME = 10000;
	StatusHandler(pBeamExpander,&RunSample::_hEventBeamExpander,MAX_BEAMEXPANDER_WAIT_TIME);

	pBeamExpander->PostflightPosition();	
}


HANDLE RunSample::_hEventLaser = NULL;
HANDLE RunSample::_hEventPowerReg = NULL;

void RunSample::SetLaser(long enable1, double power1, long enable2, double power2, long enable3, double power3, long enable4, double power4, long captureSequenceEnable, long allTTL, long allAnalog, long wavelength1, long wavelength2, long wavelength3, long wavelength4)
{
	//Get filter parameters from hardware setup.xml
	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML());

	IDevice *pLaser = NULL;	
	for(long i=0; i<4; i++)
	{
		long laserWavelength1 = wavelength1;
		long laserWavelength2 = wavelength2;
		long laserWavelength3 = wavelength3;
		long laserWavelength4 = wavelength4;
		long sequential = captureSequenceEnable;
		long laserAnalog = allAnalog;
		long laserTTL = allTTL;

		switch(i)
		{
		case 0:			
			pLaser = GetDevice(SelectedHardware::SELECTED_LASER1);
			if(pLaser != NULL)
			{
				if (sequential == TRUE)
				{
					//Toptica Sequential laser 1 logic (only set power if laser enabled, set emission if TTL Mode is off)
					if ((405 <= laserWavelength1) && (laserWavelength1 <= 785))
					{
						if (enable1 == TRUE) 
						{
							SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_POWER, power1, FALSE);
						}
						SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_ENABLE, enable1, FALSE);
						if (laserTTL == FALSE)
						{
							SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_EMISSION, ENABLE_EMISSION, FALSE);
						}
					}
					//If sequential is enabled, enable laser 1 and set its power (old MCLS logic)
					if (laserAnalog == false && laserTTL == false) 
					{
						pLaser->SetParam(IDevice::PARAM_LASER1_ENABLE, enable1);
						pLaser->SetParam(IDevice::PARAM_LASER1_POWER, power1);
						RunLaser(pLaser);
					}
				}
				else 
				{
					//If sequential is not enabled, set the power, enable state (and emission state for laser 1 if not in TTL Mode)
					if (laserTTL == FALSE)
					{
						SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_EMISSION, ENABLE_EMISSION, FALSE);
					}
					SetDeviceParamDouble(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_POWER, power1, FALSE);
					SetDeviceParamLong(SelectedHardware::SELECTED_LASER1, IDevice::PARAM_LASER1_ENABLE, enable1, FALSE);
					pLaser->SetParam(IDevice::PARAM_LASER1_ENABLE, enable1);
					pLaser->SetParam(IDevice::PARAM_LASER1_POWER, power1);
					RunLaser(pLaser);	
				}
			}
			break;
		case 1:
			pLaser = GetDevice(SelectedHardware::SELECTED_LASER2);
			if(pLaser != NULL)
			{
				if (sequential == TRUE)
				{
					//Toptica Sequential laser 2 logic (only set power if laser enabled, set emission if TTL Mode is off)
					if ((405 <= laserWavelength2) && (laserWavelength2 <= 785))
					{
						if (enable2 == TRUE) 
						{
							SetDeviceParamDouble(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_POWER, power2, FALSE);
						}
						SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_ENABLE, enable2, FALSE);
						if (laserTTL == FALSE)
						{
							SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_EMISSION, ENABLE_EMISSION, FALSE);
						}
					}
					//If sequential is enabled, enable laser 2 and set its power (old MCLS logic)
					if (laserAnalog == FALSE && laserTTL == FALSE)
					{
						pLaser->SetParam(IDevice::PARAM_LASER2_ENABLE, enable2);
						pLaser->SetParam(IDevice::PARAM_LASER2_POWER, power2);
						RunLaser(pLaser);
					}
				}
				else
				{
					//If sequential is not enabled, set the power, enable state (and emission state for laser 2 if not in TTL Mode)
					if (laserTTL == false)
					{
						SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_EMISSION, ENABLE_EMISSION, FALSE);
					}
					SetDeviceParamDouble(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_POWER, power2, FALSE);
					SetDeviceParamLong(SelectedHardware::SELECTED_LASER2, IDevice::PARAM_LASER2_ENABLE, enable2, FALSE);
					pLaser->SetParam(IDevice::PARAM_LASER2_ENABLE, enable2);
					pLaser->SetParam(IDevice::PARAM_LASER2_POWER, power2);
					RunLaser(pLaser);
				}
			}
			break;
		case 2:
			pLaser = GetDevice(SelectedHardware::SELECTED_LASER3);
			if(pLaser != NULL)
			{
				if (sequential == TRUE)
				{
					//Toptica Sequential laser 3 logic (only set power if laser enabled, set emission if TTL Mode is off)

					if ((405 <= laserWavelength3) && (laserWavelength3 <= 785))
					{
						if (enable3 == TRUE) 
						{
							SetDeviceParamDouble(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_POWER, power3, FALSE);
						}
						SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_ENABLE, enable3, FALSE);
						if (laserTTL == FALSE)
						{
							SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_EMISSION, ENABLE_EMISSION, FALSE);
						}
					}
					//If sequential is enabled, enable laser 3 and set its power (old MCLS logic)
					if (laserAnalog == FALSE && laserTTL == FALSE)
					{
						pLaser->SetParam(IDevice::PARAM_LASER3_ENABLE, enable3);
						pLaser->SetParam(IDevice::PARAM_LASER3_POWER, power3);
						RunLaser(pLaser);
					}
				}
				else
				{
					//If sequential is not enabled, set the power, enable state (and emission state for laser 3 if not in TTL Mode)
					if (laserTTL == FALSE)
					{
						SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_EMISSION, ENABLE_EMISSION, FALSE);
					}
					SetDeviceParamDouble(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_POWER, power3, FALSE);
					SetDeviceParamLong(SelectedHardware::SELECTED_LASER3, IDevice::PARAM_LASER3_ENABLE, enable3, FALSE);
					pLaser->SetParam(IDevice::PARAM_LASER3_ENABLE, enable3);
					pLaser->SetParam(IDevice::PARAM_LASER3_POWER, power3);
					RunLaser(pLaser);
				}
			}
			break;
		case 3:
			pLaser = GetDevice(SelectedHardware::SELECTED_LASER4);
			if (pLaser != NULL)
			{
				if (sequential == TRUE)
				{
					//Toptica Sequential laser 3 logic (only set power if laser enabled, set emission if TTL Mode is off)
					if ((405 <= laserWavelength4) && (laserWavelength4 <= 785))
					{
						if (enable4 == TRUE) 
						{
							SetDeviceParamDouble(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_POWER, power4, FALSE);
						}
						SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_ENABLE, enable4, FALSE);
						if (laserTTL == FALSE) 
						{
							SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_EMISSION, ENABLE_EMISSION, FALSE);
						}
					}
					//If sequential is enabled, enable laser 4 and set its power (old MCLS logic)
					if (laserAnalog == FALSE && laserTTL == FALSE)
					{
						pLaser->SetParam(IDevice::PARAM_LASER4_ENABLE, enable4);
						pLaser->SetParam(IDevice::PARAM_LASER4_POWER, power4);
						RunLaser(pLaser);
					}
				}
				else 
				{
					//If sequential is not enabled, set the power, enable state (and emission state for laser 4 if not in TTL Mode)
					if (laserTTL == FALSE)
					{
						SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_EMISSION, ENABLE_EMISSION, FALSE);
					}
					SetDeviceParamDouble(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_POWER, power4, FALSE);
					SetDeviceParamLong(SelectedHardware::SELECTED_LASER4, IDevice::PARAM_LASER4_ENABLE, enable4, FALSE);
					pLaser->SetParam(IDevice::PARAM_LASER4_ENABLE, enable4);
					pLaser->SetParam(IDevice::PARAM_LASER4_POWER, power4);
					RunLaser(pLaser);
				}
			}
			break;
		}	
	}
}

void RunSample::RunLaser(IDevice* pLa)
{				
	pLa->StartPosition();
	RunSample::_hEventLaser = CreateEvent(0, FALSE, FALSE, 0);
	const long MAX_LASER_WAIT_TIME = 10000;
	StatusHandler(pLa,&RunSample::_hEventLaser,MAX_LASER_WAIT_TIME);
	pLa->PostflightPosition();
}

void RunSample::SetupZStage(long enableHoldingVoltage)
{
	IDevice *pZStage = NULL;	

	pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

	if(pZStage != NULL)
	{
		long paramType;
		long paramAvailable;
		long paramReadOnly;
		double paramMin;
		double paramMax;
		double paramDefault;

		pZStage->GetParamInfo(IDevice::PARAM_Z_ENABLE_HOLDING_VOLTAGE, paramType,paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);

		if(paramAvailable && !paramReadOnly)
		{
			//do not move the Z Axis
			//Read the current location and set it as the target position
			double pos;
			pZStage->GetParam(IDevice::PARAM_Z_POS_CURRENT,pos);
			pZStage->SetParam(IDevice::PARAM_Z_POS,pos);

			HANDLE hEventZ = CreateEvent(0, FALSE, FALSE, 0);
			SetDeviceParameterValue(pZStage,IDevice::PARAM_Z_ENABLE_HOLDING_VOLTAGE,enableHoldingVoltage,TRUE,hEventZ,5000);
		}
	}
}

long RunSample::Execute()
{
	//single thread only
	if((_isActive) || NULL != hRunSampleThread)
		return FALSE;

	RunSampleThreadProcArgs args = { &_ob, impl.get() };
	hRunSampleThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) RunSampleThreadProc, (LPVOID)&args, 0, &dwRunSampleThreadId );
	return TRUE;
}

long RunSample::Stop()
{
	bool stopFlag = TRUE;
	_ob.SetStopCapture(stopFlag);
	StopCamera();

	return TRUE;
}

long RunSample::Status(long &status)
{
	status = ICommand::STATUS_READY;

	return TRUE;
}

long RunSample::InitializeCustomParameters()
{	
	return TRUE;
}

long RunSample::SetupCommand()
{
	if(FALSE == _setupFlag)
	{	
		_setupFlag = TRUE;
    impl->setup();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

long RunSample::TeardownCommand()
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

UINT SafetyInterlockStatusCheck()
{
	long safetyInterLockState = -1;
	while (TRUE == _shutterOpened)
	{
		if (TRUE == GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::Params::PARAM_SHUTTER_SAFETY_INTERLOCK_STATE, safetyInterLockState))
		{
			if (FALSE == safetyInterLockState)
			{
				wstring messageWstring = L"Safety Interlock is engaged or not installed. Please check if the trinoc is in eyepiece mode. \nIf error persists please contact techsupport@thorlabs.com.";
				MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Primary path shutter closed.", MB_OK);
				RunSample::getInstance()->Stop();
				return FALSE;
			}
		}
		else
		{
			logDll->TLTraceEvent(ERROR_EVENT, 1, L"RunSample SafetyInterlockStatusCheck: unable get param PARAM_SHUTTER_SAFETY_INTERLOCK_STATE");
			return FALSE;
		}
		Sleep(1000);
	}
	return TRUE;
}

void InitiateSafetyInterlockStatusCheck()
{
	long scopeType = ScopeType::UPRIGHT;
	long safetyInterlockCheckEnabled = FALSE;
	long shutterAvailable = FALSE;
	long ret = FALSE;

	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML);
	ret = pHardware->GetInvertedSettings(safetyInterlockCheckEnabled);

	if ((TRUE == GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::Params::PARAM_SCOPE_TYPE, scopeType) && ScopeType::INVERTED == scopeType) &&
		(TRUE == ret && TRUE == safetyInterlockCheckEnabled) &&
		(TRUE == GetDeviceParamLong(SelectedHardware::SELECTED_LIGHTPATH, IDevice::Params::PARAM_SHUTTER_AVAILABLE, shutterAvailable) && TRUE == shutterAvailable))
	{
		_shutterOpened = TRUE;
		SAFE_DELETE_HANDLE(_hSafetyInterLockCheckThread);
		_hSafetyInterLockCheckThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SafetyInterlockStatusCheck, NULL, 0, &_dwSafetyInterLockCheckThreadId);
		SetThreadPriority(_hSafetyInterLockCheckThread, THREAD_PRIORITY_LOWEST);
	}
}

long OpenShutter()
{	
	long ret = TRUE;

	IDevice *pShutter = NULL;

	pShutter = GetDevice(SelectedHardware::SELECTED_SHUTTER1);

	if(NULL == pShutter)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create shutter device");
		return FALSE;
	}

	const long OPEN_SHUTTER_POS = 0;
	ret = pShutter->SetParam(IDevice::PARAM_SHUTTER_POS, OPEN_SHUTTER_POS);
	if(FALSE == ret)
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"RunSample Execute could not set shutter open param");
	}
	pShutter->PreflightPosition();

	pShutter->SetupPosition ();

	ret = pShutter->StartPosition();

	HANDLE hEventShutter = CreateEvent(0, FALSE, FALSE, 0);
	const long MAX_SHUTTER_WAIT_TIME = 100;
	StatusHandler(pShutter,&hEventShutter,MAX_SHUTTER_WAIT_TIME);

	pShutter->PostflightPosition();	

	InitiateSafetyInterlockStatusCheck();

	if(FALSE == ret)
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"RunSample Execute could not open shutter");
	}
	return ret;
}

long CloseShutter()
{	
	long ret = TRUE;

	IDevice *pShutter = NULL;

	//Notify the safety interlock check thread the shutter is closed
	_shutterOpened = FALSE;

	pShutter = GetDevice(SelectedHardware::SELECTED_SHUTTER1);

	if(NULL == pShutter)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample Execute could not create shutter device");
		return FALSE;
	}

	const long CLOSE_SHUTTER_POS = 1;
	ret = pShutter->SetParam(IDevice::PARAM_SHUTTER_POS, CLOSE_SHUTTER_POS);
	if(FALSE == ret)
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"RunSample Execute could not set shutter close param");
	}
	pShutter->PreflightPosition();

	pShutter->SetupPosition ();

	ret = pShutter->StartPosition();

	HANDLE hEventShutter = CreateEvent(0, FALSE, FALSE, 0);
	const long MAX_SHUTTER_WAIT_TIME = 100;
	StatusHandler(pShutter,&hEventShutter,MAX_SHUTTER_WAIT_TIME);

	pShutter->PostflightPosition();	

	if(FALSE == ret)
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"RunSample Execute could not close shutter");
	}
	return ret;
}

long ScannerEnableProc(long cameraOrBleachScanner, long enable)
{
	//Only enable the scanner if it is a resonance scanner
	ICamera *pCamera = NULL;

	//values for cameraOrBleachScanner
	//0 - check type of active camera
	//1 - check type of active bleach scanner 


	if(0 == cameraOrBleachScanner)
	{
		pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);
	}
	else
	{
		pCamera = GetCamera(SelectedHardware::SELECTED_BLEACHINGSCANNER);
	}

	if(NULL == pCamera)
	{	
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample ScannerEnable could not create camera");
		return FALSE;
	}

	double dVal;
	if(FALSE == pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,dVal))
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample ScannerEnable could not get camera type");
		return FALSE;
	}

	if(ICamera::LSM != static_cast<long>(dVal))
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample ScannerEnable not an LSM");
		return FALSE;
	}

	if(FALSE == pCamera->GetParam(ICamera::PARAM_LSM_TYPE,dVal))
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample ScannerEnable could not get camera type");
		return FALSE;
	}

	IDevice * pControlUnit = NULL;

	pControlUnit = GetDevice(SelectedHardware::SELECTED_CONTROLUNIT);

	if(NULL == pControlUnit)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample ScannerEnable could not create ControlUnit");
		return FALSE;
	}
	switch ((ICamera::LSMType)static_cast<long>(dVal))
	{
	case ICamera::LSMType::GALVO_RESONANCE:
	case ICamera::LSMType::RESONANCE_GALVO_GALVO:
		pControlUnit->SetParam(IDevice::PARAM_SCANNER_ENABLE, enable);
		pControlUnit->PreflightPosition();
		pControlUnit->SetupPosition();
		pControlUnit->StartPosition();
		pControlUnit->PostflightPosition();

		if (TRUE == enable)
		{
			//wait for Resonance scanner to be stable, 
			//required especially for single channel acquisition:
			Sleep(300);	//140
		}
		break;
	default:
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"RunSample ScannerEnable not a resonance scanner");
		return FALSE;
	}

	return TRUE;
}

long SetPMTProc(IExperiment* pExp)
{
	long enableA,bandwidthA,enableB,bandwidthB,enableC,bandwidthC,enableD,bandwidthD;
	double gainA,offsetA,gainB,offsetB,gainC,offsetC,gainD,offsetD;
	pExp->GetPMT(enableA,gainA,bandwidthA,offsetA,enableB,gainB,bandwidthB,offsetB,enableC,gainC,bandwidthC,offsetC,enableD,gainD,bandwidthD,offsetD);

	IDevice * pPMT = NULL;

	pPMT = GetDevice(SelectedHardware::SELECTED_PMT1);

	if(NULL != pPMT)
	{
		pPMT->SetParam(IDevice::PARAM_PMT1_ENABLE, enableA);
		pPMT->SetParam(IDevice::PARAM_PMT1_GAIN_POS, gainA);
		pPMT->SetParam(IDevice::PARAM_PMT1_BANDWIDTH_POS, bandwidthA);
		pPMT->SetParam(IDevice::PARAM_PMT1_OUTPUT_OFFSET, offsetA);
		pPMT->PreflightPosition();
		pPMT->SetupPosition ();
		pPMT->StartPosition();
		pPMT->PostflightPosition();
	}

	pPMT = GetDevice(SelectedHardware::SELECTED_PMT2);

	if(NULL != pPMT)
	{
		pPMT->SetParam(IDevice::PARAM_PMT2_ENABLE, enableB);
		pPMT->SetParam(IDevice::PARAM_PMT2_GAIN_POS, gainB);
		pPMT->SetParam(IDevice::PARAM_PMT2_BANDWIDTH_POS, bandwidthB);
		pPMT->SetParam(IDevice::PARAM_PMT2_OUTPUT_OFFSET, offsetB);
		pPMT->PreflightPosition();
		pPMT->SetupPosition ();
		pPMT->StartPosition();
		pPMT->PostflightPosition();
	}

	pPMT = GetDevice(SelectedHardware::SELECTED_PMT3);

	if(NULL != pPMT)
	{
		pPMT->SetParam(IDevice::PARAM_PMT3_ENABLE, enableC);
		pPMT->SetParam(IDevice::PARAM_PMT3_GAIN_POS, gainC);
		pPMT->SetParam(IDevice::PARAM_PMT3_BANDWIDTH_POS, bandwidthC);
		pPMT->SetParam(IDevice::PARAM_PMT3_OUTPUT_OFFSET, offsetC);
		pPMT->PreflightPosition();
		pPMT->SetupPosition ();
		pPMT->StartPosition();
		pPMT->PostflightPosition();
	}

	pPMT = GetDevice(SelectedHardware::SELECTED_PMT4);

	if(NULL != pPMT)
	{
		pPMT->SetParam(IDevice::PARAM_PMT4_ENABLE, enableD);
		pPMT->SetParam(IDevice::PARAM_PMT4_GAIN_POS, gainD);
		pPMT->SetParam(IDevice::PARAM_PMT4_BANDWIDTH_POS, bandwidthD);
		pPMT->SetParam(IDevice::PARAM_PMT4_OUTPUT_OFFSET, offsetD);
		pPMT->PreflightPosition();
		pPMT->SetupPosition ();
		pPMT->StartPosition();
		pPMT->PostflightPosition();
	}
	return TRUE;
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

	ResetEvent(RunSample::_hStopStatusThread);

	HANDLE hThread = ::CreateThread( NULL, 0,(LPTHREAD_START_ROUTINE)StatusDeviceThreadProc, &statusParams, 0, &dwThread );

	DWORD dwWait = WaitForSingleObject(*pEvent, maxWaitTime);

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"StatusHandlerDevice  StatusThreadProc  failed");

		//signal to terminate the status thread:
		SetEvent(RunSample::_hStopStatusThread);
		if(WaitForSingleObject(*pEvent, maxWaitTime) != WAIT_OBJECT_0)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"Terminate  StatusDeviceThreadProc  failed");
		}
		ret = FALSE;
	}		

	CloseHandle(hThread);
	CloseHandle(*pEvent);
	return ret;
}
long SetDeviceParameterValue(IDevice *pDevice,long paramID, double val,long bWait,HANDLE hEvent,long waitTime)
{
	long ret = FALSE;

	if(NULL == pDevice)
	{
		StringCbPrintfW(message,MSG_LENGTH,L"%hs@%u: pDevice is NULL", __FUNCTION__, __LINE__);
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

	if(pDevice->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		if(paramAvailable)
		{
			ret = pDevice->SetParam(paramID,val);
			pDevice->PreflightPosition();
			pDevice->SetupPosition();
			pDevice->StartPosition();

			if(bWait)
			{
				ret = StatusHandlerDevice(pDevice,&hEvent,waitTime);
			}

			pDevice->PostflightPosition();
		}
	}
	return ret;
}

double GetCustomPowerValue(double zStart, double zStop, double zPos, string path)
{
	double result = 0;

	double percentOfZRange = min(100.0, 100.0*abs((zPos-zStart)/(zStop-zStart)));

	double lastZPercent = -1;
	double lastPower = 0;

	std::ifstream infile(path);

	double pos,power;
	char c;

	while((infile >> pos >> c >> power) && (c == ','))
	{
		//piecewise interpolate
		if((percentOfZRange > lastZPercent) && (percentOfZRange <= pos))
		{
			if(lastZPercent < 0)
			{
				//first point do not need interpolation
				result = power;
			}
			else
			{
				double slope = (power - lastPower)/(pos - lastZPercent);

				result = lastPower + slope * (percentOfZRange-lastZPercent);
			}
			break;
		}

		lastZPercent = pos;
		lastPower = power;
	}

	infile.close();
	return result;
}

/// <summary> Commands the active camera to stop waiting for any hardware events it may currently be waiting on </summary>
/// <returns> Returns false if no active camera was found </returns>
long RunSample::StopCamera()
{
	SetCameraParamLong(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_LSM_STOP_ACQUISITION,TRUE);

	return TRUE;
}

double GetTiltAdjustZValue(double fPt1X,double fPt1Y,double fPt1Z,double fPt2X,double fPt2Y,double fPt2Z,double fPt3X,double fPt3Y,double fPt3Z,double ptX,double ptY)
{
	double a,b,c,d;

	a = (fPt2Y-fPt1Y)*(fPt3Z-fPt1Z)-(fPt2Z-fPt1Z)*(fPt3Y-fPt1Y);
	b = (fPt2Z-fPt1Z)*(fPt3X-fPt1X)-(fPt2X-fPt1X)*(fPt3Z-fPt1Z);
	c = (fPt2X-fPt1X)*(fPt3Y-fPt1Y)-(fPt2Y-fPt1Y)*(fPt3X-fPt1X);
	d = -1*(a*fPt1X + b*fPt1Y + c*fPt1Z);

	double zVal = -1 *( d + a * ptX + b*ptY)/c;
	return zVal;
}

void GetZPositions(IExperiment* exp, IDevice* pZStage, double &zStartPos, double &zStopPos, double &zTiltPos, double &zStepSizeMM, long &zstageSteps, long &zStreamFrames, long &zStreamMode)
{
	double sampleOffsetX, sampleOffsetY, sampleOffsetZ;
	long tiltAdjustment;
	double fPt[3][3];
	exp->GetSample(sampleOffsetX,sampleOffsetY,sampleOffsetZ,tiltAdjustment,fPt[0][0],fPt[0][1],fPt[0][2],fPt[1][0],fPt[1][1],fPt[1][2],fPt[2][0],fPt[2][1],fPt[2][2]);

	//no tilt if no tiles is active
	long camType = 0, lsmType = ICamera::LSMType::LSMTYPE_LAST, mROIEnabled;
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_CAMERA_TYPE, camType);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_LSM_TYPE, lsmType);
	GetCameraParamLong(SelectedHardware::SELECTED_CAMERA1, ICamera::PARAM_MROI_MODE_ENABLE, mROIEnabled);
	vector<IExperiment::SubImage> SubImages; 	
	exp->GetSubImages(SubImages, camType, lsmType, mROIEnabled);
	if (0 == SubImages.size()) 
		tiltAdjustment = 0;

	//get capturemode
	long captureMode;
	exp->GetCaptureMode(captureMode);

	//get zstage
	long timePoints,triggerModeTimelapse,zEnable;
	string zstageName;
	double intervalSec, zstageStepSize;
	exp->GetTimelapse(timePoints,intervalSec,triggerModeTimelapse);
	exp->GetZStage(zstageName, zEnable,zstageSteps,zstageStepSize,zStartPos,zStreamFrames,zStreamMode);

	//get zstream
	long streamEnable,streamFrames,rawData,triggerMode,displayImage,storageMode,zFastEnable,zFastMode,flybackFrames,flybackLines,previewIndex,stimulusTriggering,dmaFrames,stimulusMaxFrames,useReferenceVoltageForFastZPockels;
	double flybackTimeAdjustMS, volumeTimeAdjustMS,stepTimeAdjustMS;
	long streamDisplayCumulativeAveragePreview = FALSE;
	exp->GetStreaming(streamEnable, streamFrames, rawData, triggerMode, displayImage, storageMode, zFastEnable, zFastMode, flybackFrames, flybackLines, flybackTimeAdjustMS, volumeTimeAdjustMS, stepTimeAdjustMS, previewIndex, stimulusTriggering, dmaFrames, stimulusMaxFrames, useReferenceVoltageForFastZPockels, streamDisplayCumulativeAveragePreview);

	zStepSizeMM = zstageStepSize / Constants::UM_TO_MM;

	//stepping less than one is not realistic experiment configuration.
	zstageSteps = max(1,zstageSteps);

	//default Tilt position at z center
	zTiltPos = zStartPos + (zstageSteps * zStepSizeMM) / 2.0;

	//if a tilt adjust is required. Calculate the value
	if(TRUE == tiltAdjustment)
	{
		//read the current X and Y position
		double xPos,yPos;
		GetDeviceParamDouble(SelectedHardware::SELECTED_XYSTAGE, IDevice::PARAM_X_POS_CURRENT, xPos);		
		GetDeviceParamDouble(SelectedHardware::SELECTED_XYSTAGE, IDevice::PARAM_Y_POS_CURRENT, yPos);
		zTiltPos = GetTiltAdjustZValue(fPt[0][0],fPt[0][1],fPt[0][2],fPt[1][0],fPt[1][1],fPt[1][2],fPt[2][0],fPt[2][1],fPt[2][2],xPos,yPos);
	}

	//configure Z positions
	if((IExperiment::Z_AND_T == captureMode) && (FALSE == zEnable) ||
		((IExperiment::STREAMING == captureMode) && (FALSE == zFastEnable)))
	{
		zstageSteps = 1;		
		zStreamMode = 0;
		zStreamFrames = 1;
		GetDeviceParamDouble(SelectedHardware::SELECTED_ZSTAGE, IDevice::PARAM_Z_POS_CURRENT, zStartPos);

		//if a tilt adjust is required. Calculate the value
		//and move to the new Z position
		if(TRUE == tiltAdjustment)
		{
			zStartPos = zTiltPos;
			SetDeviceParamDouble(SelectedHardware::SELECTED_ZSTAGE, IDevice::PARAM_Z_POS,zStartPos, TRUE);
		}
	}
	else
	{
		//if a tilt adjust is required. Calculate the value
		//and reassign the Z start position
		if(TRUE == tiltAdjustment)
		{
			//new start pos = Tilt position (new center) - distance from start to center (if start > stop then zStepSizeMM will be negative)
			zStartPos = zTiltPos - (zstageSteps * zStepSizeMM) / 2.0;
		}
	}
	//calculate Z stop position, allow single step
	zStopPos = (1 < zstageSteps) ? zStartPos + ((zstageSteps-1) * zStepSizeMM) : zStartPos + (zstageSteps * zStepSizeMM);
	//Need to round zStopPos to 0.1um. Otherwise Z&T won't reach the max Z position if we are going the full range e.g. Piezo 0-450um
	zStopPos = (ceil(zStopPos * 10000 - 0.5f)) / 10000;

	//validate z stage steps by setting z stop position (PIEZO only):
	if(NULL != pZStage)
	{
		double zStageType = 0, z_max = 0, z_min = 0, zDefault = 0;
		long zType = 0, zAvailable = 0, zReadOnly = 0;
		if(pZStage->GetParam(IDevice::PARAM_Z_STAGE_TYPE, zStageType) && (static_cast<long>(ZStageType::PIEZO) == static_cast<long>(zStageType)))
		{
			//stop position may be out of range 
			//since only steps and start position were persisted:
			if(FALSE == pZStage->SetParam(IDevice::PARAM_Z_FAST_STOP_POS,zStopPos))
			{
				//try to keep within limit by decreasing steps:
				pZStage->GetParamInfo(IDevice::PARAM_Z_POS,zType,zAvailable,zReadOnly,z_min,z_max,zDefault);
				double val_limit = max(z_min,min(z_max,zStopPos));
				int stepDiff = static_cast<int>(ceil(abs(val_limit-zStopPos)/abs(zStepSizeMM)));
				zstageSteps -= stepDiff;
				zStopPos = (1 < zstageSteps) ? max(z_min,min(z_max,zStartPos + ((zstageSteps-1) * zStepSizeMM))) :  max(z_min,min(z_max,zStartPos + (zstageSteps * zStepSizeMM)));
				pZStage->SetParam(IDevice::PARAM_Z_FAST_STOP_POS, zStopPos);
			}
		}
	}
}

void SetPower(IExperiment* exp, ICamera* camera, double zPos, double &power0, double &power1, double &power2, double &power3, double &power4, double &power5)
{
	double zStartPos, zStopPos, zTiltPos, zStepSizeMM;
	long zstageSteps, zStreamFrames, zStreamMode;
	IDevice* pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);
	GetZPositions(exp, pZStage, zStartPos, zStopPos, zTiltPos, zStepSizeMM, zstageSteps, zStreamFrames, zStreamMode);

	long enablePower,typePower,blankPercent;
	double startPower,stopPower,zeroOffset;
	string pathPower;

	//set the power regulators power
	const long EXPONENTIAL_POWER_RAMP = 1;
	const long POWER_REG_COUNT = 2;
	for(long i=0; i<POWER_REG_COUNT; i++)
	{
		exp->GetPower(enablePower,typePower,startPower,stopPower,pathPower,zeroOffset,i);

		double power = (typePower == EXPONENTIAL_POWER_RAMP) ? GetCustomPowerValue(zStartPos,zStopPos,zPos,pathPower) : startPower;

		switch(i)
		{
		case 0:
			SetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR,IDevice::PARAM_POWER_POS,power,TRUE);
			power4 = power;
			break;
		case 1:
			SetDeviceParamDouble(SelectedHardware::SELECTED_POWERREGULATOR2,IDevice::PARAM_POWER2_POS,power,TRUE);
			power5 = power;
			break;
		}
	}
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

		exp->GetPockels(i,typePower,startPower,stopPower,pathPower,blankPercent);
		double power = (typePower == EXPONENTIAL_POWER_RAMP) ? GetCustomPowerValue(zStartPos,zStopPos,zPos,pathPower) : startPower;
		camera->SetParam(param1, power);
		camera->SetParam(param2, blankPercent);
		switch(i)
		{
		case 0: power0 = power; break;
		case 1: power1 = power; break;
		case 2: power2 = power; break;
		case 3: power3 = power; break;
		}
	}
}

void SetLEDs(IExperiment* exp, ICamera* camera, double zPos, double &power1, double &power2, double &power3, double &power4, double &power5, double &power6)
{
	/*double zStartPos, zStopPos, zTiltPos, zStepSizeMM;
	long zstageSteps, zStreamFrames, zStreamMode;
	IDevice* pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);
	GetZPositions(exp, pZStage, zStartPos, zStopPos, zTiltPos, zStepSizeMM, zstageSteps, zStreamFrames, zStreamMode);*/

	long const CHROLIS_RANGE_CONVERSION = 10;
	long led1enable = FALSE, led2enable = FALSE, led3enable = FALSE, led4enable = FALSE, led5enable = FALSE, led6enable = FALSE;
	double led1power = 0, led2power = 0, led3power = 0, led4power = 0, led5power = 0, led6power = 0, mainPower = 0;

	//Enable the LEDs and set the power
	exp->GetLampLED(led1enable, led1power, led2enable, led2power, led3enable, led3power, led4enable, led4power, led5enable, led5power, led6enable, led6power, mainPower);

	//Keep in mind the range of the power of the Chrolis is from 1 - 1000, we need to multiply the value read from the experiment.xml by 10
	//Set the Main Power first then each individual LED power. Otherwise it will transform the individual powers if we do the main power last
	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP, IDevice::PARAM_LEDS_LINEAR_VALUE, mainPower * CHROLIS_RANGE_CONVERSION, TRUE);

	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED1_POWER_STATE,led1enable,TRUE);
	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED1_POWER,led1power * CHROLIS_RANGE_CONVERSION,TRUE);
	power1 = led1power;

	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED2_POWER_STATE,led2enable,TRUE);
	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED2_POWER,led2power * CHROLIS_RANGE_CONVERSION,TRUE);
	power2 = led2power;

	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED3_POWER_STATE,led3enable,TRUE);
	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED3_POWER,led3power * CHROLIS_RANGE_CONVERSION,TRUE);
	power3 = led3power;

	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED4_POWER_STATE,led4enable,TRUE);
	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED4_POWER,led4power * CHROLIS_RANGE_CONVERSION,TRUE);
	power4 = led4power;

	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED5_POWER_STATE,led5enable,TRUE);
	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED5_POWER,led5power * CHROLIS_RANGE_CONVERSION,TRUE);
	power5 = led5power;

	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED6_POWER_STATE,led6enable,TRUE);
	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP,IDevice::PARAM_LED6_POWER,led6power * CHROLIS_RANGE_CONVERSION,TRUE);
	power6 = led6power;

	//Turn ON LEDS 
	SetDeviceParamDouble(SelectedHardware::SELECTED_BFLAMP, IDevice::PARAM_LEDS_ENABLE_DISABLE, TRUE, TRUE);
}

void SetCameraParameters(IExperiment* exp, ICamera* camera)
{
	ICamera::CameraType cameraType;
	ICamera::LSMType lsmType;
	double val;
	camera->GetParam(ICamera::PARAM_CAMERA_TYPE, val);
	cameraType = (ICamera::CameraType)static_cast<long> (val);
	camera->GetParam(ICamera::PARAM_LSM_TYPE, val);
	lsmType = (ICamera::LSMType)static_cast<long> (val);
	if (ICamera::CCD == cameraType)
	{
		string camName;
		long camImageWidth;
		long camImageHeight;
		double camPixelSize;
		double camExposureTimeMS;
		long gain, blackLevel, lightMode;
		long left, top, right, bottom;
		long binningX, binningY;
		long tapsIndex, tapsBalance;
		long readoutSpeedIndex;
		long camAverageMode, camAverageNum;
		long camVericalFlip, camHorizontalFlip, imageAngle, camChannel;
		long colorImageType, polarImageType;
		long isContinuousWhiteBalance, continuousWhiteBalanceNumFrames;
		double redGain, greenGain, blueGain;

		//getting the values from the experiment setup XML files
		exp->GetCamera(camName, camImageWidth, camImageHeight, camPixelSize, camExposureTimeMS, gain, blackLevel, lightMode, left, top, right, bottom, binningX, binningY, tapsIndex, tapsBalance, readoutSpeedIndex, camAverageMode, camAverageNum, camVericalFlip, camHorizontalFlip, imageAngle, camChannel, colorImageType, polarImageType, isContinuousWhiteBalance, continuousWhiteBalanceNumFrames, redGain, greenGain, blueGain);

		camAverageMode = (1 == camAverageMode) ? camAverageNum : 1;
		camera->SetParam(ICamera::PARAM_EXPOSURE_TIME_MS, camExposureTimeMS);
		camera->SetParam(ICamera::PARAM_GAIN, gain);
		camera->SetParam(ICamera::PARAM_OPTICAL_BLACK_LEVEL, blackLevel);
		camera->SetParam(ICamera::PARAM_LIGHT_MODE, lightMode);
		camera->SetParam(ICamera::PARAM_CAPTURE_REGION_LEFT, left);
		camera->SetParam(ICamera::PARAM_CAPTURE_REGION_TOP, top);
		camera->SetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT, right);
		camera->SetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM, bottom);
		camera->SetParam(ICamera::PARAM_BINNING_X, binningX);
		camera->SetParam(ICamera::PARAM_BINNING_Y, binningY);
		camera->SetParam(ICamera::PARAM_TAP_INDEX, tapsIndex);
		camera->SetParam(ICamera::PARAM_TAP_BALANCE_MODE, tapsBalance);
		camera->SetParam(ICamera::PARAM_READOUT_SPEED_INDEX, readoutSpeedIndex);
		camera->SetParam(ICamera::PARAM_CAMERA_AVERAGENUM, camAverageNum);
		camera->SetParam(ICamera::PARAM_CAMERA_IMAGE_VERTICAL_FLIP, camVericalFlip);
		camera->SetParam(ICamera::PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP, camHorizontalFlip);
		camera->SetParam(ICamera::PARAM_CAMERA_IMAGE_ANGLE, imageAngle);
		camera->SetParam(ICamera::PARAM_CAMERA_DMA_BUFFER_COUNT, camAverageNum);
		//will average after acquisition is complete. Set to none at camera level
		camera->SetParam(ICamera::PARAM_CAMERA_AVERAGEMODE, ICamera::AVG_MODE_NONE);

		camera->SetParam(ICamera::PARAM_CAMERA_COLOR_IMAGE_TYPE, colorImageType);
		camera->SetParam(ICamera::PARAM_CAMERA_POLAR_IMAGE_TYPE, polarImageType);
		camera->SetParam(ICamera::PARAM_CAMERA_IS_CONTINUOUS_WHITE_BALANCE_ENABLED, isContinuousWhiteBalance);
		camera->SetParam(ICamera::PARAM_CAMERA_CONTINUOUS_WHITE_BALANCE_NUM_FRAMES, continuousWhiteBalanceNumFrames);
		camera->SetParam(ICamera::PARAM_CAMERA_RED_GAIN, redGain);
		camera->SetParam(ICamera::PARAM_CAMERA_GREEN_GAIN, greenGain);
		camera->SetParam(ICamera::PARAM_CAMERA_BLUE_GAIN, blueGain);
	}
	else
	{
		wstring pathAndName = exp->GetPathAndName();
		camera->SetParamString(ICamera::PARAM_MESO_EXP_PATH, (wchar_t*)pathAndName.c_str());
		camera->SetParamString(ICamera::PARAM_WAVEFORM_OUTPATH, (wchar_t*)GetDir(pathAndName).c_str());

		long areaMode, scanMode, interleave, pixelX, pixelY, chan, lsmFieldSize, offsetX, offsetY, averageMode, averageNum, clockSource, inputRange1, inputRange2, twoWayAlignment, extClockRate, flybackCycles, inputRange3, inputRange4, minimizeFlybackCycles, polarity[4], verticalFlip, horizontalFlip;
		double areaAngle, dwellTime, crsFrequencyHz = 0;
		long timeBasedLineScan = FALSE;
		long timeBasedLineScanMS = 0;
		long threePhotonEnable = FALSE;
		long numberOfPlanes = 1;
		long selectedImagingGG = 0;
		long selectedStimGG = 0;
		double pixelAspectRatioYScale = 1;

		//getting the values from the experiment setup XML files
		exp->GetLSM(areaMode, areaAngle, scanMode, interleave, pixelX, pixelY, chan, lsmFieldSize, offsetX, offsetY, averageMode, averageNum, clockSource, inputRange1, inputRange2, twoWayAlignment, extClockRate, dwellTime,
			flybackCycles, inputRange3, inputRange4, minimizeFlybackCycles, polarity[0], polarity[1], polarity[2], polarity[3], verticalFlip, horizontalFlip, crsFrequencyHz, timeBasedLineScan, timeBasedLineScanMS, threePhotonEnable, numberOfPlanes, selectedImagingGG, selectedStimGG, pixelAspectRatioYScale);
		switch (areaMode)
		{
		case 0: //Square
		{
			areaMode = ICamera::SQUARE;
			camera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE, TRUE);
		}
		break;
		case 1: //Rect
		{
			areaMode = ICamera::RECTANGLE;
			camera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE, TRUE);
		}
		break;
		case 2: //Kymopgragh
		{
			areaMode = ICamera::RECTANGLE;
			camera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE, FALSE);
		}
		break;
		case 3: //LineScan
		{
			areaMode = ICamera::RECTANGLE;
			camera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE, FALSE);
		}
		break;
		case 4: //Polyline
		{
			areaMode = ICamera::POLYLINE;
			camera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE, FALSE);
		}
		break;
		default:
		{
			areaMode = ICamera::SQUARE;
			camera->SetParam(ICamera::PARAM_LSM_GALVO_ENABLE, TRUE);
		}
		break;
		}

		averageNum = (1 == averageMode) ? averageNum : 1;
		camera->SetParam(ICamera::PARAM_LSM_AREAMODE, areaMode);
		camera->SetParam(ICamera::PARAM_LSM_SCANAREA_ANGLE, areaAngle);
		camera->SetParam(ICamera::PARAM_LSM_PIXEL_X, pixelX);
		camera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, pixelY);
		camera->SetParam(ICamera::PARAM_LSM_DWELL_TIME, dwellTime);
		camera->SetParam(ICamera::PARAM_LSM_SCANMODE, scanMode);
		camera->SetParam(ICamera::PARAM_LSM_INTERLEAVE_SCAN, interleave);
		camera->SetParam(ICamera::PARAM_LSM_CHANNEL, chan);
		camera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, lsmFieldSize);
		camera->SetParam(ICamera::PARAM_LSM_OFFSET_X, offsetX);
		camera->SetParam(ICamera::PARAM_LSM_OFFSET_Y, offsetY);
		camera->SetParam(ICamera::PARAM_LSM_AVERAGEMODE, averageMode);
		camera->SetParam(ICamera::PARAM_LSM_AVERAGENUM, averageNum);
		camera->SetParam(ICamera::PARAM_LSM_CLOCKSOURCE, clockSource);
		camera->SetParam(ICamera::PARAM_LSM_INPUTRANGE1, inputRange1);
		camera->SetParam(ICamera::PARAM_LSM_INPUTRANGE2, inputRange2);
		camera->SetParam(ICamera::PARAM_LSM_INPUTRANGE3, inputRange3);
		camera->SetParam(ICamera::PARAM_LSM_INPUTRANGE4, inputRange4);
		camera->SetParam(ICamera::PARAM_LSM_MINIMIZE_FLYBACK_CYCLES, minimizeFlybackCycles);
		camera->SetParam(ICamera::PARAM_LSM_ALIGNMENT, twoWayAlignment);
		camera->SetParam(ICamera::PARAM_LSM_EXTERNALCLOCKRATE, extClockRate);
		camera->SetParam(ICamera::PARAM_LSM_DMA_BUFFER_COUNT, averageNum);
		camera->SetParam(ICamera::PARAM_LSM_FLYBACK_CYCLE, flybackCycles);
		//will average after acquisition is complete. Set to none at camera level
		camera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_SINGLE_FRAME);
		camera->SetParam(ICamera::PARAM_LSM_POCKELS_OUTPUT_USE_REF, FALSE);

		camera->SetParam(ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION, verticalFlip);
		camera->SetParam(ICamera::PARAM_LSM_HORIZONTAL_FLIP, horizontalFlip);
		camera->SetParam(ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN, timeBasedLineScan);
		camera->SetParam(ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS, timeBasedLineScanMS);

		camera->SetParam(ICamera::PARAM_LSM_3P_ENABLE, threePhotonEnable);
		camera->SetParam(ICamera::PARAM_LSM_NUMBER_OF_PLANES, numberOfPlanes);
		camera->SetParam(ICamera::PARAM_LSM_Y_AMPLITUDE_SCALER, (int)(pixelAspectRatioYScale * 100));


		//notify the ECU of the zoom change also
		IDevice* pControlUnitDevice = NULL;
		pControlUnitDevice = GetDevice(SelectedHardware::SELECTED_CONTROLUNIT);
		if (NULL != pControlUnitDevice && ICamera::LSMType::RESONANCE_GALVO_GALVO != lsmType)
		{
			if (SetDeviceParameterValue(pControlUnitDevice, IDevice::PARAM_SCANNER_ZOOM_POS, lsmFieldSize, FALSE, NULL, 0))
			{
				//If the device is an ECU2, read the alignment value from the ECU and set it back to itself, same way as capture setup LSMFieldSize->Setter
				long minFS, maxFS, fieldSizeDefault, alignment, zone, zoneECU;
				GetDeviceParamRangeLong(SelectedHardware::SELECTED_CONTROLUNIT, IDevice::PARAM_SCANNER_ZOOM_POS, minFS, maxFS, fieldSizeDefault);
				zone = maxFS - lsmFieldSize;
				zoneECU = IDevice::PARAM_ECU_TWO_WAY_ZONE_1 + zone;
				if (GetDeviceParamLong(SelectedHardware::SELECTED_CONTROLUNIT, zoneECU, alignment))
				{
					SetDeviceParamDouble(SelectedHardware::SELECTED_CONTROLUNIT, zoneECU, alignment, FALSE);
				}
			}
		}

		SetPMTProc(exp);
		ScannerEnableProc(0, TRUE);
	}
}

long GetTotalNumOfTiles()
{
	return RunSample::getInstance()->_totalNumOfTiles;
}
