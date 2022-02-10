namespace PowerControl
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
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

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class PowerControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty CalibrateCommandProperty = 
        DependencyProperty.Register(
        "CalibrateCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty PockelsCalibrateAgainEnableProperty = 
        DependencyProperty.Register(
        "PockelsCalibrateAgainEnable",
        typeof(bool),
        typeof(PowerControlUC));
        public static readonly DependencyProperty PowerMinusCommandProperty = 
        DependencyProperty.Register(
        "PowerMinusCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty PowerPlusCommandProperty = 
        DependencyProperty.Register(
        "PowerPlusCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty PowerRampAddCommandProperty = 
        DependencyProperty.Register(
        "PowerRampAddCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty PowerRampDeleteCommandProperty = 
        DependencyProperty.Register(
        "PowerRampDeleteCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty PowerRampEditCommandProperty = 
        DependencyProperty.Register(
        "PowerRampEditCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty PowerRampsCustomProperty = 
        DependencyProperty.Register("PowerRampsCustom",
        typeof(IList),
        typeof(PowerControlUC),
        new PropertyMetadata(new List<string>()));
        public static readonly DependencyProperty PowerRegCalCommandProperty = 
        DependencyProperty.Register(
        "PowerRegCalCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty PowerRegCalSaveCommandProperty = 
        DependencyProperty.Register(
        "PowerRegCalSaveCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty SelectPockelsMaskCommandProperty = 
        DependencyProperty.Register(
        "SelectPockelsMaskCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty SetPowerCommandProperty = 
        DependencyProperty.Register(
        "SetPowerCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty StepCoarseCommandProperty = 
        DependencyProperty.Register(
        "StepCoarseCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty StepFineCommandProperty = 
        DependencyProperty.Register(
        "StepFineCommand",
        typeof(ICommand),
        typeof(PowerControlUC));
        public static readonly DependencyProperty UpdatePockelsMaskToROIMaskCommandProperty = 
        DependencyProperty.Register(
        "UpdatePockelsMaskToROIMaskCommand",
        typeof(ICommand),
        typeof(PowerControlUC));

        public static DependencyProperty EnablePockelsMaskProperty = 
        DependencyProperty.RegisterAttached("EnablePockelsMask",
        typeof(bool),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onEnablePockelsMaskChanged)));
        public static DependencyProperty EncoderPositionVisibilityProperty = 
            DependencyProperty.Register(
            "EncoderPositionVisibility",
            typeof(Visibility),
            typeof(PowerControlUC));

        //        public static DependencyProperty EnableDevReadingProperty =
        //DependencyProperty.RegisterAttached("EnableDevReading",
        //typeof(bool),
        //typeof(PowerControlUC),
        //new FrameworkPropertyMetadata(new PropertyChangedCallback(onEnableDevReadingChanged)));
        public static DependencyProperty MaskAndBlankingVisibilityProperty = 
        DependencyProperty.RegisterAttached("MaskAndBlankingVisibility",
        typeof(Visibility),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onMaskAndBlankingVisibilityChanged)));
        public static DependencyProperty PCTabIndexProperty = 
        DependencyProperty.RegisterAttached("PCTabIndex",
        typeof(int),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPCTabIndexChanged)));
        public static DependencyProperty PockelsBlankingPhaseShiftPercentProperty = 
        DependencyProperty.RegisterAttached("PockelsBlankingPhaseShiftPercent",
        typeof(double),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPockelsBlankingPhaseShiftPercentChanged)));
        public static DependencyProperty PockelsBlankingPhaseShiftPercentVisibilityProperty = 
           DependencyProperty.RegisterAttached("PockelsBlankingPhaseShiftPercentVisibility",
           typeof(Visibility),
           typeof(PowerControlUC),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onPockelsBlankingPhaseShiftPercentVisibilityChanged)));
        public static DependencyProperty PockelsBlankPercentageProperty = 
        DependencyProperty.RegisterAttached("PockelsBlankPercentage",
        typeof(int),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPockelsBlankPercentageChanged)));
        public static DependencyProperty PockelsCalibrationVisibilityProperty = 
        DependencyProperty.RegisterAttached("PockelsCalibrationVisibility",
        typeof(Visibility),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPockelsCalibrationVisibilityChanged)));
        public static DependencyProperty PockelsMaskFileProperty = 
        DependencyProperty.RegisterAttached("PockelsMaskFile",
        typeof(string),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPockelsMaskFileChanged)));
        public static DependencyProperty PockelsMaskInvertProperty = 
        DependencyProperty.RegisterAttached("PockelsMaskInvert",
        typeof(bool),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPockelsMaskInvertChanged)));
        public static DependencyProperty PockelsMaskOptionsAvailableProperty = 
        DependencyProperty.RegisterAttached("PockelsMaskOptionsAvailable",
        typeof(bool),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPockelsMaskOptionsAvailableChanged)));
        public static DependencyProperty PowerGoProperty = 
        DependencyProperty.RegisterAttached("PowerGo",
        typeof(double),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerGoChanged)));
        public static DependencyProperty PowerMaxProperty = 
        DependencyProperty.RegisterAttached("PowerMax",
        typeof(double),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerMaxChanged)));
        public static DependencyProperty PowerMinProperty = 
        DependencyProperty.RegisterAttached("PowerMin",
        typeof(double),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerMinChanged)));
        public static DependencyProperty PowerModeProperty = 
        DependencyProperty.RegisterAttached("PowerMode",
        typeof(int),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerModeChanged)));
        public static DependencyProperty PowerProperty = 
        DependencyProperty.RegisterAttached("Power",
        typeof(double),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerChanged)));
        public static DependencyProperty PowerRampSelectedProperty = 
        DependencyProperty.RegisterAttached("PowerRampSelected",
        typeof(int),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerRampSelectedChanged)));
        public static DependencyProperty PowerRegCalibrationVisibilityProperty = 
        DependencyProperty.RegisterAttached("PowerRegCalibrationVisibility",
        typeof(Visibility),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerRegCalibrationVisibilityChanged)));
        public static DependencyProperty PowerRegCalName1Property = 
        DependencyProperty.RegisterAttached("PowerRegCalName1",
        typeof(string),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerRegCalName1Changed)));
        public static DependencyProperty PowerRegCalName2Property = 
        DependencyProperty.RegisterAttached("PowerRegCalName2",
        typeof(string),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerRegCalName2Changed)));
        public static DependencyProperty PowerRegCalName3Property = 
        DependencyProperty.RegisterAttached("PowerRegCalName3",
        typeof(string),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerRegCalName3Changed)));
        public static DependencyProperty PowerRegCalName4Property = 
        DependencyProperty.RegisterAttached("PowerRegCalName4",
        typeof(string),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerRegCalName4Changed)));
        public static DependencyProperty PowerRegCalName5Property = 
        DependencyProperty.RegisterAttached("PowerRegCalName5",
        typeof(string),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerRegCalName5Changed)));
        public static DependencyProperty PowerRegCalName6Property = 
        DependencyProperty.RegisterAttached("PowerRegCalName6",
        typeof(string),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerRegCalName6Changed)));
        public static DependencyProperty PowerRegEncoderPositionProperty = 
            DependencyProperty.Register(
            "PowerRegEncoderPosition",
            typeof(string),
            typeof(PowerControlUC));
        public static DependencyProperty PowerRegZeroProperty = 
        DependencyProperty.RegisterAttached("PowerRegZero",
        typeof(int),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerRegZeroChanged)));
        public static DependencyProperty PowerStepSizeProperty = 
        DependencyProperty.RegisterAttached("PowerStepSize",
        typeof(double),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerStepSizeChanged)));
        public static DependencyProperty PowerStepSizeVisibilityProperty = 
        DependencyProperty.RegisterAttached("PowerStepSizeVisibility",
        typeof(Visibility),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerStepSizeVisibilityChanged)));
        public static DependencyProperty PowerThresholdProperty = 
        DependencyProperty.RegisterAttached("PowerThreshold",
        typeof(double),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPowerThresholdChanged)));
        public static DependencyProperty VoltageMaxProperty = 
        DependencyProperty.RegisterAttached("VoltageMax",
        typeof(double),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onVoltageMaxChanged)));
        public static DependencyProperty VoltageMinProperty = 
        DependencyProperty.RegisterAttached("VoltageMin",
        typeof(double),
        typeof(PowerControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onVoltageMinChanged)));

        #endregion Fields

        #region Constructors

        public PowerControlUC()
        {
            InitializeComponent();
            LayoutRoot.DataContext = this;
            PowerRampsCustom = new List<string>();
        }

        #endregion Constructors

        #region Properties

        public ICommand CalibrateCommand
        {
            get { return (ICommand)GetValue(CalibrateCommandProperty); }
            set { SetValue(CalibrateCommandProperty, value); }
        }

        public bool EnablePockelsMask
        {
            get { return (bool)GetValue(EnablePockelsMaskProperty); }
            set { SetValue(EnablePockelsMaskProperty, value); }
        }

        public Visibility EncoderPositionVisibility
        {
            get { return (Visibility)GetValue(EncoderPositionVisibilityProperty); }
            set { SetValue(EncoderPositionVisibilityProperty, value); }
        }

        public Visibility MaskAndBlankingVisibility
        {
            get { return (Visibility)GetValue(MaskAndBlankingVisibilityProperty); }
            set { SetValue(MaskAndBlankingVisibilityProperty, value); }
        }

        public int PCTabIndex
        {
            get { return (int)GetValue(PCTabIndexProperty); }
            set { SetValue(PCTabIndexProperty, value); }
        }

        public double PockelsBlankingPhaseShiftPercent
        {
            get { return (double)GetValue(PockelsBlankingPhaseShiftPercentProperty); }
            set { SetValue(PockelsBlankingPhaseShiftPercentProperty, value); }
        }

        public Visibility PockelsBlankingPhaseShiftPercentVisibility
        {
            get { return (Visibility)GetValue(PockelsBlankingPhaseShiftPercentVisibilityProperty); }
            set { SetValue(PockelsBlankingPhaseShiftPercentVisibilityProperty, value); }
        }

        public double PockelsBlankPercentage
        {
            get { return (double)GetValue(PockelsBlankPercentageProperty); }
            set { SetValue(PockelsBlankPercentageProperty, value); }
        }

        public bool PockelsCalibrateAgainEnable
        {
            get { return (bool)GetValue(PockelsCalibrateAgainEnableProperty); }
            set { SetValue(PockelsCalibrateAgainEnableProperty, value); }
        }

        public Visibility PockelsCalibrationVisibility
        {
            get { return (Visibility)GetValue(PockelsCalibrationVisibilityProperty); }
            set { SetValue(PockelsCalibrationVisibilityProperty, value); }
        }

        public bool PockelsMaskFile
        {
            get { return (bool)GetValue(PockelsMaskFileProperty); }
            set { SetValue(PockelsMaskFileProperty, value); }
        }

        public bool PockelsMaskInvert
        {
            get { return (bool)GetValue(PockelsMaskInvertProperty); }
            set { SetValue(PockelsMaskInvertProperty, value); }
        }

        public bool PockelsMaskOptionsAvailable
        {
            get { return (bool)GetValue(PockelsMaskOptionsAvailableProperty); }
            set { SetValue(PockelsMaskOptionsAvailableProperty, value); }
        }

        public double Power
        {
            get { return (double)GetValue(PowerProperty); }
            set { SetValue(PowerProperty, value); }
        }

        public double PowerGo
        {
            get
            {
                return (double)GetValue(PowerGoProperty);
            }
            set
            {
                SetValue(PowerGoProperty, value);
            }
        }

        public double PowerMax
        {
            get { return (double)GetValue(PowerMaxProperty); }
            set { SetValue(PowerMaxProperty, value); }
        }

        public double PowerMin
        {
            get { return (double)GetValue(PowerMinProperty); }
            set { SetValue(PowerMinProperty, value); }
        }

        public ICommand PowerMinusCommand
        {
            get { return (ICommand)GetValue(PowerMinusCommandProperty); }
            set { SetValue(PowerMinusCommandProperty, value); }
        }

        public int PowerMode
        {
            get { return (int)GetValue(PowerModeProperty); }
            set { SetValue(PowerModeProperty, value); }
        }

        public ICommand PowerPlusCommand
        {
            get { return (ICommand)GetValue(PowerPlusCommandProperty); }
            set { SetValue(PowerPlusCommandProperty, value); }
        }

        public ICommand PowerRampAddCommand
        {
            get { return (ICommand)GetValue(PowerRampAddCommandProperty); }
            set { SetValue(PowerRampAddCommandProperty, value); }
        }

        public ICommand PowerRampDeleteCommand
        {
            get { return (ICommand)GetValue(PowerRampDeleteCommandProperty); }
            set { SetValue(PowerRampDeleteCommandProperty, value); }
        }

        public ICommand PowerRampEditCommand
        {
            get { return (ICommand)GetValue(PowerRampEditCommandProperty); }
            set { SetValue(PowerRampEditCommandProperty, value); }
        }

        public IList PowerRampsCustom
        {
            get { return (IList)GetValue(PowerRampsCustomProperty); }
            set { SetValue(PowerRampsCustomProperty, value); }
        }

        public int PowerRampSelected
        {
            get { return (int)GetValue(PowerRampSelectedProperty); }
            set { SetValue(PowerRampSelectedProperty, value); }
        }

        public ICommand PowerRegCalCommand
        {
            get { return (ICommand)GetValue(PowerRegCalCommandProperty); }
            set { SetValue(PowerRegCalCommandProperty, value); }
        }

        public Visibility PowerRegCalibrationVisibility
        {
            get { return (Visibility)GetValue(PowerRegCalibrationVisibilityProperty); }
            set { SetValue(PowerRegCalibrationVisibilityProperty, value); }
        }

        public string PowerRegCalName1
        {
            get { return (string)GetValue(PowerRegCalName1Property); }
            set { SetValue(PowerRegCalName1Property, value); }
        }

        public string PowerRegCalName2
        {
            get { return (string)GetValue(PowerRegCalName2Property); }
            set { SetValue(PowerRegCalName2Property, value); }
        }

        public string PowerRegCalName3
        {
            get { return (string)GetValue(PowerRegCalName3Property); }
            set { SetValue(PowerRegCalName3Property, value); }
        }

        public string PowerRegCalName4
        {
            get { return (string)GetValue(PowerRegCalName4Property); }
            set { SetValue(PowerRegCalName4Property, value); }
        }

        public string PowerRegCalName5
        {
            get { return (string)GetValue(PowerRegCalName5Property); }
            set { SetValue(PowerRegCalName5Property, value); }
        }

        public string PowerRegCalName6
        {
            get { return (string)GetValue(PowerRegCalName6Property); }
            set { SetValue(PowerRegCalName6Property, value); }
        }

        public ICommand PowerRegCalSaveCommand
        {
            get { return (ICommand)GetValue(PowerRegCalSaveCommandProperty); }
            set { SetValue(PowerRegCalSaveCommandProperty, value); }
        }

        public string PowerRegEncoderPosition
        {
            get { return (string)GetValue(PowerRegEncoderPositionProperty); }
            set { SetValue(PowerRegEncoderPositionProperty, value); }
        }

        public int PowerRegZero
        {
            get { return (int)GetValue(PowerRegZeroProperty); }
            set { SetValue(PowerRegZeroProperty, value); }
        }

        public double PowerStepSize
        {
            get { return (double)GetValue(PowerStepSizeProperty); }
            set { SetValue(PowerStepSizeProperty, value); }
        }

        public Visibility PowerStepSizeVisibility
        {
            get { return (Visibility)GetValue(PowerStepSizeVisibilityProperty); }
            set { SetValue(PowerStepSizeVisibilityProperty, value); }
        }

        public double PowerThreshold
        {
            get { return (double)GetValue(PowerThresholdProperty); }
            set { SetValue(PowerThresholdProperty, value); }
        }

        public ICommand SelectPockelsMaskCommand
        {
            get { return (ICommand)GetValue(SelectPockelsMaskCommandProperty); }
            set { SetValue(SelectPockelsMaskCommandProperty, value); }
        }

        public ICommand SetPowerCommand
        {
            get { return (ICommand)GetValue(SetPowerCommandProperty); }
            set { SetValue(SetPowerCommandProperty, value); }
        }

        public ICommand StepCoarseCommand
        {
            get { return (ICommand)GetValue(StepCoarseCommandProperty); }
            set { SetValue(StepCoarseCommandProperty, value); }
        }

        public ICommand StepFineCommand
        {
            get { return (ICommand)GetValue(StepFineCommandProperty); }
            set { SetValue(StepFineCommandProperty, value); }
        }

        public ICommand UpdatePockelsMaskToROIMaskCommand
        {
            get { return (ICommand)GetValue(UpdatePockelsMaskToROIMaskCommandProperty); }
            set { SetValue(UpdatePockelsMaskToROIMaskCommandProperty, value); }
        }

        public double VoltageMax
        {
            get { return (double)GetValue(VoltageMaxProperty); }
            set { SetValue(VoltageMaxProperty, value); }
        }

        public double VoltageMin
        {
            get { return (double)GetValue(VoltageMinProperty); }
            set { SetValue(VoltageMinProperty, value); }
        }

        #endregion Properties

        #region Methods

        public static void onEnableDevReadingChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onEnablePockelsMaskChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onMaskAndBlankingVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPCTabIndexChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPockelsBlankingPhaseShiftPercentChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPockelsBlankingPhaseShiftPercentVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPockelsBlankPercentageChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPockelsCalibrationVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPockelsMaskFileChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPockelsMaskInvertChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPockelsMaskOptionsAvailableChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerGoChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerMaxChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerMinChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerModeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerRampSelectedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerRegCalibrationVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerRegCalName1Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerRegCalName2Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerRegCalName3Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerRegCalName4Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerRegCalName5Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerRegCalName6Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerRegZeroChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerStepSizeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerStepSizeVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPowerThresholdChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onUpdatePockelsMaskToROIMaskCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onVoltageMaxChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onVoltageMinChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        private void FormattedSlider_DragCompleted(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            Application.Current.Dispatcher.Invoke((Action)(() =>
            {
                BindingExpression be = ((Slider)sender).GetBindingExpression(Slider.ValueProperty);
                be.UpdateSource();
            }));
            //EnableDevReading = true;
        }

        private void FormattedSlider_DragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
        {
            //EnableDevReading = false;
        }

        private void FormattedSlider_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            double newVal = Math.Round(((Slider)e.Source).Value);

            if (e.Delta > 0)
            {
                newVal += 1;
            }
            else if (e.Delta < 0)
            {
                newVal -= 1;
            }

            ((Slider)sender).Value = newVal;
            BindingExpression be = ((Slider)sender).GetBindingExpression(Slider.ValueProperty);
            be.UpdateSource();

            e.Handled = true;
        }

        #endregion Methods

        #region Other

        //public bool EnableDevReading
        //{
        //    get { return (bool)GetValue(EnableDevReadingProperty); }
        //    set { SetValue(EnableDevReadingProperty, value); }
        //}

        #endregion Other
    }
}