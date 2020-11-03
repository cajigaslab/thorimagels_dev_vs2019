namespace PowerControl
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
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    using Microsoft.Research.DynamicDataDisplay;
    using Microsoft.Research.DynamicDataDisplay.Charts;
    using Microsoft.Research.DynamicDataDisplay.DataSources;
    using Microsoft.Research.DynamicDataDisplay.PointMarkers;
    using Microsoft.Win32;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for PowerPlot.xaml
    /// </summary>
    public partial class PowerPlotWin : Window
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

        public PowerPlotWin()
        {
            InitializeComponent();
            this.Loaded += PowerPlot_Loaded;
            this.Unloaded += PowerPlot_Unloaded;
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

            plotter.FitToView();
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

            //If the plot is empty or it only has one point, we need to display the area where new points can be added
            if (_dataX.Length <= 1 && _dataY.Length <= 1)
            {
                double zScanStop = (double)MVMManager.Instance["ZControlViewModel", "ZScanStop", (object)0];
                double zScanStart = (double)MVMManager.Instance["ZControlViewModel", "ZScanStart", (object)0];
                double minPowerPercentage = 0;
                double maxPowerPercentage = 100;
                // FitToView adds 5 around both x and y ranges for easier view of the plot, should do the same when manually setting the ranges
                if (zScanStart <= zScanStop)
                {
                    plotter.Visible = new Rect(zScanStart - 0.005, minPowerPercentage - 5, zScanStop - zScanStart + 0.010, maxPowerPercentage + 10);
                }
                else
                {
                    plotter.Visible = new Rect(zScanStop - 0.005, minPowerPercentage - 5, zScanStart - zScanStop + 0.010, maxPowerPercentage + 10);
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
                x[i] = ZScanStart + DataX[i] / 100.0 * range;
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