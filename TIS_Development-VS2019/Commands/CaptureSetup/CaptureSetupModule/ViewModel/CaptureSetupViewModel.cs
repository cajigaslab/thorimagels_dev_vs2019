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
    using System.Windows.Controls.Primitives;
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

        private readonly CaptureSetup _captureSetup;

        private BackgroundWorker _bleachWorker;
        private BackgroundWorker _bw;
        private BackgroundWorker _bwHardware = null;
        private Double _captureSetupControlPanelWidth = 0;
        private int _captureSetupModalitySwap = 0;
        private ICommand _closeProgressWindowCommand;
        private Visibility _collapsedPockels0Visibility = Visibility.Collapsed;
        private Visibility _collapsedPockels1Visibility = Visibility.Collapsed;
        private Visibility _collapsedPockels2Visibility = Visibility.Collapsed;
        private Visibility _collapsedPockels3Visibility = Visibility.Collapsed;
        private ObservableCollection<StringPC> _collapsedPowerControlName;
        private Visibility _collapsedPowerReg2Visibility = Visibility.Collapsed;
        private Visibility _collapsedPowerRegVisibility = Visibility.Collapsed;
        private Visibility _collapsedShutter2Visibility = Visibility.Collapsed;
        private IUnityContainer _container;
        private ICommand _displayROIStatsOptionsCommand;
        private bool _enableDeviceQuery = true;
        private IEventAggregator _eventAggregator;
        private XmlDocument _hardwareDoc;
        private bool _ignoreLineProfileGeneration = false;
        private XmlDocument _imageProcessDoc;
        private Visibility _imagerViewVis = Visibility.Visible;
        private bool _isHandlerConnected = false;
        private bool _isProgressWindowOff = true;
        private LineProfile _lineProfile = null;
        private bool _lineProfileActive;
        private SpinnerProgress.SpinnerProgressControl _modalitySpinner;
        private Window _modalitySpinnerWindow = null;
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
        private RelayCommand _saveStatChartCommand;
        private bool _showingPMTSafetyMessage;
        private BackgroundWorker _slmBleachWorker;
        private SpinnerProgress.SpinnerProgressControl _spinner;
        private BackgroundWorker _spinnerBackgroundWorker;
        private Window _spinnerWindow = null;
        private bool _spinnerWindowShowing = false;
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

            //_SpotScanIsEnabled = false;
            _bROIByteArray = null;

            _OutputExperiment = "Untitled001";

            CaptureSetupViewModel vm = this;

            this._captureSetup.UpdateMenuBarButton += new Action<bool>(LiveImage_UpdateMenuBarButton);

            var startLiveImageEvent = _eventAggregator.GetEvent<StartLiveImageEvent>();
            startLiveImageEvent.Subscribe(LiveCapture);

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

            _pmtSafetyTimer = new DispatcherTimer();
            _pmtSafetyTimer.Interval = TimeSpan.FromMilliseconds(1000);

            this._captureSetup.PropertyChanged += new PropertyChangedEventHandler(LiveImage_PropertyChanged);

            OverlayManagerClass.Instance.InitOverlayManagerClass(512, 512, new PixelSizeUM(1.0, 1.0), true);

            _roiStatsChartActive = false;
            _roiStatsTableActive = false;
            _lineProfileActive = false;

            if (Application.Current != null)
            {
                Application.Current.Exit += Current_Exit;
            }

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
            _statChartFrz = false;

            //TODO: the collect MVM line should probably be higher level, before capture setup loads.
            //reload mvm manager settings files
            MVMManager.CollectMVM();
            MVMManager.Instance.LoadSettings();

            MVMManager.Instance["QuickTemplatesControlViewModel", "QtConstructor"] = true;
            //Pass Capture Setup's event aggregator to RemoteIPC MVM so it can communicate with IPCModules
            MVMManager.Instance["RemoteIPCControlViewModel", "EventAggregator"] = _eventAggregator;

            //Create a background worker that will run a spinner when swapping modalities from Capture Setup
            _spinnerBackgroundWorker = new BackgroundWorker();
            _spinnerBackgroundWorker.WorkerSupportsCancellation = true;
            _spinnerBackgroundWorker.DoWork += spinnerBackgroundWorker_DoWork;
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

        public ICommand BrowseForReferenceImageCommand
        {
            get;
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

        public Visibility CameraLightPathVisibility
        {
            get
            {
                return (Visibility)MVMManager.Instance["LightPathControlViewModel", "CameraLightPathVisibility", (object)Visibility.Collapsed];
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

        //Set in MenuLSViewModel.cs and used in ResourceManagerCS to create the spinner when swapping modalities from Capture Setup
        public int CaptureSetupModalitySwap
        {
            get
            {
                return _captureSetupModalitySwap;
            }
            set
            {
                _captureSetupModalitySwap = value;
                OnPropertyChanged("CaptureSetupModalitySwap");
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
                    ResourceManagerCS.DeleteDirectory(value);

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

        public ICommand CloseProgressWindowCommand
        {
            get => _closeProgressWindowCommand ?? (_closeProgressWindowCommand = new RelayCommand(() => CloseProgressWindow()));
        }

        public double CollapsedConverRatio
        {
            get
            {
                return (double)MVMManager.Instance["XYTileControlViewModel", "ConvertRatio", (object)0.0];
            }
        }

        public string CollapsedEnableLaser1Content
        {
            get
            {
                return (string)MVMManager.Instance["MultiLaserControlViewModel", "EnableLaser1Content", (object)string.Empty];
            }
        }

        public string CollapsedEnableLaser2Content
        {
            get
            {
                return (string)MVMManager.Instance["MultiLaserControlViewModel", "EnableLaser2Content", (object)string.Empty];
            }
        }

        public string CollapsedEnableLaser3Content
        {
            get
            {
                return (string)MVMManager.Instance["MultiLaserControlViewModel", "EnableLaser3Content", (object)string.Empty];
            }
        }

        public string CollapsedEnableLaser4Content
        {
            get
            {
                return (string)MVMManager.Instance["MultiLaserControlViewModel", "EnableLaser4Content", (object)string.Empty];
            }
        }

        public double CollapsedExposureTime
        {
            get
            {
                return (double)MVMManager.Instance["CameraControlViewModel", "ExposureTimeCam", (object)0.0];
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
                int _laser1enable = (int)MVMManager.Instance["MultiLaserControlViewModel", "CollapsedLaser1Enable", (object)0];
                return (1 == _laser1enable) ? Visibility.Visible : Visibility.Collapsed;
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
                return (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser1Power", (object)0.0];
            }
        }

        //Gets the Wavelengths for lasers 1-4 to use as Label within collapsed control panel
        public string CollapsedLaser1Wavelength
        {
            get
            {
                int wavelength1 = (int)MVMManager.Instance["MultiLaserControlViewModel", "CollapsedLaser1Wavelength", (object)0];
                return wavelength1.ToString() + " nm Laser: ";
            }
        }

        public Visibility CollapsedLaser2Enable
        {
            get
            {
                int _laser2enable = (int)MVMManager.Instance["MultiLaserControlViewModel", "CollapsedLaser2Enable", (object)0];
                return (1 == _laser2enable) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public double CollapsedLaser2Power
        {
            get
            {
                return (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser2Power", (object)0.0];
            }
        }

        public string CollapsedLaser2Wavelength
        {
            get
            {
                int wavelength2 = (int)MVMManager.Instance["MultiLaserControlViewModel", "CollapsedLaser2Wavelength", (object)0];
                return wavelength2.ToString() + " nm Laser: ";
            }
        }

        public Visibility CollapsedLaser3Enable
        {
            get
            {
                int _laser3enable = (int)MVMManager.Instance["MultiLaserControlViewModel", "CollapsedLaser3Enable", (object)0];
                return (1 == _laser3enable) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public double CollapsedLaser3Power
        {
            get
            {
                return (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser3Power", (object)0.0];
            }
        }

        public string CollapsedLaser3Wavelength
        {
            get
            {
                int wavelength3 = (int)MVMManager.Instance["MultiLaserControlViewModel", "CollapsedLaser3Wavelength", (object)0];
                return wavelength3.ToString() + " nm Laser: ";
            }
        }

        public Visibility CollapsedLaser4Enable
        {
            get
            {
                int _laser4enable = (int)MVMManager.Instance["MultiLaserControlViewModel", "CollapsedLaser4Enable", (object)0];
                return (1 == _laser4enable) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public double CollapsedLaser4Power
        {
            get
            {
                return (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser4Power", (object)0.0];
            }
        }

        public string CollapsedLaser4Wavelength
        {
            get
            {
                int wavelength4 = (int)MVMManager.Instance["MultiLaserControlViewModel", "CollapsedLaser4Wavelength", (object)0];
                return wavelength4.ToString() + " nm Laser: ";
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

        public Visibility CollapsedOriginalLaserVisibility
        {
            get
            {
                return (Visibility)MVMManager.Instance["MultiLaserControlViewModel", "OriginalLaserVisibility", (object)Visibility.Collapsed];
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

        public Visibility CollapsedTopticaVisibility
        {
            get
            {
                return (Visibility)MVMManager.Instance["MultiLaserControlViewModel", "TopticaVisibility", (object)Visibility.Collapsed];
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
                DrawLineForLineScanEvent();
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
                    return (double)MVMManager.Instance["AreaControlViewModel", "LSMFieldSizeXUM", (object)1.0];
                }
                else
                {
                    return (double)MVMManager.Instance["CameraControlViewModel", "CamPixelSizeUM", (object)1.0] * ((int)MVMManager.Instance["CameraControlViewModel", "Right", (object)1] - (int)MVMManager.Instance["CameraControlViewModel", "Left", (object)0]);
                }
            }
        }

        public Visibility GGLightPathVisibility
        {
            get
            {
                return (Visibility)MVMManager.Instance["LightPathControlViewModel", "GGLightPathVisibility", (object)Visibility.Collapsed];
            }
        }

        public Visibility GRLightPathVisibility
        {
            get
            {
                return (Visibility)MVMManager.Instance["LightPathControlViewModel", "GRLightPathVisibility", (object)Visibility.Collapsed];
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
                MVMManager.Instance["AreaControlViewModel", "EnablePixelDensityChange"] = value;
                OnPropertyChanged("IsProgressWindowOff");
                OnPropertyChanged("SLMBleachNowEnabled");
                OnPropertyChanged("BleachNowEnable");
                OnPropertyChanged("SLMPanelAvailable");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("PreviewButtonEnabled");
            }
        }

        public int LightPathCamEnable
        {
            get
            {
                return (int)MVMManager.Instance["LightPathControlViewModel", "LightPathCamEnable", (object)0];
            }
        }

        public int LightPathGGEnable
        {
            get
            {
                return (int)MVMManager.Instance["LightPathControlViewModel", "LightPathGGEnable", (object)0];
            }
        }

        public int LightPathGREnable
        {
            get
            {
                return (int)MVMManager.Instance["LightPathControlViewModel", "LightPathGREnable", (object)0];
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

        //Used in ResourceManagerCS to set when the spinner should be created.
        //Used in Capture Setup MasterView.xml.cs to set when spinner should be closed
        public bool ModalitySpinnerWindowShowing
        {
            get
            {
                return _spinnerWindowShowing;
            }
            set
            {
                _spinnerWindowShowing = value;
                OnPropertyChanged("ModalitySpinnerWindowShowing");
                if (_spinnerWindowShowing == true)
                {
                    if (!_spinnerBackgroundWorker.IsBusy)
                    {
                        _spinnerBackgroundWorker.RunWorkerAsync();
                    }
                    else
                    {
                        CloseSpinnerWindow();
                        _spinnerWindowShowing = false;
                    }

                }
                else
                {
                    if (_spinnerBackgroundWorker.IsBusy)
                    {
                        _spinnerBackgroundWorker.CancelAsync();
                    }
                    CloseSpinnerWindow();
                }
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

        public XmlDocument RegistrationDoc
        {
            get
            {
                return _captureSetup.RegistrationDoc;
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

        public static void RemoveApplicationAttribute(string nodeName, string attrName)
        {
            string strTmp = string.Empty;
            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList ndList = appSettings.SelectNodes(nodeName);
            if (null != ndList)
            {
                if (XmlManager.GetAttribute(ndList[0], appSettings, attrName, ref strTmp))
                {
                    XmlManager.RemoveAttribute(ndList[0], attrName);
                    MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
                }
            }
        }

        [DllImport(".\\CaptureSetup.dll", EntryPoint = "SetupCaptureBuffers")]
        public static extern int SetupCaptureBuffers();

        public void BrowseForReferenceImage()
        {
        }

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

        public void CloseSpinnerWindow()
        {
            if (null != _modalitySpinnerWindow)
            {
                _modalitySpinnerWindow.Dispatcher.Invoke(() =>
                {
                    _modalitySpinnerWindow.Close();
                });
            }
        }

        public void ConnectHandlers()
        {
            //keep handlers be connected once and once only before release
            if (_isHandlerConnected)
                return;

            _bw.DoWork += new DoWorkEventHandler(bw_DoWork);
            _bw.ProgressChanged += new ProgressChangedEventHandler(bw_ProgressChanged);
            _bw.RunWorkerCompleted += new RunWorkerCompletedEventHandler(bw_RunWorkerCompleted);
            _bleachWorker.DoWork += new DoWorkEventHandler(bleachWorker_DoWork);
            _slmBleachWorker.DoWork += new DoWorkEventHandler(slmBleachWorker_DoWork);
            _slmBleachWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(slmBleachWorker_RunWorkerCompleted);

            OverlayManagerClass.Instance.ClearedStatsROIsEvent += Instance_ClearedStatsROIsEvent;
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
            _turretPosHandler = MVMManager.Instance["ObjectiveControlViewModel"]?.AddDynamicEventHandler("TurretPositionChangedEvent", nameof(OnTurretPositionChanged), this);
            _registrationHandler = MVMManager.Instance["AreaControlViewModel"]?.AddDynamicEventHandler("RegistrationChangedEvent", nameof(OnRegistrationChanged), this);

            this._captureSetup.ConnectHandlers();

            _isHandlerConnected = true;
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

                ResourceManagerCS.SafeCreateDirectory(expTemplatesFldr + "\\" + templateName);

                OverlayManagerClass.Instance.SaveROIs(pathTemplateROIsXAML);

                FileCopyWithExistCheck(expActiveXML, TemplateSettingFile, true);

                FileCopyWithExistCheck(expROIMask, pathTemplateROIMask, true);

                FileCopyWithExistCheck(expBleachingROIsXAML, pathTemplateBleachingROIsXAML, true);

                FileCopyWithExistCheck(expBleachingWaveFormFile, pathTemplateBleachingWaveFormH5, true);

                //copy SLM:
                if (Directory.Exists(SLMWaveformFolder[0]))
                {
                    ResourceManagerCS.SafeCreateDirectory(pathTemplateSLMWaveFormFolder);
                    //create directories:
                    foreach (string dirPath in Directory.GetDirectories(SLMWaveformFolder[0], "*", SearchOption.AllDirectories))
                        ResourceManagerCS.SafeCreateDirectory(dirPath.Replace(SLMWaveformFolder[0], pathTemplateSLMWaveFormFolder));

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

        public bool DisplayROI(string pathName = "")
        {
            pathName = (string.Empty == pathName) ? (ResourceManagerCS.GetCaptureTemplatePathString() + "ActiveROIs.xaml") : pathName;

            if (!File.Exists(pathName))
                return false;

            ROIUpdateRequested(pathName);
            return true;
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
            if (dropInfo.DragInfo.SourceCollection.Equals(SLMBleachWaveParams) &&
               dropInfo.TargetCollection.Equals(SLMBleachWaveParams))
            {
                dropInfo.Effects = DragDropEffects.Move;
            }
        }

        void IDropTarget.Drop(IDropInfo dropInfo)
        {
            try
            {
                if (dropInfo.DragInfo.SourceCollection.Equals(SLMBleachWaveParams) &&
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
            //Load ROI's before other settings so overlay manager updated
            bool isReticleChecked = false;
            bool isScaleButtonChecked = false;

            OverlayManagerClass.Instance.InitSelectROI();
            OverlayManagerClass.Instance.PersistLoadROIs(ref isReticleChecked, ref isScaleButtonChecked);
            MVMManager.Instance["ImageViewCaptureSetupVM", "IsReticleChecked"] = isReticleChecked;
            MVMManager.Instance["ImageViewCaptureSetupVM", "IsScaleButtonChecked"] = isScaleButtonChecked;

            string str = string.Empty, str1 = string.Empty;
            double dTmp = 0.0, dTmp1 = 0.0;
            int iTmp = 0;
            Int64 i64Tmp = 0;
            UInt32 uiTmp = 0;

            //determine the bleach type, GG or WideField
            SLMPhaseDirect = IsStimulator;

            //***************************************************//
            //  Reload shared settings based on UI selections    //
            //***************************************************//
            if (!BleachExpandHeader.Contains("OTM"))
            {
                //OTM shared some properties with AreaControl, reload SLM if not selected
                ((ThorSharedTypes.IMVM)MVMManager.Instance["AreaControlViewModel"]).LoadXMLSettings();
            }

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
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "powerShiftUS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    PowerShiftUS = dTmp;
                }
            }

            //SLM Patterns:
            SLMControl.ZDefocusPlotWin.LoadZCalibration();  //reload Z Calibration [X, Y] = [ZPositionUM, ZDefocusUM]
            SLMBleachWaveParams.Clear();
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
                    if (XmlManager.GetAttribute(ndList[i], ExperimentDoc, "phaseType", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                    {
                        sparam.PhaseType = iTmp;
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
            bool skipSafeBlank = _slmPanelInUse && !_slmBuildOnce;
            EpochSequence.Clear();
            if (0 == SLMBleachWaveParams.Count)
            {
                EditSLMParam("SLM_PATTERN_DELETEALL");  //clear ROIs if no SLM params
            }
            else
            {
                //populate sequences only when there are params
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
            }
            //SLM: general specs
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
                    skipSafeBlank |= SLMSequenceOn = (1 == iTmp);
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "randomEpochs", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    SLMRandomEpochs = (1 == iTmp);
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "holoGen3D", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    SLM3D = (1 == iTmp);
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "refractiveIndex", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    RefractiveIndex = dTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "sequenceEpochDelay", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    SLMSequenceEpochDelay = dTmp;
                }
                //update epochCount will trigger waveform rebuild
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "epochCount", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    skipSafeBlank |= (EpochCount != iTmp);
                    EpochCount = iTmp;
                }
                //update calibrated z offset will trigger rebuild all
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "calibZoffsetUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    if (DefocusSavedUM != dTmp)
                    {
                        DefocusSavedUM = DefocusUM = dTmp;
                        if (SLMBleachWaveParams.Count < PatternImportLimit)
                        {
                            skipSafeBlank = true;
                            EditSLMParam("SLM_REBUILDALL");
                        }
                    }
                }
                //keep safe blank if not rebuild (all)
                if (!skipSafeBlank)
                    OnTurretPositionChanged(null, null);
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
            this._captureSetup.ReleaseHandlers();
            _bw.DoWork -= new DoWorkEventHandler(bw_DoWork);
            _bw.ProgressChanged -= new ProgressChangedEventHandler(bw_ProgressChanged);
            _bw.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(bw_RunWorkerCompleted);
            _bleachWorker.DoWork -= new DoWorkEventHandler(bleachWorker_DoWork);
            _slmBleachWorker.DoWork -= new DoWorkEventHandler(slmBleachWorker_DoWork);
            _slmBleachWorker.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(slmBleachWorker_RunWorkerCompleted);

            OverlayManagerClass.Instance.ClearedStatsROIsEvent -= Instance_ClearedStatsROIsEvent;
            _pmtSafetyTimer.Stop();
            _pmtSafetyTimer.Tick -= new EventHandler(_pmtSafetyTimer_Tick);
            if (_bwHardware != null)
                _bwHardware.CancelAsync();

            OverlayManagerClass.Instance.MaskChangedEvent -= _overlayManager_MaskChangedEvent;
            MVMManager.Instance["ObjectiveControlViewModel"]?.RemoveDynamicEventHandler("TurretPositionChangedEvent", _turretPosHandler);
            MVMManager.Instance["AreaControlViewModel"]?.RemoveDynamicEventHandler("RegistrationChangedEvent", _registrationHandler);
            //close any floating panels or windows
            CloseFloatingWindows();
            //allow handlers to be connected later
            _isHandlerConnected = false;
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
                    ResourceManagerCS.DeleteFile(pathActiveBleachingROIsXAML);
                }
                if (File.Exists(expBleachingWaveFormFile))
                {
                    FileCopyWithAccessCheck(expBleachingWaveFormFile, pathActiveBleachingWaveformFile, true);
                    BleachNowEnable = true;
                }
                else
                {
                    ResourceManagerCS.DeleteFile(pathActiveBleachingWaveformFile);
                    BleachNowEnable = false;
                }
                //SLM files:
                ClearSLMFiles(SLMWaveformFolder[0], "raw");
                ClearSLMFiles(SLMWaveformFolder[0], "bmp");
                if (Directory.Exists(expSLMBleachFolder))
                {
                    ResourceManagerCS.SafeCreateDirectory(SLMWaveformFolder[0]);
                    //create directories:
                    foreach (string dirPath in Directory.GetDirectories(expSLMBleachFolder, "*", SearchOption.AllDirectories))
                        ResourceManagerCS.SafeCreateDirectory(dirPath.Replace(expSLMBleachFolder, SLMWaveformFolder[0]));

                    //copy all the files & replaces any files with the same name
                    foreach (string newPath in Directory.GetFiles(expSLMBleachFolder, "*.*", SearchOption.AllDirectories))
                        File.Copy(newPath, newPath.Replace(expSLMBleachFolder, SLMWaveformFolder[0]), true);
                }
                //update with the new experiment document
                MVMManager.Instance.LoadSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);

                ActiveSettingsReplaced();
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

        public void spinnerBackgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            if (_spinnerWindowShowing == true)
            {
                System.Threading.Thread thread = new System.Threading.Thread(() =>
                {
                    CreateSpinnerWindow();
                    System.Windows.Threading.Dispatcher.Run();
                });
                thread.SetApartmentState(System.Threading.ApartmentState.STA);
                thread.IsBackground = true;
                thread.Start();
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

        void ActiveSettingsReplaced()
        {
            bool recticleChecked = false;
            bool scaleActive = false;
            OverlayManagerClass.Instance.PersistLoadROIs(ref recticleChecked, ref scaleActive);
            MVMManager.Instance["ImageViewCaptureSetupVM", "IsReticleChecked"] = recticleChecked;
            MVMManager.Instance["ImageViewCaptureSetupVM", "IsScaleButtonChecked"] = scaleActive;
            MVMManager.Instance["ImageViewCaptureSetupVM", "ROIDrawingToolsSelectedIndex"] = 0;
            OverlayManagerClass.Instance.InitSelectROI();
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
                    // report progress.
                    System.Threading.Thread.Sleep(200);
                    worker.ReportProgress(0);
                }
            };
        }

        private void bw_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            if (CaptureSetup.ImageDataUpdated)
            {
                ((IMVM)MVMManager.Instance["ObjectiveControlViewModel", this]).OnPropertyChange("FramesPerSecondText");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("FramesPerSecondAverage");

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

        private void ClearZ2StageLockedSettings()
        {
            MVMManager.Instance["ZControlViewModel", "Z2StageLock", (object)0.0] = false;
        }

        private void CreateLineProfileWindow()
        {
            if (null == _lineProfile && true == _lineProfileActive && false == IgnoreLineProfileGeneration)
            {
                if (GetNumROI() <= 0)
                {
                    return;
                }

                _lineProfile = new LineProfileWindow.LineProfile((Color[])MVMManager.Instance["ImageViewCaptureSetupVM", "DefaultChannelColors"], CaptureSetup.MAX_CHANNELS);
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
            _spinnerWindow.Top = _spinnerWindow.Owner.Top + ((System.Windows.Controls.Panel)_spinnerWindow.Owner.Content).ActualHeight / 4;
            _spinnerWindow.Closed += new EventHandler(_spinnerWindow_Closed);
            IsProgressWindowOff = false;
            _spinnerWindow.Show();
        }

        private void CreateSpinnerWindow()
        {
            if (null != _modalitySpinnerWindow)
                return;
            //create a popup modal dialog that blocks user clicks while capturing
            _modalitySpinnerWindow = new Window();
            _modalitySpinnerWindow.Title = "Spinner";
            _modalitySpinnerWindow.ResizeMode = ResizeMode.NoResize;
            _modalitySpinnerWindow.Width = 162;
            _modalitySpinnerWindow.Height = 162;
            _modalitySpinnerWindow.WindowStyle = WindowStyle.None;
            _modalitySpinnerWindow.Background = Brushes.Transparent;
            _modalitySpinnerWindow.AllowsTransparency = true;
            System.Windows.Controls.Border border = new System.Windows.Controls.Border();
            border.BorderThickness = new Thickness(2);
            _modalitySpinnerWindow.Content = border;

            System.Windows.Controls.StackPanel sp = new System.Windows.Controls.StackPanel();
            border.Child = sp;

            _modalitySpinner = new SpinnerProgress.SpinnerProgressControl();
            _modalitySpinner.Margin = new Thickness(30, 5, 30, 5);
            System.Windows.Controls.Border borderSpinner = new System.Windows.Controls.Border();
            borderSpinner.Child = _modalitySpinner;
            sp.Children.Add(borderSpinner);

            _modalitySpinnerWindow.WindowStartupLocation = WindowStartupLocation.CenterScreen;
            _modalitySpinnerWindow.Closed += new EventHandler(_modalitySpinnerWindow_Closed);
            _modalitySpinnerWindow.Show();
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

        void Current_Exit(object sender, ExitEventArgs e)
        {
            PersistROIStatsChartWindowSettings();
            PersistMultiStatsWindowSettings();
            PersistLineProfileWindowSettings();
            ClearZ2StageLockedSettings();
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

        void DrawLineForLineScanEvent()
        {
            bool lineProfileActive = LineProfileActive;
            OverlayManagerClass.Instance.ClearAllObjects();
            MVMManager.Instance["ImageViewCaptureSetupVM", "IsReticleChecked"] = false;
            MVMManager.Instance["ImageViewCaptureSetupVM", "IsScaleButtonChecked"] = false;
            MVMManager.Instance["ImageViewCaptureSetupVM", "ROIDrawingToolsSelectedIndex"] = 0;
            OverlayManagerClass.Instance.InitSelectROI();
            if (null != LineProfile && lineProfileActive)
            {
                LineProfile.Show();
            }
            //The second point must be less than pixel x, because otherwise an extra pixel would be included
            //since the canvas is zero based, and pixelX and pixelY are one based
            OverlayManagerClass.Instance.CreateROIShape(typeof(Line), new Point(0, 0.5), new Point((int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)512] - 0.01, 0.5));
            OverlayManagerClass.Instance.InitSelectROI();
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

        private void Instance_ClearedStatsROIsEvent()
        {
            if (null != MultiROIStats)
            {
                if (ROIStatsTableActive)
                {
                    MultiROIStats.Hide();
                }
                else
                {
                    MultiROIStats.Close();
                }
            }

            if (null != ROIStatsChart)
            {
                if (ROIStatsChartActive)
                {
                    ROIStatsChart.Hide();
                }
                else
                {
                    ROIStatsChart.Close();
                    //In CaptureSetup _roiStatsChart is not closed (only hidden)
                    //force the persistance of the window settings
                    PersistROIStatsChartWindowSettings();
                }
            }

            if (null != LineProfile)
            {
                if (LineProfileActive)
                {
                    LineProfile.Hide();
                }
                else
                {
                    LineProfile.Close();
                }
            }
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
            DateTime lastBulk = DateTime.Now;
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
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["AutoFocusControlViewModel", this]).OnPropertyChange("CurrentZPosition");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("ZPosition");
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
                        tmpPos = (double)MVMManager.Instance["PowerControlViewModel", "PowerPositionCurrent", (object)0.0];
                        tmpPos = (double)MVMManager.Instance["PowerControlViewModel", "PowerPosition2Current", (object)0.0];

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

                    if (ts.TotalSeconds > .2)
                    {
                        //Update Autofocus control values
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["AutoFocusControlViewModel", this]).OnPropertyChange("CurrentZPosition");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["AutoFocusControlViewModel", this]).OnPropertyChange("AbsoluteStartPosition");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["AutoFocusControlViewModel", this]).OnPropertyChange("AbsoluteStopPosition");
                    }

                    ts = DateTime.Now - lastS;

                    if (ts.TotalSeconds > 1)// && !_showingPMTSafetyMessage)
                    {
                        //update PMT safety status
                        MVMManager.Instance["ScanControlViewModel", "PMTSafetyStatus"] = GetPMTSafetyStatus();

                        //TODO:IV remove from ImageView, put into PMT control
                        ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("PMT1Saturations");
                        ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("PMT2Saturations");
                        ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("PMT3Saturations");
                        ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("PMT4Saturations");

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
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightPathControlViewModel", this]).OnPropertyChange("LightPathGGDisplayOn");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightPathControlViewModel", this]).OnPropertyChange("LightPathGGDisplayOff");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightPathControlViewModel", this]).OnPropertyChange("LightPathGRDisplayOn");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightPathControlViewModel", this]).OnPropertyChange("LightPathGRDisplayOff");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightPathControlViewModel", this]).OnPropertyChange("LightPathCamDisplayOn");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightPathControlViewModel", this]).OnPropertyChange("LightPathCamDisplayOff");
                        if ((bool)MVMManager.Instance["LightPathControlViewModel", "IsNDDAvailable", (object)false])
                        {
                            OnPropertyChanged("DisplayOnNDD");
                            OnPropertyChanged("DisplayOffNDD");
                        }
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
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("ExternalMode1");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("ExternalMode2");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("ExternalMode3");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("ExternalMode4");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("ExternalMode5");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("ExternalMode6");

                    }

                    //update beam stablizer from device
                    MVMManager.Instance["MultiphotonControlViewModel", "BeamStablizerQuery"] = true;

                    ts = DateTime.Now - lastBulk;

                    if (ts.TotalMilliseconds > 100)
                    {
                        lastBulk = DateTime.Now;
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("ZPosOutOfBounds");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("Z2PosOutOfBounds");
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
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightPathControlViewModel", this]).OnPropertyChange("InvertedLpLeftDisplay");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightPathControlViewModel", this]).OnPropertyChange("InvertedLpCenterDisplay");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightPathControlViewModel", this]).OnPropertyChange("InvertedLpRightDisplay");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("EnableDisableLEDs");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("MasterBrightness");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED1Power");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED2Power");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED3Power");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED4Power");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED5Power");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LightEngineControlViewModel", this]).OnPropertyChange("LED6Power");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LampControlViewModel", this]).OnPropertyChange("LampON");
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["LampControlViewModel", this]).OnPropertyChange("LampPosition");

                        MVMManager.Instance["ScanControlViewModel", "UpdateCRSFrequency"] = 1;

                        //Update any stats
                        RequestROIData();
                    }

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
            //ensure this logic runs in the GUI thread and with a Render priority
            System.Windows.Application.Current.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Render,
                new Action(
                    delegate ()
                    {
                        if (OverlayManagerClass.Instance.StatsROICount <= 0) return;
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
                    }));
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

        void _modalitySpinnerWindow_Closed(object sender, EventArgs e)
        {
            _modalitySpinnerWindow = null;
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

        void _pmtSafetyTimer_Tick(object sender, EventArgs e)
        {
            ((ThorSharedTypes.IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("PMTSafetyStatus");

            //The safety flag for the hardware has tripped
            //Present a message to the user and disable the PMTs
            if ((false == (bool)MVMManager.Instance["ScanControlViewModel", "PMTSafetyStatus", (object)false]) && (_showingPMTSafetyMessage == false))
            {
                _showingPMTSafetyMessage = true;
                StopLiveCapture();
                MessageBox.Show("The PMT(s) safety has tripped. To prevent damage to the PMT(s) please reduce the light being sent to the detectors.");
                _showingPMTSafetyMessage = false;
                ((HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 0]).Value = 0;
                ((HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 1]).Value = 0;
                ((HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 2]).Value = 0;
                ((HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 3]).Value = 0;
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