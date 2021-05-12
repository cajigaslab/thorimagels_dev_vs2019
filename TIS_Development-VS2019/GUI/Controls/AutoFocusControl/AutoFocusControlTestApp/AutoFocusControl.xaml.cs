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
        public static readonly DependencyProperty AutoFocusTypeProperty =
            DependencyProperty.Register(
            "AutoFocusType",
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
        public static readonly DependencyProperty StopPositionProperty =
             DependencyProperty.Register(
             "StopPosition",
             typeof(double),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty ZMinProperty =
             DependencyProperty.Register(
             "ZMin",
             typeof(double),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty ZMaxProperty =
             DependencyProperty.Register(
             "ZMax",
             typeof(double),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty RepeatsProperty =
             DependencyProperty.Register(
             "Repeats",
             typeof(int),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty StepSizeUMProperty =
             DependencyProperty.Register(
             "StepSizeUM",
             typeof(double),
             typeof(AutoFocusControlUC));
        public static readonly DependencyProperty AutoFocusButtonEnabledProperty =
             DependencyProperty.Register(
             "AutoFocusButtonEnabled",
             typeof(bool),
             typeof(AutoFocusControlUC));
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

        public static readonly DependencyProperty ComboBoxItemsListProperty = 
              DependencyProperty.Register(
              "ComboBoxItemsList",
              typeof(ObservableCollection<string>),
              typeof(AutoFocusControlUC));
        public static readonly DependencyProperty LSMPinholeAlignmentMinusCommandProperty = 
              DependencyProperty.Register(
              "LSMPinholeAlignmentMinusCommand",
              typeof(ICommand),
              typeof(AutoFocusControlUC));
        public static readonly DependencyProperty LSMPinholeAlignmentPlusCommandProperty = 
              DependencyProperty.Register(
              "LSMPinholeAlignmentPlusCommand",
              typeof(ICommand),
              typeof(AutoFocusControlUC));
        public static readonly DependencyProperty LSMPinholeAlignmentSetCommandProperty = 
              DependencyProperty.Register(
              "LSMPinholeAlignmentSetCommand",
              typeof(ICommand),
              typeof(AutoFocusControlUC));
        public static readonly DependencyProperty PinholeADUsStringProperty = 
              DependencyProperty.Register(
              "PinholeADUsString",
              typeof(string),
              typeof(AutoFocusControlUC));
        public static readonly DependencyProperty PinholePositionProperty = 
              DependencyProperty.Register(
              "PinholePosition",
              typeof(int),
              typeof(AutoFocusControlUC));
        public static readonly DependencyProperty TxtPinholeAlignmentProperty = 
              DependencyProperty.Register(
              "TxtPinholeAlignment",
              typeof(string),
              typeof(AutoFocusControlUC));
        public static readonly DependencyProperty UpdatePinholePosTxtCommandProperty = 
              DependencyProperty.Register(
              "UpdatePinholePosTxtCommand",
              typeof(ICommand),
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

        public int AutoFocusType
        {
            get { return (int)GetValue(AutoFocusTypeProperty); }
            set { SetValue(AutoFocusTypeProperty, value); }
        }

        public ICommand RunAutoFocusCommand
        {
            get { return (ICommand)GetValue(RunAutoFocusCommandProperty); }
            set { SetValue(RunAutoFocusCommandProperty, value); }
        }

        public bool AutoFocusButtonEnabled
        {
            get { return (bool)GetValue(AutoFocusButtonEnabledProperty); }
            set { SetValue(AutoFocusButtonEnabledProperty, value); }
        }

        public double StartPosition
        {
            get { return (double)GetValue(StartPositionProperty); }
            set { SetValue(StartPositionProperty, value); }
        }

        public double StopPosition
        {
            get { return (double)GetValue(StopPositionProperty); }
            set { SetValue(StopPositionProperty, value); }
        }

        public double ZMin
        {
            get { return (double)GetValue(ZMinProperty); }
            set { SetValue(ZMinProperty, value); }
        }

        public double ZMax
        {
            get { return (double)GetValue(ZMaxProperty); }
            set { SetValue(ZMaxProperty, value); }
        }

        public int Repeats
        {
            get { return (int)GetValue(RepeatsProperty); }
            set { SetValue(RepeatsProperty, value); }
        }

        public double StepSizeUM
        {
            get { return (int)GetValue(StepSizeUMProperty); }
            set { SetValue(StepSizeUMProperty, value); }
        }

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


        public ObservableCollection<string> ComboBoxItemsList
        {
            get { return (ObservableCollection<string>)GetValue(ComboBoxItemsListProperty); }
            set { SetValue(ComboBoxItemsListProperty, value); }
        }

        public ICommand LSMPinholeAlignmentMinusCommand
        {
            get { return (ICommand)GetValue(LSMPinholeAlignmentMinusCommandProperty); }
                  set { SetValue(LSMPinholeAlignmentMinusCommandProperty, value); }
        }

        public ICommand LSMPinholeAlignmentPlusCommand
        {
            get { return (ICommand)GetValue(LSMPinholeAlignmentPlusCommandProperty); }
            set { SetValue(LSMPinholeAlignmentPlusCommandProperty, value); }
        }

        public ICommand LSMPinholeAlignmentSetCommand
        {
            get { return (ICommand)GetValue(LSMPinholeAlignmentSetCommandProperty); }
            set { SetValue(LSMPinholeAlignmentSetCommandProperty, value); }
        }

        public string PinholeADUsString
        {
            get { return (string)GetValue(PinholeADUsStringProperty); }
            set { SetValue(PinholeADUsStringProperty, value); }
        }

        public int PinholePosition
        {
            get { return (int)GetValue(PinholePositionProperty); }
            set { SetValue(PinholePositionProperty, value); }
        }

        public string TxtPinholeAlignment
        {
            get { return (string)GetValue(TxtPinholeAlignmentProperty); }
            set { SetValue(TxtPinholeAlignmentProperty, value); }
        }

        public ICommand UpdatePinholePosTxtCommand
        {
            get { return (ICommand)GetValue(UpdatePinholePosTxtCommandProperty); }
            set { SetValue(UpdatePinholePosTxtCommandProperty, value); }
        }

        #endregion Properties

        #region Methods
        void AutoFocusControlView_Loaded(object sender, RoutedEventArgs e)
        {
        }
        #endregion Methods
    }
}