namespace MultiLaserControl
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
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
    public partial class MultiLaserControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty Laser1PowerMinusCommandProperty = 
            DependencyProperty.Register(
            "Laser1PowerMinusCommand",
            typeof(ICommand),
            typeof(MultiLaserControlUC));
        public static readonly DependencyProperty Laser1PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "Laser1PowerPlusCommand",
            typeof(ICommand),
            typeof(MultiLaserControlUC));
        public static readonly DependencyProperty Laser2PowerMinusCommandProperty = 
            DependencyProperty.Register(
            "Laser2PowerMinusCommand",
            typeof(ICommand),
            typeof(MultiLaserControlUC));
        public static readonly DependencyProperty Laser2PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "Laser2PowerPlusCommand",
            typeof(ICommand),
            typeof(MultiLaserControlUC));
        public static readonly DependencyProperty Laser3PowerMinusCommandProperty = 
            DependencyProperty.Register(
            "Laser3PowerMinusCommand",
            typeof(ICommand),
            typeof(MultiLaserControlUC));
        public static readonly DependencyProperty Laser3PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "Laser3PowerPlusCommand",
            typeof(ICommand),
            typeof(MultiLaserControlUC));
        public static readonly DependencyProperty Laser4PowerMinusCommandProperty = 
            DependencyProperty.Register(
            "Laser4PowerMinusCommand",
            typeof(ICommand),
            typeof(MultiLaserControlUC));
        public static readonly DependencyProperty Laser4PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "Laser4PowerPlusCommand",
            typeof(ICommand),
            typeof(MultiLaserControlUC));

        public static DependencyProperty AllLaserVisibilityProperty = 
           DependencyProperty.Register("AllLaserVisibility",
           typeof(Visibility),
           typeof(MultiLaserControlUC));
        public static DependencyProperty EnableLaser1ContentProperty = 
            DependencyProperty.Register("EnableLaser1Content",
            typeof(string),
            typeof(MultiLaserControlUC));
        public static DependencyProperty EnableLaser2ContentProperty = 
            DependencyProperty.Register("EnableLaser2Content",
            typeof(string),
            typeof(MultiLaserControlUC));
        public static DependencyProperty EnableLaser3ContentProperty = 
            DependencyProperty.Register("EnableLaser3Content",
            typeof(string),
            typeof(MultiLaserControlUC));
        public static DependencyProperty EnableLaser4ContentProperty = 
            DependencyProperty.Register("EnableLaser4Content",
            typeof(string),
            typeof(MultiLaserControlUC));
        public static DependencyProperty EnableMultiLaserControlPanelProperty = 
            DependencyProperty.Register("EnableMultiLaserControlPanel",
            typeof(bool),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser1EnableProperty = 
            DependencyProperty.Register("Laser1Enable",
            typeof(bool),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser1MaxProperty = 
            DependencyProperty.Register("Laser1Max",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser1MinProperty = 
            DependencyProperty.Register("Laser1Min",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser1PowerProperty = 
            DependencyProperty.Register("Laser1Power",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser1WavelengthProperty = 
            DependencyProperty.Register("Laser1Wavelength",
            typeof(int),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser2EnableProperty = 
            DependencyProperty.Register("Laser2Enable",
            typeof(bool),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser2MaxProperty = 
            DependencyProperty.Register("Laser2Max",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser2MinProperty = 
            DependencyProperty.Register("Laser2Min",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser2PowerProperty = 
            DependencyProperty.Register("Laser2Power",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser2WavelengthProperty = 
            DependencyProperty.Register("Laser2Wavelength",
            typeof(int),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser3EnableProperty = 
            DependencyProperty.Register("Laser3Enable",
            typeof(bool),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser3MaxProperty = 
            DependencyProperty.Register("Laser3Max",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser3MinProperty = 
            DependencyProperty.Register("Laser3Min",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser3PowerProperty = 
            DependencyProperty.Register("Laser3Power",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser3WavelengthProperty = 
            DependencyProperty.Register("Laser3Wavelength",
            typeof(int),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser4EnableProperty = 
            DependencyProperty.Register("Laser4Enable",
            typeof(bool),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser4MaxProperty = 
            DependencyProperty.Register("Laser4Max",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser4MinProperty = 
            DependencyProperty.Register("Laser4Min",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser4PowerProperty = 
            DependencyProperty.Register("Laser4Power",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Laser4WavelengthProperty = 
            DependencyProperty.Register("Laser4Wavelength",
            typeof(int),
            typeof(MultiLaserControlUC));
        public static DependencyProperty LaserAllAnalogProperty = 
            DependencyProperty.Register("LaserAllAnalog",
            typeof(bool),
            typeof(MultiLaserControlUC));
        public static DependencyProperty LaserAllEnableProperty = 
            DependencyProperty.Register("LaserAllEnable",
            typeof(bool),
            typeof(MultiLaserControlUC));
        public static DependencyProperty LaserAllTTLProperty = 
            DependencyProperty.Register("LaserAllTTL",
            typeof(bool),
            typeof(MultiLaserControlUC));
        public static DependencyProperty MainLaserIndexProperty = 
            DependencyProperty.Register("MainLaserIndex",
            typeof(int),
            typeof(MultiLaserControlUC));
        public static DependencyProperty OriginalLaserVisibilityProperty = 
           DependencyProperty.Register("OriginalLaserVisibility",
           typeof(Visibility),
           typeof(MultiLaserControlUC));
        public static DependencyProperty Slider1MaxProperty = 
            DependencyProperty.Register("Slider1Max",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Slider1MinProperty = 
            DependencyProperty.Register("Slider1Min",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Slider2MaxProperty = 
            DependencyProperty.Register("Slider2Max",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Slider2MinProperty = 
            DependencyProperty.Register("Slider2Min",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Slider3MaxProperty = 
            DependencyProperty.Register("Slider3Max",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Slider3MinProperty = 
            DependencyProperty.Register("Slider3Min",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Slider4MaxProperty = 
            DependencyProperty.Register("Slider4Max",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty Slider4MinProperty = 
            DependencyProperty.Register("Slider4Min",
            typeof(double),
            typeof(MultiLaserControlUC));
        public static DependencyProperty SpLaser1VisibilityProperty = 
            DependencyProperty.Register("SpLaser1Visibility",
            typeof(Visibility),
            typeof(MultiLaserControlUC));
        public static DependencyProperty SpLaser2VisibilityProperty = 
            DependencyProperty.Register("SpLaser2Visibility",
            typeof(Visibility),
            typeof(MultiLaserControlUC));
        public static DependencyProperty SpLaser3VisibilityProperty = 
            DependencyProperty.Register("SpLaser3Visibility",
            typeof(Visibility),
            typeof(MultiLaserControlUC));
        public static DependencyProperty SpLaser4VisibilityProperty = 
            DependencyProperty.Register("SpLaser4Visibility",
            typeof(Visibility),
            typeof(MultiLaserControlUC));
        public static DependencyProperty SpMainLaserVisibilityProperty = 
            DependencyProperty.Register("SpMainLaserVisibility",
            typeof(Visibility),
            typeof(MultiLaserControlUC));
        public static DependencyProperty TopticaVisibilityProperty = 
           DependencyProperty.Register("TopticaVisibility",
           typeof(Visibility),
           typeof(MultiLaserControlUC));

        //These timers create a short interval before any of the enable buttons can be pressed again (allows time to get Enable state from device)
        System.Windows.Forms.Timer timer = new System.Windows.Forms.Timer();

        #endregion Fields

        #region Constructors

        public MultiLaserControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public Visibility AllLaserVisibility
        {
            get { return (Visibility)GetValue(AllLaserVisibilityProperty); }
            set { SetValue(AllLaserVisibilityProperty, value); }
        }

        public string EnableLaser1Content
        {
            get { return (string)GetValue(EnableLaser1ContentProperty); }
            set { SetValue(EnableLaser1ContentProperty, value); }
        }

        public string EnableLaser2Content
        {
            get { return (string)GetValue(EnableLaser2ContentProperty); }
            set { SetValue(EnableLaser2ContentProperty, value); }
        }

        public string EnableLaser3Content
        {
            get { return (string)GetValue(EnableLaser3ContentProperty); }
            set { SetValue(EnableLaser3ContentProperty, value); }
        }

        public string EnableLaser4Content
        {
            get { return (string)GetValue(EnableLaser4ContentProperty); }
            set { SetValue(EnableLaser4ContentProperty, value); }
        }

        public bool EnableMultiLaserControlPanel
        {
            get { return (bool)GetValue(EnableMultiLaserControlPanelProperty); }
            set { SetValue(EnableMultiLaserControlPanelProperty, value); }
        }

        public bool Laser1Enable
        {
            get { return (bool)GetValue(Laser1EnableProperty); }
            set { SetValue(Laser1EnableProperty, value); }
        }

        public double Laser1Max
        {
            get { return (double)GetValue(Laser1MaxProperty); }
            set { SetValue(Laser1MaxProperty, value); }
        }

        public double Laser1Min
        {
            get { return (double)GetValue(Laser1MinProperty); }
            set { SetValue(Laser1MinProperty, value); }
        }

        public double Laser1Power
        {
            get { return (double)GetValue(Laser1PowerProperty); }
            set { SetValue(Laser1PowerProperty, value); }
        }

        public ICommand Laser1PowerMinusCommand
        {
            get { return (ICommand)GetValue(Laser1PowerMinusCommandProperty); }
            set { SetValue(Laser1PowerMinusCommandProperty, value); }
        }

        public ICommand Laser1PowerPlusCommand
        {
            get { return (ICommand)GetValue(Laser1PowerPlusCommandProperty); }
            set { SetValue(Laser1PowerPlusCommandProperty, value); }
        }

        public int Laser1Wavelength
        {
            get { return (int)GetValue(Laser1WavelengthProperty); }
            set { SetValue(Laser1WavelengthProperty, value); }
        }

        public bool Laser2Enable
        {
            get { return (bool)GetValue(Laser2EnableProperty); }
            set { SetValue(Laser2EnableProperty, value); }
        }

        public double Laser2Max
        {
            get { return (double)GetValue(Laser2MaxProperty); }
            set { SetValue(Laser2MaxProperty, value); }
        }

        public double Laser2Min
        {
            get { return (double)GetValue(Laser2MinProperty); }
            set { SetValue(Laser2MinProperty, value); }
        }

        public double Laser2Power
        {
            get { return (double)GetValue(Laser2PowerProperty); }
            set { SetValue(Laser2PowerProperty, value); }
        }

        public ICommand Laser2PowerMinusCommand
        {
            get { return (ICommand)GetValue(Laser2PowerMinusCommandProperty); }
            set { SetValue(Laser2PowerMinusCommandProperty, value); }
        }

        public ICommand Laser2PowerPlusCommand
        {
            get { return (ICommand)GetValue(Laser2PowerPlusCommandProperty); }
            set { SetValue(Laser2PowerPlusCommandProperty, value); }
        }

        public int Laser2Wavelength
        {
            get { return (int)GetValue(Laser2WavelengthProperty); }
            set { SetValue(Laser2WavelengthProperty, value); }
        }

        public bool Laser3Enable
        {
            get { return (bool)GetValue(Laser3EnableProperty); }
            set { SetValue(Laser3EnableProperty, value); }
        }

        public double Laser3Max
        {
            get { return (double)GetValue(Laser3MaxProperty); }
            set { SetValue(Laser3MaxProperty, value); }
        }

        public double Laser3Min
        {
            get { return (double)GetValue(Laser3MinProperty); }
            set { SetValue(Laser3MinProperty, value); }
        }

        public double Laser3Power
        {
            get { return (double)GetValue(Laser3PowerProperty); }
            set { SetValue(Laser3PowerProperty, value); }
        }

        public ICommand Laser3PowerMinusCommand
        {
            get { return (ICommand)GetValue(Laser3PowerMinusCommandProperty); }
            set { SetValue(Laser3PowerMinusCommandProperty, value); }
        }

        public ICommand Laser3PowerPlusCommand
        {
            get { return (ICommand)GetValue(Laser3PowerPlusCommandProperty); }
            set { SetValue(Laser3PowerPlusCommandProperty, value); }
        }

        public int Laser3Wavelength
        {
            get { return (int)GetValue(Laser3WavelengthProperty); }
            set { SetValue(Laser3WavelengthProperty, value); }
        }

        public bool Laser4Enable
        {
            get { return (bool)GetValue(Laser4EnableProperty); }
            set { SetValue(Laser4EnableProperty, value); }
        }

        public double Laser4Max
        {
            get { return (double)GetValue(Laser4MaxProperty); }
            set { SetValue(Laser4MaxProperty, value); }
        }

        public double Laser4Min
        {
            get { return (double)GetValue(Laser4MinProperty); }
            set { SetValue(Laser4MinProperty, value); }
        }

        public double Laser4Power
        {
            get { return (double)GetValue(Laser4PowerProperty); }
            set { SetValue(Laser4PowerProperty, value); }
        }

        public ICommand Laser4PowerMinusCommand
        {
            get { return (ICommand)GetValue(Laser4PowerMinusCommandProperty); }
            set { SetValue(Laser4PowerMinusCommandProperty, value); }
        }

        public ICommand Laser4PowerPlusCommand
        {
            get { return (ICommand)GetValue(Laser4PowerPlusCommandProperty); }
            set { SetValue(Laser4PowerPlusCommandProperty, value); }
        }

        public int Laser4Wavelength
        {
            get { return (int)GetValue(Laser4WavelengthProperty); }
            set { SetValue(Laser4WavelengthProperty, value); }
        }

        public bool LaserAllAnalog
        {
            get { return (bool)GetValue(LaserAllAnalogProperty); }
            set { SetValue(LaserAllAnalogProperty, value); }
        }

        public bool LaserAllEnable
        {
            get { return (bool)GetValue(LaserAllEnableProperty); }
            set { SetValue(LaserAllEnableProperty, value); }
        }

        public bool LaserAllTTL
        {
            get { return (bool)GetValue(LaserAllTTLProperty); }
            set { SetValue(LaserAllTTLProperty, value); }
        }

        public int MainLaserIndex
        {
            get { return (int)GetValue(MainLaserIndexProperty); }
            set { SetValue(MainLaserIndexProperty, value); }
        }

        public Visibility OriginalLaserVisibility
        {
            get { return (Visibility)GetValue(OriginalLaserVisibilityProperty); }
            set { SetValue(OriginalLaserVisibilityProperty, value); }
        }

        public double Slider1Max
        {
            get { return (double)GetValue(Slider1MaxProperty); }
            set { SetValue(Slider1MaxProperty, value); }
        }

        public double Slider1Min
        {
            get { return (double)GetValue(Slider1MinProperty); }
            set { SetValue(Slider1MinProperty, value); }
        }

        public double Slider2Max
        {
            get { return (double)GetValue(Slider2MaxProperty); }
            set { SetValue(Slider2MaxProperty, value); }
        }

        public double Slider2Min
        {
            get { return (double)GetValue(Slider2MinProperty); }
            set { SetValue(Slider2MinProperty, value); }
        }

        public double Slider3Max
        {
            get { return (double)GetValue(Slider3MaxProperty); }
            set { SetValue(Slider3MaxProperty, value); }
        }

        public double Slider3Min
        {
            get { return (double)GetValue(Slider3MinProperty); }
            set { SetValue(Slider3MinProperty, value); }
        }

        public double Slider4Max
        {
            get { return (double)GetValue(Slider4MaxProperty); }
            set { SetValue(Slider4MaxProperty, value); }
        }

        public double Slider4Min
        {
            get { return (double)GetValue(Slider4MinProperty); }
            set { SetValue(Slider4MinProperty, value); }
        }

        public Visibility SpLaser1Visibility
        {
            get { return (Visibility)GetValue(SpLaser1VisibilityProperty); }
            set { SetValue(SpLaser1VisibilityProperty, value); }
        }

        public Visibility SpLaser2Visibility
        {
            get { return (Visibility)GetValue(SpLaser2VisibilityProperty); }
            set { SetValue(SpLaser2VisibilityProperty, value); }
        }

        public Visibility SpLaser3Visibility
        {
            get { return (Visibility)GetValue(SpLaser3VisibilityProperty); }
            set { SetValue(SpLaser3VisibilityProperty, value); }
        }

        public Visibility SpLaser4Visibility
        {
            get { return (Visibility)GetValue(SpLaser4VisibilityProperty); }
            set { SetValue(SpLaser4VisibilityProperty, value); }
        }

        public Visibility SpMainLaserVisibility
        {
            get { return (Visibility)GetValue(SpMainLaserVisibilityProperty); }
            set { SetValue(SpMainLaserVisibilityProperty, value); }
        }

        public Visibility TopticaVisibility
        {
            get { return (Visibility)GetValue(TopticaVisibilityProperty); }
            set { SetValue(TopticaVisibilityProperty, value); }
        }

        #endregion Properties

        #region Methods

        private void EnableLaser1_Checked(object sender, RoutedEventArgs e)
        {
            timer.Interval = 50;
            timer.Tick += timer_Tick1;
            timer.Start();
            EnableLaser1.IsEnabled = false;
        }

        private void EnableLaser2_Checked(object sender, RoutedEventArgs e)
        {
            timer.Interval = 50;
            timer.Tick += timer_Tick2;
            timer.Start();
            EnableLaser2.IsEnabled = false;
        }

        private void EnableLaser3_Checked(object sender, RoutedEventArgs e)
        {
            timer.Interval = 50;
            timer.Tick += timer_Tick3;
            timer.Start();
            EnableLaser3.IsEnabled = false;
        }

        private void EnableLaser4_Checked(object sender, RoutedEventArgs e)
        {
            timer.Interval = 50;
            timer.Tick += timer_Tick4;
            timer.Start();
            EnableLaser4.IsEnabled = false;
        }

        private void EnableLaserAll_Checked(object sender, RoutedEventArgs e)
        {
            timer.Interval = 50;
            timer.Tick += timer_TickAll;
            timer.Start();
            EnableLaserAll.IsEnabled = false;
        }

        //Sets the value in the text box to equal the value of the slider when the slider is changed (Value still not sent to laser until drag is completed)
        private void sliderLaser1_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            tbLaser1.Text = Math.Round(sliderLaser1.Value, 2).ToString();
        }

        private void sliderLaser2_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            tbLaser2.Text = Math.Round(sliderLaser2.Value, 2).ToString();
        }

        private void sliderLaser3_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            tbLaser3.Text = Math.Round(sliderLaser3.Value, 2).ToString();
        }

        private void sliderLaser4_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            tbLaser4.Text = Math.Round(sliderLaser4.Value, 2).ToString();
        }

        //Exceptions for entering a non-double character into a text box (retains previous value when focus is lost)
        private void tbLaser1MCLS_LostFocus(object sender, RoutedEventArgs e)
        {
            double number;
            if (double.TryParse(tbLaser1MCLS.Text, out number) == false && tbLaser1MCLS.Text != "0")
            {
                double laser1Power = (sliderLaser1MCLS.Value - Laser1Min) * 100 / (Laser1Max - Laser1Min);
                tbLaser1MCLS.Text = Math.Round(laser1Power, 2).ToString();
            }
        }

        private void tbLaser1_LostFocus(object sender, RoutedEventArgs e)
        {
            double number;
            if (double.TryParse(tbLaser1.Text, out number) == false && tbLaser1.Text != "0" || number > Laser1Max || number < Laser1Min)
            {
                tbLaser1.Text = Math.Round(sliderLaser1.Value, 2).ToString();
            }
        }

        private void tbLaser2MCLS_LostFocus(object sender, RoutedEventArgs e)
        {
            double number;
            if (double.TryParse(tbLaser2MCLS.Text, out number) == false && tbLaser2MCLS.Text != "0")
            {
                double laser2Power = (sliderLaser2MCLS.Value - Laser2Min) * 100 / (Laser2Max - Laser2Min);
                tbLaser2MCLS.Text = Math.Round(laser2Power, 2).ToString();
            }
        }

        private void tbLaser2_LostFocus(object sender, RoutedEventArgs e)
        {
            double number;
            if (double.TryParse(tbLaser2.Text, out number) == false && tbLaser2.Text != "0" || number > Laser2Max || number < Laser2Min)
            {
                tbLaser2.Text = Math.Round(sliderLaser2.Value, 2).ToString();
            }
        }

        private void tbLaser3MCLS_LostFocus(object sender, RoutedEventArgs e)
        {
            double number;
            if (double.TryParse(tbLaser3MCLS.Text, out number) == false && tbLaser3MCLS.Text != "0")
            {
                double laser3Power = (sliderLaser3MCLS.Value - Laser3Min) * 100 / (Laser3Max - Laser3Min);
                tbLaser3MCLS.Text = Math.Round(laser3Power, 2).ToString();
            }
        }

        private void tbLaser3_LostFocus(object sender, RoutedEventArgs e)
        {
            double number;
            if (double.TryParse(tbLaser3.Text, out number) == false && tbLaser3.Text != "0" || number > Laser3Max || number < Laser3Min)
            {
                tbLaser3.Text = Math.Round(sliderLaser3.Value, 2).ToString();
            }
        }

        private void tbLaser4MCLS_LostFocus(object sender, RoutedEventArgs e)
        {
            double number;
            if (double.TryParse(tbLaser4MCLS.Text, out number) == false && tbLaser4MCLS.Text != "0")
            {
                double laser4Power = (sliderLaser4MCLS.Value - Laser4Min) * 100 / (Laser4Max - Laser4Min);
                tbLaser4MCLS.Text = Math.Round(laser4Power, 2).ToString();
            }
        }

        private void tbLaser4_LostFocus(object sender, RoutedEventArgs e)
        {
            double number;
            if (double.TryParse(tbLaser4.Text, out number) == false && tbLaser4.Text != "0" || number > Laser4Max || number < Laser4Min)
            {
                tbLaser4.Text = Math.Round(sliderLaser4.Value, 2).ToString();
            }
        }

        void timer_Tick1(object sender, System.EventArgs e)
        {
            EnableLaser1.IsEnabled = true;
            timer.Stop();
        }

        void timer_Tick2(object sender, System.EventArgs e)
        {
            EnableLaser2.IsEnabled = true;
            timer.Stop();
        }

        void timer_Tick3(object sender, System.EventArgs e)
        {
            EnableLaser3.IsEnabled = true;
            timer.Stop();
        }

        void timer_Tick4(object sender, System.EventArgs e)
        {
            EnableLaser4.IsEnabled = true;
            timer.Stop();
        }

        void timer_TickAll(object sender, System.EventArgs e)
        {
            EnableLaserAll.IsEnabled = true;
            timer.Stop();
        }

        #endregion Methods
    }
}