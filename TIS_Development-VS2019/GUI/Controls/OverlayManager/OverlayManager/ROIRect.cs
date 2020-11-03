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

    public class ROIRect : Shape
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

        #endregion Fields

        #region Constructors

        // constructor
        public ROIRect()
        {
            _startPoint = new Point(0, 0);
            _endPoint = new Point(0, 0);
            _topLeft = new Point(0, 0);
            _roiHeight = 1;
            _roiWidth = 1;
            _rectangle = new RectangleGeometry();
            RedrawShape();
        }

        public ROIRect(Point start, Point end)
        {
            _startPoint = start;
            _endPoint = end;
            _topLeft = new Point(0, 0);
            _roiHeight = 1;
            _roiWidth = 1;
            _rectangle = new RectangleGeometry();
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

        protected override Geometry DefiningGeometry
        {
            get { return _rectangle; }
        }

        #endregion Properties

        #region Methods

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
        }

        #endregion Methods
    }
}