namespace ImageViewControl
{
    using System.Windows;

    using ThorLogging;

    /// <summary>
    /// Interaction logic for LUTSettings.xaml
    /// </summary>
    public partial class LUTSettings : Window
    {
        #region Constructors

        public LUTSettings()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Methods

        private void Button_OnCancel(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }

        #endregion Methods
    }
}