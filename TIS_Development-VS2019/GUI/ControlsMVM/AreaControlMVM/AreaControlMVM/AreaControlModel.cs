namespace AreaControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Linq;

    using ThorLogging;

    using ThorSharedTypes;

    public class AreaControlModel
    {
        #region Fields

        const int FIELDSIZE_CNT = 256;

        private double _coeff1;
        private double _coeff2;
        private double _coeff3;
        private double _coeff4;
        private int _enableBackgroundSubtraction;
        private int _enableFlatField;
        private int _enablePincushionCorrection;
        private int _LSMAreaMode = 0;
        private double _lsmField2Theta = 0.0901639344; //default: 8KHz
        private string _pathBackgroundSubtraction = string.Empty;
        private string _pathFlatField = string.Empty;
        private int _xMax;
        private int _yMax;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the AreaControlModel class
        /// </summary>
        public AreaControlModel()
        {
            _xMax = 4096;
            _yMax = 4096;
            _coeff1 = 0;
            _coeff2 = 0;
            _coeff3 = 0;
            _coeff4 = 0;
            _enablePincushionCorrection = 0;
            _enableBackgroundSubtraction = 0;
            _enableFlatField = 0;
            ZoomArray = new int[FIELDSIZE_CNT];
            LoadZoomCal();
        }

        #endregion Constructors

        #region Properties

        public int CameraType
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_TYPE, ref val);
                return val;
            }
        }

        public double CamSensorPixelSizeUM
        {
            get
            {
                double val = 1.0;
                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_PIXEL_SIZE, ref val);
                return val;
            }
        }

        public double Coeff1
        {
            get
            {
                return _coeff1;
            }
            set
            {
                _coeff1 = value;
                SetPincushionCoefficients(_coeff1, _coeff2, _coeff3, _coeff4);
            }
        }

        public double Coeff2
        {
            get
            {
                return _coeff2;
            }
            set
            {
                _coeff2 = value;
                SetPincushionCoefficients(_coeff1, _coeff2, _coeff3, _coeff4);
            }
        }

        public double Coeff3
        {
            get
            {
                return _coeff3;
            }
            set
            {
                _coeff3 = value;
                SetPincushionCoefficients(_coeff1, _coeff2, _coeff3, _coeff4);
            }
        }

        public double Coeff4
        {
            get
            {
                return _coeff4;
            }
            set
            {
                _coeff4 = value;
                SetPincushionCoefficients(_coeff1, _coeff2, _coeff3, _coeff4);
            }
        }

        public int EnableBackgroundSubtraction
        {
            get
            {
                return _enableBackgroundSubtraction;
            }
            set
            {
                _enableBackgroundSubtraction = value;
                SetBackgroundSubtractionEnable(_enableBackgroundSubtraction);
            }
        }

        public int EnableFlatField
        {
            get
            {
                return _enableFlatField;
            }
            set
            {
                _enableFlatField = value;
                SetFlatFieldEnable(_enableFlatField);
            }
        }

        public int EnableReferenceChannel
        {
            get;
            set;
        }

        public double FieldSizeCalibrationXML
        {
            get
            {
                double val = 0.0;
                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE_CALIBRATION_XML, ref val);
                return val;
            }
        }

        public bool GGSuperUserMode
        {
            get
            {
                int val = 0;
                if (1 == GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_GG_SUPER_USER, ref val))
                {
                    return (val == 1);
                }
                else
                {
                    return false;
                }
            }
            set
            {
                int val = value ? 1 : 0;
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_GG_SUPER_USER, val);
            }
        }

        public bool IsRectangleAreaModeAvailable
        {
            get
            {
                int areaMode = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AREAMODE, ref areaMode);

                int isReadOnly = ThorSharedTypes.ResourceManagerCS.GetCameraParamReadOnly((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AREAMODE);

                if (ICamera.LSMAreaMode.RECTANGLE != (ICamera.LSMAreaMode)areaMode && 1 == isReadOnly)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }

        public bool LockFieldOffset
        {
            get;
            set;
        }

        public int LSM1xFieldSize
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_1X_FIELD_SIZE, ref val);
                return val;
            }
        }

        /*
         * LSMAreaMode uses the following logic to set the PARAM_LSM_AREAMODE to the correct value. The camera ignores the
         * Kymograph mode, and only sees as if it was the rectangle mode. This was moved to the Model level so the lower
         * level can see the correct area mode, including Kymograph mode.
         */
        public int LSMAreaMode
        {
            get
            {
                if ((int)ICamera.LSMAreaMode.LINE_TIMELAPSE == _LSMAreaMode || (int)ICamera.LSMAreaMode.LINE == _LSMAreaMode)
                {
                    return _LSMAreaMode;
                }
                else
                {
                    int val = 0;
                    GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AREAMODE, ref val);
                    return val;
                }
            }
            set
            {
                _LSMAreaMode = value;
                if (ICamera.LSMAreaMode.SQUARE == (ICamera.LSMAreaMode)value ||
                    ICamera.LSMAreaMode.RECTANGLE == (ICamera.LSMAreaMode)value ||
                    ICamera.LSMAreaMode.POLYLINE == (ICamera.LSMAreaMode)value)
                {
                    SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AREAMODE, value);
                }
                else
                {
                    int val = (int)ICamera.LSMAreaMode.RECTANGLE;
                    SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_AREAMODE, val);
                }
            }
        }

        public string LSMAreaName
        {
            get
            {
                string str = string.Empty;
                int type = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref type);

                switch ((ICamera.LSMType)type)
                {
                    case ICamera.LSMType.GALVO_RESONANCE: str = "Galvo/Resonance Area Control"; break;
                    case ICamera.LSMType.GALVO_GALVO: str = "Galvo/Galvo Area Control"; break;
                    case ICamera.LSMType.RESONANCE_GALVO_GALVO: str = "RGG Area Control"; break;
                }

                return str;
            }
        }

        public double LSMField2Theta
        {
            get { return _lsmField2Theta; }
            set { _lsmField2Theta = value; }
        }

        public int LSMFieldOffsetX
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_OFFSET_X, ref val);
                return val;
            }
            set
            {
                int testVal = value;

                if (LockFieldOffset)
                {
                    testVal = 0;
                }
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_OFFSET_X, testVal);
            }
        }

        public bool  IsLSMFieldOffsetXAvailable
        {
            get
            {
                int val = ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_OFFSET_X);

                return val == 1;
            }
        }

        public double LSMFieldOffsetXFine
        {
            get
            {
                double val = 0.0;
                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_X, ref val);
                return val;
            }
            set
            {
                double val = Math.Round(value, 3); ;
                SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_X, val);
            }
        }

        public int LSMFieldOffsetY
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_OFFSET_Y, ref val);
                return val;
            }
            set
            {
                int testVal = value;
                if (LockFieldOffset)
                {
                    testVal = 0;
                }

                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_OFFSET_Y, testVal);
            }
        }

        public int LSMFieldOffsetYActual
        {
            get;
            set;
        }

        public double LSMFieldOffsetYFine
        {
            get
            {
                double val = 0.0;
                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_Y, ref val);
                return val;
            }
            set
            {
                double val = Math.Round(value, 3); ;
                SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_Y, val);
            }
        }

        public double LSMFieldScaleXFine
        {
            get
            {
                double val = 0.0;
                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_X, ref val);
                return val;
            }
            set
            {
                double val = Math.Round(value, 3); ;
                SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_X, val);
            }
        }

        public double LSMFieldScaleYFine
        {
            get
            {
                double val = 0.0;
                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y, ref val);
                return val;
            }
            set
            {
                double val = Math.Round(value, 3); ;
                SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y, val);
            }
        }

        public int LSMFieldSize
        {
            get
            {
                // value from LSM is primary,
                //user has to update control unit if different.
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, ref val);

                return val;
            }
            set
            {
                //Set the camera field size first, so Thordaq is ready to change and doesn't get stuck waiting for frame
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, value);
                //Need to sleep 200ms for the resonant to settle at the smaller field sizes. Otherwise the timing might leave Thordaq waiting for a frame
                //200ms was found testing at 2K x 2K in one-way
                System.Threading.Thread.Sleep(200);
                SetDeviceParamInt((int)SelectedHardware.SELECTED_CONTROLUNIT, (int)IDevice.Params.PARAM_SCANNER_ZOOM_POS, value, false);
                // set the two way alignment to its current dispayed value
                MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignmentCoarse"] = (int)MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignmentCoarse", (object)0];
                MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignment"] = (int)MVMManager.Instance["ScanControlViewModel", "LSMTwoWayAlignment", (object)0];
            }
        }

        public int LSMFieldSizeMax
        {
            get
            {
                int fMin = 0;
                int fMax = 0;
                int fDefault = 0;

                //Max and min are configurable in LSMs:
                GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, ref fMin, ref fMax, ref fDefault);

                return fMax;
            }
        }

        public int LSMFieldSizeMin
        {
            get
            {
                int fMin = 0;
                int fMax = 0;
                int fDefault = 0;

                //Max and min are configurable in LSMs:
                GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, ref fMin, ref fMax, ref fDefault);

                return fMin;
            }
        }

        /// <summary>
        /// Gets or sets the LSM flip horizontal.
        /// </summary>
        /// <value>The LSM flip horizontal.</value>
        public int LSMFlipHorizontal
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_HORIZONTAL_FLIP, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_HORIZONTAL_FLIP, value);
            }
        }

        public int LSMFlipVerticalScan
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_VERTICAL_SCAN_DIRECTION, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_VERTICAL_SCAN_DIRECTION, value);
            }
        }

        public int LSMLineScanEnable
        {
            get
            {
                int val = 0;

                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_GALVO_ENABLE, ref val);

                val = (0 == val) ? 1 : 0;
                return val;
            }
            set
            {
                int val = 0;
                val = (0 == value) ? 1 : 0;
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_GALVO_ENABLE, val);
            }
        }

        public int LSMPixelX
        {
            get
            {
                int val = 1;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_X, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_X, value);

                switch ((ICamera.LSMAreaMode)LSMAreaMode)
                {
                    case ICamera.LSMAreaMode.SQUARE:
                        {
                            SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_Y, value);
                        }
                        break;
                    case ICamera.LSMAreaMode.LINE_TIMELAPSE:
                        {
                            SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_Y, 1);
                        }
                        break;
                    case ICamera.LSMAreaMode.RECTANGLE:
                        {
                            if (LSMPixelY > LSMPixelX)
                            {
                                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_Y, value);
                            }
                        }
                        break;
                }
            }
        }

        public int LSMPixelXMax
        {
            get
            {
                int xMin = 0;
                int xMax = 0;
                int xDefault = 0;

                GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_X, ref xMin, ref xMax, ref xDefault);

                if (null == MVMManager.Instance["CaptureSetupViewModel"])
                    return xMax;

                //restrict the pixel density in color mode if not RGG
                //based on the stored limit value
                if ((int)MVMManager.Instance["CaptureSetupViewModel", "LSMChannel"] == 4 &&
                    (int)ICamera.LSMType.RESONANCE_GALVO_GALVO != ResourceManagerCS.GetLSMType())
                {
                    return Math.Min(xMax, _xMax);
                }
                else
                {
                    return xMax;
                }
            }
            set
            {
                _xMax = value;
            }
        }

        public int LSMPixelXMin
        {
            get
            {
                int xMin = 0;
                int xMax = 0;
                int xDefault = 0;

                GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_X, ref xMin, ref xMax, ref xDefault);

                return xMin;
            }
        }

        public int LSMPixelY
        {
            get
            {
                int val = 1;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_Y, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_Y, value);
            }
        }

        public int LSMPixelYMax
        {
            get
            {
                int yMin = 0;
                int yMax = 0;
                int yDefault = 0;

                GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_Y, ref yMin, ref yMax, ref yDefault);

                if (null == MVMManager.Instance["CaptureSetupViewModel"])
                    return yMax;

                //restrict the pixel density in color mode if not RGG
                //based on the stored limit value
                if ((int)ICamera.LSMType.RESONANCE_GALVO_GALVO != ResourceManagerCS.GetLSMType() && (int)MVMManager.Instance["CaptureSetupViewModel", "LSMChannel"] == 4)
                {
                    return Math.Min(yMax, _yMax);
                }
                else
                {
                    return yMax;
                }
            }

            set
            {
                _yMax = value;
            }
        }

        public int LSMPixelYMin
        {
            get
            {
                int yMin = 0;
                int yMax = 0;
                int yDefault = 0;

                GetCameraParamRangeInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_Y, ref yMin, ref yMax, ref yDefault);
                return yMin;
            }
        }

        public int LSMPixelYMultiple
        {
            get
            {
                int val = 1;
                if (1 == GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_PIXEL_Y_MULTIPLE, ref val))
                {
                    return val;
                }
                else
                {
                    return 32;
                }
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

        public double LSMScanAreaAngle
        {
            get
            {
                double val = 0.0;
                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_SCANAREA_ANGLE, ref val);
                return val;
            }
            set
            {
                double val = value;
                SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_SCANAREA_ANGLE, val);
            }
        }

        public int MROIModeEnable
        {
            get
            {
                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_MROI_MODE_ENABLE, ref val);
                return val;
            }
            set
            {
                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_MROI_MODE_ENABLE, value);
            }
        }

        public string PathBackgroundSubtraction
        {
            get
            {
                return _pathBackgroundSubtraction;
            }
            set
            {
                _pathBackgroundSubtraction = value;
                SetBackgroundSubtractionFile(_pathBackgroundSubtraction);
            }
        }

        public string PathFlatField
        {
            get
            {
                return _pathFlatField;
            }
            set
            {
                _pathFlatField = value;
                SetFlatFieldFile(_pathFlatField);
            }
        }

        public int PincushionCorrection
        {
            get
            {
                return _enablePincushionCorrection;
            }
            set
            {
                _enablePincushionCorrection = value;
                SetImageCorrectionEnable(value);
            }
        }

        public int RSInitMode
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_CONTROLUNIT, (int)IDevice.Params.PARAM_SCANNER_INIT_MODE, ref val);
                return val;
            }
            set
            {
                SetRSInitMode(value);
            }
        }

        public int RSLineConfigured
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_EPHYS, (int)IDevice.Params.PARAM_EPHYS_MEASURE_CONFIGURE, ref val);
                return val;
            }
        }

        public bool RSLineProbeOn
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_EPHYS, (int)IDevice.Params.PARAM_EPHYS_MEASURE, ref val);
                return (1 == val);
            }
            set
            {
                int val = (value) ? 1 : 0;
                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_EPHYS, (int)IDevice.Params.PARAM_EPHYS_MEASURE, val, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
            }
        }

        public double RSLineRate
        {
            get
            {
                double val = 0.0;
                ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_EPHYS, (int)IDevice.Params.PARAM_EPHYS_MEASURE_RATE, ref val);
                return val;
            }
        }

        public bool TimeBasedLineScan
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TIME_BASED_LINE_SCAN, ref val);
                return (1 == val);
            }
            set
            {
                int val = (value) ? 1 : 0;
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TIME_BASED_LINE_SCAN, val);
            }
        }

        public double TimeBasedLineScanTimeIncrementMS
        {
            get
            {
                double val = 0.0;
                if (1 == GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TIME_BASED_LINE_SCAN_INCREMENT_TIME_MS, ref val))
                {
                    return val;
                }
                else
                {
                    return 1;
                }
            }
        }

        public double TimeBasedLSTimeMS
        {
            get
            {
                double val = 0.0;
                GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TB_LINE_SCAN_TIME_MS, ref val);
                return val;
            }
            set
            {
                double val = Math.Round(value, 3); ;
                SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TB_LINE_SCAN_TIME_MS, val);
            }
        }

        public int[] ZoomArray
        {
            get;
        }

        #endregion Properties

        #region Methods

        public int CenterROI()
        {
            SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_OFFSET_X, 0);
            SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_OFFSET_Y, 0);
            return 0;
        }

        public void CenterScanners(int selectedCamera, bool centerWithOffset)
        {
            int centerOffset = (centerWithOffset) ? 1 : 0;
            SetCameraParamInt(selectedCamera, (int)ICamera.Params.PARAM_LSM_CENTER_WITH_OFFSET, centerOffset);
            CenterLSMScanners(selectedCamera);
        }

        public void CloseShutter()
        {
            MVMManager.Instance["PowerControlViewModel", "ShutterPosition"] = 1;
            ResourceManagerCS.PostflightCamera((int)SelectedHardware.SELECTED_CAMERA1);
            TurnOffLaser();
        }

        /// <summary>
        /// Returns the xaml path in application settings for the resolution presets for the input area mode
        /// </summary>
        /// <param name="mode"> The area mode for which to get the application settings resolution presets path </param>
        /// <returns> A string representing the xaml path to the resolution presets </returns>
        public string GetResolutionPresetPathForAreaMode(ICamera.LSMAreaMode mode)
        {
            switch (mode)
            {
                case ICamera.LSMAreaMode.SQUARE:
                    return "/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView/ResolutionPresets/SquareResolutionPresets/Resolution";
                case ICamera.LSMAreaMode.RECTANGLE:
                    return "/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView/ResolutionPresets/RectangleResolutionPresets/Resolution";
                case ICamera.LSMAreaMode.LINE:
                    return "/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView/ResolutionPresets/LineResolutionPresets/Resolution";
                //case ICamera.LSMAreaMode.POLYLINE:
                //    return "/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView/ResolutionPresets/PolylineResolutionPresets/Resolution";
                case ICamera.LSMAreaMode.LINE_TIMELAPSE:
                    return "/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView/ResolutionPresets/KymographResolutionPresets/Resolution";
                default:
                    return "/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView/ResolutionPresets/SquareResolutionPresets/Resolution";
            }
        }

        /// <summary>
        /// Reads the resolution presets for the input area mode from application settings.
        /// </summary>
        /// <param name="mode"> The area mode to get presets for </param>
        /// <returns> A sorted list of all resolution presets listed in application settings </returns>>
        public List<ImageResolution> GetResolutionPresetsFromApplicationSettings(ICamera.LSMAreaMode mode, bool isRectangleAreaModeAvailable = true)
        {
            List<ImageResolution> presets = new List<ImageResolution>();

            if (Application.Current == null)
            {
                return presets;
            }

            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            var areaMode = isRectangleAreaModeAvailable ? mode : ICamera.LSMAreaMode.SQUARE;

            XmlNodeList nodes = doc.SelectNodes(GetResolutionPresetPathForAreaMode(areaMode));
            foreach (XmlNode node in nodes)
            {
                try
                {
                    int width = 0;
                    int height = 0;
                    string value = String.Empty;
                    if (XmlManager.GetAttribute(node, doc, "width", ref value))
                    {
                        if (!String.IsNullOrEmpty(value))
                        {
                            width = int.Parse(value);
                        }
                    }
                    if (XmlManager.GetAttribute(node, doc, "height", ref value))
                    {
                        if (!String.IsNullOrEmpty(value))
                        {
                            height = int.Parse(value);
                        }
                    }

                    ImageResolution preset = new ImageResolution(width, height);
                    if (!preset.HasInvalidPixelDimension())
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

        public int LIGetFieldSizeCalibration(ref double fieldSizeCal)
        {
            return GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE_CALIBRATION, ref fieldSizeCal);
        }

        /*
         * Summary: Add the saved resolution to the xml file under the correspondent tag.
         *
         * Params: None.
         *
         * Returns: void.
         */
        public void ResolutionAdd()
        {
            List<ImageResolution> presets = new List<ImageResolution>();

            presets = GetResolutionPresetsFromApplicationSettings((ICamera.LSMAreaMode)LSMAreaMode);

            ImageResolution newRes = new ImageResolution(LSMPixelX, LSMPixelY);

            if (presets.Contains(newRes))
            {
                return;
            }

            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList nodes = doc.SelectNodes(GetResolutionPresetPathForAreaMode((ICamera.LSMAreaMode)LSMAreaMode));//(AreaMode)LSMAreaMode));

            if (nodes.Count > 0)
            {
                XmlNode resolutionTag = doc.CreateElement("Resolution");

                nodes[0].ParentNode.AppendChild(resolutionTag);

                XmlManager.SetAttribute(resolutionTag, doc, "width", LSMPixelX.ToString());
                XmlManager.SetAttribute(resolutionTag, doc, "height", LSMPixelY.ToString());

                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);

            }
        }

        public bool ROIZoomInLine(Point point1, Point point2)
        {
            double areaAngle = LSMScanAreaAngle;
            if (ResourceManagerCS.GetLSMType() == (int)ICamera.LSMType.GALVO_RESONANCE)
            {
                return false;
            }
            else
            {
                if (1 == LSMFlipHorizontal)
                {
                    double tempx1 = LSMPixelX - point2.X;
                    double tempx2 = LSMPixelX - point1.X;
                    point1.X = tempx1;
                    point2.X = tempx2;
                }
            }

            int x1 = (int)Math.Floor(point1.X);
            int x2 = (int)Math.Floor(point2.X);
            int y1 = (int)Math.Floor(point1.Y);
            int y2 = (int)Math.Floor(point2.Y);

            double centerX = (x1 + x2) / 2.0;
            double centerY = (y1 + y2) / 2.0 + (LSMPixelX / 2.0 - LSMPixelY / 2.0);
            int lineLen = (int)Math.Round(Math.Sqrt(Math.Pow((x2 - x1 + 1), 2) + Math.Pow((y2 - y1 + 1), 2)));
            if (0 == lineLen) return false;

            double offsetPx = (centerX - (LSMPixelX / 2.0));
            double offsetPy = (centerY - (LSMPixelX / 2.0));
            double offsetFx = offsetPx * ((double)LSMFieldSize / (double)LSMPixelX);
            double offsetFy = offsetPy * ((double)LSMFieldSize / (double)LSMPixelX);

            double offsetX = offsetFx * Math.Cos(areaAngle * Math.PI / 180.0) - offsetFy * Math.Sin(areaAngle * Math.PI / 180.0);
            double offsetY = offsetFx * Math.Sin(areaAngle * Math.PI / 180.0) + offsetFy * Math.Cos(areaAngle * Math.PI / 180.0);

            int tmpFieldSize = (int)Math.Round(((double)LSMFieldSize * lineLen / (double)LSMPixelX));

            LSMFieldSize = tmpFieldSize;

            double xyRatio = (double)LSMPixelY / (double)LSMPixelX;

            LSMFieldOffsetX = (int)(-Math.Round(LSMFieldScaleXFine * offsetX) + LSMFieldOffsetX);
            LSMFieldOffsetY = (int)(Math.Round(LSMFieldScaleYFine * offsetY) + LSMFieldOffsetY);

            return true;
        }

        public bool ROIZoomInPolyline(List<Point> points)
        {
            if (points.Count < 2)
            {
                return false;
            }

            int[] PtX = new int[points.Count];
            int[] Pty = new int[points.Count];

            if (1 == LSMFlipHorizontal)
            {
                for (int i = 0; i < points.Count; i++)
                {
                    PtX[i] = LSMPixelX - (int)points[i].X;
                    Pty[i] = (int)points[i].Y;
                }
            }
            else
            {
                for (int i = 0; i < points.Count; i++)
                {
                    PtX[i] = (int)points[i].X;
                    Pty[i] = (int)points[i].Y;
                }
            }

            int newPixelX = LSMPixelX;
            if (0 == VerifyPolyLine(PtX, Pty, (int)points.Count, LSMFieldSize, LSMField2Theta, LSMFieldScaleXFine, LSMFieldScaleYFine, LSMPixelY, ref newPixelX))
            {
                return false;
            }

            if (LSMPixelXMax < newPixelX || LSMPixelXMin > newPixelX)
            {
                return false;
            }

            int ONE_WAY_SCANMODE = 1;
            MVMManager.Instance["ScanControlViewModel", "LSMScanMode"] = ONE_WAY_SCANMODE;
            LSMPixelX = newPixelX;
            return true;
        }

        public bool RoiZoomInRect(Point tl, Point br)
        {
            double areaAngle = LSMScanAreaAngle;

            double fieldScaleXFine = LSMFieldScaleXFine;
            double fieldScaleYFine = LSMFieldScaleYFine;

            if (ResourceManagerCS.GetLSMType() == (int)ICamera.LSMType.GALVO_RESONANCE)
            {
                double Mx = Math.Max(Math.Abs(tl.X - (LSMPixelX / 2)), Math.Abs(br.X - (LSMPixelX / 2)));
                tl.X = (LSMPixelX / 2) - Mx;
                br.X = (LSMPixelX / 2) + Mx;
                areaAngle = 0;
                fieldScaleXFine = 1.0;
                fieldScaleYFine = 1.0;
            }
            else
            {
                if (1 == LSMFlipHorizontal)
                {
                    double tlX = LSMPixelX - br.X;
                    double brX = LSMPixelX - tl.X;
                    tl.X = tlX;
                    br.X = brX;
                }
            }

            int x1 = (int)Math.Floor(tl.X);
            int x2 = (int)Math.Floor(br.X);
            int y1 = (int)Math.Floor(tl.Y);
            int y2 = (int)Math.Floor(br.Y);

            double l = Math.Max(x2 - x1, y2 - y1);
            if (l == 0) return false;

            double centerX = (x1 + x2) / 2;
            double centerY = (y1 + y2) / 2 + (LSMPixelX / 2 - LSMPixelY / 2);
            int lineLen = (int)Math.Floor(Math.Sqrt(Math.Pow((x2 - x1 + 1), 2) + Math.Pow((y2 - y1 + 1), 2)));

            double offsetPx = (centerX - (LSMPixelX / 2));
            double offsetPy = (centerY - (LSMPixelX / 2));
            double offsetFx = offsetPx * ((double)LSMFieldSize / (double)LSMPixelX);
            double offsetFy = offsetPy * ((double)LSMFieldSize / (double)LSMPixelX);

            int offsetX = (int)(Math.Round(offsetFx * Math.Cos(areaAngle * Math.PI / 180) - offsetFy * Math.Sin(areaAngle * Math.PI / 180), 0));
            int offsetY = (int)(Math.Round(offsetFx * Math.Sin(areaAngle * Math.PI / 180) + offsetFy * Math.Cos(areaAngle * Math.PI / 180), 0));

            int tmpFieldSize = (int)Math.Round(((double)LSMFieldSize * Math.Abs(x2 - x1) / (double)LSMPixelX), 0);
            LSMFieldSize = tmpFieldSize;

            double xyRatio = (double)LSMPixelY / (double)LSMPixelX;

            LSMFieldOffsetX = (int)(-Math.Round(fieldScaleXFine * offsetX) + LSMFieldOffsetX);
            LSMFieldOffsetY = (int)(Math.Round(fieldScaleYFine * offsetY) + LSMFieldOffsetY);

            return true;
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "CenterLSMScanners")]
        private static extern bool CenterLSMScanners(int selectedCamera);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamDouble")]
        private static extern int GetCameraParamDouble(int cameraSelection, int param, ref double value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamLong")]
        private static extern int GetCameraParamInt(int cameraSelection, int param, ref int value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamRangeLong")]
        private static extern int GetCameraParamRangeInt(int cameraSelection, int paramId, ref int valMin, ref int valMax, ref int valDefault);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImage")]
        private static extern bool ReadImage([MarshalAs(UnmanagedType.LPWStr)]string path, ref IntPtr outputBuffer);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo")]
        private static extern bool ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)]string selectedFileName, ref long width, ref long height, ref long colorChannels);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetBackgroundSubtractionEnable")]
        private static extern bool SetBackgroundSubtractionEnable(int enable);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetBackgroundSubtractionFile")]
        private static extern bool SetBackgroundSubtractionFile([MarshalAs(UnmanagedType.LPWStr)]string path);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamDouble")]
        private static extern int SetCameraParamDouble(int cameraSelection, int param, double value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamLong")]
        private static extern int SetCameraParamInt(int cameraSelection, int param, int value);

        private static void SetChannelFromEnable()
        {
            //update the channel value also
            int chan = (Convert.ToInt32(MVMManager.Instance["CaptureSetupViewModel", "LSMChannelEnable0"])) | (Convert.ToInt32(MVMManager.Instance["CaptureSetupViewModel", "LSMChannelEnable1"]) << 1) | (Convert.ToInt32(MVMManager.Instance["CaptureSetupViewModel", "LSMChannelEnable2"]) << 2) | (Convert.ToInt32(MVMManager.Instance["CaptureSetupViewModel", "LSMChannelEnable3"]) << 3);

            int val = 1;
            switch (chan)
            {
                case 1: val = 1; break;
                case 2: val = 2; break;
                case 4: val = 4; break;
                case 8: val = 8; break;
                default:
                    {
                        val = 0xf;
                    }
                    break;
            }

            SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CHANNEL, val);
            SetDisplayChannels(chan);
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamLong")]
        private static extern int SetDeviceParamInt(int deviceSelection, int paramId, int param, bool wait);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetDisplayChannels")]
        private static extern bool SetDisplayChannels(int chan);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetFlatFieldEnable")]
        private static extern bool SetFlatFieldEnable(int enable);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetFlatFieldFile")]
        private static extern bool SetFlatFieldFile([MarshalAs(UnmanagedType.LPWStr)]string path);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetImageCorrectionEnable")]
        private static extern bool SetImageCorrectionEnable(int enable);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetPincushionCoefficients")]
        private static extern bool SetPincushionCoefficients(double k1, double k2, double k3, double k4);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetRSInitMode")]
        private static extern bool SetRSInitMode(int mode);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "TurnOffLaser")]
        private static extern bool TurnOffLaser();

        [DllImport(".\\Modules_Native\\GeometryUtilitiesCPP.dll", EntryPoint = "VerifyPolyLine")]
        private static extern int VerifyPolyLine(int[] PointX, int[] PointY, int count, int fieldSize, double field2Volts, double fieldScaleFineX, double fieldScaleFineY, int PixelY, ref int PixelX);

        private void LoadZoomCal()
        {
            string path = System.IO.Directory.GetCurrentDirectory();
            string zoomFile = path + "\\ZoomData.txt";
            if (!File.Exists(zoomFile))
                return;
            {
                List<int[]> fieldSizePercentage = File.ReadLines(zoomFile).Select(line => line.Split(' ').Select(s => int.Parse(s)).ToArray()).ToList();

                if (FIELDSIZE_CNT != fieldSizePercentage.Count)
                {
                    ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Warning, 1, "Number of calibrated field size is not 256 but: " + fieldSizePercentage.Count);
                    return;
                }

                for (int i = 0; i < FIELDSIZE_CNT; ++i)
                {
                    if (fieldSizePercentage[i]?.Length > 0)
                    {
                        ZoomArray[i] = fieldSizePercentage[i][0];
                    }
                }
            }
        }

        #endregion Methods
    }
}