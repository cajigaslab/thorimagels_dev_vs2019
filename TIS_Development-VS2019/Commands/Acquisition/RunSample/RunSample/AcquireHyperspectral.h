#include "..\..\..\..\Common\Sample\Sample\Acquire.h"
#include "AcquireSaveInfo.h"
#include "..\..\..\Common\BinaryImageDataUtilities\RawFile.h"
class AcquireHyperspectral :
	public IAcquire
{
public:
	AcquireHyperspectral(IExperiment *pExperiment,wstring path);
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

	static HANDLE hEvent;
	static HANDLE hEventZ;
private:
	IExperiment* _pExp;
	long _zFrame;
	long _tFrame;
	wstring _path;
};