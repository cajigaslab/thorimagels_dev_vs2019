#pragma once

#include "..\..\SigSlot.h"
#include "Publisher.h"

using namespace sigslot;

class IAcquire
{
public:
	signal1<long> CaptureImage;
	signal2<long, BOOL> SaveImage;
	signal1<long> CaptureSubImage;
	signal1<long> SaveSubImage;
	signal1<long&> StopCapture;
	signal7<long, double, double, double, double, double, double> SaveZImage;
	signal1<long> SaveTImage;
	signal1<long> CaptureComplete;
	signal1<long&> PreCapture;
	signal1<long> SequenceStepCurrent;
	signal2<long, long> StartProgressBar;
	signal1<wchar_t*> InformMessage;
	signal1<wchar_t*> NotifySavedFileIPC;
	signal5<long, long, double, double, long> AutoFocusStatus;

	virtual void SetPublisher(Publisher*) {};

	virtual long Execute(long index, long subWell) = 0;//Synchronous acquisition of data
	virtual long Execute(long index, long subWell, long zFrame, long tFrame)=0;//
	virtual long ZStreamExecute(long index, long subWell, ICamera* pCamera, long zstageSteps, long timePoints, long undefinedVar) = 0;

	virtual long CallCaptureImage(long index) = 0;
	virtual long CallSaveImage(long index, BOOL isImageUpdate) = 0;
	virtual long CallCaptureSubImage(long index) = 0;
	virtual long CallSaveSubImage(long index) = 0;
	virtual long PreCaptureEventCheck(long &status) = 0;
	virtual long StopCaptureEventCheck(long &status) = 0;
	virtual long CallSaveZImage(long index, double power0, double power1, double power2, double power3, double power4, double power5) = 0;
	virtual long CallSaveTImage(long index) = 0;
	virtual long CallCaptureComplete(long index) = 0;
	virtual long CallSequenceStepCurrent(long index) = 0;
	virtual long CallStartProgressBar(long index, long resetTotalCount = 0) = 0;
	virtual long CallInformMessage(wchar_t* message) = 0;
	virtual long CallNotifySavedFileIPC(wchar_t* message) = 0;
	virtual long CallAutoFocusStatus(long isRunning, long bestScore, double bestZPos, double nextZPos, long currRepeat) = 0;
};
