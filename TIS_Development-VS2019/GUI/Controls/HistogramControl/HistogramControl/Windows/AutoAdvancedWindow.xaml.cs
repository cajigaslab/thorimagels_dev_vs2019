namespace HistogramControl
{
    using System.Windows;

    public partial class AutoAdvancedWindow : Window
    {
        #region Constructors

        public AutoAdvancedWindow(ViewModel.HistogramControlViewModel histogramControlViewModel)
        {
            InitializeComponent();
            DataContext = histogramControlViewModel;
        }

        #endregion Constructors

        #region Methods

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
            Close();
        }

        #endregion Methods
    }
}