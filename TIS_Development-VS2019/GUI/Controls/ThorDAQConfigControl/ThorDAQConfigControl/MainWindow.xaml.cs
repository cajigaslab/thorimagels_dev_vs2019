using System.Windows;
using Telerik.Windows.Controls;
using System;

namespace ThorDAQConfigControl
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : RadWindow
    {
        View.MainContent mainContent;

        public MainWindow()
        {
            ApplicationTheme.Init();

            InitializeComponent();

            mainContent = new View.MainContent();
            this.MainContent.Content = mainContent;

            mainContent.ShowHideConfigureArea(true); // Show Waveform&Configuration panel by default for test
        }

        #region Header Menu
        private void RadMenuItemExit_Click(object sender, Telerik.Windows.RadRoutedEventArgs e)
        {
            this.Close();
        }

        private void RadMenuItemUpdateFirmware_Click(object sender, Telerik.Windows.RadRoutedEventArgs e)
        {
            //var window = new OptionWindow() { Owner = Application.Current.MainWindow };
            //window.ShowDialog();
        }
        private void RadMenuItemCommandLine_Click(object sender, Telerik.Windows.RadRoutedEventArgs e)
        {
            var flag = (sender as RadMenuItem).IsChecked;
            mainContent.ShowHideCommandLine(flag);
        }

        private void RadMenuItemConfigure_Click(object sender, Telerik.Windows.RadRoutedEventArgs e)
        {
            var flag = (sender as RadMenuItem).IsChecked;
            mainContent.ShowHideConfigureArea(flag);
        }

        private void AdvancedRadMenuItem_SubmenuOpened(object sender, Telerik.Windows.RadRoutedEventArgs e)
        {
            /*
            if (vm.IsAdvancedLocked)
            {
                PasswordWindow passwordWindow = new PasswordWindow { Owner = Application.Current.MainWindow };

                var result = passwordWindow.ShowDialog();
                if (result == null || !result.Value)
                    return;

                e.Handled = true;
            }
            */
        }

        private void RadMenuItemBOB_Click(object sender, Telerik.Windows.RadRoutedEventArgs e)
        {
            var window = new View.BobPanel() { Owner = Application.Current.MainWindow };
            window.Show();
        }
        #endregion
    }
}
