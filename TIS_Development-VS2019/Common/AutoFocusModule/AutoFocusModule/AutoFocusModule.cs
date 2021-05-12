namespace AutoFocusModule
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;

    public sealed class AutoFocusModule
    {
        #region Fields

        private static volatile AutoFocusModule instance;
        private static object syncRoot = new Object();

        #endregion Fields

        #region Constructors

        private AutoFocusModule()
        {
        }

        #endregion Constructors

        #region Properties

        public static AutoFocusModule Instance
        {
            get
            {
                if (instance == null)
                {
                    lock (syncRoot)
                    {
                        if (instance == null)
                            instance = new AutoFocusModule();
                    }
                }

                return instance;
            }
        }

        #endregion Properties

        #region Methods

        public long CallRunAutofocus(long index, long afType, ref bool bFound)
        {
            return RunAutofocus(index, afType, ref bFound);
        }

        public long GetAutoFocusStatus(ref long currentStatus, ref long bestContrastScore, ref double bestZPosition, ref double nextZPosition, ref long currentRepeatIndex)
        {
            return GetAutofocusStatus(ref currentStatus, ref bestContrastScore, ref bestZPosition, ref nextZPosition, ref currentRepeatIndex);
        }

        public long IsAutoFocusRunning()
        {
            return IsAutofocusRunning();
        }

        public long SetupAutoFocus(long afType, long repeat, double afFocusOffset, double expTimeMS, double stepSizeUM, double startPosMM, double stopPosMM, long binning, double finePercentage, long enableGUIUpdate)
        {
            return SetupAutofocus(afType, repeat, afFocusOffset, expTimeMS, stepSizeUM, startPosMM, stopPosMM, binning, finePercentage, enableGUIUpdate);
        }

        public long StopAutoFocus()
        {
            return StopAutofocus();
        }

        public long WillAFExecuteNextIteration(long afType)
        {
            return AutofocusExecuteNextIteration(afType);
        }

        [DllImport(".\\Modules_Native\\AutoFocus.dll", EntryPoint = "AutofocusExecuteNextIteration")]
        private static extern long AutofocusExecuteNextIteration(long afType);

        [DllImport(".\\Modules_Native\\AutoFocus.dll", EntryPoint = "GetAutofocusStatus")]
        private static extern long GetAutofocusStatus(ref long currentStatus, ref long bestContrastScore, ref double bestZPosition, ref double nextZPosition, ref long currentRepeatIndex);

        [DllImport(".\\Modules_Native\\AutoFocus.dll", EntryPoint = "IsAutofocusRunning")]
        private static extern long IsAutofocusRunning();

        [DllImport(".\\Modules_Native\\AutoFocus.dll", EntryPoint = "RunAutofocus")]
        private static extern long RunAutofocus(long index, long afType, ref bool bFound);

        [DllImport(".\\Modules_Native\\AutoFocus.dll", EntryPoint = "SetupAutofocus")]
        private static extern long SetupAutofocus(long afType, long repeat, double afFocusOffset, double expTimeMS, double stepSizeUM, double startPosMM, double stopPosMM, long binning, double finePercentage, long enableGUIUpdate);

        [DllImport(".\\Modules_Native\\AutoFocus.dll", EntryPoint = "StopAutofocus")]
        private static extern long StopAutofocus();

        #endregion Methods
    }
}