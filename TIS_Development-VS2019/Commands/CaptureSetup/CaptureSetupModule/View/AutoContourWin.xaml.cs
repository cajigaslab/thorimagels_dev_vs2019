namespace CaptureSetupDll.ViewModel
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
    /// Interaction logic for AutoContourWin.xaml
    /// </summary>
    public partial class AutoContourWin : Window
    {
        #region Constructors

        public AutoContourWin()
        {
            InitializeComponent();
            this.Owner = Application.Current.MainWindow;
            this.Loaded += AutoContourWin_Loaded;
            this.Unloaded += AutoContourWin_Unloaded;
            //this.Closed += AutoContourWin_Closed;
        }

        #endregion Constructors

        #region Methods

        void AutoContourWin_Loaded(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            if (vm == null)
            {
                return;
            }
            vm.AutoCoutourWinInit = true;
            vm.ChannelChanged += vm_ChannelChanged;
            List<int> channelEnable = new List<int>();
            if (vm.LSMChannelEnable0 == true)
            {
                channelEnable.Add(1);
                channelA.IsEnabled = true;
            }
            if (vm.LSMChannelEnable1 == true)
            {
                channelEnable.Add(2);
                channelB.IsEnabled = true;
            }
            if (vm.LSMChannelEnable2 == true)
            {
                channelEnable.Add(3);
                channelC.IsEnabled = true;
            }
            if (vm.LSMChannelEnable3 == true)
            {
                channelEnable.Add(4);
                channelD.IsEnabled = true;
            }
            ToggleButton[] toggleButton = { this.channelA, this.channelB, this.channelC, this.channelD };
            if (channelEnable.Contains(vm.AutoROIDisplayChannel) == true)
            {
                toggleButton[vm.AutoROIDisplayChannel - 1].IsChecked = true;
            }
        }

        void AutoContourWin_Unloaded(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            if (vm == null)
            {
                return;
            }
            vm.ChannelChanged -= vm_ChannelChanged;
            vm.AutoCoutourWinInit = false;
        }

        //void AutoContourWin_Closed(object sender, EventArgs e)
        //{
        //    Application.Current.MainWindow.Activate();
        //}
        private void btnSetChannel_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            ToggleButton[] toggleButton = {this.channelA,this.channelB,this.channelC,this.channelD};

            switch (((ToggleButton)sender).Name)
            {
                case "channelA": vm.AutoROIDisplayChannel = 1; break;
                case "channelB": vm.AutoROIDisplayChannel = 2; break;
                case "channelC": vm.AutoROIDisplayChannel = 3; break;
                case "channelD": vm.AutoROIDisplayChannel = 4; break;
            }

            if (toggleButton != null)
            {
                for (int i = 0; i < 4; i++)
                {
                    if (i == (vm.AutoROIDisplayChannel-1))
                    {
                        continue;
                    }
                    toggleButton[i].IsChecked = false;
                }
            }
        }

        private void btnUnsetChannel_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            if (vm == null)
            {
                return;
            }

            int selChannel = 0;
            switch (((ToggleButton)sender).Name)
            {
                case "channelA": selChannel = 1; break;
                case "channelB": selChannel = 2; break;
                case "channelC": selChannel = 3; break;
                case "channelD": selChannel = 4; break;
            }

            if (vm.AutoROIDisplayChannel == selChannel)
            {
                vm.AutoROIDisplayChannel = 0;
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        void vm_ChannelChanged()
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            if (vm == null)
            {
                return;
            }
            if (vm.LSMChannelEnable0 == false)
            {
                channelA.IsEnabled = false;
                channelA.IsChecked = false;
            }
            else
            {
                if (channelA.IsEnabled == false)
                {
                    channelA.IsEnabled = true;
                }
            }
            if (vm.LSMChannelEnable1 == false)
            {
                channelB.IsEnabled = false;
                channelB.IsChecked = false;
            }
            else
            {
                if (channelB.IsEnabled == false)
                {
                    channelB.IsEnabled = true;
                }
            }
            if (vm.LSMChannelEnable2 == false)
            {
                channelC.IsEnabled = false;
                channelC.IsChecked = false;
            }
            else
            {
                if (channelC.IsEnabled == false)
                {
                    channelC.IsEnabled = true;
                }
            }
            if (vm.LSMChannelEnable3 == false)
            {
                channelD.IsEnabled = false;
                channelD.IsChecked = false;
            }
            else
            {
                if (channelD.IsEnabled == false)
                {
                    channelD.IsEnabled = true;
                }
            }
            //throw new NotImplementedException();
        }

        #endregion Methods
    }
}