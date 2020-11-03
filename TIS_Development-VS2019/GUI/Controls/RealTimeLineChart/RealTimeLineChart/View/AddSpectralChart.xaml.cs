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
    public partial class AddSpectralChart : Window
    {
        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="EditDisplayOption"/> class.
        /// </summary>
        public AddSpectralChart()
        {
            InitializeComponent();
            this.Owner = Application.Current.MainWindow;
            this.Loaded += AddSpectralChart_Loaded;
        }

        #endregion Constructors

        #region Methods

        void AddSpectralChart_Loaded(object sender, RoutedEventArgs e)
        {
            lbMsg.Content = string.Empty;
        }

        /// <summary>
        /// Handles the Click event of the EditDisplayOption OK Button control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnAddSpectralChartCancel_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        /// <summary>
        /// Handles the Click event of the EditDisplayOption OK Button control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnAddSpectralChartOK_Click(object sender, RoutedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            if (0 <= cbLine.SelectedIndex)
            {
                string spectralNode = "SpectralDomain";
                string spectralLine = "SpectralChannel";
                string spectralRoot = "/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]";
                string spectralDomain = "/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain";
                string spectralLines = "/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/SpectralChannel";
                string[] str = { string.Empty, string.Empty };

                XmlNodeList specList = vm.SettingsDoc.SelectNodes(spectralDomain);

                //create if not exist
                if (0 >= specList.Count)
                {
                    specList = vm.SettingsDoc.SelectNodes(spectralRoot);
                    RealTimeLineChartViewModel.CreateXmlNode(vm.SettingsDoc, spectralNode, specList[0]);
                }
                specList = vm.SettingsDoc.SelectNodes(spectralLines);

                //return if target channel is not enabled
                XmlNode node = null;
                XmlNodeList dataList = vm.SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/DataChannel[@enable=1] | /RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/VirtualChannel[@enable=1]");
                for (int i = 0; i < dataList.Count; i++)
                {
                    if (RealTimeLineChartViewModel.GetAttribute(dataList[i], vm.SettingsDoc, "alias", ref str[0]) && (0 == str[0].CompareTo(cbLine.SelectedItem)))
                    {
                        node = dataList[i];
                    }
                }
                if (null == node)
                {
                    lbMsg.Content = "Selected data line is not enabled.";
                    return;
                }

                //return if name is not valid
                string error = string.Empty;
                if (RealTimeLineChartViewModel.ContainsInvalidCharacters(tbName.Text, ref error))
                {
                    lbMsg.Content = "Line name is not valid with: " + error;
                    return;
                }

                //add spectral channel
                specList = vm.SettingsDoc.SelectNodes(spectralDomain);
                RealTimeLineChartViewModel.CreateXmlNode(vm.SettingsDoc, spectralLine, specList[0]);
                specList = vm.SettingsDoc.SelectNodes(spectralLines);

                RealTimeLineChartViewModel.SetAttribute(specList[specList.Count - 1], vm.SettingsDoc, "enable", "1");

                RealTimeLineChartViewModel.SetAttribute(specList[specList.Count - 1], vm.SettingsDoc, "alias", tbName.Text);

                RealTimeLineChartViewModel.SetAttribute(specList[specList.Count - 1], vm.SettingsDoc, "signalType", "7");
                RealTimeLineChartViewModel.SetAttribute(specList[specList.Count - 1], vm.SettingsDoc, "group", "/FI");

                RealTimeLineChartViewModel.GetAttribute(node, vm.SettingsDoc, "alias", ref str[0]);
                RealTimeLineChartViewModel.SetAttribute(specList[specList.Count - 1], vm.SettingsDoc, "physicalChannel", str[0]);

                RealTimeLineChartViewModel.GetAttribute(node, vm.SettingsDoc, "red", ref str[0]);
                RealTimeLineChartViewModel.SetAttribute(specList[specList.Count - 1], vm.SettingsDoc, "red", str[0]);

                RealTimeLineChartViewModel.GetAttribute(node, vm.SettingsDoc, "green", ref str[0]);
                RealTimeLineChartViewModel.SetAttribute(specList[specList.Count - 1], vm.SettingsDoc, "green", str[0]);

                RealTimeLineChartViewModel.GetAttribute(node, vm.SettingsDoc, "blue", ref str[0]);
                RealTimeLineChartViewModel.SetAttribute(specList[specList.Count - 1], vm.SettingsDoc, "blue", str[0]);

                RealTimeLineChartViewModel.SetAttribute(specList[specList.Count - 1], vm.SettingsDoc, "visible", "1");
                str[0] = (0 < tbLabel.Text.Length) ? tbLabel.Text : "arb. unit";
                RealTimeLineChartViewModel.SetAttribute(specList[specList.Count - 1], vm.SettingsDoc, "yLabel", str[0]);

                vm.SettingsDoc.Save(Constants.ThorRealTimeData.SETTINGS_FILE_NAME);
                vm.LoadDocumentSettings();
                vm.CreateFreqChartSeries();
                vm.UpdateSpectralCharts();
            }

            this.Close();
        }

        #endregion Methods
    }
}