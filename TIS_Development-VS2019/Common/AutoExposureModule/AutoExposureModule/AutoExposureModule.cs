using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace AutoExposureModule
{
    public class AutoExposureEventArgs : EventArgs
    {
        #region Constructors

        public AutoExposureEventArgs(bool isStable, bool isRunning, double exposure, double gain)
        {
            IsStable = isStable;
            IsRunning = isRunning;
            ExposureTime_ms = exposure;
            Gain_db = gain;
        }

        #endregion Constructors

        #region Properties

        public double ExposureTime_ms
        {
            get; set;
        }

        public double Gain_db
        {
            get; set;
        }

        public bool IsRunning
        {
            get; set;
        }

        public bool IsStable
        {
            get; set;
        }

        #endregion Properties
    }

    public class AutoExposureModule
    {
        #region Fields

        private static volatile AutoExposureModule instance;
        private static object syncRoot = new Object();

        private IntPtr _nativeCallbackFuncPtr;
        private AutoExposureUpdateCallback _updateCallbackDelegate;

        #endregion Fields

        #region Constructors

        private AutoExposureModule()
        {
            _updateCallbackDelegate = new AutoExposureUpdateCallback(AutoExposureUpdateCallbackHandler);
            _nativeCallbackFuncPtr = Marshal.GetFunctionPointerForDelegate(_updateCallbackDelegate);
            RegisterUpdateCallback_native(_nativeCallbackFuncPtr);
        }

        ~AutoExposureModule()
        {
            // TODO: finalizer?
            StopAutoExposure_native();
            UnregisterUpdateCallback_native(_nativeCallbackFuncPtr);
        }

        #endregion Constructors

        #region Delegates

        // for public use
        public delegate void AutoExposureUpdateHandler(object sender, AutoExposureEventArgs args);

        // for non-public use
        private delegate void AutoExposureUpdateCallback(bool isStable, bool isRunning, double exposure, double gain);

        #endregion Delegates

        #region Events

        public event AutoExposureUpdateHandler AutoExposureUpdateEvent;

        #endregion Events

        #region Properties

        public static AutoExposureModule Instance
        {
            get
            {
                lock (syncRoot)
                {
                    if (null == instance)
                    {
                        instance = new AutoExposureModule();
                    }
                }
                return instance;
            }
        }

        public double TargetPercent
        {
            get
            {
                return GetTargetPercent_native();
            }

            set
            {
                SetTargetPercent_native(value);
            }
        }

        #endregion Properties

        #region Methods

        public bool IsRunning()
        {
            return IsRunning_native();
        }

        public bool IsStable()
        {
            return IsStable_native();
        }

        public void StartAutoExposure()
        {
            StartAutoExposure_native();
        }

        public void StopAutoExposure()
        {
            StopAutoExposure_native();
        }

        private static void AutoExposureUpdateCallbackHandler(bool isStable, bool isRunning, double exposure, double gain)
        {
            // called once whenever AE changes
            var args = new AutoExposureEventArgs(isStable, isRunning, exposure, gain);
            Instance.AutoExposureUpdateEvent.Invoke(Instance, args);
        }

        [DllImport(".\\Modules_Native\\AutoExposure.dll", EntryPoint = "GetAutoExposureTargetPercent")]
        private static extern double GetTargetPercent_native();

        [DllImport(".\\Modules_Native\\AutoExposure.dll", EntryPoint = "IsRunning")]
        private static extern bool IsRunning_native();

        [DllImport(".\\Modules_Native\\AutoExposure.dll", EntryPoint = "IsStable")]
        private static extern bool IsStable_native();

        [DllImport(".\\Modules_Native\\AutoExposure.dll", EntryPoint = "RegisterAutoExposureUpdateCallback")]
        private static extern void RegisterUpdateCallback_native(IntPtr c);

        [DllImport(".\\Modules_Native\\AutoExposure.dll", EntryPoint = "SetAutoExposureTargetPercent")]
        private static extern void SetTargetPercent_native(double percent);

        [DllImport(".\\Modules_Native\\AutoExposure.dll", EntryPoint = "RunAutoExposure")]
        private static extern void StartAutoExposure_native();

        [DllImport(".\\Modules_Native\\AutoExposure.dll", EntryPoint = "SetAutoExposureStopFlag")]
        private static extern void StopAutoExposure_native();

        [DllImport(".\\Modules_Native\\AutoExposure.dll", EntryPoint = "RegisterAutoExposureUpdateCallback")]
        private static extern void UnregisterUpdateCallback_native(IntPtr c);

        #endregion Methods
    }
}