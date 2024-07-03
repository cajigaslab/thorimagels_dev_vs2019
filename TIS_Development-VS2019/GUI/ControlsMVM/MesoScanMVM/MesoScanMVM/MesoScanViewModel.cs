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
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Linq;

    using MesoScan.Model;

    using OverlayManager;

    using Params;

    using ThorLogging;

    using ThorSharedTypes;

    using static ThorSharedTypes.ICamera;

    public class MesoScanViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        public const UInt32 MAX_IMAGE_SIZE = 2147483648; //2GB

        private const int REFRESH_TIME_MS = 200;

        private readonly MesoScanModel _mesoScanModel;

        private static Mutex mutex = new Mutex();
        private static Mutex mutex2 = new Mutex();

        object compareAndGenLock = new object();
        System.Timers.Timer hb;
        bool mROISpatialDisplay = false;
        bool running = false;
        bool running2 = false;
        Dictionary<ActionMessage, string> _actionReq;
        bool _doCalculation = false;
        bool _ignoreChanges = false;
        Dictionary<string, string> _localParamsVal;
        MesoParams _mesoParams;
        private List<Roi> _microScanROIs = new List<Roi>();
        Dictionary<string, List<string>> _mvmParamsName;
        Dictionary<Tuple<string, string>, string> _mvmParamsVal;
        Dictionary<string, double> _params;
        XDocument _pDoc;
        bool _previewAllROIs = true;
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
            _params = new Dictionary<string, double>
            {
                {"LSMAreaMode", 0.0},
                {"LSMPixelX", 512.0},
                {"LSMPixelY", 512.0},
                {"FullFOVPhysicalFieldSizeUM", 300.0},
                {"mROIStripePhysicalFieldSizeUM", 100.0},
                {"FullFOVStripePixels", 256.0},
                {"mROIStripePixels", 256.0},
                {"FullFOVStripeFieldSize", 255.0},
                {"mROIStripeFieldSize", 5.0},
                {"LSMFieldSize", 255.0},
                {"LSMFieldSizeXUM", 0.0},
                {"LSMFieldSizeYUM", 0.0},
                {"LSMFieldSizeMax", 0.0},
                {"LSMFieldSizeCalibration", 0.0},
                {"LSMFieldOffsetXActual", 0.0},
                {"LSMFieldOffsetYActual", 0.0},
                {"LSMScaleYScan", 1.0},
                {"LSMChannel", 1.0},
                {"CurrentPower0", 0.0 },
                {"CurrentPower1", 0.0 },
                {"CurrentPower2", 0.0 },
                {"CurrentPower3", 0.0 },
                {"ZPosition", 0.0 },
                {"LSMFlipHorizontal", 0 },
                {"LSMFlipVerticalScan", 0 }
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
                                            "LSMFieldSizeXUM",
                                            "LSMFieldSizeYUM",
                                            "LSMFieldSizeMax",
                                            "LSMFieldSizeCalibration",
                                            "FullFOVPhysicalFieldSizeUM",
                                            "mROIStripePhysicalFieldSizeUM",
                                            "LSMScaleYScan",
                                            "FullFOVStripeFieldSize",
                                            "mROIStripeFieldSize",
                                            "FullFOVStripePixels",
                                            "mROIStripePixels",
                                            "LSMFieldOffsetXActual",
                                            "LSMFieldOffsetYActual",
                                            "SelectedViewMode",
                                            "SelectedScanArea",
                                            "LSMFlipHorizontal",
                                            "LSMFlipVerticalScan"
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
                                                            {"MaxSizeInUMForMaxFieldSize", 10},
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
                                                            {"StripeFieldSize", 20},
                                                            {"IsLivingMode", 20},
                                                            {"RemapShift", 20},
                                                            {"CurrentPower", 20},
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

        public static ICamera.CameraType CameraType
        {
            get
            {
                int cameraType = 1;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_TYPE, ref cameraType);
                return (ICamera.CameraType)cameraType;
            }
        }

        public static ICamera.LSMType LSMType
        {
            get
            {
                if (ICamera.CameraType.LSM == CameraType)
                {
                    int lsmType = 1;
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);
                    return (ICamera.LSMType)lsmType;
                }
                else
                {
                    return ICamera.LSMType.LSMTYPE_LAST;
                }
            }
        }

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

        public bool IsmROIAvaliableAndEnabled
        {
            get
            {
                bool isRGG = ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType() && (int)ICamera.LSMType.RESONANCE_GALVO_GALVO == ResourceManagerCS.GetLSMType());
                bool mROIModeEnable = (int)MVMManager.Instance["AreaControlViewModel", "MROIModeEnable", (object)0] == 1;
                return isRGG && mROIModeEnable;
            }
        }

        public MesoParams mROIParams
        {
            get => _mesoParams;
        }

        public bool PreviewAllROIs
        {
            get => _previewAllROIs;
            set
            {
                if (SetProperty(ref _previewAllROIs, value))
                {
                    _previewAllROIs = value;

                    CompleteUpdateOfROIs();
                }
            }
        }

        public bool ReorderROIs
        {
            set
            {
                CompleteReorderingOfROIs();
            }
        }

        public bool ResizeROIsForFullFOVPixelDensity
        {
            set
            {
                DisplayROIsWithFULLFOVPixelDensity();
                mROISpatialDisplay = false;
            }
        }

        public bool ResizeROIsFormROIPixelDensity
        {
            set
            {
                DisplayROIsWithmROIPixelDensity();
                mROISpatialDisplay = true;
            }
        }

        public int ScanAreaID
        {
            get { return _scanAreaID; }
            set
            {
                if (_scanAreaID != value)
                {
                    _scanAreaID = value;
                    OnPropertyChanged("ScanAreaID");
                    if (!IsmROIAvaliableAndEnabled)
                    {
                        return;
                    }
                    if ((int)MesoScanTypes.Micro == _scanID)
                    {
                        int scanIndex = _scanID - 1;
                        if (_mesoParams?.TemplateScans?.Count > scanIndex)
                        {
                            for (int i = 0; i < _mesoParams.TemplateScans[scanIndex].ScanAreas?.Count; ++i)
                            {
                                if (_mesoParams.TemplateScans[scanIndex].ScanAreas[i]?.ScanAreaID == _scanAreaID)
                                {
                                    _ignoreChanges = true;
                                    MVMManager.Instance["PowerControlViewModel", "Power0"] = _mesoParams.TemplateScans[scanIndex].ScanAreas[i].Power0; //TODO:
                                    MVMManager.Instance["ZControlViewModel", "ZPosition"] = _mesoParams.TemplateScans[scanIndex].ScanAreas[i].ZPosition; //TODO:
                                    Thread.Sleep(300);
                                    _ignoreChanges = false;
                                    return;
                                }
                            }
                        }
                    }
                }
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
                    if (!IsmROIAvaliableAndEnabled)
                    {
                        return;
                    }
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

                            MapXml2MesoParams(ref q);
                            int scanIndex = _scanID - 1;
                            if ((int)MesoScanTypes.Meso == _scanID)
                            {
                                MVMManager.Instance["PowerControlViewModel", "IsMROI"] = false;
                            }
                            if (scanIndex < _mesoParams?.TemplateScans?.Count && _mesoParams.TemplateScans[scanIndex]?.ScanAreas?.Count > 0)
                            {
                                int selectedArea = (int)MesoScanTypes.Meso == _scanID ? 0 : ScanAreaID - 1;

                                if (selectedArea < _mesoParams.TemplateScans[scanIndex].ScanAreas.Count && selectedArea >= 0)
                                {
                                    MVMManager.Instance["PowerControlViewModel", "Power0"] = _mesoParams.TemplateScans[scanIndex].ScanAreas[selectedArea].Power0; //TODO: Fix this
                                    MVMManager.Instance["ZControlViewModel", "ZPosition"] = _mesoParams.TemplateScans[scanIndex].ScanAreas[selectedArea].ZPosition; //TODO:
                                }
                            }
                            if ((int)MesoScanTypes.Micro == _scanID)
                            {
                                MVMManager.Instance["PowerControlViewModel", "IsMROI"] = true;
                            }
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
                if (!IsmROIAvaliableAndEnabled)
                {
                    return;
                }
                CompareAndGenerateActionRequests();
                UpdateMicroScanAreaSettings();

                CalculateAndHandleActionRequests();
                Thread.Sleep(1);
                UpdateCaptureAndCheckLines();
            }
        }

        public bool TryForceUpdateLines
        {
            set 
            {
                if (!hb.Enabled)
                {
                    CompleteUpdateOfROIs();
                }
            }
        }

        public bool UpdateROIs
        {
            set
            {
                CompleteUpdateOfROIs();
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

        public bool CalculateAndHandleActionRequests()
        {
            if (!IsmROIAvaliableAndEnabled)
            {
                return false;
            }
            if (running) return false;

            running = true;
            mutex.WaitOne();
            {
                bool ret = false;

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
                            MVMManager.Instance["AreaControlViewModel", "LSMAreaMode"] = (int)ICamera.LSMAreaMode.SQUARE;
                            //calculate for size and position
                            double currentMagnification = (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)1.0];
                            double xDivision = _params["LSMFieldSize"] * _params["LSMFieldSizeCalibration"] / _params["FullFOVPhysicalFieldSizeUM"] / currentMagnification;
                            double xDivisionRound = Math.Round(xDivision);
                            double pixelX = xDivisionRound * _params["FullFOVStripePixels"];
                            double yScale = _params["LSMScaleYScan"];
                            double umPerPixelX = _params["FullFOVPhysicalFieldSizeUM"] / _params["FullFOVStripePixels"];
                            double umPerPixelY = (_params["FullFOVPhysicalFieldSizeUM"] / _params["FullFOVStripePixels"]) * yScale;

                            if (_doCalculation && (int)MesoScanTypes.Meso == _scanID)
                            {
                                //meso only, micro scan update based on settings
                                double aspectRatio = (int)ICamera.LSMAreaMode.SQUARE == (int)_params["LSMAreaMode"] ? 1.0 : _params["LSMPixelY"] / pixelX;
                                double[] pixelXY = ValidateImageSize(new double[2] { pixelX, pixelX * aspectRatio });

                                UpdateRequest("XPixelSize", umPerPixelX.ToString(), _scanID);
                                UpdateRequest("YPixelSize", umPerPixelY.ToString(), _scanID);
                                UpdateRequest("PhysicalFieldSize", _params["FullFOVPhysicalFieldSizeUM"].ToString(), _scanID, 0);
                                UpdateRequest("StripeFieldSize", _params["FullFOVStripeFieldSize"].ToString(), _scanID, 0);
                                UpdateRequest("StripLength", _params["FullFOVStripePixels"].ToString(), _scanID, 0);
                                UpdateRequest("TileWidth", _params["FullFOVStripePixels"].ToString(), _scanID);
                                UpdateRequest("TileHeight", _params["FullFOVStripePixels"].ToString(), _scanID);

                                UpdateRequest("SizeX", ((int)pixelXY[0]).ToString(), _scanID, 0);
                                if ((int)pixelXY[0] != (int)_params["LSMPixelX"])
                                {
                                    MVMManager.Instance["AreaControlViewModel", "LSMPixelX"] = (int)pixelXY[0];
                                    _params["LSMPixelX"] = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)0];
                                }

                                if ((int)pixelXY[0] != (int)pixelXY[1])
                                    MVMManager.Instance["AreaControlViewModel", "LSMAreaMode"] = (int)ICamera.LSMAreaMode.RECTANGLE;

                                _params["LSMAreaMode"] = (int)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)(int)ICamera.LSMAreaMode.RECTANGLE];

                                UpdateRequest("SizeY", ((int)pixelXY[1]).ToString(), _scanID, 0);
                                if ((int)pixelXY[1] != (int)_params["LSMPixelY"])
                                {
                                    MVMManager.Instance["AreaControlViewModel", "LSMPixelY"] = (int)pixelXY[1];
                                    _params["LSMPixelY"] = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)0];
                                }

                                double[] sizeUM = { _params["LSMPixelX"] * umPerPixelX, _params["LSMPixelY"] * umPerPixelY };
                                UpdateRequest("PhysicalSizeX", sizeUM[0].ToString(), _scanID, 0);
                                UpdateRequest("PhysicalSizeY", sizeUM[1].ToString(), _scanID, 0);

                                double maxXUM = _params["LSMFieldSizeMax"] * _params["LSMFieldSizeCalibration"] / currentMagnification;
                                double[] origin = { maxXUM * (-0.5), maxXUM * (-0.5) };

                                double[] position = { _params["FullFOVPhysicalFieldSizeUM"] * 0.5 - sizeUM[0] * 0.5, -sizeUM[1] * 0.5 };

                                UpdateRequest("PositionX", position[0].ToString("F4"), _scanID, 0);
                                UpdateRequest("PositionY", position[1].ToString("F4"), _scanID, 0);
                                _doCalculation = false;
                            }
                            _doCalculation = false;
                            //update active scan area
                            if ((int)MesoScanTypes.Meso == _scanID)
                            {
                                int tempZoom = 5;
                                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, ref tempZoom);

                                int newZoom = (int)_params["FullFOVStripeFieldSize"];
                                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_CONTROLUNIT, (int)IDevice.Params.PARAM_SCANNER_ZOOM_POS, newZoom, 1);

                                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, newZoom);
                                MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignmentCoarse"] = (int)MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignmentCoarse", (object)0];
                                MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignment"] = (int)MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignment", (object)0];

                                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, tempZoom);
                                MVMManager.Instance["AreaControlViewModel", "LSMAreaMode"] = (int)ICamera.LSMAreaMode.SQUARE;

                                double maxStripeSize = ((double)MVMManager.Instance["AreaControlViewModel", "LSMMaxFieldSizeXUM", (object)0]);

                                UpdateRequest("MaxSizeInUMForMaxFieldSize", maxStripeSize.ToString(), (int)MesoScanTypes.Meso, 0);

                                XElement scanInfo = q.Descendants("ScanInfo").Where(x => (int)MesoScanTypes.Meso == Int32.Parse(x.Attribute("ScanID").Value)).FirstOrDefault();

                                //set the single power point for the FULL FOV view (MESO)
                                if (null != scanInfo)
                                {
                                    XElement scanAreas = scanInfo.Element("ScanAreas");

                                    var area = scanAreas?.Descendants().FirstOrDefault();

                                    var pwrPoints = area?.Element("PowerPoints");
                                    if (pwrPoints != null)
                                    {
                                        var pw = pwrPoints.Descendants();
                                        for (int i = pw.Count() - 1; i >= 0; --i)
                                        {
                                            pw.ElementAt(i).Remove();
                                        }
                                        AddPowerPoint(ref pwrPoints, _params["ZPosition"], _params["CurrentPower0"]);
                                    }
                                }

                                //meso scan only, disable all micro scan area
                                UpdateRequest("IsEnable", true.ToString().ToLower(), (int)MesoScanTypes.Meso, 0);

                                for (int i = 0; i < _microScanROIs.Count; i++)
                                {
                                    UpdateRequest("IsEnable", false.ToString().ToLower(), (int)MesoScanTypes.Micro, _microScanROIs[i]._roiID);
                                }
                            }
                            else if (0 < _microScanROIs.Count)
                            {

                                int tempZoom = 5;

                                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, ref tempZoom);

                                int newZoom = (int)_params["mROIStripeFieldSize"];
                                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_CONTROLUNIT, (int)IDevice.Params.PARAM_SCANNER_ZOOM_POS, newZoom, 1);
                                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, newZoom);
                                MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignmentCoarse"] = (int)MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignmentCoarse", (object)0];
                                MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignment"] = (int)MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignment", (object)0];

                                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, tempZoom);

                                MVMManager.Instance["AreaControlViewModel", "LSMAreaMode"] = (int)ICamera.LSMAreaMode.SQUARE;

                                //disable Meso scan and enable single micro scan area
                                UpdateRequest("IsEnable", false.ToString().ToLower(), (int)MesoScanTypes.Meso, 0);

                                if (_previewAllROIs)
                                {
                                    for (int i = 0; i < _microScanROIs.Count; i++)
                                    {
                                        UpdateRequest("IsEnable", true.ToString().ToLower(), _scanID, _microScanROIs[i]._roiID);
                                    }
                                }
                                else
                                {
                                    for (int i = 0; i < _microScanROIs.Count; i++)
                                    {
                                        UpdateRequest("IsEnable", ((bool)(_scanAreaID == _microScanROIs[i]._roiID)).ToString().ToLower(), _scanID, _microScanROIs[i]._roiID);
                                    }
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
                                try
                                {
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
                                catch (Exception ex)
                                {
                                    ex.ToString();
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
                mutex.ReleaseMutex();

                running = false;
                return ret;
            }
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
            CompareAndGenerateActionRequests();
            UpdateMicroScanROIs();
            UpdateMicroScanAreaSettings();
            IsLivingMode = 0;
            CalculateAndHandleActionRequests();
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
            switch (prop)
            {
                case "LSMAreaMode":
                    if (_params["LSMAreaMode"] != Convert.ToDouble(val))
                    {
                        _params["LSMAreaMode"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "LSMPixelY":
                    if (_params["LSMPixelY"] != Convert.ToDouble(val))
                    {
                        _params["LSMPixelY"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "LSMFieldSize":
                    if (_params["LSMFieldSize"] != Convert.ToDouble(val))
                    {
                        _params["LSMFieldSize"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "LSMFieldSizeXUM":
                    if (_params["LSMFieldSizeXUM"] != Convert.ToDouble(val))
                    {
                        _params["LSMFieldSizeXUM"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "LSMFieldSizeYUM":
                    if (_params["LSMFieldSizeYUM"] != Convert.ToDouble(val))
                    {
                        _params["LSMFieldSizeYUM"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "LSMFieldSizeMax":
                    if (_params["LSMFieldSizeMax"] != Convert.ToDouble(val))
                    {
                        _params["LSMFieldSizeMax"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "LSMFieldSizeCalibration":
                    if (_params["LSMFieldSizeCalibration"] != Convert.ToDouble(val))
                    {
                        _params["LSMFieldSizeCalibration"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "FullFOVStripePixels":
                    if (_params["FullFOVStripePixels"] != Convert.ToDouble(val))
                    {
                        _params["FullFOVStripePixels"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "mROIStripePixels":
                    if (_params["mROIStripePixels"] != Convert.ToDouble(val))
                    {
                        _params["mROIStripePixels"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "FullFOVStripeFieldSize":
                    if (_params["FullFOVStripeFieldSize"] != Convert.ToDouble(val))
                    {
                        _params["FullFOVStripeFieldSize"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "mROIStripeFieldSize":
                    if (_params["mROIStripeFieldSize"] != Convert.ToDouble(val))
                    {
                        _params["mROIStripeFieldSize"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "FullFOVPhysicalFieldSizeUM":
                    if (_params["FullFOVPhysicalFieldSizeUM"] != Convert.ToDouble(val))
                    {
                        _params["FullFOVPhysicalFieldSizeUM"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "mROIStripePhysicalFieldSizeUM":
                    if (_params["mROIStripePhysicalFieldSizeUM"] != Convert.ToDouble(val))
                    {
                        _params["mROIStripePhysicalFieldSizeUM"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "LSMScaleYScan":
                    if (_params["LSMScaleYScan"] != Convert.ToDouble(val))
                    {
                        _params["LSMScaleYScan"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "LSMFieldOffsetXActual":
                    if (_params["LSMFieldOffsetXActual"] != Convert.ToDouble(val))
                    {
                        _params["LSMFieldOffsetXActual"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "LSMFieldOffsetYActual":
                    if (_params["LSMFieldOffsetYActual"] != Convert.ToDouble(val))
                    {
                        _params["LSMFieldOffsetYActual"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }

                    break;
                case "SelectedViewMode":
                    if (ScanID != Convert.ToInt32(val))
                    {
                        ScanID = Convert.ToInt32(val) + (int)MesoScanTypes.Meso;
                        _doCalculation = true;
                    }
                    break;
                case "SelectedScanArea":
                    _doCalculation = false;
                    ScanAreaID = Convert.ToInt32(val);
                    break;
                case "LSMFlipHorizontal":
                    if (_params["LSMFlipHorizontal"] != Convert.ToDouble(val))
                    {
                        _params["LSMFlipHorizontal"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "LSMFlipVerticalScan":
                    if (_params["LSMFlipVerticalScan"] != Convert.ToDouble(val))
                    {
                        _params["LSMFlipVerticalScan"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
            }
        }

        private void ActionRequestFromCSControl(string prop, string val)
        {
            switch (prop)
            {
                case "LSMChannel":  //[A]:0,[B]:1,[C]:2,[D]:3,All:4
                    //restart if changing single or multiple(4) channels
                    _restartScanner = (((4 > _params["LSMChannel"]) ? true : false) != ((4 > Convert.ToDouble(val)) ? true : false)) ? true : false;
                    _params["LSMChannel"] = Convert.ToDouble(val);
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
                    //     UpdateRequest(prop, ScanAreaID.ToString(), (int)MesoScanTypes.Micro, _scanAreaID);
                    break;
            }
        }

        private void ActionRequestFromPowerControl(string prop, string val)
        {
            switch (prop)
            {
                //case "Power0":
                //UpdateRequest("PowerPercentage", val, _scanID, ((int)MesoScanTypes.Meso == _scanID) ? 0 : _scanAreaID, 0);
                case "Power0":
                    if (_params["CurrentPower0"] != Convert.ToDouble(val))
                    {
                        //  UpdateRequest("CurrentPower", val, _scanID);
                        // UpdateRequest("CurrentPower", val, _scanID, ((int)MesoScanTypes.Meso == _scanID) ? 0 : _scanAreaID, _params["ZPosition"]);
                        _params["CurrentPower0"] = Convert.ToDouble(val);

                        if (!_ignoreChanges)
                        {
                            _doCalculation = true;
                        }
                    }
                    break;
                case "Power1":
                    if (_params["CurrentPower1"] != Convert.ToDouble(val))
                    {
                        _params["CurrentPower1"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "Power2":
                    if (_params["CurrentPower2"] != Convert.ToDouble(val))
                    {
                        _params["CurrentPower2"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                case "Power3":
                    if (_params["CurrentPower3"] != Convert.ToDouble(val))
                    {
                        _params["CurrentPower3"] = Convert.ToDouble(val);
                        _doCalculation = true;
                    }
                    break;
                    //UpdateRequest("CurrentPower", val, _scanID);
                    //_restartScanner = false;
                    //break;
                    //case "Power1":
                    //case "Power2":
                    //    break;
                    //case "Power3":
                    //    break;
            }
        }

        private void ActionRequestFromScanControl(string prop, string val)
        {
            for (int i = 0; i < _mesoParams.TemplateScans.Count; i++)
            {
                switch (prop)
                {
                    case "LastLSMScanMode":
                        UpdateRequest("ScanMode", val, _mesoParams.TemplateScans[i].ScanID);
                        break;
                    case "LSMSignalAverage":
                        UpdateRequest("AverageMode", val, _mesoParams.TemplateScans[i].ScanID);
                        break;
                    case "LSMSignalAverageFrames":
                        UpdateRequest("NumberOfAverageFrame", val, _mesoParams.TemplateScans[i].ScanID);
                        break;
                }
            }
        }

        //    break;
        private void ActionRequestFromZControl(string prop, string val)
        {
            switch (prop)
            {
                //case "Power0":
                //UpdateRequest("PowerPercentage", val, _scanID, ((int)MesoScanTypes.Meso == _scanID) ? 0 : _scanAreaID, 0);
                case "ZPosition":
                    //   if (_params["ZPosition"] != Convert.ToDouble(val))
                    {
                        _params["ZPosition"] = Convert.ToDouble(val);
                        if (!_ignoreChanges)
                        {
                            _doCalculation = true;
                        }
                    }
                    break;
                    //UpdateRequest("CurrentPower", val, _scanID);
                    //_restartScanner = false;
                    //break;
                    //case "Power1":
                    //    break;
                    //case "Power2":
                    //    break;
                    //case "Power3":
                    //    break;
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
                                    new XAttribute("PhysicalSizeX", 5000), //TODO: need to rename variables
                                    new XAttribute("PhysicalSizeY", 5000),
                                    new XAttribute("PhysicalSizeZ", 1),
                                    new XAttribute("PositionX", 0),
                                    new XAttribute("PositionY", 0),
                                    new XAttribute("PositionZ", 0),
                                    new XAttribute("PositionXFieldOffset", 0),
                                    new XAttribute("PositionYFieldOffset", 0),
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
                                        new XAttribute("MaxSizeInUMForMaxFieldSize", 1400.0),
                                        new XElement("ScanAreas", null),
                                        new XElement("ScanConfigure",
                                                new XAttribute("ScanMode", 1),
                                                new XAttribute("AverageMode", 0),
                                                new XAttribute("NumberOfAverageFrame", 1),
                                                new XAttribute("PhysicalFieldSize", 600),
                                                new XAttribute("StripeFieldSize", 50),
                                                new XAttribute("StripLength", 256),
                                                new XAttribute("CurrentPower", 100),
                                                new XAttribute("IsLivingMode", 0),
                                                new XAttribute("RemapShift", 0)
                                                    )
                                    );
            templateScansElement.Add(tScanInfo);
        }

        private void CompareAndGenerateActionRequests()
        {
            try
            {
                if (!IsmROIAvaliableAndEnabled)
                {
                    _actionReq.Clear();
                    return;
                }

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
                return;
            }
        }

        void CompleteReorderingOfROIs()
        {
            if (mROIParams?.TemplateScans != null)
            {
                for (int j = 0; j < mROIParams.TemplateScans.Count; ++j)
                {
                    if (mROIParams.TemplateScans[j].Name == Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Micro))
                    {
                        var mROIs = mROIParams.TemplateScans[j].ScanAreas;

                        mROIParams.TemplateScans[j].ScanAreas = new ObservableCollection<ScanArea>();

                        if (mROIs != null)
                        {
                            List<PowerPoint> PowerPoints = new List<PowerPoint>();
                            Dictionary<int, int> oldAndNewROIOrder = new Dictionary<int, int>();
                            for (int i = 0; i < mROIs.Count; i++)
                            {
                                oldAndNewROIOrder.Add(i, mROIs[i].ScanAreaID - 1);
                                PowerPoints.Add(new PowerPoint() { PowerPercentage0 = mROIs[i].Power0, ZPosition = mROIs[i].ZPosition });
                            }
                            OverlayManagerClass.Instance.ReorderROIs(oldAndNewROIOrder);
                            UpdateMicroScanAreaSettings(PowerPoints);

                            _doCalculation = true;
                            CalculateAndHandleActionRequests();
                            Thread.Sleep(1);
                            UpdateCaptureAndCheckLines();

                            break;
                        }
                    }
                }
            }
        }

        void CompleteUpdateOfROIs()
        {
            if (mROIParams?.TemplateScans != null)
            {
                for (int j = 0; j < mROIParams.TemplateScans.Count; ++j)
                {
                    if (mROIParams.TemplateScans[j].Name == Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Micro))
                    {
                        var mROIs = mROIParams.TemplateScans[j].ScanAreas;

                        mROIParams.TemplateScans[j].ScanAreas = new ObservableCollection<ScanArea>();

                        if (mROIs != null)
                        {
                            List<PowerPoint> PowerPoints = new List<PowerPoint>();

                            for (int i = 0; i < mROIs.Count; i++)
                            {
                                PowerPoints.Add(new PowerPoint() { PowerPercentage0 = mROIs[i].Power0, ZPosition = mROIs[i].ZPosition });
                            }

                            UpdateMicroScanAreaSettings(PowerPoints);

                            _doCalculation = true;
                            CalculateAndHandleActionRequests();

                            UpdateCaptureAndCheckLines();
                            break;
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Convert calibration from Confocal to Meso
        /// </summary>
        private void ConvertCalibration()
        {
            //const int FIELDSIZE_CNT = 256;
            //const int FIELDSIZE_REF = 120;
            //string path = System.IO.Directory.GetCurrentDirectory();
            //string zoomFile = path + "\\ZoomData.txt";
            //string alignFile = path + "\\AlignData.txt";
            //string mesoSettings = path + "\\ThorMesoScanSettings.xml";
            //string confocalSettings = path + "\\ThorConfocalSettings.xml";

            //if (File.Exists(mesoSettings) && File.Exists(confocalSettings))
            //{
            //    if (!File.Exists(zoomFile) && !File.Exists(alignFile))
            //        return;

            //    XDocument confocalDoc = XDocument.Load(confocalSettings);
            //    XElement confocalCal = confocalDoc.Root.Element("Calibration");
            //    double fieldSizeCalXmlVal = (null == confocalCal || null == confocalCal.Attribute("fieldSizeCalibration")) ? 80.1 : Double.Parse(confocalCal.Attribute("fieldSizeCalibration").Value);
            //    confocalCal = confocalDoc.Root.Element("Configuration");
            //    double field2theta = (null == confocalCal || null == confocalCal.Attribute("field2Theta")) ? 0.1 : Double.Parse(confocalCal.Attribute("field2Theta").Value);

            //    XDocument mesoDoc = XDocument.Load(mesoSettings);
            //    XElement res = mesoDoc.Root.Element("Resonance");
            //    XElement pockels = mesoDoc.Root.Element("Waveforms").Descendants("PokelsCellWaveform").FirstOrDefault();
            //    double pockelDutyCycle = (null == pockels || null == pockels.Attribute("PockelDutyCycle")) ? 1.0 : Double.Parse(pockels.Attribute("PockelDutyCycle").Value);
            //    pockelDutyCycle = (0 >= pockelDutyCycle) ? 1.0 : pockelDutyCycle;

            //    XElement decend1 = null, decend2 = null;
            //    List<int[]> fieldSizeAlign = new List<int[]>();

            //    //fine alignments
            //    if (File.Exists(alignFile))
            //    {
            //        decend2 = res.Descendants("TowWayAlignment").FirstOrDefault();
            //        fieldSizeAlign = File.ReadLines(alignFile).Select(line => line.Split(' ').Select(s => int.Parse(s)).ToArray()).ToList();
            //    }

            //    //zoom data
            //    if (File.Exists(zoomFile))
            //    {
            //        decend1 = res.Descendants("ZoomCalibration").FirstOrDefault();
            //        List<int[]> fieldSizePercentage = File.ReadLines(zoomFile).Select(line => line.Split(' ').Select(s => int.Parse(s)).ToArray()).ToList();
            //        if (0 < fieldSizePercentage.Count)
            //        {
            //            if (FIELDSIZE_CNT != fieldSizePercentage.Count)
            //                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Warning, 1, "Number of calibrated field size is not 256 but: " + fieldSizePercentage.Count);

            //            if (0 < fieldSizeAlign.Count && FIELDSIZE_CNT != fieldSizeAlign.Count)
            //                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Warning, 1, "Number of calibrated field size is not 256 but: " + fieldSizeAlign.Count);

            //            decend1.Elements().Remove();
            //            if (null != decend2)
            //                decend2.Elements().Remove();

            //            for (int i = 0; i < fieldSizePercentage.Count; i++)
            //            {
            //                double confocalFieldSizeCalibration = fieldSizeCalXmlVal * (1 + ((fieldSizePercentage[i][0] - fieldSizePercentage[FIELDSIZE_REF][0]) / (double)Constants.HUNDRED_PERCENT));
            //                double xUM = i * confocalFieldSizeCalibration / (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)1.0];
            //                double theta = i * field2theta;
            //                double volt = theta / 2 * (1 + (fieldSizePercentage[i][0] / (double)Constants.HUNDRED_PERCENT)) / pockelDutyCycle;
            //                decend1.Add(new XElement("Point", new XAttribute("Amplitude", xUM), new XAttribute("Voltage", volt)));

            //                if ((null != decend2) && (i < fieldSizeAlign.Count))
            //                    decend2.Add(new XElement("Point", new XAttribute("Amplitude", xUM), new XAttribute("ShiftPoints", fieldSizeAlign[i][0])));
            //            }
            //        }
            //    }
            //    mesoDoc.Save(mesoSettings);
            // }
        }

        private void DisplayROIsWithFULLFOVPixelDensity()
        {
            try
            {
                var ROIs = OverlayManagerClass.Instance.GetModeROIs(Mode.MICRO_SCANAREA);

                int pixelW = 0;
                int pixelH = 0;
                double pixelSizeX = 0;
                double pixelSizeY = 0;
                if (mROIParams?.TemplateScans != null)
                {
                    for (int j = 0; j < mROIParams.TemplateScans.Count; ++j)
                    {
                        if (mROIParams.TemplateScans[j].Name == Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Meso))
                        {
                            var mROIs = mROIParams.TemplateScans[j].ScanAreas;
                            if (mROIs?.Count > 0 && mROIs[0] != null)
                            {
                                pixelSizeX = mROIParams.TemplateScans[j].PixelSize.X;
                                pixelSizeY = mROIParams.TemplateScans[j].PixelSize.Y;
                                pixelW = (int)Math.Floor(mROIs[0].PhysicalSizeXUM / mROIs[0].PhysicalSizeXUM) * mROIs[0].SizeXPixels;
                                pixelH = (int)Math.Floor(mROIs[0].PhysicalSizeYUM / mROIs[0].PhysicalSizeYUM) * mROIs[0].SizeYPixels;
                            }

                            break;
                        }
                    }
                    int horizontalFlip = (int)_params["LSMFlipHorizontal"];
                    int horizontalFlipSign = horizontalFlip == 0 ? 1 : -1;

                    int verticvalFlip = (int)_params["LSMFlipVerticalScan"];
                    int verticalFlipSign = verticvalFlip == 0 ? 1 : -1;

                    for (int j = 0; j < mROIParams.TemplateScans.Count; ++j)
                    {
                        if (mROIParams.TemplateScans[j].Name == Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Micro))
                        {
                            var mROIs = mROIParams.TemplateScans[j].ScanAreas;

                            int imageWidth = pixelW;
                            int imageHeight = pixelH;
                            if (mROIs != null)
                            {
                                for (int i = 0; i < mROIs.Count; i++)
                                {
                                    if (i >= ROIs.Count)
                                    {
                                        break;
                                    }
                                    double stripeSize = (double)MVMManager.Instance["AreaControlViewModel", "mROIStripePhysicalFieldSizeUM", (object)0];
                                    int pixelX = (int)Math.Ceiling((mROIs[i].Stripes * stripeSize / pixelSizeX));
                                    int pixelY = (int)Math.Round((mROIs[i].PhysicalSizeYUM / pixelSizeY));
                                    double position = horizontalFlipSign * mROIs[i].PositionXUM / pixelSizeX + 0.5 * imageWidth;
                                    int left = (int)(int)Math.Round((position - pixelX / 2 / mROIs[i].Stripes));

                                    if (horizontalFlip == 1 && mROIs[i].Stripes > 1)
                                    {
                                        double stripeWidth = pixelX / mROIs[i].Stripes;
                                        int stripeCollocation = mROIs[i].Stripes - 1;
                                        left -= (int)stripeWidth * stripeCollocation;
                                    }

                                    int top = (int)(int)Math.Round((verticalFlipSign * mROIs[i].PositionYUM / pixelSizeY + 0.5 * imageHeight));

                                    if (verticvalFlip == 1)
                                    {
                                        top -= (int)pixelY;
                                    }

                                    ((ROIRect)ROIs[i]).StartPoint = new Point(left, top);
                                    ((ROIRect)ROIs[i]).EndPoint = new Point(left + pixelX, top + pixelY);
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void DisplayROIsWithmROIPixelDensity()
        {
            try
            {
                var ROIs = OverlayManagerClass.Instance.GetModeROIs(Mode.MICRO_SCANAREA);

                double imageWidthUM = 0;
                double imageHeightUM = 0;
                if (mROIParams?.TemplateScans != null)
                {
                    for (int j = 0; j < mROIParams.TemplateScans.Count; ++j)
                    {
                        if (mROIParams.TemplateScans[j].Name == Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Meso))
                        {
                            var mROIs = mROIParams.TemplateScans[j].ScanAreas;
                            if (mROIs?.Count > 0 && mROIs[0] != null)
                            {
                                imageWidthUM = mROIs[0].PhysicalSizeXUM;
                                imageHeightUM = mROIs[0].PhysicalSizeYUM;
                            }

                            break;
                        }
                    }

                    int horizontalFlip = (int)_params["LSMFlipHorizontal"];

                    int horizontalFlipSign = horizontalFlip == 0 ? 1 : -1;

                    int verticvalFlip = (int)_params["LSMFlipVerticalScan"];
                    int verticalFlipSign = verticvalFlip == 0 ? 1 : -1;

                    for (int j = 0; j < mROIParams.TemplateScans.Count; ++j)
                    {
                        if (mROIParams.TemplateScans[j].Name == Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Micro))
                        {
                            var mROIs = mROIParams.TemplateScans[j].ScanAreas;
                            var pixelSizeX = mROIParams.TemplateScans[j].PixelSize.X;
                            var pixelSizeY = mROIParams.TemplateScans[j].PixelSize.Y;
                            int imageWidth = (int)(imageWidthUM / pixelSizeX);
                            imageWidth = (imageWidth % 2) == 0 ? imageWidth : imageWidth + 1;
                            int imageHeight = (int)(imageHeightUM / pixelSizeY);
                            imageHeight = (imageHeight % 2) == 0 ? imageHeight : imageHeight + 1;
                            if (mROIs != null)
                            {
                                for (int i = 0; i < mROIs.Count; i++)
                                {
                                    if (i >= ROIs.Count)
                                    {
                                        break;
                                    }

                                    int pixelX = (int)(mROIs[i].SizeXPixels);
                                    double stripeWidth = pixelX / mROIs[i].Stripes;
                                    int pixelY = mROIs[i].SizeYPixels;
                                    double position = horizontalFlipSign * mROIs[i].PositionXUM / pixelSizeX + 0.5 * imageWidth;
                                    int left = (int)Math.Round((position - mROIs[i].SizeXPixels / 2 / mROIs[i].Stripes));
                                    if (horizontalFlip == 1 && mROIs[i].Stripes > 1)
                                    {
                                        int stripeCollocation = mROIs[i].Stripes - 1;
                                        left -= (int)stripeWidth * stripeCollocation;
                                    }
                                    int top = (int)Math.Round(verticalFlipSign * mROIs[i].PositionYUM / pixelSizeY + 0.5 * imageHeight);

                                    if (verticvalFlip == 1)
                                    {
                                        top -= (int)pixelY;
                                    }
                                    ((ROIRect)ROIs[i]).StartPoint = new Point(left, top);
                                    ((ROIRect)ROIs[i]).EndPoint = new Point(left + pixelX, top + pixelY);
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
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
                case "ZControlViewModel":
                    ActionRequestFromZControl(prop, val);
                    break;
            }
        }

        private void LoadXmlMesoParams()
        {
            if (!IsmROIAvaliableAndEnabled)
            {
                return;
            }
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
                    AddPowerPoint(ref powerPoints, _params["ZPosition"], 0.0);
                    AddScanInfo(ref templateScans, (int)MesoScanTypes.Micro);

                    //update active settings and global xmldocument when scanner is Resonance-Galvo-Galvo
                    if (IsmROIAvaliableAndEnabled)
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
                Tuple<string, string> k = new Tuple<string, string>("AreaControlViewModel", "FullFOVStripeFieldSize");
                MVMManager.Instance[k.Item1, k.Item2] = _mesoParams.TemplateScans[0].ScanConfigure._stripeFieldSize;
                _mvmParamsVal[k] = _mesoParams.TemplateScans[0].ScanConfigure._stripeFieldSize.ToString();
                GenerateActionRequest("AreaControlViewModel", "FullFOVStripeFieldSize", _mesoParams.TemplateScans[0].ScanConfigure._stripeFieldSize.ToString());

                k = new Tuple<string, string>("AreaControlViewModel", "FullFOVStripePixels");
                MVMManager.Instance[k.Item1, k.Item2] = _mesoParams.TemplateScans[0].ScanConfigure._stripLength;
                _mvmParamsVal[k] = _mesoParams.TemplateScans[0].ScanConfigure._stripLength.ToString();
                GenerateActionRequest("AreaControlViewModel", "FullFOVStripePixels", _mesoParams.TemplateScans[0].ScanConfigure._stripLength.ToString());

                k = new Tuple<string, string>("AreaControlViewModel", "mROIStripeFieldSize");
                MVMManager.Instance[k.Item1, k.Item2] = _mesoParams.TemplateScans[1].ScanConfigure._stripeFieldSize;
                _mvmParamsVal[k] = _mesoParams.TemplateScans[1].ScanConfigure._stripeFieldSize.ToString();
                GenerateActionRequest("AreaControlViewModel", "mROIStripeFieldSize", _mesoParams.TemplateScans[1].ScanConfigure._stripeFieldSize.ToString());

                k = new Tuple<string, string>("AreaControlViewModel", "mROIStripePixels");
                MVMManager.Instance[k.Item1, k.Item2] = _mesoParams.TemplateScans[1].ScanConfigure._stripLength;
                _mvmParamsVal[k] = _mesoParams.TemplateScans[1].ScanConfigure._stripLength.ToString();
                GenerateActionRequest("AreaControlViewModel", "mROIStripePixels", _mesoParams.TemplateScans[1].ScanConfigure._stripLength.ToString());

            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, ex.Message);
            }
        }

        private void MapXml2MesoParams(ref XElement q)
        {
            if (!IsmROIAvaliableAndEnabled)
            {
                return;
            }
            if (q == null) return;
            _mesoParams.TemplateScans.Clear();
            foreach (var scanInfo in q.Elements("ScanInfo"))
            {
                ScanInfo tScanInfo = new ScanInfo();
                tScanInfo.ScanID = Convert.ToInt32(scanInfo.Attribute("ScanID").Value);
                _templScanLevel1Nodes.Add(tScanInfo.ScanID);
                tScanInfo.Name = scanInfo.Attribute("Name").Value;
                tScanInfo.ObjectiveType = scanInfo.Attribute("ObjectiveType").Value;
                tScanInfo.PixelSize.X = Convert.ToDouble(scanInfo.Attribute("XPixelSize").Value);
                tScanInfo.PixelSize.Y = Convert.ToDouble(scanInfo.Attribute("YPixelSize").Value);
                tScanInfo.PixelSize.Z = Convert.ToDouble(scanInfo.Attribute("ZPixelSize").Value);
                tScanInfo.iPixelType = scanInfo.Attribute("IPixelType").ToString().GetTypeCode();
                tScanInfo.ResUnit = (ResUnit)Enum.Parse(typeof(ResUnit), scanInfo.Attribute("ResUnit").Value, true);
                tScanInfo.TileWidth = Convert.ToInt32(scanInfo.Attribute("TileWidth").Value);
                tScanInfo.TileHeight = Convert.ToInt32(scanInfo.Attribute("TileHeight").Value);
                tScanInfo.TimeInterval = Convert.ToDouble(scanInfo.Attribute("TimeInterval").Value);
                tScanInfo.SignificantBits = Convert.ToInt32(scanInfo.Attribute("SignificantBits").Value);
                tScanInfo.HasStored = Convert.ToBoolean(scanInfo.Attribute("HasStored").Value);
                var scanConfigure = scanInfo.Element("ScanConfigure");
                double stripeSize = Convert.ToDouble(scanConfigure.Attribute("PhysicalFieldSize").Value);
                var scanAreas = scanInfo.Element("ScanAreas");

                foreach (var scanArea in scanAreas.Elements("ScanArea"))
                {
                    ScanArea tScanArea = new ScanArea();
                    tScanArea.ScanAreaID = Convert.ToInt32(scanArea.Attribute("ScanAreaID").Value);
                    _templScanLevel2Nodes.Add(new Tuple<long, long>(tScanInfo.ScanID, tScanArea.ScanAreaID));
                    tScanArea.Name = scanArea.Attribute("Name").Value;
                    // tScanArea.Color = (Color)ColorConverter.ConvertFromString(scanArea.Attribute("Color").Value);
                    tScanArea.PhysicalSizeXUM = Convert.ToDouble(scanArea.Attribute("PhysicalSizeX").Value);
                    int nStripes = (int)Math.Round(tScanArea.PhysicalSizeXUM / stripeSize);
                    tScanArea.Stripes = nStripes;
                    tScanArea.PhysicalSizeYUM = Convert.ToDouble(scanArea.Attribute("PhysicalSizeY").Value);
                    tScanArea.PhysicalSizeZ = Convert.ToDouble(scanArea.Attribute("PhysicalSizeZ").Value);
                    tScanArea.PositionXUM = Convert.ToDouble(scanArea.Attribute("PositionX").Value);
                    tScanArea.PositionYUM = Convert.ToDouble(scanArea.Attribute("PositionY").Value);
                    tScanArea.PositionZ = Convert.ToDouble(scanArea.Attribute("PositionZ").Value);
                    tScanArea.SizeXPixels = (int)Convert.ToDouble(scanArea.Attribute("SizeX").Value);
                    tScanArea.SizeYPixels = (int)Convert.ToDouble(scanArea.Attribute("SizeY").Value);
                    //tScanArea.SizeXPixels = Convert.ToDouble(scanArea.Attribute("SizeZ").Value);
                    //tScanArea._sizeS = Convert.ToDouble(scanArea.Attribute("SizeS").Value);
                    //tScanArea._sizeT = Convert.ToDouble(scanArea.Attribute("SizeT").Value);
                    tScanArea.IsEnable = Convert.ToBoolean(scanArea.Attribute("IsEnable").Value);
                    //tScanArea._isActionable = Convert.ToBoolean(scanArea.Attribute("IsActionable").Value);
                    //tScanArea._currentZPosition = Convert.ToDouble(scanArea.Attribute("CurrentZPosition").Value);
                    var powerPoints = scanArea.Element("PowerPoints");
                    foreach (var powerPoint in powerPoints.Elements("PowerPoint"))
                    {
                        PowerPoint tPowerPoint = new PowerPoint();
                        tPowerPoint.ZPosition = Convert.ToDouble(powerPoint.Attribute("ZPosition").Value);
                        _templScanLevel3Nodes.Add(new Tuple<long, long, double>(tScanInfo.ScanID, tScanArea.ScanAreaID, tPowerPoint.ZPosition));
                        tPowerPoint.PowerPercentage0 = Convert.ToDouble(powerPoint.Attribute("PowerPercentage").Value);
                        tScanArea.PowerPoints.Add(tPowerPoint);
                        tScanArea.ZPosition = Convert.ToDouble(powerPoint.Attribute("ZPosition").Value);
                        tScanArea.Power0 = Convert.ToDouble(powerPoint.Attribute("PowerPercentage").Value);
                    }
                    tScanInfo.ScanAreas.Add(tScanArea);
                }

                ScanConfigure tScanConfigure = new ScanConfigure();
                tScanConfigure._scanMode = Convert.ToInt32(scanConfigure.Attribute("ScanMode").Value);
                tScanConfigure._averageMode = Convert.ToInt32(scanConfigure.Attribute("AverageMode").Value);
                tScanConfigure._numberOfAverageFrame = Convert.ToInt32(scanConfigure.Attribute("NumberOfAverageFrame").Value);
                tScanConfigure._physicalFieldSize = Convert.ToDouble(scanConfigure.Attribute("PhysicalFieldSize").Value);
                tScanConfigure._stripLength = Convert.ToInt32(scanConfigure.Attribute("StripLength").Value);
                tScanConfigure._stripeFieldSize = Convert.ToInt32(scanConfigure.Attribute("StripeFieldSize").Value);
                tScanConfigure._currentPower = Convert.ToDouble(scanConfigure.Attribute("CurrentPower").Value);
                tScanConfigure._isLivingMode = Convert.ToInt32(scanConfigure.Attribute("IsLivingMode").Value);
                tScanConfigure._remapShift = Convert.ToInt32(scanConfigure.Attribute("RemapShift").Value);
                tScanInfo.ScanConfigure = tScanConfigure;
                _mesoParams.TemplateScans.Add(tScanInfo);
            }

            int currentSelectedArea = ScanAreaID;
            ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("mROIList");
            MVMManager.Instance["AreaControlViewModel", "SelectedScanArea"] = currentSelectedArea;
        }

        private void OnTimedEvent(object source, System.Timers.ElapsedEventArgs e)
        {
            CompareAndGenerateActionRequests();
            if (_doCalculation || 0 < _actionReq.Count)
            {
                _ = (Application.Current?.Dispatcher.Invoke(DispatcherPriority.Normal,
             new Action(delegate ()
             {
                 UpdateMicroScanAreaSettings();
             })));
                CalculateAndHandleActionRequests();

                UpdateCaptureAndCheckLines();
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

        void UpdateCaptureAndCheckLines()
        {
            ResourceManagerCS.SetCameraParamString((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_MESO_EXP_PATH, _settingsPath);

            int lines = 0;
            ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_MROI_TOTAL_LINES, ref lines);

            int minLines = 0, maxLines = 0, defaultLines = 0;
            ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_MROI_TOTAL_LINES, ref minLines, ref maxLines, ref defaultLines);
            double percent = ((double)lines / (double)maxLines)*100;
            if (maxLines < lines)
            {
                //TODO: Try changing the pixel densityh to allow all ROIs, and block the new ROI tool
                //MessageBox.Show("There are too many lines in your mROIs, please reduce the pixel density, the number of ROIs, the Z distance between ROIs, or the height of your ROIs");
                //MVMManager.Instance["AreaControlViewModel", "mROIStatusMessage"] = "ERROR - Too many lines in mROI's";
            }
            else
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FORCE_SETTINGS_UPDATE, 1);
                //MVMManager.Instance["AreaControlViewModel", "mROIStatusMessage"] = "OK";
            }
            MVMManager.Instance["AreaControlViewModel", "mROIStatusPercentLines"] = Math.Round(percent, 2);
        }

        /// <summary>
        /// Set micro scan areas' geometry settings by ROIs
        /// </summary>
        private void UpdateMicroScanAreaSettings(List<PowerPoint> passedPowerPoints = null)
        {
            if (!IsmROIAvaliableAndEnabled)
            {
                return;
            }
            if (running2)
            {
                return;
            }
            running2 = true;

            mutex2.WaitOne();
            bool useArgumentPowerPoints = passedPowerPoints?.Count > 0;
            try
            {

                UpdateMicroScanROIs();

                if (TryReloadXDoc(true))
                {
                    XElement q = _pDoc.Root.Element("TemplateScans");
                    if (q == null)
                    {
                        // make sure we don't stay locked if we need to exit
                        ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                        running2 = false;
                        mutex2.ReleaseMutex();
                        return;
                    }
                    double fullFOVPhysicalSizeXUM = 0;
                    double fullFOVPhysicalSizeYUM = 0;

                    if (_mesoParams?.TemplateScans?.Count > 0 && _mesoParams.TemplateScans[0]?.ScanAreas?.Count > 0)
                    {
                        fullFOVPhysicalSizeXUM = _mesoParams.TemplateScans[0].ScanAreas[0].PhysicalSizeXUM;
                        fullFOVPhysicalSizeYUM = _mesoParams.TemplateScans[0].ScanAreas[0].PhysicalSizeYUM;
                    }

                    XElement scanInfo = q.Descendants("ScanInfo").Where(x => (int)MesoScanTypes.Micro == Int32.Parse(x.Attribute("ScanID").Value)).FirstOrDefault();
                    double currentMagnification = (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)1.0];
                    if (null != scanInfo)
                    {
                        XElement scanAreas = scanInfo.Element("ScanAreas");
                        XElement scanConfigure = scanInfo.Element("ScanConfigure");

                        if (((int)MesoScanTypes.Micro == _scanID))
                        {
                            foreach (var scanArea in scanAreas.Descendants("ScanArea"))
                            {
                                scanArea.Attribute("IsEnable").Value = false.ToString();
                            }
                        }

                        List<PowerPoint> PowerPoints = new List<PowerPoint>();

                        for (int i = 0; i < _microScanROIs.Count; i++)
                        {
                            XElement scanArea = scanAreas.Descendants("ScanArea").Where(x => _microScanROIs[i]._roiID == Int32.Parse(x.Attribute("ScanAreaID").Value)).FirstOrDefault();

                            if (scanArea == null)
                            {
                                continue;
                            }
                            var powerPoint = scanArea?.Element("PowerPoints")?.Descendants("PowerPoint")?.First();
                            if (powerPoint == null)
                            {
                                continue;
                            }
                            double zPos = Convert.ToDouble(powerPoint.Attribute("ZPosition").Value);

                            double powerPercent = Convert.ToDouble(powerPoint.Attribute("PowerPercentage").Value);
                            PowerPoints.Add(new PowerPoint() { PowerPercentage0 = powerPercent, ZPosition = zPos });
                        }

                        if (0 < _microScanROIs.Count && (int)MesoScanTypes.Micro == _scanID)
                        {
                            scanAreas.Descendants("ScanArea").Remove();
                            double yScale = _params["LSMScaleYScan"];
                            double umPerPixelX = _params["mROIStripePhysicalFieldSizeUM"] / _params["mROIStripePixels"];
                            double umPerPixelY = umPerPixelX * yScale;

                            double fullFOVumPerPixelX = _params["FullFOVPhysicalFieldSizeUM"] / _params["FullFOVStripePixels"];
                            double fullFOVumPerPixelY = fullFOVumPerPixelX * yScale;


                            for (int i = 0; i < _microScanROIs.Count; i++)
                            {
                                AddScanArea(ref scanAreas, _microScanROIs[i]._roiID);
                                XElement scanArea = scanAreas.Descendants("ScanArea").Where(x => _microScanROIs[i]._roiID == Int32.Parse(x.Attribute("ScanAreaID").Value)).FirstOrDefault();
                                XElement powerPoints = scanArea.Element("PowerPoints");

                                if (useArgumentPowerPoints && passedPowerPoints?.Count > i && passedPowerPoints[i] != null)
                                {
                                    AddPowerPoint(ref powerPoints, passedPowerPoints[i].ZPosition, passedPowerPoints[i].PowerPercentage0);
                                }
                                else
                                {
                                    //need to retrieve current power for other areas from xml somehow
                                    if (_scanAreaID == _microScanROIs[i]._roiID && (int)MesoScanTypes.Meso != _scanID)
                                    {
                                        AddPowerPoint(ref powerPoints, _params["ZPosition"], _params["CurrentPower0"]);
                                    }
                                    else
                                    {
                                        if (PowerPoints?.Count > i && PowerPoints[i] != null)
                                        {
                                            double zPos = PowerPoints[i].ZPosition;

                                            double powerPercent = PowerPoints[i].PowerPercentage0;
                                            AddPowerPoint(ref powerPoints, zPos, powerPercent);
                                        }
                                        else
                                        {
                                            AddPowerPoint(ref powerPoints, _params["ZPosition"], _params["CurrentPower0"]);
                                        }
                                    }
                                }

                                scanInfo.Attribute("XPixelSize").Value = umPerPixelX.ToString();
                                scanInfo.Attribute("YPixelSize").Value = umPerPixelY.ToString();

                                scanInfo.Attribute("MaxSizeInUMForMaxFieldSize").Value = ((double)MVMManager.Instance["AreaControlViewModel", "LSMMaxFieldSizeXUM", (object)0]).ToString();

                                scanConfigure.Attribute("PhysicalFieldSize").Value = _params["mROIStripePhysicalFieldSizeUM"].ToString();
                                scanConfigure.Attribute("StripeFieldSize").Value = _params["mROIStripeFieldSize"].ToString();
                                scanConfigure.Attribute("StripLength").Value = _params["mROIStripePixels"].ToString();
                                scanInfo.Attribute("TileWidth").Value = _params["mROIStripePixels"].ToString();
                                scanInfo.Attribute("TileHeight").Value = _params["mROIStripePixels"].ToString();

                                double[] pixelXY = new double[2];
                                if (!mROISpatialDisplay)
                                {

                                    int newPixels = (int)(Math.Round((double)_params["FullFOVStripePixels"] * (double)_params["mROIStripePhysicalFieldSizeUM"] / (double)_params["FullFOVPhysicalFieldSizeUM"] / 1.0) * 1.0);

                                    int mults = (int)(Math.Round(_microScanROIs[i]._bound.Size.Width / newPixels));
                                    mults = mults == 0 ? 1 : mults;
                                    int multsH = (int)(_microScanROIs[i]._bound.Size.Height * mults * _params["mROIStripePixels"] / (int)_microScanROIs[i]._bound.Size.Width);

                                    pixelXY = ValidateImageSize(new double[2] { mults * _params["mROIStripePixels"], 4.0 * Math.Ceiling(multsH / 4.0) });
                                }
                                else
                                {
                                    fullFOVumPerPixelX = umPerPixelX;

                                    int newPixels = (int)_params["mROIStripePixels"];

                                    int mults = (int)Math.Round(_microScanROIs[i]._bound.Size.Width / newPixels);
                                    mults = mults < 1 ? 1 : mults;
                                    int multsH = (int)(_microScanROIs[i]._bound.Size.Height * mults * _params["mROIStripePixels"] / (int)_microScanROIs[i]._bound.Size.Width);

                                    pixelXY = ValidateImageSize(new double[2] { mults * _params["mROIStripePixels"], 4.0 * Math.Ceiling(multsH / 4.0) });
                                }
                                scanArea.Attribute("SizeX").Value = ((int)pixelXY[0]).ToString();
                                scanArea.Attribute("SizeY").Value = ((int)pixelXY[1]).ToString();
                                double[] sizeUM = { pixelXY[0] * umPerPixelX, pixelXY[1] * umPerPixelY };
                                scanArea.Attribute("PhysicalSizeX").Value = sizeUM[0].ToString();
                                scanArea.Attribute("PhysicalSizeY").Value = sizeUM[1].ToString();

                                double[] sizeFullFOVUM = { fullFOVPhysicalSizeXUM, fullFOVPhysicalSizeYUM };

                                double[] position = { -sizeFullFOVUM[0] * 0.5 + _params["mROIStripePhysicalFieldSizeUM"] * 0.5, -sizeFullFOVUM[1] * 0.5 };

                                double[] roiTL = new double[2];
                                int mROIScanIndex = 1;
                                if (mROISpatialDisplay && mROIScanIndex < _mesoParams?.TemplateScans?.Count && _mesoParams.TemplateScans[mROIScanIndex].PixelSize != null)
                                {
                                    roiTL[0] = Math.Floor(_microScanROIs[i]._bound.TopLeft.X) * _mesoParams.TemplateScans[mROIScanIndex].PixelSize.X;
                                    roiTL[1] = Math.Floor(_microScanROIs[i]._bound.TopLeft.Y) * _mesoParams.TemplateScans[mROIScanIndex].PixelSize.Y;
                                }
                                else
                                {
                                    roiTL[0] = Math.Floor(_microScanROIs[i]._bound.TopLeft.X) * fullFOVumPerPixelX;
                                    roiTL[1] = Math.Floor(_microScanROIs[i]._bound.TopLeft.Y) * fullFOVumPerPixelY;
                                }
                                double stripeSize = _params["mROIStripePhysicalFieldSizeUM"];

                                int nStripes = (int)Math.Round(sizeUM[0] / stripeSize);

                                int horizontalFlip = (int)_params["LSMFlipHorizontal"];
                                double xp = 0;
                                if (horizontalFlip != 0)
                                {
                                    int width = (int)Math.Floor(_microScanROIs[i]._bound.Width);
                                    double pixelXPos = (Math.Floor(_microScanROIs[i]._bound.BottomRight.X) - width / nStripes);
                                    if (mROISpatialDisplay && mROIScanIndex < _mesoParams?.TemplateScans?.Count && _mesoParams.TemplateScans[mROIScanIndex].PixelSize != null)
                                    {
                                        roiTL[0] = pixelXPos * _mesoParams.TemplateScans[mROIScanIndex].PixelSize.X;
                                    }
                                    else
                                    {
                                        roiTL[0] = pixelXPos * fullFOVumPerPixelX;
                                    }
                                    xp = -1 * (position[0] + roiTL[0]);
                                }
                                else
                                {
                                    xp = position[0] + roiTL[0];
                                }
                                int verticalFlip = (int)_params["LSMFlipVerticalScan"];
                                double yp = 0;
                                if (verticalFlip != 0)
                                {
                                    int width = (int)Math.Floor(_microScanROIs[i]._bound.Width);
                                    double pixelYPos = (Math.Floor(_microScanROIs[i]._bound.BottomRight.Y));
                                    if (mROISpatialDisplay && mROIScanIndex < _mesoParams?.TemplateScans?.Count && _mesoParams.TemplateScans[mROIScanIndex].PixelSize != null)
                                    {
                                        roiTL[1] = pixelYPos * _mesoParams.TemplateScans[mROIScanIndex].PixelSize.Y;
                                    }
                                    else
                                    {
                                        roiTL[1] = pixelYPos * fullFOVumPerPixelY;
                                    }
                                    yp = -1 * (position[1] + roiTL[1]);
                                }
                                else
                                {
                                    yp = position[1] + roiTL[1];
                                }

                                scanArea.Attribute("PositionX").Value = xp.ToString("F4");
                                scanArea.Attribute("PositionY").Value = yp.ToString("F4");
                            }
                        }
                        else if (0 >= _microScanROIs.Count && OverlayManagerClass.Instance.ROIsLoaded)
                        {
                            scanAreas.Descendants("ScanArea").Remove();
                        }
                        MapXml2MesoParams(ref q);

                        MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS] = XmlManager.ToXmlDocument(_pDoc);
                        MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                    }
                    ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                }
            }
            catch (Exception ex)
            {
                // make sure we don't stay locked if we need to exit
                ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                ex.ToString();
            }

            running2 = false;
            mutex2.ReleaseMutex();
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
                _bound = new Rect(((ROIRect)area).StatsStartPoint, ((ROIRect)area).StatsEndPoint),
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
            double imageSize = (int)retVal[0] * (int)retVal[1] * (3.0 < _params["LSMChannel"] ? 4.0 : 1.0) * sizeof(short);
            if (MAX_IMAGE_SIZE < imageSize)
            {
                retVal[0] = (double)MAX_IMAGE_SIZE / ((int)retVal[1] * (3.0 < _params["LSMChannel"] ? 4.0 : 1.0) * sizeof(short));
                retVal[1] = retVal[0] * aspectRatio;
            }
            return retVal;
        }

        #endregion Methods
    }
}