namespace ScriptManagerDll
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

    using ScriptManagerDll.Model;
    using ScriptManagerDll.View;
    using ScriptManagerDll.ViewModel;

    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Unity;
    using Microsoft.Win32;

    public partial class MainWindow : UserControl
    {
        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
        }

        const int OFFSET_FOR_RESIZE_SCROLL = 150;

        public MainWindow(ScriptManagerViewModel ScriptManagerViewModel)
            : this()
        {
            this.DataContext = ScriptManagerViewModel;

            this.masterView.DataContext = ScriptManagerViewModel;

            this.Loaded += new RoutedEventHandler(MainWindow_Loaded);
            this.Unloaded += new RoutedEventHandler(MainWindow_Unloaded);
         }

        #endregion Constructors

        #region Methods

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
               Application.Current.MainWindow.SizeChanged += new SizeChangedEventHandler(MainWindow_SizeChanged);
            this.scrollView.Height = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
         
        }

        void MainWindow_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            this.scrollView.Height = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
        }

        void MainWindow_Unloaded(object sender, RoutedEventArgs e)
        {
        }

        #endregion Methods
    }
}