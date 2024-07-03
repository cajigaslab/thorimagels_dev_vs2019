#pragma once

#include "..\..\..\..\Common\Sample\Sample\Acquire.h"
#include "AcquireSaveInfo.h"
#include "..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\Common\BinaryImageDataUtilities\RawFile.h"

#define MAX_BLEACH_PARAMS_CNT 1
#define TIMEOUT_MS 5000

class AcquireBleaching : public IAcquire
{
public:
	AcquireBleaching(IExperiment *,wstring path);

	struct SaveParams
	{
		long index;
		long subWell;
		long width;
		long height;

		long red[3];
		long green[3];
		long blue[3];
		long bp[3];
		long wp[3];

		long colorChannels;
		long bufferChannels;
		long lsmChannels;			///bit value of all channels
		long imageID;
		double umPerPixel;
		long totFrames;
		long displayImage;
		double previewRate;
		bool isCombinedChannels; // if true, save all channels to a single TIFF
	};

	struct CaptureParams
	{
		AcquireBleaching* acqB;
		ICamera* pCam;
		SaveParams sp;
		Dimensions d;
		long currentT;
		long streaming;
		double timeInterval;
		long simultaneous;
	};

	virtual long Execute(long index, long subWell);
	long PreBleachingCapture(bool &,long width, long height);

	long PreCaptureAutoFocus(long index, long subWell, bool &ret);
	void PreBleachProtocol(IExperiment  *exp, ICamera * pBleachScanner);
	void PostBleachProtocol(IExperiment  *exp, ICamera * pCamera);
	void PostflightSLMBleachScanner(ICamera * pBleachScanner);
	static long LoadBleachWaveform(const wchar_t* bleachH5PathName, int CycleNum, int id);

	//Synchrnous acquisition of data
	virtual long Execute(long index, long subWell, long zFrame, long tFrame);//

	virtual long CallCaptureComplete(long captureComplete);
	virtual long CallCaptureImage(long index);
	virtual long CallSaveImage(long index, BOOL isImageUpdate);
	virtual long CallCaptureSubImage(long index);
	virtual long CallSaveSubImage(long index);
	virtual long PreCaptureEventCheck(long &status);
	virtual long StopCaptureEventCheck(long &status);
	virtual long CallSaveZImage(long index, double power0, double power1, double power2, double power3, double power4, double power5);
	virtual long CallSaveTImage(long index);	
	virtual long CallSequenceStepCurrent(long index);
	virtual long CallStartProgressBar(long index, long resetTotalCount = 0);
	virtual long CallInformMessage(wchar_t* message);
	virtual long CallNotifySavedFileIPC(wchar_t* message);
	virtual long CallAutoFocusStatus(long isRunning, long bestScore, double bestZPos, double nextZPos, long currRepeat);

	virtual long ZStreamExecute(long index, long subWell, ICamera* pCamera, long zstageSteps, long timePoints, long undefinedVar);

	//wrappers:
	static long CaptureWrap(void* pObj, ICamera * pCamera, long currentT, long streaming, long numFrames, double timeInterval, Dimensions d, SaveParams *sp, long simultaneous)
	{ AcquireBleaching* acqBleach = (AcquireBleaching*)pObj; return acqBleach->Capture(pCamera, currentT, streaming, numFrames, timeInterval, d, sp, simultaneous); }

	//static members:
	static HANDLE hEvent;
	static HANDLE hEventZ;
	static HANDLE hEventBleach;
	static HANDLE hStopBleach;
	static HANDLE hEventPostBleachSetupComplete;
	static HANDLE hEventStopLoad;
	static HANDLE hSLMLoadThread;
	static CRITICAL_SECTION loadAccess;
	static HANDLE hEventCapture;
	static HANDLE hStopCapture;

	//static LONGLONG _startTime;
	static GGalvoWaveformParams* bleachParams[MAX_BLEACH_PARAMS_CNT];
	static char * bleachMem;
	static streampos bleachMemSize;
	static BOOL captureStatus;
	
private:

	long SetPMT();	
	long ScannerEnable(long cameraOrBleachScanner, long enable);
	string uUIDSetup(SaveParams *sp, long bufferChannels, long timePoints, long zstageSteps, long index, long subWell);
	//void SaveData(SaveParams *sp, long currentT);
	IExperiment * _pExp;
	long SaveTimingToExperimentFolder();
	void SavePreviewImage(SaveParams *sp, long tFrame, char * pBuffer);

	long SetupCameraParams(ICamera *pCamera,long &bufferChannels, long &width, long &height,double &umPerPixel, long &displayImage, long triggerMode);	
	long PreCaptureAutoFocus(long index, long subWell);
	long PreCaptureProtocol(ICamera * pCamera, long index, long subWell, long streaming, long numFrames, long triggerEnable, long lsmChannel, Dimensions *d, SaveParams *sp);
	void PostCaptureProtocol(long streaming, long currentT, long numFrames, SaveParams *sp);
	long Capture(ICamera * pCamera, long currentT, long streaming, long numFrames, double timeInterval, Dimensions d, SaveParams *sp, long simultaneous);
	long Bleach();
	long CaptureStream(ICamera *pCamera, char * pMemoryBuffer, long currentT, long streamFrames, long avgFrames, SaveParams *sp);
	long CaptureTSeries(ICamera *pCamera, long currentT, long tFrames, long avgFrames, SaveParams *sp, double timeInterval, long simultaneous);
	void ClearBleachParams(int id){if(bleachParams[id] != NULL){if(bleachParams[id]->analogPockelSize > 0){free(bleachParams[id]->GalvoWaveformPockel);}if(bleachParams[id]->analogXYSize > 0){free(bleachParams[id]->GalvoWaveformXY);}if(bleachParams[id]->digitalSize > 0){free(bleachParams[id]->DigBufWaveform);}}}
	long CompareROIFile(wchar_t* path);

	void AverageStream(long numFrames, char *pMemoryBuffer, long avgFrames, long imageID, long size, unsigned long *pSumMemoryBuffer);
	void SaveStream(long currentT, long numFrames, SaveParams *sp);
	long ConvertRawFiles(SaveParams *sp, long preBleachingFrames, long postBleachingFrames1, long postBleachingFrames2, long skipMessage);
	long CatRawFiles(SaveParams *sp, long preBleachingFrames, long postBleachingFrames1, long postBleachingFrames2);
	long TransferRawFileByFrame(SaveParams *sp, ofstream& os, ifstream& is, int frameNum);
	long SaveRawFiles(SaveParams *sp, wchar_t *name, long size, long groupFrames, long startIndex,long totalFrames);
	void SaveTIFFChannels(SaveParams *sp, long size, char *buffer, long j, string &strOme, string &timeStamp, double dt);
	void SaveTIFFChannels(SaveParams *sp, long size, char *buffer, long j, string &strOme, string &timeStamp, double dt, long doOME, long doCompression);
	long Bleach(ICamera * pScanner,long width, long height, long bleachFrames);

	long GetConfigurableOption(string settingFile, string element, string attr, long& value);

	static BOOL _evenOdd;
	static double _lastGoodFocusPosition;
	double _adaptiveOffset;
	long _repeat;
	long _counter;
	long _tFrame;	
	long _stopStatus;
	double _zstageStepSize;
	//DWORD _expStartTime;
	wstring _path;
	DWORD _lastImageUpdateTime;
	wchar_t _rawBaseName[_MAX_PATH];
	string _wavelengthName[4];
	AcquireSaveInfo* _acquireSaveInfo;
	long _capturedImageID;
	static char* _pTemp;
	long _digiShutterEnabled;
};
