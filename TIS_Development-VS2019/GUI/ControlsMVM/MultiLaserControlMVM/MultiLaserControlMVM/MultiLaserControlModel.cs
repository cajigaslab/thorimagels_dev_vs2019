namespace MultiLaserControl.Model
{
    using System.Runtime.InteropServices;

    using ThorSharedTypes;

    public class MultiLaserControlModel
    {
        #region Fields

        public int _enableLaser1;
        public int _enableLaser2;
        public int _enableLaser3;
        public int _enableLaser4;

        double _laser1StoredPower;
        double _laser2StoredPower;
        double _laser3StoredPower;
        double _laser4StoredPower;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the MultiLaserControlModel class
        /// </summary>
        public MultiLaserControlModel()
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
                _enableLaser1 = temp;
                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_ENABLE, value, false);
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

        public double Laser1Power
        {
            get
            {

                GetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POWER, ref _laser1StoredPower);
                return _laser1StoredPower;
            }
            set
            {
                SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POWER, value, false);
            }
        }

        //Used as label for lasers that can query wavelength
        public int Laser1Wavelength
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_WAVELENGTH, ref temp);

                return temp;
            }
        }

        public int Laser2Enable
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_ENABLE, ref temp);
                _enableLaser2 = temp;
                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_ENABLE, value, false);
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

                GetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_POWER, ref _laser2StoredPower);
                return _laser2StoredPower;
            }
            set
            {
                SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_POWER, value, false);
            }
        }

        //Used as label for lasers that can query wavelength
        public int Laser2Wavelength
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER2, (int)IDevice.Params.PARAM_LASER2_WAVELENGTH, ref temp);

                return temp;
            }
        }

        public int Laser3Enable
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_ENABLE, ref temp);
                _enableLaser3 = temp;
                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_ENABLE, value, false);
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

                GetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_POWER, ref _laser3StoredPower);
                return _laser3StoredPower;
            }
            set
            {
                SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_POWER, value, false);
            }
        }

        //Used as label for lasers that can query wavelength
        public int Laser3Wavelength
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER3, (int)IDevice.Params.PARAM_LASER3_WAVELENGTH, ref temp);

                return temp;
            }
        }

        public int Laser4Enable
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_ENABLE, ref temp);
                _enableLaser4 = temp;
                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_ENABLE, value, false);
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

                GetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_POWER, ref _laser4StoredPower);
                return _laser4StoredPower;
            }
            set
            {
                SetDeviceParamDouble((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_POWER, value, false);
            }
        }

        //Used as label for lasers that can query wavelength
        public int Laser4Wavelength
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER4, (int)IDevice.Params.PARAM_LASER4_WAVELENGTH, ref temp);

                return temp;
            }
        }

        //Turns analog mode on for all lasers
        public int LaserAllAnalog
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER_ALL_ANALOG_MODE, ref temp); //No getter in lower level right now

                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER_ALL_ANALOG_MODE, value, false);
            }
        }

        public int LaserAllEnable
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER_ALL_ENABLE, ref temp); //No getter in lower level right now

                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER_ALL_ENABLE, value, false);
            }
        }

        //Turns digital lines on for all lasers
        public int LaserAllTTL
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER_ALL_TTL_MODE, ref temp); //No getter in lower level right now

                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER_ALL_TTL_MODE, value, false);
            }
        }

        //Used for OTM
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