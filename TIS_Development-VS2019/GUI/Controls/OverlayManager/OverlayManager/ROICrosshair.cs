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

    public class ROICrosshair : Shape
    {
        #region Fields

        private Point _centerPoint;
        private GeometryGroup _crossHairGroup;
        private double _size = 7;
        private double _wavelength = 0;

        #endregion Fields

        #region Constructors

        // constructor
        public ROICrosshair()
        {
            _centerPoint = new Point(0, 0);
            _crossHairGroup = new GeometryGroup();
            RedrawShape();
        }

        public ROICrosshair(Point center)
        {
            _centerPoint = center;
            _crossHairGroup = new GeometryGroup();
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

        private void RedrawShape()
        {
            LineGeometry lineV = new LineGeometry();
            LineGeometry lineH = new LineGeometry();
            EllipseGeometry innerCircle = new EllipseGeometry();

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
        }

        #endregion Methods
    }
}