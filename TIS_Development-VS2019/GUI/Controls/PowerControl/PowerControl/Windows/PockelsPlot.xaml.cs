namespace PowerControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

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
    using SciChart.Charting.Visuals.Axes.AxisBandProviders;
    using SciChart.Charting.Visuals.Axes.LabelProviders;
    using SciChart.Charting.Visuals.Events;
    using SciChart.Charting.Visuals.RenderableSeries;
    using SciChart.Core;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;
    using SciChart.Drawing.VisualXcceleratorRasterizer;

    /// <summary>
    /// Interaction logic for PockelsPlot.xaml
    /// </summary>
    public partial class PockelsPlotWin : Window
    {
        #region Fields

        double[] _dataX;
        double[] _dataY;

        #endregion Fields

        #region Constructors

        public PockelsPlotWin()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public int CurrentPockelsIndex
        {
            get;
            set;
        }

        public double PockelsActiveVoltageMax
        {
            get; set;
        }

        public double PockelsActiveVoltageMin
        {
            get;set;
        }

        #endregion Properties

        #region Methods

        public void RedrawLinePlot(int nColorChannels)
        {
            if ((_dataX != null) && (_dataY != null))
            {
                if (_dataX.Length == _dataY.Length)
                {
                    sciChartSurface.RenderableSeries = new ObservableCollection<IRenderableSeries>();
                    XyDataSeries<double, double> series = new XyDataSeries<double, double> { FifoCapacity = null, AcceptsUnsortedData = false };
                    series.Append(_dataX, _dataY);
                    var l = new FastLineRenderableSeries
                    {
                        StrokeThickness = 2,
                        Stroke = Colors.GreenYellow,
                        IsVisible = true,
                        ResamplingMode = SciChart.Data.Numerics.ResamplingMode.Auto,
                        AntiAliasing = false,
                        DataSeries = series
                    };

                    sciChartSurface.RenderableSeries.Add(l);
                    var axisBandsProvider = new NumericAxisBandsProvider();
                    var fillColor = Color.FromArgb(55, Colors.LightBlue.R, Colors.LightBlue.G, Colors.LightBlue.B);
                    axisBandsProvider.AxisBands.Add(new AxisBandInfo<DoubleRange>(new DoubleRange(PockelsActiveVoltageMin, PockelsActiveVoltageMax), fillColor));
                    XAxis.AxisBandsProvider = axisBandsProvider;
                }
            }
        }

        public void SetData(double[] DataX, double[] DataY, int nColorChannels, int channelIndex, bool redraw)
        {
            _dataX = DataX;
            _dataY = DataY;

            if (redraw)
            {
                RedrawLinePlot(nColorChannels);
            }
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        #endregion Methods
    }
}