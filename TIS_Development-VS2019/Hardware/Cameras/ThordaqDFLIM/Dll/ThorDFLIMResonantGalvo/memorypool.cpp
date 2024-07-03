#include "stdafx.h"
#include "memorypool.h"


// Constructor

MemoryPool::MemoryPool()
{
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

ULONG MemoryPool::AllocMemory(UINT channel, ULONG64 size)
{
	if (channel >= CHANNEL_NUM)
	{
		return FALSE;
	}
	vMemoryLayout[channel].startAddress = CalMemoryInUse() + this->aStartAddress;
	vMemoryLayout[channel].size = size;
	return TRUE;
}

ULONG MemoryPool::ClearUpMemory()
{
	ULONG status = FALSE;

	for (int i = 0; i < CHANNEL_NUM; i++)
	{
		vMemoryLayout[i].size = 0;
		vMemoryLayout[i].startAddress = 0;
	}

	return status;
}

ULONG64 MemoryPool::CalMemoryInUse()
{
	ULONG64 size = 0;
	for (int i = 0; i < CHANNEL_NUM; i++)
	{
		size += vMemoryLayout[i].size;
	}
	return size;
}

ULONG MemoryPool::GetMemoryPropertyByChannel(UINT channel, ULONG64& rStartAddress, ULONG64& rSize)
{

	if (channel >= CHANNEL_NUM)
	{
		return FALSE;
	}
	
	rStartAddress = vMemoryLayout[channel].startAddress;
	rSize = vMemoryLayout[channel].size;

	return TRUE;
}
