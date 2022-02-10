namespace ScanControl
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

    using ThorSharedTypes;

    public class BoolToOppositeBoolConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(bool))
                throw new InvalidOperationException("The target must be a boolean");

            return !(bool)value;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class DoubleCultureConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                //use parameter to display number of decimal places:
                string tmp = string.Empty;
                if (targetType == typeof(string))
                {
                    if (null != parameter)
                    {
                        tmp = "N" + Int32.Parse(parameter.ToString()).ToString();
                        return (Double.Parse(value.ToString()).ToString(tmp));
                    }
                    else
                    {
                        return (Double.Parse(value.ToString()).ToString());
                    }
                }
                else if (targetType == typeof(double))
                {
                    if (null != parameter)
                    {
                        tmp = "N" + Int32.Parse(parameter.ToString()).ToString();
                        return (Double.Parse(value.ToString()).ToString(tmp));
                    }
                    else
                    {
                        return (Double.Parse(value.ToString()));
                    }
                }
                else if (targetType == typeof(object))
                {
                    if (null != parameter)
                    {
                        tmp = "N" + Int32.Parse(parameter.ToString()).ToString();
                        return (Double.Parse(value.ToString()).ToString(tmp));
                    }
                    else
                    {
                        return (object)value.ToString();
                    }
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string or double");
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                if (targetType == typeof(string))
                {
                    return (Double.Parse(value.ToString())).ToString();
                }
                else if (targetType == typeof(double))
                {
                    return (Double.Parse(value.ToString()));
                }
                else if (targetType == typeof(object))
                {
                    return (object)value.ToString();
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string or double");
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        #endregion Methods
    }

    public class InputRangeConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            string str = string.Empty;

            switch ((int)value)
            {
                case 5: str = "100mV"; break;
                case 6: str = "200mV"; break;
                case 7: str = "400mV"; break;
                case 10: str = "1V"; break;
                case 11: str = "2V"; break;
                case 12: str = "4V"; break;
            }
            return str;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class InputRangeConverter2 : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            string str = string.Empty;

            switch ((int)value)
            {
                case 0: str = "OFF"; break;
                case 1: str = "4V"; break;
                case 2: str = "2.5V"; break;
                case 3: str = "1.5V"; break;
                case 4: str = "1V"; break;
                case 5: str = "650mV"; break;
                case 6: str = "400mV"; break;
                case 7: str = "250mV"; break;
                case 8: str = "150mV"; break;
            }
            return str;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class InputRangeMaxToVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            const int MAX_THORDAQ_INPUT_RANGE_INDEX = 8;

            if ((int?)value > MAX_THORDAQ_INPUT_RANGE_INDEX)
            {
                return parameter.ToString() == "0" ? Visibility.Visible : Visibility.Collapsed;
            }
            else
            {
                return parameter.ToString() == "0" ? Visibility.Collapsed : Visibility.Visible;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class InputRangeValConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            int ret = 1;
            switch ((int)value)
            {
                case 5: ret = 1; break;
                case 6: ret = 2; break;
                case 7: ret = 3; break;
                case 10: ret = 4; break;
                case 11: ret = 5; break;
                case 12: ret = 6; break;
                case 15: ret = 6; break;
            }
            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            int ret = 12;
            switch (value.ToString())
            {
                case "1": ret = 5; break;
                case "2": ret = 6; break;
                case "3": ret = 7; break;
                case "4": ret = 10; break;
                case "5": ret = 11; break;
                case "6": ret = 12; break;
                default: break;
            }
            return ret;
        }

        #endregion Methods
    }

    public class PMTOnBrushConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(Brush))
                throw new InvalidOperationException("The target must be a brush");

            Brush brush = Brushes.White;
            switch ((int)value)
            {
                case 0: brush = Brushes.Red; break;
                case 1: brush = Brushes.Green; break;
            }
            return brush;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class ScanControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty AverageFramesMinusCommandProperty = 
        DependencyProperty.Register(
        "AverageFramesMinusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty AverageFramesPlusCommandProperty = 
        DependencyProperty.Register(
        "AverageFramesPlusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty BandwidthListProperty = 
        DependencyProperty.Register(
        "BandwidthList",
        typeof(ObservableCollection<ObservableCollection<string>>),
        typeof(ScanControlUC));
        public static readonly DependencyProperty ChanDigOffsetMinusCommandProperty = 
        DependencyProperty.Register(
        "ChanDigOffsetMinusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty ChanDigOffsetPlusCommandProperty = 
        DependencyProperty.Register(
        "ChanDigOffsetPlusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty FlybackCyclesMinusCommandProperty = 
        DependencyProperty.Register(
        "FlybackCyclesMinusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty FlybackCyclesPlusCommandProperty = 
        DependencyProperty.Register(
        "FlybackCyclesPlusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty LSMAlignmentMinusCoarseCommandProperty = 
        DependencyProperty.Register(
        "LSMAlignmentMinusCoarseCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty LSMAlignmentMinusCommandProperty = 
        DependencyProperty.Register(
        "LSMAlignmentMinusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty LSMAlignmentPlusCoarseCommandProperty = 
        DependencyProperty.Register(
        "LSMAlignmentPlusCoarseCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty LSMAlignmentPlusCommandProperty = 
        DependencyProperty.Register(
        "LSMAlignmentPlusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty LSMDwellTimeMinusCommandProperty = 
        DependencyProperty.Register(
        "LSMDwellTimeMinusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty LSMDwellTimePlusCommandProperty = 
        DependencyProperty.Register(
        "LSMDwellTimePlusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty PMTGainMinusCommandProperty = 
        DependencyProperty.Register(
        "PMTGainMinusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty PMTGainPlusCommandProperty = 
        DependencyProperty.Register(
        "PMTGainPlusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty PMTOffsetMinusCommandProperty = 
        DependencyProperty.Register(
        "PMTOffsetMinusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty PMTOffsetPlusCommandProperty = 
        DependencyProperty.Register(
        "PMTOffsetPlusCommand",
        typeof(ICommand),
        typeof(ScanControlUC));
        public static readonly DependencyProperty TwoWayCalibrationCommandProperty = 
        DependencyProperty.Register(
        "TwoWayCalibrationCommand",
        typeof(ICommand),
        typeof(ScanControlUC));

        public static DependencyProperty BipolarityVisibilityProperty = 
        DependencyProperty.Register("BipolarityVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty ChanDigOffsetProperty = 
        DependencyProperty.Register("ChanDigOffset",
        typeof(CustomCollection<HwVal<int>>),
        typeof(ScanControlUC));
        public static DependencyProperty ChanDigOffsetVisibilityProperty = 
        DependencyProperty.Register("ChanDigOffsetVisibility",
        typeof(CustomCollection<Visibility>),
        typeof(ScanControlUC));
        public static DependencyProperty CoarsePanelVisibilityProperty = 
        DependencyProperty.Register("CoarsePanelVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty DigitizerBoardNameProperty = 
        DependencyProperty.Register("DigitizerBoardName",
        typeof(string),
        typeof(ScanControlUC));
        public static DependencyProperty DigOffsetLabelProperty = 
        DependencyProperty.Register("DigOffsetLabel",
        typeof(string),
        typeof(ScanControlUC));
        public static DependencyProperty DigOffsetVisibilityProperty = 
        DependencyProperty.Register("DigOffsetVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty DwellTimeSliderEnabledProperty = 
        DependencyProperty.Register("DwellTimeSliderEnabled",
        typeof(bool),
        typeof(ScanControlUC));
        public static DependencyProperty FastOneWayImagingModeEnableVisibilityProperty = 
        DependencyProperty.Register("FastOneWayImagingModeEnableVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty FramesPerSecondAverageProperty = 
        DependencyProperty.Register("FramesPerSecondAverage",
        typeof(string),
        typeof(ScanControlUC));
        public static DependencyProperty GGLSMScanVisibilityProperty = 
        DependencyProperty.Register("GGLSMScanVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty GRLSMScanVisibilityProperty = 
        DependencyProperty.Register("GRLSMScanVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty InputRangeMaxProperty = 
        DependencyProperty.Register("InputRangeMax",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty InputRangeMinProperty = 
        DependencyProperty.Register("InputRangeMin",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty InputRangeProperty = 
        DependencyProperty.Register("InputRange",
        typeof(CustomCollection<HwVal<int>>),
        typeof(ScanControlUC));
        public static DependencyProperty IsChannelVisibleProperty = 
        DependencyProperty.Register("IsChannelVisible",
        typeof(CustomCollection<Visibility>),
        typeof(ScanControlUC));
        public static DependencyProperty LSMAverageEnabledProperty = 
           DependencyProperty.Register("LSMAverageEnabled",
           typeof(bool),
           typeof(ScanControlUC));
        public static DependencyProperty LsmClkPnlEnabledProperty = 
        DependencyProperty.Register("LsmClkPnlEnabled",
        typeof(bool),
        typeof(ScanControlUC));
        public static DependencyProperty LSMClockSourceProperty = 
        DependencyProperty.Register("LSMClockSource",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMExtClockRateProperty = 
        DependencyProperty.Register("LSMExtClockRate",
        typeof(double),
        typeof(ScanControlUC));
        public static DependencyProperty LSMExternalClockPhaseOffsetProperty = 
        DependencyProperty.Register("LSMExternalClockPhaseOffset",
        typeof(double),
        typeof(ScanControlUC));
        public static DependencyProperty LSMFastOneWayImagingModeEnableProperty = 
        DependencyProperty.Register("LSMFastOneWayImagingModeEnable",
        typeof(bool),
        typeof(ScanControlUC));
        public static DependencyProperty LSMFlybackCyclesProperty = 
        DependencyProperty.Register("LSMFlybackCycles",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMFlybackTimeProperty = 
        DependencyProperty.Register("LSMFlybackTime",
        typeof(double),
        typeof(ScanControlUC));
        public static DependencyProperty LSMInterleaveScanProperty = 
        DependencyProperty.Register("LSMInterleaveScan",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMPhaseAdjusmentVisibilityProperty = 
        DependencyProperty.Register("LSMPhaseAdjusmentVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty LSMPixelDwellTimeIndexProperty = 
        DependencyProperty.Register("LSMPixelDwellTimeIndex",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMPixelDwellTimeMaxIndexProperty = 
        DependencyProperty.Register("LSMPixelDwellTimeMaxIndex",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMPixelDwellTimeMinIndexProperty = 
        DependencyProperty.Register("LSMPixelDwellTimeMinIndex",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMPixelDwellTimeProperty = 
        DependencyProperty.Register("LSMPixelDwellTime",
        typeof(double),
        typeof(ScanControlUC));
        public static DependencyProperty LSMPixelProcessProperty = 
        DependencyProperty.Register("LSMPixelProcess",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMPixelProcessVisibilityProperty = 
        DependencyProperty.Register("LSMPixelProcessVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty LSMPulseMultiplexingProperty = 
        DependencyProperty.Register("LSMPulseMultiplexing",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMPulseMultiplexingVisibilityProperty = 
        DependencyProperty.Register("LSMPulseMultiplexingVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty LSMRealtimeAveragingProperty = 
        DependencyProperty.Register("LSMRealtimeAveraging",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMScanModeProperty = 
        DependencyProperty.Register("LSMScanMode",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMSignalAverageFramesProperty = 
        DependencyProperty.Register("LSMSignalAverageFrames",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMSignalAverageProperty = 
        DependencyProperty.Register("LSMSignalAverage",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMTwoWayAlignmentCoarseProperty = 
        DependencyProperty.Register("LSMTwoWayAlignmentCoarse",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty LSMTwoWayAlignmentProperty = 
        DependencyProperty.Register("LSMTwoWayAlignment",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty NumberOfPulsesPerPixelProperty = 
        DependencyProperty.Register("NumberOfPulsesPerPixel",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty Pmt1BandwidthSelectedProperty = 
        DependencyProperty.Register("Pmt1BandwidthSelected",
        typeof(string),
        typeof(ScanControlUC));
        public static DependencyProperty Pmt2BandwidthSelectedProperty = 
        DependencyProperty.Register("Pmt2BandwidthSelected",
        typeof(string),
        typeof(ScanControlUC));
        public static DependencyProperty Pmt3BandwidthSelectedProperty = 
        DependencyProperty.Register("Pmt3BandwidthSelected",
        typeof(string),
        typeof(ScanControlUC));
        public static DependencyProperty Pmt4BandwidthSelectedProperty = 
        DependencyProperty.Register("Pmt4BandwidthSelected",
        typeof(string),
        typeof(ScanControlUC));
        public static DependencyProperty PMTBandwidthLabelVisibilityProperty = 
        DependencyProperty.Register("PMTBandwidthLabelVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty PmtBandwidthVisibilityProperty = 
        DependencyProperty.Register("PmtBandwidthVisibility",
        typeof(CustomCollection<Visibility>),
        typeof(ScanControlUC));
        public static DependencyProperty PMTGainProperty = 
        DependencyProperty.Register("PMTGain",
        typeof(CustomCollection<HwVal<int>>),
        typeof(ScanControlUC));
        public static DependencyProperty PMTOffsetLabelVisibilityProperty = 
        DependencyProperty.Register("PMTOffsetLabelVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty PMTOffsetProperty = 
        DependencyProperty.Register("PMTOffset",
        typeof(CustomCollection<HwVal<double>>),
        typeof(ScanControlUC));
        public static DependencyProperty PMTOffsetVisibilityProperty = 
        DependencyProperty.Register("PMTOffsetVisibility",
        typeof(CustomCollection<Visibility>),
        typeof(ScanControlUC));
        public static DependencyProperty PMTOnProperty = 
        DependencyProperty.Register("PMTOn",
        typeof(CustomCollection<HwVal<int>>),
        typeof(ScanControlUC));
        public static DependencyProperty PMTPolarityProperty = 
        DependencyProperty.Register("PMTPolarity",
        typeof(CustomCollection<HwVal<int>>),
        typeof(ScanControlUC));
        public static DependencyProperty PMTVoltProperty = 
        DependencyProperty.Register("PMTVolt",
        typeof(CustomCollection<HwVal<double>>),
        typeof(ScanControlUC));
        public static DependencyProperty PulsesPerPixelVisibilityProperty = 
        DependencyProperty.Register("PulsesPerPixelVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty RapidScanVisibilityProperty = 
        DependencyProperty.Register("RapidScanVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty TurnAroundOptionVisibilityProperty = 
        DependencyProperty.Register("TurnAroundOptionVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty TurnAroundTimeUSProperty = 
        DependencyProperty.Register("TurnAroundTimeUS",
        typeof(int),
        typeof(ScanControlUC));
        public static DependencyProperty TwoWayCalibrationVisibilityProperty = 
        DependencyProperty.Register("TwoWayCalibrationVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty TwoWayVisibilityProperty = 
        DependencyProperty.Register("TwoWayVisibility",
        typeof(Visibility),
        typeof(ScanControlUC));
        public static DependencyProperty UpdateDwellTimeProperty = 
        DependencyProperty.RegisterAttached(
        "UpdateDwellTime",
        typeof(bool),
        typeof(ScanControlUC));
        public static DependencyProperty UseFastestFlybackEnabledProperty = 
           DependencyProperty.Register("UseFastestFlybackEnabled",
           typeof(bool),
           typeof(ScanControlUC));
        public static DependencyProperty UseFastestSettingForFlybackCyclesProperty = 
        DependencyProperty.Register("UseFastestSettingForFlybackCycles",
        typeof(int),
        typeof(ScanControlUC));

        #endregion Fields

        #region Constructors

        public ScanControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public ICommand AverageFramesMinusCommand
        {
            get { return (ICommand)GetValue(AverageFramesMinusCommandProperty); }
            set { SetValue(AverageFramesMinusCommandProperty, value); }
        }

        public ICommand AverageFramesPlusCommand
        {
            get { return (ICommand)GetValue(AverageFramesPlusCommandProperty); }
            set { SetValue(AverageFramesPlusCommandProperty, value); }
        }

        public ObservableCollection<ObservableCollection<string>> BandwidthList
        {
            get { return (ObservableCollection<ObservableCollection<string>>)GetValue(BandwidthListProperty); }
            set { SetValue(BandwidthListProperty, value); }
        }

        public Visibility BipolarityVisibility
        {
            get { return (Visibility)GetValue(BipolarityVisibilityProperty); }
            set { SetValue(BipolarityVisibilityProperty, value); }
        }

        public CustomCollection<HwVal<int>> ChanDigOffset
        {
            get { return (CustomCollection<HwVal<int>>)GetValue(ChanDigOffsetProperty); }
            set { SetValue(ChanDigOffsetProperty, value); }
        }

        public ICommand ChanDigOffsetMinusCommand
        {
            get { return (ICommand)GetValue(ChanDigOffsetMinusCommandProperty); }
            set { SetValue(ChanDigOffsetMinusCommandProperty, value); }
        }

        public ICommand ChanDigOffsetPlusCommand
        {
            get { return (ICommand)GetValue(ChanDigOffsetPlusCommandProperty); }
            set { SetValue(ChanDigOffsetPlusCommandProperty, value); }
        }

        public CustomCollection<Visibility> ChanDigOffsetVisibility
        {
            get { return (CustomCollection<Visibility>)GetValue(ChanDigOffsetVisibilityProperty); }
            set { SetValue(ChanDigOffsetVisibilityProperty, value); }
        }

        public Visibility CoarsePanelVisibility
        {
            get { return (Visibility)GetValue(CoarsePanelVisibilityProperty); }
            set { SetValue(CoarsePanelVisibilityProperty, value); }
        }

        public string DigitizerBoardName
        {
            get { return (string)GetValue(DigitizerBoardNameProperty); }
            set { SetValue(DigitizerBoardNameProperty, value); }
        }

        public string DigOffsetLabel
        {
            get { return (string)GetValue(DigOffsetLabelProperty); }
            set { SetValue(DigOffsetLabelProperty, value); }
        }

        public Visibility DigOffsetVisibility
        {
            get { return (Visibility)GetValue(DigOffsetVisibilityProperty); }
            set { SetValue(DigOffsetVisibilityProperty, value); }
        }

        public bool DwellTimeSliderEnabled
        {
            get { return (bool)GetValue(DwellTimeSliderEnabledProperty); }
            set { SetValue(DwellTimeSliderEnabledProperty, value); }
        }

        public Visibility FastOneWayImagingModeEnableVisibility
        {
            get { return (Visibility)GetValue(FastOneWayImagingModeEnableVisibilityProperty); }
            set { SetValue(FastOneWayImagingModeEnableVisibilityProperty, value); }
        }

        public ICommand FlybackCyclesMinusCommand
        {
            get { return (ICommand)GetValue(FlybackCyclesMinusCommandProperty); }
            set { SetValue(FlybackCyclesMinusCommandProperty, value); }
        }

        public ICommand FlybackCyclesPlusCommand
        {
            get { return (ICommand)GetValue(FlybackCyclesPlusCommandProperty); }
            set { SetValue(FlybackCyclesPlusCommandProperty, value); }
        }

        public int FramesPerSecondAverage
        {
            get { return (int)GetValue(FramesPerSecondAverageProperty); }
            set { SetValue(FramesPerSecondAverageProperty, value); }
        }

        public Visibility GGLSMScanVisibility
        {
            get { return (Visibility)GetValue(GGLSMScanVisibilityProperty); }
            set { SetValue(GGLSMScanVisibilityProperty, value); }
        }

        public Visibility GRLSMScanVisibility
        {
            get { return (Visibility)GetValue(GRLSMScanVisibilityProperty); }
            set { SetValue(GRLSMScanVisibilityProperty, value); }
        }

        public CustomCollection<HwVal<int>> InputRange
        {
            get { return (CustomCollection<HwVal<int>>)GetValue(InputRangeProperty); }
            set { SetValue(InputRangeProperty, value); }
        }

        public int InputRangeMax
        {
            get { return (int)GetValue(InputRangeMaxProperty); }
            set { SetValue(InputRangeMaxProperty, value); }
        }

        public int InputRangeMin
        {
            get { return (int)GetValue(InputRangeMinProperty); }
            set { SetValue(InputRangeMinProperty, value); }
        }

        public CustomCollection<Visibility> IsChannelVisible
        {
            get { return (CustomCollection<Visibility>)GetValue(IsChannelVisibleProperty); }
            set { SetValue(IsChannelVisibleProperty, value); }
        }

        public ICommand LSMAlignmentMinusCoarseCommand
        {
            get { return (ICommand)GetValue(LSMAlignmentMinusCoarseCommandProperty); }
            set { SetValue(LSMAlignmentMinusCoarseCommandProperty, value); }
        }

        public ICommand LSMAlignmentMinusCommand
        {
            get { return (ICommand)GetValue(LSMAlignmentMinusCommandProperty); }
            set { SetValue(LSMAlignmentMinusCommandProperty, value); }
        }

        public ICommand LSMAlignmentPlusCoarseCommand
        {
            get { return (ICommand)GetValue(LSMAlignmentPlusCoarseCommandProperty); }
            set { SetValue(LSMAlignmentPlusCoarseCommandProperty, value); }
        }

        public ICommand LSMAlignmentPlusCommand
        {
            get { return (ICommand)GetValue(LSMAlignmentPlusCommandProperty); }
            set { SetValue(LSMAlignmentPlusCommandProperty, value); }
        }

        public bool LSMAverageEnabled
        {
            get { return (bool)GetValue(LSMAverageEnabledProperty); }
            set { SetValue(LSMAverageEnabledProperty, value); }
        }

        public bool LsmClkPnlEnabled
        {
            get { return (bool)GetValue(LsmClkPnlEnabledProperty); }
            set { SetValue(LsmClkPnlEnabledProperty, value); }
        }

        public int LSMClockSource
        {
            get { return (int)GetValue(LSMClockSourceProperty); }
            set { SetValue(LSMClockSourceProperty, value); }
        }

        public ICommand LSMDwellTimeMinusCommand
        {
            get { return (ICommand)GetValue(LSMDwellTimeMinusCommandProperty); }
            set { SetValue(LSMDwellTimeMinusCommandProperty, value); }
        }

        public ICommand LSMDwellTimePlusCommand
        {
            get { return (ICommand)GetValue(LSMDwellTimePlusCommandProperty); }
            set { SetValue(LSMDwellTimePlusCommandProperty, value); }
        }

        public double LSMExtClockRate
        {
            get { return (double)GetValue(LSMExtClockRateProperty); }
            set { SetValue(LSMExtClockRateProperty, value); }
        }

        public double LSMExternalClockPhaseOffset
        {
            get { return (double)GetValue(LSMExternalClockPhaseOffsetProperty); }
            set { SetValue(LSMExternalClockPhaseOffsetProperty, value); }
        }

        public bool LSMFastOneWayImagingModeEnable
        {
            get { return (bool)GetValue(LSMFastOneWayImagingModeEnableProperty); }
            set { SetValue(LSMFastOneWayImagingModeEnableProperty, value); }
        }

        public int LSMFlybackCycles
        {
            get { return (int)GetValue(LSMFlybackCyclesProperty); }
            set { SetValue(LSMFlybackCyclesProperty, value); }
        }

        public double LSMFlybackTime
        {
            get { return (double)GetValue(LSMFlybackTimeProperty); }
            set { SetValue(LSMFlybackTimeProperty, value); }
        }

        public int LSMInterleaveScan
        {
            get { return (int)GetValue(LSMInterleaveScanProperty); }
            set { SetValue(LSMInterleaveScanProperty, value); }
        }

        public Visibility LSMPhaseAdjusmentVisibility
        {
            get { return (Visibility)GetValue(LSMPhaseAdjusmentVisibilityProperty); }
            set { SetValue(LSMPhaseAdjusmentVisibilityProperty, value); }
        }

        public double LSMPixelDwellTime
        {
            get { return (double)GetValue(LSMPixelDwellTimeProperty); }
            set { SetValue(LSMPixelDwellTimeProperty, value); }
        }

        public int LSMPixelDwellTimeIndex
        {
            get { return (int)GetValue(LSMPixelDwellTimeIndexProperty); }
            set { SetValue(LSMPixelDwellTimeIndexProperty, value); }
        }

        public int LSMPixelDwellTimeMaxIndex
        {
            get { return (int)GetValue(LSMPixelDwellTimeMaxIndexProperty); }
            set { SetValue(LSMPixelDwellTimeMaxIndexProperty, value); }
        }

        public int LSMPixelDwellTimeMinIndex
        {
            get { return (int)GetValue(LSMPixelDwellTimeMinIndexProperty); }
            set { SetValue(LSMPixelDwellTimeMinIndexProperty, value); }
        }

        public int LSMPixelProcess
        {
            get { return (int)GetValue(LSMPixelProcessProperty); }
            set { SetValue(LSMPixelProcessProperty, value); }
        }

        public Visibility LSMPixelProcessVisibility
        {
            get { return (Visibility)GetValue(LSMPixelProcessVisibilityProperty); }
            set { SetValue(LSMPixelProcessVisibilityProperty, value); }
        }

        public int LSMPulseMultiplexing
        {
            get { return (int)GetValue(LSMPulseMultiplexingProperty); }
            set { SetValue(LSMPulseMultiplexingProperty, value); }
        }

        public Visibility LSMPulseMultiplexingVisibility
        {
            get { return (Visibility)GetValue(LSMPulseMultiplexingVisibilityProperty); }
            set { SetValue(LSMPulseMultiplexingVisibilityProperty, value); }
        }

        public int LSMRealtimeAveraging
        {
            get { return (int)GetValue(LSMRealtimeAveragingProperty); }
            set { SetValue(LSMRealtimeAveragingProperty, value); }
        }

        public int LSMScanMode
        {
            get { return (int)GetValue(LSMScanModeProperty); }
            set { SetValue(LSMScanModeProperty, value); }
        }

        public int LSMSignalAverage
        {
            get { return (int)GetValue(LSMSignalAverageProperty); }
            set { SetValue(LSMSignalAverageProperty, value); }
        }

        public int LSMSignalAverageFrames
        {
            get { return (int)GetValue(LSMSignalAverageFramesProperty); }
            set { SetValue(LSMSignalAverageFramesProperty, value); }
        }

        public int LSMTwoWayAlignment
        {
            get { return (int)GetValue(LSMTwoWayAlignmentProperty); }
            set { SetValue(LSMTwoWayAlignmentProperty, value); }
        }

        public int LSMTwoWayAlignmentCoarse
        {
            get { return (int)GetValue(LSMTwoWayAlignmentCoarseProperty); }
            set { SetValue(LSMTwoWayAlignmentCoarseProperty, value); }
        }

        public int NumberOfPulsesPerPixel
        {
            get { return (int)GetValue(NumberOfPulsesPerPixelProperty); }
            set { SetValue(NumberOfPulsesPerPixelProperty, value); }
        }

        public string Pmt1BandwidthSelected
        {
            get { return (string)GetValue(Pmt1BandwidthSelectedProperty); }
            set { SetValue(Pmt1BandwidthSelectedProperty, value); }
        }

        public string Pmt2BandwidthSelected
        {
            get { return (string)GetValue(Pmt2BandwidthSelectedProperty); }
            set { SetValue(Pmt2BandwidthSelectedProperty, value); }
        }

        public string Pmt3BandwidthSelected
        {
            get { return (string)GetValue(Pmt3BandwidthSelectedProperty); }
            set { SetValue(Pmt3BandwidthSelectedProperty, value); }
        }

        public string Pmt4BandwidthSelected
        {
            get { return (string)GetValue(Pmt4BandwidthSelectedProperty); }
            set { SetValue(Pmt4BandwidthSelectedProperty, value); }
        }

        public Visibility PMTBandwidthLabelVisibility
        {
            get { return (Visibility)GetValue(PMTBandwidthLabelVisibilityProperty); }
            set { SetValue(PMTBandwidthLabelVisibilityProperty, value); }
        }

        public CustomCollection<Visibility> PmtBandwidthVisibility
        {
            get { return (CustomCollection<Visibility>)GetValue(PmtBandwidthVisibilityProperty); }
            set { SetValue(PmtBandwidthVisibilityProperty, value); }
        }

        public CustomCollection<HwVal<int>> PMTGain
        {
            get { return (CustomCollection<HwVal<int>>)GetValue(PMTGainProperty); }
            set { SetValue(PMTGainProperty, value); }
        }

        public ICommand PMTGainMinusCommand
        {
            get { return (ICommand)GetValue(PMTGainMinusCommandProperty); }
            set { SetValue(PMTGainMinusCommandProperty, value); }
        }

        public ICommand PMTGainPlusCommand
        {
            get { return (ICommand)GetValue(PMTGainPlusCommandProperty); }
            set { SetValue(PMTGainPlusCommandProperty, value); }
        }

        public CustomCollection<HwVal<double>> PMTOffset
        {
            get { return (CustomCollection<HwVal<double>>)GetValue(PMTOffsetProperty); }
            set { SetValue(PMTOffsetProperty, value); }
        }

        public Visibility PMTOffsetLabelVisibility
        {
            get { return (Visibility)GetValue(PMTOffsetLabelVisibilityProperty); }
            set { SetValue(PMTOffsetLabelVisibilityProperty, value); }
        }

        public ICommand PMTOffsetMinusCommand
        {
            get { return (ICommand)GetValue(PMTOffsetMinusCommandProperty); }
            set { SetValue(PMTOffsetMinusCommandProperty, value); }
        }

        public ICommand PMTOffsetPlusCommand
        {
            get { return (ICommand)GetValue(PMTOffsetPlusCommandProperty); }
            set { SetValue(PMTOffsetPlusCommandProperty, value); }
        }

        public CustomCollection<Visibility> PMTOffsetVisibility
        {
            get { return (CustomCollection<Visibility>)GetValue(PMTOffsetVisibilityProperty); }
            set { SetValue(PMTOffsetVisibilityProperty, value); }
        }

        public CustomCollection<HwVal<int>> PMTOn
        {
            get { return (CustomCollection<HwVal<int>>)GetValue(PMTOnProperty); }
            set { SetValue(PMTOnProperty, value); }
        }

        public CustomCollection<HwVal<int>> PMTPolarity
        {
            get { return (CustomCollection<HwVal<int>>)GetValue(PMTPolarityProperty); }
            set { SetValue(PMTPolarityProperty, value); }
        }

        public CustomCollection<HwVal<double>> PMTVolt
        {
            get { return (CustomCollection<HwVal<double>>)GetValue(PMTVoltProperty); }
            set { SetValue(PMTVoltProperty, value); }
        }

        public Visibility PulsesPerPixelVisibility
        {
            get { return (Visibility)GetValue(PulsesPerPixelVisibilityProperty); }
            set { SetValue(PulsesPerPixelVisibilityProperty, value); }
        }

        public Visibility RapidScanVisibility
        {
            get { return (Visibility)GetValue(RapidScanVisibilityProperty); }
            set { SetValue(RapidScanVisibilityProperty, value); }
        }

        public Visibility TurnAroundOptionVisibility
        {
            get { return (Visibility)GetValue(TurnAroundOptionVisibilityProperty); }
            set { SetValue(TurnAroundOptionVisibilityProperty, value); }
        }

        public int TurnAroundTimeUS
        {
            get { return (int)GetValue(TurnAroundTimeUSProperty); }
            set { SetValue(TurnAroundTimeUSProperty, value); }
        }

        public ICommand TwoWayCalibrationCommand
        {
            get { return (ICommand)GetValue(TwoWayCalibrationCommandProperty); }
            set { SetValue(TwoWayCalibrationCommandProperty, value); }
        }

        public Visibility TwoWayCalibrationVisibility
        {
            get { return (Visibility)GetValue(TwoWayCalibrationVisibilityProperty); }
            set { SetValue(TwoWayCalibrationVisibilityProperty, value); }
        }

        public Visibility TwoWayVisibility
        {
            get { return (Visibility)GetValue(TwoWayVisibilityProperty); }
            set { SetValue(TwoWayVisibilityProperty, value); }
        }

        public bool UpdateDwellTime
        {
            get { return (bool)GetValue(UpdateDwellTimeProperty); }
            set { SetValue(UpdateDwellTimeProperty, value); }
        }

        public bool UseFastestFlybackEnabled
        {
            get { return (bool)GetValue(UseFastestFlybackEnabledProperty); }
            set { SetValue(UseFastestFlybackEnabledProperty, value); }
        }

        public int UseFastestSettingForFlybackCycles
        {
            get { return (int)GetValue(UseFastestSettingForFlybackCyclesProperty); }
            set { SetValue(UseFastestSettingForFlybackCyclesProperty, value); }
        }

        #endregion Properties

        #region Methods

        private void slDwellTime_GotMouseCapture(object sender, MouseEventArgs e)
        {
            UpdateDwellTime = false;
        }

        private void slDwellTime_PreviewMouseUp(object sender, MouseButtonEventArgs e)
        {
            UpdateDwellTime = true;
        }

        //public static readonly DependencyProperty **REPLACE**CommandProperty =
        // DependencyProperty.Register(
        // "**REPLACE**Command",
        // typeof(ICommand),
        // typeof(ScanControlUC));
        // public ICommand **REPLACE**
        // {
        // get { return (ICommand)GetValue(**REPLACE**CommandProperty); }
        // set { SetValue(**REPLACE**CommandProperty, value); }
        // }
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.Register("**REPLACE**",
        //typeof(int),
        //typeof(ScanControlUC));
        //on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        // get { return (int)GetValue(**REPLACE**Property); }
        // set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.Register("**REPLACE**",
        //typeof(int),
        //typeof(ScanControlUC));
        //on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        // get { return (int)GetValue(**REPLACE**Property); }
        // set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.Register("**REPLACE**",
        //typeof(int),
        //typeof(ScanControlUC));
        //on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        // get { return (int)GetValue(**REPLACE**Property); }
        // set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}
        //public static readonly DependencyProperty **REPLACE**Property =
        //DependencyProperty.Register(
        //"**REPLACE**",
        //typeof(int),
        //typeof(ScanControlUC));
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.Register("**REPLACE**",
        //typeof(int),
        //typeof(ScanControlUC));
        //on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        // get { return (int)GetValue(**REPLACE**Property); }
        // set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}
        /// <summary>
        /// Responds to the resolution preset selector being selected by updating the LSM resolutions and
        /// deselecting the just selected resolution, causing the selector to show the default text again,
        /// which is the current resolution
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void slDwellTime_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            //if (_liveVM != null && Visibility.Visible == spDwellTime.Visibility)
            //{
            // _twoWayDialog.SelectedIndex = Convert.ToInt32(((Slider)sender).Value);
            //}
        }

        private void txtTwoWayCoarse_TextChanged(object sender, TextChangedEventArgs e)
        {
            //try
            //{
            // if (_liveVM != null)
            // {
            // if (Visibility.Visible == coarsePanel.Visibility)
            // {
            // int convertion = 0;
            // if (true == Int32.TryParse(((TextBox)sender).Text, out convertion))
            // {
            // _twoWayDialog.CurrentOffset = convertion;
            // }
            // }
            // }
            //}
            //catch (FormatException ex)
            //{
            // string str = ex.Message;
            //}
        }

        private void txtTwoWay_TextChanged(object sender, TextChangedEventArgs e)
        {
            //try
            //{
            // if (_liveVM != null)
            // {
            // if (Visibility.Visible != coarsePanel.Visibility)
            // {
            // int convertion = 0;
            // if (true == Int32.TryParse(((TextBox)sender).Text, out convertion))
            // {
            // _twoWayDialog.CurrentOffset = convertion;
            // }
            // }
            // }
            //}
            //catch (FormatException ex)
            //{
            // string str = ex.Message;
            // ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + ex.Message);

            //}
        }

        #endregion Methods
    }
}