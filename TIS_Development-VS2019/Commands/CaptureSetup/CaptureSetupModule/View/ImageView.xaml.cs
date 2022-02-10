namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Interop;
    using System.Windows.Media;
    using System.Windows.Media.Effects;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    using CaptureSetupDll.Model;
    using CaptureSetupDll.ViewModel;

    using Microsoft.Win32;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for ImageView.xaml
    /// </summary>
    public partial class ImageView : UserControl
    {
        #region Fields

        private const int LUT_MAX = 255;
        private const int LUT_MIN = 0;
        private const int MAX_HISTOGRAMS = 4;

        private Point lockPoint;
        Matrix m = new Matrix();
        private string _appDocPath;
        private BackgroundWorker _bleachROIChecker;
        private SolidColorBrush[] _brushColors = new SolidColorBrush[] { Brushes.White, Brushes.Black, Brushes.Green, Brushes.Blue, Brushes.Yellow, Brushes.Red };
        private double _currentScale;
        private DispatcherTimer _histogramUpdateTimer;
        private bool _imageDataChanged;
        private double _imageHeight;
        private double _imageWidth;
        Point _newOrigin;
        Point _newStart;
        bool _nonVitualStackObjectMovingFlag = false;
        private int _orthogonalLineColorType = 0;
        private int _orthogonalLineType = 0;
        private Line[] _orthogonalViewLine = new Line[2];
        private Matrix _orthogonalViewMatrix;
        private Line[] _orthogonalXZViewLine = new Line[1];
        private Line[] _orthogonalYZViewLine = new Line[1];
        private bool _persistWhileClosing = false;
        bool _roiToolMouseDoubleClickFlag = false;
        private int _saveDlgFilterIndex = 0;
        private ScaleTransform _scaleTransform;
        private Point _scrollStartPoint;
        private bool _shiftDown;
        private TransformGroup _transformGroup;
        private TranslateTransform _translateTransform;
        private bool _updatingOverlayObject;
        private int _whitePointMaxVal = 255;

        #endregion Fields

        #region Constructors

        public ImageView()
        {
            InitializeComponent();

            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
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
            overlayCanvas.RenderTransform = _transformGroup;
            presentationCanvas.RenderTransform = _transformGroup;
            xyOrthogonalImageCanvas.RenderTransform = _transformGroup;

            _currentScale = 1.0;
            zoomSlider.Value = 1.0;

            histogram1.DataBrushColor = Colors.Black;
            histogram2.DataBrushColor = Colors.Red;
            histogram3.DataBrushColor = Colors.Green;
            histogram4.DataBrushColor = Colors.Blue;
            histogram1.BwTextBoxFocued += new EventHandler(H1_BWTextBoxFocused);
            histogram2.BwTextBoxFocued += new EventHandler(H2_BWTextBoxFocused);
            histogram3.BwTextBoxFocued += new EventHandler(H3_BWTextBoxFocused);
            histogram4.BwTextBoxFocued += new EventHandler(H4_BWTextBoxFocused);
            _imageDataChanged = false;
            _updatingOverlayObject = false;
            _shiftDown = false;
            _histogramUpdateTimer = new DispatcherTimer();
            _histogramUpdateTimer.Interval = TimeSpan.FromSeconds(1);

            this.Loaded += new RoutedEventHandler(ImageView_Loaded);
            this.Unloaded += new RoutedEventHandler(ImageView_Unloaded);
            this.Dispatcher.ShutdownStarted += new EventHandler(ImageView_closing);
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
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

        #region Delegates

        private delegate bool BleachROIStateChecker();

        #endregion Delegates

        #region Methods

        public void ImageView_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.LeftShift || e.Key == Key.RightShift)
            {
                _shiftDown = true;
            }
            else if (e.Key == Key.Delete || e.Key == Key.Back)
            {
                CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
                if (vm == null)
                {
                    return;
                }
                if (null == vm.Bitmap)
                {
                    ROIDrawingTools.SelectedIndex = 0;
                    return;
                }
                OverlayManagerClass.Instance.DeleteSelectedROIs(ref overlayCanvas);
                if (0 == OverlayManagerClass.Instance.ROICount)
                {
                    //Force reset ROI count
                    OverlayManagerClass.Instance.ClearAllObjects(ref overlayCanvas);
                    if (null != vm.MultiROIStats)
                    {
                        if (vm.ROIStatsTableActive)
                        {
                            vm.MultiROIStats.Hide();
                        }
                        else
                        {
                            vm.MultiROIStats.Close();
                        }
                    }

                    if (null != vm.ROIStatsChart)
                    {
                        if (vm.ROIStatsChartActive)
                        {
                            vm.ROIStatsChart.Hide();
                        }
                        else
                        {
                            vm.ROIStatsChart.Close();
                            //In CaptureSetup _roiStatsChart is not closed (only hidden)
                            //force the persistance of the window settings
                            vm.PersistROIStatsChartWindowSettings();
                        }
                    }

                    if (null != vm.LineProfile)
                    {
                        if (vm.LineProfileActive)
                        {
                            vm.LineProfile.Hide();
                        }
                        else
                        {
                            vm.LineProfile.Close();
                        }
                    }
                }
            }
            else if (e.Key == Key.A && (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
            {
                CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
                if (vm == null)
                {
                    return;
                }
                ROIDrawingTools.SelectedIndex = 0;
                OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
                if (null == vm.Bitmap)
                {
                    return;
                }
                OverlayManagerClass.Instance.SelectAllROIs();
            }
            else if (e.Key == Key.Tab)
            {
                CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
                if (vm == null)
                {
                    return;
                }
                if (null == vm.Bitmap)
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
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            ROIDrawingTools.SelectedIndex = 0;
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
        }

        protected override void OnMouseDoubleClick(MouseButtonEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            Point pt = this.TranslatePoint(e.GetPosition(this), imageCanvas);
            OverlayManager.OverlayManagerClass.MouseEventEnum me = OverlayManager.OverlayManagerClass.MouseEventEnum.LEFTDOUBLECLICK;
            OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), ref overlayCanvas, new Point(_scrollStartPoint.X, _scrollStartPoint.Y), _shiftDown);
        }

        /// <summary>
        /// Get position and CaptureMouse
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            if (image1.ImageSource != null)
            {
                _imageWidth = image1.ImageSource.Width;
                _imageHeight = image1.ImageSource.Height;
            }

            if (IsMouseOver)
            {
                _scrollStartPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                if (image1.ImageSource != null)
                {
                    _scrollStartPoint.X = Math.Max(0, Math.Min(_scrollStartPoint.X, image1.ImageSource.Width - 0.001));
                    _scrollStartPoint.Y = Math.Max(0, Math.Min(_scrollStartPoint.Y, image1.ImageSource.Height - 0.001));
                }
                if (e.ClickCount == 1)  // single click
                {
                    if (e.ClickCount == 1)  // single click
                    {
                        if (e.ChangedButton == MouseButton.Left)   // left click
                        {
                            OverlayManager.OverlayManagerClass.MouseEventEnum me = OverlayManager.OverlayManagerClass.MouseEventEnum.LEFTSINGLECLICK;
                            OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), ref overlayCanvas, new Point(_scrollStartPoint.X, _scrollStartPoint.Y), _shiftDown);
                            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
                        }
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
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if (IsMouseCaptured)
            {
                if (true == _updatingOverlayObject)
                {
                    if (image1.ImageSource != null && e.LeftButton == MouseButtonState.Pressed)
                    {

                        Point currentPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                        OverlayManager.OverlayManagerClass.MouseEventEnum me = OverlayManager.OverlayManagerClass.MouseEventEnum.LEFTHOLDING;
                        currentPoint.X = Math.Max(0, Math.Min(currentPoint.X, image1.ImageSource.Width - 0.001));
                        currentPoint.Y = Math.Max(0, Math.Min(currentPoint.Y, image1.ImageSource.Height - 0.001));
                        OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), ref overlayCanvas, new Point(currentPoint.X, currentPoint.Y), _shiftDown);
                    }
                }
                else
                {
                    Point pp = new Point();
                    pp = e.MouseDevice.GetPosition(this);

                    Matrix m = imageCanvas.RenderTransform.Value;
                    m.OffsetX = _newOrigin.X + (pp.X - _newStart.X);
                    m.OffsetY = _newOrigin.Y + (pp.Y - _newStart.Y);

                    imageCanvas.RenderTransform = new MatrixTransform(m);
                    overlayCanvas.RenderTransform = new MatrixTransform(m);
                    presentationCanvas.RenderTransform = new MatrixTransform(m);
                    xyOrthogonalImageCanvas.RenderTransform = new MatrixTransform(m);
                    if (vm.OrthogonalViewStat != CaptureSetupViewModel.OrthogonalViewStatus.INACTIVE)
                    {
                        m.OffsetY += _currentScale * image1.ImageSource.Height + 10;
                        xzOrthogonalImageCanvas.RenderTransform = new MatrixTransform(m);

                        m.OffsetY -= _currentScale * image1.ImageSource.Height + 10;
                        m.OffsetX += _currentScale * image1.ImageSource.Width + 10;
                        yzOrthogonalImageCanvas.RenderTransform = new MatrixTransform(m);
                        if (pp != _newStart)
                        {
                            _nonVitualStackObjectMovingFlag = true;
                        }
                    }
                }
            }

            if (vm.OrthogonalViewStat == CaptureSetupViewModel.OrthogonalViewStatus.ACTIVE && IsMouseCaptured == false && image1.ImageSource != null && imageCanvas.ContextMenu.IsOpen == false && lockPosition.IsChecked == false)
            {
                if (ROIToolbarEnable.IsChecked == false || (ROIToolbarEnable.IsChecked == true && ROIDrawingTools.SelectedIndex == 0))
                {
                    Point startPoint;
                    startPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                    if (startPoint.X < image1.ImageSource.Width && startPoint.Y < image1.ImageSource.Height)
                    {
                        startPoint.X = Math.Max(0, Math.Min(startPoint.X, image1.ImageSource.Width - 0.001));
                        startPoint.Y = Math.Max(0, Math.Min(startPoint.Y, image1.ImageSource.Height - 0.001));

                        if (vm.VirtualZStack)
                        {
                            SetupOrthogonalView(startPoint);
                            UpdateOrthogonalView(startPoint);
                        }
                    }
                }
            }

            Point imagePixelLocation = this.TranslatePoint(e.GetPosition(this), imageCanvas);

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
                CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
                if (vm == null)
                {
                    return;
                }

                if (vm.Bitmap == null)
                {
                    ReleaseMouseCapture();
                    this.imageCanvas.ContextMenu.Visibility = Visibility.Collapsed;
                    return;
                }
                else
                {
                    this.imageCanvas.ContextMenu.Visibility = Visibility.Visible;
                }

                if ((e.ChangedButton == MouseButton.Left) && (null != image1.ImageSource))
                {
                    OverlayManager.OverlayManagerClass.MouseEventEnum me = OverlayManager.OverlayManagerClass.MouseEventEnum.LEFTMOUSEUP;
                    Point currentPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                    currentPoint.X = Math.Max(0, Math.Min(currentPoint.X, image1.ImageSource.Width - 0.001));
                    currentPoint.Y = Math.Max(0, Math.Min(currentPoint.Y, image1.ImageSource.Height - 0.001));
                    OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), ref overlayCanvas, new Point(currentPoint.X, currentPoint.Y), _shiftDown);
                    if (vm.VirtualZStack == false && vm.OrthogonalViewStat == CaptureSetupViewModel.OrthogonalViewStatus.ACTIVE && _nonVitualStackObjectMovingFlag == false)
                    {
                        if (ROIToolbarEnable.IsChecked == false || (ROIToolbarEnable.IsChecked == true && ROIDrawingTools.SelectedIndex == 0))
                        {
                            Point startPoint;
                            startPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                            if (startPoint.X < image1.ImageSource.Width && startPoint.Y < image1.ImageSource.Height)
                            {
                                startPoint.X = Math.Max(0, Math.Min(startPoint.X, image1.ImageSource.Width - 0.001));
                                startPoint.Y = Math.Max(0, Math.Min(startPoint.Y, image1.ImageSource.Height - 0.001));
                                SetupOrthogonalView(startPoint);
                            }
                        }
                    }
                    _nonVitualStackObjectMovingFlag = false;
                }

                if (e.ChangedButton == MouseButton.Right && vm.OrthogonalViewStat == CaptureSetupViewModel.OrthogonalViewStatus.ACTIVE && vm.VirtualZStack == true)
                {
                    lockPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                    lockPoint.X = Math.Max(0, Math.Min(lockPoint.X, image1.ImageSource.Width - 0.001));
                    lockPoint.Y = Math.Max(0, Math.Min(lockPoint.Y, image1.ImageSource.Height - 0.001));
                }

                vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;

                Cursor = Cursors.Arrow;

                ReleaseMouseCapture();
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
            overlayCanvas.RenderTransform = new MatrixTransform(m);
            presentationCanvas.RenderTransform = new MatrixTransform(m);
            xyOrthogonalImageCanvas.RenderTransform = new MatrixTransform(m);
            zoomSlider.Value = m.M22;
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            if (vm == null)
            {
                return;
            }

            if (vm.OrthogonalViewStat == CaptureSetupViewModel.OrthogonalViewStatus.HOLD)
            {
                Point startPoint = vm.BitmapPoint;
                SetupOrthogonalView(startPoint);
                UpdateOrthogonalView(startPoint);
            }
            else if (vm.OrthogonalViewStat == CaptureSetupViewModel.OrthogonalViewStatus.ACTIVE)
            {
                Point startPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);

                startPoint.X = Math.Max(0, Math.Min(startPoint.X, image1.ImageSource.Width - 0.001));
                startPoint.Y = Math.Max(0, Math.Min(startPoint.Y, image1.ImageSource.Height - 0.001));
                SetupOrthogonalView(startPoint);
                UpdateOrthogonalView(startPoint);
            }

            e.Handled = true;

            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        private void ActivateDeactivateOrthogonalView()
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            if (image1 != null && image1.ImageSource != null)
            {
                vm.BitmapPoint = new Point(image1.ImageSource.Width / 2, image1.ImageSource.Height / 2); //initiation with middle point
            }

            if (vm.LiveStartButtonStatus == true)
            {
                if (vm != null && image1.ImageSource != null && OrthogonalViewEnable.IsChecked == true)
                {
                    vm.InitOrthogonalView();
                    this.lockPosition.Visibility = Visibility.Visible; // contextMenu lock position is visible
                    this.lockPosition.IsChecked = false;
                }
            }

            if (false == OrthogonalViewEnable.IsChecked)
            {
                vm_CloseOrthogonalView();
            }
        }

        private void AutoEnhance1()
        {
            histogram1.MaxBinValue = _whitePointMaxVal;
            histogram1.BlackPoint = histogram1.MinValue;
            histogram1.WhitePoint = Math.Max(1, histogram1.MaxValue); //The minimum value a white point can have is 1
            sliderBP0.Value = histogram1.MinValue;
            sliderWP0.Value = Math.Max(1, histogram1.MaxValue);
        }

        private void AutoEnhance2()
        {
            histogram2.MaxBinValue = _whitePointMaxVal;
            histogram2.BlackPoint = histogram2.MinValue;
            histogram2.WhitePoint = Math.Max(1, histogram2.MaxValue); //The minimum value a white point can have is 1
            sliderBP1.Value = histogram2.MinValue;
            sliderWP1.Value = Math.Max(1, histogram2.MaxValue);
        }

        private void AutoEnhance3()
        {
            histogram3.MaxBinValue = _whitePointMaxVal;
            histogram3.BlackPoint = histogram3.MinValue;
            histogram3.WhitePoint = Math.Max(1, histogram3.MaxValue); //The minimum value a white point can have is 1
            sliderBP2.Value = histogram3.MinValue;
            sliderWP2.Value = Math.Max(1, histogram3.MaxValue);
        }

        private void AutoEnhance4()
        {
            histogram4.MaxBinValue = _whitePointMaxVal;
            histogram4.BlackPoint = histogram4.MinValue;
            histogram4.WhitePoint = Math.Max(1, histogram4.MaxValue); //The minimum value a white point can have is 1
            sliderBP3.Value = histogram4.MinValue;
            sliderWP3.Value = Math.Max(1, histogram4.MaxValue);
        }

        private void btnBPDown0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram1.BlackPoint -= 1;
        }

        private void btnBPDown1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram2.BlackPoint -= 1;
        }

        private void btnBPDown2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram3.BlackPoint -= 1;
        }

        private void btnBPDown3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram4.BlackPoint -= 1;
        }

        private void btnBPUp0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram1.BlackPoint += 1;
        }

        private void btnBPUp1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram2.BlackPoint += 1;
        }

        private void btnBPUp2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram3.BlackPoint += 1;
        }

        private void btnBPUp3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram4.BlackPoint += 1;
        }

        private void btnWPDown0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram1.WhitePoint -= 1;
        }

        private void btnWPDown1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram2.WhitePoint -= 1;
        }

        private void btnWPDown2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram3.WhitePoint -= 1;
        }

        private void btnWPDown3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram4.WhitePoint -= 1;
        }

        private void btnWPUp0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram1.WhitePoint += 1;
        }

        private void btnWPUp1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram2.WhitePoint += 1;
        }

        private void btnWPUp2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram3.WhitePoint += 1;
        }

        private void btnWPUp3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram4.WhitePoint += 1;
        }

        private void ButtonReset_Click1(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;

            histogram1.BlackPoint = LUT_MIN;
            histogram1.WhitePoint = LUT_MAX;
            sliderBP0.Value = LUT_MIN;
            sliderWP0.Value = LUT_MAX;
        }

        private void ButtonReset_Click2(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;

            histogram2.BlackPoint = LUT_MIN;
            histogram2.WhitePoint = LUT_MAX;
            sliderBP1.Value = LUT_MIN;
            sliderWP1.Value = LUT_MAX;
        }

        private void ButtonReset_Click3(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;

            histogram3.BlackPoint = LUT_MIN;
            histogram3.WhitePoint = LUT_MAX;
            sliderBP2.Value = LUT_MIN;
            sliderWP2.Value = LUT_MAX;
        }

        private void ButtonReset_Click4(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;

            histogram4.BlackPoint = LUT_MIN;
            histogram4.WhitePoint = LUT_MAX;
            sliderBP3.Value = LUT_MIN;
            sliderWP3.Value = LUT_MAX;
        }

        private void ClearAllROIs_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if (MessageBoxResult.Yes == MessageBox.Show(new Window() { Topmost = true }, "Do you wish to delete all ROIs?", "Clear ROIs?", MessageBoxButton.YesNo, MessageBoxImage.Question, MessageBoxResult.Yes))
            {
                OverlayManagerClass.Instance.ClearAllObjects(ref overlayCanvas);
                ReticleOnOff.IsChecked = false;
                ScaleOnOff.IsChecked = false;
                ROIDrawingTools.SelectedIndex = 0;
                OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);

                if (null != vm.MultiROIStats)
                {
                    if (vm.ROIStatsTableActive)
                    {
                        vm.MultiROIStats.Hide();
                    }
                    else
                    {
                        vm.MultiROIStats.Close();
                    }
                }

                if (null != vm.ROIStatsChart)
                {
                    if (vm.ROIStatsChartActive)
                    {
                        vm.ROIStatsChart.Hide();
                    }
                    else
                    {
                        vm.ROIStatsChart.Close();
                        //In CaptureSetup _roiStatsChart is not closed (only hidden)
                        //force the persistance of the window settings
                        vm.PersistROIStatsChartWindowSettings();
                    }
                }

                if (null != vm.LineProfile)
                {
                    if (vm.LineProfileActive)
                    {
                        vm.LineProfile.Hide();
                    }
                    else
                    {
                        vm.LineProfile.Close();
                    }
                }
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
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROICrosshair(ref overlayCanvas);
        }

        private void createEllipseROI_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROIEllipse(ref overlayCanvas);
        }

        private void createLineROI_Click(object sender, RoutedEventArgs e)
        {
            if (true == _roiToolMouseDoubleClickFlag)
            {
                _roiToolMouseDoubleClickFlag = false;
                return;
            }
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROILine(ref overlayCanvas);
        }

        private void createLineROI_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            _roiToolMouseDoubleClickFlag = true;
            OverlayManagerClass.Instance.InitROILineWithOptions(ref overlayCanvas);
        }

        private void createPolylineROI_Click(object sender, RoutedEventArgs e)
        {
            if (true == _roiToolMouseDoubleClickFlag)
            {
                _roiToolMouseDoubleClickFlag = false;
                return;
            }
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROIPolyline(ref overlayCanvas);
        }

        private void createPolylineROI_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            _roiToolMouseDoubleClickFlag = true;
            OverlayManagerClass.Instance.InitROIPolylineWithOptions(ref overlayCanvas);
        }

        private void createPolyROI_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROIPoly(ref overlayCanvas);
        }

        private void createRectROI_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROIRect(ref overlayCanvas);
        }

        private void DeleteSelectedROIs(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if (null == vm.Bitmap)
            {
                ROIDrawingTools.SelectedIndex = 0;
                return;
            }
            OverlayManagerClass.Instance.DeleteSelectedROIs(ref overlayCanvas);
            ROIDrawingTools.SelectedIndex = 0;
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
            if (0 == overlayCanvas.Children.Count)
            {
                //Force reset ROI count
                OverlayManagerClass.Instance.ClearAllObjects(ref overlayCanvas);
                if (null != vm.MultiROIStats)
                {
                    if (vm.ROIStatsTableActive)
                    {
                        vm.MultiROIStats.Hide();
                    }
                    else
                    {
                        vm.MultiROIStats.Close();
                    }
                }

                if (null != vm.ROIStatsChart)
                {
                    if (vm.ROIStatsChartActive)
                    {
                        vm.ROIStatsChart.Hide();
                    }
                    else
                    {
                        vm.ROIStatsChart.Close();
                        //In CaptureSetup _roiStatsChart is not closed (only hidden)
                        //force the persistance of the window settings
                        vm.PersistROIStatsChartWindowSettings();
                    }
                }

                if (null != vm.LineProfile)
                {
                    if (vm.LineProfileActive)
                    {
                        vm.LineProfile.Hide();
                    }
                    else
                    {
                        vm.LineProfile.Close();
                    }
                }
            }
        }

        private void H1_BWTextBoxFocused(object sender, EventArgs e)
        {
            AutoManualTog1.IsChecked = false;
        }

        private void H2_BWTextBoxFocused(object sender, EventArgs e)
        {
            AutoManualTog2.IsChecked = false;
        }

        private void H3_BWTextBoxFocused(object sender, EventArgs e)
        {
            AutoManualTog3.IsChecked = false;
        }

        private void H4_BWTextBoxFocused(object sender, EventArgs e)
        {
            AutoManualTog4.IsChecked = false;
        }

        /// <summary>
        /// visibility of the histogram control changed
        /// </summary>
        /// <param name="e"></param>
        void Histogram_IsVisibleChanged1(object sender, DependencyPropertyChangedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }
            UpdateHistogramData();

            histogram1.WhitePoint = (int)vm.WhitePoint0;
            histogram1.BlackPoint = (int)vm.BlackPoint0;
            histogram2.WhitePoint = (int)vm.WhitePoint1;
            histogram2.BlackPoint = (int)vm.BlackPoint1;
            histogram3.WhitePoint = (int)vm.WhitePoint2;
            histogram3.BlackPoint = (int)vm.BlackPoint2;
            histogram4.WhitePoint = (int)vm.WhitePoint3;
            histogram4.BlackPoint = (int)vm.BlackPoint3;
        }

        private void ImageView_closing(object sender, EventArgs e)
        {
            if (_persistWhileClosing)
            {
                _histogramUpdateTimer.Stop();
                _histogramUpdateTimer.Tick -= new EventHandler(_histogramUpdateTimer_Tick);

                histogram1.IsVisibleChanged -= new DependencyPropertyChangedEventHandler(Histogram_IsVisibleChanged1);

                CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
                if (vm == null)
                {
                    return;
                }

                vm.PropertyChanged -= vm_PropertyChanged;

                OverlayManagerClass.Instance.UpdatingObjectEvent -= new Action<bool>(VM_UpdatingObject);
                vm.BleachWaveformGeneratedEvent -= new Action(VM_BleachROIsExtractedEvent);
                vm.DrawLineForLineScanEvent -= new Action(VM_LineScanEnvent);
                vm.ImageDataChanged -= new Action<bool>(VM_ImageDataChanged);
                vm.ROIUpdateRequested -= new Action<string>(VM_ROIUpdateRequested);
                vm.ActiveSettingsReplaced -= new Action(VM_ActiveSettingsReplaced);
                vm.OrthogonalViewImagesLoaded -= vm_OrthogonalViewImagesLoaded;
                this.SizeChanged -= new SizeChangedEventHandler(ImageView_SizeChanged);

                //persist application settings
                MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
                vm.ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNodeList ndList = vm.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ImageViewZoom");

                if (0 != vm.ApplicationDoc.BaseURI.CompareTo(_appDocPath))
                {
                    try
                    {
                        if (File.Exists(new Uri(_appDocPath).LocalPath))
                        {
                            //current modality is different from load, persist to last modality
                            XmlDocument doc = new XmlDocument();
                            doc.Load(_appDocPath);
                            ndList = doc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ImageViewZoom");
                            if (0 < ndList.Count)
                            {
                                XmlManager.SetAttribute(ndList[0], doc, "value", vm.ZoomLevel.ToString());
                                XmlManager.SetAttribute(ndList[0], doc, "offsetX", imageCanvas.RenderTransform.Value.OffsetX.ToString());
                                XmlManager.SetAttribute(ndList[0], doc, "offsetY", imageCanvas.RenderTransform.Value.OffsetY.ToString());
                                doc.Save(new Uri(_appDocPath).LocalPath);
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + "Could not load Application Settings doc: " + _appDocPath + "\n" + ex.Message);
                        return;
                    }
                }
                else if (0 < ndList.Count)
                {
                    XmlManager.SetAttribute(ndList[0], vm.ApplicationDoc, "value", vm.ZoomLevel.ToString());
                    XmlManager.SetAttribute(ndList[0], vm.ApplicationDoc, "offsetX", imageCanvas.RenderTransform.Value.OffsetX.ToString());
                    XmlManager.SetAttribute(ndList[0], vm.ApplicationDoc, "offsetY", imageCanvas.RenderTransform.Value.OffsetY.ToString());
                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                }
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

            histogram1.IsVisibleChanged += new DependencyPropertyChangedEventHandler(Histogram_IsVisibleChanged1);

            _histogramUpdateTimer.Tick += new EventHandler(_histogramUpdateTimer_Tick);
            _histogramUpdateTimer.Start();

            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            OverlayManagerClass.Instance.UpdatingObjectEvent += new Action<bool>(VM_UpdatingObject);
            vm.DrawLineForLineScanEvent += new Action(VM_LineScanEnvent);
            vm.BleachWaveformGeneratedEvent += new Action(VM_BleachROIsExtractedEvent);

            vm.ImageDataChanged += new Action<bool>(VM_ImageDataChanged);
            vm.ROIUpdateRequested += new Action<string>(VM_ROIUpdateRequested);
            vm.ActiveSettingsReplaced += new Action(VM_ActiveSettingsReplaced);
            this.SizeChanged += new SizeChangedEventHandler(ImageView_SizeChanged);
            vm.OrthogonalViewImagesLoaded += vm_OrthogonalViewImagesLoaded;

            ROIDrawingTools.SelectedIndex = 0;

            MVMManager.Instance["AreaControlViewModel", "propOverlayCanvas"] = CaptureSetupViewModel.OverlayCanvas = overlayCanvas;

            bool reticleActive = false;
            bool scaleActive = false;
            //reloading of the ROIs will result in a stats recalculation
            //if the modality changes and the stats are run on the size of the previous
            //image detector there is a possiblity of a buffer overrun. To ensure the buffers
            //are the correct size call the SetupCaptureBuffers to reallocate according to the active image detector
            CaptureSetupViewModel.SetupCaptureBuffers();
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
            OverlayManagerClass.Instance.PersistLoadROIs(ref overlayCanvas, ref reticleActive, ref scaleActive);
            ReticleOnOff.IsChecked = reticleActive;
            ScaleOnOff.IsChecked = scaleActive;
            vm.ReticleBotton = ReticleOnOff;
            vm.ScaleBotton = ScaleOnOff;

            //hide SLM patterns if SLM panel is not in use
            if (!vm.SLMPanelInUse)
                vm.SLMPatternsVisible = false;

            CaptureSetupViewModel.PresentationCanvas = presentationCanvas;

            vm.PropertyChanged += vm_PropertyChanged;

            toolbarGrid.Width = this.ActualWidth;
            toolbarGrid.Height = this.ActualHeight;

            //Load and adjust visible items from application settings
            vm.ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList ndList = vm.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ImageViewZoom");
            _appDocPath = vm.ApplicationDoc.BaseURI;

            if (0 < ndList.Count)
            {
                _transformGroup.Children.Remove(_translateTransform);
                _translateTransform = new TranslateTransform(Convert.ToDouble(ndList[0].Attributes["offsetX"].Value), Convert.ToDouble(ndList[0].Attributes["offsetY"].Value));
                _transformGroup.Children.Add(_translateTransform);
                vm.ZoomLevel = Convert.ToDouble(ndList[0].Attributes["value"].Value);
            }
            else
            {
                MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
                vm.ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNode g = vm.ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/General");
                XmlElement ivZoom = vm.ApplicationDoc.CreateElement("ImageViewZoom");
                ivZoom.SetAttribute("value", vm.ZoomLevel.ToString());
                ivZoom.SetAttribute("offsetX", imageCanvas.RenderTransform.Value.OffsetX.ToString());
                ivZoom.SetAttribute("offsetY", imageCanvas.RenderTransform.Value.OffsetY.ToString());
                g.AppendChild(ivZoom);
                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }

            XmlNodeList node = vm.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ChannelTileViewSettings");
            string str = string.Empty;
            if (0 < node.Count)
            {
                vm.TileDisplay = XmlManager.GetAttribute(node[0], vm.ApplicationDoc, "TilingEnableOption", ref str) && (str == "1" || str == Boolean.TrueString);
            }

            node = vm.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/HistogramSettings");
            if (0 < node.Count)
            {
                if (XmlManager.GetAttribute(node[0], vm.ApplicationDoc, "ReducedBinValue", ref str))
                {
                    _whitePointMaxVal = Convert.ToInt32(str);
                    if (0 >= _whitePointMaxVal || 255 < _whitePointMaxVal)
                    {
                        _whitePointMaxVal = 255;
                    }
                }

                if (XmlManager.GetAttribute(node[0], vm.ApplicationDoc, "saturationCountVisibility", ref str))
                {
                    vm.PMTSaturationsVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
            }

            node = vm.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/OrthogonalViewSettings");
            if (0 < node.Count)
            {
                if (XmlManager.GetAttribute(node[0], vm.ApplicationDoc, "lineColorIndex", ref str))
                {
                    int temp;
                    if (int.TryParse(str, out temp))
                    {
                        _orthogonalLineColorType = temp;
                    }
                }

                if (XmlManager.GetAttribute(node[0], vm.ApplicationDoc, "lineTypeIndex", ref str))
                {
                    int temp;
                    if (int.TryParse(str, out temp))
                    {
                        _orthogonalLineType = temp;
                    }
                }

                if (XmlManager.GetAttribute(node[0], vm.ApplicationDoc, "zMultiplier", ref str))
                {
                    double temp;
                    if (double.TryParse(str, out temp))
                    {
                        vm.OrthogonalViewZMultiplier = temp;
                    }
                }

                if (XmlManager.GetAttribute(node[0], vm.ApplicationDoc, "enableOrthogonalView", ref str))
                {
                    OrthogonalViewEnable.IsChecked = str == "1";
                    this.lockPosition.Visibility = true == OrthogonalViewEnable.IsChecked ? Visibility.Visible : Visibility.Collapsed;
                }

                ActivateDeactivateOrthogonalView();
            }

            //Capture Setup is loaded, the closing should only persist if it is being closed form Capture Setup
            // Need to refactor this. Should persist in the View Model side not in the View.
            _persistWhileClosing = true;
        }

        void ImageView_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            mainGrid.Height = e.NewSize.Height;
            mainGrid.Width = e.NewSize.Width;

            toolbarGrid.Width = e.NewSize.Width;
        }

        void ImageView_Unloaded(object sender, RoutedEventArgs e)
        {
            _histogramUpdateTimer.Stop();
            _histogramUpdateTimer.Tick -= new EventHandler(_histogramUpdateTimer_Tick);

            histogram1.IsVisibleChanged -= new DependencyPropertyChangedEventHandler(Histogram_IsVisibleChanged1);

            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            vm.PropertyChanged -= vm_PropertyChanged;

            OverlayManagerClass.Instance.UpdatingObjectEvent -= new Action<bool>(VM_UpdatingObject);
            vm.BleachWaveformGeneratedEvent -= new Action(VM_BleachROIsExtractedEvent);
            vm.DrawLineForLineScanEvent -= new Action(VM_LineScanEnvent);
            vm.ImageDataChanged -= new Action<bool>(VM_ImageDataChanged);
            vm.ROIUpdateRequested -= new Action<string>(VM_ROIUpdateRequested);
            vm.ActiveSettingsReplaced -= new Action(VM_ActiveSettingsReplaced);
            vm.OrthogonalViewImagesLoaded -= vm_OrthogonalViewImagesLoaded;

            this.SizeChanged -= new SizeChangedEventHandler(ImageView_SizeChanged);

            //persist application settings
            MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
            vm.ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList ndList = vm.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ImageViewZoom");
            XmlNodeList node = vm.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ChannelTileViewSettings");

            if (0 != vm.ApplicationDoc.BaseURI.CompareTo(_appDocPath))
            {
                try
                {
                    if (File.Exists(new Uri(_appDocPath).LocalPath))
                    {
                        //current modality is different from load, persist to last modality
                        XmlDocument doc = new XmlDocument();
                        doc.Load(_appDocPath);
                        ndList = doc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ImageViewZoom");
                        if (0 < ndList.Count)
                        {
                            XmlManager.SetAttribute(ndList[0], doc, "value", vm.ZoomLevel.ToString());
                            XmlManager.SetAttribute(ndList[0], doc, "offsetX", imageCanvas.RenderTransform.Value.OffsetX.ToString());
                            XmlManager.SetAttribute(ndList[0], doc, "offsetY", imageCanvas.RenderTransform.Value.OffsetY.ToString());
                        }
                        node = doc.SelectNodes("/ApplicationSettings/DisplayOptions/General/ChannelTileViewSettings");
                        if (0 < node.Count)
                        {
                            bool tileViewIsEnabled = vm.TileDisplay;
                            XmlManager.SetAttribute(node[0], vm.ApplicationDoc, "TilingEnableOption", tileViewIsEnabled.ToString());
                        }
                        doc.Save(new Uri(_appDocPath).LocalPath);
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + "Could not load Application Settings doc: " + _appDocPath + "\n" + ex.Message);
                    return;
                }
            }
            else if (0 < ndList.Count)
            {
                XmlManager.SetAttribute(ndList[0], vm.ApplicationDoc, "value", vm.ZoomLevel.ToString());
                XmlManager.SetAttribute(ndList[0], vm.ApplicationDoc, "offsetX", imageCanvas.RenderTransform.Value.OffsetX.ToString());
                XmlManager.SetAttribute(ndList[0], vm.ApplicationDoc, "offsetY", imageCanvas.RenderTransform.Value.OffsetY.ToString());
            }

            if (0 < node.Count)
            {
                bool tileViewIsEnabled = vm.TileDisplay;
                XmlManager.SetAttribute(node[0], vm.ApplicationDoc, "TilingEnableOption", tileViewIsEnabled.ToString());
            }

            ndList = vm.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/OrthogonalViewSettings");

            if (ndList.Count <= 0)
            {
                XmlNode nodeGen = vm.ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/General");
                if (nodeGen != null)
                {
                    XmlNode nodeOrtho = vm.ApplicationDoc.CreateElement("OrthogonalViewSettings");
                    nodeGen.AppendChild(nodeOrtho);
                    ndList = vm.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/OrthogonalViewSettings");
                }
            }

            if (0 < ndList.Count)
            {
                XmlManager.SetAttribute(ndList[0], vm.ApplicationDoc, "lineColorIndex", _orthogonalLineColorType.ToString());
                XmlManager.SetAttribute(ndList[0], vm.ApplicationDoc, "lineTypeIndex", _orthogonalLineType.ToString());
                XmlManager.SetAttribute(ndList[0], vm.ApplicationDoc, "zMultiplier", vm.OrthogonalViewZMultiplier.ToString());
                string enableStr = true == OrthogonalViewEnable.IsChecked ? "1" : "0";
                XmlManager.SetAttribute(ndList[0], vm.ApplicationDoc, "enableOrthogonalView", enableStr);
            }

            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);

            _persistWhileClosing = false;
        }

        private void LoadROIs_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

            dlg.DefaultExt = ".xaml";
            dlg.Filter = "WPF XAML (.xaml)|*.xaml";

            Nullable<bool> result = dlg.ShowDialog();

            if (true == result)
            {
                bool reticleActive = false;
                bool scaleActive = false;
                OverlayManagerClass.Instance.LoadROIs(dlg.FileName, ref overlayCanvas, ref reticleActive, ref scaleActive);
                ReticleOnOff.IsChecked = reticleActive;
                ScaleOnOff.IsChecked = scaleActive;
            }
            ROIDrawingTools.SelectedIndex = 0;
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
        }

        private void originalButton_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            //verify the data context
            if (vm.Bitmap == null)
            {
                return;
            }

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
            overlayCanvas.RenderTransform = _transformGroup;
            presentationCanvas.RenderTransform = _transformGroup;
            xyOrthogonalImageCanvas.RenderTransform = new MatrixTransform(m);
            if (vm.OrthogonalViewStat != CaptureSetupViewModel.OrthogonalViewStatus.INACTIVE)
            {
                UpdateOrthogonalView(vm.BitmapPoint);
            }

            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        private void OrthogonalDisplayOption_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }
            OrthoViewDispOptWin dlg = new OrthoViewDispOptWin();
            dlg.ColorIndex = _orthogonalLineColorType;
            dlg.LineIndex = _orthogonalLineType;
            dlg.ZPixelMultiplier = vm.OrthogonalViewZMultiplier;
            if (false == dlg.ShowDialog())
            {
                if (dlg.SetFlag == true)
                {
                    int colorIndex = dlg.ColorIndex;
                    int lineTypeIndex = dlg.LineIndex;
                    if (vm.OrthogonalViewZMultiplier != dlg.ZPixelMultiplier)
                    {
                        _orthogonalLineColorType = colorIndex;
                        _orthogonalLineType = lineTypeIndex;
                        vm.OrthogonalViewZMultiplier = dlg.ZPixelMultiplier;
                        if (vm.OrthogonalViewStat != CaptureSetupViewModel.OrthogonalViewStatus.INACTIVE)
                        {
                            SetupOrthogonalView(this.lockPoint);
                            UpdateOrthogonalView(vm.BitmapPoint);
                        }
                    }
                    else   if (colorIndex != _orthogonalLineColorType || lineTypeIndex != _orthogonalLineType)
                    {
                        _orthogonalLineColorType = colorIndex;
                        _orthogonalLineType = lineTypeIndex;

                        if (vm.OrthogonalViewStat != CaptureSetupViewModel.OrthogonalViewStatus.INACTIVE)
                        {
                            UpdateOrthogonalView(vm.BitmapPoint);
                        }
                    }
                }
            }
        }

        private void OrthogonalLinesLockPosition_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            //verify the data context
            if (vm == null || image1.ImageSource == null)
            {
                return;
            }
            if (this.lockPosition.IsChecked == true)
            {
                SetupOrthogonalView(this.lockPoint);
                UpdateOrthogonalView(this.lockPoint);
                vm.OrthogonalViewStat = CaptureSetupViewModel.OrthogonalViewStatus.HOLD;
            }
            else
            {
                vm.OrthogonalViewStat = CaptureSetupViewModel.OrthogonalViewStatus.ACTIVE;
            }
        }

        private void OrthogonalViewEnable_Click(object sender, RoutedEventArgs e)
        {
            ActivateDeactivateOrthogonalView();
        }

        private void OrthogonalVirtualStack_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }
            if (vm.OrthogonalViewStat != CaptureSetupViewModel.OrthogonalViewStatus.INACTIVE)
            {
                vm_CloseOrthogonalView();

                OrthogonalViewEnable.IsChecked = true;

                vm.InitOrthogonalView();

                this.lockPosition.Visibility = Visibility.Visible; // contextMenu lock position is visible
                this.lockPosition.IsChecked = false;
            }
        }

        private void presentationCanvas_MouseMove(object sender, MouseEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if (vm.AutoROIDisplayChannel != 0 && presentationCanvas.Visibility == Visibility.Visible)
            {
                Point presentPoint;
                presentPoint = this.TranslatePoint(e.GetPosition(this), presentationCanvas);
                if (presentationImage.ImageSource != null && presentPoint.X < presentationImage.ImageSource.Width && presentPoint.Y < presentationImage.ImageSource.Height)
                {
                    presentPoint.X = Math.Max(0, Math.Min(presentPoint.X, presentationImage.ImageSource.Width - 0.001));
                    presentPoint.Y = Math.Max(0, Math.Min(presentPoint.Y, presentationImage.ImageSource.Height - 0.001));
                    short value = vm.GetImageProcessImagePixel((int)Math.Floor(presentPoint.X), (int)Math.Floor(presentPoint.Y));
                    if (value != 0)
                    {
                        double x = e.GetPosition((sender as FrameworkElement)).X;
                        double y = e.GetPosition((sender as FrameworkElement)).Y;
                        ROIIndex.Text = "ROI #" + value.ToString();
                        ROIIndex.Visibility = Visibility.Visible;
                        vm.AutoTrackToolTipX = (int)Math.Floor(x);
                        vm.AutoTrackToolTipY = (int)Math.Floor(y);
                    }
                    else
                    {
                        ROIIndex.Visibility = Visibility.Collapsed;
                    }
                }

            }
        }

        /// <summary>
        /// Handles changes in the view model with additional behaviors outside of the xaml bindings. Must be run in the dispatch thread
        /// or it will throw an exception.
        /// </summary>
        /// <param name="sender"> </param>
        /// <param name="e"> </param>
        private void ReactToPropertyChangeEvents(object sender, PropertyChangedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            if (e.PropertyName == "ZoomLevel")
            {
                ZoomDisplay(vm.ZoomLevel);
            }
        }

        /// <summary>
        /// return to 100% scale
        /// </summary>
        /// <param name="e"></param>
        private void ResetClick(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            //verify the data context
            if (vm.Bitmap == null)
            {
                return;
            }

            _currentScale = Math.Min(this.ActualHeight / (vm.Bitmap.Height), this.ActualWidth / (vm.Bitmap.Width));

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
            overlayCanvas.RenderTransform = _transformGroup;
            presentationCanvas.RenderTransform = _transformGroup;
            if (vm.OrthogonalViewStat != CaptureSetupViewModel.OrthogonalViewStatus.INACTIVE)
            {
                UpdateOrthogonalView(vm.BitmapPoint);
            }

            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        private void ReticleOnOff_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                ReticleOnOff.IsChecked = false;
                ROIDrawingTools.SelectedIndex = 0;
                return;
            }
            if (true == ReticleOnOff.IsChecked)
            {
                OverlayManagerClass.Instance.InitReticle(ref overlayCanvas, true);
                ROIDrawingTools.SelectedIndex = 0;
                OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
            }
            else
            {
                OverlayManagerClass.Instance.InitReticle(ref overlayCanvas, false);
                ROIDrawingTools.SelectedIndex = 0;
                OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
                if (0 == overlayCanvas.Children.Count)
                {
                    //Force reset ROI count
                    OverlayManagerClass.Instance.ClearAllObjects(ref overlayCanvas);
                    if (null != vm.MultiROIStats)
                    {
                        vm.MultiROIStats.Hide(); //.Close();
                    }

                    if (null != vm.ROIStatsChart)
                    {
                        vm.ROIStatsChart.Hide(); //.Close();
                        //In CaptureSetup _roiStatsChart is not closed (only hidden)
                        //force the persistance of the window settings
                        vm.PersistROIStatsChartWindowSettings();
                    }

                    //if (null != vm.LineProfile)
                    //{
                    //    vm.LineProfile.Hide(); //.Close();
                    //}
                }
            }
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

        private void rollOverButton_Click(object sender, RoutedEventArgs e)
        {
            UpdateIntensityData();
        }

        private void saveAsReference_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            if (1 == CaptureSetup.GetColorChannels())
            {
                string refChan = Application.Current.Resources["AppRootFolder"].ToString() + "\\ReferenceChannel.tif";

                vm.SaveImage(refChan, 2);

                string msg = string.Format("Reference channel saved to {0}", refChan);
                MessageBox.Show(msg);
            }
            else
            {
                MessageBox.Show("Cannot create a reference channel from a multichannel image. Choose a single channel image instead.");
            }
        }

        private void saveAs_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            // Configure save file dialog box
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Title = "Save an Image File";
            dlg.FileName = string.Format("Image_{0:yyyy-MM-dd_hh-mm-ss}", DateTime.Now);
            ;
            switch (CaptureSetup.GetColorChannels())
            {
                case 1:
                    {
                        dlg.Filter = "8 Bit Tiff file (*.tif)|*.tif|16 Bit Tiff file (*.tif)|*.tif|Jpeg file (*.jpg)|*.jpg";
                    }
                    break;
                default:
                    {
                        dlg.Filter = "24 Bit Tiff file (*.tif)|*.tif|48 Bit Tiff file (*.tif)|*.tif|Jpeg file (*.jpg)|*.jpg";
                    }
                    break;
            }

            dlg.FilterIndex = _saveDlgFilterIndex;

            // Show save file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process save file dialog box results
            if (result == true && dlg.FileName != "")
            {
                // Save the image file
                string filename = dlg.FileName;
                vm.SaveImage(filename, dlg.FilterIndex);
            }

            _saveDlgFilterIndex = dlg.FilterIndex;
        }

        private void SaveImageButton_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.SaveNow();
        }

        private void SaveROIs_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();

            dlg.DefaultExt = ".xaml";
            dlg.Filter = "WPF XAML (.xaml)|*.xaml";

            Nullable<bool> result = dlg.ShowDialog();

            if (true == result)
            {
                OverlayManagerClass.Instance.SaveROIs(dlg.FileName);
            }
            ROIDrawingTools.SelectedIndex = 0;
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
        }

        private void ScaleOnOff_Click(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                ScaleOnOff.IsChecked = false;
                ROIDrawingTools.SelectedIndex = 0;
                return;
            }
            if (true == ScaleOnOff.IsChecked)
            {
                OverlayManagerClass.Instance.InitScale(ref overlayCanvas, true);
                ROIDrawingTools.SelectedIndex = 0;
                OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
            }
            else
            {
                OverlayManagerClass.Instance.InitScale(ref overlayCanvas, false);
                ROIDrawingTools.SelectedIndex = 0;
                OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
                if (0 == overlayCanvas.Children.Count)
                {
                    //Force reset ROI count
                    OverlayManagerClass.Instance.ClearAllObjects(ref overlayCanvas);
                    if (null != vm.MultiROIStats)
                    {
                        vm.MultiROIStats.Hide(); //.Close();
                    }

                    if (null != vm.ROIStatsChart)
                    {
                        vm.ROIStatsChart.Hide(); //.Close();
                        //In CaptureSetup _roiStatsChart is not closed (only hidden)
                        //force the persistance of the window settings
                        vm.PersistROIStatsChartWindowSettings();
                    }
                }
            }
        }

        private void SelectROI_Selected(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
        }

        private void SetupOrthogonalView(Point point)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            //verify the data context
            if (vm == null || image1.ImageSource == null)
            {
                return;
            }
            vm.BitmapPoint = point;
            vm.UpdateOrthogonalViewImages();
        }

        private void slider0_DragStarted(object sender, DragStartedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
        }

        private void slider0_MouseLeftClick(object sender, MouseButtonEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
        }

        private void slider1_DragStarted(object sender, DragStartedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
        }

        private void slider1_MouseLeftClick(object sender, MouseButtonEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
        }

        private void slider2_DragStarted(object sender, DragStartedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
        }

        private void slider2_MouseLeftClick(object sender, MouseButtonEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
        }

        private void slider3_DragStarted(object sender, DragStartedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
        }

        private void slider3_MouseLeftClick(object sender, MouseButtonEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
        }

        private void UpdateHistogramData()
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            //update the data when the capture is active and the control is visible
            if (histogramEnable.IsChecked == true)
            {
                if (vm.LSMChannel == MAX_HISTOGRAMS)
                {
                    for (int i = 0; i < MAX_HISTOGRAMS; i++)
                    {
                        Visibility vis;

                        bool isEnabled = false;

                        switch (i)
                        {
                            case 0: isEnabled = vm.LSMChannelEnable0; break;
                            case 1: isEnabled = vm.LSMChannelEnable1; break;
                            case 2: isEnabled = vm.LSMChannelEnable2; break;
                            case 3: isEnabled = vm.LSMChannelEnable3; break;
                        }

                        if (isEnabled)
                        {
                            vis = Visibility.Visible;

                            Color color = vm.GetColorAssignment(i);

                            switch (i)
                            {
                                case 0:
                                    histogram1.Data = vm.HistogramData0;
                                    histogram1.DataBrushColor = color;
                                    if (AutoManualTog1.IsChecked == true) AutoEnhance1();
                                    break;
                                case 1:
                                    histogram2.Data = vm.HistogramData1;
                                    histogram2.DataBrushColor = color;
                                    if (AutoManualTog2.IsChecked == true) AutoEnhance2();
                                    break;
                                case 2:
                                    histogram3.Data = vm.HistogramData2;
                                    histogram3.DataBrushColor = color;
                                    if (AutoManualTog3.IsChecked == true) AutoEnhance3();
                                    break;
                                case 3:
                                    histogram4.Data = vm.HistogramData3;
                                    histogram4.DataBrushColor = color;
                                    if (AutoManualTog4.IsChecked == true) AutoEnhance4();
                                    break;
                            }

                        }
                        else
                        {
                            vis = Visibility.Collapsed;
                        }

                        switch (i)
                        {
                            case 0: panel1.Visibility = vis; break;
                            case 1: panel2.Visibility = vis; break;
                            case 2: panel3.Visibility = vis; break;
                            case 3: panel4.Visibility = vis; break;
                        }
                    }
                }
                else
                {
                    if (true == vm.LSMChannelEnable0)
                    {
                        panel1.Visibility = Visibility.Visible;
                        panel2.Visibility = Visibility.Collapsed;
                        panel3.Visibility = Visibility.Collapsed;
                        panel4.Visibility = Visibility.Collapsed;

                        histogram1.Data = vm.HistogramData0;
                        histogram1.DataBrushColor = Colors.Black;
                        if (AutoManualTog1.IsChecked == true) AutoEnhance1();
                    }
                    else if (true == vm.LSMChannelEnable1)
                    {
                        panel1.Visibility = Visibility.Collapsed;
                        panel2.Visibility = Visibility.Visible;
                        panel3.Visibility = Visibility.Collapsed;
                        panel4.Visibility = Visibility.Collapsed;

                        histogram2.Data = vm.HistogramData0;
                        histogram2.DataBrushColor = Colors.Black;
                        if (AutoManualTog2.IsChecked == true) AutoEnhance2();
                    }
                    else if (true == vm.LSMChannelEnable2)
                    {
                        panel1.Visibility = Visibility.Collapsed;
                        panel2.Visibility = Visibility.Collapsed;
                        panel3.Visibility = Visibility.Visible;
                        panel4.Visibility = Visibility.Collapsed;

                        histogram3.Data = vm.HistogramData0;
                        histogram3.DataBrushColor = Colors.Black;
                        if (AutoManualTog3.IsChecked == true) AutoEnhance3();
                    }
                    else if (true == vm.LSMChannelEnable3)
                    {
                        panel1.Visibility = Visibility.Collapsed;
                        panel2.Visibility = Visibility.Collapsed;
                        panel3.Visibility = Visibility.Collapsed;
                        panel4.Visibility = Visibility.Visible;

                        histogram4.Data = vm.HistogramData0;
                        histogram4.DataBrushColor = Colors.Black;
                        if (AutoManualTog4.IsChecked == true) AutoEnhance4();
                    }
                }

            }

            UpdateIntensityData();
        }

        private void UpdateIntensityData()
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            if (vm.LSMChannel == 4)
            {
                labAVal.Content = "A Val";

                for (int i = 0; i < vm.MaxChannels; i++)
                {
                    Visibility vis;

                    bool isEnabled = false;

                    switch (i)
                    {
                        case 0: isEnabled = vm.LSMChannelEnable0; break;
                        case 1: isEnabled = vm.LSMChannelEnable1; break;
                        case 2: isEnabled = vm.LSMChannelEnable2; break;
                        case 3: isEnabled = vm.LSMChannelEnable3; break;
                    }

                    if (isEnabled)
                    {
                        vis = Visibility.Visible;
                    }
                    else
                    {
                        vis = Visibility.Collapsed;
                    }

                    switch (i)
                    {
                        case 0: panelAVal.Visibility = vis; break;
                        case 1: panelBVal.Visibility = vis; break;
                        case 2: panelCVal.Visibility = vis; break;
                        case 3: panelDVal.Visibility = vis; break;
                    }
                }
            }
            else
            {
                labAVal.Content = "Val";
                panelAVal.Visibility = Visibility.Visible;
                panelBVal.Visibility = Visibility.Collapsed;
                panelCVal.Visibility = Visibility.Collapsed;
                panelDVal.Visibility = Visibility.Collapsed;
            }
        }

        private void UpdateOrthogonalView(Point point)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            //verify the data context
            if (vm == null || image1.ImageSource == null)
            {
                return;
            }

            UpdateOrthogonalViewLines(xzOrthogonalImageCanvas, xzOrthogonalImageCanvas, new Point(point.X, vm.ZNumSteps - 1), _orthogonalXZViewLine, LineDirection.vertical);
            UpdateOrthogonalViewLines(yzOrthogonalImageCanvas, yzOrthogonalImageCanvas, new Point(vm.ZNumSteps - 1, point.Y), _orthogonalYZViewLine, LineDirection.horizontal);
            UpdateOrthogonalViewLines(imageCanvas, xyOrthogonalImageCanvas, point, _orthogonalViewLine, LineDirection.horizontalAndVertical);

            _orthogonalViewMatrix = imageCanvas.RenderTransform.Value;
            _orthogonalViewMatrix.OffsetY += _currentScale * image1.ImageSource.Height + 10;
            xzOrthogonalImageCanvas.RenderTransform = new MatrixTransform(_orthogonalViewMatrix);
            _orthogonalViewMatrix = imageCanvas.RenderTransform.Value;
            _orthogonalViewMatrix.OffsetX += _currentScale * image1.ImageSource.Width + 10;
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
                        line[i].Stroke = _brushColors[_orthogonalLineColorType];
                        line[i].StrokeDashArray = _orthogonalLineType == 0 ? new DoubleCollection() { 2 } : null;
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
                            line[i].Stroke = _brushColors[_orthogonalLineColorType];
                            line[i].StrokeThickness = Math.Max(1, Math.Min(1 / _currentScale, 10));
                            if (_orthogonalLineType == 0)
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
            catch(Exception ex)
            {
                ex.ToString();
            }
        }

        private void UpdateRollOverPixelData(Point imagePixelLocation)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.RollOverPointX = (int)Math.Floor(imagePixelLocation.X);
            vm.RollOverPointY = (int)Math.Floor(imagePixelLocation.Y);

            rollOverTextX.Text = vm.RollOverPointX.ToString();
            rollOverTextY.Text = vm.RollOverPointY.ToString();
            rollOverTextInt0.Text = vm.RollOverPointIntensity0.ToString();
            rollOverTextInt1.Text = vm.RollOverPointIntensity1.ToString();
            rollOverTextInt2.Text = vm.RollOverPointIntensity2.ToString();
            rollOverTextInt3.Text = vm.RollOverPointIntensity3.ToString();
        }

        void VM_ActiveSettingsReplaced()
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }
            bool reticleActive = false;
            bool scaleActive = false;
            OverlayManagerClass.Instance.PersistLoadROIs(ref overlayCanvas, ref reticleActive, ref scaleActive);
            ReticleOnOff.IsChecked = reticleActive;
            ScaleOnOff.IsChecked = scaleActive;
        }

        private void VM_BleachROICheck()
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            //default as false, since it may take time at
            //comparing ROIs based on file hashing:
            vm.IsROIExtracted = false;
            bool result = false;
            BleachROIStateChecker stillChecking = () => { return _bleachROIChecker.IsBusy; };

            if (_bleachROIChecker.IsBusy)
            {
                _bleachROIChecker.CancelAsync();
                if ((bool)this.Dispatcher.Invoke(stillChecking, null))
                {
                    System.Threading.Thread.Sleep(50);
                    return;
                }
            }

            _bleachROIChecker.RunWorkerAsync();

            _bleachROIChecker.DoWork += delegate(object sender, DoWorkEventArgs e)
            {
                result = OverlayManagerClass.Instance.BleachCompareROIs(vm.BleachROIPath);
            };

            _bleachROIChecker.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs e)
            {
                vm.IsROIExtracted = result;
            };
        }

        void VM_BleachROIsExtractedEvent()
        {
            ROIDrawingTools.SelectedIndex = 0;
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
        }

        void vm_CloseOrthogonalView()
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }
            OrthogonalViewEnable.IsChecked = false;

            vm.OrthogonalViewStat = CaptureSetupViewModel.OrthogonalViewStatus.INACTIVE;

            xyOrthogonalImageCanvas.Children.Clear();
            xzOrthogonalImageCanvas.Children.Clear();
            yzOrthogonalImageCanvas.Children.Clear();

            this.lockPosition.IsChecked = false;
            this.lockPosition.Visibility = Visibility.Collapsed;  //hide the lock position option
            this.xzOrthogonalImageCanvas.Visibility = Visibility.Collapsed; //hide the xzImage and yzImage
            this.yzOrthogonalImageCanvas.Visibility = Visibility.Collapsed;
        }

        void VM_ImageDataChanged(bool obj)
        {
            _imageDataChanged = true;
        }

        void VM_LineScanEnvent()
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            OverlayManagerClass.Instance.ClearAllObjects(ref overlayCanvas);
            ReticleOnOff.IsChecked = false;
            ScaleOnOff.IsChecked = false;
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);

            //The second point must be less than pixel x, because otherwise an extra pixel would be included
            //since the canvas is zero based, and pixelX and pixelY are one based
            OverlayManagerClass.Instance.CreateROIShape(ref overlayCanvas, typeof(Line), new Point(0, 0.5), new Point((int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)512] - 0.01, 0.5));
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
        }

        void vm_OrthogonalViewImagesLoaded(bool cancelFlag)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }
            if (!cancelFlag)
            {
                if (vm.OrthogonalViewStat == CaptureSetupViewModel.OrthogonalViewStatus.INACTIVE)
                {
                    vm.OrthogonalViewStat = CaptureSetupViewModel.OrthogonalViewStatus.ACTIVE;//set viewmodel orthogonal view flag to true
                }

                UpdateOrthogonalView(vm.BitmapPoint);

                this.xzOrthogonalImageCanvas.Visibility = Visibility.Visible;
                this.yzOrthogonalImageCanvas.Visibility = Visibility.Visible;
            }
            else
            {
                if (this.xzOrthogonalImageCanvas.Visibility == Visibility.Collapsed)
                {
                    vm_CloseOrthogonalView();
                }
            }
        }

        /// <summary>
        /// Handles changes in the view model with additional behaviors outside of the xaml bindings. Invokes all
        /// work to be done on the dispatch thread.
        /// </summary>
        /// <param name="sender"> </param>
        /// <param name="e"> </param>
        private void vm_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            Application.Current.Dispatcher.BeginInvoke(new Action(() => ReactToPropertyChangeEvents(sender, e)));
        }

        void VM_ROIUpdateRequested(string pathName)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if ((vm == null) || (vm.Bitmap == null))
            {
                return;
            }
            if (File.Exists(pathName))
            {
                //remove existing ROIs:
                OverlayManagerClass.Instance.ClearAllObjects(ref overlayCanvas);
                ReticleOnOff.IsChecked = false;
                ScaleOnOff.IsChecked = false;
                ROIDrawingTools.SelectedIndex = 0;
                OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);

                //load from user-defined ROI file: could be BleachROIs, SLMCalibROIs, ...
                OverlayManagerClass.Instance.UserLoadROIs(pathName, ref overlayCanvas);
            }
        }

        private void VM_UpdatingObject(bool obj)
        {
            _updatingOverlayObject = obj;
        }

        /// <summary>
        /// Zooms the displayed image to the desired percent. 
        /// </summary>
        /// <param name="zoomPercent"> Percent in integer values ie. 100 is no zoom </param>
        private void ZoomDisplay(double zoomPercent)
        {
            if (Math.Abs(_currentScale - zoomPercent) < .000001)
            {
                return;
            }

            _currentScale = zoomPercent;

            _scaleTransform.ScaleX = _currentScale;
            _scaleTransform.ScaleY = _currentScale;

            imageCanvas.RenderTransform = _transformGroup;
            overlayCanvas.RenderTransform = _transformGroup;
            presentationCanvas.RenderTransform = _transformGroup;
            xyOrthogonalImageCanvas.RenderTransform = _transformGroup;

            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm.OrthogonalViewStat != CaptureSetupViewModel.OrthogonalViewStatus.INACTIVE)
            {
                UpdateOrthogonalView(vm.BitmapPoint);
            }

            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        void _histogramUpdateTimer_Tick(object sender, EventArgs e)
        {
            if (_imageDataChanged)
            {
                CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
                if (vm == null)
                {
                    return;
                }
                lock (vm.HistogramDataLock)
                {
                    UpdateHistogramData();
                }

                //Update the scrollbar height when the image data changes
                vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;

                _imageDataChanged = false;
            }
        }

        #endregion Methods
    }
}