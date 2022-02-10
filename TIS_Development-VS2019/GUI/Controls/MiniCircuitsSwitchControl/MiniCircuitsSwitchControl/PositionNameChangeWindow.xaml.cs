namespace MiniCircuitsSwitchControl
{
    using System.Windows;

    /// <summary>
    /// Interaction logic for ZoomSettings.xaml
    /// </summary>
    public partial class PositionNameChangeWin : Window
    {
        #region Constructors

        public PositionNameChangeWin()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public string SwitchPositionName
        {
            get
            {
                return tbName.Text;
            }
            set
            {
                tbName.Text = value;
            }
        }

        #endregion Properties

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