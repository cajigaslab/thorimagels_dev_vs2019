#pragma once


#include "AutoExposureResults.h"
#include "AutoExposureParams.h"

typedef void(*UPDATE_CALLBACK)(bool, bool, double, double);

class AutoExposure
{

private:

	static std::mutex _initLock;

	static AutoExposure* _singleton;
	
	static AutoExposureResults GetAutoExposure(const std::vector<int> &histogram, const AutoExposureParams &autoExposureParams);

	std::atomic_bool _isStopFlag = false;
	std::atomic_bool _isRunning = false;
	bool _frameArrived;
	std::atomic_bool _isStable = false;
	char* _currentImageBuffer;
	FrameInfoStruct _currentFrameInfo = { 0, 0, 0, 0 };
	int _currentImageBufferSize;
	int _currentImageFrameNumber;
	std::vector<UPDATE_CALLBACK> _updateCallbacks;
	double _targetPercent = 0.8;

	Dimensions GetImageDimensions(ICamera *camera);

	bool CaptureSnapshot(ICamera* camera, char* buffer, FrameInfo& frameInfo);

	void FillHistogram(unsigned short *imageData, int bitDepth, const FrameInfo& frameInfo, vector<int>& histogram);

public:
	
	static AutoExposure* GetInstance();

	void Start(ICamera *camera);

	void Stop();

	bool IsStable();

	bool IsRunning();

	bool GetAutoExposureImage(char* imageBuffer, FrameInfoStruct& frameInfo, long imageBufferSize, long& frameNumber);
	
	void RegisterUpdateCallback(UPDATE_CALLBACK);

	void UnregisterUpdateCallback(UPDATE_CALLBACK);

	void SetTargetPercent(double);

	double GetTargetPercent();

};

