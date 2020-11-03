namespace RealTimeLineChart.View
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
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

    using RealTimeLineChart.ViewModel;

    /// <summary>
    /// Interaction logic for StatsView.xaml
    /// </summary>
    public partial class StatsView : UserControl, INotifyPropertyChanged
    {
        #region Constructors

        public StatsView()
        {
            InitializeComponent();
            this.Loaded += StatsView_Loaded;
            this.Unloaded += StatsView_Unloaded;
            this.KeyDown += StatsView_KeyDown;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Methods

        private void cbDisplay_MouseLeave(object sender, MouseEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.IsResolutionEditing = false;
        }

        private void cMode_MouseLeave(object sender, MouseEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.IsModeEditing = false;
        }

        private void cRate_MouseLeave(object sender, MouseEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.IsRateEditing = false;
        }

        private void DisplayResolution_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.IsResolutionEditing = !vm.IsResolutionEditing;
        }

        private void MarkersListView_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if (vm._verticalmarker.Count != 0)
            {
                int markerSelected = MarkersListView.SelectedIndex;
                vm.SelectVerticalMarker(markerSelected);
            }
        }

        private void RaisePropertyChanged(string propName)
        {
            PropertyChangedEventHandler eh = PropertyChanged;
            if (eh != null)
            {
                eh(this, new PropertyChangedEventArgs(propName));
            }
        }

        private void SamplingMode_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.IsModeEditing = !vm.IsModeEditing;
        }

        private void SamplingRate_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.IsRateEditing = !vm.IsRateEditing;
        }

        void StatsView_KeyDown(object sender, KeyEventArgs e)
        {
            //filter for the enter or return key
            if ((e.Key == Key.Enter) || (e.Key == Key.Return))
            {
                e.Handled = true;
                TraversalRequest trNext = new TraversalRequest(FocusNavigationDirection.Next);

                UIElement keyboardFocus = (UIElement)Keyboard.FocusedElement;

                //move the focus to the next element
                if (keyboardFocus != null)
                {
                    if (keyboardFocus.GetType() == typeof(TextBox))
                    {
                        keyboardFocus.MoveFocus(trNext);
                    }
                }
            }
        }

        void StatsView_Loaded(object sender, RoutedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            MarkersListView.ItemsSource = vm.VerticalMarkerList;

            vm.RealTimeProvider = (XmlDataProvider)FindResource("RealTimeSetting");   //must keep baseUri for binding to work
            vm.SettingPath = Constants.ThorRealTimeData.SETTINGS_FILE_NAME;

            //create OTM Settings if configured
            vm.OTMProvider = (XmlDataProvider)FindResource("OTMSetting");   //must keep baseUri for binding to work
            vm.OTMSettingPath = Constants.ThorRealTimeData.OTM_SETTINGS_FILE;
        }

        void StatsView_Unloaded(object sender, RoutedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
        }

        #endregion Methods
    }

    public class VerticalMarker : INotifyPropertyChanged
    {
        #region Fields

        string comments;
        string name;
        double xCoordination;

        #endregion Fields

        #region Constructors

        public VerticalMarker(string _name, double _xCoordination)
        {
            name = _name;
            xCoordination = _xCoordination;
        }

        public VerticalMarker(string _name, double _xCoordination, string _comments)
        {
            name = _name;
            xCoordination = _xCoordination;
            comments = _comments;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string Comments
        {
            get { return comments; }
            set { comments = value; }
        }

        public string Name
        {
            get { return name; }
            set { name = value; }
        }

        public double XCoordination
        {
            get { return xCoordination; }
            set { xCoordination = value; }
        }

        #endregion Properties

        #region Methods

        private void RaisePropertyChanged(string propName)
        {
            PropertyChangedEventHandler eh = PropertyChanged;
            if (eh != null)
            {
                eh(this, new PropertyChangedEventArgs(propName));
            }
        }

        #endregion Methods
    }
}