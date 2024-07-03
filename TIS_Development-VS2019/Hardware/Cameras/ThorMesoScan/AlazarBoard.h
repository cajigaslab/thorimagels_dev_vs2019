#pragma once
#include <vector>
#include <functional>
#include "Types.h"
#include <queue>
#include <mutex> 
#include "CameraConfig.h"
using namespace std;
#define ACQUIRE_BUFFER_COUNT	64
#define TRANSFER_BUFFER_COUNT	4

class StripRecordInfo
{
public:
	StripRecordInfo()
	{

	}
	static void DeleteStripLoop(StripRecordInfo* strip)
	{
		StripRecordInfo* current = strip;
		while (current != NULL)
		{
			StripRecordInfo* next = current->nextStrip;
			delete current;
			current = next;
		}
	}

	uint32_t SkipRecordCount;	//
	uint32_t DataRecordCount;  //
	ChanBufferInfo ChanBufInfo[CHANNEL_COUNT];
	FrameROI FrameROI;
	StripRecordInfo* nextStrip;
	bool IsAverageEnd;
	bool IsFrameEnd;
	bool IsEnd;
	ScanMode ScanMode;
private:
	~StripRecordInfo()
	{
	}
}; 

class AlazarBoard
{
public:
	static long CloseAlazarBoard(AlazarBoard* alazar);
	virtual long CheckParams(Scan* scan, CameraConfig* cameraConfig) = 0;
	virtual long SetParams(Scan* scan, CameraConfig* cameraConfig, uint32_t maxBufferSize) = 0;
	virtual long SetInputRange(U8 channelId, U32 Range) = 0;
	virtual long AppendStripInfo(StripInfo* stripInfoList);
	virtual long StartAcquisition();
	virtual long StopAcquisition();
	virtual void InitCallBack(ImageCompleteCallback imageCompleteCallback) { _imageCompleteCallback = imageCompleteCallback; };
	virtual long IsStopping() { return (WAIT_OBJECT_0 == WaitForSingleObject(_stopHandle, 0)) ? TRUE : FALSE; };
	virtual long IsRunning() { return (WAIT_OBJECT_0 == WaitForSingleObject(_runningHandle, 0)) ? TRUE : FALSE; };

protected:
	static long StartThreadFunc(LPVOID pParam);
	virtual long StartAsync() = 0;
	void clearStripInfo();
	HANDLE _hAlazarSys;
	HANDLE _actualStopHandle;
	HANDLE _runningHandle;
	HANDLE _stopHandle;
	ImageCompleteCallback _imageCompleteCallback;
	bool _isReadyToStart;
	uint16_t* _acquireBuffer[ACQUIRE_BUFFER_COUNT];
	uint16_t* _transferBuffer[TRANSFER_BUFFER_COUNT];
	StripRecordInfo* _firstStripInfo;
	StripRecordInfo* _currentStripInfo;
	StripRecordInfo* _endStripInfo;
	U32 _frameTimeOut;
	ATSParams* _atsParams;

	//***************** Additional Functions *****************//
	void Terminate();
};
class AlazarBoard9440 :public AlazarBoard
{
public:
	static long FindAlazarBoard();
	static AlazarBoard9440* SelectAlazarBoard(uint32_t sid);
	long CheckParams(Scan* scan, CameraConfig* cameraConfig);
	long SetParams(Scan* scan, CameraConfig* cameraConfig, uint32_t maxBufferSize);
	long SetInputRange(U8 channelId, U32 Range);
protected:

private:
	AlazarBoard9440(HANDLE hdl);
	~AlazarBoard9440();
	static bool IsCorrectFPGA(HANDLE hdl);
	long VerifyParams(Scan* scan, CameraConfig* cameraConfig, ATSParams** atsParams);
	long SetCrsFrequency(double crsFrequency);
	long InitAlazarBoard();
	long SetDataSkippingWithPixelAveraging(bool enable);
	long SetNumRawPoints(long numPoints);
	long SetDataSkip(U32 samplePerRecord, ATSParams* atsParams);
	long StartAsync();
	long ProcessBuffer(U16* pSrc, int chNum, int width,int height, U16** pDst,ScanMode scanMode, int ignorePixels);
	long GenerateDataMap(U16* datamap, ATSParams* atsParams);
	U32 _samplePerRecord;
	U16 _datamap[65536];
	map<double, U32> _inputRangeMap;
};
