#pragma once
#include "WaveformManagerBase.h"
#include <math.h>

//voltage limit of each channel
#define MAX_VOLT_GX1 (10.0)
#define MAX_VOLT_GX2 (10.0)
#define MAX_VOLT_GY (10.0)
#define MAX_VOLT_VC (10.0)

#define MIN_VOLT_GX1 (-10.0)
#define MIN_VOLT_GX2 (-10.0)
#define MIN_VOLT_GY (-10.0)
#define MIN_VOLT_VC (-10.0)
//pokels cell param limit
#define MAX_POKELSPOINT_COUNT (1000)
#define MIN_POKELSPOINT_COUNT (10)
//voice coil param limit
#define VC_PARAM_MAX (80)
//flyback param limit
#define FLYBACK_MAX_VELOCITY (10000.0)
#define FLYBACK_MAX_OVERSHOOT (5.0)

typedef CircleBufferPointer<double> BufferPtr;
typedef CircleBufferPointer<unsigned char> UInt8BufferPtr;