﻿namespace MultiLaserControl.ViewModel
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Reflection;
    using System.Windows;
    using System.Windows.Input;
    using System.Xml;

    using MultiLaserControl.Model;

    using ThorSharedTypes;

    class MultiLaserControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly MultiLaserControlModel _MultiLaserControlModel;

        private Visibility _AllLaserVisibility; //All Laser Panel Visibility
        private int _enableLaser1;
        private int _laser1Wavelength;
        private string _EnableLaser1Content;
        private int _enableLaser2;
        private int _laser2Wavelength;
        private string _EnableLaser2Content;
        private int _enableLaser3;
        private int _laser3Wavelength;
        private string _EnableLaser3Content;
        private int _enableLaser4;
        private int _laser4Wavelength;
        private string _EnableLaser4Content;
        private bool _EnableMultiLaserControlPanel;
        private ICommand _Laser1PowerMinusCommand;
        private ICommand _Laser1PowerPlusCommand;
        private ICommand _Laser2PowerMinusCommand;
        private ICommand _Laser2PowerPlusCommand;
        private ICommand _Laser3PowerMinusCommand;
        private ICommand _Laser3PowerPlusCommand;
        private ICommand _Laser4PowerMinusCommand;
        private ICommand _Laser4PowerPlusCommand;
        private Visibility _OriginalLaserVisibility; //Original panels from MCLS project Visibility
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private Visibility _SpLaser1Visibility; //Laser 1 Panel Visibility
        private Visibility _SpLaser2Visibility; //Laser 2 Panel Visibility
        private Visibility _SpLaser3Visibility; //Laser 3 Panel Visibility
        private Visibility _SpLaser4Visibility; //Laser 4 Panel Visibility
        private Visibility _SpMainLaserVisibility; //Used for OTM
        private Visibility _TopticaVisibility; //Toptica Laser Specific Visibility (Wavelength, TTL, Analog, All Lasers)

        #endregion Fields

        #region Constructors

        public MultiLaserControlViewModel()
        {
            this._MultiLaserControlModel = new MultiLaserControlModel();
        }

        #endregion Constructors

        #region Properties

        //Visibility for stackpanel containing controls that affect all lasers
        public Visibility AllLaserVisibility
        {
            get
            {

                return _AllLaserVisibility;
            }
            set
            {
                _AllLaserVisibility = value;
                OnPropertyChanged("AllLaserVisibility");

            }
        }

        //Label for MCLS/OTM
        public string EnableLaser1Content
        {
            get
            {
                return _EnableLaser1Content;
            }
            set
            {
                _EnableLaser1Content = value;
                OnPropertyChanged("EnableLaser1Content");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedEnableLaser1Content");
            }
        }

        //Label for MCLS/OTM
        public string EnableLaser2Content
        {
            get
            {
                return _EnableLaser2Content;
            }
            set
            {
                _EnableLaser2Content = value;
                OnPropertyChanged("EnableLaser2Content");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedEnableLaser2Content");
            }
        }

        //Label for MCLS/OTM
        public string EnableLaser3Content
        {
            get
            {
                return _EnableLaser3Content;
            }
            set
            {
                _EnableLaser3Content = value;
                OnPropertyChanged("EnableLaser3Content");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedEnableLaser3Content");
            }
        }

        //Label for MCLS/OTM
        public string EnableLaser4Content
        {
            get
            {
                return _EnableLaser4Content;
            }
            set
            {
                _EnableLaser4Content = value;
                OnPropertyChanged("EnableLaser4Content");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedEnableLaser4Content");
            }
        }

        //Multi Laser Control Panel not shown when False
        public bool EnableMultiLaserControlPanel
        {
            get
            {
                return _EnableMultiLaserControlPanel;
            }
            set
            {
                _EnableMultiLaserControlPanel = value;
                OnPropertyChange("EnableMultiLaserControlPanel");
            }
        }

        public int Laser1Enable
        {
            get
            {
                _enableLaser1 = this._MultiLaserControlModel.Laser1Enable;
                OnPropertyChanged("LaserAllEnable");
                return _enableLaser1;
            }
            set
            {
                this._MultiLaserControlModel.Laser1Enable = value;
                OnPropertyChanged("Laser1Enable");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser1Enable");
            }
        }

        public int CollapsedLaser1Enable
        {
            get
            {
                return _enableLaser1;
            }
        }

        public double Laser1Max
        {
            get
            {
                return this._MultiLaserControlModel.Laser1Max;
            }
        }

        public double Laser1Min
        {
            get
            {
                return this._MultiLaserControlModel.Laser1Min;
            }
        }

        public double Laser1Power
        {
            get
            {
                return Math.Round(this._MultiLaserControlModel.Laser1Power, 2);
            }
            set
            {
                this._MultiLaserControlModel.Laser1Power = Math.Round(value, 2);
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

        //Label for Laser 1 when wavelength can be queried
        public int Laser1Wavelength
        {
            get
            {
                _laser1Wavelength = this._MultiLaserControlModel.Laser1Wavelength;
                return _laser1Wavelength;
            }
        }

        public int CollapsedLaser1Wavelength
        {
            get
            {
                return _laser1Wavelength;
            }
        }

        public int Laser2Enable
        {
            get
            {
                _enableLaser2 = this._MultiLaserControlModel.Laser2Enable;
                OnPropertyChanged("LaserAllEnable");
                return _enableLaser2;
            }
            set
            {
                this._MultiLaserControlModel.Laser2Enable = value;
                OnPropertyChanged("Laser2Enable");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser2Enable");
            }
        }

        public int CollapsedLaser2Enable
        {
            get
            {
                return _enableLaser2;
            }
        }

        public double Laser2Max
        {
            get
            {
                return this._MultiLaserControlModel.Laser2Max;
            }
        }

        public double Laser2Min
        {
            get
            {
                return this._MultiLaserControlModel.Laser2Min;
            }
        }

        public double Laser2Power
        {
            get
            {
                return Math.Round(this._MultiLaserControlModel.Laser2Power, 2);
            }
            set
            {
                this._MultiLaserControlModel.Laser2Power = Math.Round(value, 2);
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

        //Label for Laser 2 when wavelength can be queried
        public int Laser2Wavelength
        {
            get
            {
                _laser2Wavelength = this._MultiLaserControlModel.Laser2Wavelength;
                return _laser2Wavelength;
            }
        }

        public int CollapsedLaser2Wavelength
        {
            get
            {
                return _laser2Wavelength;
            }
        }

        public int Laser3Enable
        {
            get
            {
                _enableLaser3 = this._MultiLaserControlModel.Laser3Enable;
                OnPropertyChanged("LaserAllEnable");
                return _enableLaser3;
            }
            set
            {
                this._MultiLaserControlModel.Laser3Enable = value;
                OnPropertyChanged("Laser3Enable");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser3Enable");
            }
        }

        public int CollapsedLaser3Enable
        {
            get
            {
                return _enableLaser3;
            }
        }

        public double Laser3Max
        {
            get
            {
                return this._MultiLaserControlModel.Laser3Max;
            }
        }

        public double Laser3Min
        {
            get
            {
                return this._MultiLaserControlModel.Laser3Min;
            }
        }

        public double Laser3Power
        {
            get
            {
                return Math.Round(this._MultiLaserControlModel.Laser3Power, 2);
            }
            set
            {
                this._MultiLaserControlModel.Laser3Power = Math.Round(value, 2);
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

        //Label for Laser 3 when wavelength can be queried
        public int Laser3Wavelength
        {
            get
            {
                _laser3Wavelength = this._MultiLaserControlModel.Laser3Wavelength;
                return _laser3Wavelength;
            }
        }

        public int CollapsedLaser3Wavelength
        {
            get
            {
                return _laser3Wavelength;
            }
        }

        public int Laser4Enable
        {
            get
            {
                _enableLaser4 = this._MultiLaserControlModel.Laser4Enable;
                OnPropertyChanged("LaserAllEnable");
                return _enableLaser4;
            }
            set
            {
                this._MultiLaserControlModel.Laser4Enable = value;
                OnPropertyChanged("Laser4Enable");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedLaser4Enable");
            }
        }

        public int CollapsedLaser4Enable
        {
            get
            {
                return _enableLaser4;
            }
        }

        public double Laser4Max
        {
            get
            {
                return this._MultiLaserControlModel.Laser4Max;
            }
        }

        public double Laser4Min
        {
            get
            {
                return this._MultiLaserControlModel.Laser4Min;
            }
        }

        public double Laser4Power
        {
            get
            {
                return Math.Round(this._MultiLaserControlModel.Laser4Power, 2);
            }
            set
            {

                this._MultiLaserControlModel.Laser4Power = Math.Round(value, 2);
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

        //Label for Laser 4 when wavelength can be queried
        public int Laser4Wavelength
        {
            get
            {
                _laser4Wavelength = this._MultiLaserControlModel.Laser4Wavelength;
                return _laser4Wavelength;
            }
        }

        public int CollapsedLaser4Wavelength
        {
            get
            {
                return _laser4Wavelength;
            }
        }

        //Turns analog mode on for all lasers
        public int LaserAllAnalog
        {
            get
            {
                return this._MultiLaserControlModel.LaserAllAnalog;
            }
            set
            {
                this._MultiLaserControlModel.LaserAllAnalog = value;
                OnPropertyChanged("LaserAllAnalog");
            }
        }

        public int LaserAllEnable
        {
            get
            {
                if (_enableLaser1 == 1 && _enableLaser2 == 1 && _enableLaser3 == 1 && _enableLaser4 == 1)
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            set
            {
                Laser1Enable = value;
                Laser2Enable = value;
                Laser3Enable = value;
                Laser4Enable = value;
            }
        }

        //Turns digital lines on for all lasers
        public int LaserAllTTL
        {
            get
            {
                return this._MultiLaserControlModel.LaserAllTTL;
            }
            set
            {
                this._MultiLaserControlModel.LaserAllTTL = value;
                OnPropertyChanged("LaserAllTTL");
            }
        }

        //Used on dropdown for OTM
        public int MainLaserIndex
        {
            get
            {
                return this._MultiLaserControlModel.MainLaserIndex;
            }
            set
            {
                this._MultiLaserControlModel.MainLaserIndex = value;
                OnPropertyChanged("MainLaserIndex");
            }
        }

        //Visibility for old MCLS controls
        public Visibility OriginalLaserVisibility
        {
            get
            {

                return _OriginalLaserVisibility;
            }
            set
            {
                _OriginalLaserVisibility = value;
                OnPropertyChanged("OriginalLaserVisibility");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedOriginalLaserVisibility");
            }
        }

        //Visibilities of lasers 1-4
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

        //OTM visibility
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

        //Visibility for new Toptica controls
        public Visibility TopticaVisibility
        {
            get
            {

                return _TopticaVisibility;
            }
            set
            {
                _TopticaVisibility = value;
                OnPropertyChanged("TopticaVisibility");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedTopticaVisibility");
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
                myPropInfo = typeof(MultiLaserControlViewModel).GetProperty(propertyName);
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
                if (XmlManager.GetAttribute(ndList[0], expDoc, "allttl", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        LaserAllTTL = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "allanalog", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        LaserAllAnalog = tmp;
                    }
                }
            }

            ndList = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS].SelectNodes("/HardwareSettings/Lasers/Laser");

            if (ndList.Count >= 4)
            {
                EnableLaser1Content = string.Format("{0} Enable", ndList[0].Attributes["name"].Value);
                EnableLaser2Content = string.Format("{0} Enable", ndList[1].Attributes["name"].Value);
                EnableLaser3Content = string.Format("{0} Enable", ndList[2].Attributes["name"].Value);
                EnableLaser4Content = string.Format("{0} Enable", ndList[3].Attributes["name"].Value);

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
                if (ndList[0].Attributes["allenable"] == null)
                {
                    XmlAttribute attr = expDoc.CreateAttribute("allenable");
                    ndList[0].Attributes.Append(attr);
                }
                if (ndList[0].Attributes["allttl"] == null)
                {
                    XmlAttribute attr = expDoc.CreateAttribute("allttl");
                    ndList[0].Attributes.Append(attr);
                }
                if (ndList[0].Attributes["allanalog"] == null)
                {
                    XmlAttribute attr = expDoc.CreateAttribute("allanalog");
                    ndList[0].Attributes.Append(attr);
                }
                if (ndList[0].Attributes["wavelength1"] == null)
                {
                    XmlAttribute attr = expDoc.CreateAttribute("wavelength1");
                    ndList[0].Attributes.Append(attr);
                }
                if (ndList[0].Attributes["wavelength2"] == null)
                {
                    XmlAttribute attr = expDoc.CreateAttribute("wavelength2");
                    ndList[0].Attributes.Append(attr);
                }
                if (ndList[0].Attributes["wavelength3"] == null)
                {
                    XmlAttribute attr = expDoc.CreateAttribute("wavelength3");
                    ndList[0].Attributes.Append(attr);
                }
                if (ndList[0].Attributes["wavelength4"] == null)
                {
                    XmlAttribute attr = expDoc.CreateAttribute("wavelength4");
                    ndList[0].Attributes.Append(attr);
                }

                XmlManager.SetAttribute(ndList[0], expDoc, "MainLaserSelection", MainLaserIndex.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "enable1", Laser1Enable.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power1", Laser1Power.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "enable2", Laser2Enable.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power2", Laser2Power.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "enable3", Laser3Enable.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power3", Laser3Power.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "enable4", Laser4Enable.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power4", Laser4Power.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "allenable", LaserAllEnable.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "allttl", LaserAllTTL.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "allanalog", LaserAllAnalog.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "wavelength1", Laser1Wavelength.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "wavelength2", Laser2Wavelength.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "wavelength3", Laser3Wavelength.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "wavelength4", Laser4Wavelength.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power1percent", power1percent.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power2percent", power2percent.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power3percent", power3percent.ToString());
                XmlManager.SetAttribute(ndList[0], expDoc, "power4percent", power4percent.ToString());
            }
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
                            EnableMultiLaserControlPanel = false;
                            SpMainLaserVisibility = Visibility.Collapsed;
                            SpLaser1Visibility = Visibility.Collapsed;
                            SpLaser2Visibility = Visibility.Collapsed;
                            SpLaser3Visibility = Visibility.Collapsed;
                            SpLaser4Visibility = Visibility.Collapsed;
                            OriginalLaserVisibility = Visibility.Collapsed;
                            AllLaserVisibility = Visibility.Collapsed;
                            TopticaVisibility = Visibility.Collapsed;
                        }
                        else
                        {
                            EnableMultiLaserControlPanel = true;
                            if (str.Contains("OTMLaser"))
                            {
                                SpMainLaserVisibility = Visibility.Visible;
                                SpLaser1Visibility = Visibility.Visible;
                                SpLaser2Visibility = Visibility.Visible;
                                SpLaser3Visibility = Visibility.Collapsed;
                                SpLaser4Visibility = Visibility.Collapsed;
                                OriginalLaserVisibility = Visibility.Visible;
                                AllLaserVisibility = Visibility.Collapsed;
                                TopticaVisibility = Visibility.Collapsed;
                            }

                            else if (str.Contains("TopticaiChrome"))
                            {
                                SpMainLaserVisibility = Visibility.Collapsed;
                                SpLaser1Visibility = Visibility.Visible;
                                SpLaser2Visibility = Visibility.Visible;
                                SpLaser3Visibility = Visibility.Visible;
                                SpLaser4Visibility = Visibility.Visible;
                                OriginalLaserVisibility = Visibility.Collapsed;
                                AllLaserVisibility = Visibility.Visible;
                                TopticaVisibility = Visibility.Visible;
                                //Repopulates Wavelength labels after switching from MCLS to Toptica
                                if (_laser1Wavelength == 0) 
                                {
                                    OnPropertyChange("Laser1Wavelength");
                                }
                                if (_laser2Wavelength == 0)
                                {
                                    OnPropertyChange("Laser2Wavelength");
                                }
                                if (_laser3Wavelength == 0)
                                {
                                    OnPropertyChange("Laser3Wavelength");
                                }
                                if (_laser4Wavelength == 0)
                                {
                                    OnPropertyChange("Laser4Wavelength");
                                }
                            }

                            else //MCLS
                            {
                                SpMainLaserVisibility = Visibility.Collapsed;
                                SpLaser1Visibility = Visibility.Visible;
                                SpLaser2Visibility = Visibility.Visible;
                                SpLaser3Visibility = Visibility.Visible;
                                SpLaser4Visibility = Visibility.Visible;
                                OriginalLaserVisibility = Visibility.Visible;
                                AllLaserVisibility = Visibility.Collapsed;
                                TopticaVisibility = Visibility.Collapsed;
                            }
                        }
                    }
                }
            }
        }

        #endregion Methods
    }
}