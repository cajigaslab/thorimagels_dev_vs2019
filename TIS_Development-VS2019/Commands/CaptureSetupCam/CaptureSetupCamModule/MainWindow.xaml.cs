namespace CaptureSetupDll
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Net;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Animation;
    using System.Windows.Shapes;

    using CaptureSetupDll.Model;
    using CaptureSetupDll.ViewModel;

    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Unity;

    using ThorLogging;

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class MainWindow : UserControl
    {
        #region Fields

        public LiveImageViewModel liveViewModel;
        public CaptureSetupViewModel viewModel;

        const int OFFSET_FOR_RESIZE_SCROLL = 150;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
        }

        public MainWindow(CaptureSetupViewModel CaptureSetupViewModel,LiveImageViewModel liveViewModel)
            : this()
        {
            this.viewModel = CaptureSetupViewModel;

            this.MasterView.DataContext = liveViewModel;
            this.MasterView.DataContextliveImageVM = liveViewModel;

            //this.liveViewModel = liveViewModel;
            this.imageView.DataContext = liveViewModel;

            //force the path setting from the viewmodel into the view
            string path = this.viewModel.ExpPath;

            //choose the default template if the current run experiment does not exists
            if (!File.Exists(path))
            {
                string tempFolder = Application.Current.Resources["TemplatesFolder"].ToString();
                path = tempFolder + "\\Active.xml";
            }

            this.viewModel.ExpPath = path;

            //force the update menu message to be published
            this.MasterView.SetPath(path);

            this.Loaded += new RoutedEventHandler(MainWindow_Loaded);
            this.Unloaded += new RoutedEventHandler(MainWindow_Unloaded);
            Application.Current.Exit += new ExitEventHandler(Current_Exit);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Properties

        public Guid CommandGuid
        {
            get { return this.viewModel.CommandGuid; }
        }

        #endregion Properties

        #region Methods

        public void UpdateHardwareListView()
        {
            CaptureSetupDll.View.MasterView mv = new CaptureSetupDll.View.MasterView();
            mv.UpdateHardwareListView();
        }

        void Current_Exit(object sender, ExitEventArgs e)
        {
            ((LiveImageViewModel)this.imageView.DataContext).PersistData();
            ((LiveImageViewModel)this.imageView.DataContext).LiveImageStart(false);
            ((LiveImageViewModel)this.imageView.DataContext).ReleaseHandlers();
        }

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            Application.Current.MainWindow.SizeChanged += new SizeChangedEventHandler(MainWindow_SizeChanged);
            this.scrollView.Height = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
            this.imageView.Height = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
        }

        void MainWindow_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            this.scrollView.Height = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
            this.imageView.Height = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
        }

        void MainWindow_Unloaded(object sender, RoutedEventArgs e)
        {
            Application.Current.MainWindow.SizeChanged -= new SizeChangedEventHandler(MainWindow_SizeChanged);
        }

        #endregion Methods
    }
}