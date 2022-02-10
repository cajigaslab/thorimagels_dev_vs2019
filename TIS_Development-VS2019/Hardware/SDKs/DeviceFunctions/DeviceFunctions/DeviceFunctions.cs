using System;
using System.Text;
using System.Runtime.InteropServices;

namespace DeviceFunctions
{
    public class BCM
    {
        #region Fields

        public const string DLL_NAME = "ThorBCM.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class BCMPA
    {
        #region Fields

        public const string DLL_NAME = "ThorBCMPA.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class BScope
    {
        #region Fields

        public const string DLL_NAME = "ThorBScope.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class ECU
    {
        #region Fields

        public const string DLL_NAME = "ThorECU.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class EpiTurret
    {
        #region Fields

        public const string DLL_NAME = "ThorEpiTurret.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class LSKGR
    {
        #region Fields

        public const string DLL_NAME = "ThorLSKGR.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class MCLS
    {
        #region Fields

        public const string DLL_NAME = "ThorMCLS.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class MCM3000
    {
        #region Fields

        public const string DLL_NAME = "ThorMCM3000.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class MCM6000
    {
        #region Fields

        public const string DLL_NAME = "ThorMCM6000.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class ObjectiveChanger
    {
        #region Fields

        public const string DLL_NAME = "ThorObjectiveChanger.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class PinholeStepper
    {
        #region Fields

        public const string DLL_NAME = "ThorPinholeStepper.dll";

        #endregion Fields

        #region Methods
        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class PMT
    {
        #region Fields

        public const string DLL_NAME = "ThorPMT.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class PMT2100
    {
        #region Fields

        public const string DLL_NAME = "ThorPMT2100.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class PowerControl
    {
        #region Fields

        public const string DLL_NAME = "ThorPowerControl.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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

    public class ZStepper
    {
        #region Fields

        public const string DLL_NAME = "ThorZStepper.dll";

        #endregion Fields

        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "FindDevices")]
        public static extern int FindDevices(ref int deviceCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "GetParam")]
        public static extern int GetParam(int paramID, ref double param);

        [DllImport(DLL_NAME, EntryPoint = "GetParamBuffer")]
        private static extern int GetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int GetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            GetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "GetParamInfo")]
        public static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(DLL_NAME, EntryPoint = "GetParamString")]
        public static extern int GetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString, int size);

        [DllImport(DLL_NAME, EntryPoint = "PostflightPosition")]
        public static extern int PostflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "PreflightPosition")]
        public static extern int PreflightPosition();

        [DllImport(DLL_NAME, EntryPoint = "ReadPosition")]
        public static extern int ReadPosition(ThorSharedTypes.DeviceType deviceType, ref double pos);

        [DllImport(DLL_NAME, EntryPoint = "SelectDevice")]
        public static extern int SelectDevice(int Device);

        [DllImport(DLL_NAME, EntryPoint = "SetParam")]
        public static extern int SetParam(int paramID, double param);

        [DllImport(DLL_NAME, EntryPoint = "SetParamBuffer")]
        private static extern int SetParamBuffer(int paramID, IntPtr pBuffer, int size);

        public static int SetParamBuffer(int paramID, ref short[] Data, int size)
        {
            var dataIn = Marshal.AllocHGlobal(size);
            Data = new short[size];
            SetParamBuffer(paramID, dataIn, size);
            Marshal.Copy(dataIn, Data, 0, size);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SetParamString")]
        public static extern int SetParamString(int paramID, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] string paramString);

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