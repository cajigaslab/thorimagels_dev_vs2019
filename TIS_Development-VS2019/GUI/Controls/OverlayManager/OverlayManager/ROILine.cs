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

    public class ROILine : ScalableShape
    {
        #region Fields

        private LineGeometry _line;
        private Point _startPoint;
        private Point _endPoint;
        private bool _shiftDown;

        private Point _statsStartPoint;
        private Point _statsEndPoint;
        private double _imageScaleX;
        private double _imageScaleY;
        private double _toAdjustX;
        private double _toAdjustY;
        #endregion Fields

        #region Constructors
        public ROILine()
        {
            _startPoint = new Point(0, 0);
            _endPoint = new Point(0, 0);
            _line = new LineGeometry();

            _statsStartPoint = new Point(0, 0);
            _statsEndPoint = new Point(0, 0);
            _imageScaleX = 1.0;
            _imageScaleY = 1.0;
            _toAdjustX = 1.0;
            _toAdjustY = 1.0;
            RedrawShape();
        }

        public ROILine(Point startPoint, Point endPoint, double xScale, double yScale)
        {
            _startPoint = startPoint;
            _endPoint = endPoint;
            _line = new LineGeometry();

            _imageScaleX = xScale;
            _imageScaleY = yScale;
            _toAdjustX = 1.0 / xScale;
            _toAdjustY = 1.0 / yScale;
            RedrawShape();
        }
        #endregion Constructors

        #region Properties
        protected override Geometry DefiningGeometry
        {
            get { return _line; }
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

        public Point EndPoint
        {
            get { return _endPoint; }
            set
            {
                _endPoint = value;
                RedrawShape();
            }
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

        public Point StatsStartPoint
        {
            get { return _statsStartPoint; }
        }

        public Point StatsEndPoint
        {
            get { return _statsEndPoint; }
        }

        public bool ShiftDown
        {
            set { _shiftDown = value; }
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

        private void CalculatePoints()
        {
            if (true == _shiftDown)
            {
                if (Math.Abs(_startPoint.X - _endPoint.X) >= Math.Abs(_startPoint.Y - _endPoint.Y))
                {
                    _endPoint.Y = _startPoint.Y;
                }
                else
                {
                    _endPoint.X = _startPoint.X;
                }
            }
            _statsStartPoint = new Point(_startPoint.X / _imageScaleX, _startPoint.Y / _imageScaleY);
            _statsEndPoint = new Point(_endPoint.X / _imageScaleX, _endPoint.Y / _imageScaleY);
        }


        private void RedrawShape()
        {
            CalculatePoints();
            _line.StartPoint = _startPoint;
            _line.EndPoint = _endPoint;
        }
        #endregion Methods
    }
}
