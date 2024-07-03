namespace CameraControl.Model
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
    using static ThorSharedTypes.ICamera;

    public class CameraControlModel
    {
        #region Fields

        private string _activeCameraName = string.Empty;
        private int _binIndex;
        private bool _cameraCSType = false;
        private int _colorImageTypeIndex;
        private int _coolingModeIndex;
        private double _exposureTimeMax;
        private double _exposureTimeMin;
        private int _hotPixelLevelIndex;
        private double _hotPixelMax;
        private double _hotPixelMin;
        private int _lightModeMax;
        private int _lightModeMin;
        private int _polarImageTypeIndex;
        private int _readoutSpeedIndex;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the CameraControlModel class
        /// </summary>
        public CameraControlModel()
        {
            _hotPixelMax = 0.0;
            _hotPixelMin = 0.0;
            _exposureTimeMin = .001;
            _exposureTimeMax = 1000;
            _readoutSpeedIndex = -1;
            _binIndex = -1;
            _hotPixelLevelIndex = -1;
            _coolingModeIndex = -1;
        }

        #endregion Constructors

        #region Properties

        public string ActiveCameraName
        {
            get
            {
                return _activeCameraName;
            }
            set
            {
                _activeCameraName = value;
            }
        }

        public int BinIndex
        {
            get
            {
                if (-1 != _binIndex)
                {
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BIN_INDEX, ref _binIndex);
                }
                return _binIndex;
            }
            set
            {
                if (-1 != value)
                {
                    ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BIN_INDEX, value);
                }
                _binIndex = value;
            }
        }

        public int BinX
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType())
                {
                    return 1;
                }

                int val = 1;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BINNING_X, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BINNING_X, value);
            }
        }

        public int BinY
        {
            get
            {
                if ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType())
                {
                    return 1;
                }

                int val = 1;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BINNING_Y, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BINNING_Y, value);
            }
        }

        public int BitsPerPixel
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BITS_PER_PIXEL, ref val);
                return val;
            }
        }

        public double BlueGain
        {
            get
            {
                double val = 0.0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_BLUE_GAIN, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_BLUE_GAIN, value);
            }
        }

        public int Bottom
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_BOTTOM, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_BOTTOM, value);
            }
        }

        public int BottomMax
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_BOTTOM, ref exMin, ref exMax, ref exDefault);

                return exMax;
            }
        }

        public int BottomMin
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_BOTTOM, ref exMin, ref exMax, ref exDefault);

                return exMin;
            }
        }

        public int CamAverageMode
        {
            get
            {
                int val = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_AVERAGEMODE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_AVERAGEMODE, value);
            }
        }

        public int CamAverageNum
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_AVERAGENUM, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_AVERAGENUM, value);
            }
        }

        public int CamBitsPerPixel
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BITS_PER_PIXEL, ref val);
                return val;
            }
        }

        public int CamBlackLevel
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_OPTICAL_BLACK_LEVEL, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_OPTICAL_BLACK_LEVEL, value);
            }
        }

        public int CamColorImageTypeIndex
        {
            get
            {
                // TODO: this value doesn't need to be saved. you can pull it from TSI cams quickly  b\c it is a host-side param
                if (-1 != _colorImageTypeIndex)
                {
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_COLOR_IMAGE_TYPE, ref _colorImageTypeIndex);
                }
                return _colorImageTypeIndex;
            }
            set
            {
                if (-1 != value)
                {
                    ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_COLOR_IMAGE_TYPE, value);
                }
                _colorImageTypeIndex = value;
            }
        }

        public bool CameraCSType
        {
            get
            {
                return _cameraCSType;
            }
            set
            {
                _cameraCSType = value;
            }
        }

        public GainType CameraGainType
        {
            get
            {
                int gainType = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_GAIN_TYPE, ref gainType);
                return  (GainType)gainType;
            }
        }

        public int CameraImageAngle
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IMAGE_ANGLE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IMAGE_ANGLE, value);
            }
        }

        public ICamera.CameraSensorType CameraSensorType
        {
            get
            {
                int cameraSensorType = 0;
                _ = ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_SENSOR_TYPE, ref cameraSensorType);
                return (ICamera.CameraSensorType)cameraSensorType;
            }
        }

        public int CamHorizontalFlip
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP, value);
            }
        }

        public int CamImageHeight
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IMAGE_HEIGHT, ref val);
                return val;
            }
        }

        public int CamImageWidth
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IMAGE_WIDTH, ref val);
                return val;
            }
        }

        public bool CamLedAvailable
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_LED_AVAILABLE, ref val);
                return (1 == val);
            }
        }

        public int CamLedEnable
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_LED_ENABLE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_LED_ENABLE, value);
            }
        }

        public double CamOrcaFrameRate
        {
            get
            {
                double val = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_STATIC_FPS, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_STATIC_FPS, value);
            }
        }

        public int CamPolarImageTypeIndex
        {
            get
            {
                // TODO: this value doesn't need to be saved. you can pull it from TSI cams quickly  b\c it is a host-side param
                if (-1 != _polarImageTypeIndex)
                {
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_POLAR_IMAGE_TYPE, ref _polarImageTypeIndex);
                }
                return _polarImageTypeIndex;
            }
            set
            {
                if (-1 != value)
                {
                    ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_POLAR_IMAGE_TYPE, value);
                }
                _polarImageTypeIndex = value;
            }
        }

        public int CamReadoutSpeedIndex
        {
            get
            {
                if(-1 != _readoutSpeedIndex)
                {
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_READOUT_SPEED_INDEX, ref _readoutSpeedIndex);
                }
                return _readoutSpeedIndex;
            }
            set
            {
                if (-1 != value)
                {
                    ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_READOUT_SPEED_INDEX, value);
                }
                _readoutSpeedIndex = value;
            }
        }

        public int CamReadoutSpeedMax
        {
            get
            {
                int readoutMax = 0, readoutMin = 0, readoutDefault = 0;
                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_READOUT_SPEED_INDEX,ref readoutMin, ref readoutMax, ref readoutDefault);
                return readoutMax;
            }
        }

        public int CamReadoutSpeedMin
        {
            get
            {
                int readoutMax = 0, readoutMin = 0, readoutDefault = 0;
                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_READOUT_SPEED_INDEX, ref readoutMin, ref readoutMax, ref readoutDefault);
                return readoutMin;
            }
        }

        public double CamSensorPixelSizeUM
        {
            get
            {
                double val = 1;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_PIXEL_SIZE, ref val);
                return val;
            }
        }

        public int CamTapBalance
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_TAP_BALANCE_MODE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_TAP_BALANCE_MODE, value);
            }
        }

        public int CamTapIndex
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_TAP_INDEX, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_TAP_INDEX, value);
            }
        }

        public int CamTapIndexMax
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_TAP_INDEX, ref exMin, ref exMax, ref exDefault);

                return exMax;
            }
        }

        public int CamTapIndexMin
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_TAP_INDEX, ref exMin, ref exMax, ref exDefault);

                return exMin;
            }
        }

        public int CamVerticalFlip
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IMAGE_VERTICAL_FLIP, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IMAGE_VERTICAL_FLIP, value);
            }
        }

        public int ChannelNum
        {
            get
            {
                int val = 1;

                return (1 == ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_CHANNEL, ref val)) ? val : 0;
            }
        }

        public int ContinuousWhiteBalanceNumFrames
        {
            get
            {
                int continuousWhiteBalanceNumFrames = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_CONTINUOUS_WHITE_BALANCE_NUM_FRAMES, ref continuousWhiteBalanceNumFrames);
                return continuousWhiteBalanceNumFrames;
            }

            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_CONTINUOUS_WHITE_BALANCE_NUM_FRAMES, value);
            }
        }

        public bool CoolingModeAvailable
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_COOLING_AVAILABLE, ref val);
                return 1 == val;
            }
        }

        public int CoolingModeIndex
        {
            get
            {
                if (-1 != _coolingModeIndex)
                {
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_COOLING_MODE, ref _coolingModeIndex);
                }
                return _coolingModeIndex;
            }
            set
            {
                if (-1 != value)
                {
                    ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_COOLING_MODE, value);
                }
                _coolingModeIndex = value;
            }
        }

        public bool EnableReferenceChannel
        {
            get;
            set;
        }

        public EqualExposurePulseStatusType EqualExposurePulseStatus
        {
            get
            {
                int eepStatus = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_EEP_STATUS, ref eepStatus);
                return (EqualExposurePulseStatusType)eepStatus;
            }
        }

        public double EqualExposurePulseWidth
        {
            get
            {
                double pulseWidth = 0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_EEP_WIDTH, ref pulseWidth);
                return pulseWidth;
            }

            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_EEP_WIDTH, value);
            }
        }

        public double ExposureTimeCam
        {
            get
            {
                double val = 0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, ref val);

                Decimal dec = new Decimal(val);

                return Decimal.ToDouble(Decimal.Round(dec, 3));
            }
            set
            {
                Decimal dec = new Decimal(value);

                double val = Decimal.ToDouble(Decimal.Round(dec, 3));
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, val);
            }
        }

        public double ExposureTimeCam1
        {
            get
            {
                double val = 0;

                Decimal dec = new Decimal(val);

                return Decimal.ToDouble(Decimal.Round(dec, 3));
            }
            set
            {
                Decimal dec = new Decimal(value);

                double val = Decimal.ToDouble(Decimal.Round(dec, 3));
            }
        }

        public double ExposureTimeCam2
        {
            get
            {
                double val = 0;

                Decimal dec = new Decimal(val);

                return Decimal.ToDouble(Decimal.Round(dec, 3));
            }
            set
            {
                Decimal dec = new Decimal(value);

                double val = Decimal.ToDouble(Decimal.Round(dec, 3));
            }
        }

        public double ExposureTimeMax
        {
            get
            {
                double exMin = 0;
                double exMax = 0;
                double exDefault = 0;

                if (1 == ResourceManagerCS.GetCameraParamRangeDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, ref exMin, ref exMax, ref exDefault))
                {
                    _exposureTimeMax = exMax;

                }
                return _exposureTimeMax;
            }
        }

        public double ExposureTimeMin
        {
            get
            {
                double exMin = 0;
                double exMax = 0;
                double exDefault = 0;

                if (1 == ResourceManagerCS.GetCameraParamRangeDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, ref exMin, ref exMax, ref exDefault))
                {
                    _exposureTimeMin = exMin;

                }
                return _exposureTimeMin;
            }
        }

        public int FrameRateControlEnabled
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_FRAME_RATE_CONTROL_ENABLED, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_FRAME_RATE_CONTROL_ENABLED, value);
            }
        }

        public double FrameRateControlMax
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_FRAME_RATE_CONTROL_VALUE, ref exMin, ref exMax, ref exDefault);

                return exMax;
            }
        }

        public double FrameRateControlMin
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_FRAME_RATE_CONTROL_VALUE, ref exMin, ref exMax, ref exDefault);

                return exMin;
            }
        }

        public double FrameRateControlValue
        {
            get
            {
                double val = 0.0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_FRAME_RATE_CONTROL_VALUE, ref val);

                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_FRAME_RATE_CONTROL_VALUE, value);
            }
        }

        public int Gain
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_GAIN, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_GAIN, value);
            }
        }

        public double GainDecibels
        {
            get
            {
                double val = 0.0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_GAIN_DECIBELS, ref val);
                return val;
            }
            set
            {
                // NOTE: this changes 'Gain' property as well (but they are meant to be mutually exclusive)
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_GAIN_DECIBELS, value);
            }
        }

        public double GreenGain
        {
            get
            {
                double val = 0.0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_GREEN_GAIN, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_GREEN_GAIN, value);
            }
        }

        public bool HotPixelAvailable
        {
            get
            {
                bool paramAvailable = false;

                paramAvailable = (1 == ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_HOT_PIXEL_INDEX));

                return paramAvailable;
            }
        }

        public int HotPixelEnabled
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_HOT_PIXEL_ENABLED, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_HOT_PIXEL_ENABLED, value);
            }
        }

        public int HotPixelLevelIndex
        {
            get
            {
                if (-1 != _hotPixelLevelIndex)
                {
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_HOT_PIXEL_INDEX, ref _hotPixelLevelIndex);
                }
                return _hotPixelLevelIndex;
            }
            set
            {
                if (-1 != value)
                {
                    ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_HOT_PIXEL_INDEX, value);
                }
                _hotPixelLevelIndex = value;
            }
        }

        public double HotPixelMax
        {
            get
            {
                int hpMin = 0;
                int hpMax = 0;
                int hpDefault = 0;

                if (1 == ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_HOT_PIXEL_THRESHOLD_VALUE, ref hpMin, ref hpMax, ref hpDefault))
                {
                    _hotPixelMax = hpMax;

                }
                return _hotPixelMax;
            }
        }

        public double HotPixelMin
        {
            get
            {
                int hpMin = 0;
                int hpMax = 0;
                int hpDefault = 0;

                if (1 == ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_HOT_PIXEL_THRESHOLD_VALUE, ref hpMin, ref hpMax, ref hpDefault))
                {
                    _hotPixelMin = hpMin;

                }
                return _hotPixelMin;
            }
        }

        public double HotPixelVal
        {
            get
            {
                double hotPixelVal = 0.0;
                int val = 0;
                int hpMin = 0;
                int hpMax = 0;
                int hpDefault = 0;

                if (1 == ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_HOT_PIXEL_THRESHOLD_VALUE, ref hpMin, ref hpMax, ref hpDefault))
                {
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_HOT_PIXEL_THRESHOLD_VALUE, ref val);
                    if (0 != (hpMax - hpMin))
                    {
                        //hpMin is the most aggressive hot pixel correction value. hpMin = 100%
                        hotPixelVal = (100.00 * (val - hpMin) / (hpMin - hpMax)) + 100.00;
                    }
                }

                return hotPixelVal;
            }
            set
            {
                int hpMin = 0;
                int hpMax = 0;
                int hpDefault = 0;

                if (1 == ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_HOT_PIXEL_THRESHOLD_VALUE, ref hpMin, ref hpMax, ref hpDefault))
                {
                    //hpMin is the most aggressive hot pixel correction value. hpMin = 100%
                    int val = Convert.ToInt32(((hpMin - hpMax) * (value - 100.00) / 100.00) + hpMin);
                    ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_HOT_PIXEL_THRESHOLD_VALUE, val);
                }
            }
        }

        public bool IsAutoExposureSupported
        {
            get
            {
                if(1 == ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IS_AUTOEXPOSURE_SUPPORTED))
                {
                    int value = 0;
                    ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IS_AUTOEXPOSURE_SUPPORTED, ref value);
                    return value != 0;
                }
                return false;
            }
        }

        public bool IsColorGainsSupported
        {
            get
            {
                bool isR = 0 != ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_RED_GAIN);
                bool isG = 0 != ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_GREEN_GAIN);
                bool isB = 0 != ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_BLUE_GAIN);
                return isR & isG & isB;
            }
        }

        public bool IsContinuousWhiteBalanceEnabled
        {
            get
            {
                int isContinuousWhiteBalanceEnabled = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IS_CONTINUOUS_WHITE_BALANCE_ENABLED, ref isContinuousWhiteBalanceEnabled);
                return isContinuousWhiteBalanceEnabled > 0;
            }

            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_IS_CONTINUOUS_WHITE_BALANCE_ENABLED, value ? 1 : 0);
            }
        }

        public bool IsEqualExposurePulseEnabled
        {
            get
            {
                int isEnabled = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_EEP_ENABLE, ref isEnabled);
                return 0 != isEnabled;
            }

            set
            {
                int intValue = value ? 1 : 0;
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_EEP_ENABLE, intValue);
            }
        }

        public bool IsEqualExposurePulseSupported
        {
            get
            {
                bool isSupported = 0 != ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_EEP_ENABLE);
                return isSupported;
            }
        }

        public bool IsNirBoostEnabled
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_NIR_BOOST, ref val);
                return val != 0;
            }

            set
            {
                int intValue = value ? 1 : 0;
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_NIR_BOOST, intValue);
            }
        }

        public bool IsNirBoostSupported
        {
            get
            {
                return 0 != ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_NIR_BOOST);
            }
        }

        public int Left
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_LEFT, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_LEFT, value);
            }
        }

        public int LeftMax
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_LEFT, ref exMin, ref exMax, ref exDefault);

                return exMax;
            }
        }

        public int LeftMin
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_LEFT, ref exMin, ref exMax, ref exDefault);

                return exMin;
            }
        }

        public int LightMode
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LIGHT_MODE, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LIGHT_MODE, value);
            }
        }

        public int LightModeMax
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                if (1 == ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LIGHT_MODE, ref exMin, ref exMax, ref exDefault))
                {
                    _lightModeMax = exMax;

                }
                return _lightModeMax;
            }
            set
            {
            }
        }

        public int LightModeMin
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                if (1 == ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LIGHT_MODE, ref exMin, ref exMax, ref exDefault))
                {
                    _lightModeMin = exMin;

                }
                return _lightModeMin;
            }
            set
            {
            }
        }

        public bool OrcaFrameRateEnabled
        {
            get
            {
                int val = 0;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_STATIC_FPS_ENABLE, ref val);
                return 1 == val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_STATIC_FPS_ENABLE, value ? 1 : 0);
            }
        }

        public double RedGain
        {
            get
            {
                double val = 0.0;
                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_RED_GAIN, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_RED_GAIN, value);
            }
        }

        public int Right
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_RIGHT, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_RIGHT, value);
            }
        }

        public int RightMax
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_RIGHT, ref exMin, ref exMax, ref exDefault);

                return exMax;
            }
        }

        public int RightMin
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_RIGHT, ref exMin, ref exMax, ref exDefault);

                return exMin;
            }
        }

        public int Top
        {
            get
            {
                int val = 1;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_TOP, ref val);
                return val;
            }
            set
            {
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_TOP, value);
            }
        }

        public int TopMax
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_TOP, ref exMin, ref exMax, ref exDefault);

                return exMax;
            }
        }

        public int TopMin
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAPTURE_REGION_TOP, ref exMin, ref exMax, ref exDefault);

                return exMin;
            }
        }

        #endregion Properties

        #region Methods

        public List<string> GetCamResolutionPresetsFromApplicationSettings()
        {
            List<string> presets = new List<string>();

            if (Application.Current == null)
            {
                return presets;
            }

            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList nodes = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/CameraView/ResolutionPresets/Resolution");

            foreach (XmlNode node in nodes)
            {
                try
                {
                    string preset = String.Empty;
                    string top = String.Empty;
                    string left = String.Empty;
                    string bottom = String.Empty;
                    string right = String.Empty;

                    if (XmlManager.GetAttribute(node, appSettings, "top", ref top))
                    {
                        preset += string.Format("T: {0} ", top);
                    }

                    if (XmlManager.GetAttribute(node, appSettings, "left", ref left))
                    {
                        preset += string.Format("L: {0} ", left);
                    }

                    if (XmlManager.GetAttribute(node, appSettings, "bottom", ref bottom))
                    {
                        preset += string.Format("B: {0} ", bottom);
                    }

                    if (XmlManager.GetAttribute(node, appSettings, "right", ref right))
                    {
                        preset += string.Format("R: {0}", right);
                    }

                    if (("284" == top && "704" == left && "796" == bottom && "1216" == right) || ("28" == top && "448" == left && "1052" == bottom && "1472" == right))
                    {
                        if (ActiveCameraName.Contains("CS2100"))
                        {
                            presets.Add(preset);
                        }
                    }
                    else if (("768" == top && "968" == left && "1280" == bottom && "1480" == right) || ("512" == top && "712" == left && "1536" == bottom && "1736" == right))
                    {
                        if (ActiveCameraName.Contains("CS505"))
                        {
                            presets.Add(preset);
                        }
                    }
                    else if (("824" == top && "1792" == left && "1336" == bottom && "2304" == right) || ("568" == top && "1536" == left && "1592" == bottom && "2560" == right))
                    {
                        if (ActiveCameraName.Contains("CS895"))
                        {
                            presets.Add(preset);
                        }
                    }
                    else if (("896" == top && "896" == left && "1408" == bottom && "1408" == right) || ("640" == top && "640" == left && "1664" == bottom && "1664" == right))
                    {
                        //Orca Fusion. 2304x2304 sensor
                        if (ActiveCameraName.Contains("C14440"))
                        {
                            presets.Add(preset);
                        }
                    }
                    else if (("768" == top && "768" == left && "1280" == bottom && "1280" == right) || ("512" == top && "512" == left && "1536" == bottom && "1536" == right))
                    {
                        //Orca Flash. 2048x2048 sensor
                        if (ActiveCameraName.Contains("C13440"))
                        {
                            presets.Add(preset);
                        }
                    }
                    else if (("264" == top && "440" == left && "776" == bottom && "952" == right) || ("768" == top && "768" == left && "1280" == bottom && "1280" == right) || ("980" == top && "1392" == left && "1492" == bottom && "1904" == right)
                        || ("8" == top && "184" == left && "1032" == bottom && "1208" == right) || ("512" == top && "512" == left && "1536" == bottom && "1536" == right) || ("724" == top && "1136" == left && "1748" == bottom && "2160" == right))
                    {
                        if (!CameraCSType)
                        {
                            presets.Add(preset);
                        }
                    }
                    else
                    {
                        presets.Add(preset);
                    }
                }
                catch (Exception ex)
                {
                    ex.ToString();
                }
            }

            presets.Sort((a, b) => a.CompareTo(b));
            return presets;
        }

        public List<string> GetGainDiscreteOptions()
        {
            List<string> options = new List<string>();
            int gainType = 0;
            ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_GAIN_TYPE, ref gainType);
            if (gainType == (int)GainType.DISCRETE_DECIBELS)
            {
                int minVal = 0;
                int maxVal = 0;
                int defaultVal = 0;
                ResourceManagerCS.GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_GAIN, ref minVal, ref maxVal, ref defaultVal);
                int range = maxVal - minVal + 1; // include maxVal
                double[] discreteValues = new double[range];
                // TODO: !!!WARNING!!! This function always sends the buffer as a native char* pointer. "len" here is only meaningful to the native callee if it knows to associate this param ID with this template type.
                ResourceManagerCS.GetCameraParamBuffer<double>((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_GAIN_DISCRETE_DECIBELS_VALUES, discreteValues, range);
                foreach(double discreteValue in discreteValues)
                {
                    options.Add(discreteValue.ToString());
                }
            }
            return options;
        }

        public bool IsAutoExposureRunning()
        {
            return IsAutoExposureRunning_native();
        }

        public void PerformOneShotWhiteBalance()
        {
            _ = ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_ONE_SHOT_WHITE_BALANCE_FLAG, 1);
        }

        public void SetDefaultColorGains()
        {
            double minVal = 0.0;
            double maxVal = 0.0;
            double defaultVal = 0.0;
            int[] gainParams = {(int)ICamera.Params.PARAM_CAMERA_RED_GAIN, (int)ICamera.Params.PARAM_CAMERA_GREEN_GAIN , (int)ICamera.Params.PARAM_CAMERA_BLUE_GAIN };
            int selectedCam = (int)SelectedHardware.SELECTED_CAMERA1;
            foreach (int gainParam in gainParams)
            {
                ResourceManagerCS.GetCameraParamRangeDouble(selectedCam, gainParam, ref minVal, ref maxVal, ref defaultVal);
                ResourceManagerCS.SetCameraParamDouble(selectedCam, gainParam, defaultVal);
            }
        }

        public void StartAutoExposure()
        {
            StartAutoExposure_native();
        }

        public void StopAutoExposure()
        {
            StopAutoExposure_native();
        }

        [DllImport(".\\Modules_Native\\AutoExposure.dll", EntryPoint = "IsAutoExposureRunning")]
        private static extern bool IsAutoExposureRunning_native();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "CallStartAutoExposure")]
        private static extern void StartAutoExposure_native();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "CallStopAutoExposure")]
        private static extern void StopAutoExposure_native();

        #endregion Methods
    }
}