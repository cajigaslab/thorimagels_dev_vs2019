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
using Telerik.Windows.Controls;
using ThorDAQConfigControl.ViewModel;

namespace ThorDAQConfigControl.View
{
    /// <summary>
    /// Interaction logic for TestPanel.xaml
    /// </summary>
    public partial class TestPanel : UserControl
    {
        MainContentViewModel vm;
        public TestPanel()
        {
            InitializeComponent(); 
            vm = MainContentViewModel.GetInstance();
            this.DataContext = vm;

            Init();
        }

        private void Init()
        {
            for (int i = 0; i < vm.AICount; i++)
            {
                var comboBoxItem = new RadComboBoxItem();
                comboBoxItem.Content = "AI" + i;
                aiChannelNameComboBox.Items.Add(comboBoxItem);
            }

            for (int i = 0; i < vm.AOCount; i++)
            {
                var comboBoxItem = new RadComboBoxItem();
                comboBoxItem.Content = "AO" + i;
                    aoChannelNameComboBox.Items.Add(comboBoxItem);
            }

            for (int i = 0; i < vm.DCCount; i++)
            {
                var comboBoxItem = new RadComboBoxItem();
                comboBoxItem.Content = "D" + i;
                digitalChannelNameComboBox.Items.Add(comboBoxItem);
            }
        }

        private void TestPanelTabControl_SelectionChanged(object sender, RadSelectionChangedEventArgs e)
        {
            /*
            if (e.AddedItems.Count > 0)
            {
                string tabItem = (e.AddedItems[0] as RadTabItem).Content.ToString();
                switch (tabItem)
                {
                    case "analogInputTabItem":
                        break;
                    case "analogOutputTabItem":
                        break;
                    case "DigitalIOTabItem":
                        break;
                }
            }
            */
        }

        private void CheckBox_Click(object sender, RoutedEventArgs e)
        {
            if((sender as CheckBox).IsChecked == true)
            {
                yAxis.Minimum = double.NegativeInfinity;
                yAxis.Maximum = double.PositiveInfinity;
            }
            else
            {
                yAxis.Minimum = vm.AIMinLimit;
                yAxis.Maximum = vm.AIMaxLimit;
            }
        }

        private void digitalChannelNameComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            /*
            DILineDirectionListBox.Items.Clear();
            DILineStateListBox.Items.Clear();
            var lineList = vm.DIOSimulator.GetLineNames(digitalChannelNameComboBox.SelectedIndex);
            foreach (var item in lineList)
            {
                var listBoxItem = new RadListBoxItem();
                listBoxItem.Content = item;
                DILineDirectionListBox.Items.Add(listBoxItem);

                listBoxItem = new RadListBoxItem();
                listBoxItem.Content = item;
                DILineStateListBox.Items.Add(listBoxItem);
            }
            DILineDirectionListBox.SelectedIndex = 0;
            DILineStateListBox.SelectedIndex = 0;
            if (digitalChannelNameComboBox.Items?.Count > 0)
                portDirectionLabel.Content = (digitalChannelNameComboBox.SelectedValue as RadComboBoxItem).Content.ToString() + " Direction";
            */
        }

        /*
        private void DILineDirectionListBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (DILineDirectionListBox.Items?.Count > 0)
                selectedLineLabel.Content = (DILineDirectionListBox.SelectedValue as RadListBoxItem).Content.ToString();
        }
        */
    }
}
