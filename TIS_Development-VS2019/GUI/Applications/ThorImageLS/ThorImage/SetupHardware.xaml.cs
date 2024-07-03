namespace ThorImage
{
    using System;
    using System.IO;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for SetupHardware.xaml
    /// </summary>
    public partial class SetupHardware : Window
    {
        #region Fields

        //Flag used to check if HardwareSetup window was closed via the X or the OK button. 
        bool closeRequested = true;

        #endregion Fields

        #region Constructors

        public SetupHardware()
        {
            InitializeComponent();
            closeRequested = true;
            HSWindow.ResizeMode = (ResourceManagerCS.Instance.TabletModeEnabled) ? ResizeMode.CanResizeWithGrip : ResizeMode.NoResize;
            HSWindow.SizeToContent = (ResourceManagerCS.Instance.TabletModeEnabled) ? SizeToContent.Manual : SizeToContent.WidthAndHeight;
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                HSWindow.Height = HSWindow.Width = 600;
            }
        }

        #endregion Constructors

        #region Methods

        private void butOk_Click(object sender, RoutedEventArgs e)
        {
            closeRequested = false;
            if (hardwareSetupUC.IsXYStageReady == true)
            {
                this.DialogResult = true;
            }
            else
            {
                this.DialogResult = false;
            }
            ResourceManagerCS.BackupDirectory(ResourceManagerCS.GetMyDocumentsThorImageFolderString());
            this.Close();
            HardwareState.HwState.GetInstance(hardwareSetupUC.SelectedModalityName).ConfigPhase();
        }

        private void HSWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            //Only switch the startup flag in the ResourceManager if the X button was pressed
            if (closeRequested)
            {
                ResourceManagerCS.SetStartupFlag("0");

                int numErrors = ResourceManagerCS.BackupDirectory(ResourceManagerCS.GetMyDocumentsThorImageFolderString()); // backup the documents folder 
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " Backup of Documents folder finished with " + numErrors + " Errors");
                numErrors = ResourceManagerCS.BackupDirectory(Directory.GetCurrentDirectory() + "\\"); // backup the root folder
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " Backup of Root folder finished with " + numErrors + " Errors");
            }
        }

        #endregion Methods
    }
}