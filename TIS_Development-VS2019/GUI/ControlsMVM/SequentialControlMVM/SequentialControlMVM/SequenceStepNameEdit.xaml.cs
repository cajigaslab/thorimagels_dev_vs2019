namespace SequentialControl
{
    using System.Windows;

    /// <summary>
    /// Interaction logic for SequenceStepNameEdit.xaml
    /// </summary>
    public partial class SequenceStepNameEdit : Window
    {
        #region Constructors

        public SequenceStepNameEdit()
        {
            InitializeComponent();
            Loaded += SequenceStepNameEdit_Loaded;
        }

        #endregion Constructors

        #region Properties

        public string SequenceStepName
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

        void SequenceStepNameEdit_Loaded(object sender, RoutedEventArgs e)
        {
            tbName.Focus();
        }

        #endregion Methods
    }
}