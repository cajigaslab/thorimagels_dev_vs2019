namespace SLMControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    using CurveFitting;

    using Microsoft.Win32;

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
    using SciChart.Charting.Visuals.PointMarkers;
    using SciChart.Charting.Visuals.RenderableSeries;
    using SciChart.Core;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;
    using SciChart.Drawing.VisualXcceleratorRasterizer;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for PowerPlot.xaml
    /// </summary>
    public partial class ZDefocusPlotWin : Window
    {
        #region Fields

        public static Dictionary<string, ZDefocusPlotType> ZDefocusPlotTypeDictionary = new Dictionary<string, ZDefocusPlotType>()
        {
        {"CLEARALL", ZDefocusPlotType.CLEARALL},
        {"UNDO", ZDefocusPlotType.UNDO},
        {"RECORD", ZDefocusPlotType.RECORD},
        {"OK", ZDefocusPlotType.OK}
        };

        const string Z_CALIBRATION = "ZCalibration.txt";

        private double[] _defocusParams = { 0.0, 1.33, 4.5, 0.0 }; //[0]:defocus z in [um], [1]:refractive index, [2]:effective focal length in [mm], [3]:saved defocus z in [um]
        private Mutex _mDefocus = new Mutex();
        private RelayCommandWithParam _zDefocusPlotCommand;
        private Stack<double> _zPositionHistory = new Stack<double>();

        #endregion Fields

        #region Constructors

        public ZDefocusPlotWin()
        {
            InitializeComponent();
            this.DataContext = this;
            this.Loaded += ZDefocusPlot_Loaded;
        }

        #endregion Constructors

        #region Enumerations

        public enum ZDefocusPlotType
        {
            CLEARALL,
            UNDO,
            RECORD,
            OK
        }

        #endregion Enumerations

        #region Properties

        /// <summary>
        /// Z PositionUM in Z Calibration
        /// </summary>
        public static List<double> DataX
        {
            get; set;
        }

        /// <summary>
        /// Z DefocusUM in Z Calibration
        /// </summary>
        public static List<double> DataY
        {
            get; set;
        }

        public static string ZCalibrationFile
        {
            get { return ResourceManagerCS.GetCaptureTemplatePathString() + Z_CALIBRATION; }
        }

        public static int ZInvert
        {
            get
            {
                return (bool)MVMManager.Instance["ZControlViewModel", "ZInvertDevice", (object)true] ? (-1) : 1;
            }
        }

        public double DefocusUM
        {
            get
            {
                ResourceManagerCS.GetDeviceParamBuffer((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_DEFOCUS, _defocusParams, _defocusParams.Length);
                return _defocusParams[0];
            }
            set
            {
                _defocusParams[0] = value;
                SetDefocusParam();
            }
        }

        public RelayCommandWithParam ZDefocusPlotCommand
        {
            get
            {
                if (this._zDefocusPlotCommand == null)
                    this._zDefocusPlotCommand = new RelayCommandWithParam(ZDefocusFunc);

                return this._zDefocusPlotCommand;
            }
        }

        public Stack<double> ZPositionHistory
        {
            get { return _zPositionHistory; }
            set { _zPositionHistory = value; }
        }

        #endregion Properties

        #region Methods

        public static bool LoadZCalibration()
        {
            bool retVal = false;
            try
            {
                if (File.Exists(ZCalibrationFile))
                {
                    //DataX = File.ReadAllLines(zCalibrationFile).Select(l => double.Parse(l.Split(',')[0])).ToArray();
                    //DataY = File.ReadAllLines(zCalibrationFile).Select(l => double.Parse(l.Split(',')[1])).ToArray();
                    double[][] list = File.ReadAllLines(ZCalibrationFile).Select(l =>
                    l.Split(',').Select(i => double.Parse(i)).ToArray()).Select(item => item.Select(r => r * ZInvert).ToArray()).OrderBy(col => col[0]).ToArray();
                    DataX = (from col in list select col[0]).ToList();
                    DataY = (from col in list select col[1]).ToList();
                    retVal = true;
                }
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "LoadZCalibration: " + ex.Message);
            }
            return retVal;
        }

        private void RedrawLinePlot()
        {
            try
            {
                if ((DataX != null) && (DataY != null) &&
                    DataX.Count == DataY.Count && 0 < DataX.Count)
                {
                    //consider z invert to plot
                    var orderInvertDataPair = DataX.Select(x => x *= ZInvert).Zip(DataY.Select(y => y *= ZInvert), (x, y) => new { x, y }).OrderBy(pair => pair.x).ToList();
                    List<double> plotDataX = orderInvertDataPair.Select(pair => pair.x).ToList();
                    List<double> plotDataY = orderInvertDataPair.Select(pair => pair.y).ToList();
                    //plot data
                    sciChartSurface.RenderableSeries = new ObservableCollection<IRenderableSeries>();
                    var series = new XyDataSeries<double, double> { FifoCapacity = null, AcceptsUnsortedData = false };
                    series.Append(plotDataX, plotDataY);
                    var l = new FastLineRenderableSeries
                    {
                        StrokeThickness = 2,
                        Stroke = Colors.GreenYellow,
                        IsVisible = true,
                        ResamplingMode = SciChart.Data.Numerics.ResamplingMode.Auto,
                        AntiAliasing = false,
                        DataSeries = series,
                        PointMarker = new EllipsePointMarker()
                    };
                    l.PointMarker.Width = 10;
                    l.PointMarker.Height = 10;
                    l.PointMarker.Fill = Colors.IndianRed;
                    l.PointMarker.Stroke = Colors.IndianRed;
                    l.PointMarker.StrokeThickness = 1;
                    sciChartSurface.RenderableSeries.Add(l);

                    //data fitting
                    if (1 < plotDataX.Count)
                    {
                        double[] fitTmpX, fitTmpY;
                        CubicSpline.FitParametric(plotDataX.ToArray(), plotDataY.ToArray(), 100, out fitTmpX, out fitTmpY);
                        var orderedZip = fitTmpX.Zip(fitTmpY, (x, y) => new { x, y }).OrderBy(pair => pair.x).ToList();
                        fitTmpX = orderedZip.Select(pair => pair.x).ToArray();
                        fitTmpY = orderedZip.Select(pair => pair.y).ToArray();
                        var fitSeries = new XyDataSeries<double, double> { FifoCapacity = null, AcceptsUnsortedData = false };
                        fitSeries.Append(fitTmpX, fitTmpY);
                        var lfit = new FastLineRenderableSeries
                        {
                            StrokeThickness = 2,
                            Stroke = Colors.OrangeRed,
                            IsVisible = true,
                            ResamplingMode = SciChart.Data.Numerics.ResamplingMode.Auto,
                            AntiAliasing = false,
                            DataSeries = fitSeries,
                        };
                        sciChartSurface.RenderableSeries.Add(lfit);
                    }
                    ZPositionXAxis.VisibleRange = new DoubleRange(plotDataX.Min(), plotDataX.Max());
                    DefocusYAxis.VisibleRange = new DoubleRange(plotDataY.Min(), plotDataY.Max());
                }
                else
                {
                    sciChartSurface.RenderableSeries.Clear();
                }
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "RedrawLinePlot: " + ex.Message);
            }
        }

        private void SaveZCalibration()
        {
            try
            {
                var buffer = new StringBuilder();
                IEnumerable<Tuple<double, double>> zPositionDefocusPairs = DataX.Zip(DataY, (a, b) => Tuple.Create(a * ZInvert, b * ZInvert));
                zPositionDefocusPairs.ToList().ForEach(
                    item => buffer.AppendLine(String.Format("{0},{1}", item.Item1.ToString("0.#"), item.Item2.ToString("0.#"))));
                File.WriteAllText(ZCalibrationFile, buffer.ToString());
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "SaveZCalibration: " + ex.Message);
            }
        }

        private void SetDefocusParam()
        {
            //set EffectiveFocalMM based on magnification
            double[] focalLengths = (double[])MVMManager.Instance["ObjectiveControlViewModel", "FocalLengths", (object)new double[4] { 200.0, 100.0, 50.0, 200.0 }];
            _defocusParams[2] = focalLengths[0] * focalLengths[2] * (double)Constants.TURRET_FOCALLENGTH_MAGNIFICATION_RATIO / (focalLengths[1] * focalLengths[3] * (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)20.0]);

            _mDefocus.WaitOne();
            ResourceManagerCS.SetDeviceParamBuffer((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_DEFOCUS, _defocusParams, _defocusParams.Length, (int)IDevice.DeviceSetParamType.NO_EXECUTION);
            _mDefocus.ReleaseMutex();
        }

        private void ZDefocusFunc(object type)
        {
            string str = (string)type;
            switch (ZDefocusPlotTypeDictionary[str])
            {
                case ZDefocusPlotType.CLEARALL:
                    if (MessageBoxResult.Yes == MessageBox.Show(string.Format("Do you want to clear z calibration?"), "Z Calibration", MessageBoxButton.YesNo, MessageBoxImage.Question))
                    {
                        DataX.Clear();
                        DataY.Clear();
                        ZPositionHistory.Clear();
                    }
                    break;
                case ZDefocusPlotType.UNDO:
                    if (0 < ZPositionHistory.Count && null != DataX && null != DataY)
                    {
                        double dZdefocus = ZPositionHistory.Pop();
                        int idx = DataX.FindIndex(x => x == dZdefocus);
                        if (0 < idx)
                        {
                            DataX.RemoveAt(idx);
                            DataY.RemoveAt(idx);
                        }
                    }
                    break;
                case ZDefocusPlotType.RECORD:
                    double zpos = ZInvert * (double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0] * (double)Constants.UM_TO_MM;
                    ZPositionHistory.Push(zpos);
                    if (null != DataX && null != DataY)
                    {
                        DataX.Add(zpos);
                        DataY.Add(DefocusUM * ZInvert);
                        var orderedZip = DataX.Zip(DataY, (x, y) => new { x, y }).OrderBy(pair => pair.x).ToList();
                        DataX = orderedZip.Select(pair => pair.x).ToList();
                        DataY = orderedZip.Select(pair => pair.y).ToList();
                    }
                    else
                    {
                        DataX = new List<double>() { zpos };
                        DataY = new List<double>() { DefocusUM * ZInvert };
                    }
                    break;
                case ZDefocusPlotType.OK:
                    this.Close();
                    return;
                default:
                    break;
            }
            RedrawLinePlot();
            SaveZCalibration();
        }

        void ZDefocusPlot_Loaded(object sender, RoutedEventArgs e)
        {
            // set panel location
            string str = string.Empty;
            double dval = 0;
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList ndList = doc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/EditBleachWindow");
            if (ndList.Count > 0)
            {
                this.Left = (XmlManager.GetAttribute(ndList[0], doc, "left", ref str) && double.TryParse(str, out dval)) ? (int)dval : 0;
                this.Top = (XmlManager.GetAttribute(ndList[0], doc, "top", ref str) && double.TryParse(str, out dval)) ? (int)dval : 0;
            }

            // setup panel content
            LoadZCalibration();
            RedrawLinePlot();
        }

        #endregion Methods
    }
}