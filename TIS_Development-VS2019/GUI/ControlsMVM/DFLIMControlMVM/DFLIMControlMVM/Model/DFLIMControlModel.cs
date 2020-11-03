namespace DFLIMControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    using ThorLogging;

    using ThorSharedTypes;

    public class DFLIMControlModel
    {
        #region Constructors

        public DFLIMControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public int DFLIMAcquisitionMode
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_ACQUISITION_MODE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_ACQUISITION_MODE, value);
            }
        }

        public int DFLIMCoarseShift1
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_COARSE_SHIFT1, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_COARSE_SHIFT1, value);
            }
        }

        public int DFLIMCoarseShift2
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_COARSE_SHIFT2, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_COARSE_SHIFT2, value);
            }
        }

        public int DFLIMCoarseShift3
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_COARSE_SHIFT3, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_COARSE_SHIFT3, value);
            }
        }

        public int DFLIMCoarseShift4
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_COARSE_SHIFT4, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_COARSE_SHIFT4, value);
            }
        }

        public int DFLIMFineShift1
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FINE_SHIFT1, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FINE_SHIFT1, value);
            }
        }

        public int DFLIMFineShift2
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FINE_SHIFT2, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FINE_SHIFT2, value);
            }
        }

        public int DFLIMFineShift3
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FINE_SHIFT3, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FINE_SHIFT3, value);
            }
        }

        public int DFLIMFineShift4
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FINE_SHIFT4, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FINE_SHIFT4, value);
            }
        }

        public double DFLIMFreqClock1
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FREQ_CLOCK1, ref temp);

                return temp;
            }
        }

        public double DFLIMFreqClock2
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FREQ_CLOCK2, ref temp);

                return temp;
            }
        }

        public double DFLIMFreqClock3
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FREQ_CLOCK3, ref temp);

                return temp;
            }
        }

        public double DFLIMFreqClock4
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_FREQ_CLOCK4, ref temp);

                return temp;
            }
        }

        public double DFLIMImpliedFreqClock1
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_IMPLIED_FREQ_CLOCK1, ref temp);

                return temp;
            }
        }

        public double DFLIMImpliedFreqClock2
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_IMPLIED_FREQ_CLOCK2, ref temp);

                return temp;
            }
        }

        public double DFLIMImpliedFreqClock3
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_IMPLIED_FREQ_CLOCK3, ref temp);

                return temp;
            }
        }

        public double DFLIMImpliedFreqClock4
        {
            get
            {
                double temp = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_IMPLIED_FREQ_CLOCK4, ref temp);

                return temp;
            }
        }

        public int DFLIMQueryClockFrequencies
        {
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_QUERY_CLOCK_FREQS, value);
            }
        }

        public int DFLIMReSync
        {
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_RESYNC, value);
            }
        }

        public int DFLIMResyncDelay
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_RESYNC_DELAY, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_RESYNC_DELAY, value);
            }
        }

        public int DFLIMResyncEveryLine
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_RESYNC_EVERYLINE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_RESYNC_EVERYLINE, value);
            }
        }

        public int DFLIMSaveImagesOnLiveMode
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_SAVE_IMAGES_ON_LIVE_MODE, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_SAVE_IMAGES_ON_LIVE_MODE, value);
            }
        }

        public int DFLIMSaveSettings
        {
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_SAVE_SETTINGS, value);
            }
        }

        public int DFLIMSyncDelay
        {
            get
            {
                int temp = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_SYNC_DELAY, ref temp);

                return temp;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_DFLIM_SYNC_DELAY, value);
            }
        }

        #endregion Properties
    }
}