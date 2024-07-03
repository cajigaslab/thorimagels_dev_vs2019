namespace OverlayManager
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using System.Xml;

    public class ROIEllipse : ScalableShape
    {
        #region Fields

        private Point _bottomLeft;
        private Point _bottomRight;
        private EllipseGeometry _ellipse;
        private Point _endPoint;
        private Point _roiCenter;
        private double _roiHeight;
        private double _roiWidth;
        private bool _shiftDown;
        private Point _startPoint;
        private Point _topLeft;
        private Point _topRight;

        private Point _statsStartPoint;
        private Point _statsEndPoint;
        private Point _statsCenterPoint;
        private double _imageScaleX;
        private double _imageScaleY;
        private double _toAdjustX;
        private double _toAdjustY;

        #endregion Fields

        #region Constructors

        // constructor
        public ROIEllipse()
        {
            _startPoint = new Point(0, 0);
            _endPoint = new Point(0, 0);
            _topLeft = new Point(0, 0);
            _roiCenter = new Point();
            _ellipse = new EllipseGeometry();
            _roiHeight = 1;
            _roiWidth = 1;
            _shiftDown = false;

            _statsStartPoint = new Point(0, 0);
            _statsEndPoint = new Point(0, 0);
            _imageScaleX = 1.0;
            _imageScaleY = 1.0;
            _toAdjustX = 1.0;
            _toAdjustY = 1.0;

            RedrawShape();
        }

        public ROIEllipse(Point start, Point end, double xScale, double yScale)
        {
            _startPoint = start;
            _endPoint = end;
            _topLeft = new Point(0, 0);
            _roiCenter = new Point();
            _statsCenterPoint = new Point();
            _ellipse = new EllipseGeometry();
            _roiHeight = 1;
            _roiWidth = 1;
            _shiftDown = false;

            _statsStartPoint = new Point(0, 0);
            _statsEndPoint = new Point(0, 0);
            _imageScaleX = xScale;
            _imageScaleY = yScale;
            _toAdjustX = 1.0 / xScale;
            _toAdjustY = 1.0 / yScale;

            RedrawShape();
        }

        public ROIEllipse(ROIEllipse sample)
        {
            _startPoint = sample.StartPoint;
            _endPoint = sample.EndPoint;
            _topLeft = new Point(0, 0);
            _roiCenter = new Point();
            _statsCenterPoint = new Point();
            _ellipse = new EllipseGeometry();
            _roiHeight = 1;
            _roiWidth = 1;
            _shiftDown = false;

            _statsStartPoint = sample.StartPoint;
            _statsEndPoint = sample.EndPoint;
            _imageScaleX = 1.0;
            _imageScaleY = 1.0;
            _toAdjustX = 1.0;
            _toAdjustY = 1.0;

            RedrawShape();
        }

        #endregion Constructors

        #region Properties

        public Point BottomLeft
        {
            get { return _bottomLeft; }
        }

        public Point BottomRight
        {
            get { return _bottomRight; }
        }

        public Rect Bounds
        {
            get { return _ellipse.Bounds; }
        }

        public Rect StatsBounds
        {
            get
            {
                return new Rect(_statsStartPoint, _statsEndPoint);
            }
        }

        public Point Center
        {
            get { return _ellipse.Center; }
            set
            {
                _ellipse.Center = value;
                _roiCenter = value;
                _topLeft = _ellipse.Bounds.TopLeft;
                _topRight = _ellipse.Bounds.TopRight;
                _bottomLeft = _ellipse.Bounds.BottomLeft;
                _bottomRight = _ellipse.Bounds.BottomRight;
            }
        }

        public Point EndPoint
        {
            get { return _endPoint; }
            set
            {
                _endPoint = value;
                RedrawShape();
            }
        }

        public Point ROICenter
        {
            get { return _roiCenter; }
        }

        public double ROIHeight
        {
            get { return _roiHeight; }
            set
            {
                _roiHeight = value;
                RedrawShape();
            }
        }

        public double ROIWidth
        {
            get { return _roiWidth; }
            set
            {
                _roiWidth = value;
                RedrawShape();
            }
        }

        public override double ImageScaleX
        {
            get
            {
                return _imageScaleX;
            }
            set
            {
                _imageScaleX = value > 0 ? value : 1.0;
            }
        }

        public override double ImageScaleY
        {
            get
            {
                return _imageScaleY;
            }
            set
            {
                _imageScaleY = value > 0 ? value : 1.0;
            }
        }

        public Point StatsStartPoint
        {
            get => _statsStartPoint;
        }

        public Point StatsEndPoint
        {
            get => _statsEndPoint;
        }

        public Point StatsCenterPoint
        {
            get => _statsCenterPoint;
        }

        public bool ShiftDown
        {
            set { _shiftDown = value; }
        }

        public Point StartPoint
        {
            get { return _startPoint; }
            set
            {
                _startPoint = value;
                RedrawShape();
            }
        }

        public Point TopLeft
        {
            get { return _topLeft; }
        }

        public Point TopRight
        {
            get { return _topRight; }
        }

        protected override Geometry DefiningGeometry
        {
            get { return _ellipse; }
        }

        #endregion Properties

        #region Methods

        public override void ApplyScaleUpdate(double xScale, double yScale)
        {
            _toAdjustX = xScale / _imageScaleX;
            _toAdjustY = yScale / _imageScaleY;

            _imageScaleX = xScale;
            _imageScaleY = yScale;

            double updatedX = _startPoint.X * _toAdjustX;
            double updatedY = _startPoint.Y * _toAdjustY;

            _startPoint = new Point(updatedX, updatedY);

            updatedX = _endPoint.X * _toAdjustX;
            updatedY = _endPoint.Y * _toAdjustY;

            _endPoint = new Point(updatedX, updatedY);

            RedrawShape();
        }

        private void CalculateWidthAndHeight()
        {
            _roiWidth = Math.Abs(StartPoint.X - EndPoint.X);
            _roiHeight = Math.Abs(StartPoint.Y - EndPoint.Y);
            if (true == _shiftDown)
            {
                if (_roiWidth > _roiHeight)
                {
                    _roiWidth = _roiHeight;
                }
                else
                {
                    _roiHeight = _roiWidth;
                }

                if (_startPoint.X > _endPoint.X)
                {
                    _endPoint.X = _startPoint.X - _roiWidth;
                }
                else
                {
                    _endPoint.X = _startPoint.X + _roiWidth;
                }

                if (_startPoint.Y > _endPoint.Y)
                {
                    _endPoint.Y = _startPoint.Y - _roiHeight;
                }
                else
                {
                    _endPoint.Y = _startPoint.Y + _roiHeight;
                }
            }

            _statsStartPoint = new Point(_startPoint.X / _imageScaleX, _startPoint.Y / _imageScaleY);
            _statsEndPoint = new Point(_endPoint.X / _imageScaleX, _endPoint.Y / _imageScaleY);
            Rect r = new Rect(_statsStartPoint, _statsEndPoint);
            _statsCenterPoint = new Point(r.Width / 2 + r.Left, r.Height / 2 + r.Top);
        }

        private void RedrawShape()
        {
            CalculateWidthAndHeight();
            Rect r = new Rect(_startPoint, _endPoint);
            _ellipse.RadiusX = _roiWidth / 2;
            _ellipse.RadiusY = _roiHeight / 2;
            _ellipse.Center = new Point(r.Width / 2 + r.Left, r.Height / 2 + r.Top);
            _topLeft = _ellipse.Bounds.TopLeft;
            _topRight = _ellipse.Bounds.TopRight;
            _bottomLeft = _ellipse.Bounds.BottomLeft;
            _bottomRight = _ellipse.Bounds.BottomRight;
        }

        #endregion Methods
    }
}