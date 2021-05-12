namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Drawing.Design;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Xml;

    using CaptureSetupDll.Model;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetupViewModel : ViewModelBase
    {
        #region Fields

        const int MAX_HISTOGRAM_HEIGHT = 250;
        const int MAX_HISTOGRAM_WIDTH = 444;
        const double MAX_ZOOM = 1000; // In  1000x
        const int MIN_HISTOGRAM_HEIGHT = 108;
        const int MIN_HISTOGRAM_WIDTH = 192;
        const double MIN_ZOOM = .01; // Out 100x

        static readonly object _syncLock = new object();

        Color[] roiColorTables = new Color[] {Colors.Yellow, Colors.Lime, Colors.DodgerBlue, Colors.DeepPink, Colors.DarkOrange,
        Colors.Khaki, Colors.LightGreen, Colors.SteelBlue, Colors.BlueViolet,Colors.CornflowerBlue,Colors.LightPink,Colors.LavenderBlush,Colors.LightSeaGreen,
        Colors.Navy,Colors.YellowGreen, Colors.Transparent};
        private bool _allHistogramsExpanded = false;
        private bool[] _autoManualTogChecked = { false, false, false, false };
        private int _autoTrackTooltipX = 0;
        private int _autoTrackTooltipY = 0;
        WriteableBitmap _bitmap = null;
        WriteableBitmap _bitmap16 = null;
        private Point _bitmapPoint = new Point(0, 0);
        WriteableBitmap _bitmapPresentation = null;
        private bool _bitmapReady = false;
        private WriteableBitmap _bitmapXZ = null;
        private WriteableBitmap _bitmapYZ = null;
        private bool _bwOrthogonalImageLoaderDone = true;
        private string _chanAName;
        private string _chanBName;
        private string _chanCName;
        private string _chanDName;
        private ICommand _changeColorSettingsCommand;
        private bool[] _channelEnable = new bool[CaptureSetup.MAX_CHANNELS];
        private int _colorChannelsHistory = 0;
        private List<string> _currentChannelsLutFiles = new List<string>();
        private string _currentLutFile = string.Empty;
        private int _displayChannelIndex;
        private int _histogramHeight = 100;
        private int _histogramHeight1 = MIN_HISTOGRAM_HEIGHT;
        private int _histogramHeight2 = MIN_HISTOGRAM_HEIGHT;
        private int _histogramHeight3 = MIN_HISTOGRAM_HEIGHT;
        private int _histogramHeight4 = MIN_HISTOGRAM_HEIGHT;
        private int _histogramSpacing = 10;
        private int _histogramWidth = 100;
        private int _histogramWidth1 = MIN_HISTOGRAM_WIDTH;
        private int _histogramWidth2 = MIN_HISTOGRAM_WIDTH;
        private int _histogramWidth3 = MIN_HISTOGRAM_WIDTH;
        private int _histogramWidth4 = MIN_HISTOGRAM_WIDTH;
        private Visibility _isChannelVisible0 = Visibility.Visible;
        private Visibility _isChannelVisible1 = Visibility.Visible;
        private Visibility _isChannelVisible2 = Visibility.Visible;
        private Visibility _isChannelVisible3 = Visibility.Visible;
        bool _isOrthogonalViewChecked = false;
        private Visibility _isTileDisplayButtonVisible = Visibility.Visible;
        private bool _largeHistogram1 = false;
        private bool _largeHistogram2 = false;
        private bool _largeHistogram3 = false;
        private bool _largeHistogram4 = false;
        private DateTime _lastOrthogonalViewUpdateTime;
        private bool _logScaleEnabled0 = false;
        private bool _logScaleEnabled1 = false;
        private bool _logScaleEnabled2 = false;
        private bool _logScaleEnabled3 = false;
        private Brush[] _lsmChannelColor = new Brush[CaptureSetup.MAX_CHANNELS];
        private OrthogonalViewStatus _orthogonalViewStat = OrthogonalViewStatus.INACTIVE;
        double _orthogonalViewZMultiplier = 1.0;
        private byte[] _pdXZ;
        private byte[] _pdYZ;
        private Visibility _pmtSaturationsVisibility = Visibility.Visible;
        private int _progressPercentage;
        private bool _rebuildBitmap = false;
        BitmapPalette _roiPalette = null;
        private CaptureSetupDll.View.SplashScreen _splashOrthogonalView;
        private ushort[][] _tiffBufferArray;
        private bool _tileDisplay = false;

        //private bool _updateVirtualStack = true; //TODO:see if needed
        private bool _virtualZStack = true;
        private string _wavelengthName;
        private string[] _wavelengthNames = new string[CaptureSetup.MAX_CHANNELS];
        double _zoomLevel = 1;

        #endregion Fields

        #region Enumerations

        public enum OrthogonalViewStatus
        {
            INACTIVE, ACTIVE, HOLD
        }

        #endregion Enumerations

        #region Delegates

        public delegate void UpdateProgressDelegate(int percentage);

        #endregion Delegates

        #region Events

        public event Action ChannelChanged;

        //notify 3D VolumeView that the color mapping has changed
        public event Action<bool> ColorMappingChanged;

        //notify listeners (Ex. histogram) that the image has changed
        public event Action<bool> ImageDataChanged;

        //Increase the view area if the image extends out of bonds for the vertical scroll bar
        public event Action<bool> IncreaseViewArea;

        //notify 2D volumeview that the live image is ongoing
        public event Action<bool> LiveImageCapturing;

        //notify ImageView When a OrthogonalView images was loaded
        public event Action<bool> OrthogonalViewImagesLoaded;

        #endregion Events

        #region Properties

        public bool AllHistogramsExpanded
        {
            get
            {
                return _allHistogramsExpanded;
            }
            set
            {
                _allHistogramsExpanded = value;
                if (_allHistogramsExpanded)
                {
                    LargeHistogram1 = false;
                    LargeHistogram2 = false;
                    LargeHistogram3 = false;
                    LargeHistogram4 = false;
                    ExpandAllHistograms();
                }
                else
                {
                    ShrinkAllHistograms();
                }
                OnPropertyChanged("AllHistogramsExpanded");
            }
        }

        public bool AutoManualTog1Checked
        {
            get
            {
                return _autoManualTogChecked[0];
            }
            set
            {
                _autoManualTogChecked[0] = value;
                if (value && null != ImageDataChanged)
                    ImageDataChanged(true);
                OnPropertyChanged("AutoManualTog1Checked");
            }
        }

        public bool AutoManualTog2Checked
        {
            get
            {
                return _autoManualTogChecked[1];
            }
            set
            {
                _autoManualTogChecked[1] = value;
                if (value && null != ImageDataChanged)
                    ImageDataChanged(true);
                OnPropertyChanged("AutoManualTog2Checked");
            }
        }

        public bool AutoManualTog3Checked
        {
            get
            {
                return _autoManualTogChecked[2];
            }
            set
            {
                _autoManualTogChecked[2] = value;
                if (value && null != ImageDataChanged)
                    ImageDataChanged(true);
                OnPropertyChanged("AutoManualTog3Checked");
            }
        }

        public bool AutoManualTog4Checked
        {
            get
            {
                return _autoManualTogChecked[3];
            }
            set
            {
                _autoManualTogChecked[3] = value;
                if (value && null != ImageDataChanged)
                    ImageDataChanged(true);
                OnPropertyChanged("AutoManualTog4Checked");
            }
        }

        public int AutoTrackToolTipX
        {
            get
            {
                return this._autoTrackTooltipX;
            }
            set
            {
                this._autoTrackTooltipX = value;
                OnPropertyChanged("AutoTrackToolTipX");
            }
        }

        public int AutoTrackToolTipY
        {
            get
            {
                return this._autoTrackTooltipY;
            }
            set
            {
                this._autoTrackTooltipY = value;
                OnPropertyChanged("AutoTrackToolTipY");
            }
        }

        public WriteableBitmap Bitmap
        {
            get
            {
                try
                {
                    if (false == CaptureSetup.IsPixelDataReady() && false == CaptureSetup.PaletteChanged && false == _rebuildBitmap)
                    {
                        return _bitmap;
                    }

                    if (CaptureSetup.GetColorChannels() != _colorChannelsHistory)
                    {
                        _colorChannelsHistory = CaptureSetup.GetColorChannels();
                    }
                    lock (_syncLock)
                    {
                        byte[] pd = CaptureSetup.GetPixelDataByteEx(true, 0);

                        //verify pixel data is available
                        if (null == pd)
                        {
                            return _bitmap;
                        }

                        // Define parameters used to create the BitmapSource.
                        PixelFormat pf = PixelFormats.Rgb24;

                        int width = this._captureSetup.DataWidth;
                        int height = this._captureSetup.DataHeight;
                        int rawStride = (width * pf.BitsPerPixel + 7) / 8;

                        int planes = CaptureSetup.ImageInfo.numberOfPlanes > 1 ? CaptureSetup.ImageInfo.numberOfPlanes : 1;

                        int outputBitmapWidth = width;
                        int outputBitmapHeight = height;
                        bool dflimDisplayLifetimeImage = (bool)MVMManager.Instance["DFLIMControlViewModel", "DFLIMDisplayLifetimeImage", (object)false];
                        bool doLifetime = ((int)ThorSharedTypes.BufferType.DFLIM_IMAGE == CaptureSetup.ImageInfo.bufferType || (int)ThorSharedTypes.BufferType.DFLIM_ALL == CaptureSetup.ImageInfo.bufferType) && dflimDisplayLifetimeImage;
                        int channelNum = 0;
                        for (int i = 0; i < _channelEnable.Length; ++i)
                        {
                            if (true == (_channelEnable[i]))
                            {
                                ++channelNum;
                            }
                        }
                        if (0 == channelNum)
                        {
                            return null;
                        }
                        if (TileDisplay && (_colorChannelsHistory > 1 || doLifetime))
                        {

                            if (planes > 1)
                            {
                                outputBitmapWidth *= channelNum + 1;
                            }
                            else
                            {
                                switch (channelNum)
                                {
                                    case 1:
                                        {
                                            if (doLifetime)
                                            {
                                                outputBitmapWidth *= 2;
                                                outputBitmapHeight *= 1;
                                            }
                                        }
                                        break;
                                    case 2:
                                        {
                                            outputBitmapWidth *= 3;
                                            outputBitmapHeight *= 1;
                                        }
                                        break;
                                    case 3:
                                        {
                                            outputBitmapWidth *= 2;
                                            outputBitmapHeight *= 2;
                                        }
                                        break;
                                    default:
                                        {// All 4 Channels enabled
                                            outputBitmapWidth *= 3;
                                            outputBitmapHeight *= 2;
                                        }
                                        break;
                                }
                            }
                        }
                        //create a new bitmpap when one does not exist or the size of the image changes
                        if (null == _bitmap)
                        {
                            _bitmap = new WriteableBitmap(outputBitmapWidth, outputBitmapHeight, 96, 96, pf, null);
                        }
                        else
                        {
                            if ((_bitmap.Width != outputBitmapWidth) || (_bitmap.Height != outputBitmapHeight) || (_bitmap.Format != pf))
                            {
                                _bitmap = new WriteableBitmap(outputBitmapWidth, outputBitmapHeight, 96, 96, pf, null);
                            }
                        }

                        int w = _bitmap.PixelWidth;
                        int h = _bitmap.PixelHeight;

                        if ((pd.Length / 3) == (width * height))
                        {
                            //copy the color pixel data into the bitmap
                            _bitmap.WritePixels(new Int32Rect(0, 0, width, height), pd, rawStride, 0);

                            // Ignore this part if only one channel is enabled

                            if (TileDisplay && (1 < channelNum || doLifetime))
                            {
                                int offsetWidth = 1;
                                int offsetHeight = 0;
                                for (int i = 0; i < _channelEnable.Length; ++i)
                                {
                                    if (true == (_channelEnable[i]))
                                    {
                                        pd = CaptureSetup.GetPixelDataByteEx(false, i);
                                        _bitmap.WritePixels(new Int32Rect(offsetWidth * width, offsetHeight * height, width, height), pd, rawStride, 0);
                                        ++offsetWidth;
                                        if (outputBitmapWidth < (width * offsetWidth + width))
                                        {
                                            offsetWidth = 0;
                                            ++offsetHeight;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (null != ImageDataChanged)
                    {
                        ImageDataChanged(true);
                    }

                    _rebuildBitmap = false;

                    CaptureSetup.FinishedCopyingPixel();
                    return _bitmap;
                }

                catch (Exception e)
                {
                    ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Bitmap getter failed. Exception Message:\n" + e.Message);
                    return _bitmap;
                }
            }
        }

        public Point BitmapPoint
        {
            get
            {
                return _bitmapPoint;
            }
            set
            {
                _bitmapPoint = value;
            }
        }

        public WriteableBitmap BitmapPresentation
        {
            get
            {
                if (this._captureSetup.IsProcessImageReady == true)
                {
                    int width = 0;
                    int height = 0;
                    byte[] pd = CaptureSetup.GetPixelImageProcessDataByte(ref width, ref height);

                    //verify pixel data is available
                    if (pd == null)
                    {
                        return _bitmapPresentation;
                    }
                    // Define parameters used to create the BitmapSource.
                    PixelFormat pf = PixelFormats.Indexed8;
                    int rawStride = (width * pf.BitsPerPixel + 7) / 8;

                    if ((null == _roiPalette))
                    {
                        _roiPalette = BuildPaletteContour();
                    }

                    //create a new bitmpap when one does not exist or the size of the image changes
                    if ((_bitmapPresentation == null) || (_bitmapPresentation.Width != width) || (_bitmapPresentation.Height != height) || (_bitmapPresentation.Format != pf))
                    {
                        _bitmapPresentation = new WriteableBitmap(width, height, 96, 96, pf, _roiPalette);
                    }

                    int w = _bitmapPresentation.PixelWidth;
                    int h = _bitmapPresentation.PixelHeight;
                    int widthInBytes = w;
                    if (pd.Length == (width * height))
                    {
                        //copy the pixel data into the _bitmap
                        _bitmapPresentation.WritePixels(new Int32Rect(0, 0, w, h), pd, rawStride, 0);
                    }
                    this._captureSetup.IsProcessImageReady = false;
                    if (CaptureSetupViewModel.PresentationCanvas.Visibility == Visibility.Collapsed && AutoROIDisplayChannel != 0)
                    {
                        CaptureSetupViewModel.PresentationCanvas.Visibility = Visibility.Visible;
                    }
                }
                return _bitmapPresentation;
            }
        }

        public bool BitmapReady
        {
            get { return _bitmapReady; }
            set
            {
                _bitmapReady = value;
                OnPropertyChanged("BitmapReady");
            }
        }

        public WriteableBitmap BitmapXZ
        {
            get
            {
                PixelFormat pf = PixelFormats.Rgb24;
                int step = pf.BitsPerPixel / 8;
                int totalNumOfZstack = (int)MVMManager.Instance["ZControlViewModel", "ZScanNumSteps", (object)1];
                int width = this._captureSetup.DataWidth;

                if (width != 0 && _pdXZ != null)
                {
                    // Define parameters used to create the BitmapSource.
                    int rawStrideXZ = (width * pf.BitsPerPixel + 7) / 8; //_bitmapXZ

                    double pixelSizeUM = (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (double)1.0];
                    double zStepSizeUM = (double)MVMManager.Instance["ZControlViewModel", "ZScanStep", (object)0.0];

                    double yMultiplier = pixelSizeUM / zStepSizeUM / _orthogonalViewZMultiplier;

                    double dpiX = 96;
                    double dpiY = 96 * yMultiplier;

                    //create a new bitmpap when one does not exist or the size of the image changes
                    //need to round the dpi because they are changed internally, and more than 3 decimal places shouldn't make a real difference
                    if ((_bitmapXZ == null) || (_bitmapXZ.PixelWidth != width) || (_bitmapXZ.Format != pf) || (totalNumOfZstack != _bitmapXZ.PixelHeight) ||
                        (Math.Round(_bitmapXZ.DpiX, 3) != Math.Round(dpiX, 3)) || (Math.Round(_bitmapXZ.DpiY, 3) != Math.Round(dpiY, 3)))
                    {
                        _bitmapXZ = new WriteableBitmap(width, totalNumOfZstack, dpiX, dpiY, pf, null);
                    }
                    _bitmapXZ.WritePixels(new Int32Rect(0, 0, width, totalNumOfZstack), _pdXZ, rawStrideXZ, 0);

                    return _bitmapXZ;
                }
                return null;
            }
        }

        public WriteableBitmap BitmapYZ
        {
            get
            {
                PixelFormat pf = PixelFormats.Rgb24;
                int step = pf.BitsPerPixel / 8;
                int totalNumOfZstack = (int)MVMManager.Instance["ZControlViewModel", "ZScanNumSteps", (object)1];
                int height = this._captureSetup.DataHeight;

                if (height != 0 && _pdYZ != null)
                {
                    // Define parameters used to create the BitmapSource.
                    int rawStrideYZ = (totalNumOfZstack * pf.BitsPerPixel + 7) / 8;

                    double pixelSizeUM = (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (double)1.0];
                    double zStepSizeUM = (double)MVMManager.Instance["ZControlViewModel", "ZScanStep", (object)0.0];

                    double xMultiplier = pixelSizeUM / zStepSizeUM / _orthogonalViewZMultiplier;

                    double dpiX = 96.0 * xMultiplier;
                    double dpiY = 96.0;

                    //create a new bitmpap when one does not exist or the size of the image changes
                    //need to round the dpi because they are changed internally, and more than 3 decimal places shouldn't make a real difference
                    if ((_bitmapYZ == null) || (_bitmapYZ.PixelHeight != height) || (_bitmapYZ.Format != pf) || (totalNumOfZstack != _bitmapYZ.PixelWidth) ||
                        (Math.Round(_bitmapYZ.DpiX, 3) != Math.Round(dpiX, 3)) || (Math.Round(_bitmapYZ.DpiY, 3) != Math.Round(dpiY, 3)))
                    {
                        _bitmapYZ = new WriteableBitmap(totalNumOfZstack, height, dpiX, dpiY, pf, null);
                    }
                    _bitmapYZ.WritePixels(new Int32Rect(0, 0, totalNumOfZstack, height), _pdYZ, rawStrideYZ, 0);

                    return _bitmapYZ;
                }
                return null;
            }
        }

        public double BlackPoint0
        {
            get
            {
                return this._captureSetup.BlackPoint0;
            }
            set
            {
                if (value != this._captureSetup.BlackPoint0)
                {
                    this._captureSetup.BlackPoint0 = value;
                    OnPropertyChanged("BlackPoint0");
                    OnPropertyChanged("Bitmap");
                    if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && VirtualZStack == true)
                    {
                        UpdateOrthogonalViewImages();
                    }
                }
            }
        }

        public double BlackPoint1
        {
            get
            {
                return this._captureSetup.BlackPoint1;
            }
            set
            {
                if (value != this._captureSetup.BlackPoint1)
                {
                    this._captureSetup.BlackPoint1 = value;
                    OnPropertyChanged("BlackPoint1");
                    OnPropertyChanged("Bitmap");
                    if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && VirtualZStack == true)
                    {
                        UpdateOrthogonalViewImages();
                    }
                }
            }
        }

        public double BlackPoint2
        {
            get
            {
                return this._captureSetup.BlackPoint2;
            }
            set
            {
                if (value != this._captureSetup.BlackPoint2)
                {
                    this._captureSetup.BlackPoint2 = value;
                    OnPropertyChanged("BlackPoint2");
                    OnPropertyChanged("Bitmap");
                    if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && VirtualZStack == true)
                    {
                        UpdateOrthogonalViewImages();
                    }
                }
            }
        }

        public double BlackPoint3
        {
            get
            {
                return this._captureSetup.BlackPoint3;
            }
            set
            {
                if (value != this._captureSetup.BlackPoint3)
                {
                    this._captureSetup.BlackPoint3 = value;
                    OnPropertyChanged("BlackPoint3");
                    OnPropertyChanged("Bitmap");
                    if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && VirtualZStack == true)
                    {
                        UpdateOrthogonalViewImages();
                    }
                }
            }
        }

        public ICommand ChangeColorSettingsCommand
        {
            get
            {
                if (this._changeColorSettingsCommand == null)
                    this._changeColorSettingsCommand = new RelayCommand(() => ChangeColorSettings());

                return this._changeColorSettingsCommand;
            }
        }

        public string ChannelAName
        {
            get
            {
                return _chanAName;
            }

            set
            {
                _chanAName = value;
                OnPropertyChanged("ChannelAName");
            }
        }

        public string ChannelBName
        {
            get
            {
                return _chanBName;
            }

            set
            {
                _chanBName = value;
                OnPropertyChanged("ChannelBName");
            }
        }

        public string ChannelCName
        {
            get
            {
                return _chanCName;
            }

            set
            {
                _chanCName = value;
                OnPropertyChanged("ChannelCName");
            }
        }

        public string ChannelDName
        {
            get
            {
                return _chanDName;
            }

            set
            {
                _chanDName = value;
                OnPropertyChanged("ChannelDName");
            }
        }

        public ObservableCollection<StringPC> ChannelName
        {
            get;
            set;
        }

        public int ColorChannels
        {
            get
            {
                return CaptureSetup.GetColorChannels();
            }
        }

        public int DataHeight
        {
            get
            {
                return this._captureSetup.DataHeight;
            }
        }

        public int DataWidth
        {
            get
            {
                return this._captureSetup.DataWidth;
            }
        }

        public int DFLIMDiagnosticsBufferLength
        {
            get
            {
                return _captureSetup.DFLIMDiagnosticsBufferLength;
            }
            set
            {
                _captureSetup.DFLIMDiagnosticsBufferLength = value;
            }
        }

        public ushort[][] DFLIMDiagnosticsData
        {
            get
            {
                return _captureSetup.DFLIMDiagnosticsData;
            }
        }

        public object DFLIMDiagnosticsDataLock
        {
            get
            {
                return this._captureSetup.DFLIMDiagnosticsDataLock;
            }
        }

        public uint[][] DFLIMHistogramData
        {
            get
            {
                return this._captureSetup.DFLIMHistogramData;
            }
        }

        public object DFLIMHistogramDataLock
        {
            get
            {
                return this._captureSetup.DFLIMHistogramDataLock;
            }
        }

        public bool DFLIMNewHistogramData
        {
            get
            {
                return this._captureSetup.DFLIMNewHistogramData;
            }
            set
            {
                this._captureSetup.DFLIMNewHistogramData = value;
            }
        }

        public int DisplayChannelIndex
        {
            get
            {
                return _displayChannelIndex;
            }
            set
            {
                _displayChannelIndex = value;
            }
        }

        public bool GrayscaleForSingleChannel
        {
            get
            {
                return CaptureSetup.GrayscaleForSingleChannel;
            }
            set
            {
                CaptureSetup.GrayscaleForSingleChannel = value;
                OnPropertyChanged("GrayscaleForSingleChannel");
            }
        }

        public int[] HistogramData0
        {
            get
            {
                return this._captureSetup.HistogramData0;
            }
        }

        public int[] HistogramData1
        {
            get
            {
                return this._captureSetup.HistogramData1;
            }
        }

        public int[] HistogramData2
        {
            get
            {
                return this._captureSetup.HistogramData2;
            }
        }

        public int[] HistogramData3
        {
            get
            {
                return this._captureSetup.HistogramData3;
            }
        }

        public object HistogramDataLock
        {
            get
            {
                return this._captureSetup.HistogramDataLock;
            }
        }

        public int HistogramHeight
        {
            get
            {
                return _histogramHeight;
            }
            set
            {
                _histogramHeight = value;
                OnPropertyChanged("HistogramHeight");
            }
        }

        public int HistogramHeight1
        {
            get
            {
                return _histogramHeight1;
            }
            set
            {
                _histogramHeight1 = value;
                OnPropertyChanged("HistogramHeight1");
            }
        }

        public int HistogramHeight2
        {
            get
            {
                return _histogramHeight2;
            }
            set
            {
                _histogramHeight2 = value;
                OnPropertyChanged("HistogramHeight2");
            }
        }

        public int HistogramHeight3
        {
            get
            {
                return _histogramHeight3;
            }
            set
            {
                _histogramHeight3 = value;
                OnPropertyChanged("HistogramHeight3");
            }
        }

        public int HistogramHeight4
        {
            get
            {
                return _histogramHeight4;
            }
            set
            {
                _histogramHeight4 = value;
                OnPropertyChanged("HistogramHeight4");
            }
        }

        public int HistogramSpacing
        {
            get
            {
                return 10;
                //return _histogramSpacing;
            }
            set
            {
                _histogramSpacing = value;
                OnPropertyChanged("HistogramSpacing");
            }
        }

        public int HistogramWidth
        {
            get
            {
                return _histogramWidth;
            }
            set
            {
                _histogramWidth = value;
                OnPropertyChanged("HistogramWidth");
            }
        }

        public int HistogramWidth1
        {
            get
            {
                return _histogramWidth1;
            }
            set
            {
                _histogramWidth1 = value;
                OnPropertyChanged("HistogramWidth1");
            }
        }

        public int HistogramWidth2
        {
            get
            {
                return _histogramWidth2;
            }
            set
            {
                _histogramWidth2 = value;
                OnPropertyChanged("HistogramWidth2");
            }
        }

        public int HistogramWidth3
        {
            get
            {
                return _histogramWidth3;
            }
            set
            {
                _histogramWidth3 = value;
                OnPropertyChanged("HistogramWidth3");
            }
        }

        public int HistogramWidth4
        {
            get
            {
                return _histogramWidth4;
            }
            set
            {
                _histogramWidth4 = value;
                OnPropertyChanged("HistogramWidth4");
            }
        }

        public int ImageColorChannels
        {
            get
            {
                return this._captureSetup.ImageColorChannels;
            }
        }

        public string ImageNameFormat
        {
            get
            {
                return this._captureSetup.ImageNameFormat;
            }
        }

        public Visibility IsChannelVisible0
        {
            get
            {
                return _isChannelVisible0;
            }
            set
            {
                _isChannelVisible0 = value;
                OnPropertyChanged("IsChannelVisible0");
            }
        }

        public Visibility IsChannelVisible1
        {
            get
            {
                return _isChannelVisible1;
            }
            set
            {
                _isChannelVisible1 = value;
                OnPropertyChanged("IsChannelVisible1");
            }
        }

        public Visibility IsChannelVisible2
        {
            get
            {
                return _isChannelVisible2;
            }
            set
            {
                _isChannelVisible2 = value;
                OnPropertyChanged("IsChannelVisible2");
            }
        }

        public Visibility IsChannelVisible3
        {
            get
            {
                return _isChannelVisible3;
            }
            set
            {
                _isChannelVisible3 = value;
                OnPropertyChanged("IsChannelVisible3");
            }
        }

        public bool IsOrthogonalViewChecked
        {
            get
            {
                return _isOrthogonalViewChecked;
            }
            set
            {
                _isOrthogonalViewChecked = value;
                OnPropertyChanged("IsOrthogonalViewChecked");
            }
        }

        public bool IsSingleChannel
        {
            get
            {
                return _captureSetup.IsSingleChannel;
            }
        }

        public Visibility IsTileDisplayButtonVisible
        {
            get
            {
                return _isTileDisplayButtonVisible;
            }
            set
            {
                _isTileDisplayButtonVisible = value;
                OnPropertyChanged("IsTileDisplayButtonVisible");
            }
        }

        public bool LargeHistogram1
        {
            get
            {
                return _largeHistogram1;
            }
            set
            {
                _largeHistogram1 = value;
                if (_largeHistogram1)
                {
                    LargeHistogram2 = false;
                    LargeHistogram3 = false;
                    LargeHistogram4 = false;
                    AllHistogramsExpanded = false;
                    HistogramHeight1 = MAX_HISTOGRAM_HEIGHT;
                    HistogramWidth1 = MAX_HISTOGRAM_WIDTH;
                }
                else
                {
                    HistogramHeight1 = MIN_HISTOGRAM_HEIGHT;
                    HistogramWidth1 = MIN_HISTOGRAM_WIDTH;
                }
                OnPropertyChanged("LargeHistogram1");
            }
        }

        public bool LargeHistogram2
        {
            get
            {
                return _largeHistogram2;
            }
            set
            {
                _largeHistogram2 = value;
                if (_largeHistogram2)
                {
                    LargeHistogram1 = false;
                    LargeHistogram3 = false;
                    LargeHistogram4 = false;
                    AllHistogramsExpanded = false;
                    HistogramHeight2 = MAX_HISTOGRAM_HEIGHT;
                    HistogramWidth2 = MAX_HISTOGRAM_WIDTH;
                }
                else
                {
                    HistogramHeight2 = MIN_HISTOGRAM_HEIGHT;
                    HistogramWidth2 = MIN_HISTOGRAM_WIDTH;
                }
                OnPropertyChanged("LargeHistogram2");
            }
        }

        public bool LargeHistogram3
        {
            get
            {
                return _largeHistogram3;
            }
            set
            {
                _largeHistogram3 = value;
                if (_largeHistogram3)
                {
                    LargeHistogram1 = false;
                    LargeHistogram2 = false;
                    LargeHistogram4 = false;
                    AllHistogramsExpanded = false;
                    HistogramHeight3 = MAX_HISTOGRAM_HEIGHT;
                    HistogramWidth3 = MAX_HISTOGRAM_WIDTH;
                }
                else
                {
                    HistogramHeight3 = MIN_HISTOGRAM_HEIGHT;
                    HistogramWidth3 = MIN_HISTOGRAM_WIDTH;
                }
                OnPropertyChanged("LargeHistogram3");
            }
        }

        public bool LargeHistogram4
        {
            get
            {
                return _largeHistogram4;
            }
            set
            {
                _largeHistogram4 = value;
                if (_largeHistogram4)
                {
                    LargeHistogram1 = false;
                    LargeHistogram2 = false;
                    LargeHistogram3 = false;
                    AllHistogramsExpanded = false;
                    HistogramHeight4 = MAX_HISTOGRAM_HEIGHT;
                    HistogramWidth4 = MAX_HISTOGRAM_WIDTH;
                }
                else
                {
                    HistogramHeight4 = MIN_HISTOGRAM_HEIGHT;
                    HistogramWidth4 = MIN_HISTOGRAM_WIDTH;
                }
                OnPropertyChanged("LargeHistogram4");
            }
        }

        public bool LogScaleEnabled0
        {
            get
            {
                return _logScaleEnabled0;
            }
            set
            {
                _logScaleEnabled0 = value;
                OnPropertyChanged("LogScaleEnabled0");
            }
        }

        public bool LogScaleEnabled1
        {
            get
            {
                return _logScaleEnabled1;
            }
            set
            {
                _logScaleEnabled1 = value;
                OnPropertyChanged("LogScaleEnabled1");
            }
        }

        public bool LogScaleEnabled2
        {
            get
            {
                return _logScaleEnabled2;
            }
            set
            {
                _logScaleEnabled2 = value;
                OnPropertyChanged("LogScaleEnabled2");
            }
        }

        public bool LogScaleEnabled3
        {
            get
            {
                return _logScaleEnabled3;
            }
            set
            {
                _logScaleEnabled3 = value;
                OnPropertyChanged("LogScaleEnabled3");
            }
        }

        public int LSMChannel
        {
            get
            {
                return this._captureSetup.LSMChannel;
            }
            set
            {
                this._captureSetup.LSMChannel = value;
                OnPropertyChanged("LSMChannel");
                OnPropertyChanged("LSMPixelXMax");
                OnPropertyChanged("LSMPixelYMax");

                _displayChannelIndex = value;
            }
        }

        public Brush[] LSMChannelColor
        {
            get
            {
                return _lsmChannelColor;
            }
            set
            {
                _lsmChannelColor = value;
                OnPropertyChanged("LSMChannelColor");
            }
        }

        public Brush LSMChannelColor0
        {
            get
            {
                return _lsmChannelColor[0];
            }
            set
            {
                _lsmChannelColor[0] = value;
                OnPropertyChanged("LSMChannelColor0");
            }
        }

        public Brush LSMChannelColor1
        {
            get
            {
                return _lsmChannelColor[1];
            }
            set
            {
                _lsmChannelColor[1] = value;
                OnPropertyChanged("LSMChannelColor1");
            }
        }

        public Brush LSMChannelColor2
        {
            get
            {
                return _lsmChannelColor[2];
            }
            set
            {
                _lsmChannelColor[2] = value;
                OnPropertyChanged("LSMChannelColor2");
            }
        }

        public Brush LSMChannelColor3
        {
            get
            {
                return _lsmChannelColor[3];
            }
            set
            {
                _lsmChannelColor[3] = value;
                OnPropertyChanged("LSMChannelColor3");
            }
        }

        public bool[] LSMChannelEnable
        {
            get
            {
                return this._captureSetup.LSMChannelEnable;
            }
        }

        public bool LSMChannelEnable0
        {
            get
            {
                _channelEnable[0] = this._captureSetup.LSMChannelEnable0;
                return this._captureSetup.LSMChannelEnable0;
            }
            set
            {
                if (this._captureSetup.LSMChannelEnable0 != value)
                {
                    this._captureSetup.LSMChannelEnable0 = value;
                    OnPropertyChanged("LSMChannelEnable0");
                    OnPropertyChanged("LSMChannel");
                    ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMPixelXMax");
                    ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMPixelYMax");
                    bool modified = (bool)MVMManager.Instance["AreaControlViewModel", "ConfirmAreaModeSettingsForGG", false];
                    if (ChannelChanged != null)
                    {
                        ChannelChanged();
                    }
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeMinIndex");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeIndex");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTime");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeMaxIndex");
                    PersistChannels();
                    UpdateChannelData();
                }
            }
        }

        public bool LSMChannelEnable1
        {
            get
            {
                _channelEnable[1] = this._captureSetup.LSMChannelEnable1;
                return this._captureSetup.LSMChannelEnable1;
            }
            set
            {
                if (this._captureSetup.LSMChannelEnable1 != value)
                {
                    this._captureSetup.LSMChannelEnable1 = value;
                    OnPropertyChanged("LSMChannelEnable1");
                    OnPropertyChanged("LSMChannel");
                    ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMPixelXMax");
                    ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMPixelYMax");
                    bool modified = (bool)MVMManager.Instance["AreaControlViewModel", "ConfirmAreaModeSettingsForGG", (object)false];
                    if (ChannelChanged != null)
                    {
                        ChannelChanged();
                    }
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeMinIndex");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeIndex");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTime");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeMaxIndex");
                    PersistChannels();
                    UpdateChannelData();
                }
            }
        }

        public bool LSMChannelEnable2
        {
            get
            {
                _channelEnable[2] = this._captureSetup.LSMChannelEnable2;
                return this._captureSetup.LSMChannelEnable2;
            }
            set
            {
                if (this._captureSetup.LSMChannelEnable2 != value)
                {
                    this._captureSetup.LSMChannelEnable2 = value;
                    OnPropertyChanged("LSMChannelEnable2");
                    OnPropertyChanged("LSMChannel");
                    ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMPixelXMax");
                    ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMPixelYMax");
                    bool modified = (bool)MVMManager.Instance["AreaControlViewModel", "ConfirmAreaModeSettingsForGG", false];
                    if (ChannelChanged != null)
                    {
                        ChannelChanged();
                    }
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeMinIndex");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeIndex");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTime");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeMaxIndex");
                    PersistChannels();
                    UpdateChannelData();
                }
            }
        }

        public bool LSMChannelEnable3
        {
            get
            {

                _channelEnable[3] = this._captureSetup.LSMChannelEnable3;
                return this._captureSetup.LSMChannelEnable3;
            }
            set
            {
                if (this._captureSetup.LSMChannelEnable3 != value)
                {
                    this._captureSetup.LSMChannelEnable3 = value;
                    OnPropertyChanged("LSMChannelEnable3");
                    OnPropertyChanged("LSMChannel");
                    ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMPixelXMax");
                    ((IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("LSMPixelYMax");
                    bool modified = (bool)MVMManager.Instance["AreaControlViewModel", "ConfirmAreaModeSettingsForGG", false];
                    if (ChannelChanged != null)
                    {
                        ChannelChanged();
                    }
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeMinIndex");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeIndex");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTime");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeMaxIndex");
                    PersistChannels();
                    UpdateChannelData();
                }
            }
        }

        public int MaxRoiNum
        {
            get
            {
                return this._captureSetup.MaxRoiNum;
            }
            set
            {
                if (value != this._captureSetup.MaxRoiNum)
                {
                    this._captureSetup.MaxRoiNum = value;
                }
            }
        }

        public bool MinAreaActive
        {
            get
            {
                return this._captureSetup.MinAreaActive;
            }
            set
            {
                if (value != this._captureSetup.MinAreaActive)
                {
                    this._captureSetup.MinAreaActive = value;
                    OnPropertyChanged("MinAreaActive");
                }
            }
        }

        public int MinAreaValue
        {
            get
            {
                return this._captureSetup.MinAreaValue;
            }
            set
            {
                if (value != this._captureSetup.MinAreaValue)
                {
                    this._captureSetup.MinAreaValue = value;
                    OnPropertyChanged("MinAreaValue");
                }
            }
        }

        public int MinSnr
        {
            get
            {
                return this._captureSetup.MinSnr;
            }
            set
            {
                if (value != this._captureSetup.MinSnr)
                {
                    this._captureSetup.MinSnr = value;
                }
            }
        }

        public int NAcquireChannels
        {
            get
            {
                return this._captureSetup.NAcquireChannels;
            }
            //set;
        }

        public bool NewDFLIMDiagnosticsData
        {
            get
            {
                return this._captureSetup.NewDFLIMDiagnosticsData;
            }
            set
            {
                this._captureSetup.NewDFLIMDiagnosticsData = value;
            }
        }

        public int NumAvalableChannels
        {
            get
            {
                switch (this.DigitizerBoardName)
                {
                    case Model.CaptureSetup.DigitizerBoardNames.ATS9440:
                        return 4;

                    case Model.CaptureSetup.DigitizerBoardNames.ATS460:
                        return 2;

                    default:
                        return 4;

                }
            }
        }

        public int NumChannelsAvailableForDisplay
        {
            get;
            set;
        }

        public OrthogonalViewStatus OrthogonalViewStat
        {
            get
            {
                return this._orthogonalViewStat;
            }
            set
            {
                _orthogonalViewStat = value;
            }
        }

        public double OrthogonalViewZMultiplier
        {
            get
            {
                return _orthogonalViewZMultiplier;
            }
            set
            {
                if (_orthogonalViewZMultiplier > 0)
                {
                    _orthogonalViewZMultiplier = value;
                }
            }
        }

        public int PixelBitShiftValue
        {
            get
            {
                return CaptureSetup.PixelBitShiftValue;
            }
        }

        public bool PixelDataReady
        {
            get
            {
                return CaptureSetup.PixelDataReady;
            }
            set
            {
                CaptureSetup.PixelDataReady = value;
            }
        }

        public int PMT1Saturations
        {
            get
            {
                return (int)MVMManager.Instance["ScanControlViewModel", "PMT1Saturations", (object)false];
            }
        }

        public int PMT2Saturations
        {
            get
            {
                return (int)MVMManager.Instance["ScanControlViewModel", "PMT2Saturations", (object)false];
            }
        }

        public int PMT3Saturations
        {
            get
            {
                return (int)MVMManager.Instance["ScanControlViewModel", "PMT3Saturations", (object)false];
            }
        }

        public int PMT4Saturations
        {
            get
            {
                return (int)MVMManager.Instance["ScanControlViewModel", "PMT4Saturations", (object)false];
            }
        }

        public Visibility PMTSaturationsVisibility
        {
            get
            {
                return _pmtSaturationsVisibility;
            }
            set
            {
                _pmtSaturationsVisibility = value;
                OnPropertyChanged("PMTSaturationsVisibility");
            }
        }

        public int ProgressPercentage
        {
            get
            { return _progressPercentage; }
            set
            { _progressPercentage = value; }
        }

        public bool RebuildBitmap
        {
            get
            {
                return _rebuildBitmap;
            }
            set
            {
                _rebuildBitmap = value;
            }
        }

        public int RollOverPointIntensity0
        {
            get
            {
                return this._captureSetup.RollOverPointIntensity0;
            }
        }

        public int RollOverPointIntensity1
        {
            get
            {
                return this._captureSetup.RollOverPointIntensity1;
            }
        }

        public int RollOverPointIntensity2
        {
            get
            {
                return this._captureSetup.RollOverPointIntensity2;
            }
        }

        public int RollOverPointIntensity3
        {
            get
            {
                return this._captureSetup.RollOverPointIntensity3;
            }
        }

        public int RollOverPointX
        {
            get
            {
                return this._captureSetup.RollOverPointX;
            }
            set
            {
                int val = value;

                if (TileDisplay)
                {
                    val = val % this._captureSetup.DataWidth;
                }

                this._captureSetup.RollOverPointX = val;
            }
        }

        public int RollOverPointY
        {
            get
            {
                return this._captureSetup.RollOverPointY;
            }
            set
            {
                int val = value;

                if (TileDisplay)
                {
                    val = val % this._captureSetup.DataHeight;
                }

                this._captureSetup.RollOverPointY = val;
            }
        }

        public ICommand SaveNowCommand
        {
            get
            {
                if (this._saveNowCommand == null)
                    this._saveNowCommand = new RelayCommand(() => SaveNow());

                return this._saveNowCommand;
            }
        }

        public bool TileDisplay
        {
            get
            {
                return _tileDisplay;
            }
            set
            {
                if (_tileDisplay != value)
                {
                    _tileDisplay = value;
                    OnPropertyChanged("TileDisplay");

                    _rebuildBitmap = true;
                    OnPropertyChanged("Bitmap");
                }
            }
        }

        public bool VirtualZStack
        {
            get
            {
                return _virtualZStack;
            }
            set
            {
                _virtualZStack = value;
            }
        }

        public string WavelengthName
        {
            get
            {
                return _wavelengthName;
            }
            set
            {
                _wavelengthName = value;
                OnPropertyChanged("WavelengthName");
                OnPropertyChanged("WhitePoint");
                OnPropertyChanged("BlackPoint");
            }
        }

        public string[] WavelengthNames
        {
            get
            {
                if (LSMChannelEnable0)
                {
                    _wavelengthNames[0] = "ChanA";
                }
                if (LSMChannelEnable1)
                {
                    _wavelengthNames[1] = "ChanB";
                }
                if (LSMChannelEnable2)
                {
                    _wavelengthNames[2] = "ChanC";
                }
                if (LSMChannelEnable3)
                {
                    _wavelengthNames[3] = "ChanD";
                }
                return _wavelengthNames;
            }
        }

        public double WhitePoint0
        {
            get
            {
                return this._captureSetup.WhitePoint0;
            }
            set
            {
                if (value != this._captureSetup.WhitePoint0)
                {
                    this._captureSetup.WhitePoint0 = value;
                    OnPropertyChanged("WhitePoint0");
                    OnPropertyChanged("Bitmap");
                    if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && VirtualZStack == true)
                    {
                        UpdateOrthogonalViewImages();
                    }
                }
            }
        }

        public double WhitePoint1
        {
            get
            {
                return this._captureSetup.WhitePoint1;
            }
            set
            {
                if (value != this._captureSetup.WhitePoint1)
                {
                    this._captureSetup.WhitePoint1 = value;
                    OnPropertyChanged("WhitePoint1");
                    OnPropertyChanged("Bitmap");
                    if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && VirtualZStack == true)
                    {
                        UpdateOrthogonalViewImages();
                    }
                }
            }
        }

        public double WhitePoint2
        {
            get
            {
                return this._captureSetup.WhitePoint2;
            }
            set
            {
                if (value != this._captureSetup.WhitePoint2)
                {
                    this._captureSetup.WhitePoint2 = value;
                    OnPropertyChanged("WhitePoint2");
                    OnPropertyChanged("Bitmap");
                    if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && VirtualZStack == true)
                    {
                        UpdateOrthogonalViewImages();
                    }
                }
            }
        }

        public double WhitePoint3
        {
            get
            {
                return this._captureSetup.WhitePoint3;
            }
            set
            {
                if (value != this._captureSetup.WhitePoint3)
                {
                    this._captureSetup.WhitePoint3 = value;
                    OnPropertyChanged("WhitePoint3");
                    OnPropertyChanged("Bitmap");
                    if (_orthogonalViewStat != OrthogonalViewStatus.INACTIVE && VirtualZStack == true)
                    {
                        UpdateOrthogonalViewImages();
                    }
                }
            }
        }

        public int ZNumSteps
        {
            get
            {
                return (int)MVMManager.Instance["ZControlViewModel", "ZScanNumSteps", (object)1];
            }
        }

        public double ZoomLevel
        {
            get
            {
                return _zoomLevel;
            }
            set
            {
                if (value > MAX_ZOOM)
                {
                    _zoomLevel = MAX_ZOOM;
                }
                else if (value < MIN_ZOOM)
                {
                    _zoomLevel = MIN_ZOOM;
                }
                else
                {
                    _zoomLevel = value;
                }

                OnPropertyChanged("ZoomLevel");
            }
        }

        #endregion Properties

        #region Methods

        public WriteableBitmap Bitmap16()
        {
            ushort[] pd = CaptureSetup.GetPixelData();

            //verify pixel data is available
            if (pd == null)
            {
                return _bitmap16;
            }

            if (false == BuildChannelPalettes())
            {
                return null;
            }

            switch (CaptureSetup.GetColorChannels())
            {
                case 1:
                    {
                        // Define parameters used to create the BitmapSource.
                        PixelFormat pf = PixelFormats.Gray16;
                        int width = this._captureSetup.DataWidth;
                        int height = this._captureSetup.DataHeight;
                        int rawStride = (width * pf.BitsPerPixel + 7) / 8;

                        //create a new bitmpap when one does not exist or the size of the image changes
                        if (_bitmap16 == null)
                        {
                            _bitmap16 = new WriteableBitmap(width, height, 96, 96, pf, null);
                        }
                        else
                        {
                            if ((_bitmap16.Width != width) || (_bitmap16.Height != height) || (_bitmap16.Format != pf))
                            {
                                _bitmap16 = new WriteableBitmap(width, height, 96, 96, pf, null);
                            }
                        }

                        int w = _bitmap16.PixelWidth;
                        int h = _bitmap16.PixelHeight;
                        int widthInBytes = w;

                        if (pd.Length == (width * height))
                        {
                            //copy the pixel data into the _bitmap
                            _bitmap16.WritePixels((new Int32Rect(0, 0, w, h)), pd, rawStride, 0);
                        }

                    }
                    break;
                default:
                    {
                        // Define parameters used to create the BitmapSource.
                        PixelFormat pf = PixelFormats.Rgb48;

                        int width = this._captureSetup.DataWidth;
                        int height = this._captureSetup.DataHeight;
                        int rawStride = (width * pf.BitsPerPixel + 7) / 8;

                        //create a new bitmpap when one does not exist or the size of the image changes
                        if (_bitmap16 == null)
                        {
                            _bitmap16 = new WriteableBitmap(width, height, 96, 96, pf, null);

                        }
                        else
                        {
                            if ((_bitmap16.Width != width) || (_bitmap16.Height != height) || (_bitmap16.Format != pf))
                            {
                                _bitmap16 = new WriteableBitmap(width, height, 96, 96, pf, null);
                            }
                        }

                        int[] dataBufferOffsetIndex = GetBufferOffsetIndex();
                        int enabledChannelCount = dataBufferOffsetIndex.Length;
                        int w = _bitmap16.PixelWidth;
                        int h = _bitmap16.PixelHeight;

                        if ((pd.Length / CaptureSetup.GetColorChannels()) == (width * height))
                        {
                            int buffSize = pd.Length / CaptureSetup.GetColorChannels();

                            short[] pdData = new short[buffSize * 3];

                            int i = 0;
                            for (int j = 0; j < buffSize; i += 3, j++)
                            {
                                short maxRed = 0;
                                short maxGreen = 0;
                                short maxBlue = 0;
                                pdData[i] = 0;
                                pdData[i + 1] = 0;
                                pdData[i + 2] = 0;
                                const double LUT_MAX_VAL = 255.0;

                                for (int k = 0; k < enabledChannelCount; k++)
                                {
                                    int valRaw = (pd[j + dataBufferOffsetIndex[k] * buffSize]);
                                    int valRawByte = Math.Min(255, ((pd[j + dataBufferOffsetIndex[k] * buffSize])) >> 6);

                                    double percentRed = (CaptureSetup.ChannelLuts[dataBufferOffsetIndex[k]][valRawByte].R / LUT_MAX_VAL);
                                    double percentGreen = (CaptureSetup.ChannelLuts[dataBufferOffsetIndex[k]][valRawByte].G / LUT_MAX_VAL);
                                    double percentBlue = (CaptureSetup.ChannelLuts[dataBufferOffsetIndex[k]][valRawByte].B / LUT_MAX_VAL);

                                    double total = percentRed + percentGreen + percentBlue;

                                    if (total > 0)
                                    {
                                        percentRed = percentRed / total;
                                        percentGreen = percentGreen / total;
                                        percentBlue = percentBlue / total;

                                        pdData[i] = maxRed = Math.Max(maxRed, (short)(valRaw * percentRed));
                                        pdData[i + 1] = maxGreen = Math.Max(maxGreen, (short)(valRaw * percentGreen));
                                        pdData[i + 2] = maxBlue = Math.Max(maxBlue, (short)(valRaw * percentBlue));
                                    }

                                }
                            }

                            //copy the pixel data into the bitmap
                            _bitmap16.WritePixels(new Int32Rect(0, 0, w, h), pdData, rawStride, 0);
                        }
                    }
                    break;
            }

            if (null != ImageDataChanged)
            {
                ImageDataChanged(true);
            }

            return _bitmap16;
        }

        public void FireColorMappingChangedAction(bool bVal)
        {
            ColorMappingChanged(bVal);
        }

        public int[] GetBufferOffsetIndex()
        {
            //calculate the raw data buffer offset index for each of the
            //selected display channels
            List<int> dataBufferOffsetIndex = new List<int>();

            int enabledChannelCount = 0;
            for (int i = 0; i < CaptureSetup.MAX_CHANNELS; i++)
            {
                //if the channgel is enabled store the index
                bool isEnabled = false;
                switch (i)
                {
                    case 0: isEnabled = LSMChannelEnable0; break;
                    case 1: isEnabled = LSMChannelEnable1; break;
                    case 2: isEnabled = LSMChannelEnable2; break;
                    case 3: isEnabled = LSMChannelEnable3; break;
                }

                if (isEnabled)
                {
                    dataBufferOffsetIndex.Add(i);
                    enabledChannelCount++;
                }
            }
            return dataBufferOffsetIndex.ToArray();
        }

        public short GetImageProcessImagePixel(int width, int height)
        {
            return this._captureSetup.GetImageProcessData(width, height);
        }

        public void InitiOrthogonalBuffers()
        {
            PixelFormat pf = PixelFormats.Rgb24;
            int step = pf.BitsPerPixel / 8;
            int totalNumOfZstack = (int)MVMManager.Instance["ZControlViewModel", "ZScanNumSteps", (object)1];

            int width = this._captureSetup.DataWidth;
            int height = this._captureSetup.DataHeight;

            int pdXZ_DataLength = PixelFormats.Rgb24.BitsPerPixel / 8 * this._captureSetup.DataWidth * totalNumOfZstack;
            int pdYZ_DataLength = PixelFormats.Rgb24.BitsPerPixel / 8 * this._captureSetup.DataHeight * totalNumOfZstack;

            if (_bitmapXZ == null || _pdXZ == null || _pdXZ.Length != pdXZ_DataLength)
            {
                _pdXZ = new byte[pdXZ_DataLength];
            }

            if (_bitmapYZ == null || _pdYZ == null || _pdYZ.Length != pdYZ_DataLength)
            {
                _pdYZ = new byte[pdYZ_DataLength];
            }
        }

        public void InitOrthogonalView()
        {
            UpdateOrthogonalView();
        }

        public void LoadColorImageSettings()
        {
            if (true == BuildChannelPalettes())
            {

                for (int i = 0; i < CaptureSetup.MAX_CHANNELS; i++)
                {
                    Brush brush;

                    const int PALETTE_SIZE = 256;

                    double luminance = (0.2126 * CaptureSetup.ChannelLuts[i][PALETTE_SIZE - 1].R + 0.7152 * CaptureSetup.ChannelLuts[i][PALETTE_SIZE - 1].G + 0.0722 * CaptureSetup.ChannelLuts[i][PALETTE_SIZE - 1].B);

                    //if the color is too bright it will not
                    //display on a white background
                    //substitute gray if the color is too bright
                    if (luminance > 240)
                    {
                        brush = new SolidColorBrush(Colors.Gray);
                    }
                    else
                    {
                        brush = new SolidColorBrush(CaptureSetup.ChannelLuts[i][PALETTE_SIZE - 1]);
                    }

                    switch (i)
                    {
                        case 0: LSMChannelColor0 = brush; break;
                        case 1: LSMChannelColor1 = brush; break;
                        case 2: LSMChannelColor2 = brush; break;
                        case 3: LSMChannelColor3 = brush; break;
                    }
                    if (null != _lineProfile) _lineProfile.ColorAssigment[i] = ((SolidColorBrush)brush).Color;
                }
            }

            XmlNodeList wavelengths = HardwareDoc.GetElementsByTagName("Wavelength");

            if (wavelengths.Count > 0)
            {
                string str = string.Empty;
                string wlString = string.Empty;

                for (int i = 0; i < wavelengths.Count; i++)
                {
                    string bp = string.Empty;
                    string wp = string.Empty;
                    string autoTog = string.Empty;
                    string logTog = string.Empty;

                    XmlManager.GetAttribute(wavelengths[i], HardwareDoc, "bp", ref bp);
                    XmlManager.GetAttribute(wavelengths[i], HardwareDoc, "wp", ref wp);
                    XmlManager.GetAttribute(wavelengths[i], HardwareDoc, "AutoSta", ref autoTog);
                    XmlManager.GetAttribute(wavelengths[i], HardwareDoc, "LogSta", ref logTog);

                    switch (i)
                    {
                        case 0:
                            BlackPoint0 = Convert.ToDouble(bp);
                            WhitePoint0 = Convert.ToDouble(wp);
                            AutoManualTog1Checked = (string.Empty != autoTog && "1" == autoTog) ? true : false;
                            LogScaleEnabled0 = (string.Empty != logTog && "1" == logTog) ? true : false;
                            break;
                        case 1:
                            BlackPoint1 = Convert.ToDouble(bp);
                            WhitePoint1 = Convert.ToDouble(wp);
                            AutoManualTog2Checked = (string.Empty != autoTog && "1" == autoTog) ? true : false;
                            LogScaleEnabled1 = (string.Empty != logTog && "1" == logTog) ? true : false;
                            break;
                        case 2:
                            BlackPoint2 = Convert.ToDouble(bp);
                            WhitePoint2 = Convert.ToDouble(wp);
                            AutoManualTog3Checked = (string.Empty != autoTog && "1" == autoTog) ? true : false;
                            LogScaleEnabled2 = (string.Empty != logTog && "1" == logTog) ? true : false;
                            break;
                        case 3:
                            BlackPoint3 = Convert.ToDouble(bp);
                            WhitePoint3 = Convert.ToDouble(wp);
                            AutoManualTog4Checked = (string.Empty != autoTog && "1" == autoTog) ? true : false;
                            LogScaleEnabled3 = (string.Empty != logTog && "1" == logTog) ? true : false;
                            break;
                    }
                }

                MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
            }
        }

        public void SaveImage(String filename, int filterIndex)
        {
            if (_bitmap == null)
            {
                return;
            }

            //find out if tiff compression is necessary:
            bool doCompression = false;
            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/TIFFCompressionEnable");
            if (node != null)
            {
                string str = string.Empty;
                if (true == XmlManager.GetAttribute(node, ApplicationDoc, "value", ref str))
                {
                    doCompression = ("1" == str) ? true : false;
                }
            }

            FileStream stream = new FileStream(filename, FileMode.Create);

            switch (filterIndex)
            {
                case 1:
                    {
                        //8 bit tiff image save
                        TiffBitmapEncoder encoder = new TiffBitmapEncoder();
                        encoder.Frames.Add(BitmapFrame.Create(_bitmap));
                        encoder.Save(stream);
                    }
                    break;
                case 2:
                    {
                        //16 bit tiff image save
                        Bitmap16();
                        TiffBitmapEncoder encoder = new TiffBitmapEncoder();
                        encoder.Compression = (doCompression) ? TiffCompressOption.Lzw : TiffCompressOption.None;
                        BitmapMetadata bmd = new BitmapMetadata("tiff");
                        bmd.SetQuery("/ifd/{uint=270}", CreateOMEMetadata(Convert.ToInt32(_bitmap16.Width), Convert.ToInt32(_bitmap16.Height)));
                        encoder.Frames.Add(BitmapFrame.Create(_bitmap16, null, bmd, null));
                        encoder.Save(stream);
                    }
                    break;
                case 3:
                    {
                        //8 bit jpeg image save
                        JpegBitmapEncoder jpgEncoder = new JpegBitmapEncoder();
                        jpgEncoder.Frames.Add(BitmapFrame.Create(_bitmap));
                        jpgEncoder.Save(stream);
                    }
                    break;
            }

            stream.Close();
        }

        public void SaveNow()
        {
            string saveStr = this._captureSetup.GetUniqueSnapshotFilename();
            SaveImage(saveStr, 2);
        }

        public bool UpdateChannelData(string[] fileNames, int zIndexToRead, int tIndexToRead)
        {
            int pixelX = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)0];
            int pixelY = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)0];
            int zSteps = (int)MVMManager.Instance["ZControlViewModel", "ZScanNumSteps", (object)1];
            byte enabledChannels = (byte)ColorChannels;
            return _captureSetup.UpdateChannelData(fileNames, enabledChannels, 4, zIndexToRead, tIndexToRead, pixelX, pixelY, zSteps, GetRawContainsDisabledChannels());
        }

        public void UpdateOrthogonalView(bool displaySplash = true)
        {
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;
            _bwOrthogonalImageLoaderDone = false;
            ProgressPercentage = 0;

            if (displaySplash)
            {
                _splashOrthogonalView = new CaptureSetupDll.View.SplashScreen();
            }
            try
            {
                if (displaySplash)
                {
                    _splashOrthogonalView.DisplayText = "Please wait while loading images ...";
                    _splashOrthogonalView.ShowInTaskbar = false;
                    _splashOrthogonalView.Owner = Application.Current.MainWindow;
                    _splashOrthogonalView.Show();
                    _splashOrthogonalView.CancelSplashProgress += delegate (object sender, EventArgs e)
                    {
                        splashWkr.CancelAsync();
                    };
                }
                System.Windows.Threading.Dispatcher spDispatcher = null;

                if (displaySplash)
                {
                    //get dispatcher to update the contents that was created on the UI thread:
                   spDispatcher = _splashOrthogonalView.Dispatcher;
                }

                splashWkr.DoWork += delegate (object sender, DoWorkEventArgs e)
                {
                    PixelFormat pf = PixelFormats.Rgb24;
                    int step = pf.BitsPerPixel / 8;
                    int totalNumOfZstack = (int)MVMManager.Instance["ZControlViewModel", "ZScanNumSteps", (object)1];

                    _captureSetup.InitializeDataBufferOffset();
                    _captureSetup.InitializePixelDataLut();

                    InitiOrthogonalBuffers();

                    if ((null == _tiffBufferArray) || (_tiffBufferArray.Length != totalNumOfZstack))
                    {
                        _tiffBufferArray = new ushort[totalNumOfZstack][];
                    }
                    for (int i = 0; i < totalNumOfZstack; i++)
                    {
                        if (splashWkr.CancellationPending == true)
                        {
                            e.Cancel = true;
                            break;
                        }

                        //load the images
                        string[] fileNames = GetZImageFileNames(i + 1); // get first image
                        bool result = UpdateChannelData(fileNames, i, 1);

                        if (result)
                        {

                            if (null == _tiffBufferArray[i] || _tiffBufferArray[i].Length != _captureSetup.ZPreviewPixelData.Length)
                            {
                                _tiffBufferArray[i] = new ushort[_captureSetup.ZPreviewPixelData.Length];
                            }
                            Buffer.BlockCopy(_captureSetup.ZPreviewPixelData, 0, _tiffBufferArray[i], 0, _captureSetup.ZPreviewPixelData.Length * sizeof(short));

                            CreateOrthogonalBitmap(i, totalNumOfZstack);
                            //report progress:
                            _progressPercentage = (int)(i * 100 / totalNumOfZstack);
                            //create a new delegate for updating our progress text
                            UpdateProgressDelegate update = new UpdateProgressDelegate(UpdateProgressTextOrthogonal);

                            if (displaySplash)
                            {
                                //invoke the dispatcher and pass the percentage
                                spDispatcher.BeginInvoke(update, ProgressPercentage);
                            }
                        }
                    }
                    //_maxChannel = CaptureSetup.MAX_CHANNELS;
                    //_updateVirtualStack = false;

                };
                splashWkr.RunWorkerCompleted += delegate (object sender, RunWorkerCompletedEventArgs e)
                {
                    if (displaySplash)
                    {
                        _splashOrthogonalView.Close();
                    }
                    // App inherits from Application, and has a Window property called MainWindow
                    // and a List<Window> property called OpenWindows.
                    Application.Current.MainWindow.Activate();
                    _bwOrthogonalImageLoaderDone = true;

                    if (e.Cancelled == false)
                    {
                        OnPropertyChanged("BitmapXZ");
                        OnPropertyChanged("BitmapYZ");
                    }

                    if (OrthogonalViewImagesLoaded != null)
                    {
                        OrthogonalViewImagesLoaded(e.Cancelled);
                    }

                    bool continuousZStackPreview = (bool)MVMManager.Instance["ZControlViewModel", "EnableContinuousZStackPreview", (object)false];

                    //if continuous zstack is checked then continuosly update, if it has been manually stopped then don't start it again
                    if (continuousZStackPreview == true && IsOrthogonalViewChecked && !_stackPreviewStopped)
                    {
                        //need this flag to not allow the xml persistance to close the progress window
                        _startingContinuousZStackPreview = true;
                        ICommand zStackPreviewCommand = (ICommand)MVMManager.Instance["ZControlViewModel", "PreviewZStackCommand", (object)null];
                        if (null != zStackPreviewCommand)
                        {
                            zStackPreviewCommand.Execute(null);
                        }
                        _startingContinuousZStackPreview = false;
                    }
                    else if (!continuousZStackPreview)
                    {
                        CloseProgressWindow();
                    }
                };
                splashWkr.RunWorkerAsync();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ImageReview Orthogonal Images Loading Error: " + ex.Message);
                splashWkr.CancelAsync();
            }
        }

        public void UpdateOrthogonalViewImages()
        {
            //Set Click time-Gap
            TimeSpan ts;
            ts = DateTime.Now - _lastOrthogonalViewUpdateTime;
            _captureSetup.InitializeDataBufferOffset();
            _captureSetup.InitializePixelDataLut();

            InitiOrthogonalBuffers();
            if (ts.TotalSeconds > 0.01 && _bwOrthogonalImageLoaderDone == true)
            {
                if (VirtualZStack == true)
                {
                    int totalNumOfZstack = (int)MVMManager.Instance["ZControlViewModel", "ZScanNumSteps", (object)1];

                    if ((null == _tiffBufferArray) || (_tiffBufferArray.Length != totalNumOfZstack))
                    {
                        return;
                    }

                    for (int i = 0; i < totalNumOfZstack; i++)
                    {
                        if (null == _tiffBufferArray[i])
                        {
                            return;
                        }
                        CreateOrthogonalBitmap(i, totalNumOfZstack); // create Bitmap
                    }
                    OnPropertyChanged("BitmapXZ");
                    OnPropertyChanged("BitmapYZ");
                }
                else
                {
                    UpdateOrthogonalView();
                }
                _lastOrthogonalViewUpdateTime = DateTime.Now;
            }
        }

        public void UpdateProgressTextOrthogonal(int percentage)
        {
            //set our progress dialog text and value
            _splashOrthogonalView.ProgressText = string.Format("{0}%", percentage.ToString());
            _splashOrthogonalView.ProgressValue = percentage;
        }

        private void ApplyOrRevertFastFocusParams(bool isLive)
        {
            if (false == isLive)
            {
                _preFFPixelX = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)0];
                _preFFPixelY = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)0];
                _preFFScanMode = (int)MVMManager.Instance["ScanControlViewModel", "LSMScanMode", (object)0];
                _preFFDwellTime = (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime", (object)1.0];
                _preFFAreaMode = (int)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0];
                _preFFAverageMode = (int)MVMManager.Instance["ScanControlViewModel", "LSMSignalAverage", (object)0];
                _preLsmInterleaveScanMode = (int)MVMManager.Instance["ScanControlViewModel", "LSMInterleaveScan", (object)0];

                MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0] = (int)ICamera.LSMAreaMode.SQUARE;
                MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)0] = 512;
                MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)0] = 512;
                MVMManager.Instance["ScanControlViewModel", "LSMScanMode"] = 0;
                MVMManager.Instance["ScanControlViewModel", " LSMPixelDwellTime"] = 2.0;
                MVMManager.Instance["ScanControlViewModel", "LSMSignalAverage"] = 0;
                MVMManager.Instance["ScanControlViewModel", "LSMInterleaveScan"] = 1;
            }
            else
            {
                MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0] = _preFFAreaMode;
                MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)0] = _preFFPixelX;
                MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)0] = _preFFPixelY;
                MVMManager.Instance["ScanControlViewModel", "LSMScanMode"] = _preFFScanMode;
                MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = _preFFDwellTime;
                MVMManager.Instance["ScanControlViewModel", "LSMSignalAverage"] = _preFFAverageMode;
                MVMManager.Instance["ScanControlViewModel", "LSMInterleaveScan"] = _preLsmInterleaveScanMode;
            }
        }

        private bool BuildChannelPalettes()
        {
            bool ret = false;

            if (null == HardwareDoc)
            {
                return ret;
            }

            XmlNodeList ndList = HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/*");

            string str = string.Empty;
            string chanName = string.Empty;

            for (int j = 0; j < CaptureSetup.MAX_CHANNELS; j++)
            {
                switch (j)
                {
                    case 0: chanName = "ChanA"; break;
                    case 1: chanName = "ChanB"; break;
                    case 2: chanName = "ChanC"; break;
                    case 3: chanName = "ChanD"; break;
                }

                for (int i = 0; i < ndList.Count; i++)
                {
                    if (XmlManager.GetAttribute(ndList[i], HardwareDoc, "name", ref str))
                    {
                        if (str.Contains(chanName))
                        {
                            str = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\luts\\" + ndList[i].Name + ".txt";

                            //if the current lut for the channel has not changed
                            //continue on
                            if ((_currentChannelsLutFiles.Count > 0) && (_currentChannelsLutFiles[j].Equals(str)))
                            {
                                continue;
                            }

                            if (File.Exists(str))
                            {
                                StreamReader fs = new StreamReader(str);
                                string line;
                                int counter = 0;
                                try
                                {
                                    while ((line = fs.ReadLine()) != null)
                                    {
                                        string[] split = line.Split(',');

                                        if (split[0] != null)
                                        {
                                            if (split[1] != null)
                                            {
                                                if (split[2] != null)
                                                {
                                                    CaptureSetup.ChannelLuts[j][counter] = Color.FromRgb(Convert.ToByte(split[0]), Convert.ToByte(split[1]), Convert.ToByte(split[2]));
                                                }
                                            }
                                        }
                                        counter++;
                                    }
                                }
                                catch (Exception ex)
                                {
                                    string msg = ex.Message;
                                }

                                fs.Close();

                                _currentChannelsLutFiles[j] = str;
                            }
                        }
                    }
                }
            }

            ret = true;
            return ret;
        }

        private BitmapPalette BuildPalette()
        {
            List<Color> colors = new List<Color>();

            string chanName = "ChanA";
            if (LSMChannelEnable0)
                chanName = "ChanA";
            if (LSMChannelEnable1)
                chanName = "ChanB";
            if (LSMChannelEnable2)
                chanName = "ChanC";
            if (LSMChannelEnable3)
                chanName = "ChanD";

            XmlNodeList ndList = HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/*");

            string str = string.Empty;

            for (int i = 0; i < ndList.Count; i++)
            {
                if (XmlManager.GetAttribute(ndList[i], HardwareDoc, "name", ref str))
                {
                    if (str.Contains(chanName))
                    {
                        str = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\luts\\" + ndList[i].Name + ".txt";

                        if (File.Exists(str))
                        {
                            StreamReader fs = new StreamReader(str);
                            string line;
                            int counter = 0;
                            try
                            {
                                while ((line = fs.ReadLine()) != null)
                                {
                                    string[] split = line.Split(',');

                                    if (split[0] != null)
                                    {
                                        if (split[1] != null)
                                        {
                                            if (split[2] != null)
                                            {
                                                if (256 == colors.Count)
                                                {
                                                    break;
                                                }
                                                colors.Add(Color.FromRgb(Convert.ToByte(split[0]), Convert.ToByte(split[1]), Convert.ToByte(split[2])));
                                            }
                                        }
                                    }
                                    counter++;
                                }
                            }
                            catch (Exception ex)
                            {
                                string msg = ex.Message;
                            }

                            fs.Close();

                            _currentLutFile = str;
                        }
                    }
                }
            }
            return new BitmapPalette(colors);
        }

        private BitmapPalette BuildPaletteContour()
        {
            List<Color> colors = new List<Color>();
            for (int i = 0; i < roiColorTables.Length; i++)
            {
                colors.Add(roiColorTables[i]);
            }
            return new BitmapPalette(colors);
        }

        private BitmapPalette BuildPaletteGrayscale()
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

        private void CaptureNow()
        {
            MVMManager.Instance.UpdateMVMXMLSettings(ref MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS]);
            MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);

            Command command = new Command();
            command.Message = "RunSample LS";
            command.CommandGUID = new Guid("30db4357-7508-46c9-84eb-3ca0900aa4c5");
            command.Payload = new List<string>();
            command.Payload.Add("RunImmediately");

            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
        }

        private void ChangeColorSettings()
        {
            LUTSettings dlg = new LUTSettings();

            dlg.HardwareDoc = HardwareDoc;

            dlg.GrayscaleForSingleChannel = GrayscaleForSingleChannel;

            if (true == dlg.ShowDialog())
            {
                BuildChannelPalettes();

                GrayscaleForSingleChannel = dlg.GrayscaleForSingleChannel;

                ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/GrayscaleForSingleChannel");

                if (ndList.Count > 0)
                {
                    string str = (true == GrayscaleForSingleChannel) ? "1" : "0";

                    XmlManager.SetAttribute(ndList[0], ApplicationDoc, "value", str);
                }

                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);

                for (int i = 0; i < CaptureSetup.MAX_CHANNELS; i++)
                {
                    Brush brush;

                    const int PALETTE_SIZE = 256;

                    double luminance = (0.2126 * CaptureSetup.ChannelLuts[i][PALETTE_SIZE - 1].R + 0.7152 * CaptureSetup.ChannelLuts[i][PALETTE_SIZE - 1].G + 0.0722 * CaptureSetup.ChannelLuts[i][PALETTE_SIZE - 1].B);

                    //if the color is too bright it will not
                    //display on a white background
                    //substitute gray if the color is too bright
                    if (luminance > 240)
                    {
                        brush = new SolidColorBrush(Colors.Gray);
                    }
                    else
                    {
                        brush = new SolidColorBrush(CaptureSetup.ChannelLuts[i][PALETTE_SIZE - 1]);
                    }

                    switch (i)
                    {
                        case 0: LSMChannelColor0 = brush; break;
                        case 1: LSMChannelColor1 = brush; break;
                        case 2: LSMChannelColor2 = brush; break;
                        case 3: LSMChannelColor3 = brush; break;
                    }
                    if (null != _lineProfile) _lineProfile.ColorAssigment[i] = ((SolidColorBrush)brush).Color;
                }

                CaptureSetup.PaletteChanged = true;
                OnPropertyChanged("Bitmap");

                FireColorMappingChangedAction(true);
            }
        }

        private string CreateOMEMetadata(int width, int height)
        {
            string tagData = "<?xml version=\"1.0\"?><OME xmlns=\"http://www.openmicroscopy.org/Schemas/OME/2008-02\" xmlns:CA=\"http://www.openmicroscopy.org/Schemas/CA/2008-02\" xmlns:STD=\"http://www.openmicroscopy.org/Schemas/STD/2008-02\" xmlns:Bin=\"http://www.openmicroscopy.org/Schemas/BinaryFile/2008-02\" xmlns:SPW=\"http://www.openmicroscopy.org/Schemas/SPW/2008-02\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.openmicroscopy.org/Schemas/OME/2008-02 http://www.openmicroscopy.org/Schemas/OME/2008-02/ome.xsd\">";

            tagData += "<Image><Pixels";
            tagData += " DimensionOrder=";
            tagData += "\"XYCZT\"";
            tagData += "ID=\"Pixels:0\"";
            tagData += " PhysicalSizeX=\"1.0\"";
            tagData += " PhysicalSizeY=\"1.0\"";
            tagData += " PhysicalSizeZ=\"1.0\"";
            tagData += " SizeC=";
            tagData += "\"1\"";
            tagData += " SizeT=";
            tagData += "\"1\"";
            tagData += " SizeX=";
            tagData += string.Format("\"{0}\"", width);
            tagData += " SizeY=";
            tagData += string.Format("\"{0}\"", height);
            tagData += " SizeZ=";
            tagData += "\"1\"";
            tagData += " TimeIncrement=";
            tagData += "\"1\"";
            tagData += " Type=";
            tagData += "\"uint16\"";
            tagData += ">";
            tagData += "<Channel ID=";
            tagData += "\"Channel:0:0\"";
            tagData += "SamplesPerPixel=\"1\"";
            tagData += "/>";
            tagData += "<BinData BigEndian=\"false\" Length = \"0\" xmlns=\"http://www.openmicroscopy.org/Schemas/BinaryFile/2010-06\"/>";
            tagData += "<TiffData FirstC=\"0\" FirstT=\"0\" FirstZ=\"0\" IFD=\"0\" PlaneCount=\"1\">";
            tagData += "</TiffData>";
            tagData += "</Pixels>";
            tagData += "</Image>";
            tagData += "</OME>";

            return tagData;
        }

        private void CreateOrthogonalBitmap(int index, int totalNumOfZstack)
        {
            PixelFormat pf = PixelFormats.Rgb24;
            int step = pf.BitsPerPixel / 8;

            int length = this._captureSetup.DataWidth * this._captureSetup.DataHeight;
            ushort[] position = new ushort[CaptureSetup.MAX_CHANNELS]; //store the point of grayscale image which used to calculate the RGB value
            //Create the XZ orthogonal view
            for (int j = 0; j < this._captureSetup.DataWidth; j++)
            {
                for (int k = 0; k < this._captureSetup.DataBufferOffsetIndex.Count; k++)
                {
                    if (VirtualZStack)
                    {
                        position[k] = _tiffBufferArray[index][_captureSetup.DataBufferOffsetIndex[k] * length + Convert.ToInt32(Math.Floor(BitmapPoint.Y)) * _captureSetup.DataWidth + j];
                    }
                    else
                    {
                        position[k] = _captureSetup.PixelData[_captureSetup.DataBufferOffsetIndex[k] * length + Convert.ToInt32(Math.Floor(BitmapPoint.Y)) * _captureSetup.DataWidth + j];
                    }

                }
                byte[] color = _captureSetup.UpdataPixelDatabyte(position); //calculate the RGB value by current color mapping table
                for (int k = 0; k < 3; k++)
                {
                    _pdXZ[index * this._captureSetup.DataWidth * step + j * 3 + k] = color[k]; // assign the RGB value
                }
            }
            //Create the YZ orthogonal view
            for (int j = 0; j < this._captureSetup.DataHeight; j++)
            {
                for (int k = 0; k < this._captureSetup.DataBufferOffsetIndex.Count; k++)
                {
                    if (VirtualZStack)
                    {
                        position[k] = _tiffBufferArray[index][_captureSetup.DataBufferOffsetIndex[k] * length + j * _captureSetup.DataWidth + Convert.ToInt32(Math.Floor(BitmapPoint.X))];
                    }
                    else
                    {
                        position[k] = this._captureSetup.PixelData[_captureSetup.DataBufferOffsetIndex[k] * length + j * _captureSetup.DataWidth + Convert.ToInt32(Math.Floor(BitmapPoint.X))];
                    }

                }
                byte[] color = _captureSetup.UpdataPixelDatabyte(position); //calculate the RGB value by current color mapping table
                for (int k = 0; k < 3; k++)
                {
                    _pdYZ[j * totalNumOfZstack * step + index * step + k] = color[k]; // assign the RGB value
                }
            }
        }

        /// <summary>
        /// Expands all histograms to their maximum size
        /// </summary>
        void ExpandAllHistograms()
        {
            HistogramHeight1 = MAX_HISTOGRAM_HEIGHT;
            HistogramWidth1 = MAX_HISTOGRAM_WIDTH;
            HistogramHeight2 = MAX_HISTOGRAM_HEIGHT;
            HistogramWidth2 = MAX_HISTOGRAM_WIDTH;
            HistogramHeight3 = MAX_HISTOGRAM_HEIGHT;
            HistogramWidth3 = MAX_HISTOGRAM_WIDTH;
            HistogramHeight4 = MAX_HISTOGRAM_HEIGHT;
            HistogramWidth4 = MAX_HISTOGRAM_WIDTH;
        }

        private bool GetRawContainsDisabledChannels()
        {
            XmlDocument experimentDoc = new XmlDocument();
            if (File.Exists(ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "ZStackCache\\Experiment.xml"))
            {
                experimentDoc.Load(ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "ZStackCache\\Experiment.xml");
                XmlNodeList ndList = experimentDoc.SelectNodes("/ThorImageExperiment/RawData");
                if (ndList.Count > 0)
                {
                    string value = "";
                    XmlManager.GetAttribute(ndList[0], experimentDoc, "onlyEnabledChannels", ref value);
                    if (value == "1")
                        return false;
                }
            }
            return true;
        }

        // Get the file name
        private string[] GetZImageFileNames(int zValue)
        {
            string[] fileNames = new string[CaptureSetup.MAX_CHANNELS];

            int sampleSiteIndex = 1;
            int subIndex = 1;
            int timeIndex = 1;

            string experimentFolderPath = ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "ZStackCache";
            string imgNameFormat = ImageNameFormat;
            XmlNodeList ndListHW = HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

            if (null != WavelengthNames)
            {
                for (int i = 0; i < WavelengthNames.Length; i++)
                {
                    if ((experimentFolderPath != null))
                    {
                        StringBuilder sbTemp = new StringBuilder();
                        CaptureFile imageType = CaptureFile.FILE_TIFF; // Hardcode image type to TIFF, this is the type saved for ZStacked preview
                        switch (imageType)
                        {
                            case CaptureFile.FILE_BIG_TIFF:
                                {
                                    String temp = string.Format("{0}", "Image");
                                    DirectoryInfo di = new DirectoryInfo(experimentFolderPath);
                                    FileInfo[] fi = di.GetFiles(temp + "*.tif*");
                                    if (fi == null || fi.Length == 0) return null;
                                    sbTemp.AppendFormat(fi[0].FullName);
                                }
                                break;
                            case CaptureFile.FILE_RAW:
                                {
                                    String temp = string.Format("{0}{1}{2}",
                                                        "Image",
                                                        "_" + sampleSiteIndex.ToString(imgNameFormat),
                                                        "_" + subIndex.ToString(imgNameFormat));
                                    DirectoryInfo di = new DirectoryInfo(experimentFolderPath);
                                    FileInfo[] fi = di.GetFiles(temp + "*.raw*");
                                    if (fi == null || fi.Length == 0) return null;
                                    sbTemp.AppendFormat(fi[0].FullName);
                                }
                                break;
                            case CaptureFile.FILE_TIFF:
                                {
                                    /*if (CaptureModes.HYPERSPECTRAL == CaptureMode)
                                    {
                                        sbTemp.AppendFormat("{0}{1}{2}{3}{4}{5}",
                                                                experimentFolderPath + "\\",
                                                                WavelengthNames[i],
                                                                "_" + timeIndex.ToString(imgNameFormat),
                                                                "_" + subIndex.ToString(imgNameFormat),
                                                                "_" + zValue.ToString(imgNameFormat),
                                                                "_" + sampleSiteIndex.ToString(imgNameFormat) + ".tif");
                                    }
                                    else*/
                                    {
                                        sbTemp.AppendFormat("{0}{1}{2}{3}{4}{5}",
                                                                experimentFolderPath + "\\",
                                                                WavelengthNames[i],
                                                                "_" + sampleSiteIndex.ToString(imgNameFormat),
                                                                "_" + subIndex.ToString(imgNameFormat),
                                                                "_" + zValue.ToString(imgNameFormat),
                                                                "_" + timeIndex.ToString(imgNameFormat) + ".tif");
                                    }
                                }
                                break;
                            default:
                                break;
                        }

                        string strTemp = sbTemp.ToString();

                        for (int j = 0; j < ndListHW.Count; j++)
                        {
                            if (null != WavelengthNames[i])
                            {
                                if (WavelengthNames[i].Equals(ndListHW[j].Attributes["name"].Value) && _channelEnable[j])
                                {
                                    fileNames[j] = strTemp;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            return fileNames;
        }

        private void RemoteCaptureNow()
        {
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                //Set the property to start acquisition remotely with the required experiment path
                MVMManager.Instance["RemoteIPCControlViewModel", "StartAcquisition"] = "C:\\temp\\exp01";
            }
        }

        /// <summary>
        /// Shrinks all histograms to their minimum size
        /// </summary>
        void ShrinkAllHistograms()
        {
            HistogramHeight1 = MIN_HISTOGRAM_HEIGHT;
            HistogramWidth1 = MIN_HISTOGRAM_WIDTH;
            HistogramHeight2 = MIN_HISTOGRAM_HEIGHT;
            HistogramWidth2 = MIN_HISTOGRAM_WIDTH;
            HistogramHeight3 = MIN_HISTOGRAM_HEIGHT;
            HistogramWidth3 = MIN_HISTOGRAM_WIDTH;
            HistogramHeight4 = MIN_HISTOGRAM_HEIGHT;
            HistogramWidth4 = MIN_HISTOGRAM_WIDTH;
        }

        private void UpdateChannelData()
        {
            int chan = 0;
            for (int i = 0; i < _channelEnable.Length; ++i)
            {
                chan |= Convert.ToInt32(_channelEnable[i]) << i;
            }

            bool oldIsSingleChan = this.IsSingleChannel;
            _captureSetup.LSMChannelOrtho = chan;

            //only build pallete when changing from single channel to multichannel or viceversa
            if (this.IsSingleChannel != oldIsSingleChan)
            {
                BuildChannelPalettes();
            }
        }

        #endregion Methods
    }
}