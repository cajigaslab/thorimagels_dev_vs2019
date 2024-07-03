#pragma once
#include "stdafx.h"
#include "IDataProcessor.h"

class ICircularBuffer
{
public:	
	virtual size_t WriteFrames(const vector<vector<ProcessedFrame*>> pFrmData, size_t NumFrames) = 0;
	virtual long GetNextFrame(vector<ProcessedFrame*> pbuffer) = 0;
	virtual void Reset() = 0;
	virtual ~ICircularBuffer()
	{
		// Compulsory virtual destructor definition
	}
};