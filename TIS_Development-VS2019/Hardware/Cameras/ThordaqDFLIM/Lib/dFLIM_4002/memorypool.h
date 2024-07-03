#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H


typedef struct _BUFFER_STRUCT
{
	ULONG64 startAddress;
	ULONG64 size;
}BUFFER_STRUCT, *PBUFFER_STRUCT;



class MemoryPool {
public:
	// Constructor
	MemoryPool();

	MemoryPool(ULONG64 maxCapacity, ULONG64 startAdress);
	
	// Destructor
	~MemoryPool();

	ULONG RequestMemoryAllocation(ULONG64 size, ULONG64& rStartAddress);

	ULONG64 GetRemainingBufferSize();

	UINT GetTotalNumberOfBuffer();

	ULONG AllocMemory(ULONG64 size);

	ULONG ClearUpMemory();

	ULONG GetMemoryPropertyByIndex(UINT index, ULONG64& rStartAddress, ULONG64& rSize);
private:
	ULONG64 aMemoryCapacity;
	ULONG64 aStartAddress;

	vector<BUFFER_STRUCT> vMemoryLayout;

	ULONG64 CalMemoryInUse();
};

#endif