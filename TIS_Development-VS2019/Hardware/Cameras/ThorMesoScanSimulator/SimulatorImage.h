#pragma once
#include "..\ThorMesoScan\Types.h"
class SimulatorImage
{
public:
	SimulatorImage(const char* path, double fieldWidth, double fieldHeight);
	~SimulatorImage();

	void GetImageBuffer(StripInfo* stripInfo , unsigned short* buffer, int stripWidth);

private:
	int _width;
	int _height;
	unsigned short* _buffer;
	double _maxScale;
};

