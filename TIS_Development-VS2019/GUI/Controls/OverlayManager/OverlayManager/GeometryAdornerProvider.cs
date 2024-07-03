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
    using ThorSharedTypes;

    using ThorLogging;

    public abstract class TextAdornerProvider : Adorner
    {
        #region Fields

        protected static int _imageHeight;
        protected static int _imageWidth;

        protected Type _adornedElementType;
        protected Brush _foreground;
        protected FormattedText _text;
        protected VisualCollection _visualChildren;

        #endregion Fields

        #region Constructors

        protected TextAdornerProvider(UIElement adornedElement, Brush foreground, int imageWidth, int imageHeight)
            : base(adornedElement)
        {
            _imageHeight = imageHeight;
            _imageWidth = imageWidth;
            _foreground = foreground;

            _adornedElementType = adornedElement.GetType();
            if (_adornedElementType.Name.Equals("Line") || _adornedElementType.Name.Equals("Polyline") || _adornedElementType.Name.Equals("ROILine") || _adornedElementType.Name.Equals("ROIPolyline"))
            {
                _visualChildren = new VisualCollection(this);
            }
        }

        #endregion Constructors

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

        public double FontSize
        {
            get { return (0 < _imageWidth) ? _imageWidth / 50.0 : 32 / 50.0; }
        }

        public Brush ForeGround
        {
            get { return _foreground; }
            set { _foreground = value; }
        }

        protected override int VisualChildrenCount
        {
            get
            {
                if (null != _visualChildren)
                {
                    return _visualChildren.Count;
                }
                return 0;
            }
        }

        #endregion Properties

        #region Methods

        protected override Size ArrangeOverride(Size finalSize)
        {
            return finalSize;
        }

        protected override Visual GetVisualChild(int index)
        {
            return _visualChildren[index];
        }

        #endregion Methods
    }

    class GeometryAdornerProvider : TextAdornerProvider
    {
        #region Fields

        private static PixelSizeUM _umPerPixel = new PixelSizeUM(1.0, 1.0);
        private static double _imageScaleX = 1.0;
        private static double _imageScaleY = 1.0;

        #endregion Fields

        #region Constructors

        public GeometryAdornerProvider(UIElement adornedElement, Brush foreground, int imageWidth, int imageHeight)
            : base(adornedElement, foreground, imageWidth, imageHeight)
        {        }
        
        public GeometryAdornerProvider(UIElement adornedElement, Brush foreground, int imageWidth, int imageHeight, PixelSizeUM pixelSizeUM)
            : base(adornedElement, foreground, imageWidth, imageHeight)
        {
            UMPerPixel = pixelSizeUM; 
        }

        #endregion Constructors

        #region Properties

        public static double ImageScaleX
        {
            get => _imageScaleX;
            set => _imageScaleX = value;
        }

        public static double ImageScaleY
        {
            get => _imageScaleY;
            set => _imageScaleY = value;
        }

        public static PixelSizeUM UMPerPixel
        {
            get { return _umPerPixel; }
            set { _umPerPixel = value; }
        }

        #endregion Properties

        #region Methods

        protected override Size ArrangeOverride(Size finalSize)
        {
            return finalSize;
        }

        protected override Visual GetVisualChild(int index)
        {
            return _visualChildren[index];
        }

        protected override void OnRender(DrawingContext drawingContext)
        {
            if (_adornedElementType.Name.Equals("Polyline"))
            {
                Polyline polyline = (this.AdornedElement as Polyline);
                if (2 > polyline.Points.Count)
                {
                    return;
                }
                int x1 = (int)Math.Floor(polyline.Points[0].X);
                int x2 = (int)Math.Floor(polyline.Points[1].X);
                int y1 = (int)Math.Floor(polyline.Points[0].Y);
                int y2 = (int)Math.Floor(polyline.Points[1].Y);

                int maxX = (x1 > x2) ? x1 : x2;
                int maxY = (y1 > y2) ? y1 : y2;
                int minX = (x1 < x2) ? x1 : x1;
                int minY = (y1 < y2) ? y1 : y2;

                Rect segmentBounds;
                if (y1 == y2)
                {
                    segmentBounds = new Rect(minX + (maxX - minX) / 2.0 - ImageWidth / 60.0, minY + (maxY - minY) + ImageWidth / 80.0, 2, 2);
                }
                else if (x1 == x2)
                {
                    double py = minY + (maxY - minY) / 2.0;
                    segmentBounds = new Rect(x1 + 1 + ImageWidth / 80.0, py, 2, 2);
                }
                else
                {
                    double py = minY + (maxY - minY) / 2.0;
                    double m = (double)(y1 - y2) / ((double)(x1 - x2));
                    double b = y1 - m * x1;
                    double px = (py - b) / m;
                    if (m < 0)
                    {
                        segmentBounds = new Rect(px + 1 + ImageWidth / 80.0, py, 2, 2);
                    }
                    else
                    {
                        segmentBounds = new Rect(px + 1 + ImageWidth / 80.0, py + ImageWidth / 80.0, 2, 2);
                    }
                }

                int polylineLength = 1;
                for (int i = 1; i < polyline.Points.Count; i++)
                {
                    x1 = (int)Math.Floor(polyline.Points[i - 1].X);
                    x2 = (int)Math.Floor(polyline.Points[i].X);
                    y1 = (int)Math.Floor(polyline.Points[i - 1].Y);
                    y2 = (int)Math.Floor(polyline.Points[i].Y);
                    polylineLength += (int)Math.Round(Math.Sqrt(Math.Pow((x2 - x1)/_imageScaleX, 2) + Math.Pow((y2 - y1)/_imageScaleY, 2)));
                }

                double lenth = _umPerPixel.PixelWidthUM * (double)polylineLength;
                _text = new FormattedText(
                    lenth.ToString("N3") + "um", System.Threading.Thread.CurrentThread.CurrentCulture,
                    System.Windows.FlowDirection.LeftToRight,
                    new Typeface("Arial"), FontSize, _foreground,
                    VisualTreeHelper.GetDpi(this).PixelsPerDip);

                drawingContext.DrawText(_text, segmentBounds.BottomRight);
            }
            else if (_adornedElementType.Name.Equals("ROIPolyline"))
            {
                ROIPolyline polyline = (this.AdornedElement as ROIPolyline);
                if (2 > polyline.Points.Count)
                {
                    return;
                }
                int x1 = (int)Math.Floor(polyline.Points[0].X);
                int x2 = (int)Math.Floor(polyline.Points[1].X);
                int y1 = (int)Math.Floor(polyline.Points[0].Y);
                int y2 = (int)Math.Floor(polyline.Points[1].Y);

                int maxX = (x1 > x2) ? x1 : x2;
                int maxY = (y1 > y2) ? y1 : y2;
                int minX = (x1 < x2) ? x1 : x1;
                int minY = (y1 < y2) ? y1 : y2;

                Rect segmentBounds;
                if (y1 == y2)
                {
                    segmentBounds = new Rect(minX + (maxX - minX) / 2.0 - ImageWidth / 60.0, minY + (maxY - minY) + ImageWidth / 80.0, 2, 2);
                }
                else if (x1 == x2)
                {
                    double py = minY + (maxY - minY) / 2.0;
                    segmentBounds = new Rect(x1 + 1 + ImageWidth / 80.0, py, 2, 2);
                }
                else
                {
                    double py = minY + (maxY - minY) / 2.0;
                    double m = (double)(y1 - y2) / ((double)(x1 - x2));
                    double b = y1 - m * x1;
                    double px = (py - b) / m;
                    if (m < 0)
                    {
                        segmentBounds = new Rect(px + 1 + ImageWidth / 80.0, py, 2, 2);
                    }
                    else
                    {
                        segmentBounds = new Rect(px + 1 + ImageWidth / 80.0, py + ImageWidth / 80.0, 2, 2);
                    }
                }

                int polylineLength = 1;
                for (int i = 1; i < polyline.Points.Count; i++)
                {
                    x1 = (int)Math.Floor(polyline.Points[i - 1].X);
                    x2 = (int)Math.Floor(polyline.Points[i].X);
                    y1 = (int)Math.Floor(polyline.Points[i - 1].Y);
                    y2 = (int)Math.Floor(polyline.Points[i].Y);
                    polylineLength += (int)Math.Round(Math.Sqrt(Math.Pow(_umPerPixel.PixelWidthUM*(x2 - x1) / _imageScaleX, 2) + Math.Pow(_umPerPixel.PixelHeightUM*(y2 - y1) / _imageScaleY, 2)));
                }

                //double lenth = _umPerPixel.PixelWidthUM * (double)polylineLength;
                _text = new FormattedText(
                    polylineLength.ToString("N3") + "um", System.Threading.Thread.CurrentThread.CurrentCulture,
                    System.Windows.FlowDirection.LeftToRight,
                    new Typeface("Arial"), FontSize, _foreground,
                    VisualTreeHelper.GetDpi(this).PixelsPerDip);

                drawingContext.DrawText(_text, segmentBounds.BottomRight);
            }
            else if (_adornedElementType.Name.Equals("Line"))
            {
                Line line = (this.AdornedElement as Line);

                int x1 = (int)Math.Floor(line.X1);
                int x2 = (int)Math.Floor(line.X2);
                int y1 = (int)Math.Floor(line.Y1);
                int y2 = (int)Math.Floor(line.Y2);

                int maxX = (x1 > x2) ? x1 : x2;
                int maxY = (y1 > y2) ? y1 : y2;
                int minX = (x1 < x2) ? x1 : x1;
                int minY = (y1 < y2) ? y1 : y2;

                Rect segmentBounds;
                if (y1 == y2)
                {
                    segmentBounds = new Rect(minX + (maxX - minX) / 2.0 - ImageWidth / 60.0, minY + (maxY - minY) + ImageWidth / 100.0, 2, 2);
                }
                else if (x1 == x2)
                {
                    double py = minY + (maxY - minY) / 2.0;
                    segmentBounds = new Rect(x1 + 1 + ImageWidth / 80.0, py, 2, 2);
                }
                else
                {
                    double m = (double)(y1 - y2) / ((double)(x1 - x2));
                    double b = y1 - m * x1;
                    double py = minY + (maxY - minY) / 2.0;
                    double px = (py - b) / m;
                    if (m < 0)
                    {
                        segmentBounds = new Rect(px + 1 + ImageWidth / 80.0, py, 2, 2);
                    }
                    else
                    {
                        segmentBounds = new Rect(px + 1 + ImageWidth / 80.0, py + ImageWidth / 80.0, 2, 2);
                    }
                }

                int lineLength = (int)Math.Round(Math.Sqrt(Math.Pow((x2 - x1) / _imageScaleX, 2) + Math.Pow((y2 - y1) / _imageScaleY, 2))) + 1;

                double lenth = _umPerPixel.PixelWidthUM * (double)lineLength;

                _text = new FormattedText(
                    lenth.ToString("N3") + "um", System.Threading.Thread.CurrentThread.CurrentCulture,
                    System.Windows.FlowDirection.LeftToRight,
                    new Typeface("Arial"), FontSize, _foreground,
                    VisualTreeHelper.GetDpi(this).PixelsPerDip);

                drawingContext.DrawText(_text, segmentBounds.BottomRight);
            }
            else if (_adornedElementType.Name.Equals("ROILine"))
            {
                ROILine line = (this.AdornedElement as ROILine);

                int x1 = (int)Math.Floor(line.StartPoint.X);
                int x2 = (int)Math.Floor(line.EndPoint.X);
                int y1 = (int)Math.Floor(line.StartPoint.Y);
                int y2 = (int)Math.Floor(line.EndPoint.Y);

                int maxX = (x1 > x2) ? x1 : x2;
                int maxY = (y1 > y2) ? y1 : y2;
                int minX = (x1 < x2) ? x1 : x1;
                int minY = (y1 < y2) ? y1 : y2;

                Rect segmentBounds;
                if (y1 == y2)
                {
                    segmentBounds = new Rect(minX + (maxX - minX) / 2.0 - ImageWidth / 60.0, minY + (maxY - minY) + ImageWidth / 100.0, 2, 2);
                }
                else if (x1 == x2)
                {
                    double py = minY + (maxY - minY) / 2.0;
                    segmentBounds = new Rect(x1 + 1 + ImageWidth / 80.0, py, 2, 2);
                }
                else
                {
                    double m = (double)(y1 - y2) / ((double)(x1 - x2));
                    double b = y1 - m * x1;
                    double py = minY + (maxY - minY) / 2.0;
                    double px = (py - b) / m;
                    if (m < 0)
                    {
                        segmentBounds = new Rect(px + 1 + ImageWidth / 80.0, py, 2, 2);
                    }
                    else
                    {
                        segmentBounds = new Rect(px + 1 + ImageWidth / 80.0, py + ImageWidth / 80.0, 2, 2);
                    }
                }

                int lineLength = (int)Math.Round(Math.Sqrt(Math.Pow(_umPerPixel.PixelWidthUM*(x2 - x1) / _imageScaleX, 2) + Math.Pow(_umPerPixel.PixelHeightUM*(y2 - y1) / _imageScaleY, 2))) + 1;

                //double lenth = _umPerPixel.PixelWidthUM * (double)lineLength;

                _text = new FormattedText(
                    lineLength.ToString("N3") + "um", System.Threading.Thread.CurrentThread.CurrentCulture,
                    System.Windows.FlowDirection.LeftToRight,
                    new Typeface("Arial"), FontSize, _foreground,
                    VisualTreeHelper.GetDpi(this).PixelsPerDip);

                drawingContext.DrawText(_text, segmentBounds.BottomRight);
            }
        }

        #endregion Methods
    }
}