namespace RunSampleLSDll.ViewModel
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
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
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using GeometryUtilities;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using OverlayManager;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    using TilesDisplay;

    /// <summary>
    /// ViewModel class for the RunSampleLS model object
    /// </summary>
    public partial class RunSampleLSViewModel : ViewModelBase, ThorSharedTypes.IMVM
    {
        #region Fields

        public List<bool> CaptureSetupStartAfterLoading = new List<bool>();
        public int MAX_POWER_CONTROLS = 6;
        public List<bool> StartAfterLoading = new List<bool>(); //used to determine in Script mode: count > 0
        public XYTileControl.XYTileDisplay _xyTileControl;

        const int MAX_HISTOGRAM_HEIGHT = 250;
        const int MAX_HISTOGRAM_WIDTH = 444;
        const double MAX_ZOOM = 1000; // In  1000x
        const int MIN_HISTOGRAM_HEIGHT = 108;
        const int MIN_HISTOGRAM_WIDTH = 192;
        const double MIN_ZOOM = .01; // Out 100x

        // wrapped RunSampleLS object
        private readonly RunSampleLS _RunSampleLS;

        private bool _allHistogramsExpanded = false;
        private bool[] _autoManualTogChecked = { false, false, false, false };
        private WriteableBitmap _bitmap;
        private bool _bitmapReady = false;
        private DispatcherTimer _bitmapRefreshTimer; // refresh bitmap
        private Visibility _bleachOptionsVisibility;
        string _carrierType = string.Empty;
        private IUnityContainer _container;
        private int _currentSubImageCol;
        private int _currentSubImageRow;
        private ICommand _displayROIStatsOptionsCommand;
        private IEventAggregator _eventAggregator;
        private double _fieldSizeCalibration = 100.0;
        private int _histogramHeight1 = MIN_HISTOGRAM_HEIGHT;
        private int _histogramHeight2 = MIN_HISTOGRAM_HEIGHT;
        private int _histogramHeight3 = MIN_HISTOGRAM_HEIGHT;
        private int _histogramHeight4 = MIN_HISTOGRAM_HEIGHT;
        private int _histogramSpacing = 10;
        private int _histogramWidth1 = MIN_HISTOGRAM_WIDTH;
        private int _histogramWidth2 = MIN_HISTOGRAM_WIDTH;
        private int _histogramWidth3 = MIN_HISTOGRAM_WIDTH;
        private int _histogramWidth4 = MIN_HISTOGRAM_WIDTH;
        private bool _isTileButtonEnabled = true;
        private double _ivHeight;
        private double _iVScrollBarHeight;
        private bool _largeHistogram1 = false;
        private bool _largeHistogram2 = false;
        private bool _largeHistogram3 = false;
        private bool _largeHistogram4 = false;
        private DateTime _lastUpdate = DateTime.Now;
        private bool _logScaleEnabled0 = false;
        private bool _logScaleEnabled1 = false;
        private bool _logScaleEnabled2 = false;
        private bool _logScaleEnabled3 = false;
        private int _lSMFieldSize = 0;
        private string _mCLS1Name;
        private string _mCLS2Name;
        private string _mCLS3Name;
        private string _mCLS4Name;
        private bool _newExperiment;
        private int _nPixelBitShiftValueUpdates = 0;
        private bool _paletteChanged;
        Visibility _power0Visibility = Visibility.Collapsed;
        Visibility _power1Visibility = Visibility.Collapsed;
        Visibility _power2Visibility = Visibility.Collapsed;
        Visibility _power3Visibility = Visibility.Collapsed;
        Visibility _power4Visibility = Visibility.Collapsed;
        Visibility _power5Visibility = Visibility.Collapsed;
        private bool _rebuildBitmap = false;
        private IRegionManager _regionManager;
        private double _reqDiskSize;
        private ROIStatsChartWin _roiStatsChart = null;
        private bool _roiStatsChartActive;
        private Thickness _roiToolbarMargin = new Thickness(0, 0, 0, 0);
        private ICommand _RunSampleLSStartCommand;
        private ICommand _RunSampleLSStopCommand;
        string _scanAreaWellLocation = string.Empty;
        private SubscriptionToken _subscriptionToken;
        double _subSpacingXPercent = 0.0;
        double _subSpacingYPercent = 0.0;
        private Visibility _tdiOptionsVisibility;
        private Visibility _timeBasedLineScanVisibility = Visibility.Collapsed;

        // private DispatcherTimer _timer;
        private ICommand _TriggerSaveCommand;
        private double _turretMagnification = 0;
        private string _wavelengthName;
        private string _wellImageName;
        double _xYHomeOffsetX = 0;
        double _xYHomeOffsetY = 0;
        private ObservableCollection<XYPosition> _xYtableData = new ObservableCollection<XYPosition>();
        double _zoomLevel = 1;
        private Visibility _zOptionsVisibility;
        private bool _zStopPositionNotValid = false;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the RunSampleLSViewModel class
        /// </summary>
        /// <param name="RunSampleLS">Wrapped RunSampleLS object</param>
        public RunSampleLSViewModel(IEventAggregator eventAggregator, IRegionManager regionManager, IUnityContainer container, RunSampleLS RunSampleLS)
        {
            UnloadWhole = false;
            this._eventAggregator = eventAggregator;
            this._regionManager = regionManager;
            this._container = container;

            if (RunSampleLS != null)
            {
                this._RunSampleLS = RunSampleLS;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " RunSampleLS is null. Creating a new RunSampleLS object.");
                RunSampleLS = new RunSampleLS();

                if (RunSampleLS == null)
                {
                    ResourceManager rm = new ResourceManager("RunSampleLSModule.Properties.Resources", Assembly.GetExecutingAssembly());
                    ThorLog.Instance.TraceEvent(TraceEventType.Critical, 1, this.GetType().Name + " " + rm.GetString("CreateRunSampleLSModelFailed"));
                    throw new NullReferenceException("RunSampleLS");
                }

                this._RunSampleLS = RunSampleLS;
            }

            _bitmapRefreshTimer = new DispatcherTimer(DispatcherPriority.Render);
            _bitmapRefreshTimer.Interval = new TimeSpan(0, 0, 0, 0, 30);
            _bitmapRefreshTimer.Tick += RefreshBitmap;

            SubscribeToCommandEvent();

            _paletteChanged = true;
            _reqDiskSize = 0;

            OverlayManagerClass.Instance.InitOverlayManagerClass(this.SettingsImageWidth, this.SettingsImageHeight, this.PixelSizeUM, false);

            ChannelName = new ObservableCollection<StringPC>();

            for (int i = 0; i < RunSampleLS.GetMaxChannels(); i++)
            {
                ChannelName.Add(new StringPC());
            }

            PowerControlNames = new ObservableCollection<StringPC>();

            for (int i = 0; i < MAX_POWER_CONTROLS; i++)
            {
                PowerControlNames.Add(new StringPC());
            }

            _newExperiment = false; //flag to know if a new experiment has occurred
            this._RunSampleLS.ROIStatsChanged += _RunSampleLS_ROIStatsChanged;
            this.RunSampleLS.UpdateRemoteConnection += RunSampleLS_UpdateRemoteConnection;
            this._RunSampleLS.UpdataFilePath += _RunSampleLS_UpdataFilePath;

            MVMManager.CollectMVM();
            _roiStatsChartActive = false;
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Events

        public event Action<bool> HistogramPanelUpdate;

        //notify listeners (Ex. histogram) that the image has changed
        public event Action<bool> ImageDataChanged;

        //Increase the view area if the image extends out of bonds for the vertical scroll bar
        public event Action<bool> IncreaseViewArea;

        public event Action<bool> PaletteChanged;

        #endregion Events

        #region Properties

        public string ActiveExperimentPath
        {
            get
            {
                return this.RunSampleLS.ActiveExperimentPath;
            }
            set
            {
                this._RunSampleLS.ActiveExperimentPath = value;
                OnPropertyChanged("ActiveExperimentPath");
            }
        }

        public bool AllHistogramsExpanded
        {
            get
            {
                return _allHistogramsExpanded;
            }
            set
            {
                _allHistogramsExpanded = value;
                if (_allHistogramsExpanded)
                {
                    LargeHistogram1 = false;
                    LargeHistogram2 = false;
                    LargeHistogram3 = false;
                    LargeHistogram4 = false;
                    ExpandAllHistograms();
                }
                else
                {
                    ShrinkAllHistograms();
                }
                OnPropertyChanged("AllHistogramsExpanded");
            }
        }

        public bool AutoManualTog1Checked
        {
            get
            {
                return _autoManualTogChecked[0];
            }
            set
            {
                _autoManualTogChecked[0] = value;
                if (value && null != ImageDataChanged)
                    ImageDataChanged(true);
                OnPropertyChanged("AutoManualTog1Checked");
            }
        }

        public bool AutoManualTog2Checked
        {
            get
            {
                return _autoManualTogChecked[1];
            }
            set
            {
                _autoManualTogChecked[1] = value;
                if (value && null != ImageDataChanged)
                    ImageDataChanged(true);
                OnPropertyChanged("AutoManualTog2Checked");
            }
        }

        public bool AutoManualTog3Checked
        {
            get
            {
                return _autoManualTogChecked[2];
            }
            set
            {
                _autoManualTogChecked[2] = value;
                if (value && null != ImageDataChanged)
                    ImageDataChanged(true);
                OnPropertyChanged("AutoManualTog3Checked");
            }
        }

        public bool AutoManualTog4Checked
        {
            get
            {
                return _autoManualTogChecked[3];
            }
            set
            {
                _autoManualTogChecked[3] = value;
                if (value && null != ImageDataChanged)
                    ImageDataChanged(true);
                OnPropertyChanged("AutoManualTog4Checked");
            }
        }

        public int AvgFrames
        {
            get
            {
                if (0 == _RunSampleLS.AverageMode)
                {
                    return 1;
                }
                else
                {
                    return LSMAverageNum;
                }
            }
        }

        public int BinX
        {
            get
            {
                return _RunSampleLS.BinX;
            }
            set
            {
                _RunSampleLS.BinX = value;
            }
        }

        public int BinY
        {
            get
            {
                return _RunSampleLS.BinY;
            }
            set
            {
                _RunSampleLS.BinY = value;
            }
        }

        public WriteableBitmap Bitmap
        {
            get
            {
                CreateBitmap();
                if (null != ImageDataChanged) ImageDataChanged(true);
                return this._bitmap;
            }
        }

        public bool BitmapReady
        {
            get { return _bitmapReady; }
            set
            {
                _bitmapReady = value;
                OnPropertyChanged("BitmapReady");
            }
        }

        public double BlackPoint0
        {
            get
            {
                return this._RunSampleLS.BlackPoint0;
            }
            set
            {
                if (_RunSampleLS.BlackPoint0 == value) return;
                this._RunSampleLS.BlackPoint0 = value;
                _paletteChanged = true;
                OnPropertyChanged("BlackPoint0");
                OnPropertyChanged("Bitmap");
            }
        }

        public double BlackPoint1
        {
            get
            {
                return this._RunSampleLS.BlackPoint1;
            }
            set
            {
                if (_RunSampleLS.BlackPoint1 == value) return;
                this._RunSampleLS.BlackPoint1 = value;
                _paletteChanged = true;
                OnPropertyChanged("BlackPoint1");
                OnPropertyChanged("Bitmap");
            }
        }

        public double BlackPoint2
        {
            get
            {
                double result = 0.0;
                try
                {
                    result = this._RunSampleLS.BlackPoint2;
                }
                catch (IndexOutOfRangeException ex)
                {
                    string str = ex.Message;
                    result = 0.0;
                }
                return result;
            }
            set
            {
                if (_RunSampleLS.BlackPoint2 == value) return;
                this._RunSampleLS.BlackPoint2 = value;
                _paletteChanged = true;
                OnPropertyChanged("BlackPoint2");
                OnPropertyChanged("Bitmap");
            }
        }

        public double BlackPoint3
        {
            get
            {
                double result = 0.0;
                try
                {
                    result = this._RunSampleLS.BlackPoint3;
                }
                catch (IndexOutOfRangeException ex)
                {
                    string str = ex.Message;
                    result = 0.0;
                }
                return result;
            }
            set
            {
                if (_RunSampleLS.BlackPoint3 == value) return;
                this._RunSampleLS.BlackPoint3 = value;
                _paletteChanged = true;
                OnPropertyChanged("BlackPoint3");
                OnPropertyChanged("Bitmap");
            }
        }

        public int BleachFrames
        {
            get
            {
                return this._RunSampleLS.BleachFrames;
            }
            set
            {
                this._RunSampleLS.BleachFrames = value;
                OnPropertyChanged("BleachFrames");
                OnPropertyChanged("TTotalTime");
                OnPropertyChanged("DriveSpace");
            }
        }

        public Visibility BleachOptionsVisibility
        {
            get
            {
                return _bleachOptionsVisibility;
            }
            set
            {
                _bleachOptionsVisibility = value;

                OnPropertyChanged("BleachOptionsVisibility");
            }
        }

        public int BleachPostTrigger
        {
            get
            {
                return this._RunSampleLS.BleachPostTrigger;
            }

            set
            {
                this._RunSampleLS.BleachPostTrigger = value;
                OnPropertyChanged("BleachPostTrigger");
            }
        }

        public int BleachTrigger
        {
            get
            {
                return this._RunSampleLS.BleachTrigger;
            }

            set
            {
                this._RunSampleLS.BleachTrigger = value;
                OnPropertyChanged("BleachTrigger");
            }
        }

        public int CamImageHeight
        {
            get
            {
                return _RunSampleLS.CamImageHeight;
            }
            set
            {
                _RunSampleLS.CamImageHeight = value;
                OnPropertyChanged("FieldSizeHeightUM");
            }
        }

        public int CamImageWidth
        {
            get
            {
                return _RunSampleLS.CamImageWidth;
            }
            set
            {
                _RunSampleLS.CamImageWidth = value;
                OnPropertyChanged("FieldSizeWidthUM");
            }
        }

        public double CamPixelSizeUM
        {
            get
            {
                return _RunSampleLS.CamPixelSizeUM;
            }
            set
            {
                _RunSampleLS.CamPixelSizeUM = value;
                OnPropertyChanged("FieldSizeHeightUM");
                OnPropertyChanged("FieldSizeWidthUM");
            }
        }

        public int CaptureMode
        {
            get
            {
                return this._RunSampleLS.CaptureMode;
            }

            set
            {
                this._RunSampleLS.CaptureMode = value;
                //RawCapture Only when in Streaming Mode
                if (1 != value) RawDataCapture = 0;
                OnPropertyChanged("CaptureMode");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StimulusStreamVis");
                OnPropertyChanged("IsTileButtonEnabled");
                OnPropertyChanged("ZEnable");
                OnPropertyChanged("ZFastEnable");
            }
        }

        public double CarrierCenterToCenterDistanceX
        {
            get;
            set;
        }

        public double CarrierCenterToCenterDistanceY
        {
            get;
            set;
        }

        public string CarrierType
        {
            get
            {
                return _carrierType;
            }
            set
            {
                _carrierType = value;
            }
        }

        public ObservableCollection<StringPC> ChannelName
        {
            get;
            set;
        }

        public int ChannelSelection
        {
            get
            {
                return _RunSampleLS.ChannelSelection;
            }
            set
            {
                _RunSampleLS.ChannelSelection = value;
            }
        }

        public int CompletedImageCount
        {
            get
            {
                return this._RunSampleLS.CompletedImageCount;
            }
        }

        public BleachMode CurrentBleachMode
        {
            get
            {
                return _RunSampleLS.CurrentBleachMode;
            }
        }

        public int CurrentImageCount
        {
            get
            {
                return this._RunSampleLS.CurrentImageCount;
            }
        }

        public int CurrentSubImageCol
        {
            get
            {
                return _currentSubImageCol;
            }
            set
            {
                _currentSubImageCol = value;
                OnPropertyChanged("CurrentSubImageCol");
            }
        }

        public int CurrentSubImageCount
        {
            get
            {
                //if (_RunSampleLS.CurrentSubImageCount > 0 && ObjectMosaicCollection.Count > 0)
                //{
                //    ObjectMosaicCollection[_RunSampleLS.CurrentSubImageCount - 1].Background = new SolidColorBrush(Color.FromRgb(32, 193, 29));//green color
                //}
                return this._RunSampleLS.CurrentSubImageCount;
            }
        }

        public int CurrentSubImageRow
        {
            get
            {
                return _currentSubImageRow;
            }
            set
            {
                _currentSubImageRow = value;
                OnPropertyChanged("CurrentSubImageRow");
            }
        }

        public int CurrentTCount
        {
            get { return this._RunSampleLS.CurrentTCount; }
            set { this._RunSampleLS.CurrentTCount = value; OnPropertyChanged("CurrentTCount"); }
        }

        public int CurrentWellCount
        {
            get
            {
                return this._RunSampleLS.CurrentWellCount;
            }
        }

        public int CurrentZCount
        {
            get { return this._RunSampleLS.CurrentZCount; }
            set { this._RunSampleLS.CurrentZCount = value; OnPropertyChanged("CurrentZCount"); }
        }

        public uint[][] DFLIMHistogramData
        {
            get
            {
                return this.RunSampleLS.DFLIMHistogramData;
            }
        }

        public object DFLIMHistogramDataLock
        {
            get
            {
                return this.RunSampleLS.DFLIMHistogramDataLock;
            }
        }

        public bool DFLIMNewHistogramData
        {
            get
            {
                return this.RunSampleLS.DFLIMNewHistogramData;
            }
            set
            {
                this.RunSampleLS.DFLIMNewHistogramData = value;
                OnPropertyChanged("DFLIMNewHistogramData");
            }
        }

        public bool DisplayImage
        {
            get
            {
                return this._RunSampleLS.DisplayImage;
            }
            set
            {
                this._RunSampleLS.DisplayImage = value;

                if ((this._RunSampleLS.DisplayImage == true) && (this.ImageUpdaterVisibility == Visibility.Visible))
                {
                    _bitmapRefreshTimer.Start();
                }
                else
                {
                    _bitmapRefreshTimer.Stop();
                }

                OnPropertyChanged("DisplayImage");
            }
        }

        public ICommand DisplayROIStatsOptionsCommand
        {
            get
            {
                if (this._displayROIStatsOptionsCommand == null)
                    this._displayROIStatsOptionsCommand = new RelayCommand(() => DisplayROIStatsOptions());

                return this._displayROIStatsOptionsCommand;
            }
        }

        public int DMAFrames
        {
            get
            {
                return this._RunSampleLS.DMAFrames;
            }
            set
            {
                this._RunSampleLS.DMAFrames = value;
                OnPropertyChanged("DMAFrames");
                OnPropertyChanged("DMAFramesTime");
            }
        }

        public string DMAFramesTime
        {
            get
            {
                return this._RunSampleLS.DMAFramesTime;
            }
        }

        // 0. collapse; 1. visible
        public int DMAFramesVisibility
        {
            get
            {
                return this._RunSampleLS.DMAFramesVisibility;
            }
            set
            {
                this._RunSampleLS.DMAFramesVisibility = value;
                OnPropertyChanged("DMAFramesVisibility");
                //   OnPropertyChanged("StimulusStreamVis");
            }
        }

        public string DriveSpace
        {
            get
            {
                string str = String.Empty;

                System.IO.DriveInfo[] allDrives = System.IO.DriveInfo.GetDrives();

                Int64 reqSize = 0;

                bool isCCDCamera = IsCamera();
                int chan = 0;
                int rawChan = 0;
                string expPath = ResourceManagerCS.GetCaptureTemplatePathString() + "\\Active.xml";
                XmlDocument expDoc = new XmlDocument();
                expDoc.Load(expPath);

                if (isCCDCamera)
                {
                    rawChan = chan = 1;
                }
                else
                {
                    //Get the number of active channels from the int representation of four bits (CH1 CH2 CH3 CH4)
                    int channelBitmask = _RunSampleLS.GetLSMChannelAllCaptureSequenceSteps(expDoc);
                    for (int i = 0; i < RunSampleLS.GetMaxChannels(); i++)
                    {
                        int val = channelBitmask;

                        if (((val >> i) & 0x1) > 0)
                        {
                            chan++;
                        }
                    }

                    //Raw Channels Vary Based On Settings
                    if (_RunSampleLS.GetShouldSaveOnlyEnabledChannelsInRawFile() || chan == 1)
                    {
                        rawChan = chan;
                    }
                    else
                    {
                        rawChan = _RunSampleLS.GetMaxChannels();
                    }
                }

                Int64 width = this.SettingsImageWidth;
                Int64 height = this.SettingsImageHeight;

                const int BYTES_PER_PIXEL = 2;
                const double BYTES_TO_MEGABYTES = 1048576.0;
                const int ACCUMULATIVE = 1;

                if ((int)CaptureModes.T_AND_Z == this.CaptureMode)
                {
                    int nz = 1;
                    int nt = 1;
                    int nzs = 1;
                    int nsub = 1;

                    if (1 == ZEnable)
                    {
                        nz = ZNumSteps;
                    }

                    if (TEnable)
                    {
                        nt = TFrames;
                    }

                    if (ZStreamEnable && (ZStreamFrames > 1))
                    {
                        nzs = ZStreamFrames;
                    }

                    nsub = TotalTiles;

                    double val = width * height * chan * nzs * nz * nt * nsub * BYTES_PER_PIXEL / BYTES_TO_MEGABYTES;
                    reqSize = (long)(0.5 + val);
                }
                else if ((int)CaptureModes.STREAMING == this.CaptureMode)
                {
                    if (this.StreamStorageMode == 0)
                    {
                        int depth = ZFastEnable ? (_RunSampleLS.ZNumSteps + _RunSampleLS.FlybackFrames) : 1;
                        int frames = ZFastEnable ? _RunSampleLS.StreamVolumes : ((ACCUMULATIVE == _RunSampleLS.AverageMode) ? (_RunSampleLS.StreamFrames * _RunSampleLS.AverageNum) : _RunSampleLS.StreamFrames);
                        if (1 == RawDataCapture)
                        {
                            reqSize = GetNeededDiskSpaceForStreamingRaw(width, height, depth, chan, rawChan, frames);
                        }
                        else
                        {
                            reqSize = GetNeededDiskSpaceForStreaming(width, height, depth, chan, rawChan, frames);
                        }
                    }
                    else if (this.StreamStorageMode == 1)
                    {
                        //for stimulus the depth is always 1 because the size is determined by the max frames
                        //not by the number of Z slices
                        int depth = 1;
                        if (1 == RawDataCapture)
                        {
                            reqSize = GetNeededDiskSpaceForStreamingRaw(width, height, depth, chan, rawChan, _RunSampleLS.StimulusMaxFrames);
                        }
                        else
                        {
                            reqSize = GetNeededDiskSpaceForStreaming(width, height, depth, chan, rawChan, _RunSampleLS.StimulusMaxFrames);
                        }
                    }
                }
                else if ((int)CaptureModes.BLEACHING == this.CaptureMode)
                {
                    long preBleachSize;
                    long postBleachSize1;
                    long postBleachSize2;

                    if (1 == RawOption)
                    {
                        preBleachSize = GetNeededDiskSpaceForStreamingRaw(width, height, 1, chan, rawChan, PreBleachFrames);
                        postBleachSize1 = GetNeededDiskSpaceForStreamingRaw(width, height, 1, chan, rawChan, PostBleachFrames1);
                        postBleachSize2 = GetNeededDiskSpaceForStreamingRaw(width, height, 1, chan, rawChan, PostBleachFrames2);
                    }
                    else
                    {
                        preBleachSize = GetNeededDiskSpaceForStreaming(width, height, 1, chan, rawChan, PreBleachFrames);
                        postBleachSize1 = GetNeededDiskSpaceForStreaming(width, height, 1, chan, rawChan, PostBleachFrames1);
                        postBleachSize2 = GetNeededDiskSpaceForStreaming(width, height, 1, chan, rawChan, PostBleachFrames2);
                    }

                    reqSize = preBleachSize + postBleachSize1 + postBleachSize2;
                }
                else if ((int)CaptureModes.HYPERSPECTRAL == this.CaptureMode)
                {
                    int depth = 1;
                    int frames = SFSteps;
                    if (1 == RawDataCapture)
                    {
                        reqSize = GetNeededDiskSpaceForStreamingRaw(width, height, depth, chan, rawChan, frames);
                    }
                    else
                    {
                        reqSize = GetNeededDiskSpaceForStreaming(width, height, depth, chan, rawChan, frames);
                    }
                }

                _reqDiskSize = reqSize;

                const double MEGABYTES_TO_GIGABYTES = 1024.0;
                if (reqSize < 0)
                {
                    str += String.Format("Too Large\n");
                }
                else
                {
                    //convert to Gigabytes if greater than 1Gb
                    if (reqSize > MEGABYTES_TO_GIGABYTES)
                    {
                        Decimal num = new Decimal(reqSize / MEGABYTES_TO_GIGABYTES);
                        str += String.Format("Required Disk Size: ~{0} Gb\n", Decimal.Round(num, 2));
                    }
                    else
                    {
                        str += String.Format("Required Disk Size: ~{0} Mb\n", reqSize);
                    }
                }

                foreach (System.IO.DriveInfo d in allDrives)
                {
                    if (d.DriveType.Equals(System.IO.DriveType.Fixed))
                    {
                        try
                        {
                            str += String.Format("{0} {1:0} Gb available\n", d.Name, d.AvailableFreeSpace / (BYTES_TO_MEGABYTES * MEGABYTES_TO_GIGABYTES));
                        }
                        catch (System.Exception ex)
                        {
                            string strErr = ex.Message;
                            //suppress this exception and continue
                        }
                    }
                }

                if (this.CaptureMode == 1)
                {
                    ulong val = 0;
                    MEMORYSTATUSEX msex = new MEMORYSTATUSEX();
                    if (GlobalMemoryStatusEx(msex))
                    {
                        val = msex.ullAvailPhys / (ulong)BYTES_TO_MEGABYTES;
                    }
                    //etc.. Repeat for other structure members

                    int zsFrames = Math.Max(1, ZStreamFrames);

                    //************** hide the required RAM Size********************///
                    ////convert to Gigabytes if greater than 1Gb
                    //if (reqSize > MEGABYTES_TO_GIGABYTES)
                    //{
                    //    Decimal num = new Decimal(reqSize / MEGABYTES_TO_GIGABYTES);
                    //    str += String.Format("Required RAM Size: ~{0} Gb\n", Decimal.Round(num,2));
                    //}
                    //else
                    //{
                    //    str += String.Format("Required RAM Size: ~{0} Mb\n", reqSize);
                    //}

                    Decimal ram = new Decimal(val / MEGABYTES_TO_GIGABYTES);
                    str += String.Format("RAM ~{0} Gb available\n", Decimal.Round(ram, 2));
                }

                return str;
            }
        }

        public string ExperimentFolderPath
        {
            get
            {
                return _RunSampleLS.ExperimentFolderPath;
            }
        }

        public string ExperimentIndex
        {
            get
            {
                if (_RunSampleLS.DontAskUniqueExperimentName && _RunSampleLS.RunComplete)
                {
                    this._RunSampleLS.ExperimentName.MakeUnique(OutputPath);
                }
                return _RunSampleLS.ExperimentName.NameNumber;
            }
            set
            {
                _RunSampleLS.ExperimentName.NameNumber = value;
                OnPropertyChanged("ExperimentIndex");
            }
        }

        public string ExperimentName
        {
            get
            {
                return _RunSampleLS.ExperimentName.NameWithoutNumber;
            }
            set
            {
                if (0 == Regex.Matches(value, ExperimentNameNumberPattern).Count)
                {
                    this._RunSampleLS.ExperimentName.NameWithoutNumber = value;
                    OnPropertyChanged("ExperimentName");
                    ExperimentIndex = "";
                    OnPropertyChanged("ExperimentIndex");
                }
            }
        }

        public string ExperimentNameNumberPattern
        {
            get
            {
                string separator = string.IsNullOrEmpty(_RunSampleLS.ExperimentName.NameNumberSeparator) ? "_" : _RunSampleLS.ExperimentName.NameNumberSeparator;
                return (separator + "\\d{1," + (int)Constants.MAX_FILE_FORMAT_DIGITS + "}");
            }
        }

        public string ExperimentNotes
        {
            get
            {
                return this._RunSampleLS.ExperimentNotes;
            }
            set
            {
                this._RunSampleLS.ExperimentNotes = value;
                OnPropertyChanged("ExperimentNotes");
            }
        }

        public string ExperimentXMLPath
        {
            get
            {
                return _RunSampleLS.ExperimentXMLPath;
            }
        }

        public bool FastZStaircase
        {
            get
            {
                return this._RunSampleLS.FastZStaircase;
            }
            set
            {
                this._RunSampleLS.FastZStaircase = value;
                if (!this._RunSampleLS.FastZStaircase)
                {
                    FlybackFrames = Math.Max(1, FlybackFrames);
                    OnPropertyChanged("FlybackFrames");
                }
                OnPropertyChanged("FastZStaircase");
                OnPropertyChanged("ZNumSteps");
            }
        }

        public double FieldHeight
        {
            get
            {
                return this._RunSampleLS.FieldHeight;
            }
            set
            {
                this._RunSampleLS.FieldHeight = value;
                OnPropertyChanged("FieldHeight");
            }
        }

        public double FieldSizeHeightUM
        {
            get
            {

                if (_RunSampleLS.ActiveCameraType == (int)ICamera.CameraType.LSM)
                {
                    return this.LSMFieldSizeYUM;
                }
                else
                {
                    return this.CamPixelSizeUM * (this.CamImageHeight);
                }
            }
        }

        public double FieldSizeWidthUM
        {
            get
            {

                if (_RunSampleLS.ActiveCameraType == (int)ICamera.CameraType.LSM)
                {
                    return this.LSMFieldSizeXUM;
                }
                else
                {
                    return this.CamPixelSizeUM * (this.CamImageWidth);
                }
            }
        }

        public double FieldWidth
        {
            get
            {
                return this._RunSampleLS.FieldWidth;
            }
            set
            {
                this._RunSampleLS.FieldWidth = value;
                OnPropertyChanged("FieldWidth");
            }
        }

        public int FlybackFrames
        {
            get
            {
                return this._RunSampleLS.FlybackFrames;
            }
            set
            {
                this._RunSampleLS.FlybackFrames = (FastZStaircase) ? Math.Max(0, value) : Math.Max(1, value);
                OnPropertyChanged("FlybackFrames");
                OnPropertyChanged("FlybackTimeAdjustMS");
                OnPropertyChanged("StreamFramesTime");
                OnPropertyChanged("DriveSpace");
            }
        }

        public int FlybackLines
        {
            get
            {
                return this._RunSampleLS.FlybackLines;
            }
            set
            {
                this._RunSampleLS.FlybackLines = value;
                OnPropertyChanged("FlybackLines");
                OnPropertyChanged("LSMFlybackTime");
            }
        }

        public double FlybackTimeAdjustMS
        {
            get
            {
                return this._RunSampleLS.FlybackTimeAdjustMS;
            }
            set
            {
                this._RunSampleLS.FlybackTimeAdjustMS = value;
                OnPropertyChanged("FlybackTimeAdjustMS");
            }
        }

        public string FullExperimentName
        {
            get
            {
                return this._RunSampleLS.ExperimentName.FullName;
            }
            set
            {
                this._RunSampleLS.ExperimentName.FullName = value;
            }
        }

        public XmlDocument HardwareDoc
        {
            get
            {
                return RunSampleLS.HardwareDoc;
            }
            set
            {
                this.RunSampleLS.HardwareDoc = value;
            }
        }

        public int[] HistogramData0
        {
            get
            {
                return this._RunSampleLS.HistogramData0;
            }
        }

        public int[] HistogramData1
        {
            get
            {
                return this._RunSampleLS.HistogramData1;
            }
        }

        public int[] HistogramData2
        {
            get
            {
                return this._RunSampleLS.HistogramData2;
            }
        }

        public int[] HistogramData3
        {
            get
            {
                return this._RunSampleLS.HistogramData3;
            }
        }

        public int HistogramHeight1
        {
            get
            {
                return _histogramHeight1;
            }
            set
            {
                _histogramHeight1 = value;
                OnPropertyChanged("HistogramHeight1");
            }
        }

        public int HistogramHeight2
        {
            get
            {
                return _histogramHeight2;
            }
            set
            {
                _histogramHeight2 = value;
                OnPropertyChanged("HistogramHeight2");
            }
        }

        public int HistogramHeight3
        {
            get
            {
                return _histogramHeight3;
            }
            set
            {
                _histogramHeight3 = value;
                OnPropertyChanged("HistogramHeight3");
            }
        }

        public int HistogramHeight4
        {
            get
            {
                return _histogramHeight4;
            }
            set
            {
                _histogramHeight4 = value;
                OnPropertyChanged("HistogramHeight4");
            }
        }

        public int HistogramSpacing
        {
            get
            {
                return 10;
                //return _histogramSpacing;
            }
            set
            {
                _histogramSpacing = value;
                OnPropertyChanged("HistogramSpacing");
            }
        }

        public int HistogramWidth1
        {
            get
            {
                return _histogramWidth1;
            }
            set
            {
                _histogramWidth1 = value;
                OnPropertyChanged("HistogramWidth1");
            }
        }

        public int HistogramWidth2
        {
            get
            {
                return _histogramWidth2;
            }
            set
            {
                _histogramWidth2 = value;
                OnPropertyChanged("HistogramWidth2");
            }
        }

        public int HistogramWidth3
        {
            get
            {
                return _histogramWidth3;
            }
            set
            {
                _histogramWidth3 = value;
                OnPropertyChanged("HistogramWidth3");
            }
        }

        public int HistogramWidth4
        {
            get
            {
                return _histogramWidth4;
            }
            set
            {
                _histogramWidth4 = value;
                OnPropertyChanged("HistogramWidth4");
            }
        }

        public int IDMode
        {
            get
            {
                return this.RunSampleLS.IDMode;
            }
            set
            {
                this.RunSampleLS.IDMode = value;
                OnPropertyChanged("IDMode");
                OnPropertyChanged("RemotePCHostName");
            }
        }

        public int ImageColorChannels
        {
            get
            {
                return this._RunSampleLS.ImageColorChannels;
            }
        }

        public Visibility ImageUpdaterVisibility
        {
            get
            {
                return this._RunSampleLS.ImageUpdaterVisibility;
            }
            set
            {
                this._RunSampleLS.ImageUpdaterVisibility = value;
                OnPropertyChanged("ImageUpdaterVisibility");
                if (value == Visibility.Collapsed)
                {
                    this.DisplayImage = false;
                }
            }
        }

        public string InitialModality
        {
            get;
            set;
        }

        public bool IsStimulusSaving
        {
            get
            {
                return this._RunSampleLS.IsStimulusSaving;
            }
            set
            {
                this._RunSampleLS.IsStimulusSaving = value;
                OnPropertyChanged("IsStimulusSaving");
                OnPropertyChanged("StimulusSaveBtnDisplay");
            }
        }

        public bool IsTileButtonEnabled
        {
            get
            {
                if ((int)CaptureModes.STREAMING == this.CaptureMode && true == ZFastEnable)
                {
                    _isTileButtonEnabled = false;
                    //Disable TileDisplay
                    TileDisplay = false;
                }
                else
                {
                    _isTileButtonEnabled = true;
                }
                return _isTileButtonEnabled;
            }
            set
            {
                _isTileButtonEnabled = value;
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
        }

        public bool LargeHistogram1
        {
            get
            {
                return _largeHistogram1;
            }
            set
            {
                _largeHistogram1 = value;
                if (_largeHistogram1)
                {
                    LargeHistogram2 = false;
                    LargeHistogram3 = false;
                    LargeHistogram4 = false;
                    AllHistogramsExpanded = false;
                    HistogramHeight1 = MAX_HISTOGRAM_HEIGHT;
                    HistogramWidth1 = MAX_HISTOGRAM_WIDTH;
                }
                else
                {
                    HistogramHeight1 = MIN_HISTOGRAM_HEIGHT;
                    HistogramWidth1 = MIN_HISTOGRAM_WIDTH;
                }
                OnPropertyChanged("LargeHistogram1");
            }
        }

        public bool LargeHistogram2
        {
            get
            {
                return _largeHistogram2;
            }
            set
            {
                _largeHistogram2 = value;
                if (_largeHistogram2)
                {
                    LargeHistogram1 = false;
                    LargeHistogram3 = false;
                    LargeHistogram4 = false;
                    AllHistogramsExpanded = false;
                    HistogramHeight2 = MAX_HISTOGRAM_HEIGHT;
                    HistogramWidth2 = MAX_HISTOGRAM_WIDTH;
                }
                else
                {
                    HistogramHeight2 = MIN_HISTOGRAM_HEIGHT;
                    HistogramWidth2 = MIN_HISTOGRAM_WIDTH;
                }
                OnPropertyChanged("LargeHistogram2");
            }
        }

        public bool LargeHistogram3
        {
            get
            {
                return _largeHistogram3;
            }
            set
            {
                _largeHistogram3 = value;
                if (_largeHistogram3)
                {
                    LargeHistogram1 = false;
                    LargeHistogram2 = false;
                    LargeHistogram4 = false;
                    AllHistogramsExpanded = false;
                    HistogramHeight3 = MAX_HISTOGRAM_HEIGHT;
                    HistogramWidth3 = MAX_HISTOGRAM_WIDTH;
                }
                else
                {
                    HistogramHeight3 = MIN_HISTOGRAM_HEIGHT;
                    HistogramWidth3 = MIN_HISTOGRAM_WIDTH;
                }
                OnPropertyChanged("LargeHistogram3");
            }
        }

        public bool LargeHistogram4
        {
            get
            {
                return _largeHistogram4;
            }
            set
            {
                _largeHistogram4 = value;
                if (_largeHistogram4)
                {
                    LargeHistogram1 = false;
                    LargeHistogram2 = false;
                    LargeHistogram3 = false;
                    AllHistogramsExpanded = false;
                    HistogramHeight4 = MAX_HISTOGRAM_HEIGHT;
                    HistogramWidth4 = MAX_HISTOGRAM_WIDTH;
                }
                else
                {
                    HistogramHeight4 = MIN_HISTOGRAM_HEIGHT;
                    HistogramWidth4 = MIN_HISTOGRAM_WIDTH;
                }
                OnPropertyChanged("LargeHistogram4");
            }
        }

        public int LeftLabelCount
        {
            get
            {
                return this._RunSampleLS.LeftLabelCount;
            }
            set
            {
                this._RunSampleLS.LeftLabelCount = value;
                OnPropertyChanged("LeftLabelCount");
            }
        }

        public string LocalPCHostName
        {
            get
            {
                return System.Environment.MachineName;
            }
        }

        public string LocalPCIPv4
        {
            get
            {
                return this.RunSampleLS.LocalPCIPv4;
            }
        }

        public bool LogScaleEnabled0
        {
            get
            {
                return _logScaleEnabled0;
            }
            set
            {
                _logScaleEnabled0 = value;
                OnPropertyChanged("LogScaleEnabled0");
            }
        }

        public bool LogScaleEnabled1
        {
            get
            {
                return _logScaleEnabled1;
            }
            set
            {
                _logScaleEnabled1 = value;
                OnPropertyChanged("LogScaleEnabled1");
            }
        }

        public bool LogScaleEnabled2
        {
            get
            {
                return _logScaleEnabled2;
            }
            set
            {
                _logScaleEnabled2 = value;
                OnPropertyChanged("LogScaleEnabled2");
            }
        }

        public bool LogScaleEnabled3
        {
            get
            {
                return _logScaleEnabled3;
            }
            set
            {
                _logScaleEnabled3 = value;
                OnPropertyChanged("LogScaleEnabled3");
            }
        }

        //Square=0, Rect=1, Kymograph=2, Line=3, Polyline=4
        public int LSMAreaMode
        {
            get { return _RunSampleLS.LSMAreaMode; }
            set { _RunSampleLS.LSMAreaMode = value; }
        }

        public int LSMAverageMode
        {
            get { return this._RunSampleLS.AverageMode; }
            set
            {
                this._RunSampleLS.AverageMode = value;
                OnPropertyChanged("StreamFrames");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamTotalTime");
                OnPropertyChanged("StreamFramesTime");
                OnPropertyChanged("DMAFramesTime");
                OnPropertyChanged("StimulusMaxFramesTime");
                OnPropertyChanged("AvgFrames");
            }
        }

        public int LSMAverageNum
        {
            get { return this._RunSampleLS.AverageNum; }
            set
            {
                this._RunSampleLS.AverageNum = value;
                OnPropertyChanged("StreamFrames");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamTotalTime");
                OnPropertyChanged("StreamFramesTime");
                OnPropertyChanged("DMAFramesTime");
                OnPropertyChanged("StimulusMaxFramesTime");
                OnPropertyChanged("AvgFrames");
            }
        }

        public int LSMChannel
        {
            get
            {
                return this._RunSampleLS.LSMChannel;
            }
            set
            {
                this._RunSampleLS.LSMChannel = value;
            }
        }

        public Brush[] LSMChannelColor
        {
            get
            {
                return this._RunSampleLS.LSMChannelColor;
            }
            set
            {
                this._RunSampleLS.LSMChannelColor = value;
                OnPropertyChanged("LSMChannelColor");
            }
        }

        public bool[] LSMChannelEnable
        {
            get
            {
                return _RunSampleLS.LSMChannelEnable;
            }
        }

        public bool LSMChannelEnable0
        {
            get
            {
                return _RunSampleLS.LSMChannelEnable0;
            }
            set
            {
                _RunSampleLS.LSMChannelEnable0 = value;
            }
        }

        public bool LSMChannelEnable1
        {
            get
            {
                return _RunSampleLS.LSMChannelEnable1;
            }
            set
            {
                _RunSampleLS.LSMChannelEnable1 = value;
            }
        }

        public bool LSMChannelEnable2
        {
            get
            {
                return _RunSampleLS.LSMChannelEnable2;
            }
            set
            {
                _RunSampleLS.LSMChannelEnable2 = value;
            }
        }

        public bool LSMChannelEnable3
        {
            get
            {
                return _RunSampleLS.LSMChannelEnable3;
            }
            set
            {
                _RunSampleLS.LSMChannelEnable3 = value;
            }
        }

        public int LSMFieldSize
        {
            get
            {
                return _lSMFieldSize;
            }
            set
            {
                _lSMFieldSize = value;
                OnPropertyChanged("FieldSizeHeightUM");
                OnPropertyChanged("FieldSizeWidthUM");
            }
        }

        public double LSMFieldSizeCalibration
        {
            get
            {
                double retVal = 1.0;

                //If the scanner returns a field size calibration
                //use that. Otherwise use the settings file
                if (GetLSMFieldSizeCalibration(ref retVal))
                {

                }
                else
                {
                    retVal = _fieldSizeCalibration;
                }

                return retVal;

            }
            set
            {
                _fieldSizeCalibration = value;
                OnPropertyChanged("FieldSizeHeightUM");
                OnPropertyChanged("FieldSizeWidthUM");
            }
        }

        public double LSMFieldSizeXUM
        {
            get
            {
                double dVal = this.LSMFieldSize * this.LSMFieldSizeCalibration / TurretMagnification;

                dVal = Math.Round(dVal, 2);
                return dVal;
            }
        }

        public double LSMFieldSizeYUM
        {
            get
            {
                double dVal;

                if (3 == LSMAreaMode || 4 == LSMAreaMode)
                {
                    double pixelXYRatio = 1.0 / LSMPixelX;
                    dVal = this.LSMFieldSize * this.LSMFieldSizeCalibration / TurretMagnification * pixelXYRatio;
                }
                else
                {
                    double pixelXYRatio = Convert.ToDouble(LSMPixelY) / Convert.ToDouble(LSMPixelX);
                    dVal = this.LSMFieldSize * this.LSMFieldSizeCalibration / TurretMagnification * pixelXYRatio;
                }

                dVal = Math.Round(dVal, 2);
                return dVal;
            }
        }

        public double LSMFlybackTime
        {
            get
            {
                return Math.Round(this._RunSampleLS.LSMFlybackTime, 6);
            }
        }

        public int LSMPixelX
        {
            get { return this._RunSampleLS.LSMPixelX; }
            set
            {
                this._RunSampleLS.LSMPixelX = value;
                OnPropertyChanged("FieldSizeHeightUM");
                OnPropertyChanged("FieldSizeWidthUM");
            }
        }

        public int LSMPixelY
        {
            get { return this._RunSampleLS.LSMPixelY; }
            set
            {
                this._RunSampleLS.LSMPixelY = value;
                OnPropertyChanged("FieldSizeHeightUM");
                OnPropertyChanged("FieldSizeWidthUM");
            }
        }

        public int LSMScanMode
        {
            get { return this._RunSampleLS.ScanMode; }
            set
            {
                this._RunSampleLS.ScanMode = value;
                OnPropertyChanged("StreamFrames");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamTotalTime");
                OnPropertyChanged("StreamFramesTime");
                OnPropertyChanged("DMAFramesTime");
                OnPropertyChanged("StimulusMaxFramesTime");
            }
        }

        public ICamera.LSMType LSMType
        {
            get
            {
                return RunSampleLS.LSMType;
            }
        }

        public double LSMUMPerPixel
        {
            get
            {
                return _RunSampleLS.LSMUMPerPixel;
            }
            set
            {
                _RunSampleLS.LSMUMPerPixel = value;
            }
        }

        public string MCLS1Name
        {
            get
            {
                return _mCLS1Name;
            }
            set
            {
                _mCLS1Name = value;
                OnPropertyChanged("MCLS1Name");
            }
        }

        public double MCLS1Power
        {
            get
            {
                return _RunSampleLS.MCLS1Power;
            }
            set
            {
                _RunSampleLS.MCLS1Power = value;
                OnPropertyChanged("MCLS1Power");
            }
        }

        public Visibility MCLS1Visibility
        {
            get
            {
                return _RunSampleLS.MCLS1Visibility;
            }
            set
            {
                _RunSampleLS.MCLS1Visibility = value;
                OnPropertyChanged("MCLS1Visibility");
            }
        }

        public string MCLS2Name
        {
            get
            {
                return _mCLS2Name;
            }
            set
            {
                _mCLS2Name = value;
                OnPropertyChanged("MCLS2Name");
            }
        }

        public double MCLS2Power
        {
            get
            {
                return _RunSampleLS.MCLS2Power;
            }
            set
            {
                _RunSampleLS.MCLS2Power = value;
                OnPropertyChanged("MCLS2Power");
            }
        }

        public Visibility MCLS2Visibility
        {
            get
            {
                return _RunSampleLS.MCLS2Visibility;
            }
            set
            {
                _RunSampleLS.MCLS2Visibility = value;
                OnPropertyChanged("MCLS2Visibility");
            }
        }

        public string MCLS3Name
        {
            get
            {
                return _mCLS3Name;
            }
            set
            {
                _mCLS3Name = value;
                OnPropertyChanged("MCLS3Name");
            }
        }

        public double MCLS3Power
        {
            get
            {
                return _RunSampleLS.MCLS3Power;
            }
            set
            {
                _RunSampleLS.MCLS3Power = value;
                OnPropertyChanged("MCLS3Power");
            }
        }

        public Visibility MCLS3Visibility
        {
            get
            {
                return _RunSampleLS.MCLS3Visibility;
            }
            set
            {
                _RunSampleLS.MCLS3Visibility = value;
                OnPropertyChanged("MCLS3Visibility");
            }
        }

        public string MCLS4Name
        {
            get
            {
                return _mCLS4Name;
            }
            set
            {
                _mCLS4Name = value;
                OnPropertyChanged("MCLS4Name");
            }
        }

        public double MCLS4Power
        {
            get
            {
                return _RunSampleLS.MCLS4Power;
            }
            set
            {
                _RunSampleLS.MCLS4Power = value;
                OnPropertyChanged("MCLS4Power");
            }
        }

        public Visibility MCLS4Visibility
        {
            get
            {
                return _RunSampleLS.MCLS4Visibility;
            }
            set
            {
                _RunSampleLS.MCLS4Visibility = value;
                OnPropertyChanged("MCLS4Visibility");
            }
        }

        public double MMPerPixel
        {
            get
            {
                if ((LSMPixelX == 0) || (TurretMagnification == 0))
                {
                    return 1.0;
                }
                else
                {
                    return (LSMFieldSize * LSMFieldSizeCalibration) / (LSMPixelX * TurretMagnification * 1000.0);
                }
            }
        }

        public int MosaicColumnCount
        {
            get
            {
                return this._RunSampleLS.MosaicColumnCount;
            }
            set
            {
                this._RunSampleLS.MosaicColumnCount = value;
                OnPropertyChanged("MosaicColumnCount");
            }
        }

        public int MosaicRowCount
        {
            get
            {
                return this._RunSampleLS.MosaicRowCount;
            }
            set
            {
                this._RunSampleLS.MosaicRowCount = value;
                OnPropertyChanged("MosaicRowCount");
            }
        }

        public string[] MVMNames
        {
            get { return this._RunSampleLS.MVMNames; }
            set { this._RunSampleLS.MVMNames = value; }
        }

        public bool NewExperiment
        {
            get
            {
                return _newExperiment;
            }
            set
            {
                _newExperiment = value;
            }
        }

        public int NumberOfPlanes
        {
            get => _RunSampleLS.NumberOfPlanes;
            set => _RunSampleLS.NumberOfPlanes = value;
        }

        //public ObservableCollection<Button> ObjectCollection
        //{
        //    get { return this._RunSampleLS.Collection; }
        //    set
        //    {
        //        this._RunSampleLS.Collection = value;
        //        OnPropertyChanged("ObjectCollection");
        //    }
        //}
        //public ObservableCollection<Label> ObjectLeftLabelCollection
        //{
        //    get { return this._RunSampleLS.CollectionLeftLabel; }
        //    set
        //    {
        //        this._RunSampleLS.CollectionLeftLabel = value;
        //    }
        //}
        //public ObservableCollection<Button> ObjectMosaicCollection
        //{
        //    get { return this._RunSampleLS.CollectionMosaic; }
        //    set
        //    {
        //        this._RunSampleLS.CollectionMosaic = value;
        //        OnPropertyChanged("ObjectMosaicCollection");
        //    }
        //}
        //public ObservableCollection<Label> ObjectTopLabelCollection
        //{
        //    get { return this._RunSampleLS.CollectionTopLabel; }
        //    set
        //    {
        //        this._RunSampleLS.CollectionTopLabel = value;
        //    }
        //}
        public string OutputPath
        {
            get
            {
                return this._RunSampleLS.OutputPath;
            }
            set
            {
                this._RunSampleLS.OutputPath = value;
                OnPropertyChanged("OutputPath");
            }
        }

        public bool PanelsEnable
        {
            get
            {
                return this._RunSampleLS.PanelsEnable;
            }
            set
            {
                this._RunSampleLS.PanelsEnable = value;
                OnPropertyChanged("PanelsEnable");
            }
        }

        public double PercentComplete
        {
            get
            {
                return _RunSampleLS.PercentComplete;
            }
        }

        public double PhotobleachDurationSec
        {
            get
            {
                return this._RunSampleLS.PhotobleachDurationSec;
            }
            set
            {
                this._RunSampleLS.PhotobleachDurationSec = value;
                OnPropertyChanged("PhotobleachDurationSec");
            }
        }

        public int PhotobleachEnable
        {
            get
            {
                return this._RunSampleLS.PhotobleachEnable;
            }
            set
            {
                this._RunSampleLS.PhotobleachEnable = value;
                OnPropertyChanged("PhotobleachEnable");
            }
        }

        public int PhotobleachLaserPosition
        {
            get
            {
                return this._RunSampleLS.PhotobleachLaserPosition;
            }
            set
            {
                this._RunSampleLS.PhotobleachLaserPosition = value;
                OnPropertyChanged("PhotobleachLaserPosition");
            }
        }

        public double PhotobleachPowerPosition
        {
            get
            {
                return this._RunSampleLS.PhotobleachPowerPosition;
            }
            set
            {
                this._RunSampleLS.PhotobleachPowerPosition = value;
                OnPropertyChanged("PhotobleachPowerPosition");
            }
        }

        public int PinholePosition
        {
            get
            {
                return _RunSampleLS.PinholePosition;
            }
            set
            {
                _RunSampleLS.PinholePosition = value;
                OnPropertyChanged("PinholePosition");
            }
        }

        public Visibility PinholeVisibility
        {
            get
            {
                return _RunSampleLS.PinholeVisibility;
            }
            set
            {
                _RunSampleLS.PinholeVisibility = value;
                OnPropertyChanged("PinholeVisibility");
            }
        }

        public int PixelBitShiftValue
        {
            get
            {
                return _RunSampleLS.PixelBitShiftValue;
            }
        }

        public double PixelSizeUM
        {
            get
            {
                return _RunSampleLS.PixelSizeUM;
            }
        }

        public int PMT1EnableDuringBleach
        {
            get
            {
                return _RunSampleLS.PMT1EnableDuringBleach;
            }
            set
            {
                _RunSampleLS.PMT1EnableDuringBleach = value;
                OnPropertyChanged("PMT1EnableDuringBleach");
            }
        }

        public int PMT2EnableDuringBleach
        {
            get
            {
                return _RunSampleLS.PMT2EnableDuringBleach;
            }
            set
            {
                _RunSampleLS.PMT2EnableDuringBleach = value;
                OnPropertyChanged("PMT2EnableDuringBleach");
            }
        }

        public int PMT3EnableDuringBleach
        {
            get
            {
                return _RunSampleLS.PMT3EnableDuringBleach;
            }
            set
            {
                _RunSampleLS.PMT3EnableDuringBleach = value;
                OnPropertyChanged("PMT3EnableDuringBleach");
            }
        }

        public int PMT4EnableDuringBleach
        {
            get
            {
                return _RunSampleLS.PMT4EnableDuringBleach;
            }
            set
            {
                _RunSampleLS.PMT4EnableDuringBleach = value;
                OnPropertyChanged("PMT4EnableDuringBleach");
            }
        }

        public Visibility PockelsOutputReferenceVisibility
        {
            get
            {
                return (_RunSampleLS.PockelsOutputReferenceAvailable) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public int PostBleachFrames1
        {
            get
            {
                return this._RunSampleLS.PostBleachFrames1;
            }
            set
            {
                this._RunSampleLS.PostBleachFrames1 = value;
                OnPropertyChanged("PostBleachFrames1");
                OnPropertyChanged("TTotalTime");
                OnPropertyChanged("DriveSpace");
            }
        }

        public int PostBleachFrames2
        {
            get
            {
                return this._RunSampleLS.PostBleachFrames2;
            }
            set
            {
                this._RunSampleLS.PostBleachFrames2 = value;
                OnPropertyChanged("PostBleachFrames2");
                OnPropertyChanged("TTotalTime");
                OnPropertyChanged("DriveSpace");
            }
        }

        public double PostBleachInterval1
        {
            get
            {
                return this._RunSampleLS.PostBleachInterval1;
            }
            set
            {
                this._RunSampleLS.PostBleachInterval1 = value;
                OnPropertyChanged("PostBleachInterval1");
                OnPropertyChanged("TTotalTime");
            }
        }

        public double PostBleachInterval2
        {
            get
            {
                return this._RunSampleLS.PostBleachInterval2;
            }
            set
            {
                this._RunSampleLS.PostBleachInterval2 = value;
                OnPropertyChanged("PostBleachInterval2");
                OnPropertyChanged("TTotalTime");
            }
        }

        public int PostBleachStream1
        {
            get
            {
                return this._RunSampleLS.PostBleachStream1;
            }

            set
            {
                this._RunSampleLS.PostBleachStream1 = value;
                if (PreBleachStream != 0 && PostBleachStream1 != 0 && PostBleachStream2 != 0)
                {
                    ShowRawOption = Visibility.Visible;
                }
                else
                {
                    ShowRawOption = Visibility.Hidden;
                    RawOption = 0;
                }
                OnPropertyChanged("PostBleachStream1");
                OnPropertyChanged("DriveSpace");
            }
        }

        public int PostBleachStream2
        {
            get
            {
                return this._RunSampleLS.PostBleachStream2;
            }

            set
            {
                this._RunSampleLS.PostBleachStream2 = value;
                if (PreBleachStream != 0 && PostBleachStream1 != 0 && PostBleachStream2 != 0)
                {
                    ShowRawOption = System.Windows.Visibility.Visible;
                }
                else
                {
                    ShowRawOption = System.Windows.Visibility.Hidden;
                    RawOption = 0;
                }
                OnPropertyChanged("PostBleachStream2");
                OnPropertyChanged("DriveSpace");
            }
        }

        public double Power0
        {
            get
            {
                return _RunSampleLS.Power0;
            }
        }

        public Visibility Power0Visibility
        {
            get
            {
                return _power0Visibility;
            }
            set
            {
                _power0Visibility = value;
                OnPropertyChanged("Power0Visibility");
            }
        }

        public double Power1
        {
            get
            {
                return _RunSampleLS.Power1;
            }
        }

        public Visibility Power1Visibility
        {
            get
            {
                return _power1Visibility;
            }
            set
            {
                _power1Visibility = value;
                OnPropertyChanged("Power1Visibility");
            }
        }

        public double Power2
        {
            get
            {
                return _RunSampleLS.Power2;
            }
        }

        public Visibility Power2Visibility
        {
            get
            {
                return _power2Visibility;
            }
            set
            {
                _power2Visibility = value;
                OnPropertyChanged("Power2Visibility");
            }
        }

        public double Power3
        {
            get
            {
                return _RunSampleLS.Power3;
            }
        }

        public Visibility Power3Visibility
        {
            get
            {
                return _power3Visibility;
            }
            set
            {
                _power3Visibility = value;
                OnPropertyChanged("Power3Visibility");
            }
        }

        public double Power4
        {
            get
            {
                return _RunSampleLS.Power4;
            }
        }

        public Visibility Power4Visibility
        {
            get
            {
                return _power4Visibility;
            }
            set
            {
                _power4Visibility = value;
                OnPropertyChanged("Power4Visibility");
            }
        }

        public double Power5
        {
            get
            {
                return _RunSampleLS.Power5;
            }
        }

        public Visibility Power5Visibility
        {
            get
            {
                return _power5Visibility;
            }
            set
            {
                _power5Visibility = value;
                OnPropertyChanged("Power5Visibility");
            }
        }

        public ObservableCollection<StringPC> PowerControlNames
        {
            get;
            set;
        }

        public int PreBleachFrames
        {
            get
            {
                return this._RunSampleLS.PreBleachFrames;
            }
            set
            {
                this._RunSampleLS.PreBleachFrames = value;
                OnPropertyChanged("PreBleachFrames");
                OnPropertyChanged("TTotalTime");
                OnPropertyChanged("DriveSpace");
            }
        }

        public double PreBleachInterval
        {
            get
            {
                return this._RunSampleLS.PreBleachInterval;
            }
            set
            {
                this._RunSampleLS.PreBleachInterval = value;
                OnPropertyChanged("PreBleachInterval");
                OnPropertyChanged("TTotalTime");
            }
        }

        public int PreBleachStream
        {
            get
            {
                return this._RunSampleLS.PreBleachStream;
            }

            set
            {
                this._RunSampleLS.PreBleachStream = value;
                if (PreBleachStream != 0 && PostBleachStream1 != 0 && PostBleachStream2 != 0)
                {
                    ShowRawOption = Visibility.Visible;
                }
                else
                {
                    ShowRawOption = Visibility.Hidden;
                    RawOption = 0;
                }
                OnPropertyChanged("PreBleachStream");
                OnPropertyChanged("DriveSpace");
            }
        }

        public int PreviewIndex
        {
            get
            {
                return this._RunSampleLS.PreviewIndex;
            }
            set
            {
                this._RunSampleLS.PreviewIndex = Math.Max(1, value);
                this._RunSampleLS.PreviewIndex = Math.Min(this._RunSampleLS.ZNumSteps, value);
                OnPropertyChanged("PreviewIndex");
            }
        }

        public int RawDataCapture
        {
            get
            {
                return this._RunSampleLS.RawDataCapture;
            }
            set
            {
                this._RunSampleLS.RawDataCapture = value;
                OnPropertyChanged("RawDataCapture");
                OnPropertyChanged("DriveSpace");
            }
        }

        public int RawOption
        {
            get
            {
                return this._RunSampleLS.RawOption;
            }

            set
            {
                this._RunSampleLS.RawOption = value;
                OnPropertyChanged("RawOption");
                OnPropertyChanged("DriveSpace");
            }
        }

        public bool RebuildBitmap
        {
            get
            {
                return _rebuildBitmap;
            }
            set
            {
                _rebuildBitmap = value;
                if (true == value)
                {
                    _paletteChanged = value;
                }
            }
        }

        public string RemoteAppName
        {
            get
            {
                return this.RunSampleLS.RemoteAppName;
            }
            set
            {
                if ("" == value)
                {
                    return;
                }
                this.RunSampleLS.RemoteAppName = value;
                OnPropertyChanged("RemoteAppName");
            }
        }

        public bool RemoteConnection
        {
            get
            {
                return this.RunSampleLS.RemoteConnection;
            }
            set
            {
                this.RunSampleLS.RemoteConnection = value;
            }
        }

        public string RemotePCHostName
        {
            get
            {
                return this.RunSampleLS.RemotePCHostName;
            }
            set
            {
                if ("" == value)
                {
                    return;
                }
                this.RunSampleLS.RemotePCHostName = value;
                OnPropertyChanged("RemotePCHostName");
            }
        }

        public ROIStatsChartWin ROIStatsChart
        {
            get { return _roiStatsChart; }
            set { _roiStatsChart = value; }
        }

        public bool ROIStatsChartActive
        {
            get
            {
                return _roiStatsChartActive;
            }
            set
            {
                _roiStatsChartActive = value;
                if (true == _roiStatsChartActive)
                {
                    if (null != _roiStatsChart)
                    {
                        _roiStatsChart.Show();
                    }
                }
                else
                {
                    if (null != _roiStatsChart)
                    {
                        _roiStatsChart.Close();
                    }
                }
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

        public int RollOverPointIntensity0
        {
            get
            {
                return this._RunSampleLS.RollOverPointIntensity0;
            }
        }

        public int RollOverPointIntensity1
        {
            get
            {
                return this._RunSampleLS.RollOverPointIntensity1;
            }
        }

        public int RollOverPointIntensity2
        {
            get
            {
                return this._RunSampleLS.RollOverPointIntensity2;
            }
        }

        public int RollOverPointIntensity3
        {
            get
            {
                return this._RunSampleLS.RollOverPointIntensity3;
            }
        }

        public int RollOverPointX
        {
            get
            {
                return this._RunSampleLS.RollOverPointX;
            }
            set
            {
                this._RunSampleLS.RollOverPointX = value;
            }
        }

        public int RollOverPointY
        {
            get
            {
                return this._RunSampleLS.RollOverPointY;
            }
            set
            {
                this._RunSampleLS.RollOverPointY = value;
            }
        }

        /// <summary>
        /// Gets the wrapped RunSampleLS object
        /// </summary>
        public RunSampleLS RunSampleLS
        {
            get
            {
                return this._RunSampleLS;
            }
        }

        public ICommand RunSampleLSStartCommand
        {
            get
            {
                if (this._RunSampleLSStartCommand == null)
                    this._RunSampleLSStartCommand = new RelayCommand(() => RunSampleLSStart());

                return this._RunSampleLSStartCommand;
            }
        }

        public ICommand RunSampleLSStopCommand
        {
            get
            {
                if (this._RunSampleLSStopCommand == null)
                    this._RunSampleLSStopCommand = new RelayCommand(() => RunSampleLSStop());

                return this._RunSampleLSStopCommand;
            }
        }

        ///<summary>
        ///Gets or sets the left of the RunSampleLS
        ///</summary>
        public string SampleConfig
        {
            get
            {
                return this._RunSampleLS.SampleConfig;
            }
            set
            {
                this._RunSampleLS.SampleConfig = value;
                OnPropertyChanged("SampleConfig");
            }
        }

        public int SampleType
        {
            get;
            set;
        }

        public string ScanAreaWellLocation
        {
            get
            {
                return _scanAreaWellLocation;
            }
            set
            {
                _scanAreaWellLocation = value;
                OnPropertyChanged("ScanAreaWellLocation");
            }
        }

        public int SelectedRemotePCNameIndex
        {
            get
            {
                return this.RunSampleLS.SelectedRemotePCNameIndex;
            }
            set
            {
                this.RunSampleLS.SelectedRemotePCNameIndex = value;
                OnPropertyChanged("SelectedRemotePCNameIndex");
            }
        }

        public string SelectedWavelengthValue
        {
            get
            {
                return this._RunSampleLS.SelectedWavelengthValue;
            }
            set
            {
                this._RunSampleLS.SelectedWavelengthValue = value;
                OnPropertyChanged("SelectedWavelengthValue");
            }
        }

        public int SettingsImageHeight
        {
            get => _RunSampleLS.SettingsImageHeight;
        }

        public int SettingsImageWidth
        {
            get => _RunSampleLS.SettingsImageWidth;
        }

        public int SFStartWavelength
        {
            get
            {
                return _RunSampleLS.SFStartWavelength;
            }
            set
            {
                _RunSampleLS.SFStartWavelength = value;
                OnPropertyChanged("SFStartWavelength");
            }
        }

        public int SFSteps
        {
            get
            {
                return _RunSampleLS.SFSteps;
            }
            set
            {
                _RunSampleLS.SFSteps = value;
                OnPropertyChanged("SFSteps");
            }
        }

        public int SFStopWavelength
        {
            get
            {
                return _RunSampleLS.SFStopWavelength;
            }
            set
            {
                _RunSampleLS.SFStopWavelength = value;
                OnPropertyChanged("SFStopWavelength");
            }
        }

        public int SFWavelengthStepSize
        {
            get
            {
                return _RunSampleLS.SFWavelengthStepSize;
            }
            set
            {
                _RunSampleLS.SFWavelengthStepSize = value;
                OnPropertyChanged("SFWavelengthStepSize");
            }
        }

        public Visibility ShowRawOption
        {
            get
            {
                return this._RunSampleLS.ShowRawOption;
            }
            set
            {
                this._RunSampleLS.ShowRawOption = value;
                OnPropertyChanged("ShowRawOption");
            }
        }

        public int SimultaneousBleachingAndImaging
        {
            get
            {
                return _RunSampleLS.SimultaneousBleachingAndImaging;
            }
            set
            {
                _RunSampleLS.SimultaneousBleachingAndImaging = value;
                OnPropertyChanged("SimultaneousBleachingAndImaging");
            }
        }

        public bool SLM3D
        {
            get
            {
                return this._RunSampleLS.SLM3D;
            }
            set
            {
                this._RunSampleLS.SLM3D = value;
                OnPropertyChanged("SLM3D");
            }
        }

        public double SLMBleachDelay
        {
            get { return this._RunSampleLS.SLMBleachDelay; }
            set
            {
                Decimal val = Decimal.Round((Decimal)value, 3);
                this._RunSampleLS.SLMBleachDelay = (double)val;
                OnPropertyChanged("SLMBleachDelay");
            }
        }

        public ObservableCollection<SLMParams> SLMBleachWaveParams
        {
            get
            {
                return this._RunSampleLS.SLMBleachWaveParams;
            }
            set
            {
                this._RunSampleLS.SLMBleachWaveParams = value;
                OnPropertyChanged("SLMBleachWaveParams");
            }
        }

        public bool SLMSequenceOn
        {
            get
            {
                return this._RunSampleLS.SLMSequenceOn;
            }
            set
            {
                this._RunSampleLS.SLMSequenceOn = value;
                OnPropertyChanged("SLMSequenceOn");
            }
        }

        public bool StartButtonStatus
        {
            get
            {
                return _RunSampleLS.StartButtonStatus;
            }
            set
            {
                this._RunSampleLS.StartButtonStatus = value;
                OnPropertyChanged("StartButtonStatus");
            }
        }

        public string StatusMessage
        {
            get
            {
                return this._RunSampleLS.StatusMessage;
            }
            set
            {
                this._RunSampleLS.StatusMessage = value;
                OnPropertyChanged("StatusMessage");
            }
        }

        public double StepTimeAdjustMS
        {
            get
            {
                return this._RunSampleLS.StepTimeAdjustMS;
            }
            set
            {
                this._RunSampleLS.StepTimeAdjustMS = value;
                OnPropertyChanged("StepTimeAdjustMS");
            }
        }

        public int StimulusMaxFrames
        {
            get
            {
                return this._RunSampleLS.StimulusMaxFrames;
            }
            set
            {
                this._RunSampleLS.StimulusMaxFrames = value;
                OnPropertyChanged("StimulusMaxFrames");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StimulusMaxFramesTime");
            }
        }

        public string StimulusMaxFramesTime
        {
            get
            {
                return this._RunSampleLS.StimulusMaxFramesTime;
            }
        }

        public string StimulusSaveBtnDisplay
        {
            get
            {
                if (IsStimulusSaving)
                {
                    return "Saving";
                }
                else
                {
                    return "Idle";
                }
            }
        }

        public bool StimulusStreamVis
        {
            get
            {
                if ((1 == CaptureMode) && (1 == StreamStorageMode)) //Stimulus Stream only
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        public int StimulusTriggering
        {
            get
            {
                return this._RunSampleLS.StimulusTriggering;
            }
            set
            {
                this._RunSampleLS.StimulusTriggering = value;
                OnPropertyChanged("StimulusTriggering");
            }
        }

        public bool StopButtonStatus
        {
            get
            {
                return _RunSampleLS.StopButtonStatus;
            }
            set
            {
                this._RunSampleLS.StopButtonStatus = value;
                OnPropertyChanged("StopButtonStatus");
            }
        }

        public int StreamEnable
        {
            get
            {
                return this._RunSampleLS.StreamEnable;
            }
            set
            {
                this._RunSampleLS.StreamEnable = value;
                OnPropertyChanged("StreamEnable");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamTotalTime");
                OnPropertyChanged("StreamFramesTime");
                OnPropertyChanged("DMAFramesTime");
                OnPropertyChanged("StimulusMaxFramesTime");
            }
        }

        public int StreamFrames
        {
            get
            {
                return this._RunSampleLS.StreamFrames;
            }
            set
            {
                this._RunSampleLS.StreamFrames = value;
                OnPropertyChanged("StreamFrames");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamTotalTime");
                OnPropertyChanged("StreamFramesTime");
                OnPropertyChanged("DMAFramesTime");
            }
        }

        public string StreamFramesTime
        {
            get
            {
                return this._RunSampleLS.StreamFramesTime;
            }
        }

        public int StreamingDisplayRollingAveragePreview
        {
            get
            {
                return _RunSampleLS.StreamingDisplayRollingAveragePreview;
            }
            set
            {
                _RunSampleLS.StreamingDisplayRollingAveragePreview = value;

                OnPropertyChanged("StreamingDisplayRollingAveragePreview");
            }
        }

        public string StreamingPath
        {
            get
            {
                return _RunSampleLS.StreamingPath;
            }
            set
            {
                _RunSampleLS.StreamingPath = value;
            }
        }

        public int StreamStorageMode
        {
            get
            {
                return this._RunSampleLS.StreamStorageMode;
            }
            set
            {
                this._RunSampleLS.StreamStorageMode = value;
                OnPropertyChanged("StreamStorageMode");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StimulusStreamVis");
            }
        }

        public string StreamTotalTime
        {
            get
            {
                return this._RunSampleLS.StreamTotalTime;
            }
        }

        public int StreamVolumes
        {
            get
            {
                return this._RunSampleLS.StreamVolumes;
            }
            set
            {
                this._RunSampleLS.StreamVolumes = Math.Max(1, value);

                OnPropertyChanged("StreamVolumes");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamFramesTime");
            }
        }

        public double SubSpacingXPercent
        {
            get
            {
                return this._subSpacingXPercent;
            }
            set
            {
                this._subSpacingXPercent = value;
                OnPropertyChanged("SubSpacingXPercent");
            }
        }

        public double SubSpacingYPercent
        {
            get
            {
                return this._subSpacingYPercent;
            }
            set
            {
                this._subSpacingYPercent = value;
                OnPropertyChanged("SubSpacingYPercent");
            }
        }

        // z stream text box enable/unenable
        public bool TBZSEnable
        {
            get
            {
                return this._RunSampleLS.TBZSEnable;
            }
            set
            {
                this._RunSampleLS.TBZSEnable = value;

                OnPropertyChanged("TBZSEnable");
                OnPropertyChanged("TTotalTime");
                OnPropertyChanged("DriveSpace");
            }
        }

        public Visibility TDIOptionsVisibility
        {
            get
            {
                return _tdiOptionsVisibility;
            }
            set
            {
                _tdiOptionsVisibility = value;

                OnPropertyChanged("TDIOptionsVisibility");
            }
        }

        public bool TEnable
        {
            get
            {
                return this._RunSampleLS.TEnable;
            }
            set
            {
                this._RunSampleLS.TEnable = value;

                OnPropertyChanged("TEnable");
                OnPropertyChanged("DriveSpace");
            }
        }

        public int TFrames
        {
            get
            {
                return this._RunSampleLS.TFrames;
            }
            set
            {
                this._RunSampleLS.TFrames = value;
                OnPropertyChanged("TFrames");
                OnPropertyChanged("TTotalTime");
                OnPropertyChanged("DriveSpace");
            }
        }

        public bool ThreePhotonEnable
        {
            get => _RunSampleLS.ThreePhotonEnable;
            set => _RunSampleLS.ThreePhotonEnable = value;
        }

        public XYTileControl.XYTileDisplay TileControl
        {
            get
            {
                return _xyTileControl;
            }
            set
            {
                //Unload listener from last value
                if (_xyTileControl != null)
                {
                    _xyTileControl.Loaded -= tileControl_TilesLoaded;
                }

                _xyTileControl = value;

                //Load listener to next value
                if (_xyTileControl != null)
                {
                    _xyTileControl.Loaded += tileControl_TilesLoaded;
                }
            }
        }

        public bool TileDisplay
        {
            get
            {
                return _RunSampleLS.DisplayTile;
            }
            set
            {
                if (_RunSampleLS.DisplayTile != value)
                {
                    _RunSampleLS.DisplayTile = value;
                    _RunSampleLS.IsDisplayImageReady = true;
                    OnPropertyChanged("TileDisplay");
                    OnPropertyChanged("Bitmap");
                }
            }
        }

        public int TimeBasedLineScan
        {
            get
            {
                return this._RunSampleLS.TimeBasedLineScan;
            }
            set
            {
                this._RunSampleLS.TimeBasedLineScan = value;
                OnPropertyChanged("TimeBasedLineScan");
            }
        }

        public Visibility TimeBasedLineScanVisibility
        {
            get
            {
                return _timeBasedLineScanVisibility;
            }
            set
            {
                _timeBasedLineScanVisibility = value;
                OnPropertyChanged("TimeBasedLineScanVisibility");
            }
        }

        public double TimeBasedLSTimeMS
        {
            get
            {
                return this._RunSampleLS.TimeBasedLSTimeMS;
            }
            set
            {
                this._RunSampleLS.TimeBasedLSTimeMS = value;
                OnPropertyChanged("TimeBasedLSTimeMS");
            }
        }

        public double TInterval
        {
            get
            {
                return this._RunSampleLS.TInterval;
            }
            set
            {
                //Minimum time interval is 1 second for camera
                if (IsCamera())
                {
                    this._RunSampleLS.TInterval = Math.Max(1, value);
                }
                else
                {
                    this._RunSampleLS.TInterval = value;
                }
                OnPropertyChanged("TInterval");
                OnPropertyChanged("TTotalTime");
            }
        }

        public int TopLabelCount
        {
            get
            {
                return this._RunSampleLS.TopLabelCount;
            }
            set
            {
                this._RunSampleLS.TopLabelCount = value;
                OnPropertyChanged("TopLabelCount");
            }
        }

        public int TotalImageCount
        {
            get
            {
                return this._RunSampleLS.TotalImageCount;
            }
        }

        /// <summary>
        /// The total tiles to be captured. Will always capture at least 1 tile.
        /// </summary>
        public int TotalTiles
        {
            get
            {
                if (TileControl != null)
                {
                    int tiles = TileControl.TotalTiles;
                    return (tiles > 0 ? tiles : 1);
                }
                return 1;
            }
        }

        public int TriggerModeStreaming
        {
            get
            {
                return this._RunSampleLS.TriggerModeStreaming;
            }
            set
            {
                this._RunSampleLS.TriggerModeStreaming = value;
                OnPropertyChanged("TriggerModeStreaming");
            }
        }

        public int TriggerModeTimelapse
        {
            get
            {
                return this._RunSampleLS.TriggerModeTimelapse;
            }
            set
            {
                this._RunSampleLS.TriggerModeTimelapse = value;
                OnPropertyChanged("TriggerModeTimelapse");
            }
        }

        public ICommand TriggerSaveCommand
        {
            get
            {
                if (this._TriggerSaveCommand == null)
                    this._TriggerSaveCommand = new RelayCommand(() => StimulusSave());

                return this._TriggerSaveCommand;
            }
        }

        public string TTotalTime
        {
            get
            {
                return this._RunSampleLS.TTotalTime;
            }
        }

        public bool TurnOffMonitorsDuringCapture
        {
            get
            {
                return this._RunSampleLS.TurnOffMonitorsDuringCapture;
            }

            set
            {
                this._RunSampleLS.TurnOffMonitorsDuringCapture = value;
                OnPropertyChanged("TurnOffMonitorsDuringCapture");
            }
        }

        public double TurretMagnification
        {
            get
            {
                return _turretMagnification;
            }
            set
            {
                _turretMagnification = value;
                OnPropertyChanged("FieldSizeHeightUM");
                OnPropertyChanged("FieldSizeWidthUM");
            }
        }

        public int TurretPosition
        {
            get;
            set;
        }

        public bool UnloadWhole
        {
            get;
            set;
        }

        public int UseReferenceVoltageForFastZPockels
        {
            get
            {
                return _RunSampleLS.UseReferenceVoltageForFastZPockels;
            }
            set
            {
                _RunSampleLS.UseReferenceVoltageForFastZPockels = value;
                OnPropertyChanged("UseReferenceVoltageForFastZPockels");
            }
        }

        public bool VerticalTileDisplay
        {
            get
            {
                return _RunSampleLS.VerticalTileDisplay;
            }
            set
            {
                _RunSampleLS.VerticalTileDisplay = value;
            }
        }

        public double VolumeTimeAdjustMS
        {
            get
            {
                return this._RunSampleLS.VolumeTimeAdjustMS;
            }
            set
            {
                this._RunSampleLS.VolumeTimeAdjustMS = value;
                OnPropertyChanged("VolumeTimeAdjustMS");
            }
        }

        public ArrayList WavelengthList
        {
            get
            {
                return this._RunSampleLS.WavelengthList;
            }
            set
            {
                this._RunSampleLS.WavelengthList = value;
                OnPropertyChanged("WavelengthList");
            }
        }

        public string WavelengthName
        {
            get
            {
                return _wavelengthName;
            }
            set
            {
                _wavelengthName = value;
                OnPropertyChanged("WavelengthName");
                OnPropertyChanged("WhitePoint");
                OnPropertyChanged("BlackPoint");
            }
        }

        public string WellImageName
        {
            get
            {
                return this._wellImageName;
            }
            set
            {
                this._wellImageName = value;
                OnPropertyChanged("WellImageName");
            }
        }

        public double WhitePoint0
        {
            get
            {
                return this._RunSampleLS.WhitePoint0;
            }
            set
            {
                if (_RunSampleLS.WhitePoint0 == value) return;
                this._RunSampleLS.WhitePoint0 = value;
                _paletteChanged = true;
                OnPropertyChanged("WhitePoint0");
                OnPropertyChanged("Bitmap");
            }
        }

        public double WhitePoint1
        {
            get
            {
                return this._RunSampleLS.WhitePoint1;
            }
            set
            {
                if (_RunSampleLS.WhitePoint1 == value) return;
                this._RunSampleLS.WhitePoint1 = value;
                _paletteChanged = true;
                OnPropertyChanged("WhitePoint1");
                OnPropertyChanged("Bitmap");
            }
        }

        public double WhitePoint2
        {
            get
            {
                double result = 0.0;
                try
                {
                    result = this._RunSampleLS.WhitePoint2;
                }
                catch (IndexOutOfRangeException ex)
                {
                    string str = ex.Message;
                    result = 0.0;
                }
                return result;
            }
            set
            {
                if (_RunSampleLS.WhitePoint2 == value) return;
                this._RunSampleLS.WhitePoint2 = value;
                _paletteChanged = true;
                OnPropertyChanged("WhitePoint2");
                OnPropertyChanged("Bitmap");
            }
        }

        public double WhitePoint3
        {
            get
            {
                double result = 0.0;
                try
                {
                    result = this._RunSampleLS.WhitePoint3;
                }
                catch (IndexOutOfRangeException ex)
                {
                    string str = ex.Message;
                    result = 0.0;
                }
                return result;
            }
            set
            {
                if (_RunSampleLS.WhitePoint3 == value) return;
                this._RunSampleLS.WhitePoint3 = value;
                _paletteChanged = true;
                OnPropertyChanged("WhitePoint3");
                OnPropertyChanged("Bitmap");
            }
        }

        public double XPosition
        {
            get
            {
                return _RunSampleLS.XPosition;
            }
        }

        public double XYHomeOffsetX
        {
            get
            {
                return _xYHomeOffsetX;
            }
            set
            {
                _xYHomeOffsetX = value;
            }
        }

        public double XYHomeOffsetY
        {
            get
            {
                return _xYHomeOffsetY;
            }
            set
            {
                _xYHomeOffsetY = value;
            }
        }

        // We need a new XYTableData structure to send to the XYTileControl project. XYTileControl is expecting to
        // get XYtableData from it's MVM, but since RunSample doesn't have it's own instance of MVMManager, this is
        // a work around.
        public ObservableCollection<XYPosition> XYtableData
        {
            get { return _xYtableData; }
            set { _xYtableData = value; }
        }

        public double YPosition
        {
            get
            {
                return _RunSampleLS.YPosition;
            }
        }

        public bool Z2StageLock
        {
            get { return _RunSampleLS.Z2StageLock; }
            set { _RunSampleLS.Z2StageLock = value; }
        }

        public bool Z2StageMirror
        {
            get { return _RunSampleLS.Z2StageMirror; }
            set { _RunSampleLS.Z2StageMirror = value; }
        }

        public int ZEnable
        {
            get
            {
                return this._RunSampleLS.ZEnable;
            }
            set
            {
                this._RunSampleLS.ZEnable = value;

                OnPropertyChanged("ZEnable");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("ZFastEnable");
            }
        }

        public bool ZFastEnable
        {
            get
            {
                return this._RunSampleLS.ZFastEnable;
            }
            set
            {
                this._RunSampleLS.ZFastEnable = value;

                //Do not display the tile view if fastZ is enabled
                if (_RunSampleLS.ZFastEnable)
                {
                    TileDisplay = false;
                    IsTileButtonEnabled = false;
                }
                else
                {
                    IsTileButtonEnabled = true;
                }

                OnPropertyChanged("ZFastEnable");
                OnPropertyChanged("StreamFrames");
                OnPropertyChanged("StreamVolumes");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamFramesTime");
                OnPropertyChanged("IsTileButtonEnabled");
            }
        }

        public int ZFileEnable
        {
            get
            {
                return this._RunSampleLS.ZFileEnable;
            }
            set
            {
                this._RunSampleLS.ZFileEnable = value;
                // if value differs from the file read flag, reset that flag to false
                if (value != _RunSampleLS.ZFilePosRead) _RunSampleLS.ZFilePosRead = 0;
                OnPropertyChanged("ZFileEnable");
                OnPropertyChanged("ZNumSteps");
            }
        }

        public double ZFilePosScale
        {
            get
            {
                return _RunSampleLS.ZFilePosScale;
            }
            set
            {
                _RunSampleLS.ZFilePosScale = value;
                OnPropertyChanged("ZFilePosScale");
            }
        }

        public Visibility ZFileVisibility
        {
            get;
            set;
        }

        public int ZNumSteps
        {
            get
            {
                return this._RunSampleLS.ZNumSteps;
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

        public Visibility ZOptionsVisibility
        {
            get
            {
                return _zOptionsVisibility;
            }
            set
            {
                _zOptionsVisibility = value;

                OnPropertyChanged("ZOptionsVisibility");
            }
        }

        public double ZPosition
        {
            get
            {
                Decimal dec;

                dec = new Decimal(this._RunSampleLS.ZPosition);

                dec = Decimal.Round(dec, 4);

                return Convert.ToDouble(dec);
            }
            set
            {
                OnPropertyChanged("ZPosition");
            }
        }

        public double ZStartPosition
        {
            get
            {
                return _RunSampleLS.ZStartPosition;
            }
            set
            {
                if (value <= _RunSampleLS.ZMin)
                {
                    _RunSampleLS.ZStartPosition = _RunSampleLS.ZMin;
                }
                else if (value >= _RunSampleLS.ZMax)
                {
                    _RunSampleLS.ZStartPosition = _RunSampleLS.ZMax;
                }
                else
                {
                    _RunSampleLS.ZStartPosition = value;
                }

                EnsureValidZScanStop();

                OnPropertyChanged("ZStartPosition");
                OnPropertyChanged("ZNumSteps");
                OnPropertyChanged("VolumeTimeAdjustMS");
                OnPropertyChanged("StepTimeAdjustMS");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamVolumes");
                OnPropertyChanged("PreviewIndex");
                OnPropertyChanged("StreamFramesTime");
            }
        }

        public double ZStepSize
        {
            get
            {
                return this._RunSampleLS.ZStepSize;
            }
            set
            {
                double rounded = Math.Round(value, 1);
                if (rounded <= .1)
                {
                    rounded = .1;
                }

                this._RunSampleLS.ZStepSize = rounded;

                EnsureValidZScanStop();

                OnPropertyChanged("ZStepSize");
                OnPropertyChanged("ZNumSteps");
                OnPropertyChanged("VolumeTimeAdjustMS");
                OnPropertyChanged("StepTimeAdjustMS");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamVolumes");
                OnPropertyChanged("PreviewIndex");
                OnPropertyChanged("StreamFramesTime");
            }
        }

        public double ZStopPosition
        {
            get
            {
                return _RunSampleLS.ZStopPosition;
            }
            set
            {
                if (value <= _RunSampleLS.ZMin)
                {
                    _RunSampleLS.ZStopPosition = _RunSampleLS.ZMin;
                }
                else if (value >= _RunSampleLS.ZMax)
                {
                    _RunSampleLS.ZStopPosition = _RunSampleLS.ZMax;
                }
                else
                {
                    _RunSampleLS.ZStopPosition = value;
                }

                EnsureValidZScanStop();

                OnPropertyChanged("ZStopPosition");
                OnPropertyChanged("ZNumSteps");
                OnPropertyChanged("VolumeTimeAdjustMS");
                OnPropertyChanged("StepTimeAdjustMS");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamVolumes");
                OnPropertyChanged("PreviewIndex");
                OnPropertyChanged("StreamFramesTime");
            }
        }

        public bool ZStopPositionNotValid
        {
            get
            {
                return _zStopPositionNotValid;
            }
            set
            {
                _zStopPositionNotValid = value;
                OnPropertyChanged("ZStopPositionNotValid");
            }
        }

        public bool ZStreamEnable
        {
            get
            {
                return this._RunSampleLS.ZStreamEnable;
            }
            set
            {
                this._RunSampleLS.ZStreamEnable = value;

                OnPropertyChanged("ZStreamEnable");
                OnPropertyChanged("ZStreamFrames");
                OnPropertyChanged("TTotalTime");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamFramesTime");
            }
        }

        public int ZStreamFrames
        {
            get
            {
                return this._RunSampleLS.ZStreamFrames;
            }
            set
            {
                if (value < 1)  // zs frames has to be >= 1
                {
                    value = 1;
                }

                this._RunSampleLS.ZStreamFrames = value;

                OnPropertyChanged("ZStreamFrames");
                OnPropertyChanged("TTotalTime");
                OnPropertyChanged("DriveSpace");
            }
        }

        #endregion Properties

        #region Methods

        public bool BuildChannelPalettes()
        {
            return _RunSampleLS.BuildChannelPalettes();
        }

        public void CleanupOnQuit()
        {
            // make sure z file read button is off
            ZFileEnable = 0;
            // clear Z2Stage lock for safety reasons
            Z2StageLock = false;
        }

        /// <summary>
        /// Show or hide the  RunSampleLS view
        /// </summary>
        /// <param name="command"></param>
        public void CommandEventHandler(Command command)
        {
        }

        public void ConnectHandlers()
        {
            this._RunSampleLS.ConnectCallbacks();
        }

        public void ConnectionSettingsOptions()
        {
            EditPipeDialog editPipeDialog = new EditPipeDialog();
            editPipeDialog.DataContext = this;
            editPipeDialog.ShowDialog();
        }

        public void EnableHandlers()
        {
            this._RunSampleLS.Update += new Action<string>(RunSampleLS_Update);
            this._RunSampleLS.UpdateBitmapTimer += new Action<bool>(_RunSampleLS_UpdateBitmapTimer);
            this._RunSampleLS.UpdateImage += new Action<string>(RunSampleLS_UpdateImage);
            this._RunSampleLS.UpdateImageName += new Action<string>(RunSampleLS_UpdateImageName);
            this._RunSampleLS.UpdateStart += new Action<string>(RunSampleLS_UpdateStart);
            this._RunSampleLS.UpdateStartSubWell += new Action<string>(RunSampleLS_UpdateStartSubWell);
            this._RunSampleLS.UpdateEndSubWell += new Action<string>(RunSampleLS_UpdateEndSubWell);
            this._RunSampleLS.UpdateButton += new Action<bool>(RunSampleLS_UpdateButton);
            this._RunSampleLS.UpdatePanels += new Action<bool>(RunSampleLS_UpdatePanels);
            this._RunSampleLS.UpdateMenuBarButton += new Action<bool>(RunSampleLS_UpdateMenuBarButton);
            this._RunSampleLS.UpdateScript += new Action<string>(RunSampleLS_UpdateScript);
            this._RunSampleLS.ExperimentStarted += new Action(RunSampleLS_ExperimentStarted);
            this._RunSampleLS.UpdateSequenceStepDisplaySettings += new Action(_RunSampleLS_UpdateSequenceStepDisplaySettings);
            this._RunSampleLS.EnableHandlers();
        }

        public Color GetColorAssignment(int index)
        {
            return RunSampleLS.GetColorAssignment(index);
        }

        public bool GetLSMFieldSizeCalibration(ref double calibration)
        {
            return this._RunSampleLS.LIGetFieldSizeCalibration(ref calibration);
        }

        public void InitIPC()
        {
            this.RunSampleLS.LoadRemotePCHostNameFromXML();
            if (SelectedRemotePCNameIndex >= 0)
            {
                if (IDMode == 0)
                {
                    if (this.RunSampleLS._selectRemotePCName[SelectedRemotePCNameIndex] != "")
                    {
                        //only reconnect if the host name is different
                        if (false == this.RunSampleLS._selectRemotePCName[SelectedRemotePCNameIndex].Equals(RemotePCHostName))
                        {
                            RemotePCHostName = this.RunSampleLS._selectRemotePCName[SelectedRemotePCNameIndex];
                        }
                    }
                    else
                    {
                        RemotePCHostName = LocalPCHostName;
                    }

                }
                else if (IDMode == 1)
                {
                    if (this.RunSampleLS._selectRemodePCIPAddr[SelectedRemotePCNameIndex] != "")
                    {
                        //only reconnect if the host id is different
                        if (false == this.RunSampleLS._selectRemodePCIPAddr[SelectedRemotePCNameIndex].Equals(RemotePCHostName))
                        {
                            RemotePCHostName = this.RunSampleLS._selectRemodePCIPAddr[SelectedRemotePCNameIndex];
                        }
                    }
                    else
                    {
                        RemotePCHostName = this.RunSampleLS.GetLocalIP();
                    }

                }
            }
        }

        public bool IsCamera()
        {
            return this._RunSampleLS.ActiveCameraType == 0; // 0 for CCD
        }

        public void LoadXMLSettings()
        {
            //here to conform to the ThorSharedTypes.IMVM interface
        }

        // refresh CaptureSetup UI
        public void RefereshCaptureSetup()
        {
            Command command = new Command();
            command.Message = "Capture Setup";
            command.CommandGUID = new Guid("{6ecb028e-754e-4b50-b0ef-df8f344b668e}");

            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
        }

        //public ICommand UpdateActiveXML
        //{
        //    get
        //    {
        //        if (this._RunSampleLS.RunComplete)
        //            this._RunSampleLSStartCommand = new RelayCommand(() => RefreshRunSampleLS());
        //        return this._RunSampleLSStartCommand;
        //    }
        //}
        public void RefreshRunSampleLS()
        {
            Command command = new Command();
            command.Message = "Run Sample LS";
            command.CommandGUID = new Guid("30db4357-7508-46c9-84eb-3ca0900aa4c5");

            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
        }

        public void ReleaseBleachMem()
        {
            this._RunSampleLS.ReleaseBleach();
        }

        public void ReleaseHandlers()
        {
            this._RunSampleLS.Update -= new Action<string>(RunSampleLS_Update);
            this._RunSampleLS.UpdateBitmapTimer -= new Action<bool>(_RunSampleLS_UpdateBitmapTimer);
            this._RunSampleLS.UpdateImage -= new Action<string>(RunSampleLS_UpdateImage);
            this._RunSampleLS.UpdateImageName -= new Action<string>(RunSampleLS_UpdateImageName);
            this._RunSampleLS.UpdateStart -= new Action<string>(RunSampleLS_UpdateStart);
            this._RunSampleLS.UpdateStartSubWell -= new Action<string>(RunSampleLS_UpdateStartSubWell);
            this._RunSampleLS.UpdateEndSubWell -= new Action<string>(RunSampleLS_UpdateEndSubWell);
            this._RunSampleLS.UpdateButton -= new Action<bool>(RunSampleLS_UpdateButton);
            this._RunSampleLS.UpdatePanels -= new Action<bool>(RunSampleLS_UpdatePanels);
            this._RunSampleLS.UpdateMenuBarButton -= new Action<bool>(RunSampleLS_UpdateMenuBarButton);
            this._RunSampleLS.UpdateScript -= new Action<string>(RunSampleLS_UpdateScript);
            this._RunSampleLS.ExperimentStarted -= new Action(RunSampleLS_ExperimentStarted);
            this._RunSampleLS.UpdateSequenceStepDisplaySettings -= new Action(_RunSampleLS_UpdateSequenceStepDisplaySettings);

            //calling the release handlers present in the model
            _RunSampleLS.ReleaseHandlers();

            //close any floating panels or windows
            if (null != _roiStatsChart)
            {
                _roiStatsChart.Close();
            }
        }

        public void ResetImage()
        {
            this._RunSampleLS.ResetImage();
            this._bitmap = null;
            OnPropertyChanged("Bitmap");
        }

        public void RunSampleLSView_UpdateStageLocation()
        {
            OnPropertyChanged("XPosition");
            OnPropertyChanged("YPosition");
        }

        /// <summary>
        /// Capture events for showing the commands view 
        /// </summary>
        public void SubscribeToCommandEvent()
        {
            CommandShowDialogEvent commandEvent = _eventAggregator.GetEvent<CommandShowDialogEvent>();

            if (_subscriptionToken != null)
            {
                commandEvent.Unsubscribe(_subscriptionToken);
            }

            _subscriptionToken = commandEvent.Subscribe(CommandEventHandler, ThreadOption.UIThread, false);
        }

        public void UpdateCameraUIPanel()
        {
            // update time series combo group box panel
            OnPropertyChanged("TFrames");
            OnPropertyChanged("TInterval");
            OnPropertyChanged("CurrentTCount");
            OnPropertyChanged("TriggerModeTimelapse");
            OnPropertyChanged("TTotalTime");

            // update streaming group panel
            OnPropertyChanged("StreamFrames");
            OnPropertyChanged("TriggerModeStreaming");
            OnPropertyChanged("StreamTotalTime");
            OnPropertyChanged("StreamFramesTime");
            OnPropertyChanged("DMAFramesTime");
            OnPropertyChanged("StimulusMaxFramesTime");
        }

        public void UpdateExperimentFile()
        {
            string str = string.Empty;
            this._RunSampleLS.UpdateExperimentFile();

            this._RunSampleLS.ExperimentDoc.Save(ResourceManagerCS.GetCaptureTemplatePathString() + "\\Active.xml");

            if (0 == StartAfterLoading.Count)
            {
                //Copy the experiment file to the modality folder while not in scipt mode
                //read the modality from the experiment file since
                //unload can be asynchronous and the get modality function could be incorrect
                //for the experiment settings

                XmlNodeList nodeList = this._RunSampleLS.ExperimentDoc.SelectNodes("/ThorImageExperiment/Modality");

                if (nodeList.Count > 0)
                {
                    if (XmlManager.GetAttribute(nodeList[0], this._RunSampleLS.ExperimentDoc, "name", ref str))
                    {
                        try
                        {
                            File.Copy(ResourceManagerCS.GetCaptureTemplatePathString() + "\\Active.xml", ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "\\Modalities\\" + str + "\\Active.xml", true);
                        }
                        catch (Exception ex)
                        {
                            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, ex.Message);
                        }
                    }
                }
            }
        }

        public void UpdateExperimentFileNewModality()
        {
            this._RunSampleLS.UpdateExperimentFileNewModality();
        }

        public void UpdateExpXMLSettings(ref XmlDocument xmlDoc)
        {
            //here to conform to the ThorSharedTypes.IMVM interface
        }

        public void UpdateUIBindings()
        {
            this.OnPropertyChanged("");
        }

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetLSM", CharSet = CharSet.Unicode)]
        private static extern int GetLSM(ref int areaMode, ref double areaAngle, ref int scanMode, ref int pixelX, ref int pixelY, ref int channel, ref int fieldSize, ref int offsetX, ref int offsetY,
            ref int averageMode, ref int averageNum, ref int clockSource, ref int inputRange1, ref int inputRange2, ref int twoWayAlignment, ref int extClockRate, ref double dwellTime, ref int flybackCycles, ref int inputRange3, ref int inputRange4, ref int minimizeFlybackCycles);

        [DllImport(".\\StatsManager.dll", EntryPoint = "GetNumROI")]
        private static extern int GetNumROI();

        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        static extern bool GlobalMemoryStatusEx([In, Out] MEMORYSTATUSEX lpBuffer);

        [DllImport(".\\ROIDataStore.dll", EntryPoint = "RequestROIData")]
        private static extern void RequestROIData();

        private void BitmapTimer_Tick(object sender, EventArgs e)
        {
            OnPropertyChanged("Bitmap");
            RequestROIData();
        }

        private void CreateBitmap()
        {
            if ((false == this._RunSampleLS.IsDisplayImageReady) && (false == _paletteChanged))
            {
                return;
            }

            _bitmap = this._RunSampleLS.CreateBitmap();
            _paletteChanged = false;

            this._RunSampleLS.IsDisplayImageReady = false;

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " " + "Bitmap Udpated");
        }

        private bool CreateStatsChartWindow()
        {
            //Ignore if no ROIs:
            if (0 == GetNumROI())
            { return false; }

            if (null == _roiStatsChart)
            {
                _roiStatsChart = new ROIStatsChartWin();
                _roiStatsChart.DataContext = this;
                _roiStatsChart.Closed += _rOIStatsChart_Closed;

                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");

                if (node != null)
                {
                    string str = string.Empty;
                    double dVal = 0;
                    if (XmlManager.GetAttribute(node, appSettings, "left", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        _roiStatsChart.Left = dVal;
                    }
                    else
                    {
                        _roiStatsChart.Left = 0;
                    }
                    if (XmlManager.GetAttribute(node, appSettings, "top", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        _roiStatsChart.Top = dVal;
                    }
                    else
                    {
                        _roiStatsChart.Top = 0;
                    }
                    if (XmlManager.GetAttribute(node, appSettings, "width", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        _roiStatsChart.Width = dVal;
                    }
                    else
                    {
                        _roiStatsChart.Width = 400;
                    }
                    if (XmlManager.GetAttribute(node, appSettings, "height", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        _roiStatsChart.Height = dVal;
                    }
                    else
                    {
                        _roiStatsChart.Height = 400;
                    }
                }

                XmlDocument hwSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
                XmlNodeList ndListHW = hwSettings.SelectNodes("/HardwareSettings/Wavelength");
                List<string> chanNames = new List<string>();
                List<bool> chanEnable = new List<bool>();
                for (int ich = 0; ich < ndListHW.Count; ich++)
                {
                    chanNames.Add(ndListHW[ich].Attributes["name"].Value);
                    chanEnable.Add(true);
                }
                _roiStatsChart.roiChart.SetChartXLabel("Time [sec]");
                _roiStatsChart.roiChart.ClearChart();
                _roiStatsChart.roiChart.SetLegendGroup(2, chanNames.ToArray(), chanEnable.ToArray());
                //string[] featureNames = { "mean", "stddev", "min", "max" };         //, "left", "top", "width", "height"
                //bool[] featureEnable = { true, true, true, true };                  //, false, false, false, false

                List<string> featureNames = new List<string>() { "mean", "stddev", "min", "max" };
                List<bool> featureEnable = new List<bool>() { true, true, true, true };                  //, false, false, false, false

                if (1 == RunSampleLS.ImageMethod) //if DFLIM
                {
                    featureNames.Add("tbar");
                    featureEnable.Add(true);
                }

                _roiStatsChart.roiChart.SetLegendGroup(1, featureNames.ToArray(), featureEnable.ToArray());
                _roiStatsChart.roiChart.SetClockAsXAxis(true);
                _roiStatsChart.roiChart.SetFifoVisible(true);
                _roiStatsChart.roiChart.SetInLoading(true);
                _roiStatsChart.roiChart.LoadSettings();

                if (true == _roiStatsChartActive)
                {
                    _roiStatsChart.Show();
                }
            }
            else
            {
                if (true == _roiStatsChartActive && false == _roiStatsChart.IsActive)
                {
                    _roiStatsChart.Show();
                }
            }
            return true;
        }

        private void DisplayROIStatsOptions()
        {
            ActiveStatsWinChooser activeStatsWinChooser = new ActiveStatsWinChooser(true, false, false);
            activeStatsWinChooser.DataContext = this;
            activeStatsWinChooser.IsChartChecked = _roiStatsChartActive;
            activeStatsWinChooser.ShowDialog();
            _roiStatsChartActive = activeStatsWinChooser.IsChartChecked;

            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            if (null != appSettings)
            {
                XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");

                if (node != null)
                {
                    string str = (true == _roiStatsChartActive) ? "1" : "0";

                    XmlManager.SetAttribute(node, appSettings, "display", str);
                }
            }
            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
        }

        //Check and fit to the size of the ZStep
        private void EnsureValidZScanStop()
        {
            if (ZStopPosition != ZStartPosition)
            {
                double rem = Math.Round(Math.Abs((ZStopPosition - ZStartPosition)) % (ZStepSize / 1000), 4);
                if (0 != rem)
                {
                    int zDirection = 0;
                    if (ZStopPosition > ZStartPosition)
                    {
                        zDirection = 1;
                    }
                    else
                    {
                        zDirection = -1;
                    }
                    double newZScanStop = Math.Round(ZStopPosition + zDirection * (ZStepSize / 1000 - Math.Abs((ZStopPosition - ZStartPosition)) % (ZStepSize / 1000)), 4);
                    if (newZScanStop <= this._RunSampleLS.ZMin)
                    {
                        _RunSampleLS.ZStopPosition = this._RunSampleLS.ZMin;
                    }
                    else if (newZScanStop >= this._RunSampleLS.ZMax)
                    {
                        _RunSampleLS.ZStopPosition = this._RunSampleLS.ZMax;
                    }
                    else
                    {
                        _RunSampleLS.ZStopPosition = newZScanStop;
                    }

                    OnPropertyChanged("ZStopPosition");

                    if (newZScanStop != ZStopPosition)
                    {
                        ZStopPositionNotValid = true;
                    }
                    else
                    {
                        ZStopPositionNotValid = false;
                    }
                }
                else
                {
                    ZStopPositionNotValid = false;
                }
            }
        }

        /// <summary>
        /// Expands all histograms to their maximum size
        /// </summary>
        void ExpandAllHistograms()
        {
            HistogramHeight1 = MAX_HISTOGRAM_HEIGHT;
            HistogramWidth1 = MAX_HISTOGRAM_WIDTH;
            HistogramHeight2 = MAX_HISTOGRAM_HEIGHT;
            HistogramWidth2 = MAX_HISTOGRAM_WIDTH;
            HistogramHeight3 = MAX_HISTOGRAM_HEIGHT;
            HistogramWidth3 = MAX_HISTOGRAM_WIDTH;
            HistogramHeight4 = MAX_HISTOGRAM_HEIGHT;
            HistogramWidth4 = MAX_HISTOGRAM_WIDTH;
        }

        /// <summary>
        /// Calculates the needed disk space for a streaming capture saved in tiff to be captured
        /// </summary>
        /// <param name="width"> Width to capture </param>
        /// <param name="height"> Height to capture </param>
        /// <param name="depth"> Depth to capture </param>
        /// <param name="chan"> Number of channels to capture </param>
        /// <param name="rawChan"> Number of channels that will be stored in the raw file </param>
        /// <param name="frames"> The number of capture frames to take </param>
        /// <returns> Size in megabytes </returns>
        private long GetNeededDiskSpaceForStreaming(long width, long height, long depth, int chan, int rawChan, int frames)
        {
            const int BYTES_PER_PIXEL = 2;
            const double BYTES_TO_MEGABYTES = 1048576.0;

            long rawSize = GetNeededDiskSpaceForStreamingRaw(width, height, depth, chan, rawChan, frames);

            //streaming requires the disk space for a raw file plus the tiff files
            //because of the transformation process to individual files
            double total = rawSize + width * height * depth * chan * frames * BYTES_PER_PIXEL / BYTES_TO_MEGABYTES;

            return Convert.ToInt64(total);
        }

        /// <summary>
        /// Calculates the needed disk space for a raw streaming capture
        /// </summary>
        /// <param name="width"> Width to capture </param>
        /// <param name="height"> Height to capture </param>
        /// <param name="depth"> Depth to capture </param>
        /// <param name="chan"> Number of channels to capture </param>
        /// <param name="rawChan"> Number of channels that will be stored in the raw file </param>
        /// <param name="frames"> The number of capture frames to take </param>
        /// <returns> Size in megabytes </returns>>
        private long GetNeededDiskSpaceForStreamingRaw(long width, long height, long depth, int chan, int rawChan, int frames)
        {
            const int BYTES_PER_PIXEL = 2;
            const double BYTES_TO_MEGABYTES = 1048576.0;

            double rawSize = width * height * depth * rawChan * frames * BYTES_PER_PIXEL / BYTES_TO_MEGABYTES;
            return Convert.ToInt64(rawSize);
        }

        private string GetWellsTagByPoint(Point point)
        {
            string tag = "A1";
            if (CarrierType != "Slide")
            {
                int column = (CarrierCenterToCenterDistanceX != 0) ? (int)Math.Round(Math.Abs((point.X - XYHomeOffsetX)) / CarrierCenterToCenterDistanceX) : 0;
                int row = (CarrierCenterToCenterDistanceY != 0) ? (int)Math.Round(Math.Abs((point.Y - XYHomeOffsetY)) / CarrierCenterToCenterDistanceY) : 0;
                return TagWells(row, column);
            }
            else
            {
                tag = "Slide";
            }
            return tag;
        }

        private bool IsOutputPathValid()
        {
            bool ret = false;
            if (Directory.Exists(OutputPath))
            {
                DirectoryInfo di = new DirectoryInfo(OutputPath);
                if ((di.Attributes & FileAttributes.ReadOnly) == FileAttributes.ReadOnly)
                {
                    MessageBox.Show("Invalid OutputPath: The output path is read only.");
                }
                else
                {
                    ret = true;
                }
            }
            else
            {
                MessageBox.Show("Invalid OutputPath: The output path doesn't exist.");
            }

            //validate experiment name without tailing space
            char[] charsToTrim = { ' ', '\t' };
            this._RunSampleLS.ExperimentName.FullName = this._RunSampleLS.ExperimentName.FullName.Trim(charsToTrim);
            return ret;
        }

        private bool PreBleachParamsSetup()
        {
            string bROISource = ResourceManagerCS.GetCaptureTemplatePathString() + "BleachROIs.xaml";
            List<string> fileList;
            switch (CurrentBleachMode)
            {
                case BleachMode.BLEACH:
                    if (File.Exists(bROISource))
                    {
                        _RunSampleLS.BleachScanMode = BleachMode.BLEACH;
                        _RunSampleLS.BleachLongIdle = 0;

                        string str = string.Empty;
                        XmlDocument doc = new XmlDocument();
                        XmlTextReader reader = new XmlTextReader(bROISource);
                        doc.Load(reader);
                        XmlNodeList xnodes = doc.DocumentElement.ChildNodes.Item(0).ChildNodes.Item(0).ChildNodes;
                        for (int i = 0; i < xnodes.Count; i++)
                        {
                            int val;
                            Int32.TryParse(xnodes[i].ChildNodes[0].ChildNodes[0].ChildNodes[(int)ThorSharedTypes.Tag.MODE].ChildNodes[0].Value, out val);
                            BitVector32 bVec = new BitVector32(val);
                            if ((int)ThorSharedTypes.Mode.STATSONLY == bVec[BitVector32.CreateSection(255)])
                            {
                                BleachClass.GetBleachAttribute(xnodes[i], doc, 8, ref str);
                                _RunSampleLS.BleachLongIdle = Convert.ToDouble(str, CultureInfo.InvariantCulture);
                                break;
                            }
                        }
                        if (_RunSampleLS.BleachLongIdle > 0)
                        {
                            ROICapsule roiCapsule = OverlayManagerClass.LoadXamlROIs(bROISource);
                            if ((roiCapsule == null) || (roiCapsule.ROIs.Length == 0))
                            {
                                return false;
                            }

                            XmlDocument expDoc = new XmlDocument();
                            expDoc.Load(ActiveExperimentPath);
                            XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/Photobleaching");
                            double bleachLSMUMPerPixel = 1.0;
                            if (0 < ndList.Count)
                            {
                                if (XmlManager.GetAttribute(ndList[0], expDoc, "pixelSizeUM", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out bleachLSMUMPerPixel)))
                                {
                                    _RunSampleLS.BleachWParams = WaveformBuilder.LoadBleachWaveParams(bROISource, roiCapsule, this.PixelSizeUM, this.PixelSizeUM / bleachLSMUMPerPixel, this.BinX, this.BinY);
                                }
                                else
                                {
                                    _RunSampleLS.BleachWParams = WaveformBuilder.LoadBleachWaveParams(bROISource, roiCapsule, this.PixelSizeUM, 1.0, this.BinX, this.BinY);
                                }
                            }
                        }
                    }
                    break;
                case BleachMode.SLM:
                    if (!Directory.Exists(_RunSampleLS.SLMWaveformFolder[0]))
                    {
                        MessageBox.Show("Unable to locate SLMWaveforms folder.");
                        return false;
                    }
                    fileList = Directory.EnumerateFiles(_RunSampleLS.SLMWaveformFolder[0], "*.bmp ", SearchOption.TopDirectoryOnly).ToList();
                    if (0 >= fileList.Count)
                    {
                        MessageBox.Show("Unable to locate any SLM patterns. Please go to Capture Setup to create SLM patterns.");
                        return false;
                    }
                    if (SLMSequenceOn)
                    {
                        if (Directory.Exists(_RunSampleLS.SLMWaveformFolder[1]))
                        {
                            fileList = Directory.EnumerateFiles(_RunSampleLS.SLMWaveformFolder[1], "*.raw ", SearchOption.TopDirectoryOnly).ToList();
                            if (0 >= fileList.Count)
                            {
                                MessageBox.Show("Unable to locate any SLM sequences. Please go to Capture Setup to create SLM sequences.");
                                return false;
                            }
                        }
                    }
                    else if (Directory.Exists(_RunSampleLS.SLMWaveformFolder[0]))
                    {
                        fileList = Directory.EnumerateFiles(_RunSampleLS.SLMWaveformFolder[0], "*.raw ", SearchOption.TopDirectoryOnly).ToList();
                        if (0 >= fileList.Count)
                        {
                            MessageBox.Show("Unable to locate any SLM bleaching waveforms. Please go to Capture Setup to create a SLM waveform.");
                            return false;
                        }
                        else
                        {
                            _RunSampleLS.BleachLongIdle = 0;
                            _RunSampleLS.BleachScanMode = BleachMode.SLM;
                        }
                    }
                    break;
                default:
                    MessageBox.Show("Unable to locate any bleaching waveform. Please go to Capture Setup to create a waveform.");
                    return false;
            }
            return true;
        }

        /// <summary>
        /// Refreshes the bitmap and ROI data
        /// </summary>
        private void RefreshBitmap(object sender, EventArgs e)
        {
            OnPropertyChanged("Bitmap");
            if (null != HistogramPanelUpdate) HistogramPanelUpdate(true);
            RequestROIData();
        }

        private void RunSampleLSStart()
        {
            if (0 == this.ChannelSelection)
            {
                MessageBox.Show("No channel is enabled. Enable at least one channel to start.");
                return;
            }

            if (!IsOutputPathValid() || RunSampleLS.IsRunning())
                return;

            UpdateExperimentFile();
            double outputDriveSpace = 0;
            const double BYTES_TO_MEGABYTES = 1048576.0;

            OnPropertyChanged("DriveSpace");

            System.IO.DriveInfo[] allDrives = System.IO.DriveInfo.GetDrives();

            foreach (System.IO.DriveInfo d in allDrives)
            {
                if (d.DriveType.Equals(System.IO.DriveType.Fixed))
                {

                    try
                    {
                        outputDriveSpace = d.AvailableFreeSpace / BYTES_TO_MEGABYTES;
                        if (d.Name == (OutputPath[0] + ":\\"))
                        {
                            break;
                        }
                    }
                    catch (System.Exception ex)
                    {
                        string strErr = ex.Message;
                        //suppress this exception and continue
                    }
                }
            }

            if (_reqDiskSize > outputDriveSpace)
            {
                MessageBox.Show("Not enough Disk space, please update the settings or  use a disk with enough space before starting capture.", "Error: Low Disk Space", MessageBoxButton.OK, MessageBoxImage.Error);
                RunSampleLS_UpdateScript("Error");
                return;
            }

            // read z-position list if read from file enabled
            // do this irrespective of whether the file has been read before
            if (1 == ZFileEnable)
            {
                if (!RunSampleLS.getZPosFromFile()) return;
                OnPropertyChanged("ZNumSteps");
            }

            // clearing the previous image if exists from the image viewer control
            ResetImage();

            // clearing the previous image name if exists from the image viewer control
            this._wellImageName = null;
            OnPropertyChanged("WellImageName");

            //reset the chart
            if (null != _roiStatsChart)
            {
                _roiStatsChart.roiChart.ClearChart();
            }

            CurrentSubImageRow = 1;
            CurrentSubImageCol = 1;

            //Pre-bleach check:
            if ((int)CaptureModes.BLEACHING == CaptureMode)
            {
                if (!PreBleachParamsSetup())
                {
                    RunSampleLS_UpdateScript("Error");
                    return;
                }
            }

            //[Note] Active.xml is copied to experiment after _RunSampleLS.Start
            if (!_RunSampleLS.Start())
            {
                RunSampleLS_UpdateScript("Error");
            }
            if (null != PaletteChanged) PaletteChanged(true);

            //allow the shiftvalue to be updated after start is called by resetting the counter
            _nPixelBitShiftValueUpdates = 0;
        }

        private void RunSampleLSStop()
        {
            this._RunSampleLS.Stop();
            _bitmapRefreshTimer.Stop();
        }

        void RunSampleLS_ExperimentStarted()
        {
            OverlayManagerClass.Instance.SaveROIs(_RunSampleLS.ExperimentFolderPath + "ROIs.xaml");
            OverlayManagerClass.Instance.SaveMaskToPath(_RunSampleLS.ExperimentFolderPath + "ROIMask.raw");
            _newExperiment = true;
            BitmapReady = true;
            // The experiment index does not get updated here, it gets updated in the Start() function.
            OnPropertyChanged("ExperimentName");
            OnPropertyChanged("ExperimentIndex");
        }

        void RunSampleLS_Update(string statusMessage)
        {
            //We need to throttle the GUI image count update for fast frame rates (more than 200fps).
            //The throttle needs to stop before the end of acquisition (one frame before) otherwise the GUI might miss the final frame and won't know it finished
            if (CompletedImageCount < (TotalImageCount - 1) && 200 < _RunSampleLS.FrameRate)
            {
                TimeSpan ts = DateTime.Now - _lastUpdate;
                if (0.5 < ts.TotalSeconds)
                {
                    _lastUpdate = DateTime.Now;
                }
                else
                {
                    return;
                }
            }

            OnPropertyChanged("TotalImageCount");
            OnPropertyChanged("CurrentImageCount");
            OnPropertyChanged("CompletedImageCount");
            OnPropertyChanged("PercentComplete");
            OnPropertyChanged("StatusMessage");
            OnPropertyChanged("CurrentZCount");
            OnPropertyChanged("CurrentTCount");
            OnPropertyChanged("ZPosition");
            OnPropertyChanged("StimulusSaveBtnDisplay");
            OnPropertyChanged("XPosition");
            OnPropertyChanged("YPosition");
            OnPropertyChanged("Power0");
            OnPropertyChanged("Power1");
            OnPropertyChanged("Power2");
            OnPropertyChanged("Power3");
            OnPropertyChanged("Power4");
            OnPropertyChanged("Power5");

            //The bitsPerPixel might not be updated in the camera for the first Update call
            //Allow two updates to esure the right BitsPerPixel value is used to calculate
            //the PixelBitShiftValue
            if (2 > _nPixelBitShiftValueUpdates)
            {
                _RunSampleLS.UpdateCamBitsPerPixel();
                OnPropertyChanged("PixelBitShiftValue");
                _nPixelBitShiftValueUpdates++;
            }
        }

        void RunSampleLS_UpdateButton(bool status)
        {
            OnPropertyChanged("StartButtonStatus");
            OnPropertyChanged("StopButtonStatus");
        }

        void RunSampleLS_UpdateEndSubWell(string statusMessage)
        {
            OnPropertyChanged("CurrentSubImageCount");
        }

        void RunSampleLS_UpdateImage(string image)
        {
            OnPropertyChanged("Bitmap");
        }

        void RunSampleLS_UpdateImageName(string wellImageName)
        {
            _wellImageName = wellImageName;
            OnPropertyChanged("WellImageName");
        }

        void RunSampleLS_UpdateMenuBarButton(bool status)
        {
            bool btnStatus = status;
            ChangeEvent changeEvent = new ChangeEvent();
            changeEvent.ModuleName = "RunSampleLS";

            if (btnStatus)
            {
                changeEvent.IsChanged = true;
            }
            else
            {
                changeEvent.IsChanged = false;
            }

            //command published to change the status of the menu buttons in the Menu Control
            _eventAggregator.GetEvent<MenuModuleChangeEvent>().Publish(changeEvent);
        }

        void RunSampleLS_UpdatePanels(bool status)
        {
            OnPropertyChanged("PanelsEnable");
        }

        void RunSampleLS_UpdateRemoteConnection(bool obj)
        {
            OnPropertyChanged("RemoteConnection");
        }

        void RunSampleLS_UpdateScript(string obj)
        {
            //Send a command to the event aggregator if the
            //the capture was initiated with a script
            if (StartAfterLoading.Count > 0)
            {

                const string str = "1F914CCD-33DE-4f40-907A-4511AA145D8A";

                Command command = new Command();
                command.Message = "ScriptManager";
                command.CommandGUID = new Guid(str);
                command.Payload = new List<string>();

                if (obj.Equals("Complete"))
                {
                    command.Payload.Add("Complete");

                    this._RunSampleLS.PanelsEnable = false;
                    //disable Panels while the script is complete but still in the capture Tab
                    RunSampleLS_UpdatePanels(true);
                }
                else if (obj.Equals("Stop") || obj.Equals("Error"))
                {
                    command.Payload.Add("Error");

                    //notify the menus etc... that the script has completed.
                    Command finishedCommand = new Command();
                    finishedCommand.Message = "Finished";
                    _eventAggregator.GetEvent<CommandFinishedScriptEvent>().Publish(finishedCommand);
                }

                _eventAggregator.GetEvent<CommandFinishedDialogEvent>().Publish(command);
            }
        }

        void RunSampleLS_UpdateStart(string statusMessage)
        {
            OnPropertyChanged("CurrentWellCount");
        }

        void RunSampleLS_UpdateStartSubWell(string statusMessage)
        {
            ScanAreaWellLocation = GetWellsTagByPoint(new Point(XPosition, YPosition));
        }

        /// <summary>
        /// Shrinks all histograms to their minimum size
        /// </summary>
        void ShrinkAllHistograms()
        {
            HistogramHeight1 = MIN_HISTOGRAM_HEIGHT;
            HistogramWidth1 = MIN_HISTOGRAM_WIDTH;
            HistogramHeight2 = MIN_HISTOGRAM_HEIGHT;
            HistogramWidth2 = MIN_HISTOGRAM_WIDTH;
            HistogramHeight3 = MIN_HISTOGRAM_HEIGHT;
            HistogramWidth3 = MIN_HISTOGRAM_WIDTH;
            HistogramHeight4 = MIN_HISTOGRAM_HEIGHT;
            HistogramWidth4 = MIN_HISTOGRAM_WIDTH;
        }

        private void StimulusSave()
        {
            //Change the state of Saving:
            if (false == this.IsStimulusSaving)
            { this.IsStimulusSaving = true; }
            else
            { this.IsStimulusSaving = false; }
        }

        private string TagWells(int row, int column)
        {
            // A1  A2  A3 .....
            // B1  B2  B3 .....
            // .
            // Z1  Z2  Z3 ......
            // AA1 AA2 AA3......
            //.
            // AZ1 AZ2 AZ3......
            string tag = string.Empty;
            //676( 26*26 A-Z,AA- AZ,...,ZA-ZZ) rows at most
            if (row >= 26)
            {
                tag += Convert.ToChar(64 + (int)row / 26);
            }
            tag += Convert.ToChar(65 + row % 26) + (column + 1).ToString();
            return tag;
        }

        /// <summary>
        /// Runs when the tile control has loaded a new experiment
        /// </summary>
        /// <param name="obj"></param>
        private void tileControl_TilesLoaded(object sender, RoutedEventArgs e)
        {
            OnPropertyChanged("DriveSpace");
        }

        void _bitmapWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;

            while (true)
            {
                if ((worker.CancellationPending == true))
                {
                    e.Cancel = true;
                    break;
                }
                else
                {
                    // Perform a time consuming operation and report progress.
                    System.Threading.Thread.Sleep(30);
                    worker.ReportProgress(0);
                }
            };
        }

        void _bitmapWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if ((e.Cancelled == true))
            {
            }

            else if (!(e.Error == null))
            {
            }

            else
            {
            }
        }

        void _rOIStatsChart_Closed(object sender, EventArgs e)
        {
            if (!UnloadWhole) ROIStatsChartActive = false;

            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            if (null != appSettings)
            {
                XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");

                if (node != null)
                {
                    string str = string.Empty;

                    XmlManager.SetAttribute(node, appSettings, "left", ((int)Math.Round(_roiStatsChart.Left)).ToString());
                    XmlManager.SetAttribute(node, appSettings, "top", ((int)Math.Round(_roiStatsChart.Top)).ToString());
                    XmlManager.SetAttribute(node, appSettings, "width", ((int)Math.Round(_roiStatsChart.Width)).ToString());
                    XmlManager.SetAttribute(node, appSettings, "height", ((int)Math.Round(_roiStatsChart.Height)).ToString());
                    XmlManager.SetAttribute(node, appSettings, "display", (((ROIStatsChartActive) ? 1 : 0).ToString()));

                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
                }
            }
            _roiStatsChartActive = false;
            _roiStatsChart.roiChart.ClearChart();

            _roiStatsChart = null;
        }

        void _RunSampleLS_ROIStatsChanged(object sender, EventArgs e)
        {
            if (CreateStatsChartWindow())
            {
                _roiStatsChart.SetData(this._RunSampleLS.StatsNames, this.RunSampleLS.Stats, true);
            }
        }

        void _RunSampleLS_UpdataFilePath()
        {
            OnPropertyChanged("OutputPath");
            OnPropertyChanged("ExperimentName");
        }

        void _RunSampleLS_UpdateBitmapTimer(bool timer)
        {
            if ((timer) && (this.ImageUpdaterVisibility == Visibility.Visible))
            {
                _bitmapRefreshTimer.Start();
            }
            else
            {
                _bitmapRefreshTimer.Stop();
            }
        }

        void _RunSampleLS_UpdateSequenceStepDisplaySettings()
        {
            OnPropertyChanged("MCLS1Power");
            OnPropertyChanged("MCLS1Visibility");
            OnPropertyChanged("MCLS2Power");
            OnPropertyChanged("MCLS2Visibility");
            OnPropertyChanged("MCLS3Power");
            OnPropertyChanged("MCLS3Visibility");
            OnPropertyChanged("MCLS4Power");
            OnPropertyChanged("MCLS4Visibility");
            OnPropertyChanged("PinholePosition");
        }

        void _RunSampleLS_UpdateTImage(string obj)
        {
            OnPropertyChanged("Bitmap");
        }

        void _RunSampleLS_UpdateZImage(string obj)
        {
            OnPropertyChanged("Bitmap");
        }

        #endregion Methods

        #region Nested Types

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
        private class MEMORYSTATUSEX
        {
            public uint dwLength;
            public uint dwMemoryLoad;
            public ulong ullTotalPhys;
            public ulong ullAvailPhys;
            public ulong ullTotalPageFile;
            public ulong ullAvailPageFile;
            public ulong ullTotalVirtual;
            public ulong ullAvailVirtual;
            public ulong ullAvailExtendedVirtual;

            #region Constructors

            public MEMORYSTATUSEX()
            {
                this.dwLength = (uint)Marshal.SizeOf(typeof(MEMORYSTATUSEX));
            }

            #endregion Constructors
        }

        #endregion Nested Types

        #region Other

        //private double _whitePoint;

        #endregion Other
    }

    public class StorageModeVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(Visibility))
                throw new InvalidOperationException("The target must be a Visibility");

            Visibility ret = Visibility.Collapsed;

            switch (System.Convert.ToInt32(value))
            {
                case 0:
                    {
                        ret = (0 == System.Convert.ToInt32(parameter)) ? Visibility.Visible : Visibility.Collapsed;
                    }
                    break;
                case 1:
                    {
                        ret = (1 == System.Convert.ToInt32(parameter)) ? Visibility.Visible : Visibility.Collapsed;
                    }
                    break;
                default:
                    {
                    }
                    break;
            }

            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class TriggerModeTimelapseEnableConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(bool))
                throw new InvalidOperationException("The target must be a boolean");

            bool ret = false;
            switch (System.Convert.ToInt32(value))
            {
                case 0:
                case 1:
                    {
                        ret = true;
                    }
                    break;
                default:
                    {
                        ret = false;
                    }
                    break;
            }

            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }
}