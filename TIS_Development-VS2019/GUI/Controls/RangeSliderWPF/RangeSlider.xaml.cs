using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Controls.Primitives;
using ThorLogging;

namespace RangeSliderWPF
{
    /// <summary>
    /// Interaction logic for RangeSlider.xaml
    /// </summary>
    public partial class RangeSlider : UserControl
    {
        #region Fields
        //Default values in sync with the values set in XAML        
        double _sliderWidth = 10;        
        double _sliderMin = 0;
        double _sliderMax = .1;
        double _rangeMin = 0;
        double _rangeMax = .1;        
        Brush _rangeHighlightingColor;
        bool _showMarker;
        Point _ptOriginalPosition;
        string _caption = "Range Slider";
        public enum SliderOrientation {Horizontal, Vertical};

        #endregion

        #region Dependency Properties    
     
        #region Orientation

        public DependencyProperty OrientationProperty = DependencyProperty.Register(
                                                                 "Orientation",
                                                                 typeof(SliderOrientation),
                                                                 typeof(RangeSlider),
                                                                 new PropertyMetadata(SliderOrientation.Vertical, new PropertyChangedCallback(OrientationChangedCallback)));

        public SliderOrientation Orientation
        {
            get { return (SliderOrientation)GetValue(OrientationProperty); }
            set
            {
                SetValue(OrientationProperty, value);
            }
        }

        private static void OrientationChangedCallback(DependencyObject source, DependencyPropertyChangedEventArgs e)
        {
            (source as RangeSlider).UpdateOrientationValue(e.NewValue);
        }

        private void UpdateOrientationValue(object newValue)
        {
            SliderOrientation orientation = (SliderOrientation)newValue;
            if (orientation == SliderOrientation.Horizontal)
            {
                canvasH .Visibility = Visibility.Visible;
                canvasV .Visibility = Visibility.Hidden;
            }
            else
            {
                canvasV .Visibility = Visibility.Visible;
                canvasH .Visibility = Visibility.Hidden;
            }
        }

        #endregion

        #region StepSize

        public DependencyProperty StepSizeProperty = DependencyProperty.Register(
                                                                 "StepSize",
                                                                 typeof(double),
                                                                 typeof(RangeSlider),
                                                                 new PropertyMetadata(1.0, new PropertyChangedCallback(StepSizeChangedCallback)));

        public double StepSize
        {
            get { return (double)GetValue(StepSizeProperty); }
            set
            {
                SetValue(StepSizeProperty, value);
            }
        }

        private static void StepSizeChangedCallback(DependencyObject source, DependencyPropertyChangedEventArgs e)
        {
            (source as RangeSlider).UpdateStepSizeValue(e.NewValue);
        }

        private void UpdateStepSizeValue(object newValue)
        {
            try
            {
                if ((double)newValue < 0) newValue = 0.0;

                sliderH.LargeChange = (double)newValue;
                sliderH.TickFrequency = (double)newValue;
                if (sliderH.LargeChange == 0)
                    sliderH.TickPlacement = TickPlacement.None;
                else
                    sliderH.TickPlacement = TickPlacement.TopLeft;

                sliderV.LargeChange = (double)newValue;
                sliderV.TickFrequency = (double)newValue;
                if (sliderV.LargeChange == 0)
                    sliderV.TickPlacement = TickPlacement.None;
                else
                    sliderV.TickPlacement = TickPlacement.TopLeft;

                StepSize = (double)newValue;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }

        #endregion

        #region IsMoveToPointEnabled

        public DependencyProperty IsMoveToPointEnabledProperty = DependencyProperty.Register(
                                                                 "IsMoveToPointEnabled", 
                                                                 typeof(Boolean), 
                                                                 typeof(RangeSlider),
                                                                 new PropertyMetadata(new PropertyChangedCallback(IsMoveToPointEnabledChangedCallback)));                                                              

        public bool IsMoveToPointEnabled
        {
            get { return (bool)GetValue(IsMoveToPointEnabledProperty); }
            set
            {                
                SetValue(IsMoveToPointEnabledProperty, value);
            }
        }

        private static void IsMoveToPointEnabledChangedCallback(DependencyObject source, DependencyPropertyChangedEventArgs e)
        {
            (source as RangeSlider).UpdateIsMoveToPointEnabledValue(e.NewValue); 
        }

        private void UpdateIsMoveToPointEnabledValue(object newValue)
        {
            sliderH.IsMoveToPointEnabled = (bool)newValue;
            sliderV.IsMoveToPointEnabled = (bool)newValue;
        }

        #endregion

        #region SliderWidth

        public DependencyProperty SliderWidthProperty = DependencyProperty.Register(
                                                                 "SliderWidth",
                                                                 typeof(double),
                                                                 typeof(RangeSlider),
                                                                 new PropertyMetadata(10.0, new PropertyChangedCallback(SliderWidthChangedCallback)));

        public double SliderWidth
        {
            get { return (double)GetValue(SliderWidthProperty); }
            set
            {
                SetValue(SliderWidthProperty, value);
            }
        }

        private static void SliderWidthChangedCallback(DependencyObject source, DependencyPropertyChangedEventArgs e)
        {
            (source as RangeSlider).UpdateSliderWidthValue(e.NewValue);
        }

        private void UpdateSliderWidthValue(object newValue)
        {
            try
            {
                double val = Convert.ToDouble(newValue);
                if (val <= 0) val = 1;
                
                _sliderWidth = val;
                SliderWidth = val;

                sliderH.Width = _sliderWidth + 10;
                canvasH.Width = _sliderWidth + 10;

                sliderV.Height = _sliderWidth + 10;
                canvasV.Height = _sliderWidth + 10;

                UpdateRange();
                
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }
        
        #endregion

        #region MarkerValue

        public DependencyProperty MarkerValueProperty = DependencyProperty.Register(
                                                                 "MarkerValue",
                                                                 typeof(double),
                                                                 typeof(RangeSlider),
                                                                 new PropertyMetadata(new PropertyChangedCallback(MarkerValueChangedCallback)));

        public double MarkerValue
        {
            get { return (double)GetValue(MarkerValueProperty); }
            set
            {
                SetValue(MarkerValueProperty, value);
            }
        }

        private static void MarkerValueChangedCallback(DependencyObject source, DependencyPropertyChangedEventArgs e)
        {
            (source as RangeSlider).UpdateMarkerValueValue(e.NewValue);
        }

        private void UpdateMarkerValueValue(object newValue)
        {
            try
            {
                double val = Convert.ToDouble(newValue);
                if (val > SliderMax)
                {
                    sliderH.Value = SliderMax;
                    sliderV.Value = SliderMax;

                    MarkerValue = SliderMax;
                }
                else if (val < _sliderMin)
                {
                    sliderH.Value = SliderMin;
                    sliderV.Value = SliderMin;

                    MarkerValue = SliderMin;
                }
                else
                {
                    sliderH.Value = val;
                    sliderV.Value = val;

                    MarkerValue = val;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }      

        #endregion

        #region SliderMin

        public DependencyProperty SliderMinProperty = DependencyProperty.Register(
                                                                 "SliderMin",
                                                                 typeof(double),
                                                                 typeof(RangeSlider),
                                                                 new PropertyMetadata(new PropertyChangedCallback(SliderMinChangedCallback)));

        public double SliderMin
        {
            get { return (double)GetValue(SliderMinProperty); }
            set
            {
                SetValue(SliderMinProperty, value);
            }
        }

        private static void SliderMinChangedCallback(DependencyObject source, DependencyPropertyChangedEventArgs e)
        {
            (source as RangeSlider).UpdateSliderMinValue(e.NewValue);
        }

        private void UpdateSliderMinValue(object newValue)
        {
            try
            {
                double val = Convert.ToDouble(newValue);
                if (val > SliderMax)
                {
                    _sliderMin = SliderMax;
                }
                else if (val > RangeMin)
                {
                    _sliderMin = RangeMin;
                }
                else
                {
                    _sliderMin = val;
                }
                
                SetSliderMin();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }       

        #endregion

        #region SliderMax

        public DependencyProperty SliderMaxProperty = DependencyProperty.Register(
                                                                 "SliderMax",
                                                                 typeof(double),
                                                                 typeof(RangeSlider),
                                                                 new PropertyMetadata(.1, new PropertyChangedCallback(SliderMaxChangedCallback)));

        public double SliderMax
        {
            get { return (double)GetValue(SliderMaxProperty); }
            set
            {
                SetValue(SliderMaxProperty, value);
            }
        }

        private static void SliderMaxChangedCallback(DependencyObject source, DependencyPropertyChangedEventArgs e)
        {
            (source as RangeSlider).UpdateSliderMaxValue(e.NewValue);
        }

        private void UpdateSliderMaxValue(object newValue)
        {
            try
            {
                double val = Convert.ToDouble(newValue);
                if (val < SliderMin)
                {
                    _sliderMax = SliderMin;
                }
                else if (val < RangeMax)
                {
                    _sliderMax = RangeMax;
                }
                else
                {
                    _sliderMax = val;
                }

                SetSliderMax();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }        

        #endregion

        #region RangeMin

        public DependencyProperty RangeMinProperty = DependencyProperty.Register(
                                                                 "RangeMin",
                                                                 typeof(double),
                                                                 typeof(RangeSlider),
                                                                 new PropertyMetadata(new PropertyChangedCallback(RangeMinChangedCallback)));

        public double RangeMin
        {
            get { return (double)GetValue(RangeMinProperty); }
            set
            {
                SetValue(RangeMinProperty, value);
            }
        }

        private static void RangeMinChangedCallback(DependencyObject source, DependencyPropertyChangedEventArgs e)
        {
            (source as RangeSlider).UpdateRangeMinValue(e.NewValue);
        }

        private void UpdateRangeMinValue(object newValue)
        {
            try
            {
                double val = Convert.ToDouble(newValue);
                if (val > RangeMax)
                {
                    _rangeMin = RangeMax;
                }
                else if (val < SliderMin)
                {
                    _rangeMin = SliderMin;
                }
                else
                {
                    _rangeMin = val;
                }

                SetRangeMin();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Information, 1, ex.Message);// MessageBox.Show(ex.Message, _caption);
            }
        }

        #endregion

        #region RangeMax

        public DependencyProperty RangeMaxProperty = DependencyProperty.Register(
                                                                 "RangeMax",
                                                                 typeof(double),
                                                                 typeof(RangeSlider),
                                                                 new PropertyMetadata(.1, new PropertyChangedCallback(RangeMaxChangedCallback)));

        public double RangeMax
        {
            get { return (double)GetValue(RangeMaxProperty); }
            set
            {
                SetValue(RangeMaxProperty, value);
            }
        }

        private static void RangeMaxChangedCallback(DependencyObject source, DependencyPropertyChangedEventArgs e)
        {
            (source as RangeSlider).UpdateRangeMaxValue(e.NewValue);
        }

        private void UpdateRangeMaxValue(object newValue)
        {
            try
            {
                double val = Convert.ToDouble(newValue);
                if (val < RangeMin)
                {
                    _rangeMax = RangeMin;
                }
                else if (val > SliderMax)
                {
                    _rangeMax = SliderMax;
                }
                else
                {
                    _rangeMax = val;
                }

                SetRangeMax();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Information, 1, ex.Message);//MessageBox.Show(ex.Message, _caption);
            }
        }
        
        #endregion

        #region RangeHighlightingColor

        public DependencyProperty RangeHighlightingColorProperty = DependencyProperty.Register(
                                                                 "RangeHighlightingColor",
                                                                 typeof(Brush),
                                                                 typeof(RangeSlider),
                                                                 new PropertyMetadata(Brushes.LightBlue, new PropertyChangedCallback(RangeHighlightingColorChangedCallback)));

        public Brush RangeHighlightingColor
        {
            get { return (Brush)GetValue(RangeHighlightingColorProperty); }
            set
            {
                SetValue(RangeHighlightingColorProperty, value);
            }
        }

        private static void RangeHighlightingColorChangedCallback(DependencyObject source, DependencyPropertyChangedEventArgs e)
        {
            (source as RangeSlider).UpdateRangeHighlightingColorValue(e.NewValue);
        }

        private void UpdateRangeHighlightingColorValue(object newValue)
        {
            try
            {
                _rangeHighlightingColor = (Brush)newValue;
                SetRangeHighlightingColor();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }

        #endregion

        #region ShowMarker

        public DependencyProperty ShowMarkerProperty = DependencyProperty.Register(
                                                                 "ShowMarker",
                                                                 typeof(Boolean),
                                                                 typeof(RangeSlider),
                                                                 new PropertyMetadata(true, new PropertyChangedCallback(ShowMarkerChangedCallback)));

        public bool ShowMarker
        {
            get { return (bool)GetValue(ShowMarkerProperty); }
            set
            {
                SetValue(ShowMarkerProperty, value);
            }
        }

        private static void ShowMarkerChangedCallback(DependencyObject source, DependencyPropertyChangedEventArgs e)
        {
            (source as RangeSlider).UpdateShowMarkerValue(e.NewValue);
        }

        private void UpdateShowMarkerValue(object newValue)
        {
            try
            {
                _showMarker = (bool)newValue;
                SetMarkerVisibility();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }

        #endregion

        #endregion

        #region Constructor
        public RangeSlider()
        {
            InitializeComponent();
        }
        #endregion

        #region Private Methods

        private void SetMarkerVisibility()
        {
            if (sliderH.Template.FindName("PART_Track", sliderH) != null)
            {
                Thumb sliderThumb = (sliderH.Template.FindName("PART_Track", sliderH) as Track).Thumb;
                if (_showMarker)
                    sliderThumb.Visibility = Visibility.Visible;
                else
                    sliderThumb.Visibility = Visibility.Hidden;
            }

            if (sliderV.Template.FindName("PART_Track", sliderV) != null)
            {
                Thumb sliderThumb = (sliderV.Template.FindName("PART_Track", sliderV) as Track).Thumb;
                if (_showMarker)
                    sliderThumb.Visibility = Visibility.Visible;
                else
                    sliderThumb.Visibility = Visibility.Hidden;
            }
        }

        private void SetRangeHighlightingColor()
        {
            rectRangeH.Fill = _rangeHighlightingColor;
            rectRangeV.Fill = _rangeHighlightingColor;
        }

        private void SetSliderMin()
        {
            sliderH.Minimum = _sliderMin;
            sliderV.Minimum = _sliderMin;
            SliderMin = _sliderMin;              
            UpdateRange();            
        }

        private void SetSliderMax()
        {
            sliderH.Maximum = _sliderMax;
            sliderV.Maximum = _sliderMax;            
            SliderMax = _sliderMax;
            UpdateRange();
        }

        private void UpdateRange()
        {
            double selectionScale, widthScale, selectionWidth;
            double selectionStartX, selectionEndX, selectionStartY, selectionEndY;
                      
            selectionScale = (_sliderMax - _sliderMin) / _sliderWidth;
            widthScale = (_rangeMax - _rangeMin) / (_sliderMax - _sliderMin);
            selectionWidth = _sliderWidth * widthScale;

            //Horizontal slider
            selectionStartX = (sliderH.SelectionStart - _sliderMin) / selectionScale;  
            selectionEndX = selectionStartX + thumbLeft.Width + selectionWidth;     
            
            Canvas.SetLeft(thumbLeft, selectionStartX);
            Canvas.SetLeft(rectRangeH, selectionStartX + thumbLeft.Width);
            rectRangeH.Width = selectionWidth;            
            Canvas.SetLeft(thumbRight, selectionEndX);

            //Vertical slider
            selectionStartY = (sliderV.SelectionStart - _sliderMin) / selectionScale;
            selectionEndY = selectionStartY + thumbTop.Height + selectionWidth;

            Canvas.SetTop(thumbTop, canvasV.Height - selectionEndY);
            Canvas.SetTop(thumbBottom, canvasV.Height - selectionStartY);
            rectRangeV.Height = selectionWidth;
            Canvas.SetTop(rectRangeV, canvasV.Height - selectionEndY + thumbTop.Height);

            //SelectionStart and SelectionEnd values are same for both the sliders
            _rangeMin = sliderH.SelectionStart;
            _rangeMax = sliderH.SelectionEnd;

            RangeMin = sliderH.SelectionStart;
            RangeMax = sliderH.SelectionEnd;            
        }

        private void SetRangeMin()
        {
            double scale = _sliderWidth / (_sliderMax - _sliderMin);

            //Horizontal slider
            sliderH.SelectionStart = _rangeMin;            
            double physicalPlacement = (sliderH.SelectionStart - _sliderMin) * scale;

            Canvas.SetLeft(thumbLeft, physicalPlacement);
            Canvas.SetLeft(rectRangeH, physicalPlacement + thumbLeft.Width);
            rectRangeH.Width = Canvas.GetLeft(thumbRight) - thumbLeft.Width - physicalPlacement;
            
            //Vertical slider
            sliderV.SelectionStart = _rangeMin;            
            physicalPlacement = (sliderV.SelectionStart - _sliderMin) * scale;

            Canvas.SetTop(thumbBottom, canvasV.Height - physicalPlacement);            
            rectRangeV.Height = Canvas.GetTop(thumbBottom) - Canvas.GetTop(thumbTop) - thumbBottom.Height;

            RangeMin = _rangeMin;
        }

        private void SetRangeMax()
        {
            double scale = _sliderWidth / (_sliderMax - _sliderMin);

            //Horizontal slider
            sliderH.SelectionEnd = _rangeMax;            
            double physicalPlacement = (sliderH.SelectionEnd - _sliderMin) * scale;

            rectRangeH.Width = physicalPlacement - Canvas.GetLeft(thumbLeft);
            Canvas.SetLeft(thumbRight, physicalPlacement + thumbRight.Width);        
 
            //Vertical slider
            sliderV.SelectionEnd = _rangeMax;
            physicalPlacement = (_sliderMax - sliderV.SelectionEnd) * scale;
            
            Canvas.SetTop(thumbTop, physicalPlacement);
            Canvas.SetTop(rectRangeV, Canvas.GetTop(thumbTop) + thumbTop.Height);
            rectRangeV.Height = Canvas.GetTop(thumbBottom) - Canvas.GetTop(thumbTop) - thumbBottom.Height;

            RangeMax = _rangeMax;
        }

        #endregion

        #region Event Handlers

        #region Horizontal slider

        private void thumbLeft_DragDelta(object sender, System.Windows.Controls.Primitives.DragDeltaEventArgs e)
        {
            try
            {
                double horizontalChange = e.HorizontalChange;
                double leftOfThmbLeft = Canvas.GetLeft(thumbLeft);
                double leftOfThumbRight = Canvas.GetLeft(thumbRight);

                if (leftOfThmbLeft + horizontalChange < 0) horizontalChange = 0 - leftOfThmbLeft;

                if (leftOfThmbLeft + horizontalChange <= leftOfThumbRight &&
                    leftOfThmbLeft + horizontalChange >= 0 &&
                    rectRangeH.Width - horizontalChange >= 0)
                {
                    double logicalMovement = horizontalChange * (_sliderMax - _sliderMin) / _sliderWidth;
                    Canvas.SetLeft(thumbLeft, leftOfThmbLeft + horizontalChange);
                    sliderH.SelectionStart += logicalMovement;  
                    Canvas.SetLeft(rectRangeH, leftOfThmbLeft + thumbLeft.Width);
                    rectRangeH.Width -= horizontalChange;

                    _rangeMin = sliderH.SelectionStart;
                    RangeMin = sliderH.SelectionStart;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }

        private void thumbRight_DragDelta(object sender, System.Windows.Controls.Primitives.DragDeltaEventArgs e)
        {
            try
            {
                double horizontalChange = e.HorizontalChange;
                double leftOfThmbLeft = Canvas.GetLeft(thumbLeft);
                double leftOfThumbRight = Canvas.GetLeft(thumbRight);

                if (leftOfThumbRight + horizontalChange > sliderH.Width) horizontalChange = sliderH.Width - leftOfThumbRight;

                if (leftOfThumbRight + horizontalChange >= leftOfThmbLeft && 
                    leftOfThumbRight + horizontalChange <= sliderH.Width && 
                    rectRangeH.Width + horizontalChange >= 0)
                {
                    double logicalMovement = horizontalChange * (_sliderMax - _sliderMin) / _sliderWidth;
                    Canvas.SetLeft(thumbRight, leftOfThumbRight + horizontalChange);
                    sliderH.SelectionEnd += logicalMovement;
                    rectRangeH.Width += horizontalChange;

                    _rangeMax = sliderH.SelectionEnd;
                    RangeMax = sliderH.SelectionEnd;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }         

        private void sliderH_MouseMove(object sender, MouseEventArgs e)
        {
            try
            {
                if (!sliderH.IsMouseCaptured) return;

                Point ptNewPosition = e.GetPosition(canvasH);
                Vector vect = ptNewPosition - _ptOriginalPosition;                
                _ptOriginalPosition = ptNewPosition;

                double leftOfThmbLeft = Canvas.GetLeft(thumbLeft);
                double leftOfRectRange = Canvas.GetLeft(rectRangeH);
                double leftOfThumbRight = Canvas.GetLeft(thumbRight);
                double leftOfSliderH = Canvas.GetLeft(sliderH);

                if (vect.X <= 0 && leftOfRectRange - (thumbLeft.Width / 2) + vect.X < leftOfSliderH)
                    vect.X = leftOfSliderH - (leftOfRectRange - (thumbLeft.Width / 2));

                if (vect.X >= 0 && leftOfRectRange + rectRangeH.Width + (thumbRight.Width / 2) + vect.X > leftOfSliderH + sliderH.Width)
                    vect.X = (leftOfSliderH + sliderH.Width) - (leftOfRectRange + rectRangeH.Width + (thumbRight.Width / 2));

                if ((vect.X < 0 && leftOfRectRange - (thumbLeft.Width / 2) + vect.X >= leftOfSliderH) ||
                    (vect.X > 0 && leftOfRectRange + rectRangeH.Width + (thumbRight.Width / 2) + vect.X <= leftOfSliderH + sliderH.Width))
                {
                    Canvas.SetLeft(thumbLeft, leftOfThmbLeft + vect.X);
                    Canvas.SetLeft(rectRangeH, leftOfRectRange + vect.X);
                    Canvas.SetLeft(thumbRight, leftOfThumbRight + vect.X);

                    double logicalMovement = vect.X * (_sliderMax - _sliderMin) / _sliderWidth;                    
                    sliderH.SelectionStart += logicalMovement;
                    sliderH.SelectionEnd += logicalMovement;

                    _rangeMin = sliderH.SelectionStart;
                    _rangeMax = sliderH.SelectionEnd;

                    RangeMin = sliderH.SelectionStart;
                    RangeMax = sliderH.SelectionEnd;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }

        private void sliderH_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            try
            {
                sliderH.ReleaseMouseCapture();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }

        private void sliderH_GotMouseCapture(object sender, MouseEventArgs e)
        {
            try
            {
                _ptOriginalPosition = e.GetPosition(canvasH);

                double leftOfThmbLeft = Canvas.GetLeft(thumbLeft);
                double leftOfThumbRight = Canvas.GetLeft(thumbRight);

                if (_ptOriginalPosition.X > leftOfThmbLeft && _ptOriginalPosition.X < leftOfThumbRight)
                {
                    if (!e.OriginalSource.GetType().Equals(typeof(Thumb)))
                    {
                        sliderH.CaptureMouse();
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }        

        private void sliderH_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            MarkerValue = sliderH.Value;
        }

        #endregion

        #region Vertical slider

        private void thumbTop_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                double verticalChange = e.VerticalChange;
                double topOfThumbTop = Canvas.GetTop(thumbTop);
                double topOfThumbBottom = Canvas.GetTop(thumbBottom);

                if (topOfThumbTop + verticalChange < 0) verticalChange = 0 - topOfThumbTop;
                
                if (topOfThumbTop + verticalChange <= topOfThumbBottom &&
                        topOfThumbTop + verticalChange >= 0 &&
                        rectRangeV.Height - verticalChange >= 0)
                {
                    double logicalMovement = verticalChange * (_sliderMax - _sliderMin) / _sliderWidth;
                    Canvas.SetTop(thumbTop, topOfThumbTop + verticalChange);
                    sliderV.SelectionEnd -= logicalMovement;
                    Canvas.SetTop(rectRangeV, topOfThumbTop + thumbTop.Height);
                    rectRangeV.Height -= verticalChange;

                    _rangeMax = sliderV.SelectionEnd;
                    RangeMax = sliderV.SelectionEnd;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }

        private void thumbBottom_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                double verticalChange = e.VerticalChange;
                double topOfThumbTop = Canvas.GetTop(thumbTop);
                double topOfThumbBottom = Canvas.GetTop(thumbBottom);

                if (topOfThumbBottom + verticalChange > sliderV.Height) verticalChange = sliderV.Height - topOfThumbBottom;

                if (topOfThumbBottom + verticalChange >= topOfThumbTop &&
                    topOfThumbBottom + verticalChange <= sliderV.Height &&
                    rectRangeV.Height + verticalChange >= 0)
                {
                    double logicalMovement = verticalChange * (_sliderMax - _sliderMin) / _sliderWidth;
                    Canvas.SetTop(thumbBottom, topOfThumbBottom + verticalChange);
                    sliderV.SelectionStart -= logicalMovement;
                    rectRangeV.Height += verticalChange;

                    _rangeMin = sliderV.SelectionStart;
                    RangeMin = sliderV.SelectionStart;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }

        private void sliderV_MouseMove(object sender, MouseEventArgs e)
        {
            try
            {
                if (!sliderV.IsMouseCaptured) return;

                Point ptNewPosition = e.GetPosition(canvasV);
                Vector vect = ptNewPosition - _ptOriginalPosition;
                _ptOriginalPosition = ptNewPosition;

                double topOfThumbTop = Canvas.GetTop(thumbTop);
                double topOfRectRange = Canvas.GetTop(rectRangeV);
                double topOfThumbBottom = Canvas.GetTop(thumbBottom);
                double topOfSliderV = Canvas.GetTop(sliderV);

                if (vect.Y <= 0 && topOfRectRange - (thumbTop.Height / 2) + vect.Y < topOfSliderV)
                    vect.Y = topOfSliderV - (topOfRectRange - (thumbTop.Height / 2));

                if (vect.Y >= 0 && topOfRectRange + rectRangeV.Height + (thumbBottom.Height / 2) + vect.Y > topOfSliderV + sliderV.Height)
                    vect.Y = (topOfSliderV + sliderV.Height) - (topOfRectRange + rectRangeV.Height + (thumbBottom.Height / 2));

                if ((vect.Y < 0 && topOfRectRange - (thumbTop.Height / 2) + vect.Y >= topOfSliderV) ||
                    (vect.Y > 0 && topOfRectRange + rectRangeV.Height + (thumbBottom.Height / 2) + vect.Y <= topOfSliderV + sliderV.Height))
                {
                    Canvas.SetTop(thumbTop, topOfThumbTop + vect.Y);
                    Canvas.SetTop(rectRangeV, topOfRectRange + vect.Y);
                    Canvas.SetTop(thumbBottom, topOfThumbBottom + vect.Y);

                    double logicalMovement = vect.Y * (_sliderMax - _sliderMin) / _sliderWidth;
                    sliderV.SelectionStart -= logicalMovement;
                    sliderV.SelectionEnd -= logicalMovement;

                    _rangeMin = sliderV.SelectionStart;
                    _rangeMax = sliderV.SelectionEnd;

                    RangeMin = sliderV.SelectionStart;
                    RangeMax = sliderV.SelectionEnd;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }     

        private void sliderV_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            try
            {
                sliderV.ReleaseMouseCapture();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }

        private void sliderV_GotMouseCapture(object sender, MouseEventArgs e)
        {
            try
            {
                _ptOriginalPosition = e.GetPosition(canvasV);

                double topOfThumbTop = Canvas.GetTop(thumbTop);
                double topOfThumbBottom = Canvas.GetTop(thumbBottom);

                if (_ptOriginalPosition.Y > topOfThumbTop && _ptOriginalPosition.Y < topOfThumbBottom)
                {
                    if (!e.OriginalSource.GetType().Equals(typeof(Thumb)))
                    {
                        sliderV.CaptureMouse();
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, _caption);
            }
        }

        private void sliderV_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            MarkerValue = sliderV.Value;
        }

        #endregion

        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            UpdateRange();
        }
     
        #endregion

    }
}
