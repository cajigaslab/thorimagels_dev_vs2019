namespace RunSampleLSDll.ViewModel
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
    /// Interaction logic for ActiveStatsWinChooser.xaml
    /// </summary>
    public partial class ActiveStatsWinChooser : Window
    {
        #region Constructors

        public ActiveStatsWinChooser(bool showChart = true, bool showTable = true, bool showProfile = true)
        {
            InitializeComponent();
            this.Topmost = true;
            ROIStatsChart.Visibility = (showChart) ? Visibility.Visible : Visibility.Collapsed;
            ROIStatsTable.Visibility = (showTable) ? Visibility.Visible : Visibility.Collapsed;
            LineProfile.Visibility = (showProfile) ? Visibility.Visible : Visibility.Collapsed;
        }

        #endregion Constructors
        #region Properties
        public bool IsChartChecked
        {
            get
            {
                return (bool)ROIStatsChart.IsChecked;
            }
            set
            {
                ROIStatsChart.IsChecked = value;
            }
        }

        public bool IsTableChecked
        {
            get
            {
                return (bool)ROIStatsTable.IsChecked;
            }
            set
            {
                ROIStatsTable.IsChecked = value;
            }
        }

        public bool IsLineProfileChecked
        {
            get
            {
                return (bool)LineProfile.IsChecked;
            }
            set
            {
                LineProfile.IsChecked = value;
            }
        }
        #endregion
        #region Methods

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        #endregion Methods
    }
}