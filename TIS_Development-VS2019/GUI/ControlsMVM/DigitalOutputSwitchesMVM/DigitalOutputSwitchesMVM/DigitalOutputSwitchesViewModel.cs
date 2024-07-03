namespace DigitalOutputSwitches.ViewModel
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
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Threading;
    using System.Xml;

    using DigitalOutputSwitches.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public class DigitalOutputSwitchesViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly DigitalOutputSwitchesModel _DigitalOutputSwitchesModel;

        ICommand _digitalSwitchCommand;
        private int _experimentMode = 0;
        ICommand _gotoCommand;
        private double[] _gotoValue;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private DispatcherTimer _statusTimer;
        private int _switchEnable;
        private bool _switchesIsVisible = false;
        ICommand _triggerEnableCommand;
        private string _triggerError = string.Empty;
        private bool _triggerIsVisible = false;
        private ObservableCollection<StringPC> _triggerLabel = new ObservableCollection<StringPC>();
        private Dictionary<string, string> _triggerLineDic = new Dictionary<string, string>();
        private EPhysTriggerStruct _triggerStruct;

        #endregion Fields

        #region Constructors

        public DigitalOutputSwitchesViewModel()
        {
            this._DigitalOutputSwitchesModel = new DigitalOutputSwitchesModel();

            SwitchState = new ObservableCollection<IntPC>();
            SwitchName = new ObservableCollection<StringPC>();

            for (int i = 0; i < (int)Constants.MAX_SWITCHES; i++)
            {
                SwitchState.Add(new IntPC());
                SwitchName.Add(new StringPC());
            }
            _triggerStruct = new EPhysTriggerStruct();
            _triggerStruct.triggerLine = string.Empty;
            _triggerStruct.stepEdge = Enumerable.Repeat(-1, (int)Constants.EPHYS_ARRAY_SIZE).ToArray();

            //populate combobox selections and label
            TriggerTypeItems = new ObservableCollection<StringPC>();
            for (int i = (int)EPhysOutputType.DIGITAL_ONLY; i < (int)EPhysOutputType.EPHYS_LAST_OUTPUT_TYPE; i++)
            {
                TriggerTypeItems.Add(new StringPC(new String(Enum.GetName(typeof(EPhysOutputType), i).Select((ch, id) => (0 == id) ? ch : ('_' == ch ? ' ' : Char.ToLower(ch))).ToArray())));
            }
            TriggerModeItems = new ObservableCollection<StringPC>();
            for (int i = (int)EPhysTriggerMode.NONE; i < (int)EPhysTriggerMode.EPHYS_LAST_TRIGGER_MODE; i++)
            {
                TriggerModeItems.Add(new StringPC(new String(Enum.GetName(typeof(EPhysTriggerMode), i).Select((ch, id) => (0 == id) ? ch : Char.ToLower(ch)).ToArray())));
            }

            _triggerLabel.Add(new StringPC(""));
            _triggerLabel.Add(new StringPC("Frame #"));
            _triggerLabel.Add(new StringPC("Line #"));
            _triggerLabel.Add(new StringPC("Z Slice #"));
            _triggerLabel.Add(new StringPC("Capture #"));
            _triggerLabel.Add(new StringPC("Stimulation #"));
            _triggerLabel.Add(new StringPC(""));
            _triggerLabel.Add(new StringPC("Trigger #"));

            //use timer to update trigger status
            _statusTimer = new DispatcherTimer();
            _statusTimer.Interval = new TimeSpan(0, 0, 0, 0, 100);
            _statusTimer.Tick += new EventHandler(_statusTimer_Tick);

            _gotoValue = new double[2] { 0.0, 0.0 };
        }

        #endregion Constructors

        #region Properties

        public ICommand DigitalSwitchCommand
        {
            get
            {
                if (this._digitalSwitchCommand == null)
                    this._digitalSwitchCommand = new RelayCommandWithParam((x) => DigitalSwitch(x));

                return this._digitalSwitchCommand;
            }
        }

        public int ExperimentMode
        {
            get
            {
                return _experimentMode;
            }
            set
            {
                _experimentMode = value;
                OnPropertyChanged("ExperimentMode");
            }
        }

        public double GotoAnalog
        {
            get { return _gotoValue[1]; }
            set
            {
                _gotoValue[1] = value;
                OnPropertyChanged("GotoAnalog");
            }
        }

        public ICommand GotoCommand
        {
            get
            {
                if (this._gotoCommand == null)
                    this._gotoCommand = new RelayCommand(() => Goto());

                return this._gotoCommand;
            }
        }

        public int GotoDigital
        {
            get { return (int)_gotoValue[0]; }
            set
            {
                _gotoValue[0] = value;
                OnPropertyChanged("GotoDigital");
            }
        }

        public string PowerPercentString
        {
            get
            {
                return (null != _triggerStruct.powerPercent && 0 < _triggerStruct.powerPercent.Where(x => 0 < x).Count()) ?
                    string.Join(":", Array.ConvertAll(_triggerStruct.powerPercent.Where(x => 0 <= x).ToArray(), y => y.ToString())) :
                    "0";
            }
            set
            {
                string[] list = Regex.Split(value, ":");
                TriggerError = string.Empty;
                double dVal = 0.0;
                if ((int)Constants.EPHYS_ARRAY_SIZE < list.Length)
                {
                    TriggerError = "Error: too many entries for Powers.";
                }
                else
                {
                    _triggerStruct.powerPercent = Enumerable.Repeat(-1.0, (int)Constants.EPHYS_ARRAY_SIZE).ToArray();
                    for (int i = 0; i < list.Length; i++)
                    {
                        if (Double.TryParse(list[i], out dVal))
                            _triggerStruct.powerPercent[i] = dVal;
                    }
                }
                if ((0 == _triggerError.Length) || (TriggerErrorColor.Contains("Yellow")))
                {
                    SetTriggerStruct();
                    OnPropertyChanged("PowerPercentString");
                }
            }
        }

        public int SwitchEnable
        {
            get
            {
                return _switchEnable;
            }
            set
            {
                _switchEnable = value;
                OnPropertyChanged("SwitchEnable");
            }
        }

        public bool SwitchesIsVisible
        {
            get
            {
                return _switchesIsVisible;
            }
            set
            {
                _switchesIsVisible = value;
                OnPropertyChanged("SwitchesIsVisible");
            }
        }

        public ObservableCollection<StringPC> SwitchName
        {
            get;
            set;
        }

        public ObservableCollection<IntPC> SwitchState
        {
            get;
            set;
        }

        public int TriggerConfigured
        {
            get
            {
                GetTriggerStruct();
                EPhysTriggerMode lastMode = EPhysTriggerMode.MANUAL;
                switch (_triggerStruct.configured)
                {
                    case 0:
                        return _triggerStruct.configured;
                    case 1:
                        lastMode = EPhysTriggerMode.FRAME;
                        break;
                    case 2:
                        lastMode = EPhysTriggerMode.CUSTOM;
                        break;
                    case 3:
                        lastMode = EPhysTriggerMode.EPHYS_LAST_TRIGGER_MODE;
                        break;
                    default:
                        break;
                }
                int tCount = TriggerModeItems.Count;
                if ((int)lastMode < tCount)
                {
                    for (int i = tCount - 1; i >= (int)lastMode; i--)
                    {
                        TriggerModeItems.RemoveAt(i);
                    }
                }
                else if ((int)lastMode > tCount)
                {
                    for (int i = tCount; i < (int)lastMode; i++)
                    {
                        TriggerModeItems.Add(new StringPC(Enum.GetName(typeof(EPhysTriggerMode), i)));
                    }
                }
                return _triggerStruct.configured;
            }
        }

        public double TriggerDurationMS
        {
            get
            {
                return _triggerStruct.durationMS;
            }
            set
            {
                _triggerStruct.durationMS = value;
                SetTriggerStruct();
                OnPropertyChanged("TriggerDurationMS");
            }
        }

        public string TriggerEdgeString
        {
            get
            {
                return (0 < _triggerStruct.stepEdge.Where(x => 0 < x).Count()) ?
                    string.Join(":", Array.ConvertAll(_triggerStruct.stepEdge.Where(x => 0 <= x).ToArray(), y => y.ToString())) :
                    "0";
            }
            set
            {
                string[] list = Regex.Split(value, ":");
                TriggerError = string.Empty;
                int iVal = 0;
                if ((int)Constants.EPHYS_ARRAY_SIZE < list.Length)
                {
                    TriggerError = "Error: too many entries for Gaps.";
                }
                else
                {
                    _triggerStruct.stepEdge = Enumerable.Repeat(-1, (int)Constants.EPHYS_ARRAY_SIZE).ToArray();
                    for (int i = 0; i < list.Length; i++)
                    {
                        if ((!Int32.TryParse(list[i], out iVal)) || ((1 < list.Length) && (0 == iVal)))
                        {
                            TriggerError = "Warning: Gaps contain 0.";
                        }
                        _triggerStruct.stepEdge[i] = iVal;
                    }
                }
                if ((0 == _triggerError.Length) || (TriggerErrorColor.Contains("Yellow")))
                {
                    SetTriggerStruct();
                    OnPropertyChanged("TriggerEdgeString");
                }
            }
        }

        public int TriggerEnable
        {
            get
            {
                GetTriggerStruct();
                return _triggerStruct.enable;
            }
            set
            {
                _triggerStruct.enable = value;
                SetFramePerZSlice();
                SetTriggerStruct();
                OnPropertyChanged("TriggerEnable");

                //start timer to check status
                if (1 == _triggerStruct.enable)
                {
                    _statusTimer.Start();
                }
                else
                {
                    _statusTimer.Stop();
                    //update once after stop timer
                    OnPropertyChanged("TriggerImagePath");
                }
            }
        }

        public ICommand TriggerEnableCommand
        {
            get
            {
                if (this._triggerEnableCommand == null)
                    this._triggerEnableCommand = new RelayCommand(() => EnableTrigger());

                return this._triggerEnableCommand;
            }
        }

        public string TriggerError
        {
            get
            {
                if (0 < _triggerError.Length)
                {
                    return _triggerError;
                }
                else
                {
                    return ResourceManagerCS.GetDeviceError((int)SelectedHardware.SELECTED_EPHYS);
                }
            }
            set
            {
                _triggerError = value;
                OnPropertyChanged("TriggerError");
                OnPropertyChanged("TriggerErrorColor");
            }
        }

        public string TriggerErrorColor
        {
            get
            {
                return (TriggerError.Contains("Warning")) ? "Yellow" : "Red";
            }
        }

        public double TriggerIdleMS
        {
            get
            {
                return (_triggerStruct.minIdleMS >= _triggerStruct.idleMS) ? 0.0 : _triggerStruct.idleMS;
            }
            set
            {
                double val = (_triggerStruct.minIdleMS >= value) ? _triggerStruct.minIdleMS : value;
                _triggerStruct.idleMS = val;
                SetTriggerStruct();
                OnPropertyChanged("TriggerIdleMS");
            }
        }

        public string TriggerImagePath
        {
            get
            {
                //check trigger status to update enable and button image
                int status = (int)StatusType.STATUS_BUSY;
                ResourceManagerCS.GetDeviceStatus((int)SelectedHardware.SELECTED_EPHYS, ref status);

                if ((int)StatusType.STATUS_BUSY != status)
                {
                    if (_statusTimer.IsEnabled)
                        TriggerEnable = 0;

                    OnPropertyChanged("TriggerEnable");
                }
                return ((int)StatusType.STATUS_BUSY == status) ? @"/DigitalOutputSwitches;component/Icons/Stop.png" : @"/DigitalOutputSwitches;component/Icons/play2.png";
            }
        }

        public bool TriggerIsVisible
        {
            get
            {
                return _triggerIsVisible;
            }
            set
            {
                _triggerIsVisible = value;
                OnPropertyChanged("TriggerIsVisible");
            }
        }

        public int TriggerIterations
        {
            get
            {
                return _triggerStruct.iterations;
            }
            set
            {
                _triggerStruct.iterations = value;
                SetTriggerStruct();
                OnPropertyChanged("TriggerIterations");
            }
        }

        public string TriggerLabel
        {
            get
            {
                return _triggerLabel[TriggerMode].Value;
            }
        }

        public int TriggerMode
        {
            get
            {
                if (0 >= _triggerStruct.mode)
                    return 0;

                return _triggerStruct.mode;
            }
            set
            {
                if (0 <= value)
                {
                    _triggerStruct.mode = value;

                    //auto config for trigger line via RTSI
                    _triggerStruct.triggerLine = "";
                    ICamera.LSMType currentLSM = ICamera.LSMType.LSMTYPE_LAST;
                    if ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType())
                        currentLSM = (ICamera.LSMType)ResourceManagerCS.GetLSMType();

                    switch ((EPhysTriggerMode)_triggerStruct.mode)
                    {
                        case EPhysTriggerMode.CAPTURE:
                            _triggerStruct.triggerLine = (ICamera.LSMType.GALVO_RESONANCE == currentLSM) ? _triggerLineDic["CaptureGR"] : _triggerLineDic["CaptureGG"];
                            break;
                        case EPhysTriggerMode.ZSTACK:
                        case EPhysTriggerMode.FRAME:
                            _triggerStruct.triggerLine = (ICamera.LSMType.GALVO_RESONANCE == currentLSM) ? _triggerLineDic["FrameGR"] : _triggerLineDic["FrameGG"];
                            break;
                        case EPhysTriggerMode.LINE:
                            _triggerStruct.triggerLine = (ICamera.LSMType.GALVO_RESONANCE == currentLSM) ? _triggerLineDic["LineGR"] : _triggerLineDic["LineGG"];
                            break;
                        case EPhysTriggerMode.STIMULATION:
                            _triggerStruct.triggerLine = _triggerLineDic["Stimulation"];      //physical wiring from Bleach Active
                            break;
                        default:
                            TriggerError = string.Empty;
                            break;
                    }
                    SetFramePerZSlice();
                    SetTriggerStruct();
                    OnPropertyChanged("TriggerMode");
                    OnPropertyChanged("TriggerLabel");
                }
            }
        }

        public ObservableCollection<StringPC> TriggerModeItems
        {
            get;
            set;
        }

        public int TriggerRepeat
        {
            get
            {
                return _triggerStruct.repeats;
            }
            set
            {
                _triggerStruct.repeats = value;
                SetTriggerStruct();
                OnPropertyChanged("TriggerRepeat");
            }
        }

        public int TriggerStartEdge
        {
            get
            {
                return _triggerStruct.startEdge;
            }
            set
            {
                _triggerStruct.startEdge = value;
                SetTriggerStruct();
                OnPropertyChanged("TriggerStartEdge");
            }
        }

        public double TriggerStartIdleMS
        {
            get
            {
                return _triggerStruct.startIdleMS;
            }
            set
            {
                _triggerStruct.startIdleMS = value;
                SetTriggerStruct();
                OnPropertyChanged("TriggerStartIdleMS");
            }
        }

        public int TriggerType
        {
            get
            {
                return _triggerStruct.outputType;
            }
            set
            {
                _triggerStruct.outputType = value;
                SetTriggerStruct();
                OnPropertyChanged("TriggerType");
            }
        }

        public ObservableCollection<StringPC> TriggerTypeItems
        {
            get;
            set;
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName, object defaultObject = null]
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

        public void DigitalSwitch(object val)
        {
            int switchId = Convert.ToInt32(val);

            if (1 == SwitchEnable)
            {
                if (switchId < SwitchState.Count)
                {
                    ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_EPHYS, (int)IDevice.Params.PARAM_EPHYS_DIG_LINE_OUT_1 + switchId, SwitchState[switchId].Value, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                }
            }
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(DigitalOutputSwitchesViewModel).GetProperty(propertyName);
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
            int iVal = 0;
            double dVal = 0;

            //load trigger lines settings from hw settings
            XmlDocument hwDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

            XmlNodeList ndList = hwDoc.SelectNodes("/HardwareSettings/DigitalIO");
            _triggerLineDic.Clear();
            if (ndList.Count <= 0)
            {
                //create default if not exist
                MVMManager.Instance.ReloadSettings(SettingsFileType.HARDWARE_SETTINGS);
                hwDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                XmlManager.CreateXmlNode(hwDoc, "DigitalIO");
                ndList = hwDoc.SelectNodes("/HardwareSettings/DigitalIO");

                _triggerLineDic.Add("FrameGR", "/Dev3/PFI3");
                XmlManager.SetAttribute(ndList[0], hwDoc, "FrameGR", "/Dev3/PFI3");

                _triggerLineDic.Add("FrameGG", "/Dev4/PFI3");
                XmlManager.SetAttribute(ndList[0], hwDoc, "FrameGG", "/Dev4/PFI3");

                _triggerLineDic.Add("LineGR", "/Dev3/PFI4");
                XmlManager.SetAttribute(ndList[0], hwDoc, "LineGR", "/Dev3/PFI4");

                _triggerLineDic.Add("LineGG", "/Dev4/PFI4");
                XmlManager.SetAttribute(ndList[0], hwDoc, "LineGG", "/Dev4/PFI4");

                _triggerLineDic.Add("CaptureGR", "/Dev3/PFI8");
                XmlManager.SetAttribute(ndList[0], hwDoc, "CaptureGR", "/Dev3/PFI8");

                _triggerLineDic.Add("CaptureGG", "/Dev3/PFI9");
                XmlManager.SetAttribute(ndList[0], hwDoc, "CaptureGG", "/Dev3/PFI9");

                _triggerLineDic.Add("Stimulation", "/Dev4/PFI5");
                XmlManager.SetAttribute(ndList[0], hwDoc, "Stimulation", "/Dev4/PFI5");

                MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
            }
            else
            {
                XmlManager.GetAttribute(ndList[0], hwDoc, "FrameGR", ref str);
                _triggerLineDic.Add("FrameGR", str);

                XmlManager.GetAttribute(ndList[0], hwDoc, "FrameGG", ref str);
                _triggerLineDic.Add("FrameGG", str);

                XmlManager.GetAttribute(ndList[0], hwDoc, "LineGR", ref str);
                _triggerLineDic.Add("LineGR", str);

                XmlManager.GetAttribute(ndList[0], hwDoc, "LineGG", ref str);
                _triggerLineDic.Add("LineGG", str);

                XmlManager.GetAttribute(ndList[0], hwDoc, "CaptureGR", ref str);
                _triggerLineDic.Add("CaptureGR", str);

                XmlManager.GetAttribute(ndList[0], hwDoc, "CaptureGG", ref str);
                _triggerLineDic.Add("CaptureGG", str);

                XmlManager.GetAttribute(ndList[0], hwDoc, "Stimulation", ref str);
                _triggerLineDic.Add("Stimulation", str);

            }

            //load app settings
            XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/DigitalSwitchesView");
            if (ndList.Count > 0)
            {
                TriggerIsVisible = (XmlManager.GetAttribute(ndList[0], appDoc, "TriggerView", ref str) && (0 == str.CompareTo("Visible"))) ? true : false;
                SwitchesIsVisible = (XmlManager.GetAttribute(ndList[0], appDoc, "SwitchView", ref str) && (0 == str.CompareTo("Visible"))) ? true : false;
            }

            //only visible when ElectroPhys configured for trigger
            if (0 == TriggerConfigured)
                TriggerIsVisible = false;

            //default settings
            ExperimentMode = TriggerEnable = 0;

            //load exp
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            ndList = doc.SelectNodes("/ThorImageExperiment/DigitalIO");

            if (ndList.Count > 0)
            {
                //for switches
                if (XmlManager.GetAttribute(ndList[0], doc, "enable", ref str) && Int32.TryParse(str, out iVal))
                {
                    SwitchEnable = iVal;

                    for (int i = 0; i < SwitchState.Count; i++)
                    {
                        if (XmlManager.GetAttribute(ndList[0], doc, string.Format("digOut{0}", i + 1), ref str) && Int32.TryParse(str, out iVal))
                        {
                            SwitchState[i].Value = iVal;

                            //if the output is enabled send the switch values to the device
                            if (1 == SwitchEnable)
                            {
                                DigitalSwitch(i);
                            }
                        }
                    }
                }

                //for triggers
                TriggerType = ((XmlManager.GetAttribute(ndList[0], doc, "trigType", ref str)) && Int32.TryParse(str, out iVal)) ? iVal : 0;
                TriggerMode = ((XmlManager.GetAttribute(ndList[0], doc, "trigMode", ref str)) && Int32.TryParse(str, out iVal)) ? iVal : 0;
                TriggerStartEdge = ((XmlManager.GetAttribute(ndList[0], doc, "trigStartEdge", ref str)) && Int32.TryParse(str, out iVal)) ? iVal : 0;
                TriggerEdgeString = XmlManager.GetAttribute(ndList[0], doc, "trigSteps", ref str) ? str : "0";
                TriggerRepeat = ((XmlManager.GetAttribute(ndList[0], doc, "trigRepeat", ref str)) && Int32.TryParse(str, out iVal)) ? iVal : 0;
                TriggerStartIdleMS = ((XmlManager.GetAttribute(ndList[0], doc, "trigStartIdleMS", ref str)) && Double.TryParse(str, out dVal)) ? dVal : 0;
                TriggerDurationMS = ((XmlManager.GetAttribute(ndList[0], doc, "trigDurationMS", ref str)) && Double.TryParse(str, out dVal)) ? dVal : 0;
                TriggerIdleMS = ((XmlManager.GetAttribute(ndList[0], doc, "trigIdleMS", ref str)) && Double.TryParse(str, out dVal)) ? dVal : 0;
                TriggerIterations = ((XmlManager.GetAttribute(ndList[0], doc, "trigIterations", ref str)) && Int32.TryParse(str, out iVal)) ? iVal : 0;
                PowerPercentString = XmlManager.GetAttribute(ndList[0], doc, "powerPercents", ref str) ? str : "0";
            }
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/DigitalIO");
            string str = string.Empty;

            //create node if not exist
            if (ndList.Count <= 0)
            {
                XmlManager.CreateXmlNode(experimentFile, "DigitalIO");
                ndList = experimentFile.SelectNodes("/ThorImageExperiment/DigitalIO");
            }
            //persist settings
            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], experimentFile, "enable", SwitchEnable.ToString());

                for (int i = 0; i < SwitchState.Count; i++)
                {
                    XmlManager.SetAttribute(ndList[0], experimentFile, string.Format("digOut{0}", i + 1), SwitchState[i].Value.ToString());
                }

                XmlManager.SetAttribute(ndList[0], experimentFile, "trigType", TriggerType.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "trigMode", TriggerMode.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "trigStartEdge", TriggerStartEdge.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "trigSteps", TriggerEdgeString);
                XmlManager.SetAttribute(ndList[0], experimentFile, "trigRepeat", TriggerRepeat.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "trigStartIdleMS", TriggerStartIdleMS.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "trigDurationMS", TriggerDurationMS.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "trigIdleMS", TriggerIdleMS.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "trigIterations", TriggerIterations.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "powerPercents", PowerPercentString);
            }
        }

        private void EnableTrigger()
        {
            TriggerEnable = (0 == TriggerEnable) ? 1 : 0;
        }

        private void GetTriggerStruct()
        {
            byte[] tByteArray = ResourceManagerCS.StructToByteArray(_triggerStruct);
            ResourceManagerCS.GetDeviceParamBuffer((int)SelectedHardware.SELECTED_EPHYS, (int)IDevice.Params.PARAM_EPHYS_TRIG_BUFFER, tByteArray, tByteArray.Length);
            _triggerStruct = ResourceManagerCS.StructFromByteArray<EPhysTriggerStruct>(tByteArray);
        }

        private void Goto()
        {
            ResourceManagerCS.SetDeviceParamBuffer<double>((int)SelectedHardware.SELECTED_EPHYS, (int)IDevice.Params.PARAM_EPHYS_GOTO_BUFFER, _gotoValue, _gotoValue.Length, (int)IDevice.DeviceSetParamType.NO_EXECUTION);
        }

        /// <summary>
        /// set average frame count per z slice for z stack trigger mode
        /// </summary>
        private void SetFramePerZSlice()
        {
            //default as 1
            _triggerStruct.framePerZSlice = 1;

            string str = string.Empty;
            int iVal = 0;
            if (EPhysTriggerMode.ZSTACK == (EPhysTriggerMode)_triggerStruct.mode)
            {
                XmlDocument expDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
                switch ((ExperimentModes)ExperimentMode)
                {
                    case ExperimentModes.EXP_CAPTURE:
                        {
                            XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/ZStage");
                            if (ndList.Count > 0)
                            {
                                if (XmlManager.GetAttribute(ndList[0], expDoc, "zStreamMode", ref str) && (Int32.TryParse(str, out iVal)))
                                {
                                    //retrieve from active xml for z stream frame if z stream enabled
                                    if (1 == iVal)
                                    {
                                        if (XmlManager.GetAttribute(ndList[0], expDoc, "zStreamFrames", ref str) && (Int32.TryParse(str, out iVal)))
                                            _triggerStruct.framePerZSlice = iVal;
                                    }
                                    else
                                    {
                                        //retrieve based on camera type if z stack enabled
                                        if (XmlManager.GetAttribute(ndList[0], expDoc, "enable", ref str) && (Int32.TryParse(str, out iVal)) && (1 == iVal))
                                        {
                                            ndList = (ICamera.CameraType.LSM == (ICamera.CameraType)ResourceManagerCS.GetCameraType()) ?
                                                expDoc.SelectNodes("/ThorImageExperiment/LSM") :
                                                expDoc.SelectNodes("/ThorImageExperiment/Camera");

                                            if (ndList.Count > 0)
                                            {
                                                if (XmlManager.GetAttribute(ndList[0], expDoc, "averageMode", ref str) && (Int32.TryParse(str, out iVal)) && 1 == iVal)
                                                {
                                                    if (XmlManager.GetAttribute(ndList[0], expDoc, "averageNum", ref str) && (Int32.TryParse(str, out iVal)))
                                                        _triggerStruct.framePerZSlice = iVal;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    case ExperimentModes.EXP_SETUP:
                        {
                            //retrieve through mvm manager since no persistance to active.xml
                            switch ((ICamera.CameraType)ResourceManagerCS.GetCameraType())
                            {
                                case ICamera.CameraType.CCD:
                                    if (1 == (int)MVMManager.Instance["CameraControlViewModel", "CamAverageMode", (object)0])
                                        _triggerStruct.framePerZSlice = (int)MVMManager.Instance["CameraControlViewModel", "CamAverageNum", (object)1];
                                    break;
                                case ICamera.CameraType.LSM:
                                    if (1 == (int)MVMManager.Instance["ScanControlViewModel", "LSMSignalAverage", (object)0])
                                        _triggerStruct.framePerZSlice = (int)MVMManager.Instance["ScanControlViewModel", "LSMSignalAverageFrames", (object)1];
                                    break;
                                default:
                                    break;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        private bool SetTriggerStruct()
        {
            byte[] tByteArray = ResourceManagerCS.StructToByteArray(_triggerStruct);
            bool ret = (1 != ResourceManagerCS.SetDeviceParamBuffer((int)SelectedHardware.SELECTED_EPHYS, (int)IDevice.Params.PARAM_EPHYS_TRIG_BUFFER, tByteArray, tByteArray.Length, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT)) ? true : false;
            OnPropertyChanged("TriggerError");
            return ret;
        }

        private void _statusTimer_Tick(object sender, EventArgs e)
        {
            OnPropertyChanged("TriggerImagePath");
        }

        #endregion Methods
    }
}