namespace AutoFocusControl
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Globalization;
    using System.IO;
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

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class AutoFocusControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty AbsoluteStartPositionProperty = 
             DependencyProperty.Register(
             "AbsoluteStartPosition",
             typeof(double),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty AbsoluteStopPositionProperty = 
             DependencyProperty.Register(
             "AbsoluteStopPosition",
             typeof(double),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty AutoFocusButtonEnabledProperty = 
             DependencyProperty.Register(
             "AutoFocusButtonEnabled",
             typeof(bool),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty AutoFocusTypeProperty = 
            DependencyProperty.Register(
            "AutoFocusType",
            typeof(int),
            typeof(AutoFocusControlUC));
        public static readonly DependencyProperty CurrentZPositionProperty = 
             DependencyProperty.Register(
             "CurrentZPosition",
             typeof(double),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty InvertZProperty = 
             DependencyProperty.Register(
             "InvertZ",
             typeof(bool),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty RepeatsProperty = 
             DependencyProperty.Register(
             "Repeats",
             typeof(int),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty RunAutoFocusCommandProperty = 
              DependencyProperty.Register(
              "RunAutoFocusCommand",
              typeof(ICommand),
              typeof(AutoFocusControlUC));
        public static readonly DependencyProperty StartPositionProperty = 
             DependencyProperty.Register(
             "StartPosition",
             typeof(double),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty StepSizeUMProperty = 
             DependencyProperty.Register(
             "StepSizeUM",
             typeof(double),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty StopPositionProperty = 
             DependencyProperty.Register(
             "StopPosition",
             typeof(double),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty ZMaxProperty = 
             DependencyProperty.Register(
             "ZMax",
             typeof(double),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty ZMinProperty = 
             DependencyProperty.Register(
             "ZMin",
             typeof(double),
             typeof(AutoFocusControlUC));

        #endregion Fields

        #region Constructors

        public AutoFocusControlUC()
        {
            InitializeComponent();
            this.Loaded += new RoutedEventHandler(AutoFocusControlView_Loaded);
        }

        #endregion Constructors

        #region Properties

        public double AbsoluteStartPosition
        {
            get { return (double)GetValue(AbsoluteStartPositionProperty); }
            set { SetValue(AbsoluteStartPositionProperty, value); }
        }

        public double AbsoluteStopPosition
        {
            get { return (double)GetValue(AbsoluteStopPositionProperty); }
            set { SetValue(AbsoluteStopPositionProperty, value); }
        }

        public bool AutoFocusButtonEnabled
        {
            get { return (bool)GetValue(AutoFocusButtonEnabledProperty); }
            set { SetValue(AutoFocusButtonEnabledProperty, value); }
        }

        public int AutoFocusType
        {
            get { return (int)GetValue(AutoFocusTypeProperty); }
            set { SetValue(AutoFocusTypeProperty, value); }
        }

        public double CurrentZPosition
        {
            get { return (double)GetValue(CurrentZPositionProperty); }
            set { SetValue(CurrentZPositionProperty, value); }
        }

        public bool InvertZ
        {
            get { return (bool)GetValue(InvertZProperty); }
            set { SetValue(InvertZProperty, value); }
        }

        public int Repeats
        {
            get { return (int)GetValue(RepeatsProperty); }
            set { SetValue(RepeatsProperty, value); }
        }

        public ICommand RunAutoFocusCommand
        {
            get { return (ICommand)GetValue(RunAutoFocusCommandProperty); }
            set { SetValue(RunAutoFocusCommandProperty, value); }
        }

        public double StartPosition
        {
            get { return (double)GetValue(StartPositionProperty); }
            set { SetValue(StartPositionProperty, value); }
        }

        public double StepSizeUM
        {
            get { return (int)GetValue(StepSizeUMProperty); }
            set { SetValue(StepSizeUMProperty, value); }
        }

        public double StopPosition
        {
            get { return (double)GetValue(StopPositionProperty); }
            set { SetValue(StopPositionProperty, value); }
        }

        public double ZMax
        {
            get { return (double)GetValue(ZMaxProperty); }
            set { SetValue(ZMaxProperty, value); }
        }

        public double ZMin
        {
            get { return (double)GetValue(ZMinProperty); }
            set { SetValue(ZMinProperty, value); }
        }

        #endregion Properties

        #region Methods

        void AutoFocusControlView_Loaded(object sender, RoutedEventArgs e)
        {
        }

        #endregion Methods
    }
}