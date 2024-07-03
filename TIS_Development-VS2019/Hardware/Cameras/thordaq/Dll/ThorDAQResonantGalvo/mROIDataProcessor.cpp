#include "stdafx.h"
#include "mROIDataProcessor.h"
#include "mROI/mROIExperimentLoader.h"

mROIDataProcessor::mROIDataProcessor(BYTE channelEnable, UINT maxTransferNumberOfFrames, USHORT* datamap[MAX_CHANNEL_COUNT], vector<StripInfo*> imageAreas, bool horizontalFlip)
{
	_channelEnable = channelEnable;
	_maxTransferNumberOfFrames = maxTransferNumberOfFrames;
	_datamap = datamap;
	_imageAreas = imageAreas;
	_horizontalFlip = horizontalFlip;
	_channelCount = 0;
	for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
	{
		if ((channelEnable & (0x01 << c)) != 0x00)
		{
			++_channelCount;
		}
	}

	_frameLength = 0;
	_totalWidth = 0;
	_totalheight = 0;


	for (int i = 0; i < imageAreas.size(); ++i)
	{
		_frameLength += imageAreas[i]->YSize * imageAreas[i]->XSize;
		_totalWidth += imageAreas[i]->XSize;
		_totalheight += imageAreas[i]->YSize;
	}

	for (int i = 0; i < static_cast<int>(_maxTransferNumberOfFrames); ++i)
	{
		UINT currentScanAreaID = 0;
		UINT areaWidth = 0;
		UINT areaHeight = 0;
		bool areaChanged = true;
		vector<ProcessedFrame*> imageAreasData;
		vector<ProcessedFrame*> intermidiateImageAreasData;
		UINT stripesInArea = 0;
		for (int i = 0; i < imageAreas.size(); ++i)
		{
			currentScanAreaID = imageAreas[i]->ScanAreaID;
			
			if ((imageAreas.size() > i + 1 && currentScanAreaID != imageAreas[i + 1]->ScanAreaID) ||
				(imageAreas.size() == i + 1))
			{
				areaWidth += imageAreas[i]->XSize;
				areaHeight = imageAreas[i]->YSize;
				++stripesInArea;
				ProcessedFrame* pf = new ProcessedFrame(areaWidth, areaHeight, _channelCount, currentScanAreaID, areaWidth, areaHeight, 0, 0);
				pf->StripesInArea = stripesInArea;
				imageAreasData.push_back(pf);

				areaWidth = 0;
				areaHeight = 0;
				stripesInArea = 0;
			}
			else
			{
				areaWidth += imageAreas[i]->XSize;
				++stripesInArea;
			}
		}
		_pProcessedDataAreas.push_back(imageAreasData);
	}

	_frameBufferSize = sizeof(USHORT) * _frameLength * _channelCount;
}

mROIDataProcessor::~mROIDataProcessor()
{
	DeleteProcessedBuffer();
}

long mROIDataProcessor::DeleteProcessedBuffer()
{
	if (_pProcessedDataAreas.size() > 0)
	{
		for (int i = 0; i < _pProcessedDataAreas.size(); i++)
		{
			for (auto& bufferStruct : _pProcessedDataAreas.at(i))
			{
				delete bufferStruct;
			}
			_pProcessedDataAreas.at(i).clear();
		}
		_pProcessedDataAreas.clear();
	}
	return TRUE;
}

vector<vector<ProcessedFrame*>> mROIDataProcessor::ProcessBuffer(UCHAR** pFrmData, UINT numberOfFrames)
{
	if (pFrmData == NULL)
	{
		return _pProcessedDataAreas;
	}if (pFrmData == NULL)
	{
		return _pProcessedDataAreas;
	}
	USHORT* intermediateBuffer = new USHORT[_imageAreas[0]->XSize]();

	for (ULONG f = 0; f < numberOfFrames; ++f)
	{
		size_t processedChannel = 0;
		for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
		{
			if ((_channelEnable & (0x01 << c)) != 0x00)
			{
				if (pFrmData[c] == NULL)
				{
					++processedChannel;
					continue;
				}
				short* pDataRaw = ((short*)pFrmData[c]) + _frameLength * f;
				int nextStripeOffset = 0;

				int areaIndex = 0;
	
				for (int s = 0, offsetIndex = 0; s < _imageAreas.size(); ++s,++offsetIndex)
				{
					if (areaIndex != _imageAreas[s]->ActiveScanAreasIndex)
					{
						offsetIndex = 0;
					}
					areaIndex = _imageAreas[s]->ActiveScanAreasIndex;

					ProcessedFrame* proccessedFrame = _pProcessedDataAreas.at(f)[areaIndex];
					short* pDataProcessed = (short*)proccessedFrame->Data;
					UINT width = proccessedFrame->Width;
					size_t channelOffset = proccessedFrame->GetDataLengthPerChannel() * processedChannel;
					size_t xOffset = offsetIndex * proccessedFrame->Width / proccessedFrame->StripesInArea;

					for (ULONG y = 0; y < _imageAreas[s]->YSize; ++y)
					{
						size_t lineIndex = width * y;
						int collocationStripe = proccessedFrame->StripesInArea - offsetIndex - 1;
						for (ULONG x = 0; x < _imageAreas[s]->XSize; ++x)
						{
							size_t rawIndex = _imageAreas[s]->XSize * y + x + nextStripeOffset;

							if (_horizontalFlip)
							{								
								int flipXOffset = collocationStripe * _imageAreas[s]->XSize;
								size_t processedIndex = lineIndex + flipXOffset + _imageAreas[s]->XSize - x - 1 + channelOffset;
								
								if (0 > pDataRaw[rawIndex])
								{
									*(pDataProcessed + processedIndex) = 0;
								}
								else
								{
									*(pDataProcessed + processedIndex) = _datamap[c][*(pDataRaw + rawIndex)];
								}
							}
							else
							{
								size_t processedIndex = lineIndex + x + xOffset + channelOffset;
								if (0 > pDataRaw[rawIndex])
								{
									*(pDataProcessed + processedIndex) = 0;
								}								
								else
								{
									*(pDataProcessed + processedIndex) = _datamap[c][*(pDataRaw + rawIndex)];
								}
							}							
						}
					}

					nextStripeOffset += _imageAreas[s]->XSize * (_imageAreas[s]->YSize + _imageAreas[s]->flyToNextStripeSkipLines);
				}
				++processedChannel;
			}
		}
	}
	return _pProcessedDataAreas;
}