namespace ImageReviewDll.Model
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.CompilerServices;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Media.Media3D;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Serialization;

    using ImageReviewDll.ViewModel;

    using LineProfileWindow;

    using ThorLogging;

    using ThorSharedTypes;

    public class ImageFileNameClass
    {
        #region Fields

        private string _fName = string.Empty;

        #endregion Fields

        #region Constructors

        public ImageFileNameClass(string fName)
        {
            _fName = fName;
            if (_fName != string.Empty)
            {
                string pattern = "(.*)_(.*)_(.*)_(.*)_(.*).tif";
                Regex ex = new Regex(pattern, RegexOptions.IgnoreCase);
                Match match = ex.Match(_fName);
                if (match.Groups.Count >= 5)
                {
                    Wellid = Convert.ToInt32(match.Groups[2].ToString()); // No assigned
                    Tileid = Convert.ToInt32(match.Groups[3].ToString());
                    Zid = Convert.ToInt32(match.Groups[4].ToString());
                    Tid = Convert.ToInt32(match.Groups[5].ToString());
                }
            }
        }

        #endregion Constructors

        #region Properties

        public string FileName
        {
            get { return _fName; }
            set
            {
                _fName = value;
            }
        }

        public int Tid
        {
            get;
            set;
        }

        public int Tileid
        {
            get;
            set;
        }

        public int Wellid
        {
            get;
            set;
        }

        public int Zid
        {
            get;
            set;
        }

        #endregion Properties
    }

    public class ImageReview
    {
        #region Fields

        public const int BITMAP_UPDATE_TIMEOUT = 2000;
        public const int LUT_SIZE = 256;
        public const int MAX_CHANNELS = 4;

        public double[] Stats;
        public string[] StatsNames;

        readonly FrameData _frameData = new FrameData();

        private static ReportLineProfile _lineProfileCallback;
        private static int _shiftValue = 6;

        private List<Tuple<int, int>> SequenceSelectedChannel = new List<Tuple<int, int>>();
        private XmlDocument _applicationDoc;
        int _bitsPerPixel = 14;
        private int _colorChannels;
        bool _correctionParametersSet = false;
        private int _currentSpCount;
        private int _currentTCount;
        private int _currentZCount;
        private int _currentZStreamCount;
        private int _dataLength;
        private XmlDocument _experimentDoc;
        private string _experimentFolderPath;
        private bool _experimentReviewOpened;
        private string _experimentXMLPath;
        private int _foundChannelCount = 0;
        private XmlDocument _hardwareDoc;
        private IntPtr _imageData;
        int _imageDataSize = 0;
        private int _imageHeight;
        private ImgInfo _ImageInfo;
        private int _imageWidth;
        private List<string> _lastFileNames = new List<string>();
        private LineProfileData _lineProfileData;
        private int _lsmChannel;
        private double _LSMImageCalibrationGalvoTiltAngle = 25.0;
        private double _LSMImageCalibrationXAngleMax = 3.5;
        private double _LSMImageCalibrationYAngleMax = 3.0;
        private int _LSMImageDistortionCorrectionEnable = 0;
        private ushort[] _pixelData = null;
        int _sampleSiteIndex = 1;
        private int _sequenceStepChannels = 0;
        private int _sequenceStepIndex = 0;
        private int _sequenceStepsNum = 0;
        int _subTileIndex = 1;
        private int _totalSequentialChannelCount;
        private Color _wavelengthColor;
        private int _wavelengthSelectedIndex;
        private ObservableCollection<XYPosition> _xYtableData = new ObservableCollection<XYPosition>();

        #endregion Fields

        #region Constructors

        public ImageReview()
        {
            _wavelengthColor = Colors.White;

            _lineProfileCallback = new ReportLineProfile(LineProfileUpdate);
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Created");
        }

        #endregion Constructors

        #region Enumerations

        public enum ColorAssignments
        {
            RED,
            GREEN,
            BLUE,
            CYAN,
            MAGENTA,
            YELLOW,
            GRAY,
            WHITE,
            TRANSPARENT
        }

        public enum FormatMode
        {
            EXPERIMENT,
            CUSTOM
        }

        #endregion Enumerations

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate void ReportLineProfile(IntPtr lineProfile, int length, int realLength, int channelEnable, int numChannel);

        #endregion Delegates

        #region Events

        public event EventHandler LineProfileChanged;

        #endregion Events

        #region Properties

        public XmlDocument ApplicationDoc
        {
            get
            {
                return this._applicationDoc;
            }
            set
            {
                this._applicationDoc = value;
            }
        }

        public string ApplicationSettingPath
        {
            get
            {
                return (string.Empty == ExperimentModality) ? ResourceManagerCS.GetApplicationSettingsFileString() :
                    ResourceManagerCS.GetModalityApplicationSettingsFileString(ExperimentModality);
            }
        }

        public int BitsPerPixel
        {
            set
            {
                if ((value > 0) && (value <= 16))
                {
                    _shiftValue = value - 8;
                }
                else
                {
                    _shiftValue = 6;
                }
                _bitsPerPixel = value;
            }
        }

        public byte ChannelEnabled
        {
            get
            {
                string str = string.Empty;
                byte bVal = 0x0;
                if (null != _experimentDoc)
                {
                    XmlNodeList ndList = _experimentDoc.SelectNodes("/ThorImageExperiment/Wavelengths/ChannelEnable");
                    if (0 < ndList.Count)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _experimentDoc, "Set", ref str) && (byte.TryParse(str, out bVal)))
                        {
                            return bVal;
                        }
                    }
                }
                return bVal;
            }
        }

        public XmlDocument ExperimentDoc
        {
            get
            {
                return this._experimentDoc;
            }
            set
            {
                this._experimentDoc = value;
            }
        }

        public string ExperimentFolderPath
        {
            get
            {
                return _experimentFolderPath;
            }
            set
            {
                _experimentFolderPath = value.EndsWith("\\") ? value : (value + "\\");
                SaveLastExperimentInfo(_experimentFolderPath);
            }
        }

        public string ExperimentModality
        {
            get
            {
                string str = string.Empty;
                if (null != _experimentDoc)
                {
                    XmlNodeList ndList = _experimentDoc.SelectNodes("/ThorImageExperiment/Modality");
                    if (0 < ndList.Count)
                    {
                        XmlManager.GetAttribute(ndList[0], _experimentDoc, "name", ref str);
                    }
                }
                else
                {
                    if (null != MVMManager.Instance && null != MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS])
                    {
                        XmlNode nd = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS].SelectSingleNode("/ThorImageExperiment/Modality");
                        XmlManager.GetAttribute(nd, MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS], "name", ref str);
                    }
                }
                return str;
            }
        }

        //Boolean to tell when Experiment Review window is open
        //Set in ExperimentReview, used in ImageReviewViewModel/OverlayManager
        public bool ExperimentReviewOpened
        {
            get
            {
                return this._experimentReviewOpened;
            }
            set
            {
                this._experimentReviewOpened = value;
            }
        }

        public string ExperimentXMLPath
        {
            get
            {
                if (string.IsNullOrEmpty(_experimentXMLPath))
                    return ResourceManagerCS.GetActiveSettingsFileString();
                return _experimentXMLPath;
            }
            set
            {
                _experimentXMLPath = value;
            }
        }

        public FormatMode FileFormatMode
        {
            get;
            set;
        }

        public FrameData FrameData
        {
            get
            {
                return _frameData;
            }
        }

        public string[] HardwareChannelNames
        {
            get
            {
                if (null == _hardwareDoc)
                {
                    return null;
                }

                string str = string.Empty;

                XmlNodeList ndListHW = _hardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

                if (ndListHW.Count <= 0)
                {
                    return null;
                }

                string[] fileNames = new string[ndListHW.Count];

                for (int k = 0; k < ndListHW.Count; k++)
                {
                    XmlManager.GetAttribute(ndListHW[k], _hardwareDoc, "name", ref str);
                    fileNames[k] = str;
                }

                return fileNames;
            }
        }

        public XmlDocument HardwareDoc
        {
            get
            {
                return this._hardwareDoc;
            }
            set
            {
                this._hardwareDoc = value;

                //// TO DO:
                //The following code was commented by Ming because this code is never connected to VM.
                //it causes index out of bound exception when connected with VM
                /*//update the number of maximum color channels for the system
                XmlNodeList ndListHW = _hardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

                MAX_CHANNELS = ndListHW.Count;

                InitializeClassArrays(MAX_CHANNELS);*/

            }
        }

        public string HardwareSettingPath
        {
            get
            {
                return (string.Empty == ExperimentModality) ? ResourceManagerCS.GetHardwareSettingsFileString() :
                    ResourceManagerCS.GetModalityHardwareSettingsFileString(ExperimentModality);
            }
        }

        public int ImageColorChannels
        {
            get { return _colorChannels; }
            set { _colorChannels = value; }
        }

        public int ImageHeight
        {
            get { return _imageHeight; }
        }

        public ImgInfo ImageInfo
        {
            get { return _ImageInfo; }
            set { _ImageInfo = value; }
        }

        public string ImageNameFormat
        {
            get
            {
                int imgIndxDigiCnts = (int)Constants.DEFAULT_FILE_FORMAT_DIGITS;
                if (_experimentFolderPath == null)
                {
                    if (null == _applicationDoc)
                    {
                        XmlDocument appSettings = new XmlDocument();
                        _applicationDoc.Load(ApplicationSettingPath);
                    }

                    XmlNode node = _applicationDoc.SelectSingleNode("/ApplicationSettings/ImageNameFormat");
                    string str = string.Empty;
                    XmlManager.GetAttribute(node, _applicationDoc, "indexDigitCounts", ref str);
                    Int32.TryParse(str, out imgIndxDigiCnts);
                }
                else
                {
                    foreach (string fn in Directory.GetFiles(_experimentFolderPath, "*_*_*", SearchOption.TopDirectoryOnly).Where(file => file.ToLower().EndsWith("raw") || file.ToLower().EndsWith("tif")))
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

        public int ImageWidth
        {
            get { return _imageWidth; }
        }

        public bool IsSingleChannel
        {
            get
            {
                return (1 == _lsmChannel || 2 == _lsmChannel || 4 == _lsmChannel || 8 == _lsmChannel);
            }
        }

        public LineProfileData LineProfileData
        {
            get
            {
                return _lineProfileData;
            }
        }

        public int LSMChannel
        {
            get
            {
                return _lsmChannel;
            }
            set
            {
                _lsmChannel = value;
                switch (_lsmChannel)
                {
                    case 1: _wavelengthSelectedIndex = 0; break;
                    case 2: _wavelengthSelectedIndex = 1; break;
                    case 4: _wavelengthSelectedIndex = 2; break;
                    case 8: _wavelengthSelectedIndex = 3; break;
                    default: _wavelengthSelectedIndex = 0; break;
                }
            }
        }

        public double LSMImageDistortionCorrectionCalibrationGalvoTiltAngle
        {
            get => _LSMImageCalibrationGalvoTiltAngle;
            internal set
            {
                _LSMImageCalibrationGalvoTiltAngle = value;
                _correctionParametersSet = false;
                if (_frameData?.frameInfo != null)
                {
                    CopyChannelData(_frameData.frameInfo.channels, _frameData.contiguousChannels, _frameData.frameInfo.scanAreaID, _frameData.frameInfo.scanAreaIndex);
                }
            }
        }

        public double LSMImageDistortionCorrectionCalibrationXAngleMax
        {
            get => _LSMImageCalibrationXAngleMax;
            internal set
            {
                _LSMImageCalibrationXAngleMax = value;
                _correctionParametersSet = false;
                if (_frameData?.frameInfo != null)
                {
                    CopyChannelData(_frameData.frameInfo.channels, _frameData.contiguousChannels, _frameData.frameInfo.scanAreaID, _frameData.frameInfo.scanAreaIndex);
                }
            }
        }

        public double LSMImageDistortionCorrectionCalibrationYAngleMax
        {
            get => _LSMImageCalibrationYAngleMax;
            internal set
            {
                _LSMImageCalibrationYAngleMax = value;
                _correctionParametersSet = false;
                if (_frameData?.frameInfo != null)
                {
                    CopyChannelData(_frameData.frameInfo.channels, _frameData.contiguousChannels, _frameData.frameInfo.scanAreaID, _frameData.frameInfo.scanAreaIndex);
                }
            }
        }

        public int LSMImageDistortionCorrectionEnable
        {
            get => _LSMImageDistortionCorrectionEnable;
            internal set
            {
                _LSMImageDistortionCorrectionEnable = value;
                if (_frameData?.frameInfo != null)
                {
                    CopyChannelData(_frameData.frameInfo.channels, _frameData.contiguousChannels, _frameData.frameInfo.scanAreaID, _frameData.frameInfo.scanAreaIndex);
                }
            }
        }

        public int MaxChannels
        {
            get
            {
                return MAX_CHANNELS;
            }
        }

        public int PixelBitShiftValue
        {
            get
            {
                return _shiftValue;
            }
        }

        public int SampleSiteIndex
        {
            get
            {
                return this._sampleSiteIndex;
            }
            set
            {
                this._sampleSiteIndex = value;
            }
        }

        public bool SequentialModeEnabled
        {
            get;
            set;
        }

        public int SpValue
        {
            get
            {
                return _currentSpCount;
            }
            set
            {
                _currentSpCount = value;
            }
        }

        public int SubTileIndex
        {
            get
            {
                return _subTileIndex;
            }
            set
            {
                _subTileIndex = value;
            }
        }

        public int TValue
        {
            get
            {
                return _currentTCount;
            }
            set
            {
                _currentTCount = value;
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

        public int WavelengthSelectedIndex
        {
            get
            {
                return _wavelengthSelectedIndex;
            }
        }

        // We need a new XYTableData structure to send to the XYTileControl project. XYTileControl is expecting to
        // get XYtableData from it's MVM, but since ImageReview doesn't have it's own instance of MVMManager, this is
        // a work around.
        public ObservableCollection<XYPosition> XYtableData
        {
            get { return _xYtableData; }
            set { _xYtableData = value; }
        }

        public int ZStreamValue
        {
            get
            {
                return _currentZStreamCount;
            }
            set
            {
                _currentZStreamCount = value;
            }
        }

        public int ZValue
        {
            get
            {
                return _currentZCount;
            }
            set
            {
                _currentZCount = value;
            }
        }

        #endregion Properties

        #region Methods

        [DllImport("..\\Modules_Native\\ImageStoreLibrary.dll", EntryPoint = "GetImageStoreInfo")]
        public static extern int GetImageStoreInfo([MarshalAs(UnmanagedType.LPStr)] string fileName, int regionID, ref int regionCount, ref int width, ref int height, ref int channelCount, ref int zMaxCount, ref int timeCount, ref int spectCount);

        /// <summary>
        /// Loads an image from a raw file into the destination buffer
        /// </summary>
        /// <param name="destBuffer"> Buffer to load into </param>
        /// <param name="fileName"> Path and filename of the raw file </param>
        /// <param name="enabledChannels"> Bitmask of enabled channels </param>
        /// <param name="channelToRead"> The channel to read </param>
        /// <param name="zToRead"> The z slice to read </param>
        /// <param name="timeToRead"> The time to read </param>
        /// <param name="imageWidth"> The width of an image </param>
        /// <param name="imageHeight"> The height of an image </param>
        /// <param name="imageDepth"> The depth of an image </param>
        /// <param name="rawContainsDisabledChannels"> If the raw file contains placeholders for disabled channels </param>
        public static void LoadImageIntoBufferFromRawFile(IntPtr destBuffer, string fileName, byte enabledChannels, int channelToRead, int zToRead, int timeToRead, int imageWidth, int imageHeight, int imageDepth, bool rawContainsDisabledChannels)
        {
            //=== Read All Channels ===
            bool onlyOneEnabled = (enabledChannels == 1 || enabledChannels == 2 || enabledChannels == 4 || enabledChannels == 8);
            if (channelToRead == 4)
            {

                //Single channel images never contain disabled channels
                if (onlyOneEnabled)
                {
                    //Figure out which one is enabled
                    for (int i = 0; i < MAX_CHANNELS; i++)
                    {
                        if (isChannelEnabled(i, enabledChannels))
                        {
                            //Read it
                            int result = ReadChannelImageRawSlice(destBuffer, fileName, imageWidth, imageHeight, imageDepth, 4, i, zToRead, timeToRead, enabledChannels, false);
                        }
                    }
                }

                //Read all channels
                else
                {
                    for (int ch = 0; ch < channelToRead; ch++)
                    {
                        int result = ReadChannelImageRawSlice(destBuffer, fileName, imageWidth, imageHeight, imageDepth, 4, ch, zToRead, timeToRead, enabledChannels, rawContainsDisabledChannels);
                    }
                }
            }

            //=== Read One Channel ===
            else
            {
                //Single channel images never contain disabled channels
                if (onlyOneEnabled)
                {
                    int result = ReadChannelImageRawSlice(destBuffer, fileName, imageWidth, imageHeight, imageDepth, 4, channelToRead, zToRead, timeToRead, enabledChannels, false);
                }
                else
                {
                    int result = ReadChannelImageRawSlice(destBuffer, fileName, imageWidth, imageHeight, imageDepth, 4, channelToRead, zToRead, timeToRead, enabledChannels, rawContainsDisabledChannels);
                }
            }
        }

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadChannelImageRawSlice")]
        public static extern int ReadChannelImageRawSlice(IntPtr outputBuffer, [MarshalAs(UnmanagedType.LPStr)] string fileName, int width, int height,
            int zDepth, int channels, int loadChannel, int zSlice, int time, int enabledChannelsBitmask, bool containsDisabledChannels);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadChannelImages")]
        public static extern int ReadChannelImages([MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPWStr)] string[] fileNames, int size, ref IntPtr outputBuffer, int cameraWidth, int cameraHeight);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadChannelImagesRaw")]
        public static extern int ReadChannelImagesRaw(ref IntPtr outputBuffer, int bufChs, [MarshalAs(UnmanagedType.LPStr)] string fName, int fileChs, int ChToRead, int frmSize, int blkIndex);

        [DllImport("..\\Modules_Native\\ImageStoreLibrary.dll", EntryPoint = "ReadImageStoreData")]
        public static extern int ReadImageStoreData(IntPtr outputBuffer, int channelCount, int width, int height, int zSliceID, int timeID, int specID, int wID = 0, int regionID = 0);

        [DllImport(".\\StatsManager.dll", EntryPoint = "SetLineProfileLineWidth")]
        public static extern int SetLineProfileLineWidth(int width);

        public void AllocateImageDataMemoryFormROI()
        {
            if (ExperimentData.IsmROICapture)
            {
                int newSize = 0;
                int channels = (int)Math.Max(_colorChannels, MAX_CHANNELS);
                for (int i = 0; i < ExperimentData.mROIs?.Count; ++i)
                {
                    int size = ExperimentData.mROIs[i].SizeXPixels * ExperimentData.mROIs[i].SizeYPixels * 2 * channels;
                    if ((ExperimentData.mROIs[i].SizeXPixels * ExperimentData.mROIs[i].SizeYPixels * 2 * channels) > newSize)
                    {
                        newSize = size;
                    }
                }

                if (_imageData != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(_imageData);
                    _imageData = IntPtr.Zero;
                }

                _imageData = Marshal.AllocHGlobal(_imageWidth * _imageHeight * _colorChannels * 2);
                _imageDataSize = _imageWidth * _imageHeight * _colorChannels * 2;
            }
        }

        public int GetFoundChannels()
        {
            return _foundChannelCount;
        }

        public void LoadLineProfileData()
        {
            if (_pixelData != null)
            {
                bool onlyOneEnabled = (_lsmChannel == 1 || _lsmChannel == 2 || _lsmChannel == 4 || _lsmChannel == 8);
                FrameInfoStruct frameInfo = new FrameInfoStruct();
                frameInfo.bufferType = (int)BufferType.INTENSITY;
                frameInfo.imageWidth = _imageWidth;
                frameInfo.imageHeight = _imageHeight;
                frameInfo.numberOfPlanes = 1;
                if (onlyOneEnabled)
                {
                    int skip = 0;
                    switch (_lsmChannel)
                    {
                        case 1: skip = 0; break;
                        case 2: skip = _imageWidth * _imageHeight; break;
                        case 4: skip = 2 * _imageWidth * _imageHeight; break;
                        case 8: skip = 3 * _imageWidth * _imageHeight; break;
                    }
                    IEnumerable<ushort> pixelDataOneChannel = _pixelData.Skip(skip).Take(_imageWidth * _imageHeight);
                    var ta = pixelDataOneChannel.ToArray();
                    if (ImageReviewViewModel.StatsLoaderDone)
                        ComputeStats(pixelDataOneChannel.ToArray(), frameInfo, _lsmChannel, 1, 0, 0);
                }
                else
                {
                    if (ImageReviewViewModel.StatsLoaderDone)
                        ComputeStats(_pixelData, frameInfo, _lsmChannel, 1, 0, 0);
                }
            }
        }

        //public byte[] GetPixelDataByte()
        //{
        //    if (null != _pixelData) // null can happen when Experiment folder has only Experiment.xml but no image
        //    {
        //        //need to rebuid the color image because a palette option is not available for RGB images
        //        if ((_dataLength * MAX_CHANNELS == _pixelData.Length))
        //        {
        //            int i;
        //            //clear the histogram
        //            for (int k = 0; k < MAX_CHANNELS; k++)
        //            {
        //                Array.Clear(_pixelDataHistogram[k], 0, _pixelDataHistogram[k].Length);
        //            }
        //            //calculate the raw data buffer offset index for each of the
        //            //selected display channels
        //            if (_dataBufferOffsetIndex == null)
        //            {
        //                _dataBufferOffsetIndex = new List<int>();
        //            }
        //            else
        //            {
        //                _dataBufferOffsetIndex.Clear();
        //            }
        //            int j;
        //            for (i = 0; i < MAX_CHANNELS; i++)
        //            {
        //                //if the channgel is enabled store the index and
        //                //increment the enabled counter index j
        //                if (((_lsmChannel >> i) & 0x1) > 0)
        //                {
        //                    _dataBufferOffsetIndex.Add(i);
        //                }
        //            }
        //            double shiftValueResult = 64;
        //            //original shift value is determined by the examining camera type
        //            shiftValueResult = Math.Pow(2, _shiftValue);
        //            if (null == _pixelDataLUT)
        //            {
        //                _pixelDataLUT = new byte[MAX_CHANNELS][];
        //                for (int m = 0; m < MAX_CHANNELS; m++)
        //                {
        //                    _pixelDataLUT[m] = new byte[ushort.MaxValue + 1];
        //                }
        //            }
        //            //Build the 12/14-bit to 8-bit Lut
        //            for (int m = 0; m < MAX_CHANNELS; m++)
        //            {
        //                for (int k = 0; k < _pixelDataLUT[m].Length; k++)
        //                {
        //                    double val = (255.0 / (shiftValueResult * (_whitePoint[m] - _blackPoint[m]))) * (k - _blackPoint[m] * shiftValueResult);
        //                    val = (val >= 0) ? val : 0;
        //                    val = (val <= 255) ? val : 255;
        //                    _pixelDataLUT[m][k] = (byte)Math.Round(val);
        //                }
        //            }
        //            Array.Clear(_pixelDataByte, 0, 3 * _dataLength);
        //            bool useGrayScale = _grayscaleForSingleChannel && (1 == _lsmChannel || 2 == _lsmChannel || 4 == _lsmChannel || 8 == _lsmChannel);
        //            for (i = 0, j = 0; j < _dataLength; i += 3, j++)
        //            {
        //                byte maxRed = 0;
        //                byte maxGreen = 0;
        //                byte maxBlue = 0;
        //                for (int k = 0; k < _dataBufferOffsetIndex.Count; k++)
        //                {
        //                    int kk = _dataBufferOffsetIndex[k];
        //                    ushort plD = _pixelData[j + kk * _dataLength];
        //                    if (plD >= 0)
        //                    {
        //                        byte valRaw = (byte)(plD >> _shiftValue);
        //                        byte valNormalized = _pixelDataLUT[kk][plD];
        //                        Color col = ChannelLuts[kk][valNormalized];
        //                        if (useGrayScale)
        //                        {
        //                            if (0 == valNormalized)
        //                            {
        //                                Color satLowBlue = Colors.Blue;
        //                                _pixelDataByte[i] = maxRed = Math.Max(maxRed, satLowBlue.R);
        //                                _pixelDataByte[i + 1] = maxGreen = Math.Max(maxGreen, satLowBlue.G);
        //                                _pixelDataByte[i + 2] = maxBlue = Math.Max(maxBlue, satLowBlue.B);
        //                            }
        //                            else if (ChannelLuts[kk].Length - 1 == valNormalized)
        //                            {
        //                                Color satHighRed = Colors.Red;
        //                                _pixelDataByte[i] = Math.Max(maxRed, satHighRed.R);
        //                                _pixelDataByte[i + 1] = Math.Max(maxGreen, satHighRed.G);
        //                                _pixelDataByte[i + 2] = Math.Max(maxBlue, satHighRed.B);
        //                            }
        //                            else
        //                            {
        //                                _pixelDataByte[i] = maxRed = Math.Max(maxRed, col.R);
        //                                _pixelDataByte[i + 1] = maxGreen = Math.Max(maxGreen, col.G);
        //                                _pixelDataByte[i + 2] = maxBlue = Math.Max(maxBlue, col.B);
        //                            }
        //                        }
        //                        else
        //                        {
        //                            _pixelDataByte[i] = maxRed = Math.Max(maxRed, col.R);
        //                            _pixelDataByte[i + 1] = maxGreen = Math.Max(maxGreen, col.G);
        //                            _pixelDataByte[i + 2] = maxBlue = Math.Max(maxBlue, col.B);
        //                        }
        //                        _pixelDataHistogram[kk][valRaw]++;
        //                    }
        //                }
        //            }
        //        }
        //        return _pixelDataByte;
        //    }
        //    else // _pixelData == null
        //    {
        //        return null;
        //    }
        //}
        //public short[] GetPixelDataSave()
        //{
        //    if ((null == _pixelDataSave) || (_pixelDataSave.Length != (_dataLength * 3)))
        //    {
        //        _pixelDataSave = new short[_dataLength * 3];
        //    }
        //    for (int k = 0; k < MAX_CHANNELS; k++)
        //    {
        //        if (((_lsmChannel >> k) & 0x1) > 0)
        //        {
        //            Array.Clear(_pixelDataSave, 0, _dataLength * 3);
        //            for (int i = 0, j = 0; j < _dataLength; i += 3, j++)
        //            {
        //                short maxRed = 0;
        //                short maxGreen = 0;
        //                short maxBlue = 0;
        //                ushort valRaw = _pixelData[j + k * _dataLength];
        //                int valRawByte = Math.Min(255, valRaw >> _shiftValue);
        //                Color col = ChannelLuts[k][valRawByte];
        //                double total = ((double)valRaw) / (col.R + col.G + col.B);
        //                _pixelDataSave[i] = maxRed = Math.Max(maxRed, (short)(col.R * total));
        //                _pixelDataSave[i + 1] = maxGreen = Math.Max(maxGreen, (short)(col.G * total));
        //                _pixelDataSave[i + 2] = maxBlue = Math.Max(maxBlue, (short)(col.B * total));
        //            }
        //        }
        //    }
        //    return _pixelDataSave;
        //}
        public void LoadLSMChannelData()
        {
            string str = string.Empty;
            int intVal = 0;
            if (null != _experimentDoc)
            {
                XmlNodeList ndList = _experimentDoc.SelectNodes("/ThorImageExperiment/Wavelengths/ChannelEnable");
                if (0 < ndList.Count)
                {
                    if (XmlManager.GetAttribute(ndList[0], _experimentDoc, "Set", ref str) && (int.TryParse(str, out intVal)))
                    {
                        _lsmChannel = intVal;
                    }
                }
            }
        }

        public void LoadSequentialInfo()
        {
            string str = string.Empty;
            int intVal;
            if (null != _experimentDoc)
            {
                XmlNodeList ndList = _experimentDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
                _sequenceStepsNum = ndList.Count;
                SequenceSelectedChannel.Clear();
                _totalSequentialChannelCount = 0;

                foreach (XmlNode sequenceNode in ndList)
                {
                    int selectedChannel = 0;

                    if (ResourceManagerCS.GetCameraType() == (int)ICamera.CameraType.LSM)
                    {
                        XmlNode lsmNode = sequenceNode.SelectSingleNode("LSM");
                        if (XmlManager.GetAttribute(lsmNode, _experimentDoc, "channel", ref str) && Int32.TryParse(str, out intVal))
                        {
                            selectedChannel = intVal;
                        }
                    }
                    else
                    {
                        XmlNode camNode = sequenceNode.SelectSingleNode("Camera");
                        if (XmlManager.GetAttribute(camNode, _experimentDoc, "channel", ref str) && Int32.TryParse(str, out intVal))
                        {
                            selectedChannel = intVal;
                        }
                        else
                        {
                            selectedChannel = 1;
                        }
                    }

                    int count = 0;
                    for (int i = 0; i < MAX_CHANNELS; i++)
                    {
                        if ((selectedChannel & (0x0001 << i)) > 0)
                        {
                            count++;
                        }
                    }

                    SequenceSelectedChannel.Add(new Tuple<int, int>(selectedChannel, count));
                    _totalSequentialChannelCount += count;
                }
            }
        }

        public void RegisterCallbacks()
        {
            InitCallBackLineProfilePush(_lineProfileCallback);
        }

        public void UnRegisterCallbacks()
        {
            InitCallBackLineProfilePush(null);
        }

        /// <summary>
        /// Updates the channel data for the selected channel, compatible with Experiment style projects in both raw and tiff
        /// formats, as well as Custom style projects 
        /// </summary>
        /// <param name="fileNames"> List containing files to update </param>
        /// <param name="enabledChannels"> Bitmask of enabled channels </param>
        /// <param name="channelToRead"> Channel to read </param>
        /// <param name="zToRead"> Read channel at this depth </param>
        /// <param name="timeToRead"> Read channel at this time </param>
        /// <param name="imageWidth"> The width of the image in pixels </param>
        /// <param name="imageHeight"> The height of the image in pixels </param>
        /// <param name="imageDepth"> The depth of the image in pixels </param>
        /// <param name="rawContainsDisabledChannels"> Boolean representing if a raw file contains all channels, or just the enabled ones </param>
        public void UpdateChannelData(List<string> fileNames, byte enabledChannels = 0, byte channelToRead = 0, int zToRead = 0, int timeToRead = 0, int imageWidth = 0, int imageHeight = 0, int imageDepth = 0, bool rawContainsDisabledChannels = true, int scanAreaID = 0, int regionIndex = 0)
        {
            switch (FileFormatMode)
            {
                case FormatMode.CUSTOM:
                    {
                        UpdateCustomFileFormatChannelData(fileNames);
                        break;
                    }

                case FormatMode.EXPERIMENT:
                    {
                        if (SequentialModeEnabled)
                        {
                            if (fileNames.Count == _totalSequentialChannelCount)
                            {
                                int filesRead = 0;

                                for (int i = 0; i < _sequenceStepsNum; i++)
                                {
                                    _sequenceStepIndex = i;
                                    _sequenceStepChannels = SequenceSelectedChannel[i].Item1;

                                    List<string> sublistFileNames = new List<string>();
                                    for (int k = filesRead; k < filesRead + SequenceSelectedChannel[i].Item2; k++)
                                    {
                                        sublistFileNames.Add(fileNames[k]);
                                    }

                                    MVMManager.Instance["ImageViewReviewVM", "BitmapLoaded"] = false;

                                    UpdateExperimentFileFormatChannelData(sublistFileNames, enabledChannels, channelToRead, zToRead, timeToRead, imageWidth, imageHeight, imageDepth, rawContainsDisabledChannels, scanAreaID, regionIndex);

                                    filesRead += SequenceSelectedChannel[i].Item2;

                                    DateTime startTime = DateTime.Now;
                                    TimeSpan elapsedTime = DateTime.Now - startTime;
                                    while (false == (bool)MVMManager.Instance["ImageViewReviewVM", "BitmapLoaded", (object)true] && elapsedTime.TotalMilliseconds < BITMAP_UPDATE_TIMEOUT && false == ImageReviewViewModel._stopRequested)
                                    {
                                        elapsedTime = DateTime.Now - startTime;
                                    }
                                }
                            }
                        }
                        else
                        {
                            UpdateExperimentFileFormatChannelData(fileNames, enabledChannels, channelToRead, zToRead, timeToRead, imageWidth, imageHeight, imageDepth, rawContainsDisabledChannels, scanAreaID, regionIndex);
                        }
                        break;
                    }
            }
        }

        /// <summary>
        /// Updates the image data for selected channel in experiments with the custom format
        /// </summary>
        /// <param name="fileNames"> File names to read </param>
        public void UpdateCustomFileFormatChannelData(List<string> fileNames)
        {
            foreach (string file in fileNames)
            {
                if (false == File.Exists(file))
                {
                    continue;
                }

                string ext = Path.GetExtension(file);
                try
                {
                    switch (ext)
                    {
                        case ".tif":
                        case ".tiff":
                            {
                                int width = 0;
                                int height = 0;
                                int bitsPerPixel = 0;
                                _colorChannels = 1;

                                if (null != file)
                                    ReadImageInfo(file, ref width, ref height, ref _colorChannels, ref bitsPerPixel);

                                if (_imageWidth != width || _imageHeight != height)
                                {
                                    _imageWidth = width;
                                    _imageHeight = height;
                                    _correctionParametersSet = false;
                                    // hard coded to use MAX_CHANNELS, may change this later to _colorChannels
                                    _imageData = Marshal.AllocHGlobal(_imageWidth * _imageHeight * MAX_CHANNELS * 2);
                                }
                                ReadImage(file, ref _imageData);
                                CopyChannelData(MAX_CHANNELS, false, 0, 0);
                                break;
                            }

                        case ".jpg":
                        case ".jpeg":
                            {
                            }
                            break;
                    }
                }
                catch
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + "File not found exception");
                }
                finally
                {
                    if (_imageData != IntPtr.Zero)
                    {
                        Marshal.FreeHGlobal(_imageData);
                        _imageData = IntPtr.Zero;
                        _imageDataSize = 0;
                    }
                }
            }
        }

        /// <summary>
        /// Updates image data in selected channels for experiment file projects
        /// </summary>
        /// <param name="fileNames"> Files to read </param>
        /// <param name="chEnabled"> Bitmask of enabled channels</param>
        /// <param name="selectedChannel"> Selected channel </param>
        /// <param name="selectedZ"> The z coordinate of the selected channel </param>
        /// <param name="selectedTime"> The time coordinate of the selected channel </param>
        /// <param name="imageWidth"> The total width of the image in pixels </param>
        /// <param name="imageHeight"> The total height of the image in pixels </param>
        /// <param name="imageDepth"> The total depth(z) of the image in pixels </param>
        /// <param name="rawContainsDisabledChannels"> Boolean describing is a raw file is structured with blank data for disabled channels, or excludes
        ///  the data blocks for disabled channels all togeather </param>
        public void UpdateExperimentFileFormatChannelData(List<string> fileNames, byte chEnabled = 0, int selectedChannel = 0, int selectedZ = 0, int selectedTime = 0,
            int imageWidth = 0, int imageHeight = 0, int imageDepth = 0, bool rawContainsDisabledChannels = true, int scanAreaID = 0, int regionIndex = 0)
        {
            try
            {
                if (1 > fileNames.Count)
                {
                    return;
                }

                int width = 0;
                int height = 0;
                int colorChannels = 0;
                int bitsPerPixel = 0;
                bool contiguous = false;

                //=== Read File ===
                if (File.Exists(fileNames[0]))
                {
                    //=== Image Parameters ===

                    if (_imageWidth != imageWidth || _imageHeight != imageHeight)
                    {
                        _imageWidth = imageWidth;
                        _imageHeight = imageHeight;
                        _correctionParametersSet = false;
                    }
                    _colorChannels = (fileNames.Count > MAX_CHANNELS) ? fileNames.Count : MAX_CHANNELS; // Needs to change

                    switch ((CaptureFile)_ImageInfo.imageType)
                    {
                        case CaptureFile.FILE_BIG_TIFF:

                            if (_imageDataSize < _imageWidth * _imageHeight * _colorChannels * 2)
                            {
                                if (_imageData != IntPtr.Zero)
                                {
                                    Marshal.FreeHGlobal(_imageData);
                                    _imageData = IntPtr.Zero;
                                }
                                _imageData = Marshal.AllocHGlobal(_imageWidth * _imageHeight * _colorChannels * 2);
                                _imageDataSize = _imageWidth * _imageHeight * _colorChannels * 2;
                            }
                            RtlZeroMemory(_imageData, _imageWidth * _imageHeight * _colorChannels * 2);
                            if (!_lastFileNames.SequenceEqual(fileNames))
                            {
                                //load OME at first load, sliders were reset
                                int tileCount = 0, chCount = 0, zMaxCount = 0, timeCount = 0, specCount = 0;
                                ImageReview.GetImageStoreInfo(fileNames[0], 0, ref tileCount, ref width, ref height, ref chCount, ref zMaxCount, ref timeCount, ref specCount);
                                _lastFileNames = fileNames;
                            }
                            if (CaptureModes.HYPERSPECTRAL == ExperimentData.CaptureMode)
                            {
                                ReadImageStoreData(_imageData, _colorChannels, imageWidth, imageHeight, selectedZ, 0, selectedTime, 0);
                            }
                            else
                            {
                                ReadImageStoreData(_imageData, _colorChannels, imageWidth, imageHeight, selectedZ, selectedTime, 0, 0);
                            }
                            break;
                        case CaptureFile.FILE_RAW:
                            if (_imageDataSize < _imageWidth * _imageHeight * _colorChannels * 2)
                            {
                                if (_imageData != IntPtr.Zero)
                                {
                                    Marshal.FreeHGlobal(_imageData);
                                    _imageData = IntPtr.Zero;
                                }
                                _imageData = Marshal.AllocHGlobal(_imageWidth * _imageHeight * _colorChannels * 2);
                                _imageDataSize = _imageWidth * _imageHeight * _colorChannels * 2;
                            }
                            RtlZeroMemory(_imageData, _imageWidth * _imageHeight * _colorChannels * 2);

                            LoadImageIntoBufferFromRawFile(_imageData, fileNames[0], chEnabled, selectedChannel, selectedZ, selectedTime, imageWidth, imageHeight, imageDepth, rawContainsDisabledChannels);
                            break;
                        case CaptureFile.FILE_TIFF:
                            contiguous = true;
                            ReadImageInfo(fileNames[0], ref width, ref height, ref colorChannels, ref bitsPerPixel);
                            if (width <= 0 || height <= 0)
                            {
                                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, $"Image dimensions error while loading TIFF; Image Width = {width}, Image Height = {height}");
                                break;
                            }
                            bool isMultiChannel = colorChannels > 1;

                            if (_imageWidth != width || _imageHeight != height)
                            {
                                _imageWidth = width;
                                _imageHeight = height;
                                _correctionParametersSet = false;
                            }

                            _colorChannels = isMultiChannel ? colorChannels : fileNames.Count;

                            if (_imageDataSize < _imageWidth * _imageHeight * _colorChannels * 2)
                            {
                                if (_imageData != IntPtr.Zero)
                                {
                                    Marshal.FreeHGlobal(_imageData);
                                    _imageData = IntPtr.Zero;
                                }
                                _imageData = Marshal.AllocHGlobal(_imageWidth * _imageHeight * _colorChannels * 2);
                                _imageDataSize = _imageWidth * _imageHeight * _colorChannels * 2;
                            }
                            RtlZeroMemory(_imageData, _imageWidth * _imageHeight * _colorChannels * 2);

                            //read the image and output the buffer with image data
                            if (isMultiChannel)
                            {
                                ReadMultiChannelImage(fileNames[0], _colorChannels, ref _imageData, _imageWidth, _imageHeight);
                            }
                            else
                            {
                                ReadChannelImages(fileNames.ToArray(), _colorChannels, ref _imageData, _imageWidth, _imageHeight);
                            }

                            break;
                        default:
                            break;
                    }

                    CopyChannelData(_colorChannels, contiguous, scanAreaID, regionIndex);
                }
            }
            catch
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + "File not found exception");
            }
        }

        [DllImport(".\\StatsManager.dll", EntryPoint = "ComputeStats")]
        private static extern int ComputeStats(ushort[] data, FrameInfoStruct frameInfo, long colorChannels, int includeLineProfile, int includeRegularStats, int enabledChannelsOnly);

        [DllImport(".\\ThorImageProcess.dll", EntryPoint = "CorrectPreludeImageDistortion")]
        private static unsafe extern int CorrectPreludeImageDistortion(IntPtr imageData, ushort* correctedImageData, int width, int height);

        [DllImport(".\\StatsManager.dll", EntryPoint = "InitCallBackLineProfilePush")]
        private static extern void InitCallBackLineProfilePush(ReportLineProfile reportLineProfile);

        /// <summary>
        /// Returns if the channel is enabled based on the bitmask
        /// </summary>
        /// <param name="channel"> The channel to test </param>
        /// <param name="channelBitmask"> The bitmask describing enabled and disabled channels </param>
        /// <returns> True if the channel is enabled, false otherwise </returns>
        private static bool isChannelEnabled(int channel, int channelBitmask)
        {
            int firstBit = 1;
            channelBitmask = channelBitmask >> channel;
            return (1 == (channelBitmask & firstBit));
        }

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadColorImage")]
        private static extern int ReadColorImage([MarshalAs(UnmanagedType.LPWStr)] string rPath, [MarshalAs(UnmanagedType.LPWStr)] string gPath, [MarshalAs(UnmanagedType.LPWStr)] string bPath, ref IntPtr outputBuffer, int cameraWidth, int cameraHeight);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImage")]
        private static extern int ReadImage([MarshalAs(UnmanagedType.LPWStr)] string path, ref IntPtr outputBuffer);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo")]
        private static extern int ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)] string path, ref int width, ref int height, ref int colorChannels, ref int bitsPerPixel);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadMultiChannelImage")]
        private static extern int ReadMultiChannelImage([MarshalAs(UnmanagedType.LPWStr)] string fileName, int numChannels, ref IntPtr outputBuffer, int cameraWidth, int cameraHeight);

        [DllImport("kernel32.dll")]
        static extern void RtlZeroMemory(IntPtr dst, int length);

        [DllImport(".\\ThorImageProcess.dll", EntryPoint = "SetImageDistortionCorrectionParameters")]
        private static extern int SetImageDistortionCorrectionParameters(int imageWidth, int imageHeight, double xAngleMax, double yAngleMax, double galvoTiltAngle, double scanLensFocalLength);

        private unsafe void CopyChannelData(int numChannels, bool contiguousMode, int regionID, int regionIndex)
        {
            //bool imageUpdateComplete = (bool)MVMManager.Instance["ImageViewReviewVM", "ImageUpdateComplete", (object)false];
            //while (!imageUpdateComplete)
            //{
            //    Thread.Sleep(2);
            //    imageUpdateComplete = (bool)MVMManager.Instance["ImageViewReviewVM", "ImageUpdateComplete", (object)false];
            //}

            lock (_frameData.dataLock)
            {
                //TODO:IV set iv framedata here
                _dataLength = (_imageWidth * _imageHeight);

                if ((_pixelData == null) || (_pixelData.Length != (_dataLength * numChannels)))
                {
                    _pixelData = new ushort[_dataLength * numChannels];
                }

                if (1 == _LSMImageDistortionCorrectionEnable)
                {
                    if (!_correctionParametersSet)
                    {
                        SetImageDistortionCorrectionParameters(_imageWidth, _imageHeight, _LSMImageCalibrationXAngleMax, _LSMImageCalibrationYAngleMax, _LSMImageCalibrationGalvoTiltAngle, 50);
                        _correctionParametersSet = true;
                    }

                    if (contiguousMode || numChannels <=1)
                    {
                        for (int i = 0; i < numChannels; ++i)
                        {
                            IntPtr ptr = IntPtr.Add(_imageData, 2 * _imageWidth * _imageHeight * i);
                            fixed (ushort* pPixelData = &_pixelData[_imageWidth * _imageHeight * i])
                            {

                                CorrectPreludeImageDistortion(ptr, pPixelData, _imageWidth, _imageHeight);
                            }
                        }
                    }
                    else
                    {
                        for (int i = 0; i < MAX_CHANNELS; ++i)
                        {
                            IntPtr ptr = IntPtr.Add(_imageData, 2 * _imageWidth * _imageHeight * i);
                            fixed (ushort* pPixelData = &_pixelData[_imageWidth * _imageHeight * i])
                            {
                                CorrectPreludeImageDistortion(ptr, pPixelData, _imageWidth, _imageHeight);
                            }
                        }
                    }
                }
                else
                {
                    MemoryCopyManager.CopyIntPtrMemory(_imageData, _pixelData, 0, _dataLength * numChannels);
                }

                FrameInfoStruct imageInfo = new FrameInfoStruct();

                if (ExperimentData.IsmROICapture)
                {
                    int top = (int)Math.Floor((double)ExperimentData.mROIFullFOVMetadata.PixelHeight / 2 + ExperimentData.mROIs[regionIndex].PositionYUM / ExperimentData.mROIPixelSizeYUM);
                    int left = (int)Math.Floor((double)ExperimentData.mROIFullFOVMetadata.PixelWidth / 2 + (ExperimentData.mROIs[regionIndex].PositionXUM - ExperimentData.mROIs[regionIndex].PhysicalSizeXUM / ExperimentData.mROIs[regionIndex].Stripes / 2) / ExperimentData.mROIPixelSizeXUM);
                    imageInfo.bufferType = 0;
                    imageInfo.numberOfPlanes = 1;
                    imageInfo.fullFrame = 1;
                    imageInfo.copySize = (ulong)(_dataLength * numChannels);
                    imageInfo.channels = numChannels;
                    imageInfo.isNewMROIFrame = regionIndex == 0 ? 1 : 0;
                    imageInfo.isMROI = ExperimentData.IsmROICapture ? 1 : 0;
                    imageInfo.totalScanAreas = ExperimentData.mROIs.Count;
                    imageInfo.scanAreaIndex = regionIndex;
                    imageInfo.scanAreaID = regionID;
                    imageInfo.fullImageWidth = ExperimentData.mROIFullFOVMetadata.PixelWidth;
                    imageInfo.fullImageHeight = ExperimentData.mROIFullFOVMetadata.PixelHeight;
                    imageInfo.topInFullImage = top;
                    imageInfo.leftInFullImage = left;
                    imageInfo.imageHeight = _imageHeight;
                    imageInfo.imageWidth = _imageWidth;
                }
                else
                {
                    imageInfo.bufferType = 0;
                    imageInfo.imageHeight = (ExperimentData.NumberOfPlanes > 1) ? _imageHeight / ExperimentData.NumberOfPlanes : _imageHeight;
                    imageInfo.imageWidth = _imageWidth;
                    imageInfo.numberOfPlanes = ExperimentData.NumberOfPlanes;
                    imageInfo.fullFrame = 1;
                    imageInfo.copySize = (ulong)(_dataLength * numChannels);
                    imageInfo.channels = numChannels;//TODO:IV make sure we don't need actual number of channels// ExperimentData.NumberOfChannels;
                    imageInfo.sequenceIndex = SequentialModeEnabled ? _sequenceStepIndex : 0;
                    imageInfo.totalSequences = SequentialModeEnabled ? _sequenceStepsNum : 0;
                    imageInfo.sequenceSelectedChannels = SequentialModeEnabled ? _sequenceStepChannels : 0;
                }

                imageInfo.pixelAspectRatioYScale = (int)(ExperimentData.PixelAspectRatioYScale*100);
                _frameData.pixelData = _pixelData;
                _frameData.bitsPerPixel = ExperimentData.BitsPerPixel;
                _frameData.dFLIMArrivalTimeSumData = null;
                _frameData.dFLIMSinglePhotonData = null;
                _frameData.dFLIMBinDurations = null;
                _frameData.averageMode = 0;
                _frameData.averageFrameCount = 1;
                _frameData.contiguousChannels = contiguousMode;
                _frameData.channelSelection = ImageInfo.channelEnabled;
                _frameData.frameInfo = imageInfo;
                _frameData.pixelSizeUM = ExperimentData.PixelSizeUM;
            }
            MVMManager.Instance["ImageViewReviewVM", "FrameData"] = _frameData;
        }

        private void LineProfileUpdate(IntPtr lineProfile, int length, int realLength, int channelEnable, int numChannel)
        {
            if (0 < numChannel && 0 < length)
            {
                int lengthPerChannel = length / numChannel;

                _lineProfileData.LengthPerChannel = lengthPerChannel;
                _lineProfileData.PixeltoµmConversionFactor = _frameData.pixelSizeUM.PixelWidthUM * realLength / length;
                _lineProfileData.profileDataX = new double[lengthPerChannel];
                for (int i = 0; i < lengthPerChannel; i++)
                {
                    _lineProfileData.profileDataX[i] = i;
                }

                _lineProfileData.profileDataY = new double[numChannel][];

                for (int i = 0; i < numChannel; i++)
                {

                    _lineProfileData.profileDataY[i] = new double[lengthPerChannel];

                    Marshal.Copy(lineProfile + i * lengthPerChannel * sizeof(double), _lineProfileData.profileDataY[i], 0, lengthPerChannel);
                }

                _lineProfileData.channelEnable = channelEnable;
            }
            else
            {
                _lineProfileData.profileDataY = new double[0][];
                _lineProfileData.profileDataX = new double[0];
                _lineProfileData.channelEnable = 0;
            }

            if (null != LineProfileChanged)
            {
                LineProfileChanged(this, EventArgs.Empty);
            }
        }

        private void SaveLastExperimentInfo(string path)
        {
            if (null == path)
                return;

            if (null == _applicationDoc)
            {
                XmlDocument appSettings = new XmlDocument();
                _applicationDoc.Load(ApplicationSettingPath);
                ApplicationDoc = appSettings;
            }

            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/LastExperiment");
            if (node == null)
            {
                node = ApplicationDoc.CreateNode(XmlNodeType.Element, "LastExperiment", null);
                ApplicationDoc.DocumentElement.AppendChild(node);
            }

            XmlManager.SetAttribute(node, ApplicationDoc, "path", path);
            ApplicationDoc.Save(ApplicationSettingPath);
        }

        #endregion Methods
    }
}