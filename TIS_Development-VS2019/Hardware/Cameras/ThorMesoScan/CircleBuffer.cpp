#include "CircleBuffer.h"

CircleBuffer::CircleBuffer(uint32_t size)
{
	_size = size;
	_buffer = malloc(size);
	Reset();
}

CircleBuffer::~CircleBuffer()
{
	free(_buffer);
}

long CircleBuffer::ResizeBuffer(uint32_t newSize)
{
	_size = newSize;
	_buffer = realloc(_buffer, newSize);
	Reset();
	return TRUE;
}

uint32_t CircleBuffer::DataSize(uint32_t requestSize)
{
	uint32_t availableSize = 0;
	_mtx.lock();
	if (_readOffset > _dataOffset + _size || _dataOffset > _readOffset + _size)
		availableSize = 0;
	else if (_readOffset > _dataOffset)
		availableSize = _dataOffset + _size - _readOffset;
	else
		availableSize = _dataOffset - _readOffset;
	_mtx.unlock();
	if (availableSize < requestSize)
	{
		if (_isWriteOver)
			return availableSize;
		else
			return 0;
	}
	else
		return requestSize;
}

uint32_t CircleBuffer::IdleSize(uint32_t requestSize)
{
	uint32_t availableSize = 0;
	_mtx.lock();
	if (_readOffset > _takeOffset + _size || _takeOffset > _readOffset + _size)
		availableSize = 0;
	else if (_readOffset > _takeOffset)
		availableSize = _readOffset - _takeOffset;
	else
		availableSize = _readOffset + _size - _takeOffset;
	_mtx.unlock();
	if (availableSize < requestSize)
		return 0;
	else
		return requestSize;
}

void CircleBuffer::Reset()
{
	memset(_buffer, 0, _size);
	_readOffset = 0;
	_dataOffset = 0;
	_takeOffset = 0;
	_isWriteOver = false;
}

long CircleBuffer::WriteCompleted(uint32_t buffer_size, uint32_t data_size)
{
	if (buffer_size != IdleSize(buffer_size) || buffer_size < data_size)
		return FALSE;
	_mtx.lock();
	_takeOffset = _dataOffset + buffer_size;
	_dataOffset = _dataOffset + data_size;
	if (_takeOffset > _size)
		_takeOffset -= _size;
	if (_dataOffset > _size)
		_dataOffset -= _size;
	_mtx.unlock();
	return TRUE;
}

long CircleBuffer::WriteOver()
{
	_isWriteOver = true;
	return TRUE;
}

long CircleBuffer::GetReadPointer(void** p,uint32_t size)
{
	if (size != DataSize(size))
		return FALSE;
	*p = (char*)_buffer + _readOffset;
	return TRUE;
}
long CircleBuffer::ReadCompleted(uint32_t size)
{
	if (size != DataSize(size) || _readOffset + size > _size)
		return FALSE;
	_mtx.lock();
	memset(((char*)_buffer + _readOffset), 0, size);
	_readOffset += size;
	if (_readOffset >= _size)
		_readOffset -= _size;
	_mtx.unlock();
	return TRUE;
}


long CircleBuffer::ReadLast(void* data, uint32_t size)
{
	if (size != IdleSize(size))
		return FALSE;
	if (_dataOffset + size <= _size)
	{
		memcpy_s(data, size, ((char*)_buffer + _dataOffset), size);
		memset(((char*)_buffer + _dataOffset), 0, size);
	}
	else
	{
		uint32_t size1 = _size - _dataOffset;
		uint32_t size2 = size - size1;
		memcpy_s(data, size1, ((char*)_buffer + _dataOffset), size1);
		memcpy_s(((char*)data + size1), size2, _buffer, size2);
	}
	return TRUE;
}

long CircleBuffer::Write(void* data, uint32_t buffer_size, uint32_t data_size)
{
	if (buffer_size != IdleSize(buffer_size) || buffer_size > data_size)
		return FALSE;
	if (_dataOffset + buffer_size <= _size)
	{
		memcpy_s(data, buffer_size, ((char*)_buffer + _dataOffset), buffer_size);
		memset(((char*)_buffer + _dataOffset), 0, buffer_size);
	}
	else
	{
		uint32_t size1 = _size - _dataOffset;
		uint32_t size2 = buffer_size - size1;
		memcpy_s(data, size1, ((char*)_buffer + _dataOffset), size1);
		memcpy_s(((char*)data + size1), size2, _buffer, size2);
	}
	_dataOffset += data_size;
	if (_dataOffset >= _size)
		_dataOffset -= _size;
	_takeOffset += buffer_size;
	if (_takeOffset >= _size)
		_takeOffset -= _size;
	return TRUE;
}

long CircleBuffer::ReadFirst(void* data, uint32_t size)
{
	if (size != DataSize(size))
		return FALSE;
	if (_readOffset + size <= _size)
	{
		memcpy_s(data, size, ((char*)_buffer + _readOffset), size);
		memset(((char*)_buffer + _readOffset), 0, size);
	}
	else
	{
		uint32_t size1 = _size - _readOffset;
		uint32_t size2 = size - size1;
		memcpy_s(data, size1, ((char*)_buffer + _readOffset), size1);
		memcpy_s(((char*)data + size1), size2, _buffer, size2);
		memset(((char*)_buffer + _readOffset), 0, size1);
		memset(_buffer, 0, size2);
	}
	_readOffset += size;
	if (_readOffset >= _size)
		_readOffset -= _size;
	return TRUE;
}

