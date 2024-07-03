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

    public class ROICrosshair : ScalableShape
    {
        #region Fields
        private int _imageHeight;
        private int _imageWidth;
        private Point _centerPoint;
        private GeometryGroup _crossHairGroup;
        private double _size = 7;
        private double _wavelength = 0;

        private Point _statsCenterPoint;
        private double _xScale;
        private double _yScale;
        private double _toAdjustX;
        private double _toAdjustY;

        #endregion Fields

        #region Constructors

        // constructor
        public ROICrosshair()
        {
            _centerPoint = new Point(0, 0);
            _crossHairGroup = new GeometryGroup();

            _statsCenterPoint = new Point(0, 0);
            _imageWidth = 0;
            _imageHeight = 0;
            _xScale = 1.0;
            _yScale = 1.0;
            _toAdjustX = 1.0;
            _toAdjustY = 1.0;
            RedrawShape();
        }

        public ROICrosshair(Point center, double xScale, double yScale)
        {
            _centerPoint = center;
            _crossHairGroup = new GeometryGroup();
            _statsCenterPoint = center;
            _imageWidth = 0;
            _imageHeight = 0;
            _xScale = xScale;
            _yScale = yScale;
            _toAdjustX = 1.0 / xScale;
            _toAdjustY = 1.0 / yScale;
            RedrawShape();
        }

        #endregion Constructors

        #region Properties

        public Point CenterPoint
        {
            get { return _centerPoint; }
            set
            {
                _centerPoint = value;
                RedrawShape();
            }
        }

        public int ImageHeight
        {
            get { return _imageHeight; }
            set
            {
                _imageHeight = value;
                RedrawShape();
            }
        }

        public int ImageWidth
        {
            get { return _imageWidth; }
            set
            {
                _imageWidth = value;
                RedrawShape();
            }
        }

        public override double ImageScaleX
        {
            get
            {
                return _xScale;
            }
            set
            {
                _xScale = value > 0 ? value : 1.0;
            }
        }

        public override double ImageScaleY
        {
            get
            {
                return _yScale;
            }
            set
            {
                _yScale = value > 0 ? value : 1.0;
            }
        }

        public double Size
        {
            get => _size;
        }

        public Point StatsCenterPoint
        {
            get => _statsCenterPoint;
        }

        public double Wavelength
        {
            get { return _wavelength; }
            set { _wavelength = value; }
        }

        protected override Geometry DefiningGeometry
        {
            get { return _crossHairGroup; }
        }

        #endregion Properties

        #region Methods

        public override void ApplyScaleUpdate(double xScale, double yScale)
        {
            _toAdjustX = xScale / _xScale;
            _toAdjustY = yScale / _yScale;

            _xScale = xScale;
            _yScale = yScale;

            double updatedX = _centerPoint.X * _toAdjustX;
            double updatedY = _centerPoint.Y * _toAdjustY;

            _centerPoint = new Point(updatedX, updatedY);
            RedrawShape();
        }

        private void RedrawShape()
        {
            LineGeometry lineV = new LineGeometry();
            LineGeometry lineH = new LineGeometry();
            EllipseGeometry innerCircle = new EllipseGeometry();

            if (_imageHeight == 0 || _imageWidth == 0)
            {
                _size = 7;
            }
            else 
            {
                _size = Math.Max(_imageWidth, _imageHeight) / 50.0;
            }

            // line Vertical
            lineV.StartPoint = new Point(_centerPoint.X, _centerPoint.Y + _size);
            lineV.EndPoint = new Point(_centerPoint.X, _centerPoint.Y - _size);

            // line horizontal
            lineH.StartPoint = new Point(_centerPoint.X + _size, _centerPoint.Y);
            lineH.EndPoint = new Point(_centerPoint.X - _size, _centerPoint.Y);

            innerCircle.Center = _centerPoint;
            innerCircle.RadiusX = _size / 2;
            innerCircle.RadiusY = _size / 2;

            //Clear previous crosshair
            _crossHairGroup.Children.Clear();

            //Put Crosshair together
            _crossHairGroup.Children.Add(lineV);
            _crossHairGroup.Children.Add(lineH);
            _crossHairGroup.Children.Add(innerCircle);

            _statsCenterPoint = new Point(_centerPoint.X / _xScale, _centerPoint.Y / _yScale);
        }

        #endregion Methods
    }
}