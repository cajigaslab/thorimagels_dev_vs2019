namespace QuickTemplatesControl
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
    public partial class QuickTemplatesControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty ActiveQuickConfigItemCommandProperty = 
        DependencyProperty.Register(
        "ActiveQuickConfigItemCommand",
        typeof(ICommand),
        typeof(QuickTemplatesControlUC));
        public static readonly DependencyProperty DeleteQuickConfigItemCommandProperty = 
        DependencyProperty.Register(
        "DeleteQuickConfigItemCommand",
        typeof(ICommand),
        typeof(QuickTemplatesControlUC));
        public static readonly DependencyProperty OpenQuickConfigCommandProperty = 
        DependencyProperty.Register(
        "OpenQuickConfigCommand",
        typeof(ICommand),
        typeof(QuickTemplatesControlUC));
        public static readonly DependencyProperty QuickConfigCollectionProperty = 
        DependencyProperty.Register(
        "QuickConfigCollection",
        typeof(List<Object>),
        typeof(QuickTemplatesControlUC));

        #endregion Fields

        #region Constructors

        public QuickTemplatesControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public ICommand ActiveQuickConfigItemCommand
        {
            get { return (ICommand)GetValue(ActiveQuickConfigItemCommandProperty); }
            set { SetValue(ActiveQuickConfigItemCommandProperty, value); }
        }

        public ICommand DeleteQuickConfigItemCommand
        {
            get { return (ICommand)GetValue(DeleteQuickConfigItemCommandProperty); }
            set { SetValue(DeleteQuickConfigItemCommandProperty, value); }
        }

        public ICommand OpenQuickConfigCommand
        {
            get { return (ICommand)GetValue(OpenQuickConfigCommandProperty); }
            set { SetValue(OpenQuickConfigCommandProperty, value); }
        }

        public List<Object> QuickConfigCollection
        {
            get { return (List<Object>)GetValue(QuickConfigCollectionProperty); }
            set { SetValue(QuickConfigCollectionProperty, value); }
        }

        #endregion Properties

        #region Other

        //public static readonly DependencyProperty **REPLACE**CommandProperty =
        //      DependencyProperty.Register(
        //      "**REPLACE**Command",
        //      typeof(ICommand),
        //      typeof(QuickTemplatesControlUC));
        //      public ICommand **REPLACE**
        //      {
        //          get { return (ICommand)GetValue(**REPLACE**CommandProperty); }
        //          set { SetValue(**REPLACE**CommandProperty, value); }
        //      }
        //public static readonly DependencyProperty **REPLACE**Property =
        //      DependencyProperty.Register(
        //      "**REPLACE**",
        //      typeof(int),
        //      typeof(QuickTemplatesControlUC));
        //      public int **REPLACE**
        //      {
        //          get { return (int)GetValue(**REPLACE**Property); }
        //          set { SetValue(**REPLACE**Property, value); }
        //      }
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.RegisterAttached("**REPLACE**",
        //typeof(int),
        //typeof(QuickTemplatesControlUC),
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