namespace CaptureOptionsControl.ViewModel
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

    using CaptureOptionsControl.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public class CaptureOptionsControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly CaptureOptionsControlModel _CaptureOptionsControlModel;

        bool _bleachControlActive = false;
        private int _bleachFrames;
        int _captureMode = 0;
        private int _enableSimultaneous = 0;
        bool _fastzActive = false;
        private int _fastZFlybackFrames = 0;
        bool _hyperSpectralCaptureActive = false;
        private int _postBleachFrames1 = 0;
        private int _postBleachFrames2 = 0;
        private double _postBleachInterval1 = 0;
        private double _postBleachInterval2 = 0;
        private int _preBleachFrames;
        private double _preBleachInterval = 0;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private int _stimulusMaxFrames = 0;
        private int _streamFrames = 0;
        private int _tFrames = 0;
        private double _tInterval = 0;
        private bool _zfastEnable = false;
        private int _zfastMode = 0;

        #endregion Fields

        #region Constructors

        public CaptureOptionsControlViewModel()
        {
            this._CaptureOptionsControlModel = new CaptureOptionsControlModel();
        }

        #endregion Constructors

        #region Properties

        public bool BleachControlActive
        {
            get
            {
                return _bleachControlActive;
            }
            set
            {
                _bleachControlActive = value;
                OnPropertyChanged("BleachControlActive");
            }
        }

        public int BleachFrames
        {
            get
            {
                return _bleachFrames;
            }
            set
            {
                _bleachFrames = value;
                OnPropertyChanged("BleachFrames");
            }
        }

        public Visibility BleachingCaptureModeVis
        {
            get;
            set;
        }

        public int BleachPostTrigger
        {
            set
            {
                switch (value)
                {
                    case 0:
                        this.BleachPostTriggerStr = "SW Trigger";
                        break;
                    case 1:
                        this.BleachPostTriggerStr = "HW TrigFirst";
                        break;
                }
                OnPropertyChanged("BleachPostTriggerStr");
            }
        }

        public string BleachPostTriggerStr
        {
            get;
            set;
        }

        public int BleachTrigger
        {
            set
            {
                switch (value)
                {
                    case 0:
                        this.BleachTriggerStr = "SW Trigger";
                        break;
                    case 1:
                        this.BleachTriggerStr = "HW TrigFirst";
                        break;
                    case 2:
                        this.BleachTriggerStr = "HW TrigEach";
                        break;
                }
                OnPropertyChanged("BleachTriggerStr");
            }
        }

        public string BleachTriggerStr
        {
            get;
            set;
        }

        public int CaptureMode
        {
            get
            {
                return _captureMode;
            }
            set
            {
                _captureMode = value;
                switch (value)
                {
                    case (int)CaptureModes.T_AND_Z:
                        {
                            this.TSeriesCaptureModeVis = Visibility.Visible;
                            this.StreamingCaptureModeVis = Visibility.Collapsed;
                            this.BleachingCaptureModeVis = Visibility.Collapsed;
                            this.HyperspectralModeVis = Visibility.Collapsed;
                            CaptureModeStr = "Z and T";
                        }
                        break;
                    case (int)CaptureModes.STREAMING:
                        {
                            this.TSeriesCaptureModeVis = Visibility.Collapsed;
                            this.StreamingCaptureModeVis = Visibility.Visible;
                            this.BleachingCaptureModeVis = Visibility.Collapsed;
                            this.HyperspectralModeVis = Visibility.Collapsed;
                            CaptureModeStr = "Streaming";
                        }
                        break;
                    case (int)CaptureModes.BLEACHING:
                        {
                            this.TSeriesCaptureModeVis = Visibility.Collapsed;
                            this.StreamingCaptureModeVis = Visibility.Collapsed;
                            this.BleachingCaptureModeVis = Visibility.Visible;
                            this.HyperspectralModeVis = Visibility.Collapsed;
                            CaptureModeStr = "Bleaching";
                        }
                        break;
                    case (int)CaptureModes.HYPERSPECTRAL:
                        {
                            this.TSeriesCaptureModeVis = Visibility.Collapsed;
                            this.StreamingCaptureModeVis = Visibility.Collapsed;
                            this.BleachingCaptureModeVis = Visibility.Collapsed;
                            this.HyperspectralModeVis = Visibility.Visible;
                            CaptureModeStr = "Hyperspectral";
                        }
                        break;
                    default:
                        {
                            this.TSeriesCaptureModeVis = Visibility.Collapsed;
                            this.StreamingCaptureModeVis = Visibility.Collapsed;
                            this.BleachingCaptureModeVis = Visibility.Collapsed;
                            this.HyperspectralModeVis = Visibility.Collapsed;
                            CaptureModeStr = "NA";
                        }
                        break;
                }
                OnPropertyChanged("TSeriesCaptureModeVis");
                OnPropertyChanged("StreamingCaptureModeVis");
                OnPropertyChanged("BleachingCaptureModeVis");
                OnPropertyChanged("HyperspectralModeVis");
                OnPropertyChanged("CaptureModeStr");
                OnPropertyChanged("CaptureMode");
            }
        }

        public string CaptureModeStr
        {
            get;
            set;
        }

        public int DataType
        {
            set
            {
                switch (value)
                {
                    case 0:
                        this.DataTypeStr = "Tiff";
                        break;
                    case 1:
                        this.DataTypeStr = "Raw";
                        break;
                    case 2:
                        this.DataTypeStr = "OME Tiff";
                        break;
                }
                OnPropertyChanged("DataTypeStr");
            }
        }

        public string DataTypeStr
        {
            get;
            set;
        }

        public string EnabledPMTs
        {
            get;
            set;
        }

        public int EnableSimultaneous
        {
            get
            {
                return _enableSimultaneous;
            }
            set
            {
                _enableSimultaneous = value;
                if (value == 1)
                {
                    SimultaneousEnabledStr = "Simultaneous";
                    PostBleach1Label = "Stimulation Img.";
                    PostBleach2Label = "Post Stimulation";
                }
                else
                {
                    SimultaneousEnabledStr = string.Empty;
                    EnabledPMTs = string.Empty;
                    PostBleach1Label = "Post Stimulation 1";
                    PostBleach2Label = "Post Stimulation 2";
                }
                OnPropertyChanged("SimultaneousEnabledStr");
                OnPropertyChanged("EnabledPMTs");
                OnPropertyChanged("PostBleach1Label");
                OnPropertyChanged("PostBleach2Label");
            }
        }

        public bool FastZActive
        {
            get
            {
                return _fastzActive;
            }
            set
            {
                _fastzActive = value;
                OnPropertyChanged("FastZActive");
            }
        }

        public string FastZEnabledDisabledStr
        {
            get
            {
              return (ZFastEnable) ? "Enabled" : "Disabled";
            }
        }

        public int FastZFlybackFrames
        {
            get
            {
                return _fastZFlybackFrames;
            }
            set
            {
                _fastZFlybackFrames = value;
                OnPropertyChanged("StreamVolumes");
            }
        }

        public Visibility FiniteStreamingVis
        {
            get;
            set;
        }

        public bool HyperSpectralCaptureActive
        {
            get
            {
                return _hyperSpectralCaptureActive;
            }
            set
            {
                _hyperSpectralCaptureActive = value;
                OnPropertyChanged("HyperSpectralCaptureActive");
            }
        }

        public Visibility HyperspectralModeVis
        {
            get;
            set;
        }

        public int KuriousSequenceSteps
        {
            get
            {
                return (int)MVMManager.Instance["KuriosControlViewModel", "KuriousSequenceSteps", (object)0];
            }
            set
            {
                MVMManager.Instance["KuriosControlViewModel", "KuriousSequenceSteps", (object)0] = value;
                OnPropertyChanged("KuriousSequenceSteps");
            }
        }

        public string PostBleach1Label
        {
            get;
            set;
        }

        public string PostBleach2Label
        {
            get;
            set;
        }

        public int PostBleachFrames1
        {
            get
            {
                return _postBleachFrames1;
            }
            set
            {
                _postBleachFrames1 = value;
                OnPropertyChanged("PostBleachFrames1");
            }
        }

        public int PostBleachFrames2
        {
            get
            {
                return _postBleachFrames2;
            }
            set
            {
                _postBleachFrames2 = value;
                OnPropertyChanged("PostBleachFrames2");
            }
        }

        public double PostBleachInterval1
        {
            get
            {
                return _postBleachInterval1;
            }
            set
            {
                _postBleachInterval1 = value;
                OnPropertyChanged("PostBleachInterval1");
            }
        }

        public double PostBleachInterval2
        {
            get
            {
                return _postBleachInterval2;
            }
            set
            {
                _postBleachInterval2 = value;
                OnPropertyChanged("PostBleachInterval2");
            }
        }

        public string PostBleachMode1
        {
            get;
            set;
        }

        public string PostBleachMode2
        {
            get;
            set;
        }

        public int PostBleachStream1
        {
            set
            {
                this.PostBleachMode1 = (0 == value) ? "Timelapse" : "Stream";
                OnPropertyChanged("PostBleachMode1");
            }
        }

        public int PostBleachStream2
        {
            set
            {
                this.PostBleachMode2 = (0 == value) ? "Timelapse" : "Stream";
                OnPropertyChanged("PostBleachMode2");
            }
        }

        public int PreBleachFrames
        {
            get
            {
                return _preBleachFrames;
            }
            set
            {
                _preBleachFrames = value;
                OnPropertyChanged("PreBleachFrames");
            }
        }

        public double PreBleachInterval
        {
            get
            {
                return _preBleachInterval;
            }
            set
            {
                _preBleachInterval = value;
                OnPropertyChanged("PreBleachInterval");
            }
        }

        public string PreBleachMode
        {
            get;
            set;
        }

        public int PreBleachStream
        {
            set
            {
                this.PreBleachMode = (0 == value) ? "Timelapse" : "Stream";
                OnPropertyChanged("PreBleachMode");
            }
        }

        public string SimultaneousEnabledStr
        {
            get;
            set;
        }

        public string StaircaseEnabledDisabledStr
        {
            get
            {
                return (2 == ZFastMode) ? "Enabled" : "Disabled";
            }
        }

        public int StimulusMaxFrames
        {
            get
            {
                return _stimulusMaxFrames;
            }
            set
            {
                _stimulusMaxFrames = value;
                OnPropertyChanged("StimulusMaxFrames");
            }
        }

        public Visibility StimulusStreamingVis
        {
            get;
            set;
        }

        public int StreamFrames
        {
            get
            {
                return _streamFrames;
            }
            set
            {
                _streamFrames = value;
                OnPropertyChanged("StreamFrames");
                OnPropertyChanged("StreamVolumes");
            }
        }

        public Visibility StreamingCaptureModeVis
        {
            get;
            set;
        }

        public int StreamingStorageMode
        {
            set
            {
                const int FINITE = 0;
                const int STIMULUS = 1;
                switch (value)
                {
                    case FINITE:
                        this.FiniteStreamingVis = Visibility.Visible;
                        this.StimulusStreamingVis = Visibility.Collapsed;
                        this.StreamingStorageModeStr = "Finite";
                        break;
                    case STIMULUS:
                        this.FiniteStreamingVis = Visibility.Collapsed;
                        this.StimulusStreamingVis = Visibility.Visible;
                        this.StreamingStorageModeStr = "Stimulus";
                        break;
                }
                OnPropertyChanged("FiniteStreamingVis");
                OnPropertyChanged("StimulusStreamingVis");
                OnPropertyChanged("StreamingStorageModeStr");
            }
        }

        public string StreamingStorageModeStr
        {
            get;
            set;
        }

        public int StreamVolumes
        {
            get
            {
                if (0 != (double)(((int)MVMManager.Instance["ZControlViewModel", "ZScanNumSteps", (object)0]) + this.FastZFlybackFrames))
                {
                    return Convert.ToInt32(Math.Round(this.StreamFrames / (double)(((int)MVMManager.Instance["ZControlViewModel", "ZScanNumSteps", (object)0]) + this.FastZFlybackFrames)));
                }
                return 0;
            }
        }

        public int TFrames
        {
            get
            {
                return _tFrames;
            }
            set
            {
                _tFrames = value;
                OnPropertyChanged("TFrames");
            }
        }

        public double TInterval
        {
            get
            {
                return _tInterval;
            }
            set
            {
                _tInterval = value;
                OnPropertyChanged("TInterval");
            }
        }

        public int TriggerModeStreaming
        {
            set
            {
                const int SOFTWARE_MULTIFRAME = 1;
                const int HARDWARE_MULTIFRAME_TRIGGER_FIRST = 4;
                const int HARDWARE_MULTIFRAME_TRIGGER_EACH = 5;
                const int HARDWARE_MULTIFRAME_TRIGGER_EACH_BULB = 6;
                switch (value)
                {
                    case SOFTWARE_MULTIFRAME:
                        {
                            this.TriggerModeStreamingStr = "None";
                        }
                        break;
                    case HARDWARE_MULTIFRAME_TRIGGER_FIRST:
                        {
                            this.TriggerModeStreamingStr = "Trigger First";
                        }
                        break;
                    case HARDWARE_MULTIFRAME_TRIGGER_EACH:
                        {
                            this.TriggerModeStreamingStr = "Trigger Each";
                        }
                        break;
                    case HARDWARE_MULTIFRAME_TRIGGER_EACH_BULB:
                        {
                            this.TriggerModeStreamingStr = "Trigger Bulb";
                        }
                        break;
                }
                OnPropertyChanged("TriggerModeStreamingStr");
            }
        }

        public string TriggerModeStreamingStr
        {
            get;
            set;
        }

        public int TriggerModeTimelapse
        {
            set
            {
                switch (value)
                {
                    case 0:
                        this.TriggerModeTimelapseStr = "None";
                        break;
                    case 1:
                        this.TriggerModeTimelapseStr = "Trigger First";
                        break;
                    case 2:
                        this.TriggerModeTimelapseStr = "Trigger Each";
                        break;
                }
                OnPropertyChanged("TriggerModeTimelapseStr");
            }
        }

        public string TriggerModeTimelapseStr
        {
            get;
            set;
        }

        public Visibility TSeriesCaptureModeVis
        {
            get;
            set;
        }

        public bool ZFastEnable
        {
            get
            {
                if (FastZActive)
                {
                    return _zfastEnable;
                }
                else
                {
                    return false;
                }
            }
            set
            {
                _zfastEnable = value;
                OnPropertyChanged("ZFastEnable");
                OnPropertyChanged("FastZEnabledDisabledStr");
            }
        }

        public int ZFastMode
        {
            get
            {
                return _zfastMode;
            }
            set
            {
                _zfastMode = value;
                OnPropertyChanged("ZFastMode");
                OnPropertyChanged("StaircaseEnabledDisabledStr");
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
                myPropInfo = typeof(CaptureOptionsControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            XmlDocument ExperimentDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
            EnabledPMTs = string.Empty;

            XmlNodeList ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/CaptureMode");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                int tmp = 0;
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "mode", ref str))
                {
                    if (Int32.TryParse(str, out tmp))
                    {
                        CaptureMode = tmp;
                    }
                }
            }

            ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Timelapse");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "timepoints", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        TFrames = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "intervalSec", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        TInterval = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "triggerMode", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        TriggerModeTimelapse = tmp;
                    }
                }
            }

            ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Streaming");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "frames", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        StreamFrames = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "storageMode", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        StreamingStorageMode = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "triggerMode", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        TriggerModeStreaming = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "zFastEnable", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        ZFastEnable = (1 == tmp);
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "flybackFrames", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        FastZFlybackFrames = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "stimulusMaxFrames", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        StimulusMaxFrames = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "rawData", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        DataType = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "zFastMode", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        ZFastMode = tmp;
                    }
                }
            }

            ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Photobleaching");
            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "bleachTrigger", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        BleachTrigger = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "bleachPostTrigger", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        BleachPostTrigger = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "bleachFrames", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        BleachFrames = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "preBleachFrames", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PreBleachFrames = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "preBleachInterval", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PreBleachInterval = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "preBleachStream", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PreBleachStream = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "postBleachFrames1", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PostBleachFrames1 = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "postBleachInterval1", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PostBleachInterval1 = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "postBleachStream1", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PostBleachStream1 = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "postBleachFrames2", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                       PostBleachFrames2 = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "postBleachInterval2", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PostBleachInterval2 = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "postBleachStream2", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PostBleachStream2 = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "pmt1EnableDuringBleach", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        if (1 == tmp)
                        {
                            EnabledPMTs += " A";
                        }
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "pmt2EnableDuringBleach", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        if (1 == tmp)
                        {
                            EnabledPMTs += " B";
                        }
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "pmt3EnableDuringBleach", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        if (1 == tmp)
                        {
                            EnabledPMTs += " C";
                        }
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "pmt4EnableDuringBleach", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        if (1 == tmp)
                        {
                            EnabledPMTs += " D";
                        }
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "EnableSimultaneous", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        EnableSimultaneous = tmp;
                    }
                }
            }
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChange(propertyName);
            }
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/CaptureMode");
            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], experimentFile, "mode", CaptureMode.ToString());
            }
        }

        #endregion Methods
    }
}