#pragma once

#include "..\..\ImageWaveformBuilderDll.h"


///<summary> class to abstract waveform generation, design for active loading 
///all types of analog or digital, may refactor ImageWaveformBuilder in the future. </summary>
class WaveformBuilderFactory
{
public:
	WaveformBuilderFactory();
	static IWaveformBuilder *GetBuilderInstance(int id);
};
