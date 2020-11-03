namespace ThorImage
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    using ThorLogging;
    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Shell : Window
    {
        #region Constructors

        public Shell()
        {
            InitializeComponent();
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                // When tablet mode is enabled, there is a gap between menu module and capture setup
                // This is because the height is set to 64, setting to auto only increases the
                // gap in normal mode. Need to change this number manually specifically for the tablet
                MenuRow.Height = new GridLength(54);
            }

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors
    }
}