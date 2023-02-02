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
	long SetZStackActive(long zStackActive);
	long GetZStackActive();
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
	long StartAutoFocus(double magnification, long autoFocusType, BOOL& afFound);

	static BOOL _enableCopy;

private:	
	long _channelEnable;
	long _captureActive;
	long _zStackActive;
};

UINT ZStackCaptureThreadProcSciCam( LPVOID pParam );
UINT SnapshotThreadProcSciCam(LPVOID pParam);

UINT AutoFocusCaptureThreadProcSciCam(LPVOID pParam);
UINT AutoFocusStatusThreadProcSciCam();