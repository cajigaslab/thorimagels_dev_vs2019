#pragma once

struct FrameInfo;
struct SampleInfo;

class OMETiffWritter
{
private:
	OMETiffWritter();
	static bool _instanceFlag;
	static OMETiffWritter* _single;

public:
	~OMETiffWritter();
	static OMETiffWritter* getInstance();

	static long Create(char* file_name);

	static long ConfigOMEHeader(long handle, SampleInfo sample, int regionPixelX, int regionPixelY, float regionW, float regionH, int zCount, int tCount,
		int regionPositionPixelX, int regionPositionPixelY, int bitsPerPixel, float regionPixelSizeUM, double zStepSizeUM, double intervalSec, int channelNumber, const char* channels);
	
	static long SaveOMEData(long handle, int channelID, unsigned short zIndex, unsigned short tIndex, unsigned short* data);

	static long SaveAdditionalData(long handle, const char* data, int size, const char* name);
	
	static long CloseFile(long handle);
};