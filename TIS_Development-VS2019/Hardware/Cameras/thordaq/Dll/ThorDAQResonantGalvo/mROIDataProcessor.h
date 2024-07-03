#pragma once
#include "stdafx.h"
#include "IDataProcessor.h"
#include "..\..\..\..\..\Common\ThorSharedTypesCPP.h"
#include "mROI/mROIExperimentLoader.h"
struct mROIImageStruct
{
	long pixelX;
	long pixelY;
	long transitionLines;	
};


class mROIDataProcessor :
    public IDataProcessor
{
public:
	mROIDataProcessor(BYTE channelEnable, UINT maxTransferNumberOfFrames, USHORT* datamap[MAX_CHANNEL_COUNT], vector<StripInfo*> imageAreas, bool horizontalFlip);
	~mROIDataProcessor();

	vector<vector<ProcessedFrame*>>  ProcessBuffer(UCHAR** pFrmData, UINT numberOfFrames);

private:
	long DeleteProcessedBuffer();
	USHORT** _datamap;///<datamap array
	vector<vector<ProcessedFrame*>> _pIntermidiateProcessedDataAreas;
	vector<vector<ProcessedFrame*>> _pProcessedDataAreas;
	size_t	_frameBufferSize;
	BYTE _channelEnable;
	UINT _frameLength;
	UINT _maxTransferNumberOfFrames;
	UINT _channelCount;
	vector<StripInfo*> _imageAreas;
	UINT _totalWidth;
	UINT _totalheight;
	bool _horizontalFlip;
};

