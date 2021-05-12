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
        
        public Brush CancelButtonBackground
        {
            set
            {
                this.btnCancel.Background = value;
            }
        }

        public string CancelButtonContent
        {
            set 
            {
                this.btnCancel.Content = value;
            }
        }

        public Brush CancelButtonForeground
        {
            set
            {
                this.btnCancel.Foreground = value;
            }
        }

        public double CancelButtonHeight
        {
            set
            {
                this.btnCancel.Height = value;
            }
        }

        public Visibility CancelVisible
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

        public double CancelButtonWidth
        {
            set
            {
                this.btnCancel.Width = value;
            }
        }

        public string LoadingText
        {
            set
            {
                this.lblloading.Content = value;
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

        public Visibility ProgressVisible
        {
            set
            {
                this.pbProgress.Visibility = value;
                this.lblProgress.Visibility = value;
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
