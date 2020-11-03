///////////////////////////////////////////////////////////////////////////////////////////////
//
//	$Archive: maskoperations.h $
//
//	Project: Thorlabs - Visualization Module
//	
//	Content: Interface input-ouput structures and function signatures
//
//	Creator: Shrikant Mahajan
//
///////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef __SDK_H
#define __SDK_H
#ifdef __cplusplus
extern "C" {__declspec(dllexport) long  LineProfile(unsigned char* imageBuffer, long imageWidth, long imageHeight,
	long bitdepth,
	long point1X, long point1Y,
	long point2X, long point2Y,
	long lineWidth,
	unsigned char* resultBuffer);
}

#endif
#endif