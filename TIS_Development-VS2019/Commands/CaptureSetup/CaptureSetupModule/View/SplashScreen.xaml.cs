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

namespace CaptureSetupDll.View
{
    /// <summary>
    /// Interaction logic for SplashScreen.xaml
    /// </summary>
    /// TODO:Remove from this project
    public partial class SplashScreen : Window
    {
        private string _displayText;
        private string _progressText;
        private int _progressVal;

        #region Events

        public event EventHandler CancelSplashProgress = delegate { };

        #endregion Events

        #region Properties

        public string DisplayText
        {
            get { return _displayText; }
            set
            {
                _displayText = value;
                this.lblloading.Content = value;
            }
        }

        public string ProgressText
        {
            get
            {
                return _progressText;
            }
            set
            {
                _progressText = value;
                this.lblProgress.Content = value;
            }
        }

        public int ProgressValue
        {
            get
            {
                return _progressVal;
            }
            set
            {
                _progressVal = value;
                this.pbProgress.Value = value;
            }
        }

        #endregion Properties

        #region Constructors

        public SplashScreen()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Methods

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            CancelSplashProgress(sender, e);
        }

        #endregion Methods

    }
}
