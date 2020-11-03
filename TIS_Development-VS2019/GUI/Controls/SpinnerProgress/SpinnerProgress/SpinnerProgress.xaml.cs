namespace SpinnerProgress
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
    using System.Windows.Media.Animation;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    /// <summary>
    /// Interaction logic for SpinnerProgress.xaml
    /// </summary>
    public partial class SpinnerProgressControl : UserControl, INotifyPropertyChanged
    {
        #region Fields

        public static DependencyProperty SpinnerHeightProperty = 
          DependencyProperty.RegisterAttached("SpinnerHeight",
          typeof(int),
          typeof(SpinnerProgressControl),
          new FrameworkPropertyMetadata(new PropertyChangedCallback(onSpinnerHeightChanged)));
        public static DependencyProperty SpinnerWidthProperty = 
         DependencyProperty.RegisterAttached("SpinnerWidth",
         typeof(int),
         typeof(SpinnerProgressControl),
         new FrameworkPropertyMetadata(new PropertyChangedCallback(onSpinnerWidthChanged)));

        private static int _spinnerHeight = 100;
        private static int _spinnerWidth = 100;

        #endregion Fields

        #region Constructors

        public SpinnerProgressControl()
        {
            InitializeComponent();

            this.Loaded += new RoutedEventHandler(SpinnerProgressControl_Loaded);
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public int SpinnerHeight
        {
            get
            {
                return Convert.ToInt32(canvasSpin.Height);
            }
            set
            {
                canvasProgress.Width = value;
                canvasSpin.Width = value;
                OnPropertyChanged("SpinnerHeight");
            }
        }

        public int SpinnerWidth
        {
            get
            {
                return Convert.ToInt32(canvasSpin.Height);
            }
            set
            {
                canvasProgress.Height = value;
                canvasSpin.Height = value;
                OnPropertyChanged("SpinnerWidth");
            }
        }

        #endregion Properties

        #region Methods

        public static void onSpinnerHeightChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int post = Convert.ToInt32(e.NewValue);
            if (post >= 0)
            {
                int pre = _spinnerHeight;
                _spinnerHeight = post;
            }
        }

        public static void onSpinnerWidthChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int post = Convert.ToInt32(e.NewValue);
            if (post >= 0)
            {
                int pre = _spinnerWidth;
                _spinnerWidth = post;
            }
        }

        //Spinner draw
        private void drawCanvas()
        {
            canvasProgress.Visibility = Visibility.Visible;

            for (int i = 0; i < 12; i++)
            {

                Line line = new Line()
                {
                    X1 = SpinnerWidth/2,
                    X2 = SpinnerWidth/2,
                    Y1 = 0,
                    Y2 = SpinnerHeight/4,

                    StrokeThickness = 4,
                    Stroke = Brushes.Yellow,
                    Width = SpinnerWidth,
                    Height = SpinnerHeight
                };

                line.VerticalAlignment = VerticalAlignment.Center;

                line.HorizontalAlignment = HorizontalAlignment.Center;

                line.RenderTransformOrigin = new Point(.5, .5);

                line.RenderTransform = new RotateTransform(i * 30);

                line.Opacity = (double)i / 12;

                canvasSpin.Children.Add(line);
            }
        }

        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        void SpinnerProgressControl_Loaded(object sender, RoutedEventArgs e)
        {
            drawCanvas();

            DoubleAnimation a = new DoubleAnimation();

            a.From = 0;

            a.To = 360;

            a.RepeatBehavior = RepeatBehavior.Forever;

            a.SpeedRatio = 1;

            spin.BeginAnimation(RotateTransform.AngleProperty, a);
        }

        #endregion Methods
    }
}