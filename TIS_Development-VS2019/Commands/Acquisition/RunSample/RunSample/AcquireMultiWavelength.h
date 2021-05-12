#pragma once

#include "..\..\..\..\Common\Sample\Sample\Acquire.h"
#include "AcquireSingle.h"

class AcquireMultiWavelength : public IAcquire
{
public:
	AcquireMultiWavelength(IExperiment *pExperiment,wstring path);

	virtual long Execute(long index, long subWell);//Synchrnous acquisition of data
	virtual	long Execute(long index, long subWell, long zFrame, long tFrame);

	virtual long CallCaptureComplete(long captureComplete);
	virtual long CallCaptureImage(long index);
	virtual long CallSaveImage(long index, BOOL isImageUpdate);
	virtual long CallCaptureSubImage(long index);
	virtual long CallSaveSubImage(long index);
	virtual long PreCaptureEventCheck(long &status);
	virtual long StopCaptureEventCheck(long &status);
	virtual long CallStartProgressBar(long index, long resetTotalCount = 0);

	virtual long CallSaveZImage(long index, double power0, double power1, double power2, double power3, double power4, double power5);
	virtual long CallSaveTImage(long index);
	virtual long CallSequenceStepCurrent(long index);

	static HANDLE hEvent;
	static HANDLE hEventFilter[3];
	static HANDLE hEventShutter;
	static HANDLE hEventZ;



private:

	void PositionFilters(IDevice *pExcitation, double ex, IDevice *pEmission, double em, IDevice *pDichroic, double dic);
	void OpenShutter(IDevice *pShutter);
	void CloseShutter(IDevice *pShutter);

	IExperiment * _pExp;

	static double _lastGoodFocusPosition;
	static BOOL _evenOdd;
	double _adaptiveOffset;
	long _zFrame;
	long _tFrame;
	double _zstageStepSize;
	wstring _path;
};
