// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "HologramGenerator.h"

#define DllExportHoloGen extern "C" long __declspec(dllexport)

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

DllExportHoloGen SetDefocus(double n, double NAeff, long Np)
{
	return HologramGen::getInstance()->SetDefocus(n, NAeff, Np);
}

DllExportHoloGen Set3DParam(double na, double wavelength)
{
	return HologramGen::getInstance()->Set3DParam(na, wavelength);
}

DllExportHoloGen SetSize(int width, int height, double pixelUM)
{
	return HologramGen::getInstance()->SetSize(width, height, pixelUM);
}

DllExportHoloGen SetAlgorithm(int algorithmID)
{
	return HologramGen::getInstance()->SetAlgorithm(algorithmID);
}

DllExportHoloGen CombineHologramFiles(const wchar_t* pathAndFilename1, const wchar_t* pathAndFilename2, long shiftPx)
{
	return HologramGen::getInstance()->CombineHologramFiles(pathAndFilename1, pathAndFilename2, shiftPx);
}

DllExportHoloGen SetCoeffs(long fittingAlgorithm, double* affCoeffs)
{
	return HologramGen::getInstance()->SetCoeffs(fittingAlgorithm, affCoeffs);
}

DllExportHoloGen FittingTransform(float* pImgDst)
{
	return HologramGen::getInstance()->FittingTransform(pImgDst);
}

DllExportHoloGen CalculateCoeffs(const float* pSrcPoints, const float* pTgtPoints, long size, long fittingAlg, double* affCoeffs)
{
	return HologramGen::getInstance()->CalculateCoeffs(pSrcPoints, pTgtPoints, size, fittingAlg, affCoeffs);
}

DllExportHoloGen GenerateHologram(float* pImgDst, int iteCount, int weightRadiusPx, double minPercent, double maxPercent, float z)
{
	return HologramGen::getInstance()->GenerateHologram(pImgDst, iteCount, weightRadiusPx, minPercent, maxPercent, z);
}

DllExportHoloGen Generate3DHologram(void* pMemStruct, int zCount)
{
	return HologramGen::getInstance()->Generate3DHologram(pMemStruct, zCount);
}

DllExportHoloGen DefocusHologram(float* pImgDst, double kz)
{
	return HologramGen::getInstance()->DefocusHologram(pImgDst, kz);
}

DllExportHoloGen VerticalFlip(float* pImgDst)
{
	return HologramGen::getInstance()->VerticalFlip(pImgDst);
}

DllExportHoloGen RotateForAngle(float* pImgDst, double angle)
{
	return HologramGen::getInstance()->RotateForAngle(pImgDst, angle);
}

DllExportHoloGen ScaleByFactor(float* pImgDst, double scaleX, double scaleY)
{
	return HologramGen::getInstance()->ScaleByFactor(pImgDst, scaleX, scaleY);
}

DllExportHoloGen OffsetByPixels(float* pImgDst, long offsetX, long offsetY)
{
	return HologramGen::getInstance()->OffsetByPixels(pImgDst, offsetX, offsetY);
}
