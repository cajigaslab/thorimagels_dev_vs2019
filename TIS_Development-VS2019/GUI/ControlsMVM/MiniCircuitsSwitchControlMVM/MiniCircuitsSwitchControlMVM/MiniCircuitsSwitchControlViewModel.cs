namespace MiniCircuitsSwitchControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Reflection;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Xml;

    using MiniCircuitsSwitchControl.Model;

    using ThorLogging;

    using ThorSharedTypes;

    public class MiniCircuitsSwitchControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly MiniCircuitsSwitchControlModel _miniCircuitsSwitchControlModel;

        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private ICommand _switchPositionNameChangeCommand;

        #endregion Fields

        #region Constructors

        public MiniCircuitsSwitchControlViewModel()
        {
            this._miniCircuitsSwitchControlModel = new MiniCircuitsSwitchControlModel();
        }

        #endregion Constructors

        #region Properties

        public SolidColorBrush A1SwitchColor
        {
            get
            {
                return (1 == A1SwitchPosition) ? new SolidColorBrush(Colors.Orange) : new SolidColorBrush(Colors.LimeGreen);
            }
        }

        public string A1SwitchLeftName
        {
            get;
            set;
        }

        public int A1SwitchPosition
        {
            get
            {
                return this._miniCircuitsSwitchControlModel.A1SwitchPosition;
            }
            set
            {
                if (_miniCircuitsSwitchControlModel.ManualSwitchEnable)
                {
                    this._miniCircuitsSwitchControlModel.A1SwitchPosition = value;
                    OnPropertyChanged("A1SwitchPosition");
                    OnPropertyChanged("A1SwitchColor");
                }
            }
        }

        public string A1SwitchRightName
        {
            get;
            set;
        }

        public SolidColorBrush A2SwitchColor
        {
            get
            {
                return (1 == A2SwitchPosition) ? new SolidColorBrush(Colors.Orange) : new SolidColorBrush(Colors.LimeGreen);
            }
        }

        public string A2SwitchLeftName
        {
            get;
            set;
        }

        public int A2SwitchPosition
        {
            get
            {
                return this._miniCircuitsSwitchControlModel.A2SwitchPosition;
            }
            set
            {
                if (_miniCircuitsSwitchControlModel.ManualSwitchEnable)
                {
                    this._miniCircuitsSwitchControlModel.A2SwitchPosition = value;
                    OnPropertyChanged("A2SwitchPosition");
                    OnPropertyChanged("A2SwitchColor");
                }
            }
        }

        public string A2SwitchRightName
        {
            get;
            set;
        }

        public SolidColorBrush B1SwitchColor
        {
            get
            {
                return (1 == B1SwitchPosition) ? new SolidColorBrush(Colors.Orange) : new SolidColorBrush(Colors.LimeGreen);
            }
        }

        public string B1SwitchLeftName
        {
            get;
            set;
        }

        public int B1SwitchPosition
        {
            get
            {
                return this._miniCircuitsSwitchControlModel.B1SwitchPosition;
            }
            set
            {
                if (_miniCircuitsSwitchControlModel.ManualSwitchEnable)
                {
                    this._miniCircuitsSwitchControlModel.B1SwitchPosition = value;
                    OnPropertyChanged("B1SwitchPosition");
                    OnPropertyChanged("B1SwitchColor");
                }
            }
        }

        public string B1SwitchRightName
        {
            get;
            set;
        }

        public SolidColorBrush B2SwitchColor
        {
            get
            {
                return (1 == B2SwitchPosition) ? new SolidColorBrush(Colors.Orange) : new SolidColorBrush(Colors.LimeGreen);
            }
        }

        public string B2SwitchLeftName
        {
            get;
            set;
        }

        public int B2SwitchPosition
        {
            get
            {
                return this._miniCircuitsSwitchControlModel.B2SwitchPosition;
            }
            set
            {
                if (_miniCircuitsSwitchControlModel.ManualSwitchEnable)
                {
                    this._miniCircuitsSwitchControlModel.B2SwitchPosition = value;
                    OnPropertyChanged("B2SwitchPosition");
                    OnPropertyChanged("B2SwitchColor");
                }
            }
        }

        public string B2SwitchRightName
        {
            get;
            set;
        }

        public SolidColorBrush C1SwitchColor
        {
            get
            {
                return (1 == C1SwitchPosition) ? new SolidColorBrush(Colors.Orange) : new SolidColorBrush(Colors.LimeGreen);
            }
        }

        public string C1SwitchLeftName
        {
            get;
            set;
        }

        public int C1SwitchPosition
        {
            get
            {
                return this._miniCircuitsSwitchControlModel.C1SwitchPosition;
            }
            set
            {
                if (_miniCircuitsSwitchControlModel.ManualSwitchEnable)
                {
                    this._miniCircuitsSwitchControlModel.C1SwitchPosition = value;
                    OnPropertyChanged("C1SwitchPosition");
                    OnPropertyChanged("C1SwitchColor");
                }
            }
        }

        public string C1SwitchRightName
        {
            get;
            set;
        }

        public SolidColorBrush C2SwitchColor
        {
            get
            {
                return (1 == C2SwitchPosition) ? new SolidColorBrush(Colors.Orange) : new SolidColorBrush(Colors.LimeGreen);
            }
        }

        public string C2SwitchLeftName
        {
            get;
            set;
        }

        public int C2SwitchPosition
        {
            get
            {
                return this._miniCircuitsSwitchControlModel.C2SwitchPosition;
            }
            set
            {
                if (_miniCircuitsSwitchControlModel.ManualSwitchEnable)
                {
                    this._miniCircuitsSwitchControlModel.C2SwitchPosition = value;
                    OnPropertyChanged("C2SwitchPosition");
                    OnPropertyChanged("C2SwitchColor");
                }
            }
        }

        public string C2SwitchRightName
        {
            get;
            set;
        }

        public SolidColorBrush D1SwitchColor
        {
            get
            {
                return (1 == D1SwitchPosition) ? new SolidColorBrush(Colors.Orange) : new SolidColorBrush(Colors.LimeGreen);
            }
        }

        public string D1SwitchLeftName
        {
            get;
            set;
        }

        public int D1SwitchPosition
        {
            get
            {
                return this._miniCircuitsSwitchControlModel.D1SwitchPosition;
            }
            set
            {
                if (_miniCircuitsSwitchControlModel.ManualSwitchEnable)
                {
                    this._miniCircuitsSwitchControlModel.D1SwitchPosition = value;
                    OnPropertyChanged("D1SwitchPosition");
                    OnPropertyChanged("D1SwitchColor");
                }
            }
        }

        public string D1SwitchRightName
        {
            get;
            set;
        }

        public SolidColorBrush D2SwitchColor
        {
            get
            {
                return (1 == D2SwitchPosition) ? new SolidColorBrush(Colors.Orange) : new SolidColorBrush(Colors.LimeGreen);
            }
        }

        public string D2SwitchLeftName
        {
            get;
            set;
        }

        public int D2SwitchPosition
        {
            get
            {
                return this._miniCircuitsSwitchControlModel.D2SwitchPosition;
            }
            set
            {
                if (_miniCircuitsSwitchControlModel.ManualSwitchEnable)
                {
                    this._miniCircuitsSwitchControlModel.D2SwitchPosition = value;
                    OnPropertyChanged("D2SwitchPosition");
                    OnPropertyChanged("D2SwitchColor");
                }
            }
        }

        public string D2SwitchRightName
        {
            get;
            set;
        }

        public bool FirstSwitchBoxAvailable
        {
            get
            {
                return _miniCircuitsSwitchControlModel.FirstSwitchBoxAvailable;
            }
        }

        public bool ManualSwitchEnable
        {
            get
            {
                return _miniCircuitsSwitchControlModel.ManualSwitchEnable;
            }
            set
            {
                _miniCircuitsSwitchControlModel.ManualSwitchEnable = value;
                OnPropertyChanged("ManualSwitchEnable");
            }
        }

        public bool SecondSwitchBoxAvailable
        {
            get
            {
                return _miniCircuitsSwitchControlModel.SecondSwitchBoxAvailable;
            }
        }

        public ICommand SwitchPositionNameChangeCommand
        {
            get
            {
                if (this._switchPositionNameChangeCommand == null)
                    this._switchPositionNameChangeCommand = new RelayCommandWithParam((x) => SwitchPositionNameChange(x));

                return this._switchPositionNameChangeCommand;
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
                myPropInfo = typeof(MiniCircuitsSwitchControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadEpiPositionNames()
        {
            try
            {
                XmlDocument applicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                if (null != applicationDoc)
                {
                    XmlNodeList ndList = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MiniCircuitsSwitchControlView");

                    if (ndList.Count > 0)
                    {
                        string str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "A1LeftName", ref str))
                        {
                            A1SwitchLeftName = str;
                            OnPropertyChanged("A1SwitchLeftName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "A1RightName", ref str))
                        {
                            A1SwitchRightName = str;
                            OnPropertyChanged("A1SwitchRightName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "B1LeftName", ref str))
                        {
                            B1SwitchLeftName = str;
                            OnPropertyChanged("B1SwitchLeftName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "B1RightName", ref str))
                        {
                            B1SwitchRightName = str;
                            OnPropertyChanged("B1SwitchRightName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "C1LeftName", ref str))
                        {
                            C1SwitchLeftName = str;
                            OnPropertyChanged("C1SwitchLeftName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "C1RightName", ref str))
                        {
                            C1SwitchRightName = str;
                            OnPropertyChanged("C1SwitchRightName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "D1LeftName", ref str))
                        {
                            D1SwitchLeftName = str;
                            OnPropertyChanged("D1SwitchLeftName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "D1RightName", ref str))
                        {
                            D1SwitchRightName = str;
                            OnPropertyChanged("D1SwitchRightName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "A2LeftName", ref str))
                        {
                            A2SwitchLeftName = str;
                            OnPropertyChanged("A2SwitchLeftName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "A2RightName", ref str))
                        {
                            A2SwitchRightName = str;
                            OnPropertyChanged("A2SwitchRightName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "B2LeftName", ref str))
                        {
                            B2SwitchLeftName = str;
                            OnPropertyChanged("B2SwitchLeftName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "B2RightName", ref str))
                        {
                            B2SwitchRightName = str;
                            OnPropertyChanged("B2SwitchRightName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "C2LeftName", ref str))
                        {
                            C2SwitchLeftName = str;
                            OnPropertyChanged("C2SwitchLeftName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "C2RightName", ref str))
                        {
                            C2SwitchRightName = str;
                            OnPropertyChanged("C2SwitchRightName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "D2LeftName", ref str))
                        {
                            D2SwitchLeftName = str;
                            OnPropertyChanged("D2SwitchLeftName");
                        }

                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], applicationDoc, "D2RightName", ref str))
                        {
                            D2SwitchRightName = str;
                            OnPropertyChanged("D2SwitchRightName");
                        }
                    }
                }
            }
            catch (Exception e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch Control View Model -> LoadEpiPositionNames. Threw an exception: " + e.ToString());
            }
        }

        public void LoadXMLSettings()
        {
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/MiniCircuitsSwitch");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "manualSwitchEnable", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        ManualSwitchEnable = (1 == tmp);
                    }
                }
                str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "A1pos", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        A1SwitchPosition = tmp;
                    }
                }
                str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "B1pos", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        B1SwitchPosition = tmp;
                    }
                }
                str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "C1pos", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        C1SwitchPosition = tmp;
                    }
                }
                str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "D1pos", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        D1SwitchPosition = tmp;
                    }
                }
                str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "A2pos", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        A2SwitchPosition = tmp;
                    }
                }
                str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "B2pos", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        B2SwitchPosition = tmp;
                    }
                }
                str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "C2pos", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        C2SwitchPosition = tmp;
                    }
                }
                str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "D2pos", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        D2SwitchPosition = tmp;
                    }
                }
            }
            OnPropertyChanged("SecondSwitchBoxAvailable");
            LoadEpiPositionNames();

            //The active camera might've changed with the modality, set the PMT switch box to the right position
            ResourceManagerCS.Instance.UpdatePMTSwitchBox();

            if (FirstSwitchBoxAvailable)
            {
                OnPropertyChanged("A1SwitchPosition");
                OnPropertyChanged("A1SwitchColor");
                OnPropertyChanged("B1SwitchPosition");
                OnPropertyChanged("B1SwitchColor");
                OnPropertyChanged("C1SwitchPosition");
                OnPropertyChanged("C1SwitchColor");
                OnPropertyChanged("D1SwitchPosition");
                OnPropertyChanged("D1SwitchColor");
            }
            if (SecondSwitchBoxAvailable)
            {
                OnPropertyChanged("A2SwitchPosition");
                OnPropertyChanged("A2SwitchColor");
                OnPropertyChanged("B2SwitchPosition");
                OnPropertyChanged("B2SwitchColor");
                OnPropertyChanged("C2SwitchPosition");
                OnPropertyChanged("C2SwitchColor");
                OnPropertyChanged("D2SwitchPosition");
                OnPropertyChanged("D2SwitchColor");
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
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/MiniCircuitsSwitch");

            if (ndList.Count <= 0)
            {
                if (ndList.Count <= 0)
                {
                    XmlManager.CreateXmlNode(experimentFile, "MiniCircuitsSwitch");
                    ndList = experimentFile.SelectNodes("/ThorImageExperiment/MiniCircuitsSwitch");
                }
            }
            XmlManager.SetAttribute(ndList[0], experimentFile, "manualSwitchEnable", (ManualSwitchEnable) ? "1" : "0");
            XmlManager.SetAttribute(ndList[0], experimentFile, "A1pos", A1SwitchPosition.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "B1pos", B1SwitchPosition.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "C1pos", C1SwitchPosition.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "D1pos", D1SwitchPosition.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "A2pos", A2SwitchPosition.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "B2pos", B2SwitchPosition.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "C2pos", C2SwitchPosition.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "D2pos", D2SwitchPosition.ToString());
        }

        private void SwitchPositionNameChange(object button)
        {
            string buttonName = Convert.ToString(button);
            try
            {
                XmlDocument applicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                if (null != applicationDoc)
                {
                    XmlNodeList ndList = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MiniCircuitsSwitchControlView");

                    if (ndList.Count > 0)
                    {
                        string str = string.Empty;
                        PositionNameChangeWin dlg = new PositionNameChangeWin();

                        XmlManager.GetAttribute(ndList[0], applicationDoc, buttonName + "Name", ref str);

                        dlg.SwitchPositionName = str;
                        if (false == dlg.ShowDialog())
                        {
                            return;
                        }

                        // After name window is closed, read the last entered text
                        str = dlg.SwitchPositionName;

                        switch (buttonName)
                        {
                            case "A1Left":
                                A1SwitchLeftName = str;
                                OnPropertyChanged("A1SwitchLeftName");
                                break;
                            case "A1Right":
                                A1SwitchRightName = str;
                                OnPropertyChanged("A1SwitchRightName");
                                break;
                            case "B1Left":
                                B1SwitchLeftName = str;
                                OnPropertyChanged("B1SwitchLeftName");
                                break;
                            case "B1Right":
                                B1SwitchRightName = str;
                                OnPropertyChanged("B1SwitchRightName");
                                break;
                            case "C1Left":
                                C1SwitchLeftName = str;
                                OnPropertyChanged("C1SwitchLeftName");
                                break;
                            case "C1Right":
                                C1SwitchRightName = str;
                                OnPropertyChanged("C1SwitchRightName");
                                break;
                            case "D1Left":
                                D1SwitchLeftName = str;
                                OnPropertyChanged("D1SwitchLeftName");
                                break;
                            case "D1Right":
                                D1SwitchRightName = str;
                                OnPropertyChanged("D1SwitchRightName");
                                break;
                            case "A2Left":
                                A2SwitchLeftName = str;
                                OnPropertyChanged("A2SwitchLeftName");
                                break;
                            case "A2Right":
                                A2SwitchRightName = str;
                                OnPropertyChanged("A2SwitchRightName");
                                break;
                            case "B2Left":
                                B2SwitchLeftName = str;
                                OnPropertyChanged("B2SwitchLeftName");
                                break;
                            case "B2Right":
                                B2SwitchRightName = str;
                                OnPropertyChanged("B2SwitchRightName");
                                break;
                            case "C2Left":
                                C2SwitchLeftName = str;
                                OnPropertyChanged("C2SwitchLeftName");
                                break;
                            case "C2Right":
                                C2SwitchRightName = str;
                                OnPropertyChanged("C2SwitchRightName");
                                break;
                            case "D2Left":
                                D2SwitchLeftName = str;
                                OnPropertyChanged("D2SwitchLeftName");
                                break;
                            case "D2Right":
                                D2SwitchRightName = str;
                                OnPropertyChanged("D2SwitchRightName");
                                break;
                        }

                        XmlManager.SetAttribute(ndList[0], applicationDoc, buttonName + "Name", str);
                        MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                    }
                }
            }
            catch (Exception e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch Control View Model -> SwitchPositionNameChange. Threw an exception: " + e.ToString());
            }
        }

        #endregion Methods
    }
}