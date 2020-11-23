namespace LightEngineControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using ThorLogging;

    using ThorSharedTypes;

    public class LightEngineControlModel
    {
        #region Fields

        private const int CHROLIS_RANGE_CONVERSION = 10;
        private const int LENGTH = 256;

        private String _lED1ControlName = default(String);
        private Int32 _lED1LightColor = default(Int32);
        private Double _lED1Power = 0.0;
        private Boolean _lED1PowerState = default(Boolean);
        private Int32 _lED1SockelID = default(Int32);
        private Double _lED1Temperature = default(Double);
        private String _lED2ControlName = default(String);
        private Int32 _lED2LightColor = default(Int32);
        private Double _lED2Power = 0.0;
        private Boolean _lED2PowerState = default(Boolean);
        private Int32 _lED2SockelID = default(Int32);
        private Double _lED2Temperature = default(Double);
        private String _lED3ControlName = default(String);
        private Int32 _lED3LightColor = default(Int32);
        private Double _lED3Power = 0.0;
        private Boolean _lED3PowerState = default(Boolean);
        private Int32 _lED3SockelID = default(Int32);
        private Double _lED3Temperature = default(Double);
        private String _lED4ControlName = default(String);
        private Int32 _lED4LightColor = default(Int32);
        private Double _lED4Power = 0.0;
        private Boolean _lED4PowerState = default(Boolean);
        private Int32 _lED4SockelID = default(Int32);
        private Double _lED4Temperature = default(Double);
        private String _lED5ControlName = default(String);
        private Int32 _lED5LightColor = default(Int32);
        private Double _lED5Power = 0.0;
        private Boolean _lED5PowerState = default(Boolean);
        private Int32 _lED5SockelID = default(Int32);
        private Double _lED5Temperature = default(Double);
        private String _lED6ControlName = default(String);
        private Int32 _lED6LightColor = default(Int32);
        private Double _lED6Power = 0.0;
        private Boolean _lED6PowerState = default(Boolean);
        private Int32 _lED6SockelID = default(Int32);
        private Double _lED6Temperature = default(Double);
        private Double _masterBrightness = 100.0;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the LightEngineControlModel class
        /// </summary>
        public LightEngineControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public Boolean EnableDisableLEDs
        {
            get
            {
                int state = 0;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LEDS_ENABLE_DISABLE, ref state);
                return state > 0;
            }
            set
            {
                int state = (Boolean)value ? 1 : 0;
                int retCode = ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LEDS_ENABLE_DISABLE, state, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
            }
        }

        public String LED1ControlName
        {
            get
            {
                //StringBuilder led1Name = new StringBuilder(LENGTH);
                //int retCode = ResourceManagerCS.GetDeviceParamString((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED1_HEADS_COLOR_NAME, led1Name, LENGTH);
                //_lED1ControlName = led1Name.ToString();
                double peakWavelength = 0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED1_WAVELENGTH, ref peakWavelength);
                _lED1ControlName = peakWavelength.ToString();
                return _lED1ControlName;
            }
            set
            {
            }
        }

        public int LED1LightColor
        {
            get
            {
                int colorCode = -1;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED1_LIGHT_COLOR, ref colorCode);
                _lED1LightColor = colorCode;
                return _lED1LightColor;
            }
            set
            {
            }
        }

        public Double LED1Power
        {
            get
            {
                Double power = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble( (int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED1_POWER, ref power);
                _lED1Power = Math.Round(power / CHROLIS_RANGE_CONVERSION, 1);
                return _lED1Power;
            }
            set
            {
                //Range for the led power goes from 0 - 1000
                double val = value * CHROLIS_RANGE_CONVERSION;
                int retCode = ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED1_POWER, val, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED1Power = val;
            }
        }

        public Boolean LED1PowerState
        {
            get
            {
                //int state = -1;
                //int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED1_POWER_STATE, ref state);
                //_lED1PowerState = (state > 0);
                return _lED1PowerState;
            }
            set
            {
                int state = (Boolean)value ? 1 : 0;
                int retCode = ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED1_POWER_STATE, state, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED1PowerState = value;
            }
        }

        public int LED1SockelID
        {
            get
            {
                int sockelID = -1;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED1_SOCKEL_ID, ref sockelID);
                _lED1SockelID = sockelID;
                return _lED1SockelID;
            }
            set
            {
            }
        }

        public Double LED1Temperature
        {
            get
            {
                double temperature = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED1_TEMP, ref temperature);
                _lED1Temperature = Math.Round(temperature, 1);
                return _lED1Temperature;
            }
            set
            {
            }
        }

        public String LED2ControlName
        {
            get
            {
                //StringBuilder led2Name = new StringBuilder(LENGTH);
                //int retCode = ResourceManagerCS.GetDeviceParamString((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED2_HEADS_COLOR_NAME, led2Name, LENGTH);
                //_lED2ControlName = led2Name.ToString();
                double peakWavelength = 0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED2_WAVELENGTH, ref peakWavelength);
                _lED2ControlName = peakWavelength.ToString();
                return _lED2ControlName;
            }
            set
            {
                _lED2ControlName = value;
            }
        }

        public int LED2LightColor
        {
            get
            {
                int colorCode = -1;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED2_LIGHT_COLOR, ref colorCode);
                _lED2LightColor = colorCode;
                return _lED2LightColor;
            }
            set
            {
            }
        }

        public Double LED2Power
        {
            get
            {
                Double power = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED2_POWER, ref power);
                _lED2Power = Math.Round(power / CHROLIS_RANGE_CONVERSION, 1);
                return _lED2Power;
            }
            set
            {
                //Range for the led power goes from 0 - 1000
                double val = value * CHROLIS_RANGE_CONVERSION;
                int retCode = ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED2_POWER, val, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED2Power = val;
            }
        }

        public Boolean LED2PowerState
        {
            get
            {
                //int state = -1;
                //int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED2_POWER_STATE, ref state);
                //_lED2PowerState = (state > 0);
                return _lED2PowerState;
            }
            set
            {
                int state = (Boolean)value ? 1 : 0;
                int retCode = ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED2_POWER_STATE, state, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED2PowerState = value;
            }
        }

        public int LED2SockelID
        {
            get
            {
                return _lED2SockelID;
            }
            set
            {
            }
        }

        public Double LED2Temperature
        {
            get
            {
                Double temperature = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED2_TEMP, ref temperature);
                _lED2Temperature = Math.Round(temperature, 1);
                return _lED2Temperature;
            }
            set
            {
            }
        }

        public String LED3ControlName
        {
            get
            {
                //StringBuilder led3Name = new StringBuilder(LENGTH);
                //int retCode = ResourceManagerCS.GetDeviceParamString((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED3_HEADS_COLOR_NAME, led3Name, LENGTH);
                //_lED3ControlName = led3Name.ToString();
                double peakWavelength = 0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED3_WAVELENGTH, ref peakWavelength);
                _lED3ControlName = peakWavelength.ToString();
                return _lED3ControlName;
            }
            set
            {
                _lED3ControlName = value;
            }
        }

        public int LED3LightColor
        {
            get
            {
                int colorCode = -1;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED3_LIGHT_COLOR, ref colorCode);
                _lED3LightColor = colorCode;
                return _lED3LightColor;
            }
            set
            {
            }
        }

        public Double LED3Power
        {
            get
            {
                Double power = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED3_POWER, ref power);
                _lED3Power = Math.Round(power / CHROLIS_RANGE_CONVERSION, 1);
                return _lED3Power;
            }
            set
            {
                //Range for the led power goes from 0 - 1000
                double val = value * CHROLIS_RANGE_CONVERSION;
                int retCode = ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED3_POWER, val, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED3Power = val;
            }
        }

        public Boolean LED3PowerState
        {
            get
            {
                //int state = -1;
                //int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED3_POWER_STATE, ref state);
                //_lED3PowerState = (state > 0);
                return _lED3PowerState;
            }
            set
            {
                int state = (Boolean)value ? 1 : 0;
                int retCode = ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED3_POWER_STATE, state, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED3PowerState = value;
            }
        }

        public int LED3SockelID
        {
            get
            {
                int sockelID = -1;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED3_SOCKEL_ID, ref sockelID);
                _lED3SockelID = sockelID;
                return _lED3SockelID;
            }
            set
            {
            }
        }

        public Double LED3Temperature
        {
            get
            {
                Double temperature = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED3_TEMP, ref temperature);
                _lED3Temperature = Math.Round(temperature, 1);
                return _lED3Temperature;
            }
            set
            {
            }
        }

        public String LED4ControlName
        {
            get
            {
                //StringBuilder led4Name = new StringBuilder(LENGTH);
                //int retCode = ResourceManagerCS.GetDeviceParamString((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED4_HEADS_COLOR_NAME, led4Name, LENGTH);
                //_lED4ControlName = led4Name.ToString();
                double peakWavelength = 0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED4_WAVELENGTH, ref peakWavelength);
                _lED4ControlName = peakWavelength.ToString();
                return _lED4ControlName;
            }
            set
            {
                _lED4ControlName = value;
            }
        }

        public int LED4LightColor
        {
            get
            {
                int colorCode = -1;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED4_LIGHT_COLOR, ref colorCode);
                _lED4LightColor = colorCode;
                return _lED4LightColor;
            }
            set
            {
            }
        }

        public Double LED4Power
        {
            get
            {
                Double power = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED4_POWER, ref power);
                _lED4Power = Math.Round(power / CHROLIS_RANGE_CONVERSION, 1);
                return _lED4Power;
            }
            set
            {
                //Range for the led power goes from 0 - 1000
                double val = value * CHROLIS_RANGE_CONVERSION;
                int retCode = ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED4_POWER, val, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED4Power = val;
            }
        }

        public Boolean LED4PowerState
        {
            get
            {
                //int state = -1;
                //int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED4_POWER_STATE, ref state);
                //_lED4PowerState = (state > 0);
                return _lED4PowerState;
            }
            set
            {
                int state = (Boolean)value ? 1 : 0;
                int retCode = ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED4_POWER_STATE, state, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED4PowerState = value;
            }
        }

        public int LED4SockelID
        {
            get
            {
                int sockelID = -1;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED4_SOCKEL_ID, ref sockelID);
                _lED4SockelID = sockelID;
                return _lED4SockelID;
            }
            set
            {
            }
        }

        public Double LED4Temperature
        {
            get
            {
                Double temperature = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED4_TEMP, ref temperature);
                _lED4Temperature = Math.Round(temperature, 1);
                return _lED4Temperature;
            }
            set
            {
            }
        }

        public String LED5ControlName
        {
            get
            {
                //StringBuilder led5Name = new StringBuilder(LENGTH);
                //int retCode = ResourceManagerCS.GetDeviceParamString((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED5_HEADS_COLOR_NAME, led5Name, LENGTH);
                //_lED5ControlName = led5Name.ToString();
                double peakWavelength = 0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED5_WAVELENGTH, ref peakWavelength);
                _lED5ControlName = peakWavelength.ToString();
                return _lED5ControlName;
            }
            set
            {
                _lED5ControlName = value;
            }
        }

        public int LED5LightColor
        {
            get
            {
                int colorCode = -1;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED5_LIGHT_COLOR, ref colorCode);
                _lED5LightColor = colorCode;
                return _lED5LightColor;
            }
            set
            {
            }
        }

        public Double LED5Power
        {
            get
            {
                Double power = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED5_POWER, ref power);
                _lED5Power = Math.Round(power / CHROLIS_RANGE_CONVERSION, 1);
                return _lED5Power;
            }
            set
            {
                //Range for the led power goes from 0 - 1000
                double val = value * CHROLIS_RANGE_CONVERSION;
                int retCode = ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED5_POWER, val, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED5Power = val;
            }
        }

        public Boolean LED5PowerState
        {
            get
            {
                //int state = -1;
                //int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED5_POWER_STATE, ref state);
                //_lED5PowerState = (state > 0);
                return _lED5PowerState;
            }
            set
            {
                int state = (Boolean)value ? 1 : 0;
                int retCode = ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED5_POWER_STATE, state, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED5PowerState = value;
            }
        }

        public int LED5SockelID
        {
            get
            {
                int sockelID = -1;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED5_SOCKEL_ID, ref sockelID);
                _lED5SockelID = sockelID;
                return _lED5SockelID;
            }
            set
            {
            }
        }

        public Double LED5Temperature
        {
            get
            {
                Double temperature = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED5_TEMP, ref temperature);
                _lED5Temperature = Math.Round(temperature, 1);
                return _lED5Temperature;
            }
            set
            {
            }
        }

        public String LED6ControlName
        {
            get
            {
                //StringBuilder led6Name = new StringBuilder(LENGTH);
                //int retCode = ResourceManagerCS.GetDeviceParamString((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED6_HEADS_COLOR_NAME, led6Name, LENGTH);
                //_lED6ControlName = led6Name.ToString();
                double peakWavelength = 0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED6_WAVELENGTH, ref peakWavelength);
                _lED6ControlName = peakWavelength.ToString();
                return _lED6ControlName;
            }
            set
            {
                _lED6ControlName = value;
            }
        }

        public int LED6LightColor
        {
            get
            {
                int colorCode = -1;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED6_LIGHT_COLOR, ref colorCode);
                _lED6LightColor = colorCode;
                return _lED6LightColor;
            }
            set
            {
            }
        }

        public Double LED6Power
        {
            get
            {
                Double power = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED6_POWER, ref power);
                _lED6Power = Math.Round(power / CHROLIS_RANGE_CONVERSION, 1);
                return _lED6Power;
            }
            set
            {
                //Range for the led power goes from 0 - 1000
                double val = value * CHROLIS_RANGE_CONVERSION;
                int retCode = ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED6_POWER, val, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED6Power = val;
            }
        }

        public Boolean LED6PowerState
        {
            get
            {
                //int state = -1;
                //int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED6_POWER_STATE, ref state);
                //_lED6PowerState = (state > 0);
                return _lED6PowerState;
            }
            set
            {
                int state = (Boolean)value ? 1 : 0;
                int retCode = ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED6_POWER_STATE, state, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _lED6PowerState = value;
            }
        }

        public int LED6SockelID
        {
            get
            {
                int sockelID = -1;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED6_SOCKEL_ID, ref sockelID);
                _lED6SockelID = sockelID;
                return _lED6SockelID;
            }
            set
            {
            }
        }

        public Double LED6Temperature
        {
            get
            {
                Double temperature = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LED6_TEMP, ref temperature);
                _lED6Temperature = Math.Round(temperature, 1);
                return _lED6Temperature;
            }
            set
            {
            }
        }

        public Double MasterBrightness
        {
            get
            {
                Double masterPower = -1.0;
                int retCode = ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LEDS_LINEAR_VALUE, ref masterPower);
                _masterBrightness = Math.Round(masterPower / CHROLIS_RANGE_CONVERSION, 1);
                return _masterBrightness;
            }
            set
            {
                //Range for the led power goes from 0 - 1000
                double val = value * CHROLIS_RANGE_CONVERSION;
                int retCode = ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LEDS_LINEAR_VALUE, val, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                _masterBrightness = val;
            }
        }

        public bool UpdateTemperatures
        {
            get
            {
                int returnValue = 0;
                int retCode = ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_UPDATE_TEMPERATURES, ref returnValue);
                return (1 == returnValue);
            }
        }

        #endregion Properties

        #region Nested Types

        private class Native
        {
            #region Fields

            private const String importDll = ".\\Thor6WL.dll";

            #endregion Fields
        }

        #endregion Nested Types
    }
}