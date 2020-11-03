namespace CaptureSetupDll.Model
{
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetup : INotifyPropertyChanged
    {
        #region Fields

        public const int MAX_CHANNELS = 4;

        private const int DEFAULT_BITS_PER_PIXEL = 14;
        private const double LUT_MAX = 255;
        private const double LUT_MIN = 0;
        private const int PIXEL_DATA_HISTOGRAM_SIZE = 256;

        private static readonly float[] _dflimColorMap = new float[] { 0.0f, 0.0f, 0.9125f, 0.0f, 0.0f, 0.925f, 0.0f, 0.0f, 0.9375f, 0.0f, 0.0f, 0.95f, 0.0f, 0.0f, 0.9625f, 0.0f, 0.0f, 0.975f, 0.0f, 0.0f, 0.9875f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0125f, 1.0f, 0.0f, 0.025f, 1.0f, 0.0f, 0.0375f, 1.0f, 0.0f, 0.05f, 1.0f, 0.0f, 0.0625f, 1.0f, 0.0f, 0.075f, 1.0f, 0.0f, 0.0875f, 1.0f, 0.0f, 0.1f, 1.0f, 0.0f, 0.1125f, 1.0f, 0.0f, 0.125f, 1.0f, 0.0f, 0.1375f, 1.0f, 0.0f, 0.15f, 1.0f, 0.0f, 0.1625f, 1.0f, 0.0f, 0.175f, 1.0f, 0.0f, 0.1875f, 1.0f, 0.0f, 0.2f, 1.0f, 0.0f, 0.2125f, 1.0f, 0.0f, 0.225f, 1.0f, 0.0f, 0.2375f, 1.0f, 0.0f, 0.25f, 1.0f, 0.0f, 0.2625f, 1.0f, 0.0f, 0.275f, 1.0f, 0.0f, 0.2875f, 1.0f, 0.0f, 0.3f, 1.0f, 0.0f, 0.3125f, 1.0f, 0.0f, 0.325f, 1.0f, 0.0f, 0.3375f, 1.0f, 0.0f, 0.35f, 1.0f, 0.0f, 0.3625f, 1.0f, 0.0f, 0.375f, 1.0f, 0.0f, 0.3875f, 1.0f, 0.0f, 0.4f, 1.0f, 0.0f, 0.4125f, 1.0f, 0.0f, 0.425f, 1.0f, 0.0f, 0.4375f, 1.0f, 0.0f, 0.45f, 1.0f, 0.0f, 0.4625f, 1.0f, 0.0f, 0.475f, 1.0f, 0.0f, 0.4875f, 1.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.5125f, 1.0f, 0.0f, 0.525f, 1.0f, 0.0f, 0.5375f, 1.0f, 0.0f, 0.55f, 1.0f, 0.0f, 0.5625f, 1.0f, 0.0f, 0.575f, 1.0f, 0.0f, 0.5875f, 1.0f, 0.0f, 0.6f, 1.0f, 0.0f, 0.6125f, 1.0f, 0.0f, 0.625f, 1.0f, 0.0f, 0.6375f, 1.0f, 0.0f, 0.65f, 1.0f, 0.0f, 0.6625f, 1.0f, 0.0f, 0.675f, 1.0f, 0.0f, 0.6875f, 1.0f, 0.0f, 0.7f, 1.0f, 0.0f, 0.7125f, 1.0f, 0.0f, 0.725f, 1.0f, 0.0f, 0.7375f, 1.0f, 0.0f, 0.75f, 1.0f, 0.0f, 0.7625f, 1.0f, 0.0f, 0.775f, 1.0f, 0.0f, 0.7875f, 1.0f, 0.0f, 0.8f, 1.0f, 0.0f, 0.8125f, 1.0f, 0.0f, 0.825f, 1.0f, 0.0f, 0.8375f, 1.0f, 0.0f, 0.85f, 1.0f, 0.0f, 0.8625f, 1.0f, 0.0f, 0.875f, 1.0f, 0.0f, 0.8875f, 1.0f, 0.0f, 0.9f, 1.0f, 0.0f, 0.9125f, 1.0f, 0.0f, 0.925f, 1.0f, 0.0f, 0.9375f, 1.0f, 0.0f, 0.95f, 1.0f, 0.0f, 0.9625f, 1.0f, 0.0f, 0.975f, 1.0f, 0.0f, 0.9875f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0125f, 1.0f, 0.9875f, 0.025f, 1.0f, 0.975f, 0.0375f, 1.0f, 0.9625f, 0.05f, 1.0f, 0.95f, 0.0625f, 1.0f, 0.9375f, 0.075f, 1.0f, 0.925f, 0.0875f, 1.0f, 0.9125f, 0.1f, 1.0f, 0.9f, 0.1125f, 1.0f, 0.8875f, 0.125f, 1.0f, 0.875f, 0.1375f, 1.0f, 0.8625f, 0.15f, 1.0f, 0.85f, 0.1625f, 1.0f, 0.8375f, 0.175f, 1.0f, 0.825f, 0.1875f, 1.0f, 0.8125f, 0.2f, 1.0f, 0.8f, 0.2125f, 1.0f, 0.7875f, 0.225f, 1.0f, 0.775f, 0.2375f, 1.0f, 0.7625f, 0.25f, 1.0f, 0.75f, 0.2625f, 1.0f, 0.7375f, 0.275f, 1.0f, 0.725f, 0.2875f, 1.0f, 0.7125f, 0.3f, 1.0f, 0.7f, 0.3125f, 1.0f, 0.6875f, 0.325f, 1.0f, 0.675f, 0.3375f, 1.0f, 0.6625f, 0.35f, 1.0f, 0.65f, 0.3625f, 1.0f, 0.6375f, 0.375f, 1.0f, 0.625f, 0.3875f, 1.0f, 0.6125f, 0.4f, 1.0f, 0.6f, 0.4125f, 1.0f, 0.5875f, 0.425f, 1.0f, 0.575f, 0.4375f, 1.0f, 0.5625f, 0.45f, 1.0f, 0.55f, 0.4625f, 1.0f, 0.5375f, 0.475f, 1.0f, 0.525f, 0.4875f, 1.0f, 0.5125f, 0.5f, 1.0f, 0.5f, 0.5125f, 1.0f, 0.4875f, 0.525f, 1.0f, 0.475f, 0.5375f, 1.0f, 0.4625f, 0.55f, 1.0f, 0.45f, 0.5625f, 1.0f, 0.4375f, 0.575f, 1.0f, 0.425f, 0.5875f, 1.0f, 0.4125f, 0.6f, 1.0f, 0.4f, 0.6125f, 1.0f, 0.3875f, 0.625f, 1.0f, 0.375f, 0.6375f, 1.0f, 0.3625f, 0.65f, 1.0f, 0.35f, 0.6625f, 1.0f, 0.3375f, 0.675f, 1.0f, 0.325f, 0.6875f, 1.0f, 0.3125f, 0.7f, 1.0f, 0.3f, 0.7125f, 1.0f, 0.2875f, 0.725f, 1.0f, 0.275f, 0.7375f, 1.0f, 0.2625f, 0.75f, 1.0f, 0.25f, 0.7625f, 1.0f, 0.2375f, 0.775f, 1.0f, 0.225f, 0.7875f, 1.0f, 0.2125f, 0.8f, 1.0f, 0.2f, 0.8125f, 1.0f, 0.1875f, 0.825f, 1.0f, 0.175f, 0.8375f, 1.0f, 0.1625f, 0.85f, 1.0f, 0.15f, 0.8625f, 1.0f, 0.1375f, 0.875f, 1.0f, 0.125f, 0.8875f, 1.0f, 0.1125f, 0.9f, 1.0f, 0.1f, 0.9125f, 1.0f, 0.0875f, 0.925f, 1.0f, 0.075f, 0.9375f, 1.0f, 0.0625f, 0.95f, 1.0f, 0.05f, 0.9625f, 1.0f, 0.0375f, 0.975f, 1.0f, 0.025f, 0.9875f, 1.0f, 0.0125f, 1.0f, 1.0f, 0.0f, 1.0f, 0.9875f, 0.0f, 1.0f, 0.975f, 0.0f, 1.0f, 0.9625f, 0.0f, 1.0f, 0.95f, 0.0f, 1.0f, 0.9375f, 0.0f, 1.0f, 0.925f, 0.0f, 1.0f, 0.9125f, 0.0f, 1.0f, 0.9f, 0.0f, 1.0f, 0.8875f, 0.0f, 1.0f, 0.875f, 0.0f, 1.0f, 0.8625f, 0.0f, 1.0f, 0.85f, 0.0f, 1.0f, 0.8375f, 0.0f, 1.0f, 0.825f, 0.0f, 1.0f, 0.8125f, 0.0f, 1.0f, 0.8f, 0.0f, 1.0f, 0.7875f, 0.0f, 1.0f, 0.775f, 0.0f, 1.0f, 0.7625f, 0.0f, 1.0f, 0.75f, 0.0f, 1.0f, 0.7375f, 0.0f, 1.0f, 0.725f, 0.0f, 1.0f, 0.7125f, 0.0f, 1.0f, 0.7f, 0.0f, 1.0f, 0.6875f, 0.0f, 1.0f, 0.675f, 0.0f, 1.0f, 0.6625f, 0.0f, 1.0f, 0.65f, 0.0f, 1.0f, 0.6375f, 0.0f, 1.0f, 0.625f, 0.0f, 1.0f, 0.6125f, 0.0f, 1.0f, 0.6f, 0.0f, 1.0f, 0.5875f, 0.0f, 1.0f, 0.575f, 0.0f, 1.0f, 0.5625f, 0.0f, 1.0f, 0.55f, 0.0f, 1.0f, 0.5375f, 0.0f, 1.0f, 0.525f, 0.0f, 1.0f, 0.5125f, 0.0f, 1.0f, 0.5f, 0.0f, 1.0f, 0.4875f, 0.0f, 1.0f, 0.475f, 0.0f, 1.0f, 0.4625f, 0.0f, 1.0f, 0.45f, 0.0f, 1.0f, 0.4375f, 0.0f, 1.0f, 0.425f, 0.0f, 1.0f, 0.4125f, 0.0f, 1.0f, 0.4f, 0.0f, 1.0f, 0.3875f, 0.0f, 1.0f, 0.375f, 0.0f, 1.0f, 0.3625f, 0.0f, 1.0f, 0.35f, 0.0f, 1.0f, 0.3375f, 0.0f, 1.0f, 0.325f, 0.0f, 1.0f, 0.3125f, 0.0f, 1.0f, 0.3f, 0.0f, 1.0f, 0.2875f, 0.0f, 1.0f, 0.275f, 0.0f, 1.0f, 0.2625f, 0.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.2375f, 0.0f, 1.0f, 0.225f, 0.0f, 1.0f, 0.2125f, 0.0f, 1.0f, 0.2f, 0.0f, 1.0f, 0.1875f, 0.0f, 1.0f, 0.175f, 0.0f, 1.0f, 0.1625f, 0.0f, 1.0f, 0.15f, 0.0f, 1.0f, 0.1375f, 0.0f, 1.0f, 0.125f, 0.0f, 1.0f, 0.1125f, 0.0f, 1.0f, 0.1f, 0.0f, 1.0f, 0.0875f, 0.0f, 1.0f, 0.075f, 0.0f, 1.0f, 0.0625f, 0.0f, 1.0f, 0.05f, 0.0f, 1.0f, 0.0375f, 0.0f, 1.0f, 0.025f, 0.0f, 1.0f, 0.0125f, 0.0f, 1.0f, 0.0f, 0.0f, 0.9875f, 0.0f, 0.0f, 0.975f, 0.0f, 0.0f, 0.9625f, 0.0f, 0.0f, 0.95f, 0.0f, 0.0f, 0.9375f, 0.0f, 0.0f, 0.925f, 0.0f, 0.0f, 0.9125f, 0.0f, 0.0f, 0.9f, 0.0f, 0.0f };
        private static readonly object _dflimDiagnosticsDataLock = new object();
        private static readonly object _dflimHistogramDataLock = new object();
        private static readonly BitmapPalette _grayscaleLUT = BuildPaletteGrayscale();
        private static readonly object _histogramDataLock = new object();

        private static double[] _blackPoint;
        private static int _colorChannels;
        private static ushort[] _dataBuffer = null;
        private static List<int> _dataBufferOffsetIndex;
        private static int _dataHeight;
        private static int _dataLength;
        private static int _dataWidth;
        private static uint[] _dflimArrivalTimeSumData;
        private static int _dflimDiagnosticsBufferLength = 128;
        private static ushort[][] _dflimDiagnosticsData;
        private static uint[][] _dflimHistogramData;
        private static bool _dflimNewHistogramData;
        private static ushort[] _dflimSinglePhotonData;
        private static bool _grayscaleForSingleChannel = false;
        private static ReportNewImage _imageCallBack;
        private static int _imageProcessImageheight;
        private static int _imageProcessImageWidth;
        private static short[] _imageProcessLabelImageData;
        private static short[] _imageProcessPixelData;
        private static byte[] _imageProcessPixelDataByte;
        private static bool[] _lsmChannelEnable;
        private static int _lsmChannelOrtho;
        private static bool _newDFLIMDiagnosticsData;
        private static byte[][] _pal;
        private static bool _paletteChanged;
        private static ushort[] _pixelData;
        private static byte[] _pixelDataByte;
        private static int[][] _pixelDataHistogram;
        private static byte[][] _pixelDataLUT;
        private static bool _pixelDataReady;
        private static bool _pixelImageProcessDataReady;
        private static byte[] _pixelRoiDataLUT;
        private static byte[][] _rawImg = new byte[MAX_CHANNELS + 1][];
        private static ReportImageProcessData _reportImageProcessData;
        private static int _shiftValue = 6;
        private static double[] _whitePoint;

        private int _autoROIDisplayChannel = 0;
        private IntPtr _imageData;
        private int _maxRoiNum;
        private bool _minAreaActive;
        private int _minAreaValue;
        private int _minSnr;
        private int _rollOverPointX;
        private int _rollOverPointY;

        #endregion Fields

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportImageProcessData(IntPtr returnArray, IntPtr returnRawArray, ref int width, ref int height, ref int colorChannels, ref int numOfROIs);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportNewImage(IntPtr returnArray, ref FrameInfoStruct imageInfo);

        #endregion Delegates

        #region Events

        public event Action<bool> UpdateImage;

        #endregion Events

        #region Properties

        public static Color[][] ChannelLuts
        {
            get;
            set;
        }

        public static bool GrayscaleForSingleChannel
        {
            get
            {
                return _grayscaleForSingleChannel;
            }
            set
            {
                _grayscaleForSingleChannel = value;
            }
        }

        public static FrameInfoStruct ImageInfo
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

        public static int PixelBitShiftValue
        {
            get
            {
                return (GetBitsPerPixel() - 8);
            }
        }

        public int AutoROIDisplayChannel
        {
            get
            {
                return _autoROIDisplayChannel;
            }
            set
            {
                _autoROIDisplayChannel = value;
                AutoTrackingEnable(_autoROIDisplayChannel);
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
                    return LUT_MAX;
                }
            }
            set
            {
                if (_blackPoint.Length > 0)
                {
                    _blackPoint[0] = Math.Min(LUT_MAX, Math.Max(0, value));
                    _paletteChanged = true;
                }
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
                    return LUT_MAX;
                }
            }
            set
            {
                if (_blackPoint.Length > 1)
                {
                    _blackPoint[1] = Math.Min(LUT_MAX, Math.Max(0, value));
                    _paletteChanged = true;
                }
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
                    return LUT_MAX;
                }
            }
            set
            {
                if (_blackPoint.Length > 2)
                {
                    _blackPoint[2] = Math.Min(LUT_MAX, Math.Max(0, value));
                    _paletteChanged = true;
                }
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
                    return LUT_MAX;
                }
            }
            set
            {
                if (_blackPoint.Length > 3)
                {
                    _blackPoint[3] = Math.Min(LUT_MAX, Math.Max(0, value));
                    _paletteChanged = true;
                }
            }
        }

        public List<int> DataBufferOffsetIndex
        {
            get
            {
                return _dataBufferOffsetIndex;
            }
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

        public int DFLIMDiagnosticsBufferLength
        {
            get
            {
                return _dflimDiagnosticsBufferLength;
            }
            set
            {
                _dflimDiagnosticsBufferLength = value;
            }
        }

        public ushort[][] DFLIMDiagnosticsData
        {
            get
            {
                return _dflimDiagnosticsData;
            }
        }

        public object DFLIMDiagnosticsDataLock
        {
            get
            {
                return _dflimDiagnosticsDataLock;
            }
        }

        public uint[][] DFLIMHistogramData
        {
            get
            {
                return _dflimHistogramData;
            }
        }

        public object DFLIMHistogramDataLock
        {
            get
            {
                return _dflimHistogramDataLock;
            }
        }

        public bool DFLIMNewHistogramData
        {
            get
            {
                return _dflimNewHistogramData;
            }
            set
            {
                _dflimNewHistogramData = value;
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

        public object HistogramDataLock
        {
            get
            {
                return _histogramDataLock;
            }
        }

        public string ImageNameFormat
        {
            get
            {
                int imgIndxDigiCnts = (int)Constants.DEFAULT_FILE_FORMAT_DIGITS;
                if (!Directory.Exists(ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "ZStackCache"))
                {
                    XmlNode node = ThorSharedTypes.MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS].SelectSingleNode("/ApplicationSettings/ImageNameFormat");
                    string str = string.Empty;
                    XmlManager.GetAttribute(node, ThorSharedTypes.MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS], "indexDigitCounts", ref str);
                    Int32.TryParse(str, out imgIndxDigiCnts);
                }
                else
                {
                    foreach (string fn in Directory.GetFiles(ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "ZStackCache", "*_*_*", SearchOption.TopDirectoryOnly).Where(file => file.ToLower().EndsWith("raw") || file.ToLower().EndsWith("tif")))
                    {
                        string n = fn.Substring(fn.LastIndexOf('\\') + 1);
                        string str = (n.Split('_'))[1];
                        int num;
                        if (Int32.TryParse(str, out num))
                        {
                            imgIndxDigiCnts = str.Length;
                            break;
                        }
                    }
                }
                return ("D" + imgIndxDigiCnts.ToString());
            }
        }

        public bool IsProcessImageReady
        {
            get
            {
                return _pixelImageProcessDataReady;
            }
            set
            {
                _pixelImageProcessDataReady = value;
            }
        }

        public bool IsSingleChannel
        {
            get
            {
                return (1 == _lsmChannelOrtho || 2 == _lsmChannelOrtho || 4 == _lsmChannelOrtho || 8 == _lsmChannelOrtho);
            }
        }

        public int LSMChannel
        {
            get
            {

                int val = 0;
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CHANNEL, ref val);

                if (DigitizerBoardName == DigitizerBoardNames.ATS9440)
                {
                    switch (val)
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
                    switch (val)
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
                int val = 0;

                if (DigitizerBoardName == DigitizerBoardNames.ATS9440)
                {
                    //convert index value to bitwise value
                    switch (value)
                    {
                        case 0: val = 1; break;
                        case 1: val = 2; break;
                        case 2: val = 4; break;
                        case 3: val = 8; break;
                        case 4:
                            {
                                //_liParams.lsmChannel.val = (Convert.ToInt32(LSMChannelEnable[0]) | (Convert.ToInt32(LSMChannelEnable[1]) << 1) | (Convert.ToInt32(LSMChannelEnable[2]) << 2) | (Convert.ToInt32(LSMChannelEnable[3]) << 3));
                                val = 0xf;   // capture data from all channels under color mode
                            }
                            break;
                    }
                }
                else if (DigitizerBoardName == DigitizerBoardNames.ATS460)
                {
                    //convert index value to bitwise value
                    switch (value)
                    {
                        case 0: val = 1; break;
                        case 1: val = 2; break;
                        default:
                            {
                                val = (Convert.ToInt32(LSMChannelEnable0) | (Convert.ToInt32(LSMChannelEnable1) << 1));
                            }
                            break;
                    }
                }
                else
                {
                    MessageBox.Show("Error in Model.LiveImage.DigitizerBoardName setter", "Unrecognized Boardname");
                }

                SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CLOCKSOURCE, val);
            }
        }

        public bool[] LSMChannelEnable
        {
            get
            {
                return _lsmChannelEnable;
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
                if (_lsmChannelEnable[0] != value)
                {
                    _lsmChannelEnable[0] = value;
                    _paletteChanged = true;
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
                    _paletteChanged = true;
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
                    _paletteChanged = true;
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
                    _paletteChanged = true;
                    SetChannelFromEnable();
                }
            }
        }

        public int LSMChannelOrtho
        {
            get
            {
                return _lsmChannelOrtho;
            }
            set
            {
                _lsmChannelOrtho = value;
            }
        }

        public int MaxRoiNum
        {
            get
            {
                return this._maxRoiNum;
            }
            set
            {
                this._maxRoiNum = value;
                ImgProGenConf(_maxRoiNum, _minSnr);
            }
        }

        public bool MinAreaActive
        {
            get
            {
                return this._minAreaActive;
            }
            set
            {
                this._minAreaActive = value;
                EnableMinAreaFilter(_minAreaActive, _minAreaValue);
            }
        }

        public int MinAreaValue
        {
            get
            {
                return this._minAreaValue;
            }
            set
            {
                this._minAreaValue = value;
                EnableMinAreaFilter(_minAreaActive, _minAreaValue);
            }
        }

        public int MinSnr
        {
            get
            {
                return this._minSnr;
            }
            set
            {
                this._minSnr = value;
                ImgProGenConf(_maxRoiNum, _minSnr);
            }
        }

        public bool NewDFLIMDiagnosticsData
        {
            get
            {
                return _newDFLIMDiagnosticsData;
            }
            set
            {
                _newDFLIMDiagnosticsData = value;
            }
        }

        public ushort[] PixelData
        {
            get
            {
                return _pixelData;
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
                    int location = (int)(_rollOverPointX + (width) * _rollOverPointY);

                    if ((_rollOverPointX < _dataWidth) && (_rollOverPointY < _dataHeight) && (location >= 0) && (location < _pixelData.Length))
                    {
                        int val = (_pixelData[location]);
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
                        int val = (_pixelData[location + _dataLength]);
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
                        int val = (_pixelData[location + 2 * _dataLength]);
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
                        int val = (_pixelData[location + 3 * _dataLength]);
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
                    return LUT_MAX;
                }
            }
            set
            {
                if (_whitePoint.Length > 0)
                {
                    _whitePoint[0] = Math.Min(LUT_MAX, Math.Max(0, value));
                    _paletteChanged = true;
                }
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
                    return LUT_MAX;
                }
            }
            set
            {
                if (_whitePoint.Length > 1)
                {
                    _whitePoint[1] = Math.Min(LUT_MAX, Math.Max(0, value));
                    _paletteChanged = true;
                }
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
                    return LUT_MAX;
                }
            }
            set
            {
                if (_whitePoint.Length > 2)
                {
                    _whitePoint[2] = Math.Min(LUT_MAX, Math.Max(0, value));
                    _paletteChanged = true;
                }
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
                    return LUT_MAX;
                }
            }
            set
            {
                if (_whitePoint.Length > 3)
                {
                    _whitePoint[3] = Math.Min(LUT_MAX, Math.Max(0, value));
                    _paletteChanged = true;
                }
            }
        }

        #endregion Properties

        #region Methods

        public static void CreatePixelDataByteDFLIM(int chanIndex)
        {
            bool resetPixelDataHistogram = (1 == ImageInfo.fullFrame) ? true : false;

            int idx;

            for (int k = 0; k < MAX_CHANNELS; k++)
            {
                if (_rawImg[k] == null || _rawImg[k].Length != 3 * _dataLength)
                {
                    _rawImg[k] = new byte[3 * _dataLength];
                }
            }

            var tau_high_ns = ((CustomCollection<float>)MVMManager.Instance["DFLIMControlViewModel", "DFLIMTauHigh"]);
            var tau_low_ns = ((CustomCollection<float>)MVMManager.Instance["DFLIMControlViewModel", "DFLIMTauLow"]);
            var lut_high_bins = ((CustomCollection<uint>)MVMManager.Instance["DFLIMControlViewModel", "DFLIMLUTHigh"]);
            var lut_low_bins = ((CustomCollection<uint>)MVMManager.Instance["DFLIMControlViewModel", "DFLIMLUTLow"]);
            var tZero_ns = ((CustomCollection<float>)MVMManager.Instance["DFLIMControlViewModel", "DFLIMTZero"]);
            for (int k = 0; k < MAX_CHANNELS; k++)
            {
                double binDuration = 1;

                if (_lsmChannelEnable[k])
                {
                    int histoIndex = (_colorChannels > 1) ? k : 0;
                    lock (_dflimHistogramDataLock)
                    {
                        if (null == _dflimHistogramData || _dflimHistogramData[histoIndex].Length < 256 ||
                            0 == _dflimHistogramData[histoIndex][254] || 0 == _dflimHistogramData[histoIndex][255])
                        {
                            continue;
                        }
                        binDuration = 5.0 * (double)_dflimHistogramData[histoIndex][255] / 128.0 / (double)_dflimHistogramData[histoIndex][254];
                    }

                    double tau_high = tau_high_ns[k] / binDuration;
                    double tau_low = tau_low_ns[k] / binDuration;
                    double lut_high = lut_high_bins[k];
                    double lut_low = lut_low_bins[k];
                    double tZeroBins = tZero_ns[k] / binDuration;

                    int num_total = _dataLength;
                    int averageMode = (int)MVMManager.Instance["ScanControlViewModel", "LSMSignalAverage", (object)0];
                    int averageFrames = 0 == averageMode ? 1 : (int)MVMManager.Instance["ScanControlViewModel", "LSMSignalAverageFrames", (object)0];
                    double tauscale = (tau_high == tau_low) ? 1.0f : (1.0f / (tau_high - tau_low));
                    double brightness;
                    double tau_scaled;
                    int chan = _colorChannels > 1 ? k : 0;

                    //Build the LifeTime Image
                    for (int pix = 0; pix < num_total; pix++)
                    {
                        try
                        {
                            int q = pix;
                            int n = q * 3;

                            //brightness = ((double)_dflimSinglePhotonData[q + chan * num_total] / (double)averageFrames - lut_low) / (lut_high - lut_low);
                            //to get the brigtness right from the beginning, especially when averaging, we will use the intensity buffer
                            //instead of the single photon buffer.
                            const double INTENSITY_SCALE = 128.0; // TODO: this should be 1 once the intensity histogram can handle smaller intensity ranges
                            brightness = (((double)_pixelData[q + chan * num_total] / INTENSITY_SCALE) - lut_low) / (lut_high - lut_low);
                            brightness = (brightness < 0) ? 0 : ((brightness > 1) ? 1 : brightness);

                            if (_dflimSinglePhotonData[q + chan * num_total] != 0)
                            {
                                tau_scaled = tauscale * ((double)_dflimArrivalTimeSumData[q + chan * num_total] / _dflimSinglePhotonData[q + chan * num_total] - tau_low - tZeroBins);
                            }
                            else
                            {
                                tau_scaled = 0;
                            }

                            idx = (tau_scaled < 0) ? 0 : ((tau_scaled > 1) ? 255 : (int)(tau_scaled * 255 + 0.5));

                            byte red = (byte)(brightness * 255 * _dflimColorMap[3 * idx]);
                            byte green = (byte)(brightness * 255 * _dflimColorMap[3 * idx + 1]);
                            byte blue = (byte)(brightness * 255 * _dflimColorMap[3 * idx + 2]);

                            _rawImg[k][n] = red;
                            _rawImg[k][n + 1] = green;
                            _rawImg[k][n + 2] = blue;
                        }
                        catch (Exception ex)
                        {
                            ex.ToString();
                        }
                    }
                }
            }

            ////TODO: fix the parallel logic, for some reason it doesn't work correctly for the dflim image
            ////Array.Clear(_pixelDataByte, 0, 3 * _dataLength);
            //for (int k = 0; k < MAX_CHANNELS; k++)
            //{
            //    if (_lsmChannelEnable[k])
            //    {
            //        double binDuration = 1;
            //        int histoIndex = (_colorChannels > 1) ? k : 0;
            //        lock (_dflimHistogramDataLock)
            //        {
            //            if (null == _dflimHistogramData || _dflimHistogramData[histoIndex].Length < 256 ||
            //                0 == _dflimHistogramData[histoIndex][254] || 0 == _dflimHistogramData[histoIndex][255])
            //            {
            //                continue;
            //            }
            //            binDuration = 5.0 * (double)_dflimHistogramData[histoIndex][255] / 128.0 / (double)_dflimHistogramData[histoIndex][254];
            //        }

            //        double tau_high = tau_high_ns[k] / binDuration;
            //        double tau_low = tau_low_ns[k] / binDuration;
            //        double lut_high = lut_high_bins[k];
            //        double lut_low = lut_low_bins[k];
            //        double tZeroBins = tZero_ns[k] / binDuration;

            //        int num_total = _dataLength;
            //        int averageMode = (int)MVMManager.Instance["ScanControlViewModel", "LSMSignalAverage", (object)0];
            //        int averageFrames = 0 == averageMode ? 1 : (int)MVMManager.Instance["ScanControlViewModel", "LSMSignalAverageFrames", (object)0];
            //        double tauscale = (tau_high == tau_low) ? 1.0f : (1.0f / (tau_high - tau_low));
            //        double brightness;
            //        double tau_scaled;
            //        int chan = _colorChannels > 1 ? k : 0;

            //        var rangePartitioner = Partitioner.Create(0, _dataLength, (_dataLength >> 2) + 1);
            //        Parallel.ForEach
            //            (rangePartitioner, new ParallelOptions { MaxDegreeOfParallelism = 2 }, range =>
            //            {
            //                for (uint p = (uint)range.Item1; p < (uint)range.Item2; p++)
            //                {
            //                    try
            //                    {
            //                        uint q = p;
            //                        uint n = p * 3;

            //                        //brightness = ((double)_dflimSinglePhotonData[q + chan * num_total] / (double)averageFrames - lut_low) / (lut_high - lut_low);
            //                        //to get the brigtness right from the beginning, especially when averaging, we will use the intensity buffer
            //                        //instead of the single photon buffer.
            //                        const double INTENSITY_SCALE = 128.0; // TODO: this should be 1 once the intensity histogram can handle smaller intensity ranges
            //                        brightness = (((double)_pixelData[p + chan * num_total] / INTENSITY_SCALE) - lut_low) / (lut_high - lut_low);
            //                        brightness = (brightness < 0) ? 0 : ((brightness > 1) ? 1 : brightness);

            //                        if (_dflimSinglePhotonData[p + chan * num_total] != 0)
            //                        {
            //                            tau_scaled = tauscale * ((double)_dflimArrivalTimeSumData[p + chan * num_total] / _dflimSinglePhotonData[p + chan * num_total] - tau_low - tZeroBins);
            //                        }
            //                        else
            //                        {
            //                            tau_scaled = 0;
            //                        }

            //                        idx = (tau_scaled < 0) ? 0 : ((tau_scaled > 1) ? 255 : (int)(tau_scaled * 255 + 0.5));

            //                        byte red = (byte)(brightness * 255 * _dflimColorMap[3 * idx]);
            //                        byte green = (byte)(brightness * 255 * _dflimColorMap[3 * idx + 1]);
            //                        byte blue = (byte)(brightness * 255 * _dflimColorMap[3 * idx + 2]);

            //                        _rawImg[k][n] = red;
            //                        _rawImg[k][n + 1] = green;
            //                        _rawImg[k][n + 2] = blue;
            //                    }
            //                    catch (Exception ex)
            //                    {
            //                        ex.ToString();
            //                    }

            //                }
            //            }
            //        );
            //    }
            //}
        }

        public static void FinishedCopyingPixel()
        {
            _pixelDataReady = false;
        }

        public static int GetBitsPerPixel()
        {
            int camType = ResourceManagerCS.GetCameraType();
            int val = DEFAULT_BITS_PER_PIXEL;
            if (((int)ICamera.CameraType.LSM != camType) && ((int)ICamera.CameraType.LAST_CAMERA_TYPE) != camType)
            {
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BITS_PER_PIXEL, ref val);
            }
            return val;
        }

        public static int GetColorChannels()
        {
            return _colorChannels;
        }

        public static ushort[] GetPixelData()
        {
            return _pixelData;
        }

        public static byte[] GetPixelDataByteEx(bool doColor, int channelIndex)
        {
            if (null == _rawImg || null == _pixelDataByte)
            {
                return null;
            }

            if (!doColor)
            {
                return _rawImg[channelIndex];
            }

            bool dflimDisplayLifetimeImage = (bool)MVMManager.Instance["DFLIMControlViewModel", "DFLIMDisplayLifetimeImage", (object)false];

            IntPtr refChannIntPtr;
            ushort[] refChannShortArray = null;

            //no reset of histogram in partial frame
            bool resetPixelDataHistogram = (1 == ImageInfo.fullFrame) ? true : false;

            //need to rebuid the color image because a palette option is not available for RGB images
            if ((_colorChannels > 0) && (_pixelData != null) && (_dataLength * _colorChannels == _pixelData.Length))
            {
                for (int k = 0; k < MAX_CHANNELS; k++)
                {
                    if (_rawImg[k] == null || _rawImg[k].Length != 3 * _dataLength)
                    {
                        _rawImg[k] = new byte[3 * _dataLength];
                    }

                }
                Array.Clear(_pixelDataByte, 0, 3 * _dataLength);

                if (true == _paletteChanged)
                {
                    BuildNormalizationPalettes();
                    _paletteChanged = false;
                }

                // load reference channel
                bool refToRefChann = false;

                if (1 == (int)MVMManager.Instance["AreaControlViewModel", "EnableReferenceChannel", (object)0])
                {
                    string refChannDir = Application.Current.Resources["AppRootFolder"].ToString() + "\\ReferenceChannel.tif";
                    if (File.Exists(refChannDir))   // ref channel file existance
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
                                        refChannShortArray = new ushort[Convert.ToInt32(width) * Convert.ToInt32(height)];
                                        MemoryCopyManager.CopyIntPtrMemory(refChannIntPtr, refChannShortArray, 0, Convert.ToInt32(width) * Convert.ToInt32(height));
                                        refToRefChann = true;
                                    }
                                    catch (Exception e)
                                    {
                                        ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, e.Message);
                                    }
                                }
                                Marshal.FreeHGlobal(refChannIntPtr);
                            }
                        }
                    }
                }

                int shiftValue = GetBitsPerPixel() - 8;

                //in the interest of speed we are seperating the reference channel case
                //without a reference channel the logic will run faster since the
                //conditionals per pixel are removed.
                if (refToRefChann)
                {
                    //factor is derived by taking CAMERA_MAX_INTENSITY_VALUE/256
                    //const double FULL_RANGE_NORMALIZATION_FACTOR = 64.0;
                    ushort[] rawArray = _pixelData;
                    for (int k = 0; k < MAX_CHANNELS; k++)
                    {
                        if (_lsmChannelEnable[k])
                        {
                            if (resetPixelDataHistogram)
                            {
                                Array.Clear(_pixelDataHistogram[k], 0, _pixelDataHistogram[k].Length);
                            }
                            int n = k;
                            if (3 == k)
                            {
                                rawArray = refChannShortArray;
                                n = 0;
                            }
                            for (int i = 0, j = 0; j < _dataLength; i += 3, j++)
                            {
                                ushort valRaw;
                                Color col;
                                //when the reference channel option is on. do not copy the data from channel 3
                                valRaw = rawArray[j + n * _dataLength];
                                col = ChannelLuts[k][_pal[k][valRaw]];
                                _rawImg[k][i] = col.R;
                                _rawImg[k][i + 1] = col.G;
                                _rawImg[k][i + 2] = col.B;
                                if (_pixelDataByte[i] < col.R) _pixelDataByte[i] = col.R;
                                if (_pixelDataByte[i + 1] < col.G) _pixelDataByte[i + 1] = col.G;
                                if (_pixelDataByte[i + 2] < col.B) _pixelDataByte[i + 2] = col.B;

                                //only build the histogram if the color mode is selected when full frame is ready.
                                //This will allow histograms for all of the channels to be available simultaneously
                                if (resetPixelDataHistogram)
                                {
                                    byte valRawHist = (byte)(valRaw >> shiftValue);
                                    _pixelDataHistogram[k][valRawHist]++;
                                }
                            }
                        }
                    } // k
                }
                else
                {
                    //factor is derived by taking CAMERA_MAX_INTENSITY_VALUE/256
                    //const double FULL_RANGE_NORMALIZATION_FACTOR = 64.0;
                    try
                    {
                        for (int k = 0; k < MAX_CHANNELS; k++)
                        {
                            if (_lsmChannelEnable[k])
                            {
                                int chan = _colorChannels > 1 ? k : 0;
                                if (resetPixelDataHistogram)
                                {
                                    Array.Clear(_pixelDataHistogram[k], 0, _pixelDataHistogram[k].Length);
                                }
                                var rangePartitioner = Partitioner.Create(0, _dataLength, (_dataLength >> 2) + 1);
                                Parallel.ForEach
                                    (rangePartitioner, new ParallelOptions { MaxDegreeOfParallelism = 4 }, range =>
                                         {
                                             for (int q = range.Item1; q < range.Item2; q++)
                                             {
                                                 int n = q * 3;

                                                 //[TO DO] find out why bad buffer could occur due to latency.
                                                 ushort valRaw = _pixelData[q + chan * _dataLength];
                                                 if (0 > valRaw)
                                                 {
                                                     _rawImg[k][n] = _rawImg[k][n + 1] = _rawImg[k][n + 2] = 0;
                                                     _pixelDataByte[n] = _pixelDataByte[n + 1] = _pixelDataByte[n + 2] = 0;
                                                 }
                                                 else
                                                 {

                                                     // Color col = ChannelLuts[k][_pal[k][valRaw]];
                                                     Color col = 1 == _colorChannels && true == _grayscaleForSingleChannel ? _grayscaleLUT.Colors[_pal[k][valRaw]] : ChannelLuts[k][_pal[k][valRaw]];
                                                     _rawImg[k][n] = col.R;
                                                     _rawImg[k][n + 1] = col.G;
                                                     _rawImg[k][n + 2] = col.B;
                                                     if (_pixelDataByte[n] < col.R) _pixelDataByte[n] = col.R;
                                                     if (_pixelDataByte[n + 1] < col.G) _pixelDataByte[n + 1] = col.G;
                                                     if (_pixelDataByte[n + 2] < col.B) _pixelDataByte[n + 2] = col.B;
                                                 }

                                                 //only build the histogram if the color mode is selected when full frame is ready.
                                                 //This will allow histograms for all of the channels to be available simultaneously
                                                 if (resetPixelDataHistogram)
                                                 {
                                                     byte valRawHist = (byte)(valRaw >> shiftValue);
                                                     _pixelDataHistogram[k][valRawHist]++;
                                                 }
                                             }
                                         }
                                    );
                            }
                        }// k
                    }
                    catch (Exception ex)
                    {
                        ex.ToString();
                    }
                    _rawImg[MAX_CHANNELS] = _pixelDataByte;
                }
            }

            if (((int)ThorSharedTypes.BufferType.DFLIM_IMAGE == ImageInfo.bufferType ||
                (int)ThorSharedTypes.BufferType.DFLIM_ALL == ImageInfo.bufferType) &&
                dflimDisplayLifetimeImage)
            {
                CreatePixelDataByteDFLIM(channelIndex);
            }

            return _pixelDataByte;
        }

        public static byte[] GetPixelImageProcessDataByte(ref int width, ref int height)
        {
            width = _imageProcessImageWidth;
            height = _imageProcessImageheight;
            if (null == _imageProcessPixelData)
            {
                return _imageProcessPixelDataByte;
            }
            //Build the 12/14-bit to 8-bit Lut
            if (null == _pixelRoiDataLUT)
            {
                _pixelRoiDataLUT = new byte[ushort.MaxValue + 1];
                _pixelRoiDataLUT[0] = 15;
                for (int i = 1; i < _pixelRoiDataLUT.Length; i++)
                {
                    _pixelRoiDataLUT[i] = (byte)(i % 15);
                }
            }

            for (int i = 0; i < _imageProcessImageheight * _imageProcessImageWidth; i++)
            {
                if (_imageProcessPixelData[i] <= 0)
                {
                    _imageProcessPixelDataByte[i] = _pixelRoiDataLUT[0];
                }
                else
                {
                    //val = _pixelDataLUT[0][(_imageProcessPixelData[i])];
                    _imageProcessPixelDataByte[i] = _pixelRoiDataLUT[_imageProcessPixelData[i]];
                }
            }
            return _imageProcessPixelDataByte;
        }

        public static bool IsPixelDataReady()
        {
            EnableCopyToExternalBuffer();

            return _pixelDataReady;
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

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadChannelImages")]
        public static extern int ReadChannelImages([MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPWStr)]string[] fileNames, int size, ref IntPtr outputBuffer, int cameraWidth, int cameraHeight);

        public static void SetColorChannels(int channels)
        {
            _colorChannels = channels;
        }

        public short GetImageProcessData(int width, int height)
        {
            if (width > _imageProcessImageWidth || height > _imageProcessImageheight)
            {
                return 0;
            }
            return (_imageProcessLabelImageData[_imageProcessImageWidth * height + width]);
        }

        public void InitializeDataBufferOffset()
        {
            if (_dataBufferOffsetIndex == null)
            {
                _dataBufferOffsetIndex = new List<int>();
            }
            else
            {
                _dataBufferOffsetIndex.Clear();
            }

            for (int i = 0; i < MAX_CHANNELS; i++)
            {
                //if the channgel is enabled store the index and
                //increment the enabled counter index j
                if (((LSMChannelOrtho >> i) & 0x1) > 0)
                {
                    _dataBufferOffsetIndex.Add(i);
                }
            }
        }

        public void InitializePixelDataLut()
        {
            double shiftValueResult = 64;

            //original shift value is determined by the examining camera type
            shiftValueResult = Math.Pow(2, _shiftValue);

            if (null == _pixelDataLUT)
            {
                _pixelDataLUT = new byte[MAX_CHANNELS][];

                for (int m = 0; m < MAX_CHANNELS; m++)
                {
                    _pixelDataLUT[m] = new byte[ushort.MaxValue + 1];
                }
            }

            //Build the 12/14-bit to 8-bit Lut
            for (int m = 0; m < MAX_CHANNELS; m++)
            {
                for (int k = 0; k < _pixelDataLUT[m].Length; k++)
                {
                    double val = (255.0 / (shiftValueResult * (_whitePoint[m] - _blackPoint[m]))) * (k - _blackPoint[m] * shiftValueResult);
                    val = (val >= 0) ? val : 0;
                    val = (val <= 255) ? val : 255;

                    _pixelDataLUT[m][k] = (byte)Math.Round(val);
                }
            }
        }

        public byte[] UpdataPixelDatabyte(ushort[] savePixelData)
        {
            byte[] pixelData = new byte[3] { 0, 0, 0 };
            byte valNormalized;
            for (int k = 0; k < _dataBufferOffsetIndex.Count; k++)
            {
                valNormalized = _pixelDataLUT[_dataBufferOffsetIndex[k]][savePixelData[k]];

                pixelData[0] = Math.Max(pixelData[0], ChannelLuts[_dataBufferOffsetIndex[k]][valNormalized].R);
                pixelData[1] = Math.Max(pixelData[1], ChannelLuts[_dataBufferOffsetIndex[k]][valNormalized].G);
                pixelData[2] = Math.Max(pixelData[2], ChannelLuts[_dataBufferOffsetIndex[k]][valNormalized].B);
            }
            return pixelData;
        }

        public void UpdateChannelData(string[] fileNames, byte enabledChannels = 0, byte channelToRead = 0, int zToRead = 0, int timeToRead = 0, int imageWidth = 0, int imageHeight = 0, int imageDepth = 0, bool rawContainsDisabledChannels = true)
        {
            UpdateExperimentFileFormatChannelData(fileNames, enabledChannels, channelToRead, zToRead, timeToRead, imageWidth, imageHeight, imageDepth, rawContainsDisabledChannels);
        }

        ///// <summary>
        ///// Updates image data in selected channels for experiment file projects
        ///// </summary>
        ///// <param name="fileNames"> Files to read </param>
        ///// <param name="chEnabled"> Bitmask of enabled channels</param>
        ///// <param name="selectedChannel"> Selected channel </param>
        ///// <param name="selectedZ"> The z coordinate of the selected channel </param>
        ///// <param name="selectedTime"> The time coordinate of the selected channel </param>
        ///// <param name="imageWidth"> The total width of the image in pixels </param>
        ///// <param name="imageHeight"> The total height of the image in pixels </param>
        ///// <param name="imageDepth"> The total depth(z) of the image in pixels </param>
        ///// <param name="rawContainsDisabledChannels"> Boolean describing is a raw file is structured with blank data for disabled channels, or excludes
        /////  the data blocks for disabled channels all togeather </param>
        public void UpdateExperimentFileFormatChannelData(string[] fileNames, byte chEnabled = 0, int selectedChannel = 0, int selectedZ = 0, int selectedTime = 0,
            int imageWidth = 0, int imageHeight = 0, int imageDepth = 0, bool rawContainsDisabledChannels = true)
        {
            try
            {
                //=== First non null file in fileNames ===
                int i = 0;
                for (i = 0; i < MAX_CHANNELS; i++)
                {
                    if (fileNames[i] != null)
                    {
                        break;
                    }
                }
                if (i == MAX_CHANNELS) return;

                int width = 0;
                int height = 0;
                int colorChannels = 0;

                //=== Read File ===
                if (File.Exists(fileNames[i]))
                {
                    //=== Image Parameters ===
                    _dataWidth = imageWidth;
                    _dataHeight = imageHeight;
                    _colorChannels = MAX_CHANNELS;

                    CaptureFile imageType = CaptureFile.FILE_TIFF; // Hardcode imageType to TIFF since it is was ZStack is saved as
                    switch (imageType)
                    {
                        //                case CaptureFile.FILE_BIG_TIFF:
                        //                    _imageData = Marshal.AllocHGlobal(_dataWidth * _dataHeight * _colorChannels * 2);
                        //                    RtlZeroMemory(_imageData, _dataWidth * _dataHeight * _colorChannels * 2);
                        //                    if (!_lastFileNames.SequenceEqual(fileNames))
                        //                    {
                        //                        //load OME at first load, sliders were reset
                        //                        int tileCount = 0, chCount = 0, zMaxCount = 0, timeCount = 0, specCount = 0;
                        //                        CaptureSetup.GetImageStoreInfo(fileNames[i], ref tileCount, ref width, ref height, ref chCount, ref zMaxCount, ref timeCount, ref specCount);
                        //                        _lastFileNames = fileNames;
                        //                    }
                        //                    if (CaptureModes.HYPERSPECTRAL == ExperimentData.CaptureMode)
                        //                    {
                        //                        ReadImageStoreData(_imageData, _colorChannels, imageWidth, imageHeight, selectedZ, 0, selectedTime);
                        //                    }
                        //                    else
                        //{
                        //                        ReadImageStoreData(_imageData, _colorChannels, imageWidth, imageHeight, selectedZ, selectedTime, 0);
                        //}
                        //                    break;
                        //                case CaptureFile.FILE_RAW:
                        //                    _imageData = Marshal.AllocHGlobal(_dataWidth * _dataHeight * _colorChannels * 2);
                        //                    RtlZeroMemory(_imageData, _dataWidth * _dataHeight * _colorChannels * 2);

                        //                    LoadImageIntoBufferFromRawFile(_imageData, fileNames[i], chEnabled, selectedChannel, selectedZ, selectedTime, imageWidth, imageHeight, imageDepth, rawContainsDisabledChannels);
                        //                    break;
                        case CaptureFile.FILE_TIFF:
                            ReadImageInfo(fileNames[i], ref width, ref height, ref colorChannels);
                            if ((width > 0) && (height > 0))
                            {
                                // setting the parameters to be used in the View Model
                                _dataWidth = width;
                                _dataHeight = height;

                                _imageData = Marshal.AllocHGlobal(_dataWidth * _dataHeight * _colorChannels * 2);
                                RtlZeroMemory(_imageData, _dataWidth * _dataHeight * _colorChannels * 2);

                                //read the image and output the buffer with image data
                                ReadChannelImages(fileNames, _colorChannels, ref _imageData, _dataWidth, _dataHeight);
                            }
                            break;
                        default:
                            break;
                    }
                    CopyChannelData();
                }
            }
            catch
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + "File not found exception");
            }
            finally
            {
                Marshal.FreeHGlobal(_imageData);
                _imageData = IntPtr.Zero;
            }
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "AutoTrackingEnable")]
        private static extern bool AutoTrackingEnable(int channelIndex);

        private static void BuildNormalizationPalettes()
        {
            int shiftValue = GetBitsPerPixel() - 8;
            double shiftValueResult = Math.Pow(2, shiftValue);

            for (int j = 0; j < MAX_CHANNELS; j++)
            {
                for (int i = 0; i < ushort.MaxValue + 1; i++)
                {
                    double val = (255.0 / (shiftValueResult * (_whitePoint[j] - _blackPoint[j]))) * (i - _blackPoint[j] * shiftValueResult);
                    val = (val >= 0) ? val : 0;
                    val = (val <= 255) ? val : 255;

                    _pal[j][i] = (byte)Math.Round(val);
                }
            }
        }

        private static BitmapPalette BuildPaletteGrayscale()
        {
            Color[] colors = new Color[256];
            for (int i = 0; i < colors.Length; i++)
            {
                double a = 1.0;
                double b = 0;

                double dvalR = (a * i * (Colors.White.R) / 255.0) + b;
                dvalR = Math.Max(dvalR, 0);
                dvalR = Math.Min(dvalR, 255);

                double dvalG = (a * i * (Colors.White.G) / 255.0) + b;
                dvalG = Math.Max(dvalG, 0);
                dvalG = Math.Min(dvalG, 255);

                double dvalB = (a * i * (Colors.White.B) / 255.0) + b;
                dvalB = Math.Max(dvalB, 0);
                dvalB = Math.Min(dvalB, 255);

                //Display Blue/Red at Min/Max pixel value for single channel:
                if (i == 0)
                {
                    dvalB = 255;
                }
                if (i == 255)
                {
                    dvalG = 0;
                    dvalB = 0;
                }
                colors[i] = Color.FromRgb((byte)dvalR, (byte)dvalG, (byte)dvalB);
            }

            return new BitmapPalette(colors);
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "EnableCopyToExternalBuffer")]
        private static extern bool EnableCopyToExternalBuffer();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "EnableMinAreaFilter")]
        private static extern bool EnableMinAreaFilter(bool minAreaActive, int minAreaValue);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetImageDimensions")]
        private static extern bool GetImageDimensions(ref int width, ref int height);

        private static void ImageUpdate(IntPtr returnArray, ref FrameInfoStruct imageInfo)
        {
            if (_prevTickCount == 0)
            {
                _sumTickCount = 0;
            }
            else
            {
                _sumTickCount = _sumTickCount * .75 + (Environment.TickCount - _prevTickCount) * .25;
            }

            //only copy to the buffer when pixel data is not being read
            if (false == _pixelDataReady)
            {
                //_framesPerSecond = 1000.0 / (Environment.TickCount - _prevTickCount);

                int width = 0;
                int height = 0;

                if (false == GetImageDimensions(ref width, ref height))
                {
                    return;
                }

                _dataLength = width * height;

                if ((_pixelData == null) ||
                    (_pixelData.Length != (_dataLength * _colorChannels)) ||
                    (_colorChannels != imageInfo.channels))
                {
                    _colorChannels = imageInfo.channels;
                    _pixelData = new ushort[_dataLength * _colorChannels];
                    _pixelDataByte = new byte[_dataLength * 3];
                }

                //2^n  == FULL_RANGE_NORMALIZATION_FACTOR
                int shiftValue = GetBitsPerPixel() - 8;
                const int DFLIM_HISTOGRAM_BINS = 256;
                //256 items for dflimHistogram + 2 shorts per histogram item
                int totalBufferSize = _dataLength;

                if ((int)ThorSharedTypes.BufferType.DFLIM_IMAGE == imageInfo.bufferType ||
                    (int)ThorSharedTypes.BufferType.DFLIM_ALL == imageInfo.bufferType)
                {
                    //1 datalength for photon num buffer (intensity) (USHORT)
                    //1 datalength for single photon sum buffer (USHORT)
                    //2 datalength for arrival time sum buffer (UINT32)
                    //2 DFLIM_HISTOGRAM_BINS for dflim histogram (UINT32)
                    totalBufferSize = (_dataLength * 4 + DFLIM_HISTOGRAM_BINS * 2);
                }
                else
                {
                    totalBufferSize = _dataLength;
                }

                if (null == _dataBuffer || _dataBuffer.Length != totalBufferSize * _colorChannels)
                {
                    _dataBuffer = new ushort[totalBufferSize * _colorChannels];
                }

                MemoryCopyManager.CopyIntPtrMemory(returnArray, _dataBuffer, 0, totalBufferSize * _colorChannels);

                //DFLIM image type (including singlePhotonData, arrivalTimeSumData, and histogramData buffers)
                if ((int)ThorSharedTypes.BufferType.DFLIM_IMAGE == imageInfo.bufferType ||
                    (int)ThorSharedTypes.BufferType.DFLIM_ALL == imageInfo.bufferType)
                {

                    if ((_dflimSinglePhotonData == null) ||
                        (_dflimSinglePhotonData.Length != (_dataLength * _colorChannels)))
                    {
                        _dflimSinglePhotonData = new ushort[_dataLength * _colorChannels];
                    }
                    if ((_dflimArrivalTimeSumData == null) ||
                        (_dflimArrivalTimeSumData.Length != (_dataLength * _colorChannels)))
                    {
                        _dflimArrivalTimeSumData = new uint[_dataLength * _colorChannels];
                    }

                    const int SHORTS_PER_BIN = 2;
                    //copy out dflim histogram buffer
                    lock (_dflimHistogramDataLock)
                    {
                        if (_dflimHistogramData == null || _dflimHistogramData.Length != _colorChannels)
                        {
                            _dflimHistogramData = new uint[_colorChannels][];
                            for (int i = 0; i < _dflimHistogramData.Length; ++i)
                            {
                                _dflimHistogramData[i] = new uint[DFLIM_HISTOGRAM_BINS];
                            }
                        }

                        for (int i = 0; i < _colorChannels; ++i)
                        {
                            Buffer.BlockCopy(_dataBuffer, i * DFLIM_HISTOGRAM_BINS * sizeof(UInt32), _dflimHistogramData[i], 0, DFLIM_HISTOGRAM_BINS * sizeof(UInt32));
                        }

                        _dflimNewHistogramData = true;
                    }

                    //copy out the pixel data
                    Array.Copy(_dataBuffer, DFLIM_HISTOGRAM_BINS * SHORTS_PER_BIN * _colorChannels, _pixelData, 0, _dataLength * _colorChannels);
                    //copy out dflim single photon data buffer
                    Array.Copy(_dataBuffer, DFLIM_HISTOGRAM_BINS * SHORTS_PER_BIN * _colorChannels + _dataLength * _colorChannels, _dflimSinglePhotonData, 0, _colorChannels * _dataLength);

                    //copy out dflim arrival time sum data buffer
                    Buffer.BlockCopy(_dataBuffer, DFLIM_HISTOGRAM_BINS * sizeof(UInt32) * _colorChannels + _dataLength * sizeof(UInt32) * _colorChannels, _dflimArrivalTimeSumData, 0, _dataLength * sizeof(Int32) * _colorChannels);
                }
                //Dflim diagnostic mode
                else if ((int)ThorSharedTypes.BufferType.DFLIM_DIAGNOSTIC == imageInfo.bufferType)
                {
                    //copy out the pixel data
                    Array.Copy(_dataBuffer, 0, _pixelData, 0, _dataLength * _colorChannels);
                    //when in dflim diagnositc mode we grab the first _dflimDiagnosticsBufferLength pixels
                    //and then plot them
                    if (_dflimDiagnosticsData == null || _dflimDiagnosticsData.Length != _colorChannels)
                    {
                        _dflimDiagnosticsData = new ushort[_colorChannels][];
                        for (int i = 0; i < _dflimDiagnosticsData.Length; ++i)
                        {
                            _dflimDiagnosticsData[i] = new ushort[_dflimDiagnosticsBufferLength];
                        }
                    }
                    else
                    {
                        for (int i = 0; i < _dflimDiagnosticsData.Length; ++i)
                        {
                            if (_dflimDiagnosticsData[i] == null || _dflimDiagnosticsData[i].Length != _dflimDiagnosticsBufferLength)
                            {
                                _dflimDiagnosticsData[i] = new ushort[_dflimDiagnosticsBufferLength];
                            }
                        }
                    }
                    if (_colorChannels > 0)
                    {
                        lock (_dflimDiagnosticsDataLock)
                        {
                            for (int i = 0; i < _colorChannels; ++i)
                            {
                                int middleLineIndex = (height / 2) * width;
                                //copy out the diagnostic section
                                int copyLength = (_dflimDiagnosticsBufferLength <= ((_dataLength / _colorChannels) - middleLineIndex)) ? _dflimDiagnosticsBufferLength : (_dataLength / _colorChannels) - middleLineIndex;
                                Array.Copy(_dataBuffer, i * _dataLength + middleLineIndex, _dflimDiagnosticsData[i], 0, copyLength);
                            }

                            _newDFLIMDiagnosticsData = true;
                        }
                    }
                }
                else
                {
                    //copy out the pixel data
                    Array.Copy(_dataBuffer, 0, _pixelData, 0, _dataLength * _colorChannels);
                }

                //clear the histogram for single channel only when full frame is ready,
                //case of multi-channel is handled with bitmap update for performance concern
                if ((1 == _colorChannels) && (1 == imageInfo.fullFrame))
                {
                    lock (_histogramDataLock)
                    {
                        //factor is derived by taking CAMERA_MAX_INTENSITY_VALUE/256
                        //const double FULL_RANGE_NORMALIZATION_FACTOR = 64.0;
                        Array.Clear(_pixelDataHistogram[0], 0, _pixelDataHistogram[0].Length);

                        for (int i = 0; i < _dataLength * _colorChannels; i++)
                        {
                            double valHist = (_pixelData[i]) >> shiftValue;
                            _pixelDataHistogram[0][(byte)valHist]++;
                        }
                    }
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

            ImageInfo = imageInfo;
            _prevTickCount = Environment.TickCount;
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "ImgProGenConf")]
        private static extern bool ImgProGenConf(int maxRoiNum, int minSnr);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo")]
        private static extern int ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)]string path, ref int width, ref int height, ref int colorChannels);

        [DllImport("kernel32.dll")]
        static extern void RtlZeroMemory(IntPtr dst, int length);

        private void CopyChannelData()
        {
            //only copy to the buffer when pixel data is not being read
            if (_pixelDataReady == false)
            {
                _dataLength = (_dataWidth * _dataHeight);

                if ((_pixelData == null) || (_pixelData.Length != (_dataLength * MAX_CHANNELS)))
                {
                    _pixelData = new ushort[_dataLength * MAX_CHANNELS];
                    _pixelDataByte = new byte[_dataLength * 3];
                    _pixelDataHistogram = new int[MAX_CHANNELS][];

                    for (int i = 0; i < MAX_CHANNELS; i++)
                    {
                        _pixelDataHistogram[i] = new int[PIXEL_DATA_HISTOGRAM_SIZE];
                    }
                }

                MemoryCopyManager.CopyIntPtrMemory(_imageData, _pixelData, 0, _dataLength * MAX_CHANNELS);
                _pixelDataReady = true;
            }
        }

        private void ImageProcessDataUpdata(IntPtr returnArray, IntPtr returnRawArray, ref int width, ref int height, ref int colorChannels, ref int numOfROIs)
        {
            if (IsProcessImageReady == false)
            {

                //int num = numOfROIs;
                _imageProcessImageWidth = width;
                _imageProcessImageheight = height;
                int dataLength = width * height;

                if ((_imageProcessPixelData == null) || (_imageProcessPixelData.Length != dataLength))
                {
                    _imageProcessPixelData = new short[dataLength];
                    _imageProcessPixelDataByte = new byte[dataLength];
                    _imageProcessLabelImageData = new short[dataLength];
                }
                Marshal.Copy(returnArray, _imageProcessPixelData, 0, dataLength);
                Marshal.Copy(returnRawArray, _imageProcessLabelImageData, 0, dataLength);
                IsProcessImageReady = true;
            }
        }

        void Trigger_UpdateImage(bool obj)
        {
            UpdateImage(true);
        }

        #endregion Methods
    }
}