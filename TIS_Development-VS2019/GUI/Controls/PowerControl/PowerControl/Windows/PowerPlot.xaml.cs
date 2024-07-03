namespace PowerControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
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
    using SciChart.Charting3D;
    using SciChart.Core;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;
    using SciChart.Drawing.VisualXcceleratorRasterizer;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for PowerPlot.xaml
    /// </summary>
    public partial class PowerPlotWin : Window
    {
        #region Fields

        double[] _dataX;
        double[] _dataY;

        #endregion Fields

        #region Constructors

        public PowerPlotWin()
        {
            InitializeComponent();
            this.Loaded += PowerPlot_Loaded;
            this.Unloaded += PowerPlot_Unloaded;
        }

        #endregion Constructors

        #region Properties

        public bool CreationMode
        {
            get;
            set;
        }

        public int CurrentPockelsIndex
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        public void RedrawLinePlot()
        {
            if ((_dataX != null) && (_dataY != null))
            {
                if (_dataX.Length == _dataY.Length)
                {
                    double zScanStart = (double)MVMManager.Instance["ZControlViewModel", "ZScanStart", (object)0.0];
                    double zScanStop = (double)MVMManager.Instance["ZControlViewModel", "ZScanStop", (object)0.0];

                    if (zScanStart > zScanStop)
                    {
                        sciChartSurface.FlowDirection = FlowDirection.RightToLeft;
                    }
                    else
                    {
                        sciChartSurface.FlowDirection = FlowDirection.LeftToRight;
                    }

                    sciChartSurface.RenderableSeries = new ObservableCollection<IRenderableSeries>();
                    var series = new XyDataSeries<double, double> { FifoCapacity = null, AcceptsUnsortedData = true };

                    series.Append(_dataX, _dataY);

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
                    PowerAxis.VisibleRange = new DoubleRange(-1, 101);
                }
            }
        }

        public void SetData(double[] DataX, double[] DataY, bool redraw)
        {
            if (null == this.DataContext)
            {
                return;
            }

            if ((null == DataX) || (null == DataY))
            {
                return;
            }

            double ZScanStop = (double)MVMManager.Instance["ZControlViewModel", "ZScanStop", (object)0];
            double ZScanStart = (double)MVMManager.Instance["ZControlViewModel", "ZScanStart", (object)0];
            double range = ZScanStop - ZScanStart;

            double[] x = new double[DataX.Length];

            for (int i = 0; i < DataX.Length; i++)
            {
                x[i] = ZScanStart + ((DataX[i] / 100.0) * range);
            }

            _dataX = x;
            _dataY = DataY;

            if (redraw)
            {
                RedrawLinePlot();
            }
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        void PowerPlot_Loaded(object sender, RoutedEventArgs e)
        {
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList = doc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PowerRampWindow");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                double val = 0;
                if (XmlManager.GetAttribute(ndList[0], doc, "left", ref str))
                {
                    if (double.TryParse(str, out val))
                    {
                        this.Left = (int)val;
                    }
                    else
                    {
                        this.Left = 0;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "top", ref str))
                {
                    if (double.TryParse(str, out val))
                    {
                        this.Top = (int)val;
                    }
                    else
                    {
                        this.Top = 0;
                    }
                }
            }
        }

        void PowerPlot_Unloaded(object sender, RoutedEventArgs e)
        {
            MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);

            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList = doc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PowerRampWindow");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                XmlManager.SetAttribute(ndList[0], doc, "left", ((int)Math.Round(this.Left)).ToString());
                XmlManager.SetAttribute(ndList[0], doc, "top", ((int)Math.Round(this.Top)).ToString());

                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }
        }

        #endregion Methods
    }
}