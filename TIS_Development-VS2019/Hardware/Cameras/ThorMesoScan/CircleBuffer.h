#pragma once
#include "Types.h"
#include <mutex>

template<class T>
class CircleBufferPointer
{
public:
	friend class CircleBuffer;
	CircleBufferPointer operator +(const int offset) const;
	CircleBufferPointer& operator ++();
	CircleBufferPointer operator ++(int);
	CircleBufferPointer operator-(const int offset) const;
	CircleBufferPointer& operator--();
	CircleBufferPointer operator--(int);
	CircleBufferPointer& operator+=(int offset);
	CircleBufferPointer& operator-=(int offset);

	bool operator==(T* pointer) const;
	bool operator==(CircleBufferPointer<T> pointer) const;

	T & operator[](int offset) const;
	T & operator*() const;
	~CircleBufferPointer();
private:
	CircleBufferPointer(void* buffer, uint32_t size, uint32_t offset, uint32_t interval);
	T* _p;
	uint32_t _offset;
	uint32_t _size;
	uint32_t _interval;
};

class CircleBuffer :IBuffer
{
public:
	CircleBuffer(uint32_t size);
	~CircleBuffer();
	uint32_t DataSize(uint32_t size);
	long ResizeBuffer(uint32_t size);
	uint32_t IdleSize(uint32_t size);
	void Reset();

	template <class T>
	CircleBufferPointer<T> GetPointer(uint32_t offset, uint32_t interval);
	long WriteCompleted(uint32_t buffer_size, uint32_t data_size);
	long WriteOver();
	long GetReadPointer(void** p, uint32_t size);
	long ReadCompleted(uint32_t size);

	long ReadLast(void* data, uint32_t size);
	long Write(void* data, uint32_t buffer_size, uint32_t data_size);
	long ReadFirst(void* data, uint32_t size);
	void* GetBuffer() { return _buffer; };

private:
	void* _buffer;
	uint32_t _size;
	uint32_t _readOffset;
	uint32_t _dataOffset;
	uint32_t _takeOffset;
	uint32_t _isWriteOver;
	mutex _mtx;
};

template<class T>
CircleBufferPointer<T>::CircleBufferPointer(void* buffer, uint32_t size, uint32_t offset, uint32_t interval)
{
	_offset = offset;
	_p = (T*)buffer;
	_size = size;
	_interval = interval;
}

template<class T>
CircleBufferPointer<T>::~CircleBufferPointer()
{

}

template<class T>
inline CircleBufferPointer<T> CircleBufferPointer<T>::operator +(const int offset) const
{
	CircleBufferPointer<T> temp = *this;
	temp._offset += offset*_interval;
	if (temp._offset >= _size)
		temp._offset -= _size;
	return temp;
};

template<class T>
inline CircleBufferPointer<T>& CircleBufferPointer<T>::operator++()
{
	_offset += _interval;
	if (_offset >= _size)
		_offset -= _size;
	return *this;
}
template<class T>
inline CircleBufferPointer<T> CircleBufferPointer<T>::operator++(int)
{
	CircleBufferPointer<T> temp = *this;
	_offset += _interval;
	if (_offset >= _size)
		_offset -= _size;
	return temp;
}

template<class T>
inline CircleBufferPointer<T>& CircleBufferPointer<T>::operator--()
{
	if (_offset >= _interval)
	{
		_offset -= _interval;
	}
	else
	{
		_offset = _offset + _size - _interval;
	}
	return *this;
}
template<class T>
inline CircleBufferPointer<T> CircleBufferPointer<T>::operator--(int)
{
	CircleBufferPointer<T> temp = *this;
	if (_offset >= _interval)
	{
		_offset -= _interval;
	}
	else
	{
		_offset += _size - _interval;
	}
	return temp;
}

template<class T>
inline T& CircleBufferPointer<T>::operator*() const
{
	return *(_p + _offset);
};

template<class T>
inline CircleBufferPointer<T>& CircleBufferPointer<T>::operator+=(int offset)
{
	_offset += offset*_interval;
	if (_offset >= _size)
		_offset -= _size;
	return *this;
}
template<class T>
inline CircleBufferPointer<T>& CircleBufferPointer<T>::operator-=(int offset)
{
	int actualOffset = offset*_interval;
	if (_offset >= actualOffset)
	{
		_offset -= actualOffset;
	}
	else
	{
		_offset = _offset + _size - actualOffset;
	}
	return *this;
}

template<class T>
inline CircleBufferPointer<T> CircleBufferPointer<T>::operator-(const int offset) const
{
	CircleBufferPointer<T> temp = *this;
	uint32_t actualOffset = offset*_interval;
	if (temp._offset >= actualOffset)
	{
		temp._offset -= actualOffset;
	}
	else
	{
		temp._offset += _size - actualOffset;
	}
	return temp;
};


template<class T>
inline T & CircleBufferPointer<T>::operator[](int offset) const
{
	int actualOffset = offset*_interval;
	if (_offset + actualOffset < _size)
	{
		return *(_p + _offset + actualOffset);
	}
	else
	{
		return *(_p + _offset + actualOffset - _size);
	}
}

template<class T>
inline bool CircleBufferPointer<T>::operator==(T* pointer) const
{
	return _p + _offset == pointer;
}

template<class T>
inline bool CircleBufferPointer<T>::operator==(CircleBufferPointer<T> pointer) const
{
	return _p + _offset == pointer._p + pointer._offset;
}

template <class T>
CircleBufferPointer<T> CircleBuffer::GetPointer(uint32_t offset, uint32_t interval)
{
	uint32_t actual_offset = 0;
	if (offset + _dataOffset / sizeof(T) >= _size / sizeof(T))
		actual_offset = offset + _dataOffset / sizeof(T) - _size / sizeof(T);
	else
		actual_offset = offset + _dataOffset / sizeof(T);
	CircleBufferPointer<T> p(_buffer, _size / sizeof(T), actual_offset, interval);
	return p;
}
