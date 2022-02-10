namespace MiniCircuitsSwitchControl
{
    using System;
    using System.Globalization;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;

    public class BooleanToVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var flag = false;
            if (value is bool)
            {
                flag = (bool)value;
            }
            else if (value is bool?)
            {
                var nullable = (bool?)value;
                flag = nullable.GetValueOrDefault();
            }
            else if (value is int)
            {
                int temp = (int)value;
                flag = (0 == temp) ? false : true;
            }
            if (parameter != null)
            {
                if (bool.Parse((string)parameter))
                {
                    flag = !flag;
                }
            }
            if (flag)
            {
                return Visibility.Visible;
            }
            else
            {
                return Visibility.Collapsed;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var back = ((value is Visibility) && (((Visibility)value) == Visibility.Visible));
            if (parameter != null)
            {
                if ((bool)parameter)
                {
                    back = !back;
                }
            }
            return back;
        }

        #endregion Methods
    }

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class MiniCircuitsSwitchControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty A1SwitchColorProperty =
        DependencyProperty.Register(
        "A1SwitchColor",
        typeof(SolidColorBrush),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty A1SwitchLeftNameProperty =
           DependencyProperty.Register(
           "A1SwitchLeftName",
           typeof(string),
           typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty A1SwitchPositionProperty =
        DependencyProperty.Register(
        "A1SwitchPosition",
        typeof(int),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty A1SwitchRightNameProperty =
        DependencyProperty.Register(
        "A1SwitchRightName",
        typeof(string),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty A2SwitchColorProperty =
        DependencyProperty.Register(
        "A2SwitchColor",
        typeof(SolidColorBrush),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty A2SwitchLeftNameProperty =
           DependencyProperty.Register(
           "A2SwitchLeftName",
           typeof(string),
           typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty A2SwitchPositionProperty =
        DependencyProperty.Register(
        "A2SwitchPosition",
        typeof(int),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty A2SwitchRightNameProperty =
        DependencyProperty.Register(
        "A2SwitchRightName",
        typeof(string),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty B1SwitchColorProperty =
        DependencyProperty.Register(
        "B1SwitchColor",
        typeof(SolidColorBrush),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty B1SwitchLeftNameProperty =
           DependencyProperty.Register(
           "B1SwitchLeftName",
           typeof(string),
           typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty B1SwitchPositionProperty =
        DependencyProperty.Register(
        "B1SwitchPosition",
        typeof(int),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty B1SwitchRightNameProperty =
        DependencyProperty.Register(
        "B1SwitchRightName",
        typeof(string),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty B2SwitchColorProperty =
        DependencyProperty.Register(
        "B2SwitchColor",
        typeof(SolidColorBrush),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty B2SwitchLeftNameProperty =
           DependencyProperty.Register(
           "B2SwitchLeftName",
           typeof(string),
           typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty B2SwitchPositionProperty =
        DependencyProperty.Register(
        "B2SwitchPosition",
        typeof(int),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty B2SwitchRightNameProperty =
        DependencyProperty.Register(
        "B2SwitchRightName",
        typeof(string),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty C1SwitchColorProperty =
        DependencyProperty.Register(
        "C1SwitchColor",
        typeof(SolidColorBrush),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty C1SwitchLeftNameProperty =
           DependencyProperty.Register(
           "C1SwitchLeftName",
           typeof(string),
           typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty C1SwitchPositionProperty =
        DependencyProperty.Register(
        "C1SwitchPosition",
        typeof(int),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty C1SwitchRightNameProperty =
        DependencyProperty.Register(
        "C1SwitchRightName",
        typeof(string),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty C2SwitchColorProperty =
        DependencyProperty.Register(
        "C2SwitchColor",
        typeof(SolidColorBrush),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty C2SwitchLeftNameProperty =
           DependencyProperty.Register(
           "C2SwitchLeftName",
           typeof(string),
           typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty C2SwitchPositionProperty =
        DependencyProperty.Register(
        "C2SwitchPosition",
        typeof(int),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty C2SwitchRightNameProperty =
        DependencyProperty.Register(
        "C2SwitchRightName",
        typeof(string),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty D1SwitchColorProperty =
        DependencyProperty.Register(
        "D1SwitchColor",
        typeof(SolidColorBrush),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty D1SwitchLeftNameProperty =
           DependencyProperty.Register(
           "D1SwitchLeftName",
           typeof(string),
           typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty D1SwitchPositionProperty =
        DependencyProperty.Register(
        "D1SwitchPosition",
        typeof(int),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty D1SwitchRightNameProperty =
        DependencyProperty.Register(
        "D1SwitchRightName",
        typeof(string),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty D2SwitchColorProperty =
        DependencyProperty.Register(
        "D2SwitchColor",
        typeof(SolidColorBrush),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty D2SwitchLeftNameProperty =
           DependencyProperty.Register(
           "D2SwitchLeftName",
           typeof(string),
           typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty D2SwitchPositionProperty =
        DependencyProperty.Register(
        "D2SwitchPosition",
        typeof(int),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty D2SwitchRightNameProperty =
        DependencyProperty.Register(
        "D2SwitchRightName",
        typeof(string),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty ManualSwitchEnableProperty =
        DependencyProperty.Register(
        "ManualSwitchEnable",
        typeof(bool),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty SecondSwitchBoxAvailableProperty =
        DependencyProperty.Register(
        "SecondSwitchBoxAvailable",
        typeof(bool),
        typeof(MiniCircuitsSwitchControlUC));
        public static readonly DependencyProperty SwitchPositionNameChangeCommandProperty =
           DependencyProperty.Register(
           "SwitchPositionNameChangeCommand",
           typeof(ICommand),
           typeof(MiniCircuitsSwitchControlUC));

        #endregion Fields

        #region Constructors

        public MiniCircuitsSwitchControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public SolidColorBrush A1SwitchColor
        {
            get { return (SolidColorBrush)GetValue(A1SwitchColorProperty); }
            set { SetValue(A1SwitchColorProperty, value); }
        }

        public string A1SwitchLeftName
        {
            get { return (string)GetValue(A1SwitchLeftNameProperty); }
            set { SetValue(A1SwitchLeftNameProperty, value); }
        }

        public int A1SwitchPosition
        {
            get { return (int)GetValue(A1SwitchPositionProperty); }
            set { SetValue(A1SwitchPositionProperty, value); }
        }

        public string A1SwitchRightName
        {
            get { return (string)GetValue(A1SwitchRightNameProperty); }
            set { SetValue(A1SwitchRightNameProperty, value); }
        }

        public SolidColorBrush A2SwitchColor
        {
            get { return (SolidColorBrush)GetValue(A2SwitchColorProperty); }
            set { SetValue(A2SwitchColorProperty, value); }
        }

        public string A2SwitchLeftName
        {
            get { return (string)GetValue(A2SwitchLeftNameProperty); }
            set { SetValue(A2SwitchLeftNameProperty, value); }
        }

        public int A2SwitchPosition
        {
            get { return (int)GetValue(A2SwitchPositionProperty); }
            set { SetValue(A2SwitchPositionProperty, value); }
        }

        public string A2SwitchRightName
        {
            get { return (string)GetValue(A2SwitchRightNameProperty); }
            set { SetValue(A2SwitchRightNameProperty, value); }
        }

        public SolidColorBrush B1SwitchColor
        {
            get { return (SolidColorBrush)GetValue(B1SwitchColorProperty); }
            set { SetValue(B1SwitchColorProperty, value); }
        }

        public string B1SwitchLeftName
        {
            get { return (string)GetValue(B1SwitchLeftNameProperty); }
            set { SetValue(B1SwitchLeftNameProperty, value); }
        }

        public int B1SwitchPosition
        {
            get { return (int)GetValue(B1SwitchPositionProperty); }
            set { SetValue(B1SwitchPositionProperty, value); }
        }

        public string B1SwitchRightName
        {
            get { return (string)GetValue(B1SwitchRightNameProperty); }
            set { SetValue(B1SwitchRightNameProperty, value); }
        }

        public SolidColorBrush B2SwitchColor
        {
            get { return (SolidColorBrush)GetValue(B2SwitchColorProperty); }
            set { SetValue(B2SwitchColorProperty, value); }
        }

        public string B2SwitchLeftName
        {
            get { return (string)GetValue(B2SwitchLeftNameProperty); }
            set { SetValue(B2SwitchLeftNameProperty, value); }
        }

        public int B2SwitchPosition
        {
            get { return (int)GetValue(B2SwitchPositionProperty); }
            set { SetValue(B2SwitchPositionProperty, value); }
        }

        public string B2SwitchRightName
        {
            get { return (string)GetValue(B2SwitchRightNameProperty); }
            set { SetValue(B2SwitchRightNameProperty, value); }
        }

        public SolidColorBrush C1SwitchColor
        {
            get { return (SolidColorBrush)GetValue(C1SwitchColorProperty); }
            set { SetValue(C1SwitchColorProperty, value); }
        }

        public string C1SwitchLeftName
        {
            get { return (string)GetValue(C1SwitchLeftNameProperty); }
            set { SetValue(C1SwitchLeftNameProperty, value); }
        }

        public int C1SwitchPosition
        {
            get { return (int)GetValue(C1SwitchPositionProperty); }
            set { SetValue(C1SwitchPositionProperty, value); }
        }

        public string C1SwitchRightName
        {
            get { return (string)GetValue(C1SwitchRightNameProperty); }
            set { SetValue(C1SwitchRightNameProperty, value); }
        }

        public SolidColorBrush C2SwitchColor
        {
            get { return (SolidColorBrush)GetValue(C2SwitchColorProperty); }
            set { SetValue(C2SwitchColorProperty, value); }
        }

        public string C2SwitchLeftName
        {
            get { return (string)GetValue(C2SwitchLeftNameProperty); }
            set { SetValue(C2SwitchLeftNameProperty, value); }
        }

        public int C2SwitchPosition
        {
            get { return (int)GetValue(C2SwitchPositionProperty); }
            set { SetValue(C2SwitchPositionProperty, value); }
        }

        public string C2SwitchRightName
        {
            get { return (string)GetValue(C2SwitchRightNameProperty); }
            set { SetValue(C2SwitchRightNameProperty, value); }
        }

        public SolidColorBrush D1SwitchColor
        {
            get { return (SolidColorBrush)GetValue(D1SwitchColorProperty); }
            set { SetValue(D1SwitchColorProperty, value); }
        }

        public string D1SwitchLeftName
        {
            get { return (string)GetValue(D1SwitchLeftNameProperty); }
            set { SetValue(D1SwitchLeftNameProperty, value); }
        }

        public int D1SwitchPosition
        {
            get { return (int)GetValue(D1SwitchPositionProperty); }
            set { SetValue(D1SwitchPositionProperty, value); }
        }

        public string D1SwitchRightName
        {
            get { return (string)GetValue(D1SwitchRightNameProperty); }
            set { SetValue(D1SwitchRightNameProperty, value); }
        }

        public SolidColorBrush D2SwitchColor
        {
            get { return (SolidColorBrush)GetValue(D2SwitchColorProperty); }
            set { SetValue(D2SwitchColorProperty, value); }
        }

        public string D2SwitchLeftName
        {
            get { return (string)GetValue(D2SwitchLeftNameProperty); }
            set { SetValue(D2SwitchLeftNameProperty, value); }
        }

        public int D2SwitchPosition
        {
            get { return (int)GetValue(D2SwitchPositionProperty); }
            set { SetValue(D2SwitchPositionProperty, value); }
        }

        public string D2SwitchRightName
        {
            get { return (string)GetValue(D2SwitchRightNameProperty); }
            set { SetValue(D2SwitchRightNameProperty, value); }
        }

        public bool ManualSwitchEnable
        {
            get { return (bool)GetValue(ManualSwitchEnableProperty); }
            set { SetValue(ManualSwitchEnableProperty, value); }
        }

        public bool SecondSwitchBoxAvailable
        {
            get { return (bool)GetValue(SecondSwitchBoxAvailableProperty); }
            set { SetValue(SecondSwitchBoxAvailableProperty, value); }
        }

        public ICommand SwitchPositionNameChangeCommand
        {
            get { return (ICommand)GetValue(SwitchPositionNameChangeCommandProperty); }
            set { SetValue(SwitchPositionNameChangeCommandProperty, value); }
        }

        #endregion Properties
    }
}