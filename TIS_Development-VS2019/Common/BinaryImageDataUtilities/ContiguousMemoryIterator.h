#pragma once

#include "ImageIteratorImplementation.h"


/// <summary> Used to iterate over a set of memory where all channels of the image are arranged one after another contiguously </summary>
/// <remarks> 
/// The ContiguousMemoryIterator follows the ImageIterator's expected x,y,c,m,z iteration order, which is also the same as
///  its memory model arrangement.
/// </remarks>
template <typename T> class ContiguousMemoryIterator : public ImageIteratorImplementation<T>
{

public:
	ContiguousMemoryIterator(T* startingAddress);

	virtual ImageIteratorImplementation<T>& operator++();
	virtual std::shared_ptr<ImageIteratorImplementation<T> > operator++(int);
	virtual ImageIteratorImplementation<T>& operator--();
	virtual std::shared_ptr<ImageIteratorImplementation<T> > operator--(int);
	virtual T& operator* () const;
	virtual T* toPointer() const;
	virtual std::shared_ptr<ImageIteratorImplementation<T> > createCopy();



private:
	T* address;

};

/// <summary> Constructs Contiguous Memory Iterator </summary>
template <typename T> ContiguousMemoryIterator<T>::ContiguousMemoryIterator(T* startingAddress)
{
	address = startingAddress;
}


template <typename T> ImageIteratorImplementation<T>& ContiguousMemoryIterator<T>::operator++()
{
	++address;
	return *this;
}   

template <typename T> std::shared_ptr<ImageIteratorImplementation<T> > ContiguousMemoryIterator<T>::operator++(int)
{
	std::shared_ptr<ImageIteratorImplementation<T> >& clone = createCopy();
	++(*this);
	return clone;
}

template <typename T> ImageIteratorImplementation<T>& ContiguousMemoryIterator<T>::operator--()
{
	--address;
	return *this;
}

template <typename T> std::shared_ptr<ImageIteratorImplementation<T> > ContiguousMemoryIterator<T>::operator--(int)
{
	std::shared_ptr<ImageIteratorImplementation<T> >& clone = createCopy();
	--(*this);
	return clone;
}

template <typename T> T& ContiguousMemoryIterator<T>::operator* () const
{
	return *address;
}


/// <summary> Return a pointer to the data this iterator currently references </summary>
template <typename T> T* ContiguousMemoryIterator<T>::toPointer() const
{
	return address;
}


/// <summary> Returns a copy of this iterator </summary>
/// <returns>  Copy of this iterator, stored in a smart pointer </returns>
template <typename T> std::shared_ptr<ImageIteratorImplementation<T> > ContiguousMemoryIterator<T>::createCopy()
{
	std::shared_ptr<ImageIteratorImplementation<T> > clone(new ContiguousMemoryIterator<T>(*this));
	return clone;
}
