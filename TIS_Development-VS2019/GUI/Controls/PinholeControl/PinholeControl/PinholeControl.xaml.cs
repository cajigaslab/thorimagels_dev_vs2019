namespace PinholeControl
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
    public partial class PinholeControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty ComboBoxItemsListProperty = 
              DependencyProperty.Register(
              "ComboBoxItemsList",
              typeof(ObservableCollection<string>),
              typeof(PinholeControlUC));
        public static readonly DependencyProperty LSMPinholeAlignmentMinusCommandProperty = 
              DependencyProperty.Register(
              "LSMPinholeAlignmentMinusCommand",
              typeof(ICommand),
              typeof(PinholeControlUC));
        public static readonly DependencyProperty LSMPinholeAlignmentPlusCommandProperty = 
              DependencyProperty.Register(
              "LSMPinholeAlignmentPlusCommand",
              typeof(ICommand),
              typeof(PinholeControlUC));
        public static readonly DependencyProperty LSMPinholeAlignmentSetCommandProperty = 
              DependencyProperty.Register(
              "LSMPinholeAlignmentSetCommand",
              typeof(ICommand),
              typeof(PinholeControlUC));
        public static readonly DependencyProperty PinholeADUsStringProperty = 
              DependencyProperty.Register(
              "PinholeADUsString",
              typeof(string),
              typeof(PinholeControlUC));
        public static readonly DependencyProperty PinholePositionProperty = 
              DependencyProperty.Register(
              "PinholePosition",
              typeof(int),
              typeof(PinholeControlUC));
        public static readonly DependencyProperty TxtPinholeAlignmentProperty = 
              DependencyProperty.Register(
              "TxtPinholeAlignment",
              typeof(string),
              typeof(PinholeControlUC));
        public static readonly DependencyProperty UpdatePinholePosTxtCommandProperty = 
              DependencyProperty.Register(
              "UpdatePinholePosTxtCommand",
              typeof(ICommand),
              typeof(PinholeControlUC));

        #endregion Fields

        #region Constructors

        public PinholeControlUC()
        {
            InitializeComponent();
            this.Loaded += new RoutedEventHandler(PinholeControlView_Loaded);
        }

        #endregion Constructors

        #region Properties

        public ObservableCollection<string> ComboBoxItemsList
        {
            get { return (ObservableCollection<string>)GetValue(ComboBoxItemsListProperty); }
            set { SetValue(ComboBoxItemsListProperty, value); }
        }

        public ICommand LSMPinholeAlignmentMinusCommand
        {
            get { return (ICommand)GetValue(LSMPinholeAlignmentMinusCommandProperty); }
                  set { SetValue(LSMPinholeAlignmentMinusCommandProperty, value); }
        }

        public ICommand LSMPinholeAlignmentPlusCommand
        {
            get { return (ICommand)GetValue(LSMPinholeAlignmentPlusCommandProperty); }
            set { SetValue(LSMPinholeAlignmentPlusCommandProperty, value); }
        }

        public ICommand LSMPinholeAlignmentSetCommand
        {
            get { return (ICommand)GetValue(LSMPinholeAlignmentSetCommandProperty); }
            set { SetValue(LSMPinholeAlignmentSetCommandProperty, value); }
        }

        public string PinholeADUsString
        {
            get { return (string)GetValue(PinholeADUsStringProperty); }
            set { SetValue(PinholeADUsStringProperty, value); }
        }

        public int PinholePosition
        {
            get { return (int)GetValue(PinholePositionProperty); }
            set { SetValue(PinholePositionProperty, value); }
        }

        public string TxtPinholeAlignment
        {
            get { return (string)GetValue(TxtPinholeAlignmentProperty); }
            set { SetValue(TxtPinholeAlignmentProperty, value); }
        }

        public ICommand UpdatePinholePosTxtCommand
        {
            get { return (ICommand)GetValue(UpdatePinholePosTxtCommandProperty); }
            set { SetValue(UpdatePinholePosTxtCommandProperty, value); }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Get value when element is changed in combobox
        /// </summary>
        ///
        /// <param name="sender">Object combobox Errors</param>
        /// <param name="e">Fired when select different elements</param>
        /// 
        /// <exception>NONE</exception>
        private void cbPinhole_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (null != UpdatePinholePosTxtCommand)
            {
                UpdatePinholePosTxtCommand.Execute(null);
            }
        }

        void PinholeControlView_Loaded(object sender, RoutedEventArgs e)
        {
            //the slider limits have changed so we must force the PinholeConverter to execute
            //by udpating the content property on the labels.
            var be = labelPinhole.GetBindingExpression(Label.ContentProperty);
            if (be != null)
            {
                be.UpdateTarget();
            }
            if (null != UpdatePinholePosTxtCommand)
            {
                UpdatePinholePosTxtCommand.Execute(null);
            }
        }

        /// <summary>
        /// Set Pinhole to specific position without Saving
        /// </summary>
        ///
        /// <param name="sender">object Textbox</param>
        /// <param name="e">Fired when lose foucs in the editbox. </param>
        ///
        /// <exception>NONE</exception>
        private void txtPinholeAlignment_LostFocus(object sender, RoutedEventArgs e)
        {
            int val = Convert.ToInt32(TxtPinholeAlignment);
            if (null != LSMPinholeAlignmentSetCommand)
            {
                LSMPinholeAlignmentSetCommand.Execute(null);
            }
        }

        #endregion Methods

        #region Other

        //public static readonly DependencyProperty **REPLACE**CommandProperty =
        //      DependencyProperty.Register(
        //      "**REPLACE**Command",
        //      typeof(ICommand),
        //      typeof(PinholeControlUC));
        //      public ICommand **REPLACE**
        //      {
        //          get { return (ICommand)GetValue(**REPLACE**CommandProperty); }
        //          set { SetValue(**REPLACE**CommandProperty, value); }
        //      }
        //public static readonly DependencyProperty **REPLACE**Property =
        //      DependencyProperty.Register(
        //      "**REPLACE**",
        //      typeof(int),
        //      typeof(PinholeControlUC));
        //      public int **REPLACE**
        //      {
        //          get { return (int)GetValue(**REPLACE**Property); }
        //          set { SetValue(**REPLACE**Property, value); }
        //      }
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.RegisterAttached("**REPLACE**",
        //typeof(int),
        //typeof(PinholeControlUC),
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