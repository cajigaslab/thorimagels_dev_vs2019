namespace RunSampleLSDll.ViewModel
{
    using System;
    using System.Collections;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Data;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.ServiceModel;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Timers;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Interop;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Linq;

    using CustomMessageBox;

    using DatabaseInterface;

    using GeometryUtilities;

    using MesoScan.Params;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class RunSampleLS : INotifyPropertyChanged
    {
        #region Fields

        public const int MAX_CHANNELS = 4;

        public ObservableCollection<string> _dynamicLabels = new ObservableCollection<string>();

        private const int DEFAULT_BITS_PER_PIXEL = 14;
        private const int TOTAL_PATH_LENGTH = 111;
        private const int WAITUNIT_MS = 5000;

        private static readonly object _dflimHistogramDataLock = new object();

        readonly FrameData _frameData = new FrameData();

        private static PixelSizeUM _camPixelSizeUM;
        private static int _dataLength;
        private static uint[] _dflimArrivalTimeSumData;
        private static double[] _dFLIMBinDurations = new double[MAX_CHANNELS];
        private static uint[][] _dflimHistogramData;
        private static bool _dflimNewHistogramData;
        private static ushort[] _dflimSinglePhotonData;
        private static int _lsmChannel;
        private static bool[] _lsmEnableChannel;
        private static PixelSizeUM _lsmPixelSizeUM;
        private static int _maxChannels;
        private static ReportMultiROIStats _multiROIStatsCallBack;
        private static ushort[] _pixelData;
        private static ReportAutoFocusStatus _reportAutoFocusStatus;
        private static Report _reportCallBack;
        private static ReportIndex _reportCallBackImage;
        private static ReportInformMessage _reportInformMessage;
        private static ReportPreCapture _reportPreCaptureCallBack;
        private static ReportSavedFileIPC _reportSavedFileIPC;
        private static ReportSequenceStepCurrentIndex _reportSequenceStepCurrentIndexCallBack;
        private static ReportSubRowEndIndex _reportSubRowEndCallBack;
        private static ReportSubRowStartIndex _reportSubRowStartCallBack;
        private static ReportTIndex _reportTCallBack;
        private static ReportZIndex _reportZCallBack;
        private static string _statusMessage = string.Empty;

        private int SC_MONITORPOWER = 0xF170; //Using the system pre-defined MSDN constants that can be used by the SendMessage() function .
        private int WM_SYSCOMMAND = 0x0112;

        //private bool _afterSLMCycle = false; //if item found after a SLM repeat or cycle
        private int _averageMode;
        private int _averageNum;
        BackgroundWorker _backgroundWorker = new BackgroundWorker();
        private bool _backgroundWorkerDone;
        int _binX = 1;
        int _binY = 1;
        private int _bleachFrameIndex = 0;
        private int _bleachPixelArrayIndex = 0;
        private List<PixelArray> _bleachPixelArrayList = new List<PixelArray>();
        private int _bleachPixelIndex = 0;
        private BleachMode _bleachScanMode;
        private int _camBitsPerPixel = 14;
        private bool _cbZSEnable;
        private int _channelOrderCurrentIndex = 0;
        private int _channelOrderCurrentLSMChannel = 0;
        int _channelSelection = 0;
        private Guid _commandGuid;
        private int _completedImageCount;
        private int _currentImageCount;
        private int _currentSLMCycleID = 0;
        private int _currentSLMSequenceID = 0;
        private int _currentSLMWaveID = 0;
        private int _currentSubImageCount;
        private int _currentTCount;
        private int _currentWellCount;
        private int _currentZCount;
        private bool _displayImage = false;
        private int _dmaFrames = 4;
        private bool _experimentCompleteForProgressBar = false;
        private XmlDocument _experimentDoc;
        private string _experimentFolderPath;
        private FileName _experimentName = new FileName("FileName");
        private string _experimentStatus;
        private string _experimentXMLPath;
        private int _extTrigger;
        private bool _fastZStaircase = false;
        private int _flybackLines = 130;
        private double _flybackTimeAdjustMS = 0;
        private int _foundChannelCount = 0;
        private XmlDocument _hardwareDoc;
        private int _imageColorChannels;
        private int _imageCounter;
        private IntPtr _imageData;
        private int _imageHeight;
        private char[] _imagePath;
        private Visibility _imageUpdaterVis;
        private int _imageWidth;
        private int _indexDigitCounts = 3;
        bool _ismROICapture = false;
        private bool _isSaving;
        private bool _isSequentialCapture = false;
        private bool _lastInSLMCycle = false;
        private int _leftLabelCount;
        private List<string> _loadedSLMFiles = new List<string>();
        private List<string> _loadedSLMPatterns = new List<string>();
        private int _loadedSLMPatternsCnt = 0;
        private List<string> _loadedSLMSequences = new List<string>();
        private double _mCLS1Power = 0;
        private Visibility _mCLS1Visibility = Visibility.Collapsed;
        private double _mCLS2Power = 0;
        private Visibility _mCLS2Visibility = Visibility.Collapsed;
        private double _mCLS3Power = 0;
        private Visibility _mCLS3Visibility = Visibility.Collapsed;
        private double _mCLS4Power = 0;
        private Visibility _mCLS4Visibility = Visibility.Collapsed;
        private int _mosaicColumnCount;
        private int _mosaicRowCount;
        FULLFOVMetadata _mROIFullFOVMetadata = new FULLFOVMetadata();
        double _mROIPixelSizeXUM = 1;
        double _mROIPixelSizeYUM = 1;
        ObservableCollection<ScanArea> _mROIs;
        double _mROIStripLength = 1;
        private bool _newImageAvailable = false;
        private int _numberOfImageTiles = 1;
        private string _outputPath;
        private bool _panelsEnable;
        private double _photobleachDurationSec;
        private int _photobleachEnable;
        private int _photobleachLaserPosition;
        private double _photobleachPowerPosition;
        private int _pinholePosition;
        private Visibility _pinholeVisibility = Visibility.Collapsed;
        private int _pixelX;
        private int _pixelY;
        private int _pmtTripCount;
        private double _power0 = 0.0;
        private double _power1 = 0.0;
        private double _power2 = 0.0;
        private double _power3 = 0.0;
        private double _power4 = 0.0;
        private double _power5 = 0.0;
        private int _previewIndex;
        private int _previousSubImageCount;
        private int _rawDataCapture;
        private int _rawDataCaptureStreamingValue;
        private int _remoteFocusCaptureMode = 0;
        private string _remoteFocusCustomOrder = string.Empty;
        private ObservableCollection<double> _remoteFocusCustomSelectedPlanes = new ObservableCollection<double>();
        private double _remoteFocusStartPosition = 1;
        private int _remoteFocusStepSize = 1;
        private double _remoteFocusStopPosition = 16;
        private RunSampleLSCustomParams _rsParams = new RunSampleLSCustomParams();
        private bool _runComplete;
        private int _runCompleteCount;
        System.Timers.Timer _runPlateTimer;
        private string _sampleConfig;
        StringBuilder _sbNewDir;
        private int _scanMode;
        private string _selectedWavelengthValue;
        private int _sequenceStepsNum = 0;
        private List<string> _sequenceStepWavelengthNameList = new List<string>();
        int _simultaneousBleachingAndImaging = 0;
        private double[] _slmBleachDelay = { 0.0, 0.0 };
        private ObservableCollection<SLMParams> _slmBleachWaveParams = new ObservableCollection<GeometryUtilities.SLMParams>();
        private int _SLMCallbackCount = 0;
        private List<string> _slmFilesInFolder;
        private BackgroundWorker _slmLoader = new BackgroundWorker(); //Each event is unregistered before being registered to prevent duplicates
        private List<string> _slmSequencesInFolder;
        private bool _startButtonStatus;
        private DateTime _startDateTime;
        private DateTime _startUTC;
        private long _startZTime = 0;
        private string _statusMessageZ = string.Empty;
        private bool _statusMessageZUpdate = true;
        private double _stepTimeAdjustMS = 0;
        private bool _stopButtonStatus;
        private bool _stopClicked = false;
        private int _streamEnable;
        private int _streamFrames;
        int _streamingDisplayRollingAverage = 0;
        private string _streamingPath;
        private int _streamVolumes;
        private bool _tbZSEnable;
        private Brush _tbZSEnableFGColor;
        private bool _tEnable;
        private int _tFrames;
        private bool _tiffCompressionEnabled;
        private bool _tileDisplay = false;
        private int _tileHeight;
        private int _tileWidth;
        private double _tInterval;
        private int _topLabelCount;
        private int _totalImageCount;
        private int _totalImagesCompleted = 0;
        private int _triggerModeStreaming;
        private int _triggerModeTimelapse;
        private int _useReferenceVoltageForFastZPockels = 0;
        private double _volumeTimeAdjustMS = 0;
        private ArrayList _wavelengthList;
        private bool _z2StageLock;
        private bool _z2StageMirror;
        private int _zEnable;
        private bool _zFastEnable = false;
        private int _zFileEnable;
        private List<double> _zFilePosList;
        private int _zFilePosRead;
        private double _zFilePosScale;
        private bool _zInvert = false;
        private int _zNumSteps;
        private double _zStartPosition;
        private double _zStepSize;
        private double _zStopPosition;
        private bool _zStreamEnable;
        private int _zStreamFrames;

        #endregion Fields

        #region Constructors

        public RunSampleLS()
        {
            _startDateTime = DateTime.Now;
            _startUTC = DateTime.UtcNow;
            _experimentStatus = string.Empty;
            _sampleConfig = string.Empty;
            _statusMessage = string.Empty;
            _experimentName.NameWithoutNumber = "Untitled";
            _experimentName.NameNumberInt = 1;
            _outputPath = GetPreviousExperimentOutputPath(); //initialize to parent folder of previous experiment
            _completedImageCount = 0;
            _startButtonStatus = true;
            _totalImageCount = 96;//initialize to nonzero so the progress bar does not shows at the beginning
            _currentSubImageCount = 1;
            _currentZCount = 0;
            _currentTCount = 0;
            _currentWellCount = 1;
            _lsmPixelSizeUM = new PixelSizeUM(1.0, 1.0);
            _camPixelSizeUM = new PixelSizeUM(1.0, 1.0);
            _panelsEnable = true;
            _runComplete = true;
            _displayImage = true;
            _imageUpdaterVis = Visibility.Visible;
            _selectedWavelengthValue = "";
            _zEnable = 1;
            _tEnable = true;
            _backgroundWorkerDone = true;
            _maxChannels = 4;
            _lsmEnableChannel = new bool[MAX_CHANNELS];

            _tiffCompressionEnabled = true;
            //_tbZSEnable = false;
            _tbZSEnableFGColor = Brushes.Gray;
            // _zStreamFrames = 1;
            _pmtTripCount = 0;
            //Image _counter for determining the end of the row
            _imageCounter = 0;

            //create and assign callback for C++ unmanaged updates
            _reportCallBack = new Report(RunSampleLSUpdate);
            _reportCallBackImage = new ReportIndex(RunSampleLSUpdateStart);
            _reportSubRowStartCallBack = new ReportSubRowStartIndex(RunSampleLSUpdateMosaicStart);
            _reportSubRowEndCallBack = new ReportSubRowEndIndex(RunSampleLSUpdateMosaicEnd);
            _reportZCallBack = new ReportZIndex(RunSampleLSUpdateZ);
            _reportTCallBack = new ReportTIndex(RunSampleLSUpdateT);
            _multiROIStatsCallBack = new ReportMultiROIStats(MultiROIStatsUpdate);
            _reportPreCaptureCallBack = new ReportPreCapture(RunSampleLSUpdatePreCapture);
            _reportSequenceStepCurrentIndexCallBack = new ReportSequenceStepCurrentIndex(RunsampleLSUpdateSequenceStepCurrentIndx);
            _reportInformMessage = new ReportInformMessage(RunsampleLSUpdateMessage);
            _reportSavedFileIPC = new ReportSavedFileIPC(RunsampleLSNotifySavedFileToIPC);
            _reportAutoFocusStatus = new ReportAutoFocusStatus(AutoFocusStatusMessage);

            try
            {
                GetCommandGUID(ref _commandGuid);
                RunSampleLSSetupCommand();
                ConnectCallbacks();

                GetCustomParamsBinary(ref _rsParams);

            }
            catch (System.DllNotFoundException)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " DllNotFoundException");
            }

            _experimentXMLPath = ActiveExperimentPath = ResourceManagerCS.GetCaptureTemplatePathString() + "Active.xml";

            _zFilePosList = new List<double>();
            _zFilePosRead = 0;

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
            GRAY
        }

        #endregion Enumerations

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void Report(ref int index, ref int completed, ref int total, ref int timeElapsed, ref int timeRemaining, ref int captureComplete);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportAutoFocusStatus(ref int isRunning, ref int bestScore, ref double bestZPos, ref double nextZPos, ref int currRepeatatus);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate void ReportInformMessage(string index);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate void ReportLineProfile(IntPtr lineProfile, int length, int realLength, int channelEnable, int numChannel);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate void ReportMultiROIStats(IntPtr statsName, IntPtr stats, ref int length, ref int isLast);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportPreCapture(ref int status);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate void ReportSavedFileIPC(string index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportSequenceStepCurrentIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportSubRowEndIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportSubRowStartIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportTIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportZIndex(ref int index, ref double power0, ref double power1, ref double power2, ref double power3, ref double power4, ref double power5);

        #endregion Delegates

        #region Events

        public event Action CaptureComplete;

        public event Action ExperimentStarted;

        public event PropertyChangedEventHandler PropertyChanged;

        public event EventHandler ROIStatsChanged;

        public event Action<string> Update;

        public event Action<bool> UpdateBitmapTimer;

        public event Action<bool> UpdateButton;

        public event Action<string> UpdateEndSubWell;

        public event Action<string> UpdateImageName;

        public event Action<bool> UpdateMenuBarButton;

        public event Action<bool> UpdatePanels;

        public event Action<string> UpdateScript;

        public event Action UpdateSequenceStepDisplaySettings;

        public event Action<string> UpdateStart;

        public event Action<string> UpdateStartSubWell;

        #endregion Events

        #region Properties

        public static ICamera.CameraType CameraType
        {
            get
            {
                int cameraType = 1;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_TYPE, ref cameraType);
                return (ICamera.CameraType)cameraType;
            }
        }

        public static int ImageMethod
        {
            get
            {
                int imageMethod = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FRAME_TYPE, ref imageMethod);
                return imageMethod;
            }
        }

        public static ICamera.LSMType LSMType
        {
            get
            {
                if (ICamera.CameraType.LSM == CameraType)
                {
                    int lsmType = 1;
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);
                    return (ICamera.LSMType)lsmType;
                }
                else
                {
                    return ICamera.LSMType.LSMTYPE_LAST;
                }
            }
        }

        public int ActiveCameraType
        {
            get
            {
                int cameraType = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_TYPE, ref cameraType);
                return cameraType;
            }
        }

        public string ActiveExperimentPath
        {
            get;
            set;
        }

        public int AverageMode
        {
            get { return _averageMode; }
            set { _averageMode = value; }
        }

        public int AverageNum
        {
            get { return _averageNum; }
            set { _averageNum = value; }
        }

        public int BinX
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == this.ActiveCameraType)
                {
                    return 1;
                }
                else
                {
                    return _binX;
                }
            }
            set
            {
                _binX = value;
            }
        }

        public int BinY
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == this.ActiveCameraType)
                {
                    return 1;
                }
                else
                {
                    return _binY;
                }
            }
            set
            {
                _binY = value;
            }
        }

        public int BitsPerPixel
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == this.ActiveCameraType)
                {
                    const int DEFAULT_BITS_PER_PIXEL = 14;
                    return DEFAULT_BITS_PER_PIXEL;
                }
                else
                {
                    return _camBitsPerPixel;
                }
            }
        }

        public int BleachFrames
        {
            get;
            set;
        }

        public DateTime BleachLastRunTime
        {
            get;
            set;
        }

        public double BleachLongIdle
        {
            get;
            set;
        }

        public int BleachPostTrigger
        {
            get;
            set;
        }

        public BleachMode BleachScanMode
        {
            get { return _bleachScanMode; }
            set { _bleachScanMode = value; }
        }

        /// <summary>
        /// BleachTrigger(0: SW, 1: TRIGGER_FIRST, 2:TRIGGER_EACH, 3:TRIGGER_CONT)
        /// </summary>
        public int BleachTrigger
        {
            get;
            set;
        }

        public double BleachUMPerPixel
        {
            get
            {
                XmlDocument expDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
                string str = string.Empty;
                double dVal = 1, mag = 1, bfieldSize = 1, bfieldSizeCalibration = 1, bPixelX = 1;
                if (null != expDoc)
                {
                    XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/Magnification");
                    if (ndList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(ndList[0], expDoc, "mag", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                        {
                            mag = dVal;
                        }
                    }
                    ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, ref bfieldSize);
                    ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE_CALIBRATION, ref bfieldSizeCalibration);
                    ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_PIXEL_X, ref bPixelX);
                    dVal = ((bfieldSize * bfieldSizeCalibration) / (mag * bPixelX));
                    dVal = Math.Round(dVal, 3);
                }
                return dVal;
            }
        }

        public List<BleachWaveParams> BleachWParams
        {
            get;
            set;
        }

        public int CamImageHeight
        {
            get;
            set;
        }

        public int CamImageWidth
        {
            get;
            set;
        }

        public PixelSizeUM CamPixelSizeUM
        {
            get => _camPixelSizeUM;
            set => _camPixelSizeUM = value;
        }

        public int CaptureMode
        {
            get;
            set;
        }

        public bool CBZSEnable
        {
            get { return _cbZSEnable; }
            set { _cbZSEnable = value; }
        }

        public int ChannelSelection
        {
            get
            {
                return _channelSelection;
            }
            set
            {
                _channelSelection = value;
            }
        }

        public Guid CommandGuid
        {
            get { return _commandGuid; }
        }

        public int CompletedImageCount
        {
            get { return _completedImageCount; }
        }

        public BleachMode CurrentBleachMode
        {
            get
            {
                BleachScanMode = ResourceManagerCS.Instance.GetBleachMode;
                return BleachScanMode;
            }
        }

        public int CurrentImageCount
        {
            get { return _currentImageCount; }
        }

        public int CurrentSubImageCount
        {
            get { return _currentSubImageCount; }
        }

        public int CurrentTCount
        {
            get { return _currentTCount; }
            set { _currentTCount = value; }
        }

        public int CurrentWellCount
        {
            get { return _currentWellCount; }
        }

        public int CurrentZCount
        {
            get { return _currentZCount; }
            set { _currentZCount = value; }
        }

        public uint[][] DFLIMHistogramData
        {
            get
            {
                return _dflimHistogramData;
            }
        }

        public object DFLIMHistogramDataLock
        {
            get
            {
                return _dflimHistogramDataLock;
            }
        }

        public bool DFLIMNewHistogramData
        {
            get
            {
                return _dflimNewHistogramData;
            }
            set
            {
                _dflimNewHistogramData = value;
            }
        }

        public bool DisplayImage
        {
            get
            {
                return _displayImage;
            }
            set
            {
                _displayImage = value;
            }
        }

        public bool DisplayTile
        {
            get { return _tileDisplay; }
            set { _tileDisplay = value; }
        }

        public int DMAFrames
        {
            get
            {
                return _dmaFrames;
            }
            set
            {
                if (value < 4)
                {
                    _dmaFrames = 4;
                    MessageBox.Show("The minimum amount of RAM Frames is 4.");
                }
                else
                {
                    _dmaFrames = value;
                }
            }
        }

        public string DMAFramesTime
        {
            get
            {

                double totalSecTime = 0;

                string str = string.Empty;

                int averageFrames = 1;

                if (1 == _averageMode)
                {
                    averageFrames = _averageNum;
                }

                string formattedTime = string.Empty;
                if (0 != FrameRate)
                {
                    totalSecTime = _dmaFrames * averageFrames / FrameRate;

                    TimeSpan t = TimeSpan.FromSeconds(Math.Abs(totalSecTime));

                    formattedTime = string.Format("{0:D2}:{1:D2}:{2:D2}",
                                            t.Hours + t.Days * 24,
                                            t.Minutes,
                                            t.Seconds);
                }

                return formattedTime;
            }
        }

        // 0. collapse; 1. visible
        public int DMAFramesVisibility
        {
            get;
            set;
        }

        public bool DontAskUniqueExperimentName
        {
            get
            {
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                if (null != appSettings)
                {
                    XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/ExperimentSaving");

                    if (node != null)
                    {
                        string dontAskXmlValue = string.Empty;
                        XmlManager.GetAttribute(node, appSettings, "generateUniqueNameWithoutAsking", ref dontAskXmlValue);
                        return dontAskXmlValue == "1";
                    }
                }
                return false;
            }
            set
            {
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                if (null != appSettings)
                {
                    string valueString = (value) ? "1" : "0";
                    XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/ExperimentSaving");

                    if (node != null)
                    {
                        XmlManager.SetAttribute(node, appSettings, "generateUniqueNameWithoutAsking", valueString);
                        MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
                    }
                    else
                    {
                        XmlManager.CreateXmlNode(appSettings, "ExperimentSaving");
                        node = appSettings.SelectSingleNode("/ApplicationSettings/ExperimentSaving");
                        if (node != null)
                        {
                            XmlManager.SetAttribute(node, appSettings, "generateUniqueNameWithoutAsking", valueString);
                            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
                        }
                    }
                }
            }
        }

        public ObservableCollection<string> DynamicLabels
        {
            get
            {
                return _dynamicLabels;
            }
            set
            {
                _dynamicLabels = value;
            }
        }

        public string EpiTurretName
        {
            get
            {
                string str = EpiTurretPos.ToString();
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                if (null != appSettings)
                {
                    XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/EpiTurretView");
                    if (null != node)
                    {
                        if (XmlManager.GetAttribute(node, appSettings, string.Format("EpiTurretPosName{0}", EpiTurretPos), ref str))
                        {
                            return str;
                        }
                    }
                }
                return str;
            }
        }

        public int EpiTurretPos
        {
            get
            {
                int val = -1;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_EPITURRET, (int)IDevice.Params.PARAM_FW_DIC_POS, ref val);
                return val;
            }
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

        public string ExperimentFolderPath
        {
            get { return _experimentFolderPath; }
            set { _experimentFolderPath = value; }
        }

        public FileName ExperimentName
        {
            get { return _experimentName; }
            set { _experimentName = value; }
        }

        public string ExperimentNotes
        {
            get;
            set;
        }

        public string ExperimentXMLPath
        {
            get { return _experimentXMLPath; }
            set { _experimentXMLPath = value; }
        }

        public int ExtTrigger
        {
            get
            {
                return _extTrigger;
            }
            set
            {
                _extTrigger = value;
            }
        }

        public bool FastZStaircase
        {
            get
            {
                return _fastZStaircase;
            }
            set
            {
                _fastZStaircase = value;
                if (ZFastEnable)
                {
                    int val = (value) ? 0 : (int)1;
                    SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_MINIMIZE_FLYBACK_CYCLES, val);
                }
            }
        }

        public double FieldHeight
        {
            get;
            set;
        }

        public double FieldWidth
        {
            get;
            set;
        }

        public int FlybackFrames
        {
            get;
            set;
        }

        public int FlybackLines
        {
            get
            {
                return _flybackLines;
            }
            set
            {
                if (ZFastEnable)
                {
                    SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FLYBACK_CYCLE, value);
                }
                _flybackLines = value;
                OnPropertyChanged("FlybackLines");
                OnPropertyChanged("LSMFlybackTime");
            }
        }

        public double FlybackTime
        {
            get
            {
                double val = 0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FLYBACK_TIME, ref val);
                return val;
            }
        }

        /// <summary>
        /// adjust z stage flyback time to better align with frame triggers, lower limit on flyback time is 0.
        /// </summary>
        public double FlybackTimeAdjustMS
        {
            get
            {
                double tmpTime = _flybackTimeAdjustMS;
                if (0 > ((double)Constants.MS_TO_SEC * (double)FlybackFrames / FrameRate) + tmpTime)
                {
                    _flybackTimeAdjustMS = (double)Decimal.Round((decimal)(-(double)Constants.MS_TO_SEC * (double)FlybackFrames / FrameRate), 3);
                }
                return _flybackTimeAdjustMS;
            }
            set
            {
                _flybackTimeAdjustMS = (0 < ((double)Constants.MS_TO_SEC * (double)FlybackFrames / FrameRate) + value) ?
                    value : (double)Decimal.Round((decimal)(-(double)Constants.MS_TO_SEC * (double)FlybackFrames / FrameRate), 3);
            }
        }

        public double FrameRate
        {
            get
            {
                double val = 0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_FRAME_RATE, ref val);
                return val;
            }
        }

        public XmlDocument HardwareDoc
        {
            get
            {
                return this._hardwareDoc;
            }
            set
            {
                this._hardwareDoc = value;

                //update the number of maximum color channels for the system
                XmlNodeList ndListHW = _hardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

                _maxChannels = ndListHW.Count;

                // load pmt trip count:
                ndListHW = _hardwareDoc.SelectNodes("/HardwareSettings/PMT");
                if (ndListHW.Count > 0)
                {
                    string str = string.Empty;
                    int tripCount = 0;
                    if (XmlManager.GetAttribute(ndListHW[0], _hardwareDoc, "tripCount", ref str) && Int32.TryParse(str, out tripCount))
                    {
                        _pmtTripCount = tripCount;
                    }
                }
            }
        }

        public int ImageColorChannels
        {
            get { return _imageColorChannels; }
        }

        public IntPtr ImageData
        {
            get { return _imageData; }
        }

        public int ImageHeight
        {
            get { return _imageHeight; }
        }

        public char[] ImagePath
        {
            get { return _imagePath; }
            set
            {
                _imagePath = value;
            }
        }

        public Visibility ImageUpdaterVisibility
        {
            get
            {
                return this._imageUpdaterVis;
            }
            set
            {
                this._imageUpdaterVis = value;
            }
        }

        public int ImageWidth
        {
            get { return _imageWidth; }
        }

        public bool IsmROICapture
        {
            get => _ismROICapture;
        }

        public bool IsRemoteFocus
        {
            get
            {
                int zType = (int)ZStageType.PIEZO;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_STAGE_TYPE, ref zType);
                return (int)ZStageType.REMOTE_FOCUS == zType;
            }
        }

        public bool IsStimulusSaving
        {
            get
            {
                GetSaving(ref _isSaving);
                return _isSaving;
            }
            set
            {
                _isSaving = value;
                SetSaving(_isSaving);
            }
        }

        public int LeftLabelCount
        {
            get
            {
                return this._leftLabelCount;
            }
            set
            {
                this._leftLabelCount = value;
            }
        }

        public int LSMAreaMode
        {
            get;
            set;
        }

        public int LSMChannel
        {
            get
            {
                return _lsmChannel;
            }
            set
            {
                _lsmChannel = value;
            }
        }

        public bool[] LSMChannelEnable
        {
            get
            {
                return _lsmEnableChannel;
            }
        }

        public bool LSMChannelEnable0
        {
            get { return _lsmEnableChannel[0]; }
            set { _lsmEnableChannel[0] = value; }
        }

        public bool LSMChannelEnable1
        {
            get { return _lsmEnableChannel[1]; }
            set { _lsmEnableChannel[1] = value; }
        }

        public bool LSMChannelEnable2
        {
            get { return _lsmEnableChannel[2]; }
            set { _lsmEnableChannel[2] = value; }
        }

        public bool LSMChannelEnable3
        {
            get { return _lsmEnableChannel[3]; }
            set { _lsmEnableChannel[3] = value; }
        }

        public double LSMFieldScaleXFine
        {
            get
            {
                double val = 0.0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_X, ref val);
                return val;
            }
        }

        public double LSMFieldScaleYFine
        {
            get
            {
                double val = 0.0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y, ref val);
                return val;
            }
        }

        public int LSMFieldSize
        {
            get;
            set;
        }

        public double LSMFlybackTime
        {
            get
            {
                double val = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FLYBACK_TIME, ref val);

                return val;
            }
        }

        public int LSMPixelX
        {
            get { return _pixelX; }
            set { _pixelX = value; }
        }

        public int LSMPixelY
        {
            get { return _pixelY; }
            set { _pixelY = value; }
        }

        public PixelSizeUM LSMUMPerPixel
        {
            get => _lsmPixelSizeUM;
            set => value = _lsmPixelSizeUM;
        }

        public double MCLS1Power
        {
            get
            {
                return _mCLS1Power;
            }
            set
            {
                _mCLS1Power = value;
            }
        }

        public Visibility MCLS1Visibility
        {
            get
            {
                return _mCLS1Visibility;
            }
            set
            {
                _mCLS1Visibility = value;
            }
        }

        public double MCLS2Power
        {
            get
            {
                return _mCLS2Power;
            }
            set
            {
                _mCLS2Power = value;
            }
        }

        public Visibility MCLS2Visibility
        {
            get
            {
                return _mCLS2Visibility;
            }
            set
            {
                _mCLS2Visibility = value;
            }
        }

        public double MCLS3Power
        {
            get
            {
                return _mCLS3Power;
            }
            set
            {
                _mCLS3Power = value;
            }
        }

        public Visibility MCLS3Visibility
        {
            get
            {
                return _mCLS3Visibility;
            }
            set
            {
                _mCLS3Visibility = value;
            }
        }

        public double MCLS4Power
        {
            get
            {
                return _mCLS4Power;
            }
            set
            {
                _mCLS4Power = value;
            }
        }

        public Visibility MCLS4Visibility
        {
            get
            {
                return _mCLS4Visibility;
            }
            set
            {
                _mCLS4Visibility = value;
            }
        }

        public int MosaicColumnCount
        {
            get
            {
                return _mosaicColumnCount;
            }
            set
            {
                _mosaicColumnCount = value;
            }
        }

        public int MosaicRowCount
        {
            get
            {
                return _mosaicRowCount;
            }
            set
            {
                _mosaicRowCount = value;
            }
        }

        public ObservableCollection<ScanArea> mROIList
        {
            get => _mROIs;
        }

        public string[] MVMNames
        {
            get;
            set;
        }

        public int NumberOfImageTiles
        {
            get
            {
                return _numberOfImageTiles;
            }
            set
            {
                _numberOfImageTiles = value;
            }
        }

        public int NumberOfPlanes
        {
            get; set;
        }

        public string OutputPath
        {
            get { return _outputPath; }
            set { _outputPath = value; }
        }

        public bool PanelsEnable
        {
            get
            {
                return _panelsEnable;
            }
            set
            {
                _panelsEnable = value;
            }
        }

        public double PercentComplete
        {
            get
            {
                if (_experimentCompleteForProgressBar)
                {
                    return 100;
                }
                else
                {
                    double percent = _totalImagesCompleted / (double)(this.TotalImageCount) * 100;
                    return percent;
                }
            }
        }

        public double PhotobleachDurationSec
        {
            get
            {
                return _photobleachDurationSec;
            }
            set
            {
                _photobleachDurationSec = value;
            }
        }

        public int PhotobleachEnable
        {
            get
            {
                return _photobleachEnable;
            }
            set
            {
                _photobleachEnable = value;
            }
        }

        public int PhotobleachLaserPosition
        {
            get
            {
                return _photobleachLaserPosition;
            }
            set
            {
                _photobleachLaserPosition = value;
            }
        }

        public double PhotobleachPowerPosition
        {
            get
            {
                return _photobleachPowerPosition;
            }
            set
            {
                _photobleachPowerPosition = value;
            }
        }

        public int PinholePosition
        {
            get
            {
                return _pinholePosition;
            }
            set
            {
                _pinholePosition = value;
            }
        }

        public Visibility PinholeVisibility
        {
            get
            {
                return _pinholeVisibility;
            }
            set
            {
                _pinholeVisibility = value;
            }
        }

        public double PixelAspectRatioYScale
        {
            get; set;
        }

        public PixelSizeUM PixelSizeUM
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == this.ActiveCameraType)
                {
                    return this.LSMUMPerPixel;
                }
                else
                {
                    return this.CamPixelSizeUM;
                }
            }
        }

        public int PMT1EnableDuringBleach
        {
            get; set;
        }

        public int PMT2EnableDuringBleach
        {
            get; set;
        }

        public int PMT3EnableDuringBleach
        {
            get; set;
        }

        public int PMT4EnableDuringBleach
        {
            get; set;
        }

        public bool PockelsOutputReferenceAvailable
        {
            get
            {
                int useReference = 0;
                return (1 == ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_OUTPUT_USE_REF, ref useReference));
            }
        }

        public int PostBleachFrames1
        {
            get;
            set;
        }

        public int PostBleachFrames2
        {
            get;
            set;
        }

        public double PostBleachInterval1
        {
            get;
            set;
        }

        public double PostBleachInterval2
        {
            get;
            set;
        }

        public int PostBleachStream1
        {
            get;
            set;
        }

        public int PostBleachStream2
        {
            get;
            set;
        }

        public double Power0
        {
            get { return Math.Round(_power0, 2); }
        }

        public double Power1
        {
            get { return Math.Round(_power1, 2); }
        }

        public double Power2
        {
            get { return Math.Round(_power2, 2); }
        }

        public double Power3
        {
            get { return Math.Round(_power3, 2); }
        }

        public double Power4
        {
            get { return Math.Round(_power4, 2); }
        }

        public double Power5
        {
            get { return Math.Round(_power5, 2); }
        }

        public double PowerShiftUS
        {
            get;
            set;
        }

        public int PreBleachFrames
        {
            get;
            set;
        }

        public double PreBleachInterval
        {
            get;
            set;
        }

        public int PreBleachStream
        {
            get;
            set;
        }

        public int PreviewIndex
        {
            get
            {
                this._previewIndex = Math.Min(this.ZNumSteps, Math.Max(1, this._previewIndex));
                return this._previewIndex;
            }
            set
            {
                this._previewIndex = Math.Min(this.ZNumSteps, Math.Max(1, value));

            }
        }

        public int PreviousSubImageCount
        {
            get { return _previousSubImageCount; }
        }

        public int RawDataCapture
        {
            get
            {
                return _rawDataCapture;
            }
            set
            {
                _rawDataCapture = value;
            }
        }

        public int RawDataCaptureStreamingValue
        {
            get
            {
                return _rawDataCaptureStreamingValue;
            }
            set
            {
                _rawDataCaptureStreamingValue = value;
            }
        }

        public int RawOption
        {
            get;
            set;
        }

        public int RemoteFocusCaptureMode
        {
            get
            {
                return _remoteFocusCaptureMode;
            }
            set
            {
                _remoteFocusCaptureMode = value;
            }
        }

        public bool RemoteFocusCustomChecked
        {
            get;
            set;
        }

        public string RemoteFocusCustomOrder
        {
            get
            {
                return _remoteFocusCustomOrder;
            }
            set
            {
                _remoteFocusCustomOrder = value;
            }
        }

        public ObservableCollection<double> RemoteFocusCustomSelectedPlanes
        {
            get
            {
                return _remoteFocusCustomSelectedPlanes;
            }
            set
            {
                _remoteFocusCustomSelectedPlanes = value;
            }
        }

        public int RemoteFocusNumberOfPlanes
        {
            get
            {
                int numberOfPlanes = 0;
                if (0 == ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_REMOTE_FOCUS_NUMBER_OF_PLANES, ref numberOfPlanes))
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "RunSampleLS can't get param PARAM_REMOTE_FOCUS_NUMBER_OF_PLANES");
                }
                return numberOfPlanes;
            }
        }

        public double RemoteFocusStartPosition
        {
            get
            {
                return _remoteFocusStartPosition;
            }
            set
            {
                _remoteFocusStartPosition = value;
            }
        }

        public int RemoteFocusStepSize
        {
            get
            {
                return _remoteFocusStepSize;
            }
            set
            {
                _remoteFocusStepSize = value;
            }
        }

        public double RemoteFocusStopPosition
        {
            get
            {
                return _remoteFocusStopPosition;
            }
            set
            {
                _remoteFocusStopPosition = value;
            }
        }

        public bool RunComplete
        {
            get { return _runComplete; }
            set { _runComplete = value; }
        }

        public string SampleConfig
        {
            get { return _sampleConfig; }
            set
            {
                _sampleConfig = value;
            }
        }

        public int ScanMode
        {
            get { return _scanMode; }
            set { _scanMode = value; }
        }

        public string SelectedWavelengthValue
        {
            get
            {
                return this._selectedWavelengthValue;
            }
            set
            {
                this._selectedWavelengthValue = value;
            }
        }

        public int SettingsImageHeight
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == this.ActiveCameraType)
                {
                    return LSMPixelY;
                }
                else
                {
                    return this.CamImageHeight;
                }
            }
        }

        public int SettingsImageWidth
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == this.ActiveCameraType)
                {
                    return LSMPixelX;
                }
                else
                {
                    return this.CamImageWidth;
                }
            }
        }

        public int SFStartWavelength
        {
            get;
            set;
        }

        public int SFSteps
        {
            get;
            set;
        }

        public int SFStopWavelength
        {
            get;
            set;
        }

        public int SFWavelengthStepSize
        {
            get;
            set;
        }

        public Visibility ShowRawOption
        {
            get;
            set;
        }

        public int SimultaneousBleachingAndImaging
        {
            get
            {
                return _simultaneousBleachingAndImaging;
            }
            set
            {
                _simultaneousBleachingAndImaging = value;
            }
        }

        public bool SLM3D
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_3D, ref val);
                return (1 == val ? true : false);
            }
            set
            {
                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_3D, (value ? 1 : 0), (int)IDevice.DeviceSetParamType.NO_EXECUTION);
            }
        }

        public double[] SLMBleachDelay
        {
            get { return _slmBleachDelay; }
            set { _slmBleachDelay = value; }
        }

        public ObservableCollection<SLMParams> SLMBleachWaveParams
        {
            get { return _slmBleachWaveParams; }
            set { _slmBleachWaveParams = value; }
        }

        public List<int> SLMphaseType
        {
            get;
            set;
        }

        public bool SLMRandomEpochs
        {
            get;
            set;
        }

        public bool SLMSequenceOn
        {
            get;
            set;
        }

        public string[] SLMWaveBaseName
        {
            get { return new string[] { "SLMWaveform", "SLMPattern", "SLMSequence" }; }    //generic H5 waveform name, generic phase mask name
        }

        public string[] SLMWaveformFolder
        {
            get { return new string[2] { ResourceManagerCS.GetCaptureTemplatePathString() + "SLMWaveforms", ResourceManagerCS.GetCaptureTemplatePathString() + "SLMWaveforms\\SLMSequences" }; }
        }

        public string[] SLMWaveformPath
        {
            get { return new string[2] { _experimentFolderPath + "SLMWaveforms", _experimentFolderPath + "SLMWaveforms\\SLMSequences" }; }
        }

        public bool StartButtonStatus
        {
            get
            {
                return _startButtonStatus;
            }
            set
            {
                _startButtonStatus = value;
            }
        }

        public double[] Stats
        {
            get; set;
        }

        public string[] StatsNames
        {
            get; set;
        }

        public string StatusMessage
        {
            get { return _statusMessage; }
            set
            {
                _statusMessage = value;

                if (Visibility.Collapsed != (Visibility)MVMManager.Instance["RunSampleLSViewModel", "AFStatusVisibility"])
                {
                    MVMManager.Instance["RunSampleLSViewModel", "AFStatusVisibility"] = Visibility.Collapsed;
                }

                if (null != Update)
                    Update(_statusMessage);
            }
        }

        /// <summary>
        /// adjust z stage step time to better align with frame triggers, lower limit on frame time is 0.
        /// </summary>
        public double StepTimeAdjustMS
        {
            get
            {
                double tmpTime = _stepTimeAdjustMS;
                if (0 > (((double)Constants.MS_TO_SEC / FrameRate) - FlybackTime + tmpTime))
                {
                    _stepTimeAdjustMS = (double)Decimal.Round((decimal)(-(((double)Constants.MS_TO_SEC / FrameRate) - FlybackTime)), 3);
                }
                return _stepTimeAdjustMS;
            }
            set
            {
                _stepTimeAdjustMS = (0 < ((double)Constants.MS_TO_SEC / FrameRate) - FlybackTime + value) ?
                    value : (double)Decimal.Round((decimal)(-(((double)Constants.MS_TO_SEC / FrameRate) - FlybackTime)), 3);
            }
        }

        public int StimulusMaxFrames
        {
            get;
            set;
        }

        public string StimulusMaxFramesTime
        {
            get
            {

                double totalSecTime = 0;

                string str = string.Empty;

                int averageFrames = 1;

                if (1 == _averageMode)
                {
                    averageFrames = _averageNum;
                }

                string formattedTime = string.Empty;
                if (0 != FrameRate)
                {

                    totalSecTime = StimulusMaxFrames * averageFrames / FrameRate;

                    TimeSpan t = TimeSpan.FromSeconds(Math.Abs(totalSecTime));

                    formattedTime = string.Format("{0:D2}:{1:D2}:{2:D2}",
                                            t.Hours + t.Days * 24,
                                            t.Minutes,
                                            t.Seconds);
                }

                return formattedTime;
            }
        }

        public int StimulusTriggering
        {
            get;
            set;
        }

        public bool StopButtonStatus
        {
            get
            {
                return _stopButtonStatus;
            }
            set
            {
                _stopButtonStatus = value;
            }
        }

        public int StreamEnable
        {
            get { return _streamEnable; }
            set { _streamEnable = value; }
        }

        public int StreamFrames
        {
            get { return _streamFrames; }
            set { _streamFrames = value; }
        }

        public string StreamFramesTime
        {
            get
            {

                double totalSecTime = 0;

                string str = string.Empty;

                int averageFrames = 1;

                if (1 == _averageMode)
                {
                    averageFrames = _averageNum;
                }

                string formattedTime = string.Empty;
                if (0 != FrameRate)
                {

                    if (true == ZFastEnable)
                    {
                        totalSecTime = (_zNumSteps + FlybackFrames) * _streamVolumes / FrameRate;
                    }
                    else
                    {
                        totalSecTime = _streamFrames * averageFrames / FrameRate;
                    }

                    TimeSpan t = TimeSpan.FromSeconds(Math.Abs(totalSecTime));

                    formattedTime = string.Format("{0:D2}:{1:D2}:{2:D2}",
                                            t.Hours + t.Days * 24,
                                            t.Minutes,
                                            t.Seconds);
                }

                return formattedTime;
            }
        }

        public int StreamingDisplayRollingAveragePreview
        {
            get
            {
                return _streamingDisplayRollingAverage;
            }
            set
            {
                _streamingDisplayRollingAverage = value;
            }
        }

        public string StreamingPath
        {
            get
            {
                return _streamingPath;
            }
            set
            {
                _streamingPath = value;
            }
        }

        public int StreamStorageMode
        {
            get;
            set;
        }

        public string StreamTotalTime
        {
            get
            {

                double totalSecTime = 0;

                string str = string.Empty;

                //const double TIME_PER_LINE = .0000625;
                int averageFrames = 1;

                if (1 == _averageMode)
                {
                    averageFrames = _averageNum;
                }

                //int cyclesToAcquireTwoLines;

                //if (0 == _scanMode)
                //{
                //    cyclesToAcquireTwoLines = 1;
                //}
                //else
                //{
                //    //One way scan is half the rate of the two way scan
                //    cyclesToAcquireTwoLines = 2;
                //}

                //totalSecTime = cyclesToAcquireTwoLines * _streamFrames * averageFrames * _pixelY * TIME_PER_LINE;
                TimeSpan t;

                string formattedTime = string.Empty;
                if (0 != FrameRate)
                {
                    if (true == ZFastEnable)
                    {
                        totalSecTime = (_zNumSteps + FlybackFrames) * _streamVolumes / FrameRate;
                    }
                    else
                    {
                        totalSecTime = _streamFrames * averageFrames / FrameRate;
                    }

                    t = TimeSpan.FromSeconds(Math.Abs(totalSecTime));

                    formattedTime = string.Format("{0:D2}:{1:D2}:{2:D2}",
                                        t.Hours + t.Days * 24,
                                        t.Minutes,
                                        t.Seconds);
                }

                return formattedTime;
            }
        }

        public int StreamVolumes
        {
            get
            {
                return _streamVolumes;
            }
            set
            {
                _streamVolumes = value;
            }
        }

        public bool TBZSEnable
        {
            get { return _tbZSEnable; }
            set { _tbZSEnable = value; }
        }

        public Brush TBZSEnableFGColor
        {
            get { return _tbZSEnableFGColor; }
            set { _tbZSEnableFGColor = value; }
        }

        public bool TEnable
        {
            get { return _tEnable; }
            set { _tEnable = value; }
        }

        public int TFrames
        {
            get { return _tFrames; }
            set { _tFrames = value; }
        }

        public bool ThreePhotonEnable
        {
            get; set;
        }

        public int TileHeight
        {
            get { return _tileHeight; }
        }

        public int TileWidth
        {
            get { return _tileWidth; }
        }

        public int TimeBasedLineScan
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TIME_BASED_LINE_SCAN, ref val);
                return val;
            }
            set
            {
                int val = value;
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TIME_BASED_LINE_SCAN, val);
            }
        }

        public double TimeBasedLSTimeMS
        {
            get
            {
                double val = 0.0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TB_LINE_SCAN_TIME_MS, ref val);
                return val;
            }
            set
            {
                double val = Math.Round(value, 3); ;
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TB_LINE_SCAN_TIME_MS, val);
            }
        }

        public double TInterval
        {
            get { return _tInterval; }
            set { _tInterval = value; }
        }

        public int TopLabelCount
        {
            get
            {
                return this._topLabelCount;
            }
            set
            {
                this._topLabelCount = value;
            }
        }

        public int TotalImageCount
        {
            get { return _totalImageCount; }
        }

        public int TotalSubCols
        {
            get;
            set;
        }

        public int TotalSubRows
        {
            get;
            set;
        }

        public int TriggerModeStreaming
        {
            get
            {
                return _triggerModeStreaming;
            }
            set
            {
                _triggerModeStreaming = value;
            }
        }

        public int TriggerModeTimelapse
        {
            get
            {
                return _triggerModeTimelapse;
            }
            set
            {
                _triggerModeTimelapse = value;
            }
        }

        public string TTotalTime
        {
            get
            {
                TimeSpan t = TimeSpan.FromSeconds(Math.Abs((_tFrames - 1) * _tInterval));

                string formattedTime = string.Format("{0:D2}:{1:D2}:{2:D2}",
                                        t.Hours + t.Days * 24,
                                        t.Minutes,
                                        t.Seconds);
                return formattedTime;
            }
        }

        public bool TurnOffMonitorsDuringCapture
        {
            get
            {
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/RunSample/TurnOffMonitorsDuringCapture");

                if (node != null)
                {
                    string val = string.Empty;
                    XmlManager.GetAttribute(node, appSettings, "enable", ref val);
                    return val == "1";
                }
                return false;
            }
            set
            {
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                //Get Value String
                string valueString = (value) ? "1" : "0";
                XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/RunSample/TurnOffMonitorsDuringCapture");

                if (node != null)
                {
                    XmlManager.SetAttribute(node, appSettings, "enable", valueString);
                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
                }
                else
                {
                    node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/RunSample");

                    if (node != null)
                    {
                        XmlNode newNode = appSettings.CreateNode(XmlNodeType.Element, "TurnOffMonitorsDuringCapture", null);
                        node.AppendChild(newNode);
                        node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/RunSample/TurnOffMonitorsDuringCapture");
                        if (node != null)
                        {
                            XmlManager.SetAttribute(node, appSettings, "enable", valueString);
                            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
                        }
                    }
                }

                //if the acquisition is in process and the user
                //sets the 'Turn off Monitor' flag turn off the monitors immediately
                if ((false == StartButtonStatus) && (true == value))
                {
                    //turn off the monitors
                    SendMessage(0xffff, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
                }
            }
        }

        public int UseReferenceVoltageForFastZPockels
        {
            get
            {
                return _useReferenceVoltageForFastZPockels;
            }
            set
            {
                _useReferenceVoltageForFastZPockels = value;
            }
        }

        public bool VerticalTileDisplay
        {
            get;
            set;
        }

        /// <summary>
        /// adjust z stage volume time to better align with frame triggers, lower limit on volume time is 0.
        /// </summary>
        public double VolumeTimeAdjustMS
        {
            get
            {
                double tmpTime = _volumeTimeAdjustMS;
                if (0 > ((double)Constants.MS_TO_SEC * (double)_zNumSteps / FrameRate) + tmpTime)
                {
                    _volumeTimeAdjustMS = (double)Decimal.Round((decimal)(-(double)Constants.MS_TO_SEC * (double)_zNumSteps / FrameRate), 3);
                }
                return _volumeTimeAdjustMS;
            }
            set
            {
                _volumeTimeAdjustMS = (0 < ((double)Constants.MS_TO_SEC * (double)_zNumSteps / FrameRate) + value) ?
                    value : (double)Decimal.Round((decimal)(-(double)Constants.MS_TO_SEC * (double)_zNumSteps / FrameRate), 3);
            }
        }

        public WaveformDriverType WaveformDriverType
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_WAVEFORM_DRIVER_TYPE, ref temp);

                return (WaveformDriverType)temp;
            }
        }

        public ArrayList WavelengthList
        {
            get
            {
                return this._wavelengthList;
            }
            set
            {
                this._wavelengthList = value;
            }
        }

        public double XPosition
        {
            get
            {
                double val = 0;

                GetXPosition(ref val);

                return Math.Round(val, 3);
            }
        }

        public double YPosition
        {
            get
            {
                double val = 0;

                GetYPosition(ref val);

                return Math.Round(val, 3);
            }
        }

        public double Z2Position
        {
            get
            {
                double val = 0;
                ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_POS_CURRENT, ref val);
                return val;
            }
        }

        public bool Z2StageLock
        {
            get { return _z2StageLock; }
            set { _z2StageLock = value; }
        }

        public bool Z2StageMirror
        {
            get { return _z2StageMirror; }
            set { _z2StageMirror = value; }
        }

        public int ZEnable
        {
            get { return _zEnable; }
            set { _zEnable = value; }
        }

        public bool ZFastEnable
        {
            get
            {
                return _zFastEnable;
            }
            set
            {
                _zFastEnable = (ZStageType.PIEZO == ZStageType || ZStageType.REMOTE_FOCUS == ZStageType) ? value : false;

                //reset to allow any internal logic to happen
                if (_zFastEnable)
                {
                    FastZStaircase = _fastZStaircase;
                    FlybackLines = _flybackLines;
                }
            }
        }

        public int ZFileEnable
        {
            get { return _zFileEnable; }
            set { _zFileEnable = value; }
        }

        public int ZFilePosRead
        {
            get { return _zFilePosRead; }
            set { _zFilePosRead = value; }
        }

        public double ZFilePosScale
        {
            get { return _zFilePosScale; }
            set { _zFilePosScale = value; }
        }

        public bool ZInvert
        {
            get
            {
                return _zInvert;
            }
            set
            {
                _zInvert = value;
            }
        }

        public double ZMax
        {
            get
            {
                double zMax = 0;
                double zMinPos = 0;
                double zMaxPos = 0;
                double zDefaultPos = 0;

                if (GetZRange(ref zMinPos, ref zMaxPos, ref zDefaultPos))
                {
                    zMax = zMaxPos;

                }

                return zMax;
            }
        }

        public double ZMin
        {
            get
            {
                double zMin = 0;
                double zMinPos = 0;
                double zMaxPos = 0;
                double zDefaultPos = 0;

                if (GetZRange(ref zMinPos, ref zMaxPos, ref zDefaultPos))
                {
                    zMin = zMinPos;

                }

                return zMin;
            }
        }

        public int ZNumSteps
        {
            get
            {
                switch ((CaptureModes)CaptureMode)
                {
                    case CaptureModes.T_AND_Z:
                        //stepping less than one is not realistic experiment configuration.
                        if (_zFileEnable == 1)
                        {
                            if (_zFilePosRead != 1)
                            {
                                getZPosFromFile();
                            }
                            return _zNumSteps;
                        }
                        else
                        {
                            double stepFactor = (double)Constants.UM_TO_MM; //IsRemoteFocus ? 1 : (double)Constants.UM_TO_MM;
                            _zNumSteps = Math.Max(1, (int)Math.Round(Math.Abs(Math.Round((_zStopPosition - _zStartPosition), 5) / (_zStepSize / stepFactor)) + 1));
                        }
                        break;
                    case CaptureModes.STREAMING:
                        //stepping less than one is not realistic experiment configuration,
                        //special case for single step for step size == (stop - start)
                        if (IsRemoteFocus)
                        {
                            if ((int)RemoteFocusCaptureModes.Custom == RemoteFocusCaptureMode)
                            {
                                if (RemoteFocusCustomChecked) // When user types the planes we need to count the number of planes typed
                                {
                                    string[] values = RemoteFocusCustomOrder.Split(':');
                                    _zNumSteps = (string.Empty == values[values.Length - 1]) ? values.Length - 1 : values.Length;
                                }
                                else
                                {
                                    _zNumSteps = RemoteFocusCustomSelectedPlanes.Count;
                                }
                            }
                            else
                            {
                                _zNumSteps = Math.Max(1, (int)Math.Round(Math.Abs((RemoteFocusStopPosition - RemoteFocusStartPosition) / RemoteFocusStepSize) + 1));
                            }
                        }
                        else
                        {
                            _zNumSteps = ((!FastZStaircase) && (_zStepSize == (_zStopPosition - _zStartPosition) * (double)Constants.UM_TO_MM)) ?
                                Math.Max(1, (int)Math.Round(Math.Abs(Math.Round((_zStopPosition - _zStartPosition), 5) / (_zStepSize / (double)Constants.UM_TO_MM)))) :
                                Math.Max(1, (int)Math.Round(Math.Abs(Math.Round((_zStopPosition - _zStartPosition), 5) / (_zStepSize / (double)Constants.UM_TO_MM)) + 1));
                        }
                        break;
                    case CaptureModes.BLEACHING:
                    case CaptureModes.HYPERSPECTRAL:
                    default:
                        _zNumSteps = Math.Max(1, _zNumSteps);
                        break;
                }

                return _zNumSteps;
            }
        }

        public double ZPosition
        {
            get
            {
                double val = 0;
                ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_POS_CURRENT, ref val);
                return val;
            }
        }

        public ZStageType ZStageType
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_STAGE_TYPE, ref val);
                return (ZStageType)val;

            }
        }

        public double ZStartPosition
        {
            get { return _zStartPosition; }
            set { _zStartPosition = value; }
        }

        public double ZStepSize
        {
            get { return _zStepSize; }
            set { _zStepSize = value; }
        }

        public double ZStopPosition
        {
            get { return _zStopPosition; }
            set { _zStopPosition = value; }
        }

        public bool ZStreamEnable
        {
            get { return (_zStreamEnable && (_zEnable != 0)); }
            set { _zStreamEnable = value; }
        }

        public int ZStreamFrames
        {
            get { return _zStreamFrames; }
            set { _zStreamFrames = value; }
        }

        #endregion Properties

        #region Methods

        public static int GetBitsPerPixel()
        {
            int camType = ResourceManagerCS.GetCameraType();
            int val = DEFAULT_BITS_PER_PIXEL;
            if (((int)ICamera.CameraType.LSM != camType) && ((int)ICamera.CameraType.LAST_CAMERA_TYPE) != camType)
            {
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BITS_PER_PIXEL, ref val);
            }
            return val;
        }

        public static ushort[] GetPixelData()
        {
            return _pixelData;
        }

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "IsRunning")]
        public static extern bool IsRunning();

        /// <summary>
        /// Attempts to load the document at the selected document path until successful
        /// or the max number of attempts is made.
        /// </summary>
        /// <param name="xmlDoc"> The document object to load into </param>
        /// <param name="documentPath"> The path to load from </param>
        /// <param name="attempts"> The number of attempts to load to make </param>
        /// <param name="millisecondsBetweenAttempts"> The amount of time to block between each attempt </param>
        /// <returns> True if the document was successfully loaded </returns>
        public static bool TryLoadDocument(XmlDocument xmlDoc, string documentPath, int attempts = 3, int millisecondsBetweenAttempts = 10)
        {
            //When scripting the active.xml might be locked up when trying to load it here
            //Using a try-catch inside of a while loop allows to wait for 10ms and try
            //again for up to 3 times.
            int tries = 0;
            while (tries < attempts)
            {
                try
                {
                    xmlDoc.Load(documentPath);
                    return true;
                }
                catch
                {
                    tries++;
                    System.Threading.Thread.Sleep(millisecondsBetweenAttempts);
                }
            }
            return false;
        }

        public void ConnectCallbacks()
        {
            InitCallBack
                (
                _reportCallBack,
                _reportCallBackImage,
                _reportSubRowStartCallBack,
                _reportSubRowEndCallBack,
                _reportZCallBack,
                _reportTCallBack,
                _reportPreCaptureCallBack,
                _reportSequenceStepCurrentIndexCallBack,
                _reportInformMessage,
                _reportSavedFileIPC,
                _reportAutoFocusStatus
                );

            InitCallBackROIDataStore(_multiROIStatsCallBack);
            InitCallBackLineProfilePush(null);
        }

        public void CreateRemoteFocusPositionValuesTextFile()
        {
            try
            {
                if (!Directory.Exists(_experimentFolderPath + "\\RemoteFocus\\"))
                {
                    Directory.CreateDirectory(_experimentFolderPath + "\\RemoteFocus\\");
                    string positionsPath = _experimentFolderPath + "\\RemoteFocus\\RemoteFocusPositionsValues.txt";

                    // Create a StreamWriter object to write to the file
                    using (StreamWriter writer = new StreamWriter(positionsPath))
                    {
                        if (ZInvert)
                        {
                            for (int i = DynamicLabels.Count - 1; i >= 0; i--)
                            {
                                writer.WriteLine(DynamicLabels[i]);
                            }
                        }
                        else
                        {
                            foreach (string val in DynamicLabels)
                            {
                                writer.WriteLine(val);
                            }
                        }
                    }
                }
            }
            catch (Exception)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + "RunSampleLSModule: Could not create RemoteFocusPositionsValues file.");
            }
        }

        public void DisplayColorImage(List<string> fileNames, int regionID = 0, int regionIndex = 0)
        {
            try
            {
                if (1 == ImageMethod) //DFLIM Image
                {
                    int channelNum = 0;
                    for (int i = 0; i < _maxChannels; ++i)
                    {
                        if (_lsmEnableChannel[i])
                        {
                            ++channelNum;
                        }
                    }
                    int width = 32;
                    int height = 32;
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_X, ref width);
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_Y, ref height);

                    _imageWidth = width;
                    _imageHeight = height;

                    const int DFLIM_HISTOGRAM_BINS = 256;
                    //1 datalength for photon num buffer (intensity) (USHORT)
                    //1 datalength for single photon sum buffer (USHORT)
                    //2 datalength for arrival time sum buffer (UINT32)
                    //4 DFLIM_HISTOGRAM_BINS for dflim histogram (UINT32)
                    int intensityDataBufferSize = width * height;

                    int totalBufferSizeBytes = (intensityDataBufferSize * 8 + DFLIM_HISTOGRAM_BINS * 4) * channelNum;
                    _imageColorChannels = channelNum;

                    //allocate the buffer size
                    _imageData = Marshal.AllocHGlobal(totalBufferSizeBytes);

                    //memset is necessary to make sure the buffer is 0 for all channels before reading the images
                    //in cases when not all four channels are on there rollover, intensity data can show up
                    //as if there was data on a channel that is not turned on
                    memset(_imageData, 0, totalBufferSizeBytes);

                    int result = ReadRawImages(fileNames[0], totalBufferSizeBytes, ref _imageData, 0);
                }
                else
                {
                    int width = 0;
                    int height = 0;
                    int colorChannels = 0;
                    int numTiles = 0;
                    int tileWidth = 0;
                    int tileHeight = 0;
                    // get the image parameters
                    for (int i = 0; i < fileNames.Count; i++)
                    {
                        if (fileNames[i] != null)
                        {
                            //ReadImageInfo(fileNames[i], ref width, ref height, ref colorChannels);
                            ReadImageInfo2(fileNames[i], ref width, ref height, ref colorChannels, ref numTiles, ref tileWidth, ref tileHeight);
                            _numberOfImageTiles = numTiles;
                            break;
                        }
                    }

                    // multi-channel = single file with mutliple channel; normal = one file per channel
                    bool isMultiChannel = fileNames.Count == 1 && colorChannels > 1;

                    // setting the parameters to be used in the View Model
                    _imageWidth = width;
                    _imageHeight = height;
                    _tileWidth = tileWidth;
                    _tileHeight = tileHeight;
                    _imageColorChannels = isMultiChannel ? colorChannels : fileNames.Count;

                    if (_isSequentialCapture)
                    {
                        int count = 0;
                        for (int i = 0; i < MAX_CHANNELS; i++)
                        {
                            if ((_channelOrderCurrentLSMChannel & (0x0001 << i)) > 0)
                            {
                                count++;
                            }
                        }

                        List<string> sublistFileNames = new List<string>();
                        for (int i = fileNames.Count - count; i < fileNames.Count; i++)
                        {
                            sublistFileNames.Add(fileNames[i]);
                        }
                        fileNames.Clear();
                        fileNames = sublistFileNames;

                        _imageColorChannels = count;
                    }

                    //allocate the buffer size
                    _imageData = Marshal.AllocHGlobal(_imageWidth * _imageHeight * _imageColorChannels * 2);

                    //memset is necessary to make sure the buffer is 0 for all channels before reading the images
                    //in cases when not all four channels are on there rollover, intensity data can show up
                    //as if there was data on a channel that is not turned on
                    memset(_imageData, 0, _imageWidth * _imageHeight * _imageColorChannels * 2);

                    //read the image and output the buffer with image data
                    int result = 0;

                    bool isTiled = tileWidth != 0 && tileHeight != 0;

                    if (isMultiChannel)
                    {
                        if (isTiled) // FastZ
                        {
                            result = ReadMultiChannelTiledImage(fileNames[0], _imageColorChannels, ref _imageData);
                        }
                        else
                        {
                            result = ReadMultiChannelImage(fileNames[0], _imageColorChannels, ref _imageData, _imageWidth, _imageHeight);
                        }

                    }
                    else
                    {
                        if (isTiled) // FastZ
                        {
                            result = ReadChannelTiledImages(fileNames.ToArray(), _imageColorChannels, ref _imageData);
                        }
                        else
                        {
                            result = ReadChannelImages(fileNames.ToArray(), _imageColorChannels, ref _imageData, _imageWidth, _imageHeight);
                        }
                    }
                }

                CopyChannelData(regionID, regionIndex);
            }
            catch (Exception ex)
            {
                string str = ex.Message;
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + "File not found exception");
            }
            finally
            {
                Marshal.FreeHGlobal(_imageData);
            }
        }

        public void EnableHandlers()
        {
            // Set up the Background Worker Events
            _backgroundWorker.DoWork += BackgroundWorker_DoWork;
            _backgroundWorker.RunWorkerCompleted += BackgroundWorker_RunWorkerCompleted;
            _backgroundWorker.WorkerSupportsCancellation = true;
            _slmLoader.DoWork -= SLMLoader_DoWork; //Ensure event is not registered twice
            _slmLoader.DoWork += SLMLoader_DoWork;

            _slmLoader.RunWorkerCompleted -= SLMLoader_RunWorkerCompleted;
            _slmLoader.RunWorkerCompleted += SLMLoader_RunWorkerCompleted;
            _slmLoader.WorkerSupportsCancellation = true;

            _panelsEnable = true;
            // event trigerred to the view model to enable the Panels
            UpdatePanels(_panelsEnable);

            string appSettingsFolder = Application.Current.Resources["ApplicationSettingsFolder"].ToString();

            if (Directory.Exists(appSettingsFolder))
            {
                DataStore.Instance.ConnectionString = string.Format("URI=file:{0}\\{1}", appSettingsFolder, "thorDatabase.db");
                DataStore.Instance.Open();
            }
        }

        /// <summary>
        /// Checks the experiment settings file with the capture mode to make sure
        /// capture settings are valid. Will create a message box at the first check
        /// not passed. Checks include:
        ///     * Streaming and Bleaching Capture are not compatible with Capture Sequence Mode
        ///     * Streaming and Bleaching Capture are not compatible with Tiles Capture
        ///     * Sequential Capture Mode is not compatible with the CCD type Cameras
        ///     * Sequential Capture Mode only allows referencing a channel once per Capture Sequence
        ///     * FastZ Mode is not supported with images which have a width that is not a multiple of 16
        ///     * Hyperspectral Capture is not compatible with Averagin Mode
        /// </summary>
        /// <param name="expDoc"> The input document to check </param>
        /// <returns> True if all checks passed </returns>
        public bool ExperimentSettingsValidForCapture(XmlDocument expDoc)
        {
            //=== Streaming Capture is not compatible with Capture Sequence Mode ===
            XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/CaptureSequence");
            if (0 < ndList.Count)
            {
                string str = string.Empty;
                bool enable = (XmlManager.GetAttribute(ndList[0], expDoc, "enable", ref str) && ("1" == str || bool.TrueString == str)) ? true : false;
                ndList = expDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
                _sequenceStepsNum = ndList.Count;

                if (enable)
                {
                    //Capture Sequence Mode
                    if (1 < _sequenceStepsNum)
                    {
                        //check when streaming or bleaching
                        if (1 == CaptureMode || 3 == CaptureMode)
                        {
                            MessageBox.Show("\"Sequential Capture Mode\" is incompatible with Streaming and Bleaching captures. Please clear your channel sequence options for the experiment and start again.");
                            return false;
                        }
                        _isSequentialCapture = true;
                    }
                }
            }

            //check when streaming or bleaching
            if (1 == this.CaptureMode || 3 == this.CaptureMode)
            {
                //=== Streaming Capture is not compatible with Tiles Capture ===
                ndList = expDoc.SelectNodes("/ThorImageExperiment/Sample/Wells");
                int tilesCount = 0;
                for (int i = 0; i < ndList.Count; i++)
                {
                    //Get all well positions
                    XmlNodeList innderNdList = ndList[i].SelectNodes("SubImages");
                    for (int j = 0; j < innderNdList.Count; j++)
                    {
                        string str = string.Empty;
                        int iVal = 0;
                        if (XmlManager.GetAttribute(innderNdList[j], expDoc, "isEnabled", ref str) && (Boolean.TrueString == str || "1" == str))
                        {
                            str = string.Empty;
                            int subRows = 1, subColumns = 1;
                            if (IsXyViewVisible())
                            {
                                if (XmlManager.GetAttribute(innderNdList[j], expDoc, "subRows", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                                {
                                    subRows = iVal;
                                }
                                str = string.Empty;
                                if (XmlManager.GetAttribute(innderNdList[j], expDoc, "subColumns", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                                {
                                    subColumns = iVal;
                                }
                            }
                            tilesCount += subRows * subColumns;

                            //Allow up to 1 tile when streaming
                            if (1 < tilesCount)
                            {
                                MessageBox.Show("\"Tiles Mode\" is incompatible with Streaming and Bleaching capture when there is more than one tile defined. Please clear the tiles for the experiment and start again.");
                                return false;
                            }
                        }
                    }
                }
            }

            //check when FastZ is enabled
            if (1 == this.CaptureMode)
            {
                //=== FastZ Mode is not supported with images which have a width that is not a multiple of 16 ===
                string str = string.Empty;
                ndList = expDoc.SelectNodes("/ThorImageExperiment/Streaming");
                // Only check with the CCD cameras because it's currently the only type of camera that can be set to a non-multiple of 16. LSM is always set to a multiple of 32
                if (XmlManager.GetAttribute(ndList[0], expDoc, "zFastEnable", ref str) && (("1" == str || bool.TrueString == str) && (int)ICamera.CameraType.CCD == this.ActiveCameraType))
                {
                    str = string.Empty;
                    int tmpWidth = 0;
                    ndList = expDoc.SelectNodes("/ThorImageExperiment/Camera");
                    XmlManager.GetAttribute(ndList[0], expDoc, "width", ref str);
                    Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpWidth);
                    if (0 != tmpWidth % 16)
                    {
                        MessageBox.Show("\"Fast Z\" is incompatible for images whose width is not a multiple of 16. Please make sure the width of your image is a multiple of 16.");
                        return false;
                    }
                }
            }
            //check if it is a HyperSpectral Capture
            if (4 == this.CaptureMode)
            {
                //=== Hyperspectral Capture is not compatible with Averaging Mode ===
                string str = string.Empty;
                string strAverageNum = string.Empty;
                int avNum = 0;
                ndList = expDoc.SelectNodes("/ThorImageExperiment/Camera");
                XmlManager.GetAttribute(ndList[0], expDoc, "averageMode", ref str);
                XmlManager.GetAttribute(ndList[0], expDoc, "averageNum", ref strAverageNum);
                Int32.TryParse(strAverageNum, NumberStyles.Any, CultureInfo.InvariantCulture, out avNum);
                if (1 < avNum && ("1" == str || bool.TrueString == str))
                {
                    MessageBox.Show("\"Averaging Mode\" is incompatible with HyperSpectral Capture. Please make sure Averaging is disabled.");
                    return false;
                }
            }

            return true;
        }

        public int GetFoundChannels()
        {
            return _foundChannelCount;
        }

        /// <summary>
        /// Combines the LSM Channel enabled bitmasks for all channel steps, returning the overall number of enabled channels. Works
        /// if not using sequential capture mode as well.
        /// </summary>
        /// <param name="expDoc"> The xml experiment file to load from </param>
        /// <returns> Bitmask containing all enabled channels for all channel steps. </returns>
        public int GetLSMChannelAllCaptureSequenceSteps(XmlDocument expDoc)
        {
            XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");

            if (0 < ndList.Count)
            {
                //Wavelength and channel settings

                int lsmChannelCombined = 0;
                foreach (XmlNode node in ndList)
                {

                    XmlNodeList lsmNdList = node.SelectNodes("LSM");
                    if (lsmNdList.Count > 0)
                    {
                        string str = string.Empty;
                        int iVal = 0;
                        if (XmlManager.GetAttribute(lsmNdList[0], expDoc, "channel", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                        {
                            lsmChannelCombined = lsmChannelCombined | iVal;
                        }
                    }
                }
                return lsmChannelCombined;
            }
            else
            {
                return LSMChannel;
            }
        }

        public int GetMaxChannels()
        {
            return _maxChannels;
        }

        /// <summary>
        /// Returns the application settings value from the application settings xml for if a raw file experiment should only contain enabled channels,
        /// or disabled channels as well
        /// </summary>
        /// <returns> True if the raw file should only contain enabled channels </returns>
        public bool GetShouldSaveOnlyEnabledChannelsInRawFile()
        {
            XmlDocument appSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            if (appSettingsDoc != null)
            {
                XmlNodeList ndList = appSettingsDoc.SelectNodes("/ApplicationSettings/RawFileOptions");

                if (ndList.Count > 0)
                {
                    string value = "";
                    if (XmlManager.GetAttribute(ndList[0], appSettingsDoc, "saveEnabledChannelsOnly", ref value) && ("1" == value))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        // read the position list from a file
        public bool getZPosFromFile()
        {
            // clear the list and flags
            _zNumSteps = 0;
            _zFilePosRead = 0;
            if (_zFilePosList != null) _zFilePosList.Clear();

            string posFileName = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\CustomZPos\\PositionList.txt";
            StreamReader fs;
            try
            {
                fs = new StreamReader(posFileName);
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, ex.Message);
                MessageBox.Show("File load error: " + ex.Message);
                return false;
            }

            try
            {
                string line;
                while ((line = fs.ReadLine()) != null)
                {
                    if (Double.TryParse(line, NumberStyles.Any, CultureInfo.InvariantCulture, out double dVal))
                    {
                        _zFilePosList.Add(dVal);
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, ex.Message);
                MessageBox.Show("File read error: " + ex.Message);
                fs.Close();
                return false;
            }
            fs.Close();
            _zNumSteps = _zFilePosList.Count;
            _zFilePosRead = 1;
            return true;
        }

        public bool LIGetFieldSizeCalibration(ref double fieldSizeCal)
        {
            return (1 == ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE_CALIBRATION, ref fieldSizeCal));
        }

        public void LoadmROISettings(XmlDocument expDoc)
        {
            _ismROICapture = mROIXMLMapper.MapXml2mROIParams(expDoc, LSMType, out _mROIs, out _mROIPixelSizeXUM, out _mROIPixelSizeYUM, out _mROIStripLength, out _mROIFullFOVMetadata);
        }

        public bool LoadSLMPatternName(int runtimeCal, int id, string bmpPatternName, bool start, bool phaseDirect = false, int phaseType = 0, int timeoutVal = 0)
        {
            return ((1 == LoadSLMPattern(runtimeCal, id, bmpPatternName, (start) ? 1 : 0, (phaseDirect) ? 1 : 0, phaseType, timeoutVal)) ? true : false);
        }

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

        public void ReleaseBleach()
        {
            ReleaseBleachParams();
        }

        public void ReleaseHandlers()
        {
            _backgroundWorker.DoWork -= BackgroundWorker_DoWork;
            _backgroundWorker.RunWorkerCompleted -= BackgroundWorker_RunWorkerCompleted;

            _slmLoader.DoWork -= SLMLoader_DoWork;
            _slmLoader.RunWorkerCompleted -= SLMLoader_RunWorkerCompleted;

            DataStore.Instance.Close();
        }

        public bool SetBleachFile(string slmFileName, int cycleNum)
        {
            return ((1 == SetBleachWaveformFile(slmFileName, cycleNum)) ? true : false);
        }

        public bool Start()
        {
            if (IsRunning() || _slmLoader.IsBusy || ((int)CaptureModes.STREAMING == CaptureMode && 0 == _streamFrames))
                return false;

            // event trigerred to the view model to change the status of the Start and Stop button
            _startButtonStatus = false;
            _stopButtonStatus = true;

            UpdateButton(false);

            // event trigerred to the view model to change the status of the menu bar buttons
            UpdateMenuBarButton(false);

            _slmLoader.RunWorkerAsync();

            return true;
        }

        public void Stop()
        {
            ThorImage.SplashScreen splash = new ThorImage.SplashScreen();

            _backgroundWorkerDone = false;

            try
            {
                BackgroundWorker worker = new BackgroundWorker();
                worker.DoWork += (o, ea) =>
                {
                    do
                    {
                        System.Threading.Thread.Sleep(1);//wait till the background process and/or image tile capturing process completes
                    }
                    while ((_slmLoader.IsBusy) || (_backgroundWorker.IsBusy) || (IsRunning() == true));
                };

                worker.RunWorkerCompleted += (o, ea) =>
                {
                    splash.Close();

                    _experimentStatus = "Stopped";

                    LogPMTTripCount();

                    UpdateExperimentFile(_experimentXMLPath, true);

                    // App inherits from Application, and has a Window property called MainWindow
                    // and a List<Window> property called OpenWindows.
                    Application.Current.MainWindow.Activate();

                    _completedImageCount = _currentImageCount = _currentSubImageCount = 0;

                    // event trigerred to the view model to update the status of the progress feedback
                    StatusMessage = "The experiment has stopped";

                    _startButtonStatus = true;
                    _stopButtonStatus = false;
                    UpdateButton(true);
                    // event trigerred to the view model to change the status of the menu bar button
                    UpdateMenuBarButton(true);

                    _panelsEnable = true;
                    // event trigerred to the view model to enable the Panels
                    UpdatePanels(_panelsEnable);

                    IsStimulusSaving = false;
                    _runComplete = true;
                    _backgroundWorkerDone = true;
                    //notify any running script that the command has stopped
                    UpdateScript("Stop");
                };

                _slmLoader.CancelAsync();
                _panelsEnable = false;
                UpdatePanels(_panelsEnable);
                splash.ShowInTaskbar = false;
                splash.Owner = Application.Current.MainWindow;
                splash.Show();
                worker.RunWorkerAsync();

                //disable digital trigger at stop
                MVMManager.Instance["DigitalOutputSwitchesViewModel", "TriggerEnable"] = 0;
                RunSampleLSStop();
                Thread t2 = new Thread(delegate ()
                {
                    Thread.Sleep(300);        //delay so there is a small amount of time after the last pulse in o-scope data
                    bool ipcDownlinkFlag = (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "IPCDownlinkFlag", (object)false];
                    bool remoteConnection = (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "RemoteConnection", (object)false];
                    if (ipcDownlinkFlag == false && remoteConnection == true)
                    {
                        MVMManager.Instance["RemoteIPCControlViewModelBase", "StopAcquisition"] = true;
                    }
                });
                t2.Start();
                ////resetting the well color
                //int index;
                //for (index = 0; index < _collection.Count; index++)
                //{
                //    _collection[index].Background = new SolidColorBrush(Color.FromRgb(199, 199, 199)); //grey color
                //}

                ////resetting the mosaic well color
                //int i;
                //for (i = 0; i < _collectionMosaic.Count; i++)
                //{
                //    _collectionMosaic[i].Background = new SolidColorBrush(Color.FromRgb(199, 199, 199)); //grey color
                //}
            }
            catch (System.DllNotFoundException)
            {
                //RunSampleLSdll is missing
                splash.Close();
                _backgroundWorkerDone = true;
                _panelsEnable = true;
                // event trigerred to the view model to enable the Panels
                UpdatePanels(_panelsEnable);
            }
        }

        public void UpdateCamBitsPerPixel()
        {
            if ((int)ICamera.CameraType.CCD == this.ActiveCameraType)
            {
                int val = 1;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BITS_PER_PIXEL, ref val);
                _camBitsPerPixel = val;
            }
        }

        public bool UpdateExperimentFile()
        {
            return this.UpdateExperimentFile(ActiveExperimentPath, false);
        }

        public void UpdateExperimentFileNewModality()
        {
            XmlNodeList nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/LSM");

            if (nodeList.Count <= 0)
            {
                int val = 0;
                if (1 == ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP, ref val))
                {
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "horizontalFlip", val.ToString());
                }
                if (1 == ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IMAGE_VERTICAL_FLIP, ref val))
                {
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "verticalFlip", val.ToString());
                }
            }

            ExperimentDoc.Save(ActiveExperimentPath);
        }

        public bool ValidateRemoteFocusCustom(string input)
        {
            string pattern = @"^\d+(\:\d+)*$";
            if (Regex.IsMatch(input, pattern))
            {
                return true;
            }
            MessageBox.Show("The Custom Remote Focus step set doesn't match the correct syntax. Please verify each plane number is separated by colon ':' e.g. 1:2:3");
            return false;
        }

        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        [DllImport(".\\StatsManager.dll", EntryPoint = "CreateStatsManagerROIDS")]
        private static extern int CreateStatsManagerROIDS(int dstype, string str);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPath", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPath(StringBuilder path, int length);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPathAndName(StringBuilder path, int length);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "GetCommandGUID")]
        private static extern int GetCommandGUID(ref Guid guid);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "GetCustomParamsBinary")]
        private static extern int GetCustomParamsBinary(ref RunSampleLSCustomParams lidParams);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetNumberOfWavelengths")]
        private static extern int GetNumberOfWavelengths();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetPMTSafetyStatus")]
        private static extern bool GetPMTSafetyStatus();

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetSample")]
        private static extern int GetSample(ref int type, ref double offsetXMM, ref double offsetYMM);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "GetSaving")]
        private static extern bool GetSaving(ref bool _getSave);

        //[DllImport(".\\ExperimentManager.dll", EntryPoint = "GetSubImages")]
        //private static extern int GetSubImages(ref int subRows, ref int subColumns, ref double subOffsetXMM, ref double subOffsetYMM, ref double transOffsetXMM, ref double transOffsetYMM);
        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetTimelapse")]
        private static extern int GetTimelapse(ref int timePoints, ref double intervalSec);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetWavelength", CharSet = CharSet.Unicode)]
        private static extern int GetWavelength(int wavelengthIndex, StringBuilder name, int length, ref double exposureTimeMS);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetWells")]
        private static extern int GetWells(ref int startRow, ref int startColumn, ref int wellRows, ref int wellCols, ref double wellOffsetX, ref double wellOffsetY);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "GetXPosition")]
        private static extern bool GetXPosition(ref double xPosition);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "GetYPosition")]
        private static extern bool GetYPosition(ref double yPosition);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "GetZRange")]
        private static extern bool GetZRange(ref double _zMin, ref double zMax, ref double zDefault);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetZStage", CharSet = CharSet.Unicode)]
        private static extern int GetZStage(StringBuilder name, int length, ref int zstageSteps, ref double zstageStepSize, ref double zStartPos);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "InitCallBack")]
        private static extern void InitCallBack(Report report, ReportIndex reportIndex, ReportSubRowStartIndex reportSubRowStartIndex, ReportSubRowEndIndex reportSubRowEndIndex, ReportZIndex reportZIndex, ReportTIndex reportTIndex, ReportPreCapture reportPreCapture, ReportSequenceStepCurrentIndex reporSequenceStepCurrentIndx, ReportInformMessage reportInformMessage, ReportSavedFileIPC reportSavedFileIPC, ReportAutoFocusStatus reportAutoFocusStatus);

        [DllImport(".\\StatsManager.dll", EntryPoint = "InitCallBackLineProfilePush")]
        private static extern void InitCallBackLineProfilePush(ReportLineProfile reportLineProfile);

        [DllImport(".\\ROIDataStore.dll", EntryPoint = "InitCallBack")]
        private static extern void InitCallBackROIDataStore(ReportMultiROIStats reportMultiROIStats);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "LoadSLMPattern", CharSet = CharSet.Unicode)]
        private static extern int LoadSLMPattern(int runtimeCal, int id, string bmpPatternName, int doStart, int phaseDirect, int phaseType, int timeoutVal);

        [DllImport("msvcrt.dll", EntryPoint = "memset", CallingConvention = CallingConvention.Cdecl, SetLastError = false)]
        private static extern IntPtr memset(IntPtr dest, int c, int count);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadChannelImages")]
        private static extern int ReadChannelImages([MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPWStr)] string[] fileNames, int size, ref IntPtr outputBuffer, int cameraWidth, int cameraHeight);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadChannelRawImages")]
        private static extern int ReadChannelRawImages([MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPWStr)] string[] fileNames, int size, ref IntPtr outputBuffer, int cameraWidth, int cameraHeight);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadChannelTiledImages")]
        private static extern int ReadChannelTiledImages([MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPWStr)] string[] fileNames, int size, ref IntPtr outputBuffer);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadColorImage")]
        private static extern int ReadColorImage([MarshalAs(UnmanagedType.LPWStr)] string rPath, [MarshalAs(UnmanagedType.LPWStr)] string gPath, [MarshalAs(UnmanagedType.LPWStr)] string bPath, ref IntPtr outputBuffer, int cameraWidth, int cameraHeight);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImage")]
        private static extern int ReadImage([MarshalAs(UnmanagedType.LPWStr)] string path, ref IntPtr outputBuffer);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo")]
        private static extern int ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)] string path, ref int width, ref int height, ref int colorChannels, ref int bitsPerPixel);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo2")]
        private static extern int ReadImageInfo2([MarshalAs(UnmanagedType.LPWStr)] string path, ref int width, ref int height, ref int colorChannels, ref int numOfTiles, ref int tileWidth, ref int tileHeight);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadMultiChannelImage")]
        private static extern int ReadMultiChannelImage([MarshalAs(UnmanagedType.LPWStr)] string fileName, int numChannels, ref IntPtr outputBuffer, int cameraWidth, int cameraHeight);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadMultiChannelTiledImage")]
        private static extern int ReadMultiChannelTiledImage([MarshalAs(UnmanagedType.LPWStr)] string fileName, int numChannels, ref IntPtr outputBuffer);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImagesRaw")]
        private static extern int ReadRawImages([MarshalAs(UnmanagedType.LPWStr)] string path, long size, ref IntPtr outputBuffer, long offset);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "ReleaseBleachParams")]
        private static extern int ReleaseBleachParams();

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "Execute")]
        private static extern int RunSampleLSExecute();

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "SetupCommand")]
        private static extern int RunSampleLSSetupCommand();

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "Stop")]
        private static extern int RunSampleLSStop();

        [DllImport("user32.dll")]
        private static extern int SendMessage(int hWnd, int hMsg, int wParam, int lParam);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "SetActiveExperiment", CharSet = CharSet.Unicode)]
        private static extern int SetActiveExperiment(string path);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "SetBleachWaveformFile", CharSet = CharSet.Unicode)]
        private static extern int SetBleachWaveformFile(string WaveH5PathandName, int cycleNum);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamLong")]
        private static extern int SetCameraParamInt(int cameraSelection, int param, int value);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "SetCustomParamsBinary")]
        private static extern int SetCustomParamsBinary(ref RunSampleLSCustomParams lidParams);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamLong")]
        private static extern int SetDeviceParamInt(int deviceSelection, int paramId, int param, bool wait);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "SetSaving")]
        private static extern bool SetSaving(bool _toSave);

        private void AutoFocusStatusMessage(ref int status, ref int bestScore, ref double bestZPos, ref double nextZPos, ref int currRepeatatus)
        {
            string state = string.Empty;
            switch (status)
            {
                case (int)AutoFocusStatusTypes.NOT_RUNNING:
                    state = "-Not Running-";
                    break;
                case (int)AutoFocusStatusTypes.STOPPED:
                    state = "-Stopped-";
                    break;
                case (int)AutoFocusStatusTypes.COARSE_AUTOFOCUS:
                    state = "-Coarse autofocus-";
                    break;
                case (int)AutoFocusStatusTypes.FINE_AUTOFOCUS:
                    state = "-Fine autofocus-";
                    break;
            }
            MVMManager.Instance["RunSampleLSViewModel", "AFStatusVisibility"] = Visibility.Visible;
            bestZPos = Math.Round(bestZPos * 1000, 1);
            MVMManager.Instance["RunSampleLSViewModel", "AFStatusMessage"] = "\nAutofocus Running. Status: " + state + " Best Z Position found: " + bestZPos.ToString();
        }

        // Worker Method
        private void BackgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            if (_backgroundWorker.CancellationPending)
            {
                e.Cancel = true;
                return;
            }
        }

        // Completed Method
        private void BackgroundWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Cancelled)
            {
            }
            else if (e.Error != null)
            {
            }
            else
            {
                //checking the end of the experiment and no background process is running and for the last background thread
                if (_completedImageCount == _totalImageCount && !_backgroundWorker.IsBusy)
                {
                    // event trigerred to the view model to change the status of the Start and Stop button
                    UpdateButton(true);

                    // event trigerred to the view model to change the status of the menu bar buttons
                    UpdateMenuBarButton(true);

                    // event trigerred to the view model to change the status message
                    Update(_statusMessage);

                    _experimentStatus = "Complete";

                    LogPMTTripCount();

                    UpdateExperimentFile(_experimentXMLPath, true);

                }
                else
                {
                    if (true == _stopClicked)
                    {
                        _experimentStatus = "Stopped";

                        LogPMTTripCount();

                        UpdateExperimentFile(_experimentXMLPath, true);
                    }
                }
            }
        }

        private void BuildColorImage()
        {
            if (_experimentFolderPath == null)
            {
                return;
            }

            if (1 == ImageMethod) //DFLIM Image
            {
                string str = string.Empty;
                int imgIndxDigiCnts = (int)Constants.DEFAULT_FILE_FORMAT_DIGITS;
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                if (null != appSettings)
                {
                    XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/ImageNameFormat");
                    imgIndxDigiCnts = (XmlManager.GetAttribute(node, appSettings, "indexDigitCounts", ref str) && Int32.TryParse(str, out imgIndxDigiCnts)) ? imgIndxDigiCnts : (int)Constants.DEFAULT_FILE_FORMAT_DIGITS;
                }
                string imgNameFormat = "D" + imgIndxDigiCnts.ToString();
                List<string> fileNames = new List<string>();
                _foundChannelCount = 0;

                string exppath = _experimentFolderPath;

                StringBuilder sbTemp = new StringBuilder();
                sbTemp.AppendFormat("{0}{1}{2}{3}{4}{5}", exppath, "Image", "_" + ((int)1).ToString(imgNameFormat),
                    "_" + _currentSubImageCount.ToString(imgNameFormat), "_" + _currentZCount.ToString(imgNameFormat), "_" + _currentTCount.ToString(imgNameFormat) + ".dFLIM");
                string strTemp = sbTemp.ToString();

                StringBuilder sbPreview = new StringBuilder();
                sbPreview.AppendFormat("{0}Preview.dFLIM", exppath);
                string strPreview = sbPreview.ToString();

                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Building color image of " + strTemp);

                if (File.Exists(strPreview))
                {
                    fileNames.Add(strPreview);
                    _foundChannelCount++;
                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Built color image of " + strPreview);
                }
                else if (File.Exists(strTemp))
                {
                    fileNames.Add(strTemp);
                    _foundChannelCount++;
                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Built color image of " + strTemp);
                }

                if (_foundChannelCount > 0)
                {
                    //set the image file path for display
                    DisplayColorImage(fileNames);
                }
            }
            else
            {

                string str = string.Empty;
                string exppath = _experimentFolderPath;
                int imgIndxDigiCnts = (int)Constants.DEFAULT_FILE_FORMAT_DIGITS;
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                if (null != appSettings)
                {
                    XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/ImageNameFormat");
                    imgIndxDigiCnts = (XmlManager.GetAttribute(node, appSettings, "indexDigitCounts", ref str) && Int32.TryParse(str, out imgIndxDigiCnts)) ? imgIndxDigiCnts : (int)Constants.DEFAULT_FILE_FORMAT_DIGITS;
                }
                string imgNameFormat = "D" + imgIndxDigiCnts.ToString();

                XmlNodeList wavelengthNodes = ExperimentDoc.SelectNodes("/ThorImageExperiment/Wavelengths/Wavelength");
                _foundChannelCount = 0;

                if (_isSequentialCapture)
                {
                    wavelengthNodes = ExperimentDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep/Wavelengths/Wavelength");
                }

                if (_ismROICapture)
                {
                    for (int j = 0; j < _mROIs.Count; ++j)
                    {
                        List<string> fileNames = new List<string>();
                        var roi = _mROIs[j];
                        for (int i = 0; i < wavelengthNodes.Count; i++)
                        {
                            str = wavelengthNodes[i].Attributes["name"].Value.ToString();

                            StringBuilder sbTemp = new StringBuilder();
                            sbTemp.AppendFormat("{0}{1}{2}{3}{4}{5}", exppath, str, "_" + ((int)1).ToString(imgNameFormat),
                                "_" + _currentSubImageCount.ToString(imgNameFormat), "_" + _currentZCount.ToString(imgNameFormat), "_" + _currentTCount.ToString(imgNameFormat) + ".tif");
                            string strTemp = sbTemp.ToString();

                            StringBuilder sbPreview = new StringBuilder();
                            sbPreview.AppendFormat("{0}{1}_region_{2}_Preview.tif", exppath, str, roi.ScanAreaID);
                            string strPreview = sbPreview.ToString();

                            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Building color image of " + strTemp);

                            if (File.Exists(strPreview))
                            {
                                fileNames.Add(strPreview);
                                _foundChannelCount++;
                                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Built color image of " + strPreview);
                            }
                            else if (File.Exists(strTemp))
                            {
                                fileNames.Add(strTemp);
                                _foundChannelCount++;
                                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Built color image of " + strTemp);
                            }
                        }
                        if (_foundChannelCount > 0)
                        {
                            //set the image file path for display
                            DisplayColorImage(fileNames, roi.ScanAreaID, j);
                        }
                    }
                }
                else
                {
                    List<string> fileNames = new List<string>();
                    for (int i = 0; i < wavelengthNodes.Count; i++)
                    {

                        str = wavelengthNodes[i].Attributes["name"].Value.ToString();

                        StringBuilder sbTemp = new StringBuilder();
                        sbTemp.AppendFormat("{0}{1}{2}{3}{4}{5}", exppath, str, "_" + ((int)1).ToString(imgNameFormat),
                            "_" + _currentSubImageCount.ToString(imgNameFormat), "_" + _currentZCount.ToString(imgNameFormat), "_" + _currentTCount.ToString(imgNameFormat) + ".tif");
                        string strTemp = sbTemp.ToString();

                        StringBuilder sbPreview = new StringBuilder();
                        sbPreview.AppendFormat("{0}{1}_Preview.tif", exppath, str);
                        string strPreview = sbPreview.ToString();

                        ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Building color image of " + strTemp);

                        if (File.Exists(strPreview))
                        {
                            fileNames.Add(strPreview);
                            _foundChannelCount++;
                            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Built color image of " + strPreview);
                        }
                        else if (File.Exists(strTemp))
                        {
                            fileNames.Add(strTemp);
                            _foundChannelCount++;
                            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Built color image of " + strTemp);
                        }
                    }

                    if (_foundChannelCount > 0)
                    {
                        //set the image file path for display
                        DisplayColorImage(fileNames);
                    }
                }
            }
        }

        /// <summary>
        /// Stops experiment without updating experiment file.
        /// </summary>
        private void CancelAcquisition()
        {
            _experimentStatus = "Stopped";

            // App inherits from Application, and has a Window property called MainWindow
            // and a List<Window> property called OpenWindows.
            Application.Current.MainWindow.Activate();

            _completedImageCount = _currentImageCount = _currentSubImageCount = 0;

            // event trigerred to the view model to update the status of the progress feedback
            StatusMessage = "The experiment has stopped";

            _startButtonStatus = true;
            _stopButtonStatus = false;
            UpdateButton(true);
            // event trigerred to the view model to change the status of the menu bar button
            UpdateMenuBarButton(true);

            _panelsEnable = true;
            // event trigerred to the view model to enable the Panels
            UpdatePanels(_panelsEnable);

            IsStimulusSaving = false;
            _runComplete = true;
            _backgroundWorkerDone = true;
            //notify any running script that the command has stopped
            UpdateScript("Stop");
        }

        private void CopyChannelData(int regionID, int regionIndex)
        {
            lock (_frameData.dataLock)
            {
                _dataLength = (_imageWidth * _imageHeight);

                if ((_pixelData == null) || (_pixelData.Length != (_dataLength * _imageColorChannels)))
                {
                    _pixelData = new ushort[_dataLength * _imageColorChannels];
                }

                if (1 == ImageMethod) //dFLIM
                {
                    const int DFLIM_HISTOGRAM_BINS = 256;

                    //DFLIM data description
                    //1 datalength for photon num buffer (intensity) (USHORT)
                    //1 datalength for single photon sum buffer (USHORT)
                    //2 datalength for arrival time sum buffer (UINT32)
                    //4 DFLIM_HISTOGRAM_BINS for dflim histogram (UINT32)

                    const int SHORTS_PER_BIN = 2;
                    //copy out dflim histogram buffer
                    lock (_dflimHistogramDataLock)
                    {
                        if (_dflimHistogramData == null || _dflimHistogramData.Length != _imageColorChannels)
                        {
                            _dflimHistogramData = new uint[_imageColorChannels][];
                            for (int i = 0; i < _dflimHistogramData.Length; ++i)
                            {
                                _dflimHistogramData[i] = new uint[DFLIM_HISTOGRAM_BINS];
                            }
                        }

                        for (int i = 0; i < _imageColorChannels; ++i)
                        {
                            MemoryCopyManager.CopyIntPtrMemory(_imageData, i * DFLIM_HISTOGRAM_BINS, _dflimHistogramData[i], 0, DFLIM_HISTOGRAM_BINS);
                        }

                        _dflimNewHistogramData = true;
                    }

                    if ((_dflimSinglePhotonData == null) ||
                        (_dflimSinglePhotonData.Length != (_dataLength * _imageColorChannels)))
                    {
                        _dflimSinglePhotonData = new ushort[_dataLength * _imageColorChannels];
                    }
                    if ((_dflimArrivalTimeSumData == null) ||
                        (_dflimArrivalTimeSumData.Length != (_dataLength * _imageColorChannels)))
                    {
                        _dflimArrivalTimeSumData = new uint[_dataLength * _imageColorChannels];
                    }

                    //copy out the pixel data
                    MemoryCopyManager.CopyIntPtrMemory(_imageData, DFLIM_HISTOGRAM_BINS * SHORTS_PER_BIN * _imageColorChannels, _pixelData, 0, _dataLength * _imageColorChannels);

                    //copy out dflim single photon data buffer
                    MemoryCopyManager.CopyIntPtrMemory(_imageData, DFLIM_HISTOGRAM_BINS * SHORTS_PER_BIN * _imageColorChannels + _dataLength * _imageColorChannels, _dflimSinglePhotonData, 0, _dataLength * _imageColorChannels);

                    //copy out dflim arrival time sum data buffer
                    MemoryCopyManager.CopyIntPtrMemory(_imageData, DFLIM_HISTOGRAM_BINS * _imageColorChannels + _dataLength * _imageColorChannels, _dflimArrivalTimeSumData, 0, _dataLength * _imageColorChannels);

                    for (int k = 0; k < MAX_CHANNELS; k++)
                    {
                        int histoIndex = (_foundChannelCount > 1) ? k : 0;

                        if (null == _dflimHistogramData || _dflimHistogramData[histoIndex].Length < 256 ||
                            0 == _dflimHistogramData[histoIndex][254] || 0 == _dflimHistogramData[histoIndex][255])
                        {
                            continue;
                        }
                        _dFLIMBinDurations[k] = 5.0 * (double)_dflimHistogramData[histoIndex][255] / 128.0 / (double)_dflimHistogramData[histoIndex][254];
                    }
                }
                else
                {
                    switch (_imageColorChannels)
                    {
                        case 1:
                            {
                                MemoryCopyManager.CopyIntPtrMemory(_imageData, _pixelData, 0, _dataLength);
                            }
                            break;
                        default:
                            {
                                MemoryCopyManager.CopyIntPtrMemory(_imageData, _pixelData, 0, _dataLength * _imageColorChannels);
                            }
                            break;
                    }
                }

                FrameInfoStruct imageInfo = new FrameInfoStruct();
                if (_ismROICapture)
                {
                    int top = (int)Math.Floor((double)_mROIFullFOVMetadata.PixelHeight / 2 + _mROIs[regionIndex].PositionYUM / _mROIPixelSizeYUM);
                    int left = (int)Math.Floor((double)_mROIFullFOVMetadata.PixelWidth / 2 + (_mROIs[regionIndex].PositionXUM - _mROIs[regionIndex].PhysicalSizeXUM / _mROIs[regionIndex].Stripes / 2) / _mROIPixelSizeXUM);
                    imageInfo.bufferType = ImageMethod;
                    imageInfo.numberOfPlanes = NumberOfPlanes;
                    imageInfo.fullFrame = 1;
                    imageInfo.copySize = (ulong)(_dataLength * _imageColorChannels);
                    imageInfo.channels = _imageColorChannels;
                    imageInfo.isNewMROIFrame = regionIndex == 0 ? 1 : 0;
                    imageInfo.isMROI = _ismROICapture ? 1 : 0;
                    imageInfo.totalScanAreas = _mROIs.Count;
                    imageInfo.scanAreaIndex = regionIndex;
                    imageInfo.scanAreaID = regionID;
                    imageInfo.fullImageWidth = _mROIFullFOVMetadata.PixelWidth;
                    imageInfo.fullImageHeight = _mROIFullFOVMetadata.PixelHeight;
                    imageInfo.topInFullImage = top;
                    imageInfo.leftInFullImage = left;
                    imageInfo.imageHeight = _imageHeight;
                    imageInfo.imageWidth = _imageWidth;
                }
                else
                {
                    imageInfo.bufferType = ImageMethod;
                    imageInfo.imageHeight = (NumberOfPlanes > 1) ? _imageHeight / NumberOfPlanes : _imageHeight;
                    imageInfo.imageWidth = _imageWidth;
                    imageInfo.numberOfPlanes = NumberOfPlanes;
                    imageInfo.fullFrame = 1;
                    imageInfo.copySize = (ulong)(_dataLength * _imageColorChannels);
                    imageInfo.channels = _imageColorChannels;
                    imageInfo.sequenceIndex = _channelOrderCurrentIndex;
                    imageInfo.totalSequences = _sequenceStepsNum;
                    imageInfo.sequenceSelectedChannels = _channelOrderCurrentLSMChannel;
                }

                _frameData.pixelData = _pixelData;
                _frameData.bitsPerPixel = GetBitsPerPixel();
                _frameData.dFLIMArrivalTimeSumData = _dflimArrivalTimeSumData;
                _frameData.dFLIMSinglePhotonData = _dflimSinglePhotonData;
                _frameData.dFLIMBinDurations = _dFLIMBinDurations;
                _frameData.averageMode = _averageMode;
                _frameData.averageFrameCount = _averageNum;
                _frameData.contiguousChannels = true;
                _frameData.channelSelection = _channelSelection;
                _frameData.frameInfo = imageInfo;
                _frameData.frameInfo.pixelAspectRatioYScale = (int)(PixelAspectRatioYScale * 100);
                _frameData.pixelSizeUM = PixelSizeUM;
                _frameData.isFastZPreviewImage = ZFastEnable && CaptureMode == (int)CaptureModes.STREAMING;
            }

            MVMManager.Instance["ImageViewCaptureVM", "FrameData"] = _frameData;
        }

        private XmlElement CreateWavelengthTag(string name, int exposureTime)
        {
            //create a new XML tag for the wavelength settings
            XmlElement newElement = this.ExperimentDoc.CreateElement("Wavelength");

            XmlAttribute nameAttribute = this.ExperimentDoc.CreateAttribute("name");
            XmlAttribute expAttribute = this.ExperimentDoc.CreateAttribute("exposureTimeMS");

            nameAttribute.Value = name;
            expAttribute.Value = exposureTime.ToString();

            newElement.Attributes.Append(nameAttribute);
            newElement.Attributes.Append(expAttribute);

            return newElement;
        }

        private void GetMCLSChannelInfo(XmlDocument expDoc, XmlNodeList nodeList, string enableString, string powerString, ref Visibility vis, ref double power)
        {
            string strTemp = string.Empty;

            if (XmlManager.GetAttribute(nodeList[0], expDoc, enableString, ref strTemp))
            {
                if (strTemp.Equals("1"))
                {
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, powerString, ref strTemp))
                    {
                        vis = Visibility.Visible;
                        power = Convert.ToDouble(strTemp);
                    }
                }
                else
                {
                    vis = Visibility.Collapsed;
                    power = 0;
                }
            }
        }

        //Get the Output path from the latest experiment on the database
        private string GetPreviousExperimentOutputPath()
        {
            string appSettingsFolder = Application.Current.Resources["ApplicationSettingsFolder"].ToString();

            if (Directory.Exists(appSettingsFolder))
            {
                try
                {
                    DataStore.Instance.ConnectionString = string.Format("URI=file:{0}\\{1}", appSettingsFolder, "thorDatabase.db");
                    DataStore.Instance.Open();
                    DataRow row = DataStore.Instance.ExperimentsDataSet.Tables[0].Rows[DataStore.Instance.ExperimentsDataSet.Tables[0].Rows.Count - 1];
                    DataStore.Instance.Close();
                    return System.IO.Directory.GetParent(System.IO.Directory.GetParent(row["Path"].ToString()).ToString()).ToString();
                }
                catch
                {
                    return Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString();
                }
            }
            else
            {
                return Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString();
            }
        }

        /// <summary>
        /// Gets the tiff compression enbaled setting.
        /// </summary>
        /// <returns></returns>
        private bool GetTiffCompressionEnbaledSetting()
        {
            bool ret = true;
            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/TIFFCompressionEnable");
            if (node == null) // if not exist, add the compression option
            {
                node = appSettings.CreateNode(XmlNodeType.Element, "TIFFCompressionEnable", null);
                appSettings.DocumentElement.AppendChild(node);
                XmlManager.SetAttribute(node, appSettings, "value", "1");
                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
                ret = true;
            }
            else
            {
                string str = string.Empty;
                XmlManager.GetAttribute(node, appSettings, "value", ref str);
                ret = (str == "1");
            }
            return ret;
        }

        /// <summary>
        ///When displaying tiled images, use this Function to retrieve the location in _pixelData.
        ///The location corresponds to the mouse pointer x,y coordinate. 
        /// </summary>        
        private int GetTilePixelDataLocation(int x, int y)
        {
            int location = 0;
            bool isFirtXhalf = (_imageWidth / 2 > x) ? true : false;
            int tileX = (isFirtXhalf) ? x : x - _imageWidth / 2;

            bool isFirtYhalf = (_imageHeight / 2 > y) ? true : false;
            int tileY = (isFirtYhalf) ? y : y - _imageHeight / 2;
            int pixelsPerTile = (_imageWidth / 2) * (_imageHeight / 2);
            int tileOffsetX = (isFirtXhalf) ? 0 : pixelsPerTile;
            int tileOffsetY = (isFirtYhalf) ? 0 : 2 * pixelsPerTile;

            int tileOffset = tileOffsetX + tileOffsetY;
            location = tileX + (_imageWidth / 2) * tileY + tileOffset;

            return location;
        }

        private void InitializeBleachPixelArrayParams()
        {
            double bFieldSize = 0, bScaleYScan = 0, bVerticalScan = 0, bHorizontalFlip = 0, bPixelX = 0, bPixelY = 0, bFieldOffsetXActual = 0, bFieldOffsetYActual = 0;
            double bFieldOffsetXFine = 0, bFieldOffsetYFine = 0, bFieldScaleXFine = 0, bFieldScaleYFine = 0;
            double[] bPowerMin = new double[1] { 0 };
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, ref bFieldSize);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_Y_AMPLITUDE_SCALER, ref bScaleYScan);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_VERTICAL_SCAN_DIRECTION, ref bVerticalScan);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_HORIZONTAL_FLIP, ref bHorizontalFlip);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_0, ref bPowerMin[0]);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_PIXEL_X, ref bPixelX);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_PIXEL_Y, ref bPixelY);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_OFFSET_X, ref bFieldOffsetXActual);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_OFFSET_Y, ref bFieldOffsetYActual);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_X, ref bFieldOffsetXFine);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_Y, ref bFieldOffsetYFine);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_X, ref bFieldScaleXFine);
            ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y, ref bFieldScaleYFine);

            int[] Pixel = { (int)bPixelX, (int)bPixelY };
            int[] OffsetActual = { (int)bFieldOffsetXActual, (int)bFieldOffsetYActual };
            double[] OffsetFine = { bFieldOffsetXFine, bFieldOffsetYFine };
            double[] FieldScaleFine = { bFieldScaleXFine, bFieldScaleYFine };
            WaveformBuilder.InitializeParams((int)bFieldSize, FieldScaleFine, Pixel, OffsetActual, OffsetFine, bScaleYScan / 100, (int)bVerticalScan, (int)bHorizontalFlip, bPowerMin, WaveformDriverType, PowerShiftUS);
        }

        bool IsXyViewVisible()
        {
            string str = string.Empty;
            XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList nl = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");
            XmlManager.GetAttribute(nl[0], appDoc, "Visibility", ref str);
            return (str.CompareTo("Visible") == 0);
        }

        /// <summary>
        /// load additional SLM pattern, waveform and sequence per cycle callback, use waveform indexes as reference.
        /// </summary>
        /// <param name="lastInCycle"></param>
        /// <returns></returns>
        private bool LoadSLMPatternAndFile()
        {
            bool cycleReset = false;
            PreLoadNextSLMPattern();
            string LoadedSLMName = PreLoadNextSLM();
            LoadedSLMName = (PreLoadNextSLMSequence(LoadedSLMName)) ? LoadedSLMName : string.Empty;

            //check if loaded:
            if (string.Empty == LoadedSLMName)
            {
                //no more added files located, reset counters before next cycle:
                if ((BleachFrames - 1) > _currentSLMCycleID)
                {
                    _currentSLMWaveID = _currentSLMSequenceID = 0;
                    _loadedSLMFiles.Clear();

                    if (1 < _loadedSLMSequences.Count)
                    {
                        _loadedSLMSequences.Clear();
                    }

                    cycleReset = true;
                    //try search drop-in files for next cycle:
                    LoadedSLMName = PreLoadNextSLM();
                    LoadedSLMName = (PreLoadNextSLMSequence(LoadedSLMName)) ? LoadedSLMName : string.Empty;
                }
                _currentSLMCycleID++;
            }

            //sleep delay time,
            //do cycle delay at cycle end and do sequence delay in advance sequence mode
            _SLMCallbackCount++;
            if (1 < _SLMCallbackCount)
            {
                if (cycleReset)
                {
                    if (string.Empty != LoadedSLMName && !SLMBleachDelayEx(SLMBleachDelay[0]))
                    {
                        return false;
                    }
                }
                else if (SLMSequenceOn)
                {
                    if (string.Empty != LoadedSLMName && !SLMBleachDelayEx(SLMBleachDelay[1]))
                    {
                        return false;
                    }
                }
            }
            return (string.Empty != LoadedSLMName || (BleachFrames - 1) <= _currentSLMCycleID);
        }

        /// <summary>
        /// Check PMT safety status and log in HW settings.
        /// </summary>
        private void LogPMTTripCount()
        {
            if (!GetPMTSafetyStatus())
            {
                _pmtTripCount++;
                this.HardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
                if (null != this.HardwareDoc)
                {
                    XmlNodeList ndListHW = this.HardwareDoc.SelectNodes("/HardwareSettings/PMT");
                    if (ndListHW.Count > 0)
                    {
                        string str = string.Empty;
                        if (XmlManager.GetAttribute(ndListHW[0], this.HardwareDoc, "tripCount", ref str))
                        {
                            XmlManager.SetAttribute(ndListHW[0], this.HardwareDoc, "tripCount", _pmtTripCount.ToString());
                        }
                    }
                    MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS, true);
                }
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

        private bool PreBleachCheck()
        {
            bool ret = true;
            string bROISource = ResourceManagerCS.GetCaptureTemplatePathString() + "BleachROIs.xaml";
            string bWaveformSource = ResourceManagerCS.GetCaptureTemplatePathString() + "BleachWaveform.raw";
            string bROIPath = _experimentFolderPath + "BleachROIs.xaml";
            string bWaveformPath = _experimentFolderPath + "BleachWaveform.raw";

            switch (CurrentBleachMode)
            {
                case BleachMode.BLEACH:
                    if (File.Exists(bROISource))
                    {
                        File.Copy(bROISource, bROIPath);
                    }
                    if (File.Exists(bWaveformSource))
                    {
                        File.Copy(bWaveformSource, bWaveformPath);
                    }
                    break;
                case BleachMode.SLM:
                    if (Directory.Exists(SLMWaveformFolder[0]))
                    {
                        StatusMessage = "Start loading SLM ...";
                        Directory.CreateDirectory(SLMWaveformPath[0]);
                        DirectoryInfo diSource = new DirectoryInfo(SLMWaveformFolder[0]);
                        DirectoryInfo diPath = new DirectoryInfo(SLMWaveformPath[0]);
                        foreach (FileInfo finfo in diSource.GetFiles())
                        {
                            finfo.CopyTo(Path.Combine(diPath.FullName, finfo.Name), true);
                            if (_slmLoader.CancellationPending)
                                return false;
                        }
                        //copy sequences if selected
                        if (SLMSequenceOn && Directory.Exists(SLMWaveformFolder[1]))
                        {
                            Directory.CreateDirectory(SLMWaveformPath[1]);
                            diSource = new DirectoryInfo(SLMWaveformFolder[1]);
                            diPath = new DirectoryInfo(SLMWaveformPath[1]);
                            foreach (FileInfo finfo in diSource.GetFiles())
                            {
                                finfo.CopyTo(Path.Combine(diPath.FullName, finfo.Name), true);
                                if (_slmLoader.CancellationPending)
                                    return false;
                            }
                        }
                    }
                    break;
                default:
                    break;
            }

            //Prepare for long term idle pixel bleaching:
            _bleachPixelArrayList.Clear();
            if (0 == BleachLongIdle)
            {
                WaveformBuilder.ResetPixelArray();
            }
            else
            {
                ret = RunSampleLSBleachPixelArray(BleachWParams);
            }
            //reset pixel index for callback:
            _bleachFrameIndex = _bleachPixelIndex = _bleachPixelArrayIndex = 0;

            //initialize before SLM Bleach:
            _currentSLMWaveID = _currentSLMCycleID = _currentSLMSequenceID = 0;
            _lastInSLMCycle = false;
            _loadedSLMPatterns.Clear();
            _loadedSLMFiles.Clear();
            _loadedSLMSequences.Clear();

            if (BleachMode.SLM == _bleachScanMode)
            {
                //preload all available SLM patterns:
                ret = PreLoadAllSLMPattern();
            }
            return ret;
        }

        private bool PreLoadAllSLMPattern()
        {
            if (!Directory.Exists(SLMWaveformPath[0]))
                return false;

            //reset counts:
            _loadedSLMPatternsCnt = 0;

            //default pattern based on current setting,
            //but follow at edit when available: [0]:raw (phase to be processed), [1]:phase
            SLMphaseType = new List<int>();

            //get files in folder:
            List<string> slmPatternsInFolder = Directory.EnumerateFiles(SLMWaveformPath[0], "*.bmp ", SearchOption.TopDirectoryOnly).ToList();

            //locate filenames to load from the list:
            for (int i = 0; i < _slmBleachWaveParams.Count; i++)
            {
                string waveFileName = SLMWaveformPath[0] + "\\" + SLMWaveBaseName[1] + "_" + _slmBleachWaveParams[i].BleachWaveParams.ID.ToString("D" + FileName.GetDigitCounts().ToString()) + ".bmp";

                string matchingFileName = slmPatternsInFolder.FirstOrDefault(checkString => checkString.Contains(waveFileName));
                if (null != matchingFileName)
                {
                    _loadedSLMPatternsCnt++;
                    SLMphaseType.Add(_slmBleachWaveParams[i].PhaseType);
                    _loadedSLMPatterns.Add(waveFileName);
                }
            }

            //check if other files added:
            List<string> addedFiles = slmPatternsInFolder.Except(_loadedSLMPatterns).OrderBy(s => s).ToList();
            for (int i = 0; i < addedFiles.Count(); i++)
            {
                _loadedSLMPatternsCnt++;
                _loadedSLMPatterns.Add(addedFiles[i]);
            }

            //find out maximum pattern time for SLM timeout in msec and determine run-time calculation mode:
            int timeoutVal = 0;
            int setArmTrigger = 1;
            for (int i = 0; i < _slmBleachWaveParams.Count; i++)
            {
                timeoutVal = (int)Math.Max(timeoutVal, _slmBleachWaveParams[i].BleachWaveParams.PrePatIdleTime + _slmBleachWaveParams[i].BleachWaveParams.Iterations * (_slmBleachWaveParams[i].BleachWaveParams.PreIdleTime + _slmBleachWaveParams[i].Duration + _slmBleachWaveParams[i].BleachWaveParams.PostIdleTime) + _slmBleachWaveParams[i].BleachWaveParams.PostPatIdleTime);
                if (_slmBleachWaveParams[i].Duration < (double)Constants.REARM_TIME_MS)
                    setArmTrigger = 0;
            }
            ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_SET_ARM_TRIGGER, setArmTrigger, (int)IDevice.DeviceSetParamType.NO_EXECUTION);

            //keep slm phase type available if not from SLM param:
            if (1 > SLMphaseType.Count)
                SLMphaseType.Add(SLM3D ? 1 : 0);

            //load to SLM:
            bool phaseDirect = (int)ICamera.LSMType.STIMULATE_MODULATOR == ResourceManagerCS.GetBleacherType();
            for (int i = 0; i < _loadedSLMPatternsCnt; i++)
            {
                if (!_slmLoader.CancellationPending)
                {
                    StatusMessage = "Loading SLM pattern # (" + i + ")";
                    bool doStart = (i == (_loadedSLMPatternsCnt - 1)) ? true : false;
                    this.LoadSLMPatternName(SLMSequenceOn ? 1 : 0, i, _loadedSLMPatterns[i], doStart, phaseDirect, (i < SLMphaseType.Count ? SLMphaseType[i] : SLMphaseType[0]), timeoutVal);
                    System.Threading.Thread.Sleep(1);
                }
            }

            return (!_slmLoader.CancellationPending && 0 < _loadedSLMPatternsCnt) ? true : false;
        }

        private string PreLoadNextSLM()
        {
            string LoadedSLMName = string.Empty;
            Random rnd = new Random();

            if (!Directory.Exists((SLMSequenceOn ? SLMWaveformPath[1] : SLMWaveformPath[0])))
                return LoadedSLMName;

            //get files in folder:
            _slmFilesInFolder = Directory.EnumerateFiles((SLMSequenceOn ? SLMWaveformPath[1] : SLMWaveformPath[0]), "*.raw ", SearchOption.TopDirectoryOnly).ToList();
            List<string> candidates = _slmFilesInFolder.Except(_loadedSLMFiles).OrderBy(s => s).ToList();

            if (SLMSequenceOn && SLMRandomEpochs && 0 < candidates.Count)
            {
                //random select epoch waveform:
                string targetFile = candidates[rnd.Next(candidates.Count())];
                if (this.SetBleachFile(targetFile, 1))   //set cycle count = 1 to reload waveform everytime
                {
                    LoadedSLMName = targetFile;
                    _loadedSLMFiles.Add(LoadedSLMName);
                    _currentSLMWaveID++;
                }
            }
            else
            {
                //locate first filename to load from the list:
                int targetCount = SLMSequenceOn ? _slmFilesInFolder.Count : _slmBleachWaveParams.Count;
                while ((string.Empty == LoadedSLMName) && (targetCount > _currentSLMWaveID))
                {
                    string waveFileName = (SLMSequenceOn ? SLMWaveformPath[1] : SLMWaveformPath[0]) + "\\" + SLMWaveBaseName[0] + "_" +
                        (SLMSequenceOn ? (uint)(_currentSLMWaveID + 1) : _slmBleachWaveParams[_currentSLMWaveID].BleachWaveParams.ID).ToString("D" + FileName.GetDigitCounts().ToString()) + ".raw";

                    //check if available in the folder, skip if just loaded:
                    string matchingFileName = _slmFilesInFolder.FirstOrDefault(checkString => checkString.Contains(waveFileName));
                    if (null != matchingFileName && !_loadedSLMFiles.Contains(waveFileName))
                    {
                        if (this.SetBleachFile(matchingFileName, 1))
                        {
                            LoadedSLMName = waveFileName;
                            _loadedSLMFiles.Add(LoadedSLMName);
                            //_afterSLMCycle = false;
                        }
                    }
                    _currentSLMWaveID++;
                }
            }

            //flag only if there's more to load:
            _lastInSLMCycle = (0 < _slmFilesInFolder.Except(_loadedSLMFiles).OrderBy(s => s).ToList().Count()) ? false : true;

            //if not loaded from the list, check if other files added:
            if (string.Empty == LoadedSLMName)
            {
                //_afterSLMCycle = true;
                List<string> addedFiles = _slmFilesInFolder.Except(_loadedSLMFiles).OrderBy(s => s).ToList();
                if (0 < addedFiles.Count())
                {
                    //found send file name to load:
                    if (this.SetBleachFile(addedFiles[0], 1))
                    {
                        LoadedSLMName = addedFiles[0];
                        _loadedSLMFiles.Add(LoadedSLMName);
                    }
                }
                _lastInSLMCycle = (1 >= addedFiles.Count()) ? true : _lastInSLMCycle;
            }

            if (string.Empty != LoadedSLMName)
            {
                StatusMessage = string.Format("Loading cycle {0}, SLM stimulation: '{1}' ...", (_currentSLMCycleID + 1).ToString(), Path.GetFileNameWithoutExtension(LoadedSLMName));
                System.Threading.Thread.Sleep(1);
            }
            return LoadedSLMName;
        }

        private bool PreLoadNextSLMPattern()
        {
            int currentCnt = _loadedSLMPatternsCnt;

            //get files in folder:
            List<string> slmPatternsInFolder = Directory.EnumerateFiles(SLMWaveformPath[0], "*.bmp ", SearchOption.TopDirectoryOnly).ToList();

            //check if other files added:
            bool phaseDirect = (int)ICamera.LSMType.STIMULATE_MODULATOR == ResourceManagerCS.GetBleacherType();
            List<string> addedFiles = slmPatternsInFolder.Except(_loadedSLMPatterns).OrderBy(s => s).ToList();
            for (int i = 0; i < addedFiles.Count(); i++)
            {
                //use last available SLM phaseType since no records of it from other files:
                if (this.LoadSLMPatternName(SLMSequenceOn ? 1 : 0, _loadedSLMPatternsCnt, addedFiles[i], true, phaseDirect, SLMphaseType.Last()))
                {
                    _loadedSLMPatternsCnt++;
                    _loadedSLMPatterns.Add(addedFiles[i]);
                }
            }

            return (currentCnt < _loadedSLMPatternsCnt) ? true : false;
        }

        /// <summary>
        /// load next SLM Sequence, return true if loaded new and in sync with PreLoadNextSLM
        /// </summary>
        /// <returns></returns>
        private bool PreLoadNextSLMSequence(string LoadedSLMName)
        {
            //return if not in advance sequence mode
            if (!SLMSequenceOn)
                return true;

            string LoadedSequenceName = string.Empty, seqFileName = string.Empty;

            //get files in folder:
            _slmSequencesInFolder = Directory.EnumerateFiles(SLMWaveformPath[1], "*.txt ", SearchOption.TopDirectoryOnly).ToList();

            //locate first filename to load from the list:
            while ((string.Empty == LoadedSequenceName) && (_slmSequencesInFolder.Count > _currentSLMSequenceID))
            {
                seqFileName = SLMWaveformPath[1] + "\\" + SLMWaveBaseName[2] + "_" + (_currentSLMSequenceID + 1).ToString("D" + FileName.GetDigitCounts().ToString()) + ".txt";

                //check if available in the folder, skip if just loaded:
                string loadedFileName = _loadedSLMSequences.FirstOrDefault(checkString => checkString.Contains(seqFileName));
                if (null != loadedFileName)
                {
                    LoadedSequenceName = loadedFileName;
                }
                else
                {
                    string matchingFileName = _slmSequencesInFolder.FirstOrDefault(checkString => checkString.Contains(seqFileName));
                    if (null != matchingFileName)
                    {
                        if (1 == ResourceManagerCS.SetDeviceParamString((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_SEQ_FILENAME, matchingFileName, (int)IDevice.DeviceSetParamType.NO_EXECUTION))
                        {
                            LoadedSequenceName = matchingFileName;
                            _loadedSLMSequences.Add(LoadedSequenceName);
                            //_afterSLMCycle = false;
                        }
                    }
                }
                _currentSLMSequenceID++;
            }

            //flag only if there's more to load:
            _lastInSLMCycle = (0 < _slmSequencesInFolder.Except(_loadedSLMSequences).OrderBy(s => s).ToList().Count()) ? false : true;

            //if not loaded from the list, check if other files added:
            if (String.IsNullOrEmpty(LoadedSequenceName))
            {
                //_afterSLMCycle = true;
                List<string> addedFiles = _slmSequencesInFolder.Except(_loadedSLMSequences).OrderBy(s => s).ToList();
                if (0 < addedFiles.Count())
                {
                    if (1 == ResourceManagerCS.SetDeviceParamString((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_SEQ_FILENAME, addedFiles[0], (int)IDevice.DeviceSetParamType.NO_EXECUTION))
                    {
                        LoadedSequenceName = addedFiles[0];
                        _loadedSLMSequences.Add(LoadedSequenceName);
                    }
                }
                _lastInSLMCycle = (1 >= addedFiles.Count()) ? true : _lastInSLMCycle;
            }

            if (!String.IsNullOrEmpty(LoadedSequenceName))
            {
                StatusMessage = string.Format("Loading cycle {0}, SLM sequence: '{1}' ...", (_currentSLMCycleID + 1).ToString(), Path.GetFileNameWithoutExtension(LoadedSequenceName));
            }

            return String.IsNullOrEmpty(LoadedSequenceName) ? false : true;
        }

        private bool RunSampleLSBleachPixelArray(List<BleachWaveParams> bParams)
        {
            if (bParams.Count <= 0)
            {
                return false;
            }
            InitializeBleachPixelArrayParams();

            foreach (BleachWaveParams bw in bParams)
            {
                PixelArray pxArray = new PixelArray();
                WaveformBuilder.ResetPixelArray();

                switch (bw.shapeType)
                {
                    case "Rectangle":
                        switch (bw.Fill)
                        {
                            case (int)BleachWaveParams.FillChoice.Raster:
                                WaveformBuilder.PixelRectTopDown(bw);
                                break;
                            default:
                                WaveformBuilder.PixelRectContour(bw);
                                break;
                        }
                        break;
                    case "Polygon":
                        WaveformBuilder.PixelPolygon(bw);
                        break;
                    case "Crosshair":
                        WaveformBuilder.PixelSpot(bw);
                        break;
                    case "Line":
                        WaveformBuilder.PixelLine(bw);
                        break;
                    case "Polyline":
                        WaveformBuilder.PixelPolyTrace(bw, false);
                        break;
                    case "Ellipse":
                        WaveformBuilder.PixelEllipse(bw);
                        break;
                    default:
                        break;
                }
                WaveformBuilder.GetPixelArray().AppendTo(ref pxArray);
                _bleachPixelArrayList.Add(pxArray);
            }
            return true;
        }

        private void RunsampleLSNotifySavedFileToIPC(string message)
        {
            MVMManager.Instance["RemoteIPCControlViewModelBase", "NotifyOfSavedFile"] = message;
        }

        //to update the completed working WELL status and progress feedback
        private void RunSampleLSUpdate(ref int index, ref int completed, ref int total, ref int timeElapsed, ref int timeRemaining, ref int captureComlete)
        {
            if (true == _backgroundWorkerDone)
            {
                int lCompleted = completed;
                int lTotal = total;
                if (CaptureModes.T_AND_Z == (CaptureModes)CaptureMode && ZStreamEnable)
                {
                    lCompleted *= ZStreamFrames;
                    lTotal *= ZStreamFrames;
                }
                int h_elapse, m_elapse, s_elapse, h_remain, m_remain, s_remain, rem;

                h_elapse = timeElapsed / (1000 * 60 * 60);
                rem = (timeElapsed - h_elapse * (1000 * 60 * 60));
                m_elapse = rem / (1000 * 60);
                rem = (timeElapsed - h_elapse * (1000 * 60 * 60) - m_elapse * (1000 * 60));
                s_elapse = rem / 1000;

                h_remain = timeRemaining / (1000 * 60 * 60);
                rem = (timeRemaining - h_remain * (1000 * 60 * 60));
                m_remain = rem / (1000 * 60);
                rem = (timeRemaining - h_remain * (1000 * 60 * 60) - m_remain * (1000 * 60));
                s_remain = rem / 1000;

                _currentImageCount = index;
                _completedImageCount = Math.Min(lCompleted, lTotal);
                _totalImagesCompleted = lCompleted;
                _totalImageCount = lTotal;

                if ((_completedImageCount == _totalImageCount))
                {
                    Thread t2 = new Thread(delegate ()
                    {
                        Thread.Sleep(300);        //delay so there is a small amount of time after the last pulse in o-scope data
                        bool ipcDownlinkFlag = (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "IPCDownlinkFlag", (object)false];
                        bool remoteConnection = (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "RemoteConnection", (object)false];
                        if (ipcDownlinkFlag == false && remoteConnection == true)
                        {
                            MVMManager.Instance["RemoteIPCControlViewModelBase", "StopAcquisition"] = true;
                        }
                    });
                    t2.Start();
                }

                //user stopped the experiment
                if (_stopClicked == true)
                {
                    StatusMessage = string.Format("Experiment stopped. Completed {0} of {1}. Time elapsed {2}:{03:D2}:{04:D2}", lCompleted, lTotal, h_elapse, m_elapse, s_elapse);

                    _startButtonStatus = true;
                    _stopButtonStatus = false;
                    if (null != UpdateButton) UpdateButton(true);

                    _stopClicked = false;
                    _runComplete = true;

                    _currentZCount = 0;
                    _currentTCount = 0;
                    _currentWellCount = 1;
                    _currentSubImageCount = 1;

                    if (TurnOffMonitorsDuringCapture)
                    {
                        SendMessage(0xffff, WM_SYSCOMMAND, SC_MONITORPOWER, -1);//DLL function
                    }

                }
                else if ((false == _runComplete) && (captureComlete == 1) && (_completedImageCount == _totalImageCount))  //Capture and image saving is done
                {
                    //disable digital trigger
                    MVMManager.Instance["DigitalOutputSwitchesViewModel", "TriggerEnable"] = 0;

                    _experimentCompleteForProgressBar = true;
                    if ((1 == _zEnable) && (_statusMessageZ.Length > 0))
                    {
                        StatusMessage = string.Format("Completed {0} of {1}. Time elapsed {2}:{3:D2}:{4:D2}\r{5}", lCompleted, lTotal, h_elapse, m_elapse, s_elapse, _statusMessageZ);
                    }
                    else
                    {
                        StatusMessage = string.Format("Completed {0} of {1}. Time elapsed {2}:{3:D2}:{4:D2}", lCompleted, lTotal, h_elapse, m_elapse, s_elapse);
                    }

                    UpdateBitmapTimer(false);

                    // event trigerred to the view model to change the status of the Start and Stop button
                    _startButtonStatus = true;
                    _stopButtonStatus = false;
                    if (null != UpdateButton) UpdateButton(true);

                    //reset the image count
                    _currentImageCount = 0;
                    _currentSubImageCount = 0; // set to an invalid value so that the coloring is not affected

                    // event trigerred to the view model to change the status of the menu bar buttons
                    if (null != UpdateMenuBarButton) UpdateMenuBarButton(true);

                    _experimentStatus = "Complete";

                    LogPMTTripCount();

                    UpdateExperimentFile(_experimentXMLPath, true);
                    CaptureComplete();

                    if (0 == _runCompleteCount)
                    {
                        UpdateScript("Complete");
                    }
                    _runComplete = true;

                    _runCompleteCount++;

                    if (TurnOffMonitorsDuringCapture)
                    {
                        SendMessage(0xffff, WM_SYSCOMMAND, SC_MONITORPOWER, -1);//DLL function
                    }

                    if (null != UpdateButton) UpdateButton(true);

                    _runComplete = true;

                    //UpdateScript("Complete") needs to be the last function called when a Capture is complete. Otherwise
                    //it could try to start a new capture before the current one is completely done.
                    UpdateScript("Complete");

                    return;

                    //  _currentZCount = 1;
                    //  _currentTCount = 1;
                    //  _currentWellCount = 1;
                    //  _currentSubImageCount = 1;
                    //XmlDocument sysDoc = new XmlDocument();

                    //string sysFile = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\ThorImage\\Application Settings\\ApplicationSettings.xml";

                    //sysDoc.Load(sysFile);

                    //XmlNodeList ndList = sysDoc.SelectNodes("/ApplicationSettings/FtpInformation");

                    //string ftpusername = ndList[0].Attributes["username"].Value.ToString();
                    //string ftppassword = ndList[0].Attributes["password"].Value.ToString();
                    //string ftploc = ndList[0].Attributes["location"].Value.ToString();

                    //string ftpdetails = ExperimentManager.SetFtpDetails(ftpusername, ftppassword, ftploc);

                    //string pat = @"(.*)(\\Experiments\\)(.*)";
                    //string[] strResult = Regex.Split(_experimentXMLPath, pat);

                    //if(strResult.Length == 5)
                    //{
                    //    string relativePath = strResult[2] + strResult[3];
                    //    relativePath = relativePath.Replace("\\", "/");
                    //    string loginId = Application.Current.Resources["UserName"].ToString();

                    //    //User should have Admin rights to add the experiment information into the db

                    //    ExperimentManager expManager = new ExperimentManager();

                    //    string result = expManager.addExperimentInformationFromXML(relativePath);
                    //}
                }
                else
                {
                    if (lCompleted <= lTotal || 1 < _sequenceStepsNum)
                    {
                        if ((1 == _zEnable) && (_statusMessageZ.Length > 0))
                        {
                            // if stream and stimulus, then no remaining time displaying
                            if (((int)CaptureModes.STREAMING == CaptureMode) && (this.StreamStorageMode == 1))
                            {
                                StatusMessage = (0 == _sequenceStepsNum) ? string.Format("Progress {0} of {1}. Time elapsed {2}:{03:D2}:{04:D2}", lCompleted, lTotal, h_elapse, m_elapse, s_elapse) :
                                    string.Format("Progress {0} of {1}, sequence {2} of {3}. Time elapsed {4}:{05:D2}:{06:D2}", lCompleted, lTotal, _channelOrderCurrentIndex + 1, _sequenceStepsNum, h_elapse, m_elapse, s_elapse);
                            }
                            else
                            {
                                StatusMessage = (0 == _sequenceStepsNum) ? string.Format("Progress {0} of {1}. Time elapsed {2}:{03:D2}:{04:D2} remaining {5}:{06:D2}:{07:D2}\r{8}", lCompleted, lTotal, h_elapse, m_elapse, s_elapse, h_remain, m_remain, s_remain, _statusMessageZ) :
                                    string.Format("Progress {0} of {1}, sequence {2} of {3}. Time elapsed {4}:{05:D2}:{06:D2} remaining {7}:{08:D2}:{09:D2}\r{10}", lCompleted, lTotal, _channelOrderCurrentIndex + 1, _sequenceStepsNum, h_elapse, m_elapse, s_elapse, h_remain, m_remain, s_remain, _statusMessageZ);
                            }
                        }
                        else
                        {
                            // if stream and stimulus, then no remaining time displaying
                            if (((int)CaptureModes.STREAMING == CaptureMode) && (this.StreamStorageMode == 1))
                            {
                                StatusMessage = (0 == _sequenceStepsNum) ? string.Format("Progress {0} of {1}. Time elapsed {2}:{03:D2}:{04:D2}", lCompleted, lTotal, h_elapse, m_elapse, s_elapse) :
                                    string.Format("Progress {0} of {1}, sequence {2} of {3}. Time elapsed {4}:{05:D2}:{06:D2}", lCompleted, lTotal, _channelOrderCurrentIndex + 1, _sequenceStepsNum, h_elapse, m_elapse, s_elapse);
                            }
                            else
                            {
                                StatusMessage = (0 == _sequenceStepsNum) ? string.Format("Progress {0} of {1}. Time elapsed {2}:{03:D2}:{04:D2} remaining {5}:{06:D2}:{07:D2}", lCompleted, lTotal, h_elapse, m_elapse, s_elapse, h_remain, m_remain, s_remain) :
                                    string.Format("Progress {0} of {1}, sequence {2} of {3}. Time elapsed {4}:{05:D2}:{06:D2} remaining {7}:{08:D2}:{09:D2}", lCompleted, lTotal, _channelOrderCurrentIndex + 1, _sequenceStepsNum, h_elapse, m_elapse, s_elapse, h_remain, m_remain, s_remain);
                            }
                        }
                    }
                }
                //row counter increment
                _imageCounter++;

                if ((int)CaptureModes.STREAMING != CaptureMode)
                {
                    //if not currently working to display another image
                    _newImageAvailable = true;
                }
                //ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " RunSampleLSUpdate New image available");

                if (null != UpdateButton) UpdateButton(true);
            }
        }

        private void RunsampleLSUpdateMessage(string message)
        {
            StatusMessage = message;
        }

        //to update the completed working Sub WELL status
        private void RunSampleLSUpdateMosaicEnd(ref int index)
        {
            string message = "Current sub well number captured";

            // event trigerred to the view model to update the status of the well
            UpdateEndSubWell(message);

            UpdateActiveImage();

            // event trigerred to the view model to update image name
            UpdateImageName("SubImage " + index);

            _currentSubImageCount = index;

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Mosaic End Update");
        }

        //to update the current working Sub WELL status
        private void RunSampleLSUpdateMosaicStart(ref int index)
        {
            _currentSubImageCount = index;
            _previousSubImageCount = index;
            string message = "previous sub well number captured";

            // event trigerred to the view model to update the status of the well
            UpdateStartSubWell(message);
        }

        private void RunSampleLSUpdatePreCapture(ref int status)
        {
            if ((int)CaptureModes.BLEACHING == CaptureMode)
            {
                string bWaveformPath = _experimentFolderPath + "BleachWaveform.raw";

                //do messge display:
                switch (status)
                {
                    case (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_LOADING:
                        StatusMessage = string.Format("Loading stimulation waveform ...");
                        return;
                    case (int)PreCaptureStatus.PRECAPTURE_WAVEFORME_DONELOAD:
                        StatusMessage = string.Format("Start stimulation ...");
                        return;
                    case (int)PreCaptureStatus.PRECAPTURE_DONE:
                        StatusMessage = string.Format("Done stimulation ...");
                        return;
                }

                switch (_bleachScanMode)
                {
                    case BleachMode.BLEACH:
                        if (0 == _bleachPixelArrayList.Count)
                        {
                            //not long idle time:
                            this.SetBleachFile(bWaveformPath, BleachFrames);
                            status = (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_LAST_CYCLE;
                            return;
                        }
                        else
                        {
                            //regenerate waveform:
                            if ((BleachFrames == _bleachFrameIndex) && (_bleachPixelArrayIndex == _bleachPixelArrayList.Count))
                            {
                                status = (int)PreCaptureStatus.PRECAPTURE_DONE;
                                return;
                            }
                            //Wait long idle time:
                            if ((0 == _bleachFrameIndex) && (0 == _bleachPixelArrayIndex) && (0 == _bleachPixelIndex))
                            {
                                //set initial only, no need to wait
                            }
                            else
                            {
                                int timeWait = (int)((WaveformBuilder.MS_TO_S * BleachLongIdle) - new TimeSpan(DateTime.Now.Ticks - BleachLastRunTime.Ticks).TotalMilliseconds);
                                if (timeWait > 0)
                                {
                                    System.Threading.Thread.Sleep(timeWait);
                                }
                                //user stopped exp:
                                if (_runComplete)
                                {
                                    status = (int)PreCaptureStatus.PRECAPTURE_DONE;
                                    return;
                                }
                            }
                            BleachLastRunTime = DateTime.Now;

                            //regenerate waveform:
                            Point pt = _bleachPixelArrayList[_bleachPixelArrayIndex].GetPixel(_bleachPixelIndex);
                            InitializeBleachPixelArrayParams();
                            WaveformBuilder.ResetWaveform();
                            WaveformBuilder.BuildTravel(pt, 0, 0, 0);
                            WaveformBuilder.BuildSpot(BleachWParams[_bleachPixelArrayIndex], new double[1] { BleachWParams[_bleachPixelArrayIndex].Power });
                            if (((BleachFrames - 1) == _bleachFrameIndex) && (_bleachPixelArrayIndex == _bleachPixelArrayList.Count - 1) && (_bleachPixelIndex == _bleachPixelArrayList[_bleachPixelArrayIndex].Size - 1))
                            {
                                WaveformBuilder.ReturnHome(true);
                                status = (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_LAST_CYCLE;
                            }
                            else
                            {
                                WaveformBuilder.ReturnHome(false);
                                status = (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_MID_CYCLE;
                            }

                            //save waveform:
                            WaveformBuilder.SaveWaveform(bWaveformPath, false);

                            while (!WaveformBuilder.CheckSaveState())
                            {
                                System.Threading.Thread.Sleep(1);
                            }

                            if (!WaveformBuilder.GetSaveResult())
                            {
                                status = (int)PreCaptureStatus.PRECAPTURE_DONE;
                                return;
                            }

                            //set lower level:
                            this.SetBleachFile(bWaveformPath, 1);

                            //offset both list & pixel indexes:
                            if (_bleachPixelIndex < _bleachPixelArrayList[_bleachPixelArrayIndex].Size - 1)
                            {
                                _bleachPixelIndex++;
                            }
                            else if (_bleachPixelIndex == _bleachPixelArrayList[_bleachPixelArrayIndex].Size - 1)
                            {
                                _bleachPixelIndex = 0;
                                _bleachPixelArrayIndex++;
                                //next cycle:
                                if (_bleachPixelArrayList.Count == _bleachPixelArrayIndex)
                                {
                                    _bleachPixelArrayIndex = 0;
                                    _bleachFrameIndex++;
                                }
                            }
                        }
                        break;
                    case BleachMode.SLM:
                        if ((int)PreCaptureStatus.PRECAPTURE_BLEACHER_IDLE == status)
                        {
                            //go through cycles: the last one if no next file to load
                            if (!LoadSLMPatternAndFile())
                            {
                                status = (int)PreCaptureStatus.PRECAPTURE_DONE;
                            }
                            else if ((1 == BleachFrames) && (1 >= _slmFilesInFolder.Count))  //single pattern, single cycle
                            {
                                status = (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_LAST_CYCLE;
                            }
                            else
                            {
                                //last if last waveform of all cycles
                                status = (_lastInSLMCycle && (BleachFrames - 1) <= _currentSLMCycleID) ?
                                    (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_LAST_CYCLE : (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_MID_CYCLE;
                            }
                        }
                        break;
                }
            }
        }

        private void RunsampleLSUpdateSequenceStepCurrentIndx(ref int currentIndex)
        {
            _channelOrderCurrentIndex = currentIndex;   // zero-based index
            SetSequenceStepDisplaySettings(ExperimentDoc);
            SetSequenceStepMCLS(ExperimentDoc);
            SetSequenceDigitalSwitches(ExperimentDoc);
            if (_isSequentialCapture)
            {
                ChannelSelection |= _channelOrderCurrentLSMChannel;
            }
        }

        //to update the current working WELL status
        private void RunSampleLSUpdateStart(ref int index)
        {
            _currentWellCount = index;
            string message = "Current well number captured";

            // event trigerred to the view model to update the status of the well
            UpdateStart(message);
        }

        //to update the completed working Sub WELL status
        private void RunSampleLSUpdateT(ref int index)
        {
            _currentTCount = index;
            //   string message = "Current T index";

            // if _currentZCount is not updated, set it to 1
            // the images starting from index 1 while _currentZCount was initialized 0
            if (0 >= _currentZCount)
            {
                _currentZCount = 1;
            }

            if (0 >= _currentSubImageCount)
            {
                _currentSubImageCount = 1;
            }

            if ((int)CaptureModes.STREAMING == CaptureMode)
            {
                //if not currently working to display another image
                _newImageAvailable = true;
            }

            if ((0 == ZEnable) || CaptureMode != (int)CaptureModes.T_AND_Z)
            {
                UpdateActiveImage();
            }
            if (null != Update) Update(_statusMessage);

            //    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Timepoint Update");
        }

        //to update the completed working Sub WELL status
        private void RunSampleLSUpdateZ(ref int index, ref double power0, ref double power1, ref double power2, ref double power3, ref double power4, ref double power5)
        {
            _currentZCount = index;

            _power0 = power0;
            _power1 = power1;
            _power2 = power2;
            _power3 = power3;
            _power4 = power4;
            _power5 = power5;

            // if _currentTCount is not updated, set it to 1
            // the images starting from index 1 while _currentTCount was initialized 0
            if (0 > _currentTCount)
            {
                _currentTCount = 0;
            }

            if (0 >= _currentSubImageCount)
            {
                _currentSubImageCount = 1;
            }

            if (1 == ZEnable && 0 == CaptureMode)
            {
                if (1 == _currentZCount)
                {
                    _startZTime = DateTime.Now.Ticks;

                    if (_statusMessageZUpdate)
                    {
                        string formattedTime = string.Format("{0:D2}:{1:D2}:{2:D2}", 0, 0, 0);
                        _statusMessageZ = string.Format("Z stack time {0}", formattedTime);
                        _statusMessageZUpdate = false;
                    }
                }
                else if (_currentZCount == ZNumSteps)
                {

                    long finishTime = DateTime.Now.Ticks;

                    TimeSpan t = TimeSpan.FromTicks(finishTime - _startZTime);

                    string formattedTime = string.Format("{0:D2}:{1:D2}:{2:D2}",
                                        t.Hours + t.Days * 24,
                                        t.Minutes,
                                        t.Seconds);

                    _statusMessageZ = string.Format("Z stack time {0}", formattedTime);
                }
            }
            else
            {
                _statusMessageZ = String.Empty;
            }

            if ((1 == ZEnable) && CaptureMode == (int)CaptureModes.T_AND_Z)
            {
                UpdateActiveImage();
            }

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Z Slice Update");
        }

        private void SaveLastExperimentInfo(string path)
        {
            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/LastExperiment");
            if (node == null)
            {
                node = appSettings.CreateNode(XmlNodeType.Element, "LastExperiment", null);
                appSettings.DocumentElement.AppendChild(node);
            }

            XmlManager.SetAttribute(node, appSettings, "path", path);
            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
        }

        private void SetCaptureSequence()
        {
            _channelOrderCurrentIndex = 0;
            _sequenceStepsNum = 0;

            //Load active.xml accounting for possible lockup during scripting
            XmlDocument expDoc = new XmlDocument();
            if (!TryLoadDocument(expDoc, ActiveExperimentPath))
            {
                MessageBox.Show("Load  ERROR");
                return;
            }

            XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/CaptureSequence");
            if (0 < ndList.Count)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], expDoc, "enable", ref str))
                {
                    if ("1" == str)
                    {
                        ndList = expDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");

                        _sequenceStepsNum = ndList.Count;

                        //if _sequenceStepsNum > 1, set the initial _lsmChannel
                        if (0 < _sequenceStepsNum)
                        {
                            SetSequenceStepDisplaySettings(expDoc);
                            ChannelSelection = _channelOrderCurrentLSMChannel;
                            SetSequenceStepMCLS(expDoc);
                            SetSequenceDigitalSwitches(ExperimentDoc);
                        }
                    }
                    else
                    {
                        _sequenceStepWavelengthNameList = new List<string>();
                    }
                }
                else
                {
                    _sequenceStepWavelengthNameList = new List<string>();
                }
            }
            else
            {
                _sequenceStepWavelengthNameList = new List<string>();
            }
        }

        private void SetSequenceDigitalSwitches(XmlDocument expDoc)
        {
            if (null == expDoc) return;
            XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
            if (ndList.Count <= _channelOrderCurrentIndex) return;

            XmlNodeList digitalNdList = ndList[_channelOrderCurrentIndex].SelectNodes("DigitalIO");
            if (digitalNdList.Count <= 0) return;
            string str = string.Empty;
            int[] switchPositions = new int[8];
            int enable = 0;
            if (XmlManager.GetAttribute(digitalNdList[0], expDoc, "enable", ref str))
            {
                Int32.TryParse(str, out enable);
            }
            if (XmlManager.GetAttribute(digitalNdList[0], expDoc, "digOut1", ref str))
            {
                Int32.TryParse(str, out switchPositions[0]);
            }
            if (XmlManager.GetAttribute(digitalNdList[0], expDoc, "digOut2", ref str))
            {
                Int32.TryParse(str, out switchPositions[1]);
            }
            if (XmlManager.GetAttribute(digitalNdList[0], expDoc, "digOut3", ref str))
            {
                Int32.TryParse(str, out switchPositions[2]);
            }
            if (XmlManager.GetAttribute(digitalNdList[0], expDoc, "digOut4", ref str))
            {
                Int32.TryParse(str, out switchPositions[3]);
            }
            if (XmlManager.GetAttribute(digitalNdList[0], expDoc, "digOut5", ref str))
            {
                Int32.TryParse(str, out switchPositions[4]);
            }
            if (XmlManager.GetAttribute(digitalNdList[0], expDoc, "digOut6", ref str))
            {
                Int32.TryParse(str, out switchPositions[5]);
            }
            if (XmlManager.GetAttribute(digitalNdList[0], expDoc, "digOut7", ref str))
            {
                Int32.TryParse(str, out switchPositions[6]);
            }
            if (XmlManager.GetAttribute(digitalNdList[0], expDoc, "digOut8", ref str))
            {
                Int32.TryParse(str, out switchPositions[7]);
            }
            MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchEnable"] = enable;
            ObservableCollection<IntPC> switchState = new ObservableCollection<IntPC>();

            for (int i = 0; i < (int)Constants.MAX_SWITCHES; i++)
            {
                switchState.Add(new IntPC());
                switchState[i].Value = switchPositions[i];
            }

            MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchState"] = switchState;
            ((IMVM)MVMManager.Instance["DigitalOutputSwitchesViewModel", this]).OnPropertyChange("SwitchState");
            ICommand digitalSwitchCommand = (ICommand)MVMManager.Instance["DigitalOutputSwitchesViewModel", "DigitalSwitchCommand", (object)null];
            for (int i = 0; i < (int)Constants.MAX_SWITCHES; i++)
            {
                digitalSwitchCommand.Execute(i);
            }
        }

        private void SetSequenceStepDisplaySettings(XmlDocument expDoc)
        {
            if (null == expDoc) return;
            string str = string.Empty;
            int iVal = 0;
            if (0 < _sequenceStepsNum)
            {
                //Wavelength and channel settings
                XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
                if (ndList.Count <= _channelOrderCurrentIndex) return;
                _sequenceStepWavelengthNameList = new List<string>();

                if (ndList.Count <= _channelOrderCurrentIndex) return;

                XmlNodeList lsmNdList = ndList[_channelOrderCurrentIndex].SelectNodes("LSM");
                if (lsmNdList.Count > 0)
                {
                    if (XmlManager.GetAttribute(lsmNdList[0], expDoc, "channel", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        _channelOrderCurrentLSMChannel = iVal;
                    }

                    //If the imaging camera is a CCD or CMOS, we need to read the camera tag. If there is not camera tag it should set the channel number to 1
                    if (ResourceManagerCS.GetCameraType() == (int)ICamera.CameraType.CCD)
                    {
                        XmlNodeList camNdList = ndList[_channelOrderCurrentIndex].SelectNodes("Camera");
                        if (camNdList.Count > 0)
                        {
                            if (XmlManager.GetAttribute(camNdList[0], expDoc, "channel", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                            {
                                _channelOrderCurrentLSMChannel = iVal;
                            }
                        }
                        else
                        {
                            _channelOrderCurrentLSMChannel = 1;
                        }
                    }

                    XmlNodeList wavelengthsNdList = ndList[_channelOrderCurrentIndex].SelectNodes("Wavelengths");
                    XmlNodeList wavelengthNdList = wavelengthsNdList[0].SelectNodes("Wavelength");
                    for (int j = 0; j < wavelengthNdList.Count; j++)
                    {
                        if (XmlManager.GetAttribute(wavelengthNdList[j], expDoc, "name", ref str))
                            _sequenceStepWavelengthNameList.Add(str);
                    }
                }

                //Pinhole Settings
                XmlNodeList pinholeNdList = ndList[_channelOrderCurrentIndex].SelectNodes("PinholeWheel");

                if (pinholeNdList.Count > 0)
                {
                    if (XmlManager.GetAttribute(pinholeNdList[0], expDoc, "micrometers", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        this.PinholePosition = iVal;
                    }
                }

                //Let the viewModel know there have been changes
                if (null != UpdateSequenceStepDisplaySettings) UpdateSequenceStepDisplaySettings();
            }
        }

        private void SetSequenceStepMCLS(XmlDocument expDoc)
        {
            if (null == expDoc) return;
            XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
            if (ndList.Count <= _channelOrderCurrentIndex) return;

            XmlNodeList mclsNdList = ndList[_channelOrderCurrentIndex].SelectNodes("MCLS");
            if (mclsNdList.Count <= 0) return;
            string str = string.Empty;
            Visibility vis1 = Visibility.Collapsed;
            Visibility vis2 = Visibility.Collapsed;
            Visibility vis3 = Visibility.Collapsed;
            Visibility vis4 = Visibility.Collapsed;
            double pow1 = 0;
            double pow2 = 0;
            double pow3 = 0;
            double pow4 = 0;
            //Set all lasers to Visible if one is visible (Otherwise the Capture tab can move up and down depending on the number of lasers visible at the moment)
            GetMCLSChannelInfo(expDoc, mclsNdList, "enable1", "power1percent", ref vis1, ref pow1);
            GetMCLSChannelInfo(expDoc, mclsNdList, "enable2", "power2percent", ref vis2, ref pow2);
            GetMCLSChannelInfo(expDoc, mclsNdList, "enable3", "power3percent", ref vis3, ref pow3);
            GetMCLSChannelInfo(expDoc, mclsNdList, "enable4", "power4percent", ref vis4, ref pow4);
            if (vis1 == Visibility.Visible || vis2 == Visibility.Visible || vis3 == Visibility.Visible || vis4 == Visibility.Visible)
            {
                MCLS1Visibility = Visibility.Visible;
                MCLS2Visibility = Visibility.Visible;
                MCLS3Visibility = Visibility.Visible;
                MCLS4Visibility = Visibility.Visible;
            }

            MCLS1Power = pow1;
            MCLS2Power = pow2;
            MCLS3Power = pow3;
            MCLS4Power = pow4;

            //Let the viewModel know there have been changes
            if (null != UpdateSequenceStepDisplaySettings) UpdateSequenceStepDisplaySettings();
        }

        /// <summary>
        /// Execute delay time between SLM repeats
        /// </summary>
        /// <returns></returns>
        private bool SLMBleachDelayEx(double delayTime)
        {
            //check status every 5 seconds
            int timeWait = (int)(WaveformBuilder.MS_TO_S * delayTime);
            while (timeWait > 0)
            {
                BleachLastRunTime = DateTime.Now;
                System.Threading.Thread.Sleep(((WAITUNIT_MS < timeWait) ? WAITUNIT_MS : timeWait));
                //check if bleaching:
                if (!IsRunning())
                {
                    return false;
                }
                timeWait -= (int)(new TimeSpan(DateTime.Now.Ticks - BleachLastRunTime.Ticks).TotalMilliseconds);
            }
            return true;
        }

        private void SLMLoader_DoWork(object sender, DoWorkEventArgs e)
        {
            try
            {
                ActiveExperimentPath = ResourceManagerCS.GetCaptureTemplatePathString() + "Active.xml";
                if (true == _runComplete)
                {
                    //Load active.xml accounting for possible lockup during scripting
                    XmlDocument expDoc = new XmlDocument();
                    _isSequentialCapture = false;
                    if (!TryLoadDocument(expDoc, ActiveExperimentPath))
                    {
                        return; //false;
                    }

                    //Check for compatibility
                    if (!ExperimentSettingsValidForCapture(expDoc))
                    {
                        return; //false;
                    }

                    _ismROICapture = mROIXMLMapper.MapXml2mROIParams(expDoc, LSMType, out _mROIs, out _mROIPixelSizeXUM, out _mROIPixelSizeYUM, out _mROIStripLength, out _mROIFullFOVMetadata);
                    bool noExpName = false;
                    _currentWellCount = 0;
                    _currentImageCount = 0;
                    _runCompleteCount = 0;
                    _totalImagesCompleted = 0;
                    //Kirk: T and Z counters set to 0 then Updated to fix Redmine Bug #33
                    _currentTCount = 0;
                    _currentZCount = 0;
                    Update("Reset Counters");

                    //image counter used to determine the end of a row for kicking the background image generation process
                    _imageCounter = 0;

                    _sbNewDir = new StringBuilder(_outputPath + "\\" + _experimentName.FullName);

                    // This is done for customers who are looking to index their files from the first experiment itself
                    string strTemp = string.Empty;
                    System.Xml.XmlDocument appSettings = new System.Xml.XmlDocument();
                    string appSettingsFile = ResourceManagerCS.GetApplicationSettingsFileString();
                    ResourceManagerCS.BorrowDocMutexCS(SettingsFileType.APPLICATION_SETTINGS);
                    appSettings.Load(appSettingsFile);
                    ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.APPLICATION_SETTINGS);
                    System.Xml.XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/ImageNameFormat");

                    if (node != null && node.Attributes != null)
                    {
                        _indexDigitCounts = Convert.ToInt32(node.Attributes["indexDigitCounts"].Value);

                    }

                    bool folderExists = Directory.Exists(_sbNewDir.ToString()); //first check to see if the directory we are about to create already exists

                    bool alwaysAppendOn = (XmlManager.GetAttribute(node, appSettings, "alwaysAppendIndex", ref strTemp) && ("1" == strTemp || Boolean.TrueString == strTemp));

                    FileName strGen = new FileName(_experimentName.NameWithoutNumber, false); //string generator used to generate string the same was as ThorSync

                    if (folderExists || alwaysAppendOn)
                    {
                        FileName name = new FileName(_experimentName.NameWithoutNumber, false); //override the global FileName with this

                        if (_experimentName.NameWithoutNumber == _experimentName.FullName)//if nothings appended
                        {
                            //case for Append On and it is the first file being created

                            name.NameNumberInt = 1; //this part is confusing
                            name.MakeUnique(_outputPath);
                            name.NameNumberInt = 0;      //the reason for doing this is, calling make unique when NameNumber is 0 and append index is on
                            name.MakeUnique(_outputPath);  // calling only make unique with NameNumber as zero and append on will create a file with no append becuase that is a unique file to the FileName class
                        }
                        else if (name.NameNumberInt == 0)
                        {
                            //case for Append On and it is the second file (indexed at 0 so it would look like _0001)
                            name.NameNumberInt = 1;
                            name.MakeUnique(_outputPath);
                        }
                        else
                        {
                            //default case
                            name.MakeUnique(_outputPath);
                        }

                        //if block is to force ThorImage to name a new folder the same way as ThorSync
                        if (false == alwaysAppendOn) //Always append is off here overwrite previous cases
                        {
                            strGen.MakeUnique(_outputPath);
                            name.NameNumber = strGen.NameNumber;

                        }
                        //Confirm That User Wants New Name
                        if ((folderExists) && !DontAskUniqueExperimentName)
                        {
                            Application.Current.Dispatcher.Invoke((Action)delegate
                            {
                                string msg = string.Format("A data set already exists at this location. Would you like to use the name {0} instead?", name.FullName);
                                CustomMessageBox messageBox = new CustomMessageBox(msg, "Data already exists", "Don't ask again", "Yes", "No");
                                if (!messageBox.ShowDialog().GetValueOrDefault(false))
                                {
                                    noExpName = true;
                                    e.Cancel = true;
                                    return; //false;
                                }
                                else
                                {
                                    DontAskUniqueExperimentName = messageBox.CheckBoxChecked;
                                }
                            });
                            if (noExpName)
                            {
                                return;
                            }
                        }

                        _experimentName = name;

                        _sbNewDir = new StringBuilder(_outputPath + "\\" + _experimentName.FullName);
                        OnPropertyChanged("ExperimentName");

                    }

                    string totalPath = _sbNewDir.ToString();

                    int totalPathLength;
                    if (_indexDigitCounts <= 0)
                    {
                        totalPathLength = TOTAL_PATH_LENGTH;
                    }
                    else
                    {
                        totalPathLength = TOTAL_PATH_LENGTH - (_indexDigitCounts - 1) * 4;
                    }

                    if (totalPath.Length > totalPathLength)
                    {
                        string msg = string.Format(" Your path name and file name combined should be less than {0} characters", totalPathLength);
                        MessageBox.Show(msg);
                        return; //false;
                    }

                    bool ipcDownlinkFlag = (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "IPCDownlinkFlag", (object)false];
                    bool remoteConnection = (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "RemoteConnection", (object)false];
                    bool remoteSavingStats = (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "RemoteSavingStats", (object)false];
                    if (ipcDownlinkFlag == false && remoteConnection == true && remoteSavingStats == true)
                    {
                        MVMManager.Instance["RemoteIPCControlViewModelBase", "StartAcquisition"] = OutputPath + "\\" + ExperimentName;
                    }

                    //Create the new ROIDataStore to hold the ROI stats
                    const int DSTYPE_SQLITE = 1;
                    CreateStatsManagerROIDS(DSTYPE_SQLITE, _streamingPath + "\\ROIDataStore");

                    //Create the new experiment directory(s)
                    Directory.CreateDirectory(_sbNewDir.ToString() + @"\jpeg");
                    Directory.CreateDirectory(_sbNewDir.ToString() + @"\jpeg\1x");

                    //assign the experiment xml path
                    _experimentXMLPath = _sbNewDir.ToString() + "\\Experiment.xml";

                    _experimentFolderPath = _sbNewDir.ToString() + "\\";

                    string batchName = DataStore.Instance.GetBatchName();

                    DataStore.Instance.AddExperiment(_experimentName.FullName, _experimentFolderPath, batchName);

                    StringBuilder sbExp = new StringBuilder(_experimentXMLPath);

                    //copy the experiment path to the runsample parameters
                    _rsParams.path = String.Copy(_experimentXMLPath);

                    try
                    {
                        XmlNodeList nlA = expDoc.SelectNodes("/ThorImageExperiment/Name");
                        if (0 < nlA.Count)
                        {
                            XmlManager.SetAttribute(nlA[0], expDoc, "name", _experimentName.FullName);
                            XmlManager.SetAttribute(nlA[0], expDoc, "path", _sbNewDir.ToString());
                        }
                        ResourceManagerCS.BorrowDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                        expDoc.Save(ActiveExperimentPath);
                        ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);

                        //Save power ramp & pockel info to experiment folder
                        Directory.CreateDirectory(_experimentFolderPath + "\\powerramp\\");
                        XmlNodeList pockels = expDoc.SelectNodes("/ThorImageExperiment/Pockels");
                        foreach (XmlNode x in pockels)
                        {
                            string pockel_path = string.Empty;
                            XmlManager.GetAttribute(x, expDoc, "path", ref pockel_path);
                            if (!pockel_path.Equals(string.Empty))
                            {
                                if ((!File.Exists(_experimentFolderPath + "\\powerramp\\" + Path.GetFileName(pockel_path))) &&
                                    File.Exists(pockel_path))
                                {
                                    File.Copy(pockel_path, _experimentFolderPath + "\\powerramp\\" + Path.GetFileName(pockel_path));
                                }
                            }
                        }

                        XmlNodeList powerramps = expDoc.SelectNodes("/ThorImageExperiment/PowerRegulator");
                        string fval = string.Empty;
                        XmlManager.GetAttribute(powerramps[0], expDoc, "path", ref fval);

                        if ((!File.Exists(_experimentFolderPath + "\\powerramp\\" + Path.GetFileName(fval))) && File.Exists(fval))

                            File.Copy(fval, _experimentFolderPath + "\\powerramp\\" + Path.GetFileName(fval));

                        XmlNodeList powerramps2 = expDoc.SelectNodes("/ThorImageExperiment/PowerRegulator2");
                        string fval2 = string.Empty;
                        XmlManager.GetAttribute(powerramps2[0], expDoc, "path", ref fval2);

                        if ((!File.Exists(_experimentFolderPath + "\\powerramp\\" + Path.GetFileName(fval2))) && File.Exists(fval2))

                            File.Copy(fval2, _experimentFolderPath + "\\powerramp\\" + Path.GetFileName(fval2));

                    }
                    catch (Exception ex)
                    {
                        string str = ex.Message;
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + "RunSampleLSModule: Unable to save new experiment path.");
                    }
                    //overwrite the active experiment settings
                    File.Copy(ActiveExperimentPath, _experimentXMLPath);

                    //update the channel view for sequential capture
                    MVMManager.Instance["ImageViewCaptureVM", "IsInSequentialMode"] = _isSequentialCapture;
                    if (_isSequentialCapture)
                    {
                        if (_experimentXMLPath.Substring(_experimentXMLPath.Length - 4) == ".xml")
                        {
                            //Create a copy of the Experiment.xml file to modify the colors in the file. This way Experiment.xml stays unmodified.
                            string colorExperiment = _experimentXMLPath.Substring(0, _experimentXMLPath.Length - 4) + "SequentialColorSettings.xml";
                            if (false == File.Exists(colorExperiment))
                            {
                                File.Copy(_experimentXMLPath, colorExperiment);
                            }
                            MVMManager.Instance["ImageViewCaptureVM", "SequentialExperimentPath"] = colorExperiment;
                        }
                    }

                    try
                    {
                        //Update the variables list LAST OUTPUT value
                        const string PATH_LAST_OUTPUT = "LAST OUTPUT";
                        string strVar = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\VariableList.xml";
                        XmlDocument varDoc = new XmlDocument();
                        varDoc.Load(strVar);
                        XmlNodeList ndList = varDoc.SelectNodes("/VariableList/Path");

                        foreach (XmlNode nd in ndList)
                        {
                            string str = nd.Attributes["name"].Value;
                            if (str.Equals(PATH_LAST_OUTPUT))
                            {
                                nd.Attributes["value"].Value = _sbNewDir.ToString();
                                break;
                            }
                        }
                        varDoc.Save(strVar);
                    }
                    catch (Exception ex)
                    {
                        string str = ex.Message;
                        //error loading/saving the variables list
                        ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Unable to load/save variables list");
                    }

                    _tiffCompressionEnabled = GetTiffCompressionEnbaledSetting();

                    //Copy bleach ROI and waveform:
                    if ((int)CaptureModes.BLEACHING == CaptureMode)
                    {
                        //do preBleach arm and check:
                        PreBleachCheck();
                    }

                    _runComplete = false;
                    _experimentCompleteForProgressBar = false;

                    // event trigerred to the view model to change the status of the Start and Stop button
                    _startButtonStatus = false;
                    _stopButtonStatus = true;
                    UpdateButton(false);

                    _statusMessageZUpdate = true;

                    // event trigerred to the view model to change the status of the menu bar buttons
                    UpdateMenuBarButton(false);

                    // event trigerred to let the viewModel know that the experiment has started
                    ExperimentStarted();

                    //enable digital trigger at start, digital output view model will fetch for average
                    //frame number from active.xml, make sure it happens when active and exp are the same copy.
                    MVMManager.Instance["DigitalOutputSwitchesViewModel", "TriggerEnable"] = 1;

                    _experimentStatus = "started";
                    _startDateTime = DateTime.Now;
                    _startUTC = DateTime.UtcNow;
                    if (true == UpdateExperimentFile(_experimentXMLPath, false))
                    {
                        if (SetActiveExperiment(_experimentXMLPath) == 1)
                        {
                            //Check the sequential capture settings and set the channel appropiately
                            SetCaptureSequence();
                            //save experiment path to applicationSettings.xml
                            SaveLastExperimentInfo(_experimentFolderPath);

                            if (SetCustomParamsBinary(ref _rsParams) == 1)
                            {
                                if (TurnOffMonitorsDuringCapture)
                                {
                                    //turn off the monitors
                                    SendMessage(0xffff, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
                                }

                                // Set the Simulator Image Index to the first frame in the experiment folder
                                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_SIM_INDEX, (int)1);
                                if (RunSampleLSExecute() == 1)
                                {
                                    StatusMessage = "Starting Experiment";
                                    UpdateBitmapTimer(true);
                                }
                            }
                        }
                    }
                    else
                    {
                        _startButtonStatus = true;
                        _stopButtonStatus = false;
                        UpdateButton(true);
                        UpdateMenuBarButton(true);
                    }
                }
            }
            catch (System.DllNotFoundException ex)
            {
                MessageBox.Show(ex.Message.ToString());
                //RunSampleLSdll is missing
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message.ToString());
            }
        }

        private void SLMLoader_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Cancelled)
            {
                CancelAcquisition();
            }
            else if (e.Error != null)
            {
                MessageBox.Show(e.Error.Message.ToString());
                Stop();
            }
        }

        private void UpdateActiveImage()
        {
            // updating the images in the image viewer synchronously
            if ((true == _newImageAvailable) && (_displayImage))
            {
                BuildColorImage();
                _newImageAvailable = false;
            }
        }

        /// <summary>
        /// Update experiment file to specified path.
        /// </summary>
        /// <param name="expXML"></param>
        /// <returns></returns>
        private bool UpdateExperimentFile(string expXML, bool updateCaptureSequence)
        {
            //keep decimal dot in xml:
            bool tempSwitchCI = false;
            System.Globalization.CultureInfo originalCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();

            if (0 == originalCultureInfo.NumberFormat.NumberDecimalSeparator.CompareTo(","))
            {
                tempSwitchCI = true;
                originalCultureInfo.NumberFormat.NumberDecimalSeparator = ".";
                System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
            }

            if (File.Exists(expXML) == false)
            {
                return false;
            }

            //Load active.xml accounting for possible lockup during scripting
            XmlDocument expDoc = new XmlDocument();
            if (!TryLoadDocument(expDoc, ActiveExperimentPath))
            {
                MessageBox.Show("SCRIPT ERROR");
                return false;
            }

            //update settings by MVM
            // This is where devices like the manual EpiTurret save their current position
            MVMManager.Instance.UpdateMVMXMLSettings(ref expDoc, MVMNames);

            //used to load color images.
            ExperimentDoc = expDoc;

            XmlNodeList ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/LSM");

            if (ndList.Count > 0)
            {
                _lsmChannel = Convert.ToInt32(ndList[0].Attributes["channel"].Value);
            }

            if ((int)CaptureModes.STREAMING == CaptureMode)
            {
                _streamEnable = 1;
            }
            //else if ((int)CaptureModes.TDI == CaptureMode)
            //{
            //    _streamEnable = 2;
            //}
            else
            {
                _streamEnable = 0;
            }

            XmlNodeList nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Streaming");

            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "enable", _streamEnable.ToString());
                if (this.StreamEnable > 0)
                {
                    ZStreamEnable = false;
                    //update the stream frames if the fast z is enabled in Finite Stream mode(0):
                    if ((this.StreamStorageMode == 0) && (true == this.ZFastEnable))
                    {
                        FlybackFrames = IsRemoteFocus ? 0 : FlybackFrames;
                        this.StreamFrames = (this.ZNumSteps + this.FlybackFrames) * this.StreamVolumes;
                    }
                    //update the saved stream frames in Stimulus Stream mode(1):
                    if ((this.StreamStorageMode == 1) && (this.CurrentTCount != 0))
                    {
                        this.StreamFrames = this.CurrentTCount;
                    }
                }

                if ((int)CaptureModes.HYPERSPECTRAL == CaptureMode)
                {
                    this.StreamFrames = this.SFSteps;
                }

                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "frames", this.StreamFrames.ToString());

                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "rawData", Convert.ToInt32(_rawDataCapture).ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "displayImage", Convert.ToInt32(_displayImage).ToString());

                switch (_triggerModeStreaming)
                {
                    case 0:
                        {
                            const int SOFTWARE_MULTIFRAME = 1;
                            XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "triggerMode", Convert.ToInt32(SOFTWARE_MULTIFRAME).ToString());
                        }
                        break;
                    case 1:
                        {
                            const int HARDWARE_MULTIFRAME_TRIGGER_FIRST = 4;
                            XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "triggerMode", Convert.ToInt32(HARDWARE_MULTIFRAME_TRIGGER_FIRST).ToString());
                        }
                        break;
                    case 2:
                        {
                            const int HW_MULTI_FRAME_TRIGGER_EACH = 5;
                            XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "triggerMode", Convert.ToInt32(HW_MULTI_FRAME_TRIGGER_EACH).ToString());
                        }
                        break;
                    case 3:
                        {
                            const int HW_MULTI_FRAME_TRIGGER_EACH_BULB = 6;
                            XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "triggerMode", Convert.ToInt32(HW_MULTI_FRAME_TRIGGER_EACH_BULB).ToString());
                        }
                        break;
                }
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "storageMode", StreamStorageMode.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "zFastEnable", (Convert.ToInt32(ZFastEnable)).ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "zFastMode", (Convert.ToInt32(FastZStaircase || IsRemoteFocus) + (int)ZPiezoAnalogMode.ANALOG_MODE_SINGLE_WAVEFORM).ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "flybackFrames", FlybackFrames.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "flybackLines", FlybackLines.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "flybackTimeAdjustMS", FlybackTimeAdjustMS.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "volumeTimeAdjustMS", VolumeTimeAdjustMS.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "stepTimeAdjustMS", StepTimeAdjustMS.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "previewIndex", PreviewIndex.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "stimulusTriggering", StimulusTriggering.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "dmaFrames", DMAFrames.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "stimulusMaxFrames", StimulusMaxFrames.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "displayRollingAveragePreview", StreamingDisplayRollingAveragePreview.ToString());
                if (PockelsOutputReferenceAvailable)
                {
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "useReferenceVoltageForFastZPockels", UseReferenceVoltageForFastZPockels.ToString());
                }
                else
                {
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "useReferenceVoltageForFastZPockels", "0");
                }

                if ((int)CaptureModes.STREAMING == CaptureMode)
                {
                    if (true == ZFastEnable)
                    {
                        //use the existing z start/stop/step
                        //t frames is not known at setup.
                        //When the stop button is pressed the t frames will be updated
                        if (0 != _zNumSteps + FlybackFrames)
                        {
                            _tFrames = _streamFrames / (_zNumSteps + FlybackFrames);
                        }
                    }
                    else
                    {
                        //disable the Z attributes and set the t frames equal to the stream frames
                        //this will ensure the viewer loads the images correctly
                        if ((1 == _streamEnable) || (2 == _streamEnable))
                        {
                            _zNumSteps = 1;
                            _tFrames = _streamFrames;
                        }
                    }
                }
            }

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/LSM");
            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "frameRate", FrameRate.ToString("N3"));
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "tbLineScanTimeMS", TimeBasedLSTimeMS.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "averageNum", AverageNum.ToString());
            }

            _photobleachEnable = ((int)CaptureModes.BLEACHING == CaptureMode) ? 1 : 0;

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Photobleaching");

            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "enable", _photobleachEnable.ToString());

                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "laserPos", _photobleachLaserPosition.ToString());

                int durationMS = Convert.ToInt32(_photobleachDurationSec * 1000);
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "durationMS", durationMS.ToString());

                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "powerPos", _photobleachPowerPosition.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "bleachTrigger", BleachTrigger.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "bleachPostTrigger", BleachPostTrigger.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "preBleachFrames", PreBleachFrames.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "preBleachInterval", PreBleachInterval.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "preBleachStream", PreBleachStream.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "bleachFrames", BleachFrames.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "postBleachFrames1", PostBleachFrames1.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "postBleachInterval1", PostBleachInterval1.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "postBleachStream1", PostBleachStream1.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "postBleachFrames2", PostBleachFrames2.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "postBleachInterval2", PostBleachInterval2.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "postBleachStream2", PostBleachStream2.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "rawOption", RawOption.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "EnableSimultaneous", SimultaneousBleachingAndImaging.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "pmt1EnableDuringBleach", PMT1EnableDuringBleach.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "pmt2EnableDuringBleach", PMT2EnableDuringBleach.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "pmt3EnableDuringBleach", PMT3EnableDuringBleach.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "pmt4EnableDuringBleach", PMT4EnableDuringBleach.ToString());

                if (1 == _photobleachEnable)
                {
                    //when photobleaching is enabled. Disable the Z frames and match the Tframes
                    //to the total number of frames captured in the experiment.
                    _zEnable = 1;
                    _tEnable = true;

                    _tFrames = PreBleachFrames + PostBleachFrames1 + PostBleachFrames2;
                }
            }

            nodeList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/Camera");
            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "bitsPerPixel", _camBitsPerPixel.ToString());
            }

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/ZStage");

            if (nodeList.Count > 0)
            {
                double stepSize = _zStepSize;

                if (_zStartPosition > _zStopPosition)
                {
                    stepSize *= -1;
                }

                //check if a Z stack is being captured
                if (((int)CaptureModes.T_AND_Z == CaptureMode && 1 == _zEnable) || ((int)CaptureModes.STREAMING == CaptureMode && ZFastEnable))
                {
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "steps", _zNumSteps.ToString());
                }

                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "enable", ZEnable.ToString());

                if (_zStreamFrames < 1)
                {
                    _zStreamFrames = 1;
                }

                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "startPos", _zStartPosition.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "stepSizeUM", stepSize.ToString());
                int zStreamMode = (true == ZStreamEnable) ? 1 : 0;
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "zStreamMode", zStreamMode.ToString());
                _zStreamFrames = (true == ZStreamEnable) ? _zStreamFrames : 1;
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "zStreamFrames", _zStreamFrames.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "zFileEnable", _zFileEnable.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "zFilePosScale", _zFilePosScale.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "z2StageLock", _z2StageLock == true ? "1" : "0");
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "z2StageMirror", _z2StageMirror == true ? "1" : "0");
            }

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Timelapse");

            if (nodeList.Count > 0)
            {
                if ((false == _tEnable) && (0 == _streamEnable))
                {
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "timepoints", Convert.ToString(1));
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "triggerMode", Convert.ToString(0));
                }
                else
                {
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "timepoints", _tFrames.ToString());
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "triggerMode", _triggerModeTimelapse.ToString());
                }
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "intervalSec", _tInterval.ToString());
            }

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Modality");
            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "primaryDetectorType", this.ActiveCameraType.ToString());
            }

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Name");
            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "name", _experimentName.FullName);
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "path", _outputPath + "\\" + _experimentName.FullName);
            }

            //Set the date for the file
            string formatDate = "MM/dd/yyyy HH:mm:ss";
            string date = _startDateTime.ToString(formatDate);

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Date");

            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "date", date);
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "uTime", ((int)Math.Truncate((_startUTC.Subtract(new DateTime(1970, 1, 1))).TotalSeconds)).ToString());
            }

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/User");

            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "name", Environment.UserName.ToString());
            }
            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Computer");

            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "name", Environment.MachineName.ToString());
            }
            bool remoteConnection = (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "RemoteConnection", (object)false];

            bool isSaving = (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "IsSaving", (object)false];
            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/ThorSyncDataPath");
            if (remoteConnection)
            {
                string filePath = (string)MVMManager.Instance["RemoteIPCControlViewModelBase", "ThorSyncFilePath", (object)false];

                if (filePath == "" || !isSaving)
                {
                    if (nodeList.Count != 0)
                    {
                        XmlNode removeNode = ExperimentDoc.SelectSingleNode("/ThorImageExperiment");
                        removeNode.RemoveChild(nodeList[0]);
                    }
                }
                else
                {

                    if (nodeList.Count == 0)
                        XmlManager.CreateXmlNode(ExperimentDoc, "ThorSyncDataPath");
                    XmlManager.SetAttribute(ExperimentDoc.SelectNodes("/ThorImageExperiment/ThorSyncDataPath")[0], ExperimentDoc, "path", filePath);
                }
            }
            else
            {
                if (nodeList.Count != 0)
                {
                    XmlNode removeNode = ExperimentDoc.SelectSingleNode("/ThorImageExperiment");
                    removeNode.RemoveChild(nodeList[0]);
                }
            }
            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Software");

            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "version", System.Diagnostics.Process.GetCurrentProcess().MainModule.FileVersionInfo.FileVersion.ToString());
            }

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/CaptureMode");

            if (nodeList.Count <= 0)
            {
                //version 1.5 the capture mode is added
                XmlManager.CreateXmlNode(ExperimentDoc, "CaptureMode");
                nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/CaptureMode");
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "mode", this.CaptureMode.ToString());
            }
            else
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "mode", this.CaptureMode.ToString());
            }

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/ExperimentStatus");

            if (nodeList.Count <= 0)
            {
                //version 2.4 the ExperimentStatus is added
                XmlManager.CreateXmlNode(ExperimentDoc, "ExperimentStatus");
                nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/ExperimentStatus");
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "value", _experimentStatus);
            }
            else
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "value", _experimentStatus);
            }

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/ExperimentNotes");
            if (nodeList.Count <= 0)
            {
                //version 2.4 the ExperimentStatus is added
                XmlManager.CreateXmlNode(ExperimentDoc, "ExperimentNotes");
                nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/ExperimentNotes");
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "text", ExperimentNotes);
            }
            else
            {
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "text", ExperimentNotes);
            }

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/RawData");
            string RawContainsEnabledOnly = "1";
            if (!GetShouldSaveOnlyEnabledChannelsInRawFile())
            {
                RawContainsEnabledOnly = "0";
            }
            if (nodeList.Count <= 0)
            {
                //version 3.0 the RawData tag is added
                XmlManager.CreateXmlNode(ExperimentDoc, "RawData");
                nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/RawData");
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "onlyEnabledChannels", RawContainsEnabledOnly);
            }
            else
            {
                //SetAttribute(nodeList[0], ExperimentDoc, "onlyEnabledChannels", "1");
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "onlyEnabledChannels", RawContainsEnabledOnly);
            }

            nodeList = ExperimentDoc.SelectNodes("/ThorImageExperiment/RemoteFocus");

            if (nodeList.Count <= 0)
            {
                XmlManager.CreateXmlNode(ExperimentDoc, "RemoteFocus");
                ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/RemoteFocus");
            }
            if (nodeList.Count > 0)
            {
                //For streaming capture use the values from the Remotefocus panel. For Z&T use the values from the usual Z Stack panel
                if ((int)CaptureModes.STREAMING == CaptureMode)
                {
                    double rfStepSize = (RemoteFocusStartPosition > RemoteFocusStopPosition) ? RemoteFocusStepSize * -1 : RemoteFocusStepSize;
                    int steps = ZFastEnable ? _zNumSteps : 1;
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "steps", steps.ToString());
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "startPlane", RemoteFocusStartPosition.ToString());
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "stepSize", rfStepSize.ToString());

                }
                else if ((int)CaptureModes.T_AND_Z == CaptureMode)
                {
                    double stepSize = (_zStartPosition > _zStopPosition) ? _zStepSize * -1 : _zStepSize;
                    int steps = (1 == _zEnable) ? _zNumSteps : 1;
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "steps", steps.ToString());
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "startPlane", (_zStartPosition * (double)Constants.UM_TO_MM).ToString());
                    XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "stepSize", stepSize.ToString());
                }

                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "IsRemoteFocus", Convert.ToInt32(IsRemoteFocus).ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "captureMode", RemoteFocusCaptureMode.ToString());
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "customSequenceEnabled", Convert.ToInt32(RemoteFocusCustomChecked).ToString());

                if (IsRemoteFocus)
                {
                    if ((int)RemoteFocusCaptureModes.Custom == RemoteFocusCaptureMode && !RemoteFocusCustomChecked)
                    {
                        ObservableCollection<double> sortedPlanes = new ObservableCollection<double>(RemoteFocusCustomSelectedPlanes.OrderBy(n => n));
                        if (sortedPlanes.Count > 0)
                        {
                            string planeSequence = sortedPlanes[0].ToString();
                            for (int i = 1; i < RemoteFocusCustomSelectedPlanes.Count; i++)
                            {
                                planeSequence += ":";
                                planeSequence += sortedPlanes[i].ToString();
                            }
                            XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "customSequence", planeSequence);
                        }
                    }
                    else if (RemoteFocusCustomChecked && (int)RemoteFocusCaptureModes.Custom == RemoteFocusCaptureMode && ValidateRemoteFocusCustom(RemoteFocusCustomOrder))
                    {
                        XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "customSequence", RemoteFocusCustomOrder.ToString());
                    }
                    else
                    {
                        XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "customSequence", string.Empty);
                    }

                    CreateRemoteFocusPositionValuesTextFile();
                }
            }

            //If the Capture Sequence Steps > 1 and updateCaptureSequence is set to true
            //update the settings file to have all the wavelengths, channels set
            //by doing this the experiment when done can be reviewed like any other experiment
            if (true == updateCaptureSequence)
            {
                if (0 < _sequenceStepsNum)
                {
                    XmlNodeList sequenceStepNdList = ExperimentDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
                    long totalChan = 0, camTotalChan = 0, val = 0;
                    List<string> wavelengthNameList = new List<string>();
                    List<int> wavelengthExpTimeList = new List<int>();
                    string nyquistExWavelengthNM = string.Empty;
                    string nyquistEmWavelengthNM = string.Empty;
                    string str = string.Empty;
                    for (int i = 0; i < sequenceStepNdList.Count; i++)
                    {
                        //If camera is LSM use the LSM tag to read the channel number
                        if (ResourceManagerCS.GetCameraType() == (int)ICamera.CameraType.LSM)
                        {
                            XmlNodeList lsmNdList = sequenceStepNdList[i].SelectNodes("LSM");
                            if (lsmNdList.Count <= 0) return false;
                            if (XmlManager.GetAttribute(lsmNdList[0], ExperimentDoc, "channel", ref str))
                            {
                                if (long.TryParse(str, out val)) totalChan |= val;
                            }
                        }
                        else
                        {
                            //If the imaging camera is a CCD or CMOS, we need to read the camera tag. If there is not camera tag it should set the channel number to 1
                            XmlNodeList camNdList = sequenceStepNdList[i].SelectNodes("Camera");
                            if (camNdList.Count > 0)
                            {
                                if (XmlManager.GetAttribute(camNdList[0], expDoc, "channel", ref str) && long.TryParse(str, out val))
                                {
                                    camTotalChan |= val;
                                }
                            }
                            else
                            {
                                camTotalChan |= 1;
                            }
                        }

                        XmlNodeList wavelengthsNdList = sequenceStepNdList[i].SelectNodes("Wavelengths");
                        XmlNodeList wavelengthNdList = wavelengthsNdList[0].SelectNodes("Wavelength");
                        for (int j = 0; j < wavelengthNdList.Count; j++)
                        {
                            XmlManager.GetAttribute(wavelengthNdList[j], ExperimentDoc, "name", ref str);
                            wavelengthNameList.Add(str);
                            XmlManager.GetAttribute(wavelengthNdList[j], ExperimentDoc, "exposureTimeMS", ref str);
                            int iVal = 0;
                            int.TryParse(str, out iVal);
                            wavelengthExpTimeList.Add(iVal);
                        }
                        XmlManager.GetAttribute(wavelengthNdList[wavelengthNdList.Count - 1], ExperimentDoc, "nyquistExWavelengthNM", ref str);
                        nyquistExWavelengthNM = str;
                        XmlManager.GetAttribute(wavelengthNdList[wavelengthNdList.Count - 1], ExperimentDoc, "nyquistEmWavelengthNM", ref str);
                        nyquistEmWavelengthNM = str;
                    }

                    ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/LSM");
                    if (0 < ndList.Count)
                    {
                        XmlManager.SetAttribute(ndList[0], ExperimentDoc, "channel", totalChan.ToString());
                    }

                    ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Camera");
                    if (0 < ndList.Count)
                    {
                        XmlManager.SetAttribute(ndList[0], ExperimentDoc, "channel", camTotalChan.ToString());
                    }

                    ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/Wavelengths");

                    if (0 < ndList.Count)
                    {
                        ndList[0].RemoveAll();
                    }

                    ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/Wavelengths");
                    if (0 < ndList.Count)
                    {
                        XmlManager.SetAttribute(ndList[0], ExperimentDoc, "nyquistExWavelengthNM", nyquistExWavelengthNM);
                        XmlManager.SetAttribute(ndList[0], ExperimentDoc, "nyquistEmWavelengthNM", nyquistEmWavelengthNM);
                        SortedDictionary<string, int> wavelengthsExposureTime = new SortedDictionary<string, int>();
                        for (int i = 0; i < wavelengthNameList.Count; i++)
                        {
                            XmlElement newElement = CreateWavelengthTag(wavelengthNameList[i], wavelengthExpTimeList[i]);
                            ndList[0].AppendChild(newElement);
                        }

                        XmlElement newElement2 = this.ExperimentDoc.CreateElement("ChannelEnable");
                        XmlAttribute ChanAttribute = this.ExperimentDoc.CreateAttribute("Set");
                        ChanAttribute.Value = (ResourceManagerCS.GetCameraType() == (int)ICamera.CameraType.LSM) ? totalChan.ToString() : camTotalChan.ToString();
                        newElement2.Attributes.Append(ChanAttribute);
                        ndList[0].AppendChild(newElement2);
                    }
                }
            }

            //save the updated experiment file
            ExperimentDoc.Save(expXML);

            //Copying the last experiment settings back to the Active.xml file
            //copy before the Capture Sequence settings
            if (!expXML.Equals(ActiveExperimentPath))
            {
                File.Copy(expXML, ActiveExperimentPath, true);
            }

            // save the z-position list if read from file enabled
            // I only want to do this for the Experiment.xml file, not for the Active.xml
            if (_zFileEnable == 1 && expXML.Contains("Experiment.xml"))
            {
                // save to xml file
                //XmlDocument doc = new XmlDocument();
                //doc.Load(expXML);
                //XmlNode node = doc.SelectSingleNode("/ThorImageExperiment/PosList");
                //if(node == null)
                //{
                //    node = doc.SelectSingleNode("/ThorImageExperiment");
                //    if(node != null)
                //    {
                //        XmlNode newNode = doc.CreateNode(XmlNodeType.Element, "PosList", null);
                //        node.AppendChild(newNode);
                //        node = doc.SelectSingleNode("/ThorImageExperiment/PosList");
                //    }
                //}
                ////Delete any previously stored positions in Experiment.xml
                //node.RemoveAll();
                ////Add all new positions to Experiment.xml
                //for (int i = 0; i < _zFilePosList.Count; i++)
                //{
                //    //XmlManager.SetAttribute(node, doc, "item", _zFilePosList.ElementAt(i).ToString());
                //    XmlAttribute attr = doc.CreateAttribute("item");
                //    attr.Value = _zFilePosList.ElementAt(i).ToString();
                //    node.Attributes.Append(attr);
                //}
                //doc.Save(expXML);
                // save to xml file
                XDocument doc = XDocument.Load(expXML);
                XElement q = doc.Root;
                XElement pl = doc.Root.Element("PosList");
                if (pl != null) pl.Remove();
                q.Add(new XElement("PosList", _zFilePosList.Select(x => new XElement("item", x))));
                doc.Save(expXML);
            }

            //give back CultureInfo:
            if (tempSwitchCI)
            {
                originalCultureInfo.NumberFormat.NumberDecimalSeparator = ",";
                System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
            }

            return true;
        }

        // update specified bitmap with the data given
        // normal images without tiles are regarded as images with tile of 1, whose width and height equals image width and height
        private void updateTiledBitmap(WriteableBitmap bm, byte[] data, int width, int height, int tileWidth, int tileHeight)
        {
            //TODO:IV
            //// Define parameters used to create the BitmapSource.
            //int rawStride = (tileWidth * _bitmap.Format.BitsPerPixel + 7) / 8;
            //int tileSize = tileWidth * tileHeight * 3;
            //int tileIndex = 0;

            //try
            //{
            //    if (data.Length == width * height * 3)
            //    {
            //        for (int y = 0; y < height; y += tileHeight)
            //        {
            //            for (int x = 0; x < width; x += tileWidth)
            //            {
            //                int offset = tileIndex * tileSize;
            //                //ArraySegment<byte> tileData = new ArraySegment<byte>(data, offset, tileSize);
            //                bm.WritePixels(new Int32Rect(x, y, tileWidth, tileHeight), data, rawStride, offset);
            //                tileIndex++;
            //            }
            //        }
            //    }
            //}
            //catch (Exception ex)
            //{
            //    string errMsg = ex.Message;
            //}
        }

        #endregion Methods

        #region Nested Types

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        struct RunSampleLSCustomParams
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
            public string path;
        }

        #endregion Nested Types
    }
}