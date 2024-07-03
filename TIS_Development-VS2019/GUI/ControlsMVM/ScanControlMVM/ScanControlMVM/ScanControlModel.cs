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

    public class ScanControlModel
    {
        #region Fields

        public const int MAX_CHANNELS = 4;

        private DateTime _lastPMTSafetyUpdate = DateTime.Now;
        bool[] _lsmChannelEnable;
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
            _pmtMode = new int[MAX_CHANNELS];
            LsmClkPnlEnabled = true;
            _lsmChannelEnable = new bool[MAX_CHANNELS];
            for (int i = 0; i < MAX_CHANNELS; i++)
            {
                _lsmChannelEnable[i] = true;
            }
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
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_TYPE, ref val);
                return val;
            }
        }

        public Visibility CoarsePanelVisibility
        {
            get;
            set;
        }

        public int ControlUnitType
        {
            get
            {
                int type = (int)ScopeType.UPRIGHT;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_CONTROLUNIT, (int)IDevice.Params.PARAM_CONTROL_UNIT_TYPE, ref type);

                return type;
            }
        }

        public double FramesPerSecond
        {
            get
            {
                double val = 0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_FRAME_RATE, ref val);
                return val;
            }
        }

        public int ImagingRampExtensionCycles
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGING_RAMP_EXTENSION_CYCLES, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGING_RAMP_EXTENSION_CYCLES, value);
            }
        }

        public bool IsImagingRampExtensionCyclesAvailable
        {
            get
            {
                int val = ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGING_RAMP_EXTENSION_CYCLES);

                return val == 1;
            }
        }

        public bool IsLSMCurrentCRSFrequencyAvailable
        {
            get
            {
                int val = ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CURRENT_CRS_FREQUENCY);

                return val == 1;
            }
        }

        public bool IsLSMImageDistortionCorrectionAvailable
        {
            get
            {
                int val = ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_ENABLE);

                return val == 1;
            }
        }

        public bool IsLSMImageDistortionCorrectionCalibrationGalvoTiltAngleAvailable
        {
            get
            {
                int val = ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_GALVO_TILT_ANGLE);

                return val == 1;
            }
        }

        public bool IsLSMImageDistortionCorrectionCalibrationXAngleMaxAvailable
        {
            get
            {
                int val = ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_X_ANGLE_MAX);

                return val == 1;
            }
        }

        public bool IsLSMImageDistortionCorrectionCalibrationYAngleMaxAvailable
        {
            get
            {
                int val = ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_Y_ANGLE_MAX);

                return val == 1;
            }
        }

        public bool IsPreImagingCalibrationCyclesAvailable
        {
            get
            {
                int val = ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PREIMAGING_CALIBRATION_CYCLES);

                return val == 1;
            }
        }

        public DateTime LastPMTSafetyUpdate
        {
            get { return _lastPMTSafetyUpdate; }
            set { _lastPMTSafetyUpdate = value; }
        }

        public int LSMChannel
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CHANNEL, ref val);
                return val;
            }
        }

        public bool[] LSMChannelEnable
        {
            get => _lsmChannelEnable;
        }

        public bool LSMChannelEnable0
        {
            get
            {
                return _lsmChannelEnable[0];
            }
            set
            {
                if (_lsmChannelEnable[0] != value)
                {
                    _lsmChannelEnable[0] = value;
                    SetChannelFromEnable();
                }
            }
        }

        public bool LSMChannelEnable1
        {
            get
            {
                return _lsmChannelEnable[1];
            }
            set
            {
                if (_lsmChannelEnable[1] != value)
                {
                    _lsmChannelEnable[1] = value;
                    SetChannelFromEnable();
                }
            }
        }

        public bool LSMChannelEnable2
        {
            get
            {
                return _lsmChannelEnable[2];
            }
            set
            {
                if (_lsmChannelEnable[2] != value)
                {
                    _lsmChannelEnable[2] = value;
                    SetChannelFromEnable();
                }
            }
        }

        public bool LSMChannelEnable3
        {
            get
            {
                return _lsmChannelEnable[3];
            }
            set
            {
                if (_lsmChannelEnable[3] != value)
                {
                    _lsmChannelEnable[3] = value;
                    SetChannelFromEnable();
                }
            }
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
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CLOCKSOURCE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CLOCKSOURCE, value);
            }
        }

        public double LSMCurrentCRSFrequency
        {
            get
            {
                double val = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CURRENT_CRS_FREQUENCY, ref val);

                return val;
            }
        }

        public int LSMExtClockRate
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_EXTERNALCLOCKRATE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_EXTERNALCLOCKRATE, value);
            }
        }

        public double LSMExternalClockPhaseOffset
        {
            get
            {
                double val = 1;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_EXTERNAL_CLOCK_PHASE_OFFSET, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_EXTERNAL_CLOCK_PHASE_OFFSET, value);
            }
        }

        public bool LSMFastOneWayImagingModeEnable
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FAST_ONEWAY_MODE_ENABLE, ref val);
                return val == 1;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FAST_ONEWAY_MODE_ENABLE, value == true ? 1 : 0);
            }
        }

        public int LSMFlybackCycles
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FLYBACK_CYCLE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FLYBACK_CYCLE, value);
            }
        }

        public double LSMFlybackTime
        {
            get
            {
                double val = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FLYBACK_TIME, ref val);

                return val;
            }
        }

        public int LSMGalvoRate
        {
            get;
            set;
        }

        public double LSMImageDistortionCorrectionCalibrationGalvoTiltAngle
        {
            get
            {
                double val = 0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_GALVO_TILT_ANGLE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_GALVO_TILT_ANGLE, value);
            }
        }

        public double LSMImageDistortionCorrectionCalibrationXAngleMax
        {
            get
            {
                double val = 0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_X_ANGLE_MAX, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_X_ANGLE_MAX, value);
            }
        }

        public double LSMImageDistortionCorrectionCalibrationYAngleMax
        {
            get
            {
                double val = 0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_Y_ANGLE_MAX, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_CALIBRATION_Y_ANGLE_MAX, value);
            }
        }

        public int LSMImageDistortionCorrectionEnable
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_ENABLE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_DISTORTION_CORRECTION_ENABLE, value);
            }
        }

        public int LSMImageOnFlyback
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_ON_FLYBACK, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_ON_FLYBACK, value);
            }
        }

        public int LSMInterleaveScan
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_INTERLEAVE_SCAN, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_INTERLEAVE_SCAN, Convert.ToInt32(value));
            }
        }

        public bool LSMIsFastOneWayImagingModeEnableAvailable
        {
            get
            {

                int val = ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FAST_ONEWAY_MODE_ENABLE);

                return val == 1;
            }
        }

        public int LSMIsImageOnFlybackAvailable
        {
            get
            {
                int val = ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IMAGE_ON_FLYBACK);
                return val;
            }
        }

        public bool LSMIsLineAveragingAvailable
        {
            get
            {
                int val = ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_LINE_AVERAGING_ENABLE);

                return val == 1;
            }
        }

        public int LSMLineAveragingEnable
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_LINE_AVERAGING_ENABLE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_LINE_AVERAGING_ENABLE, value);
            }
        }

        public int LSMLineAveragingNumber
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_LINE_AVERAGING_NUMBER, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_LINE_AVERAGING_NUMBER, value);
            }
        }

        public double LSMPixelDwellTime
        {
            get
            {
                double val = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DWELL_TIME, ref val);
                return val;

            }

            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DWELL_TIME, value);
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
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_PROCESS, ref val);
                }
                return val;
            }
            set
            {
                int lsmType = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);
                if (lsmType == (int)ICamera.LSMType.GALVO_GALVO && ResourceManagerCS.Instance.GetActiveLSMName().Equals("GalvoGalvo"))
                {
                    ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_PROCESS, value);
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
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PULSE_MULTIPLEXING_ENABLE, value);
            }
        }

        // LSMQueryExternalClockRate is currently used only with Thordaq. Eventually it could be used with any other camera.
        public int LSMQueryExternalClockRate
        {
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_QUERY_EXTERNALCLOCKRATE, value);
            }
        }

        public int LSMRealtimeAveraging
        {
            get
            {
                int val = 1;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE, (double)value);
            }
        }

        public double LSMScaleYScan
        {
            get
            {
                int val = 1;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_Y_AMPLITUDE_SCALER, ref val);

                double dVal = val / 100.0;

                Decimal dec = new Decimal(dVal);

                dec = Decimal.Round(dec, 2);
                return Convert.ToDouble(dec.ToString());
            }
            set
            {
                int val = Convert.ToInt32(value * 100);
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_Y_AMPLITUDE_SCALER, (double)val);
            }
        }

        public int LSMScanMode
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_SCANMODE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_SCANMODE, value);
            }
        }

        public string LSMScannerName
        {
            get
            {
                string str = string.Empty;
                int type = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref type);

                switch ((ICamera.LSMType)type)
                {
                    case ICamera.LSMType.GALVO_RESONANCE: str = "Galvo/Resonance Scanner Control"; break;
                    case ICamera.LSMType.RESONANCE_GALVO_GALVO: str = "Resonance/Galvo/Galvo Scanner Control"; break;
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
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AVERAGEMODE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AVERAGEMODE, value);
            }
        }

        public int LSMTwoWayAlignment
        {
            get
            {
                int lsmType = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);

                int val = 0;
                if ((int)ICamera.LSMType.GALVO_RESONANCE == lsmType || (int)ICamera.LSMType.RESONANCE_GALVO_GALVO == lsmType)
                {
                    GetTwoWayAlignmentFine((int)MVMManager.Instance["AreaControlViewModel", "LSMFieldSize"], ref val);
                }
                else
                {
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_ALIGNMENT, ref val);
                }
                return val;
            }
            set
            {
                int lsmType = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);

                if ((int)ICamera.LSMType.GALVO_RESONANCE == lsmType || (int)ICamera.LSMType.RESONANCE_GALVO_GALVO == lsmType)
                {
                    SetTwoWayAlignmentFine((int)MVMManager.Instance["AreaControlViewModel", "LSMFieldSize"], value);
                }
                else
                {
                    ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_ALIGNMENT, value);
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
                int valMin = 0, valMax = 0, valDefault = 0;
                ResourceManagerCS.GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_CONTROLUNIT, (int)IDevice.Params.PARAM_SCANNER_ALIGN_POS, ref valMin, ref valMax, ref valDefault);
                if (valMax < value)
                {
                    value = valMax;
                }
                if (valMin > value)
                {
                    value = valMin;
                }
                SetTwoWayAlignmentCoarse((int)MVMManager.Instance["AreaControlViewModel", "LSMFieldSize"], value);
            }
        }

        public int MovingAvarageFilterEnable
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_RESEARCH_CAMERA_0, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_RESEARCH_CAMERA_0, value);
            }
        }

        public double PMT1Atenuation
        {
            get
            {
                double val = 0;

                ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT1, (int)IDevice.Params.PARAM_PMT1_GAIN_POS_CURRENT_VOLTS, ref val);

                return val;
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

        public double PMT2Atenuation
        {
            get
            {
                double val = 0;

                ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT2, (int)IDevice.Params.PARAM_PMT2_GAIN_POS_CURRENT_VOLTS, ref val);

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

        public double PMT3Atenuation
        {
            get
            {
                double val = 0;

                ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT3, (int)IDevice.Params.PARAM_PMT3_GAIN_POS_CURRENT_VOLTS, ref val);

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

        public double PMT4Atenuation
        {
            get
            {
                double val = 0;

                ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT4, (int)IDevice.Params.PARAM_PMT4_GAIN_POS_CURRENT_VOLTS, ref val);

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
                    ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_PMT1, (int)IDevice.Params.PARAM_PMT1_TYPE, value[0], 0);
                    ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_PMT2, (int)IDevice.Params.PARAM_PMT2_TYPE, value[1], 0);
                    ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_PMT3, (int)IDevice.Params.PARAM_PMT3_TYPE, value[2], 0);
                    ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_PMT4, (int)IDevice.Params.PARAM_PMT4_TYPE, value[3], 0);
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

        public int PreImagingCalibrationCycles
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PREIMAGING_CALIBRATION_CYCLES, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PREIMAGING_CALIBRATION_CYCLES, value);
            }
        }

        public int SignalAverageFrames
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AVERAGENUM, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AVERAGENUM, value);
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
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_GG_TURNAROUNDTIME_US, value);
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
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_MINIMIZE_FLYBACK_CYCLES, ref val);
                return val > 0;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_MINIMIZE_FLYBACK_CYCLES, Convert.ToInt32(value));
            }
        }

        #endregion Properties

        #region Methods

        public void GetChanDigOffsetAvailable(int index, ref bool val)
        {
            int offset = 0;
            val = (1 == ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DIG_OFFSET_0 + index, ref offset));
        }

        public double GetDetectorAtenuation(int index)
        {
            switch (index)
            {
                case 0: return PMT1Atenuation;
                case 1: return PMT2Atenuation;
                case 2: return PMT3Atenuation;
                case 3: return PMT4Atenuation;
                default: return 0;
            }
        }

        public string[] GetPMTAvailableBandwidths(int index)
        {
            const int LENGTH = 255;
            StringBuilder paramSB = new StringBuilder(LENGTH);
            switch (index)
            {
                case 0: ResourceManagerCS.GetDeviceParamString((int)SelectedHardware.SELECTED_PMT1, (int)IDevice.Params.PARAM_PMT1_AVAILABLE_BANDWIDTHS, paramSB, LENGTH); break;
                case 1: ResourceManagerCS.GetDeviceParamString((int)SelectedHardware.SELECTED_PMT2, (int)IDevice.Params.PARAM_PMT2_AVAILABLE_BANDWIDTHS, paramSB, LENGTH); break;
                case 2: ResourceManagerCS.GetDeviceParamString((int)SelectedHardware.SELECTED_PMT3, (int)IDevice.Params.PARAM_PMT3_AVAILABLE_BANDWIDTHS, paramSB, LENGTH); break;
                case 3: ResourceManagerCS.GetDeviceParamString((int)SelectedHardware.SELECTED_PMT4, (int)IDevice.Params.PARAM_PMT4_AVAILABLE_BANDWIDTHS, paramSB, LENGTH); break;
            }
            return paramSB.ToString().Split(',');
        }

        public bool GetPMTBandwidthIsAvailable(int index, ref int bwPos)
        {
            return (1 == ResourceManagerCS.GetDeviceParamInt((int)Enum.Parse(typeof(SelectedHardware),
                                             string.Format("SELECTED_PMT{0}", index + 1)),
                                             (int)Enum.Parse(typeof(IDevice.Params),
                                             string.Format("PARAM_PMT{0}_BANDWIDTH_POS", index + 1)),
                                             ref bwPos));
        }

        public double GetPMTGainStepSize(int index)
        {
            double gainStep = 1;
            switch (index)
            {
                case 0: ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT1, (int)IDevice.Params.PARAM_PMT1_GAIN_STEP_SIZE, ref gainStep); break;
                case 1: ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT2, (int)IDevice.Params.PARAM_PMT2_GAIN_STEP_SIZE, ref gainStep); break;
                case 2: ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT3, (int)IDevice.Params.PARAM_PMT3_GAIN_STEP_SIZE, ref gainStep); break;
                case 3: ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT4, (int)IDevice.Params.PARAM_PMT4_GAIN_STEP_SIZE, ref gainStep); break;
            }

            return gainStep;
        }

        public void GetPMTOffsetIsAvailable(int index, ref bool val)
        {
            double offset = 0;
            switch (index)
            {
                case 0: val = 1 == ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT1, (int)IDevice.Params.PARAM_PMT1_OUTPUT_OFFSET, ref offset); break;
                case 1: val = 1 == ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT2, (int)IDevice.Params.PARAM_PMT2_OUTPUT_OFFSET, ref offset); break;
                case 2: val = 1 == ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT3, (int)IDevice.Params.PARAM_PMT3_OUTPUT_OFFSET, ref offset); break;
                case 3: val = 1 == ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT4, (int)IDevice.Params.PARAM_PMT4_OUTPUT_OFFSET, ref offset); break;
            }
        }

        public double GetPMTOffsetStepSize(int index)
        {
            double offset = 0.02;
            switch (index)
            {
                case 0: ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT1, (int)IDevice.Params.PARAM_PMT1_OFFSET_STEP_SIZE, ref offset); break;
                case 1: ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT2, (int)IDevice.Params.PARAM_PMT2_OFFSET_STEP_SIZE, ref offset); break;
                case 2: ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT3, (int)IDevice.Params.PARAM_PMT3_OFFSET_STEP_SIZE, ref offset); break;
                case 3: ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_PMT4, (int)IDevice.Params.PARAM_PMT4_OFFSET_STEP_SIZE, ref offset); break;
            }

            return offset;
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetPMTSafetyStatus")]
        private static extern bool GetPMTSafetyStatus();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetTwoWayAlignmentCoarse")]
        private static extern int GetTwoWayAlignmentCoarse(int fieldSize, ref int value);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetTwoWayAlignmentFine")]
        private static extern int GetTwoWayAlignmentFine(int fieldSize, ref int value);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetDisplayChannels")]
        static extern long SetDisplayChannels(int chan);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetTwoWayAlignmentCoarse")]
        private static extern int SetTwoWayAlignmentCoarse(int fieldSize, int value);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetTwoWayAlignmentFine")]
        private static extern int SetTwoWayAlignmentFine(int fieldSize, int value);

        void SetChannelFromEnable()
        {
            //update the channel value also
            int chan = (Convert.ToInt32(_lsmChannelEnable[0]) | (Convert.ToInt32(_lsmChannelEnable[1]) << 1) | (Convert.ToInt32(_lsmChannelEnable[2]) << 2) | (Convert.ToInt32(_lsmChannelEnable[3]) << 3));

            ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CHANNEL, chan);
            SetDisplayChannels(chan);
        }

        #endregion Methods
    }
}