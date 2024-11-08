#include "..\..\..\..\common\PDLL\pdll.h"

enum MORPH_TYPE
{
	MORPH_ERODE,
	MORPH_DILATE,
	MORPH_OPEN,
	MORPH_CLOSE
};
enum BI_TYPE
{
	BI_FIXED,
	BI_OTSU,
    BI_IN_RANGE = 10,
	BI_OUT_RANGE,
	BI_EQUAL,
	BI_GREATER,
	BI_LESS,
	BI_NOT_EQUAL
};

enum MOMENT_TYPE
{
	M_0_0,
	M_0_1,
	M_1_0,
	M_1_1,
	M_2_0,
	M_0_2,
	M_2_1,
	M_1_2,
	M_3_0,
	M_0_3,
};
enum FILTER_TYPE
{
	FILTER_GREATER,
	FILTER_LESS,
	FILTER_EQUAL,
	FILTER_NOT_EQUAL,
	FILTER_AREA = 100
};

class ImageProcessDll: public PDLL
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(ImageProcessDll);
	//use DECLARE_FUNCTION4 since this function has 4 parameters
#pragma warning(pop)

	DECLARE_FUNCTION3(LONG, GetThreshold_Otsu_16u, const USHORT*, ULONG, USHORT&)
	DECLARE_FUNCTION4(LONG, InRange_InPlace_16u, USHORT*, USHORT, USHORT, USHORT)
	DECLARE_FUNCTION5(LONG, InRange_NotInPlace_16u, const USHORT*, USHORT*, USHORT, USHORT, USHORT)
	DECLARE_FUNCTION3(LONG, Dilate3x3_16u, USHORT*, USHORT, USHORT)
	DECLARE_FUNCTION3(LONG, Erode3x3_16u, USHORT*, USHORT, USHORT)
	DECLARE_FUNCTION6(LONG, Moments_00_01_10, USHORT*,  USHORT, USHORT, double&, double&, double&)
	DECLARE_FUNCTION6(LONG, LableMarkers, USHORT*, USHORT, USHORT, int, USHORT, int&)
	DECLARE_FUNCTION7(LONG, LableImage, USHORT*, USHORT*, USHORT, USHORT, USHORT&, USHORT, USHORT)
	DECLARE_FUNCTION5(LONG, FindContourImg, USHORT*, USHORT, USHORT, USHORT, USHORT*)
	DECLARE_FUNCTION4(LONG, MorphImage, USHORT* ,USHORT , USHORT , MORPH_TYPE )
	DECLARE_FUNCTION5(LONG, BinarizeImage, USHORT*, USHORT, USHORT, BI_TYPE, int)
	DECLARE_FUNCTION6(LONG, Moments, USHORT*, USHORT, USHORT,int, MOMENT_TYPE, double* )
	DECLARE_FUNCTION8(LONG, FilterImage, USHORT*, USHORT, USHORT, int&, FILTER_TYPE, long, int, long*)
	DECLARE_FUNCTION6(LONG, GetArea,USHORT*, USHORT, USHORT,int, long*, int)
	
};

class ImageDistortionCorrectionDll : public PDLL
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(ImageDistortionCorrectionDll);
	//use DECLARE_FUNCTION4 since this function has 4 parameters
#pragma warning(pop)
	DECLARE_FUNCTION6(LONG, SetImageDistortionCorrectionParameters, int, int, double, double, double, double);
	DECLARE_FUNCTION4(LONG, CorrectPreludeImageDistortion, const USHORT*, USHORT*, int, int);
};