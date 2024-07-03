namespace DFLIMControl.ViewModel
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
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Xml;

    using DFLIMControl.Model;

    using DFLIMSetupAssistant;

    using FLIMFitting.View;

    using ThorLogging;

    using ThorSharedTypes;

    public class DFLIMControlViewModelBase : VMBase
    {
        #region Fields

        protected Color[] _channelsColors;
        protected string _currentViewModel;
        protected Dictionary<KeyValuePair<int, SolidColorBrush>, uint[]> _dflimHistogramDictionary;
        protected long _diagnosticsDataCopyCounter = 0;
        protected bool _diagnosticsDataReady = false;
        protected FlimFittingWindow _flimFittingWindow = null;
        protected long _histogramCopyCounter = 0;
        protected bool _histogramDataReady = false;
        protected string _imageViewMVMName;
        protected string _mainViewModelName;

        const long NUM_CHANNELS = 4;

        private static readonly object _dflimDiagnosticsMVMDataLock = new object();
        private static readonly object _dflimHistogramMVMDataLock = new object();

        private readonly int[] _currentCoarseShift = new int[NUM_CHANNELS];
        private readonly int[] _currentFineShift = new int[NUM_CHANNELS];

        private Dictionary<KeyValuePair<int, SolidColorBrush>, ushort[]> _dflimDiagnosticsDataDictionary;
        private ICommand _DFLIMDisplayFitCommand;
        private bool _dflimDisplayLifetimeImage = true;
        private Visibility _fitButtonVisibility = Visibility.Collapsed;
        private Visibility _hwControlsVisibility = Visibility.Collapsed;
        private bool _loadingSettings = false;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();

        #endregion Fields

        #region Constructors

        public DFLIMControlViewModelBase()
        {
            DFLIMTauHigh = new CustomCollection<float>();
            DFLIMTauLow = new CustomCollection<float>();
            DFLIMLUTHigh = new CustomCollection<uint>();
            DFLIMLUTLow = new CustomCollection<uint>();
            DFLIMTZero = new CustomCollection<float>();

            for (int i = 0; i < NUM_CHANNELS; i++)
            {
                DFLIMTauHigh.Add(3f);
                DFLIMTauLow.Add(1f);
                DFLIMLUTHigh.Add(25);
                DFLIMLUTLow.Add(5);
                DFLIMTZero.Add(0.8f);
            }
            DFLIMTauHigh.CollectionChanged += LifetimeImageSettingChanged;
            DFLIMTauLow.CollectionChanged += LifetimeImageSettingChanged;
            DFLIMLUTHigh.CollectionChanged += LifetimeImageSettingChanged;
            DFLIMLUTLow.CollectionChanged += LifetimeImageSettingChanged;
            DFLIMTZero.CollectionChanged += LifetimeImageSettingChanged;

            DFLIMTZero.CollectionChanged += DFLIMTZero_CollectionChanged;
        }

        #endregion Constructors

        #region Properties

        public bool ClosePropertyWindows
        {
            set
            {
                if (value)
                {
                    if (null != _flimFittingWindow)
                    {
                        _flimFittingWindow.Close();
                    }
                }
            }
        }

        public int DFLIMDiagnosticsBufferLengthSelectedIndex
        {
            get
            {
                if (string.IsNullOrWhiteSpace(_mainViewModelName))
                {
                    return -1;
                }
                if ((int)MVMManager.Instance[_mainViewModelName, "DFLIMDiagnosticsBufferLength"] == 128)
                {
                    return 0;
                }
                else if ((int)MVMManager.Instance[_mainViewModelName, "DFLIMDiagnosticsBufferLength"] == 2048)
                {
                    return 1;
                }
                return -1;
            }
            set
            {
                if (string.IsNullOrWhiteSpace(_mainViewModelName))
                {
                    return;
                }
                MVMManager.Instance[_mainViewModelName, "DFLIMDiagnosticsBufferLength"] = value == 0 ? 128 : 2048;
                OnPropertyChanged("DFLIMDiagnosticsBufferLengthSelectedIndex");
            }
        }

        public long DFLIMDiagnosticsDataCopyCounter
        {
            get
            {
                return _diagnosticsDataCopyCounter;
            }
        }

        public Dictionary<KeyValuePair<int, SolidColorBrush>, ushort[]> DFLIMDiagnosticsDataDictionary
        {
            get
            {
                return _dflimDiagnosticsDataDictionary;
            }
        }

        public object DFLIMDiagnosticsMVMDataLock
        {
            get
            {
                return _dflimDiagnosticsMVMDataLock;
            }
        }

        public ICommand DFLIMDisplayFitCommand
        {
            get
            {
                if (_DFLIMDisplayFitCommand == null)
                {
                    _DFLIMDisplayFitCommand = new RelayCommand(() => DFLIMFLIMFitting());
                }

                return _DFLIMDisplayFitCommand;
            }
        }

        public bool DFLIMDisplayLifetimeImage
        {
            get
            {
                return _dflimDisplayLifetimeImage;
            }
            set
            {
                if (string.IsNullOrWhiteSpace(_mainViewModelName))
                {
                    return;
                }
                _dflimDisplayLifetimeImage = value;
                OnPropertyChanged("DFLIMDisplayLifetimeImage");
                MVMManager.Instance[_mainViewModelName, "RebuildBitmap"] = true;
                ((ThorSharedTypes.IMVM)MVMManager.Instance[_mainViewModelName, this]).OnPropertyChange("Bitmap");
            }
        }

        public Visibility DFLIMFitVisibility
        {
            get
            {
                return _fitButtonVisibility;
            }
            set
            {
                _fitButtonVisibility = value;
                OnPropertyChange("DFLIMFitVisibility");
            }
        }

        public long DFLIMHistogramCopyCounter
        {
            get
            {
                return _histogramCopyCounter;
            }
        }

        public Dictionary<KeyValuePair<int, SolidColorBrush>, uint[]> DFLIMHistogramDictionary
        {
            get
            {
                return _dflimHistogramDictionary;
            }
        }

        public object DFLIMHistogramMVMDataLock
        {
            get
            {
                return _dflimHistogramMVMDataLock;
            }
        }

        public Visibility DFLIMHWControlsVisibility
        {
            get
            {
                return _hwControlsVisibility;
            }
            set
            {
                _hwControlsVisibility = value;
                OnPropertyChanged("DFLIMHWControlsVisibility");
            }
        }

        public CustomCollection<uint> DFLIMLUTHigh
        {
            get;
            set;
        }

        public CustomCollection<uint> DFLIMLUTLow
        {
            get;
            set;
        }

        public CustomCollection<float> DFLIMTauHigh
        {
            get;
            set;
        }

        public CustomCollection<float> DFLIMTauLow
        {
            get;
            set;
        }

        public CustomCollection<float> DFLIMTZero
        {
            get;
            set;
        }

        public bool DiagnosticsDataReady
        {
            get
            {
                return _diagnosticsDataReady;
            }
            set
            {
                _diagnosticsDataReady = value;
            }
        }

        public bool HistogramDataReady
        {
            get
            {
                return _histogramDataReady;
            }
            set
            {
                _histogramDataReady = value;
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
                myPropInfo = typeof(DFLIMControlViewModelBase).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            _loadingSettings = true;
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/DFLIM");
            string str = string.Empty;
            int tmpInt = 0;
            uint tmpUInt = 0;
            float tmpFloat = 0.0f;
            if (ndList.Count > 0)
            {
                if (XmlManager.GetAttribute(ndList[0], doc, "displayLifetimeImage", ref str) && (Int32.TryParse(str, out tmpInt)))
                {
                    this.DFLIMDisplayLifetimeImage = 1 == tmpInt;
                }
                else
                {
                    this.DFLIMDisplayLifetimeImage = false;
                }

                for (int i = 0; i < NUM_CHANNELS; ++i)
                {
                    if (XmlManager.GetAttribute(ndList[0], doc, "tauHigh" + i, ref str) && (float.TryParse(str, out tmpFloat)))
                    {
                        this.DFLIMTauHigh[i] = tmpFloat;
                    }
                    else
                    {
                        this.DFLIMTauHigh[i] = 3;
                    }

                    if (XmlManager.GetAttribute(ndList[0], doc, "tauLow" + i, ref str) && (float.TryParse(str, out tmpFloat)))
                    {
                        this.DFLIMTauLow[i] = tmpFloat;
                    }
                    else
                    {
                        this.DFLIMTauLow[i] = 1;
                    }

                    if (XmlManager.GetAttribute(ndList[0], doc, "lutHigh" + i, ref str) && (uint.TryParse(str, out tmpUInt)))
                    {
                        this.DFLIMLUTHigh[i] = tmpUInt;
                    }
                    else
                    {
                        this.DFLIMLUTHigh[i] = 25;
                    }

                    if (XmlManager.GetAttribute(ndList[0], doc, "lutLow" + i, ref str) && (uint.TryParse(str, out tmpUInt)))
                    {
                        this.DFLIMLUTLow[i] = tmpUInt;
                    }
                    else
                    {
                        this.DFLIMLUTLow[i] = 25;
                    }

                    if (XmlManager.GetAttribute(ndList[0], doc, "tZero" + i, ref str) && (float.TryParse(str, out tmpFloat)))
                    {
                        this.DFLIMTZero[i] = tmpFloat;
                    }
                    else
                    {
                        this.DFLIMTZero[i] = 0.8f;
                    }
                }
            }
            else
            {
                this.DFLIMDisplayLifetimeImage = false;
            }

            _loadingSettings = false;
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
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/DFLIM");

            if (ndList.Count <= 0)
            {
                CreateXmlNode(experimentFile, "DFLIM");
                ndList = experimentFile.SelectNodes("/ThorImageExperiment/DFLIM");
            }

            if (ndList.Count > 0)
            {
                int temp = this.DFLIMDisplayLifetimeImage ? 1 : 0;
                XmlManager.SetAttribute(ndList[0], experimentFile, "displayLifetimeImage", temp.ToString());
                for (int i = 0; i < NUM_CHANNELS; ++i)
                {
                    XmlManager.SetAttribute(ndList[0], experimentFile, "tauHigh" + i, DFLIMTauHigh[i].ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "tauLow" + i, DFLIMTauLow[i].ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "lutHigh" + i, DFLIMLUTHigh[i].ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "lutLow" + i, DFLIMLUTLow[i].ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "tZero" + i, DFLIMTZero[i].ToString());
                }
            }
        }

        protected void CopyDiagnostics()
        {
            if (string.IsNullOrWhiteSpace(_mainViewModelName))
            {
                return;
            }
            if (null != MVMManager.Instance[_mainViewModelName])
            {
                if ((bool)MVMManager.Instance[_mainViewModelName, "NewDFLIMDiagnosticsData"])
                {
                    ushort[][] dflimDiagnosticsData;

                    lock ((object)MVMManager.Instance[_mainViewModelName, "DFLIMDiagnosticsDataLock"])
                    {
                        MVMManager.Instance[_mainViewModelName, "NewDFLIMDiagnosticsData"] = false;
                        ushort[][] temp = (ushort[][])MVMManager.Instance[_mainViewModelName, "DFLIMDiagnosticsData"];
                        dflimDiagnosticsData = new ushort[temp.Length][];
                        for (int i = 0; i < temp.Length; ++i)
                        {
                            dflimDiagnosticsData[i] = ObjectExtensions.Copy(temp[i]);
                        }
                    }
                    bool[] lsmChannelEnable = (bool[])MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable"];
                    Application.Current.Dispatcher.Invoke(new Action(() =>
                    {
                        _channelsColors = (Color[])MVMManager.Instance[_imageViewMVMName, "DefaultChannelColors"];

                        List<int> channels = new List<int>();
                        for (int i = 0; i < NUM_CHANNELS; ++i)
                        {
                            if (lsmChannelEnable[i])
                            {
                                channels.Add(i);
                            }
                        }

                        lock (_dflimDiagnosticsMVMDataLock)
                        {
                            _dflimDiagnosticsDataDictionary = new Dictionary<KeyValuePair<int, SolidColorBrush>, ushort[]>();
                            for (int i = 0; i < dflimDiagnosticsData.Length; ++i)
                            {
                                var channel = i;
                                if (dflimDiagnosticsData.Length == 1)
                                {
                                    if ((bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable0"])
                                    {
                                        channel = 0;
                                    }
                                    else if ((bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable1"])
                                    {
                                        channel = 1;
                                    }
                                    else if ((bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable2"])
                                    {
                                        channel = 2;
                                    }
                                    else if ((bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable3"])
                                    {
                                        channel = 3;
                                    }
                                }
                            }

                            _dflimDiagnosticsDataDictionary = new Dictionary<KeyValuePair<int, SolidColorBrush>, ushort[]>();
                            for (int i = 0; i < dflimDiagnosticsData.Length; ++i)
                            {
                                if (NUM_CHANNELS == dflimDiagnosticsData.Length)
                                {
                                    if (lsmChannelEnable[i])
                                    {
                                        var channel = new KeyValuePair<int, SolidColorBrush>(i, new SolidColorBrush(_channelsColors[i]));
                                        _dflimDiagnosticsDataDictionary.Add(channel, (ushort[])(object)dflimDiagnosticsData[i]);
                                    }
                                }
                                else
                                {

                                    var channel = new KeyValuePair<int, SolidColorBrush>(channels[i], new SolidColorBrush(_channelsColors[channels[i]]));
                                    _dflimDiagnosticsDataDictionary.Add(channel, (ushort[])(object)dflimDiagnosticsData[i]);
                                }
                            }

                            ++_diagnosticsDataCopyCounter;
                            _diagnosticsDataReady = true;
                        }
                    }));
                }
            }
        }

        protected void CopyHistogram()
        {
            if (string.IsNullOrWhiteSpace(_mainViewModelName))
            {
                return;
            }
            if (null != MVMManager.Instance[_mainViewModelName])
            {
                try
                {
                    if ((bool)MVMManager.Instance[_mainViewModelName, "DFLIMNewHistogramData"])
                    {
                        uint[][] dflimHistogramData;

                        lock ((object)MVMManager.Instance[_mainViewModelName, "DFLIMHistogramDataLock"])
                        {
                            MVMManager.Instance[_mainViewModelName, "DFLIMNewHistogramData"] = false;
                            uint[][] temp = (uint[][])MVMManager.Instance[_mainViewModelName, "DFLIMHistogramData"];
                            dflimHistogramData = new uint[temp.Length][];
                            for (int i = 0; i < temp.Length; ++i)
                            {
                                dflimHistogramData[i] = ObjectExtensions.Copy(temp[i]);
                            }
                        }

                        bool[] lsmChannelEnable = (bool[])MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable"];
                        Application.Current.Dispatcher.Invoke(new Action(() =>
                        {
                            _channelsColors = (Color[])MVMManager.Instance[_imageViewMVMName, "DefaultChannelColors"];

                            List<int> channels = new List<int>();
                            for (int i = 0; i < NUM_CHANNELS; ++i)
                            {
                                if (lsmChannelEnable[i])
                                {
                                    channels.Add(i);
                                }
                            }

                            //FLIMFitting.FLIMHistogramGroupData histogramGroupData = null;
                            lock (_dflimHistogramMVMDataLock)
                            {
                                OnPropertyChanged("DFLIMHistogramMVMDataLock");
                                _dflimHistogramDictionary = new Dictionary<KeyValuePair<int, SolidColorBrush>, uint[]>();
                                for (int i = 0; i < dflimHistogramData.Length; ++i)
                                {
                                    if (NUM_CHANNELS == dflimHistogramData.Length)
                                    {
                                        if (lsmChannelEnable[i])
                                        {
                                            var channel = new KeyValuePair<int, SolidColorBrush>(i, new SolidColorBrush(_channelsColors[i]));
                                            _dflimHistogramDictionary.Add(channel, (dflimHistogramData[i]));
                                        }
                                    }
                                    else
                                    {

                                        var channel = new KeyValuePair<int, SolidColorBrush>(channels[i], new SolidColorBrush(_channelsColors[channels[i]]));
                                        _dflimHistogramDictionary.Add(channel, (dflimHistogramData[i]));

                                        ////to convert from uint to int[] array do the following
                                        //var x =  new Dictionary<KeyValuePair<int, SolidColorBrush>, int[]>();
                                        //x.Add(channel, (int[])(object)dflimHistogramData[i]);
                                    }
                                }
                                ++_histogramCopyCounter;
                                _histogramDataReady = true;
                            }
                        }));
                        OnPropertyChanged("DFLIMHistogramMVMDataLock");
                        OnPropertyChanged("DFLIMHistogramCopyCounter");
                        OnPropertyChanged("DFLIMHistogramDictionary");

                        try
                        {
                            if (null != _flimFittingWindow)
                            {
                                _flimFittingWindow.FLIMHistogramGroups = PrepareFLIMFitChannelHistogramData();

                            }
                        }
                        catch (Exception ex)
                        {
                            ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " CopyHistogram inner " + ex.Message);
                            ex.ToString();
                        }
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " CopyHistogram outter " + ex.Message);
                    ex.ToString();
                }
            }
        }

        [DllImport(".\\StatsManager.dll", EntryPoint = "SetTZero")]
        private static extern int SetTZero(double tZero, int channelIndex);

        private void CreateXmlNode(XmlDocument doc, string nodeName)
        {
            XmlNode node = doc.CreateNode(XmlNodeType.Element, nodeName, null);
            doc.DocumentElement.AppendChild(node);
        }

        private void DFLIMFLIMFitting()
        {
            if (null == _flimFittingWindow)
            {
                _flimFittingWindow = new FlimFittingWindow();
            }

            _flimFittingWindow.Closed += _flimFittingWindow_Closed;
            _flimFittingWindow.AutoFitOnce = true;
            _flimFittingWindow.UpdateTZero += _flimFittingWindow_UpdateTZero;
            List<FLIMFitting.FLIMHistogramGroupData> flimFitHistogramDrata;

            flimFitHistogramDrata = PrepareFLIMFitChannelHistogramData();

            if (null != flimFitHistogramDrata)
            {
                _flimFittingWindow.FLIMHistogramGroups = flimFitHistogramDrata;
            }
            _flimFittingWindow.Show();
        }

        void DFLIMTZero_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            for (int i = 0; i < DFLIMTZero.Count; ++i)
            {
                SetTZero((double)DFLIMTZero[i], i);
            }
        }

        void LifetimeImageSettingChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            if (_loadingSettings)
            {
                return;
            }
            UpdateLifeTimeImage();
        }

        private List<FLIMFitting.FLIMHistogramGroupData> PrepareFLIMFitChannelHistogramData()
        {
            lock (_dflimHistogramMVMDataLock)
            {
                if (null == _dflimHistogramDictionary || _dflimHistogramDictionary.Count <= 0)
                {
                    return null;
                }
                var histogramGroupData = new FLIMFitting.FLIMHistogramGroupData();

                histogramGroupData.GroupName = "Channel Full Frame Histograms";
                histogramGroupData.GroupType = FLIMFitting.HistogramGroupType.ChannelHistograms;

                foreach (var dataSet in _dflimHistogramDictionary)
                {
                    histogramGroupData.Colors.Add(dataSet.Key.Value);
                    histogramGroupData.Channels.Add(dataSet.Key.Key);
                    histogramGroupData.Histograms.Add(ObjectExtensions.Copy(dataSet.Value));
                    string channel = string.Empty;
                    switch (dataSet.Key.Key)
                    {
                        case 0: channel = "A"; break;
                        case 1: channel = "B"; break;
                        case 2: channel = "C"; break;
                        case 3: channel = "D"; break;
                        case 4: channel = "E"; break;
                        case 5: channel = "F"; break;
                        default: channel = dataSet.Key.Key.ToString(); break;
                    }
                    histogramGroupData.HistrogramNames.Add(channel);
                }

                return new List<FLIMFitting.FLIMHistogramGroupData> { histogramGroupData };
            }
        }

        void UpdateLifeTimeImage()
        {
            if (string.IsNullOrWhiteSpace(_mainViewModelName))
            {
                return;
            }
            if (_dflimDisplayLifetimeImage)
            {
                MVMManager.Instance[_mainViewModelName, "RebuildBitmap"] = true;
                ((IMVM)MVMManager.Instance[_mainViewModelName, this]).OnPropertyChange("Bitmap");
            }
        }

        void _flimFittingWindow_Closed(object sender, EventArgs e)
        {
            _flimFittingWindow = null;
        }

        void _flimFittingWindow_UpdateTZero(Dictionary<int, double> tZeroDictionary)
        {
            if (null == tZeroDictionary || tZeroDictionary.Count <= 0)
            {
                return;
            }
            _loadingSettings = true;
            foreach (var tZero in tZeroDictionary)
            {
                DFLIMTZero[tZero.Key] = (float)(tZero.Value);
            }
            _loadingSettings = false;

            UpdateLifeTimeImage();
        }

        #endregion Methods
    }
}