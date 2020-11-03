#pragma once

#include "..\..\..\..\Common\Sample\Sample\Acquire.h"
#include "AutoFocus.h"

class AcquireZStack : public IAcquire
{
public:
	AcquireZStack(IAutoFocus *,IExperiment *,wstring path);

	virtual long Execute(long index, long subWell);//Synchrnous acquisition of data
	virtual long Execute(long index, long subWell, long zFrame, long tFrame);//

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

	static HANDLE hEvent;
	static HANDLE hEventAutoFocus;
	static HANDLE hEventZ;
	static long _stopZCapture; //indicates if stop requested by user since start of experiment


private:

	long SetAutoFocusStartZPosition(double afStartPos,BOOL bWait,BOOL afFound);
	long AutoFocusAndRetry(long index, IDevice *pAutoFocusDevice, BOOL &afFound);
	long SetZPosition(double pos,BOOL bWait);
	long StopZ();
	long SetPMT();
	long ScannerEnable(long enable);
	long CaptureZStack(long index, long subWell, auto_ptr<IAcquire> &acqZFrame);

	IAutoFocus * _pAutoFocus;
	IExperiment * _pExp;
	IDevice * _pPower;
	ICamera * _pCamera;

	static BOOL _evenOdd;
	static double _lastGoodFocusPosition;
	double _adaptiveOffset;
	long _repeat;
	long _counter;
	long _tFrame;
	wstring _path;
};
