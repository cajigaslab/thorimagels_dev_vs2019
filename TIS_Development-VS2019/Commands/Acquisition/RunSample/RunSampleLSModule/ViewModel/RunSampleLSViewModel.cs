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

    using CustomMessageBox;

    using GeometryUtilities;

    using MesoScan.Params;

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

        // wrapped RunSampleLS object
        private readonly RunSampleLS _RunSampleLS;

        private string _aFStatusMessage = string.Empty;
        private Visibility _aFStatusVisibility = Visibility.Collapsed;
        private Visibility _bleachOptionsVisibility;
        string _carrierType = string.Empty;
        private int _currentSubImageCol;
        private int _currentSubImageRow;
        private bool _displayAspectRatio = false;
        private ICommand _displayROIStatsOptionsCommand;
        private IEventAggregator _eventAggregator;
        private double _fieldSizeCalibration = 100.0;
        ScrollBarVisibility _ivScrollbarVisibility = ScrollBarVisibility.Hidden;
        private DateTime _lastUpdate = DateTime.Now;
        private int _lSMFieldSize = 0;
        private string _mCLS1Name;
        private string _mCLS2Name;
        private string _mCLS3Name;
        private string _mCLS4Name;
        bool _mROIShowOverlays = false;
        bool _mROISpatialDisplaybleEnable = true;
        private bool _newExperiment;
        private int _nPixelBitShiftValueUpdates = 0;
        Visibility _power0Visibility = Visibility.Collapsed;
        Visibility _power1Visibility = Visibility.Collapsed;
        Visibility _power2Visibility = Visibility.Collapsed;
        Visibility _power3Visibility = Visibility.Collapsed;
        Visibility _power4Visibility = Visibility.Collapsed;
        Visibility _power5Visibility = Visibility.Collapsed;
        private IRegionManager _regionManager;
        private double _reqDiskSize;
        private DispatcherTimer _roiDataUpdateTimer; // refresh bitmap
        private ROIStatsChartWin _roiStatsChart = null;
        private bool _roiStatsChartActive;
        private Thickness _roiToolbarMargin = new Thickness(0, 0, 0, 0);
        private ICommand _RunSampleLSStartCommand;
        private ICommand _RunSampleLSStopCommand;
        string _scanAreaWellLocation = string.Empty;
        private int _selectedScanArea = 0;
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
        private bool _zInvertLimits = false;
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

            _roiDataUpdateTimer = new DispatcherTimer(DispatcherPriority.Render);
            _roiDataUpdateTimer.Interval = new TimeSpan(0, 0, 0, 0, 30);
            _roiDataUpdateTimer.Tick += RoiDataUpdate;

            SubscribeToCommandEvent();

            _reqDiskSize = 0;

            OverlayManagerClass.Instance.InitOverlayManagerClass(this.SettingsImageWidth, this.SettingsImageHeight, this.PixelSizeUM, false);

            PowerControlNames = new ObservableCollection<StringPC>();

            for (int i = 0; i < MAX_POWER_CONTROLS; i++)
            {
                PowerControlNames.Add(new StringPC());
            }

            _newExperiment = false; //flag to know if a new experiment has occurred
            this._RunSampleLS.ROIStatsChanged += _RunSampleLS_ROIStatsChanged;

            MVMManager.CollectMVM();
            _roiStatsChartActive = false;
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

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

        public Visibility AdvancedImageControlPanelVisibility
        {
            get
            {
                //Should be updated when more controls get added to the expander. Visibility here is based on visibility of aspect ratio option
                return DisplayAspectRatioOptionVisibility;
            }
        }

        public string AFStatusMessage
        {
            get
            {
                return _aFStatusMessage;
            }
            set
            {
                _aFStatusMessage = value;
                OnPropertyChanged("AFStatusMessage");
            }
        }

        public Visibility AFStatusVisibility
        {
            get
            {
                return _aFStatusVisibility;
            }
            set
            {
                _aFStatusVisibility = value;
                OnPropertyChanged("AFStatusVisibility");
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
                OnPropertyChanged("IsStimulationIntervalVisible");
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

        public PixelSizeUM CamPixelSizeUM
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
                if (1 != value)
                {
                    RawDataCaptureStreamingValue = RawDataCapture;
                    RawDataCapture = 0;
                }
                else
                {
                    RawDataCapture = RawDataCaptureStreamingValue;
                }

                OnPropertyChanged("CaptureMode");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StimulusStreamVis");
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

        public bool DisplayAspectRatio
        {
            get => _displayAspectRatio;
            set
            {
                _displayAspectRatio = value;
                MVMManager.Instance["ImageViewCaptureVM", "DisplayPixelAspectRatio"] = value;
            }
        }

        public Visibility DisplayAspectRatioOptionVisibility
        {
            get
            {
                return PixelAspectRatioYScale > 1 ? Visibility.Visible : Visibility.Collapsed;
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
                    _roiDataUpdateTimer.Start();
                }
                else
                {
                    _roiDataUpdateTimer.Stop();
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

        public bool DontAskUniqueOutputPath
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
                        XmlManager.GetAttribute(node, appSettings, "generateOutputFolderWithoutAsking", ref dontAskXmlValue);
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
                        XmlManager.SetAttribute(node, appSettings, "generateOutputFolderWithoutAsking", valueString);
                        MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
                    }
                    else
                    {
                        XmlManager.CreateXmlNode(appSettings, "ExperimentSaving");
                        node = appSettings.SelectSingleNode("/ApplicationSettings/ExperimentSaving");
                        if (node != null)
                        {
                            XmlManager.SetAttribute(node, appSettings, "generateOutputFolderWithoutAsking", valueString);
                            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
                        }
                    }
                }
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

        public ObservableCollection<string> DynamicLabels
        {
            get
            {
                return _RunSampleLS.DynamicLabels;
            }
            set
            {
                _RunSampleLS.DynamicLabels = value;
                OnPropertyChanged("DynamicLabels");
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
                    FileName inputName = new FileName(value, false);

                    this._RunSampleLS.ExperimentName.NameWithoutNumber = inputName.NameWithoutNumber;

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
                    return this.CamPixelSizeUM.PixelHeightUM * (this.CamImageHeight);
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
                    return this.CamPixelSizeUM.PixelWidthUM * (this.CamImageWidth);
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

        public bool IsRemoteFocus
        {
            get
            {
                return _RunSampleLS.IsRemoteFocus;
            }
        }

        public bool IsStimulationIntervalVisible
        {
            get
            {
                return !(3 == BleachTrigger || 0 == CurrentBleachMode);
            }
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

        public ScrollBarVisibility IVScrollbarVisibility
        {
            get => _ivScrollbarVisibility;
            set
            {
                _ivScrollbarVisibility = value;
                OnPropertyChanged("IVScrollbarVisibility");
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

        public PixelSizeUM LSMUMPerPixel
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
                    return (LSMFieldSize * LSMFieldSizeCalibration) / (LSMPixelX * TurretMagnification * (double)Constants.UM_TO_MM);
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

        public ObservableCollection<ScanArea> mROIList
        {
            get => _RunSampleLS.mROIList;
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

                    if (_mROIShowOverlays)
                    {
                        mROISpatialDisplaybleEnableCapture = true;
                    }

                    MVMManager.Instance["ImageViewCaptureVM", "ROIToolVisible"] = _mROIShowOverlays ?
                        new bool[14] { false, false, false, false, false, false, false, false, false, false, false, false, false, false } :
                        new bool[14] { true, true, true, true, true, true, true, true, true, true, true, true, true, true };

                    if (_mROIShowOverlays)
                    {
                        OverlayManagerClass.Instance.PixelUnitSizeXY = new int[2] { mROIStripePixels, 1 };
                        OverlayManagerClass.Instance.ValidateROIs();

                        var ROIs = OverlayManagerClass.Instance.GetModeROIs(Mode.MICRO_SCANAREA);
                        if (ROIs.Count > SelectedmROIIndex && SelectedmROIIndex >= 0)
                        {
                            MVMManager.Instance["ImageViewCaptureVM", "mROIPriorityIndex"] = SelectedmROIIndex;
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
                    OverlayManagerClass.Instance.mROIsDisableMoveAndResize = true;
                    if (_mROIShowOverlays)
                    {
                        MVMManager.Instance["ImageViewCaptureVM", "mROIPriorityIndex"] = SelectedmROIIndex;
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

        public bool mROISpatialDisplaybleEnableCapture
        {
            get => _mROISpatialDisplaybleEnable;
            set
            {
                _mROISpatialDisplaybleEnable = value;
                if (!value)
                {
                    mROIShowOverlays = false;
                }
                OnPropertyChanged("mROISpatialDisplaybleEnableCapture");
                MVMManager.Instance["ImageViewCaptureVM", "mROISpatialDisplaybleEnable"] = value;
            }
        }

        public double mROIStripePhysicalFieldSizeUM
        {
            get
            {
                if (_RunSampleLS.IsmROICapture && _RunSampleLS.mROIList?.Count > 0 && _RunSampleLS.mROIList[0] != null)
                {
                    return Math.Round(_RunSampleLS.mROIList[0].PhysicalSizeXUM / _RunSampleLS.mROIList[0].Stripes, 2);
                }

                return 0;
            }
        }

        public int mROIStripePixels
        {
            get
            {
                if (_RunSampleLS.IsmROICapture && _RunSampleLS.mROIList?.Count > 0 && _RunSampleLS.mROIList[0] != null)
                {
                    return _RunSampleLS.mROIList[0].SizeXPixels / _RunSampleLS.mROIList[0].Stripes;
                }

                return 0;
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

        public double PixelAspectRatioYScale
        {
            get
            {
                return _RunSampleLS.PixelAspectRatioYScale;
            }
            set
            {
                RunSampleLS.PixelAspectRatioYScale = value;
                OnPropertyChanged("PixelAspectRatioYScale");
                OnPropertyChanged("DisplayAspectRatioOptionVisibility");
                OnPropertyChanged("AdvancedImageControlPanelVisibility");
            }
        }

        public PixelSizeUM PixelSizeUM
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

        public double PowerShiftUS
        {
            get
            {
                return this._RunSampleLS.PowerShiftUS;
            }
            set
            {
                this._RunSampleLS.PowerShiftUS = value;
                OnPropertyChanged("PowerShiftUS");
            }
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

        public int RawDataCaptureStreamingValue
        {
            get
            {
                return this._RunSampleLS.RawDataCaptureStreamingValue;
            }
            set
            {
                this._RunSampleLS.RawDataCaptureStreamingValue = value;
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

        public int RemoteFocusCaptureMode
        {
            get
            {
                return _RunSampleLS.RemoteFocusCaptureMode;
            }
            set
            {
                _RunSampleLS.RemoteFocusCaptureMode = value;
                OnPropertyChanged("RemoteFocusCaptureMode");
                OnPropertyChanged("ZNumSteps");
                OnPropertyChanged("DriveSpace");
            }
        }

        public bool RemoteFocusCustomChecked
        {
            get
            {
                return _RunSampleLS.RemoteFocusCustomChecked;
            }
            set
            {
                _RunSampleLS.RemoteFocusCustomChecked = value;
                OnPropertyChanged("RemoteFocusCustomChecked");
                OnPropertyChanged("ZNumSteps");
            }
        }

        public string RemoteFocusCustomOrder
        {
            get
            {
                return _RunSampleLS.RemoteFocusCustomOrder;
            }
            set
            {
                _RunSampleLS.RemoteFocusCustomOrder = value;
                OnPropertyChanged("RemoteFocusCustomOrder");
                OnPropertyChanged("ZNumSteps");
                OnPropertyChanged("DriveSpace");
            }
        }

        public ObservableCollection<double> RemoteFocusCustomSelectedPlanes
        {
            get
            {
                return _RunSampleLS.RemoteFocusCustomSelectedPlanes;
            }
            set
            {
                _RunSampleLS.RemoteFocusCustomSelectedPlanes = value;
            }
        }

        public int RemoteFocusNumberOfTicks
        {
            get
            {
                return _RunSampleLS.RemoteFocusNumberOfPlanes - 2;
            }
        }

        public double RemoteFocusStartPosition
        {
            get
            {
                return _RunSampleLS.RemoteFocusStartPosition;
            }
            set
            {
                if (value < _RunSampleLS.ZMin)
                {
                    _RunSampleLS.RemoteFocusStartPosition = _RunSampleLS.ZMin;
                }
                else if (value > _RunSampleLS.ZMax)
                {
                    _RunSampleLS.RemoteFocusStartPosition = _RunSampleLS.ZMax;
                }
                else
                {
                    _RunSampleLS.RemoteFocusStartPosition = value;
                }
                EnsureValidRemoteFocusScanStop();
                OnPropertyChanged("RemoteFocusStartPosition");
                OnPropertyChanged("ZNumSteps");
                OnPropertyChanged("DriveSpace");
            }
        }

        public int RemoteFocusStepSize
        {
            get
            {
                return _RunSampleLS.RemoteFocusStepSize;
            }
            set
            {
                _RunSampleLS.RemoteFocusStepSize = value;
                EnsureValidRemoteFocusScanStop();
                OnPropertyChanged("RemoteFocusStepSize");
                OnPropertyChanged("ZNumSteps");
                OnPropertyChanged("DriveSpace");
            }
        }

        public double RemoteFocusStopPosition
        {
            get
            {
                return _RunSampleLS.RemoteFocusStopPosition;
            }
            set
            {
                if (value < _RunSampleLS.ZMin)
                {
                    _RunSampleLS.RemoteFocusStopPosition = _RunSampleLS.ZMin;
                }
                else if (value > _RunSampleLS.ZMax)
                {
                    _RunSampleLS.RemoteFocusStopPosition = _RunSampleLS.ZMax;
                }
                else
                {
                    _RunSampleLS.RemoteFocusStopPosition = value;
                }
                EnsureValidRemoteFocusScanStop();
                OnPropertyChanged("RemoteFocusStopPosition");
                OnPropertyChanged("ZNumSteps");
                OnPropertyChanged("DriveSpace");
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
                        _roiStatsChart.Hide();
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
                    this._RunSampleLSStartCommand = new RelayCommand(() => Application.Current?.Dispatcher.BeginInvoke((Action)(() =>
                    {
                        RunSampleLSStart();
                    }), DispatcherPriority.Normal));

                return this._RunSampleLSStartCommand;
            }
        }

        public ICommand RunSampleLSStopCommand
        {
            get
            {
                

                if (this._RunSampleLSStopCommand == null)
                    this._RunSampleLSStopCommand = new RelayCommand(() => Application.Current?.Dispatcher.BeginInvoke((Action)(() =>
                    {
                        RunSampleLSStop();
                    }), DispatcherPriority.Normal));

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

                MVMManager.Instance["ImageViewCaptureVM", "mROIPriorityIndex"] = value - 1;

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
            get { return this._RunSampleLS.SLMBleachDelay[0]; }
            set
            {
                this._RunSampleLS.SLMBleachDelay[0] = (double)Decimal.Round((Decimal)value, 3);
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

        public bool SLMRandomEpochs
        {
            get
            {
                return this._RunSampleLS.SLMRandomEpochs;
            }
            set
            {
                this._RunSampleLS.SLMRandomEpochs = value;
                OnPropertyChanged("SLMRandomEpochs");
            }
        }

        public double SLMSequenceEpochDelay
        {
            get { return this._RunSampleLS.SLMBleachDelay[1]; }
            set
            {
                this._RunSampleLS.SLMBleachDelay[1] = (double)Decimal.Round((Decimal)value, 3);
                OnPropertyChanged("SLMSequenceEpochDelay");
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

                OnPropertyChanged("ZFastEnable");
                OnPropertyChanged("StreamFrames");
                OnPropertyChanged("StreamVolumes");
                OnPropertyChanged("DriveSpace");
                OnPropertyChanged("StreamFramesTime");
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

        public bool ZInvert
        {
            get
            {
                return _RunSampleLS.ZInvert;
            }
            set
            {
                _RunSampleLS.ZInvert = value;
                OnPropertyChanged("ZInvert");
                OnPropertyChanged("ZInvertLimits");
                OnPropertyChanged("DynamicLabels");
            }
        }

        public bool ZInvertLimits
        {
            get
            {
                if (_zInvertLimits && ZInvert)
                {
                    return false;
                }
                else
                {
                    return _zInvertLimits || ZInvert;
                }
            }
            set
            {
                _zInvertLimits = value;
            }
        }

        public double ZMax
        {
            get => _RunSampleLS.ZMax;
        }

        public double ZMin
        {
            get => _RunSampleLS.ZMin;
        }

        public int ZNumSteps
        {
            get
            {
                return this._RunSampleLS.ZNumSteps;
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
                if (IsRemoteFocus && _RunSampleLS.DynamicLabels.Count > 0 && _RunSampleLS.ZPosition >= ZMin && _RunSampleLS.ZPosition <= ZMax)
                {
                    double pos;
                    Double.TryParse(_RunSampleLS.DynamicLabels[(int)_RunSampleLS.ZPosition - 1], NumberStyles.Any, CultureInfo.InvariantCulture, out pos);
                    return pos;
                }
                else
                {
                    Decimal dec;

                    dec = new Decimal(_RunSampleLS.ZPosition);

                    dec = Decimal.Round(dec, 4);

                    return Convert.ToDouble(dec);
                }
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
                double min = IsRemoteFocus ? _RunSampleLS.ZMin / (double)Constants.UM_TO_MM : _RunSampleLS.ZMin;
                double max = IsRemoteFocus ? _RunSampleLS.ZMax / (double)Constants.UM_TO_MM : _RunSampleLS.ZMax;
                if (value <= min)
                {
                    _RunSampleLS.ZStartPosition = min;
                }
                else if (value >= max)
                {
                    _RunSampleLS.ZStartPosition = max;
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

        public string ZStartPositionLabel
        {
            get
            {
                return IsRemoteFocus ? "Start Plane" : "Start Position";
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
                double rounded = IsRemoteFocus ? Math.Round(value) : Math.Round(value, 1);
                if (rounded <= .1)
                {
                    rounded = IsRemoteFocus ? 1 : .1;
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
                double min = IsRemoteFocus ? _RunSampleLS.ZMin / (double)Constants.UM_TO_MM : _RunSampleLS.ZMin;
                double max = IsRemoteFocus ? _RunSampleLS.ZMax / (double)Constants.UM_TO_MM : _RunSampleLS.ZMax;
                if (value <= min)
                {
                    _RunSampleLS.ZStopPosition = min;
                }
                else if (value >= max)
                {
                    _RunSampleLS.ZStopPosition = max;
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

        public string ZStopPositionLabel
        {
            get
            {
                return IsRemoteFocus ? "Stop Plane" : "Stop Position";
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

        public void EnableHandlers()
        {
            this._RunSampleLS.Update += new Action<string>(RunSampleLS_Update);
            this._RunSampleLS.UpdateBitmapTimer += new Action<bool>(_RunSampleLS_UpdateBitmapTimer);
            this._RunSampleLS.UpdateImageName += new Action<string>(RunSampleLS_UpdateImageName);
            this._RunSampleLS.UpdateStart += new Action<string>(RunSampleLS_UpdateStart);
            this._RunSampleLS.UpdateStartSubWell += new Action<string>(RunSampleLS_UpdateStartSubWell);
            this._RunSampleLS.UpdateEndSubWell += new Action<string>(RunSampleLS_UpdateEndSubWell);
            this._RunSampleLS.UpdateButton += new Action<bool>(RunSampleLS_UpdateButton);
            this._RunSampleLS.UpdatePanels += new Action<bool>(RunSampleLS_UpdatePanels);
            this._RunSampleLS.UpdateMenuBarButton += new Action<bool>(RunSampleLS_UpdateMenuBarButton);
            this._RunSampleLS.UpdateScript += new Action<string>(RunSampleLS_UpdateScript);
            this._RunSampleLS.ExperimentStarted += new Action(RunSampleLS_ExperimentStarted);
            this._RunSampleLS.CaptureComplete += new Action(RunSampleLS_CaptureComplete);
            this._RunSampleLS.UpdateSequenceStepDisplaySettings += new Action(_RunSampleLS_UpdateSequenceStepDisplaySettings);
            OverlayManagerClass.Instance.mROISelectedEvent += Instance_OverlaymROISelectedEvent;
            OverlayManagerClass.Instance.mROIsDisableMoveAndResize = true;
            this._RunSampleLS.EnableHandlers();
        }

        public bool GetLSMFieldSizeCalibration(ref double calibration)
        {
            return this._RunSampleLS.LIGetFieldSizeCalibration(ref calibration);
        }

        public bool IsCamera()
        {
            return this._RunSampleLS.ActiveCameraType == 0; // 0 for CCD
        }

        public void LoadMROISettings(XmlDocument expDoc)
        {
            _RunSampleLS.LoadmROISettings(expDoc);
            OnPropertyChanged("SelectedScanArea");
            OnPropertyChanged("SelectedmROIIndex");
            OnPropertyChanged("mROIList");
            OnPropertyChanged("mROIShowOverlays");
            OnPropertyChanged("mROIStripePixels");
            OnPropertyChanged("mROIStripePhysicalFieldSizeUM");
        }

        public void LoadOverlayManagerSettings()
        {
            bool isReticleChecked = false;
            bool isScaleButtonChecked = false;

            OverlayManagerClass.Instance.InitSelectROI();
            OverlayManagerClass.Instance.PersistLoadROIs(ref isReticleChecked, ref isScaleButtonChecked);
            MVMManager.Instance["ImageViewCaptureVM", "IsReticleChecked"] = isReticleChecked;
            MVMManager.Instance["ImageViewCaptureVM", "IsScaleButtonChecked"] = isScaleButtonChecked;
        }

        public void LoadRemoteFocusPositionValues()
        {
            //Load Dynamic Labels for Remote Focus
            if (IsRemoteFocus)
            {
                string posFile = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + @"\RemoteFocusPositionValues.xml";

                if (File.Exists(posFile))
                {
                    ObservableCollection<string> dynamicLabels = new ObservableCollection<string>();
                    ObservableCollection<string> originalLabels = new ObservableCollection<string>();
                    double remoteFocusCalibrationMagnification = 1;
                    try
                    {
                        XmlDocument remoteFocusPositionValues = new XmlDocument();
                        remoteFocusPositionValues.Load(posFile);
                        XmlNodeList ndList = remoteFocusPositionValues.SelectNodes("/RemoteFocusPositionSettings/MeasurementObjective");
                        string mg = ndList[0].Attributes["magnification"].Value;
                        double tmp = 0;
                        if (Double.TryParse(mg, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                        {
                            remoteFocusCalibrationMagnification = tmp;
                        }

                        ndList = remoteFocusPositionValues.SelectNodes("/RemoteFocusPositionSettings/MeasuredPlaneDistance");
                        if (ndList.Count > 0)
                        {
                            foreach (XmlAttribute attr in ndList[0].Attributes)
                            {
                                dynamicLabels.Add(attr.Value);
                                originalLabels.Add(attr.Value);
                            }
                        }
                    }
                    catch (System.IO.IOException e)
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " error loading RemoteFocusPositionValues.xml. Exception: " + e.Message);
                    }

                    if (dynamicLabels.Count != _RunSampleLS.RemoteFocusNumberOfPlanes)
                    {
                        MessageBox.Show("Number of planes don't match the number of measured distance inputs in RemoteFocusPositionValues.xml.");
                    }

                    // From Joe Ma. The axial (z) amplification goes by the square of the objective power. Formula is (16X mag / objective mag)^2
                    double mag = (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)1.0];
                    double ratio = Math.Pow(remoteFocusCalibrationMagnification / mag, 2);
                    for (int k = 0; k < dynamicLabels.Count; k++)
                    {
                        double val = 0;
                        if (double.TryParse(originalLabels[k], out val))
                        {
                            int index = ZInvertLimits ? dynamicLabels.Count - 1 - k : k; //If Z limits are inverted, flip the order of the plane values
                            val *= ratio;
                            dynamicLabels[index] = Math.Round(val, 1).ToString();
                        }
                    }

                    DynamicLabels = dynamicLabels;
                }
            }
        }

        public void LoadXMLSettings()
        {
            //here to conform to the ThorSharedTypes.IMVM interface
            ((IMVM)MVMManager.Instance["RemoteIPCControlViewModelBase", this]).OnPropertyChange("RemoteConnection");
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
            this._RunSampleLS.UpdateImageName -= new Action<string>(RunSampleLS_UpdateImageName);
            this._RunSampleLS.UpdateStart -= new Action<string>(RunSampleLS_UpdateStart);
            this._RunSampleLS.UpdateStartSubWell -= new Action<string>(RunSampleLS_UpdateStartSubWell);
            this._RunSampleLS.UpdateEndSubWell -= new Action<string>(RunSampleLS_UpdateEndSubWell);
            this._RunSampleLS.UpdateButton -= new Action<bool>(RunSampleLS_UpdateButton);
            this._RunSampleLS.UpdatePanels -= new Action<bool>(RunSampleLS_UpdatePanels);
            this._RunSampleLS.UpdateMenuBarButton -= new Action<bool>(RunSampleLS_UpdateMenuBarButton);
            this._RunSampleLS.UpdateScript -= new Action<string>(RunSampleLS_UpdateScript);
            this._RunSampleLS.ExperimentStarted -= new Action(RunSampleLS_ExperimentStarted);
            this._RunSampleLS.CaptureComplete -= new Action(RunSampleLS_CaptureComplete);
            this._RunSampleLS.UpdateSequenceStepDisplaySettings -= new Action(_RunSampleLS_UpdateSequenceStepDisplaySettings);

            OverlayManagerClass.Instance.mROISelectedEvent -= Instance_OverlaymROISelectedEvent;
            //calling the release handlers present in the model
            _RunSampleLS.ReleaseHandlers();

            //close any floating panels or windows
            if (null != _roiStatsChart)
            {
                _roiStatsChart.Close();
            }
            OverlayManagerClass.Instance.mROIsDisableMoveAndResize = false;

            mROIShowOverlays = false;
        }

        public void ResetImage()
        {
            //TODO:IV
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

        public void UpdateNumberOfPlanes()
        {
            OnPropertyChanged("ZNumSteps");
            OnPropertyChanged("DriveSpace");
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

        private bool ConfirmDirectoryIsReadOnly()
        {
            DirectoryInfo di = new DirectoryInfo(OutputPath);
            if ((di.Attributes & FileAttributes.ReadOnly) == FileAttributes.ReadOnly)
            {
                MessageBox.Show("Invalid OutputPath: The output path is read only.");
            }
            else
            {
                return true;
            }
            return false;
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
        private void EnsureValidRemoteFocusScanStop()
        {
            if (RemoteFocusStopPosition != RemoteFocusStartPosition)
            {
                double rem = Math.Round(Math.Abs(RemoteFocusStopPosition - RemoteFocusStartPosition) % RemoteFocusStepSize);
                if (0 != rem)
                {
                    int zDirection = 0;
                    if (RemoteFocusStopPosition > RemoteFocusStartPosition)
                    {
                        zDirection = 1;
                    }
                    else
                    {
                        zDirection = -1;
                    }

                    double newZScanStop = Math.Round(RemoteFocusStopPosition + zDirection * (RemoteFocusStepSize - rem));
                    if (newZScanStop < _RunSampleLS.ZMin)
                    {
                        _RunSampleLS.RemoteFocusStopPosition = _RunSampleLS.ZMin;
                    }
                    else if (newZScanStop > _RunSampleLS.ZMax)
                    {
                        _RunSampleLS.RemoteFocusStopPosition = _RunSampleLS.ZMax;
                    }
                    else
                    {
                        _RunSampleLS.RemoteFocusStopPosition = newZScanStop;
                    }

                    OnPropertyChanged("RemoteFocusStopPosition");

                    if (newZScanStop != RemoteFocusStopPosition)
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

        //Check and fit to the size of the ZStep
        private void EnsureValidZScanStop()
        {
            if (ZStopPosition != ZStartPosition)
            {
                double stepFactor = (double)Constants.UM_TO_MM; //IsRemoteFocus ? 1 : (double)Constants.UM_TO_MM;
                double rem = Math.Round(Math.Abs(ZStopPosition - ZStartPosition) % (ZStepSize / stepFactor), 4);
                double min = IsRemoteFocus ? _RunSampleLS.ZMin / (double)Constants.UM_TO_MM : _RunSampleLS.ZMin;
                double max = IsRemoteFocus ? _RunSampleLS.ZMax / (double)Constants.UM_TO_MM : _RunSampleLS.ZMax;
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
                    double newZScanStop = Math.Round(ZStopPosition + zDirection * (ZStepSize / stepFactor - Math.Abs(ZStopPosition - ZStartPosition) % (ZStepSize / stepFactor)), 4);
                    if (newZScanStop <= min)
                    {
                        _RunSampleLS.ZStopPosition = min;
                    }
                    else if (newZScanStop >= max)
                    {
                        _RunSampleLS.ZStopPosition = max;
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

        private void Instance_OverlaymROISelectedEvent(int obj)
        {
            if (_mROIShowOverlays)
            {
                _selectedScanArea = obj;
                OnPropertyChanged("SelectedScanArea");
                OnPropertyChanged("SelectedmROIIndex");

                MVMManager.Instance["ImageViewCaptureVM", "mROIPriorityIndex"] = obj - 1;
            }
        }

        private bool IsOutputPathValid()
        {
            bool ret = false;
            if (Directory.Exists(OutputPath))
            {
                ret = ConfirmDirectoryIsReadOnly();
            }
            else
            {
                //Confirm that user wants to create a new folder in the output path
                if (!DontAskUniqueOutputPath)
                {
                    CustomMessageBox messageBox = new CustomMessageBox("The output path doesn't exist. Would you like to create a folder with the \npath: " + OutputPath + "?", "Output Folder Doesn't Exist", "Don't ask again", "Yes", "No");
                    if (!messageBox.ShowDialog().GetValueOrDefault(false))
                    {
                        return ret;
                    }
                    else
                    {
                        DontAskUniqueOutputPath = messageBox.CheckBoxChecked;
                    }
                }
                try
                {
                    Directory.CreateDirectory(OutputPath);
                    ret = ConfirmDirectoryIsReadOnly();
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Could not create a new folder with the path: " + OutputPath + "\nException thrown: " + ex.Message, "Error creating Output Path folder");
                    return ret;
                }
            }

            //validate experiment name without tailing space
            char[] charsToTrim = { ' ', '\t' };

            this._RunSampleLS.ExperimentName.NameWithoutNumber = this._RunSampleLS.ExperimentName.NameWithoutNumber.Trim(charsToTrim);

            //Remove "." from the last character because file system doesn't allow for names ending in "." similar to the trim, file system doesn't allow whitespace at beginng and end

            //Always append shouldn't need this
            if (this._RunSampleLS.ExperimentName.NameWithoutNumber.EndsWith(".") && 1 < this._RunSampleLS.ExperimentName.NameWithoutNumber.Length)
            {
                // Remove the last character
                this._RunSampleLS.ExperimentName.NameWithoutNumber = this._RunSampleLS.ExperimentName.NameWithoutNumber.Remove(this._RunSampleLS.ExperimentName.NameWithoutNumber.Length - 1);
            }

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
                                    _RunSampleLS.BleachWParams = WaveformBuilder.LoadBleachWaveParams(bROISource, roiCapsule, this.PixelSizeUM.PixelWidthUM, this.PixelSizeUM.PixelWidthUM / bleachLSMUMPerPixel, this.BinX, this.BinY);
                                }
                                else
                                {
                                    _RunSampleLS.BleachWParams = WaveformBuilder.LoadBleachWaveParams(bROISource, roiCapsule, this.PixelSizeUM.PixelWidthUM, 1.0, this.BinX, this.BinY);
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
        private void RoiDataUpdate(object sender, EventArgs e)
        {
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

            OverlayManagerClass.Instance.UpdateParams(_RunSampleLS.SettingsImageWidth, _RunSampleLS.SettingsImageHeight, _RunSampleLS.PixelSizeUM);
            //[Note] Active.xml is copied to experiment after _RunSampleLS.Start
            if (!_RunSampleLS.Start())
            {
                RunSampleLS_UpdateScript("Error");
            }

            MVMManager.Instance["ImageViewCaptureVM", "ResetBitmap"] = true;

            //TODO:IV _nPixelBitShiftValueUpdates?
            //allow the shiftvalue to be updated after start is called by resetting the counter
            _nPixelBitShiftValueUpdates = 0;

            OnPropertyChanged("SelectedScanArea");
            OnPropertyChanged("SelectedmROIIndex");
            OnPropertyChanged("mROIList");
            OnPropertyChanged("mROIShowOverlays");
            OnPropertyChanged("mROIStripePixels");
            OnPropertyChanged("mROIStripePhysicalFieldSizeUM");
        }

        private void RunSampleLSStop()
        {
            this._RunSampleLS.Stop();
            _roiDataUpdateTimer.Stop();
            Application.Current.Dispatcher.Invoke(() =>
            {
                OverlayManagerClass.Instance.SaveROIs(_RunSampleLS.ExperimentFolderPath + "ROIs.xaml");
                OverlayManagerClass.Instance.SaveMaskToPath(_RunSampleLS.ExperimentFolderPath + "ROIMask.raw");
            });
        }

        void RunSampleLS_CaptureComplete()
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                OverlayManagerClass.Instance.SaveROIs(_RunSampleLS.ExperimentFolderPath + "ROIs.xaml");
                OverlayManagerClass.Instance.SaveMaskToPath(_RunSampleLS.ExperimentFolderPath + "ROIMask.raw");
            });
        }

        void RunSampleLS_ExperimentStarted()
        {
            _newExperiment = true;
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

        void _RunSampleLS_UpdateBitmapTimer(bool timer)
        {
            if ((timer) && (this.ImageUpdaterVisibility == Visibility.Visible))
            {
                _roiDataUpdateTimer.Start();
            }
            else
            {
                _roiDataUpdateTimer.Stop();
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

        ////notify listeners (Ex. histogram) that the image has changed
        //public event Action<bool> ImageDataChanged;

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