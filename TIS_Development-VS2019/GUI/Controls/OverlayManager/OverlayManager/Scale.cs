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

    public class Scale : ScalableShape
    {
        #region Fields

        protected double _scaleFieldWidth;
        protected double _scaleFieldHeight;

        protected int _imageHeight;
        protected int _imageWidth;
        protected GeometryGroup _scaleGroup;

        private double _imageScaleX;
        private double _imageScaleY;

        #endregion Fields

        #region Constructors

        public Scale()
        {
            _imageWidth = 1;
            _imageHeight = 1;
            _scaleFieldWidth = 1.0;
            _scaleFieldHeight = 1.0;
            _imageScaleX = 1;
            _imageScaleY = 1;
            _scaleGroup = new GeometryGroup();
        }

        public Scale(double xScale, double yScale)
        {
            _imageWidth = 1;
            _imageHeight = 1;
            _scaleFieldWidth = 1.0;
            _scaleFieldHeight = 1.0;
            _imageScaleX = xScale;
            _imageScaleY = yScale;
            _scaleGroup = new GeometryGroup();
        }

        #endregion Constructors

        #region Properties

        public double XScaleImageLen
        {
            get
            {
                return _imageWidth * XScaleLen / _scaleFieldWidth;
            }
        }

        public double YScaleImageLen
        {
            get
            {
                return _imageHeight * _imageScaleY * YScaleLen / _scaleFieldHeight;
            }
        }

        public double XScaleLen
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

        public double YScaleLen
        {
            get
            {
                double scaleLen = _scaleFieldHeight / 4.0;
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

        public double RelativeYFactor
        {
            get
            {
                return _imageHeight * ImageScaleY / 20.0;
            }
        }

        public double RelativeXFactor
        {
            get
            {
                return _imageWidth * ImageScaleX / 20.0;
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

        public double ScaleFieldHeight
        {
            get { return _scaleFieldHeight; }
            set
            {
                _scaleFieldHeight = Math.Max(1, value);
                RedrawShape();
            }
        }

        protected override Geometry DefiningGeometry
        {
            get { return _scaleGroup; }
        }

        #endregion Properties

        #region Methods

        public override void ApplyScaleUpdate(double xScale, double yScale)
        {
            _imageScaleX = xScale;
            _imageScaleY = yScale;
            RedrawShape();
        }

        protected virtual void RedrawShape()
        {

        }
        #endregion Methods
    }

    public class ScaleLines : Scale
    {
        #region Methods

        public override void ApplyScaleUpdate(double xScale, double yScale)
        {
            base.ApplyScaleUpdate(xScale, yScale);
            RedrawShape();
        }

        protected override void RedrawShape()
        {
            LineGeometry lineXBridge = new LineGeometry();
            LineGeometry lineXLeft = new LineGeometry();
            LineGeometry lineXRight = new LineGeometry();

            LineGeometry lineYBottom = new LineGeometry();
            LineGeometry lineYTop = new LineGeometry();
            LineGeometry lineYBridge = new LineGeometry();

            double multFactor = 3.0 / 32.0;

            _scaleGroup.Children.Clear();

            // Bounding points for scale in X
            Point xScaleBottomLeftCorner = new Point();
            xScaleBottomLeftCorner.X = _imageWidth * ImageScaleX * multFactor - 0.25;    // left bottom corner
            xScaleBottomLeftCorner.Y = RelativeYFactor * 18.5 + 0.5;

            Point xScaleBottomRightCorner = new Point();
            xScaleBottomRightCorner.X = _imageWidth * ImageScaleX * multFactor + XScaleImageLen - 0.25;    // right bottom corner
            xScaleBottomRightCorner.Y = RelativeYFactor * 18.5 + 0.5;

            Point xScaleTopLeftCorner = new Point();
            xScaleTopLeftCorner.X = _imageWidth * ImageScaleX * multFactor - 0.25;    // left top corner
            xScaleTopLeftCorner.Y = RelativeYFactor * 19 - 0.5;

            Point xScaleTopRightCorner = new Point();
            xScaleTopRightCorner.X = _imageWidth * ImageScaleX * multFactor + XScaleImageLen - 0.25;    // right top corner
            xScaleTopRightCorner.Y = RelativeYFactor * 19 - 0.5;

            // Bounding points for scale in Y
            Point yScaleBottomLeftCorner = new Point();
            yScaleBottomLeftCorner.X = RelativeXFactor + 0.5;    // left bottom corner
            yScaleBottomLeftCorner.Y = _imageHeight * ImageScaleY - _imageHeight * ImageScaleY * multFactor - 0.25;

            Point yScaleBottomRightCorner = new Point();
            yScaleBottomRightCorner.X = RelativeXFactor * 1.5 + 0.5;    // right bottom corner
            yScaleBottomRightCorner.Y = _imageHeight * ImageScaleY - _imageHeight * ImageScaleY * multFactor - 0.25;

            Point yScaleTopLeftCorner = new Point();
            yScaleTopLeftCorner.X = RelativeXFactor + 0.5;    // left top corner
            yScaleTopLeftCorner.Y = _imageHeight * ImageScaleY - _imageHeight * ImageScaleY * multFactor - YScaleImageLen - 0.25;

            Point yScaleTopRightCorner = new Point();
            yScaleTopRightCorner.X = RelativeXFactor * 1.5 + 0.5;    // right top corner
            yScaleTopRightCorner.Y = _imageHeight * ImageScaleY - _imageHeight * ImageScaleY * multFactor - YScaleImageLen - 0.25;

            /*            Point markOrigin = new Point();
                        markOrigin.X = xScaleTopLeftCorner.X + ScaleImageLen / 2;    // left top corner
                        markOrigin.Y = xScaleTopLeftCorner.Y - 1.22 * RelativeFactor;*/

            // construct x scale lines
            lineXLeft.StartPoint = new Point(xScaleTopLeftCorner.X, xScaleTopLeftCorner.Y);
            lineXLeft.EndPoint = new Point(xScaleBottomLeftCorner.X, xScaleBottomLeftCorner.Y);

            lineXRight.StartPoint = new Point(xScaleTopRightCorner.X, xScaleTopRightCorner.Y);
            lineXRight.EndPoint = new Point(xScaleBottomRightCorner.X, xScaleBottomRightCorner.Y);

            lineXBridge.StartPoint = new Point(xScaleTopLeftCorner.X - 2, xScaleTopLeftCorner.Y);
            lineXBridge.EndPoint = new Point(xScaleTopRightCorner.X + 2, xScaleTopRightCorner.Y);


            // construct y scale lines
            lineYTop.StartPoint = new Point(yScaleTopLeftCorner.X, yScaleTopLeftCorner.Y);
            lineYTop.EndPoint = new Point(yScaleTopRightCorner.X, yScaleTopRightCorner.Y);

            lineYBottom.StartPoint = new Point(yScaleBottomLeftCorner.X, yScaleBottomLeftCorner.Y);
            lineYBottom.EndPoint = new Point(yScaleBottomRightCorner.X, yScaleBottomRightCorner.Y);

            lineYBridge.StartPoint = new Point(yScaleTopLeftCorner.X, yScaleTopLeftCorner.Y - 2);
            lineYBridge.EndPoint = new Point(yScaleBottomLeftCorner.X, yScaleBottomLeftCorner.Y + 2);


            // add lines for X Scale
            _scaleGroup.Children.Add(lineXLeft);
            _scaleGroup.Children.Add(lineXRight);
            _scaleGroup.Children.Add(lineXBridge);

            //Only add the scale to the Y if the Pixel Aspect Ratio is not 1:1
            double pixelSizeRatio = (_scaleFieldWidth / _imageWidth) / (_scaleFieldHeight / _imageHeight);
            if (ImageScaleX != ImageScaleY || Math.Round(pixelSizeRatio, 1) != 1)
            { 
                _scaleGroup.Children.Add(lineYTop);
                _scaleGroup.Children.Add(lineYBottom);
                _scaleGroup.Children.Add(lineYBridge);
            }
        }

        #endregion Methods
    }

    public class ScaleNumbers : Scale
    {
        #region Methods

        public override void ApplyScaleUpdate(double xScale, double yScale)
        {
            base.ApplyScaleUpdate(xScale, yScale);
            RedrawShape();
        }

        protected override void RedrawShape()
        {
            _scaleGroup.Children.Clear();

            Point lbCor = new Point();
            lbCor.X = _imageWidth * ImageScaleX * 3.0 / 32.0 - 0.5;    // left top corner
            lbCor.Y = RelativeYFactor * 19 - 0.5;

            Point markOrigin = new Point();
            markOrigin.X = lbCor.X + XScaleImageLen / 2;    // left top corner
            markOrigin.Y = lbCor.Y - 1.22 * RelativeYFactor;
            // text
            var scaleTxtX = new FormattedText(((int)XScaleLen).ToString(),
                                             CultureInfo.CurrentUICulture,
                                             FlowDirection.LeftToRight,
                                             new Typeface("Arial"),
                                             Math.Max((int)(Math.Max(RelativeYFactor/ ImageScaleY, _imageWidth / 20)), 1),
                                             Brushes.Green,
                                             VisualTreeHelper.GetDpi(this).PixelsPerDip);
            scaleTxtX.TextAlignment = TextAlignment.Center;
            Geometry scaleMarkX = scaleTxtX.BuildGeometry(markOrigin);

            Point lbCor2 = new Point();
            lbCor2.X = RelativeXFactor * 2 - 0.5;
            lbCor2.Y = _imageHeight * ImageScaleY - _imageHeight * ImageScaleY * 3.0 / 32.0 - 0.5; 

            Point markOrigin2 = new Point();
            markOrigin2.X = lbCor2.X;
            markOrigin2.Y = lbCor2.Y - YScaleImageLen / 2 - Math.Max((int)(Math.Max(RelativeYFactor / (ImageScaleY * 2), _imageWidth / 40)), 1);

            var scaleTxtY = new FormattedText(((int)YScaleLen).ToString(),
                                 CultureInfo.CurrentUICulture,
                                 FlowDirection.LeftToRight,
                                 new Typeface("Arial"),
                                 Math.Max((int)(Math.Max(RelativeYFactor/ ImageScaleY, _imageWidth / 20)), 1),
                                 Brushes.Green,
                                 VisualTreeHelper.GetDpi(this).PixelsPerDip);
            scaleTxtY.TextAlignment = TextAlignment.Center;
            Geometry scaleMarkY = scaleTxtY.BuildGeometry(markOrigin2);

            //Put Reticle together            
            _scaleGroup.Children.Add(scaleMarkX);

            //Only add the scale to the Y if the Pixel Aspect Ratio is not 1:1
            double pixelSizeRatio = (_scaleFieldWidth / _imageWidth) / (_scaleFieldHeight / _imageHeight);
            if (ImageScaleX != ImageScaleY || Math.Round(pixelSizeRatio, 1) != 1)
            {
                _scaleGroup.Children.Add(scaleMarkY);
            }

        }

        #endregion Methods
    }
}