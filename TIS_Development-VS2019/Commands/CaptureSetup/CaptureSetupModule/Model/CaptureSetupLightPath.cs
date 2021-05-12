namespace CaptureSetupDll.Model
{
    using System;
    using System.Collections.Generic;
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

    public partial class CaptureSetup : INotifyPropertyChanged
    {
        #region Fields

        private int _lightPathCamEnable;
        private int _lightPathGGEnable;
        private int _lightPathGREnable;
        private int _nddPosition;

        #endregion Fields

        #region Properties

        public int InvertedLightPathPos
        {
            get
            {
                int val = -1;
                GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_INVERTED_POS, ref val);
                return val;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_INVERTED_POS, value, false);
            }
        }

        public bool InvertedLpCenterEnable
        {
            get
            {
                return (InvertedLightPathPos == 1) ? true : false;
            }
            set
            {
                if (value)
                    InvertedLightPathPos = 1;
            }
        }

        public bool InvertedLpLeftEnable
        {
            get
            {
                return (InvertedLightPathPos == 0) ? true : false;
            }
            set
            {
                if (value)
                    InvertedLightPathPos = 0;
            }
        }

        public bool InvertedLpRightEnable
        {
            get
            {
                return (InvertedLightPathPos == 2) ? true : false;
            }
            set
            {
                if (value)
                    InvertedLightPathPos = 2;
            }
        }

        public bool IsNDDAvailable
        {
            get
            {
                int val = 0;
                if (1 == GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_NDD_AVAILABLE, ref val))
                {
                    return (1 == val);
                }
                return false;
            }
        }

        public int LightPathCamEnable
        {
            get
            {
                int val = 0;
                if (1 == GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_CAMERA, ref val))   //0: LIGHTPATH_GG, 1: LIGHTPATH_GR, 2: LIGHTPATH_CAMERA
                {
                    _lightPathCamEnable = (int)val;
                }
                return _lightPathCamEnable;
            }
            set
            {
                if (1 == SetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_CAMERA, value, false))
                {
                    _lightPathCamEnable = value;
                }
            }
        }

        public int LightPathGGEnable
        {
            get
            {
                int val = 0;
                if (1 == GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_GG, ref val))
                {
                    _lightPathGGEnable = (int)val;
                }
                return _lightPathGGEnable;
            }
            set
            {
                if (1 == SetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_GG, value, false))
                {
                    _lightPathGGEnable = value;
                }
            }
        }

        public int LightPathGREnable
        {
            get
            {
                int val = 0;
                if (1 == GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_GR, ref val))
                {
                    _lightPathGREnable = (int)val;
                }
                return _lightPathGREnable;
            }
            set
            {
                if (1 == SetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_GR, value, false))
                {
                    _lightPathGREnable = value;
                }
            }
        }

        public int LightPathScopeType
        {
            get
            {
                int val = 0;
                GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_SCOPE_TYPE, ref val);
                return val;
            }
        }

        public int PositionNDD
        {
            get
            {
                int val = 0;
                if (1 == GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_NDD, ref val))
                {
                    _nddPosition = (int)val;
                }
                return _nddPosition;
            }
            set
            {
                if (1 == SetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_NDD, value, false))
                {
                    _nddPosition = value;
                }
            }
        }

        #endregion Properties
    }
}