namespace OTMControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Input;
    using System.Xml;

    using OTMControl.Model;

    using ThorSharedTypes;

    public class OTMControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private ICommand _centerTrapCommand = null;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private ICommand _saveCalibrationCommand = null;
        private ICommand _trapAOffsetMinusCommand = null;
        private ICommand _trapAOffsetPlusCommand = null;
        private ICommand _trapAOffsetResetCommand = null;
        private ICommand _trapAScaleMinusCommand = null;
        private ICommand _trapAScalePlusCommand = null;
        private ICommand _trapAScaleResetCommand = null;
        private ICommand _trapAStepUMMinusCommand = null;
        private ICommand _trapAStepUMPlusCommand = null;
        private ICommand _trapAumMinusCommand = null;
        private ICommand _trapAumPlusCommand = null;
        private ICommand _trapBOffsetMinusCommand = null;
        private ICommand _trapBOffsetPlusCommand = null;
        private ICommand _trapBOffsetResetCommand = null;
        private ICommand _trapBScaleMinusCommand = null;
        private ICommand _trapBScalePlusCommand = null;
        private ICommand _trapBScaleResetCommand = null;
        private ICommand _trapBStepUMMinusCommand = null;
        private ICommand _trapBStepUMPlusCommand = null;
        private ICommand _trapBumMinusCommand = null;
        private ICommand _trapBumPlusCommand = null;
        private Int64 _trapLastCalibTimeUnix = 0;

        #endregion Fields

        #region Constructors

        public OTMControlViewModel()
        {
            InitializeProperties();
        }

        #endregion Constructors

        #region Enumerations

        public enum TrapAxis
        {
            AXIS_X,
            AXIS_Y,
            AXIS_Z
        }

        public enum TrapPath
        {
            PATH_A,
            PATH_B
        }

        #endregion Enumerations

        #region Properties

        public ICommand CenterTrapCommand
        {
            get
            {
                if (this._centerTrapCommand == null)
                    this._centerTrapCommand = new RelayCommandWithParam((x) => CenterTrap(x));

                return this._centerTrapCommand;
            }
        }

        public XmlDocument PersistGlobalOTMCalibration
        {
            set
            {
                SaveOTMCalibration(value);
            }
        }

        public double PixelSizeUM
        {
            get { return (double)MVMManager.Instance["CaptureSetupViewModel", "PixelSizeUM", (object)1.0]; }
        }

        public ICommand SaveCalibrationCommand
        {
            get
            {
                if (this._saveCalibrationCommand == null)
                    this._saveCalibrationCommand = new RelayCommand(() => SaveCalibration());

                return this._saveCalibrationCommand;
            }
        }

        public CustomCollection<HwVal<double>> TrapAOffset
        {
            get;
            set;
        }

        public ICommand TrapAOffsetMinusCommand
        {
            get
            {
                if (this._trapAOffsetMinusCommand == null)
                    this._trapAOffsetMinusCommand = new RelayCommandWithParam((x) => TrapAOffsetMinus(x));

                return this._trapAOffsetMinusCommand;
            }
        }

        public ICommand TrapAOffsetPlusCommand
        {
            get
            {
                if (this._trapAOffsetPlusCommand == null)
                    this._trapAOffsetPlusCommand = new RelayCommandWithParam((x) => TrapAOffsetPlus(x));

                return this._trapAOffsetPlusCommand;
            }
        }

        public ICommand TrapAOffsetResetCommand
        {
            get
            {
                if (this._trapAOffsetResetCommand == null)
                    this._trapAOffsetResetCommand = new RelayCommandWithParam((x) => TrapAOffsetReset(x));

                return this._trapAOffsetResetCommand;
            }
        }

        public CustomCollection<PC<double>> TrapAScale
        {
            get;
            set;
        }

        public ICommand TrapAScaleMinusCommand
        {
            get
            {
                if (this._trapAScaleMinusCommand == null)
                    this._trapAScaleMinusCommand = new RelayCommandWithParam((x) => TrapAScaleMinus(x));

                return this._trapAScaleMinusCommand;
            }
        }

        public ICommand TrapAScalePlusCommand
        {
            get
            {
                if (this._trapAScalePlusCommand == null)
                    this._trapAScalePlusCommand = new RelayCommandWithParam((x) => TrapAScalePlus(x));

                return this._trapAScalePlusCommand;
            }
        }

        public ICommand TrapAScaleResetCommand
        {
            get
            {
                if (this._trapAScaleResetCommand == null)
                    this._trapAScaleResetCommand = new RelayCommandWithParam((x) => TrapAScaleReset(x));

                return this._trapAScaleResetCommand;
            }
        }

        public CustomCollection<PC<double>> TrapAStepUM
        {
            get;
            set;
        }

        public ICommand TrapAStepUMMinusCommand
        {
            get
            {
                if (this._trapAStepUMMinusCommand == null)
                    this._trapAStepUMMinusCommand = new RelayCommandWithParam((x) => TrapAStepUMMinus(x));

                return this._trapAStepUMMinusCommand;
            }
        }

        public ICommand TrapAStepUMPlusCommand
        {
            get
            {
                if (this._trapAStepUMPlusCommand == null)
                    this._trapAStepUMPlusCommand = new RelayCommandWithParam((x) => TrapAStepUMPlus(x));

                return this._trapAStepUMPlusCommand;
            }
        }

        public CustomCollection<HwVal<double>> TrapAum
        {
            get;
            set;
        }

        public ICommand TrapAumMinusCommand
        {
            get
            {
                if (this._trapAumMinusCommand == null)
                    this._trapAumMinusCommand = new RelayCommandWithParam((x) => TrapAumMinus(x));

                return this._trapAumMinusCommand;
            }
        }

        public ICommand TrapAumPlusCommand
        {
            get
            {
                if (this._trapAumPlusCommand == null)
                    this._trapAumPlusCommand = new RelayCommandWithParam((x) => TrapAumPlus(x));

                return this._trapAumPlusCommand;
            }
        }

        public CustomCollection<HwVal<double>> TrapBOffset
        {
            get;
            set;
        }

        public ICommand TrapBOffsetMinusCommand
        {
            get
            {
                if (this._trapBOffsetMinusCommand == null)
                    this._trapBOffsetMinusCommand = new RelayCommandWithParam((x) => TrapBOffsetMinus(x));

                return this._trapBOffsetMinusCommand;
            }
        }

        public ICommand TrapBOffsetPlusCommand
        {
            get
            {
                if (this._trapBOffsetPlusCommand == null)
                    this._trapBOffsetPlusCommand = new RelayCommandWithParam((x) => TrapBOffsetPlus(x));

                return this._trapBOffsetPlusCommand;
            }
        }

        public ICommand TrapBOffsetResetCommand
        {
            get
            {
                if (this._trapBOffsetResetCommand == null)
                    this._trapBOffsetResetCommand = new RelayCommandWithParam((x) => TrapBOffsetReset(x));

                return this._trapBOffsetResetCommand;
            }
        }

        public CustomCollection<PC<double>> TrapBScale
        {
            get;
            set;
        }

        public ICommand TrapBScaleMinusCommand
        {
            get
            {
                if (this._trapBScaleMinusCommand == null)
                    this._trapBScaleMinusCommand = new RelayCommandWithParam((x) => TrapBScaleMinus(x));

                return this._trapBScaleMinusCommand;
            }
        }

        public ICommand TrapBScalePlusCommand
        {
            get
            {
                if (this._trapBScalePlusCommand == null)
                    this._trapBScalePlusCommand = new RelayCommandWithParam((x) => TrapBScalePlus(x));

                return this._trapBScalePlusCommand;
            }
        }

        public ICommand TrapBScaleResetCommand
        {
            get
            {
                if (this._trapBScaleResetCommand == null)
                    this._trapBScaleResetCommand = new RelayCommandWithParam((x) => TrapBScaleReset(x));

                return this._trapBScaleResetCommand;
            }
        }

        public CustomCollection<PC<double>> TrapBStepUM
        {
            get;
            set;
        }

        public ICommand TrapBStepUMMinusCommand
        {
            get
            {
                if (this._trapBStepUMMinusCommand == null)
                    this._trapBStepUMMinusCommand = new RelayCommandWithParam((x) => TrapBStepUMMinus(x));

                return this._trapBStepUMMinusCommand;
            }
        }

        public ICommand TrapBStepUMPlusCommand
        {
            get
            {
                if (this._trapBStepUMPlusCommand == null)
                    this._trapBStepUMPlusCommand = new RelayCommandWithParam((x) => TrapBStepUMPlus(x));

                return this._trapBStepUMPlusCommand;
            }
        }

        public CustomCollection<HwVal<double>> TrapBum
        {
            get;
            set;
        }

        public ICommand TrapBumMinusCommand
        {
            get
            {
                if (this._trapBumMinusCommand == null)
                    this._trapBumMinusCommand = new RelayCommandWithParam((x) => TrapBumMinus(x));

                return this._trapBumMinusCommand;
            }
        }

        public ICommand TrapBumPlusCommand
        {
            get
            {
                if (this._trapBumPlusCommand == null)
                    this._trapBumPlusCommand = new RelayCommandWithParam((x) => TrapBumPlus(x));

                return this._trapBumPlusCommand;
            }
        }

        public int TrapCalAlert
        {
            get
            {
                return ((DateTime.Now - TrapLastCalibTime).TotalDays > System.Globalization.DateTimeFormatInfo.CurrentInfo.DayNames.Length) ? (int)RangeEnum.YELLOW : (int)RangeEnum.NO_COLOR;
            }
        }

        public CustomCollection<PC<bool>> TrapEnable
        {
            get;
            set;
        }

        public DateTime TrapLastCalibTime
        {
            get { return ResourceManagerCS.ToDateTimeFromUnix(_trapLastCalibTimeUnix); }
        }

        public Int64 TrapLastCalibTimeUnix
        {
            get { return _trapLastCalibTimeUnix; }
            set
            {
                _trapLastCalibTimeUnix = value;
                OnPropertyChanged("TrapLastCalibTimeUnix");
                OnPropertyChanged("TrapLastCalibTime");
                OnPropertyChanged("TrapCalAlert");
            }
        }

        public int TrapMode
        {
            get;
            set;
        }

        public bool ViewModelIsLoad
        {
            get;
            set;
        }

        public PC<double> Volt2UM
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
                myPropInfo = typeof(OTMControlViewModel).GetProperty(propertyName);
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

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/OTM");

            string str = string.Empty;
            int iVal = 0;
            double dVal = 0.0;
            Int64 lVal = 0;

            if (ndList.Count > 0)
            {
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapMode", ref str)) && (Int32.TryParse(str, out iVal)))
                {
                    this.TrapMode = iVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "Volt2UM", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.Volt2UM.Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapAEnable", ref str)) && (Int32.TryParse(str, out iVal)))
                {
                    this.TrapEnable[0].Value = (1 == iVal) ? true : false;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapAXum", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapAum[0].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapAYum", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapAum[1].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapAZum", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapAum[2].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapAXStepUM", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapAStepUM[0].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapAYStepUM", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapAStepUM[1].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapAZStepUM", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapAStepUM[2].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapAOffsetX", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapAOffset[0].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapAOffsetY", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapAOffset[1].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapAScaleX", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapAScale[0].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapAScaleY", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapAScale[1].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapBEnable", ref str)) && (Int32.TryParse(str, out iVal)))
                {
                    this.TrapEnable[1].Value = (1 == iVal) ? true : false;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapBXum", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapBum[0].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapBYum", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapBum[1].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapBZum", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapBum[2].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapBXStepUM", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapBStepUM[0].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapBYStepUM", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapBStepUM[1].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapBZStepUM", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapBStepUM[2].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapBOffsetX", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapBOffset[0].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapBOffsetY", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapBOffset[1].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapBScaleX", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapBScale[0].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapBScaleY", ref str)) && (Double.TryParse(str, out dVal)))
                {
                    this.TrapBScale[1].Value = dVal;
                }
                if ((XmlManager.GetAttribute(ndList[0], doc, "TrapLastCalibTimeUnix", ref str)) && (Int64.TryParse(str, out lVal)))
                {
                    this.TrapLastCalibTimeUnix = lVal;
                }
            }
        }

        public void OnPropertyChange(string propertyName)
        {
        }

        public void UpdateExpXMLSettings(ref XmlDocument xmlDoc)
        {
            //return if not loaded
            if (!ViewModelIsLoad)
                return;

            XmlNodeList ndList = xmlDoc.SelectNodes("/ThorImageExperiment/OTM");
            if (0 >= ndList.Count)
            {
                XmlManager.CreateXmlNode(xmlDoc, "OTM");
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/OTM");
            }
            if (0 < ndList.Count)
            {
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapMode", this.TrapMode.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "Volt2UM", this.Volt2UM.Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapAEnable", ((this.TrapEnable[0].Value) ? (int)1 : 0).ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapAXum", this.TrapAum[0].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapAYum", this.TrapAum[1].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapAZum", this.TrapAum[2].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapAXStepUM", this.TrapAStepUM[0].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapAYStepUM", this.TrapAStepUM[1].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapAZStepUM", this.TrapAStepUM[2].Value.ToString());

                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapBEnable", ((this.TrapEnable[1].Value) ? (int)1 : 0).ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapBXum", this.TrapBum[0].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapBYum", this.TrapBum[1].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapBXStepUM", this.TrapBStepUM[0].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapBYStepUM", this.TrapBStepUM[1].Value.ToString());

                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapLastCalibTimeUnix", this.TrapLastCalibTimeUnix.ToString());
            }
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "CenterLSMScanners")]
        private static extern bool CenterLSMScanners(int selectedCamera);

        private static void EnforceTrap(int idx, double val)
        {
            ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_CENTER_WITH_OFFSET, (int)1);
            CenterLSMScanners((int)SelectedHardware.SELECTED_BLEACHINGSCANNER);
        }

        private void CenterTrap(object x)
        {
            int idx = Int32.Parse(x.ToString());

            switch ((TrapPath)idx)
            {
                case TrapPath.PATH_A:
                    this.TrapAum[0].Value = 0.0;
                    this.TrapAum[1].Value = 0.0;
                    break;
                case TrapPath.PATH_B:
                    this.TrapBum[0].Value = 0.0;
                    this.TrapBum[1].Value = 0.0;
                    break;
                default:
                    break;
            }
        }

        private void InitializeProperties()
        {
            Volt2UM = new PC<double>(1.0);
            TrapEnable = new CustomCollection<PC<bool>>() { new PC<bool>(false), new PC<bool>(false) };

            TrapAum = new CustomCollection<HwVal<double>>();
            TrapAum.Add(new HwVal<double>((int)TrapAxis.AXIS_X, (int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_HIGHRES_OFFSET_X));
            TrapAum.Add(new HwVal<double>((int)TrapAxis.AXIS_Y, (int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_HIGHRES_OFFSET_Y));
            TrapAum.Add(new HwVal<double>((int)TrapAxis.AXIS_Z, (int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POS, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT));

            TrapAum[(int)TrapAxis.AXIS_X].AdditionalSetLogic = (x, y) => EnforceTrap(x, y);
            TrapAum[(int)TrapAxis.AXIS_Y].AdditionalSetLogic = (x, y) => EnforceTrap(x, y);

            TrapAStepUM = new CustomCollection<PC<double>>() { new PC<double>(0.0), new PC<double>(0.0), new PC<double>(0.0) };

            TrapAOffset = new CustomCollection<HwVal<double>>();
            TrapAOffset.Add(new HwVal<double>((int)TrapAxis.AXIS_X, (int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_X));
            TrapAOffset.Add(new HwVal<double>((int)TrapAxis.AXIS_Y, (int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_Y));

            TrapAScale = new CustomCollection<PC<double>>() { new PC<double>(0.0), new PC<double>(0.0) };

            TrapBum = new CustomCollection<HwVal<double>>();
            TrapBum.Add(new HwVal<double>((int)TrapAxis.AXIS_X, (int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_HIGHRES_OFFSET_X2));
            TrapBum.Add(new HwVal<double>((int)TrapAxis.AXIS_Y, (int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_HIGHRES_OFFSET_Y2));

            TrapBum[(int)TrapAxis.AXIS_X].AdditionalSetLogic = (x, y) => EnforceTrap(x, y);
            TrapBum[(int)TrapAxis.AXIS_Y].AdditionalSetLogic = (x, y) => EnforceTrap(x, y);

            TrapBStepUM = new CustomCollection<PC<double>>() { new PC<double>(0.0), new PC<double>(0.0) };

            TrapBOffset = new CustomCollection<HwVal<double>>();
            TrapBOffset.Add(new HwVal<double>((int)TrapAxis.AXIS_X, (int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_X2));
            TrapBOffset.Add(new HwVal<double>((int)TrapAxis.AXIS_Y, (int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_Y2));

            TrapBScale = new CustomCollection<PC<double>>() { new PC<double>(0.0), new PC<double>(0.0) };
        }

        private void SaveCalibration()
        {
            //persist calibration as global params to all modalities
            MVMManager.Instance["CaptureSetupViewModel", "PersistGlobalExperimentXMLNow"] = GlobalExpAttribute.OTM;
        }

        private void SaveOTMCalibration(XmlDocument xmlDoc)
        {
            XmlNodeList ndList = xmlDoc.SelectNodes("/ThorImageExperiment/OTM");
            if (0 >= ndList.Count)
            {
                XmlManager.CreateXmlNode(xmlDoc, "OTM");
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/OTM");
            }
            if (0 < ndList.Count)
            {
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapAOffsetX", this.TrapAOffset[0].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapAOffsetY", this.TrapAOffset[1].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapAScaleX", this.TrapAScale[0].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapAScaleY", this.TrapAScale[1].Value.ToString());

                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapBOffsetX", this.TrapBOffset[0].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapBOffsetY", this.TrapBOffset[1].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapBScaleX", this.TrapBScale[0].Value.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "TrapBScaleY", this.TrapBScale[1].Value.ToString());
            }
            TrapLastCalibTimeUnix = ResourceManagerCS.DateTimeToUnixTimestamp(DateTime.Now);
        }

        private void TrapAOffsetMinus(object x)
        {
            this.TrapAOffset[Int32.Parse(x.ToString())].Value -= 0.001;
        }

        private void TrapAOffsetPlus(object x)
        {
            this.TrapAOffset[Int32.Parse(x.ToString())].Value += 0.001;
        }

        private void TrapAOffsetReset(object x)
        {
            this.TrapAOffset[1].Value = this.TrapAOffset[0].Value = 0.0;
        }

        private void TrapAScaleMinus(object x)
        {
            this.TrapAScale[Int32.Parse(x.ToString())].Value -= 0.001;
        }

        private void TrapAScalePlus(object x)
        {
            this.TrapAScale[Int32.Parse(x.ToString())].Value += 0.001;
        }

        private void TrapAScaleReset(object x)
        {
            this.TrapAScale[1].Value = this.TrapAScale[0].Value = 0.0;
        }

        private void TrapAStepUMMinus(object x)
        {
            this.TrapAStepUM[Int32.Parse(x.ToString())].Value -= 0.001;
        }

        private void TrapAStepUMPlus(object x)
        {
            this.TrapAStepUM[Int32.Parse(x.ToString())].Value += 0.001;
        }

        private void TrapAumMinus(object x)
        {
            int idx = Int32.Parse(x.ToString());
            this.TrapAum[idx].Value -= this.TrapAStepUM[idx].Value;
        }

        private void TrapAumPlus(object x)
        {
            int idx = Int32.Parse(x.ToString());
            this.TrapAum[idx].Value += this.TrapAStepUM[idx].Value;
        }

        private void TrapBOffsetMinus(object x)
        {
            this.TrapBOffset[Int32.Parse(x.ToString())].Value -= 0.001;
        }

        private void TrapBOffsetPlus(object x)
        {
            this.TrapBOffset[Int32.Parse(x.ToString())].Value += 0.001;
        }

        private void TrapBOffsetReset(object x)
        {
            this.TrapBOffset[1].Value = this.TrapBOffset[0].Value = 0.0;
        }

        private void TrapBScaleMinus(object x)
        {
            this.TrapBScale[Int32.Parse(x.ToString())].Value -= 0.001;
        }

        private void TrapBScalePlus(object x)
        {
            this.TrapBScale[Int32.Parse(x.ToString())].Value += 0.001;
        }

        private void TrapBScaleReset(object x)
        {
            this.TrapBScale[1].Value = this.TrapBScale[0].Value = 0.0;
        }

        private void TrapBStepUMMinus(object x)
        {
            this.TrapBStepUM[Int32.Parse(x.ToString())].Value -= 0.001;
        }

        private void TrapBStepUMPlus(object x)
        {
            this.TrapBStepUM[Int32.Parse(x.ToString())].Value += 0.001;
        }

        private void TrapBumMinus(object x)
        {
            int idx = Int32.Parse(x.ToString());
            this.TrapBum[idx].Value -= this.TrapBStepUM[idx].Value;
        }

        private void TrapBumPlus(object x)
        {
            int idx = Int32.Parse(x.ToString());
            this.TrapBum[idx].Value += this.TrapBStepUM[idx].Value;
        }

        #endregion Methods
    }
}