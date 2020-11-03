namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
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

    using Microsoft.Research.DynamicDataDisplay;
    using Microsoft.Research.DynamicDataDisplay.Charts;
    using Microsoft.Research.DynamicDataDisplay.DataSources;
    using Microsoft.Research.DynamicDataDisplay.PointMarkers;
    using Microsoft.Win32;

    /// <summary>
    /// Interaction logic for PlotterControl.xaml
    /// </summary>
    public partial class LineProfile : Window
    {
        #region Fields

        private static Point _endPoint;
        private static Point _startPoint;

        private Color _dataBrushColor;
        double[] _dataX;
        double[] _dataY;

        //DefaultContextMenu menu;
        private string _horizontalAxisTitle;
        private int _initialChildrenCount;
        private string _title;
        private string _verticalAxisTitle;

        #endregion Fields

        #region Constructors

        public LineProfile()
        {
            InitializeComponent();

            _dataBrushColor = Colors.Black;

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

            _dataX = new double[1];
            _dataY = new double[1];

            _dataX[0] = 0;
            _dataY[0] = 0;

            EnumerableDataSource<double> x = new EnumerableDataSource<double>(_dataX);

            x.SetXMapping(xVal => xVal);

            EnumerableDataSource<double> y = new EnumerableDataSource<double>(_dataY);

            y.SetYMapping(yVal => yVal);

            CompositeDataSource ds = new CompositeDataSource(x, y);

            LineGraph lg = plotter.AddLineGraph(ds, Colors.Black, 1, "Data");

            lg.FilteringEnabled = false;

            plotter.FitToView();
        }

        #endregion Constructors

        #region Events

        public event Action<int> LineWidthChange;

        #endregion Events

        #region Properties

        public static Point EndPointProperty
        {
            get { return _endPoint; }
            set { _endPoint = value; }
        }

        //Line Profile Code for adorners :Start
        public static Point StartPointProperty
        {
            get { return _startPoint; }
            set { _startPoint = value; }
        }

        public double[] DataX
        {
            set
            {
                try
                {
                    _dataX = value;
                    RedrawLinePlot();
                }
                catch (Exception ex)
                {
                    string str = ex.Message;
                }
            }
            get
            {
                return _dataX;
            }
        }

        public double[] DataY
        {
            set
            {
                try
                {
                   _dataY = value;
                   RedrawLinePlot();
                }
                catch (Exception ex)
                {
                    string str = ex.Message;
                }
            }
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

        public string LTitle
        {
            get
            {
                return _title;
            }
            set
            {
                _title = value;
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

            EnumerableDataSource<double> x = new EnumerableDataSource<double>(_dataX);
            x.SetXMapping(xVal => xVal);

            EnumerableDataSource<double> y = new EnumerableDataSource<double>(_dataY);

            y.SetYMapping(yVal => yVal);

            CompositeDataSource ds = new CompositeDataSource(x, y);

            ((LineGraph)plotter.Children.ElementAt(startIndex)).DataSource = ds;

            plotter.FitToView();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            LineWidthChange(Convert.ToInt32(txtLineWidth.Text));
        }

        #endregion Methods

        #region Other

        //Line Profile code for adorners: End

        #endregion Other
    }
}