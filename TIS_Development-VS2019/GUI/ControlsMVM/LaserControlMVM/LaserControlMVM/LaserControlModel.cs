namespace LaserControl.Model
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

    public class LaserControlModel
    {
        #region Fields

        int _currentLaser1Position;
        bool _isCurLaser1PosDirt;
        double _laser1StoredPower;
        double _laser2StoredPower;
        double _laser3StoredPower;
        double _laser4StoredPower;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the LaserControlModel class
        /// </summary>
        public LaserControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public int Laser1Enable
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_ENABLE, ref temp);

                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_ENABLE, value, false);

                if (value == 1)
                {
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POWER, _laser1StoredPower, false);
                }
            }
        }

        public double Laser1Max
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;
                double tempDefault = 0;

                GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POWER, ref tempMin, ref tempMax, ref tempDefault);

                return tempMax;
            }
        }

        public double Laser1Min
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;
                double tempDefault = 0;

                GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POWER, ref tempMin, ref tempMax, ref tempDefault);

                return tempMin;
            }
        }

        public int Laser1Position
        {
            get
            {
                int temp = 0;

                if (_isCurLaser1PosDirt)
                {
                    GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POS, ref temp);

                    _currentLaser1Position = temp;
                    _isCurLaser1PosDirt = false;
                }
                else
                {
                    temp = _currentLaser1Position;
                }

                return temp;
            }
            set
            {
                _isCurLaser1PosDirt = true;

                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POS, value, false);
            }
        }

        public double Laser1Power
        {
            get
            {
                //If laser is enabled, get from device
                if (1 == Laser1Enable)
                {
                    double temp = 0;

                    GetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POWER, ref temp);

                    return temp;
                }
                else //If offline, get stored value
                {
                    return _laser1StoredPower;
                }
            }
            set
            {
                _laser1StoredPower = value;
                if (1 == Laser1Enable)
                {
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POWER, value, false);
                }
            }
        }

        public int Laser2Enable
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_ENABLE, ref temp);

                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_ENABLE, value, false);

                if (value == 1)
                {
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_POWER, _laser2StoredPower, false);
                }
            }
        }

        public double Laser2Max
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;
                double tempDefault = 0;

                GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_POWER, ref tempMin, ref tempMax, ref tempDefault);

                return tempMax;
            }
        }

        public double Laser2Min
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;
                double tempDefault = 0;

                GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_POWER, ref tempMin, ref tempMax, ref tempDefault);

                return tempMin;
            }
        }

        public double Laser2Power
        {
            get
            {
                //If laser is enabled, get from device
                if (2 == Laser2Enable)
                {
                    double temp = 0;

                    GetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_POWER, ref temp);

                    return temp;
                }
                else //If offline, get stored value
                {
                    return _laser2StoredPower;
                }
            }
            set
            {
                _laser2StoredPower = value;
                if (1 == Laser2Enable)
                {
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_POWER, value, false);
                }
            }
        }

        public int Laser3Enable
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_ENABLE, ref temp);

                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_ENABLE, value, false);

                if (value == 1)
                {
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_POWER, _laser3StoredPower, false);
                }
            }
        }

        public double Laser3Max
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;
                double tempDefault = 0;

                GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_POWER, ref tempMin, ref tempMax, ref tempDefault);

                return tempMax;
            }
        }

        public double Laser3Min
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;
                double tempDefault = 0;

                GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_POWER, ref tempMin, ref tempMax, ref tempDefault);

                return tempMin;
            }
        }

        public double Laser3Power
        {
            get
            {
                //If laser is enabled, get from device
                if (3 == Laser3Enable)
                {
                    double temp = 0;

                    GetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_POWER, ref temp);

                    return temp;
                }
                else //If offline, get stored value
                {
                    return _laser3StoredPower;
                }
            }
            set
            {
                _laser3StoredPower = value;
                if (1 == Laser3Enable)
                {
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_POWER, value, false);
                }
            }
        }

        public int Laser4Enable
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_ENABLE, ref temp);

                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_ENABLE, value, false);

                if (value == 1)
                {
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_POWER, _laser4StoredPower, false);
                }
            }
        }

        public double Laser4Max
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;
                double tempDefault = 0;

                GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_POWER, ref tempMin, ref tempMax, ref tempDefault);

                return tempMax;
            }
        }

        public double Laser4Min
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;
                double tempDefault = 0;

                GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_POWER, ref tempMin, ref tempMax, ref tempDefault);

                return tempMin;
            }
        }

        public double Laser4Power
        {
            get
            {
                //If laser is enabled, get from device
                if (4 == Laser4Enable)
                {
                    double temp = 0;

                    GetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_POWER, ref temp);

                    return temp;
                }
                else //If offline, get stored value
                {
                    return _laser4StoredPower;
                }
            }
            set
            {
                _laser4StoredPower = value;
                if (1 == Laser4Enable)
                {
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_POWER, value, false);
                }
            }
        }

        public int MainLaserIndex
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER_POWER, ref temp);

                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER_POWER, value, false);
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamDouble")]
        private static extern int GetDeviceParamDouble(int deviceSelection, int paramId, ref double param);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamLong")]
        private static extern int GetDeviceParamInt(int deviceSelection, int paramId, ref int param);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamRangeDouble")]
        private static extern int GetDeviceParamRangeDouble(int deviceSelection, int paramId, ref double valMin, ref double valMax, ref double valDefault);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamDouble")]
        private static extern int SetDeviceParamDouble(int deviceSelection, int paramId, double param, bool wait);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamLong")]
        private static extern int SetDeviceParamInt(int deviceSelection, int paramId, int param, bool wait);

        #endregion Methods
    }
}