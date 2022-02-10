using System;
using System.Text;
using System.Runtime.InteropServices;

namespace CameraFunctions
{
    public class Confocal
    {
        #region Fields

        public const string DLL_NAME = "ThorConfocal.dll";

        #endregion Fields
        #region Methods

        [DllImport(DLL_NAME, EntryPoint = "CopyAcquisition")]
        private static extern int CopyAcquisition(IntPtr pDataBuffer/* char* pDataBuffer */);

        public static int CopyAcquisition(ref short[] Data, int imgSize)
        {
            var dataIn = Marshal.AllocHGlobal(imgSize);
            Data = new short[imgSize / 2];
            CopyAcquisition(dataIn);
            Marshal.Copy(dataIn, Data, 0, imgSize / 2);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }



        [DllImport(DLL_NAME, EntryPoint = "FindCameras")]
        public static extern int FindCameras(ref int cameraCount);

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

        [DllImport(DLL_NAME, EntryPoint = "PostflightAcquisition")]
        private static extern int PostflightAcquisition(IntPtr pDataBuffer  /* char* pDataBuffer */);

        public int PostflightAcquisition()
        {
            PostflightAcquisition(IntPtr.Zero);
            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "PreflightAcquisition")]
        private static extern int PreflightAcquisition(IntPtr pDataBuffer/* char* pDataBuffer */);

        public int PreflightAcquisition()
        {
            PreflightAcquisition(IntPtr.Zero);
            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SelectCamera")]
        public static extern int SelectCamera(int camera);

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

        [DllImport(DLL_NAME, EntryPoint = "SetupAcquisition")]
        private static extern int SetupAcquisition(IntPtr pDataBuffer/* char* pDataBuffer */);

        public int SetupAcquisition()
        {
            SetupAcquisition(IntPtr.Zero);
            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "StartAcquisition")]
        private static extern int StartAcquisition(IntPtr pDataBuffer/* char* pDataBuffer */);

        public int StartAcquisition()
        {
            StartAcquisition(IntPtr.Zero);
            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "StatusAcquisition")]
        public static extern int StatusAcquisition(ref int status);

        [DllImport(DLL_NAME, EntryPoint = "TeardownCamera")]
        public static extern int TeardownCamera();

        #endregion Methods

    }

    public class ConfocalGalvo
    {
        #region Fields

        public const string DLL_NAME = "ThorConfocalGalvo.dll";

        #endregion Fields

        #region Methods
        [DllImport(DLL_NAME, EntryPoint = "CopyAcquisition")]
        private static extern int CopyAcquisition(IntPtr pDataBuffer/* char* pDataBuffer */);

        public static int CopyAcquisition(ref short[] Data, int imgSize)
        {
            var dataIn = Marshal.AllocHGlobal(imgSize);
            Data = new short[imgSize / 2];
            CopyAcquisition(dataIn);
            Marshal.Copy(dataIn, Data, 0, imgSize / 2);
            Marshal.FreeHGlobal(dataIn);

            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "FindCameras")]
        public static extern int FindCameras(ref int cameraCount);

        [DllImport(DLL_NAME, EntryPoint = "GetLastErrorMsg")]
        public static extern int GetLastErrorMsg([Out, MarshalAsAttribute(UnmanagedType.LPStr)] string paramString, int size);

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

        [DllImport(DLL_NAME, EntryPoint = "PostflightAcquisition")]
        private static extern int PostflightAcquisition(IntPtr pDataBuffer  /* char* pDataBuffer */);

        public int PostflightAcquisition()
        {
            PostflightAcquisition(IntPtr.Zero);
            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "PreflightAcquisition")]
        private static extern int PreflightAcquisition(IntPtr pDataBuffer/* char* pDataBuffer */);

        public int PreflightAcquisition()
        {
            PreflightAcquisition(IntPtr.Zero);
            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "SelectCamera")]
        public static extern int SelectCamera(int camera);

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

        [DllImport(DLL_NAME, EntryPoint = "SetupAcquisition")]
        private static extern int SetupAcquisition(IntPtr pDataBuffer/* char* pDataBuffer */);

        public int SetupAcquisition()
        {
            SetupAcquisition(IntPtr.Zero);
            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "StartAcquisition")]
        private static extern int StartAcquisition(IntPtr pDataBuffer/* char* pDataBuffer */);

        public int StartAcquisition()
        {
            StartAcquisition(IntPtr.Zero);
            return 1;
        }

        [DllImport(DLL_NAME, EntryPoint = "StatusAcquisition")]
        public static extern int StatusAcquisition(ref int status);

        [DllImport(DLL_NAME, EntryPoint = "TeardownCamera")]
        public static extern int TeardownCamera();

        #endregion Methods

    }
}
