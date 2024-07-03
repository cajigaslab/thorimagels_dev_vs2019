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

        public double DDSAmplitude0
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_REVERB_DDS0_AMPLITUDE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_REVERB_DDS0_AMPLITUDE, value);
            }
        }

        public double DDSAmplitude1
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_REVERB_DDS1_AMPLITUDE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_REVERB_DDS1_AMPLITUDE, value);
            }
        }

        public int DDSEnable
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_REVERB_DDS_ENABLE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_REVERB_DDS_ENABLE, value);
            }
        }

        public double DDSPhase0
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_REVERB_DDS0_PHASE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_REVERB_DDS0_PHASE, value);
            }
        }

        public double DDSPhase1
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_REVERB_DDS1_PHASE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_REVERB_DDS1_PHASE, value);
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

        public int LUTOffset3P
        {
            get
            {
                int temp = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_LUT_OFFSET, ref temp);
                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_LUT_OFFSET, value);
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

        public int ThreePhotonPhaseCoarse0
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_COARSE_0, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_COARSE_0, value);
            }
        }

        public int ThreePhotonPhaseCoarse1
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_COARSE_1, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_COARSE_1, value);
            }
        }

        public int ThreePhotonPhaseCoarse2
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_COARSE_2, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_COARSE_2, value);
            }
        }

        public int ThreePhotonPhaseCoarse3
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_COARSE_3, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_3P_ALIGN_COARSE_3, value);
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

        public int ThreePhotonDownsamplingRate
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_DOWNSAMPLING_RATE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_DOWNSAMPLING_RATE, value);
            }
        }

        public int EnableDownsamplingRateChange
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_ENABLE_DOWNSAMPLING_RATE_CHANGE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_3P_ENABLE_DOWNSAMPLING_RATE_CHANGE, value);
            }
        }

        #endregion Properties
    }
}