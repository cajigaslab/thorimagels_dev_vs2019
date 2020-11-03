namespace RealTimeLineChart.View
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
    /// Interaction logic for SplashProgress.xaml
    /// </summary>
    public partial class SplashProgress : Window
    {
        private bool _displayProgress = true;
        private bool _displayCancel = true;
        private string _displayText;
        private string _progressText;
        private int _progressVal;

        #region Events

        public event EventHandler CancelSplashProgress = delegate { };

        #endregion Events

        #region Properties

        public bool DisplayProgress
        {
            get {return _displayProgress;}
            set
            {
                _displayProgress = value;
                this.lblProgress.Visibility = (_displayProgress) ? Visibility.Visible : Visibility.Collapsed;
                this.pbProgress.Visibility = (_displayProgress) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public bool DisplayCancel
        {
            get { return _displayCancel; }
            set
            {
                _displayCancel = value;
                this.btnCancel.Visibility = (_displayCancel) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

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
                this.lblProgress.Content= value;
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

        public SplashProgress()
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