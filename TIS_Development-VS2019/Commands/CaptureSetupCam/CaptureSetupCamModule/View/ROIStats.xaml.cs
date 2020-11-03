namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.ServiceModel;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Xml;

    using ThorLogging;

    /// <summary>
    /// </summary>
    public partial class ROIStats : Window, INotifyPropertyChanged
    {
        #region Fields

        public static readonly DependencyProperty MaxProperty = 
            DependencyProperty.Register("Max", typeof(int), typeof(ROIStats));
        public static readonly DependencyProperty MaxPropertyB = 
            DependencyProperty.Register("MaxB", typeof(int), typeof(ROIStats));
        public static readonly DependencyProperty MaxPropertyG = 
            DependencyProperty.Register("MaxG", typeof(int), typeof(ROIStats));
        public static readonly DependencyProperty MaxPropertyR = 
            DependencyProperty.Register("MaxR", typeof(int), typeof(ROIStats));
        public static readonly DependencyProperty MeanProperty = 
            DependencyProperty.Register("Mean", typeof(double), typeof(ROIStats));
        public static readonly DependencyProperty MeanPropertyB = 
            DependencyProperty.Register("MeanB", typeof(double), typeof(ROIStats));
        public static readonly DependencyProperty MeanPropertyG = 
            DependencyProperty.Register("MeanG", typeof(double), typeof(ROIStats));
        public static readonly DependencyProperty MeanPropertyR = 
            DependencyProperty.Register("MeanR", typeof(double), typeof(ROIStats));
        public static readonly DependencyProperty MinProperty = 
            DependencyProperty.Register("Min", typeof(int), typeof(ROIStats));
        public static readonly DependencyProperty MinPropertyB = 
            DependencyProperty.Register("MinB", typeof(int), typeof(ROIStats));
        public static readonly DependencyProperty MinPropertyG = 
            DependencyProperty.Register("MinG", typeof(int), typeof(ROIStats));
        public static readonly DependencyProperty MinPropertyR = 
            DependencyProperty.Register("MinR", typeof(int), typeof(ROIStats));
        public static readonly DependencyProperty StdDevProperty = 
            DependencyProperty.Register("StdDev", typeof(double), typeof(ROIStats));
        public static readonly DependencyProperty StdDevPropertyB = 
            DependencyProperty.Register("StdDevB", typeof(double), typeof(ROIStats));
        public static readonly DependencyProperty StdDevPropertyG = 
            DependencyProperty.Register("StdDevG", typeof(double), typeof(ROIStats));
        public static readonly DependencyProperty StdDevPropertyR = 
            DependencyProperty.Register("StdDevR", typeof(double), typeof(ROIStats));

        public static DependencyProperty channelsProperty = 
                                      DependencyProperty.RegisterAttached("Channels",
                                      typeof(int),
                                      typeof(ROIStats));
        public static DependencyProperty roiHeightProperty = 
                                      DependencyProperty.RegisterAttached("ROIHeight",
                                      typeof(int),
                                      typeof(ROIStats));
        public static DependencyProperty roiLeftProperty = 
                                      DependencyProperty.RegisterAttached("ROILeft",
                                      typeof(int),
                                      typeof(ROIStats));
        public static DependencyProperty roiTopProperty = 
                                      DependencyProperty.RegisterAttached("ROITop",
                                      typeof(int),
                                      typeof(ROIStats));
        public static DependencyProperty roiWidthProperty = 
                                      DependencyProperty.RegisterAttached("ROIWidth",
                                      typeof(int),
                                      typeof(ROIStats));

        #endregion Fields

        #region Constructors

        public ROIStats()
        {
            InitializeComponent();
            this.DataContext = this;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public int Channels
        {
            get { return (int)GetValue(channelsProperty); }
            set
            {
                SetValue(channelsProperty, value);

                if (value == 1)
                {
                    grayPanel.Visibility = Visibility.Visible;
                    redPanel.Visibility = Visibility.Collapsed;
                    greenPanel.Visibility = Visibility.Collapsed;
                    bluePanel.Visibility = Visibility.Collapsed;
                }
                else
                {
                    grayPanel.Visibility = Visibility.Collapsed;
                    redPanel.Visibility = Visibility.Visible;
                    greenPanel.Visibility = Visibility.Visible;
                    bluePanel.Visibility = Visibility.Visible;
                }
            }
        }

        public int Max
        {
            get { return (int)GetValue(MaxProperty); }
            set { SetValue(MaxProperty, value); }
        }

        public int MaxB
        {
            get { return (int)GetValue(MaxPropertyB); }
            set { SetValue(MaxPropertyB, value); }
        }

        public int MaxG
        {
            get { return (int)GetValue(MaxPropertyG); }
            set { SetValue(MaxPropertyG, value); }
        }

        public int MaxR
        {
            get { return (int)GetValue(MaxPropertyR); }
            set { SetValue(MaxPropertyR, value); }
        }

        public double Mean
        {
            get { return (double)GetValue(MeanProperty); }
            set { SetValue(MeanProperty, value); }
        }

        public double MeanB
        {
            get { return (double)GetValue(MeanPropertyB); }
            set { SetValue(MeanPropertyB, value); }
        }

        public double MeanG
        {
            get { return (double)GetValue(MeanPropertyG); }
            set { SetValue(MeanPropertyG, value); }
        }

        public double MeanR
        {
            get { return (double)GetValue(MeanPropertyR); }
            set { SetValue(MeanPropertyR, value); }
        }

        public int Min
        {
            get { return (int)GetValue(MinProperty); }
            set { SetValue(MinProperty, value); }
        }

        public int MinB
        {
            get { return (int)GetValue(MinPropertyB); }
            set { SetValue(MinPropertyB, value); }
        }

        public int MinG
        {
            get { return (int)GetValue(MinPropertyG); }
            set { SetValue(MinPropertyG, value); }
        }

        public int MinR
        {
            get { return (int)GetValue(MinPropertyR); }
            set { SetValue(MinPropertyR, value); }
        }

        public int ROIHeight
        {
            get
            {
                { return (int)GetValue(roiHeightProperty); }
            }
            set
            {
                { SetValue(roiHeightProperty, value); }
            }
        }

        public int ROILeft
        {
            get
            {
                { return (int)GetValue(roiLeftProperty); }
            }
            set
            {
                { SetValue(roiLeftProperty, value); }
            }
        }

        public int ROITop
        {
            get
            {
                { return (int)GetValue(roiTopProperty); }
            }
            set
            {
                { SetValue(roiTopProperty, value); }
            }
        }

        public int ROIWidth
        {
            get
            {
                { return (int)GetValue(roiWidthProperty); }
            }
            set
            {
                { SetValue(roiWidthProperty, value); }
            }
        }

        public double StdDev
        {
            get { return (double)GetValue(StdDevProperty); }
            set { SetValue(StdDevProperty, value); }
        }

        public double StdDevB
        {
            get { return (double)GetValue(StdDevPropertyB); }
            set { SetValue(StdDevPropertyB, value); }
        }

        public double StdDevG
        {
            get { return (double)GetValue(StdDevPropertyG); }
            set { SetValue(StdDevPropertyG, value); }
        }

        public double StdDevR
        {
            get { return (double)GetValue(StdDevPropertyR); }
            set { SetValue(StdDevPropertyR, value); }
        }

        #endregion Properties

        #region Methods

        protected virtual void OnPropertyChanged(String propertyName)
        {
            if (System.String.IsNullOrEmpty(propertyName))
            {
                return;
            }
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private void Button_OnOK(object sender, RoutedEventArgs e)
        {
            Close();
        }

        #endregion Methods
    }
}