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

    public class Reticle : ScalableShape
    {
        #region Fields

        private Point _centerPoint;
        private int _imageHeight;
        private int _imageWidth;
        private double _xScale;
        private double _yScale;
        private double _toAdjustX;
        private double _toAdjustY;
        private GeometryGroup _reticleGroup;

        #endregion Fields

        #region Constructors

        // constructor
        public Reticle()
        {
            _imageWidth = 0;
            _imageHeight = 0;
            _xScale = 1.0;
            _yScale = 1.0;
            _centerPoint = new Point();
            _reticleGroup = new GeometryGroup();
        }

        public Reticle(double scaleX, double scaleY)
        {
            _imageWidth = 0;
            _imageHeight = 0;
            _xScale = scaleX;
            _yScale = scaleY;
            _yScale = 1.0;
            _centerPoint = new Point();
            _reticleGroup = new GeometryGroup();
        }

        #endregion Constructors

        #region Properties

        public Point CenterPoint
        {
            get { return _centerPoint; }
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

        protected override Geometry DefiningGeometry
        {
            get { return _reticleGroup; }
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

        #endregion Properties

        #region Methods

        public override void ApplyScaleUpdate(double xScale, double yScale)
        {
            _toAdjustX = xScale / _xScale;
            _toAdjustY = yScale / _yScale;

            _xScale = xScale;
            _yScale = yScale;

            RedrawShape();
        }

        private void RedrawShape()
        {
            LineGeometry lineU = new LineGeometry();
            LineGeometry lineD = new LineGeometry();
            LineGeometry lineL = new LineGeometry();
            LineGeometry lineR = new LineGeometry();
            _centerPoint = new Point();
            _reticleGroup.Children.Clear();
            RectangleGeometry innerRectangle = new RectangleGeometry();
            double centerX = _imageWidth * _xScale / 2 + 0.5;
            double centerY = _imageHeight * _yScale / 2 + 0.5;
            Point pt = new Point();
            pt.X = centerX;    // center of image
            pt.Y = centerY;
            _centerPoint = pt;

            // line horizontal
            lineL.StartPoint = new Point(0, centerY);
            lineL.EndPoint = new Point(centerX - 0.5, centerY);

            lineR.StartPoint = new Point(centerX + 0.5, centerY);
            lineR.EndPoint = new Point(_imageWidth * _xScale, centerY);

            // line verticle
            lineU.StartPoint = new Point(centerX, 0);
            lineU.EndPoint = new Point(centerX, centerY - 0.5);

            lineD.StartPoint = new Point(centerX, centerY + 0.5);
            lineD.EndPoint = new Point(centerX, _imageHeight * _yScale);

            // inner rectangle
            double halfRectWidth = _imageWidth * _xScale / 20;
            double halfRectHeight = _imageHeight * _yScale / 20;
            double rectangleTop = pt.Y - halfRectHeight + 1;
            double rectangleLeft = pt.X - halfRectWidth + 1;
            double rectangleBottom = pt.Y + halfRectHeight;
            double rectangleRight = pt.X + halfRectWidth;

            innerRectangle.Rect = new Rect(pt.X - halfRectWidth, pt.Y - halfRectHeight, 2 * halfRectWidth, 2 * halfRectHeight);

            //Put Reticle together
            _reticleGroup.Children.Add(lineL);
            _reticleGroup.Children.Add(lineR);
            _reticleGroup.Children.Add(lineU);
            _reticleGroup.Children.Add(lineD);
            _reticleGroup.Children.Add(innerRectangle);
        }

        #endregion Methods
    }
}