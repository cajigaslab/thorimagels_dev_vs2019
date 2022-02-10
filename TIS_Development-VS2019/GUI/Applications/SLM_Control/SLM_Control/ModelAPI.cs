namespace SLM_Control.Model
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;

    #region Enumerations

    public enum DeviceParams
    {
        PARAM_FIRST_PARAM = 0,
        PARAM_DEVICE_TYPE = 0,

        PARAM_SLM_FUNC_MODE = 1130,
        PARAM_SLM_ARRAY_ID,
        PARAM_SLM_POINTS_ARRAY,
        PARAM_SLM_PIXEL_X,
        PARAM_SLM_PIXEL_Y,
        PARAM_SLM_BMP_FILENAME,
        PARAM_SLM_RESET_AFFINE,
        PARAM_SLM_BLANK,
        PARAM_SLM_TIMEOUT,
        PARAM_SLM_RUNTIME_CALC,
        PARAM_SLM_SEQ_FILENAME,
        PARAM_SLM_CALIB_Z,
        PARAM_SLM_NA,
        PARAM_SLM_WAVELENGTH,
        PARAM_SLM_WAVELENGTH_SELECT,
        PARAM_SLM_3D,
        PARAM_SLM_PHASE_DIRECT,
    }

    public enum SLMFunctionMode
    {
        LOAD_PHASE_ONLY = 0,
        PHASE_CALIBRATION,
        SAVE_PHASE,
        LAST_FUNCTION
    }

    #endregion Enumerations

    public static class SLMDeviceFuncs
    {
        #region Fields

        public const string DLL_NAME = ".\\ThorSLMPDM512.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        public static bool GetParamAvailable(int paramId)
        {
            int paramtype = 0;
            int paramavail = 0;
            int paramreadonly = 0;
            double paramMin = 0;
            double paramMax = 0;
            double paramDefault = 0;
            SLMDeviceFuncs.GetParamInfo(paramId, ref paramtype, ref paramavail, ref paramreadonly, ref paramMin, ref paramMax, ref paramDefault);
            return (1 == paramavail) ? true : false;
        }

        public static int GetParamBuffer<T>(int paramId, T[] buf, int len)
        {
            int ret = 0;
            var gch = default(GCHandle);
            try
            {
                gch = GCHandle.Alloc(buf, GCHandleType.Pinned);
                ret = GetParamBuffer(paramId, gch.AddrOfPinnedObject(), len);
            }
            finally
            {
                if (gch.IsAllocated)
                    gch.Free();
            }
            return ret;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        public static extern int GetParamBuffer(int paramID, IntPtr buf, int len);

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

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

        public static int SetParamString(int param, string value)
        {
            return SetParamString(param, new StringBuilder(value));
        }

        [DllImport(DLL_NAME, EntryPoint = "SetupPosition")]
        public static extern int SetupPosition();

        [DllImport(DLL_NAME, EntryPoint = "StartPosition")]
        public static extern int StartPosition();

        [DllImport(DLL_NAME, EntryPoint = "StatusPosition")]
        public static extern int StatusPosition(ref int status);

        [DllImport(DLL_NAME, EntryPoint = "TeardownDevice")]
        public static extern int TeardownDevice();

        [DllImport(DLL_NAME, EntryPoint = "SetParamString", CharSet = CharSet.Unicode)]
        private static extern int SetParamString(int paramID, StringBuilder value);

        #endregion Methods
    }
}