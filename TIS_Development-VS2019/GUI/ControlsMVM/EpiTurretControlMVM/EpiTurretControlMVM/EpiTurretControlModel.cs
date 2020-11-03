namespace EpiTurretControl.Model
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

    public class EpiTurretControlModel
    {
        #region Constructors

        /// <summary>
        /// Create a new instance of the EpiTurretControlModel class
        /// </summary>
        public EpiTurretControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public int EpiTurretPos
        {
            get
            {
                int val = 0;

                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_EPITURRET, (int)IDevice.Params.PARAM_FW_DIC_POS, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_EPITURRET, (int)IDevice.Params.PARAM_EPI_TURRET_POS, value, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
            }
        }

        #endregion Properties
    }
}