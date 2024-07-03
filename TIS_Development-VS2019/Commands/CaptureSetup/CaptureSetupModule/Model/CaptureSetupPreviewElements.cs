namespace CaptureSetupDll.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetup : INotifyPropertyChanged
    {
        #region Fields

        private static ReportZStackCaptureFinished _zStackPreviewFinishedCallBack;

        #endregion Fields

        #region Methods

        public void StopZStackPreview()
        {
            MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = false;
            StopZStackCapture();
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "StopZStackCapture")]
        private static extern bool StopZStackCapture();

        private void ZStackFinished()
        {
            if (true == (bool)MVMManager.Instance["ZControlViewModel", "IsZStackCapturing", (object)false])
            {
                MVMManager.Instance["ZControlViewModel", "IsZStackCapturing"] = false;
                MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = false;
                MVMManager.Instance["CaptureSetupViewModel", "PreviewProtocol"] = "ZStackPreviewStop";
            }

            if (true == (bool)MVMManager.Instance["SequentialControlViewModel", "IsSequentialCapturing", (object)false])
            {
                MVMManager.Instance["SequentialControlViewModel", "IsSequentialCapturing"] = false;
                MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = false;
                MVMManager.Instance["CaptureSetupViewModel", "PreviewProtocol"] = "SequentialPreviewStop";
            }
        }

        #endregion Methods
    }
}