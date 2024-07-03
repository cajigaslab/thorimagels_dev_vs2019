namespace ImageViewControl
{
    using System;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Interop;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using System.Windows.Threading;

    using HistogramControl.ViewModel;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for ImageView.xaml
    /// </summary>
    public partial class ImageView : UserControl
    {
        #region Fields

        public static DependencyProperty BitmapReadyProperty =
        DependencyProperty.Register("BitmapReady",
        typeof(bool),
        typeof(ImageView));
        public static DependencyProperty HistogramViewModelsProperty =
          DependencyProperty.Register("HistogramViewModels",
          typeof(ObservableCollection<ObservableCollection<HistogramControlViewModel>>),
          typeof(ImageView));
        public static DependencyProperty ImageCanvasProperty =
        DependencyProperty.Register("ImageCanvas",
        typeof(Canvas),
        typeof(ImageView));
        public static DependencyProperty ImageOffsetXProperty =
        DependencyProperty.Register("ImageOffsetX",
        typeof(double),
        typeof(ImageView),
                new FrameworkPropertyMetadata(
                new PropertyChangedCallback(OnXYImageOffsetChanged)));
        public static DependencyProperty ImageOffsetYProperty =
        DependencyProperty.Register("ImageOffsetY",
        typeof(double),
        typeof(ImageView),
                new FrameworkPropertyMetadata(
                new PropertyChangedCallback(OnXYImageOffsetChanged)));
        public static DependencyProperty ImageResetSizeCommandProperty =
           DependencyProperty.Register("ImageResetSizeCommand",
           typeof(ICommand),
           typeof(ImageView));
        public static DependencyProperty ImageSaveAsCommandProperty =
          DependencyProperty.Register("ImageSaveAsCommand",
          typeof(ICommand),
          typeof(ImageView));
        public static DependencyProperty ImageSaveAsReferenceCommandProperty =
         DependencyProperty.Register("ImageSaveAsReferenceCommand",
         typeof(ICommand),
         typeof(ImageView));
        public static DependencyProperty ImageSetToFullSizeCommandProperty =
        DependencyProperty.Register("ImageSetToFullSizeCommand",
        typeof(ICommand),
        typeof(ImageView));
        public static DependencyProperty IsOrthogonalViewCheckedProperty =
        DependencyProperty.Register("IsOrthogonalViewChecked",
        typeof(bool),
        typeof(ImageView));
        public static DependencyProperty IVScrollBarHeightProperty =
        DependencyProperty.Register("IVScrollBarHeight",
        typeof(double),
        typeof(ImageView));
        public static DependencyProperty LSMChannelProperty =
        DependencyProperty.Register("LSMChannel",
        typeof(int),
        typeof(ImageView));
        public static DependencyProperty OrthogonalChangeCountProperty =
        DependencyProperty.Register("OrthogonalChangeCount",
        typeof(long),
        typeof(ImageView),
               new FrameworkPropertyMetadata(
               new PropertyChangedCallback(OnOrthogonalChangeCountChanged)));
        public static DependencyProperty OrthogonalDisplayOptionsCommandProperty =
        DependencyProperty.Register("OrthogonalDisplayOptionsCommand",
        typeof(ICommand),
        typeof(ImageView));
        public static DependencyProperty OrthogonalLineColorTypeProperty =
        DependencyProperty.Register("OrthogonalLineColorType",
        typeof(int),
        typeof(ImageView));
        public static DependencyProperty OrthogonalLinesLockPositionCommandProperty =
        DependencyProperty.Register("OrthogonalLinesLockPositionCommand",
        typeof(ICommand),
        typeof(ImageView));
        public static DependencyProperty OrthogonalLineTypeProperty =
        DependencyProperty.Register("OrthogonalLineType",
        typeof(int),
        typeof(ImageView));
        public static DependencyProperty OrthogonalViewPositionProperty =
         DependencyProperty.Register("OrthogonalViewPosition",
         typeof(Point),
         typeof(ImageView));
        public static DependencyProperty RollOverPointXProperty =
        DependencyProperty.Register("RollOverPointX",
        typeof(int),
        typeof(ImageView));
        public static DependencyProperty RollOverPointYProperty =
        DependencyProperty.Register("RollOverPointY",
        typeof(int),
        typeof(ImageView));
        public static DependencyProperty ZNumStepsProperty =
        DependencyProperty.Register("ZNumSteps",
        typeof(int),
        typeof(ImageView));
        public static DependencyProperty ZoomLevelProperty =
        DependencyProperty.Register("ZoomLevel",
        typeof(double),
        typeof(ImageView),
                new FrameworkPropertyMetadata(
                new PropertyChangedCallback(OnZoomLevelChanged)));
        public static DependencyProperty HelpTextProperty = 
            DependencyProperty.Register("HelpText",
                typeof(string),
                typeof(ImageView));
        public static DependencyProperty HelpTextVisibilityProperty =
            DependencyProperty.Register("HelpTextVisibility",
                typeof(Visibility),
                typeof(ImageView));

        private Point lockPoint;
        Matrix m = new Matrix();
        bool mouseDown = false;
        DispatcherTimer timerToUpdateOverlays = new DispatcherTimer();
        private BackgroundWorker _bleachROIChecker;
        private SolidColorBrush[] _brushColors = new SolidColorBrush[] { Brushes.White, Brushes.Black, Brushes.Green, Brushes.Blue, Brushes.Yellow, Brushes.Red };
        private double _currentScale;
        Point _lastPosOverlay;
        Point _newOrigin;
        Point _newStart;
        bool _nonVitualStackObjectMovingFlag = false;
        private Line[] _orthogonalViewLine = new Line[2];
        private Matrix _orthogonalViewMatrix;
        private Line[] _orthogonalXZViewLine = new Line[1];
        private Line[] _orthogonalYZViewLine = new Line[1];
        bool _roiToolMouseDoubleClickFlag = false;
        private ScaleTransform _scaleTransform;
        private Point _scrollStartPoint;
        private bool _shiftDown;
        private TransformGroup _transformGroup;
        private TranslateTransform _translateTransform;
        private bool _updatingOverlayObject;

        #endregion Fields

        #region Constructors

        public ImageView()
        {
            InitializeComponent();

            if (DesignerProperties.GetIsInDesignMode(this) == true)
                return;

            zoomSlider.Value = _currentScale;

            _translateTransform = new TranslateTransform();
            _scaleTransform = new ScaleTransform();
            _transformGroup = new TransformGroup();
            _bleachROIChecker = new BackgroundWorker();
            _bleachROIChecker.WorkerSupportsCancellation = true;

            _transformGroup.Children.Add(_scaleTransform);
            _transformGroup.Children.Add(_translateTransform);

            imageCanvas.RenderTransform = _transformGroup;
            xyOrthogonalImageCanvas.RenderTransform = _transformGroup;

            _currentScale = 1.0;
            zoomSlider.Value = 1.0;

            _updatingOverlayObject = false;
            _shiftDown = false;

            Loaded += new RoutedEventHandler(ImageView_Loaded);
            Unloaded += new RoutedEventHandler(ImageView_Unloaded);
            timerToUpdateOverlays.Tick += Timer_Tick;
            timerToUpdateOverlays.Interval = new TimeSpan(0, 0, 0, 0, 50);
            imageGrid.Items.CurrentChanged += Items_CurrentChanged;
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Enumerations

        enum LineDirection
        {
            horizontal,
            vertical,
            horizontalAndVertical
        }

        #endregion Enumerations

        #region Properties

        public bool BitmapReady
        {
            get { return (bool)GetValue(BitmapReadyProperty); }
            set { SetValue(BitmapReadyProperty, value); }
        }

        public string HelpText
        {
            get { return (string)GetValue(HelpTextProperty); }
            set { SetValue(HelpTextProperty, value); }
        }

        public Visibility HelpTextVisibility
        {
            get { return (Visibility)GetValue(HelpTextVisibilityProperty); }
            set { SetValue(HelpTextVisibilityProperty, value); }
        }

        public ObservableCollection<ObservableCollection<HistogramControlViewModel>> HistogramViewModels
        {
            get { return (ObservableCollection<ObservableCollection<HistogramControlViewModel>>)GetValue(HistogramViewModelsProperty); }
            set { SetValue(HistogramViewModelsProperty, value); }
        }

        public Canvas ImageCanvas
        {
            get { return (Canvas)GetValue(ImageCanvasProperty); }
            set { SetValue(ImageCanvasProperty, value); }
        }

        public double ImageOffsetX
        {
            get { return (double)GetValue(ImageOffsetXProperty); }
            set { SetValue(ImageOffsetXProperty, value); }
        }

        public double ImageOffsetY
        {
            get { return (double)GetValue(ImageOffsetYProperty); }
            set { SetValue(ImageOffsetYProperty, value); }
        }

        public ICommand ImageResetSizeCommand
        {
            get { return (ICommand)GetValue(ImageResetSizeCommandProperty); }
            set { SetValue(ImageResetSizeCommandProperty, value); }
        }

        public ICommand ImageSaveAsCommand
        {
            get { return (ICommand)GetValue(ImageSaveAsCommandProperty); }
            set { SetValue(ImageSaveAsCommandProperty, value); }
        }

        public ICommand ImageSaveAsReferenceCommand
        {
            get { return (ICommand)GetValue(ImageSaveAsReferenceCommandProperty); }
            set { SetValue(ImageSaveAsReferenceCommandProperty, value); }
        }

        public ICommand ImageSetToFullSizeCommand
        {
            get { return (ICommand)GetValue(ImageSetToFullSizeCommandProperty); }
            set { SetValue(ImageSetToFullSizeCommandProperty, value); }
        }

        public bool IsOrthogonalViewChecked
        {
            get { return (bool)GetValue(IsOrthogonalViewCheckedProperty); }
            set { SetValue(IsOrthogonalViewCheckedProperty, value); }
        }

        public double IVScrollBarHeight
        {
            get { return (double)GetValue(IVScrollBarHeightProperty); }
            set { SetValue(IVScrollBarHeightProperty, value); }
        }

        public int LSMChannel
        {
            get { return (int)GetValue(LSMChannelProperty); }
            set { SetValue(LSMChannelProperty, value); }
        }

        public long OrthogonalChangeCount
        {
            get { return (long)GetValue(OrthogonalChangeCountProperty); }
            set { SetValue(OrthogonalChangeCountProperty, value); }
        }

        public ICommand OrthogonalDisplayOptionsCommand
        {
            get { return (ICommand)GetValue(OrthogonalDisplayOptionsCommandProperty); }
            set { SetValue(OrthogonalDisplayOptionsCommandProperty, value); }
        }

        public int OrthogonalLineColorType
        {
            get { return (int)GetValue(OrthogonalLineColorTypeProperty); }
            set { SetValue(OrthogonalLineColorTypeProperty, value); }
        }

        public ICommand OrthogonalLinesLockPositionCommand
        {
            get { return (ICommand)GetValue(OrthogonalLinesLockPositionCommandProperty); }
            set { SetValue(OrthogonalLinesLockPositionCommandProperty, value); }
        }

        public int OrthogonalLineType
        {
            get { return (int)GetValue(OrthogonalLineTypeProperty); }
            set { SetValue(OrthogonalLineTypeProperty, value); }
        }

        public Point OrthogonalViewPosition
        {
            get { return (Point)GetValue(OrthogonalViewPositionProperty); }
            set { SetValue(OrthogonalViewPositionProperty, value); }
        }

        public int RollOverPointX
        {
            get { return (int)GetValue(RollOverPointXProperty); }
            set { SetValue(RollOverPointXProperty, value); }
        }

        public int RollOverPointY
        {
            get { return (int)GetValue(RollOverPointYProperty); }
            set { SetValue(RollOverPointYProperty, value); }
        }

        public int ZNumSteps
        {
            get { return (int)GetValue(ZNumStepsProperty); }
            set { SetValue(ZNumStepsProperty, value); }
        }

        public double ZoomLevel
        {
            get { return (double)GetValue(ZoomLevelProperty); }
            set { SetValue(ZoomLevelProperty, value); }
        }

        #endregion Properties

        #region Methods

        public static void OnOrthogonalChangeCountChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                (d as ImageView).UpdateOrthogonalView();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, ex.Message);
            }
        }

        public static void OnXYImageOffsetChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                (d as ImageView).UpdateXYOffsetTransform();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, ex.Message);
            }
        }

        public static void OnZoomLevelChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                (d as ImageView).ZoomDisplay();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, ex.Message);
            }
        }

        public void ImageView_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.LeftShift || e.Key == Key.RightShift)
            {
                _shiftDown = true;
            }
            else if (e.Key == Key.Delete || e.Key == Key.Back)
            {
                if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
                {
                    ROIDrawingTools.SelectedIndex = 0;
                    return;
                }
                OverlayManagerClass.Instance.DeleteSelectedROIs();
                if (0 == OverlayManagerClass.Instance.ROICount)
                {
                    OverlayManagerClass.Instance.ClearAllObjects();
                }
            }
            else if (e.Key == Key.A && (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
            {
                ROIDrawingTools.SelectedIndex = 0;
                OverlayManagerClass.Instance.InitSelectROI();
                if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
                {
                    return;
                }
                OverlayManagerClass.Instance.SelectAllROIs();
            }
            else if (e.Key == Key.Tab) 
            {
                if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
                {
                    return;
                }
                ROIDrawingTools.SelectedIndex = 0;

                if ((Keyboard.Modifiers & ModifierKeys.Shift) == ModifierKeys.Shift)
                {
                    OverlayManagerClass.Instance.SelectPrevROI();
                }
                else
                {
                    OverlayManagerClass.Instance.SelectNextROI();
                }

                e.Handled = true;

            }

            else if (e.Key == Key.Enter || (e.Key == Key.Return))
            {
                if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
                {
                    return;
                }

                e.Handled = true;

                TraversalRequest trNext = new TraversalRequest(FocusNavigationDirection.Next);

                UIElement keyFocus = (UIElement)Keyboard.FocusedElement;

                //move the focus to the next element:
                if ((keyFocus != null) && (keyFocus.GetType() == typeof(TextBox)))
                {
                    keyFocus.MoveFocus(trNext);
                }

            }
        }

        public void ImageView_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.LeftShift || e.Key == Key.RightShift)
            {
                _shiftDown = false;
            }
        }

        public void SetSelectROI()
        {
            ROIDrawingTools.SelectedIndex = 0;
        }

        /// <summary>
        /// Get position and CaptureMouse
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            if (IsMouseOver && false == _updatingOverlayObject)
            {
                _scrollStartPoint = TranslatePoint(e.GetPosition(this), imageCanvas);
                if (0 < imageGrid.Items?.Count)
                {
                    _scrollStartPoint.X = Math.Max(0, Math.Min(_scrollStartPoint.X, imageGrid.ActualWidth - 0.001));
                    _scrollStartPoint.Y = Math.Max(0, Math.Min(_scrollStartPoint.Y, imageGrid.ActualHeight - 0.001));
                }
                if (e.ClickCount == 1)  // single click
                {

                    if (e.ChangedButton == MouseButton.Left)   // left click
                    {
                        IVScrollBarHeight = (0 < imageGrid.Items?.Count && null != _translateTransform) ? ((imageGrid.ActualHeight + 0.001) * _currentScale + _translateTransform.Y) : IVScrollBarHeight;
                    }

                }
                _newStart = e.GetPosition(this);
                _newOrigin.X = imageCanvas.RenderTransform.Value.OffsetX;
                _newOrigin.Y = imageCanvas.RenderTransform.Value.OffsetY;
                CaptureMouse();
            }

            base.OnMouseDown(e);
        }

        /// <summary>
        /// If IsMouseCaptured scroll to correct position. 
        /// Where position is updated by animation timer
        /// </summary>
        protected override void OnMouseMove(MouseEventArgs e)
        {
            if (IsMouseCaptured)
            {
                if (false == _updatingOverlayObject)
                {
                    Point pp = new Point();
                    pp = e.MouseDevice.GetPosition(this);

                    Matrix m = imageCanvas.RenderTransform.Value;
                    m.OffsetX = _newOrigin.X + (pp.X - _newStart.X);
                    m.OffsetY = _newOrigin.Y + (pp.Y - _newStart.Y);

                    imageCanvas.RenderTransform = new MatrixTransform(m);
                    xyOrthogonalImageCanvas.RenderTransform = new MatrixTransform(m);
                    if (true == OrthogonalViewEnable.IsChecked)
                    {
                        m.OffsetY += _currentScale * imageGrid.ActualHeight + 10;
                        xzOrthogonalImageCanvas.RenderTransform = new MatrixTransform(m);

                        m.OffsetY -= _currentScale * imageGrid.ActualHeight + 10;
                        m.OffsetX += _currentScale * imageGrid.ActualWidth + 10;
                        yzOrthogonalImageCanvas.RenderTransform = new MatrixTransform(m);
                        if (pp != _newStart)
                        {
                            _nonVitualStackObjectMovingFlag = true;
                        }
                    }
                }
            }

            if (true == OrthogonalViewEnable.IsChecked && IsMouseCaptured == false && 0 < imageGrid.Items?.Count && imageCanvas.ContextMenu.IsOpen == false && lockPosition.IsChecked == false)
            {
                if (ROIToolbarEnable.IsChecked == false || (ROIToolbarEnable.IsChecked == true && ROIDrawingTools.SelectedIndex == 0))
                {
                    Point startPoint;
                    startPoint = TranslatePoint(e.GetPosition(this), imageCanvas);
                    if (startPoint.X < imageGrid.ActualWidth && startPoint.Y < imageGrid.ActualHeight)
                    {
                        startPoint.X = Math.Max(0, Math.Min(startPoint.X, imageGrid.ActualWidth - 0.001));
                        startPoint.Y = Math.Max(0, Math.Min(startPoint.Y, imageGrid.ActualHeight - 0.001));

                        OrthogonalViewPosition = startPoint;
                        UpdateOrthogonalView(startPoint);
                    }
                }
            }

            Point imagePixelLocation = TranslatePoint(e.GetPosition(this), imageCanvas);

            UpdateRollOverPixelData(imagePixelLocation);

            base.OnMouseMove(e);
        }

        /// <summary>
        /// Release MouseCapture if its captured
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseUp(MouseButtonEventArgs e)
        {
            if (IsMouseCaptured)
            {

                if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
                {
                    ReleaseMouseCapture();
                    imageCanvas.ContextMenu.Visibility = Visibility.Collapsed;
                    return;
                }
                else
                {
                    imageCanvas.ContextMenu.Visibility = Visibility.Visible;
                }

                if ((e.ChangedButton == MouseButton.Left) && (0 < imageGrid.Items?.Count))
                {

                    if (IsOrthogonalViewChecked && !_nonVitualStackObjectMovingFlag)
                    {
                        if (ROIToolbarEnable.IsChecked == false || (ROIToolbarEnable.IsChecked == true && ROIDrawingTools.SelectedIndex == 0))
                        {
                            Point startPoint;
                            startPoint = TranslatePoint(e.GetPosition(this), imageCanvas);
                            if (startPoint.X < imageGrid.ActualWidth && startPoint.Y < imageGrid.ActualHeight)
                            {
                                startPoint.X = Math.Max(0, Math.Min(startPoint.X, imageGrid.ActualWidth - 0.001));
                                startPoint.Y = Math.Max(0, Math.Min(startPoint.Y, imageGrid.ActualHeight - 0.001));
                                OrthogonalViewPosition = startPoint;
                            }
                        }
                    }

                    _nonVitualStackObjectMovingFlag = false;
                }

                if (e.ChangedButton == MouseButton.Right)
                {
                    lockPoint = TranslatePoint(e.GetPosition(this), imageCanvas);
                    lockPoint.X = Math.Max(0, Math.Min(lockPoint.X, imageGrid.ActualWidth - 0.001));
                    lockPoint.Y = Math.Max(0, Math.Min(lockPoint.Y, imageGrid.ActualHeight - 0.001));
                }

                IVScrollBarHeight = (0 < imageGrid.Items?.Count && null != _translateTransform) ? ((imageGrid.ActualHeight + 0.001) * _currentScale + _translateTransform.Y) : IVScrollBarHeight;

                Cursor = Cursors.Arrow;

                ReleaseMouseCapture();
            }
            if (mouseDown)
            {
                mouseDown = false;
                OverlayManagerClass.MouseEventEnum me = OverlayManagerClass.MouseEventEnum.LEFTMOUSEUP;
                OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), _lastPosOverlay, _shiftDown);
            }
            base.OnMouseUp(e);
        }

        /// <summary>
        /// Release MouseCapture if its captured
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseWheel(MouseWheelEventArgs e)
        {
            if (e == null) return;

            Point p = new Point();
            p = e.MouseDevice.GetPosition(imageCanvas);

            m = imageCanvas.RenderTransform.Value;

            double nextScale = _currentScale;
            if (e.Delta > 0)
            {
                nextScale *= 1.1;
                if (nextScale > zoomSlider.Maximum)
                {
                    if (_currentScale <= zoomSlider.Maximum)
                    {
                        nextScale = zoomSlider.Maximum;
                    }
                    else
                    {
                        return;
                    }
                }
            }
            else
            {
                nextScale /= 1.1;
                if (nextScale < zoomSlider.Minimum)
                {
                    if (_currentScale >= zoomSlider.Minimum)
                    {
                        nextScale = zoomSlider.Minimum;
                    }
                    else
                    {
                        return;
                    }
                }
            }
            double scaleFactor = nextScale / _currentScale;
            m.ScaleAtPrepend(scaleFactor, scaleFactor, p.X, p.Y);
            _currentScale = nextScale;

            imageCanvas.RenderTransform = new MatrixTransform(m);
            xyOrthogonalImageCanvas.RenderTransform = new MatrixTransform(m);
            zoomSlider.Value = m.M22;

            if (true == OrthogonalViewEnable.IsChecked && lockPosition.IsChecked)
            {
                Point startPoint = OrthogonalViewPosition;
                OrthogonalViewPosition = startPoint;
                UpdateOrthogonalView(startPoint);
            }
            else if (true == OrthogonalViewEnable.IsChecked)
            {
                Point startPoint = TranslatePoint(e.GetPosition(this), imageCanvas);

                startPoint.X = Math.Max(0, Math.Min(startPoint.X, imageGrid.ActualWidth - 0.001));
                startPoint.Y = Math.Max(0, Math.Min(startPoint.Y, imageGrid.ActualHeight - 0.001));
                OrthogonalViewPosition = startPoint;
                UpdateOrthogonalView(startPoint);
            }

            e.Handled = true;

            IVScrollBarHeight = (0 < imageGrid.Items?.Count && null != _translateTransform) ? ((imageGrid.ActualHeight + 0.001) * _currentScale + _translateTransform.Y) : IVScrollBarHeight;
        }

        private void ActivateDeactivateOrthogonalView()
        {
            if (0 < imageGrid.Items?.Count)
            {
                OrthogonalViewPosition = new Point(imageGrid.ActualWidth / 2, imageGrid.ActualHeight / 2); //initiation with middle point
            }

            if (false == OrthogonalViewEnable.IsChecked)
            {
                vm_CloseOrthogonalView();
            }
            else
            {
                lockPosition.Visibility = Visibility.Visible;  //hide the lock position option
                xzOrthogonalImageCanvas.Visibility = Visibility.Visible; //hide the xzImage and yzImage
                yzOrthogonalImageCanvas.Visibility = Visibility.Visible;
            }
        }

        string ConvertScaleToPercent(double scale)
        {
            Decimal dec = new Decimal(scale * 100);
            string str = Decimal.Round(dec, 0).ToString() + "%";

            return str;
        }

        private void createCrosshairROI_Click(object sender, RoutedEventArgs e)
        {
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROICrosshair();
        }

        private void createEllipseROI_Click(object sender, RoutedEventArgs e)
        {
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROIEllipse();
        }

        private void createLineROI_Click(object sender, RoutedEventArgs e)
        {
            if (true == _roiToolMouseDoubleClickFlag)
            {
                _roiToolMouseDoubleClickFlag = false;
                return;
            }
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROILine();
        }

        private void createLineROI_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            _roiToolMouseDoubleClickFlag = true;
            OverlayManagerClass.Instance.InitROILineWithOptions();
        }

        private void createPolylineROI_Click(object sender, RoutedEventArgs e)
        {
            if (true == _roiToolMouseDoubleClickFlag)
            {
                _roiToolMouseDoubleClickFlag = false;
                return;
            }
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROIPolyline();
        }

        private void createPolylineROI_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            _roiToolMouseDoubleClickFlag = true;
            OverlayManagerClass.Instance.InitROIPolylineWithOptions();
        }

        private void createPolyROI_Click(object sender, RoutedEventArgs e)
        {
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROIPoly();
        }

        private void createRectROI_Click(object sender, RoutedEventArgs e)
        {
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROIRect();
        }

        private void DeleteSelectedROIs(object sender, RoutedEventArgs e)
        {
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                ROIDrawingTools.SelectedIndex = 0;
                return;
            }
            OverlayManagerClass.Instance.DeleteSelectedROIs();
            ROIDrawingTools.SelectedIndex = 0;
            OverlayManagerClass.Instance.InitSelectROI();
            if (0 == OverlayManagerClass.Instance.ROICount)
            {
                //Force reset ROI count
                OverlayManagerClass.Instance.ClearAllObjects();
            }
        }

        void ImageView_Loaded(object sender, RoutedEventArgs e)
        {
            if (System.Environment.OSVersion.Version.Major <= 5)
            {
                //WinXP
                //force the control into software rendering mode
                //there is a memory leak in the .net 3.51 version
                //*TODO* remove when new frame is used and memory leak is resolved

                HwndSource hwndSource = PresentationSource.FromVisual(this) as HwndSource;

                HwndTarget hwndTarget = hwndSource.CompositionTarget;

                // this is the new WPF API to force render mode.

                hwndTarget.RenderMode = RenderMode.SoftwareOnly;
            }

            OverlayManagerClass.Instance.UpdatingObjectEvent += VM_UpdatingObject;

            SizeChanged += new SizeChangedEventHandler(ImageView_SizeChanged);

            toolbarGrid.Width = ActualWidth;
            toolbarGrid.Height = ActualHeight;
            helpGrid.Width = ActualWidth;
            helpGrid.Height = ActualHeight;

            if (DataContext is IViewModelActions)
            {
                (DataContext as IViewModelActions).HandleViewLoaded();
            }

            if (IsOrthogonalViewChecked)
            {
                ActivateDeactivateOrthogonalView();
            }

            ImageCanvas = imageCanvas;
        }

        void ImageView_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            mainGrid.Height = e.NewSize.Height;
            mainGrid.Width = e.NewSize.Width;

            toolbarGrid.Width = e.NewSize.Width;
            toolbarGrid.Height = e.NewSize.Height;

            helpGrid.Width = e.NewSize.Width;
            FrameworkElement parent = this.Parent as FrameworkElement;
            if (parent != null)
            {
                // This is to keep the help text at the bottom of any parent window (ex: stays at the bottom of a ScrollViewer while user scrolls)
                helpGrid.Height = parent.ActualHeight+2;
            }
        }

        void ImageView_Unloaded(object sender, RoutedEventArgs e)
        {
            SizeChanged -= new SizeChangedEventHandler(ImageView_SizeChanged);
            OverlayManagerClass.Instance.UpdatingObjectEvent -= VM_UpdatingObject;

            if (DataContext is IViewModelActions)
            {
                (DataContext as IViewModelActions).HandleViewUnloaded();
            }
        }

        private void image_MouseDown(object sender, MouseButtonEventArgs e)
        {
            _lastPosOverlay = e.GetPosition(sender as IInputElement);
            mouseDown = true;
            if (e.ClickCount == 1)  // single click
            {
                //look for the button inside of the grid to execute the command
                //this command is they way we let the vm know which image is selected
                //before we start drawing
                if (sender is Button button)
                {
                    button.Command.Execute(null);
                }
                else if (sender is Grid parentGrid)
                {
                    Button button2 = parentGrid.GetChildOfType<Button>();
                    button2?.Command?.Execute(null);
                }
                else
                {
                    var image = sender as Image;

                    var grid = VisualTreeHelper.GetParent(image);
                    Button button2 = grid.GetChildOfType<Button>();
                    button2?.Command?.Execute(null);
                }

                if (e.ChangedButton == MouseButton.Left)
                {
                    OverlayManagerClass.MouseEventEnum me = OverlayManagerClass.MouseEventEnum.LEFTSINGLECLICK;
                    OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), _lastPosOverlay, _shiftDown);
                }
            }
            else if (e.ClickCount == 2)
            {
                OverlayManagerClass.MouseEventEnum me = OverlayManagerClass.MouseEventEnum.LEFTDOUBLECLICK;
                OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), _lastPosOverlay, _shiftDown);
            }
        }

        private void image_MouseMove(object sender, MouseEventArgs e)
        {
            if (true == _updatingOverlayObject)
            {
                if (mouseDown)
                {
                    _lastPosOverlay = e.GetPosition(sender as IInputElement);
                    OverlayManagerClass.MouseEventEnum me = OverlayManagerClass.MouseEventEnum.LEFTHOLDING;
                    OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), _lastPosOverlay, _shiftDown);
                }
            }
        }

        private void image_MouseUp(object sender, MouseButtonEventArgs e)
        {
            mouseDown = false;
            OverlayManagerClass.MouseEventEnum me = OverlayManagerClass.MouseEventEnum.LEFTMOUSEUP;
            Point currentPoint = e.GetPosition(sender as IInputElement);
            OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), currentPoint, _shiftDown);
        }

        private void Items_CurrentChanged(object sender, EventArgs e)
        {
            //start timer here to give some time for the adorder layer to be created
            timerToUpdateOverlays.Start();
        }

        private void originalButton_Click(object sender, RoutedEventArgs e)
        {
            //Change the zoom
            _scaleTransform.ScaleX = 1.0;
            _scaleTransform.ScaleY = 1.0;

            _scaleTransform.CenterX = 0;
            _scaleTransform.CenterY = 0;

            zoomSlider.Value = 1.0;

            double translateX = _translateTransform.X;
            double translateY = _translateTransform.Y;

            // translate the image back to origin in X Y coordinates
            _translateTransform.X -= translateX;
            _translateTransform.Y -= translateY;

            imageCanvas.RenderTransform = _transformGroup;
            xyOrthogonalImageCanvas.RenderTransform = new MatrixTransform(m);
            if (IsOrthogonalViewChecked)
            {
                UpdateOrthogonalView(OrthogonalViewPosition);
            }

            IVScrollBarHeight = (0 < imageGrid.Items?.Count && null != _translateTransform) ? ((imageGrid.ActualHeight + 0.001) * _currentScale + _translateTransform.Y) : IVScrollBarHeight;
        }

        private void OrthogonalLinesLockPosition_Click(object sender, RoutedEventArgs e)
        {
            //verify the data context
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                return;
            }
            if (lockPosition.IsChecked == true)
            {
                OrthogonalViewPosition = lockPoint;
                UpdateOrthogonalView(lockPoint);
            }
        }

        private void OrthogonalViewEnable_Click(object sender, RoutedEventArgs e)
        {
            ActivateDeactivateOrthogonalView();
        }

        /// <summary>
        /// return to 100% scale
        /// </summary>
        /// <param name="e"></param>
        private void ResetClick(object sender, RoutedEventArgs e)
        {
            ////verify the data context
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                return;
            }

            _currentScale = Math.Min(ActualHeight / imageGrid.ActualHeight, ActualWidth / imageGrid.ActualWidth);

            //Change the zoom
            _scaleTransform.ScaleX = _currentScale;
            _scaleTransform.ScaleY = _currentScale;

            _scaleTransform.CenterX = 0;
            _scaleTransform.CenterY = 0;

            zoomSlider.Value = _currentScale;

            double translateX = _translateTransform.X;
            double translateY = _translateTransform.Y;

            // translate the image back to origin in X Y coordinates
            _translateTransform.X -= translateX;
            _translateTransform.Y -= translateY;

            imageCanvas.RenderTransform = _transformGroup;

            if (true == OrthogonalViewEnable.IsChecked)
            {
                UpdateOrthogonalView(OrthogonalViewPosition);
            }

            IVScrollBarHeight = (0 < imageGrid.Items?.Count && null != _translateTransform) ? ((imageGrid.ActualHeight + 0.001) * _currentScale + _translateTransform.Y) : IVScrollBarHeight;
        }

        /// <summary>
        /// Handles the ROI toolbar visibility being changed by making sure all items in the toolbar are deselected
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ROIToolbar_VisibilityChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if (!(bool)e.NewValue) //Not Visible
            {
                ROIDrawingTools.SelectedItem = SelectTool;
            }
        }

        private void SaveROIs_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();

            dlg.DefaultExt = ".xaml";
            dlg.Filter = "WPF XAML (.xaml)|*.xaml";

            Nullable<bool> result = dlg.ShowDialog();

            if (true == result)
            {
                OverlayManagerClass.Instance.SaveROIs(dlg.FileName);
            }
            ROIDrawingTools.SelectedIndex = 0;
            OverlayManagerClass.Instance.InitSelectROI();
        }

        private void SelectROI_Selected(object sender, RoutedEventArgs e)
        {
            OverlayManagerClass.Instance.InitSelectROI();
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            //stop the timer since we only wanted to count once since the imageGrid changed
            timerToUpdateOverlays.Stop();

            //ask overlay manager to update the visible ROIs
            OverlayManagerClass.Instance.UpdateVisibleROIS();
        }

        void UpdateOrthogonalView()
        {
            UpdateOrthogonalView(OrthogonalViewPosition);
        }

        private void UpdateOrthogonalView(Point point)
        {
            if ((0 >= imageGrid.Items?.Count || null == imageGrid.Items))
            {
                return;
            }
            ImageBrush brush = (ImageBrush)imageCanvas.Background;
            if (brush == null) return;
            if (brush.ImageSource == null)
            {
                brush.ImageSource = (System.Windows.Media.Imaging.WriteableBitmap) MVMManager.Instance["ImageViewCaptureSetupVM", "Bitmap"];
                imageCanvas.Background = brush;

            }

            UpdateOrthogonalViewLines(xzOrthogonalImageCanvas, xzOrthogonalImageCanvas, new Point(point.X, ZNumSteps - 1), _orthogonalXZViewLine, LineDirection.vertical);
            UpdateOrthogonalViewLines(yzOrthogonalImageCanvas, yzOrthogonalImageCanvas, new Point(ZNumSteps - 1, point.Y), _orthogonalYZViewLine, LineDirection.horizontal);
            UpdateOrthogonalViewLines(imageCanvas, xyOrthogonalImageCanvas, point, _orthogonalViewLine, LineDirection.horizontalAndVertical);

            _orthogonalViewMatrix = imageCanvas.RenderTransform.Value;
            _orthogonalViewMatrix.OffsetY += _currentScale * imageGrid.ActualHeight + 10;
            xzOrthogonalImageCanvas.RenderTransform = new MatrixTransform(_orthogonalViewMatrix);
            _orthogonalViewMatrix = imageCanvas.RenderTransform.Value;
            _orthogonalViewMatrix.OffsetX += _currentScale * imageGrid.ActualWidth + 10;
            yzOrthogonalImageCanvas.RenderTransform = new MatrixTransform(_orthogonalViewMatrix);
        }

        private void UpdateOrthogonalViewLines(Canvas srcCanvas, Canvas dstCanvas, Point point, Line[] line, LineDirection lineDirection)
        {
            try
            {
                for (int i = 0; i < line.Length; i++)
                {
                    if (line[i] == null)
                    {
                        line[i] = new Line();
                        line[i].Stroke = _brushColors[OrthogonalLineColorType];
                        line[i].StrokeDashArray = OrthogonalLineType == 0 ? new DoubleCollection() { 2 } : null;
                        line[i].SnapsToDevicePixels = true;
                        line[i].SetValue(RenderOptions.EdgeModeProperty, EdgeMode.Aliased);
                        line[i].StrokeThickness = Math.Max(1, Math.Min(1 / _currentScale, 10));
                        line[i].IsHitTestVisible = false; // block all the hit handled of line and raise from canvas
                        line[i].Uid = "OrthogonalViewLine" + i.ToString();
                    }
                    for (int j = 0; j < dstCanvas.Children.Count; j++)
                    {
                        if (dstCanvas.Children[j].Uid == "OrthogonalViewLine" + i.ToString())
                        {
                            dstCanvas.Children.RemoveAt(j);
                            line[i].Stroke = _brushColors[OrthogonalLineColorType];
                            line[i].StrokeThickness = Math.Max(1, Math.Min(1 / _currentScale, 10));
                            if (OrthogonalLineType == 0)
                            {
                                line[i].StrokeDashArray = new DoubleCollection() { 2 };
                            }
                            else
                            {
                                line[i].StrokeDashArray = null;
                            }
                        }
                    }
                }

                switch (lineDirection)
                {
                    case LineDirection.vertical:
                        {
                            ImageBrush brush = (ImageBrush)srcCanvas.Background;
                            if (null == brush || null == brush.ImageSource) return;
                            line[0].X1 = point.X;
                            line[0].X2 = point.X;
                            line[0].Y2 = 0;
                            line[0].Y1 = brush.ImageSource.Height;
                            dstCanvas.Children.Add(line[0]);
                        }
                        break;
                    default:
                        {
                            ImageBrush brush = (ImageBrush)srcCanvas.Background;
                            if (null == brush || null == brush.ImageSource) return;
                            line[0].X1 = 0;
                            line[0].X2 = brush.ImageSource.Width;
                            line[0].Y2 = point.Y;
                            line[0].Y1 = point.Y;

                            dstCanvas.Children.Add(line[0]);
                            if (line.Length > 1)
                            {
                                line[1].X1 = point.X;
                                line[1].X2 = point.X;
                                line[1].Y2 = 0;
                                line[1].Y1 = brush.ImageSource.Height;
                                dstCanvas.Children.Add(line[1]);
                            }
                        }
                        break;
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void UpdateRollOverPixelData(Point imagePixelLocation)
        {
            RollOverPointX = (int)Math.Floor(imagePixelLocation.X);
            RollOverPointY = (int)Math.Floor(imagePixelLocation.Y);
        }

        void UpdateXYOffsetTransform()
        {
            _transformGroup.Children.Remove(_translateTransform);
            _translateTransform = new TranslateTransform(ImageOffsetX, ImageOffsetY);
            _transformGroup.Children.Add(_translateTransform);
        }

        void vm_CloseOrthogonalView()
        {
            OrthogonalViewEnable.IsChecked = false;

            xyOrthogonalImageCanvas.Children.Clear();
            xzOrthogonalImageCanvas.Children.Clear();
            yzOrthogonalImageCanvas.Children.Clear();

            lockPosition.IsChecked = false;
            lockPosition.Visibility = Visibility.Collapsed;  //hide the lock position option
            xzOrthogonalImageCanvas.Visibility = Visibility.Collapsed; //hide the xzImage and yzImage
            yzOrthogonalImageCanvas.Visibility = Visibility.Collapsed;
        }

        void VM_LineScanEnvent()
        {
            OverlayManagerClass.Instance.ClearAllObjects();
            ReticleOnOff.IsChecked = false;
            ScaleOnOff.IsChecked = false;
            OverlayManagerClass.Instance.InitSelectROI();

            //The second point must be less than pixel x, because otherwise an extra pixel would be included
            //since the canvas is zero based, and pixelX and pixelY are one based
            OverlayManagerClass.Instance.CreateROIShape(typeof(Line), new Point(0, 0.5), new Point((int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)512] - 0.01, 0.5));
            OverlayManagerClass.Instance.InitSelectROI();
        }

        /// <summary>
        /// Handles changes in the view model with additional behaviors outside of the xaml bindings. Invokes all
        /// work to be done on the dispatch thread.
        /// </summary>
        /// <param name="sender"> </param>
        /// <param name="e"> </param>
        private void vm_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            //Application.Current.Dispatcher.BeginInvoke(new Action(() => ReactToPropertyChangeEvents(sender, e)));
        }

        private void VM_UpdatingObject(bool obj)
        {
            _updatingOverlayObject = obj;
        }

        /// <summary>
        /// Zooms the displayed image to the desired percent. 
        /// </summary>
        /// <param name="zoomPercent"> Percent in integer values ie. 100 is no zoom </param>
        private void ZoomDisplay()
        {
            if (Math.Abs(_currentScale - ZoomLevel) < .000001)
            {
                return;
            }

            _currentScale = ZoomLevel;

            _scaleTransform.ScaleX = _currentScale;
            _scaleTransform.ScaleY = _currentScale;

            imageCanvas.RenderTransform = _transformGroup;

            xyOrthogonalImageCanvas.RenderTransform = _transformGroup;

            if (true == OrthogonalViewEnable.IsChecked)
            {
                UpdateOrthogonalView(OrthogonalViewPosition);
            }
            
            IVScrollBarHeight = (0 < imageGrid.Items?.Count && null != _translateTransform) ? ((imageGrid.ActualHeight + 0.001) * _currentScale + _translateTransform.Y) : IVScrollBarHeight;
        }

        #endregion Methods
    }
}