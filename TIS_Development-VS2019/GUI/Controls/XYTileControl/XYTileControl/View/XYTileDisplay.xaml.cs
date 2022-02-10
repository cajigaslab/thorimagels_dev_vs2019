namespace XYTileControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    using SciChart.Charting.Visuals;
    using SciChart.Charting.Visuals.Annotations;

    using ThorLogging;

    using ThorSharedTypes;

    using XYTileControl.CustomModifiers;
    using XYTileControl.ViewModel;

    #region Enumerations

    /// <summary>
    /// Copy destination enumeration
    /// </summary>
    public enum CopyDst : int
    {
        Selected, // copy tiles of this well to other selected wells
        All       // copy tiles pf this well to all wells
    }

    /// <summary>
    /// Edit: CaptureSetup
    /// Capture: Capture
    /// View: ImageReview
    /// </summary>
    public enum TileDisplayMode : int
    {
        Edit,
        Capture,
        View,
        DefaultView,
    }

    #endregion Enumerations

    /// <summary>
    /// Record Well Information
    /// </summary>
    public struct WellPosition
    {
        #region Fields

        public int Column; // The horizatonal position of well
        public int Row; // The vertical    position of well

        #endregion Fields

        #region Constructors

        public WellPosition(int row, int column)
        {
            this.Column = column;
            this.Row = row;
        }

        #endregion Constructors
    }

    /// <summary>
    /// Interaction logic for XYTileDisplay.xaml
    /// </summary>
    public partial class XYTileDisplay : UserControl, INotifyPropertyChanged
    {
        #region Fields

        public static DependencyProperty ActiveXMLProperty = 
        DependencyProperty.RegisterAttached("ActiveXML",
        typeof(string),
        typeof(XYTileDisplay));
        public static DependencyProperty ApplicationSettingsProperty = 
        DependencyProperty.RegisterAttached("ApplicationSettings",
        typeof(string),
        typeof(XYTileDisplay));
        public static DependencyProperty CurrentCarrierProperty = 
        DependencyProperty.RegisterAttached("CurrentCarrier",
        typeof(Carrier),
        typeof(XYTileDisplay));
        public static DependencyProperty DefaultOverlapXProperty = 
        DependencyProperty.RegisterAttached("DefaultOverlapX",
        typeof(double),
        typeof(XYTileDisplay));
        public static DependencyProperty DefaultOverlapYProperty = 
        DependencyProperty.RegisterAttached("DefaultOverlapY",
        typeof(double),
        typeof(XYTileDisplay));
        public static DependencyProperty DisplayTileGridProperty = 
        DependencyProperty.RegisterAttached("DisplayTileGrid",
        typeof(int),
        typeof(XYTileDisplay));
        public static DependencyProperty FP1XYZProperty = 
        DependencyProperty.RegisterAttached("FP1XYZ",
        typeof(double[]),
        typeof(XYTileDisplay));
        public static DependencyProperty FP2XYZProperty = 
        DependencyProperty.RegisterAttached("FP2XYZ",
        typeof(double[]),
        typeof(XYTileDisplay));
        public static DependencyProperty FP3XYZProperty = 
        DependencyProperty.RegisterAttached("FP3XYZ",
        typeof(double[]),
        typeof(XYTileDisplay));
        public static DependencyProperty HomePosXProperty = 
        DependencyProperty.RegisterAttached("HomePosX",
        typeof(double),
        typeof(XYTileDisplay),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onHomePosChanged)));
        public static DependencyProperty HomePosYProperty = 
        DependencyProperty.RegisterAttached("HomePosY",
        typeof(double),
        typeof(XYTileDisplay),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onHomePosChanged)));
        public static DependencyProperty HomePosZProperty = 
        DependencyProperty.RegisterAttached("HomePosZ",
        typeof(double),
        typeof(XYTileDisplay));

        /// <summary>
        /// The maximum x property
        /// </summary>
        public static DependencyProperty MaxXProperty = 
        DependencyProperty.RegisterAttached("MaxX",
        typeof(double),
        typeof(XYTileDisplay),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onRangeAreaChanged)));

        /// <summary>
        /// The maximum y property
        /// </summary>
        public static DependencyProperty MaxYProperty = 
        DependencyProperty.RegisterAttached("MaxY",
        typeof(double),
        typeof(XYTileDisplay),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onRangeAreaChanged)));

        /// <summary>
        /// The minimum x property
        /// </summary>
        public static DependencyProperty MinXProperty = 
        DependencyProperty.RegisterAttached("MinX",
        typeof(double),
        typeof(XYTileDisplay),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onRangeAreaChanged)));

        /// <summary>
        /// The minimum y property
        /// </summary>
        public static DependencyProperty MinYProperty = 
        DependencyProperty.RegisterAttached("MinY",
        typeof(double),
        typeof(XYTileDisplay),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onRangeAreaChanged)));

        /// <summary>
        /// The Exec mode property
        /// </summary>
        public static DependencyProperty ModeProperty = 
        DependencyProperty.RegisterAttached("Mode",
        typeof(TileDisplayMode),
        typeof(XYTileDisplay));

        /// <summary>
        /// The scan area height property
        /// </summary>
        public static DependencyProperty ScanAreaHeightProperty = 
        DependencyProperty.RegisterAttached("ScanAreaHeight",
        typeof(double),
        typeof(XYTileDisplay),
         new FrameworkPropertyMetadata(new PropertyChangedCallback(onScanAreaHeightChanged)));

        /// <summary>
        /// The scan area width property
        /// </summary>
        public static DependencyProperty ScanAreaWidthProperty = 
        DependencyProperty.RegisterAttached("ScanAreaWidth",
        typeof(double),
        typeof(XYTileDisplay),
         new FrameworkPropertyMetadata(new PropertyChangedCallback(onScanAreaWidthChanged)));

        /// <summary>
        /// The current x position property
        /// </summary>
        public static DependencyProperty ScanAreaXPositionProperty = 
        DependencyProperty.RegisterAttached("ScanAreaXPosition",
        typeof(double),
        typeof(XYTileDisplay),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onScanAreaXPositionChanged)));

        /// <summary>
        /// The current y position property
        /// </summary>
        public static DependencyProperty ScanAreaYPositionProperty = 
        DependencyProperty.RegisterAttached("ScanAreaYPosition",
        typeof(double),
        typeof(XYTileDisplay),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onScanAreaYPositionChanged)));

        /// <summary>
        /// The current z position property
        /// </summary>
        public static DependencyProperty ScanAreaZPositionProperty = 
        DependencyProperty.RegisterAttached("ScanAreaZPosition",
        typeof(double),
        typeof(XYTileDisplay));

        /// <summary>
        /// The step size x property
        /// </summary>
        public static DependencyProperty StepSizeXProperty = 
        DependencyProperty.RegisterAttached("StepSizeX",
        typeof(double),
        typeof(XYTileDisplay));

        /// <summary>
        /// The step size y property
        /// </summary>
        public static DependencyProperty StepSizeYProperty = 
        DependencyProperty.RegisterAttached("StepSizeY",
        typeof(double),
        typeof(XYTileDisplay));
        public static DependencyProperty TileIndexProperty = 
        DependencyProperty.RegisterAttached("TileIndex",
        typeof(int),
        typeof(XYTileDisplay));
        public static DependencyProperty TiltAdjustmentProperty = 
        DependencyProperty.RegisterAttached("TiltAdjustment",
        typeof(int),
        typeof(XYTileDisplay));
        public static DependencyProperty XYControlPanelScaleProperty = 
        DependencyProperty.RegisterAttached("XYControlPanelScale",
        typeof(double),
        typeof(XYTileDisplay));
        public static DependencyProperty XYPanelScaleProperty = 
        DependencyProperty.RegisterAttached("XYPanelScale",
        typeof(double),
        typeof(XYTileDisplay));
        public static DependencyProperty XYtableDataProperty = 
        DependencyProperty.RegisterAttached("XYtableData",
        typeof(ObservableCollection<XYPosition>),
        typeof(XYTileDisplay));
        public static DependencyProperty XZeroVisibilityProperty = 
        DependencyProperty.RegisterAttached("XZeroVisibile",
        typeof(Visibility),
        typeof(XYTileDisplay));
        public static DependencyProperty YZeroVisibilityProperty = 
        DependencyProperty.RegisterAttached("YZeroVisibile",
        typeof(Visibility),
        typeof(XYTileDisplay));

        private const double MAX_STAGE_STEP = 100;
        private const double MIN_STAGE_STEP = .0001;

        private XYtileViewModel viewModel;
        private XYPosition _duplicatedTiles = new XYPosition();
        private Point _homeFieldPos; //The mouse position of right-click contextmenu
        private bool _isChartDragSelectModifierEnabled = true;
        private Visibility _isControlPanelVisible = Visibility.Visible;
        private bool _isDataGridReadOnly = false;
        private Visibility _isTileTableVisible = Visibility.Visible;
        private Visibility _nextPreviousTileStackPanelVisible = Visibility.Collapsed;
        private ObservableCollection<Carrier> _sampleCarrierCollection = null;
        private int _selectedCarrierIndex = -1;
        private Visibility _setStepToWellSizeVisibility = Visibility.Collapsed;
        private int _xyTableDragStartIndex = -1;
        private bool _xyTableIsDrag = false;
        private Cursor _xyTableOldCur = null;
        private int _xyTableSelectedIndex = -1;

        #endregion Fields

        #region Constructors

        public XYTileDisplay()
        {
            // Ensure SetLicenseKey is called once, before any SciChartSurface instance is created
            // Check this code into your version-control and it will enable SciChart
            // for end-users of your application.
            //
            // You can test the Runtime Key is installed correctly by Running your application
            // OUTSIDE Of Visual Studio (no debugger attached). Trial watermarks should be removed.
            SciChartSurface.SetRuntimeLicenseKey("TCqHmTkInF/fDQuv2IRL2jISc44wjQP46+iIvQjEtY21jW+X66HmcupG3FzPOD39A8zSj8i8vKIUgW2r9wgDzzuy3RK/gQsogW5d2SN0QVo0tnTzAd/uEWHLFeS2W17/2hf//FVKxwU4704JENFsCxYbOoPZHbpNwbTJovnl1QjEabIjy1KzBkA2fJMJbWF8wPRTD0ruKUEnrHpXOuvpTOQlr7a6XSmUlJ5o/Vsx7oJRcIYm70L7shDDXu1hHEqICpBtcCb91kpgNMaAZoWJwhYiBmowdHbgszC9lm3o6hlLi35y88379sblqhR1b7rIh80hoc3XwfQUmPydvU6RAwLUyIYT/z28JOl3kx0pReVdlLQd5bfdldNeNrI6J3ajng427j2udkQpNqQxNUEbLH9D/qqr5xeez+F/O4FWIYiYJvs9pgMamA6GYfGnV1sQ2spekHboGxh5PWfNgAWTuqFU/arLx5W1LYhT75WcXUe8pSXX1JD6qGD7/G4l9KpN+CYuZrXh1Zl9ND5KLicMDvfX65W+B8ka0TZbLIFExmsWSwNt+n6osLwE48Q8JsPb1+WCzy+1oCaFnyGXcpK5LlVB0Dcg9VdcDnwmrEQ=");

            InitializeComponent();
            this.Loaded += XYTileDisplay_Loaded;
            this.Unloaded += XYTileDisplay_Unloaded;
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                XYPositionExpander.IsExpanded = true;
                PositionTable.IsExpanded = true;
                TileEnabled.Visibility = Visibility.Collapsed;
                sciChart.RenderPriority = RenderPriority.Normal;
            }
        }

        #endregion Constructors

        #region Delegates

        /// <summary>
        /// define event header to call back high-level
        /// </summary>
        /// <param name="tilesIndex">Index of the tiles.</param>
        public delegate void ImageChangedEventHandler(int wellIndex, int tilesIndex);

        #endregion Delegates

        #region Events

        /// <summary>
        /// Occurs when user wants to move tiled area to postion as scanarea as center
        /// </summary>
        public event Action<XYPosition> CenterTiles;

        /// <summary>
        /// Occurs when user wants to copy tiles
        /// </summary>
        public event Action<XYPosition, int> CopyTiles;

        /// <summary>
        /// Occurs when [deselect all wells].
        /// </summary>
        public event Action DeselectAllWellsEvent;

        /// <summary>
        /// Occurs when experiment file path is changed
        /// </summary>
        public event Action<string> ExpPathChanged;

        /// <summary>
        /// Occurs when user wants to load specific experiment or application file 
        /// </summary>
        public event Action Load;

        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Occurs when [scan area size changed].
        /// </summary>
        public event Action ScanAreaSizeChanged;

        /// <summary>
        /// Occurs when [single position checked status changed].
        /// </summary>
        public event Action<bool> SinglePositionCheckedStatusChanged;

        /// <summary>
        /// Occurs when [tiles index changed event].
        /// </summary>
        public event ImageChangedEventHandler TilesIndexChangedEvent;

        /// <summary>
        /// Occurs when [tiles position changed].
        /// </summary>
        public event Action<XYPosition> TilesPositionChanged;

        /// <summary>
        /// Occurs when [well selected status changed].
        /// </summary>
        public event Action<MenuItem, bool> WellSelectedStatusChanged;

        /// <summary>
        /// Occurs when [x position changed].
        /// </summary>
        public event Action XPositionChanged;

        /// <summary>
        /// Occurs when [x set zero].
        /// </summary>
        public event EventHandler XSetZero;

        /// <summary>
        /// Occurs when [xy stage range changed].
        /// </summary>
        public event Action XYStageRangeChanged;

        /// <summary>
        /// Occurs when [y position changed].
        /// </summary>
        public event Action YPositionChanged;

        /// <summary>
        /// Occurs when [y set zero].
        /// </summary>
        public event EventHandler YSetZero;

        #endregion Events

        #region Properties

        /// <summary>
        /// Gets or sets the active or exp XML.
        /// </summary>
        /// <value>
        /// The active XML.
        /// </value>
        public string ActiveXML
        {
            get
            {
                return (string)GetValue(ActiveXMLProperty);
            }
            set
            {
                SetValue(ActiveXMLProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the application settings.
        /// </summary>
        /// <value>
        /// The application settings.
        /// </value>
        public string ApplicationSettings
        {
            get
            {
                return (string)GetValue(ApplicationSettingsProperty);
            }
            set
            {
                SetValue(ApplicationSettingsProperty, value);
            }
        }

        public Carrier CurrentCarrier
        {
            get { return (Carrier)GetValue(CurrentCarrierProperty); }
            set
            {
                SetValue(CurrentCarrierProperty, value);
            }
        }

        /// <summary>
        /// Gets the name of the current sample carrier.
        /// </summary>
        /// <value>
        /// The name of the current sample carrier.
        /// </value>
        public String CurrentSampleCarrierName
        {
            get
            {
                if (SampleCarrierCollection != null && SelectedCarrierIndex >= 0)
                {
                    return (SampleCarrierCollection[SelectedCarrierIndex] as Carrier).Name;
                }
                return String.Empty;
            }
        }

        /// <summary>
        /// Gets or sets the default overlap x.
        /// </summary>
        /// <value>
        /// The default overlap x.
        /// </value>
        public double DefaultOverlapX
        {
            get
            {
                return (double)GetValue(DefaultOverlapXProperty);
            }
            set
            {
                SetValue(DefaultOverlapXProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the default overlap y.
        /// </summary>
        /// <value>
        /// The default overlap y.
        /// </value>
        public double DefaultOverlapY
        {
            get
            {
                return (double)GetValue(DefaultOverlapYProperty);
            }
            set
            {
                SetValue(DefaultOverlapYProperty, value);
            }
        }

        public int DisplayTileGrid
        {
            get
            {
                return (int)GetValue(DisplayTileGridProperty);
            }
            set
            {
                SetValue(DisplayTileGridProperty, value);
                OnPropertyChanged("DisplayTileGrid");
            }
        }

        /// <summary>
        /// Gets the duplicated tiles.
        /// </summary>
        /// <value>
        /// The duplicated tiles.
        /// </value>
        public XYPosition DuplicatedTiles
        {
            get
            {
                return _duplicatedTiles;
            }
        }

        public string FocusPoint1
        {
            get
            {
                Decimal x = (null != FP1XYZ) ? new Decimal(FP1XYZ[0]) : 0;
                Decimal y = (null != FP1XYZ) ? new Decimal(FP1XYZ[1]) : 0;
                Decimal z = (null != FP1XYZ) ? new Decimal(FP1XYZ[2]) : 0;

                return string.Format("X:{0} Y:{1} Z:{2}", Decimal.Round(x, 4).ToString(), Decimal.Round(y, 4).ToString(), Decimal.Round(z, 4).ToString());
            }
        }

        public string FocusPoint2
        {
            get
            {
                Decimal x = (null != FP2XYZ) ? new Decimal(FP2XYZ[0]) : 0;
                Decimal y = (null != FP2XYZ) ? new Decimal(FP2XYZ[1]) : 0;
                Decimal z = (null != FP2XYZ) ? new Decimal(FP2XYZ[2]) : 0;

                return string.Format("X:{0} Y:{1} Z:{2}", Decimal.Round(x, 4).ToString(), Decimal.Round(y, 4).ToString(), Decimal.Round(z, 4).ToString());
            }
        }

        public string FocusPoint3
        {
            get
            {
                Decimal x = (null != FP3XYZ) ? new Decimal(FP3XYZ[0]) : 0;
                Decimal y = (null != FP3XYZ) ? new Decimal(FP3XYZ[1]) : 0;
                Decimal z = (null != FP3XYZ) ? new Decimal(FP3XYZ[2]) : 0;

                return string.Format("X:{0} Y:{1} Z:{2}", Decimal.Round(x, 4).ToString(), Decimal.Round(y, 4).ToString(), Decimal.Round(z, 4).ToString());
            }
        }

        public double[] FP1XYZ
        {
            get
            {
                return (double[])GetValue(FP1XYZProperty);
            }
            set
            {
                SetValue(FP1XYZProperty, value);
            }
        }

        public double[] FP2XYZ
        {
            get
            {
                return (double[])GetValue(FP2XYZProperty);
            }
            set
            {
                SetValue(FP2XYZProperty, value);
            }
        }

        public double[] FP3XYZ
        {
            get
            {
                return (double[])GetValue(FP3XYZProperty);
            }
            set
            {
                SetValue(FP3XYZProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the home position x.
        /// </summary>
        /// <value>
        /// The home position x.
        /// </value>
        public double HomePosX
        {
            get
            {
                return (double)GetValue(HomePosXProperty);
            }
            set
            {
                SetValue(HomePosXProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the home position y.
        /// </summary>
        /// <value>
        /// The home position y.
        /// </value>
        public double HomePosY
        {
            get
            {
                return (double)GetValue(HomePosYProperty);
            }
            set
            {
                SetValue(HomePosYProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the home position z.
        /// </summary>
        /// <value>
        /// The home position z.
        /// </value>
        public double HomePosZ
        {
            get
            {
                return (double)GetValue(HomePosZProperty);
            }
            set
            {
                SetValue(HomePosZProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is chart drag select modifier enabled.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is chart drag select modifier enabled; otherwise, <c>false</c>.
        /// </value>
        public bool IsChartDragSelectModifierEnabled
        {
            get
            {
                return _isChartDragSelectModifierEnabled;
            }
            set
            {
                _isChartDragSelectModifierEnabled = value;
                OnPropertyChanged("IsChartDragSelectModifierEnabled");
            }
        }

        /// <summary>
        /// Gets or sets the is control panel visible.
        /// </summary>
        /// <value>
        /// The is control panel visible.
        /// </value>
        public Visibility IsControlPanelVisible
        {
            get
            {
                return _isControlPanelVisible;
            }
            set
            {
                _isControlPanelVisible = value;
                OnPropertyChanged("IsControlPanelVisible");
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this data grid is readonly.
        /// </summary>
        /// <value>
        /// <c>true</c> if data grid is read only; otherwise, <c>false</c>.
        /// </value>
        public bool IsDataGridReadOnly
        {
            get
            {
                return _isDataGridReadOnly;
            }
            set
            {
                _isDataGridReadOnly = value;
                OnPropertyChanged("IsDataGridReadOnly");
            }
        }

        /// <summary>
        /// Gets or sets the is tile table visible.
        /// </summary>
        /// <value>
        /// The is tile table visible.
        /// </value>
        public Visibility IsTileTableVisible
        {
            get
            {
                return _isTileTableVisible;
            }
            set
            {
                _isTileTableVisible = value;
                OnPropertyChanged("IsTileTableVisible");
            }
        }

        /// <summary>
        /// Gets or sets the maximum x.
        /// </summary>
        /// <value>
        /// The maximum x.
        /// </value>
        public double MaxX
        {
            get
            {
                return (double)GetValue(MaxXProperty);
            }
            set
            {
                SetValue(MaxXProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the maximum y.
        /// </summary>
        /// <value>
        /// The maximum y.
        /// </value>
        public double MaxY
        {
            get
            {
                return (double)GetValue(MaxYProperty);
            }
            set
            {
                SetValue(MaxYProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the minimum x.
        /// </summary>
        /// <value>
        /// The minimum x.
        /// </value>
        public double MinX
        {
            get
            {
                return (double)GetValue(MinXProperty);
            }
            set
            {
                SetValue(MinXProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the minimum y.
        /// </summary>
        /// <value>
        /// The minimum y.
        /// </value>
        public double MinY
        {
            get
            {
                return (double)GetValue(MinYProperty);
            }
            set
            {
                SetValue(MinYProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the mode.
        /// </summary>
        /// <value>
        /// The mode.
        /// </value>
        public TileDisplayMode Mode
        {
            get
            {
                return (TileDisplayMode)GetValue(ModeProperty);
            }
            set
            {
                SetValue(ModeProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the is next previous stack panel visible.
        /// </summary>
        /// <value>
        /// The is tile table visible.
        /// </value>
        public Visibility NextPreviousTileStackPanelVisible
        {
            get
            {
                return _nextPreviousTileStackPanelVisible;
            }
            set
            {
                _nextPreviousTileStackPanelVisible = value;
                OnPropertyChanged("NextPreviousTileStackPanelVisible");
            }
        }

        /// <summary>
        /// Gets or sets the sample carrier collection.
        /// </summary>
        /// <value>
        /// The sample carrier collection.
        /// </value>
        public ObservableCollection<Carrier> SampleCarrierCollection
        {
            get
            {
                return this._sampleCarrierCollection;
            }
            set
            {
                this._sampleCarrierCollection = value;
            }
        }

        /// <summary>
        /// Gets or sets the height of the scan area.
        /// </summary>
        /// <value>
        /// The height of the scan area.unit[mm]
        /// </value>
        public double ScanAreaHeight
        {
            get
            {
                return (double)GetValue(ScanAreaHeightProperty) / 1000;
            }
            set
            {
                SetValue(ScanAreaHeightProperty, value * 1000);
            }
        }

        /// <summary>
        /// Gets or sets the width of the scan area.
        /// </summary>
        /// <value>
        /// The width of the scan area. unit[mm]
        /// </value>
        public double ScanAreaWidth
        {
            get
            {
                return (double)GetValue(ScanAreaWidthProperty) / 1000;
            }
            set
            {
                SetValue(ScanAreaWidthProperty, value * 1000);
            }
        }

        /// <summary>
        /// Gets or sets the current x position.
        /// </summary>
        /// <value>
        /// The current x position.
        /// </value>
        public double ScanAreaXPosition
        {
            get
            {
                return (double)GetValue(ScanAreaXPositionProperty);
            }
            set
            {
                SetValue(ScanAreaXPositionProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether [current x position out of bounds].
        /// </summary>
        /// <value>
        /// <c>true</c> if [current x position out of bounds]; otherwise, <c>false</c>.
        /// </value>
        public bool ScanAreaXPosOutOfBounds
        {
            get
            {
                return (ScanAreaXPosition < MinX) | (ScanAreaXPosition > MaxX);
            }
        }

        /// <summary>
        /// Gets or sets the current y position.
        /// </summary>
        /// <value>
        /// The current y position.
        /// </value>
        public double ScanAreaYPosition
        {
            get
            {
                return (double)GetValue(ScanAreaYPositionProperty);
            }
            set
            {
                SetValue(ScanAreaYPositionProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether [current y position out of bounds].
        /// </summary>
        /// <value>
        /// <c>true</c> if [current y position out of bounds]; otherwise, <c>false</c>.
        /// </value>
        public bool ScanAreaYPosOutOfBounds
        {
            get
            {
                return (ScanAreaYPosition < MinY) | (ScanAreaYPosition > MaxY);
            }
        }

        /// <summary>
        /// Gets or sets the current z position.
        /// </summary>
        /// <value>
        /// The current z position.
        /// </value>
        public double ScanAreaZPosition
        {
            get
            {
                return (double)GetValue(ScanAreaZPositionProperty);
            }
            set
            {
                SetValue(ScanAreaZPositionProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the index of the current carrier.
        /// </summary>
        /// <value>
        /// The index of the current carrier.
        /// </value>
        public int SelectedCarrierIndex
        {
            get
            {
                return _selectedCarrierIndex;
            }
            set
            {
                _selectedCarrierIndex = value;
                OnPropertyChanged("CurrentSampleCarrierName");
                if (SampleCarrierCollection[SelectedCarrierIndex].Type != CarrierType.Slide)
                {
                    SelectionEnabled.IsEnabled = true;
                }
                else
                {
                    SelectionEnabled.IsEnabled = false;
                }
                CurrentCarrier = SampleCarrierCollection[SelectedCarrierIndex];
                viewModel.ResetSample();
            }
        }

        /// <summary>
        /// Gets or sets the Set Step to well size visibility.
        /// </summary>
        /// <value>
        /// The is control panel visible.
        /// </value>
        public Visibility SetStepToWellSizeVisibility
        {
            get
            {
                return _setStepToWellSizeVisibility;
            }
            set
            {
                _setStepToWellSizeVisibility = value;
                OnPropertyChanged("SetStepToWellSizeVisibility");
            }
        }

        /// <summary>
        /// Gets or sets the step size x.
        /// </summary>
        /// <value>
        /// The step size x.
        /// </value>
        public double StepSizeX
        {
            get
            {
                return (double)GetValue(StepSizeXProperty);
            }
            set
            {
                SetValue(StepSizeXProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the step size y.
        /// </summary>
        /// <value>
        /// The step size y.
        /// </value>
        public double StepSizeY
        {
            get
            {
                return (double)GetValue(StepSizeYProperty);
            }
            set
            {
                SetValue(StepSizeYProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the index of the tile. The index is defined in the viewModel
        /// </summary>
        /// <value>
        /// The index of the tile. 
        /// </value>
        public int TileIndex
        {
            get
            {
                return (int)GetValue(TileIndexProperty);
            }
            set
            {
                SetValue(TileIndexProperty, value);
            }
        }

        public int TiltAdjustment
        {
            get
            {
                return (int)GetValue(TiltAdjustmentProperty);
            }
            set
            {
                SetValue(TiltAdjustmentProperty, value);
            }
        }

        /// <summary>
        /// The total number of enabled tiles
        /// </summary>
        public int TotalTiles
        {
            get
            {
                if (null != viewModel)
                {
                    return viewModel.GetTotalNumberTiles();
                }
                else
                {
                    return 0;
                }
            }
        }

        public double XYControlPanelScale
        {
            get { return (double)GetValue(XYControlPanelScaleProperty); }
            set { SetValue(XYControlPanelScaleProperty, value); }
        }

        public double XYPanelScale
        {
            get { return (double)GetValue(XYPanelScaleProperty); }
            set { SetValue(XYPanelScaleProperty, value); }
        }

        /// <summary>
        /// Gets or sets the x ytable data.
        /// </summary>
        /// <value>
        /// The x ytable data.
        /// </value>
        public ObservableCollection<XYPosition> XYtableData
        {
            get
            {
                return (ObservableCollection<XYPosition>)GetValue(XYtableDataProperty);
            }
            set
            {
                SetValue(XYtableDataProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the x zero visibility.
        /// </summary>
        /// <value>The x zero visibility.</value>
        public int XZeroVisibility
        {
            get
            {
                return (int)GetValue(XZeroVisibilityProperty);
            }
            set
            {
                SetValue(XZeroVisibilityProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the y zero visibility.
        /// </summary>
        /// <value>The y zero visibility.</value>
        public int YZeroVisibility
        {
            get
            {
                return (int)GetValue(YZeroVisibilityProperty);
            }
            set
            {
                SetValue(YZeroVisibilityProperty, value);
            }
        }

        #endregion Properties

        #region Methods

        public static T FindVisualChild<T>(DependencyObject current)
            where T : DependencyObject
        {
            if (current == null) return null;
            int childrenCount = VisualTreeHelper.GetChildrenCount(current);
            for (int i = 0; i < childrenCount; i++)
            {
                DependencyObject child = VisualTreeHelper.GetChild(current, i);
                if (child is T) return (T)child;
                T result = FindVisualChild<T>(child);
                if (result != null) return result;
            }
            return null;
        }

        public static void onHomePosChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ((XYTileDisplay)d).viewModel.ResetSampleHomeLocation();
        }

        /// <summary>
        /// Ons the range area changed.
        /// </summary>
        /// <param name="d">The d.</param>
        /// <param name="e">The <see cref="DependencyPropertyChangedEventArgs"/> instance containing the event data.</param>
        public static void onRangeAreaChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if ((d as XYTileDisplay).XYStageRangeChanged != null)
            {
                (d as XYTileDisplay).XYStageRangeChanged();
            }
        }

        /// <summary>
        /// Ons the scan area height changed.
        /// </summary>
        /// <param name="d">The d.</param>
        /// <param name="e">The <see cref="DependencyPropertyChangedEventArgs"/> instance containing the event data.</param>
        public static void onScanAreaHeightChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                if ((d as XYTileDisplay).ScanAreaSizeChanged != null)
                {
                    (d as XYTileDisplay).ScanAreaSizeChanged();
                }
            }
            catch (Exception ex)
            {
                if (ex.Message != string.Empty)
                {
                    //MessageBox.Show(ex.Message, "Field Height changed inappropriately");
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Field Height changed inappropriately. Exception thrown: " + ex.Message);
                }

            }
        }

        /// <summary>
        /// Ons the scan area width changed.
        /// </summary>
        /// <param name="d">The d.</param>
        /// <param name="e">The <see cref="DependencyPropertyChangedEventArgs"/> instance containing the event data.</param>
        public static void onScanAreaWidthChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                if ((d as XYTileDisplay).ScanAreaSizeChanged != null)
                {
                    (d as XYTileDisplay).ScanAreaSizeChanged();
                }
            }
            catch (Exception ex)
            {
                if (ex.Message != string.Empty)
                {
                    //MessageBox.Show(ex.Message, "Field Width changed inappropriately");
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Field Width changed inappropriately. Exception thrown: " + ex.Message);
                }

            }
        }

        /// <summary>
        /// Ons the scan area x position changed.
        /// </summary>
        /// <param name="d">The d.</param>
        /// <param name="e">The <see cref="DependencyPropertyChangedEventArgs"/> instance containing the event data.</param>
        public static void onScanAreaXPositionChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                if ((d as XYTileDisplay).XPositionChanged != null)
                {
                    (d as XYTileDisplay).XPositionChanged();
                }
                if ((d as XYTileDisplay) != null)
                {
                    (d as XYTileDisplay).OnPropertyChanged("ScanAreaXPosOutOfBounds");
                }
            }
            catch (Exception ex)
            {
                if (ex.Message != string.Empty)
                {
                    //MessageBox.Show(ex.Message, "Field Height changed inappropriately");
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Field X changed inappropriately. Exception thrown: " + ex.Message);
                }

            }
        }

        /// <summary>
        /// Ons the scan area y position changed.
        /// </summary>
        /// <param name="d">The d.</param>
        /// <param name="e">The <see cref="DependencyPropertyChangedEventArgs"/> instance containing the event data.</param>
        public static void onScanAreaYPositionChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                if ((d as XYTileDisplay).YPositionChanged != null)
                {
                    (d as XYTileDisplay).YPositionChanged();
                }
                if ((d as XYTileDisplay) != null)
                {
                    (d as XYTileDisplay).OnPropertyChanged("ScanAreaYPosOutOfBounds");
                }
            }
            catch (Exception ex)
            {
                if (ex.Message != string.Empty)
                {
                    //MessageBox.Show(ex.Message, "Field Height changed inappropriately");
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Field Y changed inappropriately. Exception thrown: " + ex.Message);
                }

            }
        }

        /// <summary>
        /// Changes the index of the tiles.
        /// </summary>
        /// <param name="wellIndex">Index of the well.</param>
        /// <param name="tileIndex">Index of the tile.</param>
        public void ChangeTilesIndex(int wellIndex, int tileIndex)
        {
            if (TilesIndexChangedEvent != null)
            {
                TilesIndexChangedEvent(wellIndex, tileIndex);
            }
        }

        /// <summary>
        /// Gets the wells position from tag.
        /// </summary>
        /// <param name="tag">The tag.</param>
        /// <returns></returns>
        public WellPosition GetWellsPositionFromTag(string tag)
        {
            //676( 26*26 A-Z,AA- AZ,...,ZA-ZZ) rows at most
            var chars = tag.ToCharArray();
            int numOfChar = 0;
            int row = 0;
            int column = 0;
            for (int i = chars.Count() - 1; i >= 0; i--)// backwards
            {
                if (!Char.IsNumber(chars[i]))
                {
                    numOfChar++;
                    row += ((int)chars[i] - 65) + (numOfChar - 1) * 26;//A1 -> (0,0), AA ->(26,0)
                }
            }
            column = Convert.ToInt32(tag.Substring(Math.Max(0, numOfChar)));
            WellPosition wellTemplate = new WellPosition(row, column - 1);
            return wellTemplate;
        }

        /// <summary>
        /// Gets the wells tag by point.
        /// </summary>
        /// <param name="point">The point.</param>
        /// <param name="currentCarrier">The current carrier.</param>
        /// <returns></returns>
        public string GetWellsTagByPoint(Point point, Carrier currentCarrier)
        {
            int column = (currentCarrier.Template.CenterToCenterX != 0) ? (int)Math.Round(Math.Abs((point.X - HomePosX)) / currentCarrier.Template.CenterToCenterX) : 0;
            int row = (currentCarrier.Template.CenterToCenterY != 0) ? (int)Math.Round(Math.Abs((point.Y - HomePosY)) / currentCarrier.Template.CenterToCenterY) : 0;
            return TagWells(row, column);
        }

        /// <summary>
        /// Called when [property changed].
        /// </summary>
        /// <param name="propertyName">Name of the property.</param>
        public void OnPropertyChanged(string propertyName)
        {
            this.VerifyPropertyName(propertyName);

            if (Control.PropertyChanged != null)
            {
                Control.PropertyChanged(Control, new PropertyChangedEventArgs(propertyName));
            }
        }

        /// <summary>
        /// Selects the specific experiment file.
        /// </summary>
        /// <param name="expFlie">The exp flie.</param>
        public void SelectExpFile(string expFile)
        {
            //Load the showTileGrid Setting before loading experiment
            try
            {
                XmlDocument appSettings = new XmlDocument();
                XmlDocument expXML = new XmlDocument();
                appSettings.Load(ResourceManagerCS.GetApplicationSettingsFileString());
                expXML.Load(expFile);
                XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");
                var sampleRootNode = expXML.SelectSingleNode("/ThorImageExperiment/Sample");

                if (null != ndList && ndList.Count > 0)
                {
                    string s = string.Empty;
                    XmlManager.GetAttribute(ndList[0], appSettings, "showTileGrid", ref s);
                    this.ShowGrid.IsChecked = ("1" == s || s == bool.TrueString) ? true : false;
                    if (null != sampleRootNode)
                    {
                        if ("1" == s || s == bool.TrueString)
                        {
                            XmlManager.SetAttribute(sampleRootNode, expXML, "DisplayTileGrid", "1");
                        }
                        else
                        {
                            XmlManager.SetAttribute(sampleRootNode, expXML, "DisplayTileGrid", "0");
                        }
                        expXML.Save(expFile);
                    }
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Failed to select experiment file. Exception thrown: " + ex.Message);
            }
            if (ExpPathChanged != null)
            {
                ExpPathChanged(expFile);
            }
        }

        /// </summary>  
        /// <param name="image">image to set opacity on</param>  
        /// <param name="opacity">percentage of opacity</param>  
        /// <returns></returns>  
        public System.Drawing.Bitmap SetImageOpacity(System.Drawing.Bitmap image, double opacity)
        {
            try
            {
                //create a Bitmap the size of the image provided
                System.Drawing.Bitmap bmp = new System.Drawing.Bitmap(image.Width, image.Height);

                //create a graphics object from the image
                using (System.Drawing.Graphics gfx = System.Drawing.Graphics.FromImage(bmp))
                {

                    //create a color matrix object
                    System.Drawing.Imaging.ColorMatrix matrix = new System.Drawing.Imaging.ColorMatrix();

                    //set the opacity
                    matrix.Matrix33 = (float)opacity;

                    //create image attributes
                    System.Drawing.Imaging.ImageAttributes attributes = new System.Drawing.Imaging.ImageAttributes();

                    //set the color(opacity) of the image
                    attributes.SetColorMatrix(matrix, System.Drawing.Imaging.ColorMatrixFlag.Default, System.Drawing.Imaging.ColorAdjustType.Bitmap);

                    //now draw the image
                    gfx.DrawImage(image, new System.Drawing.Rectangle(0, 0, bmp.Width, bmp.Height), 0, 0, image.Width, image.Height, System.Drawing.GraphicsUnit.Pixel, attributes);
                }
                return bmp;
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Failed to set image opacity. Exception thrown: " + ex.Message);
                MessageBox.Show(ex.Message);
                return null;
            }
        }

        /// <summary>
        /// Tags the wells.
        /// </summary>
        /// <param name="row">The row.</param>
        /// <param name="column">The column.</param>
        /// <returns></returns>
        public string TagWells(int row, int column)
        {
            // A1  A2  A3 .....
            // B1  B2  B3 .....
            // .
            // Z1  Z2  Z3 ......
            // AA1 AA2 AA3......
            //.
            // AZ1 AZ2 AZ3......
            string tag = string.Empty;
            //676( 26*26 A-Z,AA- AZ,...,ZA-ZZ) rows at most
            if (row >= 26)
            {
                tag += Convert.ToChar(64 + (int)row / 26);
            }
            tag += Convert.ToChar(65 + row % 26) + (column + 1).ToString();
            return tag;
        }

        /// <summary>
        /// Verifies the name of the property.
        /// </summary>
        /// <param name="propertyName">Name of the property.</param>
        public void VerifyPropertyName(string propertyName)
        {
            if (TypeDescriptor.GetProperties(Control)[propertyName] == null)
            {
                Debug.Fail("Invalid property name: " + propertyName);
            }
        }

        /// <summary>
        /// Load experiment and application file by external triigger
        /// </summary>
        public void XYTileDisplay_Load()
        {
            if (Load != null)
            {
                Load();
            }
        }

        /// <summary>
        /// Handles the Click event of the Next Tile Button.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnNextTile_Click(object sender, RoutedEventArgs e)
        {
            viewModel.MoveToNextTile();
        }

        /// <summary>
        /// Handles the Click event of the Previous Tile Button.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnPreviousTile_Click(object sender, RoutedEventArgs e)
        {
            viewModel.MoveToPreviousTile();
        }

        /// <summary>
        /// Cells the changed, which only affects "isEnable" to avoid double-click issue
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void CellChanged(object sender, RoutedEventArgs e)
        {
            if (sender as DataGridCell != null && (sender as DataGridCell).Column != null && (sender as DataGridCell).Column.Header != null || ((System.Windows.Controls.DataGridCell)(sender)).IsEditing == true)
            {
                viewModel.XYTable_EditItem(XYtableData, XYtable.SelectedIndex, 0, (e.OriginalSource as ToggleButton).IsChecked);
            }
        }

        /// <summary>
        /// Move scan area to homelocation
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void ContainerMenu_GoToHomeLocation_Click(object sender, RoutedEventArgs e)
        {
            double x = HomePosX;
            double y = HomePosY;
            if (x >= MinX && x <= MaxX && y >= MinY && y <= MaxY) // make sure home location is in the correct range
            {
                ScanAreaXPosition = x; // assign homelocationX to ScanareaX
                ScanAreaYPosition = y; // assign homelocationY to ScanareaY
            }
            else
            {
                MessageBox.Show("Position is out of range!\nPlease adjust the home position or change the XY Range in the setting.", "Out Of Range Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
            }
        }

        /// <summary>
        /// Move Scanarea to mouse location.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void ContainerMenu_GoToLocation_Click(object sender, RoutedEventArgs e)
        {
            ScanAreaXPosition = _homeFieldPos.X; // _homeFieldPosX is the location where user right-click contextmenu, which avoids shifting position to contectmenuitem position.
            ScanAreaYPosition = _homeFieldPos.Y; // _homeFieldPosY is the location where user right-click contextmenu, which avoids shifting position to contectmenuitem position.
        }

        /// <summary>
        /// Make the mouse location to homelocation.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void ContainerMenu_MyLocation_Click(object sender, RoutedEventArgs e)
        {
            double x = Math.Round(HomePosX - (_homeFieldPos.X - ScanAreaXPosition), 4);// define the new homelocation. _homeFieldPosX is the location where user right-click contextmenu, which avoids shifting position to contectmenuitem position.
            double y = Math.Round(HomePosY - (_homeFieldPos.Y - ScanAreaYPosition), 4); // define the new homelocation. _homeFieldPosY is the location where user right-click contextmenu, which avoids shifting position to contectmenuitem position.
            if (MessageBox.Show(string.Format("Do you want to change home location to X: {0}, Y: {1}?", x.ToString(), y.ToString()), "Home Location Change Alert", MessageBoxButton.OKCancel, MessageBoxImage.Warning) == MessageBoxResult.OK)
            {
                HomePosX = x;
                HomePosY = y;
                OnPropertyChanged("HomePosX"); // to make sure GUI is update once
            }
        }

        /// <summary>
        /// Handles the Opened event of the ContextMenu control.Mark mouse location where user right-click contextmenu, which avoids shifting position to contectmenuitem position. 
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void ContextMenu_Opened(object sender, RoutedEventArgs e)
        {
            _homeFieldPos = this.SelectModifier.GetMousePosition(); // Get mouse location.

            ContextMenu ContainerContex = (ContextMenu)Control.FindResource("ContainerContextMenu");
            ContextMenu WellContextMenu = (ContextMenu)Control.FindResource("WellContextMenu");
            for (int i = 0; i < ContainerContex.Items.Count; i++)
            {
                MenuItem goToPosition = ContainerContex.Items[i] as MenuItem;
                if (goToPosition != null && goToPosition.Header.ToString() == "Move Here") // "Move here" only shows when the position in the correct XY range
                {
                    if (_homeFieldPos.X > MinX && _homeFieldPos.X < MaxX && _homeFieldPos.Y > MinY && _homeFieldPos.Y < MaxY)
                    {
                        goToPosition.Visibility = Visibility.Visible;
                    }
                    else
                    {
                        goToPosition.Visibility = Visibility.Collapsed;
                    }
                }
            }
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the ControlPanel_Overview control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void ControlPanel_Overview_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            viewModel.ResetSampleView();
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the ControlPanel_ScanAreaView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void ControlPanel_ScanAreaView_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            viewModel.RestScanAreaView();
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the ControlPanel_SelectionEnabled control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void ControlPanel_SelectionEnabled_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (SelectModifier.SelectEnabled == false)
            {
                SelectModifier.SelectEnabled = true;
                SelectModifier.TileEnabled = false;
                ZoomPanModifier.IsEnabled = false;
                TileEnabled.IsChecked = false;
            }
            else
            {
                SelectModifier.SelectEnabled = false;
                ZoomPanModifier.IsEnabled = true;
            }
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the ControlPanel_TileEnabled control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void ControlPanel_TileEnabled_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (SelectModifier.TileEnabled == false)
            {
                SelectModifier.SelectEnabled = false;
                SelectModifier.TileEnabled = true;
                ZoomPanModifier.IsEnabled = false;
                SelectionEnabled.IsChecked = false;
            }
            else
            {
                SelectModifier.TileEnabled = false;
                ZoomPanModifier.IsEnabled = true;
            }
        }

        /// <summary>
        /// Handles the Click event of the CopyTilesToAllWell control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void CopyTilesToAllWell_Click(object sender, RoutedEventArgs e)
        {
            MenuItem menu = sender as MenuItem;
            if (menu != null)
            {
                BoxAnnotation tile = ((ContextMenu)((MenuItem)(menu.Parent)).Parent).PlacementTarget as BoxAnnotation;
                if (CopyTiles != null)
                {
                    CopyTiles(tile.Tag as XYPosition, (int)CopyDst.All);
                }
            }
        }

        /// <summary>
        /// Handles the Click event of the CopyTilesToSelectedWell control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void CopyTilesToSelectedWell_Click(object sender, RoutedEventArgs e)
        {
            MenuItem menu = sender as MenuItem;
            if (menu != null)
            {
                BoxAnnotation tile = ((ContextMenu)((MenuItem)(menu.Parent)).Parent).PlacementTarget as BoxAnnotation;
                if (CopyTiles != null)
                {
                    CopyTiles(tile.Tag as XYPosition, (int)CopyDst.Selected);
                }
            }
        }

        private void fp1Btn_Click(object sender, RoutedEventArgs e)
        {
            FP1XYZ[0] = ScanAreaXPosition;
            FP1XYZ[1] = ScanAreaYPosition;
            FP1XYZ[2] = (double)MVMManager.Instance["ZControlViewModel", "ZPosition"];

            OnPropertyChanged("FocusPoint1");
        }

        private void fp1GoBtn_Click(object sender, RoutedEventArgs e)
        {
            ScanAreaXPosition = FP1XYZ[0];
            ScanAreaYPosition = FP1XYZ[1];
            MVMManager.Instance["ZControlViewModel", "ZPosition"] = FP1XYZ[2];
        }

        private void fp2Btn_Click(object sender, RoutedEventArgs e)
        {
            FP2XYZ[0] = ScanAreaXPosition;
            FP2XYZ[1] = ScanAreaYPosition;
            FP2XYZ[2] = (double)MVMManager.Instance["ZControlViewModel", "ZPosition"];

            OnPropertyChanged("FocusPoint2");
        }

        private void fp2GoBtn_Click(object sender, RoutedEventArgs e)
        {
            ScanAreaXPosition = FP2XYZ[0];
            ScanAreaYPosition = FP2XYZ[1];
            MVMManager.Instance["ZControlViewModel", "ZPosition"] = FP2XYZ[2];
        }

        private void fp3Btn_Click(object sender, RoutedEventArgs e)
        {
            FP3XYZ[0] = ScanAreaXPosition;
            FP3XYZ[1] = ScanAreaYPosition;
            FP3XYZ[2] = (double)MVMManager.Instance["ZControlViewModel", "ZPosition"];

            OnPropertyChanged("FocusPoint3");
        }

        private void fp3GoBtn_Click(object sender, RoutedEventArgs e)
        {
            ScanAreaXPosition = FP3XYZ[0];
            ScanAreaYPosition = FP3XYZ[1];
            MVMManager.Instance["ZControlViewModel", "ZPosition"] = FP3XYZ[2];
        }

        private Cursor GenerateCursor()
        {
            DataGridRow row = (DataGridRow)XYtable.ItemContainerGenerator.ContainerFromIndex(_xyTableDragStartIndex);
            Point o = row.TransformToAncestor(Application.Current.MainWindow).Transform(new Point(0, 0));
            var hp = FindVisualChild<DataGridColumnHeadersPresenter>(XYtable);
            ScrollViewer sv = FindVisualChild<ScrollViewer>(XYtable);
            System.Drawing.Size sz = new System.Drawing.Size((int)XYtable.ActualWidth, (int)(row.ActualHeight));
            System.Drawing.Bitmap bmp = new System.Drawing.Bitmap(sz.Width, sz.Height);
            System.Drawing.Graphics g = System.Drawing.Graphics.FromImage(bmp);
            g.CopyFromScreen((int)(o.X + XYtable.CellsPanelHorizontalOffset + sv.HorizontalOffset), (int)(o.Y + hp.ActualHeight), 0, 0, sz);
            System.Drawing.Bitmap bmp05 = SetImageOpacity(bmp, 0.7);
            Microsoft.Win32.SafeHandles.SafeFileHandle h = new Microsoft.Win32.SafeHandles.SafeFileHandle(bmp05.GetHicon(), true);

            g.Dispose();
            bmp.Dispose();
            bmp05.Dispose();
            return System.Windows.Interop.CursorInteropHelper.Create(h);
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the SelectCarrierBtn control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void SelectCarrierBtn_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            SampleCarrierTemplate dlg = new SampleCarrierTemplate();
            dlg.DataContext = this;
            dlg.ShowDialog();
        }

        /// <summary>
        /// Handles the Click event of the SetHomeCommand control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void SetHomeCommand_Click(object sender, RoutedEventArgs e)
        {
            HomePosX = ScanAreaXPosition;
            HomePosY = ScanAreaYPosition;
            HomePosZ = ScanAreaZPosition;
        }

        /// <summary>
        /// Handles the Click event of the SetXYToTileSize control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void SetXYToTileSize_Click(object sender, RoutedEventArgs e)
        {
            if (null != viewModel)
            {
                StepSizeX = ScanAreaWidth;
                StepSizeY = ScanAreaHeight;
            }
        }

        /// <summary>
        /// Handles the Click event of the SetXYToWellSize control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void SetXYToWellSize_Click(object sender, RoutedEventArgs e)
        {
            if (null != viewModel)
            {
                StepSizeX = CurrentCarrier.Template.CenterToCenterX;
                StepSizeY = CurrentCarrier.Template.CenterToCenterY;
            }
        }

        /// <summary>
        /// Handles the Click event of the SetZeroX control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void SetZeroX_Click(object sender, RoutedEventArgs e)
        {
            if (XSetZero != null)
            {
                XSetZero(this, e);
            }
            HomePosX = HomePosX - ScanAreaXPosition;
        }

        /// <summary>
        /// Handles the Click event of the SetZeroY control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void SetZeroY_Click(object sender, RoutedEventArgs e)
        {
            if (YSetZero != null)
            {
                YSetZero(this, e);
            }
            HomePosY = HomePosY - ScanAreaYPosition;
        }

        /// <summary>
        /// Handles the Click event of the ShowGrid control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void ShowGrid_Click(object sender, RoutedEventArgs e)
        {
            CheckBox showGrid = sender as CheckBox;
            viewModel.ShowTilesGrid(showGrid.IsChecked);
        }

        /// <summary>
        /// Handles the Click event of the SinglePosition control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void SinglePosition_Click(object sender, RoutedEventArgs e)
        {
            if (SinglePositionCheckedStatusChanged != null && (sender as CheckBox).IsChecked.HasValue)
            {
                SinglePositionCheckedStatusChanged((bool)(sender as CheckBox).IsChecked);
            }
        }

        /// <summary>
        /// Handles the Click event of the StepSizeXCoarse control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void StepSizeXCoarse_Click(object sender, RoutedEventArgs e)
        {
            StepSizeX *= 10;
            StepSizeX = Math.Max(MIN_STAGE_STEP, Math.Min(MAX_STAGE_STEP, StepSizeX));
        }

        /// <summary>
        /// Handles the Click event of the StepSizeXFine control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void StepSizeXFine_Click(object sender, RoutedEventArgs e)
        {
            StepSizeX /= 10;
            StepSizeX = Math.Max(MIN_STAGE_STEP, Math.Min(MAX_STAGE_STEP, StepSizeX));
        }

        /// <summary>
        /// Handles the Click event of the StepSizeYCoarse control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void StepSizeYCoarse_Click(object sender, RoutedEventArgs e)
        {
            StepSizeY *= 10;
            StepSizeY = Math.Max(MIN_STAGE_STEP, Math.Min(MAX_STAGE_STEP, StepSizeY));
        }

        /// <summary>
        /// Handles the Click event of the StepSizeYFine control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void StepSizeYFine_Click(object sender, RoutedEventArgs e)
        {
            StepSizeY /= 10;
            StepSizeY = Math.Max(MIN_STAGE_STEP, Math.Min(MAX_STAGE_STEP, StepSizeY));
        }

        /// <summary>
        /// Handles the Click event of the TileContextMenu_DeleteTiles control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void TileContextMenu_DeleteTiles_Click(object sender, RoutedEventArgs e)
        {
            MenuItem menu = sender as MenuItem;
            if (menu != null)
            {
                BoxAnnotation tile = ((ContextMenu)menu.Parent).PlacementTarget as BoxAnnotation;
                XYPosition xyPosition = tile.Tag as XYPosition;
                viewModel.XYTable_DeleteItem(XYtableData, XYtableData.IndexOf(xyPosition));
            }
        }

        /// <summary>
        /// Handles the Click event of the TileContextMenu_DuplicateTiles control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void TileContextMenu_DuplicateTiles_Click(object sender, RoutedEventArgs e)
        {
            MenuItem menu = sender as MenuItem;
            if (menu != null)
            {
                BoxAnnotation tile = ((ContextMenu)menu.Parent).PlacementTarget as BoxAnnotation;
                _duplicatedTiles = tile.Tag as XYPosition;
                SelectModifier.CreateDuplicatedArea(ScanAreaWidth * (1 - Convert.ToDouble(_duplicatedTiles.OverlapX) / 100) * (Convert.ToInt32(_duplicatedTiles.TileCol) - 1) + ScanAreaWidth, ScanAreaHeight + ScanAreaHeight * (Convert.ToInt32(_duplicatedTiles.TileRow) - 1) * (1 - Convert.ToDouble(_duplicatedTiles.OverlapY) / 100));
                SelectModifier.DuplicateEnabled = true;
            }
        }

        /// <summary>
        /// Handles the Click event of the TileContextMenu_GoToTilePosition control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void TileContextMenu_GoToTilePosition_Click(object sender, RoutedEventArgs e)
        {
            MenuItem menu = sender as MenuItem;
            if (menu != null)
            {
                BoxAnnotation tile = ((ContextMenu)menu.Parent).PlacementTarget as BoxAnnotation;
                double x = (Convert.ToDouble(tile.X1) + Convert.ToDouble(tile.X2)) / 2;
                double y = (Convert.ToDouble(tile.Y1) + Convert.ToDouble(tile.Y2)) / 2;

                if (x >= MinX && x <= MaxX && y >= MinY && y <= MaxY)
                {
                    ScanAreaXPosition = x;
                    ScanAreaYPosition = y;
                }
                else
                {
                    MessageBox.Show("Position is out of range!\nPlease adjust the tile position or change the XY Range in the setting.", "Out Of Range Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                }
            }
        }

        /// <summary>
        /// Handles the MoveAsScanAreaCenterClick event of the TileContextMenu control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void TileContextMenu_MoveAsScanAreaCenterClick(object sender, RoutedEventArgs e)
        {
            MenuItem menu = sender as MenuItem;
            if (menu != null)
            {
                BoxAnnotation tile = ((ContextMenu)menu.Parent).PlacementTarget as BoxAnnotation;
                XYPosition xyPosition = tile.Tag as XYPosition;
                if (CenterTiles != null)
                {
                    CenterTiles(xyPosition);
                }
            }
        }

        /// <summary>
        /// Handles the Click event of the TileContextMenu_MoveToCenter control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void TileContextMenu_MoveToCenter_Click(object sender, RoutedEventArgs e)
        {
            MenuItem menu = sender as MenuItem;
            if (menu != null)
            {
                BoxAnnotation tile = ((ContextMenu)menu.Parent).PlacementTarget as BoxAnnotation;
                XYPosition xyPosition = tile.Tag as XYPosition;
                if (TilesPositionChanged != null)
                {
                    TilesPositionChanged(xyPosition);
                }
            }
        }

        /// <summary>
        /// Handles the Opened event of the TileContextMenu control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void TileContextMenu_Opened(object sender, RoutedEventArgs e)
        {
            ContextMenu contextMenu = sender as ContextMenu;

            for (int i = 0; i < contextMenu.Items.Count; i++)
            {
                MenuItem moveToCenter = contextMenu.Items[i] as MenuItem;
                if (moveToCenter != null && moveToCenter.Header.ToString() == "Move To Center")
                {
                    if (SampleCarrierCollection[SelectedCarrierIndex].Type != CarrierType.Slide)
                    {
                        moveToCenter.Visibility = Visibility.Visible;
                    }
                    else
                    {
                        moveToCenter.Visibility = Visibility.Collapsed;
                    }
                }

                MenuItem CopyTiles = contextMenu.Items[i] as MenuItem;
                if (CopyTiles != null && CopyTiles.Header.ToString() == "Copy Tiled Area to")
                {
                    if (SampleCarrierCollection[SelectedCarrierIndex].Type != CarrierType.Slide)
                    {
                        CopyTiles.Visibility = Visibility.Visible;
                    }
                    else
                    {
                        CopyTiles.Visibility = Visibility.Collapsed;
                    }
                }
            }
        }

        /// <summary>
        /// Handles the Click event of the ContextMenu_DeselectWell control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void WellContextMenu_DeselectAll_Click(object sender, RoutedEventArgs e)
        {
            MenuItem menu = sender as MenuItem;
            if (DeselectAllWellsEvent != null)
            {
                DeselectAllWellsEvent();
            }
        }

        /// <summary>
        /// Handles the Click event of the ContextMenu_DeselectWell control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void WellContextMenu_DeselectWell_Click(object sender, RoutedEventArgs e)
        {
            MenuItem menu = sender as MenuItem;
            if (WellSelectedStatusChanged != null)
            {
                WellSelectedStatusChanged(menu, false);
            }
        }

        /// <summary>
        /// Handles the Click event of the ContextMenu_GoToCenter control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void WellContextMenu_GoToCenter_Click(object sender, RoutedEventArgs e)
        {
            MenuItem menu = sender as MenuItem;
            if (menu != null)
            {
                BoxAnnotation tile = ((ContextMenu)menu.Parent).PlacementTarget as BoxAnnotation;
                ScanAreaXPosition = (Convert.ToDouble(tile.X1) + Convert.ToDouble(tile.X2)) / 2;
                ScanAreaYPosition = (Convert.ToDouble(tile.Y1) + Convert.ToDouble(tile.Y2)) / 2;
            }
        }

        /// <summary>
        /// Handles the Click event of the ContextMenu_SelectWell control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void WellContextMenu_SelectWell_Click(object sender, RoutedEventArgs e)
        {
            MenuItem menu = sender as MenuItem;
            if (WellSelectedStatusChanged != null)
            {
                WellSelectedStatusChanged(menu, true);
            }
        }

        /// <summary>
        /// Handles the Click event of the XPosMinus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void XPosMinus_Click(object sender, RoutedEventArgs e)
        {
            ScanAreaXPosition -= StepSizeX;
        }

        /// <summary>
        /// Handles the Click event of the XPosPlus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void XPosPlus_Click(object sender, RoutedEventArgs e)
        {
            ScanAreaXPosition += StepSizeX;
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the XYTableAddPosition control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void XYTableAddPosition_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            viewModel.XYTable_AddItem(XYtableData, new XYPosition(), -1, true);
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the XYTableClearAllPositions control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void XYTableClearAllPositions_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (XYtable.Items.Count > 0)
            {
                for (int j = 0; j < XYtable.Items.Count; j++)
                {
                    viewModel.XYTable_DeleteItem(XYtableData, j);
                    j--;
                }
            }
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the XYTableDeletePosition control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void XYTableDeletePosition_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (XYtable.SelectedIndex >= 0)
            {
                viewModel.XYTable_DeleteItem(XYtableData, XYtable.SelectedIndex);
            }
        }

        private void XYTableGotoPosition_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (XYtable.SelectedIndex >= 0)
            {
                _xyTableSelectedIndex = XYtable.SelectedIndex;
                viewModel.XYTable_SelectItem(XYtableData, _xyTableSelectedIndex);
                double x = Convert.ToDouble(XYtableData[_xyTableSelectedIndex].X);
                double y = Convert.ToDouble(XYtableData[_xyTableSelectedIndex].Y);

                x = x + HomePosX;
                y = y + HomePosY;

                if (x >= MinX && x <= MaxX && y >= MinY && y <= MaxY)
                {
                    ScanAreaXPosition = x;
                    ScanAreaYPosition = y;
                }
                else
                {
                    MessageBox.Show("Position is out of range!\nPlease adjust the tile position or change the XY Range in the setting.", "Out Of Range Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                }

            }
        }

        /// <summary>
        /// Handles the CellEditEnding event of the XYtable control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="DataGridCellEditEndingEventArgs"/> instance containing the event data.</param>
        private void XYtable_CellEditEnding(object sender, DataGridCellEditEndingEventArgs e)
        {
            if (e.Column.DisplayIndex != 0)
            {
                _xyTableSelectedIndex = XYtable.SelectedIndex;
                if (((TextBox)e.EditingElement).Text == String.Empty)
                {
                    XYPosition xyposition = XYtableData[_xyTableSelectedIndex];
                    viewModel.XYTable_DeleteItem(XYtableData, _xyTableSelectedIndex);
                    viewModel.XYTable_AddItem(XYtableData, xyposition, _xyTableSelectedIndex);
                }
                else
                {
                    viewModel.XYTable_EditItem(XYtableData, XYtable.SelectedIndex, e.Column.DisplayIndex, ((TextBox)e.EditingElement).Text);
                }
            }
        }

        /// <summary>
        /// Handles the DoubleClick event of the XYtable control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void XYtable_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (XYtable.SelectedIndex >= 0)
            {
                _xyTableSelectedIndex = XYtable.SelectedIndex;
                viewModel.XYTable_SelectItem(XYtableData, _xyTableSelectedIndex);
            }
        }

        /// <summary>
        /// Handles the MouseLeftButtonUp event of the XYtable control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void XYTable_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            //only allow row manipulation when in edit mode
            if (Mode != TileDisplayMode.Edit) return;
            if (XYtable.SelectedIndex >= 0 && _xyTableIsDrag)
            {
                _xyTableIsDrag = false;

                if (_xyTableDragStartIndex >= 0 && _xyTableDragStartIndex != XYtable.SelectedIndex)
                {
                    if (_xyTableDragStartIndex > XYtable.SelectedIndex)
                    {
                        var it = XYtableData[_xyTableDragStartIndex];
                        XYtableData.RemoveAt(_xyTableDragStartIndex);
                        XYtableData.Insert(XYtable.SelectedIndex, it);
                    }
                    else if (_xyTableDragStartIndex < XYtable.SelectedIndex)
                    {
                        var it = XYtableData[_xyTableDragStartIndex];
                        XYtableData.Insert(XYtable.SelectedIndex + 1, it);
                        XYtableData.RemoveAt(_xyTableDragStartIndex);
                    }
                }
                XYtable.Cursor = _xyTableOldCur;
                _xyTableOldCur = null;
            }
        }

        /// <summary>
        /// Handles the MouseMove event of the XYtable control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void XYTable_MouseMove(object sender, MouseEventArgs e)
        {
            //only allow row manipulation when in edit mode
            if (Mode != TileDisplayMode.Edit) return;
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                if (XYtable.SelectedIndex >= 0 && (!_xyTableIsDrag))
                {
                    _xyTableIsDrag = true;

                    _xyTableOldCur = XYtable.Cursor;
                    _xyTableDragStartIndex = XYtable.SelectedIndex;
                    XYtable.Cursor = GenerateCursor();
                }
            }
            else
            {
                _xyTableIsDrag = false;

                XYtable.Cursor = _xyTableOldCur;

            }
        }

        /// <summary>
        /// Handles the Loaded event of the XYTileDisplay control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void XYTileDisplay_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {
                //set the application settings to default value when the view is loaded,
                //not honor modalities for global carrier settings:
                this.ApplicationSettings = Application.Current.Resources["ApplicationSettingsFile"].ToString();
            
                Creator creator = new Creator(Control);
                viewModel = creator.FactoryMethod(Mode);
                if (null != viewModel && null != Control)
                {
                    viewModel.XYtileViewModel_Loaded();
                }
            }
            catch(Exception ex)
            {
                ex.ToString();
            }
        }

        /// <summary>
        /// Handles the Unloaded event of the XYTileDisplay control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void XYTileDisplay_Unloaded(object sender, RoutedEventArgs e)
        {
            if (null != viewModel)
            {
                viewModel.XYtileViewModel_Unloaded();
            }
            sciChart.Annotations.Clear();
            //if (null != XYtableData)
            //{
            //    XYtableData.Clear();
            //}
        }

        /// <summary>
        /// Handles the Click event of the YPosMinus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void YPosMinus_Click(object sender, RoutedEventArgs e)
        {
            ScanAreaYPosition -= StepSizeY;
        }

        /// <summary>
        /// Handles the Click event of the YPosPlus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void YPosPlus_Click(object sender, RoutedEventArgs e)
        {
            ScanAreaYPosition += StepSizeY;
        }

        #endregion Methods
    }
}