namespace DummyControl
{
    using System;
    using System.Collections;
    using System.Collections.ObjectModel;
    using System.Collections.Generic;
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
    public partial class DummyControlUC : UserControl
    {
        #region Fields

        #endregion Fields

        #region Constructors

        public DummyControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties


        #endregion Properties

        #region Methods


        //public static readonly DependencyProperty **REPLACE**CommandProperty =
        //      DependencyProperty.Register(
        //      "**REPLACE**Command",
        //      typeof(ICommand),
        //      typeof(DummyControlUC));

        //      public ICommand **REPLACE**
        //      {
        //          get { return (ICommand)GetValue(**REPLACE**CommandProperty); }
        //          set { SetValue(**REPLACE**CommandProperty, value); }
        //      }


        //public static readonly DependencyProperty **REPLACE**Property =
        //      DependencyProperty.Register(
        //      "**REPLACE**",
        //      typeof(int),
        //      typeof(DummyControlUC));

        //      public int **REPLACE**
        //      {
        //          get { return (int)GetValue(**REPLACE**Property); }
        //          set { SetValue(**REPLACE**Property, value); }
        //      }


        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.RegisterAttached("**REPLACE**",
        //typeof(int),
        //typeof(DummyControlUC),
        //new FrameworkPropertyMetadata(new PropertyChangedCallback(on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        //    get { return (int)GetValue(**REPLACE**Property); }
        //    set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}



        #endregion Methods
    }
}