#include "stdafx.h"
#include "Image.h"


Image::Image()
{
	x = 0;
	y = 0;
	z = 0;
	c = 0;
	m = 0;
	t = 0;
	bufferType = BufferType::INTENSITY;
}

Image::Image(Dimensions d,wstring memMapPath)
:pMem(new ImageMemory())
{
	x = d.x;
	y = d.y;
	z = d.z;
	c = d.c;
	m = d.m;
	t = d.t;
	dataType = d.dType;
	bufferType = d.imageBufferType;
	if(FALSE == pMem->AllocateMem(d.mType,d.dType,d.x, d.y, d.c,d.z,d.m,d.t,d.imageBufferType,memMapPath))
	{
		exception e("Unable to allocate image memory");
		throw e;
	}
}

Image::Image(Dimensions d,wstring memMapPath,wstring tmpFName)
:pMem(new ImageMemory())
{
	x = d.x;
	y = d.y;
	z = d.z;
	c = d.c;
	m = d.m;
	t = d.t;
	dataType = d.dType;
	bufferType = d.imageBufferType;
	if(FALSE == pMem->setTempFileName(tmpFName))
	{
		exception e("Unable to set temp file name");
		throw e;
	}
	if(FALSE == pMem->AllocateMem(d.mType,d.dType,d.x, d.y, d.c,d.z,d.m,d.t,d.imageBufferType,memMapPath))
	{
		exception e("Unable to allocate image memory");
		throw e;
	}	
}

Image::~Image()
{
}

Image::Image(Image& imageIn)
{
	Dimensions d;
	imageIn.GetImageDimensions(d);
	x = d.x;
	y = d.y;
	c = d.c;
	m = d.m;
	z = d.z;
	t = d.t;
	dataType = d.dType;
	bufferType = d.imageBufferType;
	pMem->AllocateMem(d.mType,d.dType,d.x, d.y, d.c,d.z,d.m,d.t,d.imageBufferType,NULL);

	//TODO COPY THE DATA BUFFER BETWEEN IMAGES
}

long Image::GetImageDimensions(Dimensions &d)
{
	d.x = x;
	d.y = y;
	d.c = c;
	d.m = m;
	d.z = z;
	d.t = t;
	d.dType = dataType;

	d.mType = pMem->GetMemoryType();
	d.imageBufferType = bufferType;
	return TRUE;
}

char * Image::GetDataPtr(long c,long m,long z,long t)
{
	return pMem->GetMemPtr(c,m,z,t);
}

char * Image::GetDataPtr(long c,long m,long z,long t, UINT64 offset)
{
	return pMem->GetMemPtr(c,m,z,t,offset);
}

void Image::UnlockDataPtr(long c,long m,long z,long t)
{
	pMem->UnlockMemPtr(c,m,z,t);
}

wstring Image::GetImagePath()
{
	return pMem->GetImageMemoryPath();
}

DummyOp::DummyOp()
{
}

DummyOp::~DummyOp()
{
}

long DummyOp::Execute(IImage *image)
{
	Dimensions d;

	if(FALSE == image->GetImageDimensions(d))
	{
		return FALSE;
	}

	char * ptr = image->GetDataPtr(0,0,0,0);

	if(NULL == ptr)
	{
		return FALSE;
	}

	return TRUE;
}