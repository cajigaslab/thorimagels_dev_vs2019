#include "stdafx.h"
#include "Observer.h"

extern void (*myFunctionPointerBeginImage)(long * index);
extern void (*myFunctionPointerBeginSubImage)(long * index);
extern void (*myFunctionPointerEndSubImage)(long * index);
extern void (*myFunctionPointerSaveZImage)(long* index, double* power0, double* power1, double* power2, double* power3, double *power4, double *power5);
extern void (*myFunctionPointerSaveTImage)(long * index);
//extern void (*myFunctionPointerCaptureComplete)(long * index);
extern void (*myFunctionPointer)(long * index,long * completed,long * total, long * timeElapsed, long * timeRemaining, long * captureComplete);
extern void (*myFunctionPointerPreCapture)(long * status);
extern void (*myFunctionPointerSequenceStepCurrent)(long* index);
extern void (*myFunctionPointerInformMessage)(wchar_t* message);
extern void (*myFunctionPointerFileSavedNameAndPathForIPC)(wchar_t* message);
extern void (*myFunctionPointerAutoFocusStatus)(long* isRunning, long* bestScore, double* bestZPos, double* nextZPos, long* currRepeat);

Observer::Observer()
{
	_totalImageCount=0;
	_elapsedImageCount=_currentImageCount=0;
	_stopCapture=0;
	_captureComplete = FALSE;
	_totalSequenceStep=0;
	_currentSequenceStep=0;
	_channelLastIdx=0;
	_rt = 0;
	_i = 0;
	_et = 0;
}

void Observer::OnCaptureComplete(long captureComplete)
{
	if (TRUE == captureComplete)
	{
		_captureComplete = TRUE;
		myFunctionPointer(&_i,&_totalImageCount, &_totalImageCount, &_et, &_rt, &_captureComplete);
	}
	else
	{
		_captureComplete = FALSE;
	}

}

void Observer::OnCaptureImage(long index)
{
	//		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"RunSample Observer OnCaptureImage");

	long i = index;
	myFunctionPointerBeginImage(&i);
}

void Observer::OnSaveImage(long index, BOOL isImageUpdate)
{
	//		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"RunSample Observer OnSaveImage");

	DWORD elapsedTime = 0;
	DWORD remainingTime = 0;
	long count = 0;

	long updateIndex = index;
	//if the index is -1 it means that the index is not available to the caller
	//in this case replace the index with the last recorded index
	if (-1 == index)
	{
		updateIndex = _currentImageCount;
	}

	/// to start timer: OnSaveImage(0, true)
	if((0 == _currentImageCount) && (TRUE == isImageUpdate))
	{
		startTime = GetTickCount();
		elapsedTime = 0;
		remainingTime = 0;
		_currentImageCount++;
		_channelLastIdx =  _currentSequenceStep;
		_channelFrmCount = _elapsedImageCount = _currentImageCount;
		count = 0;
	}		
	else
	{
		elapsedTime = GetTickCount() - startTime;
		if(isImageUpdate) 
		{
			//calculate time per image and multiply by remaining number of images
			remainingTime = (0 == _totalSequenceStep) ? (elapsedTime/(_currentImageCount)) * (_totalImageCount - _currentImageCount) :
				(elapsedTime/(_elapsedImageCount)) * ((_totalImageCount*_totalSequenceStep) - (_elapsedImageCount));

			if(0 == _totalSequenceStep)
			{
				// actual finish frames with all channels
				count = _currentImageCount;
				_currentImageCount++;
			}
			else
			{
				// consider Channel Sequence mode, handle transition of channels:
				if(_channelLastIdx != _currentSequenceStep)
				{						
					if (_totalSequenceStep == _channelLastIdx)
					{
						// reset index
						_channelFrmCount = _currentImageCount;
					}
					else
					{
						// rewind index
						_currentImageCount = _channelFrmCount;
					}
					_channelLastIdx = _currentSequenceStep;
				}
				count = _currentImageCount;
				_currentImageCount++;
				_elapsedImageCount++;
			}
		}
		else
		{
			// use the passed in parameter to predict remaining time
			if((updateIndex > 0) && (updateIndex <= _totalImageCount)) {
				remainingTime = (elapsedTime/(updateIndex)) * (_totalImageCount - updateIndex);
			}
			count = updateIndex;
		}
	}

	long rt = static_cast<long>(remainingTime);
	_rt = rt;
	long et = static_cast<long>(elapsedTime);
	_et = et;
	long i = updateIndex;
	_i = i;
	_captureComplete = FALSE;
	myFunctionPointer(&i,&count, &_totalImageCount, &et, &rt, &_captureComplete);
}

void Observer::OnCaptureSubImage(long index)
{
	//		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"RunSample Observer OnCapture SubImage");

	long i = index;
	myFunctionPointerBeginSubImage(&i);
}

void Observer::OnPreCapture(long &status)
{
	myFunctionPointerPreCapture(&status);
}

void Observer::OnSaveSubImage(long index)
{
	//		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"RunSample Observer OnSave SubImage");	

	long i = index;
	myFunctionPointerEndSubImage(&i);
}

void Observer::OnSaveZImage(long index, double power0, double power1, double power2, double power3, double power4, double power5)
{
	//		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"RunSample Observer OnSave Z Image");	

	long i = index;
	myFunctionPointerSaveZImage(&i, &power0, &power1, &power2, &power3, &power4, &power5);
}

void Observer::OnSaveTImage(long index)
{
	//		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"RunSample Observer OnSave T Image");	

	long i = index;
	myFunctionPointerSaveTImage(&i);
}

void Observer::OnStopCapture(long &status)
{
	status = _stopCapture;
}

void Observer::OnSequenceStepCurrent(long index)
{
	_currentSequenceStep = index + 1;	// 1-based for _currentSequenceStep
	myFunctionPointerSequenceStepCurrent(&index);
}

void Observer::SetStopCapture(long status)
{
	_stopCapture = status;
}

void Observer::OnProgressBarStart(long index, long resetTotalCount)
{
	//		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"RunSample Observer OnSave T Image");	

	if(0 == _currentImageCount && 1 == index)
	{
		OnSaveImage(0, TRUE);
	}
	if(0 < resetTotalCount)
	{
		_totalImageCount = resetTotalCount;
	}
}

void Observer::OnAutoFocusRunning(long isRunning, long bestScore, double bestZPos, double nextZPos, long currRepeat)
{
	myFunctionPointerAutoFocusStatus(&isRunning, &bestScore, &bestZPos, &nextZPos, &currRepeat);
}

void Observer::OnInformMessage(wchar_t* message)
{
	myFunctionPointerInformMessage(message);
}

void Observer::OnNotifySavedFileIPC(wchar_t* message)
{
	myFunctionPointerFileSavedNameAndPathForIPC(message);
}

void Observer::SetTotalImagecount(long count, long channelCount)
{	
	_totalImageCount = count;
	_totalSequenceStep = channelCount;
	_elapsedImageCount = _currentImageCount = 0;
	_currentSequenceStep = 0;
	_stopCapture=0;
}

