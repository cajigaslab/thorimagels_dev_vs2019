namespace ImageReviewDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Resources;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Serialization;

    using AviFile;

    using BitMiracle.LibTiff.Classic;

    using HDF5CS;

    using ImageReviewDll.Model;
    using ImageReviewDll.OME;

    using LineProfileWindow;

    using MatlabEngineWrapper;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;
    using Microsoft.Win32;

    using MultiROIStats;

    using OverlayManager;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    using d = System.Drawing;

    using i = System.Drawing.Imaging;

    public static class DispatcherEx
    {
        #region Methods

        public static void InvokeOrExecute(this Dispatcher dispatcher, Action action)
        {
            if (dispatcher.CheckAccess())
            {
                action();
            }
            else
            {
                dispatcher.BeginInvoke(DispatcherPriority.Normal, action);
            }
        }

        #endregion Methods
    }

    /// <summary>
    /// ViewModel class for the ExpSetup model object
    /// </summary>
    public class ImageReviewViewModel : ViewModelBase, ThorSharedTypes.IMVMReview
    {
        #region Fields

        public static bool _bitmapIsLoading = false;
        public static bool _stopRequested = false;

        public ObservableCollection<double> _remoteFocusPositions = new ObservableCollection<double>();

        const double MAX_ZOOM = 1000; // In  1000x
        const double MIN_ZOOM = .01; // Out 100x

        // wrapped ImageReview object
        private readonly ImageReview _imageReview;

        private static Mutex InUpdateROIStatsMut = new Mutex();

        private bool _allowImageUpdate = false;
        private AviManager _aviManager = null;
        private VideoStream _aviStream = null;
        private BackgroundWorker _bw = new BackgroundWorker();
        private BackgroundWorker _bwOrthogonalImageLoader = new BackgroundWorker();
        private BackgroundWorker _bwStatsLoader = new BackgroundWorker();
        //Line profile uses this to check if Stats calculate window is currently loading
        private static bool _bwStatsLoaderDone = true; 
        private CaptureModes _captureMode;
        private Brush[] _channelColor = new Brush[ImageReview.MAX_CHANNELS];
        private int _completedImageCount;
        private IUnityContainer _container;
        private CurrentMovieParameterEnum _currentMovieParameter;
        private ICommand _displayROIStatsOptionsCommand;
        private IEventAggregator _eventAggregator;
        private bool _experimentReviewOpened = false;
        private int _fieldSizeX;
        private int _fieldSizeY;
        private H5CSWrapper _hdf5Reader = null;
        private int _imageBlkIndex;
        Thread _imageLoadingThread = null;
        private string _imagePathandName;
        private bool _indexControlsEnabled = true;
        private Visibility[] _isChannelVisible = new Visibility[ImageReview.MAX_CHANNELS];
        private volatile bool _isExperimentLoading = false;
        private bool _isRawExperiment = false;
        private bool _isRemoteFocus = false;
        private bool _isScaleOnOffChecked = false;
        private double _ivHeight;
        private double _iVScrollBarHeight;
        ScrollBarVisibility _ivScrollbarVisibility = ScrollBarVisibility.Hidden;
        private LineProfile _lineProfile = null;
        private bool _lineProfileActive;
        private bool _liveSnapshotStatus;
        volatile bool _loadImages = false;
        private bool _maskReady;
        private int _mEnd;
        private int _mEndValue;
        private ICommand _mEndValueMinusCommand;
        private ICommand _mEndValuePlusCommand;
        private string[][] _movieFileNameList;
        private string _movieFilePath;
        private double _movieFPS;
        bool _mROIShowOverlays = false;
        bool _mROISpatialDisplaybleEnable = true;
        private int _mStart;
        private int _mStartValue;
        private ICommand _mStartValueMinusCommand;
        private ICommand _mStartValuePlusCommand;
        private List<MultiROIStatsUC> _multiROIStatsWindows = null;
        private string _outputDirectory;
        private string _outputExperiment;
        private bool _panelsEnable = true;
        private List<ImageFileNameClass> _primaryChannelFileNames;
        private int _primaryChannelIndex = 0;
        private int _progressPercentage;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private IRegionManager _regionManager;
        private ICommand _roiCalculationCommand;
        private int _roiCalMode = 1; //1: ROI Calculation, 2: ROI Load
        private bool _roiControlEnabled;
        private ICommand _roiLoadCommand;
        private bool _roiStatsChartActive;
        private List<ROIStatsChartWin> _roiStatsCharts = null;
        private bool _roiStatsTableActive;
        private Thickness _roiToolbarMargin = new Thickness(0, 0, 0, 0);
        bool _runImageLoadingThread = false;
        private int _selectedScanArea = 0;

        //private ICommand _scanAreaIDMinusCommand;
        //private ICommand _scanAreaIDPlusCommand;
        //private int _scanAreaIndex = 0;
        //private int _scanAreaIndexMax = 0;
        //private int _scanAreaIndexMin = 0;
        //private bool _scanAreaIsEnabled;
        //private bool _scanAreaIsLive = false;
        //private bool _scanAreaVisible = false;
        private int _sliderSpMax;
        private int _sliderSpMin;
        private int _sliderTMax;
        private int _sliderTMin;
        private int _sliderZMax;
        private int _sliderZMin;
        private int _sliderZStreamMax;
        private int _sliderZStreamMin;
        private SpinnerProgress.SpinnerProgressControl _spinner;
        private Window _spinnerWindow = null;
        private bool _spIsEnabled;
        private bool _spIsLive;
        private ImageReviewDll.View.SplashScreen _splash;
        private ICommand _spValueMinusCommand;
        private ICommand _spValuePlusCommand;
        private bool _tIsEnabled;
        private bool _tIsLive;
        private int _totalImageCount;
        private int _tValue3D;
        private ICommand _tValueMinusCommand;
        private ICommand _tValuePlusCommand;
        private object _updateBitmapLock = new object();
        private ICommand _viewMatlabCOmmand;
        private ICommand _viewOMEXMLCommand;
        private ICommand _viewThorAnalysisCommand;

        // 0: 2D viewer
        // 1: 3D viewer
        private int _viewType;
        private ICommand _windowsExplorerCommand;
        private bool _xyVisible = false;
        private bool _zIsEnabled;
        private bool _zIsLive;
        double _zoomLevel = 1;
        private bool _zStreamIsEnabled;
        private bool _zStreamIsLive;
        private int _zStreamMode;
        private int _zStreamValue3D;
        private ICommand _zStreamValueMinusCommand;
        private ICommand _zStreamValuePlusCommand;
        private ICommand _zValueMinusCommand;
        private ICommand _zValuePlusCommand;
        private bool _zVisible = true;
        private double _zVolumeSpacing = 1.0;

        ICommand _LSMImageDistortionCorrectionCalibrationGalvoTiltAngleMinusCommand;
        ICommand _LSMImageDistortionCorrectionCalibrationGalvoTiltAnglePlusCommand;
        ICommand _LSMImageDistortionCorrectionCalibrationXAngleMaxMinusCommand;
        ICommand _LSMImageDistortionCorrectionCalibrationXAngleMaxPlusCommand;
        ICommand _LSMImageDistortionCorrectionCalibrationYAngleMaxMinusCommand;
        ICommand _LSMImageDistortionCorrectionCalibrationYAngleMaxPlusCommand;
        Visibility _LSMImageDistortionCorrectionCalibrationVisibility = Visibility.Visible;

        private bool rxFlag = true;
        private bool rxFlagZ = true;
        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the ExpSetupViewModel class
        /// </summary>
        /// <param name="ExpSetup">Wrapped ExpSetup object</param>
        public ImageReviewViewModel(IEventAggregator eventAggregator, IRegionManager regionManager, IUnityContainer container, ImageReview imageReview)
        {
            this._eventAggregator = eventAggregator;
            this._regionManager = regionManager;
            this._container = container;

            if (imageReview != null)
            {
                this._imageReview = imageReview;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " ExpSetup is null. Creating a new ImageReview object.");

                imageReview = new ImageReview();

                if (imageReview == null)
                {
                    ResourceManager rm = new ResourceManager("ImageReview.Properties.Resources", Assembly.GetExecutingAssembly());
                    ThorLog.Instance.TraceEvent(TraceEventType.Critical, 1, this.GetType().Name + " " + rm.GetString("CreateImageReviewViewModelFailed"));
                    throw new NullReferenceException("ExpSetup");
                }

                this._imageReview = imageReview;
            }

            _zIsLive = false;
            _zStreamIsLive = false;
            _spIsLive = false;
            _tIsLive = false;
            _zIsEnabled = true;
            _zStreamIsEnabled = true;
            _tIsEnabled = true;
            _spIsEnabled = true;
            _outputDirectory = ReadLast3dOutputPath();
            _outputExperiment = "Untitled001";
            _primaryChannelFileNames = new List<ImageFileNameClass>();
            _currentMovieParameter = CurrentMovieParameterEnum.T;

            _roiStatsChartActive = false;
            _roiStatsTableActive = false;
            _lineProfileActive = false;

            //Set boolean in OverlayManager if instance derived from Experiment Review window
            if (ExperimentReviewOpened == true)
            {
                OverlayManagerClass.Instance.ReviewWindowOpened = true;
            }

            OverlayManagerClass.Instance.InitOverlayManagerClass(ExperimentData.ImageInfo.pixelX, ExperimentData.ImageInfo.pixelY, ExperimentData.PixelSizeUM, false);
            EnableHandlers();

            VirtualZStack = true;

            _imageReview.LineProfileChanged += _imageReview_LineProfileChanged;
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Enumerations

        public enum CurrentMovieParameterEnum
        {
            T, Sp, Z, ZStream
        }

        public enum ROIStatsOrigin
        {
            CALCULATE, LOAD
        }

        #endregion Enumerations

        #region Delegates

        public delegate void UpdateProgressDelegate(int percentage);

        #endregion Delegates

        #region Events

        //notify ImageView When a previous analysis was loaded
        public event Action AnalysisLoaded;

        //notify ImageView to enable/disable orthogonal view
        public event Action CloseOrthogonalView;

        //notify 3D VolumeView that the color mapping has changed
        public event Action<bool> ColorMappingChanged;

        //notify ImageView When a new experiment path is available
        public event Action ExperimentPathChanged;

        //Increase the view area if the image extends out of bonds for the vertical scroll bar
        public event Action<bool> IncreaseViewArea;

        //notify 3D VolumeView that the 3D volumne should be rendered
        public event Action RenderVolume;

        #endregion Events

        #region Properties

        public double LSMImageDistortionCorrectionCalibrationGalvoTiltAngle
        {
            get => _imageReview.LSMImageDistortionCorrectionCalibrationGalvoTiltAngle;
            set
            {
                _imageReview.LSMImageDistortionCorrectionCalibrationGalvoTiltAngle = value;
                OnPropertyChanged(nameof(LSMImageDistortionCorrectionCalibrationGalvoTiltAngle));
            }
        }

        public ICommand LSMImageDistortionCorrectionCalibrationGalvoTiltAngleMinusCommand
        {
            get
            {
                if (_LSMImageDistortionCorrectionCalibrationGalvoTiltAngleMinusCommand == null)
                {
                    _LSMImageDistortionCorrectionCalibrationGalvoTiltAngleMinusCommand = new RelayCommand(() => LSMImageDistortionCorrectionCalibrationGalvoTiltAngle -= 0.1);
                }
                return _LSMImageDistortionCorrectionCalibrationGalvoTiltAngleMinusCommand;
            }
        }

        public ICommand LSMImageDistortionCorrectionCalibrationGalvoTiltAnglePlusCommand
        {
            get
            {
                if (_LSMImageDistortionCorrectionCalibrationGalvoTiltAnglePlusCommand == null)
                {
                    _LSMImageDistortionCorrectionCalibrationGalvoTiltAnglePlusCommand = new RelayCommand(() => LSMImageDistortionCorrectionCalibrationGalvoTiltAngle += 0.1);
                }
                return _LSMImageDistortionCorrectionCalibrationGalvoTiltAnglePlusCommand;
            }
        }


        public double LSMImageDistortionCorrectionCalibrationXAngleMax
        {
            get => _imageReview.LSMImageDistortionCorrectionCalibrationXAngleMax;
            set
            {
                _imageReview.LSMImageDistortionCorrectionCalibrationXAngleMax = value;
                OnPropertyChanged(nameof(LSMImageDistortionCorrectionCalibrationXAngleMax));
            }
        }

        public ICommand LSMImageDistortionCorrectionCalibrationXAngleMaxMinusCommand
        {
            get
            {
                if (_LSMImageDistortionCorrectionCalibrationXAngleMaxMinusCommand == null)
                {
                    _LSMImageDistortionCorrectionCalibrationXAngleMaxMinusCommand = new RelayCommand(() => LSMImageDistortionCorrectionCalibrationXAngleMax -= 0.1);
                }
                return _LSMImageDistortionCorrectionCalibrationXAngleMaxMinusCommand;
            }
        }

        public ICommand LSMImageDistortionCorrectionCalibrationXAngleMaxPlusCommand
        {
            get
            {
                if (_LSMImageDistortionCorrectionCalibrationXAngleMaxPlusCommand == null)
                {
                    _LSMImageDistortionCorrectionCalibrationXAngleMaxPlusCommand = new RelayCommand(() => LSMImageDistortionCorrectionCalibrationXAngleMax += 0.1);
                }
                return _LSMImageDistortionCorrectionCalibrationXAngleMaxPlusCommand;
            }
        }


        public double LSMImageDistortionCorrectionCalibrationYAngleMax
        {
            get => _imageReview.LSMImageDistortionCorrectionCalibrationYAngleMax;
            set
            {
                _imageReview.LSMImageDistortionCorrectionCalibrationYAngleMax = value;
                OnPropertyChanged(nameof(LSMImageDistortionCorrectionCalibrationYAngleMax));
            }
        }

        public ICommand LSMImageDistortionCorrectionCalibrationYAngleMaxMinusCommand
        {
            get
            {
                if (_LSMImageDistortionCorrectionCalibrationYAngleMaxMinusCommand == null)
                {
                    _LSMImageDistortionCorrectionCalibrationYAngleMaxMinusCommand = new RelayCommand(() => LSMImageDistortionCorrectionCalibrationYAngleMax -= 0.1);
                }
                return _LSMImageDistortionCorrectionCalibrationYAngleMaxMinusCommand;
            }
        }

        public ICommand LSMImageDistortionCorrectionCalibrationYAngleMaxPlusCommand
        {
            get
            {
                if (_LSMImageDistortionCorrectionCalibrationYAngleMaxPlusCommand == null)
                {
                    _LSMImageDistortionCorrectionCalibrationYAngleMaxPlusCommand = new RelayCommand(() => LSMImageDistortionCorrectionCalibrationYAngleMax += 0.1);
                }
                return _LSMImageDistortionCorrectionCalibrationYAngleMaxPlusCommand;
            }
        }

        public int LSMImageDistortionCorrectionEnable
        {
            get => _imageReview.LSMImageDistortionCorrectionEnable;
            set
            {
                _imageReview.LSMImageDistortionCorrectionEnable = value;
                OnPropertyChanged(nameof(LSMImageDistortionCorrectionEnable));
            }
        }

        public Visibility LSMImageDistortionCorrectionVisibility
        {
            get => _LSMImageDistortionCorrectionCalibrationVisibility;
            set
            {
                _LSMImageDistortionCorrectionCalibrationVisibility = value;
                OnPropertyChanged(nameof(LSMImageDistortionCorrectionVisibility));
            }
        }

        public Visibility AdvancedImageControlPanelVisibility
        {
            get
            {
                //Should be updated when more controls get added to the expander. Visibility here is based on visibility of aspect ratio option
                return Visibility.Visible;
            }
        }

        public bool AllowImageUpdate
        {
            get
            {
                return _allowImageUpdate;
            }
            set
            {
                _allowImageUpdate = value;
            }
        }

        public XmlDocument ApplicationDoc
        {
            get
            {
                return this._imageReview.ApplicationDoc;
            }
            set
            {
                this._imageReview.ApplicationDoc = value;
            }
        }

        public string ApplicationSettingPath
        {
            get { return _imageReview.ApplicationSettingPath; }
        }

        public bool BitMapUpdate
        {
            get
            {
                UpdateBitmap(.01);
                return false;
            }
            set { UpdateBitmap(.01); }
        }

        //public WriteableBitmap Bitmap
        //{
        //    get
        //    {
        //        return _bitmap;
        //    }
        //    set
        //    {
        //        if (!_allowImageUpdate) return;
        //        UpdateBitmap(.01);
        //        //TODO:IV this should be elsewhere
        //        _imageReview.LoadLineProfileData();
        //    }
        //}
        public int BitsPerPixel
        {
            set
            {
                this._imageReview.BitsPerPixel = value;
                OnPropertyChanged("PixelBitShiftValue");
            }
        }

        public Visibility BurnInVisibility
        {
            get
            {
                return (CaptureModes.STREAMING == ExperimentData.CaptureMode && CaptureFile.FILE_BIG_TIFF == ExperimentData.ImageInfo.imageType) ? Visibility.Collapsed : Visibility.Visible;
            }
        }

        public CaptureModes CaptureMode
        {
            get
            {
                return _captureMode;
            }
            set
            {
                _captureMode = value;
                OnPropertyChanged("CaptureMode");
            }
        }

        //This is to prevent feedback loops
        public int ChangeFromIPCTValue
        {
            get
            {
                return this._imageReview.TValue;
            }
            set
            {
                if (this._imageReview.TValue == value)
                {
                    SyncROIChart();
                    OnPropertyChanged("DepthForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImageCalculable");

                    return;
                }

                if (TMax >= value && TMin <= value)
                {
                    this._imageReview.TValue = value;
                    this._tValue3D = value;

                    OnPropertyChanged("ZValue");
                    OnPropertyChanged("TValue");
                    OnPropertyChanged("DepthForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImageCalculable");
                    OnPropertyChanged("TBoxValue");

                    _ = UpdateBitmap(.01);

                    if (ViewType == Convert.ToInt32(ViewTypes.ViewType3D))
                    {
                        OnPropertyChanged("TValue3D");
                    }
                }
                //TODO:IV
                //if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && CloseOrthogonalView != null)
                //{
                //    CloseOrthogonalView();
                //}
                //_updateVirtualStack = true;

                if ((bool)MVMManager.Instance["ImageViewReviewVM", "IsOrthogonalViewChecked"])
                {
                    MVMManager.Instance["ImageViewReviewVM", "TIndexFromReview"] = value;
                }

                _ = UpdateBitmap(.01);
                //Thread.Sleep(5);
                
                SyncROIChart();
            }
        }

        public int ChangeFromIPCZValue
        {
            get
            {
                return this._imageReview.ZValue;
            }
            set
            {
                if (this._imageReview.ZValue == value)
                {
                    SyncROIChart();
                    OnPropertyChanged("DepthForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImage");
                    OnPropertyChanged("SpectralForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImageCalculable");
                    return;
                }

                if (ZMax >= value && ZMin <= value)
                {
                    this._imageReview.ZValue = value;
                    OnPropertyChanged("ZValue");
                    OnPropertyChanged("DepthForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImage");
                    OnPropertyChanged("SpectralForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImageCalculable");
                    OnPropertyChanged("ZBoxValue");

                    UpdateBitmap(.01);
                }

                //TODO:IV
                //if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE)
                //{
                //    ZChanged();
                //}

                UpdateBitmap(.01);
                //System.Threading.Thread.Sleep(5);
                

                SyncROIChart();
            }
        }

        public byte ChannelEnabled
        {
            get { return _imageReview.ChannelEnabled; }
        }

        public int CompletedImageCount
        {
            get
            {
                return _completedImageCount;
            }
            set
            {
                _completedImageCount = value;

                OnPropertyChanged("CompletedImageCount");
            }
        }

        public CurrentMovieParameterEnum currentMovieParameter
        {
            get
            {
                return _currentMovieParameter;
            }
            set
            {
                _currentMovieParameter = value;
            }
        }

        public double DepthForCurrentPreviewImage
        {
            get
            {
                if (IsRemoteFocus && ZValue > 0)
                {
                    return _remoteFocusPositions[ExperimentData.PlaneSequence[ZValue - 1] - 1];
                }
                return ExperimentData.ZStepSizeUM * (ZValue - 1);
            }
        }

        public Visibility DisplayAspectRatioOptionVisibility
        {
            get
            {
                return PixelAspectRatioYScale > 1 ? Visibility.Visible : Visibility.Collapsed;
            }
            set
            {
                OnPropertyChanged("DisplayAspectRatioOptionVisibility");
            }
        }

        private bool _displayAspectRatio = false;
        public bool DisplayAspectRatio
        {
            get => _displayAspectRatio;
            set
            {
                _displayAspectRatio = value;
                MVMManager.Instance["ImageViewReviewVM", "DisplayPixelAspectRatio"] = value;
            }
        }

        public ICommand DisplayROIStatsOptionsCommand
        {
            get
            {
                if (null == _displayROIStatsOptionsCommand)
                {
                    _displayROIStatsOptionsCommand = new RelayCommand(() => DisplayROIStatsOptions());
                }

                return _displayROIStatsOptionsCommand;
            }
        }


        public XmlDocument ExperimentDoc
        {
            get
            {
                return this._imageReview.ExperimentDoc;
            }
            set
            {
                this._imageReview.ExperimentDoc = value;
                ReloadSettingsByExpModality();
                MVMManager.Instance["ImageViewReviewVM", "IsOrthogonalViewChecked"] = false;
            }
        }

        public string ExperimentFolderPath
        {
            get { return _imageReview.ExperimentFolderPath; }
            set
            {
                _imageReview.ExperimentFolderPath = value;
                ROIsDirectory = value;
                OnPropertyChanged("ExperimentFolderPath");
                if (ExperimentFolderPath != string.Empty)
                {
                    ROIControlEnabled = true;
                    OnPropertyChanged("ROIControlEnabled");
                }
            }
        }

        public double ExperimentFramerate
        {
            get
            {
                double frameRate = 0;
                if (ExperimentDoc != null)
                {
                    string str = string.Empty;
                    XmlNodeList ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/LSM");
                    if (ndList.Count > 0)
                    {
                        XmlManager.GetAttribute(ndList[0], ExperimentDoc, "frameRate", ref str);
                    }

                    Double.TryParse(str, out frameRate);
                }
                return frameRate;
            }
        }


        public string ExperimentModality
        {
            get
            {
                return this._imageReview.ExperimentModality;
            }
        }

        //Boolean to keep track of whether an Experiment Review Window is open
        //Set in ImageReviewModel, used for OverlayManager
        public bool ExperimentReviewOpened
        {
            get
            {
                _experimentReviewOpened = ImageReview.ExperimentReviewOpened;
                return _experimentReviewOpened;
            }
        }

        public string ExperimentXMLPath
        {
            get { return _imageReview.ExperimentXMLPath; }
            set
            {
                _imageReview.ExperimentXMLPath = value;
                OnPropertyChanged("ExperimentXMLPath");
            }
        }

        public int FieldSizeX
        {
            get
            {
                return _fieldSizeX;
            }
            set
            {
                _fieldSizeX = value;
                OnPropertyChanged("FieldSizeX");
            }
        }

        public int FieldSizeY
        {
            get
            {
                return _fieldSizeY;
            }
            set
            {
                _fieldSizeY = value;
                OnPropertyChanged("FieldSizeY");
            }
        }

        public ImageReview.FormatMode FileFormatMode
        {
            get
            {
                return this._imageReview.FileFormatMode;
            }
            set
            {
                this._imageReview.FileFormatMode = value;
            }
        }

        public XmlDocument HardwareDoc
        {
            get
            {
                return this._imageReview.HardwareDoc;
            }
            set
            {
                this._imageReview.HardwareDoc = value;
            }
        }

        public string HardwareSettingPath
        {
            get { return _imageReview.HardwareSettingPath; }
        }

        public int ImageBlkIndex
        {
            get { return _imageBlkIndex; }
            set { _imageBlkIndex = value; }
        }

        public int ImageColorChannels
        {
            get
            {
                return _imageReview.ImageColorChannels;
            }

            set
            {
                _imageReview.ImageColorChannels = value;
            }
        }

        public ImgInfo ImageInfo
        {
            get { return this._imageReview.ImageInfo; }
            set { this._imageReview.ImageInfo = value; }
        }

        public string ImageNameFormat
        {
            get
            {
                return this._imageReview.ImageNameFormat;
            }
        }

        public string ImagePathandName
        {
            get
            {
                return _imagePathandName;
            }
            set
            {
                _imagePathandName = value;
                //TODO:IV make sure this works with new iv
                //OnPropertyChanged("Bitmap");
                UpdateBitmap(.01);
                OnPropertyChanged("ImagePathandName");
            }
        }

        /// <summary>
        /// Gets the wrapped ImageReview object
        /// </summary>
        public ImageReview ImageReview
        {
            get
            {
                return this._imageReview;
            }
        }

        public ImageReview ImageReviewObject
        {
            get
            {
                return _imageReview;
            }
        }

        public bool IndexControlsEnabled
        {
            get
            {
                return _indexControlsEnabled;
            }
            set
            {
                _indexControlsEnabled = value;
                OnPropertyChanged("IndexControlsEnabled");
            }
        }

        public Visibility[] IsChannelVisible
        {
            get
            {
                return _isChannelVisible;
            }
            set
            {
                _isChannelVisible = value;
                OnPropertyChanged("IsChannelVisible");
            }
        }

        public bool IsExperimentLoading
        {
            set
            {
                _isExperimentLoading = value;
            }
        }

        public bool IsMatlabInstalled
        {
            get
            {
                if (File.Exists(".\\Modules\\MatlabEngine.dll") && File.Exists(".\\NativeMatlabEngine.dll"))
                {
                    return MatlabEngine.Instance.IsMatlabValid();
                }
                return false;
            }
        }

        public Visibility IsMovieAvailable
        {
            get
            {
                return IsSpVisible.Equals(Visibility.Visible) || IsZStreamVisible.Equals(Visibility.Visible) || IsTVisible.Equals(Visibility.Visible) || ZVisible.Equals(Visibility.Visible) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public bool IsRawExperiment
        {
            get
            {
                return _isRawExperiment;
            }
            set
            {
                if (_isRawExperiment != value)
                    _isRawExperiment = value;
                OnPropertyChanged("IsRawExperiment");
            }
        }

        public bool IsRemoteFocus
        {
            get
            {
                return _isRemoteFocus;
            }
            set
            {
                _isRemoteFocus = value;
                OnPropertyChanged("IsRemoteFocus");
                OnPropertyChanged("PlanePositionVisibility");
                OnPropertyChanged("RemoteFocusPlaneNumber");
                OnPropertyChanged("DepthForCurrentPreviewImage");
            }
        }

        public bool IsScaleOnOffChecked
        {
            get
            {
                return _isScaleOnOffChecked;
            }
            set
            {
                _isScaleOnOffChecked = value;
                OnPropertyChanged("IsScaleOnOffChecked");
            }
        }

        public bool IsSingleChannel
        {
            get
            {
                return _imageReview.IsSingleChannel;
            }
        }

        public Visibility IsSpVisible
        {
            get
            {
                return (CaptureModes.HYPERSPECTRAL != CaptureMode) ? Visibility.Collapsed : Visibility.Visible;
            }
        }


        //Save the current height of the image display space
        public double IVHeight
        {
            get
            {
                return _ivHeight;
            }
            set
            {
                _ivHeight = value;
                IVScrollBarHeight = _ivHeight;
            }
        }

        public double IVScrollBarHeight
        {
            get
            {
                return _iVScrollBarHeight;
            }
            set
            {
                //Compare the height of the display space with the height of the image
                // if the image height is bigger, make the scrollbar visible and set it's height
                // Leave a small gap of 10 pixels below the image to see the end of it easier
                _iVScrollBarHeight = ((value + 10) > (IVHeight + 11)) ? (value + 10) : IVHeight;
                IncreaseViewArea(true);
                OnPropertyChanged("IVScrollBarHeight");
                OnPropertyChanged("IVScrollbarVisibility");
            }
        }

        //TODO
        //Update visibility to use bound property
        public ScrollBarVisibility IVScrollbarVisibility
        {
            get
            {
                if (IVScrollBarHeight > IVHeight)
                {
                    return ScrollBarVisibility.Visible;
                }
                return ScrollBarVisibility.Hidden;
            }
            set
            {
                _ivScrollbarVisibility = value;
            }
        }

        public LineProfile LineProfile
        {
            get { return _lineProfile; }
            set { _lineProfile = value; }
        }

        public bool LineProfileActive
        {
            get
            {
                return _lineProfileActive;
            }
            set
            {
                _lineProfileActive = value;
                if (true == _lineProfileActive)
                {
                    LoadLineProfileData(); //Sets the data in stats manager so the line profile has an image reference when drawn.
                    if (null != _lineProfile)
                    {
                        _lineProfile.Show();
                    }
                }
                else
                {
                    if (null != _lineProfile)
                    {
                        _lineProfile.Hide();
                    }
                }
            }
        }

        public Color[] LineProfileColorAssignment
        {
            set
            {
                if (null == _lineProfile)
                {
                    return;
                }
                _lineProfile.ColorAssigment = value;
            }
        }

        public bool LiveSnapshotStatus
        {
            get
            {
                return _liveSnapshotStatus;
            }
            set
            {
                _liveSnapshotStatus = value;
            }
        }

        public int LSMChannel
        {
            get
            {
                return _imageReview.LSMChannel;
            }
        }

        public int LSMNumberOfPlanes
        {
            get => ExperimentData.NumberOfPlanes;
        }

        public bool MaskReady
        {
            get
            {
                return _maskReady;
            }
            set
            {
                _maskReady = value;
                OnPropertyChanged("MaskReady");
            }
        }

        public int MaxChannels
        {
            get
            {
                return _imageReview.MaxChannels;
            }
        }

        public int MEnd
        {
            get
            {
                return _mEnd;
            }
            set
            {
                _mEnd = value;
                OnPropertyChanged("MEnd");
            }
        }

        public int MEndValue
        {
            get
            {
                return _mEndValue;
            }
            set
            {
                switch (_currentMovieParameter)
                {
                    case CurrentMovieParameterEnum.T:
                        if (TMax >= value && TMin <= value)
                        {
                            _mEndValue = value;
                            OnPropertyChanged("MEndValue");
                        }
                        break;
                    case CurrentMovieParameterEnum.Sp:
                        if (SpMax >= value && SpMin <= value)
                        {
                            _mEndValue = value;
                            OnPropertyChanged("MEndValue");
                        }
                        break;
                    case CurrentMovieParameterEnum.Z:
                        if (ZMax >= value && ZMin <= value)
                        {
                            _mEndValue = value;
                            OnPropertyChanged("MEndValue");
                        }
                        break;
                    case CurrentMovieParameterEnum.ZStream:
                        if (ZStreamMax >= value && ZStreamMin <= value)
                        {
                            _mEndValue = value;
                            OnPropertyChanged("MEndValue");
                        }
                        break;
                }
            }
        }

        public ICommand MEndValueMinusCommand
        {
            get
            {
                if (this._mEndValueMinusCommand == null)
                    this._mEndValueMinusCommand = new RelayCommand(() => MEndValueMinus());

                return this._mEndValueMinusCommand;
            }
        }

        public ICommand MEndValuePlusCommand
        {
            get
            {
                if (this._mEndValuePlusCommand == null)
                    this._mEndValuePlusCommand = new RelayCommand(() => MEndValuePlus());

                return this._mEndValuePlusCommand;
            }
        }

        public String MovieFilePath
        {
            get
            {
                return _movieFilePath;
            }
            set
            {
                _movieFilePath = value;
                OnPropertyChanged("MovieFilePath");
            }
        }

        public ObservableCollection<MesoScan.Params.ScanArea> mROIList
        {
            get => ExperimentData.mROIs;
            set
            {
                var x = value;
            }
        }

        public bool mROIShowOverlays
        {
            get
            {
                return _mROIShowOverlays;
            }
            set
            {
                if (_mROIShowOverlays != value)
                {
                    _mROIShowOverlays = value;
                    OnPropertyChanged("mROIShowOverlays");

                    if (value)
                    {
                        mROISpatialDisplaybleEnable = true;
                    }

                    MVMManager.Instance["ImageViewReviewVM", "ROIToolVisible"] = (_mROIShowOverlays) ?
                        new bool[14] { false, false, false, false, false, false, false, false, false, false, false, false, false, false } :
                        new bool[14] { true, true, true, true, true, true, true, true, true, true, true, true, true, true };

                    if (_mROIShowOverlays)
                    {
                        OverlayManagerClass.Instance.ValidateROIs();

                        var ROIs = OverlayManagerClass.Instance.GetModeROIs(Mode.MICRO_SCANAREA);
                        if (ROIs.Count > SelectedmROIIndex && SelectedmROIIndex >= 0)
                        {
                            MVMManager.Instance["ImageViewReviewVM", "mROIPriorityIndex"] = SelectedmROIIndex;
                            OverlayManagerClass.Instance.SelectSingleROI(ROIs[SelectedmROIIndex]);
                        }
                        else
                        {
                            OverlayManagerClass.Instance.DeselectAllROIs();
                        }
                    }

                    OverlayManagerClass.Instance.InitSelectROI();
                    OverlayManagerClass.Instance.DisplayModeROI(_mROIShowOverlays ? new ThorSharedTypes.Mode[1] { ThorSharedTypes.Mode.STATSONLY } : new ThorSharedTypes.Mode[1] { ThorSharedTypes.Mode.MICRO_SCANAREA }, false);
                    OverlayManagerClass.Instance.DisplayModeROI(_mROIShowOverlays ? new ThorSharedTypes.Mode[1] { ThorSharedTypes.Mode.MICRO_SCANAREA } : new ThorSharedTypes.Mode[1] { ThorSharedTypes.Mode.STATSONLY }, true);
                    OverlayManagerClass.Instance.CurrentMode = _mROIShowOverlays ? ThorSharedTypes.Mode.MICRO_SCANAREA : ThorSharedTypes.Mode.STATSONLY;

                    if (_mROIShowOverlays)
                    {
                        MVMManager.Instance["ImageViewReviewVM", "mROIPriorityIndex"] = SelectedmROIIndex;
                        var ROIs = OverlayManagerClass.Instance.GetModeROIs(Mode.MICRO_SCANAREA);
                        if (ROIs.Count > SelectedmROIIndex && SelectedmROIIndex >= 0)
                        {
                            OverlayManagerClass.Instance.SelectSingleROI(ROIs[SelectedmROIIndex]);
                        }
                        else
                        {
                            OverlayManagerClass.Instance.DeselectAllROIs();
                        }
                    }
                }
            }
        }

        public bool mROISpatialDisplaybleEnable
        {
            get => _mROISpatialDisplaybleEnable;
            set
            {
                _mROISpatialDisplaybleEnable = value;
                if (!value)
                {
                    mROIShowOverlays = false;
                }
                OnPropertyChanged("mROISpatialDisplaybleEnable");
                MVMManager.Instance["ImageViewReviewVM", "mROISpatialDisplaybleEnable"] = value;
            }
        }

        public double mROIStripePhysicalFieldSizeUM
        {
            get
            {
                if (ExperimentData.IsmROICapture && mROIList?.Count > 0 && mROIList[0] != null)
                {
                    return Math.Round(mROIList[0].PhysicalSizeXUM / mROIList[0].Stripes, 2);
                }

                return 0;
            }
        }

        public int mROIStripePixels
        {
            get
            {
                if (ExperimentData.IsmROICapture && mROIList?.Count > 0 && mROIList[0] != null)
                {
                    return mROIList[0].SizeXPixels / mROIList[0].Stripes;
                }

                return 0;
            }
        }

        public int MStart
        {
            get
            {
                return _mStart;
            }
            set
            {
                _mStart = value;
                OnPropertyChanged("MStart");
            }
        }

        public int MStartValue
        {
            get
            {
                return _mStartValue;
            }
            set
            {
                switch (_currentMovieParameter)
                {
                    case CurrentMovieParameterEnum.T:
                        if (TMax >= value && TMin <= value)
                        {
                            _mStartValue = value;
                            OnPropertyChanged("MStartValue");
                        }
                        break;
                    case CurrentMovieParameterEnum.Sp:
                        if (SpMax >= value && SpMin <= value)
                        {
                            _mStartValue = value;
                            OnPropertyChanged("MStartValue");
                        }
                        break;
                    case CurrentMovieParameterEnum.Z:
                        if (ZMax >= value && ZMin <= value)
                        {
                            _mStartValue = value;
                            OnPropertyChanged("MStartValue");
                        }
                        break;
                    case CurrentMovieParameterEnum.ZStream:
                        if (ZStreamMax >= value && ZStreamMin <= value)
                        {
                            _mStartValue = value;
                            OnPropertyChanged("MStartValue");
                        }
                        break;
                }
            }
        }

        public ICommand MStartValueMinusCommand
        {
            get
            {
                if (this._mStartValueMinusCommand == null)
                    this._mStartValueMinusCommand = new RelayCommand(() => MStartValueMinus());

                return this._mStartValueMinusCommand;
            }
        }

        public ICommand MStartValuePlusCommand
        {
            get
            {
                if (this._mStartValuePlusCommand == null)
                    this._mStartValuePlusCommand = new RelayCommand(() => MStartValuePlus());

                return this._mStartValuePlusCommand;
            }
        }

        public List<MultiROIStatsUC> MultiROIStatsWindows
        {
            get { return _multiROIStatsWindows; }
            set { _multiROIStatsWindows = value; }
        }

        public string OutputDirectory
        {
            get { return _outputDirectory; }
            set
            {
                _outputDirectory = value;
                OnPropertyChanged("OutputDirectory");
            }
        }

        public string OutputExperiment
        {
            get { return _outputExperiment; }
            set
            {
                _outputExperiment = value;
                OnPropertyChanged("OutputExperiment");
            }
        }

        public bool PanelsEnable
        {
            get
            { return _panelsEnable; }
            set
            {
                _panelsEnable = value;
                OnPropertyChanged("PanelsEnable");
            }
        }

        public int PixelBitShiftValue
        {
            get
            {
                return _imageReview.PixelBitShiftValue;
            }
        }

        public PixelSizeUM PixelSizeUM
        {
            get => ExperimentData.PixelSizeUM;
        }

        double _pixelAspectRatioYScale = 1;
        public double PixelAspectRatioYScale
        {
            get => _pixelAspectRatioYScale;
            set
            {
                _pixelAspectRatioYScale = value;
                OnPropertyChanged("PixelAspectRatioYScale");
                OnPropertyChanged("DisplayAspectRatioOptionVisibility");
                OnPropertyChanged("AdvancedImageControlPanelVisibility");
            }
        }

        public Visibility PlanePositionVisibility
        {
            get
            {
                return ExperimentData.IsRemoteFocus ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public List<ImageFileNameClass> PrimaryChannelFileNames
        {
            get { return _primaryChannelFileNames; }
            set
            {
                _primaryChannelFileNames = value;
                OnPropertyChanged("PrimaryChannelFileNames");
            }
        }

        public int PrimaryChannelIndex
        {
            get { return _primaryChannelIndex; }
            set
            {
                _primaryChannelIndex = value;
                OnPropertyChanged("PrimaryChannelIndex");
            }
        }

        public int ProgressPercentage
        {
            get
            { return _progressPercentage; }
            set
            { _progressPercentage = value; }
        }

        public int RemoteFocusPlaneNumber
        {
            get
            {
                if (IsRemoteFocus && ZValue > 0)
                {
                    return ExperimentData.PlaneSequence[ZValue - 1];
                }
                return 0;
            }
        }

        public ICommand ROICalculation
        {
            get
            {
                if (this._roiCalculationCommand == null)
                    this._roiCalculationCommand = new RelayCommand(() => btRoiCal_Click());

                return this._roiCalculationCommand;
            }
        }

        public bool ROIControlEnabled
        {
            get { return _roiControlEnabled; }
            set
            {
                if (ExperimentFolderPath == null)
                {
                    _roiControlEnabled = false;
                }
                else
                {
                    _roiControlEnabled = (ExperimentFolderPath == string.Empty) ? false : value;
                }
                OnPropertyChanged("ROIControlEnabled");
            }
        }

        public ICommand ROILoad
        {
            get
            {
                if (this._roiLoadCommand == null)
                    this._roiLoadCommand = new RelayCommand(() => btRoiLoad_Click());

                return this._roiLoadCommand;
            }
        }

        public string ROIsDirectory
        {
            get;
            set;
        }

        public bool ROIStatsChartActive
        {
            get
            {
                return _roiStatsChartActive;
            }
            set
            {
                if (0 < _roiStatsCharts.Count)
                {
                    if (value)
                    {
                        foreach (ROIStatsChartWin win in _roiStatsCharts)
                        {
                            if (null != win)
                            {
                                win.Show();
                            }
                        }
                    }
                    else
                    {
                        foreach (ROIStatsChartWin win in _roiStatsCharts)
                        {
                            if (null != win)
                            {
                                win.Hide();
                            }
                        }
                    }
                }
                _roiStatsChartActive = value;
            }
        }

        public List<ROIStatsChartWin> ROIStatsChartW
        {
            get { return _roiStatsCharts; }
            set { _roiStatsCharts = value; }
        }

        public bool ROIStatsTableActive
        {
            get
            {
                return _roiStatsTableActive;
            }
            set
            {
                if (0 < _multiROIStatsWindows.Count)
                {
                    if (value)
                    {
                        foreach (MultiROIStatsUC win in _multiROIStatsWindows)
                        {
                            if (null != win)
                            {
                                win.Show();
                            }
                        }
                    }
                    else
                    {
                        foreach (MultiROIStatsUC win in _multiROIStatsWindows)
                        {
                            if (null != win)
                            {
                                win.Hide();
                            }

                        }
                    }
                }
                _roiStatsTableActive = value;
            }
        }

        public Thickness ROItoolbarMargin
        {
            get
            {
                return _roiToolbarMargin;
            }
            set
            {
                _roiToolbarMargin = value;
                OnPropertyChanged("ROItoolbarMargin");
            }
        }

        public int SampleSiteIndex
        {
            get
            {
                return this._imageReview.SampleSiteIndex;
            }
            set
            {
                this._imageReview.SampleSiteIndex = value;
                if (CloseOrthogonalView != null)
                {
                    CloseOrthogonalView();
                }
                //TODO:IV
                //_updateVirtualStack = true;
            }
        }

        public int ScanAreaID
        {
            get
            {
                return 0;
            }
            set
            {
                //if (null != ImageInfo.scanAreaIDList && 0 < ImageInfo.scanAreaIDList.Count && 0 >= value && 0 <= value)
                //{
                //    _scanAreaIndex = ImageInfo.scanAreaIDList.FindIndex(c => c.RegionID == (value - ImageInfo.scanAreaIDList.Where(n => n.RegionID <= value).Min(n => value - n.RegionID)));
                //    OnPropertyChanged("ScanAreaID");

                //    UpdateBitmap(.01);
                //}
            }
        }

        public int SelectedmROIIndex
        {
            get
            {
                return SelectedScanArea - 1;
            }
            set
            {
                SelectedScanArea = value + 1;

            }
        }

        public int SelectedScanArea
        {
            get
            {
                return _selectedScanArea;
            }
            set
            {
                _selectedScanArea = value;
                OnPropertyChanged("SelectedScanArea");
                OnPropertyChanged("SelectedmROIIndex");

                MVMManager.Instance["ImageViewReviewVM", "mROIPriorityIndex"] = value - 1;

                if (SelectedmROIIndex >= 0)
                {
                    if (_mROIShowOverlays)
                    {
                        var ROIs = OverlayManagerClass.Instance.GetModeROIs(Mode.MICRO_SCANAREA);
                        if (ROIs?.Count > SelectedmROIIndex)
                        {

                            ROIs = OverlayManagerClass.Instance.GetModeROIs(Mode.MICRO_SCANAREA);
                            OverlayManagerClass.Instance.SelectSingleROI(ROIs[SelectedmROIIndex]);
                        }
                        else
                        {
                            OverlayManagerClass.Instance.DeselectAllROIs();
                        }
                    }
                }
                else if (_mROIShowOverlays)
                {
                    OverlayManagerClass.Instance.DeselectAllROIs();
                }
            }
        }
        public bool RXFlag
        {
            get
            {
                return rxFlag;
            }
            set
            {
                rxFlag = value;
            }
        }

        public bool RXFlagZ
        {
            get
            {
                return rxFlagZ;
            }
            set
            {
                rxFlagZ = value;
            }
        }

        public bool SequentialModeEnabled
        {
            get
            {
                return _imageReview.SequentialModeEnabled;
            }
            set
            {
                _imageReview.SequentialModeEnabled = value;
            }
        }

        //public int ScanAreaIDMax
        //{
        //    get
        //    {
        //        if (null == ImageInfo.scanAreaIDList)
        //            return 0;
        //        return (_scanAreaIndexMax < ImageInfo.scanAreaIDList.Count) ? ImageInfo.scanAreaIDList[_scanAreaIndexMax].RegionID : 0;
        //    }
        //    set
        //    {
        //        if (null != ImageInfo.scanAreaIDList)
        //        {
        //            if (0 < ImageInfo.scanAreaIDList.Count)
        //            {
        //                _scanAreaIndexMax = ImageInfo.scanAreaIDList.FindIndex(c => c.RegionID == (value - ImageInfo.scanAreaIDList.Where(n => n.RegionID <= value).Min(n => value - n.RegionID)));
        //                OnPropertyChanged("ScanAreaIDMax");
        //            }
        //        }
        //    }
        //}
        //public int ScanAreaIDMin
        //{
        //    get
        //    {
        //        if (null == ImageInfo.scanAreaIDList)
        //            return 0;
        //        return (_scanAreaIndexMin < ImageInfo.scanAreaIDList.Count) ? ImageInfo.scanAreaIDList[_scanAreaIndexMin].RegionID : 0;
        //    }
        //    set
        //    {
        //        if (null != ImageInfo.scanAreaIDList)
        //        {
        //            if (0 < ImageInfo.scanAreaIDList.Count)
        //            {
        //                _scanAreaIndexMin = ImageInfo.scanAreaIDList.FindIndex(c => c.RegionID == (value - ImageInfo.scanAreaIDList.Where(n => n.RegionID <= value).Min(n => value - n.RegionID)));
        //                OnPropertyChanged("ScanAreaIDMin");
        //            }
        //        }
        //    }
        //}
        //public ICommand ScanAreaIDMinusCommand
        //{
        //    get
        //    {
        //        if (this._scanAreaIDMinusCommand == null)
        //            this._scanAreaIDMinusCommand = new RelayCommand(() => ScanAreaIDMinus());
        //        return this._scanAreaIDMinusCommand;
        //    }
        //}
        //public ICommand ScanAreaIDPlusCommand
        //{
        //    get
        //    {
        //        if (this._scanAreaIDPlusCommand == null)
        //            this._scanAreaIDPlusCommand = new RelayCommand(() => ScanAreaIDPlus());
        //        return this._scanAreaIDPlusCommand;
        //    }
        //}
        //public string ScanAreaImagePathPlay
        //{
        //    get
        //    {
        //        if (_scanAreaIsLive)
        //        {
        //            return @"/ImageReview;component/Icons/Stop.png";
        //        }
        //        else
        //        {
        //            return @"/ImageReview;component/Icons/Play.png";
        //        }
        //    }
        //}
        //public bool ScanAreaIsEnabled
        //{
        //    get
        //    {
        //        return _scanAreaIsEnabled;
        //    }
        //    set
        //    {
        //        _scanAreaIsEnabled = value;
        //        OnPropertyChanged("ScanAreaIsEnabled");
        //    }
        //}
        //public bool ScanAreaIsLive
        //{
        //    get
        //    {
        //        return _scanAreaIsLive;
        //    }
        //    set
        //    {
        //        _scanAreaIsLive = value;
        //        ROIControlEnabled = (true == value) ? false : true;
        //        OnPropertyChanged("ScanAreaImagePathPlay");
        //        OnPropertyChanged("ROIControlEnabled");
        //    }
        //}
        //public bool ScanAreaVisible
        //{
        //    get { return _scanAreaVisible; }
        //    set
        //    {
        //        _scanAreaVisible = value;
        //        OnPropertyChanged("ScanAreaVisible");
        //    }
        //}
        public int SpectralForCurrentPreviewImage
        {
            get
            {
                if (ExperimentData.SpStart > ExperimentData.SpEnd)
                {
                    return ExperimentData.SpStart - (SpValue - 1) * ExperimentData.SpPace;
                }
                return (SpValue - 1) * ExperimentData.SpPace + ExperimentData.SpStart;
            }
        }

        public string SpImagePathPlay
        {
            get
            {
                if (_spIsLive)
                {
                    return @"/ImageReview;component/Icons/Stop.png";
                }
                else
                {
                    return @"/ImageReview;component/Icons/Play.png";
                }
            }
        }

        public bool SpIsEnabled
        {
            get
            {
                return _spIsEnabled;
            }
            set
            {
                _spIsEnabled = value;
                OnPropertyChanged("SpIsEnabled");
            }
        }

        public bool SpIsLive
        {
            get
            {
                return _spIsLive;
            }
            set
            {
                _spIsLive = value;
                ROIControlEnabled = (true == value) ? false : true;
                OnPropertyChanged("SpImagePathPlay");
                OnPropertyChanged("ROIControlEnabled");
            }
        }

        public int SpMax
        {
            get
            {
                return _sliderSpMax;
            }
            set
            {
                _sliderSpMax = value;
                OnPropertyChanged("SpMax");
                OnPropertyChanged("IsSpVisible");
            }
        }

        public int SpMin
        {
            get
            {
                return _sliderSpMin;
            }
            set
            {
                _sliderSpMin = value;
                OnPropertyChanged("SpMin");
                OnPropertyChanged("IsSpVisible");
            }
        }

        public int SpValue
        {
            get { return this._imageReview.SpValue; }
            set
            {
                if (this._imageReview.SpValue == value)
                {
                    SyncROIChart();
                    OnPropertyChanged("SpectralForCurrentPreviewImage");
                    OnPropertyChanged("DepthForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImageCalculable");
                    return;
                }

                if (SpMax >= value && SpMin <= value)
                {
                    this._imageReview.SpValue = value;

                    OnPropertyChanged("SpValue");
                    OnPropertyChanged("SpectralForCurrentPreviewImage");
                    OnPropertyChanged("DepthForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImageCalculable");
                    UpdateBitmap(.01);
                }
                //TODO:IV
                //if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && CloseOrthogonalView != null)
                //{
                //    CloseOrthogonalView();
                //}
                //_updateVirtualStack = true;

                SyncROIChart();
            }
        }

        public ICommand SpValueMinusCommand
        {
            get
            {
                if (this._spValueMinusCommand == null)
                    this._spValueMinusCommand = new RelayCommand(() => SpValueMinus());

                return this._spValueMinusCommand;
            }
        }

        public ICommand SpValuePlusCommand
        {
            get
            {
                if (this._spValuePlusCommand == null)
                    this._spValuePlusCommand = new RelayCommand(() => SpValuePlus());

                return this._spValuePlusCommand;
            }
        }

        public static bool StatsLoaderDone
        {
            get => _bwStatsLoaderDone;
        }

        public int SubTileIndex
        {
            get
            {
                return this._imageReview.SubTileIndex;
            }
            set
            {
                this._imageReview.SubTileIndex = value;
                if (CloseOrthogonalView != null)
                {
                    CloseOrthogonalView();
                }
                //TODO:IV
                //_updateVirtualStack = true;
                OnPropertyChanged("SubTileIndex");
            }
        }

        public string TImagePathPlay
        {
            get
            {
                if (_tIsLive)
                {
                    return @"/ImageReview;component/Icons/Stop.png";
                }
                else
                {
                    return @"/ImageReview;component/Icons/Play.png";
                }
            }
        }

        public double TimeForCurrentPreviewImage
        {
            get
            {
                int zStackFrames = ExperimentData.ImageInfo.flybackFrames + ExperimentData.ImageInfo.zSteps;
                double frameTime = 1.0 / (ExperimentFramerate) * (ExperimentData.AverageMode ? ExperimentData.ImgPerAvg : 1);
                return ((TValue - 1) * zStackFrames + (ZValue - 1)) * frameTime;
            }
        }

        public bool TimeForCurrentPreviewImageCalculable
        {
            get
            {
                return (CaptureModes.STREAMING == ExperimentData.CaptureMode) ? true : false;
            }
        }

        public bool TIsEnabled
        {
            get
            {
                return _tIsEnabled;
            }
            set
            {
                _tIsEnabled = value;
                OnPropertyChanged("TIsEnabled");
            }
        }

        public bool TIsLive
        {
            get
            {
                return _tIsLive;
            }
            set
            {
                _tIsLive = value;
                ROIControlEnabled = (true == value) ? false : true;
                OnPropertyChanged("TImagePathPlay");
                OnPropertyChanged("ROIControlEnabled");
            }
        }

        public int TMax
        {
            get
            {
                return _sliderTMax;
            }
            set
            {
                _sliderTMax = value;
                OnPropertyChanged("TMax");
                OnPropertyChanged("IsTVisible");
                OnPropertyChanged("IsMovieAvailable");

            }
        }

        public int TMin
        {
            get
            {
                return _sliderTMin;
            }
            set
            {
                _sliderTMin = value;
                OnPropertyChanged("TMin");
                OnPropertyChanged("IsTVisible");
                OnPropertyChanged("IsMovieAvailable");

            }
        }

        public int TotalImageCount
        {
            get
            {
                return _totalImageCount;
            }
            set
            {
                _totalImageCount = value;

                OnPropertyChanged("TotalImageCount");
            }
        }

        public int TotalScanAreas
        {
            get => ExperimentData.IsmROICapture ? ExperimentData.mROIs.Count : 1;
        }

        public int TBoxValue
        {
            get
            {
                return this._imageReview.TValue;
            }
            set
            {
                RXFlag = false;
                TValue = value;
                RXFlag = true;
            }
        }

        public int TValue
        {
            get
            {
                return this._imageReview.TValue;
            }
            set
            {
                if (this._imageReview.TValue == value)
                {
                    SyncROIChart();
                    OnPropertyChanged("DepthForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImageCalculable");

                    return;
                }
                
                if (TMax >= value && TMin <= value)
                {
                    this._imageReview.TValue = value;
                    this._tValue3D = value;

                    OnPropertyChanged("ZValue");
                    OnPropertyChanged("TValue");
                    OnPropertyChanged("DepthForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImageCalculable");

                    _ = UpdateBitmap(.01);
                    if(false == RXFlag && (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "RemoteConnection"])
                        MVMManager.Instance["RemoteIPCControlViewModelBase", "ThorsyncFrameSync"] = TValue.ToString() + "/" + ZValue.ToString() + "/" + ZMax.ToString();

                    if (ViewType == Convert.ToInt32(ViewTypes.ViewType3D))
                    {
                        OnPropertyChanged("TValue3D");
                    }
                }
                //TODO:IV
                //if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && CloseOrthogonalView != null)
                //{
                //    CloseOrthogonalView();
                //}
                //_updateVirtualStack = true;

                if ((bool)MVMManager.Instance["ImageViewReviewVM", "IsOrthogonalViewChecked"])
                {
                    MVMManager.Instance["ImageViewReviewVM", "TIndexFromReview"] = value;
                }
                OnPropertyChanged("TBoxValue");

                SyncROIChart();
            }
        }

        // this fires volume rendering
        public int TValue3D
        {
            get
            {
                return this._tValue3D;
            }
            set
            {
                this._tValue3D = value;
                OnPropertyChanged("TValue3D");
            }
        }

        public ICommand TValueMinusCommand
        {
            get
            {
                if (this._tValueMinusCommand == null)
                    this._tValueMinusCommand = new RelayCommand(() => TValueMinus());

                return this._tValueMinusCommand;
            }
        }

        public ICommand TValuePlusCommand
        {
            get
            {
                if (this._tValuePlusCommand == null)
                    this._tValuePlusCommand = new RelayCommand(() => TValuePlus());

                return this._tValuePlusCommand;
            }
        }

        public bool UpdateMap
        {
            get
            {
                UpdateBitmap();
                return false;
            }
            set
            {
                UpdateBitmap();
            }
        }

        public ICommand ViewMatlab
        {
            get
            {

                if (this._viewMatlabCOmmand == null)
                {

                    this._viewMatlabCOmmand = new RelayCommand(() =>
                    {
                        try
                        {
                            LaunchMatlab();
                        }
                        catch (FileNotFoundException e)
                        {
                            ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ViewMatlab: " + e.Message);
                            MessageBox.Show("Failed to launch Matlab. Please make sure the Matlab Feature was selected during the installation of ThorImageLS");
                        }
                    }
                    );
                }
                return this._viewMatlabCOmmand;

            }
        }

        public ICommand ViewOMEXML
        {
            get
            {
                if (this._viewOMEXMLCommand == null)
                    this._viewOMEXMLCommand = new RelayCommand(() => LaunchFijiOMEXML());

                return this._viewOMEXMLCommand;
            }
        }

        public ICommand ViewThorAnalysis
        {
            get
            {
                if (this._viewThorAnalysisCommand == null)
                    this._viewThorAnalysisCommand = new RelayCommand(() => LaunchThorAnalysis());

                return this._viewThorAnalysisCommand;
            }
        }

        public int ViewType
        {
            get
            {
                return (int)_viewType;
            }
            set
            {
                _viewType = value;
                OnPropertyChanged("ViewType");

                if (ViewType == Convert.ToInt32(ViewTypes.ViewType3D))
                {
                    if (SequentialModeEnabled)
                    {
                        MessageBox.Show("Sequential Mode is not supported with 3D view yet.");
                    }
                    else
                    {
                        if (CloseOrthogonalView != null)
                        {
                            CloseOrthogonalView();
                        }
                        OnPropertyChanged("TValue3D");
                        OnPropertyChanged("ZStreamValue3D");
                        if (null != RenderVolume) RenderVolume();
                    }
                }
            }
        }

        public bool VirtualZStack
        {
            get;
            set;
        }

        public Color WavelengthColor
        {
            get
            {
                return this._imageReview.WavelengthColor;
            }
            set
            {
                this._imageReview.WavelengthColor = value;
                OnPropertyChanged("WavelengthColor");
            }
        }

        public string[] WavelengthNames
        {
            get;
            set;
        }

        public int WavelengthSelectedIndex
        {
            get
            {
                return this._imageReview.WavelengthSelectedIndex;
            }
        }

        public ICommand WindowsExplorer
        {
            get
            {
                if (this._windowsExplorerCommand == null)
                    this._windowsExplorerCommand = new RelayCommand(() => LaunchWindowsExplorer());

                return this._windowsExplorerCommand;
            }
        }

        public ObservableCollection<XYPosition> XYtableData
        {
            get
            {
                return this._imageReview.XYtableData;
            }
            set
            {
                this._imageReview.XYtableData = value;
                OnPropertyChanged("XYtableData");
            }
        }

        public bool XYVisible
        {
            get { return _xyVisible; }
            set
            {
                _xyVisible = value;
                OnPropertyChanged("XYVisible");
            }
        }

        public string ZImagePathPlay
        {
            get
            {
                if (_zIsLive)
                {
                    return @"/ImageReview;component/Icons/Stop.png";
                }
                else
                {
                    return @"/ImageReview;component/Icons/Play.png";
                }
            }
        }

        public bool ZIsEnabled
        {
            get
            {
                return _zIsEnabled;
            }
            set
            {
                _zIsEnabled = value;
                OnPropertyChanged("ZIsEnabled");
            }
        }

        public bool ZIsLive
        {
            get
            {
                return _zIsLive;
            }
            set
            {
                _zIsLive = value;
                ROIControlEnabled = (true == value) ? false : true;
                OnPropertyChanged("ROIControlEnabled");
                OnPropertyChanged("ZImagePathPlay");
            }
        }

        public int ZMax
        {
            get
            {
                return _sliderZMax;
            }
            set
            {
                _sliderZMax = value;
                OnPropertyChanged("ZMax");
                OnPropertyChanged("ZVisible");
            }
        }

        public int ZMin
        {
            get
            {
                return _sliderZMin;
            }
            set
            {
                _sliderZMin = value;
                OnPropertyChanged("ZMin");
                OnPropertyChanged("ZVisible");
            }
        }

        public double ZoomLevel
        {
            get
            {
                return _zoomLevel;
            }
            set
            {
                if (value > MAX_ZOOM)
                {
                    _zoomLevel = MAX_ZOOM;
                }
                else if (value < MIN_ZOOM)
                {
                    _zoomLevel = MIN_ZOOM;
                }
                else
                {
                    _zoomLevel = value;
                }

                OnPropertyChanged("ZoomLevel");
            }
        }

        public string ZStreamImagePathPlay
        {
            get
            {
                if (_zStreamIsLive)
                {
                    return @"/ImageReview;component/Icons/Stop.png";
                }
                else
                {
                    return @"/ImageReview;component/Icons/Play.png";
                }
            }
        }

        public bool ZStreamIsEnabled
        {
            get
            {
                return _zStreamIsEnabled;
            }
            set
            {
                _zStreamIsEnabled = value;
                OnPropertyChanged("ZStreamIsEnabled");
            }
        }

        public bool ZStreamIsLive
        {
            get
            {
                return _zStreamIsLive;
            }
            set
            {
                _zStreamIsLive = value;
                ROIControlEnabled = (true == value) ? false : true;
                OnPropertyChanged("ROIControlEnabled");
                OnPropertyChanged("ZStreamImagePathPlay");
            }
        }

        public int ZStreamMax
        {
            get
            {
                return _sliderZStreamMax;
            }
            set
            {
                _sliderZStreamMax = value;
                OnPropertyChanged("ZStreamMax");
                OnPropertyChanged("IsZStreamVisible");
            }
        }

        public int ZStreamMin
        {
            get
            {
                return _sliderZStreamMin;
            }
            set
            {
                _sliderZStreamMin = value;
                OnPropertyChanged("ZStreamMin");
                OnPropertyChanged("IsZStreamVisible");
            }
        }

        public int ZStreamMode
        {
            get
            {
                return _zStreamMode;
            }
            set
            {
                _zStreamMode = value;
                OnPropertyChanged("ZStreamMode");
            }
        }

        public int ZStreamValue
        {
            get
            {
                return this._imageReview.ZStreamValue;
            }
            set
            {
                if (this._imageReview.ZStreamValue == value)
                {
                    SyncROIChart();
                    return;
                }

                if (ZStreamMax >= value && ZStreamMin <= value)
                {
                    this._imageReview.ZStreamValue = value;
                    this._zStreamValue3D = value;
                    OnPropertyChanged("ZStreamValue");

                    UpdateBitmap(.01);
                    //TODO:IV
                    //if (ViewType == Convert.ToInt32(ViewTypes.ViewType3D))
                    //{
                    //    OnPropertyChanged("ZStreamValue3D");
                    //}
                }
                //TODO:IV
                //if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && CloseOrthogonalView != null)
                //{
                //    CloseOrthogonalView();
                //}
                //_updateVirtualStack = true;
                SyncROIChart();
            }
        }

        public int ZStreamValue3D
        {
            get
            {
                return this._zStreamValue3D;
            }
            set
            {
                this._zStreamValue3D = value;
                OnPropertyChanged("ZStreamValue3D");
            }
        }

        public ICommand ZStreamValueMinusCommand
        {
            get
            {
                if (this._zStreamValueMinusCommand == null)
                    this._zStreamValueMinusCommand = new RelayCommand(() => ZStreamValueMinus());

                return this._zStreamValueMinusCommand;
            }
        }

        public ICommand ZStreamValuePlusCommand
        {
            get
            {
                if (this._zStreamValuePlusCommand == null)
                    this._zStreamValuePlusCommand = new RelayCommand(() => ZStreamValuePlus());

                return this._zStreamValuePlusCommand;
            }
        }

        public int ZBoxValue
        {
            get
            {
                return ZValue;
            }
            set
            {
                RXFlagZ = false;
                ZValue = value;
                RXFlagZ = true;
            }
        }

        public int ZValue
        {
            get
            {
                return this._imageReview.ZValue;
            }
            set
            {
                if (this._imageReview.ZValue == value)
                {
                    SyncROIChart();
                    OnPropertyChanged("DepthForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImage");
                    OnPropertyChanged("SpectralForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImageCalculable");
                    OnPropertyChanged("RemoteFocusPlaneNumber");
                    return;
                }

                if (ZMax >= value && ZMin <= value)
                {
                    this._imageReview.ZValue = value;
                    OnPropertyChanged("ZValue");
                    OnPropertyChanged("DepthForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImage");
                    OnPropertyChanged("SpectralForCurrentPreviewImage");
                    OnPropertyChanged("TimeForCurrentPreviewImageCalculable");
                    OnPropertyChanged("RemoteFocusPlaneNumber");
                    OnPropertyChanged("ZBoxValue");

                    UpdateBitmap(.01);
                    if (false == RXFlagZ && (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "RemoteConnection"])
                        MVMManager.Instance["RemoteIPCControlViewModelBase", "ThorsyncFrameSync"] = TValue.ToString() + "/" + ZValue.ToString() + "/" + ZMax.ToString();
                }

                //TODO:IV
                //if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE)
                //{
                //    ZChanged();
                //}

                SyncROIChart();

            }
        }

        public ICommand ZValueMinusCommand
        {
            get
            {
                if (this._zValueMinusCommand == null)
                    this._zValueMinusCommand = new RelayCommand(() => ZValueMinus());

                return this._zValueMinusCommand;
            }
        }

        public ICommand ZValuePlusCommand
        {
            get
            {
                if (this._zValuePlusCommand == null)
                    this._zValuePlusCommand = new RelayCommand(() => ZValuePlus());

                return this._zValuePlusCommand;
            }
        }


        public Visibility ZVisible
        {
            get
            {
                /*
                if (ZMax == ZMin) 
                    return Visibility.Hidden;
                return _zVisible ? Visibility.Visible : Visibility.Collapsed; 
                */
                return ZMax <= ZMin ? Visibility.Collapsed : Visibility.Visible;
            }
            set
            {
                if (ZMax == ZMin)
                {
                    _zVisible = false;
                    return;
                }
                _zVisible = value == Visibility.Visible;
                OnPropertyChanged("ZVisible");
            }
        }

        public Visibility IsTVisible
        {
            get
            {
                return _sliderTMin == _sliderTMax ? Visibility.Collapsed : Visibility.Visible;
            }
        }

        public Visibility IsZStreamVisible
        {
            get
            {
                return (_sliderZStreamMax == _sliderZStreamMin) ? Visibility.Collapsed : Visibility.Visible;

            }
        }


        public double ZVolumeSpacing
        {
            get
            {
                return _zVolumeSpacing;
            }
            set
            {
                _zVolumeSpacing = Math.Abs(value);

                OnPropertyChanged("ZVolumeSpacing");
            }
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    myPropInfo.SetValue(this, value);
                }
            }
        }

        public object this[string propertyName, int index, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        return collection.GetType().GetProperty("Item").GetValue(collection, new object[] { index });
                    }
                    else
                    {
                        return myPropInfo.GetValue(this, null);
                    }
                }
                return defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        collection.GetType().GetProperty("Item").SetValue(collection, value, new object[] { index });
                    }
                    else
                    {
                        myPropInfo.SetValue(this, value, null);
                    }
                }
            }
        }

        #endregion Indexers

        #region Methods

        public void CloseAll()
        {
            if (null != _roiStatsCharts)
            {
                for (int i = 0; i < _roiStatsCharts.Count; i++)
                {
                    if (null != _roiStatsCharts[i])
                    {
                        _roiStatsCharts[i].Close();
                    }
                }
                _roiStatsCharts.Clear();
            }

            if (null != _multiROIStatsWindows)
            {
                for (int i = 0; i < _multiROIStatsWindows.Count; i++)
                {
                    if (null != _multiROIStatsWindows[i])
                    {
                        _multiROIStatsWindows[i].Close();
                    }
                }
                _multiROIStatsWindows.Clear();
            }

            if (null != _lineProfile)
            {
                _lineProfile.Close();
            }
        }

        public void CloseProgressWindow()
        {
            if (null != this._spinnerWindow)
            {
                this._spinnerWindow.Close();
            }
        }

        public void CreateColorMovie(string[][] fileNameList, string destFileNamePath, double fps)
        {
            _movieFileNameList = fileNameList;
            MovieFilePath = destFileNamePath;
            _movieFPS = fps;

            PanelsEnable = false;
            SetMenuBarEnable(false);

            _bw.RunWorkerAsync();
        }

        public void CreateProgressWindow(string message, Brush background)
        {
            if (null != _spinnerWindow)
                return;
            //create a popup modal dialog that blocks user clicks while capturing
            _spinnerWindow = new Window();
            _spinnerWindow.ResizeMode = ResizeMode.NoResize;
            _spinnerWindow.Width = 150;
            _spinnerWindow.Height = 150;
            _spinnerWindow.WindowStyle = WindowStyle.None;
            _spinnerWindow.Background = background;
            _spinnerWindow.AllowsTransparency = true;
            Border border = new Border();
            border.BorderThickness = new Thickness(2);
            _spinnerWindow.Content = border;

            _spinner = new SpinnerProgress.SpinnerProgressControl(message);
            _spinner.Margin = new Thickness(5);
            _spinner.SpinnerWidth = 120;
            _spinner.SpinnerHeight = 120;
            border.Child = _spinner;

            _spinnerWindow.Owner = Application.Current.MainWindow;
            _spinnerWindow.Left = _spinnerWindow.Owner.Left + ((Panel)_spinnerWindow.Owner.Content).ActualWidth / 4;
            _spinnerWindow.Top = _spinnerWindow.Owner.Top + ((Panel)_spinnerWindow.Owner.Content).ActualHeight / 4;
            _spinnerWindow.Closed += new EventHandler(_spinnerWindow_Closed);
            _spinnerWindow.Show();
        }

        public void DelayUpdateBitmap(int milli)
        {
            Thread.Sleep(milli);
            UpdateBitmap(0.01);
        }

        public void EnableHandlers()
        {
            //invoked multiple times at single image review,
            //do release before enable:lin
            ReleaseHandlers();

            _bw.ProgressChanged += new ProgressChangedEventHandler(_bw_ProgressChanged);
            _bw.DoWork += new DoWorkEventHandler(_bw_DoWork);
            _bw.RunWorkerCompleted += new RunWorkerCompletedEventHandler(_bw_RunWorkerCompleted);
            _bw.WorkerReportsProgress = true;
            _bw.WorkerSupportsCancellation = true;

            OverlayManagerClass.Instance.MaskChangedEvent += _overlayManager_MaskChangedEvent;
            OverlayManagerClass.Instance.MaskWillChangeEvent += _overlayManager_MaskWillChangeEvent;

            ROIStatsChart.ViewModel.ChartViewModel.OnXReviewPositionChanged += UpdateAsXReviewPositionChanged;
            OverlayManagerClass.Instance.mROISelectedEvent += Instance_OverlaymROISelectedEvent;
            OverlayManagerClass.Instance.mROIsDisableMoveAndResize = true;
            _imageReview.RegisterCallbacks();
        }

        public void ExecuteROILoadTask(ROIStatsOrigin roiStatsOrigin)
        {
            //background worker to update progress:
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;

            _bwStatsLoaderDone = false;
            ProgressPercentage = 0;
            _splash = new ImageReviewDll.View.SplashScreen();
            try
            {
                if (ROIStatsOrigin.CALCULATE == roiStatsOrigin)
                {
                    _splash.DisplayText = "Please wait while calculating ROI statistics ...";
                }
                else
                {
                    _splash.DisplayText = "Please wait while loading ROI statistics ...";
                }
                _splash.ShowInTaskbar = false;
                _splash.Owner = Application.Current.MainWindow;
                _splash.Show();
                PanelsEnable = false;
                OnPropertyChanged("PanelsEnable");
                _splash.CancelSplashProgress += new EventHandler(CancelUpdateProgress);

                //get dispatcher to update the contents that was created on the UI thread:
                System.Windows.Threading.Dispatcher spDispatcher = _splash.Dispatcher;

                _bwStatsLoader.DoWork += new DoWorkEventHandler(_bwStatsLoader_DoWork);
                _bwStatsLoader.RunWorkerCompleted += new RunWorkerCompletedEventHandler(_bwStatsLoader_RunWorkerCompleted);
                _bwStatsLoader.WorkerSupportsCancellation = true;

                _bwStatsLoader.RunWorkerAsync();
                splashWkr.RunWorkerAsync();
                splashWkr.DoWork += delegate (object sender, DoWorkEventArgs e)
                {
                    do
                    {
                        if (splashWkr.CancellationPending == true)
                        {
                            e.Cancel = true;
                            break;
                        }
                        //wait for loading progress:
                        System.Threading.Thread.Sleep(50);

                        //create a new delegate for updating our progress text
                        UpdateProgressDelegate update = new UpdateProgressDelegate(UpdateProgressText);

                        //invoke the dispatcher and pass the percentage
                        spDispatcher.BeginInvoke(update, ProgressPercentage);
                    }
                    while ((true == _bwStatsLoader.IsBusy) || (_bwStatsLoaderDone == false));
                };

                splashWkr.RunWorkerCompleted += delegate (object sender, RunWorkerCompletedEventArgs e)
                {
                    _splash.Close();
                    // App inherits from Application, and has a Window property called MainWindow
                    // and a List<Window> property called OpenWindows.
                    Application.Current.MainWindow.Activate();

                    //Release handles:
                    _bwStatsLoader.DoWork -= new DoWorkEventHandler(_bwStatsLoader_DoWork);
                    _bwStatsLoader.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(_bwStatsLoader_RunWorkerCompleted);

                    //zoom extend chart if cancel:
                    if (null != _roiStatsCharts)
                    {
                        if (null != _roiStatsCharts[_roiStatsCharts.Count - 1])
                        {
                            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SetInLoading(false);
                            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.UpdataXVisibleRange(1, ZMax * TMax * ZStreamMax * SpMax);
                        }
                    }
                };
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ImageReview Stats Loading Error: " + ex.Message);
                _bwStatsLoader.CancelAsync();
                splashWkr.CancelAsync();
            }
        }

        public void ExperimentDataLoaded()
        {
            if (null != ExperimentPathChanged) ExperimentPathChanged();
            CloseAll();
            //TODO:IV
            //_updateVirtualStack = true;
            SequentialModeEnabled = XmlManager.ReadAttribute<Boolean>(ExperimentDoc, "/ThorImageExperiment/CaptureSequence", "enable");
            MVMManager.Instance["ImageViewReviewVM", "IsInSequentialMode"] = SequentialModeEnabled;
            if (SequentialModeEnabled)
            {
                //update the channel view for sequential capture
                if (ExperimentXMLPath.Substring(ExperimentXMLPath.Length - 4) == ".xml")
                {
                    //Create a copy of the Experiment.xml file to modify the colors in the file. This way Experiment.xml stays unmodified.
                    string colorExperiment = ExperimentXMLPath.Substring(0, ExperimentXMLPath.Length - 4) + "SequentialColorSettings.xml";
                    if (false == File.Exists(colorExperiment))
                    {
                        File.Copy(ExperimentXMLPath, colorExperiment);
                    }
                    MVMManager.Instance["ImageViewReviewVM", "SequentialExperimentPath"] = colorExperiment;
                }
                _imageReview.LoadSequentialInfo();
            }

            UpdateBitmapAndEventSubscribers();
            _imageReview.LoadLSMChannelData();//Channel information needs to be loaded from XML when experiment is loaded
            OnPropertyChanged("BurnInVisibility");
            MVMManager.Instance["ImageViewReviewVM", "ZStepNum"] = ExperimentData.ZMax;
            MVMManager.Instance["ImageViewReviewVM", "ZStepSizeUM"] = ExperimentData.ZStepSizeUM;
            MVMManager.Instance["ImageViewReviewVM", "ExperimentPath"] = ExperimentFolderPath;
            OnPropertyChanged("mROIList");
            OnPropertyChanged("mROIShowOverlays");
            OnPropertyChanged("mROIStripePixels");
            OnPropertyChanged("mROIStripePhysicalFieldSizeUM");
        }

        public void FireColorMappingChangedAction(bool bVal)
        {
            ColorMappingChanged(bVal);
        }

        /// <summary>
        /// retrieve all file names (of type) in the folder based on given expression.
        /// </summary>
        /// <param name="folderPath"></param>
        /// <param name="expression"></param>
        /// <param name="fileType"></param>
        /// <returns></returns>
        public List<String> GetFileList(string folderPath, string expression, string fileType)
        {
            Regex ex = new Regex(expression, RegexOptions.IgnoreCase);
            var fileArray = System.IO.Directory.GetFiles(folderPath, fileType, SearchOption.TopDirectoryOnly).Where(path => ex.IsMatch(path)).ToList();
            return fileArray.ToList();
        }

        // get the files to show in fiji with ome-xml
        public List<string> GetFileNames(string[] filesList)
        {
            int i = 0;  // number of files been searched
            bool notFound = true;
            string selectedChannel = string.Empty;
            List<string> filesToShow = new List<string>();  // collection of names of files to show

            for (int n = 0; n < ImageReview.MAX_CHANNELS; n++)
            {   // collect samples from all channel: 1st image from each channel
                i = 0;
                notFound = true;

                if (n == 0)
                {
                    selectedChannel = "ChanA";
                }
                else if (n == 1)
                {
                    selectedChannel = "ChanB";
                }
                else if (n == 2)
                {
                    selectedChannel = "ChanC";
                }

                else if (n == 3)
                {
                    selectedChannel = "ChanD";
                }

                do // search in untitled folder for image file to display
                {
                    if (filesList[i].Contains(selectedChannel))
                    {
                        filesToShow.Add(filesList[i]);
                        notFound = false;
                    }

                    i++;
                } while (notFound && i < filesList.Length);
            }

            return filesToShow;
        }

        public void GetInscribeParameters(ref d.Point[] linePoints, ref d.RectangleF markRect, ref double scaleLen, ref d.StringFormat drawMarkFormat)
        {
            double _imageWidth = ExperimentData.ImageInfo.pixelX;
            double _imageHeight = ExperimentData.ImageInfo.pixelY;
            double _fieldWidth = Math.Round(ExperimentData.PixelSizeUM.PixelWidthUM * _imageWidth);

            scaleLen = _fieldWidth / 4.0;
            if (scaleLen > 100.0)
                scaleLen = Math.Round(scaleLen / 100.0, 0) * 100;
            else if (scaleLen > 10.0)
                scaleLen = Math.Round(scaleLen / 10.0, 0) * 10;
            else
                scaleLen = Math.Round(scaleLen, 0);
            if (scaleLen == 0) return;

            double scaleImageLen = _imageWidth * scaleLen / _fieldWidth;
            double relativeFactor = _imageHeight / 20.0;

            d.Point ltCor = new d.Point();
            ltCor.X = (int)(_imageWidth * 3.0 / 32.0 + 0.5);    // left bottom corner
            ltCor.Y = (int)(relativeFactor * 18.5 + 0.5);
            d.Point rtCor = new d.Point();
            rtCor.X = (int)(_imageWidth * 3.0 / 32.0 + scaleImageLen + 0.5);    // right bottom corner
            rtCor.Y = (int)(relativeFactor * 18.5 + 0.5);
            d.Point lbCor = new d.Point();
            lbCor.X = (int)(_imageWidth * 3.0 / 32.0 + 0.5);    // left top corner
            lbCor.Y = (int)(relativeFactor * 19 + 0.5);
            d.Point rbCor = new d.Point();
            rbCor.X = (int)(_imageWidth * 3.0 / 32.0 + scaleImageLen + 0.5);    // right top corner
            rbCor.Y = (int)(relativeFactor * 19 + 0.5);

            markRect = new d.RectangleF((float)ltCor.X,
                                                     (float)(ltCor.Y - Math.Max(relativeFactor, _imageWidth / 20) * (0.71 +
                                                     0.2 * (Math.Max(relativeFactor, _imageWidth / 20) / Math.Min(relativeFactor, _imageWidth / 20) - 1))),
                                                     (float)scaleImageLen,
                                                     (float)(Math.Max(relativeFactor, _imageWidth / 20) * 1.2));
            d.Point[] tmpLinePoints = { ltCor, lbCor, rbCor, rtCor };
            linePoints = tmpLinePoints;

            drawMarkFormat = new d.StringFormat();
            drawMarkFormat.Alignment = d.StringAlignment.Center;
            drawMarkFormat.LineAlignment = d.StringAlignment.Center;
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(ImageReviewViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        /// <summary>
        /// Returns the number of images in the raw file input based on the current set
        /// image parameters
        /// </summary>
        /// <param name="filepath"> Path to raw file </param>
        /// <returns> The number of images in the raw file </returns>
        public long ImagesInRawFile(string filepath)
        {
            FileInfo fileInfo = new FileInfo(filepath);
            long fileSize = fileInfo.Length;
            int zSteps = ImageInfo.isZFastEnabled ? ImageInfo.zSteps + ImageInfo.flybackFrames : 1;
            int imageSize = ImageInfo.pixelX * ImageInfo.pixelY * NumChannelsInRaw() * zSteps * 2;

            return fileSize / imageSize / ExperimentData.NumberOfPlanes;
        }

        public async void InscribeScaleToImages()
        {
            PanelsEnable = false;
            SetMenuBarEnable(false);

            d.Point[] linePoints = new d.Point[4];
            d.RectangleF markRect = new d.RectangleF();
            double scaleLen = 0;
            d.StringFormat drawMarkFormat = new d.StringFormat();
            GetInscribeParameters(ref linePoints, ref markRect, ref scaleLen, ref drawMarkFormat);

            int step = (int)(ExperimentData.ImageInfo.pixelX * ExperimentData.ImageInfo.pixelY * 2 * ExperimentData.NumberOfPlanes);

            DirectoryInfo di = new DirectoryInfo(ExperimentFolderPath);
            TotalImageCount = 0;
            int tn = di.GetFiles("*.tif").Length;
            if (tn > 10) //if tif file number for any experientment is greater than 10, we believe it's a capture with single page tifs
            {
                TotalImageCount = tn * 2 + di.GetFiles("*.raw").Length * 3;
            }
            else
            {
                foreach (FileInfo fi in di.GetFiles("*.tif"))
                {
                    using (Tiff t = Tiff.Open(fi.FullName, "r"))
                    {
                        TotalImageCount += (t.NumberOfDirectories() * 2);
                    }
                }
                foreach (FileInfo fi in di.GetFiles("*.raw"))
                {
                    TotalImageCount += ((int)(fi.Length / step) * 3);
                }
            }
            CompletedImageCount = 0;

            HashSet<string> dl = new HashSet<string>(Directory.GetDirectories(di.FullName).Where(s => s.Contains("WithScale")).ToArray<string>());
            int k = 0;
            for (; k < dl.Count(); k++)
                if (!dl.Contains(di.FullName + "WithScale" + k.ToString()))
                    break;
            string savePath = Path.Combine(di.FullName, "WithScale" + k.ToString());
            try
            {
                Directory.CreateDirectory(savePath);
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
                return;
            }

            await System.Threading.Tasks.Task.Run(() =>
            {
                foreach (FileInfo fi in di.GetFiles())
                {
                    switch (fi.Extension)
                    {
                        case ".raw":
                            if (GetTotalFreeSpace(Path.GetPathRoot(fi.FullName)) <= 2 * fi.Length) return;

                            byte[] imgs = InscribeScaleToImagesRaw(fi, linePoints, markRect, scaleLen, drawMarkFormat, step);
                            if (imgs.Length >= step)
                                SaveImagesAsRawFile(fi, ref imgs, step, savePath);
                            break;

                        case ".tif":
                            if (GetTotalFreeSpace(Path.GetPathRoot(fi.FullName)) <= 2 * fi.Length) return;
                            List<byte[]> images = InscribeScaleToImagesTiff(fi, linePoints, markRect, scaleLen, drawMarkFormat);
                            if (images.Count > 0)
                                SaveImagesAsTiffFile(fi, ref images, savePath);

                            break;
                    }
                }
            });
            PanelsEnable = true;
            SetMenuBarEnable(true);
        }

        public long LaunchFijiOMEXML()
        {
            long ret = 0;

            string fijiExeFile = string.Empty;  // note: Bio-Formats does NOT support headless mode.
            string fileName = string.Empty;
            string runCommand = string.Empty;
            string bioformats = "Bio-Formats";
            string options = string.Empty;
            List<string> filesToShow = new List<string>();

            const string CMD_FIJI = "/CommandList/Command/FijiExe";

            string strFile = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\CommandList.xml";

            XmlDocument doc = new XmlDocument();
            doc.Load(strFile);

            XmlNode node = doc.SelectSingleNode(CMD_FIJI);
            if (null != node)
            {
                XmlManager.GetAttribute(node, doc, "value", ref fijiExeFile);
            }

            string folder = _imageReview.ExperimentFolderPath;

            if (folder != null)
            {
                string[] filesList = System.IO.Directory.GetFiles(folder, "*.tif");

                if (filesList.Length > 0)
                {
                    filesToShow = GetFileNames(filesList); // what files to display
                }
                else // if no image files in folder, simply launch fiji
                {
                    runCommand = string.Format("run(\"'{0}'\")", bioformats);
                }
            }
            else // if image folder no existance, simply launch fiji
            {
                return ret;
            }

            /*------------------------------- command line argument format -------------------------------
               * notes for command line
              1. /C                   command line parameter, exit after execute
              2. \"                   handles escape characters in entire command line
              3. {0}                  fiji/imagej exe file
              4. {1}                  image file name
            ----------------------------------------------------------------------------------------------*/
            string fijiCmdText = string.Empty;
            System.Diagnostics.Process process = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();

            if (filesToShow.Count > 0)
            {
                fileName = @filesToShow[0];

                if (filesToShow.Count > 1) // color mode force hyperstack so that to display all channel on a single image
                {
                    options = string.Format("display_ome-xml view=Hyperstack stack_order=XYCZT");
                }
                else // gray scale mode only force to display ome=xml
                {
                    options = string.Format("display_ome-xml");    // escape
                }

                runCommand = string.Format("run('{0}','open=\\'{1}\\' {2}')", bioformats, fileName.Replace("\\", "\\\\"), options);
                fijiCmdText = string.Format("/C \"\"{0}\" -eval \"{1}\"", fijiExeFile, runCommand);

                try
                {
                    startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
                    startInfo.FileName = "cmd.exe";

                    startInfo.Arguments = fijiCmdText;
                    process.StartInfo = startInfo;
                    process.Start();
                    //_processOn = true;        // like to keep this in case user changes
                    //process.WaitForExit();

                    ret = 1;
                }
                catch (Exception ex)
                {
                    ex.ToString();
                    ret = 0;
                }
            }
            else
            {
                fijiCmdText = string.Format("/C \"\"{0}\" -eval {1}", fijiExeFile, runCommand);

                try
                {
                    startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
                    startInfo.FileName = "cmd.exe";

                    startInfo.Arguments = fijiCmdText;
                    process.StartInfo = startInfo;
                    process.Start();
                    //_processOn = true;
                    //process.WaitForExit();

                    ret = 1;
                }
                catch (Exception ex)
                {
                    ex.ToString();
                    ret = 0;
                }
            }

            return ret;
        }

        public long LaunchMatlab()
        {
            long ret = 0;
            string str = string.Empty;
            string path = _imageReview.ExperimentXMLPath;
            if (path != null)
            {
                switch (ImageInfo.imageType)
                {
                    case CaptureFile.FILE_BIG_TIFF:
                        break;
                    case CaptureFile.FILE_RAW:
                        str = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\..\\Matlab Scripts\\" + "loadThorlabsExperimentRaw.m";
                        MatlabEngine.Instance.StartMatlabApp(Path.GetFullPath(str), path);
                        break;
                    case CaptureFile.FILE_TIFF:
                        str = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\..\\Matlab Scripts\\" + "loadThorlabsExperiment.m";
                        MatlabEngine.Instance.StartMatlabApp(Path.GetFullPath(str), path);
                        break;
                    default:
                        break;
                }
                ret = 1;
            }
            return ret;
        }

        public long LaunchThorAnalysis()
        {
            long ret = 0;

            // Judge whether ThorAnalysis has been installed
            var isThorAnalysisInstalled = false;
            string exePath = "";
            var regKey = Registry.LocalMachine;
            var regSubKey = regKey.OpenSubKey(@"SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\", false);
            foreach (var keyName in regSubKey.GetSubKeyNames())
            {
                if (keyName.Contains("ThorAnalysis"))
                {
                    isThorAnalysisInstalled = true;

                    var subKey = regSubKey.OpenSubKey(keyName);
                    exePath = subKey.GetValue("")?.ToString();
                    break;
                }
            }

            if (!isThorAnalysisInstalled || string.IsNullOrEmpty(exePath))
            {
                MessageBox.Show("Failed to launch ThorAnalysis, please install or contact Thorlabs Support");
            }
            else
            {
                //Load active.xml to find experiment folder and type
                XmlDocument experimentDoc = LoadDocAsReadOnly(_imageReview.ExperimentXMLPath);
                var pathXmlNode = experimentDoc.SelectSingleNode("/ThorImageExperiment/Name");
                var typeXmlNode = experimentDoc.SelectSingleNode("/ThorImageExperiment/Streaming");
                if (pathXmlNode != null && typeXmlNode != null)
                {
                    string pathString = string.Empty;
                    string typeString = string.Empty;
                    if (XmlManager.GetAttribute(pathXmlNode, experimentDoc, "path", ref pathString)
                        && XmlManager.GetAttribute(typeXmlNode, experimentDoc, "rawData", ref typeString))
                    {
                        if (typeString == "2") // OME-Tiff, open directly
                        {
                            var imgFile = pathString + "\\Image.tif";
                            if (File.Exists(imgFile))
                            {
                                LoadOMETiffInProcess(exePath, imgFile);
                                ret = 1;
                            }
                        }
                        else if (typeString == "0" || typeString == "1")
                        {
                            // Start converting
                            Task task = Task.Run(() =>
                            {
                                // Convert from Tiff or RAW to OME Tiff
                                ClassicTiffConverter tiffConverter = new ClassicTiffConverter(pathString);
                                long result = tiffConverter.DoConvert(out string convertedImgFile);

                                if (result == 0)
                                {
                                    LoadOMETiffInProcess(exePath, convertedImgFile);
                                    ret = 1;
                                }

                            });
                            task.ContinueWith(x =>
                            {
                                Application.Current.Dispatcher.BeginInvoke((Action)delegate ()
                                {
                                    CloseProgressWindow();
                                });
                            });

                            // Show progress window
                            Application.Current.Dispatcher.BeginInvoke((Action)delegate ()
                            {
                                CreateProgressWindow("Converting", Brushes.DimGray);
                            });
                        }
                    }
                }
            }

            return ret;
        }

        public void LoadColorImageSettings()
        {
            MVMManager.Instance["ImageViewReviewVM", "ReviewApplicationSettings"] = ApplicationDoc;
            MVMManager.Instance["ImageViewReviewVM", "ReviewApplicationSettingsPath"] = File.Exists(ApplicationSettingPath) ? ApplicationSettingPath : string.Empty;
            MVMManager.Instance["ImageViewReviewVM", "ReviewHardwareSettings"] = HardwareDoc; //Loading the HardwareSettings document triggers a LoadColors process in ImageViewControlMVM
            MVMManager.Instance["ImageViewReviewVM", "ReviewHardwareSettingsPath"] = File.Exists(HardwareSettingPath) ? HardwareSettingPath : string.Empty;

            if (null != _lineProfile)
            {
                _lineProfile.ColorAssigment = GetColorAssignments();
            }
        }

        public XmlDocument LoadDocAsReadOnly(string xmlPath)
        {
            XmlDocument origDoc = new XmlDocument();
            origDoc.Load(xmlPath);
            // Create a clone of the original document to make sure we don't modify Experiment.xml
            XmlDocument readOnlyExpDoc = new XmlDocument();
            readOnlyExpDoc.LoadXml(origDoc.OuterXml);
            origDoc = null;
            return readOnlyExpDoc;
        }

        public Boolean LoadExpFolder(string folderPath)
        {
            StringBuilder sbTemp = new StringBuilder();
            DirectoryInfo di = new DirectoryInfo(folderPath);
            FileInfo[] fi;
            int totalTileZTCount = 0;    //used for raw or big tiff, 0: if load failed
            int timeCount = 0;           //time frame count

            //default if load failed
            this.PrimaryChannelIndex = 0;
            this.PrimaryChannelFileNames.Clear();
            if (CaptureModes.HYPERSPECTRAL == CaptureMode)
            {
                this.SpMax = 1;
            }
            else
            {
                //this.TMax = 1;

            }
            //this.ZMax = 1;

            //load file
            switch (ImageInfo.imageType)
            {
                case CaptureFile.FILE_BIG_TIFF:
                    int regionCount = 0, width = 0, height = 0, channelCount = 0, zMaxCount = 0, specCount = 0;
                    sbTemp.AppendFormat("{0}", "Image");
                    fi = di.GetFiles(sbTemp + "*.tif*");
                    if (fi != null && fi.Length != 0 && true == File.Exists(fi[0].FullName))
                    {
                        ImageReview.GetImageStoreInfo(fi[0].FullName, ScanAreaID, ref regionCount, ref width, ref height, ref channelCount, ref zMaxCount, ref timeCount, ref specCount);

                        this.ZMax = (1 < ImageInfo.zSteps && true == ImageInfo.isZFastEnabled) ? ImageInfo.zSteps : 1;

                        totalTileZTCount = zMaxCount * timeCount;
                    }
                    break;
                case CaptureFile.FILE_RAW:
                    {
                        sbTemp.AppendFormat("{0}{1}{2}",
                            "Image",
                            "_" + this.SampleSiteIndex.ToString(ImageNameFormat),
                            "_" + this.SubTileIndex.ToString(ImageNameFormat));
                        fi = di.GetFiles(sbTemp + "*.raw*");

                        if (fi != null && fi.Length != 0 && true == File.Exists(fi[0].FullName))
                        {
                            //timeCount does not include tiles since no raw mode for Z & T
                            timeCount = (int)ImagesInRawFile(fi[0].FullName);
                            timeCount = (1 > timeCount) ? 1 : timeCount;

                            this.ZMax = (1 < ImageInfo.zSteps && true == ImageInfo.isZFastEnabled) ? ImageInfo.zSteps : 1;

                            totalTileZTCount = this.ZMax * timeCount;
                        }
                    }
                    break;
                case CaptureFile.FILE_TIFF:
                    {
                        var chanLists = new Dictionary<int, List<String>>();
                        string pattern = string.Empty;

                        for (int i = 0; i < this.WavelengthNames.Length; i++)
                        {
                            pattern = this.WavelengthNames[i] + "_(.*)_(.*)_(.*)_(.*)";
                            chanLists[i] = this.GetFileList(folderPath, pattern, "*.tif");
                            this.PrimaryChannelIndex = ((i > 0) && (chanLists[i].Count > chanLists[0].Count)) ? i : 0;
                        }
                        //Sort ref. channel's files in order to be PrimaryChannelFileNames:
                        this.SortPrimaryChannelFileList(chanLists[this.PrimaryChannelIndex]);

                        totalTileZTCount = this.PrimaryChannelFileNames.Count;
                        if (0 < this.PrimaryChannelFileNames.Count)
                        {
                            if (_sliderZStreamMax > 1)
                                timeCount = this.PrimaryChannelFileNames[this.PrimaryChannelFileNames.Count - 1].Tid / this.PrimaryChannelFileNames[this.PrimaryChannelFileNames.Count - 1].Zid / _sliderZStreamMax;
                            else
                                timeCount = this.PrimaryChannelFileNames[this.PrimaryChannelFileNames.Count - 1].Tid;
                        }

                        if (timeCount > 0)
                            ZMax = totalTileZTCount / timeCount;

                        //reset ZMax for Tiff format, no PrimaryChannelFileNames for other file types
                        if (totalTileZTCount > 0)
                        {
                            //reset ZMax:
                            if (this.PrimaryChannelFileNames.Find(x => x.Zid == this.ZMax) == null) //modified
                            {
                                for (int i = (this.ZMax - 1); i > 0; i--)
                                {
                                    if (this.PrimaryChannelFileNames.Find(x => x.Zid == i) != null)
                                    {
                                        this.ZMax = i;
                                        break;
                                    }
                                }
                            }

                        }

                    }
                    break;
                default:
                    break;
            }

            //file is loaded, reset TMax:
            if (totalTileZTCount > 0)
            {
                if (CaptureModes.HYPERSPECTRAL == CaptureMode)
                {
                    if (this.ZStreamMax > 1 || this.ZMax > 1)
                    {
                        this.SpMax = (int)Math.Ceiling((double)totalTileZTCount / (this.ZMax * this.ZStreamMax));
                    }
                    else
                    {
                        this.SpMax = ImageInfo.spectralSteps;
                    }
                }
                else
                {
                    this.TMax = timeCount;
                }
            }
            return true;
        }

        public void LoadLineProfileData()
        {
            _imageReview.LoadLineProfileData();
        }

        public void LoadViewModelSettingsDoc()
        {
            XmlDocument hwSettingsDoc = new XmlDocument();
            if (File.Exists(HardwareSettingPath))
            {
                hwSettingsDoc.Load(HardwareSettingPath);
                HardwareDoc = hwSettingsDoc;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " Unable to load hardware setttings");
            }

            XmlDocument appSettingsDoc = new XmlDocument();
            if (File.Exists(ApplicationSettingPath))
            {
                appSettingsDoc.Load(ApplicationSettingPath);
                ApplicationDoc = appSettingsDoc;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " Unable to load application settings");
            }
            OnPropertyChanged("BurnInVisibility");
        }

        public void LoadXMLSettings()
        {
            XmlNodeList node = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ChannelTileViewSettings");
            string str = string.Empty;
            if (0 < node.Count)
            {
                MVMManager.Instance["ImageViewReviewVM", "TileDisplay"] = XmlManager.GetAttribute(node[0], ApplicationDoc, "TilingEnableOption", ref str) && (str == "1" || str == bool.TrueString);
                MVMManager.Instance["ImageViewReviewVM", "VerticalTileDisplay"] = XmlManager.GetAttribute(node[0], ApplicationDoc, "VerticalTiling", ref str) && (str == "1" || str == bool.TrueString);
            }
        }

        /// <summary>
        /// Calculates the number of channels that are expected to be actually represented 
        /// in a binary raw file based on the current image parameters
        /// </summary>
        /// <returns> The number of channels in the raw binary file </returns>
        public int NumChannelsInRaw()
        {
            int channelsInRaw = 0;

            //Count Enabled Channels
            int enabledChannels = 0;
            int channelBit = 1;
            for (int channel = 0; channel < ImageReview.MAX_CHANNELS; channel++)
            {
                if ((ImageInfo.channelEnabled & channelBit) > 0)
                {
                    enabledChannels++;
                }
                channelBit *= 2;
            }

            if (enabledChannels <= 1)
            {
                channelsInRaw = 1;
            }
            else if (ExperimentData.OnlyEnabledChannels != 1)
            {
                channelsInRaw = ImageReview.MAX_CHANNELS;
            }
            else
            {
                channelsInRaw = enabledChannels;
            }

            return channelsInRaw;
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        public void ReleaseHandlers()
        {
            _bw.ProgressChanged -= new ProgressChangedEventHandler(_bw_ProgressChanged);
            _bw.DoWork -= new DoWorkEventHandler(_bw_DoWork);
            _bw.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(_bw_RunWorkerCompleted);

            OverlayManagerClass.Instance.MaskChangedEvent -= _overlayManager_MaskChangedEvent;
            OverlayManagerClass.Instance.MaskWillChangeEvent -= _overlayManager_MaskWillChangeEvent;

            ROIStatsChart.ViewModel.ChartViewModel.OnXReviewPositionChanged -= UpdateAsXReviewPositionChanged;
            _imageReview.UnRegisterCallbacks();
            OverlayManagerClass.Instance.mROIsDisableMoveAndResize = false;
            OverlayManagerClass.Instance.mROISelectedEvent -= Instance_OverlaymROISelectedEvent;

            mROIShowOverlays = false;
        }

        public void ReloadSettingsByExpModality()
        {
            LoadViewModelSettingsDoc();

            SetDisplayOptions();

            LoadColorImageSettings();

            //notice current experiment modality
            if (null != _eventAggregator)
            {
                Command modalityCommand = new Command();
                modalityCommand.Message = ExperimentModality;
                _eventAggregator.GetEvent<CommandReviewModalityEvent>().Publish(modalityCommand);
            }
        }

        //TODO:IV
        //public void SaveImage(String filename, int filterIndex)
        //{
        //    FileStream stream = new FileStream(filename, FileMode.Create);
        //    switch (filterIndex)
        //    {
        //        case 1:
        //            {
        //                //8 bit tiff image save
        //                TiffBitmapEncoder encoder = new TiffBitmapEncoder();
        //                encoder.Frames.Add(BitmapFrame.Create(_bitmap));
        //                encoder.Save(stream);
        //            }
        //            break;
        //        case 2:
        //            {
        //                //16 bit tiff image save
        //                Bitmap16();
        //                TiffBitmapEncoder encoder = new TiffBitmapEncoder();
        //                encoder.Frames.Add(BitmapFrame.Create(_bitmap16));
        //                encoder.Save(stream);
        //            }
        //            break;
        //        case 3:
        //            {
        //                //8 bit jpeg image save
        //                JpegBitmapEncoder jpgEncoder = new JpegBitmapEncoder();
        //                jpgEncoder.Frames.Add(BitmapFrame.Create(_bitmap));
        //                jpgEncoder.Save(stream);
        //            }
        //            break;
        //    }
        //    stream.Close();
        //}
        public void SetDisplayOptions()
        {
            XmlNodeList ndList;

            ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");
            if (ndList.Count > 0)
            {
                XYVisible = ndList[0].Attributes["Visibility"].Value.Equals("Visible");
            }

            ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ZView");
            if (ndList.Count > 0)
            {
                ZVisible = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Hidden;
            }
        }

        /// <summary>
        /// Sort provided image file names in order.
        /// </summary>
        /// <param name="fNameListIn"></param>
        public void SortPrimaryChannelFileList(List<string> fNameListIn)
        {
            if (fNameListIn.Count <= 0)
            { return; }

            string pattern = "(.*)_(.*)_(.*)_(.*)_(.*).tif";
            Regex ex = new Regex(pattern, RegexOptions.IgnoreCase);
            ImageFileNameClass[] fNameClass = new ImageFileNameClass[fNameListIn.Count];

            for (int i = 0; i < fNameListIn.Count; i++)
            {
                Match match = ex.Match(fNameListIn[i]);
                fNameClass[i] = new ImageFileNameClass(fNameListIn[i]);
            }
            IEnumerable<ImageFileNameClass> query = fNameClass.OrderBy(x => x.Wellid).ThenBy(x => x.Tileid).ThenBy(x => x.Tid).ThenBy(x => x.Zid);
            PrimaryChannelFileNames = query.ToList();
        }

        public void StartImageLoadingThread()
        {
            _runImageLoadingThread = true;
            _imageLoadingThread = new Thread(new ThreadStart(BuildBitmapsTask));
            _imageLoadingThread.IsBackground = true;
            _imageLoadingThread.Priority = ThreadPriority.AboveNormal;
            _imageLoadingThread.Start();
        }

        public void StopImageLoadingThread()
        {
            _runImageLoadingThread = false;
            _imageLoadingThread?.Abort();
        }

        public void SyncROIChart()
        {
            if (null != _roiStatsCharts)
            {
                if (CaptureModes.HYPERSPECTRAL == CaptureMode)
                {
                    int x = (SpValue - 1) * ZMax * ZStreamMax + (ZValue - 1) * ZStreamMax + ZStreamValue;
                    for (int i = 0; i < _roiStatsCharts.Count; i++)
                    {
                        if (null != _roiStatsCharts[i])
                        {
                            _roiStatsCharts[i].UpdataXReviewPosition(x);
                        }
                    }
                }
                else
                {
                    int x = (TValue - 1) * ZMax * ZStreamMax + (ZValue - 1) * ZStreamMax + ZStreamValue;
                    for (int i = 0; i < _roiStatsCharts.Count; i++)
                    {
                        if (null != _roiStatsCharts[i])
                        {
                            _roiStatsCharts[i].UpdataXReviewPosition(x);
                        }
                    }
                }
            }
        }

        public void UpdateAsXReviewPositionChanged(double x, string[] basicStatNames, double[] basicStatValues, string[] arithmeticStatNames, double[] arithmeticStatValues, int tag)
        {
            if (null != _multiROIStatsWindows)
            {
                if (true == InUpdateROIStatsMut.WaitOne(1000))
                {
                    try
                    {
                        if (null != _multiROIStatsWindows[tag])
                            if (basicStatNames.Length == basicStatValues.Length && 0 < basicStatNames.Length && arithmeticStatNames.Length == arithmeticStatValues.Length)
                            {
                                _multiROIStatsWindows[tag].SetData(basicStatNames, basicStatValues);
                                _multiROIStatsWindows[tag].SetArithmeticsData(arithmeticStatNames, arithmeticStatValues);
                            }
                        SpMax = Math.Max(1, SpMax);
                        TMax = Math.Max(1, TMax);
                        ZMax = Math.Max(1, ZMax);
                        ZStreamMax = Math.Max(1, ZStreamMax);

                        if (x > (SpMax * ZMax * TMax * ZStreamMax))
                        {
                            SpValue = SpMax;
                            TValue = TMax;
                            ZValue = ZMax;
                            ZStreamValue = ZStreamMax;
                        }
                        else
                        {
                            if (ZValue != (int)((((int)x % (ZMax * ZStreamMax) + (ZStreamMax - 1)) / ZStreamMax)))
                            {
                                if (0 == (int)((((int)x % (ZMax * ZStreamMax) + (ZStreamMax - 1)) / ZStreamMax)))
                                {
                                    ZValue = ZMax;
                                }
                                else
                                {
                                    ZValue = (int)((((int)x % (ZMax * ZStreamMax) + (ZStreamMax - 1)) / ZStreamMax));
                                }
                            }
                            if (CaptureModes.HYPERSPECTRAL == CaptureMode)
                            {
                                if (SpValue != (int)(Math.Ceiling(x / (ZMax * ZStreamMax))))
                                {
                                    SpValue = (int)(Math.Ceiling(x / (ZMax * ZStreamMax)));
                                }
                            }
                            else
                            {
                                if (TValue != (int)(Math.Ceiling(x / (ZMax * ZStreamMax))))
                                {
                                    TValue = (int)(Math.Ceiling(x / (ZMax * ZStreamMax)));
                                }
                            }
                            if (ZStreamValue != ((int)x % ZStreamMax))
                            {
                                ZStreamValue = ((int)x % ZStreamMax == 0) ? ZStreamMax : (int)x % ZStreamMax;
                            }
                        }
                        InUpdateROIStatsMut.ReleaseMutex();
                        SyncROIChart();
                    }
                    catch
                    {
                        InUpdateROIStatsMut.ReleaseMutex();
                    }
                }
            }
        }

        /// <summary>
        /// Forces the bitmap to reload based on the current parameters if a minimum time has passed
        /// </summary>
        /// <param name="minTimeBetweenUpdates"> The minimum time since the last update. 
        ///  Will not update if the time elapsed is less than this argument. </param>
        /// <returns> The amount of time since the last update. 
        /// 0 is returned if updated this call. -1 is returned if the image trying to be loaded is invalid </returns>
        public Task<double> UpdateBitmap(double minTimeBetweenUpdates = 0)
        {
            if (!_allowImageUpdate) return Task.FromResult(0.0);
            _loadImages = true;
            return Task.FromResult(0.0);
        }

        /// <summary>
        /// Rebuilds the bitmap and notifies subscribers
        /// e.g. Histograms
        /// </summary>
        public void UpdateBitmapAndEventSubscribers()
        {
            UpdateBitmap(.01);
        }

        /// <summary>
        /// update channel's data, use scan area list if exist, regular pixelX / pixelY otherwise
        /// e.g. Histograms
        /// </summary>
        public Task<bool> UpdateChannelData(List<string> fileNames, int zIndexToRead, int tIndexToRead, int scanAreaID = 0, int scanAreaIndex = 0)
        {
            if (ExperimentData.IsmROICapture && ImageInfo.imageType == CaptureFile.FILE_RAW)
            {
                int[] pixelXY = { ExperimentData.mROIs[scanAreaIndex].SizeXPixels, ExperimentData.mROIs[scanAreaIndex].SizeYPixels };

                _imageReview.UpdateChannelData(fileNames, ImageInfo.channelEnabled, 4, zIndexToRead, tIndexToRead, pixelXY[0], pixelXY[1] * ExperimentData.NumberOfPlanes, ImageInfo.zSteps
                + ImageInfo.flybackFrames, ExperimentData.OnlyEnabledChannels != 1, scanAreaID, scanAreaIndex);
            }
            else
            {
                var srStruct = ImageInfo.scanAreaIDList.SingleOrDefault(i => i.RegionID == ScanAreaID);

                int[] pixelXY = { (0 < ImageInfo.scanAreaIDList.Count && 0 < srStruct.SizeX) ? (int)srStruct.SizeX : ImageInfo.pixelX,
                           (0 < ImageInfo.scanAreaIDList.Count && 0 < srStruct.SizeY) ? (int)srStruct.SizeY : ImageInfo.pixelY};

                _imageReview.UpdateChannelData(fileNames, ImageInfo.channelEnabled, 4, zIndexToRead, tIndexToRead, pixelXY[0], pixelXY[1] * ExperimentData.NumberOfPlanes, ImageInfo.zSteps
                    + ImageInfo.flybackFrames, ExperimentData.OnlyEnabledChannels != 1, ScanAreaID);
            }
            return Task.FromResult(true);
        }

        public void UpdateExpXMLSettings(ref XmlDocument xmlDoc)
        {
        }

        //TODO:IV
        //public void UpdateGrayscaleForSingleChannel()
        //{
        //    //if this is a single channel experiment
        //    //check to see user wants to view the data
        //    //as grayscale
        //    if (null == ApplicationDoc)
        //        return;
        //    XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/GrayscaleForSingleChannel");
        //    string str = string.Empty;
        //    if (ndList.Count > 0)
        //    {
        //        if (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "value", ref str))
        //        {
        //            GrayscaleForSingleChannel = ("1" == str || Boolean.TrueString == str) ? true : false;
        //        }
        //    }
        //}
        //TODO:IV
        //public void UpdateOrthogonalBitmapAndEventSubscribers()
        //{
        //    if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE)
        //    {
        //        UpdateOrthogonalViewImages();
        //    }
        //}
        //TODO:IV
        //public void UpdateOrthogonalView()
        //{
        //    BackgroundWorker splashWkr = new BackgroundWorker();
        //    splashWkr.WorkerSupportsCancellation = true;
        //    _bwOrthogonalImageLoaderDone = false;
        //    ProgressPercentage = 0;
        //    _splash = new ImageReviewDll.View.SplashScreen();
        //    try
        //    {
        //        _splash.DisplayText = "Please wait while loading images ...";
        //        _splash.ShowInTaskbar = false;
        //        _splash.Owner = Application.Current.MainWindow;
        //        _splash.Show();
        //        _splash.CancelSplashProgress += delegate (object sender, EventArgs e)
        //        {
        //            splashWkr.CancelAsync();
        //        };
        //        PanelsEnable = false;
        //        //get dispatcher to update the contents that was created on the UI thread:
        //        System.Windows.Threading.Dispatcher spDispatcher = _splash.Dispatcher;
        //        splashWkr.DoWork += delegate (object sender, DoWorkEventArgs e)
        //        {
        //            int totalNumOfZstack = this.ZMax - this.ZMin + 1;
        //            if ((false == VirtualZStack || true == _updateVirtualStack) ||
        //                (null == _tiffBufferArray) || (_tiffBufferArray.Length != totalNumOfZstack))
        //            {
        //                if ((null == _tiffBufferArray) || (_tiffBufferArray.Length != totalNumOfZstack))
        //                {
        //                    _tiffBufferArray = new ushort[totalNumOfZstack][];
        //                }
        //                for (int i = 0; i < totalNumOfZstack; i++)
        //                {
        //                    if (splashWkr.CancellationPending == true)
        //                    {
        //                        e.Cancel = true;
        //                        break;
        //                    }
        //                    //load the images
        //                    string[] fileNames = GetFileNames(this.ZMin + i); // get first image
        //                    UpdateChannelData(fileNames, i, ((CaptureModes.HYPERSPECTRAL == CaptureMode) ? SpValue - 1 : TValue - 1));
        //                    if (VirtualZStack)
        //                    {
        //                        if (null == _tiffBufferArray[i] || _tiffBufferArray[i].Length != _imageReview.PixelData.Length)
        //                        {
        //                            _tiffBufferArray[i] = new ushort[_imageReview.PixelData.Length];
        //                        }
        //                        Buffer.BlockCopy(_imageReview.PixelData, 0, _tiffBufferArray[i], 0, _imageReview.PixelData.Length * sizeof(short));
        //                    }
        //                    CreateOrthogonalBitmap(i);
        //                    ImageReview.FinishedCopyingPixel();
        //                    //report progress:
        //                    _progressPercentage = (int)(i * 100 / totalNumOfZstack);
        //                    //create a new delegate for updating our progress text
        //                    UpdateProgressDelegate update = new UpdateProgressDelegate(UpdateProgressText);
        //                    //invoke the dispatcher and pass the percentage
        //                    spDispatcher.BeginInvoke(update, ProgressPercentage);
        //                }
        //                _maxChannel = this.ImageReview.MaxChannels;
        //                _updateVirtualStack = false;
        //            }
        //        };
        //        splashWkr.RunWorkerCompleted += delegate (object sender, RunWorkerCompletedEventArgs e)
        //        {
        //            _splash.Close();
        //            // App inherits from Application, and has a Window property called MainWindow
        //            // and a List<Window> property called OpenWindows.
        //            Application.Current.MainWindow.Activate();
        //            _bwOrthogonalImageLoaderDone = true;
        //            PanelsEnable = true;
        //            if (e.Cancelled == false)
        //            {
        //                OnPropertyChanged("BitmapXZ");
        //                OnPropertyChanged("BitmapYZ");
        //            }
        //            if (OrthogonalViewImagesLoaded != null)
        //            {
        //                OrthogonalViewImagesLoaded(e.Cancelled);
        //            }
        //        };
        //        splashWkr.RunWorkerAsync();
        //    }
        //    catch (Exception ex)
        //    {
        //        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ImageReview Orthogonal Images Loading Error: " + ex.Message);
        //        splashWkr.CancelAsync();
        //    }
        //}
        //public void UpdateOrthogonalViewImages()
        //{
        //    //Check image files.
        //    string[] fileNames = GetFileNames(ZMin);
        //    bool bEmpty = true;
        //    foreach (string file in fileNames)
        //    {
        //        if (null != file && 0 != file.Length && file != string.Empty) // If first image is not existed
        //        {
        //            bEmpty = false;
        //            break;
        //        }
        //    }
        //    if (true == bEmpty)
        //    {
        //        MessageBox.Show("No Image is found", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
        //        return;
        //    }
        //    //Set Click time-Gap
        //    TimeSpan ts;
        //    ts = DateTime.Now - _lastOrthogonalViewUpdateTime;
        //    if (ts.TotalSeconds > 0.01 && _bwOrthogonalImageLoaderDone == true)
        //    {
        //        if (VirtualZStack == true)
        //        {
        //            int totalNumOfZstack = this.ZMax - this.ZMin + 1;
        //            for (int i = 0; i < totalNumOfZstack; i++)
        //            {
        //                CreateOrthogonalBitmap(i); // create Bitmap
        //            }
        //            OnPropertyChanged("BitmapXZ");
        //            OnPropertyChanged("BitmapYZ");
        //        }
        //        else
        //        {
        //            UpdateOrthogonalView();
        //        }
        //        _lastOrthogonalViewUpdateTime = DateTime.Now;
        //    }
        //}
        /// <summary>
        /// this is the method that the UpdateProgressDelegate will execute
        /// </summary>
        /// <param name="percentage"></param>
        public void UpdateProgressText(int percentage)
        {
            //set our progress dialog text and value
            _splash.ProgressText = string.Format("{0}%", percentage.ToString());
            _splash.ProgressValue = percentage;
        }

        protected virtual void BuildBitmapsTask()
        {
            do
            {
                Thread.Sleep(1);
                if (_loadImages && !_isExperimentLoading)
                {
                    lock (_updateBitmapLock)
                    {
                        //_bitmapIsLoading = true;
                        if (ZStreamIsEnabled && ZIsEnabled && SpIsEnabled && TIsEnabled)
                        {
                            // Show progress window
                            Application.Current.Dispatcher.BeginInvoke((Action)delegate ()
                            {
                                CreateProgressWindow("Loading", Brushes.Transparent);
                            });
                            IndexControlsEnabled = false;
                        }

                        _loadImages = false;
                        bool loadImagesFail= -1 == LoadImages();

                        if (false == IndexControlsEnabled)
                        {
                            Application.Current.Dispatcher.BeginInvoke((Action)delegate ()
                            {
                                CloseProgressWindow();
                                if (loadImagesFail)
                                    MessageBox.Show("No Experiment data available.", "Failed to load experiment", MessageBoxButton.OK, MessageBoxImage.Error);

                            });
                            IndexControlsEnabled = true;
                        }
                        _bitmapIsLoading = false;
                    }
                }
            }
            while (_runImageLoadingThread);
        }

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "CheckGroupDataset")]
        private static extern int CheckH5GrpData(string groupnm, string datasetnm, ref UInt64 size);

        [DllImport("..\\Modules_Native\\HDF5IO.dll", EntryPoint = "CloseFileIO")]
        private static extern int CloseH5File();

        [DllImport(".\\StatsManager.dll", EntryPoint = "ComputeStats")]
        private static extern int ComputeStats(short[] data, FrameInfoStruct frameInfo, long colorChannels, int includeLineProfile, int includeRegularStats, int enabledChannelsOnly);

        [DllImport(".\\StatsManager.dll", EntryPoint = "CreateStatsManagerROIDS")]
        private static extern int CreateStatsManagerROIDS(long dsType, byte[] pathAndName);

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "GetGroupDatasetNames", CharSet = CharSet.Unicode)]
        private static extern int GetGroupDatasetNames(string path, out IntPtr groups, out int gnum, out IntPtr datasets, out int dnum);

        [DllImport(".\\StatsManager.dll", EntryPoint = "GetNumROI")]
        private static extern int GetNumROI();

        [DllImport(".\\StatsManager.dll", EntryPoint = "IsStatsComplete")]
        private static extern int IsStatsComplete();

        [DllImport("..\\Modules_Native\\HDF5IO.dll", EntryPoint = "OpenFileIO", CharSet = CharSet.Unicode)]
        private static extern int OpenH5File(string filenm, int openType);

        [DllImport("..\\Modules_Native\\HDF5IO.dll", EntryPoint = "ReadData")]
        private static extern int ReadH5Data(string groupnm, string datasetnm, IntPtr buf, int dataType, UInt64 start, UInt64 readsize);

        [DllImport(".\\ROIDataStore.dll", EntryPoint = "RequestROIData")]
        private static extern void RequestROIData();

        [DllImport("..\\Modules_Native\\HDF5IO.dll", EntryPoint = "SetPathandFilename", CharSet = CharSet.Unicode)]
        private static extern int SetPathandFilename(string path);

        private System.Drawing.Bitmap BitmapFromSource(BitmapSource bitmapsource)
        {
            System.Drawing.Bitmap bitmap;
            using (MemoryStream outStream = new MemoryStream())
            {
                // from System.Media.BitmapImage to System.Drawing.Bitmap
                BitmapEncoder enc = new BmpBitmapEncoder();
                enc.Frames.Add(BitmapFrame.Create(bitmapsource));
                enc.Save(outStream);
                bitmap = new System.Drawing.Bitmap(outStream);
            }
            return bitmap;
        }

        private BitmapSource BitmapSourceFromArray(byte[] pixels, int width, int height)
        {
            WriteableBitmap bitmap = new WriteableBitmap(width, height, 96, 96, PixelFormats.Rgb48, null);

            bitmap.WritePixels(new Int32Rect(0, 0, width, height), pixels, width * (bitmap.Format.BitsPerPixel / 8), 0);

            return bitmap;
        }

        private byte[] BitmapSourceToArray(BitmapSource bitmapSource)
        {
            // Stride = (width) x (bytes per pixel)
            int stride = (int)bitmapSource.PixelWidth * (bitmapSource.Format.BitsPerPixel / 8);
            byte[] pixels = new byte[(int)bitmapSource.PixelHeight * stride];

            bitmapSource.CopyPixels(pixels, stride, 0);

            return pixels;
        }

        private void btRoiCal_Click()
        {
            if (0 == GetNumROI())   // return if there is no roi defined
            {
                MessageBox.Show("No ROIs defined.");
                return;
            }

            string[] fileNamesToRead = new string[4];

            string folderName = "Analysis001";
            StringBuilder sbNewDir = new StringBuilder(ImageReview.ExperimentFolderPath + "Analysis001");

            if (Directory.Exists(sbNewDir.ToString()))
            {
                string str = "Analysis001";
                do
                {
                    str = CreateUniqueName(str);
                }
                while (Directory.Exists(ImageReview.ExperimentFolderPath + str));

                folderName = str;
                sbNewDir = new StringBuilder(ImageReview.ExperimentFolderPath + str);
            }
            string analysisFolderPath = sbNewDir.ToString();
            Directory.CreateDirectory(analysisFolderPath);

            //Prepare mask, dataStore and h5 file for loading:
            CreateStatsManagerROIDS(2, Encoding.ASCII.GetBytes(analysisFolderPath + "\\ROIData.h5"));
            _hdf5Reader = new H5CSWrapper(analysisFolderPath + "\\ROIData.h5");
            OverlayManagerClass.Instance.SaveMaskToPath(analysisFolderPath + "\\ROIMask.raw");
            OverlayManagerClass.Instance.SaveROIs(analysisFolderPath + "\\ROIs.xaml");
            string tittle = ExperimentFolderPath.Substring(Directory.GetParent(Directory.GetParent(ExperimentFolderPath).ToString()).ToString().Length) + folderName + "  (Edit Mode)";
            CreateStatsChartWindow(true, tittle, ExperimentFolderPath + folderName);

            //set the ROI chart x visible range:
            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.UpdataXVisibleRange(1, ZMax * TMax * ZStreamMax * SpMax);
            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SetInLoading(true);

            CreateMultiStatsWindow(tittle);
            _roiCalMode = 1;
            SetMenuBarEnable(false);
            ExecuteROILoadTask(ROIStatsOrigin.CALCULATE);
        }

        private void btRoiLoad_Click()
        {
            try
            {
                Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

                dlg.InitialDirectory = ExperimentFolderPath;
                dlg.DefaultExt = ".h5";
                dlg.Filter = "HDF Files (*.h5)|*.h5";

                Nullable<bool> result = dlg.ShowDialog();
                if (result == true)
                {
                    string fileName = dlg.FileName;
                    string exp_ana_name = GetExpAnalysisName(fileName);
                    _hdf5Reader = new H5CSWrapper(fileName);
                    if (true == _hdf5Reader.SetSavePathAndFileName(fileName))
                    {
                        //Notice for loading ROIs before start:
                        if (null != AnalysisLoaded)
                        {
                            ROIsDirectory = System.IO.Directory.GetParent(fileName).ToString();
                            AnalysisLoaded();
                        }
                        int str_i = Directory.GetParent(Directory.GetParent(Directory.GetParent(fileName).ToString()).ToString()).ToString().Length;
                        int str_e = fileName.LastIndexOf("\\");
                        string ttitle = fileName.Substring(str_i, str_e - str_i) + "  (Review Mode)";
                        CreateStatsChartWindow(false, ttitle, fileName.Substring(0, str_e));

                        //set the ROI chart x visible range:
                        _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.UpdataXVisibleRange(1, ZMax * TMax * ZStreamMax * SpMax);
                        _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SetInLoading(true);

                        CreateMultiStatsWindow(ttitle);

                        _roiCalMode = 2;
                        ExecuteROILoadTask(ROIStatsOrigin.LOAD);
                    }
                }
            }
            catch (Exception e)
            {
                e.ToString();
            }
        }

        private void CancelUpdateProgress(object sender, EventArgs e)
        {
            //cancel the loading:
            _bwStatsLoader.CancelAsync();
        }

        private void CreateLineProfileWindow()
        {
            if (null == _lineProfile && true == _lineProfileActive)
            {
                if (GetNumROI() <= 0)
                {
                    return;
                }

                _lineProfile = new LineProfileWindow.LineProfile(GetColorAssignments(), _imageReview.MaxChannels);
                _lineProfile.Closed += _lineProfile_Closed;
                _lineProfile.LineWidthChange += _lineProfile_LineWidthChange;

                string appSettingsFile = (string.Empty == ExperimentModality) ? ResourceManagerCS.GetApplicationSettingsFileString() : ResourceManagerCS.GetModalityApplicationSettingsFileString(ExperimentModality);
                if (null == ApplicationDoc)
                {
                    XmlDocument appSettings = new XmlDocument();
                    appSettings.Load(appSettingsFile);
                    ApplicationDoc = appSettings;
                }

                XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/LineProfileWindow");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;
                    double val = 0;
                    XmlManager.GetAttribute(ndList[0], ApplicationDoc, "left", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _lineProfile.Left = (int)val;
                    }
                    else
                    {
                        _lineProfile.Left = 0;
                    }
                    XmlManager.GetAttribute(ndList[0], ApplicationDoc, "top", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _lineProfile.Top = (int)val;
                    }
                    else
                    {
                        _lineProfile.Top = 0;
                    }
                    XmlManager.GetAttribute(ndList[0], ApplicationDoc, "width", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _lineProfile.Width = (int)val;
                    }
                    else
                    {
                        _lineProfile.Width = 400;
                    }
                    XmlManager.GetAttribute(ndList[0], ApplicationDoc, "height", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _lineProfile.Height = (int)val;
                    }
                    else
                    {
                        _lineProfile.Height = 400;
                    }
                    XmlManager.SetAttribute(ndList[0], ApplicationDoc, "reset", "0");
                    ApplicationDoc.Save(appSettingsFile);
                }

                _lineProfile.Show();
            }
        }

        private void CreateMultiStatsWindow(string name = "")
        {
            if (null == _multiROIStatsWindows)
            {
                _multiROIStatsWindows = new List<MultiROIStatsUC>();
            }

            int lastNotNullIndx = -1;
            for (int i = _multiROIStatsWindows.Count - 1; i >= 0; i--)
            {
                if (null != _multiROIStatsWindows[i])
                {
                    lastNotNullIndx = i;
                    break;
                }
            }

            _multiROIStatsWindows.Add(new MultiROIStatsUC(name));
            _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Tag = _multiROIStatsWindows.Count - 1;
            _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Closed += _multiROIStats_Closed;

            if (-1 < lastNotNullIndx)
            {
                _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Left = _multiROIStatsWindows[lastNotNullIndx].Left + 20;
                _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Top = _multiROIStatsWindows[lastNotNullIndx].Top + 20;
                _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Width = _multiROIStatsWindows[lastNotNullIndx].Width;
                _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Height = _multiROIStatsWindows[lastNotNullIndx].Height;
            }
            else
            {
                if (null == ApplicationDoc)
                {
                    string appSettingsFile = (string.Empty == ExperimentModality) ? ResourceManagerCS.GetApplicationSettingsFileString() : ResourceManagerCS.GetModalityApplicationSettingsFileString(ExperimentModality);
                    XmlDocument appSettings = new XmlDocument();
                    appSettings.Load(appSettingsFile);
                    ApplicationDoc = appSettings;
                }

                XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIStatsWindow");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;
                    double val = 0;
                    XmlManager.GetAttribute(ndList[0], ApplicationDoc, "left", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Left = (int)val;
                    }
                    else
                    {
                        _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Left = 0;
                    }
                    XmlManager.GetAttribute(ndList[0], ApplicationDoc, "top", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Top = (int)val;
                    }
                    else
                    {
                        _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Top = 0;
                    }
                    XmlManager.GetAttribute(ndList[0], ApplicationDoc, "width", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Width = (int)val;
                    }
                    else
                    {
                        _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Width = 400;
                    }
                    XmlManager.GetAttribute(ndList[0], ApplicationDoc, "height", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Height = (int)val;
                    }
                    else
                    {
                        _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Height = 400;
                    }
                }
            }
            _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].Show();
            ROIStatsTableActive = true;
        }

        //private void CreateOrthogonalBitmap(int index)
        //{
        //    int totalNumOfZstack = this.ZMax - this.ZMin + 1;
        //    PixelFormat pf = PixelFormats.Rgb24;
        //    int step = pf.BitsPerPixel / 8;
        //    int length = this._imageReview.ImageWidth * this._imageReview.ImageHeight;
        //    ushort[] position = new ushort[ImageReview.MaxChannels]; //store the point of grayscale image which used to calculate the RGB value
        //    //Create the XZ orthogonal view
        //    for (int j = 0; j < this._imageReview.ImageWidth; j++)
        //    {
        //        for (int k = 0; k < this._imageReview.DataBufferOffsetIndex.Count; k++)
        //        {
        //            if (VirtualZStack)
        //            {
        //                position[k] = _tiffBufferArray[index][_imageReview.DataBufferOffsetIndex[k] * length + Convert.ToInt32(Math.Floor(BitmapPoint.Y)) * _imageReview.ImageWidth + j];
        //            }
        //            else
        //            {
        //                position[k] = _imageReview.PixelData[_imageReview.DataBufferOffsetIndex[k] * length + Convert.ToInt32(Math.Floor(BitmapPoint.Y)) * _imageReview.ImageWidth + j];
        //            }
        //        }
        //        byte[] color = ImageReview.UpdataPixelDatabyte(position); //calculate the RGB value by current color mapping table
        //        for (int k = 0; k < 3; k++)
        //        {
        //            _pdXZ[index * this._imageReview.ImageWidth * step + j * 3 + k] = color[k]; // assign the RGB value
        //        }
        //    }
        //    //Create the YZ orthogonal view
        //    for (int j = 0; j < this._imageReview.ImageHeight; j++)
        //    {
        //        for (int k = 0; k < this._imageReview.DataBufferOffsetIndex.Count; k++)
        //        {
        //            if (VirtualZStack)
        //            {
        //                position[k] = _tiffBufferArray[index][_imageReview.DataBufferOffsetIndex[k] * length + j * _imageReview.ImageWidth + Convert.ToInt32(Math.Floor(BitmapPoint.X))];
        //            }
        //            else
        //            {
        //                position[k] = this._imageReview.PixelData[_imageReview.DataBufferOffsetIndex[k] * length + j * _imageReview.ImageWidth + Convert.ToInt32(Math.Floor(BitmapPoint.X))];
        //            }
        //        }
        //        byte[] color = ImageReview.UpdataPixelDatabyte(position); //calculate the RGB value by current color mapping table
        //        for (int k = 0; k < 3; k++)
        //        {
        //            _pdYZ[j * totalNumOfZstack * step + index * step + k] = color[k]; // assign the RGB value
        //        }
        //    }
        //}
        private void CreateStatsChartWindow(bool editable, string tittle, string path)
        {
            if (null == _roiStatsCharts)
            {
                _roiStatsCharts = new List<ROIStatsChartWin>();
            }

            int lastNotNullIndx = -1;
            for (int i = _roiStatsCharts.Count - 1; i >= 0; i--)
            {
                if (null != _roiStatsCharts[i])
                {
                    lastNotNullIndx = i;
                    break;
                }
            }

            _roiStatsCharts.Add(new ROIStatsChartWin(tittle));
            _roiStatsCharts[_roiStatsCharts.Count - 1].Tag = _roiStatsCharts.Count - 1;
            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SetTag(_roiStatsCharts.Count - 1);
            _roiStatsCharts[_roiStatsCharts.Count - 1].SetPath(path);
            _roiStatsCharts[_roiStatsCharts.Count - 1].Closed += _roiStatsChart_Closed;
            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SkipGeometricInfo(false);
            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SetChartXLabel("Frame [count]");
            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.ResetChart(editable);
            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SetClockAsXAxis(false);
            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SetFifoVisible(false);

            //reload settings:
            LoadViewModelSettingsDoc();
            XmlNodeList ndListHW = HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");
            List<string> chanNames = new List<string>();
            List<bool> chanEnable = new List<bool>();
            for (int ich = 0; ich < ndListHW.Count; ich++)
            {
                chanNames.Add(ndListHW[ich].Attributes["name"].Value);
                chanEnable.Add(true);
            }
            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SetLegendGroup(2, chanNames.ToArray(), chanEnable.ToArray());
            string[] featureNames = { "mean", "stddev", "min", "max" };     //, "left", "top", "width", "height"
            bool[] featureEnable = { true, true, true, true };              //, false, false, false, false
            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SetLegendGroup(1, featureNames, featureEnable);

            if (-1 < lastNotNullIndx)
            {
                _roiStatsCharts[_roiStatsCharts.Count - 1].Left = _roiStatsCharts[lastNotNullIndx].Left + 20;
                _roiStatsCharts[_roiStatsCharts.Count - 1].Top = _roiStatsCharts[lastNotNullIndx].Top + 20;
                _roiStatsCharts[_roiStatsCharts.Count - 1].Width = _roiStatsCharts[lastNotNullIndx].Width;
                _roiStatsCharts[_roiStatsCharts.Count - 1].Height = _roiStatsCharts[lastNotNullIndx].Height;
            }
            else
            {
                XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");
                if (node != null)
                {
                    string str = string.Empty;
                    double val = 0;
                    XmlManager.GetAttribute(node, ApplicationDoc, "left", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _roiStatsCharts[_roiStatsCharts.Count - 1].Left = (int)val;
                    }
                    else
                    {
                        _roiStatsCharts[_roiStatsCharts.Count - 1].Left = 0;
                    }
                    XmlManager.GetAttribute(node, ApplicationDoc, "top", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _roiStatsCharts[_roiStatsCharts.Count - 1].Top = (int)val;
                    }
                    else
                    {
                        _roiStatsCharts[_roiStatsCharts.Count - 1].Top = 0;
                    }
                    XmlManager.GetAttribute(node, ApplicationDoc, "width", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _roiStatsCharts[_roiStatsCharts.Count - 1].Width = (int)val;
                    }
                    else
                    {
                        _roiStatsCharts[_roiStatsCharts.Count - 1].Width = 400;
                    }
                    XmlManager.GetAttribute(node, ApplicationDoc, "height", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _roiStatsCharts[_roiStatsCharts.Count - 1].Height = (int)val;
                    }
                    else
                    {
                        _roiStatsCharts[_roiStatsCharts.Count - 1].Height = 400;
                    }
                }
            }
            _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.LoadSettings();
            _roiStatsCharts[_roiStatsCharts.Count - 1].Show();
            ROIStatsChartActive = true;
        }

        private string CreateUniqueName(string str)
        {
            Match match = Regex.Match(str, "(.*)([0-9]{3,})");

            if (match.Groups.Count > 2)
            {
                int val = Convert.ToInt32(match.Groups[2].Value);

                val++;

                str = match.Groups[1].Value + String.Format("{0:000}", val);
            }
            else
            {
                str = str + "001";
            }

            return str;
        }

        private void DisplayROIStatsOptions()
        {
            ActiveStatsWinChooser activeStatsWinChooser = new ActiveStatsWinChooser();
            activeStatsWinChooser.DataContext = this;
            activeStatsWinChooser.ShowDialog();

            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");

            if (node != null)
            {
                string str = (true == _roiStatsChartActive) ? "1" : "0";

                XmlManager.SetAttribute(node, ApplicationDoc, "display", str);
            }

            node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIStatsWindow");

            if (node != null)
            {
                string str = (true == _roiStatsTableActive) ? "1" : "0";

                XmlManager.SetAttribute(node, ApplicationDoc, "display", str);
            }

            node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/LineProfileWindow");

            if (node != null)
            {
                string str = (true == _lineProfileActive) ? "1" : "0";

                XmlManager.SetAttribute(node, ApplicationDoc, "display", str);
            }

            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
        }

        private Color[] GetColorAssignments()
        {
            return (Color[])MVMManager.Instance["ImageViewReviewVM", "DefaultChannelColors"];
        }

        private i.ImageCodecInfo GetEncoderInfo(String mimeType)
        {
            i.ImageCodecInfo[] encoders;
            encoders = i.ImageCodecInfo.GetImageEncoders();
            for (int j = 0; j < encoders.Length; ++j)
            {
                if (encoders[j].MimeType == mimeType)
                    return encoders[j];
            }
            return null;
        }

        private string GetExpAnalysisName(string str)
        {
            int init = Directory.GetParent(Directory.GetParent(ExperimentFolderPath).ToString()).ToString().Length;
            int len = str.LastIndexOf("\\") - init;
            return str.Substring(init, len);
        }

        private bool GetExperimentStatus(ref string status)
        {
            bool ret = false;
            if (File.Exists(ExperimentXMLPath))
            {
                XmlDocument experimentDoc = LoadDocAsReadOnly(ExperimentXMLPath);
                XmlNodeList ndList = experimentDoc.SelectNodes("/ThorImageExperiment/ExperimentStatus");
                if (ndList.Count > 0)
                {
                    ret = XmlManager.GetAttribute(ndList[0], experimentDoc, "value", ref status);
                }
            }

            return ret;
        }

        // Get the file name
        private List<string> GetFileNames(int zValue, int scanAreaID = 0)
        {
            List<string> fileNames = new List<string>();

            if (FileFormatMode == ImageReview.FormatMode.EXPERIMENT)
            {
                int sampleSiteIndex = this.SampleSiteIndex;
                int subIndex = this.SubTileIndex;
                int timeIndex;
                timeIndex = (CaptureModes.HYPERSPECTRAL == CaptureMode) ? SpValue : TValue;

                if (((ZStreamMode > 0) && (ZStreamMax > 1) && (CaptureModes.T_AND_Z == CaptureMode)) ||
                    (ImageInfo.isZFastEnabled && (CaptureFile.FILE_RAW == ImageInfo.imageType)))
                {
                    timeIndex = (CaptureModes.HYPERSPECTRAL == CaptureMode) ?
                        ((SpValue - 1) * (ZMax + ImageInfo.flybackFrames) + (ZValue - 1)) * ZStreamMax + ZStreamValue :
                        ((TValue - 1) * (ZMax + ImageInfo.flybackFrames) + (ZValue - 1)) * ZStreamMax + ZStreamValue;
                }

                ImageBlkIndex = timeIndex - 1;
                if (HardwareDoc == null)
                {
                    LoadViewModelSettingsDoc();
                }

                string imgNameFormat = ImageNameFormat;
                XmlNodeList ndListHW = HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");
                if (null != WavelengthNames)
                {
                    for (int i = 0; i < WavelengthNames.Length; i++)
                    {
                        if ((ExperimentFolderPath != null))
                        {
                            StringBuilder sbTemp = new StringBuilder();
                            switch (ImageInfo.imageType)
                            {
                                case CaptureFile.FILE_BIG_TIFF:
                                    {
                                        String temp = string.Format("{0}", "Image");
                                        DirectoryInfo di = new DirectoryInfo(ExperimentFolderPath);
                                        FileInfo[] fi = di.GetFiles(temp + "*.tif*");
                                        if (fi == null || fi.Length == 0) return null;
                                        sbTemp.AppendFormat(fi[0].FullName);
                                    }
                                    break;
                                case CaptureFile.FILE_RAW:
                                    {
                                        if (ExperimentData.IsmROICapture)
                                        {
                                            String temp = "Image_scan_" + sampleSiteIndex.ToString() + "_region_" + scanAreaID.ToString();
                                            DirectoryInfo di = new DirectoryInfo(ExperimentFolderPath);
                                            FileInfo[] fi = di.GetFiles(temp + "*.raw*");
                                            if (fi == null || fi.Length == 0) return null;
                                            sbTemp.AppendFormat(fi[0].FullName);
                                        }
                                        else
                                        {
                                            String temp = string.Format("{0}{1}{2}",
                                                                "Image",
                                                                "_" + sampleSiteIndex.ToString(imgNameFormat),
                                                                "_" + subIndex.ToString(imgNameFormat));
                                            DirectoryInfo di = new DirectoryInfo(ExperimentFolderPath);
                                            FileInfo[] fi = di.GetFiles(temp + "*.raw*");
                                            if (fi == null || fi.Length == 0) return null;
                                            sbTemp.AppendFormat(fi[0].FullName);
                                        }
                                    }
                                    break;
                                case CaptureFile.FILE_TIFF:
                                    {
                                        if (CaptureModes.HYPERSPECTRAL == CaptureMode)
                                        {
                                            sbTemp.AppendFormat("{0}{1}{2}{3}{4}{5}",
                                                                    ExperimentFolderPath + "\\",
                                                                    WavelengthNames[i],
                                                                    "_" + timeIndex.ToString(imgNameFormat),
                                                                    "_" + subIndex.ToString(imgNameFormat),
                                                                    "_" + zValue.ToString(imgNameFormat),
                                                                    "_" + sampleSiteIndex.ToString(imgNameFormat) + ".tif");
                                        }
                                        else
                                        {
                                            sbTemp.AppendFormat("{0}{1}{2}{3}{4}{5}",
                                                                    ExperimentFolderPath + "\\",
                                                                    WavelengthNames[i],
                                                                    "_" + sampleSiteIndex.ToString(imgNameFormat),
                                                                    "_" + subIndex.ToString(imgNameFormat),
                                                                    "_" + zValue.ToString(imgNameFormat),
                                                                    "_" + timeIndex.ToString(imgNameFormat) + ".tif");
                                        }
                                    }
                                    break;
                                default:
                                    break;
                            }

                            string strTemp = sbTemp.ToString();

                            //For sequential capture, ignore the cross check with HardwareSettings for the name
                            if (SequentialModeEnabled)
                            {
                                fileNames.Add(strTemp);
                            }
                            else
                            {

                                for (int j = 0; j < ndListHW.Count; j++)
                                {
                                    if (WavelengthNames[i].Equals(ndListHW[j].Attributes["name"].Value))
                                    {
                                        fileNames.Add(strTemp);
                                        break;
                                    }
                                    else
                                    {
                                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Channel name " + WavelengthNames[i] + " doesn't match expected name in HardwareSettings.xml: " + ndListHW[j].Attributes["name"].Value);
                                    }
                                }
                            }
                            if (false == File.Exists(strTemp))
                            {
                                return fileNames;
                            }
                        }
                    }
                }
            }
            else
            {
                fileNames.Add(ImagePathandName);
            }

            return fileNames;
        }

        private List<int> GetGray16MarkLocations(d.Point[] linePoints, d.RectangleF markRect, double scaleLen, d.StringFormat drawMarkFormat)
        {
            int imgByBytes = ExperimentData.ImageInfo.pixelX * ExperimentData.ImageInfo.pixelY * 2;
            double relativeFactor = ExperimentData.ImageInfo.pixelX / 20.0;
            byte[] bytes48 = new byte[3 * imgByBytes];
            d.Bitmap bmp = new d.Bitmap(ExperimentData.ImageInfo.pixelX,
                                        ExperimentData.ImageInfo.pixelY,
                                        i.PixelFormat.Format48bppRgb);
            using (d.Graphics g = d.Graphics.FromImage(bmp))
            {
                g.DrawLines(new d.Pen(d.Brushes.White, 4), linePoints);
                g.DrawString(((int)scaleLen).ToString(), new d.Font("Arial", (int)(relativeFactor * 0.77)), d.Brushes.White, markRect, drawMarkFormat);
            }
            i.BitmapData data = bmp.LockBits(new d.Rectangle(d.Point.Empty, bmp.Size),
                     i.ImageLockMode.WriteOnly,
                     i.PixelFormat.Format48bppRgb);
            Marshal.Copy(data.Scan0, bytes48, 0, 3 * imgByBytes);
            List<int> markLocations = new List<int>();
            for (int k = 0; k < imgByBytes / 2; k++)
            {
                if (bytes48[6 * k + 1] + bytes48[6 * k] != 0) markLocations.Add(k);
            }

            return markLocations;
        }

        private long GetTotalFreeSpace(string driveName)
        {
            foreach (DriveInfo drive in DriveInfo.GetDrives())
            {
                if (drive.IsReady && drive.Name == driveName)
                {
                    return drive.TotalFreeSpace;
                }
            }
            return -1;
        }

        private byte[] InscribeScaleToImagesRaw(FileInfo fi, d.Point[] linePoints, d.RectangleF markRect, double scaleLen, d.StringFormat drawMarkFormat, int step)
        {
            List<int> markLocations = GetGray16MarkLocations(linePoints, markRect, scaleLen, drawMarkFormat);

            int offset = 0;
            byte[] bytes = File.ReadAllBytes(fi.FullName);
            CompletedImageCount += (bytes.Length / step);
            while (offset + step <= fi.Length)
            {
                foreach (int j in markLocations)
                {
                    bytes[offset + 2 * j] = 0x00;
                    bytes[offset + 2 * j + 1] = 0x3F;
                }
                offset += step;
                CompletedImageCount++;
            }
            return bytes;
        }

        private List<byte[]> InscribeScaleToImagesTiff(FileInfo fi, d.Point[] linePoints, d.RectangleF markRect, double scaleLen, d.StringFormat drawMarkFormat)
        {
            List<byte[]> images = new List<byte[]>();
            List<int> markLocations = GetGray16MarkLocations(linePoints, markRect, scaleLen, drawMarkFormat);

            using (Tiff input = Tiff.Open(fi.FullName, "r"))
            {
                int pages = input.NumberOfDirectories();

                for (short k = 0; k < pages; k++)
                {
                    input.SetDirectory(k);
                    int w = input.GetField(TiffTag.IMAGEWIDTH)[0].ToInt();
                    int h = input.GetField(TiffTag.IMAGELENGTH)[0].ToInt();
                    byte[] buf = new byte[w * h * 2];
                    int strips = input.NumberOfStrips();
                    int offset = 0;
                    for (short l = 0; l < strips; l++)
                    {
                        int r = input.ReadEncodedStrip(l, buf, offset, w * h * 2 - offset);
                        if (r > 0) offset += r;
                    }
                    images.Add(buf);
                    CompletedImageCount++;
                }

                input.Close();
            }

            foreach (byte[] iSrc in images)
            {
                foreach (int j in markLocations)
                {
                    if (2 * j >= iSrc.Length) break;
                    iSrc[2 * j] = 0x00;
                    iSrc[2 * j + 1] = 0x3F;
                }
            }

            return images;
        }

        private void Instance_OverlaymROISelectedEvent(int obj)
        {
            if (_mROIShowOverlays)
            {
                _selectedScanArea = obj;
                OnPropertyChanged("SelectedScanArea");
                OnPropertyChanged("SelectedmROIIndex");

                MVMManager.Instance["ImageViewReviewVM", "mROIPriorityIndex"] = obj - 1;
            }
        }

        private void LaunchWindowsExplorer()
        {
            string explorerExe = "explorer.exe";
            string folder = _imageReview.ExperimentFolderPath;
            string explorerCmdText = string.Format("/C \"\"{0}\" \"{1}\"", explorerExe, folder);

            try
            {
                System.Diagnostics.Process process = new System.Diagnostics.Process();
                System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
                startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
                startInfo.FileName = "cmd.exe";

                startInfo.Arguments = explorerCmdText;
                process.StartInfo = startInfo;
                process.Start();
                //    _processOn = true;
                process.WaitForExit();
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        double LoadImages()
        {
            if (ExperimentData.IsmROICapture)
            {
                for (int j = 0; j < ExperimentData.mROIs?.Count; j++)
                {
                    List<string> fileNames = GetFileNames(ZValue, ExperimentData.mROIs[j].ScanAreaID);
                    if (!(fileNames?.Count > 0))
                    {
                        continue;
                    }
                    else
                    {
                        int f = 0;
                        for (int i = 0; i < fileNames.Count; i++)
                        {
                            if ((fileNames[i] != null) && (File.Exists(fileNames[i]))) f++;
                        }
                        if (f == 0)
                        {
                            continue;
                        }
                    }

                    bool bEmpty = true;
                    foreach (string file in fileNames)
                    {
                        if (!string.IsNullOrEmpty(file))
                        {
                            bEmpty = false;
                            break;
                        }
                    }

                    if (bEmpty)
                        continue;

                    MVMManager.Instance["ImageViewReviewVM", "BitmapLoaded"] = false;

                    UpdateChannelData(fileNames, ZValue - 1, ((CaptureModes.HYPERSPECTRAL == CaptureMode) ? SpValue - 1 : TValue - 1), ExperimentData.mROIs[j].ScanAreaID, j);

                    DateTime startTime = DateTime.Now;
                    TimeSpan elapsedTime = DateTime.Now - startTime;
                    while (false == (bool)MVMManager.Instance["ImageViewReviewVM", "BitmapLoaded", (object)true] && elapsedTime.TotalMilliseconds < ImageReview.BITMAP_UPDATE_TIMEOUT && false == _stopRequested)
                    {
                        elapsedTime = DateTime.Now - startTime;
                    }
                }
                return 0;
            }
            else
            {
                List<string> fileNames = GetFileNames(ZValue);
                if (null == fileNames || 0 == fileNames.Count)
                {
                    return -1;
                }
                else
                {
                    int f = 0;
                    for (int i = 0; i < fileNames.Count; i++)
                    {
                        if ((fileNames[i] != null) && (File.Exists(fileNames[i]))) f++;
                    }
                    if (f == 0)
                    {
                        return -1;
                    }
                }

                bool bEmpty = true;
                foreach (string file in fileNames)
                {
                    if (!string.IsNullOrEmpty(file))
                    {
                        bEmpty = false;
                        break;
                    }
                }

                if (bEmpty)
                    return -1;

                UpdateChannelData(fileNames, ZValue - 1, ((CaptureModes.HYPERSPECTRAL == CaptureMode) ? SpValue - 1 : (TValue - 1 < 0 ? 0 : TValue - 1)));
                if (_lineProfileActive)
                {
                    LoadLineProfileData();
                }
                return 0;
            }
        }

        private void LoadOMETiffInProcess(string exePath, string imgFile)
        {
            Process.Start(exePath, imgFile);
        }

        private void MEndValueMinus()
        {
            MEndValue -= 1;
        }

        private void MEndValuePlus()
        {
            MEndValue += 1;
        }

        private void MStartValueMinus()
        {
            MStartValue -= 1;
        }

        private void MStartValuePlus()
        {
            MStartValue += 1;
        }

        private void PersistLineProfileWindowSettings()
        {
            if (null == _lineProfile)
            {
                return;
            }
            string appSettingsFile = (string.Empty == ExperimentModality) ? ResourceManagerCS.GetApplicationSettingsFileString() : ResourceManagerCS.GetModalityApplicationSettingsFileString(ExperimentModality);
            if (null == ApplicationDoc)
            {
                XmlDocument appSettings = new XmlDocument();
                appSettings.Load(appSettingsFile);
                ApplicationDoc = appSettings;
            }

            XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/LineProfileWindow");
            if (ndList.Count > 0)
            {
                XmlNode node = ndList[0];
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "left", ((int)Math.Round(_lineProfile.Left)).ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "top", ((int)Math.Round(_lineProfile.Top)).ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "width", ((int)Math.Round(_lineProfile.Width)).ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "height", ((int)Math.Round(_lineProfile.Height)).ToString());
            }
            //save the information:
            ApplicationDoc.Save(appSettingsFile);
            _lineProfile = null;
        }

        private string ReadLast3dOutputPath()
        {
            if (null == ApplicationDoc)
            {
                string appSettingsFile = (string.Empty == ExperimentModality) ? ResourceManagerCS.GetApplicationSettingsFileString() : ResourceManagerCS.GetModalityApplicationSettingsFileString(ExperimentModality);
                XmlDocument appSettings = new XmlDocument();
                appSettings.Load(appSettingsFile);
                ApplicationDoc = appSettings;
            }
            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/Last3dOutputPath");
            if (node == null)
            {
                return Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString();
            }
            else
            {
                if (node.Attributes.GetNamedItem("path") != null)
                {
                    return node.Attributes["path"].Value;
                }
                else
                {
                    return Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString();
                }
            }
        }

        private void SaveImagesAsRawFile(FileInfo fi, ref byte[] images, int imgSizeInBytes, string savePath)
        {
            using (var stream = new FileStream(savePath + "\\" + fi.Name, FileMode.Create))
            {
                stream.Write(images, 0, images.Length);
                CompletedImageCount += (images.Length / imgSizeInBytes);
            }
        }

        private void SaveImagesAsTiffFile(FileInfo fi, ref List<byte[]> images, string savePath)
        {
            if (images.Count < 1) return;
            using (Tiff output = Tiff.Open(savePath + "\\" + fi.Name, "w"))
            {
                for (int i = 0; i < images.Count; i++)
                {
                    output.SetField(TiffTag.IMAGEWIDTH, ExperimentData.ImageInfo.pixelX);
                    output.SetField(TiffTag.IMAGELENGTH, ExperimentData.ImageInfo.pixelY);
                    output.SetField(TiffTag.SAMPLESPERPIXEL, 1);
                    output.SetField(TiffTag.BITSPERSAMPLE, 16);
                    output.SetField(TiffTag.ORIENTATION, BitMiracle.LibTiff.Classic.Orientation.TOPLEFT);
                    output.SetField(TiffTag.ROWSPERSTRIP, ExperimentData.ImageInfo.pixelY);
                    output.SetField(TiffTag.XRESOLUTION, 10000 / ExperimentData.PixelSizeUM.PixelWidthUM);
                    output.SetField(TiffTag.YRESOLUTION, 10000 / ExperimentData.PixelSizeUM.PixelHeightUM);
                    output.SetField(TiffTag.RESOLUTIONUNIT, ResUnit.CENTIMETER);
                    output.SetField(TiffTag.PLANARCONFIG, PlanarConfig.CONTIG);
                    output.SetField(TiffTag.PHOTOMETRIC, Photometric.MINISBLACK);
                    output.SetField(TiffTag.COMPRESSION, Compression.NONE);
                    output.SetField(TiffTag.FILLORDER, FillOrder.MSB2LSB);

                    output.WriteEncodedStrip(0, images[i], 0, images[0].Length);

                    output.WriteDirectory();
                    CompletedImageCount++;
                }
                output.Close();
            }
        }

        //private void ScanAreaIDMinus()
        //{
        //    _scanAreaIndex = Math.Min(ImageInfo.scanAreaIDList.Count - 1, Math.Max(0, _scanAreaIndex - 1));
        //    ScanAreaID = ImageInfo.scanAreaIDList[_scanAreaIndex].RegionID;
        //}
        //private void ScanAreaIDPlus()
        //{
        //    _scanAreaIndex = Math.Min(ImageInfo.scanAreaIDList.Count - 1, Math.Max(0, _scanAreaIndex + 1));
        //    ScanAreaID = ImageInfo.scanAreaIDList[_scanAreaIndex].RegionID;
        //}
        void SetMenuBarEnable(bool status)
        {
            ChangeEvent changeEvent = new ChangeEvent();
            changeEvent.ModuleName = "ImageReview";

            changeEvent.IsChanged = status;

            if (null != _eventAggregator)
            {
                //command published to change the status of the menu buttons in the Menu Control
                _eventAggregator.GetEvent<MenuModuleChangeEvent>().Publish(changeEvent);
            }
        }

        private void SpValueMinus()
        {
            SpValue -= 1;
        }

        private void SpValuePlus()
        {
            SpValue += 1;
        }

        private void TValueMinus()
        {
            RXFlag = false;
            TValue -= 1;
            RXFlag = true;
        }

        private void TValuePlus()
        {
            RXFlag = false;
            TValue += 1;
            RXFlag = true;
        }

        private void ZStreamValueMinus()
        {
            ZStreamValue -= 1;
        }

        private void ZStreamValuePlus()
        {
            ZStreamValue += 1;
        }

        private void ZValueMinus()
        {
            RXFlagZ = false;
            ZValue -= 1;
            RXFlagZ = true;
        }

        private void ZValuePlus()
        {
            ZValue += 1;
        }

        private void _bwStatsLoader_DoWork(object sender, DoWorkEventArgs e)
        {
            string[] fileNamesToRead = new string[4];
            const int HUNDRED_PERCENT = 100;
            int percent = 0;
            string pattern = string.Empty;

            H5NodeInfo h5NodeInfo = new H5NodeInfo();
            bool multiStatsUpdated = false;
            //For efficiency allocate dataIn and dataOut before the forloop starts
            //The image size and the number of channels won't change during the calculation
            IntPtr dataIn = IntPtr.Zero;
            short[] dataOut = null;
            if (1 == _roiCalMode)       //1: calculation, 2: load
            {
                if (WavelengthNames.Count() == 1)
                {
                    dataIn = Marshal.AllocHGlobal(ImageReview.ImageWidth * ImageReview.ImageHeight * 2);
                    dataOut = new short[ImageReview.ImageWidth * ImageReview.ImageHeight];
                }
                else
                {
                    dataIn = Marshal.AllocHGlobal(ImageReview.ImageWidth * ImageReview.ImageHeight * ImageReview.MaxChannels * 2);
                    dataOut = new short[ImageReview.ImageWidth * ImageReview.ImageHeight * ImageReview.MaxChannels];
                }
            }

            if (null != _roiStatsCharts)
            {
                if (null != _roiStatsCharts[_roiStatsCharts.Count - 1])
                {
                    //Set flag to false to inform ROIChart that data loading is not complete
                    _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SetDataStoreLoadComplete(false);
                }
            }

            //reload hw settings:
            LoadViewModelSettingsDoc();
            XmlNodeList ndListHW = HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

            string status = string.Empty;
            while ("Complete" != status && "Stopped" != status && "NoStatus" != status)
            {

                LoadExpFolder(ImageReview.ExperimentFolderPath);
                if (false == GetExperimentStatus(ref status))
                {
                    status = "NoStatus";
                }
                Application.Current.Dispatcher.InvokeOrExecute(() =>
                {
                    if (CaptureModes.HYPERSPECTRAL == CaptureMode)
                    {
                        _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.UpdataXVisibleRange(1, ZMax * SpMax * ZStreamMax);
                    }
                    else
                    {
                        _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.UpdataXVisibleRange(1, ZMax * TMax * ZStreamMax);
                    }
                });

                int totalFrames = 0;

                if ("Complete" != status && "Stopped" != status)
                {
                    totalFrames = (CaptureModes.HYPERSPECTRAL == CaptureMode) ? ZMax * SpMax * ZStreamMax - 1 : ZMax * TMax * ZStreamMax - 1;
                }
                else
                {
                    totalFrames = (CaptureModes.HYPERSPECTRAL == CaptureMode) ? ZMax * SpMax * ZStreamMax : ZMax * TMax * ZStreamMax;
                }

                if (totalFrames <= 0)
                { return; }

                //start stats processing:
                for (int fid = 0; fid < totalFrames; fid++)
                {
                    int time = fid / ZMax;
                    int zSlice = fid - time * ZMax;

                    if (CaptureFile.FILE_TIFF == ImageInfo.imageType)
                    {
                        //extract all channels' file name:
                        pattern = WavelengthNames[PrimaryChannelIndex] + "_(.*)_(.*)_(.*)_(.*).tif";
                        Regex ex = new Regex(pattern, RegexOptions.IgnoreCase);
                        Match match = ex.Match(PrimaryChannelFileNames[(this.SubTileIndex - 1) * totalFrames + fid].FileName);
                        for (int i = 0; i < WavelengthNames.Count(); i++)
                        {
                            string chanFileName = string.Empty;
                            if (i != PrimaryChannelIndex)
                            {
                                string newFilename = WavelengthNames[i] + "_" +
                                                     match.Groups[1].ToString() + "_" +
                                                     match.Groups[2].ToString() + "_" +
                                                     match.Groups[3].ToString() + "_" +
                                                     match.Groups[4].ToString() + ".tif";
                                List<String> tmpfName = GetFileList(ImageReview.ExperimentFolderPath, newFilename, "*.tif").ToList();
                                if (tmpfName.Count > 0)
                                {
                                    chanFileName = tmpfName[0];
                                }
                            }
                            else
                            {
                                chanFileName = PrimaryChannelFileNames[(this.SubTileIndex - 1) * totalFrames + fid].FileName;
                            }

                            //determine fileNamesToRead based on 1x or 4x channels:
                            if ((WavelengthNames.Count() == 1))
                            {
                                fileNamesToRead[0] = chanFileName;
                            }
                            else
                            {
                                for (int ich = 0; ich < ndListHW.Count; ich++)
                                {
                                    if (WavelengthNames[i].Equals(ndListHW[ich].Attributes["name"].Value))
                                    {
                                        fileNamesToRead[ich] = chanFileName;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if (1 == _roiCalMode)       //1: calculation, 2: load
                    {
                        try
                        {
                            String temp, fname;
                            DirectoryInfo di = new DirectoryInfo(ExperimentFolderPath);
                            int channelCount = (1 == WavelengthNames.Count()) ? 1 : ImageReview.MaxChannels;

                            switch (ImageInfo.imageType)
                            {
                                case CaptureFile.FILE_BIG_TIFF:
                                    //load OME once
                                    if (0 == fid)
                                    {
                                        int regionCount = 0, width = 0, height = 0, chCount = 0, zMaxCount = 0, timeCount = 0, specCount = 0;
                                        temp = string.Format("{0}", "Image");
                                        fname = (di.GetFiles(temp + "*.tif*")[0].FullName);
                                        ImageReview.GetImageStoreInfo(fname, ScanAreaID, ref regionCount, ref width, ref height, ref chCount, ref zMaxCount, ref timeCount, ref specCount);
                                    }
                                    if (CaptureModes.HYPERSPECTRAL == CaptureMode)
                                    {
                                        ImageReview.ReadImageStoreData(dataIn, channelCount, ImageInfo.pixelX, ImageInfo.pixelY, zSlice, 0, time);
                                    }
                                    else
                                    {
                                        ImageReview.ReadImageStoreData(dataIn, channelCount, ImageInfo.pixelX, ImageInfo.pixelY, zSlice, time, 0);
                                    }
                                    break;
                                case CaptureFile.FILE_RAW:
                                    temp = string.Format("{0}{1}{2}",
                                        "Image",
                                "_" + this.SampleSiteIndex.ToString(ImageNameFormat),
                                "_" + this.SubTileIndex.ToString(ImageNameFormat));
                                    fname = (di.GetFiles(temp + "*.raw*")[0].FullName);
                                    if (1 == WavelengthNames.Count())
                                    {
                                        ImageReview.ReadChannelImagesRaw(ref dataIn, 1, fname, ImageInfo.fileChs, WavelengthSelectedIndex, ImageInfo.frameSize, fid + ((int)(fid / (ImageInfo.zSteps))) * (ImageInfo.flybackFrames));
                                    }
                                    else
                                    {
                                        ImageReview.LoadImageIntoBufferFromRawFile(dataIn, fname, ImageInfo.channelEnabled, ImageReview.MaxChannels, zSlice, time, ImageInfo.pixelX, ImageInfo.pixelY * ExperimentData.NumberOfPlanes, ZMax, ExperimentData.OnlyEnabledChannels != 1);
                                    }
                                    break;
                                case CaptureFile.FILE_TIFF:
                                    ImageReview.ReadChannelImages(fileNamesToRead, channelCount, ref dataIn, ImageReview.ImageWidth, ImageReview.ImageHeight);
                                    break;
                                default:
                                    break;
                            }

                            Marshal.Copy(dataIn, dataOut, 0, ImageReview.ImageWidth * ImageReview.ImageHeight * channelCount);
                            FrameInfoStruct frameInfo = new FrameInfoStruct();
                            frameInfo.bufferType = (int)BufferType.INTENSITY;
                            frameInfo.imageWidth = ImageReview.ImageWidth;
                            frameInfo.imageHeight = ImageReview.ImageHeight;
                            frameInfo.numberOfPlanes = 1;
                            ComputeStats(dataOut, frameInfo, ChannelEnabled, 0, 1, 0);
                        }
                        catch (Exception ex)
                        {
                            ex.ToString();
                        }
                    }
                    //user request to cancel:
                    if (_bwStatsLoader.CancellationPending == true)
                    {
                        if (1 == _roiCalMode) { Marshal.FreeHGlobal(dataIn); }
                        _hdf5Reader.DestroyH5();
                        _hdf5Reader = null;
                        return;
                    }

                    if (1 == _roiCalMode)
                    {
                        //Wait for stats calculation and storage before requesting data
                        while (0 == IsStatsComplete())
                        {
                            System.Threading.Thread.Sleep(1);
                        }
                    }
                    //***   data update based on DataStore callback is not always latest, load directly from file instead.  ***//
                    //RequestROIData();
                    if (fid == 0)
                    {
                        //get group dataset names once:
                        if (true == _hdf5Reader.OpenH5())
                        {
                            _hdf5Reader.GetGroupDatasetName("/", ref h5NodeInfo);
                            _hdf5Reader.CloseH5();
                        }
                    }
                    if (null == h5NodeInfo.groups)
                    {
                        if (1 == _roiCalMode) { Marshal.FreeHGlobal(dataIn); }
                        _hdf5Reader.DestroyH5();
                        _hdf5Reader = null;
                        MessageBox.Show("The HDF5 file you have chosen is corrupted or empty", "Empty file",
                                        MessageBoxButton.OK, MessageBoxImage.Exclamation,
                                        MessageBoxResult.OK, MessageBoxOptions.DefaultDesktopOnly);
                        return;
                    }
                    ulong dataSize = 0;
                    if (true == _hdf5Reader.OpenH5())
                    {
                        if (true == _hdf5Reader.CheckH5GrpDataset("/" + h5NodeInfo.groups[0], "/" + h5NodeInfo.groups[0], ref dataSize))
                        {
                            if ((ulong)fid > dataSize)
                            { break; }
                            double[] readData = new double[1];
                            double[] value = new double[h5NodeInfo.groups.Length];
                            for (int j = 0; j < h5NodeInfo.groups.Length; j++)
                            {
                                readData = _hdf5Reader.ReadDoubleData("/" + h5NodeInfo.groups[j], "/" + h5NodeInfo.groups[j], (ulong)fid, (ulong)1);
                                value[j] = readData[0];
                            }
                            Application.Current.Dispatcher.Invoke(() =>
                            {

                                if (true == InUpdateROIStatsMut.WaitOne(-1))
                                {
                                    try
                                    {
                                        if (null != _roiStatsCharts)
                                        {
                                            if (null != _roiStatsCharts[_roiStatsCharts.Count - 1])
                                            {
                                                _roiStatsCharts[_roiStatsCharts.Count - 1].SetData(h5NodeInfo.groups, value, false);

                                                if (false == multiStatsUpdated && null != _multiROIStatsWindows && _roiStatsCharts.Count == _multiROIStatsWindows.Count)
                                                {
                                                    //Set the MultiROIStats view to the first dataset once:
                                                    if (null != _multiROIStatsWindows[_multiROIStatsWindows.Count - 1])
                                                    {
                                                        _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].SetData(h5NodeInfo.groups, value);
                                                        _multiROIStatsWindows[_multiROIStatsWindows.Count - 1].SetArithmeticsData(_roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.ArithmeticNames, _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.ArithmeticStats);
                                                        multiStatsUpdated = true;
                                                    }
                                                }
                                            }
                                        }
                                        InUpdateROIStatsMut.ReleaseMutex();
                                    }
                                    catch (Exception exception)
                                    {
                                        exception.ToString();
                                        InUpdateROIStatsMut.ReleaseMutex();
                                    }
                                }

                            });
                        }
                        _hdf5Reader.CloseH5();
                    }
                    //report progress:
                    percent = (int)(fid * HUNDRED_PERCENT / totalFrames);
                    Interlocked.Exchange(ref _progressPercentage, percent);
                }
            }
            //done:
            if (1 == _roiCalMode) { Marshal.FreeHGlobal(dataIn); }
            _hdf5Reader.DestroyH5();
            _hdf5Reader = null;
        }

        private void _bwStatsLoader_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (null != _roiStatsCharts)
            {
                if (null != _roiStatsCharts[_roiStatsCharts.Count - 1])
                {
                    _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.ZoomExtend();

                    //Set flag to false to inform ROIChart that data loading is complete
                    _roiStatsCharts[_roiStatsCharts.Count - 1].ROIChart.SetDataStoreLoadComplete(true);
                }
            }
            SetMenuBarEnable(true);
            PanelsEnable = true;
            OnPropertyChanged("PanelsEnable");
            _bwStatsLoaderDone = true;
        }

        void _bw_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;

            if (worker.CancellationPending)
            {
                return;
            }

            List<string> fileNames = new List<string>();
            _completedImageCount = 0;
            _totalImageCount = 1;

            for (int n = 0; n < _movieFileNameList[0].Length; n++)
            {
                int zID = (CurrentMovieParameterEnum.Z == currentMovieParameter) ? n : ZValue - 1;
                int tID = (CurrentMovieParameterEnum.Z == currentMovieParameter) ? TValue - 1 : n;

                switch (ImageInfo.imageType)
                {
                    case CaptureFile.FILE_BIG_TIFF:
                    case CaptureFile.FILE_RAW:
                        //passthrough filename
                        fileNames.Add(_movieFileNameList[0][0]);
                        break;
                    case CaptureFile.FILE_TIFF:
                        //allocate a string array to accomodate the maximum number of channels
                        //the unused element will be left empty
                        for (int i = 0; i < _movieFileNameList.Length; i++)
                        {
                            string str = _movieFileNameList[i][n];

                            for (int j = 0; j < _imageReview.HardwareChannelNames.Count(); j++)
                            {
                                string hwChanName = _imageReview.HardwareChannelNames[j];
                                if (str.Contains(hwChanName))
                                {
                                    fileNames.Add(_movieFileNameList[i][n]);
                                    break;
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }

                MVMManager.Instance["ImageViewReviewVM", "BitmapLoaded"] = false;
                UpdateChannelData(fileNames, zID, tID);
                DateTime startTime = DateTime.Now;
                TimeSpan elapsedTime = DateTime.Now - startTime;
                while (false == (bool)MVMManager.Instance["ImageViewReviewVM", "BitmapLoaded", (object)true] /*&& elapsedTime.TotalMilliseconds < ImageReview.BITMAP_UPDATE_TIMEOUT */&& false == ImageReviewViewModel._stopRequested)
                {
                    elapsedTime = DateTime.Now - startTime;
                }
                WriteableBitmap wBmp = (WriteableBitmap)MVMManager.Instance["ImageViewReviewVM", "Bitmap", true];
                d.Bitmap bmp = null;
                Application.Current.Dispatcher.Invoke(new Action(() => 
                {
                    using (MemoryStream outStream = new MemoryStream())
                    {
                        BitmapEncoder enc = new BmpBitmapEncoder();
                        enc.Frames.Add(BitmapFrame.Create((BitmapSource)wBmp));
                        enc.Save(outStream);
                        bmp = new d.Bitmap(outStream);
                    }

                }));
                    //TODO:IV get bitmap from imageViewVM
                    //System.Drawing.Bitmap bmp = BitmapFromSource(_bitmap);
                    ////create avi at beginning:
                if (0 == n)
                {
                    _aviManager = new AviManager(_movieFilePath, false);
                    _aviStream = _aviManager.AddVideoStream(true, _movieFPS, bmp);
                }
                else
                {
                    _aviStream.AddFrame(bmp);
                }
                bmp.Dispose();

                int percent = (int)(100.0 * ((double)n / _movieFileNameList[0].Length));
                worker.ReportProgress(percent);
                _completedImageCount = n + 1;
                _totalImageCount = _movieFileNameList[0].Length;
                fileNames.Clear();

            }
        }

        void _bw_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            OnPropertyChanged("CompletedImageCount");
            OnPropertyChanged("TotalImageCount");
        }

        void _bw_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (null != _aviManager)
            {
                _aviManager.Close();
                _aviManager = null;
            }
            System.Diagnostics.Process.Start(_movieFilePath);

            PanelsEnable = true;
            SetMenuBarEnable(true);

            if (e.Cancelled)
            { }
            else if (null != e.Error)
            {
                if (e.Error.ToString().Contains("Exception in AVIFileOpen:"))
                {
                    MessageBox.Show("Ensure the file you are trying to write to is not being used by another application.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
                else
                {
                    MessageBox.Show(e.Error.ToString(), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        void _imageReview_LineProfileChanged(object sender, EventArgs e)
        {
            Application.Current.Dispatcher.Invoke((Action)(() =>
            {
                if (0 == _imageReview.LineProfileData.channelEnable ||
                   0 == _imageReview.LineProfileData.profileDataX.Length)
                {
                    if (null != _lineProfile)
                    {
                        _lineProfile.Close();
                    }
                }
                else
                {
                    CreateLineProfileWindow();
                    if (null != _lineProfile)
                    {
                        _lineProfile.SetData(_imageReview.LineProfileData);
                    }
                }
            }));
        }

        void _lineProfile_Closed(object sender, EventArgs e)
        {
            PersistLineProfileWindowSettings();
            _lineProfileActive = false;
            OnPropertyChanged("LineProfileActive");
        }

        void _lineProfile_LineWidthChange(int lineWidth)
        {
            ImageReview.SetLineProfileLineWidth(lineWidth);
        }

        void _multiROIStats_Closed(object sender, EventArgs e)
        {
            int indx = (int)(sender as MultiROIStatsUC).Tag;
            string appSettingsFile = (string.Empty == ExperimentModality) ? ResourceManagerCS.GetApplicationSettingsFileString() : ResourceManagerCS.GetModalityApplicationSettingsFileString(ExperimentModality);
            if (null == ApplicationDoc)
            {
                XmlDocument appSettings = new XmlDocument();
                appSettings.Load(appSettingsFile);
                ApplicationDoc = appSettings;
            }

            XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIStatsWindow");

            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "left", ((int)Math.Round(_multiROIStatsWindows[indx].Left)).ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "top", ((int)Math.Round(_multiROIStatsWindows[indx].Top)).ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "width", ((int)Math.Round(_multiROIStatsWindows[indx].Width)).ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "height", ((int)Math.Round(_multiROIStatsWindows[indx].Height)).ToString());
            }
            _multiROIStatsWindows[indx] = null;

            bool empty = true;
            for (int i = 0; i < _multiROIStatsWindows.Count; i++)
            {
                if (null != _multiROIStatsWindows[i])
                {
                    empty = false;
                    break;
                }
            }
            if (empty)
            {
                ROIStatsTableActive = false;
            }

            ApplicationDoc.Save(appSettingsFile);
        }

        void _overlayManager_MaskChangedEvent()
        {
            MaskReady = true;
        }

        void _overlayManager_MaskWillChangeEvent()
        {
            MaskReady = false;
        }

        void _roiStatsChart_Closed(object sender, EventArgs e)
        {
            int indx = (int)(sender as ROIStatsChartWin).Tag;
            string appSettingsFile = (string.Empty == ExperimentModality) ? ResourceManagerCS.GetApplicationSettingsFileString() : ResourceManagerCS.GetModalityApplicationSettingsFileString(ExperimentModality);
            if (null == ApplicationDoc)
            {
                XmlDocument appSettings = new XmlDocument();
                appSettings.Load(appSettingsFile);
                ApplicationDoc = appSettings;
            }

            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");

            if (node != null)
            {

                XmlManager.SetAttribute(node, ApplicationDoc, "left", ((int)Math.Round(_roiStatsCharts[indx].Left)).ToString());
                XmlManager.SetAttribute(node, ApplicationDoc, "top", ((int)Math.Round(_roiStatsCharts[indx].Top)).ToString());
                XmlManager.SetAttribute(node, ApplicationDoc, "width", ((int)Math.Round(_roiStatsCharts[indx].Width)).ToString());
                XmlManager.SetAttribute(node, ApplicationDoc, "height", ((int)Math.Round(_roiStatsCharts[indx].Height)).ToString());

                ApplicationDoc.Save(appSettingsFile);
            }
            _roiStatsCharts[indx].ROIChart.SaveArithmeticStats();
            _roiStatsCharts[indx].ROIChart.ClearChart();
            //_rOIStatsChart.ROIChart.ClearLegendGroup(true);
            _roiStatsCharts[indx] = null;

            bool empty = true;
            for (int i = 0; i < _roiStatsCharts.Count; i++)
            {
                if (null != _roiStatsCharts[i])
                {
                    empty = false;
                    break;
                }
            }
            if (empty)
            {
                ROIStatsChartActive = false;
            }
        }

        void _spinnerWindow_Closed(object sender, EventArgs e)
        {
            _spinnerWindow = null;
        }

        #endregion Methods
    }
}