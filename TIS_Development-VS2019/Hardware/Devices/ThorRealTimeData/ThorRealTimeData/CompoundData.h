#pragma once

//****************************************************************************************//
//*** data structure to include all kinds of channels into one, has to keep			   ***//
//*** the order of members for consistancy in the mirror structure on C# level.		   ***//
//*** gcLengthCom: data length of global counter, "Com" stands for combined 		   ***//
//*** high & low part into 64 bits unsigned integer.								   ***//
//*** aiLength, diLength: data length of analog input or digital input channels,	   ***//
//*** each includes length_of_one_channel * number_of_channels.						   ***//
//*** ciLengthCom: data length of counter input, "Com" stands for combined			   ***//
//*** high & low part into 64 bits unsigned integer.								   ***//
//*** aiDataPtr, diDataPtr: pointers of analog input or digital input data memory.	   ***//
//*** ciData64Ptr, gCtr64Ptr: pointers of counter input or global counter data memory. ***//
//****************************************************************************************//

extern "C" _declspec(dllexport) typedef struct CompoundDataStruct
{
	size_t				gcLength;
	size_t				aiLength;
	size_t				diLength;
	size_t				ciLength;
	size_t				viLength;
	double*				aiDataPtr;
	unsigned char*		diDataPtr;
	unsigned long*		ciDataPtr;
	double*				viDataPtr;
	unsigned __int64*	gCtr64Ptr;
}CompDataStruct;

typedef struct StimulusSave
{
	bool enable;
	long signalType;
	long stimChannelID;		//0-based
	double threshold;
	std::string lineName;
}StimulusSaveStruct;

typedef void (_cdecl *DataUpdateCallback)(CompoundDataStruct* compoundData);

extern void (*dataPointer)(CompoundDataStruct* compoundData);

class CompoundData
{
private:
	unsigned long*	gCtr;
	CompDataStruct*	strucData;
	long _saving;
	time_t _createTime;
	size_t _localGCtrSize;

	long allocBufferMem();
	long dellocBufferMem();	

public:
	//constructors:
	CompoundData(size_t gcsize,size_t aisize,size_t disize,size_t cisize, size_t visize = 0);
	CompoundData(CompoundData* cd,int interleave);
	CompoundData(CompoundData* cd,StimulusSaveStruct* ssaveMode);

	//functions:
	long CopyCompoundData(CompoundData* cd);
	long SetupGlobalCounter(unsigned long &overflowGCtrCnt);
	long SetupGlobalCounter(unsigned __int64 initGCtr,unsigned __int64 offsetCnt, unsigned long &overflowGCtrCnt);
	long SetSaving(long toSave);
	long SetCreateTime(time_t ctime);

	//get properties:
	size_t GetaiLengthValue();
	size_t GetdiLengthValue();
	size_t GetciLengthValue();
	size_t GetviLengthValue();
	size_t GetgcLengthValue();
	size_t GetgcLengthComValue();
	unsigned long* GetgCtr();
	CompDataStruct* GetStrucData();
	long GetStrucData(CompDataStruct* ptr);
	long GetSaving();
	time_t GetCreateTime();

	//destructor:
	~CompoundData();
};
