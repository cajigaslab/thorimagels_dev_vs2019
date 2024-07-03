namespace HistogramControl
{
    using System;
    using System.Collections.ObjectModel;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Shapes;

    using HistogramControl.ViewModel;

    using SciChart.Charting.Visuals;

    /// <summary>
    /// Interaction logic for HistogramControl.xaml
    /// </summary>
    public partial class HistogramControlView : UserControl
    {
        #region Fields

        public static readonly DependencyProperty HistogramChannelsProperty = DependencyProperty.Register("HistogramChannels", typeof(ObservableCollection<HistogramChannel>), typeof(HistogramControlView));
        public static readonly DependencyProperty IsChannelVisibilityVisibleProperty = DependencyProperty.Register("IsChannelVisibilityVisible", typeof(bool), typeof(HistogramControlView));

        Path _bendableLine;
        private bool _isDragging = false;
        private Point _lastMousePoint;

        #endregion Fields

        #region Constructors

        public HistogramControlView()
        {
            SciChartSurface.SetRuntimeLicenseKey("TCqHmTkInF/fDQuv2IRL2jISc44wjQP46+iIvQjEtY21jW+X66HmcupG3FzPOD39A8zSj8i8vKIUgW2r9wgDzzuy3RK/gQsogW5d2SN0QVo0tnTzAd/uEWHLFeS2W17/2hf//FVKxwU4704JENFsCxYbOoPZHbpNwbTJovnl1QjEabIjy1KzBkA2fJMJbWF8wPRTD0ruKUEnrHpXOuvpTOQlr7a6XSmUlJ5o/Vsx7oJRcIYm70L7shDDXu1hHEqICpBtcCb91kpgNMaAZoWJwhYiBmowdHbgszC9lm3o6hlLi35y88379sblqhR1b7rIh80hoc3XwfQUmPydvU6RAwLUyIYT/z28JOl3kx0pReVdlLQd5bfdldNeNrI6J3ajng427j2udkQpNqQxNUEbLH9D/qqr5xeez+F/O4FWIYiYJvs9pgMamA6GYfGnV1sQ2spekHboGxh5PWfNgAWTuqFU/arLx5W1LYhT75WcXUe8pSXX1JD6qGD7/G4l9KpN+CYuZrXh1Zl9ND5KLicMDvfX65W+B8ka0TZbLIFExmsWSwNt+n6osLwE48Q8JsPb1+WCzy+1oCaFnyGXcpK5LlVB0Dcg9VdcDnwmrEQ=");
            InitializeComponent();
            SizeChanged += new SizeChangedEventHandler(SizeChangedHandler);
        }

        #endregion Constructors

        #region Properties

        public ObservableCollection<HistogramChannel> HistogramChannels
        {
            get { return (ObservableCollection<HistogramChannel>)GetValue(HistogramChannelsProperty); }
            set { SetValue(HistogramChannelsProperty, value); }
        }

        public bool IsChannelVisibilityVisible
        {
            get { return (bool)GetValue(IsChannelVisibilityVisibleProperty); }
            set { SetValue(IsChannelVisibilityVisibleProperty, value); }
        }

        #endregion Properties

        #region Methods

        private void bendableLine_Loaded(object sender, RoutedEventArgs e)
        {
            _bendableLine = sender as Path;
            _bendableLine.MouseDown += BendableLine_MouseDown;
        }

        private void BendableLine_MouseDown(object sender, MouseButtonEventArgs e)
        {
            _isDragging = true;
            _lastMousePoint = e.GetPosition(HistogramSciChart);
            e.Handled = true;
        }

        private void bendableLine_Unloaded(object sender, RoutedEventArgs e)
        {
            _bendableLine.MouseDown -= BendableLine_MouseDown;
            _bendableLine = null;
        }

        private void HistogramSciChart_MouseLeave(object sender, MouseEventArgs e)
        {
            _isDragging = false;
        }

        private void HistogramSciChart_MouseMove(object sender, MouseEventArgs e)
        {
            if (_isDragging)
            {
                HistogramControlViewModel vm = (HistogramControlViewModel)DataContext;
                if (null == vm)
                {
                    return;
                }

                var currentMousePoint = e.GetPosition(HistogramSciChart);

                if (0 != vm.Gamma)
                {
                    vm.Gamma += (currentMousePoint.Y - _lastMousePoint.Y) / (25 / vm.Gamma);
                }

                e.Handled = true;

                _lastMousePoint = currentMousePoint;
            }
        }

        private void HistogramSciChart_MouseUp(object sender, MouseButtonEventArgs e)
        {
            _isDragging = false;
        }

        private void PART_BoxAnnotationRoot_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            HistogramControlViewModel vm = (HistogramControlViewModel)DataContext;
            if (null != vm)
            {
                vm.AnnotationBoxWidth = customAnnotation.ActualWidth;
            }
        }

        private void SizeChangedHandler(object sender, EventArgs e)
        {
            HistogramControlViewModel vm = (HistogramControlViewModel)DataContext;
            if (null != vm)
            {
                vm.RedrawHistogram();
            }
        }

        #endregion Methods
    }
}