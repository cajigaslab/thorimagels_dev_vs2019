namespace OverlayManager
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
    /// Interaction logic for LineROIDoubleClickOptions.xaml
    /// </summary>
    public partial class LineROIOptions : Window
    {
        #region Constructors

        public LineROIOptions()
        {
            InitializeComponent();
            DataContext = this;
        }

        #endregion Constructors

        #region Properties

        public bool ShowLength
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        #endregion Methods
    }
}