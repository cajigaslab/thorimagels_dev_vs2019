namespace PowerControl
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
    /// Interaction logic for PockelsPlot.xaml
    /// </summary>
    public partial class PockelsPlotWin : Window
    {
        #region Fields

        private static int _nColorChannels = 1;
        private static Color[] _pockelsPlotColor;

        List<double[]> _dataX;
        List<double[]> _dataY;
        private string _horizontalAxisTitle;
        private int _initialChildrenCount;
        private string _verticalAxisTitle;

        #endregion Fields

        #region Constructors

        public PockelsPlotWin()
        {
            InitializeComponent();

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

            _pockelsPlotColor = new Color[_nColorChannels];

            _pockelsPlotColor[0] = Colors.Black;

            LineGraph lg = new LineGraph();

            if (_nColorChannels > 0)    // plotter init
            {
                _dataX = new List<double[]>();
                _dataY = new List<double[]>();

                double[] dataXOneCh = new double[1];
                double[] dataYOneCh = new double[1];

                dataXOneCh[0] = 0;
                dataYOneCh[0] = 0;

                for (int i = 0; i < _nColorChannels; i++)
                {
                    _dataX.Add(dataXOneCh);    // data x-y mapping init
                    _dataY.Add(dataYOneCh);

                    EnumerableDataSource<double> xOneCh = new EnumerableDataSource<double>(dataXOneCh);
                    EnumerableDataSource<double> yOneCh = new EnumerableDataSource<double>(dataYOneCh);

                    xOneCh.SetXMapping(xVal => xVal);
                    yOneCh.SetXMapping(yVal => yVal);

                    CompositeDataSource dsOneCh = new CompositeDataSource(xOneCh, yOneCh);

                    lg = plotter.AddLineGraph(dsOneCh, _pockelsPlotColor[i], 1, "Data");

                    lg.FilteringEnabled = false;
                }

                plotter.FitToView();
            }
            else
            {
                return;
            }
        }

        #endregion Constructors

        #region Properties

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

        public void RedrawLinePlot(int nColorChannels)
        {
            int startIndex = _initialChildrenCount;

            if ((_nColorChannels > 0) && (_dataX != null) && (_dataY != null))
            {
                CompositeDataSource[] dsCh = new CompositeDataSource[_nColorChannels];

                for (int i = 0; i < nColorChannels; i++)    // color reset
                {
                    ((LineGraph)plotter.Children.ElementAt(startIndex + i)).LinePen = new Pen(new SolidColorBrush(Colors.Transparent), 1);
                }

                for (int i = 0; i < _nColorChannels; i++)
                {
                    if (_dataX[i].Length == _dataY[i].Length)
                    {
                        EnumerableDataSource<double> xOneCh = new EnumerableDataSource<double>(_dataX[i]);
                        xOneCh.SetXMapping(xVal => xVal);
                        EnumerableDataSource<double> yOneCh = new EnumerableDataSource<double>(_dataY[i]);
                        yOneCh.SetYMapping(yVal => yVal);
                        CompositeDataSource ds = new CompositeDataSource(xOneCh, yOneCh);

                        ((LineGraph)plotter.Children.ElementAt(startIndex + i)).DataSource = ds;

                        ((LineGraph)plotter.Children.ElementAt(startIndex + i)).LinePen = new Pen(new SolidColorBrush(_pockelsPlotColor[i]), 1);
                    }
                }

                plotter.FitToView();
            }
        }

        public void SetData(double[] DataX, double[] DataY, int nColorChannels, int channelIndex, bool redraw)
        {
            _dataX[channelIndex] = DataX;
            _dataY[channelIndex] = DataY;

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