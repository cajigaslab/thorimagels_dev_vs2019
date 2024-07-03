#pragma once

#include "stdafx.h"

extern wchar_t message[_MAX_PATH];

// This class is exported from the ThorMesoScanSimulator.dll
class ThorMesoScanSimulator:public ICamera
{
private:
	vector<Scan*> _scansCache;
	Scan* _scan;
	CameraConfig* _cameraConfig;
	WaveformParams* _waveformParams;
	StripInfo* _firstStrip;
	StripInfo* _currentStrip;
	StripInfo* _firstStripInLoop;
	std::mutex _mtx;
	long _stripCount;
	unsigned int _stripWidth;
	ImageCompleteCallback _imageCompleteCallback;
	static long StartThreadFunc(LPVOID pParam);
	long StartAsync();
	HANDLE _runningHandle;
	HANDLE _actualStopHandle;
	HANDLE _stopHandle;
	long ResolveScanInfo();
	static std::once_flag _onceFlag;
	static unique_ptr<ThorMesoScanSimulator> _single;
	void PrintInformation(StripInfo* stripe, uint16_t* buffer, int cId);
	vector<SimulatorImage*> _simulatorImages;
	bool GetSimulatorImages();

	//***	additional members	***//

	const int MAX_PIXEL;
	const int MIN_PIXEL;
	const int DEFAULT_PIXEL;

	static CRITICAL_SECTION _accessSection; ///<critical section control for setup and start acquisition
	HANDLE _hThread; ///<async thread to generate buffer
	std::unique_ptr<LoadMeso> _expLoader;///<experiment loader to create scan info from file
	std::mutex _dmaMutex;///<Mutex to claim the exclusive access to the buffer
	int _acquireStatus;///<status to reflect current camera acquisition
	DMABufferInfo _bufferInfo;///<info of each dma buffer, used to determine recreation of dma
	unsigned short* _pFrmDllBuffer[MAX_DMABUFNUM];///<dma buffers in scan area index order
	map<long, long long> _bufferOrder;///<map of dma buffer index and completed frame index
	long long _indexOfLastCompletedFrame;///<counter for tracking the sequence index of the current frame
	long long _indexOfLastCopiedFrame;///<counter for tracking the index of the frames being copied out to the user
	long _bufCompleteID; ///<DMA index of current buffer in acquire
	long _pixelX; ///<pixel size X
	long _pixelY; ///<pixel size Y
	long _scanMode; ///<scan mode
	long _triggerMode; ///<trigger mode
	long _isLivingMode; ///<live imaging
	vector<Channel> _channels; ///<channels' info

public:
	ThorMesoScanSimulator(void);
	~ThorMesoScanSimulator();
	static ThorMesoScanSimulator* getInstance();
	static void LogMessage(wchar_t *message,long eventLevel);

	long FindCameras(long &cameraCount);
	long SelectCamera(const long camera);
	long TeardownCamera();
	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double &param);
	long PreflightAcquisition(char * pDataBuffer);
	long SetupAcquisition(char * pDataBuffer);
	long StartAcquisition(char * pDataBuffer);
	long StatusAcquisition(long &status);
	long StatusAcquisitionEx(long &status, long &indexOfLastCompletedFrame);
	long CopyAcquisition(char * pDataBuffer, void* frameInfo);
	long PostflightAcquisition(char * pDataBuffer);
	long GetLastErrorMsg(wchar_t * errMsg, long size);
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);
	long InitCallBack(ImageCompleteCallback imageCompleteCallback);
	long GetMapParam(const long paramID, double inputValue, double &param);
	long SetMapParam(const long paramID, double inputValue, double param);
	long GetDeviceConfigration(const long paramID, void** param);
	long SaveConfigration();
	long GetCaptureTime(Scan scans[], const uint8_t scanSize, long& timeMillisecond);

	//fill partial DMA frame buffer when callback
	void ImageComplete(ImageCompleteStates state, ChanBufferInfo info, FrameROI roi, unsigned short* buffer, unsigned int size);
	void ClearStripeList();
	void ClearScan();
	void StopScan();
};