namespace SequentialControl
{
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class SequentialControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty ApplyStepTemplateCommandProperty = 
        DependencyProperty.Register(
        "ApplyStepTemplateCommand",
        typeof(ICommand),
        typeof(SequentialControlUC));
        public static readonly DependencyProperty DeleteStepSequenceCommandProperty = 
        DependencyProperty.Register(
        "DeleteStepSequenceCommand",
        typeof(ICommand),
        typeof(SequentialControlUC));
        public static readonly DependencyProperty DeleteStepTemplateCommandProperty = 
        DependencyProperty.Register(
        "DeleteStepTemplateCommand",
        typeof(ICommand),
        typeof(SequentialControlUC));
        public static readonly DependencyProperty EnableSequentialCaptureProperty = 
        DependencyProperty.Register(
        "EnableSequentialCapture",
        typeof(bool),
        typeof(SequentialControlUC));
        public static readonly DependencyProperty IsTabletModeEnabledProperty = 
        DependencyProperty.Register(
        "IsTabletModeEnabled",
        typeof(Visibility),
        typeof(SequentialControlUC));
        public static readonly DependencyProperty PreviewSequentialCommandProperty = 
        DependencyProperty.Register(
        "PreviewSequentialCommand",
        typeof(ICommand),
        typeof(SequentialControlUC));
        public static readonly DependencyProperty SequenceLineNumberProperty = 
        DependencyProperty.Register(
        "SequenceLineNumber",
        typeof(int),
        typeof(SequentialControlUC));
        public static readonly DependencyProperty TemplateListAddCommandProperty = 
        DependencyProperty.Register(
        "TemplateListAddCommand",
        typeof(ICommand),
        typeof(SequentialControlUC));
        public static readonly DependencyProperty UpdateStepTemplateCommandProperty = 
        DependencyProperty.Register(
        "UpdateStepTemplateCommand",
        typeof(ICommand),
        typeof(SequentialControlUC));

        #endregion Fields

        #region Constructors

        public SequentialControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public ICommand ApplyStepTemplateCommand
        {
            get { return (ICommand)GetValue(ApplyStepTemplateCommandProperty); }
            set { SetValue(ApplyStepTemplateCommandProperty, value); }
        }

        public ICommand DeleteStepSequenceCommand
        {
            get { return (ICommand)GetValue(DeleteStepSequenceCommandProperty); }
            set { SetValue(DeleteStepSequenceCommandProperty, value); }
        }

        public ICommand DeleteStepTemplateCommand
        {
            get { return (ICommand)GetValue(DeleteStepTemplateCommandProperty); }
            set { SetValue(DeleteStepTemplateCommandProperty, value); }
        }

        public bool EnableSequentialCapture
        {
            get { return (bool)GetValue(EnableSequentialCaptureProperty); }
            set { SetValue(EnableSequentialCaptureProperty, value); }
        }

        public Visibility IsTabletModeEnabled
        {
            get { return (Visibility)GetValue(IsTabletModeEnabledProperty); }
            set { SetValue(IsTabletModeEnabledProperty, value); }
        }

        public ICommand PreviewSequentialCommand
        {
            get { return (ICommand)GetValue(PreviewSequentialCommandProperty); }
            set { SetValue(PreviewSequentialCommandProperty, value); }
        }

        public int SequenceLineNumber
        {
            get { return (int)GetValue(SequenceLineNumberProperty); }
            set { SetValue(SequenceLineNumberProperty, value); }
        }

        public ICommand TemplateListAddCommand
        {
            get { return (ICommand)GetValue(TemplateListAddCommandProperty); }
            set { SetValue(TemplateListAddCommandProperty, value); }
        }

        public ICommand UpdateStepTemplateCommand
        {
            get { return (ICommand)GetValue(UpdateStepTemplateCommandProperty); }
            set { SetValue(UpdateStepTemplateCommandProperty, value); }
        }

        #endregion Properties
    }
}