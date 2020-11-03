namespace PowerControl
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    using Microsoft.Win32;

    /// <summary>
    /// Interaction logic for PowerControlNameEdit.xaml
    /// </summary>
    public partial class PowerControlNameEditWin : Window
    {
        #region Constructors

        public PowerControlNameEditWin()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public string PowerControlName
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

        #endregion Methods
    }
}