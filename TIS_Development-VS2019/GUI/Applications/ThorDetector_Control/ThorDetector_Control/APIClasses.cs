namespace ThorDetector_Control
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;

    #region Enumerations

    enum DetectorBandwidths
    {
        BW_250kHz = 250000,
        BW_1MHz = 1000000,
        BW_2_5MHz = 2500000,
        BW_15MHz = 15000000,
        BW_30MHz = 30000000,
        BW_80MHz = 80000000,
        BW_200MHz = 200000000,
        BW_300MHz = 300000000
    }

    enum DeviceParams
    {
        PARAM_DEVICE_TYPE,

        PARAM_CONNECTION_STATUS = 3,

        PARAM_PMT1_GAIN_POS = 700,
        PARAM_PMT1_ENABLE = 701,
        PARAM_PMT2_GAIN_POS = 702,
        PARAM_PMT2_ENABLE = 703,
        PARAM_PMT3_GAIN_POS = 704,
        PARAM_PMT3_ENABLE = 705,
        PARAM_PMT4_GAIN_POS = 706,
        PARAM_PMT4_ENABLE = 707,

        PARAM_PMT1_SAFETY = 713,
        PARAM_PMT2_SAFETY = 714,
        PARAM_PMT3_SAFETY = 715,
        PARAM_PMT4_SAFETY = 716,

        PARAM_PMT1_BANDWIDTH_POS = 722,
        PARAM_PMT2_BANDWIDTH_POS = 723,
        PARAM_PMT3_BANDWIDTH_POS = 724,
        PARAM_PMT4_BANDWIDTH_POS = 725,

        PARAM_PMT1_GAIN_POS_CURRENT_VOLTS = 750,
        PARAM_PMT1_OUTPUT_OFFSET,
        PARAM_PMT1_OUTPUT_OFFSET_CURRENT,
        PARAM_PMT1_SERIALNUMBER,
        PARAM_PMT2_GAIN_POS_CURRENT_VOLTS,
        PARAM_PMT2_OUTPUT_OFFSET,
        PARAM_PMT2_OUTPUT_OFFSET_CURRENT,
        PARAM_PMT2_SERIALNUMBER,
        PARAM_PMT3_GAIN_POS_CURRENT_VOLTS,
        PARAM_PMT3_OUTPUT_OFFSET,
        PARAM_PMT3_OUTPUT_OFFSET_CURRENT,
        PARAM_PMT3_SERIALNUMBER,
        PARAM_PMT4_GAIN_POS_CURRENT_VOLTS,
        PARAM_PMT4_OUTPUT_OFFSET,
        PARAM_PMT4_OUTPUT_OFFSET_CURRENT,
        PARAM_PMT4_SERIALNUMBER,
        PARAM_PMT5_GAIN_POS,
        PARAM_PMT5_ENABLE,
        PARAM_PMT5_BANDWIDTH_POS,
        PARAM_PMT5_SAFETY,
        PARAM_PMT5_GAIN_POS_CURRENT_VOLTS,
        PARAM_PMT5_OUTPUT_OFFSET,
        PARAM_PMT5_OUTPUT_OFFSET_CURRENT,
        PARAM_PMT5_SERIALNUMBER,
        PARAM_PMT6_GAIN_POS,
        PARAM_PMT6_ENABLE,
        PARAM_PMT6_BANDWIDTH_POS,
        PARAM_PMT6_SAFETY,
        PARAM_PMT6_GAIN_POS_CURRENT_VOLTS,
        PARAM_PMT6_OUTPUT_OFFSET,
        PARAM_PMT6_OUTPUT_OFFSET_CURRENT,
        PARAM_PMT6_SERIALNUMBER,
        PARAM_CONNECTED_PMTS,

        PARAM_PMT1_FIRMWAREUPDATE,
        PARAM_PMT2_FIRMWAREUPDATE,
        PARAM_PMT3_FIRMWAREUPDATE,
        PARAM_PMT4_FIRMWAREUPDATE,
        PARAM_PMT5_FIRMWAREUPDATE,
        PARAM_PMT6_FIRMWAREUPDATE,

        PARAM_PMT_CLEAR_TRIP = 797,

        PARAM_PMT1_BANDWIDTH_POS_CURRENT = 920,
        PARAM_PMT2_BANDWIDTH_POS_CURRENT,
        PARAM_PMT3_BANDWIDTH_POS_CURRENT,
        PARAM_PMT4_BANDWIDTH_POS_CURRENT,
        PARAM_PMT5_BANDWIDTH_POS_CURRENT,
        PARAM_PMT6_BANDWIDTH_POS_CURRENT,

        PARAM_PMT1_SATURATIONS,
        PARAM_PMT2_SATURATIONS,
        PARAM_PMT3_SATURATIONS,
        PARAM_PMT4_SATURATIONS,
        PARAM_PMT5_SATURATIONS,
        PARAM_PMT6_SATURATIONS,

        PARAM_PMT1_DETECTOR_TYPE,
        PARAM_PMT2_DETECTOR_TYPE,
        PARAM_PMT3_DETECTOR_TYPE,
        PARAM_PMT4_DETECTOR_TYPE,
        PARAM_PMT5_DETECTOR_TYPE,
        PARAM_PMT6_DETECTOR_TYPE,

        PARAM_PMT1_OFFSET_STEP_SIZE,
        PARAM_PMT2_OFFSET_STEP_SIZE,
        PARAM_PMT3_OFFSET_STEP_SIZE,
        PARAM_PMT4_OFFSET_STEP_SIZE,
        PARAM_PMT5_OFFSET_STEP_SIZE,
        PARAM_PMT6_OFFSET_STEP_SIZE,
        PARAM_PMT1_GAIN_STEP_SIZE,
        PARAM_PMT2_GAIN_STEP_SIZE,
        PARAM_PMT3_GAIN_STEP_SIZE,
        PARAM_PMT4_GAIN_STEP_SIZE,
        PARAM_PMT5_GAIN_STEP_SIZE,
        PARAM_PMT6_GAIN_STEP_SIZE
    }

    enum DeviceType
    {
        DEVICE_TYPE_FIRST = 0,

        PMT1 = 0x00000400,
        PMT2 = 0x00000800,
        PMT3 = 0x00080000,
        PMT4 = 0x00100000,
        PMT5 = 0x04000000,
        PMT6 = 0x08000000,

        DEVICE_TYPE_LAST
    }

    #endregion Enumerations

    public static class ThorDetectorFunctions
    {
        #region Fields

        public const string DLL_NAME = ".\\ThorDetector.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] StringBuilder paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(int deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [MarshalAsAttribute(UnmanagedType.LPWStr)] string param);

        [DllImport(DLL_NAME, EntryPoint = "SetupPosition")]
        public static extern int SetupPosition();

        [DllImport(DLL_NAME, EntryPoint = "StartPosition")]
        public static extern int StartPosition();

        [DllImport(DLL_NAME, EntryPoint = "StatusPosition")]
        public static extern int StatusPosition(ref int status);

        [DllImport(DLL_NAME, EntryPoint = "TeardownDevice")]
        public static extern int TeardownDevice();

        #endregion Methods
    }
}