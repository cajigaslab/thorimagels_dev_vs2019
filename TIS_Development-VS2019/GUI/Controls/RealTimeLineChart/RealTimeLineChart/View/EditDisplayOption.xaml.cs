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
    /// Interaction logic for EditDisplayOption.xaml
    /// </summary>
    public partial class EditDisplayOption : Window
    {
        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="EditDisplayOption"/> class.
        /// </summary>
        public EditDisplayOption()
        {
            InitializeComponent();
            this.Owner = Application.Current.MainWindow;
        }

        #endregion Constructors

        #region Methods

        /// <summary>
        /// Handles the Click event of the EditDisplayOption OK Button control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnEditDisplayOptionOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        #endregion Methods
    }
}