#pragma once

#include "ImageIteratorImplementation.h"
#include <iostream>


/// <summary>
/// Provides a class used to iterator over an image. The order of iteration is x,y,c,m,z.
/// <remarks>
/// The ImageIterator delegates its functionality to ImageIteratorImplementation subclasses, allowing it to support any
/// underlying ImageMemoryModel while still presenting a unified interface to other classes. </remarks>
template <typename T> class ImageIterator : public std::iterator<std::bidirectional_iterator_tag, T> 
{

public:
	ImageIterator(std::shared_ptr<ImageIteratorImplementation<T>> implementationIterator);
	~ImageIterator();

	bool operator==(ImageIterator<T> const& other) const;
	bool operator!=(ImageIterator<T> const& other) const;
	ImageIterator<T>& operator++();
	ImageIterator<T> operator++(int);
	ImageIterator<T>& operator--();
	ImageIterator<T> operator--(int);
	T& operator* () const;
	ImageIterator<T>(const ImageIterator<T>& copyFrom);
	ImageIterator<T>& operator = (const ImageIterator<T>& assignFrom);


private:
	std::shared_ptr<ImageIteratorImplementation<T>> implementationIterator;

};

template <typename T> ImageIterator<T>::~ImageIterator()
{
	//std::cout << "Destruction" << std::endl;
}

template <typename T> ImageIterator<T>::ImageIterator(std::shared_ptr<ImageIteratorImplementation<T>> implementationIterator)
{
	this->implementationIterator = implementationIterator;
}

template <typename T> bool ImageIterator<T>::operator==(ImageIterator<T> const& other) const
{
	return  *implementationIterator == *(other.implementationIterator);
}

template <typename T> bool ImageIterator<T>::operator!=(ImageIterator<T> const& other) const
{
	return  *implementationIterator != (*other.implementationIterator);
}


/// <summary> Prefix incrementor. Increments this iterator to the next object </summary>
/// <returns> Reference to this iterator after incrementation </returns>
template <typename T> ImageIterator<T>& ImageIterator<T>::operator++()
{
	static int count=0;
	++count;
	++(*implementationIterator);
	return *this;
}   


/// <summary> Postfix incrementor. Increments this iterator to the next object, but
///           returns an unincremented copy of itself </summary>
/// <returns>  An unincremented copy of itself </returns>
template <typename T> ImageIterator<T> ImageIterator<T>::operator++(int)
{
	ImageIterator<T> copy(*this);
	++(*implementationIterator);
	return copy;
}


/// <summary> Prefix decrementor. Decrements this iterator to the previous object </summary>
/// <returns> Reference to this iterator after decrementation </returns>
template <typename T> ImageIterator<T>& ImageIterator<T>::operator--()
{
	--(*implementationIterator);
	return *this;
}


/// <summary> Postfix decrementor. Decrements this iterator to the previoius object, but
///           returns an undecremented copy of itself </summary>
/// <returns>  An undecremented copy of itself </returns>
template <typename T> ImageIterator<T> ImageIterator<T>::operator--(int)
{
	ImageIterator<T> copy(*this);
	--(*implementationIterator);
	return copy;
}


/// <summary> Get a reference to the underlying object </summary>
/// <returns>  Reference to object this incrementor currently points to </returns>
template <typename T> T& ImageIterator<T>::operator* () const
{
	return **implementationIterator;
}


/// <summary> Copy constructor </summary>
/// <param name="copyFrom"> Create a new SmartFileMapping object from the current one, sharing the same underlying file map</param>
template <typename T> ImageIterator<T>::ImageIterator(const ImageIterator<T>& copyFrom)
{
	implementationIterator = copyFrom.implementationIterator->createCopy();
}


/// <summary> Assignent operator </summary>
/// <param name="assignFrom"> Assign to this object from the input object</param>
template <typename T> ImageIterator<T>& ImageIterator<T>::operator=(const ImageIterator<T>& assignFrom) 
{

	//Dont assign to oneself
	if(this != &assignFrom)
	{
		//Not the same file
		if(implementationIterator != assignFrom.implementationIterator)
		{

			//Create Copy Of Assignment
			ImageIterator<T> copy(assignFrom);

			//Assign to this
			implementationIterator = copy.implementationIterator;

		}
	}

	return *this;
}