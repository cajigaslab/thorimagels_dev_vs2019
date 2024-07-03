#include "stdafx.h"
#include "dataprocessor.h"


DataProcessor::DataProcessor(BYTE channelEnable, UINT width, UINT height, UINT linesToSkipFromTop, UINT linesToSkipFromBottom, UINT maxTransferNumberOfFrames, USHORT* datamap[MAX_CHANNEL_COUNT], bool imagingOnFLyback)
{
	_channelCount = 0;
	for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
	{
		if ((channelEnable & (0x01 << c)) != 0x00)
		{
			++_channelCount;
		}
	}
	_linesToSkipFromTop = linesToSkipFromTop;
	
	_rawFrameLength = width * height;
	_processedFrameLength = width * (height - linesToSkipFromTop - linesToSkipFromBottom);
	_skippedLinesOffset = linesToSkipFromTop * width;
	_totalLinesOffset = (linesToSkipFromTop + linesToSkipFromBottom) * width;
	_channelEnable = channelEnable;
	_maxTransferNumberOfFrames = maxTransferNumberOfFrames;
	_rawFrameBufferSize = sizeof(USHORT) * _rawFrameLength * _channelCount;
	_processedFrameBufferSize = sizeof(USHORT) * _processedFrameLength * _channelCount;
	_datamap = datamap;

	UINT realHeight = height - linesToSkipFromTop - linesToSkipFromBottom;
	for (int i = 0; i < static_cast<int>(_maxTransferNumberOfFrames); ++i)
	{
		ProcessedFrame* pf = new ProcessedFrame(width, realHeight, _channelCount, 0, width, realHeight, 0, 0);
		vector<ProcessedFrame*> imageAreasData;

		imageAreasData.push_back(pf);

		_pProcessedData.push_back(imageAreasData);
	}

	_imagingOnFLyback = imagingOnFLyback;
	_isFlybackFrame = false;
}

DataProcessor::~DataProcessor()
{
	DeleteProcessedBuffer();
}

long DataProcessor::DeleteProcessedBuffer()
{
	if (_pProcessedData.size() > 0)
	{
		for (int i = 0; i < _pProcessedData.size(); i++)
		{
			for (auto& bufferStruct : _pProcessedData.at(i))
			{
				delete bufferStruct;
			}
			_pProcessedData.at(i).clear();
		}
		_pProcessedData.clear();
	}
	return TRUE;
}

vector<vector<ProcessedFrame*>> DataProcessor::ProcessBuffer(UCHAR** pFrmData, UINT numberOfFrames)
{
	if (pFrmData == NULL)
	{
		return _pProcessedData;
	}
	for (ULONG f = 0; f < numberOfFrames; ++f)
	{		
		short* pDataProcessed = (short*)_pProcessedData.at(f)[0]->Data;
		size_t processedChannel = 0;

		if (_isFlybackFrame)
		{
			//if imaging on flyback, like the else case but from the last pixel to the first
			for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
			{
				if ((_channelEnable & (0x01 << c)) != 0x00)
				{
					if (pFrmData[c] == NULL)
					{
						++processedChannel;
						continue;
					}
					short* pDataRaw = ((short*)pFrmData[c]) + _rawFrameLength * f;

					int x = 0;
					int y = _pProcessedData.at(f)[0]->Height;
					for (ULONG i = 0; i < _rawFrameLength - _totalLinesOffset; ++i)
					{
						size_t index = processedChannel * _processedFrameLength + (_pProcessedData.at(f)[0]->Width)* (y - 1) + x;

						++x;
						if (x == _pProcessedData.at(f)[0]->Width)
						{
							x = 0;
							--y;
						}
						if (0 > *(pDataRaw + i))
						{
							*(pDataProcessed + index + _skippedLinesOffset) = 0;
						}
						*(pDataProcessed + index) = _datamap[c][*(pDataRaw + i + _skippedLinesOffset)];
					}
					++processedChannel;
				}
			}
		}
		else
		{
			for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
			{
				if ((_channelEnable & (0x01 << c)) != 0x00)
				{
					if (pFrmData[c] == NULL)
					{
						++processedChannel;
						continue;
					}
					short* pDataRaw = ((short*)pFrmData[c]) + _rawFrameLength * f;

					for (ULONG i = 0; i < _rawFrameLength - _totalLinesOffset; ++i)
					{
						size_t index = processedChannel * _processedFrameLength + i;
						if (0 > *(pDataRaw + i))
						{
							*(pDataProcessed + index + _skippedLinesOffset) = 0;
						}
						*(pDataProcessed + index) = _datamap[c][*(pDataRaw + i + _skippedLinesOffset)];
					}
					++processedChannel;
				}
			}
		}
		_isFlybackFrame = _imagingOnFLyback && (!_isFlybackFrame);
	}
	return _pProcessedData;
}