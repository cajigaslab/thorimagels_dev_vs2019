namespace RealTimeLineChart.View
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

    using RealTimeLineChart.ViewModel;

    /// <summary>
    /// Interaction logic for ChannelsReviewView.xaml
    /// </summary>
    public partial class ChannelsReviewView : UserControl
    {
        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="ChannelsReviewView"/> class.
        /// </summary>
        public ChannelsReviewView()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Methods

        /// <summary>
        /// Handles the PreviewMouseDown event of the LoadRecent control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void LoadRecent_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.LoadMostRecentFile();
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the Load control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void Load_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.LoadFile();
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the PrintScreen control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void PrintScreen_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.PrintScreen();
        }

        #endregion Methods
    }
}