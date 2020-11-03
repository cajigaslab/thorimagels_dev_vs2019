namespace SetScriptPath
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;

    /// <summary>
    /// Interaction logic for Add.xaml
    /// </summary>
    public partial class Add : Window
    {
        #region Constructors

        public Add()
        {
            InitializeComponent();

            this.DataContext = this;
        }

        #endregion Constructors

        #region Properties

        public string Alias
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            if (Alias == null || Alias == String.Empty)
            {
                MessageBox.Show("Name is empty! Please enter a valid Name.");
            }
            else
            {
                DialogResult = true;
                this.Close();
            }
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            this.Close();
        }

        #endregion Methods
    }
}