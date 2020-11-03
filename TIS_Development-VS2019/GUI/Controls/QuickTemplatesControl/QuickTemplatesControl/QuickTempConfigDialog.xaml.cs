namespace QuickTemplatesControl
{
    using System;
    using System.Collections.Generic;
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
    using System.Windows.Shapes;

    /// <summary>
    /// Interaction logic for QuickTempConfigDialog.xaml
    /// </summary>
    public partial class QuickTempConfigDialog : Window
    {
        #region Fields

        public static DependencyProperty AutoStartProperty = 
        DependencyProperty.Register(
        "AutoStart",
        typeof(bool),
        typeof(QuickTempConfigDialog));
        public static DependencyProperty CaptureModeProperty = 
        DependencyProperty.Register(
        "CaptureMode",
        typeof(ThorSharedTypes.CaptureModes),
        typeof(QuickTempConfigDialog));
        public static DependencyProperty FilePathProperty = 
        DependencyProperty.Register(
        "FilePath",
        typeof(string),
        typeof(QuickTempConfigDialog));
        public static DependencyProperty IdProperty = 
        DependencyProperty.Register(
        "Id",
        typeof(int),
        typeof(QuickTempConfigDialog));

        #endregion Fields

        #region Constructors

        public QuickTempConfigDialog(List<ThorSharedTypes.CaptureModes> pCaptureModes)
        {
            InitializeComponent();
            captureModescbBox.ItemsSource = pCaptureModes;
        }

        #endregion Constructors

        #region Properties

        public bool AutoStart
        {
            get { return (bool)GetValue(AutoStartProperty); }
            set { SetValue(AutoStartProperty, value); }
        }

        public ThorSharedTypes.CaptureModes CaptureMode
        {
            get { return (ThorSharedTypes.CaptureModes)GetValue(CaptureModeProperty); }
            set { SetValue(CaptureModeProperty, value); }
        }

        public Button expCanele
        {
            get { return this.Canele; }
        }

        public CheckBox expCbAutoStart
        {
            get { return this.cbAutoStart; }
        }

        public Button expLoadFile
        {
            get { return this.LoadFile; }
        }

        public Button expOK
        {
            get { return this.OK; }
        }

        public TextBox expTxtFilePath
        {
            get { return this.txtFilePath; }
        }

        public string FilePath
        {
            get { return (string)GetValue(FilePathProperty); }
            set { SetValue(FilePathProperty, value); }
        }

        public int Id
        {
            get { return (int)GetValue(IdProperty); }
            set { SetValue(IdProperty, value); }
        }

        #endregion Properties
    }
}