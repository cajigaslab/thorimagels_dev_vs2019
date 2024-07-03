#include "stdafx.h"
#include "ImageDistortionCorrection.h"
#include <corecrt_math_defines.h>

ImageDistortionCorrection::ImageDistortionCorrection()
{
	_pTempDoubleBuffCorrection = nullptr;
	_width = 0;
	_height = 0;
}

bool ImageDistortionCorrection::_instanceFlag = false;
std::unique_ptr<ImageDistortionCorrection> ImageDistortionCorrection::_single(nullptr);//Instantiated on first use

ImageDistortionCorrection::~ImageDistortionCorrection()
{
    _instanceFlag = false;
	_single.release();
	if (_pTempDoubleBuffCorrection != nullptr)
	{
		delete[] _pTempDoubleBuffCorrection;
		_pTempDoubleBuffCorrection = nullptr;
	}
}

ImageDistortionCorrection* ImageDistortionCorrection::getInstance()
{
    if (!_instanceFlag)
    {
		_instanceFlag = true;
        _single.reset(new ImageDistortionCorrection());       
    }
    return _single.get();
}

// Convert radians to degrees and vice versa
inline double radians(double deg) { return deg * M_PI / 180.0; }
inline double degrees(double rad) { return rad * 180.0 / M_PI; }

long ImageDistortionCorrection::SetImageDistortionCorrectionParameters(int imageWidth, int imageHeight, double xAngleMax, double yAngleMax, double galvoTiltAngle, double scanLensFocalLength)
{
	int width = imageWidth;
	int height = imageHeight;
   _width = imageWidth;
   _height = imageHeight;
	const double fs = scanLensFocalLength; // Scan Lens focal Length
	const double XAngleMax = xAngleMax; // degree optical half angle max
	const double YAngleMax = yAngleMax; // degree optical half angle max
	const double AngleIn = galvoTiltAngle; // Galvo Tilt angle

	double XAngleStep = XAngleMax / (width / 2.0f);
	double YAngleStep = YAngleMax / (height / 2.0f);

	std::vector<double> XAngleList(width), YAngleList(height);
	XAngleList[0] = -XAngleMax;
	for (int i = 1; i < width; ++i)
	{
		XAngleList[i] = XAngleList[i - 1] + XAngleStep;
	}

	YAngleList[0] = -YAngleMax;
	for (int j = 1; j < height; ++j)
	{
		YAngleList[j] = YAngleList[j - 1] + YAngleStep;
	}
	std::vector<double> imgPixX(width * height);
	std::vector<double> imgPixY(width * height);


	// Calculate the actual X, Y coordinates (distorted) at the focal plane corresponding to the square matrix of angles
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			double lx = sin(radians(AngleIn + XAngleList[i])) * cos(radians(AngleIn)) - cos(radians(AngleIn + XAngleList[i])) * sin(radians(AngleIn)) * cos(radians(YAngleList[j]));
			double ly = cos(radians(AngleIn + XAngleList[i])) * sin(radians(YAngleList[j]));
			double lz = cos(radians(AngleIn + XAngleList[i])) * cos(radians(AngleIn)) * cos(radians(YAngleList[j])) + sin(radians(AngleIn + XAngleList[i])) * sin(radians(AngleIn));

			if (lx == 0 && ly == 0) {
				imgPixX[j * width + i] = 0;
				imgPixY[j * width + i] = 0;
			}
			else {
				imgPixX[j * width + i] = fs * acos(lz) * lx / sqrt(lx * lx + ly * ly);
				imgPixY[j * width + i] = fs * acos(lz) * ly / sqrt(lx * lx + ly * ly);
			}
		}
	}

	double YMax = *std::max_element(imgPixY.begin(), imgPixY.end());
	double XMax = *std::max_element(imgPixX.begin(), imgPixX.end());
	double YMin = *std::min_element(imgPixY.begin(), imgPixY.end());
	double XMin = *std::min_element(imgPixX.begin(), imgPixX.end());
	YMax = max(std::abs(YMax), std::abs(YMin));
	XMax = max(std::abs(XMax), std::abs(XMin));

	double XScale = (width / 2.0) / XMax;
	double YScale = (height / 2.0) / YMax;

	_imageDistortionCorrectionPixelData.resize(width * height);
	std::vector<double> antAliasWt((width + 1) * (height + 1), 0.0);
	// Compute indices for each pixel and store them
	//compue antialiasing matrix
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			PixelData& pd = _imageDistortionCorrectionPixelData[j * width + i];
			pd.pixelIndex = j * width + i;
			int indX1 = static_cast<int>(std::floor(XScale * imgPixX[j * width + i] + width / 2.0));
			int indX2 = static_cast<int>(std::ceil(XScale * imgPixX[j * width + i] + width / 2.0));
			int indY1 = static_cast<int>(std::floor(YScale * imgPixY[j * width + i] + height / 2.0));
			int indY2 = static_cast<int>(std::ceil(YScale * imgPixY[j * width + i] + height / 2.0));
			double deltaX = XScale * imgPixX[j * width + i] + width / 2.0 - indX1;
			double deltaY = YScale * imgPixY[j * width + i] + height / 2.0 - indY1;
			pd.pixTL = indY1 * width + indX1;
			pd.pixTR = indY1 * width + indX2;
			pd.pixBL = indY2 * width + indX1;
			pd.pixBR = indY2 * width + indX2;
			antAliasWt[pd.pixTL] += (1.0 - deltaX) * (1.0 - deltaY);
			antAliasWt[pd.pixTR] += deltaX * (1.0 - deltaY);
			antAliasWt[pd.pixBL] += (1.0 - deltaX) * deltaY;
			antAliasWt[pd.pixBR] += deltaX * deltaY;
		}
	}

	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			if (antAliasWt[j * width + i] != 0.0) {
				antAliasWt[j * width + i] = 1.0 / antAliasWt[j * width + i];
			}
		}
	}

	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			PixelData& pd = _imageDistortionCorrectionPixelData[j * width + i];
			int indX1 = static_cast<int>(std::floor(XScale * imgPixX[j * width + i] + width / 2.0));
			int indX2 = static_cast<int>(std::ceil(XScale * imgPixX[j * width + i] + width / 2.0));
			int indY1 = static_cast<int>(std::floor(YScale * imgPixY[j * width + i] + height / 2.0));
			int indY2 = static_cast<int>(std::ceil(YScale * imgPixY[j * width + i] + height / 2.0));
			double deltaX = XScale * imgPixX[j * width + i] + width / 2.0 - indX1;
			double deltaY = YScale * imgPixY[j * width + i] + height / 2.0 - indY1;
			pd.factorTL = (1 - deltaX) * (1 - deltaY) * antAliasWt[pd.pixTL];
			pd.factorTR = deltaX * (1 - deltaY) * antAliasWt[pd.pixTR];
			pd.factorBL = (1 - deltaX) * deltaY * antAliasWt[pd.pixBL];
			pd.factorBR = deltaX * deltaY * antAliasWt[pd.pixBR];
		}
	}

	int cores = omp_get_num_procs();
	int num_threads = static_cast<int>(cores * 0.7);
	omp_set_num_threads(num_threads);
	if (_pTempDoubleBuffCorrection != nullptr)
	{
		delete[] _pTempDoubleBuffCorrection;
	}
	_pTempDoubleBuffCorrection = new double[width * height];

	return TRUE;
}

long ImageDistortionCorrection::CorrectPreludeImageDistortion(const USHORT* srcImage, USHORT* dstImage, int width, int height)
{
	if (width != _width || height != _height)
	{
		return FALSE;
	}

	const long imageSize = _width * _height;

	memset(_pTempDoubleBuffCorrection, 0, imageSize * sizeof(double));

	const unsigned short* pixels = srcImage;// +imageSize * c2;

	for (int idx = 0; idx < imageSize; ++idx)
	{
		const PixelData& pd = _imageDistortionCorrectionPixelData[idx];

		// Assuming pixelData has been appropriately prepared to include only valid indices
		const double pixelValue = pixels[pd.pixelIndex];

		_pTempDoubleBuffCorrection[pd.pixTL] += pixelValue * pd.factorTL;
		_pTempDoubleBuffCorrection[pd.pixTR] += pixelValue * pd.factorTR;
		_pTempDoubleBuffCorrection[pd.pixBL] += pixelValue * pd.factorBL;
		_pTempDoubleBuffCorrection[pd.pixBR] += pixelValue * pd.factorBR;
	}


	unsigned short* pixelsDup = dstImage;

#pragma omp parallel for schedule(static, 40)
	for (int idx = 0; idx < imageSize; ++idx)
	{
		pixelsDup[idx] = static_cast<USHORT>(std::round(_pTempDoubleBuffCorrection[idx]));
	}
	return TRUE;
}