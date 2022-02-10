#region Header

// Part of the following code is inspired by the work of Omar G. Salem

#endregion Header

namespace GeometryUtilities
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Media;
    using System.Windows.Shapes;

    using OverlayManager;

    using ThorLogging;

    public class GeometryTypes
    {
        #region Fields

        public const double EPS = 0.000001;
        public const double RAD_DEGREE = Math.PI / 180;

        #endregion Fields

        #region Enumerations

        public enum EllipseType
        {
            Unknown,
            Circle,
            Ellipse
        }

        public enum PolygonDirection
        {
            Unknown,
            Clockwise,
            Count_Clockwise
        }

        public enum PolygonType
        {
            Unknown,
            Convex,
            Concave
        }

        public enum VertexType
        {
            Unknown,
            ConvexPoint,
            ConcavePoint
        }

        #endregion Enumerations
    }

    /// <summary>
    /// line: ax+by+c=0, with start point and end point, direction from start point ->end point
    /// </summary>
    public class LineSegment
    {
        #region Fields

        private Point _endPoint;
        private double _length;
        private Point _startPoint;
        private double _Vx;
        private double _Vy;

        #endregion Fields

        #region Constructors

        public LineSegment(Point startPoint, Point endPoint)
        {
            this._startPoint = startPoint;
            this._endPoint = endPoint;
            this._length = Math.Sqrt(Math.Pow((_endPoint.X - _startPoint.X), 2) + Math.Pow(((_endPoint.Y - _startPoint.Y)), 2));
            this._Vx = _endPoint.X - _startPoint.X;
            this._Vy = _endPoint.Y - _startPoint.Y;
        }

        #endregion Constructors

        #region Properties

        public Point EndPoint
        {
            get { return _endPoint; }
        }

        public double Length
        {
            get
            {
                return _length;
            }
        }

        public Point StartPoint
        {
            get { return _startPoint; }
        }

        public double Vx
        {
            get { return _Vx; }
        }

        public double Vy
        {
            get { return _Vy; }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// crossProduct = this X given_line.
        /// </summary>
        /// <param name="line"></param>
        /// <returns></returns>
        public double crossProductWithLine(LineSegment line)
        {
            return ((line.Vy * this.Vx) - (line.Vx * this.Vy));
        }

        public bool GetIntercept(LineSegment line, ref Point intercept)
        {
            double crossProduct = this.crossProductWithLine(line);
            if (Math.Abs(crossProduct) < GeometryTypes.EPS)
            {
                //parallel:
                return false;
            }
            else
            {
                double Ix = ((line.EndPoint.X - line.StartPoint.X) * (this.StartPoint.Y - line.StartPoint.Y) - (line.EndPoint.Y - line.StartPoint.Y) * (this.StartPoint.X - line.StartPoint.X)) / crossProduct;
                double Iy = ((this.EndPoint.X - this.StartPoint.X) * (this.StartPoint.Y - line.StartPoint.Y) - (this.EndPoint.Y - this.StartPoint.Y) * (this.StartPoint.X - line.StartPoint.X)) / crossProduct;
                if ((Ix > Math.Max(this.GetXMin(), line.GetXMin()))
                    && (Ix < Math.Min(this.GetXMax(), line.GetXMax()))
                    && (Iy > Math.Max(this.GetYMin(), line.GetYMin()))
                    && (Iy < Math.Min(this.GetYMax(), line.GetYMax()))
                    )
                {
                    intercept.X = Ix;
                    intercept.Y = Iy;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        public double GetXMax()
        {
            return Math.Max(_startPoint.X, _endPoint.X);
        }

        public double GetXMin()
        {
            return Math.Min(_startPoint.X, _endPoint.X);
        }

        public double GetYMax()
        {
            return Math.Max(_startPoint.Y, _endPoint.Y);
        }

        public double GetYMin()
        {
            return Math.Min(_startPoint.Y, _endPoint.Y);
        }

        public bool InLineSegment(Point pt)
        {
            bool ptInLine = false;

            double Vpt_x = pt.X - _startPoint.X;
            double Vpt_y = pt.Y - _startPoint.Y;
            double crossProduct = (this.Vx * Vpt_y) - (Vpt_x * this.Vy);
            if (Math.Abs(crossProduct) < GeometryTypes.EPS)
            {
                ptInLine = true;
            }
            return ptInLine;
        }

        public bool IsHorizontal()
        {
            if (this.Vy < GeometryTypes.EPS)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        public bool IsVertical()
        {
            if (this.Vx < GeometryTypes.EPS)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        #endregion Methods
    }

    public class Polygon
    {
        #region Fields

        private bool allowAddVertex = true;
        private List<LineSegment> _edges;
        private PointCollection _vertices;

        #endregion Fields

        #region Constructors

        public Polygon()
        {
            _vertices = new PointCollection();
            _edges = new List<LineSegment>();
        }

        public Polygon(PointCollection verticesIn)
        {
            _vertices = verticesIn;
            _edges = new List<LineSegment>();
            GenerateEdges();
        }

        #endregion Constructors

        #region Properties

        public List<LineSegment> Edges
        {
            get { return _edges; }
        }

        public PointCollection Vertices
        {
            get { return _vertices; }
        }

        public int VerticesCount
        {
            get { return _vertices.Count; }
        }

        #endregion Properties

        #region Methods

        public bool addVertex(Point pt)
        {
            if (!allowAddVertex)
            {
                return false;
            }
            if (_vertices.Count > 0)
            {
                LineSegment addLineSeg = new LineSegment(_vertices[_vertices.Count - 1], pt);
                _edges.Add(addLineSeg);
            }
            _vertices.Add(pt);
            return true;
        }

        public void finalizePolygon()
        {
            if (_vertices.Count > 2)
            {
                LineSegment addLineSeg = new LineSegment(_vertices[_vertices.Count - 1], _vertices[0]);
                _edges.Add(addLineSeg);
            }
            allowAddVertex = false;
        }

        public void ReverseVerticesDirection()
        {
            int vCnt = _vertices.Count;

            PointCollection newVertices = new PointCollection();
            for (int i = 0; i < vCnt; i++)
            {
                newVertices.Add(_vertices[i]);
            }
            for (int i = 0; i < vCnt; i++)
            {
                _vertices[i] = newVertices[vCnt - 1 - i];
            }
        }

        public GeometryTypes.PolygonDirection VerticesDirection()
        {
            int j = 0, crossProductCnt = 0;
            int vCount = _vertices.Count;
            if (_vertices.Count < 3)
                return GeometryTypes.PolygonDirection.Unknown;

            for (int i = 0; i < _vertices.Count; i++)
            {
                j = (i + 1) % _vertices.Count;
                double crossProduct = _edges[i].crossProductWithLine(_edges[j]);
                crossProductCnt = (crossProduct > 0) ? crossProductCnt++ : crossProductCnt--;
            }

            if (crossProductCnt < 0)
            {
                return GeometryTypes.PolygonDirection.Count_Clockwise;
            }
            else if (crossProductCnt > 0)
            {
                return GeometryTypes.PolygonDirection.Clockwise;
            }
            else
            {
                return GeometryTypes.PolygonDirection.Unknown;
            }
        }

        private void GenerateEdges()
        {
            if (_vertices.Count > 1)
            {
                _edges.Clear();
                for (int i = 0; i < _vertices.Count - 1; i++)
                {
                    LineSegment line = new LineSegment(_vertices[i], _vertices[i + 1]);
                    _edges.Add(line);
                }
            }
        }

        #endregion Methods
    }

    public static class ProcessBitmap
    {
        #region Fields

        private static string fname = string.Empty;
        private static int saveCount = 0;

        #endregion Fields

        #region Enumerations

        public enum MatrixType
        {
            Dilation,
            Erosion,
            EdgeDetection
        }

        public enum SaveShapeType
        {
            Fill,
            Center_Only
        }

        #endregion Enumerations

        #region Methods

        public static byte[] ApplyFilter(byte[] inBuffer, int width, int height, MatrixType type, int pixelSize)
        {
            int imageIdxOffset = 0, filterIdxOffset = 0;
            int blue = 0, green = 0, red = 0;               //Color order: BGR instead of RGB
            int stride = width * 4;
            int centerIdx = pixelSize * (((pixelSize * 2) + 1) + 1);
            byte[] outBuffer = new byte[stride * height];

            Matrix2D m2d = new Matrix2D((pixelSize * 2) + 1);
            switch (type)
            {
                case MatrixType.EdgeDetection:
                    m2d.SetType(MatrixType.EdgeDetection);
                    break;
            }
            int[] mtrx = m2d.Array;

            byte reset = 0;

            //iterate through image:
            for (int j = pixelSize; j < height - pixelSize; j++)
            {
                for (int i = pixelSize; i < width - pixelSize; i++)
                {
                    imageIdxOffset = j * stride + i * 4;
                    blue = green = red = reset;

                    //iterate through filter:
                    for (int filterIdY = -pixelSize; filterIdY <= pixelSize; filterIdY++)
                    {
                        for (int filterIdX = -pixelSize; filterIdX <= pixelSize; filterIdX++)
                        {
                            filterIdxOffset = imageIdxOffset + (filterIdY * stride) + (filterIdX * 4);
                            switch (type)
                            {
                                case MatrixType.EdgeDetection:
                                    blue += inBuffer[filterIdxOffset] * mtrx[filterIdY * (2 * pixelSize + 1) + filterIdX + centerIdx];
                                    green += inBuffer[filterIdxOffset + 1] * mtrx[filterIdY * (2 * pixelSize + 1) + filterIdX + centerIdx];
                                    red += inBuffer[filterIdxOffset + 2] * mtrx[filterIdY * (2 * pixelSize + 1) + filterIdX + centerIdx];
                                    break;
                            }
                        }
                    }
                    //threshold on binary image:
                    blue = (blue > 128 ? 255 : (blue <= 128 ? 0 : blue));
                    green = (green > 128 ? 255 : (green <= 128 ? 0 : green));
                    red = (red > 128 ? 255 : (red <= 128 ? 0 : red));
                    outBuffer[imageIdxOffset] = (byte)blue;
                    outBuffer[imageIdxOffset + 1] = (byte)green;
                    outBuffer[imageIdxOffset + 2] = (byte)red;
                    outBuffer[imageIdxOffset + 3] = (byte)255;
                }
            }
            return outBuffer;
        }

        /// <summary>
        /// return total number of non-zero pixel counts of indexed bitmap
        /// </summary>
        /// <param name="inputMap"></param>
        /// <returns></returns>
        public static int BinaryBitmapNonZeroCount(System.Drawing.Bitmap inputMap)
        {
            byte[] buffer = ProcessBitmap.LoadBinaryBitmap(inputMap);

            int imageIdxOffset = 0;
            byte white = 0xFF;               //Color order: BGR instead of RGB
            int stride = inputMap.Width * 4;
            int nonZeroCount = 0;
            for (int j = 0; j < inputMap.Height; j++)
            {
                for (int i = 0; i < inputMap.Width; i++)
                {
                    imageIdxOffset = j * stride + i * 4;
                    if ((white == buffer[imageIdxOffset]) && (white == buffer[imageIdxOffset + 1]) && (white == buffer[imageIdxOffset + 2]))
                    {
                        nonZeroCount++;
                    }
                }
            }
            return nonZeroCount;
        }

        /// <summary>
        /// Do morphologic processing on binary bitmap (filter bounary included), dilate, erode, with or without detecting edge.
        /// </summary>
        /// <param name="inputMap"></param>
        /// <param name="type"></param>
        /// <param name="pixelSize"></param>
        /// <returns></returns>
        public static byte[] BinaryFilter(byte[] inBuffer, int width, int height, MatrixType type, int pixelSize)
        {
            //int filterSize = pixelSize * 2 + 1;
            int imageIdxOffset = 0, filterIdxOffset = 0;
            int blue = 0, green = 0, red = 0;               //Color order: BGR instead of RGB
            int stride = width * 4;
            int centerIdx = pixelSize * (((pixelSize * 2) + 1) + 1);
            byte[] outBuffer = new byte[stride * height];

            Matrix2D m2d = new Matrix2D(3);
            m2d.SetType(MatrixType.EdgeDetection);
            int[] mtrx = m2d.Array;

            byte reset = 0;
            if (type == MatrixType.Erosion)
            {
                reset = (byte)255;
            }

            //iterate through image:
            for (int j = pixelSize; j < height - pixelSize; j++)
            {
                for (int i = pixelSize; i < width - pixelSize; i++)
                {
                    imageIdxOffset = j * stride + i * 4;
                    blue = green = red = reset;

                    //iterate through filter:
                    for (int filterIdY = -pixelSize; filterIdY <= pixelSize; filterIdY++)
                    {
                        for (int filterIdX = -pixelSize; filterIdX <= pixelSize; filterIdX++)
                        {
                            filterIdxOffset = imageIdxOffset + (filterIdY * stride) + (filterIdX * 4);
                            switch (type)
                            {
                                case MatrixType.Dilation:
                                    if (inBuffer[filterIdxOffset] > blue)
                                    {
                                        blue = inBuffer[filterIdxOffset];
                                    }
                                    if (inBuffer[filterIdxOffset + 1] > green)
                                    {
                                        green = inBuffer[filterIdxOffset + 1];
                                    }
                                    if (inBuffer[filterIdxOffset + 2] > red)
                                    {
                                        red = inBuffer[filterIdxOffset + 2];
                                    }
                                    break;
                                case MatrixType.Erosion:
                                    if (inBuffer[filterIdxOffset] < blue)
                                    {
                                        blue = inBuffer[filterIdxOffset];
                                    }
                                    if (inBuffer[filterIdxOffset + 1] < green)
                                    {
                                        green = inBuffer[filterIdxOffset + 1];
                                    }
                                    if (inBuffer[filterIdxOffset] < red)
                                    {
                                        red = inBuffer[filterIdxOffset + 2];
                                    }
                                    break;
                                case MatrixType.EdgeDetection:
                                    blue += inBuffer[filterIdxOffset] * mtrx[filterIdY * (2 * pixelSize + 1) + filterIdX + centerIdx];
                                    green += inBuffer[filterIdxOffset + 1] * mtrx[filterIdY * (2 * pixelSize + 1) + filterIdX + centerIdx];
                                    red += inBuffer[filterIdxOffset + 2] * mtrx[filterIdY * (2 * pixelSize + 1) + filterIdX + centerIdx];
                                    break;
                            }
                            if (type != MatrixType.EdgeDetection)
                            {
                                blue = (blue > 255 ? 255 : (blue < 0 ? 0 : blue));
                                green = (green > 255 ? 255 : (green < 0 ? 0 : green));
                                red = (red > 255 ? 255 : (red < 0 ? 0 : red));
                                outBuffer[imageIdxOffset] = (byte)blue;
                                outBuffer[imageIdxOffset + 1] = (byte)green;
                                outBuffer[imageIdxOffset + 2] = (byte)red;
                                outBuffer[imageIdxOffset + 3] = (byte)255;
                            }
                        }
                    }
                    if (type == MatrixType.EdgeDetection)
                    {
                        //threshold:
                        blue = (blue > 128 ? 255 : (blue <= 128 ? 0 : blue));
                        green = (green > 128 ? 255 : (green <= 128 ? 0 : green));
                        red = (red > 128 ? 255 : (red <= 128 ? 0 : red));
                        outBuffer[imageIdxOffset] = (byte)blue;
                        outBuffer[imageIdxOffset + 1] = (byte)green;
                        outBuffer[imageIdxOffset + 2] = (byte)red;
                        outBuffer[imageIdxOffset + 3] = (byte)255;
                    }
                }
            }
            return outBuffer;
        }

        /// <summary>
        /// Convert color bitmap to indexed bitmap
        /// </summary>
        /// <param name="bmp"></param>
        /// <returns></returns>
        public static System.Drawing.Bitmap ConvertBmpToGrayscale(System.Drawing.Bitmap bmp)
        {
            var result = new System.Drawing.Bitmap(bmp.Width, bmp.Height, System.Drawing.Imaging.PixelFormat.Format8bppIndexed);
            try
            {
                System.Drawing.Imaging.BitmapData data = result.LockBits(new System.Drawing.Rectangle(0, 0, result.Width, result.Height), System.Drawing.Imaging.ImageLockMode.WriteOnly, System.Drawing.Imaging.PixelFormat.Format8bppIndexed);
                byte[] bytes = new byte[data.Height * data.Stride];
                Marshal.Copy(data.Scan0, bytes, 0, bytes.Length);

                for (int y = 0; y < bmp.Height; y++)
                {
                    for (int x = 0; x < bmp.Width; x++)
                    {
                        var c = bmp.GetPixel(x, y);
                        var rgb = (byte)((c.R + c.G + c.B) / 3);
                        bytes[y * data.Stride + x] = rgb;
                    }
                }
                Marshal.Copy(bytes, 0, data.Scan0, bytes.Length);
                result.UnlockBits(data);
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "ConvertBmpToGrayscale: " + ex.Message);
            }
            return result;
        }

        /// <summary>
        /// Create a binary mask of 8bppIndexed bitmap based on provided shapes
        /// </summary>
        /// <param name="ptGroups"></param>
        /// <param name="imWidth"></param>
        /// <param name="imHeight"></param>
        /// <returns></returns>
        public static System.Drawing.Bitmap CreateBinaryBitmap(int imWidth, int imHeight, List<Point> ptGroups, List<int> ptValues = null)
        {
            System.Drawing.Bitmap bmp8bit = new System.Drawing.Bitmap(imWidth, imHeight, System.Drawing.Imaging.PixelFormat.Format8bppIndexed);
            System.Drawing.Imaging.BitmapData bmpData = bmp8bit.LockBits(new System.Drawing.Rectangle(0, 0, bmp8bit.Width, bmp8bit.Height), System.Drawing.Imaging.ImageLockMode.WriteOnly, System.Drawing.Imaging.PixelFormat.Format8bppIndexed);
            byte[] bytes = new byte[bmpData.Height * bmpData.Stride];
            System.Runtime.InteropServices.Marshal.Copy(bmpData.Scan0, bytes, 0, bytes.Length);
            if (null != ptGroups)
            {
                for (int i = 0; i < ptGroups.Count; i++)
                {
                    //opposite coordinates of our image:
                    bytes[(int)ptGroups[i].Y * bmpData.Stride + (int)ptGroups[i].X] = (null == ptValues) ? (byte)255 : (byte)(ptValues[i]);
                }
            }
            System.Runtime.InteropServices.Marshal.Copy(bytes, 0, bmpData.Scan0, bytes.Length);
            bmp8bit.UnlockBits(bmpData);
            return bmp8bit;
        }

        /// <summary>
        /// Create a binary mask of 8bppIndexed bitmap based on provided shapes
        /// </summary>
        /// <param name="imgSize"></param>
        /// <param name="shapeGroups"></param>
        /// <returns></returns>
        public static System.Drawing.Bitmap CreateBinaryBitmap(int[] imgSize, List<Shape> shapeGroups, SaveShapeType saveType = SaveShapeType.Fill, Point? offsetToCenterVec = null)
        {
            System.Drawing.Bitmap bitmap = new System.Drawing.Bitmap(imgSize[0], imgSize[1]);
            if (null == shapeGroups)
                return bitmap;

            try
            {
                foreach (Shape shape in shapeGroups)
                {
                    if (typeof(ROICrosshair) == shape.GetType())
                    {
                        bitmap.SetPixel(((int)((ROICrosshair)shape).CenterPoint.X), ((int)((ROICrosshair)shape).CenterPoint.Y), System.Drawing.Color.White);
                    }
                    else
                    {
                        double left = double.MaxValue, right = 0, top = double.MaxValue, bottom = 0;
                        using (System.Drawing.Graphics gr = System.Drawing.Graphics.FromImage(bitmap))
                        {
                            switch (saveType)
                            {
                                case SaveShapeType.Fill:
                                    shape.Dispatcher.Invoke(new Action(() =>
                                     {
                                         if (typeof(ROIEllipse) == shape.GetType())
                                         {
                                             gr.FillEllipse(new System.Drawing.SolidBrush(System.Drawing.Color.White),
                                                 new System.Drawing.RectangleF((float)((ROIEllipse)shape).TopLeft.X, (float)((ROIEllipse)shape).TopLeft.Y, (float)((ROIEllipse)shape).ROIWidth, (float)((ROIEllipse)shape).ROIHeight));
                                         }
                                         else if (typeof(ROIRect) == shape.GetType())
                                         {
                                             gr.FillRectangle(new System.Drawing.SolidBrush(System.Drawing.Color.White),
                                                 new System.Drawing.RectangleF((float)((ROIRect)shape).TopLeft.X, (float)((ROIRect)shape).TopLeft.Y, (float)((ROIRect)shape).ROIWidth, (float)((ROIRect)shape).ROIHeight));
                                         }
                                         else if (typeof(ROIPoly) == shape.GetType())
                                         {
                                             gr.FillPolygon(new System.Drawing.SolidBrush(System.Drawing.Color.White),
                                                 ((ROIPoly)shape).Points.Select(elm => new System.Drawing.Point((int)elm.X, (int)elm.Y)).ToArray());
                                         }
                                         else if (typeof(Line) == shape.GetType())
                                         {
                                             gr.DrawLine(new System.Drawing.Pen(new System.Drawing.SolidBrush(System.Drawing.Color.White), 1),
                                                 new System.Drawing.Point((int)((Line)shape).X1, (int)((Line)shape).Y1), new System.Drawing.Point((int)((Line)shape).X2, (int)((Line)shape).Y2));
                                         }
                                         else if (typeof(Polyline) == shape.GetType())
                                         {
                                             System.Drawing.Point[] dpts = new System.Drawing.Point[((Polyline)shape).Points.Count];
                                             for (int i = 0; i < ((Polyline)shape).Points.Count; i++)
                                                 dpts[i] = new System.Drawing.Point(Convert.ToInt32(((Polyline)shape).Points[i].X), Convert.ToInt32(((Polyline)shape).Points[i].Y));

                                             gr.DrawLines(new System.Drawing.Pen(new System.Drawing.SolidBrush(System.Drawing.Color.White), 1), dpts);
                                         }
                                     }));
                                    break;
                                case SaveShapeType.Center_Only:
                                    shape.Dispatcher.Invoke(new Action(() =>
                                    {
                                        if (typeof(ROIEllipse) == shape.GetType())
                                        {
                                            bitmap.SetPixel((int)(((ROIEllipse)shape).Center.X + ((null != offsetToCenterVec) ? ((Point)offsetToCenterVec).X : 0.0)), (int)(((ROIEllipse)shape).Center.Y + ((null != offsetToCenterVec) ? ((Point)offsetToCenterVec).Y : 0.0)), System.Drawing.Color.White);
                                        }
                                        else if (typeof(ROIRect) == shape.GetType())
                                        {
                                            bitmap.SetPixel((int)(((((ROIRect)shape).TopLeft.X + ((ROIRect)shape).BottomRight.X) / 2) + ((null != offsetToCenterVec) ? ((Point)offsetToCenterVec).X : 0.0)), (int)(((((ROIRect)shape).TopLeft.Y + ((ROIRect)shape).BottomRight.Y) / 2) + ((null != offsetToCenterVec) ? ((Point)offsetToCenterVec).Y : 0.0)), System.Drawing.Color.White);
                                        }
                                        else if (typeof(ROIPoly) == shape.GetType())
                                        {
                                            //[TODO] need a better method to get a polygon center.
                                            for (int i = 0; i < ((ROIPoly)shape).Points.Count; i++)
                                            {
                                                left = Math.Min(left, ((ROIPoly)shape).Points[i].X);
                                                right = Math.Max(right, ((ROIPoly)shape).Points[i].X);
                                                top = Math.Min(top, ((ROIPoly)shape).Points[i].Y);
                                                bottom = Math.Min(bottom, ((ROIPoly)shape).Points[i].Y);
                                            }
                                            bitmap.SetPixel((int)(((bottom + right) / 2) + ((null != offsetToCenterVec) ? ((Point)offsetToCenterVec).X : 0.0)), (int)(((top + bottom) / 2) + ((null != offsetToCenterVec) ? ((Point)offsetToCenterVec).Y : 0.0)), System.Drawing.Color.White);
                                        }
                                        else if (typeof(Line) == shape.GetType())
                                        {
                                            bitmap.SetPixel((int)(((((Line)shape).X1 + ((Line)shape).X2) / 2) + ((null != offsetToCenterVec) ? ((Point)offsetToCenterVec).X : 0.0)), (int)(((((Line)shape).Y1 + ((Line)shape).Y2) / 2) + ((null != offsetToCenterVec) ? ((Point)offsetToCenterVec).Y : 0.0)), System.Drawing.Color.White);
                                        }
                                        else if (typeof(Polyline) == shape.GetType())
                                        {
                                            //[TODO] need a better method to get a polyline center.
                                            for (int i = 0; i < ((Polyline)shape).Points.Count; i++)
                                            {
                                                left = Math.Min(left, ((Polyline)shape).Points[i].X);
                                                right = Math.Max(right, ((Polyline)shape).Points[i].X);
                                                top = Math.Min(top, ((Polyline)shape).Points[i].Y);
                                                bottom = Math.Min(bottom, ((Polyline)shape).Points[i].Y);
                                            }
                                            bitmap.SetPixel((int)(((bottom + right) / 2) + ((null != offsetToCenterVec) ? ((Point)offsetToCenterVec).X : 0.0)), (int)(((top + bottom) / 2) + ((null != offsetToCenterVec) ? ((Point)offsetToCenterVec).Y : 0.0)), System.Drawing.Color.White);
                                        }
                                    }));
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error, 1, "CreateBinaryBitmap: " + ex.Message);
            }
            return ConvertBmpToGrayscale(bitmap);
        }

        public static System.Drawing.Bitmap CreateBitmap(List<Point> ptGroups, bool fillByGraphic, bool offsetToZero, int extendPx, ref Point translate)
        {
            int minX = (int)ptGroups.Min(pt => pt.X);
            int minY = (int)ptGroups.Min(pt => pt.Y);
            int maxX = (int)ptGroups.Max(pt => pt.X);
            int maxY = (int)ptGroups.Max(pt => pt.Y);
            int imgWidth, imgHeight;

            if (offsetToZero)
            {
                translate.X = (minX < extendPx) ? 0 : (minX - extendPx);
                translate.Y = (minY < extendPx) ? 0 : (minY - extendPx);
                imgWidth = maxX - minX + 1 + 2 * extendPx;
                imgHeight = maxY - minY + 1 + 2 * extendPx;
            }
            else
            {
                translate.X = 0;
                translate.Y = 0;
                imgWidth = maxX + 2 * extendPx;
                imgHeight = maxY + 2 * extendPx;
            }
            System.Drawing.Bitmap bitmap = new System.Drawing.Bitmap(imgWidth, imgHeight);

            if (!fillByGraphic)
            {
                //fill all points:
                foreach (Point pt in ptGroups)
                {
                    bitmap.SetPixel((int)pt.X - (int)translate.X - 1, (int)pt.Y - (int)translate.Y - 1, System.Drawing.Color.White);
                }
            }
            else
            {
                //fill inside of the polygon (no edge):
                using (System.Drawing.Graphics g = System.Drawing.Graphics.FromImage(bitmap))
                {
                    g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
                    g.Clear(System.Drawing.Color.Black);
                    System.Drawing.Point[] ptArray = new System.Drawing.Point[ptGroups.Count];

                    for (int i = 0; i < ptGroups.Count; i++)
                    {
                        ptArray[i] = new System.Drawing.Point((int)ptGroups[i].X - (int)translate.X - 1, (int)ptGroups[i].Y - (int)translate.Y - 1);
                    }

                    g.FillPolygon(System.Drawing.Brushes.White, ptArray);
                }
            }

            return bitmap;
        }

        public static bool findClosestPoint(List<Point> inputPts, Point target, ref Point foundPt)
        {
            if (inputPts.Count == 0)
            {
                return false;
            }
            double distance = Math.Sqrt(Math.Pow(inputPts[0].X - target.X, 2) + Math.Pow(inputPts[0].Y - target.Y, 2));
            foundPt = inputPts[0];

            for (int i = 0; i < inputPts.Count; i++)
            {
                double tmp = Math.Sqrt(Math.Pow(inputPts[i].X - target.X, 2) + Math.Pow(inputPts[i].Y - target.Y, 2));
                if (tmp < distance)
                {
                    distance = tmp;
                    foundPt = inputPts[i];
                }
            }
            return true;
        }

        public static Dictionary<int, List<Point>> FindConnectedObjects(this System.Drawing.Bitmap inputMap)
        {
            int TagCount = 1;
            Dictionary<int, Tag> tagGroups = new Dictionary<int, Tag>();
            Dictionary<int, List<Point>> connectedObjs = new Dictionary<int, List<Point>>();
            List<int> neighborTags = new List<int>();
            int[,] lookUpTable = new int[inputMap.Width, inputMap.Height];

            for (int j = 0; j < inputMap.Height; j++)
            {
                for (int i = 0; i < inputMap.Width; i++)
                {
                    if ((inputMap.GetPixel(i, j).B == System.Drawing.Color.Empty.B) && (inputMap.GetPixel(i, j).G == System.Drawing.Color.Empty.G) && (inputMap.GetPixel(i, j).R == System.Drawing.Color.Empty.R))
                    {
                        //background:
                        continue;
                    }

                    //find neighbors:
                    int currentTag;
                    neighborTags.Clear();

                    for (int y = j - 1; y <= j + 2 && y < inputMap.Height - 1; y++)
                    {
                        for (int x = i - 1; x <= i + 2 && x < inputMap.Width - 1; x++)
                        {
                            if (y > -1 && x > -1 && lookUpTable[x, y] != 0)
                            {
                                neighborTags.Add(lookUpTable[x, y]);
                            }
                        }
                    }

                    if (!neighborTags.Any())
                    {
                        currentTag = TagCount;
                        tagGroups.Add(currentTag, new Tag(currentTag));
                        TagCount++;
                    }
                    else
                    {
                        currentTag = neighborTags.Min(x => tagGroups[x].GetHome().Name);
                        Tag tmpHome = tagGroups[currentTag].GetHome();

                        foreach (int nearTag in neighborTags)
                        {
                            if (tmpHome.Name != tagGroups[nearTag].GetHome().Name)
                            {
                                tagGroups[nearTag].Join(tagGroups[currentTag]);
                            }
                        }
                    }

                    lookUpTable[i, j] = currentTag;
                }
            }

            //Combine all connected:
            for (int j = 0; j < inputMap.Height; j++)
            {
                for (int i = 0; i < inputMap.Width; i++)
                {
                    int objID = lookUpTable[i, j];
                    if (objID != 0)
                    {
                        objID = tagGroups[objID].GetHome().Name;

                        if (!connectedObjs.ContainsKey(objID))
                        {
                            connectedObjs[objID] = new List<Point>();
                        }

                        connectedObjs[objID].Add(new Point(i, j));
                    }
                }
            }

            return connectedObjs;
        }

        public static byte[] LoadBinaryBitmap(System.Drawing.Bitmap inputMap)
        {
            System.Drawing.Imaging.BitmapData imageData = inputMap.LockBits(new System.Drawing.Rectangle(0, 0, inputMap.Width, inputMap.Height), System.Drawing.Imaging.ImageLockMode.ReadOnly, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

            byte[] inBuffer = new byte[imageData.Stride * imageData.Height];

            Marshal.Copy(imageData.Scan0, inBuffer, 0, inBuffer.Length);

            inputMap.UnlockBits(imageData);

            return inBuffer;
        }

        /// <summary>
        /// open objects on binary bitmap (filter bounary included).
        /// </summary>
        /// <param name="inputMap"></param>
        /// <returns></returns>
        public static System.Drawing.Bitmap OpenPolygonBinaryFilter(this System.Drawing.Bitmap inputMap)
        {
            byte[] imgBuffer = LoadBinaryBitmap(inputMap);
            imgBuffer = BinaryFilter(imgBuffer, inputMap.Width, inputMap.Height, MatrixType.Erosion, 1);
            imgBuffer = BinaryFilter(imgBuffer, inputMap.Width, inputMap.Height, MatrixType.Dilation, 1);
            return OutputBinaryBitmap(imgBuffer, inputMap.Width, inputMap.Height);
        }

        public static System.Drawing.Bitmap OutputBinaryBitmap(byte[] inBuffer, int width, int height)
        {
            //output bitmap:
            int length = width * height;
            System.Drawing.Bitmap outputMap = new System.Drawing.Bitmap(width, height);
            System.Drawing.Imaging.BitmapData outputData = outputMap.LockBits(new System.Drawing.Rectangle(0, 0, outputMap.Width, outputMap.Height), System.Drawing.Imaging.ImageLockMode.WriteOnly, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            Marshal.Copy(inBuffer, 0, outputData.Scan0, inBuffer.Length);
            outputMap.UnlockBits(outputData);

            return outputMap;
        }

        public static void SaveMap(this System.Drawing.Bitmap inputMap, string path)
        {
            fname = string.Format(path + "{0}.bmp", saveCount.ToString("000"));
            inputMap.Save(fname, System.Drawing.Imaging.ImageFormat.Bmp);
            saveCount++;
        }

        /// <summary>
        /// input bitmap has to be edge-detected and contains single object only.
        /// </summary>
        /// <param name="inputMap"></param>
        /// <param name="translate"></param>
        /// <returns></returns>
        public static List<Point> TraceConnectedBoundary(this System.Drawing.Bitmap inputMap, Point translate, int skipPixels)
        {
            int neighborOffset = 0, imageIdxOffset = 0;
            bool foundFirst = false, finish = false;

            List<Point> foundPoints = new List<Point>();
            List<Point> tracePoints = new List<Point>();
            List<Point> eightConnectedNeighbors = new List<Point>();

            Point home = new Point(-1, -1);
            Point trace = new Point(-1, -1);

            byte[] inBuffer = LoadBinaryBitmap(inputMap);
            int stride = inputMap.Width * 4;

            //Clockwise neighbors:
            Polygon polyNeighbor = new Polygon();
            polyNeighbor.addVertex(new Point(1, -1));
            polyNeighbor.addVertex(new Point(1, 0));
            polyNeighbor.addVertex(new Point(1, 1));
            polyNeighbor.addVertex(new Point(0, 1));
            polyNeighbor.addVertex(new Point(-1, 1));
            polyNeighbor.addVertex(new Point(-1, 0));
            polyNeighbor.addVertex(new Point(-1, -1));
            polyNeighbor.addVertex(new Point(0, -1));
            polyNeighbor.finalizePolygon();

            //find top-left home pixel:
            for (int j = 0; j < inputMap.Height; j++)
            {
                for (int i = 0; i < inputMap.Width; i++)
                {
                    imageIdxOffset = j * stride + i * 4;

                    //assume black and white image:
                    if ((0 != inBuffer[imageIdxOffset]) && (0 != inBuffer[imageIdxOffset + 1]) && (0 != inBuffer[imageIdxOffset + 2]))
                    {
                        if (!foundFirst)
                        {
                            home.X = i;
                            home.Y = j;
                            foundFirst = true;
                            break;
                        }
                    }
                }
                if (foundFirst)
                    break;
            }

            if (foundFirst)
            {
                while (home != trace)
                {
                    //initialize trace:
                    if ((trace.X == -1) || (trace.Y == -1))
                    {
                        tracePoints.Add(new Point(translate.X + home.X, translate.Y + home.Y));
                        trace = home;
                    }

                    foundFirst = false;

                    //clockwise locate non-zero neighbors:
                    eightConnectedNeighbors.Clear();
                    imageIdxOffset = (int)trace.Y * stride + (int)trace.X * 4;

                    foreach (Point pt in polyNeighbor.Vertices)
                    {
                        neighborOffset = imageIdxOffset + ((int)pt.Y * stride) + ((int)pt.X * 4);

                        if ((0 != inBuffer[neighborOffset]) && (0 != inBuffer[neighborOffset + 1]) && (0 != inBuffer[neighborOffset + 2]))
                        {
                            eightConnectedNeighbors.Add(new Point(pt.X, pt.Y));
                        }
                    }

                    //determine next:
                    if (!eightConnectedNeighbors.Any())
                    {
                        //isolated
                        break;
                    }
                    else
                    {
                        Point currentTrace = new Point(trace.X, trace.Y);
                        foreach (Point pt in eightConnectedNeighbors)
                        {
                            Point nextNeighbor = new Point(currentTrace.X + pt.X, currentTrace.Y + pt.Y);
                            Point target = new Point(translate.X + nextNeighbor.X, translate.Y + nextNeighbor.Y);

                            if (!foundFirst)
                            {
                                if (!tracePoints.Contains(target))
                                {
                                    tracePoints.Add(target);
                                    trace.X += (int)pt.X;
                                    trace.Y += (int)pt.Y;
                                    foundFirst = true;
                                }
                                else
                                {
                                    if ((nextNeighbor == home) && (tracePoints.Count > 6))
                                    {
                                        //finish trace
                                        trace = home;
                                        finish = true;
                                        break;
                                    }
                                }
                            }
                            else if (!tracePoints.Contains(target) && (!foundPoints.Contains(nextNeighbor)))
                            {
                                //store other found neighbors:
                                foundPoints.Add(nextNeighbor);
                            }
                        }
                    }

                    if (finish)
                        break;

                    if (!foundFirst)
                    {
                        //trace broken, find nearest:
                        Point closest = new Point();
                        while ((foundPoints.Count > 0) && (!foundFirst))
                        {
                            if (findClosestPoint(foundPoints, trace, ref closest))
                            {
                                Point target = new Point(translate.X + closest.X, translate.Y + closest.Y);
                                if (!tracePoints.Contains(target))
                                {
                                    tracePoints.Add(target);
                                    trace = closest;
                                    foundPoints.RemoveAt(foundPoints.IndexOf(closest));
                                    foundFirst = true;
                                }
                                else
                                {
                                    foundPoints.RemoveAt(foundPoints.IndexOf(closest));
                                }
                            }
                            else
                                break;
                        }
                    }
                    if (!foundFirst)
                        break;
                }
            }

            //skip pixels:
            List<Point> outPoints = new List<Point>();
            for (int px = 0; px < tracePoints.Count; px += skipPixels)
            {
                outPoints.Add(tracePoints[px]);
            }
            return outPoints;
        }

        #endregion Methods

        #region Nested Types

        public class Matrix2D
        {
            #region Fields

            private int size = 0;
            private int[] _matrixArray;

            #endregion Fields

            #region Constructors

            public Matrix2D(int side)
            {
                size = (int)Math.Pow(side, 2);
                _matrixArray = new int[size];
            }

            #endregion Constructors

            #region Properties

            public int[] Array
            {
                get
                {
                    return _matrixArray;
                }
            }

            #endregion Properties

            #region Methods

            public int[] Indexed(int initial)
            {
                int[] index = new int[size];
                for (int i = 0; i < size; i++)
                {
                    index[i] = initial;
                }
                return index;
            }

            public void SetType(MatrixType type)
            {
                int side = (int)Math.Sqrt(size);
                int step = (side - 1) / 2;
                int centerID = step * (side + 1);
                int centerValue = 4 * step;
                switch (type)
                {
                    case MatrixType.EdgeDetection:
                        _matrixArray = this.Indexed(0);
                        for (int j = -step; j <= step; j++)
                        {
                            for (int i = -step; i <= step; i++)
                            {
                                if ((i == 0) || (j == 0))
                                {
                                    if ((i == 0) && (j == 0))
                                        _matrixArray[centerID] = centerValue;
                                    else
                                        _matrixArray[j * side + i + centerID] = -1;
                                }
                                else
                                {
                                    _matrixArray[j * side + i + centerID] = 0;
                                }
                            }
                        }
                        break;
                }
            }

            #endregion Methods
        }

        internal class Tag
        {
            #region Constructors

            public Tag(int Name)
            {
                this.Name = Name;
                this.Home = this;
                this.Rank = 0;
            }

            #endregion Constructors

            #region Properties

            public Tag Home
            {
                get;
                set;
            }

            public int Name
            {
                get;
                set;
            }

            public int Rank
            {
                get;
                set;
            }

            #endregion Properties

            #region Methods

            internal Tag GetHome()
            {
                if (this.Home != this)
                {
                    this.Home = this.Home.GetHome();
                }

                return this.Home;
            }

            internal void Join(Tag guest)
            {
                if (guest.Rank < this.Rank)
                {
                    guest.Home = this;
                }
                else
                {
                    this.Home = guest;
                    if (this.Rank == guest.Rank)
                    {
                        guest.Rank++;
                    }
                }
            }

            #endregion Methods
        }

        #endregion Nested Types
    }
}