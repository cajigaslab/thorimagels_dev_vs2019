#include "stdafx.h"
#include "RunSample.h"
#include "ImageCorrection.h"

auto_ptr<CorrectionImage> backgroundImage(new CorrectionImage());
auto_ptr<CorrectionImage> flatFieldImage(new CorrectionImage());

CorrectionImage::CorrectionImage()
	{
		_pBuffer = NULL;
		_width = 0;
		_height = 0;
		_colorChannels = 0;
		_meanIntensity = 0;
	}

long CorrectionImage::Create(wstring ws, long w, long h, long c)
	{
		if(_pBuffer != NULL)
		{
			delete _pBuffer;
		}

		_pBuffer = new unsigned short[w*h*c];
		_width = w;
		_height = h;
		_colorChannels = c;
		
		ReadImage((char*)ws.c_str(),(char*&)_pBuffer);

		unsigned short *pD = _pBuffer;

		unsigned long totalIntensity = 0;

			for(long y=0; y<_height; y++)
			{
				for(long x=0; x<_width; x++)
				{
					totalIntensity += *pD;
					pD++;
				}
			}

		_meanIntensity = totalIntensity/(double)(_width*_height);

		return TRUE;
	}

double CorrectionImage::GetMeanIntensity()
{
	return _meanIntensity;
}

long CorrectionImage::GetWidth()
{
	return _width;
}

long CorrectionImage::GetHeight()
{
	return _height;
}

long CorrectionImage::GetColorChannels()
{
	return _colorChannels;
}

unsigned short* CorrectionImage::GetBuffer()
{
	return _pBuffer;	
}


CorrectionImage::~CorrectionImage()
	{
		if(_pBuffer !=NULL)
		{
			delete _pBuffer;
		}
	}



long SetupImageCorrectionBuffers(IExperiment *pExp,long w, long h)
{
	long enablePincushion;
	double pinCoeff1;
	double pinCoeff2; 
	double pinCoeff3; 
	long enableBackgroundSubtraction; 
	string pathBackgroundSubtraction; 
	long enableFlatField; 
	string pathFlatField;

	pExp->GetImageCorrection(enablePincushion, pinCoeff1, pinCoeff2, pinCoeff3, enableBackgroundSubtraction, pathBackgroundSubtraction, enableFlatField, pathFlatField);

	if(enableBackgroundSubtraction)
	{
		wstring ws(pathBackgroundSubtraction.length(),L'\x0');

		std::copy(pathBackgroundSubtraction.begin(), pathBackgroundSubtraction.end(), ws.begin());

		long width,height,colorChannels;

		if(FALSE == ReadImageInfo((wchar_t*)ws.c_str(), width, height, colorChannels))
		{
			return FALSE;
		}

		if((w != width)||(h != height))
		{
			return FALSE;
		}

		backgroundImage->Create(ws,width,height,colorChannels);
	}
	
	if(enableFlatField)
	{		
		wstring ws(pathFlatField.length(),L'\x0');

		std::copy(pathFlatField.begin(), pathFlatField.end(), ws.begin());

		long width,height,colorChannels;

		if(FALSE == ReadImageInfo((wchar_t*)ws.c_str(), width, height, colorChannels))
		{
			return FALSE;
		}
	
		if((w != width)||(h != height))
		{
			return FALSE;
		}

		flatFieldImage->Create(ws,width,height,colorChannels);
	}

	return TRUE;
}

void Corrections(char * pBuffer, long width, long height, long channels, int enablePincushion, int enableBackgroundSubtraction, int enableFlatField, double k1,double k2, double k3, double k4)
{
	if(enablePincushion)
	{
		switch(channels)
		{
		case 1:	
			{
				PincushionCorrection(pBuffer , static_cast<long>(width), static_cast<long>(height), 0, k1, k2,k3,k4);
			}
			break;
		case 3:
			{
				PincushionCorrection(pBuffer , static_cast<long>(width), static_cast<long>(height), 0, k1, k2,k3,k4);
				PincushionCorrection(pBuffer + static_cast<long>(width*height*2) , static_cast<long>(width), static_cast<long>(height), 0, k1, k2,k3,k4);		
			}
			break;
		}
	}

	//combine the background subtraction and flat field if both are active
	if(enableBackgroundSubtraction && enableFlatField)
	{
		if((width == backgroundImage->GetWidth())&&
			(height == backgroundImage->GetHeight())&&
			(width == flatFieldImage->GetWidth())&&
			(height == flatFieldImage->GetHeight()))
		{			
			long offsetD;
			long offsetB;
			long offsetF;

			unsigned short * pD = (unsigned short*)pBuffer;
			unsigned short * pB = backgroundImage->GetBuffer();
			unsigned short * pF = flatFieldImage->GetBuffer();

			for(long c=0; c<channels; c++)
			{
				offsetD = width * height * c;
				//if the data is multi channel but the correction images are single channel reuse the first channel
				offsetB = width * height * min(backgroundImage->GetColorChannels()-1,c);
				offsetF = width * height * min(flatFieldImage->GetColorChannels()-1,c);

				pD = (unsigned short*)pBuffer + offsetD;
				pB = backgroundImage->GetBuffer() + offsetB;
				pF = flatFieldImage->GetBuffer() + offsetF;

				FlatFieldAndBackgroundCorrection(pD,pB,pF, flatFieldImage->GetMeanIntensity(), width, height, channels);
			}
		}

	}
	else if (enableBackgroundSubtraction)
	{
		if((width == backgroundImage->GetWidth())&&
			(height == backgroundImage->GetHeight()))
		{			
			long offsetD;
			long offsetB;

			unsigned short * pD = (unsigned short*)pBuffer;
			unsigned short * pB = backgroundImage->GetBuffer();

			for(long c=0; c<channels; c++)
			{
				offsetD = width * height * c;
				//if the data is multi channel but the correction images are single channel reuse the first channel
				offsetB = width * height * min(backgroundImage->GetColorChannels()-1,c);

				pD = (unsigned short*)pBuffer + offsetD;
				pB = backgroundImage->GetBuffer() + offsetB;
				BackgroundCorrection(pD,pB,width, height, channels);
			}
		}
	}
	else if(enableFlatField)
	{
		if(	(width == flatFieldImage->GetWidth())&&
			(height == flatFieldImage->GetHeight()))
		{			
			long offsetD;
			long offsetF;

			unsigned short * pD = (unsigned short*)pBuffer;
			unsigned short * pF = flatFieldImage->GetBuffer();

			for(long c=0; c<channels; c++)
			{
				offsetD = width * height * c;
				//if the data is multi channel but the correction images are single channel reuse the first channel
				offsetF = width * height * min(flatFieldImage->GetColorChannels()-1,c);

				pD = (unsigned short*)pBuffer + offsetD;
				pF = flatFieldImage->GetBuffer() + offsetF;
				FlatFieldCorrection(pD,pF, flatFieldImage->GetMeanIntensity(), width, height, channels);
			}
		}
	}

}


void ImageCorrections(IExperiment * pExp, char* pBuffer, long width, long height, long channels)
{	
	long enablePincushion;
	double pinCoeff1;
	double pinCoeff2; 
	double pinCoeff3; 
	long enableBackgroundSubtraction; 
	string pathBackgroundSubtraction; 
	long enableFlatField; 
	string pathFlatField;

	pExp->GetImageCorrection(enablePincushion, pinCoeff1, pinCoeff2, pinCoeff3, enableBackgroundSubtraction, pathBackgroundSubtraction, enableFlatField, pathFlatField);


	Corrections(pBuffer,  width,  height,  channels,  enablePincushion,  enableBackgroundSubtraction,  enableFlatField,  pinCoeff1, pinCoeff2,  pinCoeff3,  1.0);
}


