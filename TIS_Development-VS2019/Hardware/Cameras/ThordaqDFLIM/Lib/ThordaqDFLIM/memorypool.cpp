#include "stdafx.h"
#include "memorypool.h"


// Constructor

MemoryPool::MemoryPool()
{
	this->vMemoryLayout = vector<BUFFER_STRUCT>();
	return;
}

MemoryPool::MemoryPool( ULONG64 maxCapacity , ULONG64 startAdress)
{
	MemoryPool();
	this->aStartAddress = startAdress;
	this->aMemoryCapacity = maxCapacity - startAdress;
	return;
}
	
// Destructor
MemoryPool::~MemoryPool()
{
	ClearUpMemory();
	return;
}


ULONG MemoryPool::RequestMemoryAllocation(ULONG64 size, ULONG64& rStartAddress)
{
	ULONG status = FALSE;
	if (size > GetRemainingBufferSize())
	{
		status = FALSE;
	}else
	{
		rStartAddress = CalMemoryInUse() + this->aStartAddress;
		status = TRUE;
	}
	return status;
}

ULONG64 MemoryPool::GetRemainingBufferSize()
{
	return aMemoryCapacity - CalMemoryInUse();
}

ULONG MemoryPool::AllocMemory(ULONG64 size)
{
	ULONG status = TRUE;
	BUFFER_STRUCT buffer = BUFFER_STRUCT();
	buffer.startAddress = CalMemoryInUse() + this->aStartAddress;
	buffer.size = size;
	vMemoryLayout.push_back(buffer);
	return status;
}

ULONG MemoryPool::ClearUpMemory()
{
	ULONG status = FALSE;

	vMemoryLayout.clear();

	return status;
}

ULONG64 MemoryPool::CalMemoryInUse()
{
	ULONG64 size = 0;
	if (vMemoryLayout.empty())
	{
		return size;
	}

	for (std::vector<BUFFER_STRUCT>::iterator it = vMemoryLayout.begin(); it != vMemoryLayout.end(); ++it)
	{
		size += it->size;
	}
	return size;
}

ULONG MemoryPool::GetMemoryPropertyByIndex(UINT index, ULONG64& rStartAddress, ULONG64& rSize)
{

	ULONG status = TRUE;

	if (index >= vMemoryLayout.size())
	{
		status = FALSE;
	}else
	{
		rStartAddress = vMemoryLayout[index].startAddress;
		rSize = vMemoryLayout[index].size;
	}

	return status;
}

UINT MemoryPool::GetTotalNumberOfBuffer()
{
	return static_cast<UINT>(vMemoryLayout.size());
}