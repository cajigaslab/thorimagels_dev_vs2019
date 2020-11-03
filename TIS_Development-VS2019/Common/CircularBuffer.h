#pragma once

#include <stdio.h>
#include <string>
#include "BufferLock.h"

//**************************************************************************//
//*** class to create a circular memory feature of bytes, allow us		 ***//
//*** to deal with all different type of data; capacity is to show		 ***//
//*** total memory space allocated, and size is occupied memory.		 ***//
//*** allowOverflow(1): allows to write memory even when fully occupied, ***//
//*** it will circle back to start location, read memory will clear 	 ***//
//*** part of memory.													 ***//
//*** allowOverflow(0):It will not allow to write any more once the		 ***//
//*** memory is fully occupied, and only read memory will clear part of  ***//
//*** the memory for next writing.										 ***//
//**************************************************************************//

[event_source(native)]
class CircularBuffer
{
private:
	size_t _startIndex, _endIndex, _size, _capacity;
	char* _data;
	char* _pTmp;
	int _allowOverflow; ///<allow over-write buffer if not read fast enough
	BufferLock _bufAccess;
	std::string _name;

public:
	__event void BufferSizeReachedEvent();

	CircularBuffer(std::string name)
		: _name(name)
		, _startIndex(0)
		, _endIndex(0)
		, _size(0)
		, _capacity(0)
		, _data(NULL)
		, _pTmp(NULL)
		, _allowOverflow(FALSE)
	{};
	~CircularBuffer(){};

	// Allocate memory
	void AllocMem(size_t capacity)
	{
		_capacity = capacity;
		if (NULL == _data)
		{
			_data = (char*)std::malloc(_capacity);
		}
		else
		{
			_pTmp = (char*)std::realloc(_data, _capacity);
			if (_pTmp != NULL)
			{
				_data = _pTmp;
			}
		}		
	};

	// Set allow overflow
	void SetOverflow(int allow) { _allowOverflow = allow; }

	// Get circular buffer size in bytes
	size_t Size() const { return _size; }

	// Get current capacity in bytes
	size_t Capacity() const { return _capacity; }

	// Get buffer size is sufficient for generating event
	long IsFull() const { return (_capacity <= _size) ? TRUE : FALSE; }

	// Write memory into the circular buffer, return number of bytes written
	size_t Write(const char *data, size_t bytes, bool lock = false)
	{
		if ((NULL == data) || (NULL == _data) || (0 == bytes)) return 0;

		size_t capacity = _capacity;
		size_t writeBytes = (TRUE == _allowOverflow) ? (bytes) : std::min(bytes, (capacity - _size));
		if(0 == writeBytes) return 0;

		if(lock) _bufAccess.Enter();

		// Write in a single step
		if (writeBytes <= capacity - _endIndex)
		{
			memcpy_s(_data + _endIndex, writeBytes, data, writeBytes);
			_endIndex += writeBytes;
			if (_endIndex == capacity) _endIndex = 0;
		}
		// Write in two steps
		else
		{
			size_t size1 = capacity - _endIndex;
			memcpy_s(_data + _endIndex, size1, data, size1);
			size_t size2 = writeBytes - size1;
			memcpy_s(_data, size2, data + size1, size2);
			_endIndex = size2;
		}

		size_t addBytes = ((capacity - _size) < writeBytes) ? (capacity - _size) : writeBytes;
		_size += addBytes;		

		if(lock) _bufAccess.Leave();

		//notice of circular buffer is reached target size
		if(_capacity <= _size)
		{
			BufferSizeReachedEvent();
		}

		return writeBytes;
	};

	// Read memory out of the circular buffer while not writing, return number of bytes read
	size_t Read(char *data, size_t bytes, bool tryLock = false)
	{
		if ((NULL == _data) || (bytes == 0)) 
			return 0;

		if(tryLock)
		{
			if(!_bufAccess.TryEnter())
				return 0;
		}

		size_t capacity = _capacity;
		size_t readBytes = std::min(bytes,_size);

		// Read in a single step
		if (readBytes <= capacity - _startIndex)
		{
			memcpy_s(data, readBytes, _data + _startIndex, readBytes);
			_startIndex += readBytes;
			if (_startIndex == capacity) _startIndex = 0;
		}
		// Read in two steps
		else
		{
			size_t size1 = capacity - _startIndex;
			memcpy_s(data, size1, _data + _startIndex, size1);
			size_t size2 = readBytes - size1;
			memcpy_s(data + size1, size2, _data, size2);
			_startIndex = size2;
		}

		if(tryLock)
			_bufAccess.Leave();

		_size -= readBytes;
		return readBytes;
	};

	// Copy memory from the circular buffer without changing size, return copied buffer (user responsibility to release)
	unsigned char* Copy(size_t bytes, bool tryLock = false)
	{
		if ((NULL == _data) || (bytes == 0)) 
			return NULL;

		if(tryLock)
		{
			if(!_bufAccess.TryEnter())
				return NULL;
		}

		size_t capacity = _capacity;
		size_t readBytes = std::min(bytes,_size);
		unsigned char* pOut = (unsigned char*)malloc(readBytes);

		// Read in a single step
		if (readBytes <= capacity - _startIndex)
		{
			memcpy_s(pOut, readBytes, _data + _startIndex, readBytes);
			_startIndex += readBytes;
			if (_startIndex == capacity) _startIndex = 0;
		}
		// Read in two steps
		else
		{
			size_t size1 = capacity - _startIndex;
			memcpy_s(pOut, size1, _data + _startIndex, size1);
			size_t size2 = readBytes - size1;
			memcpy_s(pOut + size1, size2, _data, size2);
			_startIndex = size2;
		}

		if(tryLock)
			_bufAccess.Leave();

		return pOut;
	};

	// Clear memory by resetting indexes
	void Clear()
	{
		_startIndex = _endIndex = _size = 0;
	}

	// Retrieve for current memory pointer
	char* GetEndPtr()
	{
		return &_data[_endIndex];
	};

	//try access memory, return false if not available
	BOOL TryLock() { return _bufAccess.TryEnter(); }

	//access memory, return when available
	void Lock() { _bufAccess.Enter(); }

	//release access of memory
	void ReleaseLock() { _bufAccess.Leave(); }

	// Release memory
	void ReleaseMem()
	{
		_bufAccess.Enter();

		if(NULL != _data)
		{
			free(_data);
			_data = NULL;
		}
		_startIndex = _endIndex =_size = 0;

		_bufAccess.Leave();
	};
};
