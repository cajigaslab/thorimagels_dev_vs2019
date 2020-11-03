namespace ScriptFijiProcessingDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Win32;

    using ScriptFijiProcessingDll.View;
    using ScriptFijiProcessingDll.ViewModel;


    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for MasterView.xaml
    /// </summary>
    public partial class MasterView : UserControl
    {
        #region Constructors

        public MasterView()
        {
            InitializeComponent();

            this.Loaded += new RoutedEventHandler(MasterView_Loaded);
            this.Unloaded += new RoutedEventHandler(MasterView_Unloaded);
        }

        #endregion Constructors

        #region Methods

        void MasterView_Loaded(object sender, RoutedEventArgs e)
        {
            if (null == ((MasterView)sender).DataContext)
            {
                return;
            }

            ((ScriptFijiProcessingViewModel)this.DataContext).EnableHandlers();
        }

        void MasterView_Unloaded(object sender, RoutedEventArgs e)
        {
            if (null == ((MasterView)sender).DataContext)
            {
                return;
            }
            ((ScriptFijiProcessingViewModel)this.DataContext).ReleaseHandlers();
        }

        #endregion Methods
    }
}