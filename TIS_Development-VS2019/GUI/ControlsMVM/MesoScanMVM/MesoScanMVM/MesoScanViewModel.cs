namespace MesoScan.ViewModel
{
    using System;
    using System.Collections.Concurrent;
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
    using System.Windows.Shapes;
    using System.Xml;
    using System.Xml.Linq;

    using MesoScan.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public class MesoScanViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        public const UInt32 MAX_IMAGE_SIZE = 2147483648; //2GB

        private const int REFRESH_TIME_MS = 200;

        private readonly MesoScanModel _mesoScanModel;

        System.Timers.Timer hb;
        Dictionary<ActionMessage, string> _actionReq;
        bool _doCalculation = false;
        Dictionary<string, string> _localParamsVal;
        MesoParams _mesoParams;
        private List<Roi> _microScanROIs = new List<Roi>();
        Dictionary<string, List<string>> _mvmParamsName;
        Dictionary<Tuple<string, string>, string> _mvmParamsVal;
        XDocument _pDoc;
        Dictionary<string, double> _positionParams;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private bool _restartScanner = true;
        private int _scanAreaID = 1;
        private int _scanID = (int)MesoScanTypes.Meso;
        private string _settingsPath = string.Empty;
        Dictionary<string, long> _templParamList;
        HashSet<long> _templScanLevel1Nodes;
        HashSet<Tuple<long, long>> _templScanLevel2Nodes;
        HashSet<Tuple<long, long, double>> _templScanLevel3Nodes;

        #endregion Fields

        #region Constructors

        public MesoScanViewModel()
        {
            this._mesoScanModel = new MesoScanModel();
            _mesoParams = new MesoParams();
            _actionReq = new Dictionary<ActionMessage, string>();
            _doCalculation = false;
            _positionParams = new Dictionary<string, double>
            {
                {"LSMAreaMode", 0.0},
                {"LSMPixelX", 0.0},
                {"LSMPixelY", 0.0},
                {"PhysicalFieldSize", 0.0},
                {"StripLength", 0.0},
                {"LSMFieldSize", 0.0},
                {"LSMFieldSizeMax", 0.0},
                {"LSMFieldSizeCalibration", 0.0},
                {"LSMFieldOffsetXActual", 0.0},
                {"LSMFieldOffsetYActual", 0.0},
                {"LSMChannel", 1.0}
            };
            _templScanLevel1Nodes = new HashSet<long>();
            _templScanLevel2Nodes = new HashSet<Tuple<long, long>>();
            _templScanLevel3Nodes = new HashSet<Tuple<long, long, double>>();

            _mvmParamsName = new Dictionary<string, List<string>>()
            {
                {"AreaControlViewModel", new List<string>()
                                        {
                                            "LSMAreaMode",
                                            "LSMPixelY",
                                            "LSMFieldSize",
                                            "LSMFieldSizeMax",
                                            "LSMFieldSizeCalibration",
                                            "SelectedStripSize",
                                            "MesoStripPixels",
                                            "LSMFieldOffsetXActual",
                                            "LSMFieldOffsetYActual",
                                            "SelectedViewMode",
                                            "SelectedScanArea"
                                        }
                },
                {"ScanControlViewModel", new List<string>()
                                        {
                                            "LastLSMScanMode",
                                            "LSMSignalAverage",
                                            "LSMSignalAverageFrames"
                                        }
                },
                {"CaptureSetupViewModel", new List<string>()
                                        {
                                            "LSMChannel"
                                        }
                },
                {"PowerControlViewModel", new List<string>()
                                        {
                                            "Power0",
                                            "Power1",
                                            "Power2",
                                            "Power3"
                                        }
                },
                 {"ZControlViewModel", new List<string>()
                                        {
                                            "ZPosition"
                                        }
                }
            };

            _mvmParamsVal = new Dictionary<Tuple<string, string>, string>();
            foreach (var mvm in _mvmParamsName)
            {
                foreach (var param in mvm.Value)
                {
                    Tuple<string, string> k = new Tuple<string, string>(mvm.Key, param);
                    _mvmParamsVal.Add(k, "");
                }
            }

            _localParamsVal = new Dictionary<string, string>
                                        {
                                            {"ScanID", "1"},
                                            {"ScanAreaID", "1"}
                                        };

            _templParamList = new Dictionary<string, long>{
                                                            {"ScanID", 10},
                                                            {"ObjectiveType", 10},
                                                            {"XPixelSize", 10},
                                                            {"YPixelSize", 10},
                                                            {"ZPixelSize", 10},
                                                            {"IPixelType", 10},
                                                            {"ResUnit", 10},
                                                            {"TileWidth", 10},
                                                            {"TileHeight", 10},
                                                            {"TimeInterval", 10},
                                                            {"SignificantBits", 10},
                                                            {"HasStored", 10},
                                                            {"ScanMode", 20},
                                                            {"AverageMode", 20},
                                                            {"NumberOfAverageFrame", 20},
                                                            {"PhysicalFieldSize", 20},
                                                            {"StripLength", 20},
                                                            {"CurrentPower", 20},
                                                            {"IsLivingMode", 20},
                                                            {"RemapShift", 20},
                                                            {"ScanAreaID", 30},
                                                            {"PhysicalSizeX", 30},
                                                            {"PhysicalSizeY", 30},
                                                            {"PhysicalSizeZ", 30},
                                                            {"PositionX", 30},
                                                            {"PositionY", 30},
                                                            {"PositionZ", 30},
                                                            {"SizeX", 30},
                                                            {"SizeY", 30},
                                                            {"SizeZ", 30},
                                                            {"SizeS", 30},
                                                            {"SizeT", 30},
                                                            {"IsEnable", 30},
                                                            {"IsActionable", 30},
                                                            {"CurrentZPosition", 30},
                                                            {"ZPosition", 40},
                                                            {"PowerPercentage", 40}
                                                          };

            LoadXmlMesoParams();

            hb = new System.Timers.Timer();
            hb.Elapsed += new System.Timers.ElapsedEventHandler(OnTimedEvent);
            hb.Interval = REFRESH_TIME_MS; //predefined to check/update active.xml every 200ms

            //update settings to RGG for field size
            SettingsPath = ResourceManagerCS.GetActiveSettingsFileString();
        }

        #endregion Constructors

        #region Properties

        public int IsLivingMode
        {
            get { return hb.Enabled ? 1 : 0; }
            set
            {
                UpdateRequest("IsLivingMode", value.ToString(), _scanID);
                if (1 == value)
                {
                    //push to file and only work with active settings in live imaging
                    SettingsPath = ResourceManagerCS.GetActiveSettingsFileString();
                    System.Threading.Thread.Sleep(REFRESH_TIME_MS);
                }
                hb.Enabled = (1 == value);
                OnPropertyChanged("IsLivingMode");
            }
        }

        public bool IsResonanceGalvoGalvo
        {
            get
            {
                return ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType() && (int)ICamera.LSMType.RESONANCE_GALVO_GALVO == ResourceManagerCS.GetLSMType());
            }
        }

        public int ScanAreaID
        {
            get { return _scanAreaID; }
            set
            {
                _scanAreaID = value;
                OnPropertyChanged("ScanAreaID");
            }
        }

        public int ScanID
        {
            get { return _scanID; }
            set
            {
                if (_scanID != value)
                {
                    _scanID = ((int)MesoScanTypes.Micro == value) ? (int)MesoScanTypes.Micro : (int)MesoScanTypes.Meso;
                    OnPropertyChanged("ScanID");

                    if (TryReloadXDoc())
                    {
                        XElement q = _pDoc.Root.Element("TemplateScans");
                        XElement scanInfo = q.Descendants("ScanInfo").Where(x => _scanID == Int32.Parse(x.Attribute("ScanID").Value)).FirstOrDefault();
                        if (null != scanInfo)
                        {
                            XElement scanAreas = scanInfo.Element("ScanAreas");
                            XElement scanArea = null;
                            if ((int)MesoScanTypes.Micro == _scanID)
                            {
                                scanArea = scanAreas.Descendants("ScanArea").Where(x => true == bool.Parse(x.Attribute("IsEnable").Value.ToLower())).FirstOrDefault();
                                if (null == scanArea)
                                {
                                    //no micro scan area is enabled, correct by selected scan area
                                    scanArea = scanAreas.Descendants("ScanArea").Where(x => (int)MVMManager.Instance["AreaControlViewModel", "SelectedScanArea", (object)0] == Int32.Parse(x.Attribute("ScanAreaID").Value)).FirstOrDefault();
                                    if (null != scanArea)
                                    {
                                        scanArea.Attribute("IsEnable").Value = "true";
                                        MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS] = XmlManager.ToXmlDocument(_pDoc);
                                        MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS, true);
                                    }
                                }
                            }
                            else
                            {
                                scanArea = scanAreas.Descendants("ScanArea").Where(x => 0 == Int32.Parse(x.Attribute("ScanAreaID").Value)).FirstOrDefault();
                            }

                            if (null != scanArea)
                            {
                                MVMManager.Instance["AreaControlViewModel", "LSMPixelX"] = Convert.ToInt32(scanArea.Attribute("SizeX").Value);
                                MVMManager.Instance["AreaControlViewModel", "LSMPixelY"] = Convert.ToInt32(scanArea.Attribute("SizeY").Value);

                                if ((int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)0] != (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)0])
                                    MVMManager.Instance["AreaControlViewModel", "LSMAreaMode"] = (int)ICamera.LSMAreaMode.RECTANGLE;
                                else
                                    MVMManager.Instance["AreaControlViewModel", "LSMAreaMode"] = (int)ICamera.LSMAreaMode.SQUARE;
                            }

                            MVMManager.Instance["AreaControlViewModel", "SelectedStripSize"] = Convert.ToInt32(scanInfo.Element("ScanConfigure").Attribute("PhysicalFieldSize").Value);
                            MVMManager.Instance["AreaControlViewModel", "MesoStripPixels"] = Convert.ToInt32(scanInfo.Element("ScanConfigure").Attribute("StripLength").Value);
                            MVMManager.Instance["PowerControlViewModel", "Power0"] = Convert.ToInt32(scanInfo.Element("ScanConfigure").Attribute("CurrentPower").Value);
                        }
                    }
                }
            }
        }

        public string SettingsPath
        {
            get { return _settingsPath; }
            set
            {
                _settingsPath = value;
                OnPropertyChanged("SettingsPath");

                UpdateMicroScanAreaSettings();
                CompareAndGenerateActionRequest();
                ResourceManagerCS.SetCameraParamString((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_MESO_EXP_PATH, _settingsPath);
            }
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

        public bool CompareAndGenerateActionRequest()
        {
            bool ret = false;
            if (!IsResonanceGalvoGalvo)
            {
                _actionReq.Clear();
                return ret;
            }

            try
            {
                //compare with other mvms
                foreach (var vm in _mvmParamsName)
                {
                    foreach (var param in vm.Value)
                    {
                        Tuple<string, string> k = new Tuple<string, string>(vm.Key, param);
                        string val = MVMManager.Instance[vm.Key, param, (object)0].ToString();
                        if (val.CompareTo(_mvmParamsVal[k]) != 0)
                        {
                            _mvmParamsVal[k] = val;
                            GenerateActionRequest(vm.Key, param, val);
                        }

                    }
                }
                //compare with local props
                foreach (var param in _localParamsVal.ToList())
                {
                    string val = this[param.Key, (object)0].ToString();
                    if (val.CompareTo(param.Value) != 0)
                    {
                        _localParamsVal[param.Key] = val;
                        GenerateActionRequest("this", param.Key);
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, ex.Message);
                return ret;
            }

            //update active xml
            if (_doCalculation || 0 < _actionReq.Count)
            {
                bool doLock = (0 == _settingsPath.CompareTo(ResourceManagerCS.GetActiveSettingsFileString())) ? true : false;

                if (!TryReloadXDoc(doLock))
                {
                    if (doLock)
                        ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                    return ret;
                }
                try
                {
                    XElement q = _pDoc.Root.Element("TemplateScans");
                    if (null != q)
                    {
                        //calculate for size and position
                        double umPerFieldSize = _positionParams["PhysicalFieldSize"] / _positionParams["LSMFieldSizeCalibration"];
                        double pixelX = _positionParams["LSMFieldSize"] * _positionParams["StripLength"] / umPerFieldSize;
                        double umPerPixel = _positionParams["PhysicalFieldSize"] / _positionParams["StripLength"];
                        if (_doCalculation)
                        {
                            //meso only, micro scan update based on settings
                            double aspectRatio = (int)ICamera.LSMAreaMode.SQUARE == (int)_positionParams["LSMAreaMode"] ? 1.0 : _positionParams["LSMPixelY"] / pixelX;
                            double[] pixelXY = ValidateImageSize(new double[2] { pixelX, pixelX * aspectRatio });

                            UpdateRequest("XPixelSize", umPerPixel.ToString(), _scanID);
                            UpdateRequest("YPixelSize", umPerPixel.ToString(), _scanID);
                            UpdateRequest("PhysicalFieldSize", _positionParams["PhysicalFieldSize"].ToString(), _scanID, 0);
                            UpdateRequest("StripLength", _positionParams["StripLength"].ToString(), _scanID, 0);
                            UpdateRequest("TileWidth", _positionParams["StripLength"].ToString(), _scanID);
                            UpdateRequest("TileHeight", _positionParams["StripLength"].ToString(), _scanID);

                            UpdateRequest("SizeX", ((int)pixelXY[0]).ToString(), _scanID, 0);
                            if ((int)pixelXY[0] != (int)_positionParams["LSMPixelX"])
                            {
                                MVMManager.Instance["AreaControlViewModel", "LSMPixelX"] = (int)pixelXY[0];
                                _positionParams["LSMPixelX"] = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)0];
                            }

                            if ((int)pixelXY[0] != (int)pixelXY[1])
                                MVMManager.Instance["AreaControlViewModel", "LSMAreaMode"] = (int)ICamera.LSMAreaMode.RECTANGLE;

                            _positionParams["LSMAreaMode"] = (int)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)(int)ICamera.LSMAreaMode.RECTANGLE];

                            UpdateRequest("SizeY", ((int)pixelXY[1]).ToString(), _scanID, 0);
                            if ((int)pixelXY[1] != (int)_positionParams["LSMPixelY"])
                            {
                                MVMManager.Instance["AreaControlViewModel", "LSMPixelY"] = (int)pixelXY[1];
                                _positionParams["LSMPixelY"] = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)0];
                            }

                            double[] sizeUM = { _positionParams["LSMPixelX"] * umPerPixel, _positionParams["LSMPixelY"] * umPerPixel };
                            UpdateRequest("PhysicalSizeX", sizeUM[0].ToString(), _scanID, 0);
                            UpdateRequest("PhysicalSizeY", sizeUM[1].ToString(), _scanID, 0);

                            double maxXUM = _positionParams["LSMFieldSizeMax"] * _positionParams["LSMFieldSizeCalibration"];
                            double[] origin = { maxXUM * (-0.5), maxXUM * (-0.5) };

                            double[] position = {(_positionParams["LSMFieldSize"] * (-0.5) - (_positionParams["LSMFieldOffsetXActual"])) * _positionParams["LSMFieldSizeCalibration"] - origin[0],
                                        (_positionParams["LSMFieldSize"] * aspectRatio * (-0.5) + (_positionParams["LSMFieldOffsetYActual"])) * _positionParams["LSMFieldSizeCalibration"] - origin[1]};

                            UpdateRequest("PositionX", position[0].ToString("F4"), _scanID, 0);
                            UpdateRequest("PositionY", position[1].ToString("F4"), _scanID, 0);

                            _doCalculation = false;
                        }
                        //update active scan area
                        if ((int)MesoScanTypes.Meso == _scanID)
                        {
                            //meso scan only, disable all micro scan area
                            UpdateRequest("IsEnable", true.ToString().ToLower(), (int)MesoScanTypes.Meso, 0);

                            for (int i = 0; i < _microScanROIs.Count; i++)
                            {
                                UpdateRequest("IsEnable", false.ToString().ToLower(), (int)MesoScanTypes.Micro, _microScanROIs[i]._roiID);
                            }
                        }
                        else if (0 < _microScanROIs.Count)
                        {
                            //disable Meso scan and enable single micro scan area
                            UpdateRequest("IsEnable", false.ToString().ToLower(), (int)MesoScanTypes.Meso, 0);

                            //update active from micro scan area settings
                            XElement lsm = _pDoc.Root.Element("LSM");
                            XElement microScanInfo = q.Descendants("ScanInfo").Where(x => (int)MesoScanTypes.Micro == Int32.Parse(x.Attribute("ScanID").Value)).FirstOrDefault();
                            XElement microScanAreas = microScanInfo.Element("ScanAreas");
                            XElement scanArea = microScanAreas.Descendants("ScanArea").Where(x => _scanAreaID == Int32.Parse(x.Attribute("ScanAreaID").Value)).FirstOrDefault();

                            lsm.Attribute("pixelX").Value = scanArea.Attribute("SizeX").Value;
                            MVMManager.Instance["AreaControlViewModel", "LSMPixelX"] = Int32.Parse(lsm.Attribute("pixelX").Value);
                            _positionParams["LSMPixelX"] = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)0];

                            MVMManager.Instance["AreaControlViewModel", "LSMAreaMode"] = (int)ICamera.LSMAreaMode.RECTANGLE;
                            _positionParams["LSMAreaMode"] = (int)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)(int)ICamera.LSMAreaMode.RECTANGLE];

                            lsm.Attribute("pixelY").Value = scanArea.Attribute("SizeY").Value;
                            MVMManager.Instance["AreaControlViewModel", "LSMPixelY"] = Int32.Parse(lsm.Attribute("pixelY").Value);
                            _positionParams["LSMPixelY"] = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)0];

                            for (int i = 0; i < _microScanROIs.Count; i++)
                            {
                                UpdateRequest("IsEnable", ((bool)(_scanAreaID == _microScanROIs[i]._roiID)).ToString().ToLower(), _scanID, _microScanROIs[i]._roiID);
                            }
                        }

                        //handle action requests
                        foreach (var item in _actionReq)
                        {
                            ActionMessage am = item.Key;
                            XElement temp = null;
                            XElement tInfo = null;
                            XElement tArea = null;
                            XElement tPowerPoint = null;
                            if (am._scanInfoId != -1)
                            {
                                tInfo = q.Descendants("ScanInfo").Where(x => x.Attribute("ScanID").Value.Equals(am._scanInfoId.ToString())).FirstOrDefault();
                                if (am._scanAreaId != -1)
                                {
                                    tArea = tInfo.Element("ScanAreas").Descendants("ScanArea").Where(x => x.Attribute("ScanAreaID").Value.Equals(am._scanAreaId.ToString())).FirstOrDefault();
                                    if (am._zposition != -1)
                                    {
                                        tPowerPoint = tArea.Element("PowerPoints").Descendants("PowerPoint").Where(x => x.Attribute("ZPosition").Value.Equals(am._zposition.ToString())).FirstOrDefault();
                                    }
                                }
                            }
                            switch (am._actionName)
                            {
                                case "ScanInfo":
                                    temp = q;
                                    if (item.Value.CompareTo("Create") == 0)
                                        AddScanInfo(ref temp, (int)am._scanInfoId);
                                    else
                                        RemoveScanInfo(ref temp, (int)am._scanInfoId);
                                    break;
                                case "ScanArea":
                                    temp = tInfo.Element("ScanAreas");
                                    if (item.Value.CompareTo("Create") == 0)
                                        AddScanArea(ref temp, (int)am._scanAreaId);
                                    else
                                        RemoveScanArea(ref temp, (int)am._scanAreaId);
                                    break;
                                case "PowerPoint":
                                    temp = tArea.Element("PowerPoints");
                                    if (item.Value.CompareTo("Create") == 0)
                                        AddPowerPoint(ref temp, am._zposition, 100.0);
                                    else
                                        RemovePowerPoint(ref temp, am._zposition);
                                    break;
                                default:
                                    if (tPowerPoint != null)
                                        temp = tPowerPoint;
                                    else if (tArea != null)
                                        temp = tArea;
                                    else
                                        temp = tInfo;

                                    if (_templParamList[am._actionName] == 20)
                                    {
                                        temp.Element("ScanConfigure").Attribute(am._actionName).Value = item.Value;
                                    }
                                    else if (_templParamList[am._actionName] == 21)
                                    {
                                        temp.Element("ScanConfigure").Element(am._actionName).Value = item.Value;
                                    }
                                    else
                                    {
                                        temp.Attribute(am._actionName).Value = item.Value;
                                        if (tArea == temp)
                                        {
                                            XElement lsm = _pDoc.Root.Element("LSM");
                                            if (am._actionName.Equals("SizeX"))
                                                lsm.Attribute("pixelX").Value = item.Value;
                                            if (am._actionName.Equals("SizeY"))
                                                lsm.Attribute("pixelY").Value = item.Value;
                                        }
                                    }
                                    break;
                            }
                        }

                        _pDoc.Save(_settingsPath);
                        if (doLock)
                            MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS] = XmlManager.ToXmlDocument(_pDoc);

                        _actionReq.Clear();
                        ret = true;
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, ex.Message);
                }
                if (doLock)
                    ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
            }
            return ret;
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(MesoScanViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            LoadXmlMesoParams();
            ConvertCalibration();
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
            UpdateMicroScanROIs();
            IsLivingMode = 0;
            CompareAndGenerateActionRequest();
        }

        public long UpdateRequest(string paramName = "", string paramValue = "", int scanId = -1, int scanAreaId = -1, double zPosition = -1)
        {
            long ret = -1;
            switch (paramName)
            {
                case "ScanInfo":
                    if (scanId > -1)
                    {
                        ActionMessage am = new ActionMessage("ScanInfo", scanId);
                        if (!_templScanLevel1Nodes.Contains(scanId))
                        {
                            // create scanInfo
                            _templScanLevel1Nodes.Add(scanId);
                        }
                        else
                        {
                            // delete scanInfo
                            _templScanLevel1Nodes.Remove(scanId);
                        }

                        if (_actionReq.ContainsKey(am))
                            _actionReq.Remove(am);
                        else
                            _actionReq.Add(am, "Delete");

                        ret = 1;
                    }
                    break;
                case "ScanArea":
                    if (scanId > -1 && scanAreaId > -1)
                    {
                        ActionMessage am = new ActionMessage("ScanArea", scanId, scanAreaId);

                        Tuple<long, long> t2 = new Tuple<long, long>(scanId, scanAreaId);
                        if (!_templScanLevel2Nodes.Contains(t2))
                        {
                            // create scanArea
                            paramValue = "Create";
                            _templScanLevel2Nodes.Add(t2);
                        }
                        else
                        {
                            // delete scanArea
                            paramValue = "Delete";
                            _templScanLevel2Nodes.Remove(t2);
                        }

                        if (_actionReq.ContainsKey(am))
                            _actionReq.Remove(am);
                        else
                            _actionReq.Add(am, paramValue);

                        ret = 1;
                    }
                    break;
                case "PowerPoint":
                    if (scanId > -1 && scanAreaId > -1 && zPosition > -1)
                    {
                        ActionMessage am = new ActionMessage("PowerPoint", scanId, scanAreaId, zPosition);
                        Tuple<long, long, double> t3 = new Tuple<long, long, double>(scanId, scanAreaId, zPosition);
                        if (!_templScanLevel3Nodes.Contains(new Tuple<long, long, double>(scanId, scanAreaId, zPosition)))
                        {
                            // create powerpoint
                            paramValue = "Create";
                            _templScanLevel3Nodes.Add(t3);
                        }
                        else
                        {
                            // delete powerpoint
                            paramValue = "Delete";
                            _templScanLevel3Nodes.Remove(t3);
                        }

                        if (_actionReq.ContainsKey(am))
                            _actionReq.Remove(am);
                        else
                            _actionReq.Add(am, paramValue);

                        ret = 1;
                    }
                    break;
                case "Name":
                    if (zPosition == -1)
                    {
                        ActionMessage am = (scanId > -1) ? new ActionMessage("Name", scanId, scanAreaId) : null;

                        if (null != am)
                        {
                            if (_actionReq.ContainsKey(am))
                                _actionReq[am] = paramValue;
                            else
                                _actionReq.Add(am, paramValue);
                        }
                    }
                    break;
                default:
                    if (_templParamList.ContainsKey(paramName))
                    {
                        ActionMessage am = null;
                        if (_templParamList[paramName] < 30 && scanId > -1 && _templScanLevel1Nodes.Contains(scanId))
                        {
                            am = new ActionMessage(paramName, scanId);
                        }
                        else if (_templParamList[paramName] < 40 && scanId > -1 && scanAreaId > -1 && _templScanLevel2Nodes.Contains(new Tuple<long, long>(scanId, scanAreaId)))
                        {
                            am = new ActionMessage(paramName, scanId, scanAreaId);
                        }
                        else if (scanId > -1 && scanAreaId > -1 && zPosition > -1 && _templScanLevel3Nodes.Contains(new Tuple<long, long, double>(scanId, scanAreaId, zPosition)))
                        {
                            am = new ActionMessage(paramName, scanId, scanAreaId, zPosition);
                        }

                        if (null != am)
                        {
                            if (_actionReq.ContainsKey(am))
                                _actionReq[am] = paramValue;
                            else
                                _actionReq.Add(am, paramValue);
                        }
                    }
                    break;
            }
            return ret;
        }

        private void ActionRequestFromAreaControl(string prop, string val)
        {
            _doCalculation = true;
            switch (prop)
            {
                case "LSMAreaMode":
                    _positionParams["LSMAreaMode"] = Convert.ToDouble(val);
                    break;
                case "LSMPixelY":
                    _positionParams["LSMPixelY"] = Convert.ToDouble(val);
                    break;
                case "LSMFieldSize":
                    _positionParams["LSMFieldSize"] = Convert.ToDouble(val);
                    break;
                case "LSMFieldSizeMax":
                    _positionParams["LSMFieldSizeMax"] = Convert.ToDouble(val);
                    break;
                case "LSMFieldSizeCalibration":
                    _positionParams["LSMFieldSizeCalibration"] = Convert.ToDouble(val);
                    break;
                case "SelectedStripSize":
                    _positionParams["PhysicalFieldSize"] = Convert.ToDouble(val);
                    break;
                case "MesoStripPixels":
                    _positionParams["StripLength"] = Convert.ToDouble(val);
                    break;
                case "LSMFieldOffsetXActual":
                    _positionParams["LSMFieldOffsetXActual"] = Convert.ToDouble(val);
                    break;
                case "LSMFieldOffsetYActual":
                    _positionParams["LSMFieldOffsetYActual"] = Convert.ToDouble(val);
                    break;
                case "SelectedViewMode":
                    ScanID = Convert.ToInt32(val) + (int)MesoScanTypes.Meso;
                    break;
                case "SelectedScanArea":
                    _doCalculation = false;
                    ScanAreaID = Convert.ToInt32(val);
                    break;
            }
        }

        private void ActionRequestFromCSControl(string prop, string val)
        {
            switch (prop)
            {
                case "LSMChannel":  //[A]:0,[B]:1,[C]:2,[D]:3,All:4
                    //restart if changing single or multiple(4) channels
                    _restartScanner = (((4 > _positionParams["LSMChannel"]) ? true : false) != ((4 > Convert.ToDouble(val)) ? true : false)) ? true : false;
                    _positionParams["LSMChannel"] = Convert.ToDouble(val);
                    _doCalculation = true;
                    break;
            }
        }

        private void ActionRequestFromLocal(string prop)
        {
            switch (prop)
            {
                case "ScanID":
                    UpdateRequest(prop, ScanID.ToString(), _scanID);

                    //use IsEnable at Meso to switch between Meso or Micro
                    bool val = ((int)MesoScanTypes.Meso == _scanID) ? true : false;
                    UpdateRequest("IsEnable", val.ToString().ToLower(), (int)MesoScanTypes.Meso, 0);
                    break;
                case "ScanAreaID":
                    UpdateRequest(prop, ScanAreaID.ToString(), (int)MesoScanTypes.Micro, _scanAreaID);
                    break;
            }
        }

        private void ActionRequestFromPowerControl(string prop, string val)
        {
            switch (prop)
            {
                case "Power0":
                    UpdateRequest("PowerPercentage", val, _scanID, ((int)MesoScanTypes.Meso == _scanID) ? 0 : _scanAreaID, 0);
                    UpdateRequest("CurrentPower", val, _scanID);
                    _restartScanner = false;
                    break;
                case "Power1":
                    break;
                case "Power2":
                    break;
                case "Power3":
                    break;
            }
        }

        private void ActionRequestFromScanControl(string prop, string val)
        {
            for (int i = 0; i < _mesoParams._templateScans.Count; i++)
            {
                switch (prop)
                {
                    case "LastLSMScanMode":
                        UpdateRequest("ScanMode", val, _mesoParams._templateScans[i]._scanId);
                        break;
                    case "LSMSignalAverage":
                        UpdateRequest("AverageMode", val, _mesoParams._templateScans[i]._scanId);
                        break;
                    case "LSMSignalAverageFrames":
                        UpdateRequest("NumberOfAverageFrame", val, _mesoParams._templateScans[i]._scanId);
                        break;
                }
            }
        }

        private void AddPowerPoint(ref XElement powerPointsElement, double zPosition, double powerPercentage)
        {
            XElement tPowerPoint = new XElement("PowerPoint",
                                        new XAttribute("ZPosition", zPosition),
                                        new XAttribute("PowerPercentage", powerPercentage)
                                                );
            powerPointsElement.Add(tPowerPoint);
        }

        private void AddScanArea(ref XElement scanAreasElement, int scanAreaId)
        {
            string name = "ISA" + scanAreaId.ToString();
            XElement tScanArea = new XElement("ScanArea",
                                    new XAttribute("ScanAreaID", scanAreaId),
                                    new XAttribute("Name", name),
                                    new XAttribute("Color", "#7D144389"),
                                    new XAttribute("PhysicalSizeX", 5000),
                                    new XAttribute("PhysicalSizeY", 5000),
                                    new XAttribute("PhysicalSizeZ", 1),
                                    new XAttribute("PositionX", 0),
                                    new XAttribute("PositionY", 0),
                                    new XAttribute("PositionZ", 0),
                                    new XAttribute("SizeX", 2134),
                                    new XAttribute("SizeY", 2134),
                                    new XAttribute("SizeZ", 1),
                                    new XAttribute("SizeS", 1),
                                    new XAttribute("SizeT", 1),
                                    new XAttribute("IsEnable", false),
                                    new XAttribute("IsActionable", true),
                                    new XAttribute("CurrentZPosition", 0),
                                    new XElement("PowerPoints", null)
                                            );
            scanAreasElement.Add(tScanArea);
        }

        private void AddScanInfo(ref XElement templateScansElement, int scanId)
        {
            string name = (1 == scanId) ? "Meso" : "Micro";
            XElement tScanInfo = new XElement("ScanInfo",
                                        new XAttribute("ScanID", scanId),
                                        new XAttribute("Name", name),
                                        new XAttribute("ObjectiveType", "4x"),
                                        new XAttribute("XPixelSize", 2.34375),
                                        new XAttribute("YPixelSize", 2.34375),
                                        new XAttribute("ZPixelSize", 0.5),
                                        new XAttribute("IPixelType", "UINT16"),
                                        new XAttribute("ResUnit", "Micron"),
                                        new XAttribute("TileWidth", 256),
                                        new XAttribute("TileHeight", 256),
                                        new XAttribute("TimeInterval", 0.5),
                                        new XAttribute("SignificantBits", 14),
                                        new XAttribute("HasStored", false),
                                        new XElement("ScanAreas", null),
                                        new XElement("ScanConfigure",
                                                new XAttribute("ScanMode", 1),
                                                new XAttribute("AverageMode", 0),
                                                new XAttribute("NumberOfAverageFrame", 1),
                                                new XAttribute("PhysicalFieldSize", 600),
                                                new XAttribute("StripLength", 256),
                                                new XAttribute("CurrentPower", 100),
                                                new XAttribute("IsLivingMode", 0),
                                                new XAttribute("RemapShift", 0)
                                                    )
                                    );
            templateScansElement.Add(tScanInfo);
        }

        /// <summary>
        /// Convert calibration from Confocal to Meso
        /// </summary>
        private void ConvertCalibration()
        {
            const int FIELDSIZE_CNT = 256;
            const int FIELDSIZE_REF = 120;
            string path = System.IO.Directory.GetCurrentDirectory();
            string zoomFile = path + "\\ZoomData.txt";
            string alignFile = path + "\\AlignData.txt";
            string mesoSettings = path + "\\ThorMesoScanSettings.xml";
            string confocalSettings = path + "\\ThorConfocalSettings.xml";

            if (File.Exists(mesoSettings) && File.Exists(confocalSettings))
            {
                if (!File.Exists(zoomFile) && !File.Exists(alignFile))
                    return;

                XDocument confocalDoc = XDocument.Load(confocalSettings);
                XElement confocalCal = confocalDoc.Root.Element("Calibration");
                double fieldSizeCalXmlVal = (null == confocalCal || null == confocalCal.Attribute("fieldSizeCalibration")) ? 80.1 : Double.Parse(confocalCal.Attribute("fieldSizeCalibration").Value);
                confocalCal = confocalDoc.Root.Element("Configuration");
                double field2theta = (null == confocalCal || null == confocalCal.Attribute("field2Theta")) ? 0.1 : Double.Parse(confocalCal.Attribute("field2Theta").Value);

                XDocument mesoDoc = XDocument.Load(mesoSettings);
                XElement res = mesoDoc.Root.Element("Resonance");
                XElement pockels = mesoDoc.Root.Element("Waveforms").Descendants("PokelsCellWaveform").FirstOrDefault();
                double pockelDutyCycle = (null == pockels || null == pockels.Attribute("PockelDutyCycle")) ? 1.0 : Double.Parse(pockels.Attribute("PockelDutyCycle").Value);
                pockelDutyCycle = (0 >= pockelDutyCycle) ? 1.0 : pockelDutyCycle;

                XElement decend1 = null, decend2 = null;
                List<int[]> fieldSizeAlign = new List<int[]>();

                //fine alignments
                if (File.Exists(alignFile))
                {
                    decend2 = res.Descendants("TowWayAlignment").FirstOrDefault();
                    fieldSizeAlign = File.ReadLines(alignFile).Select(line => line.Split(' ').Select(s => int.Parse(s)).ToArray()).ToList();
                }

                //zoom data
                if (File.Exists(zoomFile))
                {
                    decend1 = res.Descendants("ZoomCalibration").FirstOrDefault();
                    List<int[]> fieldSizePercentage = File.ReadLines(zoomFile).Select(line => line.Split(' ').Select(s => int.Parse(s)).ToArray()).ToList();
                    if (0 < fieldSizePercentage.Count)
                    {
                        if (FIELDSIZE_CNT != fieldSizePercentage.Count)
                            ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Warning, 1, "Number of calibrated field size is not 256 but: " + fieldSizePercentage.Count);

                        if (0 < fieldSizeAlign.Count && FIELDSIZE_CNT != fieldSizeAlign.Count)
                            ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Warning, 1, "Number of calibrated field size is not 256 but: " + fieldSizeAlign.Count);

                        decend1.Elements().Remove();
                        if (null != decend2)
                            decend2.Elements().Remove();

                        for (int i = 0; i < fieldSizePercentage.Count; i++)
                        {
                            double confocalFieldSizeCalibration = fieldSizeCalXmlVal * (1 + ((fieldSizePercentage[i][0] - fieldSizePercentage[FIELDSIZE_REF][0]) / (double)Constants.HUNDRED_PERCENT));
                            double xUM = i * confocalFieldSizeCalibration / (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)1.0];
                            double theta = i * field2theta;
                            double volt = theta / 2 * (1 + (fieldSizePercentage[i][0] / (double)Constants.HUNDRED_PERCENT)) / pockelDutyCycle;
                            decend1.Add(new XElement("Point", new XAttribute("Amplitude", xUM), new XAttribute("Voltage", volt)));

                            if ((null != decend2) && (i < fieldSizeAlign.Count))
                                decend2.Add(new XElement("Point", new XAttribute("Amplitude", xUM), new XAttribute("ShiftPoints", fieldSizeAlign[i][0])));
                        }
                    }
                }
                mesoDoc.Save(mesoSettings);
            }
        }

        private void DoDeleteCreateRequest()
        {
            var item = _actionReq.FirstOrDefault(e => e.Value.CompareTo("Delete") == 0);
            while (item.Key != null)
            {

            }
            foreach (var it in _actionReq)
            {
                if (item.Value.CompareTo("Create") != 0 && item.Value.CompareTo("Delete") != 0)
                {
                }

            }
        }

        /// <summary>
        /// convert property value to corresponding attribute/element value then generate actionMessage
        /// </summary>
        /// <param name="vm"></param>
        /// <param name="prop"></param>
        /// <param name="val"></param>
        private void GenerateActionRequest(string vm, string prop, string val = "")
        {
            switch (vm)
            {
                case "this":
                    _restartScanner = true;
                    ActionRequestFromLocal(prop);
                    break;
                case "AreaControlViewModel":
                    _restartScanner = true;
                    ActionRequestFromAreaControl(prop, val);
                    break;
                case "ScanControlViewModel":
                    _restartScanner = true;
                    ActionRequestFromScanControl(prop, val);
                    break;
                case "CaptureSetupViewModel":
                    ActionRequestFromCSControl(prop, val);
                    break;
                case "PowerControlViewModel":
                    ActionRequestFromPowerControl(prop, val);
                    break;
            }
        }

        private void LoadXmlMesoParams()
        {
            try
            {
                _pDoc = XmlManager.ToXDocument(MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS]);
                XElement q = _pDoc.Root.Element("TemplateScans");
                if (q == null)
                {
                    //create nodes for Mesoscan[1] and Microscan[2]:
                    XElement templateScans = new XElement("TemplateScans", null);
                    AddScanInfo(ref templateScans, (int)MesoScanTypes.Meso);
                    XElement scanInfo = templateScans.Descendants("ScanInfo").Where(x => (int)MesoScanTypes.Meso == Int32.Parse(x.Attribute("ScanID").Value)).FirstOrDefault();
                    XElement scanAreas = scanInfo.Element("ScanAreas");
                    AddScanArea(ref scanAreas, (int)MesoScanTypes.Meso - 1);
                    XElement scanArea = scanAreas.Descendants("ScanArea").Where(x => (int)MesoScanTypes.Meso - 1 == Int32.Parse(x.Attribute("ScanAreaID").Value)).FirstOrDefault();
                    XElement powerPoints = scanArea.Element("PowerPoints");
                    AddPowerPoint(ref powerPoints, 0.0, 100.0);
                    AddPowerPoint(ref powerPoints, 400.0, 100.0);
                    AddScanInfo(ref templateScans, (int)MesoScanTypes.Micro);

                    //update active settings and global xmldocument when scanner is Resonance-Galvo-Galvo
                    if (IsResonanceGalvoGalvo)
                    {
                        ResourceManagerCS.BorrowDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                        MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                        _pDoc = XDocument.Load(ResourceManagerCS.GetActiveSettingsFileString());
                        _pDoc.Root.Add(templateScans);
                        _pDoc.Save(ResourceManagerCS.GetActiveSettingsFileString());
                        MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS] = XmlManager.ToXmlDocument(_pDoc);
                        ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                    }
                    q = _pDoc.Root.Element("TemplateScans");
                }
                MapXml2MesoParams(ref q);

                //update through MVM for params only from TemplateScan
                Tuple<string, string> k = new Tuple<string, string>("AreaControlViewModel", "SelectedStripSize");
                MVMManager.Instance[k.Item1, k.Item2] = _mesoParams._templateScans[0]._scanConfigure._physicalFieldSize;
                _mvmParamsVal[k] = _mesoParams._templateScans[0]._scanConfigure._physicalFieldSize.ToString();
                GenerateActionRequest("AreaControlViewModel", "SelectedStripSize", _mesoParams._templateScans[0]._scanConfigure._physicalFieldSize.ToString());

                k = new Tuple<string, string>("AreaControlViewModel", "MesoStripPixels");
                MVMManager.Instance[k.Item1, k.Item2] = _mesoParams._templateScans[0]._scanConfigure._stripLength;
                _mvmParamsVal[k] = _mesoParams._templateScans[0]._scanConfigure._stripLength.ToString();
                GenerateActionRequest("AreaControlViewModel", "MesoStripPixels", _mesoParams._templateScans[0]._scanConfigure._stripLength.ToString());

            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, ex.Message);
            }
        }

        private void MapXml2MesoParams(ref XElement q)
        {
            _mesoParams._templateScans.Clear();
            foreach (var scanInfo in q.Elements("ScanInfo"))
            {
                ScanInfo tScanInfo = new ScanInfo();
                tScanInfo._scanId = Convert.ToInt32(scanInfo.Attribute("ScanID").Value);
                _templScanLevel1Nodes.Add(tScanInfo._scanId);
                tScanInfo._name = scanInfo.Attribute("Name").Value;
                tScanInfo._objectiveType = scanInfo.Attribute("ObjectiveType").Value;
                tScanInfo._pixelSize.X = Convert.ToDouble(scanInfo.Attribute("XPixelSize").Value);
                tScanInfo._pixelSize.Y = Convert.ToDouble(scanInfo.Attribute("YPixelSize").Value);
                tScanInfo._pixelSize.Z = Convert.ToDouble(scanInfo.Attribute("ZPixelSize").Value);
                tScanInfo._iPixelType = scanInfo.Attribute("IPixelType").ToString().GetTypeCode();
                tScanInfo._resUnit = (ResUnit)Enum.Parse(typeof(ResUnit), scanInfo.Attribute("ResUnit").Value, true);
                tScanInfo._tileWidth = Convert.ToInt32(scanInfo.Attribute("TileWidth").Value);
                tScanInfo._tileHeight = Convert.ToInt32(scanInfo.Attribute("TileHeight").Value);
                tScanInfo._timeInterval = Convert.ToDouble(scanInfo.Attribute("TimeInterval").Value);
                tScanInfo._significantBits = Convert.ToInt32(scanInfo.Attribute("SignificantBits").Value);
                tScanInfo._hasStored = Convert.ToBoolean(scanInfo.Attribute("HasStored").Value);
                var scanAreas = scanInfo.Element("ScanAreas");
                foreach (var scanArea in scanAreas.Elements("ScanArea"))
                {
                    ScanArea tScanArea = new ScanArea();
                    tScanArea._scanAreaId = Convert.ToInt32(scanArea.Attribute("ScanAreaID").Value);
                    _templScanLevel2Nodes.Add(new Tuple<long, long>(tScanInfo._scanId, tScanArea._scanAreaId));
                    tScanArea._name = scanArea.Attribute("Name").Value;
                    tScanArea._color = (Color)ColorConverter.ConvertFromString(scanArea.Attribute("Color").Value);
                    tScanArea._physicalSize.X = Convert.ToDouble(scanArea.Attribute("PhysicalSizeX").Value);
                    tScanArea._physicalSize.Y = Convert.ToDouble(scanArea.Attribute("PhysicalSizeY").Value);
                    tScanArea._physicalSize.Z = Convert.ToDouble(scanArea.Attribute("PhysicalSizeZ").Value);
                    tScanArea._position.X = Convert.ToDouble(scanArea.Attribute("PositionX").Value);
                    tScanArea._position.Y = Convert.ToDouble(scanArea.Attribute("PositionY").Value);
                    tScanArea._position.Z = Convert.ToDouble(scanArea.Attribute("PositionZ").Value);
                    tScanArea._size.X = Convert.ToDouble(scanArea.Attribute("SizeX").Value);
                    tScanArea._size.Y = Convert.ToDouble(scanArea.Attribute("SizeY").Value);
                    tScanArea._size.Z = Convert.ToDouble(scanArea.Attribute("SizeZ").Value);
                    tScanArea._sizeS = Convert.ToDouble(scanArea.Attribute("SizeS").Value);
                    tScanArea._sizeT = Convert.ToDouble(scanArea.Attribute("SizeT").Value);
                    tScanArea._isEnable = Convert.ToBoolean(scanArea.Attribute("IsEnable").Value);
                    tScanArea._isActionable = Convert.ToBoolean(scanArea.Attribute("IsActionable").Value);
                    tScanArea._currentZPosition = Convert.ToDouble(scanArea.Attribute("CurrentZPosition").Value);
                    var powerPoints = scanArea.Element("PowerPoints");
                    foreach (var powerPoint in powerPoints.Elements("PowerPoint"))
                    {
                        PowerPoint tPowerPoint = new PowerPoint();
                        tPowerPoint._zPosition = Convert.ToDouble(powerPoint.Attribute("ZPosition").Value);
                        _templScanLevel3Nodes.Add(new Tuple<long, long, double>(tScanInfo._scanId, tScanArea._scanAreaId, tPowerPoint._zPosition));
                        tPowerPoint._powerPercentage = Convert.ToDouble(powerPoint.Attribute("PowerPercentage").Value);
                        tScanArea._powerPoints.Add(tPowerPoint);
                    }
                    tScanInfo._scanAreas.Add(tScanArea);
                }
                var scanConfigure = scanInfo.Element("ScanConfigure");
                ScanConfigure tScanConfigure = new ScanConfigure();
                tScanConfigure._scanMode = Convert.ToInt32(scanConfigure.Attribute("ScanMode").Value);
                tScanConfigure._averageMode = Convert.ToInt32(scanConfigure.Attribute("AverageMode").Value);
                tScanConfigure._numberOfAverageFrame = Convert.ToInt32(scanConfigure.Attribute("NumberOfAverageFrame").Value);
                tScanConfigure._physicalFieldSize = Convert.ToInt32(scanConfigure.Attribute("PhysicalFieldSize").Value);
                tScanConfigure._stripLength = Convert.ToInt32(scanConfigure.Attribute("StripLength").Value);
                tScanConfigure._currentPower = Convert.ToInt32(scanConfigure.Attribute("CurrentPower").Value);
                tScanConfigure._isLivingMode = Convert.ToInt32(scanConfigure.Attribute("IsLivingMode").Value);
                tScanConfigure._remapShift = Convert.ToInt32(scanConfigure.Attribute("RemapShift").Value);
                tScanInfo._scanConfigure = tScanConfigure;
                _mesoParams._templateScans.Add(tScanInfo);
            }
        }

        private void OnTimedEvent(object source, System.Timers.ElapsedEventArgs e)
        {
            if (CompareAndGenerateActionRequest())
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FORCE_SETTINGS_UPDATE, (_restartScanner ? 1 : 0));
            }
        }

        private void RemovePowerPoint(ref XElement powerPointsElement, double zPosition)
        {
            XElement powerPoint = powerPointsElement.Descendants("PowerPoint").Where(x => x.Attribute("ZPosition").Value.Equals(zPosition.ToString())).FirstOrDefault();
            if (null != powerPoint)
                powerPoint.Remove();
        }

        private void RemoveScanArea(ref XElement scanAreasElement, int scanAreaId)
        {
            XElement scanArea = scanAreasElement.Descendants("ScanArea").Where(x => x.Attribute("ScanAreaID").Value.Equals(scanAreaId.ToString())).FirstOrDefault();
            if (null != scanArea)
                scanArea.Remove();
        }

        private void RemoveScanInfo(ref XElement templateScansElement, int scanId)
        {
            XElement scanInfo = templateScansElement.Descendants("ScanInfo").Where(x => x.Attribute("ScanID").Value.Equals(scanId.ToString())).FirstOrDefault();
            if (null != scanInfo)
                scanInfo.Remove();
        }

        private bool TryReloadXDoc(bool keepLock = false)
        {
            bool ret = false;
            try
            {
                //load from previous settings
                if (0 == _settingsPath.CompareTo(ResourceManagerCS.GetActiveSettingsFileString()))
                {
                    if (ResourceManagerCS.BorrowDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS, REFRESH_TIME_MS / 10))
                    {
                        MVMManager.Instance.ReloadSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                        _pDoc = XmlManager.ToXDocument(MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS]);
                        ret = true;
                    }
                }
                else
                {
                    _pDoc = XDocument.Load(_settingsPath);
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, ex.Message);
                ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                return ret;
            }
            if (!keepLock)
                ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);

            return ret;
        }

        /// <summary>
        /// Set micro scan areas' geometry settings by ROIs
        /// </summary>
        private void UpdateMicroScanAreaSettings()
        {
            UpdateMicroScanROIs();

            if (0 < _microScanROIs.Count)
            {
                if (TryReloadXDoc(true))
                {
                    XElement q = _pDoc.Root.Element("TemplateScans");
                    XElement scanInfo = q.Descendants("ScanInfo").Where(x => (int)MesoScanTypes.Micro == Int32.Parse(x.Attribute("ScanID").Value)).FirstOrDefault();
                    if (null != scanInfo)
                    {
                        XElement scanAreas = scanInfo.Element("ScanAreas");
                        XElement scanConfigure = scanInfo.Element("ScanConfigure");
                        scanAreas.Descendants("ScanArea").Remove();

                        double umPerPixel = _positionParams["PhysicalFieldSize"] / _positionParams["StripLength"];
                        double maxXUM = _positionParams["LSMFieldSizeMax"] * _positionParams["LSMFieldSizeCalibration"];
                        double[] origin = { maxXUM * (-0.5), maxXUM * (-0.5) };

                        for (int i = 0; i < _microScanROIs.Count; i++)
                        {
                            AddScanArea(ref scanAreas, _microScanROIs[i]._roiID);
                            XElement scanArea = scanAreas.Descendants("ScanArea").Where(x => _microScanROIs[i]._roiID == Int32.Parse(x.Attribute("ScanAreaID").Value)).FirstOrDefault();
                            XElement powerPoints = scanArea.Element("PowerPoints");
                            AddPowerPoint(ref powerPoints, 0.0, 100.0);
                            AddPowerPoint(ref powerPoints, 400.0, 100.0);

                            scanInfo.Attribute("XPixelSize").Value = umPerPixel.ToString();
                            scanInfo.Attribute("YPixelSize").Value = umPerPixel.ToString();
                            scanConfigure.Attribute("PhysicalFieldSize").Value = _positionParams["PhysicalFieldSize"].ToString();
                            scanConfigure.Attribute("StripLength").Value = _positionParams["StripLength"].ToString();
                            scanInfo.Attribute("TileWidth").Value = _positionParams["StripLength"].ToString();
                            scanInfo.Attribute("TileHeight").Value = _positionParams["StripLength"].ToString();

                            double[] pixelXY = ValidateImageSize(new double[2] { Math.Round(_microScanROIs[i]._bound.Size.Width), Math.Round(_microScanROIs[i]._bound.Size.Height) });
                            scanArea.Attribute("SizeX").Value = ((int)pixelXY[0]).ToString();
                            scanArea.Attribute("SizeY").Value = ((int)pixelXY[1]).ToString();

                            double[] sizeUM = { pixelXY[0] * umPerPixel, pixelXY[1] * umPerPixel };
                            scanArea.Attribute("PhysicalSizeX").Value = sizeUM[0].ToString();
                            scanArea.Attribute("PhysicalSizeY").Value = sizeUM[1].ToString();

                            double aspectRatio = pixelXY[1] / pixelXY[0];

                            double[] position = {(_positionParams["LSMFieldSize"] * (-0.5) - (_positionParams["LSMFieldOffsetXActual"])) * _positionParams["LSMFieldSizeCalibration"] - origin[0],
                                        (_positionParams["LSMFieldSize"] * aspectRatio * (-0.5) + (_positionParams["LSMFieldOffsetYActual"])) * _positionParams["LSMFieldSizeCalibration"] - origin[1]};

                            // roi Position = Vec of position + Vec of roiTLcorner
                            double[] roiTL = { _microScanROIs[i]._bound.TopLeft.X * umPerPixel, _microScanROIs[i]._bound.TopLeft.Y * umPerPixel };
                            scanArea.Attribute("PositionX").Value = (position[0] + roiTL[0]).ToString("F4");
                            scanArea.Attribute("PositionY").Value = (position[1] + roiTL[1]).ToString("F4");
                        }

                        MapXml2MesoParams(ref q);

                        MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS] = XmlManager.ToXmlDocument(_pDoc);
                        MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                    }
                }
                ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
            }
        }

        /// <summary>
        /// retrive micro scan area ROI indexes.
        /// </summary>
        private void UpdateMicroScanROIs()
        {
            //grab microscan areas then push to meso scanner after settings update
            _microScanROIs = OverlayManagerClass.Instance.GetModeROIs(Mode.MICRO_SCANAREA).Where(a => typeof(ROIRect) == a.GetType()).Select(area => new Roi()
            {
                _roiID = ((int[])area.Tag)[(int)Tag.SUB_PATTERN_ID],
                _bound = new Rect(((ROIRect)area).StartPoint, ((ROIRect)area).EndPoint),
            }).ToList();
        }

        /// <summary>
        /// verify image size is below 2GB
        /// </summary>
        /// <param name="pixelSizeXY"></param>
        /// <returns></returns>
        private double[] ValidateImageSize(double[] pixelSizeXY)
        {
            double[] retVal = pixelSizeXY;
            double aspectRatio = pixelSizeXY[1] / pixelSizeXY[0];
            double imageSize = (int)retVal[0] * (int)retVal[1] * (3.0 < _positionParams["LSMChannel"] ? 4.0 : 1.0) * sizeof(short);
            if (MAX_IMAGE_SIZE < imageSize)
            {
                retVal[0] = (double)MAX_IMAGE_SIZE / ((int)retVal[1] * (3.0 < _positionParams["LSMChannel"] ? 4.0 : 1.0) * sizeof(short));
                retVal[1] = retVal[0] * aspectRatio;
            }
            return retVal;
        }

        #endregion Methods
    }
}