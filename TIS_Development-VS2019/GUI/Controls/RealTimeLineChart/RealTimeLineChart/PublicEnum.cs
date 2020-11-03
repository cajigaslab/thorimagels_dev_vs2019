#if __LINE__
#pragma once
#define public
#else
namespace RealTimeLineChart
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
#endif
    public enum HWTriggerMode
    {
        SW_FREERUN = 0,
        HW_TRIGGER_SINGLE = 1,
        HW_RETRIGGERABLE = 2,
        HW_SYNCHRONIZABLE = 3
    };

    public enum HWTriggerType
    {
        DIGITAL_PFI,
        ANALOG_INPUT
    };

    public enum SignalType
    {
        FIRST_SIGNALTYPE,
        ANALOG_IN = 0,
        DIGITAL_IN = 1,
        COUNTER_IN = 2,
        COUNTER,
        PFI,
        DIGITAL_OUT,
        VIRTUAL = 6,
        SPECTRAL = 7,
        SPECTRAL_VIRTUAL = 8,
        LAST_SIGNAL_TYPE
    };
#if __LINE__
#undef public
#else
}
#endif