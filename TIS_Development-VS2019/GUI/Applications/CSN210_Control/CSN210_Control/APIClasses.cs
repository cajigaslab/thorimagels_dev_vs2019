

namespace CSN210_Control
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;

    #region Enumerations

    enum ChangerPositions
    {
        HOME,
        POS1,
        POS2,
        HOMING,
        MOVING_POS1,
        MOVING_POS2,
        DISCONNECTED
    };

    enum DeviceParams
    {
        PARAM_TURRET_POS = 502,
        PARAM_TURRET_SERIALNUMBER = 505,
        PARAM_TURRET_FIRMWAREUPDATE = 506,
        PARAM_TURRET_FIRMWAREVERSION = 507,
        PARAM_TURRET_STOP = 508,
        PARAM_TURRET_HOMED = 509,
        PARAM_TURRET_COLLISION = 510,
        PARAM_TURRET_POS_CURRENT = 726
    };

    enum DeviceType
    {
        DEVICE_TYPE_FIRST = 0,
        TURRET = 0x00000200,
        DEVICE_TYPE_LAST
    };

    #endregion Enumerations

    public static class CSN210Functions
    {
        #region Fields

        public const string DLL_NAME = ".\\ThorObjectiveChanger.dll";

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

        [DllImport(DLL_NAME, EntryPoint = "SetupPosition")]
        public static extern int SetupPosition();

        [DllImport(DLL_NAME, EntryPoint = "StartPosition")]
        public static extern int StartPosition();

        [DllImport(DLL_NAME, EntryPoint = "StatusPosition")]
        public static extern int StatusPosition(ref int status);

        [DllImport(DLL_NAME, EntryPoint = "TeardownDevice")]
        public static extern int TeardownDevice();

        [DllImport(DLL_NAME, EntryPoint = "GetSerialNumberString")]
        public static extern int GetSerialNumberString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] StringBuilder paramString, int size);

        #endregion Methods
    }
}
