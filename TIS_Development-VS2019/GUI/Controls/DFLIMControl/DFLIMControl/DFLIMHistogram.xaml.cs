namespace DFLIMControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.Globalization;
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

    using Abt.Controls.SciChart;
    using Abt.Controls.SciChart.Model.DataSeries;
    using Abt.Controls.SciChart.Visuals;
    using Abt.Controls.SciChart.Visuals.RenderableSeries;

    using Microsoft.Research.DynamicDataDisplay;
    using Microsoft.Research.DynamicDataDisplay.Charts;
    using Microsoft.Research.DynamicDataDisplay.DataSources;
    using Microsoft.Research.DynamicDataDisplay.PointMarkers;

    using ROIStatsChart.Model;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for DFLIMHistogram.xaml
    /// </summary>
    public partial class DFLIMHistogram : UserControl
    {
        #region Constructors

        public DFLIMHistogram()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Methods

        public void Plot(Dictionary<KeyValuePair<int, SolidColorBrush>, uint[]> data)
        {
            try
            {
                ObservableCollection<IChartSeriesViewModel> chartSeries = new ObservableCollection<IChartSeriesViewModel>();
                foreach (var dataSet in data)
                {
                    IXyDataSeries<double, double> ds0;
                    if (dataSet.Value[254] == 0)
                    {
                        continue;
                    }
                    ds0 = new XyDataSeries<double, double> { FifoCapacity = null, SeriesName = dataSet.Key.ToString() };
                    double binDuration = 5.0 * (double)dataSet.Value[255] / 128.0 / (double)dataSet.Value[254];
                    double[] y = new double[241];
                    double[] x = new double[241];
                    for (int i = 0; i < x.Length; ++i)
                    {
                        if (dataSet.Value[i] < 0 || (i + 1) * binDuration < 0)
                        {
                            //invalid value, return
                            return;
                        }

                        x[i] = (i + 1) * binDuration;
                        y[i] = dataSet.Value[i];
                    }

                    ds0.Append(x, y);
                    var color = dataSet.Key.Value.Color;
                    var vm = new ChartSeriesViewModel(ds0, new XyScatterRenderableSeries() { Tag = dataSet.Key.Key.ToString(), StrokeThickness = 1, IsVisible = true, SeriesColor = color, PointMarker = new Abt.Controls.SciChart.Visuals.PointMarkers.CrossPointMarker() { Width = 1.5, Height = 1.5, StrokeThickness = 2 } });

                    vm.RenderSeries.IsVisible = true;

                    chartSeries.Add(vm);
                }

                sciChartSurface.SeriesSource = chartSeries;

                sciChartSurface.ZoomExtents();
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        /// <summary>
        /// Zooms the chart to the extent of the current data
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void FitChartToData(object sender, EventArgs e)
        {
            sciChartSurface.ZoomExtents();
        }

        private Color GetLineColor(int channel)
        {
            switch (channel)
            {
                case 0: return ((SolidColorBrush)(Brush)MVMManager.Instance["CaptureSetupViewModel", "LSMChannelColor0"]).Color;
                case 1: return ((SolidColorBrush)(Brush)MVMManager.Instance["CaptureSetupViewModel", "LSMChannelColor1"]).Color;
                case 2: return ((SolidColorBrush)(Brush)MVMManager.Instance["CaptureSetupViewModel", "LSMChannelColor2"]).Color;
                case 3: return ((SolidColorBrush)(Brush)MVMManager.Instance["CaptureSetupViewModel", "LSMChannelColor3"]).Color;
            }
            return Colors.DarkOrange;
        }

        #endregion Methods

        #region Other

        //public ObservableCollection<IChartSeriesViewModel> ChartSeries
        //{
        //    get { return _chartSeries; }
        //}

        #endregion Other
    }

    public class LogarithmicBaseConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var str = (string)value;

            var result = str.ToUpperInvariant().Equals("E") ? Math.E : Double.Parse(str, CultureInfo.InvariantCulture);

            return result;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        #endregion Methods
    }
}