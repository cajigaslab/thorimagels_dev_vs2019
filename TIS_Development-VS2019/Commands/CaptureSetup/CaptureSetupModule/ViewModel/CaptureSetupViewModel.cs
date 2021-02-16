namespace CaptureSetupDll.ViewModel
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
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    using CaptureSetupDll.Model;

    using GongSolutions.Wpf.DragDrop;

    using LineProfileWindow;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using MultiROIStats;

    using OverlayManager;

    using ROIStatsChart.ViewModel;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    using DragDrop = GongSolutions.Wpf.DragDrop.DragDrop;

    /// <summary>
    /// ViewModel class for the CaptureSetup model object
    /// </summary>
    public partial class CaptureSetupViewModel : ViewModelBase, IDropTarget, ThorSharedTypes.IMVM
    {
        #region Fields

        public static Canvas OverlayCanvas;
        public static Canvas PresentationCanvas;

        private readonly CaptureSetup _captureSetup;

        private ICommand _autoContourDisplayCommand;
        private AutoContourWin _autoContourWin = null;
        private bool _autoCoutourWinInit;
        private bool _bleachBorderEnabled;
        private BackgroundWorker _bleachWorker;
        private BackgroundWorker _bw;
        private BackgroundWorker _bwHardware = null;
        private Double _captureSetupControlPanelWidth = 0;
        private ICommand _clearAllObjectsCommand;
        private Visibility _collapsedPockels0Visibility = Visibility.Collapsed;
        private Visibility _collapsedPockels1Visibility = Visibility.Collapsed;
        private Visibility _collapsedPockels2Visibility = Visibility.Collapsed;
        private Visibility _collapsedPockels3Visibility = Visibility.Collapsed;
        private ObservableCollection<StringPC> _collapsedPowerControlName;
        private Visibility _collapsedPowerReg2Visibility = Visibility.Collapsed;
        private Visibility _collapsedPowerRegVisibility = Visibility.Collapsed;
        private Visibility _collapsedShutter2Visibility = Visibility.Collapsed;
        private IUnityContainer _container;
        private DispatcherTimer _deviceReadTimer;
        private ICommand _displayROIStatsOptionsCommand;
        private bool _enableDeviceQuery = true;
        private IEventAggregator _eventAggregator;
        private XmlDocument _hardwareDoc;
        private bool _ignoreLineProfileGeneration = false;
        private XmlDocument _imageProcessDoc;
        private Visibility _imagerViewVis = Visibility.Visible;
        private bool _isProgressWindowOff = true;
        private double _ivHeight;
        private double _iVScrollBarHeight;
        private LineProfile _lineProfile = null;
        private bool _lineProfileActive;
        private MultiROIStatsUC _multiROIStats = null;
        private string _OutputExperiment;
        private double _panelsScale = 1;
        DispatcherTimer _pmtSafetyTimer;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private string _quickTemplateExperimentPath;
        private IRegionManager _regionManager;
        private bool _restartBWHardware = false;
        private ROIStatsChartWin _roiStatsChart = null;
        private bool _roiStatsChartActive;
        private bool _roiStatsTableActive;
        private Thickness _roiToolbarMargin = new Thickness(0, 0, 0, 0);
        private RelayCommand _saveStatChartCommand;
        private bool _showingPMTSafetyMessage;
        private BackgroundWorker _slmBleachWorker;
        private SpinnerProgress.SpinnerProgressControl _spinner;
        private Window _spinnerWindow = null;
        private bool _statChartFrz;

        // 0: 2D viewer
        // 1: 3D viewer
        private int _viewType;
        private bool _wrapPanelEnabled = true;
        private Double _wrapPanelWidth = 408;
        private bool _xyCtrlVisible;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the CaptureSetupViewModel class
        /// </summary>
        public CaptureSetupViewModel(IEventAggregator eventAggregator, IRegionManager regionManager, IUnityContainer container, CaptureSetup captureSetup)
        {
            this.UnloadWholeStats = false;
            this.UnloadWholeLineProfile = false;
            this.UnloadWholeROIStatsChart = false;
            this._eventAggregator = eventAggregator;
            this._regionManager = regionManager;
            this._container = container;

            if (captureSetup != null)
            {
                this._captureSetup = captureSetup;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " CaptureSetup is null. Creating a new CaptureSetup object.");
                captureSetup = new CaptureSetup();

                if (captureSetup == null)
                {
                    ResourceManager rm = new ResourceManager("CaptureSetupDataModule.Properties.Resources", Assembly.GetExecutingAssembly());
                    ThorLog.Instance.TraceEvent(TraceEventType.Critical, 1, this.GetType().Name + " " + rm.GetString("CreateCaptureSetupModelFailed"));
                    throw new NullReferenceException("captureSetup");
                }

                this._captureSetup = captureSetup;
            }

            this._captureSetup.ROIStatsChanged += _captureSetup_ROIStatsChanged;
            this._captureSetup.LineProfileChanged += _captureSetup_LineProfileChanged;
            this._captureSetup.CaptureSetupViewModel = this;
            _isLive = false;

            _autoCoutourWinInit = false;

            //_SpotScanIsEnabled = false;
            _bROIByteArray = null;

            _OutputExperiment = "Untitled001";

            const int LUT_SIZE = 256;

            CaptureSetup.ChannelLuts = new Color[CaptureSetup.MAX_CHANNELS][];

            ChannelName = new ObservableCollection<StringPC>();

            for (int i = 0; i < CaptureSetup.MAX_CHANNELS; i++)
            {
                CaptureSetup.ChannelLuts[i] = new Color[LUT_SIZE];
                ChannelName.Add(new StringPC());
            }

            for (int i = 0; i < CaptureSetup.MAX_CHANNELS; i++)
            {
                _currentChannelsLutFiles.Add(string.Empty);
            }

            CaptureSetupViewModel vm = this;

            this._captureSetup.UpdateMenuBarButton += new Action<bool>(LiveImage_UpdateMenuBarButton);

            var startLiveImageEvent = _eventAggregator.GetEvent<StartLiveImageEvent>();
            startLiveImageEvent.Subscribe(LiveCapture);

            this._captureSetup.UpdateImage += new Action<bool>(LiveImage_Update);

            //create a background worker that will update at 30fps to udpate the bitmap image
            _bw = new BackgroundWorker();
            _bw.WorkerReportsProgress = true;
            _bw.WorkerSupportsCancellation = true;

            //bleach-slm:
            _bleachWorker = new BackgroundWorker();
            _bleachWorker.WorkerSupportsCancellation = true;
            _slmBleachWorker = new BackgroundWorker();
            _slmBleachWorker.WorkerSupportsCancellation = true;
            _slmCalibWaveParam = new GeometryUtilities.BleachWaveParams();

            _showingPMTSafetyMessage = false;

            _deviceReadTimer = new DispatcherTimer();
            _deviceReadTimer.Interval = new TimeSpan(0, 0, 0, 0, 100);

            _pmtSafetyTimer = new DispatcherTimer();
            _pmtSafetyTimer.Interval = TimeSpan.FromMilliseconds(10000);
            EnableDeviceReading = true;

            this._captureSetup.PropertyChanged += new PropertyChangedEventHandler(LiveImage_PropertyChanged);

            OverlayManagerClass.Instance.InitOverlayManagerClass(512, 512, 1.0, true);

            _roiStatsChartActive = false;
            _roiStatsTableActive = false;
            _lineProfileActive = false;

            if (Application.Current != null)
            {
                Application.Current.Exit += Current_Exit;
            }

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
            _statChartFrz = false;

            OverlayManagerClass.Instance.BinX = 1;
            OverlayManagerClass.Instance.BinY = 1;

            //TODO: the collect MVM line should probably be higher level, before capture setup loads.
            //reload mvm manager settings files
            MVMManager.CollectMVM();
            MVMManager.Instance.LoadSettings();

            MVMManager.Instance["QuickTemplatesControlViewModel", "QtConstructor"] = true;
            //Pass Capture Setup's event aggregator to RemoteIPC MVM so it can communicate with IPCModules
            MVMManager.Instance["RemoteIPCControlViewModel", "EventAggregator"] = _eventAggregator;
        }

        #endregion Constructors

        #region Enumerations

        public enum ViewerType
        {
            Viewer2D = 0,
            Viewer3D
        }

        #endregion Enumerations

        #region Delegates

        private delegate void ExitFrameHandler(DispatcherFrame frame);

        #endregion Delegates

        #region Events

        public event Action ActiveSettingsReplaced;

        public event Action DrawLineForLineScanEvent;

        #endregion Events

        #region Properties

        public XmlDocument ApplicationDoc
        {
            get
            {
                return this._captureSetup.ApplicationDoc;
            }
            set
            {
                this._captureSetup.ApplicationDoc = value;
            }
        }

        public ICommand AutoContourDisplayCommand
        {
            get
            {
                if (this._autoContourDisplayCommand == null)
                    this._autoContourDisplayCommand = new RelayCommand(() => AutoContourDisplay());

                return this._autoContourDisplayCommand;
            }
        }

        public bool AutoCoutourWinInit
        {
            get
            {
                return _autoCoutourWinInit;
            }
            set
            {
                _autoCoutourWinInit = value;
            }
        }

        public int AutoROIDisplayChannel
        {
            get
            {
                return this._captureSetup.AutoROIDisplayChannel;
            }
            set
            {
                this._captureSetup.AutoROIDisplayChannel = value;
                if (value == 0)
                {
                    PresentationCanvas.Visibility = Visibility.Collapsed;
                }
            }
        }

        public bool BleachBorderEnabled
        {
            get
            {
                return this._bleachBorderEnabled;
            }
            set
            {
                this._bleachBorderEnabled = value;
            }
        }

        public bool BWHardware
        {
            get { return (_bwHardware.IsBusy); }
            set
            {
                if (value)
                {
                    if (false == _bwHardware.IsBusy)
                    {
                        _bwHardware.RunWorkerAsync();
                    }
                    else if (true == _bwHardware.CancellationPending)
                    {
                        _restartBWHardware = true;
                    }
                }
                else
                {
                    _bwHardware.CancelAsync();
                }
            }
        }

        public bool CameraLedAvailable
        {
            get
            {
                return (bool)MVMManager.Instance["CameraControlViewModel", "CamLedAvailable", (bool)false];
            }
        }

        public int CameraLedEnable
        {
            get
            {
                return (int)MVMManager.Instance["CameraControlViewModel", "CamLedEnable", (object)0];
            }
            set
            {
                MVMManager.Instance["CameraControlViewModel", "CamLedEnable"] = value;
                OnPropertyChanged("CameraLedEnable");
            }
        }

        public Double CaptureSetupControlPanelWidth
        {
            get
            {
                return _captureSetupControlPanelWidth;
            }
            set
            {
                _captureSetupControlPanelWidth = value;
                OnPropertyChanged("CaptureSetupControlPanelWidth");
            }
        }

        public ICommand ClearAllObjectsCommand
        {
            get
            {
                if (this._clearAllObjectsCommand == null)
                    this._clearAllObjectsCommand = new RelayCommand(() => OverlayManagerClass.Instance.ClearAllObjects(ref OverlayCanvas));

                return this._clearAllObjectsCommand;
            }
        }

        public string ClearDirectory
        {
            set
            {
                if (System.IO.Directory.Exists(value))
                {
                    if (File.Exists(value + "/jpeg/DeepZoomView.xap"))
                    {
                        File.Delete(value + "/jpeg/DeepZoomView.xap");
                    }
                    if (File.Exists(value + "/jpeg/DeepZoomViewTestPage.html"))
                    {
                        File.Delete(value + "/jpeg/DeepZoomViewTestPage.html");
                    }
                    DeleteDirectory(value);

                    Directory.CreateDirectory(value);
                    Directory.CreateDirectory(value + "/jpeg");
                }
                else
                {
                    Directory.CreateDirectory(value);
                    Directory.CreateDirectory(value + "/jpeg");
                }
            }
        }

        public string CollapsedCbLaser1Content
        {
            get
            {
                return (string)MVMManager.Instance["LaserControlViewModel", "CbLaser1Content", (object)string.Empty];
            }
        }

        public string CollapsedCbLaser2Content
        {
            get
            {
                return (string)MVMManager.Instance["LaserControlViewModel", "CbLaser2Content", (object)string.Empty];
            }
        }

        public string CollapsedCbLaser3Content
        {
            get
            {
                return (string)MVMManager.Instance["LaserControlViewModel", "CbLaser3Content", (object)string.Empty];
            }
        }

        public string CollapsedCbLaser4Content
        {
            get
            {
                return (string)MVMManager.Instance["LaserControlViewModel", "CbLaser4Content", (object)string.Empty];
            }
        }

        public double CollapsedConverRatio
        {
            get
            {
                return (double)MVMManager.Instance["XYTileControlViewModel", "ConvertRatio", (object)0.0];
            }
        }

        public string CollapsedLabeLUnit
        {
            get
            {
                return (string)MVMManager.Instance["XYTileControlViewModel", "LabelUnit", (object)string.Empty];
            }
        }

        public Visibility CollapsedLaser1Enable
        {
            get
            {
                return (1 == (int)MVMManager.Instance["LaserControlViewModel", "Laser1Enable", (object)0]) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public int CollapsedLaser1Position
        {
            get
            {
                return (int)MVMManager.Instance["MultiphotonControlViewModel", "Laser1Position", (object)0];
            }
        }

        public double CollapsedLaser1Power
        {
            get
            {
                return (double)MVMManager.Instance["LaserControlViewModel", "Laser1Power", (object)0.0];
            }
        }

        public Visibility CollapsedLaser2Enable
        {
            get
            {
                return (1 == (int)MVMManager.Instance["LaserControlViewModel", "Laser2Enable", (object)0]) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public double CollapsedLaser2Power
        {
            get
            {
                return (double)MVMManager.Instance["LaserControlViewModel", "Laser2Power", (object)0.0];
            }
        }

        public Visibility CollapsedLaser3Enable
        {
            get
            {
                return (1 == (int)MVMManager.Instance["LaserControlViewModel", "Laser3Enable", (object)0]) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public double CollapsedLaser3Power
        {
            get
            {
                return (double)MVMManager.Instance["LaserControlViewModel", "Laser3Power", (object)0.0];
            }
        }

        public Visibility CollapsedLaser4Enable
        {
            get
            {
                return (1 == (int)MVMManager.Instance["LaserControlViewModel", "Laser4Enable", (object)0]) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public double CollapsedLaser4Power
        {
            get
            {
                return (double)MVMManager.Instance["LaserControlViewModel", "Laser4Power", (object)0.0];
            }
        }

        public int CollapsedLaserShutter2Pos
        {
            get
            {
                return (int)MVMManager.Instance["MultiphotonControlViewModel", "LaserShutter2Position", (object)0];
            }
        }

        public int CollapsedLaserShutterPos
        {
            get
            {
                return (int)MVMManager.Instance["MultiphotonControlViewModel", "LaserShutterPosition", (object)0];
            }
        }

        public string CollapsedLSMPixelDensity
        {
            get
            {
                string pixelX = ((int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)0]).ToString();
                string pixelY = ((int)MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)0]).ToString();
                return pixelX + " x " + pixelY;
            }
        }

        public string CollapsedLSMZoom
        {
            get
            {
                return (string)MVMManager.Instance["AreaControlViewModel", "LSMZoom", (object)string.Empty];
            }
        }

        public double CollapsedPinholeSize
        {
            get
            {
                return (double)MVMManager.Instance["PinholeControlViewModel", "PinholeSize", (double)1.0];
            }
        }

        public Visibility CollapsedPockels0Visibility
        {
            get
            {
                return _collapsedPockels0Visibility;
            }
            set
            {
                _collapsedPockels0Visibility = value;
                OnPropertyChanged("CollapsedPockels0Visibility");
            }
        }

        public Visibility CollapsedPockels1Visibility
        {
            get
            {
                return _collapsedPockels1Visibility;
            }
            set
            {
                _collapsedPockels1Visibility = value;
                OnPropertyChanged("CollapsedPockels1Visibility");
            }
        }

        public Visibility CollapsedPockels2Visibility
        {
            get
            {
                return _collapsedPockels2Visibility;
            }
            set
            {
                _collapsedPockels2Visibility = value;
                OnPropertyChanged("CollapsedPockels2Visibility");
            }
        }

        public Visibility CollapsedPockels3Visibility
        {
            get
            {
                return _collapsedPockels3Visibility;
            }
            set
            {
                _collapsedPockels3Visibility = value;
                OnPropertyChanged("CollapsedPockels3Visibility");
            }
        }

        public double CollapsedPower0
        {
            get
            {
                return (double)MVMManager.Instance["PowerControlViewModel", "Power0", (object)0.0];
            }
        }

        public double CollapsedPower1
        {
            get
            {
                return (double)MVMManager.Instance["PowerControlViewModel", "Power1", (object)0.0];
            }
        }

        public double CollapsedPower2
        {
            get
            {
                return (double)MVMManager.Instance["PowerControlViewModel", "Power2", (object)0.0];
            }
        }

        public double CollapsedPower3
        {
            get
            {
                return (double)MVMManager.Instance["PowerControlViewModel", "Power3", (object)0.0];
            }
        }

        public ObservableCollection<StringPC> CollapsedPowerControlName
        {
            get
            {
                return _collapsedPowerControlName;
            }
            set
            {
                _collapsedPowerControlName = value;
                OnPropertyChanged("CollapsedPowerControlName");
            }
        }

        public double CollapsedPowerReg
        {
            get
            {
                return (double)MVMManager.Instance["PowerControlViewModel", "PowerReg", (object)0.0];
            }
        }

        public double CollapsedPowerReg2
        {
            get
            {
                return (double)MVMManager.Instance["PowerControlViewModel", "PowerReg2", (object)0.0];
            }
        }

        public Visibility CollapsedPowerReg2Visibility
        {
            get
            {
                return _collapsedPowerReg2Visibility;
            }
            set
            {
                _collapsedPowerReg2Visibility = value;
                OnPropertyChanged("CollapsedPowerReg2Visibility");
            }
        }

        public Visibility CollapsedPowerRegVisibility
        {
            get
            {
                return _collapsedPowerRegVisibility;
            }
            set
            {
                _collapsedPowerRegVisibility = value;
                OnPropertyChanged("CollapsedPowerRegVisibility");
            }
        }

        public Visibility CollapsedShutter2Visibility
        {
            get
            {
                return _collapsedShutter2Visibility;
            }
            set
            {
                _collapsedShutter2Visibility = value;
                OnPropertyChanged("CollapsedShutter2Visibility");
            }
        }

        public double CollapsedXPosition
        {
            get
            {
                double xPosition = (double)MVMManager.Instance["XYTileControlViewModel", "XPosition", (object)0.0] * CollapsedConverRatio;
                return (double)Math.Round(xPosition, 4);
            }
        }

        public bool CollapsedXPosOutOfBounds
        {
            get
            {
                return (bool)MVMManager.Instance["XYTileControlViewModel", "XPosOutOfBounds", (object)false];
            }
        }

        public double CollapsedYPosition
        {
            get
            {
                double yPosition = (double)MVMManager.Instance["XYTileControlViewModel", "YPosition", (object)0.0] * CollapsedConverRatio;
                return (double)Math.Round(yPosition, 4);
            }
        }

        public bool CollapsedYPosOutOfBounds
        {
            get
            {
                return (bool)MVMManager.Instance["XYTileControlViewModel", "YPosOutOfBounds", (object)false];
            }
        }

        public double CollapsedZPosition
        {
            get
            {
                return (double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0.0];
            }
        }

        public bool CollapsedZPosOutOfBounds
        {
            get
            {
                return (bool)MVMManager.Instance["ZControlViewModel", "ZPosOutOfBounds", (object)false];
            }
        }

        public Guid CommandGuid
        {
            get { return this._captureSetup.CommandGuid; }
        }

        public CaptureSetup.DigitizerBoardNames DigitizerBoardName
        {
            get
            {
                return this._captureSetup.DigitizerBoardName;
            }
            set
            {
                this._captureSetup.DigitizerBoardName = value;
                OnPropertyChanged("DigitizerBoardName");
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

        public bool DrawLineForLineScan
        {
            get
            {
                return true;
            }
            set
            {
                if (null != DrawLineForLineScanEvent)
                {
                    DrawLineForLineScanEvent();
                }
            }
        }

        public bool EnableDeviceQuery
        {
            get
            {
                return _enableDeviceQuery;
            }
            set
            {
                _enableDeviceQuery = value;
            }
        }

        public bool EnableDeviceReading
        {
            get;
            set;
        }

        public XmlDocument ExperimentDoc
        {
            get
            {
                return _captureSetup.ExperimentDoc;
            }
            set
            {
                _captureSetup.ExperimentDoc = value;
            }
        }

        public String ExpPath
        {
            get
            {
                return this._captureSetup.ExpPath;

            }

            set
            {
                this._captureSetup.ExpPath = value;
                PublishChangeEvent();
            }
        }

        public double FieldSizeHeightUM
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType())
                {
                    OverlayManagerClass.Instance.BinY = 1;
                    return (double)MVMManager.Instance["AreaControlViewModel", "LSMFieldSizeYUM", (double)1.0];
                }
                else
                {
                    return (double)MVMManager.Instance["CameraControlViewModel", "CamPixelSizeUM", (object)1.0] * ((int)MVMManager.Instance["CameraControlViewModel", "Bottom", (object)1] - (int)MVMManager.Instance["CameraControlViewModel", "Top", (object)0]);
                }
            }
        }

        public double FieldSizeWidthUM
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType())
                {
                    OverlayManagerClass.Instance.BinX = 1;
                    return (double)MVMManager.Instance["AreaControlViewModel", "LSMFieldSizeXUM", (object)1.0];
                }
                else
                {
                    return (double)MVMManager.Instance["CameraControlViewModel", "CamPixelSizeUM", (object)1.0] * ((int)MVMManager.Instance["CameraControlViewModel", "Right", (object)1] - (int)MVMManager.Instance["CameraControlViewModel", "Left", (object)0]);
                }
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
            }
        }

        //This is used to forbid CreateLineProfileWindow from generating a new line profile window
        // after a template has been loaded
        public bool IgnoreLineProfileGeneration
        {
            get
            {
                return _ignoreLineProfileGeneration;
            }
            set
            {
                _ignoreLineProfileGeneration = value;
            }
        }

        public int ImageHeight
        {
            get
            {
                return _captureSetup.ImageHeight;
            }
        }

        public XmlDocument ImageProcessDoc
        {
            get
            {
                return this._imageProcessDoc;
            }
            set
            {
                this._imageProcessDoc = value;
            }
        }

        public Visibility ImagerViewVis
        {
            get
            {
                return _imagerViewVis;
            }
            set
            {
                _imagerViewVis = value;
                OnPropertyChanged("ImagerViewVis");
            }
        }

        public int ImageWidth
        {
            get
            {
                return _captureSetup.ImageWidth;
            }
        }

        public bool IsProgressWindowOff
        {
            get { return _isProgressWindowOff; }
            set
            {
                MVMManager.Instance["PowerControlViewModel", "PockelsCalibrateAgainEnable"] = _isProgressWindowOff = value;
                OnPropertyChanged("IsProgressWindowOff");
                OnPropertyChanged("SLMBleachNowEnabled");
                OnPropertyChanged("BleachNowEnable");
                OnPropertyChanged("SLMPanelAvailable");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("PreviewButtonEnabled");
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

        public string LSMAreaName
        {
            get
            {
                return (string)MVMManager.Instance["AreaControlViewModel", "LSMAreaName", (object)string.Empty];
            }
        }

        public string LSMScannerName
        {
            get
            {
                return (string)MVMManager.Instance["ScanControlViewModel", "LSMScannerName", (object)string.Empty];
            }
        }

        public int MaxChannels
        {
            get
            {
                return _captureSetup.MaxChannels;
            }
        }

        public MultiROIStatsUC MultiROIStats
        {
            get { return _multiROIStats; }
            set { _multiROIStats = value; }
        }

        public string OutputDirectory
        {
            get
            {
                if (Application.Current == null)
                {
                    return string.Empty;
                }

                ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/Last3dOutputPath");
                string str = string.Empty;
                if (true == XmlManager.GetAttribute(node, ApplicationDoc, "path", ref str))
                {
                    return str;
                }
                else
                {
                    return Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString();
                }
            }
            set
            {
                if (Application.Current != null)
                {
                    ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                    XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/Last3dOutputPath");
                    if (null == node)
                    {
                        XmlNode tempNode = ApplicationDoc.CreateNode(XmlNodeType.Element, "Last3dOutputPath", null);
                        ApplicationDoc.DocumentElement.AppendChild(tempNode);
                        node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/Last3dOutputPath");
                    }
                    XmlManager.SetAttribute(node, ApplicationDoc, "path", value);
                    OnPropertyChanged("OutputDirectory");
                }
            }
        }

        public string OutputExperiment
        {
            get
            {
                return _OutputExperiment;
            }
            set
            {
                _OutputExperiment = value;
                OnPropertyChanged("OutputExperiment");
            }
        }

        public Double PanelBorderWidth
        {
            get
            {
                return _wrapPanelWidth - 3;
            }
        }

        public double PanelsScale
        {
            get
            {
                return _panelsScale;
            }
            set
            {
                _panelsScale = value;
                OnPropertyChanged("PanelsScale");
            }
        }

        public string QuickTemplateExperimentPath
        {
            get
            {
                return _quickTemplateExperimentPath;
            }
            set
            {
                if (File.Exists(value))
                {
                    _quickTemplateExperimentPath = value;
                    var mainWindow = _container.Resolve<MainWindow>();
                    mainWindow.MasterView.UpdateExperimentInfo(_quickTemplateExperimentPath);
                }
                else
                {
                    MessageBox.Show("Could not find Template file: " + value);
                }
            }
        }

        public System.Windows.Controls.Primitives.ToggleButton ReticleBotton
        {
            get;
            set;
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

        public bool ROIStatsTableActive
        {
            get
            {
                return _roiStatsTableActive;
            }
            set
            {
                _roiStatsTableActive = value;
                if (true == _roiStatsTableActive)
                {
                    if (null != _multiROIStats)
                    {
                        _multiROIStats.Show();
                    }
                }
                else
                {
                    if (null != _multiROIStats)
                    {
                        _multiROIStats.Hide();
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

        public double ROIToolsScale
        {
            get
            {
                return (ResourceManagerCS.Instance.TabletModeEnabled) ? 0.85 : 1;
            }
        }

        public ICommand SaveStatChartCommand
        {
            get
            {
                if (_saveStatChartCommand == null)
                {
                    _saveStatChartCommand = new RelayCommand(() => OnSaveStatChart());
                }

                return _saveStatChartCommand;
            }
        }

        public System.Windows.Controls.Primitives.ToggleButton ScaleBotton
        {
            get;
            set;
        }

        public bool UnloadWholeLineProfile
        {
            get;
            set;
        }

        public bool UnloadWholeROIStatsChart
        {
            get;
            set;
        }

        public bool UnloadWholeStats
        {
            get;
            set;
        }

        public int ViewType
        {
            get
            {
                return _viewType;
            }
            set
            {
                _viewType = value;
                OnPropertyChanged("ViewType");
            }
        }

        public bool WrapPanelEnabled
        {
            get
            {
                return _wrapPanelEnabled;
            }
            set
            {
                _wrapPanelEnabled = value;
                OnPropertyChange("WrapPanelEnabled");
            }
        }

        public Double WrapPanelWidth
        {
            get
            {
                return _wrapPanelWidth;
            }
            set
            {
                _wrapPanelWidth = value;
                OnPropertyChanged("WrapPanelWidth");
                OnPropertyChanged("PanelBorderWidth");
            }
        }

        public bool XYCtrlVisible
        {
            get
            {
                return _xyCtrlVisible;
            }
            set
            {
                _xyCtrlVisible = value;
                if (!_xyCtrlVisible)    // switch to 2D view is tile icon hidden
                {
                    _viewType = 0;  // 0 2D; 1 3D; 2 tile
                    OnPropertyChanged("ViewType");
                }

                OnPropertyChanged("XYCtrlVisible");
            }
        }

        public double YScale
        {
            get
            {
                return this._captureSetup.YScale;
            }
        }

        public string ZStageName
        {
            get
            {
                return (string)MVMManager.Instance["ZControlViewModel", "ZStageName", (object)string.Empty];
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

        public static void DeleteDirectory(string target_dir)
        {
            DirectoryInfo dirInfo = new DirectoryInfo(target_dir);
            foreach (FileInfo file in dirInfo.GetFiles())
            {
                file.Delete();
            }
            foreach (DirectoryInfo dir in dirInfo.GetDirectories())
            {
                dir.Delete(true);
            }

            //string[] files = Directory.GetFiles(target_dir);
            //string[] dirs = Directory.GetDirectories(target_dir);

            //foreach (string file in files)
            //{
            //    File.SetAttributes(file, FileAttributes.Normal);
            //    File.Delete(file);
            //}

            //foreach (string dir in dirs)
            //{
            //    DeleteDirectory(dir);
            //}

            //try
            //{
            //    Directory.Delete(target_dir, false);
            //}
            //catch (System.IO.IOException)
            //{
            //    //This gives a locking application (Ex. Windows Explorer) an
            //    //opportunity to release the directory handle
            //    System.Threading.Thread.Sleep(1000);
            //    Directory.Delete(target_dir, false);
            //}
        }

        public static string GetValueString(string xPath, string attrName)
        {
            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            string tmp = string.Empty;
            XmlNodeList ndList = appSettings.SelectNodes(xPath);
            if (ndList.Count > 0)
            {
                if (null != ndList[0].Attributes.GetNamedItem(attrName))
                {
                    tmp = ndList[0].Attributes[attrName].Value;
                }
            }
            return tmp;
        }

        public static Visibility GetVisibility(string xPath, string attrName)
        {
            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList = appSettings.SelectNodes(xPath);
            if (ndList.Count > 0)
            {
                string tmp = string.Empty;
                if (null != ndList[0].Attributes.GetNamedItem(attrName))
                {
                    tmp = ndList[0].Attributes[attrName].Value;
                    return tmp.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
            }
            return Visibility.Collapsed;
        }

        [DllImport(".\\CaptureSetup.dll", EntryPoint = "SetupCaptureBuffers")]
        public static extern int SetupCaptureBuffers();

        public void CloseFloatingWindows()
        {
            this.UnloadWholeStats = true;
            this.UnloadWholeLineProfile = true;
            this.UnloadWholeROIStatsChart = true;
            //close any floating panels or windows
            if (null != _multiROIStats)
            {
                _multiROIStats.Hide();
                _multiROIStats.Close();
            }

            if (null != _roiStatsChart)
            {
                _roiStatsChart.Close();
                //In CaptureSetup _roiStatsChart is not closed (only hidden)
                //force the persistance of the window settings
                PersistROIStatsChartWindowSettings();
            }

            MVMManager.Instance["PowerControlViewModel", "ClosePropertyWindows"] = true;
            MVMManager.Instance["DFLIMControlViewModel", "ClosePropertyWindows"] = true;

            if (null != _autoContourWin)
            {
                _autoContourWin.Close();
            }

            if (null != _lineProfile)
            {
                _lineProfile.Close();
            }

            if (null != _slmParamEditWin)
            {
                _slmParamEditWin.Close();
            }

            this.UnloadWholeStats = false;
            this.UnloadWholeLineProfile = false;
            this.UnloadWholeROIStatsChart = false;
        }

        public void CloseProgressWindow()
        {
            if (null != this._spinnerWindow)
            {
                this._spinnerWindow.Close();
            }
        }

        public void ConnectHandlers()
        {
            _bw.DoWork += new DoWorkEventHandler(bw_DoWork);
            _bw.ProgressChanged += new ProgressChangedEventHandler(bw_ProgressChanged);
            _bw.RunWorkerCompleted += new RunWorkerCompletedEventHandler(bw_RunWorkerCompleted);
            _bleachWorker.DoWork += new DoWorkEventHandler(bleachWorker_DoWork);
            _slmBleachWorker.DoWork += new DoWorkEventHandler(slmBleachWorker_DoWork);

            _deviceReadTimer.Tick += new EventHandler(_deviceReadTimer_Tick);
            _deviceReadTimer.Start();
            _pmtSafetyTimer.Tick += new EventHandler(_pmtSafetyTimer_Tick);
            _pmtSafetyTimer.Start();

            if (null == _bwHardware)
            {
                _bwHardware = new BackgroundWorker();
                _bwHardware.WorkerReportsProgress = false;
                _bwHardware.WorkerSupportsCancellation = true;

                _bwHardware.DoWork += new DoWorkEventHandler(_bwHardware_DoWork);
                _bwHardware.RunWorkerCompleted += new RunWorkerCompletedEventHandler(_bwHardware_RunWorkerCompleted);
            }

            if (false == _bwHardware.IsBusy)
            {
                _bwHardware.RunWorkerAsync();
            }
            else if (true == _bwHardware.CancellationPending)
            {
                _restartBWHardware = true;
            }

            OverlayManagerClass.Instance.MaskChangedEvent += _overlayManager_MaskChangedEvent;

            this._captureSetup.ConnectHandlers();
        }

        /// <summary>
        /// backup Active Settings to an experiment or a template,
        /// Settings include: Active.xml, ActiveROIs.xaml, BleachROIs.xaml, BleachWaveform.raw, and SLMWaveforms
        /// </summary>
        /// <param name="TemplateSettingFile"></param>
        public bool CreateTemplateSettings(string TemplateSettingFile)
        {
            try
            {
                string templatesFolder = ResourceManagerCS.GetCaptureTemplatePathString();
                string expActiveXML = ResourceManagerCS.GetActiveSettingsFileString();
                string expBleachingROIsXAML = templatesFolder + "BleachROIs.xaml";
                string expBleachingWaveFormFile = templatesFolder + "BleachWaveform.raw";
                string expROIMask = templatesFolder + "ActiveROIMask.raw";

                string expTemplatesFldr = System.IO.Path.GetDirectoryName(TemplateSettingFile);
                string templateName = System.IO.Path.GetFileNameWithoutExtension(TemplateSettingFile);
                string pathTemplateROIsXAML = expTemplatesFldr + "\\" + templateName + "\\ROIs.xaml";
                string pathTemplateROIMask = expTemplatesFldr + "\\" + templateName + "\\ROIMask.raw";
                string pathTemplateBleachingROIsXAML = expTemplatesFldr + "\\" + templateName + "\\BleachROIs.xaml";
                string pathTemplateBleachingWaveFormH5 = expTemplatesFldr + "\\" + templateName + "\\BleachWaveform.raw";
                string pathTemplateSLMWaveFormFolder = expTemplatesFldr + "\\" + templateName + "\\SLMWaveforms";

                Directory.CreateDirectory(expTemplatesFldr + "\\" + templateName);

                OverlayManagerClass.Instance.SaveROIs(pathTemplateROIsXAML);

                FileCopyWithExistCheck(expActiveXML, TemplateSettingFile, true);

                FileCopyWithExistCheck(expROIMask, pathTemplateROIMask, true);

                FileCopyWithExistCheck(expBleachingROIsXAML, pathTemplateBleachingROIsXAML, true);

                FileCopyWithExistCheck(expBleachingWaveFormFile, pathTemplateBleachingWaveFormH5, true);

                //copy SLM:
                if (Directory.Exists(SLMWaveformFolder[0]))
                {
                    Directory.CreateDirectory(pathTemplateSLMWaveFormFolder);
                    //create directories:
                    foreach (string dirPath in Directory.GetDirectories(SLMWaveformFolder[0], "*", SearchOption.AllDirectories))
                        Directory.CreateDirectory(dirPath.Replace(SLMWaveformFolder[0], pathTemplateSLMWaveFormFolder));

                    //copy all the files & replaces any files with the same name
                    foreach (string newPath in Directory.GetFiles(SLMWaveformFolder[0], "*.*", SearchOption.AllDirectories))
                        FileCopyWithExistCheck(newPath, newPath.Replace(SLMWaveformFolder[0], pathTemplateSLMWaveFormFolder), true);
                }
                return true;
            }
            catch (Exception ex)
            {
                MessageBox.Show("Save template failed.\n\n" + ex.Message);
                return false;
            }
        }

        public void DeleteFile(string target_file)
        {
            try
            {
                if (File.Exists(target_file))
                {
                    File.Delete(target_file);
                }
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "DeleteFile '" + target_file + "': " + ex.Message);
            }
        }

        public bool DisplayROI(string pathName = "")
        {
            pathName = (string.Empty == pathName) ? (ResourceManagerCS.GetCaptureTemplatePathString() + "ActiveROIs.xaml") : pathName;

            if (!File.Exists(pathName))
                return false;

            ROIUpdateRequested(pathName);
            return true;
        }

        public Color GetColorAssignment(int index)
        {
            Color colorAssignment = new Color();
            const int LUT_SIZE = 256;

            colorAssignment = CaptureSetup.ChannelLuts[index][LUT_SIZE - 1];

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

        public string GetExperimentSavingPath()
        {
            return _captureSetup.SnapshotSavingPath;
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(CaptureSetupViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        void IDropTarget.DragOver(IDropInfo dropInfo)
        {
            DragDrop.DefaultDropHandler.DragOver(dropInfo);
            if (dropInfo.DragInfo.SourceCollection.Equals(_captureSequence) &&
                dropInfo.TargetCollection.Equals(_captureSequence))
            {
                dropInfo.Effects = DragDropEffects.Move;
                if (false == _isDraggingLightPathSequenceStep)
                {
                    //Persist the dragged script item
                    LightPathSequenceStep data = (LightPathSequenceStep)dropInfo.Data;
                    _previousCaptureSequenceSelectedLine = data.SequenceLineNumber - 1;
                    if (null != _captureSequence[_previousCaptureSequenceSelectedLine])
                    {
                        _draggedLightPathSequenceStep = _captureSequence[_previousCaptureSequenceSelectedLine];
                        _isDraggingLightPathSequenceStep = true;
                    }
                }
            }
            else if (dropInfo.DragInfo.SourceCollection.Equals(_lightPaths) &&
                dropInfo.TargetCollection.Equals(_lightPaths))
            {
                dropInfo.Effects = DragDropEffects.Move;
                if (false == _isDraggingLightPathSequenceStep)
                {
                    //Persist the dragged script item
                    LightPathSequenceStep data = (LightPathSequenceStep)dropInfo.Data;
                    _previousCaptureLightPathSelectedLine = data.LightPathLineNumber - 1;
                    if (null != _lightPaths[_previousCaptureLightPathSelectedLine])
                    {
                        _draggedLightPathSequenceStep = _lightPaths[_previousCaptureLightPathSelectedLine];
                        _isDraggingLightPathSequenceStep = true;
                    }
                }
            }
            else if (dropInfo.DragInfo.SourceCollection.Equals(_captureSequence) &&
                dropInfo.TargetCollection.Equals(_lightPaths))
            {
                dropInfo.Effects = DragDropEffects.None;
                _isDraggingLightPathSequenceStep = false;
            }
            else if (dropInfo.DragInfo.SourceCollection.Equals(SLMBleachWaveParams) &&
               dropInfo.TargetCollection.Equals(SLMBleachWaveParams))
            {
                dropInfo.Effects = DragDropEffects.Move;
            }
        }

        void IDropTarget.Drop(IDropInfo dropInfo)
        {
            try
            {
                _isDraggingLightPathSequenceStep = false;
                if (dropInfo.DragInfo.SourceCollection.Equals(_captureSequence) &&
                    dropInfo.TargetCollection.Equals(_captureSequence))
                {
                    if (_draggedLightPathSequenceStep != null)
                    {
                        //Persist the dragged script item
                        _captureSequence[_previousCaptureSequenceSelectedLine] = _draggedLightPathSequenceStep;
                    }
                    //moving an existing item in the list
                    DragDrop.DefaultDropHandler.Drop(dropInfo);
                    ReassignCaptureSequenceLineNumbers();
                }
                else if (dropInfo.DragInfo.SourceCollection.Equals(_lightPaths) &&
                    dropInfo.TargetCollection.Equals(_captureSequence))
                {
                    //adding a new item from the command list
                    if (_captureSequence.Count == MAX_CHANNEL_STEP_COUNT)
                    {
                        MessageBox.Show("The maximum number of Channel Steps is 4.");
                        return;
                    }
                    LightPathSequenceStep data = (LightPathSequenceStep)dropInfo.Data;
                    bool dChanEnable0 = false, dChanEnable1 = false, dChanEnable2 = false, dChanEnable3 = false;
                    data.GetLightPathSequenceStepLSMChannel(ref dChanEnable0, ref dChanEnable1, ref dChanEnable2, ref dChanEnable3);
                    int chanASum = (dChanEnable0) ? 1 : 0;
                    int chanBSum = (dChanEnable1) ? 1 : 0;
                    int chanCSum = (dChanEnable2) ? 1 : 0;
                    int chanDSum = (dChanEnable3) ? 1 : 0;
                    for (int j = 0; j < _captureSequence.Count; j++)
                    {
                        bool chanEnable0 = false, chanEnable1 = false, chanEnable2 = false, chanEnable3 = false;
                        _captureSequence[j].GetLightPathSequenceStepLSMChannel(ref chanEnable0, ref chanEnable1, ref chanEnable2, ref chanEnable3);
                        chanASum += (true == chanEnable0) ? 1 : 0;
                        chanBSum += (true == chanEnable1) ? 1 : 0;
                        chanCSum += (true == chanEnable2) ? 1 : 0;
                        chanDSum += (true == chanEnable3) ? 1 : 0;
                    }

                    //If a channel is repeated display a message to the user and return
                    if (1 < chanASum || 1 < chanBSum || 1 < chanCSum || 1 < chanDSum)
                    {
                        MessageBox.Show("Each channel is allowed only once per Capture Sequence. Make sure none of the selected channels for this channel step is selected in a different channel step.");
                        return;
                    }

                    LightPathSequenceStep si = new LightPathSequenceStep(data.Name, data.LightPathSequenceStepNode, dropInfo.InsertIndex, this.DigitizerBoardName);

                    _captureSequence.Insert(dropInfo.InsertIndex, si);
                    ReassignCaptureSequenceLineNumbers();
                }
                else if (dropInfo.DragInfo.SourceCollection.Equals(_lightPaths) &&
                    dropInfo.TargetCollection.Equals(_lightPaths))
                {
                    if (_draggedLightPathSequenceStep != null)
                    {
                        //Persist the dragged script item
                        _lightPaths[_previousCaptureLightPathSelectedLine] = _draggedLightPathSequenceStep;
                    }
                    //moving an existing item in the list
                    DragDrop.DefaultDropHandler.Drop(dropInfo);
                    ReassignLightPathListLineNumbers();
                }
                else if (dropInfo.DragInfo.SourceCollection.Equals(SLMBleachWaveParams) &&
                    dropInfo.TargetCollection.Equals(SLMBleachWaveParams))
                {
                    //moving an existing item in the list
                    DragDrop.DefaultDropHandler.Drop(dropInfo);
                    if (!this.CompareSLMParams())
                    {
                        EditSLMParam("SLM_BUILD");
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        public void LoadXMLSettings()
        {
            string str = string.Empty, str1 = string.Empty;
            double dTmp = 0.0, dTmp1 = 0.0;
            int iTmp = 0;
            Int64 i64Tmp = 0;
            UInt32 uiTmp = 0;

            //determine the bleach type, GG or WideField
            SLMPhaseDirect = IsStimulator;

            //load exp
            ExperimentDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            XmlNodeList ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Photobleaching");
            //All parameters were for bleach scanner,
            //need to be converted for camera:
            double bleacherFieldSizeCal = 100.0, fieldSizeCalInvRatio = 1.0;
            GetLSMBleacherFieldSizeCalibration(ref bleacherFieldSizeCal);
            fieldSizeCalInvRatio = (double)MVMManager.Instance["AreaControlViewModel", "LSMFieldSizeCalibration", (object)100.0] / bleacherFieldSizeCal;

            if (ndList.Count > 0)
            {
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "bleachFrames", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    BleachFrames = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "powerEnable", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    BleachPowerEnable = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "powerPos", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    BleachPower = dTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "laserEnable", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    BleachWavelengthEnable = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "laserPos", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    BleachWavelength = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "bleachQuery", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    BleachQuery = iTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "pixelSizeUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    BleachLSMUMPerPixel = dTmp;
                }
            }

            //SLM Patterns:
            SLMBleachWaveParams.Clear();  //test
            ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/SLM/Pattern");
            if ((ndList.Count > 0) && SLMPanelInUse)
            {

                for (int i = 0; i < ndList.Count; i++)
                {
                    GeometryUtilities.SLMParams sparam = new GeometryUtilities.SLMParams();

                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "name", ref str))
                    {
                        sparam.Name = str;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "pixelSizeUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.BleachWaveParams.UMPerPixel = dTmp;
                        sparam.BleachWaveParams.UMPerPixelRatio = (0 < BleachLSMUMPerPixel) ? (sparam.BleachWaveParams.UMPerPixel / BleachLSMUMPerPixel) : 1.0;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "xOffsetUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        double tmpY = 0;
                        if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "yOffsetUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpY))
                            sparam.BleachWaveParams.CenterUM = new Point(dTmp, tmpY);
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "roiWidthUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.BleachWaveParams.ROIWidthUM = dTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "roiHeightUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.BleachWaveParams.ROIHeightUM = dTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "pxSpacing", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                    {
                        sparam.PixelSpacing = PixelDensity = iTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "durationMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.Duration = dTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "iterations", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                    {
                        sparam.BleachWaveParams.Iterations = iTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "prePatIdleMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.BleachWaveParams.PrePatIdleTime = dTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "postPatIdleMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.BleachWaveParams.PostPatIdleTime = dTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "preIteIdleMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.BleachWaveParams.PreIdleTime = dTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "postIteIdleMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.BleachWaveParams.PostIdleTime = dTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "power", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.BleachWaveParams.Power = dTmp;
                        sparam.BleachWaveParams.Power1 = (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "power1", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp1)) ?
                                                        dTmp1 : -1.0;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "measurePowerMW", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.BleachWaveParams.MeasurePower = dTmp;
                        if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "measurePower1MW", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp1))
                            sparam.BleachWaveParams.MeasurePower1 = dTmp1;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "measurePowerMWPerUM2", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.SLMMeasurePowerArea = dTmp;
                        if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "measurePower1MWPerUM2", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp1))
                            sparam.SLMMeasurePowerArea1 = dTmp1;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "red", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.Red = dTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "green", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.Green = dTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "blue", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        sparam.Blue = dTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "fileID", ref str) && UInt32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out uiTmp))
                    {
                        sparam.BleachWaveParams.ID = uiTmp;
                    }
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "zValue", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {

                        sparam.BleachWaveParams.ZValue = dTmp;

                    }

                    str = (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "shape", ref str)) ? str : string.Empty;
                    sparam.BleachWaveParams.shapeType = String.IsNullOrWhiteSpace(str) ? ((0 < sparam.BleachWaveParams.ROIWidthUM) ? "Ellipse" : "Crosshair") : str;
                    SLMBleachWaveParams.Add(sparam);
                }

                //verify with bitmaps, empty table if not equivalent
                List<string> slmPatternsInFolder = Directory.EnumerateFiles(SLMWaveformFolder[0], "*.bmp ", SearchOption.TopDirectoryOnly).ToList();
                if (ndList.Count != slmPatternsInFolder.Count)
                {
                    SLMBleachWaveParams.Clear();
                }
                for (int i = 0; i < SLMBleachWaveParams.Count; i++)
                {
                    if (!slmPatternsInFolder.Contains(SLMWaveformFolder[0] + "\\" + SLMWaveBaseName[1] + "_" + SLMBleachWaveParams[i].BleachWaveParams.ID.ToString("D" + FileName.GetDigitCounts().ToString()) + ".bmp"))
                    {
                        SLMBleachWaveParams.Clear();
                        break;
                    }
                }

                //update params for future comparison:
                UpdateSLMCompParams();
            }
            //SLM Sequences: load before epoch count for building waveforms
            EpochSequence.Clear();
            ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/SLM/SequenceEpoch");
            if ((ndList.Count > 0) && SLMPanelInUse)
            {
                for (int i = 0; i < ndList.Count; i++)
                {
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "sequence", ref str) &&
                        (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "sequenceEpochCount", ref str1) && Int32.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                        )
                    {
                        EpochSequence.Add(new SLMEpochSequence(str, iTmp, SLMBleachWaveParams.Count));
                    }
                }
            }
            //SLM:
            ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/SLM");
            if ((ndList.Count > 0) && SLMPanelInUse)
            {
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "lastCalibTimeUnix", ref str) && Int64.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out i64Tmp))
                {
                    SLMLastCalibTimeUnix = i64Tmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "calibPower", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    SLMCalibPower = dTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "calibDwell", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    SLMCalibDwell = dTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "cycleDelay", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    SLMBleachDelay = dTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "advanceMode", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    SLMSequenceOn = (1 == iTmp);
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "holoGen3D", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    SLM3D = (1 == iTmp);
                }
                //keep epochCount last to update
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "epochCount", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    EpochCount = iTmp;
                }
            }

            ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/LightPath");

            if (ndList.Count > 0)
            {

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "GalvoGalvo", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    LightPathGGEnable = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "GalvoResonance", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    LightPathGREnable = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "Camera", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    LightPathCamEnable = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "InvertedLightPathPos", ref str) && "-1" != str)
                {
                    switch (str)
                    {
                        case "0":
                            InvertedLpLeftEnable = true;
                            break;
                        case "1":
                            InvertedLpCenterEnable = true;
                            break;
                        case "2":
                            InvertedLpRightEnable = true;
                            break;
                    }
                }
            }

            ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/CaptureSequence");
            if (ndList.Count > 0)
            {
                EnableSequentialCapture = 0;
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "enable", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    EnableSequentialCapture = (1 == iTmp) ? 1 : 0;
                }
            }

            ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
            CollectionCaptureSequence = new ObservableCollection<LightPathSequenceStep>();
            for (int i = 0; i < ndList.Count; i++)
            {
                if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "name", ref str))
                {
                    //We want the channel step line numbers to start at 1
                    LightPathSequenceStep si = new LightPathSequenceStep(str, ndList[i], i + 1, DigitizerBoardName);
                    CollectionCaptureSequence.Add(si);
                }
            }

            string lightPathListFolder = Application.Current.Resources["LightPathListFolder"].ToString();
            string lightPathListFile = lightPathListFolder + "\\LightPathList.xml";

            if (true == Directory.Exists(lightPathListFolder))
            {
                XmlDocument lightPathListDoc = new XmlDocument();
                lightPathListDoc.Load(lightPathListFile);
                ndList = lightPathListDoc.SelectNodes("/ThorImageLightPathList/LightPathSequenceStep");
                CollectionLightPaths = new ObservableCollection<LightPathSequenceStep>();

                for (int i = 0; i < ndList.Count; i++)
                {
                    if (XmlManager.GetAttribute(ndList[i], lightPathListDoc, "name", ref str))
                    {
                        //we want the channel step line number to start at 1
                        LightPathSequenceStep si = new LightPathSequenceStep(str, ndList[i], i + 1, DigitizerBoardName);
                        CollectionLightPaths.Add(si);
                    }
                }
            }
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        public void PersistROIStatsChartWindowSettings()
        {
            if (null == _roiStatsChart)
            {
                return;
            }
            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");

            if (node != null)
            {
                string str = string.Empty;
                if (true == XmlManager.GetAttribute(node, ApplicationDoc, "reset", ref str) && "0" == str)
                {

                    XmlManager.SetAttribute(node, ApplicationDoc, "left", ((int)Math.Round(_roiStatsChart.Left)).ToString());
                    XmlManager.SetAttribute(node, ApplicationDoc, "top", ((int)Math.Round(_roiStatsChart.Top)).ToString());
                    XmlManager.SetAttribute(node, ApplicationDoc, "width", ((int)Math.Round(_roiStatsChart.Width)).ToString());
                    XmlManager.SetAttribute(node, ApplicationDoc, "height", ((int)Math.Round(_roiStatsChart.Height)).ToString());
                    XmlManager.SetAttribute(node, ApplicationDoc, "display", (((ROIStatsChartActive) ? 1 : 0).ToString()));
                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                }
            }
            _roiStatsChart = null;
        }

        public Decimal Power2PercentConvertion(double value, double Max, double Min)
        {
            if (Max != Min)
            {
                Decimal dec = new Decimal((value - Min) * 100 / (Max - Min));
                return dec = Decimal.Round(dec, 2);
            }
            else return new Decimal(value);
        }

        public void PublishChangeEvent()
        {
            ChangeEvent changeEvent = new ChangeEvent();
            changeEvent.ModuleName = "CaptureSetup";
            changeEvent.IsChanged = true;

            //command published to change the status of the menu buttons in the Menu Control
            _eventAggregator.GetEvent<MenuModuleChangeEvent>().Publish(changeEvent);
        }

        public void RefreshAllUIBindings()
        {
            OnPropertyChanged("");
        }

        public void ReleaseHandlers()
        {
            this._captureSetup.UpdateImage -= new Action<bool>(LiveImage_Update);
            this._captureSetup.ReleaseHandlers();
            _bw.DoWork -= new DoWorkEventHandler(bw_DoWork);
            _bw.ProgressChanged -= new ProgressChangedEventHandler(bw_ProgressChanged);
            _bw.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(bw_RunWorkerCompleted);
            _bleachWorker.DoWork -= new DoWorkEventHandler(bleachWorker_DoWork);
            _slmBleachWorker.DoWork -= new DoWorkEventHandler(slmBleachWorker_DoWork);
            _deviceReadTimer.Stop();
            _deviceReadTimer.Tick -= new EventHandler(_deviceReadTimer_Tick);
            _pmtSafetyTimer.Stop();
            _pmtSafetyTimer.Tick -= new EventHandler(_pmtSafetyTimer_Tick);
            if (_bwHardware != null)
                _bwHardware.CancelAsync();

            OverlayManagerClass.Instance.MaskChangedEvent -= _overlayManager_MaskChangedEvent;
            //close any floating panels or windows
            CloseFloatingWindows();
        }

        //Get and replace Active Settings from an experiment or a template
        //Settings include: Active.xml, ROIs.xaml, BleachROIsxaml, BleachWaveform.raw, and SLMWaveforms
        public void ReplaceActiveSettings(string expSettingsFile)
        {
            try
            {
                string expFolder;
                string expROIsXAML;
                string expROIMask;
                string expBleachingROIsXAML;
                string expBleachingWaveFormFile;
                string expSpectralSequence;
                string expSLMBleachFolder;
                //if (string.Empty == _settingsTemplateName)
                //{
                expFolder = System.IO.Path.GetDirectoryName(expSettingsFile);
                if (!File.Exists(expFolder + "\\ROIs.xaml"))
                { expFolder = expSettingsFile.Substring(0, expSettingsFile.LastIndexOf('.')); }

                expROIsXAML = expFolder + "\\ROIs.xaml";
                expROIMask = expFolder + "\\ROIMask.raw";
                expBleachingROIsXAML = expFolder + "\\BleachROIs.xaml";
                expBleachingWaveFormFile = expFolder + "\\BleachWaveform.raw";
                expSpectralSequence = expFolder + "\\SpectralSequence.txt";
                expSLMBleachFolder = expFolder + "\\SLMWaveforms";
                //}
                //else
                //{
                //    expROIsXAML = _settingsTemplatesPath + "\\" + _settingsTemplateName + "\\ROIs.xaml";
                //    expROIMask = _settingsTemplatesPath + "\\" + _settingsTemplateName + "\\ROIMask.raw";
                //    expBleachingROIsXAML = _settingsTemplatesPath + "\\" + _settingsTemplateName + "\\BleachROIs.xaml";
                //    expBleachingWaveFormFile = _settingsTemplatesPath + "\\" + _settingsTemplateName + "\\BleachWaveform.raw";
                //    expSpectralSequence = _settingsTemplatesPath + "\\" + _settingsTemplateName + "\\SpectralSequence.txt";
                //    expSLMBleachFolder = _settingsTemplatesPath + "\\" + _settingsTemplateName + "\\SLMWaveforms";
                //}

                string tempFolder = ResourceManagerCS.GetCaptureTemplatePathString();
                string pathActiveXML = tempFolder + "Active.xml";
                string pathActiveROIsXAML = tempFolder + "ActiveROIs.xaml";
                string pathActiveROIMask = tempFolder + "ActiveROIMask.raw";
                string pathActiveBleachingROIsXAML = tempFolder + "BleachROIs.xaml";
                string pathActiveBleachingWaveformFile = tempFolder + "BleachWaveform.raw";
                string pathActiveSpectralSequence = tempFolder + "SpectralSequence.txt";

                FileCopyWithAccessCheck(expSettingsFile, pathActiveXML, true);

                FileCopyWithAccessCheck(expROIsXAML, pathActiveROIsXAML, true);

                FileCopyWithAccessCheck(expROIMask, pathActiveROIMask, true);

                if (File.Exists(expBleachingROIsXAML))
                {
                    FileCopyWithAccessCheck(expBleachingROIsXAML, pathActiveBleachingROIsXAML, true);
                }
                else
                {
                    DeleteFile(pathActiveBleachingROIsXAML);
                }
                if (File.Exists(expBleachingWaveFormFile))
                {
                    FileCopyWithAccessCheck(expBleachingWaveFormFile, pathActiveBleachingWaveformFile, true);
                    BleachNowEnable = true;
                }
                else
                {
                    DeleteFile(pathActiveBleachingWaveformFile);
                    BleachNowEnable = false;
                }
                //SLM files:
                ClearSLMFiles(SLMWaveformFolder[0], "raw");
                ClearSLMFiles(SLMWaveformFolder[0], "bmp");
                if (Directory.Exists(expSLMBleachFolder))
                {
                    Directory.CreateDirectory(SLMWaveformFolder[0]);
                    //create directories:
                    foreach (string dirPath in Directory.GetDirectories(expSLMBleachFolder, "*", SearchOption.AllDirectories))
                        Directory.CreateDirectory(dirPath.Replace(expSLMBleachFolder, SLMWaveformFolder[0]));

                    //copy all the files & replaces any files with the same name
                    foreach (string newPath in Directory.GetFiles(expSLMBleachFolder, "*.*", SearchOption.AllDirectories))
                        File.Copy(newPath, newPath.Replace(expSLMBleachFolder, SLMWaveformFolder[0]), true);
                }
                //update with the new experiment document
                MVMManager.Instance.LoadSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);

                if (null != ActiveSettingsReplaced)
                {
                    ActiveSettingsReplaced();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Load experiment failed. {0}", ex.Message);
                return;
            }
        }

        //assign the attribute value to the input node and document
        //if the attribute does not exist add it to the document
        public void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attValue)
        {
            XmlNode tempNode = node.Attributes[attrName];

            if (null == tempNode)
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);

                attr.Value = attValue;

                node.Attributes.Append(attr);
            }
            else
            {
                node.Attributes[attrName].Value = attValue;
            }
        }

        public void UpdateOverlayManager()
        {
            try
            {
                if (OverlayManagerClass.Instance.UmPerPixel != (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0] ||
                    OverlayManagerClass.Instance.PixelX != this.DataWidth || OverlayManagerClass.Instance.PixelY != this.DataHeight)
                {
                    OverlayManagerClass.Instance.UpdateParams(this.DataWidth, this.DataHeight, (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0]);

                    if (null == Bitmap)
                    {
                        return;
                    }
                    if (true == ReticleBotton.IsChecked)
                    {
                        OverlayManagerClass.Instance.InitReticle(ref OverlayCanvas, true);
                    }
                    if (true == ScaleBotton.IsChecked)
                    {
                        OverlayManagerClass.Instance.InitScale(ref OverlayCanvas, true);
                    }
                }
            }
            catch (Exception e)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "UpdateOverlayManager failed. Exception Message:\n" + e.Message);
                return;
            }
        }

        protected virtual bool IsFileLocked(FileInfo file)
        {
            FileStream stream = null;

            try
            {
                stream = file.Open(FileMode.Open, FileAccess.Read, FileShare.None);
            }
            catch (IOException)
            {
                //the file is unavailable because it is:
                //still being written to
                //or being processed by another thread
                //or does not exist (has already been processed)
                return true;
            }
            finally
            {
                if (stream != null)
                    stream.Close();
            }

            //file is not locked
            return false;
        }

        [DllImport(".\\StatsManager.dll", EntryPoint = "GetNumROI")]
        private static extern int GetNumROI();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetPMTSafetyStatus")]
        private static extern bool GetPMTSafetyStatus();

        [DllImport(".\\ROIDataStore.dll", EntryPoint = "RequestROIData")]
        private static extern void RequestROIData();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "UpdateStats")]
        private static extern void UpdateStats();

        private void AutoContourDisplay()
        {
            if (_autoCoutourWinInit == false)
            {
                _autoContourWin = new AutoContourWin();
                _autoContourWin.DataContext = this;
                _autoContourWin.Show();
            }
        }

        private void bw_DoWork(object sender, DoWorkEventArgs e)
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
                    //TODO: changed from 30ms to 10ms, should be tested well.
                    System.Threading.Thread.Sleep(10);
                    worker.ReportProgress(0);
                }
            };
        }

        private void bw_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            if (CaptureSetup.IsPixelDataReady())
            {
                OnPropertyChanged("Bitmap");
                //AutoROI is not currently enabled. Comment property changed for now
                // TODO: Uncomment when features are implemented
                //OnPropertyChanged("BitmapPresentation");
                ((IMVM)MVMManager.Instance["ObjectiveControlViewModel", this]).OnPropertyChange("FramesPerSecondText");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("FramesPerSecondAverage");
                UpdateOverlayManager();
                RequestROIData();
            }
        }

        private void bw_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
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

        private int ConvertBoolAry2Int(bool[] boolArray, int nDigit)
        {
            int setter = 1, output = 0;
            for (int i = 0; i < nDigit; i++, setter <<= 1)
            {
                if (boolArray[i] == true)
                { output |= setter; }
            }
            return output;
        }

        private void CreateLineProfileWindow()
        {
            if ((null == _lineProfile && true == _lineProfileActive) && false == IgnoreLineProfileGeneration)
            {
                if (GetNumROI() <= 0)
                {
                    return;
                }

                _lineProfile = new LineProfileWindow.LineProfile(GetColorAssignments(), CaptureSetup.MAX_CHANNELS);
                _lineProfile.LineWidthMax = ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType()) ? (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)32] : (int)MVMManager.Instance["CameraControlViewModel", "CamImageHeight", (object)1];
                //_lineProfile.Closed += _lineProfile_Closed;
                _lineProfile.Closing += new System.ComponentModel.CancelEventHandler(_lineProfile_Closing);
                _lineProfile.LineWidthChange += _lineProfile_LineWidthChange;

                ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/LineProfileWindow");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;
                    double val = 0;
                    _lineProfile.Left = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "left", ref str) && (double.TryParse(str, out val))) ? (int)val : 0;
                    _lineProfile.Top = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "top", ref str) && (double.TryParse(str, out val))) ? (int)val : 0;
                    _lineProfile.Width = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "width", ref str) && (double.TryParse(str, out val))) ? (int)val : 400;
                    _lineProfile.Height = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "height", ref str) && (double.TryParse(str, out val))) ? (int)val : 400;
                    str = string.Empty;
                    _lineProfile.LineWidth = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "lineWidth", ref str) && (double.TryParse(str, out val))) ? (int)val : 1;
                    str = string.Empty;
                    // If the Autoscale option is not found in Application Settings then enable it by default
                    _lineProfile.AutoScaleOption = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "Autoscale", ref str)) ? (str == "1" || str == Boolean.TrueString) : true;
                    str = string.Empty;
                    _lineProfile.ConversionActiveOption = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "ConversionToUm", ref str)) && (str == "1" || str == Boolean.TrueString);
                    str = string.Empty;
                    _lineProfile.MinimumYVal = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "YMin", ref str) && (double.TryParse(str, out val))) ? (int)val : 0;
                    str = string.Empty;
                    _lineProfile.MaximumYVal = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "YMax", ref str) && (double.TryParse(str, out val))) ? (int)val : 17000;

                    XmlManager.SetAttribute(ndList[0], ApplicationDoc, "reset", "0");
                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                }

                _lineProfile.Show();
            }
            IgnoreLineProfileGeneration = false;
        }

        private void CreateMultiStatsWindow()
        {
            if (null == _multiROIStats && true == _roiStatsTableActive)
            {
                if (GetNumROI() <= 0 || false == _roiStatsTableActive)
                {
                    return;
                }
                _multiROIStats = new MultiROIStatsUC();
                //_multiROIStats.Closed += _multiROIStats_Closed;
                _multiROIStats.Closing += new System.ComponentModel.CancelEventHandler(_multiROIStats_Closing);

                ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIStatsWindow");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;
                    double val = 0;
                    _multiROIStats.Left = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "left", ref str) && (double.TryParse(str, out val))) ? (int)val : 0;
                    _multiROIStats.Top = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "top", ref str) && (double.TryParse(str, out val))) ? (int)val : 0;
                    _multiROIStats.Width = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "width", ref str) && (double.TryParse(str, out val))) ? (int)val : 400;
                    _multiROIStats.Height = (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "height", ref str) && (double.TryParse(str, out val))) ? (int)val : 400;

                    XmlManager.SetAttribute(ndList[0], ApplicationDoc, "reset", "0");
                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                }

                _multiROIStats.Show();
            }
        }

        private void CreateProgressWindow()
        {
            if (null != _spinnerWindow)
                return;
            //create a popup modal dialog that blocks user clicks while capturing
            _spinnerWindow = new Window();
            _spinnerWindow.Title = "Preview";
            _spinnerWindow.ResizeMode = ResizeMode.NoResize;
            _spinnerWindow.Width = 162;
            _spinnerWindow.Height = 162;
            _spinnerWindow.WindowStyle = WindowStyle.None;
            _spinnerWindow.Background = Brushes.DimGray;
            _spinnerWindow.AllowsTransparency = true;
            System.Windows.Controls.Border border = new System.Windows.Controls.Border();
            border.BorderThickness = new Thickness(2);
            //  border.BorderBrush = Brushes.Yellow;
            _spinnerWindow.Content = border;

            System.Windows.Controls.StackPanel sp = new System.Windows.Controls.StackPanel();
            border.Child = sp;

            _spinner = new SpinnerProgress.SpinnerProgressControl();
            _spinner.Margin = new Thickness(30, 5, 30, 5);
            System.Windows.Controls.Border borderSpinner = new System.Windows.Controls.Border();
            borderSpinner.Child = _spinner;
            sp.Children.Add(borderSpinner);

            System.Windows.Controls.Button stopButton = new System.Windows.Controls.Button();
            stopButton.Content = "Stop Preview";
            stopButton.Background = Brushes.Red;
            stopButton.Foreground = Brushes.White;
            stopButton.Height = 35;
            stopButton.Style = null;
            stopButton.Margin = new Thickness(75, 108, 5, 5);
            stopButton.Click += new RoutedEventHandler(stopButton_Clicked);
            sp.Children.Add(stopButton);

            _spinnerWindow.Owner = Application.Current.MainWindow;
            _spinnerWindow.Left = _spinnerWindow.Owner.Left + _spinnerWindow.Width;
            _spinnerWindow.Top = _spinnerWindow.Owner.Top + ((System.Windows.Controls.Panel)_spinnerWindow.Owner.Content).ActualHeight / 2;
            _spinnerWindow.Closed += new EventHandler(_spinnerWindow_Closed);
            IsProgressWindowOff = false;
            _spinnerWindow.Show();
        }

        private bool CreateStatsChartWindow()
        {
            try
            {
                //Ignore if no ROIs:
                if (0 == GetNumROI())
                { return false; }

                if (null == _roiStatsChart)
                {
                    _roiStatsChart = new ROIStatsChartWin(this);
                    _roiStatsChart.Closing += _roiStatsChart_Closing;
                    //Closing is set in ROIStatsChartWin

                    _roiStatsChart.DataContext = this;

                    ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                    XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");

                    if (node != null)
                    {
                        string str = string.Empty;
                        double val = 0;
                        _roiStatsChart.Left = (XmlManager.GetAttribute(node, ApplicationDoc, "left", ref str) && (double.TryParse(str, out val))) ? (int)val : 0;
                        _roiStatsChart.Top = (XmlManager.GetAttribute(node, ApplicationDoc, "top", ref str) && (double.TryParse(str, out val))) ? (int)val : 0;
                        _roiStatsChart.Width = (XmlManager.GetAttribute(node, ApplicationDoc, "width", ref str) && (double.TryParse(str, out val))) ? (int)val : 400;
                        _roiStatsChart.Height = (XmlManager.GetAttribute(node, ApplicationDoc, "height", ref str) && (double.TryParse(str, out val))) ? (int)val : 400;

                        XmlManager.SetAttribute(node, ApplicationDoc, "reset", "0");
                        MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
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
                    _roiStatsChart.ROIChart.SetChartXLabel("Time [sec]");
                    _roiStatsChart.ROIChart.ClearChart();
                    _roiStatsChart.ROIChart.SetLegendGroup(2, chanNames.ToArray(), chanEnable.ToArray());
                    //, "left", "top", "width", "height"
                    List<string> featureNames = new List<string>() { "mean", "stddev", "min", "max" };
                    List<bool> featureEnable = new List<bool>() { true, true, true, true };                  //, false, false, false, false
                    if ((int)BufferType.DFLIM_IMAGE == CaptureSetup.ImageInfo.bufferType)
                    {
                        featureNames.Add("tbar");
                        featureEnable.Add(true);
                    }

                    _roiStatsChart.ROIChart.SetLegendGroup(1, featureNames.ToArray(), featureEnable.ToArray());
                    _roiStatsChart.ROIChart.SetClockAsXAxis(true);
                    _roiStatsChart.ROIChart.SetFifoVisible(true);
                    _roiStatsChart.ROIChart.SetInLoading(true);
                    _roiStatsChart.ROIChart.LoadSettings();
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
            }
            catch (Exception e)
            {
                e.ToString();
            }
            return true;
        }

        private XmlElement CreateWavelengthTag(string name, XmlDocument doc)
        {
            //create a new XML tag for the wavelength settings
            XmlElement newElement = doc.CreateElement("Wavelength");

            XmlAttribute nameAttribute = doc.CreateAttribute("name");
            XmlAttribute expAttribute = doc.CreateAttribute("exposureTimeMS");

            nameAttribute.Value = name;
            expAttribute.Value = "0".ToString();

            newElement.Attributes.Append(nameAttribute);
            newElement.Attributes.Append(expAttribute);

            return newElement;
        }

        private void CreateXmlNode(XmlDocument doc, string nodeName)
        {
            XmlNode node = doc.CreateNode(XmlNodeType.Element, nodeName, null);
            doc.DocumentElement.AppendChild(node);
        }

        void Current_Exit(object sender, ExitEventArgs e)
        {
            PersistROIStatsChartWindowSettings();
            PersistMultiStatsWindowSettings();
            PersistLineProfileWindowSettings();
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

        private void DoEvents()
        {
            DispatcherFrame frame = new DispatcherFrame();
            Dispatcher.CurrentDispatcher.BeginInvoke(DispatcherPriority.Background, new ExitFrameHandler(frm => frm.Continue = false), frame);
            Dispatcher.PushFrame(frame);
        }

        private void FileCopyWithAccessCheck(string src, string dst, bool overwrite)
        {
            if (File.Exists(dst))
            {
                FileInfo fi = new FileInfo(dst);

                while (IsFileLocked(fi))
                {
                    System.Threading.Thread.Sleep(500);
                }
            }
            try
            {
                FileCopyWithExistCheck(src, dst, overwrite);
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "FileCopyWithAccessCheck: " + ex.Message);
                return;
            }
        }

        private void FileCopyWithExistCheck(string src, string dst, bool overwrite)
        {
            if (File.Exists(src))
            {
                File.Copy(src, dst, overwrite);
            }
        }

        private Color[] GetColorAssignments()
        {
            Color[] colorAssignments = new Color[CaptureSetup.MAX_CHANNELS];

            for (int i = 0; i < CaptureSetup.MAX_CHANNELS; i++)
            {
                switch (i)
                {
                    case 0:
                        {
                            if (null != LSMChannelColor0)
                                colorAssignments[i] = ((SolidColorBrush)LSMChannelColor0).Color;
                            break;
                        }
                    case 1:
                        {
                            if (null != LSMChannelColor1)
                                colorAssignments[i] = ((SolidColorBrush)LSMChannelColor1).Color;
                            break;
                        }
                    case 2:
                        {
                            if (null != LSMChannelColor2)
                                colorAssignments[i] = ((SolidColorBrush)LSMChannelColor2).Color;
                            break;
                        }
                    case 3:
                        {
                            if (null != LSMChannelColor3)
                                colorAssignments[i] = ((SolidColorBrush)LSMChannelColor3).Color;
                            break;
                        }
                }
            }

            return colorAssignments;
        }

        void LiveImage_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if ((e.PropertyName == "IsBleaching") && (false == IsBleachStopped)) //check if bleach finished
            {
                IsBleachStopped = true;
                if (null != BleachFinished)
                {
                    BleachFinished(!IsBleaching);
                }
                if (false == IsBleaching)
                {
                    System.Windows.Application.Current.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal,
                        new Action(
                            delegate ()
                            {
                                CloseProgressWindow();
                            }
                            )
                            );
                }
                // stop background workers when bleach is done:
                _bleachWorker.CancelAsync();
                _slmBleachWorker.CancelAsync();
            }
        }

        void LiveImage_Update(bool val)
        {
        }

        void LiveImage_UpdateMenuBarButton(bool status)
        {
            OnPropertyChanged("LiveStartButtonStatus");

            bool btnStatus = status;
            Command command = new Command();
            command.CommandGUID = new Guid("1FC17C8C-960D-4f1d-902A-48C5A2032AAC");

            if (btnStatus)
            {
                command.Message = "Enable Button";
            }
            else
            {
                command.Message = "Disable Button";
            }
            //command published to change the status of the Run a Plate menu button in the Menu Control
            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
        }

        void OnSaveStatChart()
        {
            _statChartFrz = true;
            _roiStatsChart.ROIChart.SaveStatChart();
            _statChartFrz = false;
        }

        private void PersistLineProfileWindowSettings()
        {
            if (null == _lineProfile)
            {
                return;
            }
            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/LineProfileWindow");
            if (ndList.Count > 0)
            {
                XmlNode node = ndList[0];
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "left", ((int)Math.Round(_lineProfile.Left)).ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "top", ((int)Math.Round(_lineProfile.Top)).ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "width", ((int)Math.Round(_lineProfile.Width)).ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "height", ((int)Math.Round(_lineProfile.Height)).ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "display", (((LineProfileActive) ? 1 : 0).ToString()));
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "lineWidth", _lineProfile.LineWidth.ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "Autoscale", _lineProfile.AutoScaleOption.ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "ConversionToUm", _lineProfile.ConversionActiveOption.ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "YMin", _lineProfile.MinimumYVal.ToString());
                XmlManager.SetAttribute(ndList[0], ApplicationDoc, "YMax", _lineProfile.MaximumYVal.ToString());
                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }
            _lineProfile = null;
        }

        private void PersistMultiStatsWindowSettings()
        {
            if (null == _multiROIStats)
            {
                return;
            }
            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIStatsWindow");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (true == XmlManager.GetAttribute(ndList[0], ApplicationDoc, "reset", ref str) && "0" == str)
                {
                    XmlManager.SetAttribute(ndList[0], ApplicationDoc, "left", ((int)Math.Round(_multiROIStats.Left)).ToString());
                    XmlManager.SetAttribute(ndList[0], ApplicationDoc, "top", ((int)Math.Round(_multiROIStats.Top)).ToString());
                    XmlManager.SetAttribute(ndList[0], ApplicationDoc, "width", ((int)Math.Round(_multiROIStats.Width)).ToString());
                    XmlManager.SetAttribute(ndList[0], ApplicationDoc, "height", ((int)Math.Round(_multiROIStats.Height)).ToString());
                    XmlManager.SetAttribute(ndList[0], ApplicationDoc, "display", (((ROIStatsTableActive) ? 1 : 0).ToString()));
                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);

                }
            }
            _multiROIStats = null;
        }

        private string ReadLast3dOutputPath()
        {
            if (Application.Current == null)
            {
                return string.Empty;
            }

            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

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

        private void stopButton_Clicked(object sender, RoutedEventArgs e)
        {
            StopPreview();
        }

        void _bwHardware_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;
            DateTime lastZ = DateTime.Now;
            DateTime lastZ2 = DateTime.Now;
            DateTime lastX = DateTime.Now;
            DateTime lastY = DateTime.Now;
            DateTime lastR = DateTime.Now;
            for (int i = 0; i < 3; i++)
            {
                MVMManager.Instance["ZControlViewModel", "LastZUpdateTime", i] = DateTime.Now;
            }
            MVMManager.Instance["ScanControlViewModel", "LastPMTSafetyUpdate"] = MVMManager.Instance["LaserControlViewModel", "LastBeamStablizerUpdate"] = MVMManager.Instance["XYTileControlViewModel", "LastXUpdate", (object)DateTime.Now] = MVMManager.Instance["XYTileControlViewModel", "LastYUpdate", (object)DateTime.Now] = DateTime.Now;
            DateTime lastP = DateTime.Now;
            DateTime lastS = DateTime.Now;
            TimeSpan ts;
            double tmpPos = 0;

            while (true)
            {
                if ((worker.CancellationPending == true))
                {
                    e.Cancel = true;
                    break;
                }
                else
                {
                    if (false == _enableDeviceQuery)
                    {
                        System.Threading.Thread.Sleep(30);
                        continue;
                    }

                    //update Z, R stage positions from device
                    if ((bool)MVMManager.Instance["ZControlViewModel", "EnableRead", 0, (object)false])
                    {
                        ts = DateTime.Now - lastZ;
                        if (ts.TotalSeconds > .01)
                        {
                            MVMManager.Instance["ZControlViewModel", "UpdatePositions"] = "Z";
                            lastZ = DateTime.Now;
                        }
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("ZPosition");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("ZPositionBar");
                    }

                    if ((bool)MVMManager.Instance["ZControlViewModel", "EnableRead", 1, (object)false])
                    {
                        ts = DateTime.Now - lastZ2;
                        if (ts.TotalSeconds > .01)
                        {
                            MVMManager.Instance["ZControlViewModel", "UpdatePositions"] = "Z2";
                            lastZ2 = DateTime.Now;
                        }
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("Z2Position");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("Z2PositionBar");
                    }

                    if ((bool)MVMManager.Instance["ZControlViewModel", "EnableRead", 2, (object)false])
                    {
                        ts = DateTime.Now - lastR;
                        if (ts.TotalSeconds > .2)
                        {
                            MVMManager.Instance["ZControlViewModel", "UpdatePositions"] = "R";
                            lastR = DateTime.Now;
                        }
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("RPosition");
                    }

                    //update power positions from device
                    ts = DateTime.Now - lastP;

                    if (ts.TotalSeconds > .2)
                    {
                        tmpPos = (double)MVMManager.Instance["PowerControlViewModel", "PowerPositionCurrent", (object)0];
                        tmpPos = (double)MVMManager.Instance["PowerControlViewModel", "PowerPosition2Current", (object)0];

                        lastP = DateTime.Now;
                    }

                    //update XY positions from device
                    if ((bool)MVMManager.Instance["XYTileControlViewModel", "EnableXYRead", (object)false])
                    {
                        ts = DateTime.Now - lastX;

                        if (ts.TotalSeconds > .2)
                        {
                            MVMManager.Instance["XYTileControlViewModel", "UpdatePositions"] = "X";
                            lastX = DateTime.Now;
                        }

                        ts = DateTime.Now - lastY;

                        if (ts.TotalSeconds > .2)
                        {
                            MVMManager.Instance["XYTileControlViewModel", "UpdatePositions"] = "Y";
                            lastY = DateTime.Now;
                        }
                        tmpPos = (double)MVMManager.Instance["XYTileControlViewModel", "XPosition", (object)0.0];
                        tmpPos = (double)MVMManager.Instance["XYTileControlViewModel", "YPosition", (object)0.0];
                    }

                    ts = DateTime.Now - lastS;

                    if (ts.TotalSeconds > 1)// && !_showingPMTSafetyMessage)
                    {
                        //update PMT safety status
                        MVMManager.Instance["ScanControlViewModel", "PMTSafetyStatus"] = GetPMTSafetyStatus();
                        OnPropertyChanged("PMT1Saturations");
                        OnPropertyChanged("PMT2Saturations");
                        OnPropertyChanged("PMT3Saturations");
                        OnPropertyChanged("PMT4Saturations");
                        lastS = DateTime.Now;
                        // Check for collision, if the device returns true on the collision flag, change
                        // the background of the Objective label to flashing red.
                        if (1.0 == (double)MVMManager.Instance["ObjectiveControlViewModel", "CollisionStatus", (object)0.0])
                        {
                            MVMManager.Instance["ObjectiveControlViewModel", "ObjectiveChangerStatus"] = (int)OutOfRangeColors.COLOR_RED; // Red=3
                        }
                        //Update Laser shutters positions
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("LaserShutterPosition");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("LaserShutter2Position");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("Laser1Position");
                        OnPropertyChanged("CollapsedLaserShutterPos");
                        OnPropertyChanged("CollapsedLaserShutter2Pos");
                        OnPropertyChanged("CollapsedLaser1Position");
                        OnPropertyChanged("LightPathGGDisplayOn");
                        OnPropertyChanged("LightPathGGDisplayOff");
                        OnPropertyChanged("LightPathGRDisplayOn");
                        OnPropertyChanged("LightPathGRDisplayOff");
                        OnPropertyChanged("LightPathCamDisplayOn");
                        OnPropertyChanged("LightPathCamDisplayOff");
                        //Update the Chrolis led temperature values
                        if ((bool)MVMManager.Instance["LightEngineControlViewModel", "UpdateTemperatures", (object)false])
                        {
                            ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED1Temperature");
                            ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED2Temperature");
                            ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED3Temperature");
                            ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED4Temperature");
                            ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED5Temperature");
                            ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED6Temperature");
                        }
                    }

                    //update beam stablizer from device
                    MVMManager.Instance["MultiphotonControlViewModel", "BeamStablizerQuery"] = true;

                    //sleep to lessen CPU load
                    System.Threading.Thread.Sleep(2);
                }
            };
        }

        void _bwHardware_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Cancelled && _restartBWHardware)
            {
                _restartBWHardware = false;
                _bwHardware.RunWorkerAsync();
            }
        }

        void _captureSetup_LineProfileChanged(object sender, EventArgs e)
        {
            Application.Current.Dispatcher.Invoke((Action)(() =>
            {
                if (0 == _captureSetup.LineProfileData.channelEnable ||
                    0 == _captureSetup.LineProfileData.profileDataX.Length)
                {
                    if (null != _lineProfile)
                    {
                        this.UnloadWholeLineProfile = true;
                        _lineProfile.Hide();
                        _lineProfile.Close();
                        this.UnloadWholeLineProfile = false;
                    }
                }
                else
                {
                    CreateLineProfileWindow();
                    if (null != _lineProfile)
                    {
                        _lineProfile.SetData(_captureSetup.LineProfileData);
                    }
                }
            }));
        }

        void _captureSetup_ROIStatsChanged(object sender, EventArgs e)
        {
            if (OverlayCanvas.Children.Count == 0) return;
            if (ROIStatsChartActive && _roiStatsChart != null)
            {
                try
                {
                    ROIStatsChart.Show();
                }
                catch
                {
                    return;
                }
            }
            else
            {
                CreateStatsChartWindow();
            }
            if (null != _roiStatsChart)
            {
                if (!_statChartFrz)
                {
                    _roiStatsChart.SetData(_captureSetup.StatsNames, _captureSetup.Stats, true);
                }
                if (ROIStatsTableActive && _multiROIStats != null)
                {
                    try
                    {
                        MultiROIStats.Show();
                    }
                    catch
                    {
                        return;
                    }
                }
                else
                {
                    CreateMultiStatsWindow();
                }
                if (null != _multiROIStats)
                {
                    _multiROIStats.SetData(_captureSetup.StatsNames, _captureSetup.Stats);
                    _multiROIStats.SetArithmeticsData(_roiStatsChart.ROIChart.ArithmeticNames, _roiStatsChart.ROIChart.ArithmeticStats);
                    _multiROIStats.SetFieldSize((int)MVMManager.Instance["AreaControlViewModel", "LSMFieldSize", (object)5]);
                }
            }
        }

        void _deviceReadTimer_Tick(object sender, EventArgs e)
        {
            if (EnableDeviceReading)
            {
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("ZPosition");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("ZPosOutOfBounds");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("Z2Position");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("Z2PosOutOfBounds");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("RPosition");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["PowerControlViewModel", this]).OnPropertyChange("PowerReg");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["PowerControlViewModel", this]).OnPropertyChange("PowerReg2");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["PowerControlViewModel", this]).OnPropertyChange("PowerRegEncoderPosition");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["PowerControlViewModel", this]).OnPropertyChange("PowerReg2EncoderPosition");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("XPosition");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("XPosOutOfBounds");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("YPosition");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("YPosOutOfBounds");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("PMTOn");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ObjectiveControlViewModel", this]).OnPropertyChange("EpiTurretPosName");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ObjectiveControlViewModel", this]).OnPropertyChange("ObjectiveChangerStatus");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ObjectiveControlViewModel", this]).OnPropertyChange("MagComboBoxEnabled");
                OnPropertyChanged("CollapsedZPosition");
                OnPropertyChanged("CollapsedZPosOutOfBounds");
                OnPropertyChanged("CollapsedPower0");
                OnPropertyChanged("CollapsedPower1");
                OnPropertyChanged("CollapsedPower2");
                OnPropertyChanged("CollapsedPower3");
                OnPropertyChanged("CollapsedPowerReg");
                OnPropertyChanged("CollapsedPowerReg2");
                OnPropertyChanged("CollapsedXPosition");
                OnPropertyChanged("CollapsedXPosOutOfBounds");
                OnPropertyChanged("CollapsedYPosition");
                OnPropertyChanged("CollapsedYPosOutOfBounds");
                //The pixel bit depth might change when changing the taps index.
                //The update doesn't happen until the next image after the settings have
                //been pushed to the camera. Use onpropertyChanged at this location to
                //ensure the pixel bit depth (which has a direct effect on the PixelBitShiftValue)
                OnPropertyChanged("PixelBitShiftValue");

                //Only update view when reading from BeamStabilizer is enabled
                if (true == (bool)MVMManager.Instance["MultiphotonControlViewModel", "BeamStabilizerEnableReadData", (object)false])
                {
                    ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("BeamStabilizerBPACentroidX");
                    ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("BeamStabilizerBPACentroidY");
                    ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("BeamStabilizerBPAExposure");
                    ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("BeamStabilizerBPBCentroidX");
                    ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("BeamStabilizerBPBCentroidY");
                    ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("BeamStabilizerBPBExposure");

                    ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("BeamStabilizerPiezo1Pos");
                    ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("BeamStabilizerPiezo2Pos");
                    ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("BeamStabilizerPiezo3Pos");
                    ((ThorSharedTypes.IMVM)MVMManager.Instance["MultiphotonControlViewModel", this]).OnPropertyChange("BeamStabilizerPiezo4Pos");
                }

                ((ThorSharedTypes.IMVM)MVMManager.Instance["EpiTurretControlViewModel", this]).OnPropertyChange("EpiPosition1");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["EpiTurretControlViewModel", this]).OnPropertyChange("EpiPosition2");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["EpiTurretControlViewModel", this]).OnPropertyChange("EpiPosition3");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["EpiTurretControlViewModel", this]).OnPropertyChange("EpiPosition4");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["EpiTurretControlViewModel", this]).OnPropertyChange("EpiPosition5");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["EpiTurretControlViewModel", this]).OnPropertyChange("EpiPosition6");
                OnPropertyChanged("InvertedLpLeftDisplay");
                OnPropertyChanged("InvertedLpCenterDisplay");
                OnPropertyChanged("InvertedLpRightDisplay");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("EnableDisableLEDs");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("MasterBrightness");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED1Power");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED2Power");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED3Power");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED4Power");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED5Power");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED6Power");
            }

            //Update any stats
            RequestROIData();
        }

        void _lineProfile_Closed(object sender, EventArgs e)
        {
            if (!this.UnloadWholeLineProfile)
            {
                LineProfileActive = false;
            }
            PersistLineProfileWindowSettings();
        }

        void _lineProfile_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (!this.UnloadWholeLineProfile)
            {
                LineProfileActive = false;
            }
            PersistLineProfileWindowSettings();
            e.Cancel = true;
        }

        void _lineProfile_LineWidthChange(int lineWidth)
        {
            CaptureSetup.SetLineProfileLineWidth(lineWidth);
        }

        void _multiROIStats_Closed(object sender, EventArgs e)
        {
            if (!this.UnloadWholeStats)
            {
                ROIStatsTableActive = false;
            }
            PersistMultiStatsWindowSettings();
        }

        void _multiROIStats_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (!this.UnloadWholeStats)
            {
                ROIStatsTableActive = false;
            }
            PersistMultiStatsWindowSettings();
            e.Cancel = true;
        }

        void _overlayManager_MaskChangedEvent()
        {
            UpdateStats();

            ////Check Bleach ROIs when bleach border is visible.
            ////Dropped: causing UI to be unresponsive
            //if (BleachBorderEnabled)
            //{
            //    BleachROICheck();
            //}
        }

        void _overlayManager_SavingROIStats()
        {
            this.SaveNow();
        }

        void _pmtSafetyTimer_Tick(object sender, EventArgs e)
        {
            ((ThorSharedTypes.IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("PMTSafetyStatus");

            //The safety flag for the hardware has tripped
            //Present a message to the user and disable the PMTs
            if ((false == (bool)MVMManager.Instance["ScanControlViewModel", "PMTSafetyStatus", (object)false]) && (_showingPMTSafetyMessage == false))
            {
                _showingPMTSafetyMessage = true;
                MessageBox.Show("The PMT(s) safety has tripped. To prevent damage to the PMT(s) please reduce the light being sent to the detectors.");
                _showingPMTSafetyMessage = false;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 0]).Value = 0;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 1]).Value = 0;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 2]).Value = 0;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 3]).Value = 0;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 0]).Value = 0;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 1]).Value = 0;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 2]).Value = 0;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 3]).Value = 0;
                // due to the hardware/firmware reason, pmt2100 trips cannot be cleared by one time disable.
                // by multiple trials, this way (disable-enable-disable) could clear the trip firmly
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 0]).Value = 1;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 1]).Value = 1;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 2]).Value = 1;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 3]).Value = 1;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 0]).Value = 0;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 1]).Value = 0;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 2]).Value = 0;
                ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable", 3]).Value = 0;
                int val = (int)MVMManager.Instance["ScanControlViewModel", "PMTTripCount", (object)0];
                MVMManager.Instance["ScanControlViewModel", "PMTTripCount"] = val + 1;

                //update hardware settings:
                if (HardwareDoc != null)
                {
                    XmlNodeList ndList = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS].SelectNodes("/HardwareSettings/PMT");

                    if (ndList.Count > 0)
                    {
                        XmlManager.SetAttribute(ndList[0], HardwareDoc, "tripCount", ((int)MVMManager.Instance["ScanControlViewModel", "PMTTripCount", (object)0]).ToString());
                        MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
                    }
                }
            }
        }

        void _roiStatsChart_Closed(object sender, EventArgs e)
        {
            if (!this.UnloadWholeROIStatsChart)
            {
                ROIStatsChartActive = false;
            }
            PersistROIStatsChartWindowSettings();
        }

        void _roiStatsChart_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (!this.UnloadWholeROIStatsChart)
            {
                ROIStatsChartActive = false;
            }
            PersistROIStatsChartWindowSettings();
            e.Cancel = true;
        }

        void _spinnerWindow_Closed(object sender, EventArgs e)
        {
            StopPreview();
            IsProgressWindowOff = true;
            _spinnerWindow = null;
        }

        #endregion Methods
    }
}