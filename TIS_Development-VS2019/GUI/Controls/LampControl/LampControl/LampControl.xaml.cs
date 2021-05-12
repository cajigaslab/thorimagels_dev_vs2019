namespace LampControl
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

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class LampControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty ChangeLampPositionCommandProperty = 
            DependencyProperty.Register(
            "ChangeLampPositionCommand",
            typeof(ICommand),
            typeof(LampControlUC));
        public static readonly DependencyProperty SetLampPositionCommandProperty = 
            DependencyProperty.Register(
            "SetLampPositionCommand",
            typeof(ICommand),
            typeof(LampControlUC));

        public static DependencyProperty IsExternalTriggerProperty = 
            DependencyProperty.Register("IsExternalTrigger",
            typeof(bool),
            typeof(LampControlUC));
        public static DependencyProperty IsLampEnabledProperty =
            DependencyProperty.Register("IsLampEnabled",
            typeof(bool),
            typeof(LampControlUC));
        public static DependencyProperty LampMaxPositionProperty = 
            DependencyProperty.Register("LampMaxPosition",
            typeof(double),
            typeof(LampControlUC));
        public static DependencyProperty LampMinPositionProperty = 
            DependencyProperty.Register("LampMinPosition",
            typeof(double),
            typeof(LampControlUC));
        public static DependencyProperty LampONProperty = 
            DependencyProperty.Register("LampON",
            typeof(bool),
            typeof(LampControlUC));
        public static DependencyProperty LampPositionProperty = 
            DependencyProperty.Register("LampPosition",
            typeof(double),
            typeof(LampControlUC));
        public static DependencyProperty TempLampPositionProperty = 
            DependencyProperty.Register("TempLampPosition",
            typeof(double),
            typeof(LampControlUC));

        #endregion Fields

        #region Constructors

        public LampControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public ICommand ChangeLampPositionCommand
        {
            get { return (ICommand)GetValue(ChangeLampPositionCommandProperty); }
                  set { SetValue(ChangeLampPositionCommandProperty, value); }
        }

        public bool IsExternalTrigger
        {
            get { return (bool)GetValue(IsExternalTriggerProperty); }
                  set { SetValue(IsExternalTriggerProperty, value); }
        }

        public bool IsLampEnabled
        {
            get { return (bool)GetValue(IsLampEnabledProperty); }
            set { SetValue(IsLampEnabledProperty, value); }
        }

        public double LampMaxPosition
        {
            get { return (double)GetValue(LampMaxPositionProperty); }
                  set { SetValue(LampMaxPositionProperty, value); }
        }

        public double LampMinPosition
        {
            get { return (double)GetValue(LampMinPositionProperty); }
                  set { SetValue(LampMinPositionProperty, value); }
        }

        public bool LampON
        {
            get { return (bool)GetValue(LampONProperty); }
            set { SetValue(LampONProperty, value); }
        }

        public double LampPosition
        {
            get { return (double)GetValue(LampPositionProperty); }
                  set { SetValue(LampPositionProperty, value); }
        }

        public ICommand SetLampPositionCommand
        {
            get { return (ICommand)GetValue(SetLampPositionCommandProperty); }
                  set { SetValue(SetLampPositionCommandProperty, value); }
        }

        public double TempLampPosition
        {
            get { return (double)GetValue(TempLampPositionProperty); }
                  set { SetValue(TempLampPositionProperty, value); }
        }

        #endregion Properties

        #region Methods

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

        //public static readonly DependencyProperty **REPLACE**CommandProperty =
        //      DependencyProperty.Register(
        //      "**REPLACE**Command",
        //      typeof(ICommand),
        //      typeof(LampControlUC));
        //      public ICommand **REPLACE**
        //      {
        //          get { return (ICommand)GetValue(**REPLACE**CommandProperty); }
        //          set { SetValue(**REPLACE**CommandProperty, value); }
        //      }
        //public static readonly DependencyProperty **REPLACE**Property =
        //      DependencyProperty.Register(
        //      "**REPLACE**",
        //      typeof(int),
        //      typeof(LampControlUC));
        //      public int **REPLACE**
        //      {
        //          get { return (int)GetValue(**REPLACE**Property); }
        //          set { SetValue(**REPLACE**Property, value); }
        //      }
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.RegisterAttached("**REPLACE**",
        //typeof(int),
        //typeof(LampControlUC),
        //new FrameworkPropertyMetadata(new PropertyChangedCallback(on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        //    get { return (int)GetValue(**REPLACE**Property); }
        //    set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}

        #endregion Other
    }
}