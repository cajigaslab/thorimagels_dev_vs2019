namespace KuriosControl.View
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
    using System.Windows.Shapes;
    using System.Xml;

    using KuriosControl.ViewModel;

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

    /// <summary>
    /// Interaction logic for WavelengthExposureEditWindow.xaml
    /// </summary>
    public partial class WavelengthExposureEditWindow : Window
    {
        #region Fields

        double[] _dataX;
        double[] _dataY;

        #endregion Fields

        #region Constructors

        public WavelengthExposureEditWindow()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public double[] DataX
        {
            get
            {
                return _dataX;
            }
        }

        public double[] DataY
        {
            get
            {
                return _dataY;
            }
        }

        #endregion Properties

        #region Methods

        public void RedrawLinePlot()
        {
            if ((_dataX != null) && (_dataY != null))
            {
                if (_dataX.Length == _dataY.Length)
                {
                    sciChartSurface.RenderableSeries = new ObservableCollection<IRenderableSeries>();
                    var series = new XyDataSeries<double, double> { FifoCapacity = null, AcceptsUnsortedData = false };
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
                }
            }
        }

        public void SetData(double[] dataX, double[] dataY, bool redraw)
        {
            if (null == this.DataContext)
            {
                return;
            }

            if ((null == dataX) || (null == dataY))
            {
                return;
            }

            ControlViewModel vm = (ControlViewModel)this.DataContext;

            double range = vm.SequenceParameter.SeqWavelengthStop - vm.SequenceParameter.SeqWavelengthStart;

            double[] x = new double[dataX.Length];

            for (int i = 0; i < dataX.Length; i++)
            {
                x[i] = vm.SequenceParameter.SeqWavelengthStart + dataX[i] / ControlViewModel.MAX_SEQUENCE_DATA_POINTS * range;
            }

            _dataX = x;
            _dataY = dataY;

            if (redraw)
            {
                RedrawLinePlot();
            }
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        #endregion Methods
    }
}