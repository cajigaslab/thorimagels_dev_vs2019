#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#define INPUT_DEPTH 14
#include "IDataProcessor.h"
class  DataProcessor : public IDataProcessor
{
public:
	DataProcessor(BYTE channelEnable, ULONG rawPixelX, ULONG rawPixelY, ULONG linesToAverage, ULONG maxTransferNumberOfFrames, USHORT* datamap[MAX_CHANNEL_COUNT]);
	~DataProcessor();
	vector<UCHAR*> ProcessBuffer(UCHAR** pFrmData, ULONG numberOfFrames);
	

private:
	long DeleteProcessedBuffer();
	long SetupProcessedBuffer();
	//TODO: move the datamaps to outside of this class and only pass the correct datamap before acquisition, instead of having to build the maps for every acquisition
	USHORT** _datamap;///<datamap array

	vector<UCHAR*> _pFrameData;
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
};

#endif
