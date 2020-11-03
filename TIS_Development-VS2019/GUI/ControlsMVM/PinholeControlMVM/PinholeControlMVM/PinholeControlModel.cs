namespace PinholeControl.Model
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

    public class PinholeControlModel
    {
        #region Constructors

        /// <summary>
        /// Create a new instance of the PinholeControlModel class
        /// </summary>
        public PinholeControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public int LSMPinholeAlignmentPosition
        {
            get
            {
                int val = 0;

                ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_ALIGNMENT_POS_CURRENT, ref val);

                return (int)val;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_ALIGNMENT_MODE, 2, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_ALIGNMENT_POS, value, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);

                //disable the mode
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_ALIGNMENT_MODE, 0, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
            }
        }

        public int LSMSavePinholeAlignmentPosition
        {
            get
            {
                int val = 0;

                ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_ALIGNMENT_POS_CURRENT, ref val);

                return (int)val;
            }
            set
            {

                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_ALIGNMENT_MODE, 1, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_ALIGNMENT_POS, value, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_POS, 0, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                //disable the mode
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_ALIGNMENT_MODE, 0, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
            }
        }

        public int PinholeMax
        {
            get
            {
                int pinholeMin = 0;
                int pinholeMax = 0;
                int pinholeDefault = 0;

                ThorSharedTypes.ResourceManagerCS.GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_ALIGNMENT_POS, ref pinholeMin, ref pinholeMax, ref pinholeDefault);

                return pinholeMax;
            }
        }

        public int PinholeMin
        {
            get
            {
                int pinholeMin = 0;
                int pinholeMax = 0;
                int pinholeDefault = 0;

                ThorSharedTypes.ResourceManagerCS.GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_ALIGNMENT_POS, ref pinholeMin, ref pinholeMax, ref pinholeDefault);

                return pinholeMin;
            }
        }

        public int PinholePosition
        {
            get
            {
                int val = 0;
                ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_POS_CURRENT, ref val);
                return (int)val;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_PINHOLEWHEEL, (int)IDevice.Params.PARAM_PINHOLE_POS, value, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
            }
        }

        #endregion Properties
    }
}