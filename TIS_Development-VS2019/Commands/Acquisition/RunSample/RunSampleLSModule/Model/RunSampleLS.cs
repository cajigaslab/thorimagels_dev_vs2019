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
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Linq;

    using CustomMessageBox;

    using DatabaseInterface;

    using GeometryUtilities;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class RunSampleLS : INotifyPropertyChanged
    {
        #region Fields

        public const int MAX_CHANNELS = 4;

        private const int DEFAULT_BITS_PER_PIXEL = 14;
        private const double LUT_MAX = 255;
        private const double LUT_MIN = 0;
        private const int LUT_SIZE = 256;
        private const int PATH_LENGTH = 261;
        private const int PIXEL_DATA_HISTOGRAM_SIZE = 256;
        private const int TOTAL_PATH_LENGTH = 111;
        private const int WAITUNIT_MS = 5000;

        private static readonly float[] _dflimColorMap = new float[] { 0.0f, 0.0f, 0.9125f, 0.0f, 0.0f, 0.925f, 0.0f, 0.0f, 0.9375f, 0.0f, 0.0f, 0.95f, 0.0f, 0.0f, 0.9625f, 0.0f, 0.0f, 0.975f, 0.0f, 0.0f, 0.9875f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0125f, 1.0f, 0.0f, 0.025f, 1.0f, 0.0f, 0.0375f, 1.0f, 0.0f, 0.05f, 1.0f, 0.0f, 0.0625f, 1.0f, 0.0f, 0.075f, 1.0f, 0.0f, 0.0875f, 1.0f, 0.0f, 0.1f, 1.0f, 0.0f, 0.1125f, 1.0f, 0.0f, 0.125f, 1.0f, 0.0f, 0.1375f, 1.0f, 0.0f, 0.15f, 1.0f, 0.0f, 0.1625f, 1.0f, 0.0f, 0.175f, 1.0f, 0.0f, 0.1875f, 1.0f, 0.0f, 0.2f, 1.0f, 0.0f, 0.2125f, 1.0f, 0.0f, 0.225f, 1.0f, 0.0f, 0.2375f, 1.0f, 0.0f, 0.25f, 1.0f, 0.0f, 0.2625f, 1.0f, 0.0f, 0.275f, 1.0f, 0.0f, 0.2875f, 1.0f, 0.0f, 0.3f, 1.0f, 0.0f, 0.3125f, 1.0f, 0.0f, 0.325f, 1.0f, 0.0f, 0.3375f, 1.0f, 0.0f, 0.35f, 1.0f, 0.0f, 0.3625f, 1.0f, 0.0f, 0.375f, 1.0f, 0.0f, 0.3875f, 1.0f, 0.0f, 0.4f, 1.0f, 0.0f, 0.4125f, 1.0f, 0.0f, 0.425f, 1.0f, 0.0f, 0.4375f, 1.0f, 0.0f, 0.45f, 1.0f, 0.0f, 0.4625f, 1.0f, 0.0f, 0.475f, 1.0f, 0.0f, 0.4875f, 1.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.5125f, 1.0f, 0.0f, 0.525f, 1.0f, 0.0f, 0.5375f, 1.0f, 0.0f, 0.55f, 1.0f, 0.0f, 0.5625f, 1.0f, 0.0f, 0.575f, 1.0f, 0.0f, 0.5875f, 1.0f, 0.0f, 0.6f, 1.0f, 0.0f, 0.6125f, 1.0f, 0.0f, 0.625f, 1.0f, 0.0f, 0.6375f, 1.0f, 0.0f, 0.65f, 1.0f, 0.0f, 0.6625f, 1.0f, 0.0f, 0.675f, 1.0f, 0.0f, 0.6875f, 1.0f, 0.0f, 0.7f, 1.0f, 0.0f, 0.7125f, 1.0f, 0.0f, 0.725f, 1.0f, 0.0f, 0.7375f, 1.0f, 0.0f, 0.75f, 1.0f, 0.0f, 0.7625f, 1.0f, 0.0f, 0.775f, 1.0f, 0.0f, 0.7875f, 1.0f, 0.0f, 0.8f, 1.0f, 0.0f, 0.8125f, 1.0f, 0.0f, 0.825f, 1.0f, 0.0f, 0.8375f, 1.0f, 0.0f, 0.85f, 1.0f, 0.0f, 0.8625f, 1.0f, 0.0f, 0.875f, 1.0f, 0.0f, 0.8875f, 1.0f, 0.0f, 0.9f, 1.0f, 0.0f, 0.9125f, 1.0f, 0.0f, 0.925f, 1.0f, 0.0f, 0.9375f, 1.0f, 0.0f, 0.95f, 1.0f, 0.0f, 0.9625f, 1.0f, 0.0f, 0.975f, 1.0f, 0.0f, 0.9875f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0125f, 1.0f, 0.9875f, 0.025f, 1.0f, 0.975f, 0.0375f, 1.0f, 0.9625f, 0.05f, 1.0f, 0.95f, 0.0625f, 1.0f, 0.9375f, 0.075f, 1.0f, 0.925f, 0.0875f, 1.0f, 0.9125f, 0.1f, 1.0f, 0.9f, 0.1125f, 1.0f, 0.8875f, 0.125f, 1.0f, 0.875f, 0.1375f, 1.0f, 0.8625f, 0.15f, 1.0f, 0.85f, 0.1625f, 1.0f, 0.8375f, 0.175f, 1.0f, 0.825f, 0.1875f, 1.0f, 0.8125f, 0.2f, 1.0f, 0.8f, 0.2125f, 1.0f, 0.7875f, 0.225f, 1.0f, 0.775f, 0.2375f, 1.0f, 0.7625f, 0.25f, 1.0f, 0.75f, 0.2625f, 1.0f, 0.7375f, 0.275f, 1.0f, 0.725f, 0.2875f, 1.0f, 0.7125f, 0.3f, 1.0f, 0.7f, 0.3125f, 1.0f, 0.6875f, 0.325f, 1.0f, 0.675f, 0.3375f, 1.0f, 0.6625f, 0.35f, 1.0f, 0.65f, 0.3625f, 1.0f, 0.6375f, 0.375f, 1.0f, 0.625f, 0.3875f, 1.0f, 0.6125f, 0.4f, 1.0f, 0.6f, 0.4125f, 1.0f, 0.5875f, 0.425f, 1.0f, 0.575f, 0.4375f, 1.0f, 0.5625f, 0.45f, 1.0f, 0.55f, 0.4625f, 1.0f, 0.5375f, 0.475f, 1.0f, 0.525f, 0.4875f, 1.0f, 0.5125f, 0.5f, 1.0f, 0.5f, 0.5125f, 1.0f, 0.4875f, 0.525f, 1.0f, 0.475f, 0.5375f, 1.0f, 0.4625f, 0.55f, 1.0f, 0.45f, 0.5625f, 1.0f, 0.4375f, 0.575f, 1.0f, 0.425f, 0.5875f, 1.0f, 0.4125f, 0.6f, 1.0f, 0.4f, 0.6125f, 1.0f, 0.3875f, 0.625f, 1.0f, 0.375f, 0.6375f, 1.0f, 0.3625f, 0.65f, 1.0f, 0.35f, 0.6625f, 1.0f, 0.3375f, 0.675f, 1.0f, 0.325f, 0.6875f, 1.0f, 0.3125f, 0.7f, 1.0f, 0.3f, 0.7125f, 1.0f, 0.2875f, 0.725f, 1.0f, 0.275f, 0.7375f, 1.0f, 0.2625f, 0.75f, 1.0f, 0.25f, 0.7625f, 1.0f, 0.2375f, 0.775f, 1.0f, 0.225f, 0.7875f, 1.0f, 0.2125f, 0.8f, 1.0f, 0.2f, 0.8125f, 1.0f, 0.1875f, 0.825f, 1.0f, 0.175f, 0.8375f, 1.0f, 0.1625f, 0.85f, 1.0f, 0.15f, 0.8625f, 1.0f, 0.1375f, 0.875f, 1.0f, 0.125f, 0.8875f, 1.0f, 0.1125f, 0.9f, 1.0f, 0.1f, 0.9125f, 1.0f, 0.0875f, 0.925f, 1.0f, 0.075f, 0.9375f, 1.0f, 0.0625f, 0.95f, 1.0f, 0.05f, 0.9625f, 1.0f, 0.0375f, 0.975f, 1.0f, 0.025f, 0.9875f, 1.0f, 0.0125f, 1.0f, 1.0f, 0.0f, 1.0f, 0.9875f, 0.0f, 1.0f, 0.975f, 0.0f, 1.0f, 0.9625f, 0.0f, 1.0f, 0.95f, 0.0f, 1.0f, 0.9375f, 0.0f, 1.0f, 0.925f, 0.0f, 1.0f, 0.9125f, 0.0f, 1.0f, 0.9f, 0.0f, 1.0f, 0.8875f, 0.0f, 1.0f, 0.875f, 0.0f, 1.0f, 0.8625f, 0.0f, 1.0f, 0.85f, 0.0f, 1.0f, 0.8375f, 0.0f, 1.0f, 0.825f, 0.0f, 1.0f, 0.8125f, 0.0f, 1.0f, 0.8f, 0.0f, 1.0f, 0.7875f, 0.0f, 1.0f, 0.775f, 0.0f, 1.0f, 0.7625f, 0.0f, 1.0f, 0.75f, 0.0f, 1.0f, 0.7375f, 0.0f, 1.0f, 0.725f, 0.0f, 1.0f, 0.7125f, 0.0f, 1.0f, 0.7f, 0.0f, 1.0f, 0.6875f, 0.0f, 1.0f, 0.675f, 0.0f, 1.0f, 0.6625f, 0.0f, 1.0f, 0.65f, 0.0f, 1.0f, 0.6375f, 0.0f, 1.0f, 0.625f, 0.0f, 1.0f, 0.6125f, 0.0f, 1.0f, 0.6f, 0.0f, 1.0f, 0.5875f, 0.0f, 1.0f, 0.575f, 0.0f, 1.0f, 0.5625f, 0.0f, 1.0f, 0.55f, 0.0f, 1.0f, 0.5375f, 0.0f, 1.0f, 0.525f, 0.0f, 1.0f, 0.5125f, 0.0f, 1.0f, 0.5f, 0.0f, 1.0f, 0.4875f, 0.0f, 1.0f, 0.475f, 0.0f, 1.0f, 0.4625f, 0.0f, 1.0f, 0.45f, 0.0f, 1.0f, 0.4375f, 0.0f, 1.0f, 0.425f, 0.0f, 1.0f, 0.4125f, 0.0f, 1.0f, 0.4f, 0.0f, 1.0f, 0.3875f, 0.0f, 1.0f, 0.375f, 0.0f, 1.0f, 0.3625f, 0.0f, 1.0f, 0.35f, 0.0f, 1.0f, 0.3375f, 0.0f, 1.0f, 0.325f, 0.0f, 1.0f, 0.3125f, 0.0f, 1.0f, 0.3f, 0.0f, 1.0f, 0.2875f, 0.0f, 1.0f, 0.275f, 0.0f, 1.0f, 0.2625f, 0.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.2375f, 0.0f, 1.0f, 0.225f, 0.0f, 1.0f, 0.2125f, 0.0f, 1.0f, 0.2f, 0.0f, 1.0f, 0.1875f, 0.0f, 1.0f, 0.175f, 0.0f, 1.0f, 0.1625f, 0.0f, 1.0f, 0.15f, 0.0f, 1.0f, 0.1375f, 0.0f, 1.0f, 0.125f, 0.0f, 1.0f, 0.1125f, 0.0f, 1.0f, 0.1f, 0.0f, 1.0f, 0.0875f, 0.0f, 1.0f, 0.075f, 0.0f, 1.0f, 0.0625f, 0.0f, 1.0f, 0.05f, 0.0f, 1.0f, 0.0375f, 0.0f, 1.0f, 0.025f, 0.0f, 1.0f, 0.0125f, 0.0f, 1.0f, 0.0f, 0.0f, 0.9875f, 0.0f, 0.0f, 0.975f, 0.0f, 0.0f, 0.9625f, 0.0f, 0.0f, 0.95f, 0.0f, 0.0f, 0.9375f, 0.0f, 0.0f, 0.925f, 0.0f, 0.0f, 0.9125f, 0.0f, 0.0f, 0.9f, 0.0f, 0.0f };
        private static readonly object _dflimHistogramDataLock = new object();

        private static int[] _colorAssigment;
        private static int _dataLength;
        private static uint[] _dflimArrivalTimeSumData;
        private static uint[][] _dflimHistogramData;
        private static bool _dflimNewHistogramData;
        private static ushort[] _dflimSinglePhotonData;
        private static int _lsmChannel;
        private static bool[] _lsmEnableChannel;
        private static int _maxChannels;
        private static ReportMultiROIStats _multiROIStatsCallBack;
        private static ushort[] _pixelData;
        private static byte[] _pixelDataByte;
        private static int[][] _pixelDataHistogram;
        private static bool _pixelDataReady;
        private static byte[][] _rawImg = new byte[MAX_CHANNELS + 1][];
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
        private WriteableBitmap _bitmap;
        private double[] _blackPoint;
        private int _bleachFrameIndex = 0;
        private string _bleachPath = string.Empty;
        private int _bleachPixelArrayIndex = 0;
        private List<PixelArray> _bleachPixelArrayList = new List<PixelArray>();
        private int _bleachPixelIndex = 0;
        private BleachMode _bleachScanMode;
        private int _camBitsPerPixel = 14;
        private bool _cbZSEnable;
        private Brush[] _channelColor = new Brush[MAX_CHANNELS];
        private int _channelOrderCurrentIndex = 0;
        private int _channelOrderCurrentLSMChannel = 0;
        int _channelSelection = 0;
        private Guid _commandGuid;
        private int _completedImageCount;
        private List<string> _currentChannelsLutFiles = new List<string>();
        private int _currentImageCount;
        private int _currentSLMCycleID = 0;
        private int _currentSLMSequenceID = 0;
        private int _currentSLMWaveID = 0;
        private int _currentSubImageCount;
        private int _currentTCount;
        private int _currentWellCount;
        private int _currentZCount;
        private ushort[] _dataBuffer = null;
        private bool _displayImage;
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
        private bool _IPCDownlinkFlag = false;
        private bool _isSaving;
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
        private byte[][] _pixelDataLUT;
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
        private int _rollOverPointX;
        private int _rollOverPointY;
        private RunSampleLSCustomParams _rsParams = new RunSampleLSCustomParams();
        private bool _runComplete;
        System.Timers.Timer _runPlateTimer;
        private int _runTimeCal = 0;
        private string _sampleConfig;
        StringBuilder _sbNewDir;
        private int _scanMode;
        private string _selectedWavelengthValue;
        private int _sequenceStepsNum = 0;
        private List<string> _sequenceStepWavelengthNameList = new List<string>();
        private int _shiftValue = 6;
        int _simultaneousBleachingAndImaging = 0;
        private double[] _slmBleachDelay = { 0.0, 0.0 };
        private ObservableCollection<SLMParams> _slmBleachWaveParams = new ObservableCollection<GeometryUtilities.SLMParams>();
        private int _SLMCallbackCount = 0;
        private List<string> _slmFilesInFolder;
        BackgroundWorker _slmLoader = new BackgroundWorker();
        private List<string> _slmSequencesInFolder;
        private bool _startButtonStatus;
        private DateTime _startDateTime;
        private DateTime _startUTC;
        private long _startZTime = 0;
        private string _statusMessageZ = string.Empty;
        private bool _statusMessageZUpdate = true;
        private double _stepTimeAdjustMS = 0;

        //private string _startColumn;
        //private string _startRow;
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
        private double[] _whitePoint;
        private bool _z2StageLock;
        private bool _z2StageMirror;
        private int _zEnable;
        private bool _zFastEnable = false;
        private int _zFileEnable;
        private List<double> _zFilePosList;
        private int _zFilePosRead;
        private double _zFilePosScale;
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
            _panelsEnable = true;
            _runComplete = true;
            _displayImage = true;
            _imageUpdaterVis = Visibility.Visible;
            _selectedWavelengthValue = "";
            _zEnable = 1;
            _tEnable = true;
            _backgroundWorkerDone = true;
            _maxChannels = 4;
            _blackPoint = new double[_maxChannels];
            _whitePoint = new double[_maxChannels];
            _lsmEnableChannel = new bool[MAX_CHANNELS];

            _tiffCompressionEnabled = true;
            InitializeClassArrays(_maxChannels);
            //_tbZSEnable = false;
            _tbZSEnableFGColor = Brushes.Gray;
            // _zStreamFrames = 1;
            _pmtTripCount = 0;
            //timer for stop command
            _runPlateTimer = new System.Timers.Timer();

            //Image _counter for determining the end of the row
            _imageCounter = 0;

            const int LUT_SIZE = 256;

            ChannelLuts = new Color[_maxChannels][];

            for (int i = 0; i < _maxChannels; i++)
            {
                ChannelLuts[i] = new Color[LUT_SIZE];
            }

            for (int i = 0; i < _maxChannels; i++)
            {
                _currentChannelsLutFiles.Add(string.Empty);
            }

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
            IsDisplayImageReady = false;
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
        private delegate void ReportLineProfile(IntPtr lineProfile, int length, int channelEnable, int numChannel);

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

        public event Action ExperimentStarted;

        public event PropertyChangedEventHandler PropertyChanged;

        public event EventHandler ROIStatsChanged;

        public event Action UpdataFilePath;

        public event Action<string> Update;

        public event Action<bool> UpdateBitmapTimer;

        public event Action<bool> UpdateButton;

        public event Action<string> UpdateEndSubWell;

        public event Action<string> UpdateImage;

        public event Action<string> UpdateImageName;

        public event Action<bool> UpdateMenuBarButton;

        public event Action<bool> UpdatePanels;

        public event Action<bool> UpdateRemoteConnection;

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

        public static Color[][] ChannelLuts
        {
            get;
            set;
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

        public double BlackPoint0
        {
            get
            {
                return _blackPoint[0];
            }
            set
            {
                _blackPoint[0] = Math.Min(LUT_MAX, Math.Max(LUT_MIN, value));
            }
        }

        public double BlackPoint1
        {
            get
            {
                return _blackPoint[1];
            }
            set
            {
                _blackPoint[1] = Math.Min(LUT_MAX, Math.Max(LUT_MIN, value));
            }
        }

        public double BlackPoint2
        {
            get
            {
                return _blackPoint[2];
            }
            set
            {
                _blackPoint[2] = Math.Min(LUT_MAX, Math.Max(LUT_MIN, value));
            }
        }

        public double BlackPoint3
        {
            get
            {
                return _blackPoint[3];
            }
            set
            {
                _blackPoint[3] = Math.Min(LUT_MAX, Math.Max(LUT_MIN, value));
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

        public double CamPixelSizeUM
        {
            get;
            set;
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

                InitializeClassArrays(_maxChannels);

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

        public int[] HistogramData0
        {
            get
            {

                {
                    return _pixelDataHistogram[0];
                }
            }
        }

        public int[] HistogramData1
        {
            get
            {

                {
                    return _pixelDataHistogram[1];
                }
            }
        }

        public int[] HistogramData2
        {
            get
            {

                {
                    return _pixelDataHistogram[2];
                }
            }
        }

        public int[] HistogramData3
        {
            get
            {

                {
                    return _pixelDataHistogram[3];
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

        public bool IPCDownlinkFlag
        {
            get { return _IPCDownlinkFlag; }
            set { _IPCDownlinkFlag = value; }
        }

        public bool IsDisplayImageReady
        {
            get; set;
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

        public Brush[] LSMChannelColor
        {
            get
            {
                return _channelColor;
            }
            set
            {
                _channelColor = value;
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

        public double LSMUMPerPixel
        {
            get;
            set;
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

        public int PixelBitShiftValue
        {
            get
            {

                _shiftValue = this.BitsPerPixel - 8;
                return _shiftValue;
            }
        }

        public byte[] PixelDataByte
        {
            get { return _pixelDataByte; }
            set { _pixelDataByte = value; }
        }

        public double PixelSizeUM
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

        public int RawOption
        {
            get;
            set;
        }

        public int RollOverPointIntensity0
        {
            get
            {
                if (_pixelData != null && _imageHeight != 0)
                {
                    //if the requested pixel is within the buffer size
                    int location;
                    if (1 < _numberOfImageTiles)
                    {
                        location = GetTilePixelDataLocation(_rollOverPointX, _rollOverPointY);
                    }
                    else
                    {
                        location = (int)(_rollOverPointX + (_imageWidth * _rollOverPointY));
                    }
                    if ((location >= 0) && (location < _pixelData.Length))
                    {
                        int val;
                        if (_pixelData[location] < 0)
                        {
                            val = (int)(_pixelData[location] + 32768.0);
                        }
                        else
                        {
                            val = (_pixelData[location]);
                        }

                        return val;
                    }
                }
                return 0;
            }
        }

        public int RollOverPointIntensity1
        {
            get
            {
                if (_pixelData != null && _imageHeight != 0 && _imageColorChannels >= 2)
                {
                    //if the requested pixel is within the buffer size
                    int location;
                    if (1 < _numberOfImageTiles)
                    {
                        location = GetTilePixelDataLocation(_rollOverPointX, _rollOverPointY);
                    }
                    else
                    {
                        location = (int)(_rollOverPointX + (_imageWidth * _rollOverPointY));
                    }
                    if ((location >= 0) && (location + _dataLength < _pixelData.Length))
                    {
                        int val;
                        if (_pixelData[location] < 0)
                        {
                            val = (int)(_pixelData[location + _dataLength] + 32768.0);
                        }
                        else
                        {
                            val = (_pixelData[location + _dataLength]);
                        }

                        return val;
                    }
                }
                return 0;
            }
        }

        public int RollOverPointIntensity2
        {
            get
            {
                if (_pixelData != null && _imageHeight != 0 && _imageColorChannels >= 3)
                {
                    //if the requested pixel is within the buffer size
                    int location;
                    if (1 < _numberOfImageTiles)
                    {
                        location = GetTilePixelDataLocation(_rollOverPointX, _rollOverPointY);
                    }
                    else
                    {
                        location = (int)(_rollOverPointX + (_imageWidth * _rollOverPointY));
                    }
                    if ((location >= 0) && ((location + 2 * _dataLength) < _pixelData.Length))
                    {
                        int val;
                        if (_pixelData[location] < 0)
                        {
                            val = (int)(_pixelData[location + 2 * _dataLength] + 32768.0);
                        }
                        else
                        {
                            val = (_pixelData[location + 2 * _dataLength]);
                        }

                        return val;
                    }
                }
                return 0;
            }
        }

        public int RollOverPointIntensity3
        {
            get
            {
                if (_pixelData != null && _imageHeight != 0 && _imageColorChannels >= 4)
                {
                    //if the requested pixel is within the buffer size
                    int location;
                    if (1 < _numberOfImageTiles)
                    {
                        location = GetTilePixelDataLocation(_rollOverPointX, _rollOverPointY);
                    }
                    else
                    {
                        location = (int)(_rollOverPointX + (_imageWidth * _rollOverPointY));
                    }
                    if ((location >= 0) && ((location + 3 * _dataLength) < _pixelData.Length))
                    {
                        int val;
                        if (_pixelData[location] < 0)
                        {
                            val = (int)(_pixelData[location + 3 * _dataLength] + 32768.0);
                        }
                        else
                        {
                            val = (_pixelData[location + 3 * _dataLength]);
                        }

                        return val;
                    }
                }
                return 0;
            }
        }

        public int RollOverPointX
        {
            get
            {
                return _rollOverPointX;
            }
            set
            {
                _rollOverPointX = value;
            }
        }

        public int RollOverPointY
        {
            get
            {
                return _rollOverPointY;
            }
            set
            {
                _rollOverPointY = value;
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
                    if (ThreePhotonEnable && NumberOfPlanes > 1)
                    {
                        return LSMPixelY * NumberOfPlanes;
                    }
                    else
                    {
                        return LSMPixelY;
                    }
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

        public double WhitePoint0
        {
            get
            {
                return _whitePoint[0];
            }
            set
            {
                _whitePoint[0] = Math.Min(LUT_MAX, Math.Max(LUT_MIN, value));
            }
        }

        public double WhitePoint1
        {
            get
            {
                return _whitePoint[1];
            }
            set
            {
                _whitePoint[1] = Math.Min(LUT_MAX, Math.Max(LUT_MIN, value));
            }
        }

        public double WhitePoint2
        {
            get
            {
                return _whitePoint[2];
            }
            set
            {
                _whitePoint[2] = Math.Min(LUT_MAX, Math.Max(LUT_MIN, value));
            }
        }

        public double WhitePoint3
        {
            get
            {
                return _whitePoint[3];
            }
            set
            {
                _whitePoint[3] = Math.Min(LUT_MAX, Math.Max(LUT_MIN, value));
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
                _zFastEnable = (ZStageType.PIEZO == ZStageType) ? value : false;

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
                            _zNumSteps = Math.Max(1, (int)Math.Round(Math.Abs(Math.Round((_zStopPosition - _zStartPosition), 5) / (_zStepSize / (double)Constants.UM_TO_MM)) + 1));
                        }
                        break;
                    case CaptureModes.STREAMING:
                        //stepping less than one is not realistic experiment configuration,
                        //special case for single step for step size == (stop - start)
                        _zNumSteps = ((!FastZStaircase) && (_zStepSize == (_zStopPosition - _zStartPosition) * (double)Constants.UM_TO_MM)) ?
                            Math.Max(1, (int)Math.Round(Math.Abs(Math.Round((_zStopPosition - _zStartPosition), 5) / (_zStepSize / (double)Constants.UM_TO_MM)))) :
                            Math.Max(1, (int)Math.Round(Math.Abs(Math.Round((_zStopPosition - _zStartPosition), 5) / (_zStepSize / (double)Constants.UM_TO_MM)) + 1));
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

        public static Color GetColorAssignment(int index)
        {
            Color colorAssignment = new Color();
            const int LUT_SIZE = 256;

            colorAssignment = ChannelLuts[index][LUT_SIZE - 1];

            double luminance = (0.2126 * colorAssignment.R + 0.7152 * colorAssignment.G + 0.0722 * colorAssignment.B);

            //if the color is too bright it will not
            //display on a white background
            //substitute gray if the color is too bright
            if (luminance > 240)
            {
                colorAssignment = Colors.Gray;
            }
            return colorAssignment;
        }

        public static Color[] GetColorAssignments()
        {
            Color[] colorAssignments = new Color[_maxChannels];
            const int LUT_SIZE = 256;

            for (int i = 0; i < _maxChannels; i++)
            {
                colorAssignments[i] = ChannelLuts[i][LUT_SIZE - 1];
            }

            return colorAssignments;
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

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "UpdateAndPersistCurrentDevices")]
        public static extern int UpdateAndPersistCurrentDevices();

        public static void UpdatePMTSwitchBox()
        {
            //switch depending on LSM camera type
            switch (LSMType)
            {
                case ICamera.LSMType.GALVO_RESONANCE:
                    SetDeviceParamInt((int)SelectedHardware.SELECTED_PMTSWITCH, (int)IDevice.Params.PARAM_PMT_SWITCH_POS, 0, false);
                    break;
                case ICamera.LSMType.GALVO_GALVO:
                    SetDeviceParamInt((int)SelectedHardware.SELECTED_PMTSWITCH, (int)IDevice.Params.PARAM_PMT_SWITCH_POS, 1, false);
                    break;
            }
        }

        public bool BuildChannelPalettes()
        {
            bool ret = false;

            if (null == HardwareDoc)
            {
                return ret;
            }

            string str = string.Empty;
            bool grayscaleForSingleChannel = false;

            XmlNodeList ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Wavelengths/Wavelength");

            //if this is a single channel experiment
            //check to see user wants to view the data
            //as grayscale
            if (1 == ndList.Count && 1 >= _sequenceStepWavelengthNameList.Count)
            {
                XmlDocument appSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                if (null == appSettingsDoc)
                {
                    return ret;
                }

                ndList = appSettingsDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/GrayscaleForSingleChannel");

                if (ndList.Count > 0)
                {
                    grayscaleForSingleChannel = (XmlManager.GetAttribute(ndList[0], appSettingsDoc, "value", ref str) && ("1" == str || Boolean.TrueString == str)) ?
                        true : false;
                }
            }

            ndList = HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/*");

            string chanName = string.Empty;

            for (int j = 0; j < _maxChannels; j++)
            {
                switch (j)
                {
                    case 0: chanName = "ChanA"; break;
                    case 1: chanName = "ChanB"; break;
                    case 2: chanName = "ChanC"; break;
                    case 3: chanName = "ChanD"; break;
                }

                for (int i = 0; i < ndList.Count; i++)
                {
                    if (XmlManager.GetAttribute(ndList[i], HardwareDoc, "name", ref str))
                    {
                        if (str.Contains(chanName))
                        {
                            if (grayscaleForSingleChannel)
                            {
                                str = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\luts\\" + "Gray.txt";
                            }
                            else
                            {
                                str = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\luts\\" + ndList[i].Name + ".txt";
                            }

                            //if the current lut for the channel has not changed
                            //continue on
                            //if ((_currentChannelsLutFiles.Count > 0) && (_currentChannelsLutFiles[j].Equals(str)))
                            //{
                            //    continue;
                            //}

                            if (File.Exists(str))
                            {
                                StreamReader fs = new StreamReader(str);
                                string line;
                                int counter = 0;
                                try
                                {
                                    while ((line = fs.ReadLine()) != null)
                                    {
                                        string[] split = line.Split(',');

                                        if (split[0] != null)
                                        {
                                            if (split[1] != null)
                                            {
                                                if (split[2] != null)
                                                {
                                                    ChannelLuts[j][counter] = Color.FromRgb(Convert.ToByte(split[0]), Convert.ToByte(split[1]), Convert.ToByte(split[2]));
                                                }
                                            }
                                        }
                                        counter++;
                                    }
                                }
                                catch (Exception ex)
                                {
                                    string msg = ex.Message;
                                }

                                fs.Close();

                                _currentChannelsLutFiles[j] = str;
                            }
                        }
                    }
                }
            }

            for (int i = 0; i < _maxChannels; i++)
            {
                Brush brush;

                const int PALETTE_SIZE = 256;

                double luminance = (0.2126 * ChannelLuts[i][PALETTE_SIZE - 1].R + 0.7152 * ChannelLuts[i][PALETTE_SIZE - 1].G + 0.0722 * ChannelLuts[i][PALETTE_SIZE - 1].B);

                //if the color is too bright it will not
                //display on a white background
                //substitute gray if the color is too bright
                if (luminance > 240)
                {
                    brush = new SolidColorBrush(Colors.Gray);
                }
                else
                {
                    brush = new SolidColorBrush(ChannelLuts[i][PALETTE_SIZE - 1]);
                }

                LSMChannelColor[i] = brush;
            }

            ret = true;
            return ret;
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

        public WriteableBitmap CreateBitmap()
        {
            // Define parameters used to create the BitmapSource.
            PixelFormat pf = PixelFormats.Rgb24;
            int width = _imageWidth;
            int height = _imageHeight;
            //can't build bitmap if width and/or height are zero
            if (width <= 0 || height <= 0)
            {
                return null;
            }
            int rawStride = (width * pf.BitsPerPixel + 7) / 8;
            int outputBitmapWidth = width;
            int outputBitmapHeight = height;
            bool dflimDisplayLifetimeImage = (bool)MVMManager.Instance["DFLIMControlCaptureViewModel", "DFLIMDisplayLifetimeImage", (object)false];
            bool doLifetime = (1 == ImageMethod && dflimDisplayLifetimeImage);
            int channelNum = 0;
            for (int i = 0; i < _lsmEnableChannel.Length; ++i)
            {
                if (true == (_lsmEnableChannel[i]))
                {
                    ++channelNum;
                }
            }
            if (0 == channelNum)
            {
                return null;
            }
            if (_tileDisplay && (channelNum > 1 || doLifetime))
            {

                switch (channelNum)
                {
                    case 1:
                        {
                            if (doLifetime)
                            {
                                outputBitmapWidth *= VerticalTileDisplay ? 1 : 2;
                                outputBitmapHeight *= VerticalTileDisplay ? 2 : 1;
                            }
                        }
                        break;
                    case 2:
                        {
                            outputBitmapWidth *= VerticalTileDisplay ? 1 : 3;
                            outputBitmapHeight *= VerticalTileDisplay ? 3 : 1;
                        }
                        break;
                    case 3:
                        {
                            outputBitmapWidth *= 2;
                            outputBitmapHeight *= 2;
                        }
                        break;
                    default:
                        {// All 4 Channels enabled
                            outputBitmapWidth *= VerticalTileDisplay ? 2 : 3;
                            outputBitmapHeight *= VerticalTileDisplay ? 3 : 2;
                        }
                        break;
                }
            }
            //create a new bitmpap when one does not exist or the size of the image changes
            if (_bitmap == null)
            {
                _bitmap = new WriteableBitmap(outputBitmapWidth, outputBitmapHeight, 96, 96, pf, null);
            }
            else
            {
                if ((_bitmap.Width != outputBitmapWidth) || (_bitmap.Height != outputBitmapHeight) || (_bitmap.Format != pf))
                {
                    _bitmap = new WriteableBitmap(outputBitmapWidth, outputBitmapHeight, 96, 96, pf, null);
                }
            }

            CreatePixelDataByte();

            //if there is no _pixelDataByte then we shouldn't try to create the bitmap
            if (null == _pixelDataByte)
            {
                return null;
            }

            byte[] pd = _pixelDataByte;
            if ((pd.Length / 3) == (width * height))
            {
                _bitmap.WritePixels(new Int32Rect(0, 0, width, height), pd, rawStride, 0);
                if (_tileDisplay && (1 < channelNum || doLifetime))
                {
                    int offsetWidth = VerticalTileDisplay ? 0 : 1;
                    int offsetHeight = VerticalTileDisplay ? 1 : 0;
                    for (int i = 0; i < _lsmEnableChannel.Length; ++i)
                    {
                        if (true == (_lsmEnableChannel[i]))
                        {
                            pd = _rawImg[i];
                            _bitmap.WritePixels(new Int32Rect(offsetWidth * width, offsetHeight * height, width, height), pd, rawStride, 0);
                            if (VerticalTileDisplay)
                            {
                                ++offsetHeight;
                                if (outputBitmapHeight < (height * offsetHeight + height))
                                {
                                    offsetHeight = 0;
                                    ++offsetWidth;
                                }
                            }
                            else
                            {
                                ++offsetWidth;
                                if (outputBitmapWidth < (width * offsetWidth + width))
                                {
                                    offsetWidth = 0;
                                    ++offsetHeight;
                                }
                            }
                        }
                    }
                }
            }

            if (_tileWidth == 0 || _tileHeight == 0) // not fastZ; normal mode
            {
                _tileWidth = _imageWidth;
                _tileHeight = _imageHeight;
            }
            updateTiledBitmap(_bitmap, _pixelDataByte, _imageWidth, _imageHeight, _tileWidth, _tileHeight);

            FinishedCopyingPixel();

            return _bitmap;
        }

        public void DisplayColorImage(string[] fileNames)
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
                    for (int i = 0; i < _maxChannels; i++)
                    {
                        if (fileNames[i] != null)
                        {
                            //ReadImageInfo(fileNames[i], ref width, ref height, ref colorChannels);
                            ReadImageInfo2(fileNames[i], ref width, ref height, ref colorChannels, ref numTiles, ref tileWidth, ref tileHeight);
                            _numberOfImageTiles = numTiles;
                            break;
                        }
                    }

                    // setting the parameters to be used in the View Model
                    _imageWidth = width;
                    _imageHeight = height;
                    _tileWidth = tileWidth;
                    _tileHeight = tileHeight;
                    _imageColorChannels = fileNames.Length;
                    //allocate the buffer size
                    _imageData = Marshal.AllocHGlobal(_imageWidth * _imageHeight * _imageColorChannels * 2);

                    //memset is necessary to make sure the buffer is 0 for all channels before reading the images
                    //in cases when not all four channels are on there rollover, intensity data can show up
                    //as if there was data on a channel that is not turned on
                    memset(_imageData, 0, _imageWidth * _imageHeight * _imageColorChannels * 2);

                    //read the image and output the buffer with image data
                    int result = 0;
                    if (tileWidth != 0 && tileHeight != 0) // FastZ
                    {
                        result = ReadChannelTiledImages(fileNames, _imageColorChannels, ref _imageData);
                    }
                    else
                    {
                        result = ReadChannelImages(fileNames, _imageColorChannels, ref _imageData, _imageWidth, _imageHeight);
                    }
                }

                CopyChannelData();
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

            _slmLoader.DoWork += SLMLoader_DoWork;
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
                    //=== Sequential Capture Mode is not compatible with the CCD type Cameras ===
                    if ((int)ICamera.CameraType.CCD == this.ActiveCameraType)
                    {
                        MessageBox.Show("\"Sequential Capture Mode\" is incompatible with CCD camera captures. Please clear your channel sequence options for the experiment and start again.");
                        return false;
                    }

                    //Capture Sequence Mode
                    if (1 < _sequenceStepsNum)
                    {
                        int chan0 = 0;
                        int chan1 = 0;
                        int chan2 = 0;
                        int chan3 = 0;
                        for (int i = 0; i < ndList.Count; i++)
                        {
                            XmlNodeList lsmNdList = ndList[i].SelectNodes("LSM");
                            if (0 < lsmNdList.Count)
                            {
                                int binaryValue = 0;
                                if (XmlManager.GetAttribute(lsmNdList[0], expDoc, "channel", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out binaryValue))
                                {
                                    chan0 += (Convert.ToBoolean(binaryValue & 0x1)) ? 1 : 0;
                                    chan1 += (Convert.ToBoolean(binaryValue & 0x2)) ? 1 : 0;
                                    chan2 += (Convert.ToBoolean(binaryValue & 0x4)) ? 1 : 0;
                                    chan3 += (Convert.ToBoolean(binaryValue & 0x8)) ? 1 : 0;
                                }
                            }
                        }

                        //check when streaming or bleaching
                        if (1 == this.CaptureMode || 3 == this.CaptureMode)
                        {
                            MessageBox.Show("\"Sequential Capture Mode\" is incompatible with Streaming and Bleaching captures. Please clear your channel sequence options for the experiment and start again.");
                            return false;
                        }

                        //=== Sequential Capture Mode only allows referencing a channel once per Capture Sequence ===
                        if (1 < chan0 || 1 < chan1 || 1 < chan2 || 1 < chan3)
                        {
                            MessageBox.Show("\"Sequential Capture Mode\" Each channel is allowed only once per Capture Sequence. Please go back to Capture Setup and ensure you don't have a channel selected in more than one sequence step.");
                            return false;
                        }
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

        public void ResetImage()
        {
            _pixelDataReady = false;
            _pixelData = null;
            _pixelDataByte = null;
            this.IsDisplayImageReady = false;
        }

        public bool SetBleachFile(string slmFileName, int cycleNum)
        {
            return ((1 == SetBleachWaveformFile(slmFileName, cycleNum)) ? true : false);
        }

        public bool Start()
        {
            if (IsRunning() || _slmLoader.IsBusy)
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
                    if (IPCDownlinkFlag == false && RemoteConnection == true)
                    {
                        SendToIPCController(ThorPipeCommand.StopAcquiring);
                    }
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
        private static extern int ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)] string path, ref int width, ref int height, ref int colorChannels);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo2")]
        private static extern int ReadImageInfo2([MarshalAs(UnmanagedType.LPWStr)] string path, ref int width, ref int height, ref int colorChannels, ref int numOfTiles, ref int tileWidth, ref int tileHeight);

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
                string[] fileNames = new string[1];
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
                    fileNames[0] = strPreview;
                    _foundChannelCount++;
                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Built color image of " + strPreview);
                }
                else if (File.Exists(strTemp))
                {
                    fileNames[0] = strTemp;
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

                XmlNodeList ndList = _hardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/*");
                XmlNodeList ndListHW = _hardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

                List<string> fileNames = new List<string>();
                _foundChannelCount = 0;

                for (int k = 0; k < ndListHW.Count; k++)
                {
                    for (int i = 0; i < ndList.Count; i++)
                    {
                        str = ndList[i].Attributes["name"].Value.ToString();

                        if (str.Contains(ndListHW[k].Attributes["name"].Value.ToString()))
                        {
                            StringBuilder sbTemp = new StringBuilder();
                            sbTemp.AppendFormat("{0}{1}{2}{3}{4}{5}", exppath, ndListHW[k].Attributes["name"].Value.ToString(), "_" + ((int)1).ToString(imgNameFormat),
                                "_" + _currentSubImageCount.ToString(imgNameFormat), "_" + _currentZCount.ToString(imgNameFormat), "_" + _currentTCount.ToString(imgNameFormat) + ".tif");
                            string strTemp = sbTemp.ToString();

                            StringBuilder sbPreview = new StringBuilder();
                            sbPreview.AppendFormat("{0}{1}_Preview.tif", exppath, ndListHW[k].Attributes["name"].Value.ToString());
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
                    }
                }

                if (_foundChannelCount > 0)
                {
                    //set the image file path for display
                    DisplayColorImage(fileNames.ToArray());
                }
            }
        }

        private void buildLookUpTable()
        {
            double shiftValueResult = 64;

            //original shift value is determined by the examining camera type
            shiftValueResult = Math.Pow(2, _shiftValue);

            if (null == _pixelDataLUT)
            {
                _pixelDataLUT = new byte[_maxChannels][];

                for (int m = 0; m < _maxChannels; m++)
                {
                    _pixelDataLUT[m] = new byte[ushort.MaxValue + 1];
                }
            }

            //Build the 12/14-bit to 8-bit Lut
            for (int m = 0; m < _maxChannels; m++)
            {
                for (int k = 0; k < _pixelDataLUT[m].Length; k++)
                {
                    double val = (255.0 / (shiftValueResult * (_whitePoint[m] - _blackPoint[m]))) * (k - _blackPoint[m] * shiftValueResult);
                    val = (val >= 0) ? val : 0;
                    val = (val <= 255) ? val : 255;

                    _pixelDataLUT[m][k] = (byte)Math.Round(val);
                }
            }
        }

        private void clearDataHistogram()
        {
            for (int k = 0; k < _maxChannels; k++)
            {
                Array.Clear(_pixelDataHistogram[k], 0, PIXEL_DATA_HISTOGRAM_SIZE);
            }
        }

        private void CopyChannelData()
        {
            //only copy to the buffer when pixel data is not being read
            if (_pixelDataReady == false)
            {
                _dataLength = (_imageWidth * _imageHeight);

                if ((_pixelData == null) || (_pixelData.Length != (_dataLength * _imageColorChannels)))
                {
                    _pixelData = new ushort[_dataLength * _imageColorChannels];

                    _pixelDataByte = new byte[_dataLength * 3];
                }

                if (1 == ImageMethod)
                {
                    const int DFLIM_HISTOGRAM_BINS = 256;
                    //1 datalength for photon num buffer (intensity) (USHORT)
                    //1 datalength for single photon sum buffer (USHORT)
                    //2 datalength for arrival time sum buffer (UINT32)
                    //4 DFLIM_HISTOGRAM_BINS for dflim histogram (UINT32)
                    int intensityDataBufferSize = _imageWidth * _imageHeight * sizeof(ushort);

                    int totalBufferSize = (_dataLength * 4 + DFLIM_HISTOGRAM_BINS * 2);

                    if (_dataBuffer == null || _dataBuffer.Length != totalBufferSize * _imageColorChannels)
                    {
                        _dataBuffer = new ushort[totalBufferSize * _imageColorChannels];
                    }
                    MemoryCopyManager.CopyIntPtrMemory(_imageData, _dataBuffer, 0, _dataBuffer.Length);
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
                            Buffer.BlockCopy(_dataBuffer, i * DFLIM_HISTOGRAM_BINS * sizeof(UInt32), _dflimHistogramData[i], 0, DFLIM_HISTOGRAM_BINS * sizeof(UInt32));
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

                    Array.Copy(_dataBuffer, DFLIM_HISTOGRAM_BINS * SHORTS_PER_BIN * _imageColorChannels, _pixelData, 0, _dataLength * _imageColorChannels);

                    //copy out dflim single photon data buffer
                    Array.Copy(_dataBuffer, DFLIM_HISTOGRAM_BINS * SHORTS_PER_BIN * _imageColorChannels + _dataLength * _imageColorChannels, _dflimSinglePhotonData, 0, _imageColorChannels * _dataLength);

                    //copy out dflim arrival time sum data buffer
                    Buffer.BlockCopy(_dataBuffer, DFLIM_HISTOGRAM_BINS * sizeof(UInt32) * _imageColorChannels + _dataLength * sizeof(UInt32) * _imageColorChannels, _dflimArrivalTimeSumData, 0, _dataLength * sizeof(Int32) * _imageColorChannels);
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
                _pixelDataReady = true;
            }
        }

        private void CreatePixelDataByte()
        {
            if (null == _pixelDataByte)
            {
                return;
            }

            //need to rebuid the color image because a palette option is not available for RGB images
            if ((_pixelData != null) && (_dataLength * _imageColorChannels == _pixelData.Length))
            {
                clearDataHistogram();
                buildLookUpTable();

                //if the capture sequence Steps > 1, update the LSM channel
                //to the current one after each image.
                if (0 < _sequenceStepsNum)
                {
                    this.ChannelSelection = _channelOrderCurrentLSMChannel;
                }
                try
                {
                    byte valRaw;
                    byte valNormalized;
                    ushort pixelVal = 0;
                    Array.Clear(_pixelDataByte, 0, _pixelDataByte.Length);

                    int chan = 0;
                    for (int k = 0; k < _maxChannels; k++)
                    {
                        if (((ChannelSelection >> k) & 0x1) > 0)
                        {
                            if (_rawImg[k] == null || _rawImg[k].Length != 3 * _dataLength)
                            {
                                _rawImg[k] = new byte[3 * _dataLength];
                            }
                            for (int j = 0; j < _dataLength; j++)
                            {
                                int i = 3 * j;
                                pixelVal = _pixelData[j + chan * _dataLength];
                                valRaw = (byte)(pixelVal >> _shiftValue);
                                valNormalized = _pixelDataLUT[k][pixelVal];
                                Color col = ChannelLuts[k][valNormalized];
                                _rawImg[k][i] = col.R;
                                _rawImg[k][i + 1] = col.G;
                                _rawImg[k][i + 2] = col.B;
                                if (col.R > _pixelDataByte[i]) _pixelDataByte[i] = col.R;
                                if (col.G > _pixelDataByte[i + 1]) _pixelDataByte[i + 1] = col.G;
                                if (col.B > _pixelDataByte[i + 2]) _pixelDataByte[i + 2] = col.B;
                                _pixelDataHistogram[k][valRaw]++;
                            }
                            ++chan;
                        }
                    }
                }
                catch (Exception e)
                {
                    string str = e.Message;
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RunSampleLS GetPixelDataByte " + this.GetType().Name + str);
                }
            }

            bool dflimDisplayLifetimeImage = (bool)MVMManager.Instance["DFLIMControlCaptureViewModel", "DFLIMDisplayLifetimeImage", (object)false];
            if (1 == ImageMethod && dflimDisplayLifetimeImage) //DFLIM Image
            {
                CreatePixelDataByteDFLIM();
            }
        }

        private void CreatePixelDataByteDFLIM()
        {
            try
            {
                int idx;
                if (null == _pixelDataByte)
                {
                    return;
                }
                for (int k = 0; k < MAX_CHANNELS; k++)
                {
                    if (_rawImg[k] == null || _rawImg[k].Length != 3 * _dataLength)
                    {
                        _rawImg[k] = new byte[3 * _dataLength];
                    }
                }

                var tau_high_ns = ((CustomCollection<float>)MVMManager.Instance["DFLIMControlCaptureViewModel", "DFLIMTauHigh"]);
                var tau_low_ns = ((CustomCollection<float>)MVMManager.Instance["DFLIMControlCaptureViewModel", "DFLIMTauLow"]);
                var lut_high_bins = ((CustomCollection<uint>)MVMManager.Instance["DFLIMControlCaptureViewModel", "DFLIMLUTHigh"]);
                var lut_low_bins = ((CustomCollection<uint>)MVMManager.Instance["DFLIMControlCaptureViewModel", "DFLIMLUTLow"]);
                var tZero_ns = ((CustomCollection<float>)MVMManager.Instance["DFLIMControlCaptureViewModel", "DFLIMTZero"]);
                int chan = 0;
                for (int k = 0; k < MAX_CHANNELS; k++)
                {
                    double binDuration = 1;

                    if (_lsmEnableChannel[k])
                    {
                        lock (_dflimHistogramDataLock)
                        {
                            if (null == _dflimHistogramData || _dflimHistogramData[chan].Length < 256 ||
                                0 == _dflimHistogramData[chan][254] || 0 == _dflimHistogramData[chan][255])
                            {
                                continue;
                            }
                            binDuration = 5.0 * (double)_dflimHistogramData[chan][255] / 128.0 / (double)_dflimHistogramData[chan][254];
                        }

                        double tau_high = tau_high_ns[k] / binDuration;
                        double tau_low = tau_low_ns[k] / binDuration;
                        double lut_high = lut_high_bins[k];
                        double lut_low = lut_low_bins[k];
                        double tZeroBins = tZero_ns[k] / binDuration;

                        int num_total = _dataLength;
                        int averageMode = _averageMode;
                        int averageFrames = 0 == averageMode ? 1 : _averageNum;
                        double tauscale = (tau_high > tau_low) ? (1.0f / (tau_high - tau_low)) : 1.0f;
                        double brightness;
                        double tau_scaled;

                        //Build the LifeTime Image
                        for (int pix = 0; pix < num_total; pix++)
                        {
                            try
                            {
                                int q = pix;
                                int n = q * 3;
                                brightness = ((double)_dflimSinglePhotonData[q + chan * num_total] / (double)averageFrames - lut_low) / (lut_high - lut_low);
                                brightness = (brightness < 0) ? 0 : ((brightness > 1) ? 1 : brightness);

                                if (_dflimSinglePhotonData[q + chan * num_total] != 0)
                                {
                                    tau_scaled = tauscale * ((double)_dflimArrivalTimeSumData[q + chan * num_total] / _dflimSinglePhotonData[q + chan * num_total] - tau_low - tZeroBins);
                                }
                                else
                                {
                                    tau_scaled = 0;
                                }

                                idx = (tau_scaled < 0) ? 0 : ((tau_scaled > 1) ? 255 : (int)(tau_scaled * 255 + 0.5));

                                byte red = (byte)(brightness * 255 * _dflimColorMap[3 * idx]);
                                byte green = (byte)(brightness * 255 * _dflimColorMap[3 * idx + 1]);
                                byte blue = (byte)(brightness * 255 * _dflimColorMap[3 * idx + 2]);

                                _rawImg[k][n] = red;
                                _rawImg[k][n + 1] = green;
                                _rawImg[k][n + 2] = blue;
                            }
                            catch (Exception ex)
                            {
                                ex.ToString();
                            }
                        }
                        ++chan;
                    }
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
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

        private void FinishedCopyingPixel()
        {
            _pixelDataReady = false;
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
            WaveformBuilder.InitializeParams((int)bFieldSize, FieldScaleFine, Pixel, OffsetActual, OffsetFine, bScaleYScan / 100, (int)bVerticalScan, (int)bHorizontalFlip, bPowerMin, WaveformDriverType);
        }

        private void InitializeClassArrays(int channels)
        {
            _whitePoint = new double[channels];
            _blackPoint = new double[channels];
            _pixelDataHistogram = new int[channels][];
            _colorAssigment = new int[channels];

            for (int i = 0; i < channels; i++)
            {
                _whitePoint[i] = LUT_SIZE - 1;
                _blackPoint[i] = 0;
                _pixelDataHistogram[i] = new int[PIXEL_DATA_HISTOGRAM_SIZE];
            }
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
            LoadedSLMName = (PreLoadNextSLMSequence()) ? LoadedSLMName : string.Empty;

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
                    LoadedSLMName = (PreLoadNextSLMSequence()) ? LoadedSLMName : string.Empty;
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
        /// reload available SLM patterns from sequence file, return true if loaded any.
        /// </summary>
        /// <returns></returns>
        private bool LoadSLMSequence(string sequenceFile)
        {
            if (null == sequenceFile)
                return false;

            bool retVal = true;
            try
            {
                switch (_runTimeCal)
                {
                    case 2:
                        //reload patterns here when extremely short pattern time
                        List<int> patternIDs = new List<int>();
                        using (TextReader reader = new StreamReader(sequenceFile))
                        {
                            string line;
                            while ((line = reader.ReadLine()) != null)
                            {
                                patternIDs.Add(Int32.Parse(line));
                            }
                        }
                        //find out maximum pattern time for SLM timeout in msec, PatternID:1-based
                        int timeoutVal = 0;
                        foreach (int i in patternIDs)
                        {
                            timeoutVal = (int)Math.Max(timeoutVal,
                                _slmBleachWaveParams[i - 1].BleachWaveParams.PrePatIdleTime + _slmBleachWaveParams[i - 1].BleachWaveParams.Iterations * (_slmBleachWaveParams[i - 1].BleachWaveParams.PreIdleTime + _slmBleachWaveParams[i - 1].Duration + _slmBleachWaveParams[i - 1].BleachWaveParams.PostIdleTime) + _slmBleachWaveParams[i - 1].BleachWaveParams.PostPatIdleTime);
                        }

                        //load to SLM
                        for (int j = 0; j < patternIDs.Count; j++)
                        {
                            string waveFileName = SLMWaveformFolder[0] + "\\" + SLMWaveBaseName[1] + "_" + _slmBleachWaveParams[patternIDs[j] - 1].BleachWaveParams.ID.ToString("D" + FileName.GetDigitCounts().ToString()) + ".bmp";
                            if (File.Exists(waveFileName))
                            {
                                bool doStart = (j == (patternIDs.Count - 1)) ? true : false;
                                retVal = this.LoadSLMPatternName(_runTimeCal, j, waveFileName, doStart, (int)ICamera.LSMType.STIMULATE_MODULATOR == ResourceManagerCS.GetBleacherType(), _slmBleachWaveParams[patternIDs[j] - 1].PhaseType, timeoutVal);
                                if (!retVal)
                                    break;
                            }
                        }
                        break;
                    default:
                        //allow runtime load of SLM sequence
                        return (1 == ResourceManagerCS.SetDeviceParamString((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_SEQ_FILENAME, sequenceFile, (int)IDevice.DeviceSetParamType.NO_EXECUTION));
                }
                return retVal;
            }
            catch (Exception e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ReLoadSLMPatterns Error:" + e.Message);
                return false;
            }
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
            _runTimeCal = SLMSequenceOn ? 1 : 0;
            for (int i = 0; i < _slmBleachWaveParams.Count; i++)
            {
                timeoutVal = (int)Math.Max(timeoutVal, _slmBleachWaveParams[i].BleachWaveParams.PrePatIdleTime + _slmBleachWaveParams[i].BleachWaveParams.Iterations * (_slmBleachWaveParams[i].BleachWaveParams.PreIdleTime + _slmBleachWaveParams[i].Duration + _slmBleachWaveParams[i].BleachWaveParams.PostIdleTime) + _slmBleachWaveParams[i].BleachWaveParams.PostPatIdleTime);
                if (_slmBleachWaveParams[i].Duration < (double)Constants.SLM_PATTERN_TIME_MIN_MS)
                    _runTimeCal = 2;
            }

            //keep slm phase type available if not from SLM param:
            if (1 > SLMphaseType.Count)
                SLMphaseType.Add(SLM3D ? 1 : 0);

            //load to SLM:
            if (2 == _runTimeCal && SLMSequenceOn)
            {
                //do load of patterns at sequence reload ...
            }
            else
            {
                bool phaseDirect = (int)ICamera.LSMType.STIMULATE_MODULATOR == ResourceManagerCS.GetBleacherType();
                for (int i = 0; i < _loadedSLMPatternsCnt; i++)
                {
                    if (!_slmLoader.CancellationPending)
                    {
                        StatusMessage = "Loading SLM pattern # (" + i + ")";
                        bool doStart = (i == (_loadedSLMPatternsCnt - 1)) ? true : false;
                        this.LoadSLMPatternName(_runTimeCal, i, _loadedSLMPatterns[i], doStart, phaseDirect, (i < SLMphaseType.Count ? SLMphaseType[i] : SLMphaseType[0]), timeoutVal);
                        System.Threading.Thread.Sleep(1);
                    }
                }
            }

            return (!_slmLoader.CancellationPending && 0 < _loadedSLMPatternsCnt) ? true : false;
        }

        private string PreLoadNextSLM()
        {
            string LoadedSLMName = string.Empty;

            if (!Directory.Exists((SLMSequenceOn ? SLMWaveformPath[1] : SLMWaveformPath[0])))
                return LoadedSLMName;

            //get files in folder:
            _slmFilesInFolder = Directory.EnumerateFiles((SLMSequenceOn ? SLMWaveformPath[1] : SLMWaveformPath[0]), "*.raw ", SearchOption.TopDirectoryOnly).ToList();

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
                if (this.LoadSLMPatternName(_runTimeCal, _loadedSLMPatternsCnt, addedFiles[i], true, phaseDirect, SLMphaseType.Last()))
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
        private bool PreLoadNextSLMSequence()
        {
            //return if not in advance sequence mode
            if (!SLMSequenceOn)
                return true;

            string LoadedSequenceName = string.Empty;

            //get files in folder:
            _slmSequencesInFolder = Directory.EnumerateFiles(SLMWaveformPath[1], "*.txt ", SearchOption.TopDirectoryOnly).ToList();

            //locate first filename to load from the list:
            while ((string.Empty == LoadedSequenceName) && (_slmSequencesInFolder.Count > _currentSLMSequenceID))
            {
                string seqFileName = SLMWaveformPath[1] + "\\" + SLMWaveBaseName[2] + "_" + (_currentSLMSequenceID + 1).ToString("D" + FileName.GetDigitCounts().ToString()) + ".txt";

                //check if available in the folder, skip if just loaded:
                string loadedFileName = _loadedSLMSequences.FirstOrDefault(checkString => checkString.Contains(seqFileName));
                if (null != loadedFileName)
                {
                    LoadedSequenceName = loadedFileName;
                }
                else
                {
                    string matchingFileName = _slmSequencesInFolder.FirstOrDefault(checkString => checkString.Contains(seqFileName));
                    if (LoadSLMSequence(matchingFileName))
                    {
                        LoadedSequenceName = matchingFileName;
                        _loadedSLMSequences.Add(LoadedSequenceName);
                        //_afterSLMCycle = false;
                    }
                }
                _currentSLMSequenceID++;
            }

            //flag only if there's more to load:
            _lastInSLMCycle = (0 < _slmSequencesInFolder.Except(_loadedSLMSequences).OrderBy(s => s).ToList().Count()) ? false : true;

            //if not loaded from the list, check if other files added:
            if (string.Empty == LoadedSequenceName)
            {
                //_afterSLMCycle = true;
                List<string> addedFiles = _slmSequencesInFolder.Except(_loadedSLMSequences).OrderBy(s => s).ToList();
                if (0 < addedFiles.Count())
                {
                    if (LoadSLMSequence(addedFiles[0]))
                    {
                        LoadedSequenceName = addedFiles[0];
                        _loadedSLMSequences.Add(LoadedSequenceName);
                    }
                }
                _lastInSLMCycle = (1 >= addedFiles.Count()) ? true : _lastInSLMCycle;
            }

            if (string.Empty != LoadedSequenceName)
            {
                StatusMessage = string.Format("Loading cycle {0}, SLM sequence: '{1}' ...", (_currentSLMCycleID + 1).ToString(), Path.GetFileNameWithoutExtension(LoadedSequenceName));
            }

            return (string.Empty == LoadedSequenceName) ? false : true;
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
            SendToIPCController(ThorPipeCommand.NotifySavedFile, message);
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

                    //Ensure that the bitmap has updated with the most recent data
                    if (null != UpdateImage) UpdateImage("image");

                    _experimentStatus = "Complete";

                    LogPMTTripCount();

                    UpdateExperimentFile(_experimentXMLPath, true);

                    if (IPCDownlinkFlag == false && RemoteConnection == true)
                    {
                        SendToIPCController(ThorPipeCommand.StopAcquiring);
                    }

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

        private void RunSampleLSUpdateCaptureComplete(ref int index)
        {
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
                            this.ChannelSelection = _channelOrderCurrentLSMChannel;
                            SetSequenceStepMCLS(expDoc);
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

                    XmlNodeList wavelengthsNdList = ndList[_channelOrderCurrentIndex].SelectNodes("Wavelengths");
                    XmlNodeList wavelengthNdList = wavelengthsNdList[0].SelectNodes("Wavelength");
                    for (int j = 0; j < wavelengthNdList.Count; j++)
                    {
                        if (XmlManager.GetAttribute(wavelengthNdList[j], expDoc, "name", ref str))
                            _sequenceStepWavelengthNameList.Add(str);
                    }
                    BuildChannelPalettes();
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
            ActiveExperimentPath = ResourceManagerCS.GetCaptureTemplatePathString() + "Active.xml";
            if (true == _runComplete)
            {

                //Load active.xml accounting for possible lockup during scripting
                XmlDocument expDoc = new XmlDocument();
                if (!TryLoadDocument(expDoc, ActiveExperimentPath))
                {
                    return;// false;
                }

                //Check for compatibility
                if (!ExperimentSettingsValidForCapture(expDoc))
                {
                    return;// false;
                }

                _currentWellCount = 0;
                _currentImageCount = 0;
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

                if (Directory.Exists(_sbNewDir.ToString()) || (XmlManager.GetAttribute(node, appSettings, "alwaysAppendIndex", ref strTemp) && ("1" == strTemp || Boolean.TrueString == strTemp)))
                {
                    FileName name = new FileName(_experimentName);
                    name.MakeUnique(_outputPath);

                    //Confirm That User Wants New Name
                    if (!DontAskUniqueExperimentName)
                    {
                        Application.Current.Dispatcher.Invoke((Action)delegate
                        {
                            string msg = string.Format("A data set already exists at this location. Would you like to use the name {0} instead?", name.FullName);
                            CustomMessageBox messageBox = new CustomMessageBox(msg, "Data already exists", "Don't ask again", "Yes", "No");
                            if (!messageBox.ShowDialog().GetValueOrDefault(false))
                            {
                                return;// false;
                            }
                            else
                            {
                                DontAskUniqueExperimentName = messageBox.CheckBoxChecked;
                            }
                        });
                    }

                    //Rename
                    _experimentName = name;
                    _sbNewDir = new StringBuilder(_outputPath + "\\" + _experimentName.FullName);

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

                if (IPCDownlinkFlag == false && RemoteConnection == true && RemoteSavingStats == true)
                {
                    SendToIPCController(ThorPipeCommand.StartAcquiring, _sbNewDir.ToString());
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
                }
                catch (Exception ex)
                {
                    string str = ex.Message;
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + "RunSampleLSModule: Unable to save new experiment path.");
                }
                //overwrite the active experiment settings
                File.Copy(ActiveExperimentPath, _experimentXMLPath);

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
                    if (!PreBleachCheck())
                        return;
                }

                _runComplete = false;
                _experimentCompleteForProgressBar = false;

                _statusMessageZUpdate = true;

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

                        BuildChannelPalettes();

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

        private void SLMLoader_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Cancelled)
            {
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
            if ((true == _newImageAvailable) && (false == IsDisplayImageReady) && (_displayImage))
            {
                BuildColorImage();
                IsDisplayImageReady = true;
                _newImageAvailable = false;
                //do not trigger view model for the bitmap. A timer will update the bitmap and read from the model changes
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
                XmlManager.SetAttribute(nodeList[0], ExperimentDoc, "zFastMode", (Convert.ToInt32(FastZStaircase) + (int)ZPiezoAnalogMode.ANALOG_MODE_SINGLE_WAVEFORM).ToString());
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

                        _tFrames = _streamFrames / (_zNumSteps + FlybackFrames);
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
                if (((0 == CaptureMode) && (1 == _zEnable)) || ((1 == CaptureMode) && (ZFastEnable)))
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
            }//hello

            //If the Capture Sequence Steps > 1 and updateCaptureSequence is set to true
            //update the settings file to have all the wavelengths, channels set
            //by doing this the experiment when done can be reviewed like any other experiment
            if (true == updateCaptureSequence)
            {
                if (0 < _sequenceStepsNum)
                {
                    XmlNodeList sequenceStepNdList = ExperimentDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
                    long totalChan = 0;
                    List<string> wavelengthNameList = new List<string>();
                    List<int> wavelengthExpTimeList = new List<int>();
                    string nyquistExWavelengthNM = string.Empty;
                    string nyquistEmWavelengthNM = string.Empty;
                    for (int i = 0; i < sequenceStepNdList.Count; i++)
                    {
                        XmlNodeList lsmNdList = sequenceStepNdList[i].SelectNodes("LSM");
                        if (sequenceStepNdList.Count <= 0) return false;
                        string str = string.Empty;
                        if (XmlManager.GetAttribute(lsmNdList[0], ExperimentDoc, "channel", ref str))
                        {
                            int val = 0;
                            if (int.TryParse(str, out val)) totalChan += val;
                        }

                        XmlNodeList wavelengthsNdList = sequenceStepNdList[i].SelectNodes("Wavelengths");
                        XmlNodeList wavelengthNdList = wavelengthsNdList[0].SelectNodes("Wavelength");
                        for (int j = 0; j < wavelengthNdList.Count; j++)
                        {
                            XmlManager.GetAttribute(wavelengthNdList[j], ExperimentDoc, "name", ref str);
                            wavelengthNameList.Add(str);
                            XmlManager.GetAttribute(wavelengthNdList[j], ExperimentDoc, "exposureTimeMS", ref str);
                            int val = 0;
                            int.TryParse(str, out val);
                            wavelengthExpTimeList.Add(val);
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
                            wavelengthsExposureTime.Add(wavelengthNameList[i], wavelengthExpTimeList[i]);
                        }

                        foreach (KeyValuePair<string, int> wavelengthExposureTime in wavelengthsExposureTime)
                        {
                            XmlElement newElement = CreateWavelengthTag(wavelengthExposureTime.Key, wavelengthExposureTime.Value);
                            ndList[0].AppendChild(newElement);
                        }
                        XmlElement newElement2 = this.ExperimentDoc.CreateElement("ChannelEnable");
                        XmlAttribute ChanAttribute = this.ExperimentDoc.CreateAttribute("Set");
                        ChanAttribute.Value = totalChan.ToString();
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
            // Define parameters used to create the BitmapSource.
            int rawStride = (tileWidth * _bitmap.Format.BitsPerPixel + 7) / 8;
            int tileSize = tileWidth * tileHeight * 3;
            int tileIndex = 0;

            try
            {
                if (data.Length == width * height * 3)
                {
                    for (int y = 0; y < height; y += tileHeight)
                    {
                        for (int x = 0; x < width; x += tileWidth)
                        {
                            int offset = tileIndex * tileSize;
                            //ArraySegment<byte> tileData = new ArraySegment<byte>(data, offset, tileSize);
                            bm.WritePixels(new Int32Rect(x, y, tileWidth, tileHeight), data, rawStride, offset);
                            tileIndex++;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                string errMsg = ex.Message;
            }
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