namespace OverlayManager
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Shapes;

    using ThorLogging;

    public class AdornerProvider : Adorner
    {
        #region Fields

        private static int _imageHeight;
        private static int _imageWidth;

        private Type _adornedElementType;
        private Thumb _linePanningThumb; // For Line Overlay
        private Thumb[] _lineResizingThumb; // For Line Overlay
        private Thumb _polyPanningThumb; // for Polygon Overlay and Polyline Overlay
        private Thumb[] _polyReshapeThumb; // For Polygon and polyline Overlays
        private Thumb _rectPanningThumb; // For Rectangle & Text Overlays
        private Thumb _rectResizingThumbTop, _rectResizingThumbBottom, _rectResizingThumbLeft, _rectResizingThumbRight; // For Rectangle Overlays
        private Thumb _singleThumb; //For Reticle, Crosshair, Ellipse Overlays
        private VisualCollection _visualChildren;

        #endregion Fields

        #region Constructors

        public AdornerProvider(UIElement adornedElement, int imageWidth, int imageHeight)
            : base(adornedElement)
        {
            _adornedElementType = adornedElement.GetType();

            _imageWidth = imageWidth;
            _imageHeight = imageHeight;

            if (_adornedElementType.Name.Equals("TextBox") || _adornedElementType.Name.Equals("ROIRect"))
            {
                _visualChildren = new VisualCollection(this);

                BuildPanningdAdornerCorner(ref _rectPanningThumb, Cursors.ScrollAll);

                BuildResizeAdornerCorner(ref _rectResizingThumbTop, Cursors.SizeNS);
                BuildResizeAdornerCorner(ref _rectResizingThumbBottom, Cursors.SizeNS);
                BuildResizeAdornerCorner(ref _rectResizingThumbLeft, Cursors.SizeWE);
                BuildResizeAdornerCorner(ref _rectResizingThumbRight, Cursors.SizeWE);

                _rectPanningThumb.DragDelta += new DragDeltaEventHandler(RectPanningThumb_DragDelta);
                _rectPanningThumb.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);

                _rectResizingThumbTop.DragDelta += new DragDeltaEventHandler(RectResizingThumbTop_DragDelta);
                _rectResizingThumbTop.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);

                _rectResizingThumbBottom.DragDelta += new DragDeltaEventHandler(RectResizingThumbBottom_DragDelta);
                _rectResizingThumbBottom.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);

                _rectResizingThumbRight.DragDelta += new DragDeltaEventHandler(RectResizingThumbRight_DragDelta);
                _rectResizingThumbRight.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);

                _rectResizingThumbLeft.DragDelta += new DragDeltaEventHandler(RectResizingThumbLeft_DragDelta);
                _rectResizingThumbLeft.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);
            }
            else if (_adornedElementType.Name.Equals("Line"))
            {
                _visualChildren = new VisualCollection(this);
                BuildPanningdAdornerCorner(ref _linePanningThumb, Cursors.ScrollAll);
                _linePanningThumb.DragDelta += new DragDeltaEventHandler(LinePanningThumb_DragDelta);
                _linePanningThumb.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);
                _lineResizingThumb = new Thumb[2];
                for (int i = 0; i < 2; i++)
                {
                    BuildResizeAdornerCorner(ref _lineResizingThumb[i], Cursors.SizeAll);
                    _lineResizingThumb[i].DragDelta += new DragDeltaEventHandler(LineResizeThumb_DragDelta);
                    _lineResizingThumb[i].DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);
                    _lineResizingThumb[i].Tag = i;
                }
            }
            else if (_adornedElementType.Name.Equals("ROIPoly"))
            {
                _visualChildren = new VisualCollection(this);
                int polyPoints = ((ROIPoly)adornedElement).Points.Count;
                if (polyPoints > 2)
                {
                    BuildPanningdAdornerCorner(ref _polyPanningThumb, Cursors.ScrollAll);
                    _polyPanningThumb.DragDelta += new DragDeltaEventHandler(PolyPannigThumb_DragDelta);
                    _polyPanningThumb.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);
                    _polyReshapeThumb = new Thumb[polyPoints];
                    for (int i = 0; i < polyPoints; i++)
                    {
                        BuildResizeAdornerCorner(ref _polyReshapeThumb[i], Cursors.SizeAll);
                        _polyReshapeThumb[i].DragDelta += new DragDeltaEventHandler(PolyReshapeThumb_DragDelta);
                        _polyReshapeThumb[i].DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);
                        _polyReshapeThumb[i].Tag = i;
                    }
                }
            }
            else if (_adornedElementType.Name.Equals("ROICrosshair"))
            {
                _visualChildren = new VisualCollection(this);
                BuildPanningdAdornerCorner(ref _singleThumb, Cursors.ScrollAll);
                _singleThumb.DragDelta += new DragDeltaEventHandler(CrosshairPanningThumb_DragDelta);
                _singleThumb.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);
            }
            else if (_adornedElementType.Name.Equals("Reticle"))
            {
                _visualChildren = new VisualCollection(this);
                BuildPanningdAdornerCorner(ref _singleThumb, Cursors.Arrow);
            }
            else if (_adornedElementType.Name.Equals("Polyline"))
            {
                _visualChildren = new VisualCollection(this);
                int polyPoints = ((Polyline)adornedElement).Points.Count;

                if (polyPoints > 1)
                {
                    BuildPanningdAdornerCorner(ref _polyPanningThumb, Cursors.ScrollAll);
                    _polyPanningThumb.DragDelta += new DragDeltaEventHandler(PolylinePannigThumb_DragDelta);
                    _polyPanningThumb.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);
                    _polyReshapeThumb = new Thumb[polyPoints];
                    for (int i = 0; i < polyPoints; i++)
                    {
                        BuildResizeAdornerCorner(ref _polyReshapeThumb[i], Cursors.SizeAll);
                        _polyReshapeThumb[i].DragDelta += new DragDeltaEventHandler(PolylineReshapeThumb_DragDelta);
                        _polyReshapeThumb[i].DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);
                        _polyReshapeThumb[i].Tag = i;
                    }
                }
            }
            else if (_adornedElementType.Name.Equals("ROIEllipse"))
            {
                _visualChildren = new VisualCollection(this);
                BuildPanningdAdornerCorner(ref _rectPanningThumb, Cursors.ScrollAll);
                BuildResizeAdornerCorner(ref _rectResizingThumbTop, Cursors.SizeNS);
                BuildResizeAdornerCorner(ref _rectResizingThumbBottom, Cursors.SizeNS);
                BuildResizeAdornerCorner(ref _rectResizingThumbLeft, Cursors.SizeWE);
                BuildResizeAdornerCorner(ref _rectResizingThumbRight, Cursors.SizeWE);

                _rectPanningThumb.DragDelta += new DragDeltaEventHandler(EllipsePanningThumb_DragDelta);
                _rectPanningThumb.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);

                _rectResizingThumbTop.DragDelta += new DragDeltaEventHandler(EllipseResizingThumbTop_DragDelta);
                _rectResizingThumbTop.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);

                _rectResizingThumbBottom.DragDelta += new DragDeltaEventHandler(EllipseResizingThumbBottom_DragDelta);
                _rectResizingThumbBottom.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);

                _rectResizingThumbRight.DragDelta += new DragDeltaEventHandler(EllipseResizingThumbRight_DragDelta);
                _rectResizingThumbRight.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);

                _rectResizingThumbLeft.DragDelta += new DragDeltaEventHandler(EllipseResizingThumbLeft_DragDelta);
                _rectResizingThumbLeft.DragCompleted += new DragCompletedEventHandler(Thumb_DragComplete);
            }
        }

        #endregion Constructors

        #region Events

        public event Action<int> UpdateLinePosition;

        public event Action<Shape> UpdateNow;

        #endregion Events

        #region Properties

        public static int ImageHeight
        {
            get { return _imageHeight; }
            set { _imageHeight = value; }
        }

        public static int ImageWidth
        {
            get { return _imageWidth; }
            set { _imageWidth = value; }
        }

        protected override int VisualChildrenCount
        {
            get { return _visualChildren.Count; }
        }

        #endregion Properties

        #region Methods

        protected override Size ArrangeOverride(Size finalSize)
        {
            try
            {
                if (_adornedElementType.Name.Equals("TextBox"))
                {
                    double desiredWidth = AdornedElement.DesiredSize.Width;
                    double desiredHeight = AdornedElement.DesiredSize.Height;

                    double width = (double)(AdornedElement as TextBox).Width;
                    (AdornedElement as TextBox).Focus();

                    double adornerWidth = this.DesiredSize.Width;
                    double adornerHeight = this.DesiredSize.Height;
                }
                else if (_adornedElementType.Name.Equals("ROIRect"))
                {
                    UpdateRectThumbsPos();
                }
                else if (_adornedElementType.Name.Equals("Line"))
                {
                    UpdateLineThumbPos();
                }
                else if (_adornedElementType.Name.Equals("ROIPoly"))
                {
                    UpdatePolyThumbPos();
                }
                else if (_adornedElementType.Name.Equals("ROICrosshair"))
                {
                    ROICrosshair temp = AdornedElement as ROICrosshair;
                    Point corner = new Point();
                    corner.X = temp.CenterPoint.X - 3.5;
                    corner.Y = temp.CenterPoint.Y - 3.5;
                    _singleThumb.Arrange(new Rect(corner, new Size(7, 7)));

                }
                else if (_adornedElementType.Name.Equals("Reticle"))
                {
                    Reticle temp = AdornedElement as Reticle;
                    Point corner = new Point();
                    corner.X = temp.CenterPoint.X - 7;
                    corner.Y = temp.CenterPoint.Y - 7;
                    _singleThumb.Arrange(new Rect(corner, new Size(14, 14)));
                }
                else if (_adornedElementType.Name.Equals("Polyline"))
                {
                    UpdatePolylineThumbPos();
                }
                else if (_adornedElementType.Name.Equals("ROIEllipse"))
                {
                    UpdateEllipseThumbsPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "ArrangeOverride exception " + ex.Message);
            }
            return finalSize;
        }

        protected override Visual GetVisualChild(int index)
        {
            return _visualChildren[index];
        }

        protected override void OnRender(DrawingContext drawingContext)
        {
        }

        private void BuildPanningdAdornerCorner(ref Thumb cornerThumb, Cursor customizedCursor)
        {
            if (cornerThumb != null) return;

            if (customizedCursor == Cursors.ScrollAll)
            {
                cornerThumb = new Thumb();
                cornerThumb.Cursor = customizedCursor;
                cornerThumb.Height = cornerThumb.Width = 7;
                cornerThumb.Background = new SolidColorBrush(Colors.Red);
                _visualChildren.Add(cornerThumb);
                return;
            }
            else if (customizedCursor == Cursors.Arrow)
            {
                cornerThumb = new Thumb();
                cornerThumb.Cursor = customizedCursor;
                cornerThumb.Height = cornerThumb.Width = 14;
                cornerThumb.Background = new SolidColorBrush(Colors.OrangeRed);
                _visualChildren.Add(cornerThumb);
                return;
            }
            cornerThumb = new Thumb();
            cornerThumb.Cursor = customizedCursor;
            cornerThumb.Height = cornerThumb.Width = 5;
            cornerThumb.Background = new SolidColorBrush(Colors.Black);
            _visualChildren.Add(cornerThumb);
        }

        private void BuildResizeAdornerCorner(ref Thumb cornerThumb, Cursor customizedCursor)
        {
            if (cornerThumb != null) return;

            cornerThumb = new Thumb();
            cornerThumb.Cursor = customizedCursor;
            cornerThumb.Height = cornerThumb.Width = 7;
            cornerThumb.Background = new SolidColorBrush(Colors.Blue);
            _visualChildren.Add(cornerThumb);
        }

        private void CrosshairPanningThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                ROICrosshair tempCrosshair = AdornedElement as ROICrosshair;
                double deltaX = e.HorizontalChange;
                double deltaY = e.VerticalChange;
                Point newPos = new Point();

                newPos.X = tempCrosshair.CenterPoint.X + deltaX;
                newPos.Y = tempCrosshair.CenterPoint.Y + deltaY;

                bool isOverlayValid = validatePoint(newPos);

                if (true == isOverlayValid)
                {
                    (AdornedElement as ROICrosshair).CenterPoint = newPos;

                    Point corner = new Point();
                    corner.X = newPos.X - 3.5;
                    corner.Y = newPos.Y - 3.5;

                    (sender as Thumb).Arrange(new Rect(corner, new Size(7, 7)));
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "CrosshairPanningThumb_DragDelta exception " + ex.Message);
            }
        }

        private void EllipsePanningThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                ROIEllipse tempEllipse = AdornedElement as ROIEllipse;
                double deltaX = e.HorizontalChange;
                double deltaY = e.VerticalChange;
                Point newPos = new Point();

                newPos.X = tempEllipse.Center.X + deltaX;
                newPos.Y = tempEllipse.Center.Y + deltaY;
                double width = (AdornedElement as ROIEllipse).ROIWidth;
                double height = (AdornedElement as ROIEllipse).ROIHeight;

                Point newStartPoint = new Point(tempEllipse.StartPoint.X + deltaX, tempEllipse.StartPoint.Y + deltaY); ;
                Point newEndPoint = new Point(tempEllipse.EndPoint.X + deltaX, tempEllipse.EndPoint.Y + deltaY);

                bool isOverlayValid = validateEllipse(newPos, width, height);

                if (true == isOverlayValid)
                {
                    (AdornedElement as ROIEllipse).StartPoint = newStartPoint;
                    (AdornedElement as ROIEllipse).EndPoint = newEndPoint;
                    (AdornedElement as ROIEllipse).Center = newPos;
                    UpdateEllipseThumbsPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "CrosshairPanningThumb_DragDelta exception " + ex.Message);
            }
        }

        void EllipseResizingThumbBottom_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                ROIEllipse tempEllipse = AdornedElement as ROIEllipse;
                double deltaY = e.VerticalChange;
                Point newCenter = new Point();

                newCenter.X = tempEllipse.Center.X;
                newCenter.Y = tempEllipse.Center.Y + deltaY / 2;
                Point newStartPoint;
                Point newEndPoint;

                if (tempEllipse.StartPoint.Y > tempEllipse.EndPoint.Y)
                {
                    newStartPoint = new Point(tempEllipse.StartPoint.X, tempEllipse.StartPoint.Y + deltaY);
                    newEndPoint = tempEllipse.EndPoint;
                }
                else
                {
                    newStartPoint = tempEllipse.StartPoint;
                    newEndPoint = new Point(tempEllipse.EndPoint.X, tempEllipse.EndPoint.Y + deltaY);
                }

                double width = (AdornedElement as ROIEllipse).ROIWidth;
                double newHeight = (AdornedElement as ROIEllipse).ROIHeight + deltaY;
                if (1 > newHeight)
                {
                    return;
                }
                bool isOverlayValid = validateEllipse(newCenter, width, newHeight);
                if (true == isOverlayValid)
                {
                    (AdornedElement as ROIEllipse).ShiftDown = false;
                    (AdornedElement as ROIEllipse).StartPoint = newStartPoint;
                    (AdornedElement as ROIEllipse).EndPoint = newEndPoint;
                    UpdateEllipseThumbsPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        void EllipseResizingThumbLeft_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                ROIEllipse tempEllipse = AdornedElement as ROIEllipse;
                double deltaX = e.HorizontalChange;
                Point newCenter = new Point();

                newCenter.X = tempEllipse.Center.X + deltaX / 2;
                newCenter.Y = tempEllipse.Center.Y;
                Point newStartPoint;
                Point newEndPoint;

                if (tempEllipse.StartPoint.X < tempEllipse.EndPoint.X)
                {
                    newStartPoint = new Point(tempEllipse.StartPoint.X + deltaX, tempEllipse.StartPoint.Y);
                    newEndPoint = tempEllipse.EndPoint;
                }
                else
                {
                    newStartPoint = tempEllipse.StartPoint;
                    newEndPoint = new Point(tempEllipse.EndPoint.X + deltaX, tempEllipse.EndPoint.Y);
                }

                double newWidth = (AdornedElement as ROIEllipse).ROIWidth - deltaX;
                double height = (AdornedElement as ROIEllipse).ROIHeight;
                if (1 > newWidth)
                {
                    return;
                }
                bool isOverlayValid = validateEllipse(newCenter, newWidth, height);
                if (true == isOverlayValid)
                {
                    (AdornedElement as ROIEllipse).ShiftDown = false;
                    (AdornedElement as ROIEllipse).StartPoint = newStartPoint;
                    (AdornedElement as ROIEllipse).EndPoint = newEndPoint;
                    UpdateEllipseThumbsPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        void EllipseResizingThumbRight_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                ROIEllipse tempEllipse = AdornedElement as ROIEllipse;
                double deltaX = e.HorizontalChange;
                Point newCenter = new Point();

                newCenter.X = tempEllipse.Center.X + deltaX / 2;
                newCenter.Y = tempEllipse.Center.Y;
                Point newStartPoint;
                Point newEndPoint;

                if (tempEllipse.StartPoint.X > tempEllipse.EndPoint.X)
                {
                    newStartPoint = new Point(tempEllipse.StartPoint.X + deltaX, tempEllipse.StartPoint.Y);
                    newEndPoint = tempEllipse.EndPoint;
                }
                else
                {
                    newStartPoint = tempEllipse.StartPoint;
                    newEndPoint = new Point(tempEllipse.EndPoint.X + deltaX, tempEllipse.EndPoint.Y);
                }

                double newWidth = (AdornedElement as ROIEllipse).ROIWidth + deltaX;
                double height = (AdornedElement as ROIEllipse).ROIHeight;
                if (1 > newWidth)
                {
                    return;
                }
                bool isOverlayValid = validateEllipse(newCenter, newWidth, height);
                if (true == isOverlayValid)
                {
                    (AdornedElement as ROIEllipse).ShiftDown = false;
                    (AdornedElement as ROIEllipse).StartPoint = newStartPoint;
                    (AdornedElement as ROIEllipse).EndPoint = newEndPoint;
                    UpdateEllipseThumbsPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        void EllipseResizingThumbTop_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                ROIEllipse tempEllipse = AdornedElement as ROIEllipse;
                double deltaX = e.HorizontalChange;
                double deltaY = e.VerticalChange;
                Point newCenter = new Point();

                newCenter.X = tempEllipse.Center.X;
                newCenter.Y = tempEllipse.Center.Y + deltaY / 2;
                Point newStartPoint;
                Point newEndPoint;

                if (tempEllipse.StartPoint.Y < tempEllipse.EndPoint.Y)
                {
                    newStartPoint = new Point(tempEllipse.StartPoint.X, tempEllipse.StartPoint.Y + deltaY);
                    newEndPoint = tempEllipse.EndPoint;
                }
                else
                {
                    newStartPoint = tempEllipse.StartPoint;
                    newEndPoint = new Point(tempEllipse.EndPoint.X, tempEllipse.EndPoint.Y + deltaY);
                }

                double width = (AdornedElement as ROIEllipse).ROIWidth;
                double newHeight = (AdornedElement as ROIEllipse).ROIHeight - deltaY;
                if (1 > newHeight)
                {
                    return;
                }
                bool isOverlayValid = validateEllipse(newCenter, width, newHeight);
                if (true == isOverlayValid)
                {
                    (AdornedElement as ROIEllipse).ShiftDown = false;
                    (AdornedElement as ROIEllipse).StartPoint = newStartPoint;
                    (AdornedElement as ROIEllipse).EndPoint = newEndPoint;
                    UpdateEllipseThumbsPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        private void EnforceSize(FrameworkElement adornedElement)
        {
            try
            {
                if (adornedElement.Width.Equals(Double.NaN))
                    adornedElement.Width = adornedElement.DesiredSize.Width;
                if (adornedElement.Height.Equals(Double.NaN))
                    adornedElement.Height = adornedElement.DesiredSize.Height;

                FrameworkElement parent = adornedElement.Parent as FrameworkElement;
                if (parent != null)
                {
                    adornedElement.MaxHeight = parent.ActualHeight;
                    adornedElement.MaxWidth = parent.ActualWidth;
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "EnforceSize exception " + ex.Message);
            }
        }

        private void LinePanningThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            double x2Change;
            double y2Change;
            double x1Change;
            double y1Change;

            try
            {
                x2Change = x1Change = e.HorizontalChange;
                y2Change = y1Change = e.VerticalChange;

                double newX1 = (AdornedElement as Line).X1 + x1Change;
                double newY1 = (AdornedElement as Line).Y1 + y1Change;
                double newX2 = (AdornedElement as Line).X2 + x2Change;
                double newY2 = (AdornedElement as Line).Y2 + y2Change;

                Point newX1Y1 = new Point(newX1, newY1);
                Point newX2Y2 = new Point(newX2, newY2);

                bool isX1Y1Valid = validatePoint(newX1Y1);
                bool isX2Y2Valid = validatePoint(newX2Y2);

                if (isX1Y1Valid && isX2Y2Valid)
                {
                    (AdornedElement as Line).X2 = newX2;
                    (AdornedElement as Line).Y2 = newY2;
                    (AdornedElement as Line).X1 = newX1;
                    (AdornedElement as Line).Y1 = newY1;
                    UpdateLineThumbPos();
                    int[] tag = new int[2];
                    int roiIndex = ((int[])(AdornedElement as Shape).Tag)[0];
                    if (null != UpdateLinePosition) UpdateLinePosition(roiIndex);
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "linePanningThumb_DragDelta exception " + ex.Message);
            }
        }

        private void LineResizeThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            double xChange;
            double yChange;

            try
            {
                Thumb tempThumb = sender as Thumb;

                xChange = xChange = e.HorizontalChange;
                yChange = yChange = e.VerticalChange;
                double newX;
                double newY;
                if (0 == (int)tempThumb.Tag)
                {
                    newX = (AdornedElement as Line).X1 + xChange;
                    newY = (AdornedElement as Line).Y1 + yChange;
                }
                else
                {
                    newX = (AdornedElement as Line).X2 + xChange;
                    newY = (AdornedElement as Line).Y2 + yChange;
                }

                Point newPoint = new Point(newX, newY);

                bool isPointValid = validatePoint(newPoint);

                if (isPointValid)
                {
                    if (0 == (int)tempThumb.Tag)
                    {
                        (AdornedElement as Line).X1 = newX;
                        (AdornedElement as Line).Y1 = newY;
                    }
                    else
                    {

                        (AdornedElement as Line).X2 = newX;
                        (AdornedElement as Line).Y2 = newY;
                    }
                    UpdateLineThumbPos();
                    int[] tag = new int[2];
                    int roiIndex = ((int[])(AdornedElement as Shape).Tag)[0];
                    if (null != UpdateLinePosition) UpdateLinePosition(roiIndex);
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "linePanningThumb_DragDelta exception " + ex.Message);
            }
        }

        private void PolylinePannigThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            double leftChange;
            double topChange;

            try
            {
                leftChange = e.HorizontalChange;
                topChange = e.VerticalChange;

                Polyline tempPoly = AdornedElement as Polyline;
                bool isOverlayValid = false;

                int[] verticesX = new int[tempPoly.Points.Count];
                int[] verticesY = new int[tempPoly.Points.Count];
                for (int i = 0; i < tempPoly.Points.Count; i++)
                {
                    verticesX[i] = Convert.ToInt32(tempPoly.Points[i].X + leftChange);
                    verticesY[i] = Convert.ToInt32(tempPoly.Points[i].Y + topChange);
                }

                int ROIminX = int.MaxValue, ROIminY = int.MaxValue, ROImaxX = int.MinValue, ROImaxY = int.MinValue;

                for (int i = 0; i < verticesX.Length; i++)
                {
                    if (verticesX[i] < ROIminX) ROIminX = verticesX[i];
                    if (verticesY[i] < ROIminY) ROIminY = verticesY[i];
                    if (verticesX[i] > ROImaxX) ROImaxX = verticesX[i];
                    if (verticesY[i] > ROImaxY) ROImaxY = verticesY[i];
                }
                isOverlayValid = ValidateOverlay(ROIminX, ROIminY, ROImaxX - ROIminX, ROImaxY - ROIminY);
                if (true == isOverlayValid)
                {
                    for (int i = 0; i < verticesX.Length; i++)
                    {
                        (AdornedElement as Polyline).Points[i] = new Point(verticesX[i], verticesY[i]);
                    }
                    UpdatePolylineThumbPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        private void PolylineReshapeThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                Polyline tempPoly = AdornedElement as Polyline;
                Thumb tempThumb = sender as Thumb;
                double deltaX = e.HorizontalChange;
                double deltaY = e.VerticalChange;
                Point newPos = new Point();

                newPos.X = tempPoly.Points[(int)tempThumb.Tag].X + deltaX;
                newPos.Y = tempPoly.Points[(int)tempThumb.Tag].Y + deltaY;

                bool isOverlayValid = validatePoint(newPos);

                if (true == isOverlayValid)
                {
                    (AdornedElement as Polyline).Points[(int)tempThumb.Tag] = newPos;

                    Point corner = new Point();
                    corner.X = newPos.X - 3.5;
                    corner.Y = newPos.Y - 3.5;

                    UpdatePolylineThumbPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        private void PolyPannigThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            double leftChange;
            double topChange;

            try
            {
                leftChange = e.HorizontalChange;
                topChange = e.VerticalChange;

                ROIPoly tempPoly = AdornedElement as ROIPoly;
                bool isOverlayValid = false;

                int[] verticesX = new int[tempPoly.Points.Count];
                int[] verticesY = new int[tempPoly.Points.Count];
                for (int i = 0; i < tempPoly.Points.Count; i++)
                {
                    verticesX[i] = Convert.ToInt32(tempPoly.Points[i].X + leftChange);
                    verticesY[i] = Convert.ToInt32(tempPoly.Points[i].Y + topChange);
                }

                int ROIminX = int.MaxValue, ROIminY = int.MaxValue, ROImaxX = int.MinValue, ROImaxY = int.MinValue;

                for (int i = 0; i < verticesX.Length; i++)
                {
                    if (verticesX[i] < ROIminX) ROIminX = verticesX[i];
                    if (verticesY[i] < ROIminY) ROIminY = verticesY[i];
                    if (verticesX[i] > ROImaxX) ROImaxX = verticesX[i];
                    if (verticesY[i] > ROImaxY) ROImaxY = verticesY[i];
                }
                isOverlayValid = ValidateOverlay(ROIminX, ROIminY, ROImaxX - ROIminX, ROImaxY - ROIminY);
                if (true == isOverlayValid)
                {
                    for (int i = 0; i < verticesX.Length; i++)
                    {
                        (AdornedElement as ROIPoly).Points[i] = new Point(verticesX[i], verticesY[i]);
                    }
                    UpdatePolyThumbPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        private void PolyReshapeThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                ROIPoly tempPoly = AdornedElement as ROIPoly;
                Thumb tempThumb = sender as Thumb;
                double deltaX = e.HorizontalChange;
                double deltaY = e.VerticalChange;
                Point newPos = new Point();

                newPos.X = tempPoly.Points[(int)tempThumb.Tag].X + deltaX;
                newPos.Y = tempPoly.Points[(int)tempThumb.Tag].Y + deltaY;

                bool isOverlayValid = validatePoint(newPos);

                if (true == isOverlayValid)
                {
                    (AdornedElement as ROIPoly).Points[(int)tempThumb.Tag] = newPos;

                    UpdatePolyThumbPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        private void RectPanningThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {

                double leftChange = e.HorizontalChange;
                double topChange = e.VerticalChange;
                ROIRect tempRect = AdornedElement as ROIRect;
                double width = tempRect.ROIWidth;
                double height = tempRect.ROIHeight;

                double newLeft = tempRect.TopLeft.X + leftChange;
                double newTop = tempRect.TopLeft.Y + topChange;

                bool isOverlayValid = ValidateOverlay(newLeft, newTop, width, height);

                if (isOverlayValid)
                {
                    double startX = tempRect.StartPoint.X + leftChange;
                    double startY = tempRect.StartPoint.Y + topChange;
                    double endX = tempRect.EndPoint.X + leftChange;
                    double endY = tempRect.EndPoint.Y + topChange;
                    (AdornedElement as ROIRect).ShiftDown = false;
                    (AdornedElement as ROIRect).StartPoint = new Point(startX, startY);
                    (AdornedElement as ROIRect).EndPoint = new Point(endX, endY);
                    UpdateRectThumbsPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        void RectResizingThumbBottom_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                ROIRect tempRect = AdornedElement as ROIRect;
                double deltaX = e.HorizontalChange;
                double deltaY = e.VerticalChange;

                double width = tempRect.ROIWidth;
                double newHeight = tempRect.ROIHeight + deltaY;

                bool isOverlayValid = ValidateOverlay(tempRect.TopLeft.X, tempRect.TopLeft.Y, width, newHeight);

                if (isOverlayValid)
                {
                    (AdornedElement as ROIRect).ShiftDown = false;
                    (AdornedElement as ROIRect).Rect = new Rect(tempRect.TopLeft.X, tempRect.TopLeft.Y, width, newHeight);
                    UpdateRectThumbsPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        void RectResizingThumbLeft_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                ROIRect tempRect = AdornedElement as ROIRect;
                double deltaX = e.HorizontalChange;
                double deltaY = e.VerticalChange;

                double newWidth = tempRect.ROIWidth - deltaX;
                double height = tempRect.ROIHeight;

                bool isOverlayValid = ValidateOverlay(tempRect.TopLeft.X + deltaX, tempRect.TopLeft.Y, newWidth, height);

                if (isOverlayValid)
                {
                    (AdornedElement as ROIRect).ShiftDown = false;
                    (AdornedElement as ROIRect).Rect = new Rect(tempRect.TopLeft.X + deltaX, tempRect.TopLeft.Y, newWidth, height);
                    UpdateRectThumbsPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        void RectResizingThumbRight_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                ROIRect tempRect = AdornedElement as ROIRect;
                double deltaX = e.HorizontalChange;
                double deltaY = e.VerticalChange;

                double newWidth = tempRect.ROIWidth + deltaX;
                double height = tempRect.ROIHeight;

                bool isOverlayValid = ValidateOverlay(tempRect.TopLeft.X, tempRect.TopLeft.Y, newWidth, height);

                if (isOverlayValid)
                {
                    (AdornedElement as ROIRect).ShiftDown = false;
                    (AdornedElement as ROIRect).Rect = new Rect(tempRect.TopLeft.X, tempRect.TopLeft.Y, newWidth, height);
                    UpdateRectThumbsPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        void RectResizingThumbTop_DragDelta(object sender, DragDeltaEventArgs e)
        {
            try
            {
                ROIRect tempRect = AdornedElement as ROIRect;
                double deltaX = e.HorizontalChange;
                double deltaY = e.VerticalChange;

                double width = tempRect.ROIWidth;
                double newHeight = tempRect.ROIHeight - deltaY;

                bool isOverlayValid = ValidateOverlay(tempRect.TopLeft.X, tempRect.TopLeft.Y + deltaY, width, newHeight);

                if (isOverlayValid)
                {
                    (AdornedElement as ROIRect).ShiftDown = false;
                    (AdornedElement as ROIRect).Rect = new Rect(tempRect.TopLeft.X, tempRect.TopLeft.Y + deltaY, width, newHeight);
                    UpdateRectThumbsPos();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        private void Thumb_DragComplete(object sender, DragCompletedEventArgs e)
        {
            if (null != UpdateNow)
            {
                UpdateNow(AdornedElement as Shape);
            }
        }

        private void UpdateEllipseThumbsPos()
        {
            ROIEllipse tempRect = AdornedElement as ROIEllipse;

            double desiredWidth = AdornedElement.DesiredSize.Width;
            double desiredHeight = AdornedElement.DesiredSize.Height;

            double width = (double)(AdornedElement as ROIEllipse).ROIWidth;
            double height = (double)(AdornedElement as ROIEllipse).ROIHeight;

            double adornerWidth = this.DesiredSize.Width;
            double adornerHeight = this.DesiredSize.Height;

            double topX = tempRect.TopLeft.X + width / 2 - adornerWidth / 2;
            double topY = tempRect.TopLeft.Y - adornerHeight / 2;

            double BottomX = tempRect.TopLeft.X + width / 2 - adornerWidth / 2;
            double BottomY = tempRect.TopLeft.Y + height - adornerHeight / 2;

            double leftX = tempRect.TopLeft.X - adornerWidth / 2;
            double leftY = tempRect.TopLeft.Y + height / 2 - adornerHeight / 2;

            double rightX = tempRect.TopLeft.X + width - adornerWidth / 2;
            double rightY = tempRect.TopLeft.Y + height / 2 - adornerHeight / 2;

            double centerX = leftX + (rightX - leftX) / 2;
            double centerY = topY + (BottomY - topY) / 2;

            _rectPanningThumb.Arrange(new Rect(centerX, centerY, adornerWidth, adornerHeight));

            _rectResizingThumbTop.Arrange(new Rect(topX, topY, adornerWidth, adornerHeight));
            _rectResizingThumbBottom.Arrange(new Rect(BottomX, BottomY, adornerWidth, adornerHeight));
            _rectResizingThumbLeft.Arrange(new Rect(leftX, leftY, adornerWidth, adornerHeight));
            _rectResizingThumbRight.Arrange(new Rect(rightX, rightY, adornerWidth, adornerHeight));
        }

        private void UpdateLineThumbPos()
        {
            Line temp = AdornedElement as Line;
            Point lineMidPoint;

            if (temp.X1 == temp.X2)
            {
                double midY = (temp.Y1 + temp.Y2) / 2 - 3.5;
                lineMidPoint = new Point(temp.X1 - 3.5, midY);
            }
            else if (temp.Y1 == temp.Y2)
            {
                double midX = (temp.X1 + temp.X2) / 2 - 3.5;
                lineMidPoint = new Point(midX, temp.Y1 - 3.5);
            }
            else
            {
                double midX = (temp.X1 + temp.X2) / 2 - 3.5;
                double midY = (temp.Y1 + temp.Y2) / 2 - 3.5;
                lineMidPoint = new Point(midX, midY);
            }

            _linePanningThumb.Arrange(new Rect(lineMidPoint, new Size(7, 7)));

            Point linePoint0 = new Point(temp.X1 - 3.5, temp.Y1 - 3.5);
            _lineResizingThumb[0].Arrange(new Rect(linePoint0, new Size(7, 7)));

            Point linePoint1 = new Point(temp.X2 - 3.5, temp.Y2 - 3.5);
            _lineResizingThumb[1].Arrange(new Rect(linePoint1, new Size(7, 7)));
        }

        private void UpdatePolylineThumbPos()
        {
            Polyline tempPolyline = AdornedElement as Polyline;

            Point corner = new Point();
            double ROIminX = int.MaxValue, ROIminY = int.MaxValue, ROImaxX = int.MinValue, ROImaxY = int.MinValue;
            for (int i = 0; i < tempPolyline.Points.Count; i++)
            {
                corner.X = tempPolyline.Points[i].X - 3.5;
                corner.Y = tempPolyline.Points[i].Y - 3.5;
                _polyReshapeThumb[i].Arrange(new Rect(corner, new Size(7, 7)));
                if (tempPolyline.Points[i].X < ROIminX)
                {
                    ROIminX = tempPolyline.Points[i].X;
                }
                if (tempPolyline.Points[i].Y < ROIminY)
                {
                    ROIminY = tempPolyline.Points[i].Y;
                }
                if (tempPolyline.Points[i].X > ROImaxX)
                {
                    ROImaxX = tempPolyline.Points[i].X;
                }
                if (tempPolyline.Points[i].Y > ROImaxY)
                {
                    ROImaxY = tempPolyline.Points[i].Y;
                }
            }
            double midX = ROIminX + (ROImaxX - ROIminX) / 2;
            double midY = ROIminY + (ROImaxY - ROIminY) / 2;

            _polyPanningThumb.Arrange(new Rect(new Point(midX, midY), new Size(7, 7)));
        }

        private void UpdatePolyThumbPos()
        {
            ROIPoly tempPoly = AdornedElement as ROIPoly;
            Point corner = new Point();
            for (int i = 0; i < tempPoly.Points.Count; i++)
            {
                corner.X = tempPoly.Points[i].X - 3.5;
                corner.Y = tempPoly.Points[i].Y - 3.5;
                _polyReshapeThumb[i].Arrange(new Rect(corner, new Size(7, 7)));
            }

            double midX = tempPoly.Bounds.Left + (tempPoly.Bounds.Right - tempPoly.Bounds.Left) / 2;
            double midY = tempPoly.Bounds.Top + (tempPoly.Bounds.Bottom - tempPoly.Bounds.Top) / 2;
            _polyPanningThumb.Arrange(new Rect(new Point(midX, midY), new Size(7, 7)));
        }

        private void UpdateRectThumbsPos()
        {
            ROIRect tempRect = AdornedElement as ROIRect;

            double desiredWidth = AdornedElement.DesiredSize.Width;
            double desiredHeight = AdornedElement.DesiredSize.Height;

            double width = (double)(AdornedElement as ROIRect).ROIWidth;
            double height = (double)(AdornedElement as ROIRect).ROIHeight;

            double adornerWidth = this.DesiredSize.Width;
            double adornerHeight = this.DesiredSize.Height;

            double topX = tempRect.TopLeft.X + width / 2 - adornerWidth / 2;
            double topY = tempRect.TopLeft.Y - adornerHeight / 2;

            double BottomX = tempRect.TopLeft.X + width / 2 - adornerWidth / 2;
            double BottomY = tempRect.TopLeft.Y + height - adornerHeight / 2;

            double leftX = tempRect.TopLeft.X - adornerWidth / 2;
            double leftY = tempRect.TopLeft.Y + height / 2 - adornerHeight / 2;

            double rightX = tempRect.TopLeft.X + width - adornerWidth / 2;
            double rightY = tempRect.TopLeft.Y + height / 2 - adornerHeight / 2;

            double centerX = leftX + (rightX - leftX) / 2;
            double centerY = topY + (BottomY - topY) / 2;

            _rectPanningThumb.Arrange(new Rect(centerX, centerY, adornerWidth, adornerHeight));

            _rectResizingThumbTop.Arrange(new Rect(topX, topY, adornerWidth, adornerHeight));
            _rectResizingThumbBottom.Arrange(new Rect(BottomX, BottomY, adornerWidth, adornerHeight));
            _rectResizingThumbLeft.Arrange(new Rect(leftX, leftY, adornerWidth, adornerHeight));
            _rectResizingThumbRight.Arrange(new Rect(rightX, rightY, adornerWidth, adornerHeight));
        }

        private bool validateEllipse(Point PointToValidate, double roiWidth, double roiHeight)
        {
            bool isValid = false;
            try
            {
                if ((PointToValidate.X - roiWidth / 2) > 0 &&
                    (PointToValidate.X + roiWidth / 2) < (0 + _imageWidth) &&
                    (PointToValidate.Y - roiHeight / 2) > 0 &&
                    (PointToValidate.Y + roiHeight / 2) < (0 + _imageHeight))
                    isValid = true;
                else
                {
                    isValid = false;
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "validateEllipse exception " + ex.Message);
                return false;
            }
            return isValid;
        }

        private bool ValidateOverlay(double overlayLeft, double overlayTop, double overlayWidth, double overlayHeight)
        {
            bool isValid = true;

            try
            {
                List<Point> overlayPoints = new List<Point>()
                {
                    new Point(overlayLeft,overlayTop),
                    new Point(overlayLeft + overlayWidth,overlayTop),
                    new Point(overlayLeft,overlayTop+overlayHeight),
                    new Point(overlayLeft + overlayWidth,overlayTop+overlayHeight)
                };

                foreach (Point point in overlayPoints)
                {
                    if (point.X > 0 && point.X < (0 + _imageWidth) && point.Y > 0 && point.Y < (0 + _imageHeight))
                        isValid = true;
                    else
                    {
                        isValid = false;
                        return isValid;
                    }
                }
                return isValid;
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "ValidateOverlay exception " + ex.Message);
                return false;
            }
        }

        private bool validatePoint(Point PointToValidate)
        {
            bool isValid = false;
            try
            {
                if (PointToValidate.X > 0 && PointToValidate.X < (0 + _imageWidth) && PointToValidate.Y > 0 && PointToValidate.Y < (0 + _imageHeight))
                    isValid = true;
                else
                {
                    isValid = false;
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "ValidatePoint exception " + ex.Message);
                return false;
            }
            return isValid;
        }

        #endregion Methods
    }
}