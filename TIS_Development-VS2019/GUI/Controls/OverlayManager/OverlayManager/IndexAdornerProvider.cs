namespace OverlayManager
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Shapes;

    public class IndexAdornerProvider : TextAdornerProvider
    {
        #region Constructors

        public IndexAdornerProvider(UIElement adornedElement, Brush foreground, int imageWidth, int imageHeight)
            : base(adornedElement, foreground, imageWidth, imageHeight)
        {
        }

        #endregion Constructors

        #region Properties

        public int Index
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        protected override void OnRender(DrawingContext drawingContext)
        {
            double squareSize = ImageWidth / 40.0;

            if (_adornedElementType.Name.Equals("ROICrosshair"))
            {
                ROICrosshair crosshair = (this.AdornedElement as ROICrosshair);
                double boundSize = (512 > ImageWidth) ? (2 * squareSize) : squareSize;
                Rect segmentBounds = new Rect(Math.Max(crosshair.CenterPoint.X - boundSize, 0), Math.Max(crosshair.CenterPoint.Y - boundSize, 0), squareSize, squareSize);
                _text = new FormattedText(
                            Index.ToString(), System.Threading.Thread.CurrentThread.CurrentCulture,
                            System.Windows.FlowDirection.LeftToRight,
                            new Typeface("Arial"), FontSize, _foreground,
                            VisualTreeHelper.GetDpi(this).PixelsPerDip);

                drawingContext.DrawText(_text, segmentBounds.TopLeft);
            }
            else if (_adornedElementType.Name.Equals("ROIRect"))
            {
                ROIRect rect = (this.AdornedElement as ROIRect);
                Rect segmentBounds = new Rect(Math.Max(rect.TopLeft.X - squareSize, 0), Math.Max(rect.TopLeft.Y - squareSize, 0), squareSize, squareSize);
                _text = new FormattedText(
                            Index.ToString(), System.Threading.Thread.CurrentThread.CurrentCulture,
                            System.Windows.FlowDirection.LeftToRight,
                            new Typeface("Arial"), FontSize, _foreground,
                            VisualTreeHelper.GetDpi(this).PixelsPerDip);

                drawingContext.DrawText(_text, segmentBounds.TopLeft);
            }
            else if (_adornedElementType.Name.Equals("ROIPoly"))
            {
                ROIPoly poly = (this.AdornedElement as ROIPoly);
                Point pt = poly.Points.GroupBy(p => p.X).OrderBy(p => p.Key).ToList().First().FirstOrDefault();
                Rect segmentBounds = new Rect(Math.Max(pt.X - squareSize, 0), Math.Max(pt.Y - squareSize, 0), squareSize, squareSize);
                _text = new FormattedText(
                            Index.ToString(), System.Threading.Thread.CurrentThread.CurrentCulture,
                            System.Windows.FlowDirection.LeftToRight,
                            new Typeface("Arial"), FontSize, _foreground,
                            VisualTreeHelper.GetDpi(this).PixelsPerDip);

                drawingContext.DrawText(_text, segmentBounds.TopLeft);
            }
            else if (_adornedElementType.Name.Equals("ROIEllipse"))
            {
                ROIEllipse ellipse = (this.AdornedElement as ROIEllipse);
                Point pt = new Point(ellipse.Center.X - (ellipse.ROIWidth / 2 / Math.Sqrt(2)), ellipse.Center.Y - (ellipse.ROIHeight / 2 / Math.Sqrt(2)));
                Rect segmentBounds = new Rect(Math.Max(pt.X - squareSize, 0), Math.Max(pt.Y - squareSize, 0), squareSize / 2, squareSize / 2);
                _text = new FormattedText(
                            Index.ToString(), System.Threading.Thread.CurrentThread.CurrentCulture,
                            System.Windows.FlowDirection.LeftToRight,
                            new Typeface("Arial"), FontSize, _foreground,
                            VisualTreeHelper.GetDpi(this).PixelsPerDip);

                drawingContext.DrawText(_text, segmentBounds.TopLeft);
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
                    segmentBounds = new Rect(minX + (maxX - minX) / 2.0 - ImageWidth / 60.0, minY - (maxY - minY) - ImageWidth / 100.0, 2, 2);
                }
                else if (x1 == x2)
                {
                    double py = minY + (maxY - minY) / 2.0;
                    segmentBounds = new Rect(x1 - 1 - ImageWidth / 80.0, py, 2, 2);
                }
                else
                {
                    double m = (double)(y1 - y2) / ((double)(x1 - x2));
                    double b = y1 - m * x1;
                    double py = minY + (maxY - minY) / 2.0;
                    double px = (py - b) / m;
                    if (m < 0)
                    {
                        segmentBounds = new Rect(px - 1 - ImageWidth / 80.0, py + ImageWidth / 80.0, 2, 2);
                    }
                    else
                    {
                        segmentBounds = new Rect(px - 1 - ImageWidth / 80.0, py - ImageWidth / 80.0, 2, 2);
                    }
                }

                _text = new FormattedText(
                    Index.ToString(), System.Threading.Thread.CurrentThread.CurrentCulture,
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
                    segmentBounds = new Rect(minX + (maxX - minX) / 2.0 - ImageWidth / 60.0, minY - (maxY - minY) - ImageWidth / 100.0, 2, 2);
                }
                else if (x1 == x2)
                {
                    double py = minY + (maxY - minY) / 2.0;
                    segmentBounds = new Rect(x1 - 1 - ImageWidth / 80.0, py, 2, 2);
                }
                else
                {
                    double m = (double)(y1 - y2) / ((double)(x1 - x2));
                    double b = y1 - m * x1;
                    double py = minY + (maxY - minY) / 2.0;
                    double px = (py - b) / m;
                    if (m < 0)
                    {
                        segmentBounds = new Rect(px - 1 - ImageWidth / 80.0, py + ImageWidth / 80.0, 2, 2);
                    }
                    else
                    {
                        segmentBounds = new Rect(px - 1 - ImageWidth / 80.0, py - ImageWidth / 80.0, 2, 2);
                    }
                }

                _text = new FormattedText(
                    Index.ToString(), System.Threading.Thread.CurrentThread.CurrentCulture,
                    System.Windows.FlowDirection.LeftToRight,
                    new Typeface("Arial"), FontSize, _foreground,
                    VisualTreeHelper.GetDpi(this).PixelsPerDip);

                drawingContext.DrawText(_text, segmentBounds.BottomRight);
            }
            else if (_adornedElementType.Name.Equals("Polyline"))
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
                    segmentBounds = new Rect(minX + (maxX - minX) / 2.0 - ImageWidth / 60.0, minY - (maxY - minY) - ImageWidth / 80.0, 2, 2);
                }
                else if (x1 == x2)
                {
                    double py = minY + (maxY - minY) / 2.0;
                    segmentBounds = new Rect(x1 - 1 - ImageWidth / 80.0, py, 2, 2);
                }
                else
                {
                    double py = minY + (maxY - minY) / 2.0;
                    double m = (double)(y1 - y2) / ((double)(x1 - x2));
                    double b = y1 - m * x1;
                    double px = (py - b) / m;
                    if (m < 0)
                    {
                        segmentBounds = new Rect(px - 1 - ImageWidth / 80.0, py + ImageWidth / 80.0, 2, 2);
                    }
                    else
                    {
                        segmentBounds = new Rect(px - 1 - ImageWidth / 80.0, py - ImageWidth / 80.0, 2, 2);
                    }
                }

                _text = new FormattedText(
                    Index.ToString(), System.Threading.Thread.CurrentThread.CurrentCulture,
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
                    segmentBounds = new Rect(minX + (maxX - minX) / 2.0 - ImageWidth / 60.0, minY - (maxY - minY) - ImageWidth / 80.0, 2, 2);
                }
                else if (x1 == x2)
                {
                    double py = minY + (maxY - minY) / 2.0;
                    segmentBounds = new Rect(x1 - 1 - ImageWidth / 80.0, py, 2, 2);
                }
                else
                {
                    double py = minY + (maxY - minY) / 2.0;
                    double m = (double)(y1 - y2) / ((double)(x1 - x2));
                    double b = y1 - m * x1;
                    double px = (py - b) / m;
                    if (m < 0)
                    {
                        segmentBounds = new Rect(px - 1 - ImageWidth / 80.0, py + ImageWidth / 80.0, 2, 2);
                    }
                    else
                    {
                        segmentBounds = new Rect(px - 1 - ImageWidth / 80.0, py - ImageWidth / 80.0, 2, 2);
                    }
                }

                _text = new FormattedText(
                    Index.ToString(), System.Threading.Thread.CurrentThread.CurrentCulture,
                    System.Windows.FlowDirection.LeftToRight,
                    new Typeface("Arial"), FontSize, _foreground,
                    VisualTreeHelper.GetDpi(this).PixelsPerDip);

                drawingContext.DrawText(_text, segmentBounds.BottomRight);
            }
        }

        #endregion Methods
    }
}