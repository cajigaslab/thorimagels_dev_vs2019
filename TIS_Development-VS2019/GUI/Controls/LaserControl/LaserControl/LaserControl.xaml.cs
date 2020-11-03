namespace LaserControl
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
    public partial class LaserControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty Laser1PowerMinusCommandProperty = 
            DependencyProperty.Register(
            "Laser1PowerMinusCommand",
            typeof(ICommand),
            typeof(LaserControlUC));
        public static readonly DependencyProperty Laser1PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "Laser1PowerPlusCommand",
            typeof(ICommand),
            typeof(LaserControlUC));
        public static readonly DependencyProperty Laser2PowerMinusCommandProperty = 
            DependencyProperty.Register(
            "Laser2PowerMinusCommand",
            typeof(ICommand),
            typeof(LaserControlUC));
        public static readonly DependencyProperty Laser2PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "Laser2PowerPlusCommand",
            typeof(ICommand),
            typeof(LaserControlUC));
        public static readonly DependencyProperty Laser3PowerMinusCommandProperty = 
            DependencyProperty.Register(
            "Laser3PowerMinusCommand",
            typeof(ICommand),
            typeof(LaserControlUC));
        public static readonly DependencyProperty Laser3PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "Laser3PowerPlusCommand",
            typeof(ICommand),
            typeof(LaserControlUC));
        public static readonly DependencyProperty Laser4PowerMinusCommandProperty = 
            DependencyProperty.Register(
            "Laser4PowerMinusCommand",
            typeof(ICommand),
            typeof(LaserControlUC));
        public static readonly DependencyProperty Laser4PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "Laser4PowerPlusCommand",
            typeof(ICommand),
            typeof(LaserControlUC));

        public static DependencyProperty CbLaser1ContentProperty = 
            DependencyProperty.Register("CbLaser1Content",
            typeof(string),
            typeof(LaserControlUC));
        public static DependencyProperty CbLaser2ContentProperty = 
            DependencyProperty.Register("CbLaser2Content",
            typeof(string),
            typeof(LaserControlUC));
        public static DependencyProperty CbLaser3ContentProperty = 
            DependencyProperty.Register("CbLaser3Content",
            typeof(string),
            typeof(LaserControlUC));
        public static DependencyProperty CbLaser4ContentProperty = 
            DependencyProperty.Register("CbLaser4Content",
            typeof(string),
            typeof(LaserControlUC));
        public static DependencyProperty EnableLaserControlPanelProperty = 
            DependencyProperty.Register("EnableLaserControlPanel",
            typeof(bool),
            typeof(LaserControlUC));
        public static DependencyProperty Laser1EnableProperty = 
            DependencyProperty.Register("Laser1Enable",
            typeof(bool),
            typeof(LaserControlUC));
        public static DependencyProperty Laser1MaxProperty = 
            DependencyProperty.Register("Laser1Max",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Laser1MinProperty = 
            DependencyProperty.Register("Laser1Min",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Laser1PowerProperty = 
            DependencyProperty.Register("Laser1Power",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Laser2EnableProperty = 
            DependencyProperty.Register("Laser2Enable",
            typeof(bool),
            typeof(LaserControlUC));
        public static DependencyProperty Laser2MaxProperty = 
            DependencyProperty.Register("Laser2Max",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Laser2MinProperty = 
            DependencyProperty.Register("Laser2Min",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Laser2PowerProperty = 
            DependencyProperty.Register("Laser2Power",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Laser3EnableProperty = 
            DependencyProperty.Register("Laser3Enable",
            typeof(bool),
            typeof(LaserControlUC));
        public static DependencyProperty Laser3MaxProperty = 
            DependencyProperty.Register("Laser3Max",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Laser3MinProperty = 
            DependencyProperty.Register("Laser3Min",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Laser3PowerProperty = 
            DependencyProperty.Register("Laser3Power",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Laser4EnableProperty = 
            DependencyProperty.Register("Laser4Enable",
            typeof(bool),
            typeof(LaserControlUC));
        public static DependencyProperty Laser4MaxProperty = 
            DependencyProperty.Register("Laser4Max",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Laser4MinProperty = 
            DependencyProperty.Register("Laser4Min",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Laser4PowerProperty = 
            DependencyProperty.Register("Laser4Power",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty MainLaserIndexProperty = 
            DependencyProperty.Register("MainLaserIndex",
            typeof(int),
            typeof(LaserControlUC));
        public static DependencyProperty Slider1MaxProperty = 
            DependencyProperty.Register("Slider1Max",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Slider1MinProperty = 
            DependencyProperty.Register("Slider1Min",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Slider2MaxProperty = 
            DependencyProperty.Register("Slider2Max",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Slider2MinProperty = 
            DependencyProperty.Register("Slider2Min",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Slider3MaxProperty = 
            DependencyProperty.Register("Slider3Max",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Slider3MinProperty = 
            DependencyProperty.Register("Slider3Min",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Slider4MaxProperty = 
            DependencyProperty.Register("Slider4Max",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty Slider4MinProperty = 
            DependencyProperty.Register("Slider4Min",
            typeof(double),
            typeof(LaserControlUC));
        public static DependencyProperty SpLaser1VisibilityProperty = 
            DependencyProperty.Register("SpLaser1Visibility",
            typeof(Visibility),
            typeof(LaserControlUC));
        public static DependencyProperty SpLaser2VisibilityProperty = 
            DependencyProperty.Register("SpLaser2Visibility",
            typeof(Visibility),
            typeof(LaserControlUC));
        public static DependencyProperty SpLaser3VisibilityProperty = 
            DependencyProperty.Register("SpLaser3Visibility",
            typeof(Visibility),
            typeof(LaserControlUC));
        public static DependencyProperty SpLaser4VisibilityProperty = 
            DependencyProperty.Register("SpLaser4Visibility",
            typeof(Visibility),
            typeof(LaserControlUC));
        public static DependencyProperty SpMainLaserVisibilityProperty = 
            DependencyProperty.Register("SpMainLaserVisibility",
            typeof(Visibility),
            typeof(LaserControlUC));

        #endregion Fields

        #region Constructors

        public LaserControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public string CbLaser1Content
        {
            get { return (string)GetValue(CbLaser1ContentProperty); }
            set { SetValue(CbLaser1ContentProperty, value); }
        }

        public string CbLaser2Content
        {
            get { return (string)GetValue(CbLaser2ContentProperty); }
            set { SetValue(CbLaser2ContentProperty, value); }
        }

        public string CbLaser3Content
        {
            get { return (string)GetValue(CbLaser3ContentProperty); }
            set { SetValue(CbLaser3ContentProperty, value); }
        }

        public string CbLaser4Content
        {
            get { return (string)GetValue(CbLaser4ContentProperty); }
            set { SetValue(CbLaser4ContentProperty, value); }
        }

        public bool EnableLaserControlPanel
        {
            get { return (bool)GetValue(EnableLaserControlPanelProperty); }
            set { SetValue(EnableLaserControlPanelProperty, value); }
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

        public int MainLaserIndex
        {
            get { return (int)GetValue(MainLaserIndexProperty); }
            set { SetValue(MainLaserIndexProperty, value); }
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

        #endregion Properties
    }
}