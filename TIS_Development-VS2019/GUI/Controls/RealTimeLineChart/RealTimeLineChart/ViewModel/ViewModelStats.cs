namespace RealTimeLineChart.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Xml;
    using System.Xml.Linq;

    using Abt.Controls.SciChart;
    using Abt.Controls.SciChart.Model.DataSeries;
    using Abt.Controls.SciChart.Visuals;

    using global::RealTimeLineChart.Model;

    using ThorLogging;

    public partial class RealTimeLineChartViewModel : ViewModelBase
    {
        #region Fields

        private readonly double[] WATER_VISCOSITY_A = new double[] { 280.68, 511.45, 61.131, 0.459 };
        private readonly double[] WATER_VISCOSITY_B = new double[] { -1.9, -7.7, -19.6, -40 };

        private ICommand _addSpectralChartCommand;
        private ObservableCollection<string> _allDataChannel = new ObservableCollection<string>();
        private ObservableCollection<string> _allSpectralChannel = new ObservableCollection<string>();
        private ObservableCollection<string> _allSpectralPhysicalChannel = new ObservableCollection<string>();
        private ObservableCollection<string> _allVirtualChannel = new ObservableCollection<string>();
        ObservableCollection<bool> _configuration = new ObservableCollection<bool>(new List<bool>(1) { false });
        private double _cursorBottom = 0;
        private double _cursorLeft = 0;
        private double _cursorRight = 0;
        private int _cursorSelectedIndex = 0;
        private double _cursorTop = 0;
        private double _deltaFreqHz = 0;
        private ObservableCollection<string> _enabledDataChannel = new ObservableCollection<string>();
        private ObservableCollection<string> _enabledSpectralChannel = new ObservableCollection<string>();
        private ObservableCollection<string> _enabledSpectralVirtualChannel = new ObservableCollection<string>();
        private ObservableCollection<string> _enabledVirtualChannel = new ObservableCollection<string>();
        private int _freqAverageMode = 0;
        private int _freqAverageNum = 1;
        private int _freqblock = 1;
        private double _freqMax = 1000.0;
        private double _freqMin = 1.0;
        private double _freqSampleSecMax = 1.0;
        private double _freqSampleSecMin = 0.0;
        private bool _isDurationEditing = false;
        private bool _isLimitEditing = false;
        private bool _isLoaded = false;
        private bool _isModeEditing = false;
        private bool _isRateEditing = false;
        private bool _isResolutionEditing = false;
        private bool _isSizeEditing = false;
        private bool _isStackedDisplay = false;
        private double _liveSampleSec = 0.1;
        private string _otmSettingPath = string.Empty;
        private string _settingPath = string.Empty;
        ObservableCollection<bool> _visibilityCollection = new ObservableCollection<bool>(new List<bool>(7) { true, true, true, true, true, true, false });

        #endregion Fields

        #region Properties

        /// <summary>
        /// Gets the set save path command.
        /// </summary>
        /// <value>
        /// The set save path command.
        /// </value>
        public ICommand AddSpectralChartCommand
        {
            get
            {
                if (this._addSpectralChartCommand == null)
                    this._addSpectralChartCommand = new RelayCommand(() => AddSpectralChartCmd());

                return this._addSpectralChartCommand;
            }
        }

        public ObservableCollection<string> AllDataChannel
        {
            get { return _allDataChannel; }
            set
            {
                _allDataChannel = value;
                OnPropertyChanged("AllDataChannel");
            }
        }

        public ObservableCollection<string> AllSpectralChannel
        {
            get { return _allSpectralChannel; }
            set
            {
                _allSpectralChannel = value;
                OnPropertyChanged("AllSpectralChannel");
            }
        }

        public ObservableCollection<string> AllSpectralPhysicalChannel
        {
            get { return _allSpectralPhysicalChannel; }
            set
            {
                _allSpectralPhysicalChannel = value;
                OnPropertyChanged("AllSpectralPhysicalChannel");
            }
        }

        public ObservableCollection<string> AllVirtualChannel
        {
            get { return _allVirtualChannel; }
            set
            {
                _allVirtualChannel = value;
                OnPropertyChanged("AllVirtualChannel");
            }
        }

        /// <summary>
        /// Gets or sets Configuration, [0]:OTM
        /// </summary>
        public ObservableCollection<bool> Configuration
        {
            get
            {
                return _configuration;
            }
            set
            {
                _configuration = value;
                _visibilityCollection[6] = _configuration[0];
                OnPropertyChanged("Configuration");
                OnPropertyChanged("VisibilityCollection");
            }
        }

        public double CursorBottom
        {
            get
            {
                return _cursorBottom;
            }
            set
            {
                _cursorBottom = value;
                OnPropertyChanged("CursorBottom");

            }
        }

        public double CursorLeft
        {
            get
            {
                return _cursorLeft;
            }
            set
            {
                _cursorLeft = value;
                OnPropertyChanged("CursorLeft");

            }
        }

        public double CursorRight
        {
            get
            {
                return _cursorRight;
            }
            set
            {
                _cursorRight = value;
                OnPropertyChanged("CursorRight");

            }
        }

        public int CursorSelectedIndex
        {
            get
            {
                return _cursorSelectedIndex;
            }
            set
            {
                _cursorSelectedIndex = value;
                OnPropertyChanged("CursorSelectedIndex");

            }
        }

        public double CursorTop
        {
            get
            {
                return _cursorTop;
            }
            set
            {
                _cursorTop = value;
                OnPropertyChanged("CursorTop");

            }
        }

        public double DeltaFreqHz
        {
            get { return _deltaFreqHz; }
            set
            {
                _deltaFreqHz = Math.Round(value, 1);
                OnPropertyChanged("DeltaFreqHz");
            }
        }

        /// <summary>
        /// Gets the display resolution.
        /// </summary>
        /// <value>
        /// The display resolution.
        /// </value>
        public string DisplayResolution
        {
            get
            {
                switch (DisplayOptionSelectedIndex)
                {
                    case 0: return "High";
                    case 1: return "Medium";
                    case 2: return "Low";
                    default:
                        return string.Empty;
                }
            }
        }

        public ObservableCollection<string> EnabledDataChannel
        {
            get { return _enabledDataChannel; }
            set
            {
                _enabledDataChannel = value;
                OnPropertyChanged("EnabledDataChannel");
            }
        }

        public ObservableCollection<string> EnabledSpectralChannel
        {
            get { return _enabledSpectralChannel; }
            set
            {
                _enabledSpectralChannel = value;
                OnPropertyChanged("EnabledSpectralChannel");
            }
        }

        public ObservableCollection<string> EnabledSpectralVirtualChannel
        {
            get { return _enabledSpectralVirtualChannel; }
            set
            {
                _enabledSpectralVirtualChannel = value;
                OnPropertyChanged("EnabledSpectralVirtualChannel");
            }
        }

        public ObservableCollection<string> EnabledVirtualChannel
        {
            get { return _enabledVirtualChannel; }
            set
            {
                _enabledVirtualChannel = value;
                OnPropertyChanged("EnabledVirtualChannel");
            }
        }

        public int FreqAverageMode
        {
            get { return _freqAverageMode; }
            set
            {
                if (_freqAverageMode != value)
                {
                    _freqAverageMode = value;
                    OnPropertyChanged("FreqAverageMode");
                    CalculateSpectral();
                }
            }
        }

        public int FreqAverageNum
        {
            get { return _freqAverageNum; }
            set
            {
                if (_freqAverageNum != value)
                {
                    _freqAverageNum = value;
                    OnPropertyChanged("FreqAverageNum");
                    CalculateSpectral();
                }
            }
        }

        public int Freqblock
        {
            get { return _freqblock; }
            set
            {
                if (_freqblock != value)
                {
                    _freqblock = value;
                    OnPropertyChanged("Freqblock");
                    CalculateSpectral();
                }
            }
        }

        public double FreqMax
        {
            get { return _freqMax; }
            set
            {
                _freqMax = value;
                OnPropertyChanged("FreqMax");
                UpdateSpecXRange();
            }
        }

        public double FreqMin
        {
            get { return _freqMin; }
            set
            {
                _freqMin = value;
                OnPropertyChanged("FreqMin");
                UpdateSpecXRange();
            }
        }

        public double FreqSampleSecMax
        {
            get { return _freqSampleSecMax; }
            set
            {
                if (_freqSampleSecMax != Math.Round(value, 1))
                {
                    _freqSampleSecMax = value;
                    OnPropertyChanged("FreqSampleSecMax");
                    CalculateSpectral();
                }
            }
        }

        public double FreqSampleSecMin
        {
            get { return _freqSampleSecMin; }
            set
            {
                if (_freqSampleSecMin != Math.Round(value, 1))
                {
                    _freqSampleSecMin = value;
                    OnPropertyChanged("FreqSampleSecMin");
                    CalculateSpectral();
                }
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is duration editing.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is duration editing; otherwise, <c>false</c>.
        /// </value>
        public bool IsDurationEditing
        {
            get
            {
                return _isDurationEditing;
            }
            set
            {
                _isDurationEditing = value;
                OnPropertyChanged("IsDurationEditing");
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is limit editing.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is limit editing; otherwise, <c>false</c>.
        /// </value>
        public bool IsLimitEditing
        {
            get
            {
                return _isLimitEditing;
            }
            set
            {
                _isLimitEditing = value;
                OnPropertyChanged("IsLimitEditing");
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is mode editing.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is mode editing; otherwise, <c>false</c>.
        /// </value>
        public bool IsModeEditing
        {
            get
            {
                return _isModeEditing;
            }
            set
            {
                _isModeEditing = value;
                OnPropertyChanged("IsModeEditing");
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is rate editing.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is rate editing; otherwise, <c>false</c>.
        /// </value>
        public bool IsRateEditing
        {
            get
            {
                return _isRateEditing;
            }
            set
            {
                _isRateEditing = value;
                OnPropertyChanged("IsRateEditing");
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is resolution editing.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is resolution editing; otherwise, <c>false</c>.
        /// </value>
        public bool IsResolutionEditing
        {
            get
            {
                return _isResolutionEditing;
            }
            set
            {
                _isResolutionEditing = value;
                OnPropertyChanged("IsResolutionEditing");
            }
        }

        public bool IsSimulator
        {
            get
            {
                string str = string.Empty;
                XmlNodeList ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]");
                if (GetAttribute(ndList[0], SettingsDoc, "type", ref str) && 0 == str.CompareTo("Simulator") && (0 >= SimulatorFilePath.Length))
                {
                    return true;
                }
                return false;
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is size editing.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is size editing; otherwise, <c>false</c>.
        /// </value>
        public bool IsSizeEditing
        {
            get
            {
                return _isSizeEditing;
            }
            set
            {
                _isSizeEditing = value;
                OnPropertyChanged("IsSizeEditing");
            }
        }

        public bool IsStackedDisplay
        {
            get
            {
                return _isStackedDisplay;
            }
            set
            {
                if (_isStackedDisplay != value)
                {
                    _isStackedDisplay = value;
                    OnPropertyChanged("IsStackedDisplay");
                    OnPropertyChanged("XVisibleRangeChart");
                    OnPropertyChanged("XVisibleRangeStack");

                    //update measure cursor if on by hide-then-display,
                    //to avoid x cursors out of range in another panel:
                    if (IsMeasureCursorVisible)
                    {
                        IsMeasureCursorVisible = false;
                        IsMeasureCursorVisible = true;
                    }
                }
            }
        }

        public double LiveSampleSec
        {
            get { return _liveSampleSec; }
            set
            {
                _liveSampleSec = Math.Round(value, 1);
                OnPropertyChanged("LiveSampleSec");
            }
        }

        public XmlDataProvider OTMProvider
        {
            get;
            set;
        }

        public string OTMSettingPath
        {
            get { return _otmSettingPath; }
            set
            {
                _otmSettingPath = value;
                OnPropertyChanged("OTMSettingPath");
                CreateSettings("OTM");
            }
        }

        public XmlDataProvider RealTimeProvider
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the sampling rate.
        /// </summary>
        /// <value>
        /// The sampling rate.
        /// </value>
        public string SamplingRate
        {
            get
            {
                if ((0 <= SampleRate) && (0 < SampleRateList.Count))
                {
                    return SampleRateList[SampleRate];
                }
                return "";
            }
        }

        public string SettingPath
        {
            get { return _settingPath; }
            set
            {
                _settingPath = value;
                OnPropertyChanged("SettingPath");
                ReloadProvider("REALTIME");
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether [stats panel enable].
        /// </summary>
        /// <value>
        ///   <c>true</c> if [stats panel enable]; otherwise, <c>false</c>.
        /// </value>
        public bool StatsPanelEnable
        {
            get
            {
                return _statsPanelEnable;
            }
            set
            {
                _statsPanelEnable = value;
                OnPropertyChanged("StatsPanelEnable");
            }
        }

        /// <summary>
        /// Gets or sets VisibilityCollection for windows visibility.
        /// </summary>
        public ObservableCollection<bool> VisibilityCollection
        {
            get
            {
                return _visibilityCollection;
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Add spectral chart.
        /// </summary>
        public void AddSpectralChartCmd()
        {
            AddSpectralChart addSpectralChart = new AddSpectralChart();
            addSpectralChart.DataContext = this;
            addSpectralChart.cbLine.ItemsSource = EnabledDataChannel.Union(EnabledVirtualChannel).Except(AllSpectralPhysicalChannel).ToList();
            addSpectralChart.cbLine.SelectedIndex = 0;
            addSpectralChart.ShowDialog();
        }

        public void ReloadProvider(string val)
        {
            XmlDocument doc = new XmlDocument();
            switch (val)
            {
                case "OTM":
                    if (File.Exists(_otmSettingPath))
                    {
                        doc.Load(_otmSettingPath);
                        OTMProvider.IsInitialLoadEnabled = true;

                        if (null == OTMProvider.Document)
                        {
                            OTMProvider.Document = doc;
                            OTMProvider.Document.NodeChanged += new System.Xml.XmlNodeChangedEventHandler(_otmProvider_NodeChanged);
                        }
                        else
                        {
                            OTMProvider.Document.NodeChanged -= new System.Xml.XmlNodeChangedEventHandler(_otmProvider_NodeChanged);
                            OTMProvider.Document = doc;
                            OTMProvider.Document.NodeChanged += new System.Xml.XmlNodeChangedEventHandler(_otmProvider_NodeChanged);
                        }
                        OTMProvider.Refresh();
                    }
                    break;
                case "REALTIME":
                    if (File.Exists(_settingPath))
                    {
                        doc.Load(_settingPath);
                        RealTimeProvider.IsInitialLoadEnabled = true;

                        if (null == RealTimeProvider.Document)
                        {
                            RealTimeProvider.Document = doc;
                            RealTimeProvider.Document.NodeChanged += new System.Xml.XmlNodeChangedEventHandler(RealTimeProvider_NodeChanged);
                        }
                        else
                        {
                            RealTimeProvider.Document.NodeChanged -= new System.Xml.XmlNodeChangedEventHandler(RealTimeProvider_NodeChanged);
                            RealTimeProvider.Document = doc;
                            RealTimeProvider.Document.NodeChanged += new System.Xml.XmlNodeChangedEventHandler(RealTimeProvider_NodeChanged);
                        }
                        RealTimeProvider.Refresh();
                    }
                    break;
                default:
                    break;
            }
        }

        public void _otmProvider_NodeChanged(object sender, XmlNodeChangedEventArgs e)
        {
            string[] str = new string[3] { "", "", "" };
            double[] val = new double[3] { 0, 0, 0 };
            bool skipCal = false;
            bool tempSwitchCI = false;
            System.Globalization.CultureInfo originalCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();

            try
            {
                //Keep decimal dot in xml:
                if (0 == originalCultureInfo.NumberFormat.NumberDecimalSeparator.CompareTo(","))
                {
                    originalCultureInfo.NumberFormat.NumberDecimalSeparator = ".";
                    System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
                    tempSwitchCI = true;
                }

                XmlDocument document = (XmlDocument)sender;
                XmlNodeList ndList = document.SelectNodes("/OTMSettings/Parameters");

                switch (e.NewParent.Name)
                {
                    case "Radius":
                        if (GetAttribute(ndList[0], document, "Viscosity", ref str[0]) && Double.TryParse(str[0], out val[0]))
                        {
                            val[1] = 6 * Constants.ThorRealTimeData.PI * Double.Parse(e.NewValue) * val[0];
                            SetAttribute(ndList[0], document, "GammaTheory", val[1].ToString());
                        }
                        break;
                    case "Viscosity":
                        if (GetAttribute(ndList[0], document, "Radius", ref str[0]) && Double.TryParse(str[0], out val[0]))
                        {
                            val[1] = 6 * Constants.ThorRealTimeData.PI * Double.Parse(e.NewValue) * val[0];
                            SetAttribute(ndList[0], document, "GammaTheory", val[1].ToString());
                        }
                        break;
                    case "GammaTheory":
                        if (GetAttribute(ndList[0], document, "Temperature", ref str[0]) && Double.TryParse(str[0], out val[0]))
                        {
                            val[1] = (0 != Double.Parse(e.NewValue)) ? (Constants.ThorRealTimeData.BOLTZMAN_CONST * (val[0] + Constants.ThorRealTimeData.CELSIUS_TO_KELVIN) / (Double.Parse(e.NewValue) / Constants.ThorRealTimeData.M2UM) * 1e12) : 0;
                            SetAttribute(ndList[0], document, "DiffTheory", val[1].ToString());
                        }
                        break;
                    case "Temperature":
                        if (GetAttribute(ndList[0], document, "GammaTheory", ref str[0]) && Double.TryParse(str[0], out val[0]))
                        {
                            val[1] = (0 != val[0]) ? (Constants.ThorRealTimeData.BOLTZMAN_CONST * (Double.Parse(e.NewValue) + Constants.ThorRealTimeData.CELSIUS_TO_KELVIN) / (val[0] / Constants.ThorRealTimeData.M2UM) * 1e12) : 0;
                            SetAttribute(ndList[0], document, "DiffTheory", val[1].ToString());
                        }
                        break;
                    case "IsCurveFit":
                        skipCal = (!bool.Parse(e.NewValue));
                        break;
                    case "IsPureWater":
                        const int ROOM_TEMP = 300;  //[K]
                        double viscosity = 0;
                        double temperature = 25 + Constants.ThorRealTimeData.CELSIUS_TO_KELVIN; //[K]
                        double gammatheory = 0;
                        double difftheory = 0;
                        if (bool.Parse(e.NewValue))
                        {
                            if ((GetAttribute(ndList[0], document, "Temperature", ref str[0]) && Double.TryParse(str[0], out val[0]))
                                && ((GetAttribute(ndList[0], document, "Radius", ref str[1]) && Double.TryParse(str[1], out val[1]))))
                            {
                                temperature = val[0] + Constants.ThorRealTimeData.CELSIUS_TO_KELVIN;
                                for (int i = 0; i < WATER_VISCOSITY_A.Length; i++)
                                {
                                    viscosity = viscosity + WATER_VISCOSITY_A[i] * Math.Pow((temperature / ROOM_TEMP), WATER_VISCOSITY_B[i]);
                                }
                                viscosity = viscosity / Constants.ThorRealTimeData.M2UM;
                                gammatheory = 6 * Constants.ThorRealTimeData.PI * viscosity * val[1];
                                difftheory = (0 != gammatheory) ? Constants.ThorRealTimeData.BOLTZMAN_CONST * temperature / gammatheory : 0;

                                SetAttribute(ndList[0], document, "Viscosity", viscosity.ToString());
                                SetAttribute(ndList[0], document, "GammaTheory", gammatheory.ToString());
                                SetAttribute(ndList[0], document, "DiffTheory", difftheory.ToString());
                            }
                        }
                        break;
                    default:
                        break;
                }
                document.Save(_otmSettingPath);
                OTMProvider.Refresh();

                if (!skipCal)
                {
                    CalculateSpectral();
                }
                else
                {
                    //clear fitting lines:
                    for (int j = 0; j < _specChViewModels.Count; j++)
                    {
                        using (_specChViewModels[j].ChannelSeries2.SuspendUpdates())
                        {
                            _specChViewModels[j].ChannelSeries2.Clear();
                        }
                        _specChViewModels[j].Update();
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart OTMSettings_NodeChanged Error: " + ex.Message);
            }
            //give back CultureInfo:
            if (tempSwitchCI)
            {
                originalCultureInfo.NumberFormat.NumberDecimalSeparator = ",";
                System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
            }
        }

        private void CreateSettings(string val)
        {
            string[] str = { "" };
            try
            {
                switch (val)
                {
                    case "OTM":
                        if (_configuration[0])
                        {
                            if (!File.Exists(_otmSettingPath))
                            {
                                new XDocument(
                                    new XElement("OTMSettings",
                                        new XElement("Parameters",
                                            new XAttribute("IsCurveFit", "False"), new XAttribute("FreqBlock", "1"),
                                            new XAttribute("FitFreqMin", "0"), new XAttribute("FitFreqMax", "1"),
                                            new XAttribute("Temperature", "25"), new XAttribute("Radius", "1"),
                                            new XAttribute("Viscosity", "0.00089"), new XAttribute("IsPureWater", "False"),
                                            new XAttribute("GammaTheory", "0.0167761047701695"),
                                            new XAttribute("DiffTheory", "0.2454"),
                                            new XAttribute("Beta2FreqMin", "0"), new XAttribute("Beta2FreqMax", "1")
                                            ),
                                        new XElement("Fittings",
                                            new XAttribute("DiffX1", "0"), new XAttribute("DiffY1", "0"),
                                            new XAttribute("DiffX2", "0"), new XAttribute("DiffY2", "0"),
                                            new XAttribute("CornerX", "0"), new XAttribute("CornerY", "0"),
                                            new XAttribute("ChiX", "0"), new XAttribute("ChiY", "0"),
                                            new XAttribute("GammaX", "0"), new XAttribute("GammaY", "0"),
                                            new XAttribute("KappaX", "0"), new XAttribute("KappaY", "0"),
                                            new XAttribute("BetaX1", "0"), new XAttribute("BetaY1", "0"),
                                            new XAttribute("BetaX2", "0"), new XAttribute("BetaY2", "0")
                                            )
                                        )
                                    ).Save(_otmSettingPath);
                            }

                            ReloadProvider("OTM");
                        }
                        break;
                    default:
                        break;
                }

            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart CreateSettings Error: " + ex.Message);
            }
        }

        private void RealTimeProvider_NodeChanged(object sender, XmlNodeChangedEventArgs e)
        {
            string[] str = new string[2] { "", "" };
            double[] val = new double[2] { 0, 0 };
            bool tempSwitchCI = false;
            System.Globalization.CultureInfo originalCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();

            try
            {
                //Keep decimal dot in xml:
                if (0 == originalCultureInfo.NumberFormat.NumberDecimalSeparator.CompareTo(","))
                {
                    originalCultureInfo.NumberFormat.NumberDecimalSeparator = ".";
                    System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
                    tempSwitchCI = true;
                }

                XmlDocument document = (XmlDocument)sender;
                XmlNodeList ndList = document.SelectNodes("/RealTimeDataSettings/Variables/Var");

                switch (e.NewParent.Name)
                {
                    case "ID":
                        break;
                    case "Value":
                        break;
                    case "Name":
                        break;
                    default:
                        break;
                }
                document.Save(_settingPath);
                RealTimeProvider.Refresh();

                RealTimeDataCapture.Instance.UpdateVariables();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart RealTimeProvider_NodeChanged Error: " + ex.Message);
            }
            //give back CultureInfo:
            if (tempSwitchCI)
            {
                originalCultureInfo.NumberFormat.NumberDecimalSeparator = ",";
                System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
            }
        }

        private void UpdateSpecXRange()
        {
            for (int i = 0; i < _specChViewModels.Count; i++)
            {
                using (_specChViewModels[i].ChannelSeries.SuspendUpdates())
                {
                    _specChViewModels[i].XVisibleRange.SetMinMax(_freqMin, _freqMax);
                }

            }
        }

        #endregion Methods
    }
}