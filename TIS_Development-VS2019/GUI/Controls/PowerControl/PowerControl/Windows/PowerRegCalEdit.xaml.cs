namespace PowerControl
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
    /// Interaction logic for PowerRegCalEdit.xaml
    /// </summary>
    public partial class PowerRegCalEditWin : Window
    {
        #region Constructors

        public PowerRegCalEditWin()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public string PowerCalName
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

        public string PowerCalOffset
        {
            get
            {
                return lblOffset.Content.ToString();
            }
            set
            {
                lblOffset.Content = value;
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

        #endregion Methods
    }
}