
//========= Copyright Thorlabs Imaging Research Group, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef CONST_H
#define CONST_H
//#include "stdafx.h"
#include "pch.h"

#define TEST TRUE

#define MIN_CHANNEL             0x0000 // 000000B all channels are disabled
#define MAX_CHANNEL             0x003F // 111111B all channels are enabled
#define RANGE_CHANNEL           MAX_CHANNEL - MIN_CHANNEL

#define MAX_PIXEL_X				4096
#define MIN_PIXEL_X				32
#define RANGE_PIXEL_X			MAX_PIXEL_X - MIN_PIXEL_X

#define MAX_PIXEL_Y				4096
#define MIN_PIXEL_Y				1 //Allow Line Scan, which vertical resolution is 1
#define RANGE_PIXEL_Y			MAX_PIXEL_Y - MIN_PIXEL_Y

#define GALVO_PARK_POSITION     9.999

#define MAX_DATA_HSIZE			524280

#define MIN_FRAME_RATE          1
#define MAX_FRAME_RATE          UINT32_MAX
#define RANGE_FRAME_RATE		MAX_FRAME_RATE  - MIN_FRAME_RATE 

#define MIN_DEBUG_MODE          0
#define MAX_DEBUG_MODE          1
#define RANGE_DEBUG_MODE        MAX_DEBUG_MODE  - MIN_DEBUG_MODE 

#endif