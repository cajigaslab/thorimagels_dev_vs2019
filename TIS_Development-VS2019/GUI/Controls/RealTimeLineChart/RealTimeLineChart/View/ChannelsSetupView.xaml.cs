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
    /// Interaction logic for ChannelsSetupView.xaml
    /// </summary>
    public partial class ChannelsSetupView : UserControl
    {
        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="ChannelsSetupView"/> class.
        /// </summary>
        public ChannelsSetupView()
        {
            InitializeComponent();
            this.Loaded += new RoutedEventHandler(ChannelSetupView_Loaded);
        }

        #endregion Constructors

        #region Methods

        /// <summary>
        /// Handles the PreviewMouseDown event of the BleachSettings control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void BleachSettings_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.EditBleachSettings();
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the Bleach control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void Bleach_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.StartBleaching();
        }

        /// <summary>
        /// Handles the Loaded event of the ChannelSetupView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void ChannelSetupView_Loaded(object sender, RoutedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.LoadDocumentSettings();
            vm.CreateChartLines();
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the ConnectSetting control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void ConnectSetting_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.ConnectionSettingsOptions();
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the DisplayOption control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void DisplayOption_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.DisplaySettingsOptions();
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the Image control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void Image_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if (false == vm.IsCapturing)
            {
                vm.StartCapturing();
            }
            else
            {
                vm.StopCapturing();
            }
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the LineSetting control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void LineSetting_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.EditLinesSettings();
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the Settings control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void Settings_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.SamplingSettingsOptions();
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the ThorImageLSConnect control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void ThorImageLSConnect_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if (vm.ThorImageLSConnectionStats == true)
            {
                vm.ThorImageLSConnectionStats = false;
            }
            else
            {
                vm.ThorImageLSConnectionStats = true;
            }
        }

        #endregion Methods
    }
}