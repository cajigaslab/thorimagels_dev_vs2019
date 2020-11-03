namespace LaserControl.ViewModel
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
    using System.Runtime.CompilerServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Xml;

    using LaserControl.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public class LaserControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly LaserControlModel _LaserControlModel;

        private string _CbLaser1Content;
        private string _CbLaser2Content;
        private string _CbLaser3Content;
        private string _CbLaser4Content;
        private ICommand _laser1MinusCommand;
        private ICommand _laser1PlusCommand;
        private ICommand _Laser1PowerMinusCommand;
        private ICommand _Laser1PowerPlusCommand;
        private ICommand _Laser2PowerMinusCommand;
        private ICommand _Laser2PowerPlusCommand;
        private ICommand _Laser3PowerMinusCommand;
        private ICommand _Laser3PowerPlusCommand;
        private ICommand _Laser4PowerMinusCommand;
        private ICommand _Laser4PowerPlusCommand;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private ArrayList _savedWavelengths = new ArrayList { 0, 0, 0, 0, 0, 0, 0, 0 };
        private Visibility _SpLaser1Visibility;
        private Visibility _SpLaser2Visibility;
        private Visibility _SpLaser3Visibility;
        private Visibility _SpLaser4Visibility;
        private Visibility _SpMainLaserVisibility;
        private bool _EnableLaserControlPanel;

        #endregion Fields

        #region Constructors

        public LaserControlViewModel()
        {
            this._LaserControlModel = new LaserControlModel();
        }

        #endregion Constructors

        #region Properties

        public string CbLaser1Content
        {
            get
            {
                return _CbLaser1Content;
            }
            set
            {
                _CbLaser1Content = value;
                OnPropertyChanged("CbLaser1Content");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedCbLaser1Content");
            }
        }

        public string CbLaser2Content
        {
            get
            {
                return _CbLaser2Content;
            }
            set
            {
                _CbLaser2Content = value;
                OnPropertyChanged("CbLaser2Content");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedCbLaser2Content");
            }
        }

        public string CbLaser3Content
        {
            get
            {
                return _CbLaser3Content;
            }
            set
            {
                _CbLaser3Content = value;
                OnPropertyChanged("CbLaser3Content");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedCbLaser3Content");
            }
        }

        public string CbLaser4Content
        {
            get
            {
                return _CbLaser4Content;
            }
            set
            {
                _CbLaser4Content = value;
                OnPropertyChanged("CbLaser4Content");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedCbLaser4Content");
            }
        }

        public int Laser1Enable
        {
            get
            {
                return this._LaserControlModel.Laser1Enable;
            }
            set
            {
                this._LaserControlModel.Laser1Enable = value;
                OnPropertyChanged("Laser1Enable");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser1Enable");
            }
        }

        public double Laser1Max
        {
            get
            {
                return this._LaserControlModel.Laser1Max;
            }
        }

        public double Laser1Min
        {
            get
            {
                return this._LaserControlModel.Laser1Min;
            }
        }

        public ICommand Laser1MinusCommand
        {
            get
            {
                if (this._laser1MinusCommand == null)
                    this._laser1MinusCommand = new RelayCommand(() => Laser1Minus());

                return this._laser1MinusCommand;
            }
        }

        public ICommand Laser1PlusCommand
        {
            get
            {
                if (this._laser1PlusCommand == null)
                    this._laser1PlusCommand = new RelayCommand(() => Laser1Plus());

                return this._laser1PlusCommand;
            }
        }

        public int Laser1Position
        {
            get
            {
                return this._LaserControlModel.Laser1Position;
            }
            set
            {
                this._LaserControlModel.Laser1Position = value;
                OnPropertyChanged("Laser1Position");
            }
        }

        public double Laser1Power
        {
            get
            {
                return this._LaserControlModel.Laser1Power;
            }
            set
            {
                this._LaserControlModel.Laser1Power = value;
                OnPropertyChanged("Laser1Power");
                OnPropertyChanged("Laser1Max");
                OnPropertyChange("Laser1Min");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser1Power");
            }
        }

        public ICommand Laser1PowerMinusCommand
        {
            get
            {
                if (this._Laser1PowerMinusCommand == null)
                    this._Laser1PowerMinusCommand = new RelayCommand(() => Laser1PowerMinus());

                return this._Laser1PowerMinusCommand;
            }
        }

        public ICommand Laser1PowerPlusCommand
        {
            get
            {
                if (this._Laser1PowerPlusCommand == null)
                    this._Laser1PowerPlusCommand = new RelayCommand(() => Laser1PowerPlus());

                return this._Laser1PowerPlusCommand;
            }
        }

        public int Laser2Enable
        {
            get
            {
                return this._LaserControlModel.Laser2Enable;
            }
            set
            {
                this._LaserControlModel.Laser2Enable = value;
                OnPropertyChanged("Laser2Enable");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser2Enable");
            }
        }

        public double Laser2Max
        {
            get
            {
                return this._LaserControlModel.Laser2Max;
            }
        }

        public double Laser2Min
        {
            get
            {
                return this._LaserControlModel.Laser2Min;
            }
        }

        public double Laser2Power
        {
            get
            {
                return this._LaserControlModel.Laser2Power;
            }
            set
            {
                this._LaserControlModel.Laser2Power = value;
                OnPropertyChanged("Laser2Power");
                OnPropertyChanged("Laser2Max");
                OnPropertyChange("Laser2Min");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser2Power");
            }
        }

        public ICommand Laser2PowerMinusCommand
        {
            get
            {
                if (this._Laser2PowerMinusCommand == null)
                    this._Laser2PowerMinusCommand = new RelayCommand(() => Laser2PowerMinus());

                return this._Laser2PowerMinusCommand;
            }
        }

        public ICommand Laser2PowerPlusCommand
        {
            get
            {
                if (this._Laser2PowerPlusCommand == null)
                    this._Laser2PowerPlusCommand = new RelayCommand(() => Laser2PowerPlus());

                return this._Laser2PowerPlusCommand;
            }
        }

        public int Laser3Enable
        {
            get
            {
                return this._LaserControlModel.Laser3Enable;
            }
            set
            {
                this._LaserControlModel.Laser3Enable = value;
                OnPropertyChanged("Laser3Enable");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser3Enable");
            }
        }

        public double Laser3Max
        {
            get
            {
                return this._LaserControlModel.Laser3Max;
            }
        }

        public double Laser3Min
        {
            get
            {
                return this._LaserControlModel.Laser3Min;
            }
        }

        public double Laser3Power
        {
            get
            {
                return this._LaserControlModel.Laser3Power;
            }
            set
            {
                this._LaserControlModel.Laser3Power = value;
                OnPropertyChanged("Laser3Power");
                OnPropertyChanged("Laser3Max");
                OnPropertyChange("Laser3Min");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser3Power");
            }
        }

        public ICommand Laser3PowerMinusCommand
        {
            get
            {
                if (this._Laser3PowerMinusCommand == null)
                    this._Laser3PowerMinusCommand = new RelayCommand(() => Laser3PowerMinus());

                return this._Laser3PowerMinusCommand;
            }
        }

        public ICommand Laser3PowerPlusCommand
        {
            get
            {
                if (this._Laser3PowerPlusCommand == null)
                    this._Laser3PowerPlusCommand = new RelayCommand(() => Laser3PowerPlus());

                return this._Laser3PowerPlusCommand;
            }
        }

        public int Laser4Enable
        {
            get
            {
                return this._LaserControlModel.Laser4Enable;
            }
            set
            {
                this._LaserControlModel.Laser4Enable = value;
                OnPropertyChanged("Laser4Enable");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser4Enable");
            }
        }

        public double Laser4Max
        {
            get
            {
                return this._LaserControlModel.Laser4Max;
            }
        }

        public double Laser4Min
        {
            get
            {
                return this._LaserControlModel.Laser4Min;
            }
        }

        public double Laser4Power
        {
            get
            {
                return this._LaserControlModel.Laser4Power;
            }
            set
            {
                this._LaserControlModel.Laser4Power = value;
                OnPropertyChanged("Laser4Power");
                OnPropertyChanged("Laser4Max");
                OnPropertyChange("Laser4Min");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser4Power");
            }
        }

        public ICommand Laser4PowerMinusCommand
        {
            get
            {
                if (this._Laser4PowerMinusCommand == null)
                    this._Laser4PowerMinusCommand = new RelayCommand(() => Laser4PowerMinus());

                return this._Laser4PowerMinusCommand;
            }
        }

        public ICommand Laser4PowerPlusCommand
        {
            get
            {
                if (this._Laser4PowerPlusCommand == null)
                    this._Laser4PowerPlusCommand = new RelayCommand(() => Laser4PowerPlus());

                return this._Laser4PowerPlusCommand;
            }
        }

        public int MainLaserIndex
        {
            get
            {
                return this._LaserControlModel.MainLaserIndex;
            }
            set
            {
                this._LaserControlModel.MainLaserIndex = value;
                OnPropertyChanged("MainLaserIndex");
            }
        }

        public Visibility SpLaser1Visibility
        {
            get
            {

                return _SpLaser1Visibility;
            }
            set
            {
                _SpLaser1Visibility = value;
                OnPropertyChanged("SpLaser1Visibility");

            }
        }

        public Visibility SpLaser2Visibility
        {
            get
            {

                return _SpLaser2Visibility;
            }
            set
            {
                _SpLaser2Visibility = value;
                OnPropertyChanged("SpLaser2Visibility");

            }
        }

        public Visibility SpLaser3Visibility
        {
            get
            {

                return _SpLaser3Visibility;
            }
            set
            {
                _SpLaser3Visibility = value;
                OnPropertyChanged("SpLaser3Visibility");

            }
        }

        public Visibility SpLaser4Visibility
        {
            get
            {

                return _SpLaser4Visibility;
            }
            set
            {
                _SpLaser4Visibility = value;
                OnPropertyChanged("SpLaser4Visibility");

            }
        }

        public Visibility SpMainLaserVisibility
        {
            get
            {

                return _SpMainLaserVisibility;
            }
            set
            {
                _SpMainLaserVisibility = value;
                OnPropertyChanged("SpMainLaserVisibility");

            }
        }

        public bool EnableLaserControlPanel
        {
            get
            {
                return _EnableLaserControlPanel;
            }
            set
            {
                _EnableLaserControlPanel = value;
                OnPropertyChange("EnableLaserControlPanel");
            }
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

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(LaserControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            XmlDocument expDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
            XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/MCLS");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], expDoc, "MainLaserSelection", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        MainLaserIndex = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "enable1", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Laser1Enable = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "power1", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        Laser1Power = Math.Min(Laser1Max, Math.Max(Laser1Min, tmp));
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "enable2", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Laser2Enable = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "power2", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        Laser2Power = Math.Min(Laser2Max, Math.Max(Laser2Min, tmp));
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "enable3", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Laser3Enable = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "power3", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        Laser3Power = Math.Min(Laser3Max, Math.Max(Laser3Min, tmp));
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "enable4", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Laser4Enable = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "power4", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        Laser4Power = Math.Min(Laser4Max, Math.Max(Laser4Min, tmp));
                    }
                }
            }

            ndList = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS].SelectNodes("/HardwareSettings/Lasers/Laser");

            if (ndList.Count >= 4)
            {
                CbLaser1Content = string.Format("{0} Enable", ndList[0].Attributes["name"].Value);
                CbLaser2Content = string.Format("{0} Enable", ndList[1].Attributes["name"].Value);
                CbLaser3Content = string.Format("{0} Enable", ndList[2].Attributes["name"].Value);
                CbLaser4Content = string.Format("{0} Enable", ndList[3].Attributes["name"].Value);
            }

            SetDisplayOptions();
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        //public int GetSavedWavelength(int i)
        //{
        //    return (int)_savedWavelengths[i];
        //}
        public Decimal Power2PercentConvertion(double value, double Max, double Min)
        {
            if (Max != Min)
            {
                Decimal dec = new Decimal((value - Min) * 100 / (Max - Min));
                return dec = Decimal.Round(dec, 2);
            }
            else return new Decimal(value);
        }

        public void UpdateExpXMLSettings(ref XmlDocument expDoc)
        {
            XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/MCLS");
            decimal power1percent = Power2PercentConvertion(Laser1Power, Laser1Max, Laser1Min);
            decimal power2percent = Power2PercentConvertion(Laser2Power, Laser2Max, Laser2Min);
            decimal power3percent = Power2PercentConvertion(Laser3Power, Laser3Max, Laser3Min);
            decimal power4percent = Power2PercentConvertion(Laser4Power, Laser4Max, Laser4Min);

            if (ndList.Count > 0)
            {
                if (ndList[0].Attributes["MainLaserSelection"] == null)
                {
                    XmlAttribute attr = expDoc.CreateAttribute("MainLaserSelection");
                    ndList[0].Attributes.Append(attr);
                }
                XmlManager.SetAttribute(ndList[0], expDoc,"MainLaserSelection", MainLaserIndex.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc,"enable1", Laser1Enable.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc,"power1", Laser1Power.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc,"enable2", Laser2Enable.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc,"power2", Laser2Power.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc,"enable3", Laser3Enable.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc,"power3", Laser3Power.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc,"enable4", Laser4Enable.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc,"power4", Laser4Power.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power1percent", power1percent.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power2percent", power2percent.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power3percent", power3percent.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power4percent", power4percent.ToString());
            }
        }

        // Multiphoton laser -
        private void Laser1Minus()
        {
            Laser1Position -= 1;
            OnPropertyChanged("Laser1Position");
        }

        // Multiphoton laser +
        private void Laser1Plus()
        {
            Laser1Position += 1;
            OnPropertyChanged("Laser1Position");
        }

        private void Laser1PowerMinus()
        {
            Laser1Power -= (Laser1Max - Laser1Min) / 1000;
        }

        private void Laser1PowerPlus()
        {
            Laser1Power += (Laser1Max - Laser1Min) / 1000;
        }

        private void Laser2PowerMinus()
        {
            Laser2Power -= (Laser2Max - Laser2Min) / 1000;
        }

        private void Laser2PowerPlus()
        {
            Laser2Power += (Laser2Max - Laser2Min) / 1000;
        }

        private void Laser3PowerMinus()
        {
            Laser3Power -= (Laser3Max - Laser3Min) / 1000;
        }

        private void Laser3PowerPlus()
        {
            Laser3Power += (Laser3Max - Laser3Min) / 1000;
        }

        private void Laser4PowerMinus()
        {
            Laser4Power -= (Laser4Max - Laser4Min) / 1000;
        }

        private void Laser4PowerPlus()
        {
            Laser4Power += (Laser4Max - Laser4Min) / 1000;
        }

        private void SetDisplayOptions()
        {
            XmlDocument hDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndListHW = hDoc.SelectNodes("/HardwareSettings/Devices/MCLS");

            //persist the active ZStage name:
            if (ndListHW.Count > 0)
            {
                string str = string.Empty;
                for (int i = 0; i < ndListHW.Count; i++)
                {
                    XmlManager.GetAttribute(ndListHW[i], hDoc, "active", ref str);
                    if (1 == Convert.ToInt32(str))
                    {                      
                        XmlManager.GetAttribute(ndListHW[i], hDoc, "dllName", ref str);

                        if (str.Contains("Disconnected"))
                        {
                            EnableLaserControlPanel = false;
                            SpMainLaserVisibility = Visibility.Collapsed;
                            SpLaser1Visibility = Visibility.Collapsed;
                            SpLaser2Visibility = Visibility.Collapsed;
                            SpLaser3Visibility = Visibility.Collapsed;
                            SpLaser4Visibility = Visibility.Collapsed;
                        }
                        else
                        {
                            EnableLaserControlPanel = true;
                            if (str.Contains("OTMLaser"))
                            {
                                SpMainLaserVisibility = Visibility.Visible;
                                SpLaser1Visibility = Visibility.Visible;
                                SpLaser2Visibility = Visibility.Visible;
                                SpLaser3Visibility = Visibility.Collapsed;
                                SpLaser4Visibility = Visibility.Collapsed;

                            }
                            else
                            {
                                SpMainLaserVisibility = Visibility.Collapsed;
                                SpLaser1Visibility = Visibility.Visible;
                                SpLaser2Visibility = Visibility.Visible;
                                SpLaser3Visibility = Visibility.Visible;
                                SpLaser4Visibility = Visibility.Visible;
                            }
                        }
                    }
                }
            }
        }

        #endregion Methods
    }
}