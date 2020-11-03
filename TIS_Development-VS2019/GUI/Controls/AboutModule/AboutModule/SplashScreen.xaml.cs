namespace AboutDll
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Linq;
    using System.ServiceModel;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;

    /// <summary>
    /// Interaction logic for SplashScreen.xaml
    /// </summary>
    public partial class SplashScreen : Window
    {
        #region Fields

        bool _closing = false;
        bool _hides;

        #endregion Fields

        #region Constructors

        public SplashScreen(string title)
        {
            InitializeComponent();
            txtTitle.Text = title;
            _hides = false;
            this.Closing += new CancelEventHandler(SplashScreen_Closing);
            this.Deactivated += new EventHandler(SplashScreen_Deactivated);
            this.ShowInTaskbar = false;
        }

        #endregion Constructors

        #region Properties

        public bool Hides
        {
            get { return _hides; }
            set { _hides = value; }
        }

        #endregion Properties

        #region Methods

        void SplashScreen_Closing(object sender, CancelEventArgs e)
        {
            _closing = true;
        }

        void SplashScreen_Deactivated(object sender, EventArgs e)
        {
            try
            {
                if (false == _hides && false == _closing)
                {
                    _closing = true;
                    this.Close();
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void SplashScreen_MouseDown(object sender, MouseButtonEventArgs e)
        {
            try
            {
                if (false == _hides && false == _closing)
                {
                    _closing = true;
                    this.Close();
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        #endregion Methods
    }
}