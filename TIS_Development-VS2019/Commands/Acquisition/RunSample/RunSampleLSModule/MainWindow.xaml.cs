namespace RunSampleLSDll
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Net;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Animation;
    using System.Windows.Shapes;

    using RunSampleLSDll.ViewModel;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class MainWindow : UserControl
    {
        #region Fields

        //private RunSampleLSViewModel viewModel;
        const int OFFSET_FOR_RESIZE_SCROLL = 118;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
        }

        public MainWindow(RunSampleLSViewModel runSampleViewModel)
            : this()
        {
            //this.viewModel = runSampleViewModel;

            // create the ViewModel object and setup the DataContext to it
            this.RunSampleLSView.DataContext = runSampleViewModel;
            this.imageView.DataContext = runSampleViewModel;

            this.Loaded += new RoutedEventHandler(MainWindow_Loaded);
            this.Unloaded += new RoutedEventHandler(MainWindow_Unloaded);
            this.KeyDown += new KeyEventHandler(MainWindow_KeyDown);
            this.KeyUp += new KeyEventHandler(MainWindow_KeyUp);
            this.RunSampleLSView.PreviewMouseDown += new MouseButtonEventHandler(RunSampleLSView_PreviewMouseDown);

            MVMManager.Instance["ImageViewCaptureVM", "IncreaseViewAreaAction"] = new Action<bool>(AdjustImageViewSize);

            MVMManager.Instance.AddMVM("RunSampleLSViewModel", runSampleViewModel);
            //populate data contents from specified MVMs

            this.RunSampleLSView.digOutputSwitchView.DataContext = MVMManager.Instance["DigitalOutputSwitchesViewModel", runSampleViewModel];
            this.RunSampleLSView.DFLIMControlView.DataContext = MVMManager.Instance["DFLIMControlCaptureViewModel", runSampleViewModel];
            this.imageView.DataContext = MVMManager.Instance["ImageViewCaptureVM", runSampleViewModel];
            //this.RunSampleLSView.RemoteIPC.DataContext = MVMManager.Instance["RemoteIPCControlViewModelBase", runSampleViewModel];
            Application.Current.Exit += Current_Exit;

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Methods

        // This is an almost identical copy to RunSampleLSStart but with a different flag. This flag is only
        // used to start an acquisition after RunSample has been loaded. The flag from RunSampleLSStart
        // is used in other methods related to Script.
        public void RunSampleLSCaptureSetupStart(string path, string experimentName)
        {
            //the view model parameters for the experiment must first be loaded
            //reuse the loading code in the RunSample view to perform the viewmodule
            //property updates

            //if a new destination path has been specified
            if (path.Length > 0)
            {
                ((RunSampleLSViewModel)this.RunSampleLSView.DataContext).OutputPath = path;
            }

            if (experimentName.Length > 0)
            {
                ((RunSampleLSViewModel)this.RunSampleLSView.DataContext).FullExperimentName = experimentName;

                //The execution will occur after the panel has been loaded
                //This flag is checked at the end of the loaded event.
                ((RunSampleLSViewModel)this.RunSampleLSView.DataContext).CaptureSetupStartAfterLoading.Add(true);
            }
        }

        public void RunSampleLSStart(string path, string experimentName)
        {
            //the view model parameters for the experiment must first be loaded
            //reuse the loading code in the RunSample view to perform the viewmodule
            //property updates

            //if a new destination path has been specified
            if (path.Length > 0)
            {
                ((RunSampleLSViewModel)this.RunSampleLSView.DataContext).OutputPath = path;
            }

            if (experimentName.Length > 0)
            {
                ((RunSampleLSViewModel)this.RunSampleLSView.DataContext).FullExperimentName = experimentName;

                //The execution will occur after the panel has been loaded
                //This flag is checked at the end of the loaded event.
                ((RunSampleLSViewModel)this.RunSampleLSView.DataContext).StartAfterLoading.Add(true);
            }
        }

        //when the image is bigger than the display area, increase the display area so the scroll bar
        // can see the entire image
        void AdjustImageViewSize(bool obj)
        {
            if (true == obj)
            {
                double scrollBareight = (double)MVMManager.Instance["ImageViewCaptureSetupVM", "IVScrollBarHeight"];
                imageView.Height = Math.Max(scrollBareight, Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL);
            }
        }

        void Current_Exit(object sender, ExitEventArgs e)
        {
            if (this.IsLoaded)
            {
                ((RunSampleLSViewModel)this.RunSampleLSView.DataContext).CleanupOnQuit();
                ((RunSampleLSViewModel)this.RunSampleLSView.DataContext).UpdateExperimentFile();
            }
        }

        void MainWindow_KeyDown(object sender, KeyEventArgs e)
        {
            this.imageView.ImageView_KeyDown(sender, e);
        }

        void MainWindow_KeyUp(object sender, KeyEventArgs e)
        {
            this.imageView.ImageView_KeyUp(sender, e);
        }

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            Application.Current.MainWindow.SizeChanged += new SizeChangedEventHandler(MainWindow_SizeChanged);
            this.scrollView.Height = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
            this.imageView.Height = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
            RunSampleLSViewModel viewModel = (RunSampleLSViewModel)this.RunSampleLSView.DataContext;
            viewModel.MVMNames = new string[] { "DigitalOutputSwitchesViewModel", "LampControlViewModel", "ObjectiveControlViewModel", "DFLIMControlCaptureViewModel", "MiniCircuitsSwitchControlViewModel", "RemoteIPCControlViewModelBase" };
            viewModel.UnloadWhole = false;

            MVMManager.Instance["ImageViewCaptureVM", "IVHeight"] = this.imageView.Height;

            //populate data contents from specified MVMs
            viewModel.MVMNames = new string[] { "DigitalOutputSwitchesViewModel", "LampControlViewModel", "ObjectiveControlViewModel", "MiniCircuitsSwitchControlViewModel", "RemoteIPCControlViewModelBase" };
            this.RunSampleLSView.digOutputSwitchView.DataContext = MVMManager.Instance["DigitalOutputSwitchesViewModel", viewModel];
        }

        void MainWindow_SizeChanged(object sender, System.Windows.SizeChangedEventArgs e)
        {
            double newHeight = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
            if (newHeight <= 0)
            {
                newHeight = 1;
            }
            this.scrollView.Height = newHeight;
            this.imageView.Height = newHeight;

            MVMManager.Instance["ImageViewCaptureVM", "IVHeight"] = this.imageView.Height;
        }

        void MainWindow_Unloaded(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel viewModel = (RunSampleLSViewModel)this.RunSampleLSView.DataContext;
            viewModel.UnloadWhole = true;
            viewModel.ReleaseBleachMem();
            viewModel.MVMNames = new string[0];
            Application.Current.MainWindow.SizeChanged -= new SizeChangedEventHandler(MainWindow_SizeChanged);
        }

        void RunSampleLSView_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.imageView.SetSelectROI();
        }

        private void scrollViewImage_ScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            RunSampleLSViewModel viewModel = (RunSampleLSViewModel)this.RunSampleLSView.DataContext;
            if (null == viewModel)
            {
                return;
            }
            viewModel.ROItoolbarMargin = new Thickness(0, scrollViewImage.ContentVerticalOffset, 0, 0);
        }

        #endregion Methods
    }
}