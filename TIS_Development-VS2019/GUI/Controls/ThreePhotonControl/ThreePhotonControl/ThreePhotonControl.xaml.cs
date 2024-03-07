namespace ThreePhotonControl
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

    public class ADCGainConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            string str = string.Empty;

            switch ((int)value)
            {
                case 0: str = "No Gain"; break;
                case 8: str = "1 dB"; break;
                case 9: str = "5 dB"; break;
                case 10: str = "9 dB"; break;
                case 11: str = "13 dB"; break;
                case 12: str = "17 dB"; break;
                case 13: str = "21 dB"; break;
                case 14: str = "25 dB"; break;
                case 15: str = "29 dB"; break;
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

    public class ADCGainValConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            int ret = 1;
            switch ((int)value)
            {
                case 0: ret = 0; break;
                case 8: ret = 1; break;
                case 9: ret = 2; break;
                case 10: ret = 3; break;
                case 11: ret = 4; break;
                case 12: ret = 5; break;
                case 13: ret = 6; break;
                case 14: ret = 7; break;
                case 15: ret = 8; break;
            }
            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            int ret = 12;
            switch (value.ToString())
            {
                case "0": ret = 0; break;
                case "1": ret = 8; break;
                case "2": ret = 9; break;
                case "3": ret = 10; break;
                case "4": ret = 11; break;
                case "5": ret = 12; break;
                case "6": ret = 13; break;
                case "7": ret = 14; break;
                case "8": ret = 15; break;
                default: break;
            }
            return ret;
        }

        #endregion Methods
    }

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class ThreePhotonControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty NumberOfPlanesMinusCommandProperty = 
            DependencyProperty.Register(
            "NumberOfPlanesMinusCommand",
            typeof(ICommand),
            typeof(ThreePhotonControlUC));
        public static readonly DependencyProperty NumberOfPlanesPlusCommandProperty = 
            DependencyProperty.Register(
            "NumberOfPlanesPlusCommand",
            typeof(ICommand),
            typeof(ThreePhotonControlUC));
        public static readonly DependencyProperty ThreePhotonPhaseCoarseMinusCommandProperty = 
            DependencyProperty.Register(
            "ThreePhotonPhaseCoarseMinusCommand",
            typeof(ICommand),
            typeof(ThreePhotonControlUC));
        public static readonly DependencyProperty ThreePhotonPhaseCoarsePlusCommandProperty = 
            DependencyProperty.Register(
            "ThreePhotonPhaseCoarsePlusCommand",
            typeof(ICommand),
            typeof(ThreePhotonControlUC));
        public static readonly DependencyProperty ThreePhotonPhaseFineMinusCommandProperty = 
            DependencyProperty.Register(
            "ThreePhotonPhaseFineMinusCommand",
            typeof(ICommand),
            typeof(ThreePhotonControlUC));
        public static readonly DependencyProperty ThreePhotonPhaseFinePlusCommandProperty = 
            DependencyProperty.Register(
            "ThreePhotonPhaseFinePlusCommand",
            typeof(ICommand),
            typeof(ThreePhotonControlUC));
        public static readonly DependencyProperty DownsamplingRateMinusCommandProperty =
           DependencyProperty.Register(
           "DownsamplingRateMinusCommand",
           typeof(ICommand),
           typeof(ThreePhotonControlUC));
        public static readonly DependencyProperty DownsamplingRatePlusCommandProperty =
           DependencyProperty.Register(
           "DownsamplingRatePlusCommand",
           typeof(ICommand),
           typeof(ThreePhotonControlUC));

        public static DependencyProperty Disable3PCheckboxProperty = 
            DependencyProperty.Register("Disable3PCheckbox",
            typeof(bool),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty FIR1ManualControlEnableProperty = 
            DependencyProperty.Register("FIR1ManualControlEnable",
            typeof(int),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty FIRSettingsVisibilityProperty = 
            DependencyProperty.Register("FIRSettingsVisibility",
            typeof(Visibility),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty LSMFIRFilterIndexProperty = 
            DependencyProperty.Register("LSMFIRFilterIndex",
            typeof(int),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty LSMFIRFilterTapIndexProperty = 
            DependencyProperty.Register("LSMFIRFilterTapIndex",
            typeof(int),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty LSMFIRFilterTapValueProperty = 
            DependencyProperty.Register("LSMFIRFilterTapValue",
            typeof(double),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty LSMNumberOfPlanesProperty = 
           DependencyProperty.Register("LSMNumberOfPlanes",
           typeof(int),
           typeof(ThreePhotonControlUC));
        public static DependencyProperty MultiplaneVisibilityProperty = 
           DependencyProperty.Register("MultiplaneVisibility",
           typeof(Visibility),
           typeof(ThreePhotonControlUC));
        public static DependencyProperty ThreePhotonEnableProperty = 
            DependencyProperty.Register("ThreePhotonEnable",
            typeof(bool),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty ThreePhotonFreqProperty = 
            DependencyProperty.Register("ThreePhotonFreq",
            typeof(double),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty ThreePhotonMeasureFrequencyCommandProperty = 
            DependencyProperty.Register("ThreePhotonMeasureFrequencyCommand",
            typeof(ICommand),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty ThreePhotonPanelEnableProperty = 
            DependencyProperty.Register("ThreePhotonPanelEnable",
            typeof(bool),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty ThreePhotonPhaseCoarseProperty = 
            DependencyProperty.Register("ThreePhotonPhaseCoarse",
            typeof(int),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty ThreePhotonPhaseCoarseVisibilityProperty = 
            DependencyProperty.Register("ThreePhotonPhaseCoarseVisibility",
            typeof(Visibility),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty ThreePhotonPhaseFineProperty = 
            DependencyProperty.Register("ThreePhotonPhaseFine",
            typeof(int),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty ThreePhotonDownsamplingRateProperty =
            DependencyProperty.Register("ThreePhotonDownsamplingRate",
            typeof(int),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty DownsamplingRateVisibilityProperty =
            DependencyProperty.Register("DownsamplingRateVisibility",
            typeof(int),
            typeof(ThreePhotonControlUC));
        public static DependencyProperty EnableDownsamplingRateChangeProperty =
           DependencyProperty.Register("EnableDownsamplingRateChange",
           typeof(bool),
           typeof(ThreePhotonControlUC));

        #endregion Fields

        #region Constructors

        public ThreePhotonControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public bool Disable3PCheckbox
        {
            get { return (bool)GetValue(Disable3PCheckboxProperty); }
            set { SetValue(Disable3PCheckboxProperty, value); }
        }

        public int FIR1ManualControlEnable
        {
            get { return (int)GetValue(FIR1ManualControlEnableProperty); }
            set { SetValue(FIR1ManualControlEnableProperty, value); }
        }

        public Visibility FIRSettingsVisibility
        {
            get { return (Visibility)GetValue(FIRSettingsVisibilityProperty); }
            set { SetValue(FIRSettingsVisibilityProperty, value); }
        }

        public int LSMFIRFilterIndex
        {
            get { return (int)GetValue(LSMFIRFilterIndexProperty); }
            set { SetValue(LSMFIRFilterIndexProperty, value); }
        }

        public int LSMFIRFilterTapIndex
        {
            get { return (int)GetValue(LSMFIRFilterTapIndexProperty); }
            set { SetValue(LSMFIRFilterTapIndexProperty, value); }
        }

        public double LSMFIRFilterTapValue
        {
            get { return (double)GetValue(LSMFIRFilterTapValueProperty); }
            set { SetValue(LSMFIRFilterTapValueProperty, value); }
        }

        public int LSMNumberOfPlanes
        {
            get { return (int)GetValue(LSMNumberOfPlanesProperty); }
            set { SetValue(LSMNumberOfPlanesProperty, value); }
        }

        public Visibility MultiplaneVisibility
        {
            get { return (Visibility)GetValue(MultiplaneVisibilityProperty); }
            set { SetValue(MultiplaneVisibilityProperty, value); }
        }

        public ICommand NumberOfPlanesMinusCommand
        {
            get { return (ICommand)GetValue(NumberOfPlanesMinusCommandProperty); }
            set { SetValue(NumberOfPlanesMinusCommandProperty, value); }
        }

        public ICommand NumberOfPlanesPlusommand
        {
            get { return (ICommand)GetValue(NumberOfPlanesPlusCommandProperty); }
            set { SetValue(NumberOfPlanesPlusCommandProperty, value); }
        }

        public bool ThreePhotonEnable
        {
            get { return (bool)GetValue(ThreePhotonEnableProperty); }
            set { SetValue(ThreePhotonEnableProperty, value); }
        }

        public bool EnableDownsamplingRateChange
        {
            get { return (bool)GetValue(EnableDownsamplingRateChangeProperty); }
            set { SetValue(EnableDownsamplingRateChangeProperty, value); }
        }

        public double ThreePhotonFreq
        {
            get { return (double)GetValue(ThreePhotonFreqProperty); }
            set { SetValue(ThreePhotonFreqProperty, value); }
        }

        public ICommand ThreePhotonMeasureFrequencyCommand
        {
            get { return (ICommand)GetValue(ThreePhotonMeasureFrequencyCommandProperty); }
            set { SetValue(ThreePhotonMeasureFrequencyCommandProperty, value); }
        }

        public bool ThreePhotonPanelEnable
        {
            get { return (bool)GetValue(ThreePhotonPanelEnableProperty); }
            set { SetValue(ThreePhotonPanelEnableProperty, value); }
        }

        public int ThreePhotonPhaseCoarse
        {
            get { return (int)GetValue(ThreePhotonPhaseCoarseProperty); }
            set { SetValue(ThreePhotonPhaseCoarseProperty, value); }
        }

        public ICommand ThreePhotonPhaseCoarseMinusCommand
        {
            get { return (ICommand)GetValue(ThreePhotonPhaseCoarseMinusCommandProperty); }
            set { SetValue(ThreePhotonPhaseCoarseMinusCommandProperty, value); }
        }

        public ICommand ThreePhotonPhaseCoarsePlusCommand
        {
            get { return (ICommand)GetValue(ThreePhotonPhaseCoarsePlusCommandProperty); }
            set { SetValue(ThreePhotonPhaseCoarsePlusCommandProperty, value); }
        }

        public Visibility ThreePhotonPhaseCoarseVisibility
        {
            get { return (Visibility)GetValue(ThreePhotonPhaseCoarseVisibilityProperty); }
            set { SetValue(ThreePhotonPhaseCoarseVisibilityProperty, value); }
        }

        public int ThreePhotonPhaseFine
        {
            get { return (int)GetValue(ThreePhotonPhaseFineProperty); }
            set
            {
                SetValue(ThreePhotonPhaseFineProperty, value);
            }
        }

        public ICommand ThreePhotonPhaseFineMinusCommand
        {
            get { return (ICommand)GetValue(ThreePhotonPhaseFineMinusCommandProperty); }
            set { SetValue(ThreePhotonPhaseFineMinusCommandProperty, value); }
        }

        public ICommand ThreePhotonPhaseFinePlusCommand
        {
            get { return (ICommand)GetValue(ThreePhotonPhaseFinePlusCommandProperty); }
            set { SetValue(ThreePhotonPhaseFinePlusCommandProperty, value); }
        }

        public Visibility DownsamplingRateVisibility
        {
            get { return (Visibility)GetValue(DownsamplingRateVisibilityProperty); }
            set { SetValue(DownsamplingRateVisibilityProperty, value); }
        }

        public ICommand DownsamplingRateMinusCommand
        {
            get { return (ICommand)GetValue(DownsamplingRateMinusCommandProperty); }
            set { SetValue(DownsamplingRateMinusCommandProperty, value); }
        }

        public ICommand DownsamplingRatePlusommand
        {
            get { return (ICommand)GetValue(DownsamplingRatePlusCommandProperty); }
            set { SetValue(DownsamplingRatePlusCommandProperty, value); }
        }

        public int ThreePhotonDownsamplingRate
        {
            get { return (int)GetValue(ThreePhotonDownsamplingRateProperty); }
            set { SetValue(ThreePhotonDownsamplingRateProperty, value); }
        }

        #endregion Properties
    }
}