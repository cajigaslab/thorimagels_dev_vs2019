namespace CaptureSetupDll.ViewModel
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
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    /// <summary>
    /// Interaction logic for LightPathSequenceStepNameEdit.xaml
    /// </summary>
    public partial class LightPathSequenceStepNameEdit : Window
    {
        #region Constructors

        public LightPathSequenceStepNameEdit()
        {
            InitializeComponent();
            this.Loaded += LightPathSequenceStepNameEdit_Loaded;
        }

        #endregion Constructors

        #region Properties

        public string LightPathSequenceStepName
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
            this.DialogResult = false;
            this.Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            this.Close();
        }

        void LightPathSequenceStepNameEdit_Loaded(object sender, RoutedEventArgs e)
        {
            tbName.Focus();
        }

        #endregion Methods
    }
}