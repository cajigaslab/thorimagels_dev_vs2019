namespace HardwareSetupDll.View
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Xml;

    using FolderDialogControl;

    using HardwareSetupDll.ViewModel;

    using HardwareSetupUserControl;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for DisplayOptions.xaml
    /// </summary>
    public partial class HardwareSettings : Window
    {
        #region Constructors

        public HardwareSettings()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Methods

        private void HardwareSettings_Loaded(object sender, RoutedEventArgs e)
        {
            UpdateDocument();
        }

        private void btnClose_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
            Close();
        }

        private void btnUpdate_Click(object sender, RoutedEventArgs e)
        {
            UpdateDocument();
        }

        /// <summary>
        /// retrieve the application settings complete path and file name
        /// </summary>
        private void UpdateDocument()
        {
            //force the document to save
            xmlViewer.XmlDocument = null;

            try
            {
                //reload the document
                XmlDocument xmlDoc = new XmlDocument();

                xmlDoc.Load(ResourceManagerCS.GetHardwareSettingsFileString());
                xmlViewer.XmlDocument = xmlDoc;
                ResourceManagerCS.ResourceManagerLoadSettings();
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error, 1, "HardwareSetupDll UpdateDocument error: " + ex.Message);
            }
        }

        #endregion Methods
    }
}