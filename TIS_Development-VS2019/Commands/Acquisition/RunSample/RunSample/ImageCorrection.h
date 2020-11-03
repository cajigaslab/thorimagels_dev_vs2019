#pragma once


class CorrectionImage
{
public:
	CorrectionImage();
	~CorrectionImage();

	long Create(wstring ws, long w, long h, long c);

	double GetMeanIntensity();
	long GetWidth();
	long GetHeight();
	long GetColorChannels();
	unsigned short * GetBuffer();
private:

	unsigned short *_pBuffer;
	long _width;
	long _height;
	long _colorChannels;
	double _meanIntensity;
};


long SetupImageCorrectionBuffers(IExperiment *pExp,long w, long h);

void ImageCorrections(IExperiment * pExp, char* pBuffer, long width, long height, long channels);