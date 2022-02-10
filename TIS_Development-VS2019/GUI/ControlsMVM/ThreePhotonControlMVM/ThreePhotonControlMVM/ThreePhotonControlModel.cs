namespace ThreePhotonControl.Model
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

    public class ThreePhotonControlModel
    {
        #region Constructors

        /// <summary>
        /// Create a new instance of the ResearchControl1Model class
        /// </summary>
        public ThreePhotonControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public int AcquireDuringTurnAround
        {
            get
            {
                int temp = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_GG_ACQUIRE_DURING_TURAROUND, ref temp);
                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_GG_ACQUIRE_DURING_TURAROUND, value);
            }
        }

        public int FIR1ManualControlEnable
        {
            get
            {
                int temp = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_MANUAL_FIR1_CONTROL_ENABLE, ref temp);
                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_MANUAL_FIR1_CONTROL_ENABLE, value);
            }
        }

        public int LUTOffset3P
        {
            get
            {
                int temp = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_RESEARCH_CAMERA_100, ref temp);
                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_RESEARCH_CAMERA_100, value);
            }
        }

        public int LSMFIRFilterIndex
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_FIR_INDEX, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_FIR_INDEX, value);
            }
        }

        public int LSMFIRFilterTapIndex
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_FIR_TAP_INDEX, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_FIR_TAP_INDEX, value);
            }
        }

        public double LSMFIRFilterTapValue
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_FIR_TAP_VALUE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_FIR_TAP_VALUE, value);
            }
        }

        public int LSMNumberOfPlanes
        {
            get
            {
                int temp = 1;

                if (1 == ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_NUMBER_OF_PLANES, ref temp))
                {
                    return temp;
                }
                return 1;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_NUMBER_OF_PLANES, value);
            }
        }

        public int ThreePhotonEnable
        {
            get
            {
                int temp = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ENABLE, ref temp);
                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ENABLE, value);
            }
        }

        public double ThreePhotonFreq
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_EXTERNALCLOCKRATE, ref temp);
                return temp;
            }
        }

        public int ThreePhotonPhaseCoarse
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_COARSE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_COARSE, value);
            }
        }

        public int ThreePhotonPhaseFine
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_FINE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_FINE, value);
            }
        }

        #endregion Properties
    }
}