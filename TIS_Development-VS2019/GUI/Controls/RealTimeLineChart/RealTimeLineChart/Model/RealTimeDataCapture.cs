namespace RealTimeLineChart.Model
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;

    using ThorLogging;

    class RealTimeDataCapture
    {
        #region Fields

        private static readonly RealTimeDataCapture _instance = new RealTimeDataCapture();

        private StringBuilder _errMsg = null;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Prevents a default instance of the <see cref="RealTimeDataCapture"/> class from being created.
        /// </summary>
        RealTimeDataCapture()
        {
            _errMsg = new StringBuilder(256);
        }

        #endregion Constructors

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void ReportNewData(ref CompoundDataStruct compoundData);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void ReportNewSpectral(ref SpectralDataStruct spectralData);

        #endregion Delegates

        #region Properties

        /// <summary>
        /// Gets the instance RealTimeDataCapture.
        /// </summary>
        /// <value>
        /// The instance RealTimeDataCapture.
        /// </value>
        public static RealTimeDataCapture Instance
        {
            get
            {
                return _instance;
            }
        }

        /// <summary>
        /// Gets the error massage.
        /// </summary>
        /// <value>
        /// The error massage.
        /// </value>
        public StringBuilder ErrMessage
        {
            get { return _errMsg; }
        }

        #endregion Properties

        #region Methods

        public bool CreateCallback(ReportNewSpectral fCallback, ReportNewData dCallback)
        {
            try
            {
                bool ret = true;
                if (0 == InitCallBack(fCallback, dCallback))
                {
                    ret = false;
                }
                return ret;
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart Error CreateCallback" + ex.Message);
                return false;
            }
        }

        /// <summary>
        /// Enters the acquisition.
        /// </summary>
        /// <returns></returns>
        public bool EnterAcquisition()
        {
            try
            {
                bool ret = true;
                if (0 == EnterAcquire())
                {
                    ret = false;
                }
                return ret;
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart Error EnterAcquisition" + ex.Message);
                return false;
            }
        }

        /// <summary>
        /// Exits the acquisition.
        /// </summary>
        /// <returns></returns>
        public bool ExitAcquisition()
        {
            try
            {
                bool ret = true;
                if (0 == ExitAcquire())
                {
                    ret = false;
                }
                return ret;
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart Error EnterAcquisition" + ex.Message);
                return false;
            }
        }

        /// <summary>
        /// Gets the data.
        /// </summary>
        /// <param name="CompDataStruct">The comp data structure.</param>
        /// <returns></returns>
        public bool GetData(ref CompoundDataStruct CompDataStruct)
        {
            return (1 == GetStructData(ref CompDataStruct)) ? true : false;
        }

        /// <summary>
        /// Gets the error.
        /// </summary>
        /// <param name="error">The error.</param>
        /// <param name="msgLength">Length of the MSG.</param>
        public void GetError(StringBuilder error, int msgLength)
        {
            GetErrorMessage(error, msgLength);
        }

        /// <summary>
        /// Gets the saving.
        /// </summary>
        /// <returns></returns>
        public bool GetSaving()
        {
            return (1 == IsFileSaving()) ? true : false;
        }

        /// <summary>
        /// Determines whether this instance is acquiring.
        /// </summary>
        /// <returns></returns>
        public bool IsAcquiring()
        {
            return (1 == IsInAcquire()) ? true : false;
        }

        /// <summary>
        /// Determines whether [is asynchronous acquiring].
        /// </summary>
        /// <returns></returns>
        public bool IsAsyncAcquiring()
        {
            return (1 == IsInAsyncAcquire()) ? true : false;
        }

        /// <summary>
        /// Determines whether this instance is loading file.
        /// </summary>
        /// <returns></returns>
        public bool IsLoading()
        {
            return (1 == IsFileLoading()) ? true : false;
        }

        public bool LoadDataFromFile()
        {
            try
            {
                return (0 == LoadEpisode()) ? false : true;
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart Error LoadEpisode" + ex.Message);
                return false;
            }
        }

        public bool LoadSpectral()
        {
            try
            {
                return (0 == SpectralAnalysis()) ? false : true;
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart Error SpectralAnalysis" + ex.Message);
                return false;
            }
        }

        /// <summary>
        /// Sets the saving Configuration.
        /// </summary>
        /// <param name="tosave">if set to <c>true</c> [tosave].</param>
        public void SetSaving(bool tosave)
        {
            SetFileSaving((true == tosave) ? 1 : 0);
        }

        /// <summary>
        /// Starts the acquire.
        /// </summary>
        /// <returns></returns>
        public bool StartAcquire()
        {
            string showMsg;
            try
            {
                bool ret = true;
                if (0 == StartAcquireData())
                {
                    GetError(ErrMessage, 256);
                    showMsg = "Start Acquire Failed: "  + ErrMessage + " Please check configuration or contact customer support to enable ThorSync.";
                    MessageBox.Show(showMsg, "Settings Error");
                    ret = false;
                }
                return ret;
            }
            catch (Exception ex)
            {
                GetError(ErrMessage, 256);
                showMsg = "Start Acquire Failed: " + ErrMessage + " Please check configuration or contact customer support to enable ThorSync.";
                MessageBox.Show(ex.Message);
                return false;
            }
        }

        /// <summary>
        /// Starts the asynchronous acquire.
        /// </summary>
        /// <returns></returns>
        public bool StartAsyncAcquire()
        {
            string showMsg;
            try
            {
                bool ret = true;
                if (0 == StartAsyncAcquireData())
                {
                    GetError(ErrMessage, 256);
                    showMsg = "Start Async Acquire Failed: " + ErrMessage + " Please check configuration or contact customer support to enable ThorSync.";
                    MessageBox.Show(showMsg, "Settings Error");
                    ret = false;
                }
                return ret;
            }
            catch (Exception ex)
            {
                GetError(ErrMessage, 256);
                showMsg = "Start Async Acquire Failed: " + ErrMessage + " Please check configuration or contact customer support to enable ThorSync.";
                MessageBox.Show(showMsg, ex.Message);
                return false;
            }
        }

        /// <summary>
        /// Stops the acquire.
        /// </summary>
        public void StopAcquire()
        {
            try
            {
                StopAcquireData();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Stop Acquire Failed. ({0})", ex.Message);
            }
        }

        /// <summary>
        /// Stops the asynchronous acquire.
        /// </summary>
        public void StopAsyncAcquire()
        {
            try
            {
                StopAsyncAcquireData();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Stop Async Acquire Failed. ({0})", ex.Message);
            }
        }

        public void StopLoading()
        {
            StopFileLoading();
        }

        public void UpdateVariables()
        {
            try
            {
                UpdateVariable();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart Error UpdateVariables" + ex.Message);
            }
        }

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "EnterAcquire")]
        private static extern int EnterAcquire();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "ExitAcquire")]
        private static extern int ExitAcquire();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "GetErrorMessage", CharSet = CharSet.Unicode)]
        private static extern int GetErrorMessage(StringBuilder errMessage, int length);

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "GetStructData")]
        private static extern int GetStructData(ref CompoundDataStruct CompDataStruct);

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "InitCallBack")]
        private static extern int InitCallBack(ReportNewSpectral fCallback, ReportNewData dCallback);

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "IsFileLoading")]
        private static extern int IsFileLoading();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "IsFileSaving")]
        private static extern int IsFileSaving();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "IsInAcquire")]
        private static extern int IsInAcquire();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "IsInAsyncAcquire")]
        private static extern int IsInAsyncAcquire();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "LoadEpisode")]
        private static extern int LoadEpisode();

        //[DllImport("ThorRealTimeData.dll", EntryPoint = "PauseAcquireData")]
        //private static extern int PauseAcquireData();
        //[DllImport("ThorRealTimeData.dll", EntryPoint = "RestartAcquireData")]
        //private static extern int RestartAcquireData();
        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "SetFileSaving")]
        private static extern int SetFileSaving(Int32 save);

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "SpectralAnalysis")]
        private static extern int SpectralAnalysis();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "StartAcquireData")]
        private static extern int StartAcquireData();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "StartAsyncAcquireData")]
        private static extern int StartAsyncAcquireData();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "StopAcquireData")]
        private static extern int StopAcquireData();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "StopAsyncAcquireData")]
        private static extern int StopAsyncAcquireData();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "StopFileLoading")]
        private static extern void StopFileLoading();

        [DllImport(".\\Modules_Native\\ThorRealTimeData.dll", EntryPoint = "UpdateVariable")]
        private static extern void UpdateVariable();

        #endregion Methods

        #region Nested Types

        /// <summary>
        /// Identical data structure defined in ThorRealTimeData.dll.
        /// global counter (gCtr64) with size (gcLengthCom) to serve as timing (x-axis), 
        /// multiple analog input channels (aiData) with size of aiLength (length of each channel x channel numbers),
        /// multiple digital input channels (diData) with size of diLength (length of each channel x channel numbers),
        /// edge counting channel (ciData64) with size (ciLengthCom).
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct CompoundDataStruct
        {
            public UInt64 gcLength;
            public UInt64 aiLength;
            public UInt64 diLength;
            public UInt64 ciLength;
            public UInt64 viLength;
            public IntPtr aiData;
            public IntPtr diData;
            public IntPtr ciData;
            public IntPtr viData;
            public IntPtr gCtr64;
        }

        /// <summary>
        /// Identical data structure defined in ThorRealTimeData.dll.
        /// time range data (timeData) with size (freqLength) to serve as frequencies (x-axis) after conversion, 
        /// multiple spectral channels (specData) with size of specDataLength (length of each channel x channel numbers),
        /// multiple virtual spectral channels (vSpecData) with size of vspecDataLength (length of each channel x channel numbers),
        /// time range fit data (timeFitData) with size (freqFitLength) to serve as fitting frequencies (x-axis),
        /// multiple spectral fit channels (specFitData) with size of specFitDataLength (length of each channel x channel numbers).
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct SpectralDataStruct
        {
            public UInt64 freqLength;
            public UInt64 freqFitLength;
            public UInt64 specDataLength;
            public UInt64 vspecDataLength;
            public IntPtr freqData;
            public IntPtr specDataRe;
            public IntPtr specDataIm;
            public IntPtr vSpecData;
            public IntPtr freqFitData;
        }

        #endregion Nested Types
    }
}