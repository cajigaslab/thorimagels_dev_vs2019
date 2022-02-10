namespace ROIStatsChart.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;

    using ROIStatsChart.Model;

    using SciChart;
    using SciChart.Charting;
    using SciChart.Charting.ChartModifiers;
    using SciChart.Charting.Common.Extensions;
    using SciChart.Charting.Model.DataSeries;
    using SciChart.Charting.Themes;
    using SciChart.Charting.Visuals;
    using SciChart.Charting.Visuals.Annotations;
    using SciChart.Charting.Visuals.Axes;
    using SciChart.Charting.Visuals.Axes.LabelProviders;
    using SciChart.Charting.Visuals.Events;
    using SciChart.Charting.Visuals.RenderableSeries;
    using SciChart.Core;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;
    using SciChart.Drawing.VisualXcceleratorRasterizer;

    using ThorSharedTypes;

    using XMLHandle;

    #region Delegates

    public delegate void XPositionChangedHandler(double x, string[] statNames, double[] stats, string[] arithmeticNames, double[] arithmeticValues, int tag);

    #endregion Delegates

    //public delegate void XPositionChangedHandler(double x, string[] name, double[] value);
    public partial class ChartViewModel : ViewModelBase
    {
        #region Fields

        public static bool StatsCursorEnabled = true;

        private object _appResource;
        private List<string> _arithmeticEqID;
        private CustomCollection<CheckBoxDatabase> _arithmeticsCheckBoxList;
        private CustomCollection<CheckBoxDatabase> _channelCheckBoxList;
        private ObservableCollection<IRenderableSeries> _chartSeries;
        private string _chartXLabel = string.Empty;
        private bool _clockAsXAxis = false;
        private int _counterValue;
        private bool _dataStoreLoadComplete = false;
        private DateTime _dateTimeStart = new DateTime();
        private ICommand _deselectAllArithmeticsLegend;
        private ICommand _deselectAllChannelLegend;
        private ICommand _deselectAllFeatureLegend;
        private ICommand _deselectAllROILegend;
        private bool _editable;
        private string _expName;
        private CustomCollection<CheckBoxDatabase> _featureCheckBoxList;
        private int _fifoSize = 300;
        private ICommand _fifoSizeMinusCommand;
        private ICommand _fifoSizePlusCommand;
        private bool _isFifoVisible = true;
        private bool _isInLoad = false;
        private bool _isStatsCursorEnabled = true;
        private bool _panelsEnable = true;
        private string _path;
        private CustomCollection<CheckBoxDatabase> _roiCheckBoxList;
        private List<int> _roiID;
        private SciChartSurface _sciChartSurface;
        private ICommand _selectAllArithmeticsLegend;
        private ICommand _selectAllChannelLegend;
        private ICommand _selectAllFeatureLegend;
        private ICommand _selectAllROILegend;
        private List<double> _seriesData = new List<double>();
        private List<string> _seriesNames = new List<string>();
        private List<bool> _seriesVisibles = new List<bool>();
        private bool _skipGeometricInfo;
        private int _tag;
        private double _xLoc = 1;
        private double _xReviewPosition;
        private DoubleRange _xVisibleRange;

        #endregion Fields

        #region Constructors

        public ChartViewModel()
        {
            _editable = true;
            _appResource = GetApplicationSettingsFileString();
            _chartSeries = new ObservableCollection<IRenderableSeries>();

            _channelCheckBoxList = new CustomCollection<CheckBoxDatabase>();
            _featureCheckBoxList = new CustomCollection<CheckBoxDatabase>();

            _arithmeticsCheckBoxList = new CustomCollection<CheckBoxDatabase>();
            _roiCheckBoxList = new CustomCollection<CheckBoxDatabase>();

            _channelCheckBoxList.ItemPropertyChanged += CheckBoxList_ItemPropertyChanged;
            _featureCheckBoxList.ItemPropertyChanged += CheckBoxList_ItemPropertyChanged;

            _arithmeticsCheckBoxList.ItemPropertyChanged += CheckBoxList_ItemPropertyChanged;
            _arithmeticsCheckBoxList.PropertyChanged += CheckBoxList_ItemPropertyChanged;

            _roiCheckBoxList.ItemPropertyChanged += CheckBoxList_ItemPropertyChanged;
            _roiCheckBoxList.PropertyChanged += CheckBoxList_ItemPropertyChanged;

            _roiID = new List<int>();
            _arithmeticEqID = new List<string>();

            _sciChartSurface = null;

            _xVisibleRange = new DoubleRange();

            PanelsEnable = true;
            OnPropertyChanged("PanelsEnable");

            _skipGeometricInfo = true;
            XReviewPosition = 1000;
        }

        #endregion Constructors

        #region Enumerations

        public enum LegendGroup
        {
            FeatureLegend = 1,
            ChannelLegend = 2,
            ROILegend = 3,
            ArithmeticsLegend = 4
        }

        #endregion Enumerations

        #region Delegates

        public delegate void ArithmeticDataChangedEvent(object sender, EventArgs e);

        #endregion Delegates

        #region Events

        public static event XPositionChangedHandler OnXReviewPositionChanged;

        public event ArithmeticDataChangedEvent ArithmeticDataChanged;

        #endregion Events

        #region Properties

        public List<string> ArithmeticEqID
        {
            get { return _arithmeticEqID; }
            set { _arithmeticEqID = value; }
        }

        public CustomCollection<CheckBoxDatabase> ArithmeticsCheckBoxList
        {
            get
            {
                return _arithmeticsCheckBoxList;
            }
            set
            {
                _arithmeticsCheckBoxList = value;
                OnPropertyChanged("ArithmeticsCheckBoxList");
            }
        }

        public string AutoRangeX
        {
            get
            {
                if (IsFifoVisible)
                {
                    return "Always";
                }
                else
                {
                    return "Once";
                }
            }
        }

        public string AutoRangeY
        {
            get
            {
                if (IsFifoVisible)
                {
                    return "Always";
                }
                else
                {
                    return "Once";
                }
            }
        }

        public CustomCollection<CheckBoxDatabase> ChannelCheckBoxList
        {
            get
            {
                return _channelCheckBoxList;
            }
            set
            {
                _channelCheckBoxList = value;
                OnPropertyChanged("ChannelCheckBoxList");
            }
        }

        public ObservableCollection<IRenderableSeries> ChartSeries
        {
            get { return _chartSeries; }
        }

        public string ChartXLabel
        {
            get
            {
                return _chartXLabel;
            }
            set
            {
                _chartXLabel = value;
                OnPropertyChanged("ChartXLabel");
            }
        }

        public bool ClockAsXAxis
        {
            get { return _clockAsXAxis; }
            set { _clockAsXAxis = value; }
        }

        public int CounterValue
        {
            get
            {
                return _counterValue;
            }
            set
            {
                _counterValue = value;
                OnPropertyChanged("CounterValue");
            }
        }

        public bool DataStoreLoadComplete
        {
            get
            {
                return _dataStoreLoadComplete;
            }
            set
            {
                _dataStoreLoadComplete = value;
            }
        }

        public ICommand DeselectAllArithmeticsLegend
        {
            get
            {
                if (this._deselectAllArithmeticsLegend == null)
                    this._deselectAllArithmeticsLegend = new RelayCommand(() => DoSelectAllLegend(4, false));

                return this._deselectAllArithmeticsLegend;
            }
        }

        public ICommand DeselectAllChannelLegend
        {
            get
            {
                if (this._deselectAllChannelLegend == null)
                    this._deselectAllChannelLegend = new RelayCommand(() => DoSelectAllLegend(2, false));

                return this._deselectAllChannelLegend;
            }
        }

        public ICommand DeselectAllFeatureLegend
        {
            get
            {
                if (this._deselectAllFeatureLegend == null)
                    this._deselectAllFeatureLegend = new RelayCommand(() => DoSelectAllLegend(1, false));

                return this._deselectAllFeatureLegend;
            }
        }

        public ICommand DeselectAllROILegend
        {
            get
            {
                if (this._deselectAllROILegend == null)
                    this._deselectAllROILegend = new RelayCommand(() => DoSelectAllLegend(3, false));

                return this._deselectAllROILegend;
            }
        }

        public bool Editable
        {
            get
            {
                return _editable;
            }
            set
            {
                _editable = value;
            }
        }

        public string ExpName
        {
            get
            {
                return _expName;
            }
            set
            {
                _expName = value;
                OnPropertyChanged("ExpName");
            }
        }

        public CustomCollection<CheckBoxDatabase> FeatureCheckBoxList
        {
            get
            {
                return _featureCheckBoxList;
            }
            set
            {
                _featureCheckBoxList = value;
                OnPropertyChanged("FeatureCheckBoxList");
            }
        }

        public int FifoSize
        {
            get
            {
                return _fifoSize;
            }
            set
            {
                int val = Math.Max(2, value);

                if (val != _fifoSize)
                {
                    lock (_chartSeries)
                    {
                        //update Fifosize:
                        _chartSeries.ToList().ForEach(x => x.DataSeries.FifoCapacity = val);
                    }
                }
                _fifoSize = val;
                OnPropertyChanged("FifoSize");
            }
        }

        public ICommand FifoSizeMinusCommand
        {
            get
            {
                if (this._fifoSizeMinusCommand == null)
                    this._fifoSizeMinusCommand = new RelayCommand(() => FifoSizeMinus());

                return this._fifoSizeMinusCommand;
            }
        }

        public ICommand FifoSizePlusCommand
        {
            get
            {
                if (this._fifoSizePlusCommand == null)
                    this._fifoSizePlusCommand = new RelayCommand(() => FifoSizePlus());

                return this._fifoSizePlusCommand;
            }
        }

        public bool IsFifoVisible
        {
            get
            {
                return _isFifoVisible;
            }
            set
            {
                _isFifoVisible = value;
                OnPropertyChanged("IsFifoVisible");
                OnPropertyChanged("AutoRangeX");
                OnPropertyChanged("AutoRangeY");
            }
        }

        public bool IsInLoad
        {
            get
            {
                return _isInLoad;
            }
            set
            {
                _isInLoad = value;
                OnPropertyChanged("IsInLoad");
            }
        }

        public bool IsStatsCursorEnabled
        {
            get { return _isStatsCursorEnabled; }
            set
            {
                _isStatsCursorEnabled = value;
                StatsCursorEnabled = value;
            }
        }

        public bool PanelsEnable
        {
            get
            { return _panelsEnable; }
            set
            {
                _panelsEnable = value;
                OnPropertyChanged("PanelsEnable");
            }
        }

        public string Path
        {
            set
            {
                _path = value;
            }
        }

        public CustomCollection<CheckBoxDatabase> ROICheckBoxList
        {
            get
            {
                return _roiCheckBoxList;
            }
            set
            {
                _roiCheckBoxList = value;
                OnPropertyChanged("ROICheckBoxList");
            }
        }

        public List<int> ROIID
        {
            get { return _roiID; }
            set { _roiID = value; }
        }

        public SciChartSurface SciChartSurface
        {
            get { return _sciChartSurface; }
            set { _sciChartSurface = value; }
        }

        public ICommand SelectAllArithmeticsLegend
        {
            get
            {
                if (this._selectAllArithmeticsLegend == null)
                    this._selectAllArithmeticsLegend = new RelayCommand(() => DoSelectAllLegend(4, true));

                return this._selectAllArithmeticsLegend;
            }
        }

        public ICommand SelectAllChannelLegend
        {
            get
            {
                if (this._selectAllChannelLegend == null)
                    this._selectAllChannelLegend = new RelayCommand(() => DoSelectAllLegend(2, true));

                return this._selectAllChannelLegend;
            }
        }

        public ICommand SelectAllFeatureLegend
        {
            get
            {
                if (this._selectAllFeatureLegend == null)
                    this._selectAllFeatureLegend = new RelayCommand(() => DoSelectAllLegend(1, true));

                return this._selectAllFeatureLegend;
            }
        }

        public ICommand SelectAllROILegend
        {
            get
            {
                if (this._selectAllROILegend == null)
                    this._selectAllROILegend = new RelayCommand(() => DoSelectAllLegend(3, true));

                return this._selectAllROILegend;
            }
        }

        public bool SkipGeometricInfo
        {
            get
            {
                return _skipGeometricInfo;
            }
            set
            {
                _skipGeometricInfo = value;
            }
        }

        public double[] StatsData
        {
            get;
            set;
        }

        public string[] StatsNames
        {
            get;
            set;
        }

        public int Tag
        {
            get
            {
                return _tag;
            }
            set
            {
                _tag = value;
            }
        }

        public double XReviewLabel
        {
            get
            {
                return Math.Round(_xReviewPosition / 1000);
            }
        }

        public double XReviewPosition
        {
            get
            {

                return _xReviewPosition;
            }
            set
            {
                if (Math.Round(value) != Math.Round(_xReviewPosition))
                {
                    _xReviewPosition = (int)Math.Round(value);
                    OnPropertyChanged("XReviewPosition");
                    OnPropertyChanged("XReviewLabel");
                    CollectROIDataAtX(_xReviewPosition);
                }
            }
        }

        public DoubleRange XVisibleRange
        {
            get
            {
                return _xVisibleRange;
            }
            set
            {
                //limit the range of the X axis so that the user cannot go outside of the data boundaries.
                if (_chartSeries != null && _chartSeries.Count > 0 &&
                    _chartSeries[0] != null && _chartSeries[0].DataSeries.XRange != null)
                {
                    double lowerRange = value.Min;
                    double upperRange = value.Max;
                    double min = (double)_chartSeries[0].DataSeries.XRange.Min;
                    double max = (double)_chartSeries[0].DataSeries.XRange.Max;

                    if (lowerRange < min)
                    {
                        lowerRange = min;
                    }
                    if (upperRange > max)
                    {
                        upperRange = max;
                    }

                    if ((int)Math.Round(_xVisibleRange.Min) != (int)Math.Round(lowerRange) ||
                        (int)Math.Round(_xVisibleRange.Max) != (int)Math.Round(upperRange))
                    {
                        _xVisibleRange = new DoubleRange(lowerRange, upperRange);
                        OnPropertyChanged("XVisibleRange");
                    }
                }
                else
                {
                    _xVisibleRange = value;
                    OnPropertyChanged("XVisibleRange");
                }
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetApplicationSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetApplicationSettingsFilePathAndName(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetHardwareSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetHardwareSettingsFilePathAndName(StringBuilder sb, int length);

        public void ClearData()
        {
            _xLoc = 1;
            XVisibleRange.SetMinMax(0.0, 0.0);
            _chartSeries.Clear();
        }

        public void ClearLegendGroup(bool clearAll)
        {
            if (clearAll)
            {
                FeatureCheckBoxList.Clear();
                ChannelCheckBoxList.Clear();
            }
            ROICheckBoxList.Clear();
            ROIID.Clear();
            ArithmeticsCheckBoxList.Clear();
            ArithmeticEqID.Clear();
        }

        public string GetApplicationSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetApplicationSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        public string GetHardwareSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetHardwareSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        public int GetLegendGroupEnable(LegendGroup input)
        {
            int enabled = 0;
            switch (input)
            {
                case LegendGroup.FeatureLegend:
                    for (int i = 0; i < FeatureCheckBoxList.Count; i++)
                    {
                        if ((FeatureCheckBoxList[i].ID == i + 1) && (FeatureCheckBoxList[i].IsChecked))
                        {
                            enabled += 1 << i;
                        }
                    }
                    break;
                case LegendGroup.ChannelLegend:
                    for (int i = 0; i < ChannelCheckBoxList.Count; i++)
                    {
                        if ((ChannelCheckBoxList[i].ID == i + 1) && (ChannelCheckBoxList[i].IsChecked))
                        {
                            enabled += 1 << i;
                        }
                    }
                    break;
            }
            return enabled;
        }

        public void LoadDataFromDevice(bool reLoad)
        {
            string measure = string.Empty, channel = string.Empty;
            int roiIndex = 0;
            string pattern = "(.*)([A|B|C|D])([0-9]+$)";

            if ((0 == StatsNames.Length) || (0 == StatsData.Length))
            {
                return;
            }

            long seriesCount = StatsData.Length / StatsNames.Length;

            if (0 == seriesCount)
            {
                return;
            }

            //get names and data:
            GetDataSeriesNames();
            bool updateROILegend = false;
            bool updateARLegend = false;

            //After setting ROI legend, allow ignore reload:
            if ((3 > _xLoc) || (true == reLoad))
            {
                //build a list of the current series names and ROIs' id:
                List<string> curNames = new List<string>();
                List<int> curROIs = new List<int>();
                List<string> curNamesForLegends = new List<string>();
                List<string> curArithmetics = new List<string>();

                for (int i = 0; i < _seriesNames.Count; i++)
                {
                    curNamesForLegends.Add(_seriesNames[i]);
                    ParseDataSeriesName(_seriesNames[i], pattern, ref measure, ref channel, ref roiIndex);
                    if (!curROIs.Contains(roiIndex))
                    {
                        curROIs.Add(roiIndex);
                    }
                    if (-1 < _seriesNames[i].LastIndexOf("_Ar"))
                    {
                        curArithmetics.Add(_seriesNames[i]);
                    }
                }

                //set roi legend based on current series names:
                SetDataSeriesROILegend(curNamesForLegends, curROIs, ref updateROILegend);

                //set Arithmetics legend based on current series names:
                SetDataSeriesArithmeticsLegend(curNamesForLegends, curArithmetics, ref updateARLegend);

                //build a list of the series names in the previous set of data
                lock (_chartSeries)
                {
                    for (int i = 0; i < _chartSeries.Count; i++)
                    {
                        curNames.Add(_chartSeries[i].DataSeries.SeriesName);
                    }
                }

                //remove deleted series' names:
                if (_seriesNames.Count < curNames.Count)
                {
                    List<string> tmpStr = curNames.Except(_seriesNames).ToList();
                    for (int i = 0; i < tmpStr.Count; i++)
                    {
                        lock (_chartSeries)
                        {
                            int tmpID = _chartSeries.IndexOf(_chartSeries.Where(x => x.DataSeries.SeriesName.Equals(tmpStr[i])).FirstOrDefault());
                            if (tmpID > -1)
                            {
                                _chartSeries.RemoveAt(tmpID);
                            }
                        }
                    }
                }

                //extract the differences in the two and add them
                //to the chart series
                //assume that chartSeries names will be a superset of
                //the _seriesNames. (Ex. it will contain lines that were
                //deleted for historical plotting)
                curNames = _seriesNames.Except(curNames).ToList();

                //set series visibilities after exclusion:
                //Used to skip unnecessary names when in Fifo mode
                if (false == _skipGeometricInfo)
                {
                    SetDataSeriesVisibility(curNames);
                }

                //start the x axis time clock if this is the
                //first entry into the chart series
                if ((0 == _chartSeries.Count) && (curNames.Count > 0) && (_clockAsXAxis))
                {
                    _dateTimeStart = DateTime.Now;
                }

                //start create (additional) data series:
                for (int i = 0; i < curNames.Count; i++)
                {

                    double[] strokeDashArray = null;
                    string name = curNames[i];
                    int arithmetic = 0;
                    if (-1 < curNames[i].LastIndexOf("_Ar"))
                    {
                        strokeDashArray = new[] { 3.0, 3.0 };
                        int indx = curNames[i].LastIndexOf("_Ar");
                        //name = curNames[i].Substring(0, indx);
                        name = curNames[i];
                        arithmetic = Convert.ToInt32(curNames[i].Substring(indx + 3, curNames[i].Length - indx - 3));
                    }

                    IXyDataSeries<double, double> ds0;
                    if ((0 == FifoSize) || (!IsFifoVisible))
                    {
                        ds0 = new XyDataSeries<double, double> { FifoCapacity = null, SeriesName = name };
                    }
                    else
                    {
                        ds0 = new XyDataSeries<double, double> { FifoCapacity = FifoSize, SeriesName = name };
                    }
                    lock (_chartSeries)
                    {
                        bool isVisible = _skipGeometricInfo ? true : _seriesVisibles[i];
                        var l = new FastLineRenderableSeries() { DataSeries = ds0, Tag = arithmetic, StrokeDashArray = strokeDashArray, StrokeThickness = 2, IsVisible = isVisible, Stroke = (Color)ChartLineProperty.GetLineColor(curNames[i], typeof(Color)) };
                        _chartSeries.Add(l);
                    }
                }
            }

            //start append data:
            lock (_chartSeries)
            {
                var xy = new XYPoint();
                if (_clockAsXAxis)
                {
                    TimeSpan ts = DateTime.Now.Subtract(_dateTimeStart);
                    xy.X = (ts.TotalMilliseconds);
                }
                else
                {
                    xy.X = (double)_xLoc * 1000;
                }
                _arithmeticDataChartIndex.Clear();
                for (int j = 0; j < _chartSeries.Count; j++)
                {
                    int nameId = _seriesNames.IndexOf(_seriesNames.Where(x => x.Equals(_chartSeries[j].DataSeries.SeriesName)).FirstOrDefault());
                    if (true == _chartSeries[j].DataSeries.SeriesName.Contains("_Ar"))
                    {
                        _arithmeticDataChartIndex.Add(_chartSeries[j].DataSeries.SeriesName, j);
                    }
                    if (nameId > -1)
                    {
                        DoubleSeries ds = new DoubleSeries();

                        xy.Y = _seriesData[nameId];
                        ds.Add(xy);

                        ((IXyDataSeries<double, double>)_chartSeries[j].DataSeries).Append(ds.XData, ds.YData);
                    }
                }
            }
            _xLoc++;

            //restart without closing window, trigger for checking visibility:
            if ((2 == _xLoc) || (updateROILegend) || (updateARLegend))
            {
                UpdateDataSeriesSelection();
            }
        }

        public void LoadSettings()
        {
            int fEnable = 0, cEnable = 0;
            if (null != _appResource)
            {
                XMLHandler xmlSetter = new XMLHandler(_appResource);
                XmlDocument appSettings = new XmlDocument();
                if (xmlSetter.Load(appSettings))
                {
                    XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");
                    if (node != null)
                    {
                        string str = string.Empty;
                        if (false == xmlSetter.GetAttribute(node, appSettings, "legendF", ref str))
                        { return; }
                        fEnable = Convert.ToInt32(str);
                        if (false == xmlSetter.GetAttribute(node, appSettings, "legendC", ref str))
                        { return; }
                        cEnable = Convert.ToInt32(str);
                        if (false == xmlSetter.GetAttribute(node, appSettings, "fifo", ref str))
                        { return; }
                        FifoSize = Convert.ToInt32(str);
                        if (false == xmlSetter.GetAttribute(node, appSettings, "cursorStatsEnable", ref str))
                        { return; }
                        IsStatsCursorEnabled = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                }
            }

            //try setup legend selection:
            for (int ich = ChannelCheckBoxList.Count; ich > 0; ich--)
            {
                if ((cEnable >> ich - 1) > 0)
                {
                    ChannelCheckBoxList[ich - 1].IsChecked = true;
                    cEnable -= (1 << (ich - 1));
                }
                else
                {
                    ChannelCheckBoxList[ich - 1].IsChecked = false;
                }
            }
            for (int id = FeatureCheckBoxList.Count; id > 0; id--)
            {
                if ((fEnable >> id - 1) > 0)
                {
                    FeatureCheckBoxList[id - 1].IsChecked = true;
                    fEnable -= (1 << (id - 1));
                }
                else
                {
                    FeatureCheckBoxList[id - 1].IsChecked = false;
                }
            }
            LoadEquationsFromFile();
        }

        /// <summary>
        /// From Unmanaged to managed double array.
        /// </summary>
        /// <param name="pUnDbkArray"></param>
        /// <param name="AryCnt"></param>
        /// <param name="dblArray"></param>
        public void MarshalDblArray(IntPtr pUnDblArray, int AryCnt, out double[] dblArray)
        {
            if (AryCnt > 0)
            {
                dblArray = new double[AryCnt];

                Marshal.Copy(pUnDblArray, dblArray, 0, AryCnt);

                //Marshal.FreeCoTaskMem(pUnDblArray);
            }
            else
            {
                dblArray = null;
            }
        }

        /// <summary>
        /// From Unmanaged to managed string array.
        /// </summary>
        /// <param name="pUnStrArray"></param>
        /// <param name="AryCnt"></param>
        /// <param name="StrArray"></param>
        public void MarshalStrArray(IntPtr pUnStrArray, int AryCnt, out string[] StrArray)
        {
            if (AryCnt > 0)
            {
                IntPtr[] pIntPtrArray = new IntPtr[AryCnt];
                StrArray = new string[AryCnt];

                Marshal.Copy(pUnStrArray, pIntPtrArray, 0, AryCnt);

                for (int i = 0; i < AryCnt; i++)
                {
                    StrArray[i] = Marshal.PtrToStringAnsi(pIntPtrArray[i]);
                    //Marshal.FreeCoTaskMem(pIntPtrArray[i]);
                }

                //Marshal.FreeCoTaskMem(pUnStrArray);
            }
            else
            {
                StrArray = null;
            }
        }

        public void PersistSettings()
        {
            MVMManager.Instance.LoadSettings();
            XmlDocument ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");
            if (null != node)
            {
                int enable = GetLegendGroupEnable(LegendGroup.FeatureLegend);

                XmlManager.SetAttribute(node, ApplicationDoc, "legendF", enable.ToString());
                enable = GetLegendGroupEnable(LegendGroup.ChannelLegend);
                XmlManager.SetAttribute(node, ApplicationDoc, "legendC", enable.ToString());
                XmlManager.SetAttribute(node, ApplicationDoc, "fifo", FifoSize.ToString());
                string cursorEnableString = (true == _isStatsCursorEnabled) ? "1" : "0";
                XmlManager.SetAttribute(node, ApplicationDoc, "cursorStatsEnable", cursorEnableString);
                XmlManager.SetAttribute(node, ApplicationDoc, "reset", "0");
                //save xml:
                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                PersistEquationsToFile();
            }
        }

        public void SetLegendGroup(int groupID, string[] Names, bool[] enables)
        {
            switch (groupID)
            {
                case (int)LegendGroup.FeatureLegend:

                    FeatureCheckBoxList.Clear();

                    if ((Names == null) || (enables == null) || (Names.Count() != enables.Count()))
                    {
                        return;
                    }
                    for (int i = 0; i < Names.Count(); i++)
                    {
                        var db = new CheckBoxDatabase { Databases = FeatureCheckBoxList, Name = Names[i], IsChecked = enables[i], ID = i + 1, TextColor = new SolidColorBrush(Colors.White) };
                        FeatureCheckBoxList.Add(db);
                    }
                    break;
                case (int)LegendGroup.ChannelLegend:

                    ChannelCheckBoxList.Clear();

                    if ((Names == null) || (enables == null) || (Names.Count() != enables.Count()))
                    {
                        return;
                    }
                    for (int i = 0; i < Names.Count(); i++)
                    {
                        var db = new CheckBoxDatabase { Databases = ChannelCheckBoxList, Name = Names[i], IsChecked = enables[i], ID = i + 1, TextColor = new SolidColorBrush(Colors.White) };
                        ChannelCheckBoxList.Add(db);
                    }
                    break;
                case (int)LegendGroup.ROILegend:

                    ROICheckBoxList.Clear();

                    if ((Names == null) || (enables == null) || (Names.Count() != enables.Count()))
                    {
                        return;
                    }
                    for (int i = 0; i < Names.Count(); i++)
                    {
                        var db = new CheckBoxDatabase { Databases = ROICheckBoxList, Name = Names[i], IsChecked = enables[i], ID = i + 1, TextColor = new SolidColorBrush(Colors.White) };
                        ROICheckBoxList.Add(db);
                    }
                    break;
                case (int)LegendGroup.ArithmeticsLegend:
                    ArithmeticsCheckBoxList.Clear();
                    if ((Names == null) || (enables == null) || (Names.Count() != enables.Count()))
                    {
                        return;
                    }
                    for (int i = 0; i < Names.Count(); i++)
                    {
                        var db = new CheckBoxDatabase { Databases = ArithmeticsCheckBoxList, Name = Names[i], IsChecked = enables[i], ID = i + 1, TextColor = new SolidColorBrush(Colors.White) };
                        ArithmeticsCheckBoxList.Add(db);
                    }

                    break;

            }
        }

        public void ZoomExtendChartSeries()
        {
            if (ChartSeries.Count > 0)
            {
                lock (ChartSeries)
                {
                    for (int i = 0; i < ChartSeries.Count; i++)
                    {
                        ChartSeries[i].DataSeries.InvalidateParentSurface(RangeMode.ZoomToFit);
                    }
                }
            }
        }

        private void CheckBoxList_ItemPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            UpdateDataSeriesSelection();
        }

        //Get data for an X Point and bubble up event with this data
        private void CollectROIDataAtX(double x)
        {
            if (null != OnXReviewPositionChanged)
            {
                try
                {
                    int xPos = ((int)(x - 1) < 0) ? 0 : (int)(x - 1);

                    int nStats = _seriesNames.Count;
                    string[] seriesNames = new string[_chartSeries.Count];
                    double[] seriesData = new double[_chartSeries.Count];
                    string[] basicStatsNames = new string[_chartSeries.Count - _arithmeticDataChartIndex.Count];
                    double[] basicStats = new double[_chartSeries.Count - _arithmeticDataChartIndex.Count];
                    string[] arithmeticStatsNames = new string[_arithmeticDataChartIndex.Count];
                    double[] arithmeticStats = new double[_arithmeticDataChartIndex.Count];
                    lock (_chartSeries)
                    {
                        //seriesNames = _chartSeries.Select(obj => obj.DataSeries.SeriesName).ToArray();
                        //seriesData = _chartSeries.Select(obj => (double)obj.DataSeries.YValues[xPos]).ToArray();
                        int j = 0;
                        int k = 0;
                        for (int i = 0; i < _chartSeries.Count; i++)
                        {
                            if (_arithmeticDataChartIndex.ContainsKey(_chartSeries[i].DataSeries.SeriesName))
                            {
                                arithmeticStatsNames[k] = _chartSeries[i].DataSeries.SeriesName;
                                arithmeticStats[k] = ((IXyDataSeries<double, double>)_chartSeries[i].DataSeries).YValues[xPos / 1000];
                                k++;
                            }
                            else
                            {
                                basicStatsNames[j] = _chartSeries[i].DataSeries.SeriesName;
                                basicStats[j] = ((IXyDataSeries<double, double>)_chartSeries[i].DataSeries).YValues[xPos / 1000];
                                j++;
                            }
                        }
                        //Array.Copy(seriesNames, 0, basicStatsNames, 0, basicStatsNames.Length);
                        //Array.Copy(seriesData, 0, basicStats, 0, basicStats.Length);
                        //Array.Copy(seriesNames, basicStatsNames.Length, arithmeticStatsNames, 0, arithmeticStatsNames.Length);
                        //Array.Copy(seriesData, basicStats.Length, arithmeticStats, 0, arithmeticStats.Length);
                    }
                    OnXReviewPositionChanged(Math.Round(_xReviewPosition / 1000), FormatNames(basicStatsNames), basicStats, arithmeticStatsNames, arithmeticStats, _tag);
                }
                catch (Exception e)
                {
                    e.ToString();
                }

            }
        }

        private void DoSelectAllLegend(int groupID, bool selected)
        {
            switch (groupID)
            {
                case 1: //Features
                    if (FeatureCheckBoxList != null)
                    {
                        lock (FeatureCheckBoxList)
                        {
                            foreach (CheckBoxDatabase item in FeatureCheckBoxList)
                            {
                                item.IsChecked = selected;
                            }
                        }
                    }
                    break;
                case 2: //channels
                    if (ChannelCheckBoxList != null)
                    {
                        lock (ChannelCheckBoxList)
                        {
                            foreach (CheckBoxDatabase item in ChannelCheckBoxList)
                            {
                                item.IsChecked = selected;
                            }
                        }
                    }
                    break;
                case 3: //ROIs
                    if (ROICheckBoxList != null)
                    {
                        lock (ROICheckBoxList)
                        {
                            foreach (CheckBoxDatabase item in ROICheckBoxList)
                            {
                                item.IsChecked = selected;
                            }
                        }
                    }
                    break;
                case 4: //Arithmetics
                    if (ArithmeticsCheckBoxList != null)
                    {
                        lock (ArithmeticsCheckBoxList)
                        {
                            foreach (CheckBoxDatabase item in _arithmeticsCheckBoxList)
                            {
                                item.IsChecked = selected;
                            }
                        }
                    }
                    break;
            }
        }

        private void FifoSizeMinus()
        {
            FifoSize -= 1;
        }

        private void FifoSizePlus()
        {
            FifoSize += 1;
        }

        private string[] FormatNames(string[] nameIn)
        {
            string[] nameOut = new string[nameIn.Length];

            for (int i = 0; i < nameIn.Length; i++)
            {
                int roiIndex;
                int.TryParse(nameIn[i].Substring((nameIn[i].LastIndexOf('A') + 1)), out roiIndex);
                if (roiIndex != 0)
                {
                    nameOut[i] = nameIn[i].Substring(0, nameIn[i].LastIndexOf('A')) + "_1_" + roiIndex;
                    continue;
                }
                int.TryParse(nameIn[i].Substring((nameIn[i].LastIndexOf('B') + 1)), out roiIndex);
                if (roiIndex != 0)
                {
                    nameOut[i] = nameIn[i].Substring(0, nameIn[i].LastIndexOf('B')) + "_2_" + roiIndex;
                    continue;
                }
                int.TryParse(nameIn[i].Substring((nameIn[i].LastIndexOf('C') + 1)), out roiIndex);
                if (roiIndex != 0)
                {
                    nameOut[i] = nameIn[i].Substring(0, nameIn[i].LastIndexOf('C')) + "_3_" + roiIndex;
                    continue;
                }
                int.TryParse(nameIn[i].Substring((nameIn[i].LastIndexOf('D') + 1)), out roiIndex);
                if (roiIndex != 0)
                {
                    nameOut[i] = nameIn[i].Substring(0, nameIn[i].LastIndexOf('D')) + "_4_" + roiIndex;
                    continue;
                }
            }
            return nameOut;
        }

        private void GetDataSeriesNames()
        {
            string measure = string.Empty, channel = string.Empty;
            int roiIndex = 0;
            string pattern = "(.*)_(.*)_(.*)";
            string chan = string.Empty;

            _seriesNames.Clear();
            _seriesData.Clear();
            if (false == _editable)
            {
                _arithmeticNames.Clear();
            }

            //extract the total channels
            lock (StatsNames)
            {
                for (int i = 0; i < StatsNames.Length; i++)
                {
                    if (false == StatsNames[i].Contains("_Ar"))
                    {
                        ParseDataSeriesName(StatsNames[i], pattern, ref measure, ref channel, ref roiIndex);
                        if (((string.Empty == measure) || (string.Empty == channel)))
                        {
                            continue;
                        }
                        //Used to skip unnecessary names when in Fifo mode
                        if (true == _skipGeometricInfo)
                        {
                            //skip names
                            if (measure.Contains("left") || measure.Contains("top") || measure.Contains("width") || measure.Contains("height"))
                            {
                                continue;
                            }
                        }

                        switch (Convert.ToInt32(channel.ToString()))
                        {
                            case 1: chan = "A"; break;
                            case 2: chan = "B"; break;
                            case 3: chan = "C"; break;
                            case 4: chan = "D"; break;
                        }

                        //add names and data, will be used for chart:
                        string str = measure.ToString() + chan + roiIndex.ToString();
                        _seriesNames.Add(str);
                    }
                    else if (false == _editable)
                    {
                        _seriesNames.Add(StatsNames[i]);
                        _arithmeticNames.Add(StatsNames[i]);
                        _arithmeticData.Add(StatsData[i]);
                    }
                    lock (StatsData)
                    {
                        _seriesData.Add(StatsData[i]);
                    }
                }
                if (true == _editable)
                {
                    CalculateROIArithmeticsRT();
                }
            }
        }

        private double[] GetStatsSubArray(double[] source, int length)
        {
            double[] dest = new double[length];
            Array.Copy(source, 0, dest, 0, length);
            return dest;
        }

        private void LoadDataFromFile()
        {
        }

        private void ParseDataSeriesName(string name, string pattern, ref string measure, ref string channel, ref int roi)
        {
            Regex ex = new Regex(pattern, RegexOptions.IgnoreCase);
            Match match = ex.Match(name);
            if (match.Groups.Count == 4)
            {
                measure = match.Groups[1].ToString();
                channel = match.Groups[2].ToString();
                roi = Convert.ToInt32(match.Groups[3].ToString());
            }
        }

        /// <summary>
        /// Fires the ArithmeticDataChangedEvent if it is non null
        /// </summary>
        private void RaiseArithmeticDataChangedEvent()
        {
            if (ArithmeticDataChanged != null)
            {
                ArithmeticDataChanged(this, new EventArgs());
            }
        }

        private void SetDataSeriesArithmeticsLegend(List<string> curNames, List<string> curArithmetics, ref bool updateArithmeticsLegend)
        {
            lock (_arithmeticsCheckBoxList)
            {
                if (0 == curArithmetics.Count)
                {
                    ArithmeticsCheckBoxList.Clear();
                    ArithmeticEqID.Clear();
                    return;
                }

                //build or add Artithmetic legends:
                List<bool> checks = new List<bool>();
                List<string> names = new List<string>();
                List<int> ids = new List<int>();
                for (int i = 0; i < curNames.Count; i++)
                {
                    int indx = curNames[i].LastIndexOf("_Ar");
                    if (-1 < indx)
                    {
                        if (null != _arithmeticsCheckBoxList && 0 < _arithmeticsCheckBoxList.Count && checks.Count < _arithmeticsCheckBoxList.Count)
                        {
                            bool check = _arithmeticsCheckBoxList[checks.Count].IsChecked;
                            checks.Add(check);
                        }
                        else
                        {
                            checks.Add(true);
                        }

                        names.Add(curNames[i].Substring(0, indx));
                        ids.Add(Convert.ToInt32(curNames[i].Substring(indx + 3, curNames[i].Length - indx - 3)));

                        if (!ArithmeticEqID.Contains(curNames[i]))
                        {
                            ArithmeticEqID.Add(curNames[i]);

                            updateArithmeticsLegend = true;
                        }

                    }
                }

                if (true == updateArithmeticsLegend && 0 < names.Count)
                {
                    _arithmeticsCheckBoxList.Clear();
                    for (int i = 0; i < names.Count; i++)
                    {
                        var db = new CheckBoxDatabase { Databases = ArithmeticsCheckBoxList, Name = names[i], IsChecked = checks[i], ID = ids[i], TextColor = (Brush)ChartLineProperty.GetLineColor(ids[i], typeof(Brush)) };
                        ArithmeticsCheckBoxList.Add(db);
                    }
                }

                //remove deleted Artithmetic legends:
                if (curArithmetics.Count < ArithmeticEqID.Count)
                {
                    curArithmetics = ArithmeticEqID.Except(curArithmetics).ToList();
                    for (int i = 0; i < curArithmetics.Count; i++)
                    {
                        lock (_arithmeticsCheckBoxList)
                        {
                            int indx = curArithmetics[i].LastIndexOf("_Ar");
                            string name = curArithmetics[i].Substring(0, indx);
                            int arithmeticsCheckBoxId = ArithmeticsCheckBoxList.IndexOf(ArithmeticsCheckBoxList.Where(x => x.Name == name).FirstOrDefault());
                            if (arithmeticsCheckBoxId > -1)
                            {
                                ArithmeticsCheckBoxList.RemoveAt(arithmeticsCheckBoxId);
                                ArithmeticEqID.RemoveAt(arithmeticsCheckBoxId);
                                updateArithmeticsLegend = true;
                            }
                        }
                    }
                }
                ArithmeticsCheckBoxList.Sort();
            }
        }

        private void SetDataSeriesROILegend(List<string> curNames, List<int> curROIs, ref bool updateROILegend)
        {
            string measure = string.Empty, channel = string.Empty;
            int roiIndex = 0;
            string pattern = "(.*)([A|B|C|D])([0-9]+$)";
            //build or add ROI legends:
            for (int i = 0; i < curNames.Count; i++)
            {
                if (true == curNames[i].Contains("_Ar"))
                {
                    continue;
                }
                ParseDataSeriesName(curNames[i], pattern, ref measure, ref channel, ref roiIndex);
                if (!ROIID.Contains(roiIndex))
                {
                    ROIID.Add(roiIndex);
                    lock (_roiCheckBoxList)
                    {
                        var db = new CheckBoxDatabase { Databases = ROICheckBoxList, Name = "ROI" + roiIndex.ToString(), IsChecked = true, ID = roiIndex, TextColor = (Brush)ChartLineProperty.GetLineColor(roiIndex, typeof(Brush)) };   //"00"
                        ROICheckBoxList.Add(db);
                    }
                }
            }
            ROICheckBoxList.Sort();
            ROIID.Sort();
            //remove deleted ROI's legends:
            if ((0 < curROIs.Count) && (curROIs.Count < ROIID.Count))
            {
                curROIs = ROIID.Except(curROIs).ToList();
                for (int i = 0; i < curROIs.Count; i++)
                {
                    lock (_roiCheckBoxList)
                    {
                        int roiCheckBoxId = ROICheckBoxList.IndexOf(ROICheckBoxList.Where(x => x.ID == curROIs[i]).FirstOrDefault());
                        if (roiCheckBoxId > -1)
                        {
                            ROICheckBoxList.RemoveAt(roiCheckBoxId);
                            ROIID.RemoveAt(roiCheckBoxId);
                            updateROILegend = true;
                        }
                    }
                }
            }
        }

        private void SetDataSeriesVisibility(List<string> curNames)
        {
            string measure = string.Empty, channel = string.Empty;
            int roiIndex = 0;
            if (curNames.Count <= 0)
                return;

            _seriesVisibles.Clear();
            string pattern = "(.*)([A|B|C|D])([0-9]+$)";

            for (int i = 0; i < curNames.Count; i++)
            {
                ParseDataSeriesName(curNames[i], pattern, ref measure, ref channel, ref roiIndex);
                //series visibility:
                if (measure.Contains("left") || measure.Contains("top") || measure.Contains("width") || measure.Contains("height"))
                { _seriesVisibles.Add(false); }
                else
                { _seriesVisibles.Add(true); }
            }
        }

        private void UpdateDataSeriesSelection()
        {
            if (_chartSeries != null)
            {
                lock (_chartSeries)
                {
                    try
                    {
                        CustomCollection<CheckBoxDatabase> tmp = new CustomCollection<CheckBoxDatabase>();
                        int i = 0;
                        do
                        {
                            if (i == _chartSeries.Count)
                            {
                                i = 0;
                                return;
                            }

                            if (-1 < _chartSeries[i].DataSeries.SeriesName.LastIndexOf("_Ar"))
                            {
                                tmp.Clear();
                                ArithmeticsCheckBoxList.Clone(ref tmp);
                                if (i < _chartSeries.Count)
                                {
                                    int indx = _chartSeries[i].DataSeries.SeriesName.LastIndexOf("_Ar");
                                    string name = _chartSeries[i].DataSeries.SeriesName.Substring(0, indx);

                                    tmp.Filter = p => p.Name == name;
                                }
                                if (tmp.Count > 0)
                                {
                                    if (tmp[0].IsChecked)
                                    {
                                        _chartSeries[i].IsVisible = true;
                                    }
                                    else
                                    {
                                        _chartSeries[i].IsVisible = false;
                                    }
                                }
                            }
                            else
                            {
                                byte hit = 0x0, vis = 0x0;
                                string measure = string.Empty, channel = string.Empty;
                                int roiIndex = 0;
                                string pattern = "(.*)([A|B|C|D])([0-9]+$)";
                                ParseDataSeriesName(_chartSeries[i].DataSeries.SeriesName, pattern, ref measure, ref channel, ref roiIndex);
                                //channels:
                                ChannelCheckBoxList.Clone(ref tmp);
                                tmp.Filter = p => p.Name == "Chan" + channel.ToString();
                                if (tmp.Count > 0) { hit |= 0x1; if (tmp[0].IsChecked) { vis |= 0x1; } }
                                tmp.Clear();
                                //features:
                                FeatureCheckBoxList.Clone(ref tmp);
                                tmp.Filter = f => f.Name == measure.ToString();
                                if (tmp.Count > 0) { hit |= 0x2; if (tmp[0].IsChecked) { vis |= 0x2; } }
                                tmp.Clear();
                                //ROIs:
                                ROICheckBoxList.Clone(ref tmp);
                                tmp.Filter = f => f.Name == "ROI" + roiIndex.ToString();   //"00"
                                if (tmp.Count > 0) { hit |= 0x4; if (tmp[0].IsChecked) { vis |= 0x4; } }
                                tmp.Clear();
                                //visibility:
                                if (hit == 0x7)
                                {
                                    _chartSeries[i].IsVisible = (vis == 0x7) ? true : false;
                                }
                            }
                            i++;
                        } while (true);
                    }
                    catch (Exception ex)
                    {
                        ex.ToString();
                    }
                    finally
                    {
                        if (!IsInLoad)
                        {
                            RaiseArithmeticDataChangedEvent();
                        }
                    }
                }
            }
        }

        #endregion Methods
    }

    public class CheckBoxDatabase : ViewModelBase, IEquatable<CheckBoxDatabase>, IComparable<CheckBoxDatabase>
    {
        #region Fields

        private CustomCollection<CheckBoxDatabase> _databases;
        private int _id;
        private bool _isChecked;
        private string _name;
        private Brush _textColor;

        #endregion Fields

        #region Properties

        public CustomCollection<CheckBoxDatabase> Databases
        {
            get { return _databases; }
            set
            {
                _databases = value;
                OnPropertyChanged("Databases");
            }
        }

        public int ID
        {
            get { return _id; }
            set
            {
                _id = value;
                OnPropertyChanged("ID");
            }
        }

        public bool IsChecked
        {
            get { return _isChecked; }
            set
            {
                _isChecked = value;
                OnPropertyChanged("IsChecked");
            }
        }

        public string Name
        {
            get { return _name; }
            set
            {
                _name = value;
                OnPropertyChanged("Name");
            }
        }

        public Brush TextColor
        {
            get { return _textColor; }
            set
            {
                _textColor = value;
                OnPropertyChanged("TextColor");
            }
        }

        #endregion Properties

        #region Methods

        // Default comparer for CheckBoxDatabase type:
        public int CompareTo(CheckBoxDatabase comparePart)
        {
            // A null value means that this object is greater.
            if (comparePart == null)
                return 1;

            else
                return this.ID.CompareTo(comparePart.ID);
        }

        public override bool Equals(object obj)
        {
            if (obj == null) return false;
            CheckBoxDatabase objAsPart = obj as CheckBoxDatabase;
            if (objAsPart == null) return false;
            else return Equals(objAsPart);
        }

        public bool Equals(CheckBoxDatabase other)
        {
            if (other == null) return false;
            return (this.ID.Equals(other.ID));
        }

        public override int GetHashCode()
        {
            return ID;
        }

        #endregion Methods
    }
}