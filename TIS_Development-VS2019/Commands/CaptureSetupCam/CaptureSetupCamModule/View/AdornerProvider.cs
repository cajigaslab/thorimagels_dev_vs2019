namespace CaptureSetupDll.ViewModel
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

    using CaptureSetupDll.View;

    using ThorLogging;

    public class AdornerProvider : Adorner
    {
        #region Fields

        Type adornedElementType;
        Thumb linePanningThumb; // For Line Overlay
        Thumb panningThumbTop, panningThumbBottom, panningThumbLeft, panningThumbRight; // For Rectangle & Text Overlays
        ROIStats ROIInfo;
        VisualCollection visualChildren;

        #endregion Fields

        #region Constructors

        public AdornerProvider(UIElement adornedElement)
            : base(adornedElement)
        {
            adornedElementType = adornedElement.GetType();

            if (adornedElementType.Name.Equals("TextBox") || adornedElementType.Name.Equals("Rectangle"))
            {
                visualChildren = new VisualCollection(this);

                BuildAdornerCorner(ref panningThumbTop, Cursors.ScrollAll);
                BuildAdornerCorner(ref panningThumbBottom, Cursors.ScrollAll);
                BuildAdornerCorner(ref panningThumbLeft, Cursors.ScrollAll);
                BuildAdornerCorner(ref panningThumbRight, Cursors.ScrollAll);

                panningThumbTop.DragDelta += new DragDeltaEventHandler(panningThumb_DragDelta);
                panningThumbBottom.DragDelta += new DragDeltaEventHandler(panningThumbBottom_DragDelta);
                panningThumbRight.DragDelta += new DragDeltaEventHandler(panningThumbRight_DragDelta);
                panningThumbLeft.DragDelta += new DragDeltaEventHandler(panningThumbLeft_DragDelta);

                if (adornedElementType.Name.Equals("Rectangle"))
                {
                    ROIInfo = ImageView.GetROIStatsWindow();
                }
               }
            else
            {
                visualChildren = new VisualCollection(this);
                BuildAdornerCorner(ref linePanningThumb, Cursors.ScrollAll);
                linePanningThumb.DragDelta += new DragDeltaEventHandler(linePanningThumb_DragDelta);
            }
        }

        #endregion Constructors

        #region Properties

        protected override int VisualChildrenCount
        {
            get { return visualChildren.Count; }
        }

        #endregion Properties

        #region Methods

        public bool ValidateOverlay(double overlayLeft, double overlayTop, double overlayWidth, double overlayHeight)
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
                    if (point.X > 0 && point.X < (0 + ImageView.ImageWidth) && point.Y > 0 && point.Y < (0 + ImageView.ImageHeight))
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

        protected override Size ArrangeOverride(Size finalSize)
        {
            try
            {
                if (adornedElementType.Name.Equals("TextBox"))
                {
                    double desiredWidth = AdornedElement.DesiredSize.Width;
                    double desiredHeight = AdornedElement.DesiredSize.Height;

                    double width = (double)(AdornedElement as TextBox).Width;
                    (AdornedElement as TextBox).Focus();

                    double adornerWidth = this.DesiredSize.Width;
                    double adornerHeight = this.DesiredSize.Height;

                    panningThumbTop.Arrange(new Rect(Math.Abs(-adornerWidth / 2) - (width / 2), -adornerHeight / 2, adornerWidth, adornerHeight));
                    panningThumbBottom.Arrange(new Rect(Math.Abs(-adornerWidth / 2) - (width / 2), desiredHeight - adornerHeight / 2, adornerWidth, adornerHeight));
                    panningThumbLeft.Arrange(new Rect(-adornerWidth / 2, (-adornerHeight / 2) + (AdornedElement as TextBox).Height/2, adornerWidth, adornerHeight));
                    panningThumbRight.Arrange(new Rect(desiredWidth - adornerWidth / 2,(-adornerHeight / 2) + (AdornedElement as TextBox).Height/2, adornerWidth, adornerHeight));
                }
                else if (adornedElementType.Name.Equals("Rectangle"))
                {
                    double desiredWidth = AdornedElement.DesiredSize.Width;
                    double desiredHeight = AdornedElement.DesiredSize.Height;

                    double width = (double)(AdornedElement as Rectangle).Width;
                    (AdornedElement as Rectangle).Focus();

                    double adornerWidth = this.DesiredSize.Width;
                    double adornerHeight = this.DesiredSize.Height;

                    panningThumbTop.Arrange(new Rect(Math.Abs(-adornerWidth / 2) - (width / 2), -adornerHeight / 2, adornerWidth, adornerHeight));
                    panningThumbBottom.Arrange(new Rect(Math.Abs(-adornerWidth / 2) - (width / 2), desiredHeight - adornerHeight / 2, adornerWidth, adornerHeight));
                    panningThumbLeft.Arrange(new Rect(-adornerWidth / 2, (-adornerHeight / 2) + (AdornedElement as Rectangle).Height / 2, adornerWidth, adornerHeight));
                    panningThumbRight.Arrange(new Rect(desiredWidth - adornerWidth / 2, (-adornerHeight / 2) + (AdornedElement as Rectangle).Height / 2, adornerWidth, adornerHeight));
                }
                else if (adornedElementType.Name.Equals("Line"))
                {
                    Line temp = AdornedElement as Line;
                    Point lineMidPoint;

                    if (temp.X1 == temp.X2)
                    {
                        double midY = (temp.Y1 + temp.Y2) / 2 - 2.5;
                        lineMidPoint = new Point(temp.X1 - 2.5, midY);
                    }
                    else if (temp.Y1 == temp.Y2)
                    {
                        double midX = (temp.X1 + temp.X2) / 2 - 2.5;
                        lineMidPoint = new Point(midX, temp.Y1 - 2.5);
                    }
                    else
                    {
                        double midX = (temp.X1 + temp.X2) / 2 - 2.5;
                        double midY = (temp.Y1 + temp.Y2) / 2 - 2.5;
                        lineMidPoint = new Point(midX, midY);
                    }

                    linePanningThumb.Arrange(new Rect(lineMidPoint, new Size(7, 7)));
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
            return visualChildren[index];
        }

        protected override void OnRender(DrawingContext drawingContext)
        {
        }

        void BuildAdornerCorner(ref Thumb cornerThumb, Cursor customizedCursor)
        {
            if (cornerThumb != null) return;

            if (customizedCursor == Cursors.ScrollAll)
            {
                cornerThumb = new Thumb();
                cornerThumb.Cursor = customizedCursor;
                cornerThumb.Height = cornerThumb.Width = 7;
                cornerThumb.Background = new SolidColorBrush(Colors.Red);
                visualChildren.Add(cornerThumb);
                return;
            }
            cornerThumb = new Thumb();
            cornerThumb.Cursor = customizedCursor;
            cornerThumb.Height = cornerThumb.Width = 5;
            cornerThumb.Background = new SolidColorBrush(Colors.Black);
            visualChildren.Add(cornerThumb);
        }

        void EnforceSize(FrameworkElement adornedElement)
        {
            try
            {
                if (adornedElement.Width.Equals(Double.NaN))
                    adornedElement.Width = adornedElement.DesiredSize.Width ;
                if (adornedElement.Height.Equals(Double.NaN))
                    adornedElement.Height = adornedElement.DesiredSize.Height ;

                FrameworkElement parent = adornedElement.Parent as FrameworkElement;
                if (parent != null)
                {
                    adornedElement.MaxHeight = parent.ActualHeight ;
                    adornedElement.MaxWidth = parent.ActualWidth ;
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "EnforceSize exception " + ex.Message);
            }
        }

        void linePanningThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            double x2Changed;
            double y2Changed;
            double x1Changed;
            double y1Changed;

            try
            {
                x2Changed = x1Changed = e.HorizontalChange;
                y2Changed = y1Changed = e.VerticalChange;

                double newX1 = (AdornedElement as Line).X1 + x1Changed;
                double newY1 = (AdornedElement as Line).Y1 + y1Changed;
                double newX2 = (AdornedElement as Line).X2 + x2Changed;
                double newY2 = (AdornedElement as Line).Y2 + y2Changed;

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

                    LineProfile.StartPointProperty = newX1Y1;
                    LineProfile.EndPointProperty = newX2Y2;

                    ImageView.updateStatsAfterResizingOrPanning();
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "linePanningThumb_DragDelta exception " + ex.Message);
            }
        }

        void panningThumbBottom_DragDelta(object sender, DragDeltaEventArgs e)
        {
            double leftChanged;
            double topChanged;

            try
            {
                leftChanged = e.HorizontalChange;
                topChanged = e.VerticalChange;

                double newLeft = Convert.ToDouble((AdornedElement as FrameworkElement).GetValue(Canvas.LeftProperty)) + leftChanged;
                double newTop = Convert.ToDouble((AdornedElement as FrameworkElement).GetValue(Canvas.TopProperty)) + topChanged;
                double width = Convert.ToDouble((AdornedElement as FrameworkElement).Width);
                double height = Convert.ToDouble((AdornedElement as FrameworkElement).Height);

                bool isOverlayValid = ValidateOverlay(newLeft, newTop, width, height);

                if (isOverlayValid)
                {
                    (AdornedElement as FrameworkElement).SetValue(Canvas.LeftProperty, newLeft);
                    (AdornedElement as FrameworkElement).SetValue(Canvas.TopProperty, newTop);
                    if (ROIInfo != null)
                    {
                        ROIInfo.ROILeft = Convert.ToInt32(newLeft);
                        ROIInfo.ROITop = Convert.ToInt32(newTop);
                        ImageView.updateStatsAfterResizingOrPanning();
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        void panningThumbLeft_DragDelta(object sender, DragDeltaEventArgs e)
        {
            double leftChanged;
            double topChanged;

            try
            {
                leftChanged = e.HorizontalChange;
                topChanged = e.VerticalChange;

                double newLeft = Convert.ToDouble((AdornedElement as FrameworkElement).GetValue(Canvas.LeftProperty)) + leftChanged;
                double newTop = Convert.ToDouble((AdornedElement as FrameworkElement).GetValue(Canvas.TopProperty)) + topChanged;
                double width = Convert.ToDouble((AdornedElement as FrameworkElement).Width);
                double height = Convert.ToDouble((AdornedElement as FrameworkElement).Height);

                bool isOverlayValid = ValidateOverlay(newLeft, newTop, width, height);

                if (isOverlayValid)
                {
                    (AdornedElement as FrameworkElement).SetValue(Canvas.LeftProperty, newLeft);
                    (AdornedElement as FrameworkElement).SetValue(Canvas.TopProperty, newTop);
                    if (ROIInfo != null)
                    {
                        ROIInfo.ROILeft = Convert.ToInt32(newLeft);
                        ROIInfo.ROITop = Convert.ToInt32(newTop);
                        ImageView.updateStatsAfterResizingOrPanning();
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        void panningThumbRight_DragDelta(object sender, DragDeltaEventArgs e)
        {
            double leftChanged;
            double topChanged;

            try
            {
                leftChanged = e.HorizontalChange;
                topChanged = e.VerticalChange;

                double newLeft = Convert.ToDouble((AdornedElement as FrameworkElement).GetValue(Canvas.LeftProperty)) + leftChanged;
                double newTop = Convert.ToDouble((AdornedElement as FrameworkElement).GetValue(Canvas.TopProperty)) + topChanged;
                double width = Convert.ToDouble((AdornedElement as FrameworkElement).Width);
                double height = Convert.ToDouble((AdornedElement as FrameworkElement).Height);

                bool isOverlayValid = ValidateOverlay(newLeft, newTop, width, height);

                if (isOverlayValid)
                {
                    (AdornedElement as FrameworkElement).SetValue(Canvas.LeftProperty, newLeft);
                    (AdornedElement as FrameworkElement).SetValue(Canvas.TopProperty, newTop);
                    if (ROIInfo != null)
                    {
                        ROIInfo.ROILeft = Convert.ToInt32(newLeft);
                        ROIInfo.ROITop = Convert.ToInt32(newTop);
                        ImageView.updateStatsAfterResizingOrPanning();
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        void panningThumb_DragDelta(object sender, DragDeltaEventArgs e)
        {
            double leftChanged;
            double topChanged;

            try
            {
                leftChanged = e.HorizontalChange;
                topChanged = e.VerticalChange;

                double newLeft = Convert.ToDouble((AdornedElement as FrameworkElement).GetValue(Canvas.LeftProperty)) + leftChanged;
                double newTop = Convert.ToDouble((AdornedElement as FrameworkElement).GetValue(Canvas.TopProperty)) + topChanged;
                double width = Convert.ToDouble((AdornedElement as FrameworkElement).Width);
                double height = Convert.ToDouble((AdornedElement as FrameworkElement).Height);

                bool isOverlayValid = ValidateOverlay(newLeft, newTop, width, height);

                if (isOverlayValid)
                {
                    (AdornedElement as FrameworkElement).SetValue(Canvas.LeftProperty, newLeft);
                    (AdornedElement as FrameworkElement).SetValue(Canvas.TopProperty, newTop);
                    if (ROIInfo != null)
                    {
                        ROIInfo.ROILeft = Convert.ToInt32(newLeft);
                        ROIInfo.ROITop = Convert.ToInt32(newTop);
                        ImageView.updateStatsAfterResizingOrPanning();
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "panningThumb_DragDelta exception " + ex.Message);
            }
        }

        private bool validatePoint(Point PointToValidate)
        {
            bool isValid = false;
            try
            {
                if (PointToValidate.X > 0 && PointToValidate.X < (0 + ImageView.ImageWidth) && PointToValidate.Y > 0 && PointToValidate.Y < (0 + ImageView.ImageHeight))
                    isValid = true;
                else
                {
                    isValid = false;
                    return isValid;
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