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
    /// Interaction logic for CommentView.xaml
    /// </summary>
    public partial class CommentView : Window
    {
        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="CommentView"/> class.
        /// </summary>
        public CommentView()
        {
            InitializeComponent();
            this.Owner = Application.Current.MainWindow;
            this.Loaded += CommentView_Loaded;
        }

        #endregion Constructors

        #region Methods

        /// <summary>
        /// Handles the Click event of the CancelBtn control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void CancelBtn_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        /// <summary>
        /// Handles the Loaded event of the CommentView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void CommentView_Loaded(object sender, RoutedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            CommentsTextBox.Text = vm.VerticalMarkerTooltipText;
        }

        /// <summary>
        /// Handles the Click event of the OK Button control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void OKBtn_Click(object sender, RoutedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.VerticalMarkerTooltipText = CommentsTextBox.Text;
            this.Close();
        }

        /// <summary>
        /// Handles the Click event of the Time Button control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void TimeBtn_Click(object sender, RoutedEventArgs e)
        {
            CommentsTextBox.Text += DateTime.Now.ToString();
        }

        #endregion Methods
    }
}