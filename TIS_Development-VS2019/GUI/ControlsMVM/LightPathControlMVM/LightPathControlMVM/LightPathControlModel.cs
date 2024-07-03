namespace LightPathControl.Model
{
    using ThorLogging;

    using ThorSharedTypes;

    public class LightPathControlModel
    {
        #region Fields

        private int _lightPathCamEnable = 0;
        private int _lightPathGGEnable = 0;
        private int _lightPathGREnable = 1;
        private int _nddPosition;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the LightPathControlModel class
        /// </summary>
        public LightPathControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public int InvertedLightPathPos
        {
            get
            {
                int val = -1;
                if(0 == ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_INVERTED_POS, ref val))
                {
                    return -1;
                }
                return val;
            }
            set
            {
                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_INVERTED_POS, value, 0);
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
                if (1 == ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_NDD_AVAILABLE, ref val))
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
                if (1 == ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_CAMERA, ref val))   //0: LIGHTPATH_GG, 1: LIGHTPATH_GR, 2: LIGHTPATH_CAMERA
                {
                    _lightPathCamEnable = (int)val;
                }
                return _lightPathCamEnable;
            }
            set
            {
                if (1 == ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_CAMERA, value, 0))
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
                if (1 == ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_GG, ref val))
                {
                    _lightPathGGEnable = (int)val;
                }
                return _lightPathGGEnable;
            }
            set
            {
                if (1 == ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_GG, value, 0))
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
                if (1 == ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_GR, ref val))
                {
                    _lightPathGREnable = (int)val;
                }
                return _lightPathGREnable;
            }
            set
            {
                if (1 == ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_GR, value, 0))
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
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_SCOPE_TYPE, ref val);
                return val;
            }
        }

        public int PositionNDD
        {
            get
            {
                int val = 0;
                if (1 == ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_NDD, ref val))
                {
                    _nddPosition = (int)val;
                }
                return _nddPosition;
            }
            set
            {
                if (1 == ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_LIGHTPATH, (int)IDevice.Params.PARAM_LIGHTPATH_NDD, value, 0))
                {
                    _nddPosition = value;
                }
            }
        }

        public bool SecondaryGGAvailableImaging
        {
            get
            {
                int val = 0;
                if (1 == ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_SECONDARY_GG_AVAILABLE, ref val))
                {
                    return val == 1;
                }
                return false;
            }
        }

        public bool SecondaryGGAvailableStim
        {
            get
            {
                int val = 0;
                if (1 == ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_SECONDARY_GG_AVAILABLE, ref val))
                {
                    return val == 1;
                }
                return false;
            }
        }

        public int SelectedImagingGG
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_SELECTED_IMAGING_GG, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_SELECTED_IMAGING_GG, value);
            }
        }

        public int SelectedStimGG
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_SELECTED_STIM_GG, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_SELECTED_STIM_GG, value);
            }
        }

        #endregion Properties
    }
}