namespace DataHistogramControl
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
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
    using System.Windows.Threading;

    using Microsoft.Research.DynamicDataDisplay;
    using Microsoft.Research.DynamicDataDisplay.Charts;
    using Microsoft.Research.DynamicDataDisplay.DataSources;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class Histogram : UserControl, INotifyPropertyChanged
    {
        #region Fields

        private Func<int, double> linearMapping = i => i;
        private int _gateMax;
        private int _gateMin;
        int[] _histogram;
        private string _horizontalAxisTitle;
        private int _initialChildrenCount;
        private int _totalChildrenCount;
        private string _title;
        private string _verticalAxisTitle;
        private Visibility _gateVisibility = Visibility.Hidden;
        private double _xScaleFactor;

        public int[] _OriginalData = null;
        int changeCount = 0;

        #endregion Fields

        #region Constructors

        public Histogram()
        {
            InitializeComponent();

            const int BINS = 256;

            _histogram = new int[BINS];

            for (int i = 0; i < BINS; i++)
            {
                _histogram[i] = 0;
            }

            _gateMin = 0;

            _gateMax = BINS - 1;

            plotter.AxisGrid.Remove();
            plotter.Legend.Remove();

            _xScaleFactor = 1.0;

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

            EnumerableDataSource<int> x = new EnumerableDataSource<int>(Enumerable.Range(0, BINS).ToArray());

            x.SetXMapping(_x => _x);

            EnumerableDataSource<int> _histogramSource;

            _histogramSource = new EnumerableDataSource<int>(_histogram);

            Func<int, double> mapping = linearMapping;

            _histogramSource.SetYMapping(mapping);

            CompositeDataSource ds = new CompositeDataSource(x, _histogramSource);

            LineGraph lg = plotter.AddLineGraph(ds, Colors.Transparent, 1, "Data");
            lg.FilteringEnabled = false;

            for (int i = 0; i < BINS; i++)
            {
                Segment line = new Segment();

                line.StartPoint = new Point(i, 0);
                line.EndPoint = new Point(i, _histogram[i]);

                //do not show a line if the count is zero
                if (_histogram[i] == 0)
                {
                    line.Visibility = Visibility.Hidden;
                }

                line.Stroke = Brushes.Black;
                line.StrokeThickness = 1;
                plotter.Children.Add(line);
            }

            VerticalLine GateMin = new VerticalLine();
            GateMin.Stroke = Brushes.Blue;
            GateMin.StrokeThickness = 3;
            GateMin.Visibility = _gateVisibility;
            GateMin.Value = _gateMin;
            plotter.Children.Add(GateMin);

            VerticalLine GateMax = new VerticalLine();
            GateMax.Stroke = Brushes.Red;
            GateMax.StrokeThickness = 3;
            GateMax.Visibility = _gateVisibility;
            GateMax.Value = _gateMax;
            plotter.Children.Add(GateMax);

            VerticalAxisTitle vAxisTitle = new VerticalAxisTitle();
            vAxisTitle.Content = _verticalAxisTitle;
            plotter.Children.Add(vAxisTitle);

            HorizontalAxisTitle hAxisTitle = new HorizontalAxisTitle();
            hAxisTitle.Content = _horizontalAxisTitle;
            plotter.Children.Add(hAxisTitle);

            Header header = new Header();
            header.Content = _title;
            plotter.Children.Add(header);

            _totalChildrenCount = plotter.Children.Count;

            plotter.FitToView();
        }

        #endregion Constructors

        #region Events

        public event Action<int> GateMax_Changed;

        public event Action<int> GateMin_Changed;

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties        

        public int[] Data
        {
            get
            {

                return _histogram;
            }
            set
            {


                if (_histogram.Length != value.Length)
                {
                    //reallocate the data storage
                    _histogram = value;

                }
                if (changeCount == 0)
                {
                    _OriginalData = _histogram;
                    changeCount++;
                }

                RedrawHistogram();
            }
        }

        public int GateMax
        {
            get
            {
                return _gateMax;
            }
            set
            {
                _gateMax = value;
                GateMax_Changed(value);
                OnPropertyChanged("GateMax");
                OnPropertyChanged("Data");
            }
        }

        public int GateMin
        {
            get
            {
                return _gateMin;
            }
            set
            {
                _gateMin = value;
                GateMin_Changed(value);
                OnPropertyChanged("GateMin");
                OnPropertyChanged("Data");
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

        public int MaxValue
        {
            get
            {
                int i = 0;
                for (i = _histogram.Length; i > 0; i--)
                {
                    if (_histogram[i - 1] > 0)
                    {
                        break;
                    }
                }

                return i;
            }
        }

        public int MinValue
        {
            get
            {
                int i = 0;
                for (i = 0; i < _histogram.Length; i++)
                {
                    if (_histogram[i] > 0)
                    {
                        break;
                    }
                }

                return i;
            }
        }

        public string Title
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

        public Visibility GateVisibility
        {
            get
            {
                return _gateVisibility;
            }            
            set
            {
                _gateVisibility = value;
            }
        }

        private bool Redraw
        {
            get
            {
                return true;
            }
        }

        public double XScaleFactor
        {
            get
            {
                return _xScaleFactor;
            }
            set
            {
                _xScaleFactor = value;
            }
        }

        #endregion Properties

        #region Methods

        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private void RedrawHistogram()
        {
            int i = 0;
            int startIndex = _initialChildrenCount;

            EnumerableDataSource<int> x = new EnumerableDataSource<int>(Enumerable.Range(0, _histogram.Length).ToArray());

            x.SetXMapping(_x =>(_xScaleFactor * _x));

            EnumerableDataSource<int> _histogramSource;

            _histogramSource = new EnumerableDataSource<int>(_histogram);

            Func<int, double> mapping = linearMapping;

            _histogramSource.SetYMapping(mapping);

            CompositeDataSource ds = new CompositeDataSource(x, _histogramSource);

            ((LineGraph)plotter.Children.ElementAt(startIndex)).DataSource = ds;

            _totalChildrenCount = _initialChildrenCount + _histogram.Length;

            //rebuild the graph is the number of lines changes
            if (_totalChildrenCount != plotter.Children.Count)
            {
                int count = plotter.Children.Count;

                //do not remove the initial children
                if (count > _initialChildrenCount)
                {
                    for ( i = count - 1; i >= _initialChildrenCount; i--)
                    {
                        plotter.Children.RemoveAt(i);
                    }
                }

                LineGraph lg = plotter.AddLineGraph(ds, Colors.Transparent, 1, "Data");
                lg.FilteringEnabled = false;

                for ( i = 0; i < _histogram.Length; i++)
                {
                    Segment line = new Segment();
                    line.StartPoint = new Point(i*_xScaleFactor, 0);
                    line.EndPoint = new Point(i * _xScaleFactor, _histogram[i]);
                    line.Stroke = Brushes.Black;
                    line.StrokeThickness = 1;

                    //do not show a line if the count is zero
                    if (_histogram[i] == 0)
                    {
                        line.Visibility = Visibility.Hidden;
                    }

                    plotter.Children.Add(line);
                }

                VerticalLine GateMin = new VerticalLine();
                GateMin.Stroke = Brushes.Blue;
                GateMin.StrokeThickness = 3;
                GateMin.Visibility = _gateVisibility;
                GateMin.Value = _gateMin;
                plotter.Children.Add(GateMin);

                VerticalLine GateMax = new VerticalLine();
                GateMax.Stroke = Brushes.Red;
                GateMax.StrokeThickness = 3;
                GateMax.Visibility = _gateVisibility;
                GateMax.Value = _gateMax;
                plotter.Children.Add(GateMax);

                VerticalAxisTitle vAxisTitle = new VerticalAxisTitle();
                vAxisTitle.Content = _verticalAxisTitle;
                plotter.Children.Add(vAxisTitle);

                HorizontalAxisTitle hAxisTitle = new HorizontalAxisTitle();
                hAxisTitle.Content = _horizontalAxisTitle;
                plotter.Children.Add(hAxisTitle);

                Header header = new Header();
                header.Content = _title;
                plotter.Children.Add(header);
            }

            int j = startIndex + 1;

            for (i = 0; i < _histogram.Length; i++)
            {
                ((Segment)plotter.Children.ElementAt(j)).StartPoint = new Point(i * _xScaleFactor, 0);
                ((Segment)plotter.Children.ElementAt(j)).EndPoint = new Point(i * _xScaleFactor, _histogram[i]);
                ((Segment)plotter.Children.ElementAt(j)).StrokeThickness = (this.Width / (_histogram.Length));

                j++;
            }

            ((VerticalLine)plotter.Children.ElementAt(j)).Value = _gateMin;
            j++;
            ((VerticalLine)plotter.Children.ElementAt(j)).Value = _gateMax;
            j++;
            ((VerticalAxisTitle)plotter.Children.ElementAt(j)).Content = _verticalAxisTitle;
            j++;
            ((HorizontalAxisTitle)plotter.Children.ElementAt(j)).Content = _horizontalAxisTitle;
            j++;
            ((Header)plotter.Children.ElementAt(j)).Content = _title;

            plotter.FitToView();
        }

        #endregion Methods
    }
}