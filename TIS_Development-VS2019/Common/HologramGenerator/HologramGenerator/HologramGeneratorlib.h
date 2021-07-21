#include "..\..\PDLL\pdll.h"

#define AFFINE_COEFF_CNT	6
#define PROJECT_COEFF_CNT	8

typedef enum {
	GerchbergSaxton,
	CompressiveSensing,
	LAST_ALGORITHM
}HoloGenAlg;

typedef enum {
	AFFINE = 0,
	PROJECTIVE = 1,
	LAST_FITTING
}GeoFittingAlg;

class IHologramGenerator
{
public:

	virtual	long FittingTransform(float* pImgDst) = 0;
	virtual long CalculateCoeffs(const float* pSrcPoints, const float* pTgtPoints, long size,long fittingAlg, double* affCoeffs) = 0;
	virtual long GenerateHologram(float* pImg, int iteCount, int weightRadiusPx, double minPercent, double maxPercent, float z) = 0;
	virtual long Set3DParam(double na, double wavelength) = 0;
	virtual long SetSize(int width, int height, double pixelUM) = 0;
	virtual long SetAlgorithm(int algorithmID) = 0;
	virtual long CombineHologramFiles(const wchar_t * pathAndFilename1, const wchar_t * pathAndFilename2, long shiftPx) = 0;
	virtual long SetCoeffs(long fittingAlg, double* affCoeffs) = 0;
	virtual long VerticalFlip(float* pImgDst) = 0;
	virtual long RotateForAngle(float* pImgDst, double angle) = 0;
	virtual long ScaleByFactor(float* pImgDst, double scaleX, double scaleY) = 0;
	virtual long OffsetByPixels(float* pImgDst, long offsetX, long offsetY) = 0;
	virtual long NormalizePhase(float* img)=0;
};

class HoloGenDLL : public PDLL, public IHologramGenerator
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(push)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(HoloGenDLL)
#pragma warning(pop)
#pragma warning(pop)	
	DECLARE_FUNCTION1(long, FittingTransform, float*)
	DECLARE_FUNCTION5(long, CalculateCoeffs,const float*, const float*, long, long, double*)
	DECLARE_FUNCTION6(long, GenerateHologram, float*, int, int, double, double, float)
	DECLARE_FUNCTION2(long, Set3DParam, double, double)
	DECLARE_FUNCTION3(long, SetSize, int, int, double)
	DECLARE_FUNCTION1(long, SetAlgorithm, int)
	DECLARE_FUNCTION3(long, CombineHologramFiles, const wchar_t *, const wchar_t *, long)
	DECLARE_FUNCTION2(long, SetCoeffs, long, double*)
	DECLARE_FUNCTION1(long, VerticalFlip, float*)
	DECLARE_FUNCTION2(long, RotateForAngle, float*, double)
	DECLARE_FUNCTION3(long, ScaleByFactor, float*, double, double)
	DECLARE_FUNCTION3(long, OffsetByPixels, float*, long, long)
	DECLARE_FUNCTION1(long, NormalizePhase, float*)
};