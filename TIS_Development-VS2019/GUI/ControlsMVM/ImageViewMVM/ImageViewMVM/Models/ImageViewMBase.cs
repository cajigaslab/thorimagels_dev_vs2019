namespace ImageViewMVM.Models
{
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.WindowsRuntime;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    using Microsoft.Win32;

    using ThorLogging;

    using ThorSharedTypes;

    using ImageViewMVM.Shared;

    #region Enumerations

    public enum ImageViewType
    {
        CaptureSetup,
        Capture,
        Review
    }

    #endregion Enumerations

    #region Delegates

    public delegate int GetPixelInfo(int channel, int groupIndex);

    #endregion Delegates

    public abstract class ImageViewMBase
    {
        #region Fields

        public const int MAX_CHANNELS = 4;

        public double _zStepSizeUM = 0;

        protected int _bitsPerPixel = DEFAULT_BITS_PER_PIXEL;
        protected bool _chanEnableChanged = false;
        protected volatile bool _dataReadyToCopy = false;
        protected FrameData _frameData;
        protected bool _frameDataSet = false;
        protected ImageViewType _imageReviewType = ImageViewType.CaptureSetup;
        protected ObservableCollection<ObservableCollection<CompoundImage>> _imagesGrid;
        protected int _mROIPriorityIndex = 0;
        protected bool _mROISpatialDisplaybleEnable = true;
        protected bool _palletChanged = false;
        protected volatile bool _redisplayAllROIs = false;
        protected bool _runBitmapBuildingThread = false;
        protected bool _runCopyDataThread = false;
        protected volatile bool _tileViewToggled = false;
        protected volatile bool _updateBitmap = false;
        protected volatile bool _updatePixelData = false;

        const int DEFAULT_BITS_PER_PIXEL = 14;
        const int DEFAULT_CHANNEL_GROUP = 0;
        const double LUT_MAX = 255;
        const double LUT_MIN = 0;
        const int LUT_SIZE = 256;
        const int PIXEL_DATA_HISTOGRAM_SIZE = 256;

        static readonly float[] _dflimColorMap = new float[] { 0.0f, 0.0f, 0.9125f, 0.0f, 0.0f, 0.925f, 0.0f, 0.0f, 0.9375f, 0.0f, 0.0f, 0.95f, 0.0f, 0.0f, 0.9625f, 0.0f, 0.0f, 0.975f, 0.0f, 0.0f, 0.9875f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0125f, 1.0f, 0.0f, 0.025f, 1.0f, 0.0f, 0.0375f, 1.0f, 0.0f, 0.05f, 1.0f, 0.0f, 0.0625f, 1.0f, 0.0f, 0.075f, 1.0f, 0.0f, 0.0875f, 1.0f, 0.0f, 0.1f, 1.0f, 0.0f, 0.1125f, 1.0f, 0.0f, 0.125f, 1.0f, 0.0f, 0.1375f, 1.0f, 0.0f, 0.15f, 1.0f, 0.0f, 0.1625f, 1.0f, 0.0f, 0.175f, 1.0f, 0.0f, 0.1875f, 1.0f, 0.0f, 0.2f, 1.0f, 0.0f, 0.2125f, 1.0f, 0.0f, 0.225f, 1.0f, 0.0f, 0.2375f, 1.0f, 0.0f, 0.25f, 1.0f, 0.0f, 0.2625f, 1.0f, 0.0f, 0.275f, 1.0f, 0.0f, 0.2875f, 1.0f, 0.0f, 0.3f, 1.0f, 0.0f, 0.3125f, 1.0f, 0.0f, 0.325f, 1.0f, 0.0f, 0.3375f, 1.0f, 0.0f, 0.35f, 1.0f, 0.0f, 0.3625f, 1.0f, 0.0f, 0.375f, 1.0f, 0.0f, 0.3875f, 1.0f, 0.0f, 0.4f, 1.0f, 0.0f, 0.4125f, 1.0f, 0.0f, 0.425f, 1.0f, 0.0f, 0.4375f, 1.0f, 0.0f, 0.45f, 1.0f, 0.0f, 0.4625f, 1.0f, 0.0f, 0.475f, 1.0f, 0.0f, 0.4875f, 1.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.5125f, 1.0f, 0.0f, 0.525f, 1.0f, 0.0f, 0.5375f, 1.0f, 0.0f, 0.55f, 1.0f, 0.0f, 0.5625f, 1.0f, 0.0f, 0.575f, 1.0f, 0.0f, 0.5875f, 1.0f, 0.0f, 0.6f, 1.0f, 0.0f, 0.6125f, 1.0f, 0.0f, 0.625f, 1.0f, 0.0f, 0.6375f, 1.0f, 0.0f, 0.65f, 1.0f, 0.0f, 0.6625f, 1.0f, 0.0f, 0.675f, 1.0f, 0.0f, 0.6875f, 1.0f, 0.0f, 0.7f, 1.0f, 0.0f, 0.7125f, 1.0f, 0.0f, 0.725f, 1.0f, 0.0f, 0.7375f, 1.0f, 0.0f, 0.75f, 1.0f, 0.0f, 0.7625f, 1.0f, 0.0f, 0.775f, 1.0f, 0.0f, 0.7875f, 1.0f, 0.0f, 0.8f, 1.0f, 0.0f, 0.8125f, 1.0f, 0.0f, 0.825f, 1.0f, 0.0f, 0.8375f, 1.0f, 0.0f, 0.85f, 1.0f, 0.0f, 0.8625f, 1.0f, 0.0f, 0.875f, 1.0f, 0.0f, 0.8875f, 1.0f, 0.0f, 0.9f, 1.0f, 0.0f, 0.9125f, 1.0f, 0.0f, 0.925f, 1.0f, 0.0f, 0.9375f, 1.0f, 0.0f, 0.95f, 1.0f, 0.0f, 0.9625f, 1.0f, 0.0f, 0.975f, 1.0f, 0.0f, 0.9875f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0125f, 1.0f, 0.9875f, 0.025f, 1.0f, 0.975f, 0.0375f, 1.0f, 0.9625f, 0.05f, 1.0f, 0.95f, 0.0625f, 1.0f, 0.9375f, 0.075f, 1.0f, 0.925f, 0.0875f, 1.0f, 0.9125f, 0.1f, 1.0f, 0.9f, 0.1125f, 1.0f, 0.8875f, 0.125f, 1.0f, 0.875f, 0.1375f, 1.0f, 0.8625f, 0.15f, 1.0f, 0.85f, 0.1625f, 1.0f, 0.8375f, 0.175f, 1.0f, 0.825f, 0.1875f, 1.0f, 0.8125f, 0.2f, 1.0f, 0.8f, 0.2125f, 1.0f, 0.7875f, 0.225f, 1.0f, 0.775f, 0.2375f, 1.0f, 0.7625f, 0.25f, 1.0f, 0.75f, 0.2625f, 1.0f, 0.7375f, 0.275f, 1.0f, 0.725f, 0.2875f, 1.0f, 0.7125f, 0.3f, 1.0f, 0.7f, 0.3125f, 1.0f, 0.6875f, 0.325f, 1.0f, 0.675f, 0.3375f, 1.0f, 0.6625f, 0.35f, 1.0f, 0.65f, 0.3625f, 1.0f, 0.6375f, 0.375f, 1.0f, 0.625f, 0.3875f, 1.0f, 0.6125f, 0.4f, 1.0f, 0.6f, 0.4125f, 1.0f, 0.5875f, 0.425f, 1.0f, 0.575f, 0.4375f, 1.0f, 0.5625f, 0.45f, 1.0f, 0.55f, 0.4625f, 1.0f, 0.5375f, 0.475f, 1.0f, 0.525f, 0.4875f, 1.0f, 0.5125f, 0.5f, 1.0f, 0.5f, 0.5125f, 1.0f, 0.4875f, 0.525f, 1.0f, 0.475f, 0.5375f, 1.0f, 0.4625f, 0.55f, 1.0f, 0.45f, 0.5625f, 1.0f, 0.4375f, 0.575f, 1.0f, 0.425f, 0.5875f, 1.0f, 0.4125f, 0.6f, 1.0f, 0.4f, 0.6125f, 1.0f, 0.3875f, 0.625f, 1.0f, 0.375f, 0.6375f, 1.0f, 0.3625f, 0.65f, 1.0f, 0.35f, 0.6625f, 1.0f, 0.3375f, 0.675f, 1.0f, 0.325f, 0.6875f, 1.0f, 0.3125f, 0.7f, 1.0f, 0.3f, 0.7125f, 1.0f, 0.2875f, 0.725f, 1.0f, 0.275f, 0.7375f, 1.0f, 0.2625f, 0.75f, 1.0f, 0.25f, 0.7625f, 1.0f, 0.2375f, 0.775f, 1.0f, 0.225f, 0.7875f, 1.0f, 0.2125f, 0.8f, 1.0f, 0.2f, 0.8125f, 1.0f, 0.1875f, 0.825f, 1.0f, 0.175f, 0.8375f, 1.0f, 0.1625f, 0.85f, 1.0f, 0.15f, 0.8625f, 1.0f, 0.1375f, 0.875f, 1.0f, 0.125f, 0.8875f, 1.0f, 0.1125f, 0.9f, 1.0f, 0.1f, 0.9125f, 1.0f, 0.0875f, 0.925f, 1.0f, 0.075f, 0.9375f, 1.0f, 0.0625f, 0.95f, 1.0f, 0.05f, 0.9625f, 1.0f, 0.0375f, 0.975f, 1.0f, 0.025f, 0.9875f, 1.0f, 0.0125f, 1.0f, 1.0f, 0.0f, 1.0f, 0.9875f, 0.0f, 1.0f, 0.975f, 0.0f, 1.0f, 0.9625f, 0.0f, 1.0f, 0.95f, 0.0f, 1.0f, 0.9375f, 0.0f, 1.0f, 0.925f, 0.0f, 1.0f, 0.9125f, 0.0f, 1.0f, 0.9f, 0.0f, 1.0f, 0.8875f, 0.0f, 1.0f, 0.875f, 0.0f, 1.0f, 0.8625f, 0.0f, 1.0f, 0.85f, 0.0f, 1.0f, 0.8375f, 0.0f, 1.0f, 0.825f, 0.0f, 1.0f, 0.8125f, 0.0f, 1.0f, 0.8f, 0.0f, 1.0f, 0.7875f, 0.0f, 1.0f, 0.775f, 0.0f, 1.0f, 0.7625f, 0.0f, 1.0f, 0.75f, 0.0f, 1.0f, 0.7375f, 0.0f, 1.0f, 0.725f, 0.0f, 1.0f, 0.7125f, 0.0f, 1.0f, 0.7f, 0.0f, 1.0f, 0.6875f, 0.0f, 1.0f, 0.675f, 0.0f, 1.0f, 0.6625f, 0.0f, 1.0f, 0.65f, 0.0f, 1.0f, 0.6375f, 0.0f, 1.0f, 0.625f, 0.0f, 1.0f, 0.6125f, 0.0f, 1.0f, 0.6f, 0.0f, 1.0f, 0.5875f, 0.0f, 1.0f, 0.575f, 0.0f, 1.0f, 0.5625f, 0.0f, 1.0f, 0.55f, 0.0f, 1.0f, 0.5375f, 0.0f, 1.0f, 0.525f, 0.0f, 1.0f, 0.5125f, 0.0f, 1.0f, 0.5f, 0.0f, 1.0f, 0.4875f, 0.0f, 1.0f, 0.475f, 0.0f, 1.0f, 0.4625f, 0.0f, 1.0f, 0.45f, 0.0f, 1.0f, 0.4375f, 0.0f, 1.0f, 0.425f, 0.0f, 1.0f, 0.4125f, 0.0f, 1.0f, 0.4f, 0.0f, 1.0f, 0.3875f, 0.0f, 1.0f, 0.375f, 0.0f, 1.0f, 0.3625f, 0.0f, 1.0f, 0.35f, 0.0f, 1.0f, 0.3375f, 0.0f, 1.0f, 0.325f, 0.0f, 1.0f, 0.3125f, 0.0f, 1.0f, 0.3f, 0.0f, 1.0f, 0.2875f, 0.0f, 1.0f, 0.275f, 0.0f, 1.0f, 0.2625f, 0.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.2375f, 0.0f, 1.0f, 0.225f, 0.0f, 1.0f, 0.2125f, 0.0f, 1.0f, 0.2f, 0.0f, 1.0f, 0.1875f, 0.0f, 1.0f, 0.175f, 0.0f, 1.0f, 0.1625f, 0.0f, 1.0f, 0.15f, 0.0f, 1.0f, 0.1375f, 0.0f, 1.0f, 0.125f, 0.0f, 1.0f, 0.1125f, 0.0f, 1.0f, 0.1f, 0.0f, 1.0f, 0.0875f, 0.0f, 1.0f, 0.075f, 0.0f, 1.0f, 0.0625f, 0.0f, 1.0f, 0.05f, 0.0f, 1.0f, 0.0375f, 0.0f, 1.0f, 0.025f, 0.0f, 1.0f, 0.0125f, 0.0f, 1.0f, 0.0f, 0.0f, 0.9875f, 0.0f, 0.0f, 0.975f, 0.0f, 0.0f, 0.9625f, 0.0f, 0.0f, 0.95f, 0.0f, 0.0f, 0.9375f, 0.0f, 0.0f, 0.925f, 0.0f, 0.0f, 0.9125f, 0.0f, 0.0f, 0.9f, 0.0f, 0.0f };
        static readonly BitmapPalette _grayscaleLUT = BuildPaletteGrayscale();

        readonly double[] _autoBottomFittingPercentile; // TODO: unused
        readonly double[] _autoTopFittingPercetile; // TODO: unused
        readonly ObservableCollection<ObservableCollection<bool>> _channelDisplayEnable;
        readonly List<PixelDataHistogramInfo> _pixelDataHistograms;
        readonly byte[][][] _rawImg = new byte[MAX_CHANNELS][][]; // [channel, plane/ROI, pixel (x + bitmapWidth*y)]
        readonly byte[][][] _rawImgmROIPriorityDisplayAreaComposite = new byte[MAX_CHANNELS][][]; // [channel, plane/ROI, pixel (x + bitmapWidth*y)]
        readonly string[] _wavelengthNames = new string[MAX_CHANNELS];

        FrameData _lastFullFOVFrameData;
        byte[][] _24BitmROIPriorityDisplayAreaComposite; // composite of each channel [plane, pixel (x + bitmapWidth*y)]
        byte[][] _24BitPixelDataChannelComposite; // composite of each channel [plane, pixel (x + bitmapWidth*y)]
        byte[][] _24BitPixelDataForBackground; // Duplicate of composite
        private bool _allowmROIBackgroundImage = false; // State of the checkbox in area control
        volatile bool[,] _backgroundCopyFlags; //make sure the background is only copied once
        private int _backgroundHeight = 0;

        //For mROI Background image
        private int _backgroundWidth = 0;
        bool _backupCompositeForBackground = false;
        WriteableBitmap _bitmap = null;
        WriteableBitmap _bitmap16 = null;
        Thread _bitmapBuildingThread = null;
        WriteableBitmap _bitmapXZ = null;
        WriteableBitmap _bitmapYZ = null;
        private int _channelCountBeforeRefImage = 0;

        private double _bitmapUpdateRate = 0;
        private bool _calculateBitmapUpdateRate = true;
        //values are cached if reference channel is enabled so there is a non-modified channel selection available
        private int _channelSelectionBeforeRefImage = 0;
        List<Tuple<int, int>> _channelsPerSequence = new List<Tuple<int, int>>();
        bool _clearOrthogonalBitmaps = false;
        Thread _copyDataThread = null;
        volatile int _currentAreaIndex = 0;
        bool _enableHistogramUpdate = true;
        FrameDataHistory _frameDataHistory;

        //TODO: See if this is needed when loading data in review very quickly to prevent skipping
        volatile bool _frameDataReadyToSet = true;
        GetPixelInfo _getPixelInfoDelegate;
        bool _grayscaleForSingleChannel = false;
        ImageGridParameters _imageGridParameters;
        bool _isInSequentialMode = false;
        private bool _isRolloverButtonChecked = false;
        Rect[] _mROIRects;
        volatile bool _needToResizeBackground = true; // make sure th background is only resized once
        bool _onlyShowTrueSaturation = true;
        bool _orthogonalViewEnabled = false;
        Point _orthogonalViewPosition = new Point(0, 0);
        double _orthogonalViewZMultiplier = 1.0;
        byte[] _pdXZ;
        byte[] _pdYZ;
        private int _prevAreaIndex = -100;
        private bool _referenceChannelEnabled = false;
        private string _referenceChannelImageName = "";

        //Reference Channel variables
        private string _referenceChannelImagePath = "";
        byte[] _resizedBackground = null; //interpolation of composite for background. Only need one array so composite[0] is used
        int _rollOverPointX;
        int _rollOverPointY;
        bool _tileDisplay = false;
        private bool _updateBackgroundImage = false; // flag to alert the bitmap to reset
        bool _virtualZStack = true;
        private bool _whitePointIncreased = false;
        int _whitePointMaxVal = 255;
        int _zNumSteps = 1;
        IntPtr _zPreviewImageData = IntPtr.Zero;
        ulong _zPreviewImageDataSize;
        ushort[][] _zPreviewPixelData;

        #endregion Fields

        #region Constructors

        public ImageViewMBase()
        {
            _frameDataHistory = new FrameDataHistory(1);
            _autoBottomFittingPercentile = new double[MAX_CHANNELS];
            _autoTopFittingPercetile = new double[MAX_CHANNELS];
            _getPixelInfoDelegate = new GetPixelInfo(GetPixelInformation);
            _updatePixelData = false;
            _imageGridParameters = new ImageGridParameters();
            _lastFullFOVFrameData = new FrameData();
            UpdateHistogramsLock = new object();

            //create one instance of the histogram arrays
            _pixelDataHistograms = new List<PixelDataHistogramInfo>();

            ChannelLuts = new List<Color[][]>();
            ChannelLuts.Add(new Color[MAX_CHANNELS][]);
            _channelDisplayEnable = new ObservableCollection<ObservableCollection<bool>>();
            _channelDisplayEnable.Add(new ObservableCollection<bool>());
            for (int i = 0; i < MAX_CHANNELS; i++)
            {
                ChannelLuts[DEFAULT_CHANNEL_GROUP][i] = new Color[LUT_SIZE];
                _channelDisplayEnable[DEFAULT_CHANNEL_GROUP].Add(true);
            }

            AllowReferenceImage = false;
            AllowmROIBackgroundImage = false;

            _referenceChannelImageName = "ReferenceChannel.tif";
            _referenceChannelImagePath = Application.Current.Resources["AppRootFolder"].ToString() + "\\ReferenceChannel.tif";

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, GetType().Name + " Created");
        }

        #endregion Constructors

        #region Events

        public event Action BitmapUpdated;

        public event Action BitmapUpdateRateUpdated;

        public event Action PixelBitShiftValueChanged;

        public event Action<ImageIdentifier> PixelDataUpdated;

        public event Action ReferenceChannelInvalidated;

        public event EventHandler<ZoomChangeEventArgs> ZoomChanged;

        protected virtual void OnZoomChanged(ZoomChangeEventArgs e)
        {
            ZoomChanged?.Invoke(this, e);
        }

        #endregion Events

        #region Properties

        public bool AllowmROIBackgroundImage
        {
            get
            {
                return _allowmROIBackgroundImage;
            }
            set
            {
                if (value)
                {
                    if (_imagesGrid == null || _imagesGrid[0] == null)
                    {
                        //Notify the area control
                        MVMManager.Instance["AreaControlViewModel", "mROIShowFullFovAsBackground"] = false;
                        ((ThorSharedTypes.IMVM)MVMManager.Instance["AreaControlViewModel", this]).OnPropertyChange("mROIShowFullFovAsBackground");
                        MessageBox.Show("Please image with Full FOV first");
                    }
                }
                //Change in value indicating that the background should be changed
                //Used to trigger bitmap change event
                if (value != _allowmROIBackgroundImage)
                {
                    _updateBackgroundImage = true;
                }
                _allowmROIBackgroundImage = value;
            }
        }

        public bool AllowReferenceImage
        {
            get; set;
        }

        public bool BackupCompositeForBackground
        {
            get => _backupCompositeForBackground;
            set => _backupCompositeForBackground = value;
        }

        public WriteableBitmap Bitmap
        {
            get
            {
                if (_imagesGrid?.Count > 0 && _imagesGrid[0]?.Count > 0)
                {
                    return _imagesGrid[0][0].ImageXY;
                }
                return null;
            }
        }

        public WriteableBitmap Bitmap16
        {
            get => _bitmap16;
        }

        public WriteableBitmap BitmapXZ
        {
            get
            {
                if (_frameDataHistory.LastUpdatedFrame == null)
                {
                    return null;
                }

                PixelFormat pf = PixelFormats.Rgb24;
                //TODO: needs needs to change for ImageReview, we should receive the Z count at these stage
                int totalNumOfZstack = _zNumSteps;
                int width = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.imageWidth;

                if (width != 0 && _pdXZ != null && false == ClearOrthogonalBitmaps)
                {
                    // Define parameters used to create the BitmapSource.
                    int rawStrideXZ = (width * pf.BitsPerPixel + 7) / 8; //_bitmapXZ

                    double pixelSizeUM = _frameDataHistory.LastUpdatedFrame.StoredFrameData.pixelSizeUM.PixelWidthUM;
                    double zStepSizeUM = _zStepSizeUM;

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
                if (_frameDataHistory.LastUpdatedFrame == null)
                {
                    return null;
                }

                PixelFormat pf = PixelFormats.Rgb24;

                int totalNumOfZstack = _zNumSteps;
                int height = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.imageHeight;

                if (height != 0 && _pdYZ != null && _frameData != null && false == ClearOrthogonalBitmaps)
                {
                    // Define parameters used to create the BitmapSource.
                    int rawStrideYZ = (totalNumOfZstack * pf.BitsPerPixel + 7) / 8;

                    double pixelSizeUM = _frameData.pixelSizeUM.PixelWidthUM;
                    double zStepSizeUM = _zStepSizeUM;

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

        public double BitmapUpdateRate
        {
            get => _bitmapUpdateRate;
        }

        public bool CalculateBitmapUpdateRate
        {
            get => _calculateBitmapUpdateRate;
            set => _calculateBitmapUpdateRate = value;
        }

        public bool ChanEnableChanged
        {
            get
            {
                return _chanEnableChanged;
            }
            set
            {
                _chanEnableChanged = value;
            }
        }

        public int ChannelCountBeforeRefImage
        {
            set => _channelCountBeforeRefImage = value;
        }

        public ObservableCollection<ObservableCollection<bool>> ChannelDisplayEnable
        {
            get => _channelDisplayEnable;
        }

        public List<Color[][]> ChannelLuts
        {
            get;
            set;
        }

        public int ChannelSelectionBeforeRefImage
        {
            set => _channelSelectionBeforeRefImage = value;
        }

        public List<Tuple<int, int>> ChannelsPerSequence
        {
            get
            {
                return _channelsPerSequence;
            }
            set
            {
                _channelsPerSequence = value;
            }
        }

        public bool ClearOrthogonalBitmaps
        {
            get
            {
                return _clearOrthogonalBitmaps;
            }
            set
            {
                _clearOrthogonalBitmaps = value;
            }
        }

        private bool _displayPixelAspectRatio = false;
        private bool _aspectRatioChanged = false;
        public bool DisplayPixelAspectRatio
        {
            get => _displayPixelAspectRatio;
            set
            {
                _displayPixelAspectRatio = value;
                _updateBitmap = true;
                _aspectRatioChanged = true;
            }
        }

        public string DFLIMControlViewModelName
        {
            get; set;
        }

        public bool EnableHistogramUpdate
        {
            get
            {
                return _enableHistogramUpdate;
            }
            set
            {
                _enableHistogramUpdate = value;
            }
        }

        public string ExperimentPath
        {
            get; set;
        }

        public FrameData FullFOVFrameData
        {
            get
            {
                return _lastFullFOVFrameData;
            }
        }

        public virtual FrameData FrameData
        {
            get
            {
                return _frameData;
            }

            set
            {
                if (!_frameDataReadyToSet)
                {
                    return;
                }
                if (value == null)
                {
                    return;
                }
                if (IsInSequentialMode && 0 == value.frameInfo.sequenceIndex)
                {
                    EnableHistogramUpdate = false;
                }
                _frameData = value;
                if (_bitsPerPixel != _frameData.bitsPerPixel)
                {
                    _bitsPerPixel = _frameData.bitsPerPixel;
                    PixelBitShiftValueChanged?.Invoke();
                }
                _frameDataReadyToSet = false;
                _dataReadyToCopy = true;
            }
        }

        public bool FrameDataReadyToSet
        {
            get
            {
                //Not currently used
                return _frameDataReadyToSet;
            }
        }

        public GetPixelInfo GetPixelInfoDelegate
        {
            get => _getPixelInfoDelegate;
        }

        public bool GrayscaleForSingleChannel
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

        public string ImageNameFormat
        {
            get
            {
                int imgIndxDigiCnts = (int)Constants.DEFAULT_FILE_FORMAT_DIGITS;
                if (!Directory.Exists(ExperimentPath))
                {
                    XmlNode node = ThorSharedTypes.MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS].SelectSingleNode("/ApplicationSettings/ImageNameFormat");
                    string str = string.Empty;
                    XmlManager.GetAttribute(node, ThorSharedTypes.MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS], "indexDigitCounts", ref str);
                    Int32.TryParse(str, out imgIndxDigiCnts);
                }
                else
                {
                    foreach (string fn in Directory.GetFiles(ExperimentPath, "*_*_*", SearchOption.TopDirectoryOnly).Where(file => file.ToLower().EndsWith("raw") || file.ToLower().EndsWith("tif")))
                    {
                        string n = fn.Substring(fn.LastIndexOf('\\') + 1);
                        string str = (n.Split('_'))[1];
                        if (int.TryParse(str, out int num))
                        {
                            imgIndxDigiCnts = str.Length;
                            break;
                        }
                    }
                }
                return ("D" + imgIndxDigiCnts.ToString());
            }
        }

        public ObservableCollection<ObservableCollection<CompoundImage>> ImagesGrid
        {
            get => _imagesGrid;
        }

        public bool ImageUpdateComplete
        {
            get => !_updatePixelData;
        }

        public ImageViewType ImageViewType
        {
            get => _imageReviewType;
        }

        public bool IsInSequentialMode
        {
            get
            {
                return _isInSequentialMode;
            }
            set
            {
                _isInSequentialMode = value;
            }
        }

        public bool IsRolloverButtonChecked
        {
            get => _isRolloverButtonChecked;
            set => _isRolloverButtonChecked = value;
        }

        //TODO: need to change this logic to use lsmChannel only
        public bool IsSingleChannel
        {
            get
            {
                return (1 == FrameData?.channelSelection || 2 == FrameData?.channelSelection || 4 == FrameData?.channelSelection || 8 == FrameData?.channelSelection);
            }
        }

        public FrameInfoStruct LastFrameInfo
        {
            get
            {
                if (_frameDataHistory.LastUpdatedFrame == null)
                {
                    return new FrameInfoStruct() { }; // TODO check if this needs to be initialized
                }
                return _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo;
            }
        }

        public virtual int mROIPriorityIndex
        {
            get => _mROIPriorityIndex;
            set => _mROIPriorityIndex = value;
        }

        public virtual bool mROISpatialDisplaybleEnable
        {
            get => _mROISpatialDisplaybleEnable;
            set => _mROISpatialDisplaybleEnable = value;
        }

        public bool OnlyShowTrueSaturation
        {
            get => _onlyShowTrueSaturation;
            set => _onlyShowTrueSaturation = value;
        }

        public bool OrthogonalViewEnabled
        {
            get => _orthogonalViewEnabled;
            set => _orthogonalViewEnabled = value;
        }

        public Point OrthogonalViewPosition
        {
            get
            {
                return _orthogonalViewPosition;
            }
            set
            {
                _orthogonalViewPosition = value;
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

        public bool PaletteChanged
        {
            get => _palletChanged;
            set => _palletChanged = value;
        }

        public int PixelBitShiftValue
        {
            get
            {
                return (_bitsPerPixel - 8);
            }
        }

        public List<PixelDataHistogramInfo> PixelDataHistograms
        {
            get
            {
                return _pixelDataHistograms;
            }
        }

        public bool ReferenceChannelEnabled
        {
            get => _referenceChannelEnabled;
            set => _referenceChannelEnabled = value;
        }

        public string ReferenceChannelImageName
        {
            get
            {
                return _referenceChannelImageName;
            }
            set
            {
                _referenceChannelImageName = value;
            }
        }

        public string ReferenceChannelImagePath
        {
            get
            {
                return _referenceChannelImagePath;
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
                int val = value;
                if (TileDisplay && _frameDataHistory?.LastUpdatedFrame?.StoredLastFrameInfo.imageWidth > 0)
                {
                    if (_frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.isMROI == 1)
                    {
                        val %= _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.fullImageWidth;
                    }
                    else
                    {
                        val %= _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.imageWidth;
                    }
                }
                _rollOverPointX = val;
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
                int val = value;
                if (TileDisplay && _frameDataHistory?.LastUpdatedFrame?.StoredLastFrameInfo.imageHeight > 0)
                {
                    if (_frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.isMROI == 1)
                    {
                        val %= _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.fullImageHeight;
                    }
                    else
                    {
                        val %= _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.imageHeight;
                    }
                }
                _rollOverPointY = val;
            }
        }

        public bool TileDisplay
        {
            get => _tileDisplay;
            set
            {
                if (value != _tileDisplay)
                {
                    _tileDisplay = value;
                    _updateBitmap = true;
                    _tileViewToggled = true;
                }
            }
        }

        public int TotalDisplayedChannels
        {
            get;
            set;
        }

        public Object UpdateHistogramsLock
        {
            get; set;
        }

        public bool VerticalTileDisplay
        {
            get; set;
        }

        public bool VirtualZStack
        {
            get => _virtualZStack;
            set => _virtualZStack = value;
        }

        public string[] WavelengthNames
        {
            get
            {
                if (_channelDisplayEnable[DEFAULT_CHANNEL_GROUP][0])
                {
                    _wavelengthNames[0] = "ChanA";
                }
                if (_channelDisplayEnable[DEFAULT_CHANNEL_GROUP][1])
                {
                    _wavelengthNames[1] = "ChanB";
                }
                if (_channelDisplayEnable[DEFAULT_CHANNEL_GROUP][2])
                {
                    _wavelengthNames[2] = "ChanC";
                }
                if (_channelDisplayEnable[DEFAULT_CHANNEL_GROUP][3])
                {
                    _wavelengthNames[3] = "ChanD";
                }
                return _wavelengthNames;
            }
        }

        public bool WhitePointIncreased
        {
            get => _whitePointIncreased;
            set => _whitePointIncreased = value;
        }

        public int WhitePointMaxVal
        {
            get => _whitePointMaxVal;
            set => _whitePointMaxVal = value;
        }

        #endregion Properties

        #region Methods

        [DllImport("..\\Modules_Native\\ImageStoreLibrary.dll", EntryPoint = "GetImageStoreInfo")]
        public static extern int GetImageStoreInfo([MarshalAs(UnmanagedType.LPStr)] string fileName, int regionID, ref int regionCount, ref int width, ref int height, ref int channelCount, ref int zMaxCount, ref int timeCount, ref int spectCount);

        public static bool LoadRefChann(string fileName, ref IntPtr refChannPtr)
        {
            bool status = ReadImage(fileName, ref refChannPtr);
            return status;
        }

        public static bool LoadRefChannInfo(string fileName, ref long width, ref long height, ref long colorChannels, ref long bitsPerChannel)
        {
            bool status = ReadImageInfo(fileName, ref width, ref height, ref colorChannels, ref bitsPerChannel);
            return status;
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

        public void AutoEnhanceIfEnabled()
        {
            foreach (var pdHistInfo in PixelDataHistograms)
            {
                if (pdHistInfo.IsAutoPressed || (pdHistInfo.IsContinuousAutoChecked && !IsInSequentialMode))
                {
                    if (pdHistInfo.Min != pdHistInfo.BlackPoint || pdHistInfo.Max != pdHistInfo.WhitePoint)
                    {
                        pdHistInfo.IsWhiteBlackPointChanged = true;
                    }
                }
            }
        }

        public bool BrowseForReferenceImage()
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Title = "Select a Reference Image File";
            dlg.DefaultExt = "tif";
            /*Dialog doesn't like directory path to have too many backslashes*/
            dlg.InitialDirectory = ResourceManagerCS.GetMyDocumentsThorImageFolderString();
            string[] initDirSplit = dlg.InitialDirectory.Split(new char[] { '\\' }, StringSplitOptions.RemoveEmptyEntries);

            dlg.InitialDirectory = string.Join("\\", initDirSplit);
            dlg.Filter = "16 Bit Tiff file (*.tif)|*.tif";

            // Show save file dialog box
            Nullable<bool> result = dlg.ShowDialog();
            // Process save file dialog box results
            if (result == true && dlg.FileName != "")
            {
                _referenceChannelImageName = dlg.SafeFileName;
                _referenceChannelImagePath = dlg.FileName;
                return true;
            }
            else
            {
                return false;
            }
        }

        public double GetBlackPoint(ImageIdentifier imageIdentifier)
        {
            var pdHistogramInfo = GetOrCreatePixelDataHistogram(imageIdentifier, false);
            return pdHistogramInfo.BlackPoint;
        }

        public int[] GetBufferOffsetIndex()
        {
            //calculate the raw data buffer offset index for each of the
            //selected display channels
            List<int> dataBufferOffsetIndex = new List<int>();

            int enabledChannelCount = 0;
            for (int i = 0; i < ImageViewMBase.MAX_CHANNELS; i++)
            {
                //if the channgel is enabled store the index
                if (ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP][i] && (FrameData.channelSelection & (0x0001 << i)) > 0)
                {
                    dataBufferOffsetIndex.Add(i);
                    enabledChannelCount++;
                }
            }
            return dataBufferOffsetIndex.ToArray();
        }

        public int GetColorChannels()
        {
            return _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.channels;
        }

        public bool GetIsAutoButtonPressed(ImageIdentifier imageIdentifier)
        {
            var pdHistInfo = GetOrCreatePixelDataHistogram(imageIdentifier, false);
            return pdHistInfo.IsAutoPressed;
        }

        public bool GetIsContinuousAutoChecked(ImageIdentifier imageIdentifier)
        {
            var pdHistInfo = GetOrCreatePixelDataHistogram(imageIdentifier, false);
            return pdHistInfo.IsContinuousAutoChecked;
        }

        public PixelDataHistogramInfo GetOrCreatePixelDataHistogram(ImageIdentifier imageIdentifier, bool create = true)
        {
            try
            {
                foreach (var pixelDataHistogram in _pixelDataHistograms.ToList())
                {
                    if (pixelDataHistogram?.DataImageIdentifier.Equals(imageIdentifier) == true)
                    {
                        return pixelDataHistogram;
                    }
                }

                if (create)
                {
                    var newPixelDataHistogram = new PixelDataHistogramInfo(imageIdentifier);
                    _pixelDataHistograms.Add(newPixelDataHistogram);

                    return newPixelDataHistogram;
                }
                else
                {
                    return null;
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
                var newPixelDataHistogram = new PixelDataHistogramInfo(imageIdentifier);
                _pixelDataHistograms.Add(newPixelDataHistogram);

                return newPixelDataHistogram;
            }
        }

        public int GetPixelInformation(int channel, int groupIndex)
        {
            if (groupIndex < 0)
            {
                groupIndex = 0;
            }
            if (!_isRolloverButtonChecked || _frameDataHistory == null || _frameDataHistory.FrameDataList.Length < groupIndex || _frameDataHistory.FrameDataList[groupIndex] == null)
            {
                return 0;
            }
            FrameInfoStruct frameInfo = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo;

            if (_frameDataHistory.FrameDataList[_currentAreaIndex].StoredFrameData.frameInfo.isMROI == 1)
            {
                groupIndex = 0;
                for (int i = 0; i < _frameDataHistory.FrameDataList.Length; i++)
                {
                    if (_frameDataHistory.FrameDataList[i] != null && _frameDataHistory.FrameDataList[i].StoredFrameData != null)
                    {
                        FrameInfoStruct info = _frameDataHistory.FrameDataList[i].StoredFrameData.frameInfo;
                        if (info.leftInFullImage < _rollOverPointX && info.leftInFullImage + info.imageWidth > _rollOverPointX)
                        {
                            if (info.topInFullImage < _rollOverPointY && info.topInFullImage + info.imageHeight > _rollOverPointY)
                            {
                                int location = (_rollOverPointX - info.leftInFullImage) + info.imageWidth * (_rollOverPointY - info.topInFullImage);
                                int chan = info.channels == 1 ? 0 : channel;
                                int dataLength = info.imageHeight * info.imageWidth;
                                if ((_rollOverPointX < frameInfo.fullImageWidth) && (_rollOverPointY < frameInfo.fullImageHeight) && (location >= 0) && ((location + chan * dataLength) < _frameDataHistory.FrameDataList[i].StoredFrameData.pixelData.Length))
                                {
                                    int offset = dataLength;
                                    int val = (_frameDataHistory.FrameDataList[i].StoredFrameData.pixelData[location + chan * offset]);
                                    return val;
                                }
                                break;
                            }
                        }
                    }
                }
                return 0;
            }
            else
            {
                FrameData fd = _frameDataHistory.FrameDataList[groupIndex].StoredFrameData;
                int chan = fd?.frameInfo.channels == 1 ? 0 : channel;
                if (fd?.pixelData != null && fd?.frameInfo.imageHeight != 0 && fd?.frameInfo.channels >= chan + 1)
                {
                    //if the requested pixel is within the buffer size
                    int location = _rollOverPointX + frameInfo.imageWidth * _rollOverPointY;

                    int dataLength = frameInfo.imageHeight * frameInfo.imageWidth;
                    if ((_rollOverPointX < frameInfo.imageWidth) && (_rollOverPointY < frameInfo.imageHeight) && (location >= 0) && ((location + chan * dataLength) < fd.pixelData.Length))
                    {
                        int offset = dataLength;
                        int val = (fd.pixelData[location + chan * offset]);
                        return val;
                    }
                }
            }

            return 0;
        }

        public double GetWhitePoint(ImageIdentifier imageIdentifier)
        {
            var pdHistInfo = GetOrCreatePixelDataHistogram(imageIdentifier);
            return pdHistInfo.WhitePoint;
        }

        public void InitializeOrthogonalViewDataAndBuffers(int zStepNum, double zStepSizeUM)
        {
            if ((null == _zPreviewPixelData) || (_zPreviewPixelData.Length != zStepNum))
            {
                _zPreviewPixelData = new ushort[zStepNum][];
            }
            int width;
            int height;

            try
            {

                width = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.imageWidth;
                height = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.imageHeight;
            }
            catch (NullReferenceException)
            {
                width = 1;
                height = 1;
            }

            int pdXZ_DataLength = PixelFormats.Rgb24.BitsPerPixel / 8 * width * zStepNum;
            int pdYZ_DataLength = PixelFormats.Rgb24.BitsPerPixel / 8 * height * zStepNum;

            if (_bitmapXZ == null || _pdXZ == null || _pdXZ.Length != pdXZ_DataLength)
            {
                _pdXZ = new byte[pdXZ_DataLength];
            }

            if (_bitmapYZ == null || _pdYZ == null || _pdYZ.Length != pdYZ_DataLength)
            {
                _pdYZ = new byte[pdYZ_DataLength];
            }

            _zNumSteps = zStepNum;
            _zStepSizeUM = zStepSizeUM;
        }

        public void LoadZImageAndCreateOrthogonalViewBitmap(int tIndex, int zIndex, int totalZSteps)
        {
            CaptureFile imageType = CaptureFile.FILE_TIFF;
            //load the images
            string[] fileNames = GetZImageFileNames(tIndex, zIndex + 1, ref imageType); // get first image
            bool result = UpdateChannelData(fileNames, zIndex, tIndex, imageType);

            if (result)
            {

                CreateOrthogonalBitmap(zIndex, totalZSteps);
            }
        }

        public void ResetBitmap()
        {
            StopBitmapBuildingThread();
            _bitmap = null;
            _imagesGrid = null;
            _frameDataSet = false;
            _frameDataHistory = null;
            BitmapUpdated?.BeginInvoke(null, null);
            StartBitmapBuildingThread();
        }

        public void ResetFrameDataSet()
        {
            _frameDataSet = false;
        }

        public void SetAutoFittingPercentiles(int index, double top, double bottom)
        {
            _autoTopFittingPercetile[index] = Math.Min(100, Math.Max(0, top));
            _autoBottomFittingPercentile[index] = Math.Min(100, Math.Max(0, bottom));
        }

        public void SetBlackPoint(ImageIdentifier imageIdentifier, double value)
        {
            var pdHistInfo = GetOrCreatePixelDataHistogram(imageIdentifier);
            pdHistInfo.BlackPoint = Math.Min(LUT_MAX, Math.Max(LUT_MIN, value));
        }

        public void SetGamma(ImageIdentifier imageIdentifier, double value)
        {
            var pdHistInfo = GetOrCreatePixelDataHistogram(imageIdentifier);
            pdHistInfo.Gamma = Math.Min(16.0, Math.Max(0, value));
        }

        public void SetIsAutoButtonPressed(ImageIdentifier imageIdentifier, bool value)
        {
            var pdHistInfo = GetOrCreatePixelDataHistogram(imageIdentifier);
            pdHistInfo.IsAutoPressed = value;
        }

        public void SetIsContinuousChecked(ImageIdentifier imageIdentifier, bool value)
        {
            var pdHistInfo = GetOrCreatePixelDataHistogram(imageIdentifier);
            pdHistInfo.IsContinuousAutoChecked = value;
        }

        public void SetWhitePoint(ImageIdentifier imageIdentifier, double value)
        {
            var pdHistInfo = GetOrCreatePixelDataHistogram(imageIdentifier);
            pdHistInfo.WhitePoint = Math.Min(LUT_MAX, Math.Max(LUT_MIN, value));
        }

        public void StartBitmapBuildingThread()
        {
            _runBitmapBuildingThread = true;
            _bitmapBuildingThread = new Thread(new ThreadStart(BuildBitmapsTask));
            _bitmapBuildingThread.IsBackground = true;
            _bitmapBuildingThread.Priority = ThreadPriority.AboveNormal;
            _bitmapBuildingThread.Start();
        }

        public void StartCopyDataThread()
        {
            _runCopyDataThread = true;
            _copyDataThread = new Thread(CopyDataTask);
            _copyDataThread.IsBackground = true;
            _copyDataThread.Priority = ThreadPriority.AboveNormal;
            _copyDataThread.Start();
        }

        public void StopBitmapBuildingThread()
        {
            _runBitmapBuildingThread = false;
            _bitmapBuildingThread?.Abort();
            _bitmapBuildingThread?.Join();
        }

        public void StopCopyDataThread()
        {
            _runCopyDataThread = false;
            _copyDataThread?.Abort();
            _copyDataThread?.Join();
        }

        public bool UpdateChannelData(string[] fileNames, int zIndexToRead, int tIndexToRead, CaptureFile imageType)
        {
            //TODO: need to fix this for image review
            int pixelX = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.imageWidth;
            int pixelY = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.imageHeight;
            int zSteps = _zNumSteps;
            byte enabledChannels = (byte)GetColorChannels();
            return UpdateChannelData(fileNames, zIndexToRead, enabledChannels, 4, tIndexToRead, pixelX, pixelY, zSteps, GetRawContainsDisabledChannels(), imageType);
        }

/*        public void UpdateCompoundImageROILineWidths(double zoomLevel)
        {
            if (_imagesGrid != null && _imagesGrid.Count > 0)
            {
                for (int x = 0; x < _imagesGrid.Count; x++)
                {
                    for (int y = 0; y < _imagesGrid[x].Count; y++)
                    {
                        foreach (Shape roi in _imagesGrid[x][y]?.OverlayItems)
                        {
                            roi.StrokeThickness = OverlayManager.OverlayManagerClass.DefaultStrokeThickness * 1 / zoomLevel;
                        }
                    }
                }
            }
        }*/

        public abstract void UpdateOverlayManager(int width, int height, double xScale, double yScale);

        public byte[] UpdatePixelDatabyte(ushort[] savePixelData)
        {
            byte[] pixelData = new byte[3] { 0, 0, 0 };

            foreach (var pdHistInfo in PixelDataHistograms)
            {
                int channel = pdHistInfo.DataImageIdentifier.Channel;
                if (ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP][channel] && (FrameData.channelSelection & (0x0001 << channel)) > 0)
                {
                    var palette = pdHistInfo.Palette;
                    //  byte valNormalized = _pixelDataLUT[_dataBufferOffsetIndex[channelIndex]][savePixelData[channelIndex]];
                    Color col = 1 == FrameData.frameInfo.channels && true == _grayscaleForSingleChannel ? _grayscaleLUT.Colors[palette[savePixelData[channel]]] : ChannelLuts[DEFAULT_CHANNEL_GROUP][channel][palette[savePixelData[channel]]];
                    //Color col = 1 == _lastFrameInfo.channels && true == _grayscaleForSingleChannel ? _grayscaleLUT.Colors[valNormalized] : ChannelLuts[_dataBufferOffsetIndex[channelIndex]][valNormalized];

                    pixelData[0] = Math.Max(pixelData[0], col.R);
                    pixelData[1] = Math.Max(pixelData[1], col.G);
                    pixelData[2] = Math.Max(pixelData[2], col.B);
                }
            }
            return pixelData;
        }

        internal WriteableBitmap BuildBitmap()
        {
            if (_frameData == null) return null;
            ushort[] pd = _frameData.pixelData;

            //verify pixel data is available
            if (pd == null)
            {
                return _bitmap;
            }

            int width = _frameData.frameInfo.imageWidth;
            int height = _frameData.frameInfo.imageHeight;

            switch (_frameData.frameInfo.channels)
            {
                case 1:
                    {
                        //Case for monochrome/single channel images

                        PixelFormat pf = PixelFormats.Gray8;
                        int rawStride = (width * pf.BitsPerPixel + 7) / 8;

                        //create a new bitmpap when one does not exist or the size of the image changes
                        if (_bitmap == null)
                        {
                            _bitmap = new WriteableBitmap(width, height, 96, 96, pf, null);
                        }
                        else
                        {
                            if ((_bitmap.Width != width) || (_bitmap.Height != height) || (_bitmap.Format != pf))
                            {
                                _bitmap = new WriteableBitmap(width, height, 96, 96, pf, null);
                            }
                        }

                        //Scale the image data for different camera bit depths
                        //Scale factor is the target bit depth divided by the image bit depth
                        //scaled array values need to be 8bit each to properly fill the image with the calculated stride
                        double scaleFactor = Math.Pow(2, pf.BitsPerPixel) / Math.Pow(2, _frameData.bitsPerPixel);
                        byte[] scaledPixelDataCopy = new byte[_frameData.pixelData.Length];
                        for (int i = 0; i < scaledPixelDataCopy.Length; i++)
                        { scaledPixelDataCopy[i] = (byte)(_frameData.pixelData[i] * scaleFactor); }

                        int w = _bitmap.PixelWidth;
                        int h = _bitmap.PixelHeight;

                        if (pd.Length == (width * height))
                        {
                            //copy the scaled pixel data into the _bitmap
                            _bitmap.WritePixels((new Int32Rect(0, 0, w, h)), scaledPixelDataCopy, rawStride, 0);
                        }
                    }
                    break;

                default:
                    {
                        //Case for 8 bit color image
                        PixelFormat pf = PixelFormats.Rgb24;

                        int rawStride = (width * pf.BitsPerPixel + 7) / 8;

                        //create a new bitmpap when one does not exist or the size of the image changes
                        if (_bitmap == null)
                        {
                            _bitmap = new WriteableBitmap(width, height, 96, 96, pf, null);
                        }
                        else
                        {
                            if ((_bitmap.Width != width) || (_bitmap.Height != height) || (_bitmap.Format != pf))
                            {
                                _bitmap = new WriteableBitmap(width, height, 96, 96, pf, null);
                            }
                        }

                        int[] dataBufferOffsetIndex = GetBufferOffsetIndex();
                        int enabledChannelCount = dataBufferOffsetIndex.Length;
                        int w = _bitmap.PixelWidth;
                        int h = _bitmap.PixelHeight;

                        //Scale factor is the target bit depth divided by the image bit depth
                        double scaleFactor = Math.Pow(2, pf.BitsPerPixel / _frameData.frameInfo.channels) / Math.Pow(2, _frameData.bitsPerPixel);

                        if ((pd.Length / _frameData.frameInfo.channels) == (width * height))
                        {
                            int buffSize = pd.Length / _frameData.frameInfo.channels;

                            byte[] pdData = new byte[buffSize * 3];

                            int i = 0;
                            for (int j = 0; j < buffSize; i += 3, j++)
                            {
                                byte maxRed = 0;
                                byte maxGreen = 0;
                                byte maxBlue = 0;
                                pdData[i] = 0;
                                pdData[i + 1] = 0;
                                pdData[i + 2] = 0;
                                const double LUT_MAX_VAL = 255.0;

                                for (int k = 0; k < enabledChannelCount; k++)
                                {
                                    int valRaw = (pd[j + dataBufferOffsetIndex[k] * buffSize]);
                                    int valRawByte = Math.Min(255, ((pd[j + dataBufferOffsetIndex[k] * buffSize])) >> 6);

                                    double percentRed = (ChannelLuts[DEFAULT_CHANNEL_GROUP][dataBufferOffsetIndex[k]][valRawByte].R / LUT_MAX_VAL);
                                    double percentGreen = (ChannelLuts[DEFAULT_CHANNEL_GROUP][dataBufferOffsetIndex[k]][valRawByte].G / LUT_MAX_VAL);
                                    double percentBlue = (ChannelLuts[DEFAULT_CHANNEL_GROUP][dataBufferOffsetIndex[k]][valRawByte].B / LUT_MAX_VAL);

                                    double total = percentRed + percentGreen + percentBlue;

                                    if (total > 0)
                                    {
                                        percentRed /= total;
                                        percentGreen /= total;
                                        percentBlue /= total;

                                        pdData[i] = maxRed = Math.Max(maxRed, (byte)(valRaw * percentRed * scaleFactor));
                                        pdData[i + 1] = maxGreen = Math.Max(maxGreen, (byte)(valRaw * percentGreen * scaleFactor));
                                        pdData[i + 2] = maxBlue = Math.Max(maxBlue, (byte)(valRaw * percentBlue * scaleFactor));
                                    }

                                }
                            }

                            //copy the pixel data into the bitmap
                            _bitmap.WritePixels(new Int32Rect(0, 0, w, h), pdData, rawStride, 0);
                        }
                    }
                    break;
            }

            return null;
        }

        internal WriteableBitmap BuildBitmap16()
        {
            if (FrameData == null) return null;
            ushort[] pd = FrameData.pixelData;

            //verify pixel data is available
            if (pd == null)
            {
                return _bitmap16;
            }

            //if (false == BuildChannelPalettes())
            //{
            //    return null;
            //}
            int width = FrameData.frameInfo.imageWidth;
            int height = FrameData.frameInfo.imageHeight;
            switch (FrameData.frameInfo.channels)
            {
                case 1:
                    {
                        // Define parameters used to create the BitmapSource.
                        PixelFormat pf = PixelFormats.Gray16;

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

                        if ((pd.Length / FrameData.frameInfo.channels) == (width * height))
                        {
                            int buffSize = pd.Length / FrameData.frameInfo.channels;

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

                                    double percentRed = (ChannelLuts[DEFAULT_CHANNEL_GROUP][dataBufferOffsetIndex[k]][valRawByte].R / LUT_MAX_VAL);
                                    double percentGreen = (ChannelLuts[DEFAULT_CHANNEL_GROUP][dataBufferOffsetIndex[k]][valRawByte].G / LUT_MAX_VAL);
                                    double percentBlue = (ChannelLuts[DEFAULT_CHANNEL_GROUP][dataBufferOffsetIndex[k]][valRawByte].B / LUT_MAX_VAL);

                                    double total = percentRed + percentGreen + percentBlue;

                                    if (total > 0)
                                    {
                                        percentRed /= total;
                                        percentGreen /= total;
                                        percentBlue /= total;

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

            return _bitmap16;
        }


        internal WriteableBitmap BuildBitmapOrBitmap16ForOneChannel(bool is16Bit, int channel)
        {
            // special version of the bitmap builder that writes out one channel of a multichannel frame

            if (FrameData == null)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, "BuildBitmapOrBitmap16ForOneChannel called with no frame data");
                return null;
            }
            ushort[] pd = FrameData.pixelData;

            //verify pixel data is available
            if (pd == null)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, "BuildBitmapOrBitmap16ForOneChannel - frame contained null pixel data");
                return null;
            }

            //verify that the channel is in range
            if (channel >= FrameData.frameInfo.channels)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, $"BuildBitmapOrBitmap16ForOneChannel channel requested ({channel}) is out of range ({FrameData.frameInfo.channels}) for frame");
                return null;
            }

            int width = FrameData.frameInfo.imageWidth;
            int height = FrameData.frameInfo.imageHeight;

            // Define parameters used to create the BitmapSource.
            PixelFormat pf = is16Bit ? PixelFormats.Gray16 : PixelFormats.Gray8;
            int rawStride = (width * pf.BitsPerPixel + 7) / 8;

            WriteableBitmap bitmapRef = is16Bit ? _bitmap16 : _bitmap;

            //create a new bitmap when one does not exist or the size of the image changes
            if (bitmapRef == null)
            {
                bitmapRef = new WriteableBitmap(width, height, 96, 96, pf, null);
            }
            else
            {
                if (bitmapRef.Width != width || bitmapRef.Height != height || bitmapRef.Format != pf)
                {
                    bitmapRef = new WriteableBitmap(width, height, 96, 96, pf, null);
                }
            }

            int w = bitmapRef.PixelWidth;
            int h = bitmapRef.PixelHeight;
            int pd_offset = width * height * channel;

            if (is16Bit)
            {
                bitmapRef.WritePixels(new Int32Rect(0, 0, w, h), pd, rawStride, pd_offset);
                _bitmap16 = bitmapRef;
            }
            else // 8-bit
            {
                //Scale the image data for different camera bit depths
                //Scale factor is the target bit depth divided by the image bit depth
                //scaled array values need to be 8bit each to properly fill the image with the calculated stride
                double scaleFactor = Math.Pow(2, pf.BitsPerPixel) / Math.Pow(2, _frameData.bitsPerPixel);
                byte[] scaledPixelDataCopy = new byte[_frameData.pixelData.Length];
                for (int i = 0; i < scaledPixelDataCopy.Length; i++)
                { scaledPixelDataCopy[i] = (byte)(_frameData.pixelData[i] * scaleFactor); }

                if (pd.Length == width * height)
                {
                    //copy the scaled pixel data into the _bitmap
                    bitmapRef.WritePixels(new Int32Rect(0, 0, w, h), scaledPixelDataCopy, rawStride, 0);
                }
                _bitmap = bitmapRef;
            }

            return bitmapRef;
        }

        internal void CreateOrthogonalBitmap(int index, int totalNumOfZstack)
        {
            PixelFormat pf = PixelFormats.Rgb24;
            int step = pf.BitsPerPixel / 8;
            if (0 == _zPreviewPixelData.Length || null == _zPreviewPixelData[0]) return;

            FrameInfoStruct lastFrameInfo = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo;
            int length = lastFrameInfo.imageWidth * lastFrameInfo.imageHeight;

            /*if (length * ImageViewMBase.MAX_CHANNELS != _zPreviewPixelData[index].Length)
            {
                CaptureFile imageType = CaptureFile.FILE_TIFF;
                //load the images
                string[] fileNames = GetZImageFileNames(1, index + 1, ref imageType); // get first image
                if (!UpdateChannelData(fileNames, index, 1, imageType))
                {
                    return;
                }
            }*/

            ushort[] position = new ushort[ImageViewMBase.MAX_CHANNELS]; //store the point of grayscale image which used to calculate the RGB value

            //Create the XZ orthogonal view
            for (int j = 0; j < lastFrameInfo.imageWidth; j++)
            {
                for (int k = 0; k < MAX_CHANNELS; k++)
                {
                    if (ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP][k] && (FrameData?.channelSelection & (0x0001 << k)) > 0)
                    {
                        if (VirtualZStack)
                        {
                            int _indexPixelData = k * length + Convert.ToInt32(Math.Floor(OrthogonalViewPosition.Y)) * lastFrameInfo.imageWidth + j;
                            if (_indexPixelData < _zPreviewPixelData[index].Length)
                                position[k] = _zPreviewPixelData[index][_indexPixelData];
                            else return;
                        }
                        //else
                        //{
                        //    position[channelIndex] = PixelData[DataBufferOffsetIndex[channelIndex] * length + Convert.ToInt32(Math.Floor(OrthogonalViewPosition.Y)) * _lastFrameInfo.imageWidth + v];
                        //}
                    }
                }
                byte[] color = UpdatePixelDatabyte(position); //calculate the RGB value by current color mapping table

                for (int k = 0; k < 3; k++)
                {
                    _pdXZ[index * lastFrameInfo.imageWidth * step + j * 3 + k] = color[k]; // assign the RGB value
                }
            }
            //Create the YZ orthogonal view
            for (int j = 0; j < lastFrameInfo.imageHeight; j++)
            {
                for (int k = 0; k < MAX_CHANNELS; k++)
                {
                    if (ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP][k] && (FrameData?.channelSelection & (0x0001 << k)) > 0)
                    {
                        if (VirtualZStack)
                        {
                            int _indexPixelData = k * length + j * lastFrameInfo.imageWidth + Convert.ToInt32(Math.Floor(OrthogonalViewPosition.X));
                            if (_indexPixelData < _zPreviewPixelData[index].Length)
                                position[k] = _zPreviewPixelData[index][_indexPixelData];
                            else return;
                        }
                        //else
                        //{
                        //    position[channelIndex] = PixelData[_imageViewM.DataBufferOffsetIndex[channelIndex] * length + v * _imageViewM.DataWidth + Convert.ToInt32(Math.Floor(OrthogonalViewPosition.X))];
                        //}
                    }
                }

                byte[] color = UpdatePixelDatabyte(position); //calculate the RGB value by current color mapping table
                for (int k = 0; k < 3; k++)
                {
                    _pdYZ[j * totalNumOfZstack * step + index * step + k] = color[k]; // assign the RGB value
                }
            }
        }

        protected virtual void BuildBitmapsTask()
        {
            do
            {
                bool pixelDataResult = true;
                int scanAreaIndex = 0, channel;
                bool wpbpChanged = false;
                //bool updatemROIBackgroundImage = false;

                if (false == _frameDataSet || false == _updatePixelData)
                {
                    Thread.Sleep(1);
                    continue;
                }

                if (_frameDataSet && EnableHistogramUpdate)
                {
                    wpbpChanged = DidHistogramUpdate(out scanAreaIndex, out channel);
                }

                try
                {
                    if ((_frameDataSet && _frameDataHistory.IsFrameDataUpdated()) || ((wpbpChanged || _palletChanged || _chanEnableChanged || _tileViewToggled || _aspectRatioChanged || (_updateBackgroundImage && _allowmROIBackgroundImage)) && _imagesGrid != null))
                    {
                        AutoEnhanceIfEnabled();

                        bool updatePixelData = _frameDataHistory.IsFrameDataUpdated() || wpbpChanged || _palletChanged;

                        //New Image from camera, last area copied is also the updated histogram, and histogram changed
                        if (_frameDataHistory.IsFrameDataUpdated() && _currentAreaIndex == scanAreaIndex && wpbpChanged)
                        {
                            wpbpChanged = false;
                            pixelDataResult = Create24BitPixelDataByteRawAndComposite(updatePixelData, _currentAreaIndex).Result;
                        }
                        //new image from camera only. Indicates copy is finished. This is what is called during normal acquisition
                        //currentAreaIndex represents the last updated scan area
                        else if (_frameDataHistory.IsFrameDataUpdated())
                        {
                            BuildAvailableAreasHelper(updatePixelData, wpbpChanged, scanAreaIndex, ref pixelDataResult);
                            if (scanAreaIndex < 0)
                            {
                                wpbpChanged = false;
                            }
                        }

                        //These only get called if the update is not part of a new frame.
                        //:TODO: Find a better way to update only composite when one of the channel's histogram wp, bp changed
                        if ((_chanEnableChanged || _palletChanged || _tileViewToggled || WhitePointIncreased) && IsInSequentialMode && EnableHistogramUpdate)
                        {
                            WhitePointIncreased = false;
                            for (int i = 0; i < ChannelsPerSequence.Count; i++)
                            {
                                pixelDataResult = Create24BitPixelDataByteRawAndComposite(updatePixelData, i).Result;
                                CreateSequentialXYBitmap();
                            }
                        }
                        else if (wpbpChanged && scanAreaIndex >= 0) //HistogramUpdate, no need to build all areas
                        {
                            BuildAvailableAreasHelper(updatePixelData, wpbpChanged, scanAreaIndex, ref pixelDataResult);
                            wpbpChanged = false;
                        }
                        else if (wpbpChanged || _palletChanged)
                        {
                            //case for a current index of -1 is not compatible with mROI
                            if (_frameDataHistory.FrameDataList[_currentAreaIndex].StoredFrameData.frameInfo.isMROI == 1)
                            {
                                BuildAvailableAreasHelper(updatePixelData, wpbpChanged, scanAreaIndex, ref pixelDataResult);
                            }
                            else
                            {
                                if (scanAreaIndex >= 0)
                                {
                                    _frameDataHistory.FrameDataList[scanAreaIndex].FrameDataUpdated = false;
                                    _frameDataHistory.FrameDataList[scanAreaIndex].NeedsBitmapUpdate = true;
                                }
                                pixelDataResult = Create24BitPixelDataByteRawAndComposite(updatePixelData, scanAreaIndex).Result;
                            }
                        }
                        else if (_chanEnableChanged)
                        {
                            if (_currentAreaIndex >= 0)
                            {
                                _frameDataHistory.FrameDataList[_currentAreaIndex].FrameDataUpdated = false;
                                _frameDataHistory.FrameDataList[_currentAreaIndex].NeedsBitmapUpdate = true;
                                pixelDataResult = Create24BitPixelDataByteRawAndComposite(updatePixelData, _currentAreaIndex).Result;
                            }
                        }
                        else if (_aspectRatioChanged || _tileViewToggled)
                        {
                            foreach (FrameDataHistoryElement element in _frameDataHistory.FrameDataList)
                            {
                                element.NeedsBitmapUpdate = true;
                            }
                        }

                        _chanEnableChanged = false;
                        _tileViewToggled = false;
                        _palletChanged = false;
                        _aspectRatioChanged = false;
                        _updateBitmap = true;
                    }
                }
                catch (Exception ex)
                {
                    ex.ToString();
                }

                if (_redisplayAllROIs)
                {
                    //TODO: Check if this is ever used
                    _redisplayAllROIs = false;
                    var fh = _frameDataHistory.FrameDataList.ToList(); //shallowCopy
                    for (int i = 0; i < fh?.Count; ++i)
                    {
                        pixelDataResult = Create24BitPixelDataByteRawAndComposite(false, i).Result;

                        if (1 == _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.isMROI && _mROISpatialDisplaybleEnable)
                        {
                            CreateXYmROISpatialBitmap();
                        }
                        else
                        {
                            CreateXYBitmap();
                        }
                        _updateBitmap = false;

                    }
                }
                if (_updateBitmap && _frameDataHistory.LastUpdatedFrame != null && pixelDataResult)
                {
                    if (1 == _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo.isMROI && _mROISpatialDisplaybleEnable)
                    {
                        CreateXYmROISpatialBitmap();
                    }
                    else if (IsInSequentialMode)
                    {
                        CreateSequentialXYBitmap();
                    }
                    else
                    {
                        CreateXYBitmap();
                    }
                    _updateBitmap = false;
                }
                else if (!pixelDataResult)
                {
                    _updateBitmap = false;
                }
            }
            while (_runBitmapBuildingThread);
        }

        protected Task<bool> Create24BitPixelDataByteRawAndComposite(bool updatePixelData, int areaIndex)
        {
            if (areaIndex == -1)
            {
                return Create24BitPixelDataByteRawAndComposite(updatePixelData, _prevAreaIndex);
            }
            else
            {
                _prevAreaIndex = areaIndex;
            }

            if (_frameDataHistory == null || _frameDataHistory.FrameDataList.Length <= areaIndex || _frameDataHistory.FrameDataList[areaIndex] == null || _frameDataHistory.FrameDataList[areaIndex].StoredFrameData == null)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, "Error during Create24BitPixelDataByteRawAndComposite: incoming data is not valid");
                return Task.FromResult(false);
            }

            FrameData fd = _frameDataHistory.FrameDataList[areaIndex].StoredFrameData;

            lock (fd.dataLock)
            {
                if (null == _rawImg)
                {
                    return Task.FromResult(false);
                }

                //no reset of histogram in partial frame
                bool createBackupComposite = _imageReviewType == ImageViewType.CaptureSetup && (int)MVMManager.Instance["AreaControlViewModel", "MesoMicroVisible"] == 1
                    && fd.frameInfo.isMROI != 1; // Only allocate backup composite if in capture setup and mROI is possible while imaging full FOV
                bool resetPixelDataHistogram = 1 == fd.frameInfo.fullFrame && updatePixelData;
                int planes = fd.frameInfo.numberOfPlanes > 1 ? fd.frameInfo.numberOfPlanes : 1;
                int scanAreas = fd.frameInfo.totalScanAreas > 1 ? fd.frameInfo.totalScanAreas : 1;
                int sequences = fd.frameInfo.totalSequences;
                int verticalImages = scanAreas > 1 ? scanAreas : planes;
                verticalImages = IsInSequentialMode ? sequences : verticalImages;
                int scanAreaIndex = IsInSequentialMode ? fd.frameInfo.sequenceIndex : fd.frameInfo.scanAreaIndex;
                int numChannels = /*fd.frameInfo.channels*/4;
                int dataLengthPerImage = fd.frameInfo.imageHeight * fd.frameInfo.imageWidth;

                if (createBackupComposite)
                {
                    CopyHistoryElementToTempFrameData(_frameDataHistory.FrameDataList[areaIndex]);
                }

                //need to rebuid the color image because a palette option is not available for RGB images
                if (fd.frameInfo.channels > 0 && fd.pixelData != null && dataLengthPerImage * fd.frameInfo.channels * planes <= fd.pixelData.Length)
                {
                    if (scanAreas <= 1 && sequences <= 1)
                    {
                        for (int k = 0; k < numChannels; k++)
                        {
                            //  if ((FrameData.channelSelection & (0x0001 << k)) > 0) //TODO: make sure this is not needed
                            {
                                if (_rawImg[k] == null || _rawImg[k].Length != planes)
                                {
                                    _rawImg[k] = new byte[planes][];
                                }
                                for (int p = 0; p < planes; p++)
                                {
                                    if (_rawImg[k][p] == null || _rawImg[k][p].Length != 3 * dataLengthPerImage)
                                    {
                                        _rawImg[k][p] = new byte[3 * dataLengthPerImage];
                                    }
                                }
                            }
                        }

                        if (_24BitPixelDataChannelComposite == null || _24BitPixelDataChannelComposite.Length != verticalImages)
                        {
                            _24BitPixelDataChannelComposite = new byte[verticalImages][];
                            if (createBackupComposite)
                            {
                                _24BitPixelDataForBackground = new byte[verticalImages][];
                            }
                        }

                        for (int v = 0; v < verticalImages; v++)
                        {
                            if (_24BitPixelDataChannelComposite[v]?.Length != 3 * dataLengthPerImage)
                            {
                                _24BitPixelDataChannelComposite[v] = new byte[3 * dataLengthPerImage];
                                if (createBackupComposite)
                                {
                                    _24BitPixelDataForBackground[v] = new byte[3 * dataLengthPerImage];
                                    _backgroundWidth = fd.frameInfo.imageWidth;
                                    _backgroundHeight = fd.frameInfo.imageHeight;
                                }
                            }
                            else
                            {
                                Array.Clear(_24BitPixelDataChannelComposite[v], 0, 3 * dataLengthPerImage);
                                if (createBackupComposite)
                                {
                                    Array.Clear(_24BitPixelDataForBackground[v], 0, 3 * dataLengthPerImage);
                                }
                            }
                        }
                    }
                    else
                    {
                        for (int k = 0; k < numChannels; k++)
                        {
                            if (_rawImg[k] == null || _rawImg[k].Length != verticalImages)
                            {
                                _rawImg[k] = new byte[verticalImages][];
                            }
                            // for (int p = 0; p < verticalImages; p++)
                            {
                                if (_rawImg[k][scanAreaIndex] == null || _rawImg[k][scanAreaIndex].Length != 3 * dataLengthPerImage)
                                {
                                    _rawImg[k][scanAreaIndex] = new byte[3 * dataLengthPerImage];
                                }
                            }
                        }

                        if (_24BitPixelDataChannelComposite == null || _24BitPixelDataChannelComposite.Length < verticalImages)
                        {
                            _24BitPixelDataChannelComposite = new byte[verticalImages][];
                        }

                        if (_24BitPixelDataChannelComposite[scanAreaIndex]?.Length != 3 * dataLengthPerImage)
                        {
                            _24BitPixelDataChannelComposite[scanAreaIndex] = new byte[3 * dataLengthPerImage];
                        }
                        else
                        {
                            Array.Clear(_24BitPixelDataChannelComposite[scanAreaIndex], 0, 3 * dataLengthPerImage);
                        }

                        if (1 == fd.frameInfo.isMROI)
                        {
                            for (int k = 0; k < numChannels; k++)
                            {
                                if (_rawImgmROIPriorityDisplayAreaComposite[k] == null || _rawImgmROIPriorityDisplayAreaComposite[k].Length != verticalImages)
                                {
                                    _rawImgmROIPriorityDisplayAreaComposite[k] = new byte[verticalImages][];
                                }
                                // for (int p = 0; p < verticalImages; p++)
                                {
                                    if (_rawImgmROIPriorityDisplayAreaComposite[k][scanAreaIndex] == null || _rawImgmROIPriorityDisplayAreaComposite[k][scanAreaIndex].Length != 3 * dataLengthPerImage)
                                    {
                                        _rawImgmROIPriorityDisplayAreaComposite[k][scanAreaIndex] = new byte[3 * dataLengthPerImage];
                                    }
                                }
                            }

                            if (_24BitmROIPriorityDisplayAreaComposite == null || _24BitmROIPriorityDisplayAreaComposite.Length < verticalImages)
                            {
                                _24BitmROIPriorityDisplayAreaComposite = new byte[verticalImages][];
                            }

                            if (_24BitmROIPriorityDisplayAreaComposite[scanAreaIndex]?.Length != 3 * dataLengthPerImage)
                            {
                                _24BitmROIPriorityDisplayAreaComposite[scanAreaIndex] = new byte[3 * dataLengthPerImage];
                            }
                        }
                    }

                    int shiftValue = _bitsPerPixel - 8;

                    if (resetPixelDataHistogram)
                    {
                        foreach (var pixelDataHistogram in _pixelDataHistograms)
                        {
                            pixelDataHistogram.Min = ushort.MaxValue;
                            pixelDataHistogram.Max = ushort.MinValue;
                        }
                    }

                    //in the interest of speed we are seperating the reference channel case
                    //without a reference channel the logic will run faster since the
                    //conditionals per pixel are removed.
                    bool refImageInvalidated;
                    bool didRefImage = Process24BitRefImage(resetPixelDataHistogram, numChannels, verticalImages, dataLengthPerImage, shiftValue, fd, out refImageInvalidated);

                    if (!didRefImage)
                    {
                        int enabledChannelCount = 0;
                        int displayChannelCount = 0;
                        for (int i = 0; i < ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP].Count; ++i)
                        {
                            if ((fd.channelSelection & (0x0001 << i)) > 0)
                            {
                                ++enabledChannelCount;

                                if (ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP][i])
                                {
                                    ++displayChannelCount;
                                }
                            }
                        }

                        //factor is derived by taking CAMERA_MAX_INTENSITY_VALUE/256
                        //const double FULL_RANGE_NORMALIZATION_FACTOR = 64.0;
                        try
                        {
                            if (scanAreas <= 1 && sequences <= 1)
                            {
                                for (int v = 0; v < verticalImages; v++)
                                {
                                    Process24BitImage(numChannels, resetPixelDataHistogram, v, dataLengthPerImage, shiftValue, verticalImages, enabledChannelCount, displayChannelCount, fd);
                                }
                            }
                            else
                            {
                                Process24BitImage(numChannels, resetPixelDataHistogram, scanAreaIndex, dataLengthPerImage, shiftValue, 1, enabledChannelCount, displayChannelCount, fd);
                            }
                        }
                        catch (Exception ex)
                        {
                            ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, "Error with processing: " + ex.ToString());
                            ex.ToString();
                        }
                    }
                    else if (refImageInvalidated)
                    {
                        return Task.FromResult(false);
                    }

                    if (scanAreas > 1 && 1 == fd.frameInfo.isMROI)
                    {
                        for (int k = 0; k < numChannels; k++)
                        {
                            Array.Copy(_rawImg[k][scanAreaIndex], _rawImgmROIPriorityDisplayAreaComposite[k][scanAreaIndex], _rawImgmROIPriorityDisplayAreaComposite[k][scanAreaIndex].Length);
                        }
                        Array.Copy(_24BitPixelDataChannelComposite[scanAreaIndex], _24BitmROIPriorityDisplayAreaComposite[scanAreaIndex], _24BitmROIPriorityDisplayAreaComposite[scanAreaIndex].Length);
                    }
                }

                bool dflimDisplayLifetimeImage = (bool)MVMManager.Instance[DFLIMControlViewModelName, "DFLIMDisplayLifetimeImage", (object)false];
                if (((int)BufferType.DFLIM_IMAGE == fd.frameInfo.bufferType ||
                    (int)BufferType.DFLIM_ALL == fd.frameInfo.bufferType) &&
                    dflimDisplayLifetimeImage)
                {
                    CreatePixelDataByteDFLIM(); //TODO: add frame data fd
                }

                if (fd.frameInfo.isMROI == 1 && _frameDataHistory.FrameDataList.Length == fd.frameInfo.totalScanAreas)
                {
                    //_frameDataHistory.FrameDataList[fd.frameInfo.scanAreaIndex].StoredLastFrameInfo = fd.frameInfo;
                    //_frameDataHistory.FrameInfoSet = true; //TODO Moved these to the copy data method?
                    _frameDataHistory.LastUpdatedIndex = scanAreaIndex;
                }
                else
                {
                    _frameDataHistory.LastUpdatedIndex = areaIndex;
                }
            }

            return Task.FromResult(true);
        }

        // Dictionary<string, WriteableBitmap> _bitmaps;
        protected bool CreateSequentialXYBitmap()
        {
            if (_frameDataHistory == null || _frameDataHistory.LastUpdatedFrame == null) return false;// || _frameData.Length <= currentFraDataIndex || _frameData[currentFraDataIndex] == null) return false;

            FrameData fd = _frameDataHistory.LastUpdatedFrame.StoredFrameData;
            if (null == _24BitPixelDataChannelComposite)
            {
                return false;
            }

            // Define parameters used to create the BitmapSource.
            FrameInfoStruct lastFrameInfo = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo;

            PixelFormat pixelFormat = PixelFormats.Rgb24;
            int width = lastFrameInfo.imageWidth;
            int height = lastFrameInfo.imageHeight;
            int rawStride = (width * pixelFormat.BitsPerPixel + 7) / 8;

            bool dflimDisplayLifetimeImage = (bool)MVMManager.Instance[DFLIMControlViewModelName, "DFLIMDisplayLifetimeImage", (object)false];
            bool doLifetime = ((int)BufferType.DFLIM_IMAGE == lastFrameInfo.bufferType || (int)BufferType.DFLIM_ALL == lastFrameInfo.bufferType) && dflimDisplayLifetimeImage;
            bool tile = TileDisplay;

            if (Application.Current == null) return false;
            _ = (Application.Current?.Dispatcher.Invoke(DispatcherPriority.Normal,
                new Action(delegate ()
                {
                    try
                    {
                        bool setOverlayItemsOnce = false;
                        bool isVerticalLayout = VerticalTileDisplay && lastFrameInfo.totalSequences <= 1; // only do Horizontal layout for multiplane

                        if (null == _imagesGrid || lastFrameInfo.totalSequences + 1 != _imagesGrid.Count)
                        {

                            _imagesGrid = new ObservableCollection<ObservableCollection<CompoundImage>>();
                            for (int i = 0; i < lastFrameInfo.totalSequences + 1; i++)
                            {
                                _imagesGrid.Add(new ObservableCollection<CompoundImage>());
                            }
                        }

                        // composite
                        if (_imagesGrid[0].Count == 0)
                        {
                            CompoundImage compoundImage = new CompoundImage();
                            compoundImage.ImageXY = new WriteableBitmap(width, height, 96, 96, pixelFormat, null);
                            compoundImage.Selected -= CompoundImage_Selected;
                            compoundImage.Selected += CompoundImage_Selected;
                            if (!setOverlayItemsOnce)
                            {
                                var list = compoundImage.OverlayItems;
                                OverlayManager.OverlayManagerClass.Instance.GetDuplicatedROIList(ref list);
                                compoundImage.OverlayItems = list;
                                OverlayManager.OverlayManagerClass.Instance.SetOverlayItems(compoundImage.OverlayItems, false);
                                setOverlayItemsOnce = true;
                            }
                            _imagesGrid[0].Add(compoundImage);
                        }

                        var pixelData = _24BitPixelDataChannelComposite[0];
                        _imagesGrid[0][0].ImageXY.WritePixels(new Int32Rect(0, 0, width, height), pixelData, rawStride, 0);

                        _imagesGrid[lastFrameInfo.sequenceIndex + 1].Clear();

                        if (tile || doLifetime)
                        {
                            int cnt = -1;
                            for (int j = 0; j < MAX_CHANNELS; j++)
                            {
                                if ((lastFrameInfo.sequenceSelectedChannels & (0x0001 << j)) > 0)
                                {
                                    cnt++;
                                }
                                if (ChannelDisplayEnable[lastFrameInfo.sequenceIndex][j] && (lastFrameInfo.sequenceSelectedChannels & (0x0001 << j)) > 0)
                                {
                                    CompoundImage compoundImage = new CompoundImage();
                                    compoundImage.ImageXY = new WriteableBitmap(width, height, 96, 96, pixelFormat, null);
                                    compoundImage.Selected -= CompoundImage_Selected;
                                    compoundImage.Selected += CompoundImage_Selected;
                                    if (!setOverlayItemsOnce)
                                    {
                                        var list = compoundImage.OverlayItems;
                                        OverlayManager.OverlayManagerClass.Instance.GetDuplicatedROIList(ref list);
                                        compoundImage.OverlayItems = list;
                                        OverlayManager.OverlayManagerClass.Instance.SetOverlayItems(compoundImage.OverlayItems, false);
                                        setOverlayItemsOnce = true;
                                    }

                                    int offset = 0;
                                    if (!fd.contiguousChannels)
                                    {
                                        offset = (1 == ChannelsPerSequence[lastFrameInfo.sequenceIndex].Item1) ? 0 : j;
                                    }
                                    else
                                    {
                                        offset = cnt;
                                    }

                                    pixelData = _rawImg[offset][lastFrameInfo.sequenceIndex];
                                    compoundImage.ImageXY.WritePixels(new Int32Rect(0, 0, width, height), pixelData, rawStride, 0);
                                    _imagesGrid[lastFrameInfo.sequenceIndex + 1].Add(compoundImage);

                                }
                            }
                        }

                        if (_imagesGrid.Count - 2 == lastFrameInfo.sequenceIndex)
                        {
                            EnableHistogramUpdate = true;
                        }

                        UpdateOverlayManager(width, height, 1.0, 1.0);
                    }
                    catch (Exception ex)
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, GetType().Name + " exception thrown: " + ex.Message);
                    }
                })));
            if (_calculateBitmapUpdateRate)
            {
                _frameCount++;
                _endTime = DateTimeOffset.Now.ToUnixTimeMilliseconds();
                if (_endTime - _startTime >= 2000)
                {
                    _bitmapUpdateRate = (double)_frameCount / ((_endTime - _startTime) / 1000.0);
                    _startTime = _endTime;
                    BitmapUpdateRateUpdated?.BeginInvoke(null, null);
                    _frameCount = 0;
                }
            }
            BitmapUpdated?.BeginInvoke(null, null);
            return true;
        }

        //TODO: Does this need to check if the stored frame data is null as well? Should stress test.
        private long _startTime = 0;
        private long _endTime = 0;
        private int _frameCount = 0;
        protected bool CreateXYBitmap()
        {
            if (_frameDataHistory == null || _frameDataHistory.LastUpdatedFrame == null) return false;// || _frameData.Length <= currentFraDataIndex || _frameData[currentFraDataIndex] == null) return false;

            FrameData fd = _frameDataHistory.FrameDataList[_frameDataHistory.LastUpdatedIndex].StoredFrameData;
            if (null == _24BitPixelDataChannelComposite)
            {
                return false;
            }

            // Define parameters used to create the BitmapSource.
            FrameInfoStruct lastFrameInfo = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo;

            PixelFormat pixelFormat = PixelFormats.Rgb24;
            int planes = lastFrameInfo.numberOfPlanes > 1 ? lastFrameInfo.numberOfPlanes : 1;
            int scanAreas = lastFrameInfo.totalScanAreas;
            int currentAreaIndex = scanAreas > 1 ? lastFrameInfo.scanAreaIndex : 0;
            int vImages = scanAreas > 1 ? scanAreas : planes;
            bool isFastZPreview = fd.isFastZPreviewImage; //For fast Z preview in Runsample the FrameData copied is a Tile of 2 by 2 stack images

            int width = isFastZPreview ? lastFrameInfo.imageWidth / 2 : lastFrameInfo.imageWidth;
            int height = isFastZPreview ? lastFrameInfo.imageHeight / 2 : lastFrameInfo.imageHeight;
            double aspectRatioYScale = _displayPixelAspectRatio ? (double)lastFrameInfo.pixelAspectRatioYScale / 100.0 : 1.0;

            //TODO: implement logic to display mROI
            int outputBitmapWidth = isFastZPreview ? 2 : 1;
            int outputBitmapHeight = isFastZPreview ? 2 : 1;
            bool dflimDisplayLifetimeImage = (bool)MVMManager.Instance[DFLIMControlViewModelName, "DFLIMDisplayLifetimeImage", (object)false];
            bool doLifetime = ((int)BufferType.DFLIM_IMAGE == lastFrameInfo.bufferType || (int)BufferType.DFLIM_ALL == lastFrameInfo.bufferType) && dflimDisplayLifetimeImage;
            int numChannels = 0;

            bool[] channelEnable = new bool[ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP].Count];
            bool tile = TileDisplay;
            for (int i = 0; i < ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP].Count; ++i)
            {
                if (true == (ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP][i] && (fd.channelSelection & (0x0001 << i)) > 0))
                {
                    ++numChannels;
                    channelEnable[i] = true;
                }
                else
                {
                    channelEnable[i] = false;
                }
            }
            if (0 == numChannels)
            {
                return false;
            }

            // true if we should only show composites (no raw tiles)
            bool isCompositeOnly = numChannels == 1;

            if (!tile || lastFrameInfo.channels == 1)
            {
                // no tiles - only show composites
                isCompositeOnly = true;
                outputBitmapHeight *= vImages;
            }
            else if (lastFrameInfo.channels > 1 || doLifetime)
            {

                if (planes > 1 || scanAreas > 1)
                {
                    // account for composite as another channel, unless there's only composite
                    outputBitmapWidth *= isCompositeOnly ? 1 : numChannels + 1;
                    outputBitmapHeight *= vImages;
                }
                else
                {
                    switch (numChannels)
                    {
                        case 1:
                            {
                                if (doLifetime)
                                {
                                    outputBitmapWidth *= VerticalTileDisplay ? 1 : 2;
                                    outputBitmapHeight *= VerticalTileDisplay ? 2 : 1;
                                }
                            }
                            break;
                        case 2:
                            {
                                outputBitmapWidth *= VerticalTileDisplay ? 1 : 3;
                                outputBitmapHeight *= VerticalTileDisplay ? 3 : 1;
                            }
                            break;
                        case 3:
                            {
                                outputBitmapWidth *= 2;
                                outputBitmapHeight *= 2;
                            }
                            break;
                        default:
                            {
                                // All 4 Channels enabled
                                outputBitmapWidth *= VerticalTileDisplay ? 2 : 3;
                                outputBitmapHeight *= VerticalTileDisplay ? 3 : 2;
                            }
                            break;
                    }
                }
            }

            if (Application.Current == null) return false;
            _ = (Application.Current?.Dispatcher.Invoke(DispatcherPriority.Normal,
                new Action(delegate ()
                {
                    try
                    {
                        bool setOverlayItemsOnce = false;
                        bool isVerticalLayout = VerticalTileDisplay && vImages <= 1; // only do Horizontal layout for multiplane
                        int numImages = (numChannels + 1) * vImages; // +1 for composite
                        numImages *= isFastZPreview ? 4 : 1; //FastPreview has 4 images per FrameData arranged as 2x2

                        // use these to check for change in image size
                        int totalPixelWidth = outputBitmapWidth * width;
                        int totalPixelHeight = outputBitmapHeight * height;

                        /*loop through the available history elements and compare to the image grid to see which need bitmap updates
                         * For mROI, this will initialize the bitmaps twich but only require a single update for subsequent frames.
                         * This was done because there is not a guarantee that all regions will be available on first loop. This allows
                         * the bitmaps to be shown in the grid but they will blank until updated 
                        */
                        for (int e = 0; e < _frameDataHistory.FrameDataList.Length; e++)
                        {
                            FrameDataHistoryElement element = _frameDataHistory.FrameDataList[e];
                            if (element != null)
                            {
                                currentAreaIndex = scanAreas > 1 ? element.StoredLastFrameInfo.scanAreaIndex : 0;
                                width = isFastZPreview ? element.StoredLastFrameInfo.imageWidth / 2 : element.StoredLastFrameInfo.imageWidth;
                                height = isFastZPreview ? element.StoredLastFrameInfo.imageHeight / 2 : element.StoredLastFrameInfo.imageHeight;
                            }

                            if (null == _imagesGrid || !_imageGridParameters.Compare(outputBitmapWidth, outputBitmapHeight, width, height, isVerticalLayout, numChannels, vImages, aspectRatioYScale, currentAreaIndex))
                            {
                                if (_imageGridParameters.Columns != outputBitmapWidth || _imageGridParameters.Rows != outputBitmapHeight || null == _imagesGrid)
                                {
                                    // create a new bitmap with new grid parameters
                                    _imageGridParameters.Columns = outputBitmapWidth;
                                    _imageGridParameters.Rows = outputBitmapHeight;
                                    _imageGridParameters.Channels = numChannels;
                                    _imageGridParameters.Planes = vImages;
                                    _imageGridParameters.IsVertical = isVerticalLayout;
                                    _imageGridParameters.PixelWidth = new Dictionary<int, int>();
                                    _imageGridParameters.PixelWidth[currentAreaIndex] = width;
                                    _imageGridParameters.PixelHeight = new Dictionary<int, int>();
                                    _imageGridParameters.PixelHeight[currentAreaIndex] = height;
                                    _imageGridParameters.AspectRatioYScale = aspectRatioYScale;

                                    _imagesGrid = new ObservableCollection<ObservableCollection<CompoundImage>>();

                                    int tmpCount = 0;
                                    for (int i = 0; i < _imageGridParameters.Rows; i++)
                                    {
                                        _imagesGrid.Add(new ObservableCollection<CompoundImage>());
                                        for (int j = 0; j < _imageGridParameters.Columns; j++)
                                        {
                                            if (isFastZPreview && 1 >= planes && 1 >= scanAreas && 4 == numChannels && i >= _imageGridParameters.Rows - 2 && j >= _imageGridParameters.Columns - 2)
                                            {
                                                //This is a special case for when tile view is enabled, with 4 channels in FastZ Preview. This helps so the tile image grid is built in the right order
                                                continue;
                                            }

                                            CompoundImage compoundImage = new CompoundImage();
                                            compoundImage.ImageXY = new WriteableBitmap(width, height, 96, 96 / aspectRatioYScale, pixelFormat, null);
                                            compoundImage.Selected -= CompoundImage_Selected;
                                            compoundImage.Selected += CompoundImage_Selected;
                                            if (!setOverlayItemsOnce)
                                            {
                                                var list = compoundImage.OverlayItems;
                                                OverlayManager.OverlayManagerClass.Instance.GetDuplicatedROIList(ref list);
                                                compoundImage.OverlayItems = list;
                                                OverlayManager.OverlayManagerClass.Instance.SetOverlayItems(compoundImage.OverlayItems, false);
                                                setOverlayItemsOnce = true;
                                            }
                                            _imagesGrid[i].Add(compoundImage);

                                            if (++tmpCount >= numImages)
                                            {
                                                break;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    // create a new bitmap with new grid parameters
                                    _imageGridParameters.Columns = outputBitmapWidth;
                                    _imageGridParameters.Rows = outputBitmapHeight;
                                    _imageGridParameters.Channels = numChannels;
                                    _imageGridParameters.Planes = vImages;
                                    _imageGridParameters.IsVertical = isVerticalLayout;
                                    //_imageGridParameters.PixelWidth = new Dictionary<int, int>();
                                    _imageGridParameters.PixelWidth[currentAreaIndex] = width;
                                    //_imageGridParameters.PixelHeight = new Dictionary<int, int>();
                                    _imageGridParameters.PixelHeight[currentAreaIndex] = height;
                                    _imageGridParameters.AspectRatioYScale = aspectRatioYScale;

                                    for (int i = 0; i < _imageGridParameters.Rows; i++)
                                    {
                                        for (int j = 0; j < _imageGridParameters.Columns; j++)
                                        {
                                            if (scanAreas > 1)
                                            {
                                                if (currentAreaIndex == i)
                                                {
                                                    CompoundImage compoundImage = new CompoundImage();
                                                    compoundImage.ImageXY = new WriteableBitmap(width, height, 96, 96 / aspectRatioYScale, pixelFormat, null);
                                                    compoundImage.Selected -= CompoundImage_Selected;
                                                    compoundImage.Selected += CompoundImage_Selected;
                                                    if (!setOverlayItemsOnce)
                                                    {
                                                        var list = compoundImage.OverlayItems;
                                                        OverlayManager.OverlayManagerClass.Instance.GetDuplicatedROIList(ref list);
                                                        compoundImage.OverlayItems = list;
                                                        OverlayManager.OverlayManagerClass.Instance.SetOverlayItems(compoundImage.OverlayItems, false);
                                                        setOverlayItemsOnce = true;
                                                    }
                                                    _imagesGrid[i][j] = compoundImage;
                                                }
                                            }
                                            else
                                            {
                                                if (isFastZPreview && 1 >= planes && 1 >= scanAreas && 4 == numChannels && i >= _imageGridParameters.Rows - 2 && j >= _imageGridParameters.Columns - 2)
                                                {
                                                    //This is a special case for when tile view is enabled, with 4 channels in FastZ Preview. This helps so the tile image grid is built in the right order
                                                    continue;
                                                }

                                                CompoundImage compoundImage = new CompoundImage();
                                                compoundImage.ImageXY = new WriteableBitmap(width, height, 96, 96 / aspectRatioYScale, pixelFormat, null);
                                                compoundImage.Selected -= CompoundImage_Selected;
                                                compoundImage.Selected += CompoundImage_Selected;
                                                if (!setOverlayItemsOnce)
                                                {
                                                    var list = compoundImage.OverlayItems;
                                                    OverlayManager.OverlayManagerClass.Instance.GetDuplicatedROIList(ref list);
                                                    compoundImage.OverlayItems = list;
                                                    OverlayManager.OverlayManagerClass.Instance.SetOverlayItems(compoundImage.OverlayItems, false);
                                                    setOverlayItemsOnce = true;
                                                }
                                                _imagesGrid[i][j] = compoundImage;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // list of enabled channels
                        var enabledChannels = new int[numChannels];
                        for (int i = 0, j = 0; i < channelEnable.Length && j < numChannels; i++)
                        {
                            if (channelEnable[i])
                            {
                                enabledChannels[j++] = i;
                            }
                        }
                        
                        /*
                         * Loop through all available history elements and see which have recent pixel updates. If no update, the data is not copied
                         */
                        int rawStride;
                        foreach (FrameDataHistoryElement element in _frameDataHistory.FrameDataList)
                        {
                            if (!element.NeedsBitmapUpdate && _imageReviewType != ImageViewType.Review) //Always update regions in review
                            {
                                continue;
                            }
                            currentAreaIndex = scanAreas > 1 ? element.StoredLastFrameInfo.scanAreaIndex : 0;
                            width = isFastZPreview ? element.StoredLastFrameInfo.imageWidth / 2 : element.StoredLastFrameInfo.imageWidth;
                            height = isFastZPreview ? element.StoredLastFrameInfo.imageHeight / 2 : element.StoredLastFrameInfo.imageHeight;
                            rawStride = (width * pixelFormat.BitsPerPixel + 7) / 8;

                            // enumerate through images grid (x,y) and write out a bitmap foreach channel (+ composite) foreach plane
                            int tmpImageCount = 0;
                            int channels = isCompositeOnly && !doLifetime ? 1 : numChannels + 1;
                            foreach (var xy in _imageGridParameters.GetGridEnumerator())
                            {
                                int x = xy.Item1;
                                int y = xy.Item2;
                                if (scanAreas > 1 && x != currentAreaIndex)
                                {
                                    ++tmpImageCount;
                                    continue;
                                }

                                int currentPlane = tmpImageCount / channels;

                                //If this is FastZ Preview skip the uneven coordinates, they will be handled in the loop below
                                if (isFastZPreview && (1 == x % 2 || 1 == y % 2))
                                {
                                    continue;
                                }

                                // composite
                                if (tmpImageCount % channels == 0)
                                {
                                    //If this is a FastZ preview image, we need to grab each of the 4 frames from FastZ Preview and place it in the right coordinate
                                    if (isFastZPreview)
                                    {
                                        int imageBitSize = (width * height * pixelFormat.BitsPerPixel + 7) / 8;
                                        var pixelData = new byte[imageBitSize];
                                        for (int i = 0; i < 4; i++)
                                        {
                                            int testx = x + (i >> 1);
                                            int testy = y + (i & 1);
                                            Array.Copy(_24BitPixelDataChannelComposite[currentPlane], imageBitSize * i, pixelData, 0, imageBitSize);
                                            _imagesGrid[x + (i >> 1)][y + (i & 1)].ImageXY.WritePixels(new Int32Rect(0, 0, width, height), pixelData, rawStride, 0);
                                        }
                                    }
                                    else
                                    {
                                        // Copy image data to bitmap grid
                                        var pixelData = _24BitPixelDataChannelComposite[currentPlane];
                                        _imagesGrid[x][y].ImageXY.WritePixels(new Int32Rect(0, 0, width, height), pixelData, rawStride, 0);
                                    }

                                }
                                else if (tile || doLifetime)
                                {
                                    // raw image data (if tile option is enabled)
                                    int currentChannel = tmpImageCount % channels - 1;
                                    if (false == fd.contiguousChannels)
                                    {
                                        currentChannel = enabledChannels[currentChannel];
                                    }

                                    //If this is a FastZ preview image, we need to grab each of the 4 frames from FastZ Preview and place it in the right coordinate
                                    if (isFastZPreview)
                                    {
                                        int imageBitSize = (width * height * pixelFormat.BitsPerPixel + 7) / 8;
                                        byte[] pixelData = new byte[imageBitSize];
                                        for (int i = 0; i < 4; i++)
                                        {
                                            int testx = x + (i >> 1);
                                            int testy = y + (i & 1);
                                            Array.Copy(_rawImg[currentChannel][currentPlane], imageBitSize * i, pixelData, 0, imageBitSize);
                                            _imagesGrid[x + (i >> 1)][y + (i & 1)].ImageXY.WritePixels(new Int32Rect(0, 0, width, height), pixelData, rawStride, 0);
                                        }
                                    }
                                    else
                                    {
                                        // Copy image data to bitmap grid
                                        var pixelData = _rawImg[currentChannel][currentPlane];
                                        _imagesGrid[x][y].ImageXY.WritePixels(new Int32Rect(0, 0, width, height), pixelData, rawStride, 0);
                                    }
                                }

                                if (++tmpImageCount >= numImages)
                                {
                                    break;
                                }
                            }
                            element.NeedsBitmapUpdate = false;
                        }

                        UpdateOverlayManager(width, height, 1.0, aspectRatioYScale);
                    }
                    catch (Exception ex)
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " Exception: " + ex.Message);
                    }
                })));

            if (_calculateBitmapUpdateRate)
            {
                _frameCount++;
                _endTime = DateTimeOffset.Now.ToUnixTimeMilliseconds();
                if (_endTime - _startTime >= 2000)
                {
                    _bitmapUpdateRate = (double)_frameCount / ((_endTime - _startTime) / 1000.0);
                    _startTime = _endTime;
                    BitmapUpdateRateUpdated?.BeginInvoke(null, null);
                    _frameCount = 0;
                }
            }

            BitmapUpdated?.BeginInvoke(null, null);
            return true;
        }

        protected bool CreateXYmROISpatialBitmap()
        {
            if (null == _frameData)
            {
                return false;
            }
            if (null == _24BitPixelDataChannelComposite)
            {
                return false;
            }

            //Get the last updated element of the history to use as a reference forr variable initialization
            if (_frameDataHistory.LastUpdatedIndex < 0 && _frameDataHistory.LastUpdatedIndex < _frameDataHistory.FrameDataList.Length || null == _frameDataHistory.LastUpdatedFrame)
            {
                return false;
            }
            FrameInfoStruct lastUpdatedHistoryElement = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo;

            if (_mROIRects == null || lastUpdatedHistoryElement.totalScanAreas != _mROIRects?.Length)
            {
                _mROIRects = new Rect[lastUpdatedHistoryElement.totalScanAreas];
            }

            PixelFormat pixelFormat = PixelFormats.Rgb24;
            int planes = lastUpdatedHistoryElement.numberOfPlanes > 1 ? lastUpdatedHistoryElement.numberOfPlanes : 1;
            int scanAreas = 1;
            int vImages = scanAreas > 1 ? scanAreas : planes;
            int outputBitmapWidth = 1;
            int outputBitmapHeight = 1;
            bool dflimDisplayLifetimeImage = (bool)MVMManager.Instance[DFLIMControlViewModelName, "DFLIMDisplayLifetimeImage", (object)false];
            bool doLifetime = ((int)BufferType.DFLIM_IMAGE == lastUpdatedHistoryElement.bufferType || (int)BufferType.DFLIM_ALL == lastUpdatedHistoryElement.bufferType) && dflimDisplayLifetimeImage;
            int numChannels = 0;
            int bitmapWidth = lastUpdatedHistoryElement.fullImageWidth;
            int bitmapHeight = lastUpdatedHistoryElement.fullImageHeight;
            double aspectRatioYScale = _displayPixelAspectRatio ? (double)lastUpdatedHistoryElement.pixelAspectRatioYScale / 100.0 : 1.0;

            bool[] channelEnable = new bool[ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP].Count];
            bool tile = TileDisplay;
            for (int i = 0; i < ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP].Count; ++i)
            {
                if (true == (ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP][i] && (FrameData.channelSelection & (0x0001 << i)) > 0))
                {
                    ++numChannels;
                    channelEnable[i] = true;
                }
                else
                {
                    channelEnable[i] = false;
                }
            }
            if (0 == numChannels)
            {
                return false;
            }

            // true if we should only show composites (no raw tiles)
            bool isCompositeOnly = numChannels == 1;

            if (!tile)
            {
                // no tiles - only show composites
                isCompositeOnly = true;
                outputBitmapHeight *= vImages;
            }
            else if (lastUpdatedHistoryElement.channels > 1 || doLifetime)
            {
                if (planes > 1 || scanAreas > 1)
                {
                    // account for composite as another channel, unless there's only composite
                    outputBitmapWidth *= isCompositeOnly ? 1 : numChannels + 1;
                    outputBitmapHeight *= vImages;
                }
                else
                {
                    switch (numChannels)
                    {
                        case 1:
                            {
                                if (doLifetime)
                                {
                                    outputBitmapWidth *= VerticalTileDisplay ? 1 : 2;
                                    outputBitmapHeight *= VerticalTileDisplay ? 2 : 1;
                                }
                            }
                            break;
                        case 2:
                            {
                                outputBitmapWidth *= VerticalTileDisplay ? 1 : 3;
                                outputBitmapHeight *= VerticalTileDisplay ? 3 : 1;
                            }
                            break;
                        case 3:
                            {
                                outputBitmapWidth *= 2;
                                outputBitmapHeight *= 2;
                            }
                            break;
                        default:
                            {
                                // All 4 Channels enabled
                                outputBitmapWidth *= VerticalTileDisplay ? 2 : 3;
                                outputBitmapHeight *= VerticalTileDisplay ? 3 : 2;
                            }
                            break;
                    }
                }
            }

            if (Application.Current == null) return false;
            _ = (Application.Current?.Dispatcher.Invoke(DispatcherPriority.Normal,
                new Action(delegate ()
                {
                    try
                    {
                        bool setOverlayItemsOnce = false;
                        bool isVerticalLayout = VerticalTileDisplay && vImages <= 1; // only do Horizontal layout for multiplane
                        int numImages = (numChannels + 1) * vImages; // +1 for composite

                        // use these to check for change in image size
                        int totalPixelWidth = outputBitmapWidth * bitmapWidth;
                        int totalPixelHeight = outputBitmapHeight * bitmapHeight;

                        if (null == _imagesGrid || false == _imageGridParameters.Compare(outputBitmapWidth, outputBitmapHeight, totalPixelWidth, totalPixelHeight, isVerticalLayout, numChannels, vImages, aspectRatioYScale, 0) || _updateBackgroundImage)
                        {
                            if (_imageGridParameters.Columns != outputBitmapWidth || _imageGridParameters.Rows != outputBitmapHeight || _imagesGrid == null)
                            {
                                // create a new bitmap with new grid parameters
                                _imageGridParameters.Columns = outputBitmapWidth;
                                _imageGridParameters.Rows = outputBitmapHeight;
                                _imageGridParameters.Channels = numChannels;
                                _imageGridParameters.Planes = vImages;
                                _imageGridParameters.IsVertical = isVerticalLayout;
                                _imageGridParameters.PixelWidth = new Dictionary<int, int>();
                                _imageGridParameters.PixelWidth[0] = totalPixelWidth;
                                _imageGridParameters.PixelHeight = new Dictionary<int, int>();
                                _imageGridParameters.PixelHeight[0] = totalPixelHeight;
                                _imageGridParameters.AspectRatioYScale = aspectRatioYScale;

                                _imagesGrid = new ObservableCollection<ObservableCollection<CompoundImage>>();

                                int tmpCount = 0;
                                for (int i = 0; i < _imageGridParameters.Rows; i++)
                                {
                                    _imagesGrid.Add(new ObservableCollection<CompoundImage>());
                                    for (int j = 0; j < _imageGridParameters.Columns; j++)
                                    {

                                        CompoundImage compoundImage = new CompoundImage();
                                        compoundImage.ImageXY = new WriteableBitmap(bitmapWidth, bitmapHeight, 96, 96 / aspectRatioYScale, pixelFormat, null);
                                        compoundImage.Selected -= CompoundImage_Selected;
                                        compoundImage.Selected += CompoundImage_Selected;
                                        if (!setOverlayItemsOnce)
                                        {
                                            var list = compoundImage.OverlayItems;
                                            OverlayManager.OverlayManagerClass.Instance.GetDuplicatedROIList(ref list);
                                            compoundImage.OverlayItems = list;
                                            OverlayManager.OverlayManagerClass.Instance.SetOverlayItems(compoundImage.OverlayItems, false);
                                            setOverlayItemsOnce = true;
                                        }

                                        _imagesGrid[i].Add(compoundImage);

                                        if (++tmpCount >= numImages)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                // create a new bitmap with new grid parameters
                                _imageGridParameters.Columns = outputBitmapWidth;
                                _imageGridParameters.Rows = outputBitmapHeight;
                                _imageGridParameters.Channels = numChannels;
                                _imageGridParameters.Planes = vImages;
                                _imageGridParameters.IsVertical = isVerticalLayout;
                                _imageGridParameters.PixelWidth = new Dictionary<int, int>();
                                _imageGridParameters.PixelWidth[0] = totalPixelWidth;
                                _imageGridParameters.PixelHeight = new Dictionary<int, int>();
                                _imageGridParameters.PixelHeight[0] = totalPixelHeight;
                                _imageGridParameters.AspectRatioYScale = aspectRatioYScale;

                                for (int i = 0; i < _imageGridParameters.Rows; i++)
                                {
                                    for (int j = 0; j < _imageGridParameters.Columns; j++)
                                    {
                                        CompoundImage compoundImage = new CompoundImage();
                                        compoundImage.ImageXY = new WriteableBitmap(bitmapWidth, bitmapHeight, 96, 96 / aspectRatioYScale, pixelFormat, null);
                                        compoundImage.Selected -= CompoundImage_Selected;
                                        compoundImage.Selected += CompoundImage_Selected;
                                        if (!setOverlayItemsOnce)
                                        {
                                            var list = compoundImage.OverlayItems;
                                            OverlayManager.OverlayManagerClass.Instance.GetDuplicatedROIList(ref list);
                                            compoundImage.OverlayItems = list;
                                            OverlayManager.OverlayManagerClass.Instance.SetOverlayItems(compoundImage.OverlayItems, false);
                                            setOverlayItemsOnce = true;
                                        }
                                        _imagesGrid[i][j] = compoundImage;
                                    }
                                }
                            }

                            // 2d array with bool for each image in image grid. Bool is flipped to false when the background has been copied to that image.
                            _backgroundCopyFlags = new bool[_imageGridParameters.Rows, _imageGridParameters.Columns];
                            for (int i = 0; i < _imageGridParameters.Rows; i++)
                            {
                                for (int j = 0; j < _imageGridParameters.Columns; j++)
                                {
                                    _backgroundCopyFlags[i, j] = true;
                                }
                            }
                            _updateBackgroundImage = false;
                            _needToResizeBackground = true; // Only need to resize the background when the bitmaps are updated. Otherwise, keep using the same one
                        }

                        // list of enabled channels
                        var enabledChannels = new int[numChannels];
                        for (int i = 0, j = 0; i < channelEnable.Length && j < numChannels; i++)
                        {
                            if (channelEnable[i])
                            {
                                enabledChannels[j++] = i;
                            }
                        }

                        int currentAreaIndex;
                        int tmpImageCount;
                        int channels;

                        int areaWidth;
                        int areaHeight;
                        int rawStride;
                        FrameInfoStruct currentArea;
                        bool copyResizedPixelData = _allowmROIBackgroundImage && _imageReviewType == ImageViewType.CaptureSetup && _24BitPixelDataForBackground != null;
                        int backgroundRawStride = (bitmapWidth * pixelFormat.BitsPerPixel + 7) / 8;
                        if (copyResizedPixelData && _needToResizeBackground)
                        {
                            //resize here so it doesn't need to be done multiple times
                            NearestNeighborDataResize(_24BitPixelDataForBackground[0], _backgroundWidth, _backgroundHeight, ref _resizedBackground, bitmapWidth, bitmapHeight);
                            _needToResizeBackground = false;
                        }

                        foreach (FrameDataHistoryElement frameHistory in _frameDataHistory.FrameDataList)
                        {

                            if (frameHistory == null || !frameHistory.NeedsBitmapUpdate)
                            {
                                continue;
                            }
                            currentArea = frameHistory.StoredLastFrameInfo;
                            currentAreaIndex = currentArea.scanAreaIndex;

                            areaWidth = currentArea.imageWidth;
                            areaHeight = currentArea.imageHeight;
                            rawStride = (areaWidth * pixelFormat.BitsPerPixel + 7) / 8;

                            _mROIRects[currentAreaIndex] =
                                new Rect(currentArea.leftInFullImage, currentArea.topInFullImage, currentArea.imageWidth, currentArea.imageHeight);

                            // enumerate through images grid (x,y) and write out a bitmap foreach channel (+ composite) foreach plane
                            tmpImageCount = 0;
                            channels = isCompositeOnly ? 1 : numChannels + 1;
                            foreach (var xy in _imageGridParameters.GetGridEnumerator())
                            {
                                int x = xy.Item1;
                                int y = xy.Item2;

                                if (scanAreas > 1 && x != currentAreaIndex)
                                {
                                    ++tmpImageCount;
                                    continue;
                                }
                                int areaIndex = tmpImageCount / channels;

                                int left = currentArea.leftInFullImage;
                                int top = currentArea.topInFullImage;

                                if (tmpImageCount % channels == 0 && (currentArea.scanAreaIndex != mROIPriorityIndex || mROIPriorityIndex < 0 || 1 <= currentArea.totalScanAreas))
                                {
                                    // composite
                                    if (copyResizedPixelData && _backgroundCopyFlags[x, y])
                                    {
                                        //copy background to imagegrid
                                        _imagesGrid[x][y].ImageXY.WritePixels(new Int32Rect(0, 0, bitmapWidth, bitmapHeight), _resizedBackground, backgroundRawStride, 0);
                                        _backgroundCopyFlags[x, y] = false;
                                    }
                                    var pixelData = _24BitPixelDataChannelComposite[currentAreaIndex];
                                    _imagesGrid[x][y].ImageXY.WritePixels(new Int32Rect(left, top, areaWidth, areaHeight), pixelData, rawStride, 0);

                                }
                                else if (tile || doLifetime && (currentArea.scanAreaIndex != mROIPriorityIndex || mROIPriorityIndex < 0))
                                {
                                    if (copyResizedPixelData && _backgroundCopyFlags[x, y])
                                    {
                                        if (copyResizedPixelData && _backgroundCopyFlags[x, y])
                                        {
                                            //copy background to imagegrid
                                            _imagesGrid[x][y].ImageXY.WritePixels(new Int32Rect(0, 0, bitmapWidth, bitmapHeight), _resizedBackground, backgroundRawStride, 0);
                                            _backgroundCopyFlags[x, y] = false;
                                        }
                                    }

                                    // raw image data (if tile option is enabled)
                                    int currentChannel = tmpImageCount % channels - 1;
                                    currentChannel = enabledChannels[currentChannel];
                                    var pixelData = _rawImg[currentChannel][currentAreaIndex];
                                    _imagesGrid[x][y].ImageXY.WritePixels(new Int32Rect(left, top, areaWidth, areaHeight), pixelData, rawStride, 0);
                                }

                                bool intersectedWithPrioritymROI = false;
                                if (_mROIRects?.Length > 0)
                                {
                                    for (int i = _mROIRects.Length - 1; i >= 0; --i)
                                    {
                                        bool intersects = _mROIRects[currentAreaIndex].IntersectsWith(_mROIRects[i]);
                                        if (!intersects)
                                        {
                                            for (int j = i + 1; j < _mROIRects.Length; ++j)
                                            {
                                                if (_mROIRects[j].IntersectsWith(_mROIRects[i]))
                                                {
                                                    intersects = true;
                                                    break;
                                                }
                                            }
                                        }
                                        if ((i != mROIPriorityIndex || mROIPriorityIndex < 0) && _mROIRects?.Length > i &&
                                            (intersects) && currentArea.totalScanAreas > 1 && currentAreaIndex > i)
                                        {
                                            int l = (int)_mROIRects[i].Left;
                                            int t = (int)_mROIRects[i].Top;
                                            int w = (int)_mROIRects[i].Width;
                                            int h = (int)_mROIRects[i].Height;
                                            int rStride = (w * pixelFormat.BitsPerPixel + 7) / 8;
                                            if (tmpImageCount % channels == 0)
                                            {
                                                // composite
                                                var pixelData = _24BitmROIPriorityDisplayAreaComposite[i];
                                                _imagesGrid[x][y].ImageXY.WritePixels(new Int32Rect(l, t, w, h), pixelData, rStride, 0);
                                            }
                                            else if (tile || doLifetime)
                                            {
                                                // raw image data (if tile option is enabled)
                                                int currentChannel = tmpImageCount % channels - 1;
                                                currentChannel = enabledChannels[currentChannel];
                                                var pixelData = _rawImgmROIPriorityDisplayAreaComposite[currentChannel][i];
                                                _imagesGrid[x][y].ImageXY.WritePixels(new Int32Rect(l, t, w, h), pixelData, rStride, 0);
                                            }

                                        }
                                        if (_mROIRects?.Length > mROIPriorityIndex && mROIPriorityIndex >= 0 && _mROIRects[i].IntersectsWith(_mROIRects[mROIPriorityIndex]) && i != mROIPriorityIndex)
                                        {
                                            intersectedWithPrioritymROI = true;
                                        }
                                    }

                                    if (intersectedWithPrioritymROI && mROIPriorityIndex >= 0 && _mROIRects?.Length > mROIPriorityIndex && _mROIRects[mROIPriorityIndex] != null &&
                                        currentArea.totalScanAreas > 1)
                                    {
                                        int l = (int)_mROIRects[mROIPriorityIndex].Left;
                                        int t = (int)_mROIRects[mROIPriorityIndex].Top;
                                        int w = (int)_mROIRects[mROIPriorityIndex].Width;
                                        int h = (int)_mROIRects[mROIPriorityIndex].Height;
                                        int rStride = (w * pixelFormat.BitsPerPixel + 7) / 8;
                                        if (tmpImageCount % channels == 0)
                                        {
                                            // composite
                                            var pixelData = _24BitmROIPriorityDisplayAreaComposite[mROIPriorityIndex];
                                            _imagesGrid[x][y].ImageXY.WritePixels(new Int32Rect(l, t, w, h), pixelData, rStride, 0);
                                        }
                                        else if (tile || doLifetime)
                                        {
                                            // raw image data (if tile option is enabled)
                                            int currentChannel = tmpImageCount % channels - 1;
                                            currentChannel = enabledChannels[currentChannel];
                                            var pixelData = _rawImgmROIPriorityDisplayAreaComposite[currentChannel][mROIPriorityIndex];
                                            _imagesGrid[x][y].ImageXY.WritePixels(new Int32Rect(l, t, w, h), pixelData, rStride, 0);
                                        }
                                    }
                                }

                                if (++tmpImageCount >= numImages)
                                {
                                    break;
                                }
                            }
                            frameHistory.NeedsBitmapUpdate = false;
                        }
                        UpdateOverlayManager(bitmapWidth, bitmapHeight, 1.0, aspectRatioYScale);
                    }
                    catch (Exception ex)
                    {
                        ex.ToString();
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, "Error creating ImagesGrid");
                    }
                })));
            if (_calculateBitmapUpdateRate)
            {
                _frameCount++;
                _endTime = DateTimeOffset.Now.ToUnixTimeMilliseconds();
                if (_endTime - _startTime >= 2000)
                {
                    _bitmapUpdateRate = (double)_frameCount / ((_endTime - _startTime) / 1000.0);
                    _startTime = _endTime;
                    BitmapUpdateRateUpdated?.BeginInvoke(null, null);
                    _frameCount = 0;
                }
            }

            BitmapUpdated?.BeginInvoke(null, null);
            return true;
        }

        protected bool DidHistogramUpdate(out int areaIndex, out int channelIndex)
        {
            areaIndex = -1;
            channelIndex = -1;

            try
            {
                if (_frameDataSet && null != _frameDataHistory.LastUpdatedFrame)
                {
                    FrameData fd = _frameDataHistory.LastUpdatedFrame.StoredFrameData;
                    FrameInfoStruct lastFrameInfo = _frameDataHistory.LastUpdatedFrame.StoredLastFrameInfo;

                    if (fd != null)
                    {
                        int planes = lastFrameInfo.numberOfPlanes > 1 ? lastFrameInfo.numberOfPlanes : 1;
                        int scanAreas = lastFrameInfo.totalScanAreas > 1 ? lastFrameInfo.totalScanAreas : 1;
                        int verticalImages = scanAreas > 1 ? scanAreas : planes;
                        verticalImages = (fd.frameInfo.totalSequences > 1) ? fd.frameInfo.totalSequences : verticalImages;
                        int numChannels = lastFrameInfo.channels;
                        int channelSelection = fd.channelSelection;

                        lock (UpdateHistogramsLock)
                        {

                            for (int v = 0; v < verticalImages; ++v)
                            {
                                if (IsInSequentialMode && ChannelsPerSequence.Count > v)
                                {
                                    channelSelection = ChannelsPerSequence[v].Item2;
                                    numChannels = ChannelsPerSequence[v].Item1;
                                }
                                for (int c = 0; c < MAX_CHANNELS; ++c)
                                {
                                    if ((channelSelection & (0x0001 << c)) > 0)
                                    {
                                        ImageIdentifier imageIdentifier = new ImageIdentifier(c, v);

                                        PixelDataHistogramInfo pixelDataHistogram = GetOrCreatePixelDataHistogram(imageIdentifier, false);

                                        if (pixelDataHistogram?.IsWhiteBlackPointChanged == true) // TODO: why is this always true when switching tabs?
                                        {
                                            areaIndex = (scanAreas > 1 || fd.frameInfo.totalSequences > 1) ? v : 0;
                                            if (areaIndex > -1 && _frameDataHistory.FrameDataList.Length > areaIndex && _frameDataHistory.FrameDataList[areaIndex] != null)
                                            {
                                                _frameDataHistory.FrameDataList[areaIndex].NeedsBitmapUpdate = true;
                                                _frameDataHistory.FrameDataList[areaIndex].FrameDataUpdated = true; //TODO we can probably get rid of this
                                            }
                                            channelIndex = c;
                                            return true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }

            return false;
        }

        static BitmapPalette BuildPaletteGrayscale()
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
                    //dvalB = 255;
                }
                if (i == 255)
                {
                    //dvalG = 0;
                    //dvalB = 0;
                }
                colors[i] = Color.FromRgb((byte)dvalR, (byte)dvalG, (byte)dvalB);
            }

            return new BitmapPalette(colors);
        }

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImage")]
        static extern bool ReadImage([MarshalAs(UnmanagedType.LPWStr)] string path, ref IntPtr outputBuffer);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo")]
        static extern bool ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)] string selectedFileName, ref long width, ref long height, ref long colorChannels, ref long bitsPerChannel);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo")]
        static extern int ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)] string path, ref int width, ref int height, ref int colorChannels, ref long bitsPerChannel);

        //method to make the buildbitmap task less repetitive
        private void BuildAvailableAreasHelper(bool updatePixelData, bool wpbpchanged, int histogramUpdatedAreaIndex, ref bool pixelDataResult)
        {
            for (int i = 0; i < _frameDataHistory?.FrameDataList.Length; i++)
            {
                //if frame is not null and there is a update from camera or histogram
                if (_frameDataHistory.FrameDataList[i] != null && (_frameDataHistory.FrameDataList[i].FrameDataUpdated || (wpbpchanged && i == histogramUpdatedAreaIndex)))
                {
                    _frameDataHistory.FrameDataList[i].FrameDataUpdated = false;
                    _frameDataHistory.FrameDataList[i].NeedsBitmapUpdate = true;
                    if (IsInSequentialMode)
                    {
                        pixelDataResult = Create24BitPixelDataByteRawAndComposite(updatePixelData, _frameDataHistory.FrameDataList[i].StoredFrameData.frameInfo.sequenceIndex).Result;

                    }
                    else
                    {
                        pixelDataResult = Create24BitPixelDataByteRawAndComposite(updatePixelData, _frameDataHistory.FrameDataList[i].StoredFrameData.frameInfo.scanAreaIndex).Result;
                    }
                }
            }
        }

        void BuildNormalizationPalettes(PixelDataHistogramInfo pdHistInfo)
        {
            int shiftValue = _bitsPerPixel - 8;
            double shiftValueResult = Math.Pow(2, shiftValue);

            for (int i = 0; i < ushort.MaxValue + 1; i++)
            {
                double val = (255.0 / (shiftValueResult * (pdHistInfo.WhitePoint - pdHistInfo.BlackPoint))) * (i - pdHistInfo.BlackPoint * shiftValueResult);
                val = Math.Pow(val, pdHistInfo.Gamma) * (pdHistInfo.WhitePoint - pdHistInfo.BlackPoint) / Math.Pow(pdHistInfo.WhitePoint - pdHistInfo.BlackPoint, pdHistInfo.Gamma);
                val = (val >= 0) ? val : 0;
                val = (val <= 255) ? val : 255;

                pdHistInfo.Palette[i] = (byte)Math.Round(val);
            }
        }

        private void CompoundImage_Selected(CompoundImage selectedImage)
        {
            //update the overlayItems to use the selected image overlayItems
            //If clicking between images, deselect roi's from current compound image before switching
            if (OverlayManager.OverlayManagerClass.Instance.OverlayItems != selectedImage.OverlayItems)
            {
                OverlayManager.OverlayManagerClass.Instance.DeselectAllROIs();
            }
            OverlayManager.OverlayManagerClass.Instance.SetOverlayItems(selectedImage.OverlayItems, false);
        }

        void CopyData(int bitmapRows, int frameDataIndex)
        {
            _updatePixelData = false;
            lock (_frameData.dataLock)
            {
                if (bitmapRows > 1 || _frameData.frameInfo.isMROI == 1)
                {
                    if (null == _frameDataHistory || _frameDataHistory.FrameDataList.Length != bitmapRows)
                    {
                        _frameDataHistory = new FrameDataHistory(bitmapRows);
                    }
                }
                else
                {
                    frameDataIndex = 0;
                    if (null == _frameDataHistory || _frameDataHistory.FrameDataList.Length != 1)
                    {
                        _frameDataHistory = new FrameDataHistory(1);
                    }
                }

                if (_frameDataHistory.FrameDataList[frameDataIndex] == null)
                {
                    _frameDataHistory.FrameDataList[frameDataIndex] = new FrameDataHistoryElement();
                }

                lock (_frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.dataLock)
                {
                    _frameDataHistory.FrameDataList[frameDataIndex].FrameDataUpdated = false;
                    _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.averageFrameCount = _frameData.averageFrameCount;
                    _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.averageMode = _frameData.averageMode;
                    _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.bitsPerPixel = _frameData.bitsPerPixel;
                    _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.frameInfo = _frameData.frameInfo;
                    _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.bitsPerPixel = _frameData.bitsPerPixel;
                    _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.channelSelection = _frameData.channelSelection;
                    _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.contiguousChannels = _frameData.contiguousChannels;
                    _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.pixelSizeUM = _frameData.pixelSizeUM;
                    _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.isFastZPreviewImage = _frameData.isFastZPreviewImage;

                    if (_frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.dFLIMBinDurations?.Length != _frameData.dFLIMBinDurations?.Length && _frameData.dFLIMBinDurations?.Length > 0)
                    {
                        _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.dFLIMBinDurations = new double[_frameData.dFLIMBinDurations.Length];
                    }
                    if (_frameData.dFLIMBinDurations?.Length > 0)
                    {
                        Array.Copy(_frameData.dFLIMBinDurations, _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.dFLIMBinDurations, _frameData.dFLIMBinDurations.Length);
                    }

                    if (_frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.dFLIMSinglePhotonData?.Length != _frameData.dFLIMSinglePhotonData?.Length && _frameData.dFLIMSinglePhotonData?.Length > 0)
                    {
                        _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.dFLIMSinglePhotonData = new ushort[_frameData.dFLIMSinglePhotonData.Length];
                    }
                    if (_frameData.dFLIMSinglePhotonData?.Length > 0)
                    {
                        Array.Copy(_frameData.dFLIMSinglePhotonData, _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.dFLIMSinglePhotonData, _frameData.dFLIMSinglePhotonData.Length);
                    }

                    if (_frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.dFLIMArrivalTimeSumData?.Length != _frameData.dFLIMArrivalTimeSumData?.Length && _frameData.dFLIMArrivalTimeSumData?.Length > 0)
                    {
                        _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.dFLIMArrivalTimeSumData = new uint[_frameData.dFLIMArrivalTimeSumData.Length];
                    }
                    if (_frameData.dFLIMArrivalTimeSumData?.Length > 0)
                    {
                        Array.Copy(_frameData.dFLIMArrivalTimeSumData, _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.dFLIMArrivalTimeSumData, _frameData.dFLIMArrivalTimeSumData.Length);
                    }

                    if (AllowReferenceImage && _referenceChannelEnabled && !IsInSequentialMode && _frameData.frameInfo.isMROI == 0)
                    {
                        int imageDataLength = _frameData.frameInfo.fullImageWidth * _frameData.frameInfo.fullImageHeight;
                        if (_frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.pixelData?.Length != imageDataLength * MAX_CHANNELS)
                        {
                            //Ref images require that the framedata array be large enough for 4 channels
                            _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.pixelData = new ushort[imageDataLength * MAX_CHANNELS];
                        }
                        else
                        {
                            Array.Clear(_frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.pixelData, 0, imageDataLength * MAX_CHANNELS);
                        }
                        if (_frameData.pixelData?.Length > 0)
                        {
                            if (_frameData.pixelData?.Length != _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.pixelData?.Length)
                            {
                                //This is needed to handle case where incoming pixel data array is not long enough for 4 channels.
                                int numChannelsInIncomingPixelData = _frameData.pixelData.Length / imageDataLength;
                                int remainingChannelSelection = _channelSelectionBeforeRefImage;

                                //loop through the number of channels in the incoming data.
                                for (int i = 0; i < numChannelsInIncomingPixelData; i++)
                                {
                                    for (int j = 0; j < MAX_CHANNELS; j++)
                                    {
                                        if ((remainingChannelSelection & (0x0001 << j)) > 0)
                                        {
                                            //If the channel's pixel data is in the incoming frame data, copy it to the corresponding location in the stored data.
                                            remainingChannelSelection = remainingChannelSelection ^ 0x001 << i; // flip the bit so it doesn't get triggered again
                                            Array.Copy(_frameData.pixelData, imageDataLength * i, _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.pixelData, imageDataLength * j, imageDataLength);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                Array.Copy(_frameData.pixelData, _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.pixelData, _frameData.pixelData.Length);
                            }
                        }
                    }
                    else
                    {
                        if (_frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.pixelData?.Length != _frameData.pixelData?.Length && _frameData.pixelData?.Length > 0)
                        {
                            _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.pixelData = new ushort[_frameData.pixelData.Length];
                        }
                        if (_frameData.pixelData?.Length > 0)
                        {
                            Array.Copy(_frameData.pixelData, _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.pixelData, _frameData.pixelData.Length);
                        }
                    }

                    if (_bitsPerPixel != _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.bitsPerPixel)
                    {
                        _bitsPerPixel = _frameDataHistory.FrameDataList[frameDataIndex].StoredFrameData.bitsPerPixel;
                        PixelBitShiftValueChanged?.Invoke();
                    }

                    _currentAreaIndex = frameDataIndex;
                    _frameDataHistory.FrameDataList[frameDataIndex].StoredLastFrameInfo = _frameData.frameInfo;
                }
            }
            _frameDataHistory.FrameDataList[frameDataIndex].FrameDataUpdated = true;
            _frameDataSet = true;
            _updatePixelData = true;
            _frameDataReadyToSet = true;
        }

        private void CopyDataTask()
        {
            while (_runCopyDataThread)
            {
                if (_dataReadyToCopy)
                {
                    if (IsInSequentialMode)
                    {
                        CopyData(_frameData.frameInfo.totalSequences, _frameData.frameInfo.sequenceIndex);
                    }
                    else
                    {
                        CopyData(_frameData.frameInfo.totalScanAreas, _frameData.frameInfo.scanAreaIndex);
                    }
                    _dataReadyToCopy = false;
                }
                Thread.Sleep(1);
            }
        }

        private void CopyHistoryElementToTempFrameData(FrameDataHistoryElement element)
        {
            lock (_lastFullFOVFrameData.dataLock)
            {
                if (_lastFullFOVFrameData == null)
                {
                    _lastFullFOVFrameData = new FrameData();
                }
                _lastFullFOVFrameData.averageFrameCount = element.StoredFrameData.averageFrameCount;
                _lastFullFOVFrameData.averageMode = element.StoredFrameData.averageMode;
                _lastFullFOVFrameData.bitsPerPixel = element.StoredFrameData.bitsPerPixel;
                _lastFullFOVFrameData.frameInfo = element.StoredFrameData.frameInfo;
                _lastFullFOVFrameData.bitsPerPixel = element.StoredFrameData.bitsPerPixel;
                _lastFullFOVFrameData.channelSelection = element.StoredFrameData.channelSelection;
                _lastFullFOVFrameData.contiguousChannels = element.StoredFrameData.contiguousChannels;
                _lastFullFOVFrameData.pixelSizeUM = element.StoredFrameData.pixelSizeUM;
                _lastFullFOVFrameData.isFastZPreviewImage = element.StoredFrameData.isFastZPreviewImage;
                if (_lastFullFOVFrameData.pixelData == null || _lastFullFOVFrameData.pixelData.Length != element.StoredFrameData.pixelData.Length)
                {
                    _lastFullFOVFrameData.pixelData = new ushort[element.StoredFrameData.pixelData.Length];
                }
                Array.Copy(element.StoredFrameData.pixelData, _lastFullFOVFrameData.pixelData, element.StoredFrameData.pixelData.Length);
            }
        }

        void CreatePixelDataByteDFLIM()
        {
            int idx;
            int numPlanes = FrameData.frameInfo.numberOfPlanes < 1 ? 1 : FrameData.frameInfo.numberOfPlanes;
            int numChannels = FrameData.frameInfo.channels;
            int dataLength = FrameData.frameInfo.imageHeight * FrameData.frameInfo.imageWidth;
            for (int k = 0; k < _rawImg.Length; k++)
            {
                if (_rawImg[k] == null || _rawImg[k].Length != numPlanes)
                {
                    _rawImg[k] = new byte[numPlanes][];
                }

                for (int p = 0; p < numPlanes; p++)
                {
                    if (_rawImg[k][p] == null || _rawImg[k][p].Length != 3 * dataLength)
                    {
                        _rawImg[k][p] = new byte[3 * dataLength];
                    }
                }
            }

            for (int p = 0; p < numPlanes; p++)
            {
                if (_24BitPixelDataChannelComposite[p]?.Length != 3 * dataLength)
                {
                    _24BitPixelDataChannelComposite[p] = new byte[3 * dataLength];
                }
            }

            var tau_high_ns = ((CustomCollection<float>)MVMManager.Instance[DFLIMControlViewModelName, "DFLIMTauHigh"]);
            var tau_low_ns = ((CustomCollection<float>)MVMManager.Instance[DFLIMControlViewModelName, "DFLIMTauLow"]);
            var lut_high_bins = ((CustomCollection<uint>)MVMManager.Instance[DFLIMControlViewModelName, "DFLIMLUTHigh"]);
            var lut_low_bins = ((CustomCollection<uint>)MVMManager.Instance[DFLIMControlViewModelName, "DFLIMLUTLow"]);
            var tZero_ns = ((CustomCollection<float>)MVMManager.Instance[DFLIMControlViewModelName, "DFLIMTZero"]);
            int intensityScale = ImageViewType.CaptureSetup == _imageReviewType ? 128 : 1; // currently scaling by 128 on capture setup
            for (int k = 0; k < MAX_CHANNELS; k++) //TODO: see if numChannels should be used instead
            {
                if (ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP][k] && (FrameData.channelSelection & (0x0001 << k)) > 0)
                {
                    if (FrameData.dFLIMBinDurations.Length <= k || 0 == FrameData.dFLIMBinDurations[k])
                    {
                        continue;
                    }

                    double tau_high = tau_high_ns[k] / FrameData.dFLIMBinDurations[k];
                    double tau_low = tau_low_ns[k] / FrameData.dFLIMBinDurations[k];
                    double lut_high = lut_high_bins[k];
                    double lut_low = lut_low_bins[k];
                    double tZeroBins = tZero_ns[k] / FrameData.dFLIMBinDurations[k];

                    int pixelNum = dataLength;

                    int averageMode = FrameData.averageMode;
                    int averageFrames = 0 == averageMode ? 1 : FrameData.averageFrameCount;
                    double tauscale = (tau_high == tau_low) ? 1.0f : (1.0f / (tau_high - tau_low));
                    double brightness;
                    double tau_scaled;
                    int chan = FrameData?.frameInfo.channels > 1 ? k : 0;

                    //Build the LifeTime Image
                    for (int pix = 0; pix < pixelNum; pix++)
                    {
                        try
                        {
                            int q = pix;
                            int n = q * 3;

                            //brightness = ((double)_dflimSinglePhotonData[q + chan * pixelNum] / (double)averageFrames - lut_low) / (lut_high - lut_low);
                            //to get the brigtness right from the beginning, especially when averaging, we will use the intensity buffer
                            //instead of the single photon buffer.
                            // TODO: this should be 1 once the intensity histogram can handle smaller intensity ranges
                            //TODO: need to see if this is ok, different formula used in runsample
                            brightness = (((double)FrameData.pixelData[q + chan * pixelNum] / intensityScale) - lut_low) / (lut_high - lut_low);
                            brightness = (brightness < 0) ? 0 : ((brightness > 1) ? 1 : brightness);

                            if (_frameData.dFLIMSinglePhotonData[q + chan * pixelNum] != 0)
                            {
                                tau_scaled = tauscale * ((double)FrameData.dFLIMArrivalTimeSumData[q + chan * pixelNum] / FrameData.dFLIMSinglePhotonData[q + chan * pixelNum] - tau_low - tZeroBins);
                            }
                            else
                            {
                                tau_scaled = 0;
                            }

                            idx = (tau_scaled < 0) ? 0 : ((tau_scaled > 1) ? 255 : (int)(tau_scaled * 255 + 0.5));

                            byte red = (byte)(brightness * 255 * _dflimColorMap[3 * idx]);
                            byte green = (byte)(brightness * 255 * _dflimColorMap[3 * idx + 1]);
                            byte blue = (byte)(brightness * 255 * _dflimColorMap[3 * idx + 2]);

                            // TODO: multiplane DFLIM?
                            _rawImg[k][0][n] = red;
                            _rawImg[k][0][n + 1] = green;
                            _rawImg[k][0][n + 2] = blue;
                        }
                        catch (Exception ex)
                        {
                            ex.ToString();
                        }
                    }
                }
            }
        }

        bool GetRawContainsDisabledChannels()
        {
            XmlDocument experimentDoc = new XmlDocument();
            if (File.Exists(ExperimentPath + "\\Experiment.xml"))
            {
                experimentDoc.Load(ExperimentPath + "\\Experiment.xml");
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

        string[] GetZImageFileNames(int timeIndex, int zValue, ref CaptureFile imageType)
        {
            string[] fileNames = new string[ImageViewMBase.MAX_CHANNELS];

            int sampleSiteIndex = 1;
            int subIndex = 1;

            string experimentFolderPath = ExperimentPath;
            string imgNameFormat = ImageNameFormat;
            var hwDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndListHW = hwDoc.SelectNodes("/HardwareSettings/Wavelength");
            imageType = CaptureFile.FILE_TIFF; // Default image type to TIFF, this is the type saved for ZStacked preview

            //In the Review tab we need to read the image type to know what type of file we need to load
            if (experimentFolderPath != ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "ZStackCache")
            {
                XmlDocument expDoc = new XmlDocument();
                expDoc.Load(experimentFolderPath + "\\Experiment.xml");
                CaptureModes captureMode = (CaptureModes)XmlManager.ReadAttribute<Int32>(expDoc, "/ThorImageExperiment/CaptureMode", "mode");
                if (CaptureModes.STREAMING == captureMode)
                {
                    imageType = (CaptureFile)XmlManager.ReadAttribute<Int32>(expDoc, "/ThorImageExperiment/Streaming", "rawData");
                }
            }

            if (null != WavelengthNames)
            {
                for (int i = 0; i < WavelengthNames.Length; i++)
                {
                    if ((experimentFolderPath != null))
                    {
                        StringBuilder sbTemp = new StringBuilder();
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
                                    sbTemp.AppendFormat("{0}{1}{2}{3}{4}{5}",
                                                            experimentFolderPath + "\\",
                                                            WavelengthNames[i],
                                                            "_" + sampleSiteIndex.ToString(imgNameFormat),
                                                            "_" + subIndex.ToString(imgNameFormat),
                                                            "_" + zValue.ToString(imgNameFormat),
                                                            "_" + timeIndex.ToString(imgNameFormat) + ".tif");
                                }
                                break;
                            default:
                                break;
                        }

                        string strTemp = sbTemp.ToString();
                        for (int j = 0; j < ndListHW.Count; j++)
                        {
                            if (null != WavelengthNames[i] && (FrameData?.channelSelection & (0x0001 << i)) > 0)
                            {
                                if (WavelengthNames[i].Equals(ndListHW[j].Attributes["name"].Value) && ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP][j])
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

        private void NearestNeighborDataResize(byte[] input, int inputWidth, int inputHeight, ref byte[] output, int outputWidth, int outputHeight)
        {
            output = new byte[outputWidth * outputHeight * 3];
            double xScale = (double)outputWidth / inputWidth;
            double yScale = (double)outputHeight / inputHeight;

            int positionInInputArray = 0;
            int topPixelPosition = 0;
            int leftPixelPosition = 0;
            int rightPixelPosition = 0;
            int bottomPixelPosition = 0;

            double redSumOfPixels = 0;
            double greenSumOfPixels = 0;
            double blueSumOfPixels = 0;

            int numPixelsInSum = 0;

            for (int i = 0; i < outputWidth; i++)
            {
                for (int j = 0; j < outputHeight; j++)
                {
                    int transformedXPosition = (int)(i / xScale);
                    int transformedYPosition = (int)(j / yScale);

                    if (transformedXPosition >= inputWidth)
                    {
                        transformedXPosition = inputWidth - 1;
                    }
                    if (transformedYPosition >= inputHeight)
                    {
                        transformedYPosition = inputHeight - 1;
                    }

                    positionInInputArray = (inputWidth * transformedYPosition + transformedXPosition) * 3;
                    positionInInputArray = positionInInputArray - positionInInputArray % 3; //floor to multiple of 3
                    topPixelPosition = positionInInputArray - inputWidth * 3;
                    bottomPixelPosition = positionInInputArray + inputWidth * 3;
                    leftPixelPosition = positionInInputArray - 3;
                    rightPixelPosition = positionInInputArray + 3;

                    if (positionInInputArray > 0 && positionInInputArray < input.Length - 3)
                    {
                        redSumOfPixels += input[positionInInputArray];
                        greenSumOfPixels += input[positionInInputArray + 1];
                        blueSumOfPixels += input[positionInInputArray + 2];
                        numPixelsInSum++;
                    }
                    if (topPixelPosition > 0 && topPixelPosition < input.Length - 3)
                    {
                        redSumOfPixels += input[topPixelPosition];
                        greenSumOfPixels += input[topPixelPosition + 1];
                        blueSumOfPixels += input[topPixelPosition + 2];
                        numPixelsInSum++;
                    }
                    if (bottomPixelPosition > 0 && bottomPixelPosition < input.Length - 3)
                    {
                        redSumOfPixels += input[bottomPixelPosition];
                        greenSumOfPixels += input[bottomPixelPosition + 1];
                        blueSumOfPixels += input[bottomPixelPosition + 2];
                        numPixelsInSum++;
                    }
                    if (leftPixelPosition > 0 && leftPixelPosition < input.Length - 3)
                    {
                        redSumOfPixels += input[leftPixelPosition];
                        greenSumOfPixels += input[leftPixelPosition + 1];
                        blueSumOfPixels += input[leftPixelPosition + 2];
                        numPixelsInSum++;
                    }
                    if (rightPixelPosition > 0 && rightPixelPosition < input.Length - 3)
                    {
                        redSumOfPixels += input[rightPixelPosition];
                        greenSumOfPixels += input[rightPixelPosition + 1];
                        blueSumOfPixels += input[rightPixelPosition + 2];
                        numPixelsInSum++;
                    }

                    output[(outputWidth * j + i) * 3] = numPixelsInSum == 0 ? (byte)0 : (byte)(redSumOfPixels / numPixelsInSum);
                    output[((outputWidth * j + i) * 3) + 1] = numPixelsInSum == 0 ? (byte)0 : (byte)(greenSumOfPixels / numPixelsInSum);
                    output[((outputWidth * j + i) * 3) + 2] = numPixelsInSum == 0 ? (byte)0 : (byte)(blueSumOfPixels / numPixelsInSum);

                    redSumOfPixels = 0;
                    greenSumOfPixels = 0;
                    blueSumOfPixels = 0;
                    numPixelsInSum = 0;
                }
            }
        }

        int[][][] histogramDataToBeCombined;
        bool[][] chunkUpdated;
        bool Process24BitImage(int numChannels, bool resetPixelDataHistogram, int vIndex, int dataLengthPerImage, int shiftValue, int verticalImages, int enabledChannelCount, int displayChannelCount, FrameData fd)
        {
            int chan = -1;
            int channelGroup = DEFAULT_CHANNEL_GROUP;

            /*
             * Variables for parralelization logic. Parallelization is limited to 50% of the number of available logical processors. 
             * The size and number of data chunks are selected to maximize data distribution over the number of available threads. 
             * For example, if there are 16 logical processors and there is a 5MP camera with a data length of 5013504, 
             * the level of parallelization is 8 and the parallelizationChunkSize will be determined to be 3 because dataLength >> 3 
             * is 1/8th the size of the image. For systems with higher core counts, the parallelizationChunkSize will increase to a max of 1/64th
            */
            int numDifferentScanAreaSizes = fd.frameInfo.totalScanAreas > 1? fd.frameInfo.totalScanAreas : 1;
            int indexInHistogramDataArray = fd.frameInfo.isMROI == 1? vIndex : 0; 
            int levelOfParallelization = (int)(Environment.ProcessorCount * .5);
            int parallelizationChunkSize = 0;
            int numberOfChunks = 1;

            //Loop to determine the correct shift value and amount to chunk the data
            //Amount to split the data for the parallel for each. Each chunk of data should be no smaller than a 512 x 512 block
            while (dataLengthPerImage >> parallelizationChunkSize >= 262144 && parallelizationChunkSize <= 6)
            {
                parallelizationChunkSize++;
                numberOfChunks = dataLengthPerImage / ((dataLengthPerImage >> parallelizationChunkSize));
                if (numberOfChunks >= levelOfParallelization)
                {
                    break;
                }
            }

            // Only allocate backup composite if in capture setup and mROI is possible while imaging full FOV and backup array is initialized
            bool copyToBackground = _imageReviewType == ImageViewType.CaptureSetup && (int)MVMManager.Instance["AreaControlViewModel", "MesoMicroVisible"] == 1
                && _24BitPixelDataForBackground != null && fd.frameInfo.isMROI != 1;

            //Double check that the lengths of the arrays match. If not do not copy the data
            if (copyToBackground && _24BitPixelDataChannelComposite.Length == _24BitPixelDataForBackground.Length)
            {
                for (int i = 0; i < _24BitPixelDataChannelComposite.Length; i++)
                {
                    if (_24BitPixelDataChannelComposite[i].Length != _24BitPixelDataForBackground[i].Length)
                    {
                        copyToBackground = false;
                        break;
                    }
                }
            }
            else
            {
                copyToBackground = false;
            }

            for (int channelIndex = 0; channelIndex < MAX_CHANNELS; channelIndex++)
            {
                int channelSelection = IsInSequentialMode ? fd.frameInfo.sequenceSelectedChannels : fd.channelSelection;
                while ((channelSelection & (0x0001 << channelIndex)) < 1 && channelIndex < MAX_CHANNELS - 1)
                {
                    channelIndex++;
                }

                int colorIndex = channelIndex;
                ImageIdentifier imageIdentifier = new ImageIdentifier(channelIndex, vIndex);

                if (IsInSequentialMode && vIndex < ChannelLuts.Count && vIndex < ChannelDisplayEnable.Count)
                {
                    channelGroup = vIndex;
                }

                lock (UpdateHistogramsLock)
                {
                    if ((ChannelDisplayEnable[channelGroup][channelIndex] && (channelSelection & (0x0001 << channelIndex)) > 0) && ChannelLuts.Count > 0)
                    {
                        PixelDataHistogramInfo pixelDataHistogram = GetOrCreatePixelDataHistogram(imageIdentifier);

                        if (resetPixelDataHistogram || null == pixelDataHistogram.HistogramData)
                        {
                            pixelDataHistogram.HistogramData = new int[PIXEL_DATA_HISTOGRAM_SIZE];

                            /*
                             * To allow for the different threads to calculate histogram data at the same time, 
                             * each thread will have it's own array. In addition each scan region will have it's own set of arrays.
                             * This prevents the need to re-allocate the arrays each time a new region is processed. 
                             * For mROI this will be 1 per region. For all other imaging modes,
                             * only one set of histograms is needed. Histogram data is recombined at the end of the method. 
                             */
                            if (histogramDataToBeCombined == null || histogramDataToBeCombined.Length != numDifferentScanAreaSizes)
                            {
                                histogramDataToBeCombined = new int[numDifferentScanAreaSizes][][];
                                chunkUpdated = new bool[numDifferentScanAreaSizes][];
                            }
                            if (histogramDataToBeCombined[indexInHistogramDataArray] == null || histogramDataToBeCombined[indexInHistogramDataArray].Length != numberOfChunks)
                            {
                                chunkUpdated[indexInHistogramDataArray] = new bool[numberOfChunks];
                                histogramDataToBeCombined[indexInHistogramDataArray] = new int[numberOfChunks][];
                                for (int i = 0; i < numberOfChunks; i++)
                                {
                                    histogramDataToBeCombined[indexInHistogramDataArray][i] = new int[PIXEL_DATA_HISTOGRAM_SIZE];
                                }
                            }
                            else
                            {
                                for (int i = 0; i < numberOfChunks; i++)
                                {
                                    chunkUpdated[indexInHistogramDataArray][i] = false;
                                    Array.Clear(histogramDataToBeCombined[indexInHistogramDataArray][i], 0, PIXEL_DATA_HISTOGRAM_SIZE);
                                }
                            }
                        }
                        // if channel channelIndex is selected
                        if (pixelDataHistogram.IsWhiteBlackPointChanged || _palletChanged || !EnableHistogramUpdate)
                        {
                            pixelDataHistogram.IsWhiteBlackPointChanged = false;
                            BuildNormalizationPalettes(pixelDataHistogram);
                            pixelDataHistogram.IsWhiteBlackPointChanged = false;
                        }
                        if (!fd.contiguousChannels)
                        {
                            chan = fd.frameInfo.channels > 1 ? channelIndex : 0;
                        }
                        else
                        {
                            ++chan;
                        }

                        //Pre calculated values to minimize the number of calculations that need to be done in the parallel loops
                        bool doGrayscale = _grayscaleForSingleChannel && ((1 == fd.frameInfo.channels || 1 == enabledChannelCount || 1 == displayChannelCount));
                        bool safeToLookupColor = channelGroup < ChannelLuts.Count && channelIndex < ChannelDisplayEnable[channelGroup].Count && null != ChannelLuts[channelGroup] && null != ChannelLuts[channelGroup][colorIndex];
                        bool addChannelToComposite = channelGroup < ChannelLuts.Count && channelIndex < ChannelDisplayEnable[channelGroup].Count && null != ChannelLuts[channelGroup] && null != ChannelLuts[channelGroup][colorIndex] && ChannelDisplayEnable[channelGroup][channelIndex];
                        double maxPixelValueForSaturation = Math.Pow(2, _bitsPerPixel) - 1;
                        double maxPixelBitSize = 256 * Math.Pow(2, shiftValue);
                        int vvIndex = verticalImages > 1 ? vIndex : 0;
                        int rawArrayPositionModifier = (chan * verticalImages + vvIndex) * dataLengthPerImage;
                        Color maxForSaturation = Color.FromRgb((byte)255, (byte)0, (byte)0);
                        Color minForSaturation = Color.FromRgb((byte)0, (byte)0, (byte)255);

                        var rangePartitioner = Partitioner.Create(0, dataLengthPerImage, (dataLengthPerImage >> parallelizationChunkSize) + 1);
                        _ = Parallel.ForEach
                            (rangePartitioner, new ParallelOptions { MaxDegreeOfParallelism = levelOfParallelization }, range =>
                            {
                                try
                                {
                                    int currentChunk = range.Item1 / ((dataLengthPerImage >> parallelizationChunkSize));
                                    chunkUpdated[indexInHistogramDataArray][currentChunk] = true;
                                    for (int q = range.Item1; q < range.Item2; q++)
                                    {
                                        int n = q * 3;
                                        //This is a temporary fix. So the exception doesn't get thrown every time while debugging
                                        //if (FrameData.pixelData.Length <= (chan * planes + v) * dataLengthPerImage + q)
                                        //{
                                        //    continue;
                                        //}
                                        //[TO DO] find out why bad buffer could occur due to latency.
                                        ushort valRaw = fd.pixelData[rawArrayPositionModifier + q];
                                        valRaw = (ushort)Math.Min(maxPixelBitSize, valRaw);
                                        if (0 > valRaw)
                                        {
                                            _rawImg[channelIndex][vIndex][n] = _rawImg[channelIndex][vIndex][n + 1] = _rawImg[channelIndex][vIndex][n + 2] = 0;

                                            if (ChannelDisplayEnable[channelGroup][channelIndex])
                                            {
                                                _24BitPixelDataChannelComposite[vIndex][n] = _24BitPixelDataChannelComposite[vIndex][n + 1] = _24BitPixelDataChannelComposite[vIndex][n + 2] = 0;
                                                if (copyToBackground)
                                                {
                                                    _24BitPixelDataForBackground[vIndex][n] = _24BitPixelDataForBackground[vIndex][n + 1] = _24BitPixelDataForBackground[vIndex][n + 2] = 0;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            var palette = pixelDataHistogram.Palette;
                                            Color col = _grayscaleLUT.Colors[palette[valRaw]];

                                            if (doGrayscale)
                                            {
                                                if (_onlyShowTrueSaturation)
                                                {
                                                    if (valRaw == 0)
                                                    {
                                                        col = minForSaturation;
                                                    }
                                                    else if (valRaw >= maxPixelValueForSaturation)
                                                    {
                                                        col = maxForSaturation;
                                                    }
                                                    else
                                                    {
                                                        col = _grayscaleLUT.Colors[palette[valRaw]];
                                                    }
                                                }
                                                else 
                                                {
                                                    if (palette[valRaw] == 0)
                                                    {
                                                        col = minForSaturation;
                                                    }
                                                    else if (palette[valRaw] == 255)
                                                    {
                                                        col = maxForSaturation;
                                                    }
                                                    else
                                                    {
                                                        col = _grayscaleLUT.Colors[palette[valRaw]];
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (safeToLookupColor)
                                                {
                                                    col = ChannelLuts[channelGroup][colorIndex][palette[valRaw]];
                                                }
                                            }

                                            _rawImg[chan][vIndex][n] = col.R;
                                            _rawImg[chan][vIndex][n + 1] = col.G;
                                            _rawImg[chan][vIndex][n + 2] = col.B;
                                            if (addChannelToComposite)
                                            {
                                                //:TODO: For now create a single composite for sequential capture
                                                if (IsInSequentialMode)
                                                {
                                                    if (_24BitPixelDataChannelComposite[0][n] < col.R) _24BitPixelDataChannelComposite[0][n] = col.R;
                                                    if (_24BitPixelDataChannelComposite[0][n + 1] < col.G) _24BitPixelDataChannelComposite[0][n + 1] = col.G;
                                                    if (_24BitPixelDataChannelComposite[0][n + 2] < col.B) _24BitPixelDataChannelComposite[0][n + 2] = col.B;
                                                }
                                                else
                                                {
                                                    if (_24BitPixelDataChannelComposite[vIndex][n] < col.R) _24BitPixelDataChannelComposite[vIndex][n] = col.R;
                                                    if (_24BitPixelDataChannelComposite[vIndex][n + 1] < col.G) _24BitPixelDataChannelComposite[vIndex][n + 1] = col.G;
                                                    if (_24BitPixelDataChannelComposite[vIndex][n + 2] < col.B) _24BitPixelDataChannelComposite[vIndex][n + 2] = col.B;

                                                    if (copyToBackground)
                                                    {
                                                        if (_24BitPixelDataForBackground[vIndex][n] < col.R) _24BitPixelDataForBackground[vIndex][n] = col.R;
                                                        if (_24BitPixelDataForBackground[vIndex][n + 1] < col.G) _24BitPixelDataForBackground[vIndex][n + 1] = col.G;
                                                        if (_24BitPixelDataForBackground[vIndex][n + 2] < col.B) _24BitPixelDataForBackground[vIndex][n + 2] = col.B;
                                                    }
                                                }
                                            }
                                        }

                                        //only build the histogram if the color mode is selected when full frame is ready.
                                        //This will allow histograms for all of the channels to be available simultaneously
                                        if (resetPixelDataHistogram)
                                        {
                                            byte valRawHist = (byte)(valRaw >> shiftValue);
                                            histogramDataToBeCombined[indexInHistogramDataArray][currentChunk][valRawHist]++;
                                            //pixelDataHistogram.HistogramData[valRawHist]++;
                                            if (valRawHist < pixelDataHistogram.Min)
                                            {
                                                pixelDataHistogram.Min = valRawHist;
                                            }

                                            if (valRawHist > pixelDataHistogram.Max && valRawHist <= _whitePointMaxVal)
                                            {
                                                pixelDataHistogram.Max = valRawHist;
                                            }
                                        }
                                    }
                                }
                                catch (Exception e)
                                {
                                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " Exception: " + e.Message);
                                }
                            }
                            );

                        if (resetPixelDataHistogram)
                        {
                            for (int i = 0; i < PIXEL_DATA_HISTOGRAM_SIZE; i++)
                            {
                                for (int j = 0; j < histogramDataToBeCombined[indexInHistogramDataArray].Length; j++)
                                {
                                    if (chunkUpdated[indexInHistogramDataArray][j])
                                    {
                                        pixelDataHistogram.HistogramData[i] += histogramDataToBeCombined[indexInHistogramDataArray][j][i];
                                    }
                                }
                            }

                            if ((pixelDataHistogram.IsAutoPressed || pixelDataHistogram.IsContinuousAutoChecked) && (pixelDataHistogram.Min != pixelDataHistogram.BlackPoint || pixelDataHistogram.Max != pixelDataHistogram.WhitePoint)) // TODO: this code is somehwere else too...
                            {
                                pixelDataHistogram.BlackPoint = pixelDataHistogram.Min;
                                pixelDataHistogram.WhitePoint = pixelDataHistogram.Max;
                            }

                        }
                    }
                    else if (fd.contiguousChannels)
                    {
                        if ((fd.channelSelection & (0x0001 << channelIndex)) > 0)
                        {
                            ++chan;
                        }
                    }

                }
                if (resetPixelDataHistogram)
                {
                    PixelDataUpdated?.Invoke(imageIdentifier);
                }
            } // channelIndex

/*            if (_calculateBitmapUpdateRate)
            {
                _frameCount++;
                _endTime = DateTimeOffset.Now.ToUnixTimeMilliseconds();
                if (_endTime - _startTime >= 4000)
                {
                    _bitmapUpdateRate = (double)_frameCount / ((_endTime - _startTime) / 1000.0);
                    _startTime = _endTime;
                    BitmapUpdateRateUpdated?.BeginInvoke(null, null);
                    _frameCount = 0;
                }
            }*/

            return true;
        }

        bool Process24BitRefImage(bool resetPixelDataHistogram, int numChannels, int verticalImages, int dataLengthPerImage, int shiftValue, FrameData fd, out bool invalidated)
        {
            // load reference channel
            bool refToRefChann = false;
            invalidated = false;
            ushort[] refChannShortArray = null;
            if (AllowReferenceImage && _referenceChannelEnabled)
            {
                string refChannDir = Application.Current.Resources["AppRootFolder"].ToString() + "\\ReferenceChannel.tif";
                if (File.Exists(_referenceChannelImagePath))   // ref channel file existance
                {
                    long width = 0;
                    long height = 0;
                    long colorChannels = 0;
                    long bitsPerChannel = 0;
                    if (LoadRefChannInfo(_referenceChannelImagePath, ref width, ref height, ref colorChannels, ref bitsPerChannel))    // load dimention of ref image
                    {
                        switch (bitsPerChannel)
                        {
                            case 16:
                                IntPtr refChannIntPtr;
                                if (width * height == dataLengthPerImage)
                                {
                                    refChannIntPtr = Marshal.AllocHGlobal(Convert.ToInt32(width) * Convert.ToInt32(height) * 2);

                                    if (LoadRefChann(_referenceChannelImagePath, ref refChannIntPtr))  // load ref image
                                    {
                                        try
                                        {
                                            refChannShortArray = new ushort[Convert.ToInt32(width) * Convert.ToInt32(height)];
                                            MemoryCopyManager.CopyIntPtrMemory(refChannIntPtr, refChannShortArray, 0, Convert.ToInt32(width) * Convert.ToInt32(height));
                                            refToRefChann = true;
                                        }
                                        catch (Exception e)
                                        {
                                            ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, e.Message);
                                        }
                                    }
                                    Marshal.FreeHGlobal(refChannIntPtr);
                                }
                                break;
                            case 8:
                            default:
                                ReferenceChannelInvalidated?.Invoke();
                                refToRefChann = false;
                                invalidated = true;
                                break;
                        }
                    }
                }
            }
            //in the interest of speed we are seperating the reference channel case
            //without a reference channel the logic will run faster since the
            //conditionals per pixel are removed.
            if (refToRefChann)
            {
                //factor is derived by taking CAMERA_MAX_INTENSITY_VALUE/256
                //const double FULL_RANGE_NORMALIZATION_FACTOR = 64.0;
                ushort[] rawArray = fd.pixelData;
                for (int k = 0; k < numChannels; k++)
                {
                    for (int v = 0; v < verticalImages; v++)
                    {
                        if ((ChannelDisplayEnable[v][k] && (fd.channelSelection & (0x0001 << k)) == 0))
                        {
                            continue;
                        }

                        ImageIdentifier imageIdentifier = new ImageIdentifier(k, v);
                        PixelDataHistogramInfo pixelDataHistogram = GetOrCreatePixelDataHistogram(imageIdentifier);

                        lock (UpdateHistogramsLock)
                        {
                            if (resetPixelDataHistogram)
                            {
                                pixelDataHistogram.HistogramData = new int[PIXEL_DATA_HISTOGRAM_SIZE];
                            }
                            if (ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP][k])
                            {
                                if (pixelDataHistogram.IsWhiteBlackPointChanged)
                                {
                                    BuildNormalizationPalettes(pixelDataHistogram);
                                    pixelDataHistogram.IsWhiteBlackPointChanged = false;
                                }
                                int n = k;
                                if (3 == k)
                                {
                                    rawArray = refChannShortArray;
                                    n = 0;
                                }
                                for (int i = 0, j = 0; j < dataLengthPerImage; i += 3, j++)
                                {
                                    int offset = (n * verticalImages + v) * dataLengthPerImage + j;
                                    ushort valRaw = rawArray[offset];
                                    Color col;
                                    double maxPixelBitSize = 256 * Math.Pow(2, shiftValue);
                                    //when the reference channel option is on. do not copy the data from channel 3
                                    valRaw = (ushort)Math.Min(maxPixelBitSize, valRaw);
                                    col = ChannelLuts[DEFAULT_CHANNEL_GROUP][k][pixelDataHistogram.Palette[valRaw]];
                                    _rawImg[k][v][i] = col.R;
                                    _rawImg[k][v][i + 1] = col.G;
                                    _rawImg[k][v][i + 2] = col.B;

                                    if (_24BitPixelDataChannelComposite[v][i] < col.R) _24BitPixelDataChannelComposite[v][i] = col.R;
                                    if (_24BitPixelDataChannelComposite[v][i + 1] < col.G) _24BitPixelDataChannelComposite[v][i + 1] = col.G;
                                    if (_24BitPixelDataChannelComposite[v][i + 2] < col.B) _24BitPixelDataChannelComposite[v][i + 2] = col.B;

                                    //only build the histogram if the color mode is selected when full frame is ready.
                                    //This will allow histograms for all of the channels to be available simultaneously
                                    if (resetPixelDataHistogram)
                                    {
                                        byte valRawHist = (byte)(valRaw >> shiftValue);
                                        pixelDataHistogram.HistogramData[valRawHist]++;

                                        if (valRawHist < pixelDataHistogram.Min)
                                        {
                                            pixelDataHistogram.Min = valRawHist;
                                        }

                                        if (valRawHist > pixelDataHistogram.Max && valRawHist <= _whitePointMaxVal)
                                        {
                                            pixelDataHistogram.Max = valRawHist;
                                        }
                                    }
                                }
                            }
                        }
                        if (resetPixelDataHistogram)
                        {
                            if ((pixelDataHistogram.IsAutoPressed || pixelDataHistogram.IsContinuousAutoChecked) && (pixelDataHistogram.Min != pixelDataHistogram.BlackPoint || pixelDataHistogram.Max != pixelDataHistogram.WhitePoint)) // TODO: this code is somehwere else too...
                            {
                                pixelDataHistogram.BlackPoint = pixelDataHistogram.Min;
                                pixelDataHistogram.WhitePoint = pixelDataHistogram.Max;
                            }
                            PixelDataUpdated?.Invoke(imageIdentifier);
                        }
                    } // v
                } // channelIndex
                return true;
            }

            return false;
        }

        bool UpdateChannelData(string[] fileNames, int zToRead, byte enabledChannels = 0, byte channelToRead = 0, int timeToRead = 0, int imageWidth = 0, int imageHeight = 0, int imageDepth = 0, bool rawContainsDisabledChannels = true, CaptureFile imageType = CaptureFile.FILE_TIFF)
        {
            return UpdateExperimentFileFormatChannelData(fileNames, zToRead, enabledChannels, channelToRead, timeToRead, imageWidth, imageHeight, imageDepth, rawContainsDisabledChannels, imageType);
        }

        ///// <summary>
        ///// Updates image data in selected channels for experiment file projects
        ///// </summary>
        ///// <param name="fileNames"> Files to read </param>
        ///// <param name="chEnabled"> Bitmask of enabled channels</param>
        ///// <param name="selectedChannel"> Selected channel </param>
        ///// <param name="selectedZ"> The z coordinate of the selected channel </param>
        ///// <param name="selectedTime"> The time coordinate of the selected channel </param>
        ///// <param name="imageWidth"> The total bitmapWidth of the image in pixels </param>
        ///// <param name="imageHeight"> The total bitmapHeight of the image in pixels </param>
        ///// <param name="imageDepth"> The total depth(z) of the image in pixels </param>
        ///// <param name="rawContainsDisabledChannels"> Boolean describing is a raw file is structured with blank data for disabled channels, or excludes
        /////  the data blocks for disabled channels all togeather </param>
        bool UpdateExperimentFileFormatChannelData(string[] fileNames, int zIndex, byte chEnabled = 0, int selectedChannel = 0, int selectedTime = 0,
            int imageWidth = 0, int imageHeight = 0, int imageDepth = 0, bool rawContainsDisabledChannels = true, CaptureFile imageType = CaptureFile.FILE_TIFF)
        {
            bool ret = true;

            int width = 0;
            int height = 0;
            int colorChannels = 0;
            long bitsPerChannel = 0;
            int size = 0;
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
                if (i == MAX_CHANNELS) return false;

                //=== Read File ===
                if (File.Exists(fileNames[i]))
                {
                    DirectoryInfo di = new DirectoryInfo(ExperimentPath);

                    switch (imageType)
                    {
                        case CaptureFile.FILE_BIG_TIFF:

                            int regionCount = 0, chCount = 0, zMaxCount = 0, timeCount = 0, specCount = 0;
                            string temp = string.Format("{0}", "Image");
                            string fname = (di.GetFiles(temp + "*.tif*")[0].FullName);
                            GetImageStoreInfo(fname, 0, ref regionCount, ref width, ref height, ref chCount, ref zMaxCount, ref timeCount, ref specCount);

                            if ((width > 0) && (height > 0))
                            {
                                size = width * height * MAX_CHANNELS * 2;

                                if (IntPtr.Zero == _zPreviewImageData)
                                {
                                    _zPreviewImageData = Marshal.AllocHGlobal(size);
                                }
                                else if (_zPreviewImageDataSize != (UInt64)size)
                                {
                                    Marshal.FreeHGlobal(_zPreviewImageData);
                                    _zPreviewImageData = Marshal.AllocHGlobal(size);
                                }

                                _zPreviewImageDataSize = (UInt64)size;

                                ReadImageStoreData(_zPreviewImageData, MAX_CHANNELS, width, height, zIndex, selectedTime, 0);
                            }
                            break;
                        case CaptureFile.FILE_RAW:
                            fname = fileNames[i];
                            int enabledChannelCount = 0;
                            for (int k = 0; k < ImageViewMBase.MAX_CHANNELS; k++)
                            {
                                if (ChannelDisplayEnable[DEFAULT_CHANNEL_GROUP][k] && (FrameData.channelSelection & (0x0001 << k)) > 0)
                                {
                                    enabledChannelCount++;
                                }
                            }

                            width = imageWidth;
                            height = imageHeight;
                            size = width * height * MAX_CHANNELS * 2;

                            if (IntPtr.Zero == _zPreviewImageData)
                            {
                                _zPreviewImageData = Marshal.AllocHGlobal(size);
                            }
                            else if (_zPreviewImageDataSize != (UInt64)size)
                            {
                                Marshal.FreeHGlobal(_zPreviewImageData);
                                _zPreviewImageData = Marshal.AllocHGlobal(size);
                            }

                            _zPreviewImageDataSize = (UInt64)size;

                            if (1 == enabledChannelCount)
                            {
                                ReadChannelImagesRaw(ref _zPreviewImageData, 1, fname, 1, 0, size, selectedTime);
                            }
                            else
                            {
                                for (int ch = 0; ch < MAX_CHANNELS; ch++)
                                {
                                    ReadChannelImageRawSlice(_zPreviewImageData, fname, width, height, imageDepth, MAX_CHANNELS, ch, zIndex, selectedTime, FrameData.channelSelection, rawContainsDisabledChannels);
                                }
                            }
                            break;
                        case CaptureFile.FILE_TIFF:
                            ReadImageInfo(fileNames[i], ref width, ref height, ref colorChannels, ref bitsPerChannel);
                            if ((width > 0) && (height > 0))
                            {
                                size = width * height * MAX_CHANNELS * 2;

                                if (IntPtr.Zero == _zPreviewImageData)
                                {
                                    _zPreviewImageData = Marshal.AllocHGlobal(size);
                                }
                                else if (_zPreviewImageDataSize != (UInt64)size)
                                {
                                    Marshal.FreeHGlobal(_zPreviewImageData);
                                    _zPreviewImageData = Marshal.AllocHGlobal(size);
                                }

                                _zPreviewImageDataSize = (UInt64)size;

                                //read the image and output the buffer with image data
                                ReadChannelImages(fileNames, MAX_CHANNELS, ref _zPreviewImageData, width, height);
                            }
                            break;
                        default:
                            break;
                    }

                    int length = (width * height);

                    if ((_zPreviewPixelData[zIndex] == null) || (_zPreviewPixelData[zIndex].Length != (length * MAX_CHANNELS)))
                    {
                        _zPreviewPixelData[zIndex] = new ushort[length * MAX_CHANNELS];
                    }

                    MemoryCopyManager.CopyIntPtrMemory(_zPreviewImageData, _zPreviewPixelData[zIndex], 0, length * MAX_CHANNELS);
                }
            }
            catch
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + "File not found exception");
                ret = false;
            }
            if (width != imageWidth || height != imageHeight)
            {
                ret = false;
            }

            return ret;
        }

        #endregion Methods

        #region Nested Types

        private class FrameDataHistory
        {
            #region Fields

            FrameDataHistoryElement[] _frameDataList;
            bool _frameInfoSet;
            int _lastUpdatedIndex;

            #endregion Fields

            #region Constructors

            internal FrameDataHistory(int historySize)
            {
                _frameInfoSet = false;
                _lastUpdatedIndex = -1;
                _frameDataList = new FrameDataHistoryElement[historySize];
            }

            #endregion Constructors

            #region Properties

            public FrameDataHistoryElement[] FrameDataList
            {
                get => _frameDataList;
                set => _frameDataList = value;
            }

            public bool FrameInfoSet
            {
                get => _frameInfoSet;
                set => _frameInfoSet = value;
            }

            public int GetElementIndexFromScanIndex(int scanAreaIndex)
            {
                int index = -1;
                for(int i = 0; i < _frameDataList.Length; i++)
                {
                    if (_frameDataList[i]?.StoredLastFrameInfo.scanAreaIndex == scanAreaIndex)
                    {
                        index = i;
                        break;
                    }
                }
                return index;
            }

            public FrameDataHistoryElement LastUpdatedFrame
            {
                get
                {
                    if (_lastUpdatedIndex != -1 && _frameDataList?.Length > _lastUpdatedIndex)
                    {
                        return _frameDataList[_lastUpdatedIndex];
                    }
                    return null;
                }
            }

            public int LastUpdatedIndex
            {
                get => _lastUpdatedIndex;
                set => _lastUpdatedIndex = value;
            }

            #endregion Properties

            #region Methods

            public bool IsFrameDataUpdated()
            {
                int index = 0; // TODO: See if outputting the index would help reduce complexity
                if (_frameDataList == null)
                {
                    return false;
                }
                foreach (FrameDataHistoryElement frameDataHistory in _frameDataList)
                {
                    index++;
                    if (frameDataHistory != null && frameDataHistory.FrameDataUpdated)
                    {
                        return true;
                    }
                }
                return false;
            }

            #endregion Methods
        }

        private class FrameDataHistoryElement
        {
            #region Fields

            public readonly object dataLock = new object();
            FrameData _frameData;
            volatile bool _frameDataUpdated;
            volatile bool _needsBitmapUpdate;
            FrameInfoStruct _storedLastFrameInfo;

            #endregion Fields

            #region Constructors

            internal FrameDataHistoryElement()
            {
                _frameDataUpdated = false;
                _needsBitmapUpdate = false;
                _frameData = new FrameData();
                _storedLastFrameInfo = new FrameInfoStruct();
            }

            #endregion Constructors

            #region Properties

            public bool FrameDataUpdated
            {
                get
                {
                    return _frameDataUpdated;
                }
                set
                {
                    _frameDataUpdated = value;
                }
            }

            public bool NeedsBitmapUpdate
            {
                get => _needsBitmapUpdate;
                set => _needsBitmapUpdate = value;
            }

            public FrameData StoredFrameData
            {
                get
                {
                    return _frameData;
                }
                set
                {
                    _frameData = value;
                }
            }

            public FrameInfoStruct StoredLastFrameInfo
            {
                get
                {
                    return _storedLastFrameInfo;
                }
                set
                {
                    _storedLastFrameInfo = value;
                }
            }

            #endregion Properties
        }

        private class ImageGridParameters
        {
            #region Constructors

            internal ImageGridParameters()
            {
                PixelHeight = new Dictionary<int, int>();
                PixelWidth = new Dictionary<int, int>();
            }

            #endregion Constructors

            #region Properties

            public int Channels
            {
                get; set;
            }

            public int Columns
            {
                get; set;
            }

            public bool IsVertical
            {
                get; set;
            }

            public Dictionary<int, int> PixelHeight
            {
                get; set;
            }

            public Dictionary<int, int> PixelWidth
            {
                get; set;
            }

            public int Planes
            {
                get; set;
            }

            public int Rows
            {
                get; set;
            }

            public double AspectRatioYScale
            {
                get; set;
            }

            #endregion Properties

            #region Methods

            // return true if matching, false otherwise
            public bool Compare(int columns, int rows, int pixelWidth, int pixelHeight, bool isVertical, int channels, int planes, double aspectRatioYScale, int index)
            {
                return columns == Columns
                    && rows == Rows
                    && isVertical == IsVertical
                    && channels == Channels
                    && planes == Planes
                    && aspectRatioYScale == AspectRatioYScale
                    && PixelWidth.ContainsKey(index)
                    && PixelHeight.ContainsKey(index)
                    && pixelWidth == PixelWidth[index]
                    && pixelHeight == PixelHeight[index];
            }

            /// <summary>
            /// Enumerator goes through grid defined by Columns and Rows.
            /// If IsVertical == true, traverse first by row then by column.
            /// If IsVertical == false, traverse first by column then by row.
            /// </summary>
            /// <returns> An iterator of X,Y coordinates to hit every space on the grid in order. </returns>
            public IEnumerable<Tuple<int, int>> GetGridEnumerator()
            {
                if (this.IsVertical)
                {
                    for (int i = 0; i < this.Columns; i++)
                    {
                        for (int j = 0; j < this.Rows; j++)
                        {
                            // traverse each row left to right, then move to next column top to bottom
                            yield return new Tuple<int, int>(j, i);
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < this.Rows; i++)
                    {
                        for (int j = 0; j < this.Columns; j++)
                        {
                            // traverse each column top to bottom, then move to next row left to right
                            yield return new Tuple<int, int>(i, j);
                        }
                    }
                }
            }

            #endregion Methods
        }

        #endregion Nested Types
    }
}