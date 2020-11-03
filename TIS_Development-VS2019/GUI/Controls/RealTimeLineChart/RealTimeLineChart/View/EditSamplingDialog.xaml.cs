namespace RealTimeLineChart.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    /// <summary>
    /// Interaction logic for EditSamplingDialog.xaml
    /// </summary>
    public partial class EditSamplingDialog : Window
    {
        #region Constructors

        public EditSamplingDialog()
        {
            InitializeComponent();
            this.Owner = Application.Current.MainWindow;
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