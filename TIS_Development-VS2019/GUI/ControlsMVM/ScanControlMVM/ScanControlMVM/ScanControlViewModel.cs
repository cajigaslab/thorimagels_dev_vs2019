namespace ScanControl.ViewModel
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
    using System.Windows.Media;
    using System.Xml;

    using OverlayManager;

    using ScanControl.Model;

    using ThorLogging;

    using ThorSharedTypes;

    public class ScanControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        const double MIN_DWELL_TIME = 0.4;
        const int NUM_DETECTORS = 4;

        private readonly ScanControlModel _scanControlModel;

        private string _activeLSMName = string.Empty;
        ICommand _AverageFramesMinusCommand;
        ICommand _AverageFramesPlusCommand;
        private ObservableCollection<ObservableCollection<string>> _bandwidthList = new ObservableCollection<ObservableCollection<string>>();

        //the order of the Bandwidth tags is important, it needs to match the order from DetectorBandwidths in SharedEnums.cs
        private string[] _bandwidthTags = { "250 kHz", "2.5 MHz", "15 MHz", "30MHz", "80 MHz", "200MHz", "300 MHz" };
        private Dictionary<int, string> _bandwidthToStringMap = new Dictionary<int, string>();
        private Visibility _bipolarityVisibility = Visibility.Collapsed;
        ICommand _ChanDigOffsetMinusCommand;
        ICommand _ChanDigOffsetPlusCommand;
        private Visibility _coarsePanelVisibility;
        private Visibility _digOffsetVisibility;
        private bool _dwellTimeSliderEnabled = true;
        ICommand _FlybackCyclesMinusCommand;
        ICommand _FlybackCyclesPlusCommand;
        ICommand _LSMAlignmentMinusCoarseCommand;
        ICommand _LSMAlignmentMinusCommand;
        ICommand _LSMAlignmentPlusCoarseCommand;
        ICommand _LSMAlignmentPlusCommand;
        ICommand _LSMDwellTimeMinusCommand;
        ICommand _LSMDwellTimePlusCommand;
        private Visibility _LSMPixelProcessVisibility;
        private Visibility _lsmPulseMultiplexingVisibility;
        double[][] _minDwellTimeTable;
        private Visibility _pmtBandwidthLabelVisibility;
        private string[] _pmtBwSelected = new string[NUM_DETECTORS];
        ICommand _PMTGainMinusCommand;
        ICommand _PMTGainPlusCommand;
        private int _pmtoffsetavailable = 0;
        private Visibility _pmtOffsetLabelVisibility;
        ICommand _PMTOffsetMinusCommand;
        ICommand _PMTOffsetPlusCommand;
        private int _pmtTripCount;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private double _sliderIndex = 0;
        private Dictionary<string, int> _stringToBandwidthMap = new Dictionary<string, int>();
        private Visibility _turnAroundOptionVisibility;
        ICommand _TwoWayCalibrationCommand;
        private Visibility _twoWayCalibrationVisibility;
        private TwoWaySettings _twoWayDialog = new TwoWaySettings();
        private bool _updateDwellTimeFromDevice = true;

        #endregion Fields

        #region Constructors

        public ScanControlViewModel()
        {
            this._scanControlModel = new ScanControlModel();
            InitializeProperties();
            BuildDwellTimeTable();

            _pmtTripCount = 0;
        }

        #endregion Constructors

        #region Properties

        public string ActiveLSMName
        {
            get
            {
                return _activeLSMName;
            }
            set
            {
                _activeLSMName = value;
                OnPropertyChanged("RapidScanVisibility");
            }
        }

        public ICommand AverageFramesMinusCommand
        {
            get
            {
                if (this._AverageFramesMinusCommand == null) this._AverageFramesMinusCommand = new RelayCommand(() => LSMSignalAverageFrames--); return this._AverageFramesMinusCommand;
            }
        }

        public ICommand AverageFramesPlusCommand
        {
            get
            {
                if (this._AverageFramesPlusCommand == null) this._AverageFramesPlusCommand = new RelayCommand(() => LSMSignalAverageFrames++); return this._AverageFramesPlusCommand;
            }
        }

        public ObservableCollection<ObservableCollection<string>> BandwidthList
        {
            get
            {
                return _bandwidthList;
            }
            set
            {
                _bandwidthList = value;
                OnPropertyChanged("BandwidthList");
            }
        }

        public Visibility BipolarityVisibility
        {
            get
            {
                return _bipolarityVisibility;
            }
            set
            {
                _bipolarityVisibility = value;
                OnPropertyChanged("BipolarityVisibility");
            }
        }

        public double CalculatedMinDwellTime
        {
            get
            {
                return this._scanControlModel.CalculatedMinDwellTime;
            }
        }

        public CustomCollection<HwVal<int>> ChanDigOffset
        {
            get;
            set;
        }

        public ICommand ChanDigOffsetMinusCommand
        {
            get
            {
                if (this._ChanDigOffsetMinusCommand == null) this._ChanDigOffsetMinusCommand = new RelayCommandWithParam((x) => ChanDigOffset[Convert.ToInt32(x)].Value--);
                return this._ChanDigOffsetMinusCommand;
            }
        }

        public ICommand ChanDigOffsetPlusCommand
        {
            get
            {
                if (this._ChanDigOffsetPlusCommand == null) this._ChanDigOffsetPlusCommand = new RelayCommandWithParam((x) => ChanDigOffset[Convert.ToInt32(x)].Value++);
                return this._ChanDigOffsetPlusCommand;
            }
        }

        public CustomCollection<Visibility> ChanDigOffsetVisibility
        {
            get;
            set;
        }

        public Visibility CoarsePanelVisibility
        {
            get
            {
                return _coarsePanelVisibility;
            }
            set
            {
                _coarsePanelVisibility = value;
                OnPropertyChanged("CoarsePanelVisibility");
            }
        }

        public string DigOffsetLabel
        {
            get
            {
                return (ResourceManagerCS.Instance.IsThorDAQBoard) ? "DC Offset" : "Dig Offset";
            }
        }

        public Visibility DigOffsetVisibility
        {
            get { return _digOffsetVisibility; }
            set
            {
                _digOffsetVisibility = value;
                //_digOffsetVisibility = Visibility.Visible;
                //InitializeProperties();

                for (int i = 0; i < NUM_DETECTORS; i++)
                {

                    ChanDigOffsetVisibility[i] = value;
                    //ChanDigOffsetVisibility[i] = Visibility.Visible;
                }

                OnPropertyChanged("DigOffsetVisibility");
                OnPropertyChanged("ChanDigOffsetVisibility");

            }
        }

        public bool DwellTimeSliderEnabled
        {
            get
            {
                return _dwellTimeSliderEnabled;
            }
            set
            {
                _dwellTimeSliderEnabled = value;
                OnPropertyChanged("DwellTimeSliderEnabled");
            }
        }

        public bool EnablePMTGains
        {
            set
            {
                for (int i = 0; i < PMTGainEnable.Count; i++)
                {
                    if (PMTGain[i].Value > GetPMTMin(i))
                    {
                        PMTGainEnable[i].Value = value ? 1 : 0;
                    }
                }
            }
        }

        Visibility _fastOneWayImagingModeEnableVisibility = Visibility.Collapsed;
        public Visibility FastOneWayImagingModeEnableVisibility
        {
            get
            {

                return _fastOneWayImagingModeEnableVisibility;
            }
            set
            {
                if (_scanControlModel.LSMIsFastOneWayImagingModeEnableAvailable)
                {
                    _fastOneWayImagingModeEnableVisibility = value;
                }
                OnPropertyChanged("FastOneWayImagingModeEnableVisibility");
            }
        }

        public ICommand FlybackCyclesMinusCommand
        {
            get
            {
                if (this._FlybackCyclesMinusCommand == null) this._FlybackCyclesMinusCommand = new RelayCommand(() => LSMFlybackCycles--);
                return this._FlybackCyclesMinusCommand;
            }
        }

        public ICommand FlybackCyclesPlusCommand
        {
            get
            {
                if (this._FlybackCyclesPlusCommand == null) this._FlybackCyclesPlusCommand = new RelayCommand(() => LSMFlybackCycles++);
                return this._FlybackCyclesPlusCommand;
            }
        }

        public double FramesPerSecond
        {
            get
            {
                return this._scanControlModel.FramesPerSecond;
            }
        }

        public string FramesPerSecondAverage
        {
            get
            {
                if (0 == LSMSignalAverage)
                {
                    return string.Empty;
                }
                else
                {
                    return String.Format("{0} fps", (FramesPerSecond / LSMSignalAverageFrames).ToString("#0.0"));
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

        public Visibility GRLSMScanVisibility
        {
            get
            {
                return (((int)ThorSharedTypes.ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType()) &&
                    ((int)ThorSharedTypes.ICamera.LSMType.GALVO_RESONANCE == ResourceManagerCS.GetLSMType())) ?
                    Visibility.Visible : Visibility.Collapsed;
            }
        }

        public CustomCollection<HwVal<int>> InputRange
        {
            get;
            set;
        }

        public int InputRangeMax
        {
            get
            {
                int valMin = 0;
                int valMax = 0;
                int valDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_INPUTRANGE1, ref valMin, ref valMax, ref valDefault);

                return valMax;
            }
        }

        public int InputRangeMin
        {
            get
            {
                int valMin = 0;
                int valMax = 0;
                int valDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_INPUTRANGE1, ref valMin, ref valMax, ref valDefault);

                return valMin;
            }
        }

        public CustomCollection<Visibility> IsChannelVisible
        {
            get;
            set;
        }

        public int LastLSMScanMode
        {
            get;
            set;
        }

        public DateTime LastPMTSafetyUpdate
        {
            get { return _scanControlModel.LastPMTSafetyUpdate; }
            set { _scanControlModel.LastPMTSafetyUpdate = value; }
        }

        public ICommand LSMAlignmentMinusCoarseCommand
        {
            get
            {
                if (this._LSMAlignmentMinusCoarseCommand == null) this._LSMAlignmentMinusCoarseCommand = new RelayCommand(() => LSMTwoWayAlignmentCoarse--);
                return this._LSMAlignmentMinusCoarseCommand;
            }
        }

        public ICommand LSMAlignmentMinusCommand
        {
            get
            {
                if (this._LSMAlignmentMinusCommand == null) this._LSMAlignmentMinusCommand = new RelayCommand(() => LSMTwoWayAlignment--);
                return this._LSMAlignmentMinusCommand;
            }
        }

        public ICommand LSMAlignmentPlusCoarseCommand
        {
            get
            {
                if (this._LSMAlignmentPlusCoarseCommand == null) this._LSMAlignmentPlusCoarseCommand = new RelayCommand(() => LSMTwoWayAlignmentCoarse++);
                return this._LSMAlignmentPlusCoarseCommand;
            }
        }

        public ICommand LSMAlignmentPlusCommand
        {
            get
            {
                if (this._LSMAlignmentPlusCommand == null) this._LSMAlignmentPlusCommand = new RelayCommand(() => LSMTwoWayAlignment++);
                return this._LSMAlignmentPlusCommand;
            }
        }

        public bool LSMAverageEnabled
        {
            get;
            set;
        }

        public bool LsmClkPnlEnabled
        {
            get
            {
                return this._scanControlModel.LsmClkPnlEnabled;
            }
            set
            {
                this._scanControlModel.LsmClkPnlEnabled = value;
                OnPropertyChange("LsmClkPnlEnabled");

                if (value)
                {
                    LastLSMScanMode = LSMScanMode;
                }
                else if (LSMScanMode != LastLSMScanMode)
                {
                    LSMScanMode = LastLSMScanMode;
                }
            }
        }

        public int LSMClockSource
        {
            get
            {
                //binding to a check box. Convert the 1-2 values to 0-1.
                if (1 == this._scanControlModel.LSMClockSource)
                {
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
            set
            {
                if (0 == value)
                {
                    this._scanControlModel.LSMClockSource = 1;
                    LSMPulseMultiplexing = 0;
                    MVMManager.Instance["ThreePhotonControlViewModel", "ThreePhotonEnable"] = 0;
                }
                else
                {
                    if (this._scanControlModel.LSMClockSource != 2)
                    {
                        this._scanControlModel.LSMClockSource = 2;
                        LSMQueryExternalClockRate = 1;
                    }
                }
                OnPropertyChanged("LSMClockSource");
                OnPropertyChanged("LSMExtClockRate");
                OnPropertyChanged("InputRange");
            }
        }

        public int LSMDwellTimeMaxIndex
        {
            get
            {
                //determine the maximum number of steps for the for the dwell time
                return (int)((this.LSMPixelDwellTimeMax - this.LSMPixelDwellTimeMin) / this.LSMPixelDwellTimeStep);
            }
        }

        public int LSMDwellTimeMinIndex
        {
            get
            {
                const int PIXEL_DENSITY_INCREMENT = 5;//step by 32
                int index = 0;
                try
                {
                    index = Convert.ToInt32(Math.Round((_minDwellTimeTable[((int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)32] >> PIXEL_DENSITY_INCREMENT) - 1][(int)MVMManager.Instance["AreaControlViewModel", "LSMFieldSize", (object)5]] - this.LSMPixelDwellTimeMin) / this.LSMPixelDwellTimeStep, 0));
                }
                catch { }

                return index;

            }
        }

        public ICommand LSMDwellTimeMinusCommand
        {
            get
            {
                if (this._LSMDwellTimeMinusCommand == null)
                {
                    this._LSMDwellTimeMinusCommand = new RelayCommand(() => LSMDwellTimeMinusCommandFnc());
                }
                return this._LSMDwellTimeMinusCommand;
            }
        }

        public ICommand LSMDwellTimePlusCommand
        {
            get
            {
                if (this._LSMDwellTimePlusCommand == null)
                {
                    this._LSMDwellTimePlusCommand = new RelayCommand(() => LSMDwellTimePlusCommandFnc());
                }
                return this._LSMDwellTimePlusCommand;
            }
        }

        public double LSMExtClockRate
        {
            get
            {
                //convert from Hz to MHz
                double dVal = this._scanControlModel.LSMExtClockRate / (double)Constants.US_TO_SEC;
                return Math.Round(dVal, 6);
            }
            set
            {
                //Convert from Mhz to Hz
                double dVal = value * (double)Constants.US_TO_SEC;
                int iVal = Convert.ToInt32(dVal);
                this._scanControlModel.LSMExtClockRate = iVal;
                OnPropertyChanged("LSMExtClockRate");
            }
        }

        public double LSMExternalClockPhaseOffset
        {
            get
            {
                return this._scanControlModel.LSMExternalClockPhaseOffset;
            }
            set
            {
                this._scanControlModel.LSMExternalClockPhaseOffset = value;
                OnPropertyChanged("LSMExternalClockPhaseOffset");
            }
        }

        public bool LSMFastOneWayImagingModeEnable
        {
            get
            {
                return _scanControlModel.LSMFastOneWayImagingModeEnable;
            }
            set
            {
                _scanControlModel.LSMFastOneWayImagingModeEnable = value;
                OnPropertyChanged("LSMFastOneWayImagingModeEnable");
            }
        }

        public int LSMFlybackCycles
        {
            get
            {
                return _scanControlModel.LSMFlybackCycles;
            }
            set
            {
                _scanControlModel.LSMFlybackCycles = value;
                OnPropertyChanged("LSMFlybackCycles");
                OnPropertyChanged("LSMFlybackTime");
            }
        }

        public double LSMFlybackTime
        {
            get
            {
                return Math.Round(_scanControlModel.LSMFlybackTime, 6);
            }
        }

        public int LSMInterleaveScan
        {
            get
            {
                return _scanControlModel.LSMInterleaveScan;
            }
            set
            {
                _scanControlModel.LSMInterleaveScan = value;
                OnPropertyChanged("LSMInterleaveScan");
            }
        }

        public Visibility LSMPhaseAdjusmentVisibility
        {
            get
            {
                return (ResourceManagerCS.Instance.IsThorDAQBoard) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public double LSMPixelDwellTime
        {
            get
            {
                if (_updateDwellTimeFromDevice)
                {
                    double dwellTime = Math.Round(this._scanControlModel.LSMPixelDwellTime, 2, MidpointRounding.AwayFromZero);
                    _sliderIndex = (dwellTime - LSMPixelDwellTimeMin) / LSMPixelDwellTimeStep;
                    OnPropertyChanged("LSMPixelDwellTimeIndex");
                    return dwellTime;
                }
                else
                {
                    //Update the GUI only when the user is moving the slider. Once they pick a dwell time it will update the device value
                    return Math.Round(LSMPixelDwellTimeMin + LSMPixelDwellTimeIndex * LSMPixelDwellTimeStep, 2, MidpointRounding.AwayFromZero);
                }
            }
            set
            {
                //Don't do anything if out of range
                ConfirmDwellTime(ref value);
                _scanControlModel.LSMPixelDwellTime = value;
                _updateDwellTimeFromDevice = true;
                OnPropertyChanged("LSMPixelDwellTime");
                OnPropertyChanged("LSMFlybackCycles");
                OnPropertyChanged("LSMFlybackTime");
                OnPropertyChanged("LSMTwoWayAlignment");
                OnPropertyChanged("NumberOfPulsesPerPixel");
                if ((bool)MVMManager.Instance["AreaControlViewModel", "TimeBasedLineScan", (object)false])
                {
                    //If time based line scan is enabled and the dwell time is changed, recalculate the size of pixel y
                    MVMManager.Instance["AreaControlViewModel", "TimeBasedLSTimeMS"] = (double)MVMManager.Instance["AreaControlViewModel", "TimeBasedLSTimeMS", (object)1.0];
                }
            }
        }

        public double LSMPixelDwellTimeIndex
        {
            get
            {
                return _sliderIndex;
            }
            set
            {
                //Don't move the slider if the dwell time is out of bounds. Get from the camera the CalculatedMinDwell from the current field size and pixel X.
                double nextDwellTime = Math.Round(LSMPixelDwellTimeMin + value * LSMPixelDwellTimeStep, 2, MidpointRounding.AwayFromZero);
                if (nextDwellTime > CalculatedMinDwellTime || (bool)MVMManager.Instance["AreaControlViewModel", "GGSuperUserMode", (object)false])
                {
                    _sliderIndex = value;
                    OnPropertyChanged("LSMPixelDwellTimeIndex");
                    //Update GUI while moving the slider, but don't read the value from the camera yet.
                    OnPropertyChanged("LSMPixelDwellTime");
                }
            }
        }

        public double LSMPixelDwellTimeMax
        {
            get
            {
                return _scanControlModel.LSMPixelDwellTimeMax;
            }
        }

        public double LSMPixelDwellTimeMaxIndex
        {
            get
            {
                return (LSMPixelDwellTimeMax - LSMPixelDwellTimeMin) / LSMPixelDwellTimeStep;
            }
        }

        public double LSMPixelDwellTimeMin
        {
            get
            {
                return _scanControlModel.LSMPixelDwellTimeMin;
            }
        }

        public double LSMPixelDwellTimeStep
        {
            get
            {
                return _scanControlModel.LSMPixelDwellTimeStep;
            }
        }

        //ThreePhotonMVM sets the dwell time depending on the laser frequency. Don't call ConfirmDwellTime
        //because we don't want any rounding to be applied to this number.
        public double LSMPixelDwellTimeThreePhotonCall
        {
            set
            {
                //Don't do anything if out of range
                if (MIN_DWELL_TIME <= value && this.LSMPixelDwellTimeMax >= value)
                {
                    _scanControlModel.LSMPixelDwellTime = value;
                    double dwellTimeStep = this.LSMPixelDwellTimeStep;
                    LSMPixelDwellTimeIndex = (value - LSMPixelDwellTimeMin) / dwellTimeStep;
                    _updateDwellTimeFromDevice = true;
                    OnPropertyChanged("LSMPixelDwellTime");
                    OnPropertyChanged("LSMPixelDwellTimeIndex");
                    OnPropertyChanged("LSMFlybackCycles");
                    OnPropertyChanged("LSMFlybackTime");
                    OnPropertyChanged("LSMPixelDwellTimeMaxIndex");
                }
            }
        }

        public int LSMPixelProcess
        {
            get
            {
                return _scanControlModel.LSMPixelProcess;
            }
            set
            {
                _scanControlModel.LSMPixelProcess = value;
                OnPropertyChanged("LSMPixelProcess");
            }
        }

        public Visibility LSMPixelProcessVisibility
        {
            get
            {
                return _LSMPixelProcessVisibility;
            }
            set
            {
                //only set to visible if the image detector is GalvoGalvo.
                _LSMPixelProcessVisibility = (
                        ((int)ThorSharedTypes.ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType())
                        && ResourceManagerCS.Instance.GetActiveLSMName().Equals("GalvoGalvo")
                     ) ? value : Visibility.Collapsed;
                OnPropertyChanged("LSMPixelProcessVisibility");
            }
        }

        public int LSMPulseMultiplexing
        {
            get
            {
                return this._scanControlModel.LSMPulseMultiplexing;
            }
            set
            {
                if (1 == value)
                {
                    LSMClockSource = 1;
                }
                this._scanControlModel.LSMPulseMultiplexing = value;
                OnPropertyChanged("LSMPulseMultiplexing");
            }
        }

        public Visibility LSMPulseMultiplexingVisibility
        {
            get { return _lsmPulseMultiplexingVisibility; }
            set { _lsmPulseMultiplexingVisibility = value; }
        }

        public int LSMQueryExternalClockRate
        {
            set
            {
                this._scanControlModel.LSMQueryExternalClockRate = value;
            }
        }

        public int LSMRealtimeAveraging
        {
            get
            {
                return this._scanControlModel.LSMRealtimeAveraging;
            }
            set
            {
                this._scanControlModel.LSMRealtimeAveraging = value;
                OnPropertyChanged("LSMRealtimeAveraging");
            }
        }

        public double LSMScaleYScan
        {
            get
            {
                return this._scanControlModel.LSMScaleYScan;
            }
            set
            {
                this._scanControlModel.LSMScaleYScan = value;
                OnPropertyChanged("LSMScaleYScan");
            }
        }

        public int LSMScanMode
        {
            get
            {
                return this._scanControlModel.LSMScanMode;
            }
            set
            {
                if (true == this.TwoWayEnable)
                {
                    if (ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0])
                    {
                        this._scanControlModel.LSMScanMode = (int)ICamera.ScanMode.FORWARD_SCAN;
                    }
                    else if ((int)ICamera.ScanMode.FORWARD_SCAN != value && (int)ICamera.ScanMode.TWO_WAY_SCAN != value)
                    {
                        this._scanControlModel.LSMScanMode = (int)ICamera.ScanMode.TWO_WAY_SCAN;
                    }
                    else
                    {
                        this._scanControlModel.LSMScanMode = value;
                    }
                }
                else
                {
                    this._scanControlModel.LSMScanMode = (int)ICamera.ScanMode.FORWARD_SCAN;
                }
                OnPropertyChanged("LSMScanMode");
                ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMPixelXMax");
                ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMPixelYMax");
                bool modified = (bool)MVMManager.Instance["AreaControlViewModel", "ConfirmAreaModeSettingsForGG", false];
                LastLSMScanMode = this._scanControlModel.LSMScanMode;
                if ((bool)MVMManager.Instance["AreaControlViewModel", "TimeBasedLineScan", (object)false])
                {
                    //If time based line scan is enabled and the scan mode is changed, recalculate the size of pixel y
                    MVMManager.Instance["AreaControlViewModel", "TimeBasedLSTimeMS"] = (double)MVMManager.Instance["AreaControlViewModel", "TimeBasedLSTimeMS", (object)1.0];
                }
            }
        }

        public string LSMScannerName
        {
            get
            {
                return this._scanControlModel.LSMScannerName;
            }
        }

        public int LSMSignalAverage
        {
            get
            {
                return this._scanControlModel.LSMSignalAverage;
            }
            set
            {
                this._scanControlModel.LSMSignalAverage = value;
                OnPropertyChanged("LSMSignalAverage");
            }
        }

        public int LSMSignalAverageFrames
        {
            get
            {
                return this._scanControlModel.SignalAverageFrames;
            }
            set
            {
                this._scanControlModel.SignalAverageFrames = value;
                OnPropertyChanged("LSMSignalAverageFrames");
            }
        }

        public int LSMTwoWayAlignment
        {
            get
            {
                return this._scanControlModel.LSMTwoWayAlignment;
            }
            set
            {
                this._scanControlModel.LSMTwoWayAlignment = value;

                OnPropertyChanged("LSMTwoWayAlignment");
            }
        }

        public int LSMTwoWayAlignmentCoarse
        {
            get
            {
                return this._scanControlModel.LSMTwoWayAlignmentCoarse;
            }
            set
            {
                this._scanControlModel.LSMTwoWayAlignmentCoarse = value;

                OnPropertyChanged("LSMTwoWayAlignmentCoarse");
            }
        }

        public double MinDwellTimeFromTable
        {
            get
            {
                const int PIXEL_DENSITY_INCREMENT = 5;//step by 32

                return _minDwellTimeTable[(((int)(MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)32])) >> PIXEL_DENSITY_INCREMENT) - 1][(int)MVMManager.Instance["AreaControlViewModel", "LSMFieldSize", (object)5]];
            }
        }

        public int NumberOfPulsesPerPixel
        {
            get
            {
                //There are only 16 taps in the FIR filter, so the maximum number of pulses we can read per pixel is 16
                return Math.Min(16, (int)Math.Round(LSMPixelDwellTimeIndex) + 1);
            }
        }

        //public ObservableCollection<string> PmtBandwidthSelected
        //{
        //    get
        //    {
        //        if (_bandwidthList.Count > 0)
        //        {
        //            _pmtBwSelected.Clear();
        //            for (int i = 0; i < NUM_DETECTORS; i++)
        //            {
        //                if (Visibility.Visible == PmtBandwidthVisibility[i])
        //                {
        //                    _pmtBwSelected.Add(_bandwidthToStringMap[PMTBandwidth[i].Value]);
        //                }
        //                else
        //                {
        //                    _pmtBwSelected.Add(string.Empty);
        //                }
        //            }
        //        }
        //        return _pmtBwSelected;
        //    }
        //    set
        //    {
        //        _pmtBwSelected = value;
        //        if (_pmtBwSelected.Count == NUM_DETECTORS)
        //        {
        //            for(int i = 0; i < NUM_DETECTORS; i++)
        //            {
        //                if(PMTBandwidth[i].Value != _stringToBandwidthMap[_pmtBwSelected[i]] && _bandwidthList[i].Contains(_pmtBwSelected[i]))
        //                {
        //                    PMTBandwidth[i].Value = _stringToBandwidthMap[_pmtBwSelected[i]];
        //                }
        //            }
        //            OnPropertyChanged("PmtBandwidthSelected");
        //        }
        //    }
        //}
        public string Pmt1BandwidthSelected
        {
            get
            {
                if (_bandwidthList.Count > 0)
                {
                    if (Visibility.Visible == PmtBandwidthVisibility[0])
                    {
                        _pmtBwSelected[0] = _bandwidthToStringMap[PMTBandwidth[0].Value];
                    }
                }
                return _pmtBwSelected[0];
            }
            set
            {
                _pmtBwSelected[0] = value;
                if (PMTBandwidth[0].Value != _stringToBandwidthMap[_pmtBwSelected[0]] && _bandwidthList[0].Contains(_pmtBwSelected[0]))
                {
                    PMTBandwidth[0].Value = _stringToBandwidthMap[_pmtBwSelected[0]];
                }
                OnPropertyChanged("Pmt1BandwidthSelected");
            }
        }

        public int PMT1Saturations
        {
            get
            {
                return this._scanControlModel.PMT1Saturations;
            }
        }

        public string Pmt2BandwidthSelected
        {
            get
            {
                if (_bandwidthList.Count > 0)
                {
                    if (Visibility.Visible == PmtBandwidthVisibility[1])
                    {
                        _pmtBwSelected[1] = _bandwidthToStringMap[PMTBandwidth[1].Value];
                    }
                }
                return _pmtBwSelected[1];
            }
            set
            {
                _pmtBwSelected[1] = value;
                if (PMTBandwidth[1].Value != _stringToBandwidthMap[_pmtBwSelected[1]] && _bandwidthList[1].Contains(_pmtBwSelected[1]))
                {
                    PMTBandwidth[1].Value = _stringToBandwidthMap[_pmtBwSelected[1]];
                }
                OnPropertyChanged("Pmt2BandwidthSelected");
            }
        }

        public int PMT2Saturations
        {
            get
            {
                return this._scanControlModel.PMT2Saturations;
            }
        }

        public string Pmt3BandwidthSelected
        {
            get
            {
                if (_bandwidthList.Count > 0)
                {
                    if (Visibility.Visible == PmtBandwidthVisibility[2])
                    {
                        _pmtBwSelected[2] = _bandwidthToStringMap[PMTBandwidth[2].Value];
                    }
                }
                return _pmtBwSelected[2];
            }
            set
            {
                _pmtBwSelected[2] = value;
                if (PMTBandwidth[2].Value != _stringToBandwidthMap[_pmtBwSelected[2]] && _bandwidthList[2].Contains(_pmtBwSelected[2]))
                {
                    PMTBandwidth[2].Value = _stringToBandwidthMap[_pmtBwSelected[2]];
                }
                OnPropertyChanged("Pmt3BandwidthSelected");
            }
        }

        public int PMT3Saturations
        {
            get
            {
                return this._scanControlModel.PMT3Saturations;
            }
        }

        public string Pmt4BandwidthSelected
        {
            get
            {
                if (_bandwidthList.Count > 0)
                {
                    if (Visibility.Visible == PmtBandwidthVisibility[3])
                    {
                        _pmtBwSelected[3] = _bandwidthToStringMap[PMTBandwidth[3].Value];
                    }
                }
                return _pmtBwSelected[3];
            }
            set
            {
                _pmtBwSelected[3] = value;
                if (PMTBandwidth[3].Value != _stringToBandwidthMap[_pmtBwSelected[3]] && _bandwidthList[3].Contains(_pmtBwSelected[3]))
                {
                    PMTBandwidth[3].Value = _stringToBandwidthMap[_pmtBwSelected[3]];
                }
                OnPropertyChanged("Pmt4BandwidthSelected");
            }
        }

        public int PMT4Saturations
        {
            get
            {
                return this._scanControlModel.PMT4Saturations;
            }
        }

        public CustomCollection<HwVal<int>> PMTBandwidth
        {
            get;
            set;
        }

        public Visibility PMTBandwidthLabelVisibility
        {
            get
            {
                int val = 0;
                bool result = false;
                for (int i = 0; i < NUM_DETECTORS; i++)
                {
                    result = _scanControlModel.GetPMTBandwidthIsAvailable(i, ref val);
                    if (true == result)
                    {

                        return Visibility.Visible;
                    }
                }
                _pmtBandwidthLabelVisibility = Visibility.Collapsed;
                return Visibility.Collapsed;
            }

            set
            {
                _pmtBandwidthLabelVisibility = value;

                OnPropertyChanged("PMTBandwidthLabelVisibility");
            }
        }

        public CustomCollection<Visibility> PmtBandwidthVisibility
        {
            get;
            set;
        }

        public CustomCollection<HwVal<int>> PMTDetectorType
        {
            get;
            set;
        }

        public CustomCollection<HwVal<int>> PMTGain
        {
            get;
            set;
        }

        public CustomCollection<HwVal<int>> PMTGainEnable
        {
            get;
            set;
        }

        public ICommand PMTGainMinusCommand
        {
            get
            {
                if (this._PMTGainMinusCommand == null) this._PMTGainMinusCommand = new RelayCommandWithParam((x) => PMTGain[Convert.ToInt32(x)].Value--);
                return this._PMTGainMinusCommand;
            }
        }

        public ICommand PMTGainPlusCommand
        {
            get
            {
                if (this._PMTGainPlusCommand == null) this._PMTGainPlusCommand = new RelayCommandWithParam((x) => PMTGain[Convert.ToInt32(x)].Value++);
                return this._PMTGainPlusCommand;
            }
        }

        public int[] PMTMode
        {
            get
            {
                return this._scanControlModel.PMTMode;
            }

            set
            {
                this._scanControlModel.PMTMode = value;
            }
        }

        public CustomCollection<HwVal<double>> PMTOffset
        {
            get;
            set;
        }

        public Visibility PMTOffsetLabelVisibility
        {
            get
            {
                bool val = false;
                for (int i = 0; i < NUM_DETECTORS; i++)
                {
                    _scanControlModel.GetPMTOffsetIsAvailable(i, ref val);
                    if (true == val)
                    {
                        return Visibility.Visible;
                    }
                }
                _pmtOffsetLabelVisibility = Visibility.Collapsed;
                _pmtoffsetavailable = 0;
                for (int i = 0; i < NUM_DETECTORS; i++)
                {
                    PMTOffsetVisibility[i] = Visibility.Collapsed;
                }
                return Visibility.Collapsed;
            }
            set
            {
                _pmtOffsetLabelVisibility = value;
                OnPropertyChanged("PMTOffsetLabelVisibility");
            }
        }

        public ICommand PMTOffsetMinusCommand
        {
            get
            {
                if (this._PMTOffsetMinusCommand == null)
                    this._PMTOffsetMinusCommand = new RelayCommandWithParam((x) => PMTOffset[Convert.ToInt32(x)].Value -= _scanControlModel.GetPMTOffsetStepSize(Convert.ToInt32(x)));
                return this._PMTOffsetMinusCommand;
            }
        }

        public ICommand PMTOffsetPlusCommand
        {
            get
            {
                if (this._PMTOffsetPlusCommand == null)
                    this._PMTOffsetPlusCommand = new RelayCommandWithParam((x) => PMTOffset[Convert.ToInt32(x)].Value += _scanControlModel.GetPMTOffsetStepSize(Convert.ToInt32(x)));
                return this._PMTOffsetPlusCommand;
            }
        }

        public CustomCollection<Visibility> PMTOffsetVisibility
        {
            get;
            set;
        }

        public CustomCollection<HwVal<int>> PMTOn
        {
            get;
            set;
        }

        public CustomCollection<HwVal<int>> PMTPolarity
        {
            get;
            set;
        }

        public bool PMTSafetyStatus
        {
            get
            {
                return this._scanControlModel.PMTSafetyStatus;
            }
            set
            {
                this._scanControlModel.PMTSafetyStatus = value;
            }
        }

        public int PMTTripCount
        {
            get
            {
                return _pmtTripCount;
            }
            set
            {
                _pmtTripCount = value;
                OnPropertyChanged("PMTTripCount");
            }
        }

        public CustomCollection<DoublePC> PMTVolt
        {
            get;
            set;
        }

        public double[,] PMTVoltage
        {
            get
            {
                return this._scanControlModel.PMTVoltage;
            }

            set
            {
                this._scanControlModel.PMTVoltage = value;
            }
        }

        public Visibility PulsesPerPixelVisibility
        {
            get
            {
                return (1 == (int)MVMManager.Instance["ThreePhotonControlViewModel", "ThreePhotonEnable", (object)0]) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public Visibility RapidScanVisibility
        {
            get
            {
                return ((((int)ThorSharedTypes.ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType()) && ((int)ThorSharedTypes.ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType())) && !ResourceManagerCS.Instance.IsThorDAQBoard) ?
                    Visibility.Visible : Visibility.Collapsed;
            }
        }

        public Visibility TurnAroundOptionVisibility
        {
            get
            {
                return _turnAroundOptionVisibility;
            }
            set
            {
                //only set to visible if the image detector is ThorDaqGG. For now, eventually this needs to be an option for all GG
                _turnAroundOptionVisibility = (ResourceManagerCS.Instance.IsThorDAQBoard && (int)ThorSharedTypes.ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType()) ? value : Visibility.Collapsed;
                OnPropertyChanged("TurnAroundOptionVisibility");
            }
        }

        public int TurnAroundTimeUS
        {
            get
            {
                return this._scanControlModel.TurnAroundTimeUS;
            }
            set
            {
                this._scanControlModel.TurnAroundTimeUS = value;
                OnPropertyChanged("TurnAroundTimeUS");
            }
        }

        public ICommand TwoWayCalibrationCommand
        {
            get
            {
                if (this._TwoWayCalibrationCommand == null) this._TwoWayCalibrationCommand = new RelayCommand(() => TwoWayCalibration());
                return this._TwoWayCalibrationCommand;
            }
        }

        public Visibility TwoWayCalibrationVisibility
        {
            get { return _twoWayCalibrationVisibility; }
            set { _twoWayCalibrationVisibility = value; }
        }

        public bool TwoWayEnable
        {
            get
            {
                if (4 > (int)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0])
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        public Visibility TwoWayVisibility
        {
            get
            {
                if (ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0])
                {
                    return Visibility.Collapsed;
                }
                else
                {
                    return Visibility.Visible;
                }
            }
        }

        //Update Dwell Time will be called once the user release the dwell time slider bar or click on it
        public bool UpdateDwellTime
        {
            get
            {
                return true;
            }
            set
            {
                if (value)
                {
                    this.LSMPixelDwellTime = Math.Round(this.LSMPixelDwellTimeMin + LSMPixelDwellTimeIndex * this.LSMPixelDwellTimeStep, 2);
                }
                _updateDwellTimeFromDevice = value;
            }
        }

        public bool UseFastestFlybackEnabled
        {
            get;
            set;
        }

        public bool UseFastestSettingForFlybackCycles
        {
            get
            {
                return _scanControlModel.UseFastestSettingForFlybackCycles;
            }
            set
            {
                _scanControlModel.UseFastestSettingForFlybackCycles = value;
                OnPropertyChanged("UseFastestSettingForFlybackCycles");
                OnPropertyChanged("LSMFlybackCycles");
                OnPropertyChanged("LSMFlybackTime");
            }
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : null;
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

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(ScanControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/LSM");

            GenerateBandwidthList();

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                //set the scan mode first to ensure the pixel slider has the correct range

                if (XmlManager.GetAttribute(ndList[0], doc, "scanMode", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        LSMScanMode = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "interleave", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        LSMInterleaveScan = tmp;
                    }
                }

                // ---- The following section is commented because, the ennabled channels are loaded at the end of
                // ---- this function from a generic part of the settings file, as this tag only colors LSM channel ennabled.
                //if (GetAttribute(ndList[0], doc, "channel", ref str))
                //{
                //    int tmp = 0;
                //    if (Int32.TryParse(str, out tmp))
                //    {
                //        selectChannelUsingConverter(tmp);   // This method is extracted because will also be used in onLoadExperiment()
                //    }
                //}
                XmlDocument hwDoc = (XmlDocument)MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
                XmlNodeList ndWLList = hwDoc.SelectNodes("/HardwareSettings/Wavelength");
                int maxGUIChannels = (int)MVMManager.Instance["CaptureSetupViewModel", "MaxChannels", (object)4];
                for (int i = 0; i < IsChannelVisible.Count; i++)
                {
                    if (i < ndWLList.Count && i < maxGUIChannels)
                    {
                        IsChannelVisible[i] = Visibility.Visible;
                    }
                    else
                    {
                        IsChannelVisible[i] = Visibility.Collapsed;
                        // The disabling of the non-visible channels continues in CaptureSetupModule->MasterView.xaml.cs->LoadChannelSelection
                    }
                }

                LoadPixelCountXY(ndList);

                // LSMClockSource and ThreePhotonEnable need to be loaded first before the input ranges, this is because in Thordaq when one of those is checked,
                // it will default to 1.5V internally. If they were loaded after the input ranges, the input range would always be switched to 1.5V when Capture Setup is loaded

                //If 3P is enabled don't read the status of the extClockRate
                int threePhotonEnable = 0;
                if (XmlManager.GetAttribute(ndList[0], doc, "ThreePhotonEnable", ref str) && (Int32.TryParse(str, out threePhotonEnable)))
                {
                    MVMManager.Instance["ThreePhotonControlViewModel", "ThreePhotonEnable"] = threePhotonEnable;
                    if (XmlManager.GetAttribute(ndList[0], doc, "extClockRate", ref str) && 1 != threePhotonEnable)
                    {
                        int tmp = 0;
                        if (Int32.TryParse(str, out tmp))
                        {
                            LSMExtClockRate = (double)tmp / (double)Constants.US_TO_SEC;
                        }
                    }
                }

                //only need to set clock source if 3P not enabled
                if (0 == threePhotonEnable)
                {
                    if (XmlManager.GetAttribute(ndList[0], doc, "clockSource", ref str))
                    {
                        int tmp = 0;
                        if (Int32.TryParse(str, out tmp))
                        {
                            LSMClockSource = tmp - 1;
                        }
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "inputRange1", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        InputRange[0].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "inputRange2", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        InputRange[1].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "inputRange3", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        InputRange[2].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "inputRange4", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        InputRange[3].Value = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "polarity1", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTPolarity[0].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "polarity2", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTPolarity[1].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "polarity3", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTPolarity[2].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "polarity4", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTPolarity[3].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "dwellTime", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        LSMPixelDwellTime = tmp;
                        _updateDwellTimeFromDevice = true;
                        LSMPixelDwellTimeIndex = (LSMPixelDwellTime - LSMPixelDwellTimeMin) / LSMPixelDwellTimeStep;
                    }

                }

                if (XmlManager.GetAttribute(ndList[0], doc, "averageMode", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        LSMSignalAverage = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "averageNum", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        LSMSignalAverageFrames = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "minimizeFlybackCycles", ref str))
                {
                    UseFastestSettingForFlybackCycles = (str == "1");
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "flybackCycles", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        LSMFlybackCycles = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "turnAroundTimeUS", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        TurnAroundTimeUS = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "chan1DigOffset", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        ChanDigOffset[0].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "chan2DigOffset", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        ChanDigOffset[1].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "chan3DigOffset", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        ChanDigOffset[2].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "chan4DigOffset", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        ChanDigOffset[3].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "pulseMultiplexing", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        LSMPulseMultiplexing = tmp;
                    }
                }

                //bring the phase offset from the old property if externalClockPhaseOffset is not there yet.
                //making it backwards compatible
                if (XmlManager.GetAttribute(ndList[0], doc, "externalClockPhaseOffset", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, out tmp))
                    {
                        LSMExternalClockPhaseOffset = tmp;
                    }
                }
                else if (XmlManager.GetAttribute(ndList[0], doc, "pulseMultiplexingPhase", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, out tmp))
                    {
                        LSMExternalClockPhaseOffset = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "LSMPixelProcess", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        LSMPixelProcess = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "fastOnewayGGEnable", ref str))
                {
                    LSMFastOneWayImagingModeEnable = "1" == str;
                }
            }

            ActiveLSMName = ResourceManagerCS.Instance.GetActiveLSMName();

            //load the voltage settings for gain before the gain assignment
            LoadPMTHardwareSettings();

            ndList = doc.SelectNodes("/ThorImageExperiment/PMT");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "gainA", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTGain[0].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "bandwidthAHz", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        if (Visibility.Visible == PmtBandwidthVisibility[0])
                        {
                            PMTBandwidth[0].Value = tmp;
                            Pmt1BandwidthSelected = _bandwidthToStringMap[PMTBandwidth[0].Value];
                        }
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "offsetAVolts", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        PMTOffset[0].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "gainB", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTGain[1].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "bandwidthBHz", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        if (Visibility.Visible == PmtBandwidthVisibility[1])
                        {
                            PMTBandwidth[1].Value = tmp;
                            Pmt2BandwidthSelected = _bandwidthToStringMap[PMTBandwidth[1].Value];
                        }
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "offsetBVolts", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        PMTOffset[1].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "gainC", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTGain[2].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "bandwidthCHz", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        if (Visibility.Visible == PmtBandwidthVisibility[2])
                        {
                            PMTBandwidth[2].Value = tmp;
                            Pmt3BandwidthSelected = _bandwidthToStringMap[PMTBandwidth[2].Value];
                        }
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "offsetCVolts", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        PMTOffset[2].Value = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "gainD", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTGain[3].Value = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "bandwidthDHz", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        if (Visibility.Visible == PmtBandwidthVisibility[3])
                        {
                            PMTBandwidth[3].Value = tmp;
                            Pmt4BandwidthSelected = _bandwidthToStringMap[PMTBandwidth[3].Value];
                        }
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "offsetDVolts", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        PMTOffset[3].Value = tmp;
                    }
                }
            }

            XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/DigitalOffset");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                XmlManager.GetAttribute(ndList[0], appDoc, "Visibility", ref str);

                DigOffsetVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
            }

            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/PulseMultiplexing");

            if (ndList.Count > 0 && ResourceManagerCS.Instance.IsThorDAQBoard)
            {
                string str = string.Empty;
                XmlManager.GetAttribute(ndList[0], appDoc, "Visibility", ref str);

                LSMPulseMultiplexingVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
            }
            else
            {
                LSMPulseMultiplexingVisibility = Visibility.Collapsed;
            }

            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/TurnAroundAlteration");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                XmlManager.GetAttribute(ndList[0], appDoc, "Visibility", ref str);

                TurnAroundOptionVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
            }
            else
            {
                TurnAroundOptionVisibility = Visibility.Collapsed;
            }

            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/FastOneWayImaging");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                XmlManager.GetAttribute(ndList[0], appDoc, "Visibility", ref str);

                FastOneWayImagingModeEnableVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
            }
            else
            {
                FastOneWayImagingModeEnableVisibility = Visibility.Collapsed;
            }


            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/LSMPixelProcess");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                XmlManager.GetAttribute(ndList[0], appDoc, "Visibility", ref str);

                LSMPixelProcessVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
            }
            else
            {
                LSMPixelProcessVisibility = Visibility.Collapsed;
            }

            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/Bipolarity");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                XmlManager.GetAttribute(ndList[0], appDoc, "Visibility", ref str);

                BipolarityVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
            }
            else
            {
                BipolarityVisibility = Visibility.Collapsed;
            }

            OnPropertyChange("");
        }

        public void OnPropertyChange(string propertyName)
        {
            if ((null != GetPropertyInfo(propertyName)) || (string.Empty == propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/LSM");

            XmlDocument hwDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndListHW = hwDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM");

            if (ndList.Count > 0)
            {
                if ((int)ICamera.CameraType.LSM == _scanControlModel.CameraType)
                {
                    ndList = experimentFile.SelectNodes("/ThorImageExperiment/LSM");
                    ndListHW = hwDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM");

                    if (ndList.Count > 0)
                    {
                        XmlNode node = ndList[0];

                        XmlManager.SetAttribute(ndList[0], experimentFile, "inputRange1", InputRange[0].Value.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "inputRange2", InputRange[1].Value.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "inputRange3", InputRange[2].Value.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "inputRange4", InputRange[3].Value.ToString());

                        XmlManager.SetAttribute(ndList[0], experimentFile, "polarity1", PMTPolarity[0].Value.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "polarity2", PMTPolarity[1].Value.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "polarity3", PMTPolarity[2].Value.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "polarity4", PMTPolarity[3].Value.ToString());

                        ndList[0].Attributes["averageMode"].Value = this.LSMSignalAverage.ToString();
                        ndList[0].Attributes["averageNum"].Value = this.LSMSignalAverageFrames.ToString();
                        ndList[0].Attributes["scanMode"].Value = this.LastLSMScanMode.ToString();
                        XmlManager.SetAttribute(ndList[0], experimentFile, "interleave", this.LSMInterleaveScan.ToString());
                        ndList[0].Attributes["twoWayAlignment"].Value = this.LSMTwoWayAlignment.ToString();
                        ndList[0].Attributes["clockSource"].Value = (this.LSMClockSource + 1).ToString();
                        int val = Convert.ToInt32(this.LSMExtClockRate * (double)Constants.US_TO_SEC);
                        ndList[0].Attributes["extClockRate"].Value = val.ToString();
                        _updateDwellTimeFromDevice = true;
                        XmlManager.SetAttribute(ndList[0], experimentFile, "dwellTime", this.LSMPixelDwellTime.ToString());

                        XmlManager.SetAttribute(ndList[0], experimentFile, "frameRate", this.FramesPerSecond.ToString());

                        XmlManager.SetAttribute(ndList[0], experimentFile, "flybackCycles", this.LSMFlybackCycles.ToString());

                        if (ResourceManagerCS.Instance.IsThorDAQBoard && (int)ThorSharedTypes.ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType())
                        {
                            // Only need to save turnAround to active.xml if the image detector is ThorDaqGG. For now, eventually this needs to become and option for any GG
                            XmlManager.SetAttribute(ndList[0], experimentFile, "turnAroundTimeUS", this.TurnAroundTimeUS.ToString());
                        }

                        XmlManager.SetAttribute(ndList[0], experimentFile, "minimizeFlybackCycles", (UseFastestSettingForFlybackCycles ? "1" : "0"));

                        //persist name from the active LSM in hardware settings:
                        if (ndListHW.Count > 0)
                        {
                            XmlManager.SetAttribute(ndList[0], experimentFile, "name", ResourceManagerCS.Instance.GetActiveLSMName());
                        }

                        XmlManager.SetAttribute(ndList[0], experimentFile, "chan1DigOffset", this.ChanDigOffset[0].Value.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "chan2DigOffset", this.ChanDigOffset[1].Value.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "chan3DigOffset", this.ChanDigOffset[2].Value.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "chan4DigOffset", this.ChanDigOffset[3].Value.ToString());

                        XmlManager.SetAttribute(ndList[0], experimentFile, "pulseMultiplexing", this.LSMPulseMultiplexing.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "externalClockPhaseOffset", this.LSMExternalClockPhaseOffset.ToString());

                        XmlManager.SetAttribute(ndList[0], experimentFile, "fastOnewayGGEnable", LSMFastOneWayImagingModeEnable ? "1" : "0");

                        if (ResourceManagerCS.Instance.GetActiveLSMName().Equals("GalvoGalvo"))
                        {
                            // Only need to save PixelProcess to active.xml if the image detector is GalvoGalvo.
                            XmlManager.SetAttribute(ndList[0], experimentFile, "LSMPixelProcess", this.LSMPixelProcess.ToString());
                        }
                    }
                }

            }

            ndList = experimentFile.SelectNodes("/ThorImageExperiment/PMT");

            if (ndList.Count > 0)
            {
                int lsmChannel = (int)MVMManager.Instance["CaptureSetupViewModel", "LSMChannel"];
                XmlManager.SetAttribute(ndList[0], experimentFile, "gainA", PMTGain[0].Value.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "enableA", ((lsmChannel == 0) || (lsmChannel == 4)) && (PMTGain[0].Value > 0) ? "1" : "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "bandwidthAHz", PMTBandwidth[0].Value.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "offsetAVolts", PMTOffset[0].Value.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "gainB", PMTGain[1].Value.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "enableB", ((lsmChannel == 1) || (lsmChannel == 4)) && (PMTGain[1].Value > 0) ? "1" : "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "bandwidthBHz", PMTBandwidth[1].Value.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "offsetBVolts", PMTOffset[1].Value.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "gainC", PMTGain[2].Value.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "enableC", ((lsmChannel == 2) || (lsmChannel == 4)) && (PMTGain[2].Value > 0) ? "1" : "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "bandwidthCHz", PMTBandwidth[2].Value.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "offsetCVolts", PMTOffset[2].Value.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "gainD", PMTGain[3].Value.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "enableD", ((lsmChannel == 3) || (lsmChannel == 4)) && (PMTGain[3].Value > 0) ? "1" : "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "bandwidthDHz", PMTBandwidth[3].Value.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "offsetDVolts", PMTOffset[3].Value.ToString());
            }

            if (ResourceManagerCS.GetLSMType() == (int)ICamera.LSMType.GALVO_RESONANCE && File.Exists("AlignData.txt"))
            {
                string textstring;
                StreamReader alignmentfile = File.OpenText("AlignData.txt");
                StreamWriter outalignmentfile = new StreamWriter("OutAlignData.txt");
                for (int i = 0; i <= 255; i++)
                {
                    textstring = alignmentfile.ReadLine();
                    if (i == (int)MVMManager.Instance["AreaControlViewModel", "LSMFieldSize", (object)5])
                    {
                        textstring = _scanControlModel.LSMTwoWayAlignment.ToString();
                    }
                    outalignmentfile.WriteLine(textstring);
                }
                alignmentfile.Close();
                outalignmentfile.Close();
                File.Replace("OutAlignData.txt", "AlignData.txt", null);
            }
        }

        //:TODO: Need to check if this is still required in a certain use case. Checking the speed of the Galvos should be moved to the lower level
        private void BuildDwellTimeTable()
        {
            //build the minimum dwell time table

            //This is table large enough to hold the dwell times for
            //the known scanning systems
            const int PIXELDENSITY_POINTS = 128;// 4096/32
            const int FIELD_SIZE_ARRAY_SIZE = 256;

            _minDwellTimeTable = new double[PIXELDENSITY_POINTS][];

            for (int i = 0; i < PIXELDENSITY_POINTS; i++)
            {
                _minDwellTimeTable[i] = new double[FIELD_SIZE_ARRAY_SIZE];
            }

            const int START_LOC_FOR_EXTRAPOLATION = 5;
            //derived by taking Dwelltime range (9.6 - .6) / FieldSize range (255-10)
            //table for minimum values was determined empiracally
            double dwellTimeInitialStepPerFieldSize = (9.6 - .6) / (FIELD_SIZE_ARRAY_SIZE - 1 - START_LOC_FOR_EXTRAPOLATION);

            double dwellTimeSlopeChange = .007179487 / dwellTimeInitialStepPerFieldSize / PIXELDENSITY_POINTS;

            //setup first valid dwell times for pixelX 32

            const int FINAL_START_POSITION = 50;

            for (int j = 0; j < PIXELDENSITY_POINTS; j++)
            {
                for (int i = 0; i < FIELD_SIZE_ARRAY_SIZE; i++)
                {
                    int rampStartPosition = (START_LOC_FOR_EXTRAPOLATION + (FINAL_START_POSITION / START_LOC_FOR_EXTRAPOLATION * j));
                    if (i > rampStartPosition)
                    {
                        //Round the value to the nearest value
                        _minDwellTimeTable[j][i] = _scanControlModel.LSMPixelDwellTimeStep * Math.Round((.6 + (i - rampStartPosition) * (dwellTimeInitialStepPerFieldSize - dwellTimeSlopeChange * j)) / LSMPixelDwellTimeStep, 0);
                    }
                    else
                    {
                        _minDwellTimeTable[j][i] = _scanControlModel.LSMPixelDwellTimeMin;
                    }
                }
            }
        }

        private void ConfirmDwellTime(ref double newDwellTime)
        {
            int val = 0;
            ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref val);
            if ((int)ICamera.LSMType.GALVO_GALVO != val)
            {
                return;
            }
            int channel = 0;
            double dwellFactor = 0;
            int lines = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)32];

            switch ((ICamera.LSMAreaMode)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0])
            {
                case ICamera.LSMAreaMode.LINE:
                    {
                        channel = (3 < (int)MVMManager.Instance["CaptureSetupViewModel", "LSMChannel"]) ? 4 : 1;
                        dwellFactor = 0.24; //0.24 was determined experimentally
                    }
                    break;
                case ICamera.LSMAreaMode.POLYLINE:
                    {
                        channel = 1; //channel buffer size has no effect for polyline acquisition. Thus, it is not being considered
                        dwellFactor = 0.72; //0.72 was determined experimentally
                    }
                    break;
                default:
                    return;
            }
            if (ICamera.LSMAreaMode.LINE == (ICamera.LSMAreaMode)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0] ||
                ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0])
            {

                int pixelX = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)32];
                double tmpDwellTime = newDwellTime;

                //Ensure the dwell time is long enough for single line scan
                double m = lines * tmpDwellTime * pixelX * channel / 1000; //ms
                if (dwellFactor > m)
                {
                    while (true)
                    {
                        tmpDwellTime += this.LSMPixelDwellTimeStep;
                        m = lines * tmpDwellTime * pixelX * channel / 1000; //ms
                        if (dwellFactor <= m)
                        {
                            break;
                        }
                    }
                    newDwellTime = Math.Round(tmpDwellTime, 1);
                }
            }
        }

        void GenerateBandwidthList()
        {
            if (_bandwidthList.Count > 0)
            {
                for (int i = 0; i < _bandwidthList.Count; i++)
                {
                    _bandwidthList[i].Clear();
                }
                _bandwidthList.Clear();
            }

            for (int i = 0; i < NUM_DETECTORS; i++)
            {
                ObservableCollection<string> tempList = new ObservableCollection<string>();

                foreach (DetectorBandwidths k in DetectorBandwidths.GetValues(typeof(DetectorBandwidths)))
                {
                    //Try to set the bandwidth value to the PMT
                    PMTBandwidth[i].Value = (int)k;
                    //If the PMT supports that bandwidth it will return the bandwidth or the closest one it can get to
                    if ((int)k == PMTBandwidth[i].Value)
                    {
                        tempList.Add(_bandwidthToStringMap[(int)k]);
                    }
                }

                _bandwidthList.Add(tempList);
            }
        }

        private int GetPMTMax(int pmtIndex)
        {
            switch (pmtIndex)
            {
                case 0: return _scanControlModel.PMT1GainMax;
                case 1: return _scanControlModel.PMT2GainMax;
                case 2: return _scanControlModel.PMT3GainMax;
                case 3: return _scanControlModel.PMT4GainMax;
                default: return 0;
            }
        }

        private int GetPMTMin(int pmtIndex)
        {
            switch (pmtIndex)
            {
                case 0: return _scanControlModel.PMT1GainMin;
                case 1: return _scanControlModel.PMT2GainMin;
                case 2: return _scanControlModel.PMT3GainMin;
                case 3: return _scanControlModel.PMT4GainMin;
                default: return 0;
            }
        }

        private void InitializeProperties()
        {
            ChanDigOffset = new CustomCollection<HwVal<int>>();
            ChanDigOffsetVisibility = new CustomCollection<Visibility>();
            InputRange = new CustomCollection<HwVal<int>>();
            IsChannelVisible = new CustomCollection<Visibility>();
            PMTBandwidth = new CustomCollection<HwVal<int>>();
            PmtBandwidthVisibility = new CustomCollection<Visibility>();
            PMTGain = new CustomCollection<HwVal<int>>();
            PMTGainEnable = new CustomCollection<HwVal<int>>();
            PMTOffset = new CustomCollection<HwVal<double>>();
            PMTOffsetVisibility = new CustomCollection<Visibility>();
            PMTOn = new CustomCollection<HwVal<int>>();
            PMTPolarity = new CustomCollection<HwVal<int>>();
            PMTVolt = new CustomCollection<DoublePC>();
            PMTDetectorType = new CustomCollection<HwVal<int>>();

            int j = 0;
            foreach (DetectorBandwidths i in DetectorBandwidths.GetValues(typeof(DetectorBandwidths)))
            {
                _stringToBandwidthMap.Add(_bandwidthTags[j], (int)i);
                _bandwidthToStringMap.Add((int)i, _bandwidthTags[j]);
                j++;
            }

            for (int i = 0; i < NUM_DETECTORS; i++)
            {
                HwVal<int> digOffset = new HwVal<int>(i, (int)SelectedHardware.SELECTED_CAMERA1, (int)Enum.Parse(typeof(ICamera.Params), string.Format("PARAM_LSM_DIG_OFFSET_{0}", i)));
                ChanDigOffset.Add(digOffset);

                ChanDigOffsetVisibility.Add(_digOffsetVisibility);  //modified

                HwVal<int> inputRange = new HwVal<int>(i, (int)SelectedHardware.SELECTED_CAMERA1, (int)Enum.Parse(typeof(ICamera.Params), string.Format("PARAM_LSM_INPUTRANGE{0}", i + 1)));
                inputRange.AdditionalSetLogic = (x, y) => SetInputRangeAdditionalLogic(x, y);
                InputRange.Add(inputRange);

                IsChannelVisible.Add(Visibility.Collapsed);

                HwVal<int> pmtBandwidth = new HwVal<int>(i,
                                                         (int)Enum.Parse(typeof(SelectedHardware), string.Format("SELECTED_PMT{0}", i + 1)),
                                                         new int[2] { (int)Enum.Parse(typeof(IDevice.Params), string.Format("PARAM_PMT{0}_BANDWIDTH_POS_CURRENT", i + 1)), (int)Enum.Parse(typeof(IDevice.Params), string.Format("PARAM_PMT{0}_BANDWIDTH_POS", i + 1)) },
                                                         (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                PMTBandwidth.Add(pmtBandwidth);

                PmtBandwidthVisibility.Add(_pmtBandwidthLabelVisibility);   //modified from collapsed

                HwVal<int> pmtGain = new HwVal<int>(i,
                                                    (int)Enum.Parse(typeof(SelectedHardware), string.Format("SELECTED_PMT{0}", i + 1)),
                                                    (int)Enum.Parse(typeof(IDevice.Params), string.Format("PARAM_PMT{0}_GAIN_POS", i + 1)),
                                                    (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                pmtGain.AdditionalSetLogic = (x, y) => SetGainAdditionalLogic(x, y);
                PMTGain.Add(pmtGain);

                HwVal<int> pmtGainEnable = new HwVal<int>(i,
                                                            (int)Enum.Parse(typeof(SelectedHardware), string.Format("SELECTED_PMT{0}", i + 1)),
                                                            (int)Enum.Parse(typeof(IDevice.Params), string.Format("PARAM_PMT{0}_ENABLE", i + 1)),
                                                            (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                PMTGainEnable.Add(pmtGainEnable);

                HwVal<double> pmtOffset = new HwVal<double>(i,
                                                            (int)Enum.Parse(typeof(SelectedHardware), string.Format("SELECTED_PMT{0}", i + 1)),
                                                            new int[2] { (int)Enum.Parse(typeof(IDevice.Params), string.Format("PARAM_PMT{0}_OUTPUT_OFFSET_CURRENT", i + 1)), (int)Enum.Parse(typeof(IDevice.Params), string.Format("PARAM_PMT{0}_OUTPUT_OFFSET", i + 1)) },
                                                            (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                pmtOffset.AdditionalSetLogic = (x, y) => SetOffsetAdditionalLogic(x, y);
                PMTOffset.Add(pmtOffset);

                PMTOffsetVisibility.Add(_pmtOffsetLabelVisibility);  //modified from Collapsed

                HwVal<int> pmtOn = new HwVal<int>(i,
                                                  (int)Enum.Parse(typeof(SelectedHardware), string.Format("SELECTED_PMT{0}", i + 1)),
                                                  (int)Enum.Parse(typeof(IDevice.Params), string.Format("PARAM_PMT{0}_ENABLE", i + 1)),
                                                  (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                PMTOn.Add(pmtOn);

                HwVal<int> pmtPolarity = new HwVal<int>(i, (int)SelectedHardware.SELECTED_CAMERA1, (int)Enum.Parse(typeof(ICamera.Params), string.Format("PARAM_LSM_CHANNEL_POLARITY_{0}", i + 1)));
                PMTPolarity.Add(pmtPolarity);

                DoublePC pmtVolt = new DoublePC();
                PMTVolt.Add(pmtVolt);

                HwVal<int> pmtDetectorType = new HwVal<int>(i,
                                                  (int)Enum.Parse(typeof(SelectedHardware), string.Format("SELECTED_PMT{0}", i + 1)),
                                                  (int)Enum.Parse(typeof(IDevice.Params), string.Format("PARAM_PMT{0}_DETECTOR_TYPE", i + 1)),
                                                  (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                PMTDetectorType.Add(pmtDetectorType);
            }
        }

        private void LoadPixelCountXY(XmlNodeList ndList)
        {
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            string str1 = string.Empty;
            string str2 = string.Empty;
            if (XmlManager.GetAttribute(ndList[0], doc, "pixelX", ref str1) &&
                XmlManager.GetAttribute(ndList[0], doc, "pixelY", ref str2))
            {
                int x = 512;
                int y = 512;
                int tmp = 0;
                if (Int32.TryParse(str1, out tmp))
                {
                    x = tmp;
                }
                if (Int32.TryParse(str2, out tmp))
                {
                    y = tmp;
                }
                MVMManager.Instance["AreaControlViewModel", "LSMPixelX"] = x;
                MVMManager.Instance["AreaControlViewModel", "LSMPixelY"] = y;
            }
        }

        private void LoadPMTHardwareSettings()
        {
            XmlDocument hwDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndList = hwDoc.SelectNodes("/HardwareSettings/PMT");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], hwDoc, "typeA", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTMode[0] = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], hwDoc, "typeB", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTMode[1] = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], hwDoc, "typeC", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTMode[2] = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], hwDoc, "typeD", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTMode[3] = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], hwDoc, "tripCount", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PMTTripCount = tmp;
                    }
                }

            }

            PMTVoltage = new double[2, 101];
            // debug to check p
            string p = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + @"\PMT_Voltage_Mapping.txt";

            if (false == File.Exists(p))
            {
                return;
            }

            var reader = new StreamReader(File.OpenRead(p));
            int i = 0;
            while (!reader.EndOfStream)
            {
                var l = reader.ReadLine();
                var values = l.Split(',');
                if (values.Length != 101)
                {
                    // something wrong;
                }
                for (int j = 0; j < 101; j++)
                {
                    double tmp = 0;
                    if (Double.TryParse(values[j], NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        PMTVoltage[i, j] = tmp;
                    }
                }
                i++;
                if (i > 1)
                {
                    // something wrong;
                    break;
                }
            }

            PmtBandwidthVisibility.Clear();
            int bwAvailable = 0;
            int[] selectedChannel = new int[NUM_DETECTORS];

            for (int k = 0; k < NUM_DETECTORS; k++)
            {
                int bwPos = 0;

                if (_scanControlModel.GetPMTBandwidthIsAvailable(k, ref bwPos))
                {
                    bwAvailable = 1;
                    selectedChannel[k] = 1;
                }
            }

            PmtBandwidthVisibility.Clear();

            for (i = 0; i < NUM_DETECTORS; i++)
            {
                int bwPos = 0;

                if (_scanControlModel.GetPMTBandwidthIsAvailable(i, ref bwPos))
                {
                    PMTBandwidthLabelVisibility = Visibility.Visible;
                    PmtBandwidthVisibility.Add(Visibility.Visible);
                }
                else
                {
                    if ((_pmtBandwidthLabelVisibility == Visibility.Visible) && (bwAvailable == 1))
                    {
                        PmtBandwidthVisibility.Clear();
                        PMTBandwidthLabelVisibility = Visibility.Visible;
                        for (int j = 0; j < NUM_DETECTORS; j++)
                        {
                            if (selectedChannel[j] == 1)
                            {

                                PmtBandwidthVisibility.Add(Visibility.Visible);
                            }
                            else
                            {
                                PmtBandwidthVisibility.Add(Visibility.Collapsed);
                            }
                        }
                    }
                    else
                    {
                        if (bwAvailable == 0)
                        {
                            PMTBandwidthLabelVisibility = Visibility.Collapsed;
                            PmtBandwidthVisibility.Add(Visibility.Collapsed);
                        }
                        else
                        {
                            PmtBandwidthVisibility.Add(Visibility.Hidden);
                            PMTBandwidthLabelVisibility = Visibility.Hidden;
                        }
                    }

                }
                SetOffsetAdditionalLogic(i, 0);
            }
        }

        private void LSMDwellTimeMinusCommandFnc()
        {
            if (1 == (int)MVMManager.Instance["ThreePhotonControlViewModel", "ThreePhotonEnable", (object)0])
            {
                LSMPixelDwellTime = LSMPixelDwellTime - LSMPixelDwellTimeStep;
            }
            else
            {
                LSMPixelDwellTime--;
            }
        }

        private void LSMDwellTimePlusCommandFnc()
        {
            if (1 == (int)MVMManager.Instance["ThreePhotonControlViewModel", "ThreePhotonEnable", (object)0])
            {
                LSMPixelDwellTime = LSMPixelDwellTime + LSMPixelDwellTimeStep;
            }
            else
            {
                LSMPixelDwellTime++;
            }
        }

        private void SetGainAdditionalLogic(int index, int val)
        {
            int min = GetPMTMin(index);
            int max = GetPMTMax(index);

            if (min > val)
            {
                PMTGain[index].Value = min;
                return;
            }
            if (max < val)
            {
                PMTGain[index].Value = max;
                return;
            }

            if (true == (bool)MVMManager.Instance["CaptureSetupViewModel", "IsLive", (object)false])
            {
                PMTGainEnable[index].Value = (0 == val) ? 0 : 1;
            }

            if (PMTVoltage != null)
            {
                if (PMTVoltage.GetLength(1) > val)
                {
                    PMTVolt[index].Value = PMTVoltage[PMTMode[index], val];
                }
            }
        }

        private void SetInputRangeAdditionalLogic(int index, int val)
        {
            if (ResourceManagerCS.Instance.IsThorDAQBoard)
            {
                if (val > InputRangeMax || val < InputRangeMin)
                {
                    // For Thordaq, if input range is outside of range (0-8), set it to 1.5V
                    InputRange[index].Value = 3;
                }
            }
            else
            {
                if (5 != val && 6 != val && 7 != val && 10 != val && 11 != val && 12 != val)
                {
                    // For Alazar, if input range is not one of the valid values, set it to 1V
                    InputRange[index].Value = 10;
                }
            }
        }

        private void SetOffsetAdditionalLogic(int index, double val)
        {
            int offset = 0;
            if (1 == ResourceManagerCS.GetDeviceParamInt((int)Enum.Parse(typeof(SelectedHardware),
                                                         string.Format("SELECTED_PMT{0}", index + 1)),
                                                         (int)Enum.Parse(typeof(IDevice.Params),
                                                         string.Format("PARAM_PMT{0}_OUTPUT_OFFSET", index + 1)),
                                                         ref offset))
            {
                PMTOffsetVisibility[index] = Visibility.Visible;
                PMTOffsetLabelVisibility = Visibility.Visible;
                _pmtoffsetavailable = 1;
            }
            else
            {
                if (_pmtOffsetLabelVisibility == Visibility.Visible && (_pmtoffsetavailable == 1))
                {
                    PMTOffsetVisibility[index] = Visibility.Collapsed;
                }
                else
                {
                    PMTOffsetLabelVisibility = Visibility.Collapsed;
                    PMTOffsetVisibility[index] = Visibility.Collapsed;
                    _pmtoffsetavailable = 0;
                }

            }
            if (index == (NUM_DETECTORS - 1))
            {
                for (int i = 0; i < NUM_DETECTORS; i++)
                {
                    if (_pmtoffsetavailable == 1)
                    {
                        if (PMTOffsetVisibility[i] == Visibility.Collapsed)
                        {
                            PMTOffsetVisibility[i] = Visibility.Collapsed;
                        }
                    }
                }
            }
        }

        private void TwoWayCalibration()
        {
            if (_twoWayDialog.IsLoaded == false)
            {
                //Don't enable any capture while loading the twoway aligngment window
                MVMManager.Instance["CaptureSetupViewModel", "EnableCapture"] = false;

                //stop the current capture
                ((ICommand)MVMManager.Instance["CaptureSetupViewModel", "StopCommand"]).Execute(null);

                LSMTwoWayAlignment = 0;

                if (Visibility.Visible == CoarsePanelVisibility)
                {
                    LSMTwoWayAlignmentCoarse = 0;
                }

                _twoWayDialog = new TwoWaySettings();
                _twoWayDialog.Closing += new System.ComponentModel.CancelEventHandler(_twoWayDialog_Closing);
                _twoWayDialog.Closed += _twoWayDialog_Closed;
                _twoWayDialog.EnableImaging += _twoWayDialog_EnableImaging;
                _twoWayDialog.DataContext = this;
                int lsmType = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);

                if ((int)ICamera.LSMType.GALVO_GALVO == lsmType)
                {
                    _twoWayDialog.AlignMode = 2;
                    _twoWayDialog.DwellMin = LSMPixelDwellTimeMin;
                    _twoWayDialog.DwellStep = LSMPixelDwellTimeStep;
                }
                else
                {
                    _twoWayDialog.AlignMode = (Visibility.Visible == CoarsePanelVisibility) ? 1 : 0;
                }

                _twoWayDialog.Show();

                PMTGain[0].Value = 0;
                PMTGain[1].Value = 0;
                PMTGain[2].Value = 0;
                PMTGain[3].Value = 0;

                //The polarity might have been reset in the lower level
                //Read it back so the GUI matches
                PMTPolarity[0].Value = PMTPolarity[0].Value;
                PMTPolarity[1].Value = PMTPolarity[1].Value;
                PMTPolarity[2].Value = PMTPolarity[2].Value;
                PMTPolarity[3].Value = PMTPolarity[3].Value;

                //Update the offset GUI in case it has been changed in the lower level
                PMTOffset[0].Value = PMTOffset[0].Value;
                PMTOffset[1].Value = PMTOffset[1].Value;
                PMTOffset[2].Value = PMTOffset[2].Value;
                PMTOffset[3].Value = PMTOffset[3].Value;

                //Ensure multiphoton shutters are closed
                MVMManager.Instance["MultiphotonControlViewModel", "LaserShutterPosition"] = 0;
                MVMManager.Instance["MultiphotonControlViewModel", "LaserShutter2Position"] = 0;

                //Ensure MCLS lasers are disabled
                MVMManager.Instance["LaserControlViewModel", "MainLaserIndex"] = 0;
                MVMManager.Instance["LaserControlViewModel", "Laser1Enable"] = 0;
                MVMManager.Instance["LaserControlViewModel", "Laser2Enable"] = 0;
                MVMManager.Instance["LaserControlViewModel", "Laser3Enable"] = 0;
                MVMManager.Instance["LaserControlViewModel", "Laser4Enable"] = 0;

                //Update LightPath Position
                MVMManager.Instance["CaptureSetupViewModel", "LightPathGGEnable"] = 0;
                MVMManager.Instance["CaptureSetupViewModel", "LightPathGREnable"] = 0;
                MVMManager.Instance["CaptureSetupViewModel", "LightPathCamEnable"] = 0;

            }
        }

        void _twoWayDialog_Closed(object sender, EventArgs e)
        {
            //The polarity might have been reset in the lower level
            //Read it back so the GUI matches
            PMTPolarity[0].Value = PMTPolarity[0].Value;
            PMTPolarity[1].Value = PMTPolarity[1].Value;
            PMTPolarity[2].Value = PMTPolarity[2].Value;
            PMTPolarity[3].Value = PMTPolarity[3].Value;
        }

        void _twoWayDialog_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            //Don't enable any capture while closing the twoway aligngment window
            MVMManager.Instance["CaptureSetupViewModel", "EnableCapture"] = false;

            if (true == _twoWayDialog.Apply)
            {
                LSMTwoWayAlignment = 0;
            }

            if (true == (bool)MVMManager.Instance["CaptureSetupViewModel", "IsLive"])
            {
                ((ICommand)MVMManager.Instance["CaptureSetupViewModel", "StopCommand"]).Execute(null);
            }

            PMTGain[0].Value = 0;
            PMTGain[1].Value = 0;
            PMTGain[2].Value = 0;
            PMTGain[3].Value = 0;

            //Ensure multiphoton shutters are closed
            MVMManager.Instance["MultiphotonControlViewModel", "LaserShutterPosition"] = 0;
            MVMManager.Instance["MultiphotonControlViewModel", "LaserShutter2Position"] = 0;

            //Ensure MCLS lasers are disabled
            MVMManager.Instance["LaserControlViewModel", "MainLaserIndex"] = 0;
            MVMManager.Instance["LaserControlViewModel", "Laser1Enable"] = 0;
            MVMManager.Instance["LaserControlViewModel", "Laser2Enable"] = 0;
            MVMManager.Instance["LaserControlViewModel", "Laser3Enable"] = 0;
            MVMManager.Instance["LaserControlViewModel", "Laser4Enable"] = 0;

            //Update LightPath Position
            MVMManager.Instance["CaptureSetupViewModel", "LightPathGGEnable"] = 0;
            MVMManager.Instance["CaptureSetupViewModel", "LightPathGREnable"] = 0;
            MVMManager.Instance["CaptureSetupViewModel", "LightPathCamEnable"] = 0;
        }

        void _twoWayDialog_EnableImaging(bool obj)
        {
            MVMManager.Instance["CaptureSetupViewModel", "EnableCapture"] = obj;
        }

        #endregion Methods
    }
}