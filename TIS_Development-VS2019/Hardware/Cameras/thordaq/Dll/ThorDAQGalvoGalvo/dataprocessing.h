#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#define INPUT_DEPTH 14

class  DataProcessing
{
public:
	 DataProcessing();
	~ DataProcessing();

	long SetupDataMap(long datamap_mode, long channelPolarity[]);
	int ProcessBuffer(short *pFrmData,ULONG channelIndex, ULONG transferLength);


private:
	USHORT* datamap[4];///<datamap array
	USHORT  datamapPositiveSigned[65536];///<datamap for positive voltages
	USHORT  datamapNegativeSigned[65536];///<datamap for negative voltages
	USHORT  datamapPositiveUnsigned[65536];///<datamap for positive voltages
	USHORT  datamapNegativeUnsigned[65536];///<datamap for negative voltages
	USHORT  datamapIndependent[65536];///<datamap that folds the positive and negative
};

#endif
