#pragma once

#include "ImageIteratorImplementation.h"


/// <summary> Used to iterate over a set of memory where all channels of the image are interlaced on a row by row basis. </summary>
/// <remarks> 
/// The InterlacedMemoryIterator uses the dimensions of an image to skip around in the memory buffer. This makes sure it
/// still follows the expected x,y,c,m,z iteration order, even when the memory is aranged in an x,c,y,m,z arrangment.
/// </remarks>
template <typename T> class InterlacedMemoryIterator : public ImageIteratorImplementation<T>
{

public:
	InterlacedMemoryIterator(T* startingAddress, int width, int height, int channels, int mosiacs, int depth, int startingX=0, int startingY=0, int startingChannel=0);

	virtual ImageIteratorImplementation<T>& operator++();
	virtual std::shared_ptr<ImageIteratorImplementation<T> > operator++(int);
	virtual ImageIteratorImplementation<T>& operator--();
	virtual std::shared_ptr<ImageIteratorImplementation<T> > operator--(int);
	virtual T& operator* () const;
	virtual T* toPointer() const;
	virtual std::shared_ptr<ImageIteratorImplementation<T> > createCopy();
	virtual void advanceToEndOfChannel();


private:
	T* address;
	T* startingAddress;

	int width;
	int height;
	int channels;
	int mosiacs;
	int depth;


	int currentX, currentY, currentC;
	int nextLineIncrement, nextChannelIncrement, nextMosiacIncrement;


};

/// <summary> Constructs Interlaced Memory Iterator </summary>
template <typename T> InterlacedMemoryIterator<T>::InterlacedMemoryIterator(T* startingAddress, int width, int height, int channels, int mosiacs, int depth, int startingX, int startingY, int startingChannel)
{
	address = this->startingAddress = startingAddress;
	this->width = width;
	this->height = height;
	this->channels = channels;
	this->mosiacs = mosiacs;
	this->depth = depth;

	currentX=startingX;
	currentY=startingY;
	currentC=startingChannel;

	nextLineIncrement = width*(channels-1);
	nextChannelIncrement = -width*channels*height + width;
	nextMosiacIncrement = -nextChannelIncrement-nextLineIncrement;

}


/// <summary> Skips the iterator to the end of the current channel. The end here means the last position that would fall under this
///           channel +1, the same idea as calling end() on a stl container </summary>
template <typename T> void InterlacedMemoryIterator<T>::advanceToEndOfChannel()
{
	address+=width-1-currentX;
	currentX=width-1;

	address+=(height-1-currentY)*width*channels;
	currentY=height-1;

	++(*this);
}


/// <summary> Increments this iterator. Keeps track of where it is and skips the necessary amount
///           when reaching the end row, column, or channel. </summary>
template <typename T> ImageIteratorImplementation<T>& InterlacedMemoryIterator<T>::operator++()
{
	++address;
	++currentX;
	if(currentX >= width)
	{
		currentX=0;
		address+=nextLineIncrement;
		++currentY;
	}
	if(currentY >= height)
	{
		currentY=0;
		address +=nextChannelIncrement;
		++currentC;
	}
	if(currentC >= channels)
	{
		currentC=0;
		address +=nextMosiacIncrement;
	}

	return *this;
}   

template <typename T> std::shared_ptr<ImageIteratorImplementation<T> > InterlacedMemoryIterator<T>::operator++(int)
{
	std::shared_ptr<ImageIteratorImplementation<T> >& clone = createCopy();
	++(*this);
	return clone;
}

/// <summary> Decrements this iterator. Keeps track of where it is and skips the necessary amount
///           when reaching the end row, column, or channel. </summary>
template <typename T> ImageIteratorImplementation<T>& InterlacedMemoryIterator<T>::operator--()
{

	--address;
	--currentX;
	if(currentX < 0)
	{
		currentX=width-1;
		address-=nextLineIncrement;
		--currentY;
	}
	if(currentY < 0)
	{
		currentY=height-1;
		address -=nextChannelIncrement;
		--currentC;
	}
	if(currentC < 0)
	{
		currentC=channels-1;
		address -=nextMosiacIncrement;
	}

	return *this;
}

template <typename T> std::shared_ptr<ImageIteratorImplementation<T> > InterlacedMemoryIterator<T>::operator--(int)
{
	std::shared_ptr<ImageIteratorImplementation<T> >& clone = createCopy();
	--(*this);
	return clone;
}

template <typename T> T& InterlacedMemoryIterator<T>::operator* () const
{
	return *address;
}


/// <summary> Return a pointer to the data this iterator currently references </summary>
template <typename T> T* InterlacedMemoryIterator<T>::toPointer() const
{
	return address;
}


/// <summary> Returns a copy of this iterator </summary>
/// <returns>  Copy of this iterator, stored in a smart pointer </returns>
template <typename T> std::shared_ptr<ImageIteratorImplementation<T> > InterlacedMemoryIterator<T>::createCopy()
{
	std::shared_ptr<ImageIteratorImplementation<T> > clone(new InterlacedMemoryIterator<T>(*this));
	return clone;
}
