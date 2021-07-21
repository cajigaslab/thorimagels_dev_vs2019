#pragma once

#include "..\..\..\..\Common\Sample\Sample\Acquire.h"
#include "AcquireSaveInfo.h"


class AcquireTSeries : public IAcquire
{
public:
	AcquireTSeries(IExperiment *,wstring path);

	virtual long Execute(long index, long subWell);//Synchrnous acquisition of data
	virtual long Execute(long index, long subWell, long zFrame, long tFrame);//

	virtual long ZStreamExecute(long index, long subWell);
	virtual long ZStreamExecute(long index, long subWell, ICamera *pCamera, long zstageSteps, long timePoints, long triggerMode);

	virtual long CallCaptureComplete(long captureComplete);
	virtual long CallCaptureImage(long index);
	virtual long CallSaveImage(long index, BOOL isImageUpdate);
	virtual long CallCaptureSubImage(long index);
	virtual long CallSaveSubImage(long index);
	virtual long PreCaptureEventCheck(long &status);
	virtual long StopCaptureEventCheck(long &status);
	virtual long CallSaveZImage(long index, double power0, double power1, double power2, double power3, double power4, double power5);
	virtual long CallSaveTImage(long index);
	virtual long CallSequenceStepCurrent(long index);
	virtual long CallStartProgressBar(long index, long resetTotalCount = 0);
	virtual long CallInformMessage(wchar_t* message);

	static HANDLE hEvent;
	static HANDLE hEventZ;
	static long _stopTCapture; //indicates if stop requested by user since start of experiment


private:
	struct SaveParams
	{
		wstring path;
		string wavelengthName[4];
		long index;
		long subWell;
		long width;
		long height;

		long red[3];
		long green[3];
		long blue[3];
		long bp[3];
		long wp[3];

		long colorChannels;
		long imageID;
		double umPerPixel;
		long totFrames;
	};

	long SetZPosition(double pos,BOOL bWait, BOOL bPostflight);
	long SetZPositionLocked(double pos1, double pos2, BOOL bWait, BOOL bPostflight);
	long StopZ();
	long CaptureTSeries(long index, long subWell, auto_ptr<IAcquire> &acqZFrame, ICamera *pCamera);
	string uUIDSetup(auto_ptr<HardwareSetupXML> &pHardware, long timePoints, long zstageSteps, long zStreamFrames, long index, long subWell);
	long SetPMT();
	long ScannerEnable(long enable);
	IExperiment * _pExp;
	ICamera * _pCamera;

	static BOOL _evenOdd;
	static double _lastGoodFocusPosition;
	double _adaptiveOffset;
	long _repeat;
	long _counter;
	long _tFrame;	// time point
	long _zsFrame;	// stream index for certain z location
	long _zFrame;	// z location
	long _tzsFrame;	// time point for each single frame
	long _lsmChannel;
	wstring _path;

	AcquireSaveInfo* _acquireSaveInfo;
};
