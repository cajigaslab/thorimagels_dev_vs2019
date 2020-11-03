
using System.Windows.Controls;
using Microsoft.Practices.Composite.Modularity;
using Microsoft.Practices.Composite.Regions;
using Microsoft.Practices.Unity;
using HardwareSetupDll.ViewModel;
using System.Diagnostics;
using System;
using System.Windows;
using ThorLogging;
using ThorImageInfastructure;

namespace HardwareSetupDll
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private HardwareSetupViewModel viewModel;

        public MainWindow()
        {
            InitializeComponent();           
        }

        public MainWindow(HardwareSetupViewModel HardwareSetupViewModel)
            : this()
        {
            this.viewModel = HardwareSetupViewModel;

            // create the ViewModel object and setup the DataContext to it
            this.HardwareSetupView.DataContext = HardwareSetupViewModel;

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        private void Close_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
            Close();

            //publish a hardware settings xml file change event to be notified to the exp setup module.
            viewModel.PublishChangeEvent();
        }     

    }
}
