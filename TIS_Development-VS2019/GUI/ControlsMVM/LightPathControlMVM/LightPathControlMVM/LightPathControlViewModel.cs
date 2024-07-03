namespace LightPathControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Reflection;
    using System.Windows;
    using System.Xml;

    using LightPathControl.Model;

    using ThorLogging;

    using ThorSharedTypes;

    public class LightPathControlViewModel : VMBase, IMVM
    {
        #region Fields

        private readonly LightPathControlModel _lightPathControlModel;

        static Dictionary<string, LightPathBtns> LightPathCmds = new Dictionary<string, LightPathBtns>()
        {
        {"GG_IN", LightPathBtns.GG_IN},
        {"GG_OUT", LightPathBtns.GG_OUT},
        {"GG_Flip", LightPathBtns.GG_Flip},
        {"GR_IN", LightPathBtns.GR_IN},
        {"GR_OUT", LightPathBtns.GR_OUT},
        {"GR_Flip", LightPathBtns.GR_Flip},
        {"CAM_PMT", LightPathBtns.CAM_PMT},
        {"CAM_CAM", LightPathBtns.CAM_CAM},
        {"CAM_Flip", LightPathBtns.CAM_Flip},
        {"Left", LightPathBtns.Left},
        {"Center", LightPathBtns.Center},
        {"Right", LightPathBtns.Right},
        {"NDD_IN", LightPathBtns.NDD_IN},
        {"NDD_OUT", LightPathBtns.NDD_OUT},
        };

        private string _camLightPathKey;
        private string _camLightPathModifier;
        private string _ggLightPathKey;
        private string _ggLightPathModifier;
        private string _grLightPathKey;
        private string _grLightPathModifier;
        private RelayCommandWithParam _lightPathSwitch;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();

        #endregion Fields

        #region Constructors

        public LightPathControlViewModel()
        {
            this._lightPathControlModel = new LightPathControlModel();
        }

        #endregion Constructors

        #region Enumerations

        enum LightPathBtns
        {
            GG_IN = 0,
            GG_OUT = 1,
            GG_Flip = 2,
            GR_IN = 3,
            GR_OUT = 4,
            GR_Flip = 5,
            CAM_PMT = 6,
            CAM_CAM = 7,
            CAM_Flip = 8,
            Left = 9,
            Center = 10,
            Right = 11,
            NDD_IN = 12,
            NDD_OUT = 13
        }

        #endregion Enumerations

        #region Properties

        public Visibility CameraLightPathVisibility
        {
            get
            {
                return ResourceManagerCS.GetVisibility("/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView", "CameraLightPathVisibility");
            }
        }

        public string CamLightPathKey
        {
            get { return _camLightPathKey; }
            set { _camLightPathKey = value; OnPropertyChanged("CamLightPathKey"); }
        }

        public string CamLightPathModifier
        {
            get { return _camLightPathModifier; }
            set { _camLightPathModifier = value; OnPropertyChanged("CamLightPathModifier"); }
        }

        public string DisplayOffNDD
        {
            get
            {
                if (1 == PositionNDD)
                {
                    return "./Icons/ToggleOff_up.png";
                }
                else
                {
                    return "./Icons/ToggleOff_down.png";
                }
            }
        }

        public string DisplayOnNDD
        {
            get
            {
                if (1 == PositionNDD)
                {
                    return "./Icons/ToggleOn_down.png";
                }
                else
                {
                    return "./Icons/ToggleOn_up.png";
                }
            }
        }

        public string GGLightPathKey
        {
            get { return _ggLightPathKey; }
            set { _ggLightPathKey = value; OnPropertyChanged("GGLightPathKey"); }
        }

        public string GGLightPathModifier
        {
            get { return _ggLightPathModifier; }
            set { _ggLightPathModifier = value; OnPropertyChanged("GGLightPathModifier"); }
        }

        public Visibility GGLightPathVisibility
        {
            get
            {
                return ResourceManagerCS.GetVisibility("/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView", "GGLightPathVisibility");
            }
        }

        public string GRLightPathKey
        {
            get { return _grLightPathKey; }
            set { _grLightPathKey = value; OnPropertyChanged("GRLightPathKey"); }
        }

        public string GRLightPathModifier
        {
            get { return _grLightPathModifier; }
            set { _grLightPathModifier = value; OnPropertyChanged("GRLightPathModifier"); }
        }

        public Visibility GRLightPathVisibility
        {
            get
            {
                return ResourceManagerCS.GetVisibility("/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView", "GRLightPathVisibility");
            }
        }

        public int InvertedLightPathPos
        {
            get
            {
                return _lightPathControlModel.InvertedLightPathPos;
            }
            set
            {
                _lightPathControlModel.InvertedLightPathPos = value;
            }
        }

        public string InvertedLpCenterDisplay
        {
            get
            {
                if (InvertedLpCenterEnable)
                {
                    return "./Icons/ToggleOn_down.png";
                }
                else
                {
                    return "./Icons/ToggleOn_up.png";
                }
            }
        }

        public bool InvertedLpCenterEnable
        {
            get
            {
                return _lightPathControlModel.InvertedLpCenterEnable;
            }
            set
            {
                _lightPathControlModel.InvertedLpCenterEnable = value;
                OnPropertyChange("InvertedLpLeftDisplay");
                OnPropertyChange("InvertedLpCenterDisplay");
                OnPropertyChange("InvertedLpRightDisplay");
            }
        }

        public string InvertedLpLeftDisplay
        {
            get
            {
                if (this.InvertedLpLeftEnable)
                {
                    return "./Icons/ToggleOn_down.png";
                }
                else
                {
                    return "./Icons/ToggleOn_up.png";
                }
            }
        }

        public bool InvertedLpLeftEnable
        {
            get
            {
                return _lightPathControlModel.InvertedLpLeftEnable;
            }
            set
            {
                _lightPathControlModel.InvertedLpLeftEnable = value;
                OnPropertyChange("InvertedLpLeftDisplay");
                OnPropertyChange("InvertedLpCenterDisplay");
                OnPropertyChange("InvertedLpRightDisplay");
            }
        }

        public string InvertedLpRightDisplay
        {
            get
            {
                if (this.InvertedLpRightEnable)
                {
                    return "./Icons/ToggleOn_down.png";
                }
                else
                {
                    return "./Icons/ToggleOn_up.png";
                }
            }
        }

        public bool InvertedLpRightEnable
        {
            get
            {
                return _lightPathControlModel.InvertedLpRightEnable;
            }
            set
            {
                _lightPathControlModel.InvertedLpRightEnable = value;
                OnPropertyChange("InvertedLpLeftDisplay");
                OnPropertyChange("InvertedLpCenterDisplay");
                OnPropertyChange("InvertedLpRightDisplay");
            }
        }

        public Visibility IsInvertedScope
        {
            get
            {
                return ((int)ScopeType.INVERTED == _lightPathControlModel.LightPathScopeType) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public bool IsNDDAvailable
        {
            get
            {
                Visibility NDDVisibility = ResourceManagerCS.GetVisibility("/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView", "NDDLightPathVisibility");
                return (Visibility.Visible == NDDVisibility) ? _lightPathControlModel.IsNDDAvailable : false;
            }
        }

        public Visibility IsTabletModeEnabled
        {
            get
            {
                return (ResourceManagerCS.Instance.TabletModeEnabled) ? Visibility.Collapsed : Visibility.Visible;
            }
        }

        public Visibility IsUprightScope
        {
            get
            {
                return ((int)ScopeType.UPRIGHT == _lightPathControlModel.LightPathScopeType) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public string LabelNDD
        {
            get
            {
                return ResourceManagerCS.GetValueString("/ApplicationSettings/DisplayOptions/CaptureSetup/InvertedLightPathView", "NddNameChange");
            }
        }

        public string LightPathCamDisplayOff
        {
            get
            {
                if (1 == this.LightPathCamEnable)
                {
                    return "./Icons/ToggleCam_down.png";
                }
                else
                {
                    return "./Icons/ToggleCam_up.png";
                }
            }
        }

        public string LightPathCamDisplayOn
        {
            get
            {
                if (1 == this.LightPathCamEnable)
                {
                    return "./Icons/TogglePMT_up.png";
                }
                else
                {
                    return "./Icons/TogglePMT_down.png";
                }
            }
        }

        public int LightPathCamEnable
        {
            get
            {
                return _lightPathControlModel.LightPathCamEnable;
            }
            set
            {
                _lightPathControlModel.LightPathCamEnable = value;
                OnPropertyChanged("LightPathCamEnable");
                OnPropertyChanged("LightPathCamDisplayOn");
                OnPropertyChanged("LightPathCamDisplayOff");
            }
        }

        public string LightPathGGDisplayOff
        {
            get
            {
                if (1 == this.LightPathGGEnable)
                {
                    return "./Icons/ToggleOff_up.png";
                }
                else
                {
                    return "./Icons/ToggleOff_down.png";
                }
            }
        }

        public string LightPathGGDisplayOn
        {
            get
            {
                if (1 == this.LightPathGGEnable)
                {
                    return "./Icons/ToggleOn_down.png";
                }
                else
                {
                    return "./Icons/ToggleOn_up.png";
                }
            }
        }

        public int LightPathGGEnable
        {
            get
            {
                return _lightPathControlModel.LightPathGGEnable;
            }
            set
            {
                _lightPathControlModel.LightPathGGEnable = value;
                OnPropertyChanged("LightPathGGEnable");
                OnPropertyChanged("LightPathGGDisplayOn");
                OnPropertyChanged("LightPathGGDisplayOff");
            }
        }

        public string LightPathGRDisplayOff
        {
            get
            {
                if (1 == this.LightPathGREnable)
                {
                    return "./Icons/ToggleOff_up.png";
                }
                else
                {
                    return "./Icons/ToggleOff_down.png";
                }
            }
        }

        public string LightPathGRDisplayOn
        {
            get
            {
                if (1 == this.LightPathGREnable)
                {
                    return "./Icons/ToggleOn_down.png";
                }
                else
                {
                    return "./Icons/ToggleOn_up.png";
                }
            }
        }

        public int LightPathGREnable
        {
            get
            {
                return _lightPathControlModel.LightPathGREnable;
            }
            set
            {
                _lightPathControlModel.LightPathGREnable = value;
                OnPropertyChanged("LightPathGREnable");
                OnPropertyChanged("LightPathGRDisplayOn");
                OnPropertyChanged("LightPathGRDisplayOff");
            }
        }

        public string LightPathLabel_1
        {
            get
            {
                string ret;
                switch (_lightPathControlModel.LightPathScopeType)
                {
                    case (int)ScopeType.INVERTED:
                        ret = ResourceManagerCS.GetValueString("/ApplicationSettings/DisplayOptions/CaptureSetup/InvertedLightPathView", "LeftNameChange");
                        break;
                    case (int)ScopeType.UPRIGHT:
                    default:
                        ret = "Galvo/Galvo";
                        break;
                }
                return ret;
            }
        }

        public string LightPathLabel_2
        {
            get
            {
                string ret;
                switch (_lightPathControlModel.LightPathScopeType)
                {
                    case (int)ScopeType.INVERTED:
                        ret = ResourceManagerCS.GetValueString("/ApplicationSettings/DisplayOptions/CaptureSetup/InvertedLightPathView", "CenterNameChange");
                        break;
                    case (int)ScopeType.UPRIGHT:
                    default:
                        ret = "Galvo/Resonance";
                        break;
                }
                return ret;
            }
        }

        public string LightPathLabel_3
        {
            get
            {
                string ret;
                switch (_lightPathControlModel.LightPathScopeType)
                {
                    case (int)ScopeType.INVERTED:
                        ret = ResourceManagerCS.GetValueString("/ApplicationSettings/DisplayOptions/CaptureSetup/InvertedLightPathView", "RightNameChange");
                        break;
                    case (int)ScopeType.UPRIGHT:
                    default:
                        ret = "Camera";
                        break;
                }
                return ret;
            }
        }

        public int LightPathScopeType
        {
            get
            {
                return _lightPathControlModel.LightPathScopeType;
            }
        }

        public RelayCommandWithParam LightPathSwitch
        {
            get
            {
                if (this._lightPathSwitch == null)
                    this._lightPathSwitch = new RelayCommandWithParam(LightPathSwitchVM);

                return this._lightPathSwitch;
            }
        }

        public Thickness MarginCenterInvertedButton
        {
            get
            {
                if (IsNDDAvailable)
                {
                    return new Thickness(20, 0, 20, 10);
                }
                else
                {
                    return new Thickness(45, 0, 45, 10);
                }
            }
        }

        public Thickness MarginLeftInvertedButton
        {
            get
            {
                if (IsNDDAvailable)
                {
                    return new Thickness(30, 0, 20, 10);
                }
                else
                {
                    return new Thickness(45, 0, 45, 10);
                }
            }
        }

        public Thickness MarginRightInvertedButton
        {
            get
            {
                if (IsNDDAvailable)
                {
                    return new Thickness(20, 0, 20, 10);
                }
                else
                {
                    return new Thickness(45, 0, 45, 10);
                }
            }
        }

        public int PositionNDD
        {
            get
            {
                return _lightPathControlModel.PositionNDD;
            }
            set
            {
                _lightPathControlModel.PositionNDD = value;
                OnPropertyChanged("PositionNDD");
                OnPropertyChanged("DisplayOnNDD");
                OnPropertyChanged("DisplayOffNDD");
            }
        }

        public Visibility SecondaryGGVisibilityImaging
        {
            get
            {
                return _lightPathControlModel.SecondaryGGAvailableImaging ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public Visibility SecondaryGGVisibilityStim
        {
            get
            {
                return _lightPathControlModel.SecondaryGGAvailableStim ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public int SelectedImagingGG
        {
            get => _lightPathControlModel.SelectedImagingGG;
            set
            {
                _lightPathControlModel.SelectedImagingGG = value;
                OnPropertyChanged("SelectedImagingGG");
            }
        }

        public int SelectedStimGG
        {
            get => _lightPathControlModel.SelectedStimGG;
            set
            {
                _lightPathControlModel.SelectedStimGG = value;
                OnPropertyChanged("SelectedStimGG");
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
                myPropInfo = typeof(LightPathControlViewModel).GetProperty(propertyName);
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

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/LightPath");

            string str = string.Empty;
            int iTmp;

            if (ndList.Count > 0)
            {

                if (XmlManager.GetAttribute(ndList[0], doc, "GalvoGalvo", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    LightPathGGEnable = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "GalvoResonance", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    LightPathGREnable = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "Camera", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    LightPathCamEnable = iTmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "InvertedLightPathPos", ref str) && "-1" != str)
                {
                    switch (str)
                    {
                        case "0":
                            InvertedLpLeftEnable = true;
                            break;
                        case "1":
                            InvertedLpCenterEnable = true;
                            break;
                        case "2":
                            InvertedLpRightEnable = true;
                            break;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "NDD", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    PositionNDD = iTmp;
                }
            }

            ndList = doc.SelectNodes("/ThorImageExperiment/LSM");
            if (ndList.Count > 0)
            {
                if (XmlManager.GetAttribute(ndList[0], doc, "selectedImagingGG", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    SelectedImagingGG = iTmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "selectedStimGG", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    SelectedStimGG = iTmp;
                }
            }
            OnPropertyChanged("SecondaryGGVisibilityImaging");
            OnPropertyChanged("SecondaryGGVisibilityStim");
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
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/LightPath");
            if (ndList.Count <= 0)
            {
                XmlManager.CreateXmlNode(experimentFile, "LightPath");
                ndList = experimentFile.SelectNodes("/ThorImageExperiment/LightPath");
            }

            XmlManager.SetAttribute(ndList[0], experimentFile, "GalvoGalvo", this.LightPathGGEnable.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "GalvoResonance", this.LightPathGREnable.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "Camera", this.LightPathCamEnable.ToString());
            if (-1 == this.InvertedLightPathPos)
            {
                XmlManager.SetAttribute(ndList[0], experimentFile, "InvertedLightPathPos", "-1");
            }
            else
            {
                XmlManager.SetAttribute(ndList[0], experimentFile, "InvertedLightPathPos", this.InvertedLightPathPos.ToString());
            }
            XmlManager.SetAttribute(ndList[0], experimentFile, "NDD", this.PositionNDD.ToString());

            ndList = experimentFile.SelectNodes("/ThorImageExperiment/LSM");
            if (ndList.Count <= 0)
            {
                XmlManager.CreateXmlNode(experimentFile, "LSM");
                ndList = experimentFile.SelectNodes("/ThorImageExperiment/LSM");
            }
            XmlManager.SetAttribute(ndList[0], experimentFile, "selectedImagingGG", SelectedImagingGG.ToString());
            XmlManager.SetAttribute(ndList[0], experimentFile, "selectedStimGG", SelectedStimGG.ToString());
        }

        private void LightPathSwitchVM(object param)
        {
            string p = (string)param;
            int t;
            switch (LightPathCmds[p])
            {
                case LightPathBtns.GG_IN:
                    LightPathGGEnable = 1;
                    break;
                case LightPathBtns.GG_OUT:
                    LightPathGGEnable = 0;
                    break;
                case LightPathBtns.GG_Flip:
                    t = LightPathGGEnable;
                    t = (t + 1) % 2;
                    LightPathGGEnable = t;
                    break;
                case LightPathBtns.GR_IN:
                    LightPathGREnable = 1;
                    break;
                case LightPathBtns.GR_OUT:
                    LightPathGREnable = 0;
                    break;
                case LightPathBtns.GR_Flip:
                    t = LightPathGREnable;
                    t = (t + 1) % 2;
                    LightPathGREnable = t;
                    break;
                case LightPathBtns.CAM_PMT:
                    LightPathCamEnable = 0;
                    break;
                case LightPathBtns.CAM_CAM:
                    LightPathCamEnable = 1;
                    break;
                case LightPathBtns.CAM_Flip:
                    t = LightPathCamEnable;
                    t = (t + 1) % 2;
                    LightPathCamEnable = t;
                    break;
                case LightPathBtns.Left:
                    InvertedLpLeftEnable = true;
                    InvertedLpCenterEnable = false;
                    InvertedLpRightEnable = false;
                    break;
                case LightPathBtns.Center:
                    InvertedLpCenterEnable = true;
                    InvertedLpRightEnable = false;
                    InvertedLpLeftEnable = false;
                    break;
                case LightPathBtns.Right:
                    InvertedLpRightEnable = true;
                    InvertedLpLeftEnable = false;
                    InvertedLpCenterEnable = false;
                    break;
                case LightPathBtns.NDD_IN:
                    PositionNDD = 1;
                    break;
                case LightPathBtns.NDD_OUT:
                    PositionNDD = 0;
                    break;
            }
        }

        #endregion Methods
    }
}