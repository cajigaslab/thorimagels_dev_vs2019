#pragma once

//****************************************************************************************//
//*** data structure to include all kinds of channels into one, has to keep			   ***//
//*** the order of members for consistancy in the mirror structure on C# level.		   ***//
//*** data structure (FreqCompData) to include all frequency domain channels,		   ***//
//*** freqLength / freqData: data length of frequency range	values					   ***//
//*** freqFitLength / freqFitData: data length of fitting frequency range values	   ***//
//*** specDataRe/Im: spectral data real/imag part, includes freqLength * channel#	   ***//
//*** vSpecData: virtual spectral data, includes freqLength * channel#				   ***//
//*** specFitData: fitting spectral data, freqFitLength * channel#					   ***//
//****************************************************************************************//

extern "C" _declspec(dllexport) typedef struct FreqCompoundDataStruct
{
	size_t				freqLength;
	size_t				freqFitLength;
	size_t				specDataLength;
	size_t				vspecDataLength;
	double*				freqData;				//frequency range
	double*				specDataRe;
	double*				specDataIm;
	double*				vSpecData;
	double*				freqFitData;			//frequency range for fitting
}FreqCompDataStruct;

typedef void (_cdecl *SpectralUpdateCallback)(FreqCompDataStruct* freqCompoundData);

extern void (*functionPointer)(FreqCompDataStruct* freqCompoundData);

class FreqCompoundData
{
private:
	FreqCompDataStruct*	strucData;
	time_t _createTime;

	long allocBufferMem();
	void dellocBufferMem();	

public:
	//constructors:
	FreqCompoundData(size_t freqSize, size_t fisize = 0, size_t vfsize = 0, size_t freqFitSize = 0);

	//functions:
	long CopyFreqCompoundData(FreqCompoundData* fcd);
	void SetCreateTime();
	long SetFreqFitData(size_t freqFitsize);
	long SetVirtualFreqData(size_t vfsize);

	//get properties:
	size_t GetfreqSizeValue();
	size_t GetfreqFitSizeValue();
	FreqCompDataStruct* GetStrucData();
	long GetStrucData(FreqCompDataStruct* ptr);
	time_t GetCreateTime();

	//destructor:
	~FreqCompoundData();
};
