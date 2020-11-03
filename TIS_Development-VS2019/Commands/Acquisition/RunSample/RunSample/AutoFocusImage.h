#pragma once

#include "AutoFocus.h"

class AutoFocusImage : public IAutoFocus
{
public:
	AutoFocusImage();

	virtual long Execute(long index, IDevice *pAutoFocus,BOOL &afFound);//Synchrnous execution of autofocus

	static HANDLE hEventAutoFocus;
	static HANDLE hEventCamera;
	static HANDLE hEventZ;

	void SetupParameters(ICamera * pCamera, IDevice *pZStage,long repeat, double focusOffset, double expTime, double stepSizeUM,double startPosMM, double stopPosMM,long binning);
	long SetZPosition(double pos);
	long CaptureSingleImage( char *buffer,double expTime);
	void ContrastScore(char * pBuffer, long width, long height, long skipSize, long &score);
	
	long WillAFExecuteNextIteration();

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
};
