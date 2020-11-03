namespace LightEngineControl
{
    using System;
    using System.Collections.ObjectModel;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class LightEngineControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty LED1PowerMinusCommandProperty = 
            DependencyProperty.Register(
            "LED1PowerMinusCommand",
            typeof(ICommand),
            typeof(LightEngineControlUC));
        public static readonly DependencyProperty LED1PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "LED1PowerPlusCommand",
            typeof(ICommand),
            typeof(LightEngineControlUC));
        public static readonly DependencyProperty LED2PowerMinusCommandProperty = 
           DependencyProperty.Register(
           "LED2PowerMinusCommand",
           typeof(ICommand),
           typeof(LightEngineControlUC));
        public static readonly DependencyProperty LED2PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "LED2PowerPlusCommand",
            typeof(ICommand),
            typeof(LightEngineControlUC));
        public static readonly DependencyProperty LED3PowerMinusCommandProperty = 
           DependencyProperty.Register(
           "LED3PowerMinusCommand",
           typeof(ICommand),
           typeof(LightEngineControlUC));
        public static readonly DependencyProperty LED3PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "LED3PowerPlusCommand",
            typeof(ICommand),
            typeof(LightEngineControlUC));
        public static readonly DependencyProperty LED4PowerMinusCommandProperty = 
           DependencyProperty.Register(
           "LED4PowerMinusCommand",
           typeof(ICommand),
           typeof(LightEngineControlUC));
        public static readonly DependencyProperty LED4PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "LED4PowerPlusCommand",
            typeof(ICommand),
            typeof(LightEngineControlUC));
        public static readonly DependencyProperty LED5PowerMinusCommandProperty = 
           DependencyProperty.Register(
           "LED5PowerMinusCommand",
           typeof(ICommand),
           typeof(LightEngineControlUC));
        public static readonly DependencyProperty LED5PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "LED5PowerPlusCommand",
            typeof(ICommand),
            typeof(LightEngineControlUC));
        public static readonly DependencyProperty LED6PowerMinusCommandProperty = 
           DependencyProperty.Register(
           "LED6PowerMinusCommand",
           typeof(ICommand),
           typeof(LightEngineControlUC));
        public static readonly DependencyProperty LED6PowerPlusCommandProperty = 
            DependencyProperty.Register(
            "LED6PowerPlusCommand",
            typeof(ICommand),
            typeof(LightEngineControlUC));
        public static readonly DependencyProperty MasterControlVisProperty = 
            DependencyProperty.Register(
            "MasterControlVis",
            typeof(Visibility),
            typeof(LightEngineControlUC));
        public static readonly DependencyProperty MasterPowerMinusCommandProperty = 
           DependencyProperty.Register(
           "MasterPowerMinusCommand",
           typeof(ICommand),
           typeof(LightEngineControlUC));
        public static readonly DependencyProperty MasterPowerPlusCommandProperty = 
            DependencyProperty.Register(
            "MasterPowerPlusCommand",
            typeof(ICommand),
            typeof(LightEngineControlUC));
        public static readonly DependencyProperty TemperatureUnitProperty = DependencyProperty.RegisterAttached(
           "TemperatureUnit",
           typeof(String),
           typeof(LightEngineControlUC),
           new FrameworkPropertyMetadata("ºC",
               //FrameworkPropertyMetadataOptions.Inherits |
               FrameworkPropertyMetadataOptions.AffectsRender |
               FrameworkPropertyMetadataOptions.AffectsMeasure));

        public static DependencyProperty LED1ControlNameProperty = DependencyProperty.RegisterAttached(
            "LED1ControlName",
            typeof(String),
            typeof(LightEngineControlUC),
            new FrameworkPropertyMetadata("LED 1",
                //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsArrange |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                new PropertyChangedCallback(onLED1ControlNameChanged)));
        public static DependencyProperty LED1LightColorProperty = DependencyProperty.RegisterAttached(
            "LED1LightColor",
            typeof(SolidColorBrush),
            typeof(LightEngineControlUC),
            new FrameworkPropertyMetadata(new SolidColorBrush(Colors.Red),
                //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure,
                new PropertyChangedCallback(onLED1LightColorChanged)));
        public static DependencyProperty LED1PowerProperty = DependencyProperty.RegisterAttached(
            "LED1Power",
            typeof(Double),
            typeof(LightEngineControlUC),
            new FrameworkPropertyMetadata(10.0,
                //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                new PropertyChangedCallback(onLED1PowerChanged)));
        public static DependencyProperty LED1PowerStateProperty = DependencyProperty.RegisterAttached(
            "LED1PowerState",
            typeof(Boolean),
            typeof(LightEngineControlUC),
            new FrameworkPropertyMetadata(false,
                //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                new PropertyChangedCallback(onLED1PowerStateChanged)));
        public static DependencyProperty LED1SockelIDProperty = DependencyProperty.RegisterAttached(
            "LED1SockelID",
            typeof(Int32),
            typeof(LightEngineControlUC),
            new FrameworkPropertyMetadata(0,
                new PropertyChangedCallback(onLED1SockelIDChanged)));
        public static DependencyProperty LED1TemperatureProperty = DependencyProperty.RegisterAttached(
            "LED1Temperature",
            typeof(Double),
            typeof(LightEngineControlUC),
            new FrameworkPropertyMetadata(21.0,
                //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure,
                new PropertyChangedCallback(onLED1TemperatureChanged)));
        public static DependencyProperty LED2ControlNameProperty = 
        DependencyProperty.RegisterAttached("LED2ControlName",
        typeof(String),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata("LED 2", new PropertyChangedCallback(onLED2ControlNameChanged)));
        public static DependencyProperty LED2LightColorProperty = 
        DependencyProperty.RegisterAttached("LED2LightColor",
        typeof(SolidColorBrush),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(new SolidColorBrush(Colors.Blue), new PropertyChangedCallback(onLED2LightColorChanged)));
        public static DependencyProperty LED2PowerProperty = 
        DependencyProperty.RegisterAttached("LED2Power",
        typeof(Double),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(20.0, new PropertyChangedCallback(onLED2PowerChanged)));
        public static DependencyProperty LED2PowerStateProperty = 
        DependencyProperty.RegisterAttached("LED2PowerState",
        typeof(Boolean),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(false, new PropertyChangedCallback(onLED2PowerStateChanged)));
        public static DependencyProperty LED2SockelIDProperty = 
        DependencyProperty.RegisterAttached("LED2SockelID",
        typeof(Int32),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(1, new PropertyChangedCallback(onLED2SockelIDChanged)));
        public static DependencyProperty LED2TemperatureProperty = 
        DependencyProperty.RegisterAttached("LED2Temperature",
        typeof(Double),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(22.0, new PropertyChangedCallback(onLED2TemperatureChanged)));
        public static DependencyProperty LED3ControlNameProperty = 
        DependencyProperty.RegisterAttached("LED3ControlName",
        typeof(String),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata("LED 3", new PropertyChangedCallback(onLED3ControlNameChanged)));
        public static DependencyProperty LED3LightColorProperty = 
        DependencyProperty.RegisterAttached("LED3LightColor",
        typeof(SolidColorBrush),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(new SolidColorBrush(Colors.Green), new PropertyChangedCallback(onLED3LightColorChanged)));
        public static DependencyProperty LED3PowerProperty = 
        DependencyProperty.RegisterAttached("LED3Power",
        typeof(Double),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(30.0, new PropertyChangedCallback(onLED3PowerChanged)));
        public static DependencyProperty LED3PowerStateProperty = 
        DependencyProperty.RegisterAttached("LED3PowerState",
        typeof(Boolean),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(false, new PropertyChangedCallback(onLED3PowerStateChanged)));
        public static DependencyProperty LED3SockelIDProperty = 
        DependencyProperty.RegisterAttached("LED3SockelID",
        typeof(Int32),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(2, new PropertyChangedCallback(onLED3SockelIDChanged)));
        public static DependencyProperty LED3TemperatureProperty = 
        DependencyProperty.RegisterAttached("LED3Temperature",
        typeof(Double),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(23.0, new PropertyChangedCallback(onLED3TemperatureChanged)));
        public static DependencyProperty LED4ControlNameProperty = 
        DependencyProperty.RegisterAttached("LED4ControlName",
        typeof(String),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata("LED 4", new PropertyChangedCallback(onLED4ControlNameChanged)));
        public static DependencyProperty LED4LightColorProperty = 
        DependencyProperty.RegisterAttached("LED4LightColor",
        typeof(SolidColorBrush),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(new SolidColorBrush(Colors.Purple), new PropertyChangedCallback(onLED4LightColorChanged)));
        public static DependencyProperty LED4PowerProperty = 
        DependencyProperty.RegisterAttached("LED4Power",
        typeof(Double),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(40.0, new PropertyChangedCallback(onLED4PowerChanged)));
        public static DependencyProperty LED4PowerStateProperty = 
        DependencyProperty.RegisterAttached("LED4PowerState",
        typeof(Boolean),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(false, new PropertyChangedCallback(onLED4PowerStateChanged)));
        public static DependencyProperty LED4SockelIDProperty = 
        DependencyProperty.RegisterAttached("LED4SockelID",
        typeof(Int32),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(3, new PropertyChangedCallback(onLED4SockelIDChanged)));
        public static DependencyProperty LED4TemperatureProperty = 
        DependencyProperty.RegisterAttached("LED4Temperature",
        typeof(Double),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(24.0, new PropertyChangedCallback(onLED4TemperatureChanged)));
        public static DependencyProperty LED5ControlNameProperty = 
        DependencyProperty.RegisterAttached("LED5ControlName",
        typeof(String),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata("LED 5", new PropertyChangedCallback(onLED5ControlNameChanged)));
        public static DependencyProperty LED5LightColorProperty = 
        DependencyProperty.RegisterAttached("LED5LightColor",
        typeof(SolidColorBrush),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(new SolidColorBrush(Colors.Yellow), new PropertyChangedCallback(onLED5LightColorChanged)));
        public static DependencyProperty LED5PowerProperty = 
        DependencyProperty.RegisterAttached("LED5Power",
        typeof(Double),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(50.0, new PropertyChangedCallback(onLED5PowerChanged)));
        public static DependencyProperty LED5PowerStateProperty = 
        DependencyProperty.RegisterAttached("LED5PowerState",
        typeof(Boolean),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(false, new PropertyChangedCallback(onLED5PowerStateChanged)));
        public static DependencyProperty LED5SockelIDProperty = 
        DependencyProperty.RegisterAttached("LED5SockelID",
        typeof(Int32),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(4, new PropertyChangedCallback(onLED5SockelIDChanged)));
        public static DependencyProperty LED5TemperatureProperty = 
        DependencyProperty.RegisterAttached("LED5Temperature",
        typeof(Double),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(25.0, new PropertyChangedCallback(onLED5TemperatureChanged)));
        public static DependencyProperty LED6ControlNameProperty = 
        DependencyProperty.RegisterAttached("LED6ControlName",
        typeof(String),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata("LED 6", new PropertyChangedCallback(onLED6ControlNameChanged)));
        public static DependencyProperty LED6LightColorProperty = 
        DependencyProperty.RegisterAttached("LED6LightColor",
        typeof(SolidColorBrush),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(new SolidColorBrush(Colors.BlueViolet), new PropertyChangedCallback(onLED6LightColorChanged)));
        public static DependencyProperty LED6PowerProperty = 
        DependencyProperty.RegisterAttached("LED6Power",
        typeof(Double),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(60.0, new PropertyChangedCallback(onLED6PowerChanged)));
        public static DependencyProperty LED6PowerStateProperty = 
        DependencyProperty.RegisterAttached("LED6PowerState",
        typeof(Boolean),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(false, new PropertyChangedCallback(onLED6PowerStateChanged)));
        public static DependencyProperty LED6SockelIDProperty = 
        DependencyProperty.RegisterAttached("LED6SockelID",
        typeof(Int32),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(5, new PropertyChangedCallback(onLED6SockelIDChanged)));
        public static DependencyProperty LED6TemperatureProperty = 
        DependencyProperty.RegisterAttached("LED6Temperature",
        typeof(Double),
        typeof(LightEngineControlUC),
        new FrameworkPropertyMetadata(26.0, new PropertyChangedCallback(onLED6TemperatureChanged)));
        public static DependencyProperty LinearModeSettingsSelectedIdxProperty = DependencyProperty.RegisterAttached(
            "LinearModeSettingsSelectedIdx",
            typeof(Int32),
            typeof(LightEngineControlUC),
            new FrameworkPropertyMetadata(0,
                //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                new PropertyChangedCallback(onLinearModeSettingsSelectedIdxChanged)));
        public static DependencyProperty LinearModeSettingsSourceProperty = DependencyProperty.RegisterAttached(
            "LinearModeSettingsSource",
            typeof(ObservableCollection<String>),
            typeof(LightEngineControlUC),
            new FrameworkPropertyMetadata(new ObservableCollection<String>() { "Slot I", "Slot II", "Slot III" },
                //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsArrange |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                new PropertyChangedCallback(onLinearModeSettingsSourceChanged)));
        public static DependencyProperty MasterBrightnessProperty = DependencyProperty.RegisterAttached(
            "MasterBrightness",
            typeof(Double),
            typeof(LightEngineControlUC),
            new FrameworkPropertyMetadata(100.0,
                //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                new PropertyChangedCallback(onMasterBrightnessChanged)));
        public static DependencyProperty ShowTemperaturesProperty = DependencyProperty.RegisterAttached(
            "ShowTemperatures",
            typeof(Boolean),
            typeof(LightEngineControlUC),
            new FrameworkPropertyMetadata(true,
                //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsArrange |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure,
                new PropertyChangedCallback(onShowTemperatureChanged)));

        #endregion Fields

        #region Constructors

        public LightEngineControlUC()
        {
            this.InitializeComponent();

            //Grid.SetIsSharedSizeScope(this.ControlRootLayout, true);
        }

        #endregion Constructors

        #region Properties

        public String LED1ControlName
        {
            get
            {
                return this.GetValue(LED1ControlNameProperty) as String;
            }
            set
            {
                if (!this.GetValue(LED1ControlNameProperty).Equals(value))
                {
                    this.SetValue(LED1ControlNameProperty, value);
                }
            }
        }

        public SolidColorBrush LED1LightColor
        {
            get
            {
                return this.GetValue(LED1LightColorProperty) as SolidColorBrush;
            }
            set
            {
                if (!this.GetValue(LED1LightColorProperty).Equals(value))
                {
                    this.SetValue(LED1LightColorProperty, value);
                }
            }
        }

        public Double LED1Power
        {
            get
            {
                return (Double)this.GetValue(LED1PowerProperty);
            }
            set
            {
                if (!this.GetValue(LED1PowerProperty).Equals(value))
                {
                    this.SetValue(LED1PowerProperty, value);
                }
            }
        }

        public ICommand LED1PowerMinusCommand
        {
            get { return (ICommand)GetValue(LED1PowerMinusCommandProperty); }
            set { SetValue(LED1PowerMinusCommandProperty, value); }
        }

        public ICommand LED1PowerPlusCommand
        {
            get { return (ICommand)GetValue(LED1PowerPlusCommandProperty); }
            set { SetValue(LED1PowerPlusCommandProperty, value); }
        }

        public Boolean LED1PowerState
        {
            get
            {
                return (Boolean)this.GetValue(LED1PowerStateProperty);
            }
            set
            {
                if (!this.GetValue(LED1PowerStateProperty).Equals(value))
                {
                    this.SetValue(LED1PowerStateProperty, value);
                }
            }
        }

        public Int32 LED1SockelID
        {
            get
            {
                return (Int32)this.GetValue(LED1SockelIDProperty);
            }
            set
            {
                if (!this.GetValue(LED1SockelIDProperty).Equals(value))
                {
                    this.SetValue(LED1SockelIDProperty, value);
                }
            }
        }

        public Double LED1Temperature
        {
            get
            {
                return (Double)this.GetValue(LED1TemperatureProperty);
            }
            set
            {
                if (!this.GetValue(LED1TemperatureProperty).Equals(value))
                {
                    this.SetValue(LED1TemperatureProperty, value);
                }
            }
        }

        public String LED2ControlName
        {
            get
            {
                return this.GetValue(LED2ControlNameProperty) as String;
            }
            set
            {
                if (!this.GetValue(LED2ControlNameProperty).Equals(value))
                {
                    this.SetValue(LED2ControlNameProperty, value);
                }
            }
        }

        public SolidColorBrush LED2LightColor
        {
            get
            {
                return this.GetValue(LED2LightColorProperty) as SolidColorBrush;
            }
            set
            {
                if (!this.GetValue(LED2LightColorProperty).Equals(value))
                {
                    this.SetValue(LED2LightColorProperty, value);
                }
            }
        }

        public Double LED2Power
        {
            get
            {
                return (Double)this.GetValue(LED2PowerProperty);
            }
            set
            {
                if (!this.GetValue(LED2PowerProperty).Equals(value))
                {
                    this.SetValue(LED2PowerProperty, value);
                }
            }
        }

        public ICommand LED2PowerMinusCommand
        {
            get { return (ICommand)GetValue(LED2PowerMinusCommandProperty); }
            set { SetValue(LED2PowerMinusCommandProperty, value); }
        }

        public ICommand LED2PowerPlusCommand
        {
            get { return (ICommand)GetValue(LED2PowerPlusCommandProperty); }
            set { SetValue(LED2PowerPlusCommandProperty, value); }
        }

        public Boolean LED2PowerState
        {
            get
            {
                return (Boolean)this.GetValue(LED2PowerStateProperty);
            }
            set
            {
                if (!this.GetValue(LED2PowerStateProperty).Equals(value))
                {
                    this.SetValue(LED2PowerStateProperty, value);
                }
            }
        }

        public Int32 LED2SockelID
        {
            get
            {
                return (Int32)this.GetValue(LED2SockelIDProperty);
            }
            set
            {
                if (!this.GetValue(LED2SockelIDProperty).Equals(value))
                {
                    this.SetValue(LED2SockelIDProperty, value);
                }
            }
        }

        public Double LED2Temperature
        {
            get
            {
                return (Double)this.GetValue(LED2TemperatureProperty);
            }
            set
            {
                if (!this.GetValue(LED2TemperatureProperty).Equals(value))
                {
                    this.SetValue(LED2TemperatureProperty, value);
                }
            }
        }

        public String LED3ControlName
        {
            get
            {
                return this.GetValue(LED3ControlNameProperty) as String;
            }
            set
            {
                if (!this.GetValue(LED3ControlNameProperty).Equals(value))
                {
                    this.SetValue(LED3ControlNameProperty, value);
                }
            }
        }

        public SolidColorBrush LED3LightColor
        {
            get
            {
                return this.GetValue(LED3LightColorProperty) as SolidColorBrush;
            }
            set
            {
                if (!this.GetValue(LED3LightColorProperty).Equals(value))
                {
                    this.SetValue(LED3LightColorProperty, value);
                }
            }
        }

        public Double LED3Power
        {
            get
            {
                return (Double)this.GetValue(LED3PowerProperty);
            }
            set
            {
                if (!this.GetValue(LED3PowerProperty).Equals(value))
                {
                    this.SetValue(LED3PowerProperty, value);
                }
            }
        }

        public ICommand LED3PowerMinusCommand
        {
            get { return (ICommand)GetValue(LED3PowerMinusCommandProperty); }
            set { SetValue(LED3PowerMinusCommandProperty, value); }
        }

        public ICommand LED3PowerPlusCommand
        {
            get { return (ICommand)GetValue(LED3PowerPlusCommandProperty); }
            set { SetValue(LED3PowerPlusCommandProperty, value); }
        }

        public Boolean LED3PowerState
        {
            get
            {
                return (Boolean)this.GetValue(LED3PowerStateProperty);
            }
            set
            {
                if (!this.GetValue(LED3PowerStateProperty).Equals(value))
                {
                    this.SetValue(LED3PowerStateProperty, value);
                }
            }
        }

        public Int32 LED3SockelID
        {
            get
            {
                return (Int32)this.GetValue(LED3SockelIDProperty);
            }
            set
            {
                if (!this.GetValue(LED3SockelIDProperty).Equals(value))
                {
                    this.SetValue(LED3SockelIDProperty, value);
                }
            }
        }

        public Double LED3Temperature
        {
            get
            {
                return (Double)this.GetValue(LED3TemperatureProperty);
            }
            set
            {
                if (!this.GetValue(LED3TemperatureProperty).Equals(value))
                {
                    this.SetValue(LED3TemperatureProperty, value);
                }
            }
        }

        public String LED4ControlName
        {
            get
            {
                return this.GetValue(LED4ControlNameProperty) as String;
            }
            set
            {
                if (!this.GetValue(LED4ControlNameProperty).Equals(value))
                {
                    this.SetValue(LED4ControlNameProperty, value);
                }
            }
        }

        public SolidColorBrush LED4LightColor
        {
            get
            {
                return this.GetValue(LED4LightColorProperty) as SolidColorBrush;
            }
            set
            {
                if (!this.GetValue(LED4LightColorProperty).Equals(value))
                {
                    this.SetValue(LED4LightColorProperty, value);
                }
            }
        }

        public Double LED4Power
        {
            get
            {
                return (Double)this.GetValue(LED4PowerProperty);
            }
            set
            {
                if (!this.GetValue(LED4PowerProperty).Equals(value))
                {
                    this.SetValue(LED4PowerProperty, value);
                }
            }
        }

        public ICommand LED4PowerMinusCommand
        {
            get { return (ICommand)GetValue(LED4PowerMinusCommandProperty); }
            set { SetValue(LED4PowerMinusCommandProperty, value); }
        }

        public ICommand LED4PowerPlusCommand
        {
            get { return (ICommand)GetValue(LED4PowerPlusCommandProperty); }
            set { SetValue(LED4PowerPlusCommandProperty, value); }
        }

        public Boolean LED4PowerState
        {
            get
            {
                return (Boolean)this.GetValue(LED4PowerStateProperty);
            }
            set
            {
                if (!this.GetValue(LED4PowerStateProperty).Equals(value))
                {
                    this.SetValue(LED4PowerStateProperty, value);
                }
            }
        }

        public Int32 LED4SockelID
        {
            get
            {
                return (Int32)this.GetValue(LED4SockelIDProperty);
            }
            set
            {
                if (!this.GetValue(LED4SockelIDProperty).Equals(value))
                {
                    this.SetValue(LED4SockelIDProperty, value);
                }
            }
        }

        public Double LED4Temperature
        {
            get
            {
                return (Double)this.GetValue(LED4TemperatureProperty);
            }
            set
            {
                if (!this.GetValue(LED4TemperatureProperty).Equals(value))
                {
                    this.SetValue(LED4TemperatureProperty, value);
                }
            }
        }

        public String LED5ControlName
        {
            get
            {
                return this.GetValue(LED5ControlNameProperty) as String;
            }
            set
            {
                if (!this.GetValue(LED5ControlNameProperty).Equals(value))
                {
                    this.SetValue(LED5ControlNameProperty, value);
                }
            }
        }

        public SolidColorBrush LED5LightColor
        {
            get
            {
                return this.GetValue(LED5LightColorProperty) as SolidColorBrush;
            }
            set
            {
                if (!this.GetValue(LED5LightColorProperty).Equals(value))
                {
                    this.SetValue(LED5LightColorProperty, value);
                }
            }
        }

        public Double LED5Power
        {
            get
            {
                return (Double)this.GetValue(LED5PowerProperty);
            }
            set
            {
                if (!this.GetValue(LED5PowerProperty).Equals(value))
                {
                    this.SetValue(LED5PowerProperty, value);
                }
            }
        }

        public ICommand LED5PowerMinusCommand
        {
            get { return (ICommand)GetValue(LED5PowerMinusCommandProperty); }
            set { SetValue(LED5PowerMinusCommandProperty, value); }
        }

        public ICommand LED5PowerPlusCommand
        {
            get { return (ICommand)GetValue(LED5PowerPlusCommandProperty); }
            set { SetValue(LED5PowerPlusCommandProperty, value); }
        }

        public Boolean LED5PowerState
        {
            get
            {
                return (Boolean)this.GetValue(LED5PowerStateProperty);
            }
            set
            {
                if (!this.GetValue(LED5PowerStateProperty).Equals(value))
                {
                    this.SetValue(LED5PowerStateProperty, value);
                }
            }
        }

        public Int32 LED5SockelID
        {
            get
            {
                return (Int32)this.GetValue(LED5SockelIDProperty);
            }
            set
            {
                if (!this.GetValue(LED5SockelIDProperty).Equals(value))
                {
                    this.SetValue(LED5SockelIDProperty, value);
                }
            }
        }

        public Double LED5Temperature
        {
            get
            {
                return (Double)this.GetValue(LED5TemperatureProperty);
            }
            set
            {
                if (!this.GetValue(LED5TemperatureProperty).Equals(value))
                {
                    this.SetValue(LED5TemperatureProperty, value);
                }
            }
        }

        public String LED6ControlName
        {
            get
            {
                return this.GetValue(LED6ControlNameProperty) as String;
            }
            set
            {
                if (!this.GetValue(LED6ControlNameProperty).Equals(value))
                {
                    this.SetValue(LED6ControlNameProperty, value);
                }
            }
        }

        public SolidColorBrush LED6LightColor
        {
            get
            {
                return this.GetValue(LED6LightColorProperty) as SolidColorBrush;
            }
            set
            {
                if (!this.GetValue(LED6LightColorProperty).Equals(value))
                {
                    this.SetValue(LED6LightColorProperty, value);
                }
            }
        }

        public Double LED6Power
        {
            get
            {
                return (Double)this.GetValue(LED6PowerProperty);
            }
            set
            {
                if (!this.GetValue(LED6PowerProperty).Equals(value))
                {
                    this.SetValue(LED6PowerProperty, value);
                }
            }
        }

        public ICommand LED6PowerMinusCommand
        {
            get { return (ICommand)GetValue(LED6PowerMinusCommandProperty); }
            set { SetValue(LED6PowerMinusCommandProperty, value); }
        }

        public ICommand LED6PowerPlusCommand
        {
            get { return (ICommand)GetValue(LED6PowerPlusCommandProperty); }
            set { SetValue(LED6PowerPlusCommandProperty, value); }
        }

        public Boolean LED6PowerState
        {
            get
            {
                return (Boolean)this.GetValue(LED6PowerStateProperty);
            }
            set
            {
                if (!this.GetValue(LED6PowerStateProperty).Equals(value))
                {
                    this.SetValue(LED6PowerStateProperty, value);
                }
            }
        }

        public Int32 LED6SockelID
        {
            get
            {
                return (Int32)this.GetValue(LED6SockelIDProperty);
            }
            set
            {
                if (!this.GetValue(LED6SockelIDProperty).Equals(value))
                {
                    this.SetValue(LED6SockelIDProperty, value);
                }
            }
        }

        public Double LED6Temperature
        {
            get
            {
                return (Double)this.GetValue(LED6TemperatureProperty);
            }
            set
            {
                if (!this.GetValue(LED6TemperatureProperty).Equals(value))
                {
                    this.SetValue(LED6TemperatureProperty, value);
                }
            }
        }

        public Int32 LinearModeSettingsSelectedIdx
        {
            get
            {
                return (Int32)this.GetValue(LinearModeSettingsSelectedIdxProperty);
            }
            set
            {
                if (!this.GetValue(LinearModeSettingsSelectedIdxProperty).Equals(value))
                {
                    this.SetValue(LinearModeSettingsSelectedIdxProperty, value);
                }
            }
        }

        public ObservableCollection<String> LinearModeSettingsSource
        {
            get
            {
                return (ObservableCollection<String>)this.GetValue(LinearModeSettingsSourceProperty);
            }
            set
            {
                if (!this.GetValue(LinearModeSettingsSourceProperty).Equals(value))
                {
                    this.SetValue(LinearModeSettingsSourceProperty, value);
                }
            }
        }

        public Double MasterBrightness
        {
            get
            {
                return (Double)this.GetValue(MasterBrightnessProperty);
            }
            set
            {
                if (!this.GetValue(MasterBrightnessProperty).Equals(value))
                {
                    this.SetValue(MasterBrightnessProperty, value);
                }
            }
        }

        public Visibility MasterControlVis
        {
            get { return (Visibility)GetValue(MasterControlVisProperty); }
            set { SetValue(MasterControlVisProperty, value); }
        }

        public ICommand MasterPowerMinusCommand
        {
            get { return (ICommand)GetValue(MasterPowerMinusCommandProperty); }
            set { SetValue(MasterPowerMinusCommandProperty, value); }
        }

        public ICommand MasterPowerPlusCommand
        {
            get { return (ICommand)GetValue(MasterPowerPlusCommandProperty); }
            set { SetValue(MasterPowerPlusCommandProperty, value); }
        }

        public Boolean ShowTemperatures
        {
            get
            {
                return (Boolean)this.GetValue(ShowTemperaturesProperty);
            }
            set
            {
                if (!this.GetValue(ShowTemperaturesProperty).Equals(value))
                {
                    this.SetValue(ShowTemperaturesProperty, value);
                }
            }
        }

        public String TemperatureUnit
        {
            get
            {
                return this.GetValue(TemperatureUnitProperty) as String;
            }

            set
            {
                if (!(this.GetValue(TemperatureUnitProperty) as String).Equals(value as String))
                {
                    this.SetValue(TemperatureUnitProperty, value as String);
                }
            }
        }

        #endregion Properties

        #region Methods

        public static void onLED1ControlNameChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED1LightColorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED1PowerChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED1PowerStateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED1SockelIDChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED1TemperatureChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED2ControlNameChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED2LightColorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED2PowerChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED2PowerStateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED2SockelIDChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED2TemperatureChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED3ControlNameChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED3LightColorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED3PowerChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED3PowerStateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED3SockelIDChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED3TemperatureChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED4ControlNameChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED4LightColorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED4PowerChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED4PowerStateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED4SockelIDChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED4TemperatureChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED5ControlNameChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED5LightColorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED5PowerChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED5PowerStateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED5SockelIDChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED5TemperatureChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED6ControlNameChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED6LightColorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED6PowerChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED6PowerStateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED6SockelIDChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLED6TemperatureChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLinearModeSettingsSelectedIdxChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLinearModeSettingsSourceChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onMasterBrightnessChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            // TODO: ask device for the corresponding LEDs power
        }

        public static void onShowTemperatureChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        //public static readonly DependencyProperty **REPLACE**CommandProperty =
        //      DependencyProperty.Register(
        //      "**REPLACE**Command",
        //      typeof(ICommand),
        //      typeof(LightEngineControlUC));
        //      public ICommand **REPLACE**
        //      {
        //          get { return (ICommand)GetValue(**REPLACE**CommandProperty); }
        //          set { SetValue(**REPLACE**CommandProperty, value); }
        //      }
        //public static readonly DependencyProperty **REPLACE**Property =
        //      DependencyProperty.Register(
        //      "**REPLACE**",
        //      typeof(int),
        //      typeof(LightEngineControlUC));
        //      public int **REPLACE**
        //      {
        //          get { return (int)GetValue(**REPLACE**Property); }
        //          set { SetValue(**REPLACE**Property, value); }
        //      }
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.RegisterAttached("**REPLACE**",
        //typeof(int),
        //typeof(LightEngineControlUC),
        //new FrameworkPropertyMetadata(new PropertyChangedCallback(on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        //    get { return (int)GetValue(**REPLACE**Property); }
        //    set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}
        public void LED_CTRL_ALL_Loaded(Object obj, RoutedEventArgs eventArgs)
        {
        }

        private void OnDataContextChanged(Object sender, DependencyPropertyChangedEventArgs e)
        {
            //if (e.NewValue.Equals(null))
            //{
            //    Binding selfBind = new Binding
            //    {
            //        RelativeSource = new RelativeSource()
            //        {
            //            AncestorType = typeof(LightEngineControlUC),
            //            Mode = RelativeSourceMode.FindAncestor,
            //            AncestorLevel = 1
            //        }
            //    };
            //    this.ControlRootLayout.SetBinding(DataContextProperty, selfBind);
            //}
            //else
            //{
            //    Binding dcBind = new Binding("DataContext")
            //    {
            //        RelativeSource = new RelativeSource()
            //        {
            //            AncestorType = typeof(LightEngineControlUC),
            //            Mode = RelativeSourceMode.FindAncestor,
            //            AncestorLevel = 1
            //        }
            //    };
            //    this.ControlRootLayout.SetBinding(DataContextProperty, dcBind);
            //}
        }

        #endregion Methods
    }
}