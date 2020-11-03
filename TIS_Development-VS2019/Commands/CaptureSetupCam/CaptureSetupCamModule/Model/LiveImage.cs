namespace CaptureSetupDll.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
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

    public class LiveImage
    {
        #region Fields

        public const int MAX_CHANNELS = 4;

        private const int PIXEL_DATA_HISTOGRAM_SIZE = 256;

        private static double[] _blackPoint;
        private static CameraType _camType;
        private static int[] _colorAssigment;
        private static int _colorChannels;
        private static int _dataHeight;
        private static int _dataLength;
        private static int _dataWidth;
        private static double _framesPerSecond = 0;
        private static ReportNewImage _imageCallBack;
        private static LiveImageDataCustomParams _liParams = new LiveImageDataCustomParams();
        private static bool[] _lsmChannelEnable;
        private static byte[][] _pal;
        private static bool _paletteChanged;
        private static short[] _pixelData;
        private static byte[] _pixelDataByte;
        private static int[][] _pixelDataHistogram;
        private static byte[] _pixelDataLUT;
        private static bool _pixelDataReady;
        private static int _prevTickCount = 0;
        private static double _sumTickCount = 0;
        private static Object _thisLock;
        private static double[] _whitePoint;
        private static int _xMax;
        private static int _yMax;

        private int _bitsPerPixel;
        private BackgroundWorker _bw = new BackgroundWorker();
        private double _coeff1;
        private double _coeff2;
        private double _coeff3;
        private double _coeff4;
        private Guid _commandGuid;
        private DigitizerBoardNames _digitizerBoardName = DigitizerBoardNames.ATS460;
        private int _enableBackgroundSubtraction;
        private int _enableFlatField;
        private int _enablePincushionCorrection;
        private String _expPath;
        private int _filterPositionDic;
        private int _filterPositionEm;
        private int _filterPositionEx;
        private int _lampPosition;
        private bool _liveStartButtonStatus;
        private bool _liveStopButtonStatus;
        private string _pathBackgroundSubtraction = string.Empty;
        private string _pathFlatField = string.Empty;
        private int _rollOverPointX;
        private int _rollOverPointY;
        private int _shutterPosition;
        private int _turretPosition;
        private Color _wavelengthColor;

        #endregion Fields

        #region Constructors

        public LiveImage()
        {
            _liParams.left.val = 0;
            _liParams.right.val = 1280;
            _liParams.top.val = 0;
            _liParams.bottom.val = 1024;
            _liParams.binX.val = 1;
            _liParams.binY.val = 1;
            _liParams.lsmChannel.val = 0x01;
            _liParams.lsmFieldSize.val = 120;
            _liParams.lsmPixelX.val = 1024;
            _liParams.lsmPixelY.val = 1024;
            _liParams.lsmScanMode.val = 0;
            _colorChannels = MAX_CHANNELS;
            _liParams.exposureTimeCam0.val = 10;
            _liParams.exposureTimeCam1.val = 10;
            _liParams.exposureTimeCam2.val = 10;
            _pixelDataReady = false;
            _liveStartButtonStatus = true;
            _liveStopButtonStatus = false;
            _wavelengthColor = Colors.White;
            _turretPosition = 0;
            _lampPosition = 0;
            _shutterPosition = 1;
            _dataWidth = 1024;
            _dataHeight = 1024;
            _coeff1 = 0;
            _coeff2 = 0;
            _coeff3 = 0;
            _coeff4 = 0;
            _enablePincushionCorrection = 0;
            _enableBackgroundSubtraction = 0;
            _enableFlatField = 0;
            _lsmChannelEnable = new bool[MAX_CHANNELS];
            _xMax = 4096;
            _yMax = 4096;

            //enable the first 4 channels only
            for (int i = 0; i < 4; i++)
            {
                _lsmChannelEnable[i] = true;
            }

            _whitePoint = new double[_colorChannels];
            _blackPoint = new double[_colorChannels];

            for (int i = 0; i < _colorChannels; i++)
            {
                _whitePoint[i] = 255;
                _blackPoint[i] = 0;
            }

            _paletteChanged = true;

            //create and assign callback for C++ unmanaged updates
            _imageCallBack = new ReportNewImage(ImageUpdate);

            try
            {
                GetCommandGUID(ref _commandGuid);

                LiveImageDataSetupCommand();

                InitCallBack(_imageCallBack);

                GetCustomParamsBinary(ref _liParams);

                int numCameras = 0;
                GetNumberOfCameras(ref numCameras);

                if (CamType == CameraType.LSM)
                {
                    const int MAX_LSM_CHANNELS = 4;
                    //LSM connected
                    _colorChannels = MAX_LSM_CHANNELS;
                }
                else
                {
                    switch (numCameras)
                    {
                        case 3: _colorChannels = 3; break;
                        default: _colorChannels = 1; break;
                    }
                }

                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + string.Format(" {0} cameras found. Color channels {1}", numCameras, _colorChannels));
            }
            catch (System.DllNotFoundException)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " DllNotFoundException");
            }

            _thisLock = new Object();

            //create one instance of the histogram arrays
            _pixelDataHistogram = new int[_colorChannels][];

            for (int i = 0; i < _colorChannels; i++)
            {
               _pixelDataHistogram[i] = new int[PIXEL_DATA_HISTOGRAM_SIZE];
            }

            _pal = new byte[_colorChannels][];

            for (int i = 0; i < _colorChannels; i++)
            {
                _pal[i] = new byte[256];
            }

            _colorAssigment = new int[_colorChannels];

               ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Created");
        }

        #endregion Constructors

        #region Enumerations

        public enum AreaMode
        {
            SQUARE=0,
            RECTANGLE=1,
            LINE = 2
        }

        public enum CameraType
        {
            CCD = 0,
            LSM = 1,
            CCD_MOSAIC = 2
        }

        public enum ColorAssignments
        {
            RED,
            GREEN,
            BLUE,
            CYAN,
            MAGENTA,
            YELLOW,
            GRAY
        }

        public enum DigitizerBoardNames
        {
            ATS460,
            ATS9440
        }

        #endregion Enumerations

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportNewImage(IntPtr returnArray, ref int colorChannels);

        #endregion Delegates

        #region Events

        public event Action<bool> UpdateImage;

        public event Action<bool> UpdateMenuBarButton;

        #endregion Events

        #region Properties

        public static bool PaletteChanged
        {
            get
            {
                return _paletteChanged;
            }
            set
            {
                _paletteChanged = value;
            }
        }

        public int BitsPerPixel
        {
            get
            {
                int temp = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_BITS_PER_PIXEL, ref temp);
                _bitsPerPixel = temp;
                return _bitsPerPixel;
            }
        }

        public double BlackPoint0
        {
            get
            {
                if (_blackPoint.Length > 0)
                {
                    return _blackPoint[0];
                }
                else
                {
                    return 255;
                }
            }
            set
            {
                if (_blackPoint.Length > 0)
                {
                    _blackPoint[0] = value;
                }
                _paletteChanged = true;
            }
        }

        public double BlackPoint1
        {
            get
            {
                if (_blackPoint.Length > 1)
                {
                    return _blackPoint[1];
                }
                else
                {
                    return 255;
                }
            }
            set
            {
                if (_blackPoint.Length > 1)
                {
                    _blackPoint[1] = value;
                }
                _paletteChanged = true;
            }
        }

        public double BlackPoint2
        {
            get
            {
                if (_blackPoint.Length > 2)
                {
                    return _blackPoint[2];
                }
                else
                {
                    return 255;
                }
            }
            set
            {
                if (_blackPoint.Length > 2)
                {
                    _blackPoint[2] = value;
                }
                _paletteChanged = true;
            }
        }

        public double BlackPoint3
        {
            get
            {
                if (_blackPoint.Length > 3)
                {
                    return _blackPoint[3];
                }
                else
                {
                    return 255;
                }
            }
            set
            {
                if (_blackPoint.Length > 3)
                {
                    _blackPoint[3] = value;
                }
                _paletteChanged = true;
            }
        }

        public CameraType CamType
        {
            get
            {
                int temp = 0;
                GetCameraType(ref temp);
                _camType = (CameraType)temp;
                return (CameraType)temp;
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
                SetPincushionCoefficients(_coeff1, _coeff2,_coeff3,_coeff4);
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

        public Guid CommandGuid
        {
            get { return _commandGuid; }
        }

        public int DataHeight
        {
            get
            {
                return _dataHeight;
            }
        }

        public int DataWidth
        {
            get
            {
                return _dataWidth;
            }
        }

        public DigitizerBoardNames DigitizerBoardName
        {
            get
            {
                return _digitizerBoardName;
            }
            set
            {
                _digitizerBoardName = value;
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

        public String ExpPath
        {
            get
            {
                return _expPath;
            }

            set
            {
                _expPath = value;
            }
        }

        public int FilterPositionDic
        {
            get
            {
                return _filterPositionDic;
            }
            set
            {
                if (SetFilterPositionDic(value))
                {
                    _filterPositionDic = value;
                }
            }
        }

        public int FilterPositionEm
        {
            get
            {
                return _filterPositionEm;
            }
            set
            {
                if (SetFilterPositionEm(value))
                {
                    _filterPositionEm = value;
                }
            }
        }

        public int FilterPositionEx
        {
            get
            {
                return _filterPositionEx;
            }
            set
            {
                if (SetFilterPositionEx(value))
                {
                    _filterPositionEx = value;
                }
            }
        }

        public double FramesPerSecond
        {
            get
            {
                GetFrameRate(ref _framesPerSecond);

                return _framesPerSecond;
            }
        }

        public int[] HistogramData0
        {
            get
            {
                        return _pixelDataHistogram[0];
            }
        }

        public int[] HistogramData1
        {
            get
            {
                        return _pixelDataHistogram[1];
            }
        }

        public int[] HistogramData2
        {
            get
            {
                        return _pixelDataHistogram[2];
            }
        }

        public int[] HistogramData3
        {
            get
            {
                        return _pixelDataHistogram[3];
            }
        }

        public int ImageColorChannels
        {
            get { return _colorChannels; }
        }

        public int InputRangeChannel1
        {
            get { return _liParams.lsmInputRangeChannel1.val; }
            set
            {
                int temp = _liParams.lsmInputRangeChannel1.val;

                _liParams.lsmInputRangeChannel1.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmInputRangeChannel1.val = temp;
            }
        }

        public int InputRangeChannel2
        {
            get { return _liParams.lsmInputRangeChannel2.val; }
            set
            {
                int temp = _liParams.lsmInputRangeChannel2.val;

                _liParams.lsmInputRangeChannel2.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmInputRangeChannel2.val = temp;
            }
        }

        public int InputRangeChannel3
        {
            get { return _liParams.lsmInputRangeChannel3.val; }
            set
            {
                int temp = _liParams.lsmInputRangeChannel3.val;

                _liParams.lsmInputRangeChannel3.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmInputRangeChannel3.val = temp;
            }
        }

        public int InputRangeChannel4
        {
            get { return _liParams.lsmInputRangeChannel4.val; }
            set
            {
                int temp = _liParams.lsmInputRangeChannel4.val;

                _liParams.lsmInputRangeChannel4.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmInputRangeChannel4.val = temp;
            }
        }

        public int LampPosition
        {
            get
            {
                return _lampPosition;
            }
            set
            {
                if (SetBFLampPosition(value))
                {
                    _lampPosition = value;
                }
            }
        }

        public int Laser1Enable
        {
            get
            {
                int temp = 0;

                if (GetMCLSLaserEnable(0,ref temp))
                {
                }

                return temp;
            }
            set
            {
                SetMCLSLaserEnable(0,value);
            }
        }

        public double Laser1Max
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;

                if (GetMCLSLaserRange(0, ref tempMin, ref tempMax))
                {
                }

                return tempMax;
            }
        }

        public double Laser1Min
        {
            get
            {
                double tempMin=0;
                double tempMax=0;

                if (GetMCLSLaserRange(0, ref tempMin, ref tempMax))
                {
                }

                return tempMin;
            }
        }

        public int Laser1Position
        {
            get
            {
                int temp = 0;

                if (GetLaserPosition(0, ref temp))
                {
                }

                return temp;
            }
            set
            {
                SetLaserPosition(0, value);
            }
        }

        public double Laser1Power
        {
            get
            {
                double temp=0;

                if (GetMCLSLaserPower(0, ref temp))
                {
                }

                return temp;
            }
            set
            {
                SetMCLSLaserPower(0, value);
            }
        }

        public int Laser2Enable
        {
            get
            {
                int temp = 0;

                if (GetMCLSLaserEnable(1, ref temp))
                {
                }

                return temp;
            }
            set
            {
                SetMCLSLaserEnable(1, value);
            }
        }

        public double Laser2Max
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;

                if (GetMCLSLaserRange(1, ref tempMin, ref tempMax))
                {
                }

                return tempMax;
            }
        }

        public double Laser2Min
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;

                if (GetMCLSLaserRange(1, ref tempMin, ref tempMax))
                {
                }

                return tempMin;
            }
        }

        public double Laser2Power
        {
            get
            {
                double temp = 0;

                if (GetMCLSLaserPower(1, ref temp))
                {
                }

                return temp;
            }
            set
            {
                SetMCLSLaserPower(1, value);
            }
        }

        public int Laser3Enable
        {
            get
            {
                int temp = 0;

                if (GetMCLSLaserEnable(2, ref temp))
                {
                }

                return temp;
            }
            set
            {
                SetMCLSLaserEnable(2, value);
            }
        }

        public double Laser3Max
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;

                if (GetMCLSLaserRange(2, ref tempMin, ref tempMax))
                {
                }

                return tempMax;
            }
        }

        public double Laser3Min
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;

                if (GetMCLSLaserRange(2, ref tempMin, ref tempMax))
                {
                }

                return tempMin;
            }
        }

        public double Laser3Power
        {
            get
            {
                double temp = 0;

                if (GetMCLSLaserPower(2, ref temp))
                {
                }

                return temp;
            }
            set
            {
                SetMCLSLaserPower(2, value);
            }
        }

        public int Laser4Enable
        {
            get
            {
                int temp = 0;

                if (GetMCLSLaserEnable(3, ref temp))
                {
                }

                return temp;
            }
            set
            {
                SetMCLSLaserEnable(3, value);
            }
        }

        public double Laser4Max
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;

                if (GetMCLSLaserRange(3, ref tempMin, ref tempMax))
                {
                }

                return tempMax;
            }
        }

        public double Laser4Min
        {
            get
            {
                double tempMin = 0;
                double tempMax = 0;

                if (GetMCLSLaserRange(3, ref tempMin, ref tempMax))
                {
                }

                return tempMin;
            }
        }

        public double Laser4Power
        {
            get
            {
                double temp = 0;

                if (GetMCLSLaserPower(3, ref temp))
                {
                }

                return temp;
            }
            set
            {
                SetMCLSLaserPower(3, value);
            }
        }

        public int LaserShutterPosition
        {
            get
            {
                int val=0;
                GetLaserShutterPosition(ref val);
                return val;
            }
            set
            {
                SetLaserShutterPosition(value);
            }
        }

        public bool LiveStartButtonStatus
        {
            get
            {
                return _liveStartButtonStatus;
            }
            set
            {
                _liveStartButtonStatus = value;
            }
        }

        public bool LiveStopButtonStatus
        {
            get
            {
                return _liveStopButtonStatus;
            }
            set
            {
                _liveStopButtonStatus = value;
            }
        }

        public int LSMAreaMode
        {
            get
            {
                return _liParams.lsmAreaMode.val;
            }
            set
            {
                int temp = _liParams.lsmAreaMode.val;

                _liParams.lsmAreaMode.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmAreaMode.val = temp;
            }
        }

        public int LSMChannel
        {
            get
            {
                if (DigitizerBoardName == DigitizerBoardNames.ATS9440)
                {
                    switch (_liParams.lsmChannel.val)
                    {
                        case 1: return 0;
                        case 2: return 1;
                        case 4: return 2;
                        case 8: return 3;
                        default: return 4;
                    }
                }
                else if (DigitizerBoardName == DigitizerBoardNames.ATS460)
                {
                    switch (_liParams.lsmChannel.val)
                    {
                        case 1: return 0;
                        case 2: return 1;
                        default: return 4;
                    }
                }
                else
                {
                    MessageBox.Show("Error in Model.LiveImage.DigitizerBoardName setter", "Unrecognized Boardname");
                    return -1;
                }
            }
            set
            {
                int temp = _liParams.lsmChannel.val;
                if (DigitizerBoardName == DigitizerBoardNames.ATS9440)
                {
                    //convert index value to bitwise value
                    switch (value)
                    {
                        case 0: _liParams.lsmChannel.val = 1; break;
                        case 1: _liParams.lsmChannel.val = 2; break;
                        case 2: _liParams.lsmChannel.val = 4; break;
                        case 3: _liParams.lsmChannel.val = 8; break;
                        case 4:
                            {
                                _liParams.lsmChannel.val = (Convert.ToInt32(LSMChannelEnable[0]) | (Convert.ToInt32(LSMChannelEnable[1]) << 1) | (Convert.ToInt32(LSMChannelEnable[2]) << 2) | (Convert.ToInt32(LSMChannelEnable[3]) << 3));
                            }
                            break;
                    }
                }
                else if (DigitizerBoardName == DigitizerBoardNames.ATS460)
                {
                    //convert index value to bitwise value
                    switch (value)
                    {
                        case 0: _liParams.lsmChannel.val = 1; break;
                        case 1: _liParams.lsmChannel.val = 2; break;
                        default:
                            {
                                _liParams.lsmChannel.val = (Convert.ToInt32(LSMChannelEnable[0]) | (Convert.ToInt32(LSMChannelEnable[1]) << 1));
                            }
                            break;
                    }
                }
                else
                {
                    MessageBox.Show("Error in Model.LiveImage.DigitizerBoardName setter", "Unrecognized Boardname");
                }

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                    }
                    else
                    {
                        _liParams.lsmChannel.val = temp;
                    }
                }
            }
        }

        public bool[] LSMChannelEnable
        {
            get
            {
                return _lsmChannelEnable;
            }
            set
            {
                _lsmChannelEnable = value;

                //update the channel value also

                if(LSMChannel == 4)
                {
                    _liParams.lsmChannel.val = (Convert.ToInt32(_lsmChannelEnable[0]) | (Convert.ToInt32(_lsmChannelEnable[1]) << 1) | (Convert.ToInt32(_lsmChannelEnable[2]) << 2) | (Convert.ToInt32(_lsmChannelEnable[3]) << 3));

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                    }
                }}
            }
        }

        public int LSMClockSource
        {
            get { return _liParams.lsmClockSource.val; }
            set
            {
                int temp = _liParams.lsmClockSource.val;

                _liParams.lsmClockSource.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmClockSource.val = temp;
            }
        }

        public int LSMExtClockRate
        {
            get { return _liParams.lsmExtClockRate.val; }
            set
            {
                int temp = _liParams.lsmExtClockRate.val;

                _liParams.lsmExtClockRate.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmExtClockRate.val = temp;
            }
        }

        public int LSMFieldOffsetX
        {
            get {

                double xyRatio = (double)LSMPixelY / (double)LSMPixelX;
                return (int)((_liParams.lsmFieldOffsetX.val + 128 - (int)(_liParams.lsmFieldSize.val / (Math.Sqrt(1 + (xyRatio * xyRatio)) * 2.0))));
            }
            set
            {
                double xyRatio = (double)LSMPixelY / (double)LSMPixelX;
                int temp = _liParams.lsmFieldOffsetX.val;

                int testVal = value - 128 + (int)(_liParams.lsmFieldSize.val / (Math.Sqrt(1 + (xyRatio * xyRatio)) * 2.0));

                _liParams.lsmFieldOffsetX.val = testVal;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = testVal;
                    }
                }

                _liParams.lsmFieldOffsetX.val = temp;
            }
        }

        public int LSMFieldOffsetY
        {
            get
            {
                double xyRatio = (double)LSMPixelY / (double)LSMPixelX;
                return (int)((_liParams.lsmFieldOffsetY.val + 128 - (int)(_liParams.lsmFieldSize.val /(Math.Sqrt(1 + 1/(xyRatio*xyRatio))* 2.0))));
            }
            set
            {
                double xyRatio = (double)LSMPixelY / (double)LSMPixelX;
                int temp = _liParams.lsmFieldOffsetY.val;
                int testVal = value - 128 + (int)(_liParams.lsmFieldSize.val / (Math.Sqrt(1 + 1/(xyRatio * xyRatio)) * 2.0));
                _liParams.lsmFieldOffsetY.val = testVal;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = testVal;
                    }
                }

                _liParams.lsmFieldOffsetY.val = temp;
            }
        }

        public int LSMFieldSize
        {
            get { return _liParams.lsmFieldSize.val; }
            set
            {
                int temp = _liParams.lsmFieldSize.val;

                _liParams.lsmFieldSize.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmFieldSize.val = temp;
            }
        }

        public int LSMFieldSizeMax
        {
            get
            {
                int fMin = 0;
                int fMax = 0;
                GetFieldSizeRange(ref fMin, ref fMax);

                return fMax;
            }
        }

        public int LSMFieldSizeMin
        {
            get
            {
                int fMin = 0;
                int fMax = 0;
                GetFieldSizeRange(ref fMin, ref fMax);
                return fMin;
            }
        }

        public int LSMPinholeAlignmentPosition
        {
            get
            {
                int val = 0;

                if (GetPinholeAlignmentPosition(ref val))
                {
                }
                return (int)val;
            }
            set
            {
                SetPinholeAlignmentPosition(value);
            }
        }

        public int LSMPixelX
        {
            get { return _liParams.lsmPixelX.val; }
            set
            {
                int temp = _liParams.lsmPixelX.val;

                _liParams.lsmPixelX.val = value;

                switch ((AreaMode)_liParams.lsmAreaMode.val)
                {
                    case AreaMode.SQUARE:
                        {
                            _liParams.lsmPixelY.val = value;
                        }
                        break;
                    case AreaMode.LINE:
                        {
                            _liParams.lsmPixelY.val = 1;
                        }
                        break;

                }

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmPixelX.val = temp;

                switch ((AreaMode)_liParams.lsmAreaMode.val)
                {
                    case AreaMode.SQUARE:
                    case AreaMode.LINE:
                        {
                            _liParams.lsmPixelY.val = temp;
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

                GetPixelXRange(ref xMin, ref xMax);

                //restrict the pixel density in color mode
                //based on the stored limit value
                if (LSMChannel == 4)
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
                int xMin=0;
                int xMax=0;
                GetPixelXRange(ref xMin, ref xMax);
                return xMin;
            }
        }

        public int LSMPixelY
        {
            get { return _liParams.lsmPixelY.val; }
            set
            {
                int temp = _liParams.lsmPixelY.val;

                _liParams.lsmPixelY.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmPixelY.val = temp;
            }
        }

        public int LSMPixelYMax
        {
            get
            {
                int yMin = 0;
                int yMax = 0;
                GetPixelYRange(ref yMin, ref yMax);

                //restrict the pixel density in color mode
                //based on the stored limit value
                if (LSMChannel == 4)
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
                GetPixelXRange(ref yMin, ref yMax);
                return yMin;
            }
        }

        public int LSMScanMode
        {
            get { return _liParams.lsmScanMode.val; }
            set
            {
                int temp = _liParams.lsmScanMode.val;

                _liParams.lsmScanMode.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmScanMode.val = temp;
            }
        }

        public int LSMSignalAverage
        {
            get { return _liParams.lsmAverageMode.val; }
            set
            {
                int temp = _liParams.lsmAverageMode.val;

                _liParams.lsmAverageMode.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmAverageMode.val = temp;
            }
        }

        public int LSMTwoWayAlignment
        {
            get { return _liParams.lsmTwoWayAlignment.val; }
            set
            {
                int temp = _liParams.lsmTwoWayAlignment.val;

                _liParams.lsmTwoWayAlignment.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmTwoWayAlignment.val = temp;
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

        public int PinholeMax
        {
            get
            {
                int pinholeMin = 0;
                int pinholeMax = 0;

                if (GetPinholeRange(ref pinholeMin, ref pinholeMax))
                {
                }

                return pinholeMax;
            }
        }

        public int PinholeMin
        {
            get
            {
                int pinholeMin = 0;
                int pinholeMax = 0;

                if (GetPinholeRange(ref pinholeMin, ref pinholeMax))
                {
                }

                return pinholeMin;
            }
        }

        public int PinholePosition
        {
            get
            {
                int val = 0;

                if (GetPinholePosition(ref val))
                {
                }
                return (int)val;
            }
            set
            {

                SetPinholePosition(value);
            }
        }

        public int PMT1Gain
        {
            get
            {
                int val = 0;

                if (GetPMT1Position(ref val))
                {
                }
                    return (int)val;
            }
            set
            {

                SetPMT1Position(value);
            }
        }

        public int PMT1GainEnable
        {
            get
            {
                int val = 0;

                if (GetPMT1Enable(ref val))
                {
                }
                return val;
            }
            set
            {

                SetPMT1Enable(value);
            }
        }

        public int PMT1GainMax
        {
            get
            {
                int pmtMin = 0;
                int pmtMax = 0;
                int pmtDefault = 0;

                if (GetPMT1Range(ref pmtMin, ref pmtMax, ref pmtDefault))
                {
                }

                return pmtMax;
            }
        }

        public int PMT1GainMin
        {
            get
            {
                int pmtMin = 0;
                int pmtMax = 0;
                int pmtDefault = 0;

                if (GetPMT1Range(ref pmtMin, ref pmtMax, ref pmtDefault))
                {
                }

                return pmtMin;
            }
        }

        public int PMT2Gain
        {
            get
            {
                int val=0;

                if (GetPMT2Position(ref val))
                {
                }
                    return val;
            }
            set
            {
                SetPMT2Position(value);
            }
        }

        public int PMT2GainEnable
        {
            get
            {
                int val = 0;

                if (GetPMT2Enable(ref val))
                {
                }
                return val;
            }
            set
            {

                SetPMT2Enable(value);
            }
        }

        public int PMT2GainMax
        {
            get
            {
                int pmtMin = 0;
                int pmtMax = 0;
                int pmtDefault = 0;

                if (GetPMT2Range(ref pmtMin, ref pmtMax, ref pmtDefault))
                {
                }

                return pmtMax;
            }
        }

        public int PMT2GainMin
        {
            get
            {
                int pmtMin = 0;
                int pmtMax = 0;
                int pmtDefault = 0;

                if (GetPMT2Range(ref pmtMin, ref pmtMax, ref pmtDefault))
                {
                }

                return pmtMin;
            }
        }

        public int PMT3Gain
        {
            get
            {
                int val = 0;

                if (GetPMT3Position(ref val))
                {
                }
                return (int)val;
            }
            set
            {

                SetPMT3Position(value);
            }
        }

        public int PMT3GainEnable
        {
            get
            {
                int val = 0;

                if (GetPMT3Enable(ref val))
                {
                }
                return val;
            }
            set
            {

                SetPMT3Enable(value);
            }
        }

        public int PMT3GainMax
        {
            get
            {
                int pmtMin = 0;
                int pmtMax = 0;
                int pmtDefault = 0;

                if (GetPMT3Range(ref pmtMin, ref pmtMax, ref pmtDefault))
                {
                }

                return pmtMax;
            }
        }

        public int PMT3GainMin
        {
            get
            {
                int pmtMin = 0;
                int pmtMax = 0;
                int pmtDefault = 0;

                if (GetPMT3Range(ref pmtMin, ref pmtMax, ref pmtDefault))
                {
                }

                return pmtMin;
            }
        }

        public int PMT4Gain
        {
            get
            {
                int val = 0;

                if (GetPMT4Position(ref val))
                {
                }
                return (int)val;
            }
            set
            {

                SetPMT4Position(value);
            }
        }

        public int PMT4GainEnable
        {
            get
            {
                int val = 0;

                if (GetPMT4Enable(ref val))
                {
                }
                return val;
            }
            set
            {

                SetPMT4Enable(value);
            }
        }

        public int PMT4GainMax
        {
            get
            {
                int pmtMin = 0;
                int pmtMax = 0;
                int pmtDefault = 0;

                if (GetPMT4Range(ref pmtMin, ref pmtMax, ref pmtDefault))
                {
                }

                return pmtMax;
            }
        }

        public int PMT4GainMin
        {
            get
            {
                int pmtMin = 0;
                int pmtMax = 0;
                int pmtDefault = 0;

                if (GetPMT4Range(ref pmtMin, ref pmtMax, ref pmtDefault))
                {
                }

                return pmtMin;
            }
        }

        public bool PMTSafetyStatus
        {
            get
            {
                return GetPMTSafetyStatus();
            }
        }

        public int PowerMax
        {
            get
            {
                int pMax = 0;
                int pMinPos = 0;
                int pMaxPos = 0;
                int pDefaultPos = 0;

                if (GetPowerRange(ref pMinPos, ref pMaxPos, ref pDefaultPos))
                {
                    pMax = pMaxPos;

                }

                return pMax;
            }
        }

        public int PowerMin
        {
            get
            {
                int pMin = 0;
                int pMinPos = 0;
                int pMaxPos = 0;
                int pDefaultPos = 0;

                if (GetPowerRange(ref pMinPos, ref pMaxPos, ref pDefaultPos))
                {
                    pMin = pMinPos;

                }

                return pMin;
            }
        }

        public int PowerPockelsBlankPercentage
        {
            get
            {
                int pos = 0;

                GetCameraParameterValueInt((int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0, ref pos);

                return pos;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0,(double)value);
            }
        }

        public int PowerPosition
        {
            get
            {
                int pos = 0;

                GetPowerPosition(ref pos);

                return pos;
            }
            set
            {
                SetPowerPosition(value);

            }
        }

        public int RollOverPointIntensity0
        {
            get
            {
                if (_pixelData != null)
                {
                    int width = 0;
                    int height = 0;

                    if (false == GetImageDimensions(ref width, ref height))
                    {
                        return 0;
                    }
                    _dataWidth = width;
                    _dataHeight = height;

                    //if the requested pixel is within the buffer size
                    int location = (int)(_rollOverPointX  +  (width) * _rollOverPointY);

                    if ((_rollOverPointX < _dataWidth) && (_rollOverPointY < _dataHeight) && (location >= 0) && (location < _pixelData.Length))
                    {
                        int val;
                        if (_pixelData[location] < 0)
                        {
                            val = (int)(_pixelData[location] + 32768.0);
                        }
                        else
                        {
                            val = (_pixelData[location]);
                        }

                        return val;
                    }
                }
                return 0;
            }
        }

        public int RollOverPointIntensity1
        {
            get
            {
                int width = 0;
                int height = 0;

                if (false == GetImageDimensions(ref width, ref height))
                {
                    return 0;
                }
                _dataWidth = width;
                _dataHeight = height;
                _dataLength = width * height;

                if (_pixelData != null && _dataHeight != 0 && _colorChannels >= 2)
                {
                    //if the requested pixel is within the buffer size
                    int location = (int)(_rollOverPointX + ((_dataWidth)) * _rollOverPointY);

                    if ((_rollOverPointX < _dataWidth) && (_rollOverPointY < _dataHeight) && (location >= 0) && ((location + +_dataLength) < _pixelData.Length))
                    {
                        int val;
                        if (_pixelData[location] < 0)
                        {
                            val = (int)(_pixelData[location +  _dataLength] + 32768.0);
                        }
                        else
                        {
                            val = (_pixelData[location +  _dataLength]);
                        }

                        return val;
                    }
                }
                return 0;
            }
        }

        public int RollOverPointIntensity2
        {
            get
            {
                int width = 0;
                int height = 0;

                if (false == GetImageDimensions(ref width, ref height))
                {
                    return 0;
                }
                _dataWidth = width;
                _dataHeight = height;
                _dataLength = width * height;

                if (_pixelData != null && _dataHeight != 0 && _colorChannels >= 3)
                {
                    //if the requested pixel is within the buffer size
                    int location = (int)(_rollOverPointX + ((_dataWidth)) * _rollOverPointY);

                    if ((_rollOverPointX < _dataWidth) && (_rollOverPointY < _dataHeight) && (location >= 0) && ((location + 2 * _dataLength) < _pixelData.Length))
                    {
                        int val;
                        if (_pixelData[location] < 0)
                        {
                            val = (int)(_pixelData[location + 2 * _dataLength] + 32768.0);
                        }
                        else
                        {
                            val = (_pixelData[location + 2 * _dataLength]);
                        }

                        return val;
                    }
                }
                return 0;
            }
        }

        public int RollOverPointIntensity3
        {
            get
            {
                int width = 0;
                int height = 0;

                if (false == GetImageDimensions(ref width, ref height))
                {
                    return 0;
                }
                _dataWidth = width;
                _dataHeight = height;
                _dataLength = width * height;

                if (_pixelData != null && _dataHeight != 0 && _colorChannels >= 4)
                {
                    //if the requested pixel is within the buffer size
                    int location = (int)(_rollOverPointX + ((_dataWidth)) * _rollOverPointY);

                    if ((_rollOverPointX < _dataWidth) && (_rollOverPointY < _dataHeight) && (location >= 0) && ((location + 3 * _dataLength) < _pixelData.Length))
                    {
                        int val;
                        if (_pixelData[location] < 0)
                        {
                            val = (int)(_pixelData[location + 3 * _dataLength] + 32768.0);
                        }
                        else
                        {
                            val = (_pixelData[location + 3 * _dataLength]);
                        }

                        return val;
                    }
                }
                return 0;
            }
        }

        public int RollOverPointX
        {
            get
            {
                return _rollOverPointX;
            }
            set
            {
                _rollOverPointX = value;
            }
        }

        public int RollOverPointY
        {
            get
            {
                return _rollOverPointY;
            }
            set
            {
                _rollOverPointY = value;
            }
        }

        public int ShutterPosition
        {
            get
            {
                return _shutterPosition;
            }
            set
            {
                if (SetShutterPosition(value))
                {
                    _shutterPosition = value;
                }
            }
        }

        public int SignalAverageFrames
        {
            get { return _liParams.lsmAverageNum.val; }
            set
            {
                int temp = _liParams.lsmAverageNum.val;

                _liParams.lsmAverageNum.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lsmAverageNum.val = temp;
            }
        }

        public int TurretPosition
        {
            get
            {
                return _turretPosition;
            }
            set
            {
                ////escape the objective first. then return the focus
                //double zMinPos = 0;
                //double zMaxPos = 0;
                //double zDefaultPos = 0;

                //if (false == GetZRange(ref zMinPos, ref zMaxPos, ref zDefaultPos))
                //{
                //    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " TurretPosition unable to get z range");
                //    return;
                //}
                //double temp = 0;

                //if (false == GetZPosition(ref temp))
                //{
                //    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " TurretPosition unable to get z position");
                //    return;
                //}

                //double retractPosition;
                //double range = Math.Abs(zMaxPos - zMinPos);

                //if (zMaxPos > 0)
                //{
                //    retractPosition = zMaxPos - (.2 * range);
                //}
                //else
                //{
                //    retractPosition = zMaxPos + (.2 * range);
                //}

                ////move within 20% of the top of travel
                //if (false == SetZPosition(retractPosition))
                //{
                //    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " TurretPosition unable to set z position");
                //    return;
                //}

                if (true == SetTurretPosition(value+1))
                {
                    _turretPosition = value;
                }
                else
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " TurretPosition SetTurretPosition failed");
                }

                ////return to z location
                //if (false == SetZPosition(temp))
                //{
                //    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " TurretPosition return to z position failed");
                //}
            }
        }

        public Color WavelengthColor
        {
            get
            {
                return _wavelengthColor;
            }
            set
            {
                _wavelengthColor = value;
            }
        }

        public double WhitePoint0
        {
            get
            {
                if (_whitePoint.Length > 0)
                {
                    return _whitePoint[0];
                }
                else
                {
                    return 255;
                }
            }
            set
            {
                if (_whitePoint.Length > 0)
                {
                    _whitePoint[0] = value;
                }
                _paletteChanged = true;
            }
        }

        public double WhitePoint1
        {
            get
            {
                if (_whitePoint.Length > 1)
                {
                    return _whitePoint[1];
                }
                else
                {
                    return 255;
                }
            }
            set
            {
                if (_whitePoint.Length > 1)
                {
                    _whitePoint[1] = value;
                }
                _paletteChanged = true;
            }
        }

        public double WhitePoint2
        {
            get
            {
                if (_whitePoint.Length > 2)
                {
                    return _whitePoint[2];
                }
                else
                {
                    return 255;
                }
            }
            set
            {
                if (_whitePoint.Length > 2)
                {
                    _whitePoint[2] = value;
                }
                _paletteChanged = true;
            }
        }

        public double WhitePoint3
        {
            get
            {
                if (_whitePoint.Length > 3)
                {
                    return _whitePoint[3];
                }
                else
                {
                    return 255;
                }
            }
            set
            {
                if (_whitePoint.Length > 3)
                {
                    _whitePoint[3] = value;
                }
                _paletteChanged = true;
            }
        }

        public double XMax
        {
            get
            {
                double xMax=0;
                double xMinPos = 0;
                double xMaxPos = 0;
                double xDefaultPos = 0;

                if (GetXRange(ref xMinPos, ref xMaxPos, ref xDefaultPos))
                {
                    xMax = xMaxPos;

                }

                return xMax;
            }
        }

        public double XMin
        {
            get
            {
                double xMin=0;
                double xMinPos = 0;
                double xMaxPos = 0;
                double xDefaultPos = 0;

                if (GetXRange(ref xMinPos, ref xMaxPos, ref xDefaultPos))
                {
                    xMin = xMinPos;

                }

                return xMin;
            }
        }

        public double XPosition
        {
            get
            {
                double pos = 0.0;

                if (GetXPosition(ref pos))
                {
                    Decimal dec = new Decimal(pos);

                    pos = Decimal.ToDouble(Decimal.Round(dec, 4));
                }

                return pos;
            }
            set
            {
                double temp = value;

                if (SetXPosition(temp))
                {
                    temp = value;
                }
            }
        }

        public double YMax
        {
            get
            {
                double yMax = 0;
                double yMinPos = 0;
                double yMaxPos = 0;
                double yDefaultPos = 0;

                if (GetYRange(ref yMinPos, ref yMaxPos, ref yDefaultPos))
                {
                    yMax = yMaxPos;

                }

                return yMax;
            }
        }

        public double YMin
        {
            get
            {
                double yMin = 0;
                double yMinPos = 0;
                double yMaxPos = 0;
                double yDefaultPos = 0;

                if (GetYRange(ref yMinPos, ref yMaxPos, ref yDefaultPos))
                {
                    yMin = yMinPos;

                }

                return yMin;
            }
        }

        public double YPosition
        {
            get
            {
                double pos=0.0;

                if (GetYPosition(ref pos))
                {
                    Decimal dec = new Decimal(pos);

                    pos = Decimal.ToDouble(Decimal.Round(dec, 4));
                }

                return pos;
            }
            set
            {
                double temp = value;

                if (SetYPosition(temp))
                {
                    temp = value;
                }
            }
        }

        public double ZMax
        {
            get
            {
                double zMax=0;
                double zMinPos = 0;
                double zMaxPos = 0;
                double zDefaultPos = 0;

                if (GetZRange(ref zMinPos, ref zMaxPos, ref zDefaultPos))
                {
                    zMax = zMaxPos;

                }

                return zMax;
            }
        }

        public double ZMin
        {
            get
            {
                double zMin = 0;
                double zMinPos = 0;
                double zMaxPos = 0;
                double zDefaultPos = 0;

                if (GetZRange(ref zMinPos, ref zMaxPos, ref zDefaultPos))
                {
                    zMin = zMinPos;

                }

                return zMin;
            }
        }

        public double ZPosition
        {
            get
            {
                double pos = 0.0;

                if (GetZPosition(ref pos))
                {
                    Decimal dec = new Decimal(pos);

                    pos = Decimal.ToDouble(Decimal.Round(dec, 4));
                }

                return pos;
            }
            set
            {
                double temp = value;

                if(SetZPositionNoWait(temp))
                    {
                        temp = value;
                    }
            }
        }

        #endregion Properties

        #region Methods

        public static void FinishedCopyingPixel()
        {
            {
                _pixelDataReady = false;
            }
        }

        public static int GetColorAssignment(int index)
        {
            return _colorAssigment[index];
        }

        public static int GetColorChannels()
        {
            return _colorChannels;
        }

        public static short[] GetPixelData()
        {
            return _pixelData;
        }

        public static byte[] GetPixelDataByte()
        {
            if (_pixelData == null)
                return null;

            int shiftValue = 4;
            double shiftValueResult = 64;

            switch (_camType)
            {
                case CameraType.LSM:
                    {
                        shiftValue = 6;
                        shiftValueResult = Math.Pow(2, shiftValue);
                    }
                    break;

                case CameraType.CCD:
                case CameraType.CCD_MOSAIC:
                    {
                        //shiftValue = 4;
                        //shiftValueResult = Math.Pow(2, shiftValue);
                        int bitsPerPixel = 0;
                        GetCameraParameterValueInt((int)ICamera.Params.PARAM_BITS_PER_PIXEL, ref bitsPerPixel);
                        shiftValue = bitsPerPixel - 8;
                        shiftValueResult = Math.Pow(2, shiftValue);
                    }
                    break;

                default:
                    {
                        shiftValue = 6;
                        shiftValueResult = Math.Pow(2, shiftValue);
                    }
                    break;
            }

            if (null == _pixelDataLUT)
            {
                _pixelDataLUT = new byte[ushort.MaxValue + 1];
            }

            //Build the 12/14-bit to 8-bit Lut
            for (int i = 0; i < _pixelDataLUT.Length; i++)
            {
                double val = (255.0 / (shiftValueResult * (_whitePoint[0] - _blackPoint[0]))) * (i - _blackPoint[0] * shiftValueResult);
                val = (val >= 0) ? val : 0;
                val = (val <= 255) ? val : 255;

                _pixelDataLUT[i] = (byte)Math.Round(val);
            }

            for (int i = 0; i < _dataLength * _colorChannels; i++)
            {
                byte val;

                if (_pixelData[i] < 0)
                {
                    val = _pixelDataLUT[(_pixelData[i] + 32768)];
                }
                else
                {
                    val = _pixelDataLUT[(_pixelData[i])];
                }

                _pixelDataByte[i] = val;
            }

            return _pixelDataByte;
        }

        public static byte[] GetPixelDataByteEx(bool doColor, int channelIndex)
        {
            //need to rebuid the color image because a palette option is not available for RGB images
            if ((_colorChannels > 1) && (_pixelData != null) && (_dataLength * _colorChannels == _pixelData.Length))
            {
                int i;

                if (doColor)
                {
                    //clear the histogram
                    for (i = 0; i < PIXEL_DATA_HISTOGRAM_SIZE; i++)
                    {
                        for (int k = 0; k < _colorChannels; k++)
                        {
                            _pixelDataHistogram[k][i] = 0;
                        }
                    }
                }

                if (_paletteChanged == true)
                {
                    BuildRGBColorPalettes();
                }

                //calculate the raw data buffer offset index for each of the
                //selected display channels
                int[] dataBufferOffsetIndex = new int[MAX_CHANNELS];

                int j;
                int enabledChannelCount = 0;
                for (i = 0, enabledChannelCount = 0; i < MAX_CHANNELS; i++)
                {
                    //if the channgel is enabled store the index and
                    //increment the enabled counter index j

                    if (true == doColor)
                    {
                        if (_lsmChannelEnable[i])
                        {
                            dataBufferOffsetIndex[enabledChannelCount] = i;
                            enabledChannelCount++;
                        }
                    }
                    else
                    {
                        if (_lsmChannelEnable[i] && (i == channelIndex))
                        {
                            dataBufferOffsetIndex[enabledChannelCount] = i;
                            enabledChannelCount++;
                        }

                    }
                }

                //factor is derived by taking CAMERA_MAX_INTENSITY_VALUE/256
                //const double FULL_RANGE_NORMALIZATION_FACTOR = 64.0;
                for (i = 0, j = 0; j < _dataLength; i += 3, j++)
                {
                    byte valRaw;
                    const int SHIFT_VALUE = 6;

                    byte maxRed = 0;
                    byte maxGreen = 0;
                    byte maxBlue = 0;

                    _pixelDataByte[i] = 0;
                    _pixelDataByte[i + 1] = 0;
                    _pixelDataByte[i + 2] = 0;

                    for (int k = 0; k < enabledChannelCount; k++)
                    {
                        valRaw = (byte)((_pixelData[j + dataBufferOffsetIndex[k] * _dataLength]) >> SHIFT_VALUE);

                        byte newRaw = (byte)(valRaw >> 1);

                        switch ((ColorAssignments)_colorAssigment[dataBufferOffsetIndex[k]])
                        {
                            case ColorAssignments.RED:
                                {
                                    _pixelDataByte[i] = maxRed = Math.Max(maxRed, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                }
                                break;
                            case ColorAssignments.GREEN:
                                {
                                    _pixelDataByte[i + 1] = maxGreen = Math.Max(maxGreen, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                }
                                break;
                            case ColorAssignments.BLUE:
                                {
                                    _pixelDataByte[i + 2] = maxBlue = Math.Max(maxBlue, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                }
                                break;
                            case ColorAssignments.CYAN:
                                {
                                    _pixelDataByte[i + 1] = maxGreen = Math.Max(maxGreen, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                    _pixelDataByte[i + 2] = maxBlue = Math.Max(maxBlue, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                }
                                break;
                            case ColorAssignments.MAGENTA:
                                {
                                    _pixelDataByte[i] = maxRed = Math.Max(maxRed, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                    _pixelDataByte[i + 2] = maxBlue = Math.Max(maxBlue, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                }
                                break;
                            case ColorAssignments.YELLOW:
                                {
                                    _pixelDataByte[i] = maxRed = Math.Max(maxRed, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                    _pixelDataByte[i + 1] = maxGreen = Math.Max(maxGreen, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                }
                                break;
                            case ColorAssignments.GRAY:
                                {
                                    _pixelDataByte[i] = maxRed = Math.Max(maxRed, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                    _pixelDataByte[i + 1] = maxGreen = Math.Max(maxGreen, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                    _pixelDataByte[i + 2] = maxBlue = Math.Max(maxBlue, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                }
                                break;
                        }

                        //only build the histogram if the color mode is selected.
                        //This will allow histograms for all of the channels to be available simultaneously
                        if (doColor)
                        {
                            _pixelDataHistogram[dataBufferOffsetIndex[k]][valRaw]++;
                        }
                    }
                }
            }

            return _pixelDataByte;
        }

        public static bool IsPixelDataReady()
        {
            {
                EnableCopyToExternalBuffer();

                return _pixelDataReady;
            }
        }

        public static void SetColorAssignment(int index,int value)
        {
            _colorAssigment[index] = value;
        }

        public static void SetColorChannels(int channels)
        {
            _colorChannels = channels;
        }

        public bool AutoExposure(double exposure)
        {
            if (SetCustomParamsBinary(ref _liParams) == 1)
            {
                double exposureResult=0;
                double multiplier=0;
                if (false == AutoExposure(exposure,ref exposureResult, ref multiplier))
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " AutoExposure failed");
                    return false;
                }

                _liParams.exposureTimeCam0.val = exposureResult;

                SetCustomParamsBinary(ref _liParams);
            }

            return true;
        }

        public bool AutoFocus(double magnification)
        {
            if (false == StartAutoFocus(magnification))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " StartAutoFocus failed");
                return false;
            }

            return true;
        }

        public int NumberAvailableCameras()
        {
            int numCams = 0;

            if (GetNumberOfCameras(ref numCams))
            {
                return numCams;
            }
            else
            {
                return 0;
            }
        }

        public bool SetZZero()
        {
            return SetZStageZero();
        }

        public void Snapshot()
        {
            //ensure the buffer is copied after the capture
            _pixelDataReady = false;

            if (SetCustomParamsBinary(ref _liParams) == 1)
            {
                //Execute will set all of the capture parameters
                if (LiveImageDataExecute() == 1)
                {
                    DisablePMTGains();
                    EnablePMTGains();
                    LiveSnapshot();
                    DisablePMTGains();
                }
            }
        }

        public void SnapshotColor(int redEx, int redDic, int redEm, double redExp, int greenEx, int greenDic, int greenEm, double greenExp, int blueEx, int blueDic, int blueEm, double blueExp, int useGray, double grayExp)
        {
            //ensure the buffer is copied after the capture
            _pixelDataReady = false;

            if (SetCustomParamsBinary(ref _liParams) == 1)
            {
                //Execute will set all of the capture parameters
                if (LiveImageDataExecute() == 1)
                {
                    LiveSnapshotColor(redEx, redDic, redEm, redExp, greenEx, greenDic, greenEm, greenExp, blueEx, blueDic, blueEm, blueExp, useGray, grayExp);
                }
            }
        }

        public void Start()
        {
            try
            {
                _liveStartButtonStatus = false;
                _liveStopButtonStatus = true;
                // event trigerred to the view model to change the status of the menu bar buttons
                UpdateMenuBarButton(false);

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    //Execute will set all of the capture parameters
                    if (LiveImageDataExecute() == 1)
                    {
                        DisablePMTGains();
                        EnablePMTGains();

                        StartLiveCapture();
                    }
                }
            }
            catch (System.DllNotFoundException)
            {
                //CaptureSetupDll is missing
            }
        }

        public void Stop()
        {
            try
            {
                _liveStartButtonStatus = true;
                _liveStopButtonStatus = false;
                // event trigerred to the view model to change the status of the menu bar buttons
                UpdateMenuBarButton(true);
                DisablePMTGains();
                StopLiveCapture();
                _prevTickCount = 0;
                _framesPerSecond = 0;
            }
            catch (System.DllNotFoundException)
            {
                //CaptureSetupDll is missing
            }
        }

        public bool StopZ()
        {
            return StopZStage();
        }

        public bool VerifyDecoder(int id)
        {
           // Decoder is not used anymore it was removed from the device types
            // long devId = GetDeviceID((int) DeviceType.DECODER);
            long devId = 0;

            if (devId == id)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "AutoExposure")]
        private static extern bool AutoExposure(double exposure, ref double exposureResult, ref double multiplier);

        private static void BuildRGBColorPalettes()
        {
            for (int j = 0; j < _colorChannels; j++)
            {
                for (int i = 0; i < 256; i++)
                {
                    double a = (255.0) / (_whitePoint[j] - _blackPoint[j]);
                    double b = 0 - (a * _blackPoint[j]);

                    double dval = (a * i) + b;
                    dval = Math.Max(dval, 0);
                    dval = Math.Min(dval, 255);

                    _pal[j][i] = (byte)dval;
                }
            }
        }

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "EnableCopyToExternalBuffer")]
        private static extern bool EnableCopyToExternalBuffer();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraParameterValueInt")]
        private static extern bool GetCameraParameterValueInt(int param, ref int value);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraType")]
        private static extern bool GetCameraType(ref int type);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCommandGUID")]
        private static extern int GetCommandGUID(ref Guid guid);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCustomParamsBinary")]
        private static extern int GetCustomParamsBinary(ref LiveImageDataCustomParams lidParams);

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "GetDeviceID")]
        private static extern int GetDeviceID(int deviceType);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetFieldSizeRange")]
        private static extern bool GetFieldSizeRange(ref int fieldSizeMin, ref int fieldSizeMax);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetFrameRate")]
        private static extern bool GetFrameRate(ref double rate);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetImageDimensions")]
        private static extern bool GetImageDimensions(ref int width, ref int height);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetLaserPosition")]
        private static extern bool GetLaserPosition(int id, ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetLaserShutterPosition")]
        private static extern bool GetLaserShutterPosition(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetMCLSLaserEnable")]
        private static extern bool GetMCLSLaserEnable(int id, ref int enable);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetMCLSLaserPower")]
        private static extern bool GetMCLSLaserPower(int id, ref double power);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetMCLSLaserRange")]
        private static extern bool GetMCLSLaserRange(int id, ref double laserMin, ref double laserMax);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetNumberOfCameras")]
        private static extern bool GetNumberOfCameras(ref int numCameras);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPinholeAlignmentPosition")]
        private static extern bool GetPinholeAlignmentPosition(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPinholePosition")]
        private static extern bool GetPinholePosition(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPinholeRange")]
        private static extern bool GetPinholeRange(ref int pinholeMin, ref int pinholeMax);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPixelXRange")]
        private static extern bool GetPixelXRange(ref int pixelXMin, ref int pixelXMax);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPixelYRange")]
        private static extern bool GetPixelYRange(ref int pixelYMin, ref int pixelYMax);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT1Enable")]
        private static extern bool GetPMT1Enable(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT1Position")]
        private static extern bool GetPMT1Position(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT1Range")]
        private static extern bool GetPMT1Range(ref int pmtMin, ref int pmtMax, ref int pmtDefault);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT2Enable")]
        private static extern bool GetPMT2Enable(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT2Position")]
        private static extern bool GetPMT2Position(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT2Range")]
        private static extern bool GetPMT2Range(ref int pmtMin, ref int pmtMax, ref int pmtDefault);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT3Enable")]
        private static extern bool GetPMT3Enable(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT3Position")]
        private static extern bool GetPMT3Position(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT3Range")]
        private static extern bool GetPMT3Range(ref int pmtMin, ref int pmtMax, ref int pmtDefault);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT4Enable")]
        private static extern bool GetPMT4Enable(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT4Position")]
        private static extern bool GetPMT4Position(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMT4Range")]
        private static extern bool GetPMT4Range(ref int pmtMin, ref int pmtMax, ref int pmtDefault);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPMTSafetyStatus")]
        private static extern bool GetPMTSafetyStatus();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPowerPosition")]
        private static extern bool GetPowerPosition(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetPowerRange")]
        private static extern bool GetPowerRange(ref int pMin, ref int pMax, ref int pDefault);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetTurretPosition")]
        private static extern bool GetTurretPosition(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetXPosition")]
        private static extern bool GetXPosition(ref double pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetXRange")]
        private static extern bool GetXRange(ref double XMin, ref double XMax, ref double XDefault);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetYPosition")]
        private static extern bool GetYPosition(ref double pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetYRange")]
        private static extern bool GetYRange(ref double YMin, ref double YMax, ref double YDefault);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetZPosition")]
        private static extern bool GetZPosition(ref double pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetZRange")]
        private static extern bool GetZRange(ref double _zMin, ref double zMax, ref double zDefault);

        private static void ImageUpdate(IntPtr returnArray, ref int channels)
        {
            //keep a running sum of the tick count for a frames per sec calculation
            if(_prevTickCount == 0)
            {
                _sumTickCount = 0;
            }
            else
            {
                _sumTickCount = _sumTickCount*.75 + (Environment.TickCount - _prevTickCount)*.25;
            }

            //only copy to the buffer when pixel data is not being read
            if (_pixelDataReady == false)
            {
                int width = 0;
                int height = 0;

                if (false == GetImageDimensions(ref width, ref height))
                {
                    return;
                }

                _dataLength = width * height;

                _colorChannels = channels;

                if ((_pixelData == null) || (_pixelData.Length != (_dataLength * _colorChannels)))
                {
                    _pixelData = new short[_dataLength * _colorChannels];
                    _pixelDataHistogram = new int[_colorChannels][];

                    if (_colorChannels == 1)
                    {
                        _pixelDataByte = new byte[_dataLength];
                    }
                    else
                    {
                        _pixelDataByte = new byte[_dataLength * 3];
                    }

                    for (int i = 0; i < _colorChannels; i++)
                    {
                        _pixelDataHistogram[i] = new int[PIXEL_DATA_HISTOGRAM_SIZE];
                    }
                }

                int shiftValue = 4;

                switch(_camType)
                {
                    case CameraType.LSM:
                        {
                            shiftValue = 6;
                        }
                        break;

                    case CameraType.CCD:
                    case CameraType.CCD_MOSAIC:
                        {
                            //shiftValue = 4;
                            int bitsPerPixel = 0;
                            GetCameraParameterValueInt((int)ICamera.Params.PARAM_BITS_PER_PIXEL, ref bitsPerPixel);
                            shiftValue = bitsPerPixel - 8;
                        }
                        break;

                    default:
                        {
                            shiftValue = 6;
                        }
                        break;
                }

                switch (_colorChannels)
                {
                    case 1:
                        {
                            Marshal.Copy(returnArray, _pixelData, 0, _dataLength * _colorChannels);

                            //clear the histogram
                            for (int i = 0; i < PIXEL_DATA_HISTOGRAM_SIZE; i++)
                            {
                                _pixelDataHistogram[0][i] = 0;
                            }

                            for (int i = 0; i < _dataLength * _colorChannels; i++)
                            {                                
                                double valHist;

                                //use the non normalized values for the histogram creation
                                if (_pixelData[i] < 0)
                                {
                                    valHist = (_pixelData[i] + 32768) >> shiftValue;
                                }
                                else
                                {
                                    valHist = (_pixelData[i]) >> shiftValue;
                                }

                                _pixelDataHistogram[0][(byte)valHist]++;
                            }
                        }
                        break;
                    default:
                        {
                            Marshal.Copy(returnArray, _pixelData, 0, _dataLength * _colorChannels);
                        }
                        break;

                }

                _dataWidth = width;
                _dataHeight = height;
                _pixelDataReady = true;
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ImageUpdate pixeldata updated");
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ImageUpdate pixeldata not ready");
            }

            _prevTickCount = Environment.TickCount;
        }

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "InitCallBack")]
        private static extern void InitCallBack(ReportNewImage reportNewImage);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "Execute")]
        private static extern int LiveImageDataExecute();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetupCommand")]
        private static extern int LiveImageDataSetupCommand();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "Snapshot")]
        private static extern bool LiveSnapshot();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SnapshotColor")]
        private static extern bool LiveSnapshotColor(int redEx, int redDic, int redEm, double redExp, int greenEx, int greenDic, int greenEm, double greenExp, int blueEx, int blueDic, int blueEm, double blueExp, int useGray, double grayExp);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetBackgroundSubtractionEnable")]
        private static extern bool SetBackgroundSubtractionEnable(int enable);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetBackgroundSubtractionFile")]
        private static extern bool SetBackgroundSubtractionFile([MarshalAs(UnmanagedType.LPWStr)]string path);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetBFLampPosition")]
        private static extern bool SetBFLampPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetCameraParameterValueDouble")]
        private static extern bool SetCameraParameterValueDouble(int param, double value);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetCustomParamsBinary")]
        private static extern int SetCustomParamsBinary(ref LiveImageDataCustomParams lidParams);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetFilterPositionDic")]
        private static extern bool SetFilterPositionDic(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetFilterPositionEm")]
        private static extern bool SetFilterPositionEm(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetFilterPositionEx")]
        private static extern bool SetFilterPositionEx(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetFlatFieldEnable")]
        private static extern bool SetFlatFieldEnable(int enable);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetFlatFieldFile")]
        private static extern bool SetFlatFieldFile([MarshalAs(UnmanagedType.LPWStr)]string path);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetImageCorrectionEnable")]
        private static extern bool SetImageCorrectionEnable(int enable);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetLaserPosition")]
        private static extern bool SetLaserPosition(int id, int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetLaserShutterPosition")]
        private static extern bool SetLaserShutterPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetMCLSLaserEnable")]
        private static extern bool SetMCLSLaserEnable(int id, int enable);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetMCLSLaserPower")]
        private static extern bool SetMCLSLaserPower(int id, double power);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPincushionCoefficients")]
        private static extern bool SetPincushionCoefficients(double k1, double k2,double k3, double k4);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPinholeAlignmentPosition")]
        private static extern bool SetPinholeAlignmentPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPinholePosition")]
        private static extern bool SetPinholePosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPMT1Enable")]
        private static extern bool SetPMT1Enable(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPMT1Position")]
        private static extern bool SetPMT1Position(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPMT2Enable")]
        private static extern bool SetPMT2Enable(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPMT2Position")]
        private static extern bool SetPMT2Position(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPMT3Enable")]
        private static extern bool SetPMT3Enable(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPMT3Position")]
        private static extern bool SetPMT3Position(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPMT4Enable")]
        private static extern bool SetPMT4Enable(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPMT4Position")]
        private static extern bool SetPMT4Position(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPowerPosition")]
        private static extern bool SetPowerPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetShutterPosition")]
        private static extern bool SetShutterPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetTurretPosition")]
        private static extern bool SetTurretPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetXPosition")]
        private static extern bool SetXPosition(double pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetYPosition")]
        private static extern bool SetYPosition(double pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetZPosition")]
        private static extern bool SetZPosition(double pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetZPositionNoWait")]
        private static extern bool SetZPositionNoWait(double pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetZStageZero")]
        private static extern bool SetZStageZero();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "StartAutoFocus")]
        private static extern bool StartAutoFocus(double magnification);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "StartLiveCapture")]
        private static extern bool StartLiveCapture();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "StopLiveCapture")]
        private static extern bool StopLiveCapture();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "StopZStage")]
        private static extern bool StopZStage();

        private void DisablePMTGains()
        {
            PMT1GainEnable = 0;
            PMT2GainEnable = 0;
            PMT3GainEnable = 0;
            PMT4GainEnable = 0;
        }

        private void EnablePMTGains()
        {
            PMT1GainEnable = 1;
            PMT2GainEnable = 1;
            PMT3GainEnable = 1;
            PMT4GainEnable = 1;
        }

        void Trigger_UpdateImage(bool obj)
        {
            UpdateImage(true);
        }

        #endregion Methods

        #region Nested Types

        [StructLayout(LayoutKind.Sequential)]
        struct DoubleParam
        {
            public double val;
            public int alias;
            public int useAlias;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct IntParam
        {
            public int val;
            public int alias;
            public int useAlias;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct LiveImageDataCustomParams
        {
            public IntParam left;
            public IntParam right;
            public IntParam top;
            public IntParam bottom;
            public IntParam binX;
            public IntParam binY;
            public DoubleParam exposureTimeCam0;
            public DoubleParam exposureTimeCam1;
            public DoubleParam exposureTimeCam2;
            public IntParam lightMode;
            public IntParam gain;
            public IntParam lsmScanMode;
            public IntParam lsmPixelX;
            public IntParam lsmPixelY;
            public IntParam lsmFieldSize;
            public IntParam lsmChannel;
            public IntParam lsmAverageMode;
            public IntParam lsmAverageNum;
            public IntParam lsmInputRangeChannel1;
            public IntParam lsmInputRangeChannel2;
            public IntParam lsmInputRangeChannel3;
            public IntParam lsmInputRangeChannel4;
            public IntParam lsmFieldOffsetX;
            public IntParam lsmFieldOffsetY;
            public IntParam lsmTwoWayAlignment;
            public IntParam lsmClockSource;
            public IntParam lsmExtClockRate;
            public IntParam lsmAreaMode;
        }

        #endregion Nested Types
    }
}