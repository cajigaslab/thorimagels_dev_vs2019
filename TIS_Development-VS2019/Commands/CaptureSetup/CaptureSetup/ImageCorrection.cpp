#include "stdafx.h"
#include "ippi.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"

extern char * pMemoryBuffer;
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


long CaptureSetup::SetPincushionCoefficients(double k1,double k2,double k3,double k4)
{
	_coefficientK1 = k1;
	_coefficientK2 = k2;
	_coefficientK3 = k3;

	_coefficientK4 = 1 - (k1 + k2 + k3);

	return TRUE;
}

long CaptureSetup::GetPincushionCoefficients(double &k1,double &k2,double &k3,double &k4)
{
	k1=_coefficientK1;
	k2=_coefficientK2;
	k3=_coefficientK3;
	k4=_coefficientK4;

	return TRUE;
}

long CaptureSetup::SetImageCorrectionEnable(int enable)
{
	_enablePincushionCorrection = enable;

	return TRUE;
}

DllExportLiveImage SetPincushionCoefficients(double k1, double k2,double k3, double k4)
{
	return CaptureSetup::getInstance()->SetPincushionCoefficients( k1, k2, k3, k4);
}

DllExportLiveImage SetBackgroundSubtractionEnable(int enable)
{
	CaptureSetup::getInstance()->_enableBackgroundSubtraction = enable;

	return TRUE;
}


DllExportLiveImage SetBackgroundSubtractionFile(char *selectedFileName)
{

	wstring ws((wchar_t*)selectedFileName);

	long width,height,colorChannels, bitsPerChannel;

	if(FALSE == ReadImageInfo((wchar_t*)ws.c_str(), width, height, colorChannels, bitsPerChannel))
	{
		return FALSE;
	}

	backgroundImage->Create(ws,width,height,colorChannels);

	return TRUE;
}

DllExportLiveImage SetFlatFieldFile(char *selectedFileName)
{
	wstring ws((wchar_t*)selectedFileName);

	long width,height,colorChannels, bitsPerChannel;

	if(FALSE == ReadImageInfo((wchar_t*)ws.c_str(), width, height, colorChannels, bitsPerChannel))
	{
		return FALSE;
	}

	flatFieldImage->Create(ws,width,height,colorChannels);

	return TRUE;
}

DllExportLiveImage SetFlatFieldEnable(int enable)
{
	CaptureSetup::getInstance()->_enableFlatField = enable;

	return TRUE;
}

DllExportLiveImage SetImageCorrectionEnable(int enable)
{
	CaptureSetup::getInstance()->_enablePincushionCorrection = enable;

	return TRUE;
}

void ImageCorrections(char * pBuffer, long width, long height, long channels, int enablePincushion, int enableBackgroundSubtraction, int enableFlatField, double k1,double k2, double k3, double k4)
{
	if(enablePincushion)
	{
		for(long i=0; i<channels; i++)
		{
				PincushionCorrection(pBuffer + static_cast<long>(width*height*i) , static_cast<long>(width), static_cast<long>(height), 0, k1, k2,k3,k4);				
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
//#pragma omp parallel for
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
//#pragma omp parallel for
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
//#pragma omp parallel for
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

void ResizeImage(char * pSource, long srcWidth, long srcHeight, char * pDestination, long dstWidth, long dstHeight)
{
	IppiSize size  = {srcWidth, srcHeight}; 
	IppiRect srect = {0, 0, srcWidth, srcHeight}; 
	IppiRect drect = {0, 0, dstWidth, dstHeight};

	double Xscale = static_cast<double>(dstWidth) / static_cast<double>(srcWidth);
	double Yscale = static_cast<double>(dstHeight) / static_cast<double>(srcHeight); 

	int nChannel  = 1;	

	int INTERPOLATIONMODE = IPPI_INTER_CUBIC2P_CATMULLROM;


	 /* interploation options:
09	       IPPI_INTER_NN||IPPI_INTER_LINEAR|| IPPI_INTER_CUBIC
10	       IPPI_INTER_CUBIC2P_BSPLINE||IPPI_INTER_CUBIC2P_CATMULLROM||PPI_INTER_CUBIC2P_B05C03
11	       IPPI_INTER_SUPER||IPPI_INTER_LANCZOS */
	char * buf;
	int bufSize;

	// calculation of work buffer size 
	ippiResizeGetBufSize( srect, drect, nChannel, INTERPOLATIONMODE, &bufSize );
	//ippiResizeGetBufSize( srect, drect, 1, IPPI_INTER_CUBIC2P_CATMULLROM, &bufsize );

	// memory allocate 
 	buf = (char*)malloc( bufSize );

	IppStatus status = ippStsErr;

	// function call 
	if( NULL != buf )
		status = ippiResizeSqrPixel_16u_C1R((Ipp16u*)pSource, size, srcWidth*sizeof(Ipp16u), srect, (Ipp16u*)pDestination, dstWidth*sizeof(Ipp16u), drect, Xscale, Yscale, 0, 0, INTERPOLATIONMODE, (Ipp8u*)buf );
    
}