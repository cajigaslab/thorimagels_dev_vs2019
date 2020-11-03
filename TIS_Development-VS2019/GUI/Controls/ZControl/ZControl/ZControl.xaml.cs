namespace ZControl
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Globalization;
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

    using ThorLogging;

    using ThorSharedTypes;

    public class CommandParameterConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            int val = Convert.ToInt32(value);

            if ("top" == (parameter.ToString()))
            {
                if (1 == val)
                {
                    return "Z_POS_MINUS";
                }
                else
                {
                    return "Z_POS_PLUS";
                }
            }
            else if ("bottom" == (parameter.ToString()))
            {
                if (1 == val)
                {
                    return "Z_POS_PLUS";
                }
                else
                {
                    return "Z_POS_MINUS";
                }
            }
            return string.Empty;
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return 0;
        }

        #endregion Methods
    }

    public class CommandParameterConverter2 : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            int val = Convert.ToInt32(value);

            if ("top" == (parameter.ToString()))
            {
                if (1 == val)
                {
                    return "Z2_POS_MINUS";
                }
                else
                {
                    return "Z2_POS_PLUS";
                }
            }
            else if ("bottom" == (parameter.ToString()))
            {
                if (1 == val)
                {
                    return "Z2_POS_PLUS";
                }
                else
                {
                    return "Z2_POS_MINUS";
                }
            }
            return string.Empty;
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return 0;
        }

        #endregion Methods
    }

    public class FontFamilyConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (targetType != typeof(FontFamily))
                throw new InvalidOperationException("The target must be a FontFamily");

            int val = Convert.ToInt32(value);

            int param = Convert.ToInt32(parameter);

            if (val == param)
            {
                return new FontFamily("Marlett");
            }
            else
            {
                return new FontFamily("Trebuchet MS");
            }
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return 0;
        }

        #endregion Methods
    }

    public class UpDownButtonContentConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            int val = Convert.ToInt32(value);

            if ("top" == (parameter.ToString()))
            {
                if (1 == val)
                {
                    return "5";
                }
                else
                {
                    return "+";
                }
            }
            else if ("bottom" == (parameter.ToString()))
            {
                if (1 == val)
                {
                    return "6";
                }
                else
                {
                    return "-";
                }
            }
            return string.Empty;
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return 0;
        }

        #endregion Methods
    }

    /// <summary>
    /// Interaction logic for ZControlUC.xaml
    /// </summary>
    public partial class ZControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty PreviewButtonEnabledProperty = 
            DependencyProperty.Register(
            "PreviewButtonEnabled",
            typeof(bool),
            typeof(ZControlUC));
        public static readonly DependencyProperty PreviewZStackCommandProperty = 
            DependencyProperty.Register(
            "PreviewZStackCommand",
            typeof(ICommand),
            typeof(ZControlUC));
        public static readonly DependencyProperty RPositionProperty = 
            DependencyProperty.Register(
            "RPosition",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty RStageVisibilityProperty = 
            DependencyProperty.RegisterAttached(
            "RStageVisibility",
            typeof(Visibility),
            typeof(ZControlUC),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onRStageVisibilityChanged)));
        public static readonly DependencyProperty SecondaryZScaleProperty = 
           DependencyProperty.Register(
           "SecondaryZScale",
           typeof(double),
           typeof(ZControlUC));
        public static readonly DependencyProperty Z2InvertLimitsProperty = 
            DependencyProperty.Register(
            "Z2InvertLimits",
            typeof(bool),
            typeof(ZControlUC));
        public static readonly DependencyProperty Z2MaxProperty = 
            DependencyProperty.Register(
            "Z2Max",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty Z2MinProperty = 
            DependencyProperty.Register(
            "Z2Min",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty Z2PositionBarProperty = 
            DependencyProperty.Register(
            "Z2PositionBar",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty Z2PositionProperty = 
            DependencyProperty.Register(
            "Z2Position",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty Z2PosOutOfBoundsProperty = 
            DependencyProperty.Register(
            "Z2PosOutOfBounds",
            typeof(bool),
            typeof(ZControlUC));
        public static readonly DependencyProperty Z2StageVisibilityProperty = 
            DependencyProperty.RegisterAttached(
            "Z2StageVisibility",
            typeof(Visibility),
            typeof(ZControlUC),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onZ2StageVisibilityChanged)));
        public static readonly DependencyProperty Z2StepSizeProperty = 
            DependencyProperty.Register(
            "Z2StepSize",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty Z2ZeroVisibilityProperty = 
            DependencyProperty.RegisterAttached(
            "Z2ZeroVisibility",
            typeof(Visibility),
            typeof(ZControlUC),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onZ2ZeroVisibilityChanged)));
        public static readonly DependencyProperty ZCenterProperty = 
            DependencyProperty.Register(
            "ZCenter",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZCommandProperty = 
            DependencyProperty.Register(
            "ZCommand",
            typeof(ICommand),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZGotoValueProperty = 
            DependencyProperty.Register(
            "ZGotoValue",
            typeof(double[]),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZInvert2Property = 
            DependencyProperty.Register(
            "ZInvert2",
            typeof(bool),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZInvertDevice2Property = 
            DependencyProperty.Register(
            "ZInvertDevice2",
            typeof(bool),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZInvertDeviceProperty = 
            DependencyProperty.Register(
            "ZInvertDevice",
            typeof(bool),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZInvertLimitsProperty = 
            DependencyProperty.Register(
            "ZInvertLimits",
            typeof(bool),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZInvertProperty = 
            DependencyProperty.Register(
            "ZInvert",
            typeof(bool),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZInvertUpDown2Property = 
            DependencyProperty.Register(
            "ZInvertUpDown2",
            typeof(int),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZInvertUpDownProperty = 
            DependencyProperty.Register(
            "ZInvertUpDown",
            typeof(int),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZInvertVisibilityProperty = 
            DependencyProperty.RegisterAttached(
            "ZInvertVisibility",
            typeof(Visibility),
            typeof(ZControlUC),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onZInvertVisibilityChanged)));
        public static readonly DependencyProperty ZMaxProperty = 
            DependencyProperty.Register(
            "ZMax",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZMinProperty = 
            DependencyProperty.Register(
            "ZMin",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZPositionBarProperty = 
            DependencyProperty.Register(
            "ZPositionBar",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZPositionProperty = 
            DependencyProperty.Register(
            "ZPosition",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZPosOutOfBoundsProperty = 
            DependencyProperty.Register(
            "ZPosOutOfBounds",
            typeof(bool),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZScanNumStepsProperty = 
            DependencyProperty.Register(
            "ZScanNumSteps",
            typeof(int),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZScanStartProperty = 
            DependencyProperty.Register(
            "ZScanStart",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZScanStepProperty = 
            DependencyProperty.Register(
            "ZScanStep",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZScanStopNotValidProperty = 
            DependencyProperty.Register(
            "ZScanStopNotValid",
            typeof(bool),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZScanStopProperty = 
            DependencyProperty.Register(
            "ZScanStop",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZScanThicknessProperty = 
            DependencyProperty.Register(
            "ZScanThickness",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZSectionThicknessProperty = 
            DependencyProperty.Register(
            "ZSectionThickness",
            typeof(double),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZStage2GoToLocationCommandProperty = 
            DependencyProperty.Register(
            "ZStage2GoToLocationCommand",
            typeof(ICommand),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZStage2LocationNamesProperty = 
            DependencyProperty.Register(
            "ZStage2LocationNames",
            typeof(ObservableCollection<string>),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZStage2LocationSaveCommandProperty = 
            DependencyProperty.Register(
            "ZStage2LocationSaveCommand",
            typeof(ICommand),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZStage2NameProperty = 
            DependencyProperty.Register(
            "ZStage2Name",
            typeof(string),
            typeof(ZControlUC));
        public static readonly DependencyProperty ZStepSizeProperty = 
            DependencyProperty.Register(
            "ZStepSize",
            typeof(double),
            typeof(ZControlUC));

        #endregion Fields

        #region Constructors

        public ZControlUC()
        {
            InitializeComponent();
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                Z2Border.BorderBrush = Brushes.Transparent;
                BorderTitle.Content = "Z and R Control:";
                SecondaryZExpander.IsExpanded = true;
            }
        }

        #endregion Constructors

        #region Properties

        public bool PreviewButtonEnabled
        {
            get { return (bool)GetValue(PreviewButtonEnabledProperty); }
            set { SetValue(PreviewButtonEnabledProperty, value); }
        }

        public ICommand PreviewZStackCommand
        {
            get { return (ICommand)GetValue(PreviewZStackCommandProperty); }
            set { SetValue(PreviewZStackCommandProperty, value); }
        }

        public double RPosition
        {
            get { return (double)GetValue(RPositionProperty); }
            set { SetValue(RPositionProperty, value); }
        }

        public Visibility RStageVisibility
        {
            get { return (Visibility)GetValue(RStageVisibilityProperty); }
            set { SetValue(RStageVisibilityProperty, value); }
        }

        public double SecondaryZScale
        {
            get { return (double)GetValue(SecondaryZScaleProperty); }
            set { SetValue(SecondaryZScaleProperty, value); }
        }

        public bool Z2InvertLimits
        {
            get { return (bool)GetValue(Z2InvertLimitsProperty); }
            set { SetValue(Z2InvertLimitsProperty, value); }
        }

        public double Z2Max
        {
            get { return (double)GetValue(Z2MaxProperty); }
            set { SetValue(Z2MaxProperty, value); }
        }

        public double Z2Min
        {
            get { return (double)GetValue(Z2MinProperty); }
            set { SetValue(Z2MinProperty, value); }
        }

        public double Z2Position
        {
            get { return (double)GetValue(Z2PositionProperty); }
            set { SetValue(Z2PositionProperty, value); }
        }

        public double Z2PositionBar
        {
            get { return (double)GetValue(Z2PositionBarProperty); }
            set { SetValue(Z2PositionBarProperty, value); }
        }

        public bool Z2PosOutOfBounds
        {
            get { return (bool)GetValue(Z2PosOutOfBoundsProperty); }
            set { SetValue(Z2PosOutOfBoundsProperty, value); }
        }

        public Visibility Z2StageVisibility
        {
            get { return (Visibility)GetValue(Z2StageVisibilityProperty); }
            set { SetValue(Z2StageVisibilityProperty, value); }
        }

        public double Z2StepSize
        {
            get { return (double)GetValue(Z2StepSizeProperty); }
            set { SetValue(Z2StepSizeProperty, value); }
        }

        public Visibility Z2ZeroVisibility
        {
            get { return (Visibility)GetValue(Z2ZeroVisibilityProperty); }
            set { SetValue(Z2ZeroVisibilityProperty, value); }
        }

        public double ZCenter
        {
            get { return (double)GetValue(ZCenterProperty); }
            set { SetValue(ZCenterProperty, value); }
        }

        public ICommand ZCommand
        {
            get { return (ICommand)GetValue(ZCommandProperty); }
            set { SetValue(ZCommandProperty, value); }
        }

        public double[] ZGotoValue
        {
            get { return (double[])GetValue(ZGotoValueProperty); }
            set { SetValue(ZGotoValueProperty, value); }
        }

        public bool ZInvert
        {
            get { return (bool)GetValue(ZInvertProperty); }
            set { SetValue(ZInvertProperty, value); }
        }

        public bool ZInvert2
        {
            get { return (bool)GetValue(ZInvert2Property); }
            set { SetValue(ZInvert2Property, value); }
        }

        public bool ZInvertDevice
        {
            get { return (bool)GetValue(ZInvertDeviceProperty); }
            set { SetValue(ZInvertDeviceProperty, value); }
        }

        public bool ZInvertDevice2
        {
            get { return (bool)GetValue(ZInvertDevice2Property); }
            set { SetValue(ZInvertDevice2Property, value); }
        }

        public bool ZInvertLimits
        {
            get { return (bool)GetValue(ZInvertLimitsProperty); }
            set { SetValue(ZInvertLimitsProperty, value); }
        }

        public int ZInvertUpDown
        {
            get { return (int)GetValue(ZInvertUpDownProperty); }
            set { SetValue(ZInvertUpDownProperty, value); }
        }

        public int ZInvertUpDown2
        {
            get { return (int)GetValue(ZInvertUpDown2Property); }
            set { SetValue(ZInvertUpDown2Property, value); }
        }

        public Visibility ZInvertVisibility
        {
            get { return (Visibility)GetValue(ZInvertVisibilityProperty); }
            set { SetValue(ZInvertVisibilityProperty, value); }
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

        public double ZPosition
        {
            get { return (double)GetValue(ZPositionProperty); }
            set { SetValue(ZPositionProperty, value); }
        }

        public double ZPositionBar
        {
            get { return (double)GetValue(ZPositionBarProperty); }
            set { SetValue(ZPositionBarProperty, value); }
        }

        public bool ZPosOutOfBounds
        {
            get { return (bool)GetValue(ZPosOutOfBoundsProperty); }
            set { SetValue(ZPosOutOfBoundsProperty, value); }
        }

        public int ZScanNumSteps
        {
            get { return (int)GetValue(ZScanNumStepsProperty); }
            set { SetValue(ZScanNumStepsProperty, value); }
        }

        public double ZScanStart
        {
            get { return (double)GetValue(ZScanStartProperty); }
            set { SetValue(ZScanStartProperty, value); }
        }

        public double ZScanStep
        {
            get { return (double)GetValue(ZScanStepProperty); }
            set { SetValue(ZScanStepProperty, value); }
        }

        public double ZScanStop
        {
            get { return (double)GetValue(ZScanStopProperty); }
            set { SetValue(ZScanStopProperty, value); }
        }

        public bool ZScanStopNotValid
        {
            get { return (bool)GetValue(ZScanStopNotValidProperty); }
            set { SetValue(ZScanStopNotValidProperty, value); }
        }

        public double ZScanThickness
        {
            get { return (double)GetValue(ZScanThicknessProperty); }
            set { SetValue(ZScanThicknessProperty, value); }
        }

        public double ZSectionThickness
        {
            get { return (double)GetValue(ZSectionThicknessProperty); }
            set { SetValue(ZSectionThicknessProperty, value); }
        }

        public ICommand ZStage2GoToLocationCommand
        {
            get { return (ICommand)GetValue(ZStage2GoToLocationCommandProperty); }
            set { SetValue(ZStage2GoToLocationCommandProperty, value); }
        }

        public ObservableCollection<string> ZStage2LocationNames
        {
            get { return (ObservableCollection<string>)GetValue(ZStage2LocationNamesProperty); }
            set { SetValue(ZStage2LocationNamesProperty, value); }
        }

        public ICommand ZStage2LocationSaveCommand
        {
            get { return (ICommand)GetValue(ZStage2LocationSaveCommandProperty); }
            set { SetValue(ZStage2LocationSaveCommandProperty, value); }
        }

        public string ZStage2Name
        {
            get { return (string)GetValue(ZStage2NameProperty); }
            set { SetValue(ZStage2NameProperty, value); }
        }

        public double ZStepSize
        {
            get { return (double)GetValue(ZStepSizeProperty); }
            set { SetValue(ZStepSizeProperty, value); }
        }

        #endregion Properties

        #region Methods

        public static void onRStageVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onZ2StageVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onZ2ZeroVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onZInvertVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        #endregion Methods
    }
}