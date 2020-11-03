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

    public class Reticle : Shape
    {
        #region Fields

        private Point _centerPoint;
        private int _imageHeight;
        private int _imageWidth;
        private GeometryGroup _reticleGroup;

        #endregion Fields

        #region Constructors

        // constructor
        public Reticle()
        {
            _imageWidth = 0;
            _imageHeight = 0;
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

        #endregion Properties

        #region Methods

        private void RedrawShape()
        {
            LineGeometry lineU = new LineGeometry();
            LineGeometry lineD = new LineGeometry();
            LineGeometry lineL = new LineGeometry();
            LineGeometry lineR = new LineGeometry();
            _centerPoint = new Point();
            _reticleGroup = new GeometryGroup();
            RectangleGeometry innerRectangle = new RectangleGeometry();
            double centerX = _imageWidth / 2 + 0.5;
            double centerY = _imageHeight / 2 + 0.5;
            Point pt = new Point();
            pt.X = centerX;    // center of image
            pt.Y = centerY;
            _centerPoint = pt;

            // line horizontal
            lineL.StartPoint = new Point(0, centerY);
            lineL.EndPoint = new Point(centerX - 0.5, centerY);

            lineR.StartPoint = new Point(centerX + 0.5, centerY);
            lineR.EndPoint = new Point(_imageWidth, centerY);

            // line verticle
            lineU.StartPoint = new Point(centerX, 0);
            lineU.EndPoint = new Point(centerX, centerY - 0.5);

            lineD.StartPoint = new Point(centerX, centerY + 0.5);
            lineD.EndPoint = new Point(centerX, _imageHeight);

            // inner rectangle
            double halfRectWidth = _imageWidth / 20;
            double halfRectHeight = _imageHeight / 20;
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