namespace ZControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
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

    class ZControlModel : ThorSharedTypes.VMBase
    {
        #region Fields

        private CustomCollection<bool> _enableRead = new CustomCollection<bool>(new bool[3] { true, true, true }); //Z, Z2, R
        private bool _isZStackCapturing = false;
        CustomCollection<DateTime> _lastUpdateTime = new CustomCollection<DateTime>(new DateTime[3] { DateTime.Now, DateTime.Now, DateTime.Now }); //Z, Z2, R
        private double _rPosition = 0.0;
        private double _z2Position = 0.0;
        private double _zPosition = 0.0;

        #endregion Fields

        #region Constructors

        public ZControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public CustomCollection<bool> EnableRead
        {
            get { return _enableRead; }
        }

        public bool IsZStackCapturing
        {
            get
            {
                return _isZStackCapturing;
            }
            set
            {
                _isZStackCapturing = value;
                OnPropertyChanged("IsZStackCapturing");
            }
        }

        public CustomCollection<DateTime> LastUpdateTime
        {
            get { return _lastUpdateTime; }
            set { _lastUpdateTime = value; }
        }

        public double RMax
        {
            get
            {
                double rMax = 0;
                double rMinPos = 0;
                double rMaxPos = 0;
                double rDefaultPos = 0;

                if (1 == ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_RSTAGE, (int)IDevice.Params.PARAM_R_POS, ref rMinPos, ref rMaxPos, ref rDefaultPos))
                {
                    rMax = rMaxPos;

                }

                return rMax;
            }
        }

        public double RMin
        {
            get
            {
                double rMin = 0;
                double rMinPos = 0;
                double rMaxPos = 0;
                double rDefaultPos = 0;

                if (1 == ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_RSTAGE, (int)IDevice.Params.PARAM_R_POS, ref rMinPos, ref rMaxPos, ref rDefaultPos))
                {
                    rMin = rMinPos;

                }

                return rMin;
            }
        }

        public double RPosition
        {
            get
            {
                return _rPosition;
            }
            set
            {
                double temp = value;

                _enableRead[2] = false;

                if (1 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_RSTAGE, (int)IDevice.Params.PARAM_R_POS, temp, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                {
                    temp = value;
                }
                _enableRead[2] = true;
            }
        }

        public bool RStageAvailable
        {
            get
            {
                double val = 0;
                return (1 == ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_RSTAGE, (int)IDevice.Params.PARAM_R_POS, ref val));
            }
        }

        public string UpdatePositions
        {
            set
            {
                switch (value)
                {
                    case "Z":
                        if (GetZPosition(ref _zPosition))
                        {
                            Decimal dec = new Decimal(_zPosition);

                            _zPosition = Decimal.ToDouble(Decimal.Round(dec, 4));
                        }
                        break;
                    case "Z2":
                        if (GetZ2Position(ref _z2Position))
                        {
                            Decimal dec = new Decimal(_z2Position);

                            _z2Position = Decimal.ToDouble(Decimal.Round(dec, 4));
                        }
                        break;
                    case "R":
                        if (GetRPosition(ref _rPosition))
                        {
                            Decimal dec = new Decimal(_rPosition);

                            _rPosition = Decimal.ToDouble(Decimal.Round(dec, 4));
                        }
                        break;
                }
            }
        }

        public double Z2Max
        {
            get
            {
                double zMax = 0;
                double zMinPos = 0;
                double zMaxPos = 0;
                double zDefaultPos = 0;

                if (1 == ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_POS, ref zMinPos, ref zMaxPos, ref zDefaultPos))
                {
                    zMax = zMaxPos;

                }

                return zMax;
            }
        }

        public double Z2Min
        {
            get
            {
                double zMin = 0;
                double zMinPos = 0;
                double zMaxPos = 0;
                double zDefaultPos = 0;

                if (1 == ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_POS, ref zMinPos, ref zMaxPos, ref zDefaultPos))
                {
                    zMin = zMinPos;

                }

                return zMin;
            }
        }

        public double Z2Position
        {
            get
            {
                return _z2Position;
            }
            set
            {
                double temp = value;

                //do not allow the zposition to be read while the stage is moving
                //for some z positioners (all motion) the read value will be zero while
                //the stage is moving and creates a strang behavior in the GUI with the
                //values jumping between an invalid value and the true value
                _enableRead[1] = false;

                if (1 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_POS, temp, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                {
                    _z2Position = value;
                }

                _enableRead[1] = true;
            }
        }

        public bool Z2StageAvailable
        {
            get
            {
                double val = 0;
                return (1 == ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_POS, ref val));
            }
        }

        public bool ZInvertDevice
        {
            get
            {
                int val = 0;

                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_INVERT, ref val);

                return (0 == val) ? false : true;
            }
        }

        public bool ZInvertDevice2
        {
            get
            {
                int val = 0;

                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_INVERT, ref val);

                return (0 == val) ? false : true;
            }
        }

        public int ZInvertUpDown
        {
            get;
            set;
        }

        public int ZInvertUpDown2
        {
            get;
            set;
        }

        public double ZMax
        {
            get
            {
                double zMax = 0;
                double zMinPos = 0;
                double zMaxPos = 0;
                double zDefaultPos = 0;

                if (1 == ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_POS, ref zMinPos, ref zMaxPos, ref zDefaultPos))
                {
                    zMax = zMaxPos;

                }

                return zMax;
            }
        }

        public double ZMin
        {
            get
            {
                double zMin = 0;
                double zMinPos = 0;
                double zMaxPos = 0;
                double zDefaultPos = 0;

                if (1 == ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_POS, ref zMinPos, ref zMaxPos, ref zDefaultPos))
                {
                    zMin = zMinPos;

                }

                return zMin;
            }
        }

        public double ZPosition
        {
            get
            {
                return _zPosition;
            }
            set
            {
                double temp = value;

                //do not allow the zposition to be read while the stage is moving
                //for some z positioners (all motion) the read value will be zero while
                //the stage is moving and creates a strang behavior in the GUI with the
                //values jumping between an invalid value and the true value
                _enableRead[0] = false;

                if (1 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_POS, temp, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                {
                    _zPosition = value;
                }

                _enableRead[0] = true;
            }
        }

        public string ZStage2Name
        {
            get
            {
                int MAX_NAME_LEN = 256;
                IntPtr deviceNamePtr = Marshal.AllocHGlobal(MAX_NAME_LEN);
                if (GetZStage2Name(ref deviceNamePtr, MAX_NAME_LEN))
                {
                    string deviceName = Marshal.PtrToStringAnsi(deviceNamePtr);
                    Marshal.FreeHGlobal(deviceNamePtr);
                    return deviceName;
                }
                else
                {
                    Marshal.FreeHGlobal(deviceNamePtr);
                    return string.Empty;
                }
            }
        }

        public string ZStageName
        {
            get
            {
                int MAX_NAME_LEN = 256;
                IntPtr deviceNamePtr = Marshal.AllocHGlobal(MAX_NAME_LEN);
                if (GetZStageName(ref deviceNamePtr, MAX_NAME_LEN))
                {
                    string deviceName = Marshal.PtrToStringAnsi(deviceNamePtr);
                    Marshal.FreeHGlobal(deviceNamePtr);
                    return deviceName;
                }
                else
                {
                    Marshal.FreeHGlobal(deviceNamePtr);
                    return string.Empty;
                }
            }
        }

        #endregion Properties

        #region Methods

        public int SetRZero()
        {
            return ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_RSTAGE, (int)IDevice.Params.PARAM_R_ZERO, 1, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
        }

        public int SetZ2Zero()
        {
            return ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_ZERO, 1, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
        }

        public int SetZZero()
        {
            return ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_ZERO, 1, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
        }

        public void StartZStackPreview(double zStartPos, double zStopPos, double zstageStepSize, long zstageSteps)
        {
            CaptureZStack(zStartPos, zStopPos, zstageStepSize, zstageSteps);
        }

        public int StopR()
        {
            //stop will interupt immediately no need to for status
            return ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_RSTAGE, (int)IDevice.Params.PARAM_R_STOP, 1, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
        }

        public int StopZ()
        {
            return ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_STOP, 1, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
        }

        public int StopZ2()
        {
            return ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_STOP, 1, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "CaptureZStack")]
        private static extern bool CaptureZStack(double zStartPos, double zStopPos, double zstageStepSize, long zstageSteps);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetRPosition")]
        private static extern bool GetRPosition(ref double pos);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetZ2Position")]
        private static extern bool GetZ2Position(ref double pos);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetZPosition")]
        private static extern bool GetZPosition(ref double pos);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetZStage2Name")]
        private static extern bool GetZStage2Name(ref IntPtr deviceName, int lenth);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetZStageName")]
        private static extern bool GetZStageName(ref IntPtr deviceName, int lenth);

        #endregion Methods
    }
}