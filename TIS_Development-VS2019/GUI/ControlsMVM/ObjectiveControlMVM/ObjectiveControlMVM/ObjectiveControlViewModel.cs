namespace ObjectiveControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;

    using ObjectiveControl.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public class ObjectiveControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly ObjectiveControlModel _ObjectiveControlModel;

        private double[] _focalLengths = { 200.0, 100.0, 50.0, 200.0 }; //f1, f2, f3, f4
        private double _NA;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();

        #endregion Fields

        #region Constructors

        public ObjectiveControlViewModel()
        {
            this._ObjectiveControlModel = new ObjectiveControlModel();

            EpiTurretPosNames = new ObservableCollection<StringPC>();
            const int MAX_TURRET_POS = 6;
            for (int i = 0; i < MAX_TURRET_POS; i++)
            {
                EpiTurretPosNames.Add(new StringPC());
            }
        }

        #endregion Constructors

        #region Properties

        public string BeamExp2Text
        {
            get
            {
                return _ObjectiveControlModel.BeamExp2.ToString() + " X";
            }
        }

        public Visibility BeamExp2Vis
        {
            get;
            set;
        }

        public string BeamExpText
        {
            get
            {
                return _ObjectiveControlModel.BeamExp.ToString() + " X";
            }
        }

        public Visibility BeamExpVis
        {
            get;
            set;
        }

        public double CollisionStatus
        {
            get
            {
                return _ObjectiveControlModel.CollisionStatus;
            }
        }

        public string EpiTurretPosName
        {
            get
            {
                //Check if the current position is within a valid position
                if (_ObjectiveControlModel.EpiTurretPos > 0 && _ObjectiveControlModel.EpiTurretPos < 7)
                {
                    return EpiTurretPosNames[_ObjectiveControlModel.EpiTurretPos - 1].Value;
                }
                return _ObjectiveControlModel.EpiTurretPos.ToString();
            }
        }

        public ObservableCollection<StringPC> EpiTurretPosNames
        {
            get;
            set;
        }

        public string EpiTurretPosText
        {
            get
            {
                return _ObjectiveControlModel.EpiTurretPos.ToString();
            }
        }

        public Visibility EpiTurretVis
        {
            get;
            set;
        }

        public double[] FocalLengths
        {
            get { return _focalLengths; }
        }

        public string FramesPerSecondText
        {
            get
            {
                //formating
                string formating = "";
                double framesPerSecond = 0.0;
                if ((bool)MVMManager.Instance["ScanControlViewModel", "UseFastestSettingForFlybackCycles", (object)false])
                {
                    formating = "{0} fps";
                }
                else
                {
                    formating = "[{0}] fps";
                }
                framesPerSecond = (double)MVMManager.Instance["CaptureSetupViewModel", "FramesPerSecond", (object)0.0];
                return String.Format(formating + "\n{1} ms/f",
                                     framesPerSecond.ToString("#0.0"),
                                     (1000.0 / framesPerSecond).ToString("#0.0"));
            }
        }

        public bool IsObjectiveSwitching
        {
            get
            {
                return this._ObjectiveControlModel.IsObjectiveSwitching;
            }
        }

        public bool MagComboBoxEnabled
        {
            get
            {
                return this._ObjectiveControlModel.MagComboBoxEnabled;
            }
            set
            {
                this._ObjectiveControlModel.MagComboBoxEnabled = value;
                OnPropertyChanged("MagComboBoxEnabled");
            }
        }

        public double NA
        {
            get
            {
                return this._NA;
            }
            set
            {
                this._NA = value;
                OnPropertyChanged("NA");
            }
        }

        public int ObjectiveChangerStatus
        {
            get
            {
                return this._ObjectiveControlModel.ObjectiveChangerStatus;
            }
            set
            {
                this._ObjectiveControlModel.ObjectiveChangerStatus = value;
                OnPropertyChanged("ObjectiveChangerStatus");
            }
        }

        public ObservableCollection<string> ObjectiveNames
        {
            get
            {
                return this._ObjectiveControlModel.ObjectiveNames;
            }
            set
            {
                this._ObjectiveControlModel.ObjectiveNames = value;
            }
        }

        //public string TurretBeamExpansion
        //{
        //    get
        //    {
        //        return this._ObjectiveControlModel.TurretBeamExpansion;
        //    }
        //}
        public double TurretMagnification
        {
            get
            {
                return this._ObjectiveControlModel.TurretMagnification;
            }
        }

        public string TurretName
        {
            get
            {
                return this._ObjectiveControlModel.TurretName;
            }
        }

        public int TurretPosition
        {
            get
            {
                return this._ObjectiveControlModel.TurretPosition;
            }
            set
            {
                this._ObjectiveControlModel.TurretPosition = value;
                ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMFieldSizeXUM");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("FieldSizeWidthUM");
                ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMFieldSizeYUM");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("FieldSizeHeightUM");
                ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMFieldSizeXMM");
                ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMFieldSizeYMM");
                ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMUMPerPixel");

                ((IMVM)MVMManager.Instance["PinholeControlViewModel", this]).OnPropertyChange("PinholePosition");
                //To update the pinhole position combo box, set it to -1 then set it back to the position it was suposed to be in.
                //this is for GUI purposes only. Otherwise combo box thinks it is fine in the current position and shouln't update
                int pinholePos = (int)MVMManager.Instance["PinholeControlViewModel", "PinholePosition", (object)0];
                MVMManager.Instance["PinholeControlViewModel", "PinholePosition"] = -1;
                ((IMVM)MVMManager.Instance["PinholeControlViewModel", this]).OnPropertyChange("ComboBoxItemsList");
                MVMManager.Instance["PinholeControlViewModel", "PinholePosition"] = pinholePos;
                ((IMVM)MVMManager.Instance["PinholeControlViewModel", this]).OnPropertyChange("PinholeADUs");
                ((IMVM)MVMManager.Instance["PinholeControlViewModel", this]).OnPropertyChange("PinholeADUsString");
                //OnPropertyChanged("TurretBeamExpansion");
                ((IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("ZSectionThickness");
                OnPropertyChanged("BeamExpText");
                OnPropertyChanged("BeamExp2Text");
                OnPropertyChanged("EpiTurretPosName");
                ((IMVM)MVMManager.Instance["CameraControlViewModel", this]).OnPropertyChange("CameraRegionHeightUM");
                ((IMVM)MVMManager.Instance["CameraControlViewModel", this]).OnPropertyChange("CameraRegionWidthUM");
                ((IMVM)MVMManager.Instance["CameraControlViewModel", this]).OnPropertyChange("CamPixelSizeUM");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");

                _ObjectiveControlModel.GetBeamExpansion(value); //update info from hardware settings
                NA = _ObjectiveControlModel.NumericalAperture;
                OnPropertyChanged("TurretPosition");

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

        public void FlagDisconnectedDevices()
        {
            string str = string.Empty;
            string dllName = string.Empty;
            XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/Devices/Turret");
            if (0 < ndList.Count)
            {
                for (int i = 0; i < ndList.Count; i++)
                {
                    str = string.Empty;
                    if (XmlManager.GetAttribute(ndList[i], hardwareDoc, "active", ref str))
                    {
                        if ("1" == str)
                        {
                            if (XmlManager.GetAttribute(ndList[i], hardwareDoc, "dllName", ref dllName))
                            {
                                this._ObjectiveControlModel.ObjectveChangerIsDisconnected = "Disconnected" == dllName;
                            }
                        }
                    }
                }
            }
            ndList = hardwareDoc.SelectNodes("/HardwareSettings/Devices/ZStage");
            if (0 < ndList.Count)
            {
                for (int i = 0; i < ndList.Count; i++)
                {
                    str = string.Empty;
                    dllName = string.Empty;
                    if (XmlManager.GetAttribute(ndList[i], hardwareDoc, "active", ref str))
                    {
                        if ("1" == str)
                        {
                            if (XmlManager.GetAttribute(ndList[i], hardwareDoc, "dllName", ref dllName))
                            {
                                this._ObjectiveControlModel.ZStageIsDisconnected = "Disconnected" == dllName;
                            }
                        }
                    }
                }
            }
            ndList = hardwareDoc.SelectNodes("/HardwareSettings/Devices/ZStage2");
            if (0 < ndList.Count)
            {
                for (int i = 0; i < ndList.Count; i++)
                {
                    str = string.Empty;
                    dllName = string.Empty;
                    if (XmlManager.GetAttribute(ndList[i], hardwareDoc, "active", ref str))
                    {
                        if ("1" == str)
                        {
                            if (XmlManager.GetAttribute(ndList[i], hardwareDoc, "dllName", ref dllName))
                            {
                                this._ObjectiveControlModel.ZStage2IsDisconnected = "Disconnected" == dllName;
                            }
                        }
                    }
                }
            }
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(ObjectiveControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadApplicationSettings()
        {
            //load Application Settings
            XmlDocument ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BeamExpView");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "Visibility", ref str))
                {
                    BeamExpVis = (str.Equals("Visible")) ? Visibility.Visible : Visibility.Collapsed;
                }
            }

            ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BeamExp2View");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "Visibility", ref str))
                {
                    BeamExp2Vis = (str.Equals("Visible")) ? Visibility.Visible : Visibility.Collapsed;
                }
            }

            ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/EpiTurretView");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], ApplicationDoc, "Visibility", ref str))
                {
                    EpiTurretVis = (str.Equals("Visible")) ? Visibility.Visible : Visibility.Collapsed;
                }
                XmlDocument HardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
                XmlNodeList nl = HardwareDoc.SelectNodes("/HardwareSettings/Devices/EpiTurret");
                if (nl.Count > 0)
                {
                    for (int i = 0; i < nl.Count; i++)
                    {
                        string a = string.Empty;
                        if (XmlManager.GetAttribute(nl[i], HardwareDoc, "active", ref a))
                        {
                            if (a.CompareTo("1") == 0)
                            {
                                a = string.Empty;
                                if (XmlManager.GetAttribute(nl[i], HardwareDoc, "dllName", ref a))
                                {
                                    if (a.CompareTo("Disconnected") == 0)
                                    {
                                        EpiTurretVis = Visibility.Collapsed;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    EpiTurretVis = Visibility.Collapsed;
                }
                for (int i = 0; i < EpiTurretPosNames.Count; i++)
                {
                    str = string.Empty;
                    if (XmlManager.GetAttribute(ndList[0], ApplicationDoc, string.Format("EpiTurretPosName{0}", i + 1), ref str))
                    {
                        EpiTurretPosNames[i].Value = str;
                    }
                }
            }
            else
            {
                XmlNode n = ApplicationDoc.CreateNode(XmlNodeType.Element, "EpiTurretView", null);
                XmlAttribute a = ApplicationDoc.CreateAttribute("Visibility");
                a.Value = "Collapsed";
                XmlAttribute name1 = ApplicationDoc.CreateAttribute("EpiTurretPosName1");
                name1.Value = "1";
                XmlAttribute name2 = ApplicationDoc.CreateAttribute("EpiTurretPosName2");
                name2.Value = "2";
                XmlAttribute name3 = ApplicationDoc.CreateAttribute("EpiTurretPosName3");
                name3.Value = "3";
                XmlAttribute name4 = ApplicationDoc.CreateAttribute("EpiTurretPosName4");
                name4.Value = "4";
                XmlAttribute name5 = ApplicationDoc.CreateAttribute("EpiTurretPosName5");
                name5.Value = "5";
                XmlAttribute name6 = ApplicationDoc.CreateAttribute("EpiTurretPosName6");
                name6.Value = "6";
                n.Attributes.Append(a);
                n.Attributes.Append(name1);
                n.Attributes.Append(name2);
                n.Attributes.Append(name3);
                n.Attributes.Append(name4);
                n.Attributes.Append(name5);
                n.Attributes.Append(name6);
                ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup");
                ndList[0].AppendChild(n);
                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                EpiTurretVis = Visibility.Collapsed;
            }
            OnPropertyChanged("BeamExpVis");
            OnPropertyChanged("BeamExp2Vis");
            OnPropertyChanged("EpiTurretVis");
        }

        public void LoadHardwareSettings()
        {
            string str = string.Empty;
            double dVal;
            XmlDocument hwSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndObj = hwSettings.SelectNodes("/HardwareSettings/Objectives");
            if (0 < ndObj.Count)
            {
                if (!XmlManager.GetAttribute(ndObj[0], hwSettings, "f1MM", ref str))
                {
                    //create default if not already exist
                    MVMManager.Instance.ReloadSettings(SettingsFileType.HARDWARE_SETTINGS);
                    hwSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
                    ndObj = hwSettings.SelectNodes("/HardwareSettings/Objectives");
                    for (int i = 0; i < _focalLengths.Length; i++)
                    {
                        XmlManager.SetAttribute(ndObj[0], hwSettings, "f" + (i + 1).ToString() + "MM", _focalLengths[i].ToString());
                    }
                    MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
                }
                else
                {
                    for (int i = 0; i < _focalLengths.Length; i++)
                    {
                        _focalLengths[i] = (XmlManager.GetAttribute(ndObj[0], hwSettings, "f" + (i + 1).ToString() + "MM", ref str) && Double.TryParse(str, out dVal)) ? dVal : _focalLengths[i];
                    }
                }
            }
        }

        public void LoadLastSavedMag()
        {
            XmlDocument hardwareSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndList = hardwareSettings.SelectNodes("/HardwareSettings/Objectives");
            string magName = string.Empty;
            string na = string.Empty;
            if (ndList.Count > 0)
            {
                if (XmlManager.GetAttribute(ndList[0], hardwareSettings, "lastSavedMagnification", ref magName))
                {
                    if (ObjectiveNames.Contains(magName))
                    {
                        this._ObjectiveControlModel.SetInitialTurretPosition(ObjectiveNames.IndexOf(magName));
                        int tempIndex = ObjectiveNames.IndexOf(magName);
                        NA = _ObjectiveControlModel.NumericalAperture;

                        return;
                    }
                }
                //Didn't find last Magnification. Set initialMagnification to the same as the one in Active.xml
                XmlDocument active = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
                ndList = active.SelectNodes("/ThorImageExperiment/Magnification");
                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], active, "name", ref magName))
                    {
                        if (ObjectiveNames.Contains(magName))
                        {
                            this._ObjectiveControlModel.SetInitialTurretPosition(ObjectiveNames.IndexOf(magName));
                            return;
                        }
                    }
                }
            }
        }

        public void LoadXMLSettings()
        {
            LoadApplicationSettings();

            LoadHardwareSettings();

            //load hardware settings before load exp for mutural items
            XmlNodeList ndListObj = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS].DocumentElement["Objectives"].GetElementsByTagName("Objective");

            bool isDifferent = false;
            if (ObjectiveNames.Count != ndListObj.Count)
            {
                isDifferent = true;
            }
            else
            {
                int i = 0;
                foreach (XmlElement element in ndListObj)
                {
                    string name = element.GetAttribute("name").ToString();
                    if (false == ObjectiveNames[i].ToString().Equals(name))
                    {
                        isDifferent = true;
                        break;
                    }
                    i++;
                }
            }
            if (true == isDifferent)
            {
                ObjectiveNames.Clear();
                foreach (XmlElement element in ndListObj)
                {
                    ObjectiveNames.Add(element.GetAttribute("name").ToString());
                }
            }

            LoadLastSavedMag();

            FlagDisconnectedDevices();

            //load exp
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            //set the active magnifiation for the combo box
            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/Magnification");

            if (ndList.Count > 0)
            {
                int i = 0;
                double dTmp = 0.0;
                string name = string.Empty;
                bool foundTurretPosition = false;

                //if the name of the objective is stored in the file
                //use the name to determine the position of the turret.
                //Otherwise use the magnification value
                if (XmlManager.GetAttribute(ndList[0], doc, "name", ref name))
                {
                    foreach (XmlElement element in ndListObj)
                    {
                        string hwName = element.GetAttribute("name").ToString();

                        if (hwName.Equals(name))
                        {
                            TurretPosition = i;
                            foundTurretPosition = true;
                            break;
                        }
                        i++;
                    }
                }

                if (false == foundTurretPosition)
                {
                    i = 0;
                    foreach (XmlElement element in ndListObj)
                    {

                        if (Double.TryParse(element.GetAttribute("mag").ToString(), NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                        {
                            double hwMag = dTmp;
                            if (Double.TryParse(ndList[0].Attributes["mag"].Value, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp))
                            {
                                double mag = dTmp;
                                const double MAG_PRECISION = .001;
                                if ((hwMag >= mag) && (hwMag < (mag + MAG_PRECISION)))
                                {
                                    TurretPosition = i;
                                    foundTurretPosition = true;
                                    break;
                                }
                                i++;
                            }
                        }
                    }

                    if (false == foundTurretPosition)
                    {
                        TurretPosition = 0;
                    }
                }
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
            //Check if the Objective Changer is moving and wait til it's done.
            System.Threading.Thread.Sleep(10);
            if (IsObjectiveSwitching)
            {
                SpinnerProgress.SpinnerProgressWindow dlg = new SpinnerProgress.SpinnerProgressWindow();
                dlg.ProgressText = "Switching Objectives";
                dlg.WindowStartupLocation = System.Windows.WindowStartupLocation.CenterScreen;
                dlg.Show();
                while (IsObjectiveSwitching)
                {
                    System.Threading.Thread.Sleep(10);
                }
                dlg.Close();
                //Check for collision status
                if (1.0 == CollisionStatus)
                {
                    MessageBox.Show("Collision detected. Make sure the path between both objectives is clear");
                }
            }

            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/Magnification");

            if (ndList.Count > 0)
            {
                ndList[0].Attributes["mag"].Value = this.TurretMagnification.ToString();

                XmlManager.SetAttribute(ndList[0], experimentFile, "name", this.TurretName);

            }

            //Only save the position if the Epi Turret moves manually
            int scopeType = (int)ScopeType.UPRIGHT;
            if (1 != ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_EPITURRET, (int)IDevice.Params.PARAM_SCOPE_TYPE, ref scopeType) || (int)ScopeType.UPRIGHT == scopeType)
            {
                ndList = experimentFile.SelectNodes("/ThorImageExperiment/EPITurret");
                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], experimentFile, "pos", EpiTurretPosText);
                    XmlManager.SetAttribute(ndList[0], experimentFile, "name", EpiTurretPosName);
                }
                else
                {
                    XmlNode n = experimentFile.CreateNode(XmlNodeType.Element, "EPITurret", null);
                    XmlAttribute ps = experimentFile.CreateAttribute("pos");
                    XmlAttribute nm = experimentFile.CreateAttribute("name");
                    ps.Value = EpiTurretPosText;
                    nm.Value = EpiTurretPosName;
                    n.Attributes.Append(ps);
                    n.Attributes.Append(nm);
                    ndList = experimentFile.SelectNodes("/ThorImageExperiment");
                    ndList[0].AppendChild(n);
                }
            }

            UpdateLastSavedMag();
        }

        public void UpdateLastSavedMag()
        {
            //Persist globally the last magnification. This way the Objective changer will know the escape distance of the last used magnification
            List<string> xmlFiles = new List<string>();
            XmlDocument hardwareSettings = new XmlDocument();
            XmlNodeList ndList;
            ResourceManagerCS.BorrowDocMutexCS(SettingsFileType.HARDWARE_SETTINGS);
            MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS, false); // Save any pending changes to the current Hardware Settings
            foreach (string xmlFile in Directory.GetFiles(ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "Modalities", "HardwareSettings.xml", SearchOption.AllDirectories))
            {
                xmlFiles.Add(xmlFile);
            }
            //persist to all modalities:
            for (int fid = 0; fid < xmlFiles.Count; fid++)
            {
                if (!File.Exists(xmlFiles[fid]))
                {
                    continue;
                }
                hardwareSettings.Load(xmlFiles[fid]);
                ndList = hardwareSettings.SelectNodes("/HardwareSettings/Objectives");
                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], hardwareSettings, "lastSavedMagnification", this.TurretName);
                }
                hardwareSettings.Save(xmlFiles[fid]);
            }
            MVMManager.Instance.LoadSettings(SettingsFileType.HARDWARE_SETTINGS, false); // Load HWSettings with updated values
            ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.HARDWARE_SETTINGS);
        }

        #endregion Methods
    }
}