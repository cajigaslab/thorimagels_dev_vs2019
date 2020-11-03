#pragma once

#define DllExport_ThorDiskIO extern "C" long __declspec(dllexport)

DllExport_ThorDiskIO ReadImageInfo(wchar_t * selectedFileName, long &width, long &height, long &colorChannels);
DllExport_ThorDiskIO ReadImage(char *selectedFileName, char* &outputBuffer);
DllExport_ThorDiskIO ReadColorImage(char *redFileName, char *greenFileName, char *blueFileName, char* &outputBuffer, long cameraWidth, long cameraHeight);