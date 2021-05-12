namespace AutoFocusControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
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

    public class AutoFocusControlModel
    {
        #region Constructors

        /// <summary>
        /// Create a new instance of the AutoFocusControlModel class
        /// </summary>
        public AutoFocusControlModel()
        {
        }

        #endregion Constructors

        #region Methods

        public bool AutoFocus(double magnification, int AutoFocusType, ref bool AutoFocusFound)
        {
            if (false == StartAutoFocus(magnification, AutoFocusType, ref AutoFocusFound))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " StartAutoFocus failed");
                return false;
            }

            return true;
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "StartAutoFocus")]
        private static extern bool StartAutoFocus(double magnification, int AutoFocusType, ref bool AutoFocusFound);

        #endregion Methods
    }
}