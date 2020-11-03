
//TLBC1lib.h
#include "..\..\..\..\..\..\Common\PDLL\pdll.h"
//dll wrapper class using the virtual class
#include <visatype.h>
#include "TLBC1_Calculations.h"
#include "TLBC1_structs.h"

#if defined( _WIN32 ) || defined ( _WIN64 )
#define __STDCALL  __stdcall
#define __CDECL    __cdecl
#define __INT64    __int64
#define __UINT64    unsigned __int64
#else
#define __STDCALL
#define __CDECL
#define __INT64    long long
#define __UINT64    unsigned long long
#endif

class TLBC1lib
{
public:
	virtual ViStatus _VI_FUNC TLBC1_get_device_count (ViSession vi, ViPUInt32 device_count) = 0;
	virtual ViStatus _VI_FUNC TLBC1_get_device_information (ViSession vi, ViUInt32 device_index, ViChar* manufacturer,ViChar* model_name, ViChar* serial_number,ViPBoolean device_available,ViChar* resource_name) = 0; 
	virtual ViStatus _VI_FUNC TLBC1_init (ViRsrc rsrcName, ViBoolean id_query, ViBoolean reset_instr, ViPSession vi) = 0;
	virtual ViStatus _VI_FUNC TLBC1_set_clip_level (ViSession vi, ViReal64 clip_level) = 0;
	virtual ViStatus _VI_FUNC TLBC1_set_precision_mode (ViSession vi, ViUInt8 precision_mode) = 0;
	virtual ViStatus _VI_FUNC TLBC1_set_auto_exposure (ViSession vi, ViBoolean auto_exposure) = 0;
	virtual ViStatus _VI_FUNC TLBC1_close (ViSession vi) = 0;
	virtual ViStatus _VI_FUNC TLBC1_get_scan_data (ViSession vi, TLBC1_Calculations* const scan_data) = 0;
	virtual ViStatus _VI_FUNC TLBC1_get_exposure_time (ViSession vi, ViPReal64 exposure_time) = 0;
};

class TLBC1Dll : public PDLL, TLBC1lib
{
	//call the macro and pass your class name
	DECLARE_CLASS(TLBC1Dll)

	DECLARE_FUNCTION2(ViStatus, TLBC1_get_device_count, ViSession, ViPUInt32);

	DECLARE_FUNCTION7(ViStatus, TLBC1_get_device_information, ViSession, ViUInt32, ViChar*, ViChar*, ViChar*, ViPBoolean, ViChar*);

	DECLARE_FUNCTION4(ViStatus, TLBC1_init, ViRsrc, ViBoolean , ViBoolean, ViPSession);

	DECLARE_FUNCTION2(ViStatus, TLBC1_set_clip_level, ViSession, ViReal64);

	DECLARE_FUNCTION2(ViStatus, TLBC1_set_precision_mode, unsigned long, unsigned char);

	DECLARE_FUNCTION2(ViStatus, TLBC1_set_auto_exposure, ViSession, ViBoolean);

	DECLARE_FUNCTION1(ViStatus, TLBC1_close, ViSession);

	DECLARE_FUNCTION2(ViStatus, TLBC1_get_scan_data, ViSession, TLBC1_Calculations* const);

	DECLARE_FUNCTION2(ViStatus, TLBC1_get_exposure_time, ViSession, ViPReal64);

};