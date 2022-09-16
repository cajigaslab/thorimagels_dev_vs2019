namespace AreaControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using AreaControl.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public class AreaControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        const int ZOOM_TABLE_LENGTH = 8;

        private readonly AreaControlModel _areaControlModel;

        private double originalDwellTime;
        double[] _calibrationArray = new double[256];
        private ICommand _centerROICommand;
        private ICommand _centerScannersCommand;
        private ICommand _closeShutterCommand;
        private bool _configMicroScanArea = true;
        private ICommand _dwellTimeMinusCommand;
        private ICommand _dwellTimePlusCommand;
        private bool _enableResolutionPresets = true;
        private bool _fieldFromROIZoomMode = false;
        ICommand _fieldOffsetFineResetCommand;
        private ICommand _fieldOffsetXFineMinusCommand;
        private ICommand _fieldOffsetXFinePlusCommand;
        private ICommand _fieldOffsetXMinusCommand;
        private ICommand _fieldOffsetXPlusCommand;
        private ICommand _fieldOffsetYFineMinusCommand;
        private ICommand _fieldOffsetYFinePlusCommand;
        private ICommand _fieldOffsetYMinusCommand;
        private ICommand _fieldOffsetYPlusCommand;
        ICommand _fieldScaleFineResetCommand;
        private ICommand _fieldScaleXFineMinusCommand;
        private ICommand _fieldScaleXFinePlusCommand;
        private ICommand _fieldScaleYFineMinusCommand;
        private ICommand _fieldScaleYFinePlusCommand;
        private double _fieldSizeCalibration;
        private ICommand _fieldSizeMinusCommand;
        private ICommand _fieldSizePlusCommand;
        private Visibility _fieldSizeVisibility;
        private ICommand _imagingCenterScannersCommand;
        private bool _imagingStatus = true;
        private long _lsmLastCalibrationDateUnix;
        private ICommand _lsmSaveCalibrationCommand;
        ICommand _lsmScanAreaAngleMinusCommand;
        ICommand _lsmScanAreaAnglePlusCommand;
        double _LSMUMPerPixel = 0.0;
        private int _mesoStripPixels;
        private ICommand _mesoStripPixelsMinusCommand;
        private ICommand _mesoStripPixelsPlusCommand;
        private ICommand _nyquistCommand;
        private Canvas _overlayCanvas;
        private Canvas _overviewOverlayCanvas;
        private bool _overviewVisible = false;
        private MesoOverview _overviewWin = null;
        private bool _previousAlwaysUseFastestState = true;
        private int _previousAverageState = 0;
        private bool _previousTimebasedSelection = false;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        ICommand _resolutionAddCommand;
        private ICommand _returnToOriginalAreaCommand;
        private ICommand _roiZoomInCommand;
        private ICommand _selectBackgroundCommand;
        private int _selectedScanArea = 1;
        private int _selectedStripSize = 300;
        private int _selectedViewMode = 0;
        private ICommand _selectFlatFieldCommand;
        private DispatcherTimer _statusTimer = null;
        private ICommand _storeRSRateCommand = null;
        private List<int> _stripSizes = new List<int>(new int[] { 300, 400, 500, 600 });
        private ICommand _timeBasedLSTimeMSMinusCommand;
        private ICommand _timeBasedLSTimeMSPlusCommand;
        private bool _timedBasedVisibility = false;
        bool _useCalibrationArray = false;
        private ICommand _zoomMinusCommand;
        private ICommand _zoomPlusCommand;
        int[] _zoomTable = new int[ZOOM_TABLE_LENGTH];

        #endregion Fields

        #region Constructors

        public AreaControlViewModel()
        {
            this._areaControlModel = new AreaControlModel();

            //default properties
            _mesoStripPixels = MesoStripPixelsRange[0];
            LockFieldOffset = false;
            ConfigMicroScanArea = false;

            LoadZoomData();

            //use timer to update status
            _statusTimer = new DispatcherTimer();
            _statusTimer.Interval = new TimeSpan(0, 0, 0, 0, 100);
            _statusTimer.Tick += new EventHandler(_statusTimer_Tick);
        }

        #endregion Constructors

        #region Properties

        public Visibility AreaAngleVisibility
        {
            get
            {

                if ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType() &&
                    ThorSharedTypes.ICamera.LSMAreaMode.POLYLINE != (ThorSharedTypes.ICamera.LSMAreaMode)LSMAreaMode)
                {
                    return Visibility.Visible;
                }
                else
                {
                    return Visibility.Collapsed;
                }
            }
        }

        public ICommand CenterROICommand
        {
            get
            {
                if (this._centerROICommand == null)
                    this._centerROICommand = new RelayCommand(() => CenterROI());

                return this._centerROICommand;
            }
        }

        public ICommand CenterScannersCommand
        {
            get
            {
                if (this._centerScannersCommand == null)
                    this._centerScannersCommand = new RelayCommandWithParam((x) => CenterScanners(x));

                return this._centerScannersCommand;
            }
        }

        public ICommand CloseShutterCommand
        {
            get
            {
                if (this._closeShutterCommand == null)
                    this._closeShutterCommand = new RelayCommand(() => CloseShutter());

                return this._closeShutterCommand;
            }
        }

        public double Coeff1
        {
            get
            {
                return _areaControlModel.Coeff1;
            }
            set
            {
                _areaControlModel.Coeff1 = value;
                OnPropertyChanged("Coeff1");
            }
        }

        public double Coeff2
        {
            get
            {
                return _areaControlModel.Coeff2;
            }
            set
            {
                _areaControlModel.Coeff2 = value;
                OnPropertyChanged("Coeff2");
            }
        }

        public double Coeff3
        {
            get
            {
                return _areaControlModel.Coeff3;
            }
            set
            {
                _areaControlModel.Coeff3 = value;
                OnPropertyChanged("Coeff3");
            }
        }

        public double Coeff4
        {
            get
            {
                return _areaControlModel.Coeff4;
            }
            set
            {
                _areaControlModel.Coeff4 = value;
                OnPropertyChanged("Coeff4");
            }
        }

        public bool ConfigMicroScanArea
        {
            get
            {
                return _configMicroScanArea;
            }
            set
            {
                if (_configMicroScanArea != value)
                {
                    _configMicroScanArea = value;
                    OnPropertyChanged("ConfigMicroScanArea");

                    MVMManager.Instance["CaptureSetupViewModel", "WrapPanelEnabled"] = !_configMicroScanArea;
                    MVMManager.Instance["CaptureSetupViewModel", "ROIToolVisible"] = (_configMicroScanArea) ?
                        new bool[14] { true, false, true, false, false, false, false, false, false, true, true, true, true, false } :
                        new bool[14] { true, true, true, true, true, true, true, true, true, true, true, true, true, true };

                    OverlayManagerClass.Instance.InitSelectROI(ref _overlayCanvas);
                    OverlayManagerClass.Instance.DisplayModeROI(ref _overlayCanvas, _configMicroScanArea ? new ThorSharedTypes.Mode[1] { ThorSharedTypes.Mode.STATSONLY } : new ThorSharedTypes.Mode[1] { ThorSharedTypes.Mode.MICRO_SCANAREA }, false);
                    OverlayManagerClass.Instance.DisplayModeROI(ref _overlayCanvas, _configMicroScanArea ? new ThorSharedTypes.Mode[1] { ThorSharedTypes.Mode.MICRO_SCANAREA } : new ThorSharedTypes.Mode[1] { ThorSharedTypes.Mode.STATSONLY }, true);
                    OverlayManagerClass.Instance.CurrentMode = _configMicroScanArea ? ThorSharedTypes.Mode.MICRO_SCANAREA : ThorSharedTypes.Mode.STATSONLY;

                    if (_configMicroScanArea)
                    {
                        OverlayManagerClass.Instance.ObjectSizeChangedEvent += OverlayManager_ObjectSizeChangedEvent;
                        MVMManager.Instance["CaptureSetupViewModel", "SLMPatternsVisible"] = false;

                        //save overview
                        if (null != MVMManager.Instance["CaptureSetupViewModel", "Bitmap", null])
                        {
                            BitmapSource bmSource = ((WriteableBitmap)MVMManager.Instance["CaptureSetupViewModel", "Bitmap"]).Clone();
                            using (FileStream stream = new FileStream(MesoOverview.MesoOverviewPathAndName, FileMode.Create))
                            {
                                BmpBitmapEncoder bmEncoder = new BmpBitmapEncoder();
                                bmEncoder.Frames.Add(BitmapFrame.Create(bmSource));
                                bmEncoder.Save(stream);
                            }
                        }
                    }
                    else
                    {
                        OverlayManagerClass.Instance.ObjectSizeChangedEvent -= OverlayManager_ObjectSizeChangedEvent;
                        OnPropertyChanged("MicroScanAreas");
                        OnPropertyChanged("SelectedScanArea");
                    }
                    OnPropertyChanged("StripVisible");
                    OnPropertyChanged("ROIFrameRate");
                }
            }
        }

        public bool ConfirmAreaModeSettingsForGG
        {
            get
            {
                if ((int)ICamera.LSMType.GALVO_GALVO != ResourceManagerCS.GetLSMType())
                {
                    return false;
                }
                int channel = 0;
                double dwellFactor = 0;
                int lines = 0;

                switch ((ICamera.LSMAreaMode)LSMAreaMode)
                {
                    case ICamera.LSMAreaMode.SQUARE:
                        {
                            return false;
                        }
                    case ICamera.LSMAreaMode.RECTANGLE:
                        {
                            return false;
                        }
                    case ICamera.LSMAreaMode.LINE_TIMELAPSE:
                        {
                            int tempY = LSMPixelY;
                            LSMPixelY = Math.Max(tempY, LSMPixelXMin);
                            return true;
                        }
                    case ICamera.LSMAreaMode.LINE:
                        {
                            const int TWO_WAY_SCAN = 0;
                            if (TWO_WAY_SCAN == (int)MVMManager.Instance["ScanControlViewModel", "LSMScanMode", (object)0])
                            {
                                if (1 == LSMPixelY)
                                {
                                    lines = 2;
                                }
                                else
                                {
                                    lines = LSMPixelY;
                                }
                            }
                            else
                            {
                                if (2 == LSMPixelY)
                                {
                                    lines = 1;
                                }
                                else
                                {
                                    lines = LSMPixelY;
                                }
                            }

                            channel = (3 < (int)MVMManager.Instance["CaptureSetupViewModel", "LSMChannel", (object)0]) ? 4 : 1;
                            dwellFactor = 0.24; //0.24 was determined experimentally
                        }
                        break;
                    case ICamera.LSMAreaMode.POLYLINE:
                        {
                            lines = LSMPixelY;
                            channel = 1; //channel buffer size has no effect for polyline acquisition. Thus, it is not being considered
                            dwellFactor = 0.72; //0.72 was determined experimentally
                        }
                        break;
                }

                if (ICamera.LSMAreaMode.LINE == (ICamera.LSMAreaMode)LSMAreaMode ||
                    ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)LSMAreaMode)
                {

                    int pixelX = LSMPixelX;
                    double newDwellTime = (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime", (object)0];

                    //Ensure th dwell time is long enough for single line scan
                    double m = lines * newDwellTime * pixelX * channel / 1000; //ms
                    if (dwellFactor > m)
                    {
                        while (true)
                        {
                            newDwellTime += (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTimeStep", (object)0];
                            m = lines * newDwellTime * pixelX * channel / 1000; //ms
                            if (dwellFactor <= m)
                            {
                                break;
                            }
                        }
                        MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = newDwellTime;
                    }
                    LSMPixelY = lines;

                    return true;
                }
                return false;
            }
        }

        /// <summary>
        /// The currently set image resolution
        /// </summary>
        public ImageResolution CurrentResolution
        {
            get
            {
                return new ImageResolution(this._areaControlModel.LSMPixelX, this._areaControlModel.LSMPixelY);
            }
        }

        public int EnableBackgroundSubtraction
        {
            get
            {
                return _areaControlModel.EnableBackgroundSubtraction;
            }
            set
            {
                _areaControlModel.EnableBackgroundSubtraction = value;
                OnPropertyChanged("EnableBackgroundSubtraction");
            }
        }

        public int EnableFlatField
        {
            get
            {
                return _areaControlModel.EnableFlatField;
            }
            set
            {
                _areaControlModel.EnableFlatField = value;
                OnPropertyChanged("EnableFlatField");
            }
        }

        public int EnablePincushionCorrection
        {
            get
            {
                return _areaControlModel.PincushionCorrection;
            }
            set
            {
                _areaControlModel.PincushionCorrection = value;
                OnPropertyChanged("EnablePincushionCorrection");
            }
        }

        public int EnableReferenceChannel
        {
            get
            {
                return _areaControlModel.EnableReferenceChannel;
            }
            set
            {
                _areaControlModel.EnableReferenceChannel = value;
            }
        }

        public bool EnableResolutionPresets
        {
            get
            {
                return _enableResolutionPresets;
            }
            set
            {
                _enableResolutionPresets = value;
                OnPropertyChanged("EnableResolutionPresets");
            }
        }

        public bool FieldFromROIEnable
        {
            get
            {
                if (1 >= this.LSMAreaMode)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        public bool FieldFromROIZoomMode
        {
            get
            {
                return _fieldFromROIZoomMode;
            }
            set
            {
                _fieldFromROIZoomMode = value;
            }
        }

        public Visibility FieldSizeVisible
        {
            get
            {
                return _fieldSizeVisibility;
            }
            set
            {
                _fieldSizeVisibility = value;
                OnPropertyChanged("FieldSizeVisible");
            }
        }

        public bool FielSizeAdjustMentEnable
        {
            get
            {
                if (ICamera.LSMAreaMode.POLYLINE != (ICamera.LSMAreaMode)LSMAreaMode)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        public Visibility GGLSMScanVisibility
        {
            get
            {
                return (((int)ThorSharedTypes.ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType()) &&
                    ((int)ThorSharedTypes.ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType())) ?
                    Visibility.Visible : Visibility.Collapsed;
            }
        }

        public bool GGSuperUserMode
        {
            get
            {
                return this._areaControlModel.GGSuperUserMode;
            }
            set
            {
                this._areaControlModel.GGSuperUserMode = value;
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMFlybackCycles");
            }
        }

        public Visibility GRLSMScanVisibility
        {
            get
            {
                return (((int)ThorSharedTypes.ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType()) &&
                    ((int)ThorSharedTypes.ICamera.LSMType.GALVO_RESONANCE == ResourceManagerCS.GetLSMType())) ?
                    Visibility.Visible : Visibility.Collapsed;
            }
        }

        public bool ImageStartStatusArea
        {
            get
            {
                return _imagingStatus;
            }
            set
            {
                _imagingStatus = value;
                OnPropertyChanged("ImageStartStatusArea");
            }
        }

        public ICommand ImagingCenterScannersCommand
        {
            get
            {
                if (this._imagingCenterScannersCommand == null)
                    this._imagingCenterScannersCommand = new RelayCommandWithParam((x) => ImagingCenterScanners(x));

                return this._imagingCenterScannersCommand;
            }
        }

        public Visibility IsGalvoRes
        {
            get
            {
                return ((int)ICamera.LSMType.GALVO_RESONANCE == ResourceManagerCS.GetLSMType()) ? Visibility.Hidden : Visibility.Visible;
            }
        }

        public bool IsRectangleAreaModeAvailable
        {
            get
            {
                return this._areaControlModel.IsRectangleAreaModeAvailable;
            }
        }

        public Visibility KymographVisibility
        {
            get
            {
                return ((int)ICamera.LSMType.GALVO_RESONANCE == ResourceManagerCS.GetLSMType()) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public bool LockFieldOffset
        {
            get
            {
                return this._areaControlModel.LockFieldOffset;
            }
            set
            {
                this._areaControlModel.LockFieldOffset = value;
                OnPropertyChanged("LockFieldOffset");
                CenterROI();
            }
        }

        public int LSM1xFieldSize
        {
            get
            {
                return this._areaControlModel.LSM1xFieldSize;
            }
        }

        public int LSMAreaMode
        {
            get
            {
                return this._areaControlModel.LSMAreaMode;
            }
            set
            {
                if (ICamera.LSMAreaMode.LINE == (ICamera.LSMAreaMode)LSMAreaMode || ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)LSMAreaMode)
                {
                    //If we are moving away from Line or Polyline mode, save the state of the time based line scan checkbox
                    _previousTimebasedSelection = TimeBasedLineScan;
                    TimeBasedLineScan = false;
                }
                if (ICamera.LSMAreaMode.LINE == (ICamera.LSMAreaMode)value || ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)value)
                {
                    //When moving to Line or Polyline mode bring back the previous state of the timed based line scan checkbox
                    TimeBasedLineScan = _previousTimebasedSelection;
                }
                this._areaControlModel.LSMAreaMode = value;
                if (ICamera.LSMAreaMode.SQUARE == (ICamera.LSMAreaMode)value ||
                    ICamera.LSMAreaMode.RECTANGLE == (ICamera.LSMAreaMode)value ||
                    ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)value)
                {
                    int tempY = this._areaControlModel.LSMPixelY;

                    //match the X and Y to ensure that the areamode set function passes
                    this._areaControlModel.LSMPixelY = this._areaControlModel.LSMPixelX;

                    switch ((ICamera.LSMAreaMode)value)
                    {
                        case ICamera.LSMAreaMode.SQUARE:
                            {
                                this._areaControlModel.LSMPixelY = this._areaControlModel.LSMPixelX;
                                this.LSMLineScanEnable = 0;
                                this.EnableResolutionPresets = true;
                            }
                            break;
                        case ICamera.LSMAreaMode.RECTANGLE:
                            {
                                this._areaControlModel.LSMPixelY = Math.Max(tempY, this._areaControlModel.LSMPixelXMin);
                                this.LSMLineScanEnable = 0;
                                this.EnableResolutionPresets = true;
                            }
                            break;
                        case ICamera.LSMAreaMode.POLYLINE:
                            {
                                this._areaControlModel.LSMPixelY = 1;
                                this.LSMLineScanEnable = 1;
                                if (true == (bool)MVMManager.Instance["CaptureSetupViewModel", "IsLive", (object)false])
                                {
                                    MVMManager.Instance["CaptureSetupViewModel", "DrawLineForLineScan"] = true;
                                }
                                this.EnableResolutionPresets = false;
                            }
                            break;
                    }

                }
                else
                {
                    if ((int)ICamera.LSMAreaMode.LINE_TIMELAPSE == value)
                    {
                        int tempY = this._areaControlModel.LSMPixelY;
                        this._areaControlModel.LSMPixelY = Math.Max(tempY, this._areaControlModel.LSMPixelXMin);
                        this.EnableResolutionPresets = true;
                    }
                    else //Line mode
                    {
                        this.LSMPixelY = 1;
                        if (true == (bool)MVMManager.Instance["CaptureSetupViewModel", "IsLive", (object)false])
                        {
                            MVMManager.Instance["CaptureSetupViewModel", "DrawLineForLineScan"] = true;
                        }
                        this.EnableResolutionPresets = true;
                    }
                    this.LSMLineScanEnable = 1;
                }

                this.EnableResolutionPresets = !TimeBasedLineScan;

                //Recalculate time based line scan parameters
                if (TimeBasedLineScan)
                {
                    TimeBasedLSTimeMS = TimeBasedLSTimeMS;
                }

                OnPropertyChanged("ResolutionPresets");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("TwoWayEnable");
                OnPropertyChanged("FieldFromROIEnable");
                OnPropertyChanged("AreaAngleVisibility");
                OnPropertyChanged("LSMAreaMode");
                OnPropertyChanged("LSMPixelX");
                OnPropertyChanged("LSMPixelY");
                OnPropertyChanged("LSMFieldSizeDisplayX");
                OnPropertyChanged("LSMFieldSizeDisplayY");
                OnPropertyChanged("LSMFieldOffsetXDisplay");
                OnPropertyChanged("LSMFieldOffsetYDisplay");
                OnPropertyChanged("PixelYSliderVibility");
                OnPropertyChanged("PixelXSliderVibility");
                OnPropertyChanged("PolylineScanVisibility");
                OnPropertyChanged("RectangleAreaModeVisibility");
                OnPropertyChanged("TimedBasedVisiblity");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("TwoWayVisibility");
                OnPropertyChanged("FielSizeAdjustMentEnable");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMFlybackCycles");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMFlybackTime");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMInterleaveScan");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");
            }
        }

        public string LSMAreaName
        {
            get
            {
                return this._areaControlModel.LSMAreaName;
            }
        }

        public ICommand LSMDwellTimeMinusCommand
        {
            get
            {
                if (this._dwellTimeMinusCommand == null)
                    this._dwellTimeMinusCommand = new RelayCommand(() => DwellTimeMinus());

                return this._dwellTimeMinusCommand;
            }
        }

        public ICommand LSMDwellTimePlusCommand
        {
            get
            {
                if (this._dwellTimePlusCommand == null)
                    this._dwellTimePlusCommand = new RelayCommand(() => DwellTimePlus());

                return this._dwellTimePlusCommand;
            }
        }

        public double LSMField2Theta
        {
            get
            {
                return _areaControlModel.LSMField2Theta;
            }
            set
            {
                _areaControlModel.LSMField2Theta = value;
                OnPropertyChanged("LSMField2Theta");
            }
        }

        public ICommand LSMFieldOffsetFineResetCommand
        {
            get
            {
                if (this._fieldOffsetFineResetCommand == null)
                    this._fieldOffsetFineResetCommand = new RelayCommand(() => FieldOffsetFineReset());

                return this._fieldOffsetFineResetCommand;
            }
        }

        public int LSMFieldOffsetXActual
        {
            get
            {
                if (this.LockFieldOffset)
                {
                    return 0;
                }
                int val = _areaControlModel.LSMFieldOffsetX;
                return val;
            }
            set
            {
                if (this.LockFieldOffset)
                {
                    value = 0;
                }
                if ((value <= (this.LSMFieldSizeMax - this.LSMFieldSize) / 2) && (value >= -(this.LSMFieldSizeMax - this.LSMFieldSize) / 2))
                {
                    if (LSMAreaName.Contains("Resonance"))
                    {
                        _areaControlModel.LSMFieldOffsetX = 0;
                    }
                    else
                    {
                        _areaControlModel.LSMFieldOffsetX = value;
                    }
                    OnPropertyChanged("LSMFieldSizeDisplayX");
                    OnPropertyChanged("LSMFieldOffsetXActual");
                    OnPropertyChanged("LSMFieldOffsetXDisplay");
                }
            }
        }

        public int LSMFieldOffsetXDisplay
        {
            get
            {
                if (LSMFlipVerticalScan == 0)
                {
                    return (int)Math.Round((double)(this.LSMFieldOffsetXActual * (-1) + 128 - this.LSMFieldSizeDisplayX) / 2, 0);
                }
                else
                {
                    return (int)Math.Round((double)(this.LSMFieldOffsetXActual + 128 - this.LSMFieldSizeDisplayX) / 2, 0);
                }
            }
            set
            {
                if (!LSMAreaName.Contains("Resonance"))
                {

                    if (LSMFlipVerticalScan == 0)
                    {
                        this.LSMFieldOffsetXActual = (-1) * (value * 2 - 128 + this.LSMFieldSizeDisplayX);
                    }
                    else
                    {
                        this.LSMFieldOffsetXActual = value * 2 - 128 + this.LSMFieldSizeDisplayX;
                    }
                }
                OnPropertyChanged("LSMFieldSizeDisplayX");
                OnPropertyChanged("LSMFieldOffsetXActual");
                OnPropertyChanged("LSMFieldOffsetXDisplay");
            }
        }

        public double LSMFieldOffsetXFine
        {
            get
            {
                return this._areaControlModel.LSMFieldOffsetXFine;
            }
            set
            {
                this._areaControlModel.LSMFieldOffsetXFine = value;
                OnPropertyChanged("LSMFieldOffsetXFine");
            }
        }

        public ICommand LSMFieldOffsetXFineMinusCommand
        {
            get
            {
                if (this._fieldOffsetXFineMinusCommand == null)
                    this._fieldOffsetXFineMinusCommand = new RelayCommand(() => FieldOffsetXFineMinus());

                return this._fieldOffsetXFineMinusCommand;
            }
        }

        public ICommand LSMFieldOffsetXFinePlusCommand
        {
            get
            {
                if (this._fieldOffsetXFinePlusCommand == null)
                    this._fieldOffsetXFinePlusCommand = new RelayCommand(() => FieldOffsetXFinePlus());

                return this._fieldOffsetXFinePlusCommand;
            }
        }

        public ICommand LSMFieldOffsetXMinusCommand
        {
            get
            {
                if (this._fieldOffsetXMinusCommand == null)
                    this._fieldOffsetXMinusCommand = new RelayCommand(() => FieldOffsetXMinus());

                return this._fieldOffsetXMinusCommand;
            }
        }

        public ICommand LSMFieldOffsetXPlusCommand
        {
            get
            {
                if (this._fieldOffsetXPlusCommand == null)
                    this._fieldOffsetXPlusCommand = new RelayCommand(() => FieldOffsetXPlus());

                return this._fieldOffsetXPlusCommand;
            }
        }

        public int LSMFieldOffsetYActual
        {
            get
            {
                if (this.LockFieldOffset)
                {
                    return 0;
                }

                return _areaControlModel.LSMFieldOffsetY;
            }
            set
            {
                if (this.LockFieldOffset)
                {
                    value = 0;
                }
                if ((value <= (this.LSMFieldSizeMax - this.LSMFieldSize) / 2) && (value >= -(this.LSMFieldSizeMax - this.LSMFieldSize) / 2))
                {

                    _areaControlModel.LSMFieldOffsetY = value;
                    OnPropertyChanged("LSMFieldSizeDisplayY");
                    OnPropertyChanged("LSMFieldOffsetYActual");
                    OnPropertyChanged("LSMFieldOffsetYDisplay");
                }
            }
        }

        public int LSMFieldOffsetYDisplay
        {
            get
            {
                if (1 == LSMLineScanEnable)
                {
                    int fs = Math.Max(1, (int)Math.Round(CalculateFieldOffsetYDisplay(this.LSMPixelX, 1, this.LSMFieldSize), 0));
                    return (int)Math.Round((double)(this.LSMFieldOffsetYActual + 128 - fs) / 2, 0);
                }
                else
                {
                    int fs = Math.Max(1, (int)Math.Round(CalculateFieldOffsetYDisplay(this.LSMPixelX, this.LSMPixelY, this.LSMFieldSize), 0));
                    return (int)Math.Round((double)(this.LSMFieldOffsetYActual + 128 - fs) / 2, 0);
                }
            }
            set
            {
                int fs = Math.Max(1, (int)Math.Round(CalculateFieldOffsetYDisplay(this.LSMPixelX, this.LSMPixelY, this.LSMFieldSize), 0));

                this.LSMFieldOffsetYActual = value * 2 - 128 + fs;
                OnPropertyChanged("LSMFieldSizeDisplayY");
                OnPropertyChanged("LSMFieldOffsetYDisplay");
                OnPropertyChanged("LSMFieldSizeDisplayY");
            }
        }

        public double LSMFieldOffsetYFine
        {
            get
            {
                return this._areaControlModel.LSMFieldOffsetYFine;
            }
            set
            {
                this._areaControlModel.LSMFieldOffsetYFine = value;
                OnPropertyChanged("LSMFieldOffsetYFine");
            }
        }

        public ICommand LSMFieldOffsetYFineMinusCommand
        {
            get
            {
                if (this._fieldOffsetYFineMinusCommand == null)
                    this._fieldOffsetYFineMinusCommand = new RelayCommand(() => FieldOffsetYFineMinus());

                return this._fieldOffsetYFineMinusCommand;
            }
        }

        public ICommand LSMFieldOffsetYFinePlusCommand
        {
            get
            {
                if (this._fieldOffsetYFinePlusCommand == null)
                    this._fieldOffsetYFinePlusCommand = new RelayCommand(() => FieldOffsetYFinePlus());

                return this._fieldOffsetYFinePlusCommand;
            }
        }

        public ICommand LSMFieldOffsetYMinusCommand
        {
            get
            {
                if (this._fieldOffsetYMinusCommand == null)
                    this._fieldOffsetYMinusCommand = new RelayCommand(() => FieldOffsetYMinus());

                return this._fieldOffsetYMinusCommand;
            }
        }

        public ICommand LSMFieldOffsetYPlusCommand
        {
            get
            {
                if (this._fieldOffsetYPlusCommand == null)
                    this._fieldOffsetYPlusCommand = new RelayCommand(() => FieldOffsetYPlus());

                return this._fieldOffsetYPlusCommand;
            }
        }

        public ICommand LSMFieldScaleFineResetCommand
        {
            get
            {
                if (this._fieldScaleFineResetCommand == null)
                    this._fieldScaleFineResetCommand = new RelayCommand(() => FieldScaleFineReset());

                return this._fieldScaleFineResetCommand;
            }
        }

        public double LSMFieldScaleXFine
        {
            get
            {
                return this._areaControlModel.LSMFieldScaleXFine;
            }
            set
            {
                this._areaControlModel.LSMFieldScaleXFine = value;
                OnPropertyChanged("LSMFieldScaleXFine");
            }
        }

        public ICommand LSMFieldScaleXFineMinusCommand
        {
            get
            {
                if (this._fieldScaleXFineMinusCommand == null)
                    this._fieldScaleXFineMinusCommand = new RelayCommand(() => FieldScaleXFineMinus());

                return this._fieldScaleXFineMinusCommand;
            }
        }

        public ICommand LSMFieldScaleXFinePlusCommand
        {
            get
            {
                if (this._fieldScaleXFinePlusCommand == null)
                    this._fieldScaleXFinePlusCommand = new RelayCommand(() => FieldScaleXFinePlus());

                return this._fieldScaleXFinePlusCommand;
            }
        }

        public double LSMFieldScaleYFine
        {
            get
            {
                return this._areaControlModel.LSMFieldScaleYFine;
            }
            set
            {
                this._areaControlModel.LSMFieldScaleYFine = value;
                OnPropertyChanged("LSMFieldScaleYFine");
            }
        }

        public ICommand LSMFieldScaleYFineMinusCommand
        {
            get
            {
                if (this._fieldScaleYFineMinusCommand == null)
                    this._fieldScaleYFineMinusCommand = new RelayCommand(() => FieldScaleYFineMinus());

                return this._fieldScaleYFineMinusCommand;
            }
        }

        public ICommand LSMFieldScaleYFinePlusCommand
        {
            get
            {
                if (this._fieldScaleYFinePlusCommand == null)
                    this._fieldScaleYFinePlusCommand = new RelayCommand(() => FieldScaleYFinePlus());

                return this._fieldScaleYFinePlusCommand;
            }
        }

        public int LSMFieldSize
        {
            get
            {
                return this._areaControlModel.LSMFieldSize;
            }
            set
            {
                if ((LSMPixelX == 0) || (LSMPixelY == 0))
                {
                    this._areaControlModel.LSMFieldSize = 0;
                    return;
                }

                this._areaControlModel.LSMFieldSize = value;
                OnPropertyChanged("LSMFieldSize");

                OnPropertyChanged("LSMFieldSizeDisplayX");
                OnPropertyChanged("LSMFieldSizeDisplayY");
                OnPropertyChanged("LSMFieldOffsetYDisplay");
                OnPropertyChanged("LSMFieldOffsetXDisplay");

                OnPropertyChanged("LSMFieldSizeXUM");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("FieldSizeWidthUM");
                OnPropertyChanged("LSMFieldSizeYUM");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("FieldSizeHeightUM");
                OnPropertyChanged("LSMFieldSizeXMM");
                OnPropertyChanged("LSMFieldSizeYMM");
                OnPropertyChanged("LSMUMPerPixel");

                //Not necessary, LSMTwoWayAlignmentCoarse and LSMTwoWayAlignment is already set in AreaControlModel LSMFieldSize Set
                //((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMTwoWayAlignmentCoarse");
                //((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMTwoWayAlignment");

                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");

                //if (MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"]; < _minDwellTimeTable[(this.LSMPixelX >> PIXEL_DENSITY_INCREMENT) - 1][value])
                //{
                //    MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"]; = Math.Round(_minDwellTimeTable[(this.LSMPixelX >> PIXEL_DENSITY_INCREMENT) - 1][value], 1);

                //    LSMGalvoRate = (MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"]; > 2.0) ? 0 : 1;
                //}

                //:TODO:Delete the commented code below if it works correctly for all GG scanners
                //try
                //{
                //    double minDwellTime = (double)MVMManager.Instance["ScanControlViewModel", "MinDwellTimeFromTable", (object)0];
                //    if ((double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime", (object)0] < minDwellTime && !GGSuperUserMode)
                //    {
                //        MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = Math.Round(minDwellTime, 1);
                //    }
                //}
                //catch (Exception e)
                //{
                //    e.ToString();
                //}

                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMDwellTimeMinIndex");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMDwellTimeMaxIndex");
                OnPropertyChanged("LSMZoom");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLSMZoom");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMFlybackCycles");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMFlybackTime");

            }
        }

        public double LSMFieldSizeCalibration
        {
            get
            {
                double retVal = 1.0;

                //If the scanner returns a field size calibration
                //use that. Otherwise use the settings file
                if (1 == GetLSMFieldSizeCalibration(ref retVal))
                {

                }
                else
                {
                    retVal = _fieldSizeCalibration;
                }

                //use the zoomdata calibration if
                //the scanner is G/R

                //*NOTE* this logic is moved
                //into the G/R dll, since it has the zoom
                //information available to return the adjusted
                //value
                //if ((int)ICamera.LSMType.GALVO_RESONANCE == GetLSMType())
                //{
                //    if (_useCalibrationArray)
                //    {
                //        retVal = retVal + retVal * _calibrationArray[LSMFieldSize];
                //    }
                //}

                return retVal;

            }
            set
            {
                _fieldSizeCalibration = value;
                OnPropertyChanged("LSMFieldSizeXUM");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("FieldSizeWidthUM");
                OnPropertyChanged("LSMFieldSizeYUM");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("FieldSizeHeightUM");
                OnPropertyChanged("LSMFieldSizeXMM");
                OnPropertyChanged("LSMFieldSizeYMM");
                OnPropertyChanged("LSMUMPerPixel");
                OnPropertyChanged("MMPerPixel");
            }
        }

        public int LSMFieldSizeDisplayX
        {
            get
            {
                //return Math.Max(1,Convert.ToInt32(CalculateFieldOffsetXDisplay(this.LSMPixelX, this.LSMPixelY, this.LSMFieldSize)));
                return Math.Max(1, (int)Math.Round(CalculateFieldOffsetXDisplay(this.LSMPixelX, this.LSMPixelY, this.LSMFieldSize), 0));
            }
            set
            {
                if ((int)ICamera.LSMType.GALVO_RESONANCE == ResourceManagerCS.GetLSMType())
                {
                    this.LSMFieldOffsetXActual = 0;
                }
            }
        }

        public int LSMFieldSizeDisplayY
        {
            get
            {
                if (1 == LSMLineScanEnable)
                {
                    //return Math.Max(1, Convert.ToInt32(CalculateFieldOffsetYDisplay(this.LSMPixelX, 1, this.LSMFieldSize)));
                    return Math.Max(1, (int)Math.Round(CalculateFieldOffsetYDisplay(this.LSMPixelX, 1, this.LSMFieldSize), 0));
                }
                else
                {
                    //return Math.Max(1, Convert.ToInt32(CalculateFieldOffsetYDisplay(this.LSMPixelX, this.LSMPixelY, this.LSMFieldSize)));
                    return Math.Max(1, (int)Math.Round(CalculateFieldOffsetYDisplay(this.LSMPixelX, this.LSMPixelY, this.LSMFieldSize), 0));
                }
            }
            set
            {
            }
        }

        public int LSMFieldSizeMax
        {
            get
            {
                return this._areaControlModel.LSMFieldSizeMax;
            }
        }

        public int LSMFieldSizeMin
        {
            get
            {
                return this._areaControlModel.LSMFieldSizeMin;
            }
        }

        public ICommand LSMFieldSizeMinusCommand
        {
            get
            {
                if (this._fieldSizeMinusCommand == null)
                    this._fieldSizeMinusCommand = new RelayCommand(() => FieldSizeMinus());

                return this._fieldSizeMinusCommand;
            }
        }

        public ICommand LSMFieldSizePlusCommand
        {
            get
            {
                if (this._fieldSizePlusCommand == null)
                    this._fieldSizePlusCommand = new RelayCommand(() => FieldSizePlus());

                return this._fieldSizePlusCommand;
            }
        }

        public double LSMFieldSizeXMM
        {
            get
            {
                double dVal = this._areaControlModel.LSMFieldSize * this.LSMFieldSizeCalibration / (1000.0 * (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)0.0]);

                dVal = Math.Round(dVal, 2);
                return dVal;
            }
        }

        public double LSMFieldSizeXUM
        {
            get
            {
                double dVal = this._areaControlModel.LSMFieldSize * this.LSMFieldSizeCalibration / (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)0.0];

                dVal = Math.Round(dVal, 2);
                return dVal;
            }
        }

        public double LSMFieldSizeYMM
        {
            get
            {
                double dVal = this._areaControlModel.LSMFieldSize * this.LSMFieldSizeCalibration / (1000.0 * (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)0.0]);

                dVal = Math.Round(dVal, 2);
                return dVal;
            }
        }

        public double LSMFieldSizeYUM
        {
            get
            {
                double dVal;

                if (1 == LSMLineScanEnable)
                {
                    double pixelXYRatio = 1.0 / LSMPixelX;
                    dVal = this._areaControlModel.LSMFieldSize * this.LSMFieldSizeCalibration / (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)0.0] * pixelXYRatio;
                }
                else
                {
                    double pixelXYRatio = Convert.ToDouble(LSMPixelY) / Convert.ToDouble(LSMPixelX);
                    dVal = this._areaControlModel.LSMFieldSize * this.LSMFieldSizeCalibration / (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)0.0] * pixelXYRatio;
                }

                dVal = Math.Round(dVal, 2);
                return dVal;
            }
        }

        /// <summary>
        /// Gets or sets the LSM flip horizontal.
        /// </summary>
        /// <value>The LSM flip horizontal.</value>
        public int LSMFlipHorizontal
        {
            get
            {
                return this._areaControlModel.LSMFlipHorizontal;
            }

            set
            {
                this._areaControlModel.LSMFlipHorizontal = value;
                OnPropertyChanged("LSMFlipHorizontal");
            }
        }

        public int LSMFlipVerticalScan
        {
            get
            {
                return this._areaControlModel.LSMFlipVerticalScan;
            }

            set
            {
                this._areaControlModel.LSMFlipVerticalScan = value;
                OnPropertyChanged("LSMFlipVerticalScan");
            }
        }

        public string LSMLastCalibrationDate
        {
            get
            {
                return string.Format("Calibration Date {0}", ResourceManagerCS.ToDateTimeFromUnix(LSMLastCalibrationDateUnix));
            }
        }

        public long LSMLastCalibrationDateUnix
        {
            get
            {
                return _lsmLastCalibrationDateUnix;
            }
            set
            {
                _lsmLastCalibrationDateUnix = value;
                OnPropertyChanged("LSMLastCalibrationDateUnix");
                OnPropertyChanged("LSMLastCalibrationDate");
            }
        }

        public int LSMLineScanEnable
        {
            get
            {
                return this._areaControlModel.LSMLineScanEnable;
            }
            set
            {
                this._areaControlModel.LSMLineScanEnable = value;
                OnPropertyChanged("LSMLineScanEnable");
                OnPropertyChanged("LSMFieldSizeDisplayX");
                OnPropertyChanged("LSMFieldSizeDisplayY");
                OnPropertyChanged("LSMFieldOffsetYDisplay");
                OnPropertyChanged("LSMFieldOffsetXDisplay");
                OnPropertyChanged("LSMFieldSizeYUM");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("FieldSizeHeightUM");
            }
        }

        public int LSMPixelX
        {
            get
            {
                return _areaControlModel.LSMPixelX;
            }
            set
            {

                //logic to allow clicking on the slider
                //and change the pixel density
                if ((0 != value % 32) && (int)ICamera.LSMType.RESONANCE_GALVO_GALVO != ResourceManagerCS.GetLSMType())
                {
                    int currentPixelX = _areaControlModel.LSMPixelX;
                    if (16 > Math.Abs(value - currentPixelX))
                    {
                        return;
                    }

                    value = value >> 5; // divide by 32
                    value = value << 5; // multiply by 32
                }

                if (value <= LSMPixelXMax && value >= LSMPixelXMin)
                {
                    this._areaControlModel.LSMPixelX = value;

                    if (0 >= value)
                    {
                        return;
                    }

                    //:TODO: This should be checked in the lower level. It would be easier to follow.
                    //double minDwellTime = (double)MVMManager.Instance["ScanControlViewModel", "MinDwellTimeFromTable", (object)0.0];

                    //if ((double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime", (object)0.0] < minDwellTime && !GGSuperUserMode)
                    //{
                    //    MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = Math.Round(minDwellTime, 2);
                    //}

                    OnPropertyChanged("CurrentResolution");
                    OnPropertyChanged("LSMPixelX");
                    OnPropertyChanged("LSMPixelY");
                    OnPropertyChanged("LSMPixelXMax");
                    OnPropertyChanged("LSMPixelYMax");

                    //check that the dwell time is still acceptable
                    //send the value back down to the device and update
                    double dwellVal = (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime", (object)0.0];
                    MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = dwellVal;

                    //check that the field size is still acceptable
                    //send the value back down to the device and update
                    //int val = this._areaControlModel.LSMFieldSize;
                    //int val = this.LSMFieldSize;
                    //this._areaControlModel.LSMFieldSize = val;
                    //this.LSMFieldSize = val;
                    OnPropertyChanged("LSMFieldSize");
                    OnPropertyChanged("LSMFieldSizeDisplayX");
                    OnPropertyChanged("LSMFieldSizeDisplayY");
                    OnPropertyChanged("LSMFieldOffsetYDisplay");
                    OnPropertyChanged("LSMFieldOffsetXDisplay");
                    OnPropertyChanged("LSMFieldSizeXUM");
                    ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("FieldSizeWidthUM");
                    OnPropertyChanged("LSMFieldSizeYUM");
                    ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("FieldSizeHeightUM");
                    OnPropertyChanged("LSMFieldSizeXMM");
                    OnPropertyChanged("LSMFieldSizeYMM");
                    OnPropertyChanged("LSMUMPerPixel");
                    OnPropertyChanged("MMPerPixel");
                    OnPropertyChanged("RectangleAreaModeVisibility");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMDwellTimeMinIndex");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMDwellTimeMaxIndex");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMFlybackCycles");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMFlybackTime");
                    ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
                    ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");
                    ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLSMPixelDensity");

                    bool modified = ConfirmAreaModeSettingsForGG;

                    if (TimeBasedLineScan)
                    {
                        //Whenever Pixel X is changed, recalculate pixel y
                        TimeBasedLSTimeMS = TimeBasedLSTimeMS;
                    }
                }
            }
        }

        public int LSMPixelXMax
        {
            get
            {
                return this._areaControlModel.LSMPixelXMax;
            }
            set
            {
                this._areaControlModel.LSMPixelXMax = value;
                OnPropertyChanged("LSMPixelXMax");
            }
        }

        public int LSMPixelXMin
        {
            get
            {
                return this._areaControlModel.LSMPixelXMin;
            }
        }

        public int LSMPixelY
        {
            get
            {
                return _areaControlModel.LSMPixelY;
            }
            set
            {
                int currentPixelY = _areaControlModel.LSMPixelY;
                if (value < 1)
                {
                    return;
                }
                int multiple = LSMPixelYMultiple;
                if (2 < value && 0 != value % multiple && (int)ICamera.LSMType.RESONANCE_GALVO_GALVO != ResourceManagerCS.GetLSMType())
                {
                    if (multiple == 32)
                    {
                        if (16 > Math.Abs(value - currentPixelY))
                        {
                            return;
                        }
                    }

                    if (value > _areaControlModel.LSMPixelY)
                    {
                        value /= multiple; // divide by 32
                        value *= multiple; // multiply by 32
                        value += multiple;
                    }
                    else
                    {
                        value /= multiple; // divide by 32
                        value *= multiple; // multiply by 32
                    }

                    if (2 == value && 1 == currentPixelY)
                    {
                        value = this.LSMPixelXMin;
                    }
                    else if (2 >= value)
                    {
                        value = 1;
                    }
                }

                //if value is 2 Always coerce to one
                //and let the logic below decide the final pixelY
                if (2 == value)
                {
                    value = 1;
                }

                const int TWO_WAY_SCAN = 0;

                if ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType() &&
                    (ICamera.LSMAreaMode.LINE == (ICamera.LSMAreaMode)LSMAreaMode ||
                    ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)LSMAreaMode))
                {
                    int channel = 0;
                    double dwellFactor = 0;
                    switch ((ICamera.LSMAreaMode)LSMAreaMode)
                    {
                        case ICamera.LSMAreaMode.LINE_TIMELAPSE:
                            {
                                int tempY = this._areaControlModel.LSMPixelY;
                                this._areaControlModel.LSMPixelY = Math.Max(tempY, this._areaControlModel.LSMPixelXMin);
                            }
                            break;
                        case ICamera.LSMAreaMode.LINE:
                            {
                                channel = (3 < (int)MVMManager.Instance["CaptureSetupViewModel", "LSMChannel", (object)0]) ? 4 : 1;
                                dwellFactor = 0.24; //Determined experimentally

                                if (1 == value && TWO_WAY_SCAN == (int)MVMManager.Instance["ScanControlViewModel", "LSMScanMode"])
                                {
                                    value = 2;
                                }
                            }
                            break;
                        case ICamera.LSMAreaMode.POLYLINE:
                            {
                                channel = 1; //channel buffer size has no effect for polyline acquisition. Thus, it is not being considered
                                dwellFactor = 0.72; //Determined experimentally
                            }
                            break;
                    }

                    int pixelX = this.LSMPixelX;
                    double newDwellTime = (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime", (object)0];
                    //Ensure th dwell time is long enough for single line scan
                    double m = value * newDwellTime * pixelX * channel / 1000; //ms
                    if (dwellFactor > m)
                    {
                        while (true)
                        {
                            newDwellTime += (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTimeStep", (object)0];
                            m = value * newDwellTime * pixelX * channel / 1000; //ms
                            if (dwellFactor <= m)
                            {
                                break;
                            }
                        }
                        if (!GGSuperUserMode)
                        {
                            MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = newDwellTime;
                        }
                    }
                }
                else if (ICamera.LSMAreaMode.RECTANGLE == (ICamera.LSMAreaMode)LSMAreaMode)
                {
                    if (value < LSMPixelYMin)
                    {
                        value = LSMPixelYMin;
                    }
                }
                this._areaControlModel.LSMPixelY = Math.Min(value, this._areaControlModel.LSMPixelYMax);

                OnPropertyChanged("CurrentResolution");
                OnPropertyChanged("LSMPixelY");
                OnPropertyChanged("LSMPixelXMax");
                OnPropertyChanged("LSMPixelYMax");
                OnPropertyChanged("LSMFieldSizeXUM");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("FieldSizeWidthUM");
                OnPropertyChanged("LSMFieldSizeYUM");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("FieldSizeHeightUM");
                OnPropertyChanged("LSMFieldSizeXMM");
                OnPropertyChanged("LSMFieldSizeYMM");
                OnPropertyChanged("LSMUMPerPixel");
                OnPropertyChanged("LSMFieldSizeDisplayX");
                OnPropertyChanged("LSMFieldSizeDisplayY");
                OnPropertyChanged("LSMFieldOffsetYDisplay");
                OnPropertyChanged("LSMFieldOffsetXDisplay");
                OnPropertyChanged("RectangleAreaModeVisibility");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMFlybackCycles");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMFlybackTime");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLSMPixelDensity");
            }
        }

        public int LSMPixelYMax
        {
            get
            {
                return this._areaControlModel.LSMPixelYMax;
            }
            set
            {
                this._areaControlModel.LSMPixelYMax = value;
                OnPropertyChanged("LSMPixelYMax");
            }
        }

        public int LSMPixelYMin
        {
            get
            {
                return this._areaControlModel.LSMPixelYMin;
            }
        }

        public int LSMPixelYMultiple
        {
            get
            {
                return _areaControlModel.LSMPixelYMultiple;
            }
        }

        public ICommand LSMSaveCalibrationCommand
        {
            get
            {
                if (this._lsmSaveCalibrationCommand == null)
                    this._lsmSaveCalibrationCommand = new RelayCommand(() => SaveCalibrationCommand());

                return this._lsmSaveCalibrationCommand;
            }
        }

        public double LSMScaleYScan
        {
            get
            {
                return this._areaControlModel.LSMScaleYScan;
            }

            set
            {
                this._areaControlModel.LSMScaleYScan = value;
                OnPropertyChanged("LSMScaleYScan");
            }
        }

        public double LSMScanAreaAngle
        {
            get
            {
                return Math.Round(_areaControlModel.LSMScanAreaAngle, 2);
            }
            set
            {
                _areaControlModel.LSMScanAreaAngle = value;
                OnPropertyChanged("LSMScanAreaAngle");
            }
        }

        public ICommand LSMScanAreaAngleMinusCommand
        {
            get
            {
                if (this._lsmScanAreaAngleMinusCommand == null)
                    this._lsmScanAreaAngleMinusCommand = new RelayCommand(() => ScanAreaAngleMinus());

                return this._lsmScanAreaAngleMinusCommand;
            }
        }

        public ICommand LSMScanAreaAnglePlusCommand
        {
            get
            {
                if (this._lsmScanAreaAnglePlusCommand == null)
                    this._lsmScanAreaAnglePlusCommand = new RelayCommand(() => ScanAreaAnglePlus());

                return this._lsmScanAreaAnglePlusCommand;
            }
        }

        public double LSMUMPerPixel
        {
            get
            {
                if ((int)ICamera.LSMAreaMode.POLYLINE != this.LSMAreaMode)
                {
                    double dVal = ((this.LSMFieldSize * this.LSMFieldSizeCalibration) / ((double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)0.0] * this.LSMPixelX));
                    _LSMUMPerPixel = Math.Round(dVal, 3);
                }

                return _LSMUMPerPixel;
            }
            set
            {
                _LSMUMPerPixel = value;
                OnPropertyChanged("LSMUMPerPixel");
            }
        }

        public string LSMZoom
        {
            get
            {
                if (this.LSMFieldSize <= 0)
                {
                    return string.Empty;
                }
                //else if (this.LSM1xFieldSize > 255)
                //{
                //    this.LSM1xFieldSize = 255;
                //}

                double val = (this._areaControlModel.LSM1xFieldSize / (double)this.LSMFieldSize);

                Decimal d = new Decimal(val);

                d = Decimal.Round(d, 1);

                return d.ToString() + "X";
            }
        }

        public ICommand LSMZoomMinusCommand
        {
            get
            {
                if (this._zoomMinusCommand == null)
                    this._zoomMinusCommand = new RelayCommand(() => ZoomMinus());

                return this._zoomMinusCommand;
            }
        }

        public ICommand LSMZoomPlusCommand
        {
            get
            {
                if (this._zoomPlusCommand == null)
                    this._zoomPlusCommand = new RelayCommand(() => ZoomPlus());

                return this._zoomPlusCommand;
            }
        }

        public int MesoMicroVisible
        {
            get
            {
                return ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType() && (int)ICamera.LSMType.RESONANCE_GALVO_GALVO == ResourceManagerCS.GetLSMType()) ?
                    (int)1 : (int)0;
            }
        }

        public int MesoStripPixels
        {
            get
            {
                return _mesoStripPixels;
            }
            set
            {
                _mesoStripPixels = value;
                OverlayManagerClass.Instance.PixelUnitSizeXY = new int[2] { _mesoStripPixels, (int)Constants.PIXEL_X_MIN };
                OverlayManagerClass.Instance.ValidateROIs(ref _overlayCanvas);
                OnPropertyChange("MesoStripPixels");
            }
        }

        public ICommand MesoStripPixelsMinusCommand
        {
            get
            {
                if (this._mesoStripPixelsMinusCommand == null)
                    this._mesoStripPixelsMinusCommand = new RelayCommand(() => MesoStripPixelsMinus());

                return this._mesoStripPixelsMinusCommand;
            }
        }

        public ICommand MesoStripPixelsPlusCommand
        {
            get
            {
                if (this._mesoStripPixelsPlusCommand == null)
                    this._mesoStripPixelsPlusCommand = new RelayCommand(() => MesoStripPixelsPlus());

                return this._mesoStripPixelsPlusCommand;
            }
        }

        public int[] MesoStripPixelsRange
        {
            get { return new int[2] { 64, 2048 }; }   // { min, max }
        }

        public int MesoStripPixelsStep
        {
            get { return (int)Constants.PIXEL_X_MIN; }
        }

        public List<int> MicroScanAreas
        {
            get { return OverlayManagerClass.Instance.GetModeSubPatternIDs(Mode.MICRO_SCANAREA); }
        }

        public double MMPerPixel
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == this._areaControlModel.CameraType)
                {
                    if ((0 == LSMPixelX) || (0 == (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)0]))
                        return 0;
                    return (LSMFieldSize * LSMFieldSizeCalibration) / (LSMPixelX * (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)0] * 1000.0);
                }
                else
                {
                    if (0 == (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)0])
                        return 0;
                    return (_areaControlModel.CamSensorPixelSizeUM / ((double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)0] * 1000));
                }
            }
        }

        public ICommand NyquistCommand
        {
            get
            {
                if (this._nyquistCommand == null)
                    this._nyquistCommand = new RelayCommand(() => Nyquist());

                return this._nyquistCommand;
            }
        }

        public int NyquistEmWavelength
        {
            get;
            set;
        }

        public int NyquistExWavelength
        {
            get;
            set;
        }

        public double NyquistIndexOfRefraction
        {
            get;
            set;
        }

        public double NyquistProjectedPinholeMagnification
        {
            get
            {
                const double SCANLENSFOCALLENGTH = 70;
                const double PINHOLECOLLECTORLENSFOCALLENGTH = 75;

                return PINHOLECOLLECTORLENSFOCALLENGTH / SCANLENSFOCALLENGTH;
            }
        }

        public Canvas OverviewOverlayCanvas
        {
            get
            {
                return _overviewOverlayCanvas;
            }
            set
            {
                _overviewOverlayCanvas = value;
            }
        }

        public bool OverviewVisible
        {
            get
            {
                return _overviewVisible;
            }
            set
            {
                _overviewVisible = value;
                if (_overviewVisible)
                {
                    if (null == _overviewWin)
                    {
                        _overviewWin = new MesoOverview();
                        _overviewWin.Closed += _overviewWin_Closed;
                        OverviewOverlayCanvas = _overviewWin.OverlayCanvas;
                        _overviewWin.Show();
                    }
                }
                else
                {
                    if (null != _overviewWin)
                    {
                        _overviewWin.Close();
                        _overviewWin = null;
                    }
                }
                OnPropertyChanged("OverviewVisible");
            }
        }

        public string PathBackgroundSubtraction
        {
            get
            {
                return _areaControlModel.PathBackgroundSubtraction;
            }
            set
            {
                _areaControlModel.PathBackgroundSubtraction = value;
                OnPropertyChanged("PathBackgroundSubtraction");
            }
        }

        public string PathFlatField
        {
            get
            {
                return _areaControlModel.PathFlatField;
            }
            set
            {
                _areaControlModel.PathFlatField = value;
                OnPropertyChanged("PathFlatField");
            }
        }

        public double PixelSizeUM
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == this._areaControlModel.CameraType)
                {
                    return this.LSMUMPerPixel;
                }
                else
                {
                    return (double)MVMManager.Instance["CameraControlViewModel", "CamPixelSizeUM", (object)1.0];
                }
            }
        }

        public Visibility PixelXSliderVibility
        {
            get
            {
                if (ICamera.LSMAreaMode.POLYLINE != (ICamera.LSMAreaMode)LSMAreaMode)
                {
                    return Visibility.Visible;
                }
                else
                {
                    return Visibility.Collapsed;
                }
            }
        }

        public Visibility PixelYSliderVibility
        {
            get
            {
                if (ICamera.LSMAreaMode.SQUARE != (ICamera.LSMAreaMode)LSMAreaMode && true == IsRectangleAreaModeAvailable && false == TimeBasedLineScan)
                {
                    return Visibility.Visible;
                }
                else
                {
                    return Visibility.Collapsed;
                }
            }
        }

        public Visibility PolylineScanVisibility
        {
            get
            {
                if (ICamera.LSMAreaMode.POLYLINE != (ICamera.LSMAreaMode)LSMAreaMode)
                {
                    return Visibility.Collapsed;
                }
                else
                {
                    return Visibility.Visible;
                }
            }
        }

        public Canvas propOverlayCanvas
        {
            get
            {
                return _overlayCanvas;
            }
            set
            {
                _overlayCanvas = value;
            }
        }

        public Visibility RectangleAreaModeVisibility
        {
            get
            {
                if (true == IsRectangleAreaModeAvailable)
                {
                    return Visibility.Visible;
                }
                else
                {
                    return Visibility.Collapsed;
                }
            }
        }

        public ICommand ResolutionAddCommand
        {
            get
            {
                if (this._resolutionAddCommand == null)
                    this._resolutionAddCommand = new RelayCommand(() => ResolutionAdd());

                return this._resolutionAddCommand;
            }
        }

        public List<ImageResolution> ResolutionPresets
        {
            get
            {
                return _areaControlModel.GetResolutionPresetsFromApplicationSettings((ICamera.LSMAreaMode)LSMAreaMode, IsRectangleAreaModeAvailable);
            }
        }

        public ICommand ReturnToOriginalAreaCommand
        {
            get
            {
                if (_returnToOriginalAreaCommand == null)
                    _returnToOriginalAreaCommand = new RelayCommand(() => ReturnToOriginalArea());

                return _returnToOriginalAreaCommand;
            }
        }

        public string ROIFrameRate
        {
            get { return string.Empty; }
        }

        public ICommand RoiZoomInCommand
        {
            get
            {
                if (this._roiZoomInCommand == null)
                    this._roiZoomInCommand = new RelayCommand(() => RoiZoomIn());

                return this._roiZoomInCommand;
            }
        }

        public int RSInitMode
        {
            get
            {
                return this._areaControlModel.RSInitMode;
            }
            set
            {
                this._areaControlModel.RSInitMode = value;
                OnPropertyChanged("RSInitMode");
            }
        }

        public int RSLineConfigured
        {
            get
            {
                return this._areaControlModel.RSLineConfigured;
            }
        }

        public bool RSLineProbeOn
        {
            get
            {
                return this._areaControlModel.RSLineProbeOn;
            }
            set
            {
                this._areaControlModel.RSLineProbeOn = value;
                OnPropertyChanged("RSLineProbeOn");
            }
        }

        public double RSLineRate
        {
            get
            {
                return (double)Math.Round(this._areaControlModel.RSLineRate / (double)Constants.MS_TO_SEC, (int)5);
            }
        }

        public bool RSLineVisible
        {
            get
            {
                int cameraType = (int)ICamera.CameraType.LAST_CAMERA_TYPE, lsmType = (int)ICamera.LSMType.LSMTYPE_LAST;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_TYPE, ref cameraType);
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);
                return (0 < RSLineConfigured && (int)ICamera.CameraType.LSM == cameraType && (int)ICamera.LSMType.GALVO_GALVO != lsmType) ?
                    true : false;
            }
        }

        public ICommand SelectBackgroundCommand
        {
            get
            {
                if (this._selectBackgroundCommand == null)
                    this._selectBackgroundCommand = new RelayCommand(() => SelectBackground());

                return this._selectBackgroundCommand;
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
            }
        }

        public int SelectedStripSize
        {
            get
            {
                return _selectedStripSize;
            }
            set
            {
                _selectedStripSize = value;
                OnPropertyChange("SelectedStripSize");
            }
        }

        public int SelectedViewMode
        {
            get
            {
                if (0 == MesoMicroVisible)
                {
                    return 0;
                }
                return _selectedViewMode;
            }
            set
            {
                _selectedViewMode = (0 == MesoMicroVisible) ? 0 : value;
                OnPropertyChange("MicroScanAreas");
                if (0 == _selectedViewMode)
                {
                    ConfigMicroScanArea = false;
                    MVMManager.Instance["CaptureSetupViewModel", "WrapPanelEnabled"] = true;
                }
                else
                {
                    MVMManager.Instance["CaptureSetupViewModel", "WrapPanelEnabled"] = (0 >= MicroScanAreas.Count) ? false : true;
                }
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("PreviewButtonEnabled");

                if (1 == MesoMicroVisible)
                    MVMManager.Instance["MesoScanViewModel", "ScanID"] = _selectedViewMode + (int)MesoScanTypes.Meso;

                OnPropertyChange("SelectedViewMode");
                OnPropertyChange("SelectedScanArea");
                OnPropertyChange("StripVisible");
            }
        }

        public ICommand SelectFlatFieldCommand
        {
            get
            {
                if (this._selectFlatFieldCommand == null)
                    this._selectFlatFieldCommand = new RelayCommand(() => SelectFlatField());

                return this._selectFlatFieldCommand;
            }
        }

        public ICommand StoreRSRateCommand
        {
            get
            {
                if (this._storeRSRateCommand == null)
                    this._storeRSRateCommand = new RelayCommand(() => StoreRSRate());

                return this._storeRSRateCommand;
            }
        }

        public List<int> StripSizes
        {
            get
            {
                return _stripSizes;
            }
        }

        public bool StripVisible
        {
            get
            {
                if (1 == MesoMicroVisible)
                {
                    if (0 == SelectedViewMode)
                        return true;
                    else if (ConfigMicroScanArea)
                        return true;
                }
                return false;
            }
        }

        public bool TimeBasedLineScan
        {
            get
            {
                return this._areaControlModel.TimeBasedLineScan;
            }
            set
            {
                this._areaControlModel.TimeBasedLineScan = value;
                EnableResolutionPresets = !value;
                //Save the current state of averaging and flyback cycles before disabling them
                _previousAverageState = (int)MVMManager.Instance["ScanControlViewModel", "LSMSignalAverage"];
                _previousAlwaysUseFastestState = (bool)MVMManager.Instance["ScanControlViewModel", "UseFastestSettingForFlybackCycles"];
                if (value)
                {
                    //Recalculate the number of lines for the time based line scan
                    TimeBasedLSTimeMS = TimeBasedLSTimeMS;
                    //Set cumulative to NONE and enable 'always use fastest' for flyback cycles
                    MVMManager.Instance["ScanControlViewModel", "LSMSignalAverage"] = 0;
                    MVMManager.Instance["ScanControlViewModel", "UseFastestSettingForFlybackCycles"] = true;
                    MVMManager.Instance["ScanControlViewModel", "LSMAverageEnabled"] = false;
                    MVMManager.Instance["ScanControlViewModel", "UseFastestFlybackEnabled"] = false;
                }
                else
                {
                    MVMManager.Instance["ScanControlViewModel", "LSMSignalAverage"] = _previousAverageState;
                    MVMManager.Instance["ScanControlViewModel", "UseFastestSettingForFlybackCycles"] = _previousAlwaysUseFastestState;
                    MVMManager.Instance["ScanControlViewModel", "LSMAverageEnabled"] = true;
                    MVMManager.Instance["ScanControlViewModel", "UseFastestFlybackEnabled"] = true;
                    if (LSMPixelX < LSMPixelY)
                    {
                        LSMPixelY = LSMPixelX;
                    }
                }
                OnPropertyChanged("TimeBasedLineScan");
                OnPropertyChanged("LSMPixelY");
                OnPropertyChanged("PixelYSliderVibility");
                OnPropertyChanged("CurrentResolution");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("IVScrollbarVisibility");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMAverageEnabled");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("UseFastestFlybackEnabled");
            }
        }

        public double TimeBasedLSTimeMS
        {
            get
            {
                return _areaControlModel.TimeBasedLSTimeMS;
            }
            set
            {
                _areaControlModel.TimeBasedLSTimeMS = value;
                OnPropertyChanged("TimeBasedLSTimeMS");
                OnPropertyChanged("LSMPixelY");
                OnPropertyChanged("CurrentResolution");
            }
        }

        public ICommand TimeBasedLSTimeMSMinusCommand
        {
            get
            {
                if (this._timeBasedLSTimeMSMinusCommand == null)
                    this._timeBasedLSTimeMSMinusCommand = new RelayCommand(() => TimeBasedLSTimeMSMinus());

                return this._timeBasedLSTimeMSMinusCommand;
            }
        }

        public ICommand TimeBasedLSTimeMSPlusCommand
        {
            get
            {
                if (this._timeBasedLSTimeMSPlusCommand == null)
                    this._timeBasedLSTimeMSPlusCommand = new RelayCommand(() => TimeBasedLSTimeMSPlus());

                return this._timeBasedLSTimeMSPlusCommand;
            }
        }

        public Visibility TimedBasedVisiblity
        {
            get
            {
                if (_timedBasedVisibility && !ResourceManagerCS.Instance.IsThorDAQBoard && ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType()) && 
                    ((int)ICamera.LSMAreaMode.LINE == LSMAreaMode || (int)ICamera.LSMAreaMode.POLYLINE == LSMAreaMode || (int)ICamera.LSMAreaMode.LINE_TIMELAPSE == LSMAreaMode))
                {
                    return Visibility.Visible;
                }
                TimeBasedLineScan = false;
                return Visibility.Collapsed;
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

        public int GetLSMFieldSizeCalibration(ref double calibration)
        {
            return this._areaControlModel.LIGetFieldSizeCalibration(ref calibration);
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(AreaControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            string str = string.Empty;
            int iTmp = 0;
            double dTmp = 0.0;
            Int64 i64Tmp = 0;

            //load from hardware settings first, then load exp
            XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/LSM");

            if (ndList.Count > 0)
            {
                if (XmlManager.GetAttribute(ndList[0], hardwareDoc, "pixelXLimit", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    LSMPixelXMax = iTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], hardwareDoc, "pixelYLimit", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    LSMPixelYMax = iTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], hardwareDoc, "field2Theta", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    LSMField2Theta = dTmp;
                }
            }

            //load from hardware settings first, then load exp
            XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView/FieldFromROIZoom");
            if (ndList.Count > 0)
            {
                str = string.Empty;
                XmlManager.GetAttribute(ndList[0], appDoc, "value", ref str);

                FieldFromROIZoomMode = ("1" == str || Boolean.TrueString == str) ? true : false;
            }

            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView/GGSuperUser");
            if (ndList.Count > 0)
            {
                str = string.Empty;
                XmlManager.GetAttribute(ndList[0], appDoc, "value", ref str);

                GGSuperUserMode = ("1" == str || Boolean.TrueString == str) ? true : false;
            }

            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView/TimeBasedScan");
            if (ndList.Count > 0)
            {
                str = string.Empty;
                XmlManager.GetAttribute(ndList[0], appDoc, "Visibility", ref str);

                _timedBasedVisibility = ("Visible" == str || "visible" == str);
                OnPropertyChanged("TimedBasedVisiblity");
            }

            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            ndList = doc.SelectNodes("/ThorImageExperiment/LSM");

            if (ndList.Count > 0)
            {

                if (XmlManager.GetAttribute(ndList[0], doc, "timeBasedLineScan", ref str))
                {
                    TimeBasedLineScan = ("1" == str || Boolean.TrueString == str);
                    _previousTimebasedSelection = TimeBasedLineScan;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "areaMode", ref str))
                {
                    int areaMode = 0;
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        areaMode = tmp;
                    }
                    //If LSMAreaMode == LineScan (single line)
                    if (ICamera.LSMAreaMode.LINE == (ICamera.LSMAreaMode)areaMode ||
                        ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)areaMode)
                    {
                        LSMAreaMode = ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType()) ? areaMode : 0;
                    }
                    else
                    {
                        LSMAreaMode = areaMode;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "pixelSizeUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    LSMUMPerPixel = dTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "fieldSize", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    LSMFieldSize = iTmp;
                }
                if ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType())
                {
                    if (XmlManager.GetAttribute(ndList[0], doc, "areaAngle", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                    {
                        LSMScanAreaAngle = dTmp;
                    }
                }
                else
                {
                    LSMScanAreaAngle = 0;
                }

                LockFieldOffset = false;
                if (XmlManager.GetAttribute(ndList[0], doc, "offsetX", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    LSMFieldOffsetXActual = (int)dTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "offsetY", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    LSMFieldOffsetYActual = (int)dTmp;
                }

                //if there is no offset then lock the field offset for x and y to be zero
                if (0 == LSMFieldOffsetXActual && 0 == LSMFieldOffsetYActual)
                {
                    LockFieldOffset = false;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "horizontalFlip", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    LSMFlipHorizontal = iTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "verticalFlip", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    LSMFlipVerticalScan = iTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "fineOffsetX", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    LSMFieldOffsetXFine = dTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "fineOffsetY", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    LSMFieldOffsetYFine = dTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "fineScaleX", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    LSMFieldScaleXFine = dTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "fineScaleY", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    LSMFieldScaleYFine = dTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "tbLineScanTimeMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    TimeBasedLSTimeMS = dTmp;
                }
            }

            ndList = doc.SelectNodes("/ThorImageExperiment/ImageCorrection");

            if (ndList.Count > 0)
            {
                if (XmlManager.GetAttribute(ndList[0], doc, "enablePincushion", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    EnablePincushionCorrection = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "pinCoeff1", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    Coeff1 = dTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "pinCoeff2", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    Coeff2 = dTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "pinCoeff3", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    Coeff3 = dTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "enableBackgroundSubtraction", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    EnableBackgroundSubtraction = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "pathBackgroundSubtraction", ref str))
                {
                    PathBackgroundSubtraction = str;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "enableFlatField", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    EnableFlatField = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "pathFlatField", ref str))
                {
                    PathFlatField = str;
                }
            }

            double magnification = 1;

            ndList = doc.SelectNodes("/ThorImageExperiment/Magnification");

            if (ndList.Count > 0)
            {
                if (XmlManager.GetAttribute(ndList[0], doc, "mag", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    magnification = dTmp;
                }

                NyquistIndexOfRefraction = 1.0;
                if (XmlManager.GetAttribute(ndList[0], doc, "indexOfRefraction", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    NyquistIndexOfRefraction = dTmp;
                }
            }

            ndList = doc.SelectNodes("/ThorImageExperiment/Wavelengths");

            if (ndList.Count > 0)
            {
                NyquistExWavelength = 488;
                if (XmlManager.GetAttribute(ndList[0], doc, "nyquistExWavelengthNM", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    NyquistExWavelength = iTmp;
                }
                NyquistEmWavelength = 488;
                if (XmlManager.GetAttribute(ndList[0], doc, "nyquistEmWavelengthNM", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    NyquistEmWavelength = iTmp;
                }
            }

            ndList = doc.SelectNodes("/ThorImageExperiment/Photobleaching");

            if (XmlManager.GetAttribute(ndList[0], doc, "calibrationDateUnix", ref str) && Int64.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out i64Tmp))
            {
                LSMLastCalibrationDateUnix = i64Tmp;
            }

            //Update Waveform for Polyline Scan
            if (ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)LSMAreaMode)
            {
                ReloadPolylineScan();
            }

            OnPropertyChange("");

            if (RSLineVisible)
            { _statusTimer.Start(); }
            else
            { _statusTimer.Stop(); }

            //After the modality is changed we need to rebuild the zoom table. Set _zoomTable[0] = 0 to rebuild it.
            _zoomTable[0] = 0;
        }

        public void OnPropertyChange(string propertyName)
        {
            if ((null != GetPropertyInfo(propertyName)) || (string.Empty == propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        public void ReloadPolylineScan()
        {
            if (ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)LSMAreaMode)
            {
                double dweltime = (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime", (object)0];
                int pixelY = this.LSMPixelY;
                ReturnToOriginalArea();
                RoiZoomIn();
                this.LSMPixelY = pixelY;
                MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = dweltime;
            }
        }

        /// <summary>
        /// Try to set LSM field size, return old value if not valid.
        /// </summary>
        /// <param name="setVal"></param>
        /// :TODO: This wont be needed anymore. This is handled in the lower level
        //public void TrySetLSMFieldSize(int setVal)
        //{
        //    int originalVal = this.LSMFieldSize;
        //    this.LSMFieldSize = setVal;
        //    if (setVal != this.LSMFieldSize)
        //    {
        //        this.LSMFieldSize = originalVal;
        //    }
        //}
        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            ConfigMicroScanArea = false;

            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/LSM");

            if (ndList.Count > 0)
            {
                XmlNode node = ndList[0];

                XmlManager.SetAttribute(ndList[0], experimentFile, "pixelX", this.LSMPixelX.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "pixelY", this.LSMPixelY.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "areaMode", this.LSMAreaMode.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "fieldSize", this.LSMFieldSize.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "offsetX", LSMFieldOffsetXActual.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "offsetY", LSMFieldOffsetYActual.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "areaAngle", this.LSMScanAreaAngle.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "widthUM", this.LSMFieldSizeXUM.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "heightUM", this.LSMFieldSizeYUM.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "pixelSizeUM", this.LSMUMPerPixel.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "horizontalFlip", this.LSMFlipHorizontal.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "verticalFlip", this.LSMFlipVerticalScan.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "fineOffsetX", this.LSMFieldOffsetXFine.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "fineOffsetY", this.LSMFieldOffsetYFine.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "fineScaleX", this.LSMFieldScaleXFine.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "fineScaleY", this.LSMFieldScaleYFine.ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "timeBasedLineScan", (Convert.ToInt32(this.TimeBasedLineScan)).ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "tbLineScanTimeMS", this.TimeBasedLSTimeMS.ToString());
            }

            ndList = experimentFile.SelectNodes("/ThorImageExperiment/Magnification");

            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], experimentFile, "indexOfRefraction", this.NyquistIndexOfRefraction.ToString());
            }

            ndList = experimentFile.SelectNodes("/ThorImageExperiment/ImageCorrection");

            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], experimentFile, "enablePincushion", this.EnablePincushionCorrection.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "pinCoeff1", this.Coeff1.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "pinCoeff2", this.Coeff2.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "pinCoeff3", this.Coeff3.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "enableBackgroundSubtraction", this.EnableBackgroundSubtraction.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "pathBackgroundSubtraction", this.PathBackgroundSubtraction.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "enableFlatField", this.EnableFlatField.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "pathFlatField", this.PathFlatField.ToString());
            }
        }

        private void BuildZoomTable()
        {
            if (0 == _zoomTable[0])
            {
                _zoomTable[0] = Convert.ToInt32(Math.Min(LSM1xFieldSize * 2.0, LSMFieldSizeMax));
                _zoomTable[1] = LSM1xFieldSize;
            }
            for (int i = 2; i < ZOOM_TABLE_LENGTH; i++)
            {
                _zoomTable[i] = _zoomTable[i - 1] / 2;
            }
            ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
            ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");
        }

        private double CalculateFieldOffsetXDisplay(int pixelX, int pixelY, int fieldSize)
        {
            if ((pixelX != 0) || (pixelY != 0))
            {
                double xyRatio = (double)pixelY / (double)pixelX;
                return fieldSize / (Math.Sqrt(1 + (xyRatio * xyRatio)) * 2.0);
            }
            return 0;
        }

        private double CalculateFieldOffsetYDisplay(int pixelX, int pixelY, int fieldSize)
        {
            if ((pixelX != 0) || (pixelY != 0))
            {
                double xyRatio = (double)pixelY / (double)pixelX;
                return fieldSize / (Math.Sqrt(1 + 1 / (xyRatio * xyRatio)) * 2.0);
            }
            return 0;
        }

        private void CenterROI()
        {
            this._areaControlModel.CenterROI();
            this.LSMFieldOffsetXActual = 0;
            this.LSMFieldOffsetYActual = 0;
        }

        private void CenterScanners(object type)
        {
            //keep last scan mode for later imaging
            if (ICamera.ScanMode.FORWARD_SCAN >= (ICamera.ScanMode)MVMManager.Instance["ScanControlViewModel", "LSMScanMode", (object)0])
                MVMManager.Instance["ScanControlViewModel", "LastLSMScanMode"] = (int)MVMManager.Instance["ScanControlViewModel", "LSMScanMode", (object)0];

            switch ((SelectedHardware)type)
            {
                case SelectedHardware.SELECTED_CAMERA1:
                    RSInitMode = 0;
                    this._areaControlModel.CenterScanners((int)SelectedHardware.SELECTED_CAMERA1, false);
                    break;
                case SelectedHardware.SELECTED_BLEACHINGSCANNER:
                    this._areaControlModel.CenterScanners((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, false);
                    break;
            }
        }

        private void CloseShutter()
        {
            this._areaControlModel.CloseShutter();
        }

        private void DwellTimeMinus()
        {
            double val = (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime", (object)0] - (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTimeStep", (object)0];

            MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = Math.Round(val, 1, MidpointRounding.AwayFromZero);
        }

        private void DwellTimePlus()
        {
            double val = (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime", (object)0] + (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTimeStep", (object)0];

            MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = Math.Round(val, 1, MidpointRounding.AwayFromZero);
        }

        private void FieldOffsetFineReset()
        {
            this.LSMFieldOffsetXFine = 0;
            this.LSMFieldOffsetYFine = 0;
        }

        private void FieldOffsetXFineMinus()
        {
            this.LSMFieldOffsetXFine -= .001;
        }

        private void FieldOffsetXFinePlus()
        {
            this.LSMFieldOffsetXFine += .001;
        }

        private void FieldOffsetXMinus()
        {
            this.LSMFieldOffsetXActual -= 1;
        }

        private void FieldOffsetXPlus()
        {
            this.LSMFieldOffsetXActual += 1;
        }

        private void FieldOffsetYFineMinus()
        {
            this.LSMFieldOffsetYFine -= .001;
        }

        private void FieldOffsetYFinePlus()
        {
            this.LSMFieldOffsetYFine += .001;
        }

        private void FieldOffsetYMinus()
        {
            this.LSMFieldOffsetYActual -= 1;
        }

        private void FieldOffsetYPlus()
        {
            this.LSMFieldOffsetYActual += 1;
        }

        private void FieldScaleFineReset()
        {
            this.LSMFieldScaleXFine = 1;
            this.LSMFieldScaleYFine = 1;
        }

        private void FieldScaleXFineMinus()
        {
            this.LSMFieldScaleXFine -= .001;
        }

        private void FieldScaleXFinePlus()
        {
            this.LSMFieldScaleXFine += .001;
        }

        private void FieldScaleYFineMinus()
        {
            this.LSMFieldScaleYFine -= .001;
        }

        private void FieldScaleYFinePlus()
        {
            this.LSMFieldScaleYFine += .001;
        }

        private void FieldSizeMinus()
        {
            if (_areaControlModel.LSMFieldSize > _areaControlModel.LSMFieldSizeMin)
            {
                this.LSMFieldSize--;
                OnPropertyChanged("LSMFieldSize");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");
            }
        }

        private void FieldSizePlus()
        {
            if (_areaControlModel.LSMFieldSize < _areaControlModel.LSMFieldSizeMax)
            {
                this.LSMFieldSize++;
                OnPropertyChanged("LSMFieldSize");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");
            }
        }

        private int[] FindFieldSizeInZoomTable(int prevOrNext)
        {
            int[] val = new int[2];

            //previous
            if (0 == prevOrNext)
            {
                val[0] = 0;
                val[1] = _zoomTable[val[0]];
                bool previousZoomFound = false;
                for (int i = 1; i < ZOOM_TABLE_LENGTH - 1; i++)
                {
                    if ((this.LSMFieldSize <= _zoomTable[i]) && (this.LSMFieldSize > _zoomTable[i + 1]))
                    {
                        val[0] = i - 1;
                        val[1] = _zoomTable[i - 1];
                        previousZoomFound = true;
                        break;
                    }
                }

                //if the next zoom wasn't found
                if (!previousZoomFound)
                {
                    //check if the field size is exactly the last one in the table
                    if ((this.LSMFieldSize == _zoomTable[ZOOM_TABLE_LENGTH - 1]))
                    {
                        val[0] = ZOOM_TABLE_LENGTH - 2;
                        val[1] = _zoomTable[ZOOM_TABLE_LENGTH - 2];
                        previousZoomFound = true;
                        return val;
                    }

                    // if no value was found then check if the current value is outside
                    //of the zoom table values (smaller than all)
                    bool fieldSizeSmallerThanAllTableValues = true;
                    for (int i = 0; i < ZOOM_TABLE_LENGTH; i++)
                    {
                        if ((this.LSMFieldSize > _zoomTable[i]))
                        {
                            fieldSizeSmallerThanAllTableValues = false;
                        }
                    }

                    //if its outside of the zoom table values then force the value to be the first zoom value from the zoom table
                    if (fieldSizeSmallerThanAllTableValues)
                    {
                        val[0] = 1;
                        val[1] = _zoomTable[ZOOM_TABLE_LENGTH - 1];

                        return val;
                    }

                    // if no value was found then check if the current value is outside
                    //of the zoom table values  (larger than all)
                    bool fieldSizeGreaterThanAllTableValues = true;
                    for (int i = 0; i < ZOOM_TABLE_LENGTH; i++)
                    {
                        if ((this.LSMFieldSize < _zoomTable[i]))
                        {
                            fieldSizeGreaterThanAllTableValues = false;
                        }
                    }

                    //if its outside of the zoom table values and we are trying to zoom in then keep it at its location
                    if (fieldSizeGreaterThanAllTableValues)
                    {
                        val[0] = 1;
                        val[1] = this.LSMFieldSize;

                        return val;
                    }
                }
            }
            else
            {
                val[0] = ZOOM_TABLE_LENGTH - 1;
                val[1] = _zoomTable[ZOOM_TABLE_LENGTH - 1];
                //next

                bool nextZoomFound = false;
                for (int i = 0; i < ZOOM_TABLE_LENGTH - 1; i++)
                {
                    if ((this.LSMFieldSize <= _zoomTable[i]) && (this.LSMFieldSize > _zoomTable[i + 1]))
                    {
                        val[0] = i + 1;
                        val[1] = _zoomTable[i + 1];
                        nextZoomFound = true;
                        break;
                    }
                }

                //if the next zoom wasn't found
                if (!nextZoomFound)
                {
                    // if no value was found then check if the current value is outside
                    //of the zoom table values  (larger than all)
                    bool fieldSizeGreaterThanAllTableValues = true;
                    for (int i = 0; i < ZOOM_TABLE_LENGTH; i++)
                    {
                        if ((this.LSMFieldSize < _zoomTable[i]))
                        {
                            fieldSizeGreaterThanAllTableValues = false;
                        }
                    }
                    //if its outside of the zoom table values then force the value to be the first zoom value from the zoom table
                    if (fieldSizeGreaterThanAllTableValues)
                    {
                        val[0] = 1;
                        val[1] = _zoomTable[0];

                        return val;
                    }

                    // if no value was found then check if the current value is outside
                    //of the zoom table values  (smaller than all)
                    bool fieldSizeSmallerThanAllTableValues = true;
                    for (int i = 0; i < ZOOM_TABLE_LENGTH; i++)
                    {
                        if ((this.LSMFieldSize > _zoomTable[i]))
                        {
                            fieldSizeSmallerThanAllTableValues = false;
                        }
                    }

                    //if its outside of the zoom table values and we are trying to zoom in then keep it at its location
                    if (fieldSizeSmallerThanAllTableValues)
                    {
                        val[0] = 1;
                        val[1] = this.LSMFieldSize;

                        return val;
                    }
                }
            }

            return val;
        }

        private double GetPixelsPerInch()
        {
            double pixelsPerInch = 96;

            //the hardware doc is valid and the camera type is LSM
            const int LSM_CAMERA_TYPE = 1;
            if (this._areaControlModel.CameraType == LSM_CAMERA_TYPE)
            {
                double magnification = 10.0;

                XmlNodeList ndList = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS].SelectNodes("/HardwareSettings/Objectives/Objective");

                if (ndList.Count >= (int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0])
                {
                    XmlNode node = ndList[(int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0]].Attributes.GetNamedItem("mag");

                    if (node != null)
                    {
                        magnification = Convert.ToDouble(node.Value, CultureInfo.InvariantCulture);
                    }
                }

                double umPerPixel = (LSMFieldSize * LSMFieldSizeCalibration) / (LSMPixelX * magnification);

                const double UM_PER_INCH_CONVERSION = 25399.99999997256800000002962656;

                pixelsPerInch = UM_PER_INCH_CONVERSION / umPerPixel;
            }
            return pixelsPerInch;
        }

        private void ImagingCenterScanners(object type)
        {
            switch ((SelectedHardware)type)
            {
                case SelectedHardware.SELECTED_BLEACHINGSCANNER:
                    if (null != (double[])MVMManager.Instance["CaptureSetupViewModel", "BleachCalibrateFineOffsetXY", (object)0])
                    {
                        MVMManager.Instance["CaptureSetupViewModel", "BleachLSMOffsetXYFine"] = MVMManager.Instance["CaptureSetupViewModel", "BleachCalibrateFineOffsetXY", (object)0];
                        this._areaControlModel.CenterScanners((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, true);
                    }
                    break;
            }
        }

        private void LoadZoomData()
        {
            if (File.Exists("./ZoomData.txt"))
            {
                StreamReader sr = new StreamReader("./ZoomData.txt", System.Text.Encoding.ASCII, false);

                _useCalibrationArray = true;

                for (int i = 0; i < 256; i++)
                {
                    try
                    {
                        string str = sr.ReadLine();
                        _calibrationArray[i] = Convert.ToInt32(str) / 100.0;
                    }
                    catch (SystemException ex)
                    {
                        _useCalibrationArray = false;
                        ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, ex.Message);
                    }
                }

                //field size 120 is used in the calibration
                //adjust the calibration values to scale from the 120 value
                if (_useCalibrationArray)
                {
                    double offset = _calibrationArray[120];

                    for (int i = 0; i < 256; i++)
                    {
                        _calibrationArray[i] = _calibrationArray[i] - offset;
                    }
                }

                sr.Close();
            }
        }

        private void MesoStripPixelsMinus()
        {
            MesoStripPixels = Math.Max(MesoStripPixelsRange[0], MesoStripPixels - MesoStripPixelsStep);
        }

        private void MesoStripPixelsPlus()
        {
            MesoStripPixels = Math.Min(MesoStripPixelsRange[1], MesoStripPixels + MesoStripPixelsStep);
        }

        private void Nyquist()
        {
            double magnification = 20.0;
            double na = 1.0;

            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

            XmlNodeList ndList = doc.SelectNodes("/HardwareSettings/Objectives/Objective");

            string str = string.Empty;

            if (ndList.Count >= (int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0])
            {
                XmlManager.GetAttribute(ndList[(int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0]], doc, "mag", ref str);

                magnification = Convert.ToDouble(str, CultureInfo.InvariantCulture);

                XmlManager.GetAttribute(ndList[(int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0]], doc, "na", ref str);

                na = Convert.ToDouble(str, CultureInfo.InvariantCulture);
            }

            double umPerPixel = (LSMFieldSize * LSMFieldSizeCalibration) / (LSMPixelX * magnification);

            NyquistCalculator dlg = new NyquistCalculator();

            dlg.NumericalAperture = na;
            dlg.IndexOfRefraction = NyquistIndexOfRefraction;
            dlg.PinholeSizeUM = (double)MVMManager.Instance["PinholeControlViewModel", "PinholeSizeUM", (object)0.0];
            dlg.Magnification = magnification;
            dlg.FieldSize = LSMFieldSize;
            dlg.FieldSizeCalibration = LSMFieldSizeCalibration;
            dlg.AspectRatio = (double)LSMPixelY / (double)LSMPixelX;
            dlg.ExcitationWavelength = NyquistExWavelength;
            dlg.EmissionWavelength = NyquistEmWavelength;
            dlg.LsmPixelXMax = LSMPixelXMax;
            dlg.LsmPixelYMax = LSMPixelYMax;

            double pinholeSize = 0;
            switch (Convert.ToInt32(MVMManager.Instance["PinholeControlViewModel", "PinholePosition", (object)0]))
            {
                case 0: pinholeSize = 25; break;
                case 1: pinholeSize = 30; break;
                case 2: pinholeSize = 35; break;
                case 3: pinholeSize = 40; break;
                case 4: pinholeSize = 45; break;
                case 5: pinholeSize = 50; break;
                case 6: pinholeSize = 60; break;
                case 7: pinholeSize = 70; break;
                case 8: pinholeSize = 80; break;
                case 9: pinholeSize = 90; break;
                case 10: pinholeSize = 100; break;
                case 11: pinholeSize = 125; break;
                case 12: pinholeSize = 200; break;
                case 13: pinholeSize = 300; break;
                case 14: pinholeSize = 1000; break;
                case 15: pinholeSize = 2000; break;
                default: pinholeSize = 0; break;
            }
            dlg.PinholeSizeUM = pinholeSize;

            if (true == dlg.ShowDialog())
            {
                NyquistIndexOfRefraction = dlg.IndexOfRefraction;
                NyquistExWavelength = dlg.ExcitationWavelength;
                NyquistEmWavelength = dlg.EmissionWavelength;

                switch (dlg.Modality)
                {
                    case 0:
                        {
                            if (!double.IsNaN(dlg.UMPerStepZConfocal) && (dlg.PixelXConfocal <= LSMPixelXMax) && (dlg.PixelYConfocal <= LSMPixelYMax))
                            {
                                LSMPixelX = dlg.PixelXConfocal;
                                LSMPixelY = dlg.PixelYConfocal;
                                MVMManager.Instance["ZControlViewModel", "ZScanStep"] = dlg.UMPerStepZConfocal;
                            }
                        }
                        break;
                    case 1:
                        {
                            if (!double.IsNaN(dlg.UMPerStepZ2P) && (dlg.PixelX2P <= LSMPixelXMax) && (dlg.PixelY2P <= LSMPixelYMax))
                            {
                                LSMPixelX = dlg.PixelX2P;
                                LSMPixelY = dlg.PixelY2P;
                                MVMManager.Instance["ZControlViewModel", "ZScanStep"] = dlg.UMPerStepZ2P;
                            }
                        }
                        break;
                }
            }
        }

        private void OverlayManager_ObjectSizeChangedEvent(double arg1, double arg2)
        {
            OverlayManagerClass.Instance.ValidateROIs(ref _overlayCanvas);
        }

        private void ResolutionAdd()
        {
            if (MessageBoxResult.Yes == MessageBox.Show(string.Format("Do you want to add the resolution {0} x {1}", LSMPixelX, LSMPixelY), "Add Resolution", MessageBoxButton.YesNo))
            {
                this._areaControlModel.ResolutionAdd();

                OnPropertyChanged("ResolutionPresets");
            }
        }

        private void ReturnToOriginalArea()
        {
            int areaMode = 0;
            int scanMode = 0;
            int offsetX = 0;
            int offsetY = 0;
            int fieldSize = 5;
            int pixelX = 32;
            int pixelY = 32;
            double areaAngle = 0;
            double dwellTime = (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTimeMin", (object)0];
            int interleaveScan = 0;

            if (true == OverlayManagerClass.Instance.LoadOriginalFieldAndROIs(ref areaMode, ref scanMode, ref fieldSize, ref offsetX, ref offsetY, ref pixelX, ref pixelY, ref areaAngle, ref dwellTime, ref interleaveScan, ref _overlayCanvas))
            {
                //The order in which each property is updated is critical
                //areaMode should be assigned first to allow the camera
                //to accept the other settings.
                this.LSMAreaMode = areaMode;
                MVMManager.Instance["ScanControlViewModel", "LSMScanMode"] = scanMode;
                MVMManager.Instance["ScanControlViewModel", "LSMInterleaveScan"] = interleaveScan;
                this.LSMPixelX = pixelX;
                this.LSMPixelY = pixelY;

                this.LSMFieldSize = fieldSize;

                this.LSMScanAreaAngle = areaAngle;
                MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = originalDwellTime;
                this.LSMFieldOffsetXActual = offsetX;
                this.LSMFieldOffsetYActual = offsetY;
            }
        }

        private void RoiZoomIn()
        {
            List<Point> points = new List<Point>();
            OverlayManagerClass.ROIType roiType = 0;
            if (OverlayManagerClass.Instance.QueryTheLastROIRange(ref points, ref roiType))
            {
                if ((int)ICamera.LSMAreaMode.RECTANGLE < this.LSMAreaMode)
                {
                    return;
                }
                MVMManager.Instance["CaptureSetupViewModel", "UnloadWholeStats"] = true;
                MVMManager.Instance["CaptureSetupViewModel", "UnloadWholeLineProfile"] = true;

                int originalAreaMode = this.LSMAreaMode;
                int originalScanMode = (int)MVMManager.Instance["ScanControlViewModel", "LSMScanMode", (object)0];
                int originalFieldSize = this.LSMFieldSize;
                int originalOffsetX = this.LSMFieldOffsetXActual;
                int originalOffsetY = this.LSMFieldOffsetYActual;
                int originalPixelX = this.LSMPixelX;
                int originalPixelY = this.LSMPixelY;
                double originalAngle = this.LSMScanAreaAngle;
                originalDwellTime = (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime", (object)0];
                int originalInterleaveScan = (int)MVMManager.Instance["ScanControlViewModel", "LSMInterleaveScan", (object)0];
                switch (roiType)
                {
                    case OverlayManagerClass.ROIType.RECTANGLE:
                        {
                            if (LockFieldOffset == true)
                            {
                                LockFieldOffset = false;
                            }
                            if (false == _areaControlModel.RoiZoomInRect(points[0], points[1]))
                            {
                                MessageBox.Show("Invalid range  to zoom in");
                                return;
                            }
                            OverlayManagerClass.Instance.NewFieldFromROI(originalAreaMode, originalScanMode, originalFieldSize, originalOffsetX, originalOffsetY, originalPixelX, originalPixelY, originalAngle, originalDwellTime, originalInterleaveScan, ref _overlayCanvas);

                            // Change Pixel Density for faster Scan Time
                            // Floor the X and Y values to get the Rectangle Size (no decimals)
                            if (true == FieldFromROIZoomMode)
                            {
                                LSMAreaMode = (int)ICamera.LSMAreaMode.RECTANGLE;

                                int x1 = (int)Math.Floor(points[0].X);
                                int x2 = (int)Math.Floor(points[1].X);
                                int y1 = (int)Math.Floor(points[0].Y);
                                int y2 = (int)Math.Floor(points[1].Y);

                                int tmpPixelX = x2 - x1;
                                int tmpPixelY = y2 - y1;

                                LSMPixelX = tmpPixelX;
                                LSMPixelY = tmpPixelY;
                            }
                        }
                        break;
                    case OverlayManagerClass.ROIType.LINE:
                        {
                            if (LockFieldOffset == true)
                            {
                                LockFieldOffset = false;
                            }

                            if (false == _areaControlModel.ROIZoomInLine(points[0], points[1]))
                            {
                                this.LSMScanAreaAngle = originalAngle;
                                MessageBox.Show("Invalid range  to zoom in");
                                return;
                            }
                            OverlayManagerClass.Instance.NewFieldFromROI(originalAreaMode, originalScanMode, originalFieldSize, originalOffsetX, originalOffsetY, originalPixelX, originalPixelY, originalAngle, originalDwellTime, originalInterleaveScan, ref _overlayCanvas);

                            //Floor the X and Y values to get the pixel location (no decimals)
                            double x1 = Math.Floor(points[0].X);
                            double x2 = Math.Floor(points[1].X);
                            double y1 = Math.Floor(points[0].Y);
                            double y2 = Math.Floor(points[1].Y);

                            double opp = y2 - y1;
                            double adj = x2 - x1;

                            //Math.Atan handles +-infinity cases
                            LSMScanAreaAngle += (1 == LSMFlipHorizontal) ? -Math.Atan(opp / adj) * 180.0 / Math.PI : Math.Atan(opp / adj) * 180.0 / Math.PI;
                            LSMAreaMode = (int)ICamera.LSMAreaMode.LINE;
                        }
                        break;
                    case OverlayManagerClass.ROIType.POLYLINE:
                        {
                            if ((int)ICamera.LSMType.GALVO_GALVO != ResourceManagerCS.GetLSMType())
                            {
                                return;
                            }
                            double umPerPixel = _LSMUMPerPixel;
                            if (false == _areaControlModel.ROIZoomInPolyline(points))
                            {
                                MessageBox.Show("Invalid range  to zoom in");
                                return;
                            }
                            OverlayManagerClass.Instance.NewFieldFromROI(originalAreaMode, originalScanMode, originalFieldSize, originalOffsetX, originalOffsetY, originalPixelX, originalPixelY, originalAngle, originalDwellTime, originalInterleaveScan, ref _overlayCanvas);
                            OnPropertyChanged("LSMPixelX");
                            LSMAreaMode = (int)ICamera.LSMAreaMode.POLYLINE;
                            bool modified = ConfirmAreaModeSettingsForGG;

                            _LSMUMPerPixel = umPerPixel;
                            OnPropertyChanged("LSMUMPerPixel");
                        }
                        break;
                }

                //ReticleBotton.IsChecked = false;

                //if (null != _multiROIStats)
                //{
                //    _multiROIStats.Close();
                //}

                //if (null != _roiStatsChart)
                //{
                //    _roiStatsChart.Close();
                //}

                //if (null != _lineProfile)
                //{
                //    _lineProfile.Close();
                //}

                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMScanMode");
                OnPropertyChanged("LSMFieldOffsetXActual");
                OnPropertyChanged("LSMFieldOffsetYActual");
                OnPropertyChanged("LSMFieldSize");
                OnPropertyChanged("LSMZoom");
                OnPropertyChanged("LSMFieldSizeXUM");
                OnPropertyChanged("LSMFieldSizeYUM");
                OnPropertyChanged("LSMScanAreaAngle");
                OnPropertyChanged("LSMFieldSizeDisplayX");
                OnPropertyChanged("LSMFieldSizeDisplayY");
                OnPropertyChanged("LSMFieldOffsetXDisplay");
                OnPropertyChanged("LSMFieldOffsetYDisplay");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMInterleaveScan");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLSMZoom");

                MVMManager.Instance["CaptureSetupViewModel", "UnloadWholeStats"] = false;
                MVMManager.Instance["CaptureSetupViewModel", "UnloadWholeLineProfile"] = false;
            }
        }

        private void SaveCalibrationCommand()
        {
            this.LSMLastCalibrationDateUnix = ResourceManagerCS.DateTimeToUnixTimestamp(DateTime.Now);

            //persist calibration as global params to all modalities:
            MVMManager.Instance["CaptureSetupViewModel", "PersistGlobalExperimentXMLNow"] = GlobalExpAttribute.GALVO_CALIBTATION;
        }

        private void ScanAreaAngleMinus()
        {
            this.LSMScanAreaAngle -= 1;
        }

        private void ScanAreaAnglePlus()
        {
            this.LSMScanAreaAngle += 1;
        }

        private void SelectBackground()
        {
            // Configure open file dialog box
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "Tif"; // Default file name
            dlg.DefaultExt = ".tif"; // Default file extension
            dlg.Filter = "Tif files (.tif)|*.tif"; // Filter files by extension

            // Show open file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process open file dialog box results
            if (result == true)
            {
                // Open document
                this.PathBackgroundSubtraction = dlg.FileName;
            }
        }

        private void SelectFlatField()
        {
            // Configure open file dialog box
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "Tif"; // Default file name
            dlg.DefaultExt = ".tif"; // Default file extension
            dlg.Filter = "Tif files (.tif)|*.tif"; // Filter files by extension

            // Show open file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process open file dialog box results
            if (result == true)
            {
                // Open document
                this.PathFlatField = dlg.FileName;
            }
        }

        private void StoreRSRate()
        {
            MVMManager.Instance.ReloadSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
            XmlDocument experimentFile = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/LSM");
            XmlManager.SetAttribute(ndList[0], experimentFile, "crsFrequencyHz", this._areaControlModel.RSLineRate.ToString());
            MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
        }

        private void TimeBasedLSTimeMSMinus()
        {
            this.TimeBasedLSTimeMS -= _areaControlModel.TimeBasedLineScanTimeIncrementMS;
        }

        private void TimeBasedLSTimeMSPlus()
        {
            this.TimeBasedLSTimeMS += _areaControlModel.TimeBasedLineScanTimeIncrementMS;
        }

        private void ZoomMinus()
        {
            BuildZoomTable();
            int[] val = FindFieldSizeInZoomTable(0);
            if (0 == val[0])
            {
                //LSM field size real maximum depends on other parameters,
                //especially GalvoGalvo, test to find out maximum under current setup:
                this.LSMFieldSize = val[1];
                if (this.LSMFieldSize != val[1])
                {
                    //limit max to be zoom 1x:
                    this.LSMFieldSize = _zoomTable[0] = _zoomTable[1];
                }
                else
                {
                    //reset zoom table:
                    _zoomTable[0] = 0;
                }
            }
            else
            {
                this.LSMFieldSize = val[1];
            }
        }

        private void ZoomPlus()
        {
            BuildZoomTable();
            this.LSMFieldSize = FindFieldSizeInZoomTable(1).ElementAt(1);
        }

        void _overviewWin_Closed(object sender, EventArgs e)
        {
            _overviewWin = null;
            _overviewVisible = false;
            OnPropertyChanged("OverviewVisible");
        }

        private void _statusTimer_Tick(object sender, EventArgs e)
        {
            OnPropertyChanged("RSLineRate");
        }

        #endregion Methods
    }
}