namespace MCM6000_Control
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;

    #region Enumerations

    enum DeviceParams
    {
        PARAM_DEVICE_TYPE,

        PARAM_X_POS = 200,
        PARAM_X_ZERO = 202,
        PARAM_X_POS_CURRENT = 207,
        PARAM_X_STOP = 209,
        PARAM_X_POS_MOVE_BY = 225,

        PARAM_Y_POS = 300,
        PARAM_Y_ZERO = 302,
        PARAM_Y_POS_CURRENT = 307,
        PARAM_Y_STOP = 309,
        PARAM_Y_POS_MOVE_BY = 325,

        PARAM_Z_POS = 400,
        PARAM_Z_ZERO = 402,
        PARAM_Z_POS_CURRENT = 407,
        PARAM_Z_STOP = 409,
        PARAM_Z_POS_MOVE_BY = 432,
        PARAM_Z_ELEVATOR_POS_CURRENT = 433,

        PARAM_R_POS = 1200,
        PARAM_R_ZERO = 1202,
        PARAM_R_POS_CURRENT = 1207,
        PARAM_R_STOP = 1209,
        PARAM_R_POS_MOVE_BY = 1214,

        PARAM_LIGHTPATH_GG = 1850,
        PARAM_LIGHTPATH_GR,
        PARAM_LIGHTPATH_CAMERA,
    }

    enum DeviceType
    {
        DEVICE_TYPE_FIRST = 0,

        STAGE_X = 0x04,
        STAGE_Y = 0x08,
        STAGE_Z = 0x10,
        STAGE_R = 0x400000,

        DEVICE_TYPE_LAST
    }

    #endregion Enumerations

    public static class MCM6000Functions
    {
        #region Fields

        public const string DLL_NAME = ".\\ThorMCM6000.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

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