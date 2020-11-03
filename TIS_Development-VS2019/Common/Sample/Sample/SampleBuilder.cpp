#include "stdafx.h"
#include "Sample.h"
#include "SampleSite.h"
#include "SampleBuilder.h"


SampleBuilder::SampleBuilder()
{
}



Sample *SampleBuilder::CreatePlateMosaicSample(double sampleOffsetX, double sampleOffsetY, double sampleOffsetZ, vector<IExperiment::SubImage>& subImages)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Sample Sample::CreatePlateMosaicSample");
	int imageIndex = 0;
	Sample *s = new Sample();
	for (int i = 0; i < subImages.size(); i++)
	{
		for (int j = 0; j < subImages[i].subRows; j++)
		{
			if (j%2 == 0)
			{
				for (int k = 0; k < subImages[i].subColumns; k++)
				{
					SampleSite site;
					Position pos;
					double x = (sampleOffsetX + subImages[i].transOffsetXMM + k * (1 - subImages[i].overlapX) * subImages[i].scanAreaWidth);
					double y = (sampleOffsetY + subImages[i].transOffsetYMM - j * (1 - subImages[i].overlapY) * subImages[i].scanAreaHeight);
					pos.Set(x, y);
					site.Add(pos);
					site.SetIndex(imageIndex + k);
					s->Add(site);
					wsprintf(message,L"Sample SampleBuilder::CreatePlateMosaicSample X %d.%d",(int)x,(int)abs(static_cast<long>((x - static_cast<long>(x))*1000)));
					logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
					wsprintf(message,L"Sample SampleBuilder::CreatePlateMosaicSample Y %d.%d",(int)y,(int)abs(static_cast<long>((y - static_cast<long>(y))*1000)));
					logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
				}
			}else
			{
				for (int k = subImages[i].subColumns - 1; k >= 0; k--)
				{
					SampleSite site;
					Position pos;
					double x = (sampleOffsetX + subImages[i].transOffsetXMM + k * (1 - subImages[i].overlapX) * subImages[i].scanAreaWidth);
					double y = (sampleOffsetY + subImages[i].transOffsetYMM - j * (1 - subImages[i].overlapY) * subImages[i].scanAreaHeight);
					pos.Set(x, y);
					site.Add(pos);
					site.SetIndex(imageIndex + k);
					s->Add(site);
					wsprintf(message,L"Sample SampleBuilder::CreatePlateMosaicSample X %d.%d",(int)x,(int)abs(static_cast<long>((x - static_cast<long>(x))*1000)));
					logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
					wsprintf(message,L"Sample SampleBuilder::CreatePlateMosaicSample Y %d.%d",(int)y,(int)abs(static_cast<long>((y - static_cast<long>(y))*1000)));
					logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
				}	
			}
			imageIndex += subImages[i].subColumns;
		}
	}
	return s;
}



Sample *SampleBuilder::CreatePlateSample(double sampleOffsetX, double sampleOffsetY, long wellRows, long wellCols, double wellOffsetX, double wellOffsetY)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Sample Sample::CreatePlateSample");
	
	Sample *s = new Sample();

	long row,col;

	for(row=0; row<wellRows; row++)
	{
		for(col=0; col<wellCols; col++)
		{
			SampleSite site;
			Position pos;
			pos.Set(sampleOffsetX + (col*wellOffsetX),sampleOffsetY + (row*wellOffsetY));
			site.Add(pos);
			s->Add(site);			
		}	
	}

	s->SetDimensions(wellRows,wellCols);

	return s;
}
Sample *SampleBuilder::CreateSample(double sampleOffsetX, double sampleOffsetY, PositionsIterator positions)
{
	return (new Sample());
}
Sample *SampleBuilder::CreateSampleFromFile(FILE *file)
{
	return (new Sample());
}