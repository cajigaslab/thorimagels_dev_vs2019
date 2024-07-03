#pragma once
#include "IDataProcessor.h"
#include "..\..\..\..\..\Common\ThorSharedTypesCPP.h"

class MultiPlaneDataProcessor :
        public IDataProcessor
{
public:
	MultiPlaneDataProcessor(BYTE channelEnable, ULONG rawPixelX, ULONG rawPixelY, ULONG maxTransferNumberOfFrames, ULONG linesToAverage, ULONG numberOfPlanes, USHORT* datamap[MAX_CHANNEL_COUNT], ULONG scanmode, ULONG blankLines);
	~MultiPlaneDataProcessor();

	vector<UCHAR*> ProcessBuffer(UCHAR** pFrmData, ULONG numberOfFrames);

private:
	long DeleteProcessedBuffer();
	long SetupProcessedBuffer();
	USHORT** _datamap;///<datamap array
	vector<UCHAR*> _pProcessedData;
	size_t	_rawFrameBufferSize;
	size_t	_processedFrameBufferSize;
	ULONG _rawPixelX;
	ULONG _rawPixelY;
	ULONG _processedPixelX;
	ULONG _processedPixelY;
	BYTE _channelEnable;
	size_t _rawFrameLength;
	size_t _processedFrameLength;
	ULONG _linesToAverage;
	ULONG _maxTransferNumberOfFrames;
	ULONG _channelCount;
	ULONG _numberOfPlanes;
	ULONG _scanMode;
	ULONG _blankLines;
};

