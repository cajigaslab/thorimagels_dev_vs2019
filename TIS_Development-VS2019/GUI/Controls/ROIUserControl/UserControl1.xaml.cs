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

        public static DependencyProperty numberOfFieldStripsProperty = 
                                    DependencyProperty.RegisterAttached("NumberOfFieldStrips", 
                                    typeof(int), 
                                    typeof(UserControl1), 
                                    new FrameworkPropertyMetadata(
                                         new PropertyChangedCallback(onNumberOfStripsValueChanged)));

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
        static bool _isNumberFieldStripsChanged;
        static int _oldOuterRectLeft;
        static int _oldOuterRectTop;
        static Brush _outerRectColor;
        static int _outerRectHeight;
        static int _outerRectLeft;
        static int _outerRectTop;
        static int _outerRectWidth;
        static int _numberFieldStrips;
        Point _center, _topRight, _bottomLeft;
        int _diameter = 0;
        List<Point> _innerRectDimension = null;
        List<Line> fieldDivisionLines;
        double _mouseHorizontalPosition;
        Point _mouseLeftDownPoint;
        double _mouseVerticalPosition;
        Point _pointAtMaxDistance;

        #endregion Fields

        #region Constructors

        public UserControl1()
        {
            InitializeComponent();
            fieldDivisionLines = new List<Line>();
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
                { SetValue(innerRectLeftProperty, value);}
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

        public int NumberOfFieldStrips
        {
            get
            {
                { return (int)GetValue(numberOfFieldStripsProperty); }
            }
            set
            {
                { SetValue(numberOfFieldStripsProperty, value); }
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

        public static void onNumberOfStripsValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _isNumberFieldStripsChanged = true;
                _numberFieldStrips = Convert.ToInt32(e.NewValue);
                (d as UserControl1).drawInnerRect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        private void drawDivisionLines()
        {
            //Number of divisions changed, clear array and add new children to canvas
            //Only need to draw lines if number of divisions is greater than 1. 
            if (_isNumberFieldStripsChanged)
            {
                //Check if any lines are presently drawn. If so, remove them
                if (0 != fieldDivisionLines.Count)
                {
                    for (int i = 0; i < fieldDivisionLines.Count; i++)
                    {
                        imageCanvas.Children.Remove(fieldDivisionLines[i]);
                    }
                }
                fieldDivisionLines = new List<Line>();

                //Fill the list of lines with _numberFieldStrips-1 items. 
                for (int i = 1; i < _numberFieldStrips; i++)
                {
                    fieldDivisionLines.Add(new Line()
                    {
                        X1 = 0,
                        X2 = 0,
                        Y1 = 0,
                        Y2 = 0,
                        StrokeThickness = 1,
                        Visibility = Visibility.Visible,
                        Stroke = Brushes.Black,
                        StrokeDashArray = new DoubleCollection(new Double[] { 3, 3 })
                    });
                }
                _isNumberFieldStripsChanged = false;
            }
            try
            {
                //positions for drawing in pixel coordinates
                double innerRectCenterX = _innerRectLeft + _innerRectWidth / 2;
                double innerRectCenterY = _innerRectTop + _innerRectHeight / 2;
                double nonRotatedLineCenterX;
                double nonRotatedLineCenterY;
                double rotatedLineCenterX;
                double rotatedLineCenterY;

                // magnitude in pixels
                double lineDistanceFromCenterInnerRect;

                //top and bottom positions for each line
                double lineTopX;
                double lineTopY;
                double lineBottomX;
                double lineBottomY;

                for (int i = 0; i < fieldDivisionLines.Count; i++)
                {
                    nonRotatedLineCenterX = _innerRectLeft + (_innerRectWidth / _numberFieldStrips) * (i + 1);
                    nonRotatedLineCenterY = innerRectCenterY;

                    //Distance formula. Center of the line to the center of the rectangle. This should be constant for all rotation angles for each line
                    lineDistanceFromCenterInnerRect =
                        Math.Sqrt(Math.Pow(nonRotatedLineCenterX - innerRectCenterX, 2) + Math.Pow(nonRotatedLineCenterY - innerRectCenterY, 2));

                    //Change in X and Y position of the center of a line rotating around a point given by: 
                    // deltaX = distanceFromCenter - distanceFromCenter*cos(angle)
                    // deltaY = distanceFromCenter*sin(angle)
                    //Sign functions used to denote where the line center is relative to rectangle center
                    rotatedLineCenterX = nonRotatedLineCenterX +
                        (lineDistanceFromCenterInnerRect - lineDistanceFromCenterInnerRect * Math.Cos(_innerRectAngle * Math.PI / 180)) *
                        Math.Sign(innerRectCenterX - nonRotatedLineCenterX);
                    rotatedLineCenterY = nonRotatedLineCenterY -
                        lineDistanceFromCenterInnerRect * Math.Sin(_innerRectAngle * Math.PI / 180) * Math.Sign(innerRectCenterX - nonRotatedLineCenterX);

                    lineTopX = rotatedLineCenterX;
                    lineBottomX = rotatedLineCenterX;
                    lineTopY = rotatedLineCenterY + _innerRectHeight / 2;
                    lineBottomY = rotatedLineCenterY - _innerRectHeight / 2;

                    if (!imageCanvas.Children.Contains(fieldDivisionLines[i]))
                    {
                        imageCanvas.Children.Add(fieldDivisionLines[i]);
                    }
                    fieldDivisionLines[i].X1 = lineTopX;
                    fieldDivisionLines[i].X2 = lineBottomX;
                    fieldDivisionLines[i].Y1 = lineTopY;
                    fieldDivisionLines[i].Y2 = lineBottomY;

                    //render transform rotates line around calculated center point. makes the code cleaner with much fewer calculations
                    fieldDivisionLines[i].RenderTransform = new RotateTransform(_innerRectAngle, rotatedLineCenterX, rotatedLineCenterY);                    
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        private Double Distance(double x1, double x2, double y1, double y2)
        {
            return Math.Sqrt(Math.Pow(x2- x1, 2) + Math.Pow(y2 - y1, 2));
        }

        private void drawInnerRect()
        {
            //_isCircleVisible = false;
            try
            {
                if (_isCircleVisible)
                {
                    if (_innerRectHeight == 0 && _innerRectWidth == 0) return;

                    validateInnerRectDimensionWhenShapeIsCircle();
                    _innerRectDimension = getInnerRectDimensions(_innerRectLeft, _innerRectTop, _innerRectWidth, _innerRectHeight);
                    bool isDimensionValid = ValidateRectDimension(_innerRectDimension);
                    int count = 0;

                    while ((count < 20) && (!isDimensionValid))
                    {
                        _innerRectDimension = getInnerRectDimensions(_innerRectLeft, _innerRectTop, _innerRectWidth, _innerRectHeight);
                        isDimensionValid = ValidateRectDimension(_innerRectDimension);
                        SetInnerRectDimensionsForCircle();
                        count++;
                    }
                    if (isDimensionValid == false)
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
                drawDivisionLines();
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
                    RotatePointAroundInnerRectCenter(new Point(Left,Top)),
                    RotatePointAroundInnerRectCenter(new Point(Left+Width,Top)),
                    RotatePointAroundInnerRectCenter(new Point(Left, Top+Height)),
                    RotatePointAroundInnerRectCenter(new Point(Left+Width,Top+Height))
                };
                return innerRectPoint;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
                return null;
            }
        }

        private Point RotatePointAroundInnerRectCenter(Point nonRotatedPoint)
        {
            //https://www.euclideanspace.com/maths/geometry/affine/aroundPoint/matrix2d/index.htm
            Point rotatedPoint = new Point(nonRotatedPoint.X,nonRotatedPoint.Y);
            double rotationAngle = _innerRectAngle * Math.PI / 180; //convert to radians
            double innerRectCenterX = _innerRectLeft + _innerRectWidth / 2;
            double innerRectCenterY = _innerRectTop + _innerRectHeight / 2;

            rotatedPoint.X = nonRotatedPoint.X * Math.Cos(rotationAngle) + -1 * nonRotatedPoint.Y * Math.Sin(rotationAngle) + innerRectCenterX -
                innerRectCenterX * Math.Cos(rotationAngle) - -1 * innerRectCenterY * Math.Sin(rotationAngle);

            rotatedPoint.Y = nonRotatedPoint.X*Math.Sin(rotationAngle) + nonRotatedPoint.Y * Math.Cos(rotationAngle) + innerRectCenterY -
                innerRectCenterX * Math.Sin(rotationAngle) - innerRectCenterY * Math.Cos(rotationAngle); 

            return rotatedPoint;
        }

        private double getMaximumDistanceFromCenter()
        {
            double maxDistance = 0;

            foreach (Point tempPoint in _innerRectDimension)
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
            if (!innerRect.IsMouseCaptured) return;
            try
            {
                innerRect = sender as Rectangle;

                int deltaH = Convert.ToInt32(e.GetPosition(null).X - _mouseHorizontalPosition);
                int deltaV = Convert.ToInt32(e.GetPosition(null).Y - _mouseVerticalPosition);

                Point actualMoveAmount = NudgeDeltaValuesForInnerRectMove(deltaH, deltaV);

                int newTop = Convert.ToInt32(actualMoveAmount.Y + (double)innerRect.GetValue(Canvas.TopProperty));
                int newLeft = Convert.ToInt32(actualMoveAmount.X + (double)innerRect.GetValue(Canvas.LeftProperty));

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

                UpdateLinePositionsWhenDragged();
            }
            catch (Exception)
            {}
        }

        //adjust the requested change in position for innerRect so that the new position will be valid
        //delta values represent the amount the inner rect is requested to move
        private Point NudgeDeltaValuesForInnerRectMove(int deltaH, int deltaV)
        {
            //Corner furthest from the center at requested position
            Point innerRectCornerAtMaxDistance = GetPointFurthestFromCenter(deltaH, deltaV);

            Point adjustedDistances = new Point(deltaH,deltaV);
            if (_isCircleVisible)
            {
                double cornerDistanceFromCenter = Math.Sqrt(Math.Pow(innerRectCornerAtMaxDistance.X - _center.X, 2) + Math.Pow(innerRectCornerAtMaxDistance.Y - _center.Y, 2));
                double polarAngleToCorner;

                if (cornerDistanceFromCenter > innerCircle.Width / 2)
                {
                    //corner at the requested position of innerRect is outside of the circle.
                    //project the requested point to the edge of the closest point on the circle. This will always be on the line
                    //drawn from center of circle to the point. 
                    //polar coordinate angle to the point inverseTan(yDistanceFromCenter/xDistanceFromCenter)
                    polarAngleToCorner = Math.Atan((innerRectCornerAtMaxDistance.Y - _center.Y) / (innerRectCornerAtMaxDistance.X - _center.X));

                    //point on circle closest to requested point
                    double projectionPointX = _center.X + Math.Sign(innerRectCornerAtMaxDistance.X - _center.X) * (innerCircle.Width / 2) * Math.Cos(polarAngleToCorner);
                    double projectionPointY = _center.Y + Math.Sign(innerRectCornerAtMaxDistance.X - _center.X) * (innerCircle.Width / 2) * Math.Sin(polarAngleToCorner);

                    //Move overlapped corner to the edge of the circle
                    adjustedDistances.X = deltaH + (innerRectCornerAtMaxDistance.X - projectionPointX) * -1;
                    adjustedDistances.Y = deltaV + (innerRectCornerAtMaxDistance.Y - projectionPointY) * -1;
                }
            }
            else
            {
                //Circle is not visible containing box is a square
                //corner is out of bounds in the x
                if (innerRectCornerAtMaxDistance.X < _outerRectLeft)
                {
                    adjustedDistances.X = deltaH + _outerRectLeft - innerRectCornerAtMaxDistance.X;
                }
                else if (innerRectCornerAtMaxDistance.X > _outerRectLeft + _outerRectWidth)
                {
                    adjustedDistances.X = deltaH - innerRectCornerAtMaxDistance.X + _outerRectLeft + _outerRectWidth;
                }

                //corner is out of bounds in the y
                if (innerRectCornerAtMaxDistance.Y < _outerRectTop)
                {
                    adjustedDistances.Y = deltaV + _outerRectTop - innerRectCornerAtMaxDistance.Y;
                }
                else if (innerRectCornerAtMaxDistance.Y > _outerRectTop + _outerRectHeight)
                {
                    adjustedDistances.Y = deltaV - innerRectCornerAtMaxDistance.Y + _outerRectTop + _outerRectHeight;
                }
            }
            return adjustedDistances;
        }

        //Gets the corner of inner rect that is furthest from the center of the view area. 
        //DeltaH and DeltaV can be used to get the corner from a requested position
        private Point GetPointFurthestFromCenter(int deltaH, int deltaV)
        {
            List<Point> innerRectPoints = getInnerRectDimensions((int)((double)innerRect.GetValue(Canvas.LeftProperty) + deltaH), (int)((double)innerRect.GetValue(Canvas.TopProperty) + deltaV), _innerRectWidth, _innerRectHeight);
            Point furthestPoint = new Point();
            double currentFurthestDistance = 0;

            foreach (Point corner in innerRectPoints)
            {
                double pointDistanceFromCenter =
                    Math.Sqrt(Math.Pow(corner.X - _center.X, 2) + Math.Pow(corner.Y - _center.Y, 2));
                if (pointDistanceFromCenter > currentFurthestDistance)
                {
                    currentFurthestDistance = pointDistanceFromCenter;
                    furthestPoint = corner;
                }
            }

            return furthestPoint;
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
            _innerRectDimension = getInnerRectDimensions(_innerRectLeft, _innerRectTop, _innerRectWidth, _innerRectHeight);
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

        private void SetInnerRectDimensionsForCircle()
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

        private void UpdateLinePositionsWhenDragged()
        {
            /**The DrawDivisionLines method is not always called during a drag event leading to the position of the lines not always
            matching the position of the inner rectangle. This is caused by the different ways the canvas is handling the children. 
            The main shapes are declared in xaml with positions bound to the cs, while the lines are declared and positions set in the cs. 
            When the binding values change in the innerRectDrag method, the positions for the xaml shapes will automatically change while the lines
            stay in the current position until the draw method is called. This method updates the lines while a drag is happening
            and is just the draw-logic portion of the DrawDivisionLines method. **/

            try
            {
                double innerRectCenterX = _innerRectLeft + _innerRectWidth / 2;
                double innerRectCenterY = _innerRectTop + _innerRectHeight / 2;
                double nonRotatedLineCenterX;
                double nonRotatedLineCenterY;
                double rotatedLineCenterX;
                double rotatedLineCenterY;

                double lineDistanceFromCenterInnerRect;

                double lineTopX;
                double lineTopY;
                double lineBottomX;
                double lineBottomY;

                for (int i = 0; i < fieldDivisionLines.Count; i++)
                {
                    nonRotatedLineCenterX = _innerRectLeft + (_innerRectWidth / _numberFieldStrips) * (i + 1);
                    nonRotatedLineCenterY = innerRectCenterY;

                    lineDistanceFromCenterInnerRect =
                        Math.Sqrt(Math.Pow(nonRotatedLineCenterX - innerRectCenterX, 2) + Math.Pow(nonRotatedLineCenterY - innerRectCenterY, 2));
                    rotatedLineCenterX = nonRotatedLineCenterX +
                        (lineDistanceFromCenterInnerRect - lineDistanceFromCenterInnerRect * Math.Cos(_innerRectAngle * Math.PI / 180)) *
                        Math.Sign(innerRectCenterX - nonRotatedLineCenterX);
                    rotatedLineCenterY = nonRotatedLineCenterY -
                        lineDistanceFromCenterInnerRect * Math.Sin(_innerRectAngle * Math.PI / 180) * Math.Sign(innerRectCenterX - nonRotatedLineCenterX);

                    lineTopX = rotatedLineCenterX;
                    lineBottomX = rotatedLineCenterX;
                    lineTopY = rotatedLineCenterY + _innerRectHeight / 2;
                    lineBottomY = rotatedLineCenterY - _innerRectHeight / 2;

                    fieldDivisionLines[i].X1 = lineTopX;
                    fieldDivisionLines[i].X2 = lineBottomX;
                    fieldDivisionLines[i].Y1 = lineTopY;
                    fieldDivisionLines[i].Y2 = lineBottomY;
                    fieldDivisionLines[i].RenderTransform = new RotateTransform(_innerRectAngle, rotatedLineCenterX, rotatedLineCenterY);
                }
            }
            catch (Exception)
            {}
        }
        #endregion Methods
    }
}