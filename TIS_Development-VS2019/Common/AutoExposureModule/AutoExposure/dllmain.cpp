#include "stdafx.h"
#include "AutoExposureImpl.h"
#include "AutoExposure.h"

extern "C" void __declspec(dllexport) RunAutoExposure(ICamera *camera)
{
	return AutoExposure::GetInstance()->Start(camera);
}

extern "C" void __declspec(dllexport) SetAutoExposureStopFlag()
{
	return AutoExposure::GetInstance()->Stop();
}

extern "C" bool __declspec(dllexport) IsAutoExposureStable()
{
	return AutoExposure::GetInstance()->IsStable();
}

extern "C" bool __declspec(dllexport) IsAutoExposureRunning()
{
	return AutoExposure::GetInstance()->IsRunning();
}

extern "C" bool __declspec(dllexport) GetAutoExposureImage(char* imageBuffer, FrameInfoStruct & frameInfo, long imageBufferSize, long& frameNumber)
{
	return AutoExposure::GetInstance()->GetAutoExposureImage(imageBuffer, frameInfo, imageBufferSize, frameNumber);
	return false;
}

extern "C" void __declspec(dllexport) RegisterAutoExposureUpdateCallback(void(*callback)(bool, bool, double, double))
{
	AutoExposure::GetInstance()->RegisterUpdateCallback(callback);
}

extern "C" void __declspec(dllexport) UnregisterAutoExposureUpdateCallback(void(*callback)(bool, bool, double, double))
{
	AutoExposure::GetInstance()->UnregisterUpdateCallback(callback);
}

extern "C" void __declspec(dllexport) SetAutoExposureTargetPercent(double percent)
{
	AutoExposure::GetInstance()->SetTargetPercent(percent);
}

extern "C" double __declspec(dllexport) GetAutoExposureTargetPercent()
{
	return AutoExposure::GetInstance()->GetTargetPercent();
}
