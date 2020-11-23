namespace ImageReviewDll.View
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
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

    using ImageReviewDll.Model;
    using ImageReviewDll.ViewModel;

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

        private Point lockPoint;
        Matrix m = new Matrix();
        private SolidColorBrush[] _brushColors = new SolidColorBrush[] { Brushes.White, Brushes.Black, Brushes.Green, Brushes.Blue, Brushes.Yellow, Brushes.Red };
        private int _colorType = 0;
        private Point _currentOffset;
        private double _currentScale;
        private int _lineType = 0;
        Point _newOrigin;
        Point _newStart;
        bool _nonVitualStackObjectMovingFlag = false;
        private Line[] _orthogonalViewLine = new Line[2];
        private Matrix _orthogonalViewMatrix;
        private Line[] _orthogonalXZViewLine = new Line[2];
        private Line[] _orthogonalYZViewLine = new Line[2];
        Point _p;
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

            //  ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
            _shiftDown = false;
            this.Loaded += new RoutedEventHandler(ImageView_Loaded);

            _currentOffset.X = 0;
            _currentOffset.Y = 0;

            _translateTransform = new TranslateTransform();
            _scaleTransform = new ScaleTransform();
            _transformGroup = new TransformGroup();

            _transformGroup.Children.Add(_scaleTransform);
            _transformGroup.Children.Add(_translateTransform);

            imageCanvas.RenderTransform = _transformGroup;
            overlayCanvas.RenderTransform = _transformGroup;
            xyImageCanvas.RenderTransform = _transformGroup;

            _updatingOverlayObject = false;

            histogram0.DataBrushColor = Colors.Black;
            histogram1.DataBrushColor = Colors.Red;
            histogram2.DataBrushColor = Colors.Green;
            histogram3.DataBrushColor = Colors.Blue;
            histogram0.BwTextBoxFocued += new EventHandler(H1_BWTextBoxFocused);
            histogram1.BwTextBoxFocued += new EventHandler(H2_BWTextBoxFocused);
            histogram2.BwTextBoxFocued += new EventHandler(H3_BWTextBoxFocused);
            histogram3.BwTextBoxFocued += new EventHandler(H4_BWTextBoxFocused);

            _currentScale = 1.0;
            zoomSlider.Value = _currentScale;
            zoomText.Text = ConvertScaleToPercent(_currentScale);

            histogram0.IsVisibleChanged += new DependencyPropertyChangedEventHandler(Histogram_IsVisibleChanged);
            this.SizeChanged += new SizeChangedEventHandler(ImageView_SizeChanged);
        }

        #endregion Constructors

        #region Enumerations

        private enum MouseEvent
        {
            LEFTSINGLECLICK,
            RIGHTSINGLECLICK,
            LEFTDOUBLECLICK,
            RIGHTDOUBLECLICK,
            LEFTHOLDING,
            RIGHTHOLDING,
            LEFTMOUSEUP,
            RIGHTMOUSEUP
        }

        #endregion Enumerations

        #region Methods

        public void ImageView_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.LeftShift || e.Key == Key.RightShift)
            {
                _shiftDown = true;
            }
            else if (e.Key == Key.Delete || e.Key == Key.Back)
            {
                ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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
                if (0 == overlayCanvas.Children.Count)
                {
                    //Force reset ROI count
                    OverlayManagerClass.Instance.ClearAllObjects(ref overlayCanvas);
                }
            }
            else if (e.Key == Key.A && (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
            {
                ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
                if (vm == null)
                {
                    return;
                }
                ROIDrawingTools.SelectedIndex = 0;
                if (null == vm.Bitmap)
                {
                    return;
                }
                OverlayManagerClass.Instance.SelectAllROIs();
            }
            else if (e.Key == Key.Tab)
            {
                ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            ROIDrawingTools.SelectedIndex = 0;
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
        }

        protected override void OnMouseDoubleClick(MouseButtonEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            MouseEvent me = MouseEvent.LEFTDOUBLECLICK;
            OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), ref overlayCanvas, new Point(_scrollStartPoint.X, _scrollStartPoint.Y), _shiftDown);
        }

        /// <summary>
        /// Get position and CaptureMouse
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (vm == null)
            {
                return;
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
                    if (e.ChangedButton == MouseButton.Left)   // left click
                    {
                        MouseEvent me = new MouseEvent();
                        me = MouseEvent.LEFTSINGLECLICK;
                        OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), ref overlayCanvas, new Point(_scrollStartPoint.X, _scrollStartPoint.Y), _shiftDown);
                        vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
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
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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
                        MouseEvent me = new MouseEvent();
                        me = MouseEvent.LEFTHOLDING;
                        Point currentPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
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
                    xyImageCanvas.RenderTransform = new MatrixTransform(m);
                    if (vm.OrthogonalViewStat != ImageReviewViewModel.OrthogonalViewStatus.INACTIVE)
                    {
                        m.OffsetY += _currentScale * image1.ImageSource.Height + 10;
                        xzImageCanvas.RenderTransform = new MatrixTransform(m);

                        m.OffsetY -= _currentScale * image1.ImageSource.Height + 10;
                        m.OffsetX += _currentScale * image1.ImageSource.Width + 10;
                        yzImageCanvas.RenderTransform = new MatrixTransform(m);
                        if (pp != _newStart)
                        {
                            _nonVitualStackObjectMovingFlag = true;
                        }
                    }
                }
            }
            if (vm.OrthogonalViewStat == ImageReviewViewModel.OrthogonalViewStatus.ACTIVE && IsMouseCaptured == false && image1.ImageSource != null && imageCanvas.ContextMenu.IsOpen == false && lockPosition.IsChecked == false)
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
            vm.RollOverPointX = (int)Math.Floor(imagePixelLocation.X);
            vm.RollOverPointY = (int)Math.Floor(imagePixelLocation.Y);

            rollOverTextX.Text = vm.RollOverPointX.ToString();
            rollOverTextY.Text = vm.RollOverPointY.ToString();
            rollOverTextInt0.Text = vm.RollOverPointIntensity0.ToString();
            rollOverTextInt1.Text = vm.RollOverPointIntensity1.ToString();
            rollOverTextInt2.Text = vm.RollOverPointIntensity2.ToString();
            rollOverTextInt3.Text = vm.RollOverPointIntensity3.ToString();

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
                ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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

                if (e.ChangedButton == MouseButton.Left)
                {
                    MouseEvent me = new MouseEvent();
                    me = MouseEvent.LEFTMOUSEUP;
                    Point currentPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                    currentPoint.X = Math.Max(0, Math.Min(currentPoint.X, image1.ImageSource.Width - 0.001));
                    currentPoint.Y = Math.Max(0, Math.Min(currentPoint.Y, image1.ImageSource.Height - 0.001));
                    OverlayManagerClass.Instance.MouseEvent(Convert.ToInt16(me), ref overlayCanvas, new Point(currentPoint.X, currentPoint.Y), _shiftDown);
                    vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
                    if (vm.VirtualZStack == false && vm.OrthogonalViewStat == ImageReviewViewModel.OrthogonalViewStatus.ACTIVE && _nonVitualStackObjectMovingFlag == false)
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
                if (e.ChangedButton == MouseButton.Right && vm.OrthogonalViewStat == ImageReviewViewModel.OrthogonalViewStatus.ACTIVE && vm.VirtualZStack == true)
                {
                    lockPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                    lockPoint.X = Math.Max(0, Math.Min(lockPoint.X, image1.ImageSource.Width - 0.001));
                    lockPoint.Y = Math.Max(0, Math.Min(lockPoint.Y, image1.ImageSource.Height - 0.001));
                }

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

            _p = e.MouseDevice.GetPosition(imageCanvas);

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
            m.ScaleAtPrepend(scaleFactor, scaleFactor, _p.X, _p.Y);
            _currentScale = nextScale;

            imageCanvas.RenderTransform = new MatrixTransform(m);
            overlayCanvas.RenderTransform = new MatrixTransform(m);
            xyImageCanvas.RenderTransform = new MatrixTransform(m);
            zoomSlider.Value = m.M22;

            e.Handled = true;

            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        private void AutoEnhance1()
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
                return;
            histogram0.MaxBinValue = _whitePointMaxVal;
            histogram0.BlackPoint = histogram0.MinValue;
            histogram0.WhitePoint = histogram0.MaxValue;

            slBp0.Value = histogram0.MinValue;
            slWp0.Value = histogram0.MaxValue;
        }

        private void AutoEnhance2()
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
                return;
            histogram1.MaxBinValue = _whitePointMaxVal;
            histogram1.BlackPoint = histogram1.MinValue;
            slBp1.Value = histogram1.MinValue;
            histogram1.WhitePoint = histogram1.MaxValue;
            slWp1.Value = histogram1.MaxValue;
        }

        private void AutoEnhance3()
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
                return;
            histogram2.MaxBinValue = _whitePointMaxVal;
            histogram2.BlackPoint = histogram2.MinValue;
            slBp2.Value = histogram2.MinValue;
            histogram2.WhitePoint = histogram2.MaxValue;
            slWp2.Value = histogram2.MaxValue;
        }

        private void AutoEnhance4()
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
                return;
            histogram3.MaxBinValue = _whitePointMaxVal;
            histogram3.BlackPoint = histogram3.MinValue;
            slBp3.Value = histogram3.MinValue;
            histogram3.WhitePoint = histogram3.MaxValue;
            slWp3.Value = histogram3.MaxValue;
        }

        private void btnBPDown0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram0.BlackPoint -= 1;
            slBp0.Value -= 1;
        }

        private void btnBPDown1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram1.BlackPoint -= 1;
            slBp1.Value -= 1;
        }

        private void btnBPDown2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram2.BlackPoint -= 1;
            slBp2.Value -= 1;
        }

        private void btnBPDown3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram3.BlackPoint -= 1;
            slBp3.Value -= 1;
        }

        private void btnBPUp0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram0.BlackPoint += 1;
            slBp0.Value += 1;
        }

        private void btnBPUp1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram1.BlackPoint += 1;
            slBp1.Value += 1;
        }

        private void btnBPUp2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram2.BlackPoint += 1;
            slBp2.Value += 1;
        }

        private void btnBPUp3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram3.BlackPoint += 1;
            slBp3.Value += 1;
        }

        private void btnWPDown0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram0.WhitePoint -= 1;
            slWp0.Value -= 1;
        }

        private void btnWPDown1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram1.WhitePoint -= 1;
            slWp1.Value -= 1;
        }

        private void btnWPDown2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram2.WhitePoint -= 1;
            slWp2.Value -= 1;
        }

        private void btnWPDown3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram3.WhitePoint -= 1;
            slWp3.Value -= 1;
        }

        private void btnWPUp0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram0.WhitePoint += 1;
            slWp0.Value += 1;
        }

        private void btnWPUp1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram1.WhitePoint += 1;
            slWp1.Value += 1;
        }

        private void btnWPUp2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram2.WhitePoint += 1;
            slWp2.Value += 1;
        }

        private void btnWPUp3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram3.WhitePoint += 1;
            slWp3.Value += 1;
        }

        private void ButtonReset_Click0(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;

            histogram0.BlackPoint = 0;
            histogram0.WhitePoint = 255;
            slBp0.Value = 0;
            slWp0.Value = 255;
            SendUpdateToBitmap();

            //            histogram0.LogScaleData = false;
        }

        private void ButtonReset_Click1(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;

            histogram1.BlackPoint = 0;
            histogram1.WhitePoint = 255;
            slBp1.Value = 0;
            slWp1.Value = 255;
            SendUpdateToBitmap();

            //            histogram1.LogScaleData = false;
        }

        private void ButtonReset_Click2(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;

            histogram2.BlackPoint = 0;
            histogram2.WhitePoint = 255;
            slBp2.Value = 0;
            slWp2.Value = 255;
            SendUpdateToBitmap();

            //            histogram2.LogScaleData = false;
        }

        private void ButtonReset_Click3(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;

            histogram3.BlackPoint = 0;
            histogram3.WhitePoint = 255;
            slBp3.Value = 0;
            slWp3.Value = 255;
            SendUpdateToBitmap();

            //            histogram3.LogScaleData = false;
        }

        private void ClearAllROIs_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if (MessageBoxResult.Yes == MessageBox.Show(new Window() { Topmost = true }, "Do you wish to delete all ROIs?", "Clear ROIs?", MessageBoxButton.YesNo, MessageBoxImage.Question, MessageBoxResult.Yes))
            {

                ReticleOnOff.IsChecked = false;
                ScaleOnOff.IsChecked = false;
                OverlayManagerClass.Instance.ClearAllObjects(ref overlayCanvas);
                ROIDrawingTools.SelectedIndex = 0;
                OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
            }
        }

        string ConvertScaleToPercent(double scale)
        {
            try
            {
                Decimal dec = new Decimal(scale * 100);
                string str = Decimal.Round(dec, 0).ToString() + "%";

                return str;
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        private void createCrosshairROI_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                ROIDrawingTools.SelectedIndex = -1;
                return;
            }
            OverlayManagerClass.Instance.InitROICrosshair(ref overlayCanvas);
        }

        private void createEllipseROI_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            OverlayManagerClass.Instance.InitROILine(ref overlayCanvas);
        }

        private void createLineROI_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            OverlayManagerClass.Instance.InitROIPolyline(ref overlayCanvas);
        }

        private void createPolylineROI_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            _roiToolMouseDoubleClickFlag = true;
            OverlayManagerClass.Instance.InitROIPolylineWithOptions(ref overlayCanvas);
        }

        private void createPolyROI_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            OverlayManagerClass.Instance.InitROIPoly(ref overlayCanvas);
        }

        private void createRectROI_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            OverlayManagerClass.Instance.InitROIRect(ref overlayCanvas);
        }

        private void createReticle_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            OverlayManagerClass.Instance.InitReticle(ref overlayCanvas, true);
        }

        private void DeleteSelectedROIs(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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
            if (0 == overlayCanvas.Children.Count)
            {
                //Force reset ROI count
                OverlayManagerClass.Instance.ClearAllObjects(ref overlayCanvas);
            }
            ROIDrawingTools.SelectedIndex = 0;
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
        }

        private void displayOption_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }
            OrthoViewDispOptWin dlg = new OrthoViewDispOptWin();
            int colorIndex = 0;
            int lineTypeIndex = 0;
            if (false == dlg.ShowDialog())
            {
                if (dlg.SetFlag == true)
                {
                    colorIndex = dlg.ColorIndex;
                    lineTypeIndex = dlg.LineIndex;
                    if (colorIndex != _colorType || lineTypeIndex != _lineType)
                    {
                        _colorType = colorIndex;
                        _lineType = lineTypeIndex;
                        if (vm.OrthogonalViewStat != ImageReviewViewModel.OrthogonalViewStatus.INACTIVE)
                        {
                            UpdateOrthogonalView(vm.BitmapPoint);
                        }

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

        private void histogramEnable_Checked(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }
            UpdateHistogramData();
        }

        /// <summary>
        /// visibility of the histogram control changed
        /// </summary>
        /// <param name="e"></param>
        void Histogram_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }
            // UpdateHistogramData();
        }

        void ImageView_Loaded(object sender, RoutedEventArgs e)
        {
            if (System.Environment.OSVersion.Version.Major <= 5)
            {
                //force the control into software rendering mode
                //there is a memory leak in the .net 3.51 version
                //*TODO* remove when new frame is used and memory leak is resolved
                HwndSource hwndSource = PresentationSource.FromVisual(this) as HwndSource;

                if (hwndSource != null)
                {
                    HwndTarget hwndTarget = hwndSource.CompositionTarget;

                    // this is the new WPF API to force render mode.

                    hwndTarget.RenderMode = RenderMode.SoftwareOnly;
                }
            }

            _updatingOverlayObject = false;
            ROIDrawingTools.SelectedIndex = 0;

            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            //All the View Model event registration needs to happen at the loaded event because
            //the view model is still null when construction.
            //To avoid being registered to an event more than once we first try to unregister from
            //these events and then register again
            vm.ImageDataChanged -= new Action<bool>(VM_ImageDataChanged);
            vm.ImageDataChanged += new Action<bool>(VM_ImageDataChanged);
            OverlayManagerClass.Instance.UpdatingObjectEvent -= new Action<bool>(VM_UpdatingObject);
            OverlayManagerClass.Instance.UpdatingObjectEvent += new Action<bool>(VM_UpdatingObject);
            vm.ExperimentPathChanged -= vm_ExperimentPathChanged;
            vm.ExperimentPathChanged += vm_ExperimentPathChanged;
            vm.ZChanged -= vm_ZChanged;
            vm.ZChanged += vm_ZChanged;
            vm.AnalysisLoaded -= vm_AnalysisLoaded;
            vm.AnalysisLoaded += vm_AnalysisLoaded;
            vm.OrthogonalViewImagesLoaded -= vm_OrthogonalViewImagesLoaded;
            vm.OrthogonalViewImagesLoaded += vm_OrthogonalViewImagesLoaded;
            OverlayManagerClass.Instance.UpdateParams(vm.ImageReview.ImageWidth, vm.ImageReview.ImageHeight, ExperimentData.LSMUMPerPixel);
            bool reticleActive = false;
            bool scaleActive = false;
            OverlayManagerClass.Instance.LoadROIs(vm.ROIsDirectory + "\\ROIs.xaml", ref overlayCanvas, ref reticleActive, ref scaleActive);
            ReticleOnOff.IsChecked = reticleActive;
            ScaleOnOff.IsChecked = scaleActive;
            vm.CloseOrthogonalView -= vm_CloseOrthogonalView;
            vm.CloseOrthogonalView += vm_CloseOrthogonalView;
            vm.PropertyChanged -= vm_PropertyChanged;
            vm.PropertyChanged += vm_PropertyChanged;
            vm.LoadLineProfileData();

            XmlNode node = vm.ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/General/HistogramSettings");
            if (null != node)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(node, vm.ApplicationDoc, "ReducedBinValue", ref str))
                {
                    _whitePointMaxVal = Convert.ToInt32(str);
                    if (0 >= _whitePointMaxVal || 255 < _whitePointMaxVal)
                    {
                        _whitePointMaxVal = 255;
                    }
                }
            }
        }

        void ImageView_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            mainGrid.Height = e.NewSize.Height;
            mainGrid.Width = e.NewSize.Width;
            toolbarGrid.Width = e.NewSize.Width;
        }

        private void LoadROIs_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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

        private void lockPosition_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null || image1.ImageSource == null)
            {
                return;
            }
            if (this.lockPosition.IsChecked == true)
            {
                SetupOrthogonalView(this.lockPoint);
                UpdateOrthogonalView(this.lockPoint);
                vm.OrthogonalViewStat = ImageReviewViewModel.OrthogonalViewStatus.HOLD;
            }
            else
            {
                vm.OrthogonalViewStat = ImageReviewViewModel.OrthogonalViewStatus.ACTIVE;
            }
        }

        private void originalButton_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;

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
            zoomText.Text = "100%";

            double translateX = _translateTransform.X;
            double translateY = _translateTransform.Y;

            // translate the image back to origin in X Y coordinates
            _translateTransform.X -= translateX;
            _translateTransform.Y -= translateY;

            imageCanvas.RenderTransform = _transformGroup;
            overlayCanvas.RenderTransform = _transformGroup;
            xyImageCanvas.RenderTransform = _transformGroup;
            if (vm.OrthogonalViewStat != ImageReviewViewModel.OrthogonalViewStatus.INACTIVE)
            {
                UpdateOrthogonalView(vm.BitmapPoint);
            }

            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        /// <summary>
        /// Handles the Click event of the OrthogonalViewEnable control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void OrthogonalViewEnable_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;

            if (vm != null && image1.ImageSource != null && OrthogonalViewEnable.IsChecked == true)
            {

                vm.BitmapPoint = new Point(image1.ImageSource.Width / 2, image1.ImageSource.Height / 2); //initiation with middle point

                vm.InitOrthogonalView();

                if (vm.VirtualZStack == true)
                {
                    this.lockPosition.Visibility = Visibility.Visible; // contextMenu lock position is visible
                    this.lockPosition.IsChecked = false;
                }
            }
            else//disable the orthogonal view
            {
                vm_CloseOrthogonalView();
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
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;

            //verify the data context
            if (vm.Bitmap == null)
            {
                return;
            }

            _currentScale = Math.Min(this.ActualHeight / (vm.Bitmap.Height), this.ActualWidth / (vm.Bitmap.Width));
            _scaleTransform.ScaleX = _currentScale;
            _scaleTransform.ScaleY = _currentScale;

            _scaleTransform.CenterX = 0;
            _scaleTransform.CenterY = 0;

            zoomSlider.Value = _currentScale;
            zoomText.Text = ConvertScaleToPercent(_currentScale);

            double translateX = _translateTransform.X;
            double translateY = _translateTransform.Y;

            _translateTransform.X -= translateX;
            _translateTransform.Y -= translateY;

            imageCanvas.RenderTransform = _transformGroup;
            overlayCanvas.RenderTransform = _transformGroup;
            if (vm.OrthogonalViewStat != ImageReviewViewModel.OrthogonalViewStatus.INACTIVE)
            {
                UpdateOrthogonalView(vm.BitmapPoint);
            }

            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        private void ReticleOnOff_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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

        private void saveAs_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;

            // Configure save file dialog box
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Title = "Save an Image File";
            dlg.FileName = "Image";

            switch (vm.ImageReview.ImageColorChannels)
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

        private void SaveROIs_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (null == vm.Bitmap)
            {
                vm.IsScaleOnOffChecked = false;
                ROIDrawingTools.SelectedIndex = 0;
                return;
            }
            if (true == vm.IsScaleOnOffChecked)
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
                }
            }
        }

        private void SelectROI_Selected(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
        }

        private void SendUpdateToBitmap()
        {
            //logic in bitmap prevents it from being updated
            //rapidly. Therefore when the blackpoint and whitepoint
            //are set in succession the second property may get
            //reject by the bitmap. To work around this introduce
            //a delay before telling the bitmap to update

            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
                return;
            System.Threading.Thread.Sleep(10);

            vm.UpdateBitmapAndEventSubscribers();
        }

        private void SetHistogramColor(int index, Color color)
        {
            switch (index)
            {
                case 0: histogram0.DataBrushColor = color; break;
                case 1: histogram1.DataBrushColor = color; break;
                case 2: histogram2.DataBrushColor = color; break;
                case 3: histogram3.DataBrushColor = color; break;
            }
        }

        private void SetupOrthogonalView(Point point)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
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

        private void slider1_DragStarted(object sender, DragStartedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
        }

        private void slider2_DragStarted(object sender, DragStartedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
        }

        private void slider3_DragStarted(object sender, DragStartedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
        }

        private void UpdateHistogramAndColor(ImageReviewViewModel vm)
        {
            histogram0.Data = vm.HistogramData0;
            if (vm.AutoManualTog1Checked == true) AutoEnhance1();
            histogram1.Data = vm.HistogramData1;
            if (vm.AutoManualTog2Checked == true) AutoEnhance2();
            histogram2.Data = vm.HistogramData2;
            if (vm.AutoManualTog3Checked == true) AutoEnhance3();
            histogram3.Data = vm.HistogramData3;
            if (vm.AutoManualTog4Checked == true) AutoEnhance4();

            for (int i = 0; i < vm.ImageReview.MaxChannels; i++)
            {
                SetHistogramColor(i, vm.GetColorAssignment(i));
            }
        }

        private void UpdateHistogramData()
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            //Update the scrollbar height when the image data changes
            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;

            //update the data when the capture is active and the control is visible
            if (histogramEnable.IsChecked == true)
            {
                UpdateHistogramAndColor(vm);

                for (int i = 0; i < vm.ImageReview.MaxChannels; i++)
                {
                    switch (i)
                    {
                        case 0: panel0.Visibility = vm.ChannelEnableA ? Visibility.Visible : Visibility.Collapsed; break;
                        case 1: panel1.Visibility = vm.ChannelEnableB ? Visibility.Visible : Visibility.Collapsed; break;
                        case 2: panel2.Visibility = vm.ChannelEnableC ? Visibility.Visible : Visibility.Collapsed; break;
                        case 3: panel3.Visibility = vm.ChannelEnableD ? Visibility.Visible : Visibility.Collapsed; break;
                    }
                }
            }
            UpdateIntensityData();
        }

        private void UpdateIntensityData()
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            if (vm.ImageColorChannels == 1)
            {
                labAVal.Content = "Val";
                panelAVal.Visibility = Visibility.Visible;
                panelBVal.Visibility = Visibility.Collapsed;
                panelCVal.Visibility = Visibility.Collapsed;
                panelDVal.Visibility = Visibility.Collapsed;
            }
            else
            {
                labAVal.Content = "A Val";

                for (int i = 0; i < vm.MaxChannels; i++)
                {
                    switch (i)
                    {
                        case 0: panelAVal.Visibility = (vm.LSMChannel & 1) >= 1 ? Visibility.Visible : Visibility.Collapsed; break;
                        case 1: panelBVal.Visibility = (vm.LSMChannel & 2) >= 1 ? Visibility.Visible : Visibility.Collapsed; break;
                        case 2: panelCVal.Visibility = (vm.LSMChannel & 4) >= 1 ? Visibility.Visible : Visibility.Collapsed; break;
                        case 3: panelDVal.Visibility = (vm.LSMChannel & 8) >= 1 ? Visibility.Visible : Visibility.Collapsed; break;
                    }
                }
            }
        }

        private void UpdateOrthogonalView(Point point)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null || image1.ImageSource == null)
            {
                return;
            }

            UpdateOrthogonalViewLines(xzImageCanvas, xzImageCanvas, new Point(point.X, vm.ZValue), _orthogonalXZViewLine);
            UpdateOrthogonalViewLines(yzImageCanvas, yzImageCanvas, new Point(vm.ZValue, point.Y), _orthogonalYZViewLine);
            UpdateOrthogonalViewLines(imageCanvas, xyImageCanvas, point, _orthogonalViewLine);

            _orthogonalViewMatrix = imageCanvas.RenderTransform.Value;
            _orthogonalViewMatrix.OffsetY += _currentScale * image1.ImageSource.Height + 10;
            xzImageCanvas.RenderTransform = new MatrixTransform(_orthogonalViewMatrix);
            _orthogonalViewMatrix = imageCanvas.RenderTransform.Value;
            _orthogonalViewMatrix.OffsetX += _currentScale * image1.ImageSource.Width + 10;
            yzImageCanvas.RenderTransform = new MatrixTransform(_orthogonalViewMatrix);
        }

        private void UpdateOrthogonalViewLines(Canvas srcCanvas, Canvas dstCanvas, Point point, Line[] line)
        {
            for (int i = 0; i < 2; i++)
            {
                if (line[i] == null)
                {
                    line[i] = new Line();
                    line[i].Stroke = _brushColors[0];
                    line[i].StrokeDashArray = new DoubleCollection() { 2 };
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
                        line[i].Stroke = _brushColors[_colorType];
                        line[i].StrokeThickness = Math.Max(1, Math.Min(1 / _currentScale, 10));
                        if (_lineType == 0)
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

            ImageBrush brush = (ImageBrush)srcCanvas.Background;
            line[0].X1 = 0;
            line[0].X2 = brush.ImageSource.Width;
            line[0].Y2 = point.Y;
            line[0].Y1 = point.Y;
            line[1].X1 = point.X;
            line[1].X2 = point.X;
            line[1].Y2 = 0;
            line[1].Y1 = brush.ImageSource.Height;
            dstCanvas.Children.Add(line[0]);
            dstCanvas.Children.Add(line[1]);
        }

        private void virtualStack_Click(object sender, RoutedEventArgs e)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }
            if (vm.OrthogonalViewStat != ImageReviewViewModel.OrthogonalViewStatus.INACTIVE)
            {
                vm_CloseOrthogonalView();

                OrthogonalViewEnable.IsChecked = true;

                vm.InitOrthogonalView();

                if (this.virtualStack.IsChecked)
                {
                    this.lockPosition.Visibility = Visibility.Visible; // contextMenu lock position is visible
                    this.lockPosition.IsChecked = false;
                }
            }
        }

        void vm_AnalysisLoaded()
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            bool reticleActive = false;
            bool scaleActive = false;
            OverlayManagerClass.Instance.LoadROIs(vm.ROIsDirectory + "\\ROIs.xaml", ref overlayCanvas, ref reticleActive, ref scaleActive);
            ReticleOnOff.IsChecked = reticleActive;
            ScaleOnOff.IsChecked = scaleActive;
        }

        /// <summary>
        /// Closes the orthogonal view.
        /// </summary>
        void vm_CloseOrthogonalView()
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }
            OrthogonalViewEnable.IsChecked = false;

            vm.OrthogonalViewStat = ImageReviewViewModel.OrthogonalViewStatus.INACTIVE;

            xyImageCanvas.Children.Clear();
            xzImageCanvas.Children.Clear();
            yzImageCanvas.Children.Clear();

            this.lockPosition.IsChecked = false;
            this.lockPosition.Visibility = Visibility.Collapsed;  //hide the lock position option
            this.xzImageCanvas.Visibility = Visibility.Collapsed; //hide the xzImage and yzImage
            this.yzImageCanvas.Visibility = Visibility.Collapsed;
        }

        void vm_ExperimentPathChanged()
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            OverlayManagerClass.Instance.UpdateParams(ExperimentData.ImageInfo.pixelX, ExperimentData.ImageInfo.pixelY * ExperimentData.NumberOfPlanes, ExperimentData.LSMUMPerPixel);
            vm_CloseOrthogonalView();

            bool reticleActive = false;
            bool scaleActive = false;
            OverlayManagerClass.Instance.LoadROIs(vm.ExperimentFolderPath + "\\ROIs.xaml", ref overlayCanvas, ref reticleActive, ref scaleActive);
            ReticleOnOff.IsChecked = reticleActive;
            ScaleOnOff.IsChecked = scaleActive;
            histogram0.BlackPoint = (int)vm.BlackPoint0;
            histogram1.BlackPoint = (int)vm.BlackPoint1;
            histogram2.BlackPoint = (int)vm.BlackPoint2;
            histogram3.BlackPoint = (int)vm.BlackPoint3;
            histogram0.WhitePoint = (int)vm.WhitePoint0;
            histogram1.WhitePoint = (int)vm.WhitePoint1;
            histogram2.WhitePoint = (int)vm.WhitePoint2;
            histogram3.WhitePoint = (int)vm.WhitePoint3;
        }

        void VM_ImageDataChanged(bool obj)
        {
            UpdateHistogramData();
        }

        void vm_OrthogonalViewImagesLoaded(bool cancelFlag)
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }
            if (!cancelFlag)
            {
                if (vm.OrthogonalViewStat == ImageReviewViewModel.OrthogonalViewStatus.INACTIVE)
                {
                    vm.OrthogonalViewStat = ImageReviewViewModel.OrthogonalViewStatus.ACTIVE;//set viewmodel orthogonal view flag to true
                }

                UpdateOrthogonalView(vm.BitmapPoint);

                this.xzImageCanvas.Visibility = Visibility.Visible;
                this.yzImageCanvas.Visibility = Visibility.Visible;
            }
            else
            {
                if (this.xzImageCanvas.Visibility == Visibility.Collapsed)
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

        private void VM_UpdatingObject(bool obj)
        {
            _updatingOverlayObject = obj;
        }

        void vm_ZChanged()
        {
            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }
            //throw new NotImplementedException();
            if (vm.OrthogonalViewStat != ImageReviewViewModel.OrthogonalViewStatus.INACTIVE)
            {
                if (this.lockPosition.IsChecked == true)
                {
                    UpdateOrthogonalViewLines(xzImageCanvas, xzImageCanvas, new Point(this.lockPoint.X, vm.ZValue), _orthogonalXZViewLine);
                    UpdateOrthogonalViewLines(yzImageCanvas, yzImageCanvas, new Point(vm.ZValue, this.lockPoint.Y), _orthogonalYZViewLine);
                }
                else
                {
                    if (xyImageCanvas.Children.Count > 0)
                    {
                        UpdateOrthogonalViewLines(xzImageCanvas, xzImageCanvas, new Point(_orthogonalViewLine[1].X1, vm.ZValue), _orthogonalXZViewLine);
                        UpdateOrthogonalViewLines(yzImageCanvas, yzImageCanvas, new Point(vm.ZValue, _orthogonalViewLine[0].Y1), _orthogonalYZViewLine);
                    }
                }
            }
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
            xyImageCanvas.RenderTransform = _transformGroup;

            ImageReviewViewModel vm = (ImageReviewViewModel)this.DataContext;
            if (null == vm)
            {
                return;
            }
            if (vm.OrthogonalViewStat != ImageReviewViewModel.OrthogonalViewStatus.INACTIVE)
            {
                UpdateOrthogonalView(vm.BitmapPoint);
            }

            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        #endregion Methods
    }
}