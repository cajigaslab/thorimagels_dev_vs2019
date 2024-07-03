namespace CaptureSetupDll.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using LineProfileWindow;
    using OverlayManager;
    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetup : INotifyPropertyChanged
    {
        #region Fields

        public double[] Stats;
        public string[] StatsNames;

        private static ReportCaptureNotification _captureNotificationCallback;
        private static ReportLineProfile _lineProfileCallback;
        private static ReportMultiROIStats _multiROIStatsCallBack;

        private XmlDocument _applicationDoc;
        private Guid _commandGuid;
        private XmlDocument _experimentDoc;
        private String _expPath;
        private LineProfileData _lineProfileData;

        #endregion Fields

        #region Constructors

        public CaptureSetup()
        {
            _liveStartButtonStatus = true;
            _isBleaching = false;

            CreateCallbackHandlers();

            try
            {
                //attach to the live image data dll
                GetCommandGUID(ref _commandGuid);

                RegisterCallbackHandlers();

                //GetTotalColorChannels();

                CaptureSetupSetupCommand();
            }
            catch (System.DllNotFoundException e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " DllNotFoundException " + e.Message);
            }

            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetActiveExperimentPathAndName(sb, PATH_LENGTH);

            _expPath = sb.ToString();

            if (_expPath.Length == 0)
            {
                string templatesFolder = Directory.GetCurrentDirectory();

                if (Application.Current != null)
                {
                    templatesFolder = ResourceManagerCS.GetCaptureTemplatePathString();
                }

                _expPath = templatesFolder + "\\Active.xml";

            }
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Created");
        }

        #endregion Constructors

        #region Enumerations

        public enum ColorAssignments
        {
            RED,
            GREEN,
            BLUE,
            CYAN,
            MAGENTA,
            YELLOW,
            GRAY,
            TRANSPARENT
        }

        #endregion Enumerations

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void Report(ref int index, ref int completed, ref int total, ref int timeElapsed, ref int timeRemaining);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportCaptureNotification(ref CaptureNotification captureNotificiation);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate void ReportLineProfile(IntPtr lineProfile, int length, int realLength, int channelEnable, int numChannel);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate void ReportMultiROIStats(IntPtr statsName, IntPtr stats, ref int length, ref int isLast);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportPreCapture(ref int status);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportSubRowEndIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportSubRowStartIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportTIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportZIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportZStackCaptureFinished();

        #endregion Delegates

        #region Events

        public event EventHandler LineProfileChanged;

        public event PropertyChangedEventHandler PropertyChanged;

        public event EventHandler ROIStatsChanged;

        public event Action<bool> UpdateMenuBarButton;

        #endregion Events

        #region Properties

        public XmlDocument ApplicationDoc
        {
            get { return _applicationDoc; }
            set { _applicationDoc = value; }
        }

        public Guid CommandGuid
        {
            get { return _commandGuid; }
        }

        public XmlDocument ExperimentDoc
        {
            get
            {
                return this._experimentDoc;
            }
            set
            {
                this._experimentDoc = value;
            }
        }

        public String ExpPath
        {
            get
            {
                return _expPath;
            }

            set
            {
                _expPath = value;

                //CaptureSetupCustomParams escParams;

                //escParams.version = 1.0;

                //escParams.path = _expPath;

                //SetCustomParamsBinaryCS(ref escParams);
                //CaptureSetupExecute();
            }
        }

        public int ImageHeight
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType())
                {
                    return (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)0];
                }
                else
                {
                    return (int)MVMManager.Instance["CameraControlViewModel", "CamImageHeight", (object)1];
                }
            }
        }

        public int ImageWidth
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType())
                {
                    return (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)512];
                }
                else
                {
                    return (int)MVMManager.Instance["CameraControlViewModel", "CamImageWidth", (object)1];
                }
            }
        }

        public LineProfileData LineProfileData
        {
            get
            {
                return _lineProfileData;
            }
        }

        public int MaxChannels
        {
            get
            {
                int fMin = 0;
                int fMax = 0;
                int fDefault = 0;

                if ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType())
                {
                    GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CHANNEL, ref fMin, ref fMax, ref fDefault);
                }
                else
                {
                    GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_CHANNEL, ref fMin, ref fMax, ref fDefault);
                }

                int MAX_CHAN_BITS = 6;
                int numChan = 0;
                for (int i = 0; i < MAX_CHAN_BITS; ++i)
                {
                    if (0 != (fMax >> i) % 2)
                    {
                        ++numChan;
                    }
                }

                return numChan;
            }
        }

        public XmlDocument RegistrationDoc
        {
            get
            {
                return MVMManager.Instance.SettingsDoc[(int)SettingsFileType.REGISTRATION_SETTINGS];
            }
        }

        public double YScale
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType())
                {
                    return (double)MVMManager.Instance["ScanControlViewModel", "LSMScaleYScan", (object)1.0];
                }
                else
                {
                    return 1;
                }
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\StatsManager.dll", EntryPoint = "SetLineProfileLineWidth")]
        public static extern int SetLineProfileLineWidth(int width);

        public void ConnectHandlers()
        {
            InitCallBack(_imageCallBack, _zStackPreviewFinishedCallBack);
            InitCallBackBleach(_BleachNowFinishedCallBack, _PreBleachCallBack);
            InitCallBackBleachSLM(_BleachNowSLMFinishedCallBack, _PreBleachSLMCallBack);

            InitCallBackROIDataStore(_multiROIStatsCallBack);
            InitCallBackLineProfilePush(_lineProfileCallback);
            const int DSTYPE_PASSTHROUGH = 0;

            CreateStatsManagerROIDS(DSTYPE_PASSTHROUGH, "");

            SetCaptureNotificationCallback(_captureNotificationCallback);
        }

        //TODO:IV should remove?
        public void MarshalStrArray(IntPtr pUnStrArray, int AryCnt, out string[] StrArray)
        {
            if (AryCnt > 0)
            {
                IntPtr[] pIntPtrArray = new IntPtr[AryCnt];
                StrArray = new string[AryCnt];

                Marshal.Copy(pUnStrArray, pIntPtrArray, 0, AryCnt);

                for (int i = 0; i < AryCnt; i++)
                {
                    StrArray[i] = Marshal.PtrToStringAnsi(pIntPtrArray[i]);
                }

            }
            else
            {
                StrArray = null;
            }
        }

        public void OnPropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        public void ReleaseHandlers()
        {
            //Ensure LineProfile is not opened when capture setup is not running
            InitCallBackLineProfilePush(null);
            //Reset the data for the line profile and call the event
            //The line profile will see there is no data and will reset itself
            //This may happen if the overlay manager loads the rois before
            //we send null as the callback function
            _lineProfileData.profileDataY = new double[0][];
            _lineProfileData.profileDataX = new double[0];
            _lineProfileData.channelEnable = 0;
            if (null != LineProfileChanged)
            {
                LineProfileChanged(this, EventArgs.Empty);
            }
        }

        [DllImport(".\\StatsManager.dll", EntryPoint = "CreateStatsManagerROIDS")]
        private static extern int CreateStatsManagerROIDS(int dstype, string str);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPathAndName(StringBuilder path, int length);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamDouble")]
        private static extern int GetCameraParamDouble(int cameraSelection, int param, ref double value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamLong")]
        private static extern int GetCameraParamInt(int cameraSelection, int param, ref int value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamRangeDouble")]
        private static extern int GetCameraParamRangeDouble(int cameraSelection, int paramId, ref double valMin, ref double valMax, ref double valDefault);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamRangeLong")]
        private static extern int GetCameraParamRangeInt(int cameraSelection, int paramId, ref int valMin, ref int valMax, ref int valDefault);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetCaptureTemplatePath", CharSet = CharSet.Unicode)]
        private static extern int GetCaptureTemplatePath(StringBuilder sb, int length);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetCommandGUID")]
        private static extern int GetCommandGUID(ref Guid guid);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamBuffer")]
        private static extern int GetDeviceParamBuffer(int deviceSelection, int paramId, byte[] buf, long len);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamDouble")]
        private static extern int GetDeviceParamDouble(int deviceSelection, int paramId, ref double param);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamLong")]
        private static extern int GetDeviceParamInt(int deviceSelection, int paramId, ref int param);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamRangeDouble")]
        private static extern int GetDeviceParamRangeDouble(int deviceSelection, int paramId, ref double valMin, ref double valMax, ref double valDefault);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamRangeLong")]
        private static extern int GetDeviceParamRangeInt(int deviceSelection, int paramId, ref int valMin, ref int valMax, ref int valDefault);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetHardwareSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetHardwareSettingsFilePathAndName(StringBuilder sb, int length);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetNumberOfCameras")]
        private static extern bool GetNumberOfCameras(ref int numCameras);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "InitCallBack")]
        private static extern void InitCallBack(ReportNewImage reportNewImage, ReportZStackCaptureFinished reportZStackCaptureFinished);

        [DllImport(".\\StatsManager.dll", EntryPoint = "InitCallBackLineProfilePush")]
        private static extern void InitCallBackLineProfilePush(ReportLineProfile reportLineProfile);

        [DllImport(".\\ROIDataStore.dll", EntryPoint = "InitCallBack")]
        private static extern void InitCallBackROIDataStore(ReportMultiROIStats reportMultiROIStats);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImage")]
        private static extern bool ReadImage([MarshalAs(UnmanagedType.LPWStr)] string path, ref IntPtr outputBuffer);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo")]
        private static extern bool ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)] string selectedFileName, ref long width, ref long height, ref long colorChannels, ref long bitsPerPixel);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamDouble")]
        private static extern int SetCameraParamDouble(int cameraSelection, int param, double value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamLong")]
        private static extern int SetCameraParamInt(int cameraSelection, int param, int value);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetCaptureNotificationCallback")]
        private static extern void SetCaptureNotificationCallback(ReportCaptureNotification reportCaptureNotification);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamBuffer")]
        private static extern int SetDeviceParamBuffer(int deviceSelection, int paramId, string buf, long len, bool wait);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamDouble")]
        private static extern int SetDeviceParamDouble(int deviceSelection, int paramId, double param, bool wait);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamLong")]
        private static extern int SetDeviceParamInt(int deviceSelection, int paramId, int param, bool wait);

        private void CreateCallbackHandlers()
        {
            //create and assign callback for C++ unmanaged updates
            _imageCallBack = new ReportNewImage(ImageUpdate);
            _zStackPreviewFinishedCallBack = new ReportZStackCaptureFinished(ZStackFinished);
            //create and assign callback for C++ unmanaged updates
            _BleachNowFinishedCallBack = new ReportBleachNowFinished(BleachNowFinished);
            _PreBleachCallBack = new PreBleachCallback(BleachCallback);
            _BleachNowSLMFinishedCallBack = new ReportBleachSLMNowFinished(BleachSLMNowFinished);
            _PreBleachSLMCallBack = new PreBleachSLMCallback(BleachSLMCallback);

            _multiROIStatsCallBack = new ReportMultiROIStats(MultiROIStatsUpdate);
            _lineProfileCallback = new ReportLineProfile(LineProfileUpdate);

            _captureNotificationCallback = new ReportCaptureNotification(OnCaptureNotification);
        }

        private void LineProfileUpdate(IntPtr lineProfile, int length, int realLength, int channelEnable, int numChannel)
        {
            if (0 < numChannel && 0 < length)
            {

                if (((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType()))
                {
                    // Sending conversion factor to the line profile view
                    double pixelWidth = ((PixelSizeUM)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)null]).PixelWidthUM;
                    double lengthinµm = (double)(realLength * ((PixelSizeUM)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)null]).PixelWidthUM);
                    double roiLength = OverlayManagerClass.Instance.GetCurrentROILength();
                    if (roiLength > 0 && _lineProfileData.profileDataX != null)
                        _lineProfileData.PixeltoµmConversionFactor = roiLength / _lineProfileData.profileDataX.Length;
                    else
                        _lineProfileData.PixeltoµmConversionFactor = lengthinµm;
                }

                else if (((int)ICamera.CameraType.LSM != ResourceManagerCS.GetCameraType()))
                {
                    // Sending conversion factor to the line profile view
                    double lengthinµm = (double)(realLength * (double)MVMManager.Instance["CameraControlViewModel", "CamPixelSizeUM", (object)1]);

                    double roiLength = OverlayManagerClass.Instance.GetCurrentROILength();
                    if (roiLength > 0 && _lineProfileData.profileDataX != null)
                        _lineProfileData.PixeltoµmConversionFactor = roiLength / _lineProfileData.profileDataX.Length;
                    else
                        _lineProfileData.PixeltoµmConversionFactor = lengthinµm / length;
                }

                int lengthPerChannel = length / numChannel;

                _lineProfileData.LengthPerChannel = lengthPerChannel;

                _lineProfileData.profileDataX = new double[lengthPerChannel];
                for (int i = 0; i < lengthPerChannel; i++)
                {
                    _lineProfileData.profileDataX[i] = i;
                }

                _lineProfileData.profileDataY = new double[numChannel][];

                for (int i = 0; i < numChannel; i++)
                {

                    _lineProfileData.profileDataY[i] = new double[lengthPerChannel];

                    Marshal.Copy(lineProfile + i * lengthPerChannel * sizeof(double), _lineProfileData.profileDataY[i], 0, lengthPerChannel);
                }

                _lineProfileData.channelEnable = channelEnable;

            }
            else
            {
                _lineProfileData.profileDataY = new double[0][];
                _lineProfileData.profileDataX = new double[0];
                _lineProfileData.channelEnable = 0;
            }

            if (null != LineProfileChanged)
            {
                LineProfileChanged(this, EventArgs.Empty);
            }
        }

        private void MultiROIStatsUpdate(IntPtr statsName, IntPtr stats, ref int length, ref int isLast)
        {
            //MarshalStrArray(statsName, length, out StatsNames);
            if (length > 0)
            {
                IntPtr[] pIntPtrArray = new IntPtr[length];
                if (null == StatsNames || StatsNames.Length != length)
                {
                    StatsNames = new string[length];
                }
                Marshal.Copy(statsName, pIntPtrArray, 0, length);

                for (int i = 0; i < length; i++)
                {
                    StatsNames[i] = Marshal.PtrToStringAnsi(pIntPtrArray[i]);
                }

            }
            else
            {
                StatsNames = null;
                return;
            }
            if ((Stats == null) || (Stats.Length != length))
            {
                Stats = new double[length];
            }

            Marshal.Copy(stats, Stats, 0, StatsNames.Length);

            EventHandler handler = ROIStatsChanged;

            if (handler != null)
            {
                handler(this, EventArgs.Empty);
            }
        }

        private void OnCaptureNotification(ref CaptureNotification captureNotification)
        {
            if ((int)ICamera.CameraType.CCD == ResourceManagerCS.GetCameraType() && 0 != captureNotification.isAsyncParamUpdate)
            {
                ((ICommand)MVMManager.Instance["CameraControlViewModel", "ResyncParametersCommand"]).Execute(null);
            }
        }

        private void RegisterCallbackHandlers()
        {
            CaptureSetupSetupCommand();

            ConnectHandlers();
        }

        #endregion Methods
    }
}