namespace LightPathControl
{
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class LightPathControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty IsUprightScopeProperty =
        DependencyProperty.Register(
        "IsUprightScope",
        typeof(Visibility),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty LightPathSwitchProperty =
        DependencyProperty.Register(
        "LightPathSwitch",
        typeof(ICommand),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty GGLightPathVisibilityProperty =
        DependencyProperty.Register(
        "GGLightPathVisibility",
        typeof(Visibility),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty LightPathLabel_1Property =
           DependencyProperty.Register(
           "LightPathLabel_1",
           typeof(string),
           typeof(LightPathControlUC));
        public static readonly DependencyProperty LightPathGGDisplayOffProperty =
           DependencyProperty.Register(
           "LightPathGGDisplayOff",
           typeof(string),
           typeof(LightPathControlUC));
        public static readonly DependencyProperty LightPathGGDisplayOnProperty =
           DependencyProperty.Register(
           "LightPathGGDisplayOn",
           typeof(string),
           typeof(LightPathControlUC));
        public static readonly DependencyProperty MarginLeftInvertedButtonProperty =
        DependencyProperty.Register(
        "MarginLeftInvertedButton",
        typeof(Thickness),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty InvertedLpLeftDisplayProperty =
        DependencyProperty.Register(
        "InvertedLpLeftDisplay",
        typeof(string),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty GRLightPathVisibilityProperty =
        DependencyProperty.Register(
        "GRLightPathVisibility",
        typeof(Visibility),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty LightPathLabel_2Property =
           DependencyProperty.Register(
           "LightPathLabel_2",
           typeof(string),
           typeof(LightPathControlUC));
        public static readonly DependencyProperty LightPathGRDisplayOnProperty =
        DependencyProperty.Register(
        "LightPathGRDisplayOn",
        typeof(string),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty LightPathGRDisplayOffProperty =
        DependencyProperty.Register(
        "LightPathGRDisplayOff",
        typeof(string),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty MarginCenterInvertedButtonProperty =
        DependencyProperty.Register(
        "MarginCenterInvertedButton",
        typeof(Thickness),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty InvertedLpCenterDisplayProperty =
           DependencyProperty.Register(
           "InvertedLpCenterDisplay",
           typeof(string),
           typeof(LightPathControlUC));
        public static readonly DependencyProperty CameraLightPathVisibilityProperty =
        DependencyProperty.Register(
        "CameraLightPathVisibility",
        typeof(Visibility),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty LightPathLabel_3Property =
        DependencyProperty.Register(
        "LightPathLabel_3",
        typeof(string),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty LightPathCamDisplayOnProperty =
        DependencyProperty.Register(
        "LightPathCamDisplayOn",
        typeof(string),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty LightPathCamDisplayOffProperty =
           DependencyProperty.Register(
           "LightPathCamDisplayOff",
           typeof(string),
           typeof(LightPathControlUC));
        public static readonly DependencyProperty MarginRightInvertedButtonProperty =
        DependencyProperty.Register(
        "MarginRightInvertedButton",
        typeof(Thickness),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty InvertedLpRightDisplayProperty =
        DependencyProperty.Register(
        "InvertedLpRightDisplay",
        typeof(string),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty IsNDDAvailableProperty =
        DependencyProperty.Register(
        "IsNDDAvailable",
        typeof(bool),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty LabelNDDProperty =
           DependencyProperty.Register(
           "LabelNDD",
           typeof(string),
           typeof(LightPathControlUC));
        public static readonly DependencyProperty DisplayOnNDDProperty =
        DependencyProperty.Register(
        "DisplayOnNDD",
        typeof(string),
        typeof(LightPathControlUC));
        public static readonly DependencyProperty DisplayOffNDDProperty =
        DependencyProperty.Register(
        "DisplayOffNDD",
        typeof(string),
        typeof(LightPathControlUC));

        #endregion Fields

        #region Constructors

        public LightPathControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public Visibility IsUprightScope
        {
            get { return (Visibility)GetValue(IsUprightScopeProperty); }
            set { SetValue(IsUprightScopeProperty, value); }
        }

        public ICommand LightPathSwitch
        {
            get { return (ICommand)GetValue(LightPathSwitchProperty); }
            set { SetValue(LightPathSwitchProperty, value); }
        }

        public Visibility GGLightPathVisibility
        {
            get { return (Visibility)GetValue(GGLightPathVisibilityProperty); }
            set { SetValue(GGLightPathVisibilityProperty, value); }
        }

        public string LightPathLabel_1
        {
            get { return (string)GetValue(LightPathLabel_1Property); }
            set { SetValue(LightPathLabel_1Property, value); }
        }

        public string LightPathGGDisplayOn        {
            get { return (string)GetValue(LightPathGGDisplayOnProperty); }
            set { SetValue(LightPathGGDisplayOnProperty, value); }
        }

        public string LightPathGGDisplayOff
        {
            get { return (string)GetValue(LightPathGGDisplayOffProperty); }
            set { SetValue(LightPathGGDisplayOffProperty, value); }
        }

        public Thickness MarginLeftInvertedButton
        {
            get { return (Thickness)GetValue(MarginLeftInvertedButtonProperty); }
            set { SetValue(MarginLeftInvertedButtonProperty, value); }
        }

        public string InvertedLpLeftDisplay
        {
            get { return (string)GetValue(InvertedLpLeftDisplayProperty); }
            set { SetValue(InvertedLpLeftDisplayProperty, value); }
        }

        public Visibility GRLightPathVisibility
        {
            get { return (Visibility)GetValue(GRLightPathVisibilityProperty); }
            set { SetValue(GRLightPathVisibilityProperty, value); }
        }

        public string LightPathLabel_2
        {
            get { return (string)GetValue(LightPathLabel_2Property); }
            set { SetValue(LightPathLabel_2Property, value); }
        }

        public string LightPathGRDisplayOn
        {
            get { return (string)GetValue(LightPathGRDisplayOnProperty); }
            set { SetValue(LightPathGRDisplayOnProperty, value); }
        }

        public string LightPathGRDisplayOff
        {
            get { return (string)GetValue(LightPathGRDisplayOffProperty); }
            set { SetValue(LightPathGRDisplayOffProperty, value); }
        }

        public Thickness MarginCenterInvertedButton
        {
            get { return (Thickness)GetValue(MarginCenterInvertedButtonProperty); }
            set { SetValue(MarginCenterInvertedButtonProperty, value); }
        }

        public string InvertedLpCenterDisplay
        {
            get { return (string)GetValue(InvertedLpCenterDisplayProperty); }
            set { SetValue(InvertedLpCenterDisplayProperty, value); }
        }

        public Visibility CameraLightPathVisibility
        {
            get { return (Visibility)GetValue(CameraLightPathVisibilityProperty); }
            set { SetValue(CameraLightPathVisibilityProperty, value); }
        }

        public string LightPathLabel_3
        {
            get { return (string)GetValue(LightPathLabel_3Property); }
            set { SetValue(LightPathLabel_3Property, value); }
        }

        public string LightPathCamDisplayOn
        {
            get { return (string)GetValue(LightPathCamDisplayOnProperty); }
            set { SetValue(LightPathCamDisplayOnProperty, value); }
        }

        public string LightPathCamDisplayOff
        {
            get { return (string)GetValue(LightPathCamDisplayOffProperty); }
            set { SetValue(LightPathCamDisplayOffProperty, value); }
        }

        public Thickness MarginRightInvertedButton
        {
            get { return (Thickness)GetValue(MarginRightInvertedButtonProperty); }
            set { SetValue(MarginRightInvertedButtonProperty, value); }
        }

        public string InvertedLpRightDisplay
        {
            get { return (string)GetValue(InvertedLpRightDisplayProperty); }
            set { SetValue(InvertedLpRightDisplayProperty, value); }
        }

        public bool IsNDDAvailable
        {
            get { return (bool)GetValue(IsNDDAvailableProperty); }
            set { SetValue(IsNDDAvailableProperty, value); }
        }

        public string LabelNDD
        {
            get { return (string)GetValue(LabelNDDProperty); }
            set { SetValue(LabelNDDProperty, value); }
        }

        public string DisplayOnNDD
        {
            get { return (string)GetValue(DisplayOnNDDProperty); }
            set { SetValue(DisplayOnNDDProperty, value); }
        }

        public string DisplayOffNDD
        {
            get { return (string)GetValue(DisplayOffNDDProperty); }
            set { SetValue(DisplayOffNDDProperty, value); }
        }

        #endregion Properties
    }
}