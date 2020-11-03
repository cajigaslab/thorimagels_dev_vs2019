namespace ROIUserControl
{
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

    public partial class UserControl1 : UserControl
    {
        #region Fields

        public static readonly DependencyProperty LsmTypeNameProperty = 
                                     DependencyProperty.Register(
                                     "LsmTypeName",
                                     typeof(string),
                                     typeof(UserControl1));

        public static DependencyProperty innerRectAngleProperty = 
                                     DependencyProperty.RegisterAttached("InnerRectAngle",
                                     typeof(double),
                                     typeof(UserControl1),
                                     new FrameworkPropertyMetadata(
                                            new PropertyChangedCallback(onInnerRectAngleAngleValueChanged)));
        public static DependencyProperty innerRectColorProperty = 
                                    DependencyProperty.RegisterAttached("InnerRectColor",
                                    typeof(Brush),
                                    typeof(UserControl1),
                                    new FrameworkPropertyMetadata(
                                           new PropertyChangedCallback(onInnerColorValueChanged)));
        public static DependencyProperty innerRectHeightProperty = 
                                   DependencyProperty.RegisterAttached("InnerRectHeight",
                                   typeof(int),
                                   typeof(UserControl1),
                                   new FrameworkPropertyMetadata(
                                          new PropertyChangedCallback(onInnerHeightValueChanged)));
        public static DependencyProperty innerRectLeftProperty = 
                                        DependencyProperty.RegisterAttached("InnerRectLeft",
                                        typeof(int),
                                        typeof(UserControl1),
                                        new FrameworkPropertyMetadata(
                                               new PropertyChangedCallback(onInnerLeftValueChanged)));
        public static DependencyProperty innerRectPanningState = 
                                  DependencyProperty.RegisterAttached("PanningState",
                                  typeof(bool),
                                  typeof(UserControl1),
                                  new FrameworkPropertyMetadata(
                                         new PropertyChangedCallback(onInnerRectPanningStateChanged)));
        public static DependencyProperty innerRectTopProperty = 
                                 DependencyProperty.RegisterAttached("InnerRectTop",
                                 typeof(int),
                                 typeof(UserControl1),
                                 new FrameworkPropertyMetadata(
                                        new PropertyChangedCallback(onInnerTopValueChanged)));
        public static DependencyProperty innerRectWidthProperty = 
                                   DependencyProperty.RegisterAttached("InnerRectWidth",
                                   typeof(int),
                                   typeof(UserControl1),
                                   new FrameworkPropertyMetadata(
                                          new PropertyChangedCallback(onInnerWidthValueChanged)));
        public static DependencyProperty isInnerCircleVisibleProperty = 
                                  DependencyProperty.RegisterAttached("IsCircleVisible",
                                  typeof(bool),
                                  typeof(UserControl1),
                                  new FrameworkPropertyMetadata(
                                         new PropertyChangedCallback(onCircleVisibilityChanged)));
        public static DependencyProperty outerRectColorProperty = 
                                    DependencyProperty.RegisterAttached("OuterRectColor",
                                    typeof(Brush),
                                    typeof(UserControl1),
                                    new FrameworkPropertyMetadata(
                                           new PropertyChangedCallback(onColorValueChanged)));
        public static DependencyProperty outerRectHeightProperty = 
                                    DependencyProperty.RegisterAttached("OuterRectHeight",
                                    typeof(int),
                                    typeof(UserControl1),
                                    new FrameworkPropertyMetadata(
                                           new PropertyChangedCallback(onHeightValueChanged)));
        public static DependencyProperty outerRectLeftProperty = 
                                         DependencyProperty.RegisterAttached("OuterRectLeft",
                                         typeof(int),
                                         typeof(UserControl1),
                                         new FrameworkPropertyMetadata(
                                                new PropertyChangedCallback(onLeftValueChanged)));
        public static DependencyProperty outerRectTopProperty = 
                                      DependencyProperty.RegisterAttached("OuterRectTop",
                                      typeof(int),
                                      typeof(UserControl1),
                                      new FrameworkPropertyMetadata(
                                             new PropertyChangedCallback(onTopValueChanged)));
        public static DependencyProperty outerRectWidthProperty = 
                                     DependencyProperty.RegisterAttached("OuterRectWidth",
                                     typeof(int),
                                     typeof(UserControl1),
                                     new FrameworkPropertyMetadata(
                                            new PropertyChangedCallback(onWidthValueChanged)));

        static bool _allowDragDraw;
        static double _innerRectAngle = 0;
        static Brush _innerRectColor;
        static int _innerRectHeight = 0;
        static int _innerRectLeft = 0;
        static int _innerRectTop = 0;
        static int _innerRectWidth = 0;
        static bool _isCircleVisible;
        static bool _isInnerRectHeightChanged = false;
        static bool _isInnerRectLeftChanged = false;
        static bool _isInnerRectTopChanged = false;
        static bool _isInnerRectWidthChanged = false;
        static bool _isOuterLeftChanged;
        static bool _isOuterTopChanged;
        static int _oldOuterRectLeft;
        static int _oldOuterRectTop;
        static Brush _outerRectColor;
        static int _outerRectHeight;
        static int _outerRectLeft;
        static int _outerRectTop;
        static int _outerRectWidth;

        Point _center, _topRight, _bottomLeft;
        int _diameter = 0;
        List<Point> _innerRectDimention = null;
        double _mouseHorizontalPosition;
        Point _mouseLeftDownPoint;
        double _mouseVerticalPosition;
        Point _pointAtMaxDistance;

        #endregion Fields

        #region Constructors

        public UserControl1()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public double InnerRectAngle
        {
            get
            {
                { return (double)GetValue(innerRectAngleProperty); }
            }
            set
            {
                { SetValue(innerRectAngleProperty, value); }
            }
        }

        public System.Windows.Media.Brush InnerRectColor
        {
            get
            {
                { return (System.Windows.Media.Brush)GetValue(innerRectColorProperty); }
            }
            set
            {
                { SetValue(innerRectColorProperty, value); }
            }
        }

        public int InnerRectHeight
        {
            get
            {
                { return (int)GetValue(innerRectHeightProperty); }
            }
            set
            {
                { SetValue(innerRectHeightProperty, value); }
            }
        }

        public int InnerRectLeft
        {
            get
            {
                { return (int)GetValue(innerRectLeftProperty); }
            }
            set
            {
                { SetValue(innerRectLeftProperty, value); }
            }
        }

        public int InnerRectTop
        {
            get
            {
                { return (int)GetValue(innerRectTopProperty); }
            }
            set
            {
                { SetValue(innerRectTopProperty, value); }
            }
        }

        public int InnerRectWidth
        {
            get
            {
                { return (int)GetValue(innerRectWidthProperty); }
            }
            set
            {
                { SetValue(innerRectWidthProperty, value); }
            }
        }

        public bool IsCircleVisible
        {
            get
            {
                { return (bool)GetValue(isInnerCircleVisibleProperty); }
            }
            set
            {
                { SetValue(isInnerCircleVisibleProperty, value); }
            }
        }

        public string LsmTypeName
        {
            get
            {
                { return (string)GetValue(LsmTypeNameProperty); }
            }
            set
            {
                { SetValue(LsmTypeNameProperty, value); }
            }
        }

        public Point OuterRectCenter
        {
            get
            {
                try
                {
                    if (_innerRectHeight == _innerRectWidth)
                    {
                        return _center;
                    }
                    else
                    {
                        return new Point(_innerRectWidth / 2, _innerRectHeight / 2);
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Error");
                    return new Point(0, 0);
                }
            }
        }

        public System.Windows.Media.Brush OuterRectColor
        {
            get
            {
                { return (System.Windows.Media.Brush)GetValue(outerRectColorProperty); }
            }
            set
            {
                { SetValue(outerRectColorProperty, value); }
            }
        }

        public int OuterRectHeight
        {
            get
            {
                { return (int)GetValue(outerRectHeightProperty); }
            }
            set
            {
                { SetValue(outerRectHeightProperty, value); }
            }
        }

        public int OuterRectLeft
        {
            get
            {
                { return (int)GetValue(outerRectLeftProperty); }
            }
            set
            {
                { SetValue(outerRectLeftProperty, value); }
            }
        }

        public int OuterRectTop
        {
            get
            {
                { return (int)GetValue(outerRectTopProperty); }
            }
            set
            {
                { SetValue(outerRectTopProperty, value); }
            }
        }

        public int OuterRectWidth
        {
            get
            {
                { return (int)GetValue(outerRectWidthProperty); }
            }
            set
            {
                { SetValue(outerRectWidthProperty, value); }
            }
        }

        public bool PanningState
        {
            get
            {
                { return (bool)GetValue(innerRectPanningState); }
            }
            set
            {
                { SetValue(innerRectPanningState, value); }
            }
        }

        #endregion Properties

        #region Methods

        public static void onCircleVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _isCircleVisible = (bool)e.NewValue;
                if (_isCircleVisible && _outerRectWidth == _outerRectHeight)
                {
                    (d as UserControl1).drawOuterRect();
                }
                else if (_isCircleVisible == false)
                {
                    (d as UserControl1).innerCircle.Visibility = Visibility.Hidden;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        public static void onColorValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _outerRectColor = e.NewValue as System.Windows.Media.Brush;
                (d as UserControl1).outerRect.Fill = _outerRectColor;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        public static void onHeightValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _outerRectHeight = Convert.ToInt32(e.NewValue);
                (d as UserControl1).drawOuterRect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        public static void onInnerColorValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _innerRectColor = e.NewValue as System.Windows.Media.Brush;
                (d as UserControl1).innerRect.Fill = _innerRectColor;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        public static void onInnerHeightValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _innerRectHeight = Convert.ToInt32(e.NewValue);
                _isInnerRectHeightChanged = true;
                (d as UserControl1).drawInnerRect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        public static void onInnerLeftValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _isInnerRectLeftChanged = true;
                _innerRectLeft = Convert.ToInt32(e.NewValue);
                (d as UserControl1).drawInnerRect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        public static void onInnerRectAngleAngleValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _innerRectAngle = Convert.ToDouble(e.NewValue);
                (d as UserControl1).drawInnerRect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        public static void onInnerRectPanningStateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _allowDragDraw = (bool)e.NewValue;
            }
            catch (Exception EX)
            {
                MessageBox.Show(EX.Message, "Error");
            }
        }

        public static void onInnerTopValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _isInnerRectTopChanged = true;
                _innerRectTop = Convert.ToInt32(e.NewValue);
                (d as UserControl1).drawInnerRect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        public static void onInnerWidthValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _innerRectWidth = Convert.ToInt32(e.NewValue);
                _isInnerRectWidthChanged = true;
                (d as UserControl1).drawInnerRect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        public static void onLeftValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _oldOuterRectLeft = _outerRectLeft;
                _isOuterLeftChanged = true;
                _outerRectLeft = Convert.ToInt32(e.NewValue);
                (d as UserControl1).drawOuterRect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        public static void onTopValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _oldOuterRectTop = _outerRectTop;
                _outerRectTop = Convert.ToInt32(e.NewValue);
                _isOuterTopChanged = true;
                (d as UserControl1).drawOuterRect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        public static void onWidthValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _outerRectWidth = Convert.ToInt32(e.NewValue);
                (d as UserControl1).drawOuterRect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        private void drawInnerRect()
        {
            try
            {
                if (_isCircleVisible)
                {
                    if (_innerRectHeight == 0 && _innerRectWidth == 0) return;

                    validateInnerRectDimensionWhenShapeIsCircle();
                    _innerRectDimention = getInnerRectDimensions(_innerRectLeft, _innerRectTop, _innerRectWidth, _innerRectHeight);
                    bool isDimentionValid = ValidateRectDimension(_innerRectDimention);
                    int count = 0;

                    while ((count < 20) && (!isDimentionValid))
                    {
                        _innerRectDimention = getInnerRectDimensions(_innerRectLeft, _innerRectTop, _innerRectWidth, _innerRectHeight);
                        isDimentionValid = ValidateRectDimension(_innerRectDimention);
                        SetInnerRectDimentionsForCircle();
                        count++;
                    }
                    if (isDimentionValid == false)
                        makeFinalAdjustment();
                }
                else
                {
                    bool isInnerRectValid = validateInnerRectDimensionWhenShapeIsSquare();
                }

                Canvas.SetLeft(innerRect, _innerRectLeft);
                Canvas.SetTop(innerRect, _innerRectTop);

                innerRect.Width = _innerRectWidth;
                innerRect.Height = _innerRectHeight;

                innerRect.RenderTransform = new RotateTransform(_innerRectAngle, _innerRectWidth / 2, _innerRectHeight / 2);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        private void drawOuterRect()
        {
            try
            {
                Canvas.SetLeft(outerRect, _outerRectLeft);
                Canvas.SetTop(outerRect, _outerRectTop);
                outerRect.Width = _outerRectWidth;
                outerRect.Height = _outerRectHeight;

                Canvas.SetLeft(innerCircle, _outerRectLeft);
                Canvas.SetTop(innerCircle, _outerRectTop);
                innerCircle.Height = _outerRectHeight;
                innerCircle.Width = _outerRectWidth;

                if (_isCircleVisible && (_outerRectWidth == _outerRectHeight))
                {
                    innerCircle.Visibility = Visibility.Visible;
                }
                else
                {
                    innerCircle.Visibility = Visibility.Hidden;
                }

                setCircleVariables();

                if (_isOuterLeftChanged)
                {
                    int left = _outerRectLeft - _oldOuterRectLeft;
                    _innerRectLeft = _innerRectLeft + left;
                    _isOuterLeftChanged = false;
                }
                if (_isOuterTopChanged)
                {
                    int top = _outerRectTop - _oldOuterRectTop;
                    _innerRectTop = _innerRectTop + top;
                    _isOuterTopChanged = false;
                }
                _isInnerRectTopChanged = _isInnerRectWidthChanged = _isInnerRectHeightChanged = _isInnerRectLeftChanged = true;
                drawInnerRect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        private List<Point> getInnerRectDimensions(int Left, int Top, int Width, int Height)
        {
            try
            {
                List<Point> innerRectPoint = new List<Point>
                {
                    new Point(Left,Top),
                    new Point(Left+Width,Top),
                    new Point(Left, Top+Height),
                    new Point(Left+Width,Top+Height)
                };
                return innerRectPoint;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
                return null;
            }
        }

        private double getMaximumDistanceFromCenter()
        {
            double maxDistance = 0;

            foreach (Point tempPoint in _innerRectDimention)
            {
                double distance = ((tempPoint.X - _center.X) * (tempPoint.X - _center.X)) + ((tempPoint.Y - _center.Y) * (tempPoint.Y - _center.Y));
                double actualDistance = Math.Sqrt(distance);
                if (actualDistance > maxDistance)
                {
                    maxDistance = actualDistance;
                    _pointAtMaxDistance = tempPoint;
                }
            }
            return maxDistance;
        }

        private void innerCircle_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (_allowDragDraw)
            {
                if (!innerCircle.IsMouseCaptured)
                {
                    _mouseLeftDownPoint = e.GetPosition(imageCanvas);
                    innerCircle.CaptureMouse();
                    ValidateInnerRectPoint(_mouseLeftDownPoint);
                }
            }
        }

        private void innerCircle_MouseMove(object sender, MouseEventArgs e)
        {
            if (_allowDragDraw)
            {
                if (innerCircle.IsMouseCaptured)
                {
                    Point currentPoint = e.GetPosition(imageCanvas);

                    int width = Convert.ToInt32(Math.Abs(_mouseLeftDownPoint.X - currentPoint.X));
                    int height = Convert.ToInt32(Math.Abs(_mouseLeftDownPoint.Y - currentPoint.Y));
                    int left = Convert.ToInt32(Math.Min(_mouseLeftDownPoint.X, currentPoint.X));
                    int top = Convert.ToInt32(Math.Min(_mouseLeftDownPoint.Y, currentPoint.Y));

                    List<Point> rectDimensions = getInnerRectDimensions(left, top, width, height);
                    bool ValidPoints = ValidateRectDimension(rectDimensions);

                    if (ValidPoints)
                    {
                        innerRect.Width = width;
                        _innerRectWidth = width;
                        innerRect.Height = height;
                        _innerRectHeight = height;
                        Canvas.SetLeft(innerRect, left);
                        _innerRectLeft = left;
                        Canvas.SetTop(innerRect, top);
                        _innerRectTop = top;

                        //Reflect two way binding values - Start
                        InnerRectHeight = _innerRectHeight;
                        InnerRectWidth = _innerRectWidth;
                        InnerRectTop = _innerRectTop;
                        InnerRectLeft = _innerRectLeft;
                        //Reflect two way binding values - End
                    }
                }
            }
        }

        private void innerCircle_MouseUp(object sender, MouseButtonEventArgs e)
        {
            try
            {
                innerCircle.ReleaseMouseCapture();
                InnerRectHeight = _innerRectHeight;
                InnerRectWidth = _innerRectWidth;
                InnerRectTop = _innerRectTop;
                InnerRectLeft = _innerRectLeft;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        private void innerRect_MouseDown(object sender, MouseButtonEventArgs e)
        {
            innerRect = sender as Rectangle;
            _mouseHorizontalPosition = e.GetPosition(null).X;
            _mouseVerticalPosition = e.GetPosition(null).Y;
            innerRect.CaptureMouse();
        }

        private void innerRect_MouseMove(object sender, MouseEventArgs e)
        {
            List<Point> validPoints;
            if (!innerRect.IsMouseCaptured) return;
            try
            {
                innerRect = sender as Rectangle;

                int deltaH = Convert.ToInt32(e.GetPosition(null).X - _mouseHorizontalPosition);
                int deltaV = Convert.ToInt32(e.GetPosition(null).Y - _mouseVerticalPosition);

                int newTop = Convert.ToInt32(deltaV + (double)innerRect.GetValue(Canvas.TopProperty));
                int newLeft = Convert.ToInt32(deltaH + (double)innerRect.GetValue(Canvas.LeftProperty));

                bool isPointsValid = false;

                int Width = Convert.ToInt32(_innerRectWidth - _outerRectLeft);
                int Height = Convert.ToInt32(_innerRectHeight - _outerRectTop);

                if (_isCircleVisible)
                {
                    validPoints = getInnerRectDimensions(newLeft, newTop, Width + _outerRectLeft, Height + _outerRectTop);
                }
                else
                {
                    validPoints = getInnerRectDimensions(newLeft, newTop, _innerRectWidth, _innerRectHeight);
                }
                isPointsValid = ValidateRectDimension(validPoints);

                if (isPointsValid == true)
                {
                    Canvas.SetTop(innerRect, newTop);
                    _innerRectTop = newTop;
                    if (!LsmTypeName.Contains("Resonance"))
                    {
                        Canvas.SetLeft(innerRect, newLeft);
                        _innerRectLeft = newLeft;
                    }
                    //Reflect two way binding values - Start
                    InnerRectTop = _innerRectTop;
                    InnerRectLeft = _innerRectLeft;
                    //Reflect two way binding values - End

                    _mouseVerticalPosition = e.GetPosition(null).Y;
                    _mouseHorizontalPosition = e.GetPosition(null).X;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        private void innerRect_MouseUp(object sender, MouseButtonEventArgs e)
        {
            try
            {
                innerRect = sender as Rectangle;
                innerRect.ReleaseMouseCapture();
                _mouseVerticalPosition = -1;
                _mouseHorizontalPosition = -1;
                InnerRectTop = _innerRectTop;
                InnerRectLeft = _innerRectLeft;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        private void makeFinalAdjustment()
        {
            _innerRectDimention = getInnerRectDimensions(_innerRectLeft, _innerRectTop, _innerRectWidth, _innerRectHeight);
            double maximumDistanceFromCenter = getMaximumDistanceFromCenter();
            double adjustmentDistance = maximumDistanceFromCenter - (_diameter / 2);
            adjustmentDistance = Math.Sqrt((adjustmentDistance * adjustmentDistance) / 2);

            if (adjustmentDistance > 2)
            {
                if (_innerRectWidth - adjustmentDistance > 0 && _innerRectHeight - adjustmentDistance > 0)
                {
                    _innerRectHeight = Convert.ToInt32(Math.Abs(_innerRectHeight - adjustmentDistance));
                    _innerRectWidth = Convert.ToInt32(Math.Abs(_innerRectWidth - adjustmentDistance));
                }
                else if (_innerRectWidth - adjustmentDistance < 0 && _innerRectHeight - adjustmentDistance > 0)
                {
                    _innerRectHeight = Convert.ToInt32(Math.Abs(_innerRectHeight - adjustmentDistance));
                }
                else if (_innerRectWidth - adjustmentDistance > 0 && _innerRectHeight - adjustmentDistance < 0)
                {
                    _innerRectWidth = Convert.ToInt32(Math.Abs(_innerRectWidth - adjustmentDistance));
                }
                drawInnerRect();
            }
        }

        private void outerRect_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (_allowDragDraw)
            {
                if (!outerRect.IsMouseCaptured)
                {
                    _mouseLeftDownPoint = e.GetPosition(imageCanvas);
                    outerRect.CaptureMouse();
                }
            }
        }

        private void outerRect_MouseMove(object sender, MouseEventArgs e)
        {
            if (_allowDragDraw)
            {
                if (outerRect.IsMouseCaptured)
                {
                    Point currentPoint = e.GetPosition(imageCanvas);

                    int width = Convert.ToInt32(Math.Abs(_mouseLeftDownPoint.X - currentPoint.X));
                    int height = Convert.ToInt32(Math.Abs(_mouseLeftDownPoint.Y - currentPoint.Y));
                    int left = Convert.ToInt32(Math.Min(_mouseLeftDownPoint.X, currentPoint.X));
                    int top = Convert.ToInt32(Math.Min(_mouseLeftDownPoint.Y, currentPoint.Y));

                    List<Point> rectDimensions = getInnerRectDimensions(left, top, width, height);
                    bool ValidPoints = ValidateRectDimension(rectDimensions);

                    if (ValidPoints)
                    {
                        innerRect.Width = width;
                        innerRect.Height = height;
                        Canvas.SetLeft(innerRect, left);
                        Canvas.SetTop(innerRect, top);
                        _innerRectHeight = height;
                        _innerRectWidth = width;
                        _innerRectLeft = left;
                        _innerRectTop = top;

                        //Reflect two way binding values - Start
                        InnerRectHeight = _innerRectHeight;
                        InnerRectWidth = _innerRectWidth;
                        InnerRectTop = _innerRectTop;
                        InnerRectLeft = _innerRectLeft;
                        //Reflect two way binding values - End
                    }
                }
            }
        }

        private void outerRect_MouseUp(object sender, MouseButtonEventArgs e)
        {
            try
            {
                outerRect.ReleaseMouseCapture();
                InnerRectHeight = _innerRectHeight;
                InnerRectWidth = _innerRectWidth;
                InnerRectTop = _innerRectTop;
                InnerRectLeft = _innerRectLeft;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        private void setCircleVariables()
        {
            try
            {
                _bottomLeft = new Point(_outerRectLeft, _outerRectTop + _outerRectHeight);
                _topRight = new Point(_outerRectWidth + _outerRectLeft, _outerRectTop);
                _center = new Point((_outerRectWidth / 2), (_outerRectHeight / 2));
                _center.X = _center.X + _outerRectLeft;
                _center.Y = _center.Y + _outerRectTop;
                _diameter = _outerRectHeight;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        private void SetInnerRectDimentionsForCircle()
        {
            double maximumDistanceFromCenter = getMaximumDistanceFromCenter();
            double adjustmentDistance = Math.Abs(maximumDistanceFromCenter - (_diameter / 2));
            Point topLeftOfInner = new Point(_innerRectLeft, _innerRectTop);
            adjustmentDistance = Convert.ToInt32(Math.Sqrt((adjustmentDistance * adjustmentDistance) / 2));

            if (_pointAtMaxDistance.X > _center.X && _pointAtMaxDistance.Y > _center.Y) //4th
            {
                _innerRectLeft = Convert.ToInt32(Math.Abs(_innerRectLeft - adjustmentDistance));
                _innerRectTop = Convert.ToInt32(Math.Abs(_innerRectTop - adjustmentDistance));
            }
            else if (_pointAtMaxDistance.X < _center.X && _pointAtMaxDistance.Y < _center.Y)//1st
            {
                _innerRectLeft = Convert.ToInt32(_innerRectLeft + adjustmentDistance);
                _innerRectTop = Convert.ToInt32(_innerRectTop + adjustmentDistance);
            }
            else if (_pointAtMaxDistance.X > _center.X && _pointAtMaxDistance.Y < _center.Y)//2nd
            {
                _innerRectLeft = Convert.ToInt32(Math.Abs(_innerRectLeft - adjustmentDistance));
                _innerRectTop = Convert.ToInt32(_innerRectTop + adjustmentDistance);
            }
            else if (_pointAtMaxDistance.X < _center.X && _pointAtMaxDistance.Y > _center.Y)//3rd
            {
                _innerRectLeft = Convert.ToInt32(Math.Abs(_innerRectLeft + adjustmentDistance));
                _innerRectTop = Convert.ToInt32(_innerRectTop - adjustmentDistance);
            }
        }

        private void validateInnerRectDimensionWhenShapeIsCircle()
        {
            if (_innerRectHeight > _diameter)
                _innerRectHeight = _diameter;
            if (_innerRectWidth > _diameter)
                _innerRectWidth = _diameter;
            if (_innerRectLeft < _outerRectLeft)
                _innerRectLeft = _outerRectLeft;
            if (_innerRectLeft > _outerRectLeft + _outerRectWidth)
                _innerRectLeft = Math.Abs(_outerRectWidth - _innerRectWidth);
            if (_innerRectTop < _outerRectTop)
                _innerRectTop = _outerRectTop;
            if (_innerRectTop > _outerRectTop + _outerRectWidth)
                _innerRectTop = Math.Abs(_outerRectHeight - _innerRectHeight);
        }

        private bool validateInnerRectDimensionWhenShapeIsSquare()
        {
            bool isValid = true;
            if (_isInnerRectLeftChanged)
            {
                if (_innerRectLeft < _outerRectLeft)
                    _innerRectLeft = _outerRectLeft;
                if (_innerRectLeft > (_outerRectLeft + _outerRectWidth))
                    _innerRectLeft = (_outerRectLeft + _outerRectWidth) - _innerRectWidth;
                if ((_innerRectWidth + _innerRectLeft) > (_outerRectWidth + _outerRectLeft))
                {
                    int diff = Math.Abs((_innerRectWidth + _innerRectLeft) - (_outerRectWidth + _outerRectLeft));
                    _innerRectLeft = Math.Abs(_innerRectLeft - diff);
                }
            }
            if (_isInnerRectTopChanged)
            {
                if (_innerRectTop < _outerRectTop)
                    _innerRectTop = _outerRectTop;
                if (_innerRectTop > (_outerRectTop + _outerRectHeight))
                    _innerRectTop = (_outerRectTop + _outerRectHeight) - _innerRectHeight;
                if ((_innerRectHeight + _innerRectTop) > (_outerRectHeight + _outerRectTop))
                {
                    int diff = Math.Abs((_innerRectHeight + _innerRectTop) - (_outerRectHeight + _outerRectTop));
                    _innerRectTop = Math.Abs(_innerRectTop - diff);
                }
            }
            if (_isInnerRectWidthChanged)
            {
                if (_innerRectWidth >= _outerRectWidth)
                {
                    _innerRectWidth = _outerRectWidth;
                    _innerRectLeft = _outerRectLeft;
                }
                if ((_innerRectWidth + _innerRectLeft) > (_outerRectWidth + _outerRectLeft))
                {
                    int diff = Math.Abs((_innerRectWidth + _innerRectLeft) - (_outerRectWidth + _outerRectLeft));
                    _innerRectLeft = Math.Abs(_innerRectLeft - diff);
                }
            }
            if (_isInnerRectHeightChanged)
            {
                if (_innerRectHeight >= _outerRectHeight)
                {
                    _innerRectHeight = _outerRectHeight;
                    _innerRectTop = _outerRectTop;
                }
                if ((_innerRectHeight + _innerRectTop) > (_outerRectHeight + _outerRectTop))
                {
                    int diff = Math.Abs((_innerRectHeight + _innerRectTop) - (_outerRectHeight + _outerRectTop));
                    _innerRectTop = Math.Abs(_innerRectTop - diff);

                }
            }
            return isValid;
        }

        private bool ValidateInnerRectPoint(Point innerRectPoint)
        {
            bool valid = false;

            double a, b;
            a = (_topRight.X - _bottomLeft.X) / 2;
            b = (_topRight.Y - _bottomLeft.Y) / 2;

            double pointX, pointY;

            pointX = (double)(Math.Pow(innerRectPoint.X - _center.X, 2) / Math.Pow(a, 2));
            pointY = (double)(Math.Pow(innerRectPoint.Y - _center.Y, 2) / Math.Pow(b, 2));

            double value = pointX + pointY;

            if (value < 1)
                valid = true;
            else
            {
                valid = false;
                return valid;
            }

            return valid;
        }

        private bool ValidateRectDimension(List<Point> innerRectPoint)
        {
            bool valid = false;

            if (_isCircleVisible)
            {
                double a, b;
                a = (_topRight.X - _bottomLeft.X) / 2;
                b = (_topRight.Y - _bottomLeft.Y) / 2;

                foreach (Point temp in innerRectPoint)
                {
                    double pointX, pointY;

                    pointX = (double)(Math.Pow(temp.X - _center.X, 2) / Math.Pow(a, 2));
                    pointY = (double)(Math.Pow(temp.Y - _center.Y, 2) / Math.Pow(b, 2));

                    double value = pointX + pointY;

                    if (value < 1)
                    {
                        valid = true;
                    }
                    else
                    {
                        valid = false;
                        return valid;
                    }
                }
                return valid;
            }
            else
            {
                foreach (Point temp in innerRectPoint)
                {
                    if (temp.X > _outerRectLeft && temp.X < _outerRectWidth + _outerRectLeft && temp.Y > _outerRectTop && temp.Y < _outerRectHeight + _outerRectTop)
                        valid = true;
                    else
                    {
                        valid = false;
                        return valid;
                    }
                }
                return valid;
            }
        }

        #endregion Methods
    }
}