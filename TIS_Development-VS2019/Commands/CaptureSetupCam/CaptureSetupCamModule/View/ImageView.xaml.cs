namespace CaptureSetupDll.View
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

    using CaptureSetupDll.Model;
    using CaptureSetupDll.ViewModel;

    using Microsoft.Win32;

    using ThorLogging;

    /// <summary>
    /// Interaction logic for ImageView.xaml
    /// </summary>
    public partial class ImageView : UserControl
    {
        #region Fields

        private const int MAX_HISTOGRAMS = 4;

        private static LiveImageViewModel viewModelForROIStats;
        private static bool _addAnOverlay = false;
        private static double _imageHeight;
        private static double _imageWidht;
        private static bool _isLineRoi = false;
        private static bool _isRectRoi = false;
        private static OverlayManager.OverlayType _overlayType;

        private double deltaValue;
        Matrix m = new Matrix();
        Point newOrigin;
        Point newStart;
        Point p;
        Point pp = new Point();
        Point _currentImagePixelLocation;
        private double _currentScale;
        private DispatcherTimer _histogramUpdateTimer;
        private bool _imageDataChanged;
        private int _saveDlgFilterIndex = 0;
        private ScaleTransform _scaleTransform;
        private Point _scrollStartPoint;
        private TransformGroup _transformGroup;
        private TranslateTransform _translateTransform;
        //private RotateTransform _rotateTransform;
        //private TranslateTransform _preRotateTranslate, _postRotateTranslate;

        #endregion Fields

        #region Constructors

        public ImageView()
        {
            InitializeComponent();

            zoomSlider.Value = _currentScale;

            _translateTransform = new TranslateTransform();
            _scaleTransform = new ScaleTransform();
            _transformGroup = new TransformGroup();
            ////Kirk : Trying to make image rotate and still scale/translate correctly
            //_rotateTransform = new RotateTransform();
            //_preRotateTranslate = new TranslateTransform();
            //_postRotateTranslate = new TranslateTransform();

            ////Kirk : Need to remove hardcoded values, but need to figure out where image loaded event is
            //_preRotateTranslate.X = -1 * 1392 / 2;
            //_preRotateTranslate.Y = -1 * 1040 / 2;
            //_postRotateTranslate.X = 1040 / 2;
            //_postRotateTranslate.Y = 1392 / 2;
            //_rotateTransform.Angle = 90;

            _transformGroup.Children.Add(_scaleTransform);
            _transformGroup.Children.Add(_translateTransform);

            ////Kirk : Trying to make image rotate and still scale/translate correctly
            //_transformGroup.Children.Add(_preRotateTranslate);
            //_transformGroup.Children.Add(_rotateTransform);
            //_transformGroup.Children.Add(_postRotateTranslate);

            imageCanvas.RenderTransform = _transformGroup;
            overlayCanvas.RenderTransform = _transformGroup;

            _currentScale = 1.0;
            zoomText.Text = ConvertScaleToPercent(_currentScale);
            zoomSlider.Value = 1.0;

            histogram1.DataBrushColor = Colors.Black;
            histogram2.DataBrushColor = Colors.Red;
            histogram3.DataBrushColor = Colors.Green;
            histogram4.DataBrushColor = Colors.Blue;
            _imageDataChanged = false;
            _histogramUpdateTimer = new DispatcherTimer();
            _histogramUpdateTimer.Interval = TimeSpan.FromSeconds(1);

            this.Loaded += new RoutedEventHandler(ImageView_Loaded);
            this.Unloaded += new RoutedEventHandler(ImageView_Unloaded);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Properties

        public static bool AddAnOverlayProperty
        {
            get { return _addAnOverlay; }
            set
            {
                _addAnOverlay = value;
            }
        }

        public static double ImageHeight
        {
            get { return _imageHeight; }
            set { }
        }

        public static double ImageWidth
        {
            get { return _imageWidht; }
            set { }
        }

        public static bool IsLineROIProperty
        {
            set
            {
                _isLineRoi = value;
            }
            get
            {
                return _isLineRoi;
            }
        }

        public static bool IsRectROIProperty
        {
            set
            {
                _isRectRoi = value;
            }
            get
            {
                return _isRectRoi;
            }
        }

        public static OverlayManager.OverlayType OverlayTypeProperty
        {
            get
            {
                return _overlayType;
            }
            set
            {
                _overlayType = value;
            }
        }

        #endregion Properties

        #region Methods

        public static LiveImageViewModel GetLiveImageViewModelObject()
        {
            try
            {
                if (viewModelForROIStats != null)
                {
                    return viewModelForROIStats;
                }
                else
                {
                    return null;
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ImageView " + "GetLiveImageViewModelObject exception " + ex.Message);
                return null;
            }
        }

        public static ROIStats GetROIStatsWindow()
        {
            try
            {
                if (viewModelForROIStats._overlayManager.ROIStatsWindow != null)
                {
                    return viewModelForROIStats._overlayManager.ROIStatsWindow;
                }
                return null;

            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ImageView " + "getROIStatsWindow exception " + ex.Message);
                return null;
            }
        }

        public static void updateStatsAfterResizingOrPanning()
        {
            try
            {
               viewModelForROIStats._overlayManager.UpdateStats((int)viewModelForROIStats.Bitmap.Width, (int)viewModelForROIStats.Bitmap.Height, LiveImage.GetColorChannels());
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ImageView " + "updateStatsAfterResizingOrPanning exception " + ex.Message);
            }
        }

        /// <summary>
        /// Get position and CaptureMouse
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            viewModelForROIStats = (LiveImageViewModel)this.DataContext;
            if (image1.ImageSource != null)
            {
                _imageWidht = image1.ImageSource.Width;
                _imageHeight = image1.ImageSource.Height;

                ////Kirk
                //_preRotateTranslate.X = -1 * image1.ImageSource.Width / 2;
                //_preRotateTranslate.Y = -1 * image1.ImageSource.Height / 2;

                //_postRotateTranslate.X = 1 * image1.ImageSource.Width / 2;
                //_postRotateTranslate.Y = 1 * image1.ImageSource.Height / 2;
            }

            if (vm == null)
            {
                return;
            }
            if (IsMouseOver)
            {
                if (AddAnOverlayProperty)
                {
                    _scrollStartPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);

                    _scrollStartPoint.X = Math.Max(0, Math.Min(_scrollStartPoint.X, image1.ImageSource.Width));
                    _scrollStartPoint.Y = Math.Max(0, Math.Min(_scrollStartPoint.Y, image1.ImageSource.Height));
                    //remove an existing roi

                    overlayCanvas.Children.Clear();
                    vm._overlayManager.CreateObject(OverlayTypeProperty, ref overlayCanvas, new Point(_scrollStartPoint.X, _scrollStartPoint.Y));

                    CaptureMouse();
                }
                else
                {

                    if (OverlayManager._adornerProvided)
                    {
                        if (IsRectROIProperty)
                            vm._overlayManager.RemoveAdorners("rectangle");
                        if (IsLineROIProperty)
                            vm._overlayManager.RemoveAdorners("line");
                    }

                }

                newStart = e.GetPosition(this);
                newOrigin.X = imageCanvas.RenderTransform.Value.OffsetX;
                newOrigin.Y = imageCanvas.RenderTransform.Value.OffsetY;
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
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if (IsMouseCaptured)
            {
                if (AddAnOverlayProperty)
                {
                    Point currentPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);

                    currentPoint.X = Math.Max(0, Math.Min(currentPoint.X, image1.ImageSource.Width));
                    currentPoint.Y = Math.Max(0, Math.Min(currentPoint.Y, image1.ImageSource.Height));
                    vm._overlayManager.ObjectResize(new Point(currentPoint.X, currentPoint.Y), ref overlayCanvas);
                }
                else
                {

                    pp = e.MouseDevice.GetPosition(this);

                    Matrix m = imageCanvas.RenderTransform.Value;
                    m.OffsetX = newOrigin.X + (pp.X - newStart.X);
                    m.OffsetY = newOrigin.Y + (pp.Y - newStart.Y);

                    imageCanvas.RenderTransform = new MatrixTransform(m);
                    overlayCanvas.RenderTransform = new MatrixTransform(m);
                }
            }

            Point imagePixelLocation = this.TranslatePoint(e.GetPosition(this), imageCanvas);

            UpdateRollOverPixelData(imagePixelLocation);

            _currentImagePixelLocation = imagePixelLocation;

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
                if (AddAnOverlayProperty)
                {
                    LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
                    if (vm == null)
                    {
                        return;
                    }

                    vm._overlayManager.UpdateStats((int)vm.Bitmap.Width, (int)vm.Bitmap.Height, LiveImage.GetColorChannels());

                    AddAnOverlayProperty = false;
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

            p = e.MouseDevice.GetPosition(imageCanvas);

            m = imageCanvas.RenderTransform.Value;

            deltaValue = e.Delta;
            double compare = Math.Round(_currentScale, 2);
            if (e.Delta > 0)
            {
                if (compare >= zoomSlider.Maximum) return;
                _currentScale += .1;
                _currentScale = Math.Min(_currentScale, 10);
                m.ScaleAtPrepend(1.1, 1.1, p.X, p.Y);

            }
            else
            {
                if (compare <= zoomSlider.Minimum) return;
                _currentScale -= .1;
                _currentScale = Math.Max(_currentScale, .1);
                m.ScaleAtPrepend(1 / 1.1, 1 / 1.1, p.X, p.Y);

            }

            imageCanvas.RenderTransform = new MatrixTransform(m);
            overlayCanvas.RenderTransform = new MatrixTransform(m);
            zoomSlider.Value = m.M22;

            base.OnMouseWheel(e);
        }

        private void ButtonAutoEnahnce_Click1(object sender, RoutedEventArgs e)
        {
            histogram1.BlackPoint = histogram1.MinValue;
            histogram1.WhitePoint = histogram1.MaxValue;
            sliderBP0.Value = histogram1.MinValue;
            sliderWP0.Value = histogram1.MaxValue;
        }

        private void ButtonAutoEnahnce_Click2(object sender, RoutedEventArgs e)
        {
            histogram2.BlackPoint = histogram2.MinValue;
            histogram2.WhitePoint = histogram2.MaxValue;
            sliderBP1.Value = histogram2.MinValue;
            sliderWP1.Value = histogram2.MaxValue;
        }

        private void ButtonAutoEnahnce_Click3(object sender, RoutedEventArgs e)
        {
            histogram3.BlackPoint = histogram3.MinValue;
            histogram3.WhitePoint = histogram3.MaxValue;
            sliderBP2.Value = histogram3.MinValue;
            sliderWP2.Value = histogram3.MaxValue;
        }

        private void ButtonAutoEnahnce_Click4(object sender, RoutedEventArgs e)
        {
            histogram4.BlackPoint = histogram4.MinValue;
            histogram4.WhitePoint = histogram4.MaxValue;
            sliderBP3.Value = histogram4.MinValue;
            sliderWP3.Value = histogram4.MaxValue;
        }

        string ConvertScaleToPercent(double scale)
        {
            Decimal dec = new Decimal(scale * 100);
            string str = Decimal.Round(dec, 0).ToString() + "%";

            return str;
        }

        private void createLineROI_Click(object sender, RoutedEventArgs e)
        {
            AddAnOverlayProperty = true;
            OverlayTypeProperty = OverlayManager.OverlayType.LINE;

            IsLineROIProperty = true;
            IsRectROIProperty = false;

            if (OverlayManager._adornerProvided)
            {
                viewModelForROIStats._overlayManager.RemoveAdorners("line");
            }
        }

        private void createRectROI_Click(object sender, RoutedEventArgs e)
        {
            AddAnOverlayProperty = true;

            OverlayTypeProperty = OverlayManager.OverlayType.RECTANGLE;

            IsLineROIProperty = false;
            IsRectROIProperty = true;

            if (OverlayManager._adornerProvided)
            {
                viewModelForROIStats._overlayManager.RemoveAdorners("rectangle");
            }
        }

        /// <summary>
        /// blackpoint value is changing
        /// </summary>
        /// <param name="e"></param>
        void Histogram_BlackPoint_Changed1(int obj)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            vm.BlackPoint0 = obj;
        }

        void Histogram_BlackPoint_Changed2(int obj)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            vm.BlackPoint1 = obj;
        }

        void Histogram_BlackPoint_Changed3(int obj)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            vm.BlackPoint2 = obj;
        }

        void Histogram_BlackPoint_Changed4(int obj)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            vm.BlackPoint3 = obj;
        }

        /// <summary>
        /// visibility of the histogram control changed
        /// </summary>
        /// <param name="e"></param>
        void Histogram_IsVisibleChanged1(object sender, DependencyPropertyChangedEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

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

        /// <summary>
        /// whitepoint value is changing
        /// </summary>
        /// <param name="e"></param>
        void Histogram_WhitePoint_Changed1(int obj)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            vm.WhitePoint0 = obj;
        }

        void Histogram_WhitePoint_Changed2(int obj)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            vm.WhitePoint1 = obj;
        }

        void Histogram_WhitePoint_Changed3(int obj)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            vm.WhitePoint2 = obj;
        }

        void Histogram_WhitePoint_Changed4(int obj)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            vm.WhitePoint3 = obj;
        }

        void ImageView_Loaded(object sender, RoutedEventArgs e)
        {
            zoomSlider.ValueChanged += new RoutedPropertyChangedEventHandler<double>(ZoomSlider_ValueChanged);

            histogram1.BlackPoint_Changed += new Action<int>(Histogram_BlackPoint_Changed1);
            histogram1.WhitePoint_Changed += new Action<int>(Histogram_WhitePoint_Changed1);
            histogram2.BlackPoint_Changed += new Action<int>(Histogram_BlackPoint_Changed2);
            histogram2.WhitePoint_Changed += new Action<int>(Histogram_WhitePoint_Changed2);
            histogram3.BlackPoint_Changed += new Action<int>(Histogram_BlackPoint_Changed3);
            histogram3.WhitePoint_Changed += new Action<int>(Histogram_WhitePoint_Changed3);
            histogram4.BlackPoint_Changed += new Action<int>(Histogram_BlackPoint_Changed4);
            histogram4.WhitePoint_Changed += new Action<int>(Histogram_WhitePoint_Changed4);

            histogram1.IsVisibleChanged += new DependencyPropertyChangedEventHandler(Histogram_IsVisibleChanged1);

            SetDisplayOptions();

            _histogramUpdateTimer.Tick += new EventHandler(_histogramUpdateTimer_Tick);
            _histogramUpdateTimer.Start();
        }

        void ImageView_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            mainGrid.Height = e.NewSize.Height;
            mainGrid.Width = e.NewSize.Width;

            toolbarGrid.Width = e.NewSize.Width;
        }

        void ImageView_Unloaded(object sender, RoutedEventArgs e)
        {
            zoomSlider.ValueChanged -= new RoutedPropertyChangedEventHandler<double>(ZoomSlider_ValueChanged);

            _histogramUpdateTimer.Stop();
            _histogramUpdateTimer.Tick -= new EventHandler(_histogramUpdateTimer_Tick);

            histogram1.BlackPoint_Changed -= new Action<int>(Histogram_BlackPoint_Changed1);
            histogram1.WhitePoint_Changed -= new Action<int>(Histogram_WhitePoint_Changed1);
            histogram2.BlackPoint_Changed -= new Action<int>(Histogram_BlackPoint_Changed2);
            histogram2.WhitePoint_Changed -= new Action<int>(Histogram_WhitePoint_Changed2);
            histogram3.BlackPoint_Changed -= new Action<int>(Histogram_BlackPoint_Changed3);
            histogram3.WhitePoint_Changed -= new Action<int>(Histogram_WhitePoint_Changed3);
            histogram4.BlackPoint_Changed -= new Action<int>(Histogram_BlackPoint_Changed4);
            histogram4.WhitePoint_Changed -= new Action<int>(Histogram_WhitePoint_Changed4);

            histogram1.IsVisibleChanged -= new DependencyPropertyChangedEventHandler(Histogram_IsVisibleChanged1);

            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            vm._overlayManager.CloseDialog();
        }

        private void originalButton_Click(object sender, RoutedEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

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

            _translateTransform.X -= translateX;
            _translateTransform.Y -= translateY;
        }

        /// <summary>
        /// return to 100% scale
        /// </summary>
        /// <param name="e"></param>
        private void ResetClick(object sender, RoutedEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

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
            zoomText.Text = ConvertScaleToPercent(_currentScale);

            //Move to the top left
            //Point topLeft = new Point(0, 0);
            //Point imagePixelLocation = this.TranslatePoint(topLeft, imageCanvas);

            double translateX = _translateTransform.X;
            double translateY = _translateTransform.Y;

            //_translateTransform.X = 0;
            //_translateTransform.Y = 0;

            _translateTransform.X -=  translateX;
            _translateTransform.Y -=  translateY;
        }

        private void rollOverButton_Click(object sender, RoutedEventArgs e)
        {
            UpdateIntensityData();
        }

        private void saveAs_Click(object sender, RoutedEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            // Configure save file dialog box
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Title = "Save an Image File";
            dlg.FileName = string.Format("Image_{0:yyyy-MM-dd_hh-mm-ss}",DateTime.Now);
            ;
            switch (LiveImage.GetColorChannels())
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

        private void SetDisplayOptions()
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            //configure the visibility of the XY control panel
            angleMagButton.Visibility = Visibility.Collapsed;

            XmlNodeList ndList = vm.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");
            if (ndList.Count > 0)
            {
                if (ndList[0].Attributes["Visibility"].Value.Equals("Visible") && vm.IsXYStagePresent)
                {
                    angleMagButton.Visibility = Visibility.Visible;
                }
            }
        }

        private void UpdateHistogramData()
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

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
                    for(int i = 0; i < MAX_HISTOGRAMS; i++)
                    {
                        Visibility vis;
                        if (vm.LSMChannelEnable[i])
                        {
                            vis = Visibility.Visible;

                            Color color = Colors.Gray;

                            switch(vm.GetColorAssignment(i))
                            {
                                case 0:color = Colors.Red;break;
                                case 1:color = Colors.Green;break;
                                case 2:color = Colors.Blue;break;
                                case 3:color = Colors.Cyan;break;
                                case 4:color = Colors.Magenta;break;
                                case 5:color = Colors.Yellow;break;
                                case 6:color = Colors.Gray;break;
                            }

                            switch (i)
                            {
                                case 0: histogram1.Data = vm.HistogramData0; histogram1.DataBrushColor = color; break;
                                case 1: histogram2.Data = vm.HistogramData1; histogram2.DataBrushColor = color; break;
                                case 2: histogram3.Data = vm.HistogramData2; histogram3.DataBrushColor = color; break;
                                case 3: histogram4.Data = vm.HistogramData3; histogram4.DataBrushColor = color; break;
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
                    panel1.Visibility = Visibility.Visible;
                    panel2.Visibility = Visibility.Collapsed;
                    panel3.Visibility = Visibility.Collapsed;
                    panel4.Visibility = Visibility.Collapsed;

                    histogram1.Data = vm.HistogramData0;
                    histogram1.DataBrushColor = Colors.Black;
                }

            }

            UpdateIntensityData();
        }

        private void UpdateIntensityData()
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

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
                    if (vm.LSMChannelEnable[i])
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

        private void UpdateRollOverPixelData(Point imagePixelLocation)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.RollOverPointX = (int)imagePixelLocation.X;
            vm.RollOverPointY = (int)imagePixelLocation.Y;

            rollOverTextX.Text = Convert.ToInt32(imagePixelLocation.X).ToString();
            rollOverTextY.Text = Convert.ToInt32(imagePixelLocation.Y).ToString();
            rollOverTextInt0.Text = vm.RollOverPointIntensity0.ToString();
            rollOverTextInt1.Text = vm.RollOverPointIntensity1.ToString();
            rollOverTextInt2.Text = vm.RollOverPointIntensity2.ToString();
            rollOverTextInt3.Text = vm.RollOverPointIntensity3.ToString();
        }

        private void UserControl_Loaded(object sender, RoutedEventArgs e)
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

            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            vm.ImageDataChanged += new Action<bool>(VM_ImageDataChanged);

            this.SizeChanged += new SizeChangedEventHandler(ImageView_SizeChanged);

            vm._overlayManager.ClosingROIStats += new Action(VM_ClosingROIStats);
            vm._overlayManager.ClosingLineProfile += new Action(VM_ClosingLineProfile);
            vm._overlayManager.LineWidthChange += new Action(VM_LineWidthChange);
        }

        private void UserControl_Unloaded(object sender, RoutedEventArgs e)
        {
            this.SizeChanged -= new SizeChangedEventHandler(ImageView_SizeChanged);

            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            vm.StopCommand.Execute(null);
        }

        void VM_ClosingLineProfile()
        {
            overlayCanvas.Children.Clear();
            IsLineROIProperty = false;
            IsRectROIProperty = false;
        }

        void VM_ClosingROIStats()
        {
            overlayCanvas.Children.Clear();
            IsLineROIProperty = false;
            IsRectROIProperty = false;
        }

        unsafe void VM_ImageDataChanged(bool obj)
        {
            _imageDataChanged = true;
            UpdateRollOverPixelData(_currentImagePixelLocation);

            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
            vm.GridAlignImage = (bool)angleMagButton.IsChecked;

            if (angleMagButton.IsChecked == true && cbCalcMag.IsChecked == true)
            {
                short[] data = vm.GetData_Int16();
                fixed (short* ptr = data)
                {
                    double gridSize = double.Parse(tbGridSize.Text);

                    //Temporarily commented until build is fixed 9/23/2013 Ike
                    //double mag = 1.0;
                    double mag = CPP_Library.CImg_Wrapper.MagCalculation(ptr, vm.Width, vm.Height, 6.45, gridSize);

                    tbMagnification.Text = mag.ToString("F3");
                }
            }
        }

        void VM_LineWidthChange()
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            vm._overlayManager.UpdateStats((int)vm.Bitmap.Width, (int)vm.Bitmap.Height, LiveImage.GetColorChannels());
        }

        /// <summary>
        /// Zooming via slider
        /// </summary>
        /// <param name="e"></param>
        void ZoomSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            _currentScale = e.NewValue;
            Matrix mm = new Matrix();
            mm = imageCanvas.RenderTransform.Value;

            _scaleTransform.ScaleX = _currentScale;
            _scaleTransform.ScaleY = _currentScale;

            _translateTransform.X = mm.OffsetX;
            _translateTransform.Y = mm.OffsetY;

            imageCanvas.RenderTransform = _transformGroup;
            overlayCanvas.RenderTransform = _transformGroup;
            zoomText.Text = ConvertScaleToPercent(_currentScale);
        }

        private void zoomText_KeyUp(object sender, KeyEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            //verify the data context
            if (vm == null)
            {
                return;
            }

            ZoomSettings dlg = new ZoomSettings();
            if (((TextBox)sender).Text != "" || ((TextBox)sender).Text != String.Empty)
            {
                dlg.ZoomLevel = Convert.ToInt32(((TextBox)sender).Text.ToString().Replace('%', ' '));

                if (true == dlg.ShowDialog())
                {
                    _currentScale = dlg.ZoomLevel / 100.0;

                    _scaleTransform.ScaleX = _currentScale;
                    _scaleTransform.ScaleY = _currentScale;
                    imageCanvas.RenderTransform = _transformGroup;
                    overlayCanvas.RenderTransform = _transformGroup;
                    zoomText.Text = ConvertScaleToPercent(_currentScale);
                    zoomSlider.Value = _currentScale;
                }
            }
        }

        void _histogramUpdateTimer_Tick(object sender, EventArgs e)
        {
            if (_imageDataChanged)
            {
                LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
                if (vm == null)
                {
                    return;
                }

                UpdateHistogramData();
                _imageDataChanged = false;
            }
        }

        #endregion Methods
    }
}
