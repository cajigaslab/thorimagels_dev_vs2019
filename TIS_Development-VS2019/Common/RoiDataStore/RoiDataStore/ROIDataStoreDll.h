#include "..\..\PDLL\pdll.h"
//typedef void (_cdecl *pushROIDataCallBack)(char** statsName, double* stats, long &nStats, long &isLast);
class ROIDataStoreDll : public PDLL
{	
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(ROIDataStoreDll)
#pragma warning(pop)
	
	//DECLARE_FUNCTION1(void, InitCallBack, pushROIDataCallBack)
	DECLARE_FUNCTION2(void, CreateROIDataStore, long, char*)
	DECLARE_FUNCTION4(void, LoadROIData, char**, double*, long, long)
};
