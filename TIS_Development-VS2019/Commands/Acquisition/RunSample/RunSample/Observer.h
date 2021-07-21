#pragma once


class Observer : public has_slots<>
{
public:
	Observer();

	void OnCaptureComplete(long val);
	void OnCaptureImage(long val);
	void OnSaveImage(long val, BOOL isImageUpdate);
	void OnCaptureSubImage(long val);
	void OnSaveSubImage(long val);
	void OnStopCapture(long &val);
	void OnPreCapture(long &val);
	void OnSaveZImage(long val, double power0, double power1, double power2, double power3,double power4, double power5);
	void OnSaveTImage(long val);
	void OnSequenceStepCurrent(long val);
	void OnProgressBarStart(long val, long resetTotalCount = 0);
	void OnInformMessage(wchar_t* message);
	void SetStopCapture(long status);
	void SetTotalImagecount(long count, long channelCount);		///< count: total frame #, channelCount: total channel steps for CaptureSequence mode

private:

	DWORD startTime;				///< experiment start time
	long _captureComplete;
	long _totalImageCount;			///< total image count to be finished
	long _currentImageCount;		///< current image count, will be rewinded in CaptureSequence mode 
	long _elapsedImageCount;		///< elapsed image count, won't be rewinded, used to estimate remaining time
	long _stopCapture;
	long _totalSequenceStep;		///< total channel sequences to be finished in CaptureSequence mode
	long _currentSequenceStep;		///< current channel sequence count, 1-based index, will be updated before saving
	long _channelLastIdx;			///< previous channel sequence count, keep track of channel sequence count changes
	long _channelFrmCount;			///< local frame count in CaptureSequence mode, used to rewind current image count
	long _rt;						///< remaining time
	long _et;						///< elapsed time
	long _i;
};
