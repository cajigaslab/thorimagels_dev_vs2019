namespace MenuLSDll.ViewModel
{
    using System.Windows;
    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for HardwareConnections.xaml
    /// </summary>
    public partial class HardwareConnections : Window
    {
        #region Constructors

        public HardwareConnections()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Methods

        private void butOk_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            this.Close();
            ResourceManagerCS.BackupDirectory(ResourceManagerCS.GetMyDocumentsThorImageFolderString());

        }

        #endregion Methods
    }
}