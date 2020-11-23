// HologramGenerator.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "HologramGenerator.h"
#include "HologramGeneratorlib.h"
#include <cmath>
#include <complex>


using namespace std;

auto_ptr<IPPCVDll> ippcvDll(new IPPCVDll(L".\\.\\ippcvu8-7.0.dll"));
auto_ptr<IPPCOREDll> ippcoreDll(new IPPCOREDll(L".\\.\\ippcore-7.0.dll"));
auto_ptr<IPPIDll> ippiDll(new IPPIDll(L".\\.\\ippiu8-7.0.dll"));
auto_ptr<IPPSDll> ippsDll(new IPPSDll(L".\\.\\ipps-7.0.dll"));
auto_ptr<IPPMDll> ippmDll(new IPPMDll(L".\\.\\ippm-7.0.dll"));
auto_ptr<LogDll> logDll;

//static members:
bool HologramGen::instanceFlag = false;
std::auto_ptr<HologramGen> HologramGen::single;
wchar_t errMsg[MSG_LENGTH];
double _refractiveIndex=1.33;
long _slmXsize=512;
long _slmYsize=512;
double lambda=1.040;//1.030 for Nick Robinson
double _waveNumber=2*PI/lambda;
double _lensRadius=25.4;	//in mm
double _focalLength=200;	//in mm
double _NA=1;//_lensRadius/_focalLength;	//0.8 for Nick Robinson
double _alpha=asin(_NA/_refractiveIndex);

HologramGen::HologramGen()
{
	_mtrxWidth = _mtrxHeight = _mtrxLength = 0;
	_pixelUM = 0;
	_holoGenMethod = HoloGenAlg::GerchbergSaxton;
	_coeffs = NULL;
}

///singleton
HologramGen* HologramGen::getInstance()
{
	if(!instanceFlag)
	{
		try
		{
			single.reset(new HologramGen());
			instanceFlag = true;
		}
		catch(...)
		{
			throw;
		}
	}
	return single.get();
}

long HologramGen::NormalizePhase(float* img)
{

	Ipp32f minPhase, maxPhase;

	if (img!=NULL)
	{
		ippsDll->ippsMin_32f(img, _mtrxLength, &minPhase);
		ippsDll->ippsAddC_32f_I(abs(minPhase), img, _mtrxLength);
		ippsDll->ippsMax_32f(img,_mtrxLength,&maxPhase);
		ippsDll->ippsMulC_32f_I(MAX_PIXEL_VALUE/maxPhase,img,_mtrxLength);
		return TRUE;
	}


	return TRUE;
}

long HologramGen::Set3DParam(double na, double wavelength)
{
	_NA=na;
	_waveNumber=2*PI/wavelength;
	_alpha=asin(_NA/_refractiveIndex);
	return TRUE;
}


long HologramGen::SetSize(int width, int height, double pixelUM)
{
	if(((width%2) == 0) && ((height%2) == 0))
	{
		_mtrxWidth = width;
		_mtrxHeight = height;
		_mtrxLength = _mtrxWidth*_mtrxHeight;
		_pixelUM = pixelUM;
		return TRUE;
	}
	return FALSE;
}

long HologramGen::SetAlgorithm(int algorithmID)
{
	if(LAST_ALGORITHM > (HoloGenAlg)algorithmID)
	{
		_holoGenMethod = (HoloGenAlg)algorithmID;
		return TRUE;
	}
	return FALSE;
}

// combine two holograms to first, from center parts of each: [1 | 2], left from 1 and right from 2
long HologramGen::CombineHologramFiles(const wchar_t * pathAndFilename1, const wchar_t * pathAndFilename2)
{
	std::wstring fname[2] = {std::wstring(pathAndFilename1), std::wstring(pathAndFilename2)};

	//test load of bmp and resize
	int imgWidth[2] = {0}, imgHeight[2] = {0};
	long size; BITMAPINFO bmi;
	unsigned char* imgRead[2] = {NULL};
	for (int i = 0; i < 2; i++)
	{
		imgRead[i] = LoadBMP(&imgWidth[i], &imgHeight[i], &size, &bmi.bmiHeader, fname[i].c_str());
	}
	if (NULL == imgRead[0] || NULL == imgRead[1] || imgWidth[0] != imgWidth[1] || imgHeight[0] != imgHeight[1])
	{
		for (int i = 0; i < 2; i++)
		{
			if (NULL != imgRead[i])
				delete[] imgRead[i];
		}
		return FALSE;
	}

	long newSize;
	unsigned char* imgRGB[2] = {NULL};
	for (int i = 0; i < 2; i++)
	{
		imgRGB[i] = ConvertBGRToRGBBuffer(imgRead[i], bmi.bmiHeader, &newSize);
	}

	int channelCnt = static_cast<int>(bmi.bmiHeader.biBitCount/CHAR_BIT);
	BYTE* pSrc = imgRGB[1];
	BYTE* pDst = imgRGB[0];
	size_t dLength = channelCnt*bmi.bmiHeader.biWidth * sizeof(BYTE);
	for (int j = 0; j < bmi.bmiHeader.biHeight; j++)
	{
		memcpy_s(pDst, static_cast<int>(dLength/2), (pDst+static_cast<int>(dLength/4)), static_cast<int>(dLength/2));
		memcpy_s((pDst+dLength/2), static_cast<int>(dLength/2), (pSrc+static_cast<int>(dLength/4)), static_cast<int>(dLength/2));
		pDst += dLength/sizeof(BYTE);
		pSrc += dLength/sizeof(BYTE);
	}

	//save on the first file name
	unsigned char* imgBGR = ConvertRGBToBGRBuffer(imgRGB[0], bmi.bmiHeader, &size);
	SaveBMP(imgBGR, imgWidth[0], imgHeight[0], size, fname[0].c_str());
	delete[] imgBGR;
	for (int i = 0; i < 2; i++)
	{
		delete[] imgRGB[i];
		delete[] imgRead[i];
	}
	return TRUE;
}

// fitting transformation with preset coefficients by SetCoeffs
long HologramGen::FittingTransform(float* pImgDst)
{
	switch ((GeoFittingAlg)_fittingMethod)
	{
	case GeoFittingAlg::AFFINE:
		return AffineTransform(pImgDst);
	case GeoFittingAlg::PROJECTIVE:
		return ProjectTransform(pImgDst);
	default:
		break;
	}
	return FALSE;
}

// calculate fitting coefficients out of source(x1,y1,x2,y2...) to target(x1,y1,x2,y2...): Coeffs = (Src^-1)*Tgt
long HologramGen::CalculateCoeffs(const float* pSrcPoints, const float* pTgtPoints, long size, long fittingAlg, double* coeffs)
{
	_fittingMethod = (GeoFittingAlg)fittingAlg;

	switch (_fittingMethod)
	{
	case GeoFittingAlg::AFFINE:
		return CalculateAffineCoeffs(pSrcPoints, pTgtPoints, size, coeffs);
	case GeoFittingAlg::PROJECTIVE:
		return CalculateProjectCoeffs(pSrcPoints, pTgtPoints, size, coeffs);
	default:
		break;
	}
	return FALSE;
}

double* HologramGen::CalculateZernikeCoeffs(double n,double k,double z, double alpha)
{
	double* zernCoeff;
	zernCoeff=(double*)malloc(3*sizeof(double));

	zernCoeff[0]=n*k*z*pow(sin(alpha),2)/8/PI/sqrt(3)*(1+1/4*pow(sin(alpha),2)+9/80*(pow(sin(alpha),4))+1/16*(pow(sin(alpha),6)));
	zernCoeff[1]=n*k*z*(pow(sin(alpha),4))/96/PI/sqrt(5)*(1+3/4*(pow(sin(alpha),2))+15/18*(pow(sin(alpha),4)));
	zernCoeff[2]=n*k*z*(pow(sin(alpha),6))/640/PI/sqrt(7)*(1+5/4*(pow(sin(alpha),2)));

	return zernCoeff;
}

double* HologramGen::CalculateZernikePoly(double u, double v)
{
	double* zernPoly;
	zernPoly=(double*)malloc(3*sizeof(double));

	zernPoly[0]=sqrt(3)*(2*(pow(u,2)+pow(v,2))-1);
	zernPoly[1]=sqrt(5)*(6*pow(pow(u,2)+pow(v,2),2)-6*(pow(u,2)+pow(v,2))+1);
	zernPoly[2]=sqrt(7)*(20*pow(pow(u,2)+pow(v,2),3)-30*pow(pow(u,2)+pow(v,2),2)+12*(pow(u,2)+pow(v,2)-1));

	return zernPoly;
}


// set coefficients in fitting transformation
long HologramGen::SetCoeffs(long algorithm, double* affCoeffs)
{	
	switch ((GeoFittingAlg)algorithm)
	{
	case GeoFittingAlg::AFFINE:
		_fittingMethod = GeoFittingAlg::AFFINE;

		_coeffs = (double*)realloc(_coeffs, AFFINE_COEFF_CNT*sizeof(double));
		if((NULL != _coeffs) && (NULL != affCoeffs))
		{
			memcpy_s(_coeffs,AFFINE_COEFF_CNT*sizeof(double),affCoeffs,AFFINE_COEFF_CNT*sizeof(double));
		}
		break;
	case GeoFittingAlg::PROJECTIVE:
		_fittingMethod = GeoFittingAlg::PROJECTIVE;

		_coeffs = (double*)realloc(_coeffs, PROJECT_COEFF_CNT*sizeof(double));
		if((NULL != _coeffs) && (NULL != affCoeffs))
		{
			memcpy_s(_coeffs,PROJECT_COEFF_CNT*sizeof(double),affCoeffs,PROJECT_COEFF_CNT*sizeof(double));
		}
		break;
	default:
		break;
	}

	return TRUE;
}

//generate hologram with weight by distance from center, affine Transformaton
long HologramGen::GenerateHologram(float* pImgDst, int iteCount, float z)
{
	long ret = TRUE;

	Ipp32f* pPhase = ippsDll->ippsMalloc_32f(_mtrxLength);
	ippsDll->ippsZero_32f(pPhase,_mtrxLength);

	//linear weight by distance from center:
	WeightByDistance(pImgDst);

	//generate phase:
	switch (_holoGenMethod)
	{
	case HoloGenAlg::GerchbergSaxton:
		ret = PhaseGenByGS(pImgDst, pPhase, iteCount);
		break;
	case HoloGenAlg::CompressiveSensing:
		ret = PhaseGenBy3DGS(pImgDst, pPhase, iteCount,z);
		break;

	default:
		ret = FALSE;
		break;
	}

	ippsDll->ippsCopy_32f(pPhase,pImgDst,_mtrxLength);

	//clear:
	ippsDll->ippsFree(pPhase);

	return ret;
}

//rotate image with supplied angle, (+) in clockwise
long HologramGen::RotateForAngle(float* pImgDst, double angle)
{
	long ret = TRUE;
	int stepBytes;
	long xCenter = _mtrxWidth/2;
	long yCenter = _mtrxHeight/2;
	double angleRad = angle * PI / HALF_CIRCLE;
	Ipp32f* pBuf = ippiDll->ippiMalloc_32f_C1(_mtrxWidth, _mtrxHeight, &stepBytes);

	float* pSrc = pImgDst;
	float* pDst = pBuf;
	for (int i = 0; i < _mtrxWidth; i++)
	{
		for (int j = 0; j < _mtrxHeight; j++)
		{
			if(0 < *pSrc)
			{
				long x = i - xCenter;
				long y = j - yCenter;
				long xDst =  max(0, min(_mtrxWidth, static_cast<long>(floor((x*cos(angleRad)) + (y*sin(angleRad)) + xCenter + 0.5))));
				long yDst =  max(0, min(_mtrxHeight, static_cast<long>(floor((y*cos(angleRad)) - (x*sin(angleRad)) + yCenter + 0.5))));
				*(pDst + xDst*_mtrxWidth + yDst) = *pSrc;
			}
			pSrc++;
		}
	}
	memcpy_s(pImgDst, _mtrxLength*sizeof(float), pBuf, _mtrxLength*sizeof(float));
	ippiDll->ippiFree(pBuf);
	return ret;
}

//vertical flip image
long HologramGen::VerticalFlip(float* pImgDst)
{
	long ret = TRUE;

	int stepBytes;
	IppiSize ippSize = {_mtrxWidth ,_mtrxHeight};

	Ipp32f* pBuf = ippiDll->ippiMalloc_32f_C1(ippSize.width, ippSize.height, &stepBytes);
	if(ippStsNoErr == ippiDll->ippiMirror_32f_C1R(pImgDst, stepBytes, pBuf, stepBytes, ippSize, ippAxsHorizontal))
	{
		memcpy_s(pImgDst, _mtrxLength*sizeof(float), pBuf, _mtrxLength*sizeof(float));
	}
	else
	{
		StringCbPrintfW(errMsg,MSG_LENGTH,L"HologramGenerator VerticalFlip failed");
		LogMessage(ERROR_EVENT);
		ret = FALSE;
	}
	ippiDll->ippiFree(pBuf);
	return ret;
}

//scale image by factors
long HologramGen::ScaleByFactor(float* pImgDst, double scaleX, double scaleY)
{
	long ret = TRUE;
	int stepBytes;

	long xCenter = _mtrxWidth/2;
	long yCenter = _mtrxHeight/2;
	Ipp32f* pBuf = ippiDll->ippiMalloc_32f_C1(_mtrxWidth, _mtrxHeight, &stepBytes);

	float* pSrc = pImgDst;
	float* pDst = pBuf;
	for (int i = 0; i < _mtrxWidth; i++)
	{
		for (int j = 0; j < _mtrxHeight; j++)
		{
			if(0 < *pSrc)
			{
				long x = i - xCenter;
				long y = j - yCenter;
				long xDst = max(0, min(_mtrxWidth, static_cast<long>(floor((x*scaleX) + xCenter + 0.5))));
				long yDst = max(0, min(_mtrxHeight, static_cast<long>(floor((y*scaleY) + yCenter + 0.5))));
				*(pDst + xDst*_mtrxWidth + yDst) = *pSrc;
			}
			pSrc++;
		}
	}
	memcpy_s(pImgDst, _mtrxLength*sizeof(float), pBuf, _mtrxLength*sizeof(float));
	ippiDll->ippiFree(pBuf);
	return ret;
}

//offset image by pixels, +offsetX: right, +offsetY: up
long HologramGen::OffsetByPixels(float* pImgDst, long offsetX, long offsetY)
{
	long ret = TRUE;
	int stepBytes;
	Ipp32f* pBuf = ippiDll->ippiMalloc_32f_C1(_mtrxWidth, _mtrxHeight, &stepBytes);

	float* pSrc = pImgDst;
	float* pDst = pBuf;
	for (int i = 0; i < _mtrxWidth; i++)
	{
		for (int j = 0; j < _mtrxHeight; j++)
		{
			if(0 < *pSrc)
			{
				long x = i - offsetY;
				long y = j + offsetX;
				long xDst = max(0, min(_mtrxWidth, static_cast<long>(x)));
				long yDst = max(0, min(_mtrxHeight, static_cast<long>(y)));
				*(pDst + xDst*_mtrxWidth + yDst) = *pSrc;
			}
			pSrc++;
		}
	}
	memcpy_s(pImgDst, _mtrxLength*sizeof(float), pBuf, _mtrxLength*sizeof(float));
	ippiDll->ippiFree(pBuf);
	return ret;
}

///***	private functions	***///

//affine transformation with preset affine coefficients: {x = C00*x'+C01*y'+C02, y = C10*x'+C11*y'+C12}
long HologramGen::AffineTransform(float* pImgDst)
{
	long ret = TRUE;

	if((NULL == _coeffs))
		return FALSE;

	int stepBytes;
	Ipp32f* pAff = ippiDll->ippiMalloc_32f_C1(_mtrxWidth, _mtrxHeight, &stepBytes);
	ippsDll->ippsSet_32f(0.0,pAff,_mtrxLength);

	float* pSrc = pImgDst;
	float* pDst = pAff;
	for (int i = 0; i < _mtrxHeight; i++)		//i -> y coordinate
	{
		for (int j = 0; j < _mtrxWidth; j++)	//j -> x coordinate
		{
			if(0 < *pSrc)
			{
				//vertical opposite origins between image and affine (_mtrxHeight-i):
				long x = static_cast<long>(floor(_coeffs[0]*j+_coeffs[1]*i+_coeffs[2]+0.5));
				long y = static_cast<long>(floor(_coeffs[3]*j+_coeffs[4]*i+_coeffs[5]+0.5));
				long xDst = max(0, min(_mtrxHeight, x));
				long yDst = max(0, min(_mtrxWidth, y));
				*(pDst + xDst*_mtrxHeight + yDst) = *pSrc;
			}
			pSrc++;
		}
	}
	memcpy_s(pImgDst, _mtrxLength*sizeof(float), pAff, _mtrxLength*sizeof(float));
	ippiDll->ippiFree(pAff);
	return ret;
}

//projective transformation with preset coefficients: {kx = C00*x'+C01*y'+C02, ky = C10*x'+C11*y'+C12, k = C20*x'+C21*y'+1 }
long HologramGen::ProjectTransform(float* pImgDst)
{
	long ret = TRUE;

	if((NULL == _coeffs))
		return FALSE;

	int stepBytes;
	Ipp32f* pAff = ippiDll->ippiMalloc_32f_C1(_mtrxWidth, _mtrxHeight, &stepBytes);
	ippsDll->ippsSet_32f(0.0,pAff,_mtrxLength);

	float* pSrc = pImgDst;
	float* pDst = pAff;
	for (int i = 0; i < _mtrxHeight; i++)		//i -> y coordinate
	{
		for (int j = 0; j < _mtrxWidth; j++)	//j -> x coordinate
		{
			if(0 < *pSrc)
			{
				//vertical opposite origins between image and affine (_mtrxHeight-i):
				double k = _coeffs[6]*j+_coeffs[7]*(_mtrxHeight-i)+1;
				long x = static_cast<long>(floor(((_coeffs[0]*j+_coeffs[1]*(_mtrxHeight-i)+_coeffs[2])/k)+0.5));
				long y = static_cast<long>(floor(((_coeffs[3]*j+_coeffs[4]*(_mtrxHeight-i)+_coeffs[5])/k)+0.5));
				long xDst = max(0, min(_mtrxHeight, x));
				long yDst = max(0, min(_mtrxWidth, y));
				*(pDst + xDst*_mtrxHeight + yDst) = *pSrc;
			}
			pSrc++;
		}
	}
	memcpy_s(pImgDst, _mtrxLength*sizeof(float), pAff, _mtrxLength*sizeof(float));
	ippiDll->ippiFree(pAff);
	return ret;
}

//affine coefficients out of source(x1,y1,x2,y2...) to target(x1,y1,x2,y2...): Coeffs = (Src^-1)*Tgt
long HologramGen::CalculateAffineCoeffs(const float* pSrcPoints, const float* pTgtPoints, long size, double* affCoeffs)
{
	long ret = TRUE;

	//image origin is vertically opposite to matrix origin:
	Ipp32f* p1 = (Ipp32f*)(pSrcPoints+1);
	Ipp32f* p2 = (Ipp32f*)(pTgtPoints+1);
	for (int i = 0; i < static_cast<int>(size/2); i++)
	{
		*(p1) = _mtrxHeight - *(pSrcPoints+2*i+1);
		p1+=2;
		*(p2) = _mtrxHeight - *(pTgtPoints+2*i+1);
		p2+=2;
	}

	////offset origin to image center:	//From previous code.. maybe testing the offset
	//p1 = (Ipp32f*)(pSrcPoints);
	//p2 = (Ipp32f*)(pTgtPoints);
	//for (int i = 0; i < static_cast<int>(size/2); i++)
	//{
	//	*(p1+2*i) = *(p1+2*i) - (_mtrxWidth/2);
	//	*(p1+2*i+1) = *(p1+2*i+1) - (_mtrxHeight/2);
	//	*(p2+2*i) = *(p2+2*i) - (_mtrxWidth/2);
	//	*(p2+2*i+1) = *(p2+2*i+1) - (_mtrxHeight/2);
	//}

	int unitStride = sizeof(Ipp32f);
	int strideX = AFFINE_COEFF_CNT * sizeof(Ipp32f);
	int mtrxSize = size*AFFINE_COEFF_CNT;
	Ipp32f* pSrc = ippsDll->ippsMalloc_32f(mtrxSize);
	ippsDll->ippsZero_32f(pSrc,mtrxSize);
	Ipp32f* pDeComp = ippsDll->ippsMalloc_32f(mtrxSize);

	Ipp32f* pTgt = (Ipp32f*)pTgtPoints;
	Ipp32f* pBuf = ippsDll->ippsMalloc_32f(size);
	Ipp32f* pAff = ippsDll->ippsMalloc_32f(strideX);

	//prepare Src matrix:[x1 y1 1  0  0 0;
	//					  0  0  0 x1 y1 1;
	//					  x2 y2 1  0  0 0;
	//					  0  0  0 x2 y2 1;
	//								   ...]
	Ipp32f* pS = (Ipp32f*)pSrcPoints;
	Ipp32f* pD = (Ipp32f*)pSrc;
	for (int i = 0; i < static_cast<int>(size/2); i++)
	{
		*(pD) = *(pD + AFFINE_COEFF_CNT + 3) =*(pS);
		*(pD + 1) = *(pD + AFFINE_COEFF_CNT + 4) =*(pS + 1);
		*(pD + 2) = *(pD + AFFINE_COEFF_CNT + 5) =(Ipp32f)1.0;	
		pD += 2*AFFINE_COEFF_CNT;
		pS += 2;
	}

	//calculate coefficients:
	if(ippStsNoErr != ippmDll->ippmQRDecomp_m_32f(pSrc,strideX,unitStride,pBuf,pDeComp,strideX,unitStride,AFFINE_COEFF_CNT,size))
	{
		StringCbPrintfW(errMsg,MSG_LENGTH,L"HologramGenerator CalculateAffineCoeffs failed: ippmQRDecomp_m_32f");
		LogMessage(ERROR_EVENT);
		ret = FALSE;
	}

	if(ippStsNoErr != ippmDll->ippmQRBackSubst_mva_32f(pDeComp,strideX,unitStride,pBuf,pTgt,unitStride,unitStride,pAff,unitStride,unitStride,AFFINE_COEFF_CNT,size,1))
	{
		StringCbPrintfW(errMsg,MSG_LENGTH,L"HologramGenerator CalculateAffineCoeffs failed: ippmQRBackSubst_mva_32f");
		LogMessage(ERROR_EVENT);
		ret = FALSE;
	}

	//copy affine coefficients:
	if(TRUE == ret)
	{
		Ipp32f* pResl = pAff;
		double* pAffOut = affCoeffs;
		for (int i = 0; i < AFFINE_COEFF_CNT; i++)
		{
			*pAffOut = *pResl;
			pAffOut++;
			pResl++;
		}
	}
	ippsDll->ippsFree(pSrc);
	ippsDll->ippsFree(pDeComp);
	ippsDll->ippsFree(pBuf);
	ippsDll->ippsFree(pAff);
	return ret;
}

//projective coefficients out of source(x1,y1,x2,y2...) to target(x1,y1,x2,y2...): Coeffs = (Src^-1)*Tgt
long HologramGen::CalculateProjectCoeffs(const float* pSrcPoints, const float* pTgtPoints, long size, double* projCoeffs)
{
	long ret = TRUE;

	//image origin is vertically opposite to projective origin:
	Ipp32f* p1 = (Ipp32f*)(pSrcPoints+1);
	Ipp32f* p2 = (Ipp32f*)(pTgtPoints+1);
	for (int i = 0; i < static_cast<int>(size/2); i++)
	{
		*(p1) = _mtrxHeight - *(pSrcPoints+2*i+1);
		p1+=2;
		*(p2) = _mtrxHeight - *(pTgtPoints+2*i+1);
		p2+=2;
	}

	////offset origin to image center: //From previous code.. maybe testing the offset
	//p1 = (Ipp32f*)(pSrcPoints);
	//p2 = (Ipp32f*)(pTgtPoints);
	//for (int i = 0; i < static_cast<int>(size/2); i++)
	//{
	//	*(p1+2*i) = *(p1+2*i) - (_mtrxWidth/2);
	//	*(p1+2*i+1) = *(p1+2*i+1) - (_mtrxHeight/2);
	//	*(p2+2*i) = *(p2+2*i) - (_mtrxWidth/2);
	//	*(p2+2*i+1) = *(p2+2*i+1) - (_mtrxHeight/2);
	//}

	int unitStride = sizeof(Ipp32f);
	int strideX = PROJECT_COEFF_CNT * sizeof(Ipp32f);
	int mtrxSize = size*PROJECT_COEFF_CNT;
	Ipp32f* pSrc = ippsDll->ippsMalloc_32f(mtrxSize);
	ippsDll->ippsZero_32f(pSrc,mtrxSize);
	Ipp32f* pDeComp = ippsDll->ippsMalloc_32f(mtrxSize);

	Ipp32f* pTgt = (Ipp32f*)pTgtPoints;
	Ipp32f* pBuf = ippsDll->ippsMalloc_32f(size);
	Ipp32f* pAff = ippsDll->ippsMalloc_32f(strideX);

	//prepare Src matrix:[x1 y1 1  0  0 0 -u1x1 -u1y1;
	//					  0  0  0 x1 y1 1 -v1x1 -v1y1;
	//					  x2 y2 1  0  0 0 -u2x2 -u2y2;
	//					  0  0  0 x2 y2 1 -v2x2 -v2y2;
	//											   ...]
	Ipp32f* pS = (Ipp32f*)pSrcPoints;
	Ipp32f* pT = (Ipp32f*)pTgtPoints;
	Ipp32f* pD = (Ipp32f*)pSrc;
	for (int i = 0; i < static_cast<int>(size/2); i++)
	{
		*(pD) = *(pD + PROJECT_COEFF_CNT + 3) =*(pS);
		*(pD + 1) = *(pD + PROJECT_COEFF_CNT + 4) =*(pS + 1);
		*(pD + 2) = *(pD + PROJECT_COEFF_CNT + 5) =(Ipp32f)1.0;	
		*(pD + 6) = -*(pT) * *(pS);
		*(pD + PROJECT_COEFF_CNT + 6) = -*(pT+1) * *(pS);
		*(pD + 7) = -*(pT) * *(pS+1);
		*(pD + PROJECT_COEFF_CNT + 7) = -*(pT+1) * *(pS+1);
		pD += 2 * PROJECT_COEFF_CNT;
		pS += 2;
		pT += 2;
	}

	//calculate coefficients:
	if(ippStsNoErr != ippmDll->ippmQRDecomp_m_32f(pSrc,strideX,unitStride,pBuf,pDeComp,strideX,unitStride,PROJECT_COEFF_CNT,size))
	{
		StringCbPrintfW(errMsg,MSG_LENGTH,L"HologramGenerator CalculateAffineCoeffs failed: ippmQRDecomp_m_32f");
		LogMessage(ERROR_EVENT);
		ret = FALSE;
	}

	if(ippStsNoErr != ippmDll->ippmQRBackSubst_mva_32f(pDeComp,strideX,unitStride,pBuf,pTgt,unitStride,unitStride,pAff,unitStride,unitStride,PROJECT_COEFF_CNT,size,1))
	{
		StringCbPrintfW(errMsg,MSG_LENGTH,L"HologramGenerator CalculateAffineCoeffs failed: ippmQRBackSubst_mva_32f");
		LogMessage(ERROR_EVENT);
		ret = FALSE;
	}

	//copy affine coefficients:
	if(TRUE == ret)
	{
		Ipp32f* pResl = pAff;
		double* pAffOut = projCoeffs;
		for (int i = 0; i < PROJECT_COEFF_CNT; i++)
		{
			*pAffOut = *pResl;
			pAffOut++;
			pResl++;
		}
	}
	ippsDll->ippsFree(pSrc);
	ippsDll->ippsFree(pDeComp);
	ippsDll->ippsFree(pBuf);
	ippsDll->ippsFree(pAff);
	return ret;
}

//FFT with input magnitude and phase:
void HologramGen::FFT(float* pPolMagnDst, float* pPolPhaseDst, bool forward)
{
	IppStatus ippStatus;
	IppiSize ippSize = {_mtrxWidth, _mtrxHeight};
	IppiDFTSpec_C_32fc* spec;
	int stepBytes;

	//prepare mem:
	ippiDll->ippiDFTInitAlloc_C_32fc(&spec, ippSize, IPP_FFT_DIV_FWD_BY_N, ippAlgHintAccurate);
	Ipp32fc* pSrcC = ippiDll->ippiMalloc_32fc_C1(_mtrxWidth,_mtrxHeight,&stepBytes);
	Ipp32fc* pDstC = ippiDll->ippiMalloc_32fc_C1(_mtrxWidth,_mtrxHeight,&stepBytes);

	//polar to complex:
	ippStatus = ippsDll->ippsPolarToCart_32fc(pPolMagnDst, pPolPhaseDst, pSrcC, _mtrxLength);
	if(forward)
	{
		//Forward FFT:
		ippStatus = ippiDll->ippiDFTFwd_CToC_32fc_C1R(pSrcC,stepBytes,pDstC,stepBytes,spec,0);
	}
	else
	{
		//Inverse FFT:
		ippStatus = ippiDll->ippiDFTInv_CToC_32fc_C1R(pSrcC,stepBytes,pDstC,stepBytes,spec,0);
	}
	//complex to polar:
	ippStatus = ippsDll->ippsCartToPolar_32fc(pDstC, pPolMagnDst, pPolPhaseDst, _mtrxLength);

	//clear mem:
	ippiDll->ippiFree(pSrcC);
	ippiDll->ippiFree(pDstC);
	ippiDll->ippiDFTFree_C_32fc(spec);
}

//Apply Gaussian filter to image
void HologramGen::FilterGauss(float* pImgDst, int kernelSize, double diaRatio)
{
	IppiSize nOffset, nSize, nRoiSize;
	int stepBytes;
	nOffset.width = max(static_cast<int>(kernelSize/2), static_cast<int>((_mtrxWidth-_mtrxWidth*diaRatio)/2));
	nOffset.height = max(static_cast<int>(kernelSize/2), static_cast<int>((_mtrxHeight-_mtrxHeight*diaRatio)/2));
	nSize.width = _mtrxWidth;
	nSize.height = _mtrxHeight;
	nRoiSize.width = min(_mtrxWidth - 2*static_cast<int>(kernelSize/2), static_cast<int>(_mtrxWidth*diaRatio));
	nRoiSize.height =min(_mtrxHeight - 2*static_cast<int>(kernelSize/2), static_cast<int>(_mtrxHeight*diaRatio));

	Ipp32f* pSrcStart = &pImgDst[nOffset.width * _mtrxHeight + nOffset.height];

	Ipp32f* pGauss = ippiDll->ippiMalloc_32f_C1(_mtrxWidth, _mtrxHeight, &stepBytes);
	Ipp32f* pDstStart = &pGauss[nOffset.width * _mtrxHeight + nOffset.height];
	switch (kernelSize)
	{
	case 3:
		ippiDll->ippiFilterGauss_32f_C1R(pSrcStart, stepBytes, pDstStart, stepBytes, nRoiSize, ippMskSize3x3);
		break;
	case 5:
		ippiDll->ippiFilterGauss_32f_C1R(pSrcStart, stepBytes, pDstStart, stepBytes, nRoiSize, ippMskSize5x5);
		break;
	default:
		break;
	}
	ippsDll->ippsCopy_32f(pGauss, pImgDst, _mtrxLength);
	ippsDll->ippsFree(pGauss);	
}

//get image intensity by sqrt of value
void HologramGen::GetImageIntensity(float* pImg, float* pDst)
{
	float* pSrc = pImg;
	float* pD = pDst;
	for (int i = 0; i < _mtrxLength; i++)
	{
		if(0 < *pSrc)
		{
			*pD = sqrt(*pSrc);
		}
		pSrc++;
		pD++;
	}
}

//load hologram image
void HologramGen::LoadPhaseImage(float* pImg, float* pDst)
{
	float* pSrc = pImg;
	float* pD = pDst;
	for (int i = 0; i < _mtrxLength; i++)
	{
		*pD = static_cast<float>(((*pSrc)*2*PI/MAX_PIXEL_VALUE)-PI);
		pSrc++;
		pD++;
	}
}

//fftshift from top-left corner to center
long HologramGen::QuadrantShift(float* pImgDst)
{
	Ipp32f* pTmp = ippsDll->ippsMalloc_32f(_mtrxLength);
	int halfX = static_cast<int>(floor(_mtrxWidth/2));
	int halfY = static_cast<int>(floor(_mtrxHeight/2));
	int dataLength = halfX*sizeof(Ipp32f);
	int offset = halfX;
	int step = _mtrxWidth;
	int delta = step*halfY;

	Ipp32f* pSrc = pImgDst;
	Ipp32f* pDst = pTmp;
	for (int i = 0; i < halfY; i++)
	{
		//2nd quardrant at destination:
		memcpy_s(pDst,dataLength,pSrc+delta+offset,dataLength);
		//1st quardrant at destination:
		memcpy_s(pDst+offset,dataLength,pSrc+delta,dataLength);
		//4th quardrant at destination:
		memcpy_s(pDst+delta+offset,dataLength,pSrc,dataLength);
		//3rd quardrant at destination:
		memcpy_s(pDst+delta,dataLength,pSrc+offset,dataLength);
		pDst += step;
		pSrc += step;
	}

	ippsDll->ippsCopy_32f(pTmp,pImgDst,_mtrxLength);
	ippsDll->ippsFree(pTmp);
	return TRUE;
}

//clear memory usage before destruction
void HologramGen::ClearMem()
{
	if(NULL != _coeffs)
	{
		free(_coeffs);
		_coeffs = NULL;
	}
}

//log message on ThorLogging
void HologramGen::LogMessage(long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, errMsg);
#endif
}

//hologram generation by Gerchberg-Saxton
long HologramGen::PhaseGenByGS(float* pImg, float* pPolPhase, int iterateCount)
{

	IppStatus ippStatus;
	IppiSize ippSize = {_mtrxWidth, _mtrxHeight};
	IppiRect  roi = {0, 0, ippSize.width, ippSize.height};
	int step = ippSize.width * sizeof(float);
	//int stepBytes;

	Ipp32f* pPolMagn = ippsDll->ippsMalloc_32f(_mtrxLength);
	ippsDll->ippsSet_32f((Ipp32f)1.0,pPolMagn,_mtrxLength);

	//***	image target	***//
	Ipp32f* pImgInt = ippsDll->ippsMalloc_32f(_mtrxLength);
	GetImageIntensity(pImg, pImgInt);
	//shift target DC term to center
	FilterGauss(pImgInt, 3, 1);
	QuadrantShift(pImgInt);

	////***	unit source	***//
	//Ipp32f* pUnit = ippiDll->ippiMalloc_32f_C1(_mtrxWidth,_mtrxHeight,&stepBytes);
	//ippsDll->ippsSet_32f((Ipp32f)1.0,pUnit,_mtrxLength);

	//***	initialize buffers	***//
	//ippsDll->ippsCopy_32f(pUnit, pPolMagn, _mtrxLength);
	//random phase:
	IppsRandUniState_32f* pRus;
	unsigned int seed = 0;
	ippsDll->ippsRandUniformInitAlloc_32f(&pRus, static_cast<float>(-PI), static_cast<float>(PI), seed);
	ippsDll->ippsRandUniform_32f(pPolPhase, _mtrxLength, pRus);
	ippsDll->ippsRandUniformFree_32f(pRus);




	for (int i = 1; i <= iterateCount; i++)	
	{


		//FFT
		FFT(pPolMagn, pPolPhase, true);

		//replace with target:
		ippStatus =ippsDll-> ippsCopy_32f(pImgInt,pPolMagn,_mtrxLength);

		//IFFT
		FFT(pPolMagn, pPolPhase, false);

		//apply gaussian instead of unit int. copy
		//for uniform int. distribution:
		FilterGauss(pPolMagn, 3, 0.5);

		//ippsDll->ippsCopy_32f(pUnit, pPolMagn, _mtrxLength);

	}

	////*** DEBUG	***//
	////do FFT to check image
	//FFT(pPolMagn, pPolPhase, true);
	//ippsDll->ippsCopy_32f(pPolMagn, pPolPhase, _mtrxLength);

	//offset & normalize phase
	Ipp32f minPhase, maxPhase;
	ippsDll->ippsMin_32f(pPolPhase, _mtrxLength, &minPhase);
	ippsDll->ippsAddC_32f_I(abs(minPhase), pPolPhase, _mtrxLength);
	ippsDll->ippsMax_32f(pPolPhase,_mtrxLength,&maxPhase);
	ippsDll->ippsMulC_32f_I(MAX_PIXEL_VALUE/maxPhase,pPolPhase,_mtrxLength);

	//clear:
	//ippsDll->ippsFree(pUnit);
	ippsDll->ippsFree(pPolMagn);
	ippsDll->ippsFree(pImgInt);
	return TRUE;
}


long HologramGen::PhaseGenBy3DGS(float* pImg, float* pPolPhase, int iterateCount, double z)
{

	double pixel_size=1;//*pow(10,-6);
	double z_step=0;//15*pow(10,-6);
	int max_z_step=iterateCount;
	complex<double> unity(0, 1);

	IppStatus ippStatus;
	IppiSize ippSize = {_mtrxWidth, _mtrxHeight};
	IppiRect  roi = {0, 0, ippSize.width, ippSize.height};
	int step = ippSize.width * sizeof(float);
	//int stepBytes;

	Ipp32f* pPolMagn = ippsDll->ippsMalloc_32f(_mtrxLength);
	ippsDll->ippsSet_32f((Ipp32f)1.0,pPolMagn,_mtrxLength);

	//***	image target	***//
	Ipp32f* pImgInt = ippsDll->ippsMalloc_32f(_mtrxLength);
	GetImageIntensity(pImg, pImgInt);
	//shift target DC term to center
	FilterGauss(pImgInt, 3, 1);
	QuadrantShift(pImgInt);

	////***	unit source	***//
	//Ipp32f* pUnit = ippiDll->ippiMalloc_32f_C1(_mtrxWidth,_mtrxHeight,&stepBytes);
	//ippsDll->ippsSet_32f((Ipp32f)1.0,pUnit,_mtrxLength);

	//***	initialize buffers	***//
	//ippsDll->ippsCopy_32f(pUnit, pPolMagn, _mtrxLength);
	//random phase:
	IppsRandUniState_32f* pRus;
	unsigned int seed = 0;
	ippsDll->ippsRandUniformInitAlloc_32f(&pRus, static_cast<float>(-PI), static_cast<float>(PI), seed);
	ippsDll->ippsRandUniform_32f(pPolPhase, _mtrxLength, pRus);
	ippsDll->ippsRandUniformFree_32f(pRus);

	double *coeff;
	double *poly;

	Ipp32f* totalPhase = ippsDll->ippsMalloc_32f(_mtrxLength * sizeof(Ipp32f));



	coeff=CalculateZernikeCoeffs(_refractiveIndex,_waveNumber,z,_alpha);

	double a;
	double b;


	for (int i = 1; i <= iterateCount; i++)	
	{


		//FFT
		FFT(pPolMagn, pPolPhase, true);

		//replace with target:
		ippStatus =ippsDll-> ippsCopy_32f(pImgInt,pPolMagn,_mtrxLength);

		//IFFT
		FFT(pPolMagn, pPolPhase, false);

		//apply gaussian instead of unit int. copy
		//for uniform int. distribution:
		FilterGauss(pPolMagn, 3, 0.5);



	}




	for(int v=0;v<_slmYsize;v++)
	{
		for (int u=0;u<_slmXsize;u++)
		{
			poly=CalculateZernikePoly(((double)u- (double)_slmXsize/2)/ (double)_slmXsize*2,((double)v- (double)_slmYsize/2)/ (double)_slmYsize*2);
			//poly=CalculateZernikePoly(((double)u),((double)v));

			a = cos(-2 * PI*(coeff[0] * poly[0] + coeff[1] * poly[1] + coeff[2] * poly[2]));
			b = sin(-2 * PI*(coeff[0] * poly[0] + coeff[1] * poly[1] + coeff[2] * poly[2]));

			complex<double> complexnumber(a, b);


			totalPhase[(v)*_slmXsize + u] = static_cast<Ipp32f>(arg(complexnumber) + PI);
			complex<double> tempComplex(0,pPolPhase[(v )*_slmXsize + u]+totalPhase[(v )*_slmXsize + u]);
			pPolPhase[(v )*_slmXsize + u]= static_cast<float>(arg(exp(tempComplex)));



		}
	}


	NormalizePhase(pPolPhase);


	////*** DEBUG	***//
	////do FFT to check image
	//FFT(pPolMagn, pPolPhase, true);
	//ippsDll->ippsCopy_32f(pPolMagn, pPolPhase, _mtrxLength);

	//offset & normalize phase
	//Ipp32f minPhase, maxPhase;
	//ippsDll->ippsMin_32f(pPolPhase, _mtrxLength, &minPhase);
	//ippsDll->ippsAddC_32f_I(abs(minPhase), pPolPhase, _mtrxLength);
	//ippsDll->ippsMax_32f(pPolPhase,_mtrxLength,&maxPhase);
	//ippsDll->ippsMulC_32f_I(MAX_PIXEL_VALUE/maxPhase,pPolPhase,_mtrxLength);

	//clear:
	//ippsDll->ippsFree(pUnit);
	ippsDll->ippsFree(pPolMagn);
	ippsDll->ippsFree(pImgInt);
	return TRUE;
}






long HologramGen::WeightByDistance(float* pImgDst)
{
	if((NULL == pImgDst) || (0 == _mtrxWidth) || (0 == _mtrxHeight))
		return FALSE;

	//y = mx+b, linear mapping
	const int FIRST_ORDER_RADIUS = 183;
	const int DEFAULT_RADIUS = 256;
	const int MAX_PERCENT = 75;
	const int MIN_PERCENT = 25;
	const int HUNDRED_PERCENT = 100;
	double circleBound = (floor(_mtrxWidth/2))*FIRST_ORDER_RADIUS/DEFAULT_RADIUS;
	double m = MAX_PERCENT/circleBound;
	double b = MIN_PERCENT;
	int centerX = static_cast<int>(floor(_mtrxWidth/2));
	int centerY = static_cast<int>(floor(_mtrxHeight/2));

	//sort 2D array to 1D, then assign pixel values
	float* pSrc = pImgDst;
	for (long i = 0; i < static_cast<long>(_mtrxWidth); i++)
	{
		for (long j = 0; j < static_cast<long>(_mtrxHeight); j++)
		{
			if(0 < (*pSrc))
			{
				double disFromCenter = sqrt(pow((i-centerX),2)+pow((j-centerY),2));
				float weightValue = static_cast<float>((m*disFromCenter+b)/HUNDRED_PERCENT*MAX_PIXEL_VALUE);
				if(circleBound > disFromCenter)
				{
					*pSrc = weightValue;
				}
			}
			pSrc++;
		}
	}

	return TRUE;
}

//Filter for single pixel per pattern point
long HologramGen::SinglePassFilter(float* pImgDst)
{
	if((NULL == pImgDst) || (0 == _mtrxWidth) || (0 == _mtrxHeight))
		return FALSE;

	//sort 2D array to 1D, then assign pixel values
	float* pSrc = pImgDst;
	for (long i = 0; i < static_cast<long>(_mtrxWidth); i++)
	{
		for (long j = 0; j < static_cast<long>(_mtrxHeight); j++)
		{
			if(0 < (*pSrc))
			{
				//search neighbors: right, bottom, bottom-right
				//not the last point
				if((i != (_mtrxWidth-1)) || (j != (_mtrxHeight-1)))	
				{
					if((*pSrc) <= (*(pSrc+1)))
					{
						(*pSrc) = 0;
					}
					else
					{
						(*(pSrc+1)) = 0;
					}
				}
				//not the last row
				if(j != (_mtrxHeight-1))							
				{
					if((*pSrc) <= (*(pSrc+_mtrxWidth)))
					{
						(*pSrc) = 0;
					}
					else
					{
						(*(pSrc+_mtrxWidth)) = 0;
					}
				}
				//not the last row or the last point of the second last row
				if((j != (_mtrxHeight-1)) || ((j == (_mtrxHeight-2)) && (i != (_mtrxWidth-1))))
				{
					if((*pSrc) <= (*(pSrc+_mtrxWidth+1)))
					{
						(*pSrc) = 0;
					}
					else
					{
						(*(pSrc+_mtrxWidth+1)) = 0;
					}
				}
			}
			pSrc++;
		}
	}

	return TRUE;
}
