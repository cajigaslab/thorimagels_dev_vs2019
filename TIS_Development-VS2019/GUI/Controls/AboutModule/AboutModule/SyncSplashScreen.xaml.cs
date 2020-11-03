namespace AboutDll
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
    /// Interaction logic for SyncSplashScreen.xaml
    /// </summary>
    public partial class SyncSplashScreen : Window
    {
        #region Fields

        bool _closing = false;
        bool _hideScreen;

        #endregion Fields

        #region Constructors

        public SyncSplashScreen(string title)
        {
            InitializeComponent();
            RTTitle.Text = title;
            _hideScreen = false;
            this.Closing += new System.ComponentModel.CancelEventHandler(SyncSplashScreen_Closing);
            this.Deactivated += new EventHandler(SyncSplashScreen_Deactivated);
            this.ShowInTaskbar = false;
        }

        #endregion Constructors

        #region Methods

        void SyncSplashScreen_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            _closing = true;
        }

        void SyncSplashScreen_Deactivated(object sender, EventArgs e)
        {
            try
            {
                if (false == _hideScreen && false == _closing)
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

        private void SyncSplashScreen_MouseDown(object sender, MouseButtonEventArgs e)
        {
            try
            {
                if (false == _hideScreen && false == _closing)
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