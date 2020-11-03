#pragma once
#include "IImageRoutine.h"

class ImageRoutineSciCam:IImageRoutine
{
public:
	ImageRoutineSciCam();
	~ImageRoutineSciCam();

	long InitCallbacks(imageCompleteCallback, completeCallback);
	long InitParameters();
	long Snapshot(SnapshotSaveParams *);
	long EnableCopyToExternalBuffer();
	long CaptureZStack(double zStartPos, double zStopPos, double zstageStepSize, long zstageSteps);
	long StopZStackCapture();
	long SetDisplayChannels(long);
	long GetDisplayChannels();
	long SetupCaptureBuffers();
	long StartLiveCapture();
	long StopLiveCapture();
	long SetCaptureActive(long active);
	long GetCaptureActive();
	long CopyAcquisition(long isFullFrame = 1);
	long GetImageDimensions(long &width, long &height);
	long CaptureSingleImageWithAverage(char *buffer, double exposureTime, long binningX, long binningY , long avgFrames, long snapshotFlag);

private:	
	BOOL _enableCopy;
	long _channelEnable;
	long _captureActive;
};

UINT ZStackCaptureThreadProcSciCam( LPVOID pParam );
UINT SnapshotThreadProcSciCam(LPVOID pParam);