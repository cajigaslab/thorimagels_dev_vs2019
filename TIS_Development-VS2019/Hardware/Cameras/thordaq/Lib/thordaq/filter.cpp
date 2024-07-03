#include "stdafx.h"
#include "filter.h"


void FIRFilter::initializeFilterTable()
{
	for (int i = 0; i <= 8; i++) //max downsample rate 2^8
	{
		int downsample_rate = static_cast<int>(pow(2,i));
		int filter1_tap = min(FIR_FILTER_TAP_COUNT,downsample_rate);
		int freq = 1;
		for (int filter2_tap = FIR_FILTER_TAP_COUNT; filter2_tap > 0; filter2_tap--)
		{
			if (filter1_tap == 1 && filter2_tap == 1)
				continue;
			while(freq <= INT_MAX)
			{
				double amp = abs(sin(M_PI * freq  * min(16,filter1_tap)/ ADC_MAX_SAMPLE_RATE) / (filter1_tap * sin(M_PI * freq / ADC_MAX_SAMPLE_RATE))) 
					*  abs(sin(M_PI * freq  * filter2_tap/ (ADC_MAX_SAMPLE_RATE / downsample_rate)) / (filter2_tap * sin(M_PI * freq / (ADC_MAX_SAMPLE_RATE / downsample_rate))));
				//double gain = 20.0 * log10f (amp);
				if (amp < 0.708) //-3db //if (gain < -3.0)
				{
					gFirFilterTable[i][filter2_tap-1] = freq;
					break;
				}
				freq+=250;
			}
		}
	}
	gFirFilterTable[0][0] = INT_MAX;

	//int freq = 1;
	//double pulse_interleave_mode_adc_sample_rate = ADC_MAX_SAMPLE_RATE / 2.0;
	//for (int i = FITLER_MAX_TAP; i > 1; i--)
	//{
	//	while(freq <= INT_MAX)
	//	{
	//		double amp = abs(sin(M_PI * freq  * i/ pulse_interleave_mode_adc_sample_rate) / (i * sin(M_PI * freq / pulse_interleave_mode_adc_sample_rate)));
	//		//double gain = 20.0 * log10f (amp);
	//		//if (gain < -3.0)
	//		if (amp < 0.708) //-3db 
	//		{
	//			gPulseInterleaveFirFilterTable[i-1] = freq;
	//			break;
	//		}
	//		freq+=250;
	//	}
	//}
	//gPulseInterleaveFirFilterTable[0] = INT_MAX;
}

FilterSettings FIRFilter::findFilterSettings(double frequency, double adcSampleRate)
{
	UINT16  downsample_rate = static_cast<UINT16>(ceill((log(ADC_MAX_SAMPLE_RATE / adcSampleRate)) * M_LOG2E));
	UINT16  min_offset = UINT16_MAX;
	UINT32  isFound = FALSE;
	FilterSettings filterSettings;
	UINT16  max_downsample_rate = 4;//normal search adc sample rate from 10,000,000 - 160,000,000
	if (downsample_rate > 4) // slow scan, already lose data, keep the max sample rate
	{
		max_downsample_rate = min(8,downsample_rate);
	}
	for (int i = downsample_rate; i <= max_downsample_rate; i++)
	{
		for (int j = 15; j >=0; j--)
		{
			double diff = frequency - gFirFilterTable[i][j];
			if (diff < 0)
			{
				filterSettings.filter1Settings = static_cast<int>(pow(2,i));
				filterSettings.filter2Settings = j + 1;
				break;
			}
			else
			{
				isFound = TRUE;
				if (diff < min_offset)
				{
					filterSettings.filter1Settings = static_cast<int>(pow(2,i));
					filterSettings.filter2Settings = j + 1;
					min_offset  = static_cast<unsigned short>(ceil(diff - 0.5));
				}
			}
		}
		if (isFound) break;
	} 
	return filterSettings;
}

FilterFactory::FilterFactory(FilterType filterType)
{
	if (filterType == FilterType::FIR)
	{
		pFilterFactory = new FIRFilter();
	}else
	{
		pFilterFactory = nullptr;
	}
}

FilterFactory::~FilterFactory()
{
	SAFE_DELETE_PTR(pFilterFactory);
}

Filter* FilterFactory::getFilterFactory()
{
	return pFilterFactory;
}