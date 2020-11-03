namespace ThorSharedTypes
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Linq.Expressions;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Input;
    using System.Xml;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorLogging;

    public class HwVal<T> : INotifyPropertyChanged
    {
        #region Fields

        //Fire action after the provided get method has been called
        public Action<int, T> AdditionalGetLogic = null;

        //Fire action after the provided set method has been called
        public Action<int, T> AdditionalSetLogic = null;

        private HWGetParamAvailableCallback getParamAvailableMethod = null;
        private HWGetParamCallback getParamMethod = null;
        private HWGetCallbackRange getParamRangeMethod = null;
        private HWGetParamReadOnlyCallback getParamReadOnlyMethod = null;
        private HWSetParamCallback setParamMethod = null;
        private HWType _hwType;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// constructor
        /// </summary>
        /// <param name="index"></param>
        /// <param name="selection"></param>
        /// <param name="paramID"></param>
        /// <param name="waitTillDone">wait till done will only work for HW of type device</param>
        public HwVal(int index, int hwSelection, int paramID, int waitTillDone = (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT)
        {
            Index = index;
            Selection = hwSelection;

            switch ((SelectedHardware)hwSelection)
            {
                case SelectedHardware.SELECTED_CAMERA1:
                case SelectedHardware.SELECTED_BLEACHINGSCANNER:
                    _hwType = HWType.Camera;
                    break;
                default:
                    _hwType = HWType.Device;
                    break;
            }

            WaitTillDone = waitTillDone;
            ParamIDGetter = paramID;
            ParamIDSetter = paramID;
            InitParamMethods(_hwType);
        }

        /// <summary>
        /// constructor
        /// </summary>
        /// <param name="index"></param>
        /// <param name="selection"></param>
        /// <param name="paramID"></param>
        /// <param name="waitTillDone">wait till done will only work for HW of type device</param>
        public HwVal(int index, int hwSelection, int[] paramID, int waitTillDone = (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT)
        {
            Index = index;
            Selection = hwSelection;

            switch ((SelectedHardware)hwSelection)
            {
                case SelectedHardware.SELECTED_CAMERA1:
                case SelectedHardware.SELECTED_BLEACHINGSCANNER:
                    _hwType = HWType.Camera;
                    break;
                default:
                    _hwType = HWType.Device;
                    break;
            }

            WaitTillDone = waitTillDone;

            ParamIDGetter = paramID[0];
            ParamIDSetter = paramID[1];
            InitParamMethods(_hwType);
        }

        #endregion Constructors

        #region Delegates

        public delegate int HWGetCallbackRange(int arg1, int arg2, ref T arg3, ref T arg4, ref T arg5);

        public delegate int HWGetParamAvailableCallback(int arg1, int arg2);

        public delegate int HWGetParamCallback(int arg1, int arg2, ref T arg3);

        public delegate int HWGetParamReadOnlyCallback(int arg1, int arg2);

        public delegate int HWSetParamCallback(int arg1, int arg2, T arg3, int arg4);

        #endregion Delegates

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public bool IsAvailable
        {
            get
            {
                return getParamAvailableMethod(Selection, ParamIDSetter) == 1;
            }
        }

        public bool IsReadOnly
        {
            get
            {
                return getParamReadOnlyMethod(Selection, ParamIDSetter) == 1;
            }
        }

        public T Max
        {
            get
            {
                T tMin = default(T);
                T tMax = default(T);
                T tDefault = default(T);
                if (1 == getParamRangeMethod(Selection, ParamIDSetter, ref tMin, ref tMax, ref tDefault))
                {
                    return tMax;
                }
                return default(T);
            }
        }

        public T Min
        {
            get
            {
                T tMin = default(T);
                T tMax = default(T);
                T tDefault = default(T);
                if (1 == getParamRangeMethod(Selection, ParamIDSetter, ref tMin, ref tMax, ref tDefault))
                {
                    return tMin;
                }
                return default(T);
            }
        }

        public T Value
        {
            get
            {
                T val = default(T);

                if (IsAvailable)
                {
                    getParamMethod(Selection, ParamIDGetter, ref val);

                    if (null != AdditionalGetLogic)
                    {
                        AdditionalGetLogic(Index, val);
                    }
                }
                return val;
            }

            set
            {
                if (IsAvailable)
                {
                    setParamMethod(Selection, ParamIDSetter, value, WaitTillDone);

                    if (null != AdditionalSetLogic)
                    {
                        AdditionalSetLogic(Index, value);
                    }

                    OnPropertyChanged("Value");
                }
            }
        }

        private int Index
        {
            get;
            set;
        }

        private int ParamIDGetter
        {
            get;
            set;
        }

        private int ParamIDSetter
        {
            get;
            set;
        }

        private int Selection
        {
            get;
            set;
        }

        private int WaitTillDone
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Initialize the get and set parameter methods
        /// </summary>
        private void InitParamMethods(HWType hwType)
        {
            getParamMethod = GetSetHWParams<T>.GetParamMethod(hwType);
            setParamMethod = GetSetHWParams<T>.SetParamMethod(hwType);
            getParamRangeMethod = GetSetHWParams<T>.GetParamRangeMethod(hwType);
            getParamAvailableMethod = GetSetHWParams<T>.GetParamAvailableMethod(hwType);
            getParamReadOnlyMethod = GetSetHWParams<T>.GetParamReadOnlyMethod(hwType);
        }

        void OnPropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }

    internal static class GetSetHWParams<T>
    {
        #region Methods

        internal static HwVal<T>.HWGetParamAvailableCallback GetParamAvailableMethod(HWType hwType)
        {
            switch (hwType)
            {
                case HWType.Camera:
                    return (HwVal<T>.HWGetParamAvailableCallback)ResourceManagerCS.GetCameraParamAvailable;
                case HWType.Device:
                    return (HwVal<T>.HWGetParamAvailableCallback)ResourceManagerCS.GetDeviceParamAvailable;
            }

            return null;
        }

        internal static HwVal<T>.HWGetParamCallback GetParamMethod(HWType hwType)
        {
            switch (hwType)
            {
                case HWType.Camera:
                    if (typeof(T) == typeof(double))
                    {
                        return (HwVal<T>.HWGetParamCallback)((object)(HwVal<double>.HWGetParamCallback)ResourceManagerCS.GetCameraParamDouble);
                    }
                    else if (typeof(T) == typeof(int))
                    {
                        return (HwVal<T>.HWGetParamCallback)((object)(HwVal<int>.HWGetParamCallback)ResourceManagerCS.GetCameraParamInt);
                    }
                    else
                    {
                        return null;
                    }
                case HWType.Device:
                    if (typeof(T) == typeof(double))
                    {
                        return (HwVal<T>.HWGetParamCallback)((object)(HwVal<double>.HWGetParamCallback)ResourceManagerCS.GetDeviceParamDouble);
                    }
                    else if (typeof(T) == typeof(int))
                    {
                        return (HwVal<T>.HWGetParamCallback)((object)(HwVal<int>.HWGetParamCallback)ResourceManagerCS.GetDeviceParamInt);
                    }
                    else
                    {
                        return null;
                    }
            }

            return null;
        }

        internal static HwVal<T>.HWGetCallbackRange GetParamRangeMethod(HWType hwType)
        {
            switch (hwType)
            {
                case HWType.Camera:
                    if (typeof(T) == typeof(double))
                    {
                        return (HwVal<T>.HWGetCallbackRange)((object)(HwVal<double>.HWGetCallbackRange)ResourceManagerCS.GetCameraParamRangeDouble);
                    }
                    else if (typeof(T) == typeof(int))
                    {
                        return (HwVal<T>.HWGetCallbackRange)((object)(HwVal<int>.HWGetCallbackRange)ResourceManagerCS.GetCameraParamRangeInt);
                    }
                    else
                    {
                        return null;
                    }
                case HWType.Device:
                    if (typeof(T) == typeof(double))
                    {
                        return (HwVal<T>.HWGetCallbackRange)((object)(HwVal<double>.HWGetCallbackRange)ResourceManagerCS.GetDeviceParamRangeDouble);
                    }
                    else if (typeof(T) == typeof(int))
                    {
                        return (HwVal<T>.HWGetCallbackRange)((object)(HwVal<int>.HWGetCallbackRange)ResourceManagerCS.GetDeviceParamRangeInt);
                    }
                    else
                    {
                        return null;
                    }
            }

            return null;
        }

        internal static HwVal<T>.HWGetParamReadOnlyCallback GetParamReadOnlyMethod(HWType hwType)
        {
            switch (hwType)
            {
                case HWType.Camera:
                    return (HwVal<T>.HWGetParamReadOnlyCallback)ResourceManagerCS.GetCameraParamReadOnly;
                case HWType.Device:
                    return (HwVal<T>.HWGetParamReadOnlyCallback)ResourceManagerCS.GetDeviceParamReadOnly;
            }

            return null;
        }

        internal static HwVal<T>.HWSetParamCallback SetParamMethod(HWType hwType)
        {
            switch (hwType)
            {
                case HWType.Camera:
                    if (typeof(T) == typeof(double))
                    {
                        return (HwVal<T>.HWSetParamCallback)((object)(HwVal<double>.HWSetParamCallback)SetCameraParamDouble);
                    }
                    else if (typeof(T) == typeof(int))
                    {
                        return (HwVal<T>.HWSetParamCallback)((object)(HwVal<int>.HWSetParamCallback)SetCameraParamInt);
                    }
                    else
                    {
                        return null;
                    }
                case HWType.Device:
                    if (typeof(T) == typeof(double))
                    {
                        return (HwVal<T>.HWSetParamCallback)((object)(HwVal<double>.HWSetParamCallback)ResourceManagerCS.SetDeviceParamDouble);
                    }
                    else if (typeof(T) == typeof(int))
                    {
                        return (HwVal<T>.HWSetParamCallback)((object)(HwVal<int>.HWSetParamCallback)ResourceManagerCS.SetDeviceParamInt);
                    }
                    else
                    {
                        return null;
                    }
            }

            return null;
        }

        private static int SetCameraParamDouble(int cameraSelection, int paramId, double param, int exeOrWait)
        {
            return ResourceManagerCS.SetCameraParamDouble(cameraSelection, paramId, param);
        }

        private static int SetCameraParamInt(int cameraSelection, int paramId, int param, int exeOrWait)
        {
            return ResourceManagerCS.SetCameraParamInt(cameraSelection, paramId, param);
        }

        #endregion Methods
    }
}