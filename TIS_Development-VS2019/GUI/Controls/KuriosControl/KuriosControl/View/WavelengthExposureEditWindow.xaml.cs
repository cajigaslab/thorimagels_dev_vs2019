namespace KuriosControl.View
{
    using System;
    using System.Collections.Generic;
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

    using KuriosControl.ViewModel;

    using Microsoft.Research.DynamicDataDisplay;
    using Microsoft.Research.DynamicDataDisplay.Charts;
    using Microsoft.Research.DynamicDataDisplay.DataSources;
    using Microsoft.Research.DynamicDataDisplay.PointMarkers;

    /// <summary>
    /// Interaction logic for WavelengthExposureEditWindow.xaml
    /// </summary>
    public partial class WavelengthExposureEditWindow : Window
    {
        #region Fields

        private static Color _powerPlotColor;

        double[] _dataX;
        double[] _dataY;
        private string _horizontalAxisTitle;
        private int _initialChildrenCount;
        CirclePointMarker _marker;
        Pen _pen;
        private string _verticalAxisTitle;

        #endregion Fields

        #region Constructors

        public WavelengthExposureEditWindow()
        {
            InitializeComponent();
            this.Loaded += WavelengthExposureEditWindow_Loaded;
            this.Unloaded += WavelengthExposureEditWindow_Unloaded;
            plotter.Legend.Remove();

            _initialChildrenCount = plotter.Children.Count;

            int count = plotter.Children.Count;

            //do not remove the initial children
            if (count > _initialChildrenCount)
            {
                for (int i = count - 1; i >= _initialChildrenCount; i--)
                {
                    plotter.Children.RemoveAt(i);
                }
            }

            _pen = new Pen(Brushes.Black, 1);

            _powerPlotColor = new Color();

            _powerPlotColor = Colors.Black;

            _marker = new CirclePointMarker() { Size = 10, Fill = Brushes.Red };

            LineAndMarker<MarkerPointsGraph> lg = new LineAndMarker<MarkerPointsGraph>();

            double[] dataXOneCh = new double[1];
            double[] dataYOneCh = new double[1];

            dataXOneCh[0] = 0;
            dataYOneCh[0] = 0;

            _dataX = dataXOneCh;    // data x-y mapping init
            _dataY = dataYOneCh;

            EnumerableDataSource<double> xOneCh = new EnumerableDataSource<double>(dataXOneCh);
            EnumerableDataSource<double> yOneCh = new EnumerableDataSource<double>(dataYOneCh);

            xOneCh.SetXMapping(xVal => xVal);
            yOneCh.SetXMapping(yVal => yVal);

            CompositeDataSource dsOneCh = new CompositeDataSource(xOneCh, yOneCh);

            lg = plotter.AddLineGraph(dsOneCh, _pen, _marker, null);

            //    lg.FilteringEnabled = false;

            plotter.FitToView();
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

        public string HorizontalAxisTitle
        {
            get
            {
                return _horizontalAxisTitle;
            }
            set
            {
                _horizontalAxisTitle = value;
            }
        }

        public string VerticalAxisTitle
        {
            get
            {
                return _verticalAxisTitle;
            }
            set
            {
                _verticalAxisTitle = value;
            }
        }

        #endregion Properties

        #region Methods

        public void RedrawLinePlot()
        {
            int startIndex = _initialChildrenCount;

            if ((_dataX != null) && (_dataY != null))
            {
                CompositeDataSource[] dsCh = new CompositeDataSource[1];

                plotter.Children.RemoveAt(startIndex);
                plotter.Children.RemoveAt(startIndex);

                if (_dataX.Length == _dataY.Length)
                {
                    EnumerableDataSource<double> xOneCh = new EnumerableDataSource<double>(_dataX);
                    xOneCh.SetXMapping(xVal => xVal);
                    EnumerableDataSource<double> yOneCh = new EnumerableDataSource<double>(_dataY);
                    yOneCh.SetYMapping(yVal => yVal);
                    CompositeDataSource ds = new CompositeDataSource(xOneCh, yOneCh);

                    plotter.AddLineGraph(ds, _pen, _marker, null);
                }
            }

            plotter.FitToView();
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

        void WavelengthExposureEditWindow_Loaded(object sender, RoutedEventArgs e)
        {
            //throw new NotImplementedException();
        }

        void WavelengthExposureEditWindow_Unloaded(object sender, RoutedEventArgs e)
        {
            //throw new NotImplementedException();
        }

        #endregion Methods
    }
}