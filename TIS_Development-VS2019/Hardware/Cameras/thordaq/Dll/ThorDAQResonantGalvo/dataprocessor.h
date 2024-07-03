#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#define INPUT_DEPTH 14
#include "IDataProcessor.h"
class  DataProcessor : public IDataProcessor
{
public:
	DataProcessor(BYTE channelEnable, UINT width, UINT height, UINT linesToSkipFromTop, UINT linesToSkipFromBottom, UINT maxTransferNumberOfFrames, USHORT* datamap[MAX_CHANNEL_COUNT], bool imagingOnFLyback);
	~DataProcessor();
	vector<vector<ProcessedFrame*>> ProcessBuffer(UCHAR** pFrmData, UINT numberOfFrames);


private:
	long DeleteProcessedBuffer();
	USHORT** _datamap;///<datamap array
	vector<vector<ProcessedFrame*>>  _pProcessedData;
	size_t	_rawFrameBufferSize;
	size_t	_processedFrameBufferSize;
	BYTE _channelEnable;
	UINT _rawFrameLength;
	UINT _processedFrameLength;
	UINT _linesToSkipFromTop;
	size_t _skippedLinesOffset;
	size_t _totalLinesOffset;
	UINT _maxTransferNumberOfFrames;
	UINT _channelCount;
	bool _imagingOnFLyback;
	bool _isFlybackFrame;
};

#endif
