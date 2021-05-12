namespace LightEngineControl.Controls
{
    using System;
    using System.Collections.ObjectModel;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
    using System.Windows.Input;
    using System.Windows.Media;

    /// <summary>
    /// Interaction logic for LEDSlider.xaml
    /// </summary>
    public partial class LEDSlider : UserControl
    {
        #region Fields

        /// <summary>
        /// Hide/Show the Temperature Grid Labels
        /// </summary>
        public static readonly DependencyProperty HasTemperatureProperty = DependencyProperty.RegisterAttached(
            "HasTemperature",
            typeof(Boolean),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(true,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsArrange |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure));

        /// <summary>
        /// </summary>
        public static readonly DependencyProperty IDProperty = DependencyProperty.RegisterAttached(
          "ID",
          typeof(Int32),
          typeof(LEDSlider),
          new FrameworkPropertyMetadata(0));

        /// <summary>
        /// Hide/Show the On-Off-Toggle-Slide
        /// </summary>
        public static readonly DependencyProperty IsCheckableProperty = DependencyProperty.RegisterAttached(
            "IsCheckable",
            typeof(Boolean),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(true,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsArrange |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure));

        /// <summary>
        /// Checked Dependency Property
        /// </summary>
        public static readonly DependencyProperty IsCheckedProperty = DependencyProperty.RegisterAttached(
            "IsChecked",
            typeof(Boolean),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(false,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));

        /// <summary>
        /// Decides if the Control Name becomes a Label or an Edit Field
        /// </summary>
        public static readonly DependencyProperty IsNameEditableProperty = DependencyProperty.RegisterAttached(
            "IsNameEditable",
            typeof(Boolean),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(true,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsArrange |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure));

        /// <summary>
        /// The Color of the Slider-Track and Thumb-Value-Indicator
        /// </summary>
        public static readonly DependencyProperty LightColorProperty = DependencyProperty.RegisterAttached(
            "LightColor",
            typeof(SolidColorBrush),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(Brushes.DarkGray,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure));

        /// <summary>
        /// Linear-Mode Settings Source Property
        /// </summary>
        public static readonly DependencyProperty LinearModeSettingsSourceProperty = DependencyProperty.RegisterAttached(
            "LinearModeSettingsSource",
            typeof(ObservableCollection<String>),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(new ObservableCollection<String>() { "**EMPTY**" },
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));

        /// <summary>
        /// Value Dependency Property - The Power Value of the Slider and Input Control
        /// </summary>
        //public static readonly DependencyProperty MasterValueProperty = ValueInput.ValueProperty.AddOwner(
        //    typeof(LEDSlider),
        //    new FrameworkPropertyMetadata(0.0,
        //        //FrameworkPropertyMetadataOptions.Inherits |
        //        FrameworkPropertyMetadataOptions.AffectsRender |
        //        FrameworkPropertyMetadataOptions.AffectsMeasure |
        //        FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
        //        new PropertyChangedCallback(OnMasterValueChanged)));
        public static readonly DependencyProperty MasterValueProperty = DependencyProperty.RegisterAttached(
            "MasterValue",
            typeof(Double),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(0.0,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                new PropertyChangedCallback(OnValueChanged)));
        public static readonly DependencyProperty PeakWavelengthToolTipProperty = DependencyProperty.RegisterAttached(
           "PeakWavelengthToolTip",
           typeof(String),
           typeof(LEDSlider),
           new FrameworkPropertyMetadata(string.Empty,
            //FrameworkPropertyMetadataOptions.Inherits |
               FrameworkPropertyMetadataOptions.AffectsRender |
               FrameworkPropertyMetadataOptions.AffectsMeasure |
               FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));

        /// <summary>
        /// Selected Linear-Mode Settings-Name Property
        /// </summary>
        public static readonly DependencyProperty SelectedLinearModeSettingsNameProperty = DependencyProperty.RegisterAttached(
            "SelectedLinearModeSettingsName",
            typeof(String),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata("Slot I",
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));

        /// <summary>
        /// Selected Linear-Mode Settings Property
        /// </summary>
        public static readonly DependencyProperty SelectedLinearModeSettingsProperty = DependencyProperty.RegisterAttached(
            "SelectedLinearModeSettings",
            typeof(Int32),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(0,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));

        /// <summary>
        /// Hide/Show the Temperature Labels
        /// </summary>
        public static readonly DependencyProperty ShowTemperatureProperty = DependencyProperty.RegisterAttached(
            "ShowTemperature",
            typeof(Boolean),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(true,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsArrange |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure));

        /// <summary>
        /// Hide/Show the Temperature Labels
        /// </summary>
        public static readonly DependencyProperty TemperatureProperty = DependencyProperty.RegisterAttached(
            "Temperature",
            typeof(Double),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(1.0,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure));
        public static readonly DependencyProperty TemperatureUnitProperty = DependencyProperty.RegisterAttached(
           "TemperatureUnit",
           typeof(String),
           typeof(LEDSlider),
           new FrameworkPropertyMetadata("ºC",
            //FrameworkPropertyMetadataOptions.Inherits |
               FrameworkPropertyMetadataOptions.AffectsRender |
               FrameworkPropertyMetadataOptions.AffectsMeasure));

        /// <summary>
        /// </summary>
        public static readonly DependencyProperty TextProperty = DependencyProperty.RegisterAttached(
            "Text",
            typeof(String),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata("LEDSlider",
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));

        /// <summary>
        /// Value Dependency Property - The Power Value of the Slider and Input Control
        /// </summary>
        public static readonly DependencyProperty ValueProperty = DependencyProperty.RegisterAttached(
            "Value",
            typeof(Double),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(0.0,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure |
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                new PropertyChangedCallback(OnValueChanged)));

        /// <summary>
        /// Hide/Show the Setting Controls
        /// </summary>
        private static readonly DependencyProperty IsMasterProperty = DependencyProperty.RegisterAttached(
            "IsMaster",
            typeof(Boolean),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(false,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsArrange |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure));
        private static readonly DependencyProperty LampPowerMinusCommandProperty = DependencyProperty.RegisterAttached(
            "LampPowerMinusCommand",
            typeof(ICommand),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(null,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsArrange |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure));
        private static readonly DependencyProperty LampPowerPlusCommandProperty = DependencyProperty.RegisterAttached(
            "LampPowerPlusCommand",
            typeof(ICommand),
            typeof(LEDSlider),
            new FrameworkPropertyMetadata(null,
            //FrameworkPropertyMetadataOptions.Inherits |
                FrameworkPropertyMetadataOptions.AffectsArrange |
                FrameworkPropertyMetadataOptions.AffectsRender |
                FrameworkPropertyMetadataOptions.AffectsMeasure));

        #endregion Fields

        #region Constructors

        public LEDSlider()
        {
            this.InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        /// <summary>
        /// Gets/Sets HasTemperature Property
        /// </summary>
        public Boolean HasTemperature
        {
            get
            {
                return (Boolean)this.GetValue(HasTemperatureProperty);
            }

            set
            {
                if (value != (Boolean)this.GetValue(HasTemperatureProperty))
                {
                    this.SetValue(HasTemperatureProperty, value);
                }
            }
        }

        /// <summary>
        /// </summary>
        public Int32 ID
        {
            get
            {
                return (Int32)this.GetValue(IDProperty);
            }

            set
            {
                this.SetValue(IDProperty, value);
            }
        }

        /// <summary>
        /// Gets/Sets IsCheackable Property
        /// </summary>
        public Boolean IsCheckable
        {
            get
            {
                return (Boolean)this.GetValue(IsCheckableProperty);
            }

            set
            {
                if (value != (Boolean)this.GetValue(IsCheckableProperty))
                {
                    this.SetValue(IsCheckableProperty, value);
                }
            }
        }

        /// <summary>
        /// Gets/Sets the IsChecked Property
        /// </summary>
        public Boolean IsChecked
        {
            get
            {
                return (Boolean)this.GetValue(IsCheckedProperty);
            }

            set
            {
                this.SetValue(IsCheckedProperty, value);
            }
        }

        /// <summary>
        /// Gets/Sets IsMaster Property
        /// </summary>
        public Boolean IsMaster
        {
            get
            {
                return (Boolean)this.GetValue(IsMasterProperty);
            }

            set
            {
                if (value != (Boolean)this.GetValue(IsMasterProperty))
                {
                    this.SetValue(IsMasterProperty, value);
                }
            }
        }

        /// <summary>
        /// Gets/Sets IsNameEditable Property
        /// </summary>
        public Boolean IsNameEditable
        {
            get
            {
                return (Boolean)this.GetValue(IsNameEditableProperty);
            }

            set
            {
                if (value != (Boolean)this.GetValue(IsNameEditableProperty))
                {
                    this.SetValue(IsNameEditableProperty, value);
                }
            }
        }

        /// <summary>
        /// Gets/Sets LampPowerMinusCommand Property
        /// </summary>
        public ICommand LampPowerMinusCommand
        {
            get
            {
                return (ICommand)this.GetValue(LampPowerMinusCommandProperty);
            }

            set
            {
                if (value != (ICommand)this.GetValue(LampPowerMinusCommandProperty))
                {
                    this.SetValue(LampPowerMinusCommandProperty, value);
                }
            }
        }

        /// <summary>
        /// Gets/Sets LampPowerPlusCommand Property
        /// </summary>
        public ICommand LampPowerPlusCommand
        {
            get
            {
                return (ICommand)this.GetValue(LampPowerPlusCommandProperty);
            }

            set
            {
                if (value != (ICommand)this.GetValue(LampPowerPlusCommandProperty))
                {
                    this.SetValue(LampPowerPlusCommandProperty, value);
                }
            }
        }

        /// <summary>
        /// Gets/Sets LightColor Property
        /// </summary>
        public SolidColorBrush LightColor
        {
            get
            {
                return this.GetValue(LightColorProperty) as SolidColorBrush;
            }

            set
            {
                if (value != this.GetValue(LightColorProperty) as SolidColorBrush)
                {
                    this.SetValue(LightColorProperty, value);
                }
            }
        }

        /// <summary>
        /// Gets/Sets the Linear-Mode Settings Source Property
        /// </summary>
        public ObservableCollection<String> LinearModeSettingsSource
        {
            get
            {
                return this.GetValue(LinearModeSettingsSourceProperty) as ObservableCollection<String>;
            }

            set
            {
                this.SetValue(LinearModeSettingsSourceProperty, value);
            }
        }

        /// <summary>
        /// </summary>
        public Double MasterValue
        {
            get
            {
                return (Double)this.GetValue(MasterValueProperty);
            }

            set
            {
                if (!value.Equals((Double)this.GetValue(MasterValueProperty)))
                {
                    this.SetValue(MasterValueProperty, value);
                }
            }
        }

        /// <summary>
        /// Gets string to be used as Tooltip to display the peak wavelength
        /// </summary>
        public String PeakWavelengthToolTip
        {
            get
            {
                return (String)this.GetValue(PeakWavelengthToolTipProperty);
            }

            set
            {
                this.SetValue(PeakWavelengthToolTipProperty, value);
            }
        }

        /// <summary>
        /// Gets/Sets the Selected Linear-Mode Settings Property
        /// </summary>
        public Int32 SelectedLinearModeSettings
        {
            get
            {
                return (Int32)this.GetValue(SelectedLinearModeSettingsProperty);
            }

            set
            {
                if (value != (Int32)this.GetValue(SelectedLinearModeSettingsProperty))
                {
                    this.SetValue(SelectedLinearModeSettingsProperty, value);
                }
            }
        }

        /// <summary>
        /// Gets/Sets the Selected Linear-Mode Settings-Name Property
        /// </summary>
        public String SelectedLinearModeSettingsName
        {
            get
            {
                return this.GetValue(SelectedLinearModeSettingsNameProperty) as String;
            }

            set
            {
                if ((value as String).Equals(this.GetValue(SelectedLinearModeSettingsNameProperty) as String))
                {
                    this.SetValue(SelectedLinearModeSettingsNameProperty, value);
                }
            }
        }

        /// <summary>
        /// Gets/Sets HasTemperature Property
        /// </summary>
        public Boolean ShowTemperature
        {
            get
            {
                return (Boolean)this.GetValue(ShowTemperatureProperty);
            }

            set
            {
                if (value != (Boolean)this.GetValue(ShowTemperatureProperty))
                {
                    this.SetValue(ShowTemperatureProperty, value);
                }
            }
        }

        /// <summary>
        /// Gets/Sets Temperature Property
        /// </summary>
        public Double Temperature
        {
            get
            {
                return (Double)this.GetValue(TemperatureProperty);
            }

            set
            {
                if (value != (Double)this.GetValue(TemperatureProperty))
                {
                    this.SetValue(TemperatureProperty, value);
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

        /// <summary>
        /// </summary>
        public String Text
        {
            get
            {
                return (String)this.GetValue(TextProperty);
            }

            set
            {
                this.SetValue(TextProperty, value);
            }
        }

        //public static readonly DependencyProperty ValueProperty = DependencyProperty.RegisterAttached(
        //    "Value",
        //    typeof(Double),
        //    typeof(LEDSlider),
        //    new FrameworkPropertyMetadata(0.0,
        //        //FrameworkPropertyMetadataOptions.Inherits |
        //        FrameworkPropertyMetadataOptions.AffectsRender |
        //        FrameworkPropertyMetadataOptions.AffectsMeasure |
        //        FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
        //        new PropertyChangedCallback(OnValueChanged)));
        /// <summary>
        /// </summary>
        public Double Value
        {
            get
            {
                return (Double)this.GetValue(ValueProperty);
            }

            set
            {
                if (!value.Equals((Double)this.GetValue(ValueProperty)))
                {
                    this.SetValue(ValueProperty, value);
                }
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Calculates the Thumb Location when the Power Property has changed.
        /// </summary>
        /// <param name="d"><see cref="LEDSlider"/></param>
        /// <param name="e"></param>
        private static void OnMasterValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            //if (d is LEDSlider)
            //{
            //    LEDSlider led = d as LEDSlider;

            //    led.TriggerThumbLocationCalculation();
            //}
        }

        /// <summary>
        /// Calculates the Thumb Location when the Power Property has changed.
        /// </summary>
        /// <param name="d"><see cref="LEDSlider"/></param>
        /// <param name="e"></param>
        private static void OnValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            //if (d is LEDSlider)
            //{
            //    LEDSlider led = d as LEDSlider;

            //    led.TriggerThumbLocationCalculation();
            //}
        }

        private void INPUT_CONTROLS_KeyUp(Object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                // Remove the Focus to execute 'UpdateSourceTrigger = LostFocus'
                var scope = FocusManager.GetFocusScope(this);
                FocusManager.SetFocusedElement(scope, null);
                Keyboard.ClearFocus();

                // needed if 'UpdateSourceTrigger = Explicit'
                //BindingExpression bindingExpression =
                //    BindingOperations.GetBindingExpression(sender as XamNumericInput, XamNumericInput.ValueProperty);
                //bindingExpression.UpdateSource();
            }
        }

        private void NAME_PART_DoubleClick(Object sender, MouseButtonEventArgs e)
        {
            (sender as TextBox).SelectAll();
        }

        private void NAME_PART_GotFocus(Object sender, RoutedEventArgs e)
        {
            (sender as TextBox).SelectAll();
        }

        private void NAME_PART_KeyUp(Object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                // Remove the Focus to execute 'UpdateSourceTrigger = LostFocus'
                var scope = FocusManager.GetFocusScope(this);
                FocusManager.SetFocusedElement(scope, null);
                Keyboard.ClearFocus();

                // needed if 'UpdateSourceTrigger = Explicit'
                //BindingExpression bindingExpression =
                //    BindingOperations.GetBindingExpression(sender as TextBox, TextBox.TextProperty);
                //bindingExpression.UpdateSource();
            }
        }

        private void SLIDER_PART_DragCompleted(Object sender, DragCompletedEventArgs e)
        {
            //if (!this.IsMaster)
            //{
            //}
            var scope = FocusManager.GetFocusScope(this);
            FocusManager.SetFocusedElement(scope, null);
            Keyboard.ClearFocus();

            //BindingExpression bindingExpression =
            //        BindingOperations.GetBindingExpression(sender as XamNumericSlider, XamNumericSlider.ValueProperty);
            //bindingExpression.UpdateSource();
        }

        private void SWITCH_PART_Click(Object sender, RoutedEventArgs e)
        {
        }

        #endregion Methods

        #region Other

        //BackgroundWorker updateWorker = new BackgroundWorker() { WorkerSupportsCancellation = true };
        //bool restartUpdateWorker = false;
        //int thumbPositionCalculationDelayMS = 50;
        //private bool isFirstLoad = true;
        /// <summary>
        /// Thumb Location Property
        /// <para>
        /// Reflects the Top-Right-Corner of the Thumb within the <see cref="LEDSlider"/> Coordinate System.
        /// </para>
        /// </summary>
        //public static readonly DependencyProperty ThumbLocationProperty = DependencyProperty.RegisterAttached(
        //    "ThumbLocation",
        //    typeof(Point),
        //    typeof(LEDSlider),
        //    new FrameworkPropertyMetadata(new Point(0, 0),
        //        //FrameworkPropertyMetadataOptions.Inherits |
        //        FrameworkPropertyMetadataOptions.AffectsRender |
        //        FrameworkPropertyMetadataOptions.AffectsMeasure));
        /// <summary>
        /// Gets/Sets the Thumb Location Property
        /// </summary>
        /// <value>Top Right Corner</value>
        //public Point ThumbLocation
        //{
        //    get
        //    {
        //        return (Point)this.GetValue(ThumbLocationProperty);
        //    }
        //    set
        //    {
        //        if (value != (Point)this.GetValue(ThumbLocationProperty))
        //        {
        //            this.SetValue(ThumbLocationProperty, value);
        //        }
        //    }
        //}
        //private void CalcThumbPosition()
        //{
        //    Point tempPoint = this.SLIDER_PART.Thumb.TranslatePoint(new Point(0, this.SLIDER_PART.Thumb.ActualHeight * (-1.0)), this);
        //    tempPoint.X += this.SLIDER_PART.Thumb.ActualWidth;
        //    ThumbLocation = tempPoint;
        //}

        #endregion Other
    }
}