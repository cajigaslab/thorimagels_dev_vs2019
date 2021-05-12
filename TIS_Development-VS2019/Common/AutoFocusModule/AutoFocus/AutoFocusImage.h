#pragma once

#include "AutoFocusClass.h"

class AutoFocusImage : public IAutoFocus
{
public:
	AutoFocusImage();

	long Execute(long index, IDevice *pAutoFocus,BOOL &afFound) override;//Synchrnous execution of autofocus

	static HANDLE hEventAutoFocus;
	static HANDLE hEventCamera;
	static HANDLE hEventZ;

	void SetupParameters(ICamera * pCamera, IDevice *pZStage,long repeat, double focusOffset, double expTime, double stepSizeUM,double startPosMM, double stopPosMM,long binning, double finePercentage, long enableGUIUpdate);
	long SetZPosition(double pos);
	long CaptureSingleImage( char *buffer,double expTime);
	void ContrastScore(char * pBuffer, long width, long height, long skipSize, long &score);
	long GetImageBuffer(char* pBuffer, long& frameNumber, long& currentRepeat, long& status, long& zSteps, long& currentZIndex);
	void SetStopFlag(long stopValue);
	void GetStatus(long& currentStatus, long& bestContrastScore, double& bestZPosition, double& nextZPosition, long& currentRepeatIndex);
	
	long WillAFExecuteNextIteration();

	enum AutoFocusStatusTypes
	{
		NOT_RUNNING,
		STOPPED,
		COARSE_AUTOFOCUS,
		FINE_AUTOFOCUS,
		HARDWARE_AUTOFOCUS
	};

private:

	ICamera *_pCamera;
	IDevice *_pZStage;

	double _expTimeMS;
	double _stepSizeUM;
	double _startPosMM;
	double _stopPosMM;
	long _repeat;
	long _counter;
	double _focusOffset;

	long _binning;
	long _width;
	long _height;
	long _imageReady;
	char* _imageBuffer;

	long _frameNumber;
	long _zSteps;
	long _currentZIndex;
	long _autoFocusStatus;
	long _bestContrastScore;
	long _enableGUIUpdate;
	double _bestZPositionFound;
	double _nextZPosition;
	double _finePercentageDecrease;

	static atomic_long _stopFlag;
};
