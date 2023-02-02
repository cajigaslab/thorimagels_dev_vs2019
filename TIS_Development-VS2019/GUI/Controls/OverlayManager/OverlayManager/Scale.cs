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
    using System.Globalization;

    public class Scale : Shape
    {
        #region Fields

        protected double _scaleFieldWidth;
        protected int _imageHeight;
        protected int _imageWidth;
        protected GeometryGroup _scaleGroup;

        #endregion Fields

        #region Constructors

        // constructor
        public Scale()
        {
            _imageWidth = 1;
            _imageHeight = 1;
            _scaleFieldWidth = 1.0;
            _scaleGroup = new GeometryGroup();
        }

        #endregion Constructors

        #region Properties

        public double ScaleImageLen
        {
            get
            {
                return _imageWidth * ScaleLen / _scaleFieldWidth;
            }
        }

        public double ScaleLen
        {
            get
            {
                double scaleLen = _scaleFieldWidth / 4.0;
                if (scaleLen > 100.0)
                {
                    scaleLen = Math.Round(scaleLen / 100.0, 0) * 100;
                }
                else if (scaleLen > 10.0)
                {
                    scaleLen = Math.Round(scaleLen / 10.0, 0) * 10;
                }
                else
                {
                    scaleLen = Math.Round(scaleLen, 0);
                }
                return scaleLen;
            }
        }

        public double RelativeFactor
        {
            get
            {
                return _imageHeight / 20.0;
            }
        }

        public int ImageHeight
        {
            get { return _imageHeight; }
            set
            {
                _imageHeight = Math.Max(1, value);
                RedrawShape();
            }
        }

        public int ImageWidth
        {
            get { return _imageWidth; }
            set
            {
                _imageWidth = Math.Max(1, value);
                RedrawShape();
            }
        }

        public double ScaleFieldWidth
        {
            get { return _scaleFieldWidth; }
            set
            {
                _scaleFieldWidth = Math.Max(1, value);
                RedrawShape();
            }
        }

        protected override Geometry DefiningGeometry
        {
            get { return _scaleGroup; }
        }

        #endregion Properties

        #region Methods

        protected virtual void RedrawShape()
        {

        }

        #endregion Methods
    }

    public class ScaleLines : Scale
    {
        #region Methods

        protected override void RedrawShape()
        {
            LineGeometry lineD = new LineGeometry();

            LineGeometry lineL = new LineGeometry();
            LineGeometry lineR = new LineGeometry();

            _scaleGroup = new GeometryGroup();


            Point ltCor = new Point();
            ltCor.X = _imageWidth * 3.0 / 32.0 - 0.25;    // left bottom corner
            ltCor.Y = RelativeFactor * 18.5 + 0.5;

            Point rtCor = new Point();
            rtCor.X = _imageWidth * 3.0 / 32.0 + ScaleImageLen - 0.25;    // right bottom corner
            rtCor.Y = RelativeFactor * 18.5 + 0.5;

            Point lbCor = new Point();
            lbCor.X = _imageWidth * 3.0 / 32.0 - 0.25;    // left top corner
            lbCor.Y = RelativeFactor * 19 - 0.5;

            Point rbCor = new Point();
            rbCor.X = _imageWidth * 3.0 / 32.0 + ScaleImageLen - 0.25;    // right top corner
            rbCor.Y = RelativeFactor * 19 - 0.5;

            Point markOrigin = new Point();
            markOrigin.X = lbCor.X + ScaleImageLen / 2;    // left top corner
            markOrigin.Y = lbCor.Y - 1.22 * RelativeFactor;

            // line
            lineL.StartPoint = new Point(lbCor.X, lbCor.Y);
            lineL.EndPoint = new Point(ltCor.X, ltCor.Y);

            lineR.StartPoint = new Point(rbCor.X, rbCor.Y);
            lineR.EndPoint = new Point(rtCor.X, rtCor.Y);

            lineD.StartPoint = new Point(lbCor.X - 2, lbCor.Y);
            lineD.EndPoint = new Point(rbCor.X + 2, rbCor.Y);

            //Put Reticle together            
            _scaleGroup.Children.Add(lineL);
            _scaleGroup.Children.Add(lineR);
            _scaleGroup.Children.Add(lineD);

        }

        #endregion Methods
    }

    public class ScaleNumbers : Scale
    {
        #region Methods

        protected override void RedrawShape()
        {
            _scaleGroup = new GeometryGroup();

            Point lbCor = new Point();
            lbCor.X = _imageWidth * 3.0 / 32.0 - 0.5;    // left top corner
            lbCor.Y = RelativeFactor * 19 - 0.5;

            Point markOrigin = new Point();
            markOrigin.X = lbCor.X + ScaleImageLen / 2;    // left top corner
            markOrigin.Y = lbCor.Y - 1.22 * Math.Max(RelativeFactor, _imageWidth / 20);
            // text
            var scaleTxt = new FormattedText(((int)ScaleLen).ToString(),
                                             CultureInfo.CurrentUICulture,
                                             FlowDirection.LeftToRight,
                                             new Typeface("Arial"),
                                             Math.Max((int)(Math.Max(RelativeFactor, _imageWidth / 20)), 1),
                                             Brushes.Green,
                                             VisualTreeHelper.GetDpi(this).PixelsPerDip);
            scaleTxt.TextAlignment = TextAlignment.Center;
            Geometry scaleMark = scaleTxt.BuildGeometry(markOrigin);

            //Put Reticle together            
            _scaleGroup.Children.Add(scaleMark);
        }

        #endregion Methods
    }
}