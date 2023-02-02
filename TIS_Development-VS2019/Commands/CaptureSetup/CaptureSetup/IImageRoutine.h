#pragma once
typedef void (_cdecl *completeCallback)();
typedef void (_cdecl *imageCompleteCallback)(char * buffer, FrameInfoStruct &imageInfo);
typedef void (_cdecl *preCapturetype)(long* status);

extern void (*myFunctionPointer)(char * buffer,FrameInfoStruct &imageInfo);
//mechanism to notify listeners that the z Stack capture is finished
extern void (*myFuncPtrZStack)();

struct SnapshotSaveParams
{
	wchar_t path[MAX_PATH];
	int channelBitMask;
	int saveMultiPage;
};

#define MAX_IMAGE_ROUTINES 3

class IImageRoutine
{
public:

	virtual long InitCallbacks(imageCompleteCallback, completeCallback)=0;
	virtual long InitParameters()=0;
	virtual long Snapshot(SnapshotSaveParams *)=0;
	virtual long EnableCopyToExternalBuffer()=0;
	virtual long CaptureZStack(double , double , double , long )=0;
	virtual long StopZStackCapture()=0;
	virtual long SetZStackActive(long)=0;
	virtual long GetZStackActive()=0;
	virtual long SetDisplayChannels(long)=0;
	virtual long GetDisplayChannels()=0;
	virtual long SetupCaptureBuffers()=0;
	virtual long StartLiveCapture()=0;
	virtual long StopLiveCapture()=0;
	virtual long SetCaptureActive(long)=0;
	virtual long GetCaptureActive()=0;
	virtual long CopyAcquisition(long)=0;
	virtual long GetImageDimensions(long&, long&)=0;
	virtual long StartAutoFocus(double, long, BOOL&) = 0;
};