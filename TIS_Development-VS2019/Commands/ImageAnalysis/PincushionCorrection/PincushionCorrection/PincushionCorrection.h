
#define DllExport_Pin extern "C" long __declspec( dllexport )

DllExport_Pin PincushionCorrection(char * pImageBuffer, long width, long height, long dataType, double k1, double k2, double k3, double k4);


struct Point
{
		long x;
		long y;
	};