#pragma once

#include <iterator>
#include <memory>

/// <summary> The underlying implementation of an image iterator, specifying the bare minimum for an iterator.
///           Can be used to change the underlying behavior of a ImageIterator while perserving a singular interface. </summary>
template <typename T> class ImageIteratorImplementation : public std::iterator<std::bidirectional_iterator_tag, T> 
{

public:


	virtual bool operator==(ImageIteratorImplementation<T> const& other) const;
	virtual bool operator!=(ImageIteratorImplementation<T> const& other) const;
	virtual ImageIteratorImplementation<T>& operator++()=0;
	virtual std::shared_ptr<ImageIteratorImplementation<T> > operator++(int)=0;
	virtual ImageIteratorImplementation<T>& operator--()=0;
	virtual std::shared_ptr<ImageIteratorImplementation<T> > operator--(int)=0;
	virtual T& operator* () const=0;
	virtual T* toPointer() const=0;
	virtual std::shared_ptr<ImageIteratorImplementation<T> > createCopy()=0;


};


template <typename T> bool ImageIteratorImplementation<T>::operator==(ImageIteratorImplementation const& other) const
{
	return toPointer() == other.toPointer();
}

template <typename T> bool ImageIteratorImplementation<T>::operator!=(ImageIteratorImplementation const& other) const
{
	return toPointer() != other.toPointer();
}
