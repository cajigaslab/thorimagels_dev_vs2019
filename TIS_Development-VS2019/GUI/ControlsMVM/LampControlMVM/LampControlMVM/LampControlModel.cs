namespace LampControl.Model
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

    public class LampControlModel
    {
        #region Fields

        private bool _getFirstLampPos = true;
        private bool _isExternalTrigger;
        private double _lampPosition;
        private long _lampTerminal;
        private bool _isLamp1Enable;
        private bool _isLamp2Enable;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the LampControlModel class
        /// </summary>
        public LampControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public bool IsExternalTrigger
        {
            get
            {
                int mode = 0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LAMP_MODE, ref mode);
                _isExternalTrigger = mode == 7;
                return _isExternalTrigger;
            }
            set
            {
                if (value != _isExternalTrigger)
                {
                    _isExternalTrigger = value;
                    ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LAMP_MODE, _isExternalTrigger ? 7 : 2, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                }
            }
        }

        public double LampPosition
        {
            get
            {
                if (_getFirstLampPos)
                {
                    ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LAMP_POS, ref _lampPosition);
                    _getFirstLampPos = false;
                }
                return _lampPosition;
            }
            set
            {
                _lampPosition = value;
                ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LAMP_POS, _lampPosition, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
            }
        }

        public long LampTerminal
        {
            get
            {
                int _terminal=0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LAMP_TERMINAL, ref _terminal);
                _lampTerminal = (long)_terminal;

                return _lampTerminal;
            }
            set
            {
                _lampTerminal = value;
                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LAMP_TERMINAL, (int)_lampTerminal, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
            }
        }

        public bool Lamp1Enable
        {
            get
            {
                int _terminal = 0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LAMP1_CONNECTION, ref _terminal);

                
                
                _isLamp1Enable = (_terminal>0)?true:false;

                return _isLamp1Enable;
            }
            set
            {
                _isLamp1Enable = value;
                
            }
        }

        public bool Lamp2Enable
        {
            get
            {
                int _terminal = 0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BFLAMP, (int)IDevice.Params.PARAM_LAMP2_CONNECTION, ref _terminal);
                _isLamp2Enable = (_terminal > 0) ? true : false;

                return _isLamp2Enable;
            }
            set
            {
                _isLamp2Enable = value;
                
            }
        }


        #endregion Properties
    }
}