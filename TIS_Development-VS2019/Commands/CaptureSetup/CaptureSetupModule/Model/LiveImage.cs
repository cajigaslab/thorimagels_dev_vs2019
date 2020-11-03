namespace CaptureSetupDll.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
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

    public class LiveImage : INotifyPropertyChanged
    {
        #region Fields

        public const int MAX_CHANNELS = 4;

        private const int PIXEL_DATA_HISTOGRAM_SIZE = 256;

        private static double[] _blackPoint;
        private static ReportBleachNowFinished _BleachNowFinishedCallBack;
        private static int[] _colorAssigment;
        private static int _colorChannels;
        private static int _dataHeight;
        private static int _dataLength;
        private static int _dataWidth;
        private static double _framesPerSecond = 0;
        private static ReportNewImage _imageCallBack;
        private static LiveImageDataCustomParams _liParams = new LiveImageDataCustomParams();
        private static bool[] _lsmChannelEnable;

        //       private static int _nAcquireChannels = 4;
        private static byte[][] _pal;
        private static bool _paletteChanged;
        private static short[] _pixelData;
        private static byte[] _pixelDataByte;
        private static int[][] _pixelDataHistogram;
        private static byte[][] _pixelDataLUT;
        private static bool _pixelDataReady;
        private static int _prevTickCount = 0;
        private static Report _reportCallBack;
        private static ReportIndex _reportCallBackImage;
        private static ReportSubRowEndIndex _reportSubRowEndCallBack;
        private static ReportSubRowStartIndex _reportSubRowStartCallBack;
        private static ReportTIndex _reportTCallBack;
        private static ReportZIndex _reportZCallBack;
        private static double _sumTickCount = 0;
        private static double[] _whitePoint;
        private static int _xMax;
        private static int _yMax;
        private static ReportZStackCaptureFinished _zStackPreviewFinishedCallBack;

        private BackgroundWorker _bwHardware;
        private double _coeff1;
        private double _coeff2;
        private double _coeff3;
        private double _coeff4;
        private Guid _commandGuid;

        //private DigitizerBoardNames _digitizerBoardName = DigitizerBoardNames.ATS460;
        private DigitizerBoardNames _digitizerBoardName = DigitizerBoardNames.ATS9440;
        private int _enableBackgroundSubtraction;
        private int _enableFlatField;
        private int _enablePincushionCorrection;
        int _enablePockelsMask;
        private bool _enableZRead = true;
        private double _exposureTimeMax;
        private double _exposureTimeMin;
        private String _expPath;
        string _filePockelsMask = string.Empty;
        private int _filterPositionDic;
        private int _filterPositionEm;
        private int _filterPositionEx;
        private int _gainMax;
        private int _gainMin;
        private bool _isBleaching;
        private bool _isTilesCapturing;
        private bool _isZStackCapturing;
        private int _lampPosition;
        private int _lightModeMax;
        private int _lightModeMin;
        private int _lightPathCamEnable;
        private int _lightPathGGEnable;
        private int _lightPathGREnable;
        private bool _liveStartButtonStatus;
        private bool _liveStopButtonStatus;
        private string _pathBackgroundSubtraction = string.Empty;
        private string _pathFlatField = string.Empty;
        bool _pmtSafetyStatus = true;
        private int _powerPosition;
        private int _preBleachPower;
        private int _preBleachWavelength;
        private bool _restartBWHardware = false;
        private int _rollOverPointX;
        private int _rollOverPointY;
        private RunSampleLSCustomParams _rsParams = new RunSampleLSCustomParams();
        private int _shutterPosition;
        private int _turretPosition;
        private Color _wavelengthColor;
        private double _xPosition;
        private double _yPosition;
        private double _zPosition;

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
            _exposureTimeMin = .001;
            _exposureTimeMax = 1000;
            _pixelDataReady = false;
            _liveStartButtonStatus = true;
            _liveStopButtonStatus = false;
            _isZStackCapturing = false;
            _isTilesCapturing = false;
            _isBleaching = false;
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
            _lightPathGGEnable = 0;
            _lightPathGREnable = 1;
            _lightPathCamEnable = 0;

            //enable the first 4 channels only
            for (int i = 0; i < MAX_CHANNELS; i++)
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

            _bwHardware = new BackgroundWorker();
            _bwHardware.WorkerReportsProgress = false;
            _bwHardware.WorkerSupportsCancellation = true;

            _bwHardware.DoWork += new DoWorkEventHandler(_bwHardware_DoWork);
            _bwHardware.RunWorkerCompleted += new RunWorkerCompletedEventHandler(_bwHardware_RunWorkerCompleted);

            CreateCallbackHandlers();

            try
            {
                //attach to the live image data dll
                GetCommandGUID(ref _commandGuid);

                RegisterCallbackHandlers();

                GetTotalColorChannels();
            }
            catch (System.DllNotFoundException e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " DllNotFoundException " + e.Message);
            }

            //create one instance of the histogram arrays
            //_pixelDataHistogram = new int[_colorChannels][];
            _pixelDataHistogram = new int[MAX_CHANNELS][];

            //for (int i = 0; i < _colorChannels; i++)
            for (int i = 0; i < MAX_CHANNELS; i++)
            {
               _pixelDataHistogram[i] = new int[PIXEL_DATA_HISTOGRAM_SIZE];
            }

            _pal = new byte[_colorChannels][];

            for (int i = 0; i < _colorChannels; i++)
            {
                _pal[i] = new byte[ushort.MaxValue + 1];
            }

            _colorAssigment = new int[_colorChannels];

            LockFieldOffset = true;

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

        public enum ColorAssignments
        {
            RED,
            GREEN,
            BLUE,
            CYAN,
            MAGENTA,
            YELLOW,
            GRAY,
            TRANSPARENT
        }

        public enum DigitizerBoardNames
        {
            ATS460,
            ATS9440
        }

        #endregion Enumerations

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void Report(ref int index, ref int completed, ref int total, ref int timeElapsed, ref int timeRemaining);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportBleachNowFinished();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportNewImage(IntPtr returnArray, ref int colorChannels);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportSubRowEndIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportSubRowStartIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportTIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportZIndex(ref int index);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportZStackCaptureFinished();

        #endregion Delegates

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        public event Action<bool> UpdateImage;

        public event Action<bool> UpdateMenuBarButton;

        #endregion Events

        #region Properties

        public static int EnableReferenceChannel
        {
            get;
            set;
        }

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

        public int BinX
        {
            get { return _liParams.binX.val; }
            set
            {
                int temp = _liParams.binX.val;

                _liParams.binX.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.binX.val = temp;
            }
        }

        public int BinY
        {
            get { return _liParams.binY.val; }
            set
            {
                int temp = _liParams.binY.val;

                _liParams.binY.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.binY.val = temp;
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

        public int BleachPower
        {
            get;
            set;
        }

        public int BleachPowerEnable
        {
            get;
            set;
        }

        public int BleachQuery
        {
            get;
            set;
        }

        public int BleachWavelength
        {
            get;
            set;
        }

        public int BleachWavelengthEnable
        {
            get;
            set;
        }

        public int Bottom
        {
            get { return _liParams.bottom.val; }
            set
            {
                int temp = _liParams.bottom.val;

                _liParams.bottom.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.bottom.val = temp;
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

        public int EnablePockelsMask
        {
            get
            {
                return _enablePockelsMask;
            }
            set
            {
                _enablePockelsMask = value;
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_POCKELS_MASK_ENABLE, (double)_enablePockelsMask);
            }
        }

        public double ExposureTimeCam0
        {
            get {
                Decimal dec = new Decimal(_liParams.exposureTimeCam0.val);

                return Decimal.ToDouble(Decimal.Round(dec,3));
            }
            set
            {
                double temp = _liParams.exposureTimeCam0.val;

                _liParams.exposureTimeCam0.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        Decimal dec = new Decimal(value);

                        temp = Decimal.ToDouble(Decimal.Round(dec,3));
                    }
                }

                _liParams.exposureTimeCam0.val = temp;
            }
        }

        public double ExposureTimeCam1
        {
            get { return _liParams.exposureTimeCam1.val; }
            set
            {
                double temp = _liParams.exposureTimeCam1.val;

                _liParams.exposureTimeCam1.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.exposureTimeCam1.val = temp;
            }
        }

        public double ExposureTimeCam2
        {
            get { return _liParams.exposureTimeCam2.val; }
            set
            {
                double temp = _liParams.exposureTimeCam2.val;

                _liParams.exposureTimeCam2.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.exposureTimeCam2.val = temp;
            }
        }

        public double ExposureTimeMax
        {
            get
            {
                double exMin = 0;
                double exMax = 0;
                double exDefault = 0;

                if (GetCameraParameterRangeDouble((int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, ref exMin, ref exMax, ref exDefault))
                {
                    _exposureTimeMax = exMax;

                }
                return _exposureTimeMax;
            }
            set
            {
            }
        }

        public double ExposureTimeMin
        {
            get
            {
                double exMin = 0;
                double exMax = 0;
                double exDefault = 0;

                if (GetCameraParameterRangeDouble((int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, ref exMin, ref exMax, ref exDefault))
                {
                    _exposureTimeMin = exMin;

                }
                return _exposureTimeMin;
            }
            set
            {
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

        public int Gain
        {
            get { return _liParams.gain.val; }
            set
            {
                int temp = _liParams.gain.val;

                _liParams.gain.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.gain.val = temp;
            }
        }

        public int GainMax
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                if (GetCameraParameterRangeInt((int)ICamera.Params.PARAM_GAIN, ref exMin, ref exMax, ref exDefault))
                {
                    _gainMax = exMax;

                }
                return _gainMax;
            }
            set
            {
            }
        }

        public int GainMin
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                if (GetCameraParameterRangeInt((int)ICamera.Params.PARAM_GAIN, ref exMin, ref exMax, ref exDefault))
                {
                    _gainMin = exMin;

                }
                return _gainMin;
            }
            set
            {
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

        public bool IsBleaching
        {
            get
            {
                return _isBleaching;
            }
            set
            {
                _isBleaching = value;
                OnPropertyChanged("IsBleaching");
            }
        }

        public bool IsTilesCapturing
        {
            get
            {
                return _isTilesCapturing;
            }
            set
            {
                _isTilesCapturing = value;
                OnPropertyChanged("IsTilesCapturing");
            }
        }

        public bool IsZStackCapturing
        {
            get
            {
                return _isZStackCapturing;
            }
            set
            {
                _isZStackCapturing = value;
                OnPropertyChanged("IsZStackCapturing");
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

        public int Left
        {
            get { return _liParams.left.val; }
            set
            {
                int temp = _liParams.left.val;

                _liParams.left.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.left.val = temp;
            }
        }

        public int LightMode
        {
            get { return _liParams.lightMode.val; }
            set
            {
                int temp = _liParams.lightMode.val;

                _liParams.lightMode.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.lightMode.val = temp;
            }
        }

        public int LightModeMax
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                if (GetCameraParameterRangeInt((int)ICamera.Params.PARAM_LIGHT_MODE, ref exMin, ref exMax, ref exDefault))
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

                if (GetCameraParameterRangeInt((int)ICamera.Params.PARAM_LIGHT_MODE, ref exMin, ref exMax, ref exDefault))
                {
                    _lightModeMin = exMin;

                }
                return _lightModeMin;
            }
            set
            {
            }
        }

        public int LightPathCamEnable
        {
            get
            {
                long val = 0;
                if (GetLightPath(2, ref val))   //0: LIGHTPATH_GG, 1: LIGHTPATH_GR, 2: LIGHTPATH_CAMERA
                {
                    _lightPathCamEnable = (int)val;
                }
                return _lightPathCamEnable;
            }
            set
            {
                if (SetLightPath(2, value))
                {
                    _lightPathCamEnable = value;
                }
            }
        }

        public int LightPathGGEnable
        {
            get
            {
                long val = 0;
                if (GetLightPath(0, ref val))   //0: LIGHTPATH_GG, 1: LIGHTPATH_GR, 2: LIGHTPATH_CAMERA
                {
                    _lightPathGGEnable = (int)val;
                }
                return _lightPathGGEnable;
            }
            set
            {
                if (SetLightPath(0, value))
                {
                    _lightPathGGEnable = value;
                }
            }
        }

        public int LightPathGREnable
        {
            get
            {
                long val = 0;
                if (GetLightPath(1, ref val))   //0: LIGHTPATH_GG, 1: LIGHTPATH_GR, 2: LIGHTPATH_CAMERA
                {
                    _lightPathGREnable = (int)val;
                }
                return _lightPathGREnable;
            }
            set
            {
                if (SetLightPath(1, value))
                {
                    _lightPathGREnable = value;
                }
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

        public bool LockFieldOffset
        {
            get; set;
        }

        public int LSM1xFieldSize
        {
            get
            {
                int val = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_LSM_1X_FIELD_SIZE, ref val);
                return val;
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
                                //_liParams.lsmChannel.val = (Convert.ToInt32(LSMChannelEnable[0]) | (Convert.ToInt32(LSMChannelEnable[1]) << 1) | (Convert.ToInt32(LSMChannelEnable[2]) << 2) | (Convert.ToInt32(LSMChannelEnable[3]) << 3));
                                _liParams.lsmChannel.val = 0xf;   // capture data from all channels under color mode
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
                                _liParams.lsmChannel.val = (Convert.ToInt32(LSMChannelEnable0) | (Convert.ToInt32(LSMChannelEnable1) << 1));
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

        public bool LSMChannelEnable0
        {
            get
            {
                return _lsmChannelEnable[0];
            }
            set
            {
                _lsmChannelEnable[0] = value;
                SetChannelFromEnable();
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
                _lsmChannelEnable[1] = value;
                SetChannelFromEnable();
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
                _lsmChannelEnable[2] = value;
                SetChannelFromEnable();
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
                _lsmChannelEnable[3] = value;
                SetChannelFromEnable();
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
            get
            {
                double xyRatio = (double)LSMPixelY / (double)LSMPixelX;
                //return (int)((_liParams.lsmFieldOffsetX.val + 128 - (int)(this.LSMFieldSize / (Math.Sqrt(1 + (xyRatio * xyRatio)) * 2.0))));
                return (int)Math.Round((_liParams.lsmFieldOffsetX.val + 128 - (this.LSMFieldSize / (Math.Sqrt(1 + (xyRatio * xyRatio)) * 2.0))), 0);
            }
            set
            {
                double xyRatio = (double)LSMPixelY / (double)LSMPixelX;
                int temp = _liParams.lsmFieldOffsetX.val;

                //int testVal = value - 128 + (int)(this.LSMFieldSize / (Math.Sqrt(1 + (xyRatio * xyRatio)) * 2.0));
                int testVal = (int)Math.Round(value - 128 + (this.LSMFieldSize / (Math.Sqrt(1 + (xyRatio * xyRatio)) * 2.0)), 0);

                if (LockFieldOffset)
                {
                    testVal = 0;
                }

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

        public double LSMFieldOffsetXFine
        {
            get
            {
                double val=0.0;
                GetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_FINE_OFFSET_X,ref val);
                return val;
            }
            set
            {
                double val = Math.Round(value, 3); ;
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_FINE_OFFSET_X,val);
            }
        }

        public int LSMFieldOffsetY
        {
            get
            {
                double xyRatio = (double)LSMPixelY / (double)LSMPixelX;
                //return (int)((_liParams.lsmFieldOffsetY.val + 128 - (int)(this.LSMFieldSize / (Math.Sqrt(1 + 1 / (xyRatio * xyRatio)) * 2.0))));
                return (int)Math.Round((_liParams.lsmFieldOffsetY.val + 128 - (this.LSMFieldSize / (Math.Sqrt(1 + 1 / (xyRatio * xyRatio)) * 2.0))),0);
            }
            set
            {
                double xyRatio = (double)LSMPixelY / (double)LSMPixelX;
                int temp = _liParams.lsmFieldOffsetY.val;
                int testVal = (int)Math.Round(value - 128 + (this.LSMFieldSize / (Math.Sqrt(1 + 1 / (xyRatio * xyRatio)) * 2.0)), 0);
                if (LockFieldOffset)
                {
                    testVal = 0;
                }

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

        public double LSMFieldOffsetYFine
        {
            get
            {
                double val = 0.0;
                GetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_FINE_OFFSET_Y, ref val);
                return val;
            }
            set
            {
                double val = Math.Round(value, 3); ;
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_FINE_OFFSET_Y, val);
            }
        }

        public double LSMFieldScaleXFine
        {
            get
            {
                double val = 0.0;
                GetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_X,ref val);
                return val;
            }
            set
            {
                double val = Math.Round(value, 3);;
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_X, val);
            }
        }

        public double LSMFieldScaleYFine
        {
            get
            {
                double val = 0.0;
                GetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y, ref val);
                return val;
            }
            set
            {
                double val = Math.Round(value, 3); ;
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y, val);
            }
        }

        public int LSMFieldSize
        {
            get
            {
                int val=0;
                GetFieldSize(ref val);
                return val;
            }
            set
            {
                SetFieldSize(value);

                // set the two way alignment to its current dispayed value
                LSMTwoWayAlignmentCoarse = LSMTwoWayAlignmentCoarse;
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

        public int LSMFlipVerticalScan
        {
            get
            {
                int val = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_LSM_VERTICAL_SCAN_DIRECTION, ref val);
                return val;
            }
            set
            {
                SetCameraParameterValueInt((int)ICamera.Params.PARAM_LSM_VERTICAL_SCAN_DIRECTION, value);
            }
        }

        public int LSMGalvoRate
        {
            get;
            set;
        }

        public int LSMLineScanEnable
        {
            get
            {
                int val = 0;
                if (GetLineScanEnable(ref val))
                {
                }
                return val;
            }
            set
            {
                SetLineScanEnable(value);
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

        public double LSMPixelDwellTime
        {
            get
            {
                double val = 0;

                GetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_DWELL_TIME, ref val);

                return val;

            }

            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_DWELL_TIME, value);
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

        public int LSMRealtimeAveraging
        {
            get
            {
                int val = 1;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE, ref val);
                return val;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE,(double)value);
            }
        }

        public double LSMScaleYScan
        {
            get
            {
                int val = 1;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_LSM_Y_AMPLITUDE_SCALER, ref val);

                double dVal = val / 100.0;

                Decimal dec = new Decimal(dVal);

                dec = Decimal.Round(dec,2);
                return Convert.ToDouble(dec.ToString());
            }
            set
            {
                int val = Convert.ToInt32(value * 100);
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_Y_AMPLITUDE_SCALER, (double)val);
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

        public string LSMScannerName
        {
            get
            {
                string str = string.Empty;
                int type=0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_LSM_TYPE, ref type);

                switch((ICamera.LSMType)type)
                {
                    case ICamera.LSMType.GALVO_RESONANCE: str = "Galvo/Resonance Scanner"; break;
                    case ICamera.LSMType.GALVO_GALVO: str = "Galvo/Galvo Scanner"; break;
                }

                return str;
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
            get //{ return _liParams.lsmTwoWayAlignment.val; }
            {
                int val = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_LSM_ALIGNMENT, ref val);
                _liParams.lsmTwoWayAlignment.val = val;
                return _liParams.lsmTwoWayAlignment.val;
            }
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

        public int LSMTwoWayAlignmentCoarse
        {
            get
            {
                int val = 0;
                GetTwoWayAlignmentCoarse(LSMFieldSize, ref val);
                return val;
            }
            set
            {
                SetTwoWayAlignmentCoarse(LSMFieldSize, value);
            }
        }

        public int NAcquireChannels
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
                        case 1: return 1;
                        case 2: return 1;
                        default: return 2;
                    }
                }
                else
                {
                    MessageBox.Show("Error in Model.LiveImage.DigitizerBoardName setter", "Unrecognized Boardname");
                    return -1;
                }
            }
             //           set
             //           {
             //               if (DigitizerBoardName == DigitizerBoardNames.ATS9440)
             //               {
             //                   // number of the current active channels can be either 1 or 4
             //                   switch (value)
             //                   {
             //                       case 4: _nAcquireChannels = 4; break;
             //                      default:
             //                           {
             //                               _nAcquireChannels = 1;
             //                           }
             //                           break;
             //                  }
             //               }
             //               else if (DigitizerBoardName == DigitizerBoardNames.ATS460)
             //               {
             //                   // number of the current active channels can be either 1 or 2
             //                   switch (value)
             //                   {
             //                       case 4: _nAcquireChannels = 2; break;
             //                       default:
             //                           {
             //                               _nAcquireChannels = 1;
             //                           }
             //                          break;
             //                   }
             //               }
             //               else
             //               {
             //                   MessageBox.Show("Error in Model.LiveImage.DigitizerBoardName setter", "Unrecognized Boardname");
             //               }
             //           }
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
                OnPropertyChanged("PMT1Gain");
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
                OnPropertyChanged("PMT2Gain");
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
                OnPropertyChanged("PMT3Gain");
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
                OnPropertyChanged("PMT4Gain");
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
                return _pmtSafetyStatus;
            }
        }

        public string PockelsMaskFile
        {
            get
            {
                return _filePockelsMask;
            }
            set
            {
                _filePockelsMask = value;
                SetPockelsMaskFile(_filePockelsMask);
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

                GetCameraParameterValueInt((int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE, ref pos);

                return pos;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE,(double)value);
            }
        }

        public int PowerPosition
        {
            get
            {
                return _powerPosition;
            }
            set
            {
                SetPowerPosition(value);

            }
        }

        public int Right
        {
            get { return _liParams.right.val; }
            set
            {
                int temp = _liParams.right.val;

                _liParams.right.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.right.val = temp;
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

        public int Top
        {
            get { return _liParams.top.val; }
            set
            {
                int temp = _liParams.top.val;

                _liParams.top.val = value;

                if (SetCustomParamsBinary(ref _liParams) == 1)
                {
                    if (LiveImageDataExecute() == 1)
                    {
                        temp = value;
                    }
                }

                _liParams.top.val = temp;
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

                if (value < 0)
                {
                    return;
                }

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
                return _xPosition;
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
                return _yPosition;
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
                return _zPosition;
            }
            set
            {
                double temp = value;

                //do not allow the zposition to be read while the stage is moving
                //for some z positioners (all motion) the read value will be zero while
                //the stage is moving and creates a strang behavior in the GUI with the
                //values jumping between an invalid value and the true value
                _enableZRead = false;

                if(SetZPositionNoWait(temp))
                    {
                        temp = value;
                    }

                _enableZRead = true;
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
            int shiftValue = 6;
            double shiftValueResult = 64;

            shiftValue = 6;
            shiftValueResult = Math.Pow(2, shiftValue);

            if (null == _pixelDataLUT)
            {
                _pixelDataLUT = new byte[MAX_CHANNELS][];

                for (int c = 0; c < MAX_CHANNELS; c++)
                {
                    _pixelDataLUT[c] = new byte[ushort.MaxValue + 1];
                }
            }

            // if there is no new image and only the GUI pixel count changed, return _pizelDataByte without updating it
            if (_pixelData.Length != _dataLength)
            {
                return _pixelDataByte;
            }
            //Build the 12/14-bit to 8-bit Lut
            for (int c = 0; c < MAX_CHANNELS; c++)
            {
                for (int i = 0; i < _pixelDataLUT[c].Length; i++)
                {
                    double val = (255.0 / (shiftValueResult * (_whitePoint[c] - _blackPoint[c]))) * (i - _blackPoint[c] * shiftValueResult);
                    //val = (val >= 0) ? val : 0;
                    //val = (val <= 255) ? val : 255;
                    //reserve Min & Max for special display:
                    if (i <= (_pixelDataLUT[c].Length >> 2)-2)      //16382
                    {
                        val = (val <= 1) ? 1 : val;
                        val = (val <= 254) ? val : 254;
                    }
                    else
                    { val = 255; }                                  //>=16383
                    if (i == 0)
                    { val = 0; }
                    _pixelDataLUT[c][i] = (byte)Math.Round(val);
                }
            }

            for (int c = 0; c < _colorChannels; c++)
            {
                for (int i = 0; i < _dataLength; i++)
                {
                    byte val;

                    if (_pixelData[i] < 0)
                    {
                        val = _pixelDataLUT[c][(_pixelData[i] + 32768)];
                    }
                    else
                    {
                        val = _pixelDataLUT[c][(_pixelData[i])];
                    }

                    _pixelDataByte[i] = val;
                }
            }

            return _pixelDataByte;
        }

        public static byte[] GetPixelDataByteEx(bool doColor, int channelIndex)
        {
            IntPtr refChannIntPtr;
            short[] refChannShortArray = null;

            //need to rebuid the color image because a palette option is not available for RGB images
            if ((_colorChannels > 1) && (_pixelData != null) && (_dataLength * _colorChannels == _pixelData.Length))
            {
                int i;

                if (doColor)
                {
                    //clear the histogram
                    for (i = 0; i < PIXEL_DATA_HISTOGRAM_SIZE; i++)
                    {
                        //for (int k = 0; k < _colorChannels; k++)
                        for (int k = 0; k < MAX_CHANNELS; k++)
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

                // load reference channel
                bool refToRefChann = false;
                string refChannDir = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\ThorImageLS\\ReferenceChannel.tif";

                if ((1==EnableReferenceChannel) && File.Exists(refChannDir))   // ref channel file existance
                {
                    long width = 0;
                    long height = 0;
                    long colorChannels = 0;
                    if (LoadRefChannInfo(refChannDir, ref width, ref height, ref colorChannels))    // load dimention of ref image
                    {
                        if (width * height == _dataLength)
                        {
                            refChannIntPtr = Marshal.AllocHGlobal(Convert.ToInt32(width) * Convert.ToInt32(height) * 2);

                            if (LoadRefChann(refChannDir, ref refChannIntPtr))  // load ref image
                            {
                                try
                                {
                                    refChannShortArray = new short[Convert.ToInt32(width) * Convert.ToInt32(height)];
                                    Marshal.Copy(refChannIntPtr, refChannShortArray, 0, Convert.ToInt32(width) * Convert.ToInt32(height));
                                    refToRefChann = true;
                                }
                                catch (Exception e)
                                {
                                    ThorLog.Instance.TraceEvent(TraceEventType.Information,1,e.Message);
                                }
                            }
                            else
                            {

                            }
                        }
                        else
                        {

                        }
                    }
                }

                //factor is derived by taking CAMERA_MAX_INTENSITY_VALUE/256
                //const double FULL_RANGE_NORMALIZATION_FACTOR = 64.0;
                for (i = 0, j = 0; j < _dataLength; i += 3, j++)
                {
                    ushort valRaw;
                    const int SHIFT_VALUE = 6;
                    byte valRawHist;

                    byte maxRed = 0;
                    byte maxGreen = 0;
                    byte maxBlue = 0;

                    _pixelDataByte[i] = 0;
                    _pixelDataByte[i + 1] = 0;
                    _pixelDataByte[i + 2] = 0;

                    for (int k = 0; k < enabledChannelCount; k++)
                    {
                        valRaw = (ushort)(_pixelData[j + dataBufferOffsetIndex[k] * _dataLength]);
                        valRawHist = (byte)(valRaw >> SHIFT_VALUE);

                        ushort newRaw = (byte)(valRaw >> 1);
                        byte tmp = 0;
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
                                    //_pixelDataByte[i + 1] = maxGreen = Math.Max(maxGreen, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                    //_pixelDataByte[i + 2] = maxBlue = Math.Max(maxBlue, _pal[dataBufferOffsetIndex[k]][newRaw]);

                                    tmp = Math.Max(maxGreen, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                    maxGreen = tmp;
                                    _pixelDataByte[i+1] = tmp;

                                    tmp = Math.Max(maxBlue, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                    maxBlue = tmp;
                                    _pixelDataByte[i+2] = tmp;
                                }
                                break;
                            case ColorAssignments.MAGENTA:
                                {
                                    //_pixelDataByte[i] = maxRed = Math.Max(maxRed, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                    //_pixelDataByte[i + 2] = maxBlue = Math.Max(maxBlue, _pal[dataBufferOffsetIndex[k]][newRaw]);

                                    tmp = Math.Max(maxRed, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                    maxRed = tmp;
                                    _pixelDataByte[i] = tmp;

                                    tmp = Math.Max(maxBlue, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                    maxBlue = tmp;
                                    _pixelDataByte[i+2] = tmp;
                                }
                                break;
                            case ColorAssignments.YELLOW:
                                {
                                    //_pixelDataByte[i] = maxRed = Math.Max(maxRed, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                    //_pixelDataByte[i + 1] = maxGreen = Math.Max(maxGreen, _pal[dataBufferOffsetIndex[k]][newRaw]);

                                    tmp = Math.Max(maxRed, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                    maxRed = tmp;
                                    _pixelDataByte[i] = tmp;

                                    tmp = Math.Max(maxGreen, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                    maxGreen = tmp;
                                    _pixelDataByte[i+1] = tmp;
                                }
                                break;
                            case ColorAssignments.GRAY:
                                {
                                    //_pixelDataByte[i] = maxRed = Math.Max(maxRed, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                    //_pixelDataByte[i + 1] = maxGreen = Math.Max(maxGreen, _pal[dataBufferOffsetIndex[k]][newRaw]);
                                    //_pixelDataByte[i + 2] = maxBlue = Math.Max(maxBlue, _pal[dataBufferOffsetIndex[k]][newRaw]);

                                    tmp = Math.Max(maxRed, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                    maxRed = tmp;
                                    _pixelDataByte[i] = tmp;

                                    tmp = Math.Max(maxGreen, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                    maxGreen = tmp;
                                    _pixelDataByte[i + 1] = tmp;

                                    tmp = Math.Max(maxBlue, _pal[dataBufferOffsetIndex[k]][valRaw]);
                                    maxBlue = tmp;
                                    _pixelDataByte[i + 2] = tmp;
                                }
                                break;
                            case ColorAssignments.TRANSPARENT:
                                {
                                    _pixelDataByte[0] = 0;
                                    _pixelDataByte[1] = 0;
                                    _pixelDataByte[2] = 0;
                                    _pixelDataByte[3] = 0;
                                }
                                break;
                        }

                        //only build the histogram if the color mode is selected.
                        //This will allow histograms for all of the channels to be available simultaneously
                        if (doColor)
                        {
                            _pixelDataHistogram[dataBufferOffsetIndex[k]][valRawHist]++;
                        }
                    } // k

                    if ((refToRefChann)&& ((true == doColor) || ((false == doColor)&&(3 == channelIndex))))
                    {
                        byte tmp = 0;
                        valRaw = (ushort)refChannShortArray[j];
                        tmp = Math.Max(maxRed, _pal[3][valRaw]);
                        maxRed = tmp;
                        _pixelDataByte[i] = tmp;
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

        public static bool LoadRefChann(string fileName, ref IntPtr refChannPtr)
        {
            bool status = ReadImage(fileName, ref refChannPtr);
            return status;
        }

        public static bool LoadRefChannInfo(string fileName, ref long width, ref long height, ref long colorChannels)
        {
            bool status = ReadImageInfo(fileName, ref width, ref height, ref colorChannels);
            return status;
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

        public int CameraType()
        {
            int camType=0;
            GetCameraType(ref camType);

            return camType;
        }

        public int CenterROI()
        {
            _liParams.lsmFieldOffsetX.val = 0;
            _liParams.lsmFieldOffsetY.val = 0;

            if (SetCustomParamsBinary(ref _liParams) == 1)
            {
                if (LiveImageDataExecute() == 1)
                {
                }
            }
            return 0;
        }

        public void ConnectHandlers()
        {
            InitCallBack(_imageCallBack, _zStackPreviewFinishedCallBack);
            InitCallBackBleach(_BleachNowFinishedCallBack);

            RSInitCallBack
                (
                _reportCallBack,
                _reportCallBackImage,
                _reportSubRowStartCallBack,
                _reportSubRowEndCallBack,
                _reportZCallBack,
                _reportTCallBack
                );

            if (false == _bwHardware.IsBusy)
            {
                _bwHardware.RunWorkerAsync();
            }
            else if (true == _bwHardware.CancellationPending)
            {
                _restartBWHardware = true;
            }
        }

        public bool GetBleacherFieldSizeCalibration(ref double fieldSizeCal)
        {
            return GetBleachScannerParameterValueDouble((int)ICamera.Params.PARAM_LSM_FIELD_SIZE_CALIBRATION, ref fieldSizeCal);
        }

        public bool GetBleacherType(ref long bleachScannerType)
        {
            bool getBleachScanner = GetBleachScannerType(ref bleachScannerType);
            return getBleachScanner;
        }

        public int GetLSMType()
        {
            int lsmType = 0;
            GetLSMType(ref lsmType);
            return lsmType;
        }

        public bool LIGetFieldSizeCalibration(ref double fieldSizeCal)
        {
            return GetCameraParameterValueDouble((int)ICamera.Params.PARAM_LSM_FIELD_SIZE_CALIBRATION, ref fieldSizeCal);
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

        public void OnPropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        public void ReleaseHandlers()
        {
            _bwHardware.CancelAsync();
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

        public void StartBleach(int bfieldSize, int bwidth, int bheight, int boffsetX, int boffsetY, int FrameNum)
        {
            //ensure the buffer is copied after the capture
            _pixelDataReady = false;

            if (SetCustomParamsBinary(ref _liParams) == 1)
            {
                //Execute will set all of the capture parameters
                if (LiveImageDataExecute() == 1)
                {
                    //stop the background hardware updates
                    _bwHardware.CancelAsync();

                    DisablePMTGains();

                    if (1 == BleachWavelengthEnable)
                    {
                        _preBleachWavelength = Laser1Position;
                        Laser1Position = BleachWavelength;
                        System.Threading.Thread.Sleep(5000);
                    }

                    if (1 == BleachPowerEnable)
                    {
                        _preBleachPower = PowerPosition;

                        SetBleachPowerPosition(BleachPower);
                    }

                    if (1 == BleachQuery)
                    {
                        //power control will position while the use is being queried
                        MessageBox.Show("The scanner is now ready to bleach. Make your manual light path adjustments now. Then press OK", "Ready to Bleach", MessageBoxButton.OK);
                    }
                    else
                    {
                        //wait for the power to control to position
                        System.Threading.Thread.Sleep(5000);
                    }

                    //if the scanner is flipped change the sign of the offset value
                    int flip=0;
                    if (true == GetBleachScannerParameterValueInt((int)ICamera.Params.PARAM_LSM_VERTICAL_SCAN_DIRECTION, ref flip))
                    {
                        //boffsetX *= (1 == flip) ? -1 : 1;
                    }

                    LiveBleach(bfieldSize, bwidth, bheight, boffsetX, boffsetY, FrameNum);

                    //restart the background hardware updates
                    if (false == _bwHardware.IsBusy)
                    {
                        _bwHardware.RunWorkerAsync();
                    }
                    else if (true == _bwHardware.CancellationPending)
                    {
                        _restartBWHardware = true;
                    }
                }
            }
        }

        public void StartTilesPreview(string dir)
        {
            _rsParams.path = dir + "/Experiment.xml";

            if (SetCustomParamsBinary(ref _rsParams) == 1)
            {
                if (RunSampleLSExecute() == 1)
                {
                }
            }
        }

        public void StartZStackPreview(double zStartPos, double zStopPos, double zstageStepSize, long zstageSteps)
        {
            if (SetCustomParamsBinary(ref _liParams) == 1)
            {
                //Execute will set all of the capture parameters
                if (LiveImageDataExecute() == 1)
                {
                    //stop the background hardware updates
                    _bwHardware.CancelAsync();

                    DisablePMTGains();
                    EnablePMTGains();
                    CaptureZStack(zStartPos, zStopPos, zstageStepSize, zstageSteps);

                    //restart the background hardware updates
                    if (false == _bwHardware.IsBusy)
                    {
                        _bwHardware.RunWorkerAsync();
                    }
                    else if (true == _bwHardware.CancellationPending)
                    {
                        _restartBWHardware = true;
                    }
                }
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

        public void StopBleach()
        {
            StopLiveBleach();
            ///EnablePMTGains();
        }

        public void StopTilesPreview()
        {
            DisablePMTGains();
            RunSampleLSStop();
        }

        public bool StopZ()
        {
            return StopZStage();
        }

        public void StopZStackPreview()
        {
            DisablePMTGains();
            StopZStackCapture();
        }

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "AutoExposure")]
        private static extern bool AutoExposure(double exposure, ref double exposureResult, ref double multiplier);

        private static void BuildRGBColorPalettes()
        {
            int shiftValue;
            double shiftValueResult;

            shiftValue = 6;
            shiftValueResult = Math.Pow(2, shiftValue);

            for (int j = 0; j < _colorChannels; j++)
            {
                for (int i = 0; i < ushort.MaxValue + 1; i++)
                {
                    double val = (255.0 / (shiftValueResult * (_whitePoint[j] - _blackPoint[j]))) * (i - _blackPoint[j] * shiftValueResult);
                    val = (val >= 0) ? val : 0;
                    val = (val <= 255) ? val : 255;

                    _pal[j][i] = (byte)val;
                }
            }
        }

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "CaptureTiles")]
        private static extern bool CaptureTiles();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "CaptureZStack")]
        private static extern bool CaptureZStack(double zStartPos, double zStopPos, double zstageStepSize, long zstageSteps);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "EnableCopyToExternalBuffer")]
        private static extern bool EnableCopyToExternalBuffer();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetBleachScannerParameterValueDouble")]
        private static extern bool GetBleachScannerParameterValueDouble(int param, ref double value);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetBleachScannerParameterValueLong")]
        private static extern bool GetBleachScannerParameterValueInt(int param, ref int value);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetBleachScannerType")]
        private static extern bool GetBleachScannerType(ref long type);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraParameterRangeDouble")]
        private static extern bool GetCameraParameterRangeDouble(int param, ref double valMin, ref double valMax, ref double valDefault);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraParameterRangeInt")]
        private static extern bool GetCameraParameterRangeInt(int param, ref int valMin, ref int valMax, ref int valDefault);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraParameterValueDouble")]
        private static extern bool GetCameraParameterValueDouble(int param, ref double value);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraParameterValueInt")]
        private static extern bool GetCameraParameterValueInt(int param, ref int value);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraType")]
        private static extern bool GetCameraType(ref int type);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCommandGUID")]
        private static extern int GetCommandGUID(ref Guid guid);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCustomParamsBinary")]
        private static extern int GetCustomParamsBinary(ref LiveImageDataCustomParams lidParams);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetFieldSize")]
        private static extern bool GetFieldSize(ref int fieldSize);

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

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetLightPath")]
        private static extern bool GetLightPath(int id, ref long val);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetLineScanEnable")]
        private static extern bool GetLineScanEnable(ref int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetLSMType")]
        private static extern bool GetLSMType(ref int type);

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

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetTwoWayAlignmentCoarse")]
        private static extern int GetTwoWayAlignmentCoarse(int fieldSize, ref int value);

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
                    //_pixelDataHistogram = new int[_colorChannels][];
                    _pixelDataHistogram = new int[MAX_CHANNELS][];

                    if (_colorChannels == 1)
                    {
                        _pixelDataByte = new byte[_dataLength];
                    }
                    else
                    {
                        _pixelDataByte = new byte[_dataLength * 3];
                    }

                    //for (int i = 0; i < _colorChannels; i++)
                    for (int i = 0; i < MAX_CHANNELS; i++)
                    {
                        _pixelDataHistogram[i] = new int[PIXEL_DATA_HISTOGRAM_SIZE];
                    }
                }

                //2^n  == FULL_RANGE_NORMALIZATION_FACTOR
                const int SHIFT_VALUE = 6;
                switch (_colorChannels)
                {
                    case 1:
                        {
                            Marshal.Copy(returnArray, _pixelData, 0, _dataLength * _colorChannels);

                            //factor is derived by taking CAMERA_MAX_INTENSITY_VALUE/256
                            //const double FULL_RANGE_NORMALIZATION_FACTOR = 64.0;

                            //clear the histogram
                            for (int i = 0; i < PIXEL_DATA_HISTOGRAM_SIZE; i++)
                            {
                                _pixelDataHistogram[0][i] = 0;
                            }

                            for (int i = 0; i < _dataLength * _colorChannels; i++)
                            {
                                double valHist;

                                if (_pixelData[i] < 0)
                                {
                                    valHist = (_pixelData[i] + 32768) >> SHIFT_VALUE;
                                }
                                else
                                {
                                    valHist = (_pixelData[i]) >> SHIFT_VALUE;
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
        private static extern void InitCallBack(ReportNewImage reportNewImage, ReportZStackCaptureFinished reportZStackCaptureFinished);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "InitCallBackBleach")]
        private static extern void InitCallBackBleach(ReportBleachNowFinished reportBleachNowFinished);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "Bleach")]
        private static extern bool LiveBleach(int bfieldSize, int bwidth, int bheight, int boffsetX, int boffsetY, int FrameNum);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "Execute")]
        private static extern int LiveImageDataExecute();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetupCommand")]
        private static extern int LiveImageDataSetupCommand();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "Snapshot")]
        private static extern bool LiveSnapshot();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SnapshotColor")]
        private static extern bool LiveSnapshotColor(int redEx, int redDic, int redEm, double redExp, int greenEx, int greenDic, int greenEm, double greenExp, int blueEx, int blueDic, int blueEm, double blueExp, int useGray, double grayExp);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImage")]
        private static extern bool ReadImage([MarshalAs(UnmanagedType.LPWStr)]string path, ref IntPtr outputBuffer);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo")]
        private static extern bool ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)]string selectedFileName, ref long width, ref long height, ref long colorChannels);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "GetCustomParamsBinary")]
        private static extern int RSGetCustomParamsBinary(ref RunSampleLSCustomParams lidParams);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "InitCallBack")]
        private static extern void RSInitCallBack(Report report, ReportIndex reportIndex, ReportSubRowStartIndex reportSubRowStartIndex, ReportSubRowEndIndex reportSubRowEndIndex, ReportZIndex reportZIndex, ReportTIndex reportTIndex);

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "Execute")]
        private static extern int RunSampleLSExecute();

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "SetupCommand")]
        private static extern int RunSampleLSSetupCommand();

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "Stop")]
        private static extern int RunSampleLSStop();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetBackgroundSubtractionEnable")]
        private static extern bool SetBackgroundSubtractionEnable(int enable);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetBackgroundSubtractionFile")]
        private static extern bool SetBackgroundSubtractionFile([MarshalAs(UnmanagedType.LPWStr)]string path);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetBFLampPosition")]
        private static extern bool SetBFLampPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetBleachPowerPosition")]
        private static extern bool SetBleachPowerPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetCameraParameterValueDouble")]
        private static extern bool SetCameraParameterValueDouble(int param, double value);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetCameraParameterValueInt")]
        private static extern bool SetCameraParameterValueInt(int param, int value);

        private static void SetChannelFromEnable()
        {
            //update the channel value also
            int chan = (Convert.ToInt32(_lsmChannelEnable[0]) | (Convert.ToInt32(_lsmChannelEnable[1]) << 1) | (Convert.ToInt32(_lsmChannelEnable[2]) << 2) | (Convert.ToInt32(_lsmChannelEnable[3]) << 3));

            switch (chan)
            {
                case 1: _liParams.lsmChannel.val = 1; break;
                case 2: _liParams.lsmChannel.val = 2; break;
                case 4: _liParams.lsmChannel.val = 4; break;
                case 8: _liParams.lsmChannel.val = 8; break;
                default:
                    {
                        _liParams.lsmChannel.val = 0xf;
                    }
                    break;
            }

            if (SetCustomParamsBinary(ref _liParams) == 1)
            {
                if (LiveImageDataExecute() == 1)
                {
                }
            }
        }

        [DllImport(".\\Modules_Native\\RunSample.dll", EntryPoint = "SetCustomParamsBinary")]
        private static extern int SetCustomParamsBinary(ref RunSampleLSCustomParams lidParams);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetCustomParamsBinary")]
        private static extern int SetCustomParamsBinary(ref LiveImageDataCustomParams lidParams);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetFieldSize")]
        private static extern bool SetFieldSize(int fieldSize);

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

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetLightPath")]
        private static extern bool SetLightPath(int id, long val);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetLineScanEnable")]
        private static extern bool SetLineScanEnable(int pos);

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

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPockelsMaskFile")]
        private static extern bool SetPockelsMaskFile([MarshalAs(UnmanagedType.LPWStr)]string path);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetPowerPosition")]
        private static extern bool SetPowerPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetShutterPosition")]
        private static extern bool SetShutterPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetTurretPosition")]
        private static extern bool SetTurretPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetTwoWayAlignmentCoarse")]
        private static extern int SetTwoWayAlignmentCoarse(int fieldSize, int value);

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

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "StopLiveBleach")]
        private static extern bool StopLiveBleach();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "StopLiveCapture")]
        private static extern bool StopLiveCapture();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "StopZStackCapture")]
        private static extern bool StopZStackCapture();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "StopZStage")]
        private static extern bool StopZStage();

        private void BleachNowFinished()
        {
            if (true == IsBleaching)
            {
                IsBleaching = false;

                if (1 == BleachPowerEnable)
                {
                    //in the event that the power control is being shared
                    //set the power back to the prebleach value
                    SetPowerPosition(_preBleachPower);
                }

                if (1 == BleachWavelengthEnable)
                {
                    Laser1Position = _preBleachWavelength;
                    System.Threading.Thread.Sleep(5000);

                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Setting Pre Bleach Wavelength " + _preBleachWavelength.ToString());

                }

                if (1 == BleachQuery)
                {
                    MessageBox.Show("The scanner has completed the bleach. Make your manual light path adjustments now. Then press OK", "Completed Bleach", MessageBoxButton.OK);
                }

                EnablePMTGains();
            }
        }

        private void CreateCallbackHandlers()
        {
            //create and assign callback for C++ unmanaged updates
            _imageCallBack = new ReportNewImage(ImageUpdate);
            _zStackPreviewFinishedCallBack = new ReportZStackCaptureFinished(ZStackFinished);
            //create and assign callback for C++ unmanaged updates
            _BleachNowFinishedCallBack = new ReportBleachNowFinished(BleachNowFinished);
            //create and assign callback for C++ unmanaged updates
            _reportCallBack = new Report(RSUpdate);
            _reportCallBackImage = new ReportIndex(RSUpdateStart);
            _reportSubRowStartCallBack = new ReportSubRowStartIndex(RSUpdateMosaicStart);
            _reportSubRowEndCallBack = new ReportSubRowEndIndex(RSUpdateMosaicEnd);
            _reportZCallBack = new ReportZIndex(RSUpdateZ);
            _reportTCallBack = new ReportTIndex(RSUpdateT);
        }

        private void DisablePMTGains()
        {
            PMT1GainEnable = 0;
            PMT2GainEnable = 0;
            PMT3GainEnable = 0;
            PMT4GainEnable = 0;
        }

        private void EnablePMTGains()
        {
            if (PMT1Gain != 0)
            {
                PMT1GainEnable = 1;
            }
            if (PMT2Gain != 0)
            {
                PMT2GainEnable = 1;
            }
            if (PMT3Gain != 0)
            {
                PMT3GainEnable = 1;
            }
            if (PMT4Gain != 0)
            {
                PMT4GainEnable = 1;
            }
        }

        private void GetTotalColorChannels()
        {
            int numCameras = 0;
            GetNumberOfCameras(ref numCameras);

            if (CameraType() == 1)
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
        }

        private void RegisterCallbackHandlers()
        {
            LiveImageDataSetupCommand();
            GetCustomParamsBinary(ref _liParams);

            //attach to the runsample dll
            RunSampleLSSetupCommand();
            RSGetCustomParamsBinary(ref _rsParams);

            ConnectHandlers();
        }

        private void RSUpdate(ref int index, ref int completed, ref int total, ref int timeElapsed, ref int timeRemaining)
        {
            if ((true == IsTilesCapturing)&&(completed == total))
            {
                IsTilesCapturing = false;
                DisablePMTGains();
            }
        }

        private void RSUpdateMosaicEnd(ref int index)
        {
        }

        //to update the current working Sub WELL status
        private void RSUpdateMosaicStart(ref int index)
        {
        }

        //to update the current working WELL status
        private void RSUpdateStart(ref int index)
        {
        }

        //to update the completed working Sub WELL status
        private void RSUpdateT(ref int index)
        {
        }

        //to update the completed working Sub WELL status
        private void RSUpdateZ(ref int index)
        {
        }

        void Trigger_UpdateImage(bool obj)
        {
            UpdateImage(true);
        }

        private void ZStackFinished()
        {
            if (true == IsZStackCapturing)
            {
                IsZStackCapturing = false;
                DisablePMTGains();
            }
        }

        void _bwHardware_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;

            DateTime lastZ = DateTime.Now;
            DateTime lastX = DateTime.Now;
            DateTime lastY = DateTime.Now;
            DateTime lastP = DateTime.Now;
            DateTime lastS = DateTime.Now;

            TimeSpan ts;

            while (true)
            {
                if ((worker.CancellationPending == true))
                {
                    e.Cancel = true;
                    break;
                }
                else
                {
                    // Perform a time consuming operation and report progress.
                    if (_enableZRead)
                    {
                        ts = DateTime.Now - lastZ;

                        if (ts.TotalSeconds > .01)
                        {
                            bool ZStatus = GetZPosition(ref _zPosition);
                            if (ZStatus)
                            {
                                Decimal dec = new Decimal(_zPosition);

                                _zPosition = Decimal.ToDouble(Decimal.Round(dec, 4));
                            }
                            lastZ = DateTime.Now;
                        }
                    }

                    ts = DateTime.Now - lastP;

                    if (ts.TotalSeconds > .2)
                    {
                        GetPowerPosition(ref _powerPosition);

                        lastP = DateTime.Now;
                    }

                    ts = DateTime.Now - lastX;

                    if (ts.TotalSeconds > .2)
                    {
                        if (GetXPosition(ref _xPosition))
                        {
                            Decimal dec = new Decimal(_xPosition);

                            _xPosition = Decimal.ToDouble(Decimal.Round(dec, 4));
                        }

                        lastX = DateTime.Now;
                    }
                    ts = DateTime.Now - lastY;

                    if (ts.TotalSeconds > .2)
                    {
                        if (GetYPosition(ref _yPosition))
                        {
                            Decimal dec = new Decimal(_yPosition);

                            _yPosition = Decimal.ToDouble(Decimal.Round(dec, 4));
                        }

                        lastY = DateTime.Now;
                    }
                    ts = DateTime.Now - lastS;

                    if (ts.TotalSeconds > 1.0)
                    {
                        _pmtSafetyStatus = GetPMTSafetyStatus();

                        lastS = DateTime.Now;
                    }
                }
            };
        }

        void _bwHardware_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Cancelled && _restartBWHardware)
            {
                _restartBWHardware = false;
                _bwHardware.RunWorkerAsync();
            }
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

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        struct RunSampleLSCustomParams
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
            public string path;
        }

        #endregion Nested Types
    }
}