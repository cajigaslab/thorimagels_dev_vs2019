namespace ImageViewMVM.ViewModels
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Runtime.Serialization.Formatters.Binary;
    using System.Security.Cryptography;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    using HistogramControl.ViewModel;

    using ImageViewMVM.Models;

    using Microsoft.Win32;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    using ImageViewMVM.Shared;

    #region Enumerations

    public enum OrthogonalViewStatus
    {
        INACTIVE, ACTIVE, HOLD
    }

    #endregion Enumerations

    public class ImageViewVVMBase : VMBase, IViewModelActions
    {
        #region Fields

        public Canvas _imageCanvas;

        protected ImageViewMBase _imageViewM;
        protected string _mainViewModel;

        const int CHANNEL_BUTTON_SIZE = 51;
        const int LUT_SIZE = 256;
        const int MAX_CHANNELS = 4;
        const double MAX_ZOOM = 1000; // In  1000x
        const double MIN_ZOOM = .001; // Out 100x

        readonly List<string> _currentChannelsLutFiles = new List<string>();
        private readonly ObservableCollection<ObservableCollection<HistogramControlViewModel>> _histogramViewModelsGrid = new ObservableCollection<ObservableCollection<HistogramControlViewModel>>();
        readonly Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        readonly string[] _wavelengthNames = new string[MAX_CHANNELS];

        private static bool _bitmapLoaded = false;

        List<Color>[] _allLuts;

        //static readonly object _syncLock = new object();
        string _appDocPath;
        bool _bitmapReady = false;
        private String _bitmapUpdateRateText = "";
        private Visibility _bitmapUpdateRateTextVisibility = Visibility.Collapsed;
        private ICommand _browseForReferenceImageCommand;
        bool _bwOrthogonalImageLoaderDone = true;
        private int _cachedChannelSelection = 0;
        private int _cachedChannelSelectionReferenceImage = 0;
        private int _cachedNumChannelsForHistogram = 0;
        private int _cachedNumChannelsReferenceImage = 0;
        private int _cachedNumPlanesForHistogram = 0;
        string _chanAName;
        string _chanBName;
        string _chanCName;
        string _chanDName;
        ICommand _changeColorSettingsCommand;
        ObservableCollection<ChannelGroupViewModel> _channelGroups = new ObservableCollection<ChannelGroupViewModel>();
        ObservableCollection<ImageViewControl.LUTComboBoxView> _colorPalettesUI;
        ICommand _colorSettingsOKCommand;
        ImageViewControl.LUTSettings _colorSettingsWindow;
        string[] _defaultChannelColorNames = new string[MAX_CHANNELS];
        Color[] _defaultChannelColors = new Color[MAX_CHANNELS];
        ICommand _deleteAllROIsCommand = null;
        ICommand _deleteSelectedROIsCommand;
        string _helpText = "";
        Visibility _helpTextVisibility = Visibility.Hidden;
        private Thickness _helpToolbarMargin;
        List<HistogramChannelSettings> _histogramSettings = new List<HistogramChannelSettings>();
        ICommand _imageReferenceChannelSaveAsCommand;
        ICommand _imageResetSizeCommand;
        ICommand _imageSaveAsCommand;
        ICommand _imageSaveAsReferenceCommand;
        ICommand _imageSetFullSizeCommand;
        bool _isLineChartHistogram = false;
        bool _isMergedHistogram = false;
        bool _isOrthogonalPositionLocked = false;
        bool _isReticleChecked = false;
        bool _isScaleButtonChecked = false;
        Visibility _isTileDisplayButtonVisible = Visibility.Visible;
        double _ivHeight;
        double _iVScrollBarHeight;
        DateTime _lastOrthogonalViewUpdateTime;
        bool _loaded = false;
        ICommand _loadROIsFromXamlCommand = null;
        string[] _lutFiles;
        List<string> _lutNames;
        int _numHistogramGridLines;
        long _orthogonalChangeCount = 0;
        ICommand _orthogonalDisplayOptionsCommand;
        int _orthogonalLineColorType = 0;
        ICommand _orthogonalLinesLockPositionCommand;
        int _orthogonalLineType = 0;

        //Point _orthogonalViewPosition = new Point(0, 0);
        Visibility _orthogonalViewVisibility = Visibility.Visible;
        int _progressPercentage;
        bool _requestHistogramRebuild = false;
        XmlDocument _reviewHardwareSettings;
        int _roiDrawingToolsSelectedIndex = 0;
        Thickness _roiToolbarMargin = new Thickness(0, 0, 0, 0);
        private bool[] _roiToolVisible = new bool[16] { true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false };
        int _saveDlgFilterIndex = 0;
        ICommand _saveNowCommand;
        string _sequentialExperimentPath;
        int _sPChannelColumn = 3;
        ImageViewControl.ProgressSplashScreen _splashOrthogonalView;
        int _sPROIToolsColumn = 2;
        int _sPToolsColumn = 3;
        Thickness _sPToolsMargin;
        int _tIndexFromReview = 1;
        ICommand _updateOrthogonalViewCommand = null;
        private double _verticalToolbarOffset;
        double _zoomLevel = 1;
        double _zoomOffsetX = 0;
        double _zoomOffsetY = 0;
        int _zStepNum = 1;

        #endregion Fields

        #region Constructors

        public ImageViewVVMBase(ImageViewMBase imageViewM)
        {
            _imageViewM = imageViewM;
            short firstLetter = ((short)'A');
            TotalDisplayedChannels = 0;
            _channelGroups.Add(new ChannelGroupViewModel());

            for (int i = 0; i < ImageViewMBase.MAX_CHANNELS; i++)
            {
                char channelLetter = ((char)(firstLetter + i));
                _channelGroups[0].Channels.Add(new ChannelViewModel() { ChannelName = "Chan" + channelLetter, GetPixelInfoDelegate = _imageViewM.GetPixelInfoDelegate, GroupIndex = 0, Index = i });
                _currentChannelsLutFiles.Add(string.Empty);
                _channelGroups[0].Channels[i].ChannelDisplayEnabledChanged += ImageViewVVMBase_ChannelDisplayEnabledChanged;
                TotalDisplayedChannels++;
            }
        }

        #endregion Constructors

        #region Delegates

        public delegate void UpdateProgressDelegate(int percentage);

        #endregion Delegates

        #region Events

        //notify 3D VolumeView that the color mapping has changed
        public event Action<bool> ColorMappingChanged;

        //Increase the view area if the image extends out of bonds for the vertical scroll bar
        public event Action<bool> IncreaseViewArea;

        //notify ImageView When a OrthogonalView images was loaded
        public event Action<bool> OrthogonalViewImagesLoaded;

        #endregion Events

        #region Properties

        public static bool BitmapLoaded
        {
            get
            {
                return _bitmapLoaded;
            }
            set
            {
                _bitmapLoaded = value;
            }
        }

        public bool AllowmROIBackgroundImage
        {
            get
            {
                return _imageViewM.AllowmROIBackgroundImage;
            }
            set
            {
                _imageViewM.AllowmROIBackgroundImage = value;
            }
        }

        /*        public bool BackupCompositeForBackground
                {
                    get
                    {
                        return _imageViewM.BackupCompositeForBackground;
                    }
                    set
                    {
                        _imageViewM.BackupCompositeForBackground = value;
                    }
                }*/
        public WriteableBitmap Bitmap
        {
            get
            {
                return _imageViewM.Bitmap;
            }
        }

        public WriteableBitmap Bitmap16
        {
            get
            {
                return _imageViewM.Bitmap16;
            }
        }

        public bool BitmapReady
        {
            get => _bitmapReady;
            set => SetProperty(ref _bitmapReady, value);
        }

        public String BitmapUpdateRateText
        {
            get => _bitmapUpdateRateText;
        }

        public Visibility BitmapUpdateRateTextVisibility
        {
            get => _bitmapUpdateRateTextVisibility;
            set => _bitmapUpdateRateTextVisibility = value;
        }

        public WriteableBitmap BitmapXZ
        {
            get => _imageViewM.BitmapXZ;
        }

        public WriteableBitmap BitmapYZ
        {
            get => _imageViewM.BitmapYZ;
        }

        public ICommand BrowseForReferenceImageCommand
        {
            get
            {
                if (_browseForReferenceImageCommand == null)
                {
                    _browseForReferenceImageCommand = new RelayCommand(() => BrowseForReferenceImage());
                }
                return _browseForReferenceImageCommand;
            }
        }

        public ICommand ChangeColorSettingsCommand
        {
            get => _changeColorSettingsCommand ?? (_changeColorSettingsCommand = new RelayCommand(() => ChangeColorSettings()));
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

        public ObservableCollection<ChannelGroupViewModel> ChannelGroups
        {
            get => _channelGroups;
        }

        public List<Tuple<int, int>> ChannelsPerSequence
        {
            get
            {
                return _imageViewM.ChannelsPerSequence;
            }
            set
            {
                _imageViewM.ChannelsPerSequence = value;
            }
        }

        public Action<bool> ColorMappingChangedAddHandler
        {
            get => ColorMappingChanged;
            set => ColorMappingChanged += value;
        }

        public Action<bool> ColorMappingChangedRemoveHandler
        {
            get => ColorMappingChanged;
            set => ColorMappingChanged -= value;
        }

        public ICommand ColorSettingsOKCommand
        {
            get => _colorSettingsOKCommand ?? (_colorSettingsOKCommand = new RelayCommand(() => SaveColorSettings()));
        }

        public string[] DefaultChannelColorNames
        {
            get
            {
                return _defaultChannelColorNames;
            }
        }

        public Color[] DefaultChannelColors
        {
            get
            {
                return _defaultChannelColors;
            }
        }

        public ICommand DeleteAllROIsCommand
        {
            get => _deleteAllROIsCommand ?? (_deleteAllROIsCommand = new RelayCommand(() => DeleteAllROIs()));
        }

        public ICommand DeleteSelectedROIsCommand
        {
            get => _deleteSelectedROIsCommand ?? (_deleteSelectedROIsCommand = new RelayCommand(() => DeleteSelectedROIs()));
        }

        public bool DisplayPixelAspectRatio
        {
            set
            {
                _imageViewM.DisplayPixelAspectRatio = value;
            }
            get
            {
                return _imageViewM.DisplayPixelAspectRatio;
            }
        }

        public ICommand DisplayROIStatsOptionsCommand
        {
            get => (ICommand)MVMManager.Instance[_mainViewModel, "DisplayROIStatsOptionsCommand", (object)new RelayCommand(() => { })];
        }

        public virtual bool DoneLoadingOrthogonalView
        {
            set
            {
                CloseProgressWindow();
            }
        }

        public bool EnableHistogramUpdate
        {
            get
            {
                return _imageViewM.EnableHistogramUpdate;
            }
            set
            {
                _imageViewM.EnableHistogramUpdate = value;
            }
        }

        public string ExperimentPath
        {
            set
            {
                _imageViewM.ExperimentPath = value;
                IsOrthogonalViewChecked = false;
            }
        }

        public bool EnableFullFOVReset
        {
            set
            {
                if (_imageViewM.FullFOVFrameData != null)
                {
                    FrameData = _imageViewM.FullFOVFrameData;
                }
            }
        }

        public FrameData FrameData
        {
            private get => _imageViewM.FrameData;
            set
            {
                if (_imageViewM.AllowReferenceImage)
                {
                    if (1 == (int)MVMManager.Instance["AreaControlViewModel", "EnableReferenceChannel", (object)0]
                    || (bool)MVMManager.Instance["CameraControlViewModel", "EnableReferenceChannel", (object)false])
                    {
                        //Cache values in case there is an error with the ref image process so the channels can be reset
                        _cachedNumChannelsReferenceImage = value.frameInfo.channels;
                        _cachedChannelSelectionReferenceImage = value.channelSelection;

                        //Store incoming values so model can allocate arrays properly
                        _imageViewM.ChannelSelectionBeforeRefImage = value.channelSelection;
                        _imageViewM.ChannelCountBeforeRefImage = value.frameInfo.channels;

                        //Intercept frame info if ref channel enabled to set the channels so arrays get allocated properly
                        value.frameInfo.channels = MAX_CHANNELS;//value.frameInfo.channels == MAX_CHANNELS? MAX_CHANNELS : value.frameInfo.channels + 1;
                        value.channelSelection = value.channelSelection | 0x001 << 3;

                        _imageViewM.ReferenceChannelEnabled = true;
                    }
                    else
                    {
                        _imageViewM.ReferenceChannelEnabled = false;
                    }
                }
                _imageViewM.FrameData = value;
                UpdateHistogramGridIfNeeded(); // must do this first since histograms grid could change
                UpdatePolarScaling();
                UpdateChannelVisibility();
            }
        }

        public bool FrameDataReadyToSet
        {
            get
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, "ImageViewVMBase FrameDataReadyToSet");
                return _imageViewM.FrameDataReadyToSet;
            }
        }

        public bool GrayscaleForSingleChannel
        {
            get
            {
                return _imageViewM.GrayscaleForSingleChannel;
            }
            set
            {
                _imageViewM.GrayscaleForSingleChannel = value;
                OnPropertyChanged("GrayscaleForSingleChannel");
            }
        }

        public string HelpText
        {
            get
            {
                return _helpText;
            }

            set
            {
                _helpText = value;
            }
        }

        public Visibility HelpTextVisibility
        {
            get
            {
                return _helpTextVisibility;
            }

            set
            {
                _helpTextVisibility = value;
            }
        }

        public Thickness HelpToolbarMargin
        {
            get
            {
                return _helpToolbarMargin;
            }

            set
            {
                _helpToolbarMargin = value;
                OnPropertyChanged("HelpToolbarMargin");
            }
        }

        public ObservableCollection<ObservableCollection<HistogramControlViewModel>> HistogramViewModels
        {
            get
            {
                return _histogramViewModelsGrid;
            }
        }

        public Canvas ImageCanvas
        {
            get => _imageCanvas;
            set => _imageCanvas = value;
        }

        public string ImageNameFormat
        {
            get
            {
                return _imageViewM.ImageNameFormat;
            }
        }

        public double ImageOffsetX
        {
            get => _zoomOffsetX;
            set => SetProperty(ref _zoomOffsetX, value);
        }

        public double ImageOffsetY
        {
            get => _zoomOffsetY;
            set => SetProperty(ref _zoomOffsetY, value);
        }

        public ICommand ImageReferenceChannelSaveAsCommand
        {
            get => _imageReferenceChannelSaveAsCommand ?? (_imageReferenceChannelSaveAsCommand = new RelayCommand(() => SaveReferenceChannelAs()));
        }

        public ICommand ImageResetSizeCommand
        {
            get => _imageResetSizeCommand ?? (_imageResetSizeCommand = new RelayCommand(() => ChangeColorSettings()));
        }

        public ICommand ImageSaveAsCommand
        {
            get => _imageSaveAsCommand ?? (_imageSaveAsCommand = new RelayCommand(() => SaveAs()));
        }

        public ICommand ImageSaveAsReferenceCommand
        {
            get => _imageSaveAsReferenceCommand ?? (_imageSaveAsReferenceCommand = new RelayCommand(() => SaveAsReference()));
        }

        public ICommand ImageSetFullSizeCommand
        {
            get => _imageSetFullSizeCommand ?? (_imageSetFullSizeCommand = new RelayCommand(() => ChangeColorSettings()));
        }

        public ObservableCollection<ObservableCollection<CompoundImage>> ImagesGrid
        {
            get => _imageViewM.ImagesGrid;
        }

        public bool ImageUpdateComplete
        {
            get => _imageViewM.ImageUpdateComplete;
        }

        public Action<bool> IncreaseViewAreaAction
        {
            get => IncreaseViewArea;
            set => IncreaseViewArea += value;
        }

        public bool IsInSequentialMode
        {
            get
            {
                return _imageViewM.IsInSequentialMode;
            }
            set
            {
                if (true == _imageViewM.IsInSequentialMode && false == value)
                {
                    _imageViewM.IsInSequentialMode = value;
                    _ = (Application.Current?.Dispatcher.Invoke(DispatcherPriority.Normal,
                new Action(delegate ()
                {
                    RebuildChannels();
                    _requestHistogramRebuild = true;
                    LoadColorImageSettings();
                    _imageViewM.ResetBitmap();
                    EnableHistogramUpdate = true;
                })));
                }
                else
                {
                    _imageViewM.IsInSequentialMode = value;
                }
            }
        }

        public bool IsLineChartHistogram
        {
            get => _isLineChartHistogram;
            set
            {
                if (value == _isLineChartHistogram)
                {
                    return;
                }
                _isLineChartHistogram = value;

                foreach (var histogramViewModelRow in _histogramViewModelsGrid)
                {
                    foreach (var histogramViewModel in histogramViewModelRow)
                    {
                        histogramViewModel.IsLineChart = _isLineChartHistogram;
                    }
                }
            }
        }

        public bool IsOrthogonalPositionLocked
        {
            get => _isOrthogonalPositionLocked;
            set => SetProperty(ref _isOrthogonalPositionLocked, value);
        }

        public bool IsOrthogonalViewChecked
        {
            get
            {
                return _imageViewM.OrthogonalViewEnabled;
            }
            set
            {
                if (_imageViewM.OrthogonalViewEnabled != value)
                {
                    _imageViewM.OrthogonalViewEnabled = value;
                    OnPropertyChanged("IsOrthogonalViewChecked");

                    if (_imageViewM.OrthogonalViewEnabled)
                    {
                        InitOrthogonalView();
                    }
                    //Clear the orthogonal bitmaps when orthogonal view is unchecked from another view model
                    if (value == false)
                    {
                        _imageViewM.ClearOrthogonalBitmaps = true;
                        OnPropertyChanged("BitmapXZ");
                        OnPropertyChanged("BitmapYZ");
                        ++OrthogonalChangeCount;
                        _imageViewM.ClearOrthogonalBitmaps = false;
                    }
                }
            }
        }

        public bool IsReticleChecked
        {
            get => _isReticleChecked;
            set
            {
                if (0 >= ImagesGrid?.Count)
                {
                    SetProperty(ref _isReticleChecked, false);
                    ROIDrawingToolsSelectedIndex = 0;
                    return;
                }
                SetProperty(ref _isReticleChecked, value);
                ROIDrawingToolsSelectedIndex = 0;
                if (value)
                {
                    OverlayManagerClass.Instance.InitReticle(true);
                    OverlayManagerClass.Instance.InitSelectROI();
                }
                else
                {
                    OverlayManagerClass.Instance.InitReticle(false);
                    OverlayManagerClass.Instance.InitSelectROI();
                    if (0 == OverlayManagerClass.Instance.ROICount)
                    {
                        //Force reset ROI count
                        OverlayManagerClass.Instance.ClearAllObjects();
                    }
                }
            }
        }

        public bool IsRolloverButtonChecked
        {
            get
            {
                return _imageViewM.IsRolloverButtonChecked;
            }
            set
            {
                _imageViewM.IsRolloverButtonChecked = value;
            }
        }

        public bool IsScaleButtonChecked
        {
            get => _isScaleButtonChecked;
            set
            {
                if (0 >= ImagesGrid?.Count)
                {
                    SetProperty(ref _isScaleButtonChecked, false);
                    ROIDrawingToolsSelectedIndex = 0;
                    return;
                }
                SetProperty(ref _isScaleButtonChecked, value);
                ROIDrawingToolsSelectedIndex = 0;
                if (value)
                {
                    OverlayManagerClass.Instance.InitScale(true);
                    OverlayManagerClass.Instance.InitSelectROI();
                }
                else
                {
                    OverlayManagerClass.Instance.InitScale(false);
                    OverlayManagerClass.Instance.InitSelectROI();
                }
            }
        }

        public bool IsSingleChannel
        {
            get
            {
                return _imageViewM.IsSingleChannel;
            }
        }

        public bool IsSingleMergedHistogram
        {
            get => _isMergedHistogram;
            set
            {
                if (value == _isMergedHistogram)
                {
                    return;
                }
                _isMergedHistogram = value;
                Application.Current.Dispatcher.Invoke(new Action(() =>
                {
                    BuildHistograms();
                }));
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

        //Save the current height of the image display space
        public double IVHeight
        {
            get
            {
                return _ivHeight;
            }
            set
            {
                _ivHeight = value;
                IVScrollBarHeight = _ivHeight;
            }
        }

        public double IVScrollBarHeight
        {
            get
            {
                return _iVScrollBarHeight;
            }
            set
            {
                //Compare the height of the display space with the height of the image
                // if the image height is bigger, make the scrollbar visible and set it's height
                // Leave a small gap of 10 pixels below the image to see the end of it easier
                _iVScrollBarHeight = ((value + 10) > (IVHeight + 11)) ? (value + 10) : IVHeight;
                IncreaseViewArea?.Invoke(true);
                if (IVScrollBarHeight > IVHeight)
                {
                    MVMManager.Instance[_mainViewModel, "IVScrollbarVisibility"] = ScrollBarVisibility.Visible;
                }
                else
                {
                    MVMManager.Instance[_mainViewModel, "IVScrollbarVisibility"] = ScrollBarVisibility.Hidden;
                }

                OnPropertyChanged("IVScrollBarHeight");
            }
        }

        public FrameInfoStruct LastFrameInfo
        {
            get => _imageViewM.LastFrameInfo;
        }

        public ICommand LoadXamlROIsCommand
        {
            get => _loadROIsFromXamlCommand ?? (_loadROIsFromXamlCommand = new RelayCommand(() => LoadROIsFromXAML()));
        }

        public int mROIPriorityIndex
        {
            set
            {
                _imageViewM.mROIPriorityIndex = value;
                var ROIs = OverlayManagerClass.Instance.GetModeROIs(Mode.MICRO_SCANAREA);
                if (ROIs.Count > value && value >= 0)
                {
                    SetOverlayItemsForROI(ROIs[value]);
                }
            }
        }

        public bool mROISpatialDisplaybleEnable
        {
            get => _imageViewM.mROISpatialDisplaybleEnable;
            set
            {
                _imageViewM.mROISpatialDisplaybleEnable = value;
            }
        }

        public int NumHistogramGridLines
        {
            get => _numHistogramGridLines;
            set
            {
                if (_numHistogramGridLines < 0)
                {
                    return;
                }
                _numHistogramGridLines = value;
                foreach (var histogramViewModelRow in _histogramViewModelsGrid)
                {
                    foreach (var histogramViewModel in histogramViewModelRow)
                    {
                        histogramViewModel.NumGridLines = _numHistogramGridLines;
                    }
                }
                OnPropertyChanged("NumHistogramGridLines");
            }
        }

        public bool OnlyShowTrueSaturation
        {
            get => _imageViewM.OnlyShowTrueSaturation;
            set => _imageViewM.OnlyShowTrueSaturation = value;
        }

        public long OrthogonalChangeCount
        {
            get => _orthogonalChangeCount;
            set => SetProperty(ref _orthogonalChangeCount, value);
        }

        public ICommand OrthogonalDisplayOptionsCommand
        {
            get => _orthogonalDisplayOptionsCommand ?? (_orthogonalDisplayOptionsCommand = new RelayCommand(() => DisplayOrthogonalOptions()));
        }

        public int OrthogonalLineColorType
        {
            get => _orthogonalLineColorType;
            set => SetProperty(ref _orthogonalLineColorType, value);
        }

        public ICommand OrthogonalLinesLockPositionCommand
        {
            get => _orthogonalLinesLockPositionCommand ?? (_orthogonalLinesLockPositionCommand = new RelayCommand(() => ChangeColorSettings()));
        }

        public int OrthogonalLineType
        {
            get => _orthogonalLineType;
            set => SetProperty(ref _orthogonalLineType, value);
        }

        public Point OrthogonalViewPosition
        {
            get
            {
                return _imageViewM.OrthogonalViewPosition;
            }
            set
            {
                _imageViewM.OrthogonalViewPosition = value;
                UpdateOrthogonalViewImages();
            }
        }

        public Visibility OrthogonalViewVisibility
        {
            get => _orthogonalViewVisibility;
            set => SetProperty(ref _orthogonalViewVisibility, value);
        }

        public double OrthogonalViewZMultiplier
        {
            get
            {
                return _imageViewM.OrthogonalViewZMultiplier;
            }
            set
            {
                if (_imageViewM.OrthogonalViewZMultiplier > 0)
                {
                    _imageViewM.OrthogonalViewZMultiplier = value;
                }
            }
        }

        public int PixelBitShiftValue
        {
            get
            {
                return _imageViewM.PixelBitShiftValue;
            }
        }

        //TODO: think how to display pmt saturations, the property change, eliminate if not needed
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

        public int ProgressPercentage
        {
            get
            { return _progressPercentage; }
            set
            { _progressPercentage = value; }
        }

        public string ReferenceChannelImageName
        {
            get
            {
                return _imageViewM.ReferenceChannelImageName;
            }
        }

        public string ReferenceChannelImagePath
        {
            get
            {
                return _imageViewM.ReferenceChannelImagePath;
            }
        }

        public bool ResetBitmap
        {
            set
            {
                _imageViewM.ResetBitmap();
                //_imageViewM.PixelDataHistograms.Clear();
            }
        }

        public XmlDocument ReviewApplicationSettings
        {
            get;
            set;
        }

        public string ReviewApplicationSettingsPath
        {
            get;
            set;
        }

        public XmlDocument ReviewHardwareSettings
        {
            get
            {
                return _reviewHardwareSettings;
            }
            set
            {
                _reviewHardwareSettings = value;
                LoadColorImageSettings();
            }
        }

        public string ReviewHardwareSettingsPath
        {
            get;
            set;
        }

        public ICommand ROICalculationCommand
        {
            get => (ICommand)MVMManager.Instance[_mainViewModel, "ROICalculation", (object)new RelayCommand(() => { })];
        }

        public int ROIDrawingToolsSelectedIndex
        {
            get => _roiDrawingToolsSelectedIndex;
            set
            {
                _roiDrawingToolsSelectedIndex = value;
                OnPropertyChanged("ROIDrawingToolsSelectedIndex");
            }
        }

        public ICommand ROILoadCommand
        {
            get => (ICommand)MVMManager.Instance[_mainViewModel, "ROILoad", (object)new RelayCommand(() => { })];
        }

        public Thickness ROItoolbarMargin
        {
            get
            {
                return _roiToolbarMargin;
            }
            set
            {
                _roiToolbarMargin = value;
                OnPropertyChanged("ROItoolbarMargin");
            }
        }

        /// <summary>
        /// Binding items: select[0], Line[1], Rectangle[2], Ellopse[3], Polygon[4], Polyline[5], Crosshair[6], Reticle[7], Scale[8], DeleteSingle[9], ClearAll[10], Load[11], Save[12], ChooseStats[13]
        /// </summary>
        public bool[] ROIToolVisible
        {
            get
            { return _roiToolVisible; }
            set
            {
                _roiToolVisible = value;
                OnPropertyChanged("ROIToolVisible");
            }
        }

        public int RollOverPointX
        {
            get
            {
                return _imageViewM.RollOverPointX;
            }
            set
            {
                _imageViewM.RollOverPointX = value;
                OnPropertyChanged("RollOverPointX");
                for (int i = 0; i < ChannelGroups.Count; ++i)
                {
                    foreach (ChannelViewModel chan in ChannelGroups[i].Channels)
                    {
                        chan.RollOverPositionChanged = true;
                    }
                }
            }
        }

        public int RollOverPointY
        {
            get
            {
                return _imageViewM.RollOverPointY;
            }
            set
            {
                _imageViewM.RollOverPointY = value;
                OnPropertyChanged("RollOverPointY");
                for (int i = 0; i < ChannelGroups.Count; ++i)
                {
                    foreach (ChannelViewModel chan in ChannelGroups[i].Channels)
                    {
                        chan.RollOverPositionChanged = true;
                    }
                }
            }
        }

        public ICommand SaveNowCommand
        {
            get => _saveNowCommand ?? (_saveNowCommand = new RelayCommand(() => SaveNow()));
        }

        public string SaveNowImagePath
        {
            get; set;
        }

        public string SequentialExperimentPath
        {
            get
            {
                return _sequentialExperimentPath;
            }
            set
            {
                _sequentialExperimentPath = value;
                EnableHistogramUpdate = false;
                // must be done on application thread
                Application.Current.Dispatcher.Invoke(new Action(() => LoadSequentialChannelsFromXML(_sequentialExperimentPath)));
            }
        }

        public int SPChannelColumn
        {
            get
            {
                return _sPChannelColumn;
            }
            set
            {
                _sPChannelColumn = value;
                OnPropertyChanged("SPChannelColumn");
            }
        }

        public int SPROIToolsColumn
        {
            get
            {
                return _sPROIToolsColumn;
            }
            set
            {
                _sPROIToolsColumn = value;
                OnPropertyChanged("SPROIToolsColumn");
            }
        }

        public int SPToolsColumn
        {
            get
            {
                return _sPToolsColumn;
            }
            set
            {
                _sPToolsColumn = value;
                OnPropertyChanged("SPToolsColumn");
            }
        }

        public Thickness SPToolsMargin
        {
            get
            {
                return _sPToolsMargin;
            }
            set
            {
                _sPToolsMargin = value;
                OnPropertyChanged("SPToolsMargin");
            }
        }

        public bool StartBitmapBuildingThread
        {
            set
            {
                _imageViewM.StartBitmapBuildingThread();
            }
        }

        public bool StopBitmapBuildingThread
        {
            set
            {
                _imageViewM.StopBitmapBuildingThread();
            }
        }

        public bool TileDisplay
        {
            get
            {
                return _imageViewM.TileDisplay;
            }
            set
            {
                if (_imageViewM.TileDisplay != value)
                {
                    _imageViewM.TileDisplay = value;
                    OnPropertyChanged("TileDisplay");
                }
            }
        }

        public int TIndexFromReview
        {
            get
            {
                return _tIndexFromReview;
            }
            set
            {
                if (value != _tIndexFromReview)
                {
                    _tIndexFromReview = value;
                    UpdateOrthogonalView(true, _tIndexFromReview);
                }
            }
        }

        public int TotalDisplayedChannels
        {
            get
            {
                return _imageViewM.TotalDisplayedChannels;
            }
            set
            {
                //Only update when we pass the 6 channels threshold
                if (_imageViewM.TotalDisplayedChannels <= 6 && value > 6)
                {
                    SPChannelColumn = 2;
                    SPROIToolsColumn = 3;
                    SPToolsColumn = 4;
                    SPToolsMargin = new Thickness(5, 5, 5, 5);
                    foreach (ChannelGroupViewModel channelGroup in ChannelGroups)
                    {
                        channelGroup.ChannelsStackOrientation = "Horizontal";
                    }
                }
                else if (value <= 6)
                {
                    SPChannelColumn = SPToolsColumn = 3;
                    SPROIToolsColumn = 2;
                    SPToolsMargin = new Thickness(5, value * CHANNEL_BUTTON_SIZE, 5, 5);
                }
                _imageViewM.TotalDisplayedChannels = value;
            }
        }

        public ICommand UpdateOrthogonalViewCommand
        {
            get => _updateOrthogonalViewCommand ?? (_updateOrthogonalViewCommand = new RelayCommand(() => UpdateOrthogonalView()));
        }

        public bool VerticalTileDisplay
        {
            get
            {
                return _imageViewM.VerticalTileDisplay;
            }
            set
            {
                _imageViewM.VerticalTileDisplay = value;
            }
        }

        public double VerticalToolbarOffset
        {
            get
            {
                return _verticalToolbarOffset;
            }

            set
            {
                _verticalToolbarOffset = value;
                ROItoolbarMargin = new Thickness(0, _verticalToolbarOffset, 0, 0);
                HelpToolbarMargin = new Thickness(-1, _verticalToolbarOffset, 0, 0);
            }
        }

        public bool VirtualZStack
        {
            get
            {
                return _imageViewM.VirtualZStack;
            }
            set
            {
                _imageViewM.VirtualZStack = value;
            }
        }

        //Send the current WP and BP for the 3D volume view. This will be called right before the 3D volume is rendered
        public List<List<int>> VolumeViewHistogramBPWP
        {
            get
            {
                List<int> histogramBPList = new List<int>();
                List<int> histogramWPList = new List<int>();
                for (int i = 0; i < HistogramViewModels.Count; i++)
                {
                    for (int j = 0; j < HistogramViewModels[i].Count; j++)
                    {
                        histogramBPList.Add((int)HistogramViewModels[i][j].ThresholdBP);
                        histogramWPList.Add((int)HistogramViewModels[i][j].ThresholdWP);
                    }
                }
                List<List<int>> histogramBPWP = new List<List<int>>
                {
                    histogramBPList,
                    histogramWPList
                };
                return histogramBPWP;
            }
        }

        //Sned the current channel selection for the 3D volume view. This will be called right before the 3D volume is rendered
        public int VolumeViewVisibleChannels
        {
            get
            {
                int val = 0;
                foreach (ChannelGroupViewModel channelGroup in ChannelGroups)
                {
                    if (MAX_CHANNELS == channelGroup.Channels.Count)
                    {
                        for (int i = 0; i < MAX_CHANNELS; i++)
                        {
                            if (channelGroup.Channels[i].ChannelVisibility == Visibility.Visible)
                            {
                                val |= (0x0001 << i);
                            }
                        }
                    }
                }
                return val;
            }
        }

        public string[] WavelengthNames
        {
            get
            {
                if (ChannelGroups[0].Channels.Count > 0 && ChannelGroups[0].Channels[0].ChannelDisplayEnable)
                {
                    _wavelengthNames[0] = "ChanA";
                }
                if (ChannelGroups[0].Channels.Count > 0 && ChannelGroups[0].Channels[1].ChannelDisplayEnable)
                {
                    _wavelengthNames[1] = "ChanB";
                }
                if (ChannelGroups[0].Channels.Count > 0 && ChannelGroups[0].Channels[2].ChannelDisplayEnable)
                {
                    _wavelengthNames[2] = "ChanC";
                }
                if (ChannelGroups[0].Channels.Count > 0 && ChannelGroups[0].Channels[3].ChannelDisplayEnable)
                {
                    _wavelengthNames[3] = "ChanD";
                }
                return _wavelengthNames;
            }
        }

        public int WhitePointMaxVal
        {
            get => _imageViewM.WhitePointMaxVal;
            set => _imageViewM.WhitePointMaxVal = value;
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
                    UpdateROILineWidths(value);
                }

                OnPropertyChanged("ZoomLevel");
            }
        }

        public virtual int ZStepNum
        {
            get => _zStepNum;
            set => SetProperty(ref _zStepNum, value);
        }

        public virtual double ZStepSizeUM
        {
            get;
            set;
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : null;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    myPropInfo.SetValue(this, value);
                }
            }
        }

        public object this[string propertyName, int index, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        return collection.GetType().GetProperty("Item").GetValue(collection, new object[] { index });
                    }
                    else
                    {
                        return myPropInfo.GetValue(this, null);
                    }
                }
                return defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        collection.GetType().GetProperty("Item").SetValue(collection, value, new object[] { index });
                    }
                    else
                    {
                        myPropInfo.SetValue(this, value, null);
                    }
                }
            }
        }

        #endregion Indexers

        #region Methods

        public void BlackPointThresholdUpdate(ImageIdentifier imageIdentifier, double blackPointThreshold)
        {
            _imageViewM.SetBlackPoint(imageIdentifier, blackPointThreshold);

            if (_imageViewM.OrthogonalViewEnabled && VirtualZStack)
            {
                //TODO: these xz yz bitmaps should also be created in separate thread
                UpdateOrthogonalViewImages();
            }
        }

        public void BrowseForReferenceImage()
        {
            if (_imageViewM.BrowseForReferenceImage())
            {
                OnPropertyChange("ReferenceChannelImageName");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CameraControlViewModel"]).OnPropertyChange("ReferenceChannelImageName");
                ((ThorSharedTypes.IMVM)MVMManager.Instance["AreaControlViewModel"]).OnPropertyChange("ReferenceChannelImageName");
            }
        }

        public void BuildHistograms()
        {
            if (null == HistogramViewModels)
            {
                return;
            }

            lock (_imageViewM.UpdateHistogramsLock)
            {
                // cleanup any existing histogram view models
                for (int i = 0; i < HistogramViewModels.Count; i++)
                {
                    for (int j = 0; j < HistogramViewModels[i].Count; j++)
                    {
                        var histogramViewModel = HistogramViewModels[i][j];

                        // save the current histogram channel settings
                        foreach (int dataChannel in histogramViewModel.GetAllDataChannels())
                        {
                            var imageIdentifier = histogramViewModel.GetImageIdentifier(dataChannel);
                            string imageIdentifierAsKey = imageIdentifier.AsKeyString();
                            var histogramChannelSettings = histogramViewModel.GetChannelSettings(dataChannel);
                            int matchingIndex = _histogramSettings.FindIndex(match => match.ImageIdentifierKey == imageIdentifierAsKey);
                            if (-1 == matchingIndex)
                            {
                                _histogramSettings.Add(histogramChannelSettings);
                            }
                            else
                            {
                                _histogramSettings[matchingIndex] = histogramChannelSettings;
                            }
                        }

                        // detach signals from outdated histogram view models grid
                        histogramViewModel.WhitePointThresholdUpdated -= WhitePointThresholdUpdate;
                        histogramViewModel.BlackPointThresholdUpdated -= BlackPointThresholdUpdate;
                        histogramViewModel.GammaUpdated -= GammaUpdate;
                        histogramViewModel.AutoClicked -= HistogramAuto;
                        histogramViewModel.ContinuousAutoChanged -= HistogramContinuousAuto;
                        histogramViewModel.ExpandAllHistograms -= ExpandAllHistograms;
                        histogramViewModel.HistogramFittingUpdated -= HistogramFittingUpdate;
                        histogramViewModel.WhitePointIncreased -= WhitePointIncrease;
                    }
                }

                // clear the grid
                HistogramViewModels.Clear();

                bool[] lsmChannelEnabled;

                if (_imageViewM.ImageViewType == ImageViewType.Review)
                {
                    int lsmChannelFromReview = (int)MVMManager.Instance["ImageReviewViewModel", "LSMChannel"];
                    lsmChannelEnabled = new bool[MAX_CHANNELS];
                    for (int c = 0; c < MAX_CHANNELS; c++)
                    {
                        lsmChannelEnabled[c] = (lsmChannelFromReview & (0x0001 << c)) > 0;
                    }
                }
                else
                {
                    lsmChannelEnabled = (bool[])MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable"];
                }
                List<int> enabledChannelsIndex = new List<int>();
                for (int c = 0; c < MAX_CHANNELS; c++)
                {
                    if (lsmChannelEnabled[c])
                    {
                        enabledChannelsIndex.Add(c);
                    }
                }
                int numPlanes = 1;
                int vImages = 1;
                // create a new grid of histogram view models
                try
                {
                    numPlanes = _imageViewM.ImageViewType == ImageViewType.Review ? (int)MVMManager.Instance["ImageReviewViewModel", "LSMNumberOfPlanes"] : (int)MVMManager.Instance["ThreePhotonControlViewModel", "LSMNumberOfPlanes"];

                }
                catch (System.NullReferenceException)
                {

                }
                try
                {
                    vImages = _imageViewM.ImageViewType == ImageViewType.Review ? (int)MVMManager.Instance["ImageReviewViewModel", "TotalScanAreas"] : (int)MVMManager.Instance["AreaControlViewModel", "TotalScanAreas"];
                }
                catch (System.NullReferenceException)
                {

                }
                int numChannels = MAX_CHANNELS;

                if (null != FrameData)
                {
                    numPlanes = FrameData.frameInfo.numberOfPlanes > 1 ? FrameData.frameInfo.numberOfPlanes : 1;
                    vImages = FrameData.frameInfo.totalScanAreas > 1 ? FrameData.frameInfo.totalScanAreas : numPlanes;
                    numChannels = Math.Min(FrameData.frameInfo.channels, MAX_CHANNELS); // TODO: test case where numChannels > 4
                }

                if (IsInSequentialMode)
                {
                    vImages = ChannelsPerSequence.Count;
                }
                bool isVertical = vImages == 1; // for non-multiplane images, keep the histograms vertical like in previous versions

                // signal attacher action
                var AttachSignalsAction = new Action<HistogramControlViewModel>(histogramControlViewModel =>
                {
                    histogramControlViewModel.WhitePointThresholdUpdated += WhitePointThresholdUpdate;
                    histogramControlViewModel.BlackPointThresholdUpdated += BlackPointThresholdUpdate;
                    histogramControlViewModel.GammaUpdated += GammaUpdate;
                    histogramControlViewModel.AutoClicked += HistogramAuto;
                    histogramControlViewModel.ContinuousAutoChanged += HistogramContinuousAuto;
                    histogramControlViewModel.ExpandAllHistograms += ExpandAllHistograms;
                    histogramControlViewModel.HistogramFittingUpdated += HistogramFittingUpdate;
                    histogramControlViewModel.WhitePointIncreased += WhitePointIncrease;
                });

                if (_isMergedHistogram)
                {
                    // all image channels are merged together: one row for each plane with one view model each
                    for (int p = 0; p < vImages; p++)
                    {
                        var histogramRow = new ObservableCollection<HistogramControlViewModel>();
                        var histogramControlViewModel = new HistogramControlViewModel();
                        AttachSignalsAction(histogramControlViewModel);
                        histogramRow.Add(histogramControlViewModel);

                        if (IsInSequentialMode && ChannelsPerSequence.Count > 0)
                        {
                            numChannels = ChannelsPerSequence[p].Item1;
                            enabledChannelsIndex.Clear();
                            for (int ch = 0; ch < MAX_CHANNELS; ch++)
                            {
                                if ((ChannelsPerSequence[p].Item2 & (0x0001 << ch)) > 0)
                                {
                                    enabledChannelsIndex.Add(ch);
                                }
                            }
                        }

                        // TODO: include plane for multiplane
                        histogramControlViewModel.ChannelName = "Merged"; 

                        for (int c = 0; c < numChannels; c++)
                        {
                            int chanIndex = (numChannels == enabledChannelsIndex.Count) ? enabledChannelsIndex[c] : c;
                            var imageIdentifier = new ImageIdentifier(chanIndex, p);
                            var imageIdKey = imageIdentifier.AsKeyString();
                            int chanEnabled = (IsInSequentialMode) ? c : chanIndex;
                            if (ChannelGroups[p].Channels.Count > 0 && chanEnabled < ChannelGroups[p].Channels.Count)
                            {
                                Color channelColor = ((SolidColorBrush)ChannelGroups[p].Channels[chanEnabled].ChannelColor).Color; //Get the color of the first SubChannel, should be the same for the rest
                                histogramControlViewModel.ActivateChannel(chanIndex, channelColor, 0.8, imageIdentifier);
                                if(numChannels == 1)
                                {
                                    histogramControlViewModel.ChannelName = ChannelGroups[p].Channels[chanEnabled].ChannelName;
                                }
                            }
                            histogramControlViewModel.IsLineChart = _isLineChartHistogram;
                            histogramControlViewModel.ShiftValue = _imageViewM.PixelBitShiftValue;
                            histogramControlViewModel.NumGridLines = _numHistogramGridLines;
                        }
                        HistogramViewModels.Add(histogramRow);
                    }
                }
                else if (isVertical)
                {
                    // transposed version of standard histogram grid: x = planes, y = channels
                    for (int c = 0; c < numChannels; c++)
                    {
                        int chanIndex = (numChannels == enabledChannelsIndex.Count) ? enabledChannelsIndex[c] : c;
                        var histogramRow = new ObservableCollection<HistogramControlViewModel>();
                        int chanEnabled = (IsInSequentialMode) ? c : chanIndex;
                        for (int p = 0; p < vImages; p++)
                        {
                            var imageIdentifier = new ImageIdentifier(chanIndex, p);
                            var imageIdKey = imageIdentifier.AsKeyString();
                            var histogramControlViewModel = new HistogramControlViewModel();
                            AttachSignalsAction(histogramControlViewModel);
                            if (ChannelGroups[p].Channels.Count > 0 && chanEnabled < ChannelGroups[p].Channels.Count)
                            {
                                Color channelColor = ((SolidColorBrush)ChannelGroups[p].Channels[chanEnabled].ChannelColor).Color; //Get the color of the first SubChannel, should be the same for the rest
                                histogramControlViewModel.ActivateChannel(chanIndex, channelColor, 0.8, imageIdentifier);
                                histogramControlViewModel.ChannelName = ChannelGroups[p].Channels[chanEnabled].ChannelName;
                            }
                            histogramControlViewModel.IsLineChart = _isLineChartHistogram;
                            histogramControlViewModel.ShiftValue = _imageViewM.PixelBitShiftValue;
                            histogramRow.Add(histogramControlViewModel);
                        }
                        HistogramViewModels.Add(histogramRow);
                    }
                }
                else
                {
                    // standard histogram grid: x = channels, y = planes
                    int plane = 0; // When not imaging in sequential mode, there will only be a single channel group so the plane should be 0
                    for (int p = 0; p < vImages; p++)
                    {
                        plane = IsInSequentialMode ? p : 0; //If in sequential, the plane is equal to the loop iterator

                        var histogramRow = new ObservableCollection<HistogramControlViewModel>();

                        if (IsInSequentialMode && ChannelsPerSequence.Count > 0)
                        {
                            numChannels = ChannelsPerSequence[plane].Item1;
                            enabledChannelsIndex.Clear();
                            for (int ch = 0; ch < MAX_CHANNELS; ch++)
                            {
                                if ((ChannelsPerSequence[plane].Item2 & (0x0001 << ch)) > 0)
                                {
                                    enabledChannelsIndex.Add(ch);
                                }
                            }
                        }
                        for (int c = 0; c < numChannels; c++)
                        {
                            int chanIndex = (numChannels == enabledChannelsIndex.Count) ? enabledChannelsIndex[c] : c;
                            var imageIdentifier = new ImageIdentifier(chanIndex, p);// mROI treats the incoming scan areas as seperate planes so this stays as p
                            var imageIdKey = imageIdentifier.AsKeyString();
                            var histogramControlViewModel = new HistogramControlViewModel();
                            int chanEnabled = (IsInSequentialMode) ? c : chanIndex;
                            AttachSignalsAction(histogramControlViewModel);

                            if (ChannelGroups[plane].Channels.Count > 0)
                            {
                                Color channelColor = ((SolidColorBrush)ChannelGroups[plane].Channels[chanEnabled].ChannelColor).Color; //Get the color of the first SubChannel, should be the same for the rest
                                histogramControlViewModel.ActivateChannel(chanIndex, channelColor, 0.8, imageIdentifier);
                                histogramControlViewModel.ChannelName = ChannelGroups[plane].Channels[chanEnabled].ChannelName;
                            }

                            histogramControlViewModel.IsLineChart = _isLineChartHistogram;
                            histogramControlViewModel.ShiftValue = _imageViewM.PixelBitShiftValue;
                            histogramRow.Add(histogramControlViewModel);
                        }
                        HistogramViewModels.Add(histogramRow);
                    }
                }

                // initialize all the new histogram view models
                for (int i = 0; i < HistogramViewModels.Count; i++)
                {
                    for (int j = 0; j < HistogramViewModels[i].Count; j++)
                    {
                        var histogramViewModel = HistogramViewModels[i][j];

                        // apply any saved settings
                        foreach (int dataChannel in histogramViewModel.GetAllDataChannels())
                        {
                            var imageIdentifier = histogramViewModel.GetImageIdentifier(dataChannel);
                            string imageIdentifierAsKey = imageIdentifier.AsKeyString();

                            var matchingSettings = _histogramSettings.Find(match => match.ImageIdentifierKey == imageIdentifierAsKey);
                            if (null != matchingSettings)
                            {
                                histogramViewModel.ApplyChannelSettings(matchingSettings);
                                break; // there could be multiple settings that apply to the same view model; just take the first one
                            }
                        }
                    }
                }
            }
        }

        public void FireColorMappingChangedAction(bool bVal)
        {
            ColorMappingChanged?.Invoke(bVal);
        }

        public void GammaUpdate(ImageIdentifier imageIdentifier, double gamma)
        {
            _imageViewM.SetGamma(imageIdentifier, gamma);

            if (_imageViewM.OrthogonalViewEnabled && VirtualZStack)
            {
                //TODO: these xz yz bitmaps should also be created in separate thread
                UpdateOrthogonalViewImages();
            }
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            if (!_properties.TryGetValue(propertyName, out PropertyInfo myPropInfo))
            {
                myPropInfo = typeof(ImageViewVVMBase).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void HandleViewLoaded()
        {
            //don't again until unloaded
            if (_loaded)
            {
                return;
            }

            //Load ROI's for the new view
            //If in review, ROI's should be loaded from an active experiment instead of from templates
            ROIDrawingToolsSelectedIndex = 0;

            LoadHistogramSettings();
            BuildHistograms();
            BuildChannelPalettes();
            LoadColorImageSettings();
            LoadChannelVisibility();

            //make sure there is only one registration per event
            _imageViewM.BitmapUpdated -= ImageViewM_BitmapUpdated;
            _imageViewM.BitmapUpdateRateUpdated -= ImageViewM_BitmapUpdateRateUpdated;
            _imageViewM.PixelDataUpdated -= _imageViewM_PixelDataUpdated;
            _imageViewM.PixelBitShiftValueChanged -= _imageViewM_PixelBitShiftValueChanged;
            _imageViewM.ReferenceChannelInvalidated -= ImageViewM_ReferenceChannelInvalidated;
            _imageViewM.ZoomChanged -= ImageViewM_ZoomAfterImageUpdate;
            OverlayManagerClass.Instance.ROIListUpdated -= Instance_ROIListUpdated;
            OverlayManagerClass.Instance.NeedOverlayItemsUpdateToActiveROIs -= Instance_NeedOverlayItemsUpdateToActiveROIs;

            _imageViewM.BitmapUpdated += ImageViewM_BitmapUpdated;
            _imageViewM.BitmapUpdateRateUpdated += ImageViewM_BitmapUpdateRateUpdated;
            _imageViewM.PixelDataUpdated += _imageViewM_PixelDataUpdated;
            _imageViewM.PixelBitShiftValueChanged += _imageViewM_PixelBitShiftValueChanged;
            _imageViewM.ReferenceChannelInvalidated += ImageViewM_ReferenceChannelInvalidated;
            _imageViewM.ZoomChanged += ImageViewM_ZoomAfterImageUpdate;
            OverlayManagerClass.Instance.ROIListUpdated += Instance_ROIListUpdated;
            OverlayManagerClass.Instance.NeedOverlayItemsUpdateToActiveROIs += Instance_NeedOverlayItemsUpdateToActiveROIs;

            _imageViewM.StartCopyDataThread();
            _imageViewM.StartBitmapBuildingThread();

            //Update ROI zoom levels on load. Otherwise, the zoom levels for the previous view will be used
            UpdateROILineWidths(ZoomLevel);

            _loaded = true;
        }

        public void HandleViewUnloaded()
        {
            _imageViewM.BitmapUpdated -= ImageViewM_BitmapUpdated;
            _imageViewM.BitmapUpdateRateUpdated -= ImageViewM_BitmapUpdateRateUpdated;
            _imageViewM.PixelDataUpdated -= _imageViewM_PixelDataUpdated;
            _imageViewM.PixelBitShiftValueChanged -= _imageViewM_PixelBitShiftValueChanged;
            _imageViewM.ReferenceChannelInvalidated -= ImageViewM_ReferenceChannelInvalidated;
            _imageViewM.ZoomChanged -= ImageViewM_ZoomAfterImageUpdate;
            OverlayManagerClass.Instance.ROIListUpdated -= Instance_ROIListUpdated;
            OverlayManagerClass.Instance.NeedOverlayItemsUpdateToActiveROIs -= Instance_NeedOverlayItemsUpdateToActiveROIs;

            _imageViewM.StopBitmapBuildingThread();
            _imageViewM.StopCopyDataThread();
            for (int i = 0; i < HistogramViewModels.Count; i++)
            {
                for (int j = 0; j < HistogramViewModels[i].Count; j++)
                {
                    HistogramViewModels[i][j].WhitePointThresholdUpdated -= WhitePointThresholdUpdate;
                    HistogramViewModels[i][j].BlackPointThresholdUpdated -= BlackPointThresholdUpdate;
                    HistogramViewModels[i][j].GammaUpdated -= GammaUpdate;
                    HistogramViewModels[i][j].AutoClicked -= HistogramAuto;
                    HistogramViewModels[i][j].ContinuousAutoChanged -= HistogramContinuousAuto;
                    HistogramViewModels[i][j].ExpandAllHistograms -= ExpandAllHistograms;
                    HistogramViewModels[i][j].HistogramFittingUpdated -= HistogramFittingUpdate;
                    HistogramViewModels[i][j].WhitePointIncreased -= WhitePointIncrease;
                }
            }

            //Disable Reference channel whenever changing modalities
            MVMManager.Instance["AreaControlViewModel", "EnableReferenceChannel"] = 0;
            MVMManager.Instance["CameraControlViewModel", "EnableReferenceChannel"] = false;

            _loaded = false;
        }

        public void HistogramAuto(ImageIdentifier imageIdentifier, bool isAutoPressed)
        {
            _imageViewM.SetIsAutoButtonPressed(imageIdentifier, isAutoPressed);

            _imageViewM.AutoEnhanceIfEnabled();
        }

        public void HistogramContinuousAuto(ImageIdentifier imageIdentifier, bool isContinuousAuto)
        {
            _imageViewM.SetIsContinuousChecked(imageIdentifier, isContinuousAuto);

            _imageViewM.AutoEnhanceIfEnabled();
        }

        public void HistogramFittingUpdate(ImageIdentifier imageIdentifier, double topPercentileReduction, double bottomPercentileReduction)
        {
            _imageViewM.SetAutoFittingPercentiles(imageIdentifier.Channel, topPercentileReduction, bottomPercentileReduction);
        }

        public void InitOrthogonalView()
        {
            UpdateOrthogonalView();
        }

        public void LoadChannelVisibility()
        {
            try
            {
                var experimentDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
                XmlNodeList ChannelEnable = experimentDoc.GetElementsByTagName("ChannelEnable");
                if (ChannelEnable.Count > 0)
                {
                    string ChanEnableSet = string.Empty;
                    XmlManager.GetAttribute(ChannelEnable[0], experimentDoc, "Set", ref ChanEnableSet);
                    //iterate through binary string to check channels:
                    int tmp = 0;
                    if (Int32.TryParse(ChanEnableSet, out tmp))
                    {
                        string binaryString = Convert.ToString(tmp, 2).PadLeft(MAX_CHANNELS, '0');
                        int numberOfchannels = 0;

                        foreach (ChannelGroupViewModel channelGroup in ChannelGroups)
                        {
                            if (channelGroup.Channels.Count > 0)
                            {
                                for (int i = 0; i < MAX_CHANNELS; i++)
                                {
                                    if (binaryString[MAX_CHANNELS - 1 - i] == '1')
                                    {
                                        channelGroup.Channels[i].ChannelVisibility = Visibility.Visible;
                                        numberOfchannels++;
                                    }
                                }
                            }
                        }
                        SPToolsMargin = IsInSequentialMode ? SPToolsMargin : new Thickness(5, numberOfchannels * CHANNEL_BUTTON_SIZE, 5, 5);
                    }
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + ex.Message);
            }
        }

        public void LoadColorImageSettings()
        {
            string str = string.Empty;
            var hwDoc = (null != ReviewHardwareSettings && _mainViewModel == "ImageReviewViewModel") ? ReviewHardwareSettings : MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndList = hwDoc.SelectNodes("/HardwareSettings/ColorChannels/*");
            int sequenceIndex = 0;

            if (null == _allLuts || null == _lutNames)
            {
                return;
            }

            foreach (ChannelGroupViewModel channelGroup in ChannelGroups)
            {
                for (int i = 0; i < channelGroup.Channels.Count; i++)
                {
                    if (IsInSequentialMode)
                    {
                        channelGroup.Channels[i].LutNames = _lutNames;
                        channelGroup.Channels[i].LUTColors = _allLuts;
                        channelGroup.Channels[i].SelectedColorUI = _lutNames.IndexOf(channelGroup.Channels[i].ChannelColorString);
                        channelGroup.Channels[i].ChannelColor = new SolidColorBrush(channelGroup.Channels[i].GetChannelColor());
                        _imageViewM.ChannelLuts[sequenceIndex][channelGroup.Channels[i].Index] = _allLuts[channelGroup.Channels[i].SelectedColorUI].ToArray();
                    }
                    else
                    {
                        string chanName = channelGroup.Channels[i].ChannelName;

                        channelGroup.Channels[i].ColorPalettesUI = _colorPalettesUI;

                        for (int k = 0; k < ndList.Count; k++)
                        {
                            if (XmlManager.GetAttribute(ndList[k], hwDoc, "name", ref str))
                            {
                                if (str.Contains(chanName))
                                {
                                    channelGroup.Channels[i].LutNames = _lutNames;
                                    channelGroup.Channels[i].LUTColors = _allLuts;
                                    channelGroup.Channels[i].ChannelColorString = ndList[k].Name;
                                    channelGroup.Channels[i].SelectedColorUI = _lutNames.IndexOf(channelGroup.Channels[i].ChannelColorString);
                                    channelGroup.Channels[i].ChannelColor = new SolidColorBrush(channelGroup.Channels[i].GetChannelColor());
                                    _imageViewM.ChannelLuts[0][channelGroup.Channels[i].Index] = _allLuts[channelGroup.Channels[i].SelectedColorUI].ToArray();
                                }
                            }
                        }
                        _defaultChannelColorNames[channelGroup.Channels[i].Index] = channelGroup.Channels[i].ChannelColorString;
                        _defaultChannelColors[channelGroup.Channels[i].Index] = channelGroup.Channels[i].GetChannelColor();
                    }
                }
                sequenceIndex++;
            }

            // set histogram channel's color based on the corresponding image channel
            for (int row = 0; row < _histogramViewModelsGrid.Count; row++)
            {
                for (int col = 0; col < _histogramViewModelsGrid[row].Count; col++)
                {
                    var histogramViewModel = _histogramViewModelsGrid[row][col];
                    foreach (int histChannel in histogramViewModel.GetAllDataChannels())
                    {
                        var imageIdentifier = histogramViewModel.GetImageIdentifier(histChannel);
                        int plane = IsInSequentialMode ? imageIdentifier.Plane : 0; // Only sequential will have more than 1 color group
                        if (imageIdentifier.Channel < ChannelGroups.Count && ChannelGroups[plane].Channels.Count > 0)
                        {
                            int chan = IsInSequentialMode ? col : imageIdentifier.Channel;
                            SolidColorBrush brush = (SolidColorBrush)ChannelGroups[plane].Channels[chan].ChannelColor;
                            histogramViewModel.SetDataBrushColor(histChannel, brush.Color);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Grabs a list of settings from the active experiment and maps them to image identifiers.
        /// Settings are used later when a histogram view model (for a given image identifier) is created for the first time.
        /// </summary>
        public void LoadHistogramSettings()
        {
            _histogramSettings.Clear();
            bool isMutexAcquired = ResourceManagerCS.BorrowDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS, (int)Constants.TIMEOUT_MS);
            if (!isMutexAcquired)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, GetType().Name + "Could not acquire active experiment settings mutex\n");
                return;
            }
            try
            {
                _ = MVMManager.Instance.ReloadSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                var activeExperimentDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
                XmlNode histogramsNode = activeExperimentDoc.SelectSingleNode("/ThorImageExperiment/Histograms");
                if (null == histogramsNode)
                {
                    return;
                }

                XmlNodeList histogramNodes = histogramsNode.ChildNodes;
                foreach (XmlNode node in histogramsNode.ChildNodes)
                {
                    var settings = HistogramChannelSettings.MakeFromXmlNode(ref activeExperimentDoc, node);
                    _histogramSettings.Add(settings);
                }
            }
            finally
            {
                _ = ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
            }
        }

        public bool LoadSequentialChannelsFromXML(string experimentPath)
        {
            if (Directory.Exists(System.IO.Path.GetDirectoryName(experimentPath)))
            {
                XmlDocument doc = new XmlDocument();
                doc.Load(experimentPath);

                XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/CaptureSequence");
                string str = string.Empty;
                int iTmp;
                int sequenceStepIndex = 0;

                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], doc, "enable", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                    {
                        if (1 == iTmp)
                        {
                            ndList = doc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
                            foreach (ChannelGroupViewModel channelGroup in ChannelGroups)
                            {
                                channelGroup.Channels.Clear();
                                TotalDisplayedChannels = 0;
                            }
                            ChannelsPerSequence.Clear();
                            ChannelGroups.Clear();
                            _imageViewM.ChannelLuts.Clear();
                            _imageViewM.ChannelDisplayEnable.Clear();

                            foreach (XmlNode sequenceStep in ndList)
                            {
                                int channelsEnabled;
                                ChannelGroups.Add(new ChannelGroupViewModel() { ChannelGroupName = "Sequence Step: " + (sequenceStepIndex + 1) });
                                _imageViewM.ChannelLuts.Add(new Color[MAX_CHANNELS][]);
                                _imageViewM.ChannelDisplayEnable.Add(new ObservableCollection<bool>());
                                XmlNodeList wavelengthsList = sequenceStep.SelectNodes("Wavelengths/Wavelength");
                                XmlNode camNode = (ResourceManagerCS.GetCameraType() == (int)ICamera.CameraType.LSM) ? sequenceStep.SelectSingleNode("LSM") : sequenceStep.SelectSingleNode("Camera");
                                if (null != camNode)
                                {
                                    if (XmlManager.GetAttribute(camNode, doc, "channel", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out channelsEnabled))
                                    {
                                        int count = 0;
                                        for (int i = 0; i < MAX_CHANNELS; i++)
                                        {
                                            _imageViewM.ChannelLuts[sequenceStepIndex][i] = new Color[LUT_SIZE];
                                            _imageViewM.ChannelDisplayEnable[sequenceStepIndex].Add(true);
                                            if ((channelsEnabled & (0x0001 << i)) > 0)
                                            {
                                                ChannelViewModel newChannel = new ChannelViewModel();

                                                XmlManager.GetAttribute(wavelengthsList[count], doc, "name", ref str);
                                                newChannel.ChannelName = str;
                                                if (XmlManager.GetAttribute(wavelengthsList[count], doc, "color", ref str))
                                                {
                                                    Color chanColor = (Color)ColorConverter.ConvertFromString(str);
                                                    newChannel.ChannelColor = new SolidColorBrush(chanColor);
                                                }
                                                if (XmlManager.GetAttribute(wavelengthsList[count], doc, "colorName", ref str))
                                                {
                                                    newChannel.ChannelColorString = str;
                                                }
                                                newChannel.Index = i;
                                                newChannel.GroupIndex = sequenceStepIndex;
                                                newChannel.ChannelVisibility = Visibility.Visible;

                                                newChannel.ColorPalettesUI = _colorPalettesUI;

                                                newChannel.LutNames = _lutNames;
                                                newChannel.LUTColors = _allLuts;
                                                newChannel.SelectedColorUI = _lutNames.IndexOf(newChannel.ChannelColorString);
                                                newChannel.ChannelDisplayEnabledChanged += ImageViewVVMBase_ChannelDisplayEnabledChanged;
                                                newChannel.GetPixelInfoDelegate = _imageViewM.GetPixelInfoDelegate;
                                                ChannelGroups[sequenceStepIndex].Channels.Add(newChannel);
                                                count++;
                                                _imageViewM.ChannelLuts[sequenceStepIndex][i] = _allLuts[newChannel.SelectedColorUI].ToArray();
                                            }
                                        }
                                        ChannelsPerSequence.Add(new Tuple<int, int>(count, channelsEnabled));
                                        TotalDisplayedChannels += count;
                                        ChannelGroups[sequenceStepIndex].ChannelsStackOrientation = TotalDisplayedChannels > 6 ? "Horizontal" : "Vertical";
                                    }
                                }
                                sequenceStepIndex++;
                            }
                        }
                    }
                }
                BuildHistograms();
            }
            return false;
        }

        //TODO: Add tag for framerate visibility for debugging
        //TODO: this function should move from here when the channel selection is divoced from the channel View
        public void LoadXMLSettings()
        {
            //Load and adjust visible items from application settings
            //MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
            var applicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            _appDocPath = applicationDoc.BaseURI;
            XmlNodeList ndList = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ImageViewZoom");

            if (0 < ndList.Count)
            {
                if (double.TryParse(ndList[0].Attributes["offsetX"].Value, out double zox))
                {
                    ImageOffsetX = zox;
                }

                if (double.TryParse(ndList[0].Attributes["offsetY"].Value, out double zoy))
                {
                    ImageOffsetY = zoy;
                }

                if (double.TryParse(ndList[0].Attributes["value"].Value, out double zoom))
                {
                    ZoomLevel = zoom;
                }
            }
            else
            {
                MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
                applicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNode g = applicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/General");
                XmlElement ivZoom = applicationDoc.CreateElement("ImageViewZoom");
                ivZoom.SetAttribute("value", "100.0");
                ivZoom.SetAttribute("offsetX", "0.0");
                ivZoom.SetAttribute("offsetY", "0.0");
                g.AppendChild(ivZoom);
                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }

            XmlNodeList node = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ChannelTileViewSettings");
            string str = string.Empty;
            if (0 < node.Count)
            {
                TileDisplay = XmlManager.GetAttribute(node[0], applicationDoc, "TilingEnableOption", ref str) && (str == "1" || str == bool.TrueString);
                VerticalTileDisplay = XmlManager.GetAttribute(node[0], applicationDoc, "VerticalTiling", ref str) && (str == "1" || str == bool.TrueString);
            }

            node = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/GrayscaleForSingleChannel");
            if (node.Count > 0)
            {
                str = string.Empty;

                if (XmlManager.GetAttribute(node[0], applicationDoc, "value", ref str))
                {
                    GrayscaleForSingleChannel = ("1" == str || Boolean.TrueString == str) ? true : false;
                }
            }

            node = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/OnlyShowTrueSaturation");
            if (node.Count > 0)
            {
                str = string.Empty;

                if (XmlManager.GetAttribute(node[0], applicationDoc, "value", ref str))
                {
                    OnlyShowTrueSaturation = ("1" == str || Boolean.TrueString == str) ? true : false;
                }
            }

            node = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/HistogramSettings");
            if (0 < node.Count)
            {
                if (XmlManager.GetAttribute(node[0], applicationDoc, "ReducedBinValue", ref str))
                {
                    WhitePointMaxVal = Convert.ToInt32(str);
                    if (0 >= WhitePointMaxVal || 255 < WhitePointMaxVal)
                    {
                        WhitePointMaxVal = 255;
                    }
                }

                if (XmlManager.GetAttribute(node[0], applicationDoc, "EnableMergedHistogram", ref str))
                {
                    IsSingleMergedHistogram = Convert.ToBoolean(str);
                }

                if (XmlManager.GetAttribute(node[0], applicationDoc, "EnableLineChart", ref str))
                {
                    IsLineChartHistogram = Convert.ToBoolean(str);
                }

                if (XmlManager.GetAttribute(node[0], applicationDoc, "NumberOfGridLines", ref str))
                {
                    NumHistogramGridLines = Convert.ToInt32(str);
                }
            }

            node = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/OrthogonalViewSettings");
            if (0 < node.Count)
            {
                if (XmlManager.GetAttribute(node[0], applicationDoc, "lineColorIndex", ref str))
                {
                    if (int.TryParse(str, out int temp))
                    {
                        _orthogonalLineColorType = temp;
                    }
                }

                if (XmlManager.GetAttribute(node[0], applicationDoc, "lineTypeIndex", ref str))
                {
                    if (int.TryParse(str, out int temp))
                    {
                        _orthogonalLineType = temp;
                    }
                }

                if (XmlManager.GetAttribute(node[0], applicationDoc, "zMultiplier", ref str))
                {
                    if (double.TryParse(str, out double temp))
                    {
                        OrthogonalViewZMultiplier = temp;
                    }
                }

                if (XmlManager.GetAttribute(node[0], applicationDoc, "enableOrthogonalView", ref str))
                {
                    _imageViewM.OrthogonalViewEnabled = str == "1";
                    OnPropertyChanged("IsOrthogonalViewChecked");
                }
            }

            node = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/DebugSettings");
            if (0 < node.Count)
            {
                if (XmlManager.GetAttribute(node[0], applicationDoc, "showViewportUpdateRate", ref str))
                {
                    if (bool.TryParse(str, out bool temp))
                    {
                        _imageViewM.CalculateBitmapUpdateRate = temp;
                        if (temp)
                        {
                            _bitmapUpdateRateTextVisibility = Visibility.Visible;
                        }
                        else
                        {
                            _bitmapUpdateRateTextVisibility = Visibility.Collapsed;
                        }
                        OnPropertyChanged("BitmapUpdateRateTextVisibility");
                    }
                }
            }
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        public void PersistHistogramSettings(ref XmlDocument experimentFile)
        {
            XmlNode histogramsNode = experimentFile.SelectSingleNode("/ThorImageExperiment/Histograms");
            if (null == histogramsNode)
            {
                XmlNode thorImageExperimentsNode = experimentFile.SelectSingleNode("/ThorImageExperiment");
                if (thorImageExperimentsNode == null)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, GetType().Name + "Could not locate ThorImageExperiment XML node.\n");
                    return;
                }
                histogramsNode = experimentFile.CreateElement("Histograms");
                _ = thorImageExperimentsNode.AppendChild(histogramsNode);
            }
            histogramsNode.RemoveAll();

            for (int i = 0; i < _histogramViewModelsGrid.Count; ++i)
            {
                var _histogramRow = _histogramViewModelsGrid[i];
                for (int j = 0; j < _histogramRow.Count; ++j)
                {
                    var histogramViewModel = _histogramRow[j];
                    foreach (int histChannel in histogramViewModel.GetAllDataChannels())
                    {
                        XmlNode histogramNode = experimentFile.CreateElement("Histogram");
                        var histogramChannelSettings = histogramViewModel.GetChannelSettings(histChannel);
                        histogramChannelSettings.SaveToXmlNode(ref experimentFile, ref histogramNode);
                        _ = histogramsNode.AppendChild(histogramNode);
                    }
                }
            }
        }

        public void RebuildChannels()
        {
            short firstLetter = ((short)'A');
            TotalDisplayedChannels = 0;
            ChannelGroups.Clear();
            ChannelGroups.Add(new ChannelGroupViewModel());
            _imageViewM.ChannelLuts.Clear();
            _imageViewM.ChannelLuts.Add(new Color[MAX_CHANNELS][]);
            _imageViewM.ChannelDisplayEnable.Clear();
            _imageViewM.ChannelDisplayEnable.Add(new ObservableCollection<bool>());

            for (int i = 0; i < ImageViewMBase.MAX_CHANNELS; i++)
            {
                char channelLetter = ((char)(firstLetter + i));
                ChannelGroups[0].Channels.Add(new ChannelViewModel() { ChannelName = "Chan" + channelLetter, GetPixelInfoDelegate = _imageViewM.GetPixelInfoDelegate, GroupIndex = 0, Index = i });
                ChannelGroups[0].Channels[i].ChannelDisplayEnabledChanged += ImageViewVVMBase_ChannelDisplayEnabledChanged;
                TotalDisplayedChannels++;
                _imageViewM.ChannelLuts[0][i] = new Color[LUT_SIZE];
                _imageViewM.ChannelDisplayEnable[0].Add(true);
            }
        }

        public void SaveColorSettings()
        {
            if (false == IsInSequentialMode)
            {
                XmlDocument hwDoc = (null != ReviewHardwareSettings && _mainViewModel == "ImageReviewViewModel") ? ReviewHardwareSettings : MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                for (int i = 0; i < _lutNames.Count; i++)
                {
                    XmlNodeList ndList = hwDoc.SelectNodes("/HardwareSettings/ColorChannels/" + _lutNames[i]);

                    if (ndList.Count > 0)
                    {
                        ndList[0].Attributes["name"].Value = string.Empty;
                        for (int k = 0; k < ChannelGroups.Count; k++)
                        {
                            foreach (ChannelViewModel channel in ChannelGroups[k].Channels)
                            {
                                if (i == channel.SelectedColorUI)
                                {
                                    ndList[0].Attributes["name"].Value += channel.ChannelName;
                                }
                            }
                        }
                        if (ndList[0].Attributes["name"].Value == string.Empty)
                        {
                            ndList[0].Attributes["name"].Value = "None";
                        }
                    }
                    else
                    {
                        XmlElement newElement = hwDoc.CreateElement(_lutNames[i]);
                        XmlAttribute newAttribute = hwDoc.CreateAttribute("name");
                        newAttribute.Value = string.Empty;
                        for (int k = 0; k < ChannelGroups.Count; k++)
                        {
                            foreach (ChannelViewModel channel in ChannelGroups[k].Channels)
                            {
                                if (i == channel.SelectedColorUI)
                                {
                                    newAttribute.Value += channel.ChannelName;
                                }
                            }
                        }
                        if (newAttribute.Value == string.Empty)
                        {
                            newAttribute.Value = "None";
                        }

                        newElement.Attributes.Append(newAttribute);

                        ndList = hwDoc.SelectNodes("/HardwareSettings/ColorChannels");

                        ndList[0].AppendChild(newElement);
                    }
                }
                if (null != ReviewHardwareSettings && string.Empty != ReviewHardwareSettingsPath && _mainViewModel == "ImageReviewViewModel")
                {
                    hwDoc.Save(ReviewHardwareSettingsPath);
                }
                else
                {
                    MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
                }
            }
            else
            {
                XmlDocument expDoc = new XmlDocument();

                if (SequentialExperimentPath == ResourceManagerCS.GetCaptureTemplatePathString() + "Active.xml")
                {
                    expDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
                }
                else
                {
                    expDoc.Load(SequentialExperimentPath);
                }
                XmlNodeList wavelengthNodes = expDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep/Wavelengths/Wavelength");
                string str = string.Empty;
                for (int i = 0; i < ChannelGroups.Count; i++)
                {
                    foreach (ChannelViewModel channel in ChannelGroups[i].Channels)
                    {
                        foreach (XmlNode wavelength in wavelengthNodes)
                        {
                            if (XmlManager.GetAttribute(wavelength, expDoc, "name", ref str) && str.Equals(channel.ChannelName))
                            {
                                XmlManager.SetAttribute(wavelength, expDoc, "colorName", channel.ChannelColorString);
                                Color color = channel.GetChannelColor();
                                XmlManager.SetAttribute(wavelength, expDoc, "color", color.ToString());
                                channel.ChannelColor = new SolidColorBrush(color);
                            }
                        }
                    }
                }
                if (SequentialExperimentPath == ResourceManagerCS.GetCaptureTemplatePathString() + "Active.xml")
                {
                    MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                    MVMManager.Instance["SequentialControlViewModel", "UpdateSequencesColors"] = true;
                }
                else
                {
                    expDoc.Save(SequentialExperimentPath);
                }
            }

            _colorSettingsWindow.DialogResult = true;
            _colorSettingsWindow.Close();
        }

        public void SaveImage(string filename, int filterIndex)
        {
            if (string.IsNullOrWhiteSpace(filename))
            {
                return;
            }

            //find out if tiff compression is necessary:
            bool doCompression = false;
            var applicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNode node = applicationDoc.SelectSingleNode("/ApplicationSettings/TIFFCompressionEnable");
            if (node != null)
            {
                string str = string.Empty;
                if (true == XmlManager.GetAttribute(node, applicationDoc, "value", ref str))
                {
                    doCompression = ("1" == str) ? true : false;
                }
            }

            FileStream stream;
            try
            {
                stream = new FileStream(filename, FileMode.Create);
            }
            catch (IOException ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, $"Unable to Save Image: {ex.Message}");
                return;
            }

            // create the bitmaps
            if (FrameData.frameInfo.channels != 1 && GetTheOnlyVisibleChannel(out ChannelViewModel loneChannel))
            {
                // TODO: this is a special case where we want to save one channel of a multichannel image - implement a more comprehensive solution in the future
                switch (filterIndex)
                {
                    case 1:
                    case 3:
                        {
                            //8 bit image save
                            _ = _imageViewM.BuildBitmapOrBitmap16ForOneChannel(false, loneChannel.Index);
                            if (Bitmap == null)
                            {
                                return;
                            }
                        }
                        break;
                    case 2:
                        {
                            //16 bit image save
                            _ = _imageViewM.BuildBitmapOrBitmap16ForOneChannel(true, loneChannel.Index);
                            if (Bitmap16 == null)
                            {
                                return;
                            }
                        }
                        break;
                }
            }
            else // normal bitmap building
            {
                switch (filterIndex)
                {
                    case 1:
                    case 3:
                    {
                        //8 bit
                        _ = _imageViewM.BuildBitmap();
                        if (Bitmap == null)
                        {
                            return;
                        }
                    }
                    break;
                    case 2:
                    {
                        //16 bit
                        _ = _imageViewM.BuildBitmap16();
                        if (Bitmap16 == null)
                        {
                            return;
                        }
                    }
                    break;
                }
            }

            switch (filterIndex)
            {
                case 1:
                    {
                        //8 bit tiff image save
                        TiffBitmapEncoder encoder = new TiffBitmapEncoder();
                        encoder.Frames.Add(BitmapFrame.Create(Bitmap));
                        encoder.Save(stream);
                    }
                    break;
                case 2:
                    {
                        //16 bit tiff image save
                        TiffBitmapEncoder encoder = new TiffBitmapEncoder();
                        encoder.Compression = doCompression ? TiffCompressOption.Lzw : TiffCompressOption.None;
                        BitmapMetadata bmd = new BitmapMetadata("tiff");
                        bmd.SetQuery("/ifd/{uint=270}", CreateOMEMetadata(Convert.ToInt32(_imageViewM.Bitmap16.Width), Convert.ToInt32(_imageViewM.Bitmap16.Height)));
                        encoder.Frames.Add(BitmapFrame.Create(_imageViewM.Bitmap16, null, bmd, null));
                        encoder.Save(stream);
                    }
                    break;
                case 3:
                    {
                        //8 bit jpeg image save
                        JpegBitmapEncoder jpgEncoder = new JpegBitmapEncoder();
                        jpgEncoder.Frames.Add(BitmapFrame.Create(Bitmap));
                        jpgEncoder.Save(stream);
                    }
                    break;
            }

            stream.Close();
        }

        public void SaveNow()
        {
            SaveImage(SaveNowImagePath, 2);
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            //persist application settings
            MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
            var applicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList nodeList = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ImageViewZoom");
            XmlNodeList channelTileNodes = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ChannelTileViewSettings");
            XmlNodeList histogramNodes = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/HistogramSettings");

            // TODO: reduce code dupe here
            if (0 != applicationDoc.BaseURI.CompareTo(_appDocPath))
            {
                try
                {
                    if (File.Exists(new Uri(_appDocPath).LocalPath))
                    {
                        //current modality is different from load, persist to last modality
                        XmlDocument doc = new XmlDocument();
                        doc.Load(_appDocPath);
                        nodeList = doc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ImageViewZoom");
                        if (0 < nodeList.Count)
                        {
                            XmlManager.SetAttribute(nodeList[0], doc, "value", ZoomLevel.ToString());  //TODO: load in vm
                            XmlManager.SetAttribute(nodeList[0], doc, "offsetX", ImageCanvas.RenderTransform.Value.OffsetX.ToString());
                            XmlManager.SetAttribute(nodeList[0], doc, "offsetY", ImageCanvas.RenderTransform.Value.OffsetY.ToString());
                        }
                        channelTileNodes = doc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ChannelTileViewSettings");
                        if (0 < channelTileNodes.Count)
                        {
                            bool tileViewIsEnabled = TileDisplay; //TODO: load in vm
                            XmlManager.SetAttribute(channelTileNodes[0], applicationDoc, "TilingEnableOption", tileViewIsEnabled.ToString());
                        }
                        if (0 < histogramNodes.Count)
                        {
                            XmlManager.SetAttribute(histogramNodes[0], doc, "ReducedBinValue", WhitePointMaxVal.ToString());
                            XmlManager.SetAttribute(histogramNodes[0], doc, "EnableMergedHistogram", IsSingleMergedHistogram.ToString());
                            XmlManager.SetAttribute(histogramNodes[0], doc, "EnableLineChart", IsLineChartHistogram.ToString());
                            XmlManager.SetAttribute(histogramNodes[0], doc, "NumberOfGridLines", NumHistogramGridLines.ToString());
                        }

                        doc.Save(new Uri(_appDocPath).LocalPath);
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, GetType().Name + "Could not load Application Settings doc: " + _appDocPath + "\n" + ex.Message);
                    return;
                }
            }
            else if (0 < nodeList.Count)
            {
                XmlManager.SetAttribute(nodeList[0], applicationDoc, "value", ZoomLevel.ToString());  //TODO: load in vm
                XmlManager.SetAttribute(nodeList[0], applicationDoc, "offsetX", ImageCanvas.RenderTransform.Value.OffsetX.ToString());
                XmlManager.SetAttribute(nodeList[0], applicationDoc, "offsetY", ImageCanvas.RenderTransform.Value.OffsetY.ToString());
            }

            if (0 < channelTileNodes.Count)
            {
                bool tileViewIsEnabled = TileDisplay;  //TODO: load in vm
                XmlManager.SetAttribute(channelTileNodes[0], applicationDoc, "TilingEnableOption", tileViewIsEnabled.ToString());
            }

            nodeList = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/OrthogonalViewSettings");

            if (nodeList.Count <= 0)
            {
                XmlNode nodeGen = applicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/General");
                if (nodeGen != null)
                {
                    XmlNode nodeOrtho = applicationDoc.CreateElement("OrthogonalViewSettings");
                    nodeGen.AppendChild(nodeOrtho);
                    nodeList = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/OrthogonalViewSettings");
                }
            }

            if (0 < nodeList.Count)
            {
                XmlManager.SetAttribute(nodeList[0], applicationDoc, "lineColorIndex", OrthogonalLineColorType.ToString());
                XmlManager.SetAttribute(nodeList[0], applicationDoc, "lineTypeIndex", OrthogonalLineType.ToString());
                XmlManager.SetAttribute(nodeList[0], applicationDoc, "zMultiplier", OrthogonalViewZMultiplier.ToString());  //TODO: load in vm
                string enableStr = true == IsOrthogonalViewChecked ? "1" : "0";
                XmlManager.SetAttribute(nodeList[0], applicationDoc, "enableOrthogonalView", enableStr);
            }

            if (0 < histogramNodes.Count)
            {
                XmlManager.SetAttribute(histogramNodes[0], applicationDoc, "ReducedBinValue", WhitePointMaxVal.ToString());
                XmlManager.SetAttribute(histogramNodes[0], applicationDoc, "EnableMergedHistogram", IsSingleMergedHistogram.ToString());
                XmlManager.SetAttribute(histogramNodes[0], applicationDoc, "EnableLineChart", IsLineChartHistogram.ToString());
                XmlManager.SetAttribute(histogramNodes[0], applicationDoc, "NumberOfGridLines", NumHistogramGridLines.ToString());
            }

            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);

            PersistHistogramSettings(ref experimentFile);
        }

        public void UpdateOrthogonalViewImages()
        {
            if (IsOrthogonalViewChecked)
            {
                //Set Click time-Gap
                TimeSpan ts;
                int totalNumOfZstack = ZStepNum;
                double zStepSize = ZStepSizeUM;
                ts = DateTime.Now - _lastOrthogonalViewUpdateTime;
                _imageViewM.InitializeOrthogonalViewDataAndBuffers(totalNumOfZstack, zStepSize);

                if (ts.TotalSeconds > 0.01 && _bwOrthogonalImageLoaderDone == true)
                {
                    if (VirtualZStack == true)
                    {
                        for (int i = 0; i < totalNumOfZstack; i++)
                        {
                            _imageViewM.CreateOrthogonalBitmap(i, totalNumOfZstack); // create Bitmap
                        }
                        //TODO: these xz yz bitmaps should also be created in separate thread
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
        }

        public void UpdateProgressTextOrthogonal(int percentage)
        {
            //set our progress dialog text and value
            _splashOrthogonalView.ProgressText = string.Format("{0}%", percentage.ToString());
            _splashOrthogonalView.ProgressValue = percentage;
        }

        public void UpdateROILineWidths(double zoomLevel)
        {
            //Update the live Overlays in Overlay manager so the correct stroke thickness gets persisted.
            OverlayManagerClass.Instance.ZoomLevel = zoomLevel;
            //Update compound image overlays so they update if tile view is active.
            //_imageViewM.UpdateCompoundImageROILineWidths(zoomLevel);
        }

        public void WhitePointThresholdUpdate(ImageIdentifier imageIdentifier, double whitePointThreshold)
        {
            _imageViewM.SetWhitePoint(imageIdentifier, whitePointThreshold);

            if (_imageViewM.OrthogonalViewEnabled && VirtualZStack)
            {
                //TODO: these xz yz bitmaps should also be created in separate thread
                UpdateOrthogonalViewImages();
            }
        }

        protected void CloseProgressWindow()
        {
            ((ICommand)MVMManager.Instance[_mainViewModel, "CloseProgressWindowCommand", null])?.Execute(null);
        }

        bool BuildChannelPalettes()
        {
            bool ret = false;
            string str;

            if (null == _lutFiles || null == _colorPalettesUI || null == _allLuts || null == _lutNames)
            {
                var hwDoc = (null != ReviewHardwareSettings && _mainViewModel == "ImageReviewViewModel") ? ReviewHardwareSettings : MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                if (null == hwDoc)
                {
                    return ret;
                }

                str = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\luts\\";

                if (!Directory.Exists(str))
                {
                    return ret;
                }

                _lutFiles = Directory.GetFiles(str, "*.txt");
                _colorPalettesUI = new ObservableCollection<ImageViewControl.LUTComboBoxView>();
                _allLuts = new List<Color>[_lutFiles.Length];
                _lutNames = new List<string>();

                for (int i = 0; i < _lutFiles.Length; i++)
                {
                    string p = System.IO.Path.GetDirectoryName(_lutFiles[i]) + "\\";
                    string f = System.IO.Path.GetFileName(_lutFiles[i]);
                    string[] tokens = f.Split(' ');
                    string nName = "";
                    for (int j = 0; j < tokens.Length; j++)
                    {
                        nName += tokens[j];
                    }
                    File.Move(p + f, p + nName);
                    _lutFiles[i] = p + nName;
                    _lutNames.Add(System.IO.Path.GetFileNameWithoutExtension(_lutFiles[i]));

                    List<Color> colors = new List<Color>();

                    StreamReader fs = new StreamReader(_lutFiles[i]);
                    string line;
                    int counter = 0;
                    try
                    {
                        while ((line = fs.ReadLine()) != null)
                        {
                            string[] split = line.Split(',');
                            if (split.Length >= 3)
                            {
                                if (split[0] != null)
                                {
                                    if (split[1] != null)
                                    {
                                        if (split[2] != null)
                                        {
                                            colors.Add(Color.FromRgb(Convert.ToByte(split[0]), Convert.ToByte(split[1]), Convert.ToByte(split[2])));
                                        }
                                    }
                                }
                            }
                            counter++;
                        }
                    }
                    catch (Exception ex)
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, ex.Message);
                    }

                    fs.Close();

                    //if the length of the file matches the bitmap size
                    if (256 == counter)
                    {
                        _colorPalettesUI.Add(new ImageViewControl.LUTComboBoxView(colors, _lutNames[i]));
                        _allLuts[i] = colors;
                    }
                    else
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, "ImageViewMVM BuildChannelPalettes error, unable to build palette for text file: " + f + " check if the LUT file has 256 lines");
                    }
                }
            }

            ret = true;
            return ret;
        }

        void ChangeColorSettings()
        {
            _colorSettingsWindow = new ImageViewControl.LUTSettings();

            _colorSettingsWindow.DataContext = this;

            if (true == _colorSettingsWindow.ShowDialog())
            {
                BuildChannelPalettes(); //Probably not needed unless we want to drop LUT files on the fly

                var applicationDoc = (null != ReviewApplicationSettings && _mainViewModel == "ImageReviewViewModel") ? ReviewApplicationSettings : MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNodeList ndList = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/GrayscaleForSingleChannel");

                if (ndList.Count > 0)
                {
                    string str = (true == GrayscaleForSingleChannel) ? "1" : "0";

                    XmlManager.SetAttribute(ndList[0], applicationDoc, "value", str);
                }

                ndList = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/OnlyShowTrueSaturation");

                if (ndList.Count > 0)
                {
                    string str = (true == OnlyShowTrueSaturation) ? "1" : "0";

                    XmlManager.SetAttribute(ndList[0], applicationDoc, "value", str);
                }

                if (null != ReviewApplicationSettings && _mainViewModel == "ImageReviewViewModel")
                {
                    applicationDoc.Save(ReviewApplicationSettingsPath);
                }
                else
                {
                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                }

                LoadColorImageSettings();

                _imageViewM.PaletteChanged = true;

                FireColorMappingChangedAction(true);
            }
        }

        string CreateOMEMetadata(int width, int height)
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

        void DeleteAllROIs()
        {
            if (MessageBoxResult.Yes == MessageBox.Show(new Window() { Topmost = true }, "Do you wish to delete all ROIs?", "Clear ROIs?", MessageBoxButton.YesNo, MessageBoxImage.Question, MessageBoxResult.Yes))
            {
                OverlayManagerClass.Instance.ClearAllObjects();
                ROIDrawingToolsSelectedIndex = 0;
                _isReticleChecked = false;
                _isScaleButtonChecked = false;
                OnPropertyChanged("IsReticleChecked");
                OnPropertyChanged("IsScaleButtonChecked");
                OverlayManagerClass.Instance.InitSelectROI();
            }
        }

        void DeleteSelectedROIs()
        {
            //TODO: overlay manager should let us know when all ROIs have been deleted
            OverlayManagerClass.Instance.DeleteSelectedROIs();
            ROIDrawingToolsSelectedIndex = 0;
            OverlayManagerClass.Instance.InitSelectROI();
            if (0 == OverlayManagerClass.Instance.ROICount)
            {
                //Force reset ROI count
                OverlayManagerClass.Instance.ClearAllObjects();
            }
        }

        //public Canvas OverlayCanvas
        //{
        //    get
        //    {
        //        return _oc;
        //    }
        //    set
        //    {
        //        _oc = value;
        //        OverlayManagerClass.Instance.OverlayCanvas = _oc;
        //    }
        //}
        void DisplayOrthogonalOptions()
        {
            ImageViewControl.OrthogonalViewOptionsWindow dlg = new ImageViewControl.OrthogonalViewOptionsWindow();
            dlg.ColorIndex = OrthogonalLineColorType;
            dlg.LineIndex = _orthogonalLineType;
            dlg.ZPixelMultiplier = OrthogonalViewZMultiplier;
            if (false == dlg.ShowDialog())
            {
                if (dlg.SetFlag == true)
                {
                    int colorIndex = dlg.ColorIndex;
                    int lineTypeIndex = dlg.LineIndex;
                    if (OrthogonalViewZMultiplier != dlg.ZPixelMultiplier)
                    {
                        OrthogonalLineColorType = colorIndex;
                        _orthogonalLineType = lineTypeIndex;
                        OrthogonalViewZMultiplier = dlg.ZPixelMultiplier;
                        if (IsOrthogonalViewChecked)
                        {
                            UpdateOrthogonalViewImages();
                            ++OrthogonalChangeCount;
                        }
                    }
                    else if (colorIndex != OrthogonalLineColorType || lineTypeIndex != _orthogonalLineType)
                    {
                        OrthogonalLineColorType = colorIndex;
                        OrthogonalLineType = lineTypeIndex;

                        if (IsOrthogonalViewChecked)
                        {
                            ++OrthogonalChangeCount;
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Expands all histograms to their maximum size. expandOrShrink is true for expand, and false for shrink
        /// </summary>
        void ExpandAllHistograms(bool expandOrShrink)
        {
            for (int i = 0; i < HistogramViewModels.Count; i++)
            {
                for (int j = 0; j < HistogramViewModels[i].Count; j++)
                {
                    HistogramViewModels[i][j].LargeHistogram = expandOrShrink;
                    HistogramViewModels[i][j].UpdateAllHistogramExpandButton(expandOrShrink);
                }
            }
        }

        void ImageViewM_BitmapUpdated()
        {
            OnPropertyChanged("ImagesGrid");
            BitmapReady = true;
            BitmapLoaded = true;
        }

        void ImageViewM_BitmapUpdateRateUpdated()
        {
            _bitmapUpdateRateText = "Image Update Rate: " + _imageViewM.BitmapUpdateRate;
            OnPropertyChanged("BitmapUpdateRateText");
        }

        void ImageViewM_ReferenceChannelInvalidated()
        {
            //Disable Reference channel and display message alerting user that image is invalid
            MVMManager.Instance["AreaControlViewModel", "EnableReferenceChannel"] = 0;
            MVMManager.Instance["CameraControlViewModel", "EnableReferenceChannel"] = false;
            ((ThorSharedTypes.IMVM)MVMManager.Instance["CameraControlViewModel"]).OnPropertyChange("EnableReferenceChannel");
            ((ThorSharedTypes.IMVM)MVMManager.Instance["AreaControlViewModel"]).OnPropertyChange("EnableReferenceChannel");
            MessageBox.Show("Please select a single channel 16-bit image as the reference", "Reference Channel Image Invalid");

            _imageViewM.FrameData.frameInfo.channels = _cachedNumChannelsReferenceImage;
            _imageViewM.FrameData.channelSelection = _cachedChannelSelection;
            FrameData = _imageViewM.FrameData; // Feed the modified framedata back through to force a bitmap update with the non-reference data
        }

        private void ImageViewM_ZoomAfterImageUpdate(object sender, ZoomChangeEventArgs args)
        {
            double currentZoomLevel = ZoomLevel;
            ZoomLevel /= args.ZoomFactorChange;

        }

        private void ImageViewVVMBase_ChannelDisplayEnabledChanged(int group, int index)
        {
            for (int i = 0; i < ChannelGroups[group].Channels.Count; i++)
            {
                if (ChannelGroups[group].Channels[i].Index == index)
                {
                    _imageViewM.ChannelDisplayEnable[group][index] = ChannelGroups[group].Channels[i].ChannelDisplayEnable;
                    _imageViewM.ChanEnableChanged = true;
                }
            }
        }

        private void Instance_NeedOverlayItemsUpdateToActiveROIs(System.Windows.Shapes.Shape shape)
        {
            SetOverlayItemsForROI(shape);
        }

        private void Instance_ROIListUpdated()
        {
            for (int i = 0; i < ImagesGrid?.Count; ++i)
            {
                for (int j = 0; j < ImagesGrid[i].Count; ++j)
                {
                    if (ImagesGrid[i][j].OverlayItems != OverlayManagerClass.Instance.OverlayItems || !OverlayManagerClass.Instance.IsAdornerLayerValid())
                    {
                        var items = ImagesGrid[i][j].OverlayItems;
                        OverlayManagerClass.Instance.GetDuplicatedROIList(ref items);
                        ImagesGrid[i][j].OverlayItems = items;
                    }
                }
            }
        }

        private void LoadROIsFromXAML()
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

            dlg.DefaultExt = ".xaml";
            dlg.Filter = "WPF XAML (.xaml)|*.xaml";

            Nullable<bool> result = dlg.ShowDialog();

            if (true == result)
            {
                OverlayManagerClass.Instance.LoadROIs(dlg.FileName, ref _isReticleChecked, ref _isScaleButtonChecked);
                OnPropertyChanged("IsReticleChecked");
                OnPropertyChanged("IsScaleButtonChecked");
            }
            ROIDrawingToolsSelectedIndex = 0;
            OverlayManagerClass.Instance.InitSelectROI();
        }

        void SaveAs()
        {
            // Configure save file dialog box
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Title = "Save an Image File";
            dlg.FileName = string.Format("Image_{0:yyyy-MM-dd_hh-mm-ss}", DateTime.Now);
            ;
            if (_imageViewM.IsSingleChannel)
            {

                dlg.Filter = "8 Bit Tiff file (*.tif)|*.tif|16 Bit Tiff file (*.tif)|*.tif|Jpeg file (*.jpg)|*.jpg";
            }
            else
            {
                dlg.Filter = "24 Bit Tiff file (*.tif)|*.tif|48 Bit Tiff file (*.tif)|*.tif|Jpeg file (*.jpg)|*.jpg";
            }

            dlg.FilterIndex = _saveDlgFilterIndex;

            // Show save file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process save file dialog box results
            if (result == true && dlg.FileName != "")
            {
                // Save the image file
                string filename = dlg.FileName;
                SaveImage(filename, dlg.FilterIndex);
            }

            _saveDlgFilterIndex = dlg.FilterIndex;
        }

        /// <summary>
        /// Returns true if there is only one channel visible. 
        /// If the return value is true, then visibleChannel is set to that lone channel.
        /// </summary>
        /// <param name="visibleChannel"></param>
        /// <returns></returns>
        bool GetTheOnlyVisibleChannel(out ChannelViewModel visibleChannel)
        {
            bool isOneVisible = false;
            visibleChannel = null;
            foreach (var channelGroup in ChannelGroups)
            {
                foreach (var channel in channelGroup.Channels)
                {
                    if(channel.ChannelDisplayEnable && channel.ChannelVisibility == Visibility.Visible)
                    {
                        if (isOneVisible)
                        { 
                            visibleChannel = null;
                            return false;
                        }
                        isOneVisible = true;
                        visibleChannel = channel;
                    }
                }
            }
            return isOneVisible;
        }

        void SaveAsReference()
        {
            if (_imageViewM.IsSingleChannel || GetTheOnlyVisibleChannel(out _))
            {
                string refChan = Application.Current.Resources["AppRootFolder"].ToString() + "\\ReferenceChannel.tif";
                SaveImage(refChan, 2);
                UpdateRegistration();
                string msg = string.Format("Reference channel saved to {0}", refChan);
                MessageBox.Show(msg);
            }
            else
            {
                MessageBox.Show("Cannot create a reference channel from a multichannel image. Choose a single channel image instead.");
            }
        }

        void SaveReferenceChannelAs()
        {
            if (1 == _imageViewM.GetColorChannels() || GetTheOnlyVisibleChannel(out _))
            {
                //string refChan = Application.Current.Resources["AppRootFolder"].ToString() + "\\ReferenceChannel.tif";
                SaveFileDialog dlg = new SaveFileDialog();
                dlg.Title = "Save an Image File";
                dlg.AddExtension = true;
                dlg.DefaultExt = "tif";
                dlg.FileName = string.Format("Image_{0:yyyy-MM-dd_hh-mm-ss}", DateTime.Now);
                dlg.InitialDirectory = Application.Current.Resources["AppRootFolder"].ToString();
                dlg.Filter = "16 Bit Tiff file (*.tif)|*.tif";

                // Show save file dialog box
                Nullable<bool> result = dlg.ShowDialog();
                // Process save file dialog box results
                if (result == true && dlg.FileName != "")
                {
                    // Save the image file
                    string filename = dlg.FileName;
                    SaveImage(filename, 2);

                    string msg = string.Format("Reference channel saved to {0}", filename);
                    MessageBox.Show(msg);
                }
            }
            else
            {
                MessageBox.Show("Cannot create a reference channel from a multichannel image. Choose a single channel image instead.");
            }
        }

        void SetOverlayItemsForROI(Shape roi)
        {
            for (int i = 0; i < ImagesGrid?.Count; ++i)
            {
                for (int j = 0; j < ImagesGrid[i].Count; ++j)
                {
                    if (ImagesGrid[i][j].OverlayItems != OverlayManagerClass.Instance.OverlayItems)
                    {
                        for (int k = 0; k < ImagesGrid[i][j].OverlayItems?.Count; ++k)
                        {
                            if (ImagesGrid[i][j].OverlayItems[k] == roi)
                            {
                                //If clicking between images, deselect roi's from current compound image before switching
                                //This case is needed in case an roi is selected on a different image than the imcoming roi.
                                if (OverlayManager.OverlayManagerClass.Instance.OverlayItems != ImagesGrid[i][j].OverlayItems)
                                {
                                    OverlayManager.OverlayManagerClass.Instance.DeselectAllROIs();
                                }
                                OverlayManagerClass.Instance.SetOverlayItems(ImagesGrid[i][j].OverlayItems, false);
                                return;
                            }
                        }
                    }
                }
            }

            //This was causing an issue with roi's disappearing in capture on mROI. Doesn't seem to cause any other issues when removed. Leaving commented for now just in case
            //if (ImagesGrid?.Count > 0 && ImagesGrid[0].Count > 0)
            //{
            //    OverlayManagerClass.Instance.SetOverlayItems(ImagesGrid[0][0].OverlayItems, false);
            //}
        }

        void UpdateChannelVisibility()
        {
            int numberOfchannels = 0;
            foreach (ChannelGroupViewModel channelGroup in ChannelGroups)
            {
                if (MAX_CHANNELS == channelGroup.Channels.Count)
                {
                    for (int i = 0; i < MAX_CHANNELS; i++)
                    {
                        if (true == (FrameData?.channelSelection & (0x0001 << i)) > 0)
                        {
                            channelGroup.Channels[i].ChannelVisibility = Visibility.Visible;
                            numberOfchannels++;
                        }
                        else
                        {
                            channelGroup.Channels[i].ChannelVisibility = Visibility.Collapsed;
                        }
                    }
                }
            }
            SPToolsMargin = IsInSequentialMode ? SPToolsMargin : new Thickness(5, numberOfchannels * CHANNEL_BUTTON_SIZE, 5, 5);

            foreach (var histogramRow in HistogramViewModels)
            {
                foreach (var histogramViewModel in histogramRow)
                {
                    bool isVisible = false;
                    foreach (int dataChannel in histogramViewModel.GetAllDataChannels())
                    {
                        var imageIdentifier = histogramViewModel.GetImageIdentifier(dataChannel);
                        int imageChannel = imageIdentifier.Channel;

                        if ((FrameData?.channelSelection & (0x0001 << imageChannel)) > 0 || IsInSequentialMode)
                        {
                            isVisible = true;
                        }
                    }
                    // if any of the histogram's data channels are visible, then the histogram is visible, otherwise it is collapsed
                    histogramViewModel.HistogramVisibility = isVisible ? Visibility.Visible : Visibility.Collapsed;
                }
            }
        }

        void UpdateHistogramGridIfNeeded()
        {
            bool isChanged = false;
            int numPlanes = Math.Max(FrameData.frameInfo.numberOfPlanes, 1);
            int areas = Math.Max(FrameData.frameInfo.totalScanAreas, 1);
            int vSize = areas > 1 ? areas : numPlanes;
            isChanged |= FrameData.frameInfo.channels != _cachedNumChannelsForHistogram;
            isChanged |= vSize != _cachedNumPlanesForHistogram;
            isChanged |= FrameData.channelSelection != _cachedChannelSelection;
            isChanged |= _requestHistogramRebuild;

            //Do not rebuild the histogram in sequential mode
            if (isChanged && !IsInSequentialMode)
            {
                // must be done on application thread
                Application.Current.Dispatcher.Invoke(new Action(() => BuildHistograms()));

                _cachedNumChannelsForHistogram = FrameData.frameInfo.channels;
                _cachedNumPlanesForHistogram = vSize;
                _cachedChannelSelection = FrameData.channelSelection;
                _requestHistogramRebuild = false;
            }
        }

        void UpdateOrthogonalView(bool displaySplash = true, int tIndex = 1)
        {
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;
            _bwOrthogonalImageLoaderDone = false;
            ProgressPercentage = 0;

            if (displaySplash)
            {
                _splashOrthogonalView = new ImageViewControl.ProgressSplashScreen();
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

                    int totalNumOfZstack = ZStepNum; //TODO: this needs to be different per control
                    double zStepSize = ZStepSizeUM;
                    _imageViewM.InitializeOrthogonalViewDataAndBuffers(totalNumOfZstack, zStepSize);

                    for (int i = 0; i < totalNumOfZstack; i++)
                    {
                        if (splashWkr.CancellationPending == true)
                        {
                            e.Cancel = true;
                            break;
                        }

                        _imageViewM.LoadZImageAndCreateOrthogonalViewBitmap(tIndex, i, totalNumOfZstack);
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
                    //_maxChannel = _imageViewM.MAX_CHANNELS;
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

                    DoneLoadingOrthogonalView = true;

                    ++OrthogonalChangeCount;
                };
                splashWkr.RunWorkerAsync();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ImageReview Orthogonal Images Loading Error: " + ex.Message);
                splashWkr.CancelAsync();
            }
        }

        void UpdatePolarScaling()
        {
            if (null == _imageViewM.FrameData)
            {
                return;
            }

            foreach (var histogramRow in _histogramViewModelsGrid)
            {
                foreach (var histogram in histogramRow)
                {
                    histogram.ScalingMin = PolarUtilities.GetUnitsMinimum(_imageViewM.FrameData);
                    histogram.ScalingMax = PolarUtilities.GetUnitsMaximum(_imageViewM.FrameData);
                    histogram.UnitSymbol = PolarUtilities.GetUnitsSymbol(_imageViewM.FrameData);
                }
            }
        }

        void UpdateRegistration()
        {
            try
            {
                MVMManager.Instance.ReloadSettings(SettingsFileType.REGISTRATION_SETTINGS);
                XmlDocument regDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.REGISTRATION_SETTINGS];
                XmlNodeList ndList = regDoc.SelectNodes("/ThorImageRegistration/Registrations/Registration[@Active='1']");
                for (int i = 0; i < ndList.Count; i++)
                {
                    XmlManager.SetAttribute(ndList[i], regDoc, "Active", "0");
                }
                ndList = regDoc.SelectNodes("/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']");
                if (ndList.Count <= 0)
                {
                    ndList = regDoc.SelectNodes("/ThorImageRegistration/Registrations");
                    XmlManager.CreateXmlNodeWithinNode(regDoc, ndList[0], "Registration");
                    ndList = regDoc.SelectNodes("/ThorImageRegistration/Registrations/Registration");
                }
                XmlManager.SetAttribute(ndList[ndList.Count - 1], regDoc, "Date", DateTime.Now.ToString());
                XmlManager.SetAttribute(ndList[ndList.Count - 1], regDoc, "Modality", ResourceManagerCS.GetModality());
                XmlManager.SetAttribute(ndList[ndList.Count - 1], regDoc, "CameraType", ResourceManagerCS.GetCameraType().ToString());
                switch ((ICamera.CameraType)ResourceManagerCS.GetCameraType())
                {
                    case ICamera.CameraType.CCD:
                        XmlManager.SetAttribute(ndList[ndList.Count - 1], regDoc, "Width", MVMManager.Instance["CameraControlViewModel", "CamImageWidth", (object)0].ToString());
                        XmlManager.SetAttribute(ndList[ndList.Count - 1], regDoc, "Height", MVMManager.Instance["CameraControlViewModel", "CamImageHeight", (object)0].ToString());
                        break;
                    case ICamera.CameraType.LSM:
                        XmlManager.SetAttribute(ndList[ndList.Count - 1], regDoc, "Zoom", MVMManager.Instance["AreaControlViewModel", "LSMZoom", (object)0].ToString());
                        XmlManager.SetAttribute(ndList[ndList.Count - 1], regDoc, "FieldSize", MVMManager.Instance["AreaControlViewModel", "LSMFieldSize", (object)0].ToString());
                        break;
                    default:
                        break;
                }
                XmlManager.SetAttribute(ndList[ndList.Count - 1], regDoc, "Active", "1");
                MVMManager.Instance.SaveSettings(SettingsFileType.REGISTRATION_SETTINGS);
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ImageView UpdateRegistration: " + ex.Message);
            }
        }

        void WhitePointIncrease(bool val)
        {
            _imageViewM.WhitePointIncreased = val;
        }

        private void _imageViewM_PixelBitShiftValueChanged()
        {
            for (int i = 0; i < HistogramViewModels.Count; i++)
            {
                var histogramRow = HistogramViewModels[0];
                for (int j = 0; j < histogramRow.Count; j++)
                {
                    HistogramViewModels[i][j].ShiftValue = _imageViewM.PixelBitShiftValue;
                }
            }
            OnPropertyChanged("PixelBitShiftValue");
        }

        private void _imageViewM_PixelDataUpdated(ImageIdentifier imageIdentifier)
        {
            if (_histogramViewModelsGrid.Count <= 0)
            {
                return;
            }

            lock (_imageViewM.UpdateHistogramsLock)
            {
                var pixelDataHistogram = _imageViewM.PixelDataHistograms.Find(x => x.DataImageIdentifier.Equals(imageIdentifier));
                if (null == pixelDataHistogram)
                {
                    // No pixel data histograms found that match the channel and plane
                    return;
                }

                var data = pixelDataHistogram.HistogramData;

                foreach (var histogramRow in HistogramViewModels)
                {
                    foreach (var histogramViewModel in histogramRow)
                    {
                        foreach (int dataChannel in histogramViewModel.GetAllDataChannels())
                        {
                            if (histogramViewModel.GetImageIdentifier(dataChannel).Equals(imageIdentifier))
                            {
                                histogramViewModel.SetData(dataChannel, data);
                                if (histogramViewModel.ContinuousAuto || _imageViewM.GetIsAutoButtonPressed(imageIdentifier))
                                {
                                    histogramViewModel.AutoBP = _imageViewM.GetBlackPoint(imageIdentifier);
                                    histogramViewModel.AutoWP = _imageViewM.GetWhitePoint(imageIdentifier);
                                    _imageViewM.SetIsAutoButtonPressed(imageIdentifier, false);
                                }
                            }
                        }
                    }
                }
            }
        }

        #endregion Methods
    }
}