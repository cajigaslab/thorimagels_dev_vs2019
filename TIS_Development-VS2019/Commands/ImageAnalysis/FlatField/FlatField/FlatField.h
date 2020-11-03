
#define DllExport_FlatField extern "C" long __declspec( dllexport )

DllExport_FlatField BackgroundCorrection(unsigned short *pBufferData, unsigned short *pBufferCorrection, long w, long h, long c);
DllExport_FlatField FlatFieldCorrection(unsigned short *pBufferData, unsigned short *pBufferCorrection, double flatFieldMeanValue, long w, long h, long c);
DllExport_FlatField FlatFieldAndBackgroundCorrection(unsigned short * pBufferData,unsigned short * pBufferCorrectionBackground,unsigned short * pBufferCorrectionFlatField, double flatFieldMeanValue, long w, long h, long c);

