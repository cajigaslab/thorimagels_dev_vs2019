#pragma once
#include "Types.h"
#include "CircleBuffer.h"
#include "CameraConfig.h"
#include <mutex>
#include "Logger.h"

typedef std::function<void(StripInfo*)> StripInfoChangedCallBack;

class WaveformManagerBase
{
public:
	WaveformManagerBase();
	~WaveformManagerBase();
	long CheckParams(Scan* scan, CameraConfig* cameraConfig);
	long SetParams(Scan* scan, CameraConfig* cameraConfig, DMABufferInfo* dmaInfo);
	long GetStripCount() { return _stripCount; };
	virtual long SetParams(WaveformParams* waveformParams, long bufferSize) = 0;
	virtual long ResetScanParams(Scan* scan,bool onlyResetUnusedMask) = 0;
	virtual long ResetAllParams() = 0;
	virtual long SetCurrentPosition(double x1Pos, double x2Pos, double yPos, double vPos) = 0;
	virtual long GetTotalScanTime(Scan scans[], uint8_t scanSize, CameraConfig* cameraConfig) = 0;
	CircleBuffer* GetWaveformBuffer(uint8_t index);
	long StartGenerate();
	long StopGenerate();
	long SetDefaultParams(CameraConfig * cameraConfig);
	void InitCallBack(StripInfoChangedCallBack stripInfoChangedCallBack) { _stripInfoChangedCallBack = stripInfoChangedCallBack; };
	unsigned int GetMaxiStripSize();

	//***	additional functions	***//
	virtual double GetTotalScanTime() = 0;
	virtual long SetupCircularBuffer(long id = -1) = 0;
	virtual void TeardownCircularBuffers() = 0;
	virtual long DefaultWaveformParams(CameraConfig* camConfig) = 0;

	long IsStopping() { return (WAIT_OBJECT_0 == WaitForSingleObject(_stopHandle, 0)) ? TRUE : FALSE; };
	long IsRunning() { return (WAIT_OBJECT_0 == WaitForSingleObject(_runningHandle, 0)) ? TRUE : FALSE; };

protected:
	CircleBuffer* _circleBufferList[MAX_CIRCLEBUFFER];
	WaveformParams* _waveformParams;
	HANDLE _actualStopHandle;
	HANDLE _runningHandle;
	virtual long IsBufferReady(StripInfo* stripe) = 0;
	virtual	long GenerateStripWaveform(StripInfo* stripInfo) = 0;
	virtual	long GenerateStripSkipInfo(StripInfo* stripInfo, bool isNeedGeneratePowerMask = true) = 0;
	virtual long GenerateStripList(Scan* scanInCache, StripInfo ** first, StripInfo ** end, unsigned int TCount, DMABufferInfo* dmaInfo) = 0;
	long SetWaveformParams(Scan* scan, CameraConfig * cameraConfig);

private:
	StripInfoChangedCallBack _stripInfoChangedCallBack;
	HANDLE _stopHandle;
	HANDLE _waveformReadyHandle;
	StripInfo* _firstStrip;
	StripInfo* _currentStrip;
	StripInfo* _firstStripInLoop;
	mutex _mutex;
	vector<Scan*> _scansCache;
	long _stripCount;
	unsigned int _maxStipSize;

	long SetConfigParams(CameraConfig * cameraConfig);

	static void StartGenerateThread(LPVOID pParam);
	void GenerateAsync();

	//***	additional members	***//

	HANDLE _hThread;

	//***	additional functions	***//

	void ClearStripsAndScan();
};

