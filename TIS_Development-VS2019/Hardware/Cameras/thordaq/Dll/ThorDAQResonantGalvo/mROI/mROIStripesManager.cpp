#include "stdafx.h"
#include "mROIStripesManager.h"
#include "Logger.h"
#include <thread>
#include "..\..\..\..\..\Common\HighPerfTimer.h"

mROIStripesManager::mROIStripesManager()
{
}

mROIStripesManager::~mROIStripesManager()
{

}

std::unique_ptr<mROIStripesManager> mROIStripesManager::_pInstance;
std::once_flag mROIStripesManager::_onceFlag;

mROIStripesManager* mROIStripesManager::GetInstance()
{
	std::call_once(_onceFlag,
		[] {
			_pInstance.reset(new mROIStripesManager);
	});
	return _pInstance.get();
}


long mROIStripesManager::GenerateStripList(Scan* scanInCache, vector<StripInfo*> &stripList)
{
	long stripCount = 0;
	if (scanInCache == NULL) return stripCount;

	for (int i = 0; i < stripList.size(); ++i)
	{
		SAFE_DELETE_PTR(stripList[i]);
	}
	stripList = vector<StripInfo*>();


	unsigned int ACount = static_cast<unsigned int>(scanInCache->ScanAreas.size());
	for (unsigned int aIndex = 0; aIndex < ACount; aIndex++)
	{
		ScanArea* scanArea = scanInCache->ScanAreas.at(aIndex);
		unsigned int SCount = static_cast<unsigned int>((scanInCache->ScanConfig.IsLivingMode) ? 1 : scanInCache->ScanAreas[0]->SizeS);
		for (unsigned int sIndex = 1; sIndex <= SCount; sIndex++)
		{
			for (uint32_t zIndex = 0; zIndex < scanArea->SizeZ; zIndex++)
			{
				uint32_t stripXOffsetInPixel = 0;
				double offsetX = 0;
				while (stripXOffsetInPixel < scanArea->SizeX)
				{
					StripInfo* strip = new StripInfo();
					size_t saBufferSize = (scanInCache->Channels.size() * scanArea->SizeX * scanArea->SizeY * scanInCache->GetPixelBytes()) + sizeof(FrameInfo);	//frame size + info
					strip->FullFOVPhysicalSizeUM = scanInCache->FullFOVPhysicalSizeUM;
					strip->XPixelSize = scanInCache->XPixelSize;
					strip->YPixelSize = scanInCache->YPixelSize;
					strip->ScanAreaID = scanArea->ScanAreaID;
					strip->ScanMode = scanInCache->ScanConfig.ScanMode;
					strip->PockelsVolt = scanInCache->ScanConfig.CurrentPower;
					//strip->XPos = scanArea->PositionX + offsetX;
					strip->XPos = scanArea->PositionX + stripXOffsetInPixel * scanInCache->XPixelSize;
					strip->YPos = scanArea->PositionY;
					//strip->ZPos = scanArea->PositionZ + zIndex * scanInCache->ZPixelSize;
					//strip->XPhysicalSize = min(scanInCache->ScanConfig.PhysicalFieldSize, scanArea->PhysicalSizeX - (double)offsetX);
					strip->XPhysicalSize = scanInCache->ScanConfig.PhysicalFieldSize;// min(scanInCache->ScanConfig.PhysicalFieldSize, scanArea->PhysicalSizeX - (double)stripXOffsetInPixel * scanInCache->XPixelSize);
					strip->YPhysicalSize = scanArea->PhysicalSizeY;
					strip->StripeFieldSize = scanInCache->ScanConfig.StripeFieldSize;
					strip->ZPhysicalSize = 0;
					strip->flyToNextStripeSkipLines = TWO_WAY_SCAN == strip->ScanMode ? 256 : 128;
					strip->XPosResonMid = strip->XPos + strip->XPhysicalSize / 2;
					strip->XSize = scanInCache->ScanConfig.StripLength;//min<unsigned long>(scanInCache->ScanConfig.StripLength, scanArea->SizeX - stripXOffsetInPixel);
					strip->ActiveScanAreasIndex = aIndex;
					strip->XSize = static_cast<long>(strip->XSize) - static_cast<long>(strip->XSize) % 4;
					strip->YSize = static_cast<long>(scanArea->SizeY) - static_cast<long>(scanArea->SizeY) % 4;
					if (scanArea->PowerPoints0.size() == 1)
					{
						double power = scanArea->PowerPoints0[0]->PowerPercentage / (double)Constants::HUNDRED_PERCENT;
						if (strip->Power.size() <= 0)
						{
							strip->Power.push_back(power);
						}
						else
						{
							strip->Power[0] = power;
						}

						strip->ZPos = scanArea->PowerPoints0[0]->ZPosition;
					}
					else
					{
						for (int i = 0; i < (int)scanArea->PowerPoints0.size() - 1; i++)
						{
							double power = 0;
							if (scanArea->PowerPoints0[i + 1]->ZPosition >= strip->ZPos)
							{
								
								power = (scanArea->PowerPoints0[i]->PowerPercentage +
									(scanArea->PowerPoints0[i + 1]->PowerPercentage - scanArea->PowerPoints0[i]->PowerPercentage) *
									(strip->ZPos - scanArea->PowerPoints0[i]->ZPosition) /
									(scanArea->PowerPoints0[i + 1]->ZPosition - scanArea->PowerPoints0[i]->ZPosition))
									/ (double)Constants::HUNDRED_PERCENT;
								if (strip->Power.size() <= 0)
								{
									strip->Power.push_back(power);
								}
								else
								{
									strip->Power[0] = power;
								}
								break;
							}

							power = scanArea->PowerPoints0[0]->PowerPercentage / (double)Constants::HUNDRED_PERCENT;
							if (strip->Power.size() <= 0)
							{
								strip->Power.push_back(power);
							}
							else
							{
								strip->Power[0] = power;
							}
							strip->ZPos = scanArea->PowerPoints0[0]->ZPosition;
						}
					}
					for (vector<PowerBox*>::iterator iter = scanArea->PowerBoxs.begin(); iter != scanArea->PowerBoxs.end(); ++iter)
					{
						if (strip->ZPos >= (*iter)->StartZ && strip->ZPos <= (*iter)->EndZ)
						{
							if ((*iter)->PowerROI->Bound.x > strip->XPos + strip->XPhysicalSize || (*iter)->PowerROI->Bound.x + (*iter)->PowerROI->Bound.width < strip->XPos)
								continue;

							bool isExist = false;
							for (vector<pair<ROI*, double>>::iterator iterPower = strip->ROIPower.begin(); iterPower != strip->ROIPower.end(); ++iterPower)
							{
								if ((*iter)->PowerROI->ROIID == (*iterPower).first->ROIID)
								{
									isExist = true;
									break;
								}
							}
							if (isExist)
								continue;
							strip->ROIPower.push_back(pair<ROI*, double>((*iter)->PowerROI, (*iter)->PowerPercentage / (double)Constants::HUNDRED_PERCENT));
						}
					}

					strip->FrameROI.x = stripXOffsetInPixel;
					strip->FrameROI.y = 0;
					strip->FrameROI.width = min<unsigned long>(scanArea->SizeX - stripXOffsetInPixel, scanInCache->ScanConfig.StripLength);
					strip->FrameROI.height = scanArea->SizeY;
					strip->FrameROI.frameWidth = scanArea->SizeX;
					strip->FrameROI.frameHeight = scanArea->SizeY;
					strip->IsFrameStart = (0 == stripXOffsetInPixel && 0 == zIndex && 0 == aIndex) ? true : false;
					stripList.push_back(strip);
					stripCount++;
					stripXOffsetInPixel += scanInCache->ScanConfig.StripLength;
					offsetX += scanArea->PositionX;
				}

			}
		}
	}
	if (stripCount > 0)
	{
		stripList[0]->IsStart = true;
		stripList[stripCount - 1]->IsEnd = true;
		stripList[stripCount - 1]->flyToNextStripeSkipLines = 0;
	}
	return stripCount;
}

