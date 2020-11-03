// Sample.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Position.h"
#include "Sample.h"

Sample::Sample()
{
	_sampleOffsetX = 0;
	_sampleOffsetY = 0;
	_totalRows = 0;
	_totalCols = 0;
	_rows = 0;
	_cols = 0;
	_currentRow = -1;
	_currentCol = -1;
}

void Sample::GoToOffset(double offsetX, double offsetY)
{
}
void Sample::GoToSiteAndIndex(long site, long index)
{
}
void Sample::GoToSiteAndOffset(long site, double offsetX, double offsetY)
{
}
void Sample::GoToWellSite(long row, long col, long subRow, long subCol)
{
}
void Sample::GoToWellSiteAndIndex(long row, long col, long index)
{
}
void Sample::GoToWellSiteAndOffset(long row, long col, long subRow, long subColumn, double sampleOffsetX, double sampleOffsetY, double wellOffsetX, double wellOffsetY, double transOffsetX, double transOffsetY, double subOffsetX, double subOffsetY, sampleFPPrototype dm)
{
	if((row < 0)||(row >= _rows)||(col < 0)||(col >= _cols))
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"Sample Sample::GoToWellSiteAndOffset invalid row or col");
		return;
	}

	double x,y;

	x = sampleOffsetX + transOffsetX + col*wellOffsetX + subColumn*subOffsetX;
	y = sampleOffsetY + transOffsetY + row*wellOffsetY + subRow*subOffsetY;

	sampleFunctionPointer = dm;
	if(sampleFunctionPointer != NULL)
	{
		(*sampleFunctionPointer)(x,y);
	}
}

void Sample::Add(SampleSite site)
{
	_sampleSites.push_back(site);
}

void Sample::SetDimensions(long rows,long cols)
{
	_rows = rows;
	_cols = cols;	
}

void Sample::SetDimensionsMosaic(long startRow, long startCol, long totalRows, long totalCols, long rows,long cols,long subrows,long subcols)
{
	_startRow = startRow;
	_startCol = startCol;
	_totalRows = totalRows;
	_totalCols = totalCols;
	_rows = rows;
	_cols = cols;
	_subrows = subrows;
	_subcols = subcols;
}

void Sample::GoToAllWellSites(IDevice* xyStage, IAcquire* acquire, IExperiment* pExp)
{
	double x,y;
	long counter = 0;
	long captureStatus;
	long captureMode;
	pExp->GetCaptureMode(captureMode);
	double intervalSec;
	long timePoints,triggerModeTimelapse;
	pExp->GetTimelapse(timePoints,intervalSec,triggerModeTimelapse);

	//The timepoint location can be manipulated at runtime. This is
	//to allow the rearrangement of time loops with other logic. 
	//To keep track of the true timepoint set the TimelapseTOffset value
	long captureTimes = 1;
	if (captureMode == IExperiment::Z_AND_T)
	{
		captureTimes = timePoints;
	}
	for (int t = 0; t < captureTimes; t++)
	{
		pExp->SetTimelapseTOffset(t);
		_it = _sampleSites.begin();
		HANDLE hTimer = NULL;
		LARGE_INTEGER liDueTime;

		liDueTime.QuadPart = static_cast<LONGLONG>(-10000000LL * intervalSec);

		// Create an unnamed waitable timer.
		hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
		if (NULL == hTimer)
		{
			return;
		}

		// Set a timer to wait for intervalSec seconds.
		if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
		{
			return;
		}

		if (_sampleSites.size() > 0 )
		{
			for (int i = 0; i < _sampleSites.size(); i++)
			{
				//Check the Stop event has occurred
				acquire->StopCaptureEventCheck(captureStatus);
			
				//In the event of STOP exit the loop
				if(captureStatus)
				{
					return;
				}
				_it[i].Get(0).Get(x,y);

				acquire->CallCaptureSubImage(_it[i].GetIndex() +1);   //this place need changes

				xyStage->SetParam(IDevice::PARAM_X_POS,x);
				xyStage->SetParam(IDevice::PARAM_Y_POS,y);
				xyStage->PreflightPosition();
				xyStage->SetupPosition();
				xyStage->StartPosition();

				wsprintf(message,L"Sample GoToAllWellSites X %d.%d",(int)x,(int)abs(static_cast<long>((x - static_cast<long>(x))*1000)));
				logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
				wsprintf(message,L"Sample GoToAllWellSites Y %d.%d",(int)y,(int)abs(static_cast<long>((y - static_cast<long>(y))*1000)));
				logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

				long status = IDevice::STATUS_BUSY;

				do
				{
					xyStage->StatusPosition(status);
				}
				while(status == IDevice::STATUS_BUSY);

				if(status != IDevice::STATUS_READY)
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"Sample GoToAllWellSites Device error");
					break;
				}
		
				if(acquire->Execute(1, _it[i].GetIndex() +1) != TRUE)
				{
					logDll->TLTraceEvent(INFORMATION_EVENT,1,L"Sample GoToAllWellSites Acquire execute failed");
					break;
				}

				/*counter++;
				subCount++;*/

				acquire->CallSaveSubImage(_it[i].GetIndex() +1);//this place need changes
			}
		}
		else
		{
			//Check the Stop event has occurred
			acquire->StopCaptureEventCheck(captureStatus);
			
			//In the event of STOP exit the loop
			if(captureStatus)
			{
				return;
			}

			if(acquire->Execute(1, 1) != TRUE)
			{
				logDll->TLTraceEvent(INFORMATION_EVENT,1,L"Sample GoToAllWellSites Acquire execute failed");
				break;
			}
		}

		//Don't wait after the last timepoint is done
		if (t < captureTimes - 1)
		{
			std::clock_t start;
			double duration;

			start = std::clock();
			while(WaitForSingleObject(hTimer, 1) != WAIT_OBJECT_0)
			{
				//Check the Stop event has occurred
				acquire->StopCaptureEventCheck(captureStatus);
				//In the event of STOP exit the loop
				if(captureStatus)
				{
					break;
				}
				Sleep(1);
				duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
				if (0.5 <= duration)
				{
					start = std::clock();
					//Update the view. If the index is -1, the observer will use the last available index
					acquire->CallSaveImage(-1, FALSE);
				}
			}
		}
		CloseHandle(hTimer);
	}

	acquire->CaptureComplete(TRUE);
}
