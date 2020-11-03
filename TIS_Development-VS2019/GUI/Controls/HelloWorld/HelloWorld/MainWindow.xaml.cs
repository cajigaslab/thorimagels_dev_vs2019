namespace HelloWorldDll
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Net;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Animation;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    using HelloWorldDll.Model;
    using HelloWorldDll.View;
    using HelloWorldDll.ViewModel;

    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Unity;
    using Microsoft.Win32;

    using ThorLogging;

    public partial class MainWindow : UserControl
    {
        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
        }

        public MainWindow(HelloWorldViewModel HelloWorldViewModel)
            : this()
        {
            this.DataContext = HelloWorldViewModel;

            this.masterView.DataContext = HelloWorldViewModel;

            this.Loaded += new RoutedEventHandler(MainWindow_Loaded);
            this.Unloaded += new RoutedEventHandler(MainWindow_Unloaded);
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Methods

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
        }

        void MainWindow_Unloaded(object sender, RoutedEventArgs e)
        {
        }

        #endregion Methods
    }
}