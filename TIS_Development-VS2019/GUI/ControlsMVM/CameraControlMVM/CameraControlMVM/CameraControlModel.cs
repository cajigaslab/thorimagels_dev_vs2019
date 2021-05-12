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

    public class CameraControlModel
    {
        #region Fields

        private string _activeCameraName = string.Empty;
        private int _binIndex;
        private bool _cameraCSType = false;
        private double _exposureTimeMax;
        private double _exposureTimeMin;
        private int _hotPixelLevelIndex;
        private double _hotPixelMax;
        private double _hotPixelMin;
        private int _lightModeMax;
        private int _lightModeMin;
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

                    if ("284" == top && "704" == left && "796" == bottom && "1216" == right)
                    {
                        if (ActiveCameraName.Contains("CS2100"))
                        {
                            presets.Add(preset);
                        }
                    }
                    else if ("768" == top && "968" == left && "1280" == bottom && "1480" == right)
                    {
                        if (ActiveCameraName.Contains("CS505"))
                        {
                            presets.Add(preset);
                        }
                    }
                    else if ("824" == top && "1792" == left && "1336" == bottom && "2304" == right)
                    {
                        if (ActiveCameraName.Contains("CS895"))
                        {
                            presets.Add(preset);
                        }
                    }
                    else if (("264" == top && "440" == left && "776" == bottom && "952" == right) || ("768" == top && "768" == left && "1280" == bottom && "1280" == right) || ("980" == top && "1392" == left && "1492" == bottom && "1904" == right))
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

        #endregion Methods
    }
}