namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Xml;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetupViewModel : ViewModelBase
    {
        #region Events

        //notify 3D volumeview that the z stack capture is finished
        public event Action<bool> ZStackCaptureFinished;

        //notify 2D volumeview that the z stack capture is started
        public event Action<bool> ZStackCapturing;

        #endregion Events
    }
}