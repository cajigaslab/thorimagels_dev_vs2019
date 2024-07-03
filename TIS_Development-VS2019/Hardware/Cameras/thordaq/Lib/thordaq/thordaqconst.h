
//========= Copyright Thorlabs Imaging Research Group, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef CONST_H
#define CONST_H
#include "stdafx.h"

#define TEST TRUE

#define MIN_CHANNEL              0x0000 // 000000B all channels are disabled
#define MAX_CHANNEL              0x003F // 111111B all channels are enabled
#define RANGE_CHANNEL            MAX_CHANNEL - MIN_CHANNEL

#define MAX_PIXEL_X		4096
#define MIN_PIXEL_X		32
#define RANGE_PIXEL_X   MAX_PIXEL_X - MIN_PIXEL_X

#define GALVO_PARK_POSITION          9.999

#define MAX_PIXEL_Y		4096 * 4
#define MIN_PIXEL_Y		1 //Allow Line Scan, which vertical resolution is 1
#define RANGE_PIXEL_Y   MAX_PIXEL_Y - MIN_PIXEL_Y

#define MIN_FRAME_RATE           1
#define MAX_FRAME_RATE           UINT32_MAX
#define RANGE_FRAME_RATE         MAX_FRAME_RATE  - MIN_FRAME_RATE 

#define MIN_DEBUG_MODE           0
#define MAX_DEBUG_MODE           1
#define RANGE_DEBUG_MODE         MAX_DEBUG_MODE  - MIN_DEBUG_MODE 

#define MAX_AUX_DIG_OUT_CHANNEL	 5

#define MIN_NUM_PLANES			1
#define MAX_NUM_PLANES			16

#define MAX_FLYBACK_DAC_SAMPLES 65535

#endif