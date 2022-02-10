#pragma once

#if defined(HOLOGEN_EXPORTS)
#define DllExport_HOLOGEN __declspec(dllexport)
#else
//    definitions used when using DLL
#define DllExport_HOLOGEN __declspec(dllimport)
#endif

#pragma warning( push )
#pragma warning( disable : 4251 )

class DllExport_HOLOGEN HologramGen
{
private:
	static bool instanceFlag;									///singleton created flag
	static std::auto_ptr<HologramGen> single;					///pointer to internal object

	int	_mtrxWidth;
	int _mtrxHeight;
	int _mtrxLength;
	int _holoGenMethod;
	double _pixelUM;
	double* _coeffs;
	long _fittingMethod;
	double _refractiveN;
	double _NAeff;
	long _fftSize;

public:
	static HologramGen* getInstance();

	~HologramGen() { ClearMem(); instanceFlag = false; }

	long FittingTransform(float* pImgDst);
	long CalculateCoeffs(const float* pSrcPoints, const float* pTgtPoints, long size, long fittingAlg, double* coeffs);
	double* CalculateZernikeCoeffs(double n, double k, double z, double alpha);
	double* CalculateZernikePoly(double u, double v);
	long NormalizePhase(float* img);
	long Set3DParam(double na, double wavelength);
	long SetDefocus(double n, double NAeff, long Np);
	long SetSize(int width, int height, double pixelUM);
	long SetAlgorithm(int algorithmID);
	long CombineHologramFiles(const wchar_t* pathAndFilename1, const wchar_t* pathAndFilename2, long shiftPx);
	long SetCoeffs(long algorithm, double* affCoeffs);
	long VerticalFlip(float* pImgDst);
	long RotateForAngle(float* pImgDst, double angle);
	long ScaleByFactor(float* pImgDst, double scaleX, double scaleY);
	long OffsetByPixels(float* pImgDst, long offsetX, long offsetY);
	long GenerateHologram(float* pImgDst, int iteCount, int weightRadiusPx, double minPercent, double maxPercent, float z);
	long Generate3DHologram(void* pMemStruct, int zCount);
	long DefocusHologram(float* pImgDst, double kz);

private:
	HologramGen();
	long AffineTransform(float* pImgDst);
	long ProjectTransform(float* pImgDst);
	long CalculateAffineCoeffs(const float* pSrcPoints, const float* pTgtPoints, long size, double* affCoeffs);
	long CalculateProjectCoeffs(const float* pSrcPoints, const float* pTgtPoints, long size, double* projCoeffs);
	void FFT(float* pPolMagnDst, float* pPolPhaseDst, bool forward);
	void FilterGauss(float* pImgDst, int kernelSize, double diaRatio);
	void GetImageIntensity(float* pImg, float* pDst);
	void LoadPhaseImage(float* pImg, float* pDst);
	long QuadrantShift(float* pImgDst);
	void ClearMem();
	void LogMessage(long eventLevel);
	long PhaseGenByGS(float* pImgIn, float* pPhaseDst, int iterateCount);
	long PhaseGenBy3DGS(float* pImg, float* pPolPhase, int iterateCount, double z);
	long WeightByDistance(float* pImgDst, int weightRadiusPx, double minPercent = 25, double maxPercent = 75);
	long SinglePassFilter(float* pImgDst);
	long GenerateZHologram(float* pImgInt, float* pImgPhase, void* pImgCx, float* pTarget, float zValue);

};

#pragma warning( push )