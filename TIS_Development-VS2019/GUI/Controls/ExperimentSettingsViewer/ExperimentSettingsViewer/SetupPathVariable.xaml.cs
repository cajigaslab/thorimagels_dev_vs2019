namespace ExperimentSettingsViewer
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
    /// Interaction logic for SetupPathVariable.xaml
    /// </summary>
    public partial class SetupPathVariable : Window
    {
        #region Constructors

        public SetupPathVariable()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Methods

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        #endregion Methods
    }
}