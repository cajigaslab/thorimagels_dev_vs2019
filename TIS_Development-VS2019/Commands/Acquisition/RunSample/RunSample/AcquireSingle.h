#pragma once

#include "..\..\..\..\Common\Sample\Sample\Acquire.h"
#include "AcquireSaveInfo.h"

class AcquireSingle : public IAcquire
{
public:
	
	struct SaveParams
	{
		long doOME;
		long doCompression;
		long doJPEG;
	};

	AcquireSingle(IExperiment *pExperiment,wstring path);

	virtual long Execute(long index, long subWell);//Synchrnous acquisition of data
	virtual	long Execute(long index, long subWell, long zFrame, long tFrame);

	virtual long CallCaptureComplete(long captureComplete);
	virtual long CallCaptureImage(long index);
	virtual long CallSaveImage(long index, BOOL isImageUpdate);
	virtual long CallCaptureSubImage(long index);
	virtual long CallSaveSubImage(long index);
	virtual long PreCaptureEventCheck(long &status);
	virtual long StopCaptureEventCheck(long &status);
	virtual long CallSaveZImage(long index, double power0, double power1, double power2, double power3,double power4, double power5);
	virtual long CallSaveTImage(long index);
	virtual long CallSequenceStepCurrent(long index);
	virtual long CallStartProgressBar(long index, long resetTotalCount = 0);
	virtual long CallInformMessage(wchar_t* message);
	virtual long CallNotifySavedFileIPC(wchar_t* message);
	virtual long CallAutoFocusStatus(long isRunning, long bestScore, double bestZPos, double nextZPos, long currRepeat);

	virtual long ZStreamExecute(long index, long subWell, ICamera* pCamera, long zstageSteps, long timePoints, long undefinedVar);

	static HANDLE hEvent;
	static HANDLE hEventZ;

private:

	string uUIDSetup(auto_ptr<HardwareSetupXML> &pHardware, long timePoints, long zstageSteps, long index, long subWell);

	IExperiment * _pExp;

	static BOOL _evenOdd;
	static double _lastGoodFocusPosition;
	double _adaptiveOffset;
	long _repeat;
	long _counter;
	long _zFrame;
	long _tFrame;
	wstring _path;
	SaveParams _sp;

	AcquireSaveInfo* _acquireSaveInfo;
		
};
