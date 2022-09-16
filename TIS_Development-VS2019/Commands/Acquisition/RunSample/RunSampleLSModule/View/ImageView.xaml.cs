namespace RunSampleLSDll.View
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
    using System.Xml;

    using Microsoft.Win32;

    using OverlayManager;

    using RunSampleLSDll.ViewModel;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for MasterView.xaml
    /// </summary>
    public partial class ImageView : UserControl
    {
        #region Fields

        private const int LUT_MAX = 256;
        private const int LUT_MIN = 0;

        private Point _currentOffset;
        private double _currentScale;
        Matrix _m = new Matrix();
        Point _newOrigin;
        Point _newStart;
        bool _roiToolMouseDoubleClickFlag = false;
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

            _currentOffset.X = 0;
            _currentOffset.Y = 0;

            zoomSlider.Value = _currentScale;

            _translateTransform = new TranslateTransform();
            _scaleTransform = new ScaleTransform();
            _transformGroup = new TransformGroup();

            _transformGroup.Children.Add(_translateTransform);
            _transformGroup.Children.Add(_scaleTransform);

            imageCanvas.RenderTransform = _transformGroup;
            overlayCanvas.RenderTransform = _transformGroup;

            _currentScale = 1.0;

            _updatingOverlayObject = false;

            _shiftDown = false;

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");

            this.Loaded += new RoutedEventHandler(ImageView_Loaded);
            this.Unloaded += new RoutedEventHandler(ImageView_Unloaded);
            histogram0.BwTextBoxFocued += new EventHandler(H1_BWTextBoxFocused);
            histogram1.BwTextBoxFocued += new EventHandler(H2_BWTextBoxFocused);
            histogram2.BwTextBoxFocued += new EventHandler(H3_BWTextBoxFocused);
            histogram3.BwTextBoxFocued += new EventHandler(H4_BWTextBoxFocused);
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
                RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
                    if (null != vm.ROIStatsChart)
                    {
                        vm.ROIStatsChart.Close();
                    }
                }
            }
            else if (e.Key == Key.A && (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
            {
                RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
                RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            ROIDrawingTools.SelectedIndex = 0;
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
        }

        protected override void OnMouseDoubleClick(MouseButtonEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
                        MouseEvent me = new MouseEvent();
                        me = MouseEvent.LEFTHOLDING;
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
                try
                {
                    RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
                    if (vm == null)
                    {
                        return;
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
                    }

                    if (vm.Bitmap == null)
                    {
                        ReleaseMouseCapture();
                        this.imageCanvas.ContextMenu.Visibility = Visibility.Collapsed;
                        return;
                    }
                    else
                    {
                        if (this.imageCanvas.ContextMenu != null)
                        {
                            this.imageCanvas.ContextMenu.Visibility = Visibility.Visible;
                        }
                    }

                    Cursor = Cursors.Arrow;
                }
                catch (Exception ex)
                {
                    ex.ToString();
                }
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

            _m = imageCanvas.RenderTransform.Value;

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
            _m.ScaleAtPrepend(scaleFactor, scaleFactor, p.X, p.Y);
            _currentScale = nextScale;

            imageCanvas.RenderTransform = new MatrixTransform(_m);
            overlayCanvas.RenderTransform = new MatrixTransform(_m);
            zoomSlider.Value = _m.M22;

            base.OnMouseWheel(e);

            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        private void AutoEnhance1()
        {
            if (true == histogramEnable.IsChecked)
            {
                histogram0.MaxBinValue = _whitePointMaxVal;
                histogram0.BlackPoint = histogram0.MinValue;
                histogram0.WhitePoint = Math.Max(1, histogram0.MaxValue); //The minimum value a white point can have is 1
                sliderBlackPoint0.Value = histogram0.MinValue;
                sliderWhitePoint0.Value = Math.Max(1, histogram0.MaxValue);
            }
        }

        private void AutoEnhance2()
        {
            if (true == histogramEnable.IsChecked)
            {
                histogram1.MaxBinValue = _whitePointMaxVal;
                histogram1.BlackPoint = histogram1.MinValue;
                histogram1.WhitePoint = Math.Max(1, histogram1.MaxValue); //The minimum value a white point can have is 1
                sliderBlackPoint1.Value = histogram1.MinValue;
                sliderWhitePoint1.Value = Math.Max(1, histogram1.MaxValue);
            }
        }

        private void AutoEnhance3()
        {
            if (true == histogramEnable.IsChecked)
            {
                histogram2.MaxBinValue = _whitePointMaxVal;
                histogram2.BlackPoint = histogram2.MinValue;
                histogram2.WhitePoint = Math.Max(1, histogram2.MaxValue); //The minimum value a white point can have is 1
                sliderBlackPoint2.Value = histogram2.MinValue;
                sliderWhitePoint2.Value = Math.Max(1, histogram2.MaxValue);
            }
        }

        private void AutoEnhance4()
        {
            if (true == histogramEnable.IsChecked)
            {
                histogram3.MaxBinValue = _whitePointMaxVal;
                histogram3.BlackPoint = histogram3.MinValue;
                histogram3.WhitePoint = Math.Max(1, histogram3.MaxValue); //The minimum value a white point can have is 1
                sliderBlackPoint3.Value = histogram3.MinValue;
                sliderWhitePoint3.Value = Math.Max(1, histogram3.MaxValue);
            }
        }

        private void btnBPDown0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram0.BlackPoint -= 1;
        }

        private void btnBPDown1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram1.BlackPoint -= 1;
        }

        private void btnBPDown2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram2.BlackPoint -= 1;
        }

        private void btnBPDown3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram3.BlackPoint -= 1;
        }

        private void btnBPUp0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram0.BlackPoint += 1;
        }

        private void btnBPUp1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram1.BlackPoint += 1;
        }

        private void btnBPUp2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram2.BlackPoint += 1;
        }

        private void btnBPUp3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram3.BlackPoint += 1;
        }

        private void btnWPDown0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram0.WhitePoint -= 1;
        }

        private void btnWPDown1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram1.WhitePoint -= 1;
        }

        private void btnWPDown2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram2.WhitePoint -= 1;
        }

        private void btnWPDown3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram3.WhitePoint -= 1;
        }

        private void btnWPUp0_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;
            histogram0.WhitePoint += 1;
        }

        private void btnWPUp1_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;
            histogram1.WhitePoint += 1;
        }

        private void btnWPUp2_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;
            histogram2.WhitePoint += 1;
        }

        private void btnWPUp3_Click(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;
            histogram3.WhitePoint += 1;
        }

        private void ButtonReset_Click0(object sender, RoutedEventArgs e)
        {
            AutoManualTog1.IsChecked = false;

            histogram0.BlackPoint = LUT_MIN;
            histogram0.WhitePoint = LUT_MAX;
            sliderBlackPoint0.Value = LUT_MIN;
            sliderWhitePoint0.Value = LUT_MAX;
        }

        private void ButtonReset_Click1(object sender, RoutedEventArgs e)
        {
            AutoManualTog2.IsChecked = false;

            histogram1.BlackPoint = LUT_MIN;
            histogram1.WhitePoint = LUT_MAX;
            sliderBlackPoint1.Value = LUT_MIN;
            sliderWhitePoint1.Value = LUT_MAX;
        }

        private void ButtonReset_Click2(object sender, RoutedEventArgs e)
        {
            AutoManualTog3.IsChecked = false;

            histogram2.BlackPoint = LUT_MIN;
            histogram2.WhitePoint = LUT_MAX;
            sliderBlackPoint2.Value = LUT_MIN;
            sliderWhitePoint2.Value = LUT_MAX;
        }

        private void ButtonReset_Click3(object sender, RoutedEventArgs e)
        {
            AutoManualTog4.IsChecked = false;

            histogram3.BlackPoint = LUT_MIN;
            histogram3.WhitePoint = LUT_MAX;
            sliderBlackPoint3.Value = LUT_MIN;
            sliderWhitePoint3.Value = LUT_MAX;
        }

        private void ClearAllROIs_Click(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
                if (null != vm.ROIStatsChart)
                {
                    vm.ROIStatsChart.Close();
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
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            OverlayManagerClass.Instance.InitROICrosshair(ref overlayCanvas);
        }

        private void createEllipseROI_Click(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            OverlayManagerClass.Instance.InitROIEllipse(ref overlayCanvas);
        }

        private void createLineROI_Click(object sender, RoutedEventArgs e)
        {
            if (true == _roiToolMouseDoubleClickFlag)
            {
                _roiToolMouseDoubleClickFlag = false;
                return;
            }
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            OverlayManagerClass.Instance.InitROILine(ref overlayCanvas);
        }

        private void createLineROI_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            OverlayManagerClass.Instance.InitROIPolyline(ref overlayCanvas);
        }

        private void createPolylineROI_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            _roiToolMouseDoubleClickFlag = true;
            OverlayManagerClass.Instance.InitROIPolylineWithOptions(ref overlayCanvas);
        }

        private void createPolyROI_Click(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            OverlayManagerClass.Instance.InitROIPoly(ref overlayCanvas);
        }

        private void createRectROI_Click(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            OverlayManagerClass.Instance.InitROIRect(ref overlayCanvas);
        }

        private void DeleteSelectedROIs(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
                if (null != vm.ROIStatsChart)
                {
                    vm.ROIStatsChart.Close();
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
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }
            UpdateHistogramData();
        }

        /// <summary>
        /// Timer for screen update
        /// </summary>
        /// <param name="e"></param>
        private void ImageAndHistogramTimer_Tick(object sender, EventArgs e)
        {
            try
            {
                RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;

                //verify the data context
                if (vm == null)
                {
                    return;
                }
                UpdateHistogramData();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "ImageAndHistogramTimer_Tick exception " + ex.Message);
            }
        }

        void ImageView_Loaded(object sender, RoutedEventArgs e)
        {
            if (System.Environment.OSVersion.Version.Major <= 5)
            {
                //force the control into software rendering mode
                //there is a memory leak in the .net 3.51 version
                //*TODO* remove when new frame is used and memory leak is resolved

                HwndSource hwndSource = PresentationSource.FromVisual(this) as HwndSource;

                HwndTarget hwndTarget = hwndSource.CompositionTarget;

                // this is the new WPF API to force render mode.

                hwndTarget.RenderMode = RenderMode.SoftwareOnly;
            }

            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }

            vm.ImageDataChanged += new Action<bool>(VM_ImageDataChanged);
            vm.PaletteChanged += new Action<bool>(VM_PaletteChanged);
            vm.HistogramPanelUpdate += new Action<bool>(VM_HistogramPanelUpdate);
            this.SizeChanged += new SizeChangedEventHandler(ImageView_SizeChanged);

            OverlayManagerClass.Instance.BinX = vm.BinX;
            OverlayManagerClass.Instance.BinY = vm.BinY;
            OverlayManagerClass.Instance.UpdatingObjectEvent += new Action<bool>(VM_UpdatingObject);
            OverlayManagerClass.Instance.UpdateParams(vm.SettingsImageWidth, vm.SettingsImageHeight, vm.PixelSizeUM);
            bool reticleActive = false;
            bool scaleActive = false;
            OverlayManagerClass.Instance.PersistLoadROIs(ref overlayCanvas, ref reticleActive, ref scaleActive);
            ReticleOnOff.IsChecked = reticleActive;
            ScaleOnOff.IsChecked = scaleActive;
            vm.PropertyChanged += vm_PropertyChanged;
            ROIDrawingTools.SelectedIndex = 0;
            UpdatePalette();
            UpdateHistogramData();

            string str = string.Empty;

            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/General/ChannelTileViewSettings");
            if (null != node)
            {
                vm.TileDisplay = XmlManager.GetAttribute(node, appSettings, "TilingEnableOption", ref str) && (str == "1" || str == Boolean.TrueString);
                vm.VerticalTileDisplay = XmlManager.GetAttribute(node, appSettings, "VerticalTiling", ref str) && (str == "1" || str == Boolean.TrueString);
            }

            node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/General/HistogramSettings");
            if (null != node)
            {
                if (XmlManager.GetAttribute(node, appSettings, "ReducedBinValue", ref str))
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

        void ImageView_Unloaded(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }
            vm.ImageDataChanged -= new Action<bool>(VM_ImageDataChanged);
            vm.PaletteChanged -= new Action<bool>(VM_PaletteChanged);
            this.SizeChanged -= new SizeChangedEventHandler(ImageView_SizeChanged);

            OverlayManagerClass.Instance.UpdatingObjectEvent -= new Action<bool>(VM_UpdatingObject);
            vm.PropertyChanged -= vm_PropertyChanged;

            if (true == vm.NewExperiment)
            {
                vm.NewExperiment = false;
                OverlayManagerClass.Instance.SaveROIs(vm.ExperimentFolderPath + "ROIs.xaml");
                OverlayManagerClass.Instance.SaveMaskToPath(vm.ExperimentFolderPath + "ROIMask.raw");
            }

            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/General/ChannelTileViewSettings");

            if (null != node)
            {
                XmlManager.SetAttribute(node, appSettings, "TilingEnableOption", vm.TileDisplay.ToString());
                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }
        }

        private void LoadROIs_Click(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;

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

            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        /// <summary>
        /// Handles changes in the view model with additional behaviors outside of the xaml bindings. Must be run in the dispatch thread
        /// or it will throw an exception.
        /// </summary>
        /// <param name="sender"> </param>
        /// <param name="e"> </param>
        private void ReactToPropertyChangeEvents(object sender, PropertyChangedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;

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

            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        private void ReticleOnOff_Click(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
                    if (null != vm.ROIStatsChart)
                    {
                        vm.ROIStatsChart.Close();
                    }
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

        private void SaveROIs_Click(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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
                    if (null != vm.ROIStatsChart)
                    {
                        vm.ROIStatsChart.Close();
                    }
                }
            }
        }

        private void SelectROI_Selected(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            OverlayManagerClass.Instance.InitSelectROI(ref overlayCanvas);
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

        private void TileDisplay_Click(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            vm.TileDisplay = (bool)TileEnable.IsChecked;
        }

        private void UpdateHistogramData()
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;

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

                for (int i = 0; i < vm.RunSampleLS.GetMaxChannels(); i++)
                {

                    int val = vm.RunSampleLS.ChannelSelection;

                    if (((val >> i) & 0x1) > 0)
                    {

                        switch (i)
                        {
                            case 0:
                                histogram0.Data = vm.HistogramData0;
                                if (vm.AutoManualTog1Checked == true)
                                {
                                    histogram0.WhitePoint = histogram0.MaxValue;
                                    histogram0.BlackPoint = histogram0.MinValue;
                                }
                                else
                                {
                                    histogram0.WhitePoint = (int)vm.WhitePoint0;
                                    histogram0.BlackPoint = (int)vm.BlackPoint0;
                                }
                                break;
                            case 1:
                                histogram1.Data = vm.HistogramData1;
                                if (vm.AutoManualTog2Checked == true)
                                {
                                    histogram1.WhitePoint = histogram1.MaxValue;
                                    histogram1.BlackPoint = histogram1.MinValue;
                                }
                                else
                                {
                                    histogram1.WhitePoint = (int)vm.WhitePoint1;
                                    histogram1.BlackPoint = (int)vm.BlackPoint1;
                                }
                                break;
                            case 2:
                                histogram2.Data = vm.HistogramData2;
                                if (vm.AutoManualTog3Checked == true)
                                {
                                    histogram2.WhitePoint = histogram2.MaxValue;
                                    histogram2.BlackPoint = histogram2.MinValue;
                                }
                                else
                                {
                                    histogram2.WhitePoint = (int)vm.WhitePoint2;
                                    histogram2.BlackPoint = (int)vm.BlackPoint2;
                                }
                                break;
                            case 3:
                                histogram3.Data = vm.HistogramData3;
                                if (vm.AutoManualTog4Checked == true)
                                {
                                    histogram3.WhitePoint = histogram3.MaxValue;
                                    histogram3.BlackPoint = histogram3.MinValue;
                                }
                                else
                                {
                                    histogram3.WhitePoint = (int)vm.WhitePoint3;
                                    histogram3.BlackPoint = (int)vm.BlackPoint3;
                                }
                                break;
                        }
                    }
                }
            }

            UpdateIntensityData();
        }

        private void UpdateIntensityData()
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }
        }

        private void UpdatePalette()
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            //verify the data context
            if (vm == null)
            {
                return;
            }

            int index = 0;

            for (int i = 0; i < vm.RunSampleLS.GetMaxChannels(); i++)
            {
                SetHistogramColor(i, vm.GetColorAssignment(i));

                Visibility vis;

                int val = vm.ChannelSelection;

                if (((val >> i) & 0x1) > 0)
                {
                    vis = Visibility.Visible;
                    index = i;
                }
                else
                {
                    vis = Visibility.Collapsed;
                }

                switch (i)
                {
                    case 0: panel0.Visibility = vis; break;
                    case 1: panel1.Visibility = vis; break;
                    case 2: panel2.Visibility = vis; break;
                    case 3: panel3.Visibility = vis; break;
                }
            }

            if (1 == vm.ImageColorChannels)
            {
                SetHistogramColor(index, Colors.Black);
            }
        }

        private void UpdateRollOverPixelData(Point imagePixelLocation)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
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

        void VM_HistogramPanelUpdate(bool obj)
        {
            if (AutoManualTog1.IsChecked == true) AutoEnhance1();
            if (AutoManualTog2.IsChecked == true) AutoEnhance2();
            if (AutoManualTog3.IsChecked == true) AutoEnhance3();
            if (AutoManualTog4.IsChecked == true) AutoEnhance4();
        }

        void VM_ImageDataChanged(bool obj)
        {
            UpdateHistogramData();
        }

        void VM_PaletteChanged(bool obj)
        {
            UpdatePalette();
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

            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.IVScrollBarHeight = (null != this.image1.ImageSource && null != _translateTransform) ? ((this.image1.ImageSource.Height + 0.001) * _currentScale + _translateTransform.Y) : vm.IVScrollBarHeight;
        }

        #endregion Methods
    }
}