namespace ZControl.ViewModel
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Xml;

    using ThorLogging;

    using ThorSharedTypes;

    using ZControl;
    using ZControl.Model;

    public class ZControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        public static Dictionary<int, string> LocationDictionary = new Dictionary<int, string>() 
        {
        {0, "location1"},
        {1, "location2"},
        {2, "location3"},
        {3, "location4"},
        {4, "location5"},
        {5, "location6"},
        };
        public static Dictionary<int, string> LocationNameDictionary = new Dictionary<int, string>() 
        {
        {0, "locationName1"},
        {1, "locationName2"},
        {2, "locationName3"},
        {3, "locationName4"},
        {4, "locationName5"},
        {5, "locationName6"},
        };
        public static Dictionary<string, ZCommandType> ZCommandTypeDictionary = new Dictionary<string, ZCommandType>()
        {
        {"Z_STEPSIZE_COARSE", ZCommandType.Z_STEPSIZE_COARSE},
        {"Z_STEPSIZE_FINE", ZCommandType.Z_STEPSIZE_FINE},
        {"Z2_STEPSIZE_COARSE", ZCommandType.Z2_STEPSIZE_COARSE},
        {"Z2_STEPSIZE_FINE", ZCommandType.Z2_STEPSIZE_FINE},
        {"GO_Z", ZCommandType.GO_Z},
        {"GO_Z2", ZCommandType.GO_Z2},
        {"GO_Z_CENTER", ZCommandType.GO_Z_CENTER},
        {"GO_Z_SCANSTART", ZCommandType.GO_Z_SCANSTART},
        {"GO_Z_SCANSTOP", ZCommandType.GO_Z_SCANSTOP},
        {"STOP_Z", ZCommandType.STOP_Z},
        {"STOP_Z2", ZCommandType.STOP_Z2},
        {"SET_Z_ZERO", ZCommandType.SET_Z_ZERO},
        {"SET_Z2_ZERO", ZCommandType.SET_Z2_ZERO},
        {"SET_Z_SCANSTART", ZCommandType.SET_Z_SCANSTART},
        {"SET_Z_SCANSTOP", ZCommandType.SET_Z_SCANSTOP},
        {"Z_POS_PLUS", ZCommandType.Z_POS_PLUS},
        {"Z_POS_MINUS", ZCommandType.Z_POS_MINUS},
        {"Z2_POS_PLUS", ZCommandType.Z2_POS_PLUS},
        {"Z2_POS_MINUS", ZCommandType.Z2_POS_MINUS}
        };

        const double MAX_Z_STEP_SIZE = 1;
        const double MIN_Z_STEP_SIZE = .0001;

        private bool _isZCaptureStopped = false;
        private double _lastZPositionVal = 0.0;
        private CustomCollection<DateTime> _lastZSetTime = new CustomCollection<DateTime>(new DateTime[3] { DateTime.Now, DateTime.Now, DateTime.Now }); //Z, Z2, R
        private ICommand _previewZStackCommand;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private bool _z2Invert = false;
        private bool _z2InvertLimits = false;
        private string _z2PosMinusKey;
        private string _z2PosMinusModifier;
        private string _z2PosPlusKey;
        private string _z2PosPlusModifier;
        private double _z2StepSize = .0100;
        private string _z2StopKey;
        private string _z2StopModifier;
        private string _z2ZeroKey;
        private string _z2ZeroModifier;
        private double _zCenter;
        private ICommand _zCommand;
        private ZControlModel _zControlModel;
        private double[] _zGotoValue = { 0.0, 0.0 };
        private bool _zInvert = false;
        private bool _zInvertLimits = false;
        private string _zPosMinusKey;
        private string _zPosMinusModifier;
        private string _zPosPlusKey;
        private string _zPosPlusModifier;
        private int _zRangeDirection = 0;
        private double _zRangeMax;
        private double _zRangeMin;
        private int _zScanNumSteps;
        private double _zScanStart;
        private double _zScanStep;
        private double _zScanStop;
        private bool _zScanStopNotValid = false;
        private double _zScanThickness;
        private string _zStackCacheDirectory;
        private ICommand _zStage2GoToLocationCommand;
        private CustomCollection<string> _zStage2LocationNames = new CustomCollection<string>(new List<string>() { string.Empty, string.Empty, string.Empty, string.Empty, string.Empty, string.Empty });
        private ICommand _zStage2LocationSaveCommand;
        private string _zStartKey;
        private string _zStartModifier;
        private double _zStepSize;
        private string _zStopKey;
        private string _zStopModifier;
        private string _zZeroKey;
        private string _zZeroModifier;

        #endregion Fields

        #region Constructors

        public ZControlViewModel()
        {
            _zControlModel = new ZControlModel();
            _zScanStart = 0;
            _zScanStop = 0;
            _zScanStep = 1;
            _zScanNumSteps = 1;
            _zRangeMax = 0;
            _zRangeMin = 0;
            _zStepSize = .0100;
        }

        #endregion Constructors

        #region Enumerations

        public enum ZCommandType
        {
            Z_STEPSIZE_COARSE,
            Z_STEPSIZE_FINE,
            Z2_STEPSIZE_COARSE,
            Z2_STEPSIZE_FINE,
            GO_Z,
            GO_Z2,
            GO_Z_CENTER,
            GO_Z_SCANSTART,
            GO_Z_SCANSTOP,
            STOP_Z,
            STOP_Z2,
            SET_Z_ZERO,
            SET_Z2_ZERO,
            SET_Z_SCANSTART,
            SET_Z_SCANSTOP,
            Z_POS_PLUS,
            Z_POS_MINUS,
            Z2_POS_PLUS,
            Z2_POS_MINUS
        }

        #endregion Enumerations

        #region Properties

        public CustomCollection<bool> EnableRead
        {
            get { return _zControlModel.EnableRead; }
        }

        public bool IsZCaptureStopped
        {
            get { return _isZCaptureStopped; }
            set { _isZCaptureStopped = value; OnPropertyChanged("IsZCaptureStopped"); }
        }

        public bool IsZStackCapturing
        {
            get
            {
                return _zControlModel.IsZStackCapturing;
            }
            set
            {
                _zControlModel.IsZStackCapturing = value;
                OnPropertyChanged("IsZStackCapturing");
            }
        }

        public CustomCollection<DateTime> LastUpdateTime
        {
            get { return _zControlModel.LastUpdateTime; }
            set { _zControlModel.LastUpdateTime = value; OnPropertyChanged("LastZUpdateTime"); }
        }

        public Dictionary<int, string> LocationNames
        {
            get { return LocationNameDictionary; }
        }

        public bool PreviewButtonEnabled
        {
            get
            {
                return ((bool)MVMManager.Instance["CaptureSetupViewModel", "LiveStartButtonStatus", (object)false] &&
                    (bool)MVMManager.Instance["CaptureSetupViewModel", "IsProgressWindowOff", (object)false] &&
                    (bool)MVMManager.Instance["CaptureSetupViewModel", "WrapPanelEnabled", (object)false]);
            }
        }

        public ICommand PreviewZStackCommand
        {
            get
            {
                if (this._previewZStackCommand == null)
                    this._previewZStackCommand = new RelayCommand(() => StartZStackPreview());

                return this._previewZStackCommand;
            }
        }

        public double RPosition
        {
            get
            {
                return _zControlModel.RPosition;
            }
            set
            {
                TimeSpan ts = DateTime.Now - _lastZSetTime[2];

                if (ts.TotalSeconds > .25)
                {
                    _zControlModel.RPosition = value;

                    OnPropertyChanged("RPosition");

                    _lastZSetTime[2] = DateTime.Now;
                }
            }
        }

        public Visibility RStageVisibility
        {
            get
            {
                return (_zControlModel.RStageAvailable) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public double SecondaryZScale
        {
            get
            {
                return (ResourceManagerCS.Instance.TabletModeEnabled) ? 1.13 : 1;
            }
        }

        public string UpdatePositions
        {
            set
            {
                _zControlModel.UpdatePositions = value;
            }
        }

        public double VolumeSpacingZ
        {
            get
            {
                return ZScanStep / ((double)MVMManager.Instance["AreaControlViewModel", "MMPerPixel", (object)0] * 1000);
            }
        }

        public bool Z2Invert
        {
            get
            {
                return _z2Invert;
            }
            set
            {
                _z2Invert = value;
                OnPropertyChanged("Z2Invert");
                OnPropertyChanged("Z2InvertLimits");
                OnPropertyChanged("Z2Position");
                PersistZ2Invert();
            }
        }

        public bool Z2InvertLimits
        {
            get
            {
                if (_z2InvertLimits && Z2Invert)
                {
                    return false;
                }
                else
                {
                    return (_z2InvertLimits || Z2Invert);
                }
            }
            set
            {
                _z2InvertLimits = value;
                OnPropertyChanged("ZPosition");
            }
        }

        public double Z2Max
        {
            get
            {
                return _zControlModel.Z2Max;
            }
        }

        public double Z2Min
        {
            get
            {
                return _zControlModel.Z2Min;
            }
        }

        public double Z2Position
        {
            get
            {
                return _zControlModel.Z2Position;
            }
            set
            {
                TimeSpan ts = DateTime.Now - _lastZSetTime[1];

                if (ts.TotalSeconds > .25)
                {
                    _zControlModel.Z2Position = value;

                    OnPropertyChanged("Z2Position");
                    OnPropertyChanged("Z2PositionBar");
                    OnPropertyChanged("Z2PosOutOfBounds");

                    _lastZSetTime[1] = DateTime.Now;
                }
            }
        }

        public double Z2PositionBar
        {
            get
            {
                if (_z2InvertLimits)
                {
                    return -_zControlModel.Z2Position;
                }
                else
                {
                    return _zControlModel.Z2Position;
                }
            }
            set
            {
                //This is needed otherwise will crash at startup because
                // setter is missing and ZPosition is configured two-way/one-way
            }
        }

        public string Z2PosMinusKey
        {
            get { return _z2PosMinusKey; }
            set { _z2PosMinusKey = value; OnPropertyChanged("Z2PosMinusKey"); }
        }

        public string Z2PosMinusModifier
        {
            get { return _z2PosMinusModifier; }
            set { _z2PosMinusModifier = value; OnPropertyChanged("Z2PosMinusModifier"); }
        }

        public bool Z2PosOutOfBounds
        {
            get
            {
                bool ret;
                if (_zControlModel.Z2Position < _zControlModel.Z2Min || _zControlModel.Z2Position > _zControlModel.Z2Max)
                    ret = true;
                else
                    ret = false;
                return ret;
            }
        }

        public string Z2PosPlusKey
        {
            get { return _z2PosPlusKey; }
            set { _z2PosPlusKey = value; OnPropertyChanged("Z2PosPlusKey"); }
        }

        public string Z2PosPlusModifier
        {
            get { return _z2PosPlusModifier; }
            set { _z2PosPlusModifier = value; OnPropertyChanged("Z2PosPlusModifier"); }
        }

        public double Z2SliderStepSize
        {
            get
            {
                return (Z2Max - Z2Min) / 20.0;
            }
        }

        public Visibility Z2StageVisibility
        {
            get
            {
                return (_zControlModel.Z2StageAvailable) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public double Z2StepSize
        {
            get
            {
                return _z2StepSize;
            }
            set
            {
                Decimal dec = new Decimal(value);
                _z2StepSize = Decimal.ToDouble(Decimal.Round(dec, 4));
                OnPropertyChanged("Z2StepSize");
            }
        }

        public string Z2StopKey
        {
            get { return _z2StopKey; }
            set { _z2StopKey = value; OnPropertyChanged("Z2StopKey"); }
        }

        public string Z2StopModifier
        {
            get { return _z2StopModifier; }
            set { _z2StopModifier = value; OnPropertyChanged("Z2StopModifier"); }
        }

        public string Z2ZeroKey
        {
            get { return _z2ZeroKey; }
            set { _z2ZeroKey = value; OnPropertyChanged("Z2ZeroKey"); }
        }

        public string Z2ZeroModifier
        {
            get { return _z2ZeroModifier; }
            set { _z2ZeroModifier = value; OnPropertyChanged("Z2ZeroModifier"); }
        }

        public Visibility Z2ZeroVisibility
        {
            get
            {
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ZView");
                if (ndList.Count > 0)
                {
                    string tmp = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], appSettings, "z2ZeroVisibility", ref tmp))
                    {
                        return tmp.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }
                    else
                    {
                        return Visibility.Collapsed;
                    }
                }

                return Visibility.Collapsed;
            }
        }

        public double ZCenter
        {
            get
            {
                _zCenter = (_zScanStart + _zScanStop) / 2;
                return _zCenter = Double.Parse(String.Format("{0:0.####}", _zCenter));
            }
        }

        public ICommand ZCommand
        {
            get
            {
                if (this._zCommand == null)
                    this._zCommand = new RelayCommandWithParam((x) => ZCommands(x));

                return this._zCommand;
            }
        }

        public double[] ZGotoValue
        {
            get { return _zGotoValue; }
            set
            {
                _zGotoValue = value;
                OnPropertyChanged("ZGotoValue");
            }
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
                OnPropertyChanged("ZInvert");
                OnPropertyChanged("ZInvertLimits");
                OnPropertyChanged("ZPosition");
                PersistZInvert();
            }
        }

        /// <summary>
        /// Gets a value indicating whether [z invert device].
        /// This value indicates if the device has inverted its coordinate
        /// system. If true set the Min/Max values for display text boxes opposite to the
        /// default icon Graphic
        /// </summary>
        /// <value><c>true</c> if [z invert device]; otherwise, <c>false</c>.</value>
        public bool ZInvertDevice
        {
            get
            {
                return _zControlModel.ZInvertDevice;
            }
        }

        /// <summary>
        /// Gets a value indicating whether [z invert device].
        /// This value indicates if the device has inverted its coordinate
        /// system. If true set the Min/Max values for display text boxes opposite to the
        /// default icon Graphic
        /// </summary>
        /// <value><c>true</c> if [z invert device]; otherwise, <c>false</c>.</value>
        public bool ZInvertDevice2
        {
            get
            {
                return _zControlModel.ZInvertDevice2;
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
                    return (_zInvertLimits || ZInvert);
                }
            }
            set
            {
                _zInvertLimits = value;
                OnPropertyChanged("ZPosition");
            }
        }

        public int ZInvertUpDown
        {
            get
            {
                return _zControlModel.ZInvertUpDown;
            }
            set
            {
                _zControlModel.ZInvertUpDown = value;
                OnPropertyChanged("ZInvertUpDown");
            }
        }

        public int ZInvertUpDown2
        {
            get
            {
                return _zControlModel.ZInvertUpDown2;
            }
            set
            {
                _zControlModel.ZInvertUpDown2 = value;
                OnPropertyChanged("ZInvertUpDown2");
            }
        }

        /// <summary>
        /// Gets the z invert visibility.
        /// </summary>
        /// <value>The z invert visibility.</value>
        public Visibility ZInvertVisibility
        {
            get
            {
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ZView");
                if (ndList.Count > 0)
                {
                    string tmp = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], appSettings, "InvertVisibility", ref tmp))
                    {
                        return tmp.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }
                    else
                    {
                        return Visibility.Collapsed;
                    }
                }

                return Visibility.Collapsed;
            }
        }

        public double ZMax
        {
            get
            {
                return _zControlModel.ZMax;

            }
        }

        public double ZMin
        {
            get
            {
                return _zControlModel.ZMin;
            }
        }

        public double ZPosition
        {
            get
            {
                return _zControlModel.ZPosition;
            }
            set
            {
                TimeSpan ts = DateTime.Now - _lastZSetTime[0];

                if (ts.TotalSeconds > .25)
                {
                    _lastZPositionVal = _zControlModel.ZPosition = value;

                    OnPropertyChanged("ZPosition");
                    OnPropertyChanged("ZPositionBar");
                    MVMManager.Instance["PowerControlViewModel", "RedrawPowerPlot"] = true;
                    OnPropertyChanged("ZPosOutOfBounds");
                    _lastZSetTime[0] = DateTime.Now;
                }
            }
        }

        public double ZPositionBar
        {
            get
            {
                if (_zInvertLimits)
                {
                    return -_zControlModel.ZPosition;
                }
                else
                {
                    return _zControlModel.ZPosition;
                }
            }
            set
            {
                //This is needed otherwise will crash at startup because
                // setter is missing and ZPosition is configured two-way/one-way
            }
        }

        public string ZPosMinusKey
        {
            get { return _zPosMinusKey; }
            set { _zPosMinusKey = value; OnPropertyChanged("ZPosMinusKey"); }
        }

        public string ZPosMinusModifier
        {
            get { return _zPosMinusModifier; }
            set { _zPosMinusModifier = value; OnPropertyChanged("ZPosMinusModifier"); }
        }

        public bool ZPosOutOfBounds
        {
            get
            {
                bool ret;
                if (_zControlModel.ZPosition < _zControlModel.ZMin || _zControlModel.ZPosition > _zControlModel.ZMax)
                    ret = true;
                else
                    ret = false;
                return ret;
            }
        }

        public string ZPosPlusKey
        {
            get { return _zPosPlusKey; }
            set { _zPosPlusKey = value; OnPropertyChanged("ZPosPlusKey"); }
        }

        public string ZPosPlusModifier
        {
            get { return _zPosPlusModifier; }
            set { _zPosPlusModifier = value; OnPropertyChanged("ZPosPlusModifier"); }
        }

        public double ZRangeMax
        {
            get
            {
                return _zRangeMax = Double.Parse(String.Format("{0:0.#####}", _zRangeMax));  // "123.456";
            }
            set
            {
                if (value <= _zControlModel.ZMin)
                {
                    _zRangeMax = _zControlModel.ZMin;
                }
                else if (value >= _zControlModel.ZMax)
                {
                    _zRangeMax = _zControlModel.ZMax;
                }
                else
                {
                    _zRangeMax = value;
                }

                if (_zRangeDirection == 0)
                {
                    _zScanStop = _zRangeMax;
                    OnPropertyChanged("ZScanStop");
                }
                else
                {
                    _zScanStart = _zRangeMax;
                    OnPropertyChanged("ZScanStart");
                }

                OnPropertyChanged("ZRangeMax");
                OnPropertyChanged("ZScanNumSteps");
                OnPropertyChanged("ZScanThickness");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("StreamVolumes");
            }
        }

        public double ZRangeMin
        {
            get
            {
                return _zRangeMin = Double.Parse(String.Format("{0:0.#####}", _zRangeMin));  // "123.456";
            }
            set
            {
                if (value <= _zControlModel.ZMin)
                {
                    _zRangeMin = _zControlModel.ZMin;
                }
                else if (value >= _zControlModel.ZMax)
                {
                    _zRangeMin = _zControlModel.ZMax;
                }
                else
                {
                    _zRangeMin = value;
                }

                if (_zRangeDirection == 0)
                {
                    _zScanStart = _zRangeMin;
                    OnPropertyChanged("ZScanStart");
                }
                else
                {
                    _zScanStop = _zRangeMin;
                    OnPropertyChanged("ZScanStop");
                }

                OnPropertyChanged("ZRangeMin");
                OnPropertyChanged("ZScanNumSteps");
                OnPropertyChanged("ZScanThickness");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("StreamVolumes");
            }
        }

        public int ZScanNumSteps
        {
            get
            {
                const double UM_TO_MM = .001;
                // _zScanNumSteps = (int)Math.Abs((_zScanStart - _zScanStop) / (_zScanStep * UM_TO_MM));
                _zScanNumSteps = (int)Math.Round(Math.Abs(Math.Round((_zScanStart - _zScanStop), 5) / (_zScanStep * UM_TO_MM)) + 1); // 0.001mm-0.0045mm@0.5micron: 8 steps
                return _zScanNumSteps;
            }
        }

        public double ZScanStart
        {
            get
            {
                return _zScanStart = Double.Parse(String.Format("{0:0.#####}", _zScanStart));
            }
            set
            {
                if (value <= _zControlModel.ZMin)
                {
                    _zScanStart = _zControlModel.ZMin;
                }
                else if (value >= _zControlModel.ZMax)
                {
                    _zScanStart = _zControlModel.ZMax;
                }
                else
                {
                    _zScanStart = value;
                }

                if (_zScanStart > _zScanStop)
                {
                    _zRangeDirection = 1;
                    _zRangeMin = _zScanStop;
                    _zRangeMax = _zScanStart;
                }
                else
                {
                    _zRangeDirection = 0;
                    _zRangeMin = _zScanStart;
                    _zRangeMax = _zScanStop;
                }

                //Check and fit to the size of the ZStep
                EnsureValidZScanStop();

                OnPropertyChanged("ZRangeMax");
                OnPropertyChanged("ZRangeMin");

                OnPropertyChanged("ZScanStart");
                OnPropertyChanged("ZScanNumSteps");
                OnPropertyChanged("ZScanThickness");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("StreamVolumes");
                OnPropertyChanged("ZCenter");
                MVMManager.Instance["PowerControlViewModel", "RedrawPowerPlot"] = true;

            }
        }

        public double ZScanStep
        {
            get
            {
                return _zScanStep;
            }
            set
            {
                if (value != 0)
                {
                    double rounded = Math.Round(value, 1);
                    if (rounded <= .1)
                    {
                        rounded = .1;
                    }

                    _zScanStep = rounded;

                    //Check and fit to the size of the ZStep
                    EnsureValidZScanStop();

                    OnPropertyChanged("ZScanStep");
                    OnPropertyChanged("ZScanThickness");
                    OnPropertyChanged("ZScanNumSteps");
                    OnPropertyChanged("VolumeSpacingZ");
                    ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("StreamVolumes");
                }
            }
        }

        public double ZScanStop
        {
            get
            {
                // return _zScanStop = Double.Parse(String.Format("{0:0.###}", _zScanStop));
                if (Math.Round((_zScanStop - _zScanStart), 5) * 1e+3 % (_zScanStep) == 0)
                {
                    return _zScanStop;
                }
                else
                {
                    return _zScanStop = Double.Parse(String.Format("{0:0.#####}", _zScanStop));
                }
            }
            set
            {
                if (value <= _zControlModel.ZMin)
                {
                    _zScanStop = _zControlModel.ZMin;
                }
                else if (value >= _zControlModel.ZMax)
                {
                    _zScanStop = _zControlModel.ZMax;
                }
                else
                {

                    _zScanStop = value;
                    //round to the tens of nanometers
                    _zScanStop = Math.Round(_zScanStop, 5);
                }

                if (_zScanStop > _zScanStart)
                {
                    _zRangeDirection = 0;
                    _zRangeMin = _zScanStart;
                    _zRangeMax = _zScanStop;
                }
                else
                {
                    _zRangeDirection = 1;
                    _zRangeMin = _zScanStop;
                    _zRangeMax = _zScanStart;
                }

                //Check and fit to the size of the ZStep
                EnsureValidZScanStop();

                OnPropertyChanged("ZRangeMax");
                OnPropertyChanged("ZRangeMin");

                OnPropertyChanged("ZScanStop");
                OnPropertyChanged("ZScanNumSteps");
                OnPropertyChanged("ZScanThickness");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("StreamVolumes");
                OnPropertyChanged("ZCenter");
                MVMManager.Instance["PowerControlViewModel", "RedrawPowerPlot"] = true;

            }
        }

        public bool ZScanStopNotValid
        {
            get
            {
                return _zScanStopNotValid;
            }
            set
            {
                _zScanStopNotValid = value;
                OnPropertyChanged("ZScanStopNotValid");
            }
        }

        public double ZScanThickness
        {
            get
            {
                const double MM_TO_UM = 1000;
                double val = _zScanStart - _zScanStop;

                _zScanThickness = Math.Abs(Double.Parse(String.Format("{0:0.###}", val * MM_TO_UM)));  // "123.456";
                return _zScanThickness;
            }
        }

        public double ZSectionThickness
        {
            get
            {
                double zSectionThickness = 0.0;

                XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                if (null == hardwareDoc)
                {
                    return zSectionThickness;
                }

                double airyUnit = 0;

                XmlNodeList objList = hardwareDoc.SelectNodes("/HardwareSettings/Objectives/Objective");

                if (objList.Count > (int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0])
                {
                    double mag = Convert.ToDouble(objList[(int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0]].Attributes["mag"].Value.ToString(), CultureInfo.InvariantCulture);
                    double na = Convert.ToDouble(objList[(int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0]].Attributes["na"].Value.ToString(), CultureInfo.InvariantCulture);

                    const double SCANLENS_MAG = 1.071428571;
                    const double WAVELENGTH_UM = .488;
                    airyUnit = ((2 * 0.61 * WAVELENGTH_UM * SCANLENS_MAG * mag) / na);

                    double waveLength = .488;
                    double refractiveIndex = 1.3;
                    const double SQRT_TWO = 1.414213562;

                    zSectionThickness = Math.Sqrt(Math.Pow((waveLength * refractiveIndex) / (na * na), 2) + Math.Pow(((double)MVMManager.Instance["PinholeControlViewModel", "PinholeSizeUM", (object)0.0] / airyUnit) * refractiveIndex * SQRT_TWO * 1.22 * waveLength / (na * na), 2));

                    Decimal dec = new Decimal(zSectionThickness);
                    zSectionThickness = Decimal.ToDouble(Decimal.Round(dec, 3));
                }

                return zSectionThickness;
            }
        }

        public double ZSliderStepSize
        {
            get
            {
                return (ZMax - ZMin) / 20.0;
            }
        }

        public string ZStackCacheDirectory
        {
            get
            {
                return _zStackCacheDirectory;
            }
            set
            {
                _zStackCacheDirectory = value;
                OnPropertyChanged("ZStackCacheDirectory");
            }
        }

        public ICommand ZStage2GoToLocationCommand
        {
            get
            {
                if (this._zStage2GoToLocationCommand == null)
                    this._zStage2GoToLocationCommand = new RelayCommandWithParam((x) => ZStage2GoToLocation(x));

                return this._zStage2GoToLocationCommand;
            }
        }

        public CustomCollection<string> ZStage2LocationNames
        {
            get
            {
                return _zStage2LocationNames;
            }
            set
            {
                _zStage2LocationNames = value;
                OnPropertyChanged("ZStage2LocationNames");
            }
        }

        public ICommand ZStage2LocationSaveCommand
        {
            get
            {
                if (this._zStage2LocationSaveCommand == null)
                    this._zStage2LocationSaveCommand = new RelayCommandWithParam((x) => ZStage2LocationSave(x));

                return this._zStage2LocationSaveCommand;
            }
        }

        public string ZStage2Name
        {
            get
            {
                return _zControlModel.ZStage2Name;
            }
        }

        public string ZStageName
        {
            get
            {
                return _zControlModel.ZStageName;
            }
        }

        public string ZStartKey
        {
            get { return _zStartKey; }
            set { _zStartKey = value; OnPropertyChanged("ZStartKey"); }
        }

        public string ZStartModifier
        {
            get { return _zStartModifier; }
            set { _zStartModifier = value; OnPropertyChanged("ZStartModifier"); }
        }

        public double ZStepSize
        {
            get
            {
                return _zStepSize;
            }
            set
            {
                Decimal dec = new Decimal(value);
                _zStepSize = Decimal.ToDouble(Decimal.Round(dec, 4));
                OnPropertyChanged("ZStepSize");
            }
        }

        public string ZStopKey
        {
            get { return _zStopKey; }
            set { _zStopKey = value; OnPropertyChanged("ZStopKey"); }
        }

        public string ZStopModifier
        {
            get { return _zStopModifier; }
            set { _zStopModifier = value; OnPropertyChanged("ZStopModifier"); }
        }

        public string ZZeroKey
        {
            get { return _zZeroKey; }
            set { _zZeroKey = value; OnPropertyChanged("ZZeroKey"); }
        }

        public string ZZeroModifier
        {
            get { return _zZeroModifier; }
            set { _zZeroModifier = value; OnPropertyChanged("ZZeroModifier"); }
        }

        public Visibility ZZeroVisibility
        {
            get
            {
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ZView");
                if (ndList.Count > 0)
                {
                    string tmp = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], appSettings, "zZeroVisibility", ref tmp))
                    {
                        return tmp.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }
                    else
                    {
                        return Visibility.Collapsed;
                    }
                }

                return Visibility.Collapsed;
            }
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = typeof(ZControlViewModel).GetProperty(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = typeof(ZControlViewModel).GetProperty(propertyName);
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
                myPropInfo = typeof(ZControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            XmlDocument experimentDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            XmlNodeList ndList = experimentDoc.SelectNodes("/ThorImageExperiment/ZStage");
            double dTmp = 0.0;
            int iTmp = 0;
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                double scanStep = 0;
                int numSteps = 0;
                if (XmlManager.GetAttribute(ndList[0], experimentDoc, "startPos", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    ZScanStart = dTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], experimentDoc, "steps", ref str) && Int32.TryParse(str, out iTmp))
                {
                    numSteps = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], experimentDoc, "stepSizeUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                {
                    scanStep = dTmp;
                }
                //display step as an unsigned value
                ZScanStep = Math.Abs(scanStep);
                ZScanStop = ZScanStart + (numSteps - 1) * scanStep / (double)Constants.UM_TO_MM;
            }

            // Set an OnPropertyChanged event for all properties
            OnPropertyChange("");

            OnPropertyChange("ZPosition");
            ZStackCacheDirectory = Application.Current.Resources["ZStackCacheFolder"].ToString();
        }

        public void OnPropertyChange(string propertyName)
        {
            base.OnPropertyChanged(propertyName);
        }

        public void UpdateExpXMLSettings(ref XmlDocument xmlDoc)
        {
        }

        //Check and fit to the size of the ZStep
        private void EnsureValidZScanStop()
        {
            if (_zScanStop != _zScanStart)
            {
                double rem = Math.Round(Math.Abs((_zScanStop - _zScanStart)) % (_zScanStep / 1000), 4);
                if (0 != rem)
                {
                    int zDirection = 0;
                    if (_zScanStop > _zScanStart)
                    {
                        zDirection = 1;
                    }
                    else
                    {
                        zDirection = -1;
                    }
                    //round to the tens of nanometers
                    double newZScanStop = Math.Round(_zScanStop + zDirection * (_zScanStep / 1000 - rem), 5);
                    if (newZScanStop <= _zControlModel.ZMin)
                    {
                        _zScanStop = _zControlModel.ZMin;
                    }
                    else if (newZScanStop >= _zControlModel.ZMax)
                    {
                        _zScanStop = _zControlModel.ZMax;
                    }
                    else
                    {
                        _zScanStop = newZScanStop;
                    }

                    OnPropertyChanged("ZScanStop");

                    if (_zScanStop > _zScanStart)
                    {
                        _zRangeDirection = 0;
                        _zRangeMin = _zScanStart;
                        _zRangeMax = _zScanStop;
                    }
                    else
                    {
                        _zRangeDirection = 1;
                        _zRangeMin = _zScanStop;
                        _zRangeMax = _zScanStart;
                    }

                    if (newZScanStop != _zScanStop)
                    {
                        ZScanStopNotValid = true;
                    }
                    else
                    {
                        ZScanStopNotValid = false;
                    }
                }
                else
                {
                    ZScanStopNotValid = false;
                }
            }
        }

        /// <summary>
        /// Go to the new z position
        /// </summary>
        private void GoZ()
        {
            ZPosition = ZGotoValue[0] / (double)Constants.UM_TO_MM;
            OnPropertyChanged("ZPosition");
            OnPropertyChanged("ZPosOutOfBounds");
        }

        /// <summary>
        /// Go to the new z position
        /// </summary>
        private void GoZ2()
        {
            Z2Position = ZGotoValue[1] / (double)Constants.UM_TO_MM;
            OnPropertyChanged("Z2Position");
            OnPropertyChanged("Z2PosOutOfBounds");
        }

        private void GoZCenter()
        {
            ZPosition = _zCenter;
            OnPropertyChanged("ZPosition");
            OnPropertyChanged("ZPosOutOfBounds");
        }

        private void GoZScanStart()
        {
            ZPosition = _zScanStart;
            OnPropertyChanged("ZPosition");
            OnPropertyChanged("ZPosOutOfBounds");
        }

        private void GoZScanStop()
        {
            ZPosition = _zScanStop;
            OnPropertyChanged("ZPosition");
            OnPropertyChanged("ZPosOutOfBounds");
        }

        private void PersistZ2Invert()
        {
            MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ZView");
            if (ndList.Count > 0)
            {
                string tmp = (true == Z2Invert) ? "1" : "0";

                XmlManager.SetAttribute(ndList[0], appSettings, "Invert2", tmp);
            }
            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
        }

        private void PersistZInvert()
        {
            MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ZView");
            if (ndList.Count > 0)
            {
                string tmp = (true == ZInvert) ? "1" : "0";

                XmlManager.SetAttribute(ndList[0], appSettings, "Invert", tmp);
            }
            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
        }

        private void SetZ2Zero()
        {
            _zControlModel.SetZ2Zero();

            OnPropertyChanged("Z2Position");
            OnPropertyChanged("Z2PosOutOfBounds");
        }

        private void SetZScanStart()
        {
            ZScanStart = _zControlModel.ZPosition;
            _zRangeMin = _zControlModel.ZPosition;
            OnPropertyChanged("ZRangeMin");
            OnPropertyChanged("ZScanNumSteps");
            OnPropertyChanged("ZScanThickness");
            OnPropertyChanged("ZCenter");
            ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("StreamVolumes");
        }

        private void SetZScanStop()
        {
            ZScanStop = _zControlModel.ZPosition;
            _zRangeMax = _zControlModel.ZPosition;
            OnPropertyChanged("ZRangeMax");
            OnPropertyChanged("ZScanNumSteps");
            OnPropertyChanged("ZScanThickness");
            OnPropertyChanged("ZCenter");
            ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("StreamVolumes");
        }

        private void SetZZero()
        {
            _zControlModel.SetZZero();

            OnPropertyChanged("ZPosition");
            OnPropertyChanged("ZPosOutOfBounds");
        }

        private void StartZStackPreview()
        {
            //stop Live Capture
            MVMManager.Instance["CaptureSetupViewModel", "LiveCaptureProperty"] = false;

            if (false == IsZStackCapturing)
            {
                IsZStackCapturing = true;
                _isZCaptureStopped = false;

                //update Active.xml
                MVMManager.Instance["CaptureSetupViewModel", "PersistDataNow"] = true;

                try
                {
                    MVMManager.Instance["CaptureSetupViewModel", "ClearDirectory"] = ZStackCacheDirectory;

                    // copy Active.xml to ZStackCacheDirectory\Experiment.xml
                    string templatesFolder = ResourceManagerCS.GetCaptureTemplatePathString();
                    string srcFile = templatesFolder + "\\Active.xml";
                    string destFile = ZStackCacheDirectory + "\\Experiment.xml";
                    System.IO.File.Copy(srcFile, destFile);

                    // Change the tag "rawData" in \ThorImageLS 3.1\ZStackCache\Experiment.xml to 0, so vtk doesn't try to
                    // read .raw files
                    XmlDocument exp = new XmlDocument();
                    exp.Load(destFile);
                    XmlNodeList ndList = exp.SelectNodes("/ThorImageExperiment/Streaming");
                    ndList[0].Attributes["rawData"].Value = "0";
                    exp.Save(destFile);
                }
                catch (System.IO.IOException e)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " " + e.Message);
                }

                //stop the background hardware updates
                MVMManager.Instance["CaptureSetupViewModel", "BWHardware"] = false;

                MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = false;
                MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = true;

                _zControlModel.StartZStackPreview(ZScanStart, ZScanStop, ZScanStep, ZScanNumSteps);

                //restart the background hardware updates
                MVMManager.Instance["CaptureSetupViewModel", "BWHardware"] = true;

                MVMManager.Instance["CaptureSetupViewModel", "PreviewProtocol"] = "ZStackPreview";
            }
        }

        private void StopZ()
        {
            _zControlModel.StopZ();
        }

        private void StopZ2()
        {
            _zControlModel.StopZ2();
        }

        private void z2PosMinus()
        {
            Z2Position -= Z2StepSize;
        }

        private void z2PosPlus()
        {
            Z2Position += Z2StepSize;
        }

        private void ZCommands(object type)
        {
            double largerStepSize = 0, smallerStepSize = 0;
            switch (ZCommandTypeDictionary[(string)type])
            {
                case ZCommandType.Z_STEPSIZE_COARSE:
                    largerStepSize = ZStepSize * 10;

                    if (largerStepSize > MAX_Z_STEP_SIZE)
                    {
                        largerStepSize = MAX_Z_STEP_SIZE;
                    }

                    ZStepSize = largerStepSize;
                    break;
                case ZCommandType.Z_STEPSIZE_FINE:
                    smallerStepSize = ZStepSize / 10;

                    if (smallerStepSize < MIN_Z_STEP_SIZE)
                    {
                        smallerStepSize = MIN_Z_STEP_SIZE;
                    }

                    ZStepSize = smallerStepSize;

                    break;
                case ZCommandType.Z2_STEPSIZE_COARSE:
                    largerStepSize = Z2StepSize * 10;

                    if (largerStepSize > MAX_Z_STEP_SIZE)
                    {
                        largerStepSize = MAX_Z_STEP_SIZE;
                    }

                    Z2StepSize = largerStepSize;
                    break;
                case ZCommandType.Z2_STEPSIZE_FINE:
                    smallerStepSize = Z2StepSize / 10;

                    if (smallerStepSize < MIN_Z_STEP_SIZE)
                    {
                        smallerStepSize = MIN_Z_STEP_SIZE;
                    }

                    Z2StepSize = smallerStepSize;
                    break;
                case ZCommandType.GO_Z:
                    GoZ();
                    break;
                case ZCommandType.GO_Z2:
                    GoZ2();
                    break;
                case ZCommandType.GO_Z_CENTER:
                    GoZCenter();
                    break;
                case ZCommandType.GO_Z_SCANSTART:
                    GoZScanStart();
                    break;
                case ZCommandType.GO_Z_SCANSTOP:
                    GoZScanStop();
                    break;
                case ZCommandType.STOP_Z:
                    StopZ();
                    break;
                case ZCommandType.STOP_Z2:
                    StopZ2();
                    break;
                case ZCommandType.SET_Z_ZERO:
                    SetZZero();
                    break;
                case ZCommandType.SET_Z2_ZERO:
                    SetZ2Zero();
                    break;
                case ZCommandType.SET_Z_SCANSTART:
                    SetZScanStart();
                    break;
                case ZCommandType.SET_Z_SCANSTOP:
                    SetZScanStop();
                    break;
                case ZCommandType.Z_POS_PLUS:
                    zPosPlus();
                    break;
                case ZCommandType.Z_POS_MINUS:
                    zPosMinus();
                    break;
                case ZCommandType.Z2_POS_PLUS:
                    z2PosPlus();
                    break;
                case ZCommandType.Z2_POS_MINUS:
                    z2PosMinus();
                    break;
                default:
                    break;
            }
        }

        private void zPosMinus()
        {
            ZPosition -= ZStepSize;
        }

        private void zPosPlus()
        {
            ZPosition += ZStepSize;
        }

        private void ZStage2GoToLocation(object index)
        {
            try
            {
                XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                if (hardwareDoc != null)
                {
                    XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/ZStage2");

                    if (ndList.Count > 0)
                    {
                        string strName = string.Empty;
                        string strVal = string.Empty;

                        if (XmlManager.GetAttribute(ndList[0], hardwareDoc, LocationNameDictionary[Convert.ToInt32(index)], ref strName))
                        {
                            if (!string.IsNullOrEmpty(strName) && !string.IsNullOrWhiteSpace(strName))
                            {
                                if (XmlManager.GetAttribute(ndList[0], hardwareDoc, LocationDictionary[Convert.ToInt32(index)], ref strVal))
                                {
                                    double tmp = 0.0;
                                    if (Double.TryParse(strVal, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                                    {
                                        Z2Position = tmp;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception e)
            {
                e.ToString();
            }
        }

        private void ZStage2LocationSave(object index)
        {
            MVMManager.Instance.ReloadSettings(SettingsFileType.HARDWARE_SETTINGS);
            XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

            if (hardwareDoc != null)
            {
                XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/ZStage2");

                if (ndList.Count > 0)
                {
                    ZStagePresetLocationEdit dlg = new ZStagePresetLocationEdit();

                    string str = string.Empty;
                    string strLocation = string.Empty;
                    int id = Convert.ToInt32(index);

                    XmlManager.GetAttribute(ndList[0], hardwareDoc, LocationNameDictionary[id], ref str); XmlManager.GetAttribute(ndList[0], hardwareDoc, LocationDictionary[id], ref strLocation);

                    dlg.LocationName = str;
                    double z2Temp = Z2Position; //update Z2 position
                    dlg.DataContext = this;
                    if (false == dlg.ShowDialog())
                    {
                        return;
                    }
                    str = dlg.LocationName;

                    XmlManager.SetAttribute(ndList[0], hardwareDoc, LocationNameDictionary[id], str); XmlManager.SetAttribute(ndList[0], hardwareDoc, LocationDictionary[id], Z2Position.ToString()); ZStage2LocationNames[id] = str;

                    MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
                }
            }
        }

        #endregion Methods
    }
}