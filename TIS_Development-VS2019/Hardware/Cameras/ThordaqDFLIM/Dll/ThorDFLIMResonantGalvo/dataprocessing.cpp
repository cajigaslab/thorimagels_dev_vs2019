#include "stdafx.h"
#include "dataprocessing.h"


 DataProcessing:: DataProcessing()
{
	memset(datamap,0,sizeof(datamap));

	{
		//folding mapping for positive and negative amplifiers
		//datamap should reflect 14bit resolution of the digitizer

		//negative voltage mapping
		for (int i = 0; i < 8192; i++)
		{
			datamapIndependent[i] = (8191 - i) * 2;
		}

		//positive voltage mapping
		for (int i = 8192; i < 65536; i++)
		{
			datamapIndependent[i] = 0;
		}
	}

	{
		//16 bit mapping with most significant data in positive polarity
		//positive voltage mapping
		for (int i = 0; i < 16384; i++)
		{
			datamapPositiveSigned[i] = static_cast<USHORT>(i-8192);
		}
		//positive voltage mapping
		for (int i = 16384; i < 65536; i++)
		{
			datamapPositiveSigned[i] = 8192;
		}
	}

	{
		//16 bit mapping with most significant data in negative polarity
		//negative voltage mapping
		for (int i = 0; i < 16384; i++)
		{
			datamapNegativeSigned[i] = static_cast<USHORT>((16383 - i) - 8192);
		}
		for (int i = 16384; i < 65536; i++)
		{
			datamapNegativeSigned[i] = static_cast<USHORT>(- 8192);
		}
	}

	{
		//negative voltage mapping
		for (int i = 0; i < 8192; i++)
		{
			datamapPositiveUnsigned[i] = 0;
		}

		//positive voltage mapping
		for (int i = 8192; i < 16384; i++)
		{
			datamapPositiveUnsigned[i] = (i - 8192) * 2;
		}

		//positive voltage mapping
		for (int i = 16384; i < 65536; i++)
		{
			datamapNegativeUnsigned[i] = 16382;
		}
	}

	{
		//negative voltage mapping
		for (int i = 0; i < 8192; i++)
		{
			datamapNegativeUnsigned[i] = (8191 - i) * 2;
		}

		//positive voltage mapping
		for (int i = 8192; i < 65536; i++)
		{
			datamapNegativeUnsigned[i] = 0;
		}
	}

}

 DataProcessing::~ DataProcessing()
{

}


 long DataProcessing::SetupDataMap(long datamap_mode, long channelPolarity[])
{
	unsigned int i;

	switch( datamap_mode)
	{
	case ICamera::POLARITY_INDEPENDENT:
		{
			//folding mapping for positive and negative amplifiers
			//datamap should reflect 14bit resolution of the digitizer
			//negative voltage mapping
			for (i = 0; i <4; i++)
			{
				datamap[i] = &(datamapIndependent[0]);
			}
		}
		break;
	case ICamera::POLARITY_POSITIVE:
		{
			//16 bit mapping with most significant data in positive polarity
			//positive voltage mapping
			for (i = 0; i < 4; i++)
			{
				datamap[i] = &(datamapPositiveSigned[0]);
			}
		}
		break;
	case ICamera::POLARITY_NEGATIVE:
		{
			//16 bit mapping with most significant data in negative polarity
			//negative voltage mapping
			for (i = 0; i < 4; i++)
			{
				datamap[i] = &(datamapNegativeSigned[0]);
			}
		}
		break;
	default:
		for(i=0; i<4; i++)
		{
			datamap[i] = (0 == channelPolarity[i]) ? &(datamapNegativeUnsigned[0]) : &(datamapPositiveUnsigned[0]); 
		}
	}
	return TRUE;
}

 int DataProcessing::ProcessBuffer(USHORT *pFrmData,ULONG channelIndex, ULONG transferLength)
{

	for (ULONG i = 0; i < transferLength; i++)
	{
		*(pFrmData+i) = datamap[channelIndex][*(pFrmData+i)];
	}
	return TRUE;
}