namespace OTMControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
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

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for OTMControlUC.xaml
    /// </summary>
    public partial class OTMControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty CenterTrapCommandProperty = 
        DependencyProperty.Register(
        "CenterTrapCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty SaveCalibrationCommandProperty = 
        DependencyProperty.Register(
        "SaveCalibrationCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAAngleMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapAAngleMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAAnglePlusCommandProperty = 
        DependencyProperty.Register(
        "TrapAAnglePlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAAngleProperty = 
        DependencyProperty.Register(
        "TrapAAngle",
        typeof(double),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAMaxHeightMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapAMaxHeightMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAMaxHeightPlusCommandProperty = 
        DependencyProperty.Register(
        "TrapAMaxHeightPlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAMaxHeightProperty = 
        DependencyProperty.Register(
        "TrapAMaxHeight",
        typeof(double),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAMaxWidthMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapAMaxWidthMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAMaxWidthPlusCommandProperty = 
        DependencyProperty.Register(
        "TrapAMaxWidthPlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAMaxWidthProperty = 
        DependencyProperty.Register(
        "TrapAMaxWidth",
        typeof(double),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAOffsetMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapAOffsetMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAOffsetPlusCommandProperty = 
        DependencyProperty.Register(
        "TrapAOffsetPlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAOffsetProperty = 
        DependencyProperty.Register(
        "TrapAOffset",
        typeof(CustomCollection<HwVal<double>>),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAOffsetResetCommandProperty = 
        DependencyProperty.Register(
        "TrapAOffsetResetCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAScaleMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapAScaleMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAScalePlusCommandProperty = 
        DependencyProperty.Register(
        "TrapAScalePlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAScaleProperty = 
        DependencyProperty.Register(
        "TrapAScale",
        typeof(CustomCollection<PC<double>>),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAScaleResetCommandProperty = 
        DependencyProperty.Register(
        "TrapAScaleResetCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAStepUMMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapAStepUMMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAStepUMPlusCommandProperty = 
        DependencyProperty.Register(
        "TrapAStepUMPlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAStepUMProperty = 
        DependencyProperty.Register(
        "TrapAStepUM",
        typeof(CustomCollection<PC<double>>),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAumMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapAumMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAumPlusCommandProperty = 
        DependencyProperty.Register(
        "TrapAumPlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapAumProperty = 
        DependencyProperty.Register(
        "TrapAum",
        typeof(CustomCollection<HwVal<double>>),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBAngleMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapBAngleMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBAnglePlusCommandProperty = 
        DependencyProperty.Register(
        "TrapBAnglePlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBAngleProperty = 
        DependencyProperty.Register(
        "TrapBAngle",
        typeof(double),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBMaxHeightMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapBMaxHeightMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBMaxHeightPlusCommandProperty = 
        DependencyProperty.Register(
        "TrapBMaxHeightPlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBMaxHeightProperty = 
        DependencyProperty.Register(
        "TrapBMaxHeight",
        typeof(double),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBMaxWidthMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapBMaxWidthMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBMaxWidthPlusCommandProperty = 
        DependencyProperty.Register(
        "TrapBMaxWidthPlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBMaxWidthProperty = 
        DependencyProperty.Register(
        "TrapBMaxWidth",
        typeof(double),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBOffsetMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapBOffsetMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBOffsetPlusCommandProperty = 
        DependencyProperty.Register(
        "TrapBOffsetPlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBOffsetProperty = 
        DependencyProperty.Register(
        "TrapBOffset",
        typeof(CustomCollection<HwVal<double>>),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBOffsetResetCommandProperty = 
        DependencyProperty.Register(
        "TrapBOffsetResetCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBScaleMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapBScaleMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBScalePlusCommandProperty = 
        DependencyProperty.Register(
        "TrapBScalePlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBScaleProperty = 
        DependencyProperty.Register(
        "TrapBScale",
        typeof(CustomCollection<PC<double>>),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBScaleResetCommandProperty = 
        DependencyProperty.Register(
        "TrapBScaleResetCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBStepUMMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapBStepUMMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBStepUMPlusCommandProperty = 
        DependencyProperty.Register(
        "TrapBStepUMPlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBStepUMProperty = 
        DependencyProperty.Register(
        "TrapBStepUM",
        typeof(CustomCollection<PC<double>>),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBumMinusCommandProperty = 
        DependencyProperty.Register(
        "TrapBumMinusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBumPlusCommandProperty = 
        DependencyProperty.Register(
        "TrapBumPlusCommand",
        typeof(ICommand),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapBumProperty = 
        DependencyProperty.Register(
        "TrapBum",
        typeof(CustomCollection<HwVal<double>>),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapCalAlertProperty = 
        DependencyProperty.Register(
        "TrapCalAlert",
        typeof(int),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapEnableProperty = 
        DependencyProperty.Register(
        "TrapEnable",
        typeof(CustomCollection<PC<bool>>),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapLastCalibTimeProperty = 
        DependencyProperty.Register(
        "TrapLastCalibTime",
        typeof(DateTime),
        typeof(OTMControlUC));
        public static readonly DependencyProperty TrapModeProperty = 
        DependencyProperty.Register(
        "TrapMode",
        typeof(int),
        typeof(OTMControlUC));

        #endregion Fields

        #region Constructors

        public OTMControlUC()
        {
            InitializeComponent();
            LayoutRoot.DataContext = this;
        }

        #endregion Constructors

        #region Properties

        public ICommand CenterTrapCommand
        {
            get { return (ICommand)GetValue(CenterTrapCommandProperty); }
            set { SetValue(CenterTrapCommandProperty, value); }
        }

        public ICommand SaveCalibrationCommand
        {
            get { return (ICommand)GetValue(SaveCalibrationCommandProperty); }
            set { SetValue(SaveCalibrationCommandProperty, value); }
        }

        public double TrapAAngle
        {
            get { return (double)GetValue(TrapAAngleProperty); }
            set { SetValue(TrapAAngleProperty, value); }
        }

        public ICommand TrapAAngleMinusCommand
        {
            get { return (ICommand)GetValue(TrapAAngleMinusCommandProperty); }
            set { SetValue(TrapAAngleMinusCommandProperty, value); }
        }

        public ICommand TrapAAnglePlusCommand
        {
            get { return (ICommand)GetValue(TrapAAnglePlusCommandProperty); }
            set { SetValue(TrapAAnglePlusCommandProperty, value); }
        }

        public double TrapAMaxHeight
        {
            get { return (double)GetValue(TrapAMaxHeightProperty); }
            set { SetValue(TrapAMaxHeightProperty, value); }
        }

        public ICommand TrapAMaxHeightMinusCommand
        {
            get { return (ICommand)GetValue(TrapAMaxHeightMinusCommandProperty); }
            set { SetValue(TrapAMaxHeightMinusCommandProperty, value); }
        }

        public ICommand TrapAMaxHeightPlusCommand
        {
            get { return (ICommand)GetValue(TrapAMaxHeightPlusCommandProperty); }
            set { SetValue(TrapAMaxHeightPlusCommandProperty, value); }
        }

        public double TrapAMaxWidth
        {
            get { return (double)GetValue(TrapAMaxWidthProperty); }
            set { SetValue(TrapAMaxWidthProperty, value); }
        }

        public ICommand TrapAMaxWidthMinusCommand
        {
            get { return (ICommand)GetValue(TrapAMaxWidthMinusCommandProperty); }
            set { SetValue(TrapAMaxWidthMinusCommandProperty, value); }
        }

        public ICommand TrapAMaxWidthPlusCommand
        {
            get { return (ICommand)GetValue(TrapAMaxWidthPlusCommandProperty); }
            set { SetValue(TrapAMaxWidthPlusCommandProperty, value); }
        }

        public CustomCollection<HwVal<double>> TrapAOffset
        {
            get { return (CustomCollection<HwVal<double>>)GetValue(TrapAOffsetProperty); }
            set { SetValue(TrapAOffsetProperty, value); }
        }

        public ICommand TrapAOffsetMinusCommand
        {
            get { return (ICommand)GetValue(TrapAOffsetMinusCommandProperty); }
            set { SetValue(TrapAOffsetMinusCommandProperty, value); }
        }

        public ICommand TrapAOffsetPlusCommand
        {
            get { return (ICommand)GetValue(TrapAOffsetPlusCommandProperty); }
            set { SetValue(TrapAOffsetPlusCommandProperty, value); }
        }

        public ICommand TrapAOffsetResetCommand
        {
            get { return (ICommand)GetValue(TrapAOffsetResetCommandProperty); }
            set { SetValue(TrapAOffsetResetCommandProperty, value); }
        }

        public CustomCollection<PC<double>> TrapAScale
        {
            get { return (CustomCollection<PC<double>>)GetValue(TrapAScaleProperty); }
            set { SetValue(TrapAScaleProperty, value); }
        }

        public ICommand TrapAScaleMinusCommand
        {
            get { return (ICommand)GetValue(TrapAScaleMinusCommandProperty); }
            set { SetValue(TrapAScaleMinusCommandProperty, value); }
        }

        public ICommand TrapAScalePlusCommand
        {
            get { return (ICommand)GetValue(TrapAScalePlusCommandProperty); }
            set { SetValue(TrapAScalePlusCommandProperty, value); }
        }

        public ICommand TrapAScaleResetCommand
        {
            get { return (ICommand)GetValue(TrapAScaleResetCommandProperty); }
            set { SetValue(TrapAScaleResetCommandProperty, value); }
        }

        public CustomCollection<PC<double>> TrapAStepUM
        {
            get { return (CustomCollection<PC<double>>)GetValue(TrapAStepUMProperty); }
            set { SetValue(TrapAStepUMProperty, value); }
        }

        public ICommand TrapAStepUMMinusCommand
        {
            get { return (ICommand)GetValue(TrapAStepUMMinusCommandProperty); }
            set { SetValue(TrapAStepUMMinusCommandProperty, value); }
        }

        public ICommand TrapAStepUMPlusCommand
        {
            get { return (ICommand)GetValue(TrapAStepUMPlusCommandProperty); }
            set { SetValue(TrapAStepUMPlusCommandProperty, value); }
        }

        public CustomCollection<HwVal<double>> TrapAum
        {
            get { return (CustomCollection<HwVal<double>>)GetValue(TrapAumProperty); }
            set { SetValue(TrapAumProperty, value); }
        }

        public ICommand TrapAumMinusCommand
        {
            get { return (ICommand)GetValue(TrapAumMinusCommandProperty); }
            set { SetValue(TrapAumMinusCommandProperty, value); }
        }

        public ICommand TrapAumPlusCommand
        {
            get { return (ICommand)GetValue(TrapAumPlusCommandProperty); }
            set { SetValue(TrapAumPlusCommandProperty, value); }
        }

        public double TrapBAngle
        {
            get { return (double)GetValue(TrapBAngleProperty); }
            set { SetValue(TrapBAngleProperty, value); }
        }

        public ICommand TrapBAngleMinusCommand
        {
            get { return (ICommand)GetValue(TrapBAngleMinusCommandProperty); }
            set { SetValue(TrapBAngleMinusCommandProperty, value); }
        }

        public ICommand TrapBAnglePlusCommand
        {
            get { return (ICommand)GetValue(TrapBAnglePlusCommandProperty); }
            set { SetValue(TrapBAnglePlusCommandProperty, value); }
        }

        public double TrapBMaxHeight
        {
            get { return (double)GetValue(TrapBMaxHeightProperty); }
            set { SetValue(TrapBMaxHeightProperty, value); }
        }

        public ICommand TrapBMaxHeightMinusCommand
        {
            get { return (ICommand)GetValue(TrapBMaxHeightMinusCommandProperty); }
            set { SetValue(TrapBMaxHeightMinusCommandProperty, value); }
        }

        public ICommand TrapBMaxHeightPlusCommand
        {
            get { return (ICommand)GetValue(TrapBMaxHeightPlusCommandProperty); }
            set { SetValue(TrapBMaxHeightPlusCommandProperty, value); }
        }

        public double TrapBMaxWidth
        {
            get { return (double)GetValue(TrapBMaxWidthProperty); }
            set { SetValue(TrapBMaxWidthProperty, value); }
        }

        public ICommand TrapBMaxWidthMinusCommand
        {
            get { return (ICommand)GetValue(TrapBMaxWidthMinusCommandProperty); }
            set { SetValue(TrapBMaxWidthMinusCommandProperty, value); }
        }

        public ICommand TrapBMaxWidthPlusCommand
        {
            get { return (ICommand)GetValue(TrapBMaxWidthPlusCommandProperty); }
            set { SetValue(TrapBMaxWidthPlusCommandProperty, value); }
        }

        public CustomCollection<HwVal<double>> TrapBOffset
        {
            get { return (CustomCollection<HwVal<double>>)GetValue(TrapBOffsetProperty); }
            set { SetValue(TrapBOffsetProperty, value); }
        }

        public ICommand TrapBOffsetMinusCommand
        {
            get { return (ICommand)GetValue(TrapBOffsetMinusCommandProperty); }
            set { SetValue(TrapBOffsetMinusCommandProperty, value); }
        }

        public ICommand TrapBOffsetPlusCommand
        {
            get { return (ICommand)GetValue(TrapBOffsetPlusCommandProperty); }
            set { SetValue(TrapBOffsetPlusCommandProperty, value); }
        }

        public ICommand TrapBOffsetResetCommand
        {
            get { return (ICommand)GetValue(TrapBOffsetResetCommandProperty); }
            set { SetValue(TrapBOffsetResetCommandProperty, value); }
        }

        public CustomCollection<PC<double>> TrapBScale
        {
            get { return (CustomCollection<PC<double>>)GetValue(TrapBScaleProperty); }
            set { SetValue(TrapBScaleProperty, value); }
        }

        public ICommand TrapBScaleMinusCommand
        {
            get { return (ICommand)GetValue(TrapBScaleMinusCommandProperty); }
            set { SetValue(TrapBScaleMinusCommandProperty, value); }
        }

        public ICommand TrapBScalePlusCommand
        {
            get { return (ICommand)GetValue(TrapBScalePlusCommandProperty); }
            set { SetValue(TrapBScalePlusCommandProperty, value); }
        }

        public ICommand TrapBScaleResetCommand
        {
            get { return (ICommand)GetValue(TrapBScaleResetCommandProperty); }
            set { SetValue(TrapBScaleResetCommandProperty, value); }
        }

        public CustomCollection<PC<double>> TrapBStepUM
        {
            get { return (CustomCollection<PC<double>>)GetValue(TrapBStepUMProperty); }
            set { SetValue(TrapBStepUMProperty, value); }
        }

        public ICommand TrapBStepUMMinusCommand
        {
            get { return (ICommand)GetValue(TrapBStepUMMinusCommandProperty); }
            set { SetValue(TrapBStepUMMinusCommandProperty, value); }
        }

        public ICommand TrapBStepUMPlusCommand
        {
            get { return (ICommand)GetValue(TrapBStepUMPlusCommandProperty); }
            set { SetValue(TrapBStepUMPlusCommandProperty, value); }
        }

        public CustomCollection<HwVal<double>> TrapBum
        {
            get { return (CustomCollection<HwVal<double>>)GetValue(TrapBumProperty); }
            set { SetValue(TrapBumProperty, value); }
        }

        public ICommand TrapBumMinusCommand
        {
            get { return (ICommand)GetValue(TrapBumMinusCommandProperty); }
            set { SetValue(TrapBumMinusCommandProperty, value); }
        }

        public ICommand TrapBumPlusCommand
        {
            get { return (ICommand)GetValue(TrapBumPlusCommandProperty); }
            set { SetValue(TrapBumPlusCommandProperty, value); }
        }

        public int TrapCalAlert
        {
            get { return (int)GetValue(TrapCalAlertProperty); }
            set { SetValue(TrapCalAlertProperty, value); }
        }

        public CustomCollection<PC<bool>> TrapEnable
        {
            get { return (CustomCollection<PC<bool>>)GetValue(TrapEnableProperty); }
            set { SetValue(TrapEnableProperty, value); }
        }

        public DateTime TrapLastCalibTime
        {
            get { return (DateTime)GetValue(TrapLastCalibTimeProperty); }
            set { SetValue(TrapLastCalibTimeProperty, value); }
        }

        public int TrapMode
        {
            get { return (int)GetValue(TrapModeProperty); }
            set { SetValue(TrapModeProperty, value); }
        }

        #endregion Properties
    }
}