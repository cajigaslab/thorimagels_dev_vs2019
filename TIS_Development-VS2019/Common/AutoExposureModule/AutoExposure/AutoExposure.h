#pragma once

extern "C" void __declspec(dllexport) RunAutoExposure(ICamera *camera);
extern "C" void __declspec(dllexport) SetAutoExposureStopFlag();
extern "C" bool __declspec(dllexport) IsAutoExposureStable();
extern "C" bool __declspec(dllexport) IsAutoExposureRunning();
extern "C" bool __declspec(dllexport) GetAutoExposureImage(char *imageBuffer, FrameInfoStruct& frameInfo, long imageBufferSize, long& frameNumber);
extern "C" void __declspec(dllexport) RegisterAutoExposureUpdateCallback(void(*callback)(bool, bool, double, double));
extern "C" void __declspec(dllexport) UnregisterAutoExposureUpdateCallback(void(*callback)(bool, bool, double, double));
extern "C" void __declspec(dllexport) SetAutoExposureTargetPercent(double);
extern "C" double __declspec(dllexport) GetAutoExposureTargetPercent();
