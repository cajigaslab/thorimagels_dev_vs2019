namespace HardwareSetupDll
{
    using System;
    using System.Diagnostics;
    using System.Windows;
    using System.Windows.Controls;

    using HardwareSetupDll.ViewModel;

    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Unity;

    using ThorLogging;

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        #region Fields

        private HardwareSetupViewModel viewModel;
        private HardwareSetupModuleLS hsModuleLS;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
            try
            {
                // Make this window the topmost within the app
                this.Owner = Application.Current.MainWindow;
            }
            catch (Exception e)
            {
                e.ToString();
            }
        }

        public MainWindow(HardwareSetupViewModel HardwareSetupViewModel, HardwareSetupModuleLS HardwareSetupModuleLS)
            : this()
        {
            this.viewModel = HardwareSetupViewModel;

            // create the ViewModel object and setup the DataContext to it
            this.HardwareSetupView.DataContext = HardwareSetupViewModel;

            this.DataContext = HardwareSetupModuleLS;

            this.hsModuleLS = HardwareSetupModuleLS;

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Methods

        private void Close_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
            Close();
        }

        private void Exit_Click(object sender, EventArgs e)
        {
            hsModuleLS.RefereshCaptureSetup();
        }

        #endregion Methods

    }
}