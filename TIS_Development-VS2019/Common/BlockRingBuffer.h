#pragma once

#include <list>
#include "BufferLock.h"

//*************************************************************************//
//*** class to create a ring memory feature to manage buffer in			***//
//*** First-In-First-Out way. The buffer can be read/write in units		***//
//*** of block, no overflow is allowed.									***//
//*************************************************************************//

class BlockRingBuffer
{
private:

	BufferLock _bufAccess;
	std::list<UCHAR*> _bufArray;
	long _blockSize;				// block size in units
	long _blockSizeInByte;			// block size in bytes
	long _unitSizeInByte;			// unit size in bytes
	long _blockCount;				// target total block counts
	long _sigType;					// signal type, defined at ImageWaveformBuilderDll.h
	long _unitIdx;					// current index of pieces of one block, 0-based

public:

	void (*SpaceAvailableCallback)(long, long);

	//Construct ring buffer of memory blocks
	//[sigType]:SignalType, [unitSizeInByte]: bytes per unit [blockCount]: number of blocks, [blockSize]: number of units in a block
	BlockRingBuffer(long sigType, long unitSizeInByte, long blockCount, long blockSize) 
		:_sigType(sigType)
		,_unitSizeInByte(unitSizeInByte)
		,_blockSize(blockSize)
		,_blockSizeInByte(unitSizeInByte * blockSize)
		,_blockCount(blockCount)
		,_unitIdx(0)
	{
	};

	~BlockRingBuffer()
	{
		ClearBlocks();
	};

	//Invoke callback
	long CheckWritableBlockCounts(long invokeCallback)
	{
		_bufAccess.Enter();
		long available = _blockCount - static_cast<long>(_bufArray.size());

		if(invokeCallback)
			(*SpaceAvailableCallback)(_sigType, available);

		_bufAccess.Leave();
		return available;
	};

	//clean up buffer
	void ClearBlocks()
	{
		_bufAccess.Enter();

		if(1 > _bufArray.size())
		{
			_bufAccess.Leave();
			return;
		}

		while(_bufArray.size())
		{
			if(NULL != _bufArray.front())
			{
				delete[] _bufArray.front();
				_bufArray.front() = NULL;
			}
			_bufArray.pop_front();
		}
		_bufAccess.Leave();
	};

	//Get available number of blocks
	long GetReadableBlockCounts()
	{
		long val = 0;
		_bufAccess.Enter();
		val = static_cast<long>(_bufArray.size());
		_bufAccess.Leave();
		return val;
	};

	//Get block count
	long GetBlockCount()
	{
		return _blockCount;
	};

	//Get unit size in Bytes
	long GetUnitSizeInBytes()
	{
		return _unitSizeInByte;
	};

	//Get size of one block
	long GetBlockSize()
	{
		return _blockSize;
	};

	//Get a block size in bytes
	long GetBlockSizeInByte()
	{
		return _blockSizeInByte;
	};

	//Get signal type
	long GetSignalType(){ return _sigType; };

	//Get unit index from last block, 0: full, otherwise number of units
	long GetUnitsInLastBlock() { return _unitIdx; };

	//Read a block of units out of buffer, return 1 if successful
	long ReadBlocks(UCHAR* pbuffer, long invokeCallback = TRUE)
	{
		long ret = 1;

		_bufAccess.Enter();

		if(1 > _bufArray.size())
		{
			_bufAccess.Leave();
			return 0;
		}

		//copy buffer:
		UCHAR* pDst = pbuffer;
		memcpy_s((void*)pDst, _blockSizeInByte, (void*)(_bufArray.front()), _blockSizeInByte);

		//clear front:
		if(NULL != _bufArray.front())
		{
			delete[] _bufArray.front();
			_bufArray.front() = NULL;
		}
		_bufArray.pop_front();

		//invoke callback for space available to write
		long sizeAvailable = _blockCount - static_cast<long>(_bufArray.size());
		_bufAccess.Leave();

		if (invokeCallback)
			(*SpaceAvailableCallback)(_sigType, sizeAvailable);

		return ret;
	};

	///<summary> Read number of units out of the last block only, return 1 if successful </summary>
	///<param name="pbuffer"> buffer pointer for output </param>
	///<param name="offset"> offset in units from the beginning of the block since no circular mechanism within block </param>
	///<param name="numUnits"> unit counts to be read </param>
	long ReadUnits(UCHAR* pbuffer, long offset, long numUnits, long invokeCallback = TRUE)
	{
		long ret = 1;

		_bufAccess.Enter();

		if(1 > _bufArray.size())
		{
			_bufAccess.Leave();
			return 0;
		}

		//copy buffer:
		UCHAR* pDst = pbuffer;
		UCHAR* pSrc = _bufArray.front();
		pSrc += (offset * _unitSizeInByte);
		long unitToRead = (_unitIdx < numUnits) ? _unitIdx : numUnits;
		memcpy_s((void*)pDst, unitToRead * _unitSizeInByte, (void*)(pSrc), unitToRead * _unitSizeInByte);
		_unitIdx -= unitToRead;

		//clear front if no units left
		if(0 >= _unitIdx)
		{
			_unitIdx = 0;
			if(NULL != _bufArray.front())
			{
				delete[] _bufArray.front();
				_bufArray.front() = NULL;
			}
			_bufArray.pop_front();
		}

		//invoke callback for space available to write
		long sizeAvailable = _blockCount - static_cast<long>(_bufArray.size());
		_bufAccess.Leave();

		if (invokeCallback)
			(*SpaceAvailableCallback)(_sigType, sizeAvailable);

		return ret;
	};

	//Write number of block of units into buffer, return 1 if successful
	long WriteBlocks(const UCHAR* pbuffer, long numBlock)
	{
		long ret = 1;

		_bufAccess.Enter();
		try
		{
			if(_blockCount < static_cast<long>(_bufArray.size()) + numBlock)
			{
				_bufAccess.Leave();
				return 0;
			}

			const UCHAR *src = pbuffer;
			for (int i = 0; i < numBlock; i++)
			{
				_bufArray.push_back(new UCHAR[_blockSizeInByte]);
				memcpy_s((void*)(_bufArray.back()), _blockSizeInByte, (void*)(src), _blockSizeInByte);
				src += _blockSizeInByte;
			}

			_unitIdx = 0;
		}
		catch(...)
		{
			ret = 0;
		}
		_bufAccess.Leave();
		return ret;
	};

	//Write number of units into block buffers, allow more than one block size, return 1 if successful
	long WriteUnits(const UCHAR* pbuffer, long numUnits)
	{
		long ret = 1;

		_bufAccess.Enter();
		try
		{
			long numBlockToExtend = 0;
			if (_blockSize < (_unitIdx + numUnits))
			{
				numBlockToExtend = (0 != ((_unitIdx + numUnits) % _blockSize) && 1 < ((double)(_unitIdx + numUnits) / (double)_blockSize)) ? ((_unitIdx + numUnits) / _blockSize) + 1 : ((_unitIdx + numUnits) / _blockSize);
			}

			//return if over the block count limit
			if (_blockCount < static_cast<long>(_bufArray.size()) + numBlockToExtend)
			{
				_bufAccess.Leave();
				return 0;
			}

			//write units until done
			long idxRemain = numUnits;
			UCHAR* pSrc = (UCHAR*)pbuffer;
			while (0 < idxRemain)
			{
				if (0 == _unitIdx)
				{
					_bufArray.push_back(new UCHAR[_blockSizeInByte]);
				}

				long numUnitToWrite = (_blockSize < (_unitIdx + idxRemain)) ?  (std::max)((long)0, (_blockSize - _unitIdx)) : (std::max)((long)0, idxRemain);
				size_t numUnitsInBytes = numUnitToWrite * _unitSizeInByte;
				memcpy_s((void*)(_bufArray.back() + _unitIdx * _unitSizeInByte), numUnitsInBytes, (void*)(pSrc), numUnitsInBytes);
				pSrc += numUnitsInBytes;
				idxRemain -= numUnitToWrite;
				_unitIdx += numUnitToWrite;

				//block is full
				if(_blockSize <= _unitIdx)
					_unitIdx = 0;
			}
		}
		catch(...)
		{
			ret = 0;
		}
		_bufAccess.Leave();
		return ret;
	};

};
