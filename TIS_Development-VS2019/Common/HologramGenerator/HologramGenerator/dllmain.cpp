// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "HologramGenerator.h"

#define DllExportHoloGen extern "C" long __declspec(dllexport)

BOOL APIENTRY DllMain( HMODULE hModule,
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

DllExportHoloGen Set3DParam(double na,double wavelength)
{
	return HologramGen::getInstance()->Set3DParam(na,wavelength);
}



DllExportHoloGen SetSize(int width, int height)
{
	return HologramGen::getInstance()->SetSize(width, height);
}

DllExportHoloGen SetAlgorithm(int algorithmID)
{
	return HologramGen::getInstance()->SetAlgorithm(algorithmID);
}

DllExportHoloGen SetPathandFilename(const wchar_t * pathAndFilename)
{
	return HologramGen::getInstance()->SetPathandFilename(pathAndFilename);
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

DllExportHoloGen GenerateHologram(float* pImgDst, int iteCount, float z)
{
	return HologramGen::getInstance()->GenerateHologram(pImgDst, iteCount, z);
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
