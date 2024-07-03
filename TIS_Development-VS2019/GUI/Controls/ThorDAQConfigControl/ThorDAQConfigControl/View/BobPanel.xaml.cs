using System.Collections.Generic;
using System.Windows.Controls;
using Telerik.Windows.Controls;
using ThorDAQConfigControl.Controls;
using ThorDAQConfigControl.ViewModel;
using ThorSharedTypes;

namespace ThorDAQConfigControl.View
{
    /// <summary>
    /// Interaction logic for BobPanel.xaml
    /// </summary>
    public partial class BobPanel : RadWindow
    {
        MainContentViewModel vm;

        public BobPanel()
        {
            InitializeComponent();
            vm = MainContentViewModel.GetInstance();
            this.DataContext = vm;

            LoadBOBs();
            vm.IsBobPanelShown = true;
            this.Closed += BobPanel_Closed;
        }

        private void BobPanel_Closed(object sender, WindowClosedEventArgs e)
        {
            vm.IsBobPanelShown = false;
        }

        // Add ports into the panel
        private void LoadBOBs()
        {
            var startEnum = (int)BBoxLEDenum.AO0;
            for (int i = 0; i < vm.AOCount; i++)
            {
                var item = new BOBItem(startEnum + i, "AO" + i);
                vm.AOBobList.Add(item);
                if (i < 6)
                {
                    AO1M200Grid.Children.Add(item);
                    Grid.SetColumn(item, i);
                }
                else if (i < 10)
                {
                    AO1M50Grid.Children.Add(item);
                    Grid.SetColumn(item, i - 6);
                }
                else
                {
                    AO20M50Grid.Children.Add(item);
                    Grid.SetColumn(item, i - 10);
                }
            }

            startEnum = (int)BBoxLEDenum.AI0;
            for (int i = 0; i < vm.AICount; i++)
            {
                var item = new BOBItem(startEnum + i, "AI" + i);
                vm.AIBobList.Add(item);
                if (i < 6)
                {
                    AO200KGrid.Children.Add(item);
                    Grid.SetColumn(item, i);
                }
                else
                {
                    AO1KGrid.Children.Add(item);
                    Grid.SetColumn(item, i - 6);
                }
            }

            startEnum = (int)BBoxLEDenum.D0;
            for (int i = 0; i < vm.DCCount; i++)
            {
                var item = new BOBItem(startEnum + i, "D" + i);
                vm.DCBobList.Add(item);
                if (i < 8)
                {
                    DIOLine1Grid.Children.Add(item);
                    Grid.SetColumn(item, i);
                }
                else if (i < 16)
                {
                    DIOLine2Grid.Children.Add(item);
                    Grid.SetColumn(item, i - 8);
                }
                else if (i < 24)
                {
                    DIOLine3Grid.Children.Add(item);
                    Grid.SetColumn(item, i - 16);
                }
                else
                {
                    DIOLine4Grid.Children.Add(item);
                    Grid.SetColumn(item, i - 24);
                }
            }
        }
    }
}
