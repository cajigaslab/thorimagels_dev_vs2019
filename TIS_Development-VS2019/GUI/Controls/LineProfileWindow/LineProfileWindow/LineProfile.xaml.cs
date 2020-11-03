namespace LineProfileWindow
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
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

    using LineProfileWindow.ViewModel;

    using Microsoft.Research.DynamicDataDisplay;
    using Microsoft.Research.DynamicDataDisplay.Charts;
    using Microsoft.Research.DynamicDataDisplay.DataSources;
    using Microsoft.Research.DynamicDataDisplay.PointMarkers;
    using Microsoft.Research.DynamicDataDisplay.ViewportRestrictions;

    using ThorLogging;

    public struct LineProfileData
    {
        #region Fields

        public long channelEnable;
        public int LengthPerChannel;
        public double PixeltoµmConversionFactor;

        //public long numChannel;
        public double[] profileDataX;
        public double[][] profileDataY;

        #endregion Fields
    }

    /// <summary>
    /// Interaction logic for LineProfile.xaml
    /// </summary>
    public partial class LineProfile : Window
    {
        #region Fields

        LineProfileViewModel _vm;

        #endregion Fields

        #region Constructors

        public LineProfile(Color[] colorAssignment, int maxChannels)
        {
            InitializeComponent();

            plotter.Legend.Remove();
            _vm = new LineProfileViewModel();
            this.DataContext = _vm;
            _vm.LineWidthChange += _vm_LineWidthChange;
            _vm.MaxChannels = maxChannels;
            _vm.InitialChildrenCount = plotter.Children.Count;
            _vm.ChannelEnable = new bool[_vm.MaxChannels];
            int count = plotter.Children.Count;

            //do not remove the initial children
            if (count > _vm.InitialChildrenCount)
            {
                for (int i = count - 1; i >= _vm.InitialChildrenCount; i--)
                {
                    plotter.Children.RemoveAt(i);
                }
            }

            _vm.ColorAssigment = colorAssignment;

            LineGraph lg = new LineGraph();

            if (_vm.ColorAssigment.Length == _vm.MaxChannels)    // plotter init
            {
                double[] dataXOneCh = new double[1];
                double[] dataYOneCh = new double[1];

                dataXOneCh[0] = 0;
                dataYOneCh[0] = 0;

                for (int i = 0; i < _vm.MaxChannels; i++)
                {
                    EnumerableDataSource<double> xOneCh = new EnumerableDataSource<double>(dataXOneCh);
                    EnumerableDataSource<double> yOneCh = new EnumerableDataSource<double>(dataYOneCh);

                    xOneCh.SetXMapping(xVal => xVal);
                    yOneCh.SetXMapping(yVal => yVal);

                    CompositeDataSource dsOneCh = new CompositeDataSource(xOneCh, yOneCh);

                    lg = plotter.AddLineGraph(dsOneCh, _vm.ColorAssigment[i], 1, "Data");

                    lg.FilteringEnabled = false;
                }

                plotter.FitToView();
            }
            try
            {
                // Make this window the topmost within the app
                this.Owner = Application.Current.MainWindow;
            }
            catch (Exception e)
            {
                e.ToString();
            }
            this.Closed += LineProfile_Closed;
        }

        #endregion Constructors

        #region Events

        public event Action<int> LineWidthChange;

        #endregion Events

        #region Properties

        public bool AutoScaleOption
        {
            get
            {
                return _vm.IsAutoScaleActive;
            }
            set
            {
                _vm.IsAutoScaleActive = value;
            }
        }

        public Color[] ColorAssigment
        {
            get
            {
                return _vm.ColorAssigment;
            }
            set
            {
                _vm.ColorAssigment = value;
                RedrawLinePlot();
            }
        }

        public bool ConversionActiveOption
        {
            get
            {
                return _vm.IsConversionActive;
            }
            set
            {
                _vm.IsConversionActive = value;
            }
        }

        public int LineWidth
        {
            get
            {
                return _vm.LineWidth;
            }
            set
            {
                _vm.LineWidth = Math.Max(1, value);
            }
        }

        public int LineWidthMax
        {
            get
            {
                return _vm.LineWidthMax;
            }
            set
            {
                _vm.LineWidthMax = value;
            }
        }

        public double MaximumYVal
        {
            get
            {
                return _vm.YmaxValue;
            }
            set
            {
                _vm.YmaxValue = value;
            }
        }

        public double MinimumYVal
        {
            get
            {
                return _vm.YminValue;
            }
            set
            {
                _vm.YminValue = value;
            }
        }

        #endregion Properties

        #region Methods

        public void RedrawLinePlot()
        {
            int startIndex = _vm.InitialChildrenCount;

            if ((_vm.NumChannel > 0) && (null != _vm.LineProfileData.profileDataX) && (null != _vm.LineProfileData.profileDataY))
            {
                CompositeDataSource[] dsCh = new CompositeDataSource[_vm.NumChannel];

                for (int i = 0; i < _vm.MaxChannels; i++)    // color reset
                {
                    ((LineGraph)plotter.Children.ElementAt(startIndex + i)).LinePen = new Pen(new SolidColorBrush(Colors.Transparent), 1);
                }

                int j = 0;
                for (int i = 0; i < _vm.MaxChannels; i++)
                {
                    if (j < _vm.LineProfileData.profileDataY.Length &&
                        _vm.LineProfileData.profileDataX.Length == _vm.LineProfileData.profileDataY[j].Length &&
                        true == _vm.ChannelEnable[i])
                    {
                        // If the conversion is active, change the ProfileDataX to show values in µm instead
                        if (_vm.IsConversionActive)
                        {
                            for (int k = 0; k < _vm.LineProfileData.LengthPerChannel; k++)
                            {
                                _vm.LineProfileData.profileDataX[k] = k * _vm.LineProfileData.PixeltoµmConversionFactor;
                            }
                        }

                        EnumerableDataSource<double> xOneCh = new EnumerableDataSource<double>(_vm.LineProfileData.profileDataX);
                        xOneCh.SetXMapping(xVal => xVal);
                        EnumerableDataSource<double> yOneCh = new EnumerableDataSource<double>(_vm.LineProfileData.profileDataY[j]);
                        yOneCh.SetYMapping(yVal => yVal);
                        CompositeDataSource ds = new CompositeDataSource(xOneCh, yOneCh);

                        ((LineGraph)plotter.Children.ElementAt(startIndex + i)).DataSource = ds;

                        ((LineGraph)plotter.Children.ElementAt(startIndex + i)).LinePen = new Pen(new SolidColorBrush(_vm.ColorAssigment[i]), 1);
                        j++;
                    }
                }

                plotter.FitToView();
                if (false == _vm.IsAutoScaleActive)
                {
                    // Autoscale adds 20 to each limit for easier view of the plot, should do the same
                    plotter.Visible = new Rect(-20, _vm.YminValue, (double)_vm.LineProfileData.profileDataX.Max()+40, _vm.YmaxValue-_vm.YminValue);

                }

                //if (true == _vm.IsConversionActive)
                //{
                //    // Here the conversion pushed x axis forward by 20 so it started from -20 and we made it to fit between between it's first and last value by using below command so it starts from 0 now
                //    plotter.Visible = new Rect(_vm.LineProfileData.profileDataX[0], _vm.YminValue, (double)_vm.LineProfileData.profileDataX[_vm.LineProfileData.profileDataX.Length-1], _vm.YmaxValue - _vm.YminValue);
                //}

            }
        }

        public void SetData(LineProfileData lineprofileData)
        {
            _vm.SetData(lineprofileData);

            RedrawLinePlot();
        }

        void LineProfile_Closed(object sender, EventArgs e)
        {
            try
            {
                Application.Current.MainWindow.Activate();
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void SaveAs_Click(object sender, RoutedEventArgs e)
        {
            _vm.SaveAs();
        }

        private void _vm_LineWidthChange(int obj)
        {
            LineWidthChange(obj);
        }

        #endregion Methods
    }
}