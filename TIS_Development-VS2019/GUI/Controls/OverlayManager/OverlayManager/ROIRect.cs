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

    public class ROIRect : ScalableShape
    {
        #region Fields

        private Point _bottomLeft;
        private Point _bottomRight;
        private Point _endPoint;
        private RectangleGeometry _rectangle;
        private double _roiHeight;
        private double _roiWidth;
        private bool _shiftDown;
        private Point _startPoint;
        private Point _topLeft;
        private Point _topRight;

        private Point _statsStartPoint;
        private Point _statsEndPoint;

        private Point _statsTopLeft;
        private Point _statsTopRight;
        private Point _statsBottomLeft;
        private Point _statsBottomRight;

        private double _imageScaleX;
        private double _imageScaleY;
        private double _toAdjustX;
        private double _toAdjustY;

        #endregion Fields

        #region Constructors

        // constructor
        public ROIRect()
        {
            _startPoint = new Point(0, 0);
            _endPoint = new Point(0, 0);
            _topLeft = new Point(0, 0);
            _statsStartPoint = new Point(0, 0);
            _statsEndPoint = new Point(0, 0);
            _statsTopLeft = new Point(0,0);
            _statsTopRight = new Point(0, 0);
            _statsBottomLeft = new Point(0, 0);
            _statsBottomRight = new Point(0, 0);

            _roiHeight = 1;
            _roiWidth = 1;
            _imageScaleX = 1.0;
            _imageScaleY = 1.0;
            _toAdjustX = 1.0;
            _toAdjustY = 1.0;
            _rectangle = new RectangleGeometry();
            RedrawShape();
        }

        public ROIRect(Point start, Point end, double xScale, double yScale)
        {
            _startPoint = start;
            _endPoint = end;
            _statsStartPoint = start;
            _statsEndPoint = end;
            _topLeft = new Point(0, 0);
            _statsTopLeft = new Point(0, 0);
            _statsTopRight = new Point(0, 0);
            _statsBottomLeft = new Point(0, 0);
            _statsBottomRight = new Point(0, 0);

            _roiHeight = 1;
            _roiWidth = 1;
            _rectangle = new RectangleGeometry();
            _imageScaleX = xScale;
            _imageScaleY = yScale;
            _toAdjustX = 1.0/xScale;
            _toAdjustY = 1.0/yScale;
            RedrawShape();
        }

        #endregion Constructors

        #region Properties

        public Point BottomLeft
        {
            get { return _bottomLeft; }
            set
            {
                _bottomLeft = value;
            }
        }

        public Point BottomRight
        {
            get { return _bottomRight; }
            set
            {
                _bottomRight = value;
            }
        }

        public Point EndPoint
        {
            get { return _endPoint; }
            set
            {
                _endPoint = value;
                if (_endPoint.X == _startPoint.X)
                {
                    _endPoint.X = _startPoint.X + 1;
                }
                if (_endPoint.Y == _startPoint.Y)
                {
                    _endPoint.Y = _startPoint.Y + 1;
                }
                RedrawShape();
            }
        }

        public Rect Rect
        {
            set
            {
                _rectangle.Rect = value;
                _startPoint = _topLeft = _rectangle.Bounds.TopLeft;
                _topRight = _rectangle.Bounds.TopRight;
                _bottomLeft = _rectangle.Bounds.BottomLeft;
                _endPoint = _bottomRight = _rectangle.Bounds.BottomRight;
                _roiWidth = _rectangle.Bounds.Width;
                _roiHeight = _rectangle.Bounds.Height;
                RedrawShape();
            }
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
            set
            {
                _topLeft = value;
            }
        }

        public Point TopRight
        {
            get { return _topRight; }
            set
            {
                _topRight = value;
            }
        }

        public Point StatsTopLeft
        {
            get => _statsTopLeft;
        }

        public Point StatsTopRight
        {
            get => _statsTopRight;
        }

        public Point StatsBottomLeft
        {
            get => _statsBottomLeft;
        }

        public Point StatsBottomRight
        {
            get => _statsBottomRight;
        }

        protected override Geometry DefiningGeometry
        {
            get { return _rectangle; }
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
        }

        private void RedrawShape()
        {
            CalculateWidthAndHeight();
            if (_startPoint.X <= _endPoint.X)
            {
                if (_startPoint.Y <= _endPoint.Y)
                {
                    _rectangle.Rect = new Rect(_startPoint.X, _startPoint.Y, _roiWidth, _roiHeight);
                }
                else
                {
                    _rectangle.Rect = new Rect(_startPoint.X, _endPoint.Y, _roiWidth, _roiHeight);
                }
            }
            else
            {
                if (_startPoint.Y <= _endPoint.Y)
                {
                    _rectangle.Rect = new Rect(_endPoint.X, _startPoint.Y, _roiWidth, _roiHeight);
                }
                else
                {
                    _rectangle.Rect = new Rect(_endPoint.X, _endPoint.Y, _roiWidth, _roiHeight);
                }
            }
            _topLeft = _rectangle.Bounds.TopLeft;
            _topRight = _rectangle.Bounds.TopRight;
            _bottomLeft = _rectangle.Bounds.BottomLeft;
            _bottomRight = _rectangle.Bounds.BottomRight;

            _statsTopLeft = new Point(_rectangle.Bounds.TopLeft.X/_imageScaleX, _rectangle.Bounds.TopLeft.Y/_imageScaleY);
            _statsTopRight = new Point(_rectangle.Bounds.TopRight.X / _imageScaleX, _rectangle.Bounds.TopRight.Y / _imageScaleY);
            _statsBottomLeft = new Point(_rectangle.Bounds.BottomLeft.X / _imageScaleX, _rectangle.Bounds.BottomLeft.Y / _imageScaleY);
            _statsBottomRight = new Point(_rectangle.Bounds.BottomRight.X / _imageScaleX, _rectangle.Bounds.BottomRight.Y / _imageScaleY);

        }

        #endregion Methods
    }
}