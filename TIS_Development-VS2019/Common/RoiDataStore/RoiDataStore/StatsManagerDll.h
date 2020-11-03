#include "..\..\PDLL\pdll.h"

class StatsManagerDll : public PDLL
{
	DECLARE_CLASS(StatsManagerDll)
	DECLARE_FUNCTION1(void, InitCallBack, pullROIDataCallBack)
};
