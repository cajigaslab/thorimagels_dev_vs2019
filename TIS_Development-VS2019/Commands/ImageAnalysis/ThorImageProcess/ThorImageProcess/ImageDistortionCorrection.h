#pragma once
#include <Windows.h>
#include <memory>
#include <vector>
#include <math.h>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <omp.h>

class ImageDistortionCorrection
{
private:
	// Define a structure to hold indices for a pixel
	struct PixelData
	{
		int pixTL, pixTR, pixBL, pixBR;
		double factorTL, factorTR, factorBL, factorBR;
		int pixelIndex;
	};
	static bool _instanceFlag;
	static std::unique_ptr<ImageDistortionCorrection> _single;
	ImageDistortionCorrection();
	double* _pTempDoubleBuffCorrection;
	std::vector<PixelData> _imageDistortionCorrectionPixelData;
	int _width, _height;
public:
	static ImageDistortionCorrection* getInstance();
	long SetImageDistortionCorrectionParameters(int imageWidth, int imageHeight, double xAngleMax, double yAngleMax, double galvoTiltAngle, double scanLensFocalLength);
	long CorrectPreludeImageDistortion(const USHORT* srcImage, USHORT* dstImage, int width, int height);
	~ImageDistortionCorrection();
};

