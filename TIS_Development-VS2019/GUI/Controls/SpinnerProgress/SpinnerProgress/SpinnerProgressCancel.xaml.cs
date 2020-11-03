namespace SpinnerProgress
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
    /// Interaction logic for SpinnerProgressCancel.xaml
    /// </summary>
    public partial class SpinnerProgressCancel : UserControl
    {
        #region Events

        public event EventHandler CancelSplashProgress = delegate { };

        #endregion Events

        #region Properties
        
        public Visibility CancelVisiable
        {
            set
            {
                this.btnCancel.Visibility = value;
            }
        }

        public string DisplayText
        {
            set
            {
                this.lblloading.Content = value;
            }
        }

        public Visibility ProgressVisible
        {
            set
            {
                this.pbProgress.Visibility = value;
                this.lblProgress.Visibility = value;
            }
        }

        public string ProgressText
        {
            set
            {
                this.lblProgress.Content = value;
            }
        }

        public int ProgressValue
        {
            set
            {
                this.pbProgress.Value = value;
            }
        }

        #endregion Properties

        #region Constructors

        public SpinnerProgressCancel()
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
