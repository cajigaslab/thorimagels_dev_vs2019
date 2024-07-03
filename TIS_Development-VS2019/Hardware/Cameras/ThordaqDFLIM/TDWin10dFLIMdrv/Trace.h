/*++

Module Name:

    Trace.h

Abstract:

    Header file for the debug tracing related function defintions and macros.

Environment:

    Kernel mode

--*/

//
// Define the tracing flags.
//
// Tracing GUID - 0d61673a-91d5-4e21-9742-3dfbb9dfb87f
//
/*
#define WPP_CONTROL_GUIDS                                              \
    WPP_DEFINE_CONTROL_GUID(                                           \
        TDWin10dFLIMdrvTraceGuid, (0d61673a,91d5,4e21,9742,3dfbb9dfb87f), \
                                                                            \
        WPP_DEFINE_BIT(MYDRIVER_ALL_INFO)                              \
        WPP_DEFINE_BIT(TRACE_DRIVER)                                   \
        WPP_DEFINE_BIT(TRACE_DEVICE)                                   \
        WPP_DEFINE_BIT(TRACE_QUEUE)                                    \
        )                             

#define WPP_FLAG_LEVEL_LOGGER(flag, level)                                  \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)                                 \
    (WPP_LEVEL_ENABLED(flag) &&                                             \
     WPP_CONTROL(WPP_BIT_ ## flag).Level >= level)

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
           WPP_LEVEL_LOGGER(flags)
               
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

//           
// WPP orders static parameters before dynamic parameters. To support the Trace function
// defined below which sets FLAGS=MYDRIVER_ALL_INFO, a custom macro must be defined to
// reorder the arguments to what the .tpl configuration file expects.
//
#define WPP_RECORDER_FLAGS_LEVEL_ARGS(flags, lvl) WPP_RECORDER_LEVEL_FLAGS_ARGS(lvl, flags)
#define WPP_RECORDER_FLAGS_LEVEL_FILTER(flags, lvl) WPP_RECORDER_LEVEL_FLAGS_FILTER(lvl, flags)

//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC Trace{FLAGS=MYDRIVER_ALL_INFO}(LEVEL, MSG, ...);
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
// end_wpp
//
*/
 

#ifdef DBG
extern unsigned int DbgComponentID;
// To use Debug Print open Regedit on the machine running the drivers.
//  Goto HKLM\SYSTEM\CurrentControlSet\Control\Session Manager
//    if the "Debug Print Filter" key does not exist create it.
//    In the HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter 
//       section add a 32 bit unsigned long value called "IHVDRIVER" or "IHVNETWORK" for NDIS
//       drivers.  Set the value to:
//			1 for ERRORs only,
//			2 for ERRORs and WARNINGs,
//			3 for ERRORs, WARNINGs and Info,
//			4 to receive all messages
//
#define	DEBUG_VERBOSE	DPFLTR_INFO_LEVEL + 1
#define	DEBUG_INFO		DPFLTR_TRACE_LEVEL
#define	DEBUG_TRACE		DPFLTR_INFO_LEVEL
#define	DEBUG_WARN		DPFLTR_WARNING_LEVEL
#define	DEBUG_ERROR		DPFLTR_ERROR_LEVEL
#define	DEBUG_ALWAYS	DPFLTR_ERROR_LEVEL

#define DEBUGP(level, ...) \
{\
	DbgPrintEx(DbgComponentID, level, "dFLIM.SYS:"); \
    DbgPrintEx(DbgComponentID, level, __VA_ARGS__); \
	DbgPrintEx(DbgComponentID, level, "\n"); \
} 

#else // Not debug version

// Make DEBUGP macros null 
#define	DEBUGP(level, ...)

#endif // Debug vs. non debug versions.

