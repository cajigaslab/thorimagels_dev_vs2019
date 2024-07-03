namespace LampControl.ViewModel
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
    using System.Xml;

    using LampControl.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public class LampControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly LampControlModel _LampControlModel;

        private bool _isLamp1 = false;
        private bool _isLamp1ExtTrig = false;
        private bool _isLamp2 = false;
        private bool _isLamp2ExtTrig = false;
        private bool _isLampEnable = false;
        private bool _lamp1Enable = true;
        private bool _lamp2Enable = true;
        private double _lampPosition1 = 0.0;
        private double _lampPosition2 = 0.0;
        private double _lampPositionInterval = 5;
        private double _lampPositionTickFrequency = 25;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private bool _stopUpdate = false;
        private double _tempLampPosition;

        #endregion Fields

        #region Constructors

        public LampControlViewModel()
        {
            this._LampControlModel = new LampControlModel();
        }

        #endregion Constructors

        #region Properties

        public ICommand ChangeLampPosCommand
        {
            get
            {
                return new RelayCommandWithParam((param) =>
                {
                    if ("+" == param.ToString())
                    {
                        LampPosition += 1;
                    }
                    else
                    {
                        LampPosition -= 1;
                    }
                });
            }
        }

        public bool IsExternalTrigger
        {
            get { return _LampControlModel.IsExternalTrigger; }
            set
            {
                if (IsLamp2)
                {
                    _isLamp1ExtTrig = value;
                }
                else
                {
                    _isLamp2ExtTrig = value;
                }
                _LampControlModel.IsExternalTrigger = value;
                OnPropertyChanged("IsExternalTrigger");
                OnPropertyChanged("LampPosition");
                OnPropertyChanged("TempLampPosition");
            }
        }

        public bool IsLamp1
        {
            get
            {
                return _isLamp1;
            }
            set
            {
                if (Lamp1Enable == true)
                {
                    LampTerminal = 1;
                    _isLamp1 = value;
                    _isLamp2 = false;
                    IsExternalTrigger = _isLamp1ExtTrig;
                    LampPosition = _lampPosition1;
                }
                else
                {
                    _isLamp1 = false;
                }

                OnPropertyChanged("IsLamp2");
                OnPropertyChanged("IsLamp1");
            }
        }

        public bool IsLamp2
        {
            get
            {
                return _isLamp2;
            }
            set
            {
                if (Lamp2Enable == true)
                {
                    LampTerminal = 2;
                    _isLamp2 = value;
                    _isLamp1 = false;
                    IsExternalTrigger = _isLamp2ExtTrig;
                    LampPosition = _lampPosition2;
                }
                else
                {
                    _isLamp2 = false;

                }
                OnPropertyChanged("IsLamp1");
                OnPropertyChanged("IsLamp2");
            }
        }

        public bool IsLampEnabled
        {
            get
            {
                if (Lamp1Enable || Lamp2Enable || IsPrelude)
                {
                    _isLampEnable = true;
                }
                return _isLampEnable;
            }

            set
            {
                _isLampEnable = value;
                OnPropertyChanged("IsLampEnabled");
            }
        }

        public bool IsPrelude
        {
            get
            {
                return (int)LampTypes.Prelude_LED == _LampControlModel.LampType;
            }
        }

        public bool Lamp1Enable
        {
            get
            {

                _lamp1Enable = _LampControlModel.Lamp1Enable;
                if (_lamp1Enable == false)
                    _isLamp1 = false;

                return _lamp1Enable;
            }
            set
            {
                _lamp1Enable = value;
                OnPropertyChanged("Lamp1Enable");
            }
        }

        public bool Lamp2Enable
        {
            get
            {
                _lamp2Enable = _LampControlModel.Lamp2Enable;
                if (_lamp2Enable == false)
                    _isLamp2 = false;
                return _lamp2Enable;
            }
            set
            {

                _lamp2Enable = value;
                OnPropertyChanged("Lamp2Enable");
            }
        }

        public double LampMaxPosition
        {
            get { return 100; }
        }

        public double LampMinPosition
        {
            get { return 0; }
        }

        public bool LampON
        {
            get
            {
                return this._LampControlModel.LampON;
            }
            set
            {
                this._LampControlModel.LampON = value;
                OnPropertyChanged("LampON");
            }
        }

        public double LampPosition
        {
            get
            {
                double lampPosition = _LampControlModel.LampPosition;
                if (_tempLampPosition != lampPosition && false == _stopUpdate)
                {
                    _tempLampPosition = lampPosition;
                    OnPropertyChanged("TempLampPosition");
                }
                return lampPosition;
            }
            set
            {
                if (value < 0 || value > 100)
                {
                    return;
                }
                if (IsLamp2)
                {
                    _lampPosition2 = value;
                }
                else
                {
                    _lampPosition1 = value;
                }
                _LampControlModel.LampPosition = value;
                _tempLampPosition = Math.Round(value, 2);
                _stopUpdate = false;
                OnPropertyChanged("LampPosition");
                OnPropertyChanged("TempLampPosition");
            }
        }

        public double LampPositionInterval
        {
            get
            {
                return _lampPositionInterval;
            }
            set
            {
                _lampPositionInterval = value;
                OnPropertyChanged("LampPositionInterval");
            }
        }

        public double LampPositionTickFrequency
        {
            get { return _lampPositionTickFrequency; }
            set
            {
                _lampPositionTickFrequency = value;
                OnPropertyChanged("LampPositionTickFrequency");
            }
        }

        public double LampPower
        {
            get { return 60; }
        }

        public long LampTerminal
        {
            get
            {
                return _LampControlModel.LampTerminal;
            }
            set
            {
                _LampControlModel.LampTerminal = value;
                OnPropertyChanged("LampTerminal");
            }
        }

        public ICommand SetLampPosCommand
        {
            get
            {
                return new RelayCommand(() =>
                {
                    if (TempLampPosition < 0)
                    {
                        TempLampPosition = 0;
                    }
                    else if (TempLampPosition > 100)
                    {
                        TempLampPosition = 100;
                    }
                    else { }
                    LampPosition = TempLampPosition;
                });
            }
        }

        public double TempLampPosition
        {
            get { return _tempLampPosition; }
            set
            {
                _stopUpdate = true;
                _tempLampPosition = value;
                OnPropertyChanged("TempLampPosition");
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
                myPropInfo = typeof(LampControlViewModel).GetProperty(propertyName);
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

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/LAMP");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                bool tmp;
                int tmpInt = 0;
                double tmpDouble = 0.0;

                if (XmlManager.GetAttribute(ndList[0], doc, "IsExternalTrigger", ref str))
                {
                    if (bool.TryParse(str, out tmp))
                    {
                        //If LampPosition is found, set the current LED terminal to the saved external trigger
                        if (1 == LampTerminal)
                        {
                            _isLamp1ExtTrig = tmp;
                        }
                        else if (2 == LampTerminal)
                        {
                            _isLamp2ExtTrig = tmp;
                        }
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "IsExternalTriggerTerm1", ref str))
                {
                    if (bool.TryParse(str, out tmp))
                    {
                        _isLamp1ExtTrig = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "IsExternalTriggerTerm2", ref str))
                {
                    if (bool.TryParse(str, out tmp))
                    {
                        _isLamp2ExtTrig = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "LampPosition", ref str))
                {
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpDouble))
                    {
                        //If LampPosition is found, set the current LED terminal to the power saved
                        if (1 == LampTerminal)
                        {
                            _lampPosition1 = tmpDouble;
                        }
                        else if (2 == LampTerminal)
                        {
                            _lampPosition2 = tmpDouble;
                        }
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "LampPosition1", ref str))
                {
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpDouble))
                    {
                        _lampPosition1 = tmpDouble;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "LampPosition2", ref str))
                {
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpDouble))
                    {
                        _lampPosition2 = tmpDouble;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "LampTerminal", ref str))
                {
                    if (Int32.TryParse(str, out tmpInt))
                    {
                        if (1 == tmpInt)
                        {
                            IsLamp1 = true;
                        }
                        else if (2 == tmpInt)
                        {
                            IsLamp2 = true;
                        }
                    }
                }
                //If LampTerminal is not found enable the lamp selected in the device
                else
                {
                    if (1 == LampTerminal)
                    {
                        IsLamp1 = true;
                    }
                    else if (2 == LampTerminal)
                    {
                        IsLamp2 = true;
                    }
                }
            }
            else
            {
                IsExternalTrigger = false;
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
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/LAMP");
            if (ndList.Count <= 0)
            {
                ThorSharedTypes.XmlManager.CreateXmlNode(experimentFile, "LAMP");
                ndList = experimentFile.SelectNodes("/ThorImageExperiment/LAMP");
            }
            XmlManager.SetAttribute(ndList[0], experimentFile, "IsExternalTriggerTerm1", this.IsExternalTrigger.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "IsExternalTriggerTerm2", this.IsExternalTrigger.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "LampPosition1", this._lampPosition1.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "LampPosition2", this._lampPosition2.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "LampTerminal", this.LampTerminal.ToString());
            LampON = false;
        }

        #endregion Methods
    }
}