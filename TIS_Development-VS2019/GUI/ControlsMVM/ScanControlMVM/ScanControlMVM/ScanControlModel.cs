namespace ScanControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
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

    public class ScanControlModel
    {
        #region Fields

        private DateTime _lastPMTSafetyUpdate = DateTime.Now;
        private int[] _pmtMode;
        bool _pmtSafetyStatus = true;
        private double[,] _pmtVoltage;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the ScanControlModel class
        /// </summary>
        public ScanControlModel()
        {
            const int MAX_CHANNELS = 4;
            _pmtMode = new int[MAX_CHANNELS];
            LsmClkPnlEnabled = true;
        }

        #endregion Constructors

        #region Properties

        public double CalculatedMinDwellTime
        {
            get
            {
                double val = 0.4;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CALCULATED_MIN_DWELL, ref val);
                return val;
            }
        }

        public int CameraType
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_TYPE, ref val);
                return val;
            }
        }

        public Visibility CoarsePanelVisibility
        {
            get;
            set;
        }

        public double FramesPerSecond
        {
            get
            {
                double val = 0;
                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_FRAME_RATE, ref val);
                return val;
            }
        }

        public DateTime LastPMTSafetyUpdate
        {
            get { return _lastPMTSafetyUpdate; }
            set { _lastPMTSafetyUpdate = value; }
        }

        public bool LsmClkPnlEnabled
        {
            get;
            set;
        }

        public int LSMClockSource
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CLOCKSOURCE, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CLOCKSOURCE, value);
            }
        }

        public int LSMExtClockRate
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_EXTERNALCLOCKRATE, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_EXTERNALCLOCKRATE, value);
            }
        }

        public int LSMFlybackCycles
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FLYBACK_CYCLE, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FLYBACK_CYCLE, value);
            }
        }

        public double LSMFlybackTime
        {
            get
            {
                double val = 0;

                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FLYBACK_TIME, ref val);

                return val;
            }
        }

        public int LSMGalvoRate
        {
            get;
            set;
        }

        public int LSMInterleaveScan
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_INTERLEAVE_SCAN, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_INTERLEAVE_SCAN, Convert.ToInt32(value));
            }
        }

        public double LSMPixelDwellTime
        {
            get
            {
                double val = 0;

                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DWELL_TIME, ref val);
                return val;

            }

            set
            {
                SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DWELL_TIME, value);
            }
        }

        public double LSMPixelDwellTimeMax
        {
            get
            {
                double valMin = 0;
                double valMax = 0;
                double valDefault = 0;

                ResourceManagerCS.GetCameraParamRangeDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DWELL_TIME, ref valMin, ref valMax, ref valDefault);

                return valMax;

            }
        }

        public double LSMPixelDwellTimeMin
        {
            get
            {
                double valMin = 0;
                double valMax = 0;
                double valDefault = 0;

                ResourceManagerCS.GetCameraParamRangeDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DWELL_TIME, ref valMin, ref valMax, ref valDefault);

                return valMin;
            }
        }

        public double LSMPixelDwellTimeStep
        {
            get
            {
                double val = 0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DWELL_TIME_STEP, ref val);
                return val;
            }
        }

        public int LSMPixelProcess
        {
            get
            {
                int val = 0;
                int lsmType = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);
                if (lsmType == (int)ICamera.LSMType.GALVO_GALVO && ResourceManagerCS.Instance.GetActiveLSMName().Equals("GalvoGalvo"))
                {
                    GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_PROCESS, ref val);
                }
                return val;
            }
            set
            {
                int lsmType = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);
                if (lsmType == (int)ICamera.LSMType.GALVO_GALVO && ResourceManagerCS.Instance.GetActiveLSMName().Equals("GalvoGalvo"))
                {
                    SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_PROCESS, value);
                }
            }
        }

        public int LSMPulseMultiplexing
        {
            get
            {
                int val = 1;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PULSE_MULTIPLEXING_ENABLE, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PULSE_MULTIPLEXING_ENABLE, value);
            }
        }

        public double LSMPulseMultiplexingPhase
        {
            get
            {
                double val = 1;
                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PULSE_MULTIPLEXING_PHASE, ref val);
                return val;
            }
            set
            {
                SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PULSE_MULTIPLEXING_PHASE, value);
            }
        }

        // LSMQueryExternalClockRate is currently used only with Thordaq. Eventually it could be used with any other camera.
        public int LSMQueryExternalClockRate
        {
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_QUERY_EXTERNALCLOCKRATE, value);
            }
        }

        public int LSMRealtimeAveraging
        {
            get
            {
                int val = 1;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE, ref val);
                return val;
            }
            set
            {
                SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE, (double)value);
            }
        }

        public double LSMScaleYScan
        {
            get
            {
                int val = 1;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_Y_AMPLITUDE_SCALER, ref val);

                double dVal = val / 100.0;

                Decimal dec = new Decimal(dVal);

                dec = Decimal.Round(dec, 2);
                return Convert.ToDouble(dec.ToString());
            }
            set
            {
                int val = Convert.ToInt32(value * 100);
                SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_Y_AMPLITUDE_SCALER, (double)val);
            }
        }

        public int LSMScanMode
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_SCANMODE, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_SCANMODE, value);
            }
        }

        public string LSMScannerName
        {
            get
            {
                string str = string.Empty;
                int type = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref type);

                switch ((ICamera.LSMType)type)
                {
                    case ICamera.LSMType.GALVO_RESONANCE: str = "Galvo/Resonance Scanner Control"; break;
                    case ICamera.LSMType.GALVO_GALVO: str = "Galvo/Galvo Scanner Control"; break;
                }

                return str;
            }
        }

        public int LSMSignalAverage
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AVERAGEMODE, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AVERAGEMODE, value);
            }
        }

        public int LSMTwoWayAlignment
        {
            get
            {
                int lsmType = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);

                int val = 0;
                if ((int)ICamera.LSMType.GALVO_RESONANCE == lsmType)
                {
                    GetTwoWayAlignmentFine((int)MVMManager.Instance["AreaControlViewModel", "LSMFieldSize"], ref val);
                }
                else
                {
                    GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_ALIGNMENT, ref val);
                }
                return val;
            }
            set
            {
                int lsmType = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);

                if ((int)ICamera.LSMType.GALVO_RESONANCE == lsmType)
                {
                    SetTwoWayAlignmentFine((int)MVMManager.Instance["AreaControlViewModel", "LSMFieldSize"], value);
                }
                else
                {
                    SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_ALIGNMENT, value);
                }
            }
        }

        public int LSMTwoWayAlignmentCoarse
        {
            get
            {
                int val = 0;
                GetTwoWayAlignmentCoarse((int)MVMManager.Instance["AreaControlViewModel", "LSMFieldSize"], ref val);
                return val;
            }
            set
            {
                SetTwoWayAlignmentCoarse((int)MVMManager.Instance["AreaControlViewModel", "LSMFieldSize"], value);
            }
        }

        public int PMT1GainMax
        {
            get
            {
                int valMin = 0;
                int valMax = 0;
                int valDefault = 0;

                ResourceManagerCS.GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_PMT1, (int)IDevice.Params.PARAM_PMT1_GAIN_POS, ref valMin, ref valMax, ref valDefault);

                return valMax;
            }
        }

        public int PMT1GainMin
        {
            get
            {
                int valMin = 0;
                int valMax = 0;
                int valDefault = 0;

                ResourceManagerCS.GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_PMT1, (int)IDevice.Params.PARAM_PMT1_GAIN_POS, ref valMin, ref valMax, ref valDefault);

                return valMin;
            }
        }

        public int PMT1Saturations
        {
            get
            {
                int val = 0;

                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_PMT1, (int)IDevice.Params.PARAM_PMT1_SATURATIONS, ref val);

                return val;
            }
        }

        public int PMT2GainMax
        {
            get
            {
                int valMin = 0;
                int valMax = 0;
                int valDefault = 0;

                ResourceManagerCS.GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_PMT2, (int)IDevice.Params.PARAM_PMT2_GAIN_POS, ref valMin, ref valMax, ref valDefault);

                return valMax;
            }
        }

        public int PMT2GainMin
        {
            get
            {
                int valMin = 0;
                int valMax = 0;
                int valDefault = 0;

                ResourceManagerCS.GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_PMT2, (int)IDevice.Params.PARAM_PMT2_GAIN_POS, ref valMin, ref valMax, ref valDefault);

                return valMin;
            }
        }

        public int PMT2Saturations
        {
            get
            {
                int val = 0;

                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_PMT2, (int)IDevice.Params.PARAM_PMT2_SATURATIONS, ref val);

                return val;
            }
        }

        public int PMT3GainMax
        {
            get
            {
                int valMin = 0;
                int valMax = 0;
                int valDefault = 0;

                ResourceManagerCS.GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_PMT3, (int)IDevice.Params.PARAM_PMT3_GAIN_POS, ref valMin, ref valMax, ref valDefault);

                return valMax;
            }
        }

        public int PMT3GainMin
        {
            get
            {
                int valMin = 0;
                int valMax = 0;
                int valDefault = 0;

                ResourceManagerCS.GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_PMT3, (int)IDevice.Params.PARAM_PMT3_GAIN_POS, ref valMin, ref valMax, ref valDefault);

                return valMin;
            }
        }

        public int PMT3Saturations
        {
            get
            {
                int val = 0;

                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_PMT3, (int)IDevice.Params.PARAM_PMT3_SATURATIONS, ref val);

                return val;
            }
        }

        public int PMT4GainMax
        {
            get
            {
                int valMin = 0;
                int valMax = 0;
                int valDefault = 0;

                ResourceManagerCS.GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_PMT4, (int)IDevice.Params.PARAM_PMT4_GAIN_POS, ref valMin, ref valMax, ref valDefault);

                return valMax;
            }
        }

        public int PMT4GainMin
        {
            get
            {
                int valMin = 0;
                int valMax = 0;
                int valDefault = 0;

                ResourceManagerCS.GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_PMT4, (int)IDevice.Params.PARAM_PMT4_GAIN_POS, ref valMin, ref valMax, ref valDefault);

                return valMin;
            }
        }

        public int PMT4Saturations
        {
            get
            {
                int val = 0;

                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_PMT4, (int)IDevice.Params.PARAM_PMT4_SATURATIONS, ref val);

                return val;
            }
        }

        public int[] PMTMode
        {
            get
            {
                return _pmtMode;
            }
            set
            {
                if (4 <= value.Length)
                {
                    SetDeviceParamInt((int)SelectedHardware.SELECTED_PMT1, (int)IDevice.Params.PARAM_PMT1_TYPE, value[0], false);
                    SetDeviceParamInt((int)SelectedHardware.SELECTED_PMT2, (int)IDevice.Params.PARAM_PMT2_TYPE, value[1], false);
                    SetDeviceParamInt((int)SelectedHardware.SELECTED_PMT3, (int)IDevice.Params.PARAM_PMT3_TYPE, value[2], false);
                    SetDeviceParamInt((int)SelectedHardware.SELECTED_PMT4, (int)IDevice.Params.PARAM_PMT4_TYPE, value[3], false);
                }
                _pmtMode = value;
            }
        }

        public bool PMTSafetyStatus
        {
            get
            {
                return _pmtSafetyStatus;
            }
            set
            {
                _pmtSafetyStatus = value;
            }
        }

        public double[,] PMTVoltage
        {
            get
            {
                return _pmtVoltage;
            }
            set
            {
                _pmtVoltage = value;
            }
        }

        public int SignalAverageFrames
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AVERAGENUM, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AVERAGENUM, value);
            }
        }

        public int TurnAroundTimeUS
        {
            get
            {
                int val = 400;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_GG_TURNAROUNDTIME_US, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_GG_TURNAROUNDTIME_US, value);
            }
        }

        /// <summary>
        /// Tells the camera to always keep flyback cycles to the lowest it can
        /// with its current settings
        /// </summary>
        public bool UseFastestSettingForFlybackCycles
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_MINIMIZE_FLYBACK_CYCLES, ref val);
                return val > 0;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_MINIMIZE_FLYBACK_CYCLES, Convert.ToInt32(value));
            }
        }

        #endregion Properties

        #region Methods

        public void GetChanDigOffsetAvailable(int index, ref bool val)
        {
            int offset = 0;
            val = (1 == GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DIG_OFFSET_0 + index, ref offset));
        }

        public bool GetPMTBandwidthIsAvailable(int index, ref int bwPos)
        {
            return (1 == ResourceManagerCS.GetDeviceParamInt((int)Enum.Parse(typeof(SelectedHardware),
                                             string.Format("SELECTED_PMT{0}", index + 1)),
                                             (int)Enum.Parse(typeof(IDevice.Params),
                                             string.Format("PARAM_PMT{0}_BANDWIDTH_POS", index + 1)),
                                             ref bwPos));
        }

        public void GetPMTOffsetIsAvailable(int index, ref bool val)
        {
            int param = 0;
            switch (index)
            {
                case 0: param = (int)IDevice.Params.PARAM_PMT1_OUTPUT_OFFSET; break;
                case 1: param = (int)IDevice.Params.PARAM_PMT2_OUTPUT_OFFSET; break;
                case 2: param = (int)IDevice.Params.PARAM_PMT3_OUTPUT_OFFSET; break;
                case 3: param = (int)IDevice.Params.PARAM_PMT4_OUTPUT_OFFSET; break;
            }
            double offset = 0;
            val = (1 == GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT1, param, ref offset)) || (1 == GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT2, param, ref offset)) || (1 == GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT3, param, ref offset)) || (1 == GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT4, param, ref offset));
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamDouble")]
        private static extern int GetCameraParamDouble(int cameraSelection, int param, ref double value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamLong")]
        private static extern int GetCameraParamInt(int cameraSelection, int param, ref int value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamRangeLong")]
        private static extern int GetCameraParamRangeInt(int cameraSelection, int paramId, ref int valMin, ref int valMax, ref int valDefault);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamDouble")]
        private static extern int GetDeviceParamDouble(int deviceSelection, int paramId, ref double param);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamLong")]
        private static extern int GetDeviceParamInt(int deviceSelection, int paramId, ref int param);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamRangeLong")]
        private static extern int GetDeviceParamRangeInt(int deviceSelection, int paramId, ref int valMin, ref int valMax, ref int valDefault);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetPMTSafetyStatus")]
        private static extern bool GetPMTSafetyStatus();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetTwoWayAlignmentCoarse")]
        private static extern int GetTwoWayAlignmentCoarse(int fieldSize, ref int value);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetTwoWayAlignmentFine")]
        private static extern int GetTwoWayAlignmentFine(int fieldSize, ref int value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamDouble")]
        private static extern int SetCameraParamDouble(int cameraSelection, int param, double value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamLong")]
        private static extern int SetCameraParamInt(int cameraSelection, int param, int value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamDouble")]
        private static extern int SetDeviceParamDouble(int deviceSelection, int paramId, double param, bool wait);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamLong")]
        private static extern int SetDeviceParamInt(int deviceSelection, int paramId, int param, bool wait);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetTwoWayAlignmentCoarse")]
        private static extern int SetTwoWayAlignmentCoarse(int fieldSize, int value);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetTwoWayAlignmentFine")]
        private static extern int SetTwoWayAlignmentFine(int fieldSize, int value);

        #endregion Methods
    }
}